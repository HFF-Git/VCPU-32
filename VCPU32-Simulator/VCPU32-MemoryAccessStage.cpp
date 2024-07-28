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

bool getBit( uint32_t arg, int pos ) {
    
    return( arg & ( 1U << ( 31 - ( pos % 32 ))));
}

void setBit( uint32_t *arg, int pos ) {
    
    *arg |= ( 1U << ( 31 - ( pos % 32 )));
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

uint32_t add32( uint32_t arg1, uint32_t arg2 ) {
    
    return ( arg1 + arg2 );
}

uint32_t mapDataLen( uint32_t instr ) {
    
    switch( getBitField( instr, 15, 2 )) {
            
        case 0:     return( 1 );
        case 1:     return( 2 );
        case 2:     return( 4 );
        case 3:     return( 8 );
        default:    return( 0 );
    }
}

}; // namespace

//------------------------------------------------------------------------------------------------------------
// The address generation and memory access stage object constructor.
//
//------------------------------------------------------------------------------------------------------------
MemoryAccessStage::MemoryAccessStage( CpuCore *core ) {
    
    this -> core = core;
}

//------------------------------------------------------------------------------------------------------------
// "reset" and "tick" manage the pipeline register. A "tick" will only update the pipeline register when
// there is no stall.
//
//------------------------------------------------------------------------------------------------------------
void MemoryAccessStage::reset( )  {
    
    stalled = false;
    psPstate0.reset( );
    psPstate1.reset( );
    psInstr.reset( );
    psValA.reset( );
    psValB.reset( );
    psValX.reset( );
}

