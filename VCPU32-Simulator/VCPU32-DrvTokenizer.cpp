//------------------------------------------------------------------------------------------------------------
//
// VCPU32 - A 32-bit CPU - Simulator Tokenizer
//
//------------------------------------------------------------------------------------------------------------
// Welcome to the test driver commands. This a rather simple command loop resting on the "sscanf" C library
// routine to do the parsing.
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

//------------------------------------------------------------------------------------------------------------
// Local namespace. These routines are not visible outside this source file.
//
//------------------------------------------------------------------------------------------------------------
namespace {

const int   TOK_INPUT_LINE_SIZE = 128;
const int   TOK_NAME_SIZE       = 8;
const char  EOS_CHAR            = 0;

void upshiftStr( char *str ) {
    
    size_t len = strlen( str );
    
    if ( len > 0 ) {
        
        for ( size_t i = 0; i < len; i++ ) str[ i ] = (char) toupper((int) str[ i ] );
    }
}

#if 0
uint8_t lookupToken( char *str ) {
    
    if (( strlen( str ) == 0 ) || ( strlen ( str ) > TOK_NAME_SIZE )) return( 0 );
    
    for ( int i = 0; i < TOK_NAME_TAB_SIZE; i++ ) {
        
        if ( strcmp( str, tokNameTab[ i ].name ) == 0 ) return( i );
    }
    
    return( 0 );
}
#endif


}; // namespace


//------------------------------------------------------------------------------------------------------------
// The object constructor, nothing to do for now.
//
//------------------------------------------------------------------------------------------------------------
DrvTokenizer::DrvTokenizer( ) { }

//------------------------------------------------------------------------------------------------------------
// We initialize a couple of globals that represent the current state of the parsing process. This call is
// the first before any other method can be called.
//
//------------------------------------------------------------------------------------------------------------
uint8_t DrvTokenizer::setupTokenizer( char *lineBuf, DrvTokenTab *tokTab ) {
    
    strncpy( tokenLine, lineBuf, strlen( lineBuf ) + 1 );
    upshiftStr( tokenLine );
    
    currentLineLen          = (int) strlen( tokenLine );
    currentCharIndex        = 0;
    currentTokCharIndex     = 0;
    currentChar             = ' ';
    
    return( 0 );
}

//------------------------------------------------------------------------------------------------------------
// "nextChar" returns the next character from the token line string.
//
//------------------------------------------------------------------------------------------------------------
void DrvTokenizer::nextChar( ) {
    
    if ( currentCharIndex < currentLineLen ) {
        
        currentChar = tokenLine[ currentCharIndex ];
        currentCharIndex ++;
    }
    else currentChar = EOS_CHAR;
}

//------------------------------------------------------------------------------------------------------------
// "parseNum" will parse a number. We leave the heavy lifting of converting the numeric value to the C library.
//
//------------------------------------------------------------------------------------------------------------
void DrvTokenizer::parseNum( int sign ) {
    
    int  tmpRes = 0;
    char tmpStr[ TOK_INPUT_LINE_SIZE ] = "";
    
    do {
        
        strcat( tmpStr, &currentChar );
        nextChar( );
        
    } while ( isxdigit( currentChar ) || ( currentChar == 'X' ) || ( currentChar == 'O' ));
    
    if ( sscanf( tmpStr, "%i", &tmpRes ) == 1 ) {
        
        currentToken.tokenId    = TOK_NUM;
        currentToken.val        = tmpRes * sign;
    }
    else
        // parserError((char *) "Invalid number" )
        ;
}

//------------------------------------------------------------------------------------------------------------
// "parseString" gets a string. We manage special characters inside the string with the "\" prefix. Right
// now, we do not use strings, so the function is perhaps for the future. We will just parse it, but record
// no result. One day, the entire simulator might use the lexer functions. Then we need it.
//
//------------------------------------------------------------------------------------------------------------
void DrvTokenizer::parseString( ) {

    char stringBuf[ TOK_INPUT_LINE_SIZE ] = "";

    nextChar( );
          
    while (( currentChar != EOS_CHAR ) && ( currentChar != '"' )) {

        if ( currentChar == '\\' ) {

            nextChar( );
            if ( currentChar != EOS_CHAR ) {

                if      ( currentChar == 'n' )  strcat( stringBuf, (char *) "\n" );
                else if ( currentChar == 't' )  strcat( stringBuf, (char *) "\t" );
                else if ( currentChar == '\\' ) strcat( stringBuf, (char *) "\\" );
                else                            strcat( stringBuf, &currentChar );
            }
            else
                // parserError((char *) "Expected a \" for the string" )
                ;
        }
        else strcat( stringBuf, &currentChar );

        nextChar( );
    }

    nextChar( );

    currentToken.tokenId    = TOK_STR;
    currentToken.val        = 0;
}

