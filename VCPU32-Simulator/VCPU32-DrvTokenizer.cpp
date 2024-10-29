//------------------------------------------------------------------------------------------------------------
//
// VCPU32 - A 32-bit CPU - Simulator Tokenizer
//
//------------------------------------------------------------------------------------------------------------
// Welcome to the test driver commands. This a rather simple command loop resting on the "sscanf" C library
// routine to do the parsing.
//
//
//------------------------------------------------------------------------------------------------------------
//
// VCPU32 - A 32-bit CPU - Simulator Commands
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
#include "VCPU32-Version.h"
#include "VCPU32-Types.h"
#include "VCPU32-Driver.h"

//------------------------------------------------------------------------------------------------------------
// Local namespace. These routines are not visible outside this source file.
//
//------------------------------------------------------------------------------------------------------------
namespace {

//------------------------------------------------------------------------------------------------------------
//
//
//------------------------------------------------------------------------------------------------------------
const int   TOK_INPUT_LINE_SIZE = 256;
const int   TOK_NAME_SIZE       = 32;
const char  EOS_CHAR            = 0;

#if 0
//------------------------------------------------------------------------------------------------------------
// The global token table. All reserved words are allocated in this table. Each entry has the token name,
// the token id, the token tye id, i.e. its type, and a value associatd with the token. The value allows
// for a constant token. The parser can directly use the value in expressions.
//
// ??? do some sorting, better readbility....
//------------------------------------------------------------------------------------------------------------
DrvToken const tokTab[ ] = {
    
    //--------------------------------------------------------------------------------------------------------
    //
    //
    //--------------------------------------------------------------------------------------------------------
    { .name = "ENV",        .typ = TOK_TYP_CMD,     .tid = CMD_ENV                              },
    { "EXIT",               TOK_TYP_CMD,            CMD_EXIT,                   0               },
    { "E",                  TOK_TYP_CMD,            CMD_EXIT,                   0               },
    { "HELP",               TOK_TYP_CMD,            CMD_HELP,                   0               },
    { "?",                  TOK_TYP_CMD,            CMD_HELP,                   0               },
    { "WHELP",              TOK_TYP_CMD,            CMD_WHELP,                  0               },
    { "RESET",              TOK_TYP_CMD,            CMD_RESET,                  0               },
    { "RUN",                TOK_TYP_CMD,            CMD_RUN,                    0               },
    { "STEP",               TOK_TYP_CMD,            CMD_STEP,                   0               },
    { "S",                  TOK_TYP_CMD,            CMD_STEP,                   0               },
    { "DIS",                TOK_TYP_CMD,            CMD_DIS_ASM,                0               },
    { "ASM",                TOK_TYP_CMD,            CMD_ASM,                    0               },
    
    { "EXEC-F",             TOK_TYP_CMD,            CMD_XF,                     0               },
    { "XF",                 TOK_TYP_CMD,            CMD_XF,                     0               },
    
    { "D-REG",              TOK_TYP_CMD,            CMD_DR,                     0               },
    { "DR",                 TOK_TYP_CMD,            CMD_DR,                     0               },
    
    { "M-REG",              TOK_TYP_CMD,            CMD_MR,                     0               },
    { "MR",                 TOK_TYP_CMD,            CMD_MR,                     0               },
    
    { "HASH-VA",            TOK_TYP_CMD,            CMD_HASH_VA,                0               },
    { "HVA",                TOK_TYP_CMD,            CMD_HASH_VA,                0               },
    
    { "I-TLB",              TOK_TYP_CMD,            CMD_I_TLB,                  0               },
    { "ITLB",               TOK_TYP_CMD,            CMD_I_TLB,                  0               },
    
    { "D-TLB",              TOK_TYP_CMD,            CMD_D_TLB,                  0               },
    { "DTLB",               TOK_TYP_CMD,            CMD_D_TLB,                  0               },
    
    { "P-TLB",              TOK_TYP_CMD,            CMD_P_TLB,                  0               },
    { "PTLB",               TOK_TYP_CMD,            CMD_P_TLB,                  0               },
    
    { "D-CACHE",            TOK_TYP_CMD,            CMD_D_CACHE,                0               },
    { "DCA",                TOK_TYP_CMD,            CMD_D_CACHE,                0               },
    
    { "P-CACHE",            TOK_TYP_CMD,            CMD_P_CACHE,                0               },
    { "PCA",                TOK_TYP_CMD,            CMD_P_CACHE,                0               },

    { "D-ABS",              TOK_TYP_CMD,            CMD_DA,                     0               },
    { "DA",                 TOK_TYP_CMD,            CMD_DA,                     0               },
    
    { "M-ABS",              TOK_TYP_CMD,            CMD_MA,                     0               },
    { "MA",                 TOK_TYP_CMD,            CMD_MA,                     0               },
    
    { "D-ABS-ASM",          TOK_TYP_CMD,            CMD_DAA,                    0               },
    { "DAA",                TOK_TYP_CMD,            CMD_DAA,                    0               },
    
    { "M-ABS-ASM",          TOK_TYP_CMD,            CMD_MAA,                    0               },
    { "MAA",                TOK_TYP_CMD,            CMD_MAA,                    0               },
    
    { "LOAD-MEM",           TOK_TYP_CMD,            CMD_LMF,                    0               },
    { "SAVE-MEM",           TOK_TYP_CMD,            CMD_SMF,                    0               },
    
    //--------------------------------------------------------------------------------------------------------
    //
    //
    //--------------------------------------------------------------------------------------------------------
    { .name = "TRUE",               TOK_TYP_NIL,            TOK_TRUE,                   0               },
    { .name = "FALSE",              TOK_TYP_NIL,            TOK_FALSE,                  0               },
    { .name = "ALL",                TOK_TYP_NIL,            TOK_ALL,                    0               },
    { .name = "CPU",                TOK_TYP_NIL,            TOK_CPU,                    0               },
    { .name = "MEM",                TOK_TYP_NIL,            TOK_MEM,                    0               },
    { .name = "C",                  TOK_TYP_NIL,            TOK_C,                      0               },
    { .name = "D",                  TOK_TYP_NIL,            TOK_D,                      0               },
    { .name = "F",                  TOK_TYP_NIL,            TOK_F,                      0               },
    { .name = "I",                  TOK_TYP_NIL,            TOK_I,                      0               },
    { .name = "T",                  TOK_TYP_NIL,            TOK_T,                      0               },
    { .name = "U",                  TOK_TYP_NIL,            TOK_U,                      0               },
    
    { .name = "DEC",                FMT_SET,            TOK_DEC,                    0               },
    { .name = "HEX",                FMT_SET,            TOK_HEX,                    0               },
    { .name = "OCT",                FMT_SET,            TOK_OCT,                    0               },
   
    //--------------------------------------------------------------------------------------------------------
    // Windows.
    //
    //--------------------------------------------------------------------------------------------------------
    { .name = "WON",                TOK_TYP_CMD,            CMD_WON,                    0               },
    { .name = "WOFF",               TOK_TYP_CMD,            CMD_WOFF,                   0               },
    { .name = "WDEF",               TOK_TYP_CMD,            CMD_WDEF,                   0               },
    { .name = "WSE",                TOK_TYP_CMD,            CMD_WSE,                    0               },
    { .name = "WSD",                TOK_TYP_CMD,            CMD_WSD,                    0               },
    
    { .name = "PSE",                TOK_TYP_CMD,            CMD_PSE,                    0               },
    { .name = "PSD",                TOK_TYP_CMD,            CMD_PSD,                    0               },
    { .name = "PSR",                TOK_TYP_CMD,            CMD_PSR,                    0               },
    
    { .name = "SRE",                TOK_TYP_CMD,            CMD_SRE,                    0               },
    { .name = "SRD",                TOK_TYP_CMD,            CMD_SRD,                    0               },
    { .name = "SRR",                TOK_TYP_CMD,            CMD_SRR,                    0               },
        
    { .name = "PLE",                TOK_TYP_CMD,            CMD_PLE,                    0               },
    { .name = "PLD",                TOK_TYP_CMD,            CMD_PLD,                    0               },
    { .name = "PLR",                TOK_TYP_CMD,            CMD_PLR,                    0               },

    { .name = "SWE",                TOK_TYP_CMD,            CMD_SWE,                    0               },
    { .name = "SWD",                TOK_TYP_CMD,            CMD_SWD,                    0               },
    { .name = "SWR",                TOK_TYP_CMD,            CMD_SWR,                    0               },
    
    { .name = "CWL",                TOK_TYP_CMD,            CMD_CWL,                    0               },
    
    { .name = "WE",                 TOK_TYP_CMD,            CMD_WE,                     0               },
    { .name = "WD",                 TOK_TYP_CMD,            CMD_WD,                     0               },
    { .name = "WR",                 TOK_TYP_CMD,            CMD_WR,                     0               },
    { .name = "WF",                 TOK_TYP_CMD,            CMD_WF,                     0               },
    { .name = "WB",                 TOK_TYP_CMD,            CMD_WB,                     0               },
    { .name = "WH",                 TOK_TYP_CMD,            CMD_WH,                     0               },
    { .name = "WJ",                 TOK_TYP_CMD,            CMD_WJ,                     0               },
    { .name = "WL",                 TOK_TYP_CMD,            CMD_WL,                     0               },
    { .name = "WN",                 TOK_TYP_CMD,            CMD_WN,                     0               },
    { .name = "WK",                 TOK_TYP_CMD,            CMD_WK,                     0               },
    { .name = "WC",                 TOK_TYP_CMD,            CMD_WC,                     0               },
    { .name = "WS",                 TOK_TYP_CMD,            CMD_WS,                     0               },
    { .name = "WT",                 TOK_TYP_CMD,            CMD_WT,                     0               },
    { .name = "WX",                 TOK_TYP_CMD,            CMD_WX,                     0               },
    
    { .name = "PM",                 TOK_TYP_NIL,            TOK_PM,                     0               },
    { .name = "PC",                 TOK_TYP_NIL,            TOK_PC,                     0               },
    { .name = "IT",                 TOK_TYP_NIL,            TOK_IT,                     0               },
    { .name = "DT",                 TOK_TYP_NIL,            TOK_DT,                     0               },
    { .name = "IC",                 TOK_TYP_NIL,            TOK_IC,                     0               },
    { .name = "DC",                 TOK_TYP_NIL,            TOK_DC,                     0               },
    { .name = "UC",                 TOK_TYP_NIL,            TOK_UC,                     0               },
    { .name = "ICR",                TOK_TYP_NIL,            TOK_ICR,                    0               },
    { .name = "DCR",                TOK_TYP_NIL,            TOK_DCR,                    0               },
    { .name = "UCR",                TOK_TYP_NIL,            TOK_UCR,                    0               },
    { .name = "MCR",                TOK_TYP_NIL,            TOK_MCR,                    0               },
    { .name = "ITR",                TOK_TYP_NIL,            TOK_ITR,                    0               },
    { .name = "DTR",                TOK_TYP_NIL,            TOK_DTR,                    0               },
    { .name = "PCR",                TOK_TYP_NIL,            TOK_PCR,                    0               },
    { .name = "IOR",                TOK_TYP_NIL,            TOK_IOR,                    0               },
    { .name = "TX",                 TOK_TYP_NIL,            TOK_TX,                     0               },
    
    //--------------------------------------------------------------------------------------------------------
    // General registers.
    //
    //--------------------------------------------------------------------------------------------------------
    { .name = "R0",                 TOK_TYP_GREG,             GR_0,                       0               },
    { .name = "R1",                 TOK_TYP_GREG,             GR_1,                       1               },
    { .name = "R2",                 TOK_TYP_GREG,             GR_2,                       2               },
    { .name = "R3",                 TOK_TYP_GREG,             GR_3,                       3               },
    { .name = "R4",                 TOK_TYP_GREG,             GR_4,                       4               },
    { .name = "R5",                 TOK_TYP_GREG,             GR_5,                       5               },
    { .name = "R6",                 TOK_TYP_GREG,             GR_6,                       6               },
    { .name = "R7",                 TOK_TYP_GREG,             GR_7,                       7               },
    { .name = "R8",                 TOK_TYP_GREG,             GR_8,                       8               },
    { .name = "R9",                 TOK_TYP_GREG,             GR_9,                       9               },
    { .name = "R10",                TOK_TYP_GREG,             GR_10,                      10              },
    { .name = "R11",                TOK_TYP_GREG,             GR_11,                      11              },
    { .name = "R12",                TOK_TYP_GREG,             GR_12,                      12              },
    { .name = "R13",                TOK_TYP_GREG,             GR_13,                      13              },
    { .name = "R14",                TOK_TYP_GREG,             GR_14,                      14              },
    { .name = "R15",                TOK_TYP_GREG,             GR_15,                      15              },

    //--------------------------------------------------------------------------------------------------------
    // Segment registers.
    //
    //--------------------------------------------------------------------------------------------------------
    { .name = "S0",                 TOK_TYP_SREG,             SR_0,                       0               },
    { .name = "S1",                 TOK_TYP_SREG,             SR_1,                       1               },
    { .name = "S2",                 TOK_TYP_SREG,             SR_2,                       2               },
    { .name = "S3",                 TOK_TYP_SREG,             SR_3,                       3               },
    { .name = "S4",                 TOK_TYP_SREG,             SR_4,                       4               },
    { .name = "S5",                 TOK_TYP_SREG,             SR_5,                       5               },
    { .name = "S6",                 TOK_TYP_SREG,             SR_6,                       6               },
    { .name = "S7",                 TOK_TYP_SREG,             SR_7,                       7               },
    
    //--------------------------------------------------------------------------------------------------------
    // Control registers.
    //
    //--------------------------------------------------------------------------------------------------------
    { .name = "C0",                 TOK_TYP_CREG,             CR_0,                       0               },
    { .name = "C1",                 TOK_TYP_CREG,             CR_1,                       1               },
    { .name = "C2",                 TOK_TYP_CREG,             CR_2,                       2               },
    { .name = "C3",                 TOK_TYP_CREG,             CR_3,                       3               },
    { .name = "C4",                 TOK_TYP_CREG,             CR_4,                       4               },
    { .name = "C5",                 TOK_TYP_CREG,             CR_5,                       5               },
    { .name = "C6",                 TOK_TYP_CREG,             CR_6,                       6               },
    { .name = "C7",                 TOK_TYP_CREG,             CR_7,                       7               },
    { .name = "C8",                 TOK_TYP_CREG,             CR_8,                       8               },
    { .name = "C9",                 TOK_TYP_CREG,             CR_9,                       9               },
    { .name = "C10",                TOK_TYP_CREG,             CR_10,                      10              },
    { .name = "C11",                TOK_TYP_CREG,             CR_11,                      11              },
    { .name = "C12",                TOK_TYP_CREG,             CR_12,                      12              },
    { .name = "C13",                TOK_TYP_CREG,             CR_13,                      13              },
    { .name = "C14",                TOK_TYP_CREG,             CR_14,                      14              },
    { .name = "C15",                TOK_TYP_CREG,             CR_15,                      15              },
    { .name = "C16",                TOK_TYP_CREG,             CR_16,                      16              },
    { .name = "C17",                TOK_TYP_CREG,             CR_17,                      17              },
    { .name = "C18",                TOK_TYP_CREG,             CR_18,                      18              },
    { .name = "C19",                TOK_TYP_CREG,             CR_19,                      19              },
    { .name = "C20",                TOK_TYP_CREG,             CR_20,                      20              },
    { .name = "C21",                TOK_TYP_CREG,             CR_21,                      21              },
    { .name = "C22",                TOK_TYP_CREG,             CR_22,                      22              },
    { .name = "C23",                TOK_TYP_CREG,             CR_23,                      23              },
    { .name = "C24",                TOK_TYP_CREG,             CR_24,                      24              },
    { .name = "C25",                TOK_TYP_CREG,             CR_25,                      25              },
    { .name = "C26",                TOK_TYP_CREG,             CR_26,                      26              },
    { .name = "C27",                TOK_TYP_CREG,             CR_27,                      27              },
    { .name = "C28",                TOK_TYP_CREG,             CR_28,                      28              },
    { .name = "C29",                TOK_TYP_CREG,             CR_29,                      29              },
    { .name = "C30",                TOK_TYP_CREG,             CR_30,                      30              },
    { .name = "C31",                TOK_TYP_CREG,             CR_31,                      31              },
        
    //--------------------------------------------------------------------------------------------------------
    // Assembler mnemonics.
    //
    //--------------------------------------------------------------------------------------------------------
    { .name = "LD",                 TOK_TYP_OP_CODE,        OP_CODE_LD,                 0xC0000000      },
    { .name = "LDB",                TOK_TYP_OP_CODE,        OP_CODE_LDB,                0xC0000000      },
    { .name = "LDH",                TOK_TYP_OP_CODE,        OP_CODE_LDH,                0xC0000000      },
    { .name = "LDW",                TOK_TYP_OP_CODE,        OP_CODE_LDW,                0xC0000000      },
    { .name = "LDR",                TOK_TYP_OP_CODE,        OP_CODE_LDR,                0xD0000000      },
    { .name = "LDA",                TOK_TYP_OP_CODE,        OP_CODE_LDA,                0xC8000000      },
        
    { .name = "ST",                 TOK_TYP_OP_CODE,        OP_CODE_ST,                 0xC4200000      },
    { .name = "STB",                TOK_TYP_OP_CODE,        OP_CODE_STB,                0xC4200000      },
    { .name = "STH",                TOK_TYP_OP_CODE,        OP_CODE_STH,                0xC4200000      },
    { .name = "STW",                TOK_TYP_OP_CODE,        OP_CODE_STW,                0xC4200000      },
    { .name = "STC",                TOK_TYP_OP_CODE,        OP_CODE_STC,                0xD4000000      },
    { .name = "STA",                TOK_TYP_OP_CODE,        OP_CODE_STA,                0xCC200000      },
        
    { .name = "ADD",                TOK_TYP_OP_CODE,        OP_CODE_ADD,                0x40000000      },
    { .name = "ADDB",               TOK_TYP_OP_CODE,        OP_CODE_ADDB,               0x40000000      },
    { .name = "ADDH",               TOK_TYP_OP_CODE,        OP_CODE_ADDH,               0x40000000      },
    { .name = "ADDW",               TOK_TYP_OP_CODE,        OP_CODE_ADDW,               0x40000000      },
        
    { .name = "ADC",                TOK_TYP_OP_CODE,        OP_CODE_ADC,                0x44000000      },
    { .name = "ADCB",               TOK_TYP_OP_CODE,        OP_CODE_ADCB,               0x44000000      },
    { .name = "ADCH",               TOK_TYP_OP_CODE,        OP_CODE_ADCH,               0x44000000      },
    { .name = "ADCW",               TOK_TYP_OP_CODE,        OP_CODE_ADCW,               0x44000000      },
        
    { .name = "SUB",                TOK_TYP_OP_CODE,        OP_CODE_SUB,                0x48000000      },
    { .name = "SUBB",               TOK_TYP_OP_CODE,        OP_CODE_SUBB,               0x48000000      },
    { .name = "SUBH",               TOK_TYP_OP_CODE,        OP_CODE_SUBH,               0x48000000      },
    { .name = "SUBW",               TOK_TYP_OP_CODE,        OP_CODE_SUBW,               0x48000000      },
        
    { .name = "SBC",                TOK_TYP_OP_CODE,        OP_CODE_SBC,                0x4C000000      },
    { .name = "SBCB",               TOK_TYP_OP_CODE,        OP_CODE_SBCB,               0x4C000000      },
    { .name = "SBCH",               TOK_TYP_OP_CODE,        OP_CODE_SBCH,               0x4C000000      },
    { .name = "SBCW",               TOK_TYP_OP_CODE,        OP_CODE_SBCW,               0x4C000000      },
        
    { .name = "AND",                TOK_TYP_OP_CODE,        OP_CODE_AND,                0x50000000      },
    { .name = "ANDB",               TOK_TYP_OP_CODE,        OP_CODE_ANDB,               0x50000000      },
    { .name = "ANDH",               TOK_TYP_OP_CODE,        OP_CODE_ANDH,               0x50000000      },
    { .name = "ANDW",               TOK_TYP_OP_CODE,        OP_CODE_ANDW,               0x50000000      },
        
    { .name = "OR" ,                TOK_TYP_OP_CODE,        OP_CODE_OR,                 0x54000000      },
    { .name = "ORB",                TOK_TYP_OP_CODE,        OP_CODE_ORB,                0x54000000      },
    { .name = "ORH",                TOK_TYP_OP_CODE,        OP_CODE_ORH,                0x54000000      },
    { .name = "ORW",                TOK_TYP_OP_CODE,        OP_CODE_ORW,                0x54000000      },
        
    { .name = "XOR" ,               TOK_TYP_OP_CODE,        OP_CODE_XOR,                0x58000000      },
    { .name = "XORB",               TOK_TYP_OP_CODE,        OP_CODE_XORB,               0x58000000      },
    { .name = "XORH",               TOK_TYP_OP_CODE,        OP_CODE_XORH,               0x58000000      },
    { .name = "XORW",               TOK_TYP_OP_CODE,        OP_CODE_XORW,               0x58000000      },
        
    { .name = "CMP" ,               TOK_TYP_OP_CODE,        OP_CODE_CMP,                0x5C000000      },
    { .name = "CMPB",               TOK_TYP_OP_CODE,        OP_CODE_CMPB,               0x5C000000      },
    { .name = "CMPH",               TOK_TYP_OP_CODE,        OP_CODE_CMPH,               0x5C000000      },
    { .name = "CMPW",               TOK_TYP_OP_CODE,        OP_CODE_CMPW,               0x5C000000      },
        
    { .name = "CMPU" ,              TOK_TYP_OP_CODE,        OP_CODE_CMPU,               0x60000000      },
    { .name = "CMPUB",              TOK_TYP_OP_CODE,        OP_CODE_CMPUB,              0x60000000      },
    { .name = "CMPUH",              TOK_TYP_OP_CODE,        OP_CODE_CMPUH,              0x60000000      },
    { .name = "CMPUW",              TOK_TYP_OP_CODE,        OP_CODE_CMPUW,              0x60000000      },
    
    { .name = "LSID" ,              TOK_TYP_OP_CODE,        OP_CODE_LSID,               0x10000000      },
    { .name = "EXTR" ,              TOK_TYP_OP_CODE,        OP_CODE_EXTR,               0x14000000      },
    { .name = "DEP",                TOK_TYP_OP_CODE,        OP_CODE_DEP,                0x18000000      },
    { .name = "DSR",                TOK_TYP_OP_CODE,        OP_CODE_DSR,                0x1C000000      },
    { .name = "SHLA",               TOK_TYP_OP_CODE,        OP_CODE_SHLA,               0x20000000      },
    { .name = "CMR",                TOK_TYP_OP_CODE,        OP_CODE_CMR,                0x24000000      },
        
    { .name = "LDIL",               TOK_TYP_OP_CODE,        OP_CODE_LDIL,               0x04000000      },
    { .name = "ADDIL",              TOK_TYP_OP_CODE,        OP_CODE_ADDIL,              0x08000000      },
    { .name = "LDO",                TOK_TYP_OP_CODE,        OP_CODE_LDO,                0x0C000000      },
    
    { .name = "B",                  TOK_TYP_OP_CODE,        OP_CODE_B,                  0x80000000      },
    { .name = "GATE",               TOK_TYP_OP_CODE,        OP_CODE_GATE,               0x84000000      },
    { .name = "BR",                 TOK_TYP_OP_CODE,        OP_CODE_BR,                 0x88000000      },
    { .name = "BV",                 TOK_TYP_OP_CODE,        OP_CODE_BV,                 0x8C000000      },
    { .name = "BE",                 TOK_TYP_OP_CODE,        OP_CODE_BE,                 0x90000000      },
    { .name = "BVE",                TOK_TYP_OP_CODE,        OP_CODE_BVE,                0x94000000      },
    { .name = "CBR",                TOK_TYP_OP_CODE,        OP_CODE_CBR,                0x98000000      },
    { .name = "CBRU",               TOK_TYP_OP_CODE,        OP_CODE_CBRU,               0x9C000000      },
        
    { .name = "MR",                 TOK_TYP_OP_CODE,        OP_CODE_MR,                 0x28000000      },
    { .name = "MST",                TOK_TYP_OP_CODE,        OP_CODE_MST,                0x2C000000      },
    { .name = "DS",                 TOK_TYP_OP_CODE,        OP_CODE_DS,                 0x30000000      },
    { .name = "LDPA",               TOK_TYP_OP_CODE,        OP_CODE_LDPA,               0xE4000000      },
    { .name = "PRB",                TOK_TYP_OP_CODE,        OP_CODE_PRB,                0xE8000000      },
    { .name = "ITLB",               TOK_TYP_OP_CODE,        OP_CODE_ITLB,               0xEC000000      },
    { .name = "PTLB",               TOK_TYP_OP_CODE,        OP_CODE_PTLB,               0xF0000000      },
    { .name = "PCA",                TOK_TYP_OP_CODE,        OP_CODE_PCA,                0xF4000000      },
    { .name = "DIAG",               TOK_TYP_OP_CODE,        OP_CODE_DIAG,               0xF8000000      },
    { .name = "RFI",                TOK_TYP_OP_CODE,        OP_CODE_RFI,                0xFC000000      },
    { .name = "BRK",                TOK_TYP_OP_CODE,        OP_CODE_BRK,                0x00000000      },
        
    //--------------------------------------------------------------------------------------------------------
    // Synthetic instruction mnemonics.
    //
    //--------------------------------------------------------------------------------------------------------
    { .name = "NOP",                TOK_TYP_OP_CODE_S,      OP_CODE_S_NOP,              0               },
 
    //--------------------------------------------------------------------------------------------------------
    //
    //
    //--------------------------------------------------------------------------------------------------------
    { .name = "IA-SEG",             TOK_TYP_PSTATE_PREG,             PS_IA_SEG,                  0               },
    { .name = "IA-OFS",             TOK_TYP_PSTATE_PREG,             PS_IA_OFS,                  0               },
    { .name = "ST-REG",             TOK_TYP_PSTATE_PREG,             PS_STATUS,                  0               },
    
    { .name = "FD-IA-SEG",          TOK_TYP_FD_PREG,             FD_IA_SEG,                  0               },
    { .name = "FD-IA-OFS",          TOK_TYP_FD_PREG,             FD_IA_OFS,                  0               },
    { .name = "FD-INSTR",           TOK_TYP_FD_PREG,             FD_INSTR,                   0               },
    { .name = "FD-A",               TOK_TYP_FD_PREG,             FD_A,                       0               },
    { .name = "FD-B",               TOK_TYP_FD_PREG,             FD_B,                       0               },
    { .name = "FD-X",               TOK_TYP_FD_PREG,             FD_X,                       0               },
    
    { .name = "OF-IA-SEG",          TOK_TYP_OF_PREG,             OF_IA_SEG,                  0               },
    { .name = "OF-IA-OFS",          TOK_TYP_OF_PREG,             OF_IA_OFS,                  0               },
    { .name = "OF-INSTR",           TOK_TYP_OF_PREG,             OF_INSTR,                   0               },
    { .name = "OF-A",               TOK_TYP_OF_PREG,             OF_A,                       0               },
    { .name = "OF-B",               TOK_TYP_OF_PREG,             OF_B,                       0               },
    { .name = "OF-X",               TOK_TYP_OF_PREG,             OF_X,                       0               },
    { .name = "OF-S",               TOK_TYP_OF_PREG,             OF_S,                       0               },
    
    { .name = "IC-L1-STATE",        TOK_TYP_IC_L1_REG,          IC_L1_STATE,                0               },
    { .name = "IC-L1-REQ",          TOK_TYP_IC_L1_REG,          IC_L1_REQ,                  0               },
    { .name = "IC-L1-REQ-SEG",      TOK_TYP_IC_L1_REG,          IC_L1_REQ_SEG,              0               },
    { .name = "IC-L1-REQ-OFS",      TOK_TYP_IC_L1_REG,          IC_L1_REQ_OFS,              0               },
    { .name = "IC-L1-REQ-TAG",      TOK_TYP_IC_L1_REG,          IC_L1_REQ_TAG,              0               },
    { .name = "IC-L1-REQ-LEN",      TOK_TYP_IC_L1_REG,          IC_L1_REQ_LEN,              0               },
    { .name = "IC-L1-REQ-LAT",      TOK_TYP_IC_L1_REG,          IC_L1_LATENCY,              0               },
    { .name = "IC-L1-SETS",         TOK_TYP_IC_L1_REG,          IC_L1_SETS,                 0               },
    { .name = "IC-L1-ENTRIES",      TOK_TYP_IC_L1_REG,          IC_L1_BLOCK_ENTRIES,        0               },
    { .name = "IC-L1-B-SIZE",       TOK_TYP_IC_L1_REG,          IC_L1_BLOCK_SIZE,           0               },
   
    { .name = "DC-L1-STATE",        TOK_TYP_DC_L1_REG,          DC_L1_STATE,                0               },
    { .name = "DC-L1-REQ",          TOK_TYP_DC_L1_REG,          DC_L1_REQ,                  0               },
    { .name = "DC-L1-REQ-SEG",      TOK_TYP_DC_L1_REG,          DC_L1_REQ_SEG,              0               },
    { .name = "DC-L1-REQ-OFS",      TOK_TYP_DC_L1_REG,          DC_L1_REQ_OFS,              0               },
    { .name = "DC-L1-REQ-TAG",      TOK_TYP_DC_L1_REG,          DC_L1_REQ_TAG,              0               },
    { .name = "DC-L1-REQ-LEN",      TOK_TYP_DC_L1_REG,          DC_L1_REQ_LEN,              0               },
    { .name = "DC-L1-REQ-LAT",      TOK_TYP_DC_L1_REG,          DC_L1_LATENCY,              0               },
    { .name = "DC-L1-SETS",         TOK_TYP_DC_L1_REG,          DC_L1_SETS,                 0               },
    { .name = "DC-L1-ENTRIES",      TOK_TYP_DC_L1_REG,          DC_L1_BLOCK_ENTRIES,        0               },
    { .name = "DC-L1-B-SIZE",       TOK_TYP_DC_L1_REG,          DC_L1_BLOCK_SIZE,           0               },
  
    { .name = "UC-L2-STATE",        TOK_TYP_UC_L2_REG,          UC_L2_STATE,                0               },
    { .name = "UC-L2-REQ",          TOK_TYP_UC_L2_REG,          UC_L2_REQ,                  0               },
    { .name = "UC-L2-REQ-SEG",      TOK_TYP_UC_L2_REG,          UC_L2_REQ_SEG,              0               },
    { .name = "UC-L2-REQ-OFS",      TOK_TYP_UC_L2_REG,          UC_L2_REQ_OFS,              0               },
    { .name = "UC-L2-REQ-TAG",      TOK_TYP_UC_L2_REG,          UC_L2_REQ_TAG,              0               },
    { .name = "UC-L2-REQ-LEN",      TOK_TYP_UC_L2_REG,          UC_L2_REQ_LEN,              0               },
    { .name = "UC-L2-REQ-LAT",      TOK_TYP_UC_L2_REG,          UC_L2_LATENCY,              0               },
    { .name = "UC-L2-SETS",         TOK_TYP_UC_L2_REG,          UC_L2_SETS,                 0               },
    { .name = "UC-L2-ENTRIES",      TOK_TYP_UC_L2_REG,          UC_L2_BLOCK_ENTRIES,        0               },
    { .name = "UC-L2-B-SIZE",       TOK_TYP_UC_L2_REG,          UC_L2_BLOCK_SIZE,           0               },
    
    { .name = "ITLB-STATE",         TOK_TYP_ITLB_REG,           ITLB_STATE,                 0               },
    { .name = "ITLB-REQ",           TOK_TYP_ITLB_REG,           ITLB_REQ,                   0               },
    { .name = "ITLB-REQ-SEG",       TOK_TYP_ITLB_REG,           ITLB_REQ_SEG,               0               },
    { .name = "ITLB-REQ-OFS",       TOK_TYP_ITLB_REG,           ITLB_REQ_OFS,               0               },
    
    { .name = "DTLB-STATE",         TOK_TYP_DTLB_REG,           DTLB_STATE,                 0               },
    { .name = "DTLB-REQ",           TOK_TYP_DTLB_REG,           DTLB_REQ,                   0               },
    { .name = "DTLB-REQ-SEG",       TOK_TYP_DTLB_REG,           DTLB_REQ_SEG,               0               },
    { .name = "DTLB-REQ-OFS",       TOK_TYP_DTLB_REG,           DTLB_REQ_OFS,               0               },
    
    //--------------------------------------------------------------------------------------------------------
    //
    //
    //--------------------------------------------------------------------------------------------------------
    { "GR-SET",             REG_SET,            TOK_TYP_GREG,                     0               },
    { "GR",                 REG_SET,            TOK_TYP_GREG,                     0               },
    
    { "SR-SET",             REG_SET,            TOK_TYP_SREG,                     0               },
    { "SR",                 REG_SET,            TOK_TYP_SREG,                     0               },
    
    { "CR-SET",             REG_SET,            TOK_TYP_CREG,                     0               },
    { "CR",                 REG_SET,            TOK_TYP_CREG,                     0               },
    
    { "PS-SET",             REG_SET,            TOK_TYP_PSTATE_PREG,                     0               },
    { "PS",                 REG_SET,            TOK_TYP_PSTATE_PREG,                     0               },
   
    { "PR-SET",             REG_SET,            PR_SET,                     0               },
    { "PR",                 REG_SET,            PR_SET,                     0               },
    
    { "FD-SET",             REG_SET,            TOK_TYP_FD_PREG,                     0               },
    { "PR",                 REG_SET,            TOK_TYP_FD_PREG,                     0               },
    
    { "MA-SET",             REG_SET,            TOK_TYP_OF_PREG,                     0               },
    { "PR",                 REG_SET,            TOK_TYP_OF_PREG,                     0               },
    
    { "IC-L1-SET",          REG_SET,            TOK_TYP_IC_L1_REG,                  0               },
    { "ICL1",               REG_SET,            TOK_TYP_IC_L1_REG,                  0               },

    { "DC-L1-SET",          REG_SET,            TOK_TYP_DC_L1_REG,                  0               },
    { "DCL1",               REG_SET,            TOK_TYP_DC_L1_REG,                  0               },
    
    { "UC-L2-SET",          REG_SET,            TOK_TYP_UC_L2_REG,                  0               },
    { "UCl2",               REG_SET,            TOK_TYP_UC_L2_REG,                  0               },
    
    { "ITLB-SET",           REG_SET,            TOK_TYP_ITLB_REG,                   0               },
    { "ITRS",               REG_SET,            TOK_TYP_ITLB_REG,                   0               },
    
    { "DTLB-SET",           REG_SET,            TOK_TYP_DTLB_REG,                   0               },
    { "DTRS",               REG_SET,            TOK_TYP_DTLB_REG,                   0               },
    
    { "REG-SET-ALL",        REG_SET,            REG_SET_ALL,                0               },
    { "RS",                 REG_SET,            REG_SET_ALL,                0               },
    
    { "",                   TOK_TYP_NIL,            TOK_NIL,                    0               },
    
    { .name = "",  .tid = TOK_LAST, .typ = TOK_NIL }
};

const int   TOK_TAB_SIZE  = sizeof( tokTab ) / sizeof( *tokTab );
#endif

//------------------------------------------------------------------------------------------------------------
// The lokkup function. We just do a linear search for now. Note that we expect the last entry in the token
// table to be the NIL token, otherwise bad things will happen.
//
//------------------------------------------------------------------------------------------------------------
int lookupToken( char *str, DrvToken *tokTab ) {
    
    if (( strlen( str ) == 0 ) || ( strlen ( str ) > TOK_NAME_SIZE )) return( -1 );
    
    DrvToken *tok = tokTab;
    
    while ( tok -> tid != TOK_NIL ) {
        
        if ( strcmp( str, tok -> name ) == 0 )  return((int) ( tok - tokTab ));
        else                                    tok ++;
    }
    
    return( -1 );
}

//------------------------------------------------------------------------------------------------------------
// Little helper functions.
//
//------------------------------------------------------------------------------------------------------------
void upshiftStr( char *str ) {
    
    size_t len = strlen( str );
    
    if ( len > 0 ) {
        
        for ( size_t i = 0; i < len; i++ ) str[ i ] = (char) toupper((int) str[ i ] );
    }
}

}; // namespace


