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
// address, this is segement and offset. These two values are used to compute the index into the tag and
// data array of the L1 layer. The tag parameter is the tag obtained from the translation unit. The tag must
// match the tag stored in the memory tag array for the indexed block. The memory layers can have different
// block sizes. However, only going from smaller to larger sizes is supported. For example, a 4-word block
// L1 cache can map to a 16-word L2 cache, but not vice versa.
//
// Memory objects think in words, that is block size is specified in words, sizes are numberof words and so
// on. Exechange between the layers is done in blocks. The L1 caches however export to the CPU in interface
// to access a word, a half-word and a byte.
//
// The intended hardware will perform a lookup of TLB and caches in paralell. As a consequence the number of
// bits needed to respresent the block entries cannot be greater than the number of bits necessary to
// represent the page size minus the number of bits it takes to represent the block size. For example, of the
// block size is four words, it will take two bits to index into the
// block. If the page bit size is 12 bits then we have 10 bits left for indexing the cache, i.e. 1024
// entries.
//
// The key parameters for a memory layer are type, access type, number of entries and block size. The
// total size in words is blockEntries * blockSize. All parameters can be found in the memory descriptor.
//
// It seems a bit odd to have caches and memory modelled into one object class. However, this way several
// combinations with unified, split and several cache layers can easily be configured. Layers that do not
// support a particular access method, will just ignore the request.
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
#include "VCPU32-Types.hpp"
#include "VCPU32-Core.hpp"

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
    MO_ALLOCATE_BLOCK_VIRT      = 1,
    MO_READ_BLOCK_VIRT          = 2,
    MO_WRITE_BACK_BLOCK_VIRT    = 3,
    MO_FLUSH_BLOCK_VIRT         = 4,
    
    MO_READ_BLOCK_PHYS          = 10,
    MO_WRITE_BLOCK_PHYS         = 11,
    MO_FLUSH_BLOCK_PHYS         = 12,
    MO_PURGE_BLOCK_PHYS         = 13,
    MO_ALLOCATE_BLOCK_PHYS      = 14,
    MO_WRITE_BACK_BLOCK_PHYS    = 15,
    
    MO_READ_WORD_PHYS           = 20,
    MO_WRITE_WORD_PHYS          = 21,
};

//------------------------------------------------------------------------------------------------------------
// Helper functions. We want to make sure that the size values for blocks and lines are a power of two. The
// other helper functions compute the bit mask and teh nmber of bits in it for the cache lines. Note that for
// block size only 4, 8 and 16 is allowed.
//
//------------------------------------------------------------------------------------------------------------
uint32_t roundUp( uint32_t size, uint32_t limit ) {
    
    uint32_t power = 1;
    while(( power < size ) && ( power < limit )) power *= 2;
    
    return ( power );
}

uint16_t getBlockBits( uint16_t blockSize ) {
    
    if      ( blockSize == 4  ) return( 2 );
    else if ( blockSize == 8  ) return( 3 );
    else if ( blockSize == 16 ) return( 4 );
    else return ( 4 );
}

uint32_t getBlockBitMask( uint16_t blockSize ) {
    
    if      ( blockSize == 4  ) return( 0x03 );
    else if ( blockSize == 8  ) return( 0x07 );
    else if ( blockSize == 16 ) return( 0x0F );
    else                        return( 0x03 );
}

}; // namespace


