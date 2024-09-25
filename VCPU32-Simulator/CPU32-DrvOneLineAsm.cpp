//------------------------------------------------------------------------------------------------------------
//
// VCPU32 - A 32-bit CPU - One Line Assembler
//
//------------------------------------------------------------------------------------------------------------
// The one line assembler assembles an instruction without further context. It is intended to for testing
// instructions in the simulator. There is no symbol table or any concept of assembling multiple instructions.
// The instruction to generate test is completely self sufficient. The parser is a straightforward recursive
// descendant parser, LL1 grammar.
//
//------------------------------------------------------------------------------------------------------------
//
// VCPU32 - A 32-bit CPU - One Line Assembler
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
#include "VCPU32-Driver.h"
#include "VCPU32-Core.h"

//------------------------------------------------------------------------------------------------------------
// Local namespace. These routines are not visible outside this source file.
//
//------------------------------------------------------------------------------------------------------------
namespace {

const int   TOK_INPUT_LINE_SIZE = 80;
const int   TOK_NAME_SIZE       = 8;
const char  EOS_CHAR            = 0;


//------------------------------------------------------------------------------------------------------------
// Token types for the parser.
//
//------------------------------------------------------------------------------------------------------------
enum TokType : uint8_t {
    
    TT_NIL      = 0,
    TT_OPCODE   = 1,
    TT_GREG     = 2,
    TT_SREG     = 3,
    TT_CREG     = 4,
    TT_NUM      = 5,
    TT_IDENT    = 6,
    TT_OPT      = 7,
    
    TT_COMMA    = 10,
    TT_PERIOD   = 11,
    TT_LPAREN   = 12,
    TT_RPAREN   = 13,
    
    TT_PLUS     = 15,
    TT_MINUS    = 16,
    TT_MULT     = 17,
    TT_DIV      = 18,
    TT_MOD      = 14,
    
    TT_NEG      = 20,
    TT_AND      = 21,
    TT_OR       = 22,
    TT_XOR      = 23,
    TT_LEFT     = 24,
    TT_RIGHT    = 25,
    
    TT_ERR      = 100,
    TT_EOS      = 101
};

//------------------------------------------------------------------------------------------------------------
// A token. A token has a name, a type and a value. For the instructions, the value represents the instruction
// template for the respective instruction. We also already set the data word width and any other predefined
// bits. The parsing routines will augment this template be setting the remaining fields. The token table is
// just a list of tokens, which is searched in a linear fashion.
//
// ??? we may need some more options to detect special cases such as "ADDx" and a mode 0 instruction pattern.
// We cannot detect that there is now "dw" field in this mode and hence specifying an ADDB will go undetected.
// While this will not affect the assembly parsing and instruction generation, it looks strange,
//------------------------------------------------------------------------------------------------------------
struct Token {
    
    char        name[ TOK_NAME_SIZE ];
    uint8_t     typ;
    uint32_t    val;
};

const Token tokNameTab[ ] = {
    
    { "NIL",    TT_NIL,     0           },
    
    { "LD",     TT_OPCODE,  0xC0020000  },
    { "LDB",    TT_OPCODE,  0xC0000000  },
    { "LDH",    TT_OPCODE,  0xC0010000  },
    { "LDW",    TT_OPCODE,  0xC0020000  },
    { "LDR",    TT_OPCODE,  0xD0020000  },
    { "LDA",    TT_OPCODE,  0xC8020000  },
    
    { "ST",     TT_OPCODE,  0xC4220000  },
    { "STB",    TT_OPCODE,  0xC4200000  },
    { "STH",    TT_OPCODE,  0xC4210000  },
    { "STW",    TT_OPCODE,  0xC4220000  },
    { "STC",    TT_OPCODE,  0xD4020000  },
    { "STA",    TT_OPCODE,  0xCC220000  },
    
    { "ADD",    TT_OPCODE,  0x40000000  },
    { "ADDB",   TT_OPCODE,  0x40000000  },
    { "ADDH",   TT_OPCODE,  0x40010000  },
    { "ADDW",   TT_OPCODE,  0x40020000  },
    
    { "ADC",    TT_OPCODE,  0x44000000  },
    { "ADCB",   TT_OPCODE,  0x44000000  },
    { "ADCH",   TT_OPCODE,  0x44010000  },
    { "ADCW",   TT_OPCODE,  0x44020000  },
    
    { "SUB",    TT_OPCODE,  0x48000000  },
    { "SUBB",   TT_OPCODE,  0x48000000  },
    { "SUBH",   TT_OPCODE,  0x48010000  },
    { "SUBW",   TT_OPCODE,  0x48020000  },
    
    { "SBC",    TT_OPCODE,  0x4C000000  },
    { "SBCB",   TT_OPCODE,  0x4C000000  },
    { "SBCH",   TT_OPCODE,  0x4C010000  },
    { "SBCW",   TT_OPCODE,  0x4C020000  },
    
    { "AND",    TT_OPCODE,  0x50000000  },
    { "ANDB",   TT_OPCODE,  0x50000000  },
    { "ANDH",   TT_OPCODE,  0x50010000  },
    { "ANDW",   TT_OPCODE,  0x50020000  },
    
    { "OR" ,    TT_OPCODE,  0x54000000  },
    { "ORB",    TT_OPCODE,  0x54000000  },
    { "ORH",    TT_OPCODE,  0x54010000  },
    { "ORW",    TT_OPCODE,  0x54020000  },
    
    { "XOR" ,   TT_OPCODE,  0x58000000  },
    { "XORB",   TT_OPCODE,  0x58000000  },
    { "XORH",   TT_OPCODE,  0x58010000  },
    { "XORW",   TT_OPCODE,  0x58020000  },
    
    { "CMP" ,   TT_OPCODE,  0x5C000000  },
    { "CMPB",   TT_OPCODE,  0x5C000000  },
    { "CMPH",   TT_OPCODE,  0x5C010000  },
    { "CMPW",   TT_OPCODE,  0x5C020000  },
    
    { "CMPU" ,  TT_OPCODE,  0x60000000  },
    { "CMPUB",  TT_OPCODE,  0x60000000  },
    { "CMPUH",  TT_OPCODE,  0x60010000  },
    { "CMPUW",  TT_OPCODE,  0x60020000  },
    
    { "LSID" ,  TT_OPCODE,  0x10000000  },
    { "EXTR" ,  TT_OPCODE,  0x14000000  },
    { "DEP",    TT_OPCODE,  0x18000000  },
    { "DSR",    TT_OPCODE,  0x1C000000  },
    { "SHLA",   TT_OPCODE,  0x20000000  },
    { "CMR",    TT_OPCODE,  0x24000000  },
    
    { "LIDL",   TT_OPCODE,  0x04000000  },
    { "ADDIL",  TT_OPCODE,  0x08000000  },
    { "LDO",    TT_OPCODE,  0x0C000000  },
    
    { "B" ,     TT_OPCODE,  0x80000000  },
    { "GATE",   TT_OPCODE,  0x84000000  },
    { "BR",     TT_OPCODE,  0x88000000  },
    { "BV",     TT_OPCODE,  0x8C000000  },
    { "BE",     TT_OPCODE,  0x90000000  },
    { "BVE",    TT_OPCODE,  0x94000000  },
    { "CBR",    TT_OPCODE,  0x98000000  },
    { "CBRU",   TT_OPCODE,  0x9C000000  },
    
    { "MR",     TT_OPCODE,  0x28000000  },
    { "MST",    TT_OPCODE,  0x2C000000  },
    { "DS",     TT_OPCODE,  0x30000000  },
    { "LDPA",   TT_OPCODE,  0xE4000000  },
    { "PRB",    TT_OPCODE,  0xE8000000  },
    { "ITLB",   TT_OPCODE,  0xEC000000  },
    { "PTLB",   TT_OPCODE,  0xF0000000  },
    { "PCA",    TT_OPCODE,  0xF4000000  },
    { "DIAG",   TT_OPCODE,  0xF8000000  },
    { "RFI",    TT_OPCODE,  0xFC000000  },
    { "BRK",    TT_OPCODE,  0x00000000  },
    
    { "R0",     TT_GREG,    0           },
    { "R1",     TT_GREG,    1           },
    { "R2",     TT_GREG,    2           },
    { "R3",     TT_GREG,    3           },
    { "R4",     TT_GREG,    4           },
    { "R5",     TT_GREG,    5           },
    { "R6",     TT_GREG,    6           },
    { "R7",     TT_GREG,    7           },
    { "R8",     TT_GREG,    8           },
    { "R9",     TT_GREG,    9           },
    { "R10",    TT_GREG,    10          },
    { "R11",    TT_GREG,    11          },
    { "R12",    TT_GREG,    12          },
    { "R13",    TT_GREG,    13          },
    { "R14",    TT_GREG,    14          },
    { "R15",    TT_GREG,    15          },
    
    { "S0",     TT_SREG,    0           },
    { "S1",     TT_SREG,    1           },
    { "S2",     TT_SREG,    2           },
    { "S3",     TT_SREG,    3           },
    { "S4",     TT_SREG,    4           },
    { "S5",     TT_SREG,    5           },
    { "S6",     TT_SREG,    6           },
    { "S7",     TT_SREG,    7           },
    
    { "C0",     TT_CREG,    0           },
};

const int   TOK_NAME_TAB_SIZE  = sizeof( tokNameTab ) / sizeof( *tokNameTab );

//------------------------------------------------------------------------------------------------------------
//
//
//------------------------------------------------------------------------------------------------------------
enum ExprTyp {
    
    ET_NIL      = 0,
    ET_NUM      = 1,
    ET_GREG     = 2,
    ET_SREG     = 3,
    ET_CREG     = 4,
    ET_ADR      = 5,
    ET_EXT_ADR  = 6
};

struct Expr {
    
