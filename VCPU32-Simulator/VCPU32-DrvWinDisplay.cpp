//------------------------------------------------------------------------------------------------------------
//
// VCPU32 - A 32-bit CPU - Simulator Commands window mode display
//
//------------------------------------------------------------------------------------------------------------
// This module contains the window display routines used by the command interpreter. There are two modes,
// the line modes and the windows mode. The window mode uses a ton of escape sequences to create a terminal
// window screen and displays sub windows on the screen.
//
//------------------------------------------------------------------------------------------------------------
//
// VCPU32 - A 32-bit CPU - Simulator Commands window mode display
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
#include "VCPU32-Types.hpp"
#include "VCPU32-Core.hpp"
#include "VCPU32-Driver.hpp"

//------------------------------------------------------------------------------------------------------------
//
//  Global Window commands:
//
//  WON, WOFF   -> on, off
//  WRED        -> Redraw
//  WDEF        -> window defaults, show initial screen.
//
//  Stacks:
//
//  WSE, WSD        -> winStackEnable/Disable
//  UWSA, UWSB      -> setUserWinStack
//
//  Window:
//
//  enable, disable -> winEnable        -> E, D
//  back, forward   -> winMove          -> B, F
//  home, jump      -> winJump          -> H, J
//  rows            -> setRows          -> L
//  radix           -> setRadix         -> R
//  new             -> newUswerWin      -> N
//  kill            -> winUserKill      -> K
//  current         -> currentUserWin   -> C
//  toggle          -> winToggle        -> T
//
//  Windows:
//
//  Program Regs    -> PS
//  Special Regs    -> CR
//  Pipeline Regs   -> PL
//  Statistics      -> ST
//  Program Code    -> PC
//  TLB             -> IT, DT
//  T-Controller    -> ITR, DTR
//  Cache           -> IC, DC, UC
//  C-Controller    -> ICR, DCR, UCR
//  Text Window     -> TX
//  User Defined    -> UW
//  Commands        -> n/a
//
//  Combine the window command with the window to form the command to type.
//  Example: PSE -> enable general regs window.
//  Note: not all combinations are possible...
//
//------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------
// Local name space. We try to keep utility functions local to the file.
//
//------------------------------------------------------------------------------------------------------------
namespace {

//------------------------------------------------------------------------------------------------------------
// Fundamental constants for the window system.
//
//------------------------------------------------------------------------------------------------------------
const int MAX_TEXT_FIELD_LEN    = 132;
const int MAX_TEXT_LINE_SIZE    = 256;

const int MAX_WIN_ROW_SIZE      = 64;
const int MAX_WIN_COL_SIZE      = 256;
const int MAX_WINDOWS           = 32;
const int MAX_WIN_STACKS        = 4;

//------------------------------------------------------------------------------------------------------------
// Windows have a type. The type is primarily used to specify what type of window to create.
//
//------------------------------------------------------------------------------------------------------------
enum winType : int {
    
    WT_NIL          = 0,
    WT_CMD_WIN      = 1,
    WT_PS_WIN       = 2,
    WT_CR_WIN       = 3,
    WT_PL_WIN       = 4,
    WT_ST_WIN       = 5,
    WT_PM_WIN       = 6,
    WT_PC_WIN       = 7,
    WT_ITLB_WIN     = 8,
    WT_DTLB_WIN     = 9,
    WT_ICACHE_WIN   = 10,
    WT_DCACHE_WIN   = 11,
    WT_UCACHE_WIN   = 12,
    WT_ICACHE_S_WIN = 13,
    WT_DCACHE_S_WIN = 14,
    WT_UCACHE_S_WIN = 15,
    WT_MEM_S_WIN    = 16,
    WT_PDC_S_WIN    = 17,
    WT_IO_S_WIN     = 18,
    WT_ITLB_S_WIN   = 19,
    WT_DTLB_S_WIN   = 20,
    WT_TEXT_WIN     = 21
};

//------------------------------------------------------------------------------------------------------------
// Predefined windows are displayed in a fixed order when enabled. The following constants are the index of
// these windows in the window table. The first entries are used by the fixed windows and their order is
// determined by the index number assigned. All these windows are created at program start time. An index
// starting with the first user index is used for dynamically created windows.
//
//------------------------------------------------------------------------------------------------------------
enum windowIndex : int {
    
    PS_REG_WIN      = 0,
    CTRL_REG_WIN    = 1,
    PL_REG_WIN      = 2,
    STATS_WIN       = 3,
    FIRST_UWIN      = 4,
    LAST_UWIN       = 31
};

//------------------------------------------------------------------------------------------------------------
// Format descriptor for putting out a field. The options are simply ORed. The idea is that a format descriptor
// can be assembled once and used for many fields. A value of zero will indicate to simply use the previously
// established descriptor set by the attributes routine.
//
//------------------------------------------------------------------------------------------------------------
enum fmtDescOptions : uint32_t {
    
    FMT_USE_ACTUAL_ATTR = 0x0,
    
    FMT_BG_COL_DEF      = 0x00000001,
    FMT_BG_COL_RED      = 0x00000002,
    FMT_BG_COL_GREEN    = 0x00000003,
    FMT_BG_COL_YELLOW   = 0x00000004,
    
    FMT_FG_COL_DEF      = 0x00000010,
    FMT_FG_COL_RED      = 0x00000020,
    FMT_FG_COL_GREEN    = 0x00000030,
    FMT_FG_COL_YELLOW   = 0x00000040,
    
    FMT_BOLD            = 0x00000100,
    FMT_BLINK           = 0x00000200,
    FMT_INVERSE         = 0x00000400,
    FMT_ALIGN_LFT       = 0x00000800,
    FMT_TRUNC_LFT       = 0x00001000,
    
    FMT_LAST_FIELD      = 0x00002000,
    FMT_HALF_WORD       = 0x00004000,
    FMT_INVALID_NUM     = 0x00008000,
    
    FMT_DEF_ATTR        = 0x10000000
};

//-----------------------------------------------------------------------------------------------------------
// The list of windows. This local structure holds a reference to all defined windows. The first entries
// are asssigned in a fixed manner, these windows are created once and cannot be removed. The remaining
// user definable windows are added and removed dynamically.
//
//-----------------------------------------------------------------------------------------------------------
DrvWin         *windowList[ MAX_WINDOWS ];
DrvWinCommands *cmdWin;

//-----------------------------------------------------------------------------------------------------------
//
//
//-----------------------------------------------------------------------------------------------------------
uint32_t getBitField( uint32_t arg, int pos, int len, bool sign = false ) {
    
    pos = pos % 32;
    len = len % 32;
    
    uint32_t tmpM = ( 1U << len ) - 1;
    uint32_t tmpA = arg >> ( 31 - pos );
    
    if ( sign ) return( tmpA | ( ~ tmpM ));
    else        return( tmpA & tmpM );
}

//-----------------------------------------------------------------------------------------------------------
// All fprintf calls are routed through this routine. I want to see if any of the fprintf function calls
// returns an error, which may be the clue to why sometimes the screen hangs. We use snprintf to produce
// the string. This can be done many times without loosing or doubling any data. If the print operation is
// successful, we have the buffer for writing. Then we issue the write operation. It is the same idea, if
// the write aborts, it can be restarted without the fear that any data was written before the operation
// was interrupted. Note that the comiler will issue a waring, for passing a varianle "fmt" to the print
// function, which it cannot identy as a string literal. After all, "fmt" could be anything. Since we
// only use the "winPrintf" function in this file, no one else could do stupid things with it, except me.
//
// So far, the error never returned...
//-----------------------------------------------------------------------------------------------------------
template<typename... Args> int winPrintf( FILE *stream, const char *fmt, Args... args) {
    
    static char buf[ 256 ];
    
    size_t len = 0;
    
    do {
        
        len = snprintf( buf, sizeof( buf ), fmt, args... );
        
        if (( len < 0 ) && ( errno != EINTR )) {
            
            fprintf( stderr, "myPrintf (snprintf) error, errno: %lu, %s\n", len, strerror( errno ));
            exit( errno );
        }
    }
    
    while ( len < 0 );
    
    do {
        
        len = write( fileno( stdout ), buf, len );
        
        if (( len < 0 ) && ( errno != EINTR )) {
            
            fprintf( stderr, "myPrintf (write) error, errno: %lu, %s\n", len, strerror( errno ));
            exit( errno );
        }
    }
    while ( len < 0 );
    
    return((int) len );
}

//------------------------------------------------------------------------------------------------------------
// Building a screen will imply a ton of escape sequence to send to the terminal screen. The following batch
// of routines will put out the escape sequcence for clearing data, position a cursor and so on. There is
// a lot of printf that will take place. A future version could come up with a string concatenation scheme,
// and use fewer writes.
//
//------------------------------------------------------------------------------------------------------------
void clearScreen( ) {
    
    winPrintf( stdout, "\x1b[2J" );
    winPrintf( stdout, "\x1b[3J" );
}

void setAbsCursor( int row, int col ) {
    
    winPrintf( stdout, "\x1b[%d;%dH", row, col );
}

void setWindowSize( int row, int col ) {
    
    winPrintf( stdout, "\x1b[8;%d;%dt", row, col );
}

void setScrollArea( int start, int end ) {
    
    winPrintf( stdout, "\x1b[%d;%dr", start, end );
}

void clearScrollArea( ) {
    
    winPrintf( stdout, "\x1b[r" );
}

//------------------------------------------------------------------------------------------------------------
// A window will consist of lines with lines having fields on them. A field has a set of atributes such as
// foreground and background colors, bold characters and so on. This routine sets the attributes based on the
// format descriptor. If the descriptor is zero, we will just stay where are with theice attributes.
//
//------------------------------------------------------------------------------------------------------------
void setFieldAtributes( uint32_t fmtDesc ) {
    
    if ( fmtDesc != 0 ) {
        
        winPrintf( stdout, "\x1b[0m" );
        if ( fmtDesc & FMT_INVERSE )    winPrintf( stdout, "\x1b[7m" );
        if ( fmtDesc & FMT_BLINK )      winPrintf( stdout, "\x1b[5m" );
        if ( fmtDesc & FMT_BOLD )       winPrintf( stdout, "\x1b[1m" );
        
        switch ( fmtDesc & 0xF ) {
                
            case 1:     winPrintf( stdout, "\x1b[41m"); break;
            case 2:     winPrintf( stdout, "\x1b[42m"); break;
            case 3:     winPrintf( stdout, "\x1b[43m"); break;
            default:    winPrintf( stdout, "\x1b[49m");
        }
        
        switch (( fmtDesc >> 4 ) & 0xF ) {
                
            case 1:     winPrintf( stdout, "\x1b[31m"); break;
            case 2:     winPrintf( stdout, "\x1b[32m"); break;
            case 3:     winPrintf( stdout, "\x1b[33m"); break;
            default:    winPrintf( stdout, "\x1b[39m");
        }
    }
}

//------------------------------------------------------------------------------------------------------------
// Routine to figure out what size we need for a numeric word in a given radix. Decimals needs 10 digits,
// octals need 12 digits and hexadcimals need 10 digits. For a 16-bit word, the numbers are reduced to 5, 7
// and 6.
//
//------------------------------------------------------------------------------------------------------------
int strlenForNum( TokId rdx, bool halfWord ) {
    
    if      ( rdx == TOK_DEC ) return(( halfWord ) ? 5 : 10 );
    else if ( rdx == TOK_OCT ) return(( halfWord ) ? 7 : 12 );
    else if ( rdx == TOK_HEX ) return(( halfWord ) ? 6 : 10 );
    else return( 10 );
}

//------------------------------------------------------------------------------------------------------------
// Routine for putting out a 32-bit or 16-bit machine word at the current cursor position. We will just print
// out the data using the radix passed. ( HEX: 0xdddddddd, OCT: 0ddddddddddd, DEC: dddddddddd );
//
//------------------------------------------------------------------------------------------------------------
int printWord( uint32_t val, TokId radix = TOK_HEX, uint32_t fmtDesc = FMT_DEF_ATTR ) {
    
    int len;
    
    bool half   = fmtDesc & FMT_HALF_WORD;
    bool noNum  = fmtDesc & FMT_INVALID_NUM;
    
        if ( radix == TOK_DEC ) {
            
            if ( noNum ) {
                
                if ( half ) len = winPrintf( stdout, "*****" );
                else        len = winPrintf( stdout, "**********" );
            }
            else {
                
                if ( half ) len = winPrintf( stdout, "%5d", val );
                else        len = winPrintf( stdout, "%10d", val );
            }
        }
        else if ( radix == TOK_OCT ) {
            
            if ( noNum ) {
                
                if ( half ) len = winPrintf( stdout, "*******" );
                else        len = winPrintf( stdout, "************" );
            }
            else {
                
                if ( half ) len = winPrintf( stdout, "%07o", val );
                else        len = winPrintf( stdout, "%#012o", val );
            }
        }
        else if ( radix == TOK_HEX ) {
            
            if ( noNum ) {
                
                if ( half ) len = winPrintf( stdout, "******" );
                else        len = winPrintf( stdout, "**********" );
            }
            else {
                
                if ( val == 0 ) {
                    
                    if ( half ) len = winPrintf( stdout, "0x0000" );
                    else        len = winPrintf( stdout, "0x00000000" );
                    
                } else {
                    
                    if ( half ) len = winPrintf( stdout, "%#06x", val );
                    else        len = winPrintf( stdout, "%#010x", val );
                }
            }
        }
        else len = winPrintf( stdout, "***num***" );
    
    
    return( len );
}

//------------------------------------------------------------------------------------------------------------
// "displayInvalidWord" shows a set of "*" when we cannot get a value for word. We make the length of the
// "*" string accoriding to the current radix.
//
//------------------------------------------------------------------------------------------------------------
void printInvalidWord( TokId fmtType ) {
    
    if      ( fmtType == TOK_DEC )  winPrintf( stdout, "**********" );
    else if ( fmtType == TOK_OCT )  winPrintf( stdout, "************" );
    else if ( fmtType == TOK_HEX )  winPrintf( stdout, "**********" );
    else                            winPrintf( stdout, "**num**" );
}

//------------------------------------------------------------------------------------------------------------
// Routine for putting out simple text. We make sure that the string length is in the range of what the text
// size could be.
//
//------------------------------------------------------------------------------------------------------------
int printText( char *text, int len ) {
    
    if ( strlen( text ) < MAX_TEXT_FIELD_LEN ) return( winPrintf( stdout, "%s", text ));
    else return( winPrintf( stdout, "***Text***" ));
}

//------------------------------------------------------------------------------------------------------------
// Fields that have a larger size than the actual argument length in the field need to be padded left or
// right. This routine is just a simple loop emitting blanks in the current format set.
//
//------------------------------------------------------------------------------------------------------------
void padField( int dLen, int fLen ) {
    
    while ( fLen > dLen ) {
        
        winPrintf( stdout, " " );
        fLen --;
    }
}

//------------------------------------------------------------------------------------------------------------
// Routine for creating the access rights string. It consists of the page access and the two privilege
// levles.
//
//------------------------------------------------------------------------------------------------------------
void buildAccessRightsStr( char *bufStr, int bufLen, uint8_t type, uint8_t privLev1, uint8_t privLev2 ) {
    
    switch( type ) {
            
        case ACC_READ_ONLY:     snprintf( bufStr, bufLen, "[ro:%1d:%1d]", privLev1, privLev2 ); break;
        case ACC_READ_WRITE:    snprintf( bufStr, bufLen, "[rw:%1d:%1d]", privLev1, privLev2 ); break;
        case ACC_EXECUTE:       snprintf( bufStr, bufLen, "[ex:%1d:%1d]", privLev1, privLev2 ); break;
        default:                snprintf( bufStr, bufLen, "[xx:%1d:%1d]", privLev1, privLev2 ); break;
    }
}

// ??? perhaps routines for building the status word bits, the cache line bits, tlb entry bits ...

//------------------------------------------------------------------------------------------------------------
// "setRadix" ensures that we passed in a valid radix value. The default is a decimal number.
//
//------------------------------------------------------------------------------------------------------------
TokId setRadix( TokId rdx ) {
    
    return((( rdx == TOK_OCT ) || ( rdx == TOK_DEC ) || ( rdx == TOK_HEX )) ? rdx : TOK_DEC );
}

}; // namespace


