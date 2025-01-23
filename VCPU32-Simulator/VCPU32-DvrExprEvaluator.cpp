//------------------------------------------------------------------------------------------------------------
//
// VCPU32 - A 32-bit CPU - Simulator expressions
//
//------------------------------------------------------------------------------------------------------------
// ...
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
// Idea:
//
// It turns out that a better command line parser would be a more powerful way to analyze a command line.
// We have commands that just execute a command and functions that return a value. When we have a parser
// we could implement such functions as arguments to the commands. Commands them selves may just be just a
// function with a void return.
//
//      <command>   ->  <cmdId> [ <argList> ]
//      <function>  ->  <funcId> “(“ [ <argList> ] ")"
//      <argList>   ->  <expr> { <expr> }
//
// Expression have a type, which are NUM, ADR, STR, SREG, GREG and CREG.
//
//      <factor> -> <number>                        |
//                  <extAdr>                        |
//                  <string>                        |
//                  <envId>                         |
//                  <gregId>                        |
//                  <sregId>                        |
//                  <cregId>                        |
//                  "~" <factor>                    |
//                  "(" <expr> ")"
//
//      <term>      ->  <factor> { <termOp> <factor> }
//      <termOp>    ->  "*" | "/" | "%" | "&"
//
//      <expr>      ->  [ ( "+" | "-" ) ] <term> { <exprOp> <term> }
//      <exprOp>    ->  "+" | "-" | "|" | "^"
//
// If a command is called, there is no output another than what the command was issuing itself.
// If a function is called in the command place, the function result will be printed.
// If an argument represents a function, its return value will be the argument in the command.
//
// The token table becomes a kind of dictionary with name, type and values.
// The environment table needs to enhanced to allow for user defined variables.
//
//------------------------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------------------------
// Local name space. We try to keep utility functions local to the file.
//
//------------------------------------------------------------------------------------------------------------
namespace {

enum logicalOpId : int {
    
