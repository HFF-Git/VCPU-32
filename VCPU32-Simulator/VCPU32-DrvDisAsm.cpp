//------------------------------------------------------------------------------------------------------------
//
// VCPU32 - A 32-bit CPU - Dissaembler
//
//------------------------------------------------------------------------------------------------------------
// The instruction disassemble routine will format an instruction word in human readable form. An instruction
// has the general format
//
//      OpCode [ Opcode Options ] [ source ] [ target ]
//
// The disassemble routine will analyze an instruction word and present the instruction portion in the above
// order. It does make heavy use of the CPU24Instr inline routines to get to the fields and values.
//
//------------------------------------------------------------------------------------------------------------
//
// VCPU32 - A 32-bit CPU - Dissaembler
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
#include "VCPU32-Driver.hpp"
#include "VCPU32-Core.hpp"


//------------------------------------------------------------------------------------------------------------
// Local namespace. These routines are not visible otside this source file.
//
//------------------------------------------------------------------------------------------------------------
namespace {

//------------------------------------------------------------------------------------------------------------
// Signed math. Our CPU has 24 bits, which are apped to a 32-bit word of the host processor. To get a negative
// number correctly, the bit 0 ( bit 8 in the 32-bit word ) is checked and then the result is exnded to
// a negative 32bit value and returned as an int.
//
//------------------------------------------------------------------------------------------------------------
int signedVal( uint32_t val ) {
    
    if ( val & 0x00800000 ) val |= 0xFF000000;
    return((int) val );
}

//------------------------------------------------------------------------------------------------------------
// "displayHalfWord" lists out a 12-bit word in the specified number base. The half word is a 12-bit signed
// number. For decimal formatting we make sure to show a now "-nnnn" value.
//
//------------------------------------------------------------------------------------------------------------
void displayHalfWord( uint32_t val, TokId fmtType ) {
    
    if ( val == 0 ) fprintf( stdout, "0" );
    
    else {
        
        val = val & 07777;
        
        if ( fmtType == TOK_DEC ) {
            
            if ( val & 04000 ) {
                
                fprintf( stdout, "-%4d", val );
            }
            else fprintf( stdout, "%4d", val );
        }
        else if ( fmtType == TOK_OCT  ) fprintf( stdout, "%#04o", val );
        else if ( fmtType == TOK_HEX )  fprintf( stdout, "%#04x", val );
        else                            fprintf( stdout, "**num***" );
    }
}

//------------------------------------------------------------------------------------------------------------
// A little helper function to display the comparison condition in human readable form.
//
//------------------------------------------------------------------------------------------------------------
void displayComparisonCodes( uint32_t cmpCode ) {
    
    switch( cmpCode ) {
            
        case CC_EQ:  fprintf( stdout, "EQ" ); return;
        case CC_LT:  fprintf( stdout, "LT" ); return;
        case CC_GT:  fprintf( stdout, "GT" ); return;
        case CC_LS:  fprintf( stdout, "LS" ); return;
        case CC_NE:  fprintf( stdout, "NE" ); return;
        case CC_LE:  fprintf( stdout, "LE" ); return;
        case CC_GE:  fprintf( stdout, "GE" ); return;
        case CC_HI:  fprintf( stdout, "HI" ); return;
    }
}

//------------------------------------------------------------------------------------------------------------
// A little helper function to display the test condition in human readable form.
//
//------------------------------------------------------------------------------------------------------------
void displayTestCodes( uint32_t tstCode ) {
    
    switch( tstCode ) {
            
        case TC_EQ: fprintf( stdout, "EQ" ); return;
        case TC_LT: fprintf( stdout, "LT" ); return;
        case TC_GT: fprintf( stdout, "GT" ); return;
        case TC_EV: fprintf( stdout, "EV" ); return;
        case TC_NE: fprintf( stdout, "NE" ); return;
        case TC_LE: fprintf( stdout, "LE" ); return;
        case TC_GE: fprintf( stdout, "GE" ); return;
        case TC_OD: fprintf( stdout, "OD" ); return;
        default:    fprintf( stdout, "***" ); return;
    }
}

//------------------------------------------------------------------------------------------------------------
// There are quite a few instructions that use the operand argument format. This routine will format such
// an operand. The operand mode 3 to 7 are a bit special. If mode three is used by a load/store instruction,
// the immediate value is constructed by the "ofs" and the instr.[15..17] field, else just the instr.[15..17]
// field. Likewise, for modes 4 to 7, the immediate value is constructed using the "ofs" field and the
// immediate field in the operand.
//
//------------------------------------------------------------------------------------------------------------
void displayOperandModeField( uint32_t instr, TokId fmtId = TOK_DEC ) {
    
    uint32_t    opCode         = Instr::opCodeField( instr );
    bool            loadStoreInstr = (( opCodeTab[ opCode ].flags & ( LOAD_INSTR | STORE_INSTR )) ||
                                      ( opCode == OP_LEA ));
    
    switch ( Instr::opModeField( instr )) {
            
        case ADR_MODE_IMM: {
            
            if ( loadStoreInstr ) fprintf( stdout, "***" );
            else fprintf( stdout, "%d", signedVal( Instr::immGen0S9( instr )));
            
        } break;
            
        case ADR_MODE_REG: {
            
            if ( loadStoreInstr ) fprintf( stdout, "***" );
            else  fprintf( stdout, "R%d", Instr::regBIdField( instr ));
            
        } break;
            
        case ADR_MODE_TWO_REGS: {
            
            fprintf( stdout, "R%d,R%d", Instr::regAIdField( instr ), Instr::regBIdField( instr ));
            
        } break;
            
        case ADR_MODE_EXT_ADR: {
            
            if (( opCodeTab[ opCode ].flags & LOAD_INSTR ) ||
                ( opCodeTab[ opCode ].flags & STORE_INSTR ) ||
                ( opCode == OP_LEA )) {
                
                fprintf( stdout, "%d(S%d,R%d)",
                        signedVal( Instr::immGen30S3( instr )),
                        Instr::regAIdField( instr ),
                        Instr::regBIdField( instr ));
            }
            else {
                
                fprintf( stdout, "%d(S%d,R%d)",
                        signedVal( Instr::immGen0S3( instr )),
                        Instr::regAIdField( instr ),
                        Instr::regBIdField( instr ));
                
            }
        } break;
            
        case ADR_MODE_INDX_GR4: {
            
            if ( loadStoreInstr ) fprintf( stdout, "%d(R4)", signedVal( Instr::immGen30S9( instr )));
            else fprintf( stdout, "%d(R4)", signedVal( Instr::immGen0S9( instr )));
            
        } break;
            
        case ADR_MODE_INDX_GR5: {
            
            if ( loadStoreInstr ) fprintf( stdout, "%d(R5)", signedVal( Instr::immGen30S9( instr )));
            else fprintf( stdout, "%d(R5)", signedVal( Instr::immGen0S9( instr )));
            
        } break;
            
        case ADR_MODE_INDX_GR6: {
            
            if ( loadStoreInstr ) fprintf( stdout, "%d(R6)", signedVal( Instr::immGen30S9( instr )));
            else fprintf( stdout, "%d(R6)", signedVal( Instr::immGen0S9( instr )));
            
        } break;
            
        case ADR_MODE_INDX_GR7: {
            
            if ( loadStoreInstr ) fprintf( stdout, "%d(R7)", signedVal( Instr::immGen30S9( instr )));
            else fprintf( stdout, "%d(R7)", signedVal( Instr::immGen0S9( instr )));
            
        } break;
    }
}

//------------------------------------------------------------------------------------------------------------
// Each instruction has an opCode. A simple printout of the mnemonic.
//
//------------------------------------------------------------------------------------------------------------
void displayOpCode( uint32_t instr ) {
 
    fprintf( stdout, "%s", opCodeTab[ Instr::opCodeField( instr ) ].mnemonic );
}

//------------------------------------------------------------------------------------------------------------
// Some instructions have a set of further qualifiers. They are listed after a "." and are single characters.
// If there is no option in a given set is set or it is the common case value, nothing is printed.
//
//------------------------------------------------------------------------------------------------------------
void displayOpCodeOptions( uint32_t instr ) {
  
    switch ( Instr::opCodeField( instr )) {
            
        case OP_ADD:
        case OP_SUB: {
            
            if ( Instr::arithOpFlagField( instr ) > 0 ) {
                
                fprintf( stdout, "." );
                if ( Instr::useCarryField( instr ))    fprintf( stdout, "C" );
                if ( Instr::logicalOpField( instr ))   fprintf( stdout, "L" );
                if ( Instr::trapOvlField( instr ))     fprintf( stdout, "O" );
            }
            
        } break;
            
        case OP_AND:
        case OP_OR:
        case OP_XOR: {
            
            if ( Instr::boolOpFlagField( instr ) != 0 ) {
                
                fprintf( stdout, "." );
                if ( Instr::negateResField( instr )) fprintf( stdout, "N" );
                if ( Instr::complRegBField( instr )) fprintf( stdout, "C" );
            }
            
        } break;
            
        case OP_CMP: {
            
            fprintf( stdout, "." );
            displayComparisonCodes( Instr::cmpCondField( instr ));
            
        } break;
            
        case OP_LDI: {
            
            if (( Instr::ldiLeftField( instr )) || ( Instr::ldiZeroField( instr ))) {
                
                fprintf( stdout, "." );
                if ( Instr::ldiLeftField( instr )) fprintf( stdout, "L" );
                if ( Instr::ldiZeroField( instr )) fprintf( stdout, "Z" );
            }
            
        } break;
            
        case OP_EXTR: {
            
            if ( Instr::extrSignedField( instr )) {
                
                fprintf( stdout, "." );
                if ( Instr::extrSignedField( instr )) fprintf( stdout, "S" );
            }
            
        } break;
       
        case OP_DEP: {
            
            if (( Instr::depInZeroField( instr )) || ( Instr::depImmOptField( instr ))) {
                
                fprintf( stdout, "." );
                if ( Instr::depInZeroField( instr )) fprintf( stdout, "Z" );
                if ( Instr::depImmOptField( instr )) fprintf( stdout, "I" );
            }
            
        } break;
            
        case OP_SHLA: {
            
            if ( Instr::arithOpFlagField( instr ) != 0 ) {
                
                fprintf( stdout, "." );
                if ( Instr::shlaUseImmField( instr ))  fprintf( stdout, "I" );
                if ( Instr::logicalOpField( instr ))   fprintf( stdout, "L" );
                if ( Instr::trapOvlField( instr ))     fprintf( stdout, "O" );
            }
            
        } break;
            
        case OP_CMR: {
            
            fprintf( stdout, "." );
            displayTestCodes( Instr::cmrCondField( instr ));
         //   if ( CPU24Instr::cmrImmField( instr )) fprintf( stdout, "I" );
            
        } break;
            
        case OP_CBR: {
            
            fprintf( stdout, "." );
            displayComparisonCodes( Instr::cbrCondField( instr ));
            
        } break;
            
        case OP_TBR: {
            
            fprintf( stdout, "." );
            displayTestCodes( Instr::tbrCondField( instr ));
            
        } break;
            
        case OP_MST: {
            
            switch ( Instr::mstModeField( instr )) {
                    
                case 0:                                 break;
                case 1:     fprintf( stdout, ".S" );    break;
                case 2:     fprintf( stdout, ".C" );    break;
                default:    fprintf( stdout, ".***" );  break;
            }
            
        } break;
            
        case OP_PRB: {
            
            if ( Instr::prbRwAccField( instr )) fprintf( stdout, ".W" );
            else fprintf( stdout, ".R" );
            
        } break;
            
        case OP_ITLB: {
            
            fprintf( stdout, "." );
            if ( Instr::tlbKindField( instr )) fprintf( stdout, "D" );
            else fprintf( stdout, "I" );
            
            if ( Instr::tlbArgModeField( instr )) fprintf( stdout, "A" );
            else fprintf( stdout, "P" );
            
        } break;
            
        case OP_PTLB: {
            
            fprintf( stdout, "." );
            if ( Instr::tlbKindField( instr )) fprintf( stdout, "D" );
            else fprintf( stdout, "I" );
            
        } break;
            
        case OP_PCA: {
            
            fprintf( stdout, "." );
            if ( Instr::pcaKindField( instr )) fprintf( stdout, "D" );
            else fprintf( stdout, "I" );
            if ( Instr::pcaPurgeFlushField( instr )) fprintf( stdout, "F" );
            
        } break;
    }
    
    fprintf( stdout, " " );
}

//------------------------------------------------------------------------------------------------------------
// This routine display the instruction target. MOst of the time it is a general register. For the STORE
// type instructions the target adress is decoded and printed. Finally there are the MTR instructions which
// which will use a segment or control register as the target.
//
//------------------------------------------------------------------------------------------------------------
void displayTarget( uint32_t instr, TokId fmtId = TOK_DEC ) {
    
    uint32_t opCode = Instr::opCodeField( instr );
    
    if ( opCodeTab[ opCode ].flags & REG_R_INSTR ) {
        
        if ( opCode != OP_BLE ) {
            
            fprintf( stdout, "R%d", Instr::regRIdField( instr ));
        }
    }
    else if ( opCodeTab[ opCode ].flags & STORE_INSTR ) {
        
        if ( opCode == OP_STWA ) {
            
            fprintf( stdout, "%d(R%d)",
                    signedVal( Instr::immGen6S6( instr )),
                    Instr::regBIdField( instr ));
        }
        else if ( opCode == OP_STWE ) {
            
            fprintf( stdout, "%d(S%d,R%d)",
                    signedVal( Instr::immGen6S3( instr )),
                    Instr::regAIdField( instr ),
                    Instr::regBIdField( instr ));
        }
        else displayOperandModeField( instr, fmtId );
    }
    else if (( opCode == OP_MR ) && ( Instr::mrMovDirField( instr ))) {
        
        /*
        if ( CPU24Instr::mrRegTypeField( instr ))
            fprintf( stdout, "C%d", CPU24Instr::mrRegGrpField( instr ) * 8 + CPU24Instr::regBIdField( instr ));
        else fprintf( stdout, "S%d", CPU24Instr::regBIdField( instr ));
         */
    }
    else if (( opCode == OP_MR ) && ( ! Instr::mrMovDirField( instr ))) {
        
        fprintf( stdout, "R%d", Instr::regRIdField( instr ));
    }
}

//------------------------------------------------------------------------------------------------------------
// Instruction have operands. For most of the instructions this is the operand field with the defined
// addressing modes. For others it is highy instruction specific. The operand routine also has a parameter
// to specify in what radix a value is shown. Address offsets are however always printed in decimal.
//
//------------------------------------------------------------------------------------------------------------
void displayOperands( uint32_t instr, TokId fmtId = TOK_DEC ) {
    
    uint32_t opCode = Instr::opCodeField( instr );
    
    if ( opCodeTab[ opCode ].flags & OP_MODE_INSTR ) {
        
        fprintf( stdout, "," );
        
        if ( opCodeTab[ opCode ].flags & STORE_INSTR ) fprintf( stdout, "R%d", Instr::regRIdField( instr ));
        else displayOperandModeField( instr, fmtId );
    }
    else {
        
        switch ( opCode ) {
       
            case OP_LDI: {
                
                fprintf( stdout, "," );
                displayHalfWord( signedVal( Instr::immGen30S9( instr )), fmtId );
                
            } break;
                
            case OP_SHLA: {
                
                fprintf( stdout, "," );
                fprintf( stdout, "R%d", Instr::regAIdField( instr ));
                fprintf( stdout, ",R%d", Instr::regBIdField( instr ));
                
                if ( Instr::shlaSaField( instr ) > 0 )
                    fprintf( stdout, ",%d",  Instr::shlaSaField( instr ));
                
            } break;
                
            case OP_EXTR:
            case OP_DEP: {
                
                uint32_t pos = Instr::extrDepPosField( instr );
               
                fprintf( stdout, "," );
                fprintf( stdout, "R%d", Instr::regBIdField( instr ));
                
                if      ( pos == 31 )   fprintf( stdout, ",shamt" );
                else if ( pos <= 23 )   fprintf( stdout, ",%d", Instr::extrDepPosField( instr ));
                else                    fprintf( stdout, ",***" );
                
                fprintf( stdout, ",%d", Instr::extrDepLenField( instr ));
                
            } break;
                
            case OP_DSR: {
                
                fprintf( stdout, "," );
                fprintf( stdout, "R%d", Instr::regAIdField( instr ));
                fprintf( stdout, ",R%d", Instr::regBIdField( instr ));
                
                if ( Instr::dsrSaField( instr ) == 31 ) fprintf( stdout, ",shmat" );
                else fprintf( stdout, ",%d", Instr::dsrSaField( instr ));
                
            } break;
                
            case OP_LSID: {
                
                fprintf( stdout, "," );
                fprintf( stdout, "R%d", Instr::regBIdField( instr ));
                
            } break;
                
            case OP_CMR: {
                
                fprintf( stdout, "," );
                fprintf( stdout, "R%d", Instr::regBIdField( instr ));
                
                /*
                if ( CPU24Instr::cmrImmField( instr ))  fprintf( stdout, ",%d", signedVal( CPU24Instr::immGen0S6( instr )));
                else                                    fprintf( stdout, ",R%d", CPU24Instr::regAIdField( instr ));
                 */
                
            } break;
                
            case OP_LDWA: {
                
                fprintf( stdout, "," );
                fprintf( stdout, "%d(R%d)",
                        signedVal( Instr::immGen6S6( instr )),
                        Instr::regBIdField( instr ));
                
            } break;
                
            case OP_LDWE: {
                
                fprintf( stdout, "," );
                fprintf( stdout, "%d(S%d,R%d)",
                        signedVal( Instr::immGen6S3( instr )),
                        Instr::regAIdField( instr ),
                        Instr::regBIdField( instr ));
                
            } break;
                
            case OP_STWA:
            case OP_STWE: {
                
                fprintf( stdout, "," );
                fprintf( stdout, "R%d", Instr::regRIdField( instr ));
                
            } break;
                
            case OP_B: {
                
                fprintf( stdout, "%i", signedVal( Instr::immGen6S9( instr )));
                
            } break;
                
            case OP_GATE:
            case OP_BL: {
            
                fprintf( stdout, "," );
                fprintf( stdout, "%i", signedVal( Instr::immGen6S9( instr )));
                
            } break;
                
                
            case OP_BR: {
                
                fprintf( stdout, "(R%d)", Instr::regBIdField( instr ));
                
            } break;
                
            case OP_BLR: {
                
                fprintf( stdout, "," );
                fprintf( stdout, "(R%d)", Instr::regBIdField( instr ));
                
            } break;
                
            case OP_BV: {
                
                fprintf( stdout, "(R%d)", Instr::regBIdField( instr ));
                
            } break;
                
            case OP_BVR: {
                
                fprintf( stdout, "(R%d,", Instr::regAIdField( instr ));
                fprintf( stdout, "R%d)", Instr::regBIdField( instr ));
                
            } break;
                
            case OP_BE:  {
                
                fprintf( stdout, "%d,", signedVal( Instr::immGen9S3( instr )));
                fprintf( stdout, "(S%d", Instr::regAIdField( instr ));
                fprintf( stdout, ",R%d)", Instr::regBIdField( instr ));
                
            } break;
                
            case OP_BLE: {
                
                fprintf( stdout, "%d", signedVal( Instr::immGen9S3( instr )));
                fprintf( stdout, "(S%d", Instr::regAIdField( instr ));
                fprintf( stdout, ",R%d)", Instr::regBIdField( instr ));
                
            } break;
                
            case OP_CBR: {
                
                fprintf( stdout, "R%d", Instr::regAIdField( instr ));
                fprintf( stdout, ",R%d", Instr::regBIdField( instr ));
                fprintf( stdout, ",%d", signedVal( Instr::immGen6S3( instr )));
                
            } break;
                
            case OP_TBR: {
                
                fprintf( stdout, "R%d", Instr::regBIdField( instr ));
                fprintf( stdout, ",%d", signedVal( Instr::immGen6S3( instr )));
                
            } break;
                
            case OP_MR: {
                
                if ( Instr::mrMovDirField( instr )) {
                    
                    fprintf( stdout, "," );
                    fprintf( stdout, "R%d", Instr::regRIdField( instr ));
                }
                else {
                    
                    fprintf( stdout, "," );
                    
                    // ??? fix
                    /*
                     if ( CPU24Instr::mrRegTypeField( instr ))
                     fprintf( stdout, "C%d", CPU24Instr::mrRegGrpField( instr ) * 8 + CPU24Instr::regBIdField( instr ));
                     else fprintf( stdout, "S%d", CPU24Instr::regBIdField( instr ));
                     */
                }
                
            } break;
                
            case OP_MST: {
                
                fprintf( stdout, "," );
                switch( Instr::mstModeField( instr )) {
                        
                    case 0: fprintf( stdout, "R%d", Instr::regBIdField( instr ));  break;
                    case 1:
                    case 2: displayHalfWord( Instr::mstArgField( instr ), fmtId ); break;
                    default: fprintf( stdout, "***" );
                }
                
            } break;
                
            case OP_PRB: {
                
                fprintf( stdout, "," );
                if ( Instr::prbAdrModeField( instr )) fprintf( stdout, "(R%d)", Instr::regBIdField( instr ));
                else fprintf( stdout, "(S%d, R%d)", Instr::regAIdField( instr ), Instr::regBIdField( instr ));
                
            } break;
                
            case OP_LDPA: {
                
                fprintf( stdout, "," );
                if ( Instr::ldpaAdrModeField( instr )) fprintf( stdout, "(R%d)", Instr::regBIdField( instr ));
                else fprintf( stdout, "(S%d, R%d)", Instr::regAIdField( instr ), Instr::regBIdField( instr ));
                
            } break;
                
            case OP_ITLB: {
                
                fprintf( stdout, "R%d,", Instr::regRIdField( instr ));
                
                if ( Instr::tlbAdrModeField( instr )) fprintf( stdout, "(R%d)", Instr::regBIdField( instr ));
                else fprintf( stdout, "(S%d, R%d)", Instr::regAIdField( instr ), Instr::regBIdField( instr ));
                
            } break;
                
            case OP_PTLB: {
                
                if ( Instr::tlbAdrModeField( instr )) fprintf( stdout, "(R%d)", Instr::regBIdField( instr ));
                else fprintf( stdout, "(S%d, R%d)", Instr::regAIdField( instr ), Instr::regBIdField( instr ));
                
            } break;
                
            case OP_PCA: {
                
                if ( Instr::pcaAdrModeField( instr )) fprintf( stdout, "(R%d)", Instr::regBIdField( instr ));
                else fprintf( stdout, "(S%d, R%d)", Instr::regAIdField( instr ), Instr::regBIdField( instr ));
                
            } break;
        }
    }
}

}; // namespace

//************************************************************************************************************
// Object methods.
//************************************************************************************************************

//------------------------------------------------------------------------------------------------------------
// The object constructor. Nothing really to do here...
//------------------------------------------------------------------------------------------------------------
DrvDisAsm::DrvDisAsm( VCPU32Globals *glb ) {
    
    this -> glb = glb;
}

//------------------------------------------------------------------------------------------------------------
// Print an instruction, nicely formatted. An instruction has generally four parts. The opCode, the opCode
// options, the source and the target. The opCode and options are grouped as are the target and operand.
//
//------------------------------------------------------------------------------------------------------------
void DrvDisAsm::displayOpCodeAndOptions( uint32_t instr ) {
    
    displayOpCode( instr );
    displayOpCodeOptions( instr );
}

void DrvDisAsm::displayTargetAndOperands( uint32_t instr, TokId fmt ) {
    
    displayTarget( instr, fmt );
    displayOperands( instr, fmt );
}

void DrvDisAsm::displayInstr( uint32_t instr, TokId fmt ) {
    
    displayOpCodeAndOptions( instr );
    displayTargetAndOperands( instr, fmt );
}