//***********************************************************************************************************
//***********************************************************************************************************
//
// Methods for the ScreenWindow abstract class.
//
//***********************************************************************************************************
//***********************************************************************************************************

//------------------------------------------------------------------------------------------------------------
// Object constructor and destructor. We need themm as we ccreate and destroy user definable windows.
//
//------------------------------------------------------------------------------------------------------------
DrvWin::DrvWin( VCPU32Globals *glb ) { this -> glb = glb; }
DrvWin:: ~DrvWin( ) { }

//------------------------------------------------------------------------------------------------------------
// Getter/Setter methods for window attributes.
//
//------------------------------------------------------------------------------------------------------------
void    DrvWin::setWinType( int arg ) { winType = arg; }
int     DrvWin::getWinType( ) { return( winType ); }

void    DrvWin::setWinIndex( int arg ) { winUserIndex = arg; }
int     DrvWin::getWinIndex( ) { return( winUserIndex ); }

void    DrvWin::setEnable( bool arg ) { winEnabled = arg; }
bool    DrvWin::isEnabled( ) { return( winEnabled ); }

void    DrvWin::setRows( int arg ) { winRows = (( arg > MAX_WIN_ROW_SIZE ) ? MAX_WIN_ROW_SIZE : arg ); }
int     DrvWin::getRows( ) { return( winRows ); }

void    DrvWin::setColumns( int arg ) { winColumns = arg; }
int     DrvWin::getColumns( ) { return( winColumns ); }

void    DrvWin::setRadix( TokId rdx ) { winRadix = ::setRadix( rdx ); }
TokId   DrvWin::getRadix( ) { return( winRadix ); }

int     DrvWin::getWinStack( ) { return( winStack ); }
void    DrvWin::setWinStack( int wCol ) { winStack = wCol; }

int DrvWin::getDefColumns( TokId rdx ) {
    
    switch ( rdx ) {
            
        case TOK_HEX: return( winDefColumnsHex );
        case TOK_OCT: return( winDefColumnsOct );
        case TOK_DEC: return( winDefColumnsDec );
        default:      return( winDefColumnsHex );
    }
}

void DrvWin::setDefColumns( int arg, TokId rdx ) {
    
    switch ( rdx ) {
            
        case TOK_HEX: winDefColumnsHex = arg;  break;
        case TOK_OCT:  winDefColumnsOct = arg; break;
        case TOK_DEC:  winDefColumnsDec = arg; break;
        default: winDefColumnsHex = winDefColumnsOct = winDefColumnsDec = arg;
    }
}

//------------------------------------------------------------------------------------------------------------
// "setWinOrigin" sets the absolute cursor position for the terminal screen. We maintain absolute positions,
// which only may change when the terminal screen is redrawn with different window sizes. The window relative
// rows and column cursor position are set at (1,1).
//
//------------------------------------------------------------------------------------------------------------
void DrvWin::setWinOrigin( int row, int col ) {
    
    winAbsCursorRow = row;
    winAbsCursorCol = col;
    lastRowPos      = 1;
    lastColPos      = 1;
}

//------------------------------------------------------------------------------------------------------------
// "setWinCursor" sets the cursor to a windows relative position if row and column are non-zero. If they are
// zero, the last relative cursor position is used. The final absolute position is computed from the windows
// absolute row and column on the terminal screen plus the window relative row and column.
//
//------------------------------------------------------------------------------------------------------------
void DrvWin::setWinCursor( int row, int col ) {
    
    if ( row == 0 ) row = lastRowPos;
    if ( col == 0 ) col = lastColPos;
    
    if ( row > winRows )            row = winRows;
    if ( col > MAX_WIN_COL_SIZE )   col = MAX_WIN_COL_SIZE;
    
    setAbsCursor( winAbsCursorRow + row - 1, winAbsCursorCol + col );
    
    lastRowPos = row;
    lastColPos = col;
}

int DrvWin::getWinCursorRow( ) { return( lastRowPos ); }
int DrvWin::getWinCursorCol( ) { return( lastColPos ); }

//------------------------------------------------------------------------------------------------------------
// Print out a numeric field. Each call will set the format options passed via the format descriptor. If the
// field length is larger than the positions needed to print the data in the field, the data will be printed
// left or right justified in the field.
//
// ??? shouldn't we feed into printFieldText to unify field printing ?
// ??? add radix to override default setting ?
//-----------------------------------------------------------------------------------------------------------
void DrvWin::printNumericField( uint32_t val, uint32_t fmtDesc, int fLen, int row, int col ) {
    
    if ( row == 0 )                     row     = lastRowPos;
    if ( col == 0 )                     col     = lastColPos;
    if ( fmtDesc & FMT_LAST_FIELD )     col     = winColumns - fLen;
    
    int maxLen = strlenForNum( getRadix( ), ( fmtDesc & FMT_HALF_WORD ));
   
    if ( fLen == 0 ) fLen = maxLen;
    
    setFieldAtributes( fmtDesc );
    setWinCursor( row, col );
   
    if ( fLen > maxLen ) {
        
        if ( fmtDesc & FMT_ALIGN_LFT ) {
            
            printWord( val, winRadix, fmtDesc );
            padField( maxLen, fLen );
        }
        else {
            
            padField( maxLen, fLen );
            printWord( val, winRadix, fmtDesc );
        }
    }
    else printWord( val, winRadix, fmtDesc );
    
    lastRowPos  = row;
    lastColPos  = col + fLen;
}

//------------------------------------------------------------------------------------------------------------
// Print out a text field. Each call will set the format options passed via the format descriptor. If the
// field length is larger than the positions needed to print the data in the field, the data will be printed
// left or right justified in the field. If teh data is larger than the field, it will be truncated.
//
//-----------------------------------------------------------------------------------------------------------
void DrvWin::printTextField( char *text, uint32_t fmtDesc, int fLen, int row, int col ) {
    
    if ( row == 0 ) row = lastRowPos;
    if ( col == 0 ) col = lastColPos;
    
    int dLen = (int) strlen( text );
    if ( dLen > MAX_TEXT_FIELD_LEN ) dLen = MAX_TEXT_FIELD_LEN;
    
    if ( fLen == 0 ) {
        
        if ( dLen > MAX_TEXT_FIELD_LEN ) dLen = MAX_TEXT_FIELD_LEN;
        fLen = dLen;
    }
    
    if ( fmtDesc & FMT_LAST_FIELD ) col = winColumns - fLen;
    
    setWinCursor( row, col );
    setFieldAtributes( fmtDesc );
    
    if ( fLen > dLen ) {
        
        if ( fmtDesc & FMT_ALIGN_LFT ) {
            
            printText( text, dLen );
            padField( dLen, fLen );
        }
        else {
            
            padField( dLen, fLen );
            printText( text, dLen );
        }
    }
    else if ( fLen < dLen ) {
        
        if ( fmtDesc & FMT_TRUNC_LFT ) {
            
            printText(( char *) "...", 3 );
            printText((char *) text + ( dLen - fLen ) + 3, fLen - 3 );
        }
        else {
            
            printText( text, fLen - 3 );
            printText(( char *) "...", 3 );
        }
    }
    else printText( text, dLen );
    
    lastRowPos  = row;
    lastColPos  = col + fLen;
}

//------------------------------------------------------------------------------------------------------------
// It is a good idea to put the current radix into the banner line to show in what format the data in the
// body is presented. This field is when used always printed as the last field in the banner line.
//
//------------------------------------------------------------------------------------------------------------
void DrvWin::printRadixField( uint32_t fmtDesc, int fLen, int row, int col ) {
    
    setFieldAtributes( fmtDesc );
    
    if ( fmtDesc & FMT_LAST_FIELD ) col = winColumns - fLen;
    
    switch ( winRadix ) {
            
        case TOK_OCT: printTextField((char *) "oct", fmtDesc, 3, row, col); break;
        case TOK_DEC: printTextField((char *) "dec", fmtDesc, 3, row, col); break;
        case TOK_HEX: printTextField((char *) "hex", fmtDesc, 3, row, col); break;
        default: ;
    }
}

//------------------------------------------------------------------------------------------------------------
// A user defined window has a field that shows the window number as well as this is the current window.
//
//------------------------------------------------------------------------------------------------------------
void DrvWin::printWindowIdField( int stack, int index, bool current, uint32_t fmtDesc, int row, int col ) {
    
    if ( row == 0 ) row = lastRowPos;
    if ( col == 0 ) col = lastColPos;
    
    setFieldAtributes( fmtDesc );
    
    if      (( index >= 0   ) && ( index <= 10 ))   winPrintf( stdout, "(%1d:%1d)  ", stack, index );
    else if (( index >= 10  ) && ( index <= 99 ))   winPrintf( stdout, "(%1d:%2d) ", stack, index );
    else                                            winPrintf( stdout, "-***-" );
    
    winPrintf( stdout, (( current ) ? "* " : "  " ));
    
    lastRowPos  = row;
    lastColPos  = col + 9;
}

//------------------------------------------------------------------------------------------------------------
// Padding a line will write a set of blanks with the current fortmat setting to the end of the line. It is
// intended to fill for example a banner line that is in inverse video with the inverse format until the end
// of the screen column size.
//
//------------------------------------------------------------------------------------------------------------
void DrvWin::padLine( uint32_t fmtDesc ) {
    
    setFieldAtributes( fmtDesc );
    padField( lastColPos, winColumns );
}

//------------------------------------------------------------------------------------------------------------
// Each window consist of a banner and a body. The reDraw routine will invoke these mandatory routines of
// the child classes.
//
//------------------------------------------------------------------------------------------------------------
void DrvWin::reDraw( ) {
    
    if ( winEnabled ) {
        
        drawBanner( );
        drawBody( );
    }
}

//------------------------------------------------------------------------------------------------------------
// Each window allows for perhaps toggling through different content. The implmentation of this capabilty is
// entirely up to the specific window. On the "WT" command, this function wil be called.
//
//------------------------------------------------------------------------------------------------------------
void DrvWin::toggleWin( ) { }

//***********************************************************************************************************
//***********************************************************************************************************
//
// Methods for the scrollable window abstract class.
//
//***********************************************************************************************************
//***********************************************************************************************************

//------------------------------------------------------------------------------------------------------------
// Object creator.
//
//------------------------------------------------------------------------------------------------------------
DrvWinScrollable::DrvWinScrollable( VCPU32Globals *glb ) : DrvWin( glb ) { }

//------------------------------------------------------------------------------------------------------------
// Getter/Setter methods for scrollable window attributes.
//
//------------------------------------------------------------------------------------------------------------
void      DrvWinScrollable::setHomeItemAdr( uint32_t adr ) { homeItemAdr = adr; }
uint32_t  DrvWinScrollable::getHomeItemAdr( ) { return( homeItemAdr ); }

void      DrvWinScrollable::setCurrentItemAdr( uint32_t adr ) { currentItemAdr = adr; }
uint32_t  DrvWinScrollable::getCurrentItemAdr( ) { return( currentItemAdr ); }

void      DrvWinScrollable::setLimitItemAdr( uint32_t adr ) { limitItemAdr = adr; }
uint32_t  DrvWinScrollable::getLimitItemAdr( ) { return( limitItemAdr ); }

void      DrvWinScrollable::setLineIncrement( uint32_t arg ) { lineIncrement = arg; }
uint32_t  DrvWinScrollable::getLineIncrement( ) { return( lineIncrement ); }

//------------------------------------------------------------------------------------------------------------
// The scrollable window inherits from the general window. While the banner part of a window is expected to
// be implemented by the inheriting class, the body is done by this class, which will call the "drawLine"
// method implemented by the inheriting class. The "drawLine" method is passed the current item address
// which is the current line start of the item of whatever the window is displaying. The item adrress value
// is incremented by the itemsPerLine value each time the drawLine routine is called. The cursor position for
// "drawLine" method call is incremented by the linesPerItem amount. Note that the window system thinks in
// lines. If a window has items that occupy more than one line ( linesPerIem > 1 ), the line count in the
// window needs to be divided by that value.
//
//------------------------------------------------------------------------------------------------------------
void DrvWinScrollable::drawBody( ) {
    
    int numOfItemLines = ( getRows( ) - 1 );
    
    for ( int line = 0; line < numOfItemLines; line++ ) {
        
        setWinCursor( line + 2, 1 );
        drawLine( currentItemAdr + ( line * lineIncrement ));
    }
}

