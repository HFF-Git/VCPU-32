//------------------------------------------------------------------------------------------------------------
//
// VCPU32 - A 32-bit CPU - Simulator Commands
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
//      <function>  -> <funcId> “(“ [ <argList> ] ")"
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
// This is a bit of a rewrite of the command interpreter. Maybe later....
//
//------------------------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------------------------
// Local name space. We try to keep utility functions local to the file.
//
//------------------------------------------------------------------------------------------------------------
namespace {

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

struct {
    
    char    name[ TOK_NAME_SIZE ];
    char    aliasName[ TOK_ALIAS_NAME_SIZE ];
    TokId   tokGrpId;
    TokId   tokId;
    
} const tokTab[ ] = {
    
    { "COMMENT",            "#",        CMD_SET,            CMD_COMMENT             },
    { "ENV",                "",         CMD_SET,            CMD_ENV                 },
    { "EXIT",               "E",        CMD_SET,            CMD_EXIT                },
    { "HELP",               "?",        CMD_SET,            CMD_HELP                },
    { "WHELP",              "",         CMD_SET,            CMD_WHELP               },
    { "RESET",              "",         CMD_SET,            CMD_RESET               },
    { "RUN",                "",         CMD_SET,            CMD_RUN                 },
    { "STEP",               "S",        CMD_SET,            CMD_STEP                },
    { "DIS",                "",         CMD_SET,            CMD_DIS_ASM             },
    { "ASM",                "",         CMD_SET,            CMD_ASM                 },
    { "B",                  "",         CMD_SET,            CMD_B                   },
    { "BD",                 "",         CMD_SET,            CMD_BD                  },
    { "BL",                 "",         CMD_SET,            CMD_BL                  },
    { "EXEC-F",             "XF",       CMD_SET,            CMD_XF                  },
    
    { "D-REG",              "DR",       CMD_SET,            CMD_DR                  },
    { "M-REG",              "MR",       CMD_SET,            CMD_MR                  },
    
    { "HASH-VA",            "HVA",      CMD_SET,            CMD_HASH_VA             },
    
    { "I-TLB",              "ITLB",     CMD_SET,            CMD_I_TLB               },
    { "D-TLB",              "DTLB",     CMD_SET,            CMD_D_TLB               },
    { "P-TLB",              "PTLB",     CMD_SET,            CMD_P_TLB               },
    
    { "D-CACHE",            "DCA",      CMD_SET,            CMD_D_CACHE             },
    { "P-CACHE",            "PCA",      CMD_SET,            CMD_P_CACHE             },
    
    { "D-ABS",              "DA",       CMD_SET,            CMD_DA                  },
    { "M-ABS",              "MA",       CMD_SET,            CMD_MA                  },
    { "D-ABS-ASM",          "DAA",      CMD_SET,            CMD_DAA                 },
    { "M-ABS-ASM",          "MAA",      CMD_SET,            CMD_MAA                 },
    
    { "LOAD-MEM",           "LMF",      CMD_SET,            CMD_LMF                 },
    { "SAVE-MEM",           "SMF",      CMD_SET,            CMD_SMF                 },
    
    { "WON",                "",         CMD_SET,            CMD_WON                 },
    { "WOFF",               "",         CMD_SET,            CMD_WOFF                },
    { "WDEF",               "",         CMD_SET,            CMD_WDEF                },
    { "WSE",                "",         CMD_SET,            CMD_WSE                 },
    { "WSD",                "",         CMD_SET,            CMD_WSD                 },
    
    { "PSE",                "",         CMD_SET,            CMD_PSE                 },
    { "PSD",                "",         CMD_SET,            CMD_PSD                 },
    { "PSR",                "",         CMD_SET,            CMD_PSR                 },
    
    { "SRE",                "",         CMD_SET,            CMD_SRE                 },
    { "SRD",                "",         CMD_SET,            CMD_SRD                 },
    { "SRR",                "",         CMD_SET,            CMD_SRR                 },
    
    { "PLE",                "",         CMD_SET,            CMD_PLE                 },
    { "PLD",                "",         CMD_SET,            CMD_PLD                 },
    { "PLR",                "",         CMD_SET,            CMD_PLR                 },
    
    { "SWE",                "",         CMD_SET,            CMD_SWE                 },
    { "SWD",                "",         CMD_SET,            CMD_SWD                 },
    { "SWR",                "",         CMD_SET,            CMD_SWR                 },
    
    { "CWL",                "",         CMD_SET,            CMD_CWL                 },
    
    { "WE",                 "",         CMD_SET,            CMD_WE                  },
    { "WD",                 "",         CMD_SET,            CMD_WD                  },
    { "WR",                 "",         CMD_SET,            CMD_WR                  },
    { "WF",                 "",         CMD_SET,            CMD_WF                  },
    { "WB",                 "",         CMD_SET,            CMD_WB                  },
    { "WH",                 "",         CMD_SET,            CMD_WH                  },
    { "WJ",                 "",         CMD_SET,            CMD_WJ                  },
    { "WL",                 "",         CMD_SET,            CMD_WL                  },
    { "WN",                 "",         CMD_SET,            CMD_WN                  },
    { "WK",                 "",         CMD_SET,            CMD_WK                  },
    { "WC",                 "",         CMD_SET,            CMD_WC                  },
    { "WS",                 "",         CMD_SET,            CMD_WS                  },
    { "WT",                 "",         CMD_SET,            CMD_WT                  },
    { "WX",                 "",         CMD_SET,            CMD_WX                  },
    
    { "TRUE",               "",         SET_NIL,            TOK_TRUE                },
    { "FALSE",              "",         SET_NIL,            TOK_FALSE               },
    { "ALL",                "",         SET_NIL,            TOK_ALL                 },
    { "CPU",                "",         SET_NIL,            TOK_CPU                 },
    { "MEM",                "",         SET_NIL,            TOK_MEM                 },
    { "C",                  "",         SET_NIL,            TOK_C                   },
    { "D",                  "",         SET_NIL,            TOK_D                   },
    { "F",                  "",         SET_NIL,            TOK_F                   },
    { "I",                  "",         SET_NIL,            TOK_I                   },
    { "T",                  "",         SET_NIL,            TOK_T                   },
    { "U",                  "",         SET_NIL,            TOK_U                   },
    
    { "DEC",                "",         FMT_SET,            TOK_DEC                 },
    { "HEX",                "",         FMT_SET,            TOK_HEX                 },
    { "OCT",                "",         FMT_SET,            TOK_OCT                 },
    
    { "PM",                 "",         SET_NIL,            TOK_PM                  },
    { "PC",                 "",         SET_NIL,            TOK_PC                  },
    { "IT",                 "",         SET_NIL,            TOK_IT                  },
    { "DT",                 "",         SET_NIL,            TOK_DT                  },
    { "IC",                 "",         SET_NIL,            TOK_IC                  },
    { "DC",                 "",         SET_NIL,            TOK_DC                  },
    { "UC",                 "",         SET_NIL,            TOK_UC                  },
    { "ICR",                "",         SET_NIL,            TOK_ICR                 },
    { "DCR",                "",         SET_NIL,            TOK_DCR                 },
    { "UCR",                "",         SET_NIL,            TOK_UCR                 },
    { "MCR",                "",         SET_NIL,            TOK_MCR                 },
    { "ITR",                "",         SET_NIL,            TOK_ITR                 },
    { "DTR",                "",         SET_NIL,            TOK_DTR                 },
    { "PCR",                "",         SET_NIL,            TOK_PCR                 },
    { "IOR",                "",         SET_NIL,            TOK_IOR                 },
    { "TX",                 "",         SET_NIL,            TOK_TX                  },
    
    { "R0",                 "TMP",      GR_SET,             GR_0                    },
    { "R1",                 "T1",       GR_SET,             GR_1                    },
    { "R2",                 "T2",       GR_SET,             GR_2                    },
    { "R3",                 "T3",       GR_SET,             GR_3                    },
    { "R4",                 "T4",       GR_SET,             GR_4                    },
    { "R5",                 "",         GR_SET,             GR_5                    },
    { "R6",                 "",         GR_SET,             GR_6                    },
    { "R7",                 "",         GR_SET,             GR_7                    },
    { "R8",                 "",         GR_SET,             GR_8                    },
    { "R9",                 "",         GR_SET,             GR_9                    },
    { "R10",                "",         GR_SET,             GR_10                   },
    { "R11",                "",         GR_SET,             GR_11                   },
    { "R12",                "",         GR_SET,             GR_12                   },
    { "R13",                "DP",       GR_SET,             GR_13                   },
    { "R14",                "RL",       GR_SET,             GR_14                   },
    { "R15",                "SP",       GR_SET,             GR_15                   },

    { "S0",                 "",         SR_SET,             SR_0                    },
    { "S1",                 "",         SR_SET,             SR_1                    },
    { "S2",                 "",         SR_SET,             SR_2                    },
    { "S3",                 "",         SR_SET,             SR_3                    },
    { "S4",                 "",         SR_SET,             SR_4                    },
    { "S5",                 "",         SR_SET,             SR_5                    },
    { "S6",                 "",         SR_SET,             SR_6                    },
    { "S7",                 "",         SR_SET,             SR_7                    },
    
    { "C0",                 "",         CR_SET,             CR_0                    },
    { "C1",                 "",         CR_SET,             CR_1                    },
    { "C2",                 "",         CR_SET,             CR_2                    },
    { "C3",                 "",         CR_SET,             CR_3                    },
    { "C4",                 "",         CR_SET,             CR_4                    },
    { "C5",                 "",         CR_SET,             CR_5                    },
    { "C6",                 "",         CR_SET,             CR_6                    },
    { "C7",                 "",         CR_SET,             CR_7                    },
    { "C8",                 "",         CR_SET,             CR_8                    },
    { "C9",                 "",         CR_SET,             CR_9                    },
    { "C10",                "",         CR_SET,             CR_10                   },
    { "C11",                "",         CR_SET,             CR_11                   },
    { "C12",                "",         CR_SET,             CR_12                   },
    { "C13",                "",         CR_SET,             CR_13                   },
    { "C14",                "",         CR_SET,             CR_14                   },
    { "C15",                "",         CR_SET,             CR_15                   },
    { "C16",                "",         CR_SET,             CR_16                   },
    { "C17",                "",         CR_SET,             CR_17                   },
    { "C18",                "",         CR_SET,             CR_18                   },
    { "C19",                "",         CR_SET,             CR_19                   },
    { "C20",                "",         CR_SET,             CR_20                   },
    { "C21",                "",         CR_SET,             CR_21                   },
    { "C22",                "",         CR_SET,             CR_22                   },
    { "C23",                "",         CR_SET,             CR_23                   },
    { "C24",                "TMP-0",    CR_SET,             CR_24                   },
    { "C25",                "TMP-1",    CR_SET,             CR_25                   },
    { "C26",                "TMP-2",    CR_SET,             CR_26                   },
    { "C27",                "TMP-3",    CR_SET,             CR_27                   },
    { "C28",                "TMP-4",    CR_SET,             CR_28                   },
    { "C29",                "TMP-5",    CR_SET,             CR_29                   },
    { "C30",                "TMP-6",    CR_SET,             CR_30                   },
    { "C31",                "TMP-7",    CR_SET,             CR_31                   },
   
    { "IA-SEG",             "",         PS_SET,             PS_IA_SEG               },
    { "IA-OFS",             "",         PS_SET,             PS_IA_OFS               },
    { "ST-REG",             "",         PS_SET,             PS_STATUS               },
    
    { "FD-IA-SEG",          "",         FD_SET,             FD_IA_SEG               },
    { "FD-IA-OFS",          "",         FD_SET,             FD_IA_OFS               },
    { "FD-INSTR",           "",         FD_SET,             FD_INSTR                },
    { "FD-A",               "",         FD_SET,             FD_A                    },
    { "FD-B",               "",         FD_SET,             FD_B                    },
    { "FD-X",               "",         FD_SET,             FD_X                    },
    
    { "MA-IA-SEG",          "",         MA_SET,             MA_IA_SEG               },
    { "MA-IA-OFS",          "",         MA_SET,             MA_IA_OFS               },
    { "MA-INSTR",           "",         MA_SET,             MA_INSTR                },
    { "MA-A",               "",         MA_SET,             MA_A                    },
    { "MA-B",               "",         MA_SET,             MA_B                    },
    { "MA-X",               "",         MA_SET,             MA_X                    },
    { "MA-S",               "",         MA_SET,             MA_S                    },
    
    { "IC-L1-STATE",        "",         IC_L1_SET,          IC_L1_STATE             },
    { "IC-L1-REQ",          "",         IC_L1_SET,          IC_L1_REQ               },
    { "IC-L1-REQ-SEG",      "",         IC_L1_SET,          IC_L1_REQ_SEG           },
    { "IC-L1-REQ-OFS",      "",         IC_L1_SET,          IC_L1_REQ_OFS           },
    { "IC-L1-REQ-TAG",      "",         IC_L1_SET,          IC_L1_REQ_TAG           },
    { "IC-L1-REQ-LEN",      "",         IC_L1_SET,          IC_L1_REQ_LEN           },
    { "IC-L1-REQ-LAT",      "",         IC_L1_SET,          IC_L1_LATENCY           },
    { "IC-L1-SETS",         "",         IC_L1_SET,          IC_L1_SETS              },
    { "IC-L1-ENTRIES",      "",         IC_L1_SET,          IC_L1_BLOCK_ENTRIES     },
    { "IC-L1-B-SIZE",       "",         IC_L1_SET,          IC_L1_BLOCK_SIZE        },
   
    { "DC-L1-STATE",        "",         DC_L1_SET,          DC_L1_STATE             },
    { "DC-L1-REQ",          "",         DC_L1_SET,          DC_L1_REQ               },
    { "DC-L1-REQ-SEG",      "",         DC_L1_SET,          DC_L1_REQ_SEG           },
    { "DC-L1-REQ-OFS",      "",         DC_L1_SET,          DC_L1_REQ_OFS           },
    { "DC-L1-REQ-TAG",      "",         DC_L1_SET,          DC_L1_REQ_TAG           },
    { "DC-L1-REQ-LEN",      "",         DC_L1_SET,          DC_L1_REQ_LEN           },
    { "DC-L1-REQ-LAT",      "",         DC_L1_SET,          DC_L1_LATENCY           },
    { "DC-L1-SETS",         "",         DC_L1_SET,          DC_L1_SETS              },
    { "DC-L1-ENTRIES",      "",         DC_L1_SET,          DC_L1_BLOCK_ENTRIES     },
    { "DC-L1-B-SIZE",       "",         DC_L1_SET,          DC_L1_BLOCK_SIZE        },
  
    { "UC-L2-STATE",        "",         UC_L2_SET,          UC_L2_STATE             },
    { "UC-L2-REQ",          "",         UC_L2_SET,          UC_L2_REQ               },
    { "UC-L2-REQ-SEG",      "",         UC_L2_SET,          UC_L2_REQ_SEG           },
    { "UC-L2-REQ-OFS",      "",         UC_L2_SET,          UC_L2_REQ_OFS           },
    { "UC-L2-REQ-TAG",      "",         UC_L2_SET,          UC_L2_REQ_TAG           },
    { "UC-L2-REQ-LEN",      "",         UC_L2_SET,          UC_L2_REQ_LEN           },
    { "UC-L2-REQ-LAT",      "",         UC_L2_SET,          UC_L2_LATENCY           },
    { "UC-L2-SETS",         "",         UC_L2_SET,          UC_L2_SETS              },
    { "UC-L2-ENTRIES",      "",         UC_L2_SET,          UC_L2_BLOCK_ENTRIES     },
    { "UC-L2-B-SIZE",       "",         UC_L2_SET,          UC_L2_BLOCK_SIZE        },
    
    { "ITLB-STATE",         "",         ITLB_SET,           ITLB_STATE              },
    { "ITLB-REQ",           "",         ITLB_SET,           ITLB_REQ                },
    { "ITLB-REQ-SEG",       "",         ITLB_SET,           ITLB_REQ_SEG            },
    { "ITLB-REQ-OFS",       "",         ITLB_SET,           ITLB_REQ_OFS            },
    
    { "DTLB-STATE",         "",         DTLB_SET,           DTLB_STATE              },
    { "DTLB-REQ",           "",         DTLB_SET,           DTLB_REQ                },
    { "DTLB-REQ-SEG",       "",         DTLB_SET,           DTLB_REQ_SEG            },
    { "DTLB-REQ-OFS",       "",         DTLB_SET,           DTLB_REQ_OFS            },
    
    { "GR-SET",             "GR",       REG_SET,            GR_SET                  },
    { "SR-SET",             "SR",       REG_SET,            SR_SET                  },
    { "CR-SET",             "CR",       REG_SET,            CR_SET                  },
    { "PS-SET",             "PS",       REG_SET,            PS_SET                  },
    { "PR-SET",             "PR",       REG_SET,            PR_SET                  },
    { "FD-SET",             "PR",       REG_SET,            FD_SET                  },
    { "MA-SET",             "PR",       REG_SET,            MA_SET                  },
    { "IC-L1-SET",          "ICL1",     REG_SET,            IC_L1_SET               },
    { "DC-L1-SET",          "DCL1",     REG_SET,            DC_L1_SET               },
    { "UC-L2-SET",          "UCl2",     REG_SET,            UC_L2_SET               },
    { "ITLB-SET",           "ITRS",     REG_SET,            ITLB_SET                },
    { "DTLB-SET",           "DTRS",     REG_SET,            DTLB_SET                },
    
    { "REG-SET-ALL",        "RS",       REG_SET,            REG_SET_ALL             }
};

const int   TOK_TAB_SIZE  = sizeof( tokTab ) / sizeof( *tokTab );

//------------------------------------------------------------------------------------------------------------
// The command line parser simply uses the "sscanf" library routine. Here are the formats for the various
// command lines. "S" means a string input, "D" a numeric integer input, "U" an unsigned integer input.
//
//------------------------------------------------------------------------------------------------------------
const char  FMT_STR_1S[ ]       = "%32s";
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
const char  FMT_STR_CMD_LINE[ ] = "%256s";

//------------------------------------------------------------------------------------------------------------
// The command line size. The command line is rather long so that we can read in long lines form perhaps
// future script files.
//
//------------------------------------------------------------------------------------------------------------
const int   CMD_LINE_BUF_SIZE  = 256;

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
// A little helper function to remove the comment part of a command line. We do the changes on the buffer
// passed in by just setting the ed of string at the position of the ";" comment indicator.
//
// ??? a bit sloppy, we should ignore "#" inside a string.... also we do not catch more than one "#" ...
//------------------------------------------------------------------------------------------------------------
void removeComment( char *cmdBuf ) {
    
    if ( strlen( cmdBuf ) > 0 ) {
        
        char *tmp = strrchr( cmdBuf, '#' );
        if( tmp != nullptr ) *tmp = 0;
    }
}

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
// Print out an error message.
//
// ??? add all the other text over time ....
//------------------------------------------------------------------------------------------------------------
void DrvCmds::printErrMsg( ErrMsgId errNum, char *argStr ) {
    
    switch ( errNum ) {
            
        case NOT_IN_WIN_MODE_ERR:       fprintf( stdout, "Command only valid in Windows mode\n" ); break;
        case OPEN_EXEC_FILE_ERR:        fprintf( stdout, "Error while opening file: \"%s\"\n", argStr ); break;
        case EXPECTED_FILE_NAME_ERR:    fprintf( stdout, "Expected a file name\n" ); break;
        case INVALID_CMD_ERR:           fprintf( stdout, "Invalid command, use help or whelp\n "); break;
        case INVALID_WIN_STACK_ID:      fprintf( stdout, "Invalid window stack Id\n" ); break;
        case EXPECTED_STACK_ID:         fprintf( stdout, "Expected stack Id\n" ); break;
        case INVALID_WIN_ID:            fprintf( stdout, "Invalid window Id\n" ); break;
        case EXPECTED_WIN_ID:           fprintf( stdout, "Expected a window Id\n" ); break;
        case EXPECTED_FMT_OPT:          fprintf( stdout, "Expected a format option\n" ); break;
        case INVALID_WIN_TYPE:          fprintf( stdout, "Invalid window type\n" ); break;
        case EXPECTED_WIN_TYPE:         fprintf( stdout, "Expected a window type\n" ); break;
        case OUT_OF_WINDOWS_ERR:        fprintf( stdout, "Cannot create more windows\n" ); break;
            
        default: {
            
            fprintf( stdout, "Error: %d", errNum );
            if ( argStr != nullptr ) fprintf( stdout, "%32s", argStr );
            fprintf( stdout, "/n" );
        }
    }
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
// "promtpCmdLine" lists out the prompt string. For now this is just a "->". As development goes on the prompt
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
// "promptYesNoCancel" is a simple function to print a prompt string with a decision question. The answer can
//  be yes/no or cancel. A positive result is a "yes" a negative result a "no", anything else a "cancel".
//
//------------------------------------------------------------------------------------------------------------
int DrvCmds::promptYesNoCancel( char *promptStr ) {
    
    fprintf( stdout, FMT_STR_1S, promptStr );
    fprintf( stdout, " -> " );
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
// "readCmdLine" reads in the command line. For a valid command line, the trailing carriage return and/or
// line feeds are removed and the first token is interpreted as a command. The function returns the command
// found, an invalid command or an empty command line status. We loop inside the routine until we receive
// a valid command line or an EOF.
//
// Warning: on a Mac, fgets does read in a string. On the terminal program configuration, the erase character
// needs to be set to NOT send a control-H to the input. The cursor keys are just echoed to the input line
// and I do not know a way to make them actually move around in the input line.
//
//------------------------------------------------------------------------------------------------------------
bool DrvCmds::readCmdLine( char *cmdBuf ) {
    
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
void  DrvCmds::execCmdsFromFile( char* fileName ) {
    
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
            
                dispatchCmd( cmdLineBuf );
            }
        }
        else printErrMsg( OPEN_EXEC_FILE_ERR, fileName );
    }
    else printErrMsg( EXPECTED_FILE_NAME_ERR  );
}

//------------------------------------------------------------------------------------------------------------
// Help command. With no arguments, a short help overview is printed. If there is an optional argument,
// specific help on the topic is given.
//
//------------------------------------------------------------------------------------------------------------
void DrvCmds::helpCmd( char *cmdBuf ) {
    
    const char FMT_STR[ ] = "%-50s%s\n";
    
    fprintf( stdout, FMT_STR, "help", "displays syntax and a short description" );
    fprintf( stdout, FMT_STR, "#", "echoes the command input" );
    fprintf( stdout, FMT_STR, "exit (e) [<val>]", "program exit" );
    fprintf( stdout, FMT_STR, "env ( ) [<var> [<val>]]", "lists the env tab, a variable, sets a variable" );
    fprintf( stdout, FMT_STR, "reset <mode>", "resets the CPU ( CPU, MEM, STATSm ALL )" );
    fprintf( stdout, FMT_STR, "exec-f (xf) <filename> ", "execute commands from a file" );
    fprintf( stdout, FMT_STR, "run", "run the CPU" );
    fprintf( stdout, FMT_STR, "step (s) [<num>] [I|C]", "single step for instruction or clock cycle" );
    fprintf( stdout, FMT_STR, "b <seg> <ofs>", "sets a break breakpoint at virtual address seg.ofs" );
    fprintf( stdout, FMT_STR, "bd <seg> <ofs>", "deletes a break breakpoint" );
    fprintf( stdout, FMT_STR, "bl", "displays the breakpoint table" );
    fprintf( stdout, FMT_STR, "dr [<regSet>|<reg>] <fmt>]", "display registers" );
    fprintf( stdout, FMT_STR, "mr <reg> <val>", "modify registers" );
    fprintf( stdout, FMT_STR, "da <ofs> [ <len> [ fmt ]]", "display memory" );
    fprintf( stdout, FMT_STR, "daa <ofs> [ <len> [ fmt ]]", "display memory as code" );
    fprintf( stdout, FMT_STR, "ma <ofs> <val>", "modify memory" );
    fprintf( stdout, FMT_STR, "maa <ofs> <asm-str>", "modify memory as code" );
    fprintf( stdout, FMT_STR, "dis <instr>", "disassemble an instruction" );
    fprintf( stdout, FMT_STR, "asm <instr-string>", "assemble an instruction" );
    fprintf( stdout, FMT_STR, "hva <seg> <ofs>",  "returns the hash value function result" );
    fprintf( stdout, FMT_STR, "d-cache (dca) <I|D|U> [<index> <len>]", "display cache content" );
    fprintf( stdout, FMT_STR, "p-cache (pca) <I|D|U> <index> [<F>]", "flushes and purges cache data" );
    fprintf( stdout, FMT_STR, "d-tlb (dtlb) <I|D> [<index> <len>]", "display TLB content" );
    fprintf( stdout, FMT_STR, "i-tlb (itlb) <I|D> <seg> <ofs> <argAcc> <argAdr>", "inserts an entry into the TLB" );
    fprintf( stdout, FMT_STR, "p-tlb (ptlb) <I|D> <seg> <ofs>", "purges an entry from the TLB" );
    fprintf( stdout, FMT_STR, "lmf <path> <opt>", "loads memory from a file in MA command format" );
    fprintf( stdout, FMT_STR, "smf <path> <ofs> <len> ", "stores memory to a file using MA command format" );
    fprintf( stdout, FMT_STR, "won", "switches to windows mode" );
    fprintf( stdout, FMT_STR, "woff", "switches to command line mode" );
    fprintf( stdout, FMT_STR, "wdef", "reset the windows to their default values" );
    fprintf( stdout, FMT_STR, "wse",  "enable window stacks" );
    fprintf( stdout, FMT_STR, "wsd",  "disable window stacks" );
    fprintf( stdout, FMT_STR, "<win><cmd> [<args-list>]", "issue a window command, use whelp for details." );
    fprintf( stdout, "\n" );
}

//------------------------------------------------------------------------------------------------------------
// Display the window specific help.
//
//------------------------------------------------------------------------------------------------------------
void DrvCmds::winHelpCmd( char *cmdBuf ) {
    
    const char FMT_STR[ ] = "%-20s%s\n";
    
    fprintf( stdout, "Windows help \n\n" );
    fprintf( stdout, "General Syntax for Win Commands: <win><cmd> [ args ]\n\n" );
    fprintf( stdout, "Windows:\n" );
    fprintf( stdout, FMT_STR, "PS", "Program state window" );
    fprintf( stdout, FMT_STR, "SR", "Special Register window" );
    fprintf( stdout, FMT_STR, "PL", "CPU Pipeline Registers window" );
    fprintf( stdout, FMT_STR, "ST", "Statistics window" );
    fprintf( stdout, FMT_STR, "IT", "CPU Instruction TLB window" );
    fprintf( stdout, FMT_STR, "DT", "CPU Data TLB window" );
    fprintf( stdout, FMT_STR, "IC", "CPU Instruction Cache (L1) window" );
    fprintf( stdout, FMT_STR, "DC", "CPU Data Cache (L1) window" );
    fprintf( stdout, FMT_STR, "UC", "CPU Unified Cache (L2) window" );
    fprintf( stdout, FMT_STR, "PM", "Physical Memory window" );
    fprintf( stdout, FMT_STR, "PC", "Program Code Window" );
    fprintf( stdout, FMT_STR, "ICR", "CPU Instruction Cache (L1) controller registers" );
    fprintf( stdout, FMT_STR, "DCR", "CPU Data Cache (L1) controller registers" );
    fprintf( stdout, FMT_STR, "UCR", "CPU Unified Cache (L2) controller registers" );
    fprintf( stdout, FMT_STR, "MCR", "Physical Memory controller registers" );
    fprintf( stdout, FMT_STR, "ITR", "CPU Instruction TLB controller registers" );
    fprintf( stdout, FMT_STR, "DTR", "CPU Data TLB controller registers" );
    fprintf( stdout, FMT_STR, "PCR", "PDC Memory controller registers" );
    fprintf( stdout, FMT_STR, "IOR", "IO Memory controller registers" );
    fprintf( stdout, FMT_STR, "TX", "Text Window" );
    fprintf( stdout, FMT_STR, "CW", "Command Line window" );
    fprintf( stdout, FMT_STR, "W",  "User defined window" );
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
}

//------------------------------------------------------------------------------------------------------------
// Invalid command handler.
//
//------------------------------------------------------------------------------------------------------------
void DrvCmds::invalidCmd( char *cmdBuf ) {
    
    glb -> env -> setEnvVal( ENV_EXIT_CODE, -1 );
    printErrMsg( INVALID_CMD_ERR );
}

//------------------------------------------------------------------------------------------------------------
// Exit command. We will exit with the environment variable value for the exit code or the argument value
// in the command. This will be quite useful for test script development.
//
// EXIT <code>
//------------------------------------------------------------------------------------------------------------
void DrvCmds::exitCmd( char *cmdBuf ) {
    
    char    cmdStr[ TOK_NAME_SIZE ]     = "";
    char    arg1Str[ TOK_NAME_SIZE ]    = "";
    int     exitVal                     = 0;
    
    if ( sscanf( cmdBuf, FMT_STR_2S, cmdStr, arg1Str ) >= 1 ) {
        
        if ( strlen( arg1Str ) > 0 ) {
            
            int tmp = sscanf( arg1Str, "%i", &exitVal );
            if ( tmp != 1 ) fprintf( stdout, "Invalid exit code\n" );
            if ( exitVal > 255 ) fprintf( stdout, "Expected an exit code between 0 .. 255\n" );
        }
        else exitVal = glb -> env -> getEnvValInt( ENV_EXIT_CODE );
        
        exit(( exitVal > 255 ) ? 255 : exitVal );
    }
}

//------------------------------------------------------------------------------------------------------------
// Comment command. Just echo the line ... and leave the error status alone. This command is very handy for
// any script file to insert comments in that file.
//
//------------------------------------------------------------------------------------------------------------
void DrvCmds::commentCmd( char *cmdBuf ) {
    
    fprintf( stdout, "%s\n", cmdBuf );
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
// EXEC <filename>
//------------------------------------------------------------------------------------------------------------
void DrvCmds::execFileCmd( char *cmdBuf ) {
    
    char    cmdStr[ TOK_NAME_SIZE + 1 ]         = "";
    char    arg1Str[ TOK_LARGE_STR_SIZE + 1 ]   = "";
    
    if ( sscanf( cmdBuf, "%32s %256s", cmdStr, arg1Str ) >= 2 ) execCmdsFromFile( arg1Str );
    else fprintf( stdout, "Expected a file path\n " );
}

//------------------------------------------------------------------------------------------------------------
// Reset command.
//
// RESET ( CPU | MEM | STATS | ALL )
//------------------------------------------------------------------------------------------------------------
void DrvCmds::resetCmd( char *cmdBuf ) {
    
    char    cmdStr[ TOK_NAME_SIZE + 1 ]     = "";
    char    arg1Str[ TOK_NAME_SIZE + 1 ]    = "";
    int     args                            = sscanf( cmdBuf, FMT_STR_2S, cmdStr, arg1Str );
    
    if ( args < 2 ) {
        
        fprintf( stdout, "Expected CPU/MEM/ALL\n" );
        return;
    }
    
    switch ( lookupTokId( arg1Str )) {
            
        case TOK_CPU: {
            
            glb -> cpu -> reset( );
            
        } break;
            
        case TOK_MEM: {
            
            glb -> cpu -> physMem -> reset( );
            
        } break;
            
        case TOK_STATS: {
            
            // ??? reset statistics....
            
        } break;
            
        case TOK_ALL: {
            
            glb -> cpu -> reset( );
            glb -> cpu -> physMem -> reset( );
            
            // ??? reset statistics....
            
        } break;
            
        default: fprintf( stdout, "Invalid option, use help\n" );
    }
}

//------------------------------------------------------------------------------------------------------------
// Run command. The command will just run the CPU until a "halt" instruction is detected.
//
// RUN
//------------------------------------------------------------------------------------------------------------
void DrvCmds::runCmd( char *cmdBuf ) {
    
    fprintf( stdout, "RUN command to come ... \n" );
    
    // ??? idea: detect a "B 0" instruction. This is an endless loop to itself.
    // ??? drain the pipeline ?
    
    // ??? we could also have the trap handlers use this mechanism...
}

//------------------------------------------------------------------------------------------------------------
// Step command. The command will execute one instruction. Default is one instruction. There is an ENV
// variable that will set the default to be a single clock step.
//
// STEP [ <num> ] [I|C]
//------------------------------------------------------------------------------------------------------------
void DrvCmds::stepCmd( char *cmdBuf ) {
    
    char        cmdStr[ TOK_NAME_SIZE ]     = "";
    uint32_t    numOfSteps                  = 1;
    char        argStr[ TOK_NAME_SIZE ]     = "";
    int         args                        = sscanf( cmdBuf, FMT_STR_1S_1D_1S, cmdStr, &numOfSteps, argStr );
    
    if ( args < 2 ) {
        
        if ( glb -> env -> getEnvValBool( ENV_STEP_IN_CLOCKS )) glb -> cpu -> clockStep( 1 );
        else glb -> cpu -> instrStep( 1 );
    }
    else if ( args < 3 ) {
        
        glb -> cpu -> instrStep( numOfSteps );
    }
    else if ( args == 3 ) {
        
        TokId tmp = lookupTokId( argStr );
        
        if ( tmp == TOK_C ) {
            
            glb -> cpu -> clockStep( numOfSteps );
        }
        else if ( tmp == TOK_I ) {
            
            glb -> cpu -> instrStep( numOfSteps );
        }
        else fprintf( stdout, "Invalid step option, use help\n" );
    }
    else fprintf( stdout, "Invalid number of arguments, use help\n" );
}

//------------------------------------------------------------------------------------------------------------
// Set a breakpoint.
//
// B <seg> <ofs>
//------------------------------------------------------------------------------------------------------------
void DrvCmds::setBreakPointCmd( char *cmdBuf ) {
    
}

//------------------------------------------------------------------------------------------------------------
// Delete a breakpoint.
//
// BD <seg> <ofs>
//------------------------------------------------------------------------------------------------------------
void DrvCmds::deleteBreakPointCmd( char *cmdBuf ){
    
}

//------------------------------------------------------------------------------------------------------------
// List all breakpoints.
//
// BL
//------------------------------------------------------------------------------------------------------------
void DrvCmds::listBreakPointsCmd( char *cmdBuf ) {
    
}

//------------------------------------------------------------------------------------------------------------
// Disassemble command.
//
// DIS <instr> [ fmt ]
//------------------------------------------------------------------------------------------------------------
void DrvCmds::disAssembleCmd( char *cmdBuf ) {
    
    char        cmdStr[ TOK_NAME_SIZE ]     = "";
    uint32_t    instr                       = 0;
    TokId       fmtId                       = glb -> env -> getEnvValTok( ENV_FMT_DEF );
    char        arg1Str[ TOK_NAME_SIZE ]    = "";
    char        arg2Str[ TOK_NAME_SIZE ]    = "";
    int         args = sscanf( cmdBuf, FMT_STR_1S_1D_2S, cmdStr, &instr , arg1Str, arg2Str );
    
    if ( args < 2 ) {
        
        fprintf( stdout, "Expected an instruction value\n" );
        return;
    }
    
    if ( args > 2 ) {
        
        TokId argId = matchFmtOptions( arg2Str );
        if ( argId == TOK_NIL ) {
            
            fprintf( stdout, "Invalid format option\n" );
            return;
        }
        else fmtId = argId;
    }
    
    glb -> disAsm -> displayInstr( instr, fmtId );
        
    fprintf( stdout, " (" );
    glb -> lineDisplay -> displayWord( instr, fmtId );
    fprintf( stdout, ")\n" );
}

//------------------------------------------------------------------------------------------------------------
// Assemble command.
//
// ASM <instr-str> [ fmt ]
//------------------------------------------------------------------------------------------------------------
void DrvCmds::assembleCmd( char *cmdBuf ) {
    
    char        cmdStr[ TOK_NAME_SIZE ]     = "";
    uint32_t    instr                       = 0;
    TokId       fmtId                       = glb -> env -> getEnvValTok( ENV_FMT_DEF );
    char        arg1Str[ TOK_NAME_SIZE ]    = "";
    char        arg2Str[ TOK_NAME_SIZE ]    = "";
    int         args = sscanf( cmdBuf, "%s \"%[^\"]\" %s", cmdStr, arg1Str, arg2Str );
    
    if ( args < 2 ) {
        
        fprintf( stdout, "Expected the assemble string\n" );
        return;
    }
   
    if ( args > 2 ) {
        
        TokId argId = matchFmtOptions( arg2Str );
        if ( argId == TOK_NIL ) {
            
            fprintf( stdout, "Invalid format option\n" );
            return;
        }
        else fmtId = argId;
    }
    
    if ( glb -> oneLineAsm -> parseAsmLine( arg1Str, &instr )) {
        
        glb -> lineDisplay -> displayWord( instr, fmtId );
        fprintf( stdout, "\n" );
    }
}

//------------------------------------------------------------------------------------------------------------
// Display register command. This is a rather versatile command, which displays register set, register and
// all of them in one format.
//
// DR [<regSet>|<reg>] [<fmt>]
//------------------------------------------------------------------------------------------------------------
void DrvCmds::displayRegCmd( char *cmdBuf ) {
    
    char    cmdStr[ TOK_NAME_SIZE ]     = "";
    char    arg1Str[ TOK_NAME_SIZE ]    = "";
    char    arg2Str[ TOK_NAME_SIZE ]    = "";
    int     args                        = sscanf( cmdBuf, FMT_STR_3S, cmdStr, arg1Str, arg2Str );
    
    if ( args < 1 ) return;
    
    TokId   regSetId    = GR_SET;
    TokId   regId       = TOK_NIL;
    TokId   fmtId       = glb -> env -> getEnvValTok( ENV_FMT_DEF );
    
    if ( strlen( arg1Str ) > 0 ) {
        
        TokId argId = matchRegSet( arg1Str );
        if ( argId == TOK_NIL ) {
            
            argId = matchReg( arg1Str );
            if ( argId == TOK_NIL ) {
                
                argId = matchFmtOptions( arg1Str );
                if ( argId == TOK_NIL ) {
                    
                    fprintf( stdout, "Invalid register or register set\n" );
                    return;
                }
                else fmtId = argId;
            }
            else {
                
                regSetId = lookupTokGrpId( argId );
                regId = argId;
            }
        }
        else regSetId = argId;
    }
    
    if ( strlen( arg2Str ) > 0 ) {
        
        TokId argId = matchFmtOptions( arg2Str );
        if ( argId == TOK_NIL ) {
            
            fprintf( stdout, "Invalid format option\n" );
            return;
        }
        else fmtId = argId;
    }
    
    switch( regSetId ) {
            
        case GR_SET: {
            
            if ( regId == TOK_NIL ) glb -> lineDisplay -> displayGeneralRegSet( fmtId );
            else glb -> lineDisplay -> displayWord( glb -> cpu -> getReg( RC_GEN_REG_SET, ( regId - GR_0 )), fmtId );
            
        } break;
            
        case SR_SET: {
            
            if ( regId == TOK_NIL ) glb -> lineDisplay -> displaySegmentRegSet( fmtId );
            else glb -> lineDisplay -> displayWord( glb -> cpu -> getReg( RC_SEG_REG_SET, ( regId - SR_0 )), fmtId );
            
        } break;
            
        case CR_SET: {
            
            if ( regId == TOK_NIL ) glb -> lineDisplay -> displayControlRegSet( fmtId );
            else glb -> lineDisplay -> displayWord( glb -> cpu -> getReg( RC_CTRL_REG_SET, ( regId - CR_0 )), fmtId );
            
        } break;
            
        case PS_SET: {
            
            if ( regId == TOK_NIL ) glb -> lineDisplay -> displayPStateRegSet( fmtId );
            else glb -> lineDisplay -> displayWord( glb -> cpu -> getReg( RC_PROG_STATE, ( regId - PS_IA_SEG )), fmtId );
            
        } break;
            
        case IC_L1_SET: {
            
            if ( regId == TOK_NIL ) glb -> lineDisplay -> displayMemObjRegSet( glb -> cpu -> iCacheL1, fmtId );
            else glb -> lineDisplay -> displayWord( glb -> cpu -> getReg( RC_IC_L1_OBJ, ( regId - IC_L1_STATE )), fmtId );
            
        } break;
            
        case DC_L1_SET: {
            
            if ( regId == TOK_NIL ) glb -> lineDisplay -> displayMemObjRegSet( glb -> cpu -> dCacheL1, fmtId );
            else glb -> lineDisplay -> displayWord( glb -> cpu -> getReg( RC_DC_L1_OBJ, ( regId - DC_L1_STATE )), fmtId );
            
        } break;
            
        case UC_L2_SET: {
            
            if ( glb -> cpu -> uCacheL2 != nullptr ) {
                
                if ( regId == TOK_NIL ) glb -> lineDisplay -> displayMemObjRegSet( glb -> cpu -> uCacheL2, fmtId );
                else glb -> lineDisplay -> displayWord( glb -> cpu -> getReg( RC_UC_L2_OBJ, ( regId - UC_L2_STATE )), fmtId );
            }
            else fprintf( stdout, "L2 cache not configured \n" );
            
        } break;
            
        case ITLB_SET: {
            
            if ( regId == TOK_NIL ) glb -> lineDisplay -> displayTlbObjRegSet( glb -> cpu -> iTlb, fmtId );
            else glb -> lineDisplay -> displayWord( glb -> cpu -> getReg( RC_ITLB_OBJ, ( regId - ITLB_STATE )), fmtId );
            
        } break;
            
        case DTLB_SET: {
            
            if ( regId == TOK_NIL ) glb -> lineDisplay -> displayTlbObjRegSet( glb -> cpu -> dTlb, fmtId );
            else glb -> lineDisplay -> displayWord( glb -> cpu -> getReg( RC_DTLB_OBJ, ( regId - DTLB_STATE )), fmtId );
            
        } break;
      
        case PR_SET:        glb -> lineDisplay -> displayPlRegSets( fmtId );  break;
        case REG_SET_ALL:   glb -> lineDisplay -> displayAllRegSets( fmtId ); break;
            
        default: ;
    }
    
    fprintf( stdout, "\n" );
}

//------------------------------------------------------------------------------------------------------------
// Modify register command. This command modifies a register within a register set.
//
// MR <reg> <val>
//------------------------------------------------------------------------------------------------------------
void DrvCmds::modifyRegCmd( char *cmdBuf ) {
    
    char    cmdStr[ TOK_NAME_SIZE ]     = "";
    char    arg1Str[ TOK_NAME_SIZE ]    = "";
    char    arg2Str[ TOK_NAME_SIZE ]    = "";
    int     args                        = sscanf( cmdBuf, FMT_STR_3S, cmdStr, arg1Str, arg2Str );
    
    if ( args < 3 ) {
        
        fprintf( stdout, "Expected a register and a value\n" );
        return;
    }
    
    TokId   regId       = matchReg( arg1Str );
    TokId   regSetId    = lookupTokGrpId( regId );
    
    if (( regId == TOK_NIL ) || ( lookupTokGrpId( regSetId ) != REG_SET )) {
        
        fprintf( stdout, "Invalid register\n" );
        return;
    }
    
    uint32_t val = 0;
    
    if ( strlen( arg2Str ) > 0 ) {
        
        if ( sscanf( arg2Str, "%i", &val ) == 0 ) {
            
            fprintf( stdout, "Expected a value\n" );
            return;
        }
    }
    
    switch( regSetId ) {
            
        case GR_SET:    glb -> cpu -> setReg( RC_GEN_REG_SET, ( regId - GR_0 ), val );  break;
        case SR_SET:    glb -> cpu -> setReg( RC_SEG_REG_SET, ( regId - SR_0 ), val );  break;
        case CR_SET:    glb -> cpu -> setReg( RC_CTRL_REG_SET, ( regId - CR_0 ), val ); break;
        case PS_SET:    glb -> cpu -> setReg( RC_PROG_STATE, regId - PS_IA_SEG, val );  break;
        case IC_L1_SET: glb -> cpu -> setReg( RC_IC_L1_OBJ, regId - IC_L1_STATE, val ); break;
        case DC_L1_SET: glb -> cpu -> setReg( RC_DC_L1_OBJ, regId - DC_L1_STATE, val ); break;
        case UC_L2_SET: glb -> cpu -> setReg( RC_UC_L2_OBJ, regId - UC_L2_STATE, val ); break;
        case ITLB_SET:  glb -> cpu -> setReg( RC_ITLB_OBJ, regId - ITLB_STATE, val );   break;
        case DTLB_SET:  glb -> cpu -> setReg( RC_DTLB_OBJ, regId - DTLB_STATE, val );   break;
            
        case PR_SET:
        case REG_SET_ALL: fprintf( stdout, "Invalid Reg Set for operation\n" ); return;
        default: ;
    }
}

//------------------------------------------------------------------------------------------------------------
// Hash virtual address command. The TLB is indexed by a hash function, which we can test with this command.
// We will use the iTlb hash function for this command.
//
// HVA <seg> <ofs>
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
// D-TLB ( D | I ) [ <index> ] [ <len> ] [ <fmt> ] - if no index, list all entries ? practical ?
//------------------------------------------------------------------------------------------------------------
void DrvCmds::displayTLBCmd( char *cmdBuf ) {
    
    char        cmdStr[ TOK_NAME_SIZE + 1 ]     = "";
    char        tlbTypStr[ TOK_NAME_SIZE + 1 ]  = "";
    char        fmtStr[ TOK_NAME_SIZE + 1 ]     = "";
    uint32_t    ofs                             = 0;
    uint32_t    len                             = 0;
    uint32_t    tlbSize                         = 0;
    
    int         args        = sscanf( cmdBuf, "%32s %32s %i %i %32s", cmdStr, tlbTypStr, &ofs, &len, fmtStr );
    TokId       fmtId       = glb -> env -> getEnvValTok( ENV_FMT_DEF );
    TokId       tlbTypId    = lookupTokId( tlbTypStr );
    
    if ( args < 2 ) {
        
        fprintf( stdout, "Expected TLB type\n" );
        return;
    }
    
    if      ( tlbTypId == TOK_I ) tlbSize = glb -> cpu -> iTlb -> getTlbSize( );
    else if ( tlbTypId == TOK_D ) tlbSize = glb -> cpu -> dTlb -> getTlbSize( );
    else {
        
        fprintf( stdout, "Expected an I or D \n" );
        return;
    }
    
    if (( ofs > tlbSize ) || ( ofs + len > tlbSize )) {
        
        fprintf( stdout, "Index / Len exceed TLB size\n" );
        return;
    }
    
    if (( ofs == 0 ) && ( len == 0 )) len = tlbSize;
    
    if      ( tlbTypId == TOK_I ) glb -> lineDisplay -> displayTlbEntries( glb -> cpu -> iTlb, ofs, len, fmtId );
    else if ( tlbTypId == TOK_D ) glb -> lineDisplay -> displayTlbEntries( glb -> cpu -> dTlb, ofs, len, fmtId );
    
    fprintf( stdout, "\n" );
}

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
    else printErrMsg( NOT_IN_WIN_MODE_ERR );
}

void DrvCmds::winDefCmd( char *cmdBuf ) {
    
    if ( winModeOn ) {
        
        glb -> winDisplay -> windowDefaults( );
        glb -> winDisplay -> reDraw( true );
    }
    else printErrMsg( NOT_IN_WIN_MODE_ERR );
}

void DrvCmds::winStacksEnable( char *cmdBuf ) {
    
    if ( winModeOn ) {
        
        glb -> winDisplay -> winStacksEnable( true );
        glb -> winDisplay -> reDraw( true );
    }
    else printErrMsg( NOT_IN_WIN_MODE_ERR );
}

void DrvCmds::winStacksDisable( char *cmdBuf ) {
    
    if ( winModeOn ) {
        
        glb -> winDisplay -> winStacksEnable( false );
        glb -> winDisplay -> reDraw( true );
    }
    else printErrMsg( NOT_IN_WIN_MODE_ERR );
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
        
        printErrMsg( NOT_IN_WIN_MODE_ERR );
        return;
    }
    
