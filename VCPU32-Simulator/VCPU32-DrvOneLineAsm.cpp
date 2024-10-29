//------------------------------------------------------------------------------------------------------------
//
// VCPU32 - A 32-bit CPU - One Line Assembler
//
//------------------------------------------------------------------------------------------------------------
// The one line assembler assembles an instruction without further context. It is intended to for testing
// instructions in the simulator. There is no symbol table or any concept of assembling multiple instructions.
// The instruction to generate test is completely self sufficient. The parser is a straightforward recursive
// descendant parser, LL1 grammar.
//
//------------------------------------------------------------------------------------------------------------
//
// VCPU32 - A 32-bit CPU - One Line Assembler
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

//------------------------------------------------------------------------------------------------------------
// Local namespace. These routines are not visible outside this source file.
//
//------------------------------------------------------------------------------------------------------------
namespace {

//------------------------------------------------------------------------------------------------------------
// The global token table or the one line assembler. All reserved words are allocated in this table. Each
// entry has the token name, the token id, the token tye id, i.e. its type, and a value associatd with the
// token. The value allows for a constant token. The parser can directly use the value in expressions.
//
//------------------------------------------------------------------------------------------------------------
DrvToken const asmTokTab[ ] = {
    
    //--------------------------------------------------------------------------------------------------------
    // General registers.
    //
    //--------------------------------------------------------------------------------------------------------
    { .name = "R0",             .typ = TOK_TYP_GREG,         .tid = GR_0,                .val = 0           },
    { .name = "R1",             .typ = TOK_TYP_GREG,         .tid = GR_1,                .val = 1           },
    { .name = "R2",             .typ = TOK_TYP_GREG,         .tid = GR_2,                .val = 2           },
    { .name = "R3",             .typ = TOK_TYP_GREG,         .tid = GR_3,                .val = 3           },
    { .name = "R4",             .typ = TOK_TYP_GREG,         .tid = GR_4,                .val = 4           },
    { .name = "R5",             .typ = TOK_TYP_GREG,         .tid = GR_5,                .val = 5           },
    { .name = "R6",             .typ = TOK_TYP_GREG,         .tid = GR_6,                .val = 6           },
    { .name = "R7",             .typ = TOK_TYP_GREG,         .tid = GR_7,                .val = 7           },
    { .name = "R8",             .typ = TOK_TYP_GREG,         .tid = GR_8,                .val = 8           },
    { .name = "R9",             .typ = TOK_TYP_GREG,         .tid = GR_9,                .val = 9           },
    { .name = "R10",            .typ = TOK_TYP_GREG,         .tid = GR_10,               .val = 10          },
    { .name = "R11",            .typ = TOK_TYP_GREG,         .tid = GR_11,               .val = 11          },
    { .name = "R12",            .typ = TOK_TYP_GREG,         .tid = GR_12,               .val = 12          },
    { .name = "R13",            .typ = TOK_TYP_GREG,         .tid = GR_13,               .val = 13          },
    { .name = "R14",            .typ = TOK_TYP_GREG,         .tid = GR_14,               .val = 14          },
    { .name = "R15",            .typ = TOK_TYP_GREG,         .tid = GR_15,               .val = 15          },

    //--------------------------------------------------------------------------------------------------------
    // Segment registers.
    //
    //--------------------------------------------------------------------------------------------------------
    { .name = "S0",             .typ = TOK_TYP_SREG,         .tid = SR_0,                .val = 0           },
    { .name = "S1",             .typ = TOK_TYP_SREG,         .tid = SR_1,                .val = 1           },
    { .name = "S2",             .typ = TOK_TYP_SREG,         .tid = SR_2,                .val = 2           },
    { .name = "S3",             .typ = TOK_TYP_SREG,         .tid = SR_3,                .val = 3           },
    { .name = "S4",             .typ = TOK_TYP_SREG,         .tid = SR_4,                .val = 4           },
    { .name = "S5",             .typ = TOK_TYP_SREG,         .tid = SR_5,                .val = 5           },
    { .name = "S6",             .typ = TOK_TYP_SREG,         .tid = SR_6,                .val = 6           },
    { .name = "S7",             .typ = TOK_TYP_SREG,         .tid = SR_7,                .val = 7           },
    
    //--------------------------------------------------------------------------------------------------------
    // Control registers.
    //
    //--------------------------------------------------------------------------------------------------------
    { .name = "C0",             .typ = TOK_TYP_CREG,        .tid = CR_0,                .val = 0            },
    { .name = "C1",             .typ = TOK_TYP_CREG,        .tid = CR_1,                .val = 1            },
    { .name = "C2",             .typ = TOK_TYP_CREG,        .tid = CR_2,                .val = 2            },
    { .name = "C3",             .typ = TOK_TYP_CREG,        .tid = CR_3,                .val = 3            },
    { .name = "C4",             .typ = TOK_TYP_CREG,        .tid = CR_4,                .val = 4            },
    { .name = "C5",             .typ = TOK_TYP_CREG,        .tid = CR_5,                .val = 5            },
    { .name = "C6",             .typ = TOK_TYP_CREG,        .tid = CR_6,                .val = 6            },
    { .name = "C7",             .typ = TOK_TYP_CREG,        .tid = CR_7,                .val = 7            },
    { .name = "C8",             .typ = TOK_TYP_CREG,        .tid = CR_8,                .val = 8            },
    { .name = "C9",             .typ = TOK_TYP_CREG,        .tid = CR_9,                .val = 9            },
    { .name = "C10",            .typ = TOK_TYP_CREG,        .tid = CR_10,               .val = 10           },
    { .name = "C11",            .typ = TOK_TYP_CREG,        .tid = CR_11,               .val = 11           },
    { .name = "C12",            .typ = TOK_TYP_CREG,        .tid = CR_12,               .val = 12           },
    { .name = "C13",            .typ = TOK_TYP_CREG,        .tid = CR_13,               .val = 13           },
    { .name = "C14",            .typ = TOK_TYP_CREG,        .tid = CR_14,               .val = 14           },
    { .name = "C15",            .typ = TOK_TYP_CREG,        .tid = CR_15,               .val = 15           },
    { .name = "C16",            .typ = TOK_TYP_CREG,        .tid = CR_16,               .val = 16           },
    { .name = "C17",            .typ = TOK_TYP_CREG,        .tid = CR_17,               .val = 17           },
    { .name = "C18",            .typ = TOK_TYP_CREG,        .tid = CR_18,               .val = 18           },
    { .name = "C19",            .typ = TOK_TYP_CREG,        .tid = CR_19,               .val = 19           },
    { .name = "C20",            .typ = TOK_TYP_CREG,        .tid = CR_20,               .val = 20           },
    { .name = "C21",            .typ = TOK_TYP_CREG,        .tid = CR_21,               .val = 21           },
    { .name = "C22",            .typ = TOK_TYP_CREG,        .tid = CR_22,               .val = 22           },
    { .name = "C23",            .typ = TOK_TYP_CREG,        .tid = CR_23,               .val = 23           },
    { .name = "C24",            .typ = TOK_TYP_CREG,        .tid = CR_24,               .val = 24           },
    { .name = "C25",            .typ = TOK_TYP_CREG,        .tid = CR_25,               .val = 25           },
    { .name = "C26",            .typ = TOK_TYP_CREG,        .tid = CR_26,               .val = 26           },
    { .name = "C27",            .typ = TOK_TYP_CREG,        .tid = CR_27,               .val = 27           },
    { .name = "C28",            .typ = TOK_TYP_CREG,        .tid = CR_28,               .val = 28           },
    { .name = "C29",            .typ = TOK_TYP_CREG,        .tid = CR_29,               .val = 29           },
    { .name = "C30",            .typ = TOK_TYP_CREG,        .tid = CR_30,               .val = 30           },
    { .name = "C31",            .typ = TOK_TYP_CREG,        .tid = CR_31,               .val = 31           },
        
    //--------------------------------------------------------------------------------------------------------
    // Assembler mnemonics.
    //
    //--------------------------------------------------------------------------------------------------------
    { .name = "LD",             .typ = TOK_TYP_OP_CODE,     .tid = OP_CODE_LD,          .val = 0xC0000000   },
    { .name = "LDB",            .typ = TOK_TYP_OP_CODE,     .tid = OP_CODE_LDB,         .val = 0xC0000000   },
    { .name = "LDH",            .typ = TOK_TYP_OP_CODE,     .tid = OP_CODE_LDH,         .val = 0xC0000000   },
    { .name = "LDW",            .typ = TOK_TYP_OP_CODE,     .tid = OP_CODE_LDW,         .val = 0xC0000000   },
    { .name = "LDR",            .typ = TOK_TYP_OP_CODE,     .tid = OP_CODE_LDR,         .val = 0xD0000000   },
    { .name = "LDA",            .typ = TOK_TYP_OP_CODE,     .tid = OP_CODE_LDA,         .val = 0xC8000000   },
        
    { .name = "ST",             .typ = TOK_TYP_OP_CODE,     .tid = OP_CODE_ST,          .val = 0xC4000000   },
    { .name = "STB",            .typ = TOK_TYP_OP_CODE,     .tid = OP_CODE_STB,         .val = 0xC4000000   },
    { .name = "STH",            .typ = TOK_TYP_OP_CODE,     .tid = OP_CODE_STH,         .val = 0xC4000000   },
    { .name = "STW",            .typ = TOK_TYP_OP_CODE,     .tid = OP_CODE_STW,         .val = 0xC4000000   },
    { .name = "STC",            .typ = TOK_TYP_OP_CODE,     .tid = OP_CODE_STC,         .val = 0xD4000000   },
    { .name = "STA",            .typ = TOK_TYP_OP_CODE,     .tid = OP_CODE_STA,         .val = 0xCC000000   },
        
    { .name = "ADD",            .typ = TOK_TYP_OP_CODE,     .tid = OP_CODE_ADD,         .val = 0x40000000   },
    { .name = "ADDB",           .typ = TOK_TYP_OP_CODE,     .tid = OP_CODE_ADDB,        .val = 0x40000000   },
    { .name = "ADDH",           .typ = TOK_TYP_OP_CODE,     .tid = OP_CODE_ADDH,        .val = 0x40000000   },
    { .name = "ADDW",           .typ = TOK_TYP_OP_CODE,     .tid = OP_CODE_ADDW,        .val = 0x40000000   },
        
    { .name = "ADC",            .typ = TOK_TYP_OP_CODE,     .tid = OP_CODE_ADC,         .val = 0x44000000   },
    { .name = "ADCB",           .typ = TOK_TYP_OP_CODE,     .tid = OP_CODE_ADCB,        .val = 0x44000000   },
    { .name = "ADCH",           .typ = TOK_TYP_OP_CODE,     .tid = OP_CODE_ADCH,        .val = 0x44000000   },
    { .name = "ADCW",           .typ = TOK_TYP_OP_CODE,     .tid = OP_CODE_ADCW,        .val = 0x44000000   },
        
    { .name = "SUB",            .typ = TOK_TYP_OP_CODE,     .tid = OP_CODE_SUB,         .val = 0x48000000   },
    { .name = "SUBB",           .typ = TOK_TYP_OP_CODE,     .tid = OP_CODE_SUBB,        .val = 0x48000000   },
    { .name = "SUBH",           .typ = TOK_TYP_OP_CODE,     .tid = OP_CODE_SUBH,        .val = 0x48000000   },
    { .name = "SUBW",           .typ = TOK_TYP_OP_CODE,     .tid = OP_CODE_SUBW,        .val = 0x48000000   },
        
    { .name = "SBC",            .typ = TOK_TYP_OP_CODE,     .tid = OP_CODE_SBC,         .val = 0x4C000000   },
    { .name = "SBCB",           .typ = TOK_TYP_OP_CODE,     .tid = OP_CODE_SBCB,        .val = 0x4C000000   },
    { .name = "SBCH",           .typ = TOK_TYP_OP_CODE,     .tid = OP_CODE_SBCH,        .val = 0x4C000000   },
    { .name = "SBCW",           .typ = TOK_TYP_OP_CODE,     .tid = OP_CODE_SBCW,        .val = 0x4C000000   },
        
    { .name = "AND",            .typ = TOK_TYP_OP_CODE,     .tid = OP_CODE_AND,         .val = 0x50000000   },
    { .name = "ANDB",           .typ = TOK_TYP_OP_CODE,     .tid = OP_CODE_ANDB,        .val = 0x50000000   },
    { .name = "ANDH",           .typ = TOK_TYP_OP_CODE,     .tid = OP_CODE_ANDH,        .val = 0x50000000   },
    { .name = "ANDW",           .typ = TOK_TYP_OP_CODE,     .tid = OP_CODE_ANDW,        .val = 0x50000000   },
        
    { .name = "OR" ,            .typ = TOK_TYP_OP_CODE,     .tid = OP_CODE_OR,          .val = 0x54000000   },
    { .name = "ORB",            .typ = TOK_TYP_OP_CODE,     .tid = OP_CODE_ORB,         .val = 0x54000000   },
    { .name = "ORH",            .typ = TOK_TYP_OP_CODE,     .tid = OP_CODE_ORH,         .val = 0x54000000   },
    { .name = "ORW",            .typ = TOK_TYP_OP_CODE,     .tid = OP_CODE_ORW,         .val = 0x54000000   },
        
    { .name = "XOR" ,           .typ = TOK_TYP_OP_CODE,     .tid = OP_CODE_XOR,         .val = 0x58000000   },
    { .name = "XORB",           .typ = TOK_TYP_OP_CODE,     .tid = OP_CODE_XORB,        .val = 0x58000000   },
    { .name = "XORH",           .typ = TOK_TYP_OP_CODE,     .tid = OP_CODE_XORH,        .val = 0x58000000   },
    { .name = "XORW",           .typ = TOK_TYP_OP_CODE,     .tid = OP_CODE_XORW,        .val = 0x58000000   },
        
    { .name = "CMP" ,           .typ = TOK_TYP_OP_CODE,     .tid = OP_CODE_CMP,         .val = 0x5C000000   },
    { .name = "CMPB",           .typ = TOK_TYP_OP_CODE,     .tid = OP_CODE_CMPB,        .val = 0x5C000000   },
    { .name = "CMPH",           .typ = TOK_TYP_OP_CODE,     .tid = OP_CODE_CMPH,        .val = 0x5C000000   },
    { .name = "CMPW",           .typ = TOK_TYP_OP_CODE,     .tid = OP_CODE_CMPW,        .val = 0x5C000000   },
        
    { .name = "CMPU" ,          .typ = TOK_TYP_OP_CODE,     .tid = OP_CODE_CMPU,        .val = 0x60000000   },
    { .name = "CMPUB",          .typ = TOK_TYP_OP_CODE,     .tid = OP_CODE_CMPUB,       .val = 0x60000000   },
    { .name = "CMPUH",          .typ = TOK_TYP_OP_CODE,     .tid = OP_CODE_CMPUH,       .val = 0x60000000   },
    { .name = "CMPUW",          .typ = TOK_TYP_OP_CODE,     .tid = OP_CODE_CMPUW,       .val = 0x60000000   },
    
    { .name = "LSID",           .typ = TOK_TYP_OP_CODE,     .tid = OP_CODE_LSID,        .val = 0x10000000   },
    { .name = "EXTR",           .typ = TOK_TYP_OP_CODE,     .tid = OP_CODE_EXTR,        .val = 0x14000000   },
    { .name = "DEP",            .typ = TOK_TYP_OP_CODE,     .tid = OP_CODE_DEP,         .val = 0x18000000   },
    { .name = "DSR",            .typ = TOK_TYP_OP_CODE,     .tid = OP_CODE_DSR,         .val = 0x1C000000   },
    { .name = "SHLA",           .typ = TOK_TYP_OP_CODE,     .tid = OP_CODE_SHLA,        .val = 0x20000000   },
    { .name = "CMR",            .typ = TOK_TYP_OP_CODE,     .tid = OP_CODE_CMR,         .val = 0x24000000   },
        
    { .name = "LDIL",           .typ = TOK_TYP_OP_CODE,     .tid = OP_CODE_LDIL,        .val = 0x04000000   },
    { .name = "ADDIL",          .typ = TOK_TYP_OP_CODE,     .tid = OP_CODE_ADDIL,       .val = 0x08000000   },
    { .name = "LDO",            .typ = TOK_TYP_OP_CODE,     .tid = OP_CODE_LDO,         .val = 0x0C000000   },
    
    { .name = "B",              .typ = TOK_TYP_OP_CODE,     .tid = OP_CODE_B,           .val = 0x80000000   },
    { .name = "GATE",           .typ = TOK_TYP_OP_CODE,     .tid = OP_CODE_GATE,        .val = 0x84000000   },
    { .name = "BR",             .typ = TOK_TYP_OP_CODE,     .tid = OP_CODE_BR,          .val = 0x88000000   },
    { .name = "BV",             .typ = TOK_TYP_OP_CODE,     .tid = OP_CODE_BV,          .val = 0x8C000000   },
    { .name = "BE",             .typ = TOK_TYP_OP_CODE,     .tid = OP_CODE_BE,          .val = 0x90000000   },
    { .name = "BVE",            .typ = TOK_TYP_OP_CODE,     .tid = OP_CODE_BVE,         .val = 0x94000000   },
    { .name = "CBR",            .typ = TOK_TYP_OP_CODE,     .tid = OP_CODE_CBR,         .val = 0x98000000   },
    { .name = "CBRU",           .typ = TOK_TYP_OP_CODE,     .tid = OP_CODE_CBRU,        .val = 0x9C000000   },
        
    { .name = "MR",             .typ = TOK_TYP_OP_CODE,     .tid = OP_CODE_MR,          .val = 0x28000000   },
    { .name = "MST",            .typ = TOK_TYP_OP_CODE,     .tid = OP_CODE_MST,         .val = 0x2C000000   },
    { .name = "DS",             .typ = TOK_TYP_OP_CODE,     .tid = OP_CODE_DS,          .val = 0x30000000   },
    { .name = "LDPA",           .typ = TOK_TYP_OP_CODE,     .tid = OP_CODE_LDPA,        .val = 0xE4000000   },
    { .name = "PRB",            .typ = TOK_TYP_OP_CODE,     .tid = OP_CODE_PRB,         .val = 0xE8000000   },
    { .name = "ITLB",           .typ = TOK_TYP_OP_CODE,     .tid = OP_CODE_ITLB,        .val = 0xEC000000   },
    { .name = "PTLB",           .typ = TOK_TYP_OP_CODE,     .tid = OP_CODE_PTLB,        .val = 0xF0000000   },
    { .name = "PCA",            .typ = TOK_TYP_OP_CODE,     .tid = OP_CODE_PCA,         .val = 0xF4000000   },
    { .name = "DIAG",           .typ = TOK_TYP_OP_CODE,     .tid = OP_CODE_DIAG,        .val = 0xF8000000   },
    { .name = "RFI",            .typ = TOK_TYP_OP_CODE,     .tid = OP_CODE_RFI,         .val = 0xFC000000   },
    { .name = "BRK",            .typ = TOK_TYP_OP_CODE,     .tid = OP_CODE_BRK,         .val = 0x00000000   },
        
    //--------------------------------------------------------------------------------------------------------
    // Synthetic instruction mnemonics.
    //
    //--------------------------------------------------------------------------------------------------------
    { .name = "NOP",            .typ = TOK_TYP_OP_CODE_S,   .tid = OP_CODE_S_NOP,       .val =  0           },
    { .name = "SHL",            .typ = TOK_TYP_OP_CODE_S,   .tid = OP_CODE_S_SHL,       .val =  0           },
    { .name = "SHR",            .typ = TOK_TYP_OP_CODE_S,   .tid = OP_CODE_S_SHR,       .val =  0           },
    { .name = "ASL",            .typ = TOK_TYP_OP_CODE_S,   .tid = OP_CODE_S_ASL,       .val =  0           },
    { .name = "ASR",            .typ = TOK_TYP_OP_CODE_S,   .tid = OP_CODE_S_ASR,       .val =  0           },
    { .name = "ROR",            .typ = TOK_TYP_OP_CODE_S,   .tid = OP_CODE_S_ROR,       .val =  0           },
    { .name = "ROL",            .typ = TOK_TYP_OP_CODE_S,   .tid = OP_CODE_S_ROL,       .val =  0           },
    
    //--------------------------------------------------------------------------------------------------------
    // Runtime register mnemonics.
    //
    //--------------------------------------------------------------------------------------------------------
    // ??? to fill in ...
    
    //--------------------------------------------------------------------------------------------------------
    // The last token to mark the list end.
    //--------------------------------------------------------------------------------------------------------
    { .name = "",               .typ = TOK_NIL,             .tid = TOK_LAST,           .val =  0           }
    
};

//------------------------------------------------------------------------------------------------------------
// We need a tokenizer object. It is allocated only once. On each call to assmble an instruction, the
// tokenizer input is set for the current work task.
//
//------------------------------------------------------------------------------------------------------------
DrvTokenizer *tok = new DrvTokenizer( );

//------------------------------------------------------------------------------------------------------------
// Token flags. They are used to communicate additional information about the the token to the assembly
// process. Examples are the data width encoded in the opCode and the instruction mask.
//
//------------------------------------------------------------------------------------------------------------
enum TokenFlags : uint32_t {
  
