//------------------------------------------------------------------------------------------------------------
//
//  VCPU32 - A 32-bit CPU - Simulator Driver
//
//------------------------------------------------------------------------------------------------------------
// We need a simple driver for the CPU to do testing and debugging. Well, here it is.
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
#ifndef VCPU32TestDriver_h
#define VCPU32TestDriver_h

//------------------------------------------------------------------------------------------------------------
// Mac and Windows know different include files and procedure names for some POSIX routines.
//
//------------------------------------------------------------------------------------------------------------
#if __APPLE__
#include <unistd.h>
#include <termios.h>
#else
#include <io.h>
#define isatty _isatty
#define fileno _fileno
#define write  _write
#endif

#include "VCPU32-Types.h"
#include "VCPU32-Core.h"



// ??? once more stable, sort the numbers...

//------------------------------------------------------------------------------------------------------------
// Tokens and Expression have a type.
//
//------------------------------------------------------------------------------------------------------------
enum TypeId : uint16_t {

    TYP_NIL                 = 0,        TYP_SYM             = 1,            TYP_IDENT           = 2,
    TYP_NUM                 = 3,        TYP_STR             = 4,            TYP_BOOL            = 5,
    TYP_ADR                 = 6,        TYP_EXT_ADR         = 7,            TYP_CMD             = 8,
    TYP_FUNC                = 9,        TYP_TYPE            = 10,           TYP_OP_CODE         = 11,
    TYP_OP_CODE_S           = 12,
    
    TYP_REG                 = 20,       TYP_REG_PAIR        = 21,
    
    TYP_GREG                = 30,       TYP_SREG            = 31,           TYP_CREG            = 32,
    TYP_PSTATE_PREG         = 33,       TYP_FD_PREG         = 34,           TYP_MA_PREG         = 35,
    TYP_EX_PREG             = 36,
    
    TYP_IC_L1_REG           = 40,       TYP_DC_L1_REG       = 41,           TYP_UC_L2_REG       = 42,
    TYP_MEM_REG             = 43,       TYP_ITLB_REG        = 44,           TYP_DTLB_REG        = 45
    
};

//------------------------------------------------------------------------------------------------------------
// Tokens are the labels for reserved words and symbols recognized by the tokenizer objects. Tokens have a
// name, a token id, a token type and an optional value with further data.
//
//------------------------------------------------------------------------------------------------------------
enum TokId : uint16_t {
    
    // ??? take out ....
    //--------------------------------------------------------------------------------------------------------
    // Token Types. A token has an ID and a type. For example registers have a type such as "general reg" and
    // so on. The types are used by the parser routines to check types and operations on types.
    //
    //--------------------------------------------------------------------------------------------------------
    TOK_TYP_NIL             = 900,      TOK_TYP_SYM             = 901,      TOK_TYP_IDENT           = 902,
    TOK_TYP_NUM             = 903,      TOK_TYP_STR             = 904,      TOK_TYP_BOOL            = 905,
    TOK_TYP_ADR             = 906,      TOK_TYP_EXT_ADR         = 907,      TOK_TYP_CMD             = 908,
    TOK_TYP_FUNC            = 909,      TOK_TYPE_TYPE           = 999,
    
    
    TOK_TYP_GREG            = 910,      TOK_TYP_SREG            = 911,      TOK_TYP_CREG            = 912,
    TOK_TYP_PSTATE_PREG     = 913,      TOK_TYP_FD_PREG         = 914,      TOK_TYP_OF_PREG         = 915,
    TOK_TYP_IC_L1_REG       = 916,      TOK_TYP_DC_L1_REG       = 917,      TOK_TYP_UC_L2_REG       = 918,
    TOK_TYP_MEM_REG         = 919,      TOK_TYP_ITLB_REG        = 920,      TOK_TYP_DTLB_REG        = 921,

    TOK_TYP_OP_CODE         = 930,      TOK_TYP_OP_CODE_S       = 931,
    
    
    // ??? all needed anymore ? They are used in the commands, but perhaps go away...
    ENV_SET                 = 990,
    FMT_SET                 = 991,      REG_SET                 = 994,      REG_SET_ALL             = 995,
    PR_SET                  = 996,
    
    
    //--------------------------------------------------------------------------------------------------------
    // General tokens and symbols.
    //
    //--------------------------------------------------------------------------------------------------------
    TOK_NIL                 = 0,    TOK_ERR                 = 6,    TOK_EOS                 = 7,
    TOK_COMMA               = 60,   TOK_PERIOD              = 61,   TOK_LPAREN              = 62,
    TOK_RPAREN              = 63,   TOK_QUOTE               = 64,   TOK_PLUS                = 65,
    TOK_MINUS               = 66,   TOK_MULT                = 67,   TOK_DIV                 = 68,
    TOK_MOD                 = 69,   TOK_REM                 = 70,   TOK_NEG                 = 71,
    TOK_AND                 = 72,   TOK_OR                  = 73,   TOK_XOR                 = 74,
    TOK_EQ                  = 40,   TOK_NE                  = 41,   TOK_LT                  = 42,
    TOK_GT                  = 43,   TOK_LE                  = 44,   TOK_GE                  = 45,
    
    //--------------------------------------------------------------------------------------------------------
    // Token symbols. They are just reserved names used in commands and functions. Their type and optional
    // value is defined in the token tables.
    //
    //--------------------------------------------------------------------------------------------------------
    TOK_IDENT               = 50,       TOK_NUM                 = 51,       TOK_STR                 = 52,
    
    TOK_TRUE                = 3,        TOK_FALSE               = 4,
    
    TOK_CPU                 = 6,        TOK_MEM                 = 7,        TOK_STATS               = 8,
    
    TOK_C                   = 100,      TOK_D                   = 101,      TOK_F                   = 102,
    TOK_I                   = 103,      TOK_T                   = 104,      TOK_U                   = 105,
    
    TOK_PM                  = 20,       TOK_PC                  = 21,       TOK_IT                  = 22,
    TOK_DT                  = 23,       TOK_IC                  = 24,       TOK_DC                  = 25,
    TOK_UC                  = 26,       TOK_TX                  = 27,
    