    uint8_t     typ;
    uint32_t    val1;
    uint32_t    val2;
};

bool parseExpr( Expr *rExpr );

//------------------------------------------------------------------------------------------------------------
// The current parser state. There is the line, its length, and the "current" state for character and token.
//
//------------------------------------------------------------------------------------------------------------
char        tokenLine[ TOK_INPUT_LINE_SIZE ];
int         currentLineLen;
int         currentCharIndex;
int         currentTokCharIndex;

char        currentChar;
Token       currentToken;

//------------------------------------------------------------------------------------------------------------
// Instruction decoding means to fiddle with bits and bit fields. Here is a set of helper functions.
//
//------------------------------------------------------------------------------------------------------------
bool getBit( uint32_t arg, int pos ) {
    
    return(( arg & ( 1U << ( 31 - ( pos % 32 )))) ? 1 : 0 );
}

void setBit( uint32_t *arg, int pos ) {
    
    *arg |= ( 1U << ( 31 - ( pos % 32 )));
}

void clearBit( uint32_t *arg, int pos ) {
    
    *arg &= ~( 1U << ( 31 - ( pos % 32 )));
}

void setBit( uint32_t *arg, int pos, bool val ) {
    
    if ( val )  setBit( arg, pos );
    else        clearBit( arg, pos );
}

uint32_t getBitField( uint32_t arg, int pos, int len, bool sign = false ) {
    
    pos = pos % 32;
    len = len % 32;
    
    uint32_t tmpM = ( 1U << len ) - 1;
    uint32_t tmpA = arg >> ( 31 - pos );
    
    if ( sign ) return( tmpA | ( ~ tmpM ));
    else        return( tmpA & tmpM );
}

void setBitField( uint32_t *arg, int pos, int len, uint32_t val ) {
    
    pos = pos % 32;
    len = len % 32;
    
    uint32_t tmpM = ( 1U << len ) - 1;
    
    val = ( val & tmpM ) << ( 31 - pos );
    
    *arg = ( *arg & ( ~ ( tmpM << ( 31 - pos )))) | val;
}

void setImmVal( uint32_t *instr, int pos, int len, uint32_t val ) {
    
    setBit( instr, pos, (((int32_t) val < 0 ) ? 1 : 0 ));
    setBitField( instr, pos - 1, len - 1, val );
}

void setImmValU( uint32_t *instr, int pos, int len, uint32_t val ) {
    
    setBitField( instr, pos, len, val );
}

bool isInRange( int val, int low, int high ) {
    
    return(( val >= low ) && ( val <= high ));
}

bool isInRangeForBitField( int32_t val, uint8_t bitLen ) {
    
    int min = - ( 1 << ( bitLen % 32 ));
    int max = ( 1 << ( bitLen %32 )) - 1;
    return(( val <= max ) && ( val >= min ));
}

bool isInRangeForBitFieldU( uint32_t val, uint8_t bitLen ) {
    
    int max = ( 1 << ( bitLen % 32 ));
    return( val <= max );
}

void upshiftStr( char *str ) {
    
    size_t len = strlen( str );
    
    if ( len > 0 ) {
        
        for ( size_t i = 0; i < len; i++ ) str[ i ] = (char) toupper((int) str[ i ] );
    }
}

uint8_t lookupToken( char *str ) {
    
    if (( strlen( str ) == 0 ) || ( strlen ( str ) > TOK_NAME_SIZE )) return( 0 );
    
    for ( int i = 0; i < TOK_NAME_TAB_SIZE; i++ ) {
        
        if ( strcmp( str, tokNameTab[ i ].name ) == 0 ) return( i );
    }
    
    return( 0 );
}

//------------------------------------------------------------------------------------------------------------
// "nextChar" returns the next character from the token line string.
//
//------------------------------------------------------------------------------------------------------------
void nextChar( ) {
    
    if ( currentCharIndex < currentLineLen ) {
        
        currentChar = tokenLine[ currentCharIndex ];
        currentCharIndex ++;
    }
    else currentChar = EOS_CHAR;
}

//------------------------------------------------------------------------------------------------------------
// The one line assembler interface. The input string is analyzed and the corresponding instruction word is
// created. We initialize a couple of globals that represent the current state of the parsing process.
//
//------------------------------------------------------------------------------------------------------------
void setUpOneLineAssembler( char *inputStr, uint32_t *instr ) {
    
    strncpy( tokenLine, inputStr, strlen( inputStr ) + 1 );
    upshiftStr( tokenLine );
    
    *instr                  = 0;
    currentLineLen          = (int) strlen( tokenLine );
    currentCharIndex        = 0;
    currentTokCharIndex     = 0;
    currentChar             = ' ';
}

//------------------------------------------------------------------------------------------------------------
// "parserError" is a little helper that prints out the error encountered. We will print the original
// input line, a caret marker where we found the error, and then return a false.
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

//------------------------------------------------------------------------------------------------------------
// Check that the ASM line does not contain any extra tokens when the parser completed the analysis of the
// assembly line.
//
//------------------------------------------------------------------------------------------------------------
bool checkEOS( ) {
    
    if ( currentToken.typ == TT_EOS ) return( true );
    else return( parserError((char *) "Extra tokens in the assembler line" ));
}

//------------------------------------------------------------------------------------------------------------
// "getNum" will parse a number. We leave the heavy lifting of converting the numeric value to the C library.
//
//------------------------------------------------------------------------------------------------------------
void parseNum( int sign ) {
    
    int  tmpRes = 0;
    char tmpStr[ TOK_INPUT_LINE_SIZE ] = "";
    
    do {
        
        strcat( tmpStr, &currentChar );
        nextChar( );
        
    } while ( isxdigit( currentChar ) || ( currentChar == 'X' ) || ( currentChar == 'O' ));
    
    if ( sscanf( tmpStr, "%i", &tmpRes ) == 1 ) {
        
        currentToken.typ    = TT_NUM;
        currentToken.val   = tmpRes * sign;
    }
    else parserError((char *) "Invalid number" );
}

//------------------------------------------------------------------------------------------------------------
// "getIdent" parses an identifier. It is a sequence of characters starting with an alpha character. We do
// not really have user defined identifiers, only reserved words. As a qualified constant also begins with
// a character, the parsing of an identifier also needs to manage the parsing of constants with a qualifier,
// such as "L%nnn".  We first check for these kind of qualifiers and if found hand over to parse a number.
//
// ??? expand to understand ENV variables ?
//------------------------------------------------------------------------------------------------------------
void parseIdent( ) {
    
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
            else parserError((char *) "Invalid char in identifier" );
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
            else parserError((char *) "Invalid char in identifier" );
        }
    }
    
    while ( isalnum( currentChar )) {
        
        strcat( identBuf, &currentChar );
        nextChar( );
    }
    
    int currentTokenIndex = lookupToken( identBuf );
    
    if ( currentTokenIndex == 0 ) {
        
        strcpy( currentToken.name, identBuf );
        currentToken.typ = TT_IDENT;
        currentToken.val = 0;
    }
    else currentToken = tokNameTab[ currentTokenIndex ];
}

//------------------------------------------------------------------------------------------------------------
// "nextToken" provides the next token form the input source line. All information about the token parsed
// will be stored in the global "currentXXX" variables.
//
//------------------------------------------------------------------------------------------------------------
void nextToken( ) {
    
    currentToken.name[ 0 ]  = 0;;
    currentToken.typ        = TT_EOS;
    currentToken.val        = 0;
    
    while (( currentChar == ' ' ) || ( currentChar == '\n' ) || ( currentChar == '\n' )) nextChar( );
    
    currentTokCharIndex = currentCharIndex - 1;
    
    if ( isalpha( currentChar )) {
        
        parseIdent( );
    }
    else if ( isdigit( currentChar )) {
        
        parseNum( 1 );
    }
    else if ( currentChar == '.' ) {
        
        currentToken.typ = TT_OPT;
        
        nextChar( );
        while ( isalnum( currentChar )) {
            
            strcat( currentToken.name, &currentChar );
            nextChar( );
        }
    }
    else if ( currentChar == '+' ) {
        
        currentToken.typ = TT_PLUS;
        nextChar( );
    }
    else if ( currentChar == '-' ) {
        
        currentToken.typ = TT_MINUS;
        nextChar( );
    }
    else if ( currentChar == '*' ) {
        
        currentToken.typ = TT_MULT;
        nextChar( );
    }
    else if ( currentChar == '/' ) {
        
        currentToken.typ = TT_DIV;
        nextChar( );
    }
    else if ( currentChar == '%' ) {
        
        currentToken.typ = TT_MOD;
        nextChar( );
    }
    else if ( currentChar == '|' ) {
        
        currentToken.typ = TT_OR;
        nextChar( );
    }
    else if ( currentChar == '^' ) {
        
        currentToken.typ = TT_XOR;
        nextChar( );
    }
    else if ( currentChar == '~' ) {
        
        currentToken.typ = TT_NEG;
        nextChar( );
    }
    else if ( currentChar == '(' ) {
        
        currentToken.typ = TT_LPAREN;
        nextChar( );
    }
    else if ( currentChar == ')' ) {
        
        currentToken.typ = TT_RPAREN;
        nextChar( );
    }
    else if ( currentChar == ',' ) {
        
        currentToken.typ = TT_COMMA;
        nextChar( );
    }
    else if ( currentChar == EOS_CHAR ) {
        
        currentToken.name[ 0 ]  = 0;
        currentToken.typ        = TT_EOS;
        currentToken.val        = 0;
    }
    else currentToken.typ = TT_ERR;
}

