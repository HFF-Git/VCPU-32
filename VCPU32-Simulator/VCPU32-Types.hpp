//------------------------------------------------------------------------------------------------------------
//
// VCPU32 - A 32-bit CPU - General Types
//
//------------------------------------------------------------------------------------------------------------
//
// VCPU32 - A 32-bit CPU - General Types
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
#ifndef VCPU32Types_h
#define VCPU32Types_h

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <stdarg.h>
#include <iostream>

//------------------------------------------------------------------------------------------------------------
// Fundamental Constantse.
//
//------------------------------------------------------------------------------------------------------------
const uint32_t  WORD_SIZE               = 32U;
const uint32_t  HALF_WORD_SIZE          = 16U;
const uint32_t  BYTE_SIZE               = 8U;

const uint32_t  MAX_GREGS               = 16;
const uint32_t  MAX_SREGS               = 8;
const uint32_t  MAX_CREGS               = 32;

const uint32_t  PAGE_SIZE               = 4096U;
const uint32_t  PAGE_SIZE_BITS          = 12U;
const uint32_t  PAGE_BIT_MASK           = (( 1U << 12 ) - 1 );

const uint32_t  MAX_BANKS               = 16U;
const uint32_t  MAX_CPU_IDS             = 16U;

const uint32_t  MAX_MEMORY_SIZE         = UINT32_MAX;
const uint32_t  MAX_IO_MEM_SIZE         = UINT32_MAX / 16;
const uint32_t  MAX_PHYS_MEM_SIZE       = UINT32_MAX - MAX_IO_MEM_SIZE;

const uint8_t   MAX_TRAP_ID             = 32;
const uint8_t   TRAP_CODE_BLOCK_SIZE    = 32;


//------------------------------------------------------------------------------------------------------------
// Status register fields. The status word is a machine word contains various bits and field recording the
// current execution state.
//
// ??? note: under construction .... always cross check with the document.
//------------------------------------------------------------------------------------------------------------
enum StatusRegFields : uint32_t {
    
    ST_INTERRUPT_ENABLE             = ( 1U << 0 ),
    ST_DATA_TRANSLATION_ENABLE      = ( 1U << 1 ),
    ST_PROTECT_ID_CHECK_ENABLE      = ( 1U << 2 ),
   
    ST_CARRY                        = ( 1U << 15 ),
    
    ST_EXECUTION_LEVEL              = ( 1U << 29 ),
    ST_CODE_TRANSLATION_ENABLE      = ( 1U << 30 ),
    ST_MACHINE_CHECK                = ( 1U << 31 )
};

//------------------------------------------------------------------------------------------------------------
// Program State register identifiers.
//
//------------------------------------------------------------------------------------------------------------
enum ProgStateRegisterId : uint32_t {
    
    PS_REG_IA_SEG = 0,
    PS_REG_IA_OFS = 1,
    PS_REG_STATUS = 2
};

//------------------------------------------------------------------------------------------------------------
// Control register identifiers.
//
//------------------------------------------------------------------------------------------------------------
enum ControlRegisterId : uint32_t {
    
    CR_RECOVERY_CNTR    = 0x0,
    CR_SHIFT_AMOUNT     = 0x1,
    CR_RSV_2            = 0x2,
    CR_RSV_3            = 0x3,
    CR_RSV_4            = 0x4,
    CR_RSV_5            = 0x5,
    CR_RSV_6            = 0x6,
    CR_RSV_7            = 0x7,
    CR_PROTECT_ID1      = 0x8,
    CR_PROTECT_ID2      = 0x9,
    CR_PROTECT_ID3      = 0xA,
    CR_PROTECT_ID4      = 0xB,
    CR_RSV_12           = 0xC,
    CR_RSV_13           = 0xD,
    CR_RSV_14           = 0xE,
    CR_RSV_15           = 0xF,
    CR_TRAP_VECTOR_ADR  = 0x10,
    CR_TRAP_INSTR_SEG   = 0x11,
    CR_TRAP_INSTR_OFS   = 0x12,
    CR_TRAP_STAT        = 0x13,
    CR_TRAP_PARM_1      = 0x14,
    CR_TRAP_PARM_2      = 0x15,
    CR_TRAP_PARM_3      = 0x16,
    CR_RSV_22           = 0x17,
    CR_TEMP_0           = 0x18,
    CR_TEMP_1           = 0x19,
    CR_TEMP_2           = 0x1A,
    CR_TEMP_3           = 0x1B,
    CR_TEMP_4           = 0x1C,
    CR_TEMP_5           = 0x1D,
    CR_TEMP_6           = 0x1E,
    CR_TEMP_7           = 0x1F
};