    TOK_ICR                 = 28,       TOK_DCR                 = 29,       TOK_UCR                 = 30,
    TOK_ITR                 = 31,       TOK_DTR                 = 32,       TOK_MCR                 = 33,
    TOK_PCR                 = 34,       TOK_IOR                 = 35,
    
    TOK_DEC                 = 12,       TOK_OCT                 = 13,       TOK_HEX                 = 14,
    TOK_CODE                = 15,
    
    
    TOK_DEF                 = 5,
    TOK_INV                 = 1,        TOK_ALL                 = 2,
    
    
   
    //--------------------------------------------------------------------------------------------------------
    // Environment variable tokens.
    //
    // ??? should this become a separate list ?
    // ??? perhaps we want to also allow users to create their own environment variables...
    //--------------------------------------------------------------------------------------------------------
    ENV_TYP_INT             = 500,      ENV_TYP_UINT            = 501,      ENV_TYP_STR             = 502,
    ENV_TYP_BOOL            = 503,      ENV_TYP_TOK             = 504,
    
    ENV_I_TLB_SETS          = 510,      ENV_I_TLB_SIZE          = 511,
    ENV_D_TLB_SETS          = 512,      ENV_D_TLB_SIZE          = 513,
    
    ENV_I_CACHE_SETS        = 520,      ENV_I_CACHE_SIZE        = 521,      ENV_I_CACHE_LINE_SIZE   = 522,
    ENV_D_CACHE_SETS        = 530,      ENV_D_CACHE_SIZE        = 531,      ENV_D_CACHE_LINE_SIZE   = 532,
    ENV_U_CACHE_SETS        = 535,      ENV_U_CACHE_SIZE        = 536,      ENV_U_CACHE_LINE_SIZE   = 537,
    ENV_MEM_SIZE            = 541,      ENV_MEM_BANKS           = 542,      ENV_MEM_BANK_SIZE       = 543,
    ENV_MEM_R_ACC_CYCLE     = 544,      ENV_MEM_W_ACC_CYCLE     = 545,
    
    ENV_CMD_CNT             = 550,      ENV_SHOW_CMD_CNT        = 551,      ENV_ECHO_CMD            = 552,
    ENV_FMT_DEF             = 553,      ENV_EXIT_CODE           = 554,      ENV_WORDS_PER_LINE      = 555,
    ENV_PROG_VERSION        = 556,      ENV_GIT_BRANCH          = 557,      ENV_PROG_PATCH_LEVEL    = 558,
    ENV_STEP_IN_CLOCKS      = 559,      ENV_SHOW_PSTAGE_INFO    = 560,      ENV_PASS_CNT            = 561,
    ENV_FAIL_CNT            = 562,      ENV_WIN_MIN_ROWS        = 563,      ENV_WIN_TX_WIDTH        = 564,
    
    
    
    //--------------------------------------------------------------------------------------------------------
    // Line Commands.
    //
    //--------------------------------------------------------------------------------------------------------
    CMD_ENV                 = 1001,     CMD_EXIT                = 1002,
    CMD_HELP                = 1003,     CMD_WHELP               = 1004,
    
    CMD_RESET               = 1010,     CMD_RUN                 = 1011,     CMD_STEP                = 1012,
    CMD_XF                  = 1013,     CMD_DIS_ASM             = 1014,     CMD_ASM                 = 1015,
    
   // CMD_B                   = 1016,     CMD_BD                  = 1017,     CMD_BL                  = 1018,
    
    CMD_DR                  = 1020,     CMD_MR                  = 1021,
    CMD_DA                  = 1027,     CMD_MA                  = 1028,
    CMD_MAA                 = 1030,
    CMD_LMF                 = 1031,     CMD_SMF                 = 1032,
    
    CMD_HASH_VA             = 1033,
    CMD_D_TLB               = 1034,     CMD_I_TLB               = 1035,     CMD_P_TLB               = 1036,
    CMD_D_CACHE             = 1037,     CMD_P_CACHE             = 1038,
    
    //--------------------------------------------------------------------------------------------------------
    // Window Commands Tokens.
    //
    //--------------------------------------------------------------------------------------------------------
    CMD_WON                 = 2000,     CMD_WOFF                = 2001,     CMD_WDEF                = 2002,
    CMD_CWL                 = 2003,     CMD_WSE                 = 2004,     CMD_WSD                 = 2005,
    
    CMD_PSE                 = 2010,     CMD_PSD                 = 2011,     CMD_PSR                 = 2012,
    CMD_SRE                 = 2015,     CMD_SRD                 = 2016,     CMD_SRR                 = 2017,
    CMD_PLE                 = 2020,     CMD_PLD                 = 2021,     CMD_PLR                 = 2022,
    CMD_SWE                 = 2025,     CMD_SWD                 = 2026,     CMD_SWR                 = 2027,
    
    CMD_WE                  = 2050,     CMD_WD                  = 2051,     CMD_WR                  = 2052,
    CMD_WF                  = 2053,     CMD_WB                  = 2054,     CMD_WH                  = 2055,
    CMD_WJ                  = 2056,     CMD_WL                  = 2057,     CMD_WN                  = 2058,
    CMD_WK                  = 2059,     CMD_WS                  = 2060,     CMD_WC                  = 2061,
    CMD_WT                  = 2062,     CMD_WX                  = 2063,
    
    //--------------------------------------------------------------------------------------------------------
    // Predefined Function Tokens.
    //
    //--------------------------------------------------------------------------------------------------------
    // ??? assign 3000 number range ...
    
    
    //--------------------------------------------------------------------------------------------------------
    // General, Segment and Control Registers Tokens.
    //
    //--------------------------------------------------------------------------------------------------------
    GR_0                    = 4100,     GR_1                    = 4101,     GR_2                    = 4102,
    GR_3                    = 4103,     GR_4                    = 4104,     GR_5                    = 4105,
    GR_6                    = 4106,     GR_7                    = 4107,     GR_8                    = 4108,
    GR_9                    = 4109,     GR_10                   = 4110,     GR_11                   = 4111,
    GR_12                   = 4112,     GR_13                   = 4113,     GR_14                   = 4114,
    GR_15                   = 4115,     GR_SET                  = 4116,
    
