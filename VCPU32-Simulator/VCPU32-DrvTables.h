//------------------------------------------------------------------------------------------------------------
//
//  VCPU32 - A 32-bit CPU - Simulator Driver
//
//------------------------------------------------------------------------------------------------------------
// 
//
//
//------------------------------------------------------------------------------------------------------------
//
// VCPU32 - A 32-bit CPU - Simulator Driver
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
#ifndef VCPU32DrvTables_h
#define VCPU32DrvTables_h

//------------------------------------------------------------------------------------------------------------
// The global command interpreter token table. All reserved words are allocated in this table. Each entry
// has the token name, the token id, the token type id, i.e. its type, and a value associated with the token.
// The value allows for a constant token. The parser can directly use the value in expressions.
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

const int MAX_CMD_TOKEN_TAB = sizeof( cmdTokTab ) / sizeof( DrvToken );

//------------------------------------------------------------------------------------------------------------
// The error messgae table. Ech entry has the error number and the corresponding error message text.
//
//------------------------------------------------------------------------------------------------------------
DrvErrMsgTabEntry const errTab [ ] = {
    
    { .errNum = NO_ERR, .errStr = (char *) "hugo " }
    
};

const int MAX_ERR_MSG_TAB = sizeof( errTab ) / sizeof( DrvErrMsgTabEntry );

//------------------------------------------------------------------------------------------------------------
// COmmand help table.
//
//------------------------------------------------------------------------------------------------------------
DrvHelpMsgEntry const cmdHelpTab[ ] = {
    
    
};

const int MAX_CMD_HELP_TAB = sizeof( cmdHelpTab ) / sizeof( DrvHelpMsgEntry );

//------------------------------------------------------------------------------------------------------------
// Window command help table.
//
//------------------------------------------------------------------------------------------------------------
DrvHelpMsgEntry const winCmdHelpTab[ ] = {
    
    
};

const int MAX_WIN_CMD_HELP_TAB = sizeof( winCmdHelpTab ) / sizeof( DrvHelpMsgEntry );



