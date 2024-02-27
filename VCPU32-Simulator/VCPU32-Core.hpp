//------------------------------------------------------------------------------------------------------------
//
// VCPU32 - A 32-bit CPU - Core definitions
//
//------------------------------------------------------------------------------------------------------------
//
// CPU24Core represents the CPU itself. The CPU coe conists of the registers, the pipeline stages, the TLB
// and L1 caches. All these objects are defined in this file.
//
//------------------------------------------------------------------------------------------------------------
//
// VCPU32 - A 32-bit CPU - Core definitions
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
#ifndef CPU24Core_h
#define CPU24Core_h

#include "VCPU32-Types.hpp"
#include "VCPU32-Core.hpp"

//------------------------------------------------------------------------------------------------------------
// High level options for the virtual functionality. The options describe the overall strucuture of the TLB
// and Cache subsystems.
//
//------------------------------------------------------------------------------------------------------------
enum VmemOptions : uint32_t {
    
    VMEM_T_NIL                  = 0,
    VMEM_T_SPLIT_TLB            = 1,
    VMEM_T_UNIFIED_TLB          = 2,
    VMEM_T_L1_SPLIT_CACHE       = 3,
    VMEM_T_L2_UNIFIED_CACHE     = 4
};


//------------------------------------------------------------------------------------------------------------
// Basic constants for TLB, caches and memory. The intended hardware will perform a lookup of TLB and caches
// in paralell. As a consequence the number of bits needed to respresent the block entries cannot be greater
// than the number of bits necessary to represent the page size minus the number of bits it takes to represent
// the block size. For example, of the block size is four words, it will take two bits to index into the
// block. If the page bit size is 12 bits then we have 10 bits left for indexing the cache, i.e. 1024 entries.
//
//------------------------------------------------------------------------------------------------------------
const uint32_t  MAX_MEM_BLOCK_ENTRIES   = 16 * 1024 * 1024;    // ??? check ...
const uint32_t  MAX_CACHE_BLOCK_ENTRIES = 1024;
const uint16_t  MAX_BLOCK_SIZE          = 16;
const uint16_t  MAX_BLOCK_SETS          = 4;

//------------------------------------------------------------------------------------------------------------
// A register belongs to a class or registers.
//
//------------------------------------------------------------------------------------------------------------
enum RegClass : uint32_t {
    
    RC_REG_SET_NIL      = 0,
    RC_GEN_REG_SET      = 1,
    RC_SEG_REG_SET      = 2,
    RC_CTRL_REG_SET     = 3,
    RC_PROG_STATE       = 4,
    RC_FD_PSTAGE        = 5,
    RC_MA_PSTAGE        = 6,
    RC_EX_PSTAGE        = 7,
    RC_IC_L1_OBJ        = 8,
    RC_DC_L1_OBJ        = 9,
    RC_UC_L2_OBJ        = 10,
    RC_MEM_OBJ          = 11,
    RC_ITLB_OBJ         = 12,
    RC_DTLB_OBJ         = 13
};

//------------------------------------------------------------------------------------------------------------
// We support two types of TLB. The split instruction and data TLB and a unified, dual ported TLB.
//
// ??? not clear how a dual ported will be modelled yet ...
//------------------------------------------------------------------------------------------------------------
enum TlbType : uint32_t {
    
    TLB_T_NIL           = 0,
    TLB_T_L1_INSTR      = 1,
    TLB_T_L1_DATA       = 2
};

//------------------------------------------------------------------------------------------------------------
// TLB access types. The direct mapped allows for a simple indexing. The fully associative access type is
// intended for the dual ported TLB model.
//
//------------------------------------------------------------------------------------------------------------
enum TlbAccessType : uint32_t {
    
    TLB_AT_NIL                   = 0,
    TLB_AT_FULLY_ASSOCIATIVE     = 1,
    TLB_AT_DIRECT_MAPPED         = 2
};

//------------------------------------------------------------------------------------------------------------
// A TLB object is described through a TLB descriptor. Access methods are direct mapped or fully associative.
// All TLB entry tables are a power of two in size. A TLB has a number entries. The TLB is accessed in one
// cycle.
//
//------------------------------------------------------------------------------------------------------------
struct TlbDesc {
    
    TlbType        type        = TLB_T_NIL;
    TlbAccessType  accessType  = TLB_AT_NIL;
    uint16_t            entries     = 0;
    uint16_t            latency     = 0;
};

