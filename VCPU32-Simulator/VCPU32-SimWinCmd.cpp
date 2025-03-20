//------------------------------------------------------------------------------------------------------------
//
// VCPU32 - A 32-bit CPU - Simulator command window
//
//------------------------------------------------------------------------------------------------------------
// The command window is the last screen area below all enabled windows displayed. It is actuall not a window
// like the others in that it represents the locked scroll area of the terminal screen. Still, it has a
// window header and a line drawing area. However, the print methods will just emit their data without
// manipluating any window specific cursors like the other window objects. In a sense it is a simple line
// display area. Nevertheless, to enable scrolling of this window, an output buffer needs to be implemented
// that stores all output in a circular buffer to use for text output. Just like a "real" terminal.
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
// A little helper functions.
//
//------------------------------------------------------------------------------------------------------------
void upshiftStr( char *str ) {
    
    size_t len = strlen( str );
    
    if ( len > 0 ) {
        
        for ( size_t i = 0; i < len; i++ ) str[ i ] = (char) toupper((int) str[ i ] );
    }
}

int setRadix( int rdx ) {
    
    return((( rdx == 8 ) || ( rdx == 10 ) || ( rdx == 16 )) ? rdx : 10 );
}

//------------------------------------------------------------------------------------------------------------
// A little helper function to remove the comment part of a command line. We do the changes on the buffer
// passed in by just setting the end of string at the position of the "#" comment indicator.
//
// ??? a bit sloppy. what if the "#" is in a string ?
//------------------------------------------------------------------------------------------------------------
void removeComment( char *cmdBuf ) {
    
    if ( strlen( cmdBuf ) > 0 ) {
        
        char *tmp = strrchr( cmdBuf, '#' );
        if( tmp != nullptr ) *tmp = 0;
    }
}

}; // namespace



// ??? the command window should not directly print to the screen. Instead we need to print to a line
// buffer array to support scrolling in windows mode. In a sense the line mode becomes a windows mode,
// the command window just another window...
//
// ??? the command line buffer is a circular buffer of lines separated by "\n".
//
// ??? we need to ensure that only the command win prints are done with the command win print routine,
// which knows the special details...
//
// ??? a ton of work .....



//************************************************************************************************************
//************************************************************************************************************
//
// Object methods.
//
//************************************************************************************************************
//************************************************************************************************************


//------------------------------------------------------------------------------------------------------------
// Object constructor.
//
//------------------------------------------------------------------------------------------------------------
SimCommandsWin::SimCommandsWin( VCPU32Globals *glb ) : SimWin( glb ) {
    
    this -> glb = glb;
    
    tok     = new SimTokenizer( );
    eval    = new SimExprEvaluator( glb, tok );
    hist    = new SimCmdHistory( );
    winOut  = new SimCmdWinOutBuffer( );
    disAsm  = new SimDisAsm( );
}

//------------------------------------------------------------------------------------------------------------
// Get the command interpreter ready.
//
// One day we will handle command line arguments.... will this be part of the command win ?
//
//  -v           verbose
//  -i <path>    init file
//
// ??? to do ...
//------------------------------------------------------------------------------------------------------------
void SimCommandsWin::setupCmdInterpreter( int argc, const char *argv[ ] ) {
    
    while ( argc > 0 ) {
        
        argc --;
    }
    
    glb -> winDisplay  -> windowDefaults( );
}

//------------------------------------------------------------------------------------------------------------
// The default values are the initial settings when windows is brought up the first time, or for the WDEF
// command.
//
//------------------------------------------------------------------------------------------------------------
void SimCommandsWin::setDefaults( ) {
    
    setRadix( glb -> env -> getEnvVarInt((char *) ENV_RDX_DEFAULT ));
    setRows( 11 );
    setColumns( 80 );
    setDefColumns( 80 );
    setWinType( WT_CMD_WIN );
    setEnable( true );
}

//------------------------------------------------------------------------------------------------------------
// The banner line for command window.
//
//------------------------------------------------------------------------------------------------------------
void SimCommandsWin::drawBanner( ) {
    
    uint32_t fmtDesc = FMT_BOLD | FMT_INVERSE;
    
    setWinCursor( 1, 1 );
    printTextField((char *) "Commands ", fmtDesc );
    padLine( fmtDesc );
}

//------------------------------------------------------------------------------------------------------------
// The body lines of the command window are displayed after the banner line. The window is filled from the
// putput buffer.
//
//------------------------------------------------------------------------------------------------------------
void SimCommandsWin::drawBody( ) {
    
    setFieldAtributes( FMT_DEF_ATTR );
    
    // ??? tons of work to do here....
}



/*

 // ??? to sort out ...

 // Print buffer contents (for debugging)
 void printBuffer(CircularBuffer *cb) {
     printf("Buffer contents:\n");
     int i = cb->tail;
     for (int c = 0; c < cb->count; c++) {
         putchar(cb->buffer[i]);
         i = (i + 1) % BUFFER_SIZE;
     }
     printf("\n");
 }

 // Print a line given a pointer and length
 void printLine(char *line, int length) {
     for (int i = 0; i < length; i++) {
         putchar(line[i]);
     }
     printf("\n");
 }

 int main() {
     CircularBuffer cb;
     initBuffer(&cb);

     printToBuffer(&cb, "Line %d: Hello, %s!\n", 1, "world");
     printToBuffer(&cb, "Line %d: Another line\n", 2);
     printToBuffer(&cb, "Line %d: Testing formatted %s\n", 3, "output");

     printBuffer(&cb);

     int len, foundLines;
     char *ptr = getLinePointer(&cb, 1, &len, &foundLines);
     if (ptr) {
         printf("Line 1 below head (%d chars, %d lines found): ", len, foundLines);
         printLine(ptr, len);
     }

     ptr = getLinePointer(&cb, 3, &len, &foundLines);
     if (ptr) {
         printf("Line 3 below head (%d chars, %d lines found): ", len, foundLines);
         printLine(ptr, len);
     }

     return 0;
 }
 */


//------------------------------------------------------------------------------------------------------------
// Command window output buffer. We cannot directly print to the command window when we want to support
// scrolling of the command window data. Instead, all printing is routed to a command window buffer. The
// buffer is a circular structure, the oldest lines are removed when we need room. Lines are separated by the
// "\n" character. When it comes to printing the window body content, the data is taken from the windows
// ouput buffer.
//
//------------------------------------------------------------------------------------------------------------
SimCmdWinOutBuffer::SimCmdWinOutBuffer( ) { }

void SimCmdWinOutBuffer::initBuffer( ) {
    
    head    = 0;
    tail    = 0;
    count   = 0;
}

//------------------------------------------------------------------------------------------------------------
// When we needroom, the oldest data is removed. We take care that we remove entire lines, even if we would
// need less space for the new data to enter.
//
//------------------------------------------------------------------------------------------------------------
void SimCmdWinOutBuffer::makeRoom(  int requiredSpace ) {
    
    while ( count + requiredSpace > MAX_WIN_OUT_BUFFER_SIZE ) {
        while ( count > 0 && buffer[tail] != '\n') {
            tail = ( tail + 1 ) % MAX_WIN_OUT_BUFFER_SIZE;
            count--;
        }
        if ( count > 0 ) {
            tail = ( tail + 1 ) % MAX_WIN_OUT_BUFFER_SIZE;
            count--;
        }
    }
}

//------------------------------------------------------------------------------------------------------------
// Add new data to the output buffer. When there is not enough room, we will remove the oldest data entered.
// Note that we do not add entire lines, but rather add what ever is entered via the printChar methods. The
// line seprator is explicetly added by the calling routines. If the data to enter is large than the entire
// window putput buffer, we only add the last N characters from the input string. However, given that the
// window out buffer is rather large, this should never really happen.
//
//------------------------------------------------------------------------------------------------------------
void SimCmdWinOutBuffer::addToBuffer( const char *data ) {
    
    int len = (int) strlen( data );
    
    if ( len > MAX_WIN_OUT_BUFFER_SIZE ) {
        
        data += ( len - MAX_WIN_OUT_BUFFER_SIZE );
        len = MAX_WIN_OUT_BUFFER_SIZE;
    }
    
    makeRoom( len );
    
    for ( int i = 0; i < len; i++ ) {
        
        buffer[ head] = data[ i ];
        head = ( head + 1 ) % MAX_WIN_OUT_BUFFER_SIZE;
    }
    
    count += len;
}

//------------------------------------------------------------------------------------------------------------
// "printChars" will add data to the window output buffer. It is modelled after the "printf" family of output
// routines and accepts a fomrat string and a variable number of arguments.
//
//------------------------------------------------------------------------------------------------------------
int SimCmdWinOutBuffer::printChars( const char *format, ... ) {
    
    char    temp[ MAX_WIN_OUT_LINE_SIZE ];
    va_list args;
    
    va_start( args, format );
    int len = vsnprintf( temp, MAX_WIN_OUT_LINE_SIZE, format, args );
    va_end(args);

    if ( len > 0 ) {
        
        if ( len >= MAX_WIN_OUT_LINE_SIZE ) {
            
            len = MAX_WIN_OUT_LINE_SIZE - 1; // Ensure null termination
            temp[ len ] = '\0';
        }
        
        addToBuffer( temp );
        
        // ??? test only .... we fill the buffer and just print out ....
        write( 2, temp, strlen( temp ));
    }
    
    return( len );
}

//------------------------------------------------------------------------------------------------------------
// "getLinePointer" is part of the putput portion. We need a routine that allows to retrieve the last N lines
// to print them to the command window body.
//
//
// ??? to think about, what is the nest wy to display a chunk of lines from the output buffer ?
// ??? there needs to be a cursor to the actual position of the screen subsubset and the number of lines the
// current command window can hist.
//
//------------------------------------------------------------------------------------------------------------
char *SimCmdWinOutBuffer::getLinePointer( int n, int *lineLength, int *lineCount ) {
    
    if ( count == 0 ) {
        
        *lineCount = 0;
        return NULL;  // Buffer is empty
    }

    int index = head;
    int linesFound = 0;
    int lastFoundIndex = tail;  // Default to earliest available line

    while ( index != tail) {
        
        index = ( index - 1 + MAX_WIN_OUT_BUFFER_SIZE ) % MAX_WIN_OUT_BUFFER_SIZE;
        
        if ( buffer[ index ] == '\n' ) {
            
            linesFound++;
            lastFoundIndex = (index + 1 ) % MAX_WIN_OUT_BUFFER_SIZE;
            
            if ( linesFound == n ) break;
        }
    }

    *lineCount = linesFound;
    *lineLength = 0;

    int endIndex = lastFoundIndex;
    
    while ( endIndex != head && buffer[ endIndex ] != '\n' ) {
        
        endIndex = ( endIndex + 1 ) % MAX_WIN_OUT_BUFFER_SIZE;
    }

    *lineLength = ( endIndex >= lastFoundIndex )
                      ? ( endIndex - lastFoundIndex )
                      : ( MAX_WIN_OUT_BUFFER_SIZE - lastFoundIndex + endIndex );

    return &buffer[ lastFoundIndex ];
}

//------------------------------------------------------------------------------------------------------------
// "commandLineError" is a little helper that prints out the error encountered. We will print a caret marker
// where we found the error, and then return a false. Note that the position needs to add the prompt part of
// the command line to where the error was found in the command input.
//
//------------------------------------------------------------------------------------------------------------
void SimCommandsWin::cmdLineError( SimErrMsgId errNum, char *argStr ) {
   
    for ( int i = 0; i < MAX_ERR_MSG_TAB; i++ ) {
        
        if ( errMsgTab[ i ].errNum == errNum ) {
            
            winOut -> printChars( "%s\n", errMsgTab[ i ].errStr );
            return;
        }
    }
    
    winOut -> printChars( "Error: %d", errNum );
    if ( argStr != nullptr )  winOut -> printChars( "%32s", argStr );
    winOut -> printChars( "/n" );
}

