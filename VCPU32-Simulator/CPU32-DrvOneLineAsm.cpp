//------------------------------------------------------------------------------------------------------------
//
// VCPU32 - A 32-bit CPU - Dissaembler
//
//------------------------------------------------------------------------------------------------------------
// The one line assembler assembles an instruction without further context. It is intended to for testing
// instructons in teh simulator. There is no symbol table or any concept of assembling multiple instructions.
// The instruction to generate test is completely self sufficient.
//
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
    
    TT_NIL,
    TT_OPCODE,
    TT_GREG,
    TT_SREG,
    TT_CREG,
    TT_NUM,
    TT_COMMA,
    TT_PERIOD,
    TT_LPAREN,
    TT_RPAREN,
    TT_PERCENT,
    TT_IDENT,
    TT_ERR,
    TT_EOS
};

//------------------------------------------------------------------------------------------------------------
// A token. A token has a name, a tye and a value.
//
//------------------------------------------------------------------------------------------------------------
struct Token {
    
    char        tokName[ TOK_NAME_SIZE ];
    uint8_t     tokType;
    uint32_t    tokValue;
};

const Token tokNameTab[ ] = {
    
    { "NIL",    TT_NIL,     0           },
    { "LDB",    TT_OPCODE,  0xC0000000  },
    { "LDH",    TT_OPCODE,  0xC0010000  },
    { "LDW",    TT_OPCODE,  0xC0020000  },
    { "LDA",    TT_OPCODE,  0xC8020000  },
    
    { "STB",    TT_OPCODE,  0xC4200000  },
    { "STH",    TT_OPCODE,  0xC4210000  },
    { "STW",    TT_OPCODE,  0xC4220000  },
    { "STA",    TT_OPCODE,  0xCC220000  },
    
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

void upshiftStr( char *str ) {
    
    size_t len = strlen( str );
    
    if ( len > 0 ) {
        
        for ( size_t i = 0; i < len; i++ ) str[ i ] = (char) toupper((int) str[ i ] );
    }
}

uint8_t lookupToken( char *str ) {
    
    if (( strlen( str ) == 0 ) || ( strlen ( str ) > TOK_NAME_SIZE )) return( 0 );
    
    for ( int i = 0; i < TOK_NAME_TAB_SIZE; i++ ) {
        
        if ( strcmp( str, tokNameTab[ i ].tokName ) == 0 ) return( i );
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
// "getIdent" parses an identifier. It is a sequence of characters starting with an alpha character. We do
// not really have use defined identifiers, only reserved words. However, the concept of a non-reserved
// identifier is used to implement the opCode option string, which is just a list of characters.
//
// ??? what do we do about "L%xxxx" ?
//------------------------------------------------------------------------------------------------------------
void getIdent( ) {
    
    int tokStrIndex = 0;
    
    while ( isalnum( currentChar )) {
        
        currentToken.tokName[ tokStrIndex++ ] = currentChar;
        nextChar( );
    }
    
    currentToken.tokName[ tokStrIndex ] = 0;
    
    int currentTokenIndex = lookupToken( currentToken.tokName );
    
    if ( currentTokenIndex != 0 ) {
        
        currentToken = tokNameTab[ currentTokenIndex ];
    }
    else {
        
        currentToken.tokType     = TT_IDENT;
        currentToken.tokValue    = 0;
    }
}

//------------------------------------------------------------------------------------------------------------
// "getNum" will parse a number. We leave the heavy lifting to the C library.
//
// ??? maybe a bit too simple... unsigned 32-bits how ?
//------------------------------------------------------------------------------------------------------------
void getNum( int sign ) {
    
    int  index = 0;
    int  tmpRes = 0;
    char tmpStr[ TOK_INPUT_LINE_SIZE ];
    
    do {
        
        tmpStr[ index++ ] = currentChar;
        nextChar( );
    
    } while ( isxdigit( currentChar ) || ( currentChar == 'x' ) || ( currentChar == 'o' ));
    
    tmpStr[ index ] = 0;
    
    if ( sscanf( tmpStr, "%d", &tmpRes ) == 1 ) {
        
        currentToken.tokType    = TT_NUM;
        currentToken.tokValue   = tmpRes * sign;
    }
    else parserError((char *) "Invalid number" );
}

//------------------------------------------------------------------------------------------------------------
// "nextToken" provides the next token form the input source line. All information about the token parsed
// will be stored in the global "currentXXX" variables.
//
//------------------------------------------------------------------------------------------------------------
void nextToken( ) {
    
    currentToken.tokName[ 0 ]   = 0;;
    currentToken.tokType        = TT_EOS;
    currentToken.tokValue       = 0;
    
    while (( currentChar == ' ' ) || ( currentChar == '\n' ) || ( currentChar == '\n' )) nextChar( );
    
    currentTokIndex = currentCharIndex - 1;
    
    if ( isalpha( currentChar )) {
        
        getIdent( );
    }
    else if ( isdigit( currentChar )) {
        
        getNum( 1 );
    }
    else if ( currentChar == '-' ) {
        
        nextChar( );
        getNum( -1 );
    }
    else if ( currentChar == '(' ) {
        
        currentToken.tokType = TT_LPAREN;
        nextChar( );
    }
    else if ( currentChar == ')' ) {
        
        currentToken.tokType = TT_RPAREN;
        nextChar( );
    }
    else if ( currentChar == ',' ) {
        
        currentToken.tokType = TT_COMMA;
        nextChar( );
    }
    else if ( currentChar == '.' ) {
        
        currentToken.tokType = TT_PERIOD;
        nextChar( );
    }
    else currentToken.tokType = TT_ERR;
}

bool expectToken( uint8_t tokType, char *errStr ) {
    
    if ( currentToken.tokType == tokType ) {
        
        nextToken( );
        return( true );
    }
    else return( parserError( errStr ));
}

//------------------------------------------------------------------------------------------------------------
// "parseInstrOptions" will analyze the opCode option string. An opCode string looks just like an indentifier
// to the one line parser. We will look at each character in the "name" and set the options for the particular
// instruction. The routine will also analyze a sequence of period/option string. One day we may have options
// that have more than one period/option pair.
//
// ??? to do ...
//------------------------------------------------------------------------------------------------------------
bool parseInstrOptions( uint32_t *instr ) {
    
    nextToken( );
    if ( currentToken.tokType == TT_IDENT ) {
    
        switch( getBitField( *instr, 5, 6 )) {
                    
            case OP_LD:     case OP_ST:  case OP_LDA:     case OP_STA:  {
            
                if ( currentToken.tokName[ 0 ] == 'M' ) setBit( instr, 11 );
                else return( parserError((char *) "Invalid instruction option" ));
                
            } break;
                    
            case OP_ADD:    case OP_ADC:    case OP_SUB:    case OP_SBC: {
                
                for ( int i = 0; i < strlen( currentToken.tokName ); i ++ ) {
                    
                    if      ( currentToken.tokName[ i ] == 'L' ) setBit( instr, 10 );
                    else if ( currentToken.tokName[ i ] == 'O' ) setBit( instr, 11 );
                    else return( parserError((char *) "Invalid instruction option" ));
                }
                    
            } break;
                    
            case OP_AND:    case OP_OR: {
                
                for ( int i = 0; i < strlen( currentToken.tokName ); i ++ ) {
                    
                    if      ( currentToken.tokName[ i ] == 'N' ) setBit( instr, 10 );
                    else if ( currentToken.tokName[ i ] == 'C' ) setBit( instr, 11 );
                    else return( parserError((char *) "Invalid instruction option" ));
                }
                
            } break;
            
            case OP_XOR: {
                
                if      ( currentToken.tokName[ 0 ] == 'N' ) setBit( instr, 10 );
                else return( parserError((char *) "Invalid instruction option" ));

            } break;
                
            case OP_CMP:    case OP_CMPU: {
                
                if ( strcmp( currentToken.tokName, ((char *) "EQ" ))) setBitField( instr, 11, 2, 0 );
                if ( strcmp( currentToken.tokName, ((char *) "LT" ))) setBitField( instr, 11, 2, 1 );
                if ( strcmp( currentToken.tokName, ((char *) "NE" ))) setBitField( instr, 11, 2, 2 );
                if ( strcmp( currentToken.tokName, ((char *) "LE" ))) setBitField( instr, 11, 2, 3 );
                
            } break;
                
            case OP_EXTR: {
                
                for ( int i = 0; i < strlen( currentToken.tokName ); i ++ ) {
                    
                    if      ( currentToken.tokName[ i ] == 'S' ) setBit( instr, 10 );
                    else if ( currentToken.tokName[ i ] == 'A' ) setBit( instr, 11 );
                    else return( parserError((char *) "Invalid instruction option" ));
                }
                
            } break;
                
            case OP_DEP: {
                
                for ( int i = 0; i < strlen( currentToken.tokName ); i ++ ) {
                    
                    if      ( currentToken.tokName[ i ] == 'Z' ) setBit( instr, 10 );
                    else if ( currentToken.tokName[ i ] == 'A' ) setBit( instr, 11 );
                    else if ( currentToken.tokName[ i ] == 'I' ) setBit( instr, 12 );
                    else return( parserError((char *) "Invalid instruction option" ));
                }
                
            } break;
                
            case OP_DSR: {
                
                if      ( currentToken.tokName[ 0 ] == 'A' ) setBit( instr, 11 );
                else return( parserError((char *) "Invalid instruction option" ));

            } break;
                
            case OP_SHLA: {
                
                for ( int i = 0; i < strlen( currentToken.tokName ); i ++ ) {
                    
                    if      ( currentToken.tokName[ i ] == 'I' ) setBit( instr, 10 );
                    else if ( currentToken.tokName[ i ] == 'L' ) setBit( instr, 11 );
                    else if ( currentToken.tokName[ i ] == 'O' ) setBit( instr, 12 );
                    else return( parserError((char *) "Invalid instruction option" ));
                }
                
            } break;
                
            case OP_MR: {
                
                for ( int i = 0; i < strlen( currentToken.tokName ); i ++ ) {
                    
                    if      ( currentToken.tokName[ i ] == 'D' ) setBit( instr, 10 );
                    else if ( currentToken.tokName[ i ] == 'M' ) setBit( instr, 11 );
                    else return( parserError((char *) "Invalid instruction option" ));
                }
                
            } break;
                
            case OP_PRB: {
                
                for ( int i = 0; i < strlen( currentToken.tokName ); i ++ ) {
                    
                    if      ( currentToken.tokName[ i ] == 'W' ) setBit( instr, 10 );
                    else if ( currentToken.tokName[ i ] == 'I' ) setBit( instr, 11 );
                    else return( parserError((char *) "Invalid instruction option" ));
                }
                
            } break;
                
            case OP_ITLB: {
                
                if      ( currentToken.tokName[ 0 ] == 'T' ) setBit( instr, 11 );
                else return( parserError((char *) "Invalid instruction option" ));

            } break;
                
            case OP_PTLB: {
                
                for ( int i = 0; i < strlen( currentToken.tokName ); i ++ ) {
                    
                    if      ( currentToken.tokName[ i ] == 'T' ) setBit( instr, 10 );
                    else if ( currentToken.tokName[ i ] == 'M' ) setBit( instr, 11 );
                    else return( parserError((char *) "Invalid instruction option" ));
                }
                
            } break;
                
            case OP_PCA: {
                
                for ( int i = 0; i < strlen( currentToken.tokName ); i ++ ) {
                    
                    if      ( currentToken.tokName[ i ] == 'T' ) setBit( instr, 10 );
                    else if ( currentToken.tokName[ i ] == 'M' ) setBit( instr, 11 );
                    else if ( currentToken.tokName[ i ] == 'F' ) setBit( instr, 14 );
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
// "parseModeTypeInstr" parses all instructions that have an "operand" encoding.
//
// ??? this will cover a lot of instructions ....
//------------------------------------------------------------------------------------------------------------
bool parseModeTypeInstr( uint32_t *instr ) {
    
    if ( currentToken.tokType == TT_GREG ) {
        
        setBitField( instr, 9, 4, currentToken.tokValue );
        nextToken( );
    }
    else return( parserError((char *) "Expected a general register" ));
    
    if ( currentToken.tokType == TT_COMMA ) {
        
        nextToken( );
        
        if ( currentToken.tokType == TT_NUM ) {
            
            Token tmp = currentToken;
            
            nextToken( );
            if ( currentToken.tokType == TT_LPAREN ) {
                
                nextToken( );
                if ( currentToken.tokType == TT_GREG ) {
                    
                    
                    
                    nextToken( );
                    if ( currentToken.tokType != TT_RPAREN ) {
                        
                        return( parserError(( char *) "Expected a right parenthesis" ));
                    }
                    
                    return( true );
                }
                else  {
                    
                    
                    
                }
            }
            else {
                
                
            }
            
        }
        else if ( currentToken.tokType == TT_GREG ) {
            
         
            // ??? set A field
            
            nextToken( );
            
            if ( currentToken.tokType == TT_COMMA ) {
                
                nextToken( );
                if ( currentToken.tokType == TT_GREG ) {
                    
                    
                }
                else ;
            }
        }
        else if ( currentToken.tokType == TT_LPAREN ) {
            
            
        }
        
        
        
        
        // ...
        
        
        
        return( true );
        
    }
    else return( parserError((char *) "Expected a comma" ));
}

//------------------------------------------------------------------------------------------------------------
//
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrEXTR( uint32_t *instr ) {
    
    return( true );
}

//------------------------------------------------------------------------------------------------------------
//
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrDEP( uint32_t *instr ) {
    
    return( true );
}

//------------------------------------------------------------------------------------------------------------
//
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrDSR( uint32_t *instr ) {
    
    return( true );
}

//------------------------------------------------------------------------------------------------------------
//
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrCMR( uint32_t *instr ) {
    
    return( true );
}

//------------------------------------------------------------------------------------------------------------
// "parseLDxSTxOperand" parses the operand portion of the load and store instruction. The syntax is either
//
//      <ofs> ( SR, GR )
//      <ofs> ( GR )
//      ( SR, GR )
//      ( GR )
//
// We expect the instrcution template with opCode, opCode options and word length already set. The routine
// is called for load, where the operand is the source, and for store where the operand is the target.
//
//------------------------------------------------------------------------------------------------------------
bool parseLoadStoreOperand( uint32_t *instr ) {
    
    if ( currentToken.tokType == TT_NUM ) {
        
        if (((int32_t) currentToken.tokValue >= -2048 ) && ((int32_t) currentToken.tokValue <= 2047 )) {
            
            setBit( instr, 27, (((int32_t) currentToken.tokValue < 0 ) ? 1 : 0 ));
            setBitField( instr, 26, 11, (( currentToken.tokValue ) & 0x7FF ));
            nextToken( );
        }
        else parserError((char *) "Offset value out of range" );
    }
    else if ( currentToken.tokType == TT_GREG ) {
        
        setBit( instr, 10 );
        setBitField( instr, 27, 4, currentToken.tokValue );
        nextToken( );
    }
    else if ( currentToken.tokType == TT_LPAREN ) {
        
        setBitField( instr, 27, 12, 0 );
    }
    else return( parserError((char *) "Expected the operand" ));
    
    if ( currentToken.tokType == TT_LPAREN ) {
        
        nextToken( );
        if ( currentToken.tokType == TT_SREG ) {
            
            if (( getBitField( *instr, 5, 6 ) == OP_LDA ) || ( getBitField( *instr, 5, 6 ) == OP_STA )) {
                
                return( parserError((char *) "Invalid address for instruction: seg:ofs" ));
            }
            
            if (( currentToken.tokValue >= 1 ) && ( currentToken.tokValue <= 3 )) {
                
                setBitField( instr, 13, 2, currentToken.tokValue );
            }
            else return( parserError((char *) "Expected SR1 .. SR3 " ));
           
            nextToken( );
            if ( currentToken.tokType == TT_COMMA ) {
                
                nextToken( );
                if ( currentToken.tokType == TT_GREG ) {
            
                    setBitField( instr, 31, 4, currentToken.tokValue );
                    
                    nextToken( );
                    if ( currentToken.tokType == TT_RPAREN ) {
                     
                        nextToken( );
                        return( true );
                    }
                    else return( parserError((char *) "Expected a right paren" ));
                }
                else return( parserError((char *) "Expected a comma" ));
            }
            else return( parserError((char *) "Expected a comma" ));
        }
        else if ( currentToken.tokType == TT_GREG ) {
            
            setBitField( instr, 13, 2, 0 );
            setBitField( instr, 31, 4, currentToken.tokValue );
            
            nextToken( );
            if ( currentToken.tokType == TT_RPAREN ) return( true );
            else return( parserError((char *) "Expected a right paren" ));
        }
        else return( parserError((char *) "Expected a general or segement register" ));
    }
    else return( parserError((char *) "Expected a left paren" ));
    
    return( true );
}

//------------------------------------------------------------------------------------------------------------
// "parseInstrLDx" will parse the LDx instructions.
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrLoad( uint32_t *instr ) {
    
    if ( currentToken.tokType == TT_GREG ) {
        
        setBitField( instr, 9, 4, currentToken.tokValue );
        nextToken( );
    }
    else return( parserError((char *) "Expected a general register" ));
    
    if ( currentToken.tokType == TT_COMMA ) {
        
        nextToken( );
        return( parseLoadStoreOperand( instr ));
    }
    else return( parserError((char *) "Expected a comma" ));
}

//------------------------------------------------------------------------------------------------------------
// "parseInstrSTx" will parse the STx instructions.
//
//------------------------------------------------------------------------------------------------------------
bool parseInstrStore( uint32_t *instr ) {
  
    if ( parseLoadStoreOperand( instr )) {
        
        nextToken( );
        if ( currentToken.tokType == TT_COMMA ) {
            
            nextToken( );
            if ( currentToken.tokType == TT_GREG ) {
                
                setBitField( instr, 9, 4, currentToken.tokValue );
                return( true );
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
// is a routine that parses the instruction specific input. As the parsing process comes along the instruction
// template from the token name table will be augmented with further data. If all is successful, we will have
// the final instruction bit pattern.
//
//------------------------------------------------------------------------------------------------------------
bool parseLine( char *inputStr, uint32_t *instr, TokId fmtId ) {
    
    strncpy( tokenLine, inputStr, strlen( inputStr ) + 1 );
    upshiftStr( tokenLine );
    
    *instr              = 0;
    currentLineLen      = (int) strlen( tokenLine );
    currentCharIndex    = 0;
    currentTokIndex     = 0;
    currentChar         = ' ';
    
    fprintf( stdout, "In: %s\n", tokenLine );
    
    nextToken( );
    if ( currentToken.tokType == TT_OPCODE ) {
        
        *instr = currentToken.tokValue;
        
        nextToken( );
        if ( currentToken.tokType == TT_PERIOD ) {
            
            parseInstrOptions( instr );
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
            case OP_LDA:    return( parseInstrLoad( instr ));
                
            case OP_ST:
            case OP_STA:    return( parseInstrStore( instr ));

            case OP_EXTR:   return( parseInstrEXTR( instr ));
            case OP_DEP:    return( parseInstrEXTR( instr ));
            case OP_DSR:    return( parseInstrEXTR( instr ));
                
            // ...
                
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

uint8_t DrvOneLineAsm::parseAsmLine( char *inputStr, uint32_t *instr, TokId fmtId ) {
    
    return ( parseLine( inputStr, instr, fmtId ));
}
