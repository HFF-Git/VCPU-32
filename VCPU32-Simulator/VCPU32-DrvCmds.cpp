//------------------------------------------------------------------------------------------------------------
//
// VCPU32 - A 32-bit CPU - Simulator Commands
//
//------------------------------------------------------------------------------------------------------------
// Welcome to the test driver commands.
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
#include "VCPU32-Core.h"

//------------------------------------------------------------------------------------------------------------
// Local name space. We try to keep utility functions local to the file.
//
//------------------------------------------------------------------------------------------------------------
namespace {

//------------------------------------------------------------------------------------------------------------
// The global token table. All reserved words are allocated in this table. Each entry has the token name,
// the token id, the token type id, i.e. its type, and a value associated with the token. The value allows
// for a constant token. The parser can directly use the value in expressions.
//
//------------------------------------------------------------------------------------------------------------
DrvToken const cmdTokTab[ ] = {
    
    //--------------------------------------------------------------------------------------------------------
    // General tokens.
    //
    //--------------------------------------------------------------------------------------------------------
    { .name = "NIL",                .typ = TYP_SYM,                 .tid = TOK_NIL,         .val = 0        },
   
    // ??? should true and false rather be a predefine vs. a resrved token ? YES
    { .name = "TRUE",               .typ = TYP_BOOL,                .tid = TOK_IDENT,       .val = 1        },
    { .name = "FALSE",              .typ = TYP_BOOL,                .tid = TOK_IDENT,       .val = 0        },
    
    { .name = "ALL",                .typ = TYP_SYM,                 .tid = TOK_ALL                          },
    { .name = "CPU",                .typ = TYP_SYM,                 .tid = TOK_CPU                          },
    { .name = "MEM",                .typ = TYP_SYM,                 .tid = TOK_MEM                          },
    { .name = "C",                  .typ = TYP_SYM,                 .tid = TOK_C                            },
    { .name = "D",                  .typ = TYP_SYM,                 .tid = TOK_D                            },
    { .name = "F",                  .typ = TYP_SYM,                 .tid = TOK_F                            },
    { .name = "I",                  .typ = TYP_SYM,                 .tid = TOK_I                            },
    { .name = "T",                  .typ = TYP_SYM,                 .tid = TOK_T                            },
    { .name = "U",                  .typ = TYP_SYM,                 .tid = TOK_U                            },
    
    { .name = "DEC",                .typ = TYP_SYM,                 .tid = TOK_DEC,         .val = 10       },
    { .name = "DECIMAL",            .typ = TYP_SYM,                 .tid = TOK_DEC,         .val = 10       },
    { .name = "HEX",                .typ = TYP_SYM,                 .tid = TOK_HEX,         .val = 16       },
    { .name = "OCT",                .typ = TYP_SYM,                 .tid = TOK_OCT,         .val = 8        },
    { .name = "OCTAL",              .typ = TYP_SYM,                 .tid = TOK_OCT,         .val = 8        },
    { .name = "CODE",               .typ = TYP_SYM,                 .tid = TOK_CODE                         },
    
    //--------------------------------------------------------------------------------------------------------
    // Command Line tokens.
    //
    //--------------------------------------------------------------------------------------------------------
    { .name = "ENV",                .typ = TYP_CMD,                 .tid = CMD_ENV                          },
    { .name = "EXIT",               .typ = TYP_CMD,                 .tid = CMD_EXIT                         },
    { .name = "E",                  .typ = TYP_CMD,                 .tid = CMD_EXIT                         },
    { .name = "HELP",               .typ = TYP_CMD,                 .tid = CMD_HELP                         },
    { .name = "?",                  .typ = TYP_CMD,                 .tid = CMD_HELP                         },
    { .name = "WHELP",              .typ = TYP_CMD,                 .tid = CMD_WHELP                        },
    
    { .name = "RESET",              .typ = TYP_CMD,                 .tid = CMD_RESET                        },
    { .name = "RUN",                .typ = TYP_CMD,                 .tid = CMD_RUN                          },
    { .name = "STEP",               .typ = TYP_CMD,                 .tid = CMD_STEP                         },
    { .name = "S",                  .typ = TYP_CMD,                 .tid = CMD_STEP                         },
    
    { .name = "DIS",                .typ = TYP_CMD,                 .tid = CMD_DIS_ASM                      },
    { .name = "ASM",                .typ = TYP_CMD,                 .tid = CMD_ASM                          },
    
    { .name = "XF",                 .typ = TYP_CMD,                 .tid = CMD_XF                           },
    
    { .name = "DR",                 .typ = TYP_CMD,                 .tid = CMD_DR                           },
    { .name = "MR",                 .typ = TYP_CMD,                 .tid = CMD_MR                           },
    
    { .name = "DA",                 .typ = TYP_CMD,                 .tid = CMD_DA                           },
    { .name = "MA",                 .typ = TYP_CMD,                 .tid = CMD_MA                           },
    { .name = "MAA",                .typ = TYP_CMD,                 .tid = CMD_MAA                          },
   
    { .name = "ITLB",               .typ = TYP_CMD,                 .tid = CMD_I_TLB                        },
    { .name = "DTLB",               .typ = TYP_CMD,                 .tid = CMD_D_TLB                        },
    { .name = "PTLB",               .typ = TYP_CMD,                 .tid = CMD_P_TLB                        },
    
    { .name = "DCA",                .typ = TYP_CMD,                 .tid = CMD_D_CACHE                      },
    { .name = "PCA",                .typ = TYP_CMD,                 .tid = CMD_P_CACHE                      },
    
    { .name = "HVA",                .typ = TYP_CMD,                 .tid = CMD_HASH_VA                      },
    
    { .name = "LOAD_MEM",           .typ = TYP_CMD,                 .tid = CMD_LMF                          },
    { .name = "SAVE_MEM",           .typ = TYP_CMD,                 .tid = CMD_SMF                          },
    
    //--------------------------------------------------------------------------------------------------------
    // Window command tokens.
    //
    //--------------------------------------------------------------------------------------------------------
    { .name = "WON",                .typ = TYP_CMD,                 .tid = CMD_WON                          },
    { .name = "WOFF",               .typ = TYP_CMD,                 .tid = CMD_WOFF                         },
    { .name = "WDEF",               .typ = TYP_CMD,                 .tid = CMD_WDEF                         },
    { .name = "WSE",                .typ = TYP_CMD,                 .tid = CMD_WSE                          },
    { .name = "WSD",                .typ = TYP_CMD,                 .tid = CMD_WSD                          },
    
    { .name = "PSE",                .typ = TYP_CMD,                 .tid = CMD_PSE                          },
    { .name = "PSD",                .typ = TYP_CMD,                 .tid = CMD_PSD                          },
    { .name = "PSR",                .typ = TYP_CMD,                 .tid = CMD_PSR                          },
    
    { .name = "SRE",                .typ = TYP_CMD,                 .tid = CMD_SRE                          },
    { .name = "SRD",                .typ = TYP_CMD,                 .tid = CMD_SRD                          },
    { .name = "SRR",                .typ = TYP_CMD,                 .tid = CMD_SRR                          },
    
    { .name = "PLE",                .typ = TYP_CMD,                 .tid = CMD_PLE                          },
    { .name = "PLD",                .typ = TYP_CMD,                 .tid = CMD_PLD                          },
    { .name = "PLR",                .typ = TYP_CMD,                 .tid = CMD_PLR                          },
    
    { .name = "SWE",                .typ = TYP_CMD,                 .tid = CMD_SWE                          },
    { .name = "SWD",                .typ = TYP_CMD,                 .tid = CMD_SWD                          },
    { .name = "SWR",                .typ = TYP_CMD,                 .tid = CMD_SWR                          },
    
    { .name = "CWL",                .typ = TYP_CMD,                 .tid = CMD_CWL                          },
    
    { .name = "WE",                 .typ = TYP_CMD,                 .tid = CMD_WE                           },
    { .name = "WD",                 .typ = TYP_CMD,                 .tid = CMD_WD                           },
    { .name = "WR",                 .typ = TYP_CMD,                 .tid = CMD_WR                           },
    { .name = "WF",                 .typ = TYP_CMD,                 .tid = CMD_WF                           },
    { .name = "WB",                 .typ = TYP_CMD,                 .tid = CMD_WB                           },
    { .name = "WH",                 .typ = TYP_CMD,                 .tid = CMD_WH                           },
    { .name = "WJ",                 .typ = TYP_CMD,                 .tid = CMD_WJ                           },
    { .name = "WL",                 .typ = TYP_CMD,                 .tid = CMD_WL                           },
    { .name = "WN",                 .typ = TYP_CMD,                 .tid = CMD_WN                           },
    { .name = "WK",                 .typ = TYP_CMD,                 .tid = CMD_WK                           },
    { .name = "WC",                 .typ = TYP_CMD,                 .tid = CMD_WC                           },
    { .name = "WS",                 .typ = TYP_CMD,                 .tid = CMD_WS                           },
    { .name = "WT",                 .typ = TYP_CMD,                 .tid = CMD_WT                           },
    { .name = "WX",                 .typ = TYP_CMD,                 .tid = CMD_WX                           },
    
    { .name = "PM",                 .typ = TYP_SYM,                 .tid = TOK_PM                           },
    { .name = "PC",                 .typ = TYP_SYM,                 .tid = TOK_PC                           },
    { .name = "IT",                 .typ = TYP_SYM,                 .tid = TOK_IT                           },
    { .name = "DT",                 .typ = TYP_SYM,                 .tid = TOK_DT                           },
    { .name = "IC",                 .typ = TYP_SYM,                 .tid = TOK_IC                           },
    { .name = "DC",                 .typ = TYP_SYM,                 .tid = TOK_DC                           },
    { .name = "UC",                 .typ = TYP_SYM,                 .tid = TOK_UC                           },
    { .name = "ICR",                .typ = TYP_SYM,                 .tid = TOK_ICR                          },
    { .name = "DCR",                .typ = TYP_SYM,                 .tid = TOK_DCR                          },
    { .name = "UCR",                .typ = TYP_SYM,                 .tid = TOK_UCR                          },
    { .name = "MCR",                .typ = TYP_SYM,                 .tid = TOK_MCR                          },
    { .name = "ITR",                .typ = TYP_SYM,                 .tid = TOK_ITR                          },
    { .name = "DTR",                .typ = TYP_SYM,                 .tid = TOK_DTR                          },
    { .name = "PCR",                .typ = TYP_SYM,                 .tid = TOK_PCR                          },
    { .name = "IOR",                .typ = TYP_SYM,                 .tid = TOK_IOR                          },
    { .name = "TX",                 .typ = TYP_SYM,                 .tid = TOK_TX                           },
    
    //--------------------------------------------------------------------------------------------------------
    // General registers.
    //
    //--------------------------------------------------------------------------------------------------------
    { .name = "R0",                 .typ = TYP_GREG,                .tid = GR_0,            .val =  0       },
    { .name = "R1",                 .typ = TYP_GREG,                .tid = GR_1,            .val =  1       },
    { .name = "R2",                 .typ = TYP_GREG,                .tid = GR_2,            .val =  2       },
    { .name = "R3",                 .typ = TYP_GREG,                .tid = GR_3,            .val =  3       },
    { .name = "R4",                 .typ = TYP_GREG,                .tid = GR_4,            .val =  4       },
    { .name = "R5",                 .typ = TYP_GREG,                .tid = GR_5,            .val =  5       },
    { .name = "R6",                 .typ = TYP_GREG,                .tid = GR_6,            .val =  6       },
    { .name = "R7",                 .typ = TYP_GREG,                .tid = GR_7,            .val =  7       },
    { .name = "R8",                 .typ = TYP_GREG,                .tid = GR_8,            .val =  8       },
    { .name = "R9",                 .typ = TYP_GREG,                .tid = GR_9,            .val =  9       },
    { .name = "R10",                .typ = TYP_GREG,                .tid = GR_10,           .val =  10      },
    { .name = "R11",                .typ = TYP_GREG,                .tid = GR_11,           .val =  11      },
    { .name = "R12",                .typ = TYP_GREG,                .tid = GR_12,           .val =  12      },
    { .name = "R13",                .typ = TYP_GREG,                .tid = GR_13,           .val =  13      },
    { .name = "R14",                .typ = TYP_GREG,                .tid = GR_14,           .val =  14      },
    { .name = "R15",                .typ = TYP_GREG,                .tid = GR_15,           .val =  15      },
    { .name = "GR",                 .typ = TYP_GREG,                .tid = GR_SET,          .val =  0       },
    
    //--------------------------------------------------------------------------------------------------------
    // Segment registers.
    //
    //--------------------------------------------------------------------------------------------------------
    { .name = "S0",                 .typ = TYP_SREG,                .tid = SR_0,            .val =  0       },
    { .name = "S1",                 .typ = TYP_SREG,                .tid = SR_1,            .val =  1       },
    { .name = "S2",                 .typ = TYP_SREG,                .tid = SR_2,            .val =  2       },
    { .name = "S3",                 .typ = TYP_SREG,                .tid = SR_3,            .val =  3       },
    { .name = "S4",                 .typ = TYP_SREG,                .tid = SR_4,            .val =  4       },
    { .name = "S5",                 .typ = TYP_SREG,                .tid = SR_5,            .val =  5       },
    { .name = "S6",                 .typ = TYP_SREG,                .tid = SR_6,            .val =  6       },
    { .name = "S7",                 .typ = TYP_SREG,                .tid = SR_7,            .val =  7       },
    { .name = "SR",                 .typ = TYP_SREG,                .tid = SR_SET,          .val =  0       },
    
    //--------------------------------------------------------------------------------------------------------
    // Control registers.
    //
    //--------------------------------------------------------------------------------------------------------
    { .name = "C0",                 .typ = TYP_CREG,                .tid = CR_0,            .val =  0       },
    { .name = "C1",                 .typ = TYP_CREG,                .tid = CR_1,            .val =  1       },
    { .name = "C2",                 .typ = TYP_CREG,                .tid = CR_2,            .val =  2       },
    { .name = "C3",                 .typ = TYP_CREG,                .tid = CR_3,            .val =  3       },
    { .name = "C4",                 .typ = TYP_CREG,                .tid = CR_4,            .val =  4       },
    { .name = "C5",                 .typ = TYP_CREG,                .tid = CR_5,            .val =  5       },
    { .name = "C6",                 .typ = TYP_CREG,                .tid = CR_6,            .val =  6       },
    { .name = "C7",                 .typ = TYP_CREG,                .tid = CR_7,            .val =  7       },
    { .name = "C8",                 .typ = TYP_CREG,                .tid = CR_8,            .val =  8       },
    { .name = "C9",                 .typ = TYP_CREG,                .tid = CR_9,            .val =  9       },
    { .name = "C10",                .typ = TYP_CREG,                .tid = CR_10,           .val =  10      },
    { .name = "C11",                .typ = TYP_CREG,                .tid = CR_11,           .val =  11      },
    { .name = "C12",                .typ = TYP_CREG,                .tid = CR_12,           .val =  12      },
    { .name = "C13",                .typ = TYP_CREG,                .tid = CR_13,           .val =  13      },
    { .name = "C14",                .typ = TYP_CREG,                .tid = CR_14,           .val =  14      },
    { .name = "C15",                .typ = TYP_CREG,                .tid = CR_15,           .val =  15      },
    { .name = "C16",                .typ = TYP_CREG,                .tid = CR_16,           .val =  16      },
    { .name = "C17",                .typ = TYP_CREG,                .tid = CR_17,           .val =  17      },
    { .name = "C18",                .typ = TYP_CREG,                .tid = CR_18,           .val =  18      },
    { .name = "C19",                .typ = TYP_CREG,                .tid = CR_19,           .val =  19      },
    { .name = "C20",                .typ = TYP_CREG,                .tid = CR_20,           .val =  20      },
    { .name = "C21",                .typ = TYP_CREG,                .tid = CR_21,           .val =  21      },
    { .name = "C22",                .typ = TYP_CREG,                .tid = CR_22,           .val =  22      },
    { .name = "C23",                .typ = TYP_CREG,                .tid = CR_23,           .val =  23      },
    { .name = "C24",                .typ = TYP_CREG,                .tid = CR_24,           .val =  24      },
    { .name = "C25",                .typ = TYP_CREG,                .tid = CR_25,           .val =  25      },
    { .name = "C26",                .typ = TYP_CREG,                .tid = CR_26,           .val =  26      },
    { .name = "C27",                .typ = TYP_CREG,                .tid = CR_27,           .val =  27      },
    { .name = "C28",                .typ = TYP_CREG,                .tid = CR_28,           .val =  28      },
    { .name = "C29",                .typ = TYP_CREG,                .tid = CR_29,           .val =  29      },
    { .name = "C30",                .typ = TYP_CREG,                .tid = CR_30,           .val =  30      },
    { .name = "C31",                .typ = TYP_CREG,                .tid = CR_31,           .val =  31      },
    { .name = "CR",                 .typ = TYP_CREG,                .tid = CR_SET,          .val =  0       },
    
    //--------------------------------------------------------------------------------------------------------
    // CPu Core register tokens.
    //
    //--------------------------------------------------------------------------------------------------------
    { .name = "FD_PSW0",            .typ = TYP_FD_PREG,         .tid = FD_PSW0,             .val = PSTAGE_REG_ID_PSW_0      },
    { .name = "FD_PSW1",            .typ = TYP_FD_PREG,         .tid = FD_PSW1,             .val = PSTAGE_REG_ID_PSW_1      },
    { .name = "FDR",                .typ = TYP_FD_PREG,         .tid = FD_SET,              .val = 0                        },
    
    { .name = "PSW0",               .typ = TYP_FD_PREG,         .tid = FD_PSW0,             .val = PSTAGE_REG_ID_PSW_0      },
    { .name = "PSW1",               .typ = TYP_FD_PREG,         .tid = FD_PSW1,             .val = PSTAGE_REG_ID_PSW_1      },
    { .name = "PS",                 .typ = TYP_FD_PREG,         .tid = FD_SET,              .val = 0                        },
  
    { .name = "MA_PSW0",            .typ = TYP_MA_PREG,         .tid = MA_PSW0,             .val = PSTAGE_REG_ID_PSW_0      },
    { .name = "MA_PSW1",            .typ = TYP_MA_PREG,         .tid = MA_PSW1,             .val = PSTAGE_REG_ID_PSW_0      },
    { .name = "MA_INSTR",           .typ = TYP_MA_PREG,         .tid = MA_INSTR,            .val = PSTAGE_REG_ID_INSTR      },
    { .name = "MA_A",               .typ = TYP_MA_PREG,         .tid = MA_A,                .val = PSTAGE_REG_ID_VAL_A      },
    { .name = "MA_B",               .typ = TYP_MA_PREG,         .tid = MA_B,                .val = PSTAGE_REG_ID_VAL_B      },
    { .name = "MA_X",               .typ = TYP_MA_PREG,         .tid = MA_X,                .val = PSTAGE_REG_ID_VAL_X      },
    { .name = "MA_S",               .typ = TYP_MA_PREG,         .tid = MA_S,                .val = PSTAGE_REG_ID_VAL_S      },
    { .name = "MAR",                .typ = TYP_MA_PREG,         .tid = MA_SET,              .val = 0                        },
    
    { .name = "EX_PSW0",            .typ = TYP_EX_PREG,         .tid = EX_PSW0,             .val = PSTAGE_REG_ID_PSW_0      },
    { .name = "EX_PSW1",            .typ = TYP_EX_PREG,         .tid = EX_PSW1,             .val = PSTAGE_REG_ID_PSW_1      },
    { .name = "EX_INSTR",           .typ = TYP_EX_PREG,         .tid = EX_INSTR,            .val = PSTAGE_REG_ID_INSTR      },
    { .name = "EX_A",               .typ = TYP_EX_PREG,         .tid = EX_A,                .val = PSTAGE_REG_ID_VAL_A      },
    { .name = "EX_B",               .typ = TYP_EX_PREG,         .tid = EX_B,                .val = PSTAGE_REG_ID_VAL_B      },
    { .name = "EX_X",               .typ = TYP_EX_PREG,         .tid = EX_X,                .val = PSTAGE_REG_ID_VAL_X      },
    { .name = "EX_S",               .typ = TYP_EX_PREG,         .tid = EX_S,                .val = PSTAGE_REG_ID_VAL_S      },
    { .name = "EXR",                .typ = TYP_EX_PREG,         .tid = EX_SET,              .val = 0                        },
    
    // ??? fix  all them to use regId in val field....
    // ??? are the name lists complete ?????
    
    { .name = "IC_L1_STATE",        .typ = TYP_IC_L1_REG,       .tid = IC_L1_STATE,         .val = MC_REG_STATE             },
    { .name = "IC_L1_REQ",          .typ = TYP_IC_L1_REG,       .tid = IC_L1_REQ,           .val = 1        },
    { .name = "IC_L1_REQ_SEG",      .typ = TYP_IC_L1_REG,       .tid = IC_L1_REQ_SEG,       .val = MC_REG_REQ_SEG           },
    { .name = "IC_L1_REQ_OFS",      .typ = TYP_IC_L1_REG,       .tid = IC_L1_REQ_OFS,       .val = MC_REG_REQ_OFS           },
    { .name = "IC_L1_REQ_TAG",      .typ = TYP_IC_L1_REG,       .tid = IC_L1_REQ_TAG,       .val = MC_REG_REQ_TAG           },
    { .name = "IC_L1_REQ_LEN",      .typ = TYP_IC_L1_REG,       .tid = IC_L1_REQ_LEN,       .val = MC_REG_REQ_LEN           },
    { .name = "IC_L1_REQ_LAT",      .typ = TYP_IC_L1_REG,       .tid = IC_L1_LATENCY,       .val = 6        },
    { .name = "IC_L1_SETS",         .typ = TYP_IC_L1_REG,       .tid = IC_L1_SETS,          .val = MC_REG_SETS              },
    { .name = "IC_L1_ENTRIES",      .typ = TYP_IC_L1_REG,       .tid = IC_L1_BLOCK_ENTRIES, .val = MC_REG_BLOCK_ENTRIES     },
    { .name = "IC_L1_B_SIZE",       .typ = TYP_IC_L1_REG,       .tid = IC_L1_BLOCK_SIZE,    .val = MC_REG_BLOCK_SIZE        },
    { .name = "ICL1",               .typ = TYP_IC_L1_REG,       .tid = IC_L1_SET,           .val = 0                        },
    
    { .name = "DC_L1_STATE",        .typ = TYP_DC_L1_REG,       .tid = DC_L1_STATE,         .val = MC_REG_STATE             },
    { .name = "DC_L1_REQ",          .typ = TYP_DC_L1_REG,       .tid = DC_L1_REQ,           .val = 1        },
    { .name = "DC_L1_REQ_SEG",      .typ = TYP_DC_L1_REG,       .tid = DC_L1_REQ_SEG,       .val = MC_REG_REQ_SEG           },
    { .name = "DC_L1_REQ_OFS",      .typ = TYP_DC_L1_REG,       .tid = DC_L1_REQ_OFS,       .val = MC_REG_REQ_OFS           },
    { .name = "DC_L1_REQ_TAG",      .typ = TYP_DC_L1_REG,       .tid = DC_L1_REQ_TAG,       .val = MC_REG_REQ_TAG           },
    { .name = "DC_L1_REQ_LEN",      .typ = TYP_DC_L1_REG,       .tid = DC_L1_REQ_LEN,       .val = MC_REG_REQ_LEN           },
    { .name = "DC_L1_REQ_LAT",      .typ = TYP_DC_L1_REG,       .tid = DC_L1_LATENCY,       .val = 6        },
    { .name = "DC_L1_SETS",         .typ = TYP_DC_L1_REG,       .tid = DC_L1_SETS,          .val = MC_REG_SETS              },
    { .name = "DC_L1_ENTRIES",      .typ = TYP_DC_L1_REG,       .tid = DC_L1_BLOCK_ENTRIES, .val = MC_REG_BLOCK_ENTRIES     },
    { .name = "DC_L1_B_SIZE",       .typ = TYP_DC_L1_REG,       .tid = DC_L1_BLOCK_SIZE,    .val = MC_REG_BLOCK_SIZE        },
    { .name = "DCL1",               .typ = TYP_DC_L1_REG,       .tid = DC_L1_SET,           .val = 0        },
    
    { .name = "UC_L2_STATE",        .typ = TYP_UC_L2_REG,       .tid = UC_L2_STATE,         .val = MC_REG_STATE             },
    { .name = "UC_L2_REQ",          .typ = TYP_UC_L2_REG,       .tid = UC_L2_REQ,           .val = 1        },
    { .name = "UC_L2_REQ_SEG",      .typ = TYP_UC_L2_REG,       .tid = UC_L2_REQ_SEG,       .val = MC_REG_REQ_SEG           },
    { .name = "UC_L2_REQ_OFS",      .typ = TYP_UC_L2_REG,       .tid = UC_L2_REQ_OFS,       .val = MC_REG_REQ_OFS           },
    { .name = "UC_L2_REQ_TAG",      .typ = TYP_UC_L2_REG,       .tid = UC_L2_REQ_TAG,       .val = MC_REG_REQ_TAG           },
    { .name = "UC_L2_REQ_LEN",      .typ = TYP_UC_L2_REG,       .tid = UC_L2_REQ_LEN,       .val = MC_REG_REQ_LEN           },
    { .name = "UC_L2_REQ_LAT",      .typ = TYP_UC_L2_REG,       .tid = UC_L2_LATENCY,       .val = 6        },
    { .name = "UC_L2_SETS",         .typ = TYP_UC_L2_REG,       .tid = UC_L2_SETS,          .val = MC_REG_SETS              },
    { .name = "UC_L2_ENTRIES",      .typ = TYP_UC_L2_REG,       .tid = UC_L2_BLOCK_ENTRIES, .val = MC_REG_BLOCK_ENTRIES     },
    { .name = "UC_L2_B_SIZE",       .typ = TYP_UC_L2_REG,       .tid = UC_L2_BLOCK_SIZE,    .val = MC_REG_BLOCK_SIZE        },
    { .name = "UCL2",               .typ = TYP_UC_L2_REG,       .tid = DC_L1_SET,           .val = 0        },
    
    { .name = "ITLB_STATE",         .typ = TYP_ITLB_REG,        .tid = ITLB_STATE,          .val = 0        },
    { .name = "ITLB_REQ",           .typ = TYP_ITLB_REG,        .tid = ITLB_REQ,            .val = 1        },
    { .name = "ITLB_REQ_SEG",       .typ = TYP_ITLB_REG,        .tid = ITLB_REQ_SEG,        .val = 2        },
    { .name = "ITLB_REQ_OFS",       .typ = TYP_ITLB_REG,        .tid = ITLB_REQ_OFS,        .val = 3        },
    { .name = "ITLBL1",             .typ = TYP_ITLB_REG,        .tid = ITLB_SET,            .val = 4        },
    
    { .name = "DTLB_STATE",         .typ = TYP_DTLB_REG,        .tid = DTLB_STATE,          .val = MC_REG_STATE             },
    { .name = "DTLB_REQ",           .typ = TYP_DTLB_REG,        .tid = DTLB_REQ,            .val = 1        },
    { .name = "DTLB_REQ_SEG",       .typ = TYP_DTLB_REG,        .tid = DTLB_REQ_SEG,        .val = 2        },
    { .name = "DTLB_REQ_OFS",       .typ = TYP_DTLB_REG,        .tid = DTLB_REQ_OFS,        .val = 3        },
    { .name = "DTLBL1",             .typ = TYP_DTLB_REG,        .tid = DTLB_SET,            .val = 4        },
    
    //--------------------------------------------------------------------------------------------------------
    // The last token to mark the list end.
    //
    //--------------------------------------------------------------------------------------------------------
    { .name = "",                   .typ = TYP_NIL,             .tid = TOK_LAST                             }
    
};

//------------------------------------------------------------------------------------------------------------
// A little helper function to upshift a string in place.
//
//------------------------------------------------------------------------------------------------------------
void upshiftStr( char *str ) {
    
    size_t len = strlen( str );
    
    if ( len > 0 ) {
        
        for ( size_t i = 0; i < len; i++ ) str[ i ] = (char) toupper((int) str[ i ] );
    }
}

//------------------------------------------------------------------------------------------------------------
// "setRadix" ensures that we passed in a valid radix value. The default is a decimal number.
//
//------------------------------------------------------------------------------------------------------------
int setRadix( int rdx ) {
    
    return((( rdx == 8 ) || ( rdx == 10 ) || ( rdx == 16 )) ? rdx : 10 );
}

//------------------------------------------------------------------------------------------------------------
// Print out an error message text with an optional argument.
//
// ??? over time all text errors in the command should go here...
//------------------------------------------------------------------------------------------------------------
ErrMsgId cmdErr( ErrMsgId errNum, char *argStr = nullptr ) {
    
    switch ( errNum ) {
            
        case ERR_NOT_IN_WIN_MODE:           fprintf( stdout, "Command only valid in Windows mode\n" ); break;
        case ERR_OPEN_EXEC_FILE:            fprintf( stdout, "Error while opening file: \"%s\"\n", argStr ); break;
        case ERR_EXPECTED_FILE_NAME:        fprintf( stdout, "Expected a file name\n" ); break;
        case ERR_INVALID_CMD:               fprintf( stdout, "Invalid command, use help or whelp\n"); break;
        case ERR_INVALID_WIN_STACK_ID:      fprintf( stdout, "Invalid window stack Id\n" ); break;
        case ERR_EXPECTED_STACK_ID:         fprintf( stdout, "Expected stack Id\n" ); break;
        case ERR_INVALID_WIN_ID:            fprintf( stdout, "Invalid window Id\n" ); break;
        case ERR_EXPECTED_WIN_ID:           fprintf( stdout, "Expected a window Id\n" ); break;
            
        case ERR_INVALID_REG_ID:            fprintf( stdout, "Invalid register Id\n" ); break;
        case ERR_INVALID_RADIX:             fprintf( stdout, "Invalid radix\n" ); break;
            
        case ERR_EXTRA_TOKEN_IN_STR:        fprintf( stdout, "Extra tokens in command line\n" ); break;
        case ERR_EXPECTED_LPAREN:           fprintf( stdout, "Expected a left paren\n" ); break;
        case ERR_EXPECTED_RPAREN:           fprintf( stdout, "Expected a right paren\n" ); break;
        case ERR_EXPECTED_COMMA:            fprintf( stdout, "Expected a comma\n" ); break;
            
        case ERR_INVALID_EXIT_VAL:          fprintf( stdout, "Invalid program exit code\n" ); break;
            
        case ERR_ENV_VALUE_EXPR:            fprintf( stdout, "Invalid expression for ENV variable\n" ); break;
            
        case ERR_WIN_TYPE_NOT_CONFIGURED:   fprintf( stdout, "Win object type not configured\n" ); break;
            
        case ERR_EXPECTED_NUMERIC:          fprintf( stdout, "Expected a numeric value\n" ); break;
        case ERR_EXPECTED_EXT_ADR:          fprintf( stdout, "Expected a virtual address\n" ); break;
            
        case ERR_EXPR_TYPE_MATCH:           fprintf( stdout, "Expression type mismatch\n" ); break;
        case ERR_EXPR_FACTOR:               fprintf( stdout, "Expression error: factor\n" ); break;
        case ERR_EXPECTED_GENERAL_REG:      fprintf( stdout, "Expression a general reg\n" ); break;
            
        case ERR_INVALID_ARG:               fprintf( stdout, "Invalid command argument\n" ); break;
        case ERR_EXPECTED_STEPS:            fprintf( stdout, "Expected nuber of steps/instr\n" ); break;
        case ERR_INVALID_STEP_OPTION:       fprintf( stdout, "Invalid steps/instr option\n" ); break;
            
        case ERR_EXPECTED_INSTR_VAL:        fprintf( stdout, "Expected the instruction value\n" ); break;
        case ERR_TOO_MANY_ARGS_CMD_LINE:    fprintf( stdout, "Too many args in command line\n" ); break;
            
        case ERR_EXPECTED_START_OFS:        fprintf( stdout, "Expected start offset\n" ); break;
        case ERR_EXPECTED_LEN:              fprintf( stdout, "Expected length argument\n" ); break;
        case ERR_OFS_LEN_LIMIT_EXCEEDED:    fprintf( stdout, "Offset/Length exceeds limit\n" ); break;
        case ERR_EXPECTED_OFS:              fprintf( stdout, "Expected an address\n" ); break;
            
            
        case ERR_INVALID_FMT_OPT:           fprintf( stdout, "Invalid format option\n" ); break;
        case ERR_EXPECTED_FMT_OPT:          fprintf( stdout, "Expected a format option\n" ); break;
        case ERR_INVALID_WIN_TYPE:          fprintf( stdout, "Invalid window type\n" ); break;
        case ERR_EXPECTED_WIN_TYPE:         fprintf( stdout, "Expected a window type\n" ); break;
        case ERR_OUT_OF_WINDOWS:            fprintf( stdout, "Cannot create more windows\n" ); break;
            
        case ERR_TLB_TYPE:                  fprintf( stdout, "Expected a TLB type\n" ); break;
        case ERR_TLB_INSERT_OP:             fprintf( stdout, "Insert in TLB operation error\n" ); break;
        case ERR_TLB_PURGE_OP:              fprintf( stdout, "Purge from TLB operation error\n" ); break;
        case ERR_TLB_ACC_DATA:              fprintf( stdout, "Invalid TLB insert access data\n" ); break;
        case ERR_TLB_ADR_DATA:              fprintf( stdout, "Invalid TLB insert address data\n" ); break;
        case ERR_TLB_NOT_CONFIGURED:        fprintf( stdout, "TLB type not configured\n" ); break;
            
        case ERR_CACHE_TYPE:                fprintf( stdout, "Expected a cache type\n" ); break;
        case ERR_CACHE_PURGE_OP:            fprintf( stdout, "Purge from cache operation error\n" ); break;
        case ERR_CACHE_NOT_CONFIGURED:      fprintf( stdout, "Cache type not configured\n" ); break;
            
        case ERR_UNEXPECTED_EOS:            fprintf( stdout, "Unexpectedd end of command line\n" ); break;
            
        default: {
            
            fprintf( stdout, "Error: %d", errNum );
            if ( argStr != nullptr ) fprintf( stdout, "%32s", argStr );
            fprintf( stdout, "/n" );
        }
    }
    
    return( errNum );
}

//------------------------------------------------------------------------------------------------------------
// Just list all commands that we have.
//
//------------------------------------------------------------------------------------------------------------
void displayHelp( ) {
    
    const char FMT_STR[ ] = "%-50s%s\n";
    
    fprintf( stdout, FMT_STR, "help",   "displays syntax and a short description" );
    fprintf( stdout, FMT_STR, "#",      "echoes the command input" );
    fprintf( stdout, FMT_STR, "e        [ <val> ]", "program exit" );
    fprintf( stdout, FMT_STR, "env      [ <var> [ , <val> ]]", "lists the env tab, a variable, sets a variable" );
    
    fprintf( stdout, FMT_STR, "xf       <filepath>", "execute commands from a file" );
    fprintf( stdout, FMT_STR, "lmf      <path> [ , <opt> ]", "loads memory from a file" );
    fprintf( stdout, FMT_STR, "smf      <path> <ofs> [ , <len> ]", "stores memory to a file" );
    
    fprintf( stdout, FMT_STR, "reset    ( CPU|MEM|STATS|ALL )", "resets the CPU" );
    fprintf( stdout, FMT_STR, "run",    "run the CPU" );
    fprintf( stdout, FMT_STR, "s        [ <num> ] [ , I|C ]", "single step for instruction or clock cycle" );
    
    fprintf( stdout, FMT_STR, "dr       [ <regSet>| <reg> ] [ , <fmt> ]", "display registers" );
    fprintf( stdout, FMT_STR, "mr       <reg> , <val>", "modify registers" );
    
    fprintf( stdout, FMT_STR, "da       <ofs> [ , <len> ] [ , <fmt> ]", "display memory" );
    fprintf( stdout, FMT_STR, "ma       <ofs> , <val>", "modify memory" );
    fprintf( stdout, FMT_STR, "maa      <ofs> , <asm-str>", "modify memory as code" );
    
    fprintf( stdout, FMT_STR, "dis      <instr-val>", "disassemble an instruction" );
    fprintf( stdout, FMT_STR, "asm      <instr-string>", "assemble an instruction" );
    fprintf( stdout, FMT_STR, "hva      <ext-adr>",  "returns the hash value function result" );
    
    
    // ??? fix the syntax...
    fprintf( stdout, FMT_STR, "dca      <I|D|U> \",\" [<index> <len>]", "display cache content" );
    fprintf( stdout, FMT_STR, "pca      <I|D|U> \",\" <index> [<F>]", "flushes and purges cache data" );
    
    fprintf( stdout, FMT_STR, "dtlb     <I|D> [<index> <len>]", "display TLB content" );
    fprintf( stdout, FMT_STR, "itlb     <I|D> <seg> <ofs> <argAcc> <argAdr>", "inserts an entry into the TLB" );
    fprintf( stdout, FMT_STR, "ptlb     <I|D> <seg> <ofs>", "purges an entry from the TLB" );
   
    fprintf( stdout, FMT_STR, "won",    "switches to windows mode" );
    fprintf( stdout, FMT_STR, "woff",   "switches to command line mode" );
    fprintf( stdout, FMT_STR, "wdef",   "reset the windows to their default values" );
    fprintf( stdout, FMT_STR, "wse",    "enable window stacks" );
    fprintf( stdout, FMT_STR, "wsd",    "disable window stacks" );
    fprintf( stdout, FMT_STR, "<win><cmd> [<args-list>]", "issue a window command, use whelp for details." );
    fprintf( stdout, "\n" );
    
}

//------------------------------------------------------------------------------------------------------------
// List the ehlp for widows command.
//
//------------------------------------------------------------------------------------------------------------
void displayWindowHelp( ) {
    
    const char FMT_STR[ ] = "%-32s%s\n";
    
    fprintf( stdout, "Windows help \n\n" );
    fprintf( stdout, "General Syntax for Win Commands: <win><cmd> [ args ]\n\n" );
    fprintf( stdout, "Windows:\n" );
    fprintf( stdout, FMT_STR, "PS",  "Program state window" );
    fprintf( stdout, FMT_STR, "SR",  "Special Register window" );
    fprintf( stdout, FMT_STR, "PL",  "CPU Pipeline Registers window" );
    fprintf( stdout, FMT_STR, "ST",  "Statistics window" );
    fprintf( stdout, FMT_STR, "IT",  "CPU Instruction TLB window" );
    fprintf( stdout, FMT_STR, "DT",  "CPU Data TLB window" );
    fprintf( stdout, FMT_STR, "IC",  "CPU Instruction Cache (L1) window" );
    fprintf( stdout, FMT_STR, "DC",  "CPU Data Cache (L1) window" );
    fprintf( stdout, FMT_STR, "UC",  "CPU Unified Cache (L2) window" );
    fprintf( stdout, FMT_STR, "PM",  "Physical Memory window" );
    fprintf( stdout, FMT_STR, "PC",  "Program Code Window" );
    fprintf( stdout, FMT_STR, "ICR", "CPU Instruction Cache (L1) controller registers" );
    fprintf( stdout, FMT_STR, "DCR", "CPU Data Cache (L1) controller registers" );
    fprintf( stdout, FMT_STR, "UCR", "CPU Unified Cache (L2) controller registers" );
    fprintf( stdout, FMT_STR, "MCR", "Physical Memory controller registers" );
    fprintf( stdout, FMT_STR, "ITR", "CPU Instruction TLB controller registers" );
    fprintf( stdout, FMT_STR, "DTR", "CPU Data TLB controller registers" );
    fprintf( stdout, FMT_STR, "PCR", "PDC Memory controller registers" );
    fprintf( stdout, FMT_STR, "IOR", "IO Memory controller registers" );
    fprintf( stdout, FMT_STR, "TX",  "Text Window" );
    fprintf( stdout, FMT_STR, "CW",  "Command Line window" );
    fprintf( stdout, FMT_STR, "W",   "User defined window" );
    fprintf( stdout, "\n" );
    
    fprintf( stdout, "Commands:\n" );
    fprintf( stdout, FMT_STR, "E [ <wNum> ]", "Enable window display" );
    fprintf( stdout, FMT_STR, "D [ <wNum> ]", "Disable window display" );
    fprintf( stdout, FMT_STR, "B <amt> [ , <wNum> ]", "Move backward by n items" );
    fprintf( stdout, FMT_STR, "F <amt> [ , <wNum> ]", "Move forward by n items" );
    fprintf( stdout, FMT_STR, "H <pos> [ , <wNum> ]", "Set window home position or set new home position" );
    fprintf( stdout, FMT_STR, "J <pos> [ , <wNum> ]", "Set window start to new position");
    fprintf( stdout, FMT_STR, "L <lines> [ , <wNum> ]", "Set window lines including banner line" );
    fprintf( stdout, FMT_STR, "R <radix> [ , <wNum> ]", "Set window radix ( OCT, DEC, HEX )" );
    fprintf( stdout, FMT_STR, "C <wNum>", "set the window <wNum> as current window" );
    fprintf( stdout, FMT_STR, "T <wNum>", "toggle through alternate window content" );
    fprintf( stdout, FMT_STR, "X <wNum>", "exchange current window with this window");
    fprintf( stdout, FMT_STR, "N <type> [ , <arg> ]", "New user defined window ( PM, PC, IT, DT, IC, ICR, DCR, MCR, TX )" );
    fprintf( stdout, FMT_STR, "K <wNumStart> [ , <wNumEnd> ]", "Removes a range of user defined window" );
    fprintf( stdout, FMT_STR, "S <stackNum> [ , <wNumStart> ] [ , <wNumEnd>]", "moves a range of user windows into stack <stackNum>" );
    fprintf( stdout, "\n" );
    
    fprintf( stdout, "Example: SRE       -> show special register window\n" );
    fprintf( stdout, "Example: WN PM     -> create a user defined physical memory window\n" );
    fprintf( stdout, "Example: WN 20, 11 -> scroll window 11 forward by 20 lines\n" );
    fprintf( stdout, "\n" );
}

//------------------------------------------------------------------------------------------------------------
// "promptYesNoCancel" is a simple function to print a prompt string with a decision question. The answer can
//  be yes/no or cancel. A positive result is a "yes" a negative result a "no", anything else a "cancel".
//
//------------------------------------------------------------------------------------------------------------
int promptYesNoCancel( char *promptStr ) {
    
    fprintf( stdout, "%s -> ", promptStr );
    fflush( stdout );
    
    char buf[ 8 ] = "";
    
    if ( fgets( buf, 8, stdin ) != nullptr ) {
        
        if      (( buf[ 0 ] == 'Y' ) ||  ( buf[ 0 ] == 'y' ))   return( 1 );
        else if (( buf[ 0 ] == 'N' ) ||  ( buf[ 0 ] == 'n' ))   return( -1 );
        else                                                    return( 0 );
    }
    else return( 0 );
}

//------------------------------------------------------------------------------------------------------------
// A little helper function to remove the comment part of a command line. We do the changes on the buffer
// passed in by just setting the ed of string at the position of the ";" comment indicator.
//
//------------------------------------------------------------------------------------------------------------
void removeComment( char *cmdBuf ) {
    
    if ( strlen( cmdBuf ) > 0 ) {
        
        char *tmp = strrchr( cmdBuf, '#' );
        if( tmp != nullptr ) *tmp = 0;
    }
}

}; // namespace