//------------------------------------------------------------------------------------------------------------
// The object constructor, nothing to do for now.
//
//------------------------------------------------------------------------------------------------------------
DrvTokenizer::DrvTokenizer( ) { }

//------------------------------------------------------------------------------------------------------------
// We initialize a couple of globals that represent the current state of the parsing process. This call is
// the first before any other method can be called.
//
//------------------------------------------------------------------------------------------------------------
uint8_t DrvTokenizer::setupTokenizer( char *lineBuf, DrvToken *tokTab ) {
    
    strncpy( tokenLine, lineBuf, strlen( lineBuf ) + 1 );
    upshiftStr( tokenLine );
    
    this -> tokTab                  = tokTab;
    this -> currentLineLen          = (int) strlen( tokenLine );
    this -> currentCharIndex        = 0;
    this -> currentTokCharIndex     = 0;
    this -> currentChar             = ' ';
    return( NO_ERR );
}

//------------------------------------------------------------------------------------------------------------
// "parserError" is a little helper that prints out the error encountered. We will print the original
// input line, a caret marker where we found the error, and then return a false. Parsing errors typically
// result in aborting the parsing process. As this is a one line assembly, we do not need to be put effort
// into continuing reasonably with the parsing process.
//
//------------------------------------------------------------------------------------------------------------
void DrvTokenizer::tokenError( char *errStr ) {
    
    fprintf( stdout, "%s\n", tokenLine );
    
    int i = 0;

    while ( i < currentTokCharIndex ) {
        
        fprintf( stdout, " " );
        i ++;
    }
    
    fprintf( stdout, "^\n" "%s\n", errStr );
}

