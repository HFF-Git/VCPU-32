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
const SimToken cmdTokTab[ ] = {
    
    //--------------------------------------------------------------------------------------------------------
    // General tokens.
    //
    //--------------------------------------------------------------------------------------------------------
    { .name = "NIL",                .typ = TYP_SYM,                 .tid = TOK_NIL,         .val = 0        },
    
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
    //
    //
    //--------------------------------------------------------------------------------------------------------
    { .name = "COMMANDS",           .typ = TYP_CMD,                 .tid = CMD_SET                          },
    { .name = "WCOMMANDS",          .typ = TYP_WCMD,                .tid = WCMD_SET                         },
    { .name = "PREDEFINED",         .typ = TYP_PREDEFINED_FUNC,     .tid = PF_SET                           },
    { .name = "REGSET",             .typ = TYP_RSET,                .tid = REG_SET                          },
    { .name = "WTYPES",             .typ = TYP_WTYP,                .tid = WTYPE_SET                        },
    
    //--------------------------------------------------------------------------------------------------------
    // Command Line tokens.
    //
    //--------------------------------------------------------------------------------------------------------
    { .name = "HELP",               .typ = TYP_CMD,                 .tid = CMD_HELP                         },
    { .name = "?",                  .typ = TYP_CMD,                 .tid = CMD_HELP                         },
    
    { .name = "EXIT",               .typ = TYP_CMD,                 .tid = CMD_EXIT                         },
    { .name = "E",                  .typ = TYP_CMD,                 .tid = CMD_EXIT                         },
    
    { .name = "HIST",               .typ = TYP_CMD,                 .tid = CMD_HIST                         },
    { .name = "DO",                 .typ = TYP_CMD,                 .tid = CMD_DO                           },
    { .name = "REDO",               .typ = TYP_CMD,                 .tid = CMD_REDO                         },
    { .name = "ENV",                .typ = TYP_CMD,                 .tid = CMD_ENV                          },
    { .name = "XF",                 .typ = TYP_CMD,                 .tid = CMD_XF                           },
    { .name = "W",                  .typ = TYP_CMD,                 .tid = CMD_WRITE_LINE                   },
    
    { .name = "RESET",              .typ = TYP_CMD,                 .tid = CMD_RESET                        },
    { .name = "RUN",                .typ = TYP_CMD,                 .tid = CMD_RUN                          },
    { .name = "STEP",               .typ = TYP_CMD,                 .tid = CMD_STEP                         },
    { .name = "S",                  .typ = TYP_CMD,                 .tid = CMD_STEP                         },
    
    { .name = "DR",                 .typ = TYP_CMD,                 .tid = CMD_DR                           },
    { .name = "MR",                 .typ = TYP_CMD,                 .tid = CMD_MR                           },
    { .name = "DA",                 .typ = TYP_CMD,                 .tid = CMD_DA                           },
    { .name = "MA",                 .typ = TYP_CMD,                 .tid = CMD_MA                           },
    
    { .name = "ITLB",               .typ = TYP_CMD,                 .tid = CMD_I_TLB                        },
    { .name = "DTLB",               .typ = TYP_CMD,                 .tid = CMD_D_TLB                        },
    { .name = "PTLB",               .typ = TYP_CMD,                 .tid = CMD_P_TLB                        },
    
    { .name = "DCA",                .typ = TYP_CMD,                 .tid = CMD_D_CACHE                      },
    { .name = "PCA",                .typ = TYP_CMD,                 .tid = CMD_P_CACHE                      },
    
    //--------------------------------------------------------------------------------------------------------
    // Window command tokens.
    //
    //--------------------------------------------------------------------------------------------------------
    { .name = "WON",                .typ = TYP_WCMD,                .tid = CMD_WON                          },
    { .name = "WOFF",               .typ = TYP_WCMD,                .tid = CMD_WOFF                         },
    { .name = "WDEF",               .typ = TYP_WCMD,                .tid = CMD_WDEF                         },
    { .name = "WSE",                .typ = TYP_WCMD,                .tid = CMD_WSE                          },
    { .name = "WSD",                .typ = TYP_WCMD,                .tid = CMD_WSD                          },
    
    { .name = "PSE",                .typ = TYP_WCMD,                .tid = CMD_PSE                          },
    { .name = "PSD",                .typ = TYP_WCMD,                .tid = CMD_PSD                          },
    { .name = "PSR",                .typ = TYP_WCMD,                .tid = CMD_PSR                          },
    
    { .name = "SRE",                .typ = TYP_WCMD,                .tid = CMD_SRE                          },
    { .name = "SRD",                .typ = TYP_WCMD,                .tid = CMD_SRD                          },
    { .name = "SRR",                .typ = TYP_WCMD,                .tid = CMD_SRR                          },
    
    { .name = "PLE",                .typ = TYP_WCMD,                .tid = CMD_PLE                          },
    { .name = "PLD",                .typ = TYP_WCMD,                .tid = CMD_PLD                          },
    { .name = "PLR",                .typ = TYP_WCMD,                .tid = CMD_PLR                          },
    
    { .name = "SWE",                .typ = TYP_WCMD,                .tid = CMD_SWE                          },
    { .name = "SWD",                .typ = TYP_WCMD,                .tid = CMD_SWD                          },
    { .name = "SWR",                .typ = TYP_WCMD,                .tid = CMD_SWR                          },
    
    { .name = "CWL",                .typ = TYP_WCMD,                .tid = CMD_CWL                          },
    
    { .name = "WE",                 .typ = TYP_WCMD,                .tid = CMD_WE                           },
    { .name = "WD",                 .typ = TYP_WCMD,                .tid = CMD_WD                           },
    { .name = "WR",                 .typ = TYP_WCMD,                .tid = CMD_WR                           },
    { .name = "WF",                 .typ = TYP_WCMD,                .tid = CMD_WF                           },
    { .name = "WB",                 .typ = TYP_WCMD,                .tid = CMD_WB                           },
    { .name = "WH",                 .typ = TYP_WCMD,                .tid = CMD_WH                           },
    { .name = "WJ",                 .typ = TYP_WCMD,                .tid = CMD_WJ                           },
    { .name = "WL",                 .typ = TYP_WCMD,                .tid = CMD_WL                           },
    { .name = "WN",                 .typ = TYP_WCMD,                .tid = CMD_WN                           },
    { .name = "WK",                 .typ = TYP_WCMD,                .tid = CMD_WK                           },
    { .name = "WC",                 .typ = TYP_WCMD,                .tid = CMD_WC                           },
    { .name = "WS",                 .typ = TYP_WCMD,                .tid = CMD_WS                           },
    { .name = "WT",                 .typ = TYP_WCMD,                .tid = CMD_WT                           },
    { .name = "WX",                 .typ = TYP_WCMD,                .tid = CMD_WX                           },
    
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
    // Runtime architecture register names for general registers.
    //
    //--------------------------------------------------------------------------------------------------------
    { .name = "T0",                 .typ = TYP_GREG,                .tid = GR_1,            .val =  1       },
    { .name = "T1",                 .typ = TYP_GREG,                .tid = GR_2,            .val =  2       },
    { .name = "T2",                 .typ = TYP_GREG,                .tid = GR_3,            .val =  3       },
    { .name = "T3",                 .typ = TYP_GREG,                .tid = GR_4,            .val =  4       },
    { .name = "T4",                 .typ = TYP_GREG,                .tid = GR_5,            .val =  5       },
    { .name = "T5",                 .typ = TYP_GREG,                .tid = GR_6,            .val =  6       },
    { .name = "T6",                 .typ = TYP_GREG,                .tid = GR_7,            .val =  7       },

    { .name = "ARG3",               .typ = TYP_GREG,                .tid = GR_8,            .val =  8       },
    { .name = "ARG2",               .typ = TYP_GREG,                .tid = GR_9,            .val =  9       },
    { .name = "ARG1",               .typ = TYP_GREG,                .tid = GR_10,           .val =  10      },
    { .name = "ARG0",               .typ = TYP_GREG,                .tid = GR_11,           .val =  11      },

    { .name = "RET3",               .typ = TYP_GREG,                .tid = GR_8,            .val =  8       },
    { .name = "RET2",               .typ = TYP_GREG,                .tid = GR_9,            .val =  9       },
    { .name = "RET1",               .typ = TYP_GREG,                .tid = GR_10,           .val =  10      },
    { .name = "RET0",               .typ = TYP_GREG,                .tid = GR_11,           .val =  11      },
    
    { .name = "DP",                 .typ = TYP_GREG,                .tid = GR_13,           .val =  13      },
    { .name = "RL",                 .typ = TYP_GREG,                .tid = GR_14,           .val =  14      },
    { .name = "SP",                 .typ = TYP_GREG,                .tid = GR_15,           .val =  15      },
    
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
    // CPU Core register tokens.
    //
    //--------------------------------------------------------------------------------------------------------
    { .name = "FD_PSW0",            .typ = TYP_FD_PREG,         .tid = FD_PSW0,             .val = PSTAGE_REG_ID_PSW_0      },
    { .name = "FD_PSW1",            .typ = TYP_FD_PREG,         .tid = FD_PSW1,             .val = PSTAGE_REG_ID_PSW_1      },
  
    { .name = "PSW0",               .typ = TYP_FD_PREG,         .tid = FD_PSW0,             .val = PSTAGE_REG_ID_PSW_0      },
    { .name = "PSW1",               .typ = TYP_FD_PREG,         .tid = FD_PSW1,             .val = PSTAGE_REG_ID_PSW_1      },
    
    { .name = "MA_PSW0",            .typ = TYP_MA_PREG,         .tid = MA_PSW0,             .val = PSTAGE_REG_ID_PSW_0      },
    { .name = "MA_PSW1",            .typ = TYP_MA_PREG,         .tid = MA_PSW1,             .val = PSTAGE_REG_ID_PSW_0      },
    { .name = "MA_INSTR",           .typ = TYP_MA_PREG,         .tid = MA_INSTR,            .val = PSTAGE_REG_ID_INSTR      },
    { .name = "MA_A",               .typ = TYP_MA_PREG,         .tid = MA_A,                .val = PSTAGE_REG_ID_VAL_A      },
    { .name = "MA_B",               .typ = TYP_MA_PREG,         .tid = MA_B,                .val = PSTAGE_REG_ID_VAL_B      },
    { .name = "MA_X",               .typ = TYP_MA_PREG,         .tid = MA_X,                .val = PSTAGE_REG_ID_VAL_X      },
    { .name = "MA_S",               .typ = TYP_MA_PREG,         .tid = MA_S,                .val = PSTAGE_REG_ID_VAL_S      },
   
    { .name = "EX_PSW0",            .typ = TYP_EX_PREG,         .tid = EX_PSW0,             .val = PSTAGE_REG_ID_PSW_0      },
    { .name = "EX_PSW1",            .typ = TYP_EX_PREG,         .tid = EX_PSW1,             .val = PSTAGE_REG_ID_PSW_1      },
    { .name = "EX_INSTR",           .typ = TYP_EX_PREG,         .tid = EX_INSTR,            .val = PSTAGE_REG_ID_INSTR      },
    { .name = "EX_A",               .typ = TYP_EX_PREG,         .tid = EX_A,                .val = PSTAGE_REG_ID_VAL_A      },
    { .name = "EX_B",               .typ = TYP_EX_PREG,         .tid = EX_B,                .val = PSTAGE_REG_ID_VAL_B      },
    { .name = "EX_X",               .typ = TYP_EX_PREG,         .tid = EX_X,                .val = PSTAGE_REG_ID_VAL_X      },
    { .name = "EX_S",               .typ = TYP_EX_PREG,         .tid = EX_S,                .val = PSTAGE_REG_ID_VAL_S      },
    
    //--------------------------------------------------------------------------------------------------------
    // I-Cache register tokens.
    //
    //--------------------------------------------------------------------------------------------------------
    // ??? are the name lists for the registers complete ?????
    
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
    
    //--------------------------------------------------------------------------------------------------------
    // D-Cache register tokens.
    //
    //--------------------------------------------------------------------------------------------------------
    // ??? are the name lists for the registers complete ?????
    
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
    
    //--------------------------------------------------------------------------------------------------------
    // U-Cache register tokens.
    //
    //--------------------------------------------------------------------------------------------------------
    // ??? are the name lists for the registers complete ?????
    
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
    
    //--------------------------------------------------------------------------------------------------------
    // I-TLB register tokens.
    //
    //--------------------------------------------------------------------------------------------------------
    // ??? are the name lists for the registers complete ?????
    
    { .name = "ITLB_STATE",         .typ = TYP_ITLB_REG,        .tid = ITLB_STATE,          .val = 0        },
    { .name = "ITLB_REQ",           .typ = TYP_ITLB_REG,        .tid = ITLB_REQ,            .val = 1        },
    { .name = "ITLB_REQ_SEG",       .typ = TYP_ITLB_REG,        .tid = ITLB_REQ_SEG,        .val = 2        },
    { .name = "ITLB_REQ_OFS",       .typ = TYP_ITLB_REG,        .tid = ITLB_REQ_OFS,        .val = 3        },
    { .name = "ITLBL1",             .typ = TYP_ITLB_REG,        .tid = ITLB_SET,            .val = 4        },
    
    //--------------------------------------------------------------------------------------------------------
    // D-TLB register tokens.
    //
    //--------------------------------------------------------------------------------------------------------
    // ??? are the name lists for the registers complete ?????
    
    { .name = "DTLB_STATE",         .typ = TYP_DTLB_REG,        .tid = DTLB_STATE,          .val = 0        },
    { .name = "DTLB_REQ",           .typ = TYP_DTLB_REG,        .tid = DTLB_REQ,            .val = 1        },
    { .name = "DTLB_REQ_SEG",       .typ = TYP_DTLB_REG,        .tid = DTLB_REQ_SEG,        .val = 2        },
    { .name = "DTLB_REQ_OFS",       .typ = TYP_DTLB_REG,        .tid = DTLB_REQ_OFS,        .val = 3        },
    { .name = "DTLBL1",             .typ = TYP_DTLB_REG,        .tid = DTLB_SET,            .val = 4        },
    
    //--------------------------------------------------------------------------------------------------------
    // Predefined functions.
    //
    //--------------------------------------------------------------------------------------------------------
   
    { .name = "ASM",                .typ = TYP_PREDEFINED_FUNC, .tid = PF_ASSEMBLE,         .val = 0        },
    { .name = "DISASM",             .typ = TYP_PREDEFINED_FUNC, .tid = PF_DIS_ASSEMBLE,     .val = 0        },
    { .name = "HASH",               .typ = TYP_PREDEFINED_FUNC, .tid = PF_HASH,             .val = 0        },
    { .name = "ADR",                .typ = TYP_PREDEFINED_FUNC, .tid = PF_EXT_ADR,          .val = 0        },
    { .name = "S32",                .typ = TYP_PREDEFINED_FUNC, .tid = PF_S32,              .val = 0        },
    { .name = "U32",                .typ = TYP_PREDEFINED_FUNC, .tid = PF_U32,              .val = 0        }
    
};

