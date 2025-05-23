//------------------------------------------------------------------------------------------------------------
//
// VCPU32 - A 32-bit CPU - Core definitions
//
//------------------------------------------------------------------------------------------------------------
//
// CPU24Core represents the CPU itself. The CPU core consists of the registers, the pipeline stages, the TLB
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

#include "VCPU32-Types.h"
#include "VCPU32-Core.h"

//------------------------------------------------------------------------------------------------------------
// High level options for the virtual functionality. The options describe the overall structure of the TLB
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
// A register belongs to a class or registers.
//
//------------------------------------------------------------------------------------------------------------
enum RegClass : uint32_t {
    
    RC_REG_SET_NIL      = 0,
    RC_GEN_REG_SET      = 1,
    RC_SEG_REG_SET      = 2,
    RC_CTRL_REG_SET     = 3,
    RC_FD_PSTAGE        = 4,
    RC_MA_PSTAGE        = 5,
    RC_EX_PSTAGE        = 6,
    RC_IC_L1_OBJ        = 7,
    RC_DC_L1_OBJ        = 8,
    RC_UC_L2_OBJ        = 9,
    RC_MEM_OBJ          = 10,
    RC_ITLB_OBJ         = 11,
    RC_DTLB_OBJ         = 12
};

//------------------------------------------------------------------------------------------------------------
// Each pipeline consists of a "combinatorial logic" part and the pipeline registers. Our pipeline registers
// are just a set of registers.
//
//------------------------------------------------------------------------------------------------------------
enum PipeLineStageRegId : uint32_t {
    
    PSTAGE_REG_STALLED      = 0,
    PSTAGE_REG_ID_PSW_0     = 1,
    PSTAGE_REG_ID_PSW_1     = 2,
    PSTAGE_REG_ID_INSTR     = 3,
    PSTAGE_REG_ID_VAL_A     = 4,
    PSTAGE_REG_ID_VAL_B     = 5,
    PSTAGE_REG_ID_VAL_X     = 6,
    PSTAGE_REG_ID_VAL_S     = 7,
    PSTAGE_REG_ID_VAL_ST    = 8,
    PSTAGE_REG_ID_RID_A     = 9,
    PSTAGE_REG_ID_RID_B     = 10,
    PSTAGE_REG_ID_RID_X     = 11,
    PSTAGE_REG_ID_MA_CTRL   = 12,
    PSTAGE_REG_ID_EX_CTRL   = 13
};

//------------------------------------------------------------------------------------------------------------
//
//
//------------------------------------------------------------------------------------------------------------
enum MemoryObjRegId : uint32_t {
    
    MC_REG_STATE            = 0,
    MC_REG_REQ_SEG          = 1,
    MC_REG_REQ_OFS          = 2,
    MC_REG_REQ_PRI          = 3,
    MC_REG_REQ_TAG          = 4,
    MC_REG_REQ_ADR          = 5,
    MC_REG_REQ_LEN          = 6,
    MC_REG_REQ_BLOCK_INDEX  = 7,
    MC_REG_REQ_BLOCK_SET    = 8,
    MC_REG_REQ_LATENCY      = 9,
    
    MC_REG_START_ADR        = 10,
    MC_REG_END_ADR          = 11,
    MC_REG_LATENCY          = 12,
    MC_REG_BLOCK_ENTRIES    = 13,
    MC_REG_BLOCK_SIZE       = 14,
    MC_REG_SETS             = 15
};



//------------------------------------------------------------------------------------------------------------
//
//
//------------------------------------------------------------------------------------------------------------
// ??? TLB register IDs...



//------------------------------------------------------------------------------------------------------------
// We support two types of TLB. The split instruction and data TLB and a unified, dual ported TLB.
//
//------------------------------------------------------------------------------------------------------------
enum TlbType : uint32_t {
    
    TLB_T_NIL           = 0,
    TLB_T_L1_INSTR      = 1,
    TLB_T_L1_DATA       = 2,
    TLB_T_L1_DUAL       = 3
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
    uint16_t       entries     = 0;
    uint16_t       latency     = 0;
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
    MEM_T_PHYS_MEM    = 4,
    MEM_T_PDC_MEM     = 5,
    MEM_T_IO_MEM      = 6
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
// how many clock cycles it will take to perform the respective operation. For main memory, the PDC and the
// IO memory there is a start and ending address, since these memory will not cover all of the possible
// memory range.
//
//------------------------------------------------------------------------------------------------------------
struct CpuMemDesc {
    
