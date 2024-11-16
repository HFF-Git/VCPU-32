//------------------------------------------------------------------------------------------------------------
//
// VCPU32 - A 32-bit CPU - Simulator Commands line mode display
//
//------------------------------------------------------------------------------------------------------------
// This module contains the line mode display routines used by the command interpreter.
//
//------------------------------------------------------------------------------------------------------------
//
// VCPU32 - A 32-bit CPU - Simulator Commands line mode display
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
#include "VCPU32-Core.h"
#include "VCPU32-Driver.h"

//------------------------------------------------------------------------------------------------------------
//
//
//------------------------------------------------------------------------------------------------------------
namespace {

//------------------------------------------------------------------------------------------------------------
// "displayInvalidWord" shows a set of "*" when we cannot get a value for word. We make the length of the
// "*" string according to the current radix.
//
//------------------------------------------------------------------------------------------------------------
void displayInvalidWord( int rdx ) {
    
    if      ( rdx == 10 )  fprintf( stdout, "**********" );
    else if ( rdx == 8  )  fprintf( stdout, "************" );
    else if ( rdx == 16 )  fprintf( stdout, "**********" );
    else fprintf( stdout, "**num**" );
}

}; // namespace

//------------------------------------------------------------------------------------------------------------
// The object constructor. We just remember the globals pointer.
//
//------------------------------------------------------------------------------------------------------------
DrvLineDisplay::DrvLineDisplay( VCPU32Globals *glb ) {
    
    this -> glb = glb;
}

//------------------------------------------------------------------------------------------------------------
// "displayWord" lists out a 32-bit machine word in the specified number base. If the format parameter is
// omitted or set to "default", the environment variable for the base number is used.
//
//------------------------------------------------------------------------------------------------------------
void DrvLineDisplay::displayWord( uint32_t val, int rdx ) {
    
    if      ( rdx == 10 )  fprintf( stdout, "%10d", val );
    else if ( rdx == 8  )  fprintf( stdout, "%#012o", val );
    else if ( rdx == 16 )  {
        
        if ( val == 0 ) fprintf( stdout, "0x00000000" );
        else fprintf( stdout, "%#010x", val );
    }
    else fprintf( stdout, "**num**" );
}

//------------------------------------------------------------------------------------------------------------
// "displayHalfWord" lists out a 12-bit word in the specified number base. If the format parameter is omitted
// or set to "default", the environment variable for the base number is used.
//
//------------------------------------------------------------------------------------------------------------
void DrvLineDisplay::displayHalfWord( uint32_t val, int rdx ) {
    
    if      ( rdx == 10 )  fprintf( stdout, "%5d", val );
    else if ( rdx == 8  ) fprintf( stdout, "%06o", val );
    else if ( rdx == 16 )  {
        
        if ( val == 0 ) fprintf( stdout, "0x0000" );
        else fprintf( stdout, "%#05x", val );
    }
    else fprintf( stdout, "**num**" );
}

//------------------------------------------------------------------------------------------------------------
// "displayRegsAndLabel" is a little building block to display a set of registers and a label. For example:
// "GR0=  xxx xxx xxx". It is an internal use only function to allow for the display of registers with some
// options on title and how many are displayed in a row. Note that this routine does not a lot of checking.
//
//------------------------------------------------------------------------------------------------------------
void DrvLineDisplay::displayRegsAndLabel(   RegClass  regSetId,
                                            int       regStart,
                                            int       numOfRegs,
                                            char      *lineLabel,
                                            int       rdx ) {
    
    if ( strlen( lineLabel ) > 0 ) fprintf( stdout, "%s", lineLabel );
    
    for ( int i = regStart; i < regStart + numOfRegs; i++ ) {
        
        displayWord( glb -> cpu -> getReg( regSetId, i ), rdx );
        if ( i < regStart + numOfRegs ) fprintf( stdout, " " );
    }
}

//------------------------------------------------------------------------------------------------------------
// Display register set functions. The CPU has several register sets starting with the general registers, the
// segment register up to the control, memory, TLB and pipeline registers. The following routines will just
// display a particular register set.
//
//------------------------------------------------------------------------------------------------------------
void DrvLineDisplay::displayGeneralRegSet( int rdx ) {
    
    displayRegsAndLabel( RC_GEN_REG_SET, 0, 8, ((char *) "R0=   " ), rdx );
    fprintf( stdout, "\n" );
    displayRegsAndLabel( RC_GEN_REG_SET, 8, 8, ((char *) "R8=   " ), rdx );
    fprintf( stdout, "\n" );
}