    SR_0                    = 4200,     SR_1                    = 4201,     SR_2                    = 4202,
    SR_3                    = 4203,     SR_4                    = 4204,     SR_5                    = 4205,
    SR_6                    = 4206,     SR_7                    = 4207,     SR_SET                  = 4208,
    
    CR_0                    = 4300,     CR_1                    = 4301,     CR_2                    = 4302,
    CR_3                    = 4303,     CR_4                    = 4304,     CR_5                    = 4305,
    CR_6                    = 4306,     CR_7                    = 4307,     CR_8                    = 4308,
    CR_9                    = 4309,     CR_10                   = 4310,     CR_11                   = 4311,
    CR_12                   = 4312,     CR_13                   = 4313,     CR_14                   = 4314,
    CR_15                   = 4315,     CR_16                   = 4316,     CR_17                   = 4317,
    CR_18                   = 4318,     CR_19                   = 4319,     CR_20                   = 4320,
    CR_21                   = 4321,     CR_22                   = 4322,     CR_23                   = 4323,
    CR_24                   = 4324,     CR_25                   = 4325,     CR_26                   = 4326,
    CR_27                   = 4327,     CR_28                   = 4328,     CR_29                   = 4329,
    CR_30                   = 4330,     CR_31                   = 4331,     CR_SET                  = 4332,
    
    FD_PSW0                 = 4500,     FD_PSW1                 = 4501,
    FD_SET                  = 4502,
    
    MA_PSW0                 = 4600,     MA_PSW1                 = 4601,     MA_INSTR                = 4602,
    MA_A                    = 4603,     MA_B                    = 4604,     MA_X                    = 4605,
    MA_S                    = 4606,     MA_SET                  = 4607,
    
    EX_PSW0                 = 4650,     EX_PSW1                 = 4651,     EX_INSTR                = 4652,
    EX_A                    = 4653,     EX_B                    = 4654,     EX_X                    = 4655,
    EX_S                    = 4656,     EX_SET                  = 4657,
    
    IC_L1_STATE             = 4700,     IC_L1_REQ               = 4701,     IC_L1_REQ_SEG           = 4702,
    IC_L1_REQ_OFS           = 4703,     IC_L1_REQ_TAG           = 4704,     IC_L1_REQ_LEN           = 4705,
    IC_L1_LATENCY           = 4706,     IC_L1_BLOCK_ENTRIES     = 4707,     IC_L1_BLOCK_SIZE        = 4708,
    IC_L1_SETS              = 4709,     IC_L1_SET               = 4710,
    
    DC_L1_STATE             = 4720,     DC_L1_REQ               = 4721,     DC_L1_REQ_SEG           = 4722,
    DC_L1_REQ_OFS           = 4723,     DC_L1_REQ_TAG           = 4724,     DC_L1_REQ_LEN           = 4725,
    DC_L1_LATENCY           = 4726,     DC_L1_BLOCK_ENTRIES     = 4727,     DC_L1_BLOCK_SIZE        = 4728,
    DC_L1_SETS              = 4729,     DC_L1_SET               = 4730,
    
    UC_L2_STATE             = 4740,     UC_L2_REQ               = 4741,     UC_L2_REQ_SEG           = 4742,
    UC_L2_REQ_OFS           = 47243,     UC_L2_REQ_TAG          = 4744,     UC_L2_REQ_LEN           = 4745,
    UC_L2_LATENCY           = 4746,     UC_L2_BLOCK_ENTRIES     = 4747,     UC_L2_BLOCK_SIZE        = 4748,
    UC_L2_SETS              = 4749,     UC_L2_SET               = 4750,
    
    ITLB_STATE              = 4800,     ITLB_REQ                = 4801,     ITLB_REQ_SEG            = 4802,
    ITLB_REQ_OFS            = 4803,     ITLB_SET                = 4804,
    
    DTLB_STATE              = 4810,     DTLB_REQ                = 4811,     DTLB_REQ_SEG            = 4812,
    DTLB_REQ_OFS            = 4813,     DTLB_SET                = 4814,

    //--------------------------------------------------------------------------------------------------------
    // OP Code Tokens.
    //
    //--------------------------------------------------------------------------------------------------------
    OP_CODE_LD              = 5000,     OP_CODE_LDB             = 5001,     OP_CODE_LDH             = 5002,
    OP_CODE_LDW             = 5003,     OP_CODE_LDR             = 5004,     OP_CODE_LDA             = 5005,

    OP_CODE_ST              = 5010,     OP_CODE_STB             = 5011,     OP_CODE_STH             = 5012,
    OP_CODE_STW             = 5013,     OP_CODE_STC             = 5014,     OP_CODE_STA             = 5015,

    OP_CODE_ADD             = 5020,     OP_CODE_ADDB            = 5021,     OP_CODE_ADDH            = 5022,
    OP_CODE_ADDW            = 5023,

    OP_CODE_ADC             = 5025,     OP_CODE_ADCB            = 5026,     OP_CODE_ADCH            = 5027,
    OP_CODE_ADCW            = 5028,

    OP_CODE_SUB             = 5030,     OP_CODE_SUBB            = 5031,     OP_CODE_SUBH            = 5032,
    OP_CODE_SUBW            = 5033,

    OP_CODE_SBC             = 5035,     OP_CODE_SBCB            = 5036,     OP_CODE_SBCH            = 5037,
    OP_CODE_SBCW            = 5038,

    OP_CODE_AND             = 5040,     OP_CODE_ANDB            = 5041,     OP_CODE_ANDH            = 5042,
    OP_CODE_ANDW            = 5043,

    OP_CODE_OR              = 5045,     OP_CODE_ORB             = 5046,     OP_CODE_ORH             = 5047,
    OP_CODE_ORW             = 5048,

    OP_CODE_XOR             = 5050,     OP_CODE_XORB            = 5051,     OP_CODE_XORH            = 5052,
    OP_CODE_XORW            = 5053,
    
    OP_CODE_CMP             = 5060,     OP_CODE_CMPB            = 5061,     OP_CODE_CMPH            = 5062,
    OP_CODE_CMPW            = 5063,

    OP_CODE_CMPU            = 5065,     OP_CODE_CMPUB           = 5066,     OP_CODE_CMPUH           = 5067,
    OP_CODE_CMPUW           = 5068,
    
