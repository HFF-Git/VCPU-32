//------------------------------------------------------------------------------------------------------------
//
// VCPU32 - A 32-bit CPU - A memory layer for VCPU-32
//
//------------------------------------------------------------------------------------------------------------
// This memory layer class simulates the VCPU32 cache and memory hierarchy. L1 Caches are virtually indexed,
// physically tagged. L2 caches are physically indexed and tagged. Finally memory is just physically indexed.
// All caches and the physical memory are build using this generic class. The CPU simulator implements a
// layered memory model. On top are always the L1 caches on the bottom is always the memory layer. Optionally,
// there can be a L2 cache. A memory layer has two main structures, a tag array and a data array. Caches need
// a cache tag array, physical memory of course does not. In this case the tag array is not allocated. At the
// core of each object is a state machine that handles the request. If the memory layer is not busy, it is
// IDLE and can accept a new request.
//
// The memory access functions are tailored to the particular memory object. In the case of a L1 virtual
// address, this is segment and offset. These two values are used to compute the index into the tag and
// data array of the L1 layer. The tag parameter is the tag obtained from the translation unit. The tag must
// match the tag stored in the memory tag array for the indexed block. The memory layers can have different
// block sizes. However, only going from smaller to larger sizes is supported. For example, a 16 byte block
// L1 cache can map to a 64 byte block L2 cache, but not vice versa.
//
// Memory objects think in byte addresses and bytes. Exchange between the layers is done in blocks. The L1
// caches however export to the CPU in interface to access a word, a half-word and a byte. Also, the data
// in a memory layer is stored in an array of words.
//
// The intended hardware will perform a lookup of TLB and caches in parallel. As a consequence the number of
// bits needed to represent the block entries cannot be greater than the number of bits necessary to
// represent the page size minus the number of bits it takes to represent the block size. For example, of the
// block size is 16 bytes, it will take four bits to index into the block. If the page bit size is 12 bits
// then we have 8 bits left for indexing the cache, i.e. 256 entries.
//
// The key parameters for a memory layer are type, access type, number of entries and block size. The
// total size in bytes is blockEntries * blockSize. All parameters can be found in the memory descriptor.
//
// There is also an arbitration scheme. The two L1 caches my compete for the L2 cache or the physical memory
// layer when that layer is IDLE and both caches have a request. In this case the request with the highest
// priority, i.e. the lowest numeric value, will win. Right now, the PDC and IO memory address space are
// separate and thus there is no arbitration like there would on a real system bus. To look into one day.
//
//------------------------------------------------------------------------------------------------------------
//
// VCPU32 - A 32-bit CPU - A memory layer for VCPU-32
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
#include "VCPU32-Types.h"
#include "VCPU32-Core.h"


//------------------------------------------------------------------------------------------------------------
// ??? what about a victim cache complement?
//
// We add this as an option to an L1 or L2 cache.
//
// Cache Hit: No action
//
// Cache Miss, Victim Hit: The block is in the victim cache and the one in the cache are replaced with each
// other. This new entry in victim cache becomes the most recently used block.
//
// Cache Miss, Victim Miss: The block is brought to cache from next level. The block evicted from the cache
// gets stored in Victim cache.
//------------------------------------------------------------------------------------------------------------



//------------------------------------------------------------------------------------------------------------
// File local declarations. There are constants and routines used internally and not visible outside of this
// file.
//
//------------------------------------------------------------------------------------------------------------
namespace {

//------------------------------------------------------------------------------------------------------------
// Basic constants.
//
//------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------
// The memory object operates using a state machine. From IDLE it will enter a state corresponding to the
// requested operation. There are states for a virtual memory object, i.e. the L1 cache, and states for the
// L2 cache and memory object. So, not all states can be found in the three state machines for L1, L2 and
// MEM object. These states are simply ignored and result in "IDLE".
//
//------------------------------------------------------------------------------------------------------------
enum MemOpState : uint16_t {
    
    MO_IDLE                     = 0,
    MO_READ_WORD                = 1,
    MO_WRITE_WORD               = 2,
    MO_ALLOCATE_BLOCK           = 3,
    MO_READ_BLOCK               = 4,
    MO_WRITE_BLOCK              = 5,
    MO_WRITE_BACK_BLOCK         = 6,
    MO_FLUSH_BLOCK              = 7,
    MO_PURGE_BLOCK              = 8
};

//------------------------------------------------------------------------------------------------------------
// Helper functions. We want to make sure that the size values for blocks and lines are a power of two. The
// other helper functions compute the bit mask and the number of bits in it for the cache lines. Note that for
// block size only 16, 32 and 64 bytes is allowed.
//
//------------------------------------------------------------------------------------------------------------
uint32_t roundUp( uint32_t size, uint32_t limit ) {
    
    uint32_t power = 1;
    while(( power < size ) && ( power < limit )) power *= 2;
    
    return ( power );
}

uint16_t getBlockBits( uint16_t blockSize ) {
    
    if      ( blockSize == 16 ) return( 4 );
    else if ( blockSize == 32 ) return( 5 );
    else if ( blockSize == 64 ) return( 6 );
    else return ( 4 );
}

uint32_t getBlockBitMask( uint16_t blockSize ) {
    
    if      ( blockSize == 16 ) return( 0x0F );
    else if ( blockSize == 32 ) return( 0x1F );
    else if ( blockSize == 64 ) return( 0x3F );
    else                        return( 0x0F );
}

uint32_t maxBlocks( CpuMemType memType, uint32_t blockSize ) {
    
    switch ( memType ) {
            
        case MEM_T_L1_DATA:      return( MAX_CACHE_BLOCK_ENTRIES );
        case MEM_T_L1_INSTR:     return( MAX_CACHE_BLOCK_ENTRIES );
        case MEM_T_L2_UNIFIED:   return( MAX_CACHE_BLOCK_ENTRIES );
        case MEM_T_PHYS_MEM:     return( MAX_PHYS_MEM_SIZE / blockSize );
        case MEM_T_PDC_MEM:      return( MAX_PDC_MEM_SIZE / blockSize );
        case MEM_T_IO_MEM:       return( MAX_IO_MEM_SIZE / blockSize );
            
        default: return( 1 );
    }
}

}; // namespace


