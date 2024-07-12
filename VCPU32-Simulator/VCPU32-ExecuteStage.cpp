//------------------------------------------------------------------------------------------------------------
//
// VCPU32 - A 32-bit CPU - Execute Stage
//
//------------------------------------------------------------------------------------------------------------
// The CPU24 execute stage class. We will model the instruction execution after the envisioned hardware
// pipeline stages. It will give us a good idea for a hardware design. Here is a sketch of a three
// stage pipeline:
//
//  FD  - instruction fetch and decode
//  DA  - memory access
//  EX  - execute
//
// This file contains the methods for the execute pipeline stage. Each stage is a structure with the
// pipeline register data and the methods to call for controlling the stages. Each stage also has access to
// all other stages. We need this access for implementing stalling and bypassing capabilities. There is a
// common include file, CPU24PipeLine.hpp, with all declarations of all stages.
//
//------------------------------------------------------------------------------------------------------------
//
// CPU32 - A 32-bit CPU - Execute Stage
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
// File local declarations. There are constants and routines used internally and not visible outside of this
// file. Most of the routines are inline functions. They also could have been defined as C/C++ style defines,
// but this is a bit clearer to read.
//
//------------------------------------------------------------------------------------------------------------
namespace {

bool getBit( uint32_t arg, int pos ) {
    
    return( arg & ( 1U << ( 31 - ( pos % 32 ))));
}

void setBit( uint32_t *arg, int pos ) {
   
    *arg |= ( 1U << ( 31 - ( pos % 32 )));
}

void clearBit( uint32_t *arg, int pos ) {
    
    *arg &= ~( 1U << ( 31 - ( pos % 32 )));
}

uint32_t getBitField( uint32_t arg, int pos, int len, bool sign = false ) {
    
    pos = pos % 32;
    len = len % 32;
    
    uint32_t tmpM = ( 1U << len ) - 1;
    uint32_t tmpA = arg >> ( 31 - pos );
    
    if ( sign ) return( tmpA | ( ~ tmpM ));
    else        return( tmpA & tmpM );
}

void setBitField( uint32_t *arg, int pos, int len, uint32_t val ) {
    
    pos = pos % 32;
    len = len % 32;
    
    uint32_t tmpM = ( 1U << len ) - 1;
    
    val = ( val & tmpM ) << ( 31 - pos );
    
    *arg = ( *arg & ( ~tmpM )) | val;
}

//‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐
// A little helper function to compare two register values for the CBR instruction. This is a bit tricky as
// we run on a 32-bit machine. First, we sign extend to a 32-bt value and then do teh requested comparison.
// //‐‐‐‐‐‐-----------------‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐
bool compareCond( uint32_t instr, uint32_t valA, uint32_t valB ) {
   
    switch( getBitField( instr, 8, 3 )) {
            
        case CC_EQ: return( valA == valB );
        case CC_LT: return(((int32_t) valA )  < ((int32_t) valB ));
        case CC_NE: return( valA != valB );
        case CC_LE: return(((int32_t) valA )  <= ((int32_t) valB ));
        default: return( false );
    }
}

bool compareCondU( uint32_t instr, uint32_t valA, uint32_t valB ) {
    
    switch( getBitField( instr, 8, 3 )) {
            
        case CC_EQ: return( valA == valB );
        case CC_LT: return( valA  < valB );
        case CC_NE: return( valA != valB );
        case CC_LE: return( valA <= valB );
        default: return( false );
    }
}

//‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐
// A little helper function to compare 0 to a register value for the TBR instruction. This is a bit tricky as
// we run on a 32-bit machine. First, we sign extend to a 32-bt value and then do teh requested comparison.
//
//‐‐‐‐‐‐---------‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐
bool testCond( uint32_t instr, uint32_t val ) {
  
    switch ( getBitField( instr, 8, 3 )) {
            
        case CC_EQ: return( val == 0 );
        case CC_GT: return( val >  0 );
        case CC_LT: return( val <  0 );
        case CC_NE: return( val != 0 );
        case CC_LE: return( val <= 0 );
        case CC_GE: return( val >= 0 );
        default: return ( false );
    }
}

}; // namespace