    OP_CODE_LSID            = 5070,     OP_CODE_EXTR            = 5071,     OP_CODE_DEP             = 5072,
    OP_CODE_DSR             = 5073,     OP_CODE_SHLA            = 5074,     OP_CODE_CMR             = 5075,
    OP_CODE_LDIL            = 5076,     OP_CODE_ADDIL           = 5077,     OP_CODE_LDO             = 5078,

    OP_CODE_B               = 5080,     OP_CODE_GATE            = 5081,     OP_CODE_BR              = 5082,
    OP_CODE_BV              = 5083,     OP_CODE_BE              = 5084,     OP_CODE_BVE             = 5085,
    OP_CODE_CBR             = 5086,     OP_CODE_CBRU            = 5087,

    OP_CODE_MR              = 5090,     OP_CODE_MST             = 5091,     OP_CODE_DS              = 5092,
    OP_CODE_LDPA            = 5093,     OP_CODE_PRB             = 5094,     OP_CODE_ITLB            = 5095,
    OP_CODE_PTLB            = 5096,     OP_CODE_PCA             = 5097,     OP_CODE_DIAG            = 5098,
    
    OP_CODE_RFI             = 5100,     OP_CODE_BRK             = 5101,
    
    //--------------------------------------------------------------------------------------------------------
    // Synthetic OP Code Tokens.
    //
    //--------------------------------------------------------------------------------------------------------
    OP_CODE_S_NOP           = 6000,     OP_CODE_S_SHL           = 6001,     OP_CODE_S_SHR           = 6002,
    OP_CODE_S_ASL           = 6003,     OP_CODE_S_ASR           = 6004,     OP_CODE_S_ROR           = 6005,
    OP_CODE_S_ROL           = 6006,
    
    //--------------------------------------------------------------------------------------------------------
    // The last token ID. This ID is used to terminate a token table list.
    //
    //--------------------------------------------------------------------------------------------------------
    TOK_LAST                = 9999
};

//------------------------------------------------------------------------------------------------------------
// Our error messages IDs. There is a routine that maps the ID to a text string.
//
// ??? need to put all error messages here.... do a little when there is time...
//------------------------------------------------------------------------------------------------------------
enum ErrMsgId : uint16_t {
    
    NO_ERR                          = 0,
    ERR_INVALID_CMD                 = 1,
    ERR_NOT_IN_WIN_MODE             = 2,
    ERR_OPEN_EXEC_FILE              = 3,
    ERR_EXPECTED_FILE_NAME          = 4,
    ERR_INVALID_WIN_STACK_ID        = 5,
    ERR_INVALID_WIN_ID              = 6,
    ERR_EXPECTED_WIN_ID             = 7,
    ERR_INVALID_WIN_TYPE            = 8,
    ERR_EXPECTED_WIN_TYPE           = 9,
    ERR_EXPECTED_STACK_ID           = 10,
    
    ERR_INVALID_EXIT_VAL            = 1000,
    
    ERR_INVALID_REG_ID              = 200,
    
    ERR_EXTRA_TOKEN_IN_STR          = 100,
    ERR_EXPECTED_LPAREN             = 101,
    ERR_EXPECTED_RPAREN             = 102,
    ERR_EXPECTED_COMMA              = 103,
    ERR_EXPECTED_NUMERIC            = 104,
    ERR_EXPECTED_EXT_ADR            = 105,
    ERR_EXPR_TYPE_MATCH             = 106,
    ERR_EXPR_FACTOR                 = 107,
    ERR_EXPECTED_GENERAL_REG        = 108,
    ERR_EXPECTED_OFS                = 205,
    
    ERR_EXPECTED_START_OFS          = 201,
    ERR_EXPECTED_LEN                = 202,
    ERR_OFS_LEN_LIMIT_EXCEEDED      = 203,
 
    ERR_INVALID_ARG                 = 109,
    
    ERR_TLB_TYPE                    = 110,
    ERR_TLB_PURGE_OP                = 111,
    ERR_TLB_INSERT_OP               = 112,
    ERR_TLB_ACC_DATA                = 113,
    ERR_TLB_ADR_DATA                = 114,
    ERR_TLB_NOT_CONFIGURED          = 115,
    
    ERR_CACHE_TYPE                  = 116,
    ERR_CACHE_PURGE_OP              = 117,
    ERR_CACHE_NOT_CONFIGURED        = 118,
    
    ERR_EXPECTED_STEPS              = 120,
    ERR_INVALID_STEP_OPTION         = 121,
    
    ERR_EXPECTED_INSTR_VAL          = 122,
    ERR_TOO_MANY_ARGS_CMD_LINE      = 123,
    
    
    
    ERR_INVALID_FMT_OPT             = 11,
    ERR_EXPECTED_FMT_OPT            = 12,
    
    ERR_OUT_OF_WINDOWS              = 13,
    
    ERR_INVALID_NUM                 = 20,
    ERR_EXPECTED_CLOSING_QUOTE      = 21,
    ERR_INVALID_CHAR_IN_IDENT       = 22,
};

//------------------------------------------------------------------------------------------------------------
// The command line size. The command line is rather long so that we can read in long lines form perhaps
// future script files.
//
//------------------------------------------------------------------------------------------------------------
const int CMD_LINE_BUF_SIZE     = 256;
const int TOK_STR_SIZE          = 256;

//------------------------------------------------------------------------------------------------------------
// Forward declaration of the globals structure. Every object will have access to the globals structure, so
// we do not have to pass around references to all the individual objects.
//
//------------------------------------------------------------------------------------------------------------
struct VCPU32Globals;

//------------------------------------------------------------------------------------------------------------
// The command line interpreter as well as the one line assembler work the command line or assembly line
// processed as a list of tokens. A token found in a string is recorded using the token structure. The token
// types are numeric, virtual address and string.
//
//------------------------------------------------------------------------------------------------------------
struct DrvToken {

    char        name[ 32 ]  = { };
    TypeId      typ         = TYP_NIL;
    TokId       tid         = TOK_NIL;
    
    union {
        
        struct {    uint32_t val;                   };
        struct {    uint32_t seg;   uint32_t ofs;   };
        struct {    char str[ TOK_STR_SIZE ];        };
    };
};

