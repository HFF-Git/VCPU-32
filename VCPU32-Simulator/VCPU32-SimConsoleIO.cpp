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

#if __APPLE__

struct termios saveTermSetting;

uint16_t toBigEndian16( uint16_t val ) {
    
    return ( __builtin_bswap16( val ));
}

uint16_t toBigEndian32( uint32_t val ) {
    
    return ( __builtin_bswap32( val ));
}

#else

uint16_t toBigEndian16( uint16_t val ) {
    
    return ( _byteswap_ushort( val ));
}

uint16_t toBigEndian32( uint32_t val ) {
    
    return ( _byteswap_ulong( val ));
}

#endif

bool isCarriageReturnChar( int ch ) {
    
    return (( ch == '\n' ) || ( ch == '\r' ));
}

bool isBackSpaceChar( int ch ) {
    
    return (( ch == 8 ) || ( ch == 127 ));
}


// ??? remove character somewhere in the buffer, update pos index.
void removeChar( char *buf, int *pos ) {
    
    if ( *pos > 0 ) {
        
        int    index = *pos - 1;
        size_t len   = strlen( buf );
        
        for ( int i = index; i < len; i++ ) buf[ i ] = buf[ i + 1 ];
        
        *pos = *pos - 1;
    }
}


};

//------------------------------------------------------------------------------------------------------------
// Object constructur. We will save the current terminal settings, just in case.
//
//------------------------------------------------------------------------------------------------------------
SimConsoleIO::SimConsoleIO( ) {
    
    saveConsoleMode( );
}

//------------------------------------------------------------------------------------------------------------
// "isConsole" is used by teh command interpreter to figure whether we have a true terminal or just read from
// a file.
//
//------------------------------------------------------------------------------------------------------------
bool  SimConsoleIO::isConsole( ) {
    
    return( isatty( fileno( stdin )));
}

//------------------------------------------------------------------------------------------------------------
// On Mac/Linux the terminal needs to be set into raw character mode. The following routines will save the
// current settings, set the raw mode attributes, and restore the saved settings. For a windows system, these
// methods are a no operation. There is aso a non-blocking IO mode. When the simulator hands over control to
// the CPU, the console IO is mapped to the PDC console driver. The console IO becomes part of the periodic
// processing and a key pressed will set the flags in the PDC console driver data. We act as "true" hardware.
// Non-blocking mode is enabled on entry to single step and run command and disabled when we are bac to the
// simulator.
//
//
// ??? should we in general work in raw mode ? We handle a lot of character prcessing already....
//
//------------------------------------------------------------------------------------------------------------
void SimConsoleIO::saveConsoleMode( ) {
    
#if __APPLE__
    tcgetattr( fileno( stdin ), &saveTermSetting );
#endif
    
}

void SimConsoleIO::restoreConsoleMode( ) {
    
#if __APPLE__
    tcsetattr( fileno( stdin ), TCSANOW, &saveTermSetting );
#endif
    
    nonBlockingEnabled  = false;
    rawModeEnabled      = false;
}

void SimConsoleIO::setConsoleModeRaw( bool nonBlocking ) {
    
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
        
        fcntl( STDIN_FILENO, F_SETFL, flags | O_NONBLOCK );
        if ( flags == -1 ) {
            
            // ??? error ....
        }
        
        nonBlockingEnabled = true;
    }
#else
    if ( nonBlocking ) nonBlockingEnabled = true;
#endif
    
    rawModeEnabled = true;
}

