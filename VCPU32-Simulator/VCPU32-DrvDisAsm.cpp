//------------------------------------------------------------------------------------------------------------
//
// VCPU32 - A 32-bit CPU - Dissaembler
//
//------------------------------------------------------------------------------------------------------------
// The instruction disassemble routine will format an instruction word in human readable form. An instruction
// has the general format
//
//      OpCode [ Opcode Options ] [ target ] [ source ] 
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
// Instruction decoding means to get to bits and bit fields. Here is a set of helper functions.
//
//------------------------------------------------------------------------------------------------------------
bool getBit( uint32_t arg, int pos ) {
    
    return( arg & ( 1U << ( 31 - ( pos % 32 ))));
}

uint32_t getBitField( uint32_t arg, int pos, int len, bool sign = false ) {
    
    pos = pos % 32;
    len = len % 32;
    
    uint32_t tmpM = ( 1U << len ) - 1;
    uint32_t tmpA = arg >> ( 31 - pos );
    
    if ( sign ) return( tmpA | ( ~ tmpM ));
    else        return( tmpA & tmpM );
}

static inline uint32_t lowSignExtend32( uint32_t arg, int len ) {
    
    len = len % 32;
    
    uint32_t tmpM = ( 1U << ( len - 1 )) - 1;
    bool     sign = arg % 2;
    
    arg = arg >> 1;
    
    if ( sign ) return( arg |= ~ tmpM );
    else        return( arg &= tmpM );
}

static inline uint32_t immGenPosLenLowSign( uint32_t instr, int pos, int len ) {
   
    return( lowSignExtend32( Instr::getBitField( instr, pos, len ), len ));
}

uint32_t mapOpModeToIndexReg( uint32_t opMode ) {
    
    if      (( opMode >= 8 ) && ( opMode <= 15 )) return( opMode );
    else if (( opMode >= 16 ) && ( opMode <= 23 )) return( opMode - 8 );
    else if (( opMode >= 24 ) && ( opMode <= 31 )) return( opMode - 16 );
    else return( 0 );
}

//------------------------------------------------------------------------------------------------------------
// "printImmVal" disply an immediate value in the selected radix. Octals and hex numbers are printed unsigned
// quantities, decimal numbers are interpreted as signed integers. Most often decimal notation is used to
// specifiy offsets on indexed addressing modes.
//
//------------------------------------------------------------------------------------------------------------
void printImmVal( uint32_t val, TokId fmtType = TOK_HEX ) {
    
    if ( val == 0 ) fprintf( stdout, "0" );
    
    else {
       
        if      ( fmtType == TOK_DEC )  fprintf( stdout, "%d", ((int) val ));
        else if ( fmtType == TOK_OCT  ) fprintf( stdout, "%#0o", val );
        else if ( fmtType == TOK_HEX )  fprintf( stdout, "%#0x", val );
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
        default:     fprintf( stdout, "**" ); return;
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
        default:    fprintf( stdout, "**" ); return;
    }
}

//------------------------------------------------------------------------------------------------------------
// There are quite a few instructions that use the operand argument format. This routine will format such
// an operand. The disassembler shows all modes, also the ones that the assembler does not use directly.
//
//------------------------------------------------------------------------------------------------------------
void displayOperandModeField( uint32_t instr, TokId fmtId = TOK_DEC ) {
    
    uint32_t opMode = getBitField( instr, 17, 5 );
    
    switch ( opMode ) {
            
        case OP_MODE_IMM:       printImmVal( immGenPosLenLowSign( instr, 31, 14 )); break;
        case OP_MODE_ONE_REG:   fprintf( stdout, "0, R%d", getBitField( instr, 31, 4 )); break;
        case OP_MODE_TWO_REG:   {
            
            fprintf( stdout, "R%d,R%d", getBitField( instr, 27, 4 ), getBitField( instr, 31, 4 ));
            
        } break;
            
        case 3: fprintf( stdout, "***opMode(3)***" ); break;
            
        case OP_MODE_REG_INDX_W:
        case OP_MODE_REG_INDX_H:
        case OP_MODE_REG_INDX_B: {
            
            fprintf( stdout, "R%d(R%d)", getBitField( instr, 27, 4 ), getBitField( instr, 31, 4 ));
            
        } break;
            
        case 7: fprintf( stdout, "***opMode(7)***" ); break;
            
        default: {
            
            printImmVal( Instr::immGenPosLenLowSign( instr, 31, 14 ), TOK_DEC );
            
            if ( getBitField( instr, 19, 2 ) > 0 ) {
                
                fprintf( stdout, "(S%d, R%d)", mapOpModeToIndexReg( opMode ), getBitField( instr, 19, 2 ));
            }
            else fprintf( stdout, "(R%d)", mapOpModeToIndexReg( opMode ));
        }
    }
}

