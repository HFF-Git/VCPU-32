//------------------------------------------------------------------------------------------------------------
//
// VCPU32 - A 32-bit CPU - Simulator Tokenizer
//
//------------------------------------------------------------------------------------------------------------
// The tokenizer will accept an input line and return one token at a time. Upon an error, the tokenizer will
// raise an exception.
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
#include "VCPU32-SimDeclarations.h"
#include "VCPU32-SimTables.h"

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

void addChar( char *buf, int size, char ch ) {
    
    size_t len = strlen( buf );
    
    if ( len + 1 < size ) {
        
        buf[ len ]     = ch;
        buf[ len + 1 ] = 0;
    }
}

//------------------------------------------------------------------------------------------------------------
// The lookup function. We just do a linear search for now. Note that we expect the last entry in the token
// table to be the NIL token, otherwise bad things will happen.
//
//------------------------------------------------------------------------------------------------------------
int lookupToken( char *inputStr, SimToken *tokTab ) {
    
    if (( strlen( inputStr ) == 0 ) || ( strlen ( inputStr ) > TOK_NAME_SIZE )) return( -1 );
    
    // char tmpStr[ TOK_INPUT_LINE_SIZE ];
    // strcpy( tmpStr, inputStr );
    // upshiftStr( tmpStr );
   
    for ( int i = 0; i < MAX_CMD_TOKEN_TAB; i++  ) {
        
        if ( strcmp( inputStr, tokTab[ i ].name ) == 0 ) return( i );
    }
    
    return( -1 );
}

}; // namespace


//------------------------------------------------------------------------------------------------------------
// The object constructor, nothing to do for now.
//
//------------------------------------------------------------------------------------------------------------
SimTokenizer::SimTokenizer( VCPU32Globals *glb ) { }

//------------------------------------------------------------------------------------------------------------
// We initialize a couple of globals that represent the current state of the parsing process. This call is
// the first before any other method can be called.
//
//------------------------------------------------------------------------------------------------------------
void SimTokenizer::setupTokenizer( char *lineBuf, SimToken *tokTab ) {
    
    strncpy( tokenLine, lineBuf, strlen( lineBuf ) + 1 );
    
    this -> tokTab                  = tokTab;
    this -> currentLineLen          = (int) strlen( tokenLine );
    this -> currentCharIndex        = 0;
    this -> currentTokCharIndex     = 0;
    this -> currentChar             = ' ';
}

//------------------------------------------------------------------------------------------------------------
// helper functions for the current token.
//
//------------------------------------------------------------------------------------------------------------
bool        SimTokenizer::isToken( SimTokId tokId )        { return( currentToken.tid == tokId ); }
bool        SimTokenizer::isTokenTyp( SimTokTypeId typId )    { return( currentToken.typ == typId ); }

SimToken    SimTokenizer::token( )                      { return( currentToken );     }
SimTokTypeId      SimTokenizer::tokTyp( )                     { return( currentToken.typ ); }
SimTokId       SimTokenizer::tokId( )                      { return( currentToken.tid ); }
int         SimTokenizer::tokVal( )                     { return( currentToken.val ); }
char        *SimTokenizer::tokStr( )                    { return( currentToken.str ); }
uint32_t    SimTokenizer::tokSeg( )                     { return( currentToken.seg ); }
uint32_t    SimTokenizer::tokOfs( )                     { return( currentToken.ofs ); }

int         SimTokenizer::tokCharIndex( )               { return( currentCharIndex ); }
char        *SimTokenizer::tokenLineStr( )              { return( tokenLine ); }

