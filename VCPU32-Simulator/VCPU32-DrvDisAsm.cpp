//------------------------------------------------------------------------------------------------------------
//
// VCPU32 - A 32-bit CPU - Disassembler
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
// VCPU32 - A 32-bit CPU - Disassembler
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
#include "VCPU32-Driver.h"
#include "VCPU32-Core.h"

//
// ??? it would be better to produce a string rather than printing to stdout....
//

//------------------------------------------------------------------------------------------------------------
// Local namespace. These routines are not visible outside this source file.
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
   
    return( lowSignExtend32( getBitField( instr, pos, len ), len ));
}

//------------------------------------------------------------------------------------------------------------
// "printImmVal" display an immediate value in the selected radix. Octals and hex numbers are printed unsigned
// quantities, decimal numbers are interpreted as signed integers. Most often decimal notation is used to
// specify offsets on indexed addressing modes.
//
//------------------------------------------------------------------------------------------------------------
void printImmVal( uint32_t val, int rdx = 16 ) {
    
    if ( val == 0 ) fprintf( stdout, "0" );
    
    else {
       
        if      ( rdx == 10 )  fprintf( stdout, "%d", ((int) val ));
        else if ( rdx == 8  )  fprintf( stdout, "%#0o", val );
        else if ( rdx == 16 )  fprintf( stdout, "%#0x", val );
        else                   fprintf( stdout, "**num***" );
    }
}

//------------------------------------------------------------------------------------------------------------
// A little helper function to display the comparison condition in human readable form. We only decode the
// two bits which map to EQ, NE, LT and LE. A possible GT and GE cases cannot be deduced from just looking
// at the instruction.
//
//------------------------------------------------------------------------------------------------------------
void displayComparisonCodes( uint32_t cmpCode ) {
    
    switch( cmpCode ) {
            
        case CC_EQ:  fprintf( stdout, "EQ" ); return;
        case CC_LT:  fprintf( stdout, "LT" ); return;
        case CC_NE:  fprintf( stdout, "NE" ); return;
        case CC_LE:  fprintf( stdout, "LE" ); return;
        default:     fprintf( stdout, "**" ); return;
    }
}

//------------------------------------------------------------------------------------------------------------
// A little helper function to display the test condition in human readable form.
//
// ??? this is currently only used by the CMR instruction ... to think about ...
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
// There are instructions that use the operand argument format. This routine will format such an operand.
//
//------------------------------------------------------------------------------------------------------------
void displayOperandModeField( uint32_t instr, int rdx = 10 ) {
    
    uint32_t opMode = getBitField( instr, 13, 2 );
    
    switch ( opMode ) {
            
        case OP_MODE_IMM: {
            
            printImmVal( immGenPosLenLowSign( instr, 31, 18 ), TOK_DEC );
            
        } break;
          
        case OP_MODE_REG: {
          
            fprintf( stdout, "r%d, r%d", getBitField( instr, 27, 4 ), getBitField( instr, 31, 4 ));
            
        } break;
            
        case OP_MODE_REG_INDX: {
           
            fprintf( stdout, "r%d(r%d)", getBitField( instr, 27, 4 ), getBitField( instr, 31, 4 ));
            
        } break;
            
        case OP_MODE_INDX: {
            
            printImmVal( immGenPosLenLowSign( instr, 27, 12 ), TOK_DEC );
            fprintf( stdout, "(r%d)", getBitField( instr, 31, 4 ));
            
        } break;
    }
}