//------------------------------------------------------------------------------------------------------------
// "parseFactor" parses the factor syntax.
//
//------------------------------------------------------------------------------------------------------------
bool parseFactor( Expr *rExpr ) {
    
    rExpr -> typ  = ET_NIL;
    rExpr -> val1 = 0;
    rExpr -> val2 = 0;
   
    if ( currentToken.typ == TT_NUM    )  {
        
        rExpr -> typ = ET_NUM;
        rExpr -> val1 = currentToken.val;
        nextToken( );
        return( true );
    }
    else  if ( currentToken.typ == TT_GREG    )  {
        
        rExpr -> typ = ET_GREG;
        rExpr -> val1 = currentToken.val;
        nextToken( );
        return( true );
    }
    else  if ( currentToken.typ == TT_SREG    )  {
        
        rExpr -> typ = ET_SREG;
        rExpr -> val1 = currentToken.val;
        nextToken( );
        return( true );
    }
    else  if ( currentToken.typ == TT_CREG    )  {
        
        rExpr -> typ = ET_CREG;
        rExpr -> val1 = currentToken.val;
        nextToken( );
        return( true );
    }
    else if ( currentToken.typ == TT_LPAREN ) {
        
        nextToken( );
        if ( currentToken.typ == TT_SREG ) {
            
            rExpr -> typ    = ET_EXT_ADR;
            rExpr -> val1   = currentToken.val;
            
            nextToken( );
            if ( currentToken.typ == TT_COMMA ) nextToken( );
            else return( parserError((char *) "Expected a comma" ));
            
            if ( currentToken.typ == TT_GREG ) {
                
                rExpr -> val2 = currentToken.val;
                nextToken( );
            }
            else return( parserError((char *) "Expected a general reg" ));
        }
        else if ( currentToken.typ == TT_GREG ) {
            
            rExpr -> typ = ET_ADR;
            rExpr -> val1 = currentToken.val;
            nextToken( );
        }
        else if ( ! parseExpr( rExpr )) return( false );
            
        if ( currentToken.typ == TT_RPAREN ) nextToken( );
        else return( parserError((char *) "Expected a right paren" ));
        
        return( true );
    }
    else if ( currentToken.typ == TT_NEG ) {
        
        parseFactor( rExpr );
        rExpr -> val1 = ~ rExpr -> val1;
        return( true );
    }
    else {
        
        parserError((char *) "Expected.... (factor)" );
        rExpr -> typ = TT_NUM;
        rExpr -> val1 = 0;
        nextToken( );
        return( false );
    }
}

//------------------------------------------------------------------------------------------------------------
// "parseTerm" parses the term syntax.
//
//------------------------------------------------------------------------------------------------------------
bool parseTerm( Expr *rExpr ) {
    
    Expr lExpr;
    bool rStat;
    
    rStat = parseFactor( rExpr );
    
    while (( currentToken.typ == TT_MULT )  ||
           ( currentToken.typ == TT_DIV )   ||
           ( currentToken.typ == TT_MOD )   ||
           ( currentToken.typ == TT_AND ))  {
        
        uint8_t op = currentToken.typ;
        
        nextToken( );
        rStat = parseFactor( &lExpr );
        
        switch( op ) {
                
            case TT_MULT:   rExpr -> val1 = rExpr -> val1 * lExpr.val1; break;
            case TT_DIV:    rExpr -> val1 = rExpr -> val1 / lExpr.val1; break;
            case TT_MOD:    rExpr -> val1 = rExpr -> val1 % lExpr.val1; break;
            case TT_AND:    rExpr -> val1 = rExpr -> val1 & lExpr.val1; break;
        }
    }
    
    return( true );
}

//------------------------------------------------------------------------------------------------------------
// "parseExpr" parses the expression syntax. The one line assembler parser routines use this call in many
// places where a numeric expresson or an address is needed.
//
//------------------------------------------------------------------------------------------------------------
bool parseExpr( Expr *rExpr ) {
    
    Expr lExpr;
    bool rStat;
    
    if ( currentToken.typ == TT_PLUS ) {
        
        nextToken( );
        rStat = parseTerm( rExpr );
        
        if ( ! ( rExpr -> typ == TT_NUM ))
            parserError((char *) "Expected a numeric constant" );
    }
    else if ( currentToken.typ == TT_MINUS ) {
        
        nextToken( );
        rStat = parseTerm( rExpr );
        
        if ( rExpr -> typ == ET_NUM ) rExpr -> val1 = - rExpr -> val1;
        else parserError((char *) "Expected a numeric constant" );
    }
    else rStat = parseTerm( rExpr );
    
    while (( currentToken.typ == TT_PLUS ) ||
           ( currentToken.typ == TT_MINUS ) ||
           ( currentToken.typ == TT_OR )    ||
           ( currentToken.typ == TT_XOR )) {
        
        uint8_t op = currentToken.typ;
        
        nextToken( );
        rStat = parseTerm( &lExpr );
        
        switch ( op ) {
                
            case TT_PLUS:   rExpr -> val1 = rExpr -> val1 + lExpr.val1; break;
            case TT_MINUS:  rExpr -> val1 = rExpr -> val1 - lExpr.val1; break;
            case TT_OR:     rExpr -> val1 = rExpr -> val1 | lExpr.val1; break;
            case TT_XOR:    rExpr -> val1 = rExpr -> val1 ^ lExpr.val1; break;
        }
    }
    
    return( true );
}

//------------------------------------------------------------------------------------------------------------
// "parseInstrOptions" will analyze the opCode option string. An opCode string is a sequence of characters.
// We will look at each character in the "name" and set the options for the particular instruction. There are
// also cases where the only option is a multi-character sequence. We detect invalid options but not when
// the same option is repeated. E.g. a "LOL" will result in "L" and "O" set.
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrOptions( uint32_t *instr ) {
    
    char *optBuf = currentToken.name;
    
    if ( strlen( optBuf ) == 0 ) {
        
        parserError((char *) "Expected the option" );
        return( false );
    }
    
    switch( getBitField( *instr, 5, 6 )) {
            
        case OP_LD:
        case OP_ST:
        case OP_LDA:
        case OP_STA:  {
            
            if ( optBuf[ 0 ] == 'M' ) setBit( instr, 11 );
            else return( parserError((char *) "Invalid instruction option" ));
            
        } break;
            
        case OP_ADD:
        case OP_ADC:
        case OP_SUB:
        case OP_SBC: {
            
            for ( int i = 0; i < strlen( optBuf ); i ++ ) {
                
                if      ( optBuf[ i ] == 'L' ) setBit( instr, 10 );
                else if ( optBuf[ i ] == 'O' ) setBit( instr, 11 );
                else return( parserError((char *) "Invalid instruction option" ));
            }
            
        } break;
            
        case OP_AND:
        case OP_OR: {
            
            for ( int i = 0; i < strlen( currentToken.name ); i ++ ) {
                
                if      ( optBuf[ i ] == 'N' ) setBit( instr, 10 );
                else if ( optBuf[ i ] == 'C' ) setBit( instr, 11 );
                else return( parserError((char *) "Invalid instruction option" ));
            }
            
        } break;
            
        case OP_XOR: {
            
            if ( optBuf[ 0 ] == 'N' ) setBit( instr, 10 );
            else return( parserError((char *) "Invalid instruction option" ));
            
        } break;
            
        case OP_CMP:
        case OP_CMPU: {
            
            if ( strcmp( optBuf, ((char *) "EQ" ))) setBitField( instr, 11, 2, 0 );
            if ( strcmp( optBuf, ((char *) "LT" ))) setBitField( instr, 11, 2, 1 );
            if ( strcmp( optBuf, ((char *) "NE" ))) setBitField( instr, 11, 2, 2 );
            if ( strcmp( optBuf, ((char *) "LE" ))) setBitField( instr, 11, 2, 3 );
            
        } break;
            
        case OP_EXTR: {
            
            for ( int i = 0; i < strlen( currentToken.name ); i ++ ) {
                
                if      ( optBuf[ i ] == 'S' ) setBit( instr, 10 );
                else if ( optBuf[ i ] == 'A' ) setBit( instr, 11 );
                else return( parserError((char *) "Invalid instruction option" ));
            }
            
        } break;
            
        case OP_DEP: {
            
            for ( int i = 0; i < strlen( currentToken.name ); i ++ ) {
                
                if      ( optBuf[ i ] == 'Z' ) setBit( instr, 10 );
                else if ( optBuf[ i ] == 'A' ) setBit( instr, 11 );
                else if ( optBuf[ i ] == 'I' ) setBit( instr, 12 );
                else return( parserError((char *) "Invalid instruction option" ));
            }
            
        } break;
            
        case OP_DSR: {
            
            if ( optBuf[ 0 ] == 'A' ) setBit( instr, 11 );
            else return( parserError((char *) "Invalid instruction option" ));
            
        } break;
            
        case OP_SHLA: {
            
            for ( int i = 0; i < strlen( currentToken.name ); i ++ ) {
                
                if      ( optBuf[ i ] == 'I' ) setBit( instr, 10 );
                else if ( optBuf[ i ] == 'L' ) setBit( instr, 11 );
                else if ( optBuf[ i ] == 'O' ) setBit( instr, 12 );
                else return( parserError((char *) "Invalid instruction option" ));
            }
            
        } break;
            
        case OP_MR: {
            
            for ( int i = 0; i < strlen( currentToken.name ); i ++ ) {
                
                if      ( optBuf[ i ] == 'D' ) setBit( instr, 10 );
                else if ( optBuf[ i ] == 'M' ) setBit( instr, 11 );
                else return( parserError((char *) "Invalid instruction option" ));
            }
            
        } break;
            
        case OP_MST: {
            
            for ( int i = 0; i < strlen( currentToken.name ); i ++ ) {
                
                if      ( optBuf[ i ] == 'S' ) setImmValU( instr, 11, 2, 1 );
                else if ( optBuf[ i ] == 'C' ) setImmValU( instr, 11, 2, 2 );
                else return( parserError((char *) "Invalid instruction option" ));
            }
    
        } break;
            
        case OP_PRB: {
            
            for ( int i = 0; i < strlen( currentToken.name ); i ++ ) {
                
                if      ( optBuf[ i ] == 'W' ) setBit( instr, 10 );
                else if ( optBuf[ i ] == 'I' ) setBit( instr, 11 );
                else return( parserError((char *) "Invalid instruction option" ));
            }
            
        } break;
            
        case OP_ITLB: {
            
            if ( optBuf[ 0 ] == 'T' ) setBit( instr, 11 );
            else return( parserError((char *) "Invalid instruction option" ));
            
        } break;
            
        case OP_PTLB: {
            
            for ( int i = 0; i < strlen( currentToken.name ); i ++ ) {
                
                if      ( optBuf[ i ] == 'T' ) setBit( instr, 10 );
                else if ( optBuf[ i ] == 'M' ) setBit( instr, 11 );
                else return( parserError((char *) "Invalid instruction option" ));
            }
            
        } break;
            
        case OP_PCA: {
            
            for ( int i = 0; i < strlen( currentToken.name ); i ++ ) {
                
                if      ( optBuf[ i ] == 'T' ) setBit( instr, 10 );
                else if ( optBuf[ i ] == 'M' ) setBit( instr, 11 );
                else if ( optBuf[ i ] == 'F' ) setBit( instr, 14 );
                else return( parserError((char *) "Invalid instruction option" ));
            }
            
        } break;
            
        default: return( parserError((char *) "Instruction has no option" ));
    }
    
    return( true );
}

