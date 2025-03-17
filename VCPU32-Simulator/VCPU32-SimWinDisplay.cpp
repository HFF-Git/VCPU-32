//------------------------------------------------------------------------------------------------------------
//
// VCPU32 - A 32-bit CPU - Simulator window subsystem
//
//------------------------------------------------------------------------------------------------------------
// This module contains the window display routines. The window subsysten uses a ton of escape sequences to
// create a terminal window screen and displays sub windows on the screen.
//
//------------------------------------------------------------------------------------------------------------
//
// CPU32 - A 32-bit CPU - Simulator window subsystem
// Copyright (C) 2022 - 2025 Helmut Fieres
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
#include "VCPU32-Types.h"
#include "VCPU32-Core.h"
#include "VCPU32-SimDeclarations.h"
#include "VCPU32-SimTables.h"

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
//  new             -> newUserWin       -> N
//  kill            -> winUserKill      -> K
//  current         -> currentUserWin   -> C
//  toggle          -> winToggle        -> T
//
//  Windows:
//
//  Program Regs    -> PS
//  General Regs    -> GR
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
// Local name space. We try to keep utility functions local to the file. None so far.
//
//------------------------------------------------------------------------------------------------------------
namespace {

}; // namespace

//-----------------------------------------------------------------------------------------------------------
// Object constructor. We initialize the windows list and create all the predefined windows. The remainder
// of the window list is used by the user defined windows.
//
// ??? is this the logical place to create the special reg windows ?
//-----------------------------------------------------------------------------------------------------------
SimWinDisplay::SimWinDisplay( VCPU32Globals *glb ) {
    
    this -> glb = glb;
    
    for ( int i = 0; i < MAX_WINDOWS; i++ ) windowList[ i ] = nullptr;
    
    windowList[ PS_REG_WIN ]    = new SimWinProgState( glb );
    windowList[ CTRL_REG_WIN ]  = new SimWinSpecialRegs( glb );
    windowList[ PL_REG_WIN ]    = new SimWinPipeLineRegs( glb );
    windowList[ STATS_WIN ]     = new SimWinStatistics( glb );
}

//-----------------------------------------------------------------------------------------------------------
// The current window number defines which user window is marked "current" and commands that omit the window
// number in their command will use this number. There is a routine to check that we have a valid window
// number, which includes fixed and user numbers. There are also a routines that returns the first and last
// index valid for user windows.
//
//-----------------------------------------------------------------------------------------------------------
int  SimWinDisplay::getCurrentUserWindow( ) {
    
    return( currentUserWinNum );
}

void SimWinDisplay::setCurrentUserWindow( int winNum ) {
    
    currentUserWinNum = winNum;
}

int  SimWinDisplay::getFirstUserWinIndex( ) {
    
    return( FIRST_UWIN );
}

int  SimWinDisplay::getLastUserWinIndex( ) {
    
    return( LAST_UWIN );
}

//-----------------------------------------------------------------------------------------------------------
// A window number is the index into the window list. It is valid when the number is of course within bounds
// and the window list entry is actually used. A valid user window number additionally tests that the number
// is within the list portion reserved for user defined windows.
//
//-----------------------------------------------------------------------------------------------------------
bool SimWinDisplay::validWindowNum( int winNum ) {
    
    return(( winNum <= LAST_UWIN ) && ( windowList[ winNum ] != nullptr ));
}

bool SimWinDisplay::validUserWindowNum( int winNum ) {
    
    return(( winNum >= FIRST_UWIN ) && ( winNum <= LAST_UWIN ) && ( windowList[ winNum ] != nullptr ));
}

bool SimWinDisplay::validWindowStackNum( int stackNum ) {
    
    return(( stackNum >= 0 ) && ( stackNum < MAX_WIN_STACKS ));
}

bool SimWinDisplay::validUserWindowType( SimTokId winType ) {
    
    return(( winType == TOK_PM  )   || ( winType == TOK_PC )    || ( winType == TOK_IT )    ||
           ( winType == TOK_ITR )   || ( winType == TOK_DT )    || ( winType == TOK_DTR )   ||
           ( winType == TOK_IC )    || ( winType == TOK_ICR )   || ( winType == TOK_DC )    ||
           ( winType == TOK_DCR )   || ( winType == TOK_UC )    || ( winType == TOK_UCR )   ||
           ( winType == TOK_MCR )   || ( winType == TOK_TX ));
}

bool SimWinDisplay::isCurrentWin( int winNum ) {
    
    return(( validUserWindowNum( winNum ) && ( currentUserWinNum == winNum )));
}

