//------------------------------------------------------------------------------------------------------------
//
// VCPU32 - A 32-bit CPU - CPU Core
//
//------------------------------------------------------------------------------------------------------------
//
// CPU24 core. This object represents the CPU. It offers the external interfaces to the CPU. There are methods
// to control the execution as well as methods to access the CPU registers.
//
//------------------------------------------------------------------------------------------------------------
//
// VCPU32 - A 32-bit CPU - CPU Core
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
#include "VCPU32-PipeLine.hpp"

//------------------------------------------------------------------------------------------------------------
// The CPU24Core object constructor. Based on the cpu descriptor, we initilaize the registers and create the
// memory objects and the pipeline stages.
//
//------------------------------------------------------------------------------------------------------------
CpuCore::CpuCore( CpuCoreDesc *cfg ) {
    
    memcpy( &cpuDesc, cfg, sizeof( CpuCoreDesc ));
  
    stReg.init( 0, false );
    for ( uint8_t i = 0; i < 8; i++  )  gReg[ i ].init( 0, false );
    for ( uint8_t i = 0; i < 3; i++  )  sReg[ i ].init( 0, false );
    for ( uint8_t i = 4; i < 7; i++  )  sReg[ i ].init( 0, true );
    for ( uint8_t i = 0; i < 31; i++  ) cReg[ i ].init( 0, true );
    
    if ( cfg -> tlbOptions == VMEM_T_SPLIT_TLB ) {
        
        iTlb = new CpuTlb( &cpuDesc.iTlbDesc );
        dTlb = new CpuTlb( &cpuDesc.dTlbDesc );
    }
    else if ( cfg -> tlbOptions == VMEM_T_UNIFIED_TLB ) {
        
        // ??? what is the proper way to model a joint TLB ?
        iTlb = new CpuTlb( &cpuDesc.iTlbDesc );
        dTlb = new CpuTlb( &cpuDesc.dTlbDesc );
    }
    
    physMem = new PhysMem( &cpuDesc.memDesc );
    pdcMem  = new PdcMem( &cpuDesc.pdcDesc );
    
    if ( cfg -> cacheL2Options == VMEM_T_L2_UNIFIED_CACHE ) {
        
        uCacheL2 = new L2CacheMem( &cpuDesc.uCacheDescL2, physMem );
        iCacheL1 = new L1CacheMem( &cpuDesc.iCacheDescL1, uCacheL2 );
        dCacheL1 = new L1CacheMem( &cpuDesc.dCacheDescL1, uCacheL2 );
    }
    else {
        
        iCacheL1 = new L1CacheMem( &cpuDesc.iCacheDescL1, physMem );
        dCacheL1 = new L1CacheMem( &cpuDesc.dCacheDescL1, physMem );
    }
   
    fdStage = new FetchDecodeStage( this );
    maStage = new MemoryAccessStage( this );
    exStage = new ExecuteStage( this );
    
    reset( );
}

//------------------------------------------------------------------------------------------------------------
// "clearStats" resets the statistic counters in all cpu core objects.
//
//------------------------------------------------------------------------------------------------------------
void  CpuCore::clearStats( ) {
    
    iTlb -> clearStats( );
    dTlb -> clearStats( );
    
    iCacheL1 -> clearStats( );
    dCacheL1 -> clearStats( );
    if ( uCacheL2 != nullptr ) uCacheL2 -> clearStats( );
    physMem -> clearStats( );
    
    stats.clockCntr                = 0;
    stats.instrCntr                = 0;
    stats.branchesTaken            = 0;
    stats.branchesMispredicted     = 0;
}

