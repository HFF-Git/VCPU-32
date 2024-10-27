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


//------------------------------------------------------------------------------------------------------------
// The global token table. All reserved words are allocated in this table. Each entry has the token name,
// the token id, the token group id, i.e. its type, and a value associatd with the token.
//
// ??? do some sorting, better readbility....
//------------------------------------------------------------------------------------------------------------
struct {
    
    char        name[ TOK_NAME_SIZE ];
    TokId       tokGrpId;
    TokId       tokId;
    uint32_t    tokVal;
    
} const tokTab[ ] = {
    
    //--------------------------------------------------------------------------------------------------------
    //
    //
    //--------------------------------------------------------------------------------------------------------
    { "COMMENT",            CMD_SET,            CMD_COMMENT,                0               },
    { "#",                  CMD_SET,            CMD_COMMENT,                0               },
    { "ENV",                CMD_SET,            CMD_ENV,                    0               },
    { "EXIT",               CMD_SET,            CMD_EXIT,                   0               },
    { "E",                  CMD_SET,            CMD_EXIT,                   0               },
    { "HELP",               CMD_SET,            CMD_HELP,                   0               },
    { "?",                  CMD_SET,            CMD_HELP,                   0               },
    { "WHELP",              CMD_SET,            CMD_WHELP,                  0               },
    { "RESET",              CMD_SET,            CMD_RESET,                  0               },
    { "RUN",                CMD_SET,            CMD_RUN,                    0               },
    { "STEP",               CMD_SET,            CMD_STEP,                   0               },
    { "S",                  CMD_SET,            CMD_STEP,                   0               },
    { "DIS",                CMD_SET,            CMD_DIS_ASM,                0               },
    { "ASM",                CMD_SET,            CMD_ASM,                    0               },
    
    { "B",                  CMD_SET,            CMD_B,                      0               },
    { "BD",                 CMD_SET,            CMD_BD,                     0               },
    { "BL",                 CMD_SET,            CMD_BL,                     0               },
    
    { "EXEC-F",             CMD_SET,            CMD_XF,                     0               },
    { "XF",                 CMD_SET,            CMD_XF,                     0               },
    
    { "D-REG",              CMD_SET,            CMD_DR,                     0               },
    { "DR",                 CMD_SET,            CMD_DR,                     0               },
    
    { "M-REG",              CMD_SET,            CMD_MR,                     0               },
    { "MR",                 CMD_SET,            CMD_MR,                     0               },
    
    { "HASH-VA",            CMD_SET,            CMD_HASH_VA,                0               },
    { "HVA",                CMD_SET,            CMD_HASH_VA,                0               },
    
    { "I-TLB",              CMD_SET,            CMD_I_TLB,                  0               },
    { "ITLB",               CMD_SET,            CMD_I_TLB,                  0               },
    
    { "D-TLB",              CMD_SET,            CMD_D_TLB,                  0               },
    { "DTLB",               CMD_SET,            CMD_D_TLB,                  0               },
    
    { "P-TLB",              CMD_SET,            CMD_P_TLB,                  0               },
    { "PTLB",               CMD_SET,            CMD_P_TLB,                  0               },
    
    { "D-CACHE",            CMD_SET,            CMD_D_CACHE,                0               },
    { "DCA",                CMD_SET,            CMD_D_CACHE,                0               },
    
    { "P-CACHE",            CMD_SET,            CMD_P_CACHE,                0               },
    { "PCA",                CMD_SET,            CMD_P_CACHE,                0               },

    { "D-ABS",              CMD_SET,            CMD_DA,                     0               },
    { "DA",                 CMD_SET,            CMD_DA,                     0               },
    
    { "M-ABS",              CMD_SET,            CMD_MA,                     0               },
    { "MA",                 CMD_SET,            CMD_MA,                     0               },
    
    { "D-ABS-ASM",          CMD_SET,            CMD_DAA,                    0               },
    { "DAA",                CMD_SET,            CMD_DAA,                    0               },
    
    { "M-ABS-ASM",          CMD_SET,            CMD_MAA,                    0               },
    { "MAA",                CMD_SET,            CMD_MAA,                    0               },
    
    { "LOAD-MEM",           CMD_SET,            CMD_LMF,                    0               },
    { "SAVE-MEM",           CMD_SET,            CMD_SMF,                    0               },
    
    { "TRUE",               SET_NIL,            TOK_TRUE,                   0               },
    { "FALSE",              SET_NIL,            TOK_FALSE,                  0               },
    { "ALL",                SET_NIL,            TOK_ALL,                    0               },
    { "CPU",                SET_NIL,            TOK_CPU,                    0               },
    { "MEM",                SET_NIL,            TOK_MEM,                    0               },
    { "C",                  SET_NIL,            TOK_C,                      0               },
    { "D",                  SET_NIL,            TOK_D,                      0               },
    { "F",                  SET_NIL,            TOK_F,                      0               },
    { "I",                  SET_NIL,            TOK_I,                      0               },
    { "T",                  SET_NIL,            TOK_T,                      0               },
    { "U",                  SET_NIL,            TOK_U,                      0               },
    
    { "DEC",                FMT_SET,            TOK_DEC,                    0               },
    { "HEX",                FMT_SET,            TOK_HEX,                    0               },
    { "OCT",                FMT_SET,            TOK_OCT,                    0               },
   
    //--------------------------------------------------------------------------------------------------------
    // Windows.
    //
    //--------------------------------------------------------------------------------------------------------
    { "WON",                CMD_SET,            CMD_WON,                    0               },
    { "WOFF",               CMD_SET,            CMD_WOFF,                   0               },
    { "WDEF",               CMD_SET,            CMD_WDEF,                   0               },
    { "WSE",                CMD_SET,            CMD_WSE,                    0               },
    { "WSD",                CMD_SET,            CMD_WSD,                    0               },
    
    { "PSE",                CMD_SET,            CMD_PSE,                    0               },
    { "PSD",                CMD_SET,            CMD_PSD,                    0               },
    { "PSR",                CMD_SET,            CMD_PSR,                    0               },
    
    { "SRE",                CMD_SET,            CMD_SRE,                    0               },
    { "SRD",                CMD_SET,            CMD_SRD,                    0               },
    { "SRR",                CMD_SET,            CMD_SRR,                    0               },
        
    { "PLE",                CMD_SET,            CMD_PLE,                    0               },
    { "PLD",                CMD_SET,            CMD_PLD,                    0               },
    { "PLR",                CMD_SET,            CMD_PLR,                    0               },

    { "SWE",                CMD_SET,            CMD_SWE,                    0               },
    { "SWD",                CMD_SET,            CMD_SWD,                    0               },
    { "SWR",                CMD_SET,            CMD_SWR,                    0               },
    
    { "CWL",                CMD_SET,            CMD_CWL,                    0               },
    
    { "WE",                 CMD_SET,            CMD_WE,                     0               },
    { "WD",                 CMD_SET,            CMD_WD,                     0               },
    { "WR",                 CMD_SET,            CMD_WR,                     0               },
    { "WF",                 CMD_SET,            CMD_WF,                     0               },
    { "WB",                 CMD_SET,            CMD_WB,                     0               },
    { "WH",                 CMD_SET,            CMD_WH,                     0               },
    { "WJ",                 CMD_SET,            CMD_WJ,                     0               },
    { "WL",                 CMD_SET,            CMD_WL,                     0               },
    { "WN",                 CMD_SET,            CMD_WN,                     0               },
    { "WK",                 CMD_SET,            CMD_WK,                     0               },
    { "WC",                 CMD_SET,            CMD_WC,                     0               },
    { "WS",                 CMD_SET,            CMD_WS,                     0               },
    { "WT",                 CMD_SET,            CMD_WT,                     0               },
    { "WX",                 CMD_SET,            CMD_WX,                     0               },
    
    { "PM",                 SET_NIL,            TOK_PM,                     0               },
    { "PC",                 SET_NIL,            TOK_PC,                     0               },
    { "IT",                 SET_NIL,            TOK_IT,                     0               },
    { "DT",                 SET_NIL,            TOK_DT,                     0               },
    { "IC",                 SET_NIL,            TOK_IC,                     0               },
    { "DC",                 SET_NIL,            TOK_DC,                     0               },
    { "UC",                 SET_NIL,            TOK_UC,                     0               },
    { "ICR",                SET_NIL,            TOK_ICR,                    0               },
    { "DCR",                SET_NIL,            TOK_DCR,                    0               },
    { "UCR",                SET_NIL,            TOK_UCR,                    0               },
    { "MCR",                SET_NIL,            TOK_MCR,                    0               },
    { "ITR",                SET_NIL,            TOK_ITR,                    0               },
    { "DTR",                SET_NIL,            TOK_DTR,                    0               },
    { "PCR",                SET_NIL,            TOK_PCR,                    0               },
    { "IOR",                SET_NIL,            TOK_IOR,                    0               },
    { "TX",                 SET_NIL,            TOK_TX,                     0               },
    
    //--------------------------------------------------------------------------------------------------------
    // General registers.
    //
    //--------------------------------------------------------------------------------------------------------
    { "R0",                 GR_SET,             GR_0,                       0               },
    { "R1",                 GR_SET,             GR_1,                       1               },
    { "R2",                 GR_SET,             GR_2,                       2               },
    { "R3",                 GR_SET,             GR_3,                       3               },
    { "R4",                 GR_SET,             GR_4,                       4               },
    { "R5",                 GR_SET,             GR_5,                       5               },
    { "R6",                 GR_SET,             GR_6,                       6               },
    { "R7",                 GR_SET,             GR_7,                       7               },
    { "R8",                 GR_SET,             GR_8,                       8               },
    { "R9",                 GR_SET,             GR_9,                       9               },
    { "R10",                GR_SET,             GR_10,                      10              },
    { "R11",                GR_SET,             GR_11,                      11              },
    { "R12",                GR_SET,             GR_12,                      12              },
    { "R13",                GR_SET,             GR_13,                      13              },
    { "R14",                GR_SET,             GR_14,                      14              },
    { "R15",                GR_SET,             GR_15,                      15              },

    //--------------------------------------------------------------------------------------------------------
    // Segment registers.
    //
    //--------------------------------------------------------------------------------------------------------
    { "S0",                 SR_SET,             SR_0,                       0               },
    { "S1",                 SR_SET,             SR_1,                       1               },
    { "S2",                 SR_SET,             SR_2,                       2               },
    { "S3",                 SR_SET,             SR_3,                       3               },
    { "S4",                 SR_SET,             SR_4,                       4               },
    { "S5",                 SR_SET,             SR_5,                       5               },
    { "S6",                 SR_SET,             SR_6,                       6               },
    { "S7",                 SR_SET,             SR_7,                       7               },
    
    //--------------------------------------------------------------------------------------------------------
    // Control registers.
    //
    //--------------------------------------------------------------------------------------------------------
    { "C0",                 CR_SET,             CR_0,                       0               },
    { "C1",                 CR_SET,             CR_1,                       1               },
    { "C2",                 CR_SET,             CR_2,                       2               },
    { "C3",                 CR_SET,             CR_3,                       3               },
    { "C4",                 CR_SET,             CR_4,                       4               },
    { "C5",                 CR_SET,             CR_5,                       5               },
    { "C6",                 CR_SET,             CR_6,                       6               },
    { "C7",                 CR_SET,             CR_7,                       7               },
    { "C8",                 CR_SET,             CR_8,                       8               },
    { "C9",                 CR_SET,             CR_9,                       9               },
    { "C10",                CR_SET,             CR_10,                      10              },
    { "C11",                CR_SET,             CR_11,                      11              },
    { "C12",                CR_SET,             CR_12,                      12              },
    { "C13",                CR_SET,             CR_13,                      13              },
    { "C14",                CR_SET,             CR_14,                      14              },
    { "C15",                CR_SET,             CR_15,                      15              },
    { "C16",                CR_SET,             CR_16,                      16              },
    { "C17",                CR_SET,             CR_17,                      17              },
    { "C18",                CR_SET,             CR_18,                      18              },
    { "C19",                CR_SET,             CR_19,                      19              },
    { "C20",                CR_SET,             CR_20,                      20              },
    { "C21",                CR_SET,             CR_21,                      21              },
    { "C22",                CR_SET,             CR_22,                      22              },
    { "C23",                CR_SET,             CR_23,                      23              },
    { "C24",                CR_SET,             CR_24,                      24              },
    { "C25",                CR_SET,             CR_25,                      25              },
    { "C26",                CR_SET,             CR_26,                      26              },
    { "C27",                CR_SET,             CR_27,                      27              },
    { "C28",                CR_SET,             CR_28,                      28              },
    { "C29",                CR_SET,             CR_29,                      29              },
    { "C30",                CR_SET,             CR_30,                      30              },
    { "C31",                CR_SET,             CR_31,                      31              },
        
    //--------------------------------------------------------------------------------------------------------
    // Assembler mnemonics.
    //
    //--------------------------------------------------------------------------------------------------------
    { "LD",                 OP_CODE_SET,        OP_CODE_LD,                 0xC0000000      },
    { "LDB",                OP_CODE_SET,        OP_CODE_LDB,                0xC0000000      },
    { "LDH",                OP_CODE_SET,        OP_CODE_LDH,                0xC0000000      },
    { "LDW",                OP_CODE_SET,        OP_CODE_LDW,                0xC0000000      },
    { "LDR",                OP_CODE_SET,        OP_CODE_LDR,                0xD0000000      },
    { "LDA",                OP_CODE_SET,        OP_CODE_LDA,                0xC8000000      },
        
    { "ST",                 OP_CODE_SET,        OP_CODE_ST,                 0xC4200000      },
    { "STB",                OP_CODE_SET,        OP_CODE_STB,                0xC4200000      },
    { "STH",                OP_CODE_SET,        OP_CODE_STH,                0xC4200000      },
    { "STW",                OP_CODE_SET,        OP_CODE_STW,                0xC4200000      },
    { "STC",                OP_CODE_SET,        OP_CODE_STC,                0xD4000000      },
    { "STA",                OP_CODE_SET,        OP_CODE_STA,                0xCC200000      },
        
    { "ADD",                OP_CODE_SET,        OP_CODE_ADD,                0x40000000      },
    { "ADDB",               OP_CODE_SET,        OP_CODE_ADDB,               0x40000000      },
    { "ADDH",               OP_CODE_SET,        OP_CODE_ADDH,               0x40000000      },
    { "ADDW",               OP_CODE_SET,        OP_CODE_ADDW,               0x40000000      },
        
    { "ADC",                OP_CODE_SET,        OP_CODE_ADC,                0x44000000      },
    { "ADCB",               OP_CODE_SET,        OP_CODE_ADCB,               0x44000000      },
    { "ADCH",               OP_CODE_SET,        OP_CODE_ADCH,               0x44000000      },
    { "ADCW",               OP_CODE_SET,        OP_CODE_ADCW,               0x44000000      },
        
    { "SUB",                OP_CODE_SET,        OP_CODE_SUB,                0x48000000      },
    { "SUBB",               OP_CODE_SET,        OP_CODE_SUBB,               0x48000000      },
    { "SUBH",               OP_CODE_SET,        OP_CODE_SUBH,               0x48000000      },
    { "SUBW",               OP_CODE_SET,        OP_CODE_SUBW,               0x48000000      },
        
    { "SBC",                OP_CODE_SET,        OP_CODE_SBC,                0x4C000000      },
    { "SBCB",               OP_CODE_SET,        OP_CODE_SBCB,               0x4C000000      },
    { "SBCH",               OP_CODE_SET,        OP_CODE_SBCH,               0x4C000000      },
    { "SBCW",               OP_CODE_SET,        OP_CODE_SBCW,               0x4C000000      },
        
    { "AND",                OP_CODE_SET,        OP_CODE_AND,                0x50000000      },
    { "ANDB",               OP_CODE_SET,        OP_CODE_ANDB,               0x50000000      },
    { "ANDH",               OP_CODE_SET,        OP_CODE_ANDH,               0x50000000      },
    { "ANDW",               OP_CODE_SET,        OP_CODE_ANDW,               0x50000000      },
        
    { "OR" ,                OP_CODE_SET,        OP_CODE_OR,                 0x54000000      },
    { "ORB",                OP_CODE_SET,        OP_CODE_ORB,                0x54000000      },
    { "ORH",                OP_CODE_SET,        OP_CODE_ORH,                0x54000000      },
    { "ORW",                OP_CODE_SET,        OP_CODE_ORW,                0x54000000      },
        
    { "XOR" ,               OP_CODE_SET,        OP_CODE_XOR,                0x58000000      },
    { "XORB",               OP_CODE_SET,        OP_CODE_XORB,               0x58000000      },
    { "XORH",               OP_CODE_SET,        OP_CODE_XORH,               0x58000000      },
    { "XORW",               OP_CODE_SET,        OP_CODE_XORW,               0x58000000      },
        
    { "CMP" ,               OP_CODE_SET,        OP_CODE_CMP,                0x5C000000      },
    { "CMPB",               OP_CODE_SET,        OP_CODE_CMPB,               0x5C000000      },
    { "CMPH",               OP_CODE_SET,        OP_CODE_CMPH,               0x5C000000      },
    { "CMPW",               OP_CODE_SET,        OP_CODE_CMPW,               0x5C000000      },
        
    { "CMPU" ,              OP_CODE_SET,        OP_CODE_CMPU,               0x60000000      },
    { "CMPUB",              OP_CODE_SET,        OP_CODE_CMPUB,              0x60000000      },
    { "CMPUH",              OP_CODE_SET,        OP_CODE_CMPUH,              0x60000000      },
    { "CMPUW",              OP_CODE_SET,        OP_CODE_CMPUW,              0x60000000      },
    
    { "LSID" ,              OP_CODE_SET,        OP_CODE_LSID,               0x10000000      },
    { "EXTR" ,              OP_CODE_SET,        OP_CODE_EXTR,               0x14000000      },
    { "DEP",                OP_CODE_SET,        OP_CODE_DEP,                0x18000000      },
    { "DSR",                OP_CODE_SET,        OP_CODE_DSR,                0x1C000000      },
    { "SHLA",               OP_CODE_SET,        OP_CODE_SHLA,               0x20000000      },
    { "CMR",                OP_CODE_SET,        OP_CODE_CMR,                0x24000000      },
        
    { "LDIL",               OP_CODE_SET,        OP_CODE_LDIL,               0x04000000      },
    { "ADDIL",              OP_CODE_SET,        OP_CODE_ADDIL,              0x08000000      },
    { "LDO",                OP_CODE_SET,        OP_CODE_LDO,                0x0C000000      },
    
    { "B" ,                 OP_CODE_SET,        OP_CODE_B,                  0x80000000      },
    { "GATE",               OP_CODE_SET,        OP_CODE_GATE,               0x84000000      },
    { "BR",                 OP_CODE_SET,        OP_CODE_BR,                 0x88000000      },
    { "BV",                 OP_CODE_SET,        OP_CODE_BV,                 0x8C000000      },
    { "BE",                 OP_CODE_SET,        OP_CODE_BE,                 0x90000000      },
    { "BVE",                OP_CODE_SET,        OP_CODE_BVE,                0x94000000      },
    { "CBR",                OP_CODE_SET,        OP_CODE_CBR,                0x98000000      },
    { "CBRU",               OP_CODE_SET,        OP_CODE_CBRU,               0x9C000000      },
        
    { "MR",                 OP_CODE_SET,        OP_CODE_MR,                 0x28000000      },
    { "MST",                OP_CODE_SET,        OP_CODE_MST,                0x2C000000      },
    { "DS",                 OP_CODE_SET,        OP_CODE_DS,                 0x30000000      },
    { "LDPA",               OP_CODE_SET,        OP_CODE_LDPA,               0xE4000000      },
    { "PRB",                OP_CODE_SET,        OP_CODE_PRB,                0xE8000000      },
    { "ITLB",               OP_CODE_SET,        OP_CODE_ITLB,               0xEC000000      },
    { "PTLB",               OP_CODE_SET,        OP_CODE_PTLB,               0xF0000000      },
    { "PCA",                OP_CODE_SET,        OP_CODE_PCA,                0xF4000000      },
    { "DIAG",               OP_CODE_SET,        OP_CODE_DIAG,               0xF8000000      },
    { "RFI",                OP_CODE_SET,        OP_CODE_RFI,                0xFC000000      },
    { "BRK",                OP_CODE_SET,        OP_CODE_BRK,                0x00000000      },
        
    //--------------------------------------------------------------------------------------------------------
    // Synthetic instruction mnemonics.
    //
    //--------------------------------------------------------------------------------------------------------
    { "NOP",                OP_CODE_SET_S,      OP_CODE_S_NOP,              0               },
 
    //--------------------------------------------------------------------------------------------------------
    //
    //
    //--------------------------------------------------------------------------------------------------------
    { "IA-SEG",             PS_SET,             PS_IA_SEG,                  0               },
    { "IA-OFS",             PS_SET,             PS_IA_OFS,                  0               },
    { "ST-REG",             PS_SET,             PS_STATUS,                  0               },
    
    { "FD-IA-SEG",          FD_SET,             FD_IA_SEG,                  0               },
    { "FD-IA-OFS",          FD_SET,             FD_IA_OFS,                  0               },
    { "FD-INSTR",           FD_SET,             FD_INSTR,                   0               },
    { "FD-A",               FD_SET,             FD_A,                       0               },
    { "FD-B",               FD_SET,             FD_B,                       0               },
    { "FD-X",               FD_SET,             FD_X,                       0               },
    
    { "OF-IA-SEG",          OF_SET,             MA_IA_SEG,                  0               },
    { "OF-IA-OFS",          OF_SET,             MA_IA_OFS,                  0               },
    { "OF-INSTR",           OF_SET,             MA_INSTR,                   0               },
    { "OF-A",               OF_SET,             MA_A,                       0               },
    { "OF-B",               OF_SET,             MA_B,                       0               },
    { "OF-X",               OF_SET,             MA_X,                       0               },
    { "OF-S",               OF_SET,             MA_S,                       0               },
    
    { "IC-L1-STATE",        IC_L1_SET,          IC_L1_STATE,                0               },
    { "IC-L1-REQ",          IC_L1_SET,          IC_L1_REQ,                  0               },
    { "IC-L1-REQ-SEG",      IC_L1_SET,          IC_L1_REQ_SEG,              0               },
    { "IC-L1-REQ-OFS",      IC_L1_SET,          IC_L1_REQ_OFS,              0               },
    { "IC-L1-REQ-TAG",      IC_L1_SET,          IC_L1_REQ_TAG,              0               },
    { "IC-L1-REQ-LEN",      IC_L1_SET,          IC_L1_REQ_LEN,              0               },
    { "IC-L1-REQ-LAT",      IC_L1_SET,          IC_L1_LATENCY,              0               },
    { "IC-L1-SETS",         IC_L1_SET,          IC_L1_SETS,                 0               },
    { "IC-L1-ENTRIES",      IC_L1_SET,          IC_L1_BLOCK_ENTRIES,        0               },
    { "IC-L1-B-SIZE",       IC_L1_SET,          IC_L1_BLOCK_SIZE,           0               },
   
    { "DC-L1-STATE",        DC_L1_SET,          DC_L1_STATE,                0               },
    { "DC-L1-REQ",          DC_L1_SET,          DC_L1_REQ,                  0               },
    { "DC-L1-REQ-SEG",      DC_L1_SET,          DC_L1_REQ_SEG,              0               },
    { "DC-L1-REQ-OFS",      DC_L1_SET,          DC_L1_REQ_OFS,              0               },
    { "DC-L1-REQ-TAG",      DC_L1_SET,          DC_L1_REQ_TAG,              0               },
    { "DC-L1-REQ-LEN",      DC_L1_SET,          DC_L1_REQ_LEN,              0               },
    { "DC-L1-REQ-LAT",      DC_L1_SET,          DC_L1_LATENCY,              0               },
    { "DC-L1-SETS",         DC_L1_SET,          DC_L1_SETS,                 0               },
    { "DC-L1-ENTRIES",      DC_L1_SET,          DC_L1_BLOCK_ENTRIES,        0               },
    { "DC-L1-B-SIZE",       DC_L1_SET,          DC_L1_BLOCK_SIZE,           0               },
  
    { "UC-L2-STATE",        UC_L2_SET,          UC_L2_STATE,                0               },
    { "UC-L2-REQ",          UC_L2_SET,          UC_L2_REQ,                  0               },
    { "UC-L2-REQ-SEG",      UC_L2_SET,          UC_L2_REQ_SEG,              0               },
    { "UC-L2-REQ-OFS",      UC_L2_SET,          UC_L2_REQ_OFS,              0               },
    { "UC-L2-REQ-TAG",      UC_L2_SET,          UC_L2_REQ_TAG,              0               },
    { "UC-L2-REQ-LEN",      UC_L2_SET,          UC_L2_REQ_LEN,              0               },
    { "UC-L2-REQ-LAT",      UC_L2_SET,          UC_L2_LATENCY,              0               },
    { "UC-L2-SETS",         UC_L2_SET,          UC_L2_SETS,                 0               },
    { "UC-L2-ENTRIES",      UC_L2_SET,          UC_L2_BLOCK_ENTRIES,        0               },
    { "UC-L2-B-SIZE",       UC_L2_SET,          UC_L2_BLOCK_SIZE,           0               },
    
    { "ITLB-STATE",         ITLB_SET,           ITLB_STATE,                 0               },
    { "ITLB-REQ",           ITLB_SET,           ITLB_REQ,                   0               },
    { "ITLB-REQ-SEG",       ITLB_SET,           ITLB_REQ_SEG,               0               },
    { "ITLB-REQ-OFS",       ITLB_SET,           ITLB_REQ_OFS,               0               },
    
    { "DTLB-STATE",         DTLB_SET,           DTLB_STATE,                 0               },
    { "DTLB-REQ",           DTLB_SET,           DTLB_REQ,                   0               },
    { "DTLB-REQ-SEG",       DTLB_SET,           DTLB_REQ_SEG,               0               },
    { "DTLB-REQ-OFS",       DTLB_SET,           DTLB_REQ_OFS,               0               },
    
    //--------------------------------------------------------------------------------------------------------
    //
    //--------------------------------------------------------------------------------------------------------
    { "GR-SET",             REG_SET,            GR_SET,                     0               },
    { "GR",                 REG_SET,            GR_SET,                     0               },
    
    { "SR-SET",             REG_SET,            SR_SET,                     0               },
    { "SR",                 REG_SET,            SR_SET,                     0               },
    
    { "CR-SET",             REG_SET,            CR_SET,                     0               },
    { "CR",                 REG_SET,            CR_SET,                     0               },
    
    { "PS-SET",             REG_SET,            PS_SET,                     0               },
    { "PS",                 REG_SET,            PS_SET,                     0               },
   
    { "PR-SET",             REG_SET,            PR_SET,                     0               },
    { "PR",                 REG_SET,            PR_SET,                     0               },
    
    { "FD-SET",             REG_SET,            FD_SET,                     0               },
    { "PR",                 REG_SET,            FD_SET,                     0               },
    
    { "MA-SET",             REG_SET,            OF_SET,                     0               },
    { "PR",                 REG_SET,            OF_SET,                     0               },
    
    { "IC-L1-SET",          REG_SET,            IC_L1_SET,                  0               },
    { "ICL1",               REG_SET,            IC_L1_SET,                  0               },

    { "DC-L1-SET",          REG_SET,            DC_L1_SET,                  0               },
    { "DCL1",               REG_SET,            DC_L1_SET,                  0               },
    
    { "UC-L2-SET",          REG_SET,            UC_L2_SET,                  0               },
    { "UCl2",               REG_SET,            UC_L2_SET,                  0               },
    
    { "ITLB-SET",           REG_SET,            ITLB_SET,                   0               },
    { "ITRS",               REG_SET,            ITLB_SET,                   0               },
    
    { "DTLB-SET",           REG_SET,            DTLB_SET,                   0               },
    { "DTRS",               REG_SET,            DTLB_SET,                   0               },
    
    { "REG-SET-ALL",        REG_SET,            REG_SET_ALL,                0               },
    { "RS",                 REG_SET,            REG_SET_ALL,                0               },
    
    { "",                   SET_NIL,            TOK_NIL,                    0               }
};