//------------------------------------------------------------------------------------------------------------
// "promptYesNoCancel" is a simple function to print a prompt string with a decision question. The answer can
//  be yes/no or cancel. A positive result is a "yes" a negative result a "no", anything else a "cancel".
//
//------------------------------------------------------------------------------------------------------------
int SimCommandsWin::promptYesNoCancel( char *promptStr ) {
    
    int len = winOut -> printChars( "%s -> ", promptStr );
    
    char buf[ 8 ] = "";
    
    if ( glb -> console -> readCmdLine( buf, 0, len ) > 0 ) {
    
        if      (( buf[ 0 ] == 'Y' ) ||  ( buf[ 0 ] == 'y' ))   return( 1 );
        else if (( buf[ 0 ] == 'N' ) ||  ( buf[ 0 ] == 'n' ))   return( -1 );
        else                                                    return( 0 );
    }
    else return( 0 );
}

//------------------------------------------------------------------------------------------------------------
// Token analysis helper functions.
//
//------------------------------------------------------------------------------------------------------------
void SimCommandsWin::checkEOS( ) {
    
    if ( ! tok -> isToken( TOK_EOS )) throw ( ERR_TOO_MANY_ARGS_CMD_LINE );
}

void SimCommandsWin::acceptComma( ) {
    
    if ( tok -> isToken( TOK_COMMA )) tok -> nextToken( );
    else throw ( ERR_EXPECTED_COMMA );
}

void SimCommandsWin::acceptLparen( ) {
    
    if ( tok -> isToken( TOK_LPAREN )) tok -> nextToken( );
    else throw ( ERR_EXPECTED_LPAREN );
}

void SimCommandsWin::acceptRparen( ) {
    
    if ( tok -> isToken( TOK_RPAREN )) tok -> nextToken( );
    else throw ( ERR_EXPECTED_LPAREN );
}

//------------------------------------------------------------------------------------------------------------
// The simulator command interpreter features a simple command history. It is a circular buffer that holds
// the last commands. There are functions to show the command history, re-execute a previous command and to
// retrieve a previous command for editing. The command stack can be accessed with relative command numbers,
// i.e. "current - 3" or by absolute command number, when still present in the history stack.
//
//------------------------------------------------------------------------------------------------------------
SimCmdHistory::SimCmdHistory( ) {
    
    this -> head    = 0;
    this -> tail    = 0;
    this -> count   = 0;
}

//------------------------------------------------------------------------------------------------------------
// Add a command line. If the history buffer is ful, the oldest entry is re-used. The head index points to
// the next entry for allocation.
//
//------------------------------------------------------------------------------------------------------------
void SimCmdHistory::addCmdLine( char *cmdStr ) {
    
    SimCmdHistEntry *ptr = &history[ head ];
    
    ptr -> cmdId = nextCmdNum;
    strncpy( ptr -> cmdLine, cmdStr, 256 );

    if ( count == MAX_CMD_HIST_BUF_SIZE ) tail = ( tail + 1 ) % MAX_CMD_HIST_BUF_SIZE;
    else count++;

    nextCmdNum ++;
    head = ( head + 1 ) % MAX_CMD_HIST_BUF_SIZE;
}

//------------------------------------------------------------------------------------------------------------
// Get a command line from the command history. If the command reference is negative, the entry relativ to
// the top is used. "head - 1" refers to the last entry entered. If the command ID is positive, we search for
// the entry with the matching command id, if still in the history buffer. Optionally, we return the absoolute
// command Id.
//
//------------------------------------------------------------------------------------------------------------
char *SimCmdHistory::getCmdLine( int cmdRef, int *cmdId ) {
    
    if (( cmdRef >= 0 ) && (( nextCmdNum - cmdRef ) > MAX_CMD_HIST_BUF_SIZE )) return ( nullptr );
    if (( cmdRef < 0  ) && ( - cmdRef > nextCmdNum )) return ( nullptr );
    if ( count == 0 ) return ( nullptr );
        
    if ( cmdRef >= 0 ) {
        
        for ( int i = 0; i < count; i++ ) {
        
            int pos = ( tail + i ) % MAX_CMD_HIST_BUF_SIZE;
            if ( history[ pos ].cmdId == cmdRef ) {
                
                if ( cmdId != nullptr ) *cmdId = history[ pos ].cmdId;
                return( history[ pos ].cmdLine );
            }
        }
     
        return( nullptr );
    }
    else {
        
        int pos = ( head + cmdRef + MAX_CMD_HIST_BUF_SIZE) % MAX_CMD_HIST_BUF_SIZE;
        
        if (( pos < head ) && ( pos >= tail )) {
            
            if ( cmdId != nullptr ) *cmdId = history[ pos ].cmdId;
            return history[ pos ].cmdLine;
        }
        else return( nullptr );
    }
}

//------------------------------------------------------------------------------------------------------------
// The command history maintains a command counter, which we return here.
//
//------------------------------------------------------------------------------------------------------------
int SimCmdHistory::getCmdNum( ) {
    
    return( nextCmdNum );
}

int  SimCmdHistory::getCmdCount( ) {
    
    return( count );
}

//------------------------------------------------------------------------------------------------------------
// "displayInvalidWord" shows a set of "*" when we cannot get a value for word. We make the length of the
// "*" string according to the current radix.
//
//------------------------------------------------------------------------------------------------------------
void SimCommandsWin::displayInvalidWord( int rdx ) {
    
    if      ( rdx == 10 )   winOut -> printChars( "**********" );
    else if ( rdx == 8  )   winOut -> printChars( "************" );
    else if ( rdx == 16 )   winOut -> printChars( "**********" );
    else                    winOut -> printChars( "**num**" );
}

//------------------------------------------------------------------------------------------------------------
// "displayWord" lists out a 32-bit machine word in the specified number base. If the format parameter is
// omitted or set to "default", the environment variable for the base number is used.
//
//------------------------------------------------------------------------------------------------------------
void SimCommandsWin::displayWord( uint32_t val, int rdx ) {
    
    if      ( rdx == 10 )  winOut -> printChars( "%10d", val );
    else if ( rdx == 8  )  winOut -> printChars( "%#012o", val );
    else if ( rdx == 16 )  {
        
        if ( val == 0 ) winOut -> printChars( "0x00000000" );
        else winOut -> printChars( "%#010x", val );
    }
    else winOut -> printChars( "**num**" );
}

//------------------------------------------------------------------------------------------------------------
// "displayHalfWord" lists out a 12-bit word in the specified number base. If the format parameter is omitted
// or set to "default", the environment variable for the base number is used.
//
//------------------------------------------------------------------------------------------------------------
void SimCommandsWin::displayHalfWord( uint32_t val, int rdx ) {
    
    if      ( rdx == 10 )  winOut -> printChars( "%5d", val );
    else if ( rdx == 8  )  winOut -> printChars( "%06o", val );
    else if ( rdx == 16 )  {
        
        if ( val == 0 ) winOut -> printChars( "0x0000" );
        else            winOut -> printChars( "%#05x", val );
    }
    else winOut -> printChars( "**num**" );
}

//------------------------------------------------------------------------------------------------------------
// Display absolute memory content. We will show the memory starting with offset. The words per line is an
// environmental variable setting. The offset is rounded down to the next 4-byte boundary, the limit is
// rounded up to the next 4-byte boundary. We display the data in words. The absolute memory address range
// currently consist of three memory objects. There is main physical memory, PDC memory and IO memory. This
// routine will make the appropriate call.
//
//------------------------------------------------------------------------------------------------------------
void  SimCommandsWin::displayAbsMemContent( uint32_t ofs, uint32_t len, int rdx ) {
    
    uint32_t    index           = ( ofs / 4 ) * 4;
    uint32_t    limit           = ((( index + len ) + 3 ) / 4 ) * 4;
    int         wordsPerLine    = glb -> env -> getEnvVarInt((char *) ENV_WORDS_PER_LINE );
    CpuMem      *physMem        = glb -> cpu -> physMem;
    CpuMem      *pdcMem         = glb -> cpu -> pdcMem;
    CpuMem      *ioMem          = glb -> cpu -> ioMem;
    
    while ( index < limit ) {
        
        displayWord( index, rdx );
        winOut -> printChars( ": " );
        
        for ( uint32_t i = 0; i < wordsPerLine; i++ ) {
            
            if ( index < limit ) {
                
                if ((physMem != nullptr ) && ( physMem -> validAdr( index ))) {
                    
                    displayWord( physMem -> getMemDataWord( index ), rdx );
                }
                else if (( pdcMem != nullptr ) && ( pdcMem -> validAdr( index ))) {
                    
                   displayWord( pdcMem -> getMemDataWord( index ), rdx );
                }
                else if (( ioMem != nullptr ) && ( ioMem -> validAdr( index ))) {
                    
                    displayWord( ioMem -> getMemDataWord( index ), rdx );
                }
                else displayInvalidWord( rdx );
            }
                
            winOut -> printChars( " " );
            
            index += 4;
        }
        
        winOut -> printChars( "\n" );
    }
    
    winOut -> printChars( "\n" );
}

//------------------------------------------------------------------------------------------------------------
// Display absolute memory content as code shown in assembler syntax. There is one word per line.
//
//------------------------------------------------------------------------------------------------------------
void  SimCommandsWin::displayAbsMemContentAsCode( uint32_t ofs, uint32_t len, int rdx ) {
    
    uint32_t    index           = ( ofs / 4 ) * 4;
    uint32_t    limit           = ((( index + len ) + 3 ) / 4 );
    CpuMem      *physMem        = glb -> cpu -> physMem;
    CpuMem      *pdcMem         = glb -> cpu -> pdcMem;
    CpuMem      *ioMem          = glb -> cpu -> ioMem;
 
    while ( index < limit ) {
        
        displayWord( index, rdx );
        winOut -> printChars( ": " );
        
        if (( physMem != nullptr ) && ( physMem -> validAdr( index ))) {
            
            disAsm -> displayInstr( physMem -> getMemDataWord( index ), rdx );
        }
        else if (( pdcMem != nullptr ) && ( pdcMem -> validAdr( index ))) {
                
            disAsm -> displayInstr( pdcMem -> getMemDataWord( index ), rdx );
        }
        else if (( ioMem != nullptr ) && ( ioMem -> validAdr( index ))) {
                
            disAsm -> displayInstr( ioMem -> getMemDataWord( index ), rdx );
        }
        else displayInvalidWord( rdx );
        
        winOut -> printChars( "\n" );
            
        index += 4;
    }
       
    winOut -> printChars( "\n" );
}

//------------------------------------------------------------------------------------------------------------
// This routine will print a TLB entry with each field formatted.
//
//------------------------------------------------------------------------------------------------------------
void SimCommandsWin::displayTlbEntry( TlbEntry *entry, int rdx ) {
    
    winOut -> printChars( "[" );
    if ( entry -> tValid( ))            winOut -> printChars( "V" );
    else winOut -> printChars( "v" );
    if ( entry -> tDirty( ))            winOut -> printChars( "D" );
    else winOut -> printChars( "d" );
    if ( entry -> tTrapPage( ))         winOut -> printChars( "P" );
    else winOut -> printChars( "p" );
    if ( entry -> tTrapDataPage( ))     winOut -> printChars( "D" );
    else winOut -> printChars( "d" );
    winOut -> printChars( "]" );
    
    winOut -> printChars( " Acc: (%d,%d,%d)", entry -> tPageType( ), entry -> tPrivL1( ), entry -> tPrivL2( ));
    
    winOut -> printChars( " Pid: " );
    displayHalfWord( entry -> tSegId( ), rdx );
    
    winOut -> printChars( " Vpn-H: " );
    displayWord( entry -> vpnHigh, rdx );
    
    winOut -> printChars( " Vpn-L: " );
    displayWord( entry -> vpnLow, rdx );
    
    winOut -> printChars( " PPN: " );
    displayHalfWord( entry -> tPhysPage( ), rdx  );
}