    AND_OP  = 0,
    OR_OP   = 1,
    XOR_OP  = 2
};

//------------------------------------------------------------------------------------------------------------
//
//
//------------------------------------------------------------------------------------------------------------
void addOp( DrvExpr *rExpr, DrvExpr *lExpr ) {
    
    switch ( rExpr -> typ ) {
            
        case TYP_NUM: {
            
            switch ( lExpr -> typ ) {
                    
                case TYP_NUM: rExpr -> numVal += lExpr -> numVal; break;
                
                default: throw ( ERR_EXPR_TYPE_MATCH );
            }
            
        } break;
            
        case TYP_EXT_ADR: {
            
            switch ( lExpr -> typ ) {
                    
                case TYP_NUM: rExpr -> ofs += lExpr -> numVal; break;
                
                default: throw ( ERR_EXPR_TYPE_MATCH );
            }
            
        } break;
            
        default: throw ( ERR_EXPR_TYPE_MATCH );
    }
}

//------------------------------------------------------------------------------------------------------------
//
//
//------------------------------------------------------------------------------------------------------------
void subOp( DrvExpr *rExpr, DrvExpr *lExpr ) {
    
    switch ( rExpr -> typ ) {
            
        case TYP_NUM: {
            
            switch ( lExpr -> typ ) {
                    
                case TYP_NUM: rExpr -> numVal -= lExpr -> numVal; break;
                
                default: throw ( ERR_EXPR_TYPE_MATCH );
            }
            
        } break;
            
        case TYP_EXT_ADR: {
            
            switch ( lExpr -> typ ) {
                    
                case TYP_NUM: rExpr -> ofs -= lExpr -> numVal; break;
                
                default: throw ( ERR_EXPR_TYPE_MATCH );
            }
            
        } break;
            
        default: throw ( ERR_EXPR_TYPE_MATCH );
    }
}

//------------------------------------------------------------------------------------------------------------
//
//
//------------------------------------------------------------------------------------------------------------
void multOp( DrvExpr *rExpr, DrvExpr *lExpr ) {
    
    switch ( rExpr -> typ ) {
            
        case TYP_NUM: {
            
            switch ( lExpr -> typ ) {
                    
                case TYP_NUM: rExpr -> numVal *= lExpr -> numVal; break;
                    
                default: throw ( ERR_EXPR_TYPE_MATCH );
            }
            
        } break;
            
        case TYP_EXT_ADR: {
            
            switch ( lExpr -> typ ) {
                    
                case TYP_NUM: rExpr -> ofs *= lExpr -> numVal; break;
                
                default: throw ( ERR_EXPR_TYPE_MATCH );
            }
            
        } break;
            
        default: throw ( ERR_EXPR_TYPE_MATCH );
    }
}

//------------------------------------------------------------------------------------------------------------
//
//
//------------------------------------------------------------------------------------------------------------
void divOp( DrvExpr *rExpr, DrvExpr *lExpr ) {
    
    switch ( rExpr -> typ ) {
            
        case TYP_NUM: {
            
            switch ( lExpr -> typ ) {
                    
                case TYP_NUM: rExpr -> numVal /= lExpr -> numVal; break;
                
                default: throw ( ERR_EXPR_TYPE_MATCH );
            }
            
        } break;
            
        case TYP_EXT_ADR: {
            
            switch ( lExpr -> typ ) {
                    
                case TYP_NUM: rExpr -> ofs /= lExpr -> numVal; break;
                
                default: throw ( ERR_EXPR_TYPE_MATCH );
            }
            
        } break;
            
        default: throw ( ERR_EXPR_TYPE_MATCH );
    }
}

//------------------------------------------------------------------------------------------------------------
//
//
//------------------------------------------------------------------------------------------------------------
void modOp( DrvExpr *rExpr, DrvExpr *lExpr ) {
    
    switch ( rExpr -> typ ) {
            
        case TYP_NUM: {
            
            switch ( lExpr -> typ ) {
                    
                case TYP_NUM: rExpr -> numVal %= lExpr -> numVal; break;
                
                default: throw ( ERR_EXPR_TYPE_MATCH );
            }
            
        } break;
            
        case TYP_EXT_ADR: {
            
            switch ( lExpr -> typ ) {
                    
                case TYP_NUM: rExpr -> ofs %= lExpr -> numVal; break;
                
                default: throw ( ERR_EXPR_TYPE_MATCH );
            }
            
        } break;
            
        default: throw ( ERR_EXPR_TYPE_MATCH );
    }
}

//------------------------------------------------------------------------------------------------------------
//
//
//------------------------------------------------------------------------------------------------------------
void logicalOp( DrvExpr *rExpr, DrvExpr *lExpr, logicalOpId op ) {
    
    switch ( rExpr -> typ ) {
            
        case TYP_BOOL: {
            
            if ( lExpr -> typ == TYP_BOOL ) {
                
                switch ( op ) {
                        
                    case AND_OP:    rExpr -> bVal &= lExpr -> bVal; break;
                    case OR_OP:     rExpr -> bVal |= lExpr -> bVal; break;
                    case XOR_OP:    rExpr -> bVal ^= lExpr -> bVal; break;
                }
            }
            else throw ( ERR_EXPR_TYPE_MATCH );
            
        } break;
            
        case TYP_NUM: {
            
            switch ( lExpr -> typ ) {
                    
                case TYP_NUM: {
                    
                    switch ( op ) {
                            
                        case AND_OP:    rExpr -> numVal &= lExpr -> numVal; break;
                        case OR_OP:     rExpr -> numVal |= lExpr -> numVal; break;
                        case XOR_OP:    rExpr -> numVal ^= lExpr -> numVal; break;
                    }
                    
                } break;
                
                default: throw ( ERR_EXPR_TYPE_MATCH );
            }
            
        } break;
            
        default: throw ( ERR_EXPR_TYPE_MATCH );
    }
}

}; // namespace