//************************************************************************************************************
//************************************************************************************************************
//
// Object methods.
//
//************************************************************************************************************
//************************************************************************************************************


//------------------------------------------------------------------------------------------------------------
// The object constructor. We just remember where globals are.
//
//------------------------------------------------------------------------------------------------------------
DrvCmds::DrvCmds( VCPU32Globals *glb ) {
    
    this -> glb = glb;
}

//------------------------------------------------------------------------------------------------------------
// Get the command interpreter ready.
//
// One day we will handle command line arguments....
//
//  -v           verbose
//  -i <path>    init file
//
// ??? to do ...
//------------------------------------------------------------------------------------------------------------
void DrvCmds::setupCmdInterpreter( int argc, const char *argv[ ] ) {
    

    while ( argc > 0 ) {
        
        argc --;
    }
    
    glb -> winDisplay  -> windowDefaults( );
    glb -> lineDisplay -> lineDefaults( );
}

//------------------------------------------------------------------------------------------------------------
// "commandLineError" is a little helper that prints out the error encountered. We will print a caret marker
// where we found the error, and then return a false. Parsing errors typically result in aborting the parsing
// process.
//
//------------------------------------------------------------------------------------------------------------
ErrMsgId DrvCmds::cmdLineError( ErrMsgId errNum, char *argStr ) {
    
    int     i           = 0;
    int     tokIndex    = glb -> tok -> tokCharIndex( );

    while (( i < tokIndex ) && ( i < strlen( glb -> tok -> tokenLineStr( )))) {
        
        fprintf( stdout, " " );
        i ++;
    }
    
    fprintf( stdout, "^\n" );
    return( cmdErr( errNum, argStr ));
}