//------------------------------------------------------------------------------------------------------------
// We support several types of memory. In the real world they would be called caches and memory. There are
// the two L1 caches and the unified L2 cache. In addition there is the physical memory, which in the CPU
// simulator case is just a kind of cache layer but with the concept of tags and purge and flush operations.
// The idea is that a unified approach to memory allows for a flexible configuration of the layers.
//
//------------------------------------------------------------------------------------------------------------
enum CpuMemType : uint32_t {
    
    MEM_T_NIL         = 0,
    MEM_T_L1_INSTR    = 1,
    MEM_T_L1_DATA     = 2,
    MEM_T_L2_UNIFIED  = 3,
    MEM_T_PHYS_MEM    = 4
};

//------------------------------------------------------------------------------------------------------------
// Cache and memory access types. The direct mapped access types allow for a simple indexing of caches. The
// directly index access type is intended for the physical memory.
//
//------------------------------------------------------------------------------------------------------------
enum CpuMemAccessType : uint32_t {
    
    MEM_AT_NIL                  = 0,
    MEM_AT_DIRECT_INDEXED       = 1,
    MEM_AT_DIRECT_MAPPED        = 2
};

//------------------------------------------------------------------------------------------------------------
// A cache or memory object is described through a descriptor. There are the type and access types. Size
// information the number of entries in an array, the line size describes the number of words in a block.
// the block sets value described the number of sets for n-way associative caches. The latency will specify
// how many clock cycles it will take to perform the respective operation.
//
//------------------------------------------------------------------------------------------------------------
struct CpuMemDesc {
    
    CpuMemType          type            = MEM_T_NIL;
    CpuMemAccessType    accessType      = MEM_AT_NIL;
    uint32_t            blockEntries    = 0;
    uint16_t            blockSize       = 0;
    uint16_t            blockSets       = 0;
    uint16_t            latency         = 0;
};

//------------------------------------------------------------------------------------------------------------
// The CPU core object descriptor holds the configuration settings for the CPU core objects. The descpriptor
// contains the overall memory model, i.e. wheher it is a split or unified model for L1 caches or TLB, and
// descriptors for each nuilding block.
//
//------------------------------------------------------------------------------------------------------------
struct CpuCoreDesc {
    
    uint32_t        flags           = 0;
    
    VmemOptions     tlbOptions      = VMEM_T_NIL;
    VmemOptions     cacheL1Options  = VMEM_T_NIL;
    VmemOptions     cacheL2Options  = VMEM_T_NIL;
    
    CpuMemDesc      iCacheDescL1;
    CpuMemDesc      dCacheDescL1;
    CpuMemDesc      uCacheDescL2;
    CpuMemDesc      memDesc;
    
    TlbDesc         iTlbDesc;
    TlbDesc         dTlbDesc;
};

//------------------------------------------------------------------------------------------------------------
// Core to the CPU is the register set. CPU24 features a set of registers available to the programmer. The
// pipeline stage consist also of a set pf registers. All register implement with the same behaviour. There
// is an inbound value that can be set and an outbound value that can be read. During the imaginary clock
// edge, i.e. our "tick" function, the inbound value is copied to the outbound value. A registger can be
// designated as a register only accessible in privileged mode.
//
//------------------------------------------------------------------------------------------------------------
struct CpuReg {
    
public:
    
    CpuReg( uint32_t val = 0, bool isPriv = false );
    
    void        init( uint32_t val = 0, bool isPriv = false );
    void        reset( );
    void        tick( );
    
    void        load( uint32_t val );
    void        set( uint32_t val );
    uint32_t    get( );
    bool        isPrivReg( );
    
private:
    
    uint32_t    regIn       = 0;
    uint32_t    regOut      = 0;
    bool        isPriv      = false;
};

//------------------------------------------------------------------------------------------------------------
// The TLB entry. Each TLB entry has the translation information, which is the virtual page nmuber and the
// physical page nuber. Then there are the access rights of the page and the protection Id. Finally there
// are the valid, uncachable, tlb dirty, page and data reference trap bits. Regardless of the TLB model we
// use, the entries have the same format. The physical page address stored in teh TLB is a combination of
// the zero bits for the page offsets, the page index in the bank and the bank itself. For the simulator
// this is is the more flexible way. In a real hardeware implementation, the bit fields wizld just store
// the bank and page index values.
//
//
//      pInfo - as in architecture document...
//      aInfo - as in architecture document...
//
//      adrTag[0:3]      - CPU Id.
//      adrTag[4:7]      - memory bank
//      adrTag[8:19]     - memory page
//      adrTag[20:31]    - memory page offset
//
//
//------------------------------------------------------------------------------------------------------------
struct TlbEntry {
    
public:
    
