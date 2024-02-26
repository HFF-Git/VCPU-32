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
const uint32_t  WORD_SIZE       = 32U;
const uint32_t  HALF_WORD_SIZE  = 16U;
const uint32_t  BYTE_SIZE       = 8U;

const uint32_t  PAGE_SIZE       = 4096U;
const uint32_t  PAGE_SIZE_BITS  = 12U;
const uint32_t  PAGE_BIT_MASK   = (( 1U << 12 ) - 1 );

const uint32_t  MAX_BANKS       = 16U;
const uint32_t  MAX_CPU_IDS     = 16U;

const uint32_t  MAX_MEMORY_SIZE  = 3072U * 1024U * 1024U;

const uint32_t  MAX_GREGS = 8;
const uint32_t  MAX_SREGS = 8;
const uint32_t  MAX_CREGS = 32;

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

// ??? to core ?
//------------------------------------------------------------------------------------------------------------
//
//
//
//------------------------------------------------------------------------------------------------------------
enum MemoryObjRegId : uint32_t {
    
    MC_REG_STATE            = 0,
    MC_REG_REQ_SEG          = 1,
    MC_REG_REQ_OFS          = 2,
    MC_REG_REQ_PRI          = 3,
    MC_REG_REQ_TAG          = 4,
    MC_REG_REQ_ADR          = 5,
    MC_REG_REQ_LEN          = 6,
    MC_REG_REQ_BLOCK_INDEX  = 7,
    MC_REG_REQ_BLOCK_SET    = 8,
    MC_REG_REQ_LATENCY      = 9,
    
    MC_REG_LATENCY          = 10,
    MC_REG_BLOCK_ENTRIES    = 11,
    MC_REG_BLOCK_SIZE       = 12,
    MC_REG_SETS             = 13
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

// ??? leave in core ?
const uint8_t MAX_TRAP_ID              = 32;
const uint8_t TRAP_CODE_BLOCK_SIZE     = 32;

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
enum InstrOpCode {
    
    // ??? sort to new scheme ...
    
    // ??? NOP gioes, becomes BRK
    
    OP_NOP          = 000,      // the no-operation instruction.
    
    OP_ADD          = 001,      // target = target + operand ;options for carry, ovl trap, etc.
    OP_SUB          = 002,      // target = target - operand ;options for carry, ovl trap, etc.
    OP_AND          = 003,      // target = target & operand ; option to negate the result
    OP_OR           = 004,      // target = target | operand ; option to negate the result
    OP_XOR          = 005,      // target = target ^ operand ; option to negate the result
    OP_CMP          = 006,      // subtract reg2 from reg1 and set condition codes
    OP_LDI          = 007,      // load immediate
    OP_LEA          = 010,      // load effective address offset
    OP_LSID         = 011,      // load segment ID register
    OP_EXTR         = 012,      // extract bit field of operand
    OP_DEP          = 013,      // extract bit field into operand
    OP_DSR          = 014,      // double register shift right
    OP_SHLA         = 015,      // shift left and add
    
    OP_LD          = 020,      // target = [ operand ]   // ??? covers LDW, LDH, LDB
    OP_ST           = 021,      // [ operand ] = target   // ??? covers STW, STH, STB
    
    OP_LDWR         = 022,      // load word referenced
    OP_STWC         = 023,      // store word conditional
    
    OP_LDWE         = 024,      // load word from virtual address
    OP_LDHE         = 024,      // load hald-word from virtual address
    OP_LDBE         = 024,      // load byte from virtual address
    
    OP_STWE          = 025,     // store word to virtual adress
    OP_STHE          = 025,     // store half-word to virtual adress
    OP_STBE          = 025,     // store byte to virtual adress
    
    OP_LDWA         = 026,      // load word from absolute address
    OP_LDHA         = 026,      // load hald-word from absolute address
    OP_LDBA         = 026,      // load byte from absolute address
    
    OP_STWA          = 027,     // store word to absolute adress
    OP_STHA          = 027,     // store half-word to absolute adress
    OP_STBA          = 027,     // store byte to absolute adress
    
    OP_B            = 040,      // branch
    OP_BL           = 041,      // branch and link
    OP_BR           = 042,      // branch register
    OP_BLR          = 043,      // branch and link register
    OP_BV           = 044,      // branch vectored
    OP_BVR          = 045,      // branch vectored register
    OP_BE           = 046,      // branch external
    OP_BLE          = 047,      // branch and link external
    