//------------------------------------------------------------------------------------------------------------
// "parseModeTypeInstr" parses all instructions that have an "operand" encoding. The syntax is as follows:
//
//      opCode [ "." opt ] <targetReg> "," <num>                                - mode 0
//      opCode [ "." opt ] <targetReg> "," <num> "(" <baseReg> ")"              - mode 3
//      opCode [ "." opt ] <targetReg> "," <sourceReg>                          - mode 1
//      opCode [ "." opt ] <targetReg> "," <sourceRegA> "," "<sourceRegB>       - mode 1
//      opCode [ "." opt ] <targetReg> "," <indexReg> "(" <baseReg> ")"         - mode 2
//
// There is one ugly thing. The "dw" field is only applicable to the mode 2 and 3 type instructions. When
// the input is for example "ADDB r1, 100", we cannot detect based on the 2dw" set in the template that this
// is an invalid combination. For the translation no harm is done, but it would be nice to detect this and
// report an error. Perhaps we need to enhance the tokTable with some further information for these kind of
// cases. We could however also just accept it and detect that the numeric value for mode zero does not match
// the range indicated by the instruction .... Hmmm.
//
//------------------------------------------------------------------------------------------------------------
bool parseModeTypeInstr( uint32_t *instr ) {
    
    uint8_t targetRegId = 0;
    Expr    rExpr;
    
    if ( currentToken.typ == TT_GREG ) {
        
        targetRegId = currentToken.val;
        setBitField( instr, 9, 4, currentToken.val );
        nextToken( );
    }
    else return( parserError((char *) "Expected a general register" ));
    
    if ( currentToken.typ == TT_COMMA ) nextToken( );
    else return( parserError((char *) "Expected a Comma" ));
    
    if ( ! parseExpr( &rExpr )) return( false );
    
    if ( rExpr.typ == ET_NUM ) {
        
        uint32_t tokVal = rExpr.val1;
        
        nextToken( );
        if ( currentToken.typ == TT_LPAREN ) {
            
            if ( isInRangeForBitField( tokVal, 12 )) {
                
                setImmVal( instr, 27, 12, tokVal );
                nextToken( );
            }
            else return( parserError((char *) "Immediate value out of range" ));
            
            if ( currentToken.typ == TT_GREG ) {
                
                setBitField( instr, 13, 2, 3 );
                setBitField( instr, 31, 4, currentToken.val );
                
                nextToken( );
                if ( currentToken.typ == TT_RPAREN ) nextToken( );
                else return( parserError((char *) "Expected a right paren" ));
                
                checkEOS( );
            }
            else return( parserError((char *) "Expected a general reg" ));
        }
        else if ( currentToken.typ == TT_EOS ) {
            
            if ( isInRangeForBitField( tokVal, 18 )) {
                
                setImmVal( instr, 31, 18, tokVal );
            }
            else return( parserError((char *) "Immediate value out of range" ));
            
            nextToken( );
            checkEOS( );
        }
        else return( parserError((char *) "Invalid operand" ));
    }
    else if ( currentToken.typ == TT_GREG ) {
        
        uint8_t operandRegId = currentToken.val;
        
        nextToken( );
        if ( currentToken.typ == TT_EOS ) {
            
            setBitField( instr, 13, 2, 1 );
            setBitField( instr, 27, 4, targetRegId );
            setBitField( instr, 31, 4, operandRegId );
        }
        else if ( currentToken.typ == TT_COMMA ) {
            
            nextToken( );
            if ( currentToken.typ == TT_GREG ) {
                
                setBitField( instr, 13, 2, 1 );
                setBitField( instr, 27, 4, operandRegId );
                setBitField( instr, 31, 4, currentToken.val );
                
                nextToken( );
                checkEOS( );
            }
            else return( parserError((char *) "Expected a general reg" ));
        }
        else if ( currentToken.typ == TT_LPAREN ) {
            
            nextToken( );
            if ( currentToken.typ == TT_GREG ) {
                
                setBitField( instr, 13, 2, 2 );
                setBitField( instr, 27, 4, operandRegId );
                setBitField( instr, 31, 4, currentToken.val );
                
                nextToken( );
            }
            else return( parserError((char *) "Expected a general reg" ));
            
            if ( currentToken.typ == TT_RPAREN ) nextToken( );
            else return( parserError((char *) "Expected a right paren" ));
            
            checkEOS( );
        }
        else return( parserError((char *) "Invalid operand" ));
    }
    else return( parserError((char *) "Invalid operand" ));
    
    if (( getBitField( *instr, 13, 2 ) == 1 ) && ( getBitField( *instr, 15, 2 ) != 0 )) {
        
        return( parserError((char *) "Invalid opCode data width specifier for mode option" ));
    }
    
    return( true );
}

//------------------------------------------------------------------------------------------------------------
// "parseInstrLSID" parses the LSID instruction.
//
//      <opCode> <targetReg> "," <sourceReg>
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrLSID( uint32_t *instr ) {
    
    if ( currentToken.typ == TT_GREG ) {
        
        setBitField( instr, 9, 4, currentToken.val );
        nextToken( );
    }
    else return( parserError((char *) "Expected a general register" ));
    
    if ( currentToken.typ == TT_COMMA ) nextToken( );
    else return( parserError((char *) "Expected a Comma" ));
    
    if ( currentToken.typ == TT_GREG ) {
        
        setBitField( instr, 31, 4, currentToken.val );
        nextToken( );
    }
    else return( parserError((char *) "Expected a general register" ));
    
    return( checkEOS( ));
}

//------------------------------------------------------------------------------------------------------------
// "parseInstrEXTRandDEP" parses the extract or deposit instruction. The instruction has two basic formats.
// When the "A" bit is set, the position will be obtained from the shift amount control register. Otherwise
// it is encoded in the instruction.
//
//      EXTR [ ".“ <opt> ] <targetReg> "," <sourceReg> "," <pos> "," <len"
//      EXTR [ "." "A" <opt> ] <targetReg> "," <sourceReg> ", <len"
//
//      DEP [ ".“ <opt> ] <targetReg> "," <sourceReg> "," <pos> "," <len"
//      DEP [ "." "A" <opt> ] <targetReg> "," <sourceReg> ", <len"
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrEXTRandDEP( uint32_t *instr ) {
    
    if ( currentToken.typ == TT_GREG ) {
        
        setBitField( instr, 9, 4, currentToken.val );
        nextToken( );
    }
    else return( parserError((char *) "Expected a general register" ));
    
    if ( currentToken.typ == TT_COMMA ) nextToken( );
    else return( parserError((char *) "Expected a Comma" ));
    
    if ( currentToken.typ == TT_GREG ) {
        
        setBitField( instr, 31, 4, currentToken.val );
        nextToken( );
    }
    else return( parserError((char *) "Expected a general register" ));
    
    if ( currentToken.typ == TT_COMMA ) nextToken( );
    else return( parserError((char *) "Expected a Comma" ));
    
    if ( currentToken.typ == TT_NUM ) {
        
        if ( isInRangeForBitFieldU( currentToken.val, 5 )) {
            
            if ( getBit( *instr, 11 ))  setBitField( instr, 21, 5, currentToken.val );
            else                        setBitField( instr, 27, 5, currentToken.val );
        }
        else return( parserError((char *) "Immediate value out of range" ));
        
        nextToken( );
    }
    else return( parserError((char *) "Expected a Comma" ));
    
    if ( ! getBit( *instr, 11 )) {
        
        if ( currentToken.typ == TT_COMMA ) nextToken( );
        else return( parserError((char *) "Expected a Comma" ));
        
        nextToken( );
        if ( currentToken.typ == TT_NUM ) {
            
            if ( isInRangeForBitFieldU( currentToken.val, 5 )) {
                
                setBitField( instr, 27, 5, currentToken.val );
                nextToken( );
            }
            else return( parserError((char *) "Immediate value out of range" ));
        }
        else return( parserError((char *) "Expected a number" ));
    }
    
    return( checkEOS( ));
}

//------------------------------------------------------------------------------------------------------------
// The DSR instruction parses the double shift instruction. There are two flavors. If the "A" bit is set, the
// shift amount is taken from the shift amount control register, else from the instruction "len" field.
//
//      DSR [ ".“ <opt> ] <targetReg> "," <sourceRegA> "," <sourceRegB> "," <len"
//      DSR [ ".“ "A"   ] <targetReg> "," <sourceRegA> "," <sourceRegB>
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrDSR( uint32_t *instr ) {
    
    if ( currentToken.typ == TT_GREG ) {
        
        setBitField( instr, 9, 4, currentToken.val );
        nextToken( );
    }
    else return( parserError((char *) "Expected a general register" ));
    
    if ( currentToken.typ == TT_COMMA ) nextToken( );
    else return( parserError((char *) "Expected a Comma" ));
    
    if ( currentToken.typ == TT_GREG ) {
        
        setBitField( instr, 27, 4, currentToken.val );
        nextToken( );
    }
    else return( parserError((char *) "Expected a general register" ));
    
    if ( currentToken.typ == TT_COMMA ) nextToken( );
    else return( parserError((char *) "Expected a Comma" ));
    
    if ( currentToken.typ == TT_GREG ) {
        
        setBitField( instr, 31, 4, currentToken.val );
        nextToken( );
    }
    else return( parserError((char *) "Expected a general register" ));
    
    if ( ! getBit( *instr, 11 )) {
        
        if ( currentToken.typ == TT_COMMA ) nextToken( );
        else return( parserError((char *) "Expected a Comma" ));
        
        nextToken( );
        if ( currentToken.typ == TT_NUM ) {
            
            if ( isInRangeForBitFieldU( currentToken.val, 5 )) {
                
                setBitField( instr, 21, 5, currentToken.val );
                nextToken( );
            }
            else return( parserError((char *) "Immediate value out of range" ));
        }
        else return( parserError((char *) "Expected a number" ));
    }
    
    return( checkEOS( ));
}