    TF_NIL              = 0,
    TF_BYTE_INSTR       = ( 1U << 0 ),
    TF_HALF_INSTR       = ( 1U << 1 ),
    TF_WORD_INSTR       = ( 1U << 2 )
};

//------------------------------------------------------------------------------------------------------------
// Instruction encoding means to fiddle with bits and bit fields. Here is a set of helper functions.
//
//------------------------------------------------------------------------------------------------------------
bool getBit( uint32_t arg, int pos ) {
    
    return(( arg & ( 1U << ( 31 - ( pos % 32 )))) ? 1 : 0 );
}

void setBit( uint32_t *arg, int pos ) {
    
    *arg |= ( 1U << ( 31 - ( pos % 32 )));
}

void clearBit( uint32_t *arg, int pos ) {
    
    *arg &= ~( 1U << ( 31 - ( pos % 32 )));
}

void setBit( uint32_t *arg, int pos, bool val ) {
    
    if ( val )  setBit( arg, pos );
    else        clearBit( arg, pos );
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
    
    *arg = ( *arg & ( ~ ( tmpM << ( 31 - pos )))) | val;
}

void setImmVal( uint32_t *instr, int pos, int len, uint32_t val ) {
    
    setBit( instr, pos, (((int32_t) val < 0 ) ? 1 : 0 ));
    setBitField( instr, pos - 1, len - 1, val );
}

void setImmValU( uint32_t *instr, int pos, int len, uint32_t val ) {
    
    setBitField( instr, pos, len, val );
}

bool isInRange( int val, int low, int high ) {
    
    return(( val >= low ) && ( val <= high ));
}

bool isInRangeForBitField( int32_t val, uint8_t bitLen ) {
    
    int min = - ( 1 << (( bitLen -1 ) % 32 ));
    int max = ( 1 << (( bitLen - 1 ) %32 )) - 1;
    return(( val <= max ) && ( val >= min ));
}

bool isInRangeForBitFieldU( uint32_t val, uint8_t bitLen ) {
    
    int max = (( 1 << ( bitLen % 32 )) - 1 );
    return( val <= max );
}

//------------------------------------------------------------------------------------------------------------
// "parserError" is a little helper that prints out the error encountered. We will print the original
// input line, a caret marker where we found the error, and then return a false. Parsing errors typically
// result in aborting the parsing process. As this is a one line assembly, we do not need to be put effort
// into continuing reasonably with the parsing process.
//
//------------------------------------------------------------------------------------------------------------
bool parserError( char *errStr ) {
    
    fprintf( stdout, "%s\n", tok -> tokenLineStr( ));
    
    int i           = 0;
    int tokIndex    = tok -> tokCharIndex( );
    
    while ( i < tokIndex ) {
        
        fprintf( stdout, " " );
        i ++;
    }
    
    fprintf( stdout, "^\n" "%s\n", errStr );
    return( false );
}

//------------------------------------------------------------------------------------------------------------
// Check that the ASM line does not contain any extra tokens when the parser completed the analysis of the
// assembly line.
//
//------------------------------------------------------------------------------------------------------------
bool checkEOS( ) {
    
    if ( tok -> isToken( TOK_EOS )) return( true );
    else return( parserError((char *) "Extra tokens in the assembler line" ));
}

//------------------------------------------------------------------------------------------------------------
// Quite often the syntax has a construct that test the token and if correct get the next one.
//
//------------------------------------------------------------------------------------------------------------
bool acceptComma( ) {
    
    if ( tok -> isToken( TOK_COMMA )) {
        
        tok -> nextToken( );
        return( true );
    }
    else return( parserError((char *) "Expected a comma" ));
}

bool acceptLparen( ) {
    
    if ( tok -> isToken( TOK_LPAREN )) {
        
        tok -> nextToken( );
        return( true );
    }
    else return( parserError((char *) "Expected a left paren" ));
}

bool acceptRparen( ) {
    
    if ( tok -> isToken( TOK_RPAREN )) {
        
        tok -> nextToken( );
        return( true );
    }
    else return( parserError((char *) "Expected a right paren" ));
}

//------------------------------------------------------------------------------------------------------------
// "parseExpr" needs to be declared forward.
//
//------------------------------------------------------------------------------------------------------------
bool parseExpr( Expr *rExpr );

//------------------------------------------------------------------------------------------------------------
// "parseFactor" parses the factor syntax part of an expression.
//
//      <factor> -> <number>                        |
//                  <gregId>                        |
//                  <sregId>                        |
//                  <cregId>                        |
//                  "~" <factor>                    |
//                  "(" [ <sreg> "," ] <greg> ")"   |
//                  "(" <expr> ")"
//
//------------------------------------------------------------------------------------------------------------
bool parseFactor( Expr *rExpr ) {
    
    rExpr -> typ  = ET_NIL;
    rExpr -> val1 = 0;
    rExpr -> val2 = 0;
   
    if ( tok -> isTokenTyp( TOK_TYP_NUM ))  {
        
        rExpr -> typ = ET_NUM;
        rExpr -> val1 = tok -> tokVal( );
        tok -> nextToken( );
        return( true );
    }
    else  if ( tok -> isTokenTyp( TOK_TYP_GREG ))  {
        
        rExpr -> typ = ET_GREG;
        rExpr -> val1 = tok -> tokVal( );
        tok -> nextToken( );
        return( true );
    }
    else  if ( tok -> isTokenTyp( TOK_TYP_SREG ))  {
        
        rExpr -> typ = ET_SREG;
        rExpr -> val1 = tok -> tokVal( );
        tok -> nextToken( );
        return( true );
    }
    else  if ( tok -> isTokenTyp( TOK_TYP_CREG ))  {
        
        rExpr -> typ = ET_CREG;
        rExpr -> val1 = tok -> tokVal( );
        tok -> nextToken( );
        return( true );
    }
    else if ( tok -> isToken( TOK_NEG )) {
        
        parseFactor( rExpr );
        rExpr -> val1 = ~ rExpr -> val1;
        return( true );
    }
    else if ( tok -> isToken( TOK_LPAREN )) {
        
        tok -> nextToken( );
        if ( tok ->  isTokenTyp( TOK_TYP_SREG )) {
            
            rExpr -> typ    = ET_EXT_ADR;
            rExpr -> val1   = tok -> tokVal( );
            
            tok -> nextToken( );
            if ( ! acceptComma( )) return( false );
           
            if ( tok -> isTokenTyp( TOK_TYP_GREG )) {
                
                rExpr -> val2 = tok -> tokVal( );
                tok -> nextToken( );
            }
            else return( parserError((char *) "Expected a general reg" ));
        }
        else if ( tok -> isTokenTyp( TOK_TYP_GREG )) {
            
            rExpr -> typ = ET_ADR;
            rExpr -> val1 = tok -> tokVal( );
            tok -> nextToken( );
        }
        else if ( ! parseExpr( rExpr )) return( false );
        
        if ( ! acceptRparen( )) return( false );
        return( true );
    }
    else {
        
        parserError((char *) "Invalid factor in expression" );
        rExpr -> typ    = ET_NUM;
        rExpr -> val1   = 0;
        tok -> nextToken( );
        return( false );
    }
}

//------------------------------------------------------------------------------------------------------------
// "parseTerm" parses the term syntax.
//
//      <term>      ->  <factor> { <termOp> <factor> }
//      <termOp>    ->  "*" | "/" | "%" | "&"
//
//------------------------------------------------------------------------------------------------------------
bool parseTerm( Expr *rExpr ) {
    
    Expr lExpr;
    bool rStat;
    
    rStat = parseFactor( rExpr );
    
    while (( tok -> tokId( ) == TOK_MULT )   ||
           ( tok -> tokId( ) == TOK_DIV  )   ||
           ( tok -> tokId( ) == TOK_MOD  )   ||
           ( tok -> tokId( ) == TOK_AND  ))  {
        
        uint8_t op = tok -> tokId( );
        
        tok -> nextToken( );
        rStat = parseFactor( &lExpr );
        
        if ( rExpr -> typ != lExpr.typ ) {
            
            return ( parserError((char *) "Expression type mismatch" ));
        }
       
        switch( op ) {
                
            case TOK_MULT:   rExpr -> val1 = rExpr -> val1 * lExpr.val1; break;
            case TOK_DIV:    rExpr -> val1 = rExpr -> val1 / lExpr.val1; break;
            case TOK_MOD:    rExpr -> val1 = rExpr -> val1 % lExpr.val1; break;
            case TOK_AND:    rExpr -> val1 = rExpr -> val1 & lExpr.val1; break;
        }
    }
    
    return( true );
}

//------------------------------------------------------------------------------------------------------------
// "parseExpr" parses the expression syntax. The one line assembler parser routines use this call in many
// places where a numeric expression or an address is needed.
//
//      <expr>      ->  [ ( "+" | "-" ) ] <term> { <exprOp> <term> }
//      <exprOp>    ->  "+" | "-" | "|" | "^"
//
//------------------------------------------------------------------------------------------------------------
bool parseExpr( Expr *rExpr ) {
    
    Expr lExpr;
    bool rStat;
    
    if ( tok -> isToken( TOK_PLUS )) {
        
        tok -> nextToken( );
        rStat = parseTerm( rExpr );
        
        if ( ! ( rExpr -> typ == ET_NUM )) {
            
            return( parserError((char *) "Expected a numeric constant" ));
        }
    }
    else if ( tok -> isToken( TOK_MINUS )) {
        
        tok -> nextToken( );
        rStat = parseTerm( rExpr );
        
        if ( rExpr -> typ == ET_NUM ) rExpr -> val1 = - rExpr -> val1;
        else return( parserError((char *) "Expected a numeric constant" ));
    }
    else rStat = parseTerm( rExpr );
    
    while (( tok -> tokId( ) == TOK_PLUS    ) ||
           ( tok -> tokId( ) == TOK_MINUS   ) ||
           ( tok -> tokId( ) == TOK_OR      ) ||
           ( tok -> tokId( ) == TOK_XOR     )) {
        
        uint8_t op = tok -> tokId( );
        
        tok -> nextToken( );
        rStat = parseTerm( &lExpr );
        
        if ( rExpr -> typ != lExpr.typ ) {
            
            return ( parserError((char *) "Expression type mismatch" ));
        }
        
        switch ( op ) {
                
            case TOK_PLUS:   rExpr -> val1 = rExpr -> val1 + lExpr.val1; break;
            case TOK_MINUS:  rExpr -> val1 = rExpr -> val1 - lExpr.val1; break;
            case TOK_OR:     rExpr -> val1 = rExpr -> val1 | lExpr.val1; break;
            case TOK_XOR:    rExpr -> val1 = rExpr -> val1 ^ lExpr.val1; break;
        }
    }
    
    return( true );
}

//------------------------------------------------------------------------------------------------------------
// "parseInstrOptions" will analyze the opCode option string. An opCode string is a sequence of characters.
// We will look at each character in the "name" and set the options for the particular instruction. There are
// also cases where the only option is a multi-character sequence. We detect invalid options but not when
// the same option is repeated. E.g. a "LOL" will result in "L" and "O" set.
//
// ??? rework a little. The period is separate from this string...
//------------------------------------------------------------------------------------------------------------
bool parseInstrOptions( uint32_t *instr, uint32_t *flags ) {
    
    if ( ! tok -> isToken( TOK_IDENT )) {
        
        return( parserError((char *) "Expected the option qualifier(s)" ));
    }
    
    char *optBuf = tok -> tokStr( );
    
    switch( getBitField( *instr, 5, 6 )) {
            
        case OP_LD:
        case OP_ST:
        case OP_LDA:
        case OP_STA:  {
            
            if ( optBuf[ 0 ] == 'M' ) setBit( instr, 11 );
            else return( parserError((char *) "Invalid instruction option" ));
            
        } break;
            
        case OP_ADD:
        case OP_ADC:
        case OP_SUB:
        case OP_SBC: {
            
            for ( int i = 0; i < strlen( optBuf ); i ++ ) {
                
                if      ( optBuf[ i ] == 'L' ) setBit( instr, 10 );
                else if ( optBuf[ i ] == 'O' ) setBit( instr, 11 );
                else return( parserError((char *) "Invalid instruction option" ));
            }
            
        } break;
            
        case OP_AND:
        case OP_OR: {
            
            for ( int i = 0; i < strlen( optBuf ); i ++ ) {
                
                if      ( optBuf[ i ] == 'N' ) setBit( instr, 10 );
                else if ( optBuf[ i ] == 'C' ) setBit( instr, 11 );
                else return( parserError((char *) "Invalid instruction option" ));
            }
            
        } break;
            
        case OP_XOR: {
            
            if ( optBuf[ 0 ] == 'N' ) setBit( instr, 10 );
            else return( parserError((char *) "Invalid instruction option" ));
            
        } break;
            
        case OP_CMP:
        case OP_CMPU: {
            
            if      ( strcmp( optBuf, ((char *) "EQ" )) == 0 ) setBitField( instr, 11, 2, 0 );
            else if ( strcmp( optBuf, ((char *) "LT" )) == 0 ) setBitField( instr, 11, 2, 1 );
            else if ( strcmp( optBuf, ((char *) "NE" )) == 0 ) setBitField( instr, 11, 2, 2 );
            else if ( strcmp( optBuf, ((char *) "LE" )) == 0 ) setBitField( instr, 11, 2, 3 );
            else return( parserError((char *) "Invalid compare option" ));

        } break;
            
        case OP_CBR:
        case OP_CBRU: {
            
            if      ( strcmp( optBuf, ((char *) "EQ" )) == 0 ) setBitField( instr, 7, 2, 0 );
            else if ( strcmp( optBuf, ((char *) "LT" )) == 0 ) setBitField( instr, 7, 2, 1 );
            else if ( strcmp( optBuf, ((char *) "NE" )) == 0 ) setBitField( instr, 7, 2, 2 );
            else if ( strcmp( optBuf, ((char *) "LE" )) == 0 ) setBitField( instr, 7, 2, 3 );
            else return( parserError((char *) "Invalid compare option" ));

        } break;
            
        case OP_CMR: {
            
            if      ( strcmp( optBuf, ((char *) "EQ" )) == 0 ) setBitField( instr, 13, 4, 0 );
            else if ( strcmp( optBuf, ((char *) "LT" )) == 0 ) setBitField( instr, 13, 4, 1 );
            else if ( strcmp( optBuf, ((char *) "GT" )) == 0 ) setBitField( instr, 13, 4, 2 );
            else if ( strcmp( optBuf, ((char *) "EV" )) == 0 ) setBitField( instr, 13, 4, 3 );
            else if ( strcmp( optBuf, ((char *) "NE" )) == 0 ) setBitField( instr, 13, 4, 4 );
            else if ( strcmp( optBuf, ((char *) "LE" )) == 0 ) setBitField( instr, 13, 4, 5 );
            else if ( strcmp( optBuf, ((char *) "GE" )) == 0 ) setBitField( instr, 13, 4, 6 );
            else if ( strcmp( optBuf, ((char *) "OD" )) == 0 ) setBitField( instr, 13, 4, 7 );
            else return( parserError((char *) "Invalid test option" ));
            
        } break;
            
        case OP_EXTR: {
            
            for ( int i = 0; i < strlen( optBuf ); i ++ ) {
                
                if      ( optBuf[ i ] == 'S' ) setBit( instr, 10 );
                else if ( optBuf[ i ] == 'A' ) setBit( instr, 11 );
                else return( parserError((char *) "Invalid instruction option" ));
            }
            
        } break;
            
        case OP_DEP: {
            
            for ( int i = 0; i < strlen( optBuf ); i ++ ) {
                
                if      ( optBuf[ i ] == 'Z' ) setBit( instr, 10 );
                else if ( optBuf[ i ] == 'A' ) setBit( instr, 11 );
                else if ( optBuf[ i ] == 'I' ) setBit( instr, 12 );
                else return( parserError((char *) "Invalid instruction option" ));
            }
            
        } break;
            
        case OP_DSR: {
            
            if ( optBuf[ 0 ] == 'A' ) setBit( instr, 11 );
            else return( parserError((char *) "Invalid instruction option" ));
            
        } break;
            
        case OP_SHLA: {
            
            for ( int i = 0; i < strlen( optBuf ); i ++ ) {
                
                if      ( optBuf[ i ] == 'I' ) setBit( instr, 10 );
                else if ( optBuf[ i ] == 'L' ) setBit( instr, 11 );
                else if ( optBuf[ i ] == 'O' ) setBit( instr, 12 );
                else return( parserError((char *) "Invalid instruction option" ));
            }
            
        } break;
            
        case OP_MR: {
            
            for ( int i = 0; i < strlen( optBuf ); i ++ ) {
                
                if      ( optBuf[ i ] == 'D' ) setBit( instr, 10 );
                else if ( optBuf[ i ] == 'M' ) setBit( instr, 11 );
                else return( parserError((char *) "Invalid instruction option" ));
            }
            
        } break;
            
        case OP_MST: {
            
            for ( int i = 0; i < strlen( optBuf ); i ++ ) {
                
                if      ( optBuf[ i ] == 'S' ) setImmValU( instr, 11, 2, 1 );
                else if ( optBuf[ i ] == 'C' ) setImmValU( instr, 11, 2, 2 );
                else return( parserError((char *) "Invalid instruction option" ));
            }
    
        } break;
            
        case OP_PRB: {
            
            for ( int i = 0; i < strlen( optBuf ); i ++ ) {
                
                if      ( optBuf[ i ] == 'W' ) setBit( instr, 10 );
                else if ( optBuf[ i ] == 'I' ) setBit( instr, 11 );
                else return( parserError((char *) "Invalid instruction option" ));
            }
            
        } break;
            
        case OP_ITLB: {
            
            if ( optBuf[ 0 ] == 'T' ) setBit( instr, 11 );
            else return( parserError((char *) "Invalid instruction option" ));
            
        } break;
            
        case OP_PTLB: {
            
            for ( int i = 0; i < strlen( optBuf ); i ++ ) {
                
                if      ( optBuf[ i ] == 'T' ) setBit( instr, 10 );
                else if ( optBuf[ i ] == 'M' ) setBit( instr, 11 );
                else return( parserError((char *) "Invalid instruction option" ));
            }
            
        } break;
            
        case OP_PCA: {
            
            for ( int i = 0; i < strlen( optBuf ); i ++ ) {
                
                if      ( optBuf[ i ] == 'T' ) setBit( instr, 10 );
                else if ( optBuf[ i ] == 'M' ) setBit( instr, 11 );
                else if ( optBuf[ i ] == 'F' ) setBit( instr, 14 );
                else return( parserError((char *) "Invalid instruction option" ));
            }
            
        } break;
            
        default: return( parserError((char *) "Instruction has no option" ));
    }
    
    tok -> nextToken( );
    return( true );
}

//------------------------------------------------------------------------------------------------------------
// "parseLogicalAdr" analyzes a logical address, which is used by several instruction with a "seg" field.
//
//      "(" [ <segReg> "," ] <ofsReg> ")"
//
//------------------------------------------------------------------------------------------------------------
bool parseLogicalAdr( uint32_t *instr, uint32_t flags ) {
    
    Expr rExpr;
    
    if ( ! parseExpr( &rExpr )) return( false );
        
    if ( rExpr.typ == ET_EXT_ADR ) {
        
        setBitField( instr, 31, 4, rExpr.val2 );
        
        if ( isInRange( rExpr.val1, 1, 3 )) setBitField( instr, 13, 2, rExpr.val1 );
        else return( parserError((char *) "Expected SR1 .. SR3 " ));
    }
    else if ( rExpr.typ == ET_ADR ) {
        
        setBitField( instr, 31, 4, rExpr.val1 );
    }
    else return( parserError((char *) "Expected a logical address" ));
    
    return( true );
}

//------------------------------------------------------------------------------------------------------------
// "parseLoadStoreOperand" parses the operand portion of the load and store instruction family. It represents
// the source location for the load type instruction and the target for the store type instruction. The syntax
// for the <operand> portion is either a
//
//      <ofs> "(" SR "," GR ")"
//      <ofs> "(" GR ")"
//      <GR>  "(" SR "," GR ")"
//      <GR>  "(" GR ")"
//
// <loadInstr>  [ "." <opt> ] <targetReg>       "," <sourceOperand>
// <storeInstr> [ "." <opt> ] <targetOperand>   "," <sourceReg>
//
//------------------------------------------------------------------------------------------------------------
bool parseLoadStoreOperand( uint32_t *instr, uint32_t flags ) {
    
    Expr rExpr;
    
    if      ( flags & TF_BYTE_INSTR ) setBitField( instr, 15, 2, 0 );
    else if ( flags & TF_HALF_INSTR ) setBitField( instr, 15, 2, 1 );
    else if ( flags & TF_WORD_INSTR ) setBitField( instr, 15, 2, 2 );
  
    if ( ! parseExpr( &rExpr )) return( false );
    
    if ( rExpr.typ == ET_NUM ) {
        
        if ( isInRangeForBitField( rExpr.val1, 12 )) setImmVal( instr, 27, 12, rExpr.val1 );
        else return( parserError((char *) "Immediate value out of range" ));
        
        if ( ! parseExpr( &rExpr )) return( false );
    }
    else if ( rExpr.typ == ET_GREG ) {
        
        if (( getBitField( *instr, 5, 6 ) == OP_LDR ) || ( getBitField( *instr, 5, 6 ) == OP_LDR ))
            return( parserError((char *) "Register based offset is not allowed for this instruction" ));
        
        setBit( instr, 10 );
        setBitField( instr, 27, 4, rExpr.val1 );
        
        if ( ! parseExpr( &rExpr )) return( false );
    }
    
    if ( rExpr.typ == ET_ADR ) {
                    
        setBitField( instr, 13, 2, 0 );
        setBitField( instr, 31, 4, rExpr.val1 );
    }
    else if ( rExpr.typ == ET_EXT_ADR ) {
                    
        if (( getBitField( *instr, 5, 6 ) == OP_LDA ) || ( getBitField( *instr, 5, 6 ) == OP_STA )) {
                    
            return( parserError((char *) "Invalid address for instruction type" ));
        }
                
        if ( isInRange( rExpr.val1, 1, 3 )) setBitField( instr, 13, 2, rExpr.val1 );
        else return( parserError((char *) "Expected SR1 .. SR3 " ));
                    
        setBitField( instr, 31, 4, rExpr.val2 );
    }
    else return( parserError((char *) "Expected an address" ));
   
    return( true );
}

//------------------------------------------------------------------------------------------------------------
// "parseModeTypeInstr" parses all instructions that have an "operand" encoding. The syntax is as follows:
//
//      opCode [ "." <opt> ] <targetReg> "," <num>                                - mode 0
//      opCode [ "." <opt> ] <targetReg> "," <num> "(" <baseReg> ")"              - mode 3
//      opCode [ "." <opt> ] <targetReg> "," <sourceReg>                          - mode 1
//      opCode [ "." <opt> ] <targetReg> "," <sourceRegA> "," "<sourceRegB>       - mode 1
//      opCode [ "." <opt> ] <targetReg> "," <indexReg> "(" <baseReg> ")"         - mode 2
//
//------------------------------------------------------------------------------------------------------------
bool parseModeTypeInstr( uint32_t *instr, uint32_t flags ) {
    
    uint8_t targetRegId = 0;
    Expr    rExpr;
   
    if ( tok -> isTokenTyp( TOK_TYP_GREG )) {
        
        targetRegId = tok -> tokVal( );
        setBitField( instr, 9, 4, tok -> tokVal( ));
        tok -> nextToken( );
    }
    else return( parserError((char *) "Expected a general register" ));
    
    if ( ! acceptComma( )) return( false );
 
    if ( ! parseExpr( &rExpr )) return( false );
    
    if ( rExpr.typ == ET_NUM ) {
     
        if ( tok -> isToken( TOK_EOS )) {
            
            if ( isInRangeForBitField( rExpr.val1, 18 )) setImmVal( instr, 31, 18, rExpr.val1 );
            else return( parserError((char *) "Immediate value out of range" ));
        }
        else {
            
            if ( isInRangeForBitField( rExpr.val1, 12 )) setImmVal( instr, 27, 12, rExpr.val1 );
            else return( parserError((char *) "Immediate value out of range" ));
            
            if ( ! parseExpr( &rExpr )) return( false );
            
            if ( rExpr.typ == ET_ADR ) {
                
                setBitField( instr, 13, 2, 3 );
                setBitField( instr, 31, 4, rExpr.val1 );
            }
            else return( parserError((char *) "Expected an address" ));
            
            if      ( flags & TF_BYTE_INSTR ) setBitField( instr, 15, 2, 0 );
            else if ( flags & TF_HALF_INSTR ) setBitField( instr, 15, 2, 1 );
            else if ( flags & TF_WORD_INSTR ) setBitField( instr, 15, 2, 2 );
        }
    }
    else if ( rExpr.typ == ET_GREG ) {
    
        if ( tok -> isToken( TOK_EOS )) {
            
            setBitField( instr, 13, 2, 1 );
            setBitField( instr, 27, 4, targetRegId );
            setBitField( instr, 31, 4, rExpr.val1 );
        }
        else if ( tok -> isToken( TOK_COMMA )) {
            
            setBitField( instr, 13, 2, 1 );
            setBitField( instr, 27, 4, rExpr.val1 );
            
            tok -> nextToken( );
            if ( tok -> isTokenTyp( TOK_TYP_GREG )) {
                
                setBitField( instr, 13, 2, 1 );
                setBitField( instr, 27, 4, rExpr.val1 );
                setBitField( instr, 31, 4, tok -> tokVal( ));
                tok -> nextToken( );
            }
            else return( parserError((char *) "Expected a general reg" ));
        }
        else if ( tok -> isToken( TOK_LPAREN )) {
            
            setBitField( instr, 27, 4, rExpr.val1 );
            
            if (( parseExpr( &rExpr )) && ( rExpr.typ == ET_ADR )) {
                
                setBitField( instr, 13, 2, 2 );
                setBitField( instr, 31, 4, rExpr.val1 );
            }
            else return( parserError((char *) "Expected a logical address" ));
            
            if      ( flags & TF_BYTE_INSTR ) setBitField( instr, 15, 2, 0 );
            else if ( flags & TF_HALF_INSTR ) setBitField( instr, 15, 2, 1 );
            else if ( flags & TF_WORD_INSTR ) setBitField( instr, 15, 2, 2 );
        }
    }
    else return( parserError((char *) "Invalid operand" ));
    
    if (  getBitField( *instr, 13, 2 ) < 2 ) {
        
        if (( flags & TF_BYTE_INSTR ) || ( flags & TF_HALF_INSTR ))
            return( parserError((char *) "Invalid opCode data width specifier for mode option" ));
    }
    
    return( checkEOS( ));
}

//------------------------------------------------------------------------------------------------------------
// "parseInstrLSID" parses the LSID instruction.
//
//      <opCode> <targetReg> "," <sourceReg>
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrLSID( uint32_t *instr, uint32_t flags ) {
    
    if ( tok -> isTokenTyp( TOK_TYP_GREG )) {
        
        setBitField( instr, 9, 4, tok -> tokVal( ));
        tok -> nextToken( );
    }
    else return( parserError((char *) "Expected a general register" ));
    
    if ( ! acceptComma( )) return( false );
    
    if ( tok -> isTokenTyp( TOK_TYP_GREG )) {
        
        setBitField( instr, 31, 4, tok -> tokVal( ));
        tok -> nextToken( );
    }
    else return( parserError((char *) "Expected a general register" ));
    
    return( checkEOS( ));
}

//------------------------------------------------------------------------------------------------------------
// "parseInstrDEP" parses the deposit instruction. The instruction has three basic formats.
// When the "A" bit is set, the position will be obtained from the shift amount control register. Otherwise
// it is encoded in the instruction.
//
//      DEP [ ".“ <opt> ]       <targetReg> "," <sourceReg> "," <pos> "," <len>"
//      DEP [ "." "A" <opt> ]   <targetReg> "," <sourceReg> ", <len>"
//      DEP [ "." "I" <opt> ]   <targetReg> "," <val>, <pos> "," <len>
//      DEP [ "." "AI" <opt> ]  <targetReg> "," <val> "," <len>
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrDEP( uint32_t *instr, uint32_t flags ) {
    
    Expr rExpr;
    
    if ( tok -> isTokenTyp( TOK_TYP_GREG )) {
        
        setBitField( instr, 9, 4, tok -> tokVal( ));
        tok -> nextToken( );
    }
    else return( parserError((char *) "Expected a general register" ));
    
    if ( ! acceptComma( ))      return( false );
    if ( ! parseExpr( &rExpr )) return( false );
    
    if ( rExpr.typ == ET_GREG ) {
        
        setBitField( instr, 31, 4, tok -> tokVal( ));
       
        if ( ! acceptComma( ))      return( false );
        if ( ! parseExpr( &rExpr )) return( false );
        
        if ( rExpr.typ == ET_NUM ) {
            
            if ( isInRangeForBitFieldU( tok -> tokVal( ), 5 )) {
                
                if ( getBit( *instr, 11 ))  setBitField( instr, 21, 5, rExpr.val1 );
                else                        setBitField( instr, 27, 5, rExpr.val1 );
            }
            else return( parserError((char *) "Immediate value out of range" ));
        }
        else return( parserError((char *) "Expected a number" ));
        
        if ( ! getBit( *instr, 11 )) {
            
            if ( ! acceptComma( ))      return( false );
            if ( ! parseExpr( &rExpr )) return( false );
            
            if ( rExpr.typ == ET_NUM ) {
                
                if ( isInRangeForBitFieldU( rExpr.val1, 5 )) setBitField( instr, 21, 5, rExpr.val1 );
                else return( parserError((char *) "Immediate value out of range" ));
            }
            else return( parserError((char *) "Expected a number" ));
        }
    }
    else if ( rExpr.typ == ET_NUM ) {
        
        if ( getBit( *instr, 12 )) {
            
            if ( isInRangeForBitField( rExpr.val1, 4 )) setBitField( instr, 31, 4, rExpr.val1 );
            else return( parserError((char *) "Immediate value out of range" ));
            
            if ( ! acceptComma( )) return( false );
            
            if ( ! getBit( *instr, 11 )) {
               
                if ( isInRangeForBitFieldU( tok -> tokVal( ), 5 )) setBitField( instr, 27, 5, tok -> tokVal( ));
                else return( parserError((char *) "Pos value out of range" ));
                
                tok -> nextToken( );
                if ( ! acceptComma( )) return( false );
            }
            
            if ( ! parseExpr( &rExpr )) return( false );
            
            if ( rExpr.typ == ET_NUM ) {
                
                if ( isInRangeForBitFieldU( rExpr.val1, 5 )) setBitField( instr, 21, 5, rExpr.val1 );
                else return( parserError((char *) "Len value out of range" ));
            }
            else return( parserError((char *) "Expected a numeric value" ));
        }
        else return( parserError((char *) "Expected a numeric value for the I-opt" ));
    }
    else return( parserError((char *) "Expected a general register or a numeric value" ));
    
    return( checkEOS( ));
}

//------------------------------------------------------------------------------------------------------------
// The DS instruction parses the divide step instruction.
//
//      DS <targetReg> "," <sourceRegA> "," <sourceRegB>
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrDS( uint32_t *instr, uint32_t flags ) {
    
    if ( tok -> isTokenTyp( TOK_TYP_GREG )) {
        
        setBitField( instr, 9, 4, tok -> tokVal( ));
        tok -> nextToken( );
    }
    else return( parserError((char *) "Expected a general register" ));
    
    if ( ! acceptComma( )) return( false );
    
    if ( tok -> isTokenTyp( TOK_TYP_GREG )) {
        
        setBitField( instr, 27, 4, tok -> tokVal( ));
        tok -> nextToken( );
    }
    else return( parserError((char *) "Expected a general register" ));
    
    if ( ! acceptComma( )) return( false );
    
    if ( tok -> isTokenTyp( TOK_TYP_GREG )) {
        
        setBitField( instr, 31, 4, tok -> tokVal( ));
        tok -> nextToken( );
    }
    else return( parserError((char *) "Expected a general register" ));
    
    return( checkEOS( ));
}

//------------------------------------------------------------------------------------------------------------
// The DSR instruction parses the double shift instruction. There are two flavors. If the "A" bit is set, the
// shift amount is taken from the shift amount control register, else from the instruction "len" field.
//
//      DSR [ ".“ <opt> ] <targetReg> "," <sourceRegA> "," <sourceRegB> "," <len"
//      DSR [ ".“ "A"   ] <targetReg> "," <sourceRegA> "," <sourceRegB>
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrDSR( uint32_t *instr, uint32_t flags ) {
    
    Expr rExpr;
    
    if ( tok -> isTokenTyp( TOK_TYP_GREG )) {
        
        setBitField( instr, 9, 4, tok -> tokVal( ));
        tok -> nextToken( );
    }
    else return( parserError((char *) "Expected a general register" ));
    
    if ( ! acceptComma( )) return( false );
    
    if ( tok -> isTokenTyp( TOK_TYP_GREG )) {
        
        setBitField( instr, 27, 4, tok -> tokVal( ));
        tok -> nextToken( );
    }
    else return( parserError((char *) "Expected a general register" ));
    
    if ( ! acceptComma( )) return( false );
    
    if ( tok -> isTokenTyp( TOK_TYP_GREG )) {
        
        setBitField( instr, 31, 4, tok -> tokVal( ));
        tok -> nextToken( );
    }
    else return( parserError((char *) "Expected a general register" ));
    
    if ( ! getBit( *instr, 11 )) {
        
        if ( ! acceptComma( ))      return( false );
        if ( ! parseExpr( &rExpr )) return( false );
        
        if ( rExpr.typ == ET_NUM ) {
            
            if ( isInRangeForBitFieldU( rExpr.val1, 5 )) setBitField( instr, 21, 5, rExpr.val1 );
            else return( parserError((char *) "Immediate value out of range" ));
        }
        else return( parserError((char *) "Expected a number" ));
    }
    
    return( checkEOS( ));
}

//------------------------------------------------------------------------------------------------------------
// "parseInstrEXTR" parses the extract instruction. The instruction has two basic formats. When the "A" bit
// is set, the position will be obtained from the shift amount control register. Otherwise it is encoded in
// the instruction.
//
//      EXTR [ ".“ <opt> ]      <targetReg> "," <sourceReg> "," <pos> "," <len"
//      EXTR "." "A" [ <opt> ]  <targetReg> "," <sourceReg> ", <len"
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrEXTR( uint32_t *instr, uint32_t flags ) {
    
    Expr rExpr;
    
    if ( tok -> isTokenTyp( TOK_TYP_GREG )) {
        
        setBitField( instr, 9, 4, tok -> tokVal( ));
        tok -> nextToken( );
    }
    else return( parserError((char *) "Expected a general register" ));
    
    if ( ! acceptComma( )) return( false );
    
    if ( tok -> isTokenTyp( TOK_TYP_GREG )) {
        
        setBitField( instr, 31, 4, tok -> tokVal( ));
        tok -> nextToken( );
    }
    else return( parserError((char *) "Expected a general register" ));
    
    if ( ! acceptComma( ))      return( false );
    if ( ! parseExpr( &rExpr )) return( false );
    
    if ( rExpr.typ == ET_NUM ) {
        
        if ( isInRangeForBitFieldU( tok -> tokVal( ), 5 )) {
            
            if ( getBit( *instr, 11 ))  setBitField( instr, 21, 5, rExpr.val1 );
            else                        setBitField( instr, 27, 5, rExpr.val1 );
        }
        else return( parserError((char *) "Immediate value out of range" ));
    }
    else return( parserError((char *) "Expected a number" ));
    
    if ( ! getBit( *instr, 11 )) {
        
        if ( ! acceptComma( ))      return( false );
        if ( ! parseExpr( &rExpr )) return( false );
        
        if ( rExpr.typ == ET_NUM ) {
            
            if ( isInRangeForBitFieldU( rExpr.val1, 5 )) {
                
                setBitField( instr, 21, 5, rExpr.val1 );
            }
            else return( parserError((char *) "Immediate value out of range" ));
        }
        else return( parserError((char *) "Expected a number" ));
    }
    
    return( checkEOS( ));
}

//------------------------------------------------------------------------------------------------------------
// The SHLA instruction performs a shift left of "B" by "sa" and adds the "A" register to it.
//
//      SHLA [ "." <opt> ] <targetReg> "," <sourceRegA> "," <sourceRegB> "," <amt>
//      SHLA ".I" <targetReg> "," <sourceRegA> "," <val> "," <amt>
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrSHLA( uint32_t *instr, uint32_t flags ) {
    
    Expr rExpr;
    
    if ( tok -> isTokenTyp( TOK_TYP_GREG )) {
        
        setBitField( instr, 9, 4, tok -> tokVal( ));
        tok -> nextToken( );
    }
    else return( parserError((char *) "Expected a general register" ));
    
    if ( ! acceptComma( )) return( false );
    
    if ( tok -> isTokenTyp( TOK_TYP_GREG )) {
        
        setBitField( instr, 27, 4, tok -> tokVal( ));
        tok -> nextToken( );
    }
    else return( parserError((char *) "Expected a general register" ));
    
    if ( ! acceptComma( ))      return( false );
    if ( ! parseExpr( &rExpr )) return( false );
    
    if ( rExpr.typ == ET_GREG ) {
        
        if ( getBit( *instr, 10 )) return( parserError((char *) "Invalid option for register add" ));
        else setBitField( instr, 31, 4, tok -> tokVal( ));
    }
    else if ( rExpr.typ == ET_NUM ) {
        
        if ( getBit( *instr, 11 )) {
            
            if ( ! isInRangeForBitFieldU( rExpr.val1, 4 ))
                return( parserError((char *) "Immediate value out of range"));
        }
        else {
            
            if ( ! isInRangeForBitField( rExpr.val1, 4 ))
                return( parserError((char *) "Immediate value out of range"));
        }
        
        setBitField( instr, 31, 4, rExpr.val1 );
    }
    else return( parserError((char *) "Expected a general register or immediate value" ));
    
    if ( ! acceptComma( ))      return( false );
    if ( ! parseExpr( &rExpr )) return( false );
    
    if ( rExpr.typ == ET_NUM ) {
        
        if ( isInRangeForBitFieldU( rExpr.val1, 2 )) setBitField( instr, 21, 2, rExpr.val1 );
        else return( parserError((char *) "Immediate value out of range" ));
    }
    else return( parserError((char *) "Expected the shift amount" ));
    
    return( checkEOS( ));
}

//------------------------------------------------------------------------------------------------------------
// The CMR instruction tests register "B" for a condition and if true copies the "A" value to "R".
//
//      CMR "." <cond> <targetReg> "," <regA> "," <regB>
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrCMR( uint32_t *instr, uint32_t flags ) {
    
    if ( tok -> isTokenTyp( TOK_TYP_GREG )) {
        
        setBitField( instr, 9, 4, tok -> tokVal( ));
        tok -> nextToken( );
    }
    else return( parserError((char *) "Expected a general register" ));
    
    if ( ! acceptComma( )) return( false );
    
    if ( tok -> isTokenTyp( TOK_TYP_GREG )) {
        
        setBitField( instr, 27, 4, tok -> tokVal( ));
        tok -> nextToken( );
    }
    else return( parserError((char *) "Expected a general register" ));
    
    if ( ! acceptComma( )) return( false );
    
    if ( tok -> isTokenTyp( TOK_TYP_GREG )) {
        
        setBitField( instr, 31, 4, tok -> tokVal( ));
        tok -> nextToken( );
    }
    else return( parserError((char *) "Expected a general register" ));
    
    return( checkEOS( ));
}

//------------------------------------------------------------------------------------------------------------
// The "LDIL" instruction loads the immediate value encoded in the instruction left shifted into "R". The
// "ADDIL" instruction will add the value encoded in the instruction left shifted to "R". The result is
// in R1.
//
//      LDIL  <targetReg> "," <val>
//      ADDIL <sourceReg> "," <val>
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrLDILandADDIL( uint32_t *instr, uint32_t flags ) {
    
    Expr rExpr;
    
    if ( tok -> isTokenTyp( TOK_TYP_GREG )) {
        
        setBitField( instr, 9, 4, tok -> tokVal( ));
        tok -> nextToken( );
    }
    else return( parserError((char *) "Expected a general register" ));
    
    if ( ! acceptComma( )) return( false );
    
    if (( parseExpr( &rExpr )) && ( rExpr.typ == ET_NUM )) {

        if ( isInRangeForBitFieldU( rExpr.val1, 22 )) setImmValU( instr, 31, 22, rExpr.val1  );
        else return( parserError((char *) "Immediate value out of range" ));
    }
    else return( parserError((char *) "Expected a numeric expression" ));
    
    return( checkEOS( ));
}

//------------------------------------------------------------------------------------------------------------
// The "LDO" instruction computes the address of an operand, and stores the result in "R".
//
//      LDO <targetReg> "," [ <ofs> "," ] "(" <baseReg> ")"
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrLDO( uint32_t *instr, uint32_t flags ) {
    
    Expr rExpr;
    
    if ( tok -> isTokenTyp( TOK_TYP_GREG )) {
        
        setBitField( instr, 9, 4, tok -> tokVal( ));
        tok -> nextToken( );
    }
    else return( parserError((char *) "Expected a general register" ));
    
    if ( ! acceptComma( ))      return( false );
    if ( ! parseExpr( &rExpr )) return( false );
    
    if ( rExpr.typ == ET_NUM ) {
        
        if ( isInRangeForBitField( rExpr.val1, 18 )) setImmVal( instr, 27, 18, rExpr.val1 );
        else return( parserError((char *) "Immediate value out of range" ));
        
        if ( ! parseExpr( &rExpr )) return( false );
        
        if ( rExpr.typ == ET_ADR ) setBitField( instr, 31, 4, rExpr.val1 );
        else return( parserError((char *) "Expected the base register" ));
    }
    else if ( rExpr.typ == ET_ADR ) {
        
        setImmVal( instr, 27, 18, 0 );
        setBitField( instr, 31, 4, rExpr.val1 );
    }
    else return( parserError((char *) "Expected an offset or  left paren" ));
   
    return( checkEOS( ));
}

//------------------------------------------------------------------------------------------------------------
// The "B" and "GATE" instruction represent an instruction offset relative branch. Optionally, there is an
// optional return register. When omitted, R0 is used in the instruction generation.
//
//      B       <offset> [ "," <returnReg> ]
//      GATE    <offset> [ "," <returnReg> ]
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrBandGATE( uint32_t *instr, uint32_t flags ) {
    
    Expr rExpr;
    
    if (( parseExpr( &rExpr )) && ( rExpr.typ == ET_NUM )) {
        
        if ( isInRangeForBitField( rExpr.val1, 22 )) setImmVal( instr, 31, 22, rExpr.val1 );
        else return( parserError((char *) "Offset value out of range" ));
    }
    else return( parserError(( char *) "Expected an offset value" ));
    
    if ( tok -> isToken( TOK_COMMA )) {
        
        tok -> nextToken( );
        if ( tok -> isTokenTyp( TOK_TYP_GREG )) {
            
            setBitField( instr, 9, 4, tok -> tokVal( ));
            tok -> nextToken( );
        }
        else return( parserError((char *) "Expected a general reg" ));
    }
    
    return( checkEOS( ));
}

//------------------------------------------------------------------------------------------------------------
// The "BR" instruction is an IA-relative branch with the offset to be added in a general register. There is
// also an optional return register. When omitted, R0 is used in the instruction generation.
//
//      BR "(" <branchReg> ")" [ "," <returnReg> ]
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrBR( uint32_t *instr, uint32_t flags ) {
    
    if ( ! acceptLparen( )) return( false );
    
    if ( tok -> isTokenTyp( TOK_TYP_GREG )) {
        
        setBitField( instr, 31, 4, tok -> tokVal( ) );
        tok -> nextToken( );
    }
    else return( parserError((char *) "Expected a general register" ));
    
    if ( ! acceptRparen( )) return( false );
    
    if ( tok -> isToken( TOK_COMMA )) {
        
        tok -> nextToken( );
        if ( tok -> isTokenTyp( TOK_TYP_GREG )) {
            
            setBitField( instr, 9, 4, tok -> tokVal( ));
            tok -> nextToken( );
        }
        else return( parserError((char *) "Expected a general register" ));
    }
    
    return( checkEOS( ));
}

//------------------------------------------------------------------------------------------------------------
// The "BV" is an absolute branch address instruction in the same segment. Optionally, there is an optional
// return register. When omitted, R0 is used in the instruction generation.
//
//      BV "(" <targetAdrReg> ")" [ "," <returnReg> ]
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrBV( uint32_t *instr, uint32_t flags ) {
    
    if ( ! acceptLparen( )) return( false );
    
    if ( tok -> isTokenTyp( TOK_TYP_GREG )) {
        
        setBitField( instr, 31, 4, tok -> tokVal( ));
        tok -> nextToken( );
    }
    else return( parserError((char *) "Expected a general register" ));
    
    if ( ! acceptRparen( )) return( false );
    
    if ( tok -> isToken( TOK_COMMA )) {
        
        tok -> nextToken( );
        if ( tok -> isTokenTyp( TOK_TYP_GREG )) {
            
            setBitField( instr, 31, 4, tok -> tokVal( ));
            tok -> nextToken( );
        }
        else return( parserError((char *) "Expected a general register" ));
    }
    
    return( checkEOS( ));
}

//------------------------------------------------------------------------------------------------------------
// The "BE" instruction is an external branch to a segment and a segment relative offset. When the offset
// part is omitted, a zero is used. There is also an optional return register. When omitted, R0 is used in
// the instruction generation.
//
//      BE [ <ofs> ] "(" <segReg> "," <ofsReg> ")" [ "," <retSeg> ]
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrBE( uint32_t *instr, uint32_t flags ) {
    
    Expr rExpr;
    
    if ( ! parseExpr( &rExpr )) return( false );
    
    if ( rExpr.typ == ET_NUM ) {
        
        if ( isInRangeForBitField( rExpr.val1, 22 )) setImmVal( instr, 23, 14, rExpr.val1 );
        else return( parserError((char *) "Immediate value out of range" ));
        
        if ( !parseExpr( &rExpr )) return( false );
    }
    
    if ( rExpr.typ == ET_EXT_ADR ) {
        
        setBitField( instr, 27, 4, rExpr.val1 );
        setBitField( instr, 31, 4, rExpr.val2 );
    }
    else return( parserError((char *) "Expected a virtual address" ));
    
    if ( tok -> isToken( TOK_COMMA )) {
        
        tok -> nextToken( );
        if ( tok -> isTokenTyp( TOK_TYP_GREG )) {
            
            setBitField( instr, 9, 4, tok -> tokVal( ));
            tok -> nextToken( );
        }
        else return( parserError((char *) "Expected a general register" ));
    }
    
    return( checkEOS( ));
}

//------------------------------------------------------------------------------------------------------------
// The "BVE" instruction forms a logical address by adding general register "a" to base register "b". There
// is also an optional return register. When omitted, R0 is used in the instruction generation.
//
//      BVE [ <offsetReg> ] "(" <baseReg> ")" [ "," <returnReg> ]
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrBVE( uint32_t *instr, uint32_t flags ) {
    
    Expr rExpr;
    
    if ( tok -> isTokenTyp( TOK_TYP_GREG )) {
        
        setBitField( instr, 27, 4, tok -> tokVal( ) );
        tok -> nextToken( );
    }
    
    if ( ! parseExpr( &rExpr )) return( false );
    
    if ( rExpr.typ == ET_ADR ) {
        
        setBitField( instr, 31, 4, rExpr.val1 );
    }
    else return( parserError((char *) "Expected a logical address" ));
    
    if ( tok -> isToken( TOK_COMMA )) {
        
        tok -> nextToken( );
        if ( tok -> isTokenTyp( TOK_TYP_GREG )) {
            
            setBitField( instr, 9, 4, tok -> tokVal( ));
            tok -> nextToken( );
        }
        else return( parserError((char *) "Expected a general register" ));
    }
    
    return( checkEOS( ));
}

//------------------------------------------------------------------------------------------------------------
// The "CBR" and "CBRU" compare register "a" and "b" based on the condition and branch if the comparison
// result is true. The condition code is encoded in the instruction option string parsed before.
//
//      CBR  .<cond> <a>, <b>, <ofs>
//      CBRU .<cond> <a>, <b>, <ofs>
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrCBRandCBRU( uint32_t *instr, uint32_t flags ) {
    
    Expr rExpr;
    
    if ( tok -> isTokenTyp( TOK_TYP_GREG )) {
        
        setBitField( instr, 27, 4, tok -> tokVal( ) );
        tok -> nextToken( );
    }
    
    if ( ! acceptComma( )) return( false );
    
    if ( tok -> isTokenTyp( TOK_TYP_GREG )) {
        
        setBitField( instr, 31, 4, tok -> tokVal( ));
        tok -> nextToken( );
    }
    else return( parserError((char *) "Expected a general register" ));
    
    if ( ! acceptComma( )) return( false );
    
    if (( parseExpr( &rExpr )) && ( rExpr.typ == ET_NUM )) {
    
        if ( isInRangeForBitField( rExpr.val1, 16 )) {
            
            setImmVal( instr, 23, 16, rExpr.val1 );
            tok -> nextToken( );
        }
        else return( parserError((char *) "Immediate value out of range" ));
    }
    else return( parserError((char *) "Expected an offset value" ));
    
    return( checkEOS( ));
}

//------------------------------------------------------------------------------------------------------------
// "parseInstrLoad" will parse the load instructions family. The workhorse is the "parseLoadStoreOperand"
// routine, which parses the operand. General form:
//
//      <opCode>.<opt> <targetReg>, <sourceOperand>
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrLoadAndStore( uint32_t *instr, uint32_t flags ) {
    
    if ( tok -> isTokenTyp( TOK_TYP_GREG )) {
        
        setBitField( instr, 9, 4, tok -> tokVal( ));
        tok -> nextToken( );
    }
    else return( parserError((char *) "Expected a general register" ));
    
    if ( ! acceptComma( )) return( false );
    
    return( parseLoadStoreOperand( instr, flags ));
}

//------------------------------------------------------------------------------------------------------------
// The "MR" instruction is a move register instruction. We parse valid combination and assemble the
// instruction. Note that the "MR" instruction is primarily used for moving segment and control registers
// to and from a general register. However, the syntax can also be used to move between general registers.
// We will in this case emit an "OR" instruction.
//
//      MR <targetReg> "," <sourceReg>
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrMR( uint32_t *instr, uint32_t flags ) {
    
    if ( tok -> isTokenTyp( TOK_TYP_GREG )) {
        
        uint8_t tRegId = tok -> tokVal( );
        
        tok -> nextToken( );
        if ( ! acceptComma( )) return( false );
        
        if ( tok -> isTokenTyp( TOK_TYP_GREG )) {
            
            *instr = 0;
            setBitField( instr, 5, 6, OP_OR );
            setBitField( instr, 9, 4, tRegId );
            setBitField( instr, 13, 2, 1 );
            setBitField( instr, 27, 4, 0 );
            setBitField( instr, 31, 4, tok -> tokVal( ));
            tok -> nextToken( );
        }
        else if ( tok -> isTokenTyp( TOK_TYP_SREG )) {
          
            setBitField( instr, 31, 3, tok -> tokVal( ));
            setBitField( instr, 9, 4, tRegId );
            tok -> nextToken( );
        }
        else if ( tok -> isTokenTyp( TOK_TYP_CREG )) {
           
            setBit( instr, 11 );
            setBitField( instr, 31, 5, tok -> tokVal( ));
            setBitField( instr, 9, 4, tRegId );
            tok -> nextToken( );
        }
    }
    else if ( tok -> isTokenTyp( TOK_TYP_SREG )) {
        
        uint8_t tRegId = tok -> tokVal( );
        
        tok -> nextToken( );
        if ( ! acceptComma( )) return( false );
    
        if ( tok -> isTokenTyp( TOK_TYP_GREG )) {
            
            setBit( instr, 10 );
            setBitField( instr, 31, 3, tRegId );
            setBitField( instr, 9, 4, tok -> tokVal( ));
            tok -> nextToken( );
        }
        else return( parserError((char *) "Only SREG <- GREG is allowed" ));
    }
    else if ( tok -> isTokenTyp( TOK_TYP_CREG )) {
        
        uint8_t tRegId = tok -> tokVal( );
        
        tok -> nextToken( );
        if ( ! acceptComma( )) return( false );
        
        if ( tok -> isTokenTyp( TOK_TYP_GREG )) {
            
            setBit( instr, 10 );
            setBit( instr, 11 );
            setBitField( instr, 31, 5, tRegId );
            setBitField( instr, 9, 4, tok -> tokVal( ) );
            tok -> nextToken( );
        }
        else return( parserError((char *) "Only CREG <- GREG is allowed" ));
    }
    
    return( checkEOS( ));
}

//------------------------------------------------------------------------------------------------------------
// The "MST" instruction sets and clears bits in the program state word. There are two basic formats. The
// first format will use a general register for the data bits, the second format will use the value encoded
// in the instruction.
//
//      MST b
//      MST.S <val>
//      MST.C <val>
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrMST( uint32_t *instr, uint32_t flags ) {
    
    Expr rExpr;
    
    if ( tok -> isTokenTyp( TOK_TYP_GREG )) {
        
        setBitField( instr, 9, 4, tok -> tokVal( ) );
        tok -> nextToken( );
    }
    else return( parserError((char *) "Expected a general register" ));
    
    if ( ! acceptComma( ))      return( false );
    if ( ! parseExpr( &rExpr )) return( false );
    
    if ( rExpr.typ == ET_GREG ) {
        
        if ( getBitField( *instr, 11, 2 ) == 0 ) {
            
            setBitField( instr, 31, 4, rExpr.val1 );
            tok -> nextToken( );
        }
        else return( parserError((char *) "Invalid option for the MST instruction" ));
    }
    else if ( rExpr.typ == ET_NUM ) {
        
        if (( getBitField( *instr, 11, 2 ) == 1 ) || ( getBitField( *instr, 11, 2 ) == 2 )) {
            
            if ( isInRangeForBitFieldU( rExpr.val1, 6 )) setBitField( instr, 31, 6, rExpr.val1 );
            else return( parserError((char *) "Status bit field value out of range" ));
        }
        else  return( parserError((char *) "Invalid option for the MST instruction" ));
    }
    else return( parserError((char *) "Expected the status bit argument" ));
  
    return( checkEOS( ));
}

//------------------------------------------------------------------------------------------------------------
// The "LDPA" instruction loads a physical address for the logical address. When the segment is explicitly
// used, it must be in the range of SR1 to SR3.
//
//      LDPA <targetReg> ","  <indexReg> "(" [ <segmentReg>, ] <offsetReg > ")"
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrLDPA( uint32_t *instr, uint32_t flags ) {
    
    Expr rExpr;
    
    if ( tok -> isTokenTyp( TOK_TYP_GREG )) {
        
        setBitField( instr, 9, 4, tok -> tokVal( ) );
        tok -> nextToken( );
    }
    
    if ( ! acceptComma( )) return( false );
    
    if ( tok -> isTokenTyp( TOK_TYP_GREG )) {
        
        setBitField( instr, 27, 4, rExpr.val1 );
        tok -> nextToken( );
    }
    
    return( parseLogicalAdr( instr, flags ));
}

//------------------------------------------------------------------------------------------------------------
// The "PRB" instruction will test a logical address for the desired read or write access. The "I" bit will
// when cleared use the "A" reg as input, else bit 27 of the instruction.
//
//      PRB [ "." <opt> ] <targetReg> "," "(" [ <segmentReg>, ] <offsetReg > ")" [ "," <argReg> ]
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrPRB( uint32_t *instr, uint32_t flags ) {
    
    Expr rExpr;
    
    if ( tok -> isTokenTyp( TOK_TYP_GREG )) {
        
        setBitField( instr, 9, 4, tok -> tokVal( ) );
        tok -> nextToken( );
    }
    
    if ( ! acceptComma( ))                  return( false );
    if ( ! parseLogicalAdr( instr, flags )) return( false );
    if ( ! acceptComma( ))                  return( false );
    if ( ! parseExpr( &rExpr ))             return( false );
    
    if ( getBit( *instr, 11 )) {
        
        if ( rExpr.typ == ET_NUM ) {
            
            if ( isInRangeForBitFieldU( rExpr.val1, 1 )) setBit( instr, 27, rExpr.val1 );
        }
        else return( parserError((char *) "Expected a 0 or 1" ));
    }
    else if ( rExpr.typ == ET_GREG ) {
        
        setBitField( instr, 27, 4, rExpr.val1 );
    }
    else return( parserError((char *) "Expected a register or numeric value" ));
    
    return( checkEOS( ));
}

//------------------------------------------------------------------------------------------------------------
// The "ITLB" instruction will insert a new entry in the instruction or data TLB. We use the segment and
// offset register pair for the virtual address to enter.
//
//      ITLB [.<opt>] <tlbInfoReg> "," "(" <segmentReg> "," <offsetReg> ")"
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrITLB( uint32_t *instr, uint32_t flags ) {
    
    if ( tok -> isTokenTyp( TOK_TYP_GREG )) {
        
        setBitField( instr, 9, 4, tok -> tokVal( ) );
        tok -> nextToken( );
    }
    
    if ( ! acceptComma( ))  return( false );
    if ( ! acceptLparen( )) return( false );
    
    if ( tok -> isTokenTyp( TOK_TYP_SREG )) setBitField( instr, 27, 4, tok -> tokVal( ));
    else return( parserError((char *) "Expected a segement register" ));
    
    if ( ! acceptComma( )) return( false );
    
    if ( tok -> isTokenTyp( TOK_TYP_GREG )) setBitField( instr, 31, 4, tok -> tokVal( ));
    else return( parserError((char *) "Expected a general register" ));
    
    if ( ! acceptRparen( )) return( false );
   
    return( checkEOS( ));
}

//------------------------------------------------------------------------------------------------------------
// The "PTLB" instruction removes an entry from the instruction or data TLB. We use a logical address to
// refer to the TLB entry.
//
//      PTLB [ "." <opt> ] [ <indexReg" ] "(" [ <segmentReg>, ] <offsetReg > ")"
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrPTLB( uint32_t *instr, uint32_t flags ) {

    if ( tok -> isTokenTyp( TOK_TYP_GREG )) {
        
        setBitField( instr, 27, 4, tok -> tokVal( ) );
        tok -> nextToken( );
    }
    
    if ( tok -> isToken( TOK_LPAREN )) {
        
        if ( ! parseLogicalAdr( instr, flags )) return( false );
    }
    else return( parserError((char *) "Expected an index register or address" ));
        
    return( checkEOS( ));
}

//------------------------------------------------------------------------------------------------------------
// The "PCA" instruction flushes and / or remove an entry from a data or instruction cache.
//
//      PCA [ "." <opt> ] [ <indexReg" ] "(" [ <segmentReg>, ] <offsetReg > ")"
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrPCA( uint32_t *instr, uint32_t flags ) {
    
    if ( tok -> isTokenTyp( TOK_TYP_GREG )) {
        
        setBitField( instr, 27, 4, tok -> tokVal( ) );
        tok -> nextToken( );
    }
    
    if ( tok -> isToken( TOK_LPAREN )) {
        
        if ( ! parseLogicalAdr( instr, flags )) return( false );
    }
    else return( parserError((char *) "Expected an index register" ));
        
    return( checkEOS( ));
}

//------------------------------------------------------------------------------------------------------------
// The "DIAG" instruction is the instruction for invoking special hardware or diagnostic functions.
//
//      DIAG <resultReg> "," <parmRegA> "," <parmRegB> "," <info>
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrDIAG( uint32_t *instr, uint32_t flags ) {
    
    Expr rExpr;
    
    if ( tok -> isTokenTyp( TOK_TYP_GREG )) {
        
        setBitField( instr, 9, 4, tok -> tokVal( ) );
        tok -> nextToken( );
    }
    
    if ( ! acceptComma( )) return( false );
    
    if ( tok -> isTokenTyp( TOK_TYP_GREG )) {
        
        setBitField( instr, 27, 4, tok -> tokVal( ) );
        tok -> nextToken( );
    }
    else return( parserError((char *) "Expected a general register" ));
    
    if ( ! acceptComma( )) return( false );
    
    if ( tok -> isTokenTyp( TOK_TYP_GREG )) {
        
        setBitField( instr, 31, 4, tok -> tokVal( ) );
        tok -> nextToken( );
    }
    else return( parserError((char *) "Expected a general register" ));
    
    if ( ! acceptComma( )) return( false );
    
    if (( parseExpr( &rExpr )) && ( rExpr.typ == ET_NUM )) {
            
        if ( isInRangeForBitFieldU( rExpr.val1, 4 )) {
                
            setBitField( instr, 13, 4, rExpr.val1 );
            tok -> nextToken( );
        }
        else return( parserError((char *) "Immediate value out of range" ));
    }
    else return( parserError((char *) "Expected a number" ));
   
    return( checkEOS( ));
}

//------------------------------------------------------------------------------------------------------------
// The "RFI" instruction is the return from interrupt method. So far it is only the instruction with no
// further options and arguments.
//
//      RFI
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrRFI( uint32_t *instr, uint32_t flags ) {
    
    return( checkEOS( ));
}

//------------------------------------------------------------------------------------------------------------
// The "BRK" instruction will raise a trap passing along two info fields.
//
//      BRK <info1> "," <info2>
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrBRK( uint32_t *instr, uint32_t flags ) {
    
    Expr rExpr;
    
    if (( parseExpr( &rExpr )) && ( rExpr.typ == ET_NUM )) {
        
        if ( isInRangeForBitFieldU( rExpr.val1, 4 )) setImmValU( instr, 9, 4, rExpr.val1 );
        else return( parserError((char *) "Immediate value out of range" ));
    }
    else return( parserError((char *) "Expected the info1 parm" ));
    
    if ( ! acceptComma( )) return( false );
    
    if (( parseExpr( &rExpr )) && ( rExpr.typ == ET_NUM )) {
    
        if ( isInRangeForBitFieldU( rExpr.val1, 16 )) setImmValU( instr, 31, 16, rExpr.val1 );
        else return( parserError((char *) "Immediate value out of range" ));
    }
    else return( parserError((char *) "Expected the info2 parm" ));
    
    return( checkEOS( ));
}

//------------------------------------------------------------------------------------------------------------
// The "NOP" synthetic instruction emits the "BRK 0,0" instruction. Easy case.
//
//      NOP
//
//------------------------------------------------------------------------------------------------------------
bool parseSynthInstrNop( uint32_t *instr, uint32_t flags ) {
    
    *instr = 0x0;
    
    tok -> nextToken( );
    return( checkEOS( ));
}

// ??? add shift and rotate synthetic ops....

//------------------------------------------------------------------------------------------------------------
// "parseLine" will take the input string and parse the line for an instruction. In the simplified case, there
// is only the opCode mnemonic and the argument list. No labels, no comments. For each instruction, there is
// is a routine that parses the instruction specific input.
//
// An instruction starts with the opCode and the optional option qualifiers. For each opCode, the token table
// has an instruction template and some further information about the instruction, which is used to do further
// syntax checking.. For example, mapping the "LDx" instruction to "LDW" is already encoded in the template
// and set in the flags field.
//
// The next step for all instructions is to check for options. Finally, a dedicated parsing routine will
// handle the remainder of the assembly line. As the parsing process comes along the instruction template
// from the token name table will be augmented with further data. If all is successful, we will have the
// final instruction bit pattern.
//
//------------------------------------------------------------------------------------------------------------
bool parseLine( char *inputStr, uint32_t *instr ) {
    
    uint32_t    flags   = 0;
    TokId       opCode  = TOK_NIL;
    
    tok -> setupTokenizer( inputStr, (DrvToken *) asmTokTab );
    tok -> nextToken( );
    
    if ( tok -> isTokenTyp( TOK_TYP_OP_CODE )) {
        
        flags   = 0;
        opCode  = tok -> tokId( );
        *instr  = tok -> tokVal( );
       
        tok -> nextToken( );
        while ( tok -> isToken( TOK_PERIOD )) {
            
            tok -> nextToken( );
            if ( ! parseInstrOptions( instr, &flags )) return( false );
        }
        
        switch( opCode ) {
                
            case OP_CODE_ADD:   case OP_CODE_ADDW:
            case OP_CODE_ADC:   case OP_CODE_ADCW:
            case OP_CODE_SUB:   case OP_CODE_SUBW:
            case OP_CODE_SBC:   case OP_CODE_SBCW:
            case OP_CODE_AND:   case OP_CODE_ANDW:
            case OP_CODE_OR:    case OP_CODE_ORW:
            case OP_CODE_XOR:   case OP_CODE_XORW:
            case OP_CODE_CMP:   case OP_CODE_CMPW:
            case OP_CODE_CMPU:  case OP_CODE_CMPUW: {
                
                return( parseModeTypeInstr( instr, flags | TF_WORD_INSTR ));
            }
                
            case OP_CODE_ADDB:  case OP_CODE_ADCB:  case OP_CODE_SUBB:  case OP_CODE_SBCB:
            case OP_CODE_ANDB:  case OP_CODE_ORB:   case OP_CODE_XORB:  case OP_CODE_CMPB:
            case OP_CODE_CMPUB: {
                
                return( parseModeTypeInstr( instr, flags | TF_BYTE_INSTR ));
            }
                
            case OP_CODE_ADDH:  case OP_CODE_ADCH:  case OP_CODE_SUBH:  case OP_CODE_SBCH:
            case OP_CODE_ANDH:  case OP_CODE_ORH:   case OP_CODE_XORH:  case OP_CODE_CMPH:
            case OP_CODE_CMPUH: {
                
                return( parseModeTypeInstr( instr, flags | TF_HALF_INSTR ));
            }
                
            case OP_CODE_LD:    case OP_CODE_LDW:   case OP_CODE_LDA:   case OP_CODE_LDR:
            case OP_CODE_ST:    case OP_CODE_STW:   case OP_CODE_STA:   case OP_CODE_STC: {
                
                return( parseInstrLoadAndStore( instr, flags | TF_WORD_INSTR ));
            }
            
            case OP_CODE_STB:   case OP_CODE_LDB: {
                
                return( parseInstrLoadAndStore( instr, flags | TF_BYTE_INSTR ));
            }
            
            case OP_CODE_LDH:   case OP_CODE_STH: {
                
                return( parseInstrLoadAndStore( instr, flags | TF_HALF_INSTR ));
            }
            
            case OP_CODE_LSID:      return( parseInstrLSID( instr, flags ));
            case OP_CODE_EXTR:      return( parseInstrEXTR( instr, flags ));
            case OP_CODE_DEP:       return( parseInstrDEP( instr, flags ));
                
            case OP_CODE_DS:        return( parseInstrDS( instr, flags ));
                
            case OP_CODE_DSR:       return( parseInstrDSR( instr, flags ));
            case OP_CODE_SHLA:      return( parseInstrSHLA( instr, flags ));
            case OP_CODE_CMR:       return( parseInstrCMR( instr, flags ));
                
                
            case OP_CODE_LDIL:
            case OP_CODE_ADDIL:     return( parseInstrLDILandADDIL( instr, flags ));
                
            case OP_CODE_LDO:       return( parseInstrLDO( instr, flags ));
                
            case OP_CODE_B:
            case OP_CODE_GATE:      return( parseInstrBandGATE( instr, flags ));
                
            case OP_CODE_BR:        return( parseInstrBR( instr, flags ));
            case OP_CODE_BV:        return( parseInstrBV( instr, flags ));
            case OP_CODE_BE:        return( parseInstrBE( instr, flags ));
            case OP_CODE_BVE:       return( parseInstrBVE( instr, flags ));
                
            case OP_CODE_CBR:
            case OP_CODE_CBRU:      return( parseInstrCBRandCBRU( instr, flags ));
                
            case OP_CODE_MR:        return( parseInstrMR( instr, flags ));
            case OP_CODE_MST:       return( parseInstrMST( instr, flags ));
            case OP_CODE_LDPA:      return( parseInstrLDPA( instr, flags ));
            case OP_CODE_PRB:       return( parseInstrPRB( instr, flags ));
            case OP_CODE_ITLB:      return( parseInstrITLB( instr, flags ));
            case OP_CODE_PTLB:      return( parseInstrPTLB( instr, flags ));
            case OP_CODE_PCA:       return( parseInstrPCA( instr, flags ));
            case OP_CODE_DIAG:      return( parseInstrDIAG( instr, flags ));
            case OP_CODE_RFI:       return( parseInstrRFI( instr, flags ));
            case OP_CODE_BRK:       return( parseInstrBRK( instr, flags ));
            
            default: return( parserError((char *) "Invalid opcode" ));
        }
    }
    else if ( tok -> isTokenTyp( TOK_TYP_OP_CODE_S )) {
        
        flags   = 0;
        opCode  = tok -> tokId( );
        *instr  = 0;
       
        switch ( opCode ) {
                
            case OP_CODE_S_NOP:    return( parseSynthInstrNop( instr, flags ));
                
            default: return( parserError((char *) "Invalid synthetic opcode" ));
        }
    }
    else return( parserError((char *) "Expected an opcode" ));
}

} // namespace

//------------------------------------------------------------------------------------------------------------
// A simple one line assembler. This object is the counterpart to the disassembler. We will parse a one line
// input string for a valid instruction, using the syntax of the real assembler. There will be no labels and
// comments, only the opcode and the operands.
//
//------------------------------------------------------------------------------------------------------------
DrvOneLineAsm::DrvOneLineAsm( VCPU32Globals *glb ) {
    
    this -> glb = glb;
}

bool DrvOneLineAsm::parseAsmLine( char *inputStr, uint32_t *instr ) {
    
    return ( parseLine( inputStr, instr ));
}