//------------------------------------------------------------------------------------------------------------
// helper functions for the current token.
//
//------------------------------------------------------------------------------------------------------------
bool        DrvTokenizer::isToken( TokId tokId )    { return( currentTokId == tokId ); }
bool        DrvTokenizer::isTokenTyp( TokId typId ) { return( currentTokTyp == typId ); }
TokId       DrvTokenizer::tokTyp( )                 { return( currentTokTyp ); }
TokId       DrvTokenizer::tokId( )                  { return( currentTokId ); }
int         DrvTokenizer::tokVal( )                 { return( currentTokVal ); }
char        *DrvTokenizer::tokStr( )                { return( currentTokStr ); }
int         DrvTokenizer::tokCharIndex( )           { return( currentCharIndex ); }
char        *DrvTokenizer::tokenLineStr( )          { return(  tokenLine ); }

//------------------------------------------------------------------------------------------------------------
// "nextChar" returns the next character from the token line string.
//
//------------------------------------------------------------------------------------------------------------
void DrvTokenizer::nextChar( ) {
    
    if ( currentCharIndex < currentLineLen ) {
        
        currentChar = tokenLine[ currentCharIndex ];
        currentCharIndex ++;
    }
    else currentChar = EOS_CHAR;
}

//------------------------------------------------------------------------------------------------------------
// "parseNum" will parse a number. We leave the heavy lifting of converting the numeric value to the C library.
//
//------------------------------------------------------------------------------------------------------------
void DrvTokenizer::parseNum( int sign ) {
    
    char tmpStr[ TOK_INPUT_LINE_SIZE ]  = "";
    
    currentTokTyp       = TOK_TYP_NUM;
    currentTokId        = TOK_NUM;
    currentTokVal       = 0;
    currentTokStr[ 0 ]  = '\0';
    
    do {
        
        strcat( tmpStr, &currentChar );
        nextChar( );
        
    } while ( isxdigit( currentChar ) || ( currentChar == 'X' ) || ( currentChar == 'O' ));
    
    if ( sscanf( tmpStr, "%i", &currentTokVal ) == 1 ) {
        
        currentTokVal   = currentTokVal * sign;
    }
    else {
        
        currentTokVal   = 0;
        tokenError((char *) "Invalid number" );
    }
}

