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
// Idea:
//
// It turns out that a better command line parser would be a more powerful way to analyze a command line.
// We have commands that just execute a command and functions that return a value. When we have a parser
// we could implement such functions as arguments to the commands. Commands them selves may just be just a
// function with a void return.
//
//      <command>   ->  <cmdId> [ <argList> ]
//      <function>  ->  <funcId> “(“ [ <argList> ] ")"
//      <argList>   ->  <expr> { <expr> }
//
// Expression have a type, which are NUM, ADR, STR, SREG, GREG and CREG.
//
//      <factor> -> <number>                        |
//                  <string>                        |
//                  <envId>                         |
//                  <gregId>                        |
//                  <sregId>                        |
//                  <cregId>                        |
//                  "~" <factor>                    |
//                  "(" [ <sreg> "," ] <greg> ")"   |
//                  "(" <expr> ")"
//
//      <term>      ->  <factor> { <termOp> <factor> }
//      <termOp>    ->  "*" | "/" | "%" | "&"
//
//      <expr>      ->  [ ( "+" | "-" ) ] <term> { <exprOp> <term> }
//      <exprOp>    ->  "+" | "-" | "|" | "^"
//
// If a command is called, there is no output another than what the command was issuing itself.
// If a function is called in the command place, the function result will be printed.
// If an argument represents a function, its return value will be the argument in the command.
//
// The token table becomes a kind of dictionary with name, type and values.
// The environment table needs to enhanced to allow for user defined variables.
//
//------------------------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------------------------
// Local name space. We try to keep utility functions local to the file.
//
//------------------------------------------------------------------------------------------------------------
namespace {



// ??? phase out when the all commands are fully converted to the new parser...
//------------------------------------------------------------------------------------------------------------
// Token table. There is a large number of reserved tokens. Each token has a name and an optional alias name.
// Each token also belongs to a group, which allows to do a faster match during command line parsing. The
// table is searched for all kind of names, such as command names, registers names, option names and so on.
//
//------------------------------------------------------------------------------------------------------------
const int   TOK_NAME_SIZE       = 32;
const int   TOK_ALIAS_NAME_SIZE = 8;
const int   TOK_LARGE_STR_SIZE  = 256;
const int   PATH_STR_SIZE       = 256;

// ??? simplify, take out alias and make the own token tab entries...
struct {
    
    char    name[ TOK_NAME_SIZE ];
    char    aliasName[ TOK_ALIAS_NAME_SIZE ];
    TokId   tokGrpId;
    TokId   tokId;
    
} const tokTab[ ] = {
    
    { "ENV",                "",         TOK_TYP_CMD,            CMD_ENV                 },
    { "EXIT",               "E",        TOK_TYP_CMD,            CMD_EXIT                },
    { "HELP",               "?",        TOK_TYP_CMD,            CMD_HELP                },
    { "WHELP",              "",         TOK_TYP_CMD,            CMD_WHELP               },
    { "RESET",              "",         TOK_TYP_CMD,            CMD_RESET               },
    { "RUN",                "",         TOK_TYP_CMD,            CMD_RUN                 },
    { "STEP",               "S",        TOK_TYP_CMD,            CMD_STEP                },
    { "DIS",                "",         TOK_TYP_CMD,            CMD_DIS_ASM             },
    { "ASM",                "",         TOK_TYP_CMD,            CMD_ASM                 },
    { "EXEC-F",             "XF",       TOK_TYP_CMD,            CMD_XF                  },
    
    { "D-REG",              "DR",       TOK_TYP_CMD,            CMD_DR                  },
    { "M-REG",              "MR",       TOK_TYP_CMD,            CMD_MR                  },
    
    { "HASH-VA",            "HVA",      TOK_TYP_CMD,            CMD_HASH_VA             },
    
    { "I-TLB",              "ITLB",     TOK_TYP_CMD,            CMD_I_TLB               },
    { "D-TLB",              "DTLB",     TOK_TYP_CMD,            CMD_D_TLB               },
    { "P-TLB",              "PTLB",     TOK_TYP_CMD,            CMD_P_TLB               },
    
    { "D-CACHE",            "DCA",      TOK_TYP_CMD,            CMD_D_CACHE             },
    { "P-CACHE",            "PCA",      TOK_TYP_CMD,            CMD_P_CACHE             },
    
    { "D-ABS",              "DA",       TOK_TYP_CMD,            CMD_DA                  },
    { "M-ABS",              "MA",       TOK_TYP_CMD,            CMD_MA                  },
    { "D-ABS-ASM",          "DAA",      TOK_TYP_CMD,            CMD_DAA                 },
    { "M-ABS-ASM",          "MAA",      TOK_TYP_CMD,            CMD_MAA                 },
    
    { "LOAD-MEM",           "LMF",      TOK_TYP_CMD,            CMD_LMF                 },
    { "SAVE-MEM",           "SMF",      TOK_TYP_CMD,            CMD_SMF                 },
    
    { "WON",                "",         TOK_TYP_CMD,            CMD_WON                 },
    { "WOFF",               "",         TOK_TYP_CMD,            CMD_WOFF                },
    { "WDEF",               "",         TOK_TYP_CMD,            CMD_WDEF                },
    { "WSE",                "",         TOK_TYP_CMD,            CMD_WSE                 },
    { "WSD",                "",         TOK_TYP_CMD,            CMD_WSD                 },
    
    { "PSE",                "",         TOK_TYP_CMD,            CMD_PSE                 },
    { "PSD",                "",         TOK_TYP_CMD,            CMD_PSD                 },
    { "PSR",                "",         TOK_TYP_CMD,            CMD_PSR                 },
    
    { "SRE",                "",         TOK_TYP_CMD,            CMD_SRE                 },
    { "SRD",                "",         TOK_TYP_CMD,            CMD_SRD                 },
    { "SRR",                "",         TOK_TYP_CMD,            CMD_SRR                 },
    
    { "PLE",                "",         TOK_TYP_CMD,            CMD_PLE                 },
    { "PLD",                "",         TOK_TYP_CMD,            CMD_PLD                 },
    { "PLR",                "",         TOK_TYP_CMD,            CMD_PLR                 },
    
    { "SWE",                "",         TOK_TYP_CMD,            CMD_SWE                 },
    { "SWD",                "",         TOK_TYP_CMD,            CMD_SWD                 },
    { "SWR",                "",         TOK_TYP_CMD,            CMD_SWR                 },
    
    { "CWL",                "",         TOK_TYP_CMD,            CMD_CWL                 },
    
    { "WE",                 "",         TOK_TYP_CMD,            CMD_WE                  },
    { "WD",                 "",         TOK_TYP_CMD,            CMD_WD                  },
    { "WR",                 "",         TOK_TYP_CMD,            CMD_WR                  },
    { "WF",                 "",         TOK_TYP_CMD,            CMD_WF                  },
    { "WB",                 "",         TOK_TYP_CMD,            CMD_WB                  },
    { "WH",                 "",         TOK_TYP_CMD,            CMD_WH                  },
    { "WJ",                 "",         TOK_TYP_CMD,            CMD_WJ                  },
    { "WL",                 "",         TOK_TYP_CMD,            CMD_WL                  },
    { "WN",                 "",         TOK_TYP_CMD,            CMD_WN                  },
    { "WK",                 "",         TOK_TYP_CMD,            CMD_WK                  },
    { "WC",                 "",         TOK_TYP_CMD,            CMD_WC                  },
    { "WS",                 "",         TOK_TYP_CMD,            CMD_WS                  },
    { "WT",                 "",         TOK_TYP_CMD,            CMD_WT                  },
    { "WX",                 "",         TOK_TYP_CMD,            CMD_WX                  },
    
    { "TRUE",               "",         TOK_TYP_NIL,            TOK_TRUE                },
    { "FALSE",              "",         TOK_TYP_NIL,            TOK_FALSE               },
    { "ALL",                "",         TOK_TYP_NIL,            TOK_ALL                 },
    { "CPU",                "",         TOK_TYP_NIL,            TOK_CPU                 },
    { "MEM",                "",         TOK_TYP_NIL,            TOK_MEM                 },
    { "C",                  "",         TOK_TYP_NIL,            TOK_C                   },
    { "D",                  "",         TOK_TYP_NIL,            TOK_D                   },
    { "F",                  "",         TOK_TYP_NIL,            TOK_F                   },
    { "I",                  "",         TOK_TYP_NIL,            TOK_I                   },
    { "T",                  "",         TOK_TYP_NIL,            TOK_T                   },
    { "U",                  "",         TOK_TYP_NIL,            TOK_U                   },
    
    { "DEC",                "",         FMT_SET,            TOK_DEC                 },
    { "HEX",                "",         FMT_SET,            TOK_HEX                 },
    { "OCT",                "",         FMT_SET,            TOK_OCT                 },
    
    { "PM",                 "",         TOK_TYP_NIL,            TOK_PM                  },
    { "PC",                 "",         TOK_TYP_NIL,            TOK_PC                  },
    { "IT",                 "",         TOK_TYP_NIL,            TOK_IT                  },
    { "DT",                 "",         TOK_TYP_NIL,            TOK_DT                  },
    { "IC",                 "",         TOK_TYP_NIL,            TOK_IC                  },
    { "DC",                 "",         TOK_TYP_NIL,            TOK_DC                  },
    { "UC",                 "",         TOK_TYP_NIL,            TOK_UC                  },
    { "ICR",                "",         TOK_TYP_NIL,            TOK_ICR                 },
    { "DCR",                "",         TOK_TYP_NIL,            TOK_DCR                 },
    { "UCR",                "",         TOK_TYP_NIL,            TOK_UCR                 },
    { "MCR",                "",         TOK_TYP_NIL,            TOK_MCR                 },
    { "ITR",                "",         TOK_TYP_NIL,            TOK_ITR                 },
    { "DTR",                "",         TOK_TYP_NIL,            TOK_DTR                 },
    { "PCR",                "",         TOK_TYP_NIL,            TOK_PCR                 },
    { "IOR",                "",         TOK_TYP_NIL,            TOK_IOR                 },
    { "TX",                 "",         TOK_TYP_NIL,            TOK_TX                  },
    
    { "R0",                 "TMP",      TOK_TYP_GREG,             GR_0                    },
    { "R1",                 "T1",       TOK_TYP_GREG,             GR_1                    },
    { "R2",                 "T2",       TOK_TYP_GREG,             GR_2                    },
    { "R3",                 "T3",       TOK_TYP_GREG,             GR_3                    },
    { "R4",                 "T4",       TOK_TYP_GREG,             GR_4                    },
    { "R5",                 "",         TOK_TYP_GREG,             GR_5                    },
    { "R6",                 "",         TOK_TYP_GREG,             GR_6                    },
    { "R7",                 "",         TOK_TYP_GREG,             GR_7                    },
    { "R8",                 "",         TOK_TYP_GREG,             GR_8                    },
    { "R9",                 "",         TOK_TYP_GREG,             GR_9                    },
    { "R10",                "",         TOK_TYP_GREG,             GR_10                   },
    { "R11",                "",         TOK_TYP_GREG,             GR_11                   },
    { "R12",                "",         TOK_TYP_GREG,             GR_12                   },
    { "R13",                "DP",       TOK_TYP_GREG,             GR_13                   },
    { "R14",                "RL",       TOK_TYP_GREG,             GR_14                   },
    { "R15",                "SP",       TOK_TYP_GREG,             GR_15                   },
    
    { "S0",                 "",         TOK_TYP_SREG,             SR_0                    },
    { "S1",                 "",         TOK_TYP_SREG,             SR_1                    },
    { "S2",                 "",         TOK_TYP_SREG,             SR_2                    },
    { "S3",                 "",         TOK_TYP_SREG,             SR_3                    },
    { "S4",                 "",         TOK_TYP_SREG,             SR_4                    },
    { "S5",                 "",         TOK_TYP_SREG,             SR_5                    },
    { "S6",                 "",         TOK_TYP_SREG,             SR_6                    },
    { "S7",                 "",         TOK_TYP_SREG,             SR_7                    },
    
    { "C0",                 "",         TOK_TYP_CREG,             CR_0                    },
    { "C1",                 "",         TOK_TYP_CREG,             CR_1                    },
    { "C2",                 "",         TOK_TYP_CREG,             CR_2                    },
    { "C3",                 "",         TOK_TYP_CREG,             CR_3                    },
    { "C4",                 "",         TOK_TYP_CREG,             CR_4                    },
    { "C5",                 "",         TOK_TYP_CREG,             CR_5                    },
    { "C6",                 "",         TOK_TYP_CREG,             CR_6                    },
    { "C7",                 "",         TOK_TYP_CREG,             CR_7                    },
    { "C8",                 "",         TOK_TYP_CREG,             CR_8                    },
    { "C9",                 "",         TOK_TYP_CREG,             CR_9                    },
    { "C10",                "",         TOK_TYP_CREG,             CR_10                   },
    { "C11",                "",         TOK_TYP_CREG,             CR_11                   },
    { "C12",                "",         TOK_TYP_CREG,             CR_12                   },
    { "C13",                "",         TOK_TYP_CREG,             CR_13                   },
    { "C14",                "",         TOK_TYP_CREG,             CR_14                   },
    { "C15",                "",         TOK_TYP_CREG,             CR_15                   },
    { "C16",                "",         TOK_TYP_CREG,             CR_16                   },
    { "C17",                "",         TOK_TYP_CREG,             CR_17                   },
    { "C18",                "",         TOK_TYP_CREG,             CR_18                   },
    { "C19",                "",         TOK_TYP_CREG,             CR_19                   },
    { "C20",                "",         TOK_TYP_CREG,             CR_20                   },
    { "C21",                "",         TOK_TYP_CREG,             CR_21                   },
    { "C22",                "",         TOK_TYP_CREG,             CR_22                   },
    { "C23",                "",         TOK_TYP_CREG,             CR_23                   },
    { "C24",                "TMP-0",    TOK_TYP_CREG,             CR_24                   },
    { "C25",                "TMP-1",    TOK_TYP_CREG,             CR_25                   },
    { "C26",                "TMP-2",    TOK_TYP_CREG,             CR_26                   },
    { "C27",                "TMP-3",    TOK_TYP_CREG,             CR_27                   },
    { "C28",                "TMP-4",    TOK_TYP_CREG,             CR_28                   },
    { "C29",                "TMP-5",    TOK_TYP_CREG,               CR_29                   },
    { "C30",                "TMP-6",    TOK_TYP_CREG,               CR_30                   },
    { "C31",                "TMP-7",    TOK_TYP_CREG,               CR_31                   },
    
    { "IA-SEG",             "",         TOK_TYP_PSTATE_PREG,        PS_IA_SEG               },
    { "IA-OFS",             "",         TOK_TYP_PSTATE_PREG,        PS_IA_OFS               },
    { "ST-REG",             "",         TOK_TYP_PSTATE_PREG,        PS_STATUS               },
    
    { "FD-IA-SEG",          "",         TOK_TYP_FD_PREG,             FD_IA_SEG               },
    { "FD-IA-OFS",          "",         TOK_TYP_FD_PREG,             FD_IA_OFS               },
    { "FD-INSTR",           "",         TOK_TYP_FD_PREG,             FD_INSTR                },
    { "FD-A",               "",         TOK_TYP_FD_PREG,             FD_A                    },
    { "FD-B",               "",         TOK_TYP_FD_PREG,             FD_B                    },
    { "FD-X",               "",         TOK_TYP_FD_PREG,             FD_X                    },
    
    { "MA-IA-SEG",          "",         TOK_TYP_OF_PREG,             MA_IA_SEG               },
    { "MA-IA-OFS",          "",         TOK_TYP_OF_PREG,             MA_IA_OFS               },
    { "MA-INSTR",           "",         TOK_TYP_OF_PREG,             MA_INSTR                },
    { "MA-A",               "",         TOK_TYP_OF_PREG,             MA_A                    },
    { "MA-B",               "",         TOK_TYP_OF_PREG,             MA_B                    },
    { "MA-X",               "",         TOK_TYP_OF_PREG,             MA_X                    },
    { "MA-S",               "",         TOK_TYP_OF_PREG,             MA_S                    },
    
    { "IC-L1-STATE",        "",         TOK_TYP_IC_L1_REG,          IC_L1_STATE             },
    { "IC-L1-REQ",          "",         TOK_TYP_IC_L1_REG,          IC_L1_REQ               },
    { "IC-L1-REQ-SEG",      "",         TOK_TYP_IC_L1_REG,          IC_L1_REQ_SEG           },
    { "IC-L1-REQ-OFS",      "",         TOK_TYP_IC_L1_REG,          IC_L1_REQ_OFS           },
    { "IC-L1-REQ-TAG",      "",         TOK_TYP_IC_L1_REG,          IC_L1_REQ_TAG           },
    { "IC-L1-REQ-LEN",      "",         TOK_TYP_IC_L1_REG,          IC_L1_REQ_LEN           },
    { "IC-L1-REQ-LAT",      "",         TOK_TYP_IC_L1_REG,          IC_L1_LATENCY           },
    { "IC-L1-SETS",         "",         TOK_TYP_IC_L1_REG,          IC_L1_SETS              },
    { "IC-L1-ENTRIES",      "",         TOK_TYP_IC_L1_REG,          IC_L1_BLOCK_ENTRIES     },
    { "IC-L1-B-SIZE",       "",         TOK_TYP_IC_L1_REG,          IC_L1_BLOCK_SIZE        },
    
    { "DC-L1-STATE",        "",         TOK_TYP_DC_L1_REG,          DC_L1_STATE             },
    { "DC-L1-REQ",          "",         TOK_TYP_DC_L1_REG,          DC_L1_REQ               },
    { "DC-L1-REQ-SEG",      "",         TOK_TYP_DC_L1_REG,          DC_L1_REQ_SEG           },
    { "DC-L1-REQ-OFS",      "",         TOK_TYP_DC_L1_REG,          DC_L1_REQ_OFS           },
    { "DC-L1-REQ-TAG",      "",         TOK_TYP_DC_L1_REG,          DC_L1_REQ_TAG           },
    { "DC-L1-REQ-LEN",      "",         TOK_TYP_DC_L1_REG,          DC_L1_REQ_LEN           },
    { "DC-L1-REQ-LAT",      "",         TOK_TYP_DC_L1_REG,          DC_L1_LATENCY           },
    { "DC-L1-SETS",         "",         TOK_TYP_DC_L1_REG,          DC_L1_SETS              },
    { "DC-L1-ENTRIES",      "",         TOK_TYP_DC_L1_REG,          DC_L1_BLOCK_ENTRIES     },
    { "DC-L1-B-SIZE",       "",         TOK_TYP_DC_L1_REG,          DC_L1_BLOCK_SIZE        },
    
    { "UC-L2-STATE",        "",         TOK_TYP_UC_L2_REG,          UC_L2_STATE             },
    { "UC-L2-REQ",          "",         TOK_TYP_UC_L2_REG,          UC_L2_REQ               },
    { "UC-L2-REQ-SEG",      "",         TOK_TYP_UC_L2_REG,          UC_L2_REQ_SEG           },
    { "UC-L2-REQ-OFS",      "",         TOK_TYP_UC_L2_REG,          UC_L2_REQ_OFS           },
    { "UC-L2-REQ-TAG",      "",         TOK_TYP_UC_L2_REG,          UC_L2_REQ_TAG           },
    { "UC-L2-REQ-LEN",      "",         TOK_TYP_UC_L2_REG,          UC_L2_REQ_LEN           },
    { "UC-L2-REQ-LAT",      "",         TOK_TYP_UC_L2_REG,          UC_L2_LATENCY           },
    { "UC-L2-SETS",         "",         TOK_TYP_UC_L2_REG,          UC_L2_SETS              },
    { "UC-L2-ENTRIES",      "",         TOK_TYP_UC_L2_REG,          UC_L2_BLOCK_ENTRIES     },
    { "UC-L2-B-SIZE",       "",         TOK_TYP_UC_L2_REG,          UC_L2_BLOCK_SIZE        },
    
    { "ITLB-STATE",         "",         TOK_TYP_ITLB_REG,           ITLB_STATE              },
    { "ITLB-REQ",           "",         TOK_TYP_ITLB_REG,           ITLB_REQ                },
    { "ITLB-REQ-SEG",       "",         TOK_TYP_ITLB_REG,           ITLB_REQ_SEG            },
    { "ITLB-REQ-OFS",       "",         TOK_TYP_ITLB_REG,           ITLB_REQ_OFS            },
    
    { "DTLB-STATE",         "",         TOK_TYP_DTLB_REG,           DTLB_STATE              },
    { "DTLB-REQ",           "",         TOK_TYP_DTLB_REG,           DTLB_REQ                },
    { "DTLB-REQ-SEG",       "",         TOK_TYP_DTLB_REG,           DTLB_REQ_SEG            },
    { "DTLB-REQ-OFS",       "",         TOK_TYP_DTLB_REG,           DTLB_REQ_OFS            },
    
    { "GR-SET",             "GR",       REG_SET,            TOK_TYP_GREG                  },
    { "SR-SET",             "SR",       REG_SET,            TOK_TYP_SREG                  },
    { "CR-SET",             "CR",       REG_SET,            TOK_TYP_CREG                  },
    { "PS-SET",             "PS",       REG_SET,            TOK_TYP_PSTATE_PREG                  },
    { "PR-SET",             "PR",       REG_SET,            PR_SET                  },
    { "FD-SET",             "PR",       REG_SET,            TOK_TYP_FD_PREG                  },
    { "MA-SET",             "PR",       REG_SET,            TOK_TYP_OF_PREG                  },
    { "IC-L1-SET",          "ICL1",     REG_SET,            TOK_TYP_IC_L1_REG               },
    { "DC-L1-SET",          "DCL1",     REG_SET,            TOK_TYP_DC_L1_REG               },
    { "UC-L2-SET",          "UCl2",     REG_SET,            TOK_TYP_UC_L2_REG               },
    { "ITLB-SET",           "ITRS",     REG_SET,            TOK_TYP_ITLB_REG                },
    { "DTLB-SET",           "DTRS",     REG_SET,            TOK_TYP_DTLB_REG                },
    
    { "REG-SET-ALL",        "RS",       REG_SET,            REG_SET_ALL             }
    
    
};

const int   TOK_TAB_SIZE  = sizeof( tokTab ) / sizeof( *tokTab );








// ??? the new table .....
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
    { .name = "TRUE",               .typ = TYP_BOOL,                .tid = TOK_TRUE,                 1       },
    { .name = "FALSE",              .typ = TYP_BOOL,                .tid = TOK_FALSE,                0       },
    