//------------------------------------------------------------------------------------------------------------
// Token analysis helper functions.
//
//------------------------------------------------------------------------------------------------------------
ErrMsgId DrvCmds::checkEOS( ) {
    
    if ( glb -> tok -> isToken( TOK_EOS )) return( NO_ERR );
    else return( cmdLineError( ERR_EXTRA_TOKEN_IN_STR ));
}

ErrMsgId DrvCmds::acceptComma( ) {
    
    if ( glb -> tok -> isToken( TOK_COMMA )) {
        
        glb -> tok -> nextToken( );
        return( NO_ERR );
    }
    else return( cmdLineError( ERR_EXPECTED_COMMA ));
}

ErrMsgId DrvCmds::acceptLparen( ) {
    
    if ( glb -> tok -> isToken( TOK_LPAREN )) {
        
        glb -> tok -> nextToken( );
        return( NO_ERR );
    }
    else return( cmdLineError( ERR_EXPECTED_LPAREN ));
}

ErrMsgId DrvCmds::acceptRparen( ) {
    
    if ( glb -> tok -> isToken( TOK_RPAREN )) {
        
        glb -> tok -> nextToken( );
        return( NO_ERR );
    }
    else return( cmdLineError( ERR_EXPECTED_LPAREN ));
}

//------------------------------------------------------------------------------------------------------------
// Return the current command entered.
//
//------------------------------------------------------------------------------------------------------------
TokId DrvCmds::getCurrentCmd( ) {
    
    return( currentCmd );
}

