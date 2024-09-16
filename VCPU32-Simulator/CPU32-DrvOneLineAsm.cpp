//------------------------------------------------------------------------------------------------------------
//
// VCPU32 - A 32-bit CPU - Dissaembler
//
//------------------------------------------------------------------------------------------------------------
// The one line assembler assembles an instruction without further context. It is intended to for testing
// instructons in teh simulator. There is no symbol table or any concept of assembling multiple instructions.
// The instruction to generate test is completely self sufficient. The parser is a straightforward recursive
// descendant parser, LL1 grammar.
//
//------------------------------------------------------------------------------------------------------------
//
// VCPU32 - A 32-bit CPU - Dissaembler
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
// Local namespace. These routines are not visible otside this source file.
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
    
    TT_COMMA    = 10,
    TT_PERIOD   = 11,
    TT_LPAREN   = 12,
    TT_RPAREN   = 13,
    TT_PERCENT  = 14,
    
    TT_ERR      = 20,
    TT_EOS      = 21
};

//------------------------------------------------------------------------------------------------------------
// A token. A token has a name, a tye and a value. For the instructions, the value represents the instruction
// tempate for the respective instruction. We also already set the data word width and any other predefied
// bits. The parsing routines will augment this tempalte be setting the remaing fields. The token table is
// just a list of tokens, which is searched in a linear fashion.
//
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
    
    { "ADD",    TT_OPCODE,  0x40020000  },
    { "ADDB",   TT_OPCODE,  0x40000000  },
    { "ADDH",   TT_OPCODE,  0x40010000  },
    { "ADDW",   TT_OPCODE,  0x40020000  },
    
    { "ADC",    TT_OPCODE,  0x44020000  },
    { "ADCB",   TT_OPCODE,  0x44000000  },
    { "ADCH",   TT_OPCODE,  0x44010000  },
    { "ADCW",   TT_OPCODE,  0x44020000  },
    
    { "SUB",    TT_OPCODE,  0x48020000  },
    { "SUBB",   TT_OPCODE,  0x48000000  },
    { "SUBH",   TT_OPCODE,  0x48010000  },
    { "SUBW",   TT_OPCODE,  0x48020000  },
    
    { "SBC",    TT_OPCODE,  0x4C020000  },
    { "SBCB",   TT_OPCODE,  0x4C000000  },
    { "SBCH",   TT_OPCODE,  0x4C010000  },
    { "SBCW",   TT_OPCODE,  0x4C020000  },
    
    { "AND",    TT_OPCODE,  0x50020000  },
    { "ANDB",   TT_OPCODE,  0x50000000  },
    { "ANDH",   TT_OPCODE,  0x50010000  },
    { "ANDW",   TT_OPCODE,  0x50020000  },
    
    { "OR" ,    TT_OPCODE,  0x54020000  },
    { "ORB",    TT_OPCODE,  0x54000000  },
    { "ORH",    TT_OPCODE,  0x54010000  },
    { "ORW",    TT_OPCODE,  0x54020000  },
    
    { "XOR" ,   TT_OPCODE,  0x58020000  },
    { "XORB",   TT_OPCODE,  0x58000000  },
    { "XORH",   TT_OPCODE,  0x58010000  },
    { "XORW",   TT_OPCODE,  0x58020000  },
    
    { "CMP" ,   TT_OPCODE,  0x5C020000  },
    { "CMPB",   TT_OPCODE,  0x5C000000  },
    { "CMPH",   TT_OPCODE,  0x5C010000  },
    { "CMPW",   TT_OPCODE,  0x5C020000  },
    
    { "CMPU" ,  TT_OPCODE,  0x60020000  },
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
// The current parser state.
//
//------------------------------------------------------------------------------------------------------------
char        tokenLine[ TOK_INPUT_LINE_SIZE ];
int         currentLineLen;
int         currentCharIndex;
int         currentTokIndex;
char        currentChar;
Token       currentToken;

