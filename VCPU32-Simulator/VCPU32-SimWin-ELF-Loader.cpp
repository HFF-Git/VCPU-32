//------------------------------------------------------------------------------------------------------------
//
//  VCPU32 - A 32-bit CPU - ELF file loader
//
//------------------------------------------------------------------------------------------------------------
// The ELF file loader will load an executable file into the simulator physical memory. It is right now
// a rather simple loader intended for loading an initial program. No virtual memory setup, no access rights
// checking and so on. Just plain load into physcial memory what éver you find in the ELF file.
//
//------------------------------------------------------------------------------------------------------------
//
// VCPU32 - A 32-bit CPU - ELF file loader
// Copyright (C) 2025 - 2025 Helmut Fieres
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
#include "VCPU32-SimDeclarations.h"

using namespace ELFIO;

//------------------------------------------------------------------------------------------------------------
// Local name space.
//
//------------------------------------------------------------------------------------------------------------
namespace {

//------------------------------------------------------------------------------------------------------------
// A little helper to convert the byte order.
//
//------------------------------------------------------------------------------------------------------------
inline uint32_t swap32(uint32_t val) {
    
    return ((val & 0xFF) << 24) |
           ((val & 0xFF00) << 8) |
           ((val & 0xFF0000) >> 8) |
           ((val & 0xFF000000) >> 24);
}

//------------------------------------------------------------------------------------------------------------
// Open and close the ELF file. On opening we also check that it is a Big Endian type file.
//
//------------------------------------------------------------------------------------------------------------
elfio *openElfFile( char *fileName ) {
    
    ELFIO::elfio *reader = new ( std::nothrow ) elfio;
    
    if ( ! reader -> load( fileName )) throw( ERR_INVALID_ELF_FILE );
    if ( reader -> get_encoding( ) != ELFDATA2MSB ) throw( ERR_INVALID_ELF_BYTE_ORDER );
    return( reader );
}

void closeElfFile( elfio *reader ) {
    
    delete (elfio*) reader;
}

//------------------------------------------------------------------------------------------------------------
// Validate the ELF file. ELFIO does the jib and returns an error message string if there are errors.
//
//------------------------------------------------------------------------------------------------------------
bool elfioValidate( elfio *reader, char* msg, int msg_len ) {
    
    std::string error = reader -> validate( );
    
    if ( msg != nullptr && msg_len > 0 ) {
        
        strncpy( msg, error.c_str( ), (size_t) msg_len - 1 );
    }
    
    return error.empty( );
}

//------------------------------------------------------------------------------------------------------------
// Write a word to the simulator memory.
//
//------------------------------------------------------------------------------------------------------------
bool writeMem( CpuCore *cpu, uint32_t ofs, uint32_t val ) {
    
    CpuMem      *physMem    = cpu -> physMem;
    CpuMem      *pdcMem     = cpu -> pdcMem;
    CpuMem      *ioMem      = cpu -> ioMem;
    CpuMem      *mem        = nullptr;
    
    if      (( physMem != nullptr ) && ( physMem -> validAdr( ofs ))) mem = physMem;
    else if (( pdcMem  != nullptr ) && ( pdcMem  -> validAdr( ofs ))) mem = pdcMem;
    else if (( ioMem   != nullptr ) && ( ioMem   -> validAdr( ofs ))) mem = ioMem;
    
    if (((uint64_t) ofs + 4 ) > MAX_MEMORY_SIZE ) throw ( ERR_OFS_LEN_LIMIT_EXCEEDED );
    
    mem -> putMemDataWord( ofs, val );
    
    return( true );
}

//------------------------------------------------------------------------------------------------------------
// Load a segment into main memory. We are passed the segment and the CPU handle. Currently we only load
// physical memory. First we get the segment attributes and validate them for size, etc. Next we clear the
// physical memory in the size of what it sould be according to the segment data. Next, we copy the segment
// data word by word up to the segment file size attribute. Note that a segment needs to have loadable data.
// Since our memory access is on a word basis, there is one more thing. The data is correctly encoded in
// big endian format. Howeber, the coercion from that byte array to a word array will place the data in the
// host system order, i.e. littlle endian. We need to swap each word accordigly.
//
//------------------------------------------------------------------------------------------------------------
void loadSegmentIntoMemory( elfio *reader, segment *segment, CpuCore *cpu, SimWinOutBuffer *winOut ) {
    
    if ( segment ->get_type( ) == PT_LOAD ) {
      
        Elf_Xword       index       = segment -> get_index( );
        Elf_Xword       fileSize    = segment -> get_file_size( );
        Elf_Xword       memorySize  = segment -> get_memory_size( );
        const char      *dataPtr    = segment -> get_data( );
        const uint32_t  *wordPtr    = reinterpret_cast<const uint32_t*> ( dataPtr );
        Elf64_Addr      vAdr        = segment -> get_physical_address( );
        Elf_Xword       align       = segment -> get_align( );
        
        winOut -> printChars( "Loading: Seg: %2d, adr: 0x%08x, mSize: 0x%08x, align: 0x%08x\n",
                              index, vAdr, memorySize, align );
        
        if ( memorySize >= MAX_MEMORY_SIZE ) {
            
            throw( ERR_ELF_MEMORY_SIZE_EXCEEDED );
        }
        
        if ( ! (( vAdr >= 0 ) && ( vAdr<= MAX_MEMORY_SIZE ))) {
            
            throw( ERR_ELF_INVALID_ADR_RANGE );
        }
        
        if ( vAdr + memorySize >= MAX_MEMORY_SIZE ) {
            
            throw( ERR_ELF_MEMORY_SIZE_EXCEEDED );
        }
            
        for ( Elf64_Addr i = 0; i < memorySize; i += 4  ) {
            
            writeMem( cpu, uint32_t(vAdr + i), 0 );
        }
        
        for ( Elf64_Addr i = 0; i < fileSize; i += 4  ) {
            
            writeMem( cpu, uint32_t( vAdr + i ), swap32( wordPtr[ i / 4 ] ));
        }
    }
}

} // namespace