//------------------------------------------------------------------------------------------------------------
// Our friendly welcome message with the actual program version. We also set some of the environment variables
// to an initial value. Especially string variables need to be set as they are not initialized from the
// environment variable table.
//
//------------------------------------------------------------------------------------------------------------
void DrvCmds::printWelcome( ) {
    
    glb -> env -> setEnvVar((char *) ENV_EXIT_CODE, 0 );
    
    if ( isatty( fileno( stdin ))) {
       
        fprintf( stdout, "VCPU-32 Simulator, Version: %s\n", glb -> env -> getEnvVarStr((char *) ENV_PROG_VERSION ));
        fprintf( stdout, "Git Branch: %s\n", glb -> env -> getEnvVarStr((char *) ENV_GIT_BRANCH ));
    }
}

//------------------------------------------------------------------------------------------------------------
// "promptCmdLine" lists out the prompt string. For now this is just a "->". As development goes on the prompt
// string will contain some more info about the current CPU state. The prompt is only printed when the input
// comes from a terminal and not an input file.
//
//------------------------------------------------------------------------------------------------------------
void DrvCmds::promptCmdLine( ) {
    
    if ( isatty( fileno( stdin ))) {
        
        if ( glb -> env -> getEnvVarBool((char *) ENV_SHOW_CMD_CNT ))
            fprintf( stdout, "(%i) ", glb -> env -> getEnvVarInt((char *) ENV_CMD_CNT ));
        
        fprintf( stdout, "->" );
        fflush( stdout );
    }
}

//------------------------------------------------------------------------------------------------------------
// "readCmdLine" reads in the command line. For a valid command line, the trailing carriage return and/or
// line feeds are removed and the first token is interpreted as a command. The function returns the command
// found, an invalid command or an empty command line status. We loop inside the routine until we receive
// a valid command line or an EOF.
//
// Warning: on a Mac, "fgets" does read in a string. On the terminal program configuration, the erase
// character needs to be set to NOT send a control-H to the input. The cursor keys are just echoed to the
// input line and I do not know a way to make them actually move around in the input line.
//
//------------------------------------------------------------------------------------------------------------
bool DrvCmds::readInputLine( char *cmdBuf ) {
    
    while ( true ) {
        
        fflush( stdout );
        
        if ( fgets( cmdBuf, CMD_LINE_BUF_SIZE, stdin ) != nullptr ) {
            
            cmdBuf[ strcspn( cmdBuf, "\r\n") ] = 0;
            
            removeComment( cmdBuf );
            
            if ( strlen( cmdBuf ) > 0 ) {
                
                glb -> env -> setEnvVar((char *) ENV_CMD_CNT,
                                          glb -> env -> getEnvVarInt((char *) ENV_CMD_CNT ) + 1 );
                return( true );
            }
            else return( false );
        }
        else if ( feof( stdin )) exit( glb -> env -> getEnvVarInt((char *) ENV_EXIT_CODE ));
    }
    
    return( false );
}

//------------------------------------------------------------------------------------------------------------
// "execCmdsFromFile" will open a text file and interpret each line as a command. This routine is used by the
// "EXEC-F" command and also as the handler for the program argument option to execute a file before entering
// the command loop.
//
//------------------------------------------------------------------------------------------------------------
uint8_t DrvCmds::execCmdsFromFile( char* fileName ) {
    
    char cmdLineBuf[ CMD_LINE_BUF_SIZE ] = "";
    
    if ( strlen( fileName ) > 0 ) {
        
        FILE *f = fopen( fileName, "r" );
        if ( f != nullptr ) {
            
            while ( ! feof( f )) {
                
                strcpy( cmdLineBuf, "" );
                fgets( cmdLineBuf, sizeof( cmdLineBuf ), f );
                cmdLineBuf[ strcspn( cmdLineBuf, "\r\n" ) ] = 0;
                
                if ( glb -> env -> getEnvVarBool((char *) ENV_ECHO_CMD_INPUT )) {
                    
                    fprintf( stdout, "%s\n", cmdLineBuf );
                }
                
                removeComment( cmdLineBuf );
                evalInputLine( cmdLineBuf );
            }
            
            return( NO_ERR );
        }
        else return( cmdErr( ERR_OPEN_EXEC_FILE, fileName ));
    }
    else return( cmdErr( ERR_EXPECTED_FILE_NAME  ));
}

//------------------------------------------------------------------------------------------------------------
// Invalid command handler.
//
//------------------------------------------------------------------------------------------------------------
uint8_t DrvCmds::invalidCmd( ) {
    
    glb -> env -> setEnvVar((char *) ENV_EXIT_CODE, -1 );
    return( cmdErr( ERR_INVALID_CMD ));
}

//------------------------------------------------------------------------------------------------------------
// Help command. With no arguments, a short help overview is printed. If there is an optional argument,
// specific help on the topic is given.
//
//------------------------------------------------------------------------------------------------------------
uint8_t DrvCmds::helpCmd( ) {
    
    displayHelp( );
    return( NO_ERR );
}

//------------------------------------------------------------------------------------------------------------
// Display the window specific help.
//
//------------------------------------------------------------------------------------------------------------
uint8_t DrvCmds::winHelpCmd( ) {
    
    displayWindowHelp( );
    return( NO_ERR );
}

//------------------------------------------------------------------------------------------------------------
// Exit command. We will exit with the environment variable value for the exit code or the argument value
// in the command. This will be quite useful for test script development.
//
// EXIT <code>
//------------------------------------------------------------------------------------------------------------
uint8_t DrvCmds::exitCmd( ) {
    
    DrvExpr rExpr;
    int  exitVal = 0;
    
    if ( glb -> tok -> tokId( ) == TOK_EOS ) {
        
        exitVal = glb -> env -> getEnvVarInt((char *) ENV_EXIT_CODE );
        exit(( exitVal > 255 ) ? 255 : exitVal );
        return( NO_ERR );
    }
    else {
        
        if (( glb -> eval -> parseExpr( &rExpr ) == NO_ERR ) && ( rExpr.typ == TYP_NUM )
            && ( rExpr.numVal >= 0 ) &&  ( rExpr.numVal <= 255 )) {
            
            exit( exitVal );
            return( NO_ERR );
        }
        else return( cmdLineError( ERR_INVALID_EXIT_VAL ));
    }
}

//------------------------------------------------------------------------------------------------------------
// ENV command. The test driver has a few global environment variables for data format, command count and so
// on. The ENV command list them all, one in particular and also modifies one if a value is specified. If the
// ENV variable dos not exist, it will be allocated with the type of the value. A value of the token NIL will
// remove a user defined variuable.
//
// ENV [ <envName> [ <val> ]]
//------------------------------------------------------------------------------------------------------------
uint8_t DrvCmds::envCmd( ) {
    
    uint8_t rStat = NO_ERR;
   
    if ( glb -> tok -> tokId( ) == TOK_EOS ) {
        
        glb -> env -> displayEnvTable( );
    }
    else if ( glb -> tok -> tokTyp( ) == TYP_IDENT ) {
        
        DrvExpr rExpr;
        char    envName[ 32 ]; // ??? fix
        
        strcpy( envName, glb -> tok -> tokStr( ));
        upshiftStr( envName );
        
        glb -> tok -> nextToken( );
        
        if ( glb -> tok -> tokId( ) == TOK_EOS ) {
            
            rStat = glb-> env -> displayEnvTableEntry( envName );
        }
        else {
            
            rStat = glb -> eval -> parseExpr( &rExpr );
            if ( rStat != NO_ERR ) return( cmdLineError( ERR_ENV_VALUE_EXPR ));
            
            if ( rExpr.typ == TYP_NUM ) {
                
                rStat = glb -> env -> setEnvVar( envName, rExpr.numVal );
            }
            else if ( rExpr.typ == TYP_BOOL ) {
                
                rStat = glb -> env -> setEnvVar( envName, rExpr.bVal );
            }
            else if ( rExpr.typ == TYP_STR ) {
                
                rStat = glb -> env -> setEnvVar( envName, rExpr.strVal );
            }
            else if ( rExpr.typ == TYP_EXT_ADR ) {
                
                rStat = glb -> env -> setEnvVar( envName, rExpr.seg, rExpr.ofs );
            }
            else if (( rExpr.typ == TYP_SYM ) && ( rExpr.tokId == TOK_NIL )) {
                
                rStat = glb -> env -> removeEnvVar( envName );
            }
        }
    }
    
    return( rStat );
}

//------------------------------------------------------------------------------------------------------------
// Execute commands from a file command. The actual work is done in the "execCmdsFromFile" routine.
//
// EXEC "<filename>"
//------------------------------------------------------------------------------------------------------------
uint8_t DrvCmds::execFileCmd( ) {
    
    if ( glb -> tok -> tokTyp( ) == TYP_STR ) {
        
        return( execCmdsFromFile( glb -> tok -> tokStr( )));
    }
    else return cmdErr( NO_ERR, (char *) "Expected a file path" );
}

//------------------------------------------------------------------------------------------------------------
// Load physical memory command.
//
// LMF <path>
//
// ??? when we load a memory image, is tha just a binary block at an address ? Purpose ?
// ??? this will perhaps be better done via load an image from the assembler.
//------------------------------------------------------------------------------------------------------------
uint8_t DrvCmds::loadPhysMemCmd( ) {
    
    fprintf( stdout, "The Load Physical Memory command... under construction\n" );
    return( NO_ERR );
}

//------------------------------------------------------------------------------------------------------------
// Save physical memory command.
//
// SMF <path>
//
// ??? when we save a memory image, how to load it back ? Purpose ?
//------------------------------------------------------------------------------------------------------------
uint8_t DrvCmds::savePhysMemCmd( ) {
    
    fprintf( stdout, "The Save Physical Memory command... under construction\n" );
    return( NO_ERR );
}

//------------------------------------------------------------------------------------------------------------
// Reset command.
//
// RESET ( CPU | MEM | STATS | ALL )
//
// ??? when and what statistics to also reset ?
// ??? what if thee is a unified cache outside the CPU ?
//------------------------------------------------------------------------------------------------------------
uint8_t DrvCmds::resetCmd( ) {
    
    if ( glb -> tok -> tokTyp( ) == TYP_SYM ) {
        
        switch( glb -> tok -> tokId( )) {
                
            case TOK_CPU: {
                
                glb -> cpu -> reset( );
                
            } break;
                
            case TOK_MEM: {
                
                glb -> cpu -> physMem -> reset( );
                
            } break;
                
            case TOK_STATS: {
              
            } break;
                
            case TOK_ALL: {
                
                glb -> cpu -> reset( );
                glb -> cpu -> physMem -> reset( );
              
            } break;
                
            default: fprintf( stdout, "Invalid option, use help\n" );
        }
    }
    else fprintf( stdout, "Invalid option, use help\n" );
    
    return( NO_ERR );
}

//------------------------------------------------------------------------------------------------------------
// Run command. The command will just run the CPU until a "halt" instruction is detected.
//
// RUN
//------------------------------------------------------------------------------------------------------------
uint8_t DrvCmds::runCmd( ) {
    
    fprintf( stdout, "RUN command to come ... \n" );
    
    // ??? idea: detect a "B 0" instruction. This is an endless loop to itself.
    // ??? drain the pipeline ?
    
    // ??? we could also have the trap handlers use this mechanism...
    
    return( NO_ERR );
}

//------------------------------------------------------------------------------------------------------------
// Step command. The command will execute one instruction. Default is one instruction. There is an ENV
// variable that will set the default to be a single clock step.
//
// STEP [ <stes> ] [ "," "I" | "C" ]
//------------------------------------------------------------------------------------------------------------
uint8_t DrvCmds::stepCmd( ) {
    
    DrvExpr  rExpr;
    uint32_t numOfSteps = 1;
    
    if ( glb -> tok -> tokTyp( ) == TYP_NUM ) {
        
        if (( glb -> eval -> parseExpr( &rExpr ) == NO_ERR ) && ( rExpr.typ == TYP_NUM )) {
            
            numOfSteps = rExpr.numVal;
        }
        else return( cmdLineError( ERR_EXPECTED_STEPS ));
    }
    
    if ( glb -> tok -> tokId( ) == TOK_COMMA ) {
        
        glb -> tok -> nextToken( );
        if      ( glb -> tok -> tokId( ) == TOK_I ) glb -> cpu -> instrStep( numOfSteps );
        else if ( glb -> tok -> tokId( ) == TOK_C ) glb -> cpu -> clockStep( numOfSteps );
        else                                 return( cmdLineError( ERR_INVALID_STEP_OPTION ));
    }
    
    if ( checkEOS( ) == NO_ERR ) {
        
        if ( glb -> env -> getEnvVarBool((char *) ENV_STEP_IN_CLOCKS ))   glb -> cpu -> clockStep( 1 );
        else                                                                glb -> cpu -> instrStep( 1 );
    }
    
    return( NO_ERR );
}

//------------------------------------------------------------------------------------------------------------
// Disassemble command.
//
// DIS <instr> [ "," fmt ]
//------------------------------------------------------------------------------------------------------------
uint8_t DrvCmds::disAssembleCmd( ) {
    
    DrvExpr     rExpr;
    uint32_t    instr   = 0;
    int         rdx     = glb -> env -> getEnvVarInt((char *) ENV_RDX_DEFAULT );
    
    if ((  glb -> eval -> parseExpr( &rExpr ) == NO_ERR ) && ( rExpr.typ == TYP_NUM )) {
        
        instr = rExpr.numVal;
        
        if ( glb -> tok -> tokId( ) == TOK_COMMA ) {
            
            glb -> tok -> nextToken( );
            
            if (( glb -> tok -> tokId( ) == TOK_HEX ) ||
                ( glb -> tok -> tokId( ) == TOK_OCT ) ||
                ( glb -> tok -> tokId( ) == TOK_DEC )) {
                
                rdx = glb -> tok -> tokVal( );
                
                glb -> tok -> nextToken( );
            }
            else if ( glb -> tok -> tokId( ) == TOK_EOS ) {
                
                rdx = glb -> env -> getEnvVarInt((char *) ENV_RDX_DEFAULT );
            }
            else return( cmdLineError( ERR_INVALID_FMT_OPT ));
        }
        
        if ( checkEOS( ) == NO_ERR ) {
            
            glb -> disAsm -> displayInstr( instr, rdx );
            fprintf( stdout, "\n" );
            return( NO_ERR );
        }
        return( cmdLineError( ERR_TOO_MANY_ARGS_CMD_LINE ));
    }
    else return( cmdLineError( ERR_EXPECTED_INSTR_VAL ));
}

//------------------------------------------------------------------------------------------------------------
// Assemble command. We enter the routine with the token past the command token.
//
// ASM <instr-str> [ fmt ]
//------------------------------------------------------------------------------------------------------------
uint8_t DrvCmds::assembleCmd( ) {
    
    int         rdx             = glb -> env -> getEnvVarInt((char *) ENV_RDX_DEFAULT );
    uint32_t    instr           = 0;
    char        asmStr[ 256 ]   = { 0 };
    
    if ( glb -> tok -> tokId( ) == TOK_STR ) {
        
        strncpy( asmStr, glb -> tok -> tokStr( ), sizeof( asmStr ));
        
        glb -> tok -> nextToken( );
        
        if (( glb -> tok -> tokId( ) == TOK_HEX ) ||
            ( glb -> tok -> tokId( ) == TOK_OCT ) ||
            ( glb -> tok -> tokId( ) == TOK_DEC )) {
            
            rdx = glb -> tok -> tokVal( );
        }
        else if ( glb -> tok -> tokId( ) == TOK_EOS ) {
            
            rdx = glb -> env -> getEnvVarInt((char *) ENV_RDX_DEFAULT );
        }
        else return( cmdLineError( ERR_INVALID_FMT_OPT ));
    }
    else return( cmdLineError( ERR_INVALID_ARG ));
    
    if ( glb -> oneLineAsm -> parseAsmLine( asmStr, &instr )) {
        
        glb -> lineDisplay -> displayWord( instr, rdx );
        fprintf( stdout, "\n" );
    }
    
    return( NO_ERR );
}