const int MAX_CMD_TOKEN_TAB = sizeof( cmdTokTab ) / sizeof( SimToken );

//------------------------------------------------------------------------------------------------------------
// The error message table. Each entry has the error number and the corresponding error message text.
//
// ??? sort the entries... a little ...
//------------------------------------------------------------------------------------------------------------
const SimErrMsgTabEntry errMsgTab [ ] = {
    
    { .errNum = NO_ERR,                         .errStr = (char *) "NO_ERR" },
    { .errNum = ERR_NOT_SUPPORTED,              .errStr = (char *) "Command or Function not supported (yet)" },
    
    { .errNum = ERR_INVALID_CMD,                .errStr = (char *) "Invalid command, use help" },
    { .errNum = ERR_INVALID_CHAR_IN_TOKEN_LINE, .errStr = (char *) "Invalid char in input line" },
    { .errNum = ERR_INVALID_ARG,                .errStr = (char *) "Invalid argument for command" },
    { .errNum = ERR_INVALID_WIN_ID,             .errStr = (char *) "Invalid window Id" },
    { .errNum = ERR_INVALID_REG_ID,             .errStr = (char *) "Invalid register Id" },
    { .errNum = ERR_INVALID_RADIX,              .errStr = (char *) "Invalid radix" },
    { .errNum = ERR_INVALID_EXIT_VAL,           .errStr = (char *) "Invalid program exit code" },
    { .errNum = ERR_INVALID_WIN_STACK_ID,       .errStr = (char *) "Invalid window stack Id" },
    { .errNum = ERR_INVALID_STEP_OPTION,        .errStr = (char *) "Invalid steps/instr option" },
    { .errNum = ERR_INVALID_EXPR,               .errStr = (char *) "Invalid expression" },
    { .errNum = ERR_INVALID_INSTR_OPT,          .errStr = (char *) "Invalid instruction option" },
    { .errNum = ERR_INVALID_INSTR_MODE,         .errStr = (char *) "Invalid adr mode for instruction" },
    { .errNum = ERR_INVALID_REG_COMBO,          .errStr = (char *) "Invalid register combo for instruction" },
    { .errNum = ERR_INVALID_OP_CODE,            .errStr = (char *) "Invalid instruction opcode" },
    { .errNum = ERR_INVALID_S_OP_CODE,          .errStr = (char *) "Invalid synthetic instruction opcode" },
    { .errNum = ERR_INVALID_FMT_OPT,            .errStr = (char *) "Invalid format option" },
    { .errNum = ERR_INVALID_WIN_TYPE,           .errStr = (char *) "Invalid window type" },
    { .errNum = ERR_INVALID_CMD_ID,             .errStr = (char *) "Invalid command Id" },
    
    { .errNum = ERR_EXPECTED_INSTR_VAL,         .errStr = (char *) "Expected the instruction value" },
    { .errNum = ERR_EXPECTED_FILE_NAME,         .errStr = (char *) "Expected a file name" },
    { .errNum = ERR_EXPECTED_STACK_ID,          .errStr = (char *) "Expected stack Id" },
    { .errNum = ERR_EXPECTED_WIN_ID,            .errStr = (char *) "Expected a window Id" },
    { .errNum = ERR_EXPECTED_LPAREN,            .errStr = (char *) "Expected a left paren" },
    { .errNum = ERR_EXPECTED_RPAREN,            .errStr = (char *) "Expected a right paren" },
    { .errNum = ERR_EXPECTED_COMMA,             .errStr = (char *) "Expected a comma" },
    { .errNum = ERR_EXPECTED_STR,               .errStr = (char *) "Expected a string value" },
    { .errNum = ERR_EXPECTED_REG_SET,           .errStr = (char *) "Expected a register set" },
    { .errNum = ERR_EXPECTED_REG_OR_SET,        .errStr = (char *) "Expected a register or register set" },
    { .errNum = ERR_EXPECTED_NUMERIC,           .errStr = (char *) "Expected a numeric value" },
    { .errNum = ERR_EXPECTED_EXT_ADR,           .errStr = (char *) "Expected a virtual address" },
    { .errNum = ERR_EXPECTED_GENERAL_REG,       .errStr = (char *) "Expected a general reg" },
    { .errNum = ERR_EXPECTED_STEPS,             .errStr = (char *) "Expected number of steps/instr" },
    { .errNum = ERR_EXPECTED_START_OFS,         .errStr = (char *) "Expected start offset" },
    { .errNum = ERR_EXPECTED_LEN,               .errStr = (char *) "Expected length argument" },
    { .errNum = ERR_EXPECTED_OFS,               .errStr = (char *) "Expected an address" },
    { .errNum = ERR_EXPECTED_INSTR_OPT,         .errStr = (char *) "Expected the instruction options" },
    { .errNum = ERR_EXPECTED_SR1_SR3,           .errStr = (char *) "Expected SR1 .. SR3 as segment register" },
    { .errNum = ERR_EXPECTED_LOGICAL_ADR,       .errStr = (char *) "Expected a logical address" },
    { .errNum = ERR_EXPECTED_AN_OFFSET_VAL,     .errStr = (char *) "Expected an offset value" },
    { .errNum = ERR_EXPECTED_SEGMENT_REG,       .errStr = (char *) "Expected a segment register" },
    { .errNum = ERR_EXPECTED_FMT_OPT,           .errStr = (char *) "Expected a format option" },
    { .errNum = ERR_EXPECTED_WIN_TYPE,          .errStr = (char *) "Expected a window type" },
    { .errNum = ERR_EXPECTED_EXPR,              .errStr = (char *) "Expected an expression" },
   
    { .errNum = ERR_UNEXPECTED_EOS,             .errStr = (char *) "Unexpected end of command line" },
    { .errNum = ERR_NOT_IN_WIN_MODE,            .errStr = (char *) "Command only valid in Windows mode" },
    { .errNum = ERR_OPEN_EXEC_FILE,             .errStr = (char *) "Error while opening file" },
    { .errNum = ERR_EXTRA_TOKEN_IN_STR,         .errStr = (char *) "Extra tokens in command line" },
    { .errNum = ERR_ENV_VALUE_EXPR,             .errStr = (char *) "Invalid expression for ENV variable" },
    { .errNum = ERR_ENV_VAR_NOT_FOUND,          .errStr = (char *) "ENV variable not found" },
    { .errNum = ERR_WIN_TYPE_NOT_CONFIGURED,    .errStr = (char *) "Win object type not configured" },
    { .errNum = ERR_EXPR_TYPE_MATCH,            .errStr = (char *) "Expression type mismatch" },
    { .errNum = ERR_EXPR_FACTOR,                .errStr = (char *) "Expression error: factor" },
    { .errNum = ERR_TOO_MANY_ARGS_CMD_LINE,     .errStr = (char *) "Too many args in command line" },
    { .errNum = ERR_OFS_LEN_LIMIT_EXCEEDED,     .errStr = (char *) "Offset/Length exceeds limit" },
    { .errNum = ERR_UNDEFINED_PFUNC,            .errStr = (char *) "Unknown predefined function" },
    { .errNum = ERR_ENV_PREDEFINED,             .errStr = (char *) "ENV variable is predefined" },
    { .errNum = ERR_ENV_TABLE_FULL,             .errStr = (char *) "ENV Table is full" },
    { .errNum = ERR_INSTR_HAS_NO_OPT,           .errStr = (char *) "Instruction has no option" },
    { .errNum = ERR_IMM_VAL_RANGE,              .errStr = (char *) "Immediate value out of range" },
    { .errNum = ERR_INSTR_MODE_OPT_COMBO,       .errStr = (char *) "Invalid opCode data width specifier for mode option" },
    { .errNum = ERR_POS_VAL_RANGE,              .errStr = (char *) "Bit position value out of range" },
    { .errNum = ERR_LEN_VAL_RANGE,              .errStr = (char *) "Bit field length value out of range" },
    { .errNum = ERR_OFFSET_VAL_RANGE,           .errStr = (char *) "Offset value out of range" },
    { .errNum = ERR_OUT_OF_WINDOWS,             .errStr = (char *) "Cannot create more windows" },
        
    { .errNum = ERR_TLB_TYPE,                   .errStr = (char *) "Expected a TLB type" },
    { .errNum = ERR_TLB_INSERT_OP,              .errStr = (char *) "Insert in TLB operation error" },
    { .errNum = ERR_TLB_PURGE_OP,               .errStr = (char *) "Purge from TLB operation error" },
    { .errNum = ERR_TLB_ACC_DATA,               .errStr = (char *) "Invalid TLB insert access data" },
    { .errNum = ERR_TLB_ADR_DATA,               .errStr = (char *) "Invalid TLB insert address data" },
    { .errNum = ERR_TLB_NOT_CONFIGURED,         .errStr = (char *) "TLB type not configured" },
    { .errNum = ERR_TLB_SIZE_EXCEEDED,          .errStr = (char *) "TLB size exceeded" },
        
    { .errNum = ERR_CACHE_TYPE,                 .errStr = (char *) "Expected a cache type" },
    { .errNum = ERR_CACHE_PURGE_OP,             .errStr = (char *) "Purge from cache operation error" },
    { .errNum = ERR_CACHE_NOT_CONFIGURED,       .errStr = (char *) "Cache type not configured" },
    { .errNum = ERR_CACHE_SIZE_EXCEEDED,        .errStr = (char *) "Cache size exceeded" },
    { .errNum = ERR_CACHE_SET_NUM,              .errStr = (char *) "Invalid cache set" },
    
};