//------------------------------------------------------------------------------------------------------------
// Memory object constructor. We construct the object from the CPU descriptor portion for the memory object.
// A memory is a set of data organized in blocks of a block size. A memory can also have a tag array for a
// tag match operation of the selected block. Besides the the configuration descriptor, we are passed an
// optional handle to a lower memory layer. Note that the memory object is an abstract class used by a
// particular memory object. Allocating space for data and tag memory must be handled by the inheriting
// class.
//
//------------------------------------------------------------------------------------------------------------
CpuMem::CpuMem( CpuMemDesc *cfg, CpuMem *mem ) {
    
    memcpy( &cDesc, cfg, sizeof( CpuMemDesc ));
    
    cDesc.blockSize     = roundUp( cDesc.blockSize, MAX_BLOCK_SIZE );
    cDesc.blockSets     = roundUp( cDesc.blockSets, MAX_BLOCK_SETS );
    cDesc.blockEntries  = roundUp( cDesc.blockEntries, maxBlocks( cDesc.type, cDesc.blockSize ));
    cDesc.endAdr        = cDesc.startAdr + cDesc.blockEntries * cDesc.blockSize - 1;
    blockBits           = getBlockBits( cDesc.blockSize );
    blockBitMask        = getBlockBitMask( cDesc.blockSize );
    memObjPriority      = cDesc.priority;
    opState             = MO_IDLE;
    lowerMem            = mem;
}

//------------------------------------------------------------------------------------------------------------
// Reset the memory object. We clear the data structures and set the request state machine to idle.
//
//------------------------------------------------------------------------------------------------------------
void CpuMem::reset( ) {
  
    for ( uint32_t i = 0; i < cDesc.blockSets; i++ ) {
        
        if ( tagArray[ i ] != nullptr ) {
            
            for ( uint32_t j = 0; j < cDesc.blockEntries; j++ ) {
                
                tagArray[ i ] [ j ].valid = false;
                tagArray[ i ] [ j ].dirty = false;
                tagArray[ i ] [ j ].tag   = 0;
            }
        }
        
        if ( dataArray[ i ] != nullptr ) {
            
            uint32_t tmpSize = cDesc.blockEntries * cDesc.blockSize;
            for ( uint32_t j = 0; j < tmpSize; j++ ) dataArray[ i ] [ j ] = 0;
        }
    }
    
    opState.load( MO_IDLE );
    reqSeg      = 0;
    reqOfs      = 0;
    reqPri      = 0;
    reqTag      = 0;
    reqLen      = 0;
    reqPtr      = nullptr;
    reqLatency  = cDesc.latency;
   
    clearStats( );
}

//------------------------------------------------------------------------------------------------------------
// Reset the statistics. We maintain counters for total access, misses and how many cycles we waited for a
// lower layer to read/ write some data.
//
//------------------------------------------------------------------------------------------------------------
void CpuMem::clearStats( ) {
    
    accessCnt     = 0;
    missCnt       = 0;
    dirtyMissCnt  = 0;
    waitCyclesCnt = 0;
}

//------------------------------------------------------------------------------------------------------------
// The "tick" routine is invoked on every CPU clock cycle. All we do is to update any register defined. So
// far this is only the opState for the state machine. The "process" method invokes the state machine. It is
// a bit awkward to read, we invoke a procedure label stored in the object. Both routines are used in the
// clock step code of the CPU core object. First all "ticks" are handled, then all "process" code is invoked.
//
//------------------------------------------------------------------------------------------------------------
void CpuMem::tick( ) {
    
    opState.tick( );
}

//------------------------------------------------------------------------------------------------------------
// "abortMemOp" will abort any current operation. It is necessary when we flush the pipeline to avoid fetching
// data that we do not need.
//
//------------------------------------------------------------------------------------------------------------
void CpuMem::abortOp( ) {
    
    if ( opState.get( ) != MO_IDLE ) {
        
        opState.set( MO_IDLE );
        reqSeg  = 0;
        reqOfs  = 0;
        reqPri  = 0;
        reqTag  = 0;
        reqLen  = 0;
        reqPtr  = nullptr;
    }
}

//------------------------------------------------------------------------------------------------------------
// N-way-associative memories use the "matchTag" function to check for a matching tag in the set. We will
// iterate through the tag arrays at the block index, check for a valid entry and matching tag. A tag is the
// physical address with the blockBits set to zero. If no matching entry is found and the entry is valid, the
// maximum block entries constant is returned. The tag passed is physical address. We mask off the block size
// bits and compare the rest.
//
//------------------------------------------------------------------------------------------------------------
uint16_t CpuMem::matchTag( uint32_t index, uint32_t tag ) {
    
    for ( uint32_t i = 0; i < cDesc.blockSets; i++ ) {
        
        MemTagEntry *ptr = &tagArray[ i ] [ index ];
        if (( ptr -> valid ) && (( tag & ( ~ blockBitMask )) == ( ptr -> tag & ( ~ blockBitMask )))) {
            
            return( i );
        }
    }
    
    return ( MAX_BLOCK_SETS );
}

//------------------------------------------------------------------------------------------------------------
// "readWord" fills in the request data for reading a word, a half-word or a byte from the data array. The
// method supports the latency option, so that we can model the latency behavior of a physical memory
// ´request. If the memory object is IDLE, we fill in the request parameters and the next cycle will start
// processing the request. Note that this method will be called every clock cycle as long as the lower layer
// operation is not completed. The completion is signaled by the latency count being zero. Note also that the
// "IDLE" state will be set with the next clock cycle, hence we need the latency count to know that we are
// done with the current request.
//
//------------------------------------------------------------------------------------------------------------
bool CpuMem::readWord( uint32_t seg, uint32_t ofs, uint32_t tag, uint32_t len, uint32_t *word, uint32_t pri ) {
   
    if (( opState.get( ) == MO_IDLE ) && ( pri > reqPri )) {
        
        opState.set( MO_READ_WORD );
        reqSeg      = seg;
        reqOfs      = ofs;
        reqTag      = tag;
        reqPtr      = (uint8_t *) word;
        reqLen      = len;
        reqPri      = (( pri == 0 ) ? cDesc.priority : pri );
        reqLatency  = cDesc.latency;
        return( false );
    }
    else return ( reqLatency == 0 );
}

