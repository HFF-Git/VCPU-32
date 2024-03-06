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
#include "VCPU32-Types.hpp"
#include "VCPU32-Core.hpp"
#include "VCPU32-Driver.hpp"

//------------------------------------------------------------------------------------------------------------
// The object constructor. We just remember the globals pointer.
//
//------------------------------------------------------------------------------------------------------------
DrvLineDisplay::DrvLineDisplay( VCPU32Globals *glb ) {
    
    this -> glb = glb;
}

//------------------------------------------------------------------------------------------------------------
// "displayWord" lists out a 24-bit machine word in the specified number base. If the format parameter is
// omitted or set to "default", the environment variable for the base number is used.
//
//------------------------------------------------------------------------------------------------------------
void DrvLineDisplay::displayWord( uint32_t val, TokId fmtType ) {
    
    if      ( fmtType == TOK_DEC )  fprintf( stdout, "%8d", val );
    else if ( fmtType == TOK_OCT )  fprintf( stdout, "%#09o", val );
    else if ( fmtType == TOK_HEX )  {
        
        if ( val == 0 ) fprintf( stdout, "0x000000" );
        else fprintf( stdout, "%#08x", val );
    }
    else fprintf( stdout, "*******" );
}

//------------------------------------------------------------------------------------------------------------
// "displayHalfWord" lists out a 12-bit word in the specified number base. If the format parameter is omitted
// or set to "default", the environment variable for the base number is used.
//
//------------------------------------------------------------------------------------------------------------
void DrvLineDisplay::displayHalfWord( uint32_t val, TokId fmtType ) {
    
    if      ( fmtType == TOK_DEC )  fprintf( stdout, "%4d", val );
    else if ( fmtType == TOK_OCT  ) fprintf( stdout, "%04o", val );
    else if ( fmtType == TOK_HEX )  {
        
        if ( val == 0 ) fprintf( stdout, "0x000" );
        else fprintf( stdout, "%#04x", val );
    }
    else fprintf( stdout, "**num***" );
}

//------------------------------------------------------------------------------------------------------------
// "displayRegsAndLabel" is a little building block to display a set of registers and a label. For exmaple:
// "GR0=  xxx xxx xxx". It is an internal use only function to allow for the display of registers with some
// options on title and how many are displayed in a row. Note that this routine does not a lot of checking.
//
//------------------------------------------------------------------------------------------------------------
void DrvLineDisplay::displayRegsAndLabel( RegClass regSetId,
                                              int       regStart,
                                              int       numOfRegs,
                                              char      *lineLabel,
                                              TokId     fmt ) {
    
    if ( strlen( lineLabel ) > 0 ) fprintf( stdout, "%s", lineLabel );
    
    for ( int i = regStart; i < regStart + numOfRegs; i++ ) {
        
        displayWord( glb -> cpu -> getReg( regSetId, i ), fmt );
        if ( i < regStart + numOfRegs ) fprintf( stdout, " " );
    }
}

//------------------------------------------------------------------------------------------------------------
// Display register set functions. The CPU has several register sets starting with the general registers, the
// segment register up to the control, memory, TLB and pipeline registers. The following routines will just
// display a particular register set.
//
//------------------------------------------------------------------------------------------------------------
void DrvLineDisplay::displayGeneralRegSet( TokId fmt ) {
    
    displayRegsAndLabel( RC_GEN_REG_SET, 0, 8, ((char *) "R0=   " ), fmt );
    fprintf( stdout, "\n" );
    displayRegsAndLabel( RC_GEN_REG_SET, 8, 8, ((char *) "R8=   " ), fmt );
    fprintf( stdout, "\n" );
}

void DrvLineDisplay::displaySegmentRegSet( TokId fmt ) {
    
    displayRegsAndLabel( RC_SEG_REG_SET, 0, 8, ((char *) "S0=   " ), fmt );
    fprintf( stdout, "\n" );
}

void DrvLineDisplay::displayControlRegSet( TokId fmt ) {
    
    displayRegsAndLabel( RC_CTRL_REG_SET, 0, 8, ((char *) "CR0=  " ), fmt );
    fprintf( stdout, "\n" );
    
    displayRegsAndLabel( RC_CTRL_REG_SET, 8, 8, ((char *) "CR8=  " ), fmt );
    fprintf( stdout, "\n" );
    
    displayRegsAndLabel( RC_CTRL_REG_SET, 16, 8, ((char *) "CR16= " ), fmt );
    fprintf( stdout, "\n" );
    
    displayRegsAndLabel( RC_CTRL_REG_SET, 24, 8, ((char *) "CR24= " ), fmt );
    fprintf( stdout, "\n" );
}