//------------------------------------------------------------------------------------------------------------
// Display register command. This is a rather versatile command, which displays register set, register and
// all of them in one format.
//
// DR [ <regSet>|<reg> ] [ "," <fmt> ]
//
// ??? PSTATE regs and FD Stage Regs are the same ?????
//------------------------------------------------------------------------------------------------------------
uint8_t DrvCmds::displayRegCmd( ) {
    
    int     rdx         = glb -> env -> getEnvVarInt((char *) ENV_RDX_DEFAULT );
    TypeId  regSetId    = TYP_GREG;
    TokId   regId       = GR_SET;
    int     regNum      = 0;
    
    if ( glb -> tok -> tokId( ) != TOK_EOS ) {
        
        if (( glb -> tok -> tokTyp( ) == TYP_GREG )        ||
            ( glb -> tok -> tokTyp( ) == TYP_SREG )        ||
            ( glb -> tok -> tokTyp( ) == TYP_CREG )        ||
            ( glb -> tok -> tokTyp( ) == TYP_PSTATE_PREG ) ||
            ( glb -> tok -> tokTyp( ) == TYP_FD_PREG )     ||
            ( glb -> tok -> tokTyp( ) == TYP_MA_PREG )     ||
            ( glb -> tok -> tokTyp( ) == TYP_EX_PREG )     ||
            ( glb -> tok -> tokTyp( ) == TYP_IC_L1_REG )   ||
            ( glb -> tok -> tokTyp( ) == TYP_DC_L1_REG )   ||
            ( glb -> tok -> tokTyp( ) == TYP_UC_L2_REG )   ||
            ( glb -> tok -> tokTyp( ) == TYP_ITLB_REG )    ||
            ( glb -> tok -> tokTyp( ) == TYP_DTLB_REG )) {
            
            regSetId    = glb -> tok -> tokTyp( );
            regId       = glb -> tok -> tokId( );
            regNum      = glb -> tok -> tokVal( );
        }
        else {
            
            fprintf( stdout, "Invalid register or register set\n" );
            return ( NO_ERR );
        }
   
        if ( glb -> tok -> tokId( ) == TOK_COMMA ) {
            
            glb -> tok -> nextToken( );
            
            if (( glb -> tok -> tokId( ) == TOK_HEX ) ||
                ( glb -> tok -> tokId( ) == TOK_OCT ) ||
                ( glb -> tok -> tokId( ) == TOK_DEC )) {
                
                rdx = glb -> tok -> tokVal( );
            }
            else if ( glb -> tok -> tokId( ) == TOK_EOS ) {
                
                rdx = glb -> env -> getEnvVarInt((char *) ENV_RDX_DEFAULT );
            }
            else return( cmdLineError( ERR_INVALID_FMT_OPT ));
        }
    }
    
    switch( regSetId ) {
            
        case TYP_GREG: {
            
            if ( regId == GR_SET ) glb -> lineDisplay -> displayGeneralRegSet( rdx );
            else glb -> lineDisplay -> displayWord( glb -> cpu -> getReg( RC_GEN_REG_SET, regNum ), rdx );
        
        } break;
            
        case TYP_SREG: {
            
            if ( regId == SR_SET ) glb -> lineDisplay -> displaySegmentRegSet( rdx );
            else glb -> lineDisplay -> displayWord( glb -> cpu -> getReg( RC_SEG_REG_SET, regNum ), rdx );
            
        } break;
            
        case TYP_CREG: {
            
            if ( regId == CR_SET ) glb -> lineDisplay -> displayControlRegSet( rdx );
            else glb -> lineDisplay -> displayWord( glb -> cpu -> getReg( RC_CTRL_REG_SET, regNum ), rdx );
            
        } break;
            
        case TYP_IC_L1_REG: {
            
            if ( regId == IC_L1_SET ) glb -> lineDisplay -> displayMemObjRegSet( glb -> cpu -> iCacheL1, rdx );
            else glb -> lineDisplay -> displayWord( glb -> cpu -> getReg( RC_IC_L1_OBJ, regNum ), rdx );
            
        } break;
            
        case TYP_DC_L1_REG: {
            
            if ( regId == DC_L1_SET ) glb -> lineDisplay -> displayMemObjRegSet( glb -> cpu -> dCacheL1, rdx );
            else glb -> lineDisplay -> displayWord( glb -> cpu -> getReg( RC_DC_L1_OBJ, regNum ), rdx );
            
        } break;
            
        case TYP_UC_L2_REG: {
            
            if ( glb -> cpu -> uCacheL2 != nullptr ) {
                
                if ( regId == UC_L2_SET ) glb -> lineDisplay -> displayMemObjRegSet( glb -> cpu -> uCacheL2, rdx );
                else glb -> lineDisplay -> displayWord( glb -> cpu -> getReg( RC_UC_L2_OBJ, regNum ), rdx );
            }
            else fprintf( stdout, "L2 cache not configured \n" );
            
        } break;
            
        case TYP_ITLB_REG: {
            
            if ( regId == ITLB_SET ) glb -> lineDisplay -> displayTlbObjRegSet( glb -> cpu -> iTlb, rdx );
            else glb -> lineDisplay -> displayWord( glb -> cpu -> getReg( RC_ITLB_OBJ, regNum ), rdx );
            
        } break;
            
        case TYP_DTLB_REG: {
            
            if ( regId == DTLB_SET ) glb -> lineDisplay -> displayTlbObjRegSet( glb -> cpu -> dTlb, rdx );
            else glb -> lineDisplay -> displayWord( glb -> cpu -> getReg( RC_DTLB_OBJ, regNum ), rdx );
            
        } break;
            
        case TYP_FD_PREG: {
            
            if ( regId == FD_SET ) glb -> lineDisplay -> displayPlIFetchDecodeRegSet( rdx );
            else glb -> lineDisplay -> displayWord( glb -> cpu -> getReg( RC_FD_PSTAGE, regNum ), rdx );
            
        } break;
            
        case TYP_MA_PREG: {
            
            if ( regId == MA_SET ) glb -> lineDisplay -> displayPlMemoryAccessRegSet( rdx );
            else glb -> lineDisplay -> displayWord( glb -> cpu -> getReg( RC_MA_PSTAGE, regNum ), rdx );
            
        } break;
            
        case TYP_EX_PREG: {
            
            if ( regId == EX_SET ) glb -> lineDisplay -> displayPlExecuteRegSet( rdx );
            else glb -> lineDisplay -> displayWord( glb -> cpu -> getReg( RC_EX_PSTAGE, regNum ), rdx );
            
        } break;
            
        default: ;
    }

    fprintf( stdout, "\n" );
    return( NO_ERR );
}

//------------------------------------------------------------------------------------------------------------
// Modify register command. This command modifies a register within a register set.
//
// MR <reg> <val>
//------------------------------------------------------------------------------------------------------------
uint8_t DrvCmds::modifyRegCmd( ) {
    
    TypeId      regSetId    = TYP_GREG;
    TokId       regId       = TOK_NIL;
    int         regNum      = 0;
    uint32_t    val         = 0;
    DrvExpr     rExpr;
    
    if (( glb -> tok -> tokTyp( ) == TYP_GREG )        ||
        ( glb -> tok -> tokTyp( ) == TYP_SREG )        ||
        ( glb -> tok -> tokTyp( ) == TYP_CREG )        ||
        ( glb -> tok -> tokTyp( ) == TYP_PSTATE_PREG ) ||
        ( glb -> tok -> tokTyp( ) == TYP_FD_PREG )     ||
        ( glb -> tok -> tokTyp( ) == TYP_MA_PREG )     ||
        ( glb -> tok -> tokTyp( ) == TYP_EX_PREG )     ||
        ( glb -> tok -> tokTyp( ) == TYP_IC_L1_REG )   ||
        ( glb -> tok -> tokTyp( ) == TYP_DC_L1_REG )   ||
        ( glb -> tok -> tokTyp( ) == TYP_UC_L2_REG )   ||
        ( glb -> tok -> tokTyp( ) == TYP_ITLB_REG )    ||
        ( glb -> tok -> tokTyp( ) == TYP_DTLB_REG )) {
        
        regSetId    = glb -> tok -> tokTyp( );
        regId       = glb -> tok -> tokId( );
        regNum      = glb -> tok -> tokVal( );
        glb -> tok -> nextToken( );
    }
    else return( cmdLineError( ERR_INVALID_REG_ID ));
    
    if ( glb -> tok -> tokId( ) == TOK_EOS ) {
        
        fprintf( stdout, "Expected a value\n" );
        return( NO_ERR );
    }
    
    if ((  glb -> eval -> parseExpr( &rExpr ) == NO_ERR ) && ( rExpr.typ == TYP_NUM )) val = rExpr.numVal;
    else return( cmdLineError( ERR_INVALID_NUM ));
    
    switch( regSetId ) {
            
        case TYP_GREG:          glb -> cpu -> setReg( RC_GEN_REG_SET, regNum, val );    break;
        case TYP_SREG:          glb -> cpu -> setReg( RC_SEG_REG_SET, regNum, val );    break;
        case TYP_CREG:          glb -> cpu -> setReg( RC_CTRL_REG_SET, regNum, val );   break;
        case TYP_FD_PREG:       glb -> cpu -> setReg( RC_FD_PSTAGE, regNum, val );      break;
        case TYP_MA_PREG:       glb -> cpu -> setReg( RC_MA_PSTAGE, regNum, val );      break;
        case TYP_EX_PREG:       glb -> cpu -> setReg( RC_EX_PSTAGE, regNum, val );      break;
        case TYP_IC_L1_REG:     glb -> cpu -> setReg( RC_IC_L1_OBJ, regNum, val );      break;
        case TYP_DC_L1_REG:     glb -> cpu -> setReg( RC_DC_L1_OBJ, regNum, val );      break;
        case TYP_UC_L2_REG:     glb -> cpu -> setReg( RC_UC_L2_OBJ, regNum, val );      break;
        case TYP_ITLB_REG:      glb -> cpu -> setReg( RC_ITLB_OBJ, regNum, val );       break;
        case TYP_DTLB_REG:      glb -> cpu -> setReg( RC_DTLB_OBJ, regNum, val );       break;
            
        default:  fprintf( stdout, "Invalid Reg Set for operation\n" );
    }
    
    return( NO_ERR );
}

//------------------------------------------------------------------------------------------------------------
// Hash virtual address command. The TLB is indexed by a hash function, which we can test with this command.
// We will use the iTlb hash function for this command.
//
// HVA <seg>.<ofs>
//------------------------------------------------------------------------------------------------------------
uint8_t DrvCmds::hashVACmd( ) {
    
    DrvExpr rExpr;
  
    if ((  glb -> eval -> parseExpr( &rExpr ) == NO_ERR ) && ( rExpr.typ == TYP_EXT_ADR )) {
        
        fprintf( stdout, "%i\n", glb -> cpu ->iTlb ->hashAdr( rExpr.seg, rExpr.ofs ));
        return( NO_ERR );
    }
    else return( cmdLineError( ERR_EXPECTED_EXT_ADR ));
}

//------------------------------------------------------------------------------------------------------------
// Display TLB entries command.
//
// DTLB (D|I|U) [ <index> ] [ "," <len> ] [ "," <fmt> ] - if no index, list all entries ? practical ?
//------------------------------------------------------------------------------------------------------------
uint8_t DrvCmds::displayTLBCmd( ) {
 
    uint32_t    index       = 0;
    uint32_t    len         = 0;
    uint32_t    tlbSize     = 0;
    TokId       tlbTypeId   = TOK_I;
    int         rdx         = glb -> env -> getEnvVarInt((char *) ENV_RDX_DEFAULT );
    
    if ( glb -> tok -> tokId( ) == TOK_I ) {
        
        tlbSize     = glb -> cpu -> iTlb -> getTlbSize( );
        tlbTypeId   = TOK_I;
        glb -> tok -> nextToken( );
    }
    else if ( glb -> tok -> tokId( ) == TOK_D ) {
        
        tlbSize     = glb -> cpu -> dTlb -> getTlbSize( );
        tlbTypeId   = TOK_D;
        glb -> tok -> nextToken( );
    }
    else return( cmdLineError( ERR_TLB_TYPE ));
    
    if ( acceptComma( ) != NO_ERR ) return( ERR_EXPECTED_COMMA );
    
    if ( glb -> tok -> tokId( ) == TOK_COMMA ) {
        
        index = 0;
        glb -> tok -> nextToken( );
    }
    else {
        
        DrvExpr rExpr;
        
        if (  glb -> eval -> parseExpr( &rExpr ) == NO_ERR ) {
            
            index = rExpr.numVal;
            
            if ( glb -> tok -> tokId( ) == TOK_COMMA ) glb -> tok -> nextToken( );
        }
        else {
            
            printf( "Expected the start offset\n" );
            return( NO_ERR );
        }
    }
    
    if ( glb -> tok -> tokId( ) == TOK_COMMA ) {
        
        len = 1;
        glb -> tok -> nextToken( );
    }
    else {
        
        DrvExpr rExpr;
        
        if (  glb -> eval -> parseExpr( &rExpr ) == NO_ERR ) {
            
            len = rExpr.numVal;
            if ( glb -> tok -> tokId( ) == TOK_COMMA ) glb -> tok -> nextToken( );
        }
        else {
            
            printf( "Expected the start offset\n" );
            return( NO_ERR );
        }
    }
    
    if ( glb -> tok -> tokId( ) == TOK_COMMA ) {
        
        glb -> tok -> nextToken( );
        
        if (( glb -> tok -> tokId( ) == TOK_HEX ) ||
            ( glb -> tok -> tokId( ) == TOK_OCT ) ||
            ( glb -> tok -> tokId( ) == TOK_DEC )) {
            
            rdx = glb -> tok -> tokVal( );
        }
        else if ( glb -> tok -> tokId( ) == TOK_EOS ) {
            
            rdx = glb -> env -> getEnvVarInt((char *) ENV_RDX_DEFAULT );
        }
        else return( cmdLineError( ERR_INVALID_FMT_OPT ));
    }
    
    if (( index > tlbSize ) || ( index + len > tlbSize )) {
        
        fprintf( stdout, "Index / Len exceed TLB size\n" );
        return( NO_ERR );
    }
    
    if (( index == 0 ) && ( len == 0 )) len = tlbSize;
    
    if      ( tlbTypeId == TOK_I ) glb -> lineDisplay -> displayTlbEntries( glb -> cpu -> iTlb, index, len, rdx );
    else if ( tlbTypeId == TOK_D ) glb -> lineDisplay -> displayTlbEntries( glb -> cpu -> dTlb, index, len, rdx );
    
    fprintf( stdout, "\n" );
    return( NO_ERR );
}