//------------------------------------------------------------------------------------------------------------
// Tokenizer object. The command line interface as well as the one line assembler parse their buffer line.
// The tokenizer will return the tokens found in the line. The tokenizer will will work with the global
// token table found in the tokenizer source file.
//
//------------------------------------------------------------------------------------------------------------
struct DrvTokenizer {

    public:

    DrvTokenizer( );

    uint8_t     setupTokenizer( char *lineBuf, DrvToken *tokTab );
    void        nextToken( );
    DrvToken    token( );
    
    
    bool        isToken( TokId tokId );
    bool        isTokenTyp( TypeId typId );

    TypeId      tokTyp( );
    TokId       tokId( );
    int         tokVal( );
    char        *tokStr( );
    uint32_t    tokSeg( );
    uint32_t    tokOfs( );
    
    int         tokCharIndex( );
    char        *tokenLineStr( );

    private:
    
    void        tokenError( char *errMsg );
    void        nextChar( );
    void        parseNum( );
    void        parseString( );
    void        parseIdent( );

    DrvToken    currentToken;
    DrvToken    *tokTab                 = nullptr;
    char        tokenLine[ 256 ]        = { 0 };
    int         currentLineLen          = 0;
    int         currentCharIndex        = 0;
    int         currentTokCharIndex     = 0;
    char        currentChar             = ' ';
};

//------------------------------------------------------------------------------------------------------------
// Expression value. The analysis of an expression results in a value. Depending on the expression type, the
// values are simple scalar values or a structured value, such as a register pair or virtual address.
//
//------------------------------------------------------------------------------------------------------------
struct DrvExpr {
    
    TypeId typ;
   
    union {
        
        struct {
            
            TokId tokId;
        };
        
        struct {
            
            uint32_t numVal1;
        };
        
        struct {
            
            char strVal[ TOK_STR_SIZE ];
        };
        
        struct {
            
            uint32_t seg;
            uint32_t ofs;
        };
        
        struct {
            
            uint32_t adr;
        };
        
        struct {
            
            uint8_t sReg;
            uint8_t gReg;
            
        };
    };
    
    // ??? should go away... when we use extAdr.
    union {
        
        uint32_t    numVal2;
    };
};


//------------------------------------------------------------------------------------------------------------
// Driver environment variables. There is a simple "name=value" dictionary.
//
//------------------------------------------------------------------------------------------------------------
struct DrvEnv {
    
public:
    
    DrvEnv( VCPU32Globals *glb );
    
    int         getEnvTabSize( );
    TokId       lookupEnvTokId( char *str, TokId def = TOK_NIL );
    TokId       getEnvType( TokId envId );
    bool        isReadOnly( TokId envId );
    
    TokId       getEnvValTok( TokId envId );
    int32_t     getEnvValInt( TokId envId );
    uint32_t    getEnvValUInt( TokId envId );
    bool        getEnvValBool( TokId envId );
    char        *getEnvValStr( TokId envId );
    
    void        setEnvVal( TokId envId, TokId val );
    void        setEnvVal( TokId envId, int32_t val );
    void        setEnvVal( TokId envId, uint32_t val );
    void        setEnvVal( TokId envId, bool val );
    void        setEnvVal( TokId envId, char *val );
    
    uint8_t     displayEnvTable( );
    uint8_t     displayEnvTabEntry( TokId envId );
    
private:
    
    VCPU32Globals *glb = nullptr;
};


//-----------------------------------------------------------------------------------------------------------
// The "CPU24DrvBaseWin" class. The Terminal Screen will be in screen mode a set of stacks each with a list
// of screen sub windows. The default is one stack, the general register set window and the command line
// window, which also spans all stacks. Each sub window is an instance of a specific window class with this
// class as the base class. There are routines common to all windows to enable/ disable, set the lines
// displayed and so on. There are also abstract methods that the inheriting class needs to implement.
// Examples are to initialize a window, redraw and so on.
//
//-----------------------------------------------------------------------------------------------------------
struct DrvWin {
    
public:
    
    DrvWin( VCPU32Globals *glb );
    virtual         ~ DrvWin( );
    
    void            setWinType( int type );
    int             getWinType( );
    
    void            setWinIndex( int index );
    int             getWinIndex( );
    
    void            setEnable( bool arg );
    bool            isEnabled( );
    
    virtual void    setRadix( TokId radix );
    virtual TokId   getRadix( );
    
    void            setRows( int arg );
    int             getRows( );
    
    void            setDefColumns( int arg, TokId rdx = TOK_HEX );
    int             getDefColumns( TokId rdx = TOK_HEX );
    
    void            setColumns( int arg );
    int             getColumns( );
    
    void            setWinOrigin( int row, int col );
    void            setWinCursor( int row, int col );
    
    int             getWinCursorRow( );
    int             getWinCursorCol( );
    
    int             getWinStack( );
    void            setWinStack( int wStack );
    
    void            printNumericField( uint32_t val,
                                      uint32_t fmtDesc = 0,
                                      int len = 0,
                                      int row = 0,
                                      int col = 0 );
    
    void            printTextField( char *text,
                                   uint32_t fmtDesc = 0,
                                   int len = 0,
                                   int row = 0,
                                   int col = 0 );
    
    void            printRadixField( uint32_t fmtDesc = 0,
                                    int len = 0,
                                    int row = 0,
                                    int col = 0 );
    
    void            printWindowIdField( int stack,
                                        int index,
                                        bool current = false,
                                        uint32_t fmtDesc = 0,
                                        int row = 0,
                                        int col = 0 );
    
    void            padLine( uint32_t fmtDesc = 0 );
    void            clearField( int len, uint32_t fmtDesc = 0 ); 
    
    void            reDraw( );
    
    virtual void    toggleWin( );
    virtual void    setDefaults( )  = 0;
    virtual void    drawBanner( )   = 0;
    virtual void    drawBody( )     = 0;
    
protected:
    
    VCPU32Globals   *glb;
    
private:
    
    int             winType             = TOK_NIL;
    int             winUserIndex        = 0;
    
    bool            winEnabled          = false;
    bool            winCurrent          = false;
    