    CpuMemType          type            = MEM_T_NIL;
    CpuMemAccessType    accessType      = MEM_AT_NIL;
    uint32_t            blockEntries    = 0;
    uint32_t            blockSize       = 0;
    uint32_t            blockSets       = 0;
    uint32_t            startAdr        = 0;
    uint32_t            endAdr          = 0;
    uint32_t            latency         = 0;
    uint32_t            priority        = 0;
};

//------------------------------------------------------------------------------------------------------------
// The CPU core object descriptor holds the configuration settings for the CPU core objects. The descriptor
// contains the overall memory model, i.e. whether it is a split or unified model for L1 caches or TLB, and
// descriptors for each building block.
//
// ??? should the core have knowledge of L2 and MEM or just an abstract memory interface ? Consider the
// case where we have several cores ...
//------------------------------------------------------------------------------------------------------------
struct CpuCoreDesc {
    
    uint32_t            flags           = 0;
    
    VmemOptions         tlbOptions      = VMEM_T_NIL;
    VmemOptions         cacheL1Options  = VMEM_T_NIL;
    VmemOptions         cacheL2Options  = VMEM_T_NIL;
    
    CpuMemDesc          iCacheDescL1;
    CpuMemDesc          dCacheDescL1;
    CpuMemDesc          uCacheDescL2;
    CpuMemDesc          memDesc;
    CpuMemDesc          pdcDesc;
    CpuMemDesc          ioDesc;
    
    TlbDesc             iTlbDesc;
    TlbDesc             dTlbDesc;
};

//------------------------------------------------------------------------------------------------------------
// Core to the CPU is the register set. CPU24 features a set of registers available to the programmer. The
// pipeline stage consist also of a set pf registers. All register implement with the same behavior. There
// is an inbound value that can be set and an outbound value that can be read. During the imaginary clock
// edge, i.e. our "tick" function, the inbound value is copied to the outbound value. A register can be
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
    uint32_t    getLatched( );
    
    bool        getBit( int pos );
    void        setBit( int pos );
    void        setBit( int pos, bool val );
    void        clearBit( int pos );
    
    uint32_t    getBitField( int pos, int len, bool sign = false );
    void        setBitField( uint32_t val, int pos, int len );
    void        setBitField( int pos, int len );
    void        clearBitField( int pos, int len );
    void        orBitField( uint32_t val, int pos, int len  );
    void        andBitField( uint32_t val, int pos, int len  );
   
    bool        isPrivReg( );
    
private:
    
    uint32_t    regIn       = 0;
    uint32_t    regOut      = 0;
    bool        isPriv      = false;
};

//------------------------------------------------------------------------------------------------------------
// The TLB entry. Each TLB entry has the translation information, which is the virtual page number and the
// physical page number. Then there are the access rights of the page and the protection Id. Finally there
// are the valid, uncachable, tlb dirty, page and data reference trap bits. Regardless of the TLB model we
// use, the entries have the same format. 
//
//------------------------------------------------------------------------------------------------------------
struct TlbEntry {
    
public:
    
    bool        tValid( );
    void        setValid( bool arg );
    
    bool        tTrapPage( );
    bool        tDirty( );
    bool        tTrapDataPage( );
    
    uint32_t    tPageType( );
    uint32_t    tPrivL1( );
    uint32_t    tPrivL2( );
    uint16_t    tSegId( );
    uint32_t    tPhysPage( );
   
    uint32_t    vpnHigh;
    uint32_t    vpnLow;
    uint32_t    pInfo;
    uint32_t    aInfo;
};