    if ( args < 2 ) {
        
        printErrMsg( EXPECTED_WIN_ID );
        return;
    }
    
    if ( ! glb -> winDisplay -> validWindowNum( winNum )) {
        
        printErrMsg( INVALID_WIN_ID );
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
        
        printErrMsg( NOT_IN_WIN_MODE_ERR );
        return;
    }
    
    if ( args < 1 ) {
        
        printErrMsg( EXPECTED_WIN_ID );
        return;
    }
    
    if ( ! glb -> winDisplay -> validWindowNum( winNum )) {
        
        printErrMsg( INVALID_WIN_ID );
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
        
        printErrMsg( NOT_IN_WIN_MODE_ERR );
        return;
    }
    
    if ( args < 1 ) {
        
        printErrMsg( EXPECTED_WIN_ID );
        return;
    }
    
    if ( ! glb -> winDisplay -> validWindowNum( winNum )) {
        
        printErrMsg( INVALID_WIN_ID );
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
        
        printErrMsg( NOT_IN_WIN_MODE_ERR );
        return;
    }
    
    if ( strlen( fmtStr ) > 0 ) {
        
        TokId argId = matchFmtOptions( fmtStr );
        if ( argId == TOK_NIL ) {
            
            printErrMsg( EXPECTED_FMT_OPT );
            return;
        }
        else fmtId = argId;
    }
    else fmtId = glb -> env -> getEnvValTok( ENV_FMT_DEF );
    
