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
//                  "(" [ <sreg> "," ] <greg> ")"   |
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

}; // namespace


//------------------------------------------------------------------------------------------------------------
//
//
//
//------------------------------------------------------------------------------------------------------------
DrvExprEvaluator::DrvExprEvaluator( VCPU32Globals *glb ) {
    
    this -> glb = glb;
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
ErrMsgId DrvExprEvaluator::parseFactor( DrvExpr *rExpr ) {
    
    rExpr -> typ       = TYP_NIL;
    rExpr -> numVal    = 0;
    
    if ( glb -> tok -> isTokenTyp( TYP_SYM ))  {
        
        rExpr -> typ    = glb -> tok -> tokTyp( );
        rExpr -> tokId  = glb -> tok -> tokId( );
        glb -> tok -> nextToken( );
        return( NO_ERR );
    }
    else if ( glb -> tok -> isTokenTyp( TYP_CMD ))  {
        
        rExpr -> typ    = TYP_CMD;
        rExpr -> tokId  = glb -> tok -> tokId( );
        glb -> tok -> nextToken( );
        return( NO_ERR );
    }
    else if ( glb -> tok -> isTokenTyp( TYP_NUM ))  {
        
        rExpr -> typ     = TYP_NUM;
        rExpr -> numVal  = glb -> tok -> tokVal( );
        glb -> tok -> nextToken( );
        return( NO_ERR );
    }
    else if ( glb -> tok -> isTokenTyp( TYP_EXT_ADR )) {
        
        rExpr -> typ    = TYP_EXT_ADR;
        rExpr -> seg    = glb -> tok -> tokSeg( );
        rExpr -> ofs    = glb -> tok -> tokOfs( );
        return( NO_ERR );
    }
    else if ( glb -> tok -> isTokenTyp( TYP_STR ))  {
        
        rExpr -> typ = TYP_STR;
        strcpy( rExpr -> strVal, glb -> tok -> tokStr( ));
        glb -> tok -> nextToken( );
        return( NO_ERR );
    }
    else if ( glb -> tok -> isTokenTyp( TYP_GREG ))  {
        
        rExpr -> typ     = TYP_GREG;
        rExpr -> numVal = glb -> cpu -> getReg( RC_GEN_REG_SET, glb -> tok -> tokVal( ));
        glb -> tok -> nextToken( );
        return( NO_ERR );
    }
    else if ( glb -> tok -> isTokenTyp( TYP_SREG ))  {
        
        rExpr -> typ = TYP_SREG;
        rExpr -> numVal = glb -> cpu -> getReg( RC_SEG_REG_SET, glb -> tok -> tokVal( ));
        glb -> tok -> nextToken( );
        return( NO_ERR );
    }
    else if ( glb -> tok -> isTokenTyp( TYP_CREG ))  {
        
        rExpr -> typ = TYP_CREG;
        rExpr -> numVal = glb -> cpu -> getReg( RC_CTRL_REG_SET, glb -> tok -> tokVal( ));
        glb -> tok -> nextToken( );
        return( NO_ERR );
    }
    else if ( glb -> tok -> isToken( TOK_IDENT )) {
        
        
        // ??? we need to look up the ident in ENV ?
        
        
        rExpr -> typ    = glb -> tok -> tokTyp( );
        rExpr -> tokId  = glb -> tok -> tokId( );
        
        switch( rExpr -> typ ) {
                
            case TYP_BOOL:  rExpr -> bVal     = glb -> tok -> tokVal( );         break;
            case TYP_NUM:   rExpr -> numVal   = glb -> tok -> tokVal( );         break;
            case TYP_STR:   strcpy( rExpr -> strVal, glb -> tok -> tokStr( ));   break;
            
            case TYP_EXT_ADR: {
                
                rExpr -> seg = glb -> tok -> tokSeg( );
                rExpr -> ofs = glb -> tok -> tokOfs( );
                
            } break;
                
            default: fprintf( stdout, "**** uncaptured type, fix ... \n" );
        }
    
        glb -> tok -> nextToken( );
        return( NO_ERR );
    }
    else if ( glb -> tok -> isToken( TOK_NEG )) {
        
        parseFactor( rExpr );
        rExpr -> numVal = ~ rExpr -> numVal;
        return( NO_ERR );
    }
    else if ( glb -> tok -> isToken( TOK_LPAREN )) {
      
        glb -> tok -> nextToken( );
        if ( glb -> tok -> isTokenTyp( TYP_SREG )) {
            
            rExpr -> typ    = TYP_EXT_ADR;
            rExpr -> seg    = glb -> cpu -> getReg( RC_SEG_REG_SET, glb -> tok -> tokVal( ));
            
            glb -> tok -> nextToken( );
            
            if ( glb -> tok -> isToken( TOK_COMMA )) glb -> tok -> nextToken( );
            else return( ERR_EXPECTED_COMMA );
            
            if ( glb -> tok -> isTokenTyp( TYP_GREG )) {
                
                rExpr -> ofs = glb -> cpu -> getReg( RC_GEN_REG_SET, glb -> tok -> tokVal( ));
                glb -> tok -> nextToken( );
            }
            else return( ERR_EXPECTED_GENERAL_REG );
        }
        else if ( glb -> tok -> isTokenTyp( TYP_GREG )) {
            
            rExpr -> typ = TYP_ADR;
            rExpr -> numVal = glb -> tok -> tokVal( );
            glb -> tok -> nextToken( );
        }
        else {
            
            ErrMsgId rStat = parseExpr( rExpr );
            if ( rStat != NO_ERR ) return( rStat );
        }
        
        if ( glb -> tok -> isToken( TOK_RPAREN )) glb -> tok -> nextToken( );
        else return( ERR_EXPECTED_RPAREN );
    }
    else if (( glb -> tok -> tokTyp( ) == TYP_NIL ) && ( glb -> tok -> tokId( ) == TOK_EOS )) {
        
        rExpr -> typ = TYP_NIL;
        return( NO_ERR );
    }
    else {
        
        rExpr -> typ = TYP_NUM;
        rExpr -> numVal = 0;
        glb -> tok -> nextToken( );
        return( ERR_EXPR_FACTOR );
    }
    
    return( NO_ERR );
}

//------------------------------------------------------------------------------------------------------------
// "parseTerm" parses the term syntax.
//
//      <term>      ->  <factor> { <termOp> <factor> }
//      <termOp>    ->  "*" | "/" | "%" | "&"
//
// ??? type mix options ?
//------------------------------------------------------------------------------------------------------------
ErrMsgId DrvExprEvaluator::parseTerm( DrvExpr *rExpr ) {
    
    DrvExpr  lExpr;
    ErrMsgId rStat = parseFactor( rExpr );
    
    while (( glb -> tok -> tokId( ) == TOK_MULT )   ||
           ( glb -> tok -> tokId( ) == TOK_DIV  )   ||
           ( glb -> tok -> tokId( ) == TOK_MOD  )   ||
           ( glb -> tok -> tokId( ) == TOK_AND  ))  {
        
        uint8_t op = glb -> tok -> tokId( );
        
        glb -> tok -> nextToken( );
        rStat = parseFactor( &lExpr );
        
        if ( lExpr.typ != TYP_NIL ) {
            
            if ( rExpr -> typ != lExpr.typ ) {
                
                return ( ERR_EXPR_TYPE_MATCH );
            }
        }
        else return( ERR_UNEXPECTED_EOS );
        
        switch( op ) {
                
            case TOK_MULT:   rExpr -> numVal = rExpr -> numVal * lExpr.numVal; break;
            case TOK_DIV:    rExpr -> numVal = rExpr -> numVal / lExpr.numVal; break;
            case TOK_MOD:    rExpr -> numVal = rExpr -> numVal % lExpr.numVal; break;
            case TOK_AND:    rExpr -> numVal = rExpr -> numVal & lExpr.numVal; break;
        }
    }
    
    return( rStat );
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
ErrMsgId DrvExprEvaluator::parseExpr( DrvExpr *rExpr ) {
    
    DrvExpr lExpr;
    uint8_t rStat;
    
    if ( glb -> tok -> isToken( TOK_PLUS )) {
        
        glb -> tok -> nextToken( );
        rStat = parseTerm( rExpr );
        
        if ( rExpr -> typ != TYP_NUM ) {
            
            return( ERR_EXPECTED_NUMERIC );
        }
    }
    else if ( glb -> tok -> isToken( TOK_MINUS )) {
        
        glb -> tok -> nextToken( );
        rStat = parseTerm( rExpr );
        
        if ( rExpr -> typ == TYP_NUM ) rExpr -> numVal = - rExpr -> numVal;
        else return( ERR_EXPECTED_NUMERIC );
    }
    else rStat = parseTerm( rExpr );
    
    while (( glb -> tok -> isToken( TOK_PLUS   )) ||
           ( glb -> tok -> isToken( TOK_MINUS  )) ||
           ( glb -> tok -> isToken( TOK_OR     )) ||
           ( glb -> tok -> isToken( TOK_XOR    ))) {
        
        uint8_t op = glb -> tok -> tokId( );
        
        glb -> tok -> nextToken( );
        rStat = parseTerm( &lExpr );
        
        if ( lExpr.typ != TYP_NIL ) {
            
            if ( rExpr -> typ != lExpr.typ ) {
                
                return ( ERR_EXPR_TYPE_MATCH );
            }
        }
        else return( ERR_UNEXPECTED_EOS );
        
        switch ( op ) {
                
            case TOK_PLUS:   rExpr -> numVal = rExpr -> numVal + lExpr.numVal; break;
            case TOK_MINUS:  rExpr -> numVal = rExpr -> numVal - lExpr.numVal; break;
            case TOK_OR:     rExpr -> numVal = rExpr -> numVal | lExpr.numVal; break;
            case TOK_XOR:    rExpr -> numVal = rExpr -> numVal ^ lExpr.numVal; break;
        }
    }
    
    return( NO_ERR );
}