//------------------------------------------------------------------------------------------------------------
// TLB object. The different TLB models are built from this basic building block. The TLB entries contain
// the virtual to physical translation data. An entry is filled in two steps, first the data then the
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
// VCPU-32 memory objects. All caches, the physical memory and the memory mapped IO system are build using
// the CPUMem class as the base object. When it comes to caches and main memory, VCPU-32 implements a
// layered model. On top are always the L1 caches. There is an optional L2 cache. Below is the physical
// memory layer. Next is the PDC memory, which is an uncached memory region. Finally, there is the IO memory
// address range. At the core of each memory object is a state machine that handles the request. If the memory
// object is not busy, it is IDLE and can accept a new request.
//
// The memory access functions always use a segment:offset pair as the address. In the case of a virtual
// address, this is segment and offset. These two values are used to compute the index into the tag and
// data array of the memory layer. The tag parameter is the tag obtained from the translation unit and must
// match the tag stored in the memory tag array for the indexed block. The memory layers can also have
// different block sizes. However, only going from smaller to larger sizes are supported. For example, a
// 4-word block L1 cache can map to a 16-word L2 cache, but not vice versa. The block function have a length
// parameter to indicate how large the receiving layer block size actually is.
//
// All address offsets are byte addresses. All sizes are measured in bytes, rounded up to a word size when
// necessary. The data array in a cache or memory layers is also an array of bytes, which allows a greater
// flexibility in configuring different memory block sizes without a ton of address arithmetic. Some
// interfaces to the memory objects do however pass as data as a word parameter.
//
// All memory objects have the same basic structure. At the core is a memory object specific state machine
// that accepts a request and process it. The "process" methods is the abstract method that each inheriting
// class must implement. The "tick" is the system clock that advances that state machine. The CPU and the
// memory objects themselves call each other with the defined methods. To simulate a latency, the method call
// is repeated every clock cycle until the latency count is reached and the request is resolved.
//
// To simulate an arbitration a request with higher priority will overwrite a request entered but the current
// opState registers is still in the "IDLE" state. The logic is that a request is queued and the opState
// register will be loaded with the "work" state in to become visible in opState register in the next cycle.
//
//------------------------------------------------------------------------------------------------------------
struct CpuMem {
    
    CpuMem( CpuMemDesc *mDesc, CpuMem *lowerMem = nullptr );
    
    void            reset( );
    void            tick( );
    virtual void    process( ) = 0;
    void            clearStats( );
    void            abortOp( );
   
    virtual bool    readWord( uint32_t seg, uint32_t ofs, uint32_t tag, uint32_t len, uint32_t *word, uint32_t pri = 0 );
    virtual bool    writeWord( uint32_t seg, uint32_t ofs, uint32_t tag, uint32_t len, uint32_t word, uint32_t pri = 0 );

    virtual bool    readBlock( uint32_t seg, uint32_t ofs, uint32_t tag, uint8_t *buf, uint32_t len, uint32_t pri = 0 );
    virtual bool    writeBlock( uint32_t seg, uint32_t ofs, uint32_t tag, uint8_t *buf, uint32_t len, uint32_t pri = 0 );
    virtual bool    flushBlock( uint32_t seg, uint32_t ofs, uint32_t tag, uint32_t pri = 0 );
    virtual bool    purgeBlock( uint32_t seg, uint32_t ofs, uint32_t tag, uint32_t pri = 0 );
    
    int             mapAdr( uint32_t seg, uint32_t ofs );
    MemTagEntry     *getMemTagEntry( uint32_t index, uint8_t set = 0 );
    uint8_t         *getMemBlockEntry( uint32_t index, uint8_t set = 0 );
    uint32_t        getMemDataWord( uint32_t ofs, uint8_t set = 0 );
    void            putMemDataWord( uint32_t ofs, uint32_t val, uint8_t set = 0 );
    
    uint32_t        getMemSize( );
    uint32_t        getStartAdr( );
    uint32_t        getEndAdr( );
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
    bool            validAdr( uint32_t ofs );
    
protected:
    
    CpuMemDesc      cDesc;
    
    uint16_t        matchTag( uint32_t index, uint32_t tag );
    
    CpuReg          opState             = 0;
    uint16_t        reqPri              = 0;
    uint32_t        reqSeg              = 0;
    uint32_t        reqOfs              = 0;
    uint32_t        reqTag              = 0;
    uint8_t         *reqPtr             = nullptr;
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
    uint8_t         *dataArray[ MAX_BLOCK_SETS ]    = { nullptr };
    CpuMem          *lowerMem                       = nullptr;
};


//------------------------------------------------------------------------------------------------------------
// "L1CacheMem" is the memory object representing the L1 caches. It overrides the word and block access
// routines of a basic memory object, since it has a data and tag array structure. Also, a read or write
// word access is severed directly in case of a cache hit.
//
//------------------------------------------------------------------------------------------------------------
struct L1CacheMem : CpuMem {
    
    L1CacheMem( CpuMemDesc *mDesc, CpuMem *lowerMem );
    
    bool    readWord( uint32_t seg, uint32_t ofs, uint32_t adrTag, uint32_t len, uint32_t *data, uint32_t pri = 0 );
    bool    writeWord( uint32_t seg, uint32_t ofs, uint32_t len, uint32_t adrTag, uint32_t data, uint32_t pri = 0 );
    
    bool    flushBlock( uint32_t seg, uint32_t ofs, uint32_t tag, uint32_t pri = 0 );
    bool    purgeBlock( uint32_t seg, uint32_t ofs, uint32_t tag, uint32_t pri = 0 );