//------------------------------------------------------------------------------------------------------------
// CPU24Core reset function. We set all registers to a zero value. This also means that the start of program
// execution is in physical mode, privileged and at the architected address.
//
//------------------------------------------------------------------------------------------------------------
void CpuCore::reset( ) {
    
    stReg.reset( );
    for ( uint8_t i = 0; i < 8; i++  )  gReg[ i ].reset( );
    for ( uint8_t i = 0; i < 8; i++  )  sReg[ i ].reset( );
    for ( uint8_t i = 0; i < 32; i++  ) cReg[ i ].reset( );
    
    // ??? set the PC counter to a defined value ?
    
    iTlb -> reset( );
    dTlb -> reset( );
    
    iCacheL1 -> reset( );
    dCacheL1 -> reset( );
    if ( uCacheL2 != nullptr ) uCacheL2 -> reset( );
    
    fdStage -> reset( );
    maStage -> reset( );
    exStage -> reset( );
    
    clearStats( );
}

//------------------------------------------------------------------------------------------------------------
// "ClockStep" is the method that advances the simulator by one clock cycle. Each major component will use
// the input from the respective register outputs and perform the "combinatorial logic", i.e. all data in
// the input registers is processed, and any output is written to the input of the respective registers.
// For example, the FD stage will take as inputs the instruction address registers and writes its instrcution
// decoding results to the FD/MA pipeline registers.
//
// On the next "tick" all latched inputs in the registers then become the regioster output and thus the input
// for the next round of cpomponent "process". In our example, the FD/MA pipelne registers are the input to
// the MA pipeline stage.
//
// Note: we could have done also first the "ticks" and then the "process". It is just a matter of what the
// step will mean. Since the simulator does show the ouput of any register in the display commands, it is
// better to define a step as "take the ouputs of the previous stage register for processing, do the processing
// and store any result in the latch input of the registers and then do the tick to make the results visible".
//
//------------------------------------------------------------------------------------------------------------
void CpuCore::clockStep( uint32_t numOfSteps) {
 
    while ( numOfSteps > 0 ) {
      
        iTlb        -> process( );
        dTlb        -> process( );
        iCacheL1    -> process( );
        dCacheL1    -> process( );
        if ( uCacheL2 != nullptr ) uCacheL2 -> process( );
        physMem         -> process( );
        
        fdStage     -> process( );
        maStage     -> process( );
        exStage     -> process( );
        
        handleTraps( );
    
        stReg.tick( );
        for ( uint8_t i = 0; i < 8; i++  ) gReg[ i ].tick( );
        for ( uint8_t i = 0; i < 8; i++  ) sReg[ i ].tick( );
        for ( uint8_t i = 0; i < 32; i++ ) cReg[ i ].tick( );
        
        fdStage     -> tick( );
        maStage     -> tick( );
        exStage     -> tick( );
        iTlb        -> tick( );
        dTlb        -> tick( );
        iCacheL1    -> tick( );
        dCacheL1    -> tick( );
        if ( uCacheL2 != nullptr ) uCacheL2 -> tick( );
        if ( pdcMem   != nullptr ) pdcMem -> tick( );
        if ( ioMem    != nullptr ) ioMem -> tick( );
        physMem         -> tick( );
    
        stats.clockCntr++;
        
        numOfSteps = numOfSteps - 1;
    }
}

//------------------------------------------------------------------------------------------------------------
// "instrStep" will perform a number of instruction. This is different from clock step in that a clock step
// is truly a clock step, while an instruction step can take a varying number of clock cycles, depending on
// events such as cache misses, etc. At instruction start we remember the instruction address and repeat
// issuing clock steps until the instruction address is about to change. In addition, we also maintain a
// cycle count to abort if we run off with an unreasonable high number of clocksteps.
//
// Note that this does not mean that the instruction completely worked through the pipeline. if all goes well,
// every clock a new instruction enters the pipeline and another one is leaving it. However, when we have
// pipeline stalls, they will be handled transparently when stepping though the instructions.
//
//------------------------------------------------------------------------------------------------------------
const uint32_t MAX_CYCLE_PER_INSTR = 100000; // catch a run-away...