//------------------------------------------------------------------------------------------------------------
// Instruction decoding means to get to bits and bit fields. Here is a set of helper functions.
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
// "nextChar" returns the next character from the token line string. "puBackChar" is necessary to set the
// character index back one character. We need this for cases where we look two characters ahead and need
// to go back one character once we know what we are looking at.
//
//------------------------------------------------------------------------------------------------------------
void nextChar( ) {
    
    if ( currentCharIndex < currentLineLen ) {
        
        currentChar = tokenLine[ currentCharIndex ];
        currentCharIndex ++;
    }
    else currentChar = EOS_CHAR;
}

void putBackChar( ) {
    
    if ( currentCharIndex > 0 ) {
        
        currentCharIndex --;
        
        if ( currentCharIndex > 0 ) {
            
            currentChar = tokenLine[ currentCharIndex - 1];
        }
        else currentChar = ' ';
    }
    else currentChar = ' ';
}

//------------------------------------------------------------------------------------------------------------
// "parserError" is a little helper that prints out the error encountered. We will print the original
// input line, a caret marker where we found the error, and then return a false.
//
//------------------------------------------------------------------------------------------------------------
bool parserError( char *errStr ) {
    
    fprintf( stdout, "%s\n", tokenLine );
    
    int i = 0;
    while ( i < currentTokIndex ) {
        
        fprintf( stdout, " " );
        i ++;
    }
    
    fprintf( stdout, "^\n" );
    
    fprintf( stdout, "%s\n", errStr );
    return( false );
}

//------------------------------------------------------------------------------------------------------------
// Check that the ASM line does not contain any extra tokens.
//
//------------------------------------------------------------------------------------------------------------
bool checkEOS( ) {
    
    if ( currentToken.typ != TT_EOS ) {
        
        return( parserError((char *) "Extra tokens in the ASDM line" ));
    }
    else return( true );
}

//------------------------------------------------------------------------------------------------------------
// "getNum" will parse a number. We leave the heavy lifting to the C library.
//
//------------------------------------------------------------------------------------------------------------
void parseNum( int sign ) {
    
    int  index = 0;
    int  tmpRes = 0;
    char tmpStr[ TOK_INPUT_LINE_SIZE ];
    
    do {
        
        tmpStr[ index++ ] = currentChar;
        nextChar( );
        
    } while ( isxdigit( currentChar ) ||
             ( currentChar == 'X' )   || ( currentChar == 'x' ) ||
             ( currentChar == 'O' )   || ( currentChar == 'o' ));
    
    tmpStr[ index ] = 0;
    
    if ( sscanf( tmpStr, "%i", &tmpRes ) == 1 ) {
        
        currentToken.typ    = TT_NUM;
        currentToken.val   = tmpRes * sign;
    }
    else parserError((char *) "Invalid number" );
}

//------------------------------------------------------------------------------------------------------------
// "getIdent" parses an identifier. It is a sequence of characters starting with an alpha character. We do
// not really have user defined identifiers, only reserved words. However, the concept of a non-reserved
// identifier is used to implement the opCode option string, which is just a list of characters. The parsing
// of an identfier also needs to manage the parsing of constants with a qualifier, such as "L%nnn". We first
// check for these kind of qualifieres and if found hand over to parse a number.
//
//------------------------------------------------------------------------------------------------------------
void parseIdent( ) {
    
    if ( currentChar == 'L' ) {
        
        nextChar( );
        if ( currentChar == '%' ) {
            
            nextChar( );
            if ( isdigit( currentChar )) {
                
                parseNum( 1 );
                currentToken.val &= 0xFFFFFC00;
                return;
            }
            else parserError((char *) "Invalid char in identifier" );
        }
        else {
            
            currentToken.name[ 0 ] = 'L';
            currentToken.name[ 1 ] = currentChar;
        }
    }
    else if ( currentChar == 'R' ) {
        
        nextChar( );
        if ( currentChar == '%' ) {
            
            nextChar( );
            if ( isdigit( currentChar )) {
                
                parseNum( 1 );
                currentToken.val &= 0x3FF;
                return;
            }
            else parserError((char *) "Invalid char in identifier" );
        }
        else {
            
            currentToken.name[ 0 ] = 'R';
            currentToken.name[ 1 ] = currentChar;
        }
    }
    
    int tokStrIndex = 2;
    
    nextChar( );
    while ( isalnum( currentChar )) {
        
        currentToken.name[ tokStrIndex++ ] = currentChar;
        nextChar( );
    }
    
    currentToken.name[ tokStrIndex ] = 0;
    
    int currentTokenIndex = lookupToken( currentToken.name );
    
    if ( currentTokenIndex != 0 ) {
        
        currentToken = tokNameTab[ currentTokenIndex ];
    }
    else {
        
        currentToken.typ     = TT_IDENT;
        currentToken.val    = 0;
    }
}