//------------------------------------------------------------------------------------------------------------
// "parseIdent" parses an identifier. It is a sequence of characters starting with an alpha character. We do
// not really have user defined identifiers, only reserved words. As a qualified constant also begins with
// a character, the parsing of an identifier also needs to manage the parsing of constants with a qualifier,
// such as "L%nnn".  We first check for these kind of qualifiers and if found hand over to parse a number.
//
//------------------------------------------------------------------------------------------------------------
void DrvTokenizer::parseIdent( ) {
    
    char identBuf[ TOK_INPUT_LINE_SIZE ] = "";
    
    if ( currentChar == 'L' ) {
        
        strcat( identBuf, &currentChar );
        nextChar( );
        
        if ( currentChar == '%' ) {
            
            strcat( identBuf, &currentChar );
            nextChar( );
            
            if ( isdigit( currentChar )) {
                
                parseNum( 1 );
                currentToken.val &= 0xFFFFFC00;
                return;
            }
            else
                // parserError((char *) "Invalid char in identifier" )
                ;
        }
    }
    else if ( currentChar == 'R' ) {
        
        strcat( identBuf, &currentChar );
        nextChar( );
        
        if ( currentChar == '%' ) {
            
            strcat( identBuf, &currentChar );
            nextChar( );
            
            if ( isdigit( currentChar )) {
                
                parseNum( 1 );
                currentToken.val &= 0x3FF;
                return;
            }
            else
                //parserError((char *) "Invalid char in identifier" )
                ;
        }
    }
    
    while ( isalnum( currentChar )) {
        
        strcat( identBuf, &currentChar );
        nextChar( );
    }
    
    int currentTokenIndex =
    
    // lookupToken( identBuf )
    0
    ;
    
    if ( currentTokenIndex == 0 ) {
        
        strcpy( currentToken.name, identBuf );
        currentToken.tokenId = TOK_IDENT;
        currentToken.val = 0;
    }
    else
        
        // currentToken = tokNameTab[ currentTokenIndex ]
        ;
}

//------------------------------------------------------------------------------------------------------------
// "nextToken" is the entry point to the token business. It retruns the next token from the input string.
//
//------------------------------------------------------------------------------------------------------------
void DrvTokenizer::nextToken( ) {

    currentToken.name[ 0 ]      = 0;
    currentToken.tokGrpId       = TOK_NIL;
    currentToken.tokenId        = TOK_EOS;
    currentToken.val            = 0;
    
    while (( currentChar == ' ' ) || ( currentChar == '\n' ) || ( currentChar == '\n' )) nextChar( );
    
    currentTokCharIndex = currentCharIndex - 1;
    
    if ( isalpha( currentChar )) {
        
       parseIdent( );
    }
    else if ( isdigit( currentChar )) {
        
       parseNum( 1 );
    }
    else if ( currentChar == '"' ) {
        
        parseString( );
    }
    else if ( currentChar == '.' ) {
        
        currentToken.tokenId = TOK_PERIOD;
        
        nextChar( );
        while ( isalnum( currentChar )) {
            
            strcat( currentToken.name, &currentChar );
            nextChar( );
        }
    }
    else if ( currentChar == '+' ) {
        
        currentToken.tokenId = TOK_PLUS;
        nextChar( );
    }
    else if ( currentChar == '-' ) {
        
        currentToken.tokenId = TOK_MINUS;
        nextChar( );
    }
    else if ( currentChar == '*' ) {
        
        currentToken.tokenId = TOK_MULT;
        nextChar( );
    }
    else if ( currentChar == '/' ) {
        
        currentToken.tokenId = TOK_DIV;
        nextChar( );
    }
    else if ( currentChar == '%' ) {
        
        currentToken.tokenId = TOK_MOD;
        nextChar( );
    }
    else if ( currentChar == '|' ) {
        
        currentToken.tokenId = TOK_OR;
        nextChar( );
    }
    else if ( currentChar == '^' ) {
        
        currentToken.tokenId = TOK_XOR;
        nextChar( );
    }
    else if ( currentChar == '~' ) {
        
        currentToken.tokenId = TOK_NEG;
        nextChar( );
    }
    else if ( currentChar == '(' ) {
        
        currentToken.tokenId = TOK_LPAREN;
        nextChar( );
    }
    else if ( currentChar == ')' ) {
        
        currentToken.tokenId = TOK_RPAREN;
        nextChar( );
    }
    else if ( currentChar == ',' ) {
        
        currentToken.tokenId = TOK_COMMA;
        nextChar( );
    }
    else if ( currentChar == EOS_CHAR ) {
        
        currentToken.name[ 0 ]  = 0;
        currentToken.tokenId        = TOK_EOS;
        currentToken.val        = 0;
    }
    else currentToken.tokenId = TOK_ERR;
}

#if 0


// ??? how to best deal with errors ?

//------------------------------------------------------------------------------------------------------------
// "parserError" is a little helper that prints out the error encountered. We will print the original
// input line, a caret marker where we found the error, and then return a false. Parsing errors typically
// result in aborting the parsing process. As this is a one line assembly, we do not need to be put effort
// into continuing reasonably with the parsing process.
//
//------------------------------------------------------------------------------------------------------------
bool parserError( char *errStr ) {
    
    fprintf( stdout, "%s\n", tokenLine );
    
    int i = 0;
    while ( i < currentTokCharIndex ) {
        
        fprintf( stdout, " " );
        i ++;
    }
    
    fprintf( stdout, "^\n" "%s\n", errStr );
    return( false );
}


#endif

