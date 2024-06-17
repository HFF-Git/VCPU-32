//------------------------------------------------------------------------------------------------------------
//
// VCPU32 - A 32-bit CPU - A TLB for VCPU-32
//
//------------------------------------------------------------------------------------------------------------
// A TLB is a translation cache. It contains the virtual adress and the corresponding physical adress as well
// as access rights information for the page. Each virtual memory reference will conslt the TLB, it is on the
// critical path.
//
//
//------------------------------------------------------------------------------------------------------------
//
// VCPU32 - A 32-bit CPU - A TLB for VCPU-32
// Copyright (C) 2022 - 2024 Helmut Fieres
//
// This program is free software: you can redistribute it and/or modify it under the terms of the GNU
// General Public License as published by the Free Software Foundation, either version 3 of the License,
// or any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
// License for more details. You should have received a copy of the GNU General Public License along with
// this program.  If not, see <http://www.gnu.org/licenses/>.
//
//------------------------------------------------------------------------------------------------------------
#include "VCPU32-Types.hpp"
#include "VCPU32-Core.hpp"


//------------------------------------------------------------------------------------------------------------
// File local declarations. There are constants and routines used internally and not visible outside of this
// file. Most of the routines are inline functions. They also could have been defined as C++ style defines,
// but this is a bit clearer to read.
//
//------------------------------------------------------------------------------------------------------------
namespace {

//------------------------------------------------------------------------------------------------------------
//
//
//------------------------------------------------------------------------------------------------------------
const uint32_t  MAX_TLB_SIZE    = 2048;
const uint8_t   SEG_SHIFT       = 4;

//------------------------------------------------------------------------------------------------------------
// TLB state mchine states.
//
//------------------------------------------------------------------------------------------------------------
enum tlbOpState : uint32_t {
    
    TO_IDLE             = 0,
    TO_REQ_INSERT_ADR   = 1,
    TO_REQ_INSERT_PROT  = 2,
    TO_REQ_PURGE        = 3
};

//------------------------------------------------------------------------------------------------------------
//
//
//------------------------------------------------------------------------------------------------------------
bool getBit( uint32_t arg, int pos ) {
    
    return( arg & ( 1U << ( 31 - ( pos % 32 ))));
}

void setBit( uint32_t *arg, int pos, bool val = true ) {
   
    if ( val )  *arg |= ( 1U << ( 31 - ( pos % 32 )));
    else        *arg &= ~( 1U << ( 31 - ( pos % 32 )));
}

static inline uint32_t getBitField( uint32_t arg, int pos, int len, bool sign = false ) {
    
    pos = pos % 32;
    len = len % 32;
    
    uint32_t tmpM = ( 1U << len ) - 1;
    uint32_t tmpA = arg >> ( 31 - pos );
    
    if ( sign ) return( tmpA | ( ~ tmpM ));
    else        return( tmpA & tmpM );
}

//------------------------------------------------------------------------------------------------------------
// A little helper functions to round up the TLB size to power of two.
//
//------------------------------------------------------------------------------------------------------------
uint32_t roundUp( uint32_t size ) {
    
    int power = 1;
    while(( power < size ) && ( power < MAX_TLB_SIZE )) power *= 2;
    
    return( power );
}

//------------------------------------------------------------------------------------------------------------
// This is the TLB hashing function.
//
//------------------------------------------------------------------------------------------------------------
uint16_t hashTlb( uint32_t seg, uint32_t ofs, uint32_t tlbSize ) {
    
    return((( seg << SEG_SHIFT ) ^ ( ofs >> PAGE_SIZE_BITS )) % tlbSize );
}

}; // namespace

//------------------------------------------------------------------------------------------------------------
// The TLB object. It is just an array of TLB entries. Any reference is done by using the hash function to
// get to an entry. The TLB size is rounded up to the nearest power of 2 from the passed TLB size.
//
//------------------------------------------------------------------------------------------------------------
CpuTlb::CpuTlb( TlbDesc *cfg ) {
    
    memcpy( &tlbDesc, cfg, sizeof( TlbDesc ));
    
    tlbDesc.entries = roundUp( tlbDesc.entries );
    tlbArray        = (TlbEntry *) calloc( tlbDesc.entries, sizeof( TlbEntry ));
    
    reset( );
}

//------------------------------------------------------------------------------------------------------------
// Clear the TLB. This is just a simple clear of all entries in the array.
//
//------------------------------------------------------------------------------------------------------------
void CpuTlb::reset( ) {
    
    for ( uint16_t i = 0; i < tlbDesc.entries; i++ ) tlbArray[ i ].setValid( false );
}

//------------------------------------------------------------------------------------------------------------
// Clear the TLB. This is just a simple clear of all entries in the array.
//
//------------------------------------------------------------------------------------------------------------
void CpuTlb::clearStats( ) {
    
    tlbInserts      = 0;
    tlbDeletes      = 0;
    tlbAccess       = 0;
    tlbMiss         = 0;
    tlbWaitCycles   = 0;
}

