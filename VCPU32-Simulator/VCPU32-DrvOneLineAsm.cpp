//------------------------------------------------------------------------------------------------------------
//
// VCPU32 - A 32-bit CPU - One Line Assembler
//
//------------------------------------------------------------------------------------------------------------
// The one line assembler assembles an instruction without further context. It is intended to for testing
// instructions in the simulator. There is no symbol table or any concept of assembling multiple instructions.
// The instruction to generate test is completely self sufficient. The parser is a straightforward recursive
// descendant parser, LL1 grammar. It uses the C++ try / catch to escape when an error is detected. Considering
// that we only have one line to parse, there is no need to implement a better parser error recovery method.
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
#include "VCPU32-DrvTables.h"
#include "VCPU32-Core.h"

//------------------------------------------------------------------------------------------------------------
// Local namespace. These routines are not visible outside this source file.
//
//------------------------------------------------------------------------------------------------------------
namespace {

//------------------------------------------------------------------------------------------------------------
// We need a tokenizer object. It is allocated only once. On each call to assemble an instruction, the
// tokenizer input is set for the current work task.
//
//------------------------------------------------------------------------------------------------------------
DrvTokenizer *tok;

//------------------------------------------------------------------------------------------------------------
// Token flags. They are used to communicate additional information about the the token to the assembly
// process. Examples are the data width encoded in the opCode and the instruction mask.
//
//------------------------------------------------------------------------------------------------------------
enum TokenFlags : uint32_t {
  
    TF_NIL              = 0,
    TF_BYTE_INSTR       = ( 1U << 0 ),
    TF_HALF_INSTR       = ( 1U << 1 ),
    TF_WORD_INSTR       = ( 1U << 2 )
};

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
// Instruction encoding means to fiddle with bits and bit fields. Here is a set of helper functions.
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
    
    int min = - ( 1 << (( bitLen -1 ) % 32 ));
    int max = ( 1 << (( bitLen - 1 ) %32 )) - 1;
    return(( val <= max ) && ( val >= min ));
}

bool isInRangeForBitFieldU( uint32_t val, uint8_t bitLen ) {
    
    int max = (( 1 << ( bitLen % 32 )) - 1 );
    return( val <= max );
}

//------------------------------------------------------------------------------------------------------------
// "asmError" is a little helper that prints out the original input line and a caret marker where we found
// the error.
//
//------------------------------------------------------------------------------------------------------------
ErrMsgId markAsmError( ErrMsgId errNum ) {
    
    fprintf( stdout, "%s\n", tok -> tokenLineStr( ));
    
    int i           = 0;
    int tokIndex    = tok -> tokCharIndex( );
    
    while ( i < tokIndex ) {
        
        fprintf( stdout, " " );
        i ++;
    }
    
    fprintf( stdout, "^ \n" );
    return( errNum );
}

//------------------------------------------------------------------------------------------------------------
// Check that the ASM line does not contain any extra tokens when the parser completed the analysis of the
// assembly line.
//
//------------------------------------------------------------------------------------------------------------
void checkEOS( ) {
    
    if ( ! tok -> isToken( TOK_EOS )) throw( ERR_EXTRA_TOKEN_IN_STR );
}

void acceptComma( ) {
    
    if ( tok -> isToken( TOK_COMMA )) tok -> nextToken( );
    else throw( ERR_EXPECTED_COMMA );
}

void acceptLparen( ) {
    
    if ( tok -> isToken( TOK_LPAREN )) tok -> nextToken( );
    else throw( ERR_EXPECTED_LPAREN );
}

void acceptRparen( ) {
    
    if ( tok -> isToken( TOK_RPAREN )) tok -> nextToken( );
    else throw( ERR_EXPECTED_RPAREN );
}


//------------------------------------------------------------------------------------------------------------
// "parseExpr" needs to be declared forward.
//
//------------------------------------------------------------------------------------------------------------
void parseExpr( DrvExpr *rExpr );

//------------------------------------------------------------------------------------------------------------
// "parseFactor" parses the factor syntax part of an expression.
//
//      <factor> -> <number>                        |
//                  <gregId>                        |
//                  <sregId>                        |
//                  <cregId>                        |
//                  "~" <factor>                    |
//                  "(" [ <sreg> "," ] <greg> ")"   |
//                  "(" <expr> ")"
//
//------------------------------------------------------------------------------------------------------------
void parseFactor( DrvExpr *rExpr ) {
    
    rExpr -> typ  = TYP_NIL;
    rExpr -> numVal = 0;

    if ( tok -> isTokenTyp( TYP_NUM ))  {
        
        rExpr -> typ = TYP_NUM;
        rExpr -> numVal = tok -> tokVal( );
        tok -> nextToken( );
    }
    else  if ( tok -> isTokenTyp( TYP_GREG ))  {
        
        rExpr -> typ = TYP_GREG;
        rExpr -> numVal = tok -> tokVal( );
        tok -> nextToken( );
    }
    else  if ( tok -> isTokenTyp( TYP_SREG ))  {
        
        rExpr -> typ = TYP_SREG;
        rExpr -> numVal = tok -> tokVal( );
        tok -> nextToken( );
    }
    else  if ( tok -> isTokenTyp( TYP_CREG ))  {
        
        rExpr -> typ = TYP_CREG;
        rExpr -> numVal = tok -> tokVal( );
        tok -> nextToken( );
    }
    else if ( tok -> isToken( TOK_NEG )) {
        
        parseFactor( rExpr );
        rExpr -> numVal = ~ rExpr -> numVal;
    }
    else if ( tok -> isToken( TOK_LPAREN )) {
        
        tok -> nextToken( );
        if ( tok ->  isTokenTyp( TYP_SREG )) {
            
            rExpr -> typ    = TYP_EXT_ADR;
            rExpr -> sReg   = tok -> tokVal( );
            
            acceptComma( );
            
            if ( tok -> isTokenTyp( TYP_GREG )) {
                
                rExpr -> gReg = tok -> tokVal( );
                tok -> nextToken( );
            }
            else throw( ERR_EXPECTED_GENERAL_REG );
        }
        else if ( tok -> isTokenTyp( TYP_GREG )) {
            
            rExpr -> typ = TYP_ADR;
            rExpr -> numVal = tok -> tokVal( );
            tok -> nextToken( );
        }
        else parseExpr( rExpr );
        
        acceptRparen( );
    }
    else {
                
        rExpr -> typ    = TYP_NUM;
        rExpr -> numVal = 0;
        tok -> nextToken( );
        
        throw ( ERR_INVALID_EXPR );
    }
}

//------------------------------------------------------------------------------------------------------------
// "parseTerm" parses the term syntax.
//
//      <term>      ->  <factor> { <termOp> <factor> }
//      <termOp>    ->  "*" | "/" | "%" | "&"
//
//------------------------------------------------------------------------------------------------------------
void parseTerm( DrvExpr *rExpr ) {
    
    DrvExpr lExpr;
    
    parseFactor( rExpr );
    
    while (( tok -> tokId( ) == TOK_MULT )   ||
           ( tok -> tokId( ) == TOK_DIV  )   ||
           ( tok -> tokId( ) == TOK_MOD  )   ||
           ( tok -> tokId( ) == TOK_AND  ))  {
        
        uint8_t op = tok -> tokId( );
        
        tok -> nextToken( );
        parseFactor( &lExpr );
        
        if ( rExpr -> typ != lExpr.typ ) throw ( ERR_EXPR_TYPE_MATCH );
       
        switch( op ) {
                
            case TOK_MULT:   rExpr -> numVal = rExpr -> numVal * lExpr.numVal; break;
            case TOK_DIV:    rExpr -> numVal = rExpr -> numVal / lExpr.numVal; break;
            case TOK_MOD:    rExpr -> numVal = rExpr -> numVal % lExpr.numVal; break;
            case TOK_AND:    rExpr -> numVal = rExpr -> numVal & lExpr.numVal; break;
        }
    }
}

