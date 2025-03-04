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
// where we found the error, and then return a false. Note that the position needs to add the prompt part of
// the command line to where the error was found in the command input.
//
//------------------------------------------------------------------------------------------------------------
void DrvCmds::cmdLineError( ErrMsgId errNum, char *argStr ) {
   
    for ( int i = 0; i < MAX_ERR_MSG_TAB; i++ ) {
        
        if ( errMsgTab[ i ].errNum == errNum ) {
            
            glb -> console -> printChars( "%s\n", errMsgTab[ i ].errStr );
            return;
        }
    }
    
    glb -> console -> printChars( "Error: %d", errNum );
    if ( argStr != nullptr )  glb -> console -> printChars( "%32s", argStr );
    glb -> console -> printChars( "/n" );
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
// The simulator command interpreter features a simple command history. It is a circular buffer that holds
// the last commands. There are functions to show the command history, re-execute a previous command and to
// retrieve a previous command for editing. The command stack can be accessed with relative command numbers,
// i.e. "current - 3" or by absolute command number, when still present in the history stack.
//
// ??? first cut at command history.... check... it becomes part of the new command interpreter.
//------------------------------------------------------------------------------------------------------------
DrvCmdHistory::DrvCmdHistory( VCPU32Globals *glb ) {
    
    this -> glb     = glb;
    this -> head    = 0;
    this -> tail    = 0;
    this -> count   = 0;
}

//------------------------------------------------------------------------------------------------------------
// Add a command line. If the history buffer is ful, the oldest entry is re-used. The head index points to
// the next entry for allocation.
//
//------------------------------------------------------------------------------------------------------------
void DrvCmdHistory::addCmdLine( char *cmdStr ) {
    
    cmdIdCount ++;
    
    DrvCmdHistEntry *ptr = &history[ head ];
    
    ptr -> cmdId = cmdIdCount;
    strncpy( ptr -> cmdLine, cmdStr, 256 );

    if ( count == MAX_CMD_HIST_BUF_SIZE ) tail = ( tail + 1 ) % MAX_CMD_HIST_BUF_SIZE;
    else count++;

    head = ( head + 1 ) % MAX_CMD_HIST_BUF_SIZE;
}

//------------------------------------------------------------------------------------------------------------
// There is the situation that the current command is the "HIST" command itself. In this case we do not want
// to add this command to the history buffer. Unfortunately, by the time we decoded the command it is already
// in the history buffer. So, this routine will allow to remove the top command from the history buffer and
// updatimg the comdId counter accordingly.
//
//------------------------------------------------------------------------------------------------------------
void DrvCmdHistory::removeTopCmdLine( ) {
    
    cmdIdCount --;
    
    if ( count == MAX_CMD_HIST_BUF_SIZE ) tail = ( tail - 1 ) % MAX_CMD_HIST_BUF_SIZE;
    else count--;
    
    head = ( head - 1 ) % MAX_CMD_HIST_BUF_SIZE;
}

//------------------------------------------------------------------------------------------------------------
// Get a command line from the command history. If the command ID is negative, the entry relativ to the top
// is used. "head - 1" refers to the last entry entered. If the command ID is positive, we search for the
// entry with the matching command id, if still in the history buffer.
//
//------------------------------------------------------------------------------------------------------------
char *DrvCmdHistory::getCmdLine( int cmdId ) {
    
    if (( cmdId >= 0 ) && (( cmdIdCount - cmdId ) > MAX_CMD_HIST_BUF_SIZE )) return ( nullptr );
    if (( cmdId < 0  ) && ( - cmdId > cmdIdCount )) return ( nullptr );
    if ( count == 0 ) return ( nullptr );
        
    if ( cmdId >= 0 ) {
        
        for ( int i = 0; i < count; i++ ) {
        
            int pos = ( tail + i ) % MAX_CMD_HIST_BUF_SIZE;
            if ( history[ pos ].cmdId == cmdId ) return( history[ pos ].cmdLine );
        }
     
        return( nullptr );
    }
    else {
        
        int pos = ( head - 1 + cmdId + MAX_CMD_HIST_BUF_SIZE) % MAX_CMD_HIST_BUF_SIZE;
        return history[ pos ].cmdLine;
    }
}

//------------------------------------------------------------------------------------------------------------
// List the commamd history.
//
// ??? print only up to "depth".
//------------------------------------------------------------------------------------------------------------
void DrvCmdHistory::printCmdistory( int depth ) {
    
    glb -> console -> printChars( "Cmd History (%d/%d entries):\n", count, MAX_CMD_HIST_BUF_SIZE );
    
    for ( int i = 0; i < count; i++ ) {
    
        int pos = ( tail + i ) % MAX_CMD_HIST_BUF_SIZE;
    
        glb -> console -> printChars( "[%d]: %s\n", history[ pos ].cmdId, history[ pos ].cmdLine);
    }
}

//------------------------------------------------------------------------------------------------------------
// The command history maintains a command counter, which we return here.
//
//------------------------------------------------------------------------------------------------------------
int DrvCmdHistory::getCmdId( ) {
    
    return( cmdIdCount );
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
    
    if ( glb -> console -> isConsole( )) {
        
        glb -> console -> printChars( "VCPU-32 Simulator, Version: %s, Patch Level: %d\n",
                                      glb -> env -> getEnvVarStr((char *) ENV_PROG_VERSION ),
                                      glb -> env -> getEnvVarStr((char *) ENV_PATCH_LEVEL ));
        
        glb -> console -> printChars( "Git Branch: %s\n",
                                      glb -> env -> getEnvVarStr((char *) ENV_GIT_BRANCH ));
    }
}

//------------------------------------------------------------------------------------------------------------
// "promptCmdLine" lists out the prompt string. For now this is just a "->". As development goes on the prompt
// string will contain some more info about the current CPU state. The prompt is only printed when the input
// comes from a terminal and not an input file.
//
//------------------------------------------------------------------------------------------------------------
void DrvCmds::promptCmdLine( ) {
    
    if ( glb -> console -> isConsole( )) {
        
        if ( glb -> env -> getEnvVarBool((char *) ENV_SHOW_CMD_CNT )) {
            
            promptLen = glb -> console -> printChars( "(%i) ", glb -> env -> getEnvVarInt((char *) ENV_CMD_CNT ));
        }
        
        promptLen += glb -> console -> printChars( "->" );
    }
}

//------------------------------------------------------------------------------------------------------------
// "readCmdLine" reads in the command line. The function returns the number of characters read or an error
// code. We loop inside the routine until we receive a valid command line or an EOF. A non-empty command is
// also added to the command history buffer.
//
// ??? should we add the history command to the history buffer as well ?
// ??? we cannot know at this stage what the command is...
//------------------------------------------------------------------------------------------------------------
int DrvCmds::readInputLine( char *cmdBuf ) {
    
    while ( true ) {
        
        int len = glb -> console -> readLine( cmdBuf );
        
        if ( len > 0 ) {
            
            removeComment( cmdBuf );
            
            glb -> hist -> addCmdLine( cmdBuf );
            glb -> env -> setEnvVar((char *) ENV_CMD_CNT, glb -> hist -> getCmdId( ));
           
            return( len );
        }
        else return( -1 );
    }
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
                        
                        glb -> console -> printChars( "%s\n", cmdLineBuf );
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
        
        switch ( errNum ) {
                
            case ERR_OPEN_EXEC_FILE: {
                
                glb -> console -> printChars( "Error in opening file: \"%s\"", fileName );
                
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
void DrvCmds::helpCmd( ) {
    
    const char FMT_STR_SUMMARY[ ] = "%-16s%s\n";
    const char FMT_STR_DETAILS[ ] = "%s - %s\n";
    
    if ( glb -> tok -> isToken( TOK_EOS )) {
        
        for ( int i = 0; i < MAX_CMD_HELP_TAB; i++ ) {
            
            if ( cmdHelpTab[ i ].helpTypeId == TYP_CMD )
                glb -> console -> printChars( FMT_STR_SUMMARY, cmdHelpTab[ i ].cmdNameStr, cmdHelpTab[ i ].helpStr );
        }
        
        glb -> console -> printChars( "\n" );
    }
    else if (( glb -> tok -> isTokenTyp( TYP_CMD  )) ||
             ( glb -> tok -> isTokenTyp( TYP_WCMD )) ||
             ( glb -> tok -> isTokenTyp( TYP_WTYP )) ||
             ( glb -> tok -> isTokenTyp( TYP_RSET )) ||
             ( glb -> tok -> isTokenTyp( TYP_PREDEFINED_FUNC ))) {
        
        if ( glb -> tok -> isToken( CMD_SET )) {
            
            for ( int i = 0; i < MAX_CMD_HELP_TAB; i++ ) {
                
                if ( cmdHelpTab[ i ].helpTypeId == TYP_CMD )
                    glb -> console -> printChars( FMT_STR_SUMMARY, cmdHelpTab[ i ].cmdNameStr, cmdHelpTab[ i ].helpStr );
            }
            
            glb -> console -> printChars( "\n" );
        }
        else if ( glb -> tok -> isToken( WCMD_SET )) {
            
            for ( int i = 0; i < MAX_CMD_HELP_TAB; i++ ) {
                
                if ( cmdHelpTab[ i ].helpTypeId == TYP_WCMD )
                    glb -> console -> printChars( FMT_STR_SUMMARY, cmdHelpTab[ i ].cmdNameStr, cmdHelpTab[ i ].helpStr );
            }
            
            glb -> console -> printChars( "\n" );
        }
        else if ( glb -> tok -> isToken( REG_SET )) {
            
            for ( int i = 0; i < MAX_CMD_HELP_TAB; i++ ) {
                
                if ( cmdHelpTab[ i ].helpTypeId == TYP_RSET )
                    
                    glb -> console -> printChars( FMT_STR_SUMMARY, cmdHelpTab[ i ].cmdNameStr, cmdHelpTab[ i ].helpStr );
            }
            
            glb -> console -> printChars( "\n" );
        }
        else if ( glb -> tok -> isToken( WTYPE_SET )) {
            
            for ( int i = 0; i < MAX_CMD_HELP_TAB; i++ ) {
                
                if ( cmdHelpTab[ i ].helpTypeId == TYP_WTYP )
                    
                    glb -> console -> printChars( FMT_STR_SUMMARY, cmdHelpTab[ i ].cmdNameStr, cmdHelpTab[ i ].helpStr );
            }
            
            glb -> console -> printChars( "\n" );
        }
        else if ( glb -> tok -> isToken( PF_SET )) {
            
            for ( int i = 0; i < MAX_CMD_HELP_TAB; i++ ) {
                
                if ( cmdHelpTab[ i ].helpTypeId == TYP_PREDEFINED_FUNC )
                    glb -> console -> printChars( FMT_STR_SUMMARY, cmdHelpTab[ i ].cmdNameStr, cmdHelpTab[ i ].helpStr );
            }
            
            glb -> console -> printChars( "\n" );
        }
        else {
            
            for ( int i = 0; i < MAX_CMD_HELP_TAB; i++ ) {
                
                if ( cmdHelpTab[ i ].helpTokId == glb -> tok -> tokId( )) {
                    
                    glb -> console -> printChars( FMT_STR_DETAILS, cmdHelpTab[ i ].cmdSyntaxStr, cmdHelpTab[ i ].helpStr );
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
// remove a user defined variable.
//
//  ENV [ <var> [ <val> ]]",
//------------------------------------------------------------------------------------------------------------
void DrvCmds::envCmd( ) {
    
    DrvEnv *env = glb -> env;
    
    if ( glb -> tok -> tokId( ) == TOK_EOS ) {
        
        env -> displayEnvTable( );
    }
    else if ( glb -> tok -> tokTyp( ) == TYP_IDENT ) {
        
        char envName[ MAX_ENV_NAME_SIZE ];
        
        strcpy( envName, glb -> tok -> tokStr( ));
        upshiftStr( envName );
        
        glb -> tok -> nextToken( );
        if ( glb -> tok -> tokId( ) == TOK_EOS ) {
            
            if ( env -> isValid( envName )) env -> displayEnvTableEntry( envName );
            else throw ( ERR_ENV_VAR_NOT_FOUND );
        }
        else {
            
            DrvExpr rExpr;
            glb -> eval -> parseExpr( &rExpr );
            
            if      ( rExpr.typ == TYP_NUM )        env -> setEnvVar( envName, rExpr.numVal );
            else if ( rExpr.typ == TYP_BOOL )       env -> setEnvVar( envName, rExpr.bVal );
            else if ( rExpr.typ == TYP_STR )        env -> setEnvVar( envName, rExpr.strVal );
            else if ( rExpr.typ == TYP_EXT_ADR )    env -> setEnvVar( envName, rExpr.seg, rExpr.ofs );
            else if (( rExpr.typ == TYP_SYM ) && ( rExpr.tokId == TOK_NIL )) env -> removeEnvVar( envName );
        }
    }
}

//------------------------------------------------------------------------------------------------------------
// Execute commands from a file command. The actual work is done in the "execCmdsFromFile" routine.
//
// XF "<filename>"
//------------------------------------------------------------------------------------------------------------
void DrvCmds::execFileCmd( ) {
    
    if ( glb -> tok -> tokTyp( ) == TYP_STR ) {
        
        execCmdsFromFile( glb -> tok -> tokStr( ));
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
void DrvCmds::resetCmd( ) {
    
    if ( glb -> tok -> isToken( TOK_EOS )) {
        
        glb -> cpu -> reset( );
    }
    else if ( glb -> tok -> tokTyp( ) == TYP_SYM ) {
        
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
//  RUN
//
// ??? see STEP command for detils on teh console handling.
//------------------------------------------------------------------------------------------------------------
void DrvCmds::runCmd( ) {
    
    glb -> console -> printChars( "RUN command to come ... \n");
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
// Write line command.
//
//  W <expr> [ , <rdx> ]
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
            
        case TYP_BOOL: {
            
            if ( rExpr.bVal == true )   glb -> console -> printChars( "TRUE\n" );
            else                        glb -> console -> printChars( "FALSE\n" );
            
        } break;
            
        case TYP_NUM: {
            
            glb -> console -> printNum( rExpr.numVal, rdx );
            glb -> console -> printChars( "\n" );
            
        } break;
            
        case TYP_STR: {
            
            glb -> console -> printChars( "\"%s\"\n", rExpr.strVal );
            
        } break;
            
        case TYP_EXT_ADR: {
            
            glb -> console -> printNum( rExpr.seg, rdx );
            glb -> console -> printChars( "." );
            glb -> console -> printNum( rExpr.ofs, rdx );
            glb -> console -> printChars( "\n" );
            
        } break;
            
        default: throw (  ERR_INVALID_EXPR );
    }
}

//------------------------------------------------------------------------------------------------------------
// The HIST command displays the command history. Optional, we can onl report a certain depth from the top.
//
//  HIST [ depth ]
//------------------------------------------------------------------------------------------------------------
void DrvCmds::histCmd( ) {
    
    DrvExpr rExpr;
    int     depth = 1;
    
    if ( glb -> tok -> tokId( ) != TOK_EOS ) {
        
        glb -> eval -> parseExpr( &rExpr );
        
        if ( rExpr.typ == TYP_NUM ) depth = rExpr.numVal;
        else                        throw ( ERR_INVALID_NUM );
    }
    
    glb -> hist -> removeTopCmdLine( );
    glb -> hist -> printCmdistory( depth );
}

//------------------------------------------------------------------------------------------------------------
//
//
//------------------------------------------------------------------------------------------------------------
void DrvCmds::doCmd( ) {
    
}

//------------------------------------------------------------------------------------------------------------
//
//
//------------------------------------------------------------------------------------------------------------
void DrvCmds::redoCmd( ) {
    
    
}

//------------------------------------------------------------------------------------------------------------
// Display register command. This is a rather versatile command, which displays register set, register and
// all of them in one format.
//
// DR [ ( <regSet> | <reg> ) ] [ , <fmt> ]
//
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
    
    glb -> console -> printChars( "\n" );
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
// Display absolute memory command. The memory address is a byte address. The offset address is a byte address,
// the length is measured in bytes, rounded up to the a word size. We accept any address and length and only
// check that the offset plus length does not exceed the address space. The display routines, who will call
// the actual memory object will take care of gaps in the memory address range. The format specifier will
// allow for HEX, OCTAL, DECIMAL and CODE. In the case of the code option, the default number format option
// is used for showing the offset value.
//
//  DA <ofs> [ , <len> [ , <fmt> ]]
//------------------------------------------------------------------------------------------------------------
void DrvCmds::displayAbsMemCmd( ) {
    
    DrvExpr     rExpr;
    uint32_t    ofs     = 0;
    uint32_t    len     = 4;
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
//  MA <ofs> <val>
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
// Display cache entries command.
//
//  DCA ( 'I' | 'D' | 'U' ) <index> [ , <len> [ , <fmt> ]]
//------------------------------------------------------------------------------------------------------------
void DrvCmds::displayCacheCmd( ) {
    
    DrvExpr     rExpr;
    CpuMem      *cPtr           = nullptr;
    uint32_t    index           = 0;
    uint32_t    len             = 1;
    int         rdx             = glb -> env -> getEnvVarInt((char *) ENV_RDX_DEFAULT );
    
    if ( glb -> tok -> tokId( ) == TOK_I ) {
        
        cPtr = glb -> cpu -> iCacheL1;
        glb -> tok -> nextToken( );
    }
    else if ( glb -> tok -> tokId( ) == TOK_D ) {
        
        cPtr = glb -> cpu -> dCacheL1;
        glb -> tok -> nextToken( );
    }
    else if ( glb -> tok -> tokId( ) == TOK_U ) {
        
        if ( glb -> cpu -> uCacheL2 != nullptr ) {
            
            cPtr = glb -> cpu -> uCacheL2;
            glb -> tok -> nextToken( );
        }
        else throw ( ERR_CACHE_NOT_CONFIGURED );
    }
    else throw ( ERR_CACHE_TYPE );
    
    glb -> eval -> parseExpr( &rExpr );
    
    if ( rExpr.typ == TYP_NUM ) index = rExpr.numVal;
    else throw ( ERR_EXPECTED_NUMERIC );
    
    if ( glb -> tok -> tokId( ) == TOK_COMMA ) {
       
        glb -> tok -> nextToken( );
        glb -> eval -> parseExpr( &rExpr );
        
        if ( rExpr.typ == TYP_NUM ) len = rExpr.numVal;
        else throw ( ERR_EXPECTED_NUMERIC );
    }
    
    if ( glb -> tok -> tokId( ) == TOK_COMMA ) {
        
        glb -> tok -> nextToken( );
        
        if (( glb -> tok -> tokId( ) == TOK_HEX ) ||
            ( glb -> tok -> tokId( ) == TOK_OCT ) ||
            ( glb -> tok -> tokId( ) == TOK_DEC )) {
            
            rdx = glb -> tok -> tokVal( );
            glb -> tok -> nextToken( );
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
        
        glb -> lineDisplay -> displayCacheEntries( cPtr, index, len, rdx );
        
        glb -> console -> printChars( "\n" );
    }
}

//------------------------------------------------------------------------------------------------------------
// Purges a cache line from the cache.
//
//  PCA ('I' | 'D' | 'U' ) <index> [ , <set> [, 'F' ]]",
//------------------------------------------------------------------------------------------------------------
void DrvCmds::purgeCacheCmd( ) {
    
    DrvExpr     rExpr;
    CpuMem      *cPtr           = nullptr;
    uint32_t    index           = 0;
    uint32_t    set             = 0;
    bool        flush           = false;
    
    if ( glb -> tok -> tokId( ) == TOK_I ) {
        
        cPtr = glb -> cpu -> iCacheL1;
        glb -> tok -> nextToken( );
    }
    else if ( glb -> tok -> tokId( ) == TOK_D ) {
        
        cPtr = glb -> cpu -> dCacheL1;
        glb -> tok -> nextToken( );
    }
    else if ( glb -> tok -> tokId( ) == TOK_U ) {
        
        if ( glb -> cpu -> uCacheL2 != nullptr ) {
            
            cPtr = glb -> cpu -> uCacheL2;
            glb -> tok -> nextToken( );
        }
        else throw ( ERR_CACHE_NOT_CONFIGURED );
    }
    else throw ( ERR_CACHE_TYPE );
    
    glb -> eval -> parseExpr( &rExpr );
    
    if ( rExpr.typ == TYP_NUM ) index = rExpr.numVal;
    else throw (ERR_EXPECTED_NUMERIC );
    
    if ( glb -> tok -> tokId( ) == TOK_COMMA ) {
        
        glb -> tok -> nextToken( );
        glb -> eval -> parseExpr( &rExpr );
        
        if ( rExpr.typ == TYP_NUM ) set = rExpr.numVal;
        else throw ( ERR_EXPECTED_NUMERIC );
    }
    
    if ( glb -> tok -> tokId( ) == TOK_COMMA ) {
        
        glb -> tok -> nextToken( );
        
        if ( glb -> tok -> isToken( TOK_F )) flush = true;
        else throw ( ERR_INVALID_ARG );
        
        glb -> tok -> nextToken( );
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
void DrvCmds::displayTLBCmd( ) {
    
    DrvExpr     rExpr;
    uint32_t    index       = 0;
    uint32_t    len         = 0;
    uint32_t    tlbSize     = 0;
    CpuTlb      *tPtr       = nullptr;
    int         rdx         = glb -> env -> getEnvVarInt((char *) ENV_RDX_DEFAULT );
    
    if ( glb -> tok -> tokId( ) == TOK_I ) {
        
        tlbSize = glb -> cpu -> iTlb -> getTlbSize( );
        tPtr    = glb -> cpu -> iTlb;
        glb -> tok -> nextToken( );
    }
    else if ( glb -> tok -> tokId( ) == TOK_D ) {
        
        tlbSize = glb -> cpu -> dTlb -> getTlbSize( );
        tPtr    = glb -> cpu -> dTlb;
        glb -> tok -> nextToken( );
    }
    else throw ( ERR_TLB_TYPE );
    
    glb -> eval -> parseExpr( &rExpr );
    
    if ( rExpr.typ == TYP_NUM ) index = rExpr.numVal;
    else throw (ERR_EXPECTED_NUMERIC );
    
    if ( glb -> tok -> tokId( ) == TOK_COMMA ) {
        
        len = 1;
        glb -> tok -> nextToken( );
    }
    else {
       
        glb -> eval -> parseExpr( &rExpr );
        len = rExpr.numVal;
    }
    
    if ( glb -> tok -> tokId( ) == TOK_COMMA ) {
        
        glb -> tok -> nextToken( );
        
        if (( glb -> tok -> tokId( ) == TOK_HEX ) ||
            ( glb -> tok -> tokId( ) == TOK_OCT ) ||
            ( glb -> tok -> tokId( ) == TOK_DEC )) {
            
            rdx = glb -> tok -> tokVal( );
            glb -> tok -> nextToken( );
        }
        else throw ( ERR_INVALID_FMT_OPT );
    }
    
    checkEOS( );
    
    if ( len == 0 ) len = tlbSize;
    
    if (( index > tlbSize ) || ( index + len > tlbSize )) throw ( ERR_TLB_SIZE_EXCEEDED );
   
    glb -> lineDisplay -> displayTlbEntries( tPtr, index, len, rdx );
    glb -> console -> printChars( "\n" );
}

//------------------------------------------------------------------------------------------------------------
// Insert into TLB command.
//
//  ITLB ( 'I' | 'D' ) <extAdr> <argAcc> <argAdr>
//------------------------------------------------------------------------------------------------------------
void DrvCmds::insertTLBCmd( ) {
    
    DrvExpr     rExpr;
    CpuTlb      *tPtr           = nullptr;
    uint32_t    seg             = 0;
    uint32_t    ofs             = 0;
    uint32_t    argAcc          = 0;
    uint32_t    argAdr          = 0;
    
    if ( glb -> tok -> tokId( ) == TOK_I ) {
        
        tPtr = glb -> cpu -> iTlb;
        glb -> tok -> nextToken( );
    }
    else if ( glb -> tok -> tokId( ) == TOK_D ) {
        
        tPtr = glb -> cpu -> dTlb;
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
    
    if ( ! tPtr -> insertTlbEntryData( seg, ofs, argAcc, argAdr )) throw ( ERR_TLB_INSERT_OP );
}

//------------------------------------------------------------------------------------------------------------
// Purge from TLB command.
//
//  PTLB ( 'I' | 'D' ) <extAdr>"
//------------------------------------------------------------------------------------------------------------
void DrvCmds::purgeTLBCmd( ) {
    
    DrvExpr     rExpr;
    CpuTlb      *tPtr = nullptr;
    
    if ( glb -> tok -> tokId( ) == TOK_I ) {
        
        tPtr = glb -> cpu -> iTlb;
        glb -> tok -> nextToken( );
    }
    else if ( glb -> tok -> tokId( ) == TOK_D ) {
        
        tPtr = glb -> cpu -> dTlb;
        glb -> tok -> nextToken( );
    }
    else throw ( ERR_TLB_TYPE );
    
    glb -> eval -> parseExpr( &rExpr );
    
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
// Windows enable and disable. When enabled, a window does show up on the screen. The window number is
// optional, used for user definable windows.
//
//  <win>E [ <winNum> ]
//  <win>D [ <winNum> ]
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
        
        glb -> winDisplay -> windowEnable( winCmd, winNum, true );
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
void DrvCmds::winSetRadixCmd( TokId winCmd ) {
    
    if ( ! winModeOn ) throw ( ERR_NOT_IN_WIN_MODE );
    
    DrvExpr rExpr;
    int     winNum  = 0;
    int     rdx     =  glb -> env -> getEnvVarInt((char *) ENV_RDX_DEFAULT );
    
    if ( glb -> tok -> isToken( TOK_EOS )) {
        
        glb -> winDisplay -> windowRadix( winCmd, rdx, winNum );
        return;
    }
    
    if ( glb -> tok -> tokId( ) == TOK_COMMA ) {
        
        rdx = glb -> env -> getEnvVarInt((char *) ENV_RDX_DEFAULT );
        glb -> tok -> nextToken( );
    }
    else {
        
        if ( glb -> tok -> isToken( TOK_OCT ))      rdx = 8;
        else if ( glb -> tok -> isToken( TOK_DEC )) rdx = 10;
        else if ( glb -> tok -> isToken( TOK_HEX )) rdx = 16;
        else {
            
            glb -> eval -> parseExpr( &rExpr );
            if ( rExpr.typ == TYP_NUM ) rdx = setRadix( rExpr.numVal );
            else throw ( ERR_INVALID_RADIX );
        }
    }
    
    if ( glb -> tok -> tokId( ) == TOK_COMMA ) {
        
        glb -> tok -> nextToken( );
        glb -> eval -> parseExpr( &rExpr );
        
        if ( rExpr.typ == TYP_NUM ) {
            
            winNum = rExpr.numVal;
            glb -> tok -> nextToken( );
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
void DrvCmds::winForwardCmd( TokId winCmd ) {
    
    DrvExpr rExpr;
    int     winItems = 0;
    int     winNum   = 0;
    
    if ( ! winModeOn ) throw ( ERR_NOT_IN_WIN_MODE );
    
    if ( glb -> tok -> tokId( ) == TOK_EOS ) {
        
        glb -> winDisplay -> windowForward( winCmd, winItems, winNum  );
        return;
    }
    else {
        
        glb -> eval -> parseExpr( &rExpr );
        
        if ( rExpr.typ == TYP_NUM ) winItems = rExpr.numVal;
        else throw ( ERR_INVALID_NUM );
        
        if ( glb -> tok -> tokId( ) == TOK_COMMA ) {
            
            glb -> tok -> nextToken( );
            glb -> eval -> parseExpr( &rExpr );
            
            if ( rExpr.typ == TYP_NUM ) winNum = rExpr.numVal;
            else throw ( ERR_INVALID_WIN_ID );
        }
        else winNum = 0;
        
        checkEOS( );
        
        if ( ! glb -> winDisplay -> validWindowNum( winNum )) throw ( ERR_INVALID_WIN_ID );
        
        glb -> winDisplay -> windowForward( winCmd, winItems, winNum  );
    }
}

void DrvCmds::winBackwardCmd( TokId winCmd ) {
    
    DrvExpr rExpr;
    int     winItems = 0;
    int     winNum   = 0;
    
    if ( ! winModeOn ) throw ( ERR_NOT_IN_WIN_MODE );
    
    if ( glb -> tok -> tokId( ) == TOK_EOS ) {
        
        glb -> winDisplay -> windowBackward( winCmd, winItems, winNum  );
        return;
    }
    
    glb -> eval -> parseExpr( &rExpr );
    
    if ( rExpr.typ == TYP_NUM ) winItems = rExpr.numVal;
    else throw ( ERR_INVALID_NUM );
    
    if ( glb -> tok -> tokId( ) == TOK_COMMA ) {
        
        glb -> eval -> parseExpr( &rExpr );
        
        if ( rExpr.typ == TYP_NUM ) winItems = rExpr.numVal;
        else throw ( ERR_INVALID_NUM );
        
        if ( glb -> tok -> tokId( ) == TOK_COMMA ) {
            
            glb -> tok -> nextToken( );
            glb -> eval -> parseExpr( &rExpr );
            
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
void DrvCmds::winHomeCmd( TokId winCmd ) {
    
    DrvExpr rExpr;
    int     winPos   = 0;
    int     winNum   = 0;
    
    if ( ! winModeOn ) throw ( ERR_NOT_IN_WIN_MODE );
    
    if ( glb -> tok -> tokId( ) == TOK_EOS ) {
        
        glb -> winDisplay -> windowHome( winCmd, winPos, winNum  );
        return;
    }
    
    glb -> eval -> parseExpr( &rExpr );
    
    if ( rExpr.typ == TYP_NUM ) winPos = rExpr.numVal;
    else throw ( ERR_INVALID_NUM );
    
    if ( glb -> tok -> tokId( ) == TOK_COMMA ) {
        
        glb -> tok -> nextToken( );
        glb -> eval -> parseExpr( &rExpr );
        
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
void DrvCmds::winJumpCmd( TokId winCmd ) {
    
    DrvExpr rExpr;
    int     winPos   = 0;
    int     winNum   = 0;
    
    if ( ! winModeOn ) throw ( ERR_NOT_IN_WIN_MODE );
    
    if ( glb -> tok -> tokId( ) == TOK_EOS ) {
        
        glb -> winDisplay -> windowHome( winCmd, winPos, winNum  );
        return;
    }
    
    glb -> eval -> parseExpr( &rExpr );
    
    if ( rExpr.typ == TYP_NUM ) winPos = rExpr.numVal;
    else throw ( ERR_INVALID_NUM );
    
    if ( glb -> tok -> tokId( ) == TOK_COMMA ) {
        
        glb -> tok -> nextToken( );
        glb -> eval -> parseExpr( &rExpr );
        
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
void DrvCmds::winSetRowsCmd( TokId winCmd ) {
    
    DrvExpr rExpr;
    int     winLines    = 0;
    int     winNum      = 0;
    
    if ( ! winModeOn ) throw ( ERR_NOT_IN_WIN_MODE );
    
    if ( glb -> tok -> tokId( ) == TOK_EOS ) {
        
        glb -> winDisplay -> windowHome( winCmd, winLines, winNum  );
        return;
    }
    
    glb -> eval -> parseExpr( &rExpr );
    
    if ( rExpr.typ == TYP_NUM ) winLines = rExpr.numVal;
    else throw ( ERR_INVALID_NUM );
    
    if ( glb -> tok -> tokId( ) == TOK_COMMA ) {
        
        glb -> tok -> nextToken( );
        glb -> eval -> parseExpr( &rExpr );
        
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
void DrvCmds::winCurrentCmd( ) {
    
    if ( ! winModeOn ) throw ( ERR_NOT_IN_WIN_MODE );
    
    DrvExpr     rExpr;
    uint32_t    winNum = 0;
    
    if ( glb -> tok -> isToken( TOK_EOS )) throw ( ERR_EXPECTED_WIN_ID );
    
    glb -> eval -> parseExpr( &rExpr );
    
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
void  DrvCmds::winToggleCmd( ) {
    
    if ( ! winModeOn ) throw ( ERR_NOT_IN_WIN_MODE );
    
    DrvExpr     rExpr;
    uint32_t    winNum = 0;
    
    if ( glb -> tok -> isToken( TOK_EOS )) {
        
        glb -> winDisplay -> windowToggle( glb -> winDisplay -> getCurrentUserWindow( ));
        return;
    }
    
    glb -> eval -> parseExpr( &rExpr );
    
    if ( rExpr.typ == TYP_NUM ) winNum = rExpr.numVal;
    else throw ( ERR_INVALID_WIN_ID );
    
    if ( ! glb -> winDisplay -> validWindowNum( winNum )) throw ( ERR_INVALID_WIN_ID );
    
    glb -> winDisplay -> windowToggle( glb -> tok -> tokVal( ));
}

//------------------------------------------------------------------------------------------------------------
// This command exchanges the current user window with the user window specified. It allows to change the
// order of the user windows in a stacks.
//
// WX <winNum>
//------------------------------------------------------------------------------------------------------------
void DrvCmds::winExchangeCmd( ) {
    
    if ( ! winModeOn ) throw ( ERR_NOT_IN_WIN_MODE );
    
    DrvExpr     rExpr;
    uint32_t    winNum = 0;
    
    if ( glb -> tok -> isToken( TOK_EOS )) throw ( ERR_EXPECTED_WIN_ID );
    
    glb -> eval -> parseExpr( &rExpr );
    
    if ( rExpr.typ == TYP_NUM ) winNum = rExpr.numVal;
    else throw ( ERR_INVALID_WIN_ID );
    
    checkEOS( );
    
    if ( ! glb -> winDisplay -> validWindowNum( winNum )) throw ( ERR_INVALID_WIN_ID );

    glb -> winDisplay -> windowExchangeOrder( glb -> tok -> tokVal( ));
}

//------------------------------------------------------------------------------------------------------------
// This command creates a new user window. The window is assigned a free index form the windows list. This
// index is used in all the calls to this window. The window type allows to select from a code window, a
// physical memory window, a TLB and a CACHE window.
//
//  WN <winType> [ , <arg> ]
//------------------------------------------------------------------------------------------------------------
void DrvCmds::winNewWinCmd( ) {
    
    TokId   winType = TOK_NIL;
    char    *argStr = nullptr;
    
    if ( ! winModeOn ) throw ( ERR_NOT_IN_WIN_MODE );
   
    if ( glb -> tok -> tokTyp( ) == TYP_SYM ) {
        
        winType = glb -> tok -> tokId( );
        
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
        
        glb -> tok -> nextToken( );
    }
    else throw ( ERR_EXPECTED_WIN_ID );
    
    if ( glb -> tok -> tokId( ) == TOK_COMMA ) {
        
        glb -> tok -> nextToken( );
        
        if (glb ->  tok -> tokTyp( ) == TYP_STR ) argStr = glb -> tok -> tokStr( );
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
//  WS <stackNum> [ , <winNumStart> [ , <winNumEnd ]]
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
    else if ( glb -> tok -> tokId( ) == TOK_COMMA ) {
            
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
// tokenizer and dispatches based on the first token in the input line.
//
//
// ??? wouldn't it be nice to react to cursor up and down commands for a given window ? To think about...
// ??? add commands to the console window.
//------------------------------------------------------------------------------------------------------------
void DrvCmds::evalInputLine( char *cmdBuf ) {
    
    try {
        
        if ( strlen( cmdBuf ) > 0 ) {
            
            glb -> tok -> setupTokenizer( cmdBuf, (DrvToken *) cmdTokTab );
            glb -> tok -> nextToken( );
            
            if (( glb -> tok -> isTokenTyp( TYP_CMD )) || ( glb -> tok -> isTokenTyp( TYP_WCMD ))) {
                
                TokId cmdId = glb -> tok -> tokId( );
                
                glb -> tok -> nextToken( );
                
                switch( cmdId ) {
                        
                    case TOK_NIL:                                           break;
                    case CMD_EXIT:          exitCmd( );                     break;
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
                        
                    case CMD_DR:            displayRegCmd( );               break;
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
