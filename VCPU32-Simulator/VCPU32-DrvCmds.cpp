//------------------------------------------------------------------------------------------------------------
//
// VCPU32 - A 32-bit CPU - Simulator Commands
//
//------------------------------------------------------------------------------------------------------------
// Welcome to the test driver commands.
//
//
//------------------------------------------------------------------------------------------------------------
//
// VCPU32 - A 32-bit CPU - Simulator Commands
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
#include "VCPU32-Version.h"
#include "VCPU32-Types.h"
#include "VCPU32-Driver.h"
#include "VCPU32-DrvTables.h"
#include "VCPU32-Core.h"

//------------------------------------------------------------------------------------------------------------
// Local name space. We try to keep utility functions local to the file.
//
//------------------------------------------------------------------------------------------------------------
namespace {

//------------------------------------------------------------------------------------------------------------
// A little helper function to upshift a string in place.
//
//------------------------------------------------------------------------------------------------------------
void upshiftStr( char *str ) {
    
    size_t len = strlen( str );
    
    if ( len > 0 ) {
        
        for ( size_t i = 0; i < len; i++ ) str[ i ] = (char) toupper((int) str[ i ] );
    }
}

//------------------------------------------------------------------------------------------------------------
// "setRadix" ensures that we passed in a valid radix value. The default is a decimal number.
//
//------------------------------------------------------------------------------------------------------------
int setRadix( int rdx ) {
    
    return((( rdx == 8 ) || ( rdx == 10 ) || ( rdx == 16 )) ? rdx : 10 );
}

//------------------------------------------------------------------------------------------------------------
// Print out an error message text with an optional argument.
//
// ??? how about a table with err msg id and string ? simplify this routine then...
//------------------------------------------------------------------------------------------------------------
ErrMsgId cmdErr( ErrMsgId errNum, char *argStr = nullptr ) {
    
    switch ( errNum ) {
            
        case ERR_NOT_IN_WIN_MODE:           fprintf( stdout, "Command only valid in Windows mode\n" ); break;
        case ERR_OPEN_EXEC_FILE:            fprintf( stdout, "Error while opening file: \"%s\"\n", argStr ); break;
        case ERR_EXPECTED_FILE_NAME:        fprintf( stdout, "Expected a file name\n" ); break;
        case ERR_INVALID_CMD:               fprintf( stdout, "Invalid command, use help or whelp\n"); break;
        case ERR_INVALID_WIN_STACK_ID:      fprintf( stdout, "Invalid window stack Id\n" ); break;
        case ERR_EXPECTED_STACK_ID:         fprintf( stdout, "Expected stack Id\n" ); break;
        case ERR_INVALID_WIN_ID:            fprintf( stdout, "Invalid window Id\n" ); break;
        case ERR_EXPECTED_WIN_ID:           fprintf( stdout, "Expected a window Id\n" ); break;
            
        case ERR_INVALID_REG_ID:            fprintf( stdout, "Invalid register Id\n" ); break;
        case ERR_INVALID_RADIX:             fprintf( stdout, "Invalid radix\n" ); break;
            
        case ERR_EXTRA_TOKEN_IN_STR:        fprintf( stdout, "Extra tokens in command line\n" ); break;
        case ERR_EXPECTED_LPAREN:           fprintf( stdout, "Expected a left paren\n" ); break;
        case ERR_EXPECTED_RPAREN:           fprintf( stdout, "Expected a right paren\n" ); break;
        case ERR_EXPECTED_COMMA:            fprintf( stdout, "Expected a comma\n" ); break;
            
        case ERR_INVALID_EXIT_VAL:          fprintf( stdout, "Invalid program exit code\n" ); break;
            
        case ERR_ENV_VALUE_EXPR:            fprintf( stdout, "Invalid expression for ENV variable\n" ); break;
        case ERR_EXPECTED_STR:              fprintf( stdout, "Expected a string value\n" ); break;
            
        case ERR_ENV_VAR_NOT_FOUND:         fprintf( stdout, "ENV variable not found\n" ); break;
            
        case ERR_EXPECTED_REG_SET:          fprintf( stdout, "Expected a register set\n" ); break;
        case ERR_EXPECTED_REG_OR_SET:       fprintf( stdout, "Expected a register or register set\n" ); break;
            
        case ERR_WIN_TYPE_NOT_CONFIGURED:   fprintf( stdout, "Win object type not configured\n" ); break;
            
        case ERR_EXPECTED_NUMERIC:          fprintf( stdout, "Expected a numeric value\n" ); break;
        case ERR_EXPECTED_EXT_ADR:          fprintf( stdout, "Expected a virtual address\n" ); break;
            
        case ERR_EXPR_TYPE_MATCH:           fprintf( stdout, "Expression type mismatch\n" ); break;
        case ERR_EXPR_FACTOR:               fprintf( stdout, "Expression error: factor\n" ); break;
        case ERR_EXPECTED_GENERAL_REG:          fprintf( stdout, "Expression a general reg\n" ); break;
            
        case ERR_INVALID_ARG:                   fprintf( stdout, "Invalid argument for command\n" ); break;
        case ERR_EXPECTED_STEPS:                fprintf( stdout, "Expected nuber of steps/instr\n" ); break;
        case ERR_INVALID_STEP_OPTION:           fprintf( stdout, "Invalid steps/instr option\n" ); break;
            
        case ERR_EXPECTED_INSTR_VAL:            fprintf( stdout, "Expected the instruction value\n" ); break;
        case ERR_TOO_MANY_ARGS_CMD_LINE:        fprintf( stdout, "Too many args in command line\n" ); break;
            
        case ERR_EXPECTED_START_OFS:            fprintf( stdout, "Expected start offset\n" ); break;
        case ERR_EXPECTED_LEN:                  fprintf( stdout, "Expected length argument\n" ); break;
        case ERR_OFS_LEN_LIMIT_EXCEEDED:        fprintf( stdout, "Offset/Length exceeds limit\n" ); break;
        case ERR_EXPECTED_OFS:                  fprintf( stdout, "Expected an address\n" ); break;
            
        case ERR_INVALID_CHAR_IN_TOKEN_LINE:    fprintf( stdout, "Invalid char in input line\n" ); break;
        case ERR_UNDEFINED_PFUNC:           fprintf( stdout, "Unknown predefined function\n" ); break;
            
        case ERR_INVALID_EXPR:              fprintf( stdout, "Invalid expression\n" ); break;
        case ERR_EXPECTED_INSTR_OPT:        fprintf( stdout, "Expected the instructon options\n" ); break;
        case ERR_INVALID_INSTR_OPT:         fprintf( stdout, "INvalid instruction option\n" ); break;
        case ERR_INSTR_HAS_NO_OPT:          fprintf( stdout, "Instruction has no option\n" ); break;
        case ERR_EXPECTED_SR1_SR3:          fprintf( stdout, "Expected SR1 .. SR3 as segment register\n" ); break;
        case ERR_EXPECTED_LOGICAL_ADR:      fprintf( stdout, "Expected a logical address\n" ); break;
        case ERR_IMM_VAL_RANGE:             fprintf( stdout, "Immediate value out of range\n" ); break;
        case ERR_INVALID_INSTR_MODE:        fprintf( stdout, "Invalid adr mode for instruction\n" ); break;
        case ERR_INSTR_MODE_OPT_COMBO:      fprintf( stdout, "Invalid opCode data width specifier for mode option\n" ); break;
        case ERR_POS_VAL_RANGE:             fprintf( stdout, "Bit position value out of range\n" ); break;
        case ERR_LEN_VAL_RANGE:             fprintf( stdout, "Bit field length value out of range\n" ); break;
            
        case ERR_EXPECTED_AN_OFFSET_VAL:    fprintf( stdout, "Excpected an offset valuen\n" ); break;
        case ERR_OFFSET_VAL_RANGE:          fprintf( stdout, "Offset value out of range\n" ); break;
        case ERR_INVALID_REG_COMBO:         fprintf( stdout, "Invalid register combo for instruction\n" ); break;
        case ERR_EXPECTED_SEGMENT_REG:      fprintf( stdout, "Expected a segment register\n" ); break;
        case ERR_INVALID_S_OP_CODE:         fprintf( stdout, "Invalid synthetic instruction opcode\n" ); break;
            
        case ERR_INVALID_FMT_OPT:           fprintf( stdout, "Invalid format option\n" ); break;
        case ERR_EXPECTED_FMT_OPT:          fprintf( stdout, "Expected a format option\n" ); break;
        case ERR_INVALID_WIN_TYPE:          fprintf( stdout, "Invalid window type\n" ); break;
        case ERR_EXPECTED_WIN_TYPE:         fprintf( stdout, "Expected a window type\n" ); break;
        case ERR_OUT_OF_WINDOWS:            fprintf( stdout, "Cannot create more windows\n" ); break;
            
        case ERR_TLB_TYPE:                  fprintf( stdout, "Expected a TLB type\n" ); break;
        case ERR_TLB_INSERT_OP:             fprintf( stdout, "Insert in TLB operation error\n" ); break;
        case ERR_TLB_PURGE_OP:              fprintf( stdout, "Purge from TLB operation error\n" ); break;
        case ERR_TLB_ACC_DATA:              fprintf( stdout, "Invalid TLB insert access data\n" ); break;
        case ERR_TLB_ADR_DATA:              fprintf( stdout, "Invalid TLB insert address data\n" ); break;
        case ERR_TLB_NOT_CONFIGURED:        fprintf( stdout, "TLB type not configured\n" ); break;
        case ERR_TLB_SIZE_EXCEEDED:         fprintf( stdout, "TLB size exceeded\n" ); break;
            
        case ERR_CACHE_TYPE:                fprintf( stdout, "Expected a cache type\n" ); break;
        case ERR_CACHE_PURGE_OP:            fprintf( stdout, "Purge from cache operation error\n" ); break;
        case ERR_CACHE_NOT_CONFIGURED:      fprintf( stdout, "Cache type not configured\n" ); break;
        case ERR_CACHE_SIZE_EXCEEDED:       fprintf( stdout, "Cache size exceeded\n" ); break;
        case ERR_CACHE_SET_NUM:             fprintf( stdout, "Invalid cache set\n" ); break;
            
        case ERR_UNEXPECTED_EOS:            fprintf( stdout, "Unexpectedd end of command line\n" ); break;
            
        default: {
            
            fprintf( stdout, "Error: %d", errNum );
            if ( argStr != nullptr ) fprintf( stdout, "%32s", argStr );
            fprintf( stdout, "/n" );
        }
    }
    
    return( errNum );
}

//------------------------------------------------------------------------------------------------------------
// Just list all commands that we have.
//
//------------------------------------------------------------------------------------------------------------
void displayHelp( ) {
    
    const char FMT_STR[ ] = "%-50s%s\n";
    
    fprintf( stdout, FMT_STR, "help",   "displays syntax and a short description" );
    fprintf( stdout, FMT_STR, "#",      "echoes the command input" );
    fprintf( stdout, FMT_STR, "e        [ <val> ]", "program exit" );
    fprintf( stdout, FMT_STR, "env      [ <var> [ , <val> ]]", "lists the env tab, a variable, sets a variable" );
    
    fprintf( stdout, FMT_STR, "xf       <filepath>", "execute commands from a file" );
    fprintf( stdout, FMT_STR, "lmf      <path> [ , <opt> ]", "loads memory from a file" );
    fprintf( stdout, FMT_STR, "smf      <path> <ofs> [ , <len> ]", "stores memory to a file" );
    
    fprintf( stdout, FMT_STR, "reset    ( CPU|MEM|STATS|ALL )", "resets the CPU" );
    fprintf( stdout, FMT_STR, "run",    "run the CPU" );
    fprintf( stdout, FMT_STR, "s        [ <num> ] [ , I|C ]", "single step for instruction or clock cycle" );
    
    fprintf( stdout, FMT_STR, "dr       [ <regSet>| <reg> ] [ , <fmt> ]", "display registers" );
    fprintf( stdout, FMT_STR, "mr       <reg> , <val>", "modify registers" );
    
    fprintf( stdout, FMT_STR, "da       <ofs> [ , <len> ] [ , <fmt> ]", "display memory" );
    fprintf( stdout, FMT_STR, "ma       <ofs> , <val>", "modify memory" );
    fprintf( stdout, FMT_STR, "maa      <ofs> , <asm-str>", "modify memory as code" );
    
    
    // ??? fix the syntax...
    fprintf( stdout, FMT_STR, "dca      <I|D|U> \",\" [<index> <len>]", "display cache content" );
    fprintf( stdout, FMT_STR, "pca      <I|D|U> \",\" <index> [<F>]", "flushes and purges cache data" );
    
    fprintf( stdout, FMT_STR, "dtlb     <I|D> [<index> <len>]", "display TLB content" );
    fprintf( stdout, FMT_STR, "itlb     <I|D> <seg> <ofs> <argAcc> <argAdr>", "inserts an entry into the TLB" );
    fprintf( stdout, FMT_STR, "ptlb     <I|D> <seg> <ofs>", "purges an entry from the TLB" );
   
    fprintf( stdout, FMT_STR, "won",    "switches to windows mode" );
    fprintf( stdout, FMT_STR, "woff",   "switches to command line mode" );
    fprintf( stdout, FMT_STR, "wdef",   "reset the windows to their default values" );
    fprintf( stdout, FMT_STR, "wse",    "enable window stacks" );
    fprintf( stdout, FMT_STR, "wsd",    "disable window stacks" );
    fprintf( stdout, FMT_STR, "<win><cmd> [<args-list>]", "issue a window command, use whelp for details." );
    fprintf( stdout, "\n" );
    
}

//------------------------------------------------------------------------------------------------------------
// List the ehlp for widows command.
//
//------------------------------------------------------------------------------------------------------------
void displayWindowHelp( ) {
    
    const char FMT_STR[ ] = "%-32s%s\n";
    
    fprintf( stdout, "Windows help \n\n" );
    fprintf( stdout, "General Syntax for Win Commands: <win><cmd> [ args ]\n\n" );
    fprintf( stdout, "Windows:\n" );
    fprintf( stdout, FMT_STR, "PS",  "Program state window" );
    fprintf( stdout, FMT_STR, "SR",  "Special Register window" );
    fprintf( stdout, FMT_STR, "PL",  "CPU Pipeline Registers window" );
    fprintf( stdout, FMT_STR, "ST",  "Statistics window" );
    fprintf( stdout, FMT_STR, "IT",  "CPU Instruction TLB window" );
    fprintf( stdout, FMT_STR, "DT",  "CPU Data TLB window" );
    fprintf( stdout, FMT_STR, "IC",  "CPU Instruction Cache (L1) window" );
    fprintf( stdout, FMT_STR, "DC",  "CPU Data Cache (L1) window" );
    fprintf( stdout, FMT_STR, "UC",  "CPU Unified Cache (L2) window" );
    fprintf( stdout, FMT_STR, "PM",  "Physical Memory window" );
    fprintf( stdout, FMT_STR, "PC",  "Program Code Window" );
    fprintf( stdout, FMT_STR, "ICR", "CPU Instruction Cache (L1) controller registers" );
    fprintf( stdout, FMT_STR, "DCR", "CPU Data Cache (L1) controller registers" );
    fprintf( stdout, FMT_STR, "UCR", "CPU Unified Cache (L2) controller registers" );
    fprintf( stdout, FMT_STR, "MCR", "Physical Memory controller registers" );
    fprintf( stdout, FMT_STR, "ITR", "CPU Instruction TLB controller registers" );
    fprintf( stdout, FMT_STR, "DTR", "CPU Data TLB controller registers" );
    fprintf( stdout, FMT_STR, "PCR", "PDC Memory controller registers" );
    fprintf( stdout, FMT_STR, "IOR", "IO Memory controller registers" );
    fprintf( stdout, FMT_STR, "TX",  "Text Window" );
    fprintf( stdout, FMT_STR, "CW",  "Command Line window" );
    fprintf( stdout, FMT_STR, "W",   "User defined window" );
    fprintf( stdout, "\n" );
    
    fprintf( stdout, "Commands:\n" );
    fprintf( stdout, FMT_STR, "E [ <wNum> ]", "Enable window display" );
    fprintf( stdout, FMT_STR, "D [ <wNum> ]", "Disable window display" );
    fprintf( stdout, FMT_STR, "B <amt> [ , <wNum> ]", "Move backward by n items" );
    fprintf( stdout, FMT_STR, "F <amt> [ , <wNum> ]", "Move forward by n items" );
    fprintf( stdout, FMT_STR, "H <pos> [ , <wNum> ]", "Set window home position or set new home position" );
    fprintf( stdout, FMT_STR, "J <pos> [ , <wNum> ]", "Set window start to new position");
    fprintf( stdout, FMT_STR, "L <lines> [ , <wNum> ]", "Set window lines including banner line" );
    fprintf( stdout, FMT_STR, "R <radix> [ , <wNum> ]", "Set window radix ( OCT, DEC, HEX )" );
    fprintf( stdout, FMT_STR, "C <wNum>", "set the window <wNum> as current window" );
    fprintf( stdout, FMT_STR, "T <wNum>", "toggle through alternate window content" );
    fprintf( stdout, FMT_STR, "X <wNum>", "exchange current window with this window");
    fprintf( stdout, FMT_STR, "N <type> [ , <arg> ]", "New user defined window ( PM, PC, IT, DT, IC, ICR, DCR, MCR, TX )" );
    fprintf( stdout, FMT_STR, "K <wNumStart> [ , <wNumEnd> ]", "Removes a range of user defined window" );
    fprintf( stdout, FMT_STR, "S <stackNum> [ , <wNumStart> ] [ , <wNumEnd>]", "moves a range of user windows into stack <stackNum>" );
    fprintf( stdout, "\n" );
    
    fprintf( stdout, "Example: SRE       -> show special register window\n" );
    fprintf( stdout, "Example: WN PM     -> create a user defined physical memory window\n" );
    fprintf( stdout, "Example: WN 20, 11 -> scroll window 11 forward by 20 lines\n" );
    fprintf( stdout, "\n" );
}

//------------------------------------------------------------------------------------------------------------
// "promptYesNoCancel" is a simple function to print a prompt string with a decision question. The answer can
//  be yes/no or cancel. A positive result is a "yes" a negative result a "no", anything else a "cancel".
//
//------------------------------------------------------------------------------------------------------------
int promptYesNoCancel( char *promptStr ) {
    
    fprintf( stdout, "%s -> ", promptStr );
    fflush( stdout );
    
    char buf[ 8 ] = "";
    
    if ( fgets( buf, 8, stdin ) != nullptr ) {
        
        if      (( buf[ 0 ] == 'Y' ) ||  ( buf[ 0 ] == 'y' ))   return( 1 );
        else if (( buf[ 0 ] == 'N' ) ||  ( buf[ 0 ] == 'n' ))   return( -1 );
        else                                                    return( 0 );
    }
    else return( 0 );
}

//------------------------------------------------------------------------------------------------------------
// A little helper function to remove the comment part of a command line. We do the changes on the buffer
// passed in by just setting the ed of string at the position of the ";" comment indicator.
//
//------------------------------------------------------------------------------------------------------------
void removeComment( char *cmdBuf ) {
    
    if ( strlen( cmdBuf ) > 0 ) {
        
        char *tmp = strrchr( cmdBuf, '#' );
        if( tmp != nullptr ) *tmp = 0;
    }
}

}; // namespace