    void    process( );    
};

//------------------------------------------------------------------------------------------------------------
// "L2CacheMem" is an optional layer between main memory and the L1 caches. It has a data and a tag array.
// Since it is physically indexed and tagged, there is no need to overwrite the basic block access methods
// of the base class. The L2 cache state machine will do the tag match handling.
//
//
//------------------------------------------------------------------------------------------------------------
struct L2CacheMem : CpuMem {
    
    L2CacheMem( CpuMemDesc *mDesc, CpuMem *lowerMem );
   
    void    process( );
};

//------------------------------------------------------------------------------------------------------------
// "PhysMem" represents the actual main memory.
//
//------------------------------------------------------------------------------------------------------------
struct PhysMem : CpuMem {
    
    PhysMem( CpuMemDesc *mDesc );
    
    void    process( );
};

//------------------------------------------------------------------------------------------------------------
// "PdcMem" represents the processor dependent code memory range.
//
//------------------------------------------------------------------------------------------------------------
struct PdcMem : CpuMem {
    
    PdcMem( CpuMemDesc *mDesc );

    void    process( );
};

//------------------------------------------------------------------------------------------------------------
// "IoMem" represents the IO subsystem address range.
//
// ??? to do ...
//------------------------------------------------------------------------------------------------------------
struct IoMem : CpuMem {
    
    IoMem( CpuMemDesc *mDesc );
    
    void process( );
};

//------------------------------------------------------------------------------------------------------------
// CPU24 statistical data. Each major component maintains its own statistics. The CPU itself also maintains
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
// The CPU24 pipeline stages file represent the CPU24 processor pipeline. It is a three stage pipeline. The
// details of each stage are described in the declaration section for each stage in the object declaration.
//
//  FD  - instruction fetch and decode
//  MA  - memory access
//  EX  - execute
//
//------------------------------------------------------------------------------------------------------------
// The instruction fetch and decode stage will retrieve the next instruction. The instruction address to be
// used is read from the instruction address register address. Depending on whether code address translation
// is enabled or not, the instruction fetch is either a virtual memory or physical memory read operation.
// The instruction decode hardware is essentially a big combinatorial network to set the pipeline register
// fields A,B and X with known values that can be derived from the instruction fields or the register set.
// The idea is also to perform computations that solely depend on the instruction word information right in
// this stage.
//
// Like all pipeline stages, the incoming data is obtained from the pipeline register of the stage. The
// output is stored to the pipeline register of the next stage. The pipeline register of this fetch and decode
// stage is actually also the Instruction Address, IA.
//
//------------------------------------------------------------------------------------------------------------
struct FetchDecodeStage {
    
public:
    
    FetchDecodeStage( struct CpuCore *core );
    
    void            reset( );
    void            tick( );
    void            process( );
    void            stallPipeLine( );
    
    void            setupTrapData( uint32_t trapId,
                                  uint32_t psw0,
                                  uint32_t psw1,
                                  uint32_t p1 = 0,
                                  uint32_t p2 = 0,
                                  uint32_t p3 = 0 );
    
    bool            isStalled( );
    void            setStalled( bool arg );
    
    uint32_t        getPipeLineReg( uint8_t pReg );
    void            setPipeLineReg( uint8_t pReg, uint32_t val );
    
    bool            checkProtectId( uint16_t segId );
    
    bool            dependencyValA( uint32_t regId );
    bool            dependencyValB( uint32_t regId );
    bool            dependencyValX( uint32_t regId );
    bool            dependencyValST( );
    bool            consumesValB( );
    bool            consumesValX( );
    
    CpuReg          psPstate0;
    CpuReg          psPstate1;
    uint32_t        instr;
   
    uint32_t        instrFetched;
    uint32_t        instrLoad;
    uint32_t        instrLoadViaOpMode;
    uint32_t        instrStor;
    uint32_t        branchesTaken;
    uint32_t        trapsRaised;
    
private:
    
    struct CpuCore  *core   = nullptr;
    bool            stalled = false;
};

//------------------------------------------------------------------------------------------------------------
// The memory access stage first prepares the address from where to get the operand for the instruction for
// memory access type instructions. The instruction decode stage stored the A, B and X values in the pipeline
// register of this stage, as well as the instruction address and instruction.
//
// This stage now decodes any address computation for the operand and generates the final virtual address by
// selecting the segment register based on this information. Next, this stage fetches the necessary data from
// the addresses computed. The address is either a virtual address or a physical address. This state is also
// the stage where any store to memory will take place. A data fetch operation stores the date into the B
// pipeline register of the EX stage and passes all other registers just on to the EX stage. If there is no
// data fetch necessary, B is also just passed on.
//
// For branch instructions the target address is computed here and directly set as the next instruction.
// For the TLB and Cache related instructions, this stage is also the starting point of operation.
//
//------------------------------------------------------------------------------------------------------------
struct MemoryAccessStage {
    
public:
    