//------------------------------------------------------------------------------------------------------------
// Memory object constructor. We construct the object from the cache descriptor. Caches consist of the tag
// and the data array. Memory does not need the tag array. For two way associative models, there are two
// arrays of tag and data. We are passed the configuration descriptor and the handle to the the lower
// memory layer, if applicable.
//
//------------------------------------------------------------------------------------------------------------
CpuMem::CpuMem( CpuPhysMemDesc *cfg, CpuMem *mem ) {
    
    memcpy( &cDesc, cfg, sizeof( CpuPhysMemDesc ));
    
    uint32_t limit = (( cDesc.type == MEM_T_PHYS_MEM ) ? MAX_MEM_BLOCK_ENTRIES : MAX_CACHE_BLOCK_ENTRIES );
    
    cDesc.blockEntries  = roundUp( cDesc.blockEntries, limit );
    cDesc.blockSize     = roundUp( cDesc.blockSize, MAX_BLOCK_SIZE );
    cDesc.blockSets     = roundUp( cDesc.blockSets, MAX_BLOCK_SETS );
    blockBits           = getBlockBits( cDesc.blockSize );
    blockBitMask        = getBlockBitMask( cDesc.blockSize );
    opState             = MO_IDLE;
    lowerMem            = mem;
    
    if (( cDesc.type == MEM_T_L1_INSTR      ) ||
        ( cDesc.type == MEM_T_L1_DATA       ) ||
        ( cDesc.type == MEM_T_L2_UNIFIED    )) {
        
        for ( uint32_t i = 0; i < cDesc.blockSets; i++ )
            tagArray[ i ] = (MemTagEntry *) calloc( cDesc.blockEntries, sizeof( MemTagEntry));
    }
    
    for ( uint32_t i = 0; i < cDesc.blockSets; i++ )
        dataArray[ i ] = (uint32_t *) calloc( cDesc.blockEntries, cDesc.blockSize * sizeof( uint32_t ));
    
    switch ( cDesc.type ) {
            
        case MEM_T_L1_INSTR:
        case MEM_T_L1_DATA:     stateMachine = &CpuMem::processL1CacheRequest;    break;
        case MEM_T_L2_UNIFIED:  stateMachine = &CpuMem::processL2CacheRequest;    break;
        case MEM_T_PHYS_MEM:    stateMachine = &CpuMem::processMemRequest;        break;
        default: ;
    }
    
    switch ( cDesc.type ) {
            
        case MEM_T_L1_DATA:     memObjPriority = 1; break;
        case MEM_T_L1_INSTR:    memObjPriority = 2; break;
        case MEM_T_L2_UNIFIED:  memObjPriority = 3; break;
        default:                memObjPriority = 3;
            
    }
    
    reset( );
}

//------------------------------------------------------------------------------------------------------------
// Reset the memory object. We clear the data strcutuires and set the requst state machine to idle.
//
//------------------------------------------------------------------------------------------------------------
void CpuMem::reset( ) {
    
    uint32_t dSize = cDesc.blockEntries * cDesc.blockSize;
    
    for ( uint32_t i = 0; i < cDesc.blockSets; i++ ) {
        
        if ( tagArray[ i ] != nullptr ) {
            
            for ( uint32_t j = 0; j < cDesc.blockEntries; j++ ) {
                
                tagArray[ i ] [ j ].valid = false;
                tagArray[ i ] [ j ].dirty = false;
                tagArray[ i ] [ j ].tag   = 0;
            }
        }
        
        for ( uint32_t j = 0; j < dSize; j++ ) dataArray[ i ] [ j ] = 0;
    }
    
    opState.load( MO_IDLE );
    reqSeg  = 0;
    reqOfs  = 0;
    reqPri  = 0;
    reqTag  = 0;
    reqLen  = 0;
    reqPtr  = nullptr;
    
    clearStats( );
}

//------------------------------------------------------------------------------------------------------------
// Reset the statistics. We maintain counters for total access, misses and how many cycles wwe waited for a
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
// a bit akward to read, we invoke a procedure label stored in the object. Both routines are used in the
// clock step code of the CPU core object. First all "ticks" are handled, then all "process" code is invoked.
//
//------------------------------------------------------------------------------------------------------------
void CpuMem::tick( ) {
    
    opState.tick( );
}

void CpuMem::process( ) {
    
    ((*this).*(stateMachine))( );
}