void CpuCore::instrStep( uint32_t numOfInstr ) {
    
    uint32_t    previousIaSeg   = 0;
    uint32_t    previousIaOfs   = 0;
    uint32_t    cycleCount      = 0;
    uint32_t    totalCycleCount = 0;
    
    while ( numOfInstr > 0 ) {
        
        previousIaSeg = fdStage -> psInstrSeg.get( );
        previousIaOfs = fdStage -> psInstrOfs.get( );
        
        do {
            
            clockStep( 1 );
            cycleCount ++;
        }
        while (( cycleCount < MAX_CYCLE_PER_INSTR ) &&
               ( fdStage -> psInstrOfs.get( ) == previousIaOfs) &&
               ( fdStage -> psInstrSeg.get( ) == previousIaSeg ));
        
        stats.instrCntr++;
        
        numOfInstr      = numOfInstr - 1;
        totalCycleCount = totalCycleCount + cycleCount;
        cycleCount      = 0;
    }
}

//------------------------------------------------------------------------------------------------------------
// CPU register getter and setter functions used by the simulator user interface to display and modify the
// CPU programmer visible register set.
//
//------------------------------------------------------------------------------------------------------------
uint32_t CpuCore::getReg( RegClass regClass, uint8_t regNum ) {
    
    switch ( regClass ) {
            
        case RC_GEN_REG_SET:    return( gReg[ regNum % MAX_GREGS ].get( ) );
        case RC_SEG_REG_SET:    return( sReg[ regNum % MAX_SREGS ].get( ) );
        case RC_CTRL_REG_SET:   return( cReg[ regNum % MAX_CREGS ].get( ) );
            
        case RC_FD_PSTAGE:      return( fdStage -> getPipeLineReg( regNum ));
        case RC_MA_PSTAGE:      return( maStage -> getPipeLineReg( regNum ));
        case RC_EX_PSTAGE:      return( exStage -> getPipeLineReg( regNum ));
            
        case RC_IC_L1_OBJ:      return( iCacheL1 -> getMemCtrlReg( regNum ));
        case RC_DC_L1_OBJ:      return( dCacheL1 -> getMemCtrlReg( regNum ));
        case RC_UC_L2_OBJ:      return(( uCacheL2 != nullptr ) ? uCacheL2 -> getMemCtrlReg( regNum ) : 0 );
            
        case RC_ITLB_OBJ:       return( iTlb -> getTlbCtrlReg( regNum ));
        case RC_DTLB_OBJ:       return( dTlb -> getTlbCtrlReg( regNum ));
       
        case RC_PROG_STATE: {
            
            if      ( regNum == PS_REG_IA_SEG ) return( fdStage -> psInstrSeg.get( ));
            else if ( regNum == PS_REG_IA_OFS ) return( fdStage -> psInstrOfs.get( ));
            else if ( regNum == PS_REG_STATUS ) return( stReg.get( ));
            else return( 0 );
            
        } break;
            
        default: return( 0 );
    }
}

void CpuCore::setReg( RegClass regClass, uint8_t regNum, uint32_t val ) {
    
    switch ( regClass ) {
            
        case RC_GEN_REG_SET:    gReg[ regNum % MAX_GREGS ].load( val );     break;
        case RC_SEG_REG_SET:    sReg[ regNum % MAX_SREGS ].load( val );     break;
        case RC_CTRL_REG_SET:   cReg[ regNum % MAX_CREGS ].load( val );     break;
        
        case RC_FD_PSTAGE:      fdStage -> setPipeLineReg( regNum, val );   break;
        case RC_MA_PSTAGE:      maStage -> setPipeLineReg( regNum, val );   break;
        case RC_EX_PSTAGE:      exStage -> setPipeLineReg( regNum, val );   break;
            
        case RC_IC_L1_OBJ:      iCacheL1 -> setMemCtrlReg( regNum, val );   break;
        case RC_DC_L1_OBJ:      dCacheL1 -> setMemCtrlReg( regNum, val );   break;
        case RC_UC_L2_OBJ:      if ( uCacheL2 != nullptr ) iCacheL1 -> setMemCtrlReg( regNum, val ); break;
            
        case RC_ITLB_OBJ:       iTlb -> setTlbCtrlReg( regNum, val );   break;
        case RC_DTLB_OBJ:       dTlb -> setTlbCtrlReg( regNum, val );   break;
       
        case RC_PROG_STATE: {
        
            if      ( regNum == PS_REG_IA_SEG ) fdStage -> psInstrSeg.load( val );
            else if ( regNum == PS_REG_IA_OFS ) fdStage -> psInstrOfs.load( val );
            else if ( regNum == PS_REG_STATUS ) stReg.load( val );
            
        } break;
        
        default: ;
    }
}