//------------------------------------------------------------------------------------------------------------
// "displayTlbEntries" displays a set of TLB entries, line by line.
//
//------------------------------------------------------------------------------------------------------------
void SimCommandsWin::displayTlbEntries( CpuTlb *tlb, uint32_t index, uint32_t len, int rdx ) {
    
    if ( index + len <= tlb -> getTlbSize( )) {
        
        for ( uint32_t i = index; i < index + len; i++  ) {
            
            displayWord( i, rdx  );
            winOut -> printChars( ": " );
            
            TlbEntry *ptr = tlb -> getTlbEntry( i );
            if ( ptr != nullptr ) displayTlbEntry( ptr, rdx );
            
            winOut -> printChars( "\n" );
        }
        
    } else winOut -> printChars( "index + len out of range\n" );
}

//------------------------------------------------------------------------------------------------------------
// "displayCacheEntries" displays a list of cache line entries. Since we have a coupe of block sizes and
// perhaps one or more sets, the display is rather complex.
//
//------------------------------------------------------------------------------------------------------------
void SimCommandsWin::displayCacheEntries( CpuMem *cPtr, uint32_t index, uint32_t len, int rdx ) {
    
    uint32_t    blockSets       = cPtr -> getBlockSets( );
    uint32_t    wordsPerBlock   = cPtr -> getBlockSize( ) / 4;
    uint32_t    wordsPerLine    = 4;
    uint32_t    linesPerBlock   = wordsPerBlock / wordsPerLine;
   
    if ( index + len >=  cPtr -> getBlockEntries( )) {
        
        winOut -> printChars( " cache index + len out of range\n" );
        return;
    }
    
    for ( uint32_t lineIndex = index; lineIndex < index + len; lineIndex++  ) {
        
        displayWord( lineIndex, rdx  );
        winOut -> printChars( ": " );
        
        if ( blockSets >= 1 ) {
            
            MemTagEntry *tagPtr     = cPtr -> getMemTagEntry( lineIndex, 0 );
            uint32_t    *dataPtr    = (uint32_t *) cPtr -> getMemBlockEntry( lineIndex, 0 );
            
            winOut -> printChars( "(0)[" );
            if ( tagPtr -> valid )  winOut -> printChars( "V" ); else winOut -> printChars( "v" );
            if ( tagPtr -> dirty )  winOut -> printChars( "D" ); else winOut -> printChars( "d" );
            winOut -> printChars( "] (" );
            displayWord( tagPtr -> tag, rdx );
            winOut -> printChars( ") \n" );
            
            for ( int i = 0; i < linesPerBlock; i++  ) {
                
                winOut -> printChars( "            (" );
                
                for ( int j = 0; j < wordsPerLine; j++ ) {
                    
                    displayWord( dataPtr[ ( i * wordsPerLine ) + j ], rdx );
                    if ( i < 3 ) winOut -> printChars( " " );
                }
                
                winOut -> printChars( ") \n" );
            }
        }
        
        if ( blockSets >= 2 ) {
            
            MemTagEntry *tagPtr     = cPtr -> getMemTagEntry( lineIndex, 0 );
            uint32_t    *dataPtr    = (uint32_t *) cPtr -> getMemBlockEntry( lineIndex, 1 );
            
            winOut -> printChars( "            (1)[" );
            if ( tagPtr -> valid )  winOut -> printChars( "V" ); else winOut -> printChars( "v" );
            if ( tagPtr -> dirty )  winOut -> printChars( "D" ); else winOut -> printChars( "d" );
            winOut -> printChars( "] (" );
            displayWord( tagPtr -> tag, rdx );
            winOut -> printChars( ")\n" );
            
            for ( int i = 0; i < linesPerBlock; i++  ) {
                
                winOut -> printChars( "            (" );
                
                for ( int j = 0; j < wordsPerLine; j++ ) {
                    
                    displayWord( dataPtr[ ( i * wordsPerLine ) + j ], rdx );
                    if ( i < 3 ) winOut -> printChars( " " );
                }
                
                winOut -> printChars( ") \n" );
            }
        }
    }
}

//------------------------------------------------------------------------------------------------------------
// Return the current command entered.
//
//------------------------------------------------------------------------------------------------------------
SimTokId SimCommandsWin::getCurrentCmd( ) {
    
    return( currentCmd );
}

//------------------------------------------------------------------------------------------------------------
// Our friendly welcome message with the actual program version. We also set some of the environment variables
// to an initial value. Especially string variables need to be set as they are not initialized from the
// environment variable table.
//
//------------------------------------------------------------------------------------------------------------
void SimCommandsWin::printWelcome( ) {
    
    glb -> env -> setEnvVar((char *) ENV_EXIT_CODE, 0 );
    
    if ( glb -> console -> isConsole( )) {
        
        winOut -> printChars( "VCPU-32 Simulator, Version: %s, Patch Level: %d\n",
                                      glb -> env -> getEnvVarStr((char *) ENV_PROG_VERSION ),
                                      glb -> env -> getEnvVarStr((char *) ENV_PATCH_LEVEL ));
        
        winOut -> printChars( "Git Branch: %s\n",
                                      glb -> env -> getEnvVarStr((char *) ENV_GIT_BRANCH ));
    }
}

//------------------------------------------------------------------------------------------------------------
// "promptCmdLine" lists out the prompt string. For now this is just a "->". As development goes on the prompt
// string will contain some more info about the current CPU state. The prompt is only printed when the input
// comes from a terminal and not an input file.
//
//------------------------------------------------------------------------------------------------------------
int SimCommandsWin::promptCmdLine( ) {
    
    int len = 0;
    
    if ( glb -> console -> isConsole( )) {
        
        if ( glb -> env -> getEnvVarBool((char *) ENV_SHOW_CMD_CNT )) {
            
            len = winOut -> printChars( "(%i) ", glb -> env -> getEnvVarInt((char *) ENV_CMD_CNT ));
        }
        
        len += winOut -> printChars( "->" );
    }
    
    return( len );
}

//------------------------------------------------------------------------------------------------------------
// "readCmdLine" reads in the command line. The function returns the number of characters read or an error
// code. We loop inside the routine until we receive a valid command line or an EOF. A non-empty command is
// also added to the command history buffer. The routine returns the length of the command line, if empty
// a minus one is returned.
//
//------------------------------------------------------------------------------------------------------------
int SimCommandsWin::readInputLine( char *cmdBuf, int promptLen ) {
  
    int len = glb -> console -> readCmdLine( cmdBuf, 0, promptLen );
    
    // ??? add to win output buffer ?
    // winOut -> addToBuffer( cmdBuf );
   
    if ( len > 0 ) {
            
        removeComment( cmdBuf );
        return( len );
    }
    else return( -1 );
}

//------------------------------------------------------------------------------------------------------------
// "execCmdsFromFile" will open a text file and interpret each line as a command. This routine is used by the
// "XF" command and also as the handler for the program argument option to execute a file before entering
// the command loop.
//
// XF "<filepath>"
//
// ??? which error would we like to report here vs. pass on to outer command loop ?
//------------------------------------------------------------------------------------------------------------
void SimCommandsWin::execCmdsFromFile( char* fileName ) {
    
    char cmdLineBuf[ CMD_LINE_BUF_SIZE ] = "";
    
    try {
        
        if ( strlen( fileName ) > 0 ) {
            
            FILE *f = fopen( fileName, "r" );
            if ( f != nullptr ) {
                
                while ( ! feof( f )) {
                    
                    strcpy( cmdLineBuf, "" );
                    fgets( cmdLineBuf, sizeof( cmdLineBuf ), f );
                    cmdLineBuf[ strcspn( cmdLineBuf, "\r\n" ) ] = 0;
                    
                    if ( glb -> env -> getEnvVarBool((char *) ENV_ECHO_CMD_INPUT )) {
                        
                        winOut -> printChars( "%s\n", cmdLineBuf );
                    }
                    
                    removeComment( cmdLineBuf );
                    evalInputLine( cmdLineBuf );
                }
            }
            else throw( ERR_OPEN_EXEC_FILE );
        }
        else throw ( ERR_EXPECTED_FILE_NAME  );
    }
    
    catch ( SimErrMsgId errNum ) {
        
        switch ( errNum ) {
                
            case ERR_OPEN_EXEC_FILE: {
                
                winOut -> printChars( "Error in opening file: \"%s\"", fileName );
                
            } break;
                
            default: throw ( errNum );
        }
    }
}

//------------------------------------------------------------------------------------------------------------
// Help command. With no arguments, a short help overview is printed. There are commands, widow commands and
// predefined functions.
//
//  help ( cmdId | ‘commands‘ | 'wcommands‘ | ‘wtypes‘ | ‘predefined‘ | 'regset' )
//------------------------------------------------------------------------------------------------------------
void SimCommandsWin::helpCmd( ) {
    
    const char FMT_STR_SUMMARY[ ] = "%-16s%s\n";
    const char FMT_STR_DETAILS[ ] = "%s - %s\n";
    
    if ( tok -> isToken( TOK_EOS )) {
        
        for ( int i = 0; i < MAX_CMD_HELP_TAB; i++ ) {
            
            if ( cmdHelpTab[ i ].helpTypeId == TYP_CMD )
                
                winOut -> printChars( FMT_STR_SUMMARY, cmdHelpTab[ i ].cmdNameStr, cmdHelpTab[ i ].helpStr );
        }
        
        winOut -> printChars( "\n" );
    }
    else if (( tok -> isTokenTyp( TYP_CMD  )) ||
             ( tok -> isTokenTyp( TYP_WCMD )) ||
             ( tok -> isTokenTyp( TYP_WTYP )) ||
             ( tok -> isTokenTyp( TYP_RSET )) ||
             ( tok -> isTokenTyp( TYP_PREDEFINED_FUNC ))) {
        
        if ( tok -> isToken( CMD_SET )) {
            
            for ( int i = 0; i < MAX_CMD_HELP_TAB; i++ ) {
                
                if ( cmdHelpTab[ i ].helpTypeId == TYP_CMD )
                    winOut -> printChars( FMT_STR_SUMMARY, cmdHelpTab[ i ].cmdNameStr, cmdHelpTab[ i ].helpStr );
            }
            
            winOut -> printChars( "\n" );
        }
        else if ( tok -> isToken( WCMD_SET )) {
            
            for ( int i = 0; i < MAX_CMD_HELP_TAB; i++ ) {
                
                if ( cmdHelpTab[ i ].helpTypeId == TYP_WCMD )
                    winOut -> printChars( FMT_STR_SUMMARY, cmdHelpTab[ i ].cmdNameStr, cmdHelpTab[ i ].helpStr );
            }
            
            winOut -> printChars( "\n" );
        }
        else if ( tok -> isToken( REG_SET )) {
            
            for ( int i = 0; i < MAX_CMD_HELP_TAB; i++ ) {
                
                if ( cmdHelpTab[ i ].helpTypeId == TYP_RSET )
                    
                    winOut -> printChars( FMT_STR_SUMMARY, cmdHelpTab[ i ].cmdNameStr, cmdHelpTab[ i ].helpStr );
            }
            
            winOut -> printChars( "\n" );
        }
        else if ( tok -> isToken( WTYPE_SET )) {
            
            for ( int i = 0; i < MAX_CMD_HELP_TAB; i++ ) {
                
                if ( cmdHelpTab[ i ].helpTypeId == TYP_WTYP )
                    
                    winOut -> printChars( FMT_STR_SUMMARY, cmdHelpTab[ i ].cmdNameStr, cmdHelpTab[ i ].helpStr );
            }
            
            winOut -> printChars( "\n" );
        }
        else if ( tok -> isToken( PF_SET )) {
            
            for ( int i = 0; i < MAX_CMD_HELP_TAB; i++ ) {
                
                if ( cmdHelpTab[ i ].helpTypeId == TYP_PREDEFINED_FUNC )
                    winOut -> printChars( FMT_STR_SUMMARY, cmdHelpTab[ i ].cmdNameStr, cmdHelpTab[ i ].helpStr );
            }
            
            winOut -> printChars( "\n" );
        }
        else {
            
            for ( int i = 0; i < MAX_CMD_HELP_TAB; i++ ) {
                
                if ( cmdHelpTab[ i ].helpTokId == tok -> tokId( )) {
                    
                    winOut -> printChars( FMT_STR_DETAILS, cmdHelpTab[ i ].cmdSyntaxStr, cmdHelpTab[ i ].helpStr );
                }
            }
        }
    }
    else throw ( ERR_INVALID_ARG );
}