    { .name = "ALL",                .typ = TYP_NIL,                 .tid = TOK_ALL                           },
    { .name = "CPU",                .typ = TYP_NIL,                 .tid = TOK_CPU                           },
    { .name = "MEM",                .typ = TYP_NIL,                 .tid = TOK_MEM                           },
    { .name = "C",                  .typ = TYP_NIL,                 .tid = TOK_C                             },
    { .name = "D",                  .typ = TYP_NIL,                 .tid = TOK_D                             },
    { .name = "F",                  .typ = TYP_NIL,                 .tid = TOK_F                             },
    { .name = "I",                  .typ = TYP_NIL,                 .tid = TOK_I                             },
    { .name = "T",                  .typ = TYP_NIL,                 .tid = TOK_T                             },
    { .name = "U",                  .typ = TYP_NIL,                 .tid = TOK_U                             },
    
    { .name = "DEC",                .typ = TYP_NIL,                 .tid = TOK_DEC,                 10      },
    { .name = "DECIMAL",            .typ = TYP_NIL,                 .tid = TOK_DEC,                 10      },
    { .name = "HEX",                .typ = TYP_NIL,                 .tid = TOK_HEX,                 16      },
    { .name = "OCT",                .typ = TYP_NIL,                 .tid = TOK_OCT,                 8       },
    { .name = "OCTAL",              .typ = TYP_NIL,                 .tid = TOK_OCT,                 8       },
    
    
    //--------------------------------------------------------------------------------------------------------
    // Command Line tokens.
    //
    //--------------------------------------------------------------------------------------------------------
    { .name = "ENV",                .typ = TYP_CMD,                 .tid = CMD_ENV                              },
    
    { .name = "EXIT",               .typ = TYP_CMD,                 .tid = CMD_EXIT             },
    { .name = "E",                  .typ = TYP_CMD,                 .tid = CMD_EXIT            },
    { .name = "HELP",               .typ = TYP_CMD,                 .tid = CMD_HELP              },
    { .name = "?",                  .typ = TYP_CMD,                 .tid = CMD_HELP             },
    { .name = "WHELP",              .typ = TYP_CMD,                 .tid = CMD_WHELP              },
    
    { .name = "RESET",              .typ = TYP_CMD,                 .tid = CMD_RESET              },
    { .name = "RUN",                .typ = TYP_CMD,                 .tid = CMD_RUN               },
    { .name = "STEP",               .typ = TYP_CMD,                 .tid = CMD_STEP             },
    { .name = "S",                  .typ = TYP_CMD,                 .tid = CMD_STEP              },
    
    { .name = "DIS",                .typ = TYP_CMD,                 .tid = CMD_DIS_ASM             },
    { .name = "ASM",                .typ = TYP_CMD,                 .tid = CMD_ASM            },
    
    { .name = "XF",                 .typ = TYP_CMD,                 .tid = CMD_XF              },
    
    { .name = "DR",                 .typ = TYP_CMD,                 .tid = CMD_DR               },
    { .name = "MR",                 .typ = TYP_CMD,                 .tid = CMD_MR              },
    
    { .name = "DA",                 .typ = TYP_CMD,                 .tid = CMD_DA              },
    { .name = "MA",                 .typ = TYP_CMD,                 .tid = CMD_MA              },
    
    { .name = "DAA",                .typ = TYP_CMD,                 .tid = CMD_DAA         },
    { .name = "MAA",                .typ = TYP_CMD,                 .tid = CMD_MAA              },
   
    { .name = "ITLB",               .typ = TYP_CMD,                 .tid = CMD_I_TLB            },
    { .name = "DTLB",               .typ = TYP_CMD,                 .tid = CMD_D_TLB               },
    { .name = "PTLB",               .typ = TYP_CMD,                 .tid = CMD_P_TLB            },
    
    { .name = "DCA",                .typ = TYP_CMD,                 .tid = CMD_D_CACHE           },
    { .name = "PCA",                .typ = TYP_CMD,                 .tid = CMD_P_CACHE               },
    
    { .name = "HVA",                .typ = TYP_CMD,                 .tid = CMD_HASH_VA              },
    
    { .name = "LOAD_MEM",           .typ = TYP_CMD,                 .tid = CMD_LMF           },
    { .name = "SAVE_MEM",           .typ = TYP_CMD,                 .tid = CMD_SMF             },
    
    //--------------------------------------------------------------------------------------------------------
    // Window command tokens.
    //
    //--------------------------------------------------------------------------------------------------------
    { .name = "WON",                .typ = TYP_CMD,                 .tid = CMD_WON                           },
    { .name = "WOFF",               .typ = TYP_CMD,                 .tid = CMD_WOFF                          },
    { .name = "WDEF",               .typ = TYP_CMD,                 .tid = CMD_WDEF               },
    { .name = "WSE",                .typ = TYP_CMD,                 .tid = CMD_WSE              },
    { .name = "WSD",                .typ = TYP_CMD,                 .tid = CMD_WSD              },
    
    { .name = "PSE",                .typ = TYP_CMD,                 .tid = CMD_PSE              },
    { .name = "PSD",                .typ = TYP_CMD,                 .tid = CMD_PSD              },
    { .name = "PSR",                .typ = TYP_CMD,                 .tid = CMD_PSR              },
    
    { .name = "SRE",                .typ = TYP_CMD,                 .tid = CMD_SRE              },
    { .name = "SRD",                .typ = TYP_CMD,                 .tid = CMD_SRE         },
    { .name = "SRR",                .typ = TYP_CMD,                 .tid = CMD_SRR              },
    
    { .name = "PLE",                .typ = TYP_CMD,                 .tid = CMD_PLE               },
    { .name = "PLD",                .typ = TYP_CMD,                 .tid = CMD_PLD               },
    { .name = "PLR",                .typ = TYP_CMD,                 .tid = CMD_PLR              },
    
    { .name = "SWE",                .typ = TYP_CMD,                 .tid = CMD_SWE             },
    { .name = "SWD",                .typ = TYP_CMD,                 .tid = CMD_SWD             },
    { .name = "SWR",                .typ = TYP_CMD,                 .tid = CMD_SWR            },
    
    { .name = "CWL",                .typ = TYP_CMD,                 .tid = CMD_CWL              },
    
    { .name = "WE",                 .typ = TYP_CMD,                 .tid = CMD_WE            },
    { .name = "WD",                 .typ = TYP_CMD,                 .tid = CMD_WD             },
    { .name = "WR",                 .typ = TYP_CMD,                 .tid = CMD_WR               },
    { .name = "WF",                 .typ = TYP_CMD,                 .tid = CMD_WF             },
    { .name = "WB",                 .typ = TYP_CMD,                 .tid = CMD_WB          },
    { .name = "WH",                 .typ = TYP_CMD,                 .tid = CMD_WH               },
    { .name = "WJ",                 .typ = TYP_CMD,                 .tid = CMD_WJ              },
    { .name = "WL",                 .typ = TYP_CMD,                 .tid = CMD_WL             },
    { .name = "WN",                 .typ = TYP_CMD,                 .tid = CMD_WN              },
    { .name = "WK",                 .typ = TYP_CMD,                 .tid = CMD_WK               },
    { .name = "WC",                 .typ = TYP_CMD,                 .tid = CMD_WC             },
    { .name = "WS",                 .typ = TYP_CMD,                 .tid = CMD_WS               },
    { .name = "WT",                 .typ = TYP_CMD,                 .tid = CMD_WT          },
    { .name = "WX",                 .typ = TYP_CMD,                 .tid = CMD_WX            },
    
    { .name = "PM",                 .typ = TYP_NIL,                 .tid = TOK_PM               },
    { .name = "PC",                 .typ = TYP_NIL,                 .tid = TOK_PC           },
    { .name = "IT",                 .typ = TYP_NIL,                 .tid = TOK_IT               },
    { .name = "DT",                 .typ = TYP_NIL,                 .tid = TOK_DT             },
    { .name = "IC",                 .typ = TYP_NIL,                 .tid = TOK_IC             },
    { .name = "DC",                 .typ = TYP_NIL,                 .tid = TOK_DC              },
    { .name = "UC",                 .typ = TYP_NIL,                 .tid = TOK_UC               },
    { .name = "ICR",                .typ = TYP_NIL,                 .tid = TOK_ICR              },
    { .name = "DCR",                .typ = TYP_NIL,                 .tid = TOK_DCR             },
    { .name = "UCR",                .typ = TYP_NIL,                 .tid = TOK_UCR             },
    { .name = "MCR",                .typ = TYP_NIL,                 .tid = TOK_MCR             },
    { .name = "ITR",                .typ = TYP_NIL,                 .tid = TOK_ITR              },
    { .name = "DTR",                .typ = TYP_NIL,                 .tid = TOK_DTR             },
    { .name = "PCR",                .typ = TYP_NIL,                 .tid = TOK_PCR              },
    { .name = "IOR",                .typ = TYP_NIL,                 .tid = TOK_IOR          },
    { .name = "TX",                 .typ = TYP_NIL,                 .tid = TOK_TX            },
    
    //--------------------------------------------------------------------------------------------------------
    // General registers.
    //
    //--------------------------------------------------------------------------------------------------------
    { .name = "R0",                 .typ = TYP_GREG,                .tid = GR_0,                    0               },
    { .name = "R1",                 .typ = TYP_GREG,                .tid = GR_1,                    1               },
    { .name = "R2",                 .typ = TYP_GREG,                .tid = GR_2,                    2               },
    { .name = "R3",                 .typ = TYP_GREG,                .tid = GR_3,                    3               },
    { .name = "R4",                 .typ = TYP_GREG,                .tid = GR_4,                    4               },
    { .name = "R5",                 .typ = TYP_GREG,                .tid = GR_5,                    5               },
    { .name = "R6",                 .typ = TYP_GREG,                .tid = GR_6,                    6               },
    { .name = "R7",                 .typ = TYP_GREG,                .tid = GR_7,                    7               },
    { .name = "R8",                 .typ = TYP_GREG,                .tid = GR_8,                    8               },
    { .name = "R9",                 .typ = TYP_GREG,                .tid = GR_9,                    9               },
    { .name = "R10",                .typ = TYP_GREG,                .tid = GR_10,                   10              },
    { .name = "R11",                .typ = TYP_GREG,                .tid = GR_11,                   11              },
    { .name = "R12",                .typ = TYP_GREG,                .tid = GR_12,                   12              },
    { .name = "R13",                .typ = TYP_GREG,                .tid = GR_13,                   13              },
    { .name = "R14",                .typ = TYP_GREG,                .tid = GR_14,                   14              },
    { .name = "R15",                .typ = TYP_GREG,                .tid = GR_15,                   15              },
    { .name = "GR",                 .typ = TYP_GREG,                .tid = GR_SET,                  0               },
    
    //--------------------------------------------------------------------------------------------------------
    // Segment registers.
    //
    //--------------------------------------------------------------------------------------------------------
    { .name = "S0",                 .typ = TYP_SREG,                .tid = SR_0,                    0               },
    { .name = "S1",                 .typ = TYP_SREG,                .tid = SR_1,                    1               },
    { .name = "S2",                 .typ = TYP_SREG,                .tid = SR_2,                    2               },
    { .name = "S3",                 .typ = TYP_SREG,                .tid = SR_3,                    3               },
    { .name = "S4",                 .typ = TYP_SREG,                .tid = SR_4,                    4               },
    { .name = "S5",                 .typ = TYP_SREG,                .tid = SR_5,                    5               },
    { .name = "S6",                 .typ = TYP_SREG,                .tid = SR_6,                    6               },
    { .name = "S7",                 .typ = TYP_SREG,                .tid = SR_7,                    7               },
    { .name = "SR",                 .typ = TYP_SREG,                .tid = SR_SET,                  0               },
    