void DrvLineDisplay::displaySegmentRegSet( int rdx ) {
    
    displayRegsAndLabel( RC_SEG_REG_SET, 0, 8, ((char *) "S0=   " ), rdx );
    fprintf( stdout, "\n" );
}

void DrvLineDisplay::displayControlRegSet( int rdx ) {
    
    displayRegsAndLabel( RC_CTRL_REG_SET, 0, 8, ((char *) "CR0=  " ), rdx );
    fprintf( stdout, "\n" );
    
    displayRegsAndLabel( RC_CTRL_REG_SET, 8, 8, ((char *) "CR8=  " ), rdx );
    fprintf( stdout, "\n" );
    
    displayRegsAndLabel( RC_CTRL_REG_SET, 16, 8, ((char *) "CR16= " ), rdx );
    fprintf( stdout, "\n" );
    
    displayRegsAndLabel( RC_CTRL_REG_SET, 24, 8, ((char *) "CR24= " ), rdx );
    fprintf( stdout, "\n" );
}

// ??? changed ....
void DrvLineDisplay::displayPStateRegSet( int rdx ) {
    
    fprintf( stdout, "PSW0=   " );
    displayWord( glb -> cpu -> getReg( RC_FD_PSTAGE, PSTAGE_REG_ID_PSW_0 ), rdx );
    fprintf( stdout, ", PSW1=   " );
    displayWord( glb -> cpu -> getReg( RC_FD_PSTAGE, PSTAGE_REG_ID_PSW_1 ), rdx );
    fprintf( stdout, "\n" );
}

void DrvLineDisplay::displayPlIFetchDecodeRegSet( int rdx ) {
    
    if ( glb -> cpu -> getReg( RC_FD_PSTAGE, PSTAGE_REG_STALLED ) == 1 )
        fprintf( stdout, "FD(S): PSW=" );
    else
        fprintf( stdout, "FD:    PSW=" );
    
    displayWord( glb -> cpu -> getReg( RC_FD_PSTAGE, PSTAGE_REG_ID_PSW_0), rdx );
    fprintf( stdout, "." );
    displayWord( glb -> cpu -> getReg( RC_FD_PSTAGE, PSTAGE_REG_ID_PSW_1 ), rdx );
    fprintf( stdout, "\n" );
}

void DrvLineDisplay::displayPlMemoryAccessRegSet( int rdx ) {
    
    if ( glb -> cpu -> getReg( RC_MA_PSTAGE, PSTAGE_REG_STALLED ) == 1 )
        fprintf( stdout, "MA(S): PSW=" );
    else
        fprintf( stdout, "MA:    PSW=" );
    
    displayWord( glb -> cpu -> getReg( RC_MA_PSTAGE, PSTAGE_REG_ID_PSW_0 ), rdx );
    fprintf( stdout, "." );
    displayWord( glb -> cpu -> getReg( RC_MA_PSTAGE, PSTAGE_REG_ID_PSW_1 ), rdx );
    fprintf( stdout, " I=" );
    displayWord( glb -> cpu -> getReg( RC_MA_PSTAGE, PSTAGE_REG_ID_INSTR ), rdx );
    fprintf( stdout, " A=" );
    displayWord( glb -> cpu -> getReg( RC_MA_PSTAGE, PSTAGE_REG_ID_VAL_A ), rdx );
    fprintf( stdout, " B=" );
    displayWord( glb -> cpu -> getReg( RC_MA_PSTAGE, PSTAGE_REG_ID_VAL_B ), rdx );
    fprintf( stdout, " X=" );
    displayWord( glb -> cpu -> getReg( RC_MA_PSTAGE, PSTAGE_REG_ID_VAL_X ), rdx );
    fprintf( stdout, "\n" );
}