//------------------------------------------------------------------------------------------------------------
// The tick routine. The tick function, representing the CPU clock, is used to implement the TLB operation
// time for inserts and deletes in CPU cycles.
//
//------------------------------------------------------------------------------------------------------------
void CpuTlb::tick( ) {
    
    if (( tlbOpState != TO_IDLE ) & ( reqDelayCnt > 0 )) reqDelayCnt --;
}

void CpuTlb::process( ) {
    
    
}

//------------------------------------------------------------------------------------------------------------
// The insert TLB protection info method is the first part of the TLB insert routine. The entry is updated
// bit not set valid yet. This will be done with the second method. Note that the entry is just overwritten
// in any case. To simulate that a TLB may need a couple of cycles to carry out the request, we have a delay
// count decremented on each tick. If the tick is zero, let's do the work.
//
//------------------------------------------------------------------------------------------------------------
bool CpuTlb::insertTlbEntryAdr( uint32_t seg, uint32_t ofs, uint32_t data ) {
    
    TlbEntry *ptr = getTlbEntry( hashAdr( seg, ofs ));
    
    if ( tlbOpState == TO_IDLE ) {
        
        if (( ptr != nullptr ) && ( ! ptr -> tValid( ))) {
            
            tlbMiss++;
            reqOp           = TO_REQ_INSERT_ADR;
            reqData         = data;
            reqTlbEntry     = ptr;
            reqDelayCnt     = tlbDesc.latency;
        }
    }
    else {
        
        if ( reqDelayCnt == 0 ) {
            
            tlbAccess++;
            ptr -> setValid( false );
            ptr -> pInfo        = 0;
            ptr -> aInfo        = data;
            tlbOpState          = TO_IDLE;
        }
    }
    
    return( tlbOpState == TO_IDLE );
}

//------------------------------------------------------------------------------------------------------------
// The insert TLB protection info method is the second part of the TLB insert routine. The first part was
// done by the inseert TLB address instruction. This part will complete the rest of the entries and fill in
// the protection and access rights information. The entry becomes valid. To simulate that a TLB may need a
// couple of cycles to carry out the request, we have a delay count decremented on each tick. If the tick is
// zero, let's do the work.
//
//------------------------------------------------------------------------------------------------------------
bool CpuTlb::insertTlbEntryProt( uint32_t seg, uint32_t ofs, uint32_t data ) {
    
    TlbEntry *ptr = getTlbEntry( hashAdr( seg, ofs ));
    
    if ( tlbOpState == TO_IDLE ) {
        
        if (( ptr != nullptr ) && ( ! ptr -> tValid( ))) {
            
            reqOp           = TO_REQ_INSERT_PROT;
            reqData         = data;
            reqTlbEntry     = ptr;
            reqDelayCnt     = tlbDesc.latency;
        }
    }
    else {
        
        if ( reqDelayCnt == 0 ) {
            
            tlbInserts++;
            ptr -> setValid( true );
            ptr -> pInfo    = data;
            tlbOpState      = TO_IDLE;
        }
    }
    
    return( tlbOpState == TO_IDLE );
}

//------------------------------------------------------------------------------------------------------------
// Purging a TLB entry is implemented by just clearing the valid bit if the entry is found. To simulate that
// a TLB may need a couple of cycles to carry out the request, we have a delay count decremented on each
// tick. If the tick is zero, let's do the work.
//
//------------------------------------------------------------------------------------------------------------
bool CpuTlb::purgeTlbEntry( uint32_t seg, uint32_t ofs ) {
    
    TlbEntry *ptr = getTlbEntry( hashAdr( seg, ofs ));
    
    if ( tlbOpState == TO_IDLE ) {
        
        if (( ptr != nullptr ) && ( ! ptr -> tValid( ))) {
            
            reqOp           = TO_REQ_PURGE;
            reqTlbEntry     = ptr;
            reqDelayCnt     = tlbDesc.latency;
        }
    }
    else {
        
        if ( reqDelayCnt == 0 ) {
            
            tlbDeletes++;
            ptr -> setValid( false );
            tlbOpState = TO_IDLE;
        }
    }
    
    return( tlbOpState == TO_IDLE );
}

//------------------------------------------------------------------------------------------------------------
// "abortTlbOp" will abort any current TLB opration. It is necessary when we flush the pipeline to avoid
// a fetching of an instruction that we never execute.
//
//------------------------------------------------------------------------------------------------------------
void CpuTlb::abortTlbOp( ) {
    
    if ( tlbOpState != TO_IDLE ) {
        
        tlbOpState  = TO_IDLE;
        reqOp       = 0;
        reqData     = 0;
    }
}

//------------------------------------------------------------------------------------------------------------
// "insertTlbEntryData" is the routine called by the command interpreter to insert all the data into a TLB
// entry.
//
//------------------------------------------------------------------------------------------------------------
bool CpuTlb::insertTlbEntryData( uint32_t seg, uint32_t ofs, uint32_t argAcc, uint32_t argAdr ) {
    
    TlbEntry *ptr = getTlbEntry( hashAdr( seg, ofs ));
    if ( ptr != nullptr ) {
        
        ptr -> pInfo    = argAcc;
        ptr -> aInfo    = argAdr;
        ptr -> vpnHigh  = seg;
        ptr -> vpnLow   = ofs & 0xFFFF;
        ptr -> setValid( true );
        return( true );
    }
    else return( false );
}