    //--------------------------------------------------------------------------------------------------------
    // Control registers.
    //
    //--------------------------------------------------------------------------------------------------------
    { .name = "C0",                 .typ = TYP_CREG,                .tid = CR_0,                    0               },
    { .name = "C1",                 .typ = TYP_CREG,                .tid = CR_1,                    1               },
    { .name = "C2",                 .typ = TYP_CREG,                .tid = CR_2,                    2               },
    { .name = "C3",                 .typ = TYP_CREG,                .tid = CR_3,                    3               },
    { .name = "C4",                 .typ = TYP_CREG,                .tid = CR_4,                    4               },
    { .name = "C5",                 .typ = TYP_CREG,                .tid = CR_5,                    5               },
    { .name = "C6",                 .typ = TYP_CREG,                .tid = CR_6,                    6               },
    { .name = "C7",                 .typ = TYP_CREG,                .tid = CR_7,                    7               },
    { .name = "C8",                 .typ = TYP_CREG,                .tid = CR_8,                    8               },
    { .name = "C9",                 .typ = TYP_CREG,                .tid = CR_9,                    9               },
    { .name = "C10",                .typ = TYP_CREG,                .tid = CR_10,                   10              },
    { .name = "C11",                .typ = TYP_CREG,                .tid = CR_11,                   11              },
    { .name = "C12",                .typ = TYP_CREG,                .tid = CR_12,                   12              },
    { .name = "C13",                .typ = TYP_CREG,                .tid = CR_13,                   13              },
    { .name = "C14",                .typ = TYP_CREG,                .tid = CR_14,                   14              },
    { .name = "C15",                .typ = TYP_CREG,                .tid = CR_15,                   15              },
    { .name = "C16",                .typ = TYP_CREG,                .tid = CR_16,                   16              },
    { .name = "C17",                .typ = TYP_CREG,                .tid = CR_17,                   17              },
    { .name = "C18",                .typ = TYP_CREG,                .tid = CR_18,                   18              },
    { .name = "C19",                .typ = TYP_CREG,                .tid = CR_19,                   19              },
    { .name = "C20",                .typ = TYP_CREG,                .tid = CR_20,                   20              },
    { .name = "C21",                .typ = TYP_CREG,                .tid = CR_21,                   21              },
    { .name = "C22",                .typ = TYP_CREG,                .tid = CR_22,                   22              },
    { .name = "C23",                .typ = TYP_CREG,                .tid = CR_23,                   23              },
    { .name = "C24",                .typ = TYP_CREG,                .tid = CR_24,                   24              },
    { .name = "C25",                .typ = TYP_CREG,                .tid = CR_25,                   25              },
    { .name = "C26",                .typ = TYP_CREG,                .tid = CR_26,                   26              },
    { .name = "C27",                .typ = TYP_CREG,                .tid = CR_27,                   27              },
    { .name = "C28",                .typ = TYP_CREG,                .tid = CR_28,                   28              },
    { .name = "C29",                .typ = TYP_CREG,                .tid = CR_29,                   29              },
    { .name = "C30",                .typ = TYP_CREG,                .tid = CR_30,                   30              },
    { .name = "C31",                .typ = TYP_CREG,                .tid = CR_31,                   31              },
    { .name = "CR",                 .typ = TYP_CREG,                .tid = CR_SET,                  0               },
    
    //--------------------------------------------------------------------------------------------------------
    // CPu Core register tokens.
    //
    //--------------------------------------------------------------------------------------------------------
    { .name = "IA_SEG",             .typ = TYP_PSTATE_PREG,             PS_IA_SEG,                  0               },
    { .name = "IA_OFS",             .typ = TYP_PSTATE_PREG,             PS_IA_OFS,                  0               },
    { .name = "ST_REG",             .typ = TYP_PSTATE_PREG,             PS_STATUS,                  0               },
    { .name = "PS",                 .typ = TYP_PSTATE_PREG,             .tid = PS_SET,              0               },
    
    { .name = "FD_IA_SEG",          .typ = TYP_FD_PREG,                 FD_IA_SEG,                  0               },
    { .name = "FD_IA_OFS",          .typ = TYP_FD_PREG,                 FD_IA_OFS,                  0               },
    { .name = "FDR",                .typ = TYP_FD_PREG,                 .tid = FD_SET,              0               },
    
   /*
    { .name = "FD_INSTR",           .typ = TYP_FD_PREG,                 FD_INSTR,                   0               },
    { .name = "FD_A",               .typ = TYP_FD_PREG,                 FD_A,                       0               },
    { .name = "FD_B",               .typ = TYP_FD_PREG,                 FD_B,                       0               },
    { .name = "FD_X",               .typ = TYP_FD_PREG,                 FD_X,                       0               },
    
    */
    
    { .name = "MA_IA_SEG",          .typ = TYP_MA_PREG,                 MA_IA_SEG,                  0               },
    { .name = "MA_IA_OFS",          .typ = TYP_MA_PREG,                 MA_IA_OFS,                  0               },
    { .name = "MA_INSTR",           .typ = TYP_MA_PREG,                 MA_INSTR,                   0               },
    { .name = "MA_A",               .typ = TYP_MA_PREG,                 MA_A,                       0               },
    { .name = "MA_B",               .typ = TYP_MA_PREG,                 MA_B,                       0               },
    { .name = "MA_X",               .typ = TYP_MA_PREG,                 MA_X,                       0               },
    { .name = "MA_S",               .typ = TYP_MA_PREG,                 MA_S,                       0               },
    { .name = "MAR",                .typ = TYP_MA_PREG,                 .tid = MA_SET            },
    
    { .name = "EX_IA_SEG",          .typ = TYP_EX_PREG,                 EX_IA_SEG,                  0               },
    { .name = "EX_IA_OFS",          .typ = TYP_EX_PREG,                 EX_IA_OFS,                  0               },
    { .name = "EX_INSTR",           .typ = TYP_EX_PREG,                 EX_INSTR,                   0               },
    { .name = "EX_A",               .typ = TYP_EX_PREG,                 EX_A,                       0               },
    { .name = "EX_B",               .typ = TYP_EX_PREG,                 EX_B,                       0               },
    { .name = "EX_X",               .typ = TYP_EX_PREG,                 EX_X,                       0               },
    { .name = "EX_S",               .typ = TYP_EX_PREG,                 EX_S,                       0               },
    { .name = "EXR",                .typ = TYP_EX_PREG,                 .tid = EX_SET            },
    
    { .name = "IC_L1_STATE",        .typ = TYP_IC_L1_REG,               IC_L1_STATE,                0               },
    { .name = "IC_L1_REQ",          .typ = TYP_IC_L1_REG,               IC_L1_REQ,                  0               },
    { .name = "IC_L1_REQ_SEG",      .typ = TYP_IC_L1_REG,               IC_L1_REQ_SEG,              0               },
    { .name = "IC_L1_REQ_OFS",      .typ = TYP_IC_L1_REG,               IC_L1_REQ_OFS,              0               },
    { .name = "IC_L1_REQ_TAG",      .typ = TYP_IC_L1_REG,               IC_L1_REQ_TAG,              0               },
    { .name = "IC_L1_REQ_LEN",      .typ = TYP_IC_L1_REG,               IC_L1_REQ_LEN,              0               },
    { .name = "IC_L1_REQ_LAT",      .typ = TYP_IC_L1_REG,               IC_L1_LATENCY,              0               },
    { .name = "IC_L1_SETS",         .typ = TYP_IC_L1_REG,               IC_L1_SETS,                 0               },
    { .name = "IC_L1_ENTRIES",      .typ = TYP_IC_L1_REG,               IC_L1_BLOCK_ENTRIES,        0               },
    { .name = "IC_L1_B_SIZE",       .typ = TYP_IC_L1_REG,               IC_L1_BLOCK_SIZE,           0               },
    { .name = "ICL1",               .typ = TYP_IC_L1_REG,               .tid = IC_L1_SET                            },
    
    { .name = "DC_L1_STATE",        .typ = TYP_DC_L1_REG,               DC_L1_STATE,                0               },
    { .name = "DC_L1_REQ",          .typ = TYP_DC_L1_REG,               DC_L1_REQ,                  0               },
    { .name = "DC_L1_REQ_SEG",      .typ = TYP_DC_L1_REG,               DC_L1_REQ_SEG,              0               },
    { .name = "DC_L1_REQ_OFS",      .typ = TYP_DC_L1_REG,               DC_L1_REQ_OFS,              0               },
    { .name = "DC_L1_REQ_TAG",      .typ = TYP_DC_L1_REG,               DC_L1_REQ_TAG,              0               },
    { .name = "DC_L1_REQ_LEN",      .typ = TYP_DC_L1_REG,               DC_L1_REQ_LEN,              0               },
    { .name = "DC_L1_REQ_LAT",      .typ = TYP_DC_L1_REG,               DC_L1_LATENCY,              0               },
    { .name = "DC_L1_SETS",         .typ = TYP_DC_L1_REG,               DC_L1_SETS,                 0               },
    { .name = "DC_L1_ENTRIES",      .typ = TYP_DC_L1_REG,               DC_L1_BLOCK_ENTRIES,        0               },
    { .name = "DC_L1_B_SIZE",       .typ = TYP_DC_L1_REG,               DC_L1_BLOCK_SIZE,           0               },
    { .name = "DCL1",               .typ = TYP_DC_L1_REG,               .tid = DC_L1_SET                            },
    
    { .name = "UC_L2_STATE",        .typ = TYP_UC_L2_REG,               UC_L2_STATE,                0               },
    { .name = "UC_L2_REQ",          .typ = TYP_UC_L2_REG,               UC_L2_REQ,                  0               },
    { .name = "UC_L2_REQ_SEG",      .typ = TYP_UC_L2_REG,               UC_L2_REQ_SEG,              0               },
    { .name = "UC_L2_REQ_OFS",      .typ = TYP_UC_L2_REG,               UC_L2_REQ_OFS,              0               },
    { .name = "UC_L2_REQ_TAG",      .typ = TYP_UC_L2_REG,               UC_L2_REQ_TAG,              0               },
    { .name = "UC_L2_REQ_LEN",      .typ = TYP_UC_L2_REG,               UC_L2_REQ_LEN,              0               },
    { .name = "UC_L2_REQ_LAT",      .typ = TYP_UC_L2_REG,               UC_L2_LATENCY,              0               },
    { .name = "UC_L2_SETS",         .typ = TYP_UC_L2_REG,               UC_L2_SETS,                 0               },
    { .name = "UC_L2_ENTRIES",      .typ = TYP_UC_L2_REG,               UC_L2_BLOCK_ENTRIES,        0               },
    { .name = "UC_L2_B_SIZE",       .typ = TYP_UC_L2_REG,               UC_L2_BLOCK_SIZE,           0               },
    { .name = "UCL2",               .typ = TYP_UC_L2_REG,               DC_L1_SET,                  0               },
    
    { .name = "ITLB_STATE",         .typ = TYP_ITLB_REG,                ITLB_STATE,                 0               },
    { .name = "ITLB_REQ",           .typ = TYP_ITLB_REG,                ITLB_REQ,                   0               },
    { .name = "ITLB_REQ_SEG",       .typ = TYP_ITLB_REG,                ITLB_REQ_SEG,               0               },
    { .name = "ITLB_REQ_OFS",       .typ = TYP_ITLB_REG,                ITLB_REQ_OFS,               0               },
    { .name = "ITLBL1",             .typ = TYP_ITLB_REG,                ITLB_SET,                   0               },
    
    { .name = "DTLB_STATE",         .typ = TYP_DTLB_REG,                DTLB_STATE,                 0               },
    { .name = "DTLB_REQ",           .typ = TYP_DTLB_REG,                DTLB_REQ,                   0               },
    { .name = "DTLB_REQ_SEG",       .typ = TYP_DTLB_REG,                DTLB_REQ_SEG,               0               },
    { .name = "DTLB_REQ_OFS",       .typ = TYP_DTLB_REG,                DTLB_REQ_OFS,               0               },
    { .name = "DTLBL1",             .typ = TYP_DTLB_REG,                DTLB_SET,                   0               },
    
    //--------------------------------------------------------------------------------------------------------
    // The last token to mark the list end.
    //
    //--------------------------------------------------------------------------------------------------------
    { .name = "",               .typ = TYP_NIL,             .tid = TOK_LAST          }
    
};




// ??? may also go away when we have the parser ready...
//------------------------------------------------------------------------------------------------------------
// The command line parser simply uses the "sscanf" library routine. Here are the formats for the various
// command lines. "S" means a string input, "D" a numeric integer input, "U" an unsigned integer input.
//
//------------------------------------------------------------------------------------------------------------
const char  FMT_STR_1S_1D[ ]    = "%32s %i";
const char  FMT_STR_1S_2D[ ]    = "%32s %i %i";
const char  FMT_STR_1S_3D[ ]    = "%32s %i %i %i";
const char  FMT_STR_1S_1D_1S[ ] = "%32s %i %32s";
const char  FMT_STR_1S_1D_2S[ ] = "%32s %i %32s %32s";
const char  FMT_STR_2S[ ]       = "%32s %32s";
const char  FMT_STR_3S[ ]       = "%32s %32s %32s";
const char  FMT_STR_2S_1D[ ]    = "%32s %32s %i";
const char  FMT_STR_2S_2U_1S[ ] = "%32s %32s %i %i %32s";
const char  FMT_STR_2S_2D[ ]    = "%32s %32s %i %i";
const char  FMT_STR_2S_4D[ ]    = "%32s %32s %i %i %i %i";
const char  FMT_STR_2S_LS[ ]    = "%32s %32s %256s";


//------------------------------------------------------------------------------------------------------------
// The command line is broken into tokens by the tokenizer object.
//
//------------------------------------------------------------------------------------------------------------
DrvTokenizer *tok = new DrvTokenizer( );

// ??? will go away...
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


// ??? goes away ...
//------------------------------------------------------------------------------------------------------------
// Token Table management. There are a functions to lookup a token by its name or alias name, returning the
// tokenId or token group Id. There is also a function to get the name for a token Id. Straightforward.
//
//------------------------------------------------------------------------------------------------------------
TokId lookupTokId( char *str, TokId def = TOK_NIL ) {
    
    if (( strlen( str ) == 0 ) || ( strlen ( str ) > TOK_NAME_SIZE )) return( def );
    
    char tmpStr[ TOK_NAME_SIZE ];
    strncpy( tmpStr, str, strlen( str ) + 1 );
    upshiftStr( tmpStr );
    
    for ( int i = 0; i < TOK_TAB_SIZE; i++ ) {
        
        if ( strcmp( tmpStr, tokTab[ i ].name      ) == 0 ) return( tokTab[ i ].tokId );
        if ( strcmp( tmpStr, tokTab[ i ].aliasName ) == 0 ) return( tokTab[ i ].tokId );
    }
    
    return( def );
}

TokId lookupTokGrpId( char *str, TokId def = TOK_NIL ) {
    
    if (( strlen( str ) == 0 ) || ( strlen ( str ) > TOK_NAME_SIZE )) return( def );
    
    char tmpStr[ TOK_NAME_SIZE ];
    strncpy( tmpStr, str, strlen( str ) + 1 );
    upshiftStr( tmpStr );
    
    for ( int i = 0; i < TOK_TAB_SIZE; i++ ) {
        
        if ( strcmp( tmpStr, tokTab[ i ].name      ) == 0 ) return( tokTab[ i ].tokGrpId );
        if ( strcmp( tmpStr, tokTab[ i ].aliasName ) == 0 ) return( tokTab[ i ].tokGrpId );
    }
    
    return( def );
}

TokId lookupTokGrpId( TokId tok, TokId def = TOK_NIL ) {
    
    for ( int i = 0; i < TOK_TAB_SIZE; i++ ) {
        
        if ( tokTab[ i ].tokId == tok ) return( tokTab[ i ].tokGrpId );
    }
    
    return( def );
}

char *lookupTokenName( TokId tokId, char *defName = (char *) "" ) {
    
    for ( int i = 0; i < TOK_TAB_SIZE; i++ ) {
        
        if ( tokTab[ i ].tokId == tokId ) return(( char *) &tokTab[ i ].name );
    }
    
    return( defName );
}

//------------------------------------------------------------------------------------------------------------
// Utility functions to test tokens for group membership.
//
//------------------------------------------------------------------------------------------------------------
TokId matchFmtOptions( char *argStr, TokId def = TOK_NIL ) {
    
    if ( strlen ( argStr ) == 0 ) return( def );
    
    TokId tmp = lookupTokGrpId( argStr );
    return(( tmp == FMT_SET ) ? lookupTokId( argStr ) : def );
}

TokId matchRegSet( char *argStr, TokId def = TOK_NIL ) {
    
    if ( strlen ( argStr ) == 0 ) return( def );
    TokId tmp = lookupTokGrpId( argStr );
    
    return((( tmp == REG_SET ) || ( tmp == TOK_ALL )) ? lookupTokId( argStr ) : def );
}