//------------------------------------------------------------------------------------------------------------
// The "winHome" method moves the starting item address of a window within the boundaries zero and the limit
// address and sets it as the new home for the "home" command. An argument of zero will set the window back
// to the current home address. If the address is larger than the limit address of the window, the position
// will be the limit address minus the number of lines times the number of items on the line.
//
// ??? unsigned adresses ?
//------------------------------------------------------------------------------------------------------------
void DrvWinScrollable::winHome( uint32_t pos ) {
    
    if ( pos > 0 ) {
        
        int itemsPerWindow = ( getRows( ) - 1 ) * lineIncrement;
        
        if ( pos > limitItemAdr - itemsPerWindow ) homeItemAdr = limitItemAdr - itemsPerWindow;
        homeItemAdr       = pos;
        currentItemAdr    = pos;
    }
    else currentItemAdr = homeItemAdr;
}

//------------------------------------------------------------------------------------------------------------
// The "winJump" method moves the starting item adress of a window within the boundaries zero and the limit
// address.
//
//------------------------------------------------------------------------------------------------------------
void DrvWinScrollable::winJump( uint32_t pos ) {
    
   currentItemAdr = pos;
}

//------------------------------------------------------------------------------------------------------------
// Window move implements the forward / backward moves of a window. The amount is added to the current window
// body position, also making sure that we stay inside the boundaries of the address range for the window.
// If the new position would point beyound the limit address, we set the new item adress to limit minus the
// window lines times the line increment. Likewise of the new item address would be less than zero, we
// just set it to zero.
//
//------------------------------------------------------------------------------------------------------------
void DrvWinScrollable::winForward( uint32_t amt ) {
    
    if ( amt == 0 ) amt = ( getRows( ) - 1 ) * lineIncrement;
    
    if (((uint64_t) currentItemAdr + amt ) > (uint64_t) limitItemAdr ) {
        
        currentItemAdr = limitItemAdr - (( getRows( ) - 1 ) * lineIncrement );
    }
    else currentItemAdr = currentItemAdr + amt;
}

void DrvWinScrollable::winBackward( uint32_t amt ) {
    
    if ( amt == 0 ) amt = ( getRows( ) - 1 ) * lineIncrement;
    
    if ( amt <= currentItemAdr ) {
        
        currentItemAdr = currentItemAdr - amt;
    }
    else currentItemAdr = 0;
}

//***********************************************************************************************************
//***********************************************************************************************************
//
// Methods for the Program State Window class.
//
//***********************************************************************************************************
//***********************************************************************************************************

//------------------------------------------------------------------------------------------------------------
// Object creator.
//
//------------------------------------------------------------------------------------------------------------
DrvWinProgState::DrvWinProgState( VCPU32Globals *glb ) : DrvWin( glb ) { }

//------------------------------------------------------------------------------------------------------------
// The default values are the initial settings when windows is brought up the first time, or for the WDEF
// command.
//
//------------------------------------------------------------------------------------------------------------
void DrvWinProgState::setDefaults( ) {
    
    setRadix( glb -> env -> getEnvValTok( ENV_FMT_DEF ));
    
    setDefColumns( 12 + ( 8 * 11 ), TOK_HEX );
    setDefColumns( 12 + ( 8 * 13 ), TOK_OCT );
    setDefColumns( 12 + ( 8 * 11 ), TOK_DEC );
    setColumns( getDefColumns( getRadix( )));
    setRows( 4 );

    setWinType( WT_PS_WIN );
    setEnable( true );
}

//------------------------------------------------------------------------------------------------------------
// The window overrides the setRadix method to set the column width according to the radix choosen.
//
//------------------------------------------------------------------------------------------------------------
void DrvWinProgState::setRadix( TokId rdx ) {
    
    DrvWin::setRadix( rdx );
    setColumns( getDefColumns( getRadix( )));
}

//------------------------------------------------------------------------------------------------------------
// Each window consist of a headline and a body. The banner line is always shown in inverse and contains
// summary or head data for the window. The program state banner lists the instruction address and the
// status word.
//
//------------------------------------------------------------------------------------------------------------
void DrvWinProgState::drawBanner( ) {
    
    uint32_t fmtDesc = FMT_BOLD | FMT_INVERSE | FMT_ALIGN_LFT ;
    
    setWinCursor( 1, 1 );
    printTextField((char *) "Program State", ( fmtDesc ), 16 );
    
    
    printTextField((char *) "Seg:", fmtDesc, 5 );
    printNumericField( glb -> cpu -> getReg( RC_PROG_STATE, PS_REG_PSW_0 ) & 0xFFFF, fmtDesc | FMT_HALF_WORD, 8 );
    printTextField((char *) "Ofs:", fmtDesc, 5 );
    printNumericField( glb -> cpu -> getReg( RC_PROG_STATE, PS_REG_PSW_1 ), fmtDesc, 12 );
    printTextField((char *) "ST:", fmtDesc, 4 );
    
    uint32_t stat = glb -> cpu -> getReg( RC_PROG_STATE, PS_REG_PSW_0 );
    
    printTextField(( stat & ST_MACHINE_CHECK ) ? (char *) "M" : (char *) "m", fmtDesc );
    printTextField(( stat & ST_CODE_TRANSLATION_ENABLE ) ? (char *) "I" : (char *) "i", fmtDesc );
    printTextField(( stat & ST_CARRY ) ? (char *) "C" : (char *) "c", fmtDesc );
    printTextField(( stat & ST_PROTECT_ID_CHECK_ENABLE ) ? (char *) "P" : (char *) "p", fmtDesc );
    printTextField(( stat & ST_DATA_TRANSLATION_ENABLE ) ? (char *) "D" : (char *) "d", fmtDesc );
    printTextField(( stat & ST_INTERRUPT_ENABLE ) ? (char *) "E" : (char *) "e", fmtDesc );
    
    padLine( fmtDesc );
    printRadixField( fmtDesc | FMT_LAST_FIELD );
}

//------------------------------------------------------------------------------------------------------------
// Each window consist of a banner and a body. The body lines are displayed after the banner line. The
// program state window body lists the general and segment registers.
//
//------------------------------------------------------------------------------------------------------------
void DrvWinProgState::drawBody( ) {
    
    uint32_t fmtDesc = FMT_DEF_ATTR;
    
    setWinCursor( 2, 1 );
    printTextField((char *) "GR0=", ( fmtDesc | FMT_BOLD | FMT_ALIGN_LFT ), 6 );
    
    for ( int i = 0; i < 4; i++ ) {
        
        printNumericField( glb -> cpu -> getReg( RC_GEN_REG_SET, i ), fmtDesc );
        printTextField((char *) " ", fmtDesc );
    }
       
    printTextField((char *) "GR4=", ( fmtDesc | FMT_BOLD | FMT_ALIGN_LFT ), 6 );
    
    for ( int i = 4; i < 8; i++ ) {
        
        printNumericField( glb -> cpu -> getReg( RC_GEN_REG_SET, i ), fmtDesc );
        printTextField((char *) " ", fmtDesc );
    }
       
    padLine( fmtDesc );
    
    setWinCursor( 3, 1 );
    printTextField((char *) "GR8=", ( fmtDesc | FMT_BOLD | FMT_ALIGN_LFT ), 6 );
    
    for ( int i = 8; i < 12; i++ ) {
        
        printNumericField( glb -> cpu -> getReg( RC_GEN_REG_SET, i ), fmtDesc );
        printTextField((char *) " " );
    }
        
    printTextField((char *) "GR12=", ( fmtDesc | FMT_BOLD | FMT_ALIGN_LFT ), 6 );
    
    for ( int i = 12; i < 16; i++ ) {
        
        printNumericField( glb -> cpu -> getReg( RC_GEN_REG_SET, i ), fmtDesc );
        printTextField((char *) " " );
    }
        
    padLine( fmtDesc );
    
    setWinCursor( 4, 1 );
    printTextField((char *) "SR0=", ( fmtDesc | FMT_BOLD | FMT_ALIGN_LFT ), 6 );
    
    for ( int i = 0; i < 4; i++ ) {
        
        printNumericField( glb -> cpu -> getReg( RC_SEG_REG_SET, i ), fmtDesc );
        printTextField((char *) " " );
    }
        
    printTextField((char *) "SR4=", ( fmtDesc | FMT_BOLD | FMT_ALIGN_LFT ), 6 );
    
    for ( int i = 4; i < 8; i++ ) {
    
        printNumericField( glb -> cpu -> getReg( RC_SEG_REG_SET, i ), fmtDesc );
        printTextField((char *) " " );
    }
        
    padLine( fmtDesc );
}

//***********************************************************************************************************
//***********************************************************************************************************
//
// Methods for the special register window class.
//
//***********************************************************************************************************
//***********************************************************************************************************

//------------------------------------------------------------------------------------------------------------
// Object constructor.
//
//------------------------------------------------------------------------------------------------------------
DrvWinSpecialRegs::DrvWinSpecialRegs( VCPU32Globals *glb )  : DrvWin( glb ) { }

//------------------------------------------------------------------------------------------------------------
// The default values are the initial settings when windows is brought up the first time, or for the WDEF
// command.
//
//------------------------------------------------------------------------------------------------------------
void DrvWinSpecialRegs::setDefaults( ) {
    
    setRadix( glb -> env -> getEnvValTok( ENV_FMT_DEF ));
    
    setDefColumns( 12 + ( 8 * 11 ), TOK_HEX );
    setDefColumns( 12 + ( 8 * 13 ), TOK_OCT );
    setDefColumns( 12 + ( 8 * 11 ), TOK_DEC );
    setColumns( getDefColumns( getRadix( )));
    setRows( 5 );

    setWinType( WT_CR_WIN );
    setEnable( false );
}

//------------------------------------------------------------------------------------------------------------
// The window overrides the setRadix method to set the column width according to the radix choosen.
//
//------------------------------------------------------------------------------------------------------------
void DrvWinSpecialRegs::setRadix( TokId rdx ) {
    
    DrvWin::setRadix( rdx );
    setColumns( getDefColumns( getRadix( )));
}

//------------------------------------------------------------------------------------------------------------
// The banner line for the special register window.
//
//------------------------------------------------------------------------------------------------------------
void DrvWinSpecialRegs::drawBanner( ) {
    
    uint32_t fmtDesc = FMT_BOLD | FMT_INVERSE;
    
    setWinCursor( 1, 1 );
    printTextField((char *) "Special Reg", ( fmtDesc | FMT_ALIGN_LFT ), 16 );
    padLine( fmtDesc );
    printRadixField( fmtDesc | FMT_LAST_FIELD );
}

//------------------------------------------------------------------------------------------------------------
// Each window consist of a banner and a body. The body lines are displayed after the banner line. We
// currently just display all control registers. A later version may print the registers a bit more formatted
// with respect to their content.
//
//------------------------------------------------------------------------------------------------------------
void DrvWinSpecialRegs::drawBody( ) {
    
    uint32_t fmtDesc = FMT_ALIGN_LFT;
    
    setWinCursor( 2, 1 );
    printTextField((char *) "CR0=  ", ( fmtDesc | FMT_BOLD ));
    
    for ( int i = 0; i < 4; i++ ) {
        
        printNumericField( glb -> cpu -> getReg( RC_CTRL_REG_SET, i ), fmtDesc );
        printTextField((char *) " " );
    }
   
    printTextField((char *) "CR4=  ", ( fmtDesc | FMT_BOLD ));
    
    for ( int i = 4; i < 8; i++ ) {
        
        printNumericField( glb -> cpu -> getReg( RC_CTRL_REG_SET, i ), fmtDesc );
        printTextField((char *) " " );
    }
        
    padLine( fmtDesc );
    
    setWinCursor( 3, 1 );
    printTextField((char *) "CR8=  ", ( fmtDesc | FMT_BOLD ));
    
    for ( int i = 8; i < 12; i++ ) {
        
        printNumericField( glb -> cpu -> getReg( RC_CTRL_REG_SET, i ), fmtDesc );
        printTextField((char *) " " );
    }
   
    printTextField((char *) "CR12= ", ( fmtDesc | FMT_BOLD ));
    
    for ( int i = 12; i < 16; i++ ) {
        
        printNumericField( glb -> cpu -> getReg( RC_CTRL_REG_SET, i ), fmtDesc );
        printTextField((char *) " " );
    }
    
    padLine( fmtDesc );
    
    setWinCursor( 4, 1 );
    printTextField((char *) "CR16= ", ( fmtDesc | FMT_BOLD ));
    
    for ( int i = 16; i < 20; i++ ) {
        
        printNumericField( glb -> cpu -> getReg( RC_CTRL_REG_SET, i ), fmtDesc );
        printTextField((char *) " " );
    }
    
    printTextField((char *) "CR20= ", ( fmtDesc | FMT_BOLD ));
    
    
    for ( int i = 20; i < 24; i++ ) {
        
        printNumericField( glb -> cpu -> getReg( RC_CTRL_REG_SET, i ), fmtDesc );
        printTextField((char *) " " );
    }
    
    padLine( fmtDesc );
    
    setWinCursor( 5, 1 );
    printTextField((char *) "CR24= ", ( fmtDesc | FMT_BOLD ));
    
    for ( int i = 24; i < 28; i++ ) {
        
        printNumericField( glb -> cpu -> getReg( RC_CTRL_REG_SET, i ), fmtDesc );
        printTextField((char *) " " );
    }
   
    printTextField((char *) "CR28= ", ( fmtDesc | FMT_BOLD ));
    
    for ( int i = 28; i < 32; i++ ) {
        
        printNumericField( glb -> cpu -> getReg( RC_CTRL_REG_SET, i ), fmtDesc );
        printTextField((char *) " " );
    }
    
    padLine( fmtDesc );
}

//***********************************************************************************************************
//***********************************************************************************************************
//
// Methods for the CPU24 pipeline register window class.
//
//***********************************************************************************************************
//***********************************************************************************************************

//------------------------------------------------------------------------------------------------------------
// Object constructor.
//
//------------------------------------------------------------------------------------------------------------
DrvWinPipeLineRegs::DrvWinPipeLineRegs( VCPU32Globals *glb ) : DrvWin( glb ) { }

