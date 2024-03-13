//------------------------------------------------------------------------------------------------------------
//
// VCPU32 - A 32-bit CPU - Fetch and Decode Stage
//
//------------------------------------------------------------------------------------------------------------
// The CPU24 instruction fetch and decode stage class. We will model the instruction execution after the
// envisioned hardware pipeline stages. It will give us a good idea for a hardware design. Here is a sketch
// of a three stage pipeline:
//
//  FD  - instruction fetch and decode
//  DA  - memory access
//  EX  - execute
//
// This file contains the methods for the fetch and decode pipeline stage. Each stage is a structure with
// the pipeline register data and the methods to call from the CPU24 core object for controlling the stages.
// Each stage also has access to all other stages. We need this access for implementing stalling and bypassing
// capabilities. There is a common include file, CPU24PipeLine.hpp, with all declarations of all stages.
//
//------------------------------------------------------------------------------------------------------------
//
// VCPU32 - A 32-bit CPU - Fetch and Decode Stage
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
// file. Most of the routines are inline functions. 
//
//------------------------------------------------------------------------------------------------------------
namespace {

//------------------------------------------------------------------------------------------------------------
// Instructions that will do computation in the MA stage may run into the case that the register content is
// just procuded by the preceding instruction. This value has not been written back and hence we cannot
// continue until the result is in reach via a simple bypass from the EX stage. This routine will test our
// instruction currently decoded for being one that depends on the latest register content. In general these
// are all instructions that do address arithmetic using the B and X pipeline fields, or the A field for some
// branch instructions.
//
//------------------------------------------------------------------------------------------------------------
bool maStageConsumesRegValBorX( uint32_t instr ) {
    
    uint8_t opCode = Instr::opCodeField( instr );
    uint8_t opMode = Instr::opModeField( instr );
    
    return (( opCode == OP_BR )     || ( opCode == OP_BLR )     ||
            ( opCode == OP_BV )     || ( opCode == OP_BVR )     ||
            ( opCode == OP_LDWA )   || ( opCode == OP_LDWE )    ||
            ( opCode == OP_LDHA )   || ( opCode == OP_LDHE )    ||
            ( opCode == OP_LDBA )   || ( opCode == OP_LDBE )    ||
            ( opCode == OP_STWA )   || ( opCode == OP_STWE )    ||
            ( opCode == OP_STHA )   || ( opCode == OP_STHE )    ||
            ( opCode == OP_STBA )   || ( opCode == OP_STBE )    ||
            ( opCode == OP_BE )     || ( opCode == OP_BLE )     ||
            ( opCode == OP_ITLB )   || ( opCode == OP_PTLB )    ||
            (( opCode == OP_PCA ))  ||
            (( opCodeTab[ opCode ].flags & OP_MODE_INSTR ) && ( opMode >= 8 ) && ( opMode <= 31 )));
}

}; // namespace

//------------------------------------------------------------------------------------------------------------
// The instruction fetch and decode stage object constructor.
//
//------------------------------------------------------------------------------------------------------------
FetchDecodeStage::FetchDecodeStage( CpuCore *core ) {
    
    this -> core = core;
}

//------------------------------------------------------------------------------------------------------------
// "reset" and "tick" manage the pipeline register. A "tick" will only update the pipeline register when
// there is no stall.
//
//------------------------------------------------------------------------------------------------------------
void FetchDecodeStage::reset( )  {
    
    stalled = false;
    psInstrSeg.reset( );
    psInstrOfs.reset( );
    
    psInstrSeg.load( 0 );
    psInstrOfs.load( 0xF0000000 );
}

void FetchDecodeStage::tick( ) {
    
    if ( ! stalled ) {
        
        psInstrSeg.tick( );
        psInstrOfs.tick( );
    }
}

//------------------------------------------------------------------------------------------------------------
// Pipeline stall and flush. "stallPipeline" stops ourselve from being updated and pass on a NOP to the next
// stage so that no erreneous things will be done.
//
//------------------------------------------------------------------------------------------------------------
void FetchDecodeStage::stallPipeLine( ) {
    
    setStalled( true );
    
    core -> maStage -> psInstrSeg.set( instrSeg );
    core -> maStage -> psInstrOfs.set( instrOfs );
    core -> maStage -> psInstr.set( NOP_INSTR );
    core -> maStage -> psRegIdForValA.set( MAX_GREGS );
    core -> maStage -> psRegIdForValB.set( MAX_GREGS );
    core -> maStage -> psRegIdForValX.set( MAX_GREGS );
    core -> maStage -> psValA.set( 0 );
    core -> maStage -> psValB.set( 0 );
    core -> maStage -> psValX.set( 0 );
}