const int MAX_ERR_MSG_TAB = sizeof( errMsgTab ) / sizeof( SimErrMsgTabEntry );

//------------------------------------------------------------------------------------------------------------
// Help message text table. Each entry has a type field, a token field, a command syntax field and an
// explanation field.
//
//------------------------------------------------------------------------------------------------------------
const SimHelpMsgEntry cmdHelpTab[ ] = {
    
    //--------------------------------------------------------------------------------------------------------
    // Commands.
    //
    //--------------------------------------------------------------------------------------------------------
    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_HELP,
        .cmdNameStr     = (char *) "help",
        .cmdSyntaxStr   = (char *) "help ( cmdId | ‘commands‘ | 'wcommands‘ | ‘wtypes‘ | ‘predefined‘ )",
        .helpStr        = (char *) "list help information ( type \"help help\" for details )"
    },
  
    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_EXIT,
        .cmdNameStr     = (char *) "exit",
        .cmdSyntaxStr   = (char *) "exit (e) [ <val> ]",
        .helpStr        = (char *) "program exit"
    },
    
    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_HIST,
        .cmdNameStr     = (char *) "hist",
        .cmdSyntaxStr   = (char *) "hist [ depth ]",
        .helpStr        = (char *) "command history"
    },
    
    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_DO,
        .cmdNameStr     = (char *) "do",
        .cmdSyntaxStr   = (char *) "do [ cmdNum ]",
        .helpStr        = (char *) "re-execute command"
    },
    
    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_REDO,
        .cmdNameStr     = (char *) "redo",
        .cmdSyntaxStr   = (char *) "redo [ cmdNum ]",
        .helpStr        = (char *) "edit and then re-execute command"
    },
    
    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_ENV,
        .cmdNameStr     = (char *) "env",
        .cmdSyntaxStr   = (char *) "env [ <var> [ <val> ]]",
        .helpStr        = (char *) "lists the env tab, a variable, sets a variable"
    },
    
    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_XF,
        .cmdNameStr     = (char *) "xf",
        .cmdSyntaxStr   = (char *) "xf \"<filePath>\"",
        .helpStr        = (char *) "execute commands from a file"
    },
    
    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_RESET,
        .cmdNameStr     = (char *) "reset",
        .cmdSyntaxStr   = (char *) "reset ( 'CPU'|'MEM'|'STATS'|'ALL' )",
        .helpStr        = (char *) "resets the CPU"
    },
    
    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_RUN,
        .cmdNameStr     = (char *) "run",
        .cmdSyntaxStr   = (char *) "run",
        .helpStr        = (char *) "run the CPU"
    },
    
    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_STEP,
        .cmdNameStr     = (char *) "step",
        .cmdSyntaxStr   = (char *) "s [ <steps> ] [ , 'I' | 'C' ]",
        .helpStr        = (char *) "single step for instruction or clock cycle"
    },
    
    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_WRITE_LINE,
        .cmdNameStr     = (char *) "w",
        .cmdSyntaxStr   = (char *) "w <expr> [ , <rdx> ]",
        .helpStr        = (char *) "evaluates and prints an expression"
    },
    
    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_DR,
        .cmdNameStr     = (char *) "dr",
        .cmdSyntaxStr   = (char *) "dr [ ( <regSet>| <reg> ) ] [ , <fmt> ]",
        .helpStr        = (char *) "display register or register sets"
    },
    
    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_MR,
        .cmdNameStr     = (char *) "mr",
        .cmdSyntaxStr   = (char *) "mr <reg> <val>",
        .helpStr        = (char *) "modify registers"
    },
    
    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_DA,
        .cmdNameStr     = (char *) "da",
        .cmdSyntaxStr   = (char *) "da <ofs> [ , <len> ] [ , <fmt> ]",
        .helpStr        = (char *) "display absolute memory"
    },
    
    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_MA,
        .cmdNameStr     = (char *) "ma",
        .cmdSyntaxStr   = (char *) "ma <ofs> <val>",
        .helpStr        = (char *) "modify absolute memory"
    },
    
    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_D_CACHE,
        .cmdNameStr     = (char *) "dca",
        .cmdSyntaxStr   = (char *) "dca ( 'I' | 'D' | 'U' ) <index> [ , <len> [ , <fmt> ]] ",
        .helpStr        = (char *) "display cache content"
    },
    
    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_P_CACHE,
        .cmdNameStr     = (char *) "pca",
        .cmdSyntaxStr   = (char *) "pca ('I' | 'D' | 'U' ) <index> [ , <set> [, 'F' ]]",
        .helpStr        = (char *) "flushes and purges cache data"
    },
    
    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_D_TLB,
        .cmdNameStr     = (char *) "dtlb",
        .cmdSyntaxStr   = (char *) "dtlb ( 'I' | 'D' ) <index> [ , <len> [ , <rdx> ]]",
        .helpStr        = (char *) "display TLB content"
    },
    
    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_I_TLB,
        .cmdNameStr     = (char *) "itlb",
        .cmdSyntaxStr   = (char *) "itlb ( 'I' | 'D' ) <extAdr> <argAcc> <argAdr>",
        .helpStr        = (char *) "inserts an entry into the TLB"
    },
    
    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_P_TLB,
        .cmdNameStr     = (char *) "ptlb",
        .cmdSyntaxStr   = (char *) "ptlb ( 'I' | 'D' ) <extAdr>",
        .helpStr        = (char *) "purges an entry from the TLB"
    },
    
    {
        .helpTypeId = TYP_CMD,  .helpTokId  = CMD_WON,
        .cmdNameStr     = (char *) "won",
        .cmdSyntaxStr   = (char *) "won",
        .helpStr        = (char *) "switches to windows mode"
    },
    
    {
        .helpTypeId = TYP_RSET,  .helpTokId  = REG_SET,
        .cmdNameStr     = (char *) "regset",
        .cmdSyntaxStr   = (char *) "",
        .helpStr        = (char *) "\n" "\n"
                                    "GR   - " "general register set" "\n"
                                    "SR   - " "segment register set" "\n"
                                    "CR   - " "control register set" "\n"
                                    "PS   - " "program state" "\n"
                                    "PLFD - " "Pipeline FD stage input register set" "\n"
                                    "PLMA - " "Pipeline MA stage input register set" "\n"
                                    "PLEX - " "Pipeline EX stage input register set" "\n"
                                    "ICl1 - " "I-Cache control register set" "\n"
                                    "DCl1 - " "D-Cache control register set" "\n"
                                    "UCl2 - " "U-Cache control register set" "\n"
                                    "ITLB - " "I-TLB control register set" "\n"
                                    "DTLB - " "D-TLB control register set" "\n"
    },
    
    //--------------------------------------------------------------------------------------------------------
    // Window commands and types.
    //
    //--------------------------------------------------------------------------------------------------------
    {
        .helpTypeId = TYP_WCMD,  .helpTokId  = CMD_WOFF,
        .cmdNameStr     = (char *) "woff",
        .cmdSyntaxStr   = (char *) "woff",
        .helpStr        = (char *) "switches to command line mode"
    },
    
    {
        .helpTypeId = TYP_WCMD,  .helpTokId  = CMD_WDEF,
        .cmdNameStr     = (char *) "wdef",
        .cmdSyntaxStr   = (char *) "wdef",
        .helpStr        = (char *) "reset the windows to their default values"
    },
    
    {
        .helpTypeId = TYP_WCMD,  .helpTokId  = CMD_WSE,
        .cmdNameStr     = (char *) "wse",
        .cmdSyntaxStr   = (char *) "wse",
        .helpStr        = (char *) "enable window stacks"
    },
    
    {
        .helpTypeId = TYP_WCMD,  .helpTokId  = CMD_WSD,
        .cmdNameStr     = (char *) "wsd",
        .cmdSyntaxStr   = (char *) "wsd",
        .helpStr        = (char *) "disable window stacks"
    },
    
    {
        .helpTypeId     = TYP_WCMD, .helpTokId  = CMD_PSE,
        .cmdNameStr     = (char *)  "pse",
        .cmdSyntaxStr   = (char *)  "pse",
        .helpStr        = (char *)  "enable program status window display"
    },
    
    {
        .helpTypeId     = TYP_WCMD, .helpTokId  = CMD_SRE,
        .cmdNameStr     = (char *)  "sre",
        .cmdSyntaxStr   = (char *)  "sre",
        .helpStr        = (char *)  "enable special regs window display"
    },
    
    {
        .helpTypeId     = TYP_WCMD, .helpTokId  = CMD_PLE,
        .cmdNameStr     = (char *)  "ple",
        .cmdSyntaxStr   = (char *)  "ple",
        .helpStr        = (char *)  "enable pipeline regs window display"
    },
    
    {
        .helpTypeId     = TYP_WCMD, .helpTokId  = CMD_SWE,
        .cmdNameStr     = (char *)  "swe",
        .cmdSyntaxStr   = (char *)  "swe",
        .helpStr        = (char *)  "enable statistics window display"
    },
    
    {
        .helpTypeId     = TYP_WCMD, .helpTokId  = CMD_WE,
        .cmdNameStr     = (char *)  "we",
        .cmdSyntaxStr   = (char *)  "we [ <wNum> ]",
        .helpStr        = (char *)  "enable user defined window display"
    },
    
    {
        .helpTypeId     = TYP_WCMD, .helpTokId  = CMD_PSD,
        .cmdNameStr     = (char *)  "psd",
        .cmdSyntaxStr   = (char *)  "psd",
        .helpStr        = (char *)  "disable program status window display"
    },
    
    {
        .helpTypeId     = TYP_WCMD, .helpTokId  = CMD_SRD,
        .cmdNameStr     = (char *)  "srd",
        .cmdSyntaxStr   = (char *)  "srd",
        .helpStr        = (char *)  "disable special regs window display"
    },
    
    {
        .helpTypeId     = TYP_WCMD, .helpTokId  = CMD_PLD,
        .cmdNameStr     = (char *)  "pld",
        .cmdSyntaxStr   = (char *)  "pld",
        .helpStr        = (char *)  "disable pipeline regs window display"
    },
    
    {
        .helpTypeId     = TYP_WCMD, .helpTokId  = CMD_SWD,
        .cmdNameStr     = (char *)  "swd",
        .cmdSyntaxStr   = (char *)  "swd",
        .helpStr        = (char *)  "disable statistics window display"
    },
    
    {
        .helpTypeId     = TYP_WCMD, .helpTokId  = CMD_WD,
        .cmdNameStr     = (char *)  "wd",
        .cmdSyntaxStr   = (char *)  "wd [ <wNum> ]",
        .helpStr        = (char *)  "disable user defined windowf display"
    },
    
    {
        .helpTypeId     = TYP_WCMD, .helpTokId  = CMD_PSR,
        .cmdNameStr     = (char *)  "psr",
        .cmdSyntaxStr   = (char *)  "psr",
        .helpStr        = (char *)  "set program status window radix"
    },
    
    {
        .helpTypeId     = TYP_WCMD, .helpTokId  = CMD_SRR,
        .cmdNameStr     = (char *)  "srr",
        .cmdSyntaxStr   = (char *)  "srr",
        .helpStr        = (char *)  "set special regs window radix"
    },
    
    {
        .helpTypeId     = TYP_WCMD, .helpTokId  = CMD_PLR,
        .cmdNameStr     = (char *)  "plr",
        .cmdSyntaxStr   = (char *)  "plr",
        .helpStr        = (char *)  "set pipeline regs window radix"
    },
    
    {
        .helpTypeId     = TYP_WCMD, .helpTokId  = CMD_SWR,
        .cmdNameStr     = (char *)  "swr",
        .cmdSyntaxStr   = (char *)  "swr",
        .helpStr        = (char *)  "set statistics window radix"
    },
    
    {
        .helpTypeId     = TYP_WCMD, .helpTokId  = CMD_WR,
        .cmdNameStr     = (char *)  "wr",
        .cmdSyntaxStr   = (char *)  "wr [ <rdx> [ , <wNum> ]]",
        .helpStr        = (char *)  "set user defined window radix"
    },
    
    {
        .helpTypeId     = TYP_WCMD, .helpTokId  = CMD_WF,
        .cmdNameStr     = (char *)  "wf",
        .cmdSyntaxStr   = (char *)  "wf [ <amt> ] [ , <wNum> ]",
        .helpStr        = (char *)  "move backward by n items"
    },
    
    {
        .helpTypeId     = TYP_WCMD, .helpTokId  = CMD_WB,
        .cmdNameStr     = (char *)  "wb",
        .cmdSyntaxStr   = (char *)  "wb [ <amt> ] [ , <wNum> ]",
        .helpStr        = (char *)  "move backward by n items"
    },
    
    {
        .helpTypeId     = TYP_WCMD, .helpTokId  = CMD_WH,
        .cmdNameStr     = (char *)  "wh",
        .cmdSyntaxStr   = (char *)  "wh [ <pos> ] [ , <wNum> ]",
        .helpStr        = (char *)  "set window home position or set new home position"
    },
    
    {
        .helpTypeId     = TYP_WCMD, .helpTokId  = CMD_WJ,
        .cmdNameStr     = (char *)  "wj",
        .cmdSyntaxStr   = (char *)  "wj <pos> [ , <wNum> ]",
        .helpStr        = (char *)  "set window start to new position"
    },
    
    {
        .helpTypeId     = TYP_WCMD, .helpTokId  = CMD_WL,
        .cmdNameStr     = (char *)  "wl",
        .cmdSyntaxStr   = (char *)  "wl <lines> [ , <wNum> ]",
        .helpStr        = (char *)  "set window lines including banner line"
    },
    
    {
        .helpTypeId     = TYP_WCMD, .helpTokId  = CMD_WC,
        .cmdNameStr     = (char *)  "wc",
        .cmdSyntaxStr   = (char *)  "wc <wNum>",
        .helpStr        = (char *)  "set the window <wNum> as current window"
    },
    
    {
        .helpTypeId     = TYP_WCMD, .helpTokId  = CMD_WT,
        .cmdNameStr     = (char *)  "wt",
        .cmdSyntaxStr   = (char *)  "wt [ <wNum> ]",
        .helpStr        = (char *)  "toggle through alternate window content"
    },
    
    {
        .helpTypeId     = TYP_WCMD, .helpTokId  = CMD_WX,
        .cmdNameStr     = (char *)  "wx",
        .cmdSyntaxStr   = (char *)  "wx <wNum>",
        .helpStr        = (char *)  "exchange current window with this window"
    },
    
    {
        .helpTypeId     = TYP_WCMD, .helpTokId  = CMD_WN,
        .cmdNameStr     = (char *)  "wn",
        .cmdSyntaxStr   = (char *)  "wn <type> [ , <argStr> ]",
        .helpStr        = (char *)  "create a user defined window ( PM, PC, IT, DT, IC, ICR, DCR, MCR, TX )"
    },
        
    {
        .helpTypeId     = TYP_WTYP, .helpTokId  = WTYPE_SET,
        .cmdNameStr     = (char *)  "window types",
        .cmdSyntaxStr   = (char *)  "",
        .helpStr        = (char *)  "\n" "\n"
                                    "PS   - " "program state window" "\n"
                                    "SR   - " "special register window" "\n"
                                    "PL   - " "pipeline register window" "\n"
                                    "ST   - " "statistics window" "\n"
                                    "IT   - " "instruction tlb window" "\n"
                                    "DT   - " "data tlb window" "\n"
                                    "IC   - " "instruction cache window" "\n"
                                    "DC   - " "data cache window" "\n"
                                    "UC   - " "unified cache window" "\n"
                                    "PM   - " "physical memory window" "\n"
                                    "PC   - " "program code memory window" "\n"
                                    "TX   - " "text window" "\n"
                                    "CW   - " "command line window" "\n"
        
                                    "ICR  - " "instruction cache controller register window" "\n"
                                    "DCR  - " "data cache controller register window" "\n"
                                    "UCR  - " "unified cache controller register window" "\n"
                                    "MCR  - " "physical memory controller register window" "\n"
                                    "ITR  - " "instruction tlb controller register window" "\n"
                                    "DTR  - " "data tlb controller register window" "\n"
                                    "PCR  - " "PDC memory controller register window" "\n"
                                    "IOR  - " "IO memory controller register window" "\n"
    },
    
    {
        .helpTypeId     = TYP_WCMD, .helpTokId  = CMD_WK,
        .cmdNameStr     = (char *)  "wk",
        .cmdSyntaxStr   = (char *)  "wk <wStart> [ , <wEnd> ]",
        .helpStr        = (char *)  "remove a range of user defined windows"
    },
    
    {
        .helpTypeId     = TYP_WCMD, .helpTokId  = CMD_WS,
        .cmdNameStr     = (char *)  "ws",
        .cmdSyntaxStr   = (char *)  "ws <stackNum> [ , <wStart> ] [ , <wEnd>]",
        .helpStr        = (char *)  "moves a range of user windows into stack <stackNum>"
    },
    
    //--------------------------------------------------------------------------------------------------------
    // Predefined Functions.
    //
    //--------------------------------------------------------------------------------------------------------
    {
        .helpTypeId = TYP_PREDEFINED_FUNC,  .helpTokId  = PF_S32,
        .cmdNameStr     = (char *) "s32",
        .cmdSyntaxStr   = (char *) "s32 ( <expr> )",
        .helpStr        = (char *) "coerces an expression to a signed 32-bit value"
    },
    
    {
        .helpTypeId = TYP_PREDEFINED_FUNC,  .helpTokId  = PF_U32,
        .cmdNameStr     = (char *) "u32",
        .cmdSyntaxStr   = (char *) "u32 ( <expr> )",
        .helpStr        = (char *) "coerces an expression to a unsigned 32-bit value"
    },
    
    {
        .helpTypeId = TYP_PREDEFINED_FUNC,  .helpTokId  = PF_HASH,
        .cmdNameStr     = (char *) "hash",
        .cmdSyntaxStr   = (char *) "hash ( <extAdr> )",
        .helpStr        = (char *) "returns the hash value of a virtual address"
    },
    
    {
        .helpTypeId = TYP_PREDEFINED_FUNC,  .helpTokId  = PF_EXT_ADR,
        .cmdNameStr     = (char *) "adr",
        .cmdSyntaxStr   = (char *) "adr ( <extAdr> | <adr> | <sr, gr> ) | <gr> )",
        .helpStr        = (char *) "returns a virtual address"
    },
    
    {
        .helpTypeId = TYP_PREDEFINED_FUNC,  .helpTokId  = PF_ASSEMBLE,
        .cmdNameStr     = (char *) "asm",
        .cmdSyntaxStr   = (char *) "asm ( <asmStr> )",
        .helpStr        = (char *) "returns the instruction value for an assemble string"
    },
    
    {
        .helpTypeId = TYP_PREDEFINED_FUNC,  .helpTokId  = PF_DIS_ASSEMBLE,
        .cmdNameStr     = (char *) "disasm",
        .cmdSyntaxStr   = (char *) "disasm ( <instr> )",
        .helpStr        = (char *) "returns the assemble string for an instruction value"
    }
};