void DrvLineDisplay::displayPlExecuteRegSet( int rdx ) {
    
    if ( glb -> cpu -> getReg( RC_EX_PSTAGE, PSTAGE_REG_STALLED ) == 1 )
        fprintf( stdout, "EX(S): PSW=" );
    else
        fprintf( stdout, "EX:    PSW=" );
    
    displayWord( glb -> cpu -> getReg( RC_EX_PSTAGE, PSTAGE_REG_ID_PSW_0 ), rdx );
    fprintf( stdout, "." );
    displayWord( glb -> cpu -> getReg( RC_EX_PSTAGE, PSTAGE_REG_ID_PSW_1 ), rdx );
    fprintf( stdout, " I=" );
    displayWord( glb -> cpu -> getReg( RC_EX_PSTAGE, PSTAGE_REG_ID_INSTR ), rdx );
    fprintf( stdout, " A=" );
    displayWord( glb -> cpu -> getReg( RC_EX_PSTAGE, PSTAGE_REG_ID_VAL_A ), rdx );
    fprintf( stdout, " B=" );
    displayWord( glb -> cpu -> getReg( RC_EX_PSTAGE, PSTAGE_REG_ID_VAL_B ), rdx );
    fprintf( stdout, " X=" );
    displayWord( glb -> cpu -> getReg( RC_EX_PSTAGE, PSTAGE_REG_ID_VAL_X ), rdx );
    fprintf( stdout, "\n" );
}

void DrvLineDisplay::displayPlRegSets( int rdx ) {
    
    displayPlIFetchDecodeRegSet( rdx );
    displayPlMemoryAccessRegSet( rdx );
    displayPlExecuteRegSet( rdx );
}

void DrvLineDisplay::displayMemObjRegSet( CpuMem *mem, int rdx ) {
    
    fprintf( stdout, "State:   %s\n", mem -> getMemOpStr( mem -> getMemCtrlReg( MC_REG_STATE )));
  
    fprintf( stdout, "Seg:ofs: " );
    displayWord( mem -> getMemCtrlReg( MC_REG_REQ_SEG ), rdx );
    fprintf( stdout, ":" );
    displayWord( mem -> getMemCtrlReg( MC_REG_REQ_OFS ), rdx );
    fprintf( stdout, ", Tag: " );
    displayWord( mem -> getMemCtrlReg( MC_REG_REQ_TAG ), rdx );
    fprintf( stdout, ", Len: " );
    displayWord( mem -> getMemCtrlReg( MC_REG_REQ_LEN ), rdx );
    fprintf( stdout, "\n" );
    
    fprintf( stdout, "Block Entries: " );
    displayWord( mem -> getMemCtrlReg( MC_REG_BLOCK_ENTRIES ), rdx );
    fprintf( stdout, ", Block Size: " );
    displayWord( mem -> getMemCtrlReg( MC_REG_BLOCK_SIZE ), rdx );
    fprintf( stdout, ", Sets: " );
    displayWord( mem -> getMemCtrlReg( MC_REG_SETS ), rdx );
    fprintf( stdout, "\n" );
}

void DrvLineDisplay::displayTlbObjRegSet( CpuTlb *tlb, int rdx ) {
    
    fprintf( stdout, "Display TLB reg set ... to do ... \n" );
}
    
void DrvLineDisplay::displayAllRegSets( int rdx ) {
    
    displayGeneralRegSet( rdx );
    fprintf( stdout, "\n" );
    displaySegmentRegSet( rdx );
    fprintf( stdout, "\n" );
    displayControlRegSet( rdx );
    fprintf( stdout, "\n" );
    displayPStateRegSet( rdx );
    fprintf( stdout, "\n" );
    displayPlRegSets( rdx );
    fprintf( stdout, "\n" );
}

//------------------------------------------------------------------------------------------------------------
// This routine will print a TLB entry with each field formatted.
//
//------------------------------------------------------------------------------------------------------------
void DrvLineDisplay::displayTlbEntry( TlbEntry *entry, int rdx ) {
    
    fprintf( stdout, "[" );
    if ( entry -> tValid( ))            fprintf( stdout, "V" ); else fprintf( stdout, "v" );
    if ( entry -> tDirty( ))            fprintf( stdout, "D" ); else fprintf( stdout, "d" );
    if ( entry -> tTrapPage( ))         fprintf( stdout, "P" ); else fprintf( stdout, "p" );
    if ( entry -> tTrapDataPage( ))     fprintf( stdout, "D" ); else fprintf( stdout, "d" );
    fprintf( stdout, "]" );
    
    fprintf( stdout, " Acc: (%d,%d,%d)", entry -> tPageType( ), entry -> tPrivL1( ), entry -> tPrivL2( ));
    
    fprintf( stdout,  " Pid: " );
    displayHalfWord( entry -> tSegId( ), rdx );
    
    fprintf( stdout, " Vpn-H: " );
    displayWord( entry -> vpnHigh, rdx );
    
    fprintf( stdout, " Vpn-L: " );
    displayWord( entry -> vpnLow, rdx );
    
    fprintf( stdout, " PPN: " );
    displayHalfWord( entry -> tPhysPage( ), rdx  );
}

