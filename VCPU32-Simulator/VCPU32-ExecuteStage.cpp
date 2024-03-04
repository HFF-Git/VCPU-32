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

//‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐
// This methods does a sign extension for a 24bit word to 32bit so that the host CPU 32-bit arithmetic will
// produce the correct results.
//
//‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐
uint32_t signExt32( uint32_t arg ) {
    
    return(( arg & 0x00800000 ) ? ( arg | 0xFF000000 ) : ( arg & 0x00FFFFFF ));
}

//‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐
// A little helper function to compare two register values for the CBR instruction. This is a bit tricky as
// we run on a 32-bit machine. First, we sign extend to a 32-bt value and then do teh requested comparison.
// //‐‐‐‐‐‐uint32_t‐‐uint32_t‐‐‐uint32_t‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐
bool compareCond( uint32_t instr, uint32_t valA, uint32_t valB ) {
    
    int32_t tmpA = signExt32( valA );
    int32_t tmpB = signExt32( valB );
    
    switch( CPU24Instr::cbrCondField( instr )) {
            
        case CC_EQ: return( tmpA == tmpB );
        case CC_LT: return( tmpA < tmpB );
        case CC_GT: return( tmpA > tmpB );
        case CC_HI: return((uint32_t) tmpA > (uint32_t) tmpB );
        case CC_NE: return( tmpA != tmpB );
        case CC_LE: return( tmpA <= tmpB );
        case CC_GE: return( tmpA >= tmpB );
        case CC_LS: return( (uint32_t) valA < (uint32_t) tmpB );
        default: return( false );
    }
}

//‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐
// A little helper function to compare 0 to a register value for the TBR instruction. This is a bit tricky as
// we run on a 32-bit machine. First, we sign extend to a 32-bt value and then do teh requested comparison.
//
//‐‐‐‐‐‐---------‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐‐
bool testCond( uint32_t instr, uint32_t val ) {
    
    int32_t tmp = signExt32( val );
    
    switch ( CPU24Instr::tbrCondField( instr )) {
            
            
        case CC_EQ: return( tmp == 0 );
        case CC_GT: return( tmp > 0 );
        case CC_LT: return( tmp < 0 );
        case CC_NE: return( tmp != 0 );
        case CC_LE: return( tmp <= 0 );
        case CC_GE: return( tmp >= 0 );
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
    psInstrSeg.reset( );
    psInstrOfs.reset( );
    psInstr.reset( );
    psValA.reset( );
    psValB.reset( );
    psValX.reset( );
}

void ExecuteStage::tick( ) {
    
    if ( ! stalled ) {
        
        psInstrSeg.tick( );
        psInstrOfs.tick( );
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
    
    psInstrSeg.set( instrSeg );
    psInstrOfs.set( instrOfs );
    psInstr.set( OP_NOP );
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
                                  uint32_t iaSeg,
                                  uint32_t iaOfs,
                                  uint32_t pStat,
                                  uint32_t p1,
                                  uint32_t p2,
                                  uint32_t p3 ) {
    
    core -> cReg[ CR_TRAP_INSTR_SEG ].set( iaSeg );
    core -> cReg[ CR_TRAP_INSTR_OFS ].set( iaOfs );
    core -> cReg[ CR_TRAP_STAT ].set( pStat );
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
        case PSTAGE_REG_ID_IA_OFS:  return( psInstrOfs.get( ));
        case PSTAGE_REG_ID_IA_SEG:  return( psInstrSeg.get( ));
        case PSTAGE_REG_ID_INSTR:   return( psInstr.get( ));
        case PSTAGE_REG_ID_VAL_A:   return( psValA.get( ));
        case PSTAGE_REG_ID_VAL_B:   return( psValB.get( ));
        case PSTAGE_REG_ID_VAL_X:   return( psValX.get( ));
        default: return( 0 );
    }
}

void ExecuteStage::setPipeLineReg( uint32_t pReg, uint32_t val ) {
    
    switch ( pReg ) {
            
        case PSTAGE_REG_ID_IA_OFS:   psInstrOfs.load( val );        break;
        case PSTAGE_REG_ID_IA_SEG:   psInstrSeg.load( val );        break;
        case PSTAGE_REG_ID_INSTR:    psInstr.load( val );           break;
        case PSTAGE_REG_ID_VAL_A:    psValA.load( val );            break;
        case PSTAGE_REG_ID_VAL_B:    psValB.load( val );            break;
        case PSTAGE_REG_ID_VAL_X:    psValB.load( val );            break;
    }
}

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
// ??? what would we do about the segment and control registers hazards, if at all ?
// ??? note: this is a long routine. Perhaps we should split this into smaller portions.
//------------------------------------------------------------------------------------------------------------
void ExecuteStage::process( ) {
  
    instrSeg        = psInstrSeg.get( );
    instrOfs        = psInstrOfs.get( );
    instr           = psInstr.get( );
    valA            = psValA.get( );
    valB            = psValB.get( );
    valX            = psValX.get( );
    valR            = 0;
    regIdForValR    = CPU24Instr::regRIdField( instr );
    
    uint8_t opCode = CPU24Instr::opCodeField( psInstr.get( ));
    
    setStalled( false );
    
    //--------------------------------------------------------------------------------------------------------
    // Switch to the instruction and do the EX stage work.
    //
    //--------------------------------------------------------------------------------------------------------
    switch ( opCode ) {
            
        case OP_NOP: break;
            
        case OP_ADD: {
            
            valR = valA + valB;
            if (( CPU24Instr::useCarryField( instr )) && ( core -> stReg.get( ) & ST_CARRY )) valR ++;
          
            valCarry = ( valR > UINT32_MAX );
            
            if ( CPU24Instr::logicalOpField( instr )) {
                
                if (( valCarry ) && ( CPU24Instr::trapOvlField( instr ))) {
                    
                    setupTrapData( OVERFLOW_TRAP, instrSeg, instrOfs, core -> stReg.get( ), instr );
                    return;
                }
            }
            else {
                
                if ( CPU24Instr::trapOvlField( instr )) {
                    
                    if (( valR ^ valA ) & ( valR ^ valB ) & 0x00800000 ) {
                        
                        setupTrapData( OVERFLOW_TRAP, instrSeg, instrOfs, instr );
                        return;
                    }
                }
            }
           
        } break;
            
        case OP_SUB: {
            
            valR = valA + (( ~ valB & 0x00FFFFFf ) + 1 );
            if (( CPU24Instr::useCarryField( instr )) && ( core -> stReg.get( ) & ST_CARRY )) valR ++;
            
            valCarry = ( valR > UINT32_MAX );
            
            if ( CPU24Instr::logicalOpField( instr )) {
                
                if (( valCarry ) && ( CPU24Instr::trapOvlField( instr ))) {
                    
                    setupTrapData( OVERFLOW_TRAP, instrSeg, instrOfs, instr );
                    return;
                }
            }
            else {
                
                if ( CPU24Instr::trapOvlField( instr )) {
                    
                    if (( valR ^ valA ) & ( valR ^ valB ) & 0x00800000 ) {
                        
                        setupTrapData( OVERFLOW_TRAP, instrSeg, instrOfs, instr );
                        return;
                    }
                }
            }
            
        } break;
            
        case OP_AND: {
            
            if ( CPU24Instr::complRegBField( instr )) valB = ~ valB;
            valR = valA & valB;
            if ( CPU24Instr::negateResField( instr )) valR = ~ valR;
            
        } break;
            
        case OP_OR: {
            
            if ( CPU24Instr::complRegBField( instr )) valB = ~ valB;
            valR = valA | valB;
            if ( CPU24Instr::negateResField( instr )) valR = ~ valR;
            
        } break;
            
        case OP_XOR: {
            
            if ( CPU24Instr::complRegBField( instr )) valB = ~ valB;
            valR = valA ^ valB;
            if ( CPU24Instr::negateResField( instr )) valR = ~ valR;
            
        } break;
            
        case OP_CMP: {
            
            valR = (( compareCond( instr, valA, valB )) ? 1 : 0 );
          
        } break;
            
        case OP_CMR: {
            
            valR = (( compareCond( instr, valA, valB )) ? 1 : 0 );
          
        } break;
            
        case OP_EXTR: {
            
            uint8_t extrOpPos = CPU24Instr::extrDepPosField( instr );
            uint8_t extrOpLen = CPU24Instr::extrDepLenField( instr );
            
            // ??? check bit...
            if ( extrOpPos == WORD_SIZE ) core -> cReg[ CR_SHIFT_AMOUNT ].get( );
           
            uint32_t extrOpBitMask   = (( 1U << extrOpLen ) - 1 );
            valR = (( valB >> ( 31 - extrOpPos )) & extrOpBitMask );
            
            if ( CPU24Instr::extrSignedField( instr )) {
                
                if ( valR >> ( extrOpLen - 1 ) & 0x1 ) valR |= ( ~ extrOpBitMask );
            }
            
        } break;
            
        case OP_DEP: {
            
            uint8_t depOpPos  = CPU24Instr::extrDepPosField( instr );
            uint8_t depOpLen  = CPU24Instr::extrDepLenField( instr );
            
            // ??? check bit op...
            if ( depOpPos == WORD_SIZE ) core -> cReg[ CR_SHIFT_AMOUNT ].get( );
           
            uint32_t    depOpBitMask    = (( 1U << depOpLen ) - 1 );
            uint32_t    temp1           = ( valB & depOpBitMask ) << ( 31 - depOpPos );
            uint32_t    temp2           = ( valA & ( ~ ( depOpBitMask << ( 31 - depOpPos ))));
            
            valR  = (( CPU24Instr::depInZeroField( instr )) ? ( temp1 ) : ( temp1 | temp2 ));
            
        } break;
            
        case OP_DSR: {
            
            uint8_t shAmtLen = CPU24Instr::dsrSaField( instr );
            
            // ??? check big op ...
            if ( shAmtLen == WORD_SIZE ) core -> cReg[ CR_SHIFT_AMOUNT ].get( );
        
            // ??? fix ...
            valR = (( valA >> shAmtLen ) | ( WORD_SIZE - shAmtLen ));
            
        } break;
            
        case OP_SHLA: {
            
            uint8_t shAmt = CPU24Instr::shlaSaField( instr );
            
            if ( CPU24Instr::trapOvlField( instr )) {
                
                if ((( shAmt == 1 ) && ( valA & 000400000 )) ||
                    (( shAmt == 2 ) && ( valA & 000600000 )) ||
                    (( shAmt == 3 ) && ( valA & 000700000 ))) {
                    
                    setupTrapData( OVERFLOW_TRAP, instrSeg, instrOfs, instr );
                    return;
                }
            }
            
            valR = ( valA << shAmt ) + valB;
            
            if ( CPU24Instr::trapOvlField( instr )) {
                
                if (( valR ^ valA ) & ( valR ^ valB ) & 0x00800000 ) {
                    
                    setupTrapData( OVERFLOW_TRAP, instrSeg, instrOfs, instr );
                    return;
                }
            }
            
        } break;
            
        case OP_LDI: {
            
            if ( CPU24Instr::ldiZeroField( instr )) valA = 0;
            
            if ( CPU24Instr::ldiLeftField( instr )) valR = ( valA & 000007777 ) | ( valB << 12 );
            else                                    valR = ( valA & 077770000 ) | valB;
      
        } break;
    
        case OP_LD:
        case OP_LDWE:
        case OP_LDWA: {
           
            valR = valB;
           
        } break;
            
        case OP_BL:
        case OP_BLR: {
            
            valR = CPU24Instr::add22( instrOfs, 1 );
          
        } break;
            
        case OP_BLE: {
            
            core -> sReg[ 0 ].set( instrSeg );
            core -> gReg[ 0 ].set( instrOfs );
            
        } break;
            
        case OP_CBR: {
            
            bool branchTaken = CPU24Instr::immOfsSignField( instr );
            
            if (( compareCond( instr, valA, valB )) != ( branchTaken )) {
                
                core -> fdStage -> psInstrSeg.set( instrSeg );
                core -> fdStage -> psInstrOfs.set( valX );
                flushPipeLine( );
            }
            
        } break;
            
        case OP_TBR: {
            
            bool branchTaken = CPU24Instr::immOfsSignField( instr );
            
            if (( testCond( instr, valA )) != ( branchTaken )) {
                
                core -> fdStage -> psInstrSeg.set( instrSeg );
                core -> fdStage -> psInstrOfs.set( valX );
                flushPipeLine( );
            }
            
        } break;
            
        case OP_GATE: {
        
            valR = CPU24Instr::ofsSelect( valA ) | ( instrOfs & 047777777 );
            
        } break;
            
        case OP_MR: {
            
            if ( CPU24Instr::mrMovDirField( instr )) {
                
                if ( CPU24Instr::mrRegTypeField( instr )) 
                    core -> sReg[ CPU24Instr::regBIdField( instr ) ].set( valB );
                else
                    core -> cReg[ CPU24Instr::mrRegGrpField( instr ) * 8 + CPU24Instr::regBIdField( instr ) ].set( valB );
            }
            else valR = valB;
            
        } break;
            
        case OP_MST: {
            
            valR = core -> stReg.get( ) & 077;
            
            switch ( CPU24Instr::mstModeField( instr )) {
                    
                case 0: core -> stReg.set(( core -> stReg.get( ) & 00077777700 ) | ( valB & 077 ));
                case 1: core -> stReg.set( core -> stReg.get( ) | ( valB & 077 )); break;
                case 2: core -> stReg.set( core -> stReg.get( ) & (( ~ valB ) & 077 )); break;
                default: ;
            }
            
        } break;
            
        case OP_RFI: {
         
            core -> fdStage -> psInstrSeg.set( core -> cReg[ CR_TRAP_INSTR_SEG ].get( ));
            core -> fdStage -> psInstrOfs.set( core -> cReg[ CR_TRAP_INSTR_OFS ].get( ));
            core -> stReg.set( core -> cReg[ CR_TRAP_STAT ].get( ));
         
        } break;
            
        case OP_BRK: {
            
            setupTrapData( BREAK_TRAP, instrSeg, instrOfs, instr );
            return;
            
        } break;
            
        default: {
            
            setupTrapData( ILLEGAL_INSTR_TRAP, instrSeg, instrOfs, core -> stReg.get( ), instr );
            return;
        }
    }
    
    //--------------------------------------------------------------------------------------------------------
    // Conditional branch logic.
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
            
            if (( opCode == OP_ADD ) || ( opCode == OP_SUB )) {
                
                if ( valCarry ) core -> stReg.set( core -> stReg.get( ) | ST_CARRY );
                else  core -> stReg.set( core -> stReg.get( ) & ( ~ ST_CARRY ));
            }
        }
    }
    else if ( opCode == OP_MR ) {
        
        core -> gReg[ regIdForValR ].set( valR );
    }
    
    //--------------------------------------------------------------------------------------------------------
    // Bypass logic.
    //
    //--------------------------------------------------------------------------------------------------------
    FetchDecodeStage *fdStage   = core -> fdStage;
    MemoryAccessStage *maStage  = core -> maStage;
    
    if ( opCodeTab[ CPU24Instr::opCodeField( instr ) ].flags & REG_R_INSTR ) {
        
        if ( fdStage -> regIdForValA == regIdForValR ) maStage -> psValA.set( valR );
        if ( fdStage -> regIdForValB == regIdForValR ) maStage -> psValB.set( valR );
        if ( fdStage -> regIdForValX == regIdForValR ) maStage -> psValX.set( valR );
        if ( maStage -> regIdForValA == regIdForValR ) psValA.set( valR );
        if ( maStage -> regIdForValB == regIdForValR ) psValB.set( valR );
    }
}