    if ( ! glb -> winDisplay -> validWindowNum( winNum )) {
        
        printErrMsg( INVALID_WIN_ID );
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
        
        printErrMsg( NOT_IN_WIN_MODE_ERR );
        return;
    }
    
    if ( ! glb -> winDisplay -> validWindowNum( winNum )) {
        
        printErrMsg( INVALID_WIN_ID );
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
        
        printErrMsg( NOT_IN_WIN_MODE_ERR );
        return;
    }
    
    if ( ! glb -> winDisplay -> validWindowNum( winNum )) {
        
        printErrMsg( INVALID_WIN_ID );
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
        
        printErrMsg( NOT_IN_WIN_MODE_ERR );
        return;
    }
    
    if ( ! glb -> winDisplay -> validWindowNum( winNum )) {
        
        printErrMsg( INVALID_WIN_ID );
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
        
        printErrMsg( NOT_IN_WIN_MODE_ERR );
        return;
    }
    
    if ( ! glb -> winDisplay -> validWindowNum( winNum )) {
        
        printErrMsg( INVALID_WIN_ID );
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
        
        printErrMsg( NOT_IN_WIN_MODE_ERR );
        return;
    }
    
    if ( ! glb -> winDisplay -> validWindowNum( winNum )) {
        
        printErrMsg( INVALID_WIN_ID );
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
        
        printErrMsg( NOT_IN_WIN_MODE_ERR );
        return;
    }
    