//------------------------------------------------------------------------------------------------------------
// Each instruction has an opCode. For most of the instructions, the mnemonic is just a simple mapping to the
// name stored in the opCode table. However, for some instructions we need to look at more options in the
// instruction word to come up with the mnemonic. Currently we append to the opCode that allow for a word
// length to append a character to indicate byte, half-word or word access.
//
// There are also instructions have the same opCode but result in a different mnemonic. For example the MR
// instruction will decode to four different mnemonics.
//
//------------------------------------------------------------------------------------------------------------
void displayOpCode( uint32_t instr ) {
    
    uint32_t opCode = getBitField( instr, 5, 6 );
    
    if ( opCodeTab[ opCode ].flags & OP_MODE_INSTR ) {
        
        fprintf( stdout, "%s", opCodeTab[ opCode ].mnemonic );
        
        if (( getBitField( instr, 13, 2 ) == 2 ) || ( getBitField( instr, 13, 2 ) == 3 )) {
            
            switch ( getBitField( instr, 15, 2 )) {
                    
                case 0:  fprintf( stdout, "B" ); break;
                case 1:  fprintf( stdout, "H" ); break;
                case 2:  break;
                default: fprintf( stdout, "**dw**" );
            }
        }
    }
    else {
        
        switch ( opCode ) {
                
            case OP_LD: 
            case OP_ST: {
                
                fprintf( stdout, "%s", opCodeTab[ opCode ].mnemonic );
                
                switch ( getBitField( instr, 15, 2 )) {
                        
                    case 0:  fprintf( stdout, "B" ); break;
                    case 1:  fprintf( stdout, "H" ); break;
                    case 2:  break;
                    default: fprintf( stdout, "**dw**" );
                }
                
            } break;
            
            default: fprintf( stdout, "%s", opCodeTab[ opCode ].mnemonic );
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
            
        case OP_LD:     
        case OP_ST:
        case OP_LDA:    
        case OP_STA: {
            
            if ( getBit( instr, 11 )) fprintf( stdout, ".M" );
            
        } break;
            
        case OP_ADD:    
        case OP_ADC:
        case OP_SUB:    
        case OP_SBC: {
            
            if ( getBitField( instr, 11, 2 ) > 0 ) {
                
                fprintf( stdout, "." );
                if ( getBit( instr, 10 ))   fprintf( stdout, "L" );
                if ( getBit( instr, 11 ))   fprintf( stdout, "O" );
            }
            
        } break;
            
        case OP_AND:
        case OP_OR: {
            
            if ( getBitField( instr, 11, 2 ) > 0 ) {
                
                fprintf( stdout, "." );
                if ( getBit( instr, 10 )) fprintf( stdout, "N" );
                if ( getBit( instr, 11 )) fprintf( stdout, "C" );
            }
            
        } break;
        
        case OP_XOR: {
            
            if ( getBit( instr, 10 )) fprintf( stdout, ".N" );
            
        } break;
            
        case OP_CMP: 
        case OP_CMPU: {
            
            fprintf( stdout, "." );
            displayComparisonCodes( getBitField( instr, 11, 2 ));
            
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
                if ( getBit( instr, 10 ))   fprintf( stdout, "I" );
                if ( getBit( instr, 11 ))   fprintf( stdout, "L" );
                if ( getBit( instr, 12 ))   fprintf( stdout, "O" );
            }
            
        } break;
            
        case OP_CMR: {
            
            fprintf( stdout, "." );
            displayTestCodes( getBitField( instr, 13, 4 ));
            
        } break;
            
        case OP_CBR: 
        case OP_CBRU: {
            
            fprintf( stdout, "." );
            displayComparisonCodes( getBitField( instr, 7, 2 ));
            
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
            
            if (( getBit( instr, 10 ) || getBit( instr, 11 ))) {
                
                fprintf( stdout, "." );
                if ( getBit( instr, 10 )) fprintf( stdout, "W" );
                if ( getBit( instr, 11 ))fprintf( stdout, "I" );
            }
           
        } break;
            
        case OP_ITLB: {
            
            if ( getBit( instr, 10 )) fprintf( stdout, ".T" );
           
        } break;
            
        case OP_PTLB: {
            
            if ( getBit( instr, 10 )) fprintf( stdout, ".T" );
          
        } break;
            
        case OP_PCA: {
            
            if (( getBit( instr, 10 ) || getBit( instr, 11 ))) {
                
                fprintf( stdout, "." );
                if ( getBit( instr, 10 )) fprintf( stdout, "T" );
                if ( getBit( instr, 11 )) fprintf( stdout, "M" );
                if ( getBit( instr, 14 )) fprintf( stdout, "F" );
            }
            
        } break;
    }
    
    fprintf( stdout, " " );
}