//------------------------------------------------------------------------------------------------------------
// "writeWord" fills in the request data for writing a word, a half-word or a byte into the data array. The
// method supports the latency option, so that we can model the latency behavior of a physical memory
// ´request. If the memory object is IDLE, we fill in the request parameters and the next cycle will start
// processing the request. Note that this method will be called every clock cycle as long as the lower layer
// operation is not completed. The completion is signaled by the latency count being zero. Note also that the
// "IDLE" state will be set with the next clock cycle, hence we need the latency count to know that we are
// done with the current request.
//
//------------------------------------------------------------------------------------------------------------
bool CpuMem::writeWord( uint32_t seg, uint32_t ofs, uint32_t tag, uint32_t len, uint32_t word, uint32_t pri ) {
    
    if (( opState.get( ) == MO_IDLE ) && ( pri > reqPri )) {
        
        opState.set( MO_WRITE_BLOCK );
        reqSeg      = seg;
        reqOfs      = ofs;
        reqTag      = tag;
        reqPtr      = (uint8_t *) &word;
        reqLen      = len;
        reqPri      = (( pri == 0 ) ? cDesc.priority : pri );
        reqLatency  = cDesc.latency;
        
        return( false );
    }
    else return ( reqLatency == 0 );
}

//------------------------------------------------------------------------------------------------------------
// "readBlock" is called by an upper memory layer to read a block of data from a lower layer. This method will
// fill in a request for the memory layer state machine. The block sizes of the upper and lower layer do not
// necessarily have to match. However, the upper layer must always be configured smaller than the lower layer.
// If the memory object is IDLE, we fill in the request parameters and the next cycle will start processing
// the request. Note that this method will be called every clock cycle as long as the lower layer operation
// is not completed. The completion is signaled by the latency count being zero. Note also that the "IDLE"
// state will be set with the next clock cycle, hence we need the latency count to know that we are done with
// the current request.
//
//------------------------------------------------------------------------------------------------------------
bool CpuMem::readBlock( uint32_t seg, uint32_t ofs, uint32_t tag, uint8_t *buf, uint32_t len, uint32_t pri ) {
    
    if ( opState.get( ) == MO_IDLE ) {
        
        opState.set( MO_READ_BLOCK );
        reqSeg      = seg;
        reqOfs      = ofs;
        reqTag      = tag;
        reqPtr      = buf;
        reqLen      = len;
        reqPri      = (( pri == 0 ) ? cDesc.priority : pri );
        reqLatency  = cDesc.latency;
        return( false );
    }
    else return ( reqLatency == 0 );
}

//------------------------------------------------------------------------------------------------------------
// "writeBlock" will transfer multiple bytes to the lower layer. The block sizes of the upper and lower layer
// do not necessarily have to match. However, the upper layer must always be configured smaller than the lower
// layer. If the memory object is IDLE, we fill in the request parameters and the next cycle will start
// processing the request. Note that this method will be called every clock cycle as long as the lower layer 
// operation is not completed. The completion is signaled by the latency count being zero. Note also that the
// "IDLE" state will be set with the next clock cycle, hence we need the latency count to know that we are
// done with the current request.
//
//------------------------------------------------------------------------------------------------------------
bool CpuMem::writeBlock( uint32_t seg, uint32_t ofs, uint32_t tag, uint8_t *buf, uint32_t len, uint32_t pri ) {
    
    if ( opState.get( ) == MO_IDLE ) {
        
        opState.set( MO_WRITE_BLOCK );
        reqSeg      = seg;
        reqOfs      = ofs;
        reqTag      = tag;
        reqPtr      = buf;
        reqLen      = len;
        reqPri      = (( pri == 0 ) ? cDesc.priority : pri );
        reqLatency  = cDesc.latency;
        return( false );
    }
    else return ( reqLatency == 0 );
}

//------------------------------------------------------------------------------------------------------------
// "flushBlockPhys" will write the content of the block at address "adr" to the lower layer. This method only
// applies to caches that are connected to a physical memory layer. The block is marked "clean" after the
// operation. If the operation is not supported by the lower layer, the state machine will simply ignore
// this request.
//
//------------------------------------------------------------------------------------------------------------
bool CpuMem::flushBlock( uint32_t seg, uint32_t ofs, uint32_t tag, uint32_t pri ) {
    
    if ( opState.get( ) == MO_IDLE ) {
        
        opState.set( MO_WRITE_BLOCK );
        reqSeg      = seg;
        reqOfs      = ofs;
        reqTag      = tag;
        reqPtr      = nullptr;
        reqLen      = 0;
        reqPri      = (( pri == 0 ) ? cDesc.priority : pri );
        reqLatency  = cDesc.latency;
        return( false );
    }
    else return ( reqLatency == 0 );
}

//------------------------------------------------------------------------------------------------------------
// "purgeBlock" will invalidate the block at physical address "adr" to the lower layer. This method only
// applies to caches that are connected to a physical memory layer. The block is marked "clean" after the
// operation. If the operation is not supported by the lower layer, the state machine will simply ignore
// this request.
//
//------------------------------------------------------------------------------------------------------------
bool CpuMem::purgeBlock( uint32_t seg, uint32_t ofs, uint32_t tag, uint32_t pri ) {
    
    if ( opState.get( ) == MO_IDLE ) {
        
        opState.set( MO_PURGE_BLOCK );
        reqSeg      = seg;
        reqOfs      = ofs;
        reqTag      = tag;
        reqPtr      = nullptr;
        reqLen      = 0;
        reqPri      = (( pri == 0 ) ? cDesc.priority : pri );
        reqLatency  = cDesc.latency;
        return( false );
    }
    else return ( reqLatency == 0 );
}