    if ( args < 2 ) {
        
        printErrMsg( EXPECTED_WIN_TYPE );
        return;
    }
    
    if ( ! glb -> winDisplay -> validUserWindowType( winType )) {
        
        printErrMsg( INVALID_WIN_TYPE);
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
        
        printErrMsg( NOT_IN_WIN_MODE_ERR );
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
                    
                printErrMsg( INVALID_WIN_ID );
                return;
            }
            
            winNumEnd = winNumStart;
        }
    }
    else if ( args == 3 ) {
        
        if (( ! glb -> winDisplay -> validWindowNum( winNumStart )) ||
            ( ! glb -> winDisplay -> validWindowNum( winNumEnd ))) {
                
            printErrMsg( INVALID_WIN_ID );
            return;
        }
    }
    else {
        
        printErrMsg( INVALID_WIN_ID );
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
        
        printErrMsg( NOT_IN_WIN_MODE_ERR );
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
                    
                printErrMsg( INVALID_WIN_ID );
                return;
            }
            
            winNumEnd = winNumStart;
        }
    }
    else if ( args == 4 ) {
        
        if (( ! glb -> winDisplay -> validWindowNum( winNumStart )) ||
            ( ! glb -> winDisplay -> validWindowNum( winNumEnd ))) {
                
            printErrMsg( INVALID_WIN_ID );
            return;
        }
    }
    else {
        
        printErrMsg( EXPECTED_STACK_ID );
        return;
    }
    
     if ( ! glb -> winDisplay -> validWindowStackNum( stackNum )) {
         
         printErrMsg( INVALID_WIN_STACK_ID );
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
        
        printErrMsg( NOT_IN_WIN_MODE_ERR );
        return;
    }
    
    if ( args < 1 ) {
        
        printErrMsg( EXPECTED_WIN_ID );
        return;
    }
    
    if ( ! glb -> winDisplay -> validWindowNum( winNum )) {
        
        printErrMsg( INVALID_WIN_ID );
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
        
        printErrMsg( NOT_IN_WIN_MODE_ERR );
        return;
    }
    
    if ( args < 1 ) {
        
        printErrMsg( EXPECTED_WIN_ID );
        return;
    }
    
    if ( ! glb -> winDisplay -> validUserWindowNum( winNum )) {
        
        printErrMsg( INVALID_WIN_ID );
        return;
    }
    
    glb -> winDisplay -> windowExchangeOrder( lookupTokId( cmdStr ), winNum );
}