//------------------------------------------------------------------------------------------------------------
// Purge from TLB command.
//
// P-TLB <I|D|U> <extAdr>
//------------------------------------------------------------------------------------------------------------
uint8_t DrvCmds::purgeTLBCmd( ) {
    
    DrvExpr     rExpr;
    uint32_t    tlbSize     = 0;
    TokId       tlbTypeId   = TOK_I;
    
    if ( glb -> tok -> tokId( ) == TOK_I ) {
        
        tlbSize     = glb -> cpu -> iTlb -> getTlbSize( );
        tlbTypeId   = TOK_I;
        glb -> tok -> nextToken( );
    }
    else if ( glb -> tok -> tokId( ) == TOK_D ) {
        
        tlbSize     = glb -> cpu -> dTlb -> getTlbSize( );
        tlbTypeId   = TOK_D;
        glb -> tok -> nextToken( );
    }
    else return( cmdLineError( ERR_TLB_TYPE ));
    
    if ((  glb -> eval -> parseExpr( &rExpr ) == NO_ERR ) && ( rExpr.typ == TYP_EXT_ADR )) {
        
        CpuTlb *tlbPtr = ( tlbTypeId == TOK_I ) ? glb -> cpu -> iTlb : glb -> cpu -> dTlb;
        if ( tlbPtr -> purgeTlbEntryData( rExpr.seg, rExpr.ofs )) return( NO_ERR );
        else                                                      return( cmdLineError( ERR_TLB_PURGE_OP ));
    }
    else return( cmdLineError( ERR_EXPECTED_EXT_ADR ));
}

//------------------------------------------------------------------------------------------------------------
// Insert into TLB command.
//
// I-TLB <D|I|U> <extAdr> <arg-acc> <arg-adr>
//------------------------------------------------------------------------------------------------------------
uint8_t DrvCmds::insertTLBCmd( ) {
    
    DrvExpr     rExpr;
    uint32_t    tlbSize         = 0;
    TokId       tlbTypeId       = TOK_I;
    uint32_t    seg             = 0;
    uint32_t    ofs             = 0;
    uint32_t    argAcc          = 0;
    uint32_t    argAdr          = 0;
    
    if ( glb -> tok -> tokId( ) == TOK_I ) {
        
        tlbSize     = glb -> cpu -> iTlb -> getTlbSize( );
        tlbTypeId   = TOK_I;
        glb -> tok -> nextToken( );
    }
    else if ( glb -> tok -> tokId( ) == TOK_D ) {
        
        tlbSize     = glb -> cpu -> dTlb -> getTlbSize( );
        tlbTypeId   = TOK_D;
        glb -> tok -> nextToken( );
    }
    else return( cmdLineError( ERR_TLB_TYPE ));
    
    if ((  glb -> eval -> parseExpr( &rExpr ) == NO_ERR ) && ( rExpr.typ == TYP_EXT_ADR )) {
        
        seg = rExpr.seg;
        ofs = rExpr.ofs;
    }
    else return( cmdLineError( ERR_EXPECTED_EXT_ADR ));
    
    if ((  glb -> eval -> parseExpr( &rExpr ) == NO_ERR ) && ( rExpr.typ == TYP_NUM )) {
        
        argAcc = rExpr.numVal;
    }
    else return( cmdLineError( ERR_TLB_ACC_DATA ));
    
    if ((  glb -> eval -> parseExpr( &rExpr ) == NO_ERR ) && ( rExpr.typ == TYP_NUM )) {
        
        argAcc = rExpr.numVal;
    }
    else return( cmdLineError( ERR_TLB_ADR_DATA ));
    
    CpuTlb *tlbPtr = ( tlbTypeId == TOK_I ) ? glb -> cpu -> iTlb : glb -> cpu -> dTlb;
    if ( tlbPtr -> insertTlbEntryData( seg, ofs, argAcc, argAdr )) return( NO_ERR );
    else                                                           return( cmdLineError( ERR_TLB_INSERT_OP ));
}

//------------------------------------------------------------------------------------------------------------
// Display cache entries command.
//
// D-CACHE ( I|D|U ) "," [ <index> ] [ "," <len> ] [ ", " <fmt> ]
//------------------------------------------------------------------------------------------------------------
uint8_t DrvCmds::displayCacheCmd( ) {
    
    TokId       cacheTypeId     = TOK_I;
    uint32_t    cacheSize       = 0;
    CpuMem      *cPtr           = nullptr;
    uint32_t    index           = 0;
    uint32_t    len             = 0;
    int         rdx             = glb -> env -> getEnvVarInt((char *) ENV_RDX_DEFAULT );
    
    if ( glb -> tok -> tokId( ) == TOK_I ) {
        
        cacheSize     = glb -> cpu -> iTlb -> getTlbSize( );
        cacheTypeId   = TOK_I;
        glb -> tok -> nextToken( );
    }
    else if ( glb -> tok -> tokId( ) == TOK_D ) {
        
        cacheSize     = glb -> cpu -> iCacheL1 -> getMemSize( );
        cacheTypeId   = TOK_D;
        glb -> tok -> nextToken( );
    }
    else if ( glb -> tok -> tokId( ) == TOK_U ) {
        
        if ( glb -> cpu -> uCacheL2 != nullptr ) {
            
            cacheSize     = glb -> cpu -> uCacheL2 -> getMemSize( );
            cacheTypeId   = TOK_U;
            glb -> tok -> nextToken( );
        }
        else return( cmdLineError( ERR_CACHE_NOT_CONFIGURED ));
    }
    else return( cmdLineError( ERR_CACHE_TYPE ));
    
    if ( acceptComma( ) != NO_ERR ) return( ERR_EXPECTED_COMMA );
    
    if ( glb -> tok -> tokId( ) == TOK_COMMA ) {
        
        index = 0;
        glb -> tok -> nextToken( );
    }
    else {
        
        DrvExpr rExpr;
        
        if ((  glb -> eval -> parseExpr( &rExpr ) == NO_ERR ) && ( rExpr.typ == TYP_NUM )) {
            
            index = rExpr.numVal;
            if ( glb -> tok -> tokId( ) == TOK_COMMA ) glb -> tok -> nextToken( );
        }
        else {
            
            printf( "Expected the start index\n" );
            return( NO_ERR );
        }
    }
    
    if ( glb -> tok -> tokId( ) == TOK_COMMA ) {
        
        len = 1;
        glb -> tok -> nextToken( );
    }
    else {
        
        DrvExpr rExpr;
        
        if ((  glb -> eval -> parseExpr( &rExpr ) == NO_ERR ) && ( rExpr.typ == TYP_NUM )) {
            
            len = rExpr.numVal;
            if ( glb -> tok -> tokId( ) == TOK_COMMA ) glb -> tok -> nextToken( );
        }
        else {
            
            printf( "Expected number of entries\n" );
            return( NO_ERR );
        }
    }
    
    if ( glb -> tok -> tokId( ) == TOK_COMMA ) {
        
        glb -> tok -> nextToken( );
        
        if (( glb -> tok -> tokId( ) == TOK_HEX ) ||
            ( glb -> tok -> tokId( ) == TOK_OCT ) ||
            ( glb -> tok -> tokId( ) == TOK_DEC )) {
            
            rdx = glb -> tok -> tokVal( );
        }
        else if ( glb -> tok -> tokId( ) == TOK_EOS ) {
            
            rdx = glb -> env -> getEnvVarInt((char *) ENV_RDX_DEFAULT );
        }
        else return( cmdLineError( ERR_INVALID_FMT_OPT ));
    }
    
    if (( index > cacheSize ) || ( index + len > cacheSize )) {
        
        fprintf( stdout, "Index / Len exceed Cache size\n" );
        return( NO_ERR );
    }
    
    if (( index == 0 ) && ( len == 0 )) len = cacheSize;
  
    if ( cPtr != nullptr ) {
        
        uint32_t blockEntries = cPtr -> getBlockEntries( );
        
        if (( index > blockEntries ) || ( index + len > blockEntries )) {
            
            fprintf( stdout, "Index / Len exceed cache size\n" );
        }
        
        if (( index == 0 ) && ( len == 0 )) len = blockEntries;
        
        glb -> lineDisplay -> displayCacheEntries( cPtr, index, len, rdx );
        
        fprintf( stdout, "\n" );
    }
    
    return( NO_ERR );
}

//------------------------------------------------------------------------------------------------------------
// Purges a cache line from the cache.
//
// P-CACHE <I|D|U> <index> <set> [<flush>]
//------------------------------------------------------------------------------------------------------------
uint8_t DrvCmds::purgeCacheCmd( ) {
    
    TokId       cacheTypeId     = TOK_I;
    uint32_t    cacheSize       = 0;
    CpuMem      *cPtr           = nullptr;
    uint32_t    index           = 0;
    uint32_t    len             = 0;
    int         rdx             = glb -> env -> getEnvVarInt((char *) ENV_RDX_DEFAULT );
    uint32_t    set             = 0;
   
    if ( glb -> tok -> tokId( ) == TOK_I ) {
        
        cacheSize     = glb -> cpu -> iTlb -> getTlbSize( );
        cacheTypeId   = TOK_I;
        glb -> tok -> nextToken( );
    }
    else if ( glb -> tok -> tokId( ) == TOK_D ) {
        
        cacheSize     = glb -> cpu -> iCacheL1 -> getMemSize( );
        cacheTypeId   = TOK_D;
        glb -> tok -> nextToken( );
    }
    else if ( glb -> tok -> tokId( ) == TOK_U ) {
        
        if ( glb -> cpu -> uCacheL2 != nullptr ) {
            
            cacheSize     = glb -> cpu -> uCacheL2 -> getMemSize( );
            cacheTypeId   = TOK_U;
            glb -> tok -> nextToken( );
        }
        else return( cmdLineError( ERR_CACHE_NOT_CONFIGURED ));
    }
    else return( cmdLineError( ERR_CACHE_TYPE ));
    
    if ( acceptComma( ) != NO_ERR ) return( ERR_EXPECTED_COMMA );
    
    if ( glb -> tok -> tokId( ) == TOK_COMMA ) {
        
        index = 0;
        glb -> tok -> nextToken( );
    }
    else {
        
        DrvExpr rExpr;
        
        if ((  glb -> eval -> parseExpr( &rExpr ) == NO_ERR ) && ( rExpr.typ == TYP_NUM )) {
            
            index = rExpr.numVal;
            if ( glb -> tok -> tokId( ) == TOK_COMMA ) glb -> tok -> nextToken( );
        }
        else {
            
            printf( "Expected the start index\n" );
            return( NO_ERR );
        }
    }
    
    
    // ??? fix from here ....
    
    
    if ( cPtr != nullptr ) {
        
        if ( set > cPtr -> getBlockSets( ) - 1 ) {
            
            fprintf( stdout, "Invalid cache set number\n" );
            return( 99 );
        }
        
        MemTagEntry  *tagEntry = cPtr -> getMemTagEntry( index, set );
        if ( tagEntry != nullptr ) {
            
            tagEntry -> valid = false;
        }
        else fprintf( stdout, "Cache Operation failed\n" );
    }
    
    return( NO_ERR );
}

//------------------------------------------------------------------------------------------------------------
// Display absolute memory command. The memory address is a byte address. The offset address is a byte address,
// the length is measured in bytes, rounded up to the a word size. We accept any address and length and only
// check that the offset plus length does not exceed the address space. The display routines, who will call
// the actual memory object will take care of gaps in the memory address range. The format specifier will
// allow for HEX, OCTAL, DECIMAL and CODE. In the case of the code option, the default number format option
// is used for showing the offset value.
//
// DA <ofs> [ "," <len> [ "," <rdx> ]]
//------------------------------------------------------------------------------------------------------------
uint8_t DrvCmds::displayAbsMemCmd( ) {
    
    DrvExpr     rExpr;
    uint32_t    ofs     = 0;
    uint32_t    len     = 1;
    int         rdx     = glb -> env -> getEnvVarInt((char *) ENV_RDX_DEFAULT );
    
    if ((  glb -> eval -> parseExpr( &rExpr ) == NO_ERR ) && ( rExpr.typ == TYP_NUM )) ofs = rExpr.numVal;
    else return( cmdLineError( ERR_EXPECTED_START_OFS ));
   
    if ( glb -> tok -> tokId( ) == TOK_COMMA ) {
        
        glb -> tok -> nextToken( );
        if ((  glb -> eval -> parseExpr( &rExpr ) == NO_ERR ) && ( rExpr.typ == TYP_NUM )) len = rExpr.numVal;
        else return( cmdLineError( ERR_EXPECTED_LEN ));
    }
   
    if ( glb -> tok -> tokId( ) == TOK_COMMA ) {
        
        glb -> tok -> nextToken( );
        
        if (( glb -> tok -> tokId( ) == TOK_HEX ) ||
            ( glb -> tok -> tokId( ) == TOK_OCT ) ||
            ( glb -> tok -> tokId( ) == TOK_DEC )) {
            
            rdx = glb -> tok -> tokVal( );
        }
        else if ( glb -> tok -> tokId( ) == TOK_CODE ) {
            
            rdx = 100; // ??? quick hack .... fix....
        }
        else if ( glb -> tok -> tokId( ) == TOK_EOS ) {
            
            rdx = glb -> env -> getEnvVarInt((char *) ENV_RDX_DEFAULT );
        }
        else return( cmdLineError( ERR_INVALID_FMT_OPT ));
        
        glb -> tok -> nextToken( );
    }
    
    if ( checkEOS( ) == NO_ERR ) {
        
        if (((uint64_t) ofs + len ) <= UINT32_MAX ) {
            
            if ( rdx == 100 ) {
                
                glb -> lineDisplay -> displayAbsMemContentAsCode( ofs,
                                                                  len,
                                                                  glb -> env -> getEnvVarInt((char *) ENV_RDX_DEFAULT ));
            }
            else glb -> lineDisplay -> displayAbsMemContent( ofs, len, rdx );
        }
        else return( cmdLineError( ERR_OFS_LEN_LIMIT_EXCEEDED ));
    }
    
    return( NO_ERR );
}

//------------------------------------------------------------------------------------------------------------
// Modify absolute memory command. This command accepts data values for up to eight consecutive locations.
// We also use this command to populate physical memory from a script file.
//
// MA <ofs> "," <val>
//------------------------------------------------------------------------------------------------------------
uint8_t DrvCmds::modifyAbsMemCmd( ) {
    
    DrvExpr     rExpr;
    uint32_t    ofs         = 0;
    uint32_t    val         = 0;
    CpuMem      *physMem    = glb -> cpu -> physMem;
    CpuMem      *pdcMem     = glb -> cpu -> pdcMem;
    CpuMem      *ioMem      = glb -> cpu -> ioMem;
    CpuMem      *mem        = nullptr;
    
    if ((  glb -> eval -> parseExpr( &rExpr ) == NO_ERR ) && ( rExpr.typ == TYP_NUM )) ofs = rExpr.numVal;
    else return( cmdLineError( ERR_EXPECTED_OFS ));
    
    if ( acceptComma( ) != NO_ERR ) return( ERR_EXPECTED_COMMA );
    
    if ((  glb -> eval -> parseExpr( &rExpr ) == NO_ERR ) && ( rExpr.typ == TYP_NUM )) val = rExpr.numVal;
    else return( cmdLineError( ERR_INVALID_NUM ));
    
    if ( checkEOS( ) == NO_ERR ) {
        
        if      (( physMem != nullptr ) && ( physMem -> validAdr( ofs ))) mem = physMem;
        else if (( pdcMem  != nullptr ) && ( pdcMem  -> validAdr( ofs ))) mem = pdcMem;
        else if (( ioMem   != nullptr ) && ( ioMem   -> validAdr( ofs ))) mem = ioMem;
        
        if (((uint64_t) ofs + 4 ) > UINT32_MAX ) {
            
            return( cmdLineError( ERR_OFS_LEN_LIMIT_EXCEEDED ));
        }
        
        mem -> putMemDataWord( ofs, val );
    }
    
    return( NO_ERR );
}

