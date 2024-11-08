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

//------------------------------------------------------------------------------------------------------------
//
//
//------------------------------------------------------------------------------------------------------------
const int   TOK_INPUT_LINE_SIZE = 256;
const int   TOK_NAME_SIZE       = 32;
const char  EOS_CHAR            = 0;

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
// The lookup function. We just do a linear search for now. Note that we expect the last entry in the token
// table to be the NIL token, otherwise bad things will happen.
//
//------------------------------------------------------------------------------------------------------------
int lookupToken( char *inputStr, DrvToken *tokTab ) {
    
    if (( strlen( inputStr ) == 0 ) || ( strlen ( inputStr ) > TOK_NAME_SIZE )) return( -1 );
    
    char tmpStr[ TOK_INPUT_LINE_SIZE ];
    strcpy( tmpStr, inputStr );
    upshiftStr( tmpStr );
    
    DrvToken *tok = tokTab;
    
    while ( tok -> tid != TOK_NIL ) {
        
        if ( strcmp( tmpStr, tok -> name ) == 0 )   return((int) ( tok - tokTab ));
        else                                        tok ++;
    }
    
    return( -1 );
}

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
uint8_t DrvTokenizer::setupTokenizer( char *lineBuf, DrvToken *tokTab ) {
    
    strncpy( tokenLine, lineBuf, strlen( lineBuf ) + 1 );
    
    this -> tokTab                  = tokTab;
    this -> currentLineLen          = (int) strlen( tokenLine );
    this -> currentCharIndex        = 0;
    this -> currentTokCharIndex     = 0;
    this -> currentChar             = ' ';
    return( NO_ERR );
}

//------------------------------------------------------------------------------------------------------------
// "parserError" is a little helper that prints out the error encountered. We will print the original
// input line, a caret marker where we found the error, and then return a false. Parsing errors typically
// result in aborting the parsing process. As this is a one line assembly, we do not need to be put effort
// into continuing reasonably with the parsing process.
//
//------------------------------------------------------------------------------------------------------------
void DrvTokenizer::tokenError( char *errStr ) {
    
    fprintf( stdout, "%s\n", tokenLine );
    
    int i = 0;

    while ( i < currentTokCharIndex ) {
        
        fprintf( stdout, " " );
        i ++;
    }
    
    fprintf( stdout, "^\n" "%s\n", errStr );
}

//------------------------------------------------------------------------------------------------------------
// helper functions for the current token.
//
//------------------------------------------------------------------------------------------------------------
bool        DrvTokenizer::isToken( TokId tokId )        { return( currentTokId == tokId ); }
bool        DrvTokenizer::isTokenTyp( TypeId typId )    { return( currentTokTyp == typId ); }
TypeId      DrvTokenizer::tokTyp( )                     { return( currentTokTyp ); }
TokId       DrvTokenizer::tokId( )                      { return( currentTokId ); }
int         DrvTokenizer::tokVal( )                     { return( currentTokVal ); }
char        *DrvTokenizer::tokStr( )                    { return( currentTokStr ); }
int         DrvTokenizer::tokCharIndex( )               { return( currentCharIndex ); }
char        *DrvTokenizer::tokenLineStr( )              { return(  tokenLine ); }

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
// "parseNum" will parse a number. We leave the heavy lifting of converting the numeric value to the C
// library.
//
// ??? should we detect an address pair here ? <seg>.<ofs> ?
//------------------------------------------------------------------------------------------------------------
void DrvTokenizer::parseNum( ) {
    
    char tmpStr[ TOK_INPUT_LINE_SIZE ]  = "";
    
    currentTokTyp       = TYP_NUM;
    currentTokId        = TOK_NUM;
    currentTokVal       = 0;
    currentTokStr[ 0 ]  = '\0';
    
    do {
        
        strcat( tmpStr, &currentChar );
        nextChar( );
        
    } while ( isxdigit( currentChar )   || ( currentChar == 'X' ) || ( currentChar == 'O' )
                                        || ( currentChar == 'x' ) || ( currentChar == 'o' ));
    
    if ( sscanf( tmpStr, "%i", &currentTokVal ) != 1 )
      tokenError((char *) "Invalid number" );
    
    if ( currentChar == '.' ) {
        
        // ??? we would need to parse the next number. How to store them in the return ?
        
    }
    
}

//------------------------------------------------------------------------------------------------------------
// "parseString" gets a string. We manage special characters inside the string with the "\" prefix. Right
// now, we do not use strings, so the function is perhaps for the future. We will just parse it, but record
// no result. One day, the entire simulator might use the lexer functions. Then we need it.
//
//------------------------------------------------------------------------------------------------------------
void DrvTokenizer::parseString( ) {

    currentTokTyp       = TYP_STR;
    currentTokId        = TOK_STR;
    currentTokVal       = 0;
    currentTokStr[ 0 ]  = '\0';

    nextChar( );
          
    while (( currentChar != EOS_CHAR ) && ( currentChar != '"' )) {

        if ( currentChar == '\\' ) {
            
            nextChar( );
            if ( currentChar != EOS_CHAR ) {
                
                if      ( currentChar == 'n' )  strcat( currentTokStr, (char *) "\n" );
                else if ( currentChar == 't' )  strcat( currentTokStr, (char *) "\t" );
                else if ( currentChar == '\\' ) strcat( currentTokStr, (char *) "\\" );
                else                            strcat( currentTokStr, &currentChar );
            }
            else {
                
                currentTokStr[ 0 ]  = '\0';
                tokenError((char *) "Expected a closing quote" );
                break;
            }
        }
        else strcat( currentTokStr, &currentChar );

        nextChar( );
    }

    nextChar( );
}