//------------------------------------------------------------------------------------------------------------
// Evaluation Expression Object constructor.
//
//------------------------------------------------------------------------------------------------------------
DrvExprEvaluator::DrvExprEvaluator( VCPU32Globals *glb ) {
    
    this -> glb = glb;
}

//------------------------------------------------------------------------------------------------------------
// Coercin functions. Not a lot there yet. The idea is to coerce an expression into a 32-bit value where
// possible. There are signed and unisgned versions, which at the moment identical. We only have 32-bit
// values. If we have one day 16-bit and 64-bit vaous in addition, there is more to do. What we also coerce
// is the first characters of a string, right justified if shorter than 4 bytes.
//
//------------------------------------------------------------------------------------------------------------
void DrvExprEvaluator::pFuncS32( DrvExpr *rExpr ) {
    
    DrvExpr     lExpr;
    uint32_t    res = 0;
     
    glb -> tok -> nextToken( );
    if ( glb -> tok -> isToken( TOK_LPAREN )) glb -> tok -> nextToken( );
    else throw ( ERR_EXPECTED_LPAREN );
        
    parseExpr( &lExpr );
    if ( lExpr.typ == TYP_NUM ) {
        
        res = lExpr.numVal;
    }
    else if ( lExpr.typ == TYP_STR ) {
        
        if ( strlen( lExpr.strVal ) > 3 ) res = res | ( lExpr.strVal[ 3 ] << 0 );
        if ( strlen( lExpr.strVal ) > 2 ) res = res | ( lExpr.strVal[ 2 ] << 8 );
        if ( strlen( lExpr.strVal ) > 1 ) res = res | ( lExpr.strVal[ 1 ] << 16 );
        if ( strlen( lExpr.strVal ) > 0 ) res = res | ( lExpr.strVal[ 0 ] << 24 );
        
        // ??? shift right aligned ?
    }
    else throw ( ERR_EXPECTED_EXPR );
    
    rExpr -> typ    = TYP_NUM;
    rExpr -> numVal = res;

    if ( glb -> tok -> isToken( TOK_RPAREN )) glb -> tok -> nextToken( );
    else throw ( ERR_EXPECTED_RPAREN );
}

void DrvExprEvaluator::pFuncU32( DrvExpr *rExpr ) {
    
    DrvExpr     lExpr;
    uint32_t    res = 0;
     
    glb -> tok -> nextToken( );
    if ( glb -> tok -> isToken( TOK_LPAREN )) glb -> tok -> nextToken( );
    else throw ( ERR_EXPECTED_LPAREN );
        
    parseExpr( &lExpr );
    if ( lExpr.typ == TYP_NUM ) {
        
        res = lExpr.numVal;
    }
    else if ( lExpr.typ == TYP_STR ) {
        
        if ( strlen( lExpr.strVal ) > 3 ) res = res | ( lExpr.strVal[ 3 ] << 0 );
        if ( strlen( lExpr.strVal ) > 2 ) res = res | ( lExpr.strVal[ 2 ] << 8 );
        if ( strlen( lExpr.strVal ) > 1 ) res = res | ( lExpr.strVal[ 1 ] << 16 );
        if ( strlen( lExpr.strVal ) > 0 ) res = res | ( lExpr.strVal[ 0 ] << 24 );
    }
    else throw ( ERR_EXPECTED_EXPR );
    
    rExpr -> typ    = TYP_NUM;
    rExpr -> numVal = res;

    if ( glb -> tok -> isToken( TOK_RPAREN )) glb -> tok -> nextToken( );
    else throw ( ERR_EXPECTED_RPAREN );
}