    // ??? fix the bit positions ...
    
    inline bool     tValid( ) { return ( pInfo & 0400000000 ); }
    inline void     setValid( bool arg ) { if ( arg ) pInfo |= 0400000000; else  pInfo &= ~ 0400000000; }
    
    inline bool     tDirty( ) { return( pInfo & 0040000000 ); }
    inline bool     tUncachable( ) { return( pInfo & 020000000 ); }
    inline bool     tTrapPage( ) { return( pInfo & 010000000 ); }
    
    inline bool     tTrapDataPage( ) { return( pInfo & 002000000 ); }
    inline bool     tModifyExPage( ) { return( pInfo & 010000000 ); }
    
    inline int      tPageType( ) { return(( pInfo >> 18 ) & 03 ); }
    inline uint8_t  tPrivL1( ) { return(( pInfo & 000100000 ) ? 1 : 0 ); }
    inline uint8_t  tPrivL2( ) { return(( pInfo & 000040000 ) ? 1 : 0 ); }
    inline uint16_t tProtectId( ) { return( pInfo & 000037777 ); }
    
    inline uint32_t tPhysAdrTag( ) { return ( aInfo << 12 ); }
    
    inline uint16_t tPhysPage( ) { return(( aInfo >> 12 ) & 000007777 ); }
    inline uint8_t  tPhysMemBank( ) { return(( aInfo >> 12 ) & 017 ); }
    inline uint8_t  tCpuId( ) { return(( aInfo >> 16 ) & 017 ); }
 
    uint32_t        vpnHigh;
    uint32_t        vpnLow;
    uint32_t        pInfo;
    uint32_t        aInfo;
    uint32_t        adrtag;
};

//------------------------------------------------------------------------------------------------------------
// TLB object. The different TLB models are built from this basic building block. The TLB entries contain
// the viertual to physical teanslaton data. An entry is filled in two steps, first the data then the
// protection and access rights data, which will ten set the entry valid. The object also maintains a set
// of statistics to keep track of hits, misses, wait cycles and so on.
//
//------------------------------------------------------------------------------------------------------------
struct CpuTlb {
    
public:
    
    CpuTlb( TlbDesc *cfg );
    
    void            reset( );
    void            tick( );
    void            process( );
    void            clearStats( );
    
    void            abortTlbOp( );
    uint16_t        hashAdr( uint32_t seg, uint32_t ofs );
    
    bool            insertTlbEntryAdr( uint32_t seg, uint32_t ofs, uint32_t data );
    bool            insertTlbEntryProt( uint32_t seg, uint32_t ofs, uint32_t data );
    bool            purgeTlbEntry( uint32_t seg, uint32_t ofs );
    
    TlbEntry        *lookupTlbEntry( uint32_t seg, uint32_t ofs );
    TlbEntry        *getTlbEntry( uint32_t index );
    
    bool            purgeTlbEntryData( uint32_t seg, uint32_t ofs );
    bool            insertTlbEntryData( uint32_t seg, uint32_t ofs, uint32_t argAcc, uint32_t argAdr );
    
    uint16_t        getTlbSize ( );
    uint32_t        getTlbInserts( );
    uint32_t        getTlbDeletes( );
    uint32_t        getTlbAccess( );
    uint32_t        getTlbMiss( );
    uint32_t        getTlbWaitCycles( );
    
    uint32_t        getTlbCtrlReg( uint8_t tReg );
    void            setTlbCtrlReg( uint8_t tReg, uint32_t val );
    
private:
    
    TlbDesc         tlbDesc;
    
    uint32_t        tlbOpState          = 0;
    uint32_t        reqOp               = 0;
    uint32_t        reqData             = 0;
    uint32_t        reqDelayCnt         = 0;
    
    TlbEntry        *reqTlbEntry        = nullptr;
    TlbEntry        *tlbArray           = nullptr;
    
    uint32_t        tlbInserts         = 0;
    uint32_t        tlbDeletes         = 0;
    uint32_t        tlbAccess          = 0;
    uint32_t        tlbMiss            = 0;
    uint32_t        tlbWaitCycles      = 0;
};