//------------------------------------------------------------------------------------------------------------
// "parseIdent" parses an identifier. It is a sequence of characters starting with an alpha character. An
// identifier found in the token table will assume the type and value of the token found. Any other identifier
// is just an identifier symbol. There is one more thing. There are qualified constants that begin with a
// character followed by a percent character, followed by the value. During the character analysis, We first
// check for these kind of qualifiers and if found hand over to parse a number.
//
//------------------------------------------------------------------------------------------------------------
void DrvTokenizer::parseIdent( ) {
    
    currentTokTyp       = TYP_IDENT;
    currentTokId        = TOK_IDENT;
    currentTokVal       = 0;
    currentTokStr[ 0 ]  = '\0';
    
    char identBuf[ TOK_INPUT_LINE_SIZE ] = "";
    
    if ( currentChar == 'L' ) {
        
        strcat( identBuf, &currentChar );
        nextChar( );
        
        if ( currentChar == '%' ) {
            
            strcat( identBuf, &currentChar );
            nextChar( );
            
            if ( isdigit( currentChar )) {
                
                parseNum( );
                currentTokVal &= 0xFFFFFC00;
                return;
            }
            else tokenError((char *) "Invalid character in identifier" );
        }
    }
    else if ( currentChar == 'R' ) {
        
        strcat( identBuf, &currentChar );
        nextChar( );
        
        if ( currentChar == '%' ) {
            
            strcat( identBuf, &currentChar );
            nextChar( );
            
            if ( isdigit( currentChar )) {
                
                parseNum( );
                currentTokVal &= 0x3FF;
                return;
            }
            else tokenError((char *) "Invalid character in identifier" );
        }
    }
    
    while (( isalnum( currentChar )) || ( currentChar == '_' )) {
        
        strcat( identBuf, &currentChar );
        nextChar( );
    }
    
    int index = lookupToken( identBuf, tokTab );
    
    if ( index == -1 ) {
        
        currentTokTyp       = TYP_IDENT;
        currentTokId        = TOK_IDENT;
        currentTokVal       = 0;
        strcpy( currentTokStr, identBuf );
    }
    else {
        
        currentTokTyp       = tokTab[ index ].typ;
        currentTokId        = tokTab[ index ].tid;
        currentTokVal       = tokTab[ index ].val;
        strcpy( currentTokStr, identBuf );
    }
}

//------------------------------------------------------------------------------------------------------------
// "nextToken" is the entry point to the token business. It returns the next token from the input string.
//
//------------------------------------------------------------------------------------------------------------
void DrvTokenizer::nextToken( ) {

    currentTokTyp       = TYP_NIL;
    currentTokId        = TOK_NIL;
    currentTokVal       = 0;
    currentTokStr[ 0 ]  = '\0';
    
    while (( currentChar == ' ' ) || ( currentChar == '\n' ) || ( currentChar == '\n' )) nextChar( );
    
    currentTokCharIndex = currentCharIndex - 1;
    
    if ( isalpha( currentChar )) {
        
       parseIdent( );
    }
    else if ( isdigit( currentChar )) {
        
       parseNum( );
        
        // ??? parse a number pair rather here ?
    }
    else if ( currentChar == '"' ) {
        
        parseString( );
    }
    else if ( currentChar == '.' ) {
        
        currentTokTyp   = TYP_SYM;
        currentTokId    = TOK_PERIOD;
        nextChar( );
    }
    else if ( currentChar == '+' ) {
        
        currentTokId = TOK_PLUS;
        nextChar( );
    }
    else if ( currentChar == '-' ) {
        
        currentTokTyp   = TYP_SYM;
        currentTokId    = TOK_MINUS;
        nextChar( );
    }
    else if ( currentChar == '*' ) {
        
        currentTokTyp   = TYP_SYM;
        currentTokId    = TOK_MULT;
        nextChar( );
    }
    else if ( currentChar == '/' ) {
        
        currentTokTyp   = TYP_SYM;
        currentTokId    = TOK_DIV;
        nextChar( );
    }
    else if ( currentChar == '%' ) {
        
        currentTokTyp   = TYP_SYM;
        currentTokId    = TOK_MOD;
        nextChar( );
    }
    else if ( currentChar == '|' ) {
        
        currentTokTyp   = TYP_SYM;
        currentTokId    = TOK_OR;
        nextChar( );
    }
    else if ( currentChar == '^' ) {
        
        currentTokTyp   = TYP_SYM;
        currentTokId    = TOK_XOR;
        nextChar( );
    }
    else if ( currentChar == '~' ) {
        
        currentTokTyp   = TYP_SYM;
        currentTokId    = TOK_NEG;
        nextChar( );
    }
    else if ( currentChar == '(' ) {
        
        currentTokTyp   = TYP_SYM;
        currentTokId    = TOK_LPAREN;
        nextChar( );
    }
    else if ( currentChar == ')' ) {
        
        currentTokTyp   = TYP_SYM;
        currentTokId    = TOK_RPAREN;
        nextChar( );
    }
    else if ( currentChar == ',' ) {
        
        currentTokTyp   = TYP_SYM;
        currentTokId    = TOK_COMMA;
        nextChar( );
    }
    else if ( currentChar == EOS_CHAR ) {
        
        currentTokTyp   = TYP_SYM;
        currentTokId    = TOK_EOS;
    }
    else {
        
        tokenError((char *) "Invalid character in input string" );
        currentTokId = TOK_ERR;
    }
}