void DrvLineDisplay::displayPStateRegSet( TokId fmt ) {
    
    fprintf( stdout, "IA=   " );
    displayWord( glb -> cpu -> getReg( RC_PROG_STATE, PS_REG_IA_SEG ), fmt );
    fprintf( stdout, "." );
    displayWord( glb -> cpu -> getReg( RC_PROG_STATE, PS_REG_IA_OFS ), fmt );
    fprintf( stdout, ", ST= " );
    displayWord( glb -> cpu -> getReg( RC_PROG_STATE, PS_REG_STATUS ), fmt );
    fprintf( stdout, "\n" );
}

void DrvLineDisplay::displayPlIFetchDecodeRegSet( TokId fmt ) {
    
    if ( glb -> cpu -> getReg( RC_FD_PSTAGE, PSTAGE_REG_STALLED ) == 1 )
        fprintf( stdout, "FD(S): IA=" );
    else
        fprintf( stdout, "FD:    IA=" );
    
    displayWord( glb -> cpu -> getReg( RC_FD_PSTAGE, PSTAGE_REG_ID_IA_SEG ), fmt );
    fprintf( stdout, "." );
    displayWord( glb -> cpu -> getReg( RC_FD_PSTAGE, PSTAGE_REG_ID_IA_OFS ), fmt );
    fprintf( stdout, "\n" );
}

void DrvLineDisplay::displayPlMemoryAccessRegSet( TokId fmt ) {
    
    if ( glb -> cpu -> getReg( RC_MA_PSTAGE, PSTAGE_REG_STALLED ) == 1 )
        fprintf( stdout, "MA(S): IA=" );
    else
        fprintf( stdout, "MA:    IA=" );
    
    displayWord( glb -> cpu -> getReg( RC_MA_PSTAGE, PSTAGE_REG_ID_IA_SEG ), fmt );
    fprintf( stdout, "." );
    displayWord( glb -> cpu -> getReg( RC_MA_PSTAGE, PSTAGE_REG_ID_IA_OFS ), fmt );
    fprintf( stdout, " I=" );
    displayWord( glb -> cpu -> getReg( RC_MA_PSTAGE, PSTAGE_REG_ID_INSTR ), fmt );
    fprintf( stdout, " A=" );
    displayWord( glb -> cpu -> getReg( RC_MA_PSTAGE, PSTAGE_REG_ID_VAL_A ), fmt );
    fprintf( stdout, " B=" );
    displayWord( glb -> cpu -> getReg( RC_MA_PSTAGE, PSTAGE_REG_ID_VAL_B ), fmt );
    fprintf( stdout, " X=" );
    displayWord( glb -> cpu -> getReg( RC_MA_PSTAGE, PSTAGE_REG_ID_VAL_X ), fmt );
    fprintf( stdout, "\n" );
}

void DrvLineDisplay::displayPlExecuteRegSet( TokId fmt ) {
    
    if ( glb -> cpu -> getReg( RC_EX_PSTAGE, PSTAGE_REG_STALLED ) == 1 )
        fprintf( stdout, "EX(S): IA=" );
    else
        fprintf( stdout, "EX:    IA=" );
    
    displayWord( glb -> cpu -> getReg( RC_EX_PSTAGE, PSTAGE_REG_ID_IA_SEG ), fmt );
    fprintf( stdout, "." );
    displayWord( glb -> cpu -> getReg( RC_EX_PSTAGE, PSTAGE_REG_ID_IA_OFS ), fmt );
    fprintf( stdout, " I=" );
    displayWord( glb -> cpu -> getReg( RC_EX_PSTAGE, PSTAGE_REG_ID_INSTR ), fmt );
    fprintf( stdout, " A=" );
    displayWord( glb -> cpu -> getReg( RC_EX_PSTAGE, PSTAGE_REG_ID_VAL_A ), fmt );
    fprintf( stdout, " B=" );
    displayWord( glb -> cpu -> getReg( RC_EX_PSTAGE, PSTAGE_REG_ID_VAL_B ), fmt );
    fprintf( stdout, " X=" );
    displayWord( glb -> cpu -> getReg( RC_EX_PSTAGE, PSTAGE_REG_ID_VAL_X ), fmt );
    fprintf( stdout, "\n" );
}

void DrvLineDisplay::displayPlRegSets( TokId fmt ) {
    
    displayPlIFetchDecodeRegSet( fmt );
    displayPlMemoryAccessRegSet( fmt );
    displayPlExecuteRegSet( fmt );
}