//------------------------------------------------------------------------------------------------------------
// Memory tag object. Caches need a cache tag array, memory does not. In this case the tag array is just left
// empty. A cache block consist of the the tag entry and an array of words making up the data portion of the
// cache line. Since all caches are physically tagged, the tag is the physical address, which is a combined
// value of bank and offset. Tags also are the full offset word in physical memory, independent of the block
// size configured. Normally, a cache would just store the block number, saving the bits in the tag. Our
// version just stores the physical block address with the block size bits set to zero as the tag.
//
//------------------------------------------------------------------------------------------------------------
struct MemTagEntry {
    
    bool            valid   = false;
    bool            dirty   = false;
    uint32_t        tag     = 0;
};

//------------------------------------------------------------------------------------------------------------
// CPU24 memory object. All caches and the physical memory are build using this generic class. The CPU
// simulator implements a layered memory model. On top are always the level 1 caches on the bottom is
// always the memory layer. Optionally, there can be a level 2 cache. A memory layer has two main structures,
// a tag aray and a data array. Caches need a cache tag array, physical memory does not. In this case the
// tag array is not allocated. There is also a zero passed as segment and segment offset, as it is not
// required in this case. At the core of the object is a state machine that handles the request. If the
// memory layer is not busy, it is IDLE and can accept a new request.
//
// The memory access functions always use a segment:offset pair as the address. In the case of a virtual
// address, this is segement and offset. These two values are used to compute the index into the tag and
// data array of the memory layer. The tag paramater is the tag obtained from the translation unit and must
// match the tag stored in the memory tag array for the indexed block. The memory layers can also have
// different block sizes. However, only going from smaller to larger sizes are supported. For example, a
// 4-word block L1 cache can map to a 16-word L2 cache, but not vice versa. The block function have a length
// paramater to indicate how large the receiving layer block size actually is.
//
//------------------------------------------------------------------------------------------------------------
struct CPU24Mem {
    
    CPU24Mem( CpuMemDesc *mDesc, CPU24Mem *lowerMem = nullptr );
    
    void            reset( );
    void            tick( );
    void            process( );
    void            clearStats( );
    
    void            abortMemOp( );
    
    bool            readWordVirt( uint32_t seg, uint32_t ofs, uint32_t adrTag, uint32_t *word );
    bool            writeWordVirt( uint32_t seg, uint32_t ofs, uint32_t adrTag, uint32_t word );
    
    bool            flushBlockVirt( uint32_t seg, uint32_t ofs, uint32_t adrTag );
    bool            purgeBlockVirt( uint32_t seg, uint32_t ofs, uint32_t adrTag );
    
    bool            readWordPhys( uint32_t adr, uint32_t *word, uint16_t pri = 0 );
    bool            writeWordPhys( uint32_t adr, uint32_t word, uint16_t pri = 0 );

    bool            readBlockPhys( uint32_t adr, uint32_t *buf, uint32_t len, uint16_t pri = 0 );
    bool            writeBlockPhys( uint32_t adr, uint32_t *buf, uint32_t len, uint16_t pri = 0 );
    bool            flushBlockPhys(  uint32_t adr, uint16_t pri = 0 );
    bool            purgeBlockPhys(  uint32_t adr, uint16_t pri = 0 );
    
    int             mapAdr( uint32_t seg, uint32_t ofs );
    MemTagEntry     *getMemTagEntry( uint32_t index, uint8_t set = 0 );
    uint32_t        *getMemBlockEntry( uint32_t index, uint8_t set = 0 );
    
    uint32_t        getBlockEntries( );
    uint16_t        getBlockSize( );
    uint16_t        getBlockSets( );
    uint32_t        getMissCnt( );
    uint32_t        getDirtyMissCnt( );
    uint32_t        getAccessCnt( );
    uint32_t        getWaitCycleCnt( );
    
    uint32_t        getMemCtrlReg( uint8_t mReg );
    void            setMemCtrlReg( uint8_t mReg, uint32_t val );
    char            *getMemOpStr( uint32_t opArg );
    
private:
    
    CpuMemDesc      cDesc;
    
    uint16_t        matchTag( uint32_t index, uint32_t tag );
    void            processL1CacheRequest( );
    void            processL2CacheRequest( );
    void            processMemRequest( );
    