//------------------------------------------------------------------------------------------------------------
// Exit command. We will exit with the environment variable value for the exit code or the argument value
// in the command. This will be quite useful for test script development.
//
// EXIT <val>
//------------------------------------------------------------------------------------------------------------
void SimCommandsWin::exitCmd( ) {
    
    SimExpr rExpr;
    int  exitVal = 0;
    
    if ( tok -> tokId( ) == TOK_EOS ) {
        
        exitVal = glb -> env -> getEnvVarInt((char *) ENV_EXIT_CODE );
        exit(( exitVal > 255 ) ? 255 : exitVal );
    }
    else {
        
        eval -> parseExpr( &rExpr );
        
        if (( rExpr.typ == TYP_NUM ) && ( rExpr.numVal >= 0 ) && ( rExpr.numVal <= 255 )) {
            
            exit( exitVal );
        }
        else throw ( ERR_INVALID_EXIT_VAL );
    }
}

//------------------------------------------------------------------------------------------------------------
// ENV command. The test driver has a few global environment variables for data format, command count and so
// on. The ENV command list them all, one in particular and also modifies one if a value is specified. If the
// ENV variable dos not exist, it will be allocated with the type of the value. A value of the token NIL will
// remove a user defined variable.
//
//  ENV [ <var> [ <val> ]]",
//------------------------------------------------------------------------------------------------------------
void SimCommandsWin::envCmd( ) {
    
    SimEnv *env = glb -> env;
    
    if ( tok -> tokId( ) == TOK_EOS ) {
        
        env -> displayEnvTable( );
    }
    else if ( tok -> tokTyp( ) == TYP_IDENT ) {
        
        char envName[ MAX_ENV_NAME_SIZE ];
        
        strcpy( envName, tok -> tokStr( ));
        upshiftStr( envName );
        
        tok -> nextToken( );
        if ( tok -> tokId( ) == TOK_EOS ) {
            
            if ( env -> isValid( envName )) env -> displayEnvTableEntry( envName );
            else throw ( ERR_ENV_VAR_NOT_FOUND );
        }
        else {
            
            SimExpr rExpr;
            eval -> parseExpr( &rExpr );
            
            if      ( rExpr.typ == TYP_NUM )        env -> setEnvVar( envName, rExpr.numVal );
            else if ( rExpr.typ == TYP_BOOL )       env -> setEnvVar( envName, rExpr.bVal );
            else if ( rExpr.typ == TYP_STR )        env -> setEnvVar( envName, rExpr.strVal );
            else if ( rExpr.typ == TYP_EXT_ADR )    env -> setEnvVar( envName, rExpr.seg, rExpr.ofs );
            else if (( rExpr.typ == TYP_SYM ) && ( rExpr.tokId == TOK_NIL )) env -> removeEnvVar( envName );
        }
    }
}

//------------------------------------------------------------------------------------------------------------
// Up/Down Cursor handling. The up/down cursor is used to scoll the command window content. They are not
// really commands like the other commands. When the console IO detects a curosr up/down escape sequence, it
// aborts the current input, if any, and returns the up / down cusors commands "WC_CU" / "WC_DN". Their
// decoding leads to the these handler routines.
//
//
//------------------------------------------------------------------------------------------------------------
void SimCommandsWin::cursorUpCmd( ) {
    
    winOut -> printChars( "Cursor Up\n" );
}

void SimCommandsWin::cursorDownCmd( ) {
    
    winOut -> printChars( "Cursor Down\n" );
}

//------------------------------------------------------------------------------------------------------------
// Execute commands from a file command. The actual work is done in the "execCmdsFromFile" routine.
//
// XF "<filename>"
//------------------------------------------------------------------------------------------------------------
void SimCommandsWin::execFileCmd( ) {
    
    if ( tok -> tokTyp( ) == TYP_STR ) {
        
        execCmdsFromFile( tok -> tokStr( ));
    }
    else throw( ERR_EXPECTED_FILE_NAME );
}

//------------------------------------------------------------------------------------------------------------
// Reset command.
//
//  RESET [ ( 'CPU' | 'MEM' | 'STATS' | 'ALL' ) ]
//
// ??? when and what statistics to also reset ?
// ??? what if there is a unified cache outside the CPU ?
// ??? what execution mode will put the CPU ? halted ?
//------------------------------------------------------------------------------------------------------------
void SimCommandsWin::resetCmd( ) {
    
    if ( tok -> isToken( TOK_EOS )) {
        
        glb -> cpu -> reset( );
    }
    else if ( tok -> tokTyp( ) == TYP_SYM ) {
        
        switch( tok -> tokId( )) {
                
            case TOK_CPU: {
                
                glb -> cpu -> reset( );
                
            } break;
                
            case TOK_MEM: {
                
                glb -> cpu -> physMem -> reset( );
                
            } break;
                
            case TOK_STATS: {
                
            } break;
                
            case TOK_ALL: {
                
                glb -> cpu -> reset( );
                glb -> cpu -> physMem -> reset( );
                
            } break;
                
            default: throw ( ERR_INVALID_ARG );
        }
    }
    else throw ( ERR_INVALID_ARG );
}

//------------------------------------------------------------------------------------------------------------
// Run command. The command will just run the CPU until a "halt" instruction is detected.
//
//  RUN
//
// ??? see STEP command for detils on teh console handling.
//------------------------------------------------------------------------------------------------------------
void SimCommandsWin::runCmd( ) {
    
    winOut -> printChars( "RUN command to come ... \n");
}

//------------------------------------------------------------------------------------------------------------
// Step command. The command will execute one instruction. Default is one instruction. There is an ENV
// variable that will set the default to be a single clock step.
//
//  S [ <steps> ] [ , 'I' | 'C' ]
//
//
// ??? we need to handle the console window. It should be enabled before we pass control to the CPU.
// ??? make it the current window, saving the previous current window.
// ??? put the console mode into non-blocking.
// ??? hand over to the CPU.
// ??? on return from the CPU steps, enable blocking mode again and restore the current window.
//------------------------------------------------------------------------------------------------------------
void SimCommandsWin::stepCmd( ) {
    
    SimExpr  rExpr;
    uint32_t numOfSteps = 1;
    
    if ( tok -> tokTyp( ) == TYP_NUM ) {
        
        eval -> parseExpr( &rExpr );
        
        if ( rExpr.typ == TYP_NUM ) numOfSteps = rExpr.numVal;
        else throw ( ERR_EXPECTED_STEPS );
    }
    
    if ( tok -> tokId( ) == TOK_COMMA ) {
        
        tok -> nextToken( );
        if      ( tok -> tokId( ) == TOK_I ) glb -> cpu -> instrStep( numOfSteps );
        else if ( tok -> tokId( ) == TOK_C ) glb -> cpu -> clockStep( numOfSteps );
        else                                        throw ( ERR_INVALID_STEP_OPTION );
    }
    
    checkEOS( );
    
    if ( glb -> env -> getEnvVarBool((char *) ENV_STEP_IN_CLOCKS )) glb -> cpu -> clockStep( 1 );
    else                                                            glb -> cpu -> instrStep( 1 );
}

//------------------------------------------------------------------------------------------------------------
// Write line command.
//
//  W <expr> [ , <rdx> ]
//------------------------------------------------------------------------------------------------------------
void SimCommandsWin::writeLineCmd( ) {
    
    SimExpr  rExpr;
    int      rdx = glb -> env -> getEnvVarInt((char *) ENV_RDX_DEFAULT );
    
    eval -> parseExpr( &rExpr );
    
    if ( tok -> tokId( ) == TOK_COMMA ) {
        
        tok -> nextToken( );
        
        if (( tok -> tokId( ) == TOK_HEX ) ||
            ( tok -> tokId( ) == TOK_OCT ) ||
            ( tok -> tokId( ) == TOK_DEC )) {
            
            rdx = tok -> tokVal( );
            
            tok -> nextToken( );
        }
        else if ( tok -> tokId( ) == TOK_EOS ) {
            
            rdx = glb -> env -> getEnvVarInt((char *) ENV_RDX_DEFAULT );
        }
        else throw ( ERR_INVALID_FMT_OPT );
    }
    
    checkEOS( );
    
    switch ( rExpr.typ ) {
            
        case TYP_BOOL: {
            
            if ( rExpr.bVal == true )   winOut -> printChars( "TRUE\n" );
            else                        winOut -> printChars( "FALSE\n" );
            
        } break;
            
        case TYP_NUM: {
            
            glb -> console -> printNum( rExpr.numVal, rdx );
            winOut -> printChars( "\n" );
            
        } break;
            
        case TYP_STR: {
            
            winOut -> printChars( "\"%s\"\n", rExpr.strVal );
            
        } break;
            
        case TYP_EXT_ADR: {
            
            glb -> console -> printNum( rExpr.seg, rdx );
            winOut -> printChars( "." );
            glb -> console -> printNum( rExpr.ofs, rdx );
            winOut -> printChars( "\n" );
            
        } break;
            
        default: throw (  ERR_INVALID_EXPR );
    }
}

//------------------------------------------------------------------------------------------------------------
// The HIST command displays the command history. Optional, we can only report a certain depth from the top.
//
//  HIST [ depth ]
//
// ??? command numbers are shown as absolute numbers. Have an option to show relative numbers ?
// ??? how about a negatve depth shows relative numbers ?
//------------------------------------------------------------------------------------------------------------
void SimCommandsWin::histCmd( ) {
    
    SimExpr rExpr;
    int     depth = 0;
    int     cmdCount = hist -> getCmdCount( );
    
    if ( tok -> tokId( ) != TOK_EOS ) {
        
        eval -> parseExpr( &rExpr );
        
        if ( rExpr.typ == TYP_NUM ) depth = rExpr.numVal;
        else                        throw ( ERR_INVALID_NUM );
    }
    
    if (( depth == 0 ) || ( depth > cmdCount )) depth = cmdCount;
    
    for ( int i = -depth; i < 0; i++ ) {
        
        int  cmdRef = 0;
        char *cmdLine = hist -> getCmdLine( i, &cmdRef );
        
        if ( cmdLine != nullptr )
            winOut -> printChars( "[%d]: %s\n", cmdRef, cmdLine );
    }
}

//------------------------------------------------------------------------------------------------------------
// Execute a previous command again. The command Id can be an absolute command Id or a top of the command
// history buffer relative command Id. The seöected command is copied to the top of the history buffer and
// then passed to the command interpreter for execution.
//
// DO <cmdNum>
//------------------------------------------------------------------------------------------------------------
void SimCommandsWin::doCmd( ) {
    
    SimExpr rExpr;
    int     cmdId = 0;
    
    if ( tok -> tokId( ) != TOK_EOS ) {
        
        eval -> parseExpr( &rExpr );
        
        if ( rExpr.typ == TYP_NUM ) cmdId = rExpr.numVal;
        else                        throw ( ERR_INVALID_NUM );
    }
    
    char *cmdStr = hist -> getCmdLine( cmdId );
    
    if ( cmdStr != nullptr )    evalInputLine( cmdStr );
    else                        throw( ERR_INVALID_CMD_ID );
}

//------------------------------------------------------------------------------------------------------------
// REDO is almost like DO, except that we retrieve the selected command and put it already into the input
// command line string for the readCmdLine routine. We also print it without a carriaage return. The idea
// is that it can now be edited. The edited command is added to teh history buffer and then executed. The
// REDO command itself is not added to the command history stack. If he cmdNum is omitted, REDO will take
// the last command entered.
//
// REDO <cmdNum>
//------------------------------------------------------------------------------------------------------------
void SimCommandsWin::redoCmd( ) {
    
    SimExpr rExpr;
    int     cmdId = -1;
    
    if ( tok -> tokId( ) != TOK_EOS ) {
        
        eval -> parseExpr( &rExpr );
        
        if ( rExpr.typ == TYP_NUM ) cmdId = rExpr.numVal;
        else                        throw ( ERR_INVALID_NUM );
    }
    
    char *cmdStr = hist -> getCmdLine( cmdId );
    
    if ( cmdStr != nullptr ) {
        
        char tmpCmd[ 256 ];
        strncpy( tmpCmd, cmdStr, sizeof( tmpCmd ));
        
        winOut -> printChars( "%s", tmpCmd );
        if ( glb -> console -> readCmdLine( tmpCmd, (int) strlen( tmpCmd ))) evalInputLine( tmpCmd );
    }
    else throw( ERR_INVALID_CMD_ID );
}

