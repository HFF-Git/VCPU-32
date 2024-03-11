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
// Signed math. 
//
//------------------------------------------------------------------------------------------------------------
int signedVal( uint32_t val ) {
    
    return((int) val );
}

//------------------------------------------------------------------------------------------------------------
// "displayHalfWord" lists out a 16-bit word in the specified number base. The half word is a 12-bit signed
// number. For decimal formatting we make sure to show a now "-nnnn" value.
//
// ??? see how hex and octal look ....
//------------------------------------------------------------------------------------------------------------
void displayHalfWord( uint32_t val, TokId fmtType ) {
    
    if ( val == 0 ) fprintf( stdout, "0" );
    
    else {
        
        val = val & 0xFFFF;
        
        if ( fmtType == TOK_DEC ) {
            
            if ( val & 0x8000 ) {
                
                fprintf( stdout, "-%4d", val );
            }
            else fprintf( stdout, "%4d", val );
        }
        else if ( fmtType == TOK_OCT  ) fprintf( stdout, "%#06o", val );
        else if ( fmtType == TOK_HEX )  fprintf( stdout, "%#0x6x", val );
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
// an operand.
//
//------------------------------------------------------------------------------------------------------------
void displayOperandModeField( uint32_t instr, TokId fmtId = TOK_DEC ) {
    
    switch ( Instr::opModeField( instr )) {
            
        case ADR_MODE_IMM:      fprintf( stdout, "%d", signedVal( Instr::immGen0S14( instr ))); break;
            
        case ADR_MODE_NO_REGS:  break;
        
        case ADR_MODE_REG_A:    fprintf( stdout, "R%d", Instr::regAIdField( instr )); break;
        case ADR_MODE_REG_B:    fprintf( stdout, "R%d", Instr::regBIdField( instr )); break;

        case ADR_MODE_REG_A_B: {
            
            fprintf( stdout, "R%d,R%d", Instr::regAIdField( instr ), Instr::regBIdField( instr ));
            
        } break;
            
        case ADR_MODE_REG_INDX_W:
        case ADR_MODE_REG_INDX_H:
        case ADR_MODE_REG_INDX_B: {
            
            fprintf( stdout, "R%d(R%d)", Instr::regAIdField( instr ), Instr::regBIdField( instr ));
            
        } break;
            
        case ADR_MODE_EXT_INDX_W: {
            
            fprintf( stdout, "%d(S%d,R%d)",
                    signedVal( Instr::immGen0S6( instr )),
                    Instr::regAIdField( instr ),
                    Instr::regBIdField( instr ));
            
        } break;
            
        case ADR_MODE_GR10_INDX_W: case ADR_MODE_GR10_INDX_H: case ADR_MODE_GR10_INDX_B:
        case ADR_MODE_GR11_INDX_W: case ADR_MODE_GR11_INDX_H: case ADR_MODE_GR11_INDX_B:
        case ADR_MODE_GR12_INDX_W: case ADR_MODE_GR12_INDX_H: case ADR_MODE_GR12_INDX_B:
        case ADR_MODE_GR13_INDX_W: case ADR_MODE_GR13_INDX_H: case ADR_MODE_GR13_INDX_B:
        case ADR_MODE_GR14_INDX_W: case ADR_MODE_GR14_INDX_H: case ADR_MODE_GR14_INDX_B:
        case ADR_MODE_GR15_INDX_W: case ADR_MODE_GR15_INDX_H: case ADR_MODE_GR15_INDX_B: {
            
            fprintf( stdout, "%d(R%d)", signedVal( Instr::immGen0S14( instr )), Instr::opModeField( instr ));
            
        } break;
            
        default:  fprintf( stdout, "***opMode***" );
    }
}

//------------------------------------------------------------------------------------------------------------
// Each instruction has an opCode. Instructions that contains an operand are appended the data item length.
// Instructions with an operand mode encoding will append to the opCode the data item size if reqeuired. One
// exception to this rule is that an operation on a word will not always add "W" to the mnemonic.
//
//------------------------------------------------------------------------------------------------------------
void displayOpCode( uint32_t instr ) {
    
    uint32_t opCode = Instr::opCodeField( instr );
    
    fprintf( stdout, "%s", opCodeTab[ opCode ].mnemonic );
    if ( opCodeTab[ opCode ].flags & OP_MODE_INSTR ) {
        
        switch ( Instr::opModeField( instr )) {
                
            case ADR_MODE_GR10_INDX_W: case ADR_MODE_GR11_INDX_W: case ADR_MODE_GR12_INDX_W:
            case ADR_MODE_GR13_INDX_W: case ADR_MODE_GR14_INDX_W: case ADR_MODE_GR15_INDX_W: {
                
                if ( ! ( opCodeTab[ opCode ].flags & ( LOAD_INSTR | STORE_INSTR ))) fprintf( stdout, "W" );
                
            } break;
            
            case ADR_MODE_GR10_INDX_H: case ADR_MODE_GR11_INDX_H: case ADR_MODE_GR12_INDX_H:
            case ADR_MODE_GR13_INDX_H: case ADR_MODE_GR14_INDX_H: case ADR_MODE_GR15_INDX_H: {
                
                fprintf( stdout, "H" );
                
            } break;
            
            case ADR_MODE_GR10_INDX_B: case ADR_MODE_GR11_INDX_B: case ADR_MODE_GR12_INDX_B:
            case ADR_MODE_GR13_INDX_B: case ADR_MODE_GR14_INDX_B: case ADR_MODE_GR15_INDX_B: {
                
                fprintf( stdout, "B" );
                
            } break;
        }
    }
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
// This routine display the instruction target. Most of the time it is a general register. For the STORE
// type instructions the target adress is decoded and printed. Finally there are the MTR instructions which
// which will use a segment or control register as the target. There is one further exception. The BLE
// instruction will produce a register value, the return link stored in R0. This is however not shown in the
// disassembly printout.
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
        
        if (( opCode == OP_STWA ) || ( opCode == OP_STHA ) || ( opCode == OP_STBA )) {
            
            fprintf( stdout, "%d(R%d)", signedVal( Instr::immGen8S10( instr )), Instr::regBIdField( instr ));
        }
        else if (( opCode == OP_STWE ) || ( opCode == OP_STHE ) || ( opCode == OP_STBE )) {
            
            fprintf( stdout, "%d(S%d,R%d)",
                    signedVal( Instr::immGen8S6( instr )),
                    Instr::regAIdField( instr ),
                    Instr::regBIdField( instr ));
        }
        else displayOperandModeField( instr, fmtId );
    }
    else if (( opCode == OP_MR ) && ( Instr::mrMovDirField( instr ))) {
        
         if ( Instr::mrRegTypeField( instr )) fprintf( stdout, "C%d", Instr::mrArgField( instr ));
         else fprintf( stdout, "S%d", Instr::regBIdField( instr ));
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
        
    } else {
        
        switch ( opCode ) {
                
            case OP_LDIL: 
            case OP_ADDIL: {
              
                fprintf( stdout, ", %d", Instr::immLeftField( instr ));
                
            } break;
            
            case OP_SHLA: {
                
                fprintf( stdout, "," );
                fprintf( stdout, "R%d", Instr::regAIdField( instr ));
                fprintf( stdout, ",R%d", Instr::regBIdField( instr ));
                
                if ( Instr::shlaSaField( instr ) > 0 ) fprintf( stdout, ",%d",  Instr::shlaSaField( instr ));
                
            } break;
                
            case OP_EXTR:
            case OP_DEP: {
                
                // ??? this looks odd .... order ?
                
                fprintf( stdout, "," );
                fprintf( stdout, "R%d", Instr::regBIdField( instr ));
                
                if ( Instr::extrDepSaOptField( instr )) fprintf( stdout, ",shamt" );
                else fprintf( stdout, ",%d", Instr::extrDepPosField( instr ));
                
                fprintf( stdout, ",%d", Instr::extrDepLenField( instr ));
                
            } break;
                
            case OP_DSR: {
                
                fprintf( stdout, "," );
                fprintf( stdout, "R%d", Instr::regAIdField( instr ));
                fprintf( stdout, ",R%d", Instr::regBIdField( instr ));
                
                if ( Instr::dsrSaOptField( instr )) fprintf( stdout, ",shmat" );
                else fprintf( stdout, ",%d", Instr::dsrSaAmtField( instr ));
                
            } break;
                
            case OP_LSID: {
                
                fprintf( stdout, "," );
                fprintf( stdout, "R%d", Instr::regBIdField( instr ));
                
            } break;
                
            case OP_CMR: {
                
                fprintf( stdout, "," );
                fprintf( stdout, "R%d", Instr::regBIdField( instr ));
                
            } break;
                
            case OP_LDWA:
            case OP_LDHA:
            case OP_LDBA: {
                
                fprintf( stdout, "," );
                fprintf( stdout, "%d(R%d)", signedVal( Instr::immGen8S10( instr )), Instr::regBIdField( instr ));
                
            } break;
                
            case OP_LDWE:
            case OP_LDHE:
            case OP_LDBE: {
                
                fprintf( stdout, "," );
                fprintf( stdout, "%d(S%d,R%d)",
                        signedVal( Instr::immGen8S6( instr )),
                        Instr::regAIdField( instr ),
                        Instr::regBIdField( instr ));
                
            } break;
                
            case OP_STWA:
            case OP_STHA:
            case OP_STBA:
                
            case OP_STWE: 
            case OP_STHE:
            case OP_STBE: {
                
                fprintf( stdout, "," );
                fprintf( stdout, "R%d", Instr::regRIdField( instr ));
                
            } break;
                
            case OP_B: {
                
                fprintf( stdout, "%i", signedVal( Instr::immGen8S14( instr )));
                
            } break;
                
            case OP_GATE:
            case OP_BL: {
                
                fprintf( stdout, "," );
                fprintf( stdout, "%i", signedVal( Instr::immGen8S14( instr )));
                
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
                
            case OP_BE:
            case OP_BLE: {
                
                fprintf( stdout, "%d,", signedVal( Instr::immGen12S6( instr )));
                fprintf( stdout, "(S%d", Instr::regAIdField( instr ));
                fprintf( stdout, ",R%d)", Instr::regBIdField( instr ));
                
            } break;
                
            case OP_CBR: {
                
                fprintf( stdout, "R%d", Instr::regAIdField( instr ));
                fprintf( stdout, ",R%d", Instr::regBIdField( instr ));
                fprintf( stdout, ",%d", signedVal( Instr::immGen8S6( instr )));
                
            } break;
                
            case OP_TBR: {
                
                fprintf( stdout, "R%d", Instr::regBIdField( instr ));
                fprintf( stdout, ",%d", signedVal( Instr::immGen8S6( instr )));
                
            } break;
                
            case OP_MR: {
                
                if ( Instr::mrMovDirField( instr )) {
                    
                    fprintf( stdout, "," );
                    fprintf( stdout, "R%d", Instr::regRIdField( instr ));
                    
                } else {
                    
                    fprintf( stdout, "," );
                    
                    if ( Instr::mrRegTypeField( instr )) fprintf( stdout, "C%d", Instr::mrArgField( instr ));
                    else fprintf( stdout, "S%d", Instr::regBIdField( instr ));
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

