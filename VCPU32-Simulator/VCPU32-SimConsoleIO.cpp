//------------------------------------------------------------------------------------------------------------
//
// VCPU32 - A 32-bit CPU - Console IO
//
//------------------------------------------------------------------------------------------------------------
// Console IO is the piece of code that provides a single character interface for the terminal screen.  For
// the simulator, it is just plain character IO to the terminal screen.For the simulator running in CPU mode,
// the charactaers are taken from and place into the virtual console declared on the IO space.
//
// Unfortunately, PCs and Macs differ. The standard system calls typically buffer the input up to the carriage
// return. To avoid this, the terminal needs to be place in "raw" mode. And this is different for the two
// platforms.
//
//------------------------------------------------------------------------------------------------------------
//
// VCPU32 - A 32-bit CPU - Main
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
#include "VCPU32-Types.h"
#include "VCPU32-SimConsoleIO.h"
#include "VCPU32-SimDeclarations.h"
#include "VCPU32-SimTables.h"

//------------------------------------------------------------------------------------------------------------
// Local name space.
//
//------------------------------------------------------------------------------------------------------------
namespace {

//------------------------------------------------------------------------------------------------------------
// The previous teminal attribute settings. We restore them when the console IO object is deleted.
//
//------------------------------------------------------------------------------------------------------------
#if __APPLE__
struct termios saveTermSetting;
#endif

char outputBuffer[ 1024 ];

//------------------------------------------------------------------------------------------------------------
//
//
//------------------------------------------------------------------------------------------------------------
#if __APPLE__
uint16_t toBigEndian16( uint16_t val ) { return ( __builtin_bswap16( val )); }
uint16_t toBigEndian32( uint32_t val ) { return ( __builtin_bswap32( val )); }
#else
uint16_t toBigEndian16( uint16_t val ) { return ( _byteswap_ushort( val )); }
uint16_t toBigEndian32( uint32_t val ) { return ( _byteswap_ulong( val )); }
#endif

//------------------------------------------------------------------------------------------------------------
// Little helpers.
//
//------------------------------------------------------------------------------------------------------------
bool isEscapeChar( int ch ) {
    
    return ( ch == 27 );
}

bool isCarriageReturnChar( int ch ) {
    
    return (( ch == '\n' ) || ( ch == '\r' ));
}

bool isBackSpaceChar( int ch ) {
    
    return (( ch == 8 ) || ( ch == 127 ));
}

bool isLeftBracketChar( int ch ) {
    
    return ( ch == '[' );
}

//------------------------------------------------------------------------------------------------------------
// "removeChar" will remove a character from the input buffer at the cursor position and adjust the string
// size accordingly. If the cursor is at the end of the string, both string size and cursor position are
// decremented by one, otherwise the cursor stays where it is and just the string size is decremented.
//
//------------------------------------------------------------------------------------------------------------
void removeChar( char *buf, int *strSize, int *pos ) {
    
    if (( *strSize > 0 ) && ( *strSize == *pos )) {
        
        *strSize    = *strSize - 1;
        *pos        = *pos - 1;
    }
    else if (( *strSize > 0 ) && ( *pos >= 0 )) {
   
        for ( int i = *pos; i < *strSize; i++ ) buf[ i ] = buf[ i + 1 ];
        *strSize    = *strSize - 1;
    }
}

//------------------------------------------------------------------------------------------------------------
// "insertChar" will insert a character in the input buffer at the cursor position and adjust cursor and
// overall string size accordingly. There are twp basic cases. The first is simply appending to the buffer
// when both current string size and cursor position are equal. The second is when the cursor is somewhere
// in the input buffer. In this case we need to shift the characters to the right to make room first.
//
//------------------------------------------------------------------------------------------------------------
void insertChar( char *buf, int ch, int *strSize, int *pos ) {
    
    if ( *pos == *strSize ) {
        
        buf[ *strSize ] = ch;
        *strSize        = *strSize + 1;
        *pos            = *pos + 1;
    }
    else if ( *pos < *strSize ) {
        
        for ( int i = *strSize; i > *pos; i-- ) buf[ i ] = buf[ i -1 ];
        
        buf[ *pos ] = ch;
        *strSize    = *strSize + 1;
        *pos        = *pos + 1;
    }
}

//------------------------------------------------------------------------------------------------------------
// "appendChar" will add a character to the end of the buffer and adjust the overall size.
//
//------------------------------------------------------------------------------------------------------------
void appendChar( char *buf, int ch, int *strSize ) {
    
    buf[ *strSize ] = ch;
    *strSize        = *strSize + 1;
}

}; // namespace