//------------------------------------------------------------------------------------------------------------
// "getMemCtrlReg" and "setMemCtrlReg" are the getter and setter functions of the memory object static and
// actual request data. Note that not all "registers" can be modified.
//
//------------------------------------------------------------------------------------------------------------
uint32_t CpuMem::getMemCtrlReg( uint8_t mReg ) {
    
    switch( mReg ) {
            
        case MC_REG_STATE:              return( opState.get( ));
        case MC_REG_REQ_SEG:            return( reqSeg );
        case MC_REG_REQ_OFS:            return( reqOfs );
        case MC_REG_REQ_PRI:            return( reqPri );
        case MC_REG_REQ_TAG:            return( reqTag );
        case MC_REG_REQ_LEN:            return( reqLen );
        case MC_REG_REQ_LATENCY:        return( reqLatency );
            
        case MC_REG_REQ_BLOCK_INDEX:    return( reqTargetBlockIndex );
        case MC_REG_REQ_BLOCK_SET:      return( reqTargetSet );
            
        case MC_REG_START_ADR:          return( cDesc.startAdr );
        case MC_REG_END_ADR:            return( cDesc.endAdr );
        case MC_REG_BLOCK_ENTRIES:      return( cDesc.blockEntries );
        case MC_REG_BLOCK_SIZE:         return( cDesc.blockSize );
        case MC_REG_SETS:               return( cDesc.blockSets );
        case MC_REG_LATENCY:            return( cDesc.latency );
            
        default: return( 0 );
    }
}

void CpuMem::setMemCtrlReg( uint8_t mReg, uint32_t val ) {
    
    switch( mReg ) {
            
        case MC_REG_REQ_SEG:    reqSeg = val;       break;
        case MC_REG_REQ_OFS:    reqOfs = val;       break;
        case MC_REG_REQ_TAG:    reqTag = val;       break;
        case MC_REG_REQ_LEN:    reqLen = val;       break;
        case MC_REG_LATENCY:    reqLatency = val;   break;
        default: ;
    }
}

char *CpuMem::getMemOpStr( uint32_t opArg ) {
    
    switch ( opArg  ) {
            
        case MO_IDLE:                   return((char *) "IDLE" );
        case MO_READ_WORD:              return((char *) "READ WORD" );
        case MO_WRITE_WORD:             return((char *) "WRITE WORD" );
        case MO_ALLOCATE_BLOCK:         return((char *) "ALLOCATE BLOCK" );
        case MO_READ_BLOCK:             return((char *) "READ BLOCK" );
        case MO_WRITE_BLOCK:            return((char *) "WRITE BLOCK" );
        case MO_WRITE_BACK_BLOCK:       return((char *) "WRITE BACK BLOCK" );
        case MO_FLUSH_BLOCK:            return((char *) "FLUSH BLOCK" );
        case MO_PURGE_BLOCK:            return((char *) "PURGE  BLOCK" );
            
        default:                        return((char *) "****" );
    }
}

//------------------------------------------------------------------------------------------------------------
// "getMemTagEntry" is a routine called by the simulator driver to obtain a reference to the tag data entry.
// If the tag array does not exists or the indexed is out of range, a nullptr will be returned.
//
//------------------------------------------------------------------------------------------------------------
MemTagEntry  *CpuMem::getMemTagEntry( uint32_t index, uint8_t set ) {
    
    if ( index >= cDesc.blockEntries )  return( nullptr );
    if ( set >= cDesc.blockSets )       return( nullptr );
    
    return( &tagArray[ set ] [ index ] );
}

//------------------------------------------------------------------------------------------------------------
// "getMemBlockEntry" is a routine called by the simulator driver to obtain a reference to the indexed in the
// data array block.
//
//------------------------------------------------------------------------------------------------------------
uint8_t *CpuMem::getMemBlockEntry( uint32_t index, uint8_t set ) {
    
    if ( index >= cDesc.blockEntries )  return( nullptr );
    if ( set >= cDesc.blockSets )       return( nullptr );
    
    return( &dataArray[ set ] [ index * cDesc.blockSize ] );
}

//------------------------------------------------------------------------------------------------------------
// "getMemDataWord" and "putMemWord" are the routines called by the simulator display functions. They get or
// set a word in the data array of the data set. The offset is rounded down to a 4-byte boundary.
//
//------------------------------------------------------------------------------------------------------------
uint32_t CpuMem::getMemDataWord( uint32_t ofs, uint8_t set ) {
    
    if ( ofs >= cDesc.startAdr + cDesc.blockEntries * cDesc.blockSize ) return( 0 );
    if ( set >= cDesc.blockSets ) return( 0 );
    
    ofs &= 0xFFFFFFFC;
    
    uint32_t tmp = 0;
    memcpy( &tmp, &dataArray[ set ] [ ofs - cDesc.startAdr ], sizeof( uint32_t ));
    
    return( tmp );
}

void CpuMem::putMemDataWord( uint32_t ofs, uint32_t val, uint8_t set ) {
    
    if ( ofs >= cDesc.startAdr + cDesc.blockEntries * cDesc.blockSize ) return;
    if ( set >= cDesc.blockSets ) return;
    
    ofs &= 0xFFFFFFFC;
    
    uint32_t tmp = val;
    memcpy( &dataArray[ set ] [ ofs - cDesc.startAdr ], &tmp, sizeof( uint32_t ));
}

//------------------------------------------------------------------------------------------------------------
// Simple Getters.
//
//------------------------------------------------------------------------------------------------------------
uint32_t CpuMem::getMemSize( ) {
    
    return( cDesc.blockEntries * cDesc.blockSize );
}

uint32_t CpuMem::getStartAdr( ) {
    
    return( cDesc.startAdr );
}

uint32_t CpuMem::getEndAdr( ) {
    
    return( cDesc.endAdr );
}

