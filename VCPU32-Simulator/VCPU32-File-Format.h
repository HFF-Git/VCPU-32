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
// Segment Types.
//
//------------------------------------------------------------------------------------------------------------
enum SegmentTypes : uint16_t {
    
    SEG_T_NIL               = 0,
    SEG_T_CODE              = 1,
    SEG_T_GLOBAL_DATA       = 2,
    SEG_T_PRIV_DATA         = 3
  
};

enum SegmentFlags : uint16_t {
    
    SEG_F_NIL               = 0
    
};

//------------------------------------------------------------------------------------------------------------
// Section Types.
//
//------------------------------------------------------------------------------------------------------------
enum SectionTypes : uint16_t {
    
    SEC_T_NIL               = 0,
    
    SEC_T_COMP_UNIT         = 1,
    
    SEC_T_GLOBAL_INIT       = 10,
    SEC_T_GLOBAL_NON_INIT   = 11,
    SEC_T_PRIV_INIT         = 12,
    SEC_T_PRIV_NON_INIT     = 13,
    
};

enum SectionFlags : uint16_t {
    
    SEC_F_NIL               = 0
    
};

//------------------------------------------------------------------------------------------------------------
// A relocatable library will just have a list of relocatable file content. The library header describs the
// the content.
//
// ... tbd
//------------------------------------------------------------------------------------------------------------
struct LibraryHeader {
    
    uint32_t        magicWord;                  // quick check if the file is a VCPU32 file.
    FileTypes       fileType;                   // VCPU32 file type.
    uint16_t        fileVersion;                // VCPU32 file version.
    
    // ... more to come ...
    
    uint32_t        headerCheckSum;             // Checksum of header exclusing the checkSum field.
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
    
    uint32_t        segTabOfs;                  // Header relative offset of the segement table.
    uint32_t        segTabEntrySize;            // Segement table entry size.
    uint32_t        segTabQuantity;             // Total number of segment.
        
    uint32_t        secTabOfs;                  // Header relative offset of the section table.
    uint32_t        secTabEntrySize;            // Section table entry size.
    uint32_t        secTabQuantity;             // Total number of sections.
   
    uint32_t        segSecStringsOfs;           // Header relative offset for segment and section strings table.
    uint32_t        segSecStringsTabLen;        // Name table length.
    
    uint32_t        entrySegIndex;              // Segment index of segment with program entry.
    uint32_t        entySecIndex;               // Section index of segment with program entry.
    uint32_t        entryOfs;                   // Header relative offset of prgram entry.
        
    // ... more to come ...

    uint32_t        headerCheckSum;             // Checksum of header exclusing the checkSum field.
};

//------------------------------------------------------------------------------------------------------------
// Each file contains a segment table with all the segments defined in this file.
//
//------------------------------------------------------------------------------------------------------------
struct SegmentEntry {
    
    SegmentFlags    flags;                      // Sgement flags.
    SegmentTypes    type;                       // Segment type.
    uint32_t        nameOfs;                    // Header relative offset to the name in name Tab.
    uint32_t        segIndex;                   // Index of segment im segment tab.
    uint32_t        secIndex;                   // Index of section start in section tab.
    uint32_t        secQuantity;                // Number of section in segment.
    uint32_t        segSize;                    // Total size of segment.
    
    // ... more to come ...
};

//------------------------------------------------------------------------------------------------------------
// A segment is further divided into sections.
//
//------------------------------------------------------------------------------------------------------------
struct SectionEntry {
    
    SectionFlags    flags;                      // Section Flags.
    SectionTypes    type;                       // Section type.
    uint32_t        nameOfs;                    // Header relative offset to the name in name Tab.
    uint32_t        segIndex;                   // Index of segment in segment tab.
    uint32_t        secIndex;                   // Index of section start in section tab.

    uint32_t        secSegOfs;                  // Segment relative offset of section in segment.
    uint32_t        secSize;                    // Segment relative offset of section in segment.
    uint16_t        secAlignment;               // Alignment of the section.
    
    uint32_t        secFileOfs;                 // Header relative offset of the section in the file.
    
    // ... more to come ...
};

// ... strings are of the form "len:chars:0" ? We need to be able to quickly scan for a symbol...

// ... more to come ...


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
    
    uint8_t     lookupSegmentEntryIndex( uint32_t fIndex,
                                        char *segName,
                                        uint32_t *index );
    
    uint8_t     lookupSectionEntryIndex( uint32_t fIndex,
                                        uint32_t segIndex,
                                        char *secName,
                                        uint32_t *index );
    
    uint8_t     readSegmentEnry( uint32_t fileIndex,
                                uint32_t segIndex,
                                SegmentEntry *entry );
    
    uint8_t     readSectionEnry( uint32_t fileIndex,
                                uint32_t secIndex,
                                SegmentEntry *entry );
    
    
    
    // ... and so on ...
    
private:
    
};

#endif