const int   TOK_TAB_SIZE  = sizeof( tokTab ) / sizeof( *tokTab );




//------------------------------------------------------------------------------------------------------------
// The lokkup function. We just do a linear search for now.
//
//------------------------------------------------------------------------------------------------------------
int lookupToken( char *str ) {
    
    if (( strlen( str ) == 0 ) || ( strlen ( str ) > TOK_NAME_SIZE )) return( -1 );
    
    for ( int i = 0; i < TOK_TAB_SIZE; i++ ) {
        
        if ( strcmp( str, tokTab[ i ].name ) == 0 ) return( i );
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
uint8_t DrvTokenizer::setupTokenizer( char *lineBuf ) {
    
    strncpy( tokenLine, lineBuf, strlen( lineBuf ) + 1 );
    upshiftStr( tokenLine );
    
    currentLineLen          = (int) strlen( tokenLine );
    currentCharIndex        = 0;
    currentTokCharIndex     = 0;
    currentChar             = ' ';
    currentTokErr           = NO_ERR;
    
    return( 0 );
}

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
    
    int  tmpRes = 0;
    char tmpStr[ TOK_INPUT_LINE_SIZE ] = "";
    
    do {
        
        strcat( tmpStr, &currentChar );
        nextChar( );
        
    } while ( isxdigit( currentChar ) || ( currentChar == 'X' ) || ( currentChar == 'O' ));
    
    if ( sscanf( tmpStr, "%i", &tmpRes ) == 1 ) {
        
        currentToken.tokenId    = TOK_NUM;
        currentToken.val        = tmpRes * sign;
    }
    else
        // parserError((char *) "Invalid number" )
        ;
}

//------------------------------------------------------------------------------------------------------------
// "parseString" gets a string. We manage special characters inside the string with the "\" prefix. Right
// now, we do not use strings, so the function is perhaps for the future. We will just parse it, but record
// no result. One day, the entire simulator might use the lexer functions. Then we need it.
//
//------------------------------------------------------------------------------------------------------------
void DrvTokenizer::parseString( ) {

    char stringBuf[ TOK_INPUT_LINE_SIZE ] = "";

    nextChar( );
          
    while (( currentChar != EOS_CHAR ) && ( currentChar != '"' )) {

        if ( currentChar == '\\' ) {

            nextChar( );
            if ( currentChar != EOS_CHAR ) {

                if      ( currentChar == 'n' )  strcat( stringBuf, (char *) "\n" );
                else if ( currentChar == 't' )  strcat( stringBuf, (char *) "\t" );
                else if ( currentChar == '\\' ) strcat( stringBuf, (char *) "\\" );
                else                            strcat( stringBuf, &currentChar );
            }
            else
                // parserError((char *) "Expected a \" for the string" )
                ;
        }
        else strcat( stringBuf, &currentChar );

        nextChar( );
    }

    nextChar( );

    currentToken.tokenId    = TOK_STR;
    currentToken.val        = 0;
}

//------------------------------------------------------------------------------------------------------------
// "parseIdent" parses an identifier. It is a sequence of characters starting with an alpha character. We do
// not really have user defined identifiers, only reserved words. As a qualified constant also begins with
// a character, the parsing of an identifier also needs to manage the parsing of constants with a qualifier,
// such as "L%nnn".  We first check for these kind of qualifiers and if found hand over to parse a number.
//
//------------------------------------------------------------------------------------------------------------
void DrvTokenizer::parseIdent( ) {
    
    char identBuf[ TOK_INPUT_LINE_SIZE ] = "";
    
    if ( currentChar == 'L' ) {
        
        strcat( identBuf, &currentChar );
        nextChar( );
        
        if ( currentChar == '%' ) {
            
            strcat( identBuf, &currentChar );
            nextChar( );
            
            if ( isdigit( currentChar )) {
                
                parseNum( 1 );
                currentToken.val &= 0xFFFFFC00;
                return;
            }
            else
                // parserError((char *) "Invalid char in identifier" )
                ;
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
                currentToken.val &= 0x3FF;
                return;
            }
            else
                //parserError((char *) "Invalid char in identifier" )
                ;
        }
    }
    
    while ( isalnum( currentChar )) {
        
        strcat( identBuf, &currentChar );
        nextChar( );
    }
    
    int index = lookupToken( identBuf );
    
    if ( index == -1 ) {
        
        strcpy( currentToken.name, identBuf );
        currentToken.tokGrpId   = TOK_NIL;
        currentToken.tokenId    = TOK_IDENT;
        currentToken.val        = 0;
    }
    else currentToken = tokTab[ index ];
}

