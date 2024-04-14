//------------------------------------------------------------------------------------------------------------
//
// VCPU32 - A 32-bit CPU - Main
//
//------------------------------------------------------------------------------------------------------------
//
// VCPU32 - A 32-bit CPU - Main
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
#include "VCPU32-Driver.hpp"
#include "VCPU32-Core.hpp"

//------------------------------------------------------------------------------------------------------------
// The main program. We will first initialize the command interpreter and process any optional program
// arguments. Next, the descriptors are built and both are initialized with that data. There is a global
// data structure that holds the object references for easy access across the entire program.
//
// Idea: we could have for all parameters in the descriptors an environment variable. These variables can then
// be used to create a descriptor with the data coming from these variables. Also, we should have an option to
// set the environmnt variables from a file, specified as an input argument to the program.
//
// ??? do we keep all descriptors in one structure ? What if the CPU has two cores ?
// ??? is the IO subsystem part of the CPU structure ? Still, we would need a memory range to configure...
//------------------------------------------------------------------------------------------------------------
int main( int argc, const char* argv[ ] ) {
    
    VCPU32Globals     glbDesc;
    CpuCoreDesc       cpuDesc;

    cpuDesc.flags                       = 0;
    
    cpuDesc.tlbOptions                  = VMEM_T_SPLIT_TLB;
    cpuDesc.cacheL1Options              = VMEM_T_L1_SPLIT_CACHE;
    cpuDesc.cacheL2Options              = VMEM_T_NIL;
        
    cpuDesc.iTlbDesc.type               = TLB_T_L1_INSTR;
    cpuDesc.iTlbDesc.entries            = 1024;
    cpuDesc.iTlbDesc.accessType         = TLB_AT_DIRECT_MAPPED;
    
    cpuDesc.dTlbDesc.type               = TLB_T_L1_DATA;
    cpuDesc.dTlbDesc.entries            = 1024;
    cpuDesc.dTlbDesc.accessType         = TLB_AT_DIRECT_MAPPED;
    
    cpuDesc.iCacheDescL1.type           = MEM_T_L1_INSTR;
    cpuDesc.iCacheDescL1.accessType     = MEM_AT_DIRECT_MAPPED;
    cpuDesc.iCacheDescL1.blockEntries   = 1024;
    cpuDesc.iCacheDescL1.blockSize      = 16;
    cpuDesc.iCacheDescL1.blockSets      = 2;
    cpuDesc.iCacheDescL1.latency        = 0;
    cpuDesc.iCacheDescL1.priority       = 1;
    
    cpuDesc.dCacheDescL1.type           = MEM_T_L1_DATA;
    cpuDesc.dCacheDescL1.accessType     = MEM_AT_DIRECT_MAPPED;
    cpuDesc.dCacheDescL1.blockEntries   = 1024;
    cpuDesc.dCacheDescL1.blockSize      = 32;
    cpuDesc.dCacheDescL1.blockSets      = 4;
    cpuDesc.dCacheDescL1.latency        = 0;
    cpuDesc.dCacheDescL1.priority       = 2;
        
    cpuDesc.uCacheDescL2.type           = MEM_T_L2_UNIFIED;
    cpuDesc.uCacheDescL2.accessType     = MEM_AT_DIRECT_MAPPED;
    cpuDesc.uCacheDescL2.blockEntries   = 2048;
    cpuDesc.uCacheDescL2.blockSize      = 32;
    cpuDesc.uCacheDescL2.blockSets      = 2;
    cpuDesc.uCacheDescL2.latency        = 2;
    cpuDesc.uCacheDescL2.priority       = 3;
   
    cpuDesc.memDesc.type                = MEM_T_PHYS_MEM;
    cpuDesc.memDesc.accessType          = MEM_AT_DIRECT_INDEXED;
    cpuDesc.memDesc.blockEntries        = 1024 * 1024; // for just a million blocks.
    cpuDesc.memDesc.blockSize           = 16;
    cpuDesc.memDesc.blockSets           = 1;
    cpuDesc.memDesc.startAdr            = 0;
    cpuDesc.memDesc.latency             = 2;
    cpuDesc.memDesc.priority            = 3;
    
    cpuDesc.pdcDesc.type                = MEM_T_PDC_MEM;
    cpuDesc.pdcDesc.accessType          = MEM_AT_DIRECT_INDEXED;
    cpuDesc.pdcDesc.blockEntries        = 1024; // for just a thousand blocks.
    cpuDesc.pdcDesc.blockSize           = 16;
    cpuDesc.pdcDesc.blockSets           = 1;
    cpuDesc.pdcDesc.startAdr            = 0xF0000000;
    cpuDesc.pdcDesc.latency             = 2;
    cpuDesc.pdcDesc.priority            = 3;
    
    cpuDesc.ioDesc.type                 = MEM_T_IO_MEM;
    cpuDesc.ioDesc.accessType           = MEM_AT_DIRECT_INDEXED;
    cpuDesc.ioDesc.blockEntries         = 1024; // for just a thousand blocks.
    cpuDesc.ioDesc.blockSize            = 16;
    cpuDesc.ioDesc.blockSets            = 1;
    cpuDesc.ioDesc.startAdr             = 0xFFFF0000;
    cpuDesc.ioDesc.latency              = 2;
    cpuDesc.ioDesc.priority             = 3;
 
    glbDesc.cpu                         = new CpuCore( &cpuDesc );
    
    glbDesc.env                         = new DrvEnv( &glbDesc );
    glbDesc.cmds                        = new DrvCmds( &glbDesc );
    glbDesc.lineDisplay                 = new DrvLineDisplay( &glbDesc );
    glbDesc.winDisplay                  = new DrvWinDisplay( &glbDesc );
    glbDesc.disAsm                      = new DrvDisAsm( &glbDesc );
    
    glbDesc.cmds -> processCmdLineArgs( argc, argv );
    glbDesc.cmds -> printWelcome( );
    glbDesc.winDisplay -> windowDefaults( );
    glbDesc.cpu -> reset( );
    glbDesc.cmds -> cmdLoop( );
}