//------------------------------------------------------------------------------------------------------------
// "purgeTlbEntryData" is the routine called by the command interpreter to remove and enry and clear all
// teh data from the TLB.
//
//------------------------------------------------------------------------------------------------------------
bool CpuTlb::purgeTlbEntryData( uint32_t seg, uint32_t ofs ) {
    
    TlbEntry *ptr = getTlbEntry( hashAdr( seg, ofs ));
    if ( ptr != nullptr ) {
        
        ptr -> pInfo    = 0;
        ptr -> aInfo    = 0;
        ptr -> vpnHigh  = 0;
        ptr -> vpnLow   = 0;
        ptr -> setValid( false );
        return( true );
    }
    else return( false );
}

//------------------------------------------------------------------------------------------------------------
// The search TLB routine hashes into the TLB array and checks if we have a valid and address matching entry.
// We are passed the full virtual address including the page offset.
//
//------------------------------------------------------------------------------------------------------------
TlbEntry *CpuTlb::lookupTlbEntry( uint32_t seg, uint32_t ofs ) {
    
    TlbEntry *ptr = &tlbArray[ hashAdr( seg, ofs ) ];
    
   tlbAccess++;
    
    if (( ptr -> vpnHigh == seg ) && (( ptr -> vpnLow >> PAGE_SIZE_BITS ) == ( ofs >> PAGE_SIZE_BITS ))) {
        
        return( ptr );
    }
    else {
        
        tlbMiss++;
        return( nullptr );
    }
}

//------------------------------------------------------------------------------------------------------------
// "getTlbCtrlReg" and "setTlbCtrlReg" are the getter and setter functions of the TLB object static and
// actual request data. Note that not all "registers" can be modified.
//
//------------------------------------------------------------------------------------------------------------
uint32_t CpuTlb::getTlbCtrlReg( uint8_t tReg ) {
    
  
    return( 0 );
}

void CpuTlb::setTlbCtrlReg( uint8_t mReg, uint32_t val ) {
    
    
}

//------------------------------------------------------------------------------------------------------------
// The get TLB entry method returns a pointer to the TLB emtry by index.
//
//------------------------------------------------------------------------------------------------------------
TlbEntry *CpuTlb::getTlbEntry( uint32_t index ) {
    
    return(( index < tlbDesc.entries ) ? &tlbArray[ index ] : nullptr );
}

//------------------------------------------------------------------------------------------------------------
// A utility method to get the hash value for a virtual address.
//
//------------------------------------------------------------------------------------------------------------
uint16_t CpuTlb::hashAdr( uint32_t seg, uint32_t ofs ) {
    
    return( ::hashTlb( seg, ofs, tlbDesc.entries ));
}

//------------------------------------------------------------------------------------------------------------
// Getters.
//
//------------------------------------------------------------------------------------------------------------
uint16_t CpuTlb::getTlbSize ( ) {
    
    return( tlbDesc.entries );
}

uint32_t CpuTlb::getTlbInserts( ) {
    
  return( tlbInserts );
}

uint32_t CpuTlb::getTlbDeletes( ) {
    
    return( tlbDeletes );
}

uint32_t CpuTlb::getTlbAccess( ) {
    
    return( tlbAccess );
}

uint32_t CpuTlb::getTlbMiss( ) {
    
    return( tlbMiss );
}

uint32_t CpuTlb::getTlbWaitCycles( ) {
    
   return( tlbWaitCycles );
}

//------------------------------------------------------------------------------------------------------------
// Getters/Setters for the TlbEntry.
//
//------------------------------------------------------------------------------------------------------------
bool TlbEntry::tValid( ) {
    
    return( getBit( pInfo, 0 ));
}

void TlbEntry::setValid( bool arg ) {
    
    setBit( &pInfo, 0, arg );
}

bool TlbEntry::tTrapPage( ) {
    
    return( getBitField( pInfo, 1, 1 ));
}

bool TlbEntry::tDirty( ) {
    
    return( getBitField( pInfo, 2, 1 ));
}

bool TlbEntry::tTrapDataPage( ) {
    
    return( getBitField( pInfo, 3, 1 ));
}

uint32_t TlbEntry::tPageType( ) {
    
    return( getBitField( pInfo, 7, 2 ));
}

uint32_t  TlbEntry::tPrivL1( ) {
    
    return( getBitField( pInfo, 8, 1 ));
}

uint32_t  TlbEntry::tPrivL2( ) {
    
    return( getBitField( pInfo, 9, 1 ));
}

uint16_t TlbEntry::tSegId( ) {
    
    return( getBitField( pInfo, 31, 16 ));
}

uint32_t TlbEntry::tPhysPage( ) {
    
    return( getBitField( aInfo, 31, 20 ));
}