//------------------------------------------------------------------------------------------------------------
// This routine display the instruction target. Most of the time it is a general register. For the STORE
// type instructions the target address is decoded and printed. Finally there are the MTR instructions which
// which will use a segment or control register as the target. There is one further exception. The BLE
// instruction will produce a register value, the return link stored in R0. This is however not shown in the
// disassembly printout.
//
//------------------------------------------------------------------------------------------------------------
void displayTarget( uint32_t instr, int rdx = 10 ) {
    
    uint32_t opCode = getBitField( instr, 5, 6 );
    
    if (( opCodeTab[ opCode ].flags & REG_R_INSTR ) && ( ! ( opCodeTab[ opCode ].flags & BRANCH_INSTR ))) {
        
        fprintf( stdout, "r%d", getBitField( instr, 9, 4 ));
    }
    else if ( opCodeTab[ opCode ].flags & STORE_INSTR ) {
        
        fprintf( stdout, "r%d", getBitField( instr, 9, 4 ));
    }
    else if ( opCode == OP_MR ) {
        
        if ( getBit( instr, 10 )) {
            
            if ( getBit( instr, 11 )) fprintf( stdout, "c%d", getBitField( instr, 31, 5 ));
            else fprintf( stdout, "s%d", getBitField( instr, 31, 4 ));
        }
        else fprintf( stdout, "r%d", getBitField( instr, 9, 4 ));
    }
}