    TokId           winRadix            = TOK_HEX;
    int             winStack            = 0;
    int             winRows             = 0;
    int             winColumns          = 0;
    int             winDefColumnsHex    = 0;
    int             winDefColumnsOct    = 0;
    int             winDefColumnsDec    = 0;
    
    int             winAbsCursorRow     = 0;
    int             winAbsCursorCol     = 0;
    int             lastRowPos          = 0;
    int             lastColPos          = 0;
};

//-----------------------------------------------------------------------------------------------------------
// "WinScrollable" is an extension to the basic window. It implements a scrollable window of a number of
// lines. There is a high level concept of a starting index of zero and a limit. The meaning i.e. whether
// the address is a memory address or an index into a TLB or Cache array is determined by the inheriting
// class. The scrollable window will show a number of lines, the "drawLine" method needs to be implemented
// by the inheriting class. The routine is passed the item address for the line and is responsible for the
// correct interpretation of this address. The "lineIncrement" is the increment value for the item address
// passed. Item addresses are unsigned 32-bit quantities.
//-----------------------------------------------------------------------------------------------------------
struct DrvWinScrollable : DrvWin {
    
public:
    
    DrvWinScrollable( VCPU32Globals *glb );
    
    void            setHomeItemAdr( uint32_t adr );
    uint32_t        getHomeItemAdr( );
    void            setCurrentItemAdr( uint32_t adr );
    uint32_t        getCurrentItemAdr( );
    void            setLimitItemAdr( uint32_t adr );
    uint32_t        getLimitItemAdr( );
    void            setLineIncrement( uint32_t arg );
    uint32_t        getLineIncrement( );
    
    virtual void    drawBody( );
    virtual void    drawLine( uint32_t index ) = 0;
    
    void            winHome( uint32_t pos = 0 );
    void            winForward( uint32_t amt );
    void            winBackward( uint32_t amt );
    void            winJump( uint32_t pos );
    
private:
    
    uint32_t        homeItemAdr         = 0;
    uint32_t        currentItemAdr      = 0;
    uint32_t        limitItemAdr        = 0;
    uint32_t        lineIncrement       = 0;
};


//-----------------------------------------------------------------------------------------------------------
// Program State Register Window. This window holds the programmer visible state with the exception of the
// program relevant control register values. They are a separate window.
//
//-----------------------------------------------------------------------------------------------------------
struct DrvWinProgState : DrvWin {
    
public:
    
    DrvWinProgState( VCPU32Globals *glb );
    
    void setDefaults( );
    void setRadix( TokId rdx );
    void drawBanner( );
    void drawBody( );
};

//-----------------------------------------------------------------------------------------------------------
// Special Register Window. This window holds the control registers.
//
//-----------------------------------------------------------------------------------------------------------
struct DrvWinSpecialRegs : DrvWin {
    
public:
    
    DrvWinSpecialRegs( VCPU32Globals *glb );
    
    void setDefaults( );
    void setRadix( TokId rdx );
    void drawBanner( );
    void drawBody( );
};

//-----------------------------------------------------------------------------------------------------------
// Pipeline Register Window. This window holds the CPU pipeline registers.
//
//-----------------------------------------------------------------------------------------------------------
struct DrvWinPipeLineRegs : DrvWin {
    
public:
    
    DrvWinPipeLineRegs( VCPU32Globals *glb );
    
    void setDefaults( );
    void setRadix( TokId rdx );
    void drawBanner( );
    void drawBody( );
};

//-----------------------------------------------------------------------------------------------------------
// Statistics Window. This window displays the CPU statistics collected during execution.
//
//-----------------------------------------------------------------------------------------------------------
struct DrvWinStatistics : DrvWin {
    
public:
    
    DrvWinStatistics( VCPU32Globals *glb );
    
    void setDefaults( );
    void drawBanner( );
    void drawBody( );
};

//-----------------------------------------------------------------------------------------------------------
// Absolute Memory Window. A memory window will show the absolute memory content starting with the current
// address followed by a number of data words. The number of words shown is the number of lines of the
// window times the number of items, ie.e words, on a line.
//
//-----------------------------------------------------------------------------------------------------------
struct DrvWinAbsMem : DrvWinScrollable {
    
public:
    
    DrvWinAbsMem( VCPU32Globals *glb );
    
    void setDefaults( );
    void setRadix( TokId rdx );
    void drawBanner( );
    void drawLine( uint32_t index );
};

//-----------------------------------------------------------------------------------------------------------
// Code Memory Window. A code memory window will show the instruction memory content starting with the
// current address followed by the instruction and a human readable disassembled version.
//
//-----------------------------------------------------------------------------------------------------------
struct DrvWinCode : DrvWinScrollable {
    
public:
    
    DrvWinCode( VCPU32Globals *glb );
    
    void setDefaults( );
    void drawBanner( );
    void drawLine( uint32_t index );
};

//-----------------------------------------------------------------------------------------------------------
// TLB Window. The TLB data window displays the TLB entries.
//
//-----------------------------------------------------------------------------------------------------------
struct DrvWinTlb : DrvWinScrollable {
    
public:
    
    DrvWinTlb( VCPU32Globals *glb, int winType );
    
    void setDefaults( );
    void setRadix( TokId rdx );
    void drawBanner( );
    void drawLine( uint32_t index );
    
private:
    
    int           winType = 0;
    CpuTlb        *tlb    = nullptr;
};

//-----------------------------------------------------------------------------------------------------------
// Memory Object - Cache Window. The memory object window display the cache date lines. Since we can have
// caches with more than one set, the toggle function allows to flip through the sets, one at a time.
//
//-----------------------------------------------------------------------------------------------------------
struct DrvWinCache : DrvWinScrollable {
    
public:
    
    DrvWinCache( VCPU32Globals *glb, int winType );
    
    void setDefaults( );
    void setRadix( TokId rdx );
    void toggleWin( );
    void drawBanner( );
    void drawLine( uint32_t index );
    
private:
    
    int     winType         = 0;
    int     winToggleVal    = 0;
    CpuMem  *cPtr           = nullptr;
};

//-----------------------------------------------------------------------------------------------------------
// Memory Object Controller Window. Each memory object is implemented as a state machine. A request goes
// through several states. This window displays the state machine control information.
//
//-----------------------------------------------------------------------------------------------------------
struct DrvWinMemController : DrvWin {
    
public:
    