TokId matchReg( char *argStr, TokId def = TOK_NIL ) {
    
    if ( strlen ( argStr ) == 0 ) return( def );
    
    TokId tmpReg    = lookupTokId( argStr );
    TokId tmpGrp    = lookupTokGrpId( tmpReg );
    TokId tmpGrpGrp = lookupTokGrpId( tmpGrp );
    
    return(( tmpGrpGrp == REG_SET ) ? tmpReg : def );
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

//------------------------------------------------------------------------------------------------------------
// Print out an error message text with an optional argument.
//
// ??? over time all text errors in the command should go here...
//------------------------------------------------------------------------------------------------------------
void printErrMsg( ErrMsgId errNum, char *argStr = nullptr ) {
    
    switch ( errNum ) {
            
        case ERR_NOT_IN_WIN_MODE:       fprintf( stdout, "Command only valid in Windows mode\n" ); break;
        case ERR_OPEN_EXEC_FILE:        fprintf( stdout, "Error while opening file: \"%s\"\n", argStr ); break;
        case ERR_EXPECTED_FILE_NAME:    fprintf( stdout, "Expected a file name\n" ); break;
        case ERR_INVALID_CMD:           fprintf( stdout, "Invalid command, use help or whelp\n"); break;
        case ERR_INVALID_WIN_STACK_ID:  fprintf( stdout, "Invalid window stack Id\n" ); break;
        case ERR_EXPECTED_STACK_ID:     fprintf( stdout, "Expected stack Id\n" ); break;
        case ERR_INVALID_WIN_ID:        fprintf( stdout, "Invalid window Id\n" ); break;
        case ERR_EXPECTED_WIN_ID:       fprintf( stdout, "Expected a window Id\n" ); break;
            
        case ERR_EXTRA_TOKEN_IN_STR:    fprintf( stdout, "Extra tokens in command line\n" ); break;
        case ERR_EXPECTED_LPAREN:       fprintf( stdout, "Expected a left paren\n" ); break;
        case ERR_EXPECTED_RPAREN:       fprintf( stdout, "Expected a right paren\n" ); break;
        case ERR_EXPECTED_COMMA:        fprintf( stdout, "Expected a comma\n" ); break;
            
        case ERR_EXPECTED_NUMERIC:      fprintf( stdout, "Expected a numeric value\n" ); break;
        case ERR_EXPR_TYPE_MATCH:       fprintf( stdout, "Expression type mismatch\n" ); break;
        case ERR_EXPR_FACTOR:           fprintf( stdout, "Expression error: factor\n" ); break;
        case ERR_EXPECTED_GENERAL_REG:  fprintf( stdout, "Expression a general reg\n" ); break;
            
        case ERR_INVALID_ARG:           fprintf( stdout, "Invalid command argument\n" ); break;
            
            
        case ERR_INVALID_FMT_OPT:       fprintf( stdout, "Invalid format option\n" ); break;
        case ERR_EXPECTED_FMT_OPT:      fprintf( stdout, "Expected a format option\n" ); break;
        case ERR_INVALID_WIN_TYPE:      fprintf( stdout, "Invalid window type\n" ); break;
        case ERR_EXPECTED_WIN_TYPE:     fprintf( stdout, "Expected a window type\n" ); break;
        case ERR_OUT_OF_WINDOWS:        fprintf( stdout, "Cannot create more windows\n" ); break;
            
        default: {
            
            fprintf( stdout, "Error: %d", errNum );
            if ( argStr != nullptr ) fprintf( stdout, "%32s", argStr );
            fprintf( stdout, "/n" );
        }
    }
}

//------------------------------------------------------------------------------------------------------------
// "commandLineError" is a little helper that prints out the error encountered. We will print a caret marker
// where we found the error, and then return a false. Parsing errors typically result in aborting the parsing
// process.
//
//------------------------------------------------------------------------------------------------------------
bool cmdLineError( ErrMsgId errNum, char *argStr = nullptr) {
    
    int i           = 0;
    int tokIndex    = tok -> tokCharIndex( );
    
    while ( i < tokIndex ) {
        
        fprintf( stdout, " " );
        i ++;
    }
    
    fprintf( stdout, "^\n" );
    printErrMsg( errNum, argStr );
    return( false );
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
// Check that the command line does not contain any extra tokens when the parser completed the analysis of the
// command.
//
//------------------------------------------------------------------------------------------------------------
bool checkEOS( ) {
    
    if ( tok -> isToken( TOK_EOS )) return( true );
    else return( cmdLineError( ERR_EXTRA_TOKEN_IN_STR ));
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
    else return( cmdLineError( ERR_EXPECTED_COMMA ));
}

bool acceptLparen( ) {
    
    if ( tok -> isToken( TOK_LPAREN )) {
        
        tok -> nextToken( );
        return( true );
    }
    else return( cmdLineError( ERR_EXPECTED_LPAREN ));
}

bool acceptRparen( ) {
    
    if ( tok -> isToken( TOK_RPAREN )) {
        
        tok -> nextToken( );
        return( true );
    }
    else return( cmdLineError( ERR_EXPECTED_LPAREN ));
}


// ??? could become an own file "DrvExprEval", it will become more complex with types, etc.
//------------------------------------------------------------------------------------------------------------
// "parseExpr" needs to be declared forward.
//
//------------------------------------------------------------------------------------------------------------
bool parseExpr( DrvExpr *rExpr );

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
bool parseFactor( DrvExpr *rExpr ) {
    
    rExpr -> typ        = TYP_NIL;
    rExpr -> numVal1    = 0;
    rExpr -> numVal2    = 0;
    
    if ( tok -> isTokenTyp( TYP_CMD ))  {
        
        rExpr -> typ    = TYP_CMD;
        rExpr -> tokId  = tok -> tokId( );
        tok -> nextToken( );
        return( true );
    }
    else if ( tok -> isTokenTyp( TYP_NUM ))  {
        
        rExpr -> typ     = TYP_NUM;
        rExpr -> numVal1 = tok -> tokVal( );
        tok -> nextToken( );
        return( true );
    }
    else if ( tok -> isTokenTyp( TYP_STR ))  {
        
        rExpr -> typ = TYP_STR;
        strcpy( rExpr -> strVal, tok -> tokStr( ));
        tok -> nextToken( );
        return( true );
    }
    else if ( tok -> isTokenTyp( TYP_GREG ))  {
        
        rExpr -> typ     = TYP_GREG;
        rExpr -> numVal1 = tok -> tokVal( );
        tok -> nextToken( );
        return( true );
    }
    else if ( tok -> isTokenTyp( TYP_SREG ))  {
        
        rExpr -> typ = TYP_SREG;
        rExpr -> numVal1 = tok -> tokVal( );
        tok -> nextToken( );
        return( true );
    }
    else if ( tok -> isTokenTyp( TYP_CREG ))  {
        
        rExpr -> typ = TYP_CREG;
        rExpr -> numVal1 = tok -> tokVal( );
        tok -> nextToken( );
        return( true );
    }
    else if ( tok -> isTokenTyp( TYP_IDENT )) {
        
        rExpr -> typ    = TYP_IDENT;
        rExpr -> tokId  = tok -> tokId( );
        tok -> nextToken( );
        return( true );
    }
    else if ( tok -> isToken( TOK_NEG )) {
        
        parseFactor( rExpr );
        rExpr -> numVal1 = ~ rExpr -> numVal1;
        return( true );
    }
    else if ( tok -> isToken( TOK_LPAREN )) {
     
        // ??? we need to decide what we do about addresses.... a register pair would imply that
        // we get the values....
        
        tok -> nextToken( );
        if ( tok -> isTokenTyp( TYP_SREG )) {
            
            rExpr -> typ        = TYP_EXT_ADR;
            rExpr -> numVal1    = tok -> tokVal( );
            
            tok -> nextToken( );
            if ( ! acceptComma( )) return( false );
            
            if ( tok -> isTokenTyp( TYP_GREG )) {
                
                rExpr -> numVal2 = tok -> tokVal( );
                tok -> nextToken( );
            }
            else return( cmdLineError( ERR_EXPECTED_GENERAL_REG ));
        }
        else if ( tok -> isTokenTyp( TYP_GREG )) {
            
            rExpr -> typ = TYP_ADR;
            rExpr -> numVal1 = tok -> tokVal( );
            tok -> nextToken( );
        }
        else if ( ! parseExpr( rExpr )) return( false );
        
        if ( ! acceptRparen( )) return( false );
        return( true );
    }
    else {
        
        cmdLineError( ERR_EXPR_FACTOR );
        rExpr -> typ = TYP_NUM;
        rExpr -> numVal1 = 0;
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
bool parseTerm( DrvExpr *rExpr ) {
    
    DrvExpr lExpr;
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
            
            return ( cmdLineError( ERR_EXPR_TYPE_MATCH ));
        }
        
        switch( op ) {
                
            case TOK_MULT:   rExpr -> numVal1 = rExpr -> numVal1 * lExpr.numVal1; break;
            case TOK_DIV:    rExpr -> numVal1 = rExpr -> numVal1 / lExpr.numVal1; break;
            case TOK_MOD:    rExpr -> numVal1 = rExpr -> numVal1 % lExpr.numVal1; break;
            case TOK_AND:    rExpr -> numVal1 = rExpr -> numVal1 & lExpr.numVal1; break;
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
bool parseExpr( DrvExpr *rExpr ) {
    
    DrvExpr lExpr;
    bool rStat;
    
    if ( tok -> isToken( TOK_PLUS )) {
        
        tok -> nextToken( );
        rStat = parseTerm( rExpr );
        
        if ( ! ( rExpr -> typ == TYP_NUM )) {
            
            return( cmdLineError( ERR_EXPECTED_NUMERIC ));
        }
    }
    else if ( tok -> isToken( TOK_MINUS )) {
        
        tok -> nextToken( );
        rStat = parseTerm( rExpr );
        
        if ( rExpr -> typ == TYP_NUM ) rExpr -> numVal1 = - rExpr -> numVal1;
        else return( cmdLineError( ERR_EXPECTED_NUMERIC ));
    }
    else rStat = parseTerm( rExpr );
    
    while (( tok -> isToken( TOK_PLUS   )) ||
           ( tok -> isToken( TOK_MINUS  )) ||
           ( tok -> isToken( TOK_OR     )) ||
           ( tok -> isToken( TOK_XOR    ))) {
        
        uint8_t op = tok -> tokId( );
        
        tok -> nextToken( );
        rStat = parseTerm( &lExpr );
        
        if ( rExpr -> typ != lExpr.typ ) {
            
            return ( cmdLineError( ERR_EXPR_TYPE_MATCH ));
        }
        
        switch ( op ) {
                
            case TOK_PLUS:   rExpr -> numVal1 = rExpr -> numVal1 + lExpr.numVal1; break;
            case TOK_MINUS:  rExpr -> numVal1 = rExpr -> numVal1 - lExpr.numVal1; break;
            case TOK_OR:     rExpr -> numVal1 = rExpr -> numVal1 | lExpr.numVal1; break;
            case TOK_XOR:    rExpr -> numVal1 = rExpr -> numVal1 ^ lExpr.numVal1; break;
        }
    }
    
    return( true );
}

}; // namespace








//************************************************************************************************************
// Object methods.
//************************************************************************************************************

//------------------------------------------------------------------------------------------------------------
// The object constructor. We just remember where globals are.
//
//------------------------------------------------------------------------------------------------------------
DrvCmds::DrvCmds( VCPU32Globals *glb ) {
    
    this -> glb = glb;
}

//------------------------------------------------------------------------------------------------------------
// Return the current command entered.
//
//------------------------------------------------------------------------------------------------------------
TokId DrvCmds::getCurrentCmd( ) {
    
    return( currentCmd );
}

//------------------------------------------------------------------------------------------------------------
// A little helper method for ENV to display the token name of a token Id.
//
//------------------------------------------------------------------------------------------------------------
char *DrvCmds::tokIdToName( TokId tokId ) {
    
    return( ::lookupTokenName( tokId ));
}


//------------------------------------------------------------------------------------------------------------
// Our friendly welcome message with the actual program version. We also set some of the environment variables
// to an initial value. Especially string variables need to be set as they are not initialized from the
// environment variable table.
//
//------------------------------------------------------------------------------------------------------------
void DrvCmds::printWelcome( ) {
    
    glb -> env -> setEnvVal( ENV_PROG_VERSION, (char *) VERSION );
    glb -> env -> setEnvVal( ENV_GIT_BRANCH, (char *) BRANCH );
    glb -> env -> setEnvVal( ENV_PROG_PATCH_LEVEL, PATCH_LEVEL );
    glb -> env -> setEnvVal( ENV_EXIT_CODE, 0 );
    
    if ( isatty( fileno( stdin ))) {
        
        fprintf( stdout, "VCPU-32 Simulator, Version: %s\n", glb -> env -> getEnvValStr( ENV_PROG_VERSION ));
        fprintf( stdout, "Git Branch: %s\n", glb -> env -> getEnvValStr( ENV_GIT_BRANCH ));
    }
}

//------------------------------------------------------------------------------------------------------------
// One day we will handle command line arguments....
//
//  -v           verbose
//  -i <path>    init file
//
// ??? to do ...
//------------------------------------------------------------------------------------------------------------
void DrvCmds::processCmdLineArgs( int argc, const char *argv[ ] ) {
    
    while ( argc > 0 ) {
        
        argc --;
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
        
        if ( glb -> env -> getEnvValBool( ENV_SHOW_CMD_CNT ))
            fprintf( stdout, "(%i) ", glb -> env -> getEnvValInt( ENV_CMD_CNT ));
        
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
                
                glb -> env -> setEnvVal( ENV_CMD_CNT, glb -> env -> getEnvValInt( ENV_CMD_CNT ) + 1 );
                return( true );
            }
            else return( false );
        }
        else if ( feof( stdin )) exit( glb -> env -> getEnvValInt( ENV_EXIT_CODE ));
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
                
                if ( glb -> env -> getEnvValBool( ENV_ECHO_CMD )) {
                    
                    fprintf( stdout, "%s\n", cmdLineBuf );
                }
                
                removeComment( cmdLineBuf );
                evalInputLine( cmdLineBuf );
            }
        }
        else printErrMsg( ERR_OPEN_EXEC_FILE, fileName );
    }
    else printErrMsg( ERR_EXPECTED_FILE_NAME  );
    
    return( NO_ERR );
}

//------------------------------------------------------------------------------------------------------------
// Help command. With no arguments, a short help overview is printed. If there is an optional argument,
// specific help on the topic is given.
//
//------------------------------------------------------------------------------------------------------------
uint8_t DrvCmds::helpCmd( ) {
    
    const char FMT_STR[ ] = "%-50s%s\n";
    
    fprintf( stdout, FMT_STR, "help",   "displays syntax and a short description" );
    fprintf( stdout, FMT_STR, "#",      "echoes the command input" );
    fprintf( stdout, FMT_STR, "exit (e) [<val>]", "program exit" );
    fprintf( stdout, FMT_STR, "env ( )  [<var> [<val>]]", "lists the env tab, a variable, sets a variable" );
    fprintf( stdout, FMT_STR, "reset    <mode>", "resets the CPU ( CPU, MEM, STATSm ALL )" );
    fprintf( stdout, FMT_STR, "xf       <filename> ", "execute commands from a file" );
    fprintf( stdout, FMT_STR, "run", "run the CPU" );
    fprintf( stdout, FMT_STR, "s        [<num>] [I|C]", "single step for instruction or clock cycle" );
    fprintf( stdout, FMT_STR, "b        <seg> <ofs>", "sets a break breakpoint at virtual address seg.ofs" );
    fprintf( stdout, FMT_STR, "bd       <seg> <ofs>", "deletes a break breakpoint" );
    fprintf( stdout, FMT_STR, "bl", "displays the breakpoint table" );
    
    
    fprintf( stdout, FMT_STR, "dr       [<regSet>|<reg>] <fmt>]", "display registers" );
    
    
    fprintf( stdout, FMT_STR, "mr       <reg> <val>", "modify registers" );
    fprintf( stdout, FMT_STR, "da       <ofs> [ <len> [ fmt ]]", "display memory" );
    fprintf( stdout, FMT_STR, "daa      <ofs> [ <len> [ fmt ]]", "display memory as code" );
    fprintf( stdout, FMT_STR, "ma       <ofs> <val>", "modify memory" );
    fprintf( stdout, FMT_STR, "maa      <ofs> <asm-str>", "modify memory as code" );
    fprintf( stdout, FMT_STR, "dis      <instr>", "disassemble an instruction" );
    fprintf( stdout, FMT_STR, "asm      <instr-string>", "assemble an instruction" );
    fprintf( stdout, FMT_STR, "hva      <seg> <ofs>",  "returns the hash value function result" );
    fprintf( stdout, FMT_STR, "dca      <I|D|U> [<index> <len>]", "display cache content" );
    fprintf( stdout, FMT_STR, "pca      <I|D|U> <index> [<F>]", "flushes and purges cache data" );
    fprintf( stdout, FMT_STR, "dtlb     <I|D> [<index> <len>]", "display TLB content" );
    fprintf( stdout, FMT_STR, "itlb     <I|D> <seg> <ofs> <argAcc> <argAdr>", "inserts an entry into the TLB" );
    fprintf( stdout, FMT_STR, "ptlb     <I|D> <seg> <ofs>", "purges an entry from the TLB" );
    fprintf( stdout, FMT_STR, "lmf      <path> <opt>", "loads memory from a file in MA command format" );
    fprintf( stdout, FMT_STR, "smf      <path> <ofs> <len> ", "stores memory to a file using MA command format" );
    fprintf( stdout, FMT_STR, "won",  "switches to windows mode" );
    fprintf( stdout, FMT_STR, "woff", "switches to command line mode" );
    fprintf( stdout, FMT_STR, "wdef", "reset the windows to their default values" );
    fprintf( stdout, FMT_STR, "wse",  "enable window stacks" );
    fprintf( stdout, FMT_STR, "wsd",  "disable window stacks" );
    fprintf( stdout, FMT_STR, "<win><cmd> [<args-list>]", "issue a window command, use whelp for details." );
    fprintf( stdout, "\n" );
    
    return( NO_ERR );
}