//------------------------------------------------------------------------------------------------------------
// Instruction have operands. For most of the instructions this is the operand field with the defined
// addressing modes. For others it is highly instruction specific. The operand routine also has a parameter
// to specify in what radix a value is shown. Address offsets are however always printed in decimal.
//
//------------------------------------------------------------------------------------------------------------
void displayOperands( uint32_t instr, int rdx = 10 ) {
    
    uint32_t opCode = getBitField( instr, 5, 6 );
    
    switch ( opCode ) {
            
        case OP_ADD:    case OP_ADC:    case OP_SUB:    case OP_SBC:    case OP_CMP:
        case OP_CMPU:   case OP_AND:    case OP_OR:     case OP_XOR: {
            
            fprintf( stdout, ", " );
            displayOperandModeField( instr, rdx );
            
        } break;
            
        case OP_EXTR: {
            
            fprintf( stdout, ", r%d", getBitField( instr, 31, 4 ));
            
            if ( ! getBit( instr, 11 )) {
                
                fprintf( stdout, ", %d", getBitField( instr, 27, 5 ));
                fprintf( stdout, ", %d", getBitField( instr, 21, 5 ));
                
            } else fprintf( stdout, ", %d", getBitField( instr, 21, 5 ));
            
        } break;
            
        case OP_DEP: {
            
            if ( getBit( instr, 12 ))   fprintf( stdout, ", %d", getBitField( instr, 31, 4 ));
            else                        fprintf( stdout, ", r%d", getBitField( instr, 31, 4 ));
            
            if ( ! getBit( instr, 11 )) {
                
                fprintf( stdout, ", %d", getBitField( instr, 27, 5 ));
                fprintf( stdout, ", %d", getBitField( instr, 21, 5 ));
                
            } else fprintf( stdout, ", %d", getBitField( instr, 21, 5 ));
            
        } break;
            
        case OP_DSR: {
            
            fprintf( stdout, ", r%d, r%d", getBitField( instr, 27, 4 ), getBitField( instr, 31, 4 ));
            if ( ! getBit( instr, 11 )) fprintf( stdout, ", %d", getBitField( instr, 21, 5 ));
            
        } break;
            
        case OP_DS: {
            
            fprintf( stdout, ", r%d, r%d", getBitField( instr, 27, 4 ), getBitField( instr, 31, 4 ));
            
        } break;
            
        case OP_LSID: {
            
            fprintf( stdout, ", r%d", getBitField( instr, 31, 4 ));
            
        } break;
            
        case OP_CMR: {
            
            fprintf( stdout, ", r%d", getBitField( instr, 27, 4 ));
            fprintf( stdout, ", r%d", getBitField( instr, 31, 4 ));
            
        } break;
            
        case OP_DIAG: {
            
            fprintf( stdout, ",r%d, r%d, r%d, %d",
                    getBitField( instr, 9, 4  ),
                    getBitField( instr, 27, 4 ),
                    getBitField( instr, 31, 4 ),
                    getBitField( instr, 13, 4 ));
        } break;
            
        case OP_LD: case OP_ST: case OP_LDR: case OP_STC: {
            
            if ( getBit( instr, 10 )) {
                
                if ( getBitField( instr, 13, 2 ) == 0 ) {
                    
                    fprintf( stdout, ", r%d(r%d)", getBitField( instr, 27, 4 ), getBitField( instr, 31, 4 ));
                }
                else {
                    
                    fprintf( stdout, ", r%d(s%d, r%d)",
                            getBitField( instr, 27, 4 ),
                            getBitField( instr, 13, 2 ),
                            getBitField( instr, 31, 4 ));
                }
            }
            else {
                
                fprintf( stdout, ", " );
                printImmVal(  immGenPosLenLowSign( instr, 27, 12 ), TOK_DEC );
               
                if ( getBitField( instr, 13, 2 ) == 0 ) {
                    
                    fprintf( stdout, "(r%d)", getBitField( instr, 31, 4 ));
                }
                else {
                    
                    fprintf( stdout, "(s%d, r%d)", getBitField( instr, 13, 2 ), getBitField( instr, 31, 4 ));
                }
            }
        
        } break;
            
        case OP_LDA: case OP_STA: {
            
            if ( getBit( instr, 10 )) {
                
                fprintf( stdout, ", r%d(r%d)", getBitField( instr, 27, 4 ), getBitField( instr, 31, 4 ));
            }
            else {
                
                fprintf( stdout, ", " );
                printImmVal( immGenPosLenLowSign( instr, 27, 12 ), TOK_DEC );
                fprintf( stdout, "(r%d)", getBitField( instr, 31, 4 ));
            }
        
        } break;
            
        case OP_SHLA: {
            
            fprintf( stdout, ", r%d, r%d", getBitField( instr, 27, 4 ), getBitField( instr, 31, 4 ));
            if ( getBitField( instr, 21, 2 ) > 0 ) fprintf( stdout, ", %d",  getBitField( instr, 21, 2 ));
            
        } break;
            
        case OP_LDIL:
        case OP_ADDIL: {
            
            fprintf( stdout, ", " );
            printImmVal( getBitField( instr, 31, 22 ), rdx );
            
        } break;
            
        case OP_LDO: {
            
            fprintf( stdout, ", " );
            printImmVal( immGenPosLenLowSign( instr, 27, 18 ));
            fprintf( stdout, "(r%d)", getBitField( instr, 31, 4 ));
            
        } break;
            
        case OP_B: 
        case OP_GATE: {
            
            printImmVal( immGenPosLenLowSign( instr, 31, 22 ) << 2, TOK_DEC );
            if ( getBitField( instr, 9, 4 ) > 0 ) fprintf( stdout, ", r%d", getBitField( instr, 9, 4 ));
        
        } break;
            
        case OP_BR: {
           
            fprintf( stdout, "(r%d)", getBitField( instr, 31, 4 ));
            if ( getBitField( instr, 9, 4 ) > 0 ) fprintf( stdout, ", r%d", getBitField( instr, 9, 4 ));
            
        } break;
            
        case OP_BV: {
           
            fprintf( stdout, "(r%d)", getBitField( instr, 31, 4 ));
            if ( getBitField( instr, 9, 4 ) > 0 ) fprintf( stdout, ", r%d", getBitField( instr, 9, 4 ));
            
        } break;
            
        case OP_BE: {
            
            printImmVal( immGenPosLenLowSign( instr, 23, 14 ) << 2, TOK_DEC );
            fprintf( stdout, "(s%d,r%d)", getBitField( instr, 27, 4 ), getBitField( instr, 31, 4 ));
            if ( getBitField( instr, 9, 4 ) > 0 ) fprintf( stdout, ", r%d", getBitField( instr, 9, 4 ));
            
        } break;
            
        case OP_BVE: {
            
            if ( getBitField( instr, 27,4 )) fprintf( stdout, "r%d", getBitField( instr, 27,4 ));
            fprintf( stdout, "(r%d)", getBitField( instr, 31,4 ));
            if ( getBitField( instr, 9, 4 ) > 0 ) fprintf( stdout, ", r%d", getBitField( instr, 9, 4 ));
            
        } break;
            
        case OP_CBR: 
        case OP_CBRU: {
            
            fprintf( stdout, "r%d, r%d,", getBitField( instr, 27, 4 ), getBitField( instr, 31, 4 ));
            printImmVal( immGenPosLenLowSign( instr, 23, 15 ) << 2 );
            
        } break;
            
        case OP_MR: {
            
            if ( getBit( instr, 10 )) fprintf( stdout, ", r%d", getBitField( instr, 9, 4 ));
            else {
                
                if ( getBit( instr, 11 )) fprintf( stdout, ", c%d", getBitField( instr, 31, 5 ));
                else fprintf( stdout, ", s%d", getBitField( instr, 31, 3 ));
            }
            
        } break;
            
        case OP_MST: {
            
            fprintf( stdout, "," );
            switch( getBitField( instr, 11, 2 )) {
                    
                case 0:  fprintf( stdout, "r%d", getBitField( instr, 31, 4 ));  break;
                case 1:
                case 2:  fprintf( stdout, "0x%x", getBitField( instr, 31, 6 )); break;
                default: fprintf( stdout, "***" );
            }
            
        } break;
            
        case OP_PRB: {
            
            if ( getBitField( instr, 13, 2 ) > 0 ) {
                
                fprintf( stdout, ", (s%d, r%d)", getBitField(instr, 13, 2 ), getBitField( instr, 31, 4 ));
            }
            else fprintf( stdout, ", (r%d)", getBitField( instr, 31, 4 ));
            
            if ( getBit( instr, 11 ))   fprintf( stdout, ", %d", getBit( instr, 27 ));
            else                        fprintf( stdout, ", r%d", getBitField( instr, 27, 4 ));
            
        } break;
        
        case OP_LDPA: {
            
            if ( getBitField( instr, 27, 4 ) != 0 ) fprintf( stdout, "r%d", getBitField( instr, 27, 4 ));
            
            if ( getBitField( instr, 13, 2 ) > 0 ) {
                
                fprintf( stdout, "(s%d, r%d)", getBitField(instr, 13, 2 ), getBitField( instr, 31, 4 ));
            }
            else fprintf( stdout, "(r%d)", getBitField( instr, 31, 4 ));
                                                          
        } break;
            
        case OP_ITLB: {
            
            fprintf( stdout, "r%d, ", getBitField( instr, 9, 4 ));
            fprintf( stdout, "(s%d,r%d)", getBitField( instr, 27, 4 ), getBitField( instr, 31, 4 ));
            
        } break;
            
        case OP_PTLB:
        case OP_PCA:{
            
            if ( getBitField( instr, 27, 4 ) != 0 ) fprintf( stdout, "r%d", getBitField( instr, 27, 4 ));
            
            if ( getBitField( instr, 13, 2 ) > 0 ) {
                
                fprintf( stdout, "(s%d, r%d)", getBitField(instr, 13, 2 ), getBitField( instr, 31, 4 ));
            }
            else fprintf( stdout, "(r%d)", getBitField( instr, 31, 4 ));
            
        } break;
            
        case OP_BRK: {
            
            fprintf( stdout, "%d, %d", getBitField( instr, 9, 4 ), getBitField( instr, 31, 16 ));
           
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

void DrvDisAsm::displayTargetAndOperands( uint32_t instr, int rdx ) {
    
    displayTarget( instr, rdx );
    displayOperands( instr, rdx );
}

void DrvDisAsm::displayInstr( uint32_t instr, int rdx ) {
    
    displayOpCodeAndOptions( instr );
    displayTargetAndOperands( instr, rdx );
}

int DrvDisAsm::getOpCodeOptionsFieldWidth( ) {
    
    return( 12 );
}

int DrvDisAsm::getTargetAndOperandsFieldWidth( ) {
    
    return( 16 );
}