//************************************************************************************************************
//************************************************************************************************************
//
// Object methods.
//
//************************************************************************************************************
//************************************************************************************************************


//------------------------------------------------------------------------------------------------------------
// The object constructor. We just remember where globals are.
//
//------------------------------------------------------------------------------------------------------------
DrvCmds::DrvCmds( VCPU32Globals *glb ) {
    
    this -> glb = glb;
}

//------------------------------------------------------------------------------------------------------------
// Get the command interpreter ready.
//
// One day we will handle command line arguments....
//
//  -v           verbose
//  -i <path>    init file
//
// ??? to do ...
//------------------------------------------------------------------------------------------------------------
void DrvCmds::setupCmdInterpreter( int argc, const char *argv[ ] ) {
    

    while ( argc > 0 ) {
        
        argc --;
    }
    
    glb -> winDisplay  -> windowDefaults( );
    glb -> lineDisplay -> lineDefaults( );
}

//------------------------------------------------------------------------------------------------------------
// "commandLineError" is a little helper that prints out the error encountered. We will print a caret marker
// where we found the error, and then return a false. Parsing errors typically result in aborting the parsing
// process.
//
//------------------------------------------------------------------------------------------------------------
void DrvCmds::cmdLineError( ErrMsgId errNum, char *argStr ) {
    
    int     i           = 0;
    int     tokIndex    = glb -> tok -> tokCharIndex( );

    while (( i < tokIndex ) && ( i < strlen( glb -> tok -> tokenLineStr( )))) {
        
        fprintf( stdout, " " );
        i ++;
    }
    
    fprintf( stdout, "^\n" );
    cmdErr( errNum, argStr );
}

//------------------------------------------------------------------------------------------------------------
// Token analysis helper functions.
//
//------------------------------------------------------------------------------------------------------------
void DrvCmds::checkEOS( ) {
    
    if ( ! glb -> tok -> isToken( TOK_EOS )) throw ( ERR_TOO_MANY_ARGS_CMD_LINE );
}