//------------------------------------------------------------------------------------------------------------
// "parseString" gets a string. We manage special characters inside the string with the "\" prefix. Right
// now, we do not use strings, so the function is perhaps for the future. We will just parse it, but record
// no result. One day, the entire simulator might use the lexer functions. Then we need it.
//
//------------------------------------------------------------------------------------------------------------
void DrvTokenizer::parseString( ) {

    currentTokTyp       = TOK_TYP_STR;
    currentTokId        = TOK_STR;
    currentTokVal       = 0;
    currentTokStr[ 0 ]  = '\0';

    nextChar( );
          
    while (( currentChar != EOS_CHAR ) && ( currentChar != '"' )) {

        if ( currentChar == '\\' ) {
            
            nextChar( );
            if ( currentChar != EOS_CHAR ) {
                
                if      ( currentChar == 'n' )  strcat( currentTokStr, (char *) "\n" );
                else if ( currentChar == 't' )  strcat( currentTokStr, (char *) "\t" );
                else if ( currentChar == '\\' ) strcat( currentTokStr, (char *) "\\" );
                else                            strcat( currentTokStr, &currentChar );
            }
            else {
                
                currentTokStr[ 0 ]  = '\0';
                tokenError((char *) "Expected a closing quote" );
                break;
            }
        }
        else strcat( currentTokStr, &currentChar );

        nextChar( );
    }

    nextChar( );
}