//------------------------------------------------------------------------------------------------------------
// The DS instruction parses the divide step instruction.
//
//      DS <targetReg> "," <sourceRegA> "," <sourceRegB>
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrDS( uint32_t *instr ) {
    
    if ( currentToken.typ == TT_GREG ) {
        
        setBitField( instr, 9, 4, currentToken.val );
        nextToken( );
    }
    else return( parserError((char *) "Expected a general register" ));
    
    if ( currentToken.typ == TT_COMMA ) nextToken( );
    else return( parserError((char *) "Expected a Comma" ));
    
    if ( currentToken.typ == TT_GREG ) {
        
        setBitField( instr, 27, 4, currentToken.val );
        nextToken( );
    }
    else return( parserError((char *) "Expected a general register" ));
    
    if ( currentToken.typ == TT_COMMA ) nextToken( );
    else return( parserError((char *) "Expected a Comma" ));
    
    if ( currentToken.typ == TT_GREG ) {
        
        setBitField( instr, 31, 4, currentToken.val );
        nextToken( );
    }
    else return( parserError((char *) "Expected a general register" ));
    
    return( checkEOS( ));
}

//------------------------------------------------------------------------------------------------------------
// The SHLA instruction performs a shift left of "B" by "sa" and adds the "A" register to it.
//
//      SHLA [ "." <opt> ] <targetReg> "," <sourceRegA> "," <sourceRegB>
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrSHLA( uint32_t *instr ) {
    
    if ( currentToken.typ == TT_GREG ) {
        
        setBitField( instr, 9, 4, currentToken.val );
        nextToken( );
    }
    else return( parserError((char *) "Expected a general register" ));
    
    if ( currentToken.typ == TT_COMMA ) nextToken( );
    else return( parserError((char *) "Expected a Comma" ));
    
    if ( currentToken.typ == TT_GREG ) {
        
        setBitField( instr, 27, 4, currentToken.val );
        nextToken( );
    }
    else return( parserError((char *) "Expected a general register" ));
    
    if ( currentToken.typ == TT_COMMA ) nextToken( );
    else return( parserError((char *) "Expected a Comma" ));
    
    if ( currentToken.typ == TT_GREG ) {
        
        setBitField( instr, 31, 4, currentToken.val );
        nextToken( );
    }
    else return( parserError((char *) "Expected a general register" ));
    
    if ( currentToken.typ == TT_COMMA ) nextToken( );
    else return( parserError((char *) "Expected a Comma" ));
    
    if ( currentToken.typ == TT_NUM ) {
        
        if ( isInRangeForBitFieldU( currentToken.val, 2 )) {
            
            setBitField( instr, 21, 2, currentToken.val );
            nextToken( );
        }
        else return( parserError((char *) "Immediate value out of range" ));
    }
    else return( parserError((char *) "Expected the shift amount" ));
    
    return( checkEOS( ));
}

//------------------------------------------------------------------------------------------------------------
// The CMR instruction tests register "B" for a condition and if true copies the "A" value to "R".
//
//      CMR <targetReg> "," <regA> "," <regB>
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrCMR( uint32_t *instr ) {
    
    if ( currentToken.typ == TT_GREG ) {
        
        setBitField( instr, 9, 4, currentToken.val );
        nextToken( );
    }
    else return( parserError((char *) "Expected a general register" ));
    
    if ( currentToken.typ == TT_COMMA ) nextToken( );
    else return( parserError((char *) "Expected a Comma" ));
    
    if ( currentToken.typ == TT_GREG ) {
        
        setBitField( instr, 27, 4, currentToken.val );
        nextToken( );
    }
    else return( parserError((char *) "Expected a general register" ));
    
    if ( currentToken.typ == TT_COMMA ) nextToken( );
    else return( parserError((char *) "Expected a Comma" ));
    
    if ( currentToken.typ == TT_GREG ) {
        
        setBitField( instr, 31, 4, currentToken.val );
        nextToken( );
    }
    else return( parserError((char *) "Expected a general register" ));
    
    return( checkEOS( ));
}

//------------------------------------------------------------------------------------------------------------
// The "LDIL" instruction loads the immediate value encoded in the instruction left shifted into "R". The
// "ADDIL" instruction will add the value encoded in the instruction left shifted to "R". The result is
// in R1.
//
//      LDIL <targetReg> "," <val>
//      ADDIL <sourceReg> "," <val>
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrLDILandADDIL( uint32_t *instr ) {
    
    if ( currentToken.typ == TT_GREG ) {
        
        setBitField( instr, 9, 4, currentToken.val );
        nextToken( );
    }
    else return( parserError((char *) "Expected a general register" ));
    
    if ( currentToken.typ == TT_COMMA ) nextToken( );
    else return( parserError((char *) "Expected a Comma" ));
    
    if ( currentToken.typ == TT_NUM ) {
        
        if ( isInRangeForBitFieldU( currentToken.val, 22 )) {
            
            setImmValU( instr, 31, 22, currentToken.val );
            nextToken( );
        }
        else return( parserError((char *) "Immediate value out of range" ));
    }
    else return( parserError((char *) "Expected the shift amount" ));
    
    return( checkEOS( ));
}

//------------------------------------------------------------------------------------------------------------
// The "LDO" instruction computes the address of an operand, and stores the result in "R".
//
//      LDO <targetReg> "," [ <ofs> "," ] "(" <baseReg> ")"
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrLDO( uint32_t *instr ) {
    
    if ( currentToken.typ == TT_GREG ) {
        
        setBitField( instr, 9, 4, currentToken.val );
        nextToken( );
    }
    else return( parserError((char *) "Expected a general register" ));
    
    if ( currentToken.typ == TT_COMMA ) nextToken( );
    else return( parserError((char *) "Expected a Comma" ));
    
    if ( currentToken.typ == TT_NUM ) {
        
        if ( isInRangeForBitField( currentToken.val, 18 )) {
            
            setImmVal( instr, 27, 18, currentToken.val );
        }
        else return( parserError((char *) "Immediate value out of range" ));
        
        if ( currentToken.typ == TT_LPAREN ) nextToken( );
        else return( parserError((char *) "Expected a left paren" ));
    }
    else if ( currentToken.typ == TT_LPAREN ) {
        
        nextToken( );
    }
    else return( parserError((char *) "Expected an offset or  left paren" ));
    
    if ( currentToken.typ == TT_GREG ) {
        
        setBitField( instr, 31, 4, currentToken.val );
        nextToken( );
    }
    else return( parserError((char *) "Expected a general register" ));
    
    if ( currentToken.typ == TT_RPAREN ) nextToken( );
    else return( parserError((char *) "Expected a right paren" ));
    
    return( checkEOS( ));
}

//------------------------------------------------------------------------------------------------------------
// The "B" and "GATE" instruction represent an instruction offset relative branch. Optionally, there is an
// optional return register. When omitted, R0 is used in the instruction generation.
//
//      B       <offsetReg> [ "," <returnReg> ]
//      GATE    <offsetReg> [ "," <returnReg> ]
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrBandGATE( uint32_t *instr ) {
    
    if ( currentToken.typ == TT_NUM ) {
        
        if ( isInRangeForBitField( currentToken.val, 22 )) {
            
            setImmVal( instr, 31, 22, currentToken.val );
            nextToken( );
        }
        else return( parserError((char *) "Immediate value out of range" ));
    }
    
    if ( currentToken.typ == TT_COMMA ) {
        
        nextToken( );
        if ( currentToken.typ == TT_GREG ) {
            
            setBitField( instr, 9, 4, currentToken.val );
            nextToken( );
        }
        else return( parserError((char *) "Expected a general reg" ));
    }
    
    return( checkEOS( ));
}

//------------------------------------------------------------------------------------------------------------
// The "BR" instruction is an IA-relative branch with the offset to be added in a general register. There is
// also an optional return register. When omitted, R0 is used in the instruction generation.
//
//      BR <branchReg> [ "," <returnReg> ]
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrBR( uint32_t *instr ) {
    
    if ( currentToken.typ == TT_GREG ) {
        
        setBitField( instr, 31, 4, currentToken.val );
        nextToken( );
    }
    else return( parserError((char *) "Expected a general register" ));
    
    if ( currentToken.typ == TT_COMMA ) {
        
        nextToken( );
        if ( currentToken.typ == TT_GREG ) {
            
            setBitField( instr, 9, 4, currentToken.val );
            nextToken( );
        }
        else return( parserError((char *) "Expected a general register" ));
    }
    
    return( checkEOS( ));
}

//------------------------------------------------------------------------------------------------------------
// The "BV" is an absolute branch address instruction in the same segment. Optionally, there is an optional
// return register. When omitted, R0 is used in the instruction generation.
//
//      BV "(" <targetAdrReg> ")" [ "," <returnReg> ]
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrBV( uint32_t *instr ) {
    
    if ( currentToken.typ == TT_LPAREN ) nextToken( );
    else return( parserError((char *) "Expected a left paren" ));
    
    if ( currentToken.typ == TT_GREG ) {
        
        setBitField( instr, 31, 4, currentToken.val );
        nextToken( );
    }
    else return( parserError((char *) "Expected a general register" ));
    
    if ( currentToken.typ == TT_RPAREN ) nextToken( );
    else return( parserError((char *) "Expected a right paren" ));
    
    if ( currentToken.typ == TT_COMMA ) {
        
        nextToken( );
        if ( currentToken.typ == TT_GREG ) {
            
            setBitField( instr, 31, 4, currentToken.val );
            nextToken( );
        }
        else return( parserError((char *) "Expected a general register" ));
    }
    
    return( checkEOS( ));
}