//------------------------------------------------------------------------------------------------------------
// "abortMemOp" will abort any current operation. It is necessary when we flush the pipeline to avoid fetching
// data that we do not need.
//
//------------------------------------------------------------------------------------------------------------
void CpuMem::abortMemOp( ) {
    
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
// iterate through the tag arrays at the block index, check for a valid entry and matching tag. If no matching
// entry is found and the entry is valid, the maximum block entries constant is returned.
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
// "readVirt" is called from the CPU pipeline data access stage to read data from the L1 cache. The virtual
// adress is "seg.ofs". The "len" paramater specifies the number of bytes to read. Only 1, 2 and 4 bytes are
// allowed, which correspond to byte, half-word and word. The "adrTag" parameter is the physical address used
// for tag comparison for the TLB. If the L1 cache is IDLE, we directly check to see if we have a valid block
// containing the data. If so, the data is returned right away and we have no cycle penalty. Depending on the
// requested data size, the byte or half-word is returned with leading zeros filled on the right side.
// Otherwise, we first need to ALLOCATE a slot and read in the block. The next cycle will start processing
// the request. Note that the CPU core layer will call this routine every clock cycle as long as the operation
// is not completed, i.e. it is back to IDLE.
//
//------------------------------------------------------------------------------------------------------------
bool CpuMem::readVirt( uint32_t seg, uint32_t ofs, uint32_t len, uint32_t adrTag, uint32_t *word ) {
    
    if ( opState.get( ) == MO_IDLE ) {
        
        uint32_t    blockIndex  = (( ofs >> blockBits ) % cDesc.blockEntries );
        uint16_t    matchSet    = matchTag( blockIndex, adrTag );
        
        if ( matchSet < cDesc.blockSets ) {
            
            uint32_t *dataPtr = &dataArray[ matchSet ] [ blockIndex * cDesc.blockSize ];
            uint32_t *wordPtr = &dataPtr[ ofs & blockBitMask ];
            
            if      ( len == 1 )    *word = ( *wordPtr >> (( 3 - ( ofs & 0x03 )))) & 0xFF;
            else if ( len == 2 )    *word = ( *wordPtr >> (( 1 - (( ofs >> 1 ) & 0x01 )))) & 0xFFFF;
            else                    *word = *wordPtr;
            
            return( true );
        }
        else {
            
            opState.set( MO_ALLOCATE_BLOCK_VIRT );
            reqSeg              = seg;
            reqOfs              = ofs;
            reqPri              = memObjPriority;
            reqTag              = adrTag;
            reqPtr              = nullptr;
            reqLen              = 0;
            reqLatency          = cDesc.latency;
            reqTargetSet        = matchSet;
            reqTargetBlockIndex = blockIndex;
            return( false );
        }
    }
    else return(false );
}

//------------------------------------------------------------------------------------------------------------
// "writeVirt" is called from the CPU pipeline data access stage to write data to the L1 cache. The virtual
// adress is "seg.ofs". The "adrTag" parameter is the physical address tag stored in the TLB. If the L1 cache
// is IDLE, we directly check to see if we have a valid block containing the word. If so, the data is stored
// right away and we have no cycle penalty. Othwerwise, we follow the same logic as described for the read
// virtual data operation.
//
//------------------------------------------------------------------------------------------------------------
bool CpuMem::writeVirt( uint32_t seg, uint32_t ofs, uint32_t len, uint32_t adrTag, uint32_t word ) {
    
    if ( opState.get( ) == MO_IDLE ) {
        
        uint32_t    blockIndex  = (( ofs >> blockBits ) % cDesc.blockEntries );
        uint16_t    matchSet    = matchTag( blockIndex, adrTag );
        
        if ( matchSet < cDesc.blockSets ) {
            
            uint32_t *dataPtr = &dataArray[ matchSet ] [ blockIndex * cDesc.blockSize ];
            uint32_t *wordPtr = &dataPtr[ ofs & blockBitMask ];
            
            
            if ( len == 1 )    {
                
                uint32_t bitMask = 0xFF << (( ofs & 0x3 ) * 8 );
                *wordPtr = *wordPtr & ( ~ bitMask );
                *wordPtr = *wordPtr | ( word << ( ofs & 0x3 ));
                
            } else if ( len == 2 ) {
                
                uint32_t bitMask = 0xFFFF << ((( ofs >> 1 ) & 0x01 ) * 16 );
                *wordPtr = *wordPtr & ( ~ bitMask );
                *wordPtr = *wordPtr | ( word << ( ofs & 0x3 ));
                
            } else *wordPtr = word;
            
            return( true );
        }
        else {
            
            opState.set( MO_ALLOCATE_BLOCK_VIRT );
            reqSeg              = seg;
            reqOfs              = ofs;
            reqPri              = memObjPriority;
            reqTag              = adrTag;
            reqPtr              = nullptr;
            reqLen              = 0;
            reqLatency          = cDesc.latency;
            reqTargetSet        = matchSet;
            reqTargetBlockIndex = blockIndex;
            return( false );
        }
    }
    else return( false );
}

//------------------------------------------------------------------------------------------------------------
// "flushBlockVirt" is the method for writing a dirty block back to the lower layer. If there is a match and
// the block is dirty it will be written back to the lower layer. The next state will be WRITE_BACK_BLOCK.
// Otherwise the request is ignored.
//
//------------------------------------------------------------------------------------------------------------
bool CpuMem::flushBlockVirt( uint32_t seg, uint32_t ofs, uint32_t adrTag ) {
    
    if ( opState.get( ) == MO_IDLE ) {
        
        uint32_t    blockIndex  = (( ofs >> blockBits ) % cDesc.blockEntries );
        uint16_t    matchSet    = matchTag( blockIndex, adrTag );
        
        if ( matchSet < cDesc.blockSets ) {
            
            MemTagEntry *tagPtr = &tagArray[ matchSet ] [ blockIndex ];
            
            if ( tagPtr -> dirty ) {
                
                opState.set( MO_FLUSH_BLOCK_VIRT );
                reqSeg              = seg;
                reqOfs              = ofs;
                reqPri              = memObjPriority;
                reqTag              = tagPtr -> tag;
                reqPtr              = &dataArray[ matchSet ] [ blockIndex * cDesc.blockSize ];
                reqLen              = cDesc.blockSize;
                reqLatency          = cDesc.latency;
                reqTargetSet        = matchSet;
                reqTargetBlockIndex = blockIndex;
            }
        }
    }
    
    return( opState.get( ) == MO_IDLE );
}

//------------------------------------------------------------------------------------------------------------
// "purgeBlockVirt" is the method for invalidating the block in the current slot. If there is a match, the
// entry will just be set to invalid, otherwise the request is ignored. Note that there is no flush of a
// potentiall dirty block first.
//
//------------------------------------------------------------------------------------------------------------
bool CpuMem::purgeBlockVirt( uint32_t seg, uint32_t ofs, uint32_t adrTag ) {
    
    if ( opState.get( ) == MO_IDLE ) {
        
        uint32_t    blockIndex  = (( ofs >> blockBits ) % cDesc.blockEntries );
        uint16_t    matchSet    = ( matchTag( blockIndex, adrTag ) < cDesc.blockSets );
        
        if ( matchSet < cDesc.blockSets ) {
            
            opState.set( MO_IDLE );
            reqSeg              = 0;
            reqOfs              = 0;
            reqPri              = memObjPriority;
            reqTag              = 0;
            reqPtr              = nullptr;
            reqLen              = 0;
            reqLatency          = 0;
            reqTargetSet        = matchSet;
            reqTargetBlockIndex = blockIndex;
        }
    }
    
    return( opState.get( ) == MO_IDLE );
}

//------------------------------------------------------------------------------------------------------------
// "readWordPhys" is used by the CPU core to addresssphysical memory. Physical memory can be main memory and
// IO memory space. Both do not need to be fully populatd. A memory read operation fills in the request block
// when the memory layer is IDLE. Only word size transfers are allowed. The address is passed in the "adrTag"
// parameter.
//
// ??? how would we deal not existing memory ?
// ??? how would we do IO space ?
//
//------------------------------------------------------------------------------------------------------------
bool CpuMem::readPhys( uint32_t adr, uint32_t len, uint32_t *word, uint16_t pri ) {
    
    // ??? if adress <= memory boundary.... else unimplemented or IO-space ?
    
    if ( opState.get( ) == MO_IDLE ) {
        
        opState.set( MO_READ_WORD_PHYS );
        reqSeg      = 0;
        reqOfs      = 0;
        reqPri      = pri;
        reqTag      = adr;
        reqPtr      = word;
        reqLen      = len;
        reqLatency  = cDesc.latency;
        return( false );
    }
    else return ( reqLatency == 0 );
}

//------------------------------------------------------------------------------------------------------------
// "writePhys" is used by the CPU core to address the physical memory. Physical memory can be main memory and
// IO memory space. Both do not need to be fully populatd. A memory write operation fills in the request block
// when the memory layer is IDLE. Only word size transfers are allowed. The address is passed in the "adrTag"
// parameter.
//
// ??? how would we deal not existing memory ?
// ??? how would we do IO space ?
//
//------------------------------------------------------------------------------------------------------------
bool CpuMem::writePhys( uint32_t adr, uint32_t len, uint32_t word, uint16_t pri ) {
    
    if ( opState.get( ) == MO_IDLE ) {
        
        opState.set( MO_WRITE_WORD_PHYS );
        reqSeg      = 0;
        reqOfs      = 0;
        reqPri      = pri;
        reqTag      = adr;
        reqPtr      = &word;
        reqLen      = len;
        reqLatency  = cDesc.latency;
        return( false );
    }
    else return ( reqLatency == 0 );
}

//------------------------------------------------------------------------------------------------------------
// "readBlockPhys" is called by an upper layer to read a block of data. This method is implemented in the L2
// cache as well as  the physical memory object. The "adr" parameter is the physical address of the block.
// The block sizes of the upper and lower layer do not necessarily have to match. For example, we could have
// a 8-word L2 cache and a 4-word L1 cache. However, the upper layer must always be configured smaller than
// the lower layer. If the memory object is IDLE, we fill in the request parameters and the next cycle will
// start processing the request. Note that the upper layer will call this routine every clock cycle as long
// as the lower layer operation is not completed. The completion is signaled by the latency count being zero.
// Note also that the "IDLE" state will be set with the next clock cycle, hence we need the latency count to
// know that we are done with the current request.
//
// We need to simulate an arbiter for the physical memory layers. For example, when the two L1 caches are
// connected to the memory, the instruction cache has priority. This situation only arises when MEM is IDLE
// and both caches have a miss. Each cache has a request priority. Before setting the request paramaters,
// the priority value of the request field is checked. If there is a lower priority number, this method call
// simply overwrites the data. Once the requst is served, the reqPri field is cleared.
//
//------------------------------------------------------------------------------------------------------------
bool CpuMem::readBlockPhys( uint32_t adr, uint32_t *buf, uint32_t len, uint16_t pri ) {
    
    if (( opState.get( ) == MO_IDLE ) && ( pri > reqPri )) {
        
        opState.set( MO_READ_BLOCK_PHYS );
        reqSeg      = 0;
        reqOfs      = 0;
        reqPri      = pri;
        reqTag      = adr;
        reqPtr      = buf;
        reqLen      = len;
        reqLatency  = cDesc.latency;
        return( false );
    }
    else return ( reqLatency == 0 );
}

//------------------------------------------------------------------------------------------------------------
// "writeBlockPhys" will transfer multiple words to the lower layer. This method is implemented in the L2
// cache and the physical memory object. The "adr" parameter is the physical address of the block. The block
// sizes of the upper and lower layer do not necessarily have to match. For example, we could have a 8-word
// L2 cache and a 4-word L1 cache. However, the upper layer must always be configured smaller than the lower
// layer. If the memory object is IDLE, we fill in the request parameters and the next cycle will start
// processing the request. Note that the upper layer will call this routine every clock cycle as long as the
// lower layer operation is not completed. The completion is signaled by the latency count being zero. Note
// also that the "IDLE" state will be set with the next clock cycle, hence we need the latency count to know
// that we are done with the current request.
//
//------------------------------------------------------------------------------------------------------------
bool CpuMem::writeBlockPhys( uint32_t adr, uint32_t *buf, uint32_t len, uint16_t pri ) {
    
    if (( opState.get( ) == MO_IDLE ) && ( pri > reqPri )) {
        
        opState.set( MO_WRITE_BLOCK_PHYS );
        reqSeg      = 0;
        reqOfs      = 0;
        reqPri      = pri;
        reqTag      = adr;
        reqPtr      = buf;
        reqLen      = len;
        reqLatency  = cDesc.latency;
        return( false );
    }
    else return ( reqLatency == 0 );
}

//------------------------------------------------------------------------------------------------------------
// "flushBlockPhys" will write the content of the block at adress "adr" to the lower layer. This method only
// applies to L2 caches, there is no concept of flushing physical memory. The block is marked "clean" after
// the operation.
//
//------------------------------------------------------------------------------------------------------------
bool CpuMem::flushBlockPhys( uint32_t adr, uint16_t pri ) {
    
    if (( opState.get( ) == MO_IDLE ) && ( pri > reqPri )) {
        
        opState.set( MO_WRITE_BLOCK_PHYS );
        reqSeg      = 0;
        reqOfs      = 0;
        reqPri      = pri;
        reqTag      = adr;
        reqPtr      = nullptr;
        reqLen      = 0;
        reqLatency  = cDesc.latency;
        return( false );
    }
    else return ( reqLatency == 0 );
}

//------------------------------------------------------------------------------------------------------------
// "purgeBlockPhys" will invalidate the block at adress "adr" to the lower layer. This method only applies to
// L2 caches, there is no concept of purging physical meemory. The block is marked "invalid" after the
// operation.
//
//------------------------------------------------------------------------------------------------------------
bool CpuMem::purgeBlockPhys(  uint32_t adr, uint16_t pri ) {
    
    if (( opState.get( ) == MO_IDLE ) && ( pri > reqPri )) {
        
        opState.set( MO_PURGE_BLOCK_PHYS );
        reqSeg      = 0;
        reqOfs      = 0;
        reqPri      = pri;
        reqTag      = adr;
        reqPtr      = nullptr;
        reqLen      = 0;
        reqLatency  = cDesc.latency;
        return( false );
    }
    else return ( reqLatency == 0 );
}

//------------------------------------------------------------------------------------------------------------
// "processL1CacheRequest" is the state machine for the L1 cache family. An L1 cache directly serves the
// CPU core. A cache hit will have no cycle penalty, a miss will result in the data being fetched from the
// lower layer, which depends on the configured CPU to have a L2 cache or the physical memory as lower
// layer. Since a read or write hit is directly handled by the read and write methods, the state machine is
// only invoked when we deal with a miss, a flush or purge operation. The state machine has several states:
//
// MO_ALLOCATE_BLOCK_VIRT: on a cache miss, we start here. The first task is to locate the block we will
// use for the cache miss. If there is an invalid block in the sets, this is the one to use and the next state
// is MO_READ_BLOCK_VIRT, wehere we will read the block that contains the requested data. Otherwise, we will
// randomly select a block from the sets to be the canditate for serving the cache miss. If the selected
// block is dirty it will be written back first and the next state MO_WRITE_BACK_BLOCK_VIRT.
//
// MO_READ_BLOCK_VIRT: coming from the ALLOCATE state, this state will read the blick from the lower layer.
// The target block has already been identified and we will stay in this state until the lower memory request
// is served. The next state will then just be MO_IDLE, so that the next CPU request will hit the valid block
// and now serve the original request.
//
// MO_WRITE_BACK_BLOCK_VIRT: coming from the MO_READ_BLOCK_VIRT, the task is to write back a dirty block. The
// target block has already been identified and we will stay in this state until the lower memory request is
// served. Once the block is written the next state is to MO_ALLOCATE_BLOCK_VIRT the originally requested
// block.
//
//------------------------------------------------------------------------------------------------------------
void CpuMem::processL1CacheRequest( ) {
    
    switch( opState.get( )) {
            
        case MO_ALLOCATE_BLOCK_VIRT: {
            
            for ( uint16_t i = 0; i < cDesc.blockSets; i++ ) {
                
                MemTagEntry *tagPtr = &tagArray[ i ] [ reqTargetBlockIndex ];
                if ( ! tagPtr -> valid ) {
                    
                    reqTargetSet = i;
                    break;
                }
            }
            
            if ( reqTargetSet >= cDesc.blockSets ) reqTargetSet = random( ) % cDesc.blockSets;
            
            MemTagEntry *tagPtr = &tagArray[ reqTargetSet ] [ reqTargetBlockIndex ];
            
            if (( tagPtr -> valid ) && ( tagPtr -> dirty )) opState.set( MO_WRITE_BACK_BLOCK_VIRT );
            else                                            opState.set( MO_READ_BLOCK_VIRT );
            
        } break;
            
        case MO_READ_BLOCK_VIRT: {
            
            MemTagEntry *tagPtr    = &tagArray[ reqTargetSet ] [ reqTargetBlockIndex ];
            uint32_t     *dataPtr   = &dataArray[ reqTargetSet ] [ reqTargetBlockIndex * cDesc.blockSize ];
            
            if ( lowerMem -> readBlockPhys( reqTag, dataPtr, cDesc.blockSize, memObjPriority )) {
                
                tagPtr -> valid = true;
                tagPtr -> dirty = false;
                tagPtr -> tag   = ( reqTag | ( reqOfs & ( ~ blockBitMask )));
                opState.set( MO_IDLE );
            }
            else waitCyclesCnt ++;
            
        } break;
            
        case MO_WRITE_BACK_BLOCK_VIRT: {
            
            MemTagEntry *tagPtr    = &tagArray[ reqTargetSet ] [ reqTargetBlockIndex ];
            uint32_t     *dataPtr   = &dataArray[ reqTargetSet ] [ reqTargetBlockIndex * cDesc.blockSize ];
            
            if ( lowerMem -> writeBlockPhys( reqTag, dataPtr, cDesc.blockSize, memObjPriority )) {
                
                tagPtr -> valid = false;
                tagPtr -> dirty = false;
                opState.set( MO_ALLOCATE_BLOCK_VIRT );
            }
            else waitCyclesCnt ++;
            
        } break;
            
        case MO_FLUSH_BLOCK_VIRT: {
            
            if ( reqTargetSet < cDesc.blockSets ) {
                
                MemTagEntry *tagPtr    = &tagArray[ reqTargetSet ] [ reqTargetBlockIndex ];
                uint32_t     *dataPtr   = &dataArray[ reqTargetSet ] [ reqTargetBlockIndex * cDesc.blockSize ];
                
                if ( lowerMem -> writeBlockPhys( reqTag, dataPtr, cDesc.blockSize, memObjPriority )) {
                    
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
// "processL2CacheRequest" is the state machine for the L2 cache family. The L2 cache is a thing in the middle
// between the L1 cache and the physical memory. It is a physically indexed, physically tagged cache and
// serves both L1 caches with the L1 instruction cache having priority over the L1 data cache.  The "reqTag"
// field contains the adress, "reqSeg" and "reqOfs" have no meaning. In case of a cache miss, the data is
// fetched from the memory layer.
//
// There are some open questions:
//
// 1.) Is this cache strictly inclusive ? If so, an entry in L2 must exist before it is moved to L1.
// 2.) Invalidating L2 in a strictly inclusive model also needs to invalidate L1.
// 3.) Coherence protocol for more than CPU core ?
//
// ??? under construction ..... first get L1 and MEM stable...
//------------------------------------------------------------------------------------------------------------
void CpuMem::processL2CacheRequest( ) {
    
    uint32_t        blockIndex  = ( reqTag & 0x0FFFFFFF ) >> blockBits;
    uint16_t        blockSet    = matchTag( blockIndex, (( reqSeg < 24 ) | ( reqOfs & 0xFFFFFFFF ))); // ???
    MemTagEntry     *tagPtr     = nullptr;
    uint32_t        *dataPtr    = nullptr;
    bool            tagMatch    = blockSet < MAX_BLOCK_SETS;
    
    if ( tagMatch ) {
        
        tagPtr      =   &tagArray[ blockSet ] [ blockIndex ];
        dataPtr     =   &dataArray[ blockSet ] [ blockIndex ];
    }
    else {
        
        // we have a miss...
        // select a random set for use...
    }
    
    switch( opState.get( )) {
            
        case MO_READ_BLOCK_PHYS: {
            
            if ( reqLatency == 0 ) {
                
            }
            else reqLatency--;
            
        } break;
            
        case MO_WRITE_BLOCK_PHYS: {
            
            if ( reqLatency == 0 ) {
                
            }
            else reqLatency--;
            
        } break;
            
        case MO_FLUSH_BLOCK_PHYS: {
            
            if ( reqLatency == 0 ) {
                
            }
            else reqLatency--;
            
        } break;
            
        case MO_PURGE_BLOCK_PHYS: {
            
            if ( reqLatency == 0 ) {
                
            }
            else reqLatency--;
            
        } break;
            
        case MO_ALLOCATE_BLOCK_PHYS: {
            
            if ( reqLatency == 0 ) {
                
            }
            else reqLatency--;
            
        } break;
            
        case MO_WRITE_BACK_BLOCK_PHYS: {
            
            if ( reqLatency == 0 ) {
                
            }
            else reqLatency--;
            
        } break;
    }
}

//------------------------------------------------------------------------------------------------------------
// "processMemRequest" is the state machine for our physical memory object. The physical memory object is the
// final layer in our memory hierarchy. It does not support any kind of tags, multiple sets, flushing and
// purging of blocks. The only indexing method is the direct indexing method. Upon an incoming request and an
// IDLE state, the request data was stored by the external interface methods and the state machine starts
// working on the request. The state machine has only two states, MO_READ_BLOCK_PHYS and MO_WRITE_BLOCK_PHYS.
//
// It will use the latency counter to simulate the cycles it takes to serve the request. Each time the CPU
// clock ( "tick" ) advances, the latency counter is decremented. When zero is reachd the requested operation
// is executed and the state machine advances to the next state.
//
// In general, we will return the data when the latency counter decrements to zero. The indexing method
// gives us the block in our memory, which is always dataArray[ 0 ]. For a data read or write the blockBits
// portion of the request offset is the index into the selected block. The requestor block size can be
// smaller up to equal our block size. We will compute the correct offset in our block and transfer the
// data. Any other request to the state machine is treated as a NOP.
//
// One more thing. We need to simulate an arbiter. When the two L1 caxches are connected to the memory, the
// instruction cache has priority. This situation only arises when MEM is IDLE and both caches have a miss.
// Each cache has a request priority. This priroity is used to decoded which entry data is stored in the
// memory object calling methods.
//
//------------------------------------------------------------------------------------------------------------
void CpuMem::processMemRequest( ) {
    
    reqTargetBlockIndex = ( reqTag & 0x0FFFFFFF ) >> blockBits;
    
    switch( opState.get( )) {
            
        case MO_READ_WORD_PHYS: {
            
            if ( reqLatency == 0 ) {
                
                uint32_t *wordPtr = &dataArray[ 0 ] [ ( reqTag >> 2 ) ];
            
                if      ( reqLen == 1 ) *reqPtr = ( *wordPtr >> (( 3 - ( reqTag & 0x03 )))) & 0xFF;
                else if ( reqLen == 2 ) *reqPtr = ( *wordPtr >> (( 1 - (( reqTag >> 1 ) & 0x01 )))) & 0xFFFF;
                else                    *reqPtr = *wordPtr;
            }
            else reqLatency--;
            
        } break;
            
        case MO_WRITE_WORD_PHYS: {
            
            if ( reqLatency == 0 ) {
                
                uint32_t *wordPtr = &dataArray[ 0 ] [ ( reqTag >> 2 ) ];
                
                if ( reqLen == 1 )    {
                    
                    uint32_t bitMask = 0xFF << (( reqTag & 0x3 ) * 8 );
                    *wordPtr = *wordPtr & ( ~ bitMask );
                    *wordPtr = *wordPtr | ( *reqPtr << ( reqTag & 0x3 ));
                    
                } else if ( reqLen == 2 ) {
                    
                    uint32_t bitMask = 0xFFFF << ((( reqTag >> 1 ) & 0x01 ) * 16 );
                    *wordPtr = *wordPtr & ( ~ bitMask );
                    *wordPtr = *wordPtr | ( *reqPtr << ( reqTag & 0x3 ));
                    
                } else *wordPtr = *reqPtr;
            }
            else reqLatency--;
            
        } break;
            
        case MO_READ_BLOCK_PHYS: {
            
            if ( reqLatency == 0 ) {
                
                // ??? simplify ??? could it just be the reqTag >> 2 ?
                
                memcpy( reqPtr,
                       &dataArray[ 0 ] [ reqTargetBlockIndex * cDesc.blockSize + ( reqTag & blockBitMask ) ],
                       reqLen * sizeof( uint32_t ));
                
                accessCnt++;
                reqPri = 0;
                opState.set( MO_IDLE );
            }
            else reqLatency--;
            
        } break;
            
        case MO_WRITE_BLOCK_PHYS: {
            
            if ( reqLatency == 0 ) {
                
                // ??? simplify ??? could it just be the reqTag >> 2 ?
                
                memcpy( &dataArray[ 0 ] [ reqTargetBlockIndex * cDesc.blockSize + ( reqTag & blockBitMask ) ],
                       reqPtr,
                       reqLen * sizeof( uint32_t ));
                
                accessCnt++;
                reqPri = 0;
                opState.set( MO_IDLE );
            }
            else reqLatency--;
            
        } break;
    }
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
        case MO_ALLOCATE_BLOCK_VIRT:    return((char *) "ALLOCATE BLOCK VIRT" );
        case MO_READ_BLOCK_VIRT:        return((char *) "READ BLOCK VIRT" );
        case MO_WRITE_BACK_BLOCK_VIRT:  return((char *) "WRITE BACK BLOCK VIRT" );
        case MO_FLUSH_BLOCK_VIRT:       return((char *) "FLUSH BLOCK VIRT" );
            
        case MO_READ_BLOCK_PHYS:        return((char *) "READ BLOCK PHYS" );
        case MO_WRITE_BLOCK_PHYS:       return((char *) "WRITE BLOCK PHYS" );
        case MO_FLUSH_BLOCK_PHYS:       return((char *) "FLUSH BLOCK PHYS" );
        case MO_PURGE_BLOCK_PHYS:       return((char *) "PURGE BLOCK PHYS" );
        case MO_ALLOCATE_BLOCK_PHYS:    return((char *) "ALLOCATE BLOCK PHYS" );
        case MO_WRITE_BACK_BLOCK_PHYS:  return((char *) "WRITE BACK BLOCK PHYS" );
            
        case MO_READ_WORD_PHYS:         return((char *) "READ WORD PHYS" );
        case MO_WRITE_WORD_PHYS:        return((char *) "WRITE WORD PHYS" );
            
        default:                        return((char *) "****" );
    }
}

//------------------------------------------------------------------------------------------------------------
// "getMemTagEntry" is a routine called by the simulator driver to obtain a reference to the tag data entry.
// If the tag array does not exists or the indexed is out of range, a nullpr will be returned.
//
//------------------------------------------------------------------------------------------------------------
MemTagEntry  *CpuMem::getMemTagEntry( uint32_t index, uint8_t set ) {
    
    if ( index >= cDesc.blockEntries )  return( nullptr );
    if ( set >= cDesc.blockSets )       return( nullptr );
    
    return( &tagArray[ set ] [ index ] );
}

//------------------------------------------------------------------------------------------------------------
// "getMemBlockEntry" is a routine called by the simulator driver to obtain a reference to the indexed block.
// If the memory access type is n-way associative the block in "set" is returned.
//
//------------------------------------------------------------------------------------------------------------
uint32_t *CpuMem::getMemBlockEntry( uint32_t index, uint8_t set ) {
    
    if ( index >= cDesc.blockEntries )  return( nullptr );
    if ( set >= cDesc.blockSets )       return( nullptr );
    
    uint32_t *data = dataArray[ set ];
    return( &data[ index * cDesc.blockSize ] );
}

//------------------------------------------------------------------------------------------------------------
// Simple Getters.
//
//------------------------------------------------------------------------------------------------------------
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