// ??? to core ?
//------------------------------------------------------------------------------------------------------------
// Each pipeline consists of a "combinatorial logic" part and the pipeline registers. Our pipeline registers
// are just a set of registers.
//
//------------------------------------------------------------------------------------------------------------
enum PipeLineStageRegId : uint32_t {
    
    PSTAGE_REG_STALLED      = 0,
    PSTAGE_REG_ID_IA_OFS    = 1,
    PSTAGE_REG_ID_IA_SEG    = 2,
    PSTAGE_REG_ID_INSTR     = 3,
    PSTAGE_REG_ID_VAL_A     = 4,
    PSTAGE_REG_ID_VAL_B     = 5,
    PSTAGE_REG_ID_VAL_X     = 6,
    PSTAGE_REG_ID_VAL_S     = 7,
    PSTAGE_REG_ID_VAL_ST    = 8,
    PSTAGE_REG_ID_RID_A     = 9,
    PSTAGE_REG_ID_RID_B     = 10,
    PSTAGE_REG_ID_RID_X     = 11
};

//------------------------------------------------------------------------------------------------------------
// Traps. There are three classes of traps. The first type is the general TRAP. In general, it is a situation
// when an instruction detected an issue and cannot run until some intervention took place. Also, there are
// traps that should then run after the instruction executed. INTERRUPTS are events taken after the execution
// of an instruction completed and an external interrupt is pending. CHECK traps are bad news and the CPU
// cannot continue at all. In an emulator, they most likely do not occur, but in a simulator we could simulate
// a hardware situation.
//
//------------------------------------------------------------------------------------------------------------
enum TrapId : uint32_t {
    
    NO_TRAP                 = 0,
    MACHINE_CHECK           = 1,
    PHYS_ADDRESS_CHECK      = 2,
    EXT_INTERRUPT           = 3,
    ILLEGAL_INSTR_TRAP      = 4,
    PRIV_OPERATION_TRAP     = 5,
    OVERFLOW_TRAP           = 6,
    
    INSTR_MEM_PROTECT_TRAP  = 7,
    DATA_MEM_PROTECT_TRAP   = 8,
   
    ITLB_MISS_TRAP          = 10,
    ITLB_ACC_RIGHTS_TRAP    = 11,
    ITLB_PROTECT_ID_TRAP    = 12,
    ITLB_NON_ACCESS_TRAP    = 13,
    
    DTLB_MISS_TRAP          = 14,
    DTLB_ACC_RIGHTS_TRAP    = 15,
    DTLB_PROTECT_ID_TRAP    = 16,
    DTLB_NON_ACCESS_TRAP    = 17,
    
    BREAK_TRAP              = 18,
};


//------------------------------------------------------------------------------------------------------------
// A memory reference is checked for access type. The acces types specify the read, write and execute
// operations allowed for the target address.
//
//------------------------------------------------------------------------------------------------------------
enum AccessModes : uint32_t {
    
    ACC_READ_ONLY   = 0,
    ACC_READ_WRITE  = 1,
    ACC_EXECUTE     = 2,
    ACC_GATEWAY     = 3
};

//------------------------------------------------------------------------------------------------------------
// Compare condition code field values. For the comparisons test result of less than, greater than, less or
// equal than and greater or equal than, there are signed and unsigned comparison codes.
//
//------------------------------------------------------------------------------------------------------------
enum CompareConditionCodes : uint32_t {
    
    CC_EQ   = 0x0,  // a == b
    CC_LT   = 0x1,  // a <  b
    CC_GT   = 0x2,  // a >  b
    CC_LS   = 0x3,  // a <  b, Unsigned
    CC_NE   = 0x4,  // a != b
    CC_LE   = 0x5,  // a <= b
    CC_GE   = 0x6,  // a >= b
    CC_HI   = 0x7,  // a >  b, Unsigned
};

//------------------------------------------------------------------------------------------------------------
// Test condition code field values.
//
//------------------------------------------------------------------------------------------------------------
enum TestConditionCodes : uint32_t {
    
    TC_EQ   = 0x0,  // b = 0
    TC_LT   = 0x1,  // b < 0, Signed
    TC_GT   = 0x2,  // b > 0, Signed
    TC_EV   = 0x3,  // b is even
    TC_NE   = 0x4,  // b != 0
    TC_LE   = 0x5,  // b <= 0, Signed
    TC_GE   = 0x6,  // b >= 0, Signed
    TC_OD   = 0x7   // b is odd
};