//------------------------------------------------------------------------------------------------------------
// Assemble function.
//
// ASSEMBLE "(" <str> ")"
//------------------------------------------------------------------------------------------------------------
void DrvExprEvaluator::pFuncAssemble( DrvExpr *rExpr ) {
    
    DrvExpr     lExpr;
    uint32_t    instr;
    ErrMsgId    ret;
    
    glb -> tok -> nextToken( );
    if ( glb -> tok -> isToken( TOK_LPAREN )) glb -> tok -> nextToken( );
    else throw ( ERR_EXPECTED_LPAREN );
        
    parseExpr( &lExpr );
    if ( lExpr.typ == TYP_STR ) {
        
        ret = glb -> oneLineAsm -> parseAsmLine( lExpr.strVal, &instr );
        
        if ( ret == NO_ERR ) {
            
            rExpr -> typ    = TYP_NUM;
            rExpr -> numVal = instr;
        }
        else throw ( ret );
    }
    else throw ( ERR_EXPECTED_STR );

    if ( glb -> tok -> isToken( TOK_RPAREN )) glb -> tok -> nextToken( );
    else throw ( ERR_EXPECTED_RPAREN );
}

//------------------------------------------------------------------------------------------------------------
// Dis-assemble function.
//
// DISASSEMBLE "(" <str> [ "," <rdx> ] ")"
//------------------------------------------------------------------------------------------------------------
void DrvExprEvaluator::pFuncDisAssemble( DrvExpr *rExpr ) {
    
    DrvExpr     lExpr;
    uint32_t    instr;
    char        asmStr[ CMD_LINE_BUF_SIZE ];
    int         rdx = glb -> env -> getEnvVarInt((char *) ENV_RDX_DEFAULT );
    
    glb -> tok -> nextToken( );
    if ( glb -> tok -> isToken( TOK_LPAREN )) glb -> tok -> nextToken( );
    else throw ( ERR_EXPECTED_LPAREN );
    
    glb -> eval -> parseExpr( &lExpr );
    
    if ( lExpr.typ == TYP_NUM ) {
        
        instr = lExpr.numVal;
        
        if ( glb -> tok -> tokId( ) == TOK_COMMA ) {
            
            glb -> tok -> nextToken( );
            
            if (( glb -> tok -> tokId( ) == TOK_HEX ) ||
                ( glb -> tok -> tokId( ) == TOK_OCT ) ||
                ( glb -> tok -> tokId( ) == TOK_DEC )) {
                
                rdx = glb -> tok -> tokVal( );
                
                glb -> tok -> nextToken( );
            }
            else if ( glb -> tok -> tokId( ) == TOK_EOS ) {
                
                throw ( ERR_UNEXPECTED_EOS );
            }
            else throw ( ERR_INVALID_FMT_OPT );
        }
        
        if ( glb -> tok -> isToken( TOK_RPAREN )) glb -> tok -> nextToken( );
        else throw ( ERR_EXPECTED_RPAREN );
        
        glb -> disAsm -> formatInstr( asmStr, sizeof( asmStr ), instr );
        
        rExpr -> typ = TYP_STR;
        strcpy( rExpr -> strVal, asmStr );
    }
    else throw ( ERR_EXPECTED_INSTR_VAL );
}

//------------------------------------------------------------------------------------------------------------
// Virtual address hash function.
//
// HASH "(" <extAdr> ")"
//------------------------------------------------------------------------------------------------------------
void DrvExprEvaluator::pFuncHash( DrvExpr *rExpr ) {
    
    DrvExpr     lExpr;
    uint32_t    hashVal;
    
    glb -> tok -> nextToken( );
    if ( glb -> tok -> isToken( TOK_LPAREN )) glb -> tok -> nextToken( );
    else throw ( ERR_EXPECTED_LPAREN );
        
    parseExpr( &lExpr );
    if ( lExpr.typ == TYP_EXT_ADR ) {
        
        hashVal = glb -> cpu -> iTlb -> hashAdr( lExpr.seg, lExpr.ofs );
        
        rExpr -> typ    = TYP_NUM;
        rExpr -> numVal = hashVal;
    }
    else throw ( ERR_EXPECTED_STR );

    if ( glb -> tok -> isToken( TOK_RPAREN )) glb -> tok -> nextToken( );
    else throw ( ERR_EXPECTED_RPAREN );
}