uint32_t CpuMem::getBlockEntries( ) {
    
    return( cDesc.blockEntries );
}

uint16_t CpuMem::getBlockSize( ) {
    
    return( cDesc.blockSize );
}

uint16_t CpuMem::getBlockSets( ) {
    
    return( cDesc.blockSets );
}

uint32_t CpuMem::getMissCnt( ) {
    
    return( missCnt );
}

uint32_t CpuMem::getDirtyMissCnt( ) {
    
    return( dirtyMissCnt );
}

uint32_t CpuMem::getAccessCnt( )  {
    
    return( accessCnt );
}

uint32_t CpuMem::getWaitCycleCnt( )  {
    
    return( waitCyclesCnt );
}

bool CpuMem::validAdr( uint32_t ofs ) {
    
    return(( ofs >= cDesc.startAdr ) && ( ofs <= cDesc.endAdr ));
}


//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------
//
// L1 cache object methods.
//
//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------------------------
// The "L1CacheMem" represents the L1 caches instruction and data. A cache has a data and a tag array which
// we allocate right here.
//
//------------------------------------------------------------------------------------------------------------
L1CacheMem::L1CacheMem( CpuMemDesc *mDesc, CpuMem *lowerMem ) : CpuMem( mDesc, lowerMem ) {
    
    for ( uint32_t i = 0; i < cDesc.blockSets; i++ )
        tagArray[ i ] = (MemTagEntry *) calloc( cDesc.blockEntries, sizeof( MemTagEntry));
    
    for ( uint32_t i = 0; i < cDesc.blockSets; i++ )
        dataArray[ i ] = (uint8_t *) calloc( cDesc.blockEntries, cDesc.blockSize );
    
    reset( );
}

//------------------------------------------------------------------------------------------------------------
// "readWord" overrides the memory object methods. The L1 caches are called from the pipeline stages. The
// virtual address is "seg.ofs". The "len" parameter specifies the number of bytes to read. Only 1, 2 and 4 
// bytes are allowed, which correspond to byte, half-word and word. The "adrTag" parameter is the physical
// address used for tag comparison which we got from the TLB. The tag address is the full address, the memory
// object will mask out the block bit masks. If the L1 cache is IDLE, we directly check to see if we have a
// valid block containing the data. If so, the data is returned right away and we have no cycle penalty.
// Depending on the requested data size, the byte or half-word is returned with leading zeros extended.
// Otherwise, we first need to ALLOCATE a slot in the cache and read in the block. The next cycle will start
// processing the request. Note that the CPU core layer will call this routine every clock cycle as long as
// the operation is not completed, i.e. it is back to IDLE.
//
//------------------------------------------------------------------------------------------------------------
bool L1CacheMem::readWord( uint32_t seg, uint32_t ofs, uint32_t adrTag, uint32_t len, uint32_t *word, uint32_t pri ) {
    
    if ( opState.get( ) == MO_IDLE ) {
       
        uint32_t    blockIndex  = ofs / cDesc.blockSize;
        uint16_t    matchSet    = matchTag( blockIndex, adrTag );
        
        if ( matchSet < cDesc.blockSets ) {
            
            uint8_t  *blockPtr = &dataArray[ matchSet ] [ blockIndex * cDesc.blockSize ];
            uint8_t *dataPtr  = &blockPtr[ ofs & blockBitMask ];
           
            if      ( len == 1 ) *word = *((uint8_t *)  dataPtr );
            else if ( len == 2 ) *word = *((uint16_t *) dataPtr );
            else                 *word = *((uint32_t *) dataPtr );
            
            return( true );
        }
        else {
            
            opState.set( MO_ALLOCATE_BLOCK );
            reqSeg              = seg;
            reqOfs              = ofs;
            reqTag              = adrTag;
            reqPtr              = nullptr;
            reqLen              = 0;
            reqPri              = (( pri == 0 ) ? cDesc.priority : pri );
            reqLatency          = cDesc.latency;
            
            reqTargetSet        = matchSet;
            reqTargetBlockIndex = blockIndex;
            return( false );
        }
    }
    else return(false );
}

//------------------------------------------------------------------------------------------------------------
// "writeWord" overrides the base class method. It is called from the CPU pipeline data access stage to write
// data to the L1 cache. The virtual address is "seg.ofs". The "adrTag" parameter is the physical address tag
// stored in the TLB. It is the full physical address, the memory object will mask out the bit mask size with
// zeroes. If the L1 cache is IDLE, we directly check to see if we have a valid block containing the word. If
// so, the data is stored right away and we have no cycle penalty. Depending on the request data size, the
// byte or half-word is stored at the byte address in the cache. Otherwise, we follow the same logic as
// described for the read virtual data operation.
//
//------------------------------------------------------------------------------------------------------------
bool L1CacheMem::writeWord( uint32_t seg, uint32_t ofs, uint32_t adrTag, uint32_t len, uint32_t word, uint32_t pri ) {
    
    if ( opState.get( ) == MO_IDLE ) {
        
        uint32_t    blockIndex  = ofs / cDesc.blockSize;
        uint16_t    matchSet    = matchTag( blockIndex, adrTag );
       
        if ( matchSet < cDesc.blockSets ) {
            
            uint8_t *blockPtr = &dataArray[ matchSet ] [ blockIndex * cDesc.blockSize ];
            uint8_t *dataPtr  = &blockPtr[ ofs & blockBitMask ];
            
            if      ( reqLen == 1 ) *dataPtr                 = (uint8_t) word;
            else if ( reqLen == 2 ) *((uint16_t *) dataPtr ) = (uint16_t) word;
            else                    *((uint32_t *) dataPtr ) = word;
            
            return( true );
        }
        else {
            
            opState.set( MO_ALLOCATE_BLOCK );
            reqSeg              = seg;
            reqOfs              = ofs;
            reqTag              = adrTag;
            reqPtr              = nullptr;
            reqLen              = 0;
            reqPri              = (( pri == 0 ) ? cDesc.priority : pri );
            reqLatency          = cDesc.latency;
            
            reqTargetSet        = matchSet;
            reqTargetBlockIndex = blockIndex;
            return( false );
        }
    }
    else return( false );
}