//------------------------------------------------------------------------------------------------------------
// Operand mode codes. The operand in an instruction consists of an addressing mode field and the mode
// depending argument. There are 8 addressing modes:
//
//  ADR_MODE_IMM      - The argument is a 9 bit sign extended field.
//  ADR_MODE_REG      - The argument contains a zero in A and general register number in B.
//  ADR_MODE_TWO_REGS - The argument field just contains two general registers in A and B.
//  ADR_MODE_EXT_ADR  - The argument contains an offset, a segment in A, an offset in B.
//  ADR_MODE_INDX_GR4  - The base registers is GR4. The argument field contains a signed offset to be added.
//  ADR_MODE_INDX_GR5  - dito for GR5.
//  ADR_MODE_INDX_GR6  - dito for GR6.
//  ADR_MODE_INDX_GR7  - dito for GR7.
//
//------------------------------------------------------------------------------------------------------------
enum OpMode : uint32_t {
    
    ADR_MODE_IMM         = 0x0,
    ADR_MODE_REG         = 0x1,
    ADR_MODE_TWO_REGS    = 0x2,
    ADR_MODE_EXT_ADR     = 0x3,
    ADR_MODE_INDX_GR4    = 0x4,
    ADR_MODE_INDX_GR5    = 0x5,
    ADR_MODE_INDX_GR6    = 0x6,
    ADR_MODE_INDX_GR7    = 0x7
};

//------------------------------------------------------------------------------------------------------------
// Machine instruction opCodes. The first 6 bits of the instruction word are reserved for the opCode field.
// Depending on the type of instruction, not all bits are used though. Each instruction will be described in
// their defining function implementation. Note that most instruction are rather versatile and replace also
// other commonly found instructions. For example, the boolean negate can be done with an AND instruction
// with the negate flag set. Shifts and rotates can be handled by the bit field manipulation instructions.
//
//------------------------------------------------------------------------------------------------------------
enum InstrOpCode : uint8_t {
    
    // ??? group one - opMode group...
    
    OP_ADD          = 0x10,     // target = target + operand ;options for carry, ovl trap, etc.
    OP_SUB          = 0x11,     // target = target - operand ;options for carry, ovl trap, etc.
    OP_AND          = 0x12,     // target = target & operand ; option to negate the result
    OP_OR           = 0x13,     // target = target | operand ; option to negate the result
    OP_XOR          = 0x14,     // target = target ^ operand ; option to negate the result
    OP_CMP          = 0x15,     // subtract reg2 from reg1 and set condition codes
    OP_LEA          = 0x16,     // load effective address offset
    OP_LSID         = 0x17,     // load segment ID register
    
    OP_LD           = 0x18,     // target = [ operand ]   // ??? covers LDW, LDH, LDB
    OP_ST           = 0x19,     // [ operand ] = target   // ??? covers STW, STH, STB
    OP_LDWR         = 0x1A,     // load word referenced
    OP_STWC         = 0X1B,     // store word conditional
    
    OP_RSV_1C       = 0x1C,     // reserved
    OP_RSV_1D       = 0x1D,     // reserved
    OP_RSV_1E       = 0x1E,     // reserved
    OP_RSV_1F       = 0x1F,     // reserved
    
    // group two - memory reference
    
    OP_LDWE         = 0x20,     // load word from virtual address
    OP_LDHE         = 0x21,     // load hald-word from virtual address
    OP_LDBE         = 0x22,     // load byte from virtual address
    OP_STWE         = 0x23,     // store word to virtual adress
    OP_STHE         = 0x24,     // store half-word to virtual adress
    OP_STBE         = 0x25,     // store byte to virtual adress
    
    OP_LDWA         = 0x26,     // load word from absolute address
    OP_LDHA         = 0x27,     // load hald-word from absolute address
    OP_LDBA         = 0x28,     // load byte from absolute address
    OP_STWA         = 0x29,     // store word to absolute adress
    OP_STHA         = 0x2A,     // store half-word to absolute adress
    OP_STBA         = 0x2B,     // store byte to absolute adress
    
    OP_LDPA         = 0x2C,     // load physical address
    OP_PRB          = 0x2D,     // probe access
    
    OP_RSV_2E       = 0x2E,     // reserved
    OP_RSV_2F       = 0x2F,     // reserved
    
    // group three - branch group
    
    OP_B            = 0x30,     // branch
    OP_BL           = 0x31,     // branch and link
    OP_GATE         = 0x32,     // gateway instruction
    OP_BR           = 0x33,     // branch register
    OP_BLR          = 0x34,     // branch and link register
    OP_BV           = 0x35,     // branch vectored
    OP_BVR          = 0x36,     // branch vectored register
    OP_BE           = 0x37,     // branch external
    OP_BLE          = 0x38,     // branch and link external
    OP_CBR          = 0x39,     // compare and branch
    OP_TBR          = 0x3A,     // test and branch
    