//------------------------------------------------------------------------------------------------------------
// "nextToken" provides the next token form the input source line. All information about the token parsed
// will be stored in the global "currentXXX" variables.
//
// There is one tricky part. The "L%xxxxx" and alike numeric values do not start with a digit. We need to
// scan the "L" character followed by an "%" character followed by the numeric value. When there is not
// a "%" it must be treated as an identifier and put back the last character, so we can call "getIdent" to
// analyze the identifier. When there is not a numeric value following the "L%" sequeunce, it is an error.
//
// ??? maybe there is a more struczured way to deal with the "L%" type qualifiers.... for now it will do.
//------------------------------------------------------------------------------------------------------------
void nextToken( ) {
    
    currentToken.name[ 0 ]  = 0;;
    currentToken.typ        = TT_EOS;
    currentToken.val        = 0;
    
    while (( currentChar == ' ' ) || ( currentChar == '\n' ) || ( currentChar == '\n' )) nextChar( );
    
    currentTokIndex = currentCharIndex - 1;
    
    if ( isalpha( currentChar )) {
        
        parseIdent( );
    }
    else if ( isdigit( currentChar )) {
        
        parseNum( 1 );
    }
    else if ( currentChar == '-' ) {
        
        nextChar( );
        parseNum( -1 );
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
    else if ( currentChar == '.' ) {
        
        currentToken.typ = TT_PERIOD;
        nextChar( );
    }
    else currentToken.typ = TT_ERR;
}

//------------------------------------------------------------------------------------------------------------
// The one line assembler starts with a setup of the global data. The input is upshifted in this module.
//
//------------------------------------------------------------------------------------------------------------
void setUpOneLineAssembler( char *inputStr, uint32_t *instr ) {
    
    strncpy( tokenLine, inputStr, strlen( inputStr ) + 1 );
    upshiftStr( tokenLine );
    
    *instr              = 0;
    currentLineLen      = (int) strlen( tokenLine );
    currentCharIndex    = 0;
    currentTokIndex     = 0;
    currentChar         = ' ';
}

//------------------------------------------------------------------------------------------------------------
// "parseInstrOptions" will analyze the opCode option string. An opCode string looks just like an identifier
// to the one line parser. We will look at each character in the "name" and set the options for the particular
// instruction. One day we may have options that have more than one period/option pair. And an option may have
// a name that has more than one character. To be defined...
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrOptions( uint32_t *instr ) {
    
    nextToken( );
    if ( currentToken.typ == TT_IDENT ) {
        
        switch( getBitField( *instr, 5, 6 )) {
                
            case OP_LD:     case OP_ST:  case OP_LDA:     case OP_STA:  {
                
                if ( currentToken.name[ 0 ] == 'M' ) setBit( instr, 11 );
                else return( parserError((char *) "Invalid instruction option" ));
                
            } break;
                
            case OP_ADD:    case OP_ADC:    case OP_SUB:    case OP_SBC: {
                
                for ( int i = 0; i < strlen( currentToken.name ); i ++ ) {
                    
                    if      ( currentToken.name[ i ] == 'L' ) setBit( instr, 10 );
                    else if ( currentToken.name[ i ] == 'O' ) setBit( instr, 11 );
                    else return( parserError((char *) "Invalid instruction option" ));
                }
                
            } break;
                
            case OP_AND:    case OP_OR: {
                
                for ( int i = 0; i < strlen( currentToken.name ); i ++ ) {
                    
                    if      ( currentToken.name[ i ] == 'N' ) setBit( instr, 10 );
                    else if ( currentToken.name[ i ] == 'C' ) setBit( instr, 11 );
                    else return( parserError((char *) "Invalid instruction option" ));
                }
                
            } break;
                
            case OP_XOR: {
                
                if      ( currentToken.name[ 0 ] == 'N' ) setBit( instr, 10 );
                else return( parserError((char *) "Invalid instruction option" ));
                
            } break;
                
            case OP_CMP:    case OP_CMPU: {
                
                if ( strcmp( currentToken.name, ((char *) "EQ" ))) setBitField( instr, 11, 2, 0 );
                if ( strcmp( currentToken.name, ((char *) "LT" ))) setBitField( instr, 11, 2, 1 );
                if ( strcmp( currentToken.name, ((char *) "NE" ))) setBitField( instr, 11, 2, 2 );
                if ( strcmp( currentToken.name, ((char *) "LE" ))) setBitField( instr, 11, 2, 3 );
                
            } break;
                
            case OP_EXTR: {
                
                for ( int i = 0; i < strlen( currentToken.name ); i ++ ) {
                    
                    if      ( currentToken.name[ i ] == 'S' ) setBit( instr, 10 );
                    else if ( currentToken.name[ i ] == 'A' ) setBit( instr, 11 );
                    else return( parserError((char *) "Invalid instruction option" ));
                }
                
            } break;
                
            case OP_DEP: {
                
                for ( int i = 0; i < strlen( currentToken.name ); i ++ ) {
                    
                    if      ( currentToken.name[ i ] == 'Z' ) setBit( instr, 10 );
                    else if ( currentToken.name[ i ] == 'A' ) setBit( instr, 11 );
                    else if ( currentToken.name[ i ] == 'I' ) setBit( instr, 12 );
                    else return( parserError((char *) "Invalid instruction option" ));
                }
                
            } break;
                
            case OP_DSR: {
                
                if      ( currentToken.name[ 0 ] == 'A' ) setBit( instr, 11 );
                else return( parserError((char *) "Invalid instruction option" ));
                
            } break;
                
            case OP_SHLA: {
                
                for ( int i = 0; i < strlen( currentToken.name ); i ++ ) {
                    
                    if      ( currentToken.name[ i ] == 'I' ) setBit( instr, 10 );
                    else if ( currentToken.name[ i ] == 'L' ) setBit( instr, 11 );
                    else if ( currentToken.name[ i ] == 'O' ) setBit( instr, 12 );
                    else return( parserError((char *) "Invalid instruction option" ));
                }
                
            } break;
                
            case OP_MR: {
                
                for ( int i = 0; i < strlen( currentToken.name ); i ++ ) {
                    
                    if      ( currentToken.name[ i ] == 'D' ) setBit( instr, 10 );
                    else if ( currentToken.name[ i ] == 'M' ) setBit( instr, 11 );
                    else return( parserError((char *) "Invalid instruction option" ));
                }
                
            } break;
                
            case OP_PRB: {
                
                for ( int i = 0; i < strlen( currentToken.name ); i ++ ) {
                    
                    if      ( currentToken.name[ i ] == 'W' ) setBit( instr, 10 );
                    else if ( currentToken.name[ i ] == 'I' ) setBit( instr, 11 );
                    else return( parserError((char *) "Invalid instruction option" ));
                }
                
            } break;
                
            case OP_ITLB: {
                
                if      ( currentToken.name[ 0 ] == 'T' ) setBit( instr, 11 );
                else return( parserError((char *) "Invalid instruction option" ));
                
            } break;
                
            case OP_PTLB: {
                
                for ( int i = 0; i < strlen( currentToken.name ); i ++ ) {
                    
                    if      ( currentToken.name[ i ] == 'T' ) setBit( instr, 10 );
                    else if ( currentToken.name[ i ] == 'M' ) setBit( instr, 11 );
                    else return( parserError((char *) "Invalid instruction option" ));
                }
                
            } break;
                
            case OP_PCA: {
                
                for ( int i = 0; i < strlen( currentToken.name ); i ++ ) {
                    
                    if      ( currentToken.name[ i ] == 'T' ) setBit( instr, 10 );
                    else if ( currentToken.name[ i ] == 'M' ) setBit( instr, 11 );
                    else if ( currentToken.name[ i ] == 'F' ) setBit( instr, 14 );
                    else return( parserError((char *) "Invalid instruction option" ));
                }
                
            } break;
                
            default: return( parserError((char *) "Instruction has no option" ));
        }
    }
    else return( parserError((char *) "Expected the option qualifiers" ));
    
    return( true );
}

//------------------------------------------------------------------------------------------------------------
// "parseModeTypeInstr" parses all instructions that have an "operand" encoding. We enter the routine with
// the OpCode and options parsed. The mode type instruction parsing starts with the target register spot in
// the command line.
//
//      opCode [ "." opt ] <targetReg> "," <num>                                - mode 0
//      opCode [ "." opt ] <targetReg> "," <num> "(" <baseReg> ")"              - mode 3
//      opCode [ "." opt ] <targetReg> ","<sourceReg>                           - mode 1
//      opCode [ "." opt ] <targetReg> "," <sourceRegA> "," "<sourceRegB>       - mode 1
//      opCode [ "." opt ] <targetReg> "," <indexReg> "(" <baseReg> ")"         - mode 2
//
//
// There is one ugly thing. The "dw" field is only applicable to the mode 2 and 3 type instructions. Although
// we set the data width option correctly in the instruction template, a "xxxB" byte type instrction will
// set the "dw" field to "zero" as do the mode 0 and 1 type instructions. We cannot use the "dw" field for
// checking that the correct instruction mnemonic for mode 0 and 1, i.e. "ADD", is used. An "ADDB" would
// look the same.
//
//------------------------------------------------------------------------------------------------------------
bool parseModeTypeInstr( uint32_t *instr ) {
    
    uint8_t targetRegId = 0;
    
    if ( currentToken.typ == TT_GREG ) {
        
        setBitField( instr, 9, 4, currentToken.val );
        targetRegId = currentToken.val;
        nextToken( );
    }
    else return( parserError((char *) "Expected a general register" ));
    
    if ( currentToken.typ == TT_COMMA ) nextToken( );
    else return( parserError((char *) "Expected a Comma" ));
    
    if ( currentToken.typ == TT_NUM ) {
        
        uint32_t tokVal = currentToken.val;
        
        nextToken( );
        if ( currentToken.typ == TT_LPAREN ) {
            
            setImmVal( instr, 31, 12, tokVal );
            
            nextToken( );
            if ( currentToken.typ == TT_GREG ) {
                
                setBitField( instr, 13, 2, 3 );
                setBitField( instr, 31, 4, currentToken.val );
                
                nextToken( );
                checkEOS( );
            }
            else return( parserError((char *) "Expected a general reg" ));
        }
        else if ( currentToken.typ == TT_EOS ) {
            
            setBitField( instr, 13, 2, 0 );
            setImmVal( instr, 31, 18, tokVal );
            
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
            setBitField( instr, 27, 4,targetRegId );
            setBitField( instr, 31, 4,operandRegId );
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
        
        return( parserError((char *) "Invalid opCode option" ));
    }
    
    return( true );
}

//------------------------------------------------------------------------------------------------------------
// "parseInstrLSID" parses the LSID instruction.
//
//      <opCode> [ "." <opt> ] <targetReg> "," <sourceReg>
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
        
        if ( getBit( *instr, 11 )) {
            
            setBitField( instr, 21, 5, currentToken.val );
        }
        else setBitField( instr, 27, 5, currentToken.val );
        
        nextToken( );
    }
    else return( parserError((char *) "Expected a Comma" ));
    
    if ( ! getBit( *instr, 11 )) {
        
        if ( currentToken.typ == TT_COMMA ) nextToken( );
        else return( parserError((char *) "Expected a Comma" ));
        
        nextToken( );
        if ( currentToken.typ == TT_NUM ) {
            
            setBitField( instr, 27, 5, currentToken.val );
            nextToken( );
        }
        else return( parserError((char *) "Expected a nuber" ));
        
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
            
            setBitField( instr, 21, 5, currentToken.val );
            nextToken( );
        }
        else return( parserError((char *) "Expected a nuber" ));
    }

    return( checkEOS( ));
}