    OP_CBR          = 050,      // compare and branch
    OP_TBR          = 051,      // test and branch
    OP_CMR          = 052,      // conditional move register or value
   
    
    OP_MR           = 060,      // move to or from a segment or control register
    OP_MST          = 061,      // set or clear status bits
    OP_LDPA         = 062,      // load physical address
    OP_PRB          = 063,      // probe access
    OP_GATE         = 064,      // gateway instruction
    
    OP_ITLB         = 070,      // insert into TLB
    OP_PTLB         = 071,      // remove from TLB
    OP_PCA          = 072,      // purge and flush cache
    
    OP_RFI          = 075,      // return from interrupt
    OP_DIAG         = 076,      // diagnostics instruction, tbd.
    OP_BRK          = 077,      // break for debug
};

//------------------------------------------------------------------------------------------------------------
// During the instruction execution, there is a lot to check about the instructions defined. To speed up the
// process, each instruction and any special attribute to know about it is stored in a literal table. For
// each opCode there is a table entry which contains the opCode itself and flags that describe the
// instruction. These flags are used by the stages to identify characteristics of teh instruction instead of
// long IF or switch statements to test an instruction.
//
// ??? decide which way to go... no flags or all kinds of flags...
//------------------------------------------------------------------------------------------------------------
enum instrFlags : uint32_t {
    