    OP_RSV_3B       = 0x3B,     // reserved
    OP_RSV_3C       = 0x3C,     // reserved
    OP_RSV_3D       = 0x3D,     // reserved
    OP_RSV_3E       = 0x3E,     // reserved
    OP_RSV_3F       = 0x3F,     // reserved
    
    // group zero - specials...
    
    OP_BRK          = 0x00,     // break for debug
    OP_DIAG         = 0x01,     // diagnostics instruction, tbd.
    
    OP_LDI          = 0x02,     // load immediate
    OP_EXTR         = 0x03,     // extract bit field of operand
    OP_DEP          = 0x04,     // extract bit field into operand
    OP_DSR          = 0x05,     // double register shift right
    OP_SHLA         = 0x06,     // shift left and add
    OP_RSV_07       = 0x07,     // reserved for divide step support ...
    OP_CMR          = 0x0B,     // conditional move register or value
    
    OP_MR           = 0x09,     // move to or from a segment or control register
    OP_MST          = 0x0A,     // set or clear status bits
    
    OP_RSV_0B       = 0x0B,     // reserved
   
    OP_ITLB         = 0x0C,     // insert into TLB
    OP_PTLB         = 0x0D,     // remove from TLB
    OP_PCA          = 0x0E,     // purge and flush cache
    OP_RFI          = 0x0F,     // return from interrupt

};

//------------------------------------------------------------------------------------------------------------
// During the instruction execution, there is a lot to check about the instructions defined. To speed up the
// process, each instruction and any special attribute to know about it is stored in a literal table. For
// each opCode there is a table entry which contains the opCode itself and flags that describe the
// instruction. These flags are used by the stages to identify characteristics of the instruction instead of
// long "if" or "switch" statements to test an instruction.
//
//------------------------------------------------------------------------------------------------------------
enum instrFlags : uint32_t {
    
    NO_FLAGS            = 0,
    COMP_INSTR          = ( 1U << 0 ),
    LOAD_INSTR          = ( 1U << 1 ),
    STORE_INSTR         = ( 1U << 2 ),
    BRANCH_INSTR        = ( 1U << 3 ),
    CTRL_INSTR          = ( 1U << 4 ),
    OP_MODE_INSTR       = ( 1U << 5 ),
    REG_R_INSTR         = ( 1U << 6 ),
    PRIV_INSTR          = ( 1U << 7 )
};

//------------------------------------------------------------------------------------------------------------
// The instruction decoder needs to do a lot of checking on the opcode. Naturally. The following flags help
// to simplifiy this checking. Each instruction is classified with the coresponding flags.
//
// ??? decide which way to go... no flags or all kinds of flags...
//------------------------------------------------------------------------------------------------------------
// ??? rework this table ... or get rid of it ...