//------------------------------------------------------------------------------------------------------------
// The default values are the initial settings when windows is brought up the first time, or for the WDEF
// command.
//
//------------------------------------------------------------------------------------------------------------
void DrvWinPipeLineRegs::setDefaults( ) {
    
    setRadix( glb -> env -> getEnvValTok( ENV_FMT_DEF ));
    
    setDefColumns( 84, TOK_HEX );
    setDefColumns( 106, TOK_OCT );
    setDefColumns( 84, TOK_DEC );
    setColumns( getDefColumns( getRadix( )));
    setRows( 4 );
    
    setWinType( WT_PL_WIN );
    setEnable( false );
}

//------------------------------------------------------------------------------------------------------------
// The window overrides the setRadix method to set the column width according to the radix choosen.
//
//------------------------------------------------------------------------------------------------------------
void DrvWinPipeLineRegs::setRadix( TokId rdx ) {
    
    DrvWin::setRadix( rdx );
    setColumns( getDefColumns( getRadix( )));
}

//------------------------------------------------------------------------------------------------------------
// The banner line for the pipeline register window. We show the cycle counter in the banner.
//
//------------------------------------------------------------------------------------------------------------
void DrvWinPipeLineRegs::drawBanner( ) {
    
    uint32_t fmtDesc = FMT_BOLD | FMT_INVERSE;
    
    setWinCursor( 1, 1 );
    printTextField((char *) "Pipeline", ( fmtDesc | FMT_ALIGN_LFT ), 16 );
    
    printTextField((char *) "ClockSteps: ", fmtDesc );
    printNumericField( glb -> cpu -> stats.clockCntr, fmtDesc );
    
    padLine( fmtDesc );
    printRadixField( fmtDesc | FMT_LAST_FIELD );
}

//------------------------------------------------------------------------------------------------------------
// The pipeline window shows the pipeline registers of the three stages.
//
//------------------------------------------------------------------------------------------------------------
void DrvWinPipeLineRegs::drawBody( ) {
    
    uint32_t fmtDesc = FMT_DEF_ATTR;
    
    setWinCursor( 2, 1 );
    
    if ( glb -> cpu -> getReg( RC_FD_PSTAGE, PSTAGE_REG_STALLED ) == 1 )
        
        printTextField((char *) "FD(s):", ( fmtDesc | FMT_ALIGN_LFT | FMT_BOLD ), 8 );
    else
        printTextField((char *) "FD:   ", ( fmtDesc | FMT_ALIGN_LFT | FMT_BOLD ), 8 );
    
    printTextField((char *) "PSW:",  ( fmtDesc | FMT_ALIGN_LFT ), 5 );
    printNumericField( getBitField( glb -> cpu -> getReg( RC_FD_PSTAGE, PSTAGE_REG_ID_PSW_0 ), 15, 16 ),
                      ( fmtDesc | FMT_HALF_WORD ));
    printTextField(( char *) ":", ( fmtDesc | FMT_ALIGN_LFT ));
    printNumericField( getBitField( glb -> cpu -> getReg( RC_FD_PSTAGE, PSTAGE_REG_ID_PSW_0 ), 31, 16 ),
                      ( fmtDesc | FMT_HALF_WORD ));
    printTextField(( char *) "." );
    printNumericField( glb -> cpu -> getReg( RC_FD_PSTAGE, PSTAGE_REG_ID_PSW_1 ) );
    padLine( fmtDesc );
    
    setWinCursor( 3, 1 );
    
    if ( glb -> cpu -> getReg( RC_MA_PSTAGE, PSTAGE_REG_STALLED ) == 1 )
        
        printTextField((char *) "MA(s):", ( fmtDesc | FMT_ALIGN_LFT | FMT_BOLD ), 8 );
    else
        printTextField((char *) "MA:   ", ( fmtDesc | FMT_ALIGN_LFT | FMT_BOLD ), 8 );
    
    printTextField((char *) "PSW:",  ( fmtDesc | FMT_ALIGN_LFT ), 5 );
    printNumericField( getBitField( glb -> cpu -> getReg( RC_MA_PSTAGE, PSTAGE_REG_ID_PSW_0 ), 15, 16 ),
                      ( fmtDesc | FMT_HALF_WORD ));
    printTextField(( char *) ":", ( fmtDesc | FMT_ALIGN_LFT ));
    printNumericField( getBitField( glb -> cpu -> getReg( RC_MA_PSTAGE, PSTAGE_REG_ID_PSW_0 ), 31, 16 ),
                      ( fmtDesc | FMT_HALF_WORD ));
    printTextField(( char *) "." );
    printNumericField( glb -> cpu -> getReg( RC_MA_PSTAGE, PSTAGE_REG_ID_PSW_1 ) );
    padLine( fmtDesc );
    
    printTextField(( char *) "  I: " );
    printNumericField( glb -> cpu -> getReg( RC_MA_PSTAGE, PSTAGE_REG_ID_INSTR ));
    printTextField(( char *) "  A: " );
    printNumericField( glb -> cpu -> getReg( RC_MA_PSTAGE, PSTAGE_REG_ID_VAL_A ));
    printTextField(( char *) "  B: " );
    printNumericField( glb -> cpu -> getReg( RC_MA_PSTAGE, PSTAGE_REG_ID_VAL_B ));
    printTextField(( char *) "  X: " );
    printNumericField( glb -> cpu -> getReg( RC_MA_PSTAGE, PSTAGE_REG_ID_VAL_X ));
    padLine( fmtDesc );
    
    setWinCursor( 4, 1 );
    
    if ( glb -> cpu -> getReg( RC_EX_PSTAGE, PSTAGE_REG_STALLED ) == 1 )
        printTextField((char *) "EX(s):", ( fmtDesc | FMT_ALIGN_LFT | FMT_BOLD ), 8 );
    else
        printTextField((char *) "EX:   ", ( fmtDesc | FMT_ALIGN_LFT | FMT_BOLD ), 8 );
    
    printTextField((char *) "PSW:",  ( fmtDesc | FMT_ALIGN_LFT ), 5 );
    printNumericField( getBitField( glb -> cpu -> getReg( RC_EX_PSTAGE, PSTAGE_REG_ID_PSW_0 ), 15, 16 ),
                      ( fmtDesc | FMT_HALF_WORD ));
    printTextField(( char *) ":", ( fmtDesc | FMT_ALIGN_LFT ));
    printNumericField( getBitField( glb -> cpu -> getReg( RC_EX_PSTAGE, PSTAGE_REG_ID_PSW_0 ), 31, 16 ),
                      ( fmtDesc | FMT_HALF_WORD ));
    printTextField(( char *) "." );
    printNumericField( glb -> cpu -> getReg( RC_EX_PSTAGE, PSTAGE_REG_ID_PSW_1 ) );
    padLine( fmtDesc );
    
    printTextField(( char *) "  I: " );
    printNumericField( glb -> cpu -> getReg( RC_EX_PSTAGE, PSTAGE_REG_ID_INSTR ));
    printTextField(( char *) "  A: " );
    printNumericField( glb -> cpu -> getReg( RC_EX_PSTAGE, PSTAGE_REG_ID_VAL_A ));
    printTextField(( char *) "  B: " );
    printNumericField( glb -> cpu -> getReg( RC_EX_PSTAGE, PSTAGE_REG_ID_VAL_B ));
    printTextField(( char *) "  X: " );
    printNumericField( glb -> cpu -> getReg( RC_EX_PSTAGE, PSTAGE_REG_ID_VAL_X ));
    padLine( fmtDesc );
}

//***********************************************************************************************************
//***********************************************************************************************************
//
// Methods for the CPU24 statistics window class.
//
//***********************************************************************************************************
//***********************************************************************************************************

//------------------------------------------------------------------------------------------------------------
// Object constructor.
//
//------------------------------------------------------------------------------------------------------------
DrvWinStatistics::DrvWinStatistics( VCPU32Globals *glb ) : DrvWin( glb ) { }

//------------------------------------------------------------------------------------------------------------
// The default values are the initial settings when windows is brought up the first time, or for the WDEF
// command.
//
//------------------------------------------------------------------------------------------------------------
void DrvWinStatistics::setDefaults( ) {
    
    setWinType( WT_ST_WIN );
    setEnable( false );
    setRows( 4 );
    setColumns( 84 );
    setDefColumns( 84 );
    setRadix( glb -> env -> getEnvValTok( ENV_FMT_DEF ));
}

//------------------------------------------------------------------------------------------------------------
// The banner line for the statistics window.
//
//------------------------------------------------------------------------------------------------------------
void DrvWinStatistics::drawBanner( ) {
    
    uint32_t fmtDesc = FMT_BOLD | FMT_INVERSE;
    
    setWinCursor( 1, 1 );
    printTextField((char *) "Statistics", ( fmtDesc | FMT_ALIGN_LFT ), 16) ;
    printTextField((char *) "ClockSteps: ", fmtDesc );
    printNumericField( glb -> cpu -> stats.clockCntr, fmtDesc );
    padLine( fmtDesc );
    printRadixField( fmtDesc | FMT_LAST_FIELD );
}

//------------------------------------------------------------------------------------------------------------
// The data window for the statistics.
//
// ??? work in progress...
//------------------------------------------------------------------------------------------------------------
void DrvWinStatistics::drawBody( ) {
    
    uint32_t fmtDesc = FMT_DEF_ATTR;
    
    setWinCursor( 2, 1 );
    
    printTextField((char *) "... ", fmtDesc | FMT_ALIGN_LFT, 32 );
    padLine( fmtDesc );
}

//***********************************************************************************************************
//***********************************************************************************************************
//
// Methods for the physical memory window class.
//
//***********************************************************************************************************
//***********************************************************************************************************

//------------------------------------------------------------------------------------------------------------
// Object constructor.
//
//------------------------------------------------------------------------------------------------------------
DrvWinAbsMem::DrvWinAbsMem( VCPU32Globals *glb ) : DrvWinScrollable( glb ) { }

//------------------------------------------------------------------------------------------------------------
// The default values are the initial settings when windows is brought up the first time, or for the WDEF
// command. The memory window is a window where the number of lines to display can be set. However, the
// minimum is the default number of lines.
//
//------------------------------------------------------------------------------------------------------------
void DrvWinAbsMem::setDefaults( ) {
    
    setRadix( glb -> env -> getEnvValTok( ENV_FMT_DEF ));
    
    setDefColumns( 12 + ( 8 * 11 ), TOK_HEX );
    setDefColumns( 14 + ( 8 * 13 ), TOK_OCT );
    setDefColumns( 12 + ( 8 * 11 ), TOK_DEC );
    setColumns( getDefColumns( getRadix( )));
    
    setWinType( WT_PM_WIN );
    setEnable( false );
    setRows( 5 );
    setHomeItemAdr( 0 );
    setCurrentItemAdr( 0 );
    setLineIncrement( 8 * 4 );
    setLimitItemAdr( 0 );
}

//------------------------------------------------------------------------------------------------------------
// The window overrides the setRadix method to set the column width according to the radix choosen.
//
//------------------------------------------------------------------------------------------------------------
void DrvWinAbsMem::setRadix( TokId rdx ) {
    
    DrvWin::setRadix( rdx );
    setColumns( getDefColumns( getRadix( )));
}

//------------------------------------------------------------------------------------------------------------
// The banner line shows the item adress, which is the current absolute physical memory adress where the
// window body will start tp display. We also need to set the item adress limit. This is however always the
// maximum value UINT_MAX, as absolute memory is up to 4Gbytes. The drawLine method will actually check that
// the offset passed is valid and invoke the correct absolute memory portion handler.
//
//------------------------------------------------------------------------------------------------------------
void DrvWinAbsMem::drawBanner( ) {
    
    uint32_t    fmtDesc     = FMT_BOLD | FMT_INVERSE;
    uint32_t    currentAdr  = getCurrentItemAdr( );
    CpuMem      *physMem    = glb -> cpu -> physMem;
    CpuMem      *pdcMem     = glb -> cpu -> pdcMem;
    CpuMem      *ioMem      = glb -> cpu -> ioMem;
    bool        isCurrent   = glb -> winDisplay -> isCurrentWin( getWinIndex( ));
  
    setWinCursor( 1, 1 );
    printWindowIdField( getWinStack( ), getWinIndex( ), isCurrent, fmtDesc );
    
    if (( physMem != nullptr ) && ( physMem -> validAdr( currentAdr ))) {
        
        printTextField((char *) "Main Memory ", ( fmtDesc | FMT_ALIGN_LFT ), 16 );
    }
    else if (( pdcMem != nullptr ) && ( pdcMem -> validAdr( currentAdr ))) {
        
        printTextField((char *) "PDC Memory ", ( fmtDesc | FMT_ALIGN_LFT ), 16 );
    }
    else if (( ioMem != nullptr ) && ( ioMem -> validAdr( currentAdr ))) {
        
        printTextField((char *) "IO Memory ", ( fmtDesc | FMT_ALIGN_LFT ), 16 );
    }
    else printTextField((char *) "**** Memory ", ( fmtDesc | FMT_ALIGN_LFT ), 16 );
    
    printTextField((char *) "Current " );
    printNumericField( getCurrentItemAdr( ));
    printTextField((char *) "  Home: " );
    printNumericField(  getHomeItemAdr( ));
    padLine( fmtDesc );
    printRadixField( fmtDesc | FMT_LAST_FIELD );
    
    setLimitItemAdr( UINT_MAX );
}

//------------------------------------------------------------------------------------------------------------
// A scrollable window needs to implement a routine for displaying a row. We are passed the item adress and
// need to map this to the actual meaning of the particular window. The "itemAdr" value is the byte offset
// into physical memory, the line incrment is 8 * 4 = 32 bytes. We show eight words.
//
//------------------------------------------------------------------------------------------------------------
void DrvWinAbsMem::drawLine( uint32_t itemAdr ) {
    
    uint32_t    fmtDesc     = FMT_DEF_ATTR;
    uint32_t    limit       = getLineIncrement( ) - 1;
    CpuMem      *physMem    = glb -> cpu -> physMem;
    CpuMem      *pdcMem     = glb -> cpu -> pdcMem;
    CpuMem      *ioMem      = glb -> cpu -> ioMem;
    
    printNumericField( itemAdr, fmtDesc );
    printTextField((char *) ": ", fmtDesc );
    
    for ( int i = 0; i < limit; i = i + 4 ) {
        
        uint32_t ofs = itemAdr + i;
        
        if (( physMem != nullptr ) && ( physMem -> validAdr( ofs ))) {
            
            printNumericField( physMem -> getMemDataWord( ofs ), fmtDesc );
        }
        else if (( pdcMem != nullptr ) && ( pdcMem -> validAdr( ofs ))) {
            
            printNumericField( pdcMem -> getMemDataWord( ofs ), fmtDesc );
        }
        else if (( ioMem != nullptr ) && ( ioMem -> validAdr( ofs ))) {
            
            printNumericField( ioMem -> getMemDataWord( ofs ), fmtDesc );
        }
        else printNumericField( 0, fmtDesc | FMT_INVALID_NUM );
        
        printTextField((char *) " " );
    }
}