//------------------------------------------------------------------------------------------------------------
// The "BE" instruction is an external branch to a segment and a segment relative offset. When the offset
// part is omitted, a zero is used. There is also an optional return register. When omitted, R0 is used in
// the instruction generation.
//
//      BE [ <ofs> ] "(" <seg> "," <ofsReg> ")" [ "," <retSeg> ]
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrBE( uint32_t *instr ) {
    
    Expr rExpr;
    
    if (( parseExpr( &rExpr )) && ( rExpr.typ == ET_NUM )) {
        
        if ( isInRangeForBitField( rExpr.val1, 22 )) setImmVal( instr, 23, 14, rExpr.val1 );
        else return( parserError((char *) "Immediate value out of range" ));
    }
    
    if ( currentToken.typ == TT_LPAREN ) nextToken( );
    else return( parserError((char *) "Expected a left paren" ));
    
    if ( currentToken.typ == TT_SREG ) {
        
        setBitField( instr, 27, 4, currentToken.val );
        nextToken( );
    }
    else return( parserError((char *) "Expected a segment register" ));
    
    if ( currentToken.typ == TT_COMMA ) nextToken( );
    else return( parserError((char *) "Expected a comma" ));
    
    if ( currentToken.typ == TT_GREG ) {
        
        setBitField( instr, 31, 4, currentToken.val );
        nextToken( );
    }
    else return( parserError((char *) "Expected a general register" ));
    
    if ( currentToken.typ == TT_RPAREN ) nextToken( );
    else return( parserError((char *) "Expected a right paren" ));
    
    if ( currentToken.typ == TT_COMMA ) {
        
        nextToken( );
        if ( currentToken.typ == TT_GREG ) {
            
            setBitField( instr, 9, 4, currentToken.val );
            nextToken( );
        }
        else return( parserError((char *) "Expected a general register" ));
    }
    
    return( checkEOS( ));
}

//------------------------------------------------------------------------------------------------------------
// The "BVE" instruction forms a logical address by adding general register "a" to base register "b". There
// is also an optional return register. When omitted, R0 is used in the instruction generation.
//
//      BVE [ <offsetReg> ] "(" <baseReg> ")" [ "," <returnReg> ]
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrBVE( uint32_t *instr ) {
    
    if ( currentToken.typ == TT_GREG ) {
        
        setBitField( instr, 27, 4, currentToken.val );
        nextToken( );
    }
    
    if ( currentToken.typ == TT_LPAREN ) nextToken( );
    else return( parserError((char *) "Expected a left paren" ));
    
    if ( currentToken.typ == TT_GREG ) {
        
        setBitField( instr, 31, 4, currentToken.val );
        nextToken( );
    }
    else return( parserError((char *) "Expected a general register" ));
    
    if ( currentToken.typ == TT_RPAREN ) nextToken( );
    else return( parserError((char *) "Expected a right paren" ));
    
    if ( currentToken.typ == TT_COMMA ) {
        
        nextToken( );
        if ( currentToken.typ == TT_GREG ) {
            
            setBitField( instr, 9, 4, currentToken.val );
            nextToken( );
        }
        else return( parserError((char *) "Expected a general register" ));
    }
    
    return( checkEOS( ));
}

//------------------------------------------------------------------------------------------------------------
// The "CBR" and "CBRU" compare register "a" and "b" based on the condition and branch if the comparison
// result is true. The condition code is encoded in the instruction option string parsed before.
//
//      CBR  .<cond> <a>, <b>, <ofs>
//      CBRU .<cond> <a>, <b>, <ofs>
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrCBRandCBRU( uint32_t *instr ) {
    
    Expr rExpr;
    
    if ( currentToken.typ == TT_GREG ) {
        
        setBitField( instr, 27, 4, currentToken.val );
        nextToken( );
    }
    
    if ( currentToken.typ == TT_COMMA ) nextToken( );
    else return( parserError((char *) "Expected a comma" ));
    
    if ( currentToken.typ == TT_GREG ) {
        
        setBitField( instr, 31, 4, currentToken.val );
        nextToken( );
    }
    else return( parserError((char *) "Expected a general register" ));
    
    if ( currentToken.typ == TT_COMMA ) nextToken( );
    else return( parserError((char *) "Expected a comma" ));
    
    if (( parseExpr( &rExpr )) && ( rExpr.typ == ET_NUM )) {
    
        if ( isInRangeForBitField( rExpr.val1, 16 )) {
            
            setImmVal( instr, 23, 16, rExpr.val1 );
            nextToken( );
        }
        else return( parserError((char *) "Immediate value out of range" ));
    }
    else return( parserError((char *) "Expected an offset value" ));
    
    return( checkEOS( ));
}

//------------------------------------------------------------------------------------------------------------
// "parseLoadStoreOperand" parses the operand portion of the load and store instruction family. It represents
// the source location for the load type instruction and the target for the store type instruction. The syntax
// for the <operand> portion is either a
//
//      <ofs> "(" SR "," GR ")"
//      <ofs> "(" GR ")"
//      <GR>  "(" SR "," GR ")"
//      <GR>  "(" GR ")"
//
// <loadInstr>  [ "." <opt> ] <targetReg>       "," <sourceOperand>
// <storeInstr> [ "." <opt> ] <targetOperand>   "," <sourceReg>
//
//------------------------------------------------------------------------------------------------------------
bool parseLoadStoreOperand( uint32_t *instr ) {
    
    Expr rExpr;
    
    if ( ! parseExpr(&rExpr )) return( false );
    
    if ( rExpr.typ == ET_NUM ) {
        
        if ( isInRangeForBitField( rExpr.val1, 12 )) setImmVal( instr, 27, 12, rExpr.val1 );
        else return( parserError((char *) "Immediate value out of range" ));
    }
    else if ( rExpr.typ == ET_GREG ) {
        
        setBit( instr, 10 );
        setBitField( instr, 27, 4, rExpr.val1 );
    }
    else return( parserError((char *) "Expected an offset" ));
        
    if ( parseExpr( &rExpr )) {
                
        if ( rExpr.typ == ET_ADR ) {
                    
            setBitField( instr, 13, 2, 0 );
            setBitField( instr, 31, 4, rExpr.val1 );
        }
        else if ( rExpr.typ == ET_EXT_ADR ) {
                    
            if (( getBitField( *instr, 5, 6 ) == OP_LDA ) || ( getBitField( *instr, 5, 6 ) == OP_STA )) {
                    
                return( parserError((char *) "Invalid address for instruction type" ));
            }
                
            if ( isInRange( rExpr.val1, 1, 3 )) setBitField( instr, 13, 2, rExpr.val1 );
            else return( parserError((char *) "Expected SR1 .. SR3 " ));
                    
            setBitField( instr, 31, 4, rExpr.val2 );
        }
        else return( parserError((char *) "Expected an address" ));
    }
    else return( parserError((char *) "Expected an operand" ));
    
    return( true );
}

//------------------------------------------------------------------------------------------------------------
// "parseInstrLoad" will parse the load instructions family. The workhorse is the "parseLoadStoreOperand"
// routine, which parses the operand. General form:
//
//      <opCode>.<opt> <targetReg>, <sourceOperand>
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrLoad( uint32_t *instr ) {
    
    if ( currentToken.typ == TT_GREG ) {
        
        setBitField( instr, 9, 4, currentToken.val );
        nextToken( );
    }
    else return( parserError((char *) "Expected a general register" ));
    
    if ( currentToken.typ == TT_COMMA )  nextToken( );
    else return( parserError((char *) "Expected a comma" ));
    
    return( parseLoadStoreOperand( instr ));
}

//------------------------------------------------------------------------------------------------------------
// "parseInstrSTx" will parse the store instruction family. The workhorse is the "parseLoadStoreOperand"
// routine, which parses the target. General form:
//
//      <opCode>.<opt> <targetOperand>, <sourceReg>
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrStore( uint32_t *instr ) {
    
    if ( ! parseLoadStoreOperand( instr )) return( false );
    
    if ( currentToken.typ == TT_COMMA ) nextToken( );
    else return( parserError((char *) "Expected a comma" ));
  
    if ( currentToken.typ == TT_GREG ) setBitField( instr, 9, 4, currentToken.val );
    else return( parserError((char *) "Expected a general register" ));

    nextToken( );
    return( checkEOS( ));
}

