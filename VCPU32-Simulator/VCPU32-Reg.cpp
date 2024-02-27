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
#include "VCPU32-Types.hpp"
#include "VCPU32-Core.hpp"

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

bool CpuReg::isPrivReg( ) {
    
    return( isPriv );
}