//------------------------------------------------------------------------------------------------------------
// Each instruction has an opCode. Instructions that contains an operand are appended the data item length.
// Instructions with an operand mode encoding will append to the opCode the data item size if reqeuired. One
// exception to this rule is that an operation on a word will not always add "W" to the mnemonic.
//
//------------------------------------------------------------------------------------------------------------
void displayOpCode( uint32_t instr ) {
    
    uint32_t opCode = getBitField( instr, 5, 6 );
    uint32_t opMode = getBitField( instr, 17, 5 );
    
    fprintf( stdout, "%s", opCodeTab[ opCode ].mnemonic );
    
    if ( opCodeTab[ opCode ].flags & OP_MODE_INSTR ) {
        
        if (( opMode <= 8 ) && ( opMode <= 15 )) {
            
            if (( opCode == OP_LD ) || ( opCode == OP_ST )) fprintf( stdout, "W" );
        }
        else if (( opMode <= 16 ) && ( opMode <= 23 )) {
            
            fprintf( stdout, "H" );
        }
        else if (( opMode <= 24 ) && ( opMode <= 31 )) {
            
            fprintf( stdout, "B" );
        }
    }
}

//------------------------------------------------------------------------------------------------------------
// Some instructions have a set of further qualifiers. They are listed after a "." and are single characters.
// If there is no option in a given set is set or it is the common case value, nothing is printed.
//
//------------------------------------------------------------------------------------------------------------
void displayOpCodeOptions( uint32_t instr ) {
    
    uint32_t opCode = getBitField( instr, 5, 6 );
    
    switch ( opCode ) {
            
        case OP_ADD:
        case OP_SUB: {
            
            if ( getBitField( instr, 12, 3 ) > 0 ) {
                
                fprintf( stdout, "." );
                if ( getBit( instr, 10 ))   fprintf( stdout, "C" );
                if ( getBit( instr, 11 ))   fprintf( stdout, "L" );
                if ( getBit( instr, 12 ))   fprintf( stdout, "O" );
            }
            
        } break;
            
        case OP_AND:
        case OP_OR:
        case OP_XOR: {
            
            if ( getBitField( instr, 11, 2 ) > 0 ) {
                
                fprintf( stdout, "." );
                if ( getBit( instr, 10 )) fprintf( stdout, "N" );
                if ( getBit( instr, 11 )) fprintf( stdout, "C" );
            }
            
        } break;
            
        case OP_CMP: {
            
            fprintf( stdout, "." );
            displayComparisonCodes( getBitField( instr, 12, 3 ));
            
        } break;
            
        case OP_EXTR: {
            
            if ( getBitField( instr, 11, 2 )) {
                
                fprintf( stdout, "." );
                if ( getBit( instr, 10 )) fprintf( stdout, "S" );
                if ( getBit( instr, 11 )) fprintf( stdout, "A" );
            }
            
        } break;
            
        case OP_DEP: {
            
            if ( getBitField( instr, 12, 3 )) {
                
                fprintf( stdout, "." );
                if ( getBit( instr, 10 )) fprintf( stdout, "Z" );
                if ( getBit( instr, 11 )) fprintf( stdout, "A" );
                if ( getBit( instr, 12 )) fprintf( stdout, "I" );
            }
            
        } break;
            
        case OP_DSR: {
            
            if ( getBit( instr, 11 )) {
                
                fprintf( stdout, "." );
                if ( getBit( instr, 11 )) fprintf( stdout, "A" );
            }
            
        } break;
            
        case OP_SHLA: {
            
            if ( getBitField( instr, 12, 3 ) > 0 ) {
                
                fprintf( stdout, "." );
                if ( getBit( instr, 10 ))   fprintf( stdout, "Z" );
                if ( getBit( instr, 11 ))   fprintf( stdout, "L" );
                if ( getBit( instr, 12 ))   fprintf( stdout, "O" );
            }
            
        } break;
            
        case OP_CMR: {
            
            fprintf( stdout, "." );
            displayTestCodes( getBitField( instr, 12, 3 ));
            
        } break;
            
        case OP_CBR: {
            
            fprintf( stdout, "." );
            displayComparisonCodes( getBitField( instr, 8, 3 ));
            
        } break;
            
        case OP_TBR: {
            
            fprintf( stdout, "." );
            displayTestCodes( getBitField( instr, 8, 3 ));
            
        } break;
            
        case OP_MST: {
            
            switch ( getBitField( instr, 11, 2 )) {
                    
                case 0:                                 break;
                case 1:     fprintf( stdout, ".S" );    break;
                case 2:     fprintf( stdout, ".C" );    break;
                default:    fprintf( stdout, ".***" );  break;
            }
            
        } break;
            
        case OP_PRB: {
            
            if ( getBit( instr, 10 )) fprintf( stdout, ".W" );
            else fprintf( stdout, ".R" );
            
        } break;
            
        case OP_ITLB: {
            
            if ( getBit( instr, 10 )) fprintf( stdout, ".D" );
            else fprintf( stdout, ".I" );
            
            if ( getBit( instr, 11 )) fprintf( stdout, "A" );
            else fprintf( stdout, "P" );
            
        } break;
            
        case OP_PTLB: {
            
            if ( getBit( instr, 10 )) fprintf( stdout, ".D" );
            else fprintf( stdout, ".I" );
            
        } break;
            
        case OP_PCA: {
            
            if ( getBit( instr, 10 )) fprintf( stdout, ".D" );
            else fprintf( stdout, ".I" );
            
            if ( getBit( instr, 11 )) fprintf( stdout, "F" );
            
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
    
    uint32_t opCode = getBitField( instr, 5, 6 );
    
    if ( opCodeTab[ opCode ].flags & REG_R_INSTR ) {
        
        if ( opCode != OP_BLE ) {
            
            fprintf( stdout, "R%d", getBitField( instr, 9, 4 ));
        }
    }
    else if ( opCodeTab[ opCode ].flags & STORE_INSTR ) {
        
        if ( opCode == OP_STWA )  {
            
            printImmVal( getBitField( instr, 27, 18 ));
            fprintf( stdout, "(R%d)", getBitField( instr, 31, 4 ));
        }
        else displayOperandModeField( instr, fmtId );
    }
    else if (( opCode == OP_MR ) && ( getBit( instr, 11 ))) {
        
        if ( getBit( instr, 12 )) fprintf( stdout, "C%d", Instr::mrArgField( instr ));
        else fprintf( stdout, "S%d", Instr::regBIdField( instr ));
    }
    else if (( opCode == OP_MR ) && ( ! getBit( instr, 11 ))) {
        
        fprintf( stdout, "R%d", getBitField( instr, 9, 4 ));
    }
}

//------------------------------------------------------------------------------------------------------------
// Instruction have operands. For most of the instructions this is the operand field with the defined
// addressing modes. For others it is highy instruction specific. The operand routine also has a parameter
// to specify in what radix a value is shown. Address offsets are however always printed in decimal.
//
//------------------------------------------------------------------------------------------------------------
void displayOperands( uint32_t instr, TokId fmtId = TOK_DEC ) {
    
    uint32_t opCode = getBitField( instr, 5, 6 );
    
    switch ( opCode ) {
            
        case OP_ADD:    case OP_SUB:    case OP_CMP:
        case OP_AND:    case OP_OR:     case OP_XOR:
        case OP_LD:     case OP_ST:     case OP_LDO:
        case OP_LDWR:   case OP_STWC:   case OP_PRB:
        case OP_LDPA: {
            
            fprintf( stdout, "," );
            
            if ( opCodeTab[ opCode ].flags & STORE_INSTR ) fprintf( stdout, "R%d", getBitField( instr, 9, 4 ));
            else displayOperandModeField( instr, fmtId );
            
        } break;
            
        case OP_LDIL:
        case OP_ADDIL: {
            
            fprintf( stdout, "," );
            printImmVal( getBitField( instr, 31, 22 ), fmtId );
            
        } break;
            
        case OP_SHLA: {
            
            fprintf( stdout, ",R%d,R%d", getBitField( instr, 27, 4 ), getBitField( instr, 31, 4 ));
            if ( getBitField( instr, 22, 2 ) > 0 ) fprintf( stdout, ",%d",  getBitField( instr, 22, 2 ));
            
        } break;
            
        case OP_EXTR:
        case OP_DEP: {
            
            fprintf( stdout, ",R%d", getBitField( instr, 31, 4 ));
            
            if ( getBit( instr, 11 )) fprintf( stdout, ",shamt" );
            else fprintf( stdout, ",%d", getBitField( instr, 27, 5 ));
            
            fprintf( stdout, ",%d", getBitField( instr, 22, 5 ));
            
        } break;
            
        case OP_DSR: {
            
            fprintf( stdout, ",R%d,R%d", Instr::regAIdField( instr ), Instr::regBIdField( instr ));
            
            if ( getBit( instr, 11 )) fprintf( stdout, ",shmat" );
            else fprintf( stdout, ",%d", getBitField( instr, 22, 5 ));
            
        } break;
            
        case OP_LSID: {
            
            fprintf( stdout, ",R%d", getBitField( instr, 31, 4 ));
            
        } break;
            
        case OP_CMR: {
            
            fprintf( stdout, ",R%d", getBitField( instr, 31, 4 ));
            
        } break;
            
        case OP_LDWA: {
            
            fprintf( stdout, "," );
            printImmVal( immGenPosLenLowSign( instr, 27, 18 ), TOK_DEC );
            fprintf( stdout, "(R%d)", getBitField( instr, 31, 4 ));
            
        } break;
            
        case OP_LDWAX: {
            
            fprintf( stdout, ",R%d(R%d)", getBitField( instr, 27, 4 ), getBitField( instr, 31, 4 ));
        
        } break;

        case OP_STWA: {
            
            fprintf( stdout, ",R%d", getBitField( instr, 9, 4 ));
            
        } break;
            
        case OP_B: 
        case OP_GATE:
        case OP_BL: {
            
            printImmVal( immGenPosLenLowSign( instr, 31, 22 ), TOK_DEC );
                        
        } break;
            
        case OP_BR: {
            
            fprintf( stdout, "(R%d)", getBitField( instr, 31, 4 ));
            
        } break;
            
        case OP_BLR: {
            
            fprintf( stdout, ",(R%d)", getBitField( instr, 31, 4 ));
            
        } break;
            
        case OP_BV: {
            
            fprintf( stdout, "(R%d)", getBitField( instr, 31, 4 ));
            
        } break;
            
        case OP_BVR: {
            
            fprintf( stdout, "(R%d,R%d)", getBitField( instr, 27, 4 ), getBitField( instr, 31, 4 ));
            
        } break;
            
        case OP_BE:
        case OP_BLE: {
            
            printImmVal( immGenPosLenLowSign( instr, 23, 18 ));
            fprintf( stdout, ",(S%d,R%d)", getBitField( instr, 27, 4 ), getBitField( instr, 31, 4 ));
            
        } break;
            
        case OP_CBR: {
            
            fprintf( stdout, "R%d,R%d,", getBitField( instr, 27, 4 ), getBitField( instr, 31, 4 ));
            printImmVal( immGenPosLenLowSign( instr, 23, 15 ));
            
        } break;
            
        case OP_TBR: {
            
            fprintf( stdout, "R%d,", getBitField( instr, 31, 4 ));
            printImmVal( immGenPosLenLowSign( instr, 23, 15 ));
            
        } break;
            
        case OP_MR: {
            
            if ( getBit( instr, 11 )) fprintf( stdout, ",R%d", getBitField( instr, 9, 4 ));
            else {
                
                if ( getBit( instr, 12 )) fprintf( stdout, ",C%d", getBitField( instr, 31, 5 ));
                else fprintf( stdout, ",S%d", getBitField( instr, 31, 4 ));
            }
            
        } break;
            
        case OP_MST: {
            
            fprintf( stdout, "," );
            switch( getBitField( instr, 11, 2 )) {
                    
                case 0:  fprintf( stdout, "R%d", getBitField( instr, 31, 4 ));  break;
                case 1:
                case 2:  fprintf( stdout, "0x%x4", getBitField( instr, 31, 6 )); break;
                default: fprintf( stdout, "***" );
            }
            
        } break;
            
        case OP_ITLB: {
            
            fprintf( stdout, "R%d,", getBitField( instr, 9, 4 ));
            fprintf( stdout, "(S%d, R%d)", getBitField( instr, 27, 4 ), getBitField( instr, 31, 4 ));
            
        } break;
            
        case OP_PTLB: {
            
            fprintf( stdout, "(S%d, R%d)", getBitField( instr, 27, 4 ), getBitField( instr, 31, 4 ));
            
        } break;
            
        case OP_PCA: {
            
            fprintf( stdout, "(S%d, R%d)", getBitField( instr, 27, 4 ), getBitField( instr, 31, 4 ));
            
        } break;
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

