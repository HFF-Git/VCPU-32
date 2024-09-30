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
#ifndef VCPU32TestDriver_hpp
#define VCPU32TestDriver_hpp

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

//------------------------------------------------------------------------------------------------------------
// Tokens are the labels for reserved words recognized by the command input. They also serve as attribute
// values and numeric for some command options and settings.
//
//------------------------------------------------------------------------------------------------------------
enum TokId : uint16_t {
    
    //--------------------------------------------------------------------------------------------------------
    // General tokens.
    //
    //--------------------------------------------------------------------------------------------------------
    TOK_NIL                 = 0,    TOK_INV                 = 1,    TOK_ALL                 = 2,
    TOK_TRUE                = 3,    TOK_FALSE               = 4,    TOK_DEF                 = 5,
    
    TOK_CPU                 = 6,    TOK_MEM                 = 7,    TOK_STATS               = 8,
    
    TOK_C                   = 100,  TOK_D                   = 101,  TOK_F                   = 102,
    TOK_I                   = 103,  TOK_T                   = 104,  TOK_U                   = 105,
    
    TOK_DEC                 = 12,   TOK_OCT                 = 13,   TOK_HEX                 = 14,
    
    TOK_PM                  = 20,   TOK_PC                  = 21,   TOK_IT                  = 22,
    TOK_DT                  = 23,   TOK_IC                  = 24,   TOK_DC                  = 25,
    TOK_UC                  = 26,   TOK_TX                  = 27,
    
    TOK_ICR                 = 28,   TOK_DCR                 = 29,   TOK_UCR                 = 30,
    TOK_ITR                 = 31,   TOK_DTR                 = 32,   TOK_MCR                 = 33,
    TOK_PCR                 = 34,   TOK_IOR                 = 35,
    
    TOK_EQ                  = 40,   TOK_NE                  = 41,   TOK_LT                  = 42,
    TOK_GT                  = 43,   TOK_LE                  = 44,   TOK_GE                  = 45,
    
    //--------------------------------------------------------------------------------------------------------
    // Environment variable tokens.
    //
    //--------------------------------------------------------------------------------------------------------
    ENV_TYP_INT             = 500,  ENV_TYP_UINT            = 501,  ENV_TYP_STR             = 502,
    ENV_TYP_BOOL            = 503,  ENV_TYP_TOK             = 504,
    
    ENV_I_TLB_SETS          = 510,  ENV_I_TLB_SIZE          = 511,
    ENV_D_TLB_SETS          = 512,  ENV_D_TLB_SIZE          = 513,
    
    ENV_I_CACHE_SETS        = 520,  ENV_I_CACHE_SIZE        = 521,  ENV_I_CACHE_LINE_SIZE   = 522,
    ENV_D_CACHE_SETS        = 530,  ENV_D_CACHE_SIZE        = 531,  ENV_D_CACHE_LINE_SIZE   = 532,
    ENV_U_CACHE_SETS        = 535,  ENV_U_CACHE_SIZE        = 536,  ENV_U_CACHE_LINE_SIZE   = 537,
    ENV_MEM_SIZE            = 541,  ENV_MEM_BANKS           = 542,  ENV_MEM_BANK_SIZE       = 543,
    ENV_MEM_R_ACC_CYCLE     = 544,  ENV_MEM_W_ACC_CYCLE     = 545,
    
    ENV_CMD_CNT             = 550,  ENV_SHOW_CMD_CNT        = 551,  ENV_ECHO_CMD            = 552,
    ENV_FMT_DEF             = 553,  ENV_EXIT_CODE           = 554,  ENV_WORDS_PER_LINE      = 555,
    ENV_PROG_VERSION        = 556,  ENV_GIT_BRANCH          = 557,  ENV_PROG_PATCH_LEVEL    = 558,
    ENV_STEP_IN_CLOCKS      = 559,  ENV_SHOW_PSTAGE_INFO    = 560,  ENV_PASS_CNT            = 561,
    ENV_FAIL_CNT            = 562,  ENV_WIN_MIN_ROWS        = 563,  ENV_WIN_TX_WIDTH        = 564,
    
    //--------------------------------------------------------------------------------------------------------
    // Command Sets.
    //
    //--------------------------------------------------------------------------------------------------------
    SET_NIL                 = 900,  ENV_SET                 = 901,  CMD_SET                 = 902,
    FMT_SET                 = 903,  REG_SET                 = 904,  REG_SET_ALL             = 905,
    
    GR_SET                  = 910,  SR_SET                  = 911,  CR_SET                  = 912,
    PS_SET                  = 913,  FD_SET                  = 914,  MA_SET                  = 915,
    PR_SET                  = 916,
    
    IC_L1_SET               = 917,  DC_L1_SET               = 918,  UC_L2_SET               = 919,
    MEM_SET                 = 920,  ITLB_SET                = 921,  DTLB_SET                = 922,
    
    //--------------------------------------------------------------------------------------------------------
    // Line Commands.
    //
    //--------------------------------------------------------------------------------------------------------
    CMD_COMMENT             = 1000,   CMD_ENV               = 1001, CMD_EXIT              = 1002,
    CMD_HELP                = 1003,   CMD_WHELP             = 1004,
    