//------------------------------------------------------------------------------------------------------------
// The global token table or the one line assembler. All reserved words are allocated in this table. Each
// entry has the token name, the token id, the token type id, i.e. its type, and a value associated with the
// token. The value allows for a constant token. The parser can directly use the value in expressions.
//
//------------------------------------------------------------------------------------------------------------
DrvToken const asmTokTab[ ] = {
    
    //--------------------------------------------------------------------------------------------------------
    // General registers.
    //
    //--------------------------------------------------------------------------------------------------------
    { .name = "R0",             .typ = TYP_GREG,         .tid = GR_0,                .val = 0           },
    { .name = "R1",             .typ = TYP_GREG,         .tid = GR_1,                .val = 1           },
    { .name = "R2",             .typ = TYP_GREG,         .tid = GR_2,                .val = 2           },
    { .name = "R3",             .typ = TYP_GREG,         .tid = GR_3,                .val = 3           },
    { .name = "R4",             .typ = TYP_GREG,         .tid = GR_4,                .val = 4           },
    { .name = "R5",             .typ = TYP_GREG,         .tid = GR_5,                .val = 5           },
    { .name = "R6",             .typ = TYP_GREG,         .tid = GR_6,                .val = 6           },
    { .name = "R7",             .typ = TYP_GREG,         .tid = GR_7,                .val = 7           },
    { .name = "R8",             .typ = TYP_GREG,         .tid = GR_8,                .val = 8           },
    { .name = "R9",             .typ = TYP_GREG,         .tid = GR_9,                .val = 9           },
    { .name = "R10",            .typ = TYP_GREG,         .tid = GR_10,               .val = 10          },
    { .name = "R11",            .typ = TYP_GREG,         .tid = GR_11,               .val = 11          },
    { .name = "R12",            .typ = TYP_GREG,         .tid = GR_12,               .val = 12          },
    { .name = "R13",            .typ = TYP_GREG,         .tid = GR_13,               .val = 13          },
    { .name = "R14",            .typ = TYP_GREG,         .tid = GR_14,               .val = 14          },
    { .name = "R15",            .typ = TYP_GREG,         .tid = GR_15,               .val = 15          },

    //--------------------------------------------------------------------------------------------------------
    // Segment registers.
    //
    //--------------------------------------------------------------------------------------------------------
    { .name = "S0",             .typ = TYP_SREG,         .tid = SR_0,                .val = 0           },
    { .name = "S1",             .typ = TYP_SREG,         .tid = SR_1,                .val = 1           },
    { .name = "S2",             .typ = TYP_SREG,         .tid = SR_2,                .val = 2           },
    { .name = "S3",             .typ = TYP_SREG,         .tid = SR_3,                .val = 3           },
    { .name = "S4",             .typ = TYP_SREG,         .tid = SR_4,                .val = 4           },
    { .name = "S5",             .typ = TYP_SREG,         .tid = SR_5,                .val = 5           },
    { .name = "S6",             .typ = TYP_SREG,         .tid = SR_6,                .val = 6           },
    { .name = "S7",             .typ = TYP_SREG,         .tid = SR_7,                .val = 7           },
    
    //--------------------------------------------------------------------------------------------------------
    // Control registers.
    //
    //--------------------------------------------------------------------------------------------------------
    { .name = "C0",             .typ = TYP_CREG,        .tid = CR_0,                .val = 0            },
    { .name = "C1",             .typ = TYP_CREG,        .tid = CR_1,                .val = 1            },
    { .name = "C2",             .typ = TYP_CREG,        .tid = CR_2,                .val = 2            },
    { .name = "C3",             .typ = TYP_CREG,        .tid = CR_3,                .val = 3            },
    { .name = "C4",             .typ = TYP_CREG,        .tid = CR_4,                .val = 4            },
    { .name = "C5",             .typ = TYP_CREG,        .tid = CR_5,                .val = 5            },
    { .name = "C6",             .typ = TYP_CREG,        .tid = CR_6,                .val = 6            },
    { .name = "C7",             .typ = TYP_CREG,        .tid = CR_7,                .val = 7            },
    { .name = "C8",             .typ = TYP_CREG,        .tid = CR_8,                .val = 8            },
    { .name = "C9",             .typ = TYP_CREG,        .tid = CR_9,                .val = 9            },
    { .name = "C10",            .typ = TYP_CREG,        .tid = CR_10,               .val = 10           },
    { .name = "C11",            .typ = TYP_CREG,        .tid = CR_11,               .val = 11           },
    { .name = "C12",            .typ = TYP_CREG,        .tid = CR_12,               .val = 12           },
    { .name = "C13",            .typ = TYP_CREG,        .tid = CR_13,               .val = 13           },
    { .name = "C14",            .typ = TYP_CREG,        .tid = CR_14,               .val = 14           },
    { .name = "C15",            .typ = TYP_CREG,        .tid = CR_15,               .val = 15           },
    { .name = "C16",            .typ = TYP_CREG,        .tid = CR_16,               .val = 16           },
    { .name = "C17",            .typ = TYP_CREG,        .tid = CR_17,               .val = 17           },
    { .name = "C18",            .typ = TYP_CREG,        .tid = CR_18,               .val = 18           },
    { .name = "C19",            .typ = TYP_CREG,        .tid = CR_19,               .val = 19           },
    { .name = "C20",            .typ = TYP_CREG,        .tid = CR_20,               .val = 20           },
    { .name = "C21",            .typ = TYP_CREG,        .tid = CR_21,               .val = 21           },
    { .name = "C22",            .typ = TYP_CREG,        .tid = CR_22,               .val = 22           },
    { .name = "C23",            .typ = TYP_CREG,        .tid = CR_23,               .val = 23           },
    { .name = "C24",            .typ = TYP_CREG,        .tid = CR_24,               .val = 24           },
    { .name = "C25",            .typ = TYP_CREG,        .tid = CR_25,               .val = 25           },
    { .name = "C26",            .typ = TYP_CREG,        .tid = CR_26,               .val = 26           },
    { .name = "C27",            .typ = TYP_CREG,        .tid = CR_27,               .val = 27           },
    { .name = "C28",            .typ = TYP_CREG,        .tid = CR_28,               .val = 28           },
    { .name = "C29",            .typ = TYP_CREG,        .tid = CR_29,               .val = 29           },
    { .name = "C30",            .typ = TYP_CREG,        .tid = CR_30,               .val = 30           },
    { .name = "C31",            .typ = TYP_CREG,        .tid = CR_31,               .val = 31           },
        
    //--------------------------------------------------------------------------------------------------------
    // Assembler mnemonics.
    //
    //--------------------------------------------------------------------------------------------------------
    { .name = "LD",             .typ = TYP_OP_CODE,     .tid = OP_CODE_LD,          .val = 0xC0000000   },
    { .name = "LDB",            .typ = TYP_OP_CODE,     .tid = OP_CODE_LDB,         .val = 0xC0000000   },
    { .name = "LDH",            .typ = TYP_OP_CODE,     .tid = OP_CODE_LDH,         .val = 0xC0000000   },
    { .name = "LDW",            .typ = TYP_OP_CODE,     .tid = OP_CODE_LDW,         .val = 0xC0000000   },
    { .name = "LDR",            .typ = TYP_OP_CODE,     .tid = OP_CODE_LDR,         .val = 0xD0000000   },
    { .name = "LDA",            .typ = TYP_OP_CODE,     .tid = OP_CODE_LDA,         .val = 0xC8000000   },
        
    { .name = "ST",             .typ = TYP_OP_CODE,     .tid = OP_CODE_ST,          .val = 0xC4000000   },
    { .name = "STB",            .typ = TYP_OP_CODE,     .tid = OP_CODE_STB,         .val = 0xC4000000   },
    { .name = "STH",            .typ = TYP_OP_CODE,     .tid = OP_CODE_STH,         .val = 0xC4000000   },
    { .name = "STW",            .typ = TYP_OP_CODE,     .tid = OP_CODE_STW,         .val = 0xC4000000   },
    { .name = "STC",            .typ = TYP_OP_CODE,     .tid = OP_CODE_STC,         .val = 0xD4000000   },
    { .name = "STA",            .typ = TYP_OP_CODE,     .tid = OP_CODE_STA,         .val = 0xCC000000   },
        
    { .name = "ADD",            .typ = TYP_OP_CODE,     .tid = OP_CODE_ADD,         .val = 0x40000000   },
    { .name = "ADDB",           .typ = TYP_OP_CODE,     .tid = OP_CODE_ADDB,        .val = 0x40000000   },
    { .name = "ADDH",           .typ = TYP_OP_CODE,     .tid = OP_CODE_ADDH,        .val = 0x40000000   },
    { .name = "ADDW",           .typ = TYP_OP_CODE,     .tid = OP_CODE_ADDW,        .val = 0x40000000   },
        
    { .name = "ADC",            .typ = TYP_OP_CODE,     .tid = OP_CODE_ADC,         .val = 0x44000000   },
    { .name = "ADCB",           .typ = TYP_OP_CODE,     .tid = OP_CODE_ADCB,        .val = 0x44000000   },
    { .name = "ADCH",           .typ = TYP_OP_CODE,     .tid = OP_CODE_ADCH,        .val = 0x44000000   },
    { .name = "ADCW",           .typ = TYP_OP_CODE,     .tid = OP_CODE_ADCW,        .val = 0x44000000   },
        
    { .name = "SUB",            .typ = TYP_OP_CODE,     .tid = OP_CODE_SUB,         .val = 0x48000000   },
    { .name = "SUBB",           .typ = TYP_OP_CODE,     .tid = OP_CODE_SUBB,        .val = 0x48000000   },
    { .name = "SUBH",           .typ = TYP_OP_CODE,     .tid = OP_CODE_SUBH,        .val = 0x48000000   },
    { .name = "SUBW",           .typ = TYP_OP_CODE,     .tid = OP_CODE_SUBW,        .val = 0x48000000   },
        
    { .name = "SBC",            .typ = TYP_OP_CODE,     .tid = OP_CODE_SBC,         .val = 0x4C000000   },
    { .name = "SBCB",           .typ = TYP_OP_CODE,     .tid = OP_CODE_SBCB,        .val = 0x4C000000   },
    { .name = "SBCH",           .typ = TYP_OP_CODE,     .tid = OP_CODE_SBCH,        .val = 0x4C000000   },
    { .name = "SBCW",           .typ = TYP_OP_CODE,     .tid = OP_CODE_SBCW,        .val = 0x4C000000   },
        
    { .name = "AND",            .typ = TYP_OP_CODE,     .tid = OP_CODE_AND,         .val = 0x50000000   },
    { .name = "ANDB",           .typ = TYP_OP_CODE,     .tid = OP_CODE_ANDB,        .val = 0x50000000   },
    { .name = "ANDH",           .typ = TYP_OP_CODE,     .tid = OP_CODE_ANDH,        .val = 0x50000000   },
    { .name = "ANDW",           .typ = TYP_OP_CODE,     .tid = OP_CODE_ANDW,        .val = 0x50000000   },
        
    { .name = "OR" ,            .typ = TYP_OP_CODE,     .tid = OP_CODE_OR,          .val = 0x54000000   },
    { .name = "ORB",            .typ = TYP_OP_CODE,     .tid = OP_CODE_ORB,         .val = 0x54000000   },
    { .name = "ORH",            .typ = TYP_OP_CODE,     .tid = OP_CODE_ORH,         .val = 0x54000000   },
    { .name = "ORW",            .typ = TYP_OP_CODE,     .tid = OP_CODE_ORW,         .val = 0x54000000   },
        
    { .name = "XOR" ,           .typ = TYP_OP_CODE,     .tid = OP_CODE_XOR,         .val = 0x58000000   },
    { .name = "XORB",           .typ = TYP_OP_CODE,     .tid = OP_CODE_XORB,        .val = 0x58000000   },
    { .name = "XORH",           .typ = TYP_OP_CODE,     .tid = OP_CODE_XORH,        .val = 0x58000000   },
    { .name = "XORW",           .typ = TYP_OP_CODE,     .tid = OP_CODE_XORW,        .val = 0x58000000   },
        
    { .name = "CMP" ,           .typ = TYP_OP_CODE,     .tid = OP_CODE_CMP,         .val = 0x5C000000   },
    { .name = "CMPB",           .typ = TYP_OP_CODE,     .tid = OP_CODE_CMPB,        .val = 0x5C000000   },
    { .name = "CMPH",           .typ = TYP_OP_CODE,     .tid = OP_CODE_CMPH,        .val = 0x5C000000   },
    { .name = "CMPW",           .typ = TYP_OP_CODE,     .tid = OP_CODE_CMPW,        .val = 0x5C000000   },
        
    { .name = "CMPU" ,          .typ = TYP_OP_CODE,     .tid = OP_CODE_CMPU,        .val = 0x60000000   },
    { .name = "CMPUB",          .typ = TYP_OP_CODE,     .tid = OP_CODE_CMPUB,       .val = 0x60000000   },
    { .name = "CMPUH",          .typ = TYP_OP_CODE,     .tid = OP_CODE_CMPUH,       .val = 0x60000000   },
    { .name = "CMPUW",          .typ = TYP_OP_CODE,     .tid = OP_CODE_CMPUW,       .val = 0x60000000   },
    
    { .name = "LSID",           .typ = TYP_OP_CODE,     .tid = OP_CODE_LSID,        .val = 0x10000000   },
    { .name = "EXTR",           .typ = TYP_OP_CODE,     .tid = OP_CODE_EXTR,        .val = 0x14000000   },
    { .name = "DEP",            .typ = TYP_OP_CODE,     .tid = OP_CODE_DEP,         .val = 0x18000000   },
    { .name = "DSR",            .typ = TYP_OP_CODE,     .tid = OP_CODE_DSR,         .val = 0x1C000000   },
    { .name = "SHLA",           .typ = TYP_OP_CODE,     .tid = OP_CODE_SHLA,        .val = 0x20000000   },
    { .name = "CMR",            .typ = TYP_OP_CODE,     .tid = OP_CODE_CMR,         .val = 0x24000000   },
        
    { .name = "LDIL",           .typ = TYP_OP_CODE,     .tid = OP_CODE_LDIL,        .val = 0x04000000   },
    { .name = "ADDIL",          .typ = TYP_OP_CODE,     .tid = OP_CODE_ADDIL,       .val = 0x08000000   },
    { .name = "LDO",            .typ = TYP_OP_CODE,     .tid = OP_CODE_LDO,         .val = 0x0C000000   },
    
    { .name = "B",              .typ = TYP_OP_CODE,     .tid = OP_CODE_B,           .val = 0x80000000   },
    { .name = "GATE",           .typ = TYP_OP_CODE,     .tid = OP_CODE_GATE,        .val = 0x84000000   },
    { .name = "BR",             .typ = TYP_OP_CODE,     .tid = OP_CODE_BR,          .val = 0x88000000   },
    { .name = "BV",             .typ = TYP_OP_CODE,     .tid = OP_CODE_BV,          .val = 0x8C000000   },
    { .name = "BE",             .typ = TYP_OP_CODE,     .tid = OP_CODE_BE,          .val = 0x90000000   },
    { .name = "BVE",            .typ = TYP_OP_CODE,     .tid = OP_CODE_BVE,         .val = 0x94000000   },
    { .name = "CBR",            .typ = TYP_OP_CODE,     .tid = OP_CODE_CBR,         .val = 0x98000000   },
    { .name = "CBRU",           .typ = TYP_OP_CODE,     .tid = OP_CODE_CBRU,        .val = 0x9C000000   },
        
    { .name = "MR",             .typ = TYP_OP_CODE,     .tid = OP_CODE_MR,          .val = 0x28000000   },
    { .name = "MST",            .typ = TYP_OP_CODE,     .tid = OP_CODE_MST,         .val = 0x2C000000   },
    { .name = "DS",             .typ = TYP_OP_CODE,     .tid = OP_CODE_DS,          .val = 0x30000000   },
    { .name = "LDPA",           .typ = TYP_OP_CODE,     .tid = OP_CODE_LDPA,        .val = 0xE4000000   },
    { .name = "PRB",            .typ = TYP_OP_CODE,     .tid = OP_CODE_PRB,         .val = 0xE8000000   },
    { .name = "ITLB",           .typ = TYP_OP_CODE,     .tid = OP_CODE_ITLB,        .val = 0xEC000000   },
    { .name = "PTLB",           .typ = TYP_OP_CODE,     .tid = OP_CODE_PTLB,        .val = 0xF0000000   },
    { .name = "PCA",            .typ = TYP_OP_CODE,     .tid = OP_CODE_PCA,         .val = 0xF4000000   },
    { .name = "DIAG",           .typ = TYP_OP_CODE,     .tid = OP_CODE_DIAG,        .val = 0xF8000000   },
    { .name = "RFI",            .typ = TYP_OP_CODE,     .tid = OP_CODE_RFI,         .val = 0xFC000000   },
    { .name = "BRK",            .typ = TYP_OP_CODE,     .tid = OP_CODE_BRK,         .val = 0x00000000   },
        
    //--------------------------------------------------------------------------------------------------------
    // Synthetic instruction mnemonics.
    //
    //--------------------------------------------------------------------------------------------------------
    { .name = "NOP",            .typ = TYP_OP_CODE_S,   .tid = OP_CODE_S_NOP,       .val =  0           },
    { .name = "SHL",            .typ = TYP_OP_CODE_S,   .tid = OP_CODE_S_SHL,       .val =  0           },
    { .name = "SHR",            .typ = TYP_OP_CODE_S,   .tid = OP_CODE_S_SHR,       .val =  0           },
    { .name = "ASL",            .typ = TYP_OP_CODE_S,   .tid = OP_CODE_S_ASL,       .val =  0           },
    { .name = "ASR",            .typ = TYP_OP_CODE_S,   .tid = OP_CODE_S_ASR,       .val =  0           },
    { .name = "ROR",            .typ = TYP_OP_CODE_S,   .tid = OP_CODE_S_ROR,       .val =  0           },
    { .name = "ROL",            .typ = TYP_OP_CODE_S,   .tid = OP_CODE_S_ROL,       .val =  0           },
    
    //--------------------------------------------------------------------------------------------------------
    // Runtime register mnemonics.
    //
    //--------------------------------------------------------------------------------------------------------
    // ??? to fill in ...
    
    //--------------------------------------------------------------------------------------------------------
    // The last token to mark the list end.
    //--------------------------------------------------------------------------------------------------------
    { .name = "",               .typ = TYP_NIL,             .tid = TOK_LAST,           .val =  0       }
    
};

const int MAX_ASM_TOKEN_TAB = sizeof( asmTokTab ) / sizeof( DrvToken );


#endif  // VCPU32DriverTables_h