    NO_FLAGS            = 0,
    COMP_INSTR          = 000000001,
    LOAD_INSTR          = 000000002,
    STORE_INSTR         = 000000004,
    BRANCH_INSTR        = 000000010,
    CTRL_INSTR          = 000000020,
    OP_MODE_INSTR       = 000000040,
    REG_R_INSTR         = 000000100,
    PRIV_INSTR          = 000000200
};

//------------------------------------------------------------------------------------------------------------
// The instruction decoder needs to do a lot of checking on the opcode. Naturally. The following flags help
// to simplifiy this checking. Each instruction is classified with the coresponding flags.
//
// ??? decide which way to go... no flags or all kinds of flags...
//------------------------------------------------------------------------------------------------------------
const struct opCodeInfo {
    
    char        mnemonic[ 8 ] ;
    uint8_t     opCode;
    uint32_t    flags;
    
} opCodeTab[ ] = {
    
    /* 00 */  { "NOP",    OP_NOP,   ( NO_FLAGS ) },
    /* 01 */  { "ADD",    OP_ADD,   ( COMP_INSTR | OP_MODE_INSTR | REG_R_INSTR ) },
    /* 02 */  { "SUB",    OP_SUB,   ( COMP_INSTR | OP_MODE_INSTR | REG_R_INSTR ) },
    /* 03 */  { "AND",    OP_AND,   ( COMP_INSTR | OP_MODE_INSTR | REG_R_INSTR ) },
    /* 04 */  { "OR",     OP_OR,    ( COMP_INSTR | OP_MODE_INSTR | REG_R_INSTR ) },
    /* 05 */  { "XOR",    OP_XOR,   ( COMP_INSTR | OP_MODE_INSTR | REG_R_INSTR ) },
    /* 06 */  { "CMP",    OP_CMP,   ( COMP_INSTR | OP_MODE_INSTR | REG_R_INSTR ) },
    /* 07 */  { "LDI",    OP_LDI,   ( COMP_INSTR | REG_R_INSTR ) },

    /* 10 */  { "LEA",    OP_LEA,   ( OP_MODE_INSTR | REG_R_INSTR ) },
    /* 11 */  { "LSID",   OP_LSID,  ( COMP_INSTR | REG_R_INSTR ) },
    /* 12 */  { "EXTR",   OP_EXTR,  ( COMP_INSTR | REG_R_INSTR ) },
    /* 13 */  { "DEP",    OP_DEP,   ( COMP_INSTR | REG_R_INSTR ) },
    /* 14 */  { "DSR",    OP_DSR,   ( COMP_INSTR | REG_R_INSTR ) },
    /* 15 */  { "SHLA",   OP_SHLA,  ( COMP_INSTR | REG_R_INSTR ) },
    /* 16 */  { "RSV016", OP_NOP,   ( NO_FLAGS ) },
    /* 17 */  { "RSV017", OP_NOP,   ( NO_FLAGS ) },
    
    /* 20 */  { "LD",         OP_LD,    ( LOAD_INSTR  | OP_MODE_INSTR | REG_R_INSTR ) },
    /* 21 */  { "ST",     OP_ST,    ( STORE_INSTR | OP_MODE_INSTR ) },
    /* 22 */  { "LR",     OP_LDWR,    ( LOAD_INSTR  | OP_MODE_INSTR | REG_R_INSTR ) },
    /* 23 */  { "SC",     OP_STWC,    ( STORE_INSTR | OP_MODE_INSTR ) },
    /* 24 */  { "LDWE",    OP_LDWE,   ( LOAD_INSTR  | REG_R_INSTR  ) },
    /* 25 */  { "STWE",    OP_STWE,   ( STORE_INSTR ) },
    /* 26 */  { "LDWA",    OP_LDWA,   ( LOAD_INSTR  | PRIV_INSTR | REG_R_INSTR ) },
    /* 27 */  { "STWA",   OP_STWA,   ( STORE_INSTR | PRIV_INSTR ) },
    
    /* 30 */  { "RSV030", OP_NOP,   ( NO_FLAGS ) },
    /* 31 */  { "RSV031", OP_NOP,   ( NO_FLAGS ) },
    /* 32 */  { "RSV032", OP_NOP,   ( NO_FLAGS ) },
    /* 33 */  { "RSV033", OP_NOP,   ( NO_FLAGS ) },
    /* 30 */  { "RSV034", OP_NOP,   ( NO_FLAGS ) },
    /* 31 */  { "RSV035", OP_NOP,   ( NO_FLAGS ) },
    /* 32 */  { "RSV036", OP_NOP,   ( NO_FLAGS ) },
    /* 33 */  { "RSV037", OP_NOP,   ( NO_FLAGS ) },
    
    /* 40 */  { "B",      OP_B,     ( BRANCH_INSTR ) },
    /* 41 */  { "BL",     OP_BL,    ( BRANCH_INSTR | REG_R_INSTR ) },
    /* 42 */  { "BR",     OP_BR,    ( BRANCH_INSTR ) },
    /* 43 */  { "BLR",    OP_BLR,   ( BRANCH_INSTR | REG_R_INSTR ) },
    /* 44 */  { "BV",     OP_BV,    ( BRANCH_INSTR ) },
    /* 45 */  { "BVR",    OP_BVR,   ( BRANCH_INSTR ) },
    /* 46 */  { "BE",     OP_BE,    ( BRANCH_INSTR ) },
    /* 47 */  { "BLE",    OP_BLE,   ( BRANCH_INSTR | REG_R_INSTR ) },
    
    /* 50 */  { "CBR",    OP_NOP,   ( BRANCH_INSTR ) },
    /* 51 */  { "TBR",    OP_NOP,   ( BRANCH_INSTR ) },
    /* 52 */  { "CMR",    OP_CMR,   ( COMP_INSTR | REG_R_INSTR ) },
    /* 53 */  { "RSV053", OP_NOP,   ( NO_FLAGS ) },
    /* 54 */  { "RSV054", OP_NOP,   ( NO_FLAGS ) },
    /* 55 */  { "RSV055", OP_NOP,   ( NO_FLAGS ) },
    /* 56 */  { "RSV056", OP_NOP,   ( NO_FLAGS ) },
    /* 57 */  { "RSV057", OP_NOP,   ( NO_FLAGS ) },
    
    /* 60 */  { "MR",     OP_MR,    ( CTRL_INSTR ) },
    /* 61 */  { "MST",    OP_MST,   ( CTRL_INSTR | PRIV_INSTR | REG_R_INSTR ) },
    /* 62 */  { "LDPA",   OP_LDPA,  ( CTRL_INSTR | PRIV_INSTR | LOAD_INSTR | REG_R_INSTR ) },
    /* 63 */  { "PRB",    OP_PRB,   ( CTRL_INSTR | REG_R_INSTR ) },
    /* 64 */  { "GATE",   OP_GATE,  ( CTRL_INSTR | BRANCH_INSTR | REG_R_INSTR ) },
    /* 65 */  { "RSV065", OP_NOP,   ( NO_FLAGS ) },
    /* 66 */  { "RSV066", OP_NOP,   ( NO_FLAGS ) },
    /* 67 */  { "RSV067", OP_NOP,   ( NO_FLAGS ) },
    
    /* 70 */  { "ITLB",   OP_ITLB,  ( CTRL_INSTR | PRIV_INSTR ) },
    /* 71 */  { "PTLB",   OP_PTLB,  ( CTRL_INSTR | PRIV_INSTR ) },
    /* 72 */  { "PCA",    OP_PCA,   ( CTRL_INSTR ) },
    /* 73 */  { "RSV073", OP_NOP,   ( NO_FLAGS ) },
    /* 74 */  { "RSV074", OP_NOP,   ( NO_FLAGS ) },
    /* 75 */  { "RFI",    OP_RFI,   ( CTRL_INSTR | PRIV_INSTR ) },
    /* 76 */  { "DIAG",   OP_NOP,   ( CTRL_INSTR ) },
    /* 77 */  { "BRK",    OP_BRK,   ( CTRL_INSTR ) }
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
#define EXTR( i, p, l ) ((( i  ) >> ( 31 - p )) & (( 1U << l )))

struct CPU24Instr {
    
public:
    
    static inline uint32_t  opCodeField( uint32_t instr )           { return( EXTR( instr, 5, 6 )); }
    static inline uint32_t  regRIdField( uint32_t instr )           { return( EXTR( instr, 8, 3 )); }
    static inline uint32_t  regAIdField( uint32_t instr )           { return( EXTR( instr, 28, 3 )); }
    static inline uint32_t  regBIdField( uint32_t instr )           { return( EXTR( instr, 31, 3 )); }
    static inline uint32_t  operandModeField( uint32_t instr )      { return( EXTR( instr, 17, 4 )); }
    static inline uint32_t  cmpCondField( uint32_t instr )          { return( EXTR( instr, 11, 3 )); }
    static inline uint32_t  cmrCondField( uint32_t instr )          { return( EXTR( instr, 11, 3 )); }
    static inline uint32_t  cmrImmField( uint32_t instr )           { return( EXTR( instr, 12, 1 )); }
    static inline uint32_t  cbrCondField( uint32_t instr )          { return( EXTR( instr, 8, 3 )); }
    static inline uint32_t  tbrCondField( uint32_t instr )          { return( EXTR( instr, 8, 3 )); }
    static inline uint32_t  immOfsSignField( uint32_t instr )       { return( EXTR( instr, 16, 1 )); }
    static inline uint32_t  arithOpFlagField( uint32_t instr )      { return( EXTR( instr, 11, 3 )); }
    static inline uint32_t  boolOpFlagField( uint32_t instr )       { return( EXTR( instr, 11, 3 )); }
    static inline uint32_t  extrDepLenField( uint32_t instr )       { return( EXTR( instr, 20, 5 )); }
    static inline uint32_t  extrDepPosField( uint32_t instr )       { return( EXTR( instr, 25, 5 )); }
    static inline uint32_t  dsrSaField( uint32_t instr )            { return( EXTR( instr, 20, 5 )); }
    static inline bool      shlaUseImmField( uint32_t instr )       { return( EXTR( instr, 17, 2 )); }
    static inline bool      useCarryField( uint32_t instr )         { return( EXTR( instr, 9, 1 )); }
    static inline bool      logicalOpField( uint32_t instr )        { return( EXTR( instr, 10, 1 )); }
    static inline bool      trapOvlField( uint32_t instr )          { return( EXTR( instr, 11, 1 )); }
    static inline bool      negateResField( uint32_t instr )        { return( EXTR( instr, 9, 1 )); }
    static inline bool      complRegBField( uint32_t instr )        { return( EXTR( instr, 10, 1 )); }
    static inline bool      extrSignedField( uint32_t instr )       { return( EXTR( instr, 9, 1 )); }
    static inline bool      depInZeroField( uint32_t instr )        { return( EXTR( instr, 9, 1 )); }
    static inline bool      depImmOptField( uint32_t instr )        { return( EXTR( instr, 10, 1 )); }
    static inline uint32_t  shlaSaField( uint32_t instr )           { return( EXTR( instr, 17, 2 )); }
    static inline bool      ldiZeroField( uint32_t instr )          { return( EXTR( instr, 9, 1 )); }
    static inline bool      ldiLeftField( uint32_t instr )          { return( EXTR( instr, 10, 1 )); }
    static inline bool      mrMovDirField( uint32_t instr )         { return( EXTR( instr, 9, 1 )); }
    static inline bool      mrRegTypeField( uint32_t instr )        { return( EXTR( instr, 10, 1 )); }
    static inline uint32_t  mrRegGrpField( uint32_t instr )         { return( EXTR( instr, 31, 5 )); }
    static inline uint32_t  mstModeField( uint32_t instr )          { return( EXTR( instr, 10, 2 )); }
    static inline uint32_t  mstArgField( uint32_t instr )           { return( EXTR( instr, 31, 6 )); }
    static inline bool      prbAdrModeField( uint32_t instr )       { return( EXTR( instr, 9, 1 )); }
    static inline bool      prbRwAccField( uint32_t instr )         { return( EXTR( instr, 10, 1 )); }
    static inline bool      ldpaAdrModeField( uint32_t instr )      { return( EXTR( instr, 9, 1 )); }
    static inline bool      tlbAdrModeField( uint32_t instr )       { return( EXTR( instr, 9, 1 )); }
    static inline bool      tlbKindField( uint32_t instr )          { return( EXTR( instr, 10, 1 )); }
    static inline bool      tlbArgModeField( uint32_t instr )       { return( EXTR( instr, 11, 1 )); }
    static inline bool      pcaAdrModeField( uint32_t instr )       { return( EXTR( instr, 9, 1 )); }
    static inline bool      pcaKindField( uint32_t instr )          { return( EXTR( instr, 10, 1 )); }
    static inline bool      pcaPurgeFlushField( uint32_t instr )    { return( EXTR( instr, 11, 1 )); }
    
    static inline uint32_t  segSelect( uint32_t arg )               { return ( EXTR( arg, 1, 2 )); }
    static inline uint32_t  ofsSelect( uint32_t arg )               { return ( EXTR( arg, 31, 30 )); }
    
    
    // ??? new immediates ...
    
    static inline uint32_t immGen0S16( uint32_t instr ) {
        
        uint32_t tmp = EXTR( instr, 31, 16 );
        return ( EXTR( instr, 16, 1) ? ( tmp | 0xFFFF0000 ) : ( tmp ));
    }
    
    static inline uint32_t immGen0S10( uint32_t instr ) {
        
        uint32_t tmp = EXTR( instr, 25, 10 );
        return ( EXTR( instr, 16, 1) ? ( tmp | 0xFFFFFC00 ) : ( tmp ));
    }
    
    static inline uint32_t immGen0S6( uint32_t instr ) {
        
        uint32_t tmp = EXTR( instr, 31, 6 );
        return ( EXTR( instr, 16, 1) ? ( tmp | 0xFFFFFFC0 ) : ( tmp ));
    }
    
    static inline uint32_t immGen7S16( uint32_t instr ) {
      
        uint32_t tmp = ( EXTR( instr, 17, 7 )) | ( EXTR( instr, 31, 16 ) << 7 );
        return ( EXTR( instr, 16, 1) ? ( tmp | 0xFFFFFFC0 ) : ( tmp ));
    }
    
    static inline uint32_t immGen7S13( uint32_t instr ) {
      
        uint32_t tmp = ( EXTR( instr, 17, 7 )) | ( EXTR( instr, 28, 13 ) << 7 );
        return ( EXTR( instr, 16, 1) ? ( tmp | 0xFFFFFFC0 ) : ( tmp ));
    }
    
    static inline uint32_t immGen7S10( uint32_t instr ) {
      
        uint32_t tmp = ( EXTR( instr, 17, 7 )) | ( EXTR( instr, 28, 10 ) << 7 );
        return ( EXTR( instr, 16, 1) ? ( tmp | 0xFFFFFFC0 ) : ( tmp ));
    }
    
    static inline uint32_t immGen10S10( uint32_t instr ) {
      
        uint32_t tmp = ( EXTR( instr, 17, 10 )) | ( EXTR( instr, 27, 10 ) << 10 );
        return ( EXTR( instr, 16, 1) ? ( tmp | 0xFFFFFFC0 ) : ( tmp ));
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