//------------------------------------------------------------------------------------------------------------
// The execute stage is finally the stage where the work is done. The grand finale of an instruction. Inputs
// A and B from the previous stage are the inputs to the ALU operation. The result is written to the register
// files.
//
//------------------------------------------------------------------------------------------------------------
ExecuteStage::ExecuteStage( CpuCore *core ) {
    
    this -> core = core;
}

//------------------------------------------------------------------------------------------------------------
// "reset" and "tick" manage the pipeline register. A "tick" will only update the pipeline register when
// there is no stall.
//
//------------------------------------------------------------------------------------------------------------
void ExecuteStage::reset( )  {
    
    stalled = false;
    psPstate0.reset( );
    psPstate1.reset( );
    psInstr.reset( );
    psValA.reset( );
    psValB.reset( );
    psValX.reset( );
}

void ExecuteStage::tick( ) {
    
    if ( ! stalled ) {
        
        psPstate0.tick( );
        psPstate1.tick( );
        psInstr.tick( );
        psValA.tick( );
        psValB.tick( );
        psValX.tick( );
    }
}

//------------------------------------------------------------------------------------------------------------
// Pipeline stall and resume. Currently, the EX stage does not  stall the pipeline. But perhaps one day we
// have instructions that will take longer than a clock cycle and then the EX stage will also need to stall.
//
//------------------------------------------------------------------------------------------------------------
void ExecuteStage::stallPipeLine( ) {
    
    setStalled( true );
    core -> maStage -> setStalled( true );
    core -> fdStage -> setStalled( true );
}

bool ExecuteStage::isStalled( ) {
    
    return( stalled );
}

void ExecuteStage::setStalled( bool arg ) {
    
    stalled = arg;
}

//------------------------------------------------------------------------------------------------------------
// Pipeline flush. When a trap occured, the EX stage will branch to a trap handler. All instructions that
// entered the pipeline after the trapping instruction will need to be flushed. This is done by simply
// putting a NOP in the instruction fields of the MA pipeline register and our own pipeline register. This
// will overwrite whatever the previous stages execution have put there.
//
//------------------------------------------------------------------------------------------------------------
void ExecuteStage::flushPipeLine( ) {
    
    psPstate0.set( instrPsw0 );
    psPstate1.set( instrPsw1 );
    psInstr.set( NOP_INSTR );
    psValA.set( 0 );
    psValB.set( 0 );
    psValX.set( 0 );
    core -> maStage -> flushPipeLine( );
}

//------------------------------------------------------------------------------------------------------------
// When a trap is encountered at the EX stage, the setup method will record the current instruction
// address and any additional data for the trap handler. The "TMP-1" control register contains the trapId
// value.
//
// Note that we do not do anything else. The next instruction following the trapping instruction will enter
// the MA stage and a new instruction is fetched to the FD stage at the next clock. The EX stage will just
// return and the CPU24 core will analyze the trap and flush the pipeline.
//------------------------------------------------------------------------------------------------------------
void ExecuteStage::setupTrapData( uint32_t trapId,
                                 uint32_t psw0,
                                 uint32_t psw1,
                                 uint32_t p1,
                                 uint32_t p2,
                                 uint32_t p3 ) {
    
    core -> cReg[ CR_TRAP_PSW_0 ].set( psw0 );
    core -> cReg[ CR_TRAP_PSW_1 ].set( psw1 );
    core -> cReg[ CR_TRAP_PARM_1 ].set( p1 );
    core -> cReg[ CR_TRAP_PARM_2 ].set( p2 );
    core -> cReg[ CR_TRAP_PARM_3 ].set( p3 );
    core -> cReg[ CR_TEMP_1 ].set( trapId );
}

