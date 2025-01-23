//------------------------------------------------------------------------------------------------------------
//
// VCPU32 - A 32-bit CPU - Console IO
//
//------------------------------------------------------------------------------------------------------------
// Console IO is the piece of code that provide a single character interface for the terminal screen. For the
// siumlator running in CPU mode, the charactaers are taken from and place into the virtual console declared
// on the IO space. For the simulator, it is just plain character IO to the terminal screen.
//
// Unfortunately, windows and macs differ. The standard system calls typically buffer the input
// up to the carriage return. To avoid this, the terminal needs to be place in "raw" mode. And this
// is different for the two platforms.
//
// In addition, the
//
//------------------------------------------------------------------------------------------------------------
//
// VCPU32 - A 32-bit CPU - Console IO
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

#ifndef VCPU32_ConsoleIo_h
#define VCPU32_ConsoleIo_h

//------------------------------------------------------------------------------------------------------------
// Mac and Windows know different include files and procedure names for some POSIX routines.
//
//------------------------------------------------------------------------------------------------------------
#if __APPLE__
#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <stdarg.h>
#include <iostream>
#include <fcntl.h>
#else
//#include <unistd.h>
#include <conio.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <stdarg.h>
#include <iostream>
#include <io.h>
#define isatty _isatty
#define fileno _fileno
#define write  _write
#endif

#include "VCPU32-Types.h"
#include "VCPU32-ConsoleIO.h"

//------------------------------------------------------------------------------------------------------------
// Console IO object. The simulator is a character based interface. The typical terminal IO functionality
// such as buffered data input and output needs to be disabled. We run a bare bone console so to speak.There
// are two modes. In the first mode, the simulator runs and all IO is for command lines, windows and so on.
// When control is given to the CPU code, the console IO is mapped to a virtual console configured in the IO
// address space. This interface will also write and read a character at a time.
//
//------------------------------------------------------------------------------------------------------------
struct DrvConsoleIO {
    
    public:
    
    DrvConsoleIO( );
    
    bool    isConsole( );
    int     readChar( );
    void    writeChar( char ch  );
    void    saveConsoleMode( );
    void    restoreConsoleMode( );
    
    void    setConsoleModeRaw( bool nonBlocking = false );
    
    int     readLine( char *cmdBuf );
    int     printNum( uint32_t num, int rdx );
    
    template<typename... Args> int printChars( const char* fmt, Args&&... args );
    
    private:

    char printBuf[ 1024 ];
    bool nonBlockingEnabled = false;
    bool rawModeEnabled     = false;
};

//------------------------------------------------------------------------------------------------------------
// "printChars" is the entry point to printing characters to the console. It is a template function to accept
// different formats and arguments. As such it need to be included in the include file, so that the compiler
// knows what to do to built the actual function calls. Since we had a couple of issues with the window
// drawing functions of the simulator, there is additional code to catch the issue. So far, it did not occur
// again.
//
//------------------------------------------------------------------------------------------------------------
template<typename... Args> int DrvConsoleIO::printChars( const char* fmt, Args&&... args ) {
    
    size_t len = 0;
    
    do {
        
        len = snprintf( printBuf, sizeof( printBuf ), fmt, args... );
       
        if (( len < 0 ) && ( errno != EINTR )) {
            
            fprintf( stderr, "winPrintf (snprintf) error, errno: %d, %s\n", errno, strerror( errno ));
            fflush( stderr );
            exit( errno );
        }
        
    } while ( len < 0 );
    
    if ( len > 0 ) {
        
        for ( int i = 0; i < len; i++  ) writeChar( printBuf[ i ] );
    }
    
    return( static_cast<int>(len));
}

#endif /* VCPU32_ConsoleIo_h */