// ??? put into the files that use it ... ?
//------------------------------------------------------------------------------------------------------------
// Some registers are subject to the privilege mode check of the execution thread. Any register can be read
// at any priviledge level. Beyond that, there are checks for write access.
//
//------------------------------------------------------------------------------------------------------------
bool CpuCore::isPrivRegForAccMode( RegClass regClass, uint32_t regId, AccessModes mode ) {
    
    switch ( regClass ) {
            
        case RC_GEN_REG_SET:    return( gReg[ regId % 8 ].isPrivReg( ));
        case RC_SEG_REG_SET:    return( sReg[ regId % 8 ].isPrivReg( ) && mode == ACC_READ_WRITE );
        case RC_CTRL_REG_SET:   return( cReg[ regId % 32 ].isPrivReg( ) && mode == ACC_READ_WRITE );
        
        default: return( true );
    }
}

//------------------------------------------------------------------------------------------------------------
// Trap handling. This routine is called after the processing of the EX pipeline stage. Any trap that occured
// in the pipeline will set the trap data in the control registers. The trapping instruction itself will
// be changed to a NOP and work its way through the pipelines as a NOP. Any trap that is caused by an earlier
// instruction will overwrite the trap data. In the end we will have the traps in the right order showing up
// after the EX stage. This scheme works because the stages are called in the order FD, MA and EX. For
// example: a trap in FD stage will set the data. At the same clock cycle the MA stage of the previous
// instruction will also raise a trap and just overwrite the trap data. And finally, suppose there is
// also a trap in the EX stage. Then the EX stage will just overwrite. Either case we end up with the trap
// to handle first. In other words we implement the precise trap handling model.
//
// The trap handler will first detect that there is a trap to handle. This is done by checking the trap ID.
// Next, the address of the trapped instruction is compared with the instruction address of the address in
// the EX stage. If they match, the trapping instruction has passed the EX stage. We compute the trap handler
// instruction address and set the IA registers of the FD pipeline register to it. Also, the program status
// word is cleared. We are now running in absolute mode and privileged with translation disabled. What is left
// to do is to flush the pipeline. The instructions that entered the pipeline after the trapping instruction
// are "bubbled" by setting the instruction field of the MA and EX stage to NOP. There could be the case the
// pipeline as stalled when the trap is detected in an instruction that still is ahead of the stall. Just in
// case, we resume all stages. Phew.
//
// Note: one day we may expand to handle external interrupts... this would follow the same logic.
//------------------------------------------------------------------------------------------------------------
void CpuCore::handleTraps( ) {
    
    if (( cReg[ CR_TEMP_1 ].get( ) != NO_TRAP ) &&
        ( cReg[ CR_TRAP_INSTR_SEG ].get( ) == exStage -> psInstrSeg.get( )) &&
        ( cReg[ CR_TRAP_INSTR_OFS ].get( ) == exStage -> psInstrOfs.get( ))) {
        
        uint32_t trapHandlerOfs = 0;
        
        if ( cReg[ CR_TEMP_1 ].get( ) < MAX_TRAP_ID ) {
            
            trapHandlerOfs = cReg[ CR_TRAP_VECTOR_ADR ].get( ) + cReg[ CR_TEMP_1 ].get( ) * TRAP_CODE_BLOCK_SIZE;
        }
        
        stReg.set( 0 );
        fdStage -> psInstrSeg.set( 0 );
        fdStage -> psInstrOfs.set( trapHandlerOfs );
        fdStage -> setStalled( false );
        maStage -> psInstr.set( 0 );  // ??? what to really set ...
        maStage -> setStalled ( false );
        exStage -> psInstr.set( 0 );  // ??? what to really set ...
        exStage -> setStalled( false );
    }
}