//------------------------------------------------------------------------------------------------------------
// Loading a basic ELF file. This routine is rather simple. All we do is to locate the segments and load
// them into physical memory. Could be refined and do more checking one day.
//
//------------------------------------------------------------------------------------------------------------
void SimCommandsWin::loadElfFile( char *fileName ) {
    
    elfio *reader = nullptr;
    char  errMsgBuf[ 256 ];
    
    try {
        
        winOut -> printChars( "Loading %s\n", fileName );
        
        reader = openElfFile( fileName );
      
        if ( ! elfioValidate( reader, errMsgBuf, sizeof( errMsgBuf ))) {
            
            winOut -> printChars( "ELF: %s\n", errMsgBuf );
            throw( 99 );
        }
        
        Elf_Half numOfSeg = reader -> segments.size( );
        
        for ( int i = 0; i < numOfSeg; i++ ) {
            
            loadSegmentIntoMemory( reader, reader -> segments[ i ], glb -> cpu, winOut );
        }
        
        Elf64_Addr entry = reader -> get_entry( );
        
        winOut -> printChars( "Set entry: 0x%08x\n", entry );
        glb -> cpu -> setReg( RC_FD_PSTAGE, PSTAGE_REG_ID_PSW_0, (uint32_t) 0 );
        glb -> cpu -> setReg( RC_FD_PSTAGE, PSTAGE_REG_ID_PSW_1, (uint32_t) entry );
        
        winOut -> printChars( "Done\n" );
    }
    
    catch ( SimErrMsgId errNum ) {
        
        winOut -> printChars( "ELF file load error: %d\n", errNum );
    }
    
    if ( reader != nullptr ) closeElfFile( reader );
}