    DrvWinMemController( VCPU32Globals *glb, int winType );
    
    void setDefaults( );
    void drawBanner( );
    void drawBody( );
    
private:
    
    int     winType = 0;
    CpuMem  *cPtr   = nullptr;
};

//-----------------------------------------------------------------------------------------------------------
// TLB Object Controller Window. Each TLB object is implemented as a state machine. A request goes through
// several states. This window displays the state machine control information.
//
//-----------------------------------------------------------------------------------------------------------
struct DrvWinTlbController : DrvWin {
    
public:
    
    DrvWinTlbController( VCPU32Globals *glb, int winType );
    
    void setDefaults( );
    void drawBanner( );
    void drawBody( );
    
private:
    
    int     winType = 0;
    CpuMem  *tPtr   = nullptr;
};

//-----------------------------------------------------------------------------------------------------------
// Text Window. It may be handy to also display an ordinary ASCII text file. One day this will allow us to
// display for example the source code to a running program when symbolic debugging is supported.
//
//-----------------------------------------------------------------------------------------------------------
struct DrvWinText : DrvWinScrollable {
    
public:
    
    DrvWinText( VCPU32Globals *glb, char *fName );
    ~ DrvWinText( );
    
    void    setDefaults( );
    void    drawBanner( );
    void    drawLine( uint32_t index );
    
private:

    bool    openTextFile( );
    int     readTextFileLine( int linePos, char *lineBuf, int bufLen );
    
    FILE    *textFile          = nullptr;
    int     fileSizeLines      = 0;
    int     lastLinePos        = 0;
    char    fileName[ 256 ];
};

//-----------------------------------------------------------------------------------------------------------
// Command Line Window. The command window is a special class, which comes always last in the windows list
// and cannot be disabled. It is intended to be a scrollable window, where only the banner line is fixed.
//
//-----------------------------------------------------------------------------------------------------------
struct DrvWinCommands : DrvWin {
    
public:
    
    DrvWinCommands( VCPU32Globals *glb );
    
    void setDefaults( );
    void drawBanner( );
    void drawBody( );
};

//-----------------------------------------------------------------------------------------------------------
// The window display screen object is the central object that represents the screen where we have windows
// turned on. All commands send from the command input in windows mode will eventually end up as calls to
// this object. A screen is an ordered list of windows. Although you can disable a window such that it
// disappears on the screen, when enabled, it will show up in the place intended for it. For example, the
// program state register window will always be on top, follow by the special regs, followed by the pipeline
// regs. The command input scroll area is always last and is the only window that cannot be disabled.
//
//-----------------------------------------------------------------------------------------------------------
struct DrvWinDisplay {
    
public:
    
    DrvWinDisplay( VCPU32Globals *glb );
    
    void            reDraw( bool mustRedraw = false );
    
    void            windowsOn( );
    void            windowsOff( );
    void            windowDefaults( );
    void            windowCurrent( TokId winCmd, int winNum = 0 );
    void            windowEnable( TokId winCmd, int winNum = 0 );
    void            windowDisable( TokId winCmd, int winNum = 0  );
    void            winStacksEnable( bool arg );
    void            windowRadix( TokId winCmd, TokId fmtId, int winNum = 0 );
    void            windowSetRows( TokId winCmd, int rows, int winNum = 0 );
    
    void            windowHome( TokId winCmd, int amt, int winNum = 0 );
    void            windowForward( TokId winCmd, int amt, int winNum = 0 );
    void            windowBackward( TokId winCmd, int amt, int winNum = 0 );
    void            windowJump( TokId winCmd, int amt, int winNum = 0 );
    void            windowToggle( TokId winCmd, int winNum = 0 );
    void            windowExchangeOrder( TokId winCmd, int winNum );
    
    void            windowNew( TokId winCmd, TokId winType = TOK_NIL, char *argStr = nullptr );
    void            windowKill( TokId winCmd, int winNumStart, int winNumEnd = 0  );
    void            windowSetStack( int winStack, int winNumStart, int winNumEnd = 0 );
    
    int             getCurrentUserWindow( );
    void            setCurrentUserWindow( int num );
    int             getFirstUserWinIndex( );
    int             getLastUserWinIndex( );
    bool            validWindowNum( int num );
    bool            validUserWindowNum( int num );
    bool            validWindowStackNum( int num );
    bool            validUserWindowType( TokId winType );
    bool            isCurrentWin( int winNum );
    bool            isWinEnabled( int winNum );
    
private:
    
    int             computeColumnsNeeded( int winStack );
    int             computeRowsNeeded( int winStack );
    void            setWindowColumns( int winStack, int columns );
    void            setWindowOrigins( int winStack, int rowOffset = 1, int colOffset = 1 );
    
    int             actualRowSize       = 0;
    int             actualColumnSize    = 0;
    int             currentUserWinNum   = -1;
    bool            winStacksOn         = true;
    
    VCPU32Globals   *glb                = nullptr;
};

//------------------------------------------------------------------------------------------------------------
// The line mode display functions. This class combines most of the line mode display functions for displaying
// registers, memory content, data entries and so on.
//
//------------------------------------------------------------------------------------------------------------
struct DrvLineDisplay {
    
public:
    
    DrvLineDisplay( VCPU32Globals *glb );
    
    void        displayWord( uint32_t val, TokId = TOK_DEF );
    void        displayHalfWord( uint32_t val, TokId = TOK_DEF );
    void        displayAbsMemContent( uint32_t ofs, uint32_t len, TokId = TOK_DEF );
    void        displayAbsMemContentAsCode( uint32_t ofs, uint32_t len, TokId fmtId );
    