void MemoryAccessStage::tick( ) {
    
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
// Pipeline stall. "stallPipeline" stops ourselve from being updated and pass on a NOP to the next stage so
// that no erreneous things will be done. "ResumePipeline" will just enable the update again. At the MA stage
// we also have to make sure that the FD stage is also stalled. The same is true for resuming the pipeline.
//
//------------------------------------------------------------------------------------------------------------
void MemoryAccessStage::stallPipeLine( ) {
    
    setStalled( true );
    core -> fdStage -> setStalled( true );
    
    core -> exStage -> psPstate0.set( psPstate0.get( ));
    core -> exStage -> psPstate1.set( psPstate1.get( ));
    core -> exStage -> psInstr.set( NOP_INSTR );
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
// There is a small problem however. When the FD stage is already in a stall, the setting a new instruction
// address will have no effect. Worse, our instruction that does the setting now moves through the pipeline
// and the setting will be lost. This is for example the case when we have a branch and the instruction
// following the branch will cause a cache miss. We nicely wait for the cache miss and in the meantime the
// branch instruction made its way through the pipeline and the setting of a new instruction address is lost.
// When the cache miss resolves, the next instruction is simply the next one and the branch is lost. So, to
// avoid such ugly scenarios, flushing the pipeline also means to abort any cache of TLB work, and resuming
// the pipeline. This way, the next clock cycle will load the next correct instruction address as intended.
//
// I am not sure whether HW would also work this way, or we just have this issue because of the way the
// simulator works. To look into ...
//------------------------------------------------------------------------------------------------------------
void MemoryAccessStage::flushPipeLine( ) {
    
    psInstr.set( NOP_INSTR );
    psValA.set( 0 );
    psValB.set( 0 );
    psValX.set( 0 );
 
    if ( core -> fdStage -> isStalled( )) {
        
        core -> fdStage -> setStalled( false );
        core -> iCacheL1 -> abortOp( );
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
void MemoryAccessStage::setupTrapData( uint32_t trapId,
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
// "dependencyValA" checks if the instruction fetched a value from the general register file in the FD stage
// that we would just pass on to the EX stage. If that is the case, the execute stage will store its computed
// value to the pipeline register so that we have the correct value. Since nothing is changed on the "valA"
// data path, we just use the FD stage function.
//
//------------------------------------------------------------------------------------------------------------
bool MemoryAccessStage::dependencyValA( uint32_t instr, uint32_t regId ) {
    
    return( core -> fdStage -> dependencyValA( instr, regId ));
}

//------------------------------------------------------------------------------------------------------------
// "dependencyValA" checks if the instruction fetched a value from the general register file in the FD stage
// that we would just pass on to the EX stage. If that is the case, the execute stage will store its computed
// value to the pipeline register so that we have the correct value. Note that several instruction produce a
// new valB. For example, the load instruction will load a new value and store this to the "valB" pipeline
// register. We need only to cover the cases where a value was read in the FD stage and is intended to be
// passed on to the MA stage.
//
// There are also several instructions that uses the "valB" field and compute an address to be used for
// fetching data or branching. In that case we have to stall the pipeline to ensure that the address
// address computation is done with teh correct "valB" fetched from the general register file during the FD
// stage.
//
//------------------------------------------------------------------------------------------------------------
bool MemoryAccessStage::dependencyValB( uint32_t instr, uint32_t regId ) {
    
    switch ( getBitField( instr, 5, 6 )) {
            
        case OP_ADD:    case OP_ADC:    case OP_SUB:    case OP_SBC:    case OP_AND:    case OP_OR:
        case OP_XOR:    case OP_CMP:    case OP_CMPU: {
            
            uint32_t mode = getBitField( instr, 13, 2 );
            
            return((( mode == 1 ) || ( mode == 2 )) && ( getBitField( instr, 31, 4 ) == regId ));
        }
            
        case OP_LSID:   case OP_EXTR:   case OP_DEP:    case OP_DSR:    case OP_SHLA:   case OP_CMR:
        case OP_LDO:    case OP_CBR:    case OP_CBRU:   case OP_MST:    case OP_DIAG: {
            
            return( getBitField( instr, 31, 4 ) == regId );
        }
            
        default: return ( false );
    }
}

//------------------------------------------------------------------------------------------------------------
// Utility function to set and get the the pipeline register data.
//
//------------------------------------------------------------------------------------------------------------
uint32_t MemoryAccessStage::getPipeLineReg( uint8_t pReg ) {
    
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

void MemoryAccessStage::setPipeLineReg( uint8_t pReg, uint32_t val ) {
    
    switch ( pReg ) {
            
        case PSTAGE_REG_ID_PSW_0:    psPstate0.load( val );         break;
        case PSTAGE_REG_ID_PSW_1:    psPstate1.load( val );         break;
        case PSTAGE_REG_ID_INSTR:    psInstr.load( val );           break;
        case PSTAGE_REG_ID_VAL_A:    psValA.load( val );            break;
        case PSTAGE_REG_ID_VAL_B:    psValB.load( val );            break;
        case PSTAGE_REG_ID_VAL_X:    psValX.load( val );            break;
    }
}

//------------------------------------------------------------------------------------------------------------
// Access to a segment may be subject to protection checking. The little helper routine will compare the
// target segment Id with the segments stored in the protection control registers. The function returns a
// value of true when one field maztches.
//
//------------------------------------------------------------------------------------------------------------
bool MemoryAccessStage::checkProtectId( uint16_t segId ) {
    
    return((( segId  == core -> cReg[ CR_SEG_ID_0_1 ].getBitField( 15, 16 )) ||
            ( segId  == core -> cReg[ CR_SEG_ID_0_1 ].getBitField( 31, 16 )) ||
            ( segId  == core -> cReg[ CR_SEG_ID_2_3 ].getBitField( 15, 16 )) ||
            ( segId  == core -> cReg[ CR_SEG_ID_2_3 ].getBitField( 31, 16 )) ||
            ( segId  == core -> cReg[ CR_SEG_ID_4_5 ].getBitField( 15, 16 )) ||
            ( segId  == core -> cReg[ CR_SEG_ID_4_5 ].getBitField( 31, 16 )) ||
            ( segId  == core -> cReg[ CR_SEG_ID_6_7 ].getBitField( 15, 16 )) ||
            ( segId  == core -> cReg[ CR_SEG_ID_6_7 ].getBitField( 31, 16 ))));
}

//------------------------------------------------------------------------------------------------------------
// The memory access stage is primarily responsible for accessing memory data used by load and store type
// instructions. It will take the B and X value and compute the address offset for the memory data access.
// This the stage where any segment or control register is accessed.
//
// In addition, unconditional branches are further processed at this stage. The target branch offset is
// computed using the adder of the MA stage. For branches that do not save a return link, we are done and
// the EX stage is "bubbled". Otherwise, the instruction continues to the execute stage where the return
// address is computed using the EX stage ALU and stored in a general register.
//
// For the conditional branch instruction, the predicted branch target based in the offset was already
// processed in the FD stage. In this stage, the branch target address for the alternative target will
// be computed by adding B and X, which were set accordingly in the FD stage. When the EX stage evaluates
// the branch condition and determines that the branch was mispredicted, this adress will be used to
// continue the instruction stream after flushing instructions fetched wrongly from the pipeline.
//
// In general, the stall logic will always inhibit the update of the current and any previous pipeline
// register as well as flushing the next stage. Flushing is done by passing on a NOP instead of the current
// instruction. Initially, the processing assumes a non-stalled pipeline, a stall will set the flags. During
// the "ticks" executed, each time the stall is cleared but perhaps set again when the condition still exists.
//
// For instructions that access memory we will access memory data in the second part of this stage. The
// data fetched will be stored in B. This stage is also the starting point for a TLB or CACHE control
// instruction. If the TLB or Cache is busy, we stall in this stage until we have access to these resources.
// All other instructions just pass the pipeline data for A,B and X as well as the instruction address
// and the instruction itself to the next stage.
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
// done at the EX stage. For memory access type instructions, we need to clear the regIdForValB and
// regIdForValX fields, as the register content was consumed in this stage.
//
// Note: when a trap occurs, the pipeline is stalled and the procedure returns rightaway.
//
// ??? note: this is a rather long routine. Perhaps we should split this into smaller portions.
//------------------------------------------------------------------------------------------------------------
void MemoryAccessStage::process( ) {
    
    uint32_t            instr       = psInstr.get( );
    uint8_t             opCode      = getBitField( instr, 5, 6 );
    uint8_t             opMode      = getBitField( instr, 13, 2 );
    uint32_t            pOfs        = psValX.get( ) % PAGE_SIZE;
    uint32_t            pAdr        = 0;
    
    uint32_t            segSelect   = 0;
    uint32_t            segAdr      = 0;
    uint32_t            ofsAdr      = 0;
    uint32_t            dLen        = 0;
    
    ExecuteStage        *exStage    = core -> exStage;
    
    setStalled( false );
    
    //--------------------------------------------------------------------------------------------------------
    // Address computation or control instruction execution.
    //
    //--------------------------------------------------------------------------------------------------------
    switch( opCode ) {
            
        case OP_ADD:    case OP_ADC:    case OP_SUB:    case OP_SBC:
        case OP_AND:    case OP_OR:     case OP_XOR:
        case OP_CMP:    case OP_CMPU: {
            
            if ( opMode >= 2 ) {
                
                dLen    = mapDataLen( instr );
                ofsAdr  = add32( psValB.get( ), psValX.get( ));
                segAdr  = core -> sReg[ getBitField( ofsAdr, 1, 2 ) ].get( );
            }
            
        } break;
            
        case OP_LD:     case OP_ST:     case OP_LDR:    case OP_STC:
        case OP_LDPA:   case OP_PRB:  {
            
            dLen        = mapDataLen( instr );
            ofsAdr      = add32( psValB.get( ), psValX.get( ));
            segSelect   = getBitField( instr, 13, 2 );
         
            if ( segSelect == 0 ) segSelect += 4;
            segAdr = core -> sReg[ segSelect ].get( );
            
        } break;
            
        case OP_LDA:    case OP_STA: {
            
            dLen    = 4;
            segAdr  = 0;
            ofsAdr  = add32( psValB.get( ), psValX.get( ));
          
        } break;
            
        case OP_LDO:    {
            
            // ??? to do ... do it here or in EX ?
            
        } break;
            
        case OP_GATE: {
            
            core -> fdStage -> psPstate0.set( psPstate0.get( ));
            core -> fdStage -> psPstate1.set( add32( psValB.get( ), psValX.get( )));
            
            // ??? what about the priv stuff ?
            
            flushPipeLine( );
            
        } break;
            
        case OP_B:  case OP_BR:     case OP_BV: {
            
            core -> fdStage -> psPstate0.set( psPstate0.get( ));
            core -> fdStage -> psPstate1.set( add32( psValB.get( ), psValX.get( )));
            flushPipeLine( );
            
        } break;
            
        case OP_BVE: {
            
            ofsAdr      = add32( psValB.get( ), psValX.get( ));
            segSelect   = getBitField( instr, 13, 2 );
         
            if ( segSelect == 0 ) segSelect += 4;
            segAdr = core -> sReg[ segSelect ].get( );
            
            core -> fdStage -> psPstate1.set(  ofsAdr );
            core -> fdStage -> psPstate0.setBitField( segAdr, 31, 16 );
            flushPipeLine( );
            
        } break;
            
        case OP_BE: {
            
            ofsAdr = add32( psValB.get( ), psValB.get( ));
            segAdr = core -> sReg[ getBitField( instr, 27, 4 ) ].getBitField( 31, 16 );
            
            core -> fdStage -> psPstate0.setBitField( segAdr, 31, 16  );
            core -> fdStage -> psPstate1.set( ofsAdr );
        
            flushPipeLine( );
            
        } break;
            
        case OP_CBR: {
            
            ofsAdr = add32( psPstate1.get( ), psValX.get( ));
            exStage -> psValX.set( ofsAdr );
           
        } break;
            
        case OP_MR: {
            
            if ( ! getBit( instr, 11 )) {
                
                if ( getBit( instr, 12 ))  exStage -> psValB.set( core -> cReg[ instr & 0x3C ].get( ));
                else exStage -> psValB.set( core -> sReg[ getBitField( instr, 31, 4 ) ].get( ));
            }
            
        } break;
            
        case OP_ITLB: {
            
            CpuTlb      *tlbPtr = ( getBit( instr, 11 )) ? core -> dTlb : core -> iTlb;
            uint32_t    tlbSeg  = core -> sReg[ getBitField( instr, 27, 4 ) ].get( );
            
            bool rStat = false;
            
            if (getBit( instr, 12 ))
                
                rStat = tlbPtr -> insertTlbEntryProt( tlbSeg, getBitField( psValB.get( ), 31, 30 ),psValA.get( ) );
            else
                rStat = tlbPtr -> insertTlbEntryAdr( tlbSeg, getBitField( psValB.get( ), 31, 30 ), psValA.get( ) );
            
            if ( ! rStat ) {
                
                stallPipeLine( );
                return;
            }
            
        } break;
            
        case OP_PTLB: {
            
            dLen        = mapDataLen( instr );
            ofsAdr      = add32( psValB.get( ), psValX.get( ));
            segSelect   = getBitField( instr, 13, 2 );
         
            if ( segSelect == 0 ) segSelect += 4;
            segAdr = core -> sReg[ segSelect ].get( );
            
            CpuTlb   *  tlbPtr = ( getBit( instr, 11 )) ? core -> dTlb : core -> iTlb;
          
            if ( ! tlbPtr -> purgeTlbEntry( segAdr, ofsAdr )) {
                
                stallPipeLine( );
                return;
            }
            
        } break;
            
        case OP_PCA: {
            
            dLen        = mapDataLen( instr );
            ofsAdr      = add32( psValB.get( ), psValX.get( ));
            segSelect   = getBitField( instr, 13, 2 );
         
            if ( segSelect == 0 ) segSelect += 4;
            segAdr = core -> sReg[ segSelect ].get( );
            
            CpuTlb      *tlbPtr = ( getBit( instr, 11 )) ? core -> dTlb : core -> iTlb;
            L1CacheMem  *cPtr   = ( getBit( instr, 11 )) ?  core -> dCacheL1 : core -> iCacheL1;
            
            // ??? simplify ... this is quite complex to do in one cycle ... perhaps spread over MA and EX stage
            // ??? what exactly to pass to the trap handler ?
            
            TlbEntry *tlbEntryPtr = tlbPtr -> lookupTlbEntry( segAdr, ofsAdr );
            if ( tlbEntryPtr == nullptr ) {
                
                setupTrapData(( getBit( instr, 11 ) ? ITLB_NON_ACCESS_TRAP : DTLB_NON_ACCESS_TRAP ),
                              segAdr, ofsAdr, psPstate0.get( ));
                flushPipeLine( );
                return;
            }
            
            if ( ! getBit( instr, 12 ))
                cPtr -> flushBlock( segAdr, ofsAdr, ( tlbEntryPtr -> tPhysPage( ) << PAGE_SIZE_BITS ));
            else
                cPtr -> purgeBlock( segAdr, ofsAdr, ( tlbEntryPtr -> tPhysPage( ) << PAGE_SIZE_BITS ));
            
        } break;
    }
    
    //--------------------------------------------------------------------------------------------------------
    // Data load or store. This is the second half for instructions that read or write to memory. There are
    // a couple of cases. If the segment is zero, we must be privileged. The address is "0.ofs". Otherwise
    // we look up the physical address via the TLB and perform the access right checks. If the physical
    // address is in the physical memory range, we will access the cache data. The PDC memory range can only
    // be read. A write attempt is a trap. The IO range is passed to IO handler.
    //
    //--------------------------------------------------------------------------------------------------------
    if ( opCodeTab[ opCode ].flags & ( READ_INSTR | WRITE_INSTR )) {
        
        if ( psPstate0.get( ) & ST_DATA_TRANSLATION_ENABLE ) {
            
            TlbEntry   *tlbEntryPtr = core -> dTlb -> lookupTlbEntry( segAdr, ofsAdr );
            if ( tlbEntryPtr == nullptr ) {
                
                setupTrapData( DTLB_MISS_TRAP, segAdr, ofsAdr, psPstate0.get( ));
                stallPipeLine( );
                return;
            }
            
            if ( opCodeTab[ opCode ].flags & READ_INSTR ) {
                
                if (( tlbEntryPtr -> tPageType( ) != ACC_READ_WRITE ) &&
                    ( tlbEntryPtr -> tPageType( ) != ACC_READ_ONLY )) {
                    
                    setupTrapData( DTLB_ACC_RIGHTS_TRAP, segAdr, ofsAdr, psPstate0.get( ));
                    stallPipeLine( );
                    return;
                }
                
                if ( instrPrivLevel > tlbEntryPtr -> tPrivL1( )) {
                    
                    setupTrapData( DATA_MEM_PROTECT_TRAP, segAdr, ofsAdr, psPstate0.get( ));
                    stallPipeLine( );
                    return;
                }
            }
            else if ( opCodeTab[ opCode ].flags & WRITE_INSTR ) {
                
                /* change: if access type ... and not privilged...
                 
                 if (( tlbEntryPtr -> tPageType( ) != ACC_READ_WRITE ) &&
                 (( tlbEntryPtr -> tPageType( ) == ACC_EXECUTE ) && ( ! tlbEntryPtr -> tModifyExPage( ))) &&
                 (( tlbEntryPtr -> tPageType( ) == ACC_GATEWAY ) && ( ! tlbEntryPtr -> tModifyExPage( )))) {
                 
                 setupTrapData( DTLB_ACC_RIGHTS_TRAP, valS, valX, core -> stReg.get( ));
                 stallPipeLine( );
                 return;
                 }
                 */
                
                if ( instrPrivLevel > tlbEntryPtr -> tPrivL2( )) {
                    
                    setupTrapData( DATA_MEM_PROTECT_TRAP, segAdr, ofsAdr, psPstate0.get( ));
                    stallPipeLine( );
                    return;
                }
            }
            
            if ( psPstate0.get( ) & ST_PROTECT_ID_CHECK_ENABLE ) {
                
                if ( ! checkProtectId( tlbEntryPtr -> tSegId( ))) {
                    
                    setupTrapData( DTLB_PROTECT_ID_TRAP, segAdr, ofsAdr, psPstate0.get( ));
                    stallPipeLine( );
                    return;
                }
            }
            
            pAdr = tlbEntryPtr -> tPhysPage( ) | pOfs;
        }
        else {
            
            if ( psPstate0.get( ) & ST_EXECUTION_LEVEL ) {
                
                setupTrapData( DATA_MEM_PROTECT_TRAP, psPstate0.get( ), psPstate1.get( ), instr );
                stallPipeLine( );
                return;
            }
            
            pAdr = ofsAdr;
        }
        
        bool rStat = false;
        
        if ( pAdr <= core -> physMem -> getEndAdr(  )) {
            
            if ( opCodeTab[ opCode ].flags & READ_INSTR ) {
                
                uint32_t dataWord;
                rStat = core -> dCacheL1 -> readWord( segAdr, ofsAdr, pAdr, dLen, &dataWord );
                
                if ( rStat ) exStage -> psValB.set( dataWord );
            }
            else if ( opCodeTab[ opCode ].flags & WRITE_INSTR ) {
                
                rStat = core -> dCacheL1 -> writeWord( segAdr, ofsAdr, pAdr, dLen, psValA.get( ));
            }
        }
        else if (( pAdr >= core -> pdcMem -> getStartAdr( )) && ( pAdr <= core -> pdcMem -> getEndAdr( ))) {
            
            if ( opCodeTab[ opCode ].flags & READ_INSTR ) {
                
                uint32_t dataWord;
                rStat = core -> pdcMem -> readWord( 0, pAdr, 0, dLen, &dataWord );
                
                if ( rStat ) exStage -> psValB.set( dataWord );
            }
            else {
                
                // ??? cannot write to PDC, raise a trap or HPMC ?
            }
        }
        else if (( pAdr >= core -> ioMem -> getStartAdr( )) && ( pAdr <= core -> ioMem -> getEndAdr( ))) {
            
            if ( opCodeTab[ opCode ].flags & READ_INSTR ) {
                
                uint32_t dataWord;
                rStat = core -> ioMem -> readWord( 0, pAdr, 0, dLen, &dataWord );
                
                if ( rStat ) exStage -> psValB.set( dataWord );
            }
            else if ( opCodeTab[ opCode ].flags & WRITE_INSTR ) {
                
                rStat = core -> dCacheL1 -> writeWord( 0, pAdr, 0, dLen, psValA.get( ));
            }
        }
        else {
            
            // ??? invalid address. Should we raise a HPMC ?
            fprintf( stdout, "Invalid physical address in D-Access adr: %x \n", pAdr );
        }
        
        if ( ! rStat ) {
            
            stallPipeLine( );
            return;
        }
    }
    
    //--------------------------------------------------------------------------------------------------------
    // Pass the remaining data to the EX stage pipeline.
    //
    //--------------------------------------------------------------------------------------------------------
    exStage -> psInstr.set( psInstr.get( ));
    exStage -> psPstate0.set( psPstate0.get( ));
    exStage -> psPstate1.set( psPstate1.get( ));
    
    // ??? do rather at the respective case ?
    core -> exStage -> psValS.set( segAdr );
    core -> exStage -> psValX.set( ofsAdr );
}
