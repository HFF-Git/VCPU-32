//------------------------------------------------------------------------------------------------------------
//
// VCPU32 - A 32-bit CPU - Simulator window base classes
//
//------------------------------------------------------------------------------------------------------------
// This ...
//
//------------------------------------------------------------------------------------------------------------
//
// VCPU32 - A 32-bit CPU - Simulator window base classes
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
#include "VCPU32-SimVersion.h"
#include "VCPU32-Types.h"
#include "VCPU32-SimDeclarations.h"
#include "VCPU32-SimTables.h"
#include "VCPU32-Core.h"

//------------------------------------------------------------------------------------------------------------
// Local name space. We try to keep utility functions local to the file.
//
//------------------------------------------------------------------------------------------------------------
namespace {

//------------------------------------------------------------------------------------------------------------
// "setRadix" ensures that we passed in a valid radix value. The default is a decimal number.
//
//------------------------------------------------------------------------------------------------------------
int setRadix( int rdx ) {
    
    return((( rdx == 8 ) || ( rdx == 10 ) || ( rdx == 16 )) ? rdx : 10 );
}

//------------------------------------------------------------------------------------------------------------
// Routine to figure out what size we need for a numeric word in a given radix. Decimals needs 10 digits,
// octals need 12 digits and hexadecimals need 10 digits. For a 16-bit word, the numbers are reduced to 5, 7
// and 6.
//
//------------------------------------------------------------------------------------------------------------
int strlenForNum( int rdx, bool halfWord ) {
    
    if      ( rdx == 10 ) return(( halfWord ) ? 5 : 10 );
    else if ( rdx == 8  ) return(( halfWord ) ? 7 : 12 );
    else if ( rdx == 16 ) return(( halfWord ) ? 6 : 10 );
    else return( 10 );
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
// Object constructor and destructor. We need them as we create and destroy user definable windows.
//
//------------------------------------------------------------------------------------------------------------
SimWin::SimWin( VCPU32Globals *glb ) { this -> glb = glb; }
SimWin:: ~SimWin( ) { }

//------------------------------------------------------------------------------------------------------------
// Getter/Setter methods for window attributes.
//
//------------------------------------------------------------------------------------------------------------
void    SimWin::setWinType( int arg ) { winType = arg; }
int     SimWin::getWinType( ) { return( winType ); }

void    SimWin::setWinIndex( int arg ) { winUserIndex = arg; }
int     SimWin::getWinIndex( ) { return( winUserIndex ); }

void    SimWin::setEnable( bool arg ) { winEnabled = arg; }
bool    SimWin::isEnabled( ) { return( winEnabled ); }

void    SimWin::setRows( int arg ) { winRows = (( arg > MAX_WIN_ROW_SIZE ) ? MAX_WIN_ROW_SIZE : arg ); }
int     SimWin::getRows( ) { return( winRows ); }

void    SimWin::setColumns( int arg ) { winColumns = arg; }
int     SimWin::getColumns( ) { return( winColumns ); }

void    SimWin::setRadix( int rdx ) { winRadix = ::setRadix( rdx ); }
int     SimWin::getRadix( ) { return( winRadix ); }

int     SimWin::getWinStack( ) { return( winStack ); }
void    SimWin::setWinStack( int wCol ) { winStack = wCol; }

int SimWin::getDefColumns( int rdx ) {
    
    switch ( rdx ) {
            
        case 16:    return( winDefColumnsHex );
        case 8:     return( winDefColumnsOct );
        case 10:    return( winDefColumnsDec );
        default:    return( winDefColumnsHex );
    }
}

void SimWin::setDefColumns( int arg, int rdx ) {
    
    switch ( rdx ) {
            
        case 16:    winDefColumnsHex = arg; break;
        case 8:     winDefColumnsOct = arg; break;
        case 10:    winDefColumnsDec = arg; break;
        default:    winDefColumnsHex = winDefColumnsOct = winDefColumnsDec = arg;
    }
}

//------------------------------------------------------------------------------------------------------------
// "setWinOrigin" sets the absolute cursor position for the terminal screen. We maintain absolute positions,
// which only may change when the terminal screen is redrawn with different window sizes. The window relative
// rows and column cursor position are set at (1,1).
//
//------------------------------------------------------------------------------------------------------------
void SimWin::setWinOrigin( int row, int col ) {
    
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
void SimWin::setWinCursor( int row, int col ) {
    
    if ( row == 0 ) row = lastRowPos;
    if ( col == 0 ) col = lastColPos;
    
    if ( row > winRows )            row = winRows;
    if ( col > MAX_WIN_COL_SIZE )   col = MAX_WIN_COL_SIZE;
    
    glb -> console -> setAbsCursor( winAbsCursorRow + row - 1, winAbsCursorCol + col );
    
    lastRowPos = row;
    lastColPos = col;
}

int SimWin::getWinCursorRow( ) { return( lastRowPos ); }
int SimWin::getWinCursorCol( ) { return( lastColPos ); }

// ??? should this become part of teh console IO ? it is also not really a fgield but rather a change in
// attributes...
//------------------------------------------------------------------------------------------------------------
// A window will consist of lines with lines having fields on them. A field has a set of attributes such as
// foreground and background colors, bold characters and so on. This routine sets the attributes based on the
// format descriptor. If the descriptor is zero, we will just stay where are with their attributes.
//
// ??? comment the attributes meaning....
//------------------------------------------------------------------------------------------------------------
void SimWin::setFieldAtributes( uint32_t fmtDesc ) {
    
    if ( fmtDesc != 0 ) {
        
        glb -> console -> writeChars((char *) "\x1b[0m" );
        if ( fmtDesc & FMT_INVERSE )    glb -> console -> writeChars((char *) "\x1b[7m" );
        if ( fmtDesc & FMT_BLINK )      glb -> console -> writeChars((char *) "\x1b[5m" );
        if ( fmtDesc & FMT_BOLD )       glb -> console -> writeChars((char *) "\x1b[1m" );
        
        switch ( fmtDesc & 0xF ) {
                
            case 1:     glb -> console -> writeChars((char *) "\x1b[41m"); break;
            case 2:     glb -> console -> writeChars((char *) "\x1b[42m"); break;
            case 3:     glb -> console -> writeChars((char *) "\x1b[43m"); break;
            default:    glb -> console -> writeChars((char *) "\x1b[49m");
        }
        
        switch (( fmtDesc >> 4 ) & 0xF ) {
                
            case 1:     glb -> console -> writeChars((char *) "\x1b[31m"); break;
            case 2:     glb -> console -> writeChars((char *) "\x1b[32m"); break;
            case 3:     glb -> console -> writeChars((char *) "\x1b[33m"); break;
            default:    glb -> console -> writeChars((char *) "\x1b[39m");
        }
    }
}

//------------------------------------------------------------------------------------------------------------
// Routine for putting out a 32-bit or 16-bit machine word at the current cursor position. We will just print
// out the data using the radix passed. ( HEX: 0xdddddddd, OCT: 0ddddddddddd, DEC: dddddddddd );
//
//------------------------------------------------------------------------------------------------------------
int SimWin::printWord( uint32_t val, int rdx, uint32_t fmtDesc ) {
    
    int len;
    
    bool half   = fmtDesc & FMT_HALF_WORD;
    bool noNum  = fmtDesc & FMT_INVALID_NUM;
    
        if ( rdx == 10 ) {
            
            if ( noNum ) {
                
                if ( half ) len = glb -> console -> writeChars((char *) "*****" );
                else        len = glb -> console -> writeChars((char *) "**********" );
            }
            else {
                
                if ( half ) len = glb -> console -> writeChars((char *) "%5d", val );
                else        len = glb -> console -> writeChars((char *) "%10d", val );
            }
        }
        else if ( rdx == 8 ) {
            
            if ( noNum ) {
                
                if ( half ) len = glb -> console -> writeChars((char *) "*******" );
                else        len = glb -> console -> writeChars((char *) "************" );
            }
            else {
                
                if ( half ) len = glb -> console -> writeChars((char *) "%07o", val );
                else        len = glb -> console -> writeChars((char *) "%#012o", val );
            }
        }
        else if ( rdx == 16 ) {
            
            if ( noNum ) {
                
                if ( half ) len = glb -> console -> writeChars((char *) "******" );
                else        len = glb -> console -> writeChars((char *) "**********" );
            }
            else {
                
                if ( val == 0 ) {
                    
                    if ( half ) len = glb -> console -> writeChars((char *) "0x0000" );
                    else        len = glb -> console -> writeChars((char *) "0x00000000" );
                    
                } else {
                    
                    if ( half ) len = glb -> console -> writeChars((char *) "%#06x", val );
                    else        len = glb -> console -> writeChars((char *) "%#010x", val );
                }
            }
        }
        else len = glb -> console -> writeChars((char *) "***num***" );
    
    return( len );
}

//------------------------------------------------------------------------------------------------------------
// Routine for putting out simple text. We make sure that the string length is in the range of what the text
// size could be.
//
//------------------------------------------------------------------------------------------------------------
int SimWin::printText( char *text, int len ) {
    
    if ( strlen( text ) < MAX_TEXT_FIELD_LEN ) return( glb -> console -> writeChars((char *) "%s", text ));
    else return( glb -> console -> writeChars((char *) "***Text***" ));
}

//------------------------------------------------------------------------------------------------------------
// Fields that have a larger size than the actual argument length in the field need to be padded left or
// right. This routine is just a simple loop emitting blanks in the current format set.
//
//------------------------------------------------------------------------------------------------------------
void SimWin::padField( int dLen, int fLen ) {
    
    while ( fLen > dLen ) {
        
        glb -> console -> writeChars((char *) " " );
        fLen --;
    }
}

//------------------------------------------------------------------------------------------------------------
// Print out a numeric field. Each call will set the format options passed via the format descriptor. If the
// field length is larger than the positions needed to print the data in the field, the data will be printed
// left or right justified in the field.
//
// ??? shouldn't we feed into printFieldText to unify field printing ?
// ??? add radix to override default setting ?
//-----------------------------------------------------------------------------------------------------------
void SimWin::printNumericField( uint32_t val, uint32_t fmtDesc, int fLen, int row, int col ) {
    
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
// left or right justified in the field. If the data is larger than the field, it will be truncated.
//
//-----------------------------------------------------------------------------------------------------------
void SimWin::printTextField( char *text, uint32_t fmtDesc, int fLen, int row, int col ) {
    
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
void SimWin::printRadixField( uint32_t fmtDesc, int fLen, int row, int col ) {
    
    setFieldAtributes( fmtDesc );
    
    if ( fmtDesc & FMT_LAST_FIELD ) col = winColumns - fLen;
    
    switch ( winRadix ) {
            
        case 8:  printTextField((char *) "oct", fmtDesc, 3, row, col); break;
        case 10: printTextField((char *) "dec", fmtDesc, 3, row, col); break;
        case 16: printTextField((char *) "hex", fmtDesc, 3, row, col); break;
        default: ;
    }
}

//------------------------------------------------------------------------------------------------------------
// A user defined window has a field that shows the window number as well as this is the current window.
//
//------------------------------------------------------------------------------------------------------------
void SimWin::printWindowIdField( int stack, int index, bool current, uint32_t fmtDesc, int row, int col ) {
    
    if ( row == 0 ) row = lastRowPos;
    if ( col == 0 ) col = lastColPos;
    
    setFieldAtributes( fmtDesc );
    
    if      (( index >= 0   ) && ( index <= 10 ))   glb -> console -> writeChars((char *) "(%1d:%1d)  ", stack, index );
    else if (( index >= 10  ) && ( index <= 99 ))   glb -> console -> writeChars((char *) "(%1d:%2d) ", stack, index );
    else                                            glb -> console -> writeChars((char *) "-***-" );
    
    glb -> console -> writeChars((( current ) ? (char *) "* " : (char *) "  " ));
    
    lastRowPos  = row;
    lastColPos  = col + 9;
}

//------------------------------------------------------------------------------------------------------------
// Padding a line will write a set of blanks with the current format setting to the end of the line. It is
// intended to fill for example a banner line that is in inverse video with the inverse format until the end
// of the screen column size.
//
//------------------------------------------------------------------------------------------------------------
void SimWin::padLine( uint32_t fmtDesc ) {
    
    setFieldAtributes( fmtDesc );
    padField( lastColPos, winColumns );
}

//------------------------------------------------------------------------------------------------------------
//
//
//
//------------------------------------------------------------------------------------------------------------
void SimWin::clearField( int len, uint32_t fmtDesc ) {
    
    int pos = lastColPos;
    
    if ( pos + len > winColumns ) len = winColumns - pos;
    
    setFieldAtributes( fmtDesc );
    padField( lastColPos, lastColPos + len );
    
    setWinCursor( 0,  pos );
}

//------------------------------------------------------------------------------------------------------------
// Each window consist of a banner and a body. The reDraw routine will invoke these mandatory routines of
// the child classes.
//
//------------------------------------------------------------------------------------------------------------
void SimWin::reDraw( ) {
    
    if ( winEnabled ) {
        
        drawBanner( );
        drawBody( );
    }
}

//------------------------------------------------------------------------------------------------------------
// Each window allows for perhaps toggling through different content. The implementation of this capability
// is entirely up to the specific window. On the "WT" command, this function will be called.
//
//------------------------------------------------------------------------------------------------------------
void SimWin::toggleWin( ) { }

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
SimWinScrollable::SimWinScrollable( VCPU32Globals *glb ) : SimWin( glb ) { }

//------------------------------------------------------------------------------------------------------------
// Getter/Setter methods for scrollable window attributes.
//
//------------------------------------------------------------------------------------------------------------
void      SimWinScrollable::setHomeItemAdr( uint32_t adr ) { homeItemAdr = adr; }
uint32_t  SimWinScrollable::getHomeItemAdr( ) { return( homeItemAdr ); }

void      SimWinScrollable::setCurrentItemAdr( uint32_t adr ) { currentItemAdr = adr; }
uint32_t  SimWinScrollable::getCurrentItemAdr( ) { return( currentItemAdr ); }

void      SimWinScrollable::setLimitItemAdr( uint32_t adr ) { limitItemAdr = adr; }
uint32_t  SimWinScrollable::getLimitItemAdr( ) { return( limitItemAdr ); }

void      SimWinScrollable::setLineIncrement( uint32_t arg ) { lineIncrement = arg; }
uint32_t  SimWinScrollable::getLineIncrement( ) { return( lineIncrement ); }

//------------------------------------------------------------------------------------------------------------
// The scrollable window inherits from the general window. While the banner part of a window is expected to
// be implemented by the inheriting class, the body is done by this class, which will call the "drawLine"
// method implemented by the inheriting class. The "drawLine" method is passed the current item address
// which is the current line start of the item of whatever the window is displaying. The item address value
// is incremented by the itemsPerLine value each time the drawLine routine is called. The cursor position for
// "drawLine" method call is incremented by the linesPerItem amount. Note that the window system thinks in
// lines. If a window has items that occupy more than one line ( linesPerItem > 1 ), the line count in the
// window needs to be divided by that value.
//
//------------------------------------------------------------------------------------------------------------
void SimWinScrollable::drawBody( ) {
    
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
// ??? unsigned addresses ?
//------------------------------------------------------------------------------------------------------------
void SimWinScrollable::winHome( uint32_t pos ) {
    
    if ( pos > 0 ) {
        
        int itemsPerWindow = ( getRows( ) - 1 ) * lineIncrement;
        
        if ( pos > limitItemAdr - itemsPerWindow ) homeItemAdr = limitItemAdr - itemsPerWindow;
        homeItemAdr       = pos;
        currentItemAdr    = pos;
    }
    else currentItemAdr = homeItemAdr;
}

//------------------------------------------------------------------------------------------------------------
// The "winJump" method moves the starting item address of a window within the boundaries zero and the limit
// address.
//
//------------------------------------------------------------------------------------------------------------
void SimWinScrollable::winJump( uint32_t pos ) {
    
   currentItemAdr = pos;
}

//------------------------------------------------------------------------------------------------------------
// Window move implements the forward / backward moves of a window. The amount is added to the current window
// body position, also making sure that we stay inside the boundaries of the address range for the window.
// If the new position would point beyond the limit address, we set the new item address to limit minus the
// window lines times the line increment. Likewise of the new item address would be less than zero, we
// just set it to zero.
//
//------------------------------------------------------------------------------------------------------------
void SimWinScrollable::winForward( uint32_t amt ) {
    
    if ( amt == 0 ) amt = ( getRows( ) - 1 ) * lineIncrement;
    
    if (((uint64_t) currentItemAdr + amt ) > (uint64_t) limitItemAdr ) {
        
        currentItemAdr = limitItemAdr - (( getRows( ) - 1 ) * lineIncrement );
    }
    else currentItemAdr = currentItemAdr + amt;
}

void SimWinScrollable::winBackward( uint32_t amt ) {
    
    if ( amt == 0 ) amt = ( getRows( ) - 1 ) * lineIncrement;
    
    if ( amt <= currentItemAdr ) {
        
        currentItemAdr = currentItemAdr - amt;
    }
    else currentItemAdr = 0;
}