const int MAX_CMD_HELP_TAB = sizeof( cmdHelpTab ) / sizeof( SimHelpMsgEntry );

//------------------------------------------------------------------------------------------------------------
// The global token table or the one line assembler. All reserved words are allocated in this table. Each
// entry has the token name, the token id, the token type id, i.e. its type, and a value associated with the
// token. The value allows for a constant token. The parser can directly use the value in expressions.
//
//------------------------------------------------------------------------------------------------------------
const SimToken asmTokTab[ ] = {
    
    //--------------------------------------------------------------------------------------------------------
    // General registers.
    //
    //--------------------------------------------------------------------------------------------------------
    { .name = "R0",             .typ = TYP_GREG,            .tid = GR_0,                .val = 0           },
    { .name = "R1",             .typ = TYP_GREG,            .tid = GR_1,                .val = 1           },
    { .name = "R2",             .typ = TYP_GREG,            .tid = GR_2,                .val = 2           },
    { .name = "R3",             .typ = TYP_GREG,            .tid = GR_3,                .val = 3           },
    { .name = "R4",             .typ = TYP_GREG,            .tid = GR_4,                .val = 4           },
    { .name = "R5",             .typ = TYP_GREG,            .tid = GR_5,                .val = 5           },
    { .name = "R6",             .typ = TYP_GREG,            .tid = GR_6,                .val = 6           },
    { .name = "R7",             .typ = TYP_GREG,            .tid = GR_7,                .val = 7           },
    { .name = "R8",             .typ = TYP_GREG,            .tid = GR_8,                .val = 8           },
    { .name = "R9",             .typ = TYP_GREG,            .tid = GR_9,                .val = 9           },
    { .name = "R10",            .typ = TYP_GREG,            .tid = GR_10,               .val = 10          },
    { .name = "R11",            .typ = TYP_GREG,            .tid = GR_11,               .val = 11          },
    { .name = "R12",            .typ = TYP_GREG,            .tid = GR_12,               .val = 12          },
    { .name = "R13",            .typ = TYP_GREG,            .tid = GR_13,               .val = 13          },
    { .name = "R14",            .typ = TYP_GREG,            .tid = GR_14,               .val = 14          },
    { .name = "R15",            .typ = TYP_GREG,            .tid = GR_15,               .val = 15          },

    //--------------------------------------------------------------------------------------------------------
    // Runtime architcture register names for general registers.
    //
    //--------------------------------------------------------------------------------------------------------
    { .name = "T0",             .typ = TYP_GREG,            .tid = GR_1,                .val =  1       },
    { .name = "T1",             .typ = TYP_GREG,            .tid = GR_2,                .val =  2       },
    { .name = "T2",             .typ = TYP_GREG,            .tid = GR_3,                .val =  3       },
    { .name = "T3",             .typ = TYP_GREG,            .tid = GR_4,                .val =  4       },
    { .name = "T4",             .typ = TYP_GREG,            .tid = GR_5,                .val =  5       },
    { .name = "T5",             .typ = TYP_GREG,            .tid = GR_6,                .val =  6       },
    { .name = "T6",             .typ = TYP_GREG,            .tid = GR_7,                .val =  7       },

    { .name = "ARG3",           .typ = TYP_GREG,            .tid = GR_8,                .val =  8       },
    { .name = "ARG2",           .typ = TYP_GREG,            .tid = GR_9,                .val =  9       },
    { .name = "ARG1",           .typ = TYP_GREG,            .tid = GR_10,               .val =  10      },
    { .name = "ARG0",           .typ = TYP_GREG,            .tid = GR_11,               .val =  11      },

    { .name = "RET3",           .typ = TYP_GREG,            .tid = GR_8,                .val =  8       },
    { .name = "RET2",           .typ = TYP_GREG,            .tid = GR_9,                .val =  9       },
    { .name = "RET1",           .typ = TYP_GREG,            .tid = GR_10,               .val =  10      },
    { .name = "RET0",           .typ = TYP_GREG,            .tid = GR_11,               .val =  11      },
    
    { .name = "DP",             .typ = TYP_GREG,            .tid = GR_13,               .val =  13      },
    { .name = "RL",             .typ = TYP_GREG,            .tid = GR_14,               .val =  14      },
    { .name = "SP",             .typ = TYP_GREG,            .tid = GR_15,               .val =  15      },
    
    //--------------------------------------------------------------------------------------------------------
    // Segment registers.
    //
    //--------------------------------------------------------------------------------------------------------
    { .name = "S0",             .typ = TYP_SREG,            .tid = SR_0,                .val = 0           },
    { .name = "S1",             .typ = TYP_SREG,            .tid = SR_1,                .val = 1           },
    { .name = "S2",             .typ = TYP_SREG,            .tid = SR_2,                .val = 2           },
    { .name = "S3",             .typ = TYP_SREG,            .tid = SR_3,                .val = 3           },
    { .name = "S4",             .typ = TYP_SREG,            .tid = SR_4,                .val = 4           },
    { .name = "S5",             .typ = TYP_SREG,            .tid = SR_5,                .val = 5           },
    { .name = "S6",             .typ = TYP_SREG,            .tid = SR_6,                .val = 6           },
    { .name = "S7",             .typ = TYP_SREG,            .tid = SR_7,                .val = 7           },
    
    //--------------------------------------------------------------------------------------------------------
    // Control registers.
    //
    //--------------------------------------------------------------------------------------------------------
    { .name = "C0",             .typ = TYP_CREG,            .tid = CR_0,                .val = 0            },
    { .name = "C1",             .typ = TYP_CREG,            .tid = CR_1,                .val = 1            },
    { .name = "C2",             .typ = TYP_CREG,            .tid = CR_2,                .val = 2            },
    { .name = "C3",             .typ = TYP_CREG,            .tid = CR_3,                .val = 3            },
    { .name = "C4",             .typ = TYP_CREG,            .tid = CR_4,                .val = 4            },
    { .name = "C5",             .typ = TYP_CREG,            .tid = CR_5,                .val = 5            },
    { .name = "C6",             .typ = TYP_CREG,            .tid = CR_6,                .val = 6            },
    { .name = "C7",             .typ = TYP_CREG,            .tid = CR_7,                .val = 7            },
    { .name = "C8",             .typ = TYP_CREG,            .tid = CR_8,                .val = 8            },
    { .name = "C9",             .typ = TYP_CREG,            .tid = CR_9,                .val = 9            },
    { .name = "C10",            .typ = TYP_CREG,            .tid = CR_10,               .val = 10           },
    { .name = "C11",            .typ = TYP_CREG,            .tid = CR_11,               .val = 11           },
    { .name = "C12",            .typ = TYP_CREG,            .tid = CR_12,               .val = 12           },
    { .name = "C13",            .typ = TYP_CREG,            .tid = CR_13,               .val = 13           },
    { .name = "C14",            .typ = TYP_CREG,            .tid = CR_14,               .val = 14           },
    { .name = "C15",            .typ = TYP_CREG,            .tid = CR_15,               .val = 15           },
    { .name = "C16",            .typ = TYP_CREG,            .tid = CR_16,               .val = 16           },
    { .name = "C17",            .typ = TYP_CREG,            .tid = CR_17,               .val = 17           },
    { .name = "C18",            .typ = TYP_CREG,            .tid = CR_18,               .val = 18           },
    { .name = "C19",            .typ = TYP_CREG,            .tid = CR_19,               .val = 19           },
    { .name = "C20",            .typ = TYP_CREG,            .tid = CR_20,               .val = 20           },
    { .name = "C21",            .typ = TYP_CREG,            .tid = CR_21,               .val = 21           },
    { .name = "C22",            .typ = TYP_CREG,            .tid = CR_22,               .val = 22           },
    { .name = "C23",            .typ = TYP_CREG,            .tid = CR_23,               .val = 23           },
    { .name = "C24",            .typ = TYP_CREG,            .tid = CR_24,               .val = 24           },
    { .name = "C25",            .typ = TYP_CREG,            .tid = CR_25,               .val = 25           },
    { .name = "C26",            .typ = TYP_CREG,            .tid = CR_26,               .val = 26           },
    { .name = "C27",            .typ = TYP_CREG,            .tid = CR_27,               .val = 27           },
    { .name = "C28",            .typ = TYP_CREG,            .tid = CR_28,               .val = 28           },
    { .name = "C29",            .typ = TYP_CREG,            .tid = CR_29,               .val = 29           },
    { .name = "C30",            .typ = TYP_CREG,            .tid = CR_30,               .val = 30           },
    { .name = "C31",            .typ = TYP_CREG,            .tid = CR_31,               .val = 31           },
    
    //--------------------------------------------------------------------------------------------------------
    // Assembler mnemonics.
    //
    //--------------------------------------------------------------------------------------------------------
    { .name = "LD",             .typ = TYP_OP_CODE,         .tid = OP_CODE_LD,          .val = 0xC0000000   },
    { .name = "LDB",            .typ = TYP_OP_CODE,         .tid = OP_CODE_LDB,         .val = 0xC0000000   },
    { .name = "LDH",            .typ = TYP_OP_CODE,         .tid = OP_CODE_LDH,         .val = 0xC0000000   },
    { .name = "LDW",            .typ = TYP_OP_CODE,         .tid = OP_CODE_LDW,         .val = 0xC0000000   },
    { .name = "LDR",            .typ = TYP_OP_CODE,         .tid = OP_CODE_LDR,         .val = 0xD0000000   },
    { .name = "LDA",            .typ = TYP_OP_CODE,         .tid = OP_CODE_LDA,         .val = 0xC8000000   },
    
    { .name = "ST",             .typ = TYP_OP_CODE,         .tid = OP_CODE_ST,          .val = 0xC4000000   },
    { .name = "STB",            .typ = TYP_OP_CODE,         .tid = OP_CODE_STB,         .val = 0xC4000000   },
    { .name = "STH",            .typ = TYP_OP_CODE,         .tid = OP_CODE_STH,         .val = 0xC4000000   },
    { .name = "STW",            .typ = TYP_OP_CODE,         .tid = OP_CODE_STW,         .val = 0xC4000000   },
    { .name = "STC",            .typ = TYP_OP_CODE,         .tid = OP_CODE_STC,         .val = 0xD4000000   },
    { .name = "STA",            .typ = TYP_OP_CODE,         .tid = OP_CODE_STA,         .val = 0xCC000000   },
    
    { .name = "ADD",            .typ = TYP_OP_CODE,         .tid = OP_CODE_ADD,         .val = 0x40000000   },
    { .name = "ADDB",           .typ = TYP_OP_CODE,         .tid = OP_CODE_ADDB,        .val = 0x40000000   },
    { .name = "ADDH",           .typ = TYP_OP_CODE,         .tid = OP_CODE_ADDH,        .val = 0x40000000   },
    { .name = "ADDW",           .typ = TYP_OP_CODE,         .tid = OP_CODE_ADDW,        .val = 0x40000000   },
    
    { .name = "ADC",            .typ = TYP_OP_CODE,         .tid = OP_CODE_ADC,         .val = 0x44000000   },
    { .name = "ADCB",           .typ = TYP_OP_CODE,         .tid = OP_CODE_ADCB,        .val = 0x44000000   },
    { .name = "ADCH",           .typ = TYP_OP_CODE,         .tid = OP_CODE_ADCH,        .val = 0x44000000   },
    { .name = "ADCW",           .typ = TYP_OP_CODE,         .tid = OP_CODE_ADCW,        .val = 0x44000000   },
    
    { .name = "SUB",            .typ = TYP_OP_CODE,         .tid = OP_CODE_SUB,         .val = 0x48000000   },
    { .name = "SUBB",           .typ = TYP_OP_CODE,         .tid = OP_CODE_SUBB,        .val = 0x48000000   },
    { .name = "SUBH",           .typ = TYP_OP_CODE,         .tid = OP_CODE_SUBH,        .val = 0x48000000   },
    { .name = "SUBW",           .typ = TYP_OP_CODE,         .tid = OP_CODE_SUBW,        .val = 0x48000000   },
    
    { .name = "SBC",            .typ = TYP_OP_CODE,         .tid = OP_CODE_SBC,         .val = 0x4C000000   },
    { .name = "SBCB",           .typ = TYP_OP_CODE,         .tid = OP_CODE_SBCB,        .val = 0x4C000000   },
    { .name = "SBCH",           .typ = TYP_OP_CODE,         .tid = OP_CODE_SBCH,        .val = 0x4C000000   },
    { .name = "SBCW",           .typ = TYP_OP_CODE,         .tid = OP_CODE_SBCW,        .val = 0x4C000000   },
    
    { .name = "AND",            .typ = TYP_OP_CODE,         .tid = OP_CODE_AND,         .val = 0x50000000   },
    { .name = "ANDB",           .typ = TYP_OP_CODE,         .tid = OP_CODE_ANDB,        .val = 0x50000000   },
    { .name = "ANDH",           .typ = TYP_OP_CODE,         .tid = OP_CODE_ANDH,        .val = 0x50000000   },
    { .name = "ANDW",           .typ = TYP_OP_CODE,         .tid = OP_CODE_ANDW,        .val = 0x50000000   },
    
    { .name = "OR" ,            .typ = TYP_OP_CODE,         .tid = OP_CODE_OR,          .val = 0x54000000   },
    { .name = "ORB",            .typ = TYP_OP_CODE,         .tid = OP_CODE_ORB,         .val = 0x54000000   },
    { .name = "ORH",            .typ = TYP_OP_CODE,         .tid = OP_CODE_ORH,         .val = 0x54000000   },
    { .name = "ORW",            .typ = TYP_OP_CODE,         .tid = OP_CODE_ORW,         .val = 0x54000000   },
    
    { .name = "XOR" ,           .typ = TYP_OP_CODE,         .tid = OP_CODE_XOR,         .val = 0x58000000   },
    { .name = "XORB",           .typ = TYP_OP_CODE,         .tid = OP_CODE_XORB,        .val = 0x58000000   },
    { .name = "XORH",           .typ = TYP_OP_CODE,         .tid = OP_CODE_XORH,        .val = 0x58000000   },
    { .name = "XORW",           .typ = TYP_OP_CODE,         .tid = OP_CODE_XORW,        .val = 0x58000000   },
    
    { .name = "CMP" ,           .typ = TYP_OP_CODE,         .tid = OP_CODE_CMP,         .val = 0x5C000000   },
    { .name = "CMPB",           .typ = TYP_OP_CODE,         .tid = OP_CODE_CMPB,        .val = 0x5C000000   },
    { .name = "CMPH",           .typ = TYP_OP_CODE,         .tid = OP_CODE_CMPH,        .val = 0x5C000000   },
    { .name = "CMPW",           .typ = TYP_OP_CODE,         .tid = OP_CODE_CMPW,        .val = 0x5C000000   },
    
    { .name = "CMPU" ,          .typ = TYP_OP_CODE,         .tid = OP_CODE_CMPU,        .val = 0x60000000   },
    { .name = "CMPUB",          .typ = TYP_OP_CODE,         .tid = OP_CODE_CMPUB,       .val = 0x60000000   },
    { .name = "CMPUH",          .typ = TYP_OP_CODE,         .tid = OP_CODE_CMPUH,       .val = 0x60000000   },
    { .name = "CMPUW",          .typ = TYP_OP_CODE,         .tid = OP_CODE_CMPUW,       .val = 0x60000000   },
    
    { .name = "LSID",           .typ = TYP_OP_CODE,         .tid = OP_CODE_LSID,        .val = 0x10000000   },
    { .name = "EXTR",           .typ = TYP_OP_CODE,         .tid = OP_CODE_EXTR,        .val = 0x14000000   },
    { .name = "DEP",            .typ = TYP_OP_CODE,         .tid = OP_CODE_DEP,         .val = 0x18000000   },
    { .name = "DSR",            .typ = TYP_OP_CODE,         .tid = OP_CODE_DSR,         .val = 0x1C000000   },
    { .name = "SHLA",           .typ = TYP_OP_CODE,         .tid = OP_CODE_SHLA,        .val = 0x20000000   },
    { .name = "CMR",            .typ = TYP_OP_CODE,         .tid = OP_CODE_CMR,         .val = 0x24000000   },
    
    { .name = "LDIL",           .typ = TYP_OP_CODE,         .tid = OP_CODE_LDIL,        .val = 0x04000000   },
    { .name = "ADDIL",          .typ = TYP_OP_CODE,         .tid = OP_CODE_ADDIL,       .val = 0x08000000   },
    { .name = "LDO",            .typ = TYP_OP_CODE,         .tid = OP_CODE_LDO,         .val = 0x0C000000   },
    
    { .name = "B",              .typ = TYP_OP_CODE,         .tid = OP_CODE_B,           .val = 0x80000000   },
    { .name = "GATE",           .typ = TYP_OP_CODE,         .tid = OP_CODE_GATE,        .val = 0x84000000   },
    { .name = "BR",             .typ = TYP_OP_CODE,         .tid = OP_CODE_BR,          .val = 0x88000000   },
    { .name = "BV",             .typ = TYP_OP_CODE,         .tid = OP_CODE_BV,          .val = 0x8C000000   },
    { .name = "BE",             .typ = TYP_OP_CODE,         .tid = OP_CODE_BE,          .val = 0x90000000   },
    { .name = "BVE",            .typ = TYP_OP_CODE,         .tid = OP_CODE_BVE,         .val = 0x94000000   },
    { .name = "CBR",            .typ = TYP_OP_CODE,         .tid = OP_CODE_CBR,         .val = 0x98000000   },
    { .name = "CBRU",           .typ = TYP_OP_CODE,         .tid = OP_CODE_CBRU,        .val = 0x9C000000   },
    
    { .name = "MR",             .typ = TYP_OP_CODE,         .tid = OP_CODE_MR,          .val = 0x28000000   },
    { .name = "MST",            .typ = TYP_OP_CODE,         .tid = OP_CODE_MST,         .val = 0x2C000000   },
    { .name = "DS",             .typ = TYP_OP_CODE,         .tid = OP_CODE_DS,          .val = 0x30000000   },
    { .name = "LDPA",           .typ = TYP_OP_CODE,         .tid = OP_CODE_LDPA,        .val = 0xE4000000   },
    { .name = "PRB",            .typ = TYP_OP_CODE,         .tid = OP_CODE_PRB,         .val = 0xE8000000   },
    { .name = "ITLB",           .typ = TYP_OP_CODE,         .tid = OP_CODE_ITLB,        .val = 0xEC000000   },
    { .name = "PTLB",           .typ = TYP_OP_CODE,         .tid = OP_CODE_PTLB,        .val = 0xF0000000   },
    { .name = "PCA",            .typ = TYP_OP_CODE,         .tid = OP_CODE_PCA,         .val = 0xF4000000   },
    { .name = "DIAG",           .typ = TYP_OP_CODE,         .tid = OP_CODE_DIAG,        .val = 0xF8000000   },
    { .name = "RFI",            .typ = TYP_OP_CODE,         .tid = OP_CODE_RFI,         .val = 0xFC000000   },
    { .name = "BRK",            .typ = TYP_OP_CODE,         .tid = OP_CODE_BRK,         .val = 0x00000000   },

    //--------------------------------------------------------------------------------------------------------
    // Synthetic instruction mnemonics.
    //
    //--------------------------------------------------------------------------------------------------------
    { .name = "NOP",            .typ = TYP_OP_CODE_S,       .tid = OP_CODE_S_NOP,       .val =  0           },
    { .name = "SHL",            .typ = TYP_OP_CODE_S,       .tid = OP_CODE_S_SHL,       .val =  0           },
    { .name = "SHR",            .typ = TYP_OP_CODE_S,       .tid = OP_CODE_S_SHR,       .val =  0           },
    { .name = "ASL",            .typ = TYP_OP_CODE_S,       .tid = OP_CODE_S_ASL,       .val =  0           },
    { .name = "ASR",            .typ = TYP_OP_CODE_S,       .tid = OP_CODE_S_ASR,       .val =  0           },
    { .name = "ROR",            .typ = TYP_OP_CODE_S,       .tid = OP_CODE_S_ROR,       .val =  0           },
    { .name = "ROL",            .typ = TYP_OP_CODE_S,       .tid = OP_CODE_S_ROL,       .val =  0           }
     
};

const int MAX_ASM_TOKEN_TAB = sizeof( asmTokTab ) / sizeof( SimToken );

#endif  // VCPU32DriverTables_h