//------------------------------------------------------------------------------------------------------------
// The "MR" instruction is a move register instruction. We parse valid combination and assemble the
// instruction. Note that the "MR" instruction is primarily used for moving segment and control registers
// to and from a general register. However, the syntax can also be used to move between general registers.
// We will in this case emit an "OR" instruction.
//
//      MR <targetReg> "," <sourceReg>
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrMR( uint32_t *instr ) {
    
    if ( currentToken.typ == TT_GREG ) {
        
        uint8_t tRegId = currentToken.val;
        
        nextToken( );
        if ( currentToken.typ == TT_COMMA ) nextToken( );
        else return( parserError((char *) "Expected a comma" ));
        
        if ( currentToken.typ == TT_GREG ) {
            
            *instr = 0;
            setBitField( instr, 5, 6, OP_OR );
            setBitField( instr, 9, 4, tRegId );
            setBitField( instr, 13, 2, 1 );
            setBitField( instr, 27, 4, 0 );
            setBitField( instr, 31, 4, currentToken.val );
            nextToken( );
        }
        else if ( currentToken.typ == TT_SREG ) {
            
            uint8_t tRegId = currentToken.val;
            
            nextToken( );
            if ( currentToken.typ == TT_COMMA ) nextToken( );
            else return( parserError((char *) "Expected a comma" ));
            
            if ( currentToken.typ == TT_GREG ) {
                
                setBitField( instr, 31, 3, currentToken.val );
                setBitField( instr, 9, 4, tRegId );
                nextToken( );
            }
            else return( parserError((char *) "Only GREG <- SREG is allowed" ));
        }
        else if ( currentToken.typ == TT_CREG ) {
            
            uint8_t tRegId = currentToken.val;
            
            nextToken( );
            if ( currentToken.typ == TT_COMMA ) nextToken( );
            else return( parserError((char *) "Expected a comma" ));
            
            if ( currentToken.typ == TT_GREG ) {
                
                setBit( instr, 11 );
                setBitField( instr, 31, 5, currentToken.val );
                setBitField( instr, 9, 4, tRegId );
                nextToken( );
            }
            else return( parserError((char *) "Only GREG <- CREG is allowed" ));
        }
    }
    else if ( currentToken.typ == TT_SREG ) {
        
        uint8_t tRegId = currentToken.val;
        
        nextToken( );
        if ( currentToken.typ == TT_COMMA ) nextToken( );
        else return( parserError((char *) "Expected a comma" ));
        
        if ( currentToken.typ == TT_GREG ) {
            
            setBit( instr, 10 );
            setBitField( instr, 31, 3, tRegId );
            setBitField( instr, 9, 4, currentToken.val );
            nextToken( );
        }
        else return( parserError((char *) "Only SREG <- GREG is allowed" ));
    }
    else if ( currentToken.typ == TT_CREG ) {
        
        uint8_t tRegId = currentToken.val;
        
        nextToken( );
        if ( currentToken.typ == TT_COMMA ) nextToken( );
        else return( parserError((char *) "Expected a comma" ));
        
        if ( currentToken.typ == TT_GREG ) {
            
            setBit( instr, 10 );
            setBit( instr, 11 );
            setBitField( instr, 31, 5, tRegId );
            setBitField( instr, 9, 4, currentToken.val );
            nextToken( );
        }
        else return( parserError((char *) "Only CREG <- GREG is allowed" ));
    }
    
    return( checkEOS( ));
}

//------------------------------------------------------------------------------------------------------------
// The "MST" instruction sets and clears bits in the program state word. There are two basic formats. The
// first format will use a general register for the data bits, the second format will use the value encoded
// in the instruction.
//
//      MST b
//      MST.S <val>
//      MST.C <val>
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrMST( uint32_t *instr ) {
    
    Expr rExpr;
    
    if ( currentToken.typ == TT_GREG ) {
        
        setBitField( instr, 9, 4, currentToken.val );
        nextToken( );
    }
    else return( parserError((char *) "Expected a general register" ));
    
    if ( currentToken.typ == TT_COMMA ) nextToken( );
    else return( parserError((char *) "Expected a comma" ));
    
    if ( ! parseExpr( &rExpr )) return( false );
    
    if ( rExpr.typ == ET_GREG ) {
        
        if ( getBitField( *instr, 11, 2 ) == 0 ) {
            
            setBitField( instr, 31, 4, rExpr.val1 );
            nextToken( );
        }
        else return( parserError((char *) "Invalid option for the MST instruction" ));
    }
    else if ( rExpr.typ == ET_NUM ) {
        
        if (( getBitField( *instr, 11, 2 ) == 1 ) || ( getBitField( *instr, 11, 2 ) == 2 )) {
            
            if ( isInRangeForBitFieldU( rExpr.val1, 6 )) {
                
                setBitField( instr, 31, 6, rExpr.val1 );
                nextToken( );
            }
            else return( parserError((char *) "Status bit field value out of range" ));
        }
        else  return( parserError((char *) "Invalid option for the MST instruction" ));
    }
    else return( parserError((char *) "Expected the status bit argument" ));
  
    return( checkEOS( ));
}

//------------------------------------------------------------------------------------------------------------
// The "LDPA" instruction loads a physical address for the logical address. When the segment is explicitly
// used, it must be in the range of SR1 to SR3.
//
//      LDPA <targetReg> ","  <indexReg" "(" [ <segmentReg>, ] <offsetReg > ")"
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrLDPA( uint32_t *instr ) {
    
    if ( currentToken.typ == TT_GREG ) {
        
        setBitField( instr, 9, 4, currentToken.val );
        nextToken( );
    }
    
    if ( currentToken.typ == TT_COMMA ) nextToken( );
    else return( parserError((char *) "Expected a comma" ));
    
    if ( currentToken.typ == TT_GREG ) {
        
        setBitField( instr, 27, 4, currentToken.val );
        nextToken( );
    }
    else if ( currentToken.typ == TT_LPAREN ) {
        
        nextToken( );
        if ( currentToken.typ == TT_SREG ) {
            
            if ( isInRange( currentToken.val, 1, 3 )) {
                
                setBitField( instr, 13, 2, currentToken.val );
                nextToken( );
            }
            else return( parserError((char *) "Expected SR1 .. SR3 " ));
            
            if ( currentToken.typ == TT_COMMA ) nextToken( );
            else return( parserError((char *) "Expected a comma" ));
        }
        
        if ( currentToken.typ == TT_GREG ) {
            
            setBitField( instr, 31, 4, currentToken.val );
            nextToken( );
            
            if ( currentToken.typ == TT_RPAREN ) nextToken( );
            else return( parserError((char *) "Expected a right paren" ));
        }
    }
    else return( parserError((char *) "Expected an index Reg or left paren" ));
    
    return( checkEOS( ));
}

//------------------------------------------------------------------------------------------------------------
// The "PRB" instruction will test a logical address for the desired read or write access. The "I" bit will
// when cleared use the "A" reg as input, else bit 27 of the instruction.
//
//      PRB <targetReg> ","  <indexReg" "(" [ <segmentReg>, ] <offsetReg > ")"
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrPRB( uint32_t *instr ) {
    
    if ( currentToken.typ == TT_GREG ) {
        
        setBitField( instr, 9, 4, currentToken.val );
        nextToken( );
    }
    
    if ( currentToken.typ == TT_COMMA ) nextToken( );
    else return( parserError((char *) "Expected a comma" ));
    
    if ( currentToken.typ == TT_GREG ) {
        
        setBitField( instr, 27, 4, currentToken.val );
        nextToken( );
    }
    
    if ( currentToken.typ == TT_LPAREN ) nextToken( );
    else return( parserError((char *) "Expected a left paren" ));
    
    if ( currentToken.typ == TT_SREG ) {
        
        if ( isInRange( currentToken.val, 1, 3 )) {
            
            setBitField( instr, 13, 2, currentToken.val );
            nextToken( );
        }
        else return( parserError((char *) "Expected SR1 .. SR3 " ));
        
        if ( currentToken.typ == TT_COMMA ) nextToken( );
        else return( parserError((char *) "Expected a comma" ));
    }
    
    if ( currentToken.typ == TT_GREG ) {
        
        setBitField( instr, 31, 4, currentToken.val );
        nextToken( );
    }
    
    if ( currentToken.typ == TT_RPAREN ) nextToken( );
    else return( parserError((char *) "Expected a right paren" ));
    
    if ( currentToken.typ == TT_COMMA ) nextToken( );
    else return( parserError((char *) "Expected a comma" ));
    
    if ( getBit( *instr, 11 )) {
        
        if ( currentToken.typ == TT_NUM ) {
            
            if ( isInRangeForBitFieldU( currentToken.val, 1 )) {
                
                setBit( instr, currentToken.val );
                nextToken( );
            }
        }
        else return( parserError((char *) "Expected a 0 or 1" ));
    }
    else if ( currentToken.typ == TT_GREG ) {
        
        setBitField( instr, 27, 4, currentToken.val );
        nextToken( );
    }
    else return( parserError((char *) "Expected a register or numeric value" ));
    
    return( checkEOS( ));
}

//------------------------------------------------------------------------------------------------------------
// The "ITLB" instruction will insert a new entry in the instruction or data TLB. We use the segment and
// offset register pair for the virtual address to enter.
//
//      ITLB [.<opt>] <tlbInfoReg> "," "(" <segmentReg> "," <offsetReg> ")"
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrITLB( uint32_t *instr ) {
    
    if ( currentToken.typ == TT_GREG ) {
        
        setBitField( instr, 9, 4, currentToken.val );
        nextToken( );
    }
    
    if ( currentToken.typ == TT_COMMA ) nextToken( );
    else return( parserError((char *) "Expected a comma" ));
    
    if ( currentToken.typ == TT_LPAREN ) {
        
        nextToken( );
        if ( currentToken.typ == TT_SREG ) {
            
            setBitField( instr, 27, 4, currentToken.val );
            nextToken( );
            
            if ( currentToken.typ == TT_COMMA ) nextToken( );
            else return( parserError((char *) "Expected a comma" ));
        }
        else return( parserError((char *) "Expected a segment register" ));
        
        if ( currentToken.typ == TT_GREG ) {
            
            setBitField( instr, 31, 4, currentToken.val );
            nextToken( );
            
            if ( currentToken.typ == TT_RPAREN ) nextToken( );
            else return( parserError((char *) "Expected a right paren" ));
        }
    }
    else return( parserError((char *) "Expected a left paren" ));
    
    return( checkEOS( ));
}