//------------------------------------------------------------------------------------------------------------
// The SHLA instructionperforms a shift left of "B" by "sa" and adds the "A" register to it. Th result will
// go into the target register.
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
        
        setBitField( instr, 21, 2, currentToken.val );
        nextToken( );
    }
    else return( parserError((char *) "Expected the shift amount" ));
    
    return( checkEOS( ));
}

//------------------------------------------------------------------------------------------------------------
// The CMR instruction tests register "B" for a condition and if true copies the "A" value to "R".
//
//      CMR "." <cond> <targetReg> "," <regA> "," <regB>
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
// "ADDIL" instruction will add the value encoded in the instruction left shifted to "R". The resukt is
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
        
        setBitField( instr, 31, 22, currentToken.val );
        nextToken( );
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
        
        setBitField( instr, 27, 18, currentToken.val );
        
        if ( currentToken.typ == TT_LPAREN ) nextToken( );
        else return( parserError((char *) "Expected a Lparen" ));
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
//
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrBandGATE( uint32_t *instr ) {
    
    if ( currentToken.typ == TT_GREG ) {
        
        setBitField( instr, 9, 4, currentToken.val );
        nextToken( );
        
        if ( currentToken.typ == TT_COMMA ) nextToken( );
        else return( parserError((char *) "Expected a Comma" ));
    }
    
    if ( currentToken.typ == TT_NUM ) {
        
        setBitField( instr, 31, 22, currentToken.val );
    }
    
    return( checkEOS( ));
}

