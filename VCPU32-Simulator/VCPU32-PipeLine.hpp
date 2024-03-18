///------------------------------------------------------------------------------------------------------------
//
// VCPU32 - A 32-bit CPU - Pipeline Definitions
//
//------------------------------------------------------------------------------------------------------------
// The CPU24 pipeline stages file represent the CPU24 processor pipeline. It is a three stage pipeline. The
// details of each stage are described in the declaration section for each stage in the object declaration.
//
//  FD  - instruction fetch and decode
//  MA  - memory access
//  EX  - execute
//
//------------------------------------------------------------------------------------------------------------
//
// VCPU32 - A 32-bit CPU - Pipeline Definitions
// Copyright (C) 2022 -2024 Helmut Fieres
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
#ifndef VCPU32PipeLineStages_h
#define VCPU32PipeLineStages_h

#include "VCPU32-Types.hpp"
#include "VCPU32-Core.hpp"




// ??? idea: have one or two words to specify what the MA and EX stage should do. It is all decoded in the
// FD stage. Display these words also in the command driver.





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
    
    FetchDecodeStage( CpuCore *core );
    
    void            reset( );
    void            tick( );
    void            process( );
    void            stallPipeLine( );
    
    void            setupTrapData( uint32_t trapId,
                                  uint32_t iaSeg,
                                  uint32_t iaOfs,
                                  uint32_t pStat,
                                  uint32_t p1 = 0,
                                  uint32_t p2 = 0,
                                  uint32_t p3 = 0 );
    
    bool            isStalled( );
    void            setStalled( bool arg );
    
    uint32_t        getPipeLineReg( uint8_t pReg );
    void            setPipeLineReg( uint8_t pReg, uint32_t val );
   
    CpuReg          psInstrSeg;
    CpuReg          psInstrOfs;
    
    uint8_t         regIdForValA;
    uint8_t         regIdForValB;
    uint8_t         regIdForValX;
 
    uint32_t        instrFetched;
    uint32_t        instrLoad;
    uint32_t        instrLoadViaOpMode;
    uint32_t        instrStor;
    uint32_t        branchesTaken;
    uint32_t        trapsRaised;
    
private:
    
    CpuCore         *core   = nullptr;
    bool            stalled = false;
    
    uint32_t        instrSeg;
    uint32_t        instrOfs;
    uint32_t        instr;
    uint32_t        instrPrivLevel;
    uint32_t        valA;
    uint32_t        valB;
    uint32_t        valX;
};

//------------------------------------------------------------------------------------------------------------
// The memory access stage first prepares the address from where to get the operand for the instruction. The
// instruction decode stage stored the A, B and X valuzes in the pipeline register of this stage, as well as
// the instruction address and instruction.
//
// This stage now decodes any address computation for the operand and generates the final virtual address by
// selecting the segment register based on this information. Next, this stage fetches the necessary data from
// the addresses computed. The adress is either a virtual adress or a physical address. This state is also
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
    
    MemoryAccessStage( CpuCore *core );
    
    void            reset( );
    void            tick( );
    void            process( );
    void            stallPipeLine( );
    void            flushPipeLine( );
   
    void            setupTrapData( uint32_t trapId,
                                       uint32_t iaSeg,
                                       uint32_t iaOfs,
                                       uint32_t pStat,
                                       uint32_t p1 = 0,
                                       uint32_t p2 = 0,
                                       uint32_t p3 = 0 );
    
    bool            isStalled( );
    void            setStalled( bool arg );
    
    uint32_t        getPipeLineReg( uint8_t pReg );
    void            setPipeLineReg( uint8_t pReg, uint32_t val );
    
    CpuReg          psInstrSeg;
    CpuReg          psInstrOfs;
    CpuReg          psInstr;
    uint32_t        instrPrivLevel;
    CpuReg          psValA;
    CpuReg          psValB;
    CpuReg          psValX;
    CpuReg          psRegIdForValA;
    CpuReg          psRegIdForValB;
    CpuReg          psRegIdForValX;
    
    // ??? add two control words...
    
    CpuReg          psMaStageControl;
    CpuReg          psExStageOntrol;
    
    uint8_t         regIdForValA;
    uint8_t         regIdForValB;
    uint8_t         regIdForValX;
    
    uint32_t        trapsRaised;
 
private:
    
    CpuCore         *core       = nullptr;
    bool            stalled     = false;
    
    uint32_t        instrSeg;
    uint32_t        instrOfs;
    uint32_t        instr;
    uint32_t        valA;
    uint32_t        valB;
    uint32_t        valX;
    uint32_t        valS;
};

//------------------------------------------------------------------------------------------------------------
// The execute stage is finally the stage where the work is done. Inputs A and B from the previous stage
// are the main inputs to the ALU operation. If there are no traps, any results are written back to the
// register files.
//
//------------------------------------------------------------------------------------------------------------
struct ExecuteStage {
    
public:
    
    ExecuteStage( CpuCore *core );
    
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
                                       uint32_t iaSeg,
                                       uint32_t iaOfs,
                                       uint32_t pStat,
                                       uint32_t p1 = 0,
                                       uint32_t p2 = 0,
                                       uint32_t p3 = 0 );
    
    CpuReg          psInstr;
    CpuReg          psInstrSeg;
    CpuReg          psInstrOfs;
    CpuReg          psValA;
    CpuReg          psValB;
    CpuReg          psValX;
    
    // ??? add one control words...
    
    CpuReg          psExStageOntrol;
    
    uint8_t         regIdForValR;
    uint32_t        valR;
    
    uint32_t        instrExecuted;
    uint32_t        branchesTaken;
    uint32_t        branchesNotTaken;
    uint32_t        trapsRaised;
    
private:
    
    CpuCore         *core       = nullptr;
    bool            stalled     = false;
    
    uint32_t        instrSeg;
    uint32_t        instrOfs;
    uint32_t        instr;
    uint32_t        valA;
    uint32_t        valB;
    uint32_t        valX;
    uint32_t        branchTaken;
    bool            valCarry;
    
};

#endif /* CPU24PipeLineStages_h */