//------------------------------------------------------------------------------------------------------------
// Virtual address function. The portions <seg> and <expr> can be numeric values or the respective register
// content. When we only have <expr>, the segment portion is derived from the upper two bits of the offset.
//
// ADR "(" <seg> "," <expr> ")"
// ADR "(" <expr> "," <expr> ")"
// ADR "(" <ofs> ")"
//------------------------------------------------------------------------------------------------------------
void DrvExprEvaluator::pFuncExtAdr( DrvExpr *rExpr ) {
    
    DrvExpr     lExpr;
    uint32_t    seg;
    
    glb -> tok -> nextToken( );
    if ( glb -> tok -> isToken( TOK_LPAREN )) glb -> tok -> nextToken( );
    else throw ( ERR_EXPECTED_LPAREN );
    
    if ( glb -> tok -> isTokenTyp( TYP_SREG )) {
        
        seg = glb -> cpu -> getReg( RC_SEG_REG_SET, glb -> tok -> tokVal( ));
        
        glb -> tok -> nextToken( );
        if ( ! glb -> tok -> isToken( TOK_COMMA )) throw ( ERR_EXPECTED_COMMA );
        else glb -> tok -> nextToken( );
        
        parseExpr( &lExpr );
        
        if ( lExpr.typ == TYP_NUM ) {
            
            rExpr -> typ = TYP_EXT_ADR;
            rExpr -> seg = seg;
            rExpr -> ofs = lExpr.numVal;
            
            if ( glb -> tok -> isToken( TOK_RPAREN )) glb -> tok -> nextToken( );
            else throw ( ERR_EXPECTED_RPAREN );
        }
        else throw ( ERR_EXPECTED_OFS );
    }
    else {
        
        parseExpr( &lExpr );
        
        if ( lExpr.typ == TYP_NUM ) {
            
            uint32_t segId = lExpr.numVal >> 30;
            if ( segId == 0 ) segId += 4;
            
            rExpr -> typ = TYP_EXT_ADR;
            rExpr -> seg = glb -> cpu -> getReg( RC_SEG_REG_SET, segId );
            rExpr -> ofs = lExpr.numVal;
            
            if ( glb -> tok -> isToken( TOK_RPAREN )) glb -> tok -> nextToken( );
            else throw ( ERR_EXPECTED_RPAREN );
        }
        else if ( lExpr.typ == TYP_EXT_ADR ) {
            
            rExpr -> typ = TYP_EXT_ADR;
            rExpr -> seg = lExpr.seg;
            rExpr -> ofs = lExpr.ofs;
            
            if ( glb -> tok -> isToken( TOK_RPAREN )) glb -> tok -> nextToken( );
            else throw ( ERR_EXPECTED_RPAREN );
        }
        else throw( ERR_INVALID_EXPR );
    }
}

//------------------------------------------------------------------------------------------------------------
// Entry point to the predefined functions. We dispatch based on the predefined function token Id.
//
//------------------------------------------------------------------------------------------------------------
void DrvExprEvaluator::parsePredefinedFunction( DrvToken funcId, DrvExpr *rExpr ) {
    
    switch( funcId.tid ) {
            
        case PF_ASSEMBLE:       pFuncAssemble( rExpr );     break;
        case PF_DIS_ASSEMBLE:   pFuncDisAssemble( rExpr );  break;
        case PF_HASH:           pFuncHash( rExpr );         break;
        case PF_EXT_ADR:        pFuncExtAdr( rExpr );       break;
        case PF_S32:            pFuncS32( rExpr );          break;
        case PF_U32:            pFuncU32( rExpr );          break;
            
        default: throw ( ERR_UNDEFINED_PFUNC );
    }
}