//------------------------------------------------------------------------------------------------------------
// Execute command. This routine will scan the command buffer for the command token and branches to the
// respective handler.
//
//------------------------------------------------------------------------------------------------------------
void DrvCmds::dispatchCmd( char *cmdBuf ) {
    
    if ( strlen( cmdBuf ) > 0 ) {
        
        char cmdStr[ CMD_LINE_BUF_SIZE ];
        if ( sscanf( cmdBuf, FMT_STR_CMD_LINE, cmdStr ) > 0 ) {
            
            currentCmd = lookupTokId( cmdStr, TOK_INV );
            
            switch( currentCmd ) {
                    
                case TOK_NIL:                                               break;
                case CMD_COMMENT:       commentCmd( cmdBuf );               break;
                case CMD_EXIT:          exitCmd( cmdBuf );                  break;
                case CMD_HELP:          helpCmd( cmdBuf);                   break;
                case CMD_WHELP:         winHelpCmd( cmdBuf);                break;
                case CMD_ENV:           envCmd( cmdBuf);                    break;
                case CMD_XF:            execFileCmd( cmdBuf );              break;
                case CMD_RESET:         resetCmd( cmdBuf);                  break;
                case CMD_RUN:           runCmd( cmdBuf );                   break;
                case CMD_STEP:          stepCmd( cmdBuf);                   break;
                case CMD_B:             setBreakPointCmd( cmdBuf );         break;
                case CMD_BD:            deleteBreakPointCmd( cmdBuf );      break;
                case CMD_BL:            listBreakPointsCmd( cmdBuf );       break;
                case CMD_DIS_ASM:       disAssembleCmd( cmdBuf );           break;
                case CMD_ASM:           assembleCmd( cmdBuf );              break;
                case CMD_DR:            displayRegCmd( cmdBuf);             break;
                case CMD_MR:            modifyRegCmd( cmdBuf);              break;
                case CMD_HASH_VA:       hashVACmd( cmdBuf);                 break;
                case CMD_D_TLB:         displayTLBCmd( cmdBuf );            break;
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
                    
                case CMD_PSE:   case CMD_SRE:   case CMD_PLE:   case CMD_SWE:   case CMD_WE: {
                    
                    winEnableCmd( cmdBuf );
                    
                } break;
                    
                case CMD_PSD:   case CMD_SRD:   case CMD_PLD:   case CMD_SWD:   case CMD_WD: {
                    
                    winDisableCmd( cmdBuf );
                    
                } break;
                    
                case CMD_PSR:   case CMD_SRR:   case CMD_PLR:   case CMD_SWR:   case CMD_WR: {
                    
                    winSetRadixCmd( cmdBuf );
                    
                } break;
                    
                case CMD_CWL:   case CMD_WL: {
                    
                    winSetRowsCmd( cmdBuf );
                    
                } break;
                    
                default: invalidCmd( cmdBuf );
            }
        }
    }
}

//------------------------------------------------------------------------------------------------------------
// "cmdLoop" is the command interpreter. The basic loop is to prompt for the next command, read the command
// input and dispatch the command. If we are in windows mode, we also redraw the screen.
//
//------------------------------------------------------------------------------------------------------------
void DrvCmds::cmdLoop( ) {
    
    char cmdLineBuf[ CMD_LINE_BUF_SIZE ] = { 0 };
    
    while ( true ) {
        
        promptCmdLine( );
        if ( readCmdLine( cmdLineBuf )) {
            
            dispatchCmd( cmdLineBuf );
            if ( winModeOn ) glb -> winDisplay -> reDraw( );
        }
    }
}