//------------------------------------------------------------------------------------------------------------
// "parseExpr" parses the expression syntax. The one line assembler parser routines use this call in many
// places where a numeric expression or an address is needed.
//
//      <expr>      ->  [ ( "+" | "-" ) ] <term> { <exprOp> <term> }
//      <exprOp>    ->  "+" | "-" | "|" | "^"
//
//------------------------------------------------------------------------------------------------------------
void parseExpr( DrvExpr *rExpr ) {
    
    DrvExpr lExpr;
    
    if ( tok -> isToken( TOK_PLUS )) {
        
        tok -> nextToken( );
        parseTerm( rExpr );
        
        if ( ! ( rExpr -> typ == TYP_NUM )) throw ( ERR_EXPECTED_NUMERIC );
    }
    else if ( tok -> isToken( TOK_MINUS )) {
        
        tok -> nextToken( );
        parseTerm( rExpr );
        
        if ( rExpr -> typ == TYP_NUM ) rExpr -> numVal = - (int32_t) rExpr -> numVal;
        else throw( ERR_EXPECTED_NUMERIC );
    }
    else parseTerm( rExpr );
    
    while (( tok -> tokId( ) == TOK_PLUS    ) ||
           ( tok -> tokId( ) == TOK_MINUS   ) ||
           ( tok -> tokId( ) == TOK_OR      ) ||
           ( tok -> tokId( ) == TOK_XOR     )) {
        
        uint8_t op = tok -> tokId( );
        
        tok -> nextToken( );
        parseTerm( &lExpr );
        
        if ( rExpr -> typ != lExpr.typ ) throw ( ERR_EXPR_TYPE_MATCH );
        
        switch ( op ) {
                
            case TOK_PLUS:   rExpr -> numVal = rExpr -> numVal + lExpr.numVal; break;
            case TOK_MINUS:  rExpr -> numVal = rExpr -> numVal - lExpr.numVal; break;
            case TOK_OR:     rExpr -> numVal = rExpr -> numVal | lExpr.numVal; break;
            case TOK_XOR:    rExpr -> numVal = rExpr -> numVal ^ lExpr.numVal; break;
        }
    }
}

//------------------------------------------------------------------------------------------------------------
// "parseInstrOptions" will analyze the opCode option string. An opCode string is a sequence of characters.
// We will look at each character in the "name" and set the options for the particular instruction. There are
// also cases where the only option is a multi-character sequence. We detect invalid options but not when
// the same option is repeated. E.g. a "LOL" will result in "L" and "O" set.
//
//------------------------------------------------------------------------------------------------------------
void parseInstrOptions( uint32_t *instr, uint32_t *flags ) {
    
    if ( ! tok -> isToken( TOK_IDENT )) throw ( ERR_EXPECTED_INSTR_OPT );
    
    char *optBuf = tok -> tokStr( );
    
    switch( getBitField( *instr, 5, 6 )) {
            
        case OP_LD:
        case OP_ST:
        case OP_LDA:
        case OP_STA:  {
            
            if ( optBuf[ 0 ] == 'M' ) setBit( instr, 11 );
            else throw ( ERR_INVALID_INSTR_OPT );
            
        } break;
            
        case OP_ADD:
        case OP_ADC:
        case OP_SUB:
        case OP_SBC: {
            
            for ( int i = 0; i < strlen( optBuf ); i ++ ) {
                
                if      ( optBuf[ i ] == 'L' ) setBit( instr, 10 );
                else if ( optBuf[ i ] == 'O' ) setBit( instr, 11 );
                else throw ( ERR_INVALID_INSTR_OPT );
            }
            
        } break;
            
        case OP_AND:
        case OP_OR: {
            
            for ( int i = 0; i < strlen( optBuf ); i ++ ) {
                
                if      ( optBuf[ i ] == 'N' ) setBit( instr, 10 );
                else if ( optBuf[ i ] == 'C' ) setBit( instr, 11 );
                else throw ( ERR_INVALID_INSTR_OPT );
            }
            
        } break;
            
        case OP_XOR: {
            
            if ( optBuf[ 0 ] == 'N' ) setBit( instr, 10 );
            else throw ( ERR_INVALID_INSTR_OPT );
            
        } break;
            
        case OP_CMP:
        case OP_CMPU: {
            
            if      ( strcmp( optBuf, ((char *) "EQ" )) == 0 ) setBitField( instr, 11, 2, 0 );
            else if ( strcmp( optBuf, ((char *) "LT" )) == 0 ) setBitField( instr, 11, 2, 1 );
            else if ( strcmp( optBuf, ((char *) "NE" )) == 0 ) setBitField( instr, 11, 2, 2 );
            else if ( strcmp( optBuf, ((char *) "LE" )) == 0 ) setBitField( instr, 11, 2, 3 );
            else throw ( ERR_INVALID_INSTR_OPT );

        } break;
            
        case OP_CBR:
        case OP_CBRU: {
            
            if      ( strcmp( optBuf, ((char *) "EQ" )) == 0 ) setBitField( instr, 7, 2, 0 );
            else if ( strcmp( optBuf, ((char *) "LT" )) == 0 ) setBitField( instr, 7, 2, 1 );
            else if ( strcmp( optBuf, ((char *) "NE" )) == 0 ) setBitField( instr, 7, 2, 2 );
            else if ( strcmp( optBuf, ((char *) "LE" )) == 0 ) setBitField( instr, 7, 2, 3 );
            else throw ( ERR_INVALID_INSTR_OPT );

        } break;
            
        case OP_CMR: {
            
            if      ( strcmp( optBuf, ((char *) "EQ" )) == 0 ) setBitField( instr, 13, 4, 0 );
            else if ( strcmp( optBuf, ((char *) "LT" )) == 0 ) setBitField( instr, 13, 4, 1 );
            else if ( strcmp( optBuf, ((char *) "GT" )) == 0 ) setBitField( instr, 13, 4, 2 );
            else if ( strcmp( optBuf, ((char *) "EV" )) == 0 ) setBitField( instr, 13, 4, 3 );
            else if ( strcmp( optBuf, ((char *) "NE" )) == 0 ) setBitField( instr, 13, 4, 4 );
            else if ( strcmp( optBuf, ((char *) "LE" )) == 0 ) setBitField( instr, 13, 4, 5 );
            else if ( strcmp( optBuf, ((char *) "GE" )) == 0 ) setBitField( instr, 13, 4, 6 );
            else if ( strcmp( optBuf, ((char *) "OD" )) == 0 ) setBitField( instr, 13, 4, 7 );
            else throw ( ERR_INVALID_INSTR_OPT );
            
        } break;
            
        case OP_EXTR: {
            
            for ( int i = 0; i < strlen( optBuf ); i ++ ) {
                
                if      ( optBuf[ i ] == 'S' ) setBit( instr, 10 );
                else if ( optBuf[ i ] == 'A' ) setBit( instr, 11 );
                else throw ( ERR_INVALID_INSTR_OPT );
            }
            
        } break;
            
        case OP_DEP: {
            
            for ( int i = 0; i < strlen( optBuf ); i ++ ) {
                
                if      ( optBuf[ i ] == 'Z' ) setBit( instr, 10 );
                else if ( optBuf[ i ] == 'A' ) setBit( instr, 11 );
                else if ( optBuf[ i ] == 'I' ) setBit( instr, 12 );
                else throw ( ERR_INVALID_INSTR_OPT );
            }
            
        } break;
            
        case OP_DSR: {
            
            if ( optBuf[ 0 ] == 'A' ) setBit( instr, 11 );
            else throw ( ERR_INVALID_INSTR_OPT );
            
        } break;
            
        case OP_SHLA: {
            
            for ( int i = 0; i < strlen( optBuf ); i ++ ) {
                
                if      ( optBuf[ i ] == 'I' ) setBit( instr, 10 );
                else if ( optBuf[ i ] == 'L' ) setBit( instr, 11 );
                else if ( optBuf[ i ] == 'O' ) setBit( instr, 12 );
                else throw ( ERR_INVALID_INSTR_OPT );
            }
            
        } break;
            
        case OP_MR: {
            
            for ( int i = 0; i < strlen( optBuf ); i ++ ) {
                
                if      ( optBuf[ i ] == 'D' ) setBit( instr, 10 );
                else if ( optBuf[ i ] == 'M' ) setBit( instr, 11 );
                else throw ( ERR_INVALID_INSTR_OPT );
            }
            
        } break;
            
        case OP_MST: {
            
            for ( int i = 0; i < strlen( optBuf ); i ++ ) {
                
                if      ( optBuf[ i ] == 'S' ) setImmValU( instr, 11, 2, 1 );
                else if ( optBuf[ i ] == 'C' ) setImmValU( instr, 11, 2, 2 );
                else throw ( ERR_INVALID_INSTR_OPT );
            }
    
        } break;
            
        case OP_PRB: {
            
            for ( int i = 0; i < strlen( optBuf ); i ++ ) {
                
                if      ( optBuf[ i ] == 'W' ) setBit( instr, 10 );
                else if ( optBuf[ i ] == 'I' ) setBit( instr, 11 );
                else throw ( ERR_INVALID_INSTR_OPT );
            }
            
        } break;
            
        case OP_ITLB: {
            
            if ( optBuf[ 0 ] == 'T' ) setBit( instr, 11 );
            else throw ( ERR_INVALID_INSTR_OPT );
            
        } break;
            
        case OP_PTLB: {
            
            for ( int i = 0; i < strlen( optBuf ); i ++ ) {
                
                if      ( optBuf[ i ] == 'T' ) setBit( instr, 10 );
                else if ( optBuf[ i ] == 'M' ) setBit( instr, 11 );
                else throw ( ERR_INVALID_INSTR_OPT );
            }
            
        } break;
            
        case OP_PCA: {
            
            for ( int i = 0; i < strlen( optBuf ); i ++ ) {
                
                if      ( optBuf[ i ] == 'T' ) setBit( instr, 10 );
                else if ( optBuf[ i ] == 'M' ) setBit( instr, 11 );
                else if ( optBuf[ i ] == 'F' ) setBit( instr, 14 );
                else throw ( ERR_INVALID_INSTR_OPT );
            }
            
        } break;
            
        default: throw ( ERR_INSTR_HAS_NO_OPT );
    }
    
    tok -> nextToken( );
}