void DrvLineDisplay::displayMemObjRegSet( CPU24Mem *mem, TokId fmt ) {
    
    fprintf( stdout, "State:   %s\n", mem -> getMemOpStr( mem -> getMemCtrlReg( MC_REG_STATE )));
  
    fprintf( stdout, "Seg:ofs: " );
    displayWord( mem -> getMemCtrlReg( MC_REG_REQ_SEG ), fmt );
    fprintf( stdout, ":" );
    displayWord( mem -> getMemCtrlReg( MC_REG_REQ_OFS ), fmt );
    fprintf( stdout, ", Tag: " );
    displayWord( mem -> getMemCtrlReg( MC_REG_REQ_TAG ), fmt );
    fprintf( stdout, ", Len: " );
    displayWord( mem -> getMemCtrlReg( MC_REG_REQ_LEN ), fmt );
    fprintf( stdout, "\n" );
    
    fprintf( stdout, "Block Entries: " );
    displayWord( mem -> getMemCtrlReg( MC_REG_BLOCK_ENTRIES ), fmt );
    fprintf( stdout, ", Block Size: " );
    displayWord( mem -> getMemCtrlReg( MC_REG_BLOCK_SIZE ), fmt );
    fprintf( stdout, ", Sets: " );
    displayWord( mem -> getMemCtrlReg( MC_REG_SETS ), fmt );
    fprintf( stdout, "\n" );
}

void DrvLineDisplay::displayTlbObjRegSet( CpuTlb *tlb, TokId fmt ) {
    
    fprintf( stdout, "Display TLB reg set ... to do ... \n" );
}
    
void DrvLineDisplay::displayAllRegSets( TokId fmt ) {
    
    displayGeneralRegSet( fmt );
    fprintf( stdout, "\n" );
    displaySegmentRegSet( fmt );
    fprintf( stdout, "\n" );
    displayControlRegSet( fmt );
    fprintf( stdout, "\n" );
    displayPStateRegSet( fmt );
    fprintf( stdout, "\n" );
    displayPlRegSets( fmt );
}

//------------------------------------------------------------------------------------------------------------
// This routine will print a TLB entry with each field formatted.
//
//------------------------------------------------------------------------------------------------------------
void DrvLineDisplay::displayTlbEntry( TlbEntry *entry, TokId fmt ) {
    
    fprintf( stdout, "[" );
    if ( entry -> tValid( ))            fprintf( stdout, "V" ); else fprintf( stdout, "v" );
    if ( entry -> tUncachable( ))       fprintf( stdout, "U" ); else fprintf( stdout, "u" );
    if ( entry -> tDirty( ))            fprintf( stdout, "D" ); else fprintf( stdout, "d" );
    if ( entry -> tTrapPage( ))         fprintf( stdout, "P" ); else fprintf( stdout, "p" );
    if ( entry -> tTrapDataPage( ))     fprintf( stdout, "D" ); else fprintf( stdout, "d" );
    if ( entry -> tModifyExPage( ))     fprintf( stdout, "X" ); else fprintf( stdout, "x" );
    fprintf( stdout, "]" );
    
    fprintf( stdout, " Acc: (%d,%d,%d)", entry -> tPageType( ), entry -> tPrivL1( ), entry -> tPrivL2( ));
    
    fprintf( stdout,  " Pid: " );
    displayHalfWord( entry -> tProtectId( ), fmt );
    
    fprintf( stdout, " Vpn-H: " );
    displayWord( entry -> vpnHigh, fmt );
    
    fprintf( stdout, " Vpn-L: " );
    displayWord( entry -> vpnLow, fmt );
    
    fprintf( stdout, " Bank: " );
    displayHalfWord( entry -> tPhysMemBank( ), fmt );
    
    fprintf( stdout, " PPN: " );
    displayHalfWord( entry -> tPhysPage( ), fmt  );
}

//------------------------------------------------------------------------------------------------------------
// "displayTlbEntries" displays a set of TLB entries, line by line.
//
//------------------------------------------------------------------------------------------------------------
void DrvLineDisplay::displayTlbEntries( CpuTlb *tlb, uint32_t index, uint32_t len, TokId fmt ) {
    
    if ( index + len <= tlb -> getTlbSize( )) {
        
        for ( uint32_t i = index; i < index + len; i++  ) {
            
            displayWord( i, fmt  );
            fprintf( stdout, ": " );
            
            TlbEntry *ptr = tlb -> getTlbEntry( i );
            if ( ptr != nullptr ) displayTlbEntry( ptr, fmt );
            
            fprintf( stdout, "\n" );
        }
    }
    else fprintf( stdout, "index + len out of range\n" );
}

