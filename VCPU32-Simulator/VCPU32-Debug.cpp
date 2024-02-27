//------------------------------------------------------------------------------------------------------------
//
// VCPU32 - A 32-bit CPU - A basic Debugger for VCPU-32
//
//------------------------------------------------------------------------------------------------------------
//
// Modern CPUs feature an instruction and data cache. The CPU24Cache class implements these caches with
// several configuration options.
//
//------------------------------------------------------------------------------------------------------------
//
// VCPU32 - A 32-bit CPU - A basic Debugger for VCPU-32
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
#include "VCPU32-Types.hpp"
#include "VCPU32-Core.hpp"
#include "VCPU32-Debug.hpp"


//------------------------------------------------------------------------------------------------------------
//
//
//
//------------------------------------------------------------------------------------------------------------
CPU24Debug::CPU24Debug ( CpuCore *core ) {
    
    this -> core = core;
}


//------------------------------------------------------------------------------------------------------------
//
//
//------------------------------------------------------------------------------------------------------------
void CPU24Debug::initDebug ( ) {
    
    breakPointTab = (CPU24Breakpoint *) calloc( MAX_BREAK_POINTS, sizeof( CPU24Breakpoint ));
    
    for ( int i = 0; i < MAX_BREAK_POINTS; i ++ ) {
        
        breakPointTab[ i ].flags        = BP_NIL;
        breakPointTab[ i ].instrAdrSeg  = 0;
        breakPointTab[ i ].instrAdrOfs  = 0;
        breakPointTab[ i ].instr        = OP_NOP;
        breakPointTab[ i ].skipCount    = 0;
    }
}

//------------------------------------------------------------------------------------------------------------
//
//
//------------------------------------------------------------------------------------------------------------
int CPU24Debug::addBreakPoint( uint32_t seg, uint32_t ofs, uint32_t instr ) {
    
    if ( lookupBreakPoint( seg, ofs ) == nullptr ) {
        
        for ( int i = 0; i < MAX_BREAK_POINTS; i++ ) {
            
            if ( ! ( breakPointTab[ i ].flags & BP_USED )) {
                
                breakPointTab[ i ].flags        |= BP_USED;
                breakPointTab[ i ].instrAdrSeg  = seg;
                breakPointTab[ i ].instrAdrOfs  = ofs;
                breakPointTab[ i ].instr        = instr;
                breakPointTab[ i ].skipCount    = 0;
                return( i );
            }
        }
    }

    return( -1 );
}

//------------------------------------------------------------------------------------------------------------
//
//
//------------------------------------------------------------------------------------------------------------
int CPU24Debug::deleteBreakpoint( uint32_t seg, uint32_t ofs ) {

    for ( int i = 0; i < MAX_BREAK_POINTS; i++ ) {
        
        if (( breakPointTab[ i ].flags & BP_USED )      &&
            ( breakPointTab[ i ].instrAdrSeg == seg )   &&
            ( breakPointTab[ i ].instrAdrSeg == ofs )) {
            
            breakPointTab[ i ].flags        = BP_NIL;
            breakPointTab[ i ].instrAdrSeg  = 0;
            breakPointTab[ i ].instrAdrOfs  = 0;
            breakPointTab[ i ].instr        = OP_NOP;
            breakPointTab[ i ].skipCount    = 0;
            return( i );
        }
    }
    
    return( -1 );
}

//------------------------------------------------------------------------------------------------------------
//
//
//------------------------------------------------------------------------------------------------------------
CPU24Breakpoint *CPU24Debug::lookupBreakPoint( uint32_t seg, uint32_t ofs ) {
    
    for ( int i = 0; i < MAX_BREAK_POINTS; i++ ) {
        
        if (( breakPointTab[ i ].flags & BP_USED )      &&
            ( breakPointTab[ i ].instrAdrSeg == seg )   &&
            ( breakPointTab[ i ].instrAdrSeg == ofs )) {
            
            return( &breakPointTab[ i ] );
        }
    }
   
    return( nullptr );
}

CPU24Breakpoint *CPU24Debug::lookupBreakPoint( int index ) {
    
    return(( index < MAX_BREAK_POINTS ) ? &breakPointTab[ index ] : nullptr );
}


//------------------------------------------------------------------------------------------------------------
//
//
//------------------------------------------------------------------------------------------------------------
int CPU24Debug::setBreakoint( uint32_t seg, uint32_t ofs, uint32_t instr ) {
    
    // ??? tricky ... we can easily add the breakpoint to the breakpoint table, but for memory we need to
    // know the physical address of the location...
    
    return( 0 );
}

int CPU24Debug::clearBreakPoint( uint32_t seg, uint32_t ofs ) {
    
    // ??? tricky ... we can easily remove the breakpoint from the breakpoint table, but for memory we need to
    // know the physical address of the location...
    
    return( 0 );
}

//------------------------------------------------------------------------------------------------------------
//
//
//------------------------------------------------------------------------------------------------------------
void CPU24Debug::enterDebug( ) {
    
    // ??? we come in from the trap handler. First step is to replace the BRK instruction with the
    // original instruction. This will only take place when the skip count is reached.
    //
    // Next we just invoke the command interpreter.
}

//------------------------------------------------------------------------------------------------------------
//
//
//------------------------------------------------------------------------------------------------------------
void CPU24Debug::resumeProg( ) {
    
    // ??? tricky. We need to execute from here on. This means to replace again the orginal instruction
    // with the BRK instruction. Furthwermore we need to set a temporary breakpoint on the follow on
    // instruction. This is needed to set the breakpoint again that we removed before.
    // 
    
}

//------------------------------------------------------------------------------------------------------------
//
//
//------------------------------------------------------------------------------------------------------------
int CPU24Debug::translateVirtualAdr( uint32_t seg, uint32_t ofs, uint32_t *physAdr ) {
    
    // ??? not quite clear yet how to best do this .... but we need it ....
    
    return( 0 );
}