//------------------------------------------------------------------------------------------------------------
// Utility function to set and get the the pipeline register data.
//
//--------------------------------------------------------------------------------------------------
uint32_t ExecuteStage::getPipeLineReg( uint32_t pReg ) {
    
    switch ( pReg ) {
            
        case PSTAGE_REG_STALLED:    return( stalled ? 1 : 0 );
        case PSTAGE_REG_ID_PSW_0:   return( psPstate0.get( ));
        case PSTAGE_REG_ID_PSW_1:   return( psPstate1.get( ));
        case PSTAGE_REG_ID_INSTR:   return( psInstr.get( ));
        case PSTAGE_REG_ID_VAL_A:   return( psValA.get( ));
        case PSTAGE_REG_ID_VAL_B:   return( psValB.get( ));
        case PSTAGE_REG_ID_VAL_X:   return( psValX.get( ));
        default: return( 0 );
    }
}

void ExecuteStage::setPipeLineReg( uint32_t pReg, uint32_t val ) {
    
    switch ( pReg ) {
            
        case PSTAGE_REG_ID_PSW_0:    psPstate0.load( val );      break;
        case PSTAGE_REG_ID_PSW_1:    psPstate1.load( val );      break;
        case PSTAGE_REG_ID_INSTR:    psInstr.load( val );        break;
        case PSTAGE_REG_ID_VAL_A:    psValA.load( val );         break;
        case PSTAGE_REG_ID_VAL_B:    psValB.load( val );         break;
        case PSTAGE_REG_ID_VAL_X:    psValB.load( val );         break;
    }
}

#if 0
//------------------------------------------------------------------------------------------------------------
// Some registers are subject to the privilege mode check of the execution thread. Any register can be read
// at any priviledge level. Beyond that, there are checks for write access.
//
// put into the instrucution execution that makes these chccks....
//------------------------------------------------------------------------------------------------------------
bool CpuCore::isPrivRegForAccMode( RegClass regClass, uint32_t regId, AccessModes mode ) {
    
    switch ( regClass ) {
            
        case RC_GEN_REG_SET:    return( gReg[ regId % 8 ].isPrivReg( ));
        case RC_SEG_REG_SET:    return( sReg[ regId % 8 ].isPrivReg( ) && mode == ACC_READ_WRITE );
        case RC_CTRL_REG_SET:   return( cReg[ regId % 32 ].isPrivReg( ) && mode == ACC_READ_WRITE );
        
        default: return( true );
    }
}
#endif

