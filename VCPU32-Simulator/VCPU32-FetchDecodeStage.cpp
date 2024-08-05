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
#include "VCPU32-Types.h"
#include "VCPU32-Core.h"

//------------------------------------------------------------------------------------------------------------
// File local declarations. There are constants and routines used internally and not visible outside of this
// file. Most of the routines are inline functions. 
//
//------------------------------------------------------------------------------------------------------------
namespace {

uint32_t getBit( uint32_t arg, int pos ) {
    
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

uint32_t lowSignExtend32( uint32_t arg, int len ) {
    
    len = len % 32;
    
    uint32_t tmpM = ( 1U << ( len - 1 )) - 1;
    bool     sign = arg % 2;
    
    arg = arg >> 1;
    
    if ( sign ) return( arg |= ~ tmpM );
    else        return( arg &= tmpM );
}

uint32_t immGenLowSign( uint32_t instr, int pos, int len ) {
    
    pos = pos % 32;
    len = len % 32;
    
    return( lowSignExtend32( getBitField( instr, pos, len ), len ));
}

uint32_t add32( uint32_t arg1, uint32_t arg2 ) {
    
    return ( arg1 + arg2 );
}

bool isAligned( uint32_t adr, uint32_t align ) {
    
    switch( align ) {
            
        case 1: return( true );
        case 2: return(( adr & 0x1 ) == 0 );
        case 4: return(( adr & 0x3 ) == 0 );
        default: return( false );
    }
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
// ??? check the reset vector. Seg and OFs are OK, what about the status bits ?
//------------------------------------------------------------------------------------------------------------
void FetchDecodeStage::reset( )  {
    
    stalled = false;
    psPstate0.reset( );
    psPstate1.reset( );
    
    psPstate0.load( 0 );
    psPstate1.load( 0xF0000000 );
}

void FetchDecodeStage::tick( ) {
    
    if ( ! stalled ) {
        
        psPstate0.tick( );
        psPstate1.tick( );
    }
}

//------------------------------------------------------------------------------------------------------------
// Pipeline stall and flush. "stallPipeline" stops ourselve from being updated and pass on a NOP to the next
// stage so that no erreneous things will be done.
//
//------------------------------------------------------------------------------------------------------------
void FetchDecodeStage::stallPipeLine( ) {
    
    setStalled( true );
    
    core -> maStage -> psPstate0.set( psPstate0.get( ));
    core -> maStage -> psPstate1.set( psPstate1.get( ));
    core -> maStage -> psInstr.set( NOP_INSTR );
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
// "dependencyValA" checks if the instruction will fetch a value from the general register file intended to
// be stored in pipeline register "A". If that is the case, the execute stage will store its computed value
// to the pipeline register so that we have the correct value.
//
//------------------------------------------------------------------------------------------------------------
bool FetchDecodeStage::dependencyValA( uint32_t instr, uint32_t regId ) {
    
    switch ( getBitField( instr, 5, 6 )) {
            
        case OP_ADD:    case OP_ADC:    case OP_SUB:    case OP_SBC:    case OP_AND:    case OP_OR:
        case OP_XOR:    case OP_CMP:    case OP_CMPU: {
            
            uint32_t mode = getBitField( instr, 13, 2 );
            
            return((( mode == 1 ) || ( mode == 2 )) && ( getBitField( instr, 27, 4 ) == regId ));
        }
            
        case OP_DEP: {
            
            return(( ! getBit( instr, 10 )) ? ( getBitField( instr, 9, 4 ) == regId ) : false );
        }
            
        case OP_DSR:    case OP_SHLA:   case OP_CMR:    case OP_BVE:    case OP_CBR:    case OP_CBRU:
        case OP_LDPA:   case OP_PRB:    case OP_PTLB:   case OP_PCA:    case OP_DIAG: {
        
            return( getBitField( instr, 27, 4 ) == regId );
        }
            
       case OP_ST:     case OP_STA: {
            
            return( getBitField( instr, 9, 4 ) == regId );
        }
            
        default: return ( false );
    }
}

//------------------------------------------------------------------------------------------------------------
// "dependencyValB" checks if the instruction will fetch a value from the general register file intended to
// be stored in pipeline register "B". If that is the case, the execute stage will store its computed value
// to the pipeline register so that we have the correct value.
//
//------------------------------------------------------------------------------------------------------------
bool FetchDecodeStage::dependencyValB( uint32_t instr, uint32_t regId ) {
    
    switch ( getBitField( instr, 5, 6 )) {
            
        case OP_ADD:    case OP_ADC:    case OP_SUB:    case OP_SBC:    case OP_AND:    case OP_OR:
        case OP_XOR:    case OP_CMP:    case OP_CMPU: {
            
            uint32_t mode = getBitField( instr, 13, 2 );
            
            return((( mode == 1 ) || ( mode == 2 )) && ( getBitField( instr, 31, 4 ) == regId ));
        }
            
        case OP_LSID:   case OP_EXTR:   case OP_DEP:    case OP_DSR:    case OP_SHLA:   case OP_CMR:
        case OP_LDO:    case OP_LD:     case OP_ST:     case OP_LDA:    case OP_STA:    case OP_LDR:
        case OP_STC:    case OP_BV:     case OP_BE:     case OP_BVE:    case OP_CBR:    case OP_CBRU:
        case OP_MST:    case OP_LDPA:   case OP_PRB:    case OP_ITLB:   case OP_PTLB:   case OP_PCA:
        case OP_DIAG: {
            
            return( getBitField( instr, 31, 4 ) == regId );
        }
            
        default: return ( false );
    }
}

//------------------------------------------------------------------------------------------------------------
// "dependencyValX" checks if the instruction will fetch a value from the general register file intended to
// be stored in pipeline register "X". If that is the case, the execute stage will store its computed value
// to the pipeline register so that we have the correct value.
//
//------------------------------------------------------------------------------------------------------------
bool FetchDecodeStage::dependencyValX( uint32_t instr, uint32_t regId ) {
    
    switch ( getBitField( instr, 5, 6 )) {
            
        case OP_ADD:    case OP_ADC:    case OP_SUB:    case OP_SBC:    case OP_AND:    case OP_OR:
        case OP_XOR:    case OP_CMP:    case OP_CMPU: {
            
            uint32_t mode = getBitField( instr, 13, 2 );
            
            return(( mode == 2 ) && ( getBitField( instr, 27, 4 ) == regId ));
        }
            
        case OP_BR: {
            
            return( getBitField( instr, 31, 4 ) == regId );
        }
            
        case OP_BVE: {
            
            return( getBitField( instr, 27, 4 ) == regId );
        }
        
        default: return ( false );
    }
}

//------------------------------------------------------------------------------------------------------------
// Utility function to set and get the the pipeline register data.
//
//------------------------------------------------------------------------------------------------------------
uint32_t FetchDecodeStage::getPipeLineReg( uint8_t pReg ) {
    
    switch ( pReg ) {
            
        case PSTAGE_REG_STALLED:    return( stalled ? 1 : 0 );
        case PSTAGE_REG_ID_PSW_0:   return( psPstate0.get( ));
        case PSTAGE_REG_ID_PSW_1:   return( psPstate1.get( ));
        default: return( 0 );
    }
}

void FetchDecodeStage::setPipeLineReg( uint8_t pReg, uint32_t val ) {
    
    switch ( pReg ) {
            
        case PSTAGE_REG_ID_PSW_0:   psPstate0.load( val );  break;
        case PSTAGE_REG_ID_PSW_1:   psPstate1.load( val );  break;
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
// Access to a segment may be subject to protection checking. The little helper routine will compare the
// target segment Id with the segments stored in the protection control registers. The function returns a
// value of true when one field maztches.
//
//------------------------------------------------------------------------------------------------------------
bool FetchDecodeStage::checkProtectId( uint16_t segId ) {
    
    return((( segId  == getBitField( core -> cReg[ CR_SEG_ID_0_1 ].get( ), 15, 16 )) ||
            ( segId  == getBitField( core -> cReg[ CR_SEG_ID_0_1 ].get( ), 31, 16 )) ||
            ( segId  == getBitField( core -> cReg[ CR_SEG_ID_2_3 ].get( ), 15, 16 )) ||
            ( segId  == getBitField( core -> cReg[ CR_SEG_ID_2_3 ].get( ), 31, 16 )) ||
            ( segId  == getBitField( core -> cReg[ CR_SEG_ID_4_5 ].get( ), 15, 16 )) ||
            ( segId  == getBitField( core -> cReg[ CR_SEG_ID_4_5 ].get( ), 31, 16 )) ||
            ( segId  == getBitField( core -> cReg[ CR_SEG_ID_6_7 ].get( ), 15, 16 )) ||
            ( segId  == getBitField( core -> cReg[ CR_SEG_ID_6_7 ].get( ), 31, 16 ))));
}

//------------------------------------------------------------------------------------------------------------
// Instruction fetch and decode stage processing. First, we get the current instruction address from the PSW
// register. If code translation is enabled, the TLB needs to map the virtual address to a physical address.
// This can cause several traps. If the entry is not found, a ITLB_MISS_TRAP is recorded. Next the access
// rights are validated. The access is a code page (execute) access with a sufficient privilege level.
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
// and stall the next stage too. Pipeline Flushing is done by passing on a NOP instead of the current
// instruction. Initially, the processing assumes a non-stalled pipeline, a stall will set the flags. During
// the "ticks" executed, each time the stall is cleared but perhaps set again when the condition still exists.
//
// The instruction decode part then analyzes the instruction. The imaginary instruction decode hardware is
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
// The "B" register is typically used for a base address. It also holds an immediate value for the IMM type
// instructions. For instructions that use an address, "B" is designated as the base register and "X" as the
// offset value. The address generation stage exepcts these two pipeline registers to have the appropriate
// values for any address computation. Exception are instructions that use "A" and "B" and also need an
// address. The ony instruction is CBR, which will compare "A" and "B" and then do a PSW relative branch.
// In this case, only "X" is passed the offset. Note that in this case we read 3 general registers. In
// hardware this is either a 3-port read registr file or we spread the read into the next pipeline stage.
// The simulator assumes a 3-port read register file. Siilar, for the ST and STA instruction, the argument
// to store is in "A", "B" and "X" form the address.
//
// The CBR instruction uses a static branch prediction scheme. A backward branch address is considered as a
// branch taken a positive address is considered as a branch not taken. When we actually evaluate the
// condition in the EX stage, the branch decision needs to be corrected when mispredicted. For this to work,
// the alternate branch address needs to make its way to the EX stage. The MA stage will actually create
// the alternate branch target address and pass in "X" to the EX stage. Since a backward branch has a negative
// offset and is predicted as branch taken, the sign bit of the effect os used in the EX stage to figure out
// whether we mispredicted.
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
//
// ??? note: this is a rather long routine. Perhaps we should split this into smaller portions.
//------------------------------------------------------------------------------------------------------------
void FetchDecodeStage::process( ) {
    
    TlbEntry            *tlbEntryPtr    = nullptr;
    MemoryAccessStage   *maStage        = core -> maStage;
    uint32_t            instr           = NOP_INSTR;
    uint32_t            physAdr         = 0;
   
    
    //--------------------------------------------------------------------------------------------------------
    // Assume, we are not stalled.
    //
    //--------------------------------------------------------------------------------------------------------
    setStalled( false );
    
    //--------------------------------------------------------------------------------------------------------
    // Instruction Address Translation. If the instruction segment is zero, translation and protection chekcs
    // are bypassed. The offset is the physical memory address. We also must be privileged.
    //
    //--------------------------------------------------------------------------------------------------------
    if ( getBit( psPstate0.get( ), ST_CODE_TRANSLATION_ENABLE )) {
   
        tlbEntryPtr = core -> iTlb -> lookupTlbEntry( psPstate1.getBitField( 31, 16 ), psPstate1.get( ));
        if ( tlbEntryPtr == nullptr ) {
            
            setupTrapData( ITLB_MISS_TRAP, psPstate0.get( ), psPstate1.get( ), instr );
            stallPipeLine( );
            return;
        }
        
        if ( tlbEntryPtr -> tPageType( ) != ACC_EXECUTE ) {
            
            setupTrapData( ITLB_ACC_RIGHTS_TRAP, psPstate0.get( ), psPstate1.get( ), instr );
            stallPipeLine( );
            return;
        }
        
        if ( getBit( psPstate0.get( ), ST_PROTECT_ID_CHECK_ENABLE )) {
            
            if ( ! checkProtectId( tlbEntryPtr -> tSegId( ))) {
                
                setupTrapData( ITLB_PROTECT_ID_TRAP, psPstate0.get( ), psPstate1.get( ), instr );
                stallPipeLine( );
                return;
            }
        }
       
        physAdr = tlbEntryPtr -> tPhysPage( ) | psPstate1.getBitField( 31, PAGE_SIZE_BITS );
    }
    else {
    
         if ( getBit( psPstate0.get( ), ST_EXECUTION_LEVEL )) {
             
             setupTrapData( INSTR_MEM_PROTECT_TRAP, psPstate0.get( ), psPstate1.get( ), instr );
             stallPipeLine( );
             return;
         }
        
        physAdr = psPstate1.get( );
     }
    
    //--------------------------------------------------------------------------------------------------------
    // Instruction word fetch. If the address is in between the physical memory range, we pass the "seg.ofs"
    // and the "pAdr" to the routine. Note that we cover both the virtual address and the "0.ofs" case with
    // the routine to the instruction cache. The caches will use the "seg.ofs" to locate the cache lines,
    // the "pAdr" passed is the tag from the TLB which will be compared by the cache controller for a match.
    // If the address range is in the PDC address range, we invoke the PDC handler. The PDC is kind of a
    // ROM with processor dependent code. We cannot fetch data from IO memory space.
    //
    //--------------------------------------------------------------------------------------------------------
    if ( physAdr <= core -> physMem -> getEndAdr( ) - 4 ) {
        
        if ( ! core -> iCacheL1 -> readWord( psPstate0.getBitField( 15, 16 ), 
                                             psPstate1.get( ), physAdr, 4, &instr )) {
            
            stallPipeLine( );
            return;
        }
    } 
    else if (( physAdr >= core -> pdcMem -> getStartAdr( )) && ( physAdr <= core -> pdcMem -> getEndAdr( ))) {
       
        if ( ! core -> pdcMem -> readWord( 0, physAdr, physAdr, 4, &instr )) {
            
            stallPipeLine( );
            return;
        }
    }
    else {
        
        // ??? invalid address. Should we raise a HPMC ?
        fprintf( stdout, "Invalid physical address in I-Fetch adr: %x \n", physAdr );
    }
    
    //--------------------------------------------------------------------------------------------------------
    // Instruction Decode. Essentially a giant case statement.
    //
    //--------------------------------------------------------------------------------------------------------
    uint32_t opCode = getBitField( instr, 5, 6 );
    
    //--------------------------------------------------------------------------------------------------------
    // Instruction execution privilege check.
    //
    //--------------------------------------------------------------------------------------------------------
    if ( psPstate0.getBit( ST_CODE_TRANSLATION_ENABLE )) {
        
        if ( ! (( tlbEntryPtr -> tPrivL2( ) <= getBit( psPstate0.get( ), ST_EXECUTION_LEVEL )) &&
                ( getBit( psPstate0.get( ), ST_EXECUTION_LEVEL ) <= tlbEntryPtr -> tPrivL1( )))) {
                
            setupTrapData( INSTR_MEM_PROTECT_TRAP, psPstate0.get( ), psPstate1.get( ), instr );
            stallPipeLine( );
            return;
        }
    }
    
    //--------------------------------------------------------------------------------------------------------
    // Privileged instruction check.
    //
    //--------------------------------------------------------------------------------------------------------
    if ( opCodeTab[ opCode ].flags & PRIV_INSTR ) {
        
        if ( getBit( psPstate0.get( ), ST_EXECUTION_LEVEL ) > 0 ) {
            
            setupTrapData( PRIV_OPERATION_TRAP, psPstate0.get( ), psPstate1.get( ), instr );
            return;
        }
    }
    
    //--------------------------------------------------------------------------------------------------------
    // Instruction register fetch and immedoiate value generation.
    //
    //--------------------------------------------------------------------------------------------------------
    switch ( opCode ) {
        
        case OP_ADD:    case OP_ADC:    case OP_SUB:    case OP_SBC:    case OP_AND:
        case OP_OR:     case OP_XOR:    case OP_CMP:    case OP_CMPU: {
            
            uint32_t opMode = getBitField( instr, 13, 2 );
           
            if (( opMode < 2 ) && ( opCodeTab[ opCode ].flags & ( LOAD_INSTR | STORE_INSTR ))) {
                
                setupTrapData( ILLEGAL_INSTR_TRAP, psPstate0.get( ), psPstate1.get( ), instr );
                return;
            }
            
            if (( opMode == 2 ) && ( opCodeTab[ opCode ].flags & STORE_INSTR )) {
                
                setupTrapData( ILLEGAL_INSTR_TRAP, psPstate0.get( ), psPstate1.get( ), instr );
                return;
            }
            
            switch ( opMode ) {
                    
                case OP_MODE_IMM: {
                   
                    maStage -> psValA.set( 0 );
                    maStage -> psValB.set( immGenLowSign( instr, 31, 18 ));
                    maStage -> psValX.set( 0 );
                   
                } break;
                    
                case OP_MODE_REG: {
                    
                    maStage -> psValA.set( core -> gReg[ getBitField( instr, 27, 4 ) ].get( ));
                    maStage -> psValB.set( core -> gReg[ getBitField( instr, 31, 4 ) ].get( ));
                    maStage -> psValX.set( 0 );
                    
                } break;
                    
               
                case OP_MODE_REG_INDX: {
                    
                    maStage -> psValA.set( core -> gReg[ getBitField( instr, 9, 4 ) ].get( ));
                    maStage -> psValB.set( core -> gReg[ getBitField( instr, 31, 4 ) ].get( ));
                    maStage -> psValX.set( core -> gReg[ getBitField( instr, 27,4 ) ].get( ));
                    
                } break;
                    
                case OP_MODE_INDX: {
                    
                    if ( opCodeTab[ opCode ].flags & STORE_INSTR ) {
                        
                        maStage -> psValA.set( core -> gReg[ getBitField( instr, 9, 4 ) ].get( ));
                    }
                    else maStage -> psValA.set( 0 );
                    
                    maStage -> psValB.set( core -> gReg[ getBitField( instr, 31, 4 ) ].get( ));
                    maStage -> psValX.set( immGenLowSign( instr, 27, 12 ));
                    
                } break;
            }
            
        } break;
            
        case OP_ADDIL: {
        
            maStage -> psValA.set( core -> gReg[ getBitField( instr, 9, 4 ) ].get( ));
            maStage -> psValB.set( getBitField( instr, 31, 22 ) << 10 );
            maStage -> psValX.set( 0 );
            
        } break;
            
        case OP_B: {
          
            maStage -> psValA.set( 0 );
            maStage -> psValB.set( psPstate1.get( ));
            maStage -> psValX.set( immGenLowSign( instr, 31, 22 ) << 2 );
            
        } break;
            
        case OP_BE: {
            
            maStage -> psValA.set( 0 );
            maStage -> psValB.set( core -> gReg[ getBitField( instr, 31, 4 ) ].get( ));
            maStage -> psValX.set( immGenLowSign( instr, 23, 14 ) << 2 );
            
        } break;
            
        case OP_BR: {
            
            maStage -> psValA.set( 0 );
            maStage -> psValB.set( core -> gReg[ getBitField( instr, 31, 4 ) ].get( ));
            maStage -> psValX.set( psPstate1.get( ));
            
        } break;
            
        case OP_BRK: {
            
            maStage -> psValA.set( getBitField( instr, 9, 4 ));
            maStage -> psValB.set( getBitField( instr, 31, 16 ));
            maStage -> psValX.set( 0 );
         
        } break;
            
        case OP_BV: {
            
            maStage -> psValA.set( 0 );
            maStage -> psValB.set( core -> gReg[ getBitField( instr, 31, 4 ) ].get( ));
            maStage -> psValX.set( 0 );
            
        } break;
            
        case OP_BVE: {
        
            maStage -> psValA.set( 0 );
            maStage -> psValB.set( core -> gReg[ getBitField( instr, 31, 4 ) ].get( ));
            maStage -> psValX.set( core -> gReg[ getBitField( instr, 27, 4 ) ].get( ));
           
        } break;
            
        case OP_CBR:    case OP_CBRU: {
            
            maStage -> psValA.set( core -> gReg[ getBitField( instr, 27, 4 ) ].get( ));
            maStage -> psValB.set( core -> gReg[ getBitField( instr, 31, 4 ) ].get( ));
            maStage -> psValX.set( 0 );
            
        } break;
            
        case OP_CMR: {
       
            maStage -> psValA.set( core -> gReg[ getBitField( instr, 27, 4 ) ].get( ));
            maStage -> psValB.set( core -> gReg[ getBitField( instr, 31, 4 ) ].get( ));
            maStage -> psValX.set( 0 );
            
        } break;
            
        case OP_DEP: {
            
            if ( ! getBit( instr, 10 )) {
                
                maStage -> psValA.set( core -> gReg[ getBitField( instr, 9, 4 ) ].get( ));
            }
            else maStage -> psValA.set( 0 );
            
            if ( ! getBit( instr, 12 )) {
                
                maStage -> psValB.set( core -> gReg[ getBitField( instr, 31, 4 ) ].get( ));
            }
            else maStage -> psValB.set( getBitField( instr, 31, 4 ));
            
            maStage -> psValX.set( 0 );
            
        } break;
            
        case OP_DIAG: {
            
            maStage -> psValA.set( core -> gReg[ getBitField( instr, 27, 4 ) ].get( ));
            maStage -> psValB.set( core -> gReg[ getBitField( instr, 31, 4 ) ].get( ));
            maStage -> psValX.set( 0 );
            
        } break;
            
        case OP_DSR: {
            
            maStage -> psValA.set( core -> gReg[ getBitField( instr, 27, 4 ) ].get( ));
            maStage -> psValB.set( core -> gReg[ getBitField( instr, 31, 4 ) ].get( ));
            maStage -> psValX.set( 0 );
            
        } break;
            
        case OP_EXTR: {
            
            maStage -> psValA.set( 0 );
            maStage -> psValB.set( core -> gReg[ getBitField( instr, 31, 4 ) ].get( ));
            maStage -> psValX.set( 0 );
            
        } break;
            
        case OP_GATE: {
            
            maStage -> psValA.set( 0 );
            maStage -> psValB.set( psPstate1.get( ));
            maStage -> psValX.set( immGenLowSign( instr, 31, 22 ) << 2 );
           
            // ??? when do we exactly set the execution level in the status reg ? There are
            // two instructions ahead of us which should NOT benefit from the potential priv change....
            
            // ??? should we just get the TLB priv level and pass it onto the EX stage ?
            
            if ( psPstate0.getBit( ST_CODE_TRANSLATION_ENABLE )) {
               
                if ( tlbEntryPtr -> tPageType( ) == 3 ) {
            
                    psPstate0.setBit( tlbEntryPtr -> tPrivL1( ), ST_EXECUTION_LEVEL );
                }
            }
            
            psPstate0.setBit( ST_EXECUTION_LEVEL );
            
        } break;
            
        case OP_ITLB: {
            
            maStage -> psValA.set( 0 );
            maStage -> psValB.set( core -> gReg[ getBitField( instr, 31, 4 ) ].get( ));
            maStage -> psValX.set( 0 );
            
        } break;
            
        case OP_LD: case OP_LDA: {
            
            maStage -> psValA.set( 0 );
            maStage -> psValB.set( core -> gReg[ getBitField( instr, 31, 4 ) ].get( ));
            
            if ( getBit( instr, 10 )) {
            
                maStage -> psValX.set( core -> gReg[ getBitField( instr, 27, 4 ) ].get( ));
            }
            else maStage -> psValX.set( immGenLowSign( instr, 27, 12 ));
            
        } break;
            
        case OP_LDIL: {
            
            maStage -> psValA.set( 0 );
            maStage -> psValB.set( immGenLowSign( instr, 31, 22 ));
            maStage -> psValX.set( 0 );
            
        } break;
            
        case OP_LDO: {
            
            maStage -> psValA.set( 0 );
            maStage -> psValB.set( core -> gReg[ getBitField( instr, 31, 4 ) ].get( ));
            maStage -> psValX.set( immGenLowSign( instr, 27, 18 ));
            
        } break;
            
        case OP_LDPA: {
            
            maStage -> psValA.set( 0 );
            maStage -> psValB.set( core -> gReg[ getBitField( instr, 31, 4 ) ].get( ));
            maStage -> psValX.set( core -> gReg[ getBitField( instr, 27, 4 ) ].get( ));
            
        } break;
            
        case OP_LDR: {
            
            maStage -> psValA.set( 0 );
            maStage -> psValB.set( core -> gReg[ getBitField( instr, 31, 4 ) ].get( ));
            maStage -> psValX.set( immGenLowSign( instr, 27, 12 ));
            
        } break;
            
        case OP_LSID: {
            
            maStage -> psValA.set( 0 );
            maStage -> psValB.set( core -> gReg[ getBitField( instr, 31, 4 ) ].get( ));
            maStage -> psValX.set( 0 );
            
        } break;
            
        case OP_MR: {
            
            maStage -> psValA.set( 0 );
            maStage -> psValX.set( 0 );
            
            if ( getBit( instr, 11 )) {
                
                maStage -> psValB.set( core -> gReg[ getBitField( instr, 9, 4 ) ].get( ));
            }
            else maStage -> psValB.set( 0 );
            
        } break;
            
        case OP_MST: {
            
            maStage -> psValA.set( 0 );
            maStage -> psValX.set( 0 );
            
            switch( getBitField( instr, 11, 2 )) {
                    
                case 0: {
                    
                    maStage -> psValB.setBitField( core -> gReg[ getBitField( instr, 9, 4 ) ].get( ), 31, 6 );
                 
                } break;
                    
                case 1:
                case 2: {
                    
                    maStage -> psValB.set( getBitField( instr, 31, 6 ));
                    
                } break;
                    
                default: {
                    
                    setupTrapData( ILLEGAL_INSTR_TRAP, psPstate0.get( ), psPstate1.get( ), instr );
                    return;
                }
            }
            
        } break;
            
        case OP_PCA: {
            
            maStage -> psValA.set( 0 );
            maStage -> psValB.set( core -> gReg[ getBitField( instr, 31, 4 ) ].get( ));
            maStage -> psValX.set( core -> gReg[ getBitField( instr, 27, 4 ) ].get( ));
           
        } break;
            
        case OP_PRB: {
    
            maStage -> psValX.set( 0 );
            maStage -> psValB.set( core -> gReg[ getBitField( instr, 31, 4 ) ].get( ));
            
            if ( ! getBit( instr, 11 )) {
                
                maStage -> psValA.set( core -> gReg[ getBitField( instr, 27, 4 ) ].get( ));
            }
            else maStage -> psValA.setBit( 31, getBit( instr, 27 ));
        
        } break;
            
        case OP_PTLB: {
            
            maStage -> psValA.set( 0 );
            maStage -> psValB.set( core -> gReg[ getBitField( instr, 31, 4 ) ].get( ));
            maStage -> psValX.set( core -> gReg[ getBitField( instr, 27, 4 ) ].get( ));
           
        } break;
            
        case OP_RFI: {
           
        } break;
          
        case OP_SHLA:{
         
            maStage -> psValA.set( core -> gReg[ getBitField( instr, 27, 4 ) ].get( ));
            maStage -> psValB.set( core -> gReg[ getBitField( instr, 31, 4 ) ].get( ));
            maStage -> psValX.set( 0 );
            
        } break;
            
        case OP_ST: case OP_STA: {
            
            maStage -> psValA.set( core -> gReg[ getBitField( instr, 9, 4 ) ].get( ));
            maStage -> psValB.set( core -> gReg[ getBitField( instr, 31, 4 ) ].get( ));
            
            if ( getBit( instr, 10 )) {
                
                maStage -> psValX.set( core -> gReg[ getBitField( instr, 27, 4 ) ].get( ));
            }
            else maStage -> psValX.set( immGenLowSign( instr, 27, 12 ));
          
        } break;
            
        case OP_STC: {
            
            maStage -> psValA.set( core -> gReg[ getBitField( instr, 9, 4 ) ].get( ));
            maStage -> psValB.set( core -> gReg[ getBitField( instr, 31, 4 ) ].get( ));
            maStage -> psValX.set( immGenLowSign( instr, 27, 12 ));
            
        } break;
            
        default: {
            
            setupTrapData( ILLEGAL_INSTR_TRAP, psPstate0.get( ), psPstate1.get( ), instr );
            return;
        }
    }
    
    //--------------------------------------------------------------------------------------------------------
    // Instructions that will do computation in the MA stage with "B" and "X" may run into the case that the
    // register content for them is just produced by the preceeding instruction. In other words, the EX stage
    // is about to compute the new value while we are in the fetch stage for the follow-on instruction. This
    // value has not been written back and hence we cannot continue until the result is in reach via a simple
    // bypass from the EX stage. This routine will test our instruction currently decoded for being one that
    // depends on the latest register content. In general these are all instructions that do address arithmetic
    // using the B and X pipeline fields.
    //
    // ??? anything special to do for REG 0 ?
    // ??? what about the status or segment register ?
    //---------------------------------------------------------------------------------------------------------
    if ( opCodeTab[ getBitField( maStage -> psInstr.get( ), 5, 6 )].flags & REG_R_INSTR ) {
        
        uint32_t regIdR = maStage -> psInstr.getBitField( 9, 4 );
        
        if ( dependencyValB( instr, regIdR ) || dependencyValX( instr, regIdR )) {
            
            stallPipeLine( );
            return;
        }
    }
    
    //--------------------------------------------------------------------------------------------------------
    // Pass the data to the MA stage pipeline.
    //
    //--------------------------------------------------------------------------------------------------------
    core -> maStage -> psPstate0.set( psPstate0.get( ));
    core -> maStage -> psPstate1.set( psPstate1.get( ));
    core -> maStage -> psInstr.set( instr );
    
    //--------------------------------------------------------------------------------------------------------
    // Compute the next instruction address. Typically, this is the current instruction plus 4 bytes. For the
    // conditional branch we either increment by 4 or by the offset encoded in the instruction. In addition,
    // we pass on the offset or the value of 4 to the next stage.
    //
    // ??? what exactly is the instruction offset arithmetic ?
    // ??? we add a signed value to an unsigned value ....
    //--------------------------------------------------------------------------------------------------------
    if (( opCode == OP_CBR ) || ( opCode == OP_CBRU )) {
        
        if ( getBit( instr, 23 )) {
            
            psPstate1.set( add32( psPstate1.get( ), immGenLowSign( instr, 31, 22 )));
            maStage -> psValX.set( 4 );
        }
        else {
            
            psPstate1.set( psPstate1.get( ) + 4 );
            maStage -> psValX.set( immGenLowSign( instr, 31, 22 ));
        }
    }
    else psPstate1.set( psPstate1.get( ) + 4 );
}