//------------------------------------------------------------------------------------------------------------
//
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrBRandBV( uint32_t *instr ) {
    
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
//
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrBE( uint32_t *instr ) {
    
    
    
    
    return( checkEOS( ));
}

//------------------------------------------------------------------------------------------------------------
//
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrBVE( uint32_t *instr ) {
    
    
    
    return( checkEOS( ));
}

//------------------------------------------------------------------------------------------------------------
//
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrCBR( uint32_t *instr ) {
    
    
    
    return( checkEOS( ));
}

//------------------------------------------------------------------------------------------------------------
//
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrCBRU( uint32_t *instr ) {
    
    
    
    return( checkEOS( ));
}

//------------------------------------------------------------------------------------------------------------
//
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrMR( uint32_t *instr ) {
    
    return( checkEOS( ));
}

//------------------------------------------------------------------------------------------------------------
//
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrMST( uint32_t *instr ) {
    
    return( checkEOS( ));
}

//------------------------------------------------------------------------------------------------------------
//
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrLDPA( uint32_t *instr ) {
    
    return( checkEOS( ));
}

//------------------------------------------------------------------------------------------------------------
//
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrPRB( uint32_t *instr ) {
    
    return( checkEOS( ));
}

//------------------------------------------------------------------------------------------------------------
//
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrITLB( uint32_t *instr ) {
    
    return( checkEOS( ));
}

//------------------------------------------------------------------------------------------------------------
//
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrPTLB( uint32_t *instr ) {
    
    return( checkEOS( ));
}

//------------------------------------------------------------------------------------------------------------
//
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrPCA( uint32_t *instr ) {
    
    return( checkEOS( ));
}

//------------------------------------------------------------------------------------------------------------
//
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrDIAG( uint32_t *instr ) {
    
    return( checkEOS( ));
}

//------------------------------------------------------------------------------------------------------------
//
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrRFI( uint32_t *instr ) {
    
    return( checkEOS( ));
}

//------------------------------------------------------------------------------------------------------------
//
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrBRK( uint32_t *instr ) {
    
    return( checkEOS( ));
}

//------------------------------------------------------------------------------------------------------------
// "parseLoadStoreOperand" parses the operand portion of the load and store instruction family. The syntax
// for the <operand> portion is either a
//
//      <ofs> "(" SR "," GR ")"
//      <ofs> "(" GR ")"
//      "(" SR "," GR ")"
//      "(" GR ")"
//
// <loadInstr>  [ "." <opt> ] <targetReg>       "," <sourceOperand>
// <storeInstr> [ "." <opt> ] <targetOperand>   "," <sourceReg>
//
// We expect the instruction template with opCode, opCode options and word length already set. The routine
// is called for load, where the operand is the source, and for store where the operand is the target.
//
//------------------------------------------------------------------------------------------------------------
bool parseLoadStoreOperand( uint32_t *instr ) {
    
    if ( currentToken.typ == TT_NUM ) {
        
        if (((int32_t) currentToken.val >= -2048 ) && ((int32_t) currentToken.val <= 2047 )) {
            
            setImmVal( instr, 27, 12, currentToken.val );
            nextToken( );
        }
        else parserError((char *) "Offset value out of range" );
    }
    else if ( currentToken.typ == TT_GREG ) {
        
        setBit( instr, 10 );
        setBitField( instr, 27, 4, currentToken.val );
        nextToken( );
    }
    else if ( currentToken.typ == TT_LPAREN ) {
        
        setBitField( instr, 27, 12, 0 );
    }
    else return( parserError((char *) "Expected the operand" ));
    
    if ( currentToken.typ == TT_LPAREN ) {
        
        nextToken( );
        if ( currentToken.typ == TT_SREG ) {
            
            if (( getBitField( *instr, 5, 6 ) == OP_LDA ) || ( getBitField( *instr, 5, 6 ) == OP_STA )) {
                
                return( parserError((char *) "Invalid address for instruction: seg:ofs" ));
            }
            
            if (( currentToken.val >= 1 ) && ( currentToken.val <= 3 )) {
                
                setBitField( instr, 13, 2, currentToken.val );
            }
            else return( parserError((char *) "Expected SR1 .. SR3 " ));
            
            nextToken( );
            if ( currentToken.typ == TT_COMMA ) {
                
                nextToken( );
            }
            else return( parserError((char *) "Expected a comma" ));
            
            if ( currentToken.typ == TT_GREG ) {
                
                setBitField( instr, 31, 4, currentToken.val );
                
                nextToken( );
                if ( currentToken.typ == TT_RPAREN) nextToken( );
                else return( parserError((char *) "Expected a right paren" ));
            }
            else return( parserError((char *) "Expected a general reg" ));
        }
        else if ( currentToken.typ == TT_GREG ) {
            
            setBitField( instr, 13, 2, 0 );
            setBitField( instr, 31, 4, currentToken.val );
            
            nextToken( );
            if ( currentToken.typ == TT_RPAREN ) return( true );
            else return( parserError((char *) "Expected a right paren" ));
        }
        else return( parserError((char *) "Expected a general or segment register" ));
    }
    else return( parserError((char *) "Expected a left paren" ));
    
    return( true );
}

//------------------------------------------------------------------------------------------------------------
// "parseInstrLoad" will parse the load instructions family. The workhorse is the "parseLoadStoreOperand"
// routine, which parses the operand. General form:
//
//  <opCode>.<opt> <targetReg>, <sourceOperand>
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrLoad( uint32_t *instr ) {
    
    if ( currentToken.typ == TT_GREG ) {
        
        setBitField( instr, 9, 4, currentToken.val );
        nextToken( );
    }
    else return( parserError((char *) "Expected a general register" ));
    
    if ( currentToken.typ == TT_COMMA ) {
        
        nextToken( );
        return( parseLoadStoreOperand( instr ));
    }
    else return( parserError((char *) "Expected a comma" ));
}

//------------------------------------------------------------------------------------------------------------
// "parseInstrSTx" will parse the store instructionfamily. The workhorse is the "parseLoadStoreOperand"
// routine, which parses the target. General form:
//
//  <opCode>.<opt> <targetOperand>, <sourceReg>
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrStore( uint32_t *instr ) {
    
    if ( parseLoadStoreOperand( instr )) {
        
        if ( currentToken.typ == TT_COMMA ) {
            
            nextToken( );
            if ( currentToken.typ == TT_GREG ) {
                
                setBitField( instr, 9, 4, currentToken.val );
                return( checkEOS( ));
            }
            else return( parserError((char *) "Expected a general register" ));
        }
        else return( parserError((char *) "Expected a comma" ));
    }
    else return( false );
}

//------------------------------------------------------------------------------------------------------------
// "parseLine" will take the input string and parse the line for an instruction. In the simplified case, there
// is only the opCode mnemonic and the argument list. No labels, no comments. For each instruction, there is
// is a routine that parses the instruction specific input.
//
// An instruction starts with the opCode and the optional option qualifiers. For each opCode, the token table
// has an instruction template. Specials such as mapping the "LDx" instruction to "LDW" is already encoded
// in the tamplate. The next step for all instructions is to check for options. Finally, a dedicated parsing
// routine will handle the remainder of the assembly line.
//
// As the parsing process comes along the instruction template from the token name table will be augmented
// with further data. If all is successful, we will have the final instruction bit pattern.
//
//------------------------------------------------------------------------------------------------------------
bool parseLine( char *inputStr, uint32_t *instr ) {
    
    setUpOneLineAssembler( inputStr, instr );
    
    nextToken( );
    if ( currentToken.typ == TT_OPCODE ) {
        
        *instr = currentToken.val;
        
        nextToken( );
        if ( currentToken.typ == TT_PERIOD ) {
            
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
                
            case OP_DSR:    return( parseInstrDSR( instr ));
            case OP_SHLA:   return( parseInstrSHLA( instr ));
            case OP_CMR:    return( parseInstrCMR( instr ));
                
            case OP_LDIL:
            case OP_ADDIL:  return( parseInstrLDILandADDIL( instr ));
                
            case OP_LDO:    return( parseInstrLDO( instr ));
                
            case OP_B:
            case OP_GATE:   return( parseInstrBandGATE( instr ));
                
            case OP_BR:
            case OP_BV:     return( parseInstrBRandBV( instr ));
                
            case OP_BE:     return( parseInstrBE( instr ));
            case OP_BVE:    return( parseInstrBVE( instr ));
            case OP_CBR:    return( parseInstrCBR( instr ));
            case OP_CBRU:   return( parseInstrCBRU( instr ));
                
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
// A simple one line assembler. This object is teh counter part to the disassmbler. We will parse a one
// line input string for a valid instruction, using the syntax of the real assembler. There will be no
// labels and commants, only the opcode and the operands.
//
//------------------------------------------------------------------------------------------------------------
DrvOneLineAsm::DrvOneLineAsm( VCPU32Globals *glb ) {
    
    this -> glb = glb;
};

bool DrvOneLineAsm::parseAsmLine( char *inputStr, uint32_t *instr ) {
    
    return ( parseLine( inputStr, instr ));
}
