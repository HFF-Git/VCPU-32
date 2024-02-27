//------------------------------------------------------------------------------------------------------------
//
// VCPU32 - A 32-bit CPU - Debug SubSystem
//
//------------------------------------------------------------------------------------------------------------
//
// We need a basic debugging capabilty for CPU24. The idea is to set a breakpoint, which replaces the
// instruction at seg.ofs with the break instruction. Upon encounering a break instruction, a trap is
// raised which will ultimately lead to entering the command interreter of the simulator. There is one
// global breakpoint table which keeps track of the instructions replaxed with breakpoints.
//
//------------------------------------------------------------------------------------------------------------
//
// VCPU32 - A 32-bit CPU - Debug SubSystem
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
#ifndef VCPU32Debug_hpp
#define VCPU32Debug_hpp

#include "VCPU32-Types.hpp"

// ??? first ideas...

const int MAX_BREAK_POINTS = 32;

//------------------------------------------------------------------------------------------------------------
//
//
//------------------------------------------------------------------------------------------------------------
enum CPU24BreakPointFlags : uint32_t {
    
    BP_NIL          = 0,
    BP_USED         = 000000001,
    BP_ENABLED      = 000000002,
    
};


//------------------------------------------------------------------------------------------------------------
// A breakpoint table entry. A breakpoint needs to keep track of the instrcution address and the instruction
// originally found at that address. There are also some flags about whether the breakpoint is enabled and
// so on. Breakpoints can be set in a way that they only fire every nth count.
//
//------------------------------------------------------------------------------------------------------------
struct CPU24Breakpoint {
    
    uint32_t    flags       = BP_NIL;
    uint32_t    instrAdrSeg = 0;
    uint32_t    instrAdrOfs = 0;
    uint32_t    instr       = OP_NOP;
    int         skipCount   = 0;
};


//------------------------------------------------------------------------------------------------------------
// The Debugger object. The object contains the methods to manager the breakpoint table and to handle the
// mechanics of setting a breakpoint as well as entering the command interpreter.
//
//------------------------------------------------------------------------------------------------------------
struct CPU24Debug {

public:

    CPU24Debug ( CpuCore *core );
    void                initDebug ( );
    
    int                 addBreakPoint( uint32_t seg, uint32_t ofs, uint32_t instr );
    int                 deleteBreakpoint( uint32_t seg, uint32_t ofs );
    CPU24Breakpoint     *lookupBreakPoint( uint32_t seg, uint32_t ofs );
    CPU24Breakpoint     *lookupBreakPoint( int index );
    int                 getBreakPointTabSize( );
    
    int                 setBreakoint( uint32_t seg, uint32_t ofs, uint32_t instr );
    int                 clearBreakPoint( uint32_t seg, uint32_t ofs );
    
    int                 translateVirtualAdr( uint32_t seg, uint32_t ofs, uint32_t *physAdr );
 
    void                enterDebug( );
    void                resumeProg( );

private:
    
    CPU24Breakpoint     *breakPointTab = nullptr;
    CpuCore           * core         = nullptr;
};


#endif