bool SimWinDisplay::isWinEnabled( int winNum ) {

    return((( validWindowNum( winNum )) || ( validUserWindowNum( winNum )) ) &&
           ( windowList[ winNum ]->isEnabled( )));
}

//-----------------------------------------------------------------------------------------------------------
// Before drawing the screen content after the execution of a command line, we need to check whether the
// number of columns needed for a stack of windows has changed. This function just runs through the window
// list for a given stack and determines the widest column needed for that stack. When no window is enabled
// the column size will be set to the command wondow default size.
//
//-----------------------------------------------------------------------------------------------------------
int SimWinDisplay::computeColumnsNeeded( int winStack ) {
    
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
void SimWinDisplay::setWindowColumns( int winStack, int columnSize ) {
    
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
int SimWinDisplay::computeRowsNeeded( int winStack ) {
    
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
// to know the absolute position within in the overall screen. This routine will compute for each window of
// the passed stack the absolute row and column position for the window in the terminal screen.
//
//-----------------------------------------------------------------------------------------------------------
void SimWinDisplay::setWindowOrigins( int winStack, int rowOffset, int colOffset ) {
    
    int tmpRow = rowOffset;
    
    for ( int i = 0; i < MAX_WINDOWS; i++ ) {
        
        if (( windowList[ i ] != nullptr ) &&
            ( windowList[ i ] -> isEnabled( )) &&
            ( windowList[ i ] -> getWinStack( ) == winStack )) {
            
            windowList[ i ] -> setWinOrigin( tmpRow, colOffset );
            tmpRow += windowList[ i ] -> getRows( );
        }
        
        glb -> cmdWin -> setWinOrigin( tmpRow, colOffset );
    }
}

//-----------------------------------------------------------------------------------------------------------
// Window screen drawing. Each time we read in a command input and are in windows mode, the terminal screen
// is redrawn. A terminal screen consist of a list of stacks and in each stack a list of windows. There is
// always the main stack, stack Id 0. Only if we have user defined windows assigned to another stack and
// window stacks are enabled, will this stack show up in the terminal screen. If window stacks are disabled,
// all windows, regardless what their stack ID says, will show up in the main stack.
//
// We first compute the number of rows and columns needed for the windows to show in their assigned stack.
// Only enabled screens will participate in the overall screen size computation. The data is used then to
// set the window columns of a window in the respective stack to the computed columns size and to set the
// absolute origin coordinates of each window. Again, this depends whether window stacks er enabled. If the
// number of rows needed for the windows and command window is less than the defined minimum number of rows,
// the command window is enlarged to have a screen of minimum row size. When the screen size changed, we
// just redraw the screen with the command screen going last. The command screen will have a columns size
// across all visible stacks.
//
//-----------------------------------------------------------------------------------------------------------
void SimWinDisplay::reDraw( bool mustRedraw ) {
    
    int winStackColumns[ MAX_WIN_STACKS ]   = { 0 };
    int winStackRows[ MAX_WIN_STACKS ]      = { 0 };
    int defRowSize                          = glb -> env -> getEnvVarInt((char *) ENV_WIN_MIN_ROWS );
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
        else {
            
            setWindowColumns( i, maxColumnsNeeded );
            curRows += winStackRows[ i ];
        }
    }
    
    if (( maxRowsNeeded + glb -> cmdWin -> getRows( )) < defRowSize ) {
        
        glb -> cmdWin -> setRows( defRowSize - maxRowsNeeded );
        maxRowsNeeded += glb -> cmdWin -> getRows();
    }
    else maxRowsNeeded += glb -> cmdWin -> getRows( );
    
    if ( maxColumnsNeeded == 0 ) maxColumnsNeeded = glb -> cmdWin -> getDefColumns( ) + stackColumnGap;
    
    if ( winStacksOn )  glb -> cmdWin -> setColumns( maxColumnsNeeded - stackColumnGap );
    else                glb -> cmdWin -> setColumns( maxColumnsNeeded );
    
    glb -> cmdWin -> setWinOrigin( maxRowsNeeded - glb -> cmdWin -> getRows( ) + 1, 1 );
  
    if ( mustRedraw ) {
        
        actualRowSize      = maxRowsNeeded;
        actualColumnSize   = maxColumnsNeeded;
        
        glb -> console -> setWindowSize( actualRowSize, actualColumnSize );
        glb -> console -> setAbsCursor( 1, 1 );
        glb -> console -> clearScrollArea( );
        glb -> console -> clearScreen( );
        glb -> console -> setScrollArea( actualRowSize - glb -> cmdWin -> getRows( ) + 2, actualRowSize );
    }
    
    for ( int i = 0; i < MAX_WINDOWS; i++ ) {
        
        if (( windowList[ i ] != nullptr ) && ( windowList[ i ] -> isEnabled( ))) {
            
            windowList[ i ] -> reDraw( );
        }
    }
    
    glb -> cmdWin -> reDraw( );
    glb -> console -> setAbsCursor( actualRowSize, 1 );
}

//-----------------------------------------------------------------------------------------------------------
// The entry point to showing windows is to draw the screen on the "windows on" command and to clean up when
// we switch back to line mode. The window defaults method will set the windows to a preconfigured default
// value. This is quite useful when we messed up our screens. Also, if the screen is displayed garbled after
// some windows mouse based screen window changes, just do WON again to set it straight. There is also
// a function to enable or disable the window stacks feature.
//
//-----------------------------------------------------------------------------------------------------------
void SimWinDisplay::windowsOn( ) {
    
    // nothing to do here...
}

void SimWinDisplay::windowsOff( ) {
    
    glb -> console -> clearScrollArea( );
    glb -> console -> clearScreen( );
}

void SimWinDisplay::windowDefaults( ) {
    
    for ( int i = 0; i < MAX_WINDOWS; i++ ) {
        
        if ( windowList[ i ] != nullptr ) windowList[ i ] -> setDefaults( );
    }
    
    glb -> cmdWin -> setDefaults( );
}

void SimWinDisplay::winStacksEnable( bool arg ) {
    
    winStacksOn = arg;
}

//-----------------------------------------------------------------------------------------------------------
// A user defined window can be set to be the current user window. Commands that allow to specify a window
// number will use the window set by default then. Note that each user defined command that specifies the
// window number in its command will also set the current value. The user window becomes the actual window.
//
//-----------------------------------------------------------------------------------------------------------
void SimWinDisplay::windowCurrent( int winNum ) {
    
    if ( validUserWindowNum( winNum )) currentUserWinNum = winNum;
}

//-----------------------------------------------------------------------------------------------------------
// The routine sets the stack attribute for a user window. The setting is not allowed for the predefined
// window. They are always in the main window stack, which has the stack Id of zero. Theoretically we
// could have many stacks, numbered 0 to MAX_STACKS - 1. Realistically, 3 to 4 stacks will fit on a screen.
//
//-----------------------------------------------------------------------------------------------------------
void SimWinDisplay::windowSetStack( int winStack, int winNumStart, int winNumEnd ) {
    
    if (( winNumStart < FIRST_UWIN ) || ( winNumEnd > LAST_UWIN ))  return;
    if ( winStack >= MAX_WIN_STACKS ) return;
    
    if ( winNumStart > winNumEnd ) {
        
        int tmp = winNumStart;
        winNumStart = winNumEnd;
        winNumEnd = tmp;
    }
    
    for ( int i = winNumStart; i <= winNumEnd; i++ ) {
        
        if ( windowList[ i ] != nullptr ) {
            
            windowList[ i ] -> setWinStack( winStack );
            setCurrentUserWindow( i );
        }
    }
}

//-----------------------------------------------------------------------------------------------------------
// A window can be added or removed for the window list shown. We are passed an optional windows number,
// which is used when there are user defined windows for locating the window object.
//
//-----------------------------------------------------------------------------------------------------------
void SimWinDisplay::windowEnable( SimTokId winCmd, int winNum, bool show ) {
    
    switch( winCmd ) {
            
        case CMD_PSE:
        case CMD_PSD: windowList[ PS_REG_WIN ] -> setEnable( show );    break;
        case CMD_SRE:
        case CMD_SRD: windowList[ CTRL_REG_WIN ] -> setEnable( show );  break;
        case CMD_PLE:
        case CMD_PLD: windowList[ PL_REG_WIN ] -> setEnable( show );    break;
        case CMD_SWE:
        case CMD_SWD: windowList[ STATS_WIN ] -> setEnable( show );     break;
       
        case CMD_WD:
        case CMD_WE: {
            
            if ( winNum == 0 ) winNum = getCurrentUserWindow( );
            
            if ( validUserWindowNum( winNum )) {
                
                windowList[ winNum ] -> setEnable( show );
                setCurrentUserWindow( winNum );
            }
            
        } break;
            
        default: ;
    }
}

//-----------------------------------------------------------------------------------------------------------
// For the numeric values in a window, we can set the radix. The token ID for the format option is mapped to
// the actual radix value.We are passed an optional windows number, which is used when there are user defined
// windows for locating the window object. Changing the radix potentially means that the window layout needs
// to change.
//
//-----------------------------------------------------------------------------------------------------------
void SimWinDisplay::windowRadix( SimTokId winCmd, int rdx, int winNum ) {
    
    switch( winCmd ) {
            
        case CMD_PSR: windowList[ PS_REG_WIN ] -> setRadix( rdx );    break;
        case CMD_SRR: windowList[ CTRL_REG_WIN ] -> setRadix( rdx );  break;
        case CMD_PLR: windowList[ PL_REG_WIN ] -> setRadix( rdx );    break;
        case CMD_SWR: windowList[ STATS_WIN ] -> setRadix( rdx );     break;
     
        case CMD_WR: {
            
            if ( winNum == 0 ) winNum = getCurrentUserWindow( );
            
            if ( validUserWindowNum( winNum )) {
                
                windowList[ winNum ] -> setRadix( rdx );
                setCurrentUserWindow( winNum );
            }
           
        } break;
            
        default: ;
    }
    
    reDraw( true );
}

//-----------------------------------------------------------------------------------------------------------
// "setRows" is the method to set the number if lines in a window. The number includes the banner. We are
// passed an optional windows number, which is used when there are user defined windows for locating the
// window object.
//
//-----------------------------------------------------------------------------------------------------------
void SimWinDisplay::windowSetRows( SimTokId winCmd, int rows, int winNum ) {
    
    switch ( winCmd ) {
            
        case CMD_CWL: glb -> cmdWin -> setRows( rows ); break;
            
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
// cleared. If the position passed is non-zero, it will become the new home position. The position meaning
// is window dependent and the actual window will sort it out. We are passed an optional windows number,
// which is used when there are user defined windows for locating the window object.
//
//-----------------------------------------------------------------------------------------------------------
void SimWinDisplay::windowHome( SimTokId winCmd, int pos, int winNum ) {
    
    if ( winNum == 0 ) winNum = getCurrentUserWindow( );
    
    if ( validUserWindowNum( winNum )) {
        
        ((SimWinScrollable *) windowList[ winNum ] ) -> winHome( pos );
        setCurrentUserWindow( winNum );
    }
}

//-----------------------------------------------------------------------------------------------------------
// A window is scrolled forward with the "windowForward" method. The meaning of the amount is window
// dependent and the actual window will sort it out. We are passed an optional windows number, which is used
// when there are user defined windows for locating the window object.
//
//-----------------------------------------------------------------------------------------------------------
void SimWinDisplay::windowForward( SimTokId winCmd, int amt, int winNum ) {
    
    if ( winNum == 0 ) winNum = getCurrentUserWindow( );
    
    if ( validUserWindowNum( winNum )) {
        
        ((SimWinScrollable *) windowList[ winNum ] ) -> winForward( amt );
        setCurrentUserWindow( winNum );
    }
}

//-----------------------------------------------------------------------------------------------------------
// A window is scrolled backward with the "windowBackward" methods. The meaning of the amount is window
// dependent and the actual window will sort it out. We are passed an optional windows number, which is used
// when there are user defined windows for locating the window object.
//
//-----------------------------------------------------------------------------------------------------------
void SimWinDisplay::windowBackward( SimTokId winCmd, int amt, int winNum ) {
    
    if ( winNum == 0 ) winNum = getCurrentUserWindow( );
    
    if ( validUserWindowNum( winNum )) {
        
        ((SimWinScrollable *) windowList[ winNum ] ) -> winBackward( amt );
        setCurrentUserWindow( winNum );
    }
}

//-----------------------------------------------------------------------------------------------------------
// The current index can also directly be set to another location. The position meaning is window dependent
// and the actual window will sort it out. We are passed an optional windows number, which is used when
// there are user defined windows for locating the window object.
//
//-----------------------------------------------------------------------------------------------------------
void SimWinDisplay::windowJump( SimTokId winCmd, int pos, int winNum ) {
    
    if ( winNum == 0 ) winNum = getCurrentUserWindow( );
    
    if ( validUserWindowNum( winNum )) {
      
        ((SimWinScrollable *) windowList[ winNum ] ) -> winJump( pos );
        setCurrentUserWindow( winNum );
    }
}

//-----------------------------------------------------------------------------------------------------------
// The current window index can also directly be set to another location. The position meaning is window
// dependent and the actual window will sort it out. We are passed an optional windows number, which is used
// when there are user defined windows for locating the window object.
//
//-----------------------------------------------------------------------------------------------------------
void SimWinDisplay::windowToggle( int winNum ) {
    
    if ( winNum == 0 ) winNum = getCurrentUserWindow( );
    
    if ( validUserWindowNum( winNum )) {
        
        ((SimWinScrollable *) windowList[ winNum ] ) -> toggleWin( );
        setCurrentUserWindow( winNum );
    }
}

//-----------------------------------------------------------------------------------------------------------
// The display order of the windows is determined by the window index. It would however be convenient to
// modify the display order. The window exchange command will exchange the current window with the window
// specified by the index of another window.
//
//-----------------------------------------------------------------------------------------------------------
void SimWinDisplay::windowExchangeOrder( int winNum ) {
    
    int currentWindow = getCurrentUserWindow( );
    if ( winNum == currentWindow ) return;
    if ( ! validUserWindowNum( winNum )) return;
    
    std::swap( windowList[ winNum ], windowList[ currentWindow ]);
}

//-----------------------------------------------------------------------------------------------------------
// "Window New" creates a new window for certain window types. For example, it would be good to have multiple
// physical memory windows to see different locations simultaneously. The window object for the supported
// window types is created and added to the windows list. The newly created window also becomes the current
// user window.
//
//-----------------------------------------------------------------------------------------------------------
void SimWinDisplay::windowNew( SimTokId winType, char *argStr ) {
    
    int i = 0;
    
    for ( i = 0; i <= LAST_UWIN; i++ ) {
        
        if ( windowList[ i ] == nullptr ) break;
    }
    
    if ( i <= LAST_UWIN ) {
        
        switch ( winType ) {
                
            case TOK_PM: windowList[ i ] = ( SimWin * ) new SimWinAbsMem( glb ); break;
            case TOK_PC: windowList[ i ] = ( SimWin * ) new SimWinCode( glb ); break;
            case TOK_IT: windowList[ i ] = ( SimWin * ) new SimWinTlb( glb, WT_ITLB_WIN ); break;
            case TOK_DT: windowList[ i ] = ( SimWin * ) new SimWinTlb( glb, WT_DTLB_WIN ); break;
            case TOK_IC: windowList[ i ] = ( SimWin * ) new SimWinCache( glb, WT_ICACHE_WIN ); break;
            case TOK_DC: windowList[ i ] = ( SimWin * ) new SimWinCache( glb, WT_DCACHE_WIN ); break;
            case TOK_UC: windowList[ i ] = ( SimWin * ) new SimWinCache( glb, WT_UCACHE_WIN ); break;
            case TOK_TX: windowList[ i ] = ( SimWin * ) new SimWinText( glb, argStr ); break;
            case TOK_ICR: windowList[ i ] = ( SimWin * ) new SimWinMemController( glb, WT_ICACHE_S_WIN ); break;
            case TOK_DCR: windowList[ i ] = ( SimWin * ) new SimWinMemController( glb, WT_DCACHE_S_WIN ); break;
            case TOK_UCR: windowList[ i ] = ( SimWin * ) new SimWinMemController( glb, WT_UCACHE_S_WIN ); break;
            case TOK_MCR: windowList[ i ] = ( SimWin * ) new SimWinMemController( glb, WT_MEM_S_WIN ); break;
           
            default: return;
        }
        
        windowList[ i ] -> setDefaults( );
        windowList[ i ] -> setWinIndex( i );
        windowList[ i ] -> setEnable( true );
        setCurrentUserWindow( i );
    }
}

//-----------------------------------------------------------------------------------------------------------
// "Window Kill" is the counter part to user windows creation and will remove a window. The method supports
// removing a range of user windows. When we kill a window that was the current window, we need to set a
// new one. We just pick the first used entry in the user range.
//
//-----------------------------------------------------------------------------------------------------------
void SimWinDisplay::windowKill( int winNumStart, int winNumEnd ) {
    
    if (( winNumStart < FIRST_UWIN ) || ( winNumEnd > LAST_UWIN ))  return;
    
    if ( winNumStart > winNumEnd ) {
        
        int tmp = winNumStart;
        winNumStart = winNumEnd;
        winNumEnd = tmp;
    }
    
    for ( int i = winNumStart; i <= winNumEnd; i++ ) {
         
        delete ( SimWin * ) windowList[ i ];
        windowList[ i ] = nullptr;
                
        if ( getCurrentUserWindow( ) == i ) {
                    
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

