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

#ifndef VCPU32_ConsoleIo_h
#define VCPU32_ConsoleIo_h

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

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <stdarg.h>
#include <iostream>

#include "VCPU32-Types.h"
#include "VCPU32-ConsoleIo.h"

//------------------------------------------------------------------------------------------------------------
// Console IO object. The simulator is a character based interface. There are two modes. In the first mode,
// the simulator runs and all IO is for command lines, windwos and so on. When control is given to the CPU
// code, the console IO is mapped to a virtual console confugured in the IO address space. This interface
// will write and read a character at a time. The typical terminal IO functionality such as buffered data
// inout and output needs to be disabled. We run a bare bone console so to speak.
//
// The "printf" style writing is using a template for defineing teh function. The implementing code needs to
// be declared in the include file, so that the compiler can use the template where needed. It looks a bit
// ugly, but seems to be the only working way. Since we had a couple of issues with the window drawing
// functions of the simulator, there is additional code to catch the issue. So far, it did not occur again.
//
//------------------------------------------------------------------------------------------------------------
struct DrvConsoleIo {
    
    public:
    
    DrvConsoleIo( );
    
    int     readConsoleChar( );
    void    writeConsoleChar( char ch  );
    void    saveConsoleMode( );
    void    setConsoleModeRaw( );
    void    resetConsoleMode( );
    bool    readInputLine( char *cmdBuf );
    
    template<typename... Args>
    int printfConsole( const char* fmt, Args&&... args ) {
        
        static char buf[ 512 ];
        size_t      len = 0;
        
        do {
            
            len = snprintf( buf, sizeof( buf ), fmt, args... );
           
            if (( len < 0 ) && ( errno != EINTR )) {
                
                fprintf( stderr, "winPrintf (snprintf) error, errno: %d, %s\n", errno, strerror(errno));
                fflush( stdout );
                exit( errno );
            }
            
        } while ( len < 0 );
        
        do {
        
            len = write( fileno( stdout ), buf, len );
            fflush( stdout );
           
            if (( len < 0 ) && ( errno != EINTR )) {
                
                fprintf( stderr, "myPrintf (write) error, errno: %lu, %s\n", len, strerror( errno ));
                fflush( stdout );
                exit( errno );
            }
            
        } while ( len < 0 );
        
        return( static_cast<int>(len));
    }

    private:
    
    struct termios saveTermSetting;
};

#endif /* VCPU32_ConsoleIo_h */