//------------------------------------------------------------------------------------------------------------
// Modify register command. This command modifies a register within a register set.
//
// MR <reg> <val>
//------------------------------------------------------------------------------------------------------------
void SimCommandsWin::modifyRegCmd( ) {
    
    SimTokTypeId    regSetId    = TYP_GREG;
    SimTokId        regId       = TOK_NIL;
    int             regNum      = 0;
    uint32_t        val         = 0;
    SimExpr         rExpr;
    
    if (( tok -> tokTyp( ) == TYP_GREG )        ||
        ( tok -> tokTyp( ) == TYP_SREG )        ||
        ( tok -> tokTyp( ) == TYP_CREG )        ||
        ( tok -> tokTyp( ) == TYP_PSTATE_PREG ) ||
        ( tok -> tokTyp( ) == TYP_FD_PREG )     ||
        ( tok -> tokTyp( ) == TYP_MA_PREG )     ||
        ( tok -> tokTyp( ) == TYP_EX_PREG )     ||
        ( tok -> tokTyp( ) == TYP_IC_L1_REG )   ||
        ( tok -> tokTyp( ) == TYP_DC_L1_REG )   ||
        ( tok -> tokTyp( ) == TYP_UC_L2_REG )   ||
        ( tok -> tokTyp( ) == TYP_ITLB_REG )    ||
        ( tok -> tokTyp( ) == TYP_DTLB_REG )) {
        
        regSetId    = tok -> tokTyp( );
        regId       = tok -> tokId( );
        regNum      = tok -> tokVal( );
        tok -> nextToken( );
    }
    else throw ( ERR_INVALID_REG_ID );
    
    if ( tok -> tokId( ) == TOK_EOS ) throw( ERR_EXPECTED_NUMERIC );
    eval -> parseExpr( &rExpr );
    
    if ( rExpr.typ == TYP_NUM ) val = rExpr.numVal;
    else throw ( ERR_INVALID_NUM );
    
    switch( regSetId ) {
            
        case TYP_GREG:          glb -> cpu -> setReg( RC_GEN_REG_SET, regNum, val );    break;
        case TYP_SREG:          glb -> cpu -> setReg( RC_SEG_REG_SET, regNum, val );    break;
        case TYP_CREG:          glb -> cpu -> setReg( RC_CTRL_REG_SET, regNum, val );   break;
        case TYP_FD_PREG:       glb -> cpu -> setReg( RC_FD_PSTAGE, regNum, val );      break;
        case TYP_MA_PREG:       glb -> cpu -> setReg( RC_MA_PSTAGE, regNum, val );      break;
        case TYP_EX_PREG:       glb -> cpu -> setReg( RC_EX_PSTAGE, regNum, val );      break;
        case TYP_IC_L1_REG:     glb -> cpu -> setReg( RC_IC_L1_OBJ, regNum, val );      break;
        case TYP_DC_L1_REG:     glb -> cpu -> setReg( RC_DC_L1_OBJ, regNum, val );      break;
        case TYP_UC_L2_REG:     glb -> cpu -> setReg( RC_UC_L2_OBJ, regNum, val );      break;
        case TYP_ITLB_REG:      glb -> cpu -> setReg( RC_ITLB_OBJ, regNum, val );       break;
        case TYP_DTLB_REG:      glb -> cpu -> setReg( RC_DTLB_OBJ, regNum, val );       break;
            
        default: throw( ERR_EXPECTED_REG_SET );
    }
}

//------------------------------------------------------------------------------------------------------------
// Display absolute memory command. The memory address is a byte address. The offset address is a byte address,
// the length is measured in bytes, rounded up to the a word size. We accept any address and length and only
// check that the offset plus length does not exceed the address space. The display routines, who will call
// the actual memory object will take care of gaps in the memory address range. The format specifier will
// allow for HEX, OCTAL, DECIMAL and CODE. In the case of the code option, the default number format option
// is used for showing the offset value.
//
//  DA <ofs> [ , <len> [ , <fmt> ]]
//------------------------------------------------------------------------------------------------------------
void SimCommandsWin::displayAbsMemCmd( ) {
    
    SimExpr     rExpr;
    uint32_t    ofs     = 0;
    uint32_t    len     = 4;
    int         rdx     = glb -> env -> getEnvVarInt((char *) ENV_RDX_DEFAULT );
    bool        asCode  = false;
    
    eval -> parseExpr( &rExpr );
    
    if ( rExpr.typ == TYP_NUM ) ofs = rExpr.numVal;
    else throw ( ERR_EXPECTED_START_OFS );
    
    if ( tok -> tokId( ) == TOK_COMMA ) {
        
        tok -> nextToken( );
        
        if ( tok -> isToken( TOK_COMMA )) {
            
            len = 4;
        }
        else {
            
            eval -> parseExpr( &rExpr );
            
            if ( rExpr.typ == TYP_NUM ) len = rExpr.numVal;
            else throw ( ERR_EXPECTED_LEN );
        }
    }
    
    if ( tok -> tokId( ) == TOK_COMMA ) {
        
        tok -> nextToken( );
        
        if (( tok -> tokId( ) == TOK_HEX ) ||
            ( tok -> tokId( ) == TOK_OCT ) ||
            ( tok -> tokId( ) == TOK_DEC )) {
            
            rdx = tok -> tokVal( );
        }
        else if ( tok -> tokId( ) == TOK_CODE ) {
            
            asCode = true;
        }
        else if ( tok -> tokId( ) == TOK_EOS ) {
            
            rdx = glb -> env -> getEnvVarInt((char *) ENV_RDX_DEFAULT );
        }
        else throw ( ERR_INVALID_FMT_OPT );
        
        tok -> nextToken( );
    }
    
    checkEOS( );
    
    if (((uint64_t) ofs + len ) <= UINT32_MAX ) {
        
        if ( asCode ) {
            
            displayAbsMemContentAsCode( ofs, len, glb -> env -> getEnvVarInt((char *) ENV_RDX_DEFAULT ));
        }
        else displayAbsMemContent( ofs, len, rdx );
    }
    else throw ( ERR_OFS_LEN_LIMIT_EXCEEDED );
}

//------------------------------------------------------------------------------------------------------------
// Modify absolute memory command. This command accepts data values for up to eight consecutive locations.
// We also use this command to populate physical memory from a script file.
//
//  MA <ofs> <val>
//------------------------------------------------------------------------------------------------------------
void SimCommandsWin::modifyAbsMemCmd( ) {
    
    SimExpr     rExpr;
    uint32_t    ofs         = 0;
    uint32_t    val         = 0;
    CpuMem      *physMem    = glb -> cpu -> physMem;
    CpuMem      *pdcMem     = glb -> cpu -> pdcMem;
    CpuMem      *ioMem      = glb -> cpu -> ioMem;
    CpuMem      *mem        = nullptr;
    
    eval -> parseExpr( &rExpr );
    
    if ( rExpr.typ == TYP_NUM ) ofs = rExpr.numVal;
    else throw ( ERR_EXPECTED_OFS );
    
    eval -> parseExpr( &rExpr );
    
    if ( rExpr.typ == TYP_NUM ) val = rExpr.numVal;
    else throw ( ERR_INVALID_NUM );
    
    checkEOS( );
    
    if      (( physMem != nullptr ) && ( physMem -> validAdr( ofs ))) mem = physMem;
    else if (( pdcMem  != nullptr ) && ( pdcMem  -> validAdr( ofs ))) mem = pdcMem;
    else if (( ioMem   != nullptr ) && ( ioMem   -> validAdr( ofs ))) mem = ioMem;
    
    if (((uint64_t) ofs + 4 ) > UINT32_MAX ) throw ( ERR_OFS_LEN_LIMIT_EXCEEDED );
    
    mem -> putMemDataWord( ofs, val );
}

//------------------------------------------------------------------------------------------------------------
// Display cache entries command.
//
//  DCA ( 'I' | 'D' | 'U' ) <index> [ , <len> [ , <fmt> ]]
//------------------------------------------------------------------------------------------------------------
void SimCommandsWin::displayCacheCmd( ) {
    
    SimExpr     rExpr;
    CpuMem      *cPtr           = nullptr;
    uint32_t    index           = 0;
    uint32_t    len             = 1;
    int         rdx             = glb -> env -> getEnvVarInt((char *) ENV_RDX_DEFAULT );
    
    if ( tok -> tokId( ) == TOK_I ) {
        
        cPtr = glb -> cpu -> iCacheL1;
        tok -> nextToken( );
    }
    else if ( tok -> tokId( ) == TOK_D ) {
        
        cPtr = glb -> cpu -> dCacheL1;
        tok -> nextToken( );
    }
    else if ( tok -> tokId( ) == TOK_U ) {
        
        if ( glb -> cpu -> uCacheL2 != nullptr ) {
            
            cPtr = glb -> cpu -> uCacheL2;
            tok -> nextToken( );
        }
        else throw ( ERR_CACHE_NOT_CONFIGURED );
    }
    else throw ( ERR_CACHE_TYPE );
    
    eval -> parseExpr( &rExpr );
    
    if ( rExpr.typ == TYP_NUM ) index = rExpr.numVal;
    else throw ( ERR_EXPECTED_NUMERIC );
    
    if ( tok -> tokId( ) == TOK_COMMA ) {
        
        tok -> nextToken( );
        
        if ( tok -> tokId( ) == TOK_COMMA ) {
            
            len = 1;
            tok -> nextToken( );
        }
        else {
        
            eval -> parseExpr( &rExpr );
            
            if ( rExpr.typ == TYP_NUM ) len = rExpr.numVal;
            else throw ( ERR_EXPECTED_NUMERIC );
        }
    }
    
    if ( tok -> tokId( ) == TOK_COMMA ) {
        
        tok -> nextToken( );
        
        if (( tok -> tokId( ) == TOK_HEX ) ||
            ( tok -> tokId( ) == TOK_OCT ) ||
            ( tok -> tokId( ) == TOK_DEC )) {
            
            rdx = tok -> tokVal( );
            tok -> nextToken( );
        }
        else throw ( ERR_INVALID_FMT_OPT );
    }
    
    checkEOS( );
   
    if ( cPtr != nullptr ) {
        
        uint32_t blockEntries = cPtr -> getBlockEntries( );
        
        if (( index > blockEntries ) || ( index + len > blockEntries )) {
            
            throw ( ERR_CACHE_SIZE_EXCEEDED );
        }
        
        if ( len == 0 ) len = blockEntries;
        
        displayCacheEntries( cPtr, index, len, rdx );
        winOut -> printChars( "\n" );
    }
}

