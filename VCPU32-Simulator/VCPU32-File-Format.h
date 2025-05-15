//------------------------------------------------------------------------------------------------------------
//
// VCPU32 - File Format Definitions
//
//------------------------------------------------------------------------------------------------------------
// VCPU32 features a siomple file format, in close alignment to the ELF file format. There is a requirement
// for bootable, executable and relocatable files.
//
//
//------------------------------------------------------------------------------------------------------------
//
// VCPU32 - A 32-bit CPU - IO Subsystem definitions
// Copyright (C) 2022 - 2025 Helmut Fieres
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
#ifndef VCPU32_FileFormat_h
#define VCPU32_FileFormat_h

//------------------------------------------------------------------------------------------------------------
// File Types.
//
//------------------------------------------------------------------------------------------------------------
enum FileTypes : uint16_t {
    
    FT_NIL                  = 0,
    FT_BOOT_IMAGE           = 1,
    FT_EXECUTABLE           = 2,
    FT_RELOCATABLE          = 3,
    FT_EXECUTABLE_LIB       = 4,
    FT_RELOCATABLE_LIB      = 5,
};

//------------------------------------------------------------------------------------------------------------
// File Flags.
//
//------------------------------------------------------------------------------------------------------------
enum FileFlags : uint16_t {
    
  FF_NO_FLAGS       = 0x0000,
  FF_LITTLE_ENDIAN  = 0x0001
    
};

//------------------------------------------------------------------------------------------------------------
// A VCPU32 file starts with a header. All byte offsets are relative to the header offset. Note that the
// there could be more than one relocatable file in a library file, which is why the offset are relative to
// the particular file offset.
//
//------------------------------------------------------------------------------------------------------------
struct FileHeader {
    
    uint32_t        magicWord;                  // quick check if the file is a VCPU32 file.
    FileTypes       fileType;                   // VCPU32 file type.
    uint16_t        fileVersion;                // VCPU32 file version.
    uint16_t        fileFlags;                  // File flags.
    
    uint32_t        auxHeaderOfs;               // Header relative offset of the segement table.
    uint32_t        auxHeaderLen;               // Header relative offset of the segement table.
    
    uint32_t        dataAreaOfs;                // Header relative offset of the data area.
    uint32_t        dataAreaLen;                // Header relative offset of the data area.
    
    uint32_t        codeAreaOfs;                // Header relative offset of the code area.
    uint32_t        codeAreaLen;                // Header relative offset of the code area.

    uint32_t        headerCheckSum;             // Checksum of header exclusing the checkSum field.
};

//------------------------------------------------------------------------------------------------------------
//
//
// ??? to be defined ...
//------------------------------------------------------------------------------------------------------------
struct AuxHeader {
    
    uint32_t        headerFlags;                // TBD
    
    uint32_t        dataSeg;                    // Segment number where the global data is.
    uint32_t        dataOfs;                    // Offset in that segment.
    uint32_t        dataSegLen;                 // Length of data segment
    
    uint32_t        codeSeg;                    // Segment number where the entry point is.
    uint32_t        codeOfs;                    // Offset in that segment.
    uint32_t        codeSegLen;                 // Length of code segment
    
    uint32_t        entrySeg;                   // Segment number where the entry point is.
    uint32_t        entryOfs;                   // Entry point offset.
};



//------------------------------------------------------------------------------------------------------------
//
//
// ... one idea: build an object that contains all the methods to conveniently access and deploy file
// data.
//------------------------------------------------------------------------------------------------------------
struct VCPU32_ObjectFile {
  
public:
    
    uint8_t     openFile( char *filePath );
    uint8_t     closeFile( );
    
    uint8_t     readFileHeader( FileHeader );
    uint8_t     readAuxHeader( FileHeader );
    
    
    // ... and so on ...
    
private:
    
};

#endif