//***********************************************************************************************************
//***********************************************************************************************************
//
// Methods for the code memory window class.
//
//***********************************************************************************************************
//***********************************************************************************************************

//------------------------------------------------------------------------------------------------------------
// Object constructor.
//
//------------------------------------------------------------------------------------------------------------
DrvWinCode::DrvWinCode( VCPU32Globals *glb ) : DrvWinScrollable( glb ) { }

//------------------------------------------------------------------------------------------------------------
// The default values are the initial settings when windows is brought up the first time, or for the WDEF
// command. The code memory window is a window where the number of lines to display can be set. However,
// the minimum is the default number of lines.
//
//------------------------------------------------------------------------------------------------------------
void DrvWinCode::setDefaults( ) {
    
    setRadix( glb -> env -> getEnvValTok( ENV_FMT_DEF ));
    setColumns( 84 );
    setDefColumns( 84 );
    setRows( 9 );

    setHomeItemAdr( 0 );
    setCurrentItemAdr( 0 );
    setLineIncrement( 4 );
    setLimitItemAdr( 0 );
    setWinType( WT_PC_WIN );
    setEnable( false );
}

//------------------------------------------------------------------------------------------------------------
// The banner for the code window shows the code address. We also need to set the item adress limit. As this
// can change with some commands outside the windows system, better set it every time. There is one more
// thing. It would be nice to automatically scroll the window for the single step command. We detect this
// by examining the current command and adjust the current item address to scroll to the next lines to show.
//
//------------------------------------------------------------------------------------------------------------
void DrvWinCode::drawBanner( ) {
    
    uint32_t    fmtDesc             = FMT_BOLD | FMT_INVERSE;
    int         currentItemAdr      = getCurrentItemAdr( );
    int         currentItemAdrLimit = currentItemAdr + (( getRows( ) - 1 ) * getLineIncrement( ));
    int         currentIaOfs        = (int) glb -> cpu -> getReg( RC_PROG_STATE, PS_REG_PSW_1  );
    TokId       currentCmd          = glb -> cmds -> getCurrentCmd( );
    bool        isCurrent           = glb -> winDisplay -> isCurrentWin( getWinIndex( ));
    uint32_t    blockEntries        = glb -> cpu -> physMem -> getBlockEntries( );
    uint32_t    blockSize           = glb -> cpu -> physMem -> getBlockSize( );
    bool        hasIaOfsAdr         = (( currentIaOfs >= currentItemAdr ) && ( currentIaOfs <= currentItemAdrLimit ));
    
    setLimitItemAdr(( blockEntries * blockSize ) - 1 );
   
    if (( currentCmd == CMD_STEP ) && ( hasIaOfsAdr )) {
        
        if      ( currentIaOfs >= currentItemAdrLimit ) winJump( currentIaOfs );
        else if ( currentIaOfs < currentItemAdr )       winJump( currentIaOfs );
    }
    
    setWinCursor( 1, 1 );
    printWindowIdField( getWinStack( ), getWinIndex( ), isCurrent, fmtDesc );
    printTextField((char *) "Code Memory ", ( fmtDesc | FMT_ALIGN_LFT ), 16 );
    printTextField((char *) "Current: " );
    printNumericField( getCurrentItemAdr( ));
    printTextField((char *) "  Home: " );
    printNumericField(  getHomeItemAdr( ));
    padLine( fmtDesc );
    printRadixField( fmtDesc | FMT_LAST_FIELD );
}

//------------------------------------------------------------------------------------------------------------
// A scrollable window needs to implement a routine for displaying a row. We are passed the item address and
// need to map this to the actual meaning of the particular window. The disassembled format is printed in
// two parts, the first is the instruction and options, the second is the target and operand field. We make
// sure that both parts are nicely aligned.
//
//------------------------------------------------------------------------------------------------------------
void DrvWinCode::drawLine( uint32_t itemAdr ) {
    
    uint32_t    instr           = glb -> cpu -> physMem -> getMemDataWord( itemAdr );
    uint32_t    fmtDesc         = FMT_DEF_ATTR;
    uint32_t    currentIaOfs    = glb -> cpu -> getReg( RC_PROG_STATE, PS_REG_PSW_1  );
    bool        isCurrentIaOfs  = ( itemAdr == currentIaOfs );
    
    printNumericField( itemAdr, fmtDesc | FMT_ALIGN_LFT, 12 );
    printTextField(( isCurrentIaOfs ? (char *) ">" :  (char *) " " ), fmtDesc, 4 );
    
    printNumericField( instr, fmtDesc | FMT_ALIGN_LFT, 12 );
    printTextField((char *) "", fmtDesc, 16 );
    
    int pos = getWinCursorCol( );
    padLine( );
    setWinCursor( 0, pos );
    glb -> disAsm -> displayOpCodeAndOptions( instr );
    setWinCursor( 0, pos + 8 );
    glb -> disAsm -> displayTargetAndOperands( instr, getRadix( ));
    padLine( );
}

//***********************************************************************************************************
//***********************************************************************************************************
//
// Methods for the TLB class.
//
//***********************************************************************************************************
//***********************************************************************************************************

//------------------------------------------------------------------------------------------------------------
// Object constructor. All we do is to remember what kind of TLB this is.
//
//------------------------------------------------------------------------------------------------------------
DrvWinTlb::DrvWinTlb( VCPU32Globals *glb, int winType ) : DrvWinScrollable( glb ) {
    
    this -> winType = winType;
}

//------------------------------------------------------------------------------------------------------------
// We have a function to set reasonable default values for the window. The default values are the intial
// settings when windows is brought up the first time, or for the WDEF command. The TLB window is a window
// where the number of lines to display can be set. However, the minimum is the default number of lines.
//
//------------------------------------------------------------------------------------------------------------
void DrvWinTlb::setDefaults( ) {
    
    setRadix( glb -> env -> getEnvValTok( ENV_FMT_DEF ));
    
    setDefColumns( 84, TOK_HEX );
    setDefColumns( 102, TOK_OCT );
    setDefColumns( 84, TOK_DEC );
    setColumns( getDefColumns( getRadix( )));
    
    setWinType( winType );
    setEnable( false );
    setRows( 5 );
    setCurrentItemAdr( 0 );
    setLineIncrement( 1 );
    setLimitItemAdr( 0 );
    
    if      ( winType == WT_ITLB_WIN ) tlb = glb -> cpu -> iTlb;
    else if ( winType == WT_DTLB_WIN ) tlb = glb -> cpu -> dTlb;
}

//------------------------------------------------------------------------------------------------------------
// The window overrides the setRadix method to set the column width according to the radix choosen.
//
//------------------------------------------------------------------------------------------------------------
void DrvWinTlb::setRadix( TokId rdx ) {
    
    DrvWin::setRadix( rdx );
    setColumns( getDefColumns( getRadix( )));
}

//------------------------------------------------------------------------------------------------------------
// Each window consist of a banner and a body. The banner line is always shown in inverse and contains
// summary or head data for the window. We also need to set the item adress limit. As this can change with
// some commands outside the windows system, better set it every time.
//
//------------------------------------------------------------------------------------------------------------
void DrvWinTlb::drawBanner( ) {
    
    uint32_t    fmtDesc     = FMT_BOLD | FMT_INVERSE;
    bool        isCurrent   = glb -> winDisplay -> isCurrentWin( getWinIndex( ));
    
    setWinCursor( 1, 1 );
    printWindowIdField( getWinStack( ), getWinIndex( ), isCurrent, fmtDesc );
    
    if      ( winType == WT_ITLB_WIN ) printTextField((char *) "I-TLB ", ( fmtDesc | FMT_ALIGN_LFT ), 16 );
    else if ( winType == WT_DTLB_WIN ) printTextField((char *) "D-TLB ", ( fmtDesc | FMT_ALIGN_LFT ), 16 );
    else                               printTextField((char *) "***** ", ( fmtDesc | FMT_ALIGN_LFT ), 16 );
    
    printTextField((char *) "Current: " );
    printNumericField( getCurrentItemAdr( ));
    printTextField((char *) "  Home: " );
    printNumericField(  getHomeItemAdr( ));
    padLine( fmtDesc );
    printRadixField( fmtDesc | FMT_LAST_FIELD );
    
    setLimitItemAdr( tlb -> getTlbSize( ));
}

//------------------------------------------------------------------------------------------------------------
// Each window consist of a banner and a body. The body lines are displayed after the banner line. The
// number of lines can vary. A line represents an entry in the respective TLB.
//
//------------------------------------------------------------------------------------------------------------
void DrvWinTlb::drawLine( uint32_t index ) {
    
    uint32_t  fmtDesc = FMT_DEF_ATTR;
    
    printNumericField( index, fmtDesc );
    printTextField((char *) ":[", fmtDesc );
    
    if ( index > tlb -> getTlbSize( )) {
  
        printTextField((char *) "Invalid TLB index", fmtDesc );
        printTextField((char *) "]", fmtDesc );
        padLine( );
    }
    else {
        
        TlbEntry   *tEntry  = tlb -> getTlbEntry( index );
        char            tmpBuf[ 32 ];
        
        printTextField((( tEntry -> tValid( )) ? (char *) "V" : (char *) "v" ), fmtDesc );
        printTextField((( tEntry -> tDirty( )) ? (char *) "D" : (char *) "d" ), fmtDesc );
        printTextField((( tEntry -> tTrapPage( )) ? (char *) "P" : (char *) "p" ), fmtDesc );
        printTextField((( tEntry -> tTrapDataPage( )) ? (char *) "D" : (char *) "d" ), fmtDesc );
        printTextField((char *) "]", fmtDesc );
        
        buildAccessRightsStr( tmpBuf, 32, tEntry ->tPageType( ), tEntry -> tPrivL1( ), tEntry -> tPrivL2( ));
        printTextField((char *) " ACC:", fmtDesc );
        printTextField( tmpBuf, fmtDesc );
        printTextField((char *) " PID:", fmtDesc );
        printNumericField( tEntry -> tSegId( ), fmtDesc| FMT_HALF_WORD );
        printTextField((char *) " VPN:", fmtDesc );
        printNumericField( tEntry -> vpnHigh, fmtDesc );
        printTextField((char *) ".", fmtDesc );
        printNumericField( tEntry -> vpnLow, fmtDesc );
        printTextField((char *) " PPN:", fmtDesc );
        printNumericField( tEntry -> tPhysPage( ), fmtDesc );
    }
}

//***********************************************************************************************************
//***********************************************************************************************************
//
// Methods for the Cache class.
//
//***********************************************************************************************************
//***********************************************************************************************************

//------------------------------------------------------------------------------------------------------------
// Object constructor. All we do is to remember what kind of Cache this is.
//
//------------------------------------------------------------------------------------------------------------
DrvWinCache::DrvWinCache( VCPU32Globals *glb, int winType ) : DrvWinScrollable( glb ) {
    
    this -> winType = winType;
}

//------------------------------------------------------------------------------------------------------------
// We have a function to set reasonable default values for the window. The default values are the intial
// settings when windows is brought up the first time, or for the WDEF command. The TLB window is a window
// where the number of lines to display can be set. However, the minimum is the default number of lines.
//
//------------------------------------------------------------------------------------------------------------
void DrvWinCache::setDefaults( ) {
    
    if      ( winType == WT_ICACHE_WIN )    cPtr = glb -> cpu -> iCacheL1;
    else if ( winType == WT_DCACHE_WIN )    cPtr = glb -> cpu -> dCacheL1;
    else if ( winType == WT_UCACHE_WIN )    cPtr = glb -> cpu -> uCacheL2;
    
    uint32_t wordsPerBlock = cPtr -> getBlockSize( ) / 4;
    
    setRadix( glb -> env -> getEnvValTok( ENV_FMT_DEF ));
    setDefColumns( 36 + ( wordsPerBlock * 11 ), TOK_HEX );
    setDefColumns( 36 + ( wordsPerBlock * 13 ), TOK_OCT );
    setDefColumns( 36 + ( wordsPerBlock * 11 ), TOK_DEC );
    setColumns( getDefColumns( getRadix( )));
    setRows( 6 );
    
    setWinType( winType );
    setEnable( false );
    setRadix( glb -> env -> getEnvValTok( ENV_FMT_DEF ));
    setCurrentItemAdr( 0 );
    setLineIncrement( 1 );
    setLimitItemAdr( 0 );
    winToggleVal = 0;
}

//------------------------------------------------------------------------------------------------------------
// The window overrides the setRadix method to set the column width according to the radix choosen.
//
//------------------------------------------------------------------------------------------------------------
void DrvWinCache::setRadix( TokId rdx ) {
    
    DrvWin::setRadix( rdx );
    setColumns( getDefColumns( getRadix( )));
}

//------------------------------------------------------------------------------------------------------------
// We allow for toggling through the sets if the cache is an n-way associative cache.
//
//------------------------------------------------------------------------------------------------------------
void DrvWinCache::toggleWin( ) {
    
    uint32_t blockSize   = cPtr -> getBlockSets( );
    winToggleVal = ( winToggleVal + 1 ) % blockSize;
}
    