const struct opCodeInfo {
    
    char        mnemonic[ 8 ] ;
    uint8_t     opCode;
    uint32_t    flags;
    
} opCodeTab[ ] = {
    
    /* 0x00 */  { "BRK",    OP_BRK,     ( CTRL_INSTR ) },
    /* 0x01 */  { "DIAG",   OP_DIAG,    ( CTRL_INSTR ) },
    /* 0x02 */  { "LDI",    OP_LDI,     ( COMP_INSTR | REG_R_INSTR ) },
    /* 0x03 */  { "EXTR",   OP_EXTR,    ( COMP_INSTR | REG_R_INSTR ) },
    /* 0x04 */  { "DEP",    OP_DEP,     ( COMP_INSTR | REG_R_INSTR ) },
    /* 0x05 */  { "DSR",    OP_DSR,     ( COMP_INSTR | REG_R_INSTR ) },
    /* 0x06 */  { "SHLA",   OP_SHLA,    ( COMP_INSTR | REG_R_INSTR ) },
    /* 0x07 */  { "RSV_07", OP_SHLA,    ( NO_FLAGS ) },
    /* 0x08 */  { "CMR",    OP_CMR,     ( COMP_INSTR | REG_R_INSTR ) },
    /* 0x09 */  { "MR",     OP_MR,      ( CTRL_INSTR ) },
    /* 0x0A */  { "MST",    OP_MST,     ( CTRL_INSTR | PRIV_INSTR | REG_R_INSTR ) },
    /* 0x0B */  { "RSV_0B", OP_SHLA,    ( NO_FLAGS ) },
    /* 0x0C */  { "ITLB",   OP_ITLB,    ( CTRL_INSTR | PRIV_INSTR ) },
    /* 0x0D */  { "PTLB",   OP_PTLB,    ( CTRL_INSTR | PRIV_INSTR ) },
    /* 0x0E */  { "PCA",    OP_PCA,     ( CTRL_INSTR ) },
    /* 0x0F */  { "RFI",    OP_RFI,     ( CTRL_INSTR | PRIV_INSTR ) },
    
    /* 0x10 */  { "ADD",    OP_ADD,     ( COMP_INSTR | OP_MODE_INSTR | REG_R_INSTR ) },
    /* 0x11 */  { "SUB",    OP_SUB,     ( COMP_INSTR | OP_MODE_INSTR | REG_R_INSTR ) },
    /* 0x12 */  { "AND",    OP_AND,     ( COMP_INSTR | OP_MODE_INSTR | REG_R_INSTR ) },
    /* 0x13 */  { "OR",     OP_OR,      ( COMP_INSTR | OP_MODE_INSTR | REG_R_INSTR ) },
    /* 0x14 */  { "XOR",    OP_XOR,     ( COMP_INSTR | OP_MODE_INSTR | REG_R_INSTR ) },
    /* 0x15 */  { "CMP",    OP_CMP,     ( COMP_INSTR | OP_MODE_INSTR | REG_R_INSTR ) },
    /* 0x16 */  { "LEA",    OP_LEA,     ( COMP_INSTR | OP_MODE_INSTR | REG_R_INSTR ) },
    /* 0x17 */  { "LSID",   OP_LSID,    ( COMP_INSTR | OP_MODE_INSTR | REG_R_INSTR ) },
    /* 0x18 */  { "LD",     OP_LD,      ( LOAD_INSTR  | OP_MODE_INSTR | REG_R_INSTR ) },
    /* 0x19 */  { "ST",     OP_ST,      ( STORE_INSTR | OP_MODE_INSTR ) },
    /* 0x1A */  { "LDWR",   OP_LDWR,    ( LOAD_INSTR  | OP_MODE_INSTR | REG_R_INSTR ) },
    /* 0x1B */  { "STWC",   OP_STWC,    ( STORE_INSTR | OP_MODE_INSTR ) },
    /* 0x1C */  { "RSV_1C", OP_RSV_1C,  ( NO_FLAGS ) },
    /* 0x1D */  { "RSV_1D", OP_RSV_1D,  ( NO_FLAGS ) },
    /* 0x1E */  { "RSV_1E", OP_RSV_1E,  ( NO_FLAGS ) },
    /* 0x1F */  { "RSV_1F", OP_RSV_1F,  ( NO_FLAGS ) },
    
    /* 0x20 */  { "LDWE",   OP_LDWE,    ( LOAD_INSTR  | REG_R_INSTR  ) },
    /* 0x21 */  { "LDHE",   OP_LDHE,    ( LOAD_INSTR  | REG_R_INSTR  ) },
    /* 0x22 */  { "LDBE",   OP_LDBE,    ( LOAD_INSTR  | REG_R_INSTR  ) },
    /* 0x23 */  { "STWE",   OP_STWE,    ( STORE_INSTR ) },
    /* 0x24 */  { "STHE",   OP_STHE,    ( STORE_INSTR ) },
    /* 0x25 */  { "STBE",   OP_STBE,    ( STORE_INSTR ) },
    /* 0x26 */  { "LDWA",   OP_LDWA,    ( LOAD_INSTR  | PRIV_INSTR | REG_R_INSTR  ) },
    /* 0x27 */  { "LDHA",   OP_LDHA,    ( LOAD_INSTR  | PRIV_INSTR | REG_R_INSTR  ) },
    /* 0x28 */  { "LDBA",   OP_LDBA,    ( LOAD_INSTR  | PRIV_INSTR | REG_R_INSTR  ) },
    /* 0x29 */  { "STWA",   OP_STWA,    ( STORE_INSTR | PRIV_INSTR ) },
    /* 0x2A */  { "STHA",   OP_STHA,    ( STORE_INSTR | PRIV_INSTR ) },
    /* 0x2B */  { "STBA",   OP_STBA,    ( STORE_INSTR | PRIV_INSTR ) },
    /* 6x2C */  { "LDPA",   OP_LDPA,    ( LOAD_INSTR | PRIV_INSTR | REG_R_INSTR ) },
    /* 6x2D */  { "PRB",    OP_PRB,     ( CTRL_INSTR | REG_R_INSTR ) },
    /* 0x2E */  { "RSV_2E", OP_RSV_2E,  ( NO_FLAGS ) },
    /* 0x2F */  { "RSV_2F", OP_RSV_2F,  ( NO_FLAGS ) },
    
    /* 0x30 */  { "B",      OP_B,       ( BRANCH_INSTR ) },
    /* 0x31 */  { "BL",     OP_BL,      ( BRANCH_INSTR | REG_R_INSTR ) },
    /* 0x32 */  { "GATE",   OP_GATE,    ( CTRL_INSTR | BRANCH_INSTR | REG_R_INSTR ) },
    /* 0x33 */  { "BR",     OP_BR,      ( BRANCH_INSTR ) },
    /* 0x34 */  { "BLR",    OP_BLR,     ( BRANCH_INSTR | REG_R_INSTR ) },
    /* 0x35 */  { "BV",     OP_BV,      ( BRANCH_INSTR ) },
    /* 0x36 */  { "BVR",    OP_BVR,     ( BRANCH_INSTR ) },
    /* 0x37 */  { "BE",     OP_BE,      ( BRANCH_INSTR ) },
    /* 0x38 */  { "BLE",    OP_BLE,     ( BRANCH_INSTR | REG_R_INSTR ) },
    /* 0x39 */  { "CBR",    OP_CBR,     ( BRANCH_INSTR ) },
    /* 0x3A */  { "TBR",    OP_TBR,     ( BRANCH_INSTR ) },
   
    /* 0x3B */  { "RSV_3B", OP_RSV_3B,  ( NO_FLAGS ) },
    /* 0x3C */  { "RSV_3C", OP_RSV_3C,  ( NO_FLAGS ) },
    /* 0x3D */  { "RSV_3D", OP_RSV_3D,  ( NO_FLAGS ) },
    /* 0x3E */  { "RSV_3E", OP_RSV_3E,  ( NO_FLAGS ) },
    /* 0x3F */  { "RSV_3F", OP_RSV_3F,  ( NO_FLAGS ) }
};