//------------------------------------------------------------------------------------------------------------
// The "PTLB" instruction removes an entry from the instruction or data TLB. We use a logical address to
// refer to the TLB entry.
//
//      PTLB [ "." <opt> ] <targetReg> ","  <indexReg" "(" [ <segmentReg>, ] <offsetReg > ")"
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrPTLB( uint32_t *instr ) {
    
    if ( currentToken.typ == TT_GREG ) {
        
        setBitField( instr, 9, 4, currentToken.val );
        nextToken( );
    }
    
    if ( currentToken.typ == TT_COMMA ) nextToken( );
    else return( parserError((char *) "Expected a comma" ));
    
    if ( currentToken.typ == TT_GREG ) {
        
        setBitField( instr, 27, 4, currentToken.val );
        nextToken( );
    }
    else if ( currentToken.typ == TT_LPAREN ) {
        
        nextToken( );
        if ( currentToken.typ == TT_SREG ) {
            
            if ( isInRange( currentToken.val, 1, 3 )) {
                
                setBitField( instr, 13, 2, currentToken.val );
                nextToken( );
            }
            else return( parserError((char *) "Expected SR1 .. SR3 " ));
            
            if ( currentToken.typ == TT_COMMA ) nextToken( );
            else return( parserError((char *) "Expected a comma" ));
        }
        
        if ( currentToken.typ == TT_GREG ) {
            
            setBitField( instr, 31, 4, currentToken.val );
            nextToken( );
            
            if ( currentToken.typ == TT_RPAREN ) nextToken( );
            else return( parserError((char *) "Expected a right paren" ));
        }
    }
    else return( parserError((char *) "Expected an index Reg or left paren" ));
    
    return( checkEOS( ));
}

//------------------------------------------------------------------------------------------------------------
// The "PCA" instruction flushes and / or remove an entry from a data or instruction cache.
//
//      PCA [ "." <opt> ] <targetReg> ","  <ofs> "(" [ <segmentReg>, ] <offsetReg > ")"
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrPCA( uint32_t *instr ) {
    
    if ( currentToken.typ == TT_GREG ) {
        
        setBitField( instr, 9, 4, currentToken.val );
        nextToken( );
    }
    
    if ( currentToken.typ == TT_COMMA ) nextToken( );
    else return( parserError((char *) "Expected a comma" ));
    
    if ( currentToken.typ == TT_GREG ) {
        
        setBitField( instr, 27, 4, currentToken.val );
        nextToken( );
    }
    else if ( currentToken.typ == TT_LPAREN ) {
        
        nextToken( );
        if ( currentToken.typ == TT_SREG ) {
            
            if ( isInRange( currentToken.val, 1, 3 )) {
                
                setBitField( instr, 13, 2, currentToken.val );
                nextToken( );
            }
            else return( parserError((char *) "Invalid segment register" ));
            
            if ( currentToken.typ == TT_COMMA ) nextToken( );
            else return( parserError((char *) "Expected a comma" ));
        }
        
        if ( currentToken.typ == TT_GREG ) {
            
            setBitField( instr, 31, 4, currentToken.val );
            nextToken( );
            
            if ( currentToken.typ == TT_RPAREN ) nextToken( );
            else return( parserError((char *) "Expected a right paren" ));
        }
    }
    else return( parserError((char *) "Expected an index Reg or left paren" ));
    
    return( checkEOS( ));
}

//------------------------------------------------------------------------------------------------------------
// The "DIAG" instruction is the instruction for invoking special hardware or diagnostic functions.
//
//      DIAG <resultReg> "," <parmRegA> "," <parmRegB> "," <info>
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrDIAG( uint32_t *instr ) {
    
    Expr rExpr;
    
    if ( currentToken.typ == TT_GREG ) {
        
        setBitField( instr, 9, 4, currentToken.val );
        nextToken( );
    }
    
    if ( currentToken.typ == TT_COMMA ) nextToken( );
    else return( parserError((char *) "Expected a comma" ));
    
    if ( currentToken.typ == TT_GREG ) {
        
        setBitField( instr, 27, 4, currentToken.val );
        nextToken( );
    }
    else return( parserError((char *) "Expected a general register" ));
    
    if ( currentToken.typ == TT_COMMA ) nextToken( );
    else return( parserError((char *) "Expected a comma" ));
    
    if ( currentToken.typ == TT_GREG ) {
        
        setBitField( instr, 31, 4, currentToken.val );
        nextToken( );
    }
    else return( parserError((char *) "Expected a general register" ));
    
    if ( currentToken.typ == TT_COMMA ) nextToken( );
    else return( parserError((char *) "Expected a comma" ));
    
    if (( parseExpr( &rExpr )) && ( rExpr.typ == ET_NUM )) {
            
        if ( isInRangeForBitFieldU( rExpr.val1, 4 )) {
                
            setBitField( instr, 13, 4, rExpr.val1 );
            nextToken( );
        }
        else return( parserError((char *) "Immediate value out of range" ));
    }
    else return( parserError((char *) "Expected a number" ));
   
    return( checkEOS( ));
}

//------------------------------------------------------------------------------------------------------------
// The "RFI" instruction is the return from interrupt method. So far it is only the instruction with no
// further options and arguments.
//
//      RFI
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrRFI( uint32_t *instr ) {
    
    return( checkEOS( ));
}

//------------------------------------------------------------------------------------------------------------
// The "BRK" instruction will raise a trap passing along two info fields.
//
//      BRK <info1> "," <info2>
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrBRK( uint32_t *instr ) {
    
    Expr tExpr;
    
    parseExpr( &tExpr );
    if ( tExpr.typ == ET_NUM ) {
        
        if ( isInRangeForBitFieldU( tExpr.val1, 4 )) setImmValU( instr, 9, 4, tExpr.val1 );
        else return( parserError((char *) "Immediate value out of range" ));
    }
    else return( parserError((char *) "Expected the info1 parm" ));
    
    if ( currentToken.typ == TT_COMMA ) nextToken( );
    else return( parserError((char *) "Expected a comma" ));
    
    parseExpr( &tExpr );
    
    if ( tExpr.typ == TT_NUM ) {
        
        if ( isInRangeForBitFieldU( tExpr.val1, 16 )) setImmValU( instr, 31, 16, tExpr.val1 );
        else return( parserError((char *) "Immediate value out of range" ));
    }
    else return( parserError((char *) "Expected the info2 parm" ));
    
    return( checkEOS( ));
}

//------------------------------------------------------------------------------------------------------------
// "parseLine" will take the input string and parse the line for an instruction. In the simplified case, there
// is only the opCode mnemonic and the argument list. No labels, no comments. For each instruction, there is
// is a routine that parses the instruction specific input.
//
// An instruction starts with the opCode and the optional option qualifiers. For each opCode, the token table
// has an instruction template. Specials such as mapping the "LDx" instruction to "LDW" is already encoded
// in the template. The next step for all instructions is to check for options. Finally, a dedicated parsing
// routine will handle the remainder of the assembly line. As the parsing process comes along the instruction
// template from the token name table will be augmented with further data. If all is successful, we will have
// the final instruction bit pattern.
//
//------------------------------------------------------------------------------------------------------------
bool parseLine( char *inputStr, uint32_t *instr ) {
    
    setUpOneLineAssembler( inputStr, instr );
    
    nextToken( );
    if ( currentToken.typ == TT_OPCODE ) {
        
        *instr = currentToken.val;
        
        nextToken( );
        if ( currentToken.typ == TT_OPT ) {
            
            if ( ! parseInstrOptions( instr )) return( false );
            nextToken( );
        }
        
        switch ( getBitField( *instr, 5, 6 ) ) {
                
            case OP_ADD:
            case OP_ADC:
            case OP_SUB:
            case OP_SBC:
            case OP_AND:
            case OP_OR:
            case OP_XOR:
            case OP_CMP:
            case OP_CMPU:   return( parseModeTypeInstr( instr ));
                
            case OP_LD:
            case OP_LDA:
            case OP_LDR:    return( parseInstrLoad( instr ));
                
            case OP_ST:
            case OP_STA:
            case OP_STC:    return( parseInstrStore( instr ));
                
            case OP_LSID:   return( parseInstrLSID( instr ));
                
            case OP_EXTR:
            case OP_DEP:    return( parseInstrEXTRandDEP( instr ));
                
            case OP_DS:     return( parseInstrDS( instr ));
                
            case OP_DSR:    return( parseInstrDSR( instr ));
            case OP_SHLA:   return( parseInstrSHLA( instr ));
            case OP_CMR:    return( parseInstrCMR( instr ));
                
            case OP_LDIL:
            case OP_ADDIL:  return( parseInstrLDILandADDIL( instr ));
                
            case OP_LDO:    return( parseInstrLDO( instr ));
                
            case OP_B:
            case OP_GATE:   return( parseInstrBandGATE( instr ));
                
            case OP_BR:     return( parseInstrBR( instr ));
            case OP_BV:     return( parseInstrBV( instr ));
            case OP_BE:     return( parseInstrBE( instr ));
            case OP_BVE:    return( parseInstrBVE( instr ));
                
            case OP_CBR:
            case OP_CBRU:   return( parseInstrCBRandCBRU( instr ));
                
            case OP_MR:     return( parseInstrMR( instr ));
            case OP_MST:    return( parseInstrMST( instr ));
            case OP_LDPA:   return( parseInstrLDPA( instr ));
            case OP_PRB:    return( parseInstrPRB( instr ));
            case OP_ITLB:   return( parseInstrITLB( instr ));
            case OP_PTLB:   return( parseInstrPTLB( instr ));
            case OP_PCA:    return( parseInstrPCA( instr ));
            case OP_DIAG:   return( parseInstrDIAG( instr ));
            case OP_RFI:    return( parseInstrRFI( instr ));
            case OP_BRK:    return( parseInstrBRK( instr ));
                
            default:    return( parserError((char *) "Invalid opcode" ));
        }
    }
    else return( parserError((char *) "Expected an opcode" ));
}

} // namespace

//------------------------------------------------------------------------------------------------------------
// A simple one line assembler. This object is the counterpart to the disassembler. We will parse a one line
// input string for a valid instruction, using the syntax of the real assembler. There will be no labels and
// comments, only the opcode and the operands.
//
//------------------------------------------------------------------------------------------------------------
DrvOneLineAsm::DrvOneLineAsm( VCPU32Globals *glb ) {
    
    this -> glb = glb;
};

bool DrvOneLineAsm::parseAsmLine( char *inputStr, uint32_t *instr ) {
    
    return ( parseLine( inputStr, instr ));
}