//------------------------------------------------------------------------------------------------------------
// "displayCacheEntries" displays a list of cache line entries. Since we have a coupe of block sizes and
// perhaps one or more sets, the display is rather complex.
//
// ??? what to simplify, refine ?
// ??? what can be factored out to display a cache line ?
//------------------------------------------------------------------------------------------------------------
void DrvLineDisplay::displayCacheEntries( CPU24Mem *cPtr, uint32_t index, uint32_t len, TokId fmt ) {
    
    uint32_t            blockEntries    = cPtr -> getBlockEntries( );
    uint32_t            blockSize       = cPtr -> getBlockSize( );
    uint16_t            blockSets       = cPtr -> getBlockSets( );
    MemTagEntry    *tagPtr         = nullptr;
    uint32_t        *dataPtr        = nullptr;
   
    if ( index + len <=  blockEntries ) {
        
        for ( uint32_t i = index; i < index + len; i++  ) {
            
            displayWord( i, fmt  );
            fprintf( stdout, ": " );
            
            if ( blockSets >= 1 ) {
                
                tagPtr  = cPtr -> getMemTagEntry( index, 0 );
                dataPtr = cPtr -> getMemBlockEntry( index, 0 );
     
                fprintf( stdout, "(0)[" );
                if ( tagPtr -> valid )  fprintf( stdout, "V" ); else fprintf( stdout, "v" );
                if ( tagPtr -> dirty )  fprintf( stdout, "D" ); else fprintf( stdout, "d" );
                fprintf( stdout, "] (" );
                displayWord( tagPtr -> tag, fmt );
                fprintf( stdout, ")\n" );
                
                uint32_t subLineIndex = 0;
                
                while ( subLineIndex < blockSize ) {
                    
                   fprintf( stdout, "           (" );
                    for ( uint32_t i = 0; i < 4; i++ ) {
                        
                        displayWord( dataPtr[ i ], fmt );
                        if ( i < 3 ) fprintf( stdout, " " );
                    }
                    
                   fprintf( stdout, ") \n" );
                    
                    subLineIndex += 4;
                }
            }
            
            if ( blockSets >= 2 ) {
                
                tagPtr  = cPtr -> getMemTagEntry( index, 1 );
                dataPtr = cPtr -> getMemBlockEntry( index, 1 );
     
                fprintf( stdout, "           (1)[" );
                if ( tagPtr -> valid )  fprintf( stdout, "V" ); else fprintf( stdout, "v" );
                if ( tagPtr -> dirty )  fprintf( stdout, "D" ); else fprintf( stdout, "d" );
                fprintf( stdout, "] (" );
                displayWord( tagPtr -> tag, fmt );
                fprintf( stdout, ")\n" );
                
                uint32_t subLineIndex = 0;
                
                while ( subLineIndex < blockSize ) {
                    
                   fprintf( stdout, "           (" );
                    for ( uint32_t i = 0; i < 4; i++ ) {
                        
                        displayWord( dataPtr[ i ], fmt );
                        if ( i < 3 ) fprintf( stdout, " " );
                    }
                    
                   fprintf( stdout, ") \n" );
                    
                    subLineIndex += 4;
                }
            }
        }
    }
    else fprintf( stdout, " cache index + len out of range\n" );
}

//------------------------------------------------------------------------------------------------------------
// Display physical memory content. We will show the memory starting with offset. The words per line is an
// environmental variable setting.
//
//------------------------------------------------------------------------------------------------------------
void  DrvLineDisplay::displayPmemContent( uint32_t ofs, uint32_t len, TokId fmtId ) {
    
    uint32_t        index           = ofs;
    uint32_t        limit           = ofs + len;
    int             wordsPerLine    = glb -> env -> getEnvValInt( ENV_WORDS_PER_LINE );
    
    uint32_t        blockEntries    = glb -> cpu -> mem -> getBlockEntries( );
    uint32_t        blockSize       = glb -> cpu -> mem -> getBlockSize( );
    uint32_t        memSize         = blockEntries * blockSize;
    CPU24Mem        *mem            = glb -> cpu -> mem;
    uint32_t    *dataPtr        = nullptr;
    
    if ( limit >= memSize ) return;
    
    while ( index < limit ) {
        
        glb -> lineDisplay -> displayWord( index, fmtId );
        fprintf( stdout, ": " );
        
        dataPtr = mem -> getMemBlockEntry( index / blockSize ) + ( index % blockSize );
        
        if ( dataPtr != nullptr ) {
            
            for ( uint32_t i = 0; i < wordsPerLine; i++ ) {
                
                if ( index < limit ) glb -> lineDisplay -> displayWord( * ( dataPtr + i ), fmtId );
                fprintf( stdout, " " );
            }
            
            fprintf( stdout, "\n" );
        }
        
        index = index + wordsPerLine;
    }
    
    fprintf( stdout, "\n" );
}