//------------------------------------------------------------------------------------------------------------
// Object constructur. We will save the current terminal settings, just in case.
//
//------------------------------------------------------------------------------------------------------------
SimConsoleIO::SimConsoleIO( ) {
    
#if __APPLE__
    tcgetattr( fileno( stdin ), &saveTermSetting );
#endif
    
}

SimConsoleIO::~SimConsoleIO( ) {
    
#if __APPLE__
    tcsetattr( fileno( stdin ), TCSANOW, &saveTermSetting );
#endif
    
}

//------------------------------------------------------------------------------------------------------------
// The Simulator works in raw character mode. This is to support basic editiing features and IO to the
// simulator console window when teh simulation is active. There is a price to pay in that there is no nice
// buffering of input and basic line editing capabilities. On Mac/Linux the terminal needs to be set into
// raw character mode. On windows, this seems to work without special setups. Hmm, anyway. This routine will
// set the raw mode attributes. For a windows system, these methods are a no operation.
//
// There is also a non-blocking IO mode. When the simulator hands over control to the CPU, the console IO is
// mapped to the PDC console driver and output is directed to the console window. The console IO becomes part
// of the periodic processing and a key pressed will set the flags in the PDC console driver data. We act as
// "true" hardware. Non-blocking mode is enabled on entry to single step and run command and disabled when we
// are back to the simulator.
//
//
// ??? perhaps a place to save the previous setings and restore them ?
// ??? Or just do all this in the object creator ?
//------------------------------------------------------------------------------------------------------------
void SimConsoleIO::initConsoleIO( ) {
    
#if __APPLE__
    struct termios term;
    tcgetattr( fileno( stdin ), &term );
    term.c_lflag &= ~ ( ICANON | ECHO );
    term.c_iflag &= ~IGNBRK;
    term.c_cc[VDISCARD] = _POSIX_VDISABLE;
    term.c_cc[VMIN] = 1;
    term.c_cc[VTIME] = 0;
    tcsetattr( STDIN_FILENO, TCSAFLUSH, &term );
#endif
    
    blockingMode  = true;
}

//------------------------------------------------------------------------------------------------------------
// "isConsole" is used by the command interpreter to figure whether we have a true terminal or just read from
// a file.
//
//------------------------------------------------------------------------------------------------------------
bool  SimConsoleIO::isConsole( ) {
    
    return( isatty( fileno( stdin )));
}

//------------------------------------------------------------------------------------------------------------
// "setBlockingMode" will put the terminal into blocking or non-blocking mode. For the command interpreter we
// will use the blocking mode, i.e. we wait for character input. When the CPU runs, the console IO must be in
// non-blocking, and we check for input on each CPU "tick".
//
//------------------------------------------------------------------------------------------------------------
void SimConsoleIO::setBlockingMode( bool enabled ) {
    
#if __APPLE__
    
    int flags = fcntl( STDIN_FILENO, F_GETFL, 0 );
    if ( flags == -1 ) {
        
        // ??? error ....
    }
    else {
        
        if ( enabled )  flags &= ~O_NONBLOCK;
        else            flags |= O_NONBLOCK;
        
        if ( fcntl( STDIN_FILENO, F_SETFL, flags ) == -1 ) {
               
            // ??? error ....
        }
    }
#endif
    
    blockingMode = enabled;
}

//------------------------------------------------------------------------------------------------------------
// "readConsoleChar" is the single entry point to get a character from the terminal input. On Mac/Linux, this
// is the "read" system call. Whether the mode is blocking or non-blocking is set in the terminal settings.
// The read function is the same. If there is no character available, a zero is returned, otherwise the
// character.
//
// On Windows there is a similar call, which does just return one character at a time. However, there seems
// to be no real waiting function. Instead, the "_kbhit" tests for a keyboard input. In blocking mode, we
// will loop for a keyboard input and then get the character. In non-blocking mode, we test the keyboard and
// return either the character typed or a zero.
//
// ??? this also means on Windows a "busy" loop..... :-(
// ??? perhas a "sleep" eases the busy loop a little...
//------------------------------------------------------------------------------------------------------------
int SimConsoleIO::readChar( ) {
    
#if __APPLE__
    char ch;
    if ( read( STDIN_FILENO, &ch, 1 ) == 1 ) return( ch );
    else return ( 0 );
#else
    if ( blockingMode ) {
        
        while ( ! _kbhit( )) Sleep( 50 );
        return( _getch( ));
    }
    else {
        
        if ( _kbhit( )) {
        
            int ch = _getch();
            return ( ch );
        }
        else return( 0 );
    }
#endif
    
}