//------------------------------------------------------------------------------------------------------------
// "flushBlock" overrides the base class method. It is the method for writing a dirty block back to the lower
// layer. If there is a match and the block is dirty it will be written back to the lower layer. The next 
// state will be FLUSH_BLOCK_VIRT. Otherwise the request is ignored.
//
//------------------------------------------------------------------------------------------------------------
bool L1CacheMem::flushBlock( uint32_t seg, uint32_t ofs, uint32_t adrTag, uint32_t pri ) {
    
    if ( opState.get( ) == MO_IDLE ) {
        
        uint32_t    blockIndex  = ofs % cDesc.blockSize;
        uint16_t    matchSet    = matchTag( blockIndex, adrTag );
        
        if ( matchSet < cDesc.blockSets ) {
            
            MemTagEntry *tagPtr = &tagArray[ matchSet ] [ blockIndex ];
            
            if ( tagPtr -> dirty ) {
                
                opState.set( MO_FLUSH_BLOCK );
                reqSeg              = seg;
                reqOfs              = ofs;
                reqTag              = tagPtr -> tag;
                reqPtr              = nullptr;
                reqLen              = 0;
                reqPri              = (( pri == 0 ) ? cDesc.priority : pri );
                reqLatency          = cDesc.latency;
                
                reqTargetSet        = matchSet;
                reqTargetBlockIndex = blockIndex;
            }
        }
    }
    
    return( opState.get( ) == MO_IDLE );
}

//------------------------------------------------------------------------------------------------------------
// "purgeBlock" overrides the base class method. It is the method for invalidating the block in the current
// slot. If there is a match, the entry will just be set to invalid, otherwise the request is ignored.
//
//------------------------------------------------------------------------------------------------------------
bool L1CacheMem::purgeBlock( uint32_t seg, uint32_t ofs, uint32_t adrTag, uint32_t pri ) {
    
    if ( opState.get( ) == MO_IDLE ) {
        
        uint32_t    blockIndex  = ofs % cDesc.blockSize;
        uint16_t    matchSet    = ( matchTag( blockIndex, adrTag ) < cDesc.blockSets );
        
        if ( matchSet < cDesc.blockSets ) {
            
            opState.set( MO_IDLE );
            reqSeg              = 0;
            reqOfs              = 0;
            reqTag              = 0;
            reqPtr              = nullptr;
            reqLen              = 0;
            reqPri              = (( pri == 0 ) ? cDesc.priority : pri );
            reqLatency          = cDesc.latency;
            
            reqTargetSet        = matchSet;
            reqTargetBlockIndex = blockIndex;
        }
    }
    
    return( opState.get( ) == MO_IDLE );
}

//------------------------------------------------------------------------------------------------------------
// "processL1CacheRequest" is the state machine for the L1 cache family. An L1 cache directly serves the
// CPU core. The unit of data transfer is 1, 2 or 4 bytes. A cache hit will have no cycle penalty, a miss will
// result in the data being fetched from the lower layer, which depends on the configured CPU to have a L2
// cache or the physical memory as lower layer. Since a read or write hit is directly handled by the read and
// write methods, the state machine is only invoked when we deal with a miss, a flush or purge operation. The
// state machine has several states:
//
// MO_ALLOCATE_BLOCK: on a cache miss, we start here. The first task is to locate the block to use for the
// cache miss. If there is an invalid block in the sets, this is the one to use and the next state is
// MO_READ_BLOCK, where we will read the block that contains the requested data. Otherwise, we will
// randomly select a block from the sets to be the candidate for serving the cache miss. If the selected
// block is dirty it will be written back first and the next state MO_WRITE_BACK_BLOCK.
//
// MO_READ_BLOCK: coming from the MO_ALLOCATE_BLOCK state, this state will read the block from the lower
// layer. The target block has already been identified and we will stay in this state until the lower
// memory request is served. The next state will then just be MO_IDLE, so that the next CPU request will hit
// the valid block and now serve the original request.
//
// MO_WRITE_BACK_BLOCK: coming from the MO_READ_BLOCK_VIRT, the task is to write back a dirty block. The
// target block has already been identified and we will stay in this state until the lower memory request is
// served. Once the block is written the next state is MO_ALLOCATE_BLOCK_VIRT to get the originally requested
// block.
//
// MO_FLUSH_BLOCK: this is state entered when we need to explicitly flush a block. After operation, the
// next state is MO_IDLE.
//
// The cache block to read, write back or flush was stored in the two request block fields "reqTargetSet"
// and "reqTargetBlockIndex" by the cache access methods. The block index is computed from the request offset
// value. The target set is determined through finding a replacement candidate. The values are set during
// the ALLOCATE state and passed ro the follow up state. The two fields are also set for flushing a block,
// however in this case the target set is a valid set.
//
//------------------------------------------------------------------------------------------------------------
void L1CacheMem::process( ) {
    
    switch( opState.get( )) {
            
        case MO_ALLOCATE_BLOCK: {
           
            for ( int i = 0; i < cDesc.blockSets; i++ ) {
                
                MemTagEntry *tagPtr = &tagArray[ i ] [ reqTargetBlockIndex ];
                if ( ! tagPtr -> valid ) {
                    
                    reqTargetSet = i;
                    break;
                }
            }
            
            if ( reqTargetSet >= cDesc.blockSets ) reqTargetSet = rand( ) % cDesc.blockSets;
            
            MemTagEntry *tagPtr = &tagArray[ reqTargetSet ] [ reqTargetBlockIndex ];
            
            if (( tagPtr -> valid ) && ( tagPtr -> dirty )) opState.set( MO_WRITE_BACK_BLOCK );
            else                                            opState.set( MO_READ_BLOCK );
            
        } break;
            
        case MO_READ_BLOCK: {
            
            MemTagEntry *tagPtr    = &tagArray[ reqTargetSet ] [ reqTargetBlockIndex ];
            uint8_t     *blockPtr  = &dataArray[ reqTargetSet ] [ reqTargetBlockIndex * cDesc.blockSize ];
         
            if ( lowerMem -> readBlock( 0, reqTag & ( ~ blockBitMask ), 0, blockPtr, cDesc.blockSize, reqPri )) {
                
                tagPtr -> valid = true;
                tagPtr -> dirty = false;
                tagPtr -> tag   = reqOfs & ( ~ blockBitMask );
                opState.set( MO_IDLE );
            }
            else waitCyclesCnt ++;
            
        } break;
            
        case MO_WRITE_BACK_BLOCK: {
            
            MemTagEntry *tagPtr    = &tagArray[ reqTargetSet ] [ reqTargetBlockIndex ];
            uint8_t     *blockPtr  = &dataArray[ reqTargetSet ] [ reqTargetBlockIndex * cDesc.blockSize ];
            
            if ( lowerMem -> writeBlock( 0, reqTag & ( ~ blockBitMask ), 0, blockPtr, cDesc.blockSize, reqPri )) {
                
                tagPtr -> valid = false;
                tagPtr -> dirty = false;
                opState.set( MO_ALLOCATE_BLOCK );
            }
            else waitCyclesCnt ++;
            
        } break;
            
        case MO_FLUSH_BLOCK: {
            
            if ( reqTargetSet < cDesc.blockSets ) {
                
                MemTagEntry *tagPtr    = &tagArray[ reqTargetSet ] [ reqTargetBlockIndex ];
                uint8_t     *blockPtr  = &dataArray[ reqTargetSet ] [ reqTargetBlockIndex * cDesc.blockSize ];
                
                if ( lowerMem -> writeBlock( 0, reqTag & ( ~ blockBitMask ), 0, blockPtr, cDesc.blockSize, reqPri )) {
                    
                    tagPtr -> valid = false;
                    tagPtr -> dirty = false;
                    opState.set( MO_IDLE );
                }
                else waitCyclesCnt ++;
                
            } else opState.set( MO_IDLE );
            
        } break;
    }
}