//------------------------------------------------------------------------------------------------------------
// Purges a cache line from the cache.
//
//  PCA ('I' | 'D' | 'U' ) <index> [ , <set> [, 'F' ]]",
//------------------------------------------------------------------------------------------------------------
void SimCommandsWin::purgeCacheCmd( ) {
    
    SimExpr     rExpr;
    CpuMem      *cPtr           = nullptr;
    uint32_t    index           = 0;
    uint32_t    set             = 0;
    bool        flush           = false;
    
    if ( tok -> tokId( ) == TOK_I ) {
        
        cPtr = glb -> cpu -> iCacheL1;
        tok -> nextToken( );
    }
    else if ( tok -> tokId( ) == TOK_D ) {
        
        cPtr = glb -> cpu -> dCacheL1;
        tok -> nextToken( );
    }
    else if ( tok -> tokId( ) == TOK_U ) {
        
        if ( glb -> cpu -> uCacheL2 != nullptr ) {
            
            cPtr = glb -> cpu -> uCacheL2;
            tok -> nextToken( );
        }
        else throw ( ERR_CACHE_NOT_CONFIGURED );
    }
    else throw ( ERR_CACHE_TYPE );
    
    eval -> parseExpr( &rExpr );
    
    if ( rExpr.typ == TYP_NUM ) index = rExpr.numVal;
    else throw (ERR_EXPECTED_NUMERIC );
    
    if ( tok -> tokId( ) == TOK_COMMA ) {
        
        tok -> nextToken( );
        eval -> parseExpr( &rExpr );
        
        if ( rExpr.typ == TYP_NUM ) set = rExpr.numVal;
        else throw ( ERR_EXPECTED_NUMERIC );
    }
    
    if ( tok -> tokId( ) == TOK_COMMA ) {
        
        tok -> nextToken( );
        
        if ( tok -> isToken( TOK_F )) flush = true;
        else throw ( ERR_INVALID_ARG );
        
        tok -> nextToken( );
    }
    
    checkEOS( );
    
    if ( cPtr != nullptr ) {
        
        if ( set > cPtr -> getBlockSets( ) - 1 ) throw ( ERR_CACHE_SET_NUM );
        
        MemTagEntry  *tagEntry = cPtr -> getMemTagEntry( index, set );
        if ( tagEntry != nullptr ) {
            
            tagEntry -> valid = false;
        }
        else throw ( ERR_CACHE_PURGE_OP );
    }
}

//------------------------------------------------------------------------------------------------------------
// Display TLB entries command.
//
//  DTLB ( 'I' | 'D' ) <index> [ , <len> [ , <rdx> ]]
//------------------------------------------------------------------------------------------------------------
void SimCommandsWin::displayTLBCmd( ) {
    
    SimExpr     rExpr;
    uint32_t    index       = 0;
    uint32_t    len         = 0;
    uint32_t    tlbSize     = 0;
    CpuTlb      *tPtr       = nullptr;
    int         rdx         = glb -> env -> getEnvVarInt((char *) ENV_RDX_DEFAULT );
    
    if ( tok -> tokId( ) == TOK_I ) {
        
        tlbSize = glb -> cpu -> iTlb -> getTlbSize( );
        tPtr    = glb -> cpu -> iTlb;
        tok -> nextToken( );
    }
    else if ( tok -> tokId( ) == TOK_D ) {
        
        tlbSize = glb -> cpu -> dTlb -> getTlbSize( );
        tPtr    = glb -> cpu -> dTlb;
        tok -> nextToken( );
    }
    else throw ( ERR_TLB_TYPE );
    
    eval -> parseExpr( &rExpr );
    
    if ( rExpr.typ == TYP_NUM ) index = rExpr.numVal;
    else throw ( ERR_EXPECTED_NUMERIC );
    
    if ( tok -> tokId( ) == TOK_COMMA ) {
        
        tok -> nextToken( );
        
        if ( tok -> tokId( ) == TOK_COMMA ) {
            
            len = 1;
            tok -> nextToken( );
        }
        else {
            
            eval -> parseExpr( &rExpr );
            len = rExpr.numVal;
        }
    }
    
    if ( tok -> tokId( ) == TOK_COMMA ) {
        
        tok -> nextToken( );
        
        if (( tok -> tokId( ) == TOK_HEX ) ||
            ( tok -> tokId( ) == TOK_OCT ) ||
            ( tok -> tokId( ) == TOK_DEC )) {
            
            rdx = tok -> tokVal( );
            tok -> nextToken( );
        }
        else throw ( ERR_INVALID_FMT_OPT );
    }
    
    checkEOS( );
    
    if ( len == 0 ) len = tlbSize;
    
    if (( index > tlbSize ) || ( index + len > tlbSize )) throw ( ERR_TLB_SIZE_EXCEEDED );
   
    displayTlbEntries( tPtr, index, len, rdx );
    winOut -> printChars( "\n" );
}

//------------------------------------------------------------------------------------------------------------
// Insert into TLB command.
//
//  ITLB ( 'I' | 'D' ) <extAdr> <argAcc> <argAdr>
//------------------------------------------------------------------------------------------------------------
void SimCommandsWin::insertTLBCmd( ) {
    
    SimExpr     rExpr;
    CpuTlb      *tPtr           = nullptr;
    uint32_t    seg             = 0;
    uint32_t    ofs             = 0;
    uint32_t    argAcc          = 0;
    uint32_t    argAdr          = 0;
    
    if ( tok -> tokId( ) == TOK_I ) {
        
        tPtr = glb -> cpu -> iTlb;
        tok -> nextToken( );
    }
    else if ( tok -> tokId( ) == TOK_D ) {
        
        tPtr = glb -> cpu -> dTlb;
        tok -> nextToken( );
    }
    else throw ( ERR_TLB_TYPE );
    
    eval -> parseExpr( &rExpr );
    
    if ( rExpr.typ == TYP_EXT_ADR ) {
        
        seg = rExpr.seg;
        ofs = rExpr.ofs;
    }
    else throw ( ERR_EXPECTED_EXT_ADR );
    
    eval -> parseExpr( &rExpr );
    
    if ( rExpr.typ == TYP_NUM ) argAcc = rExpr.numVal;
    else throw ( ERR_TLB_ACC_DATA );
    
    eval -> parseExpr( &rExpr );
    
    if ( rExpr.typ == TYP_NUM ) argAcc = rExpr.numVal;
    else throw ( ERR_TLB_ADR_DATA );
    
    if ( ! tPtr -> insertTlbEntryData( seg, ofs, argAcc, argAdr )) throw ( ERR_TLB_INSERT_OP );
}

//------------------------------------------------------------------------------------------------------------
// Purge from TLB command.
//
//  PTLB ( 'I' | 'D' ) <extAdr>"
//------------------------------------------------------------------------------------------------------------
void SimCommandsWin::purgeTLBCmd( ) {
    
    SimExpr     rExpr;
    CpuTlb      *tPtr = nullptr;
    
    if ( tok -> tokId( ) == TOK_I ) {
        
        tPtr = glb -> cpu -> iTlb;
        tok -> nextToken( );
    }
    else if ( tok -> tokId( ) == TOK_D ) {
        
        tPtr = glb -> cpu -> dTlb;
        tok -> nextToken( );
    }
    else throw ( ERR_TLB_TYPE );
    
    eval -> parseExpr( &rExpr );
    
    if ( rExpr.typ == TYP_EXT_ADR ) {
        
        if ( ! tPtr -> purgeTlbEntryData( rExpr.seg, rExpr.ofs )) throw ( ERR_TLB_PURGE_OP );
    }
    else throw ( ERR_EXPECTED_EXT_ADR );
}

//------------------------------------------------------------------------------------------------------------
// Global windows commands. There are handlers for turning windows on, off and set them back to their default
// values. We also support two stacks of windows next to each other.
//
//------------------------------------------------------------------------------------------------------------
void SimCommandsWin::winOnCmd( ) {
    
    winModeOn = true;
    glb -> winDisplay -> windowsOn( );
    glb -> winDisplay -> reDraw( true );
}

void SimCommandsWin::winOffCmd( ) {
    
    if ( winModeOn ) {
        
        winModeOn = false;
        glb -> winDisplay -> windowsOff( );
    }
    else throw ( ERR_NOT_IN_WIN_MODE );
}

void SimCommandsWin::winDefCmd( ) {
    
    if ( winModeOn ) {
        
        glb -> winDisplay -> windowDefaults( );
        glb -> winDisplay -> reDraw( true );
    }
    else throw ( ERR_NOT_IN_WIN_MODE );
}

void SimCommandsWin::winStacksEnable( ) {
    
    if ( winModeOn ) {
        
        glb -> winDisplay -> winStacksEnable( true );
        glb -> winDisplay -> reDraw( true );
    }
    else throw ( ERR_NOT_IN_WIN_MODE );
}

void SimCommandsWin::winStacksDisable( ) {
    
    if ( winModeOn ) {
        
        glb -> winDisplay -> winStacksEnable( false );
        glb -> winDisplay -> reDraw( true );
    }
    else throw ( ERR_NOT_IN_WIN_MODE );
}

//------------------------------------------------------------------------------------------------------------
// Windows enable and disable. When enabled, a window does show up on the screen. The window number is
// optional, used for user definable windows.
//
//  <win>E [ <winNum> ]
//  <win>D [ <winNum> ]
//------------------------------------------------------------------------------------------------------------
void SimCommandsWin::winEnableCmd( SimTokId winCmd ) {
    
    int winNum = 0;
    
    if ( ! winModeOn ) throw ( ERR_NOT_IN_WIN_MODE );
    
    if ( tok -> tokId( ) != TOK_EOS ) {
        
        SimExpr rExpr;
        eval -> parseExpr( &rExpr );
        
        if ( rExpr.typ == TYP_NUM ) winNum = rExpr.numVal;
        else throw ( ERR_EXPECTED_WIN_ID );
    }
    
    if ( glb -> winDisplay -> validWindowNum( winNum )) {
        
        glb -> winDisplay -> windowEnable( winCmd, winNum, true );
        glb -> winDisplay -> reDraw( true );
    }
    else throw ( ERR_INVALID_WIN_ID );
}

void SimCommandsWin::winDisableCmd( SimTokId winCmd ) {
    
    int winNum = 0;
    
    if ( ! winModeOn ) throw ( ERR_NOT_IN_WIN_MODE );
    
    if ( tok -> tokId( ) != TOK_EOS ) {
        
        SimExpr rExpr;
        eval -> parseExpr( &rExpr );
        
        if ( rExpr.typ == TYP_NUM ) winNum = rExpr.numVal;
        else throw ( ERR_EXPECTED_WIN_ID );
    }
    
    if ( glb -> winDisplay -> validWindowNum( winNum )) {
        
        glb -> winDisplay -> windowEnable( winCmd, winNum, false );
        glb -> winDisplay -> reDraw( true );
    }
    else throw ( ERR_INVALID_WIN_ID );
}

//------------------------------------------------------------------------------------------------------------
// Windows radix. This command sets the radix for a given window. We parse the command and the format
// option and pass the tokens to the screen handler. The window number is optional, used for user definable
// windows.
//
//  <win>R [ <radix> [ "," <winNum>]]
//------------------------------------------------------------------------------------------------------------
void SimCommandsWin::winSetRadixCmd( SimTokId winCmd ) {
    
    if ( ! winModeOn ) throw ( ERR_NOT_IN_WIN_MODE );
    
    SimExpr rExpr;
    int     winNum  = 0;
    int     rdx     =  glb -> env -> getEnvVarInt((char *) ENV_RDX_DEFAULT );
    
    if ( tok -> isToken( TOK_EOS )) {
        
        glb -> winDisplay -> windowRadix( winCmd, rdx, winNum );
        return;
    }
    
    if ( tok -> tokId( ) == TOK_COMMA ) {
        
        rdx = glb -> env -> getEnvVarInt((char *) ENV_RDX_DEFAULT );
        tok -> nextToken( );
    }
    else {
        
        if ( tok -> isToken( TOK_OCT ))      rdx = 8;
        else if ( tok -> isToken( TOK_DEC )) rdx = 10;
        else if ( tok -> isToken( TOK_HEX )) rdx = 16;
        else {
            
            eval -> parseExpr( &rExpr );
            if ( rExpr.typ == TYP_NUM ) rdx = ::setRadix( rExpr.numVal );
            else throw ( ERR_INVALID_RADIX );
        }
    }
    
    if ( tok -> tokId( ) == TOK_COMMA ) {
        
        tok -> nextToken( );
        eval -> parseExpr( &rExpr );
        
        if ( rExpr.typ == TYP_NUM ) {
            
            winNum = rExpr.numVal;
            tok -> nextToken( );
        }
        else throw ( ERR_INVALID_WIN_ID );
    }
    
    if ( ! glb -> winDisplay -> validWindowNum( winNum )) throw ( ERR_INVALID_WIN_ID );
        
    glb -> winDisplay -> windowRadix( winCmd, rdx, winNum );
}