//------------------------------------------------------------------------------------------------------------
// Execute Stage processing. This stage will primarily do the computational work using the A and B output from
// the MA stage. The computational result will be written back to the registers on the next "tick". For branch
// and link type instructions the ALU is used to compute the return address and store it into the specified
// general register. For the BE and BLE instruction GR0 and SR0 are used to save the return address.
//
// We need to pass the computation result to the FD and MA stage in case there is a RAW data hazard. This
// stage will only set the target register ID for the result of the computation. For example, an instruction
// will store to R3 and the follow-on instruction is using R3. In this case the wrong value was read from the
// general register file. But we can use the "valR" value of the EX stage and just put it into the pipeline
// register of the FD stage, so that the MA stage will work with the expected value. This is generally called
// "bypassing".
//
// For the EX to FD case, it is a matter of patching the MA stage pipeline register where the FD stage put
// the old value of the register file. We check for A and for B if the FD stage indicates that it used the
// regID "R" of the EX stage in "A" or "B" in the FD stage. The data we correct is for the instruction two
// instructions behind the instruction that is currently finishing in the EX stage. In hardware you would
// have a path from the ALU output to the multipleyer before the input to the FD pipeline stage register.
//
// For the MA stage, we patch the EX stage pipeline for the A and B input. Like the previous scenario, this
// covers all the cases where the data is needed in the EX stage but of course not written back yet. All
// the cases where the MA stage would have computed with wroing values, are handdled by a pipeline stall
// in the FD stage.
//
// Note that this patching is done BEFORE we update the pipeline registers. The register Ids to be used
// are not written to the FD or MA stage yet. We therefore directly use the registerId fields from these
// stages to do our work here. The MA stage receives the register IDs via the pipeline register update and
// also stores the in local fields to be examined for hazard detection.
//
// For the TBR and CBR conditional branch instructions, we need to evaluate the condition and then compare
// the result to the branch prediction decision taken in the FD stage. If we mispredicted the pipeline needs
// to be flushed and instruction fetching continues from the alternate address passed forward through the
// X register.
//
// This routine will so far not stall the pipeline but certainly it can trap. When a trap occurs, the pipeline
// is flushed and the procedure returns rightaway. This is consistent with the other stages. The "clockStep"
// method in the CPU core, which drives the stages, will actually check for traps and handle them.
//
//
// ??? what would we do about segment and control registers hazards, if at all ?
// ??? note: this is a long routine. Perhaps we should split this into smaller portions.
//------------------------------------------------------------------------------------------------------------
void ExecuteStage::process( ) {
    
    instrPsw0       = psPstate0.get( );
    instrPsw1       = psPstate1.get( );
    instr           = psInstr.get( );
    valA            = psValA.get( );
    valB            = psValB.get( );
    valX            = psValX.get( );
    valR            = 0;
    regIdForValR    = MAX_GREGS;
    
    uint8_t opCode  = getBitField( psInstr.get( ), 5, 6 );
    
    setStalled( false );
    
    //--------------------------------------------------------------------------------------------------------
    // Switch to the instruction and do the EX stage work.
    //
    //--------------------------------------------------------------------------------------------------------
    switch ( opCode ) {
            
        case OP_ADD: {
            
            valR = valA + valB;
            if (( getBit( instr, 10 )) && ( getBit( psPstate0.get( ), ST_CARRY ))) valR ++;
            
            valCarry = ( UINT32_MAX - valA > valB );
            
            if ( getBit( instr, 11 )) {
                
                if (( valCarry ) && ( getBit( instr, 12 ))) {
                    
                    setupTrapData( OVERFLOW_TRAP, instrPsw0, instrPsw1, instr );
                    return;
                }
                
            } else {
                
                if ( getBit( instr, 12 )) {
                    
                    if (( valR ^ valA ) & ( valR ^ valB ) & 0x80000000 ) {
                        
                        setupTrapData( OVERFLOW_TRAP, instrPsw0, instrPsw1, instr );
                        return;
                    }
                }
            }
            
        } break;
            
        case OP_SUB: {
            
            valR = valA + ( ~ valB  ) + 1;
            if (( getBit( instr, 10 )) && ( getBit( psPstate0.get( ), ST_CARRY ))) valR ++;
            
            valCarry = ( valB > valA );
            
            if ( getBit( instr, 11 )) {
                
                if (( valCarry ) && ( getBit( instr, 12 ))) {
                    
                    setupTrapData( OVERFLOW_TRAP, instrPsw0, instrPsw1, instr );
                    return;
                }
                
            } else {
                
                if ( getBit( instr, 12 )) {
                    
                    if (( valR ^ valA ) & ( valR ^ valB ) & 0x80000000 ) {
                        
                        setupTrapData( OVERFLOW_TRAP, instrPsw0, instrPsw1, instr );
                        return;
                    }
                }
            }
            
        } break;
            
        case OP_AND: {
            
            if ( getBit( instr, 11 )) valB = ~ valB;
            valR = valA & valB;
            if ( getBit( instr, 10 )) valR = ~ valR;
            
        } break;
            
        case OP_OR: {
            
            if ( getBit( instr, 11 )) valB = ~ valB;
            valR = valA | valB;
            if ( getBit( instr, 10 )) valR = ~ valR;
            
        } break;
            
        case OP_XOR: {
            
            if ( getBit( instr, 11 )) valB = ~ valB;
            valR = valA ^ valB;
            if ( getBit( instr, 10 )) valR = ~ valR;
            
        } break;
            
        case OP_CMP: {
            
            valR = (( compareCond( instr, valA, valB )) ? 1 : 0 );
            
        } break;
            
        case OP_CMPU: {
            
            valR = (( compareCondU( instr, valA, valB )) ? 1 : 0 );
            
        } break;
            
        case OP_CMR: {
            
            valR = (( compareCond( instr, valA, valB )) ? 1 : 0 );
            
        } break;
            
        case OP_EXTR: {
            
            uint8_t extrOpPos = getBitField( instr, 27, 5 );
            uint8_t extrOpLen = getBitField( instr, 21, 5 );
            
            if ( getBit( instr, 11 )) extrOpPos = core -> cReg[ CR_SHIFT_AMOUNT ].get( );
            
            valR = getBitField( valB, extrOpPos, extrOpLen, getBit( instr, 10 ));
            
        } break;
            
        case OP_DEP: {
            
            uint8_t depOpPos = getBitField( instr, 27, 5 );
            uint8_t depOpLen = getBitField( instr, 21, 5 );
            
            if ( getBit( instr, 11 )) depOpPos = core -> cReg[ CR_SHIFT_AMOUNT ].get( );
            
            setBitField( &valA, depOpPos, depOpLen, valB );
            valR = valA;
            
        } break;
            
        case OP_DSR: {
            
            uint8_t shAmtLen = getBitField( instr, 21, 5 );
            
            if ( getBit( instr, 11 )) shAmtLen = core -> cReg[ CR_SHIFT_AMOUNT ].get( );
            
            valR = (( valA >> shAmtLen ) | ( valB << ( WORD_SIZE - shAmtLen )));
            
        } break;
            
        case OP_SHLA: {
            
            uint8_t shAmt = getBitField( instr, 21, 2 );
            
            if ( getBit( instr, 12 )) {
                
                if (( shAmt == 1 ) & (( valA & 0x80000000 ) != ( valA & 0x40000000 ))) {
                    
                    setupTrapData( OVERFLOW_TRAP, instrPsw0, instrPsw1, instr );
                    return;
                }
                
                else if (( shAmt == 2 ) & ((( valA & 0x80000000 ) != ( valA & 0x40000000 )) ||
                                           (( valA & 0x80000000 ) != ( valA & 0x20000000 )))) {
                    
                    setupTrapData( OVERFLOW_TRAP, instrPsw0, instrPsw1, instr );
                    return;
                }
                
                else if (( shAmt == 3 ) & ((( valA & 0x80000000 ) != ( valA & 0x40000000 )) ||
                                           (( valA & 0x80000000 ) != ( valA & 0x20000000 )) ||
                                           (( valA & 0x80000000 ) != ( valA & 0x10000000 )))) {
                    
                    setupTrapData( OVERFLOW_TRAP, instrPsw0, instrPsw1, instr );
                    return;
                }
            }
            
            valR = ( valA << shAmt ) + valB;
            
            if ( getBit( instr, 12 )) {
                
                if (( valR ^ valA ) & ( valR ^ valB ) & 0x80000000 ) {
                    
                    setupTrapData( OVERFLOW_TRAP, instrPsw0, instrPsw1, instr );
                    return;
                }
            }
            
        } break;
            
        case OP_LDIL: 
        case OP_LDO: 
        case OP_LD:
        case OP_LDA: {
            
            valR = valB;
            
        } break;
            
        case OP_ADDIL: {
            
            valR = valA + valB;
            core -> gReg[ 0 ].set( valR );
            
        } break;
            
        case OP_B: {
            
            valR = instrPsw1 + 4;
            
        } break;
            
        case OP_GATE: {
            
            // ??? the offset was already executed in the previous stage, all we do here is to set the status bit
            // and return the former privilege status.
            
            valR = valB; // ??? check when we set R
            
        } break;
            
        case OP_BE: {
            
            core -> sReg[ 0 ].set( getBitField( instrPsw0, 31, 16 ));
            valR = instrPsw1 + 4;
            
        } break;
            
        case OP_CBR: {
            
            if (( compareCond( instr, valA, valB )) != ( branchTaken )) {
                
                core -> fdStage -> psPstate0.set( instrPsw0 );
                core -> fdStage -> psPstate1.set( valX );
                flushPipeLine( );
            }
            
        } break;
            
        case OP_CBRU: {
            
            if (( compareCondU( instr, valA, valB )) != ( branchTaken )) {
                
                core -> fdStage -> psPstate0.set( instrPsw0 );
                core -> fdStage -> psPstate1.set( valX );
                flushPipeLine( );
            }
            
        } break;
            
        case OP_MR: {
            
            if ( getBit( instr, 11 )) {
                
                if ( getBit( instr, 12 ))   core -> sReg[ getBitField( instr, 31, 4 ) ].set( valB );
                else                        core -> cReg[ getBitField( instr, 31, 5  ) ].set( valB );
                
            } else valR = valB;
            
        } break;
            
        case OP_MST: {
            
            valR = getBitField( psPstate0.get( ), 15, 4 );
            
            /*  ??? FIX .....
            switch ( getBitField( instr, 11, 2 )) {
                    
                case 0: core -> stReg.set(( core -> stReg.get( ) & 0xFFFFFFC0 ) | ( valB & 0x3F ));
                case 1: core -> stReg.set( core -> stReg.get( ) | ( valB & 0x3F )); break;
                case 2: core -> stReg.set( core -> stReg.get( ) & (( ~ valB ) & 0x3F )); break;
                default: ;
            }
             */
            
        } break;
            
        case OP_ITLB: {
            
        } break;
            
        case OP_PTLB: {
            
        } break;
            
        case OP_PCA: {
            
            // ??? do the second part of the PCA ?
            
        } break;
            
        case OP_RFI: {
            
            core -> fdStage -> psPstate0.set( core -> cReg[ CR_TRAP_PSW_0 ].get( ));
            core -> fdStage -> psPstate1.set( core -> cReg[ CR_TRAP_PSW_1 ].get( ));
            
        } break;
            
        case OP_BRK: {
            
            if (( valA != 0 ) || ( valB != 0 )) {
                
                setupTrapData( BREAK_TRAP, instrPsw0, instrPsw1, instr );
                return;
            }
            
        } break;
            
        default: {
            
            setupTrapData( ILLEGAL_INSTR_TRAP, instrPsw0, instrPsw1, instr );
            return;
        }
    }
    
    //--------------------------------------------------------------------------------------------------------
    // Commit stage.
    //
    //--------------------------------------------------------------------------------------------------------
    if ( opCodeTab[ opCode ].flags & REG_R_INSTR ) {
        
        if ( opCode == OP_CMR ) {
            
            if ( testCond( instr, valB )) {
                
                core -> gReg[ regIdForValR ].set( valA );
            }
        }
        else {
            
            core -> gReg[ regIdForValR ].set( valR );
            
            if (( opCode == OP_ADD ) || ( opCode == OP_SUB )) setBit( &instrPsw1, ST_CARRY );
        }
    }
    else if ( opCode == OP_MR ) {
        
        core -> gReg[ regIdForValR ].set( valR );
    }
    
    //--------------------------------------------------------------------------------------------------------
    // Bypass logic.
    //
    // ??? what about the status bits ?
    //--------------------------------------------------------------------------------------------------------
    FetchDecodeStage *fdStage   = core -> fdStage;
    MemoryAccessStage *maStage  = core -> maStage;
    
    if ( opCodeTab[ opCode ].flags & REG_R_INSTR ) {
        
        if ( fdStage -> regIdForValA == regIdForValR ) maStage -> psValA.set( valR );
        if ( fdStage -> regIdForValB == regIdForValR ) maStage -> psValB.set( valR );
        if ( fdStage -> regIdForValX == regIdForValR ) maStage -> psValX.set( valR );
        if ( maStage -> regIdForValA == regIdForValR ) psValA.set( valR );
        if ( maStage -> regIdForValB == regIdForValR ) psValB.set( valR );
    }
}