    MemoryAccessStage( struct CpuCore *core );
    
    void            reset( );
    void            tick( );
    void            process( );
    void            stallPipeLine( );
    void            flushPipeLine( );
    
    void            setupTrapData( uint32_t trapId,
                                  uint32_t psw0,
                                  uint32_t psw1,
                                  uint32_t p1 = 0,
                                  uint32_t p2 = 0,
                                  uint32_t p3 = 0 );
    
    bool            isStalled( );
    void            setStalled( bool arg );
    
    uint32_t        getPipeLineReg( uint8_t pReg );
    void            setPipeLineReg( uint8_t pReg, uint32_t val );
    
    bool            checkProtectId( uint16_t segId );
    
    bool            dependencyValA( uint32_t regId );
    bool            dependencyValB( uint32_t regId );
    bool            dependencyValX( uint32_t regId );
    bool            dependencyValST( );
    
    CpuReg          psPstate0;
    CpuReg          psPstate1;
    CpuReg          psInstr;
    CpuReg          psValA;
    CpuReg          psValB;
    CpuReg          psValX;
    
    uint32_t        instrPrivLevel;
    uint32_t        trapsRaised;
 
private:
    
    struct CpuCore  *core       = nullptr;
    bool            stalled     = false;
};

//------------------------------------------------------------------------------------------------------------
// The execute stage is finally the stage where the work is done. Inputs A and B from the previous stage
// are the main inputs to the ALU operation. If there are no traps, any results are written back to the
// register files.
//
//------------------------------------------------------------------------------------------------------------
struct ExecuteStage {
    
public:
    
    ExecuteStage( struct CpuCore *core );
    
    void            reset( );
    void            tick( );
    void            process( );
    void            stallPipeLine( );
    void            flushPipeLine( );
    
    bool            isStalled( );
    void            setStalled( bool arg );
    
    uint32_t        getPipeLineReg( uint32_t pReg );
    void            setPipeLineReg( uint32_t pReg, uint32_t val );
    
    void            setupTrapData( uint32_t trapId,
                                  uint32_t  psw0,
                                  uint32_t  psw1,
                                  uint32_t  p1 = 0,
                                  uint32_t  p2 = 0,
                                  uint32_t  p3 = 0 );
    
    CpuReg          psPstate0;
    CpuReg          psPstate1;
    CpuReg          psInstr;
    CpuReg          psValA;
    CpuReg          psValB;
    CpuReg          psValX;

    uint32_t        instrExecuted;
    uint32_t        branchesTaken;
    uint32_t        branchesNotTaken;
    uint32_t        trapsRaised;
    
private:
    
    CpuCore         *core       = nullptr;
    bool            stalled     = false;
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
    // ??? unified cache, physical memory, PDC and IO should be moved out of the core when we have many
    // cores...
    //--------------------------------------------------------------------------------------------------------
    CpuTlb          *iTlb       = nullptr;
    CpuTlb          *dTlb       = nullptr;
    L1CacheMem      *iCacheL1   = nullptr;
    L1CacheMem      *dCacheL1   = nullptr;
    L2CacheMem      *uCacheL2   = nullptr;
    PhysMem         *physMem    = nullptr;
    PdcMem          *pdcMem     = nullptr;
    IoMem           *ioMem      = nullptr;
    
    CpuStatistics   stats;
    
private:
    
    //--------------------------------------------------------------------------------------------------------
    // The CPU configuration descriptor and the CPU registers.
    //
    //--------------------------------------------------------------------------------------------------------
    CpuCoreDesc     cpuDesc;
   
    CpuReg          gReg[ MAX_GREGS ];
    CpuReg          sReg[ MAX_SREGS ];
    CpuReg          cReg[ MAX_CREGS ];
    
    //--------------------------------------------------------------------------------------------------------
    // Utility routines.
    //
    //--------------------------------------------------------------------------------------------------------
    void            handleTraps( );
    
    //--------------------------------------------------------------------------------------------------------
    // References to other classes. The core needs to have access to the pipeline stages, the virtual and
    // physical memory. In addition, these objects need to access each other too. We declare them as "friends"
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