//------------------------------------------------------------------------------------------------------------
// Each window consist of a headline and a body. The banner line is always shown in inverse and contains
// summary or head data for the window. We also need to set the item adress limit. As this can change with
// some commands outside the windows system, better set it every time.
//
//------------------------------------------------------------------------------------------------------------
void DrvWinCache::drawBanner( ) {
    
    uint32_t    fmtDesc     = FMT_BOLD | FMT_INVERSE;
    bool        isCurrent   = glb -> winDisplay -> isCurrentWin( getWinIndex( ));
    
    setWinCursor( 1, 1 );
    printWindowIdField( getWinStack( ), getWinIndex( ), isCurrent, fmtDesc );
    
    if      ( winType == WT_ICACHE_WIN ) printTextField((char *) "I-Cache (L1) ", ( fmtDesc | FMT_ALIGN_LFT ), 16 );
    else if ( winType == WT_DCACHE_WIN ) printTextField((char *) "D-Cache (L1)", ( fmtDesc | FMT_ALIGN_LFT ), 16 );
    else if ( winType == WT_UCACHE_WIN ) printTextField((char *) "U-Cache (L2)", ( fmtDesc | FMT_ALIGN_LFT ), 16 );
    else                                 printTextField((char *) "******* ", ( fmtDesc | FMT_ALIGN_LFT ), 16 );
  
    setLimitItemAdr( cPtr -> getBlockEntries( ));
    printTextField((char *) "Set: " );
    printNumericField( winToggleVal, ( fmtDesc | FMT_HALF_WORD ));
    printTextField((char *) " Current: " );
    printNumericField( getCurrentItemAdr( ));
    printTextField((char *) "  Home: " );
    printNumericField(  getHomeItemAdr( ));
    padLine( fmtDesc );
    printRadixField( fmtDesc | FMT_LAST_FIELD );
}

//------------------------------------------------------------------------------------------------------------
// The draw line methods for the cache lists a cache entry. There are various cache line sizes. And there
// are up to two sets of cache data.
//
//------------------------------------------------------------------------------------------------------------
void DrvWinCache::drawLine( uint32_t index ) {
    
    uint32_t  fmtDesc   = FMT_DEF_ATTR;
 
    if ( index > cPtr -> getBlockEntries( )) {
  
        printNumericField( index, fmtDesc );
        printTextField((char *) ":[", fmtDesc );
        printTextField((char *) "Invalid Cache index", fmtDesc );
        printTextField((char *) "]", fmtDesc );
        padLine( );
    }
    else {
        
        MemTagEntry *tagPtr         = cPtr -> getMemTagEntry( index, winToggleVal );
        uint32_t    *dataPtr        = (uint32_t *) cPtr -> getMemBlockEntry( index, winToggleVal );
        uint32_t    wordsPerBlock   = cPtr -> getBlockSize( ) / 4;
      
        printNumericField( index, fmtDesc );
        printTextField((char *) ":[", fmtDesc );
        printTextField((( tagPtr -> valid ) ? (char *) "V" : (char *) "v" ), fmtDesc );
        printTextField((( tagPtr -> dirty ) ? (char *) "D" : (char *) "d" ), fmtDesc );
        printTextField((char *) "] (", fmtDesc );
        printNumericField( tagPtr -> tag, fmtDesc );
        printTextField((char *) ") ", fmtDesc );
        
        for ( uint32_t i = 0; i < wordsPerBlock; i++ ) {
          
            printNumericField( dataPtr[ i ], fmtDesc );
            printTextField((char *) " " );
        }
    }
}

//***********************************************************************************************************
//***********************************************************************************************************
//
// Methods for the memory object register class.
//
//***********************************************************************************************************
//***********************************************************************************************************

//------------------------------------------------------------------------------------------------------------
// Object constructor. All we do is to remember what kind of memory object this is.
//
//------------------------------------------------------------------------------------------------------------
DrvWinMemController::DrvWinMemController( VCPU32Globals *glb, int winType ) : DrvWin( glb ) {
    
    this -> winType = winType;
}

//------------------------------------------------------------------------------------------------------------
// Set up reasonlable defaults and the reference to the actual memory object.
//
//------------------------------------------------------------------------------------------------------------
void DrvWinMemController::setDefaults( ) {
    
    if      ( winType == WT_ICACHE_S_WIN )  cPtr = glb -> cpu -> iCacheL1;
    else if ( winType == WT_DCACHE_S_WIN )  cPtr = glb -> cpu -> dCacheL1;
    else if ( winType == WT_UCACHE_S_WIN )  cPtr = glb -> cpu -> uCacheL2;
    else if ( winType == WT_MEM_S_WIN    )  cPtr = glb -> cpu -> physMem;
    
    setRadix( glb -> env -> getEnvValTok( ENV_FMT_DEF ));
    setDefColumns( 84, TOK_HEX );
    setDefColumns( 108, TOK_OCT );
    setDefColumns( 84, TOK_DEC );
    setColumns( getDefColumns( getRadix( )));
    
    setWinType( winType );
    setEnable( false );
    setRadix( glb -> env -> getEnvValTok( ENV_FMT_DEF ));
    setRows(( winType == WT_MEM_S_WIN ) ? 3 : 4 );
}

//------------------------------------------------------------------------------------------------------------
// Draw the memory object banner. We will display the static configuration data for the memory object.
//
//------------------------------------------------------------------------------------------------------------
void DrvWinMemController::drawBanner( ) {
    
    uint32_t    fmtDesc     = FMT_BOLD | FMT_INVERSE;
    bool        isCurrent   = glb -> winDisplay -> isCurrentWin( getWinIndex( ));
    
    setWinCursor( 1, 1 );
    printWindowIdField( getWinStack( ), getWinIndex( ), isCurrent, fmtDesc );
    
    if      ( winType == WT_ICACHE_S_WIN ) 
        printTextField((char *) "I-Cache (L1)", ( fmtDesc | FMT_ALIGN_LFT ), 16 );
    else if ( winType == WT_DCACHE_S_WIN )
        printTextField((char *) "D-Cache (L1)", ( fmtDesc | FMT_ALIGN_LFT ), 16 );
    else if ( winType == WT_UCACHE_S_WIN )
        printTextField((char *) "U-Cache (L2)", ( fmtDesc | FMT_ALIGN_LFT ), 16 );
    else if ( winType == WT_MEM_S_WIN )
        printTextField((char *) "MEM Reg Set", ( fmtDesc | FMT_ALIGN_LFT ), 16 );
    else if ( winType == WT_PDC_S_WIN )
        printTextField((char *) "PdcMEM Reg Set", ( fmtDesc | FMT_ALIGN_LFT ), 16 );
    else if ( winType == WT_IO_S_WIN )
        printTextField((char *) "IoMEM Reg Set", ( fmtDesc | FMT_ALIGN_LFT ), 16 );
    else
        printTextField((char *) "******* ", ( fmtDesc | FMT_ALIGN_LFT ), 16 );
    
    printTextField((char *) "Range: " );
    printNumericField( cPtr -> getMemCtrlReg( MC_REG_START_ADR ), fmtDesc );
    printTextField((char *) ":" );
    printNumericField( cPtr -> getMemCtrlReg( MC_REG_END_ADR ), fmtDesc );
    
    printTextField((char *) ", Blocks: " );
    printNumericField( cPtr -> getMemCtrlReg( MC_REG_BLOCK_ENTRIES ), fmtDesc );
    printTextField((char *) ":", fmtDesc );
    printNumericField( cPtr -> getMemCtrlReg( MC_REG_BLOCK_SIZE ), ( fmtDesc | FMT_HALF_WORD ));
    
    if (( winType != WT_MEM_S_WIN ) && ( winType != WT_PDC_S_WIN ) && ( winType != WT_IO_S_WIN )) {
        
        printTextField((char *) ", Sets: " );
        printNumericField( cPtr -> getMemCtrlReg( MC_REG_SETS ), ( fmtDesc | FMT_HALF_WORD ));
    }
    
    padLine( fmtDesc );
    printRadixField( fmtDesc | FMT_LAST_FIELD );
}

//------------------------------------------------------------------------------------------------------------
// Display the memory object machine state and the actual requests.
//
//------------------------------------------------------------------------------------------------------------
void DrvWinMemController::drawBody( ) {
    
    uint32_t fmtDesc = FMT_DEF_ATTR;
    
    setWinCursor( 2, 1 );
    
    printTextField((char *) "State:", ( fmtDesc | FMT_ALIGN_LFT ), 10 );
    printTextField( cPtr -> getMemOpStr( cPtr -> getMemCtrlReg( MC_REG_STATE )),
                   ( fmtDesc | FMT_ALIGN_LFT ), 20 );
    
    setWinCursor( 3, 1 );
    printTextField((char *) "Request:", ( fmtDesc | FMT_ALIGN_LFT ));
   
    if (( winType == WT_MEM_S_WIN ) || ( winType == WT_PDC_S_WIN ) || ( winType == WT_IO_S_WIN )) {
        
        setWinCursor( 3, 11 );
        printTextField((char *) "Adr:", ( fmtDesc | FMT_ALIGN_LFT ));
        printNumericField( cPtr -> getMemCtrlReg( MC_REG_REQ_OFS ));
        printTextField((char *) "  Len: ", fmtDesc );
        printNumericField( cPtr -> getMemCtrlReg( MC_REG_REQ_LEN ), ( fmtDesc | FMT_HALF_WORD ));
        printTextField((char * ) "  Pri: ", ( fmtDesc | FMT_ALIGN_LFT | FMT_HALF_WORD ));
        printNumericField( cPtr -> getMemCtrlReg( MC_REG_REQ_PRI ));
        printTextField((char * ) "  Lat: ", ( fmtDesc | FMT_ALIGN_LFT | FMT_HALF_WORD ));
        printNumericField( cPtr -> getMemCtrlReg( MC_REG_REQ_LATENCY ));
    }
    else {
        
        setWinCursor( 3, 11 );
        printTextField((char *) "Seg:ofs:", ( fmtDesc | FMT_ALIGN_LFT ));
        printNumericField( cPtr -> getMemCtrlReg( MC_REG_REQ_SEG ));
        printTextField((char *) ":", fmtDesc );
        printNumericField( cPtr -> getMemCtrlReg( MC_REG_REQ_OFS ));
        printTextField((char *) "   Tag: ", fmtDesc );
        printNumericField( cPtr -> getMemCtrlReg( MC_REG_REQ_TAG ));
        printTextField((char *) "  Len: ", fmtDesc );
        printNumericField( cPtr -> getMemCtrlReg( MC_REG_REQ_LEN ), ( fmtDesc | FMT_HALF_WORD ));
        
        setWinCursor( 4, 11 );
        printTextField((char * ) "Pri: ", ( fmtDesc | FMT_ALIGN_LFT | FMT_HALF_WORD ));
        printNumericField( cPtr -> getMemCtrlReg( MC_REG_REQ_PRI ));
        printTextField((char * ) "  Lat: ", ( fmtDesc | FMT_ALIGN_LFT | FMT_HALF_WORD ));
        printNumericField( cPtr -> getMemCtrlReg( MC_REG_REQ_LATENCY ));
        printTextField((char * ) "  tSet: ", ( fmtDesc | FMT_ALIGN_LFT | FMT_HALF_WORD ));
        printNumericField( cPtr -> getMemCtrlReg( MC_REG_REQ_BLOCK_SET ));
        printTextField((char * ) "  tBlock: ", ( fmtDesc | FMT_ALIGN_LFT ));
        printNumericField( cPtr -> getMemCtrlReg( MC_REG_REQ_BLOCK_INDEX ));
    }
}

//***********************************************************************************************************
//***********************************************************************************************************
//
// Methods for the text window class.
//
//***********************************************************************************************************
//***********************************************************************************************************

//------------------------------------------------------------------------------------------------------------
// Object constructor. We are passed the globals and the file path. All we do right now is to remember the
// file name. The text window has a destructor method as well. We need to close a potentially opened file.
//
//------------------------------------------------------------------------------------------------------------
DrvWinText::DrvWinText( VCPU32Globals *glb, char *fName ) : DrvWinScrollable( glb ) {
    
    strcpy( fileName, fName );
}

DrvWinText:: ~DrvWinText( ) {
    
    if ( textFile != nullptr ) {
        
        fclose( textFile );
    }
}

//------------------------------------------------------------------------------------------------------------
// The default values are the initial settings when windows is brought up the first time, or for the WDEF
// command.
//
//------------------------------------------------------------------------------------------------------------
void DrvWinText::setDefaults( ) {
    
    setWinType( WT_TEXT_WIN );
    setEnable( true );
    setRows( 11 );
    setColumns( glb -> env -> getEnvValInt( ENV_WIN_TX_WIDTH ));
    setDefColumns( glb -> env -> getEnvValInt( ENV_WIN_TX_WIDTH ));
    setRadix( TOK_DEC );
    setCurrentItemAdr( 0 );
    setLineIncrement( 1 );
    setLimitItemAdr( 1 );
}

//------------------------------------------------------------------------------------------------------------
// The banner line for the text window. It contains the open file name and the current line and home line
// number. The file path may be a bit long for listing it completely, so we will truncate it on the left
// side. The routine will print the the filename, and the position into the file. The banner method also sets
// the preliminary line size of the window. This value is used until we know the actual number of lines in
// the file. Lines shown on teh display start with one, internally we start at zero.
//
//------------------------------------------------------------------------------------------------------------
void DrvWinText::drawBanner( ) {
    
    uint32_t    fmtDesc     = FMT_BOLD | FMT_INVERSE;
    bool        isCurrent   = glb -> winDisplay -> isCurrentWin( getWinIndex( ));
    
    setWinCursor( 1, 1 );
    printWindowIdField( getWinStack( ), getWinIndex( ), isCurrent, fmtDesc );
    printTextField((char *) "Text: ", ( fmtDesc | FMT_ALIGN_LFT ));
    printTextField((char *) fileName, ( fmtDesc | FMT_ALIGN_LFT | FMT_TRUNC_LFT ), 48 );
    printTextField((char *) "  Line: " );
    printNumericField( getCurrentItemAdr( ) + 1, ( fmtDesc | FMT_HALF_WORD ));
    printTextField((char *) "  Home: " );
    printNumericField(  getHomeItemAdr( ) + 1, ( fmtDesc | FMT_HALF_WORD ));
    padLine( fmtDesc );
}