//------------------------------------------------------------------------------------------------------------
// Instruction decoding means to extract a lot of different bit fields from an instruction word. To extract
// there are numerous routines available. They all have the form of "xxxField". All routines are used by the
// simulator as well as development tools such as the assembler. All instruction fields will be defined here
// and should be used throughout the development.
//
// The are common fields, such as the opCode or a regId field. They are just labeled this way. For example,
// the opcode is "opCodeField". There are also instruction specific fields. They start with the instructions
// that use them. For example the bit position in the extract and deposit instruction is "extrDepPosField".
// To simplify the decoding, a macro will accept position and length and perform the extract.
//
//------------------------------------------------------------------------------------------------------------
#define MASK( m ) (( 1U << m ) - 1 )
#define EXTR( i, p, l ) ((( i ) >> ( 31 - p )) & ( MASK( l )))
#define DEP( i, a, p, l ) i = ((( i ) & (( ~ MASK( l )) << ( 31 - p ))) | (( a ) & (( MASK( l )) << ( 31 - p ))));

struct Instr {
  
public:
   
    static inline uint32_t  opCodeField( uint32_t instr )           { return( EXTR( instr, 5, 6 )); }
    static inline uint32_t  regRIdField( uint32_t instr )           { return( EXTR( instr, 9, 4 )); }
    static inline uint32_t  regAIdField( uint32_t instr )           { return( EXTR( instr, 27, 4 )); }
    static inline uint32_t  regBIdField( uint32_t instr )           { return( EXTR( instr, 31, 4 )); }
    static inline uint32_t  opModeField( uint32_t instr )           { return( EXTR( instr, 17, 5 )); }
    static inline uint32_t  cmpCondField( uint32_t instr )          { return( EXTR( instr, 12, 3 )); }
    static inline uint32_t  cmrCondField( uint32_t instr )          { return( EXTR( instr, 12, 3 )); }
    static inline uint32_t  cbrCondField( uint32_t instr )          { return( EXTR( instr, 8, 3 )); }
    static inline uint32_t  tbrCondField( uint32_t instr )          { return( EXTR( instr, 8, 3 )); }
    static inline uint32_t  immOfsSignField( uint32_t instr )       { return( EXTR( instr, 18, 1 )); }
    static inline uint32_t  arithOpFlagField( uint32_t instr )      { return( EXTR( instr, 12, 3 )); }
    static inline uint32_t  boolOpFlagField( uint32_t instr )       { return( EXTR( instr, 12, 3 )); }
    static inline uint32_t  extrDepLenField( uint32_t instr )       { return( EXTR( instr, 22, 5 )); }
    static inline uint32_t  extrDepPosField( uint32_t instr )       { return( EXTR( instr, 27, 5 )); }
    static inline uint32_t  dsrSaField( uint32_t instr )            { return( EXTR( instr, 22, 5 )); }
    static inline bool      shlaUseImmField( uint32_t instr )       { return( EXTR( instr, 22, 2 )); }
    static inline bool      useCarryField( uint32_t instr )         { return( EXTR( instr, 10, 1 )); }
    static inline bool      logicalOpField( uint32_t instr )        { return( EXTR( instr, 11, 1 )); }
    static inline bool      trapOvlField( uint32_t instr )          { return( EXTR( instr, 12, 1 )); }
    static inline bool      negateResField( uint32_t instr )        { return( EXTR( instr, 10, 1 )); }
    static inline bool      complRegBField( uint32_t instr )        { return( EXTR( instr, 11, 1 )); }
    static inline bool      extrSignedField( uint32_t instr )       { return( EXTR( instr, 10, 1 )); }
    static inline bool      depInZeroField( uint32_t instr )        { return( EXTR( instr, 10, 1 )); }
    static inline bool      depImmOptField( uint32_t instr )        { return( EXTR( instr, 11, 1 )); }
    static inline uint32_t  shlaSaField( uint32_t instr )           { return( EXTR( instr, 22, 2 )); }
    static inline bool      ldiZeroField( uint32_t instr )          { return( EXTR( instr, 10, 1 )); }
    static inline bool      ldiLeftField( uint32_t instr )          { return( EXTR( instr, 11, 1 )); }
    static inline bool      mrMovDirField( uint32_t instr )         { return( EXTR( instr, 10, 1 )); }
    static inline uint32_t  mrArgField( uint32_t instr )            { return( EXTR( instr, 31, 6 )); }
    static inline uint32_t  mstModeField( uint32_t instr )          { return( EXTR( instr, 11, 2 )); }
    static inline uint32_t  mstArgField( uint32_t instr )           { return( EXTR( instr, 31, 6 )); }
    static inline bool      prbAdrModeField( uint32_t instr )       { return( EXTR( instr, 10, 1 )); }
    static inline bool      prbRwAccField( uint32_t instr )         { return( EXTR( instr, 11, 1 )); }
    static inline bool      ldpaAdrModeField( uint32_t instr )      { return( EXTR( instr, 10, 1 )); }
    static inline bool      tlbAdrModeField( uint32_t instr )       { return( EXTR( instr, 10, 1 )); }
    static inline bool      tlbKindField( uint32_t instr )          { return( EXTR( instr, 11, 1 )); }
    static inline bool      tlbArgModeField( uint32_t instr )       { return( EXTR( instr, 12, 1 )); }
    static inline bool      pcaAdrModeField( uint32_t instr )       { return( EXTR( instr, 10, 1 )); }
    static inline bool      pcaKindField( uint32_t instr )          { return( EXTR( instr, 11, 1 )); }
    static inline bool      pcaPurgeFlushField( uint32_t instr )    { return( EXTR( instr, 12, 1 )); }
    