//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------
//
// L2 cache object methods.
//
//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------------------------
// The "L2CacheMem" represents the L2 cache. It has a data and a tag array, which we allocate right here. The
// cache is physically indexed and physically tagged.
//
//------------------------------------------------------------------------------------------------------------
L2CacheMem::L2CacheMem( CpuMemDesc *mDesc, CpuMem *lowerMem ) : CpuMem( mDesc, lowerMem ) {
    
    for ( uint32_t i = 0; i < cDesc.blockSets; i++ )
            tagArray[ i ] = (MemTagEntry *) calloc( cDesc.blockEntries, sizeof( MemTagEntry));
    
    for ( uint32_t i = 0; i < cDesc.blockSets; i++ )
        dataArray[ i ] = (uint8_t *) calloc( cDesc.blockEntries, cDesc.blockSize );
   
    reset( );
}

//------------------------------------------------------------------------------------------------------------
// "processL2CacheRequest" is the state machine for the L2 cache. The L2 cache is a thing in the middle
// between the L1 cache and the physical memory. It is a physically indexed, physically tagged cache and
// serves both L1 caches with the L1 instruction cache having priority over the L1 data cache. The "seg"
// parameter in the access methods is zero, and so is the "tag". The ofs" parameter contains the byte address
// which is both the address and the tag for comparison.
//
// The block sizes of the upper and lower layer do not necessarily have to match. For example, we could have
// a 32 byte block L2 cache and a 16 byte block L1 cache. However, the upper layer must always be configured
// smaller than the lower layer.
//
// There are some open questions:
//
// 1.) Is this cache strictly inclusive ? If so, an entry in L2 must exist before it is moved to L1.
// 2.) Invalidating L2 in a strictly inclusive model also needs to invalidate L1.
// 3.) Coherence protocol for more than CPU core ?
//
// ??? under construction ..... first get L1 and MEM stable...
//------------------------------------------------------------------------------------------------------------
void L2CacheMem::process( ) {
    
    uint32_t        blockIndex  = reqOfs % cDesc.blockSize;
    uint16_t        blockSet    = matchTag( blockIndex, 0 ); // ??? FIX ...
    MemTagEntry     *tagPtr     = nullptr;
    uint8_t         *blockPtr   = nullptr;
    bool            tagMatch    = blockSet < MAX_BLOCK_SETS;
    
    if ( tagMatch ) {
        
        tagPtr      =   &tagArray[ blockSet ] [ blockIndex ];
        blockPtr    =   &dataArray[ blockSet ] [ blockIndex * cDesc.blockSize ];
    }
    else {
        
        // we have a miss...
        // select a random set for use...
    }
    
    switch( opState.get( )) {
            
        case MO_READ_BLOCK: {
            
            if ( reqLatency == 0 ) {
                
            }
            else reqLatency--;
            
        } break;
            
        case MO_WRITE_BLOCK: {
            
            if ( reqLatency == 0 ) {
                
            }
            else reqLatency--;
            
        } break;
            
        case MO_FLUSH_BLOCK: {
            
            if ( reqLatency == 0 ) {
                
            }
            else reqLatency--;
            
        } break;
            
        case MO_PURGE_BLOCK: {
            
            if ( reqLatency == 0 ) {
                
            }
            else reqLatency--;
            
        } break;
            
        case MO_ALLOCATE_BLOCK: {
            
            if ( reqLatency == 0 ) {
                
            }
            else reqLatency--;
            
        } break;
            
        case MO_WRITE_BACK_BLOCK: {
            
            if ( reqLatency == 0 ) {
                
            }
            else reqLatency--;
            
        } break;
    }
}


//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------
//
// Physical memory methods.
//
//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------------------------
// The "PhysMem" represents the main memory. There is exactly one data array and no tags.
//
//------------------------------------------------------------------------------------------------------------
PhysMem::PhysMem( CpuMemDesc *mDesc  ) : CpuMem( mDesc, nullptr ) {
    
    for ( uint32_t i = 0; i < cDesc.blockSets; i++ )
        dataArray[ i ] = (uint8_t *) calloc( cDesc.blockEntries, cDesc.blockSize );
   
    reset( );
}