//------------------------------------------------------------------------------------------------------------
// Display the window specific help.
//
//------------------------------------------------------------------------------------------------------------
uint8_t DrvCmds::winHelpCmd( ) {
    
    const char FMT_STR[ ] = "%-20s%s\n";
    
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
    fprintf( stdout, FMT_STR, "E [<wNum>]", "Enable window display" );
    fprintf( stdout, FMT_STR, "D [<wNum>]", "Disable window display" );
    fprintf( stdout, FMT_STR, "B <amt> [<wNum>]", "Move backward by n items" );
    fprintf( stdout, FMT_STR, "F <amt> [<wNum>]", "Move forward by n items" );
    fprintf( stdout, FMT_STR, "H <pos> [<wNum>]", "Set window home position or set new home position" );
    fprintf( stdout, FMT_STR, "J <pos> [<wNum>]", "Set window start to new position");
    fprintf( stdout, FMT_STR, "L <lines> [<wNum>]", "Set window lines including banner line" );
    fprintf( stdout, FMT_STR, "R <radix> [<wNum>]", "Set window radix ( OCT, DEC, HEX )" );
    fprintf( stdout, FMT_STR, "C <wNum>", "set the window <wNum> as current window" );
    fprintf( stdout, FMT_STR, "T <wNum>", "toggle through alternate window content" );
    fprintf( stdout, FMT_STR, "X <wNum>", "exchange current window with this window");
    fprintf( stdout, FMT_STR, "N <type> [<arg>]", "New user defined window ( PM, PC, IT, DT, IC, ICR, DCR, MCR, TX )" );
    fprintf( stdout, FMT_STR, "K <wNumStart> [<wNumEnd>]", "Removes a range of user defined window" );
    fprintf( stdout, FMT_STR, "S <stackNum> <wNumStart> [<wNumEnd>]", "moves a range of user windows into stack <stackNum>" );
    fprintf( stdout, "\n" );
    
    fprintf( stdout, "Example: SRE      -> show special register window\n" );
    fprintf( stdout, "Example: WN PM    -> create a user defined physical memory window\n" );
    fprintf( stdout, "Example: WN 20 11 -> scroll window 11 forward by 20 lines\n" );
    fprintf( stdout, "\n" );
    
    return( NO_ERR );
}

//------------------------------------------------------------------------------------------------------------
// Invalid command handler.
//
//------------------------------------------------------------------------------------------------------------
uint8_t DrvCmds::invalidCmd( ) {
    
    glb -> env -> setEnvVal( ENV_EXIT_CODE, -1 );
    printErrMsg( ERR_INVALID_CMD );
    return( ERR_INVALID_CMD );
}

//------------------------------------------------------------------------------------------------------------
// Exit command. We will exit with the environment variable value for the exit code or the argument value
// in the command. This will be quite useful for test script development.
//
// EXIT <code>
//------------------------------------------------------------------------------------------------------------
uint8_t DrvCmds::exitCmd( ) {
    
    int  exitVal = 0;
    
    if ( tok -> tokTyp( ) == TYP_NUM ) {
        
        if ( tok -> tokVal( ) > 255 ) fprintf( stdout, "Expected an exit code between 0 .. 255\n" );
        return( NO_ERR );
    }
    else if ( tok -> tokId( ) == TOK_EOS ) {
        
        exitVal = glb -> env -> getEnvValInt( ENV_EXIT_CODE );
    }
    
    exit(( exitVal > 255 ) ? 255 : exitVal );
}

//------------------------------------------------------------------------------------------------------------
// ENV command. The test driver has a few global environment variables for data format, command count and so
// on. The ENV command list them all, one in particular and also modifies one if a value is specified.
//
// ENV [ <envId> [ <val> ]]
//------------------------------------------------------------------------------------------------------------
void DrvCmds::envCmd( char *cmdBuf ) {
    
    char    cmdStr[ TOK_NAME_SIZE ]         = "";
    char    arg1Str[ TOK_NAME_SIZE ]        = "";
    char    arg2Str[ TOK_LARGE_STR_SIZE ]   = "";
    int     args   = sscanf( cmdBuf, FMT_STR_2S_LS, cmdStr, arg1Str, arg2Str );
    
    if ( args == 1 ) {
        
        glb -> env -> displayEnvTable( );
    }
    else if ( args == 2 ) {
        
        if ( glb -> env -> displayEnvTabEntry( lookupTokId( arg1Str )) == 0 )
            fprintf( stdout, "Unknown ENV variable\n" );
    }
    else if ( args == 3 ) {
        
        TokId arg1Id  = glb -> env -> lookupEnvTokId( arg1Str );
        
        if ( glb -> env -> getEnvType( arg1Id ) == TOK_NIL ) {
            
            fprintf( stdout, "Unknown ENV variable\n" );
            return;
        }
        
        if ( glb-> env -> isReadOnly( arg1Id )) {
            
            fprintf( stdout, "ENV variable is readonly\n" );
            return;
        }
        
        switch( glb -> env -> getEnvType( arg1Id )) {
                
            case ENV_TYP_TOK: {
                
                glb -> env -> setEnvVal( arg1Id, lookupTokId( arg2Str ));
                
            } break;
                
            case ENV_TYP_BOOL: {
                
                TokId argId = lookupTokId( arg2Str );
                
                if      ( argId == TOK_TRUE )  glb -> env -> setEnvVal( arg1Id, true );
                else if ( argId == TOK_FALSE )   glb -> env -> setEnvVal( arg1Id, false );
                else    fprintf( stdout, "Expected true or false\n" );
                
            } break;
                
            case ENV_TYP_INT: {
                
                int val = 0;
                if ( sscanf( arg2Str, "%i", &val ) != 1 ) {
                    
                    fprintf( stdout, "Invalid value\n" );
                    return;
                }
                
                glb -> env -> setEnvVal( arg1Id, val );
                
            } break;
                
            case ENV_TYP_UINT: {
                
                uint32_t val = 0;
                if ( sscanf( arg2Str, "%u", &val ) != 1 ) {
                    
                    fprintf( stdout, "Invalid value\n" );
                    return;
                }
                
                glb -> env -> setEnvVal( arg1Id, val );
                
            } break;
                
            case ENV_TYP_STR: {
                
                glb -> env -> setEnvVal( arg1Id, arg2Str );
                
            } break;
                
            default: ;
        }
    }
}

