//------------------------------------------------------------------------------------------------------------
//
// VCPU32 - A 32-bit CPU - Memory Access Stage
//
//------------------------------------------------------------------------------------------------------------
// The CPU24 memory access stage class. We will model the instruction execution after the envisioned
// hardware pipeline stages. It will give us a good idea for a hardware design. Here is a sketch of a three
// stage pipeline:
//
//  FD  - instruction fetch and decode
//  DA  - memory access
//  EX  - execute
//
// This file contains the methods for the memory access pipeline stage. Each stage is a structure with
// the pipeline register data and the methods to call from the CPU24 core object for controlling the stages.
// Each stage also has access to all other stages. We need this access for implementing stalling and bypassing
// capabilities. There is a common include file, CPU24PipeLine.hpp, with all declarations of all stages.
//
//------------------------------------------------------------------------------------------------------------
//
// VCPU32 - A 32-bit CPU - Memory Access Stage
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

}; // namespace

//------------------------------------------------------------------------------------------------------------
// The address generation and memory access stage object constructor.
//
//------------------------------------------------------------------------------------------------------------
MemoryAccessStage::MemoryAccessStage( CPU24Core *core ) {
    
    this -> core = core;
}

//------------------------------------------------------------------------------------------------------------
// "reset" and "tick" manage the pipeline register. A "tick" will only update the pipeline register when
// there is no stall.
//
//------------------------------------------------------------------------------------------------------------
void MemoryAccessStage::reset( )  {
    
    stalled = false;
    psInstrSeg.reset( );
    psInstrOfs.reset( );
    psInstr.reset( );
    psValA.reset( );
    psValB.reset( );
    psValX.reset( );
}

void MemoryAccessStage::tick( ) {
    
    if ( ! stalled ) {
        
        psInstrSeg.tick( );
        psInstrOfs.tick( );
        psInstr.tick( );
        psValA.tick( );
        psValB.tick( );
        psValX.tick( );
        psRegIdForValA.tick( );
        psRegIdForValB.tick( );
        psRegIdForValX.tick( );
    }
}

//------------------------------------------------------------------------------------------------------------
// Pipeline stall. "stallPipeline" stops ourselve from being updated and pass on a NOP to the next stage so
// that no erreneous things will be done. "ResumePipeline" will just enable the update again. At the MA stage
// we also have to make sure that the FD stage is also stalled. The same is true for resuming teh pipeline.
//
//------------------------------------------------------------------------------------------------------------
void MemoryAccessStage::stallPipeLine( ) {
    
    setStalled( true );
    core -> fdStage -> setStalled( true );
    
    core -> exStage -> psInstrSeg.set( instrSeg );
    core -> exStage -> psInstrOfs.set( instrOfs );
    core -> exStage -> psInstr.set( OP_NOP );
    core -> exStage -> psValA.set( 0 );
    core -> exStage -> psValB.set( 0 );
    core -> exStage -> psValX.set( 0 );
}

bool MemoryAccessStage::isStalled( ) {
    
    return( stalled );
}

void MemoryAccessStage::setStalled( bool arg ) {
    
    stalled = arg;
}

//------------------------------------------------------------------------------------------------------------
// Pipeline flush. The MA stage will need to handle a pipeline flush. When an unconditional branch is to be
// taken, the instruction fetched after the branch instruction needs to be flushed. This is simply done by
// putting a "NOP" into our pipeline register instruction field and thus overwrite what the FD stage might
// have put there for the next instruction.
//
// There is a small problem however. When the FD stage is already in a stall, the setting of a new instruction
// address will have no effect. Worse, our instruction that does the setting now moves through the pipeline and
// the setting will be lost. This is for example the case when we have a branch and the instruction following
// the branch will cause a cache miss. We nicely wait for the cache miss and in the meantime the branch
// instruction made its way through the pipeline and the setting of a new instruction address lost. When the
// cache miss resolves, the next instruction is simply the next one and the branch is lost. So, to avoid such
// ugly scenarios, flushing the pipeline also means to abort any cache of TLB work, and resuming the pipeline.
// This way, the next clock cycle will load the next correct instruction address as intended.
//
// I am not sure whether HW would also work this way, or we just have this issue because of the way the
// simulator works. To look into ...
//------------------------------------------------------------------------------------------------------------
void MemoryAccessStage::flushPipeLine( ) {
    
    psInstr.set( OP_NOP );
    psInstrSeg.set( instrSeg );
    psInstrOfs.set( instrOfs );
    psValA.set( 0 );
    psValB.set( 0 );
    psValX.set( 0 );
    psRegIdForValA.set( MAX_GREGS );
    psRegIdForValB.set( MAX_GREGS );
    psRegIdForValX.set( MAX_GREGS );
    
    if ( core -> fdStage -> isStalled( )) {
        
        core -> fdStage -> setStalled( false );
        core -> iCacheL1 -> abortMemOp( );
        core -> iTlb -> abortTlbOp( );
    }
}