//------------------------------------------------------------------------------------------------------------
// Window scrolling. This command advances the item address of a scrollable window by the number of lines
// multiplied by the number of items on a line forward or backward. The meaning of the item address and line
// items is window dependent. If the amoun is zero, the default value of the window will be used. The window
// number is optional, used for user definable windows. If omitted, we mean the current window.
//
//  <win>F [ <amt> [ , <winNum> ]]
//  <win>B [ <amt> [ , <winNum> ]]
//------------------------------------------------------------------------------------------------------------
void SimCommandsWin::winForwardCmd( SimTokId winCmd ) {
    
    SimExpr rExpr;
    int     winItems = 0;
    int     winNum   = 0;
    
    if ( ! winModeOn ) throw ( ERR_NOT_IN_WIN_MODE );
    
    if ( tok -> tokId( ) == TOK_EOS ) {
        
        glb -> winDisplay -> windowForward( winCmd, winItems, winNum  );
        return;
    }
    else {
        
        eval -> parseExpr( &rExpr );
        
        if ( rExpr.typ == TYP_NUM ) winItems = rExpr.numVal;
        else throw ( ERR_INVALID_NUM );
        
        if ( tok -> tokId( ) == TOK_COMMA ) {
            
            tok -> nextToken( );
            eval -> parseExpr( &rExpr );
            
            if ( rExpr.typ == TYP_NUM ) winNum = rExpr.numVal;
            else throw ( ERR_INVALID_WIN_ID );
        }
        else winNum = 0;
        
        checkEOS( );
        
        if ( ! glb -> winDisplay -> validWindowNum( winNum )) throw ( ERR_INVALID_WIN_ID );
        
        glb -> winDisplay -> windowForward( winCmd, winItems, winNum  );
    }
}

void SimCommandsWin::winBackwardCmd( SimTokId winCmd ) {
    
    SimExpr rExpr;
    int     winItems = 0;
    int     winNum   = 0;
    
    if ( ! winModeOn ) throw ( ERR_NOT_IN_WIN_MODE );
    
    if ( tok -> tokId( ) == TOK_EOS ) {
        
        glb -> winDisplay -> windowBackward( winCmd, winItems, winNum  );
        return;
    }
    
    eval -> parseExpr( &rExpr );
    
    if ( rExpr.typ == TYP_NUM ) winItems = rExpr.numVal;
    else throw ( ERR_INVALID_NUM );
    
    if ( tok -> tokId( ) == TOK_COMMA ) {
        
        eval -> parseExpr( &rExpr );
        
        if ( rExpr.typ == TYP_NUM ) winItems = rExpr.numVal;
        else throw ( ERR_INVALID_NUM );
        
        if ( tok -> tokId( ) == TOK_COMMA ) {
            
            tok -> nextToken( );
            eval -> parseExpr( &rExpr );
            
            if ( rExpr.typ == TYP_NUM ) winNum = rExpr.numVal;
            else throw ( ERR_INVALID_WIN_ID );
        }
        else winNum = 0;
    }
        
    checkEOS( );
    
    if ( ! glb -> winDisplay -> validWindowNum( winNum )) throw ( ERR_INVALID_WIN_ID );
    
    glb -> winDisplay -> windowBackward( winCmd, winItems, winNum  );
}

//------------------------------------------------------------------------------------------------------------
// Window home. Each window has a home item address, which was set at window creation or trough a non-zero
// value previously passed to this command. The command sets the window item address to this value. The
// meaning of the item address is window dependent. The window number is optional, used for user definable
// windows.
//
//  <win>H [ <pos> [ "," <winNum> ]]
//------------------------------------------------------------------------------------------------------------
void SimCommandsWin::winHomeCmd( SimTokId winCmd ) {
    
    SimExpr rExpr;
    int     winPos   = 0;
    int     winNum   = 0;
    
    if ( ! winModeOn ) throw ( ERR_NOT_IN_WIN_MODE );
    
    if ( tok -> tokId( ) == TOK_EOS ) {
        
        glb -> winDisplay -> windowHome( winCmd, winPos, winNum  );
        return;
    }
    
    eval -> parseExpr( &rExpr );
    
    if ( rExpr.typ == TYP_NUM ) winPos = rExpr.numVal;
    else throw ( ERR_INVALID_NUM );
    
    if ( tok -> tokId( ) == TOK_COMMA ) {
        
        tok -> nextToken( );
        eval -> parseExpr( &rExpr );
        
        if ( rExpr.typ == TYP_NUM ) winNum = rExpr.numVal;
        else throw ( ERR_INVALID_WIN_ID );
    }
    else winNum = 0;
    
    checkEOS( );
    
    if ( ! glb -> winDisplay -> validWindowNum( winNum )) throw ( ERR_INVALID_WIN_ID );
    
    glb -> winDisplay -> windowHome( winCmd, winPos, winNum  );
}

//------------------------------------------------------------------------------------------------------------
// Window jump. The window jump command sets the item address to the position argument. The meaning of the
// item address is window dependent. The window number is optional, used for user definable windows.
//
//  <win>J [ <pos> [ "," <winNum> ]]
//------------------------------------------------------------------------------------------------------------
void SimCommandsWin::winJumpCmd( SimTokId winCmd ) {
    
    SimExpr rExpr;
    int     winPos   = 0;
    int     winNum   = 0;
    
    if ( ! winModeOn ) throw ( ERR_NOT_IN_WIN_MODE );
    
    if ( tok -> tokId( ) == TOK_EOS ) {
        
        glb -> winDisplay -> windowHome( winCmd, winPos, winNum  );
        return;
    }
    
    eval -> parseExpr( &rExpr );
    
    if ( rExpr.typ == TYP_NUM ) winPos = rExpr.numVal;
    else throw ( ERR_INVALID_NUM );
    
    if ( tok -> tokId( ) == TOK_COMMA ) {
        
        tok -> nextToken( );
        eval -> parseExpr( &rExpr );
        
        if ( rExpr.typ == TYP_NUM ) winNum = rExpr.numVal;
        else throw ( ERR_INVALID_WIN_ID );
    }
    else winNum = 0;
    
    checkEOS( );
    
    if ( ! glb -> winDisplay -> validWindowNum( winNum )) throw ( ERR_INVALID_WIN_ID );
    
    glb -> winDisplay -> windowJump( winCmd, winPos, winNum  );
}

//------------------------------------------------------------------------------------------------------------
// Set window lines. This command sets the the number of rows for a window. The number includes the banner
// line. If the "lines" argument is omitted, the window default value will be used. The window number is
// optional, used for user definable windows.
//
//  <win>L [ <lines> [ "," <winNum> ]]
//------------------------------------------------------------------------------------------------------------
void SimCommandsWin::winSetRowsCmd( SimTokId winCmd ) {
    
    SimExpr rExpr;
    int     winLines    = 0;
    int     winNum      = 0;
    
    if ( ! winModeOn ) throw ( ERR_NOT_IN_WIN_MODE );
    
    if ( tok -> tokId( ) == TOK_EOS ) {
        
        glb -> winDisplay -> windowHome( winCmd, winLines, winNum  );
        return;
    }
    
    eval -> parseExpr( &rExpr );
    
    if ( rExpr.typ == TYP_NUM ) winLines = rExpr.numVal;
    else throw ( ERR_INVALID_NUM );
    
    if ( tok -> tokId( ) == TOK_COMMA ) {
        
        tok -> nextToken( );
        eval -> parseExpr( &rExpr );
        
        if ( rExpr.typ == TYP_NUM ) winNum = rExpr.numVal;
        else throw ( ERR_INVALID_WIN_ID );
    }
    else winNum = 0;
    
    checkEOS( );
    
    if ( ! glb -> winDisplay -> validWindowNum( winNum )) throw ( ERR_INVALID_WIN_ID );
    
    glb -> winDisplay -> windowSetRows( winCmd, winLines, winNum );
    glb -> winDisplay -> reDraw( true );
}

//------------------------------------------------------------------------------------------------------------
// Window current command. User definable windows are controlled by their window number. To avoid typing this
// number all the time for a user window command, a user window can explicitly be set as the current command.
//
//  WC <winNum>
//------------------------------------------------------------------------------------------------------------
void SimCommandsWin::winCurrentCmd( ) {
    
    if ( ! winModeOn ) throw ( ERR_NOT_IN_WIN_MODE );
    
    SimExpr     rExpr;
    uint32_t    winNum = 0;
    
    if ( tok -> isToken( TOK_EOS )) throw ( ERR_EXPECTED_WIN_ID );
    
    eval -> parseExpr( &rExpr );
    
    if ( rExpr.typ == TYP_NUM ) winNum = rExpr.numVal;
    else throw ( ERR_INVALID_WIN_ID );
    
    if ( ! glb -> winDisplay -> validWindowNum( rExpr.numVal )) throw ( ERR_INVALID_WIN_ID );
            
    glb -> winDisplay -> windowCurrent( rExpr.numVal );
     
    checkEOS( );
}
    
//------------------------------------------------------------------------------------------------------------
// This command toggles through alternate window content, if supported by the window. An example is the
// cache sets in a two-way associative cache. The toggle command will just flip through the sets.
//
//  WT [ <winNum> ]
//------------------------------------------------------------------------------------------------------------
void  SimCommandsWin::winToggleCmd( ) {
    
    if ( ! winModeOn ) throw ( ERR_NOT_IN_WIN_MODE );
    
    SimExpr     rExpr;
    uint32_t    winNum = 0;
    
    if ( tok -> isToken( TOK_EOS )) {
        
        glb -> winDisplay -> windowToggle( glb -> winDisplay -> getCurrentUserWindow( ));
        return;
    }
    
    eval -> parseExpr( &rExpr );
    
    if ( rExpr.typ == TYP_NUM ) winNum = rExpr.numVal;
    else throw ( ERR_INVALID_WIN_ID );
    
    if ( ! glb -> winDisplay -> validWindowNum( winNum )) throw ( ERR_INVALID_WIN_ID );
    
    glb -> winDisplay -> windowToggle( tok -> tokVal( ));
}

//------------------------------------------------------------------------------------------------------------
// This command exchanges the current user window with the user window specified. It allows to change the
// order of the user windows in a stacks.
//
// WX <winNum>
//------------------------------------------------------------------------------------------------------------
void SimCommandsWin::winExchangeCmd( ) {
    
    if ( ! winModeOn ) throw ( ERR_NOT_IN_WIN_MODE );
    
    SimExpr     rExpr;
    uint32_t    winNum = 0;
    
    if ( tok -> isToken( TOK_EOS )) throw ( ERR_EXPECTED_WIN_ID );
    
    eval -> parseExpr( &rExpr );
    
    if ( rExpr.typ == TYP_NUM ) winNum = rExpr.numVal;
    else throw ( ERR_INVALID_WIN_ID );
    
    checkEOS( );
    
    if ( ! glb -> winDisplay -> validWindowNum( winNum )) throw ( ERR_INVALID_WIN_ID );

    glb -> winDisplay -> windowExchangeOrder( tok -> tokVal( ));
}