//------------------------------------------------------------------------------------------------------------
// Modify absolute code memory command. This command accepts an address and string that represents the code
// word in assembly format.
//
// MAA <ofs> "," <asm-string>
//------------------------------------------------------------------------------------------------------------
uint8_t DrvCmds::modifyAbsMemAsCodeCmd( ) {
    
    DrvExpr     rExpr;
    uint32_t    ofs         = 0;
    uint32_t    instr       = 0;
    CpuMem      *physMem    = glb -> cpu -> physMem;
    CpuMem      *pdcMem     = glb -> cpu -> pdcMem;
    CpuMem      *ioMem      = glb -> cpu -> ioMem;
    CpuMem      *mem        = nullptr;
    
    if ((  glb -> eval -> parseExpr( &rExpr ) == NO_ERR ) && ( rExpr.typ == TYP_NUM )) ofs = rExpr.numVal;
    else return( cmdLineError( ERR_EXPECTED_OFS ));
    
    if ( acceptComma( ) != NO_ERR ) return( ERR_EXPECTED_COMMA );
    
    if ((  glb -> eval -> parseExpr( &rExpr ) == NO_ERR ) && ( rExpr.typ == TYP_STR )) ;
    else return( cmdLineError( ERR_INVALID_NUM ));
    
    if ( checkEOS( ) == NO_ERR ) {
        
        if      (( physMem != nullptr ) && ( physMem -> validAdr( ofs ))) mem = physMem;
        else if (( pdcMem  != nullptr ) && ( pdcMem  -> validAdr( ofs ))) mem = pdcMem;
        else if (( ioMem   != nullptr ) && ( ioMem   -> validAdr( ofs ))) mem = ioMem;
        
        if (((uint64_t) ofs + 4 ) > UINT32_MAX ) {
            
            return( cmdLineError( ERR_OFS_LEN_LIMIT_EXCEEDED ));
        }
        
        if ( glb -> oneLineAsm -> parseAsmLine( rExpr.strVal, &instr )) {
            
            mem -> putMemDataWord( ofs, instr );
        }
    }
    
    return( NO_ERR );
}

//------------------------------------------------------------------------------------------------------------
// Global windows commands. There are handlers for turning windows on, off and set them back to their default
// values. We also support two stacks of windows next to each other.
//
//------------------------------------------------------------------------------------------------------------
uint8_t DrvCmds::winOnCmd( ) {
    
    winModeOn = true;
    glb -> winDisplay -> windowsOn( );
    glb -> winDisplay -> reDraw( true );
    return( NO_ERR );
}

uint8_t DrvCmds::winOffCmd( ) {
    
    if ( winModeOn ) {
        
        winModeOn = false;
        glb -> winDisplay -> windowsOff( );
        return( NO_ERR );
    }
    else return( cmdLineError( ERR_NOT_IN_WIN_MODE ));
}

uint8_t DrvCmds::winDefCmd( ) {
    
    if ( winModeOn ) {
        
        glb -> winDisplay -> windowDefaults( );
        glb -> winDisplay -> reDraw( true );
        return( NO_ERR );
    }
    else return( cmdLineError( ERR_NOT_IN_WIN_MODE ));
}

uint8_t DrvCmds::winStacksEnable( ) {
    
    if ( winModeOn ) {
        
        glb -> winDisplay -> winStacksEnable( true );
        glb -> winDisplay -> reDraw( true );
        return( NO_ERR );
    }
    else return( cmdLineError( ERR_NOT_IN_WIN_MODE ));
}

uint8_t DrvCmds::winStacksDisable( ) {
    
    if ( winModeOn ) {
        
        glb -> winDisplay -> winStacksEnable( false );
        glb -> winDisplay -> reDraw( true );
        return( NO_ERR );
    }
    else return( cmdLineError( ERR_NOT_IN_WIN_MODE ));
}

//------------------------------------------------------------------------------------------------------------
// Window current command. User definable windows are controlled by their window number. To avoid typing this
// number all the time for a user window command, a user window can explicitly be set as the current command.
//
// WC <winNum>
//------------------------------------------------------------------------------------------------------------
uint8_t DrvCmds::winCurrentCmd( ) {
    
    if ( ! winModeOn ) return( cmdLineError( ERR_NOT_IN_WIN_MODE ));
       
    DrvExpr rExpr;
    
    if ((  glb -> eval -> parseExpr( &rExpr ) == NO_ERR ) && ( rExpr.typ == TYP_NUM )) {
        
        if ( glb -> winDisplay -> validWindowNum( rExpr.numVal )) {
            
            glb -> winDisplay -> windowCurrent( rExpr.numVal );
            return( NO_ERR );
        }
        else return( cmdLineError( ERR_INVALID_WIN_ID ));
    }
    else return( cmdLineError( ERR_EXPECTED_WIN_ID ));
}

//------------------------------------------------------------------------------------------------------------
// Windows enable and disable. When enabled, a window does show up on the screen. The window number is
// optional, used for user definable windows.
//
// <win>E [<winNum>]
// <win>D [<winNum>]
//------------------------------------------------------------------------------------------------------------
uint8_t DrvCmds::winEnableCmd( TokId winCmd ) {
    
    int winNum = 0;
    
    if ( ! winModeOn ) return( cmdLineError( ERR_NOT_IN_WIN_MODE ));
    
    if ( glb -> tok -> tokId( ) != TOK_EOS ) {
        
        DrvExpr rExpr;
        
        if ((  glb -> eval -> parseExpr( &rExpr ) == NO_ERR ) && ( rExpr.typ == TYP_NUM )) winNum = rExpr.numVal;
        else return( cmdLineError( ERR_EXPECTED_WIN_ID ));
    }
    
    if ( glb -> winDisplay -> validWindowNum( winNum )) {
        
        glb -> winDisplay -> windowEnable( winCmd, winNum );
        glb -> winDisplay -> reDraw( true );
        return( NO_ERR );
    }
    else return( cmdLineError( ERR_INVALID_WIN_ID ));
}

uint8_t DrvCmds::winDisableCmd( TokId winCmd ) {
        
    int winNum = 0;
    
    if ( ! winModeOn ) return( cmdLineError( ERR_NOT_IN_WIN_MODE ));
    
    if ( glb -> tok -> tokId( ) != TOK_EOS ) {
        
        DrvExpr rExpr;
        
        if ((  glb -> eval -> parseExpr( &rExpr ) == NO_ERR ) && ( rExpr.typ == TYP_NUM )) winNum = rExpr.numVal;
        else return( cmdLineError( ERR_EXPECTED_WIN_ID ));
    }
    
    if ( glb -> winDisplay -> validWindowNum( winNum )) {
        
        glb -> winDisplay -> windowDisable( winCmd, winNum );
        glb -> winDisplay -> reDraw( true );
        return( NO_ERR );
    }
    else return( cmdLineError( ERR_INVALID_WIN_ID ));
}

//------------------------------------------------------------------------------------------------------------
// Windows radix. This command sets the radix for a given window. We parse the command and the format
// option and pass the tokens to the screen handler. The window number is optional, used for user definable
// windows.
//
// <win>R [ <radix> [ "," <winNum>]]
//------------------------------------------------------------------------------------------------------------
uint8_t DrvCmds::winSetRadixCmd( TokId winCmd ) {
        
    if ( ! winModeOn ) return( cmdLineError( ERR_NOT_IN_WIN_MODE ));
                                  
    DrvExpr rExpr;
    int     winNum  = 0;
    int     rdx     = 16;
        
    if ( glb -> tok -> tokId( ) == TOK_COMMA ) {
        
        rdx = glb -> env -> getEnvVarInt((char *) ENV_RDX_DEFAULT );
        glb -> tok -> nextToken( );
    }
    else {
        
        if ((  glb -> eval -> parseExpr( &rExpr ) == NO_ERR ) && ( rExpr.typ == TYP_NUM )) {
                
            rdx = setRadix( rExpr.numVal );
        }
        else return( cmdLineError( ERR_INVALID_RADIX ));
    }
    
    if ( glb -> tok -> tokId( ) == TOK_COMMA ) {
        
        glb -> tok -> nextToken( );
        if ((  glb -> eval -> parseExpr( &rExpr ) == NO_ERR ) && ( rExpr.typ == TYP_NUM )) {
            
            winNum = rExpr.numVal;
            glb -> tok -> nextToken( );
        }
        else return( cmdLineError( ERR_INVALID_RADIX ));
    }
    
    if ( glb -> winDisplay -> validWindowNum( winNum )) {
        
        glb -> winDisplay -> windowRadix( winCmd, rdx, winNum );
        return( NO_ERR );
    }
    else return( cmdLineError( ERR_INVALID_WIN_ID ));
    
    return( NO_ERR );
}

//------------------------------------------------------------------------------------------------------------
// Window scrolling. This command advances the item address of a scrollable window by the number of lines
// multiplied by the number of items on a line forward or backward. The meaning of the item address and line
// items is window dependent. The window number is optional, used for user definable windows. If omitted,
// we mean the current window.
//
// <win>F [<items> [ "," <winNum>]]
// <win>B [<items> [ "," <winNum>]]
//------------------------------------------------------------------------------------------------------------
uint8_t DrvCmds::winForwardCmd( TokId winCmd ) {
    
    DrvExpr rExpr;
    int     winItems = 0;
    int     winNum   = 0;
    
    if ( ! winModeOn ) return( cmdLineError( ERR_NOT_IN_WIN_MODE ));
    
    if ( glb -> tok -> tokId( ) == TOK_EOS ) {
        
        glb -> winDisplay -> windowForward( winCmd, winItems, winNum  );
        return( NO_ERR );
    }
    
    if ((  glb -> eval -> parseExpr( &rExpr )) && ( rExpr.typ == TYP_NUM )) winItems = rExpr.numVal;
    else return( cmdLineError( ERR_INVALID_NUM ));
    
    if ( glb -> tok -> tokId( ) == TOK_COMMA ) {
        
        glb ->  tok -> nextToken( );
        
        if ( glb -> tok -> tokTyp( ) == TYP_NUM )winNum = glb -> tok -> tokVal( );
        else return ( cmdLineError( ERR_INVALID_WIN_ID ));
        
        if ( ! glb -> winDisplay -> validWindowNum( winNum ))
            return ( cmdLineError( ERR_INVALID_WIN_ID ));
    }
    else winNum = 0;
    
    glb -> winDisplay -> windowForward( winCmd, winItems, winNum  );
    return( NO_ERR );
}

uint8_t DrvCmds::winBackwardCmd( TokId winCmd ) {
    
    DrvExpr rExpr;
    int     winItems = 0;
    int     winNum   = 0;
    
    if ( ! winModeOn ) return( cmdLineError( ERR_NOT_IN_WIN_MODE ));
    
    if ( glb -> tok -> tokId( ) == TOK_EOS ) {
        
        glb -> winDisplay -> windowBackward( winCmd, winItems, winNum  );
        return( NO_ERR );
    }
    
    if ((  glb -> eval -> parseExpr( &rExpr )) && ( rExpr.typ == TYP_NUM )) winItems = rExpr.numVal;
    else return( cmdLineError( ERR_INVALID_NUM ));
    
    if ( glb -> tok -> tokId( ) == TOK_COMMA ) {
        
        glb -> tok -> nextToken( );
        
        if ( glb -> tok -> tokTyp( ) == TYP_NUM )winNum = glb -> tok -> tokVal( );
        else return ( cmdLineError( ERR_INVALID_WIN_ID ));
        
        if ( ! glb -> winDisplay -> validWindowNum( winNum ))
            return ( cmdLineError( ERR_INVALID_WIN_ID ));
    }
    else winNum = 0;
    
    glb -> winDisplay -> windowBackward( winCmd, winItems, winNum  );
    return( NO_ERR );
}

//------------------------------------------------------------------------------------------------------------
// Window home. Each window has a home item address, which was set at window creation or trough a non-zero
// value passed to this command. The command sets the window item address to this value. The meaning of the
// item address is window dependent. The window number is optional, used for user definable windows.
//
// <win>H [<pos> [ "," <winNum>]]
//------------------------------------------------------------------------------------------------------------
uint8_t DrvCmds::winHomeCmd( TokId winCmd ) {
    
    DrvExpr rExpr;
    int     winPos   = 0;
    int     winNum   = 0;
    
    if ( ! winModeOn ) return( cmdLineError( ERR_NOT_IN_WIN_MODE ));
    
    if ( glb -> tok -> tokId( ) == TOK_EOS ) {
        
        glb -> winDisplay -> windowHome( winCmd, winPos, winNum  );
        return( NO_ERR );
    }
    
    if ((  glb -> eval -> parseExpr( &rExpr )) && ( rExpr.typ == TYP_NUM )) winPos = rExpr.numVal;
    else return( cmdLineError( ERR_INVALID_NUM ));
    
    if ( glb -> tok -> tokId( ) == TOK_COMMA ) {
        
        glb -> tok -> nextToken( );
        
        if ( glb -> tok -> tokTyp( ) == TYP_NUM )winNum = glb -> tok -> tokVal( );
        else return ( cmdLineError( ERR_INVALID_WIN_ID ));
        
        if ( ! glb -> winDisplay -> validWindowNum( winNum ))
            return ( cmdLineError( ERR_INVALID_WIN_ID ));
    }
    else winNum = 0;
    
    glb -> winDisplay -> windowHome( winCmd, winPos, winNum  );
    return( NO_ERR );
}

//------------------------------------------------------------------------------------------------------------
// Window jump. The window jump command sets the item address to the position argument. The meaning of the
// item address is window dependent. The window number is optional, used for user definable windows.
//
// <win>J [<pos> [ "," <winNum>]]
//------------------------------------------------------------------------------------------------------------
uint8_t DrvCmds::winJumpCmd( TokId winCmd ) {
    
    DrvExpr rExpr;
    int     winPos   = 0;
    int     winNum   = 0;
    
    if ( ! winModeOn ) return( cmdLineError( ERR_NOT_IN_WIN_MODE ));
    
    if ( glb -> tok -> tokId( ) == TOK_EOS ) {
        
        glb -> winDisplay -> windowHome( winCmd, winPos, winNum  );
        return( NO_ERR );
    }
    
    if ((  glb -> eval -> parseExpr( &rExpr )) && ( rExpr.typ == TYP_NUM )) winPos = rExpr.numVal;
    else return( cmdLineError( ERR_INVALID_NUM ));
    
    if ( glb -> tok -> tokId( ) == TOK_COMMA ) {
        
        glb -> tok -> nextToken( );
        
        if ( glb -> tok -> tokTyp( ) == TYP_NUM )winNum = glb -> tok -> tokVal( );
        else return ( cmdLineError( ERR_INVALID_WIN_ID ));
        
        if ( ! glb -> winDisplay -> validWindowNum( winNum ))
            return ( cmdLineError( ERR_INVALID_WIN_ID ));
    }
    else winNum = 0;
    
    glb -> winDisplay -> windowJump( winCmd, winPos, winNum  );
    return( NO_ERR );
}

//------------------------------------------------------------------------------------------------------------
// Set window lines. This command sets the the number of rows for a window. The number includes the banner
// line. The window number is optional, used for user definable windows.
//
// <win>L [<lines> [ "," <winNum>]]
//------------------------------------------------------------------------------------------------------------
uint8_t DrvCmds::winSetRowsCmd( TokId winCmd ) {
    
    DrvExpr rExpr;
    int     winLines    = 0;
    int     winNum      = 0;
    
    if ( ! winModeOn ) return( cmdLineError( ERR_NOT_IN_WIN_MODE ));
    
    if ( glb -> tok -> tokId( ) == TOK_EOS ) {
        
        glb -> winDisplay -> windowHome( winCmd, winLines, winNum  );
        return( NO_ERR );
    }
    
    if ((  glb -> eval -> parseExpr( &rExpr )) && ( rExpr.typ == TYP_NUM )) winLines = rExpr.numVal;
    else return( cmdLineError( ERR_INVALID_NUM ));
    
    if ( glb -> tok -> tokId( ) == TOK_COMMA ) {
        
        glb -> tok -> nextToken( );
        
        if ( glb -> tok -> tokTyp( ) == TYP_NUM )winNum = glb -> tok -> tokVal( );
        else return ( cmdLineError( ERR_INVALID_WIN_ID ));
        
        if ( ! glb -> winDisplay -> validWindowNum( winNum ))
            return ( cmdLineError( ERR_INVALID_WIN_ID ));
    }
    else winNum = 0;
    
    glb -> winDisplay -> windowSetRows( winCmd, winLines, winNum );
    glb -> winDisplay -> reDraw( true );
    return( NO_ERR );
}