//------------------------------------------------------------------------------------------------------------
// "processP" is the state machine for our main memory object, which is the final layer in our memory
// hierarchy. It does not support any kind of tags, multiple sets, flushing and purging of blocks. The only
// indexing method is the direct indexing method. Upon an incoming request and an IDLE state, the request
// data stored by the external interface methods and the state machine starts working on the request. The
// state machine has only two states, MO_READ_BLOCK_PHYS and MO_WRITE_BLOCK_PHYS. All else is ignored.
//
// It will use the latency counter to simulate the cycles it takes to serve the request. Each time the CPU
// clock ( "tick" ) advances, the latency counter is decremented. When zero is reached the requested operation
// is executed and the state machine advances to the next state.
//
// In general, we will return the data when the latency counter decrements to zero. The indexing method
// gives us the block in memory in the dataArray[ 0 ] set. For a data read or write the blockBits portion of
// the request offset is the index into the selected block. The requestor block size can be smaller up to
// equal our block size. We will compute the correct offset in our block and transfer the data. Any other
// request to the state machine is treated as a NOP.
//
// ??? should we implement the word level access routines ?
//------------------------------------------------------------------------------------------------------------
void PhysMem::process( ) {
   
    switch( opState.get( )) {
            
        case MO_READ_WORD: {
            
            if ( reqLatency == 0 ) {
                
                uint8_t *dataPtr = &dataArray[ 0 ] [ reqOfs ];
                
                if      ( reqLen == 1 ) reqPtr[ 3 ] = *dataPtr;
                else if ( reqLen == 2 ) memcpy( &reqPtr[ 2 ], dataPtr, 2 );
                else if ( reqLen == 4 ) memcpy( &reqPtr, dataPtr, 4 );
            }
            else reqLatency--;
            
        } break;
            
        case MO_WRITE_WORD: {
            
            if ( reqLatency == 0 ) {
                
                uint8_t *dataPtr = &dataArray[ 0 ] [ reqOfs ];
                
                if      ( reqLen == 1 ) *dataPtr = reqPtr[ 3 ];
                else if ( reqLen == 2 ) memcpy( dataPtr, &reqPtr[ 2 ], 2 );
                else if ( reqLen == 4 ) memcpy( dataPtr, &reqPtr, 4 );
                
            } else reqLatency--;
            
        } break;
            
        case MO_READ_BLOCK: {
            
            if ( reqLatency == 0 ) {
                
                uint8_t *dataPtr = &dataArray[ 0 ] [ reqOfs ];
                memcpy( reqPtr, dataPtr, reqLen );
                
                accessCnt++;
                opState.set( MO_IDLE );
            }
            else reqLatency--;
            
        } break;
            
        case MO_WRITE_BLOCK: {
            
            if ( reqLatency == 0 ) {
                
                uint8_t *dataPtr = &dataArray[ 0 ] [ reqOfs ];
                memcpy( dataPtr, reqPtr, reqLen );
                
                accessCnt++;
                opState.set( MO_IDLE );
            }
            else reqLatency--;
            
        } break;
    }
}


//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------
//
// Processor dependent code memory methods.
//
//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------------------------
// The "PDC" represents the processor dependent code memory range. There is exactly one data array and no
// tags. The data range is read only.
//
//------------------------------------------------------------------------------------------------------------
PdcMem::PdcMem( CpuMemDesc *mDesc ) : CpuMem( mDesc, nullptr ) {
    
    for ( uint32_t i = 0; i < cDesc.blockSets; i++ )
        dataArray[ i ] = (uint8_t *) calloc( cDesc.blockEntries, cDesc.blockSize );
   
    reset( );
}

//------------------------------------------------------------------------------------------------------------
// "process" is the state machine for the PDC memory object. It is very similar to the physical memory object,
// except that there is only a read operation. The PDC content is loaded from the simulator during the
// processor reset.
//
// ??? the input is the PDC address range, which we need to map to the allocated memory. Do we better check
// a correct start address ?
//
// ??? loading TBD
//------------------------------------------------------------------------------------------------------------
void PdcMem::process( ) {
    
    switch( opState.get( )) {
            
        case MO_READ_WORD: {
            
            if ( reqLatency == 0 ) {
               
                uint8_t *dataPtr = &dataArray[ 0 ] [ reqOfs - cDesc.startAdr ];
                
                if      ( reqLen == 1 ) *dataPtr = reqPtr[ 3 ];
                else if ( reqLen == 2 ) memcpy( dataPtr, &reqPtr[ 2 ], 2 );
                else if ( reqLen == 4 ) memcpy( dataPtr, &reqPtr, 4 );
                
            } else reqLatency--;
            
        }  break;
    }
}


//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------
//
// IO Subsystem memory methods.
//
//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------------------------
// The "IoMem" represents the IO subsystem memory range. There is no data nor tag memory.
//
// ??? what else to do here ?
//------------------------------------------------------------------------------------------------------------
IoMem::IoMem( CpuMemDesc *mDesc ) : CpuMem( mDesc, nullptr ) {
    

    reset( );
}

//------------------------------------------------------------------------------------------------------------
// "process" implements the IO subsystem state machine.
//
// ??? idea of a handler to invoke that actually knows what to do ...
//------------------------------------------------------------------------------------------------------------
void IoMem::process( ) {
    
    switch( opState.get( )) {
            
        case MO_READ_WORD: {
            
            if ( reqLatency == 0 ) {
                
                // ??? index is reqOfs - startAdr
                // ??? invoke the handler routine...
                
               
            }
            else reqLatency--;
            
        }  break;
            
        case MO_WRITE_WORD: {
            
            if ( reqLatency == 0 ) {
                
                // ??? index is reqOfs - startAdr
                // ??? invoke the handler routine...
                
            }
            else reqLatency--;
            
        }  break;
    }
}