//------------------------------------------------------------------------------------------------------------
// This command creates a new user window. The window is assigned a free index form the windows list. This
// index is used in all the calls to this window. The window type allows to select from a code window, a
// physical memory window, a TLB and a CACHE window.
//
//  WN <winType> [ , <arg> ]
//------------------------------------------------------------------------------------------------------------
void SimCommandsWin::winNewWinCmd( ) {
    
    SimTokId   winType = TOK_NIL;
    char    *argStr = nullptr;
    
    if ( ! winModeOn ) throw ( ERR_NOT_IN_WIN_MODE );
   
    if ( tok -> tokTyp( ) == TYP_SYM ) {
        
        winType = tok -> tokId( );
        
        if ((( winType == TOK_PM  ) && ( glb -> cpu -> physMem   == nullptr ))     ||
            (( winType == TOK_PC  ) && ( glb -> cpu -> physMem   == nullptr ))     ||
            (( winType == TOK_MCR ) && ( glb -> cpu -> physMem   == nullptr ))     ||
            (( winType == TOK_IT  ) && ( glb -> cpu -> iTlb      == nullptr ))     ||
            (( winType == TOK_ITR ) && ( glb -> cpu -> iTlb      == nullptr ))     ||
            (( winType == TOK_DT  ) && ( glb -> cpu -> dTlb      == nullptr ))     ||
            (( winType == TOK_DTR ) && ( glb -> cpu -> dTlb      == nullptr ))     ||
            (( winType == TOK_IC  ) && ( glb -> cpu -> iCacheL1  == nullptr ))     ||
            (( winType == TOK_ICR ) && ( glb -> cpu -> iCacheL1  == nullptr ))     ||
            (( winType == TOK_DC  ) && ( glb -> cpu -> dCacheL1  == nullptr ))     ||
            (( winType == TOK_DCR ) && ( glb -> cpu -> dCacheL1  == nullptr ))     ||
            (( winType == TOK_UC  ) && ( glb -> cpu -> uCacheL2  == nullptr ))     ||
            (( winType == TOK_UCR ) && ( glb -> cpu -> uCacheL2  == nullptr ))) {
            
            throw ( ERR_WIN_TYPE_NOT_CONFIGURED );
        }
        
        if ( ! glb -> winDisplay -> validUserWindowType( winType ))
            throw ( ERR_INVALID_WIN_TYPE );
        
        tok -> nextToken( );
    }
    else throw ( ERR_EXPECTED_WIN_ID );
    
    if ( tok -> tokId( ) == TOK_COMMA ) {
        
        tok -> nextToken( );
        
        if ( tok -> tokTyp( ) == TYP_STR ) argStr = tok -> tokStr( );
        else throw ( ERR_INVALID_ARG );
    }
    
    checkEOS( );
    
    glb -> winDisplay -> windowNew( winType, argStr );
    glb -> winDisplay -> reDraw( true );
}

//------------------------------------------------------------------------------------------------------------
// This command removes  a user defined window or window range from the list of windows. A number of -1 will
// kill all user defined windows.
//
//  WK [ <winNumStart> [ "," <winNumEnd]] || ( -1 )
//------------------------------------------------------------------------------------------------------------
void SimCommandsWin::winKillWinCmd( ) {
    
    SimExpr     rExpr;
    int         winNumStart     = 0;
    int         winNumEnd       = 0;
    
    if ( ! winModeOn ) throw ( ERR_NOT_IN_WIN_MODE );
    
    if ( tok -> tokId( ) == TOK_EOS ) {
        
        winNumStart = glb -> winDisplay -> getCurrentUserWindow( );
        winNumEnd   = winNumStart;
    }
    else {
        
        eval -> parseExpr( &rExpr );
        
        if ( rExpr.typ == TYP_NUM ) winNumStart = rExpr.numVal;
        else throw ( ERR_EXPECTED_NUMERIC );
        
        if ( tok -> tokId( ) == TOK_COMMA ) {
            
            tok -> nextToken( );
            eval -> parseExpr( &rExpr );
            
            if ( rExpr.typ == TYP_NUM ) winNumEnd = rExpr.numVal;
            else throw ( ERR_EXPECTED_NUMERIC );
        }
        
        if ( winNumStart == -1 ) {
            
            winNumStart = glb -> winDisplay -> getFirstUserWinIndex( );
            winNumEnd   = glb -> winDisplay -> getLastUserWinIndex( );
        }
        
        if ( winNumStart > winNumEnd ) winNumEnd = winNumStart;
    }
    
    if (( ! glb -> winDisplay -> validWindowNum( winNumStart )) ||
        ( ! glb -> winDisplay -> validWindowNum( winNumEnd ))) throw ( ERR_INVALID_WIN_ID );
    
    glb -> winDisplay -> windowKill( winNumStart, winNumEnd );
    glb -> winDisplay -> reDraw( true );
}

//------------------------------------------------------------------------------------------------------------
// This command assigns a user window to a stack. User windows can be displayed in a separate stack of
// windows. The first stack is always the main stack, where the predefined and command window can be found.
//
//  WS <stackNum> [ , <winNumStart> [ , <winNumEnd ]]
//------------------------------------------------------------------------------------------------------------
void SimCommandsWin::winSetStackCmd( ) {
    
    SimExpr rExpr;
    int     stackNum    = 0;
    int     winNumStart = 0;
    int     winNumEnd   = 0;
    
    if ( ! winModeOn ) throw ( ERR_NOT_IN_WIN_MODE );
    
    eval -> parseExpr( &rExpr );
    
    if ( rExpr.typ == TYP_NUM ) stackNum = rExpr.numVal;
    else throw ( ERR_EXPECTED_STACK_ID );
    
    if ( ! glb -> winDisplay -> validWindowStackNum( stackNum ))
        throw ( ERR_INVALID_WIN_STACK_ID );
    
    if ( tok -> tokId( ) == TOK_EOS ) {
        
        winNumStart = glb -> winDisplay -> getCurrentUserWindow( );
        winNumEnd   = winNumStart;
    }
    else if ( tok -> tokId( ) == TOK_COMMA ) {
            
        tok -> nextToken( );
        eval -> parseExpr( &rExpr );
            
        if ( rExpr.typ == TYP_NUM ) winNumStart = rExpr.numVal;
        else throw ( ERR_EXPECTED_NUMERIC );
            
        if ( tok -> tokId( ) == TOK_COMMA ) {
                
            tok -> nextToken( );
            eval -> parseExpr( &rExpr );
                
            if ( rExpr.typ == TYP_NUM ) winNumEnd = rExpr.numVal;
            else throw ( ERR_EXPECTED_NUMERIC );
        }
        else winNumEnd = winNumStart;
    }
    else throw ( ERR_EXPECTED_COMMA );
        
    if ( winNumStart == -1 ) {
            
        winNumStart = glb -> winDisplay -> getFirstUserWinIndex( );
        winNumEnd   = glb -> winDisplay -> getLastUserWinIndex( );
    }
    
    if (( ! glb -> winDisplay -> validWindowNum( winNumStart )) ||
        ( ! glb -> winDisplay -> validWindowNum( winNumEnd ))) throw ( ERR_INVALID_WIN_ID );
    
    glb -> winDisplay -> windowSetStack( stackNum, winNumStart, winNumEnd );
    glb -> winDisplay -> reDraw( true );
}

//------------------------------------------------------------------------------------------------------------
// Evaluate input line. There are commands, functions, expressions and so on. This routine sets up the
// tokenizer and dispatches based on the first token in the input line. The commands are also added to the
// command history, with the exception of the HITS, DO and REDOP comamnds.
//
//------------------------------------------------------------------------------------------------------------
void SimCommandsWin::evalInputLine( char *cmdBuf ) {
    
    try {
        
        if ( strlen( cmdBuf ) > 0 ) {
            
            tok -> setupTokenizer( cmdBuf, (SimToken *) cmdTokTab );
            tok -> nextToken( );
            
            if (( tok -> isTokenTyp( TYP_CMD )) || ( tok -> isTokenTyp( TYP_WCMD ))) {
                
                SimTokId cmdId = tok -> tokId( );
                tok -> nextToken( );
                
                if (( cmdId != CMD_HIST ) && ( cmdId != CMD_DO ) && ( cmdId != CMD_REDO )) {
                    
                    hist -> addCmdLine( cmdBuf );
                    glb -> env -> setEnvVar((char *) ENV_CMD_CNT, hist -> getCmdNum( ));
                }
                
                switch( cmdId ) {
                        
                    case TOK_NIL:                                           break;
                    case CMD_EXIT:          exitCmd( );                     break;
                    
                    case CMD_WC_CU:         cursorUpCmd( );                 break;
                    case CMD_WC_CD:         cursorDownCmd( );               break;
                        
                    case CMD_HELP:          helpCmd( );                     break;
                    case CMD_ENV:           envCmd( );                      break;
                    case CMD_XF:            execFileCmd( );                 break;
                   
                    case CMD_WRITE_LINE:    writeLineCmd( );                break;
                        
                    case CMD_HIST:          histCmd( );                     break;
                    case CMD_DO:            doCmd( );                       break;
                    case CMD_REDO:          redoCmd( );                     break;
                        
                    case CMD_RESET:         resetCmd( );                    break;
                    case CMD_RUN:           runCmd( );                      break;
                    case CMD_STEP:          stepCmd( );                     break;
                        
                    case CMD_MR:            modifyRegCmd( );                break;
                        
                    case CMD_DA:            displayAbsMemCmd( );            break;
                    case CMD_MA:            modifyAbsMemCmd( );             break;
                        
                    case CMD_D_TLB:         displayTLBCmd( );               break;
                    case CMD_I_TLB:         insertTLBCmd( );                break;
                    case CMD_P_TLB:         purgeTLBCmd( );                 break;
                        
                    case CMD_D_CACHE:       displayCacheCmd( );             break;
                    case CMD_P_CACHE:       purgeCacheCmd( );               break;
                        
                    case CMD_WON:           winOnCmd( );                    break;
                    case CMD_WOFF:          winOffCmd( );                   break;
                    case CMD_WDEF:          winDefCmd( );                   break;
                    case CMD_WSE:           winStacksEnable( );             break;
                    case CMD_WSD:           winStacksDisable( );            break;
                        
                    case CMD_WC:            winCurrentCmd( );               break;
                    case CMD_WN:            winNewWinCmd( );                break;
                    case CMD_WK:            winKillWinCmd( );               break;
                    case CMD_WS:            winSetStackCmd( );              break;
                    case CMD_WT:            winToggleCmd( );                break;
                    case CMD_WX:            winExchangeCmd( );              break;
                        
                    case CMD_WF:            winForwardCmd( cmdId );         break;
                    case CMD_WB:            winBackwardCmd( cmdId );        break;
                    case CMD_WH:            winHomeCmd( cmdId );            break;
                    case CMD_WJ:            winJumpCmd( cmdId );            break;
                  
                    case CMD_PSE:
                    case CMD_SRE:
                    case CMD_PLE:
                    case CMD_SWE:
                    case CMD_WE:            winEnableCmd( cmdId );          break;
                       
                    case CMD_PSD:
                    case CMD_SRD:
                    case CMD_PLD:
                    case CMD_SWD:
                    case CMD_WD:            winDisableCmd( cmdId );         break;
                  
                    case CMD_PSR:
                    case CMD_SRR:
                    case CMD_PLR:
                    case CMD_SWR:
                    case CMD_WR:            winSetRadixCmd( cmdId );        break;
                        
                    case CMD_CWL:
                    case CMD_WL:            winSetRowsCmd( cmdId );         break;
                        
                    default:                throw ( ERR_INVALID_CMD );
                }
            }
            else {
                
                printf( "Got: %s\n", cmdBuf );
                
                hist -> addCmdLine( cmdBuf );
                glb -> env -> setEnvVar((char *) ENV_CMD_CNT, hist -> getCmdNum( ));
                glb -> env -> setEnvVar((char *) ENV_EXIT_CODE, -1 );
                throw ( ERR_INVALID_CMD );
            }
        }
    }
    
    catch ( SimErrMsgId errNum ) {
        
        glb -> env -> setEnvVar((char *) ENV_EXIT_CODE, -1 );
        cmdLineError( errNum );
    }
}

//------------------------------------------------------------------------------------------------------------
// "cmdLoop" is the command line input interpreter. The basic loop is to prompt for the next input, read the
// input and evaluates it. If we are in windows mode, we also redraw the screen.
//
// ??? when is the best point to redraw the windows... exactly once ?
//------------------------------------------------------------------------------------------------------------
void SimCommandsWin::cmdInterpreterLoop( ) {
    
    char cmdLineBuf[ CMD_LINE_BUF_SIZE ] = { 0 };
    
    printWelcome( );
    
    while ( true ) {
        
        int promptLen = promptCmdLine( );
        if ( readInputLine( cmdLineBuf, promptLen ) > 0 ) {
            
          //  glb -> hist -> addCmdLine( cmdLineBuf );
          //  glb -> env -> setEnvVar((char *) ENV_CMD_CNT, glb -> hist -> getCmdId( ));
            
            evalInputLine( cmdLineBuf );
            if ( winModeOn ) glb -> winDisplay -> reDraw( );
        }
    }
}