//------------------------------------------------------------------------------------------------------------
// "writeConsoleChar" is the single entry point to write to the terminal. On a Mac/Linux, this is the
// "write" system call. On windows there is a similar call, which does just prints one character at a
// time.
//
//------------------------------------------------------------------------------------------------------------
void SimConsoleIO::writeChar( char ch  ) {
    
#if __APPLE__
    write( STDOUT_FILENO, &ch, 1 );
#else
    _putch( int(ch) );
#endif
    
}

//------------------------------------------------------------------------------------------------------------
//
// ??? another version which does not use templates... try it ...
//------------------------------------------------------------------------------------------------------------
int SimConsoleIO::writeChars( const char *format, ... ) {
    
    va_list args;
    
    va_start( args, format );
    int len = vsnprintf( outputBuffer, sizeof( outputBuffer ), format, args );
    va_end(args);
    
    if ( len > 0 ) {
        
        for ( int i = 0; i < len; i++  ) writeChar( outputBuffer[ i ] );
    }
    
    return( len );
}
    
//------------------------------------------------------------------------------------------------------------
// "writeCarriageReturn" will emit a carriage return. This is done slighty different for Macs / Windows.
//
//------------------------------------------------------------------------------------------------------------
void SimConsoleIO::writeCarriageReturn( ) {
    
#if __APPLE__
    writeChar( '\n' );
#else
    writeChar( '\r' );
    writeChar( '\n' );
#endif
    
}

void SimConsoleIO::writeBackSpace( ) {
    
    printChars( "\033[D\033[P" );
}

void SimConsoleIO::writeCursorLeft( ) {
    
    printChars( "\033[D" );
}

void SimConsoleIO::writeCursorRight( ) {
    
    printChars( "\033[C" );
}

void SimConsoleIO::writeScrollUp( int n ) {
    
    printChars( "\033[%dS", n );
}

void SimConsoleIO::writeScrollDown( int n ) {
    
    printChars( "\033[%dT", n );
}

void SimConsoleIO::writeCharAtPos( int ch, int strSize, int pos ) {
    
    if ( pos == strSize ) {
        
        writeChar((char) ch );
    }
    else {
        
        // ??? ugly, we ned to know where the command line really starts
        // terminal line...
        printChars( "\033[%dG\033[1@%c", pos, ch );
    }
}

