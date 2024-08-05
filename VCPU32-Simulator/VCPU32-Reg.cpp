//------------------------------------------------------------------------------------------------------------
//
// VCPU32 - A 32-bit CPU - Register
//
//------------------------------------------------------------------------------------------------------------
//
// CPU24Reg models a machine register. A register has an input and putput part. Setting means to store a
// value in the input part, getting means a retrieval from the output part. The "tick" methods copies from
// input to output, simulating a "positive clock edge" triggered D-Flip Flop..
//
//------------------------------------------------------------------------------------------------------------
//
// VCPU32 - A 32-bit CPU - Register
// Copyright (C) 2022 - 2024  Helmut Fieres
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
#include "VCPU32-Core.h"

//------------------------------------------------------------------------------------------------------------
// Methods. Straighforward. There is a "load" method which will set both register portions. This is primarily
// used by the CPU driver to set a value and directly observe it through subsequent getter calls. A reister
// can also be marked as a priviledge write access register.
//
//------------------------------------------------------------------------------------------------------------
CpuReg::CpuReg( uint32_t val, bool isPriv ) {
    
    this -> regIn   = val;
    this -> regOut  = val;
    this -> isPriv  = isPriv;
}

void CpuReg::init( uint32_t val, bool isPriv ) {
    
    this -> regIn   = 0;
    this -> regOut  = 0;
    this -> isPriv  = isPriv;
}

void CpuReg::reset( ) {
    
    regIn  = 0;
    regOut = 0;
}

void CpuReg::tick( ) {
    
    regOut = regIn;
}

void CpuReg::load( uint32_t val ) {
    
    regIn = regOut = val;
}

void CpuReg::set( uint32_t val ) {
    
    regIn = val;
}

uint32_t CpuReg::get( ) {
 
    return( regOut );
}

uint32_t CpuReg::getLatched( ) {
    
    return( regIn );
}

bool CpuReg::getBit( int pos ) {
    
   return( getBitField( pos, 1 ));
}

void CpuReg::setBit( bool val, int pos ) {
    
    setBitField( val, pos, 1 );
}

void CpuReg::setBit( int pos ) {
    
    setBitField( 1, pos, 1 );
}

void CpuReg::clearBit( int pos ) {
    
    setBitField( 0, pos, 1 );
}

uint32_t CpuReg::getBitField( int pos, int len, bool sign ) {
    
    pos = pos % 32;
    len = len % 32;
    
    uint32_t tmpM = ( 1U << len ) - 1;
    uint32_t tmpA = regOut >> ( 31 - pos );
    
    if ( sign ) return( tmpA | ( ~ tmpM ));
    else        return( tmpA & tmpM );
}

void CpuReg::setBitField( uint32_t val, int pos, int len ) {
    
    pos = pos % 32;
    len = len % 32;
    
    uint32_t tmpM = ( 1U << len ) - 1;
    
    val = ( val & tmpM ) << ( 31 - pos );
    
    regIn = ( regIn & ( ~tmpM )) | val;
}

void  CpuReg::setBitField( int pos, int len ) {
    
    setBitField( 1, pos, len );
}

void  CpuReg::clearBitField( int pos, int len ) {
    
    setBitField( 0, pos, len );
}

void CpuReg::orBitField( uint32_t val, int pos, int len ) {
    
    pos = pos % 32;
    len = len % 32;
    
    uint32_t tmpM = ( 1U << len ) - 1;
    
    val = ( val & tmpM ) << ( 31 - pos );
    
    regIn = regIn | val;
}

void  CpuReg::andBitField( uint32_t val, int pos, int len  ) {
    
    pos = pos % 32;
    len = len % 32;
    
    uint32_t tmpM = ( 1U << len ) - 1;
    
    val = ( val & tmpM ) << ( 31 - pos );
    
    regIn = regIn & ( ~ val );
}

bool CpuReg::isPrivReg( ) {
    
    return( isPriv );
}