bool FetchDecodeStage::isStalled( ) {
    
    return( stalled );
}

void FetchDecodeStage::setStalled( bool arg ) {
    
    stalled = arg;
}

//------------------------------------------------------------------------------------------------------------
// Utility function to set and get the the pipeline register data.
//
//------------------------------------------------------------------------------------------------------------
uint32_t FetchDecodeStage::getPipeLineReg( uint8_t pReg ) {
    
    switch ( pReg ) {
            
        case PSTAGE_REG_STALLED:     return( stalled ? 1 : 0 );
        case PSTAGE_REG_ID_IA_OFS:   return( psInstrOfs.get( ));
        case PSTAGE_REG_ID_IA_SEG:   return( psInstrSeg.get( ));
        default: return( 0 );
    }
}

void FetchDecodeStage::setPipeLineReg( uint8_t pReg, uint32_t val ) {
    
    switch ( pReg ) {
            
        case PSTAGE_REG_ID_IA_OFS:   psInstrOfs.load( val );  break;
        case PSTAGE_REG_ID_IA_SEG:   psInstrSeg.load( val );  break;
    }
}

//------------------------------------------------------------------------------------------------------------
// When a trap is encountered at the FD stage, the setup methods will record the current instruction
// address and any additional data for the trap handler. We also set the trap pending bit in the status
// register. The "TMP-1" control register contains the trapId value.
//
// A trap will cause execution from a new locaton which is the trap vector address. However, we cannot
// issue the trap rightaway as an instruction in the MA or EX before us may cause a trap. These traps will
// have to go first. ALl we do here is to just set the data. Since the pipeline stages are called in order
// a "later" stage may simple overwrite the trap data. This way, a trap from a previous instruction will be
// handled first. All trap handling takes place in the EX stage.
//
// Note that we do not do anything else. The next instruction following the trapping instruction will be
// fetched and enters the FD stage at the next clock. The EX stage will when encountering the trap simply
// flush the pipeline.
//------------------------------------------------------------------------------------------------------------
void FetchDecodeStage::setupTrapData( uint32_t trapId,
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
// Instruction fetch and decode stage processing. First, we get the current instruction address from the IA
// register. If code translation is enabled, the TLB needs to map the virtual address to a physical address.
// This can cause several traps and the TLB access itself can cause a stall of the pipeline depending on the
// TLB delay configuration value. If the entry is not found, a ITLB_MISS_TRAP is recorded. Next the access
// rights are validated. The access is a code page (execute) access with a sufficient priviledge level.
// Invalid access results in a ITLB_ACC_RIGHTS_TRAP. Finally, if protection checking is enabled, the TLB
// protection info is checked against the protection ID control registers. If there is no match, an
// ITLB_PROTECT_ID_TRAP is recorded. If all worked out, we have the physical address. Also, if code
// translation was disabled, the physical address is the offset part from the current instruction register.
//
// Next, the instruction cache or physical memory is accessed. If the data is not available yet, the pipeline
// is stalled. Due to the pipeline logic, the same instruction is repeated again and again, until the read
// request issued on each clockcycle completes successfully and we can get the instruction word.
//
// In general, the stall logic will always inhibit the update of the current and any previous pipeline reg
// and flush the next stage. Flushing is done by passing on a NOP instead of the current instruction.
// Initially, the processing assumes a non-stalled pipeline, a stall will set the flags. During the "ticks"
// executed, each time the stall is cleared but perhaps set again when the condition still exists.
//
// The instruction decode part tne analyzes the instruction. The imaginary instruction decode hardware is
// essentially a big combinatorial network to set the pipeline register fields A, B and X with known values
// that can be derived from the instruction fields or the general register set. The idea is also to perform
// computations that solely depend on the instruction word information right in this stage.
//
// Since the instructions have been crafted to place the fields with the same meaning most of the time in
// the same position, instruction decoding is just extracting selecting fields. Most computational 
// instructions are of the "R = R op B" form. In this case the "A" field also holds the "R"" information.
// We will in this case just copy the "R"" field register content to the "A" pipeline register. For the other
// non-regular fields an additional instruction specific decode is necessary.
//
// For instructions that use an adress, "B" is designated as the base register and "X" as the offset value.
// The address generation stage exepcts these two pipeline registers to have the appropriate values for any
// address computation. There is one instrcution, BVR, that requires to store a register content in X. In
// hardware not a big issue, as we propbaly will use 4-four way multiplexers and can afford to store
// a reg value in X. Likewise, there are cases where we store an immediate value in B.
//
// For the CBR and TBR instruction a static branch prediction scheme is implemented. A backward branch
// address is considered as a branch taken a positive address is considered as a branch not taken. When we
// actually evaluate the condition in teh EX stage, the branch decision needs to be corrected when
// mispredicted. For this to work, the decision and the alternative address offset for a wrong decision
// taken need to make it to the EX stage. The MA stage will actually create the branch target address using
// the instruction adrdess and the offset passed in X.
//
// For instructiosn that will do arithmetic in the MA stage, we will check if the previous instruction is
// an instruction that sets a general register. If the register matches one of our just fetched registers,
// we need to stall the pipeline for one cycle such that we can resolve this issue with a simple bypass
// when the result to store has been computed.
//
// Note that in the case of traps we only record the trap information. At this stage we cannot cause a trap
// because we do not know whether a previous instruction still in flight will cause a trap. Every potential
// trap encountered in a previous instruction will overwrite the trap data recorded by this instruction.
// This is not a problem, since our instruction here is stalled and any previous instruction continues to its
// end. If that instruction raises a trap, it will overwrite the trap info, so that in any case we end up with
// the actual trap to raise in the EX stage. Before fetching the next instruction in the EX stage the trap
// pending flag is checked and if set, we will set the next instruction address to that of the respective
// trap handler. When a trap occurs, the pipeline is stalled and the procedure returns rightaway.
//
// There are three fields for supporting the register bypass implementation. For each assignment of a general
// register to valA, valB or valX, we record the register number the value came from. At the EX stage where
// the register writeback is handled, this information is used to determine whether a bypass is required.
// Since general register numbers range fron 0 to N, a value greater than N is used to indicated that the
// valA, valB or valX field was not set from a general register. All this is rather easy in hardware, it is
// just multiplexers selecting, in software we need to make sure we overwrite the correct pipeline registers
// with the value produced in stage EX.
//
// ??? note: this is a long routine. Perhaps we should split this into smaller portions.
//------------------------------------------------------------------------------------------------------------
void FetchDecodeStage::process( ) {
    
    instrSeg        = psInstrSeg.get( );
    instrOfs        = psInstrOfs.get( );
    instr           = NOP_INSTR;
    instrPrivLevel  = 0;
    valA            = 0;
    valB            = 0;
    valX            = 0;
    regIdForValA    = MAX_GREGS;
    regIdForValB    = MAX_GREGS;
    regIdForValX    = MAX_GREGS;
    
    uint8_t     opCode          = 0;
    uint32_t    pAdr            = instrOfs;
    uint32_t    pOfs            = instrOfs & PAGE_BIT_MASK;
    TlbEntry    *tlbEntryPtr    = nullptr;
    bool        unCacheable     = false;
    
    setStalled( false );
    
    //--------------------------------------------------------------------------------------------------------
    // Instruction Address Translation.
    //
    //--------------------------------------------------------------------------------------------------------
    if ( core -> stReg.get( ) & ST_CODE_TRANSLATION_ENABLE ) {
        
        tlbEntryPtr = core -> iTlb -> lookupTlbEntry( instrSeg, instrOfs );
        if ( tlbEntryPtr == nullptr ) {
            
            setupTrapData( ITLB_MISS_TRAP, instrSeg, instrOfs, core -> stReg.get( ), instr );
            stallPipeLine( );
            return;
        }
        
        if ( tlbEntryPtr -> tPageType( ) != ACC_EXECUTE ) {
            
            setupTrapData( ITLB_ACC_RIGHTS_TRAP, instrSeg, instrOfs, core -> stReg.get( ), instr );
            stallPipeLine( );
            return;
        }
        
        if ( core -> stReg.get( ) & ST_PROTECT_ID_CHECK_ENABLE ) {
            
            if (( tlbEntryPtr -> tProtectId( ) != core -> cReg[ CR_PROTECT_ID1 ].get( )) &&
                ( tlbEntryPtr -> tProtectId( ) != core -> cReg[ CR_PROTECT_ID2 ].get( )) &&
                ( tlbEntryPtr -> tProtectId( ) != core -> cReg[ CR_PROTECT_ID3 ].get( )) &&
                ( tlbEntryPtr -> tProtectId( ) != core -> cReg[ CR_PROTECT_ID4 ].get( ))) {
                
                setupTrapData( ITLB_PROTECT_ID_TRAP, instrSeg, instrOfs, core -> stReg.get( ), instr );
                stallPipeLine( );
                return;
            }
        }
        
        unCacheable = tlbEntryPtr -> tUncachable( );
        pAdr        = tlbEntryPtr -> tPhysAdrTag( ) | pOfs;
    }
    
    //--------------------------------------------------------------------------------------------------------
    // Instruction word fetch.
    //
    // ??? we perhaps need to pass on uncachable too, as this needs to go through the layers...
    //--------------------------------------------------------------------------------------------------------
    if (( unCacheable ) || ( pAdr > 0xEFFFFFFF )) {
        
        if ( ! core -> mem -> readPhys( pAdr, 4, &instr )) {
            
            stallPipeLine( );
            return;
        }
        
    } else {
        
        if ( ! core -> iCacheL1 -> readVirt( instrSeg, instrOfs, 4, pAdr, &instr )) {
            
            stallPipeLine( );
            return;
        }
    }
    
    //--------------------------------------------------------------------------------------------------------
    // Instruction execution privilege check.
    //
    //--------------------------------------------------------------------------------------------------------
    if ( core -> stReg.get( ) & ST_CODE_TRANSLATION_ENABLE ) {
        
        instrPrivLevel  = core -> stReg.get( ) & ST_EXECUTION_LEVEL;
        
        if ( ! (( tlbEntryPtr -> tPrivL2( ) <= instrPrivLevel ) && 
                ( instrPrivLevel <= tlbEntryPtr -> tPrivL1( )))) {
                
            setupTrapData( INSTR_MEM_PROTECT_TRAP, instrSeg, instrOfs, core -> stReg.get( ));
            stallPipeLine( );
            return;
        }
    }
    
    //--------------------------------------------------------------------------------------------------------
    // Instruction Decode. Essentially a giant case statement.
    //
    //--------------------------------------------------------------------------------------------------------
    opCode = Instr::opCodeField( instr );

    switch ( opCode ) {
        
        case OP_ADD:    case OP_SUB:    case OP_AND:    case OP_OR:     case OP_XOR:
        case OP_CMP:    case OP_LD:     case OP_ST:     case OP_LDWR:   case OP_STWC:   
        case OP_LOD: {
            
            uint32_t opMode = Instr::opModeField( instr );
            
            if (( opMode < 8 ) && ( opCodeTab[ opCode ].flags & ( LOAD_INSTR | STORE_INSTR ))) {
                
                setupTrapData( ILLEGAL_INSTR_TRAP, instrSeg, instrOfs, core -> stReg.get( ), instr );
                return;
            }
            
            switch ( Instr::opModeField( instr )) {
                    
                case OP_MODE_IMM: {
                   
                    regIdForValA    = Instr::regRIdField( instr );
                    valA            = core -> gReg[ regIdForValA ].get( );
                    valB            = Instr::immGen0S14( instr );
                    
                } break;
                    
                case OP_MODE_NO_REGS: {
                   
                    valA = 0;
                    valB = 0;
                    
                } break;
                    
                case OP_MODE_REG_A: {
                    
                    regIdForValA    = Instr::regAIdField( instr );
                    valA            = core -> gReg[ regIdForValA ].get( );
                    
                } break;
                    
                case OP_MODE_REG_B: {
                    
                    regIdForValB    = Instr::regBIdField( instr );
                    valB            = core -> gReg[ regIdForValB ].get( );
                    
                } break;
                    
                case OP_MODE_REG_A_B: {
                    
                    regIdForValA    = Instr::regAIdField( instr );
                    regIdForValB    = Instr::regBIdField( instr );
                    valA            = core -> gReg[ regIdForValA ].get( );
                    valB            = core -> gReg[ regIdForValB ].get( );
                    
                } break;
                    
                case OP_MODE_EXT_INDX_W: case OP_MODE_EXT_INDX_H: case OP_MODE_EXT_INDX_B: {
                    
                    if ( opCodeTab[ opCode ].flags & STORE_INSTR ) {
                        
                        regIdForValA    = Instr::regRIdField( instr );
                        valA            = core -> gReg[ regIdForValA ].get( );
                    }
                    
                    regIdForValB    = Instr::regBIdField( instr );
                    valB            = core -> gReg[ regIdForValB ].get( );
                    valX            = Instr::immGen0S6( instr );
                    
                } break;
                    
                case OP_MODE_GR10_INDX_W: case OP_MODE_GR10_INDX_H: case OP_MODE_GR10_INDX_B:
                case OP_MODE_GR11_INDX_W: case OP_MODE_GR11_INDX_H: case OP_MODE_GR11_INDX_B:
                case OP_MODE_GR12_INDX_W: case OP_MODE_GR12_INDX_H: case OP_MODE_GR12_INDX_B:
                case OP_MODE_GR13_INDX_W: case OP_MODE_GR13_INDX_H: case OP_MODE_GR13_INDX_B:
                case OP_MODE_GR14_INDX_W: case OP_MODE_GR14_INDX_H: case OP_MODE_GR14_INDX_B:
                case OP_MODE_GR15_INDX_W: case OP_MODE_GR15_INDX_H: case OP_MODE_GR15_INDX_B: {
                    
                    if ( opCodeTab[ opCode ].flags & STORE_INSTR ) {
                        
                        regIdForValA    = Instr::regRIdField( instr );
                        valA            = core -> gReg[ regIdForValA ].get( );
                    }
                    
                    regIdForValB    = Instr::mapOpModeToIndexReg( Instr::opModeField( instr ));
                    valB            = core -> gReg[ regIdForValB ].get( );
                    valX            = Instr::immGen0S14( instr );
                    
                } break;
            }
            
        } break;
        
        case OP_LDIL:   {
            
            regIdForValA    = Instr::regRIdField( instr );
            valA            = Instr::immLeftField( instr ) << 10;
            valB            = 0;
            
        } break;
            
        case OP_ADDIL: {
            
            regIdForValA    = Instr::regRIdField( instr );
            valA            = core -> gReg[ regIdForValA ].get( );
            valB            = Instr::immLeftField( instr ) << 10;
            
        } break;
            
        case OP_EXTR: {
            
            regIdForValB    = Instr::regBIdField( instr );
            valB            = core -> gReg[ regIdForValB ].get( );
            
        } break;
            
        case OP_DEP: {
            
            regIdForValA    = Instr::regRIdField( instr );
            regIdForValB    = Instr::regBIdField( instr );
            valA            = core -> gReg[ regIdForValA ].get( );
            valB            = core -> gReg[ regIdForValB ].get( );
            
        } break;
            
        case OP_DSR: {
            
            regIdForValA    = Instr::regAIdField( instr );
            regIdForValB    = Instr::regBIdField( instr );
            valA            = core -> gReg[ regIdForValA ].get( );
            valB            = core -> gReg[ regIdForValB ].get( );
            
        } break;
            
        case OP_SHLA: {
            
            regIdForValA    = Instr::regAIdField( instr );
            regIdForValB    = Instr::regBIdField( instr );
            valA            = core -> gReg[ regIdForValA ].get( );
            valB            = core -> gReg[ regIdForValB ].get( );
            
        } break;
            
        case OP_CMR: {
            
            regIdForValB    = Instr::regBIdField( instr );
            valB            = core -> gReg[ regIdForValB ].get( );
            regIdForValA    = Instr::regRIdField( instr );
            valA            = core -> gReg[ regIdForValA ].get( );
        
        } break;
            
        case OP_LDWE: case OP_LDHE: case OP_LDBE:
        case OP_STWE: case OP_STHE: case OP_STBE: {
            
            if ( opCodeTab[ opCode ].flags & STORE_INSTR ) {
                
                regIdForValA    = Instr::regRIdField( instr );
                valA            = core -> gReg[ regIdForValA ].get( );
            }
            
            regIdForValB    = Instr::regBIdField( instr );
            valB            = core -> gReg[ regIdForValB ].get( );
            valX            = Instr::immGen8S6( instr );
            
        } break;
            
        case OP_LDWA: case OP_LDHA: case OP_LDBA:
        case OP_STWA: case OP_STHA: case OP_STBA: {
            
            if ( instrPrivLevel > 0 ) {
                
                setupTrapData( PRIV_OPERATION_TRAP, instrSeg, instrOfs, core -> stReg.get( ), instr );
                return;
            }
            
            if ( opCodeTab[ opCode ].flags & STORE_INSTR ) {
                
                regIdForValA    = Instr::regRIdField( instr );
                valA            = core -> gReg[ regIdForValA ].get( );
            }
            
            regIdForValB    = Instr::regBIdField( instr );
            valB            = core -> gReg[ regIdForValB ].get( );
            valX            = Instr::immGen8S10( instr );
            
        } break;
            
        case OP_PRB: {
            
            regIdForValB    = Instr::regBIdField( instr );
            valB            = core -> gReg[ regIdForValB ].get( );
            valX            = 0;
            
        } break;
            
        case OP_LDPA: {
            
            if ( instrPrivLevel > 0 ) {
                
                setupTrapData( PRIV_OPERATION_TRAP, instrSeg, instrOfs, core -> stReg.get( ), instr );
                return;
            }
            
            regIdForValB    = Instr::regBIdField( instr );
            valB            = core -> gReg[ regIdForValB ].get( );
            valX            = 0;
            
        } break;
            
        case OP_B:
        case OP_BL: {
            
            valB = instrOfs;
            valX = Instr::immGen8S14( instr );
            
        } break;
            
        case OP_BR:
        case OP_BLR: {
            
            regIdForValB    = Instr::regBIdField( instr );
            valB            = core -> gReg[ regIdForValB ].get( );
            valX            = instrOfs;
            
        } break;
            
        case OP_BV: {
            
            regIdForValB    = Instr::regBIdField( instr );
            valB            = core -> gReg[ regIdForValB ].get( );
            
        } break;
            
        case OP_BVR: {
            
            regIdForValB    = Instr::regBIdField( instr );
            valB            = core -> gReg[ regIdForValB ].get( );
            regIdForValX    = Instr::regAIdField( instr );
            valX            = core -> gReg[ regIdForValX ].get( );
            
        } break;
            
        case OP_BE:
        case OP_BLE: {
            
            regIdForValB    = Instr::regBIdField( instr );
            valB            = core -> gReg[ regIdForValB ].get( );
            valX            = Instr::immGen12S6( instr );
            
        } break;
            
        case OP_GATE: {
            
            regIdForValA    = Instr::regRIdField( instr );
            valA            = core -> gReg[ regIdForValA ].get( ); // ??? should set the previous priv level
            
            valB            = instrOfs;
            valX            = Instr::immGen8S14( instr );
            
            if ( core -> stReg.get( ) & ST_CODE_TRANSLATION_ENABLE ) {
                
                valB = instrOfs;
           
                if ( tlbEntryPtr -> tPageType( ) == 3 ) {
                    
                    if ( tlbEntryPtr -> tPrivL1( )) core -> stReg.set( core -> stReg.get( ) | ST_EXECUTION_LEVEL );
                    else core -> stReg.set( core -> stReg.get( ) & ( ~ ST_EXECUTION_LEVEL ));
                }
            }
            else core -> stReg.set( core -> stReg.get( ) | ST_EXECUTION_LEVEL );
                
        } break;
            
        case OP_CBR: {
            
            regIdForValA    = Instr::regAIdField( instr );
            regIdForValB    = Instr::regBIdField( instr );
            valA            = core -> gReg[ regIdForValA ].get( );
            valB            = core -> gReg[ regIdForValB ].get( );
            valX            = Instr::immGen8S6( instr );
            
        } break;
            
        case OP_TBR: {
            
            regIdForValB    = Instr::regBIdField( instr );
            valB            = core -> gReg[ regIdForValB ].get( );
            valX            = Instr::immGen8S6( instr );
            
        } break;
            
        case OP_MR: {
            
            if ( Instr::mrMovDirField( instr )) {
                
                regIdForValB    = Instr::regRIdField( instr );
                valB            = core -> gReg[ regIdForValB ].get( );
            }
            
        } break;
            
        case OP_MST: {
            
            if ( instrPrivLevel > 0 ) {
                
                setupTrapData( PRIV_OPERATION_TRAP, instrSeg, instrOfs, core -> stReg.get( ));
                return;
            }
            
            switch( Instr::mstModeField( instr )) {
                    
                case 0: {
                    
                    regIdForValB    = Instr::regBIdField( instr );
                    valB            = core -> gReg[ regIdForValB ].get( ) & 0x3C;
                    
                } break;
                    
                case 1:
                case 2: valB = instr & 0x3C; break;
                    
                default: {
                    
                    setupTrapData( ILLEGAL_INSTR_TRAP, instrSeg, instrOfs, core -> stReg.get( ));
                    return;
                }
            }
            
        } break;
            
        case OP_ITLB:
        case OP_PTLB: {
            
            if ( instrPrivLevel > 0 ) {
                
                setupTrapData( PRIV_OPERATION_TRAP, instrSeg, instrOfs, core -> stReg.get( ));
                return;
            }
            
            regIdForValA    = Instr::regRIdField( instr );
            regIdForValB    = Instr::regBIdField( instr );
            valA            = core -> gReg[ regIdForValA ].get( );
            valB            = core -> gReg[ regIdForValB ].get( );
            
        } break;
            
        case OP_PCA: {
            
            regIdForValB    = Instr::regBIdField( instr );
            valB            = core -> gReg[ regIdForValB ].get( );
            
        } break;
            
        case OP_RFI: {
            
            if ( instrPrivLevel > 0 ) {
                
                setupTrapData( PRIV_OPERATION_TRAP, instrSeg, instrOfs, core -> stReg.get( ));
                return;
            }
            
        } break;
            
        case OP_BRK: {
            
            valA = Instr::brkInfoField( instr );
         
        } break;
            
        default: {
            
            setupTrapData( ILLEGAL_INSTR_TRAP, instrSeg, instrOfs, core -> stReg.get( ), instr );
            return;
        }
    }
    
    //--------------------------------------------------------------------------------------------------------
    // Stall for instructions that will modify a general register that we depend on in the next stage.
    //
    //--------------------------------------------------------------------------------------------------------
    if ( opCodeTab[ Instr::opCodeField( core -> maStage -> psInstr.get( ))].flags & REG_R_INSTR ) {
        
        if ( maStageConsumesRegValBorX( instr )) {
            
            if (( Instr::regRIdField( core -> maStage -> psInstr.get( )) == regIdForValA ) ||
                ( Instr::regRIdField( core -> maStage -> psInstr.get( )) == regIdForValB ) ||
                ( Instr::regRIdField( core -> maStage -> psInstr.get( )) == regIdForValX )) {
                
                stallPipeLine( );
                return;
            }
        }
    }
    
    //--------------------------------------------------------------------------------------------------------
    // Pass the data to the MA stage pipeline.
    //
    //--------------------------------------------------------------------------------------------------------
    core -> maStage -> psInstrSeg.set( instrSeg );
    core -> maStage -> psInstrOfs.set( instrOfs );
    core -> maStage -> psInstr.set( instr );
    core -> maStage -> psValA.set( valA );
    core -> maStage -> psValB.set( valB );
    core -> maStage -> psValX.set( valX );
    core -> maStage -> psRegIdForValA.set( regIdForValA );
    core -> maStage -> psRegIdForValB.set( regIdForValB );
    core -> maStage -> psRegIdForValX.set( regIdForValX );
    
    //--------------------------------------------------------------------------------------------------------
    // Compute the next instruction address and for conditional branches pass on the alternative offset for
    // branch decision taken via the X reg. IN al other instructions cases we increment the instruction offset
    // register. The next instruction computation uses the instruction arithmetic routine which preserves the
    // upper two bits of the instruction offset word.
    //
    // ??? what exactly is the instruction offset arithmetic ?
    //--------------------------------------------------------------------------------------------------------
    psInstrSeg.set( instrSeg );
    
    if (( opCode == OP_CBR ) || ( opCode == OP_TBR )) {
        
        if ( Instr::immOfsSignField( instr )) {
            
            // ??? we add a signed value to an unsigned value .... 
            
            psInstrOfs.set( Instr::add32( instrOfs, Instr::immGen8S6( instr )));
            core -> maStage -> psValX.set( 1 );
        }
        else {
            
            psInstrOfs.set( instrOfs + 4 );
            core -> maStage -> psValX.set( Instr::immGen8S6( instr ));
        }
    }
    else psInstrOfs.set( instrOfs + 4 );
}
