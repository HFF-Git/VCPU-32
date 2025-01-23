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
// all other stages. We need this access for implementing stalling and bypassing capabilities. 
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
#include "VCPU32-Types.h"
#include "VCPU32-Core.h"

//------------------------------------------------------------------------------------------------------------
// File local declarations. There are constants and routines used internally and not visible outside of this
// file. Most of the routines are inline functions. They also could have been defined as C/C++ style defines,
// but this is a bit clearer to read.
//
//------------------------------------------------------------------------------------------------------------
namespace {

//‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐
// Bit field access.
////‐‐‐‐‐‐-----------------‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐
bool getBit( uint32_t arg, int pos ) {
    
    return(( arg & ( 1U << ( 31 - ( pos % 32 )))) ? 1 : 0 );
}

uint32_t getBitField( uint32_t arg, int pos, int len, bool sign = false ) {
    
    pos = pos % 32;
    len = len % 32;
    
    uint32_t tmpM = ( 1U << len ) - 1;
    uint32_t tmpA = arg >> ( 31 - pos );
    
    if ( sign ) return( tmpA | ( ~ tmpM ));
    else        return( tmpA & tmpM );
}

//‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐
// Two little helper functions to compare two register values for the CMP and CBR instruction.
////‐‐‐‐‐‐-----------------‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐
bool compareCond( uint32_t instr, uint32_t valA, uint32_t valB ) {
   
    switch( getBitField( instr, 8, 2 )) {
            
        case CC_EQ: return( valA == valB );
        case CC_NE: return( valA != valB );
        case CC_LT: return(((int32_t) valA )  < ((int32_t) valB ));
        case CC_LE: return(((int32_t) valA )  <= ((int32_t) valB ));
        default: return( false );
    }
}

bool compareCondU( uint32_t instr, uint32_t valA, uint32_t valB ) {
    
    switch( getBitField( instr, 8, 2 )) {
            
        case CC_EQ: return( valA == valB );
        case CC_NE: return( valA != valB );
        case CC_LT: return( valA  < valB );
        case CC_LE: return( valA <= valB );
        default: return( false );
    }
}

//‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐
// A little helper function to compare 0 to a register value for the CMR instruction.
//
//‐‐‐‐‐‐---------‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐
bool testCond( uint32_t instr, uint32_t val ) {
  
    switch ( getBitField( instr, 13, 4 )) {
            
        case CC_EQ: return( val == 0 );
        case CC_NE: return( val != 0 );
            
       // case CC_GT: return((int32_t) val >  0 );
        case CC_LT: return((int32_t) val <  0 );
        
        case CC_LE: return((int32_t) val <= 0 );
       // case CC_GE: return((int32_t) val >= 0 );
            
        // ??? to be completed ...
            
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
// Pipeline flush. When a trap occurs, the EX stage will branch to a trap handler. All instructions that
// entered the pipeline after the trapping instruction will need to be flushed. This is done by simply
// putting a NOP in the instruction fields of the OF pipeline register and our own pipeline register. This
// will overwrite whatever the previous stages execution have put there.
//
//------------------------------------------------------------------------------------------------------------
void ExecuteStage::flushPipeLine( ) {
    
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
// the OF stage and a new instruction is fetched to the FD stage at the next clock. The EX stage will just
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
        case PSTAGE_REG_ID_VAL_X:    psValX.load( val );         break;
    }
}

#if 0
//------------------------------------------------------------------------------------------------------------
// Some registers are subject to the privilege mode check of the execution thread. Any register can be read
// at any privilege level. Beyond that, there are checks for write access.
//
// put into the instruction execution that makes these checks....
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
// Execute Stage processing. This stage will primarily do the computational work using the "A" and "B" output
// from the OF stage. The computational result will be written back to the registers on the next "tick". For
// branch and link type instructions the ALU is used to compute the return address and store it into the
// specified general register. For the BE and BLE instruction GR0 and SR0 are used to save the return address.
//
// We need to pass the computation result to the FD and OF stage in case there is a RAW data hazard. For
// example, an instruction will store to R3 and the follow-on instruction is using R3. In this case the wrong
// value was read from the general register file. But we can use the "valR" value of the EX stage and just put
// it into the pipeline register of the FD stage, so that the OF stage will work with the expected value. This
// is generally called "bypassing".
//
// For the EX to FD case, it is simply a matter of patching the OF stage pipeline register where the FD stage
// put the old value of the register file. We check the instruction for fields that fetch a general and
// compare the register Id with the target register ID value of the current instruction in the EX stage. The
// data we correct is for the instruction two instructions behind the instruction that is currently finishing
// in the EX stage. In hardware you would have a path from the ALU output to the multiplexer before the input
// to the FD pipeline stage register.
//
// For the OF stage, we patch the EX stage pipeline for the "A" and "B" input. Like the previous scenario,
// this covers all the cases where the data is needed in the EX stage but of course not written back yet. All
// the cases where the OF stage would have computed an address with wrong general register values are handled
// by a pipeline stall in the FD stage.
//
// For the CBR conditional branch instruction, we need to evaluate the condition and then compare the result
// to the branch prediction decision taken in the FD stage. If we mis-predicted the pipeline needs to be
// flushed and instruction fetching continues from the alternate address passed forward through the pipeline
// "X" register.
//
// This routine will so far not cause a stall the pipeline but certainly it can trap. When a trap occurs, the
// pipeline is flushed and the procedure returns right away. This is consistent with the other stages. The
// "clockStep" method in the CPU core, which drives the stages, will actually check for traps and handle them.
//
// Some status bits must be bypassed in order for them to be available in the follow-on instructions. The
// ADD, ADDC, SUB and SUBC instructions for example generate a carry bit. This bit needs to be available
// in the follow-on computational instruction. In addition to the FD and OF state we also set our own
// pipeline processor state word accordingly.
//
// ??? what would we do about segment and control registers hazards, if at all ?
//
// Note: this is a rather long routine. Perhaps we should split this into smaller portions.
//------------------------------------------------------------------------------------------------------------
void ExecuteStage::process( ) {
    
    uint32_t            instr       = psInstr.get( );
    uint8_t             opCode      = getBitField( psInstr.get( ), 5, 6 );
   
    MemoryAccessStage   *maStage    = core -> maStage;
    FetchDecodeStage    *fdStage    = core -> fdStage;
   
    //--------------------------------------------------------------------------------------------------------
    // Assume we are not stalled.
    //
    //--------------------------------------------------------------------------------------------------------
    setStalled( false );
    
    //--------------------------------------------------------------------------------------------------------
    // Switch to the instruction and do the EX stage work. We will all the necessary work directly in the
    // respective instruction case.
    //
    //--------------------------------------------------------------------------------------------------------
    switch ( opCode ) {
            
        case OP_ADD:
        case OP_ADC: {
            
            if ( getBit( instr, 10 )) {
                
                uint64_t tmpU = (uint64_t) psValA.get( ) + psValB.get( );
                
                if ( opCode == OP_ADC ) {
                    
                    tmpU += ( psPstate0.getBit( ST_CARRY ) ? 1 : 0 );
                }
                
                bool tmpC = ( tmpU > UINT32_MAX );
                
                if ( getBit( instr, 11 )) {
                    
                    if ( tmpC ) {
                        
                        setupTrapData( OVERFLOW_TRAP, psPstate0.get( ), psPstate1.get( ), instr );
                        return;
                    }
                }
                
                core -> gReg[ getBitField( instr, 9, 4 ) ].set((uint32_t) tmpU );
                fdStage -> psPstate0.setBit( ST_CARRY, tmpC );
                maStage -> psPstate0.setBit( ST_CARRY, tmpC );
                psPstate0.setBit( ST_CARRY, tmpC );
            }
            else {
               
                int64_t tmpS = (int64_t) psValA.get( ) + psValB.get( );
                    
                if ( opCode == OP_ADC ) {
                        
                    tmpS += ( psPstate0.getBit( ST_CARRY ) ? 1 : 0 );
                }
                
                bool tmpC = ( tmpS > INT32_MAX ) || ( tmpS < INT32_MIN );
                    
                if ( getBit( instr, 11 )) {
                    
                    if ( tmpC ) {
                        
                        setupTrapData( OVERFLOW_TRAP, psPstate0.get( ), psPstate1.get( ), instr );
                        return;
                    }
                }
                    
                core -> gReg[ getBitField( instr, 9, 4 ) ].set((uint32_t) tmpS );
                fdStage -> psPstate0.setBit( ST_CARRY, tmpC );
                maStage -> psPstate0.setBit( ST_CARRY, tmpC );
                psPstate0.setBit( ST_CARRY, tmpC );
            }
            
        } break;
            
        case OP_ADDIL: {
            
            core -> gReg[ 1 ].set( psValA.get( ) + psValB.get( ));
            
        } break;
            
        case OP_AND: {
            
            uint32_t valR;
            
            if ( getBit( instr, 11 )) psValB.set( ~ psValB.get( ));
            valR = psValA.get( ) & psValB.get( );
            if ( getBit( instr, 10 )) valR = ~ valR;
            
            core -> gReg[ getBitField( instr, 9, 4 ) ].set( valR );
            
        } break;
            
        case OP_B: {
            
            core -> gReg[ getBitField( instr, 9, 4 ) ].set( psPstate1.get( ) + 4 );
            
        } break;
            
        case OP_BE: {
            
            core -> sReg[ 0 ].set( getBitField( psPstate0.get( ), 31, 16 ));
            core -> gReg[ getBitField( instr, 9, 4 ) ].set( psPstate1.get( ) + 4 );
            
        } break;
            
        case OP_BRK: {
            
            if (( getBitField( instr, 9, 4 ) != 0 ) || ( getBitField( instr, 31, 16 ) != 0 )) {
                
                setupTrapData( BREAK_TRAP, psPstate0.get( ), psPstate1.get( ), instr, psValA.get( ), psValB.get( ) );
                return;
            }
            
        } break;
            
        case OP_CBR:    case OP_CBRU: {
                 
             bool branchPedict  = getBit( instr, 23 );
             bool branchTaken   = false;;
             
             if ( opCode == OP_CBR ) branchTaken = compareCond( instr, psValA.get( ), psValB.get( ));
             else                    branchTaken = compareCondU( instr, psValA.get( ), psValB.get( ));
             
             if ( branchPedict != branchTaken ) {
                 
                 // ??? add branch address ?
                 
                 core -> fdStage -> psPstate0.set( psPstate0.get( ));
                 core -> fdStage -> psPstate1.set( psValX.get( ));
                 flushPipeLine( );
             }
             
         } break;
                 
        case OP_CMP: {
            
            uint32_t valR = (( compareCond( instr, psValA.get( ), psValB.get( ) )) ? 1 : 0 );
            core -> gReg[ getBitField( instr, 9, 4 ) ].set( valR );
            
        } break;
            
        case OP_CMPU: {
            
            uint32_t valR = (( compareCondU( instr, psValA.get( ), psValB.get( ) )) ? 1 : 0 );
            core -> gReg[ getBitField( instr, 9, 4 ) ].set( valR );
            
        } break;
            
        case OP_CMR: {
            
            uint32_t valR = (( compareCond( instr, psValA.get( ), psValB.get( ) )) ? 1 : 0 );
            core -> gReg[ getBitField( instr, 9, 4 ) ].set( valR );
            
            if ( testCond( instr, psValB.get( ))) {
                
                core -> gReg[ getBitField( instr, 9, 4 ) ].set( psValA.get( ));
            }
            
        } break;
            
        case OP_DEP: {
            
            uint8_t  depOpPos = getBitField( instr, 27, 5 );
            uint8_t  depOpLen = getBitField( instr, 21, 5 );
          
            if ( getBit( instr, 11 )) depOpPos = core -> cReg[ CR_SHIFT_AMOUNT ].getBitField( 31, 5 );
            
            if ( getBit( instr, 10 )) psValA.set( 0 );
            
            if ( getBit( instr, 12 )) psValA.setBitField( depOpPos, depOpLen, getBitField( instr, 31, 4 ));
            else                      psValA.setBitField( depOpPos, depOpLen, psValB.get( ));
            
            core -> gReg[ getBitField( instr, 9, 4 ) ].set( psValA.get( ));
          
        } break;
            
        case OP_DIAG: {
            
        } break;
            
        case OP_DS: {
            
            uint64_t tmp = ( psValA.get( ) << 1 ) & ( psPstate0.getBit( ST_CARRY ) ? 0x1 : 0 );
            
            if ( psPstate0.getBit( ST_DIVIDE_STEP )) {
                
                tmp = tmp - psValB.get( );
                core -> gReg[ getBitField( instr, 9, 4 ) ].set((uint32_t) tmp );
            }
            else {
                
                tmp = tmp + psValB.get( );
                core -> gReg[ getBitField( instr, 9, 4 ) ].set((uint32_t) tmp );
                
                if ( tmp > UINT32_MAX ) psPstate0.setBit( ST_CARRY );
                else                    psPstate0.clearBit( ST_CARRY );
            }
            
            psPstate0.setBit( ST_DIVIDE_STEP, psPstate0.getBit( ST_CARRY ) ^ psValB.getBit( 0 ));
            
        } break;
            
        case OP_DSR: {
            
            uint8_t  shAmtLen = getBitField( instr, 21, 5 );
            uint32_t valR;
            
            if ( getBit( instr, 11 )) shAmtLen =  core -> cReg[ CR_SHIFT_AMOUNT ].getBitField( 31, 5 );
            
            valR = (( psValA.get( ) >> shAmtLen ) | ( psValB.get( ) << ( WORD_SIZE - shAmtLen )));
            core -> gReg[ getBitField( instr, 9, 4 ) ].set( valR );
            
        } break;
            
         case OP_EXTR: {
             
             uint8_t  extrOpPos = getBitField( instr, 27, 5 );
             uint8_t  extrOpLen = getBitField( instr, 21, 5 );
             uint32_t valR;
             
             if ( getBit( instr, 11 )) extrOpPos = core -> cReg[ CR_SHIFT_AMOUNT ].getBitField( 31, 5 );
             
             valR = psValB.getBitField( extrOpPos, extrOpLen, getBit( instr, 10 ));
             core -> gReg[ getBitField( instr, 9, 4 ) ].set( valR );
             
         } break;
            
        case OP_GATE: {
            
            // ??? the offset was already executed in the previous stage, all we do here is to set the status bit
            // and return the former privilege status.
            
            core -> gReg[ getBitField( instr, 9, 4 ) ].set( psValB.get( )); // ??? check when we set R
            
        } break;
            
        case OP_ITLB: {
            
        } break;
            
        case OP_LD:
        case OP_LDA: {
          
            core -> gReg[ getBitField( instr, 9, 4 ) ].set( psValB.get( ));
            if ( getBit( instr, 11 ) && ( getBitField( instr, 9, 4 ) != getBitField( instr, 31, 4 )))
                core -> gReg[ getBitField( instr, 31, 4 ) ].set( psValX.get( ));
            
        } break;
            
        case OP_LDIL:
        case OP_LDO: {
            
            core -> gReg[ getBitField( instr, 9, 4 ) ].set( psValB.get( ));
            
        } break;
            
        case OP_LSID: {
            
            core -> gReg[ getBitField( instr, 9, 4 ) ].set( psValB.get( ));
          
        } break;
            
        case OP_MR: {
            
            if ( getBit( instr, 10 )) {
                
                if ( getBit( instr, 11 ))   core -> sReg[ getBitField( instr, 31, 4 ) ].set( psValB.get( ));
                else                        core -> cReg[ getBitField( instr, 31, 5  ) ].set( psValB.get( ));
                
            } else core -> gReg[ getBitField( instr, 9, 4 ) ].set( psValB.get( ));
            
        } break;
            
        case OP_MST: {
           
            switch ( getBitField( instr, 11, 2 )) {
                    
                case 0: fdStage -> psPstate0.setBitField( getBitField( instr, 31, 4 ), 15, 4 ); break;
                case 1: fdStage -> psPstate0.orBitField( psValB.getBitField( 31, 4 ), 15, 4 );  break;
                case 2: fdStage -> psPstate0.andBitField( psValB.getBitField( 31, 4 ), 15, 4 ); break;
                default: ;
            }
            
        } break;
            
        case OP_OR: {
            
            uint32_t valR;
            
            if ( getBit( instr, 11 )) psValB.set( ~ psValB.get( ));
            valR = psValA.get( ) | psValB.get( );
            if ( getBit( instr, 10 )) valR = ~ valR;
            
            core -> gReg[ getBitField( instr, 9, 4 ) ].set( valR );
            
        } break;
            
        case OP_PCA: {
      
        } break;
            
        case OP_PTLB: {
            
        } break;
         
        case OP_RFI: {
             
            core -> fdStage -> psPstate0.set( core -> cReg[ CR_TRAP_PSW_0 ].get( ));
            core -> fdStage -> psPstate1.set( core -> cReg[ CR_TRAP_PSW_1 ].get( ));
             
        } break;
            
        case OP_ST:
        case OP_STA:    {
            
            if ( getBit( instr, 11 )) core -> gReg[ getBitField( instr, 31, 4 ) ].set( psValX.get( ));
            
        } break;
            
        case OP_SHLA: {
            
            uint8_t  shAmt = getBitField( instr, 21, 2 );
            
            if ( getBit( instr, 12 )) {
                
                uint64_t tmpU = ( psValA.get( ) << shAmt ) + psValB.get( );
                
                if ( getBit( instr, 11 )) {
                    
                    if ( tmpU > UINT32_MAX ) {
                        
                        setupTrapData( OVERFLOW_TRAP, psPstate0.get( ), psPstate1.get( ), instr );
                        return;
                    }
                }
                
                core -> gReg[ getBitField( instr, 9, 4 ) ].set((uint32_t) tmpU );
            }
            else {
                
                int64_t tmpS = ((int64_t) ( psValA.get( ) << shAmt )) + ((int32_t) psValB.get( ));
                
                if ( getBit( instr, 11 )) {
                    
                    if (( tmpS < INT32_MIN ) || ( tmpS > INT32_MAX )) {
                        
                        setupTrapData( OVERFLOW_TRAP, psPstate0.get( ), psPstate1.get( ), instr );
                        return;
                    }
                }
                
                core -> gReg[ getBitField( instr, 9, 4 ) ].set((uint32_t) tmpS );
            }
         
        } break;
            
        case OP_SUB:
        case OP_SBC: {
            
            if ( getBit( instr, 10 )) {
                
                uint64_t tmpU = (uint64_t) psValA.get( ) - psValB.get( );
                
                if ( opCode == OP_SBC ) {
                    
                    tmpU -= ( getBit( psPstate0.get( ), ST_CARRY ) ? 1 : 0 );
                }
                
                bool tmpC =  ((int64_t) tmpU < 0 );
                
                if ( getBit( instr, 11 )) {
                    
                    if ( tmpC ) {
                        
                        setupTrapData( OVERFLOW_TRAP, psPstate0.get( ), psPstate1.get( ), instr );
                        return;
                    }
                }
                
                core -> gReg[ getBitField( instr, 9, 4 ) ].set((uint32_t) tmpU );
                fdStage -> psPstate0.setBit( ST_CARRY, tmpC );
                maStage -> psPstate0.setBit( ST_CARRY, tmpC );
                psPstate0.setBit( ST_CARRY, tmpC );
            }
            else {
                
                uint64_t tmpS = (int64_t) psValA.get( ) - psValB.get( );
                
                if ( opCode == OP_SBC ) {
                    
                    tmpS -= ( getBit( psPstate0.get( ), ST_CARRY ) ? 1 : 0 );
                }
                
                bool tmpC = ( tmpS > INT32_MAX ) || ( tmpS < INT32_MIN );
                
                if ( getBit( instr, 11 )) {
                    
                    if ( tmpC ) {
                        
                        setupTrapData( OVERFLOW_TRAP, psPstate0.get( ), psPstate1.get( ), instr );
                        return;
                    }
                }
                
                core -> gReg[ getBitField( instr, 9, 4 ) ].set((uint32_t) tmpS );
                fdStage -> psPstate0.setBit( ST_CARRY, tmpC );
                maStage -> psPstate0.setBit( ST_CARRY, tmpC );
                psPstate0.setBit( ST_CARRY, tmpC );
            }
            
        } break;
            
        case OP_XOR: {
            
            uint32_t valR;
            
            valR = psValA.get( ) ^ psValB.get( );
            if ( getBit( instr, 10 )) valR = ~ valR;
            
            core -> gReg[ getBitField( instr, 9, 4 ) ].set( valR );
            
        } break;
            
        default: {
            
            setupTrapData( ILLEGAL_INSTR_TRAP, psPstate0.get( ), psPstate1.get( ), instr );
            return;
        }
    }
    
    //--------------------------------------------------------------------------------------------------------
    // Bypass logic. We check the instruction currently in the FD or OF stage and "patch" the pipeline
    // register in the OF and EX stage if needed. Again, an instruction that would depend on computed
    // results in the OF stage, has been stalled already until we can reach it via a bypass.
    //
    //--------------------------------------------------------------------------------------------------------
    if ( opCodeTab[ opCode ].flags & REG_R_INSTR ) {
        
        FetchDecodeStage    *fdStage        = core -> fdStage;
       
        uint32_t            regIdForValR    = getBitField( instr, 9, 4 );
        uint32_t            valR            = core -> gReg[ regIdForValR ].getLatched( );
        
#if 0
        printf( "FD bypass: Instr: 0x%x -> R%d, Val: 0x%x\n", instr, regIdForValR, valR );
        printf( "FD - ValA: %d\n", fdStage -> dependencyValA( regIdForValR ));
        printf( "FD - ValB: %d\n", fdStage -> dependencyValB( regIdForValR ));
        printf( "FD - ValX: %d\n", fdStage -> dependencyValX( regIdForValR ));
        printf( "OF - ValA: %d\n", maStage -> dependencyValA( regIdForValR ));
        printf( "OF - ValB: %d\n", maStage -> dependencyValB( regIdForValR ));
#endif
        
        if ( fdStage -> dependencyValA( regIdForValR )) maStage -> psValA.set( valR );
        if ( fdStage -> dependencyValB( regIdForValR )) maStage -> psValB.set( valR );
        if ( fdStage -> dependencyValX( regIdForValR )) maStage -> psValX.set( valR );
        
        if ( maStage -> dependencyValA( regIdForValR )) psValA.set( valR );
        if ( maStage -> dependencyValB( regIdForValR )) psValB.set( valR );
    }
}