//------------------------------------------------------------------------------------------------------------
// "nextToken" is the entry point to the token business. It returns the next token from the input string.
//
//------------------------------------------------------------------------------------------------------------
void DrvTokenizer::nextToken( ) {

    currentToken.name[ 0 ]      = 0;
    currentToken.tokGrpId       = TOK_NIL;
    currentToken.tokenId        = TOK_EOS;
    currentToken.val            = 0;
    
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
        
        currentToken.tokenId = TOK_PERIOD;
        
        nextChar( );
        while ( isalnum( currentChar )) {
            
            strcat( currentToken.name, &currentChar );
            nextChar( );
        }
    }
    else if ( currentChar == '+' ) {
        
        currentToken.tokenId = TOK_PLUS;
        nextChar( );
    }
    else if ( currentChar == '-' ) {
        
        currentToken.tokenId = TOK_MINUS;
        nextChar( );
    }
    else if ( currentChar == '*' ) {
        
        currentToken.tokenId = TOK_MULT;
        nextChar( );
    }
    else if ( currentChar == '/' ) {
        
        currentToken.tokenId = TOK_DIV;
        nextChar( );
    }
    else if ( currentChar == '%' ) {
        
        currentToken.tokenId = TOK_MOD;
        nextChar( );
    }
    else if ( currentChar == '|' ) {
        
        currentToken.tokenId = TOK_OR;
        nextChar( );
    }
    else if ( currentChar == '^' ) {
        
        currentToken.tokenId = TOK_XOR;
        nextChar( );
    }
    else if ( currentChar == '~' ) {
        
        currentToken.tokenId = TOK_NEG;
        nextChar( );
    }
    else if ( currentChar == '(' ) {
        
        currentToken.tokenId = TOK_LPAREN;
        nextChar( );
    }
    else if ( currentChar == ')' ) {
        
        currentToken.tokenId = TOK_RPAREN;
        nextChar( );
    }
    else if ( currentChar == ',' ) {
        
        currentToken.tokenId = TOK_COMMA;
        nextChar( );
    }
    else if ( currentChar == EOS_CHAR ) {
        
        currentToken.name[ 0 ]  = 0;
        currentToken.tokenId    = TOK_EOS;
        currentToken.val        = 0;
    }
    else currentToken.tokenId = TOK_ERR;
}

#if 0


// ??? how to best deal with errors ?

//------------------------------------------------------------------------------------------------------------
// "parserError" is a little helper that prints out the error encountered. We will print the original
// input line, a caret marker where we found the error, and then return a false. Parsing errors typically
// result in aborting the parsing process. As this is a one line assembly, we do not need to be put effort
// into continuing reasonably with the parsing process.
//
//------------------------------------------------------------------------------------------------------------
bool parserError( char *errStr ) {
    
    fprintf( stdout, "%s\n", tokenLine );
    
    int i = 0;
    while ( i < currentTokCharIndex ) {
        
        fprintf( stdout, " " );
        i ++;
    }
    
    fprintf( stdout, "^\n" "%s\n", errStr );
    return( false );
}


#endif