//------------------------------------------------------------------------------------------------------------
// "nextChar" returns the next character from the token line string.
//
//------------------------------------------------------------------------------------------------------------
void SimTokenizer::nextChar( ) {
    
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
void SimTokenizer::parseNum( ) {
    
    char tmpStr[ TOK_INPUT_LINE_SIZE ]  = "";
    
    currentToken.tid = TOK_NUM;
    currentToken.typ = TYP_NUM;
    currentToken.val = 0;
    
    do {
        
        addChar( tmpStr, sizeof( tmpStr ), currentChar );
        nextChar( );
        
    } while ( isxdigit( currentChar )   || ( currentChar == 'X' ) || ( currentChar == 'O' )
                                        || ( currentChar == 'x' ) || ( currentChar == 'o' ));
    
    if ( sscanf( tmpStr, "%i", &currentToken.val ) != 1 ) throw ( ERR_INVALID_NUM );
      
    if ( currentChar == '.' ) {
        
        nextChar( );
        if ( ! isdigit( currentChar )) throw ( ERR_EXPECTED_EXT_ADR );
           
        currentToken.seg    = currentToken.val;
        currentToken.typ    = TYP_EXT_ADR;
        tmpStr[ 0 ]         = '\0';
        
        do {
            
            addChar( tmpStr, sizeof( tmpStr ), currentChar );
            nextChar( );
            
        } while ( isxdigit( currentChar )   || ( currentChar == 'X' ) || ( currentChar == 'O' )
                                            || ( currentChar == 'x' ) || ( currentChar == 'o' ));
        
        if ( sscanf( tmpStr, "%i", &currentToken.ofs ) != 1 ) throw ( ERR_INVALID_NUM );
    }
}

//------------------------------------------------------------------------------------------------------------
// "parseString" gets a string. We manage special characters inside the string with the "\" prefix. Right
// now, we do not use strings, so the function is perhaps for the future. We will just parse it, but record
// no result. One day, the entire simulator might use the lexer functions. Then we need it.
//
//------------------------------------------------------------------------------------------------------------
void SimTokenizer::parseString( ) {
    
    currentToken.tid        = TOK_STR;
    currentToken.typ        = TYP_STR;
    currentToken.str[ 0 ]   = '\0';

    nextChar( );
    while (( currentChar != EOS_CHAR ) && ( currentChar != '"' )) {

        if ( currentChar == '\\' ) {
            
            nextChar( );
            if ( currentChar != EOS_CHAR ) {
                
                if      ( currentChar == 'n' )  strcat( currentToken.str, (char *) "\n" );
                else if ( currentChar == 't' )  strcat( currentToken.str, (char *) "\t" );
                else if ( currentChar == '\\' ) strcat( currentToken.str, (char *) "\\" );
                else addChar( currentToken.str, sizeof( currentToken.str ), currentChar );
            }
            else throw ( ERR_EXPECTED_CLOSING_QUOTE );
        }
        else addChar( currentToken.str, sizeof( currentToken.str ), currentChar );
        
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
void SimTokenizer::parseIdent( ) {
    
    currentToken.tid        = TOK_IDENT;
    currentToken.typ        = TYP_IDENT;
    currentToken.str[ 0 ]   = '\0';
    
    char identBuf[ TOK_INPUT_LINE_SIZE ] = "";
    
    if (( currentChar == 'L' ) || ( currentChar == 'l' )) {
        
        addChar( identBuf, sizeof( identBuf ), currentChar );
        nextChar( );
        
        if ( currentChar == '%' ) {
            
            addChar( identBuf, sizeof( identBuf ), currentChar );
            nextChar( );
            
            if ( isdigit( currentChar )) {
                
                parseNum( );
                currentToken.val &= 0xFFFFFC00;
                return;
            }
            else throw ( ERR_INVALID_CHAR_IN_IDENT );
        }
    }
    else if (( currentChar == 'R' ) || ( currentChar == 'r' )) {
        
        addChar( identBuf, sizeof( identBuf ), currentChar );
        nextChar( );
        
        if ( currentChar == '%' ) {
            
            addChar( identBuf, sizeof( identBuf ), currentChar );
            nextChar( );
            
            if ( isdigit( currentChar )) {
                
                parseNum( );
                currentToken.val &= 0x3FF;
                return;
            }
            else throw ( ERR_INVALID_CHAR_IN_IDENT );
        }
    }
    
    while (( isalnum( currentChar )) || ( currentChar == '_' )) {
        
        addChar( identBuf, sizeof( identBuf ), currentChar );
        nextChar( );
    }
    
    upshiftStr( identBuf );
    
    int index = lookupToken( identBuf, tokTab );
    
    if ( index == -1 ) {
        
        currentToken.typ = TYP_IDENT;
        currentToken.tid = TOK_IDENT;
        strcpy( currentToken.str, identBuf );
    }
    else currentToken = tokTab[ index ];
}

//------------------------------------------------------------------------------------------------------------
// "nextToken" is the entry point to the token business. It returns the next token from the input string.
//
//------------------------------------------------------------------------------------------------------------
void SimTokenizer::nextToken( ) {

    currentToken.typ       = TYP_NIL;
    currentToken.tid       = TOK_NIL;
    
    while (( currentChar == ' ' ) || ( currentChar == '\n' ) || ( currentChar == '\n' )) nextChar( );
    
    currentTokCharIndex = currentCharIndex - 1;
    
    if ( isalpha( currentChar )) {
        
       parseIdent( );
    }
    else if ( isdigit( currentChar )) {
        
       parseNum( );
    }
    else if ( currentChar == '"' ) {
        
        parseString( );
    }
    else if ( currentChar == '.' ) {
        
        currentToken.typ   = TYP_SYM;
        currentToken.tid   = TOK_PERIOD;
        nextChar( );
    }
    else if ( currentChar == '+' ) {
        
        currentToken.tid = TOK_PLUS;
        nextChar( );
    }
    else if ( currentChar == '-' ) {
        
        currentToken.typ    = TYP_SYM;
        currentToken.tid    = TOK_MINUS;
        nextChar( );
    }
    else if ( currentChar == '*' ) {
        
        currentToken.typ    = TYP_SYM;
        currentToken.tid    = TOK_MULT;
        nextChar( );
    }
    else if ( currentChar == '/' ) {
        
        currentToken.typ    = TYP_SYM;
        currentToken.tid    = TOK_DIV;
        nextChar( );
    }
    else if ( currentChar == '%' ) {
        
        currentToken.typ    = TYP_SYM;
        currentToken.tid    = TOK_MOD;
        nextChar( );
    }
    else if ( currentChar == '&' ) {
        
        currentToken.typ    = TYP_SYM;
        currentToken.tid    = TOK_AND;
        nextChar( );
    }
    else if ( currentChar == '|' ) {
        
        currentToken.typ    = TYP_SYM;
        currentToken.tid    = TOK_OR;
        nextChar( );
    }
    else if ( currentChar == '^' ) {
        
        currentToken.typ    = TYP_SYM;
        currentToken.tid    = TOK_XOR;
        nextChar( );
    }
    else if ( currentChar == '~' ) {
        
        currentToken.typ    = TYP_SYM;
        currentToken.tid    = TOK_NEG;
        nextChar( );
    }
    else if ( currentChar == '(' ) {
        
        currentToken.typ    = TYP_SYM;
        currentToken.tid    = TOK_LPAREN;
        nextChar( );
    }
    else if ( currentChar == ')' ) {
        
        currentToken.typ    = TYP_SYM;
        currentToken.tid    = TOK_RPAREN;
        nextChar( );
    }
    else if ( currentChar == ',' ) {
        
        currentToken.typ    = TYP_SYM;
        currentToken.tid    = TOK_COMMA;
        nextChar( );
    }
    else if ( currentChar == EOS_CHAR ) {
        
        currentToken.typ    = TYP_NIL;
        currentToken.tid    = TOK_EOS;
    }
    else {
    
        currentToken.tid = TOK_ERR;
        throw ( ERR_INVALID_CHAR_IN_TOKEN_LINE );
    }
}