//------------------------------------------------------------------------------------------------------------
// The draw line method for the text file window. We print the file content line by line. A line consists of
// the line number followed by the text. This routine will first check whether the file is already open.
// If we cannot open the file, we would now print an error message into the screen. This is also the time
// where we actually figure out hopw many lines are on the file so that we can set the limitItemAdr field
// of the window object.
//
//------------------------------------------------------------------------------------------------------------
void DrvWinText::drawLine( uint32_t index ) {
    
    uint32_t    fmtDesc = FMT_DEF_ATTR;
    char        lineBuf[ MAX_TEXT_LINE_SIZE ];
    int         lineSize = 0;
  
    if ( openTextFile( )) {
  
        lineSize = readTextFileLine( index + 1, lineBuf, sizeof( lineBuf ));
        if ( lineSize > 0 ) {
            
            printNumericField( index + 1, ( fmtDesc | FMT_HALF_WORD ));
            printTextField((char *) ": " );
            printTextField( lineBuf, fmtDesc, lineSize );
            padLine( );
        }
        else padLine( );
    }
    else printTextField((char *) "Error opening the text file", fmtDesc );
}

//------------------------------------------------------------------------------------------------------------
// "openTextFile" is called every time we want to print a line. If the file is not opened yet, it will be
// now and while we are at it, we will also count the source lines for setting the limit in the scrollable
// window. All will be remembered of course.
//
//------------------------------------------------------------------------------------------------------------
bool DrvWinText::openTextFile( ) {
    
    if ( textFile == nullptr ) {
        
        textFile = fopen( fileName, "r" );
        if ( textFile != nullptr ) {
           
            while( ! feof( textFile )) {
                
                if( fgetc( textFile ) == '\n' ) fileSizeLines++;
            }
            
            lastLinePos = 0;
            rewind( textFile );
            setLimitItemAdr( fileSizeLines );
        }
    }
    
    return( textFile != nullptr );
}

//------------------------------------------------------------------------------------------------------------
// "readTextFileLine" will get a line from the text file. Unfortunately, we do not have a line concept in a
// text file. In the worst case, we read from the beginning of the file, counting the lines read. To speed
// up a little, we remember the last line position read. If the requested line position is larger than the
// last position, we just read ahead. If it is smaller, no luck, we start to read from zero until we match.
// If equal, we just re-read the current line.
//
//------------------------------------------------------------------------------------------------------------
int DrvWinText::readTextFileLine( int linePos, char *lineBuf, int bufLen  ) {
 
    if ( textFile != nullptr ) {
        
        if ( linePos > lastLinePos ) {
            
            while ( lastLinePos < linePos ) {
                
                lastLinePos ++;
                fgets( lineBuf, bufLen, textFile );
            }
        }
        else if ( linePos < lastLinePos ) {
            
            lastLinePos = 0;
            rewind( textFile );
            
            while ( lastLinePos < linePos ) {
                
                lastLinePos ++;
                fgets( lineBuf, bufLen, textFile );
            }
        }
        else fgets( lineBuf, bufLen, textFile );
            
        lineBuf[ strcspn( lineBuf, "\r\n") ] = 0;
        return((int) strlen ( lineBuf ));
    }
    else return ( 0 );
}

//***********************************************************************************************************
//***********************************************************************************************************
//
// Methods for the command window class.
//
//***********************************************************************************************************
//***********************************************************************************************************

//------------------------------------------------------------------------------------------------------------
// Object constructor.
//
//------------------------------------------------------------------------------------------------------------
DrvWinCommands::DrvWinCommands( VCPU32Globals *glb ) : DrvWin( glb ) { }

//------------------------------------------------------------------------------------------------------------
// The default values are the intial settings when windows is brought up the first time, or for the WDEF
// command.
//
//------------------------------------------------------------------------------------------------------------
void DrvWinCommands::setDefaults( ) {
    
    setWinType( WT_CMD_WIN );
    setEnable( true );
    setRows( 11 );
    setColumns( 80 );
    setDefColumns( 80 );
    setRadix( glb -> env -> getEnvValTok( ENV_FMT_DEF ));
}

//------------------------------------------------------------------------------------------------------------
// The banner line for command window.
//
//------------------------------------------------------------------------------------------------------------
void DrvWinCommands::drawBanner( ) {
    
    uint32_t fmtDesc = FMT_BOLD | FMT_INVERSE;
    
    setWinCursor( 1, 1 );
    printTextField((char *) "Commands ", fmtDesc );
    padLine( fmtDesc );
}

//------------------------------------------------------------------------------------------------------------
// The body lines of the command window are displayed after the banner line. We will never draw in this
// window via the window routines. The body is teh terminal scroll area. What we do however, is to reset
// any character drawing attribute.
//
//------------------------------------------------------------------------------------------------------------
void DrvWinCommands::drawBody( ) {
    
    setFieldAtributes( FMT_DEF_ATTR );
}

//***********************************************************************************************************
//***********************************************************************************************************
//
// Methods for the terminal window display class.
//
//***********************************************************************************************************
//***********************************************************************************************************

//-----------------------------------------------------------------------------------------------------------
// Object constructur. We initialize the windows list and create all the predefined windows. The remainder
// of the window list is used by the user defined windows.
//
//-----------------------------------------------------------------------------------------------------------
DrvWinDisplay::DrvWinDisplay( VCPU32Globals *glb ) {
    
    this -> glb = glb;
    
    for ( int i = 0; i < MAX_WINDOWS; i++ ) windowList[ i ] = nullptr;
    
    windowList[ PS_REG_WIN ]    = new DrvWinProgState( glb );
    windowList[ CTRL_REG_WIN ]  = new DrvWinSpecialRegs( glb );
    windowList[ PL_REG_WIN ]    = new DrvWinPipeLineRegs( glb );
    windowList[ STATS_WIN ]     = new DrvWinStatistics( glb );
    cmdWin                      = new DrvWinCommands( glb );
}

//-----------------------------------------------------------------------------------------------------------
// The current window number defines which user window is marked "current" and commands that omit the window
// number in their command will use this number. There is a routrine to check that we have a valid window
// number, which includes fixed and user numbers.
//
//-----------------------------------------------------------------------------------------------------------
int  DrvWinDisplay::getCurrentUserWindow( ) {
    
    return( currentUserWinNum );
}

void DrvWinDisplay::setCurrentUserWindow( int winNum ) {
    
    currentUserWinNum = winNum;
}

//-----------------------------------------------------------------------------------------------------------
// A window number is the index into the window list. It is valid when the numberis of course within bounds
// and the window list entry is actually used. A valid user window number additionally tests that the number
// is within the list portion reserved for user defined windows.
//
//-----------------------------------------------------------------------------------------------------------
bool DrvWinDisplay::validWindowNum( int winNum ) {
    
    return(( winNum <= LAST_UWIN ) && ( windowList[ winNum ] != nullptr ));
}

bool DrvWinDisplay::validUserWindowNum( int winNum ) {
    
    return(( winNum >= FIRST_UWIN ) && ( winNum <= LAST_UWIN ) && ( windowList[ winNum ] != nullptr ));
}

bool DrvWinDisplay::validWindowStackNum( int stackNum ) {
    
    return(( stackNum >= 0 ) && ( stackNum < MAX_WIN_STACKS ));
}

bool DrvWinDisplay::validUserWindowType( TokId winType ) {
    
    return(( winType == TOK_PM )    || ( winType == TOK_PC )    || ( winType == TOK_IT ) ||
           ( winType == TOK_DT )    || ( winType == TOK_IC )    || ( winType == TOK_DC ) ||
           ( winType == TOK_UC )    || ( winType == TOK_ICR )   || ( winType == TOK_DCR ) ||
           ( winType == TOK_UCR )   || ( winType == TOK_MCR )   || ( winType == TOK_TX ));
}

bool DrvWinDisplay::isCurrentWin( int winNum ) {
    
    return(( validUserWindowNum( winNum ) && ( currentUserWinNum == winNum )));
}

//-----------------------------------------------------------------------------------------------------------
// Before drawing the screen content after the execution of a command line, we need to check whether the
// number of columns needed for a stack of windows has changed. This function just runs through the window
// list for a given stack and determines the widest column needed for that stack.
//
//-----------------------------------------------------------------------------------------------------------
int DrvWinDisplay::computeColumnsNeeded( int winStack ) {
    
    int columnSize = 0;
    
    for ( int i = 0; i < MAX_WINDOWS; i++ ) {
        
        if (( windowList[ i ] != nullptr ) &&
            ( windowList[ i ] -> isEnabled( )) &&
            ( windowList[ i ] -> getWinStack( ) == winStack )) {
            
            int columns = windowList[ i ] -> getDefColumns( windowList[ i ] -> getRadix( ));
            if ( columns > columnSize ) columnSize = columns;
        }
    }
    
    return( columnSize );
}

//-----------------------------------------------------------------------------------------------------------
// Once we know the maximum column size needed for the active windows in a stack, we need to set this
// size in all those windows, so that they print nicely with a common end of line picture.
//
//-----------------------------------------------------------------------------------------------------------
void DrvWinDisplay::setWindowColumns( int winStack, int columnSize ) {
    
    for ( int i = 0; i < MAX_WINDOWS; i++ ) {
        
        if (( windowList[ i ] != nullptr ) &&
            ( windowList[ i ] -> isEnabled( )) &&
            ( windowList[ i ] -> getWinStack( ) == winStack )) {
            
            windowList[ i ] -> setColumns( columnSize );
        }
    }
}

//-----------------------------------------------------------------------------------------------------------
// Before drawing the screen content after the execution of a command line, we need to check whether the
// number of rows needed for a stack of windows has changed. This function just runs through the window list
// and sums up the rows needed for a given stack.
//
//-----------------------------------------------------------------------------------------------------------
int DrvWinDisplay::computeRowsNeeded( int winStack ) {
    
    int rowSize = 0;
    
    for ( int i = 0; i < MAX_WINDOWS; i++ ) {
        
        if (( windowList[ i ] != nullptr ) &&
            ( windowList[ i ] -> isEnabled( )) &&
            ( windowList[ i ] -> getWinStack( ) == winStack )) {
            
            rowSize += windowList[ i ] -> getRows( );
        }
    }
    
    return( rowSize );
}

//-----------------------------------------------------------------------------------------------------------
// Content for each window is addressed in a window relative way. For this scheme to work, each window needs
// to know the abslute position within in the overall screen. This routine will compute for each window of
// the passed stack the absolute row and column position for the window in the terminal screen.
//
//-----------------------------------------------------------------------------------------------------------
void DrvWinDisplay::setWindowOrigins( int winStack, int rowOffset, int colOffset ) {
    
    int tmpRow = rowOffset;
    
    for ( int i = 0; i < MAX_WINDOWS; i++ ) {
        
        if (( windowList[ i ] != nullptr ) &&
            ( windowList[ i ] -> isEnabled( )) &&
            ( windowList[ i ] -> getWinStack( ) == winStack )) {
            
            windowList[ i ] -> setWinOrigin( tmpRow, colOffset );
            tmpRow += windowList[ i ] -> getRows( );
        }
        
        cmdWin -> setWinOrigin( tmpRow, colOffset );
    }
}

//-----------------------------------------------------------------------------------------------------------
// Window screen drawing. Each time we read in a command input and are in windows mode, the terminal screen
// is redrawn. A terminal screen consist of a list of stacks and in each stack a list of windows. There is
// always the main stack, stack Id 0. Only if we have user defined windows assigned to another stack and
// window stacks are enbled, will this stack show up in the terminal screen. If window stacks are disabled,
// all windows, regardless what their stack ID says, will show up in the main stack.
//
// We first compute the number of rows and columns needed for the windows to show in their assigned stack.
// Only enabled screens will participate in the overall screen size computation. The data is used then to
// set the window columns of a window in the respective stack to the computed columns size and to set the
// absolute origin coordinates of each window. Again, this depends whether window stacks er enabled. If the
// number of rows needed for the windows and command window is less than the defined minimum number of rows,
// the command window is enlarged to have a screen of minumum row size. When the screen size changed, we
// just redraw the screen with the command screen going last. The command screen will have a columns size
// across all visibloe stacks.
//
//-----------------------------------------------------------------------------------------------------------
void DrvWinDisplay::reDraw( bool mustRedraw ) {
    
    int winStackColumns[ MAX_WIN_STACKS ]   = { 0 };
    int winStackRows[ MAX_WIN_STACKS ]      = { 0 };
    int defRowSize                          = glb -> env -> getEnvValInt( ENV_WIN_MIN_ROWS );
    int maxRowsNeeded                       = 0;
    int maxColumnsNeeded                    = 0;
    int stackColumnGap                      = 2;
    
    for ( int i = 0; i < MAX_WIN_STACKS; i++ ) {
        
        winStackColumns[ i ] = computeColumnsNeeded( i );
        winStackRows[ i ]    = computeRowsNeeded( i );
        
        if ( winStacksOn ) {
            
            if ( winStackColumns[ i ] > 0 ) maxColumnsNeeded += winStackColumns[ i ] + stackColumnGap;
            if ( winStackRows[ i ] > maxRowsNeeded ) maxRowsNeeded = winStackRows[ i ];
        }
        else {
            
            if ( winStackColumns[ i ] > maxColumnsNeeded ) maxColumnsNeeded = winStackColumns[ i ];
            maxRowsNeeded += winStackRows[ i ];
        }
    }
    
    int curColumn = 1;
    int curRows   = 1;
    
    for ( int i = 0; i < MAX_WIN_STACKS; i++ ) {
        
        setWindowColumns( i, winStackColumns[ i ] );
        setWindowOrigins( i, curRows, curColumn );
        
        if ( winStacksOn ) {
            
            curColumn += winStackColumns[ i ];
            if ( winStackColumns[ i ] > 0 ) curColumn += stackColumnGap;
        }
        else curRows += winStackRows[ i ];
    }
    
    if (( maxRowsNeeded + cmdWin -> getRows( )) < defRowSize ) {
        
        cmdWin -> setRows( defRowSize - maxRowsNeeded );
        maxRowsNeeded += cmdWin -> getRows();
    }
    else maxRowsNeeded += cmdWin -> getRows( );
    
    if ( winStacksOn )  cmdWin -> setColumns( maxColumnsNeeded - stackColumnGap );
    else                cmdWin -> setColumns( maxColumnsNeeded );
    
    cmdWin -> setWinOrigin( maxRowsNeeded - cmdWin -> getRows( ) + 1, 1 );
  
    if ( mustRedraw ) {
        
        actualRowSize      = maxRowsNeeded;
        actualColumnSize   = maxColumnsNeeded;
        
        setWindowSize( actualRowSize, actualColumnSize );
        setAbsCursor( 1, 1 );
        clearScrollArea( );
        clearScreen( );
        
        setScrollArea( actualRowSize - cmdWin -> getRows( ) + 2, actualRowSize );
    }
    
    for ( int i = 0; i < MAX_WINDOWS; i++ ) {
        
        if (( windowList[ i ] != nullptr ) && ( windowList[ i ] -> isEnabled( ))) {
            
            windowList[ i ] -> reDraw( );
        }
    }
    
    cmdWin -> reDraw( );
    setAbsCursor( actualRowSize, 1 );
}