//------------------------------------------------------------------------------------------------------------
// "displayTlbEntries" displays a set of TLB entries, line by line.
//
//------------------------------------------------------------------------------------------------------------
void DrvLineDisplay::displayTlbEntries( CpuTlb *tlb, uint32_t index, uint32_t len, int rdx ) {
    
    if ( index + len <= tlb -> getTlbSize( )) {
        
        for ( uint32_t i = index; i < index + len; i++  ) {
            
            displayWord( i, rdx  );
            fprintf( stdout, ": " );
            
            TlbEntry *ptr = tlb -> getTlbEntry( i );
            if ( ptr != nullptr ) displayTlbEntry( ptr, rdx );
            
            fprintf( stdout, "\n" );
        }
        
    } else fprintf( stdout, "index + len out of range\n" );
}

//------------------------------------------------------------------------------------------------------------
// "displayCacheEntries" displays a list of cache line entries. Since we have a coupe of block sizes and
// perhaps one or more sets, the display is rather complex.
//
//------------------------------------------------------------------------------------------------------------
void DrvLineDisplay::displayCacheEntries( CpuMem *cPtr, uint32_t index, uint32_t len, int rdx ) {
    
    uint32_t    blockSets       = cPtr -> getBlockSets( );
    uint32_t    wordsPerBlock   = cPtr -> getBlockSize( ) / 4;
    uint32_t    wordsPerLine    = 4;
    uint32_t    linesPerBlock   = wordsPerBlock / wordsPerLine;
   
    if ( index + len >=  cPtr -> getBlockEntries( )) {
        
        fprintf( stdout, " cache index + len out of range\n" );
        return;
    }
    
    for ( uint32_t lineIndex = index; lineIndex < index + len; lineIndex++  ) {
        
        displayWord( lineIndex, rdx  );
        fprintf( stdout, ": " );
        
        if ( blockSets >= 1 ) {
            
            MemTagEntry *tagPtr     = cPtr -> getMemTagEntry( lineIndex, 0 );
            uint32_t    *dataPtr    = (uint32_t *) cPtr -> getMemBlockEntry( lineIndex, 0 );
            
            fprintf( stdout, "(0)[" );
            if ( tagPtr -> valid )  fprintf( stdout, "V" ); else fprintf( stdout, "v" );
            if ( tagPtr -> dirty )  fprintf( stdout, "D" ); else fprintf( stdout, "d" );
            fprintf( stdout, "] (" );
            displayWord( tagPtr -> tag, rdx );
            fprintf( stdout, ") \n" );
            
            for ( int i = 0; i < linesPerBlock; i++  ) {
                
                fprintf( stdout, "            (" );
                
                for ( int j = 0; j < wordsPerLine; j++ ) {
                    
                    displayWord( dataPtr[ ( i * wordsPerLine ) + j ], rdx );
                    if ( i < 3 ) fprintf( stdout, " " );
                }
                
                fprintf( stdout, ") \n" );
            }
        }
        
        if ( blockSets >= 2 ) {
            
            MemTagEntry *tagPtr     = cPtr -> getMemTagEntry( lineIndex, 0 );
            uint32_t    *dataPtr    = (uint32_t *) cPtr -> getMemBlockEntry( lineIndex, 1 );
            
            fprintf( stdout, "            (1)[" );
            if ( tagPtr -> valid )  fprintf( stdout, "V" ); else fprintf( stdout, "v" );
            if ( tagPtr -> dirty )  fprintf( stdout, "D" ); else fprintf( stdout, "d" );
            fprintf( stdout, "] (" );
            displayWord( tagPtr -> tag, rdx );
            fprintf( stdout, ")\n" );
            
            for ( int i = 0; i < linesPerBlock; i++  ) {
                
                fprintf( stdout, "            (" );
                
                for ( int j = 0; j < wordsPerLine; j++ ) {
                    
                    displayWord( dataPtr[ ( i * wordsPerLine ) + j ], rdx );
                    if ( i < 3 ) fprintf( stdout, " " );
                }
                
                fprintf( stdout, ") \n" );
            }
        }
    }
}