    CMD_RESET               = 1010,   CMD_RUN               = 1011, CMD_STEP              = 1012,
    CMD_XF                  = 1013,   CMD_DIS_ASM           = 1014, CMD_ASM               = 1015,
    
    CMD_B                   = 1016,   CMD_BD                = 1017, CMD_BL                = 1018,
    
    CMD_DR                  = 1020,   CMD_MR                = 1021,
    CMD_DA                  = 1027,   CMD_MA                = 1028,
    CMD_DAA                 = 1029,   CMD_MAA               = 1030,
    CMD_LMF                 = 1031,   CMD_SMF               = 1032,
    
    CMD_HASH_VA             = 1033,
    CMD_D_TLB               = 1034,   CMD_I_TLB             = 1035, CMD_P_TLB             = 1036,
    CMD_D_CACHE             = 1037,   CMD_P_CACHE           = 1038,
    
    //--------------------------------------------------------------------------------------------------------
    // Window Commands.
    //
    //--------------------------------------------------------------------------------------------------------
    CMD_WON                 = 2000, CMD_WOFF                = 2001, CMD_WDEF                = 2002,
    CMD_CWL                 = 2003, CMD_WSE                 = 2004, CMD_WSD                 = 2005,
    
    CMD_PSE                 = 2010, CMD_PSD                 = 2011, CMD_PSR                 = 2012,
    CMD_SRE                 = 2015, CMD_SRD                 = 2016, CMD_SRR                 = 2017,
    CMD_PLE                 = 2020, CMD_PLD                 = 2021, CMD_PLR                 = 2022,
    CMD_SWE                 = 2025, CMD_SWD                 = 2026, CMD_SWR                 = 2027,
    
    CMD_WE                  = 2050, CMD_WD                  = 2051, CMD_WR                  = 2052,
    CMD_WF                  = 2053, CMD_WB                  = 2054, CMD_WH                  = 2055,
    CMD_WJ                  = 2056, CMD_WL                  = 2057, CMD_WN                  = 2058,
    CMD_WK                  = 2059, CMD_WS                  = 2060, CMD_WC                  = 2061,
    CMD_WT                  = 2062, CMD_WX                  = 2063,
    
    //--------------------------------------------------------------------------------------------------------
    // General, Segment and Control Registers
    //
    //--------------------------------------------------------------------------------------------------------
    GR_0                    = 4100, GR_1                    = 4101, GR_2                    = 4102,
    GR_3                    = 4103, GR_4                    = 4104, GR_5                    = 4105,
    GR_6                    = 4106, GR_7                    = 4107, GR_8                    = 4108,
    GR_9                    = 4109, GR_10                   = 4110, GR_11                   = 4111,
    GR_12                   = 4112, GR_13                   = 4113, GR_14                   = 4114,
    GR_15                   = 4115,
    
    SR_0                    = 4200, SR_1                    = 4201, SR_2                    = 4202,
    SR_3                    = 4203, SR_4                    = 4204, SR_5                    = 4205,
    SR_6                    = 4206, SR_7                    = 4207,
    
    CR_0                    = 4300, CR_1                    = 4301, CR_2                    = 4302,
    CR_3                    = 4303, CR_4                    = 4304, CR_5                    = 4305,
    CR_6                    = 4306, CR_7                    = 4307, CR_8                    = 4308,
    CR_9                    = 4309, CR_10                   = 4310, CR_11                   = 4311,
    CR_12                   = 4312, CR_13                   = 4313, CR_14                   = 4314,
    CR_15                   = 4315, CR_16                   = 4316, CR_17                   = 4317,
    CR_18                   = 4318, CR_19                   = 4319, CR_20                   = 4320,
    CR_21                   = 4321, CR_22                   = 4322, CR_23                   = 4323,
    CR_24                   = 4324, CR_25                   = 4325, CR_26                   = 4326,
    CR_27                   = 4327, CR_28                   = 4328, CR_29                   = 4329,
    CR_30                   = 4330, CR_31                   = 4331,
    
    PS_IA_SEG               = 4400, PS_IA_OFS               = 4401, PS_STATUS               = 4402,
    
    FD_IA_SEG               = 4500, FD_IA_OFS               = 4501, FD_INSTR                = 4502,
    FD_A                    = 4503, FD_B                    = 4504, FD_X                    = 4505,
    
    MA_IA_SEG               = 4600, MA_IA_OFS               = 4601, MA_INSTR                = 4602,
    MA_A                    = 4603, MA_B                    = 4604, MA_X                    = 4605,
    MA_S                    = 4606,
    
    IC_L1_STATE             = 4700, IC_L1_REQ               = 4701, IC_L1_REQ_SEG           = 4702,
    IC_L1_REQ_OFS           = 4703, IC_L1_REQ_TAG           = 4704, IC_L1_REQ_LEN           = 4705,
    IC_L1_LATENCY           = 4706, IC_L1_BLOCK_ENTRIES     = 4707, IC_L1_BLOCK_SIZE        = 4708,
    IC_L1_SETS              = 4709,
    
