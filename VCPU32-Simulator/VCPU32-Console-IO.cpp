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
#include "VCPU32-ConsoleIO.h"
#include "VCPU32-Driver.h"

//------------------------------------------------------------------------------------------------------------
// Local name space.
//
//------------------------------------------------------------------------------------------------------------
namespace {

#if __APPLE__

struct termios saveTermSetting;

#else


#endif

};

//------------------------------------------------------------------------------------------------------------
// Object constructur. We will save the current terminal settings, just in case.
//
//------------------------------------------------------------------------------------------------------------
DrvConsoleIO::DrvConsoleIO( ) {
    
    saveConsoleMode( );
}

//------------------------------------------------------------------------------------------------------------
// "isConsole" is used by teh command interpreter to figure whether we have a true terminal or just read from
// a file.
//
//------------------------------------------------------------------------------------------------------------
bool  DrvConsoleIO::isConsole( ) {
    
    return( isatty( fileno( stdin )));
}

//------------------------------------------------------------------------------------------------------------
// On Mac/Linux the terminal needs to be set into raw chacater mode. The following routines will save the
// current settings, set the raw mode attributes, and restore the saved settings. For a windows system, these
// methods are a no operation.
//
//------------------------------------------------------------------------------------------------------------
void DrvConsoleIO::saveConsoleMode( ) {
    
#if __APPLE__
    tcgetattr( fileno( stdin ), &saveTermSetting );
#endif
    
}

void DrvConsoleIO::setConsoleModeRaw( ) {
    
#if __APPLE__
    struct termios term;
    tcgetattr( fileno( stdin ), &term );
    term.c_lflag &= ~ (ICANON | ECHO );
    term.c_iflag &= ~IGNBRK;
    tcsetattr( fileno( stdin ), TCSANOW, &term );
#endif
    
}

void DrvConsoleIO::resetConsoleMode( ) {
    
#if __APPLE__
    tcsetattr( fileno( stdin ), TCSANOW, &saveTermSetting );
#endif
    
}

//------------------------------------------------------------------------------------------------------------
// "readConsoleChar" is the single entry point to get a character from the terminal. On a Mac/Linux, this is
// the "getchar" system call. On windows there is a similar call, which does just return one character at a
// time.
//
//------------------------------------------------------------------------------------------------------------
int DrvConsoleIO::readChar(  ) {
    
#if __APPLE__
    return( getchar( ));
#else
    return(_getch());
#endif
    
}

//------------------------------------------------------------------------------------------------------------
// "writeConsoleChar" is the single entry point to write to the terminal. On a Mac/Linux, this is the
// "putchar" system call. On windows there is a similar call, which does just prints one character at a
// time.
//
//------------------------------------------------------------------------------------------------------------
void DrvConsoleIO::writeChar( char ch  ) {
    
#if __APPLE__
    putchar( ch );
#else
    _putch( int(ch) );
#endif
    
}

//------------------------------------------------------------------------------------------------------------
// "readInputLine" is used by the command line interpreter to get the command. Since we run in raw mode, the
// basic handling of backspace, carriage return, etc. needs to be handled directly. Characters other than the
// special chracters are piled up in a local buffer until we read in a carriage return. 
//
//------------------------------------------------------------------------------------------------------------
int DrvConsoleIO::readLine( char *cmdBuf ) {
    
    int ch;
    int index = 0;
    
    while ( true ) {
        
        ch = readChar( );
        
        if ( ch == -1 ) {
            
            return( -1 );
        }
        else if ( ch == 25 ) {
            
            // handle control Y, does not work yet ????
            return( -1 );
        }
        else if (( ch == '\n' ) || ( ch == '\r' )) {
            
            cmdBuf[ index ] = '\0';
            writeChar( ch );
            return ( index );
        }
        else if (( ch == 8 ) || ( ch == 127 )) {
            
            if ( index > 0 ) {
                
                index --;
                writeChar( '\b' );
                writeChar( ' ' );
                writeChar( '\b' );
                continue;
            }
        }
        else {
            
            if ( index < CMD_LINE_BUF_SIZE - 1 ) {
                
                if ( isprint( ch )) {
                    
                    cmdBuf[ index ] = (char) ch;
                    index ++;
                    writeChar((char) ch );
                }
            }
            else {
                
                cmdBuf[ index ] = '\0';
                return( -1 );
            }
        }
    }
}

//------------------------------------------------------------------------------------------------------------
//
//
//------------------------------------------------------------------------------------------------------------
int DrvConsoleIO::printNum( uint32_t num, int rdx ) {
    
    if      ( rdx == 10 )  return( printChars( "%d", num ));
    else if ( rdx == 8  )  return( printChars( "%#012o", num ));
    else if ( rdx == 16 )  {
        
        if ( num == 0 ) return( printChars( "0x0" ));
        else return( printChars( "%#010x", num ));
    }
    else return( printChars( "**num**" ));
}

// ??? any other little helpers for printing ?