void DrvCmds::acceptComma( ) {
    
    if ( glb -> tok -> isToken( TOK_COMMA )) glb -> tok -> nextToken( );
    else throw ( ERR_EXPECTED_COMMA );
}

void DrvCmds::acceptLparen( ) {
    
    if ( glb -> tok -> isToken( TOK_LPAREN )) glb -> tok -> nextToken( );
    else throw ( ERR_EXPECTED_LPAREN );
}

void DrvCmds::acceptRparen( ) {
    
    if ( glb -> tok -> isToken( TOK_RPAREN )) glb -> tok -> nextToken( );
    else throw ( ERR_EXPECTED_LPAREN );
}

//------------------------------------------------------------------------------------------------------------
// Return the current command entered.
//
//------------------------------------------------------------------------------------------------------------
TokId DrvCmds::getCurrentCmd( ) {
    
    return( currentCmd );
}

//------------------------------------------------------------------------------------------------------------
// Our friendly welcome message with the actual program version. We also set some of the environment variables
// to an initial value. Especially string variables need to be set as they are not initialized from the
// environment variable table.
//
//------------------------------------------------------------------------------------------------------------
void DrvCmds::printWelcome( ) {
    
    glb -> env -> setEnvVar((char *) ENV_EXIT_CODE, 0 );
    
    if ( isatty( fileno( stdin ))) {
       
        fprintf( stdout, "VCPU-32 Simulator, Version: %s\n", glb -> env -> getEnvVarStr((char *) ENV_PROG_VERSION ));
        fprintf( stdout, "Git Branch: %s\n", glb -> env -> getEnvVarStr((char *) ENV_GIT_BRANCH ));
    }
}

//------------------------------------------------------------------------------------------------------------
// "promptCmdLine" lists out the prompt string. For now this is just a "->". As development goes on the prompt
// string will contain some more info about the current CPU state. The prompt is only printed when the input
// comes from a terminal and not an input file.
//
//------------------------------------------------------------------------------------------------------------
void DrvCmds::promptCmdLine( ) {
    
    if ( isatty( fileno( stdin ))) {
        
        if ( glb -> env -> getEnvVarBool((char *) ENV_SHOW_CMD_CNT ))
            fprintf( stdout, "(%i) ", glb -> env -> getEnvVarInt((char *) ENV_CMD_CNT ));
        
        fprintf( stdout, "->" );
        fflush( stdout );
    }
}

//------------------------------------------------------------------------------------------------------------
// "readCmdLine" reads in the command line. For a valid command line, the trailing carriage return and/or
// line feeds are removed and the first token is interpreted as a command. The function returns the command
// found, an invalid command or an empty command line status. We loop inside the routine until we receive
// a valid command line or an EOF.
//
// Warning: on a Mac, "fgets" does read in a string. On the terminal program configuration, the erase
// character needs to be set to NOT send a control-H to the input. The cursor keys are just echoed to the
// input line and I do not know a way to make them actually move around in the input line.
//
//------------------------------------------------------------------------------------------------------------
bool DrvCmds::readInputLine( char *cmdBuf ) {
    
    while ( true ) {
        
       //  fflush( stdout );
        
#if 1 // current version
        
        if ( fgets( cmdBuf, CMD_LINE_BUF_SIZE, stdin ) != nullptr ) {
            
            cmdBuf[ strcspn( cmdBuf, "\r\n") ] = 0;
            
            removeComment( cmdBuf );
            
     
            if ( strlen( cmdBuf ) > 0 ) {
                
                glb -> env -> setEnvVar((char *) ENV_CMD_CNT,
                                        glb -> env -> getEnvVarInt((char *) ENV_CMD_CNT ) + 1 );
                return( true );
            }
            else return( false );
        }
        else if ( feof( stdin )) exit( glb -> env -> getEnvVarInt((char *) ENV_EXIT_CODE ));
        
        return( false );
        
#else
        
        // we need another version of command input for sharing with a running CPU. The idea is that in
        // running mode the terminal input is actally handled by a CPU monitor program with console IO
        // functions. Our job is then to move characters directly to howver the console IO is implemented.
        // A "control Y" input is taking us back to the simulator.
        //
        // Unfortunately, windows and macs differ. The standard system calls typically buffer the input
        // up to the carriage return. To avoid this, the terminal needs to be place in "raw" mode. And this
        // is diffferent for the two platforms.
        //
        // ??? need a seprate method for placing in raw mode, resettimg this mode, and reading and writing
        // a single character.
        //
        char    ch;
        int     index = 0;
        
        while ( true ) {
            
            ch = getchar( );
            
            if ( ch == 0x19 ) {
                
                // handle control Y
            }
            else if (( ch == '\n' ) || ( ch == '\r' )) {
                
                cmdBuf[ index ] = '\0';
                putchar( ch );
                return ( true );
            }
            else if ( ch == '\b' ) {
               
                if ( index > 0 ) {
                    
                    index --;
                    
                    putchar( '\b' );
                    putchar( ' ' );
                    putchar( '\b' );
                    continue;
                }
            }
            
            if ( index < CMD_LINE_BUF_SIZE - 1 ) {
                
                cmdBuf[ index ] = ch;
                index ++;
                putchar( ch );
            }
            else {
            
                cmdBuf[ index ] = '\0';
                return( false );
            }
        }
        
#endif
        
    }
}

//------------------------------------------------------------------------------------------------------------
// "execCmdsFromFile" will open a text file and interpret each line as a command. This routine is used by the
// "EXEC-F" command and also as the handler for the program argument option to execute a file before entering
// the command loop.
//
// ??? which error would we like to report here vs. pass on to outer command loop ?
//------------------------------------------------------------------------------------------------------------
void DrvCmds::execCmdsFromFile( char* fileName ) {
    
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
                        
                        fprintf( stdout, "%s\n", cmdLineBuf );
                    }
                    
                    removeComment( cmdLineBuf );
                    evalInputLine( cmdLineBuf );
                }
            }
            else throw( ERR_OPEN_EXEC_FILE );
        }
        else throw ( ERR_EXPECTED_FILE_NAME  );
    }
    
    catch ( ErrMsgId errNum ) {
        
        throw ( errNum );  // for now ...
    }
}

//------------------------------------------------------------------------------------------------------------
// Help command. With no arguments, a short help overview is printed. If there is an optional argument,
// specific help on the topic is given.
//
//------------------------------------------------------------------------------------------------------------
void DrvCmds::helpCmd( ) {
    
    displayHelp( );
}

//------------------------------------------------------------------------------------------------------------
// Display the window specific help.
//
//------------------------------------------------------------------------------------------------------------
void DrvCmds::winHelpCmd( ) {
    
    displayWindowHelp( );
}