    DC_L1_STATE             = 4710, DC_L1_REQ               = 4711, DC_L1_REQ_SEG           = 4712,
    DC_L1_REQ_OFS           = 4713, DC_L1_REQ_TAG           = 4714, DC_L1_REQ_LEN           = 4715,
    DC_L1_LATENCY           = 4716, DC_L1_BLOCK_ENTRIES     = 4717, DC_L1_BLOCK_SIZE        = 4718,
    DC_L1_SETS              = 4719,
    
    UC_L2_STATE             = 4720, UC_L2_REQ               = 4721, UC_L2_REQ_SEG           = 4722,
    UC_L2_REQ_OFS           = 4723, UC_L2_REQ_TAG           = 4724, UC_L2_REQ_LEN           = 4725,
    UC_L2_LATENCY           = 4726, UC_L2_BLOCK_ENTRIES     = 4727, UC_L2_BLOCK_SIZE        = 4728,
    UC_L2_SETS              = 4729,
    
    ITLB_STATE              = 4730, ITLB_REQ                = 4731, ITLB_REQ_SEG           = 4732,
    ITLB_REQ_OFS            = 4733,
    
    DTLB_STATE              = 4740, DTLB_REQ                = 4741, DTLB_REQ_SEG           = 4742,
    DTLB_REQ_OFS            = 4743,
};

//------------------------------------------------------------------------------------------------------------
// Our error messages IDs. There is a routine that maps the ID to a text.
//
// ??? need to put all error messages here.... do a little when there is time...
//------------------------------------------------------------------------------------------------------------
enum ErrMsgId : uint16_t {
    
    NO_ERR                      = 0,
    INVALID_CMD_ERR             = 1,
    NOT_IN_WIN_MODE_ERR         = 2,
    OPEN_EXEC_FILE_ERR          = 3,
    EXPECTED_FILE_NAME_ERR      = 4,
    INVALID_WIN_STACK_ID        = 5,
    INVALID_WIN_ID              = 6,
    EXPECTED_WIN_ID             = 7,
    INVALID_WIN_TYPE            = 8,
    EXPECTED_WIN_TYPE           = 9,
    EXPECTED_STACK_ID           = 10,
    EXPECTED_FMT_OPT            = 11,
    OUT_OF_WINDOWS_ERR          = 12
};

//------------------------------------------------------------------------------------------------------------
// Forward declaration of the globals structure. Every object will have access to the globals structure, so
// we do not have to pass around references to all the individual objects.
//
//------------------------------------------------------------------------------------------------------------
struct VCPU32Globals;

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
    
    VCPU32Globals *glb = nullptr;
    
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
    int             promptYesNoCancel( char *promptStr );
    bool            readCmdLine( char *cmdBuf );
    void            dispatchCmd( char *cmdBuf );
    void            execCmdsFromFile( char* fileName );
    
    void            printErrMsg( ErrMsgId errNum, char *argStr = nullptr );
    
    void            invalidCmd( char *cmdBuf );
    void            commentCmd( char *cmdBuf );
    void            exitCmd( char *cmdBuf );
    void            helpCmd( char *cmdBuf );
    void            winHelpCmd( char *cmdBuf );
    void            envCmd( char *cmdBuf );
    void            execFileCmd( char *cmdBuf );
    void            initCmd( char *cmdBuf );
    void            resetCmd( char *cmdBuf );
    void            runCmd( char *cmdBuf );
    void            stepCmd( char *cmdBuf );
    void            setBreakPointCmd( char *cmdBuf );
    void            deleteBreakPointCmd( char *cmdBuf );
    void            listBreakPointsCmd( char *cmdBuf );
    void            disAssembleCmd( char *cmdBuf );
    void            assembleCmd( char *cmdBuf );
    void            displayRegCmd( char *cmdBuf );
    void            modifyRegCmd( char *cmdBuf );
    void            modifyPstageRegCmd( char *cmdBuf );
    void            hashVACmd( char *cmdBuf );
    void            displayTLBCmd( char *cmdBuf );
    void            purgeTLBCmd( char *cmdBuf );
    void            insertTLBCmd( char *cmdBuf );
    void            displayCacheCmd( char *cmdBuf );
    void            purgeCacheCmd( char *cmdBuf );
    void            displayAbsMemCmd( char *cmdBuf );
    void            displayAbsMemAsCodeCmd( char *cmdBuf );
    void            modifyAbsMemCmd( char *cmdBuf );
    void            modifyAbsMemAsCodeCmd( char *cmdBuf );
    void            loadPhysMemCmd( char *cmdBuf );
    void            savePhysMemCmd( char *cmdBuf );
    
    void            winOnCmd( char *cmdBuf );
    void            winOffCmd( char *cmdBuf );
    void            winDefCmd( char *cmdBuf );
    void            winCurrentCmd( char *cmdBuf );
    void            winStacksEnable( char *cmdBuf );
    void            winStacksDisable( char *cmdBuf );
    
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
    TokId           currentCmd = TOK_INV;
    
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

#endif /* CPU24Driver_hpp */
