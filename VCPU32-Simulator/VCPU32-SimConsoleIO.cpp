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
// size accordingly. The cursor stays here it is.
//
//------------------------------------------------------------------------------------------------------------
void removeChar( char *buf, int *strSize, int *pos ) {
    
    if (( *strSize > 0 ) && ( *strSize == *pos )) {
        
        *strSize    = *strSize - 1;
        *pos        = *pos - 1;
    }
    else if (( *strSize > 0 ) && ( *pos > 0 )) {
   
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
//------------------------------------------------------------------------------------------------------------
void SimConsoleIO::initConsoleIO( bool nonBlocking ) {
    
#if __APPLE__
    struct termios term;
    tcgetattr( fileno( stdin ), &term );
    term.c_lflag &= ~ ( ICANON | ECHO );
    term.c_iflag &= ~IGNBRK;
    term.c_cc[VDISCARD] = _POSIX_VDISABLE;
    term.c_cc[VMIN] = 1;
    term.c_cc[VTIME] = 0;
    tcsetattr( STDIN_FILENO, TCSAFLUSH, &term );
    
    if ( nonBlocking ) {
        
        int flags = fcntl( STDIN_FILENO, F_GETFL, 0 );
        if ( flags == -1 ) {
            
            // ??? error ....
        }
        
        flags = fcntl( STDIN_FILENO, F_SETFL, flags | O_NONBLOCK );
        if ( flags == -1 ) {
            
            // ??? error ....
        }
    }
#endif
    
    nonBlockingEnabled  = nonBlocking;
}

//------------------------------------------------------------------------------------------------------------
// "isConsole" is used by the command interpreter to figure whether we have a true terminal or just read from
// a file.
//
//------------------------------------------------------------------------------------------------------------
bool  SimConsoleIO::isConsole( ) {
    
    return( isatty( fileno( stdin )));
}

void SimConsoleIO::setBlocking( bool enabled ) {
    
    nonBlockingEnabled = enabled;
}

//------------------------------------------------------------------------------------------------------------
// "readConsoleChar" is the single entry point to get a character from the terminal input. On Mac/Linux, this
// is the "read" system call. On windows there is a similar call, which does just return one character at a
// time. When nonBlocking is enabled, the function just returns immediately with either a character or a zero.
//
//------------------------------------------------------------------------------------------------------------
int SimConsoleIO::readChar(  ) {
    
#if __APPLE__
    char ch;
    if ( read( STDIN_FILENO, &ch, 1 ) == 1 ) return( ch );
    else return ( 0 );
#else
    if (_kbhit( )) {
        
        int ch = _getch();
        return ( ch );
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
// ??? which escape sequence should we handle directly ? which to pass on ? how ?
//------------------------------------------------------------------------------------------------------------
int SimConsoleIO::readCmdLine( char *cmdBuf, int initCmdBufLen, int cursorOfs ) {
   
    enum CharType : uint16_t {
        
        CT_NORMAL           = 0,
        CT_ESCAPE           = 1,
        CT_ESCAPE_BRACKET   = 2
    };
    
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
                
            case CT_ESCAPE: {
                
                if ( isLeftBracketChar( ch )) state = CT_ESCAPE_BRACKET;
                else                          state = CT_NORMAL;
                
            } break;
                
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
                else {
                    
                    if ( strSize < CMD_LINE_BUF_SIZE - 1 ) {
                        
                        insertChar( cmdBuf, ch, &strSize, &cursor );
                        if ( isprint( ch )) writeCharAtPos( ch, strSize, cursor + cursorOfs );
                    }
                }
                
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
                        
                        
                    // ??? for a quick test ... it seems the scroll content is lost when it
                    // leaves the screen. Seems to be a bigger issue.
                    //
                    // ??? i would need to implement a command line buffer and lnes are prited from there.
                    // A new output line is simple entered to the output line stack. Then I print from the
                    // current line buffer index backward filling the scroll area screen. The screen is thus
                    // not really a scroll area, bit a scollable command screen.
                        
                    // The functionlaity should be in the Command Window code. But we need to report the
                    // cursor keys pressed...
         
                        
                    case 'A': {
                        
                        writeScrollUp( 1 );
                        state = CT_NORMAL;
                        
                    } break;
                        
                    case 'B': {
                        
                        writeScrollDown( 1 );
                        state = CT_NORMAL;
                        
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
// ??? move all of the print routines here ? benefit ?


//------------------------------------------------------------------------------------------------------------
//
//
// ??? sketch to enable mouse reporting to capature scrolling and clicks...
// ??? scroll up: \033[M<x><y>
// ??? scroll down: \033[M<x><y>
// ??? "x" could be 32 for up and 33 for down... test it...
//------------------------------------------------------------------------------------------------------------
void enableMouseReporting( ) {
    
    printf( "\033[?1003h" ); // Enable basic mouse reporting
    fflush( stdout );
}

void disableMouseReporting( ) {
    
    printf( "\033[?1000l" ); // Enable basic mouse reporting
    fflush( stdout );
}