//-----------------------------------------------------------------------------------------------------------
// The entry point to showing windows is to draw the screen on the "windows on" command and to clean up when
// we switch back to line mode. The window defaults method will set the windows to a preconfigured default
// value. This is quite useful when we messed up our screens. Also, if the screen is displayed garbled after
// some windows mouse based screen window changes, just do WON again to set it straight. There is also
// a function to enable or disable the window stacks feature.
//
//-----------------------------------------------------------------------------------------------------------
void DrvWinDisplay::windowsOn( ) {
    
    // nothing to do here...
}

void DrvWinDisplay::windowsOff( ) {
    
    clearScrollArea( );
    clearScreen( );
}

void DrvWinDisplay::windowDefaults( ) {
    
    for ( int i = 0; i < MAX_WINDOWS; i++ ) {
        
        if ( windowList[ i ] != nullptr ) windowList[ i ] -> setDefaults( );
    }
    
    cmdWin -> setDefaults( );
}

void DrvWinDisplay::winStacksEnable( bool arg ) {
    
    winStacksOn = arg;
}

//-----------------------------------------------------------------------------------------------------------
// A user defined window can be set to be the current user window. Commands that allow to specify a window
// number will use the window set by default then. Note that each user defined command that specifies the
// window number in its command will also set the current value. The user window becomes the actual window.
//
//-----------------------------------------------------------------------------------------------------------
void DrvWinDisplay::windowCurrent( TokId winCmd, int winNum ) {
    
    if ( validUserWindowNum( winNum )) currentUserWinNum = winNum;
}

//-----------------------------------------------------------------------------------------------------------
// The routine sets the stack attribute for a user window. The setting is not allowed for the predefined
// window. They are always in the main window stack, which has the stack Id of zero. Theorethically we
// could have many stacks, numbered 0 to MAX_STACKS - 1. Realistically, 3 to 4 stacks will fit on a screen.
//
//-----------------------------------------------------------------------------------------------------------
void DrvWinDisplay::windowSetStack( int winNum, int winStack ) {
    
    if ( winNum == 0 ) winNum = getCurrentUserWindow( );
    
    if ( validUserWindowNum( winNum )) {
        
        if ( winStack < MAX_WIN_STACKS ) {
            
            windowList[ winNum ] -> setWinStack( winStack );
            setCurrentUserWindow( winNum );
        }
    }
}

//-----------------------------------------------------------------------------------------------------------
// A window can be added or removed for the window list shown. We are passed an optional windows number,
// which is used when there are user defined windows for locating the window object.
//
//-----------------------------------------------------------------------------------------------------------
void DrvWinDisplay::windowEnable( TokId winCmd, int winNum ) {
    
    switch( winCmd ) {
            
        case CMD_PSE: windowList[ PS_REG_WIN ] -> setEnable( true );    break;
        case CMD_SRE: windowList[ CTRL_REG_WIN ] -> setEnable( true );  break;
        case CMD_PLE: windowList[ PL_REG_WIN ] -> setEnable( true );    break;
        case CMD_SWE: windowList[ STATS_WIN ] -> setEnable( true );     break;
       
        case CMD_WE: {
            
            if ( winNum == 0 ) winNum = getCurrentUserWindow( );
            
            if ( validUserWindowNum( winNum )) {
                
                windowList[ winNum ] -> setEnable( true );
                setCurrentUserWindow( winNum );
            }
            
        } break;
            
        default: ;
    }
}

//-----------------------------------------------------------------------------------------------------------
// A window can be added or removed for the window list shown. We are passed an optional windows number,
// which is used when there are user defined windows for locating the window object.
//
//-----------------------------------------------------------------------------------------------------------
void DrvWinDisplay::windowDisable( TokId winCmd, int winNum ) {
    
    switch( winCmd ) {
            
        case CMD_PSD: windowList[ PS_REG_WIN ] -> setEnable( false );   break;
        case CMD_SRD: windowList[ CTRL_REG_WIN ] -> setEnable( false ); break;
        case CMD_PLD: windowList[ PL_REG_WIN ] -> setEnable( false );   break;
        case CMD_SWD: windowList[ STATS_WIN ] -> setEnable( false );    break;
     
        case CMD_WD: {
            
            if ( winNum == 0 ) winNum = getCurrentUserWindow( );
            
            if ( validUserWindowNum( winNum )) {
                
                windowList[ winNum ] -> setEnable( false );
                setCurrentUserWindow( winNum );
            }
            
        } break;
            
        default: ;
    }
}

//-----------------------------------------------------------------------------------------------------------
// For the numeric values in a window, we can set the radix. The token ID for the format option is mapped to
// the actual radix value.We are passed an optional windows number, which is used when there are user defined
// windows for locating the window object. Changing the radix potetntially means tha the window layout needs
// to change.
//
// ??? force redraw here ? All windows need to provide the "setRadix" if they change the column width.
//-----------------------------------------------------------------------------------------------------------
void DrvWinDisplay::windowRadix( TokId winCmd, TokId fmtId, int winNum ) {
    
    switch( winCmd ) {
            
        case CMD_PSR: windowList[ PS_REG_WIN ] -> setRadix( fmtId );    break;
        case CMD_SRR: windowList[ CTRL_REG_WIN ] -> setRadix( fmtId );  break;
        case CMD_PLR: windowList[ PL_REG_WIN ] -> setRadix( fmtId );    break;
        case CMD_SWR: windowList[ STATS_WIN ] -> setRadix( fmtId );     break;
     
        case CMD_WR: {
            
            if ( winNum == 0 ) winNum = getCurrentUserWindow( );
            
            if ( validUserWindowNum( winNum )) {
                
                windowList[ winNum ] -> setRadix( fmtId );
                setCurrentUserWindow( winNum );
            }
           
        } break;
            
        default: ;
    }
    
    reDraw( true );
}

//-----------------------------------------------------------------------------------------------------------
// "setRows" is the method to set the nmber if lines in a window. The number includes the banner. We are
// passed an optional windows number, which is used when there are user defined windows for locating the
// window object.
//
//-----------------------------------------------------------------------------------------------------------
void DrvWinDisplay::windowSetRows( TokId winCmd, int rows, int winNum ) {
    
    switch ( winCmd ) {
            
        case CMD_CWL: cmdWin -> setRows( rows ); break;
            
        case CMD_WL: {
            
            if ( winNum == 0 ) winNum = getCurrentUserWindow( );
            
            if ( validUserWindowNum( winNum )) {
                
                windowList[ winNum ] -> setRows( rows );
                setCurrentUserWindow( winNum );
            }
            
        } break;
            
        default: ;
    }
}

//-----------------------------------------------------------------------------------------------------------
// "winHome" will set the current position to the home index, i.e. the position with which the window was
// creared. If the position passed is non-zero, it will become the new home position. The position meaning
// is window dependent and the actual window will sort it out. We are passed an optional windows number,
// which is used when there are user defined windows for locating the window object.
//
//-----------------------------------------------------------------------------------------------------------
void DrvWinDisplay::windowHome( TokId winCmd, int pos, int winNum ) {
    
    if ( winNum == 0 ) winNum = getCurrentUserWindow( );
    
    if ( validUserWindowNum( winNum )) {
        
        ((DrvWinScrollable *) windowList[ winNum ] ) -> winHome( pos );
        setCurrentUserWindow( winNum );
    }
}

//-----------------------------------------------------------------------------------------------------------
// A window is scrolled forward with the "windowForward" method. The meaning of the amount is window
// dependent and the actual window will sort it out. We are passed an optional windows number, which is used
// when there are user defined windows for locating the window object.
//
//-----------------------------------------------------------------------------------------------------------
void DrvWinDisplay::windowForward( TokId winCmd, int amt, int winNum ) {
    
    if ( winNum == 0 ) winNum = getCurrentUserWindow( );
    
    if ( validUserWindowNum( winNum )) {
        
        ((DrvWinScrollable *) windowList[ winNum ] ) -> winForward( amt );
        setCurrentUserWindow( winNum );
    }
}

//-----------------------------------------------------------------------------------------------------------
// A window is scrolled backward with the "windowBackward" methods. The meaning of the amount is window
// dependent and the actual window will sort it out. We are passed an optional windows number, which is used
// when there are user defined windows for locating the window object.
//
//-----------------------------------------------------------------------------------------------------------
void DrvWinDisplay::windowBackward( TokId winCmd, int amt, int winNum ) {
    
    if ( winNum == 0 ) winNum = getCurrentUserWindow( );
    
    if ( validUserWindowNum( winNum )) {
        
        ((DrvWinScrollable *) windowList[ winNum ] ) -> winBackward( amt );
        setCurrentUserWindow( winNum );
    }
}

//-----------------------------------------------------------------------------------------------------------
// The current index can also directly be set to another location. The position meaning is window dependent
// and the actual window will sort it out. We are passed an optional windows number, which is used when
// there are user defined windows for locating the window object.
//
//-----------------------------------------------------------------------------------------------------------
void DrvWinDisplay::windowJump( TokId winCmd, int pos, int winNum ) {
    
    if ( winNum == 0 ) winNum = getCurrentUserWindow( );
    
    if ( validUserWindowNum( winNum )) {
        
        if ( winNum == 0 ) winNum = getCurrentUserWindow( );
        
        ((DrvWinScrollable *) windowList[ winNum ] ) -> winJump( pos );
        setCurrentUserWindow( winNum );
    }
}

//-----------------------------------------------------------------------------------------------------------
// The current index can also directly be set to another location. The position meaning is window dependent
// and the actual window will sort it out. We are passed an optional windows number, which is used when
// there are user defined windows for locating the window object.
//
//-----------------------------------------------------------------------------------------------------------
void DrvWinDisplay::windowToggle( TokId winCmd, int winNum ) {
    
    switch( winCmd ) {
            
            // ??? perhaps we will have one day also toggles for the fixed windows...
            // ??? e.g. GRT - general reg window toggle...
            
        case CMD_WT: {
            
            if ( winNum == 0 ) winNum = getCurrentUserWindow( );
            
            if ( validUserWindowNum( winNum )) {
                
                if ( winNum == 0 ) winNum = getCurrentUserWindow( );
                
                ((DrvWinScrollable *) windowList[ winNum ] ) -> toggleWin( );
                setCurrentUserWindow( winNum );
            }
            
        } break;
            
        default: ;
    }
}

//-----------------------------------------------------------------------------------------------------------
// "Window New" creates a new window for certain window types. For example, it would be good to have multiple
// physical memory windows to see different locations simultaneously. The window object for the supported
// window types is created and added to the windows list. The newly created window also becomes the current
// user window.
//
//-----------------------------------------------------------------------------------------------------------
void DrvWinDisplay::windowNew( TokId winCmd, TokId winType, char *argStr ) {
    
    int i = 0;
    
    for ( i = 0; i <= LAST_UWIN; i++ ) {
        
        if ( windowList[ i ] == nullptr ) break;
    }
    
    if ( i <= LAST_UWIN ) {
        
        switch ( winType ) {
                
            case TOK_PM: windowList[ i ] = ( DrvWin * ) new DrvWinAbsMem( glb ); break;
            case TOK_PC: windowList[ i ] = ( DrvWin * ) new DrvWinCode( glb ); break;
            case TOK_IT: windowList[ i ] = ( DrvWin * ) new DrvWinTlb( glb, WT_ITLB_WIN ); break;
            case TOK_DT: windowList[ i ] = ( DrvWin * ) new DrvWinTlb( glb, WT_DTLB_WIN ); break;
            case TOK_IC: windowList[ i ] = ( DrvWin * ) new DrvWinCache( glb, WT_ICACHE_WIN ); break;
            case TOK_DC: windowList[ i ] = ( DrvWin * ) new DrvWinCache( glb, WT_DCACHE_WIN ); break;
            case TOK_UC: windowList[ i ] = ( DrvWin * ) new DrvWinCache( glb, WT_UCACHE_WIN ); break;
            case TOK_TX: windowList[ i ] = ( DrvWin * ) new DrvWinText( glb, argStr ); break;
            case TOK_ICR: windowList[ i ] = ( DrvWin * ) new DrvWinMemController( glb, WT_ICACHE_S_WIN ); break;
            case TOK_DCR: windowList[ i ] = ( DrvWin * ) new DrvWinMemController( glb, WT_DCACHE_S_WIN ); break;
            case TOK_UCR: windowList[ i ] = ( DrvWin * ) new DrvWinMemController( glb, WT_UCACHE_S_WIN ); break;
            case TOK_MCR: windowList[ i ] = ( DrvWin * ) new DrvWinMemController( glb, WT_MEM_S_WIN ); break;
           
            default: return;
        }
        
        windowList[ i ] -> setDefaults( );
        windowList[ i ] -> setWinIndex( i );
        windowList[ i ] -> setEnable( true );
        setCurrentUserWindow( i );
    }
}

//-----------------------------------------------------------------------------------------------------------
// "Window Kill" is the counter part to user windows creation and will remove the window identified by the
// windows number permanently. When we kill a window that was the current window, we need to set a new one.
// We just pick the first used entry in the user range.
//
//-----------------------------------------------------------------------------------------------------------
void DrvWinDisplay::windowKill( TokId winCmd, int winNum ) {
    
    if ( winNum == 0 ) winNum = getCurrentUserWindow( );
    
    if (( validWindowNum( winNum )) && ( winNum >= FIRST_UWIN )) {
        
        delete ( DrvWin * ) windowList[ winNum ];
        windowList[ winNum ] = nullptr;
        
        if ( getCurrentUserWindow( ) == winNum ) {
            
            setCurrentUserWindow( 0 );
            
            for ( int i = FIRST_UWIN; i <= LAST_UWIN; i++ ) {
                
                if ( validUserWindowNum( i )) {
                    
                    setCurrentUserWindow( i );
                    break;
                }
            }
        }
    }
}