    static inline uint32_t  segSelect( uint32_t arg )               { return ( EXTR( arg, 1, 2 )); }
    static inline uint32_t  ofsSelect( uint32_t arg )               { return ( EXTR( arg, 31, 30 )); }
    
    
    // ??? new immediates ...
    
    static inline uint32_t immGen0S14( uint32_t instr ) {
        
        uint32_t tmp = EXTR( instr, 31, 14 );
        return ( EXTR( instr, 18, 1 ) ? ( tmp | 0xFFFFC000 ) : ( tmp ));
    }
    
    static inline uint32_t immGen0S6( uint32_t instr ) {
        
        uint32_t tmp = EXTR( instr, 23, 6 ) >> 8;
        return ( EXTR( instr, 18, 1 ) ? ( tmp | 0xFFFFFFC0 ) : ( tmp ));
    }
    
    static inline uint32_t immGen2S14( uint32_t instr ) {
      
        uint32_t tmp = ( EXTR( instr, 17, 2 )) | ( EXTR( instr, 31, 14 ) << 2 );
        return ( EXTR( instr, 18, 1 ) ? ( tmp | 0xFFFF0000 ) : ( tmp ));
    }
    
    static inline uint32_t immGen8S6( uint32_t instr ) {
      
        uint32_t tmp = ( EXTR( instr, 17, 8 ) >> 14 ) | ( EXTR( instr, 23, 6 ) << 6 );
        return ( EXTR( instr, 18, 1 ) ? ( tmp | 0xFFFFC000 ) : ( tmp ));
    }
    