//------------------------------------------------------------------------------------------------------------
// This command creates a new user window. The window is assigned a free index form the windows list. This
// index is used in all the calls to this window. The window type allows to select from a code window, a
// physical memory window, a TLB and a CACHE window.
//
// WN <winType> [ "," <arg> ]
//------------------------------------------------------------------------------------------------------------
uint8_t DrvCmds::winNewWinCmd( ) {
    
    TokId   winType = TOK_NIL;
    char    *argStr = nullptr;
    
    if ( ! winModeOn ) return( cmdLineError( ERR_NOT_IN_WIN_MODE ));
    
    if ( glb -> tok -> tokTyp( ) == TYP_SYM ) {
        
        winType = glb -> tok -> tokId( );
        
        if ((( winType == TOK_PM ) && ( glb -> cpu -> physMem == nullptr ))     ||
            (( winType == TOK_PC ) && ( glb -> cpu -> physMem == nullptr ))     ||
            (( winType == TOK_IT ) && ( glb -> cpu -> iTlb == nullptr ))        ||
            (( winType == TOK_DT ) && ( glb -> cpu -> dTlb == nullptr ))        ||
            (( winType == TOK_IC ) && ( glb -> cpu -> iCacheL1 == nullptr ))    ||
            (( winType == TOK_DC ) && ( glb -> cpu -> dCacheL1 == nullptr ))    ||
            (( winType == TOK_UC ) && ( glb -> cpu -> uCacheL2 == nullptr ))) {
            
            return( cmdLineError( ERR_WIN_TYPE_NOT_CONFIGURED ));
        }
        
        if ( ! glb -> winDisplay -> validUserWindowType( winType ))
            return( cmdLineError( ERR_INVALID_WIN_TYPE ));
        
        glb -> tok -> nextToken( );
    }
    
    if ( glb -> tok -> tokId( ) == TOK_COMMA ) {
        
        glb -> tok -> nextToken( );
        
        if (glb ->  tok -> tokTyp( ) == TYP_STR ) argStr = glb -> tok -> tokStr( );
        else return( cmdLineError( ERR_INVALID_ARG ));
    }
    
    glb -> winDisplay -> windowNew( winType, argStr );
    glb -> winDisplay -> reDraw( true );
    return( NO_ERR );
}

//------------------------------------------------------------------------------------------------------------
// This command removes  a user defined window or window range from the list of windows. A number of -1 will
// kill all user defined windows.
//
// WK [<winNumStart> [ "," <winNumEnd]] || ( -1 )
//------------------------------------------------------------------------------------------------------------
uint8_t DrvCmds::winKillWinCmd( ) {
    
    DrvExpr     rExpr;
    int         winNumStart     = 0;
    int         winNumEnd       = 0;
    
    if ( ! winModeOn ) return( cmdLineError( ERR_NOT_IN_WIN_MODE ));
    
    if ( glb -> tok -> tokId( ) == TOK_EOS ) {
        
        winNumStart = glb -> winDisplay -> getCurrentUserWindow( );
        winNumEnd   = winNumStart;
    }
    else {
        
        if ((  glb -> eval -> parseExpr( &rExpr )) && ( rExpr.typ == TYP_NUM )) winNumStart = rExpr.numVal;
        else return ( cmdLineError( ERR_EXPECTED_NUMERIC ));
        
        if ( glb -> tok -> tokId( ) == TOK_COMMA ) {
            
            glb -> tok -> nextToken( );
            
            if ((  glb -> eval -> parseExpr( &rExpr )) && ( rExpr.typ == TYP_NUM )) winNumEnd = rExpr.numVal;
            else return ( cmdLineError( ERR_EXPECTED_NUMERIC ));
        }
        
        if ( winNumStart == -1 ) {
            
            winNumStart = glb -> winDisplay -> getFirstUserWinIndex( );
            winNumEnd   = glb -> winDisplay -> getLastUserWinIndex( );
        }
    }
    
    if (( ! glb -> winDisplay -> validWindowNum( winNumStart )) ||
        ( ! glb -> winDisplay -> validWindowNum( winNumEnd ))) {
        
        return ( cmdLineError( ERR_INVALID_WIN_ID ));
    }
    
    glb -> winDisplay -> windowKill( winNumStart, winNumEnd );
    glb -> winDisplay -> reDraw( true );
    return( NO_ERR );
}

//------------------------------------------------------------------------------------------------------------
// This command assigns a user window to a stack. User windows can be displayed in a separate stack of
// windows. The first stack is always the main stack, where the predefined and command window can be found.
//
// WS <stackNum> [ "," <winNumStart> [ "," <winNumEnd ]]
//------------------------------------------------------------------------------------------------------------
uint8_t DrvCmds::winSetStackCmd( ) {
    
    DrvExpr rExpr;
    int     stackNum    = 0;
    int     winNumStart = 0;
    int     winNumEnd   = 0;
    
    if ( ! winModeOn ) return( cmdLineError( ERR_NOT_IN_WIN_MODE ));
    
    if ((  glb -> eval -> parseExpr( &rExpr )) && ( rExpr.typ == TYP_NUM )) stackNum = rExpr.numVal;
    else return ( cmdLineError( ERR_EXPECTED_STACK_ID ));
    
    if ( ! glb -> winDisplay -> validWindowStackNum( stackNum ))
        return( cmdLineError( ERR_INVALID_WIN_STACK_ID ));
        
    if ( glb -> tok -> tokId( ) == TOK_EOS ) {
        
        winNumStart = glb -> winDisplay -> getCurrentUserWindow( );
        winNumEnd   = winNumStart;
    }
    else {
        
        if ( glb -> tok -> tokId( ) == TOK_COMMA ) {
            
            glb -> tok -> nextToken( );
            
            if ((  glb -> eval -> parseExpr( &rExpr )) && ( rExpr.typ == TYP_NUM )) winNumStart = rExpr.numVal;
            else return ( cmdLineError( ERR_EXPECTED_NUMERIC ));
            
            if ( glb -> tok -> tokId( ) == TOK_COMMA ) {
                
                glb -> tok -> nextToken( );
                
                if ((  glb -> eval -> parseExpr( &rExpr )) && ( rExpr.typ == TYP_NUM )) winNumEnd = rExpr.numVal;
                else return ( cmdLineError( ERR_EXPECTED_NUMERIC ));
            }
            else winNumEnd = winNumStart;
        }
        
        if ( winNumStart == -1 ) {
                
            winNumStart = glb -> winDisplay -> getFirstUserWinIndex( );
            winNumEnd   = glb -> winDisplay -> getLastUserWinIndex( );
        }
    }
    
    if (( ! glb -> winDisplay -> validWindowNum( winNumStart )) ||
        ( ! glb -> winDisplay -> validWindowNum( winNumEnd ))) {
        
        return ( cmdLineError( ERR_INVALID_WIN_ID ));
    }
    
    glb -> winDisplay -> windowSetStack( stackNum, winNumStart, winNumEnd );
    glb -> winDisplay -> reDraw( true );
    return( NO_ERR );
}

//------------------------------------------------------------------------------------------------------------
// This command toggles through alternate window content, if the window dos support it. An example is the
// cache sets in a two-way associative cache. The toggle command will just flip through the sets.
//
// WT [ <winNum> ]
//------------------------------------------------------------------------------------------------------------
uint8_t  DrvCmds::winToggleCmd( ) {
    
    if ( ! winModeOn ) return( cmdLineError( ERR_NOT_IN_WIN_MODE ));
    
    if ( glb -> tok -> tokTyp( ) == TYP_NUM ) {
        
        if ( ! glb -> winDisplay -> validWindowNum( glb -> tok -> tokVal( )))
            return( cmdLineError( ERR_INVALID_WIN_ID ));
        
            glb -> winDisplay -> windowToggle( glb -> tok -> tokVal( ));
            return( NO_ERR );
    }
    else return( cmdLineError( ERR_EXPECTED_WIN_ID ));
}

//------------------------------------------------------------------------------------------------------------
// This command exchanges the current user window with the user window specified. It allows to change the
// order of the user windows in a stacks.
//
// WX <winNum>
//------------------------------------------------------------------------------------------------------------
uint8_t DrvCmds::winExchangeCmd( ) {
    
    if ( ! winModeOn ) return( cmdLineError( ERR_NOT_IN_WIN_MODE ));
    
    if ( glb -> tok -> tokTyp( ) == TYP_NUM ) {
        
        if ( ! glb -> winDisplay -> validWindowNum( glb -> tok -> tokVal( )))
            return( cmdLineError( ERR_INVALID_WIN_ID ));
        
        glb -> winDisplay -> windowExchangeOrder( glb -> tok -> tokVal( ));
        return( NO_ERR );
    }
    else return( cmdLineError( ERR_EXPECTED_WIN_ID ));
}

//------------------------------------------------------------------------------------------------------------
// Evaluate input line. There are commands, functions, expressions and so on. This routine sets up the
// tokenizer and dispatches based on the first token in the input line.
//
//------------------------------------------------------------------------------------------------------------
uint8_t DrvCmds::evalInputLine( char *cmdBuf ) {
    
    DrvExpr rExpr;
    
    if ( strlen( cmdBuf ) > 0 ) {
        
        if ( glb -> tok -> setupTokenizer( cmdBuf, (DrvToken *) cmdTokTab ) != NO_ERR ) return( ERR_INVALID_CMD );
        glb -> tok -> nextToken( );
        
        if ( glb -> eval -> parseExpr( &rExpr ) != NO_ERR ) return( ERR_INVALID_CMD );
        
        switch( rExpr.typ ) {
                
            //------------------------------------------------------------------------------------------------
            // We have a command as the first expression in the input string. Just dispatch to the command.
            //
            //------------------------------------------------------------------------------------------------
            case TYP_CMD: {
                
                switch ( rExpr.tokId) {
                        
                    case TOK_NIL:           return( NO_ERR );
                    case CMD_EXIT:          return( exitCmd( ));
                    case CMD_HELP:          return( helpCmd( ));
                    case CMD_WHELP:         return( winHelpCmd( ));
                    case CMD_ENV:           return( envCmd( ));
                    case CMD_XF:            return( execFileCmd( ));
                    case CMD_LMF:           return( loadPhysMemCmd( ));
                    case CMD_SMF:           return( savePhysMemCmd( ));
                    case CMD_RESET:         return( resetCmd( ));
                    case CMD_RUN:           return( runCmd( ));
                    case CMD_STEP:          return( stepCmd( ));
                    case CMD_DIS_ASM:       return( disAssembleCmd( ));
                    case CMD_ASM:           return( assembleCmd( ));
                    case CMD_DR:            return( displayRegCmd( ));
                    case CMD_MR:            return( modifyRegCmd( ));
                    case CMD_HASH_VA:       return( hashVACmd( ));
                    case CMD_D_TLB:         return( displayTLBCmd( ));
                    case CMD_I_TLB:         return( insertTLBCmd( ));
                    case CMD_P_TLB:         return( purgeTLBCmd( ));
                    case CMD_D_CACHE:       return( displayCacheCmd( ));
                    case CMD_P_CACHE:       return( purgeCacheCmd( ));
                    case CMD_DA:            return( displayAbsMemCmd( ));
                    case CMD_MA:            return( modifyAbsMemCmd( ));
                    case CMD_MAA:           return( modifyAbsMemAsCodeCmd( ));
    
                    case CMD_WON:           return( winOnCmd( ));
                    case CMD_WOFF:          return( winOffCmd( ));
                    case CMD_WDEF:          return( winDefCmd( ));
                    case CMD_WSE:           return( winStacksEnable( ));
                    case CMD_WSD:           return( winStacksDisable( ));
                        
                    case CMD_WC:            return( winCurrentCmd( ));
                    case CMD_WN:            return( winNewWinCmd( ));
                    case CMD_WK:            return( winKillWinCmd( ));
                    case CMD_WS:            return( winSetStackCmd( ));
                    case CMD_WT:            return( winToggleCmd( ));
                    case CMD_WX:            return( winExchangeCmd( ));
                        
                    case CMD_WF:            return( winForwardCmd( rExpr.tokId ));
                    case CMD_WB:            return( winBackwardCmd( rExpr.tokId ));
                    case CMD_WH:            return( winHomeCmd( rExpr.tokId ));
                    case CMD_WJ:            return( winJumpCmd( rExpr.tokId ));
                        
                    case CMD_PSE:
                    case CMD_SRE:
                    case CMD_PLE:
                    case CMD_SWE:
                    case CMD_WE:            return( winEnableCmd( rExpr.tokId ));
                        
                    case CMD_PSD:
                    case CMD_SRD:
                    case CMD_PLD:
                    case CMD_SWD:
                    case CMD_WD:            return( winDisableCmd( rExpr.tokId ));
                        
                    case CMD_PSR:
                    case CMD_SRR:
                    case CMD_PLR:
                    case CMD_SWR:
                    case CMD_WR:            return( winSetRadixCmd( rExpr.tokId ));
                        
                    case CMD_CWL:
                    case CMD_WL:            return( winSetRowsCmd( rExpr.tokId ));
                        
                    default:                invalidCmd( );
                }
                
            } break;
                
            //------------------------------------------------------------------------------------------------
            // Am expression result. We just print the value according to its type.
            //
            //------------------------------------------------------------------------------------------------
            case TYP_NUM:  printf( "%i\n", rExpr.numVal );         break;
            case TYP_GREG: printf( "0x%08x\n", rExpr.numVal );     break;
            case TYP_SREG: printf( "0x%04x\n", rExpr.numVal );     break;
            case TYP_CREG: printf( "0x%08x\n", rExpr.numVal );     break;
                
            //------------------------------------------------------------------------------------------------
            // Address values.
            //
            //------------------------------------------------------------------------------------------------
            case TYP_ADR:        printf( "0x%08x\n", rExpr.adr );                     break;
            case TYP_EXT_ADR:    printf( "0x%04x.0x%08x\n", rExpr.seg, rExpr.ofs );   break;
                
            //------------------------------------------------------------------------------------------------
            // No idea what it is, assume an invalid command.
            //
            //------------------------------------------------------------------------------------------------
            default: invalidCmd( );
        }
    }
    
    return( NO_ERR );
}

//------------------------------------------------------------------------------------------------------------
// "cmdLoop" is the command line input interpreter. The basic loop is to prompt for the next input, read the
// input and evaluates it. If we are in windows mode, we also redraw the screen.
//
// ??? when is the best point to redraw the windows... exactly once ?
//------------------------------------------------------------------------------------------------------------
void DrvCmds::cmdInterpreterLoop( ) {
    
    char cmdLineBuf[ CMD_LINE_BUF_SIZE ] = { 0 };
    
    printWelcome( );
    
    while ( true ) {
        
        promptCmdLine( );
        if ( readInputLine( cmdLineBuf )) {
            
            evalInputLine( cmdLineBuf );
            if ( winModeOn ) glb -> winDisplay -> reDraw( );
        }
    }
}