//------------------------------------------------------------------------------------------------------------
// "parseIdent" parses an identifier. It is a sequence of characters starting with an alpha character. We do
// not really have user defined identifiers, only reserved words. As a qualified constant also begins with
// a character, the parsing of an identifier also needs to manage the parsing of constants with a qualifier,
// such as "L%nnn".  We first check for these kind of qualifiers and if found hand over to parse a number.
//
//------------------------------------------------------------------------------------------------------------
void DrvTokenizer::parseIdent( ) {
    
    currentTokTyp       = TOK_TYP_IDENT;
    currentTokId        = TOK_IDENT;
    currentTokVal       = 0;
    currentTokStr[ 0 ]  = '\0';
    
    char identBuf[ TOK_INPUT_LINE_SIZE ] = "";
    
    if ( currentChar == 'L' ) {
        
        strcat( identBuf, &currentChar );
        nextChar( );
        
        if ( currentChar == '%' ) {
            
            strcat( identBuf, &currentChar );
            nextChar( );
            
            if ( isdigit( currentChar )) {
                
                parseNum( 1 );
                currentTokVal &= 0xFFFFFC00;
                return;
            }
            else tokenError((char *) "Invalid character in identifier" );
        }
    }
    else if ( currentChar == 'R' ) {
        
        strcat( identBuf, &currentChar );
        nextChar( );
        
        if ( currentChar == '%' ) {
            
            strcat( identBuf, &currentChar );
            nextChar( );
            
            if ( isdigit( currentChar )) {
                
                parseNum( 1 );
                currentTokVal &= 0x3FF;
                return;
            }
            else tokenError((char *) "Invalid character in identifier" );
        }
    }
    
    while ( isalnum( currentChar )) {
        
        strcat( identBuf, &currentChar );
        nextChar( );
    }
    
    int index = lookupToken( identBuf, tokTab );
    
    if ( index == -1 ) {
        
        currentTokTyp       = TOK_TYP_IDENT;
        currentTokId        = TOK_IDENT;
        currentTokVal       = 0;
        strcpy( currentTokStr, identBuf );
    }
    else {
        
        currentTokTyp       = tokTab[ index ].typ;
        currentTokId        = tokTab[ index ].tid;
        currentTokVal       = tokTab[ index ].val;
        strcpy( currentTokStr, identBuf );
    }
}

