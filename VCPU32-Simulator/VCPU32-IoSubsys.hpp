//------------------------------------------------------------------------------------------------------------
//
// VCPU32 - A 32-bit CPU - IO Subsystem definitions
//
//------------------------------------------------------------------------------------------------------------
//
// VCPU-32 implements a memory mapped IO subsysten. The CPU core issus mememory resds and writes the need
// to be passed on to the corresponding IO module.
//
//------------------------------------------------------------------------------------------------------------
//
// VCPU32 - A 32-bit CPU - IO Subsystem definitions
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
#ifndef VCPU32_IoSubsys_hpp
#define VCPU32_IoSubsys_hpp

#include "VCPU32-Types.h"
#include "VCPU32-Core.h"

//------------------------------------------------------------------------------------------------------------
// The IO module descriptor defines the IO devices that the simulator is configured with. The IO descriptor
// has as the key structure an array of IO devices.
//
// ??? well, one day...
//------------------------------------------------------------------------------------------------------------
struct CpuIoDescEntry {
    
    uint32_t entrySize;  // entry size in bytes, a multiple of pages
    uint32_t startAdr;   // the IO address range starting address.
    
};

struct CpuIoDesc {
    
    uint32_t        numDevices  = 0;
    CpuIoDescEntry  *ioArray    = nullptr;
    
};

//------------------------------------------------------------------------------------------------------------
// The IO module is the central object for an IO module. It contains the data and methods to represent a
// device in the IO memory address range. The CPU Memory Object for the IO space has a reference to this
// object. Upon receiving a memory operation for the IO address range the request is passed to this object.
// In general the IoModule follows the same implementation logic with a reset function, a imgainray clock
// and so on.
//
//------------------------------------------------------------------------------------------------------------
struct IoModule {
    
    IoModule( CpuIoDesc *ioDesc );
    
    void        reset( );
    void        tick( );
    void        process( );
    void        clearStats( );
    void        abortOp( );
    
    bool        readIo( uint32_t adr, uint32_t len, uint32_t *word, uint16_t pri = 0 );
    bool        writeIo( uint32_t adr, uint32_t len, uint32_t word, uint16_t pri = 0 );
    
    uint32_t    getIoDataWord( uint32_t ofs );
    void        putIoDataWord( uint32_t ofs, uint32_t val );
    
private:
    
    CpuIoDesc ioDesc;
    
    // ??? what structure do we need ?
    
};

#endif /* VCPU32_IoSubsys_hpp */