//------------------------------------------------------------------------------------------------------------
// Execute commands from a file command. The actual work is done in the "execCmdsFromFile" routine.
//
// EXEC "<filename>"
//------------------------------------------------------------------------------------------------------------
uint8_t DrvCmds::execFileCmd( ) {
    
    if ( tok -> tokTyp( ) == TYP_STR ) {
        
        execCmdsFromFile( tok -> tokStr( ));
    }
    else fprintf( stdout, "Expected a file path\n" );
    
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
    
    if ( tok -> tokTyp( ) == TYP_SYM ) {
        
        switch( tok -> tokId( )) {
                
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
// STEP [ <num> ] [ "," "I" | "C" ]
//------------------------------------------------------------------------------------------------------------
uint8_t DrvCmds::stepCmd( ) {
    
    uint32_t numOfSteps = 1;
    
    if ( tok -> tokTyp( ) == TYP_NUM ) {
        
        numOfSteps = tok -> tokVal( );
        tok -> nextToken( );
    }
    
    if ( tok -> tokId( ) == TOK_COMMA ) {
        
        tok -> nextToken( );
        
        if      ( tok -> tokId( ) == TOK_I ) glb -> cpu -> instrStep( numOfSteps );
        else if ( tok -> tokId( ) == TOK_C ) glb -> cpu -> clockStep( numOfSteps );
        else    fprintf( stdout, "Invalid step option, use help\n" );
        
    }
    else {
        
        if ( glb -> env -> getEnvValBool( ENV_STEP_IN_CLOCKS )) glb -> cpu -> clockStep( 1 );
        else glb -> cpu -> instrStep( 1 );
    }
    
    return( NO_ERR );
}

//------------------------------------------------------------------------------------------------------------
// Disassemble command.
//
// DIS <instr> [ fmt ]
//------------------------------------------------------------------------------------------------------------
uint8_t DrvCmds::disAssembleCmd( ) {
    
    DrvExpr     rExpr;
    uint32_t    instr   = 0;
    TokId       fmtId   = glb -> env -> getEnvValTok( ENV_FMT_DEF );
    
    if ( ! parseExpr( &rExpr )) {
        
        fprintf( stdout, "Expected an instruction value\n" );
        return( NO_ERR );
    }
    
    instr = rExpr.numVal1;
    
    if (( tok -> tokId( ) == TOK_HEX ) ||
        ( tok -> tokId( ) == TOK_OCT ) ||
        ( tok -> tokId( ) == TOK_DEC )) {
        
        fmtId = tok -> tokId( );
    }
    else if ( tok -> tokId( ) == TOK_EOS ) {
        
        fmtId = glb -> env -> getEnvValTok( ENV_FMT_DEF );
    }
    else return( cmdLineError( ERR_INVALID_FMT_OPT ));
    
    glb -> disAsm -> displayInstr( instr, fmtId );
    
    fprintf( stdout, " (" );
    glb -> lineDisplay -> displayWord( instr, fmtId );
    fprintf( stdout, ")\n" );
    
    return( NO_ERR );
}

//------------------------------------------------------------------------------------------------------------
// Assemble command. We enter the routine with the token past the command token.
//
// ASM <instr-str> [ fmt ]
//------------------------------------------------------------------------------------------------------------
uint8_t DrvCmds::assembleCmd( ) {
    
    TokId       fmtId           = glb -> env -> getEnvValTok( ENV_FMT_DEF );
    uint32_t    instr           = 0;
    char        asmStr[ 256 ]   = { 0 };
    
    if ( tok -> tokId( ) == TOK_STR ) {
        
        strncpy( asmStr, tok -> tokStr( ), sizeof( asmStr ));
        
        tok -> nextToken( );
        
        if (( tok -> tokId( ) == TOK_HEX ) ||
            ( tok -> tokId( ) == TOK_OCT ) ||
            ( tok -> tokId( ) == TOK_DEC )) {
            
            fmtId = tok -> tokId( );
        }
        else if ( tok -> tokId( ) == TOK_EOS ) {
            
            fmtId = glb -> env -> getEnvValTok( ENV_FMT_DEF );
        }
        else return( cmdLineError( ERR_INVALID_FMT_OPT ));
    }
    else return( cmdLineError( ERR_INVALID_ARG ));
    
    if ( glb -> oneLineAsm -> parseAsmLine( asmStr, &instr )) {
        
        glb -> lineDisplay -> displayWord( instr, fmtId );
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
    
    TokId   fmtId       = glb -> env -> getEnvValTok( ENV_FMT_DEF );
    TypeId  regSetId    = TYP_GREG;
    TokId   regId       = GR_SET;
    
    if ( tok -> tokId( ) != TOK_EOS ) {
        
        if (( tok -> tokTyp( ) == TYP_GREG )        ||
            ( tok -> tokTyp( ) == TYP_SREG )        ||
            ( tok -> tokTyp( ) == TYP_CREG )        ||
            ( tok -> tokTyp( ) == TYP_PSTATE_PREG ) ||
            ( tok -> tokTyp( ) == TYP_FD_PREG )     ||
            ( tok -> tokTyp( ) == TYP_MA_PREG )     ||
            ( tok -> tokTyp( ) == TYP_EX_PREG )     ||
            ( tok -> tokTyp( ) == TYP_IC_L1_REG )   ||
            ( tok -> tokTyp( ) == TYP_DC_L1_REG )   ||
            ( tok -> tokTyp( ) == TYP_UC_L2_REG )   ||
            ( tok -> tokTyp( ) == TYP_ITLB_REG )    ||
            ( tok -> tokTyp( ) == TYP_DTLB_REG )) {
            
            regSetId    = tok -> tokTyp( );
            regId       = tok -> tokId( );
        }
        else {
            
            fprintf( stdout, "Invalid register or register set\n" );
            return ( NO_ERR );
        }
   
        if ( tok -> tokId( ) == TOK_COMMA ) {
            
            tok -> nextToken( );
            
            if (( tok -> tokId( ) == TOK_HEX ) ||
                ( tok -> tokId( ) == TOK_OCT ) ||
                ( tok -> tokId( ) == TOK_DEC )) {
                
                fmtId = tok -> tokId( );
            }
            else if ( tok -> tokId( ) == TOK_EOS ) {
                
                fmtId = glb -> env -> getEnvValTok( ENV_FMT_DEF );
            }
            else return( cmdLineError( ERR_INVALID_FMT_OPT ));
        }
    }
    
    switch( regSetId ) {
            
        case TYP_GREG: {
            
            if ( regId == GR_SET ) glb -> lineDisplay -> displayGeneralRegSet( fmtId );
            else glb -> lineDisplay -> displayWord( glb -> cpu -> getReg( RC_GEN_REG_SET, ( regId - GR_0 )), fmtId );
            
        } break;
            
        case TYP_SREG: {
            
            if ( regId == SR_SET ) glb -> lineDisplay -> displaySegmentRegSet( fmtId );
            else glb -> lineDisplay -> displayWord( glb -> cpu -> getReg( RC_SEG_REG_SET, ( regId - SR_0 )), fmtId );
            
        } break;
            
        case TYP_CREG: {
            
            if ( regId == CR_SET ) glb -> lineDisplay -> displayControlRegSet( fmtId );
            else glb -> lineDisplay -> displayWord( glb -> cpu -> getReg( RC_CTRL_REG_SET, ( regId - CR_0 )), fmtId );
            
        } break;
            
        case TYP_PSTATE_PREG: {
            
            if ( regId == PS_SET ) glb -> lineDisplay -> displayPStateRegSet( fmtId );
            else glb -> lineDisplay -> displayWord( glb -> cpu -> getReg( RC_PROG_STATE, ( regId - PS_IA_SEG )), fmtId );
            
        } break;
            
        case TYP_IC_L1_REG: {
            
            if ( regId == IC_L1_SET ) glb -> lineDisplay -> displayMemObjRegSet( glb -> cpu -> iCacheL1, fmtId );
            else glb -> lineDisplay -> displayWord( glb -> cpu -> getReg( RC_IC_L1_OBJ, ( regId - IC_L1_STATE )), fmtId );
            
        } break;
            
        case TYP_DC_L1_REG: {
            
            if ( regId == DC_L1_SET ) glb -> lineDisplay -> displayMemObjRegSet( glb -> cpu -> dCacheL1, fmtId );
            else glb -> lineDisplay -> displayWord( glb -> cpu -> getReg( RC_DC_L1_OBJ, ( regId - DC_L1_STATE )), fmtId );
            
        } break;
            
        case TYP_UC_L2_REG: {
            
            if ( glb -> cpu -> uCacheL2 != nullptr ) {
                
                if ( regId == UC_L2_SET ) glb -> lineDisplay -> displayMemObjRegSet( glb -> cpu -> uCacheL2, fmtId );
                else glb -> lineDisplay -> displayWord( glb -> cpu -> getReg( RC_UC_L2_OBJ, ( regId - UC_L2_STATE )), fmtId );
            }
            else fprintf( stdout, "L2 cache not configured \n" );
            
        } break;
            
        case TYP_ITLB_REG: {
            
            if ( regId == ITLB_SET ) glb -> lineDisplay -> displayTlbObjRegSet( glb -> cpu -> iTlb, fmtId );
            else glb -> lineDisplay -> displayWord( glb -> cpu -> getReg( RC_ITLB_OBJ, ( regId - ITLB_STATE )), fmtId );
            
        } break;
            
        case TYP_DTLB_REG: {
            
            if ( regId == DTLB_SET ) glb -> lineDisplay -> displayTlbObjRegSet( glb -> cpu -> dTlb, fmtId );
            else glb -> lineDisplay -> displayWord( glb -> cpu -> getReg( RC_DTLB_OBJ, ( regId - DTLB_STATE )), fmtId );
            
        } break;
            
        case TYP_FD_PREG: {
            
            glb -> lineDisplay -> displayPlIFetchDecodeRegSet( fmtId );
            
        } break;
            
        case TYP_MA_PREG: {
            
            glb -> lineDisplay -> displayPlMemoryAccessRegSet( fmtId );
            
        } break;
            
        case TYP_EX_PREG: {
            
            glb -> lineDisplay -> displayPlExecuteRegSet( fmtId );
            
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
//
// ??? PSTATE regs and FD Stage Regs are the same ?????
//------------------------------------------------------------------------------------------------------------
uint8_t DrvCmds::modifyRegCmd( ) {
    
    TypeId      regSetId    = TYP_GREG;
    TokId       regId       = TOK_NIL;
    uint32_t    val         = 0;
    DrvExpr     rExpr;
    
    if (( tok -> tokTyp( ) == TYP_GREG )        ||
        ( tok -> tokTyp( ) == TYP_SREG )        ||
        ( tok -> tokTyp( ) == TYP_CREG )        ||
        ( tok -> tokTyp( ) == TYP_PSTATE_PREG ) ||
        ( tok -> tokTyp( ) == TYP_FD_PREG )     ||
        ( tok -> tokTyp( ) == TYP_MA_PREG )     ||
        ( tok -> tokTyp( ) == TYP_EX_PREG )     ||
        ( tok -> tokTyp( ) == TYP_IC_L1_REG )   ||
        ( tok -> tokTyp( ) == TYP_DC_L1_REG )   ||
        ( tok -> tokTyp( ) == TYP_UC_L2_REG )   ||
        ( tok -> tokTyp( ) == TYP_ITLB_REG )    ||
        ( tok -> tokTyp( ) == TYP_DTLB_REG )) {
        
        regSetId    = tok -> tokTyp( );
        regId       = tok -> tokId( );
        
        tok -> nextToken( );
    }
    else {
        
        fprintf( stdout, "Invalid register\n" );
        return ( NO_ERR );
    }
    
    if ( tok -> tokId( ) == TOK_EOS ) {
        
        fprintf( stdout, "Expected a value\n" );
        return( NO_ERR );
    }
    
    if ( ! parseExpr( &rExpr )) {
        
        fprintf( stdout, "Invalid value\n" );
        return( NO_ERR );
    }
    else val = rExpr.numVal1;
    
    switch( regSetId ) {
            
        case TYP_GREG:          glb -> cpu -> setReg( RC_GEN_REG_SET, ( regId - GR_0 ), val );  break;
        case TYP_SREG:          glb -> cpu -> setReg( RC_SEG_REG_SET, ( regId - SR_0 ), val );  break;
        case TYP_CREG:          glb -> cpu -> setReg( RC_CTRL_REG_SET, ( regId - CR_0 ), val ); break;
        case TYP_PSTATE_PREG:   glb -> cpu -> setReg( RC_PROG_STATE, regId - PS_IA_SEG, val );  break;
        case TYP_FD_PREG:       glb -> cpu -> setReg( RC_FD_PSTAGE, regId - PS_IA_SEG, val );   break;
        case TYP_MA_PREG:       glb -> cpu -> setReg( RC_MA_PSTAGE, regId - PS_IA_SEG, val );   break;
        case TYP_EX_PREG:       glb -> cpu -> setReg( RC_EX_PSTAGE, regId - EX_IA_SEG, val );   break;
        case TYP_IC_L1_REG:     glb -> cpu -> setReg( RC_IC_L1_OBJ, regId - IC_L1_STATE, val ); break;
        case TYP_DC_L1_REG:     glb -> cpu -> setReg( RC_DC_L1_OBJ, regId - DC_L1_STATE, val ); break;
        case TYP_UC_L2_REG:     glb -> cpu -> setReg( RC_UC_L2_OBJ, regId - UC_L2_STATE, val ); break;
        case TYP_ITLB_REG:      glb -> cpu -> setReg( RC_ITLB_OBJ, regId - ITLB_STATE, val );   break;
        case TYP_DTLB_REG:      glb -> cpu -> setReg( RC_DTLB_OBJ, regId - DTLB_STATE, val );   break;
            
        default:  fprintf( stdout, "Invalid Reg Set for operation\n" );
    }
    
    return( NO_ERR );
}

//------------------------------------------------------------------------------------------------------------
// Hash virtual address command. The TLB is indexed by a hash function, which we can test with this command.
// We will use the iTlb hash function for this command.
//
// HVA <seg> <ofs>
//
// ??? need to implement a virtual adress in tokenizer ...
//------------------------------------------------------------------------------------------------------------
void DrvCmds::hashVACmd( char *cmdBuf ) {
    
    char        cmdStr[ TOK_NAME_SIZE + 1 ] = "";
    uint32_t    seg                         = 0;
    uint32_t    ofs                         = 0;
    
    if( sscanf( cmdBuf, "%32s %i %i", cmdStr, &seg, &ofs ) <= 3 ) {
        
        fprintf( stdout, "To do ...\n" );
        fprintf( stdout, "%i\n", glb -> cpu ->iTlb ->hashAdr( seg, ofs ));
    }
    else fprintf( stdout, "Expected a virtual address\n" );
}

//------------------------------------------------------------------------------------------------------------
// Display TLB entries command.
//
// DTLB ( D | I ) [ <index> ] [ "," <len> ] [ "," <fmt> ] - if no index, list all entries ? practical ?
//------------------------------------------------------------------------------------------------------------
uint8_t DrvCmds::displayTLBCmd( ) {
 
    uint32_t    ofs         = 0;
    uint32_t    len         = 0;
    uint32_t    tlbSize     = 0;
    TokId       tlbTypeId   = TOK_I;
    TokId       fmtId       = glb -> env -> getEnvValTok( ENV_FMT_DEF );
    
    if ( tok -> tokId( ) == TOK_I ) {
        
        tlbSize     = glb -> cpu -> iTlb -> getTlbSize( );
        tlbTypeId   = TOK_I;
        tok -> nextToken( );
    }
    else if ( tok -> tokId( ) == TOK_D ) {
        
        tlbSize     = glb -> cpu -> dTlb -> getTlbSize( );
        tlbTypeId   = TOK_D;
        tok -> nextToken( );
    }
    else {
        
        fprintf( stdout, "Expected an I or D \n" );
        return( NO_ERR );
    }
    
    if ( tok -> tokId( ) == TOK_COMMA ) {
        
        ofs = 0;
        tok -> nextToken( );
    }
    else {
        
        DrvExpr rExpr;
        
        if ( parseExpr( &rExpr )) {
            
            ofs = rExpr.numVal1;
        }
        else {
            
            printf( "Expected the start offset\n" );
            return( NO_ERR );
        }
    }
    
    if ( tok -> tokId( ) == TOK_COMMA ) {
        
        len = 1;
        tok -> nextToken( );
    }
    else {
        
        DrvExpr rExpr;
        
        if ( parseExpr( &rExpr )) {
            
            len = rExpr.numVal1;
        }
        else {
            
            printf( "Expected the start offset\n" );
            return( NO_ERR );
        }
    }
    
    if ( tok -> tokId( ) == TOK_COMMA ) {
        
        tok -> nextToken( );
        
        if (( tok -> tokId( ) == TOK_HEX ) ||
            ( tok -> tokId( ) == TOK_OCT ) ||
            ( tok -> tokId( ) == TOK_DEC )) {
            
            fmtId = tok -> tokId( );
        }
        else if ( tok -> tokId( ) == TOK_EOS ) {
            
            fmtId = glb -> env -> getEnvValTok( ENV_FMT_DEF );
        }
        else return( cmdLineError( ERR_INVALID_FMT_OPT ));
    }
    
    if (( ofs > tlbSize ) || ( ofs + len > tlbSize )) {
        
        fprintf( stdout, "Index / Len exceed TLB size\n" );
        return( NO_ERR );
    }
    
    if (( ofs == 0 ) && ( len == 0 )) len = tlbSize;
    
    if      ( tlbTypeId == TOK_I ) glb -> lineDisplay -> displayTlbEntries( glb -> cpu -> iTlb, ofs, len, fmtId );
    else if ( tlbTypeId == TOK_D ) glb -> lineDisplay -> displayTlbEntries( glb -> cpu -> dTlb, ofs, len, fmtId );
    
    fprintf( stdout, "\n" );
    return( NO_ERR );
}




// ???? continue from here........




//------------------------------------------------------------------------------------------------------------
// Purge from TLB command.
//
// P-TLB <I|D|U> <seg> <ofs>
//------------------------------------------------------------------------------------------------------------
void DrvCmds::purgeTLBCmd( char *cmdBuf ) {
    
    char        cmdStr[ TOK_NAME_SIZE ]     = "";
    char        tlbTypStr[ TOK_NAME_SIZE ]  = "";
    uint32_t    seg                         = 0;
    uint32_t    ofs                         = 0;
    
    int         args        = sscanf( cmdBuf, FMT_STR_2S_2D, cmdStr, tlbTypStr, &seg, &ofs );
    TokId       tlbTypId    = lookupTokId( tlbTypStr );
    
    if (( args < 2 ) || (( tlbTypId != TOK_I ) && ( tlbTypId != TOK_D ))) {
        
        fprintf( stdout, "Expected TLB type\n" );
        return;
    }
    
    if ( args < 4 ) {
        
        fprintf( stdout, "Expected a virtual address\n" );
        return;
    }
    
    CpuTlb *tlbPtr = ( tlbTypId == TOK_I ) ? glb -> cpu -> iTlb : glb -> cpu -> dTlb;
    if ( ! tlbPtr -> purgeTlbEntryData( seg, ofs )) printf( "Purge TLB data failed\n" );
}

//------------------------------------------------------------------------------------------------------------
// Insert into TLB command.
//
// I-TLB <D|I> <seg> <ofs> <arg-acc> <arg-adr>
//------------------------------------------------------------------------------------------------------------
void DrvCmds::insertTLBCmd( char *cmdBuf ) {
    
    char        cmdStr[ TOK_NAME_SIZE ]     = "";
    char        tlbTypStr[ TOK_NAME_SIZE ]  = "";
    uint32_t    seg                         = 0;
    uint32_t    ofs                         = 0;
    uint32_t    argAcc                      = 0;
    uint32_t    argAdr                      = 0;
    
    int         args        = sscanf( cmdBuf, FMT_STR_2S_4D, cmdStr, tlbTypStr, &seg, &ofs, &argAcc, &argAdr );
    TokId       tlbTypId    = lookupTokId( tlbTypStr );
    
    if (( args < 2 ) || (( tlbTypId != TOK_I ) && ( tlbTypId != TOK_D ))) {
        
        fprintf( stdout, "Expected TLB type\n" );
        return;
    }
    
    if ( args < 6 ) {
        
        fprintf( stdout, "Expected virtual address and TLB data\n" );
        return;
    }
    
    CpuTlb *tlbPtr = ( tlbTypId == TOK_I ) ? glb -> cpu -> iTlb : glb -> cpu -> dTlb;
    if ( ! tlbPtr -> insertTlbEntryData( seg, ofs, argAcc, argAdr )) printf( "Insert TLB data failed\n" );
}

//------------------------------------------------------------------------------------------------------------
// Display cache entries command.
//
// D-CACHE ( I|D|U ) [ <index> ] [ <len> ] [ <fmt> ]
//------------------------------------------------------------------------------------------------------------
void  DrvCmds::displayCacheCmd( char *cmdBuf ) {
    
    char        cmdStr[ TOK_NAME_SIZE + 1 ]     = "";
    char        cTypStr[ TOK_NAME_SIZE + 1 ]    = "";
    char        fmtStr[ TOK_NAME_SIZE + 1 ]     = "";
    uint32_t    ofs                             = 0;
    uint32_t    len                             = 0;
    
    CpuMem      *cPtr                           = nullptr;
    
    int         args    = sscanf( cmdBuf, FMT_STR_2S_2U_1S, cmdStr, cTypStr, &ofs, &len, fmtStr );
    TokId       fmtId   = glb -> env -> getEnvValTok( ENV_FMT_DEF );
    TokId       cTypId  = lookupTokId( cTypStr );
    
    if ( args < 2 ) {
        
        fprintf( stdout, "Expected cache type\n" );
        return;
    }
    
    if      ( cTypId == TOK_I ) cPtr = glb -> cpu -> iCacheL1;
    else if ( cTypId == TOK_D ) cPtr = glb -> cpu -> dCacheL1;
    else if ( cTypId == TOK_U ) cPtr = glb -> cpu -> uCacheL2;
    else {
        
        fprintf( stdout, "Expected an I, D or U for cache type\n" );
        return;
    }
    
    if ( cPtr != nullptr ) {
        
        uint32_t blockEntries = cPtr -> getBlockEntries( );
        
        if (( ofs > blockEntries ) || ( ofs + len > blockEntries )) {
            
            fprintf( stdout, "Index / Len exceed cache size\n" );
        }
        
        if (( ofs == 0 ) && ( len == 0 )) len = blockEntries;
        
        glb -> lineDisplay -> displayCacheEntries( cPtr, ofs, len, fmtId );
        
        fprintf( stdout, "\n" );
    }
    else fprintf( stdout, "Cache type not configured\n" );
}

//------------------------------------------------------------------------------------------------------------
// Purges a cache line from the cache.
//
// P-CACHE <I|D|U> <index> <set> [<flush>]
//------------------------------------------------------------------------------------------------------------
void  DrvCmds::purgeCacheCmd( char *cmdBuf ) {
    
    char        cmdStr[ TOK_NAME_SIZE ]         = "";
    char        cTypStr[ TOK_NAME_SIZE ]        = "";
    char        flushOptStr[ TOK_NAME_SIZE ]    = "";
    uint32_t    index                           = 0;
    uint32_t    set                             = 0;
    CpuMem      *cPtr                           = nullptr;
    int         args    = sscanf( cmdBuf, FMT_STR_2S_2U_1S, cmdStr, cTypStr, &index, &set, flushOptStr );
    TokId       fOptId  = lookupTokId( flushOptStr, TOK_NIL );
    TokId       cTypId  = lookupTokId( cTypStr, TOK_NIL );
    
    if (( args < 2 ) || (( cTypId != TOK_I ) && ( cTypId != TOK_D ) && ( cTypId != TOK_U ))) {
        
        fprintf( stdout, "Expected cache type\n" );
        return;
    }
    
    if ( args < 3 ) {
        
        fprintf( stdout, "Expected a cache line index\n" );
        return;
    }
    
    if (( fOptId != TOK_NIL ) && ( fOptId != TOK_F )) {
        
        fprintf( stdout, "Expected a flush option\n" );
        return;
    }
    
    if      ( cTypId == TOK_I ) cPtr = glb -> cpu -> iCacheL1;
    else if ( cTypId == TOK_D ) cPtr = glb -> cpu -> dCacheL1;
    else                        cPtr = glb -> cpu -> uCacheL2;
    
    if ( cPtr != nullptr ) {
        
        if ( set > cPtr -> getBlockSets( ) - 1 ) {
            
            fprintf( stdout, "Invalid cache set number\n" );
            return;
        }
        
        MemTagEntry  *tagEntry = cPtr -> getMemTagEntry( index, set );
        if ( tagEntry != nullptr ) {
            
            tagEntry -> valid = false;
        }
        else fprintf( stdout, "Cache Operation failed\n" );
    }
}

//------------------------------------------------------------------------------------------------------------
// Display absolute memory command. The memory address is a byte address. The offset address is a byte address,
// the length is measured in bytes, rounded up to the a word size. We accept any address and length and only
// check that the offset plus length does not exceed the address space. The display routines, who will call
// the actual memory object will take care of gaps in the memory address range.
//
// DA <ofs> [ <len> [ <fmt> ]]
//------------------------------------------------------------------------------------------------------------
void DrvCmds::displayAbsMemCmd( char *cmdBuf ) {
    
    char        cmdStr[ TOK_NAME_SIZE + 1 ] = "";
    char        fmtStr[ TOK_NAME_SIZE + 1 ] = "";
    uint32_t    ofs                         = 0;
    uint32_t    len                         = 1;
    
    int         args    = sscanf( cmdBuf, "%32s %i %i %32s", cmdStr, &ofs, &len, fmtStr );
    TokId       fmtId   = glb -> env -> getEnvValTok( ENV_FMT_DEF );
    
    if ( args < 2 ) {
        
        fprintf( stdout, "Expected physical address offset\n" );
        return;
    }
    
    if (((uint64_t) ofs + len ) > UINT32_MAX ) {
        
        fprintf( stdout, "Offset / Len exceeds physical address range\n" );
        return;
    }
    
    if ( strlen( fmtStr ) > 0 ) {
        
        TokId argId = matchFmtOptions( fmtStr );
        if ( argId == TOK_NIL ) {
            
            fprintf( stdout, "Invalid format option\n" );
            return;
        }
        else fmtId = argId;
    }
    
    glb -> lineDisplay -> displayAbsMemContent( ofs, len, fmtId );
}

//------------------------------------------------------------------------------------------------------------
// Display absolute memory as code command. Similar to the "DA" command, except it will show the data as
// code in assembly syntax, one word disassembled per line.
//
// DAA <ofs> [ <len> [ <fmt> ]]
//------------------------------------------------------------------------------------------------------------
void DrvCmds::displayAbsMemAsCodeCmd( char *cmdBuf ) {
    
    char        cmdStr[ TOK_NAME_SIZE + 1 ] = "";
    char        fmtStr[ TOK_NAME_SIZE + 1 ] = "";
    uint32_t    ofs                         = 0;
    uint32_t    len                         = 4;
    
    int         args    = sscanf( cmdBuf, "%32s %i %i %32s", cmdStr, &ofs, &len, fmtStr );
    TokId       fmtId   = glb -> env -> getEnvValTok( ENV_FMT_DEF );
    
    if ( args < 2 ) {
        
        fprintf( stdout, "Expected physical address offset\n" );
        return;
    }
    
    if (((uint64_t) ofs + len ) > UINT32_MAX ) {
        
        fprintf( stdout, "Offset / Len exceeds physical address range\n" );
        return;
    }
    
    if ( strlen( fmtStr ) > 0 ) {
        
        TokId argId = matchFmtOptions( fmtStr );
        if ( argId == TOK_NIL ) {
            
            fprintf( stdout, "Invalid format option\n" );
            return;
        }
        else fmtId = argId;
    }
    
    glb -> lineDisplay -> displayAbsMemContentAsCode( ofs, len, fmtId );
}


//------------------------------------------------------------------------------------------------------------
// Modify absolute memory command. This command accepts data values for up to eight consecutive locations.
// We also use this command to populate physical memory from a script file.
//
// MA <ofs> <val1> [ <val2> [ <val3> [ <val4> [ <val5> [ <val6> [ <val7> [ <val8> ]]]]]]]
//------------------------------------------------------------------------------------------------------------
void DrvCmds::modifyAbsMemCmd( char *cmdBuf ) {
    
    char        cmdStr[ TOK_NAME_SIZE + 1 ] = "";
    uint32_t    ofs                         = 0;
    uint32_t    val1                        = 0;
    uint32_t    val2                        = 0;
    uint32_t    val3                        = 0;
    uint32_t    val4                        = 0;
    uint32_t    val5                        = 0;
    uint32_t    val6                        = 0;
    uint32_t    val7                        = 0;
    uint32_t    val8                        = 0;
    
    CpuMem      *physMem                    = glb -> cpu -> physMem;
    CpuMem      *pdcMem                     = glb -> cpu -> pdcMem;
    CpuMem      *ioMem                      = glb -> cpu -> ioMem;
    CpuMem      *mem                        = nullptr;
    
    int         args        = sscanf( cmdBuf, "%32s %i %i %i %i %i %i %i %i %i", cmdStr, &ofs,
                                     &val1, &val2, &val3, &val4, &val5, &val6, &val7, &val8 );
    
    int         numOfVal    = args - 2;
    
    if (((uint64_t) ofs + numOfVal * 4 ) > UINT32_MAX ) {
        
        fprintf( stdout, "Offset / Len exceeds physical address range\n" );
        return;
    }
    
    if ( args < 3 ) {
        
        fprintf( stdout, "Expected offset / val \n" );
        return;
    }
    
    if      (( physMem != nullptr ) && ( physMem -> validAdr( ofs ))) mem = physMem;
    else if (( pdcMem  != nullptr ) && ( pdcMem  -> validAdr( ofs ))) mem = pdcMem;
    else if (( ioMem   != nullptr ) && ( ioMem   -> validAdr( ofs ))) mem = ioMem;
    
    if ( mem != nullptr ) {
        
        if ( numOfVal >= 1 ) mem -> putMemDataWord( ofs, val1 );
        if ( numOfVal >= 2 ) mem -> putMemDataWord( ofs + 4, val2 );
        if ( numOfVal >= 3 ) mem -> putMemDataWord( ofs + 8, val3 );
        if ( numOfVal >= 4 ) mem -> putMemDataWord( ofs + 12, val4 );
        if ( numOfVal >= 5 ) mem -> putMemDataWord( ofs + 16, val5 );
        if ( numOfVal >= 6 ) mem -> putMemDataWord( ofs + 20, val6 );
        if ( numOfVal >= 7 ) mem -> putMemDataWord( ofs + 24, val7 );
        if ( numOfVal >= 8 ) mem -> putMemDataWord( ofs + 28, val8 );
    }
}

//------------------------------------------------------------------------------------------------------------
// Modify absolute code memory command. This command accepts an address and string that represents the code
// word in assembly format.
//
// MAA <ofs> <asm-string>
//------------------------------------------------------------------------------------------------------------
void DrvCmds::modifyAbsMemAsCodeCmd( char *cmdBuf ) {
    
    char        cmdStr[ TOK_NAME_SIZE + 1 ] = "";
    char        argStr[ TOK_NAME_SIZE + 1 ] = "";
    uint32_t    ofs                         = 0;
    uint32_t    instr                       = 0;
    CpuMem      *physMem                    = glb -> cpu -> physMem;
    CpuMem      *pdcMem                     = glb -> cpu -> pdcMem;
    CpuMem      *ioMem                      = glb -> cpu -> ioMem;
    CpuMem      *mem                        = nullptr;
    
    int         args = sscanf( cmdBuf, "%s %i \"%[^\"]\"", cmdStr, &ofs, argStr );
    
    if ( args < 3 ) {
        
        fprintf( stdout, "Expected offset and argument string \n" );
        return;
    }
    
    if (((uint64_t) ofs ) > UINT32_MAX ) {
        
        fprintf( stdout, "Offset / Len exceeds physical address range\n" );
        return;
    }
    
    if      (( physMem != nullptr ) && ( physMem -> validAdr( ofs ))) mem = physMem;
    else if (( pdcMem  != nullptr ) && ( pdcMem  -> validAdr( ofs ))) mem = pdcMem;
    else if (( ioMem   != nullptr ) && ( ioMem   -> validAdr( ofs ))) mem = ioMem;
    
    if ( mem != nullptr ) {
        
        if ( glb -> oneLineAsm -> parseAsmLine( argStr, &instr )) {
            
            mem -> putMemDataWord( ofs, instr );
        }
    }
}

//------------------------------------------------------------------------------------------------------------
// Load physical memory command. All we do is to refer to the script approach of executing a script file
// with a ton of MA commands.
//
// ??? do we load PDC space ? ( it is a ROM .... would be cool to load that place from a file ).
//------------------------------------------------------------------------------------------------------------
void DrvCmds::loadPhysMemCmd( char *cmdBuf ) {
    
    fprintf( stdout, "The Load Physical Memory command....\n" );
    fprintf( stdout, "Just issue an XF command with a file created by the SMF command" );
}

//------------------------------------------------------------------------------------------------------------
// Save physical memory command. We need a simple way to dump out memory. The idea is to store the data as
// a text file that contains as a series of "MA" commands. Each line contains the MA command, the offset,
// which the address irregardless of the bank organization, and 8 words of memory. The line will only be
// written to the file when any of the 8 words in this line is non-zero.
//
// SMF <path> [ <ofs> <len> ]
//
//------------------------------------------------------------------------------------------------------------
void DrvCmds::savePhysMemCmd( char *cmdBuf ) {
    
    char        cmdStr[ TOK_NAME_SIZE + 1 ]     = "";
    char        pathStr[ PATH_STR_SIZE + 1 ]    = "";
    uint32_t    ofs                             = 0;
    uint32_t    len                             = 0;
    uint32_t    wordsPerLine                    = 8;
    
    CpuMem      *physMem                        = glb -> cpu -> physMem;
    uint32_t    blockEntries                    = physMem -> getBlockEntries( );
    uint32_t    blockSize                       = physMem -> getBlockSize( );
    uint32_t    *dataPtr                        = (uint32_t *) physMem -> getMemBlockEntry( 0 );
    
    uint32_t    wordsInMem                      = blockEntries * blockSize / 4;
    uint32_t    wordIndex                       = 0;
    uint32_t    wordLimit                       = 0;
    
    
    int         args = sscanf( cmdBuf, "%32s %256s %i %i", cmdStr, pathStr, &ofs, &len );
    
    if ( args < 2 ) {
        
        fprintf( stdout, "Expected dump file path\n" );
        return;
    }
    
    wordIndex = ofs & 0xFFFFFFFC;
    wordLimit = wordIndex + ( (( len + 3 ) / 4 ) * 4 );
    
    if ( wordLimit > wordsInMem ) {
        
        fprintf( stdout, "Offset plus number of values to write exceeds memory size\n" );
        return;
    }
    
    if ( fopen( pathStr, ((char*) "r" )) != nullptr ) {
        
        if ( promptYesNoCancel((char *) "File already exists, replace ? " ) <= 0 ) return;
    }
    
    fprintf( stdout, "Dumping to \"%s\", start: %i, len: %i\n", pathStr, ofs, len );
    
    FILE *dFile = fopen( pathStr, ((char*) "w" ) );
    if ( dFile == nullptr ) {
        
        fprintf( stdout, "File open error: %s\n", strerror( errno ));
        return;
    }
    
    for ( uint32_t index = wordIndex; index < wordLimit; index = index + wordsPerLine ) {
        
        if (( dataPtr[ index + 0 ] != 0 ) || ( dataPtr[ index + 1 ] != 0 ) ||
            ( dataPtr[ index + 2 ] != 0 ) || ( dataPtr[ index + 3 ] != 0 ) ||
            ( dataPtr[ index + 4 ] != 0 ) || ( dataPtr[ index + 5 ] != 0 ) ||
            ( dataPtr[ index + 6 ] != 0 ) || ( dataPtr[ index + 7 ] != 0 )) {
            
            fprintf( dFile, "MA " );
            
            if ( index == 0 ) fprintf( dFile, "0x00000000 " );
            else fprintf( dFile, "%#010x ", index * 4 );
            
            for ( int i = 0; i < wordsPerLine; i++ ) {
                
                uint32_t tmp = dataPtr[ index + i ];
                
                if ( tmp == 0 ) fprintf( dFile, "0x00000000 " );
                else fprintf( dFile, "%#010x ", tmp );
            }
            
            fprintf( dFile, "\n" );
        }
    }
    
    if ( fclose( dFile ) != 0 ) {
        
        fprintf( stdout, "File close error: %s\n", strerror( errno ));
        return;
    }
}

//------------------------------------------------------------------------------------------------------------
// Global windows commands. There are handlers for turning windows on, off and set them back to their default
// values. We also support two stacks of windows next to each other.
//
//------------------------------------------------------------------------------------------------------------
void DrvCmds::winOnCmd( char *cmdBuf ) {
    
    winModeOn = true;
    glb -> winDisplay -> windowsOn( );
    glb -> winDisplay -> reDraw( true );
}

void DrvCmds::winOffCmd( char *cmdBuf ) {
    
    if ( winModeOn ) {
        
        winModeOn = false;
        glb -> winDisplay -> windowsOff( );
    }
    else printErrMsg( ERR_NOT_IN_WIN_MODE );
}

void DrvCmds::winDefCmd( char *cmdBuf ) {
    
    if ( winModeOn ) {
        
        glb -> winDisplay -> windowDefaults( );
        glb -> winDisplay -> reDraw( true );
    }
    else printErrMsg( ERR_NOT_IN_WIN_MODE );
}

void DrvCmds::winStacksEnable( char *cmdBuf ) {
    
    if ( winModeOn ) {
        
        glb -> winDisplay -> winStacksEnable( true );
        glb -> winDisplay -> reDraw( true );
    }
    else printErrMsg( ERR_NOT_IN_WIN_MODE );
}

void DrvCmds::winStacksDisable( char *cmdBuf ) {
    
    if ( winModeOn ) {
        
        glb -> winDisplay -> winStacksEnable( false );
        glb -> winDisplay -> reDraw( true );
    }
    else printErrMsg( ERR_NOT_IN_WIN_MODE );
}

//------------------------------------------------------------------------------------------------------------
// Window current command. User definable windows are controlled by their window number. To avoid typing this
// number all the time for a user window command, a user window can explicitly be set as the current command.
//
// WC <winNum>
//------------------------------------------------------------------------------------------------------------
void DrvCmds::winCurrentCmd( char *cmdBuf ) {
    
    char    cmdStr[ TOK_NAME_SIZE ] = "";
    int     winNum                  = 0;
    int     args                    = sscanf( cmdBuf, FMT_STR_1S_1D, cmdStr, &winNum );
    
    if ( ! winModeOn ) {
        
        printErrMsg( ERR_NOT_IN_WIN_MODE );
        return;
    }
    
    if ( args < 2 ) {
        
        printErrMsg( ERR_EXPECTED_WIN_ID );
        return;
    }
    
    if ( ! glb -> winDisplay -> validWindowNum( winNum )) {
        
        printErrMsg( ERR_INVALID_WIN_ID );
        return;
    }
    
    glb -> winDisplay -> windowCurrent( lookupTokId( cmdStr, TOK_INV ), winNum );
}

//------------------------------------------------------------------------------------------------------------
// Windows enable and disable. When enabled, a window does show up on the screen. The window number is
// optional, used for user definable windows.
//
// <win>E [<winNum>]
// <win>D [<winNum>]
//------------------------------------------------------------------------------------------------------------
void DrvCmds::winEnableCmd( char *cmdBuf ) {
    
    char    cmdStr[ TOK_NAME_SIZE ] = "";
    int     winNum                  = 0;
    int     args                    = sscanf( cmdBuf, FMT_STR_1S_1D, cmdStr, &winNum );
    
    if ( ! winModeOn ) {
        
        printErrMsg( ERR_NOT_IN_WIN_MODE );
        return;
    }
    
    if ( args < 1 ) {
        
        printErrMsg( ERR_EXPECTED_WIN_ID );
        return;
    }
    
    if ( ! glb -> winDisplay -> validWindowNum( winNum )) {
        
        printErrMsg( ERR_INVALID_WIN_ID );
        return;
    }
    
    glb -> winDisplay -> windowEnable( lookupTokId( cmdStr, TOK_INV ), winNum );
    glb -> winDisplay -> reDraw( true );
}

void DrvCmds::winDisableCmd( char *cmdBuf ) {
    
    char    cmdStr[ TOK_NAME_SIZE ] = "";
    int     winNum  = 0;
    int     args    = sscanf( cmdBuf, FMT_STR_1S_1D, cmdStr, &winNum );
    
    if ( ! winModeOn ) {
        
        printErrMsg( ERR_NOT_IN_WIN_MODE );
        return;
    }
    
    if ( args < 1 ) {
        
        printErrMsg( ERR_EXPECTED_WIN_ID );
        return;
    }
    
    if ( ! glb -> winDisplay -> validWindowNum( winNum )) {
        
        printErrMsg( ERR_INVALID_WIN_ID );
        return;
    }
    
    glb -> winDisplay -> windowDisable( lookupTokId( cmdStr, TOK_INV ), winNum );
    glb -> winDisplay -> reDraw( true );
}

//------------------------------------------------------------------------------------------------------------
// Windows radix. This command sets the radix for a given window. We parse the command and the format
// option and pass the tokens to the screen handler. The window number is optional, used for user definable
// windows.
//
// <win>R [ <radix> [<winNum>]]
//------------------------------------------------------------------------------------------------------------
void DrvCmds::winSetRadixCmd( char *cmdBuf ) {
    
    char    cmdStr[ TOK_NAME_SIZE ] = "";
    char    fmtStr[ TOK_NAME_SIZE ] = "";
    int     winNum                  = 0;
    TokId   fmtId                   = TOK_NIL;
    int     args                    = sscanf( cmdBuf, FMT_STR_2S_1D, cmdStr, fmtStr, &winNum );
    
    if ( args == 0 ) return;
    
    if ( ! winModeOn ) {
        
        printErrMsg( ERR_NOT_IN_WIN_MODE );
        return;
    }
    
    if ( strlen( fmtStr ) > 0 ) {
        
        TokId argId = matchFmtOptions( fmtStr );
        if ( argId == TOK_NIL ) {
            
            printErrMsg( ERR_EXPECTED_FMT_OPT );
            return;
        }
        else fmtId = argId;
    }
    else fmtId = glb -> env -> getEnvValTok( ENV_FMT_DEF );
    
    if ( ! glb -> winDisplay -> validWindowNum( winNum )) {
        
        printErrMsg( ERR_INVALID_WIN_ID );
        return;
    }
    
    glb -> winDisplay -> windowRadix( lookupTokId( cmdStr ), fmtId, winNum );
}

//------------------------------------------------------------------------------------------------------------
// Window scrolling. This command advances the item address of a scrollable window by the number of lines
// multiplied by the number of items on a line forward or backward. The meaning of the item address and line
// items is window dependent. The window number is optional, used for user definable windows. If omitted,
// we mean the current window.
//
// <win>F [<items> [<winNum>]]
// <win>B [<items> [<winNum>]]
//------------------------------------------------------------------------------------------------------------
void DrvCmds::winForwardCmd( char *cmdBuf ) {
    
    char    cmdStr[ TOK_NAME_SIZE ] = "";
    int     winItems                = 0;
    int     winNum                  = 0;
    int     args                    = sscanf( cmdBuf, FMT_STR_1S_2D, cmdStr, &winItems, &winNum );
    
    if ( args == 0 ) return;
    
    if ( ! winModeOn ) {
        
        printErrMsg( ERR_NOT_IN_WIN_MODE );
        return;
    }
    
    if ( ! glb -> winDisplay -> validWindowNum( winNum )) {
        
        printErrMsg( ERR_INVALID_WIN_ID );
        return;
    }
    
    glb -> winDisplay -> windowForward( lookupTokId( cmdStr ), winItems, winNum  );
}

void DrvCmds::winBackwardCmd( char *cmdBuf ) {
    
    char    cmdStr[ TOK_NAME_SIZE ] = "";
    int     winItems                = 0;
    int     winNum                  = 0;
    int     args                    = sscanf( cmdBuf, FMT_STR_1S_2D, cmdStr, &winItems, &winNum );
    
    if ( args == 0 ) return;
    
    if ( ! winModeOn ) {
        
        printErrMsg( ERR_NOT_IN_WIN_MODE );
        return;
    }
    
    if ( ! glb -> winDisplay -> validWindowNum( winNum )) {
        
        printErrMsg( ERR_INVALID_WIN_ID );
        return;
    }
    
    glb -> winDisplay -> windowBackward( lookupTokId( cmdStr ), winItems, winNum  );
}

//------------------------------------------------------------------------------------------------------------
// Window home. Each window has a home item address, which was set at window creation or trough a non-zero
// value passed to this command. The command sets the window item address to this value. The meaning of the
// item address is window dependent. The window number is optional, used for user definable windows.
//
// <win>H [<pos> [<winNum>]]
//------------------------------------------------------------------------------------------------------------
void DrvCmds::winHomeCmd( char *cmdBuf ) {
    
    char    cmdStr[ TOK_NAME_SIZE ] = "";
    int     winPos                  = 0;
    int     winNum                  = 0;
    int     args                    = sscanf( cmdBuf, FMT_STR_1S_2D, cmdStr, &winPos, &winNum );
    
    if ( args == 0 ) return;
    
    if ( ! winModeOn ) {
        
        printErrMsg( ERR_NOT_IN_WIN_MODE );
        return;
    }
    
    if ( ! glb -> winDisplay -> validWindowNum( winNum )) {
        
        printErrMsg( ERR_INVALID_WIN_ID );
        return;
    }
    
    glb -> winDisplay -> windowHome( lookupTokId( cmdStr ), winPos, winNum  );
}

//------------------------------------------------------------------------------------------------------------
// Window jump. The window jump command sets the item address to the position argument. The meaning of the
// item address is window dependent. The window number is optional, used for user definable windows.
//
// <win>J [<pos> [<winNum>]]
//------------------------------------------------------------------------------------------------------------
void DrvCmds::winJumpCmd( char *cmdBuf ) {
    
    char    cmdStr[ TOK_NAME_SIZE ] = "";
    int     winPos                  = 0;
    int     winNum                  = 0;
    int     args                    = sscanf( cmdBuf, FMT_STR_1S_2D, cmdStr, &winPos, &winNum );
    
    if ( args == 0 ) return;
    
    if ( ! winModeOn ) {
        
        printErrMsg( ERR_NOT_IN_WIN_MODE );
        return;
    }
    
    if ( ! glb -> winDisplay -> validWindowNum( winNum )) {
        
        printErrMsg( ERR_INVALID_WIN_ID );
        return;
    }
    
    glb -> winDisplay -> windowJump( lookupTokId( cmdStr ), winPos, winNum );
}

//------------------------------------------------------------------------------------------------------------
// Set window lines. This command sets the the number of rows for a window. The number includes the banner
// line. The window number is optional, used for user definable windows.
//
// <win>L [<lines> [<winNum>]]
//------------------------------------------------------------------------------------------------------------
void DrvCmds::winSetRowsCmd( char *cmdBuf ) {
    
    char    cmdStr[ TOK_NAME_SIZE ] = "";
    int     winLines                = 0;
    int     winNum                  = 0;
    int     args                    = sscanf( cmdBuf, FMT_STR_1S_2D, cmdStr, &winLines, &winNum );
    
    if ( args == 0 ) return;
    
    if ( ! winModeOn ) {
        
        printErrMsg( ERR_NOT_IN_WIN_MODE );
        return;
    }
    
    if ( ! glb -> winDisplay -> validWindowNum( winNum )) {
        
        printErrMsg( ERR_INVALID_WIN_ID );
        return;
    }
    
    glb -> winDisplay -> windowSetRows( lookupTokId( cmdStr ), winLines, winNum );
    glb -> winDisplay -> reDraw( true );
}

//------------------------------------------------------------------------------------------------------------
// This command creates a new user window. The window is assigned a free index form the windows list. This
// index is used in all the calls to this window. The window type allows to select from a code window, a
// physical memory window, a TLB and a CACHE window.
//
// WN <winType> [ <arg> ]
//------------------------------------------------------------------------------------------------------------
void DrvCmds::winNewWinCmd( char *cmdBuf ) {
    
    char    cmdStr[ TOK_NAME_SIZE ]         = "";
    char    winStr[ TOK_NAME_SIZE ]         = "";
    char    argStr[ TOK_LARGE_STR_SIZE ]    = "";
    int     args                            = sscanf( cmdBuf, FMT_STR_2S_LS, cmdStr, winStr, argStr );
    TokId   winType                         = lookupTokId( winStr );
    
    if ( ! winModeOn ) {
        
        printErrMsg( ERR_NOT_IN_WIN_MODE );
        return;
    }
    
    if ( args < 2 ) {
        
        printErrMsg( ERR_EXPECTED_WIN_TYPE );
        return;
    }
    
    if ( ! glb -> winDisplay -> validUserWindowType( winType )) {
        
        printErrMsg( ERR_INVALID_WIN_TYPE);
        return;
    }
    
    if ((( winType == TOK_PM ) && ( glb -> cpu -> physMem == nullptr ))     ||
        (( winType == TOK_PC ) && ( glb -> cpu -> physMem == nullptr ))     ||
        (( winType == TOK_IT ) && ( glb -> cpu -> iTlb == nullptr ))        ||
        (( winType == TOK_DT ) && ( glb -> cpu -> dTlb == nullptr ))        ||
        (( winType == TOK_IC ) && ( glb -> cpu -> iCacheL1 == nullptr ))    ||
        (( winType == TOK_DC ) && ( glb -> cpu -> dCacheL1 == nullptr ))    ||
        (( winType == TOK_UC ) && ( glb -> cpu -> uCacheL2 == nullptr ))) {
        
        fprintf( stdout, "Object for window is not configured \n" );
        return;
    }
    
    glb -> winDisplay -> windowNew( lookupTokId( cmdStr ), winType, argStr );
    glb -> winDisplay -> reDraw( true );
}

//------------------------------------------------------------------------------------------------------------
// This command removes  a user defined window or window range from the list of windows. A number of -1 will
// kill all user defined windows.
//
// WK [<winNumStart> [<winNumEnd]] || ( -1 )
//------------------------------------------------------------------------------------------------------------
void DrvCmds::winKillWinCmd( char * cmdBuf ) {
    
    char    cmdStr[ TOK_NAME_SIZE ] = "";
    int     winNumStart             = 0;
    int     winNumEnd               = 0;
    
    int     args                    = sscanf( cmdBuf, FMT_STR_1S_2D, cmdStr, &winNumStart, &winNumEnd );
    
    if ( ! winModeOn ) {
        
        printErrMsg( ERR_NOT_IN_WIN_MODE );
        return;
    }
    
    if ( args == 1 ) {
        
        winNumStart = glb -> winDisplay -> getCurrentUserWindow( );
        winNumEnd   = winNumStart;
    }
    else if ( args == 2 ) {
        
        if ( winNumStart == -1 ) {
            
            winNumStart = glb -> winDisplay -> getFirstUserWinIndex( );
            winNumEnd   = glb -> winDisplay -> getLastUserWinIndex( );
        }
        else {
            
            if ( ! glb -> winDisplay -> validWindowNum( winNumStart )) {
                
                printErrMsg( ERR_INVALID_WIN_ID );
                return;
            }
            
            winNumEnd = winNumStart;
        }
    }
    else if ( args == 3 ) {
        
        if (( ! glb -> winDisplay -> validWindowNum( winNumStart )) ||
            ( ! glb -> winDisplay -> validWindowNum( winNumEnd ))) {
            
            printErrMsg( ERR_INVALID_WIN_ID );
            return;
        }
    }
    else {
        
        printErrMsg( ERR_INVALID_WIN_ID );
        return;
    }
    
    glb -> winDisplay -> windowKill( lookupTokId( cmdStr ), winNumStart, winNumEnd );
    glb -> winDisplay -> reDraw( true );
}

//------------------------------------------------------------------------------------------------------------
// This command assigns a user window to a stack. User windows can be displayed in a separate stack of
// windows. The first stack is always the main stack, where the predefined and command window can be found.
//
// WS <stackNum> [ <winNumStart> [ <winNumEnd ]]
//------------------------------------------------------------------------------------------------------------
void DrvCmds::winSetStackCmd( char *cmdBuf ) {
    
    char    cmdStr[ TOK_NAME_SIZE ] = "";
    int     winNumStart             = 0;
    int     winNumEnd               = 0;
    int     stackNum                = 0;
    int     args                    = sscanf( cmdBuf, FMT_STR_1S_3D, cmdStr, &stackNum, &winNumStart, &winNumEnd );
    
    if ( ! winModeOn ) {
        
        printErrMsg( ERR_NOT_IN_WIN_MODE );
        return;
    }
    
    if ( args == 1 ) {
        
        winNumStart = glb -> winDisplay -> getCurrentUserWindow( );
        winNumEnd   = winNumStart;
    }
    else if ( args == 2 ) {
        
        winNumStart = glb -> winDisplay -> getCurrentUserWindow( );
        winNumEnd   = winNumStart;
    }
    else if ( args == 3 ) {
        
        if ( winNumStart == -1 ) {
            
            winNumStart = glb -> winDisplay -> getFirstUserWinIndex( );
            winNumEnd   = glb -> winDisplay -> getLastUserWinIndex( );
        }
        else {
            
            if ( ! glb -> winDisplay -> validWindowNum( winNumStart )) {
                
                printErrMsg( ERR_INVALID_WIN_ID );
                return;
            }
            
            winNumEnd = winNumStart;
        }
    }
    else if ( args == 4 ) {
        
        if (( ! glb -> winDisplay -> validWindowNum( winNumStart )) ||
            ( ! glb -> winDisplay -> validWindowNum( winNumEnd ))) {
            
            printErrMsg( ERR_INVALID_WIN_ID );
            return;
        }
    }
    else {
        
        printErrMsg( ERR_EXPECTED_STACK_ID );
        return;
    }
    
    if ( ! glb -> winDisplay -> validWindowStackNum( stackNum )) {
        
        printErrMsg( ERR_INVALID_WIN_STACK_ID );
        return;
    }
    
    glb -> winDisplay -> windowSetStack( stackNum, winNumStart, winNumEnd );
    glb -> winDisplay -> reDraw( true );
}


//------------------------------------------------------------------------------------------------------------
// This command toggles through alternate window content, if the window dos support it. An example is the
// cache sets in a two-way associative cache. The toggle command will just flip through the sets.
//
// WT [ <winNum> ]
//------------------------------------------------------------------------------------------------------------
void  DrvCmds::winToggleCmd( char *cmdBuf ) {
    
    char    cmdStr[ TOK_NAME_SIZE ] = "";
    int     winNum                  = 0;
    int     args                    = sscanf( cmdBuf, FMT_STR_1S_1D, cmdStr, &winNum );
    
    if ( ! winModeOn ) {
        
        printErrMsg( ERR_NOT_IN_WIN_MODE );
        return;
    }
    
    if ( args < 1 ) {
        
        printErrMsg( ERR_EXPECTED_WIN_ID );
        return;
    }
    
    if ( ! glb -> winDisplay -> validWindowNum( winNum )) {
        
        printErrMsg( ERR_INVALID_WIN_ID );
        return;
    }
    
    glb -> winDisplay -> windowToggle( lookupTokId( cmdStr ), winNum );
}

//------------------------------------------------------------------------------------------------------------
// This command exchanges the current user window with the user window specified. It allows to change the
// order of the user windows in a stacks.
//
// WX <winNum>
//------------------------------------------------------------------------------------------------------------
void  DrvCmds::winExchangeCmd( char *cmdBuf ) {
    
    char    cmdStr[ TOK_NAME_SIZE ] = "";
    int     winNum                  = 0;
    int     args                    = sscanf( cmdBuf, FMT_STR_1S_1D, cmdStr, &winNum );
    
    if ( ! winModeOn ) {
        
        printErrMsg( ERR_NOT_IN_WIN_MODE );
        return;
    }
    
    if ( args < 1 ) {
        
        printErrMsg( ERR_EXPECTED_WIN_ID );
        return;
    }
    
    if ( ! glb -> winDisplay -> validUserWindowNum( winNum )) {
        
        printErrMsg( ERR_INVALID_WIN_ID );
        return;
    }
    
    glb -> winDisplay -> windowExchangeOrder( lookupTokId( cmdStr ), winNum );
}

//------------------------------------------------------------------------------------------------------------
// Evaluate input line. There are commands, functions, expressions and so on. This routine sets up the
// tokenizer and dispatches based on the first token in the input line.
//
//------------------------------------------------------------------------------------------------------------
uint8_t DrvCmds::evalInputLine( char *cmdBuf ) {
    
    DrvExpr rExpr;
    
    if ( strlen( cmdBuf ) > 0 ) {
        
        if ( tok -> setupTokenizer( cmdBuf, (DrvToken *) cmdTokTab ) != NO_ERR ) return( ERR_INVALID_CMD );
        tok -> nextToken( );
        
        if ( ! parseExpr( &rExpr )) return( ERR_INVALID_CMD );
        
        switch( rExpr.typ ) {
                
            //------------------------------------------------------------------------------------------------
            // We have a command as the first expression in the input string. Just dispatch to the command.
            //
            // ??? the calls will change and not use the cmdBuf after all commands have been rewore to use
            // the expr mode.
            //------------------------------------------------------------------------------------------------
            case TYP_CMD: {
                
                switch ( rExpr.tokId) {
                        
                    case TOK_NIL:           return( NO_ERR );
                    case CMD_EXIT:          return( exitCmd( ));
                    case CMD_HELP:          return( helpCmd( ));
                    case CMD_WHELP:         return( winHelpCmd( ));
                        
                    case CMD_ENV:           envCmd( cmdBuf);                    break;
                        
                    case CMD_XF:            return( execFileCmd( ));
                    case CMD_RESET:         return( resetCmd( ));
                    case CMD_RUN:           return( runCmd( ));
                    case CMD_STEP:          return( stepCmd( ));
                    case CMD_DIS_ASM:       return( disAssembleCmd( ));
                    case CMD_ASM:           return( assembleCmd( ));
                    case CMD_DR:            return( displayRegCmd( ));
                    case CMD_MR:            return( modifyRegCmd( ));
                        
                        
                    case CMD_HASH_VA:       hashVACmd( cmdBuf);                 break;
                    case CMD_D_TLB:         return( displayTLBCmd( ));          break;
                    case CMD_I_TLB:         insertTLBCmd( cmdBuf );             break;
                    case CMD_P_TLB:         purgeTLBCmd( cmdBuf );              break;
                    case CMD_D_CACHE:       displayCacheCmd( cmdBuf );          break;
                    case CMD_P_CACHE:       purgeCacheCmd( cmdBuf );            break;
                    case CMD_DA:            displayAbsMemCmd( cmdBuf );         break;
                    case CMD_DAA:           displayAbsMemAsCodeCmd( cmdBuf );   break;
                    case CMD_MA:            modifyAbsMemCmd( cmdBuf);           break;
                    case CMD_MAA:           modifyAbsMemAsCodeCmd( cmdBuf);      break;
                    case CMD_LMF:           loadPhysMemCmd( cmdBuf);            break;
                    case CMD_SMF:           savePhysMemCmd( cmdBuf);            break;
                        
                    case CMD_WON:           winOnCmd( cmdBuf );                 break;
                    case CMD_WOFF:          winOffCmd( cmdBuf );                break;
                    case CMD_WDEF:          winDefCmd( cmdBuf );                break;
                    case CMD_WC:            winCurrentCmd( cmdBuf );            break;
                    case CMD_WSE:           winStacksEnable( cmdBuf );          break;
                    case CMD_WSD:           winStacksDisable( cmdBuf );         break;
                    case CMD_WN:            winNewWinCmd( cmdBuf );             break;
                    case CMD_WK:            winKillWinCmd( cmdBuf );            break;
                    case CMD_WS:            winSetStackCmd( cmdBuf );           break;
                    case CMD_WT:            winToggleCmd( cmdBuf );             break;
                    case CMD_WX:            winExchangeCmd( cmdBuf );           break;
                        
                    case CMD_WF:            winForwardCmd( cmdBuf );            break;
                    case CMD_WB:            winBackwardCmd( cmdBuf );           break;
                    case CMD_WH:            winHomeCmd( cmdBuf );               break;
                    case CMD_WJ:            winJumpCmd( cmdBuf );               break;
                        
                    case CMD_PSE:
                    case CMD_SRE:
                    case CMD_PLE:
                    case CMD_SWE:
                    case CMD_WE:            winEnableCmd( cmdBuf );             break;
                        
                    case CMD_PSD:
                    case CMD_SRD:
                    case CMD_PLD:
                    case CMD_SWD:
                    case CMD_WD:            winDisableCmd( cmdBuf );            break;
                        
                    case CMD_PSR:
                    case CMD_SRR:
                    case CMD_PLR:
                    case CMD_SWR:
                    case CMD_WR:            winSetRadixCmd( cmdBuf );           break;
                        
                    case CMD_CWL:
                    case CMD_WL:            winSetRowsCmd( cmdBuf );            break;
                        
                    default:                invalidCmd( );
                }
                
            } break;
                
            //------------------------------------------------------------------------------------------------
            // Am expression result. We just print the value according to its type.
            //
            // ??? would we actually get the values  and display them ?
            //------------------------------------------------------------------------------------------------
            case TYP_NUM:  printf( "%i\n", rExpr.numVal1 );      break;
            case TYP_GREG: printf( "R%d\n", rExpr.numVal1 );     break;
            case TYP_SREG: printf( "S%d\n", rExpr.numVal1 );     break;
            case TYP_CREG: printf( "C%d\n", rExpr.numVal1 );     break;
                
            //------------------------------------------------------------------------------------------------
            // Address values.
            //
            // ??? same question. would we accept just a numerc address and / or also register content ?
            //------------------------------------------------------------------------------------------------
            // case ET_ADR:        printf( "(0x%x)\n", rExpr.adr.ofs );
            // case ET_EXT_ADR:    printf( "(0x%x.0x%x)\n", rExpr.extAdr.seg, rExpr.extAdr.ofs );
                
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
void DrvCmds::cmdLoop( ) {
    
    char cmdLineBuf[ CMD_LINE_BUF_SIZE ] = { 0 };
    
    while ( true ) {
        
        promptCmdLine( );
        if ( readInputLine( cmdLineBuf )) {
            
            evalInputLine( cmdLineBuf );
            if ( winModeOn ) glb -> winDisplay -> reDraw( );
        }
    }
}