    void        displayGeneralRegSet( TokId fmt = TOK_DEF );
    void        displaySegmentRegSet( TokId fmt = TOK_DEF );
    void        displayControlRegSet( TokId fmt = TOK_DEF );
    void        displayPStateRegSet( TokId fmt = TOK_DEF );
    void        displayPlIFetchDecodeRegSet( TokId fmt = TOK_DEF );
    void        displayPlMemoryAccessRegSet( TokId fmt = TOK_DEF );
    void        displayPlExecuteRegSet( TokId fmt = TOK_DEF );
    void        displayPlRegSets( TokId fmt = TOK_DEF );
    void        displayAllRegSets( TokId fmt = TOK_DEF );
    void        displayTlbEntry( TlbEntry *entry, TokId fmt = TOK_DEF );
    void        displayTlbEntries( CpuTlb *tlb, uint32_t index, uint32_t len, TokId fmt = TOK_DEF );
    void        displayCacheEntries( CpuMem *cache, uint32_t index, uint32_t len, TokId fmt = TOK_DEF );
    void        displayMemObjRegSet( CpuMem *mem, TokId fmt = TOK_DEF );
    void        displayTlbObjRegSet( CpuTlb *tlb, TokId fmt = TOK_DEF );
    
private:
    
    void        displayRegsAndLabel( RegClass   regSetId,
                                    int         regStart,
                                    int         numOfRegs   = 4,
                                    char        *LineLabel  = ((char *)"" ),
                                    TokId       fmt         = TOK_DEF );
    
    VCPU32Globals *glb = nullptr;
};

//------------------------------------------------------------------------------------------------------------
// The disassembler function. The disassembler takes a machine instruction word and displays it in human
// readable form.
//
//------------------------------------------------------------------------------------------------------------
struct DrvDisAsm {
    
public:
    
    DrvDisAsm( VCPU32Globals *glb );
    void displayInstr( uint32_t instr, TokId fmt );
    void displayOpCodeAndOptions( uint32_t instr );
    void displayTargetAndOperands( uint32_t instr, TokId fmt = TOK_DEF );
    
    int  getOpCodeOptionsFieldWidth( );
    int  getTargetAndOperandsFieldWidth( );
    
    
private:
    
    VCPU32Globals *glb = nullptr;
};

//------------------------------------------------------------------------------------------------------------
// A simple one line assembler. This object is the counter part to the disassembler. We will parse a one
// line input string for a valid instruction, using the syntax of the real assembler. There will be no
// labels and comments, only the opcode and the operands.
//
//------------------------------------------------------------------------------------------------------------
struct DrvOneLineAsm {
    
public:
    
    DrvOneLineAsm( VCPU32Globals *glb );
    bool parseAsmLine( char *inputStr, uint32_t *instr );
    
private:
    
    VCPU32Globals   *glb = nullptr;
    char            *inputStr;
    
};

//------------------------------------------------------------------------------------------------------------
// The CPU driver main object. This objects implements the command loop. It is essentially a list of the
// command handlers and the functions needed to read in and analyze a command line.
//
//------------------------------------------------------------------------------------------------------------
struct DrvCmds {
    
public:
    
    DrvCmds( VCPU32Globals *glb );
   
    void            printWelcome( );
    void            processCmdLineArgs( int argc, const char *argv[ ] );
    TokId           getCurrentCmd( );
    void            cmdLoop( );
    
    static char     *tokIdToName( TokId tokId );
    
private:
    
    void            promptCmdLine( );
    bool            readInputLine( char *cmdBuf );
    uint8_t         evalInputLine( char *cmdBuf );
    uint8_t         execCmdsFromFile( char* fileName );
    
    uint8_t         parseExpr( DrvExpr *rExpr );
    uint8_t         parseTerm( DrvExpr *rExpr );
    uint8_t         parseFactor( DrvExpr *rExpr );
    
    uint8_t         invalidCmd( );
    uint8_t         exitCmd( );
    uint8_t         helpCmd( );
    uint8_t         winHelpCmd( );
    void            envCmd( char *cmdBuf );
    uint8_t         execFileCmd( );
    uint8_t         loadPhysMemCmd( );
    uint8_t         savePhysMemCmd( );
    
    uint8_t         resetCmd( );
    uint8_t         runCmd( );
    uint8_t         stepCmd( );
   
    uint8_t         disAssembleCmd( );
    uint8_t         assembleCmd( );
    uint8_t         displayRegCmd( );
    uint8_t         modifyRegCmd( );
    uint8_t         hashVACmd( );
    uint8_t         displayTLBCmd( );
    uint8_t         purgeTLBCmd( );
    uint8_t         insertTLBCmd( );
    uint8_t         displayCacheCmd( );
    uint8_t         purgeCacheCmd( );
    uint8_t         displayAbsMemCmd( );
    uint8_t         modifyAbsMemCmd( );
    uint8_t         modifyAbsMemAsCodeCmd( );
    
    uint8_t         winOnCmd( );
    uint8_t         winOffCmd( );
    uint8_t         winDefCmd( );
    uint8_t         winStacksEnable( );
    uint8_t         winStacksDisable( );

    void            winCurrentCmd( char *cmdBuf );
    void            winEnableCmd( char *cmdBUf );
    void            winDisableCmd( char *cmdBuf );
    void            winSetRadixCmd( char *cmdBuf );
    void            winForwardCmd( char *cmdBuf );
    void            winBackwardCmd( char *cmdBuf );
    void            winHomeCmd( char *cmdBuf );
    void            winJumpCmd( char *cmdBuf );
    void            winSetRowsCmd( char *cmdBuf );
    void            winNewWinCmd( char *cmdBuf );
    void            winKillWinCmd( char * cmdBuf );
    void            winSetStackCmd( char *cmdBuf );
    void            winToggleCmd( char *cmdBuf );
    void            winExchangeCmd( char *cmdBuf );
    
    VCPU32Globals   *glb       = nullptr;
    bool            winModeOn  = false;
    TokId           currentCmd = TOK_NIL;
    
};

//------------------------------------------------------------------------------------------------------------
// The globals, accessible to all objects. Turns out that all main objects need to access data from all the
// individual objects of the CPU. To ease the passing around there is the idea a global structure with a
// reference to all the individual objects.
//
//------------------------------------------------------------------------------------------------------------
struct VCPU32Globals {
    
    DrvEnv              *env            = nullptr;
    DrvDisAsm           *disAsm         = nullptr;
    DrvOneLineAsm       *oneLineAsm     = nullptr;
    
    DrvLineDisplay      *lineDisplay    = nullptr;
    DrvWinDisplay       *winDisplay     = nullptr;
    DrvCmds             *cmds           = nullptr;
    
    CpuCore             *cpu            = nullptr;
};

#endif /* CPU24Driver_h */