    static inline uint32_t immGen8S10( uint32_t instr ) {
      
        uint32_t tmp = ( EXTR( instr, 17, 8 ) >> 14 ) | ( EXTR( instr, 27, 10 ) << 6 );
        return ( EXTR( instr, 18, 1 ) ? ( tmp | 0xFFFFC000 ) : ( tmp ));
    }
    
    static inline uint32_t immGen8S14( uint32_t instr ) {
      
        uint32_t tmp = ( EXTR( instr, 17, 8 ) >> 14 ) | ( EXTR( instr, 31, 14 ) << 8 );
        return ( EXTR( instr, 18, 1 ) ? ( tmp | 0xFFFFC000 ) : ( tmp ));
    }
    
    static inline uint32_t immGen9S6( uint32_t instr ) {
      
        uint32_t tmp = ( EXTR( instr, 17, 9 ) >> 14 ) | ( EXTR( instr, 23, 6 ) << 1 );
        return ( EXTR( instr, 18, 1 ) ? ( tmp | 0xFFFFC000 ) : ( tmp ));
    }
    
    static inline uint32_t immGen12S6( uint32_t instr ) {
      
        uint32_t tmp = ( EXTR( instr, 17, 12 ) >> 14 ) | ( EXTR( instr, 23, 6 ) << 4 );
        return ( EXTR( instr, 18, 1 ) ? ( tmp | 0xFFFFC000 ) : ( tmp ));
    }
    
    
    
    // ??? old immediates - phase out ...
    
    
    static inline uint32_t immGen0S3( uint32_t instr ) {
        
        uint32_t tmp = (( instr >> 6 ) & 07 );
        return (( instr & 0400 ) ? ( tmp | 077777770 ) : ( tmp ));
    }
    
    static inline uint32_t immGen0S9( uint32_t instr ) {
        
        return (( instr & 0400 ) ? ( instr | 077777000 ) : ( instr & 0777 ));
    }
    
    static inline uint32_t immGen30S3( uint32_t instr ) {
        
        uint32_t tmp = (( instr >> 12 ) & 07 ) | (( instr >> 3 ) & 070 );
        return (( instr & 0400 ) ? ( tmp | 077777700 ) : ( tmp ));
    }
    
    static inline uint32_t immGen30S9( uint32_t instr ) {
        
        uint32_t tmp = (( instr >> 12 ) & 07 ) | (( instr << 3 ) & 07770 );
        return (( instr & 0400 ) ? ( tmp | 077770000 ) : ( tmp ));
    }
    
    static inline uint32_t immGen6S3( uint32_t instr ) {
        
        uint32_t tmp = (( instr >> 9 ) & 077 ) | ( instr & 0700 );
        return (( instr & 0400 ) ? ( tmp | 077777000 ) : ( tmp ));
    }
    
    static inline uint32_t immGen6S6( uint32_t instr ) {
        
        uint32_t tmp = (( instr >> 9 ) & 077 ) | (( instr << 3 ) & 07700 );
        return (( instr & 0400 ) ? ( tmp | 077770000 ) : ( tmp ));
    }
    
    static inline uint32_t immGen6S9( uint32_t instr ) {
        
        uint32_t tmp = (( instr >> 9 ) & 077 ) | (( instr << 6 ) & 077700 );
        return (( instr & 0400 ) ? ( tmp | 077700000 ) : ( tmp ));
    }
    
    static inline uint32_t immGen9S3( uint32_t instr ) {
        
        uint32_t tmp = (( instr >> 9 ) & 0777 ) | (( instr << 3 ) & 007000 );
        return (( instr & 0400 ) ? ( tmp | 077770000 ) : ( tmp ));
    }
    
    
    // ??? this will become easier / go away ?
        
    static inline uint32_t iaOfsPrivField( uint32_t arg ) {
        
        return ( arg >> 23 );
    }
    
    static inline uint32_t add22ForInstrAdr( uint32_t arg1, uint32_t arg2 ) {
        
        return ((( arg1 & 017777777 ) + ( arg2 & 017777777 )) | ( arg1 & 60000000 ));
    }
    
    static inline uint32_t add22( uint32_t arg1, uint32_t arg2 ) {
        
        return (( arg1 + arg2 ) & 017777777 );
    }
    
    static inline uint32_t add24( uint32_t arg1, uint32_t arg2 ) {
        
        return (( arg1 + arg2 ) & 077777777 );
    }
};
 
#endif