//------------------------------------------------------------------------------------------------------------
// "readConsoleChar" is the single entry point to get a character from the terminal. On a Mac/Linux, this is
// the "getchar" system call. On windows there is a similar call, which does just return one character at a
// time. When nonBlocking is enabled, the function just returns immediately with either a character or a zero.
//
//------------------------------------------------------------------------------------------------------------
int SimConsoleIO::readChar(  ) {
    
#if __APPLE__
    
    char ch;
    if ( read( STDIN_FILENO, &ch, 1 ) == 1 ) return( ch );
    else return ( 0 );
    
#else
    
    if (_kbhit()) {

        int ch = _getch();
        return (ch);
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



void SimConsoleIO::writeCarriageReturn( ) {
    
    #if __APPLE__
        writeChar( '\n' );
    #else
        writeChar( '\r' );
        writeChar( '\n' );
    #endif
}

//------------------------------------------------------------------------------------------------------------
// "readInputLine" is used by the command line interpreter to get the command. Since we run in raw mode, the
// basic handling of backspace, carriage return, relevant escape sequeunces, etc. needs to be pricessed in
// this routine directly. Characters other than the special chracters are piled up in a local buffer until we
// read in a carriage return. The core is a state machine that examines a character read to anaalyze whether
// this is a special character or sequence. Any "normal" character is just added to the oine buffer. The
// states are as follows:
//
//      CT_NORMAL: got a character, anylyze it.
//      CT_ESCAPE: check the chracters got. If a "[" we need to handle an escape sequence.
//      CT_ESCAPE_BRACKET: analyze the argument after "esp[" input got so far.
//
// When the character is a "\n" or "\r" we got a carriage return. We append a zero to the command line
// input got so far and if in raw mode echo the carriage return.
//
// Character codes 8 or 127 are the backspae character. A backspace operation will erase the character right
// before the position where the line cursor is. Note that the cursor is not necessarily at the end of the
// current input line. It could have been moved with the left/right cursor key to a position somewhere in the
// current command line.
//
// We also have the option of a prefilled command buffer for editing a command line before hitting return.
//
// ??? handle escape sequence.
// ??? cursor left: \033[D
// ??? cursor right: \033[C
// ??? cursor up: \033[A
// ??? cursor down: \033[B
//
// ??? which escape sequence should we handle directly ? which to pass on ? how ?
// ??? we need a kind of state machine to decode the escape seqquences we are interested in ...
//------------------------------------------------------------------------------------------------------------
int SimConsoleIO::readLine( char *cmdBuf, int cmdBufLen ) {
    
    enum CharType : uint16_t {
        
        CT_NORMAL           = 0,
        CT_ESCAPE           = 1,
        CT_ESCAPE_BRACKET   = 2
    };
    
    int         ch      = ' ';
    int         index   = 0;
    CharType    state   = CT_NORMAL;
    
    while ( true ) {
        
        ch = readChar( );
        
        switch ( state ) {
                
            case CT_ESCAPE: {
                
                if ( ch == '[' ) state = CT_ESCAPE_BRACKET;
                else             state = CT_NORMAL;
                
            } break;
                
            case CT_NORMAL: {
                
                if ( ch == 27 ) {
                    
                    state = CT_ESCAPE;
                }
                else if ( isCarriageReturnChar( ch )) {
                    
                    cmdBuf[ index ] = '\0';
                    if ( rawModeEnabled ) writeCarriageReturn( );
                    return ( index );
                }
                else if ( isBackSpaceChar( ch )) {
                    
                    if ( index > 0 ) {
                        
                        // ??? modify the buffer accordingly...
                        
                        index --;
                        
                        if ( rawModeEnabled ) {
                            
                            // ??? we need to echo what we did.
                            // ?? seems to be easier to edit the command line buffer, clear the current line
                            // and just write the new line out again.
                            
                            // removeChar( cmdBuf, <#int *pos#> );
                            // printChars( "%s", "\033[2K\033[G" );
                            // printChars( "%s", cmdBuf );
                            
                            writeChar( '\b' );
                            writeChar( ' ' );
                            writeChar( '\b' );
                        }
                    }
                }
                else {
                    
                    if ( index < CMD_LINE_BUF_SIZE - 1 ) {
                        
                        if ( isprint( ch )) {
                            
                            cmdBuf[ index ] = (char) ch;
                            index ++;
                            if ( rawModeEnabled ) writeChar((char) ch );
                        }
                    }
                    else {
                        
                        cmdBuf[ index ] = '\0';
                        return( -1 );
                    }
                }
                
            } break;
            
            case CT_ESCAPE_BRACKET: {
                
                printf ( "Escape: %c\n", ch ); // fro testing only...
                
                // ??? handle escape sequence.
                // ??? cursor left: \033[D
                // ??? cursor right: \033[C
                // ??? cursor up: \033[A
                // ??? cursor down: \033[B
                
                state = CT_NORMAL;
                
            } break;
        }
    }
}

//------------------------------------------------------------------------------------------------------------
// "printNum" is a little utility function to print out a number with a given radix.
//
//------------------------------------------------------------------------------------------------------------
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

// ??? sketch to enable mouse reporting to capature scrolling and clicks...
// ??? scroll up: \033[M<x><y>
// ??? scroll down: \033[M<x><y>
// ??? "x" could be 32 for up and 33 for down... test it...


void enableMouseReporting( ) {
    
    printf( "\033[?1003h" ); // Enable basic mouse reporting
    fflush( stdout );
}

void disableMouseReporting( ) {
    
    printf( "\033[?1000l" ); // Enable basic mouse reporting
    fflush( stdout );
}