//------------------------------------------------------------------------------------------------------------
// "parseFactor" parses the factor syntax part of an expression.
//
//      <factor> -> <number>                        |
//                  <extAdr>                        |
//                  <gregId>                        |
//                  <sregId>                        |
//                  <cregId>                        |
//                  "~" <factor>                    |
//                  "(" [ <sreg> "," ] <greg> ")"   |
//                  "(" <expr> ")"
//
//------------------------------------------------------------------------------------------------------------
void DrvExprEvaluator::parseFactor( DrvExpr *rExpr ) {
    
    rExpr -> typ       = TYP_NIL;
    rExpr -> numVal    = 0;
    
    if ( glb -> tok -> isTokenTyp( TYP_NUM ))  {
        
        rExpr -> typ     = TYP_NUM;
        rExpr -> numVal  = glb -> tok -> tokVal( );
        glb -> tok -> nextToken( );
    }
    else if ( glb -> tok -> isTokenTyp( TYP_EXT_ADR )) {
        
        rExpr -> typ    = TYP_EXT_ADR;
        rExpr -> seg    = glb -> tok -> tokSeg( );
        rExpr -> ofs    = glb -> tok -> tokOfs( );
        glb -> tok -> nextToken( );
    }
    else if ( glb -> tok -> isTokenTyp( TYP_STR ))  {
        
        rExpr -> typ = TYP_STR;
        strcpy( rExpr -> strVal, glb -> tok -> tokStr( ));
        glb -> tok -> nextToken( );
    }
    else if ( glb -> tok -> isTokenTyp( TYP_GREG ))  {
        
        rExpr -> typ    = TYP_NUM;
        rExpr -> numVal = glb -> cpu -> getReg( RC_GEN_REG_SET, glb -> tok -> tokVal( ));
        glb -> tok -> nextToken( );
    }
    else if ( glb -> tok -> isTokenTyp( TYP_SREG ))  {
        
        rExpr -> typ    = TYP_SREG;
        rExpr -> numVal = glb -> cpu -> getReg( RC_SEG_REG_SET, glb -> tok -> tokVal( ));
        glb -> tok -> nextToken( );
    }
    else if ( glb -> tok -> isTokenTyp( TYP_CREG ))  {
        
        rExpr -> typ    = TYP_CREG;
        rExpr -> numVal = glb -> cpu -> getReg( RC_CTRL_REG_SET, glb -> tok -> tokVal( ));
        glb -> tok -> nextToken( );
    }
    else if ( glb -> tok -> isTokenTyp( TYP_PREDEFINED_FUNC )) {
        
        parsePredefinedFunction( glb -> tok -> token( ), rExpr );
    }
    else if ( glb -> tok -> isToken( TOK_IDENT )) {
        
        DrvEnvTabEntry *entry = glb -> env -> getEnvVarEntry ( glb -> tok -> tokStr( ));
        
        if ( entry != nullptr ) {
            
            rExpr -> typ = entry -> typ;
            
            switch( rExpr -> typ ) {
                    
                case TYP_BOOL:  rExpr -> bVal       =  entry -> bVal;           break;
                case TYP_NUM:   rExpr -> numVal     =  entry -> iVal;           break;
                case TYP_ADR:   rExpr -> adr        =  entry -> uVal;           break;
                case TYP_STR:   strcpy( rExpr -> strVal, entry -> strVal );     break;
                
                case TYP_EXT_ADR: {
                    
                    rExpr -> seg = entry -> seg;
                    rExpr -> ofs = entry -> ofs;
                    
                } break;
                
                default: fprintf( stdout, "**** uncaptured type in factor, fix ... \n" );
            }
        }
        else throw( ERR_ENV_VAR_NOT_FOUND );
        
        glb -> tok -> nextToken( );
    }
    else if ( glb -> tok -> isToken( TOK_NEG )) {
        
        glb -> tok -> nextToken( );
        parseFactor( rExpr );
        rExpr -> numVal = ~ rExpr -> numVal;
    }
    else if ( glb -> tok -> isToken( TOK_LPAREN )) {
        
        glb -> tok -> nextToken( );
        parseExpr( rExpr );
            
        if ( glb -> tok -> isToken( TOK_RPAREN )) glb -> tok -> nextToken( );
        else throw ( ERR_EXPECTED_RPAREN );
    }
    else if (( glb -> tok -> tokTyp( ) == TYP_NIL ) && ( glb -> tok -> tokId( ) == TOK_EOS )) {
        
        rExpr -> typ = TYP_NIL;
    }
    else throw ( ERR_EXPR_FACTOR );
}