//------------------------------------------------------------------------------------------------------------
// "readInputLine" is used by the command line interpreter to get the command. Since we run in raw mode, the
// basic handling of backspace, carriage return, relevant escape sequences, etc. needs to be processed in
// this routine directly. Characters other than the special characters are piled up in a local buffer until
// we read in a carriage return. The core is a state machine that examines a character read to anaalyze
// whether this is a special character or sequence. Any "normal" character is just added to the line buffer.
// The states are as follows:
//
//      CT_NORMAL: got a character, anylyze it.
//      CT_ESCAPE: check the characters got. If a "[" we need to handle an escape sequence.
//      CT_ESCAPE_BRACKET: analyze the argument after "esp[" input got so far.
//
// A carriage return character will append a zero to the command line input got so far and if in raw mode echo
// the carriage return. We are done reading the input line.
//
// A backspace character will erase the character right before the position where the line cursor is. Note
// that the cursor is not necessarily at the end of the current input line. It could have been moved with
// the left/right cursor key to a position somewhere in the current command line.
//
// The left and right arrows move the cursor in the command line. Backspacing and inserting will then take
// place at the current cursor position shifting any content to the right of the cursor accordingly.
//
// We also have the option of a prefilled command buffer for editing a command line before hitting return.
// This option is used by the REDO command which lists a previously entered command presented for editing.
//
// Finally, there is the cursor up anddown key. These keys are used to scoll teh command line window. When
// such a key is detected, the current accumulatd input is discarded and replaced by a pseudo command for
// cursor up or down. The rooutne returns immediately.
//
// ??? what if character is zero ?????? ( blocking )
//------------------------------------------------------------------------------------------------------------
int SimConsoleIO::readCmdLine( char *cmdBuf, int initCmdBufLen, int cursorOfs ) {
   
    enum CharType : uint16_t {
        
        CT_NORMAL           = 0,
        CT_ESCAPE           = 1,
        CT_ESCAPE_BRACKET   = 2
    };
    
    char cursorUpStr[ ]     = "WC_CU";
    char cursorDownStr[ ]   = "WC_CD";
    
    
    int         ch      = ' ';
    int         strSize = 0;
    int         cursor  = 0;
    CharType    state   = CT_NORMAL;
    
    if (( initCmdBufLen > 0 ) && ( initCmdBufLen < CMD_LINE_BUF_SIZE - 1 )) {
        
        strSize = initCmdBufLen;
        cursor  = initCmdBufLen;
    }
    
    while ( true ) {
        
        ch = readChar( );
     
        switch ( state ) {
                
            case CT_NORMAL: {
                
                if ( isEscapeChar( ch )) {
                    
                    state = CT_ESCAPE;
                }
                else if ( isCarriageReturnChar( ch )) {
                    
                    appendChar( cmdBuf, 0, &strSize );
                    writeCarriageReturn( );
                    return ( strSize - 1 );
                }
                else if ( isBackSpaceChar( ch )) {
                    
                    if ( strSize > 0 ) {
                        
                        removeChar( cmdBuf, &strSize, &cursor );
                        writeBackSpace( );
                    }
                }
                else if ( ch == 0 ) {
                    
                    // ??? what to do ...
                    
                }
                else {
                    
                    if ( strSize < CMD_LINE_BUF_SIZE - 1 ) {
                        
                        insertChar( cmdBuf, ch, &strSize, &cursor );
                        if ( isprint( ch )) writeCharAtPos( ch, strSize, cursor + cursorOfs );
                    }
                }
                
            } break;
                
            case CT_ESCAPE: {
                
                if ( isLeftBracketChar( ch )) state = CT_ESCAPE_BRACKET;
                else                          state = CT_NORMAL;
                
            } break;
                
            case CT_ESCAPE_BRACKET: {
                
                switch ( ch ) {
                        
                    case 'D': {
                        
                        if ( cursor > 0 ) {
                          
                            cursor --;
                            writeCursorLeft( );
                        }
                        
                        state = CT_NORMAL;
                    
                    } break;
                        
                    case 'C': {
                        
                        if ( cursor < strSize ) {
                          
                            cursor ++;
                            writeCursorRight( );
                        }
                        
                        state = CT_NORMAL;
                        
                    } break;
                        
                    case 'A': {
                        
                        strncpy( cmdBuf, cursorUpStr, strlen( cursorUpStr ));
                        return((int) strlen( cursorUpStr ));
                        
                    } break;
                        
                    case 'B': {
                        
                        strncpy( cmdBuf, cursorDownStr, strlen( cursorDownStr ));
                        return((int) strlen( cursorDownStr ));
                        
                    } break;
                        
                    default: state = CT_NORMAL;
                }
        
            } break;
        }
    }
}

//------------------------------------------------------------------------------------------------------------
// Print routines.
//
//------------------------------------------------------------------------------------------------------------
void SimConsoleIO::clearScreen( ) {
    
    printChars((char *) "\x1b[2J" );
    printChars((char *) "\x1b[3J" );
}

void SimConsoleIO::setAbsCursor( int row, int col ) {
    
    printChars((char *) "\x1b[%d;%dH", row, col );
}

void SimConsoleIO::setWindowSize( int row, int col ) {
    
    printChars((char *) "\x1b[8;%d;%dt", row, col );
}

void SimConsoleIO::setScrollArea( int start, int end ) {
    
    printChars((char *) "\x1b[%d;%dr", start, end );
}

void SimConsoleIO::clearScrollArea( ) {
    
    printChars((char *) "\x1b[r" );
}

int SimConsoleIO::printNum( uint32_t num, int rdx ) {
    
    if      ( rdx == 10 )  return( printChars( "%d", num ));
    else if ( rdx == 8  )  return( printChars( "%#012o", num ));
    else if ( rdx == 16 )  {
        
        if ( num == 0 ) return( printChars( "0x0" ));
        else return( printChars( "%#010x", num ));
    }
    else return( printChars( "**num**" ));
}


// ??? any other little helpers for printing to move here ??