//------------------------------------------------------------------------------------------------------------
// When a trap is encountered at the MA stage, the setup method will record the current instruction
// address and any additional data for the trap handler. The "TMP-1" control register contains the trapId
// value.
//
// A trap will cause execution from a new locaton which is the trap vector address. However, we cannot just
// issue the trap rightaway as an instruction in the EX stage before us may also cause a trap. These traps
// will have to go first. ALl we do here is to just set the data. Since the pipeline stages are called in
// order a "later" stage may simple overwrite the trap data. This way, a trap from a previous instruction
// will be handled first. All trap handling takes place in the EX stage.
//
// Note that we do not do anything else. The next instruction following the trapping instruction will enter
// the MA stage and a new instruction is fetched to the FD stage at the next clock. The EX stage will when
// encountering the trap simply flush the pipeline.
//------------------------------------------------------------------------------------------------------------
void MemoryAccessStage::setupTrapData( uint32_t     trapId,
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
//------------------------------------------------------------------------------------------------------------
uint32_t MemoryAccessStage::getPipeLineReg( uint8_t pReg ) {
    
    switch ( pReg ) {
            
        case PSTAGE_REG_STALLED:    return( stalled ? 1 : 0 );
        case PSTAGE_REG_ID_IA_OFS:  return( psInstrOfs.get( ));
        case PSTAGE_REG_ID_IA_SEG:  return( psInstrSeg.get( ));
        case PSTAGE_REG_ID_INSTR:   return( psInstr.get( ));
        case PSTAGE_REG_ID_VAL_A:   return( psValA.get( ));
        case PSTAGE_REG_ID_VAL_B:   return( psValB.get( ));
        case PSTAGE_REG_ID_VAL_X:   return( psValX.get( ));
        case PSTAGE_REG_ID_RID_A:   return( psRegIdForValA.get( ));
        case PSTAGE_REG_ID_RID_B:   return( psRegIdForValB.get( ));
        case PSTAGE_REG_ID_RID_X:   return( psRegIdForValX.get( ));
        default: return( 0 );
    }
}

void MemoryAccessStage::setPipeLineReg( uint8_t pReg, uint32_t val ) {
    
    switch ( pReg ) {
            
        case PSTAGE_REG_ID_IA_OFS:   psInstrOfs.load( val );        break;
        case PSTAGE_REG_ID_IA_SEG:   psInstrSeg.load( val );        break;
        case PSTAGE_REG_ID_INSTR:    psInstr.load( val );           break;
        case PSTAGE_REG_ID_VAL_A:    psValA.load( val );            break;
        case PSTAGE_REG_ID_VAL_B:    psValB.load( val );            break;
        case PSTAGE_REG_ID_RID_A:    psRegIdForValA.load( val );    break;
        case PSTAGE_REG_ID_RID_B:    psRegIdForValB.load( val );    break;
        case PSTAGE_REG_ID_RID_X:    psRegIdForValX.load( val );    break;
    }
}

//------------------------------------------------------------------------------------------------------------
// The memory access stage is primarily responsible for access memory data used by instructions that need
// to access memory. It will take the B and X value and compute the address offset for the memory data access.
// This the stage where any segment or control register is accessed. Depending on the addressing mode the
// offset calculation is a 22-bit or a 24-bit operation.
//
// In addition, unconditional branches are further processed at this stage. The target branch offset is
// computed using the adder of the MA stage. For branches that do not save a return link, we are done and
// the EX stage is "bubbled". Otherwise, the instruction continues to the execute stage where the return
// address is computed using the EX stage ALU and stored in a general register.
//
// For the conditional branch instruction, the predicted branch target based in the offset was already
// processed in the FD stage. Now, the branch target address for the alternative target will be computed by
// adding B and X, which were set accordingly in the FD stage. When the EX stage evaluates the branch
// condition and determines that the branch was mispredicted, this adress will be used to continue the
// instruction stream after flushing instructions fetched wrongly from the pipeline.
//
// In general, the stall logic will always inhibit the update of the current and any previous pipeline
// register as well as flush the next stage. Flushing is done by passing on a NOP instead of the current
// instruction. Initially, the processing assumes a non-stalled pipeline, a stall will set the flags. During
// the "ticks" executed, each time the stall is cleared but perhaps set again when the condition still exists.
//
// For instructions that access memory we will access memory data at this stage. OpMode 3.. 7 instructions
// and all memory load type instructions, the data fetched will be stored into B. This stage is also the
// starting point for a TLB or CACHE control instruction. If the TLB or Cache is busy, we stall in this stage
// until we have access to these resources. All other instructions just pass the pipeline data for A,B and
// X as well as the instruction address and instruction to the next stage.
//
// The MA stage is using valB and valX and valA, all of which may have been read from the general register
// file during the FD stage. Certain instruction sequences can cause a RAW data hazard. In the case of the
// FD stage, the EX stage will check its target register and may overwrite the valA and valB of the FD
// pipeline register. For the MA stage, the situation is a bit more tricky. The valA value is simply passed
// through and the EX stage can just correct the value before it reaches the EX stage for that instruction.
// The valB and valX pipeline field of the FD stage is however used in address computations that result in
// a new valB being passed to the EX stage. A simple bypass cannot bring back the changed values. We have
// to repeat the instruction MA stage with the correct data. For all these cases, the MA stage therefore
// needs to be stalled until the correct value is written back to the general register and then resumed.
// The FD stage passed the register IDs for A, B and X for this check. Note that the checking is actually
// done at the EX stage. For memory access type instrcutions, we need to clear the regIdForValB and
// regIdForValX fields, as the register content was consumed in this stage.
//
// Note: when a trap occurs, the pipeline is stalled and the procedure returns rightaway.
//
// ??? note: this is a rather long routine. Perhaps we should split this into smaller portions.
//------------------------------------------------------------------------------------------------------------
void MemoryAccessStage::process( ) {
    
    instrSeg        = psInstrSeg.get( );
    instrOfs        = psInstrOfs.get( );
    instr           = psInstr.get( );
    instrPrivLevel  = CPU24Instr::iaOfsPrivField( instrOfs );
    regIdForValA    = psRegIdForValA.get( );
    regIdForValB    = psRegIdForValB.get( );
    regIdForValX    = psRegIdForValX.get( );
    valA            = psValA.get( );
    valB            = psValB.get( );
    valX            = psValX.get( );
    valS            = 0;
    
    uint8_t         opCode      = CPU24Instr::opCodeField( instr );
    uint8_t         opMode      = CPU24Instr::operandModeField( instr );
    uint32_t    pOfs        = psValX.get( ) % PAGE_SIZE;
    uint32_t    pAdr        = 0;
    bool            unCacheable = false;
    
    bool readAccessInstr        = ((( opMode >= 4 ) &&
                                    (( opCode == OP_ADD ) || ( opCode == OP_SUB )    ||
                                     ( opCode == OP_CMP ) || ( opCode == OP_AND )    ||
                                     ( opCode == OP_OR )  || ( opCode == OP_XOR )    ||
                                     ( opCode ==     OP_LD )  || ( opCode == OP_LDWR )))   ||
                                   ( opCode == OP_LDWA ) || ( opCode == OP_LDWE ));
    
    bool writeAccessInstr       = (( opCode == OP_ST )  || ( opCode == OP_STWC ) ||
                                   ( opCode == OP_STWE ) || ( opCode == OP_STWA ));
    
    setStalled( false );
    
    //--------------------------------------------------------------------------------------------------------
    // Address computation or control instruction execution.
    //
    //--------------------------------------------------------------------------------------------------------
    switch( opCode ) {
            
        case OP_ADD:    case OP_SUB:    case OP_AND:    case OP_OR:     case OP_XOR:
        case OP_CMP:    case OP_LEA:    case     OP_LD:     case OP_ST:     case OP_LDWR:
        case OP_STWC: {
            
            switch( opMode ) {
                    
                case ADR_MODE_EXT_ADR: {
                    
                    valS            = core -> sReg[ CPU24Instr::regAIdField( instr ) ].get( );
                    valX            = CPU24Instr::add24( valB, valX );
                    regIdForValX    = MAX_GREGS;
                    regIdForValB    = MAX_GREGS;
                    
                } break;
                    
                case ADR_MODE_INDX_GR4:
                case ADR_MODE_INDX_GR5:
                case ADR_MODE_INDX_GR6:
                case ADR_MODE_INDX_GR7: {
                    
                    valS            = core -> sReg[ CPU24Instr::segSelect( valB ) ].get( );
                    valX            = CPU24Instr::add22( valB, valX );
                    regIdForValX    = MAX_GREGS;
                    regIdForValB    = MAX_GREGS;
                    
                } break;
                    
                default: ;
            }
            
        } break;
            
        case OP_LDWE:
        case OP_STWE: {
            
            valS            = core -> sReg[ CPU24Instr::regAIdField( instr ) ].get( );
            valX            = CPU24Instr::add24( valB, valX );
            regIdForValX    = MAX_GREGS;
            regIdForValB    = MAX_GREGS;
            
        } break;
            
        case OP_LDWA:
        case OP_STWA: {
            
            valX            = CPU24Instr::add24( valB, valX );
            regIdForValX    = MAX_GREGS;
            regIdForValB    = MAX_GREGS;
            
        } break;
            
        case OP_B:
        case OP_BL: {
            
            core -> fdStage -> psInstrSeg.set( instrSeg );
            core -> fdStage -> psInstrOfs.set( CPU24Instr::add22( valB, valX ));
            regIdForValX    = MAX_GREGS;
            regIdForValB    = MAX_GREGS;
            valB            = 0;
            valX            = 0;
            flushPipeLine( );
            
        } break;
            
        case OP_BR:
        case OP_BLR:
        case OP_BV:
        case OP_BVR: {
            
            core -> fdStage -> psInstrSeg.set( instrSeg );
            core -> fdStage -> psInstrOfs.set( CPU24Instr::add22( valB, valX ));
            regIdForValX    = MAX_GREGS;
            regIdForValB    = MAX_GREGS;
            valB            = 0;
            valX            = 0;
            flushPipeLine( );
            
        } break;
            
        case OP_BE:
        case OP_BLE: {
            
            core -> fdStage -> psInstrSeg.set( core -> sReg[ CPU24Instr::regAIdField( instr ) ].get( ));
            core -> fdStage -> psInstrOfs.set( CPU24Instr::add22( valB, valX ));
            regIdForValX    = MAX_GREGS;
            regIdForValB    = MAX_GREGS;
            valB            = 0;
            valX            = 0;
            flushPipeLine( );
            
        } break;
            
        case OP_CBR:
        case OP_TBR: {
            
            regIdForValX    = MAX_GREGS;
            valX            = CPU24Instr::add22( instrOfs, valX );
            
        } break;
            
        case OP_GATE: {
            
            // ??? work was done in FD stage, we need to pass on the former priv level result ?
            
            regIdForValX    = MAX_GREGS;
            regIdForValB    = MAX_GREGS;
            valB            = 0;
            valX            = 0;
            flushPipeLine( );
            
        } break;
            
        case OP_MR: {
            
            if ( ! CPU24Instr::mrMovDirField( instr )) {
                
                if ( CPU24Instr::mrRegTypeField( instr ))
                    valB = core -> cReg[ CPU24Instr::mrRegGrpField( instr ) * 8 + CPU24Instr::regBIdField( instr ) ].get( );
                else valB = core -> sReg[ CPU24Instr::regBIdField( instr ) ].get( );
            }
            
        } break;
            
        case OP_ITLB: {
            
            CPU24Tlb *tlbPtr    = ( CPU24Instr::tlbKindField( instr )) ? core -> dTlb : core -> iTlb;
            uint32_t tlbSeg = (( CPU24Instr::tlbAdrModeField( instr )) ?
                                   core -> sReg[ CPU24Instr::segSelect( valB )].get( ) :
                                   core -> sReg[ CPU24Instr::regAIdField( instr ) ].get( ));
            
            bool rStat = false;
            
            if ( CPU24Instr::tlbArgModeField( instr ))
                
                rStat = tlbPtr -> insertTlbEntryProt( tlbSeg, CPU24Instr::ofsSelect( valB ), valA );
            else
                rStat = tlbPtr -> insertTlbEntryAdr( tlbSeg, CPU24Instr::ofsSelect( valB ), valA );
            
            if ( ! rStat ) {
                
                stallPipeLine( );
                return;
            }
            
        } break;
            
        case OP_PTLB: {
            
            CPU24Tlb        *tlbPtr  = ( CPU24Instr::tlbKindField( instr )) ? core -> dTlb : core -> iTlb;
            uint32_t    tlbSeg = (( CPU24Instr::tlbAdrModeField( instr )) ?
                                      core -> sReg[ CPU24Instr::segSelect( valB )].get( ) :
                                      core -> sReg[ CPU24Instr::regAIdField( instr ) ].get( ));
            
            if ( ! tlbPtr -> purgeTlbEntry( tlbSeg, CPU24Instr::ofsSelect( valB ))) {
                
                stallPipeLine( );
                return;
            }
            
        } break;
            
        case OP_PCA: {
            
            CPU24Tlb        *tlbPtr = ( CPU24Instr::tlbKindField( instr )) ? core -> dTlb : core -> iTlb;
            CPU24Mem        *cPtr   = ( CPU24Instr::pcaKindField( instr )) ?  core -> dCacheL1 : core -> iCacheL1;
            uint32_t    seg     = 0;
            uint32_t    ofs     = 0;
            
            if ( CPU24Instr::pcaAdrModeField( instr )) {
                
                seg = core -> sReg[ CPU24Instr::segSelect( valB )].get( );
                ofs = CPU24Instr::ofsSelect( valB );
            }
            else {
                
                seg = core -> sReg[ CPU24Instr::regAIdField( instr ) ].get( );
                ofs = valB;
            }
            
            CPU24TlbEntry *tlbEntryPtr = tlbPtr -> lookupTlbEntry( seg, ofs );
            if ( tlbEntryPtr == nullptr ) {
                
                setupTrapData(( CPU24Instr::pcaKindField( instr ) ? ITLB_NON_ACCESS_TRAP : DTLB_NON_ACCESS_TRAP ),
                              seg, ofs, core -> stReg.get( ));
                flushPipeLine( );
                return;
            }
            
            if ( ! CPU24Instr::pcaPurgeFlushField( instr ))
                cPtr -> flushBlockVirt( seg, ofs, tlbEntryPtr -> tPhysAdrTag( ));
            else
                cPtr -> purgeBlockVirt( seg, ofs, tlbEntryPtr -> tPhysAdrTag( )); 
            
        } break;
            
        default: {
            
            valS = 0;
            valX = 0;
        }
    }
    
    //--------------------------------------------------------------------------------------------------------
    // Data load or store.
    //
    //--------------------------------------------------------------------------------------------------------
    if ( readAccessInstr || writeAccessInstr ) {
        
        if ( core -> stReg.get( ) & ST_DATA_TRANSLATION_ENABLE ) {
            
            CPU24TlbEntry   *tlbEntryPtr = core -> dTlb -> lookupTlbEntry( valS, valX );
            if ( tlbEntryPtr == nullptr ) {
                
                setupTrapData( DTLB_MISS_TRAP, valS, valX, core -> stReg.get( ));
                stallPipeLine( );
                return;
            }
            
            if ( readAccessInstr ) {
                
                if (( tlbEntryPtr -> tPageType( ) != ACC_READ_WRITE ) &&
                    ( tlbEntryPtr -> tPageType( ) != ACC_READ_ONLY )) {
                    
                    setupTrapData( DTLB_ACC_RIGHTS_TRAP, valS, valX, core -> stReg.get( ));
                    stallPipeLine( );
                    return;
                }
                
                if ( instrPrivLevel > tlbEntryPtr -> tPrivL1( )) {
                    
                    setupTrapData( DATA_MEM_PROTECT_TRAP, valS, valX, core -> stReg.get( ));
                    stallPipeLine( );
                    return;
                }
            }
            else if ( writeAccessInstr ) {
                
                if (( tlbEntryPtr -> tPageType( ) != ACC_READ_WRITE ) &&
                    (( tlbEntryPtr -> tPageType( ) == ACC_EXECUTE ) && ( ! tlbEntryPtr -> tModifyExPage( ))) &&
                    (( tlbEntryPtr -> tPageType( ) == ACC_GATEWAY ) && ( ! tlbEntryPtr -> tModifyExPage( )))) {
                    
                    setupTrapData( DTLB_ACC_RIGHTS_TRAP, valS, valX, core -> stReg.get( ));
                    stallPipeLine( );
                    return;
                }
                
                if ( instrPrivLevel > tlbEntryPtr -> tPrivL2( )) {
                    
                    setupTrapData( DATA_MEM_PROTECT_TRAP, valS, valX, core -> stReg.get( ));
                    stallPipeLine( );
                    return;
                }
            }
            
            if ( core -> stReg.get( ) & ST_PROTECT_ID_CHECK_ENABLE ) {
                
                if (( tlbEntryPtr -> tProtectId( ) != core -> cReg[ CR_PROTECT_ID1 ].get( )) &&
                    ( tlbEntryPtr -> tProtectId( ) != core -> cReg[ CR_PROTECT_ID2 ].get( )) &&
                    ( tlbEntryPtr -> tProtectId( ) != core -> cReg[ CR_PROTECT_ID3 ].get( )) &&
                    ( tlbEntryPtr -> tProtectId( ) != core -> cReg[ CR_PROTECT_ID4 ].get( ))) {
                    
                    setupTrapData( DTLB_PROTECT_ID_TRAP, valS, valX, core -> stReg.get( ));
                    stallPipeLine( );
                    return;
                }
            }
            
            unCacheable = tlbEntryPtr -> tUncachable( );
            pAdr        = tlbEntryPtr -> tPhysAdrTag( ) | pOfs;
        }
        else pAdr = valX;
       
        bool rStat = false;
       
        if ( unCacheable ) {
            
            if      ( readAccessInstr )     rStat = core -> mem -> readWordPhys( pAdr, &valB );
            else if ( writeAccessInstr )    rStat = core -> mem -> writeWordPhys( pAdr, valA );
        }
        else {
            
            if      ( readAccessInstr )     rStat = core -> dCacheL1 -> readWordVirt( valS, valX, pAdr, &valB );
            else if ( writeAccessInstr )    rStat = core -> dCacheL1 -> writeWordVirt( valS, valX, pAdr, valA );
        }
        
        if ( ! rStat ) {
            
            stallPipeLine( );
            return;
        }
    }
    
    //--------------------------------------------------------------------------------------------------------
    // Pass the data to the EX stage pipeline.
    //
    //--------------------------------------------------------------------------------------------------------
    core -> exStage -> psInstr.set( instr );
    core -> exStage -> psInstrOfs.set( instrOfs );
    core -> exStage -> psInstrSeg.set( instrSeg );
    core -> exStage -> psValA.set( valA );
    core -> exStage -> psValB.set( valB );
    core -> exStage -> psValX.set( valX );
}