//------------------------------------------------------------------------------------------------------------
// Exit command. We will exit with the environment variable value for the exit code or the argument value
// in the command. This will be quite useful for test script development.
//
// EXIT <code>
//------------------------------------------------------------------------------------------------------------
void DrvCmds::exitCmd( ) {
    
    DrvExpr rExpr;
    int  exitVal = 0;
    
    if ( glb -> tok -> tokId( ) == TOK_EOS ) {
        
        exitVal = glb -> env -> getEnvVarInt((char *) ENV_EXIT_CODE );
        exit(( exitVal > 255 ) ? 255 : exitVal );
    }
    else {
        
        glb -> eval -> parseExpr( &rExpr );
        
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
// remove a user defined variuable.
//
// ENV [ <envName> [ <val> ]]
//------------------------------------------------------------------------------------------------------------
void DrvCmds::envCmd( ) {
    
    if ( glb -> tok -> tokId( ) == TOK_EOS ) {
        
        glb -> env -> displayEnvTable( );
    }
    else if ( glb -> tok -> tokTyp( ) == TYP_IDENT ) {
        
        char envName[ MAX_ENV_NAME_SIZE ];
        
        strcpy( envName, glb -> tok -> tokStr( ));
        upshiftStr( envName );
        
        glb -> tok -> nextToken( );
        if ( glb -> tok -> tokId( ) == TOK_EOS ) {
            
            if ( glb -> env -> isValid( envName )) {
                
                glb-> env -> displayEnvTableEntry( envName );
            }
            else ; // throw entry does not exist...
        }
        else {
            
            if ( ! glb -> env -> isValid( envName )) {
                
                // ??? allocate a new ENV
                
            }
            
            uint8_t rStat = NO_ERR;
            DrvExpr rExpr;
            
            glb -> eval -> parseExpr( &rExpr );
            if ( rStat != NO_ERR ) throw ( ERR_ENV_VALUE_EXPR );
            
            if ( rExpr.typ == TYP_NUM ) {
                
                rStat = glb -> env -> setEnvVar( envName, rExpr.numVal );
            }
            else if ( rExpr.typ == TYP_BOOL ) {
                
                rStat = glb -> env -> setEnvVar( envName, rExpr.bVal );
            }
            else if ( rExpr.typ == TYP_STR ) {
                
                rStat = glb -> env -> setEnvVar( envName, rExpr.strVal );
            }
            else if ( rExpr.typ == TYP_EXT_ADR ) {
                
                rStat = glb -> env -> setEnvVar( envName, rExpr.seg, rExpr.ofs );
            }
            else if (( rExpr.typ == TYP_SYM ) && ( rExpr.tokId == TOK_NIL )) {
                
                rStat = glb -> env -> removeEnvVar( envName );
            }
        }
    }
}

//------------------------------------------------------------------------------------------------------------
// Execute commands from a file command. The actual work is done in the "execCmdsFromFile" routine.
//
// EXEC "<filename>"
//------------------------------------------------------------------------------------------------------------
void DrvCmds::execFileCmd( ) {
    
    if ( glb -> tok -> tokTyp( ) == TYP_STR ) {
        
        execCmdsFromFile( glb -> tok -> tokStr( ));
    }
    else throw( ERR_EXPECTED_FILE_NAME );
}

//------------------------------------------------------------------------------------------------------------
// Load physical memory command.
//
// LMF <path>
//
// ??? when we load a memory image, is tha just a binary block at an address ? Purpose ?
// ??? this will perhaps be better done via load an image from the assembler.
//------------------------------------------------------------------------------------------------------------
void DrvCmds::loadPhysMemCmd( ) {
    
    fprintf( stdout, "The Load Physical Memory command... under construction\n" );
}

//------------------------------------------------------------------------------------------------------------
// Save physical memory command.
//
// SMF <path>
//
// ??? when we save a memory image, how to load it back ? Purpose ?
//------------------------------------------------------------------------------------------------------------
void DrvCmds::savePhysMemCmd( ) {
    
    fprintf( stdout, "The Save Physical Memory command... under construction\n" );
}

//------------------------------------------------------------------------------------------------------------
// Reset command.
//
// RESET ( CPU | MEM | STATS | ALL )
//
// ??? when and what statistics to also reset ?
// ??? what if thee is a unified cache outside the CPU ?
//------------------------------------------------------------------------------------------------------------
void DrvCmds::resetCmd( ) {
    
    if ( glb -> tok -> tokTyp( ) == TYP_SYM ) {
        
        switch( glb -> tok -> tokId( )) {
                
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
// RUN
//------------------------------------------------------------------------------------------------------------
void DrvCmds::runCmd( ) {
    
    fprintf( stdout, "RUN command to come ... \n" );
}

//------------------------------------------------------------------------------------------------------------
// Step command. The command will execute one instruction. Default is one instruction. There is an ENV
// variable that will set the default to be a single clock step.
//
// STEP [ <stes> ] [ "," "I" | "C" ]
//------------------------------------------------------------------------------------------------------------
void DrvCmds::stepCmd( ) {
    
    DrvExpr  rExpr;
    uint32_t numOfSteps = 1;
    
    if ( glb -> tok -> tokTyp( ) == TYP_NUM ) {
        
        glb -> eval -> parseExpr( &rExpr );
        
        if ( rExpr.typ == TYP_NUM ) numOfSteps = rExpr.numVal;
        else throw ( ERR_EXPECTED_STEPS );
    }
    
    if ( glb -> tok -> tokId( ) == TOK_COMMA ) {
        
        glb -> tok -> nextToken( );
        if      ( glb -> tok -> tokId( ) == TOK_I ) glb -> cpu -> instrStep( numOfSteps );
        else if ( glb -> tok -> tokId( ) == TOK_C ) glb -> cpu -> clockStep( numOfSteps );
        else                                        throw ( ERR_INVALID_STEP_OPTION );
    }
    
    checkEOS( );
    
    if ( glb -> env -> getEnvVarBool((char *) ENV_STEP_IN_CLOCKS )) glb -> cpu -> clockStep( 1 );
    else                                                            glb -> cpu -> instrStep( 1 );
}

//------------------------------------------------------------------------------------------------------------
// Writeline command.
//
// WL <epr> [ "," <rdx> ]
//------------------------------------------------------------------------------------------------------------
void DrvCmds::writeLineCmd( ) {
    
    DrvExpr  rExpr;
    int      rdx = glb -> env -> getEnvVarInt((char *) ENV_RDX_DEFAULT );
    
    glb -> eval -> parseExpr( &rExpr );
    
    if ( glb -> tok -> tokId( ) == TOK_COMMA ) {
        
        glb -> tok -> nextToken( );
        
        if (( glb -> tok -> tokId( ) == TOK_HEX ) ||
            ( glb -> tok -> tokId( ) == TOK_OCT ) ||
            ( glb -> tok -> tokId( ) == TOK_DEC )) {
            
            rdx = glb -> tok -> tokVal( );
            
            glb -> tok -> nextToken( );
        }
        else if ( glb -> tok -> tokId( ) == TOK_EOS ) {
            
            rdx = glb -> env -> getEnvVarInt((char *) ENV_RDX_DEFAULT );
        }
        else throw ( ERR_INVALID_FMT_OPT );
    }
    
    checkEOS( );
    
    switch ( rExpr.typ ) {
            
        case TYP_NUM: {
            
            if      ( rdx == 10 )  fprintf( stdout, "%d", rExpr.numVal );
            else if ( rdx == 8  )  fprintf( stdout, "%#012o", rExpr.numVal );
            else if ( rdx == 16 )  {
                
                if ( rExpr.numVal == 0 ) fprintf( stdout, "0x0" );
                else fprintf( stdout, "%#010x", rExpr.numVal );
            }
            else fprintf( stdout, "**num**" );
            
            fprintf( stdout, "\n" );
            
        } break;
       
        case TYP_STR: printf( "\"%s\"\n", rExpr.strVal ); break;
       
        default: throw (  ERR_INVALID_EXPR );
    }
}

//------------------------------------------------------------------------------------------------------------
// Disassemble command.
//
// DIS <instr> [ "," <rdx> ]
//------------------------------------------------------------------------------------------------------------
void DrvCmds::disAssembleCmd( ) {
    
    DrvExpr     rExpr;
    uint32_t    instr   = 0;
    int         rdx     = glb -> env -> getEnvVarInt((char *) ENV_RDX_DEFAULT );
    
    glb -> eval -> parseExpr( &rExpr );
    
    if ( rExpr.typ == TYP_NUM ) {
        
        instr = rExpr.numVal;
        
        if ( glb -> tok -> tokId( ) == TOK_COMMA ) {
            
            glb -> tok -> nextToken( );
            
            if (( glb -> tok -> tokId( ) == TOK_HEX ) ||
                ( glb -> tok -> tokId( ) == TOK_OCT ) ||
                ( glb -> tok -> tokId( ) == TOK_DEC )) {
                
                rdx = glb -> tok -> tokVal( );
                
                glb -> tok -> nextToken( );
            }
            else if ( glb -> tok -> tokId( ) == TOK_EOS ) {
                
                rdx = glb -> env -> getEnvVarInt((char *) ENV_RDX_DEFAULT );
            }
            else throw ( ERR_INVALID_FMT_OPT );
        }
        
        checkEOS( );
        
        glb -> disAsm -> displayInstr( instr, rdx );
        fprintf( stdout, "\n" );
    }
    else throw ( ERR_EXPECTED_INSTR_VAL );
}

//------------------------------------------------------------------------------------------------------------
// Display register command. This is a rather versatile command, which displays register set, register and
// all of them in one format.
//
// DR [ <regSet>|<reg> ] [ "," <fmt> ]
//
// ??? PSTATE regs and FD Stage Regs are the same ?????
//------------------------------------------------------------------------------------------------------------
void DrvCmds::displayRegCmd( ) {
    
    int     rdx         = glb -> env -> getEnvVarInt((char *) ENV_RDX_DEFAULT );
    TypeId  regSetId    = TYP_GREG;
    TokId   regId       = GR_SET;
    int     regNum      = 0;
  
    if ( glb -> tok -> tokId( ) != TOK_EOS ) {
        
        if (( glb -> tok -> tokTyp( ) == TYP_GREG )        ||
            ( glb -> tok -> tokTyp( ) == TYP_SREG )        ||
            ( glb -> tok -> tokTyp( ) == TYP_CREG )        ||
            ( glb -> tok -> tokTyp( ) == TYP_PSTATE_PREG ) ||
            ( glb -> tok -> tokTyp( ) == TYP_FD_PREG )     ||
            ( glb -> tok -> tokTyp( ) == TYP_MA_PREG )     ||
            ( glb -> tok -> tokTyp( ) == TYP_EX_PREG )     ||
            ( glb -> tok -> tokTyp( ) == TYP_IC_L1_REG )   ||
            ( glb -> tok -> tokTyp( ) == TYP_DC_L1_REG )   ||
            ( glb -> tok -> tokTyp( ) == TYP_UC_L2_REG )   ||
            ( glb -> tok -> tokTyp( ) == TYP_ITLB_REG )    ||
            ( glb -> tok -> tokTyp( ) == TYP_DTLB_REG )) {
            
            regSetId    = glb -> tok -> tokTyp( );
            regId       = glb -> tok -> tokId( );
            regNum      = glb -> tok -> tokVal( );
        }
        else throw( ERR_EXPECTED_REG_OR_SET );
        
        if ( glb -> tok -> tokId( ) == TOK_COMMA ) {
            
            glb -> tok -> nextToken( );
            
            if (( glb -> tok -> tokId( ) == TOK_HEX ) ||
                ( glb -> tok -> tokId( ) == TOK_OCT ) ||
                ( glb -> tok -> tokId( ) == TOK_DEC )) {
                
                rdx = glb -> tok -> tokVal( );
            }
            else if ( glb -> tok -> tokId( ) == TOK_EOS ) {
                
                rdx = glb -> env -> getEnvVarInt((char *) ENV_RDX_DEFAULT );
            }
            else throw ( ERR_INVALID_FMT_OPT );
        }
    }
    
    switch( regSetId ) {
            
        case TYP_GREG: {
            
            if ( regId == GR_SET ) glb -> lineDisplay -> displayGeneralRegSet( rdx );
            else glb -> lineDisplay -> displayWord( glb -> cpu -> getReg( RC_GEN_REG_SET, regNum ), rdx );
        
        } break;
            
        case TYP_SREG: {
            
            if ( regId == SR_SET ) glb -> lineDisplay -> displaySegmentRegSet( rdx );
            else glb -> lineDisplay -> displayWord( glb -> cpu -> getReg( RC_SEG_REG_SET, regNum ), rdx );
            
        } break;
            
        case TYP_CREG: {
            
            if ( regId == CR_SET ) glb -> lineDisplay -> displayControlRegSet( rdx );
            else glb -> lineDisplay -> displayWord( glb -> cpu -> getReg( RC_CTRL_REG_SET, regNum ), rdx );
            
        } break;
            
        case TYP_IC_L1_REG: {
            
            if ( regId == IC_L1_SET ) glb -> lineDisplay -> displayMemObjRegSet( glb -> cpu -> iCacheL1, rdx );
            else glb -> lineDisplay -> displayWord( glb -> cpu -> getReg( RC_IC_L1_OBJ, regNum ), rdx );
            
        } break;
            
        case TYP_DC_L1_REG: {
            
            if ( regId == DC_L1_SET ) glb -> lineDisplay -> displayMemObjRegSet( glb -> cpu -> dCacheL1, rdx );
            else glb -> lineDisplay -> displayWord( glb -> cpu -> getReg( RC_DC_L1_OBJ, regNum ), rdx );
            
        } break;
            
        case TYP_UC_L2_REG: {
            
            if ( glb -> cpu -> uCacheL2 != nullptr ) {
                
                if ( regId == UC_L2_SET ) glb -> lineDisplay -> displayMemObjRegSet( glb -> cpu -> uCacheL2, rdx );
                else glb -> lineDisplay -> displayWord( glb -> cpu -> getReg( RC_UC_L2_OBJ, regNum ), rdx );
            }
            else throw ( ERR_CACHE_NOT_CONFIGURED );
            
        } break;
            
        case TYP_ITLB_REG: {
            
            if ( regId == ITLB_SET ) glb -> lineDisplay -> displayTlbObjRegSet( glb -> cpu -> iTlb, rdx );
            else glb -> lineDisplay -> displayWord( glb -> cpu -> getReg( RC_ITLB_OBJ, regNum ), rdx );
            
        } break;
            
        case TYP_DTLB_REG: {
            
            if ( regId == DTLB_SET ) glb -> lineDisplay -> displayTlbObjRegSet( glb -> cpu -> dTlb, rdx );
            else glb -> lineDisplay -> displayWord( glb -> cpu -> getReg( RC_DTLB_OBJ, regNum ), rdx );
            
        } break;
            
        case TYP_FD_PREG: {
            
            if ( regId == FD_SET ) glb -> lineDisplay -> displayPlIFetchDecodeRegSet( rdx );
            else glb -> lineDisplay -> displayWord( glb -> cpu -> getReg( RC_FD_PSTAGE, regNum ), rdx );
            
        } break;
            
        case TYP_MA_PREG: {
            
            if ( regId == MA_SET ) glb -> lineDisplay -> displayPlMemoryAccessRegSet( rdx );
            else glb -> lineDisplay -> displayWord( glb -> cpu -> getReg( RC_MA_PSTAGE, regNum ), rdx );
            
        } break;
            
        case TYP_EX_PREG: {
            
            if ( regId == EX_SET ) glb -> lineDisplay -> displayPlExecuteRegSet( rdx );
            else glb -> lineDisplay -> displayWord( glb -> cpu -> getReg( RC_EX_PSTAGE, regNum ), rdx );
            
        } break;
            
        default: ;
    }

    fprintf( stdout, "\n" );
}

//------------------------------------------------------------------------------------------------------------
// Modify register command. This command modifies a register within a register set.
//
// MR <reg> <val>
//------------------------------------------------------------------------------------------------------------
void DrvCmds::modifyRegCmd( ) {
    
    TypeId      regSetId    = TYP_GREG;
    TokId       regId       = TOK_NIL;
    int         regNum      = 0;
    uint32_t    val         = 0;
    DrvExpr     rExpr;
    
    if (( glb -> tok -> tokTyp( ) == TYP_GREG )        ||
        ( glb -> tok -> tokTyp( ) == TYP_SREG )        ||
        ( glb -> tok -> tokTyp( ) == TYP_CREG )        ||
        ( glb -> tok -> tokTyp( ) == TYP_PSTATE_PREG ) ||
        ( glb -> tok -> tokTyp( ) == TYP_FD_PREG )     ||
        ( glb -> tok -> tokTyp( ) == TYP_MA_PREG )     ||
        ( glb -> tok -> tokTyp( ) == TYP_EX_PREG )     ||
        ( glb -> tok -> tokTyp( ) == TYP_IC_L1_REG )   ||
        ( glb -> tok -> tokTyp( ) == TYP_DC_L1_REG )   ||
        ( glb -> tok -> tokTyp( ) == TYP_UC_L2_REG )   ||
        ( glb -> tok -> tokTyp( ) == TYP_ITLB_REG )    ||
        ( glb -> tok -> tokTyp( ) == TYP_DTLB_REG )) {
        
        regSetId    = glb -> tok -> tokTyp( );
        regId       = glb -> tok -> tokId( );
        regNum      = glb -> tok -> tokVal( );
        glb -> tok -> nextToken( );
    }
    else throw ( ERR_INVALID_REG_ID );
    
    if ( glb -> tok -> tokId( ) == TOK_EOS ) throw( ERR_EXPECTED_NUMERIC );
    glb -> eval -> parseExpr( &rExpr );
    
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
// Display TLB entries command.
//
// DTLB (D|I|U) [ <index> ] [ "," <len> ] [ "," <fmt> ] - if no index, list all entries ? practical ?
//------------------------------------------------------------------------------------------------------------
void DrvCmds::displayTLBCmd( ) {
 
    uint32_t    index       = 0;
    uint32_t    len         = 0;
    uint32_t    tlbSize     = 0;
    TokId       tlbTypeId   = TOK_I;
    int         rdx         = glb -> env -> getEnvVarInt((char *) ENV_RDX_DEFAULT );
    
    glb -> tok -> nextToken( );
    
    if ( glb -> tok -> tokId( ) == TOK_I ) {
        
        tlbSize     = glb -> cpu -> iTlb -> getTlbSize( );
        tlbTypeId   = TOK_I;
        glb -> tok -> nextToken( );
    }
    else if ( glb -> tok -> tokId( ) == TOK_D ) {
        
        tlbSize     = glb -> cpu -> dTlb -> getTlbSize( );
        tlbTypeId   = TOK_D;
        glb -> tok -> nextToken( );
    }
    else throw ( ERR_TLB_TYPE );
    
    acceptComma( );
    
    if ( glb -> tok -> tokId( ) == TOK_COMMA ) {
        
        index = 0;
        glb -> tok -> nextToken( );
    }
    else {
        
        DrvExpr rExpr;
        
        glb -> eval -> parseExpr( &rExpr );
        index = rExpr.numVal;
            
        if ( glb -> tok -> tokId( ) == TOK_COMMA ) glb -> tok -> nextToken( );
    }
    
    if ( glb -> tok -> tokId( ) == TOK_COMMA ) {
        
        len = 1;
        glb -> tok -> nextToken( );
    }
    else {
        
        DrvExpr rExpr;
        
        glb -> eval -> parseExpr( &rExpr );
        
        len = rExpr.numVal;
        if ( glb -> tok -> tokId( ) == TOK_COMMA ) glb -> tok -> nextToken( );
    }
    
    if ( glb -> tok -> tokId( ) == TOK_COMMA ) {
        
        glb -> tok -> nextToken( );
        
        if (( glb -> tok -> tokId( ) == TOK_HEX ) ||
            ( glb -> tok -> tokId( ) == TOK_OCT ) ||
            ( glb -> tok -> tokId( ) == TOK_DEC )) {
            
            rdx = glb -> tok -> tokVal( );
        }
        else if ( glb -> tok -> tokId( ) == TOK_EOS ) {
            
            rdx = glb -> env -> getEnvVarInt((char *) ENV_RDX_DEFAULT );
        }
        else throw ( ERR_INVALID_FMT_OPT );
    }
    
    if (( index > tlbSize ) || ( index + len > tlbSize )) throw ( ERR_TLB_SIZE_EXCEEDED );
    if (( index == 0 ) && ( len == 0 )) len = tlbSize;
    
    if      ( tlbTypeId == TOK_I ) glb -> lineDisplay -> displayTlbEntries( glb -> cpu -> iTlb, index, len, rdx );
    else if ( tlbTypeId == TOK_D ) glb -> lineDisplay -> displayTlbEntries( glb -> cpu -> dTlb, index, len, rdx );
    
    fprintf( stdout, "\n" );
}

//------------------------------------------------------------------------------------------------------------
// Purge from TLB command.
//
// P-TLB <I|D|U> <extAdr>
//------------------------------------------------------------------------------------------------------------
void DrvCmds::purgeTLBCmd( ) {
    
    DrvExpr     rExpr;
    uint32_t    tlbSize     = 0;
    TokId       tlbTypeId   = TOK_I;
    
    if ( glb -> tok -> tokId( ) == TOK_I ) {
        
        tlbSize     = glb -> cpu -> iTlb -> getTlbSize( );
        tlbTypeId   = TOK_I;
        glb -> tok -> nextToken( );
    }
    else if ( glb -> tok -> tokId( ) == TOK_D ) {
        
        tlbSize     = glb -> cpu -> dTlb -> getTlbSize( );
        tlbTypeId   = TOK_D;
        glb -> tok -> nextToken( );
    }
    else throw ( ERR_TLB_TYPE );
    
    glb -> eval -> parseExpr( &rExpr );
    
    if ( rExpr.typ == TYP_EXT_ADR ) {
        
        CpuTlb *tlbPtr = ( tlbTypeId == TOK_I ) ? glb -> cpu -> iTlb : glb -> cpu -> dTlb;
        if ( ! tlbPtr -> purgeTlbEntryData( rExpr.seg, rExpr.ofs )) throw ( ERR_TLB_PURGE_OP );
    }
    else throw ( ERR_EXPECTED_EXT_ADR );
}

//------------------------------------------------------------------------------------------------------------
// Insert into TLB command.
//
// I-TLB <D|I|U> <extAdr> <arg-acc> <arg-adr>
//------------------------------------------------------------------------------------------------------------
void DrvCmds::insertTLBCmd( ) {
    
    DrvExpr     rExpr;
    uint32_t    tlbSize         = 0;
    TokId       tlbTypeId       = TOK_I;
    uint32_t    seg             = 0;
    uint32_t    ofs             = 0;
    uint32_t    argAcc          = 0;
    uint32_t    argAdr          = 0;
    
    if ( glb -> tok -> tokId( ) == TOK_I ) {
        
        tlbSize     = glb -> cpu -> iTlb -> getTlbSize( );
        tlbTypeId   = TOK_I;
        glb -> tok -> nextToken( );
    }
    else if ( glb -> tok -> tokId( ) == TOK_D ) {
        
        tlbSize     = glb -> cpu -> dTlb -> getTlbSize( );
        tlbTypeId   = TOK_D;
        glb -> tok -> nextToken( );
    }
    else throw ( ERR_TLB_TYPE );
    
    glb -> eval -> parseExpr( &rExpr );
    
    if ( rExpr.typ == TYP_EXT_ADR ) {
        
        seg = rExpr.seg;
        ofs = rExpr.ofs;
    }
    else throw ( ERR_EXPECTED_EXT_ADR );
    
    glb -> eval -> parseExpr( &rExpr );
    
    if ( rExpr.typ == TYP_NUM ) argAcc = rExpr.numVal;
    else throw ( ERR_TLB_ACC_DATA );
    
    glb -> eval -> parseExpr( &rExpr );
    
    if ( rExpr.typ == TYP_NUM ) argAcc = rExpr.numVal;
    else throw ( ERR_TLB_ADR_DATA );
    
    CpuTlb *tlbPtr = ( tlbTypeId == TOK_I ) ? glb -> cpu -> iTlb : glb -> cpu -> dTlb;
    if ( ! tlbPtr -> insertTlbEntryData( seg, ofs, argAcc, argAdr )) throw ( ERR_TLB_INSERT_OP );
}

//------------------------------------------------------------------------------------------------------------
// Display cache entries command.
//
// D-CACHE ( I|D|U ) "," [ <index> ] [ "," <len> ] [ ", " <fmt> ]
//------------------------------------------------------------------------------------------------------------
void DrvCmds::displayCacheCmd( ) {
    
    TokId       cacheTypeId     = TOK_I;
    uint32_t    cacheSize       = 0;
    CpuMem      *cPtr           = nullptr;
    uint32_t    index           = 0;
    uint32_t    len             = 0;
    int         rdx             = glb -> env -> getEnvVarInt((char *) ENV_RDX_DEFAULT );
    
    if ( glb -> tok -> tokId( ) == TOK_I ) {
        
        cacheSize     = glb -> cpu -> iTlb -> getTlbSize( );
        cacheTypeId   = TOK_I;
        glb -> tok -> nextToken( );
    }
    else if ( glb -> tok -> tokId( ) == TOK_D ) {
        
        cacheSize     = glb -> cpu -> iCacheL1 -> getMemSize( );
        cacheTypeId   = TOK_D;
        glb -> tok -> nextToken( );
    }
    else if ( glb -> tok -> tokId( ) == TOK_U ) {
        
        if ( glb -> cpu -> uCacheL2 != nullptr ) {
            
            cacheSize     = glb -> cpu -> uCacheL2 -> getMemSize( );
            cacheTypeId   = TOK_U;
            glb -> tok -> nextToken( );
        }
        else throw ( ERR_CACHE_NOT_CONFIGURED );
    }
    else throw ( ERR_CACHE_TYPE );
    
    acceptComma( );
    
    if ( glb -> tok -> tokId( ) == TOK_COMMA ) {
        
        index = 0;
        glb -> tok -> nextToken( );
    }
    else {
        
        DrvExpr rExpr;
        glb -> eval -> parseExpr( &rExpr );
        
        if ( rExpr.typ == TYP_NUM ) {
            
            index = rExpr.numVal;
            if ( glb -> tok -> tokId( ) == TOK_COMMA ) glb -> tok -> nextToken( );
        }
        else throw ( ERR_EXPECTED_NUMERIC );
    }
    
    if ( glb -> tok -> tokId( ) == TOK_COMMA ) {
        
        len = 1;
        glb -> tok -> nextToken( );
    }
    else {
        
        DrvExpr rExpr;
        
        glb -> eval -> parseExpr( &rExpr );
        
        if ( rExpr.typ == TYP_NUM ) {
            
            len = rExpr.numVal;
            if ( glb -> tok -> tokId( ) == TOK_COMMA ) glb -> tok -> nextToken( );
        }
        else throw ( ERR_EXPECTED_NUMERIC );
    }
    
    if ( glb -> tok -> tokId( ) == TOK_COMMA ) {
        
        glb -> tok -> nextToken( );
        
        if (( glb -> tok -> tokId( ) == TOK_HEX ) ||
            ( glb -> tok -> tokId( ) == TOK_OCT ) ||
            ( glb -> tok -> tokId( ) == TOK_DEC )) {
            
            rdx = glb -> tok -> tokVal( );
        }
        else if ( glb -> tok -> tokId( ) == TOK_EOS ) {
            
            rdx = glb -> env -> getEnvVarInt((char *) ENV_RDX_DEFAULT );
        }
        else throw ( ERR_INVALID_FMT_OPT );
    }
    
    if (( index > cacheSize ) || ( index + len > cacheSize )) throw( ERR_CACHE_SIZE_EXCEEDED );
   
    if (( index == 0 ) && ( len == 0 )) len = cacheSize;
  
    if ( cPtr != nullptr ) {
        
        uint32_t blockEntries = cPtr -> getBlockEntries( );
        
        if (( index > blockEntries ) || ( index + len > blockEntries )) {
            
            throw( ERR_CACHE_SIZE_EXCEEDED );
        }
        
        if (( index == 0 ) && ( len == 0 )) len = blockEntries;
        
        glb -> lineDisplay -> displayCacheEntries( cPtr, index, len, rdx );
        
        fprintf( stdout, "\n" );
    }
}

//------------------------------------------------------------------------------------------------------------
// Purges a cache line from the cache.
//
// P-CACHE <I|D|U> <index> <set> [<flush>]
//------------------------------------------------------------------------------------------------------------
void DrvCmds::purgeCacheCmd( ) {
    
    TokId       cacheTypeId     = TOK_I;
    uint32_t    cacheSize       = 0;
    CpuMem      *cPtr           = nullptr;
    uint32_t    index           = 0;
    uint32_t    set             = 0;
   
    if ( glb -> tok -> tokId( ) == TOK_I ) {
        
        cacheSize     = glb -> cpu -> iTlb -> getTlbSize( );
        cacheTypeId   = TOK_I;
        glb -> tok -> nextToken( );
    }
    else if ( glb -> tok -> tokId( ) == TOK_D ) {
        
        cacheSize     = glb -> cpu -> iCacheL1 -> getMemSize( );
        cacheTypeId   = TOK_D;
        glb -> tok -> nextToken( );
    }
    else if ( glb -> tok -> tokId( ) == TOK_U ) {
        
        if ( glb -> cpu -> uCacheL2 != nullptr ) {
            
            cacheSize     = glb -> cpu -> uCacheL2 -> getMemSize( );
            cacheTypeId   = TOK_U;
            glb -> tok -> nextToken( );
        }
        else throw ( ERR_CACHE_NOT_CONFIGURED );
    }
    else throw ( ERR_CACHE_TYPE );
    
    acceptComma( );
    
    if ( glb -> tok -> tokId( ) == TOK_COMMA ) {
        
        index = 0;
        glb -> tok -> nextToken( );
    }
    else {
        
        DrvExpr rExpr;
        
        glb -> eval -> parseExpr( &rExpr );
        
        if ( rExpr.typ == TYP_NUM ) {
            
            index = rExpr.numVal;
            if ( glb -> tok -> tokId( ) == TOK_COMMA ) glb -> tok -> nextToken( );
        }
        else throw (ERR_EXPECTED_NUMERIC );
    }
    
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
// Display absolute memory command. The memory address is a byte address. The offset address is a byte address,
// the length is measured in bytes, rounded up to the a word size. We accept any address and length and only
// check that the offset plus length does not exceed the address space. The display routines, who will call
// the actual memory object will take care of gaps in the memory address range. The format specifier will
// allow for HEX, OCTAL, DECIMAL and CODE. In the case of the code option, the default number format option
// is used for showing the offset value.
//
// DA <ofs> [ "," <len> [ "," <rdx> ]]
//------------------------------------------------------------------------------------------------------------
void DrvCmds::displayAbsMemCmd( ) {
    
    DrvExpr     rExpr;
    uint32_t    ofs     = 0;
    uint32_t    len     = 1;
    int         rdx     = glb -> env -> getEnvVarInt((char *) ENV_RDX_DEFAULT );
    bool        asCode  = false;
    
    glb -> eval -> parseExpr( &rExpr );
    
    if ( rExpr.typ == TYP_NUM ) ofs = rExpr.numVal;
    else throw ( ERR_EXPECTED_START_OFS );
   
    if ( glb -> tok -> tokId( ) == TOK_COMMA ) {
        
        glb -> tok -> nextToken( );
        
        if ( glb -> tok -> isToken( TOK_COMMA )) {
            
            len = 4;
        }
        else {
            
            glb -> eval -> parseExpr( &rExpr );
            
            if ( rExpr.typ == TYP_NUM ) len = rExpr.numVal;
            else throw ( ERR_EXPECTED_LEN );
        }
    }
   
    if ( glb -> tok -> tokId( ) == TOK_COMMA ) {
        
        glb -> tok -> nextToken( );
        
        if (( glb -> tok -> tokId( ) == TOK_HEX ) ||
            ( glb -> tok -> tokId( ) == TOK_OCT ) ||
            ( glb -> tok -> tokId( ) == TOK_DEC )) {
            
            rdx = glb -> tok -> tokVal( );
        }
        else if ( glb -> tok -> tokId( ) == TOK_CODE ) {
            
            asCode = true;
        }
        else if ( glb -> tok -> tokId( ) == TOK_EOS ) {
            
            rdx = glb -> env -> getEnvVarInt((char *) ENV_RDX_DEFAULT );
        }
        else throw ( ERR_INVALID_FMT_OPT );
        
        glb -> tok -> nextToken( );
    }
    
    checkEOS( );
        
    if (((uint64_t) ofs + len ) <= UINT32_MAX ) {
        
        if ( asCode ) {
            
            glb -> lineDisplay -> displayAbsMemContentAsCode( ofs, len,
                                                glb -> env -> getEnvVarInt((char *) ENV_RDX_DEFAULT ));
        }
        else glb -> lineDisplay -> displayAbsMemContent( ofs, len, rdx );
    }
    else throw ( ERR_OFS_LEN_LIMIT_EXCEEDED );
}

//------------------------------------------------------------------------------------------------------------
// Modify absolute memory command. This command accepts data values for up to eight consecutive locations.
// We also use this command to populate physical memory from a script file.
//
// MA <ofs> "," <val>
//------------------------------------------------------------------------------------------------------------
void DrvCmds::modifyAbsMemCmd( ) {
    
    DrvExpr     rExpr;
    uint32_t    ofs         = 0;
    uint32_t    val         = 0;
    CpuMem      *physMem    = glb -> cpu -> physMem;
    CpuMem      *pdcMem     = glb -> cpu -> pdcMem;
    CpuMem      *ioMem      = glb -> cpu -> ioMem;
    CpuMem      *mem        = nullptr;
    
    glb -> eval -> parseExpr( &rExpr );
    
    if ( rExpr.typ == TYP_NUM ) ofs = rExpr.numVal;
    else throw ( ERR_EXPECTED_OFS );
    
    acceptComma( );
    glb -> eval -> parseExpr( &rExpr );
    
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
// Global windows commands. There are handlers for turning windows on, off and set them back to their default
// values. We also support two stacks of windows next to each other.
//
//------------------------------------------------------------------------------------------------------------
void DrvCmds::winOnCmd( ) {
    
    winModeOn = true;
    glb -> winDisplay -> windowsOn( );
    glb -> winDisplay -> reDraw( true );
}

void DrvCmds::winOffCmd( ) {
    
    if ( winModeOn ) {
        
        winModeOn = false;
        glb -> winDisplay -> windowsOff( );
    }
    else throw ( ERR_NOT_IN_WIN_MODE );
}

void DrvCmds::winDefCmd( ) {
    
    if ( winModeOn ) {
        
        glb -> winDisplay -> windowDefaults( );
        glb -> winDisplay -> reDraw( true );
    }
    else throw ( ERR_NOT_IN_WIN_MODE );
}

void DrvCmds::winStacksEnable( ) {
    
    if ( winModeOn ) {
        
        glb -> winDisplay -> winStacksEnable( true );
        glb -> winDisplay -> reDraw( true );
    }
    else throw ( ERR_NOT_IN_WIN_MODE );
}

void DrvCmds::winStacksDisable( ) {
    
    if ( winModeOn ) {
        
        glb -> winDisplay -> winStacksEnable( false );
        glb -> winDisplay -> reDraw( true );
    }
    else throw ( ERR_NOT_IN_WIN_MODE );
}

//------------------------------------------------------------------------------------------------------------
// Window current command. User definable windows are controlled by their window number. To avoid typing this
// number all the time for a user window command, a user window can explicitly be set as the current command.
//
// WC <winNum>
//------------------------------------------------------------------------------------------------------------
void DrvCmds::winCurrentCmd( ) {
    
    if ( ! winModeOn ) throw ( ERR_NOT_IN_WIN_MODE );
       
    DrvExpr rExpr;
    glb -> eval -> parseExpr( &rExpr );
    
    if ( rExpr.typ == TYP_NUM ) {
        
        if ( glb -> winDisplay -> validWindowNum( rExpr.numVal )) {
            
            glb -> winDisplay -> windowCurrent( rExpr.numVal );
        }
        else throw ( ERR_INVALID_WIN_ID );
    }
    else throw ( ERR_EXPECTED_WIN_ID );
}

//------------------------------------------------------------------------------------------------------------
// Windows enable and disable. When enabled, a window does show up on the screen. The window number is
// optional, used for user definable windows.
//
// <win>E [<winNum>]
// <win>D [<winNum>]
//------------------------------------------------------------------------------------------------------------
void DrvCmds::winEnableCmd( TokId winCmd ) {
    
    int winNum = 0;
    
    if ( ! winModeOn ) throw ( ERR_NOT_IN_WIN_MODE );
    
    if ( glb -> tok -> tokId( ) != TOK_EOS ) {
        
        DrvExpr rExpr;
        glb -> eval -> parseExpr( &rExpr );
        
        if ( rExpr.typ == TYP_NUM ) winNum = rExpr.numVal;
        else throw ( ERR_EXPECTED_WIN_ID );
    }
    
    if ( glb -> winDisplay -> validWindowNum( winNum )) {
        
        glb -> winDisplay -> windowEnable( winCmd, winNum );
        glb -> winDisplay -> reDraw( true );
    }
    else throw ( ERR_INVALID_WIN_ID );
}

void DrvCmds::winDisableCmd( TokId winCmd ) {
        
    int winNum = 0;
    
    if ( ! winModeOn ) throw ( ERR_NOT_IN_WIN_MODE );
    
    if ( glb -> tok -> tokId( ) != TOK_EOS ) {
        
        DrvExpr rExpr;
        glb -> eval -> parseExpr( &rExpr );
        
        if ( rExpr.typ == TYP_NUM ) winNum = rExpr.numVal;
        else throw ( ERR_EXPECTED_WIN_ID );
    }
    
    if ( glb -> winDisplay -> validWindowNum( winNum )) {
        
        glb -> winDisplay -> windowDisable( winCmd, winNum );
        glb -> winDisplay -> reDraw( true );
    }
    else throw ( ERR_INVALID_WIN_ID );
}

//------------------------------------------------------------------------------------------------------------
// Windows radix. This command sets the radix for a given window. We parse the command and the format
// option and pass the tokens to the screen handler. The window number is optional, used for user definable
// windows.
//
// <win>R [ <radix> [ "," <winNum>]]
//------------------------------------------------------------------------------------------------------------
void DrvCmds::winSetRadixCmd( TokId winCmd ) {
        
    if ( ! winModeOn ) throw ( ERR_NOT_IN_WIN_MODE );
                                  
    DrvExpr rExpr;
    int     winNum  = 0;
    int     rdx     = 16;
        
    if ( glb -> tok -> tokId( ) == TOK_COMMA ) {
        
        rdx = glb -> env -> getEnvVarInt((char *) ENV_RDX_DEFAULT );
        glb -> tok -> nextToken( );
    }
    else {
        
        glb -> eval -> parseExpr( &rExpr );
        if ( rExpr.typ == TYP_NUM ) rdx = setRadix( rExpr.numVal );
        else throw ( ERR_INVALID_RADIX );
    }
    
    if ( glb -> tok -> tokId( ) == TOK_COMMA ) {
        
        glb -> tok -> nextToken( );
        glb -> eval -> parseExpr( &rExpr );
        
        if ( rExpr.typ == TYP_NUM ) {
            
            winNum = rExpr.numVal;
            glb -> tok -> nextToken( );
        }
        else throw ( ERR_INVALID_RADIX );
    }
    
    if ( glb -> winDisplay -> validWindowNum( winNum )) {
        
        glb -> winDisplay -> windowRadix( winCmd, rdx, winNum );
    }
    else throw ( ERR_INVALID_WIN_ID );
}

//------------------------------------------------------------------------------------------------------------
// Window scrolling. This command advances the item address of a scrollable window by the number of lines
// multiplied by the number of items on a line forward or backward. The meaning of the item address and line
// items is window dependent. The window number is optional, used for user definable windows. If omitted,
// we mean the current window.
//
// <win>F [<items> [ "," <winNum>]]
// <win>B [<items> [ "," <winNum>]]
//------------------------------------------------------------------------------------------------------------
void DrvCmds::winForwardCmd( TokId winCmd ) {
    
    DrvExpr rExpr;
    int     winItems = 0;
    int     winNum   = 0;
    
    if ( ! winModeOn ) throw ( ERR_NOT_IN_WIN_MODE );
    
    if ( glb -> tok -> tokId( ) == TOK_EOS ) {
        
        glb -> winDisplay -> windowForward( winCmd, winItems, winNum  );
    }
    
    glb -> eval -> parseExpr( &rExpr );
    
    if ( rExpr.typ == TYP_NUM ) winItems = rExpr.numVal;
    else throw ( ERR_INVALID_NUM );
    
    if ( glb -> tok -> tokId( ) == TOK_COMMA ) {
        
        glb ->  tok -> nextToken( );
        
        if ( glb -> tok -> tokTyp( ) == TYP_NUM )winNum = glb -> tok -> tokVal( );
        else throw ( ERR_INVALID_WIN_ID );
        
        if ( ! glb -> winDisplay -> validWindowNum( winNum )) throw ( ERR_INVALID_WIN_ID );
    }
    else winNum = 0;
    
    glb -> winDisplay -> windowForward( winCmd, winItems, winNum  );
}

void DrvCmds::winBackwardCmd( TokId winCmd ) {
    
    DrvExpr rExpr;
    int     winItems = 0;
    int     winNum   = 0;
    
    if ( ! winModeOn ) throw ( ERR_NOT_IN_WIN_MODE );
    
    if ( glb -> tok -> tokId( ) == TOK_EOS ) {
        
        glb -> winDisplay -> windowBackward( winCmd, winItems, winNum  );
    }
    
    glb -> eval -> parseExpr( &rExpr );
    
    if ( rExpr.typ == TYP_NUM ) winItems = rExpr.numVal;
    else throw ( ERR_INVALID_NUM );
    
    if ( glb -> tok -> tokId( ) == TOK_COMMA ) {
        
        glb -> tok -> nextToken( );
        
        if ( glb -> tok -> tokTyp( ) == TYP_NUM )winNum = glb -> tok -> tokVal( );
        else throw ( ERR_INVALID_WIN_ID );
        
        if ( ! glb -> winDisplay -> validWindowNum( winNum )) throw ( ERR_INVALID_WIN_ID );
    }
    else winNum = 0;
    
    glb -> winDisplay -> windowBackward( winCmd, winItems, winNum  );
}

//------------------------------------------------------------------------------------------------------------
// Window home. Each window has a home item address, which was set at window creation or trough a non-zero
// value passed to this command. The command sets the window item address to this value. The meaning of the
// item address is window dependent. The window number is optional, used for user definable windows.
//
// <win>H [<pos> [ "," <winNum>]]
//------------------------------------------------------------------------------------------------------------
void DrvCmds::winHomeCmd( TokId winCmd ) {
    
    DrvExpr rExpr;
    int     winPos   = 0;
    int     winNum   = 0;
    
    if ( ! winModeOn ) throw ( ERR_NOT_IN_WIN_MODE );
    
    if ( glb -> tok -> tokId( ) == TOK_EOS ) {
        
        glb -> winDisplay -> windowHome( winCmd, winPos, winNum  );
    }
    
    glb -> eval -> parseExpr( &rExpr );
    
    if ( rExpr.typ == TYP_NUM ) winPos = rExpr.numVal;
    else throw ( ERR_INVALID_NUM );
    
    if ( glb -> tok -> tokId( ) == TOK_COMMA ) {
        
        glb -> tok -> nextToken( );
        
        if ( glb -> tok -> tokTyp( ) == TYP_NUM )winNum = glb -> tok -> tokVal( );
        else throw ( ERR_INVALID_WIN_ID );
        
        if ( ! glb -> winDisplay -> validWindowNum( winNum )) throw ( ERR_INVALID_WIN_ID );
    }
    else winNum = 0;
    
    glb -> winDisplay -> windowHome( winCmd, winPos, winNum  );
}

//------------------------------------------------------------------------------------------------------------
// Window jump. The window jump command sets the item address to the position argument. The meaning of the
// item address is window dependent. The window number is optional, used for user definable windows.
//
// <win>J [<pos> [ "," <winNum>]]
//------------------------------------------------------------------------------------------------------------
void DrvCmds::winJumpCmd( TokId winCmd ) {
    
    DrvExpr rExpr;
    int     winPos   = 0;
    int     winNum   = 0;
    
    if ( ! winModeOn ) throw ( ERR_NOT_IN_WIN_MODE );
    
    if ( glb -> tok -> tokId( ) == TOK_EOS ) {
        
        glb -> winDisplay -> windowHome( winCmd, winPos, winNum  );
    }
    
    glb -> eval -> parseExpr( &rExpr );
    
    if ( rExpr.typ == TYP_NUM ) winPos = rExpr.numVal;
    else throw ( ERR_INVALID_NUM );
    
    if ( glb -> tok -> tokId( ) == TOK_COMMA ) {
        
        glb -> tok -> nextToken( );
        
        if ( glb -> tok -> tokTyp( ) == TYP_NUM )winNum = glb -> tok -> tokVal( );
        else throw ( ERR_INVALID_WIN_ID );
        
        if ( ! glb -> winDisplay -> validWindowNum( winNum )) throw ( ERR_INVALID_WIN_ID );
    }
    else winNum = 0;
    
    glb -> winDisplay -> windowJump( winCmd, winPos, winNum  );
}

//------------------------------------------------------------------------------------------------------------
// Set window lines. This command sets the the number of rows for a window. The number includes the banner
// line. The window number is optional, used for user definable windows.
//
// <win>L [<lines> [ "," <winNum>]]
//------------------------------------------------------------------------------------------------------------
void DrvCmds::winSetRowsCmd( TokId winCmd ) {
    
    DrvExpr rExpr;
    int     winLines    = 0;
    int     winNum      = 0;
    
    if ( ! winModeOn ) throw ( ERR_NOT_IN_WIN_MODE );
    
    if ( glb -> tok -> tokId( ) == TOK_EOS ) {
        
        glb -> winDisplay -> windowHome( winCmd, winLines, winNum  );
    }
    
    glb -> eval -> parseExpr( &rExpr );
    
    if ( rExpr.typ == TYP_NUM ) winLines = rExpr.numVal;
    else throw ( ERR_INVALID_NUM );
    
    if ( glb -> tok -> tokId( ) == TOK_COMMA ) {
        
        glb -> tok -> nextToken( );
        
        if ( glb -> tok -> tokTyp( ) == TYP_NUM )winNum = glb -> tok -> tokVal( );
        else throw ( ERR_INVALID_WIN_ID );
        
        if ( ! glb -> winDisplay -> validWindowNum( winNum )) throw ( ERR_INVALID_WIN_ID );
    }
    else winNum = 0;
    
    glb -> winDisplay -> windowSetRows( winCmd, winLines, winNum );
    glb -> winDisplay -> reDraw( true );
}

//------------------------------------------------------------------------------------------------------------
// This command creates a new user window. The window is assigned a free index form the windows list. This
// index is used in all the calls to this window. The window type allows to select from a code window, a
// physical memory window, a TLB and a CACHE window.
//
// WN <winType> [ "," <arg> ]
//------------------------------------------------------------------------------------------------------------
void DrvCmds::winNewWinCmd( ) {
    
    TokId   winType = TOK_NIL;
    char    *argStr = nullptr;
    
    if ( ! winModeOn ) throw ( ERR_NOT_IN_WIN_MODE );
    
    if ( glb -> tok -> tokTyp( ) == TYP_SYM ) {
        
        winType = glb -> tok -> tokId( );
        
        if ((( winType == TOK_PM ) && ( glb -> cpu -> physMem == nullptr ))     ||
            (( winType == TOK_PC ) && ( glb -> cpu -> physMem == nullptr ))     ||
            (( winType == TOK_IT ) && ( glb -> cpu -> iTlb == nullptr ))        ||
            (( winType == TOK_DT ) && ( glb -> cpu -> dTlb == nullptr ))        ||
            (( winType == TOK_IC ) && ( glb -> cpu -> iCacheL1 == nullptr ))    ||
            (( winType == TOK_DC ) && ( glb -> cpu -> dCacheL1 == nullptr ))    ||
            (( winType == TOK_UC ) && ( glb -> cpu -> uCacheL2 == nullptr ))) {
            
            throw ( ERR_WIN_TYPE_NOT_CONFIGURED );
        }
        
        if ( ! glb -> winDisplay -> validUserWindowType( winType ))
            throw ( ERR_INVALID_WIN_TYPE );
        
        glb -> tok -> nextToken( );
    }
    
    if ( glb -> tok -> tokId( ) == TOK_COMMA ) {
        
        glb -> tok -> nextToken( );
        
        if (glb ->  tok -> tokTyp( ) == TYP_STR ) argStr = glb -> tok -> tokStr( );
        else throw ( ERR_INVALID_ARG );
    }
    
    glb -> winDisplay -> windowNew( winType, argStr );
    glb -> winDisplay -> reDraw( true );
}

//------------------------------------------------------------------------------------------------------------
// This command removes  a user defined window or window range from the list of windows. A number of -1 will
// kill all user defined windows.
//
// WK [<winNumStart> [ "," <winNumEnd]] || ( -1 )
//------------------------------------------------------------------------------------------------------------
void DrvCmds::winKillWinCmd( ) {
    
    DrvExpr     rExpr;
    int         winNumStart     = 0;
    int         winNumEnd       = 0;
    
    if ( ! winModeOn ) throw ( ERR_NOT_IN_WIN_MODE );
    
    if ( glb -> tok -> tokId( ) == TOK_EOS ) {
        
        winNumStart = glb -> winDisplay -> getCurrentUserWindow( );
        winNumEnd   = winNumStart;
    }
    else {
        
        glb -> eval -> parseExpr( &rExpr );
        
        if ( rExpr.typ == TYP_NUM ) winNumStart = rExpr.numVal;
        else throw ( ERR_EXPECTED_NUMERIC );
        
        if ( glb -> tok -> tokId( ) == TOK_COMMA ) {
            
            glb -> tok -> nextToken( );
            glb -> eval -> parseExpr( &rExpr );
            
            if ( rExpr.typ == TYP_NUM ) winNumEnd = rExpr.numVal;
            else throw ( ERR_EXPECTED_NUMERIC );
        }
        
        if ( winNumStart == -1 ) {
            
            winNumStart = glb -> winDisplay -> getFirstUserWinIndex( );
            winNumEnd   = glb -> winDisplay -> getLastUserWinIndex( );
        }
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
// WS <stackNum> [ "," <winNumStart> [ "," <winNumEnd ]]
//------------------------------------------------------------------------------------------------------------
void DrvCmds::winSetStackCmd( ) {
    
    DrvExpr rExpr;
    int     stackNum    = 0;
    int     winNumStart = 0;
    int     winNumEnd   = 0;
    
    if ( ! winModeOn ) throw ( ERR_NOT_IN_WIN_MODE );
    
    glb -> eval -> parseExpr( &rExpr );
    
    if ( rExpr.typ == TYP_NUM ) stackNum = rExpr.numVal;
    else throw ( ERR_EXPECTED_STACK_ID );
    
    if ( ! glb -> winDisplay -> validWindowStackNum( stackNum ))
        throw ( ERR_INVALID_WIN_STACK_ID );
        
    if ( glb -> tok -> tokId( ) == TOK_EOS ) {
        
        winNumStart = glb -> winDisplay -> getCurrentUserWindow( );
        winNumEnd   = winNumStart;
    }
    else {
        
        if ( glb -> tok -> tokId( ) == TOK_COMMA ) {
            
            glb -> tok -> nextToken( );
            glb -> eval -> parseExpr( &rExpr );
            
            if ( rExpr.typ == TYP_NUM ) winNumStart = rExpr.numVal;
            else throw ( ERR_EXPECTED_NUMERIC );
            
            if ( glb -> tok -> tokId( ) == TOK_COMMA ) {
                
                glb -> tok -> nextToken( );
                glb -> eval -> parseExpr( &rExpr );
                
                if ( rExpr.typ == TYP_NUM ) winNumEnd = rExpr.numVal;
                else throw ( ERR_EXPECTED_NUMERIC );
            }
            else winNumEnd = winNumStart;
        }
        
        if ( winNumStart == -1 ) {
                
            winNumStart = glb -> winDisplay -> getFirstUserWinIndex( );
            winNumEnd   = glb -> winDisplay -> getLastUserWinIndex( );
        }
    }
    
    if (( ! glb -> winDisplay -> validWindowNum( winNumStart )) ||
        ( ! glb -> winDisplay -> validWindowNum( winNumEnd ))) throw ( ERR_INVALID_WIN_ID );
    
    glb -> winDisplay -> windowSetStack( stackNum, winNumStart, winNumEnd );
    glb -> winDisplay -> reDraw( true );
}

//------------------------------------------------------------------------------------------------------------
// This command toggles through alternate window content, if the window dos support it. An example is the
// cache sets in a two-way associative cache. The toggle command will just flip through the sets.
//
// WT [ <winNum> ]
//------------------------------------------------------------------------------------------------------------
void  DrvCmds::winToggleCmd( ) {
    
    if ( ! winModeOn ) throw ( ERR_NOT_IN_WIN_MODE );
    
    if ( glb -> tok -> tokTyp( ) == TYP_NUM ) {
        
        if ( ! glb -> winDisplay -> validWindowNum( glb -> tok -> tokVal( ))) throw ( ERR_INVALID_WIN_ID );
        glb -> winDisplay -> windowToggle( glb -> tok -> tokVal( ));
    }
    else throw ( ERR_EXPECTED_WIN_ID );
}

//------------------------------------------------------------------------------------------------------------
// This command exchanges the current user window with the user window specified. It allows to change the
// order of the user windows in a stacks.
//
// WX <winNum>
//------------------------------------------------------------------------------------------------------------
void DrvCmds::winExchangeCmd( ) {
    
    if ( ! winModeOn ) throw ( ERR_NOT_IN_WIN_MODE );
    
    if ( glb -> tok -> tokTyp( ) == TYP_NUM ) {
        
        if ( ! glb -> winDisplay -> validWindowNum( glb -> tok -> tokVal( ))) throw ( ERR_INVALID_WIN_ID );
        glb -> winDisplay -> windowExchangeOrder( glb -> tok -> tokVal( ));
    }
    else throw ( ERR_EXPECTED_WIN_ID );
}

//------------------------------------------------------------------------------------------------------------
// Evaluate input line. There are commands, functions, expressions and so on. This routine sets up the
// tokenizer and dispatches based on the first token in the input line.
//
//------------------------------------------------------------------------------------------------------------
void DrvCmds::evalInputLine( char *cmdBuf ) {
    
    try {
        
        if ( strlen( cmdBuf ) > 0 ) {
            
            glb -> tok -> setupTokenizer( cmdBuf, (DrvToken *) cmdTokTab );
            glb -> tok -> nextToken( );
            
            if ( glb -> tok -> isTokenTyp( TYP_CMD )) {
                
                TokId cmdId = glb -> tok -> tokId( );
                
                glb -> tok -> nextToken( );
                
                switch( cmdId ) {
                        
                    case TOK_NIL:                                           break;
                    case CMD_EXIT:          exitCmd( );                     break;
                    case CMD_HELP:          helpCmd( );                     break;
                    case CMD_WHELP:         winHelpCmd( );                  break;
                    case CMD_ENV:           envCmd( );                      break;
                    case CMD_XF:            execFileCmd( );                 break;
                    case CMD_LMF:           loadPhysMemCmd( );              break;
                    case CMD_SMF:           savePhysMemCmd( );              break;
                    case CMD_RESET:         resetCmd( );                    break;
                    case CMD_RUN:           runCmd( );                      break;
                    case CMD_STEP:          stepCmd( );                     break;
                    case CMD_WRITE_LINE:    writeLineCmd( );                break;
                    case CMD_DIS_ASM:       disAssembleCmd( );              break;
                    case CMD_DR:            displayRegCmd( );               break;
                    case CMD_MR:            modifyRegCmd( );                break;
    
                    case CMD_D_TLB:         displayTLBCmd( );               break;
                    case CMD_I_TLB:         insertTLBCmd( );                break;
                    case CMD_P_TLB:         purgeTLBCmd( );                 break;
                    case CMD_D_CACHE:       displayCacheCmd( );             break;
                    case CMD_P_CACHE:       purgeCacheCmd( );               break;
                    case CMD_DA:            displayAbsMemCmd( );            break;
                    case CMD_MA:            modifyAbsMemCmd( );             break;
                        
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
            else throw ( ERR_INVALID_CMD );
        }
    }
    
    catch ( ErrMsgId errNum ) {
    
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
void DrvCmds::cmdInterpreterLoop( ) {
    
    char cmdLineBuf[ CMD_LINE_BUF_SIZE ] = { 0 };
    
    printWelcome( );
    
    while ( true ) {
        
        promptCmdLine( );
        if ( readInputLine( cmdLineBuf )) {
            
            evalInputLine( cmdLineBuf );
            if ( winModeOn ) glb -> winDisplay -> reDraw( );
        }
    }
}