//------------------------------------------------------------------------------------------------------------
// "nextToken" is the entry point to the token business. It returns the next token from the input string.
//
//------------------------------------------------------------------------------------------------------------
void DrvTokenizer::nextToken( ) {

    currentTokTyp       = TOK_TYP_NIL;
    currentTokId        = TOK_NIL;
    currentTokVal       = 0;
    currentTokStr[ 0 ]  = '\0';
    
    while (( currentChar == ' ' ) || ( currentChar == '\n' ) || ( currentChar == '\n' )) nextChar( );
    
    currentTokCharIndex = currentCharIndex - 1;
    
    if ( isalpha( currentChar )) {
        
       parseIdent( );
    }
    else if ( isdigit( currentChar )) {
        
       parseNum( 1 );
    }
    else if ( currentChar == '"' ) {
        
        parseString( );
    }
    else if ( currentChar == '.' ) {
        
        currentTokTyp   = TOK_TYP_SYM;
        currentTokId    = TOK_PERIOD;
        nextChar( );
    }
    else if ( currentChar == '+' ) {
        
        currentTokId = TOK_PLUS;
        nextChar( );
    }
    else if ( currentChar == '-' ) {
        
        currentTokTyp   = TOK_TYP_SYM;
        currentTokId    = TOK_MINUS;
        nextChar( );
    }
    else if ( currentChar == '*' ) {
        
        currentTokTyp   = TOK_TYP_SYM;
        currentTokId    = TOK_MULT;
        nextChar( );
    }
    else if ( currentChar == '/' ) {
        
        currentTokTyp   = TOK_TYP_SYM;
        currentTokId    = TOK_DIV;
        nextChar( );
    }
    else if ( currentChar == '%' ) {
        
        currentTokTyp   = TOK_TYP_SYM;
        currentTokId    = TOK_MOD;
        nextChar( );
    }
    else if ( currentChar == '|' ) {
        
        currentTokTyp   = TOK_TYP_SYM;
        currentTokId    = TOK_OR;
        nextChar( );
    }
    else if ( currentChar == '^' ) {
        
        currentTokTyp   = TOK_TYP_SYM;
        currentTokId    = TOK_XOR;
        nextChar( );
    }
    else if ( currentChar == '~' ) {
        
        currentTokTyp   = TOK_TYP_SYM;
        currentTokId    = TOK_NEG;
        nextChar( );
    }
    else if ( currentChar == '(' ) {
        
        currentTokTyp   = TOK_TYP_SYM;
        currentTokId    = TOK_LPAREN;
        nextChar( );
    }
    else if ( currentChar == ')' ) {
        
        currentTokTyp   = TOK_TYP_SYM;
        currentTokId    = TOK_RPAREN;
        nextChar( );
    }
    else if ( currentChar == ',' ) {
        
        currentTokTyp   = TOK_TYP_SYM;
        currentTokId    = TOK_COMMA;
        nextChar( );
    }
    else if ( currentChar == EOS_CHAR ) {
        
        currentTokTyp   = TOK_TYP_SYM;
        currentTokId    = TOK_EOS;
    }
    else {
        
        tokenError((char *) "Invalid character in input string" );
        currentTokId = TOK_ERR;
    }
}