//------------------------------------------------------------------------------------------------------------
// "parseLogicalAdr" analyzes a logical address, which is used by several instruction with a "seg" field.
//
//      "(" [ <segReg> "," ] <ofsReg> ")"
//
//------------------------------------------------------------------------------------------------------------
void parseLogicalAdr( uint32_t *instr, uint32_t flags ) {
    
    DrvExpr rExpr;
   
    parseExpr( &rExpr );
   
    if ( rExpr.typ == TYP_EXT_ADR ) {
        
        setBitField( instr, 31, 4, rExpr.gReg );
        
        if ( isInRange( rExpr.sReg, 1, 3 )) setBitField( instr, 13, 2, rExpr.sReg );
        else throw ( ERR_EXPECTED_SR1_SR3 );
    }
    else if ( rExpr.typ == TYP_ADR ) {
        
        setBitField( instr, 31, 4, rExpr.adr );
    }
    else throw ( ERR_EXPECTED_LOGICAL_ADR );
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
void parseLoadStoreOperand( uint32_t *instr, uint32_t flags ) {
    
    DrvExpr  rExpr;
    
    if      ( flags & TF_BYTE_INSTR ) setBitField( instr, 15, 2, 0 );
    else if ( flags & TF_HALF_INSTR ) setBitField( instr, 15, 2, 1 );
    else if ( flags & TF_WORD_INSTR ) setBitField( instr, 15, 2, 2 );
  
    parseExpr( &rExpr );
   
    if ( rExpr.typ == TYP_NUM ) {
        
        if ( isInRangeForBitField( rExpr.numVal, 12 )) setImmVal( instr, 27, 12, rExpr.numVal );
        else throw ( ERR_IMM_VAL_RANGE );
            
        parseExpr( &rExpr );
    }
    else if ( rExpr.typ == TYP_GREG ) {
        
        if (( getBitField( *instr, 5, 6 ) == OP_LDR ) || ( getBitField( *instr, 5, 6 ) == OP_STC ))
            throw ( ERR_INVALID_INSTR_MODE );
            
        setBit( instr, 10 );
        setBitField( instr, 27, 4, rExpr.numVal );
        
        parseExpr( &rExpr );
    }
    
    if ( rExpr.typ == TYP_ADR ) {
                    
        setBitField( instr, 13, 2, 0 );
        setBitField( instr, 31, 4, rExpr.numVal );
    }
    else if ( rExpr.typ == TYP_EXT_ADR ) {
                    
        if (( getBitField( *instr, 5, 6 ) == OP_LDA ) || ( getBitField( *instr, 5, 6 ) == OP_STA )) {
            
            throw ( ERR_INVALID_INSTR_MODE );
        }
                
        if ( isInRange( rExpr.sReg, 1, 3 )) setBitField( instr, 13, 2, rExpr.sReg );
        else throw( ERR_EXPECTED_SR1_SR3 );
        
        setBitField( instr, 31, 4, rExpr.gReg );
    }
    else throw ( ERR_EXPECTED_LOGICAL_ADR );
}

//------------------------------------------------------------------------------------------------------------
// "parseModeTypeInstr" parses all instructions that have an "operand" encoding. The syntax is as follows:
//
//      opCode [ "." <opt> ] <targetReg> "," <num>                                - mode 0
//      opCode [ "." <opt> ] <targetReg> "," <num> "(" <baseReg> ")"              - mode 3
//      opCode [ "." <opt> ] <targetReg> "," <sourceReg>                          - mode 1
//      opCode [ "." <opt> ] <targetReg> "," <sourceRegA> "," "<sourceRegB>       - mode 1
//      opCode [ "." <opt> ] <targetReg> "," <indexReg> "(" <baseReg> ")"         - mode 2
//
//------------------------------------------------------------------------------------------------------------
void parseModeTypeInstr( uint32_t *instr, uint32_t flags ) {
    
    uint8_t     targetRegId = 0;
    DrvExpr     rExpr;
   
    if ( tok -> isTokenTyp( TYP_GREG )) {
        
        targetRegId = tok -> tokVal( );
        setBitField( instr, 9, 4, tok -> tokVal( ));
        tok -> nextToken( );
    }
    else throw ( ERR_EXPECTED_GENERAL_REG );
    
    acceptComma( );
    parseExpr( &rExpr );
    
    if ( rExpr.typ == TYP_NUM ) {
     
        if ( tok -> isToken( TOK_EOS )) {
            
            if ( isInRangeForBitField( rExpr.numVal, 18 )) setImmVal( instr, 31, 18, rExpr.numVal );
            else throw ( ERR_IMM_VAL_RANGE );
        }
        else {
            
            if ( isInRangeForBitField( rExpr.numVal, 12 )) setImmVal( instr, 27, 12, rExpr.numVal );
            else throw ( ERR_IMM_VAL_RANGE );
            
            parseExpr( &rExpr );
        
            if ( rExpr.typ == TYP_ADR ) {
                
                setBitField( instr, 13, 2, 3 );
                setBitField( instr, 31, 4, rExpr.numVal );
            }
            else throw ( ERR_EXPECTED_LOGICAL_ADR );
               
            if      ( flags & TF_BYTE_INSTR ) setBitField( instr, 15, 2, 0 );
            else if ( flags & TF_HALF_INSTR ) setBitField( instr, 15, 2, 1 );
            else if ( flags & TF_WORD_INSTR ) setBitField( instr, 15, 2, 2 );
        }
    }
    else if ( rExpr.typ == TYP_GREG ) {
    
        if ( tok -> isToken( TOK_EOS )) {
            
            setBitField( instr, 13, 2, 1 );
            setBitField( instr, 27, 4, targetRegId );
            setBitField( instr, 31, 4, rExpr.numVal );
        }
        else if ( tok -> isToken( TOK_COMMA )) {
            
            setBitField( instr, 13, 2, 1 );
            setBitField( instr, 27, 4, rExpr.numVal );
            
            tok -> nextToken( );
            if ( tok -> isTokenTyp( TYP_GREG )) {
                
                setBitField( instr, 13, 2, 1 );
                setBitField( instr, 27, 4, rExpr.numVal );
                setBitField( instr, 31, 4, tok -> tokVal( ));
                tok -> nextToken( );
            }
            else throw ( ERR_EXPECTED_GENERAL_REG );
        }
        else if ( tok -> isToken( TOK_LPAREN )) {
            
            setBitField( instr, 27, 4, rExpr.numVal );
            
            parseExpr( &rExpr );
            if ( rExpr.typ == TYP_ADR ) {
                
                setBitField( instr, 13, 2, 2 );
                setBitField( instr, 31, 4, rExpr.numVal );
            }
            else throw ( ERR_EXPECTED_LOGICAL_ADR );
            
            if      ( flags & TF_BYTE_INSTR ) setBitField( instr, 15, 2, 0 );
            else if ( flags & TF_HALF_INSTR ) setBitField( instr, 15, 2, 1 );
            else if ( flags & TF_WORD_INSTR ) setBitField( instr, 15, 2, 2 );
        }
    }
    else throw ( ERR_INVALID_INSTR_MODE );
    
    if (  getBitField( *instr, 13, 2 ) < 2 ) {
        
        if (( flags & TF_BYTE_INSTR ) || ( flags & TF_HALF_INSTR ))
            throw ( ERR_INSTR_MODE_OPT_COMBO );
    }
    
    checkEOS( );
}

//------------------------------------------------------------------------------------------------------------
// "parseInstrLSID" parses the LSID instruction.
//
//      <opCode> <targetReg> "," <sourceReg>
//
//------------------------------------------------------------------------------------------------------------
void parseInstrLSID( uint32_t *instr, uint32_t flags ) {
    
    if ( tok -> isTokenTyp( TYP_GREG )) {
        
        setBitField( instr, 9, 4, tok -> tokVal( ));
        tok -> nextToken( );
    }
    else throw ( ERR_EXPECTED_CLOSING_QUOTE );
    
    if ( tok -> isToken( TOK_COMMA )) tok -> nextToken( );
    else throw ( ERR_EXPECTED_COMMA );
               
    if ( tok -> isTokenTyp( TYP_GREG )) {
        
        setBitField( instr, 31, 4, tok -> tokVal( ));
        tok -> nextToken( );
    }
    else throw ( ERR_EXPECTED_GENERAL_REG );
    
    checkEOS( );
}

//------------------------------------------------------------------------------------------------------------
// "parseInstrDEP" parses the deposit instruction. The instruction has three basic formats.
// When the "A" bit is set, the position will be obtained from the shift amount control register. Otherwise
// it is encoded in the instruction.
//
//      DEP [ ".“ <opt> ]       <targetReg> "," <sourceReg> "," <pos> "," <len>"
//      DEP [ "." "A" <opt> ]   <targetReg> "," <sourceReg> ", <len>"
//      DEP [ "." "I" <opt> ]   <targetReg> "," <val>, <pos> "," <len>
//      DEP [ "." "AI" <opt> ]  <targetReg> "," <val> "," <len>
//
//------------------------------------------------------------------------------------------------------------
void parseInstrDEP( uint32_t *instr, uint32_t flags ) {
    
    DrvExpr rExpr;
    
    if ( tok -> isTokenTyp( TYP_GREG )) {
        
        setBitField( instr, 9, 4, tok -> tokVal( ));
        tok -> nextToken( );
    }
    else throw ( ERR_EXPECTED_GENERAL_REG );
    
    acceptComma( );
    parseExpr( &rExpr );

    if ( rExpr.typ == TYP_GREG ) {
        
        setBitField( instr, 31, 4, tok -> tokVal( ));
        
        acceptComma( );
        parseExpr( &rExpr );
       
        if ( rExpr.typ == TYP_NUM ) {
            
            if ( isInRangeForBitFieldU( tok -> tokVal( ), 5 )) {
                
                if ( getBit( *instr, 11 ))  setBitField( instr, 21, 5, rExpr.numVal );
                else                        setBitField( instr, 27, 5, rExpr.numVal );
            }
            else throw ( ERR_IMM_VAL_RANGE );
        }
        else throw ( ERR_EXPECTED_NUMERIC );
        
        if ( ! getBit( *instr, 11 )) {
            
            acceptComma( );
            parseExpr( &rExpr );

            
            if ( rExpr.typ == TYP_NUM ) {
                
                if ( isInRangeForBitFieldU( rExpr.numVal, 5 )) setBitField( instr, 21, 5, rExpr.numVal );
                else throw ( ERR_IMM_VAL_RANGE );
            }
            else throw ( ERR_EXPECTED_NUMERIC );
        }
    }
    else if ( rExpr.typ == TYP_NUM ) {
        
        if ( getBit( *instr, 12 )) {
            
            if ( isInRangeForBitField( rExpr.numVal, 4 )) setBitField( instr, 31, 4, rExpr.numVal );
            else throw ( ERR_IMM_VAL_RANGE );
            
            acceptComma( );
            
            if ( ! getBit( *instr, 11 )) {
               
                if ( isInRangeForBitFieldU( tok -> tokVal( ), 5 )) setBitField( instr, 27, 5, tok -> tokVal( ));
                else throw ( ERR_POS_VAL_RANGE );
                
                tok -> nextToken( );
                if ( tok -> isToken( TOK_COMMA )) tok -> nextToken( );
                else throw ( ERR_EXPECTED_COMMA );
            }
            
            parseExpr( &rExpr );
           
            if ( rExpr.typ == TYP_NUM ) {
                
                if ( isInRangeForBitFieldU( rExpr.numVal, 5 )) setBitField( instr, 21, 5, rExpr.numVal );
                else throw ( ERR_LEN_VAL_RANGE );
            }
            else throw ( ERR_EXPECTED_NUMERIC );
        }
        else throw ( ERR_EXPECTED_NUMERIC );
    }
    else throw ( ERR_EXPECTED_NUMERIC );
    
    checkEOS( );
}

//------------------------------------------------------------------------------------------------------------
// The DS instruction parses the divide step instruction.
//
//      DS <targetReg> "," <sourceRegA> "," <sourceRegB>
//
//------------------------------------------------------------------------------------------------------------
void parseInstrDS( uint32_t *instr, uint32_t flags ) {
    
    if ( tok -> isTokenTyp( TYP_GREG )) {
        
        setBitField( instr, 9, 4, tok -> tokVal( ));
        tok -> nextToken( );
    }
    else throw ( ERR_EXPECTED_GENERAL_REG );
    
    acceptComma( );
   
    if ( tok -> isTokenTyp( TYP_GREG )) {
        
        setBitField( instr, 27, 4, tok -> tokVal( ));
        tok -> nextToken( );
    }
    else throw ( ERR_EXPECTED_GENERAL_REG );
    
    acceptComma( );
    
    if ( tok -> isTokenTyp( TYP_GREG )) {
        
        setBitField( instr, 31, 4, tok -> tokVal( ));
        tok -> nextToken( );
    }
    else throw ( ERR_EXPECTED_GENERAL_REG );
    
    checkEOS( );
}

//------------------------------------------------------------------------------------------------------------
// The DSR instruction parses the double shift instruction. There are two flavors. If the "A" bit is set, the
// shift amount is taken from the shift amount control register, else from the instruction "len" field.
//
//      DSR [ ".“ <opt> ] <targetReg> "," <sourceRegA> "," <sourceRegB> "," <len"
//      DSR [ ".“ "A"   ] <targetReg> "," <sourceRegA> "," <sourceRegB>
//
//------------------------------------------------------------------------------------------------------------
void parseInstrDSR( uint32_t *instr, uint32_t flags ) {
    
    DrvExpr rExpr;
    
    if ( tok -> isTokenTyp( TYP_GREG )) {
        
        setBitField( instr, 9, 4, tok -> tokVal( ));
        tok -> nextToken( );
    }
    else throw ( ERR_EXPECTED_GENERAL_REG );
    
    acceptComma( );
    
    if ( tok -> isTokenTyp( TYP_GREG )) {
        
        setBitField( instr, 27, 4, tok -> tokVal( ));
        tok -> nextToken( );
    }
    else throw ( ERR_EXPECTED_GENERAL_REG );
    
    acceptComma( );
    
    if ( tok -> isTokenTyp( TYP_GREG )) {
        
        setBitField( instr, 31, 4, tok -> tokVal( ));
        tok -> nextToken( );
    }
    else throw ( ERR_EXPECTED_GENERAL_REG );
    
    if ( ! getBit( *instr, 11 )) {
        
        acceptComma( );
        parseExpr( &rExpr );
        
        if ( rExpr.typ == TYP_NUM ) {
            
            if ( isInRangeForBitFieldU( rExpr.numVal, 5 )) setBitField( instr, 21, 5, rExpr.numVal );
            else throw ( ERR_IMM_VAL_RANGE );
        }
        else throw ( ERR_EXPECTED_NUMERIC );
    }
    
    checkEOS( );
}

//------------------------------------------------------------------------------------------------------------
// "parseInstrEXTR" parses the extract instruction. The instruction has two basic formats. When the "A" bit
// is set, the position will be obtained from the shift amount control register. Otherwise it is encoded in
// the instruction.
//
//      EXTR [ ".“ <opt> ]      <targetReg> "," <sourceReg> "," <pos> "," <len"
//      EXTR "." "A" [ <opt> ]  <targetReg> "," <sourceReg> ", <len"
//
//------------------------------------------------------------------------------------------------------------
void parseInstrEXTR( uint32_t *instr, uint32_t flags ) {
    
    DrvExpr rExpr;
    
    if ( tok -> isTokenTyp( TYP_GREG )) {
        
        setBitField( instr, 9, 4, tok -> tokVal( ));
        tok -> nextToken( );
    }
    else throw ( ERR_EXPECTED_GENERAL_REG );
    
    acceptComma( );
   
    if ( tok -> isTokenTyp( TYP_GREG )) {
        
        setBitField( instr, 31, 4, tok -> tokVal( ));
        tok -> nextToken( );
    }
    else throw ( ERR_EXPECTED_GENERAL_REG );
    
    acceptComma( );
    parseExpr( &rExpr );
    
    if ( rExpr.typ == TYP_NUM ) {
        
        if ( isInRangeForBitFieldU( tok -> tokVal( ), 5 )) {
            
            if ( getBit( *instr, 11 ))  setBitField( instr, 21, 5, rExpr.numVal );
            else                        setBitField( instr, 27, 5, rExpr.numVal );
        }
        else throw ( ERR_IMM_VAL_RANGE );
    }
    else throw ( ERR_EXPECTED_NUMERIC );
    
    if ( ! getBit( *instr, 11 )) {
        
        acceptComma( );
        parseExpr( &rExpr );
       
        if ( rExpr.typ == TYP_NUM ) {
            
            if ( isInRangeForBitFieldU( rExpr.numVal, 5 )) {
                
                setBitField( instr, 21, 5, rExpr.numVal );
            }
            else throw ( ERR_IMM_VAL_RANGE );
        }
        else throw ( ERR_EXPECTED_NUMERIC );
    }
    
    checkEOS( );
}

//------------------------------------------------------------------------------------------------------------
// The SHLA instruction performs a shift left of "B" by "sa" and adds the "A" register to it.
//
//      SHLA [ "." <opt> ] <targetReg> "," <sourceRegA> "," <sourceRegB> "," <amt>
//      SHLA ".I" <targetReg> "," <sourceRegA> "," <val> "," <amt>
//
//------------------------------------------------------------------------------------------------------------
void parseInstrSHLA( uint32_t *instr, uint32_t flags ) {
    
    DrvExpr rExpr;
   
    if ( tok -> isTokenTyp( TYP_GREG )) {
        
        setBitField( instr, 9, 4, tok -> tokVal( ));
        tok -> nextToken( );
    }
    else throw ( ERR_EXPECTED_GENERAL_REG );
    
    acceptComma( );
   
    if ( tok -> isTokenTyp( TYP_GREG )) {
        
        setBitField( instr, 27, 4, tok -> tokVal( ));
        tok -> nextToken( );
    }
    else throw ( ERR_EXPECTED_GENERAL_REG );
    
    acceptComma( );
    parseExpr( &rExpr );
   
    if ( rExpr.typ == TYP_GREG ) {
        
        if ( getBit( *instr, 10 )) throw ( ERR_INSTR_MODE_OPT_COMBO );
        else setBitField( instr, 31, 4, tok -> tokVal( ));
    }
    else if ( rExpr.typ == TYP_NUM ) {
        
        if ( getBit( *instr, 11 )) {
            
            if ( ! isInRangeForBitFieldU( rExpr.numVal, 4 )) throw ( ERR_IMM_VAL_RANGE );
        }
        else {
            
            if ( ! isInRangeForBitField( rExpr.numVal, 4 )) throw ( ERR_IMM_VAL_RANGE );
        }
        
        setBitField( instr, 31, 4, rExpr.numVal );
    }
    else throw ( ERR_EXPECTED_NUMERIC );
    
    acceptComma( );
    parseExpr( &rExpr );
    
    if ( rExpr.typ == TYP_NUM ) {
        
        if ( isInRangeForBitFieldU( rExpr.numVal, 2 )) setBitField( instr, 21, 2, rExpr.numVal );
        else throw ( ERR_IMM_VAL_RANGE );
    }
    else throw ( ERR_EXPECTED_NUMERIC );
       
   checkEOS( );
}

//------------------------------------------------------------------------------------------------------------
// The CMR instruction tests register "B" for a condition and if true copies the "A" value to "R".
//
//      CMR "." <cond> <targetReg> "," <regA> "," <regB>
//
//------------------------------------------------------------------------------------------------------------
void parseInstrCMR( uint32_t *instr, uint32_t flags ) {
    
    if ( tok -> isTokenTyp( TYP_GREG )) {
        
        setBitField( instr, 9, 4, tok -> tokVal( ));
        tok -> nextToken( );
    }
    else throw ( ERR_EXPECTED_GENERAL_REG );
    
    acceptComma( );
    
    if ( tok -> isTokenTyp( TYP_GREG )) {
        
        setBitField( instr, 27, 4, tok -> tokVal( ));
        tok -> nextToken( );
    }
    else throw ( ERR_EXPECTED_GENERAL_REG );
    
    acceptComma( );
   
    if ( tok -> isTokenTyp( TYP_GREG )) {
        
        setBitField( instr, 31, 4, tok -> tokVal( ));
        tok -> nextToken( );
    }
    else throw ( ERR_EXPECTED_GENERAL_REG );
    
    checkEOS( );
}

//------------------------------------------------------------------------------------------------------------
// The "LDIL" instruction loads the immediate value encoded in the instruction left shifted into "R". The
// "ADDIL" instruction will add the value encoded in the instruction left shifted to "R". The result is
// in R1.
//
//      LDIL  <targetReg> "," <val>
//      ADDIL <sourceReg> "," <val>
//
//------------------------------------------------------------------------------------------------------------
void parseInstrLDILandADDIL( uint32_t *instr, uint32_t flags ) {
    
    DrvExpr rExpr;
    
    if ( tok -> isTokenTyp( TYP_GREG )) {
        
        setBitField( instr, 9, 4, tok -> tokVal( ));
        tok -> nextToken( );
    }
    else throw ( ERR_EXPECTED_GENERAL_REG );
    
    acceptComma( );
    parseExpr( &rExpr );
    
    if ( rExpr.typ == TYP_NUM ) {

        if ( isInRangeForBitFieldU( rExpr.numVal, 22 )) setImmValU( instr, 31, 22, rExpr.numVal  );
        else throw ( ERR_IMM_VAL_RANGE );
    }
    else throw ( ERR_EXPECTED_NUMERIC );
    
    checkEOS( );
}

//------------------------------------------------------------------------------------------------------------
// The "LDO" instruction computes the address of an operand, and stores the result in "R".
//
//      LDO <targetReg> "," [ <ofs> "," ] "(" <baseReg> ")"
//
//------------------------------------------------------------------------------------------------------------
void parseInstrLDO( uint32_t *instr, uint32_t flags ) {
    
    DrvExpr rExpr;
   
    if ( tok -> isTokenTyp( TYP_GREG )) {
        
        setBitField( instr, 9, 4, tok -> tokVal( ));
        tok -> nextToken( );
    }
    else throw ( ERR_EXPECTED_GENERAL_REG );
    
    acceptComma( );
    parseExpr( &rExpr );
    
    if ( rExpr.typ == TYP_NUM ) {
        
        if ( isInRangeForBitField( rExpr.numVal, 18 )) setImmVal( instr, 27, 18, rExpr.numVal );
        else throw ( ERR_IMM_VAL_RANGE );
        
        parseExpr( &rExpr );
        
        if ( rExpr.typ == TYP_ADR ) setBitField( instr, 31, 4, rExpr.numVal );
        else throw ( ERR_EXPECTED_GENERAL_REG );
    }
    else if ( rExpr.typ == TYP_ADR ) {
        
        setImmVal( instr, 27, 18, 0 );
        setBitField( instr, 31, 4, rExpr.numVal );
    }
    else throw ( ERR_EXPECTED_NUMERIC );
   
    checkEOS( );
}

//------------------------------------------------------------------------------------------------------------
// The "B" and "GATE" instruction represent an instruction offset relative branch. Optionally, there is an
// optional return register. When omitted, R0 is used in the instruction generation.
//
//      B       <offset> [ "," <returnReg> ]
//      GATE    <offset> [ "," <returnReg> ]
//
//------------------------------------------------------------------------------------------------------------
void parseInstrBandGATE( uint32_t *instr, uint32_t flags ) {
    
    DrvExpr rExpr;
    
    parseExpr( &rExpr );
    
    if ( rExpr.typ == TYP_NUM ) {
        
        if ( isInRangeForBitField( rExpr.numVal, 22 )) setImmVal( instr, 31, 22, rExpr.numVal );
        else throw ( ERR_OFFSET_VAL_RANGE );
    }
    else throw ( ERR_EXPECTED_AN_OFFSET_VAL );
    
    if ( tok -> isToken( TOK_COMMA )) {
        
        tok -> nextToken( );
        if ( tok -> isTokenTyp( TYP_GREG )) {
            
            setBitField( instr, 9, 4, tok -> tokVal( ));
            tok -> nextToken( );
        }
        else throw ( ERR_EXPECTED_GENERAL_REG );
    }
    
    checkEOS( );
}

//------------------------------------------------------------------------------------------------------------
// The "BR" instruction is an IA-relative branch with the offset to be added in a general register. There is
// also an optional return register. When omitted, R0 is used in the instruction generation.
//
//      BR "(" <branchReg> ")" [ "," <returnReg> ]
//
//------------------------------------------------------------------------------------------------------------
void parseInstrBR( uint32_t *instr, uint32_t flags ) {
    
    acceptLparen( );
    
    if ( tok -> isTokenTyp( TYP_GREG )) {
        
        setBitField( instr, 31, 4, tok -> tokVal( ) );
        tok -> nextToken( );
    }
    else throw ( ERR_EXPECTED_GENERAL_REG );
    
    acceptRparen( );
   
    if ( tok -> isToken( TOK_COMMA )) {
        
        tok -> nextToken( );
        if ( tok -> isTokenTyp( TYP_GREG )) {
            
            setBitField( instr, 9, 4, tok -> tokVal( ));
            tok -> nextToken( );
        }
        else throw ( ERR_EXPECTED_GENERAL_REG );
    }
    
    checkEOS( );
}

//------------------------------------------------------------------------------------------------------------
// The "BV" is an absolute branch address instruction in the same segment. Optionally, there is an optional
// return register. When omitted, R0 is used in the instruction generation.
//
//      BV "(" <targetAdrReg> ")" [ "," <returnReg> ]
//
//------------------------------------------------------------------------------------------------------------
void parseInstrBV( uint32_t *instr, uint32_t flags ) {
    
    acceptLparen( );
   
    if ( tok -> isTokenTyp( TYP_GREG )) {
        
        setBitField( instr, 31, 4, tok -> tokVal( ));
        tok -> nextToken( );
    }
    else throw ( ERR_EXPECTED_GENERAL_REG );
    
    acceptRparen( );
   
    if ( tok -> isToken( TOK_COMMA )) {
        
        tok -> nextToken( );
        if ( tok -> isTokenTyp( TYP_GREG )) {
            
            setBitField( instr, 31, 4, tok -> tokVal( ));
            tok -> nextToken( );
        }
        else throw ( ERR_EXPECTED_GENERAL_REG );
    }
    
    checkEOS( );
}

//------------------------------------------------------------------------------------------------------------
// The "BE" instruction is an external branch to a segment and a segment relative offset. When the offset
// part is omitted, a zero is used. There is also an optional return register. When omitted, R0 is used in
// the instruction generation.
//
//      BE [ <ofs> ] "(" <segReg> "," <ofsReg> ")" [ "," <retSeg> ]
//
//------------------------------------------------------------------------------------------------------------
void parseInstrBE( uint32_t *instr, uint32_t flags ) {
    
    DrvExpr rExpr;
   
    parseExpr( &rExpr );
    
    if ( rExpr.typ == TYP_NUM ) {
        
        if ( isInRangeForBitField( rExpr.numVal, 22 )) setImmVal( instr, 23, 14, rExpr.numVal );
        else throw ( ERR_IMM_VAL_RANGE );
           
        parseExpr( &rExpr );
    }
    
    if ( rExpr.typ == TYP_EXT_ADR ) {
        
        setBitField( instr, 27, 4, rExpr.sReg );
        setBitField( instr, 31, 4, rExpr.gReg );
    }
    else throw ( ERR_EXPECTED_EXT_ADR );
    
    if ( tok -> isToken( TOK_COMMA )) {
        
        tok -> nextToken( );
        if ( tok -> isTokenTyp( TYP_GREG )) {
            
            setBitField( instr, 9, 4, tok -> tokVal( ));
            tok -> nextToken( );
        }
        else throw ( ERR_EXPECTED_GENERAL_REG );
    }
    
    checkEOS( );
}

//------------------------------------------------------------------------------------------------------------
// The "BVE" instruction forms a logical address by adding general register "a" to base register "b". There
// is also an optional return register. When omitted, R0 is used in the instruction generation.
//
//      BVE [ <offsetReg> ] "(" <baseReg> ")" [ "," <returnReg> ]
//
//------------------------------------------------------------------------------------------------------------
void parseInstrBVE( uint32_t *instr, uint32_t flags ) {
    
    DrvExpr rExpr;
    
    if ( tok -> isTokenTyp( TYP_GREG )) {
        
        setBitField( instr, 27, 4, tok -> tokVal( ) );
        tok -> nextToken( );
    }
    
    parseExpr( &rExpr );
    
    if ( rExpr.typ == TYP_ADR ) {
        
        setBitField( instr, 31, 4, rExpr.numVal );
    }
    else throw ( ERR_EXPECTED_LOGICAL_ADR );
      
    if ( tok -> isToken( TOK_COMMA )) {
        
        tok -> nextToken( );
        if ( tok -> isTokenTyp( TYP_GREG )) {
            
            setBitField( instr, 9, 4, tok -> tokVal( ));
            tok -> nextToken( );
        }
        else throw ( ERR_EXPECTED_GENERAL_REG );
    }
    
    checkEOS( );
}

//------------------------------------------------------------------------------------------------------------
// The "CBR" and "CBRU" compare register "a" and "b" based on the condition and branch if the comparison
// result is true. The condition code is encoded in the instruction option string parsed before.
//
//      CBR  .<cond> <a>, <b>, <ofs>
//      CBRU .<cond> <a>, <b>, <ofs>
//
//------------------------------------------------------------------------------------------------------------
void parseInstrCBRandCBRU( uint32_t *instr, uint32_t flags ) {
    
    DrvExpr rExpr;
    
    if ( tok -> isTokenTyp( TYP_GREG )) {
        
        setBitField( instr, 27, 4, tok -> tokVal( ) );
        tok -> nextToken( );
    }
    
    acceptComma( );
   
    if ( tok -> isTokenTyp( TYP_GREG )) {
        
        setBitField( instr, 31, 4, tok -> tokVal( ));
        tok -> nextToken( );
    }
    else throw ( ERR_EXPECTED_GENERAL_REG );
    
    acceptComma( );
    parseExpr( &rExpr );
    
    if ( rExpr.typ == TYP_NUM ) {
    
        if ( isInRangeForBitField( rExpr.numVal, 16 )) {
            
            setImmVal( instr, 23, 16, rExpr.numVal );
            tok -> nextToken( );
        }
        else throw ( ERR_IMM_VAL_RANGE );
    }
    else throw ( ERR_EXPECTED_AN_OFFSET_VAL );
    
    checkEOS( );
}

//------------------------------------------------------------------------------------------------------------
// "parseInstrLoad" will parse the load instructions family. The workhorse is the "parseLoadStoreOperand"
// routine, which parses the operand. General form:
//
//      <opCode>.<opt> <targetReg>, <sourceOperand>
//
//------------------------------------------------------------------------------------------------------------
void parseInstrLoadAndStore( uint32_t *instr, uint32_t flags ) {
    
    if ( tok -> isTokenTyp( TYP_GREG )) {
        
        setBitField( instr, 9, 4, tok -> tokVal( ));
        tok -> nextToken( );
    }
    else throw ( ERR_EXPECTED_GENERAL_REG );
    
    acceptComma( );
    parseLoadStoreOperand( instr, flags );
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
void parseInstrMR( uint32_t *instr, uint32_t flags ) {
    
    if ( tok -> isTokenTyp( TYP_GREG )) {
        
        uint8_t tRegId = tok -> tokVal( );
        
        tok -> nextToken( );
        acceptComma( );
        
        if ( tok -> isTokenTyp( TYP_GREG )) {
            
            *instr = 0;
            setBitField( instr, 5, 6, OP_OR );
            setBitField( instr, 9, 4, tRegId );
            setBitField( instr, 13, 2, 1 );
            setBitField( instr, 27, 4, 0 );
            setBitField( instr, 31, 4, tok -> tokVal( ));
            tok -> nextToken( );
        }
        else if ( tok -> isTokenTyp( TYP_SREG )) {
          
            setBitField( instr, 31, 3, tok -> tokVal( ));
            setBitField( instr, 9, 4, tRegId );
            tok -> nextToken( );
        }
        else if ( tok -> isTokenTyp( TYP_CREG )) {
           
            setBit( instr, 11 );
            setBitField( instr, 31, 5, tok -> tokVal( ));
            setBitField( instr, 9, 4, tRegId );
            tok -> nextToken( );
        }
    }
    else if ( tok -> isTokenTyp( TYP_SREG )) {
        
        uint8_t tRegId = tok -> tokVal( );
        
        tok -> nextToken( );
        acceptComma( );
       
        if ( tok -> isTokenTyp( TYP_GREG )) {
            
            setBit( instr, 10 );
            setBitField( instr, 31, 3, tRegId );
            setBitField( instr, 9, 4, tok -> tokVal( ));
            tok -> nextToken( );
        }
        else throw ( ERR_INVALID_REG_COMBO );
    }
    else if ( tok -> isTokenTyp( TYP_CREG )) {
        
        uint8_t tRegId = tok -> tokVal( );
        
        tok -> nextToken( );
        acceptComma( );
       
        if ( tok -> isTokenTyp( TYP_GREG )) {
            
            setBit( instr, 10 );
            setBit( instr, 11 );
            setBitField( instr, 31, 5, tRegId );
            setBitField( instr, 9, 4, tok -> tokVal( ) );
            tok -> nextToken( );
        }
        else throw ( ERR_INVALID_REG_COMBO );
    }
    
    checkEOS( );
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
void parseInstrMST( uint32_t *instr, uint32_t flags ) {
    
    DrvExpr rExpr;
    
    if ( tok -> isTokenTyp( TYP_GREG )) {
        
        setBitField( instr, 9, 4, tok -> tokVal( ) );
        tok -> nextToken( );
    }
    else throw ( ERR_EXPECTED_GENERAL_REG );
    
    acceptComma( );
    parseExpr( &rExpr );
    
    if ( rExpr.typ == TYP_GREG ) {
        
        if ( getBitField( *instr, 11, 2 ) == 0 ) {
            
            setBitField( instr, 31, 4, rExpr.numVal );
            tok -> nextToken( );
        }
        else throw ( ERR_INVALID_INSTR_OPT );
    }
    else if ( rExpr.typ == TYP_NUM ) {
        
        if (( getBitField( *instr, 11, 2 ) == 1 ) || ( getBitField( *instr, 11, 2 ) == 2 )) {
            
            if ( isInRangeForBitFieldU( rExpr.numVal, 6 )) setBitField( instr, 31, 6, rExpr.numVal );
            else throw ( ERR_IMM_VAL_RANGE );
        }
        else throw ( ERR_INVALID_INSTR_OPT );
    }
    else throw ( ERR_EXPECTED_NUMERIC );
  
   checkEOS( );
}

//------------------------------------------------------------------------------------------------------------
// The "LDPA" instruction loads a physical address for the logical address. When the segment is explicitly
// used, it must be in the range of SR1 to SR3.
//
//      LDPA <targetReg> ","  <indexReg> "(" [ <segmentReg>, ] <offsetReg > ")"
//
//------------------------------------------------------------------------------------------------------------
void parseInstrLDPA( uint32_t *instr, uint32_t flags ) {
    
    DrvExpr rExpr;
    
    if ( tok -> isTokenTyp( TYP_GREG )) {
        
        setBitField( instr, 9, 4, tok -> tokVal( ) );
        tok -> nextToken( );
    }
    
    acceptComma( );
    
    if ( tok -> isTokenTyp( TYP_GREG )) {
        
        setBitField( instr, 27, 4, rExpr.numVal );
        tok -> nextToken( );
    }
    
    parseLogicalAdr( instr, flags );
}

//------------------------------------------------------------------------------------------------------------
// The "PRB" instruction will test a logical address for the desired read or write access. The "I" bit will
// when cleared use the "A" reg as input, else bit 27 of the instruction.
//
//      PRB [ "." <opt> ] <targetReg> "," "(" [ <segmentReg>, ] <offsetReg > ")" [ "," <argReg> ]
//
//------------------------------------------------------------------------------------------------------------
void parseInstrPRB( uint32_t *instr, uint32_t flags ) {
    
    DrvExpr     rExpr;
   
    if ( tok -> isTokenTyp( TYP_GREG )) {
        
        setBitField( instr, 9, 4, tok -> tokVal( ) );
        tok -> nextToken( );
    }
    
    acceptComma( );
    parseLogicalAdr( instr, flags );
    acceptComma( );
    parseExpr( &rExpr );
   
    if ( getBit( *instr, 11 )) {
        
        if ( rExpr.typ == TYP_NUM ) {
            
            if ( isInRangeForBitFieldU( rExpr.numVal, 1 )) setBit( instr, 27, rExpr.numVal );
        }
        else throw ( ERR_IMM_VAL_RANGE );
    }
    else if ( rExpr.typ == TYP_GREG ) {
        
        setBitField( instr, 27, 4, rExpr.numVal );
    }
    else throw ( ERR_EXPECTED_GENERAL_REG );
        
    checkEOS( );
}

//------------------------------------------------------------------------------------------------------------
// The "ITLB" instruction will insert a new entry in the instruction or data TLB. We use the segment and
// offset register pair for the virtual address to enter.
//
//      ITLB [.<opt>] <tlbInfoReg> "," "(" <segmentReg> "," <offsetReg> ")"
//
//------------------------------------------------------------------------------------------------------------
void parseInstrITLB( uint32_t *instr, uint32_t flags ) {
    
    if ( tok -> isTokenTyp( TYP_GREG )) {
        
        setBitField( instr, 9, 4, tok -> tokVal( ) );
        tok -> nextToken( );
    }
    
    acceptComma( );
    acceptLparen( );
   
    if ( tok -> isTokenTyp( TYP_SREG )) {
        
        setBitField( instr, 27, 4, tok -> tokVal( ));
        tok -> nextToken( );
    }
    else throw ( ERR_EXPECTED_SEGMENT_REG );
    
    acceptComma( );
   
    if ( tok -> isTokenTyp( TYP_GREG )) {
        
        setBitField( instr, 31, 4, tok -> tokVal( ));
        tok -> nextToken( );
    }
    else throw ( ERR_EXPECTED_GENERAL_REG );
    
    acceptRparen( );
    checkEOS( );
}

//------------------------------------------------------------------------------------------------------------
// The "PTLB" instruction removes an entry from the instruction or data TLB. We use a logical address to
// refer to the TLB entry.
//
//      PTLB [ "." <opt> ] [ <indexReg" ] "(" [ <segmentReg>, ] <offsetReg > ")"
//
//------------------------------------------------------------------------------------------------------------
void parseInstrPTLB( uint32_t *instr, uint32_t flags ) {

    if ( tok -> isTokenTyp( TYP_GREG )) {
        
        setBitField( instr, 27, 4, tok -> tokVal( ) );
        tok -> nextToken( );
    }
    
    if ( tok -> isToken( TOK_LPAREN )) parseLogicalAdr( instr, flags );
    else throw ( ERR_EXPECTED_LOGICAL_ADR );
        
    checkEOS( );
}

//------------------------------------------------------------------------------------------------------------
// The "PCA" instruction flushes and / or remove an entry from a data or instruction cache.
//
//      PCA [ "." <opt> ] [ <indexReg" ] "(" [ <segmentReg>, ] <offsetReg > ")"
//
//------------------------------------------------------------------------------------------------------------
void parseInstrPCA( uint32_t *instr, uint32_t flags ) {
    
    if ( tok -> isTokenTyp( TYP_GREG )) {
        
        setBitField( instr, 27, 4, tok -> tokVal( ) );
        tok -> nextToken( );
    }
    
    if ( tok -> isToken( TOK_LPAREN )) parseLogicalAdr( instr, flags );
    else throw ( ERR_EXPECTED_LOGICAL_ADR );
        
    checkEOS( );
}

//------------------------------------------------------------------------------------------------------------
// The "DIAG" instruction is the instruction for invoking special hardware or diagnostic functions.
//
//      DIAG <resultReg> "," <parmRegA> "," <parmRegB> "," <info>
//
//------------------------------------------------------------------------------------------------------------
void parseInstrDIAG( uint32_t *instr, uint32_t flags ) {
    
    DrvExpr rExpr;
    
    if ( tok -> isTokenTyp( TYP_GREG )) {
        
        setBitField( instr, 9, 4, tok -> tokVal( ) );
        tok -> nextToken( );
    }
    
    acceptComma( );
   
    if ( tok -> isTokenTyp( TYP_GREG )) {
        
        setBitField( instr, 27, 4, tok -> tokVal( ) );
        tok -> nextToken( );
    }
    else throw ( ERR_EXPECTED_GENERAL_REG );
    
    acceptComma( );
    
    if ( tok -> isTokenTyp( TYP_GREG )) {
        
        setBitField( instr, 31, 4, tok -> tokVal( ) );
        tok -> nextToken( );
    }
    else throw ( ERR_EXPECTED_GENERAL_REG );
    
    acceptComma( );
    parseExpr( &rExpr );
    
    if ( rExpr.typ == TYP_NUM ) {
            
        if ( isInRangeForBitFieldU( rExpr.numVal, 4 )) {
                
            setBitField( instr, 13, 4, rExpr.numVal );
            tok -> nextToken( );
        }
        else throw ( ERR_IMM_VAL_RANGE );
    }
    else throw ( ERR_EXPECTED_NUMERIC );
       
    checkEOS( );
}

//------------------------------------------------------------------------------------------------------------
// The "RFI" instruction is the return from interrupt method. So far it is only the instruction with no
// further options and arguments.
//
//      RFI
//
//------------------------------------------------------------------------------------------------------------
void parseInstrRFI( uint32_t *instr, uint32_t flags ) {
    
    checkEOS( );
}

//------------------------------------------------------------------------------------------------------------
// The "BRK" instruction will raise a trap passing along two info fields.
//
//      BRK <info1> "," <info2>
//
//------------------------------------------------------------------------------------------------------------
void parseInstrBRK( uint32_t *instr, uint32_t flags ) {
    
    DrvExpr rExpr;
    
    parseExpr( &rExpr );
    
    if ( rExpr.typ == TYP_NUM ) {
        
        if ( isInRangeForBitFieldU( rExpr.numVal, 4 )) setImmValU( instr, 9, 4, rExpr.numVal );
        else throw ( ERR_IMM_VAL_RANGE );
    }
    else throw ( ERR_EXPECTED_NUMERIC );
    
    acceptComma( );
    parseExpr( &rExpr );
    
    if ( rExpr.typ == TYP_NUM ) {
    
        if ( isInRangeForBitFieldU( rExpr.numVal, 16 )) setImmValU( instr, 31, 16, rExpr.numVal );
        else throw ( ERR_IMM_VAL_RANGE );
    }
    else  throw ( ERR_EXPECTED_NUMERIC );
    
    checkEOS( );
}

//------------------------------------------------------------------------------------------------------------
// The "NOP" synthetic instruction emits the "BRK 0,0" instruction. Easy case.
//
//      NOP
//
//------------------------------------------------------------------------------------------------------------
void parseSynthInstrNop( uint32_t *instr, uint32_t flags ) {
    
    *instr = 0x0;
    
    tok -> nextToken( );
    checkEOS( );
}

// ??? add shift and rotate synthetic ops....

//------------------------------------------------------------------------------------------------------------
// "parseLine" will take the input string and parse the line for an instruction. In the simplified case, there
// is only the opCode mnemonic and the argument list. No labels, no comments. For each instruction, there is
// is a routine that parses the instruction specific input.
//
// An instruction starts with the opCode and the optional option qualifiers. For each opCode, the token table
// has an instruction template and some further information about the instruction, which is used to do further
// syntax checking.. For example, mapping the "LDx" instruction to "LDW" is already encoded in the template
// and set in the flags field.
//
// The next step for all instructions is to check for options. Finally, a dedicated parsing routine will
// handle the remainder of the assembly line. As the parsing process comes along the instruction template
// from the token name table will be augmented with further data. If all is successful, we will have the
// final instruction bit pattern.
//
//------------------------------------------------------------------------------------------------------------
void parseLine( char *inputStr, uint32_t *instr ) {
    
    uint32_t    flags   = 0;
    TokId       opCode  = TOK_NIL;
    
    tok -> setupTokenizer( inputStr, (DrvToken *) asmTokTab );
    tok -> nextToken( );
    
    if ( tok -> isTokenTyp( TYP_OP_CODE )) {
        
        flags   = 0;
        opCode  = tok -> tokId( );
        *instr  = tok -> tokVal( );
       
        tok -> nextToken( );
        while ( tok -> isToken( TOK_PERIOD )) {
            
            tok -> nextToken( );
            parseInstrOptions( instr, &flags );
        }
        
        switch( opCode ) {
                
            case OP_CODE_ADD:   case OP_CODE_ADDW:
            case OP_CODE_ADC:   case OP_CODE_ADCW:
            case OP_CODE_SUB:   case OP_CODE_SUBW:
            case OP_CODE_SBC:   case OP_CODE_SBCW:
            case OP_CODE_AND:   case OP_CODE_ANDW:
            case OP_CODE_OR:    case OP_CODE_ORW:
            case OP_CODE_XOR:   case OP_CODE_XORW:
            case OP_CODE_CMP:   case OP_CODE_CMPW:
            case OP_CODE_CMPU:  case OP_CODE_CMPUW: {
                
                return( parseModeTypeInstr( instr, flags | TF_WORD_INSTR ));
            }
                
            case OP_CODE_ADDB:  case OP_CODE_ADCB:  case OP_CODE_SUBB:  case OP_CODE_SBCB:
            case OP_CODE_ANDB:  case OP_CODE_ORB:   case OP_CODE_XORB:  case OP_CODE_CMPB:
            case OP_CODE_CMPUB: {
                
                return( parseModeTypeInstr( instr, flags | TF_BYTE_INSTR ));
            }
                
            case OP_CODE_ADDH:  case OP_CODE_ADCH:  case OP_CODE_SUBH:  case OP_CODE_SBCH:
            case OP_CODE_ANDH:  case OP_CODE_ORH:   case OP_CODE_XORH:  case OP_CODE_CMPH:
            case OP_CODE_CMPUH: {
                
                return( parseModeTypeInstr( instr, flags | TF_HALF_INSTR ));
            }
                
            case OP_CODE_LD:    case OP_CODE_LDW:   case OP_CODE_LDA:   case OP_CODE_LDR:
            case OP_CODE_ST:    case OP_CODE_STW:   case OP_CODE_STA:   case OP_CODE_STC: {
                
                return( parseInstrLoadAndStore( instr, flags | TF_WORD_INSTR ));
            }
            
            case OP_CODE_STB:   case OP_CODE_LDB: {
                
                return( parseInstrLoadAndStore( instr, flags | TF_BYTE_INSTR ));
            }
            
            case OP_CODE_LDH:   case OP_CODE_STH: {
                
                return( parseInstrLoadAndStore( instr, flags | TF_HALF_INSTR ));
            }
            
            case OP_CODE_LSID:      return( parseInstrLSID( instr, flags ));
            case OP_CODE_EXTR:      return( parseInstrEXTR( instr, flags ));
            case OP_CODE_DEP:       return( parseInstrDEP( instr, flags ));
                
            case OP_CODE_DS:        return( parseInstrDS( instr, flags ));
                
            case OP_CODE_DSR:       return( parseInstrDSR( instr, flags ));
            case OP_CODE_SHLA:      return( parseInstrSHLA( instr, flags ));
            case OP_CODE_CMR:       return( parseInstrCMR( instr, flags ));
                
                
            case OP_CODE_LDIL:
            case OP_CODE_ADDIL:     return( parseInstrLDILandADDIL( instr, flags ));
                
            case OP_CODE_LDO:       return( parseInstrLDO( instr, flags ));
                
            case OP_CODE_B:
            case OP_CODE_GATE:      return( parseInstrBandGATE( instr, flags ));
                
            case OP_CODE_BR:        return( parseInstrBR( instr, flags ));
            case OP_CODE_BV:        return( parseInstrBV( instr, flags ));
            case OP_CODE_BE:        return( parseInstrBE( instr, flags ));
            case OP_CODE_BVE:       return( parseInstrBVE( instr, flags ));
                
            case OP_CODE_CBR:
            case OP_CODE_CBRU:      return( parseInstrCBRandCBRU( instr, flags ));
                
            case OP_CODE_MR:        return( parseInstrMR( instr, flags ));
            case OP_CODE_MST:       return( parseInstrMST( instr, flags ));
            case OP_CODE_LDPA:      return( parseInstrLDPA( instr, flags ));
            case OP_CODE_PRB:       return( parseInstrPRB( instr, flags ));
            case OP_CODE_ITLB:      return( parseInstrITLB( instr, flags ));
            case OP_CODE_PTLB:      return( parseInstrPTLB( instr, flags ));
            case OP_CODE_PCA:       return( parseInstrPCA( instr, flags ));
            case OP_CODE_DIAG:      return( parseInstrDIAG( instr, flags ));
            case OP_CODE_RFI:       return( parseInstrRFI( instr, flags ));
            case OP_CODE_BRK:       return( parseInstrBRK( instr, flags ));
              
            default: throw ( ERR_INVALID_OP_CODE );
        }
    }
    else if ( tok -> isTokenTyp( TYP_OP_CODE_S )) {
        
        flags   = 0;
        opCode  = tok -> tokId( );
        *instr  = 0;
       
        switch ( opCode ) {
                
            case OP_CODE_S_NOP: return( parseSynthInstrNop( instr, flags ));
            
            default: throw ( ERR_INVALID_S_OP_CODE );
        }
    }
    else throw ( ERR_INVALID_OP_CODE );
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
    
    tok = new DrvTokenizer( glb );
}

ErrMsgId DrvOneLineAsm::parseAsmLine( char *inputStr, uint32_t *instr ) {
    
    try {
        
        char tmpBuf[ CMD_LINE_BUF_SIZE ];
        strcpy( tmpBuf, inputStr );
        upshiftStr( tmpBuf );
        parseLine( tmpBuf, instr );
        return( NO_ERR );
    }
    catch ( ErrMsgId errNum ) {
        
        *instr = 0;
        markAsmError( errNum );
        return( errNum );
    }
}
