//------------------------------------------------------------------------------------------------------------
//
// VCPU32 - A 32-bit CPU - IO Subsystem implementation file
//
//------------------------------------------------------------------------------------------------------------
//
// VCPU-32 implements a memory mapped IO subsysten.
//
//------------------------------------------------------------------------------------------------------------
//
// VCPU32 - A 32-bit CPU - IO Subsystem implementation file
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
#include "VCPU32-IoSubsys.h"


IoModule::IoModule( CpuIoDesc *ioDesc ) {
    
    
}

void  IoModule::reset( ) {
    
}

void IoModule::tick( ) {
    
}

void IoModule::process( ) {
    
}

void IoModule::clearStats( ) {
    
}

void IoModule::abortOp( ) {
    
}

bool IoModule::readIo( uint32_t adr, uint32_t len, uint32_t *word, uint16_t pri ) {
    
    return( false );
}

bool IoModule::writeIo( uint32_t adr, uint32_t len, uint32_t word, uint16_t pri ) {
    
    return( false );
}


//------------------------------------------------------------------------------------------------------------
// The "getIoDataWrd" and "ioPutDataWord" are used by the simulator line and window display for accessing the
// IO address space data.
//
// ??? not clear what this mean for when tehre is no memory....
//------------------------------------------------------------------------------------------------------------
uint32_t IoModule::getIoDataWord( uint32_t ofs ) {
    
    return( 0 );
}
    
void IoModule::putIoDataWord( uint32_t ofs, uint32_t val ) {
    
}