//------------------------------------------------------------------------------------------------------------
// Display absolute memory content. We will show the memory starting with offset. The words per line is an
// environmental variable setting. The offset is rounded down to the next 4-byte boundary, the limit is
// rounded up to the next 4-byte boundary. We display the data in words. The absolute memory address range
// currently consist of three memory objects. There is main physical memory, PDC memory and IO memory. This
// routine will make the appropriate call.
//
//------------------------------------------------------------------------------------------------------------
void  DrvLineDisplay::displayAbsMemContent( uint32_t ofs, uint32_t len, int rdx ) {
    
    uint32_t    index           = ( ofs / 4 ) * 4;
    uint32_t    limit           = ((( index + len ) + 3 ) / 4 ) * 4;
    int         wordsPerLine    = glb -> env_n -> getEnvVarInt((char *) ENV_WORDS_PER_LINE );
    CpuMem      *physMem        = glb -> cpu -> physMem;
    CpuMem      *pdcMem         = glb -> cpu -> pdcMem;
    CpuMem      *ioMem          = glb -> cpu -> ioMem;
    
    while ( index < limit ) {
        
        glb -> lineDisplay -> displayWord( index, rdx );
        fprintf( stdout, ": " );
        
        for ( uint32_t i = 0; i < wordsPerLine; i++ ) {
            
            if ( index < limit ) {
                
                if ((physMem != nullptr ) && ( physMem -> validAdr( index ))) {
                    
                    glb -> lineDisplay -> displayWord( physMem -> getMemDataWord( index ), rdx );
                }
                else if (( pdcMem != nullptr ) && ( pdcMem -> validAdr( index ))) {
                    
                    glb -> lineDisplay -> displayWord( pdcMem -> getMemDataWord( index ), rdx );
                }
                else if (( ioMem != nullptr ) && ( ioMem -> validAdr( index ))) {
                    
                    glb -> lineDisplay -> displayWord( ioMem -> getMemDataWord( index ), rdx );
                }
                else displayInvalidWord( rdx );
            }
                
            fprintf( stdout, " " );
            
            index += 4;
        }
        
        fprintf( stdout, "\n" );
    }
    
    fprintf( stdout, "\n" );
}

//------------------------------------------------------------------------------------------------------------
// Display absolute memory content as code shown in assembler syntax. There is one word per line.
//
//------------------------------------------------------------------------------------------------------------
void  DrvLineDisplay::displayAbsMemContentAsCode( uint32_t ofs, uint32_t len, int rdx ) {
    
    uint32_t    index           = ( ofs / 4 ) * 4;
    uint32_t    limit           = ((( index + len ) + 3 ) / 4 );
    CpuMem      *physMem        = glb -> cpu -> physMem;
    CpuMem      *pdcMem         = glb -> cpu -> pdcMem;
    CpuMem      *ioMem          = glb -> cpu -> ioMem;
 
    while ( index < limit ) {
        
        glb -> lineDisplay -> displayWord( index, rdx );
        fprintf( stdout, ": " );
        
        if (( physMem != nullptr ) && ( physMem -> validAdr( index ))) {
            
            glb -> disAsm -> displayInstr( physMem -> getMemDataWord( index ), rdx );
        }
        else if (( pdcMem != nullptr ) && ( pdcMem -> validAdr( index ))) {
                
            glb -> disAsm -> displayInstr( pdcMem -> getMemDataWord( index ), rdx );
        }
        else if (( ioMem != nullptr ) && ( ioMem -> validAdr( index ))) {
                
            glb -> disAsm -> displayInstr( ioMem -> getMemDataWord( index ), rdx );
        }
        else displayInvalidWord( rdx );
        
        fprintf( stdout, "\n" );
            
        index += 1;
    }
       
    fprintf( stdout, "\n" );
}