//------------------------------------------------------------------------------------------------------------
// "parseTerm" parses the term syntax.
//
//      <term>      ->  <factor> { <termOp> <factor> }
//      <termOp>    ->  "*" | "/" | "%" | "&"
//
// ??? type mix options ?
//------------------------------------------------------------------------------------------------------------
void DrvExprEvaluator::parseTerm( DrvExpr *rExpr ) {
    
    DrvExpr lExpr;
   
    parseFactor( rExpr );
    
    while (( glb -> tok -> tokId( ) == TOK_MULT )   ||
           ( glb -> tok -> tokId( ) == TOK_DIV  )   ||
           ( glb -> tok -> tokId( ) == TOK_MOD  )   ||
           ( glb -> tok -> tokId( ) == TOK_AND  ))  {
        
        uint8_t op = glb -> tok -> tokId( );
        
        glb -> tok -> nextToken( );
        parseFactor( &lExpr );
        
        if ( lExpr.typ == TYP_NIL ) throw ( ERR_UNEXPECTED_EOS );
        
        switch( op ) {
                
            case TOK_MULT:   multOp( rExpr, &lExpr );               break;
            case TOK_DIV:    divOp( rExpr, &lExpr );                break;
            case TOK_MOD:    modOp( rExpr, &lExpr );                break;
            case TOK_AND:    logicalOp( rExpr, &lExpr, AND_OP );    break;
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
// ??? type mix options ?
//------------------------------------------------------------------------------------------------------------
void DrvExprEvaluator::parseExpr( DrvExpr *rExpr ) {
    
    DrvExpr lExpr;
    
    if ( glb -> tok -> isToken( TOK_PLUS )) {
        
        glb -> tok -> nextToken( );
        parseTerm( rExpr );
        
        if ( rExpr -> typ != TYP_NUM ) throw ( ERR_EXPECTED_NUMERIC );
    }
    else if ( glb -> tok -> isToken( TOK_MINUS )) {
        
        glb -> tok -> nextToken( );
        parseTerm( rExpr );
        
        if ( rExpr -> typ == TYP_NUM ) rExpr -> numVal = - (int32_t) rExpr -> numVal;
        else throw ( ERR_EXPECTED_NUMERIC );
    }
    else parseTerm( rExpr );
    
    while (( glb -> tok -> isToken( TOK_PLUS   )) ||
           ( glb -> tok -> isToken( TOK_MINUS  )) ||
           ( glb -> tok -> isToken( TOK_OR     )) ||
           ( glb -> tok -> isToken( TOK_XOR    ))) {
        
        uint8_t op = glb -> tok -> tokId( );
        
        glb -> tok -> nextToken( );
        parseTerm( &lExpr );
        
        if ( lExpr.typ == TYP_NIL ) throw ( ERR_UNEXPECTED_EOS );
        
        switch ( op ) {
                
            case TOK_PLUS:   addOp( rExpr, &lExpr );                break;
            case TOK_MINUS:  subOp( rExpr, &lExpr );                break;
            case TOK_OR:     logicalOp( rExpr, &lExpr, OR_OP );     break;
            case TOK_XOR:    logicalOp( rExpr, &lExpr, XOR_OP );    break;
        }
    }
}