    CpuReg          opState             = 0;
    uint16_t        reqPri              = 0;
    uint32_t        reqSeg              = 0;
    uint32_t        reqOfs              = 0;
    uint32_t        reqTag              = 0;
    uint32_t        *reqPtr             = 0;
    uint32_t        reqLen              = 0;
    uint32_t        reqLatency          = 0;
    uint16_t        reqTargetSet        = 0;
    uint32_t        reqTargetBlockIndex = 0;
    
    uint16_t        blockBits           = 0;
    uint32_t        blockBitMask        = 0;
    uint16_t        memObjPriority      = 0;
    
    uint32_t        accessCnt           = 0;;
    uint32_t        missCnt             = 0;
    uint32_t        dirtyMissCnt        = 0;
    uint32_t        waitCyclesCnt       = 0;
    
    MemTagEntry     *tagArray[ MAX_BLOCK_SETS ]     = { nullptr };
    uint32_t        *dataArray[ MAX_BLOCK_SETS ]    = { nullptr };
    CPU24Mem        *lowerMem                       = nullptr;
    void            ( CPU24Mem::*stateMachine )( )  = nullptr;
};

//------------------------------------------------------------------------------------------------------------
// CPU24 statistical data. Each major component maintains ts own statistics. The CPU itself also maintains
// some statistics. To be defined...
//
// ??? just the statistics for the CPU itself ...
//------------------------------------------------------------------------------------------------------------
struct CpuStatistics {
    
    uint32_t        clockCntr               = 0;
    uint32_t        instrCntr               = 0;
    
    uint32_t        branchesTaken           = 0;
    uint32_t        branchesMispredicted    = 0;
    
    // ??? what else ....
};

//------------------------------------------------------------------------------------------------------------
// "CPU24Core" is the processor core that executes the defined instructions set. It consists primarily of
// the CPU instruction execution part, the TLBs and Caches, and the physical memory interface. The CPU
// core is also the elements that is visible to the simulator.
//
//------------------------------------------------------------------------------------------------------------
struct CpuCore {
    
public:
    
    //--------------------------------------------------------------------------------------------------------
    // The visible part of the CPU24 object.
    //
    //--------------------------------------------------------------------------------------------------------
    CpuCore( CpuCoreDesc *cfg );
    
    void            reset( );
    void            clearStats( );
    void            clockStep( uint32_t numOfSteps = 1 );
    void            instrStep( uint32_t numOfInstr = 1 );
    
    uint32_t        getReg( RegClass regClass, uint8_t regId );
    void            setReg( RegClass regClass, uint8_t regId, uint32_t val );
    
    //--------------------------------------------------------------------------------------------------------
    // The CPU core objects. Since the driver needs access to all of them frequently, we could either have
    // a ton of getter functions, or make the public. Let's go for the latter
    //
    //--------------------------------------------------------------------------------------------------------
    CpuTlb          *iTlb       = nullptr;
    CpuTlb          *dTlb       = nullptr;
    CPU24Mem        *iCacheL1   = nullptr;
    CPU24Mem        *dCacheL1   = nullptr;
    CPU24Mem        *uCacheL2   = nullptr;
    CPU24Mem        *mem        = nullptr;
    
    CpuStatistics stats;
    
private:
    
    //--------------------------------------------------------------------------------------------------------
    // The CPU configuration descritpor and the CPU registers.
    //
    //--------------------------------------------------------------------------------------------------------
    CpuCoreDesc     cpuDesc;
    
    CpuReg          gReg[ MAX_GREGS ];
    CpuReg          sReg[ MAX_SREGS ];
    CpuReg          cReg[ MAX_CREGS ];
    CpuReg          stReg;
    
    //--------------------------------------------------------------------------------------------------------
    // Utility routines.
    //
    // ??? put priv reg check local to each file ?
    //--------------------------------------------------------------------------------------------------------
    bool            isPrivRegForAccMode( RegClass regClass, uint32_t regId, AccessModes mode );
    void            handleTraps( );
    
    //--------------------------------------------------------------------------------------------------------
    // References to other classes. The core needs to have access to the pipeline stages, the virtual and
    // physical memory. In addition, these objects need to access each other too. We delcare them as "friends"
    // to each other.
    //
    //--------------------------------------------------------------------------------------------------------
    friend struct   FetchDecodeStage;
    friend struct   MemoryAccessStage;
    friend struct   ExecuteStage;
    
    struct          FetchDecodeStage    *fdStage    = nullptr;
    struct          MemoryAccessStage   *maStage    = nullptr;
    struct          ExecuteStage        *exStage    = nullptr;
};

#endif
