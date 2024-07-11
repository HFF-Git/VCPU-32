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
// Basic constants for TLB, caches and memory. The intended hardware will perform a lookup of TLB and caches
// in paralell. As a consequence the number of bits needed to respresent the block entries cannot be greater
// than the number of bits necessary to represent the page size minus the number of bits it takes to represent
// the block size. For example, of the block size is four words, it will take two bits to index into the
// block. If the page bit size is 12 bits then we have 10 bits left for indexing the cache, i.e. 1024 entries.
//
//------------------------------------------------------------------------------------------------------------
const uint32_t  WORD_SIZE               = 32U;
const uint32_t  HALF_WORD_SIZE          = 16U;
const uint32_t  BYTE_SIZE               = 8U;

const uint32_t  MAX_GREGS               = 16;
const uint32_t  MAX_SREGS               = 8;
const uint32_t  MAX_CREGS               = 32;

const uint32_t  PAGE_SIZE               = 16384U;
const uint32_t  PAGE_SIZE_BITS          = 14U;
const uint32_t  PAGE_BIT_MASK           = (( 1U << 14 ) - 1 );

const uint32_t  MAX_MEMORY_SIZE         = UINT32_MAX;
const uint32_t  MAX_IO_MEM_SIZE         = UINT32_MAX / 16;
const uint32_t  MAX_PHYS_MEM_SIZE       = UINT32_MAX - MAX_IO_MEM_SIZE;
const uint32_t  MAX_PDC_MEM_SIZE        = MAX_IO_MEM_SIZE / 16;

const uint32_t  MAX_CACHE_BLOCK_ENTRIES = 1024;
const uint16_t  MAX_BLOCK_SIZE          = 128;
const uint16_t  MAX_BLOCK_SETS          = 4;

const uint8_t   MAX_TRAP_ID             = 32;
const uint8_t   TRAP_CODE_BLOCK_SIZE    = 32;


//------------------------------------------------------------------------------------------------------------
// Processor state fields. There are two machine words containg ingvarious bits and fields for the current
// execution state.
//
//  0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
// :--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:--:
// :M :X :C :0 :CB: reserved           :0 :D :P :E : IA segment Id                                 :  PSW-0
// :-----------------------------------------------------------------------------------------------:
// : IA offset                                                                               : 0   :  PSW-1
// :-----------------------------------------------------------------------------------------------:
//
// ??? note: under construction .... always cross check with the document.
//------------------------------------------------------------------------------------------------------------
enum StatusRegBits : uint32_t {
    
    ST_MACHINE_CHECK                = 0,
    ST_EXECUTION_LEVEL              = 1,
    ST_CODE_TRANSLATION_ENABLE      = 2,
    ST_CARRY                        = 4,
    
    ST_DATA_TRANSLATION_ENABLE      = 13,
    ST_PROTECT_ID_CHECK_ENABLE      = 14,
    ST_INTERRUPT_ENABLE             = 15
};

//------------------------------------------------------------------------------------------------------------
// Program State register identifiers.
//
//------------------------------------------------------------------------------------------------------------
enum ProgStateRegisterId : uint32_t {
    
    PS_REG_PSW_0        = 0,
    PS_REG_PSW_1        = 1,
};

//------------------------------------------------------------------------------------------------------------
// Control register identifiers.
//
//------------------------------------------------------------------------------------------------------------
enum ControlRegisterId : uint32_t {
    
    CR_SYSTEM_SWITCH    = 0x0,
    CR_RECOVERY_CNTR    = 0x1,
    CR_SHIFT_AMOUNT     = 0x2,
    CR_RSV_3            = 0x3,
    CR_SEG_ID_0_1       = 0x4,
    CR_SEG_ID_2_3       = 0x5,
    CR_SEG_ID_4_5       = 0x6,
    CR_SEG_ID_6_7       = 0x7,
    
    CR_RSV_8            = 0x8,
    CR_RSV_9            = 0x9,
    CR_RSV_10           = 0xA,
    CR_RSV_11           = 0xB,
    CR_RSV_12           = 0xC,
    CR_RSV_13           = 0xD,
    CR_RSV_14           = 0xE,
    CR_RSV_15           = 0xF,
    
    CR_TRAP_VECTOR_ADR  = 0x10,
    CR_TRAP_PSW_0       = 0x11,
    CR_TRAP_PSW_1       = 0x12,
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

//------------------------------------------------------------------------------------------------------------
// Traps. There are three classes of traps. The first type is the general TRAP. In general, it is a situation
// when an instruction detected an issue and cannot run until some intervention took place. Also, there are
// traps that should then run after the instruction executed. INTERRUPTS are events taken after the execution
// of an instruction completed and an external interrupt is pending. CHECK traps are bad news and the CPU
// cannot continue at all. In an emulator, they most likely do not occur, but in a simulator we could simulate
// a hardware situation.
//
// ??? cross check wth document...
//------------------------------------------------------------------------------------------------------------
enum TrapId : uint32_t {
    
    NO_TRAP                 = 0,
    MACHINE_CHECK           = 1,
    PHYS_ADDRESS_CHECK      = 2,
    EXT_INTERRUPT           = 3,
    ILLEGAL_INSTR_TRAP      = 4,
    PRIV_OPERATION_TRAP     = 5,
    OVERFLOW_TRAP           = 6,
    DATA_ALIGNMENT_TRAP     = 19,
    
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
// ??? changed ...
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
// Operand mode codes. The operand in an instruction consists of an operand mode field and the mode
// depending arguments.
//
//------------------------------------------------------------------------------------------------------------
enum OpMode : uint32_t {
    
    OP_MODE_IMM             = 0x0,
    OP_MODE_REG             = 0x1,
    OP_MODE_REG_INDX        = 0x2,
    OP_MODE_INDX            = 0x3
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
    
    OP_BRK          = 0x00,     // break for debug
    OP_LDIL         = 0x01,     // load immediate left
    OP_ADDIL        = 0x02,     // add immediate left
    OP_LDO          = 0x03,     // load offset
    OP_LSID         = 0x04,     // load segement id
    OP_EXTR         = 0x05,     // extract bit field of operand
    OP_DEP          = 0x06,     // extract bit field into operand
    OP_DSR          = 0x07,     // double register shift right
    OP_SHLA         = 0x08,     // shift left and add
    OP_CMR          = 0x09,     // conditional move register or value
    OP_MR           = 0x0A,     // move to or from a segment or control register
    OP_MST          = 0x0B,     // set or clear status bits
    
    OP_ADD          = 0x10,     // target = target + operand ;options for carry, ovl trap, etc.
    OP_ADC          = 0x11,     // target = target + operand ;options for carry, ovl trap, etc.
    OP_SUB          = 0x12,     // target = target - operand ;options for carry, ovl trap, etc.
    OP_SBC          = 0x13,     // target = target - operand ;options for carry, ovl trap, etc.
    OP_AND          = 0x14,     // target = target & operand ; option to negate the result
    OP_OR           = 0x15,     // target = target | operand ; option to negate the result
    OP_XOR          = 0x16,     // target = target ^ operand ; option to negate the result
    OP_CMP          = 0x17,     // subtract reg2 from reg1 and set target reg
    OP_CMPU         = 0x18,     // subtract reg2 from reg1 and set target reg
    
    OP_B            = 0x20,     // branch
    OP_GATE         = 0x21,     // gateway instruction
    OP_BR           = 0x22,     // branch register
    OP_BV           = 0x23,     // branch vectored
    OP_BE           = 0x24,     // branch external
    OP_BVE          = 0x25,     // branch and link external
    OP_CBR          = 0x26,     // compare and branch
    OP_CBRU         = 0x27,     // test and branch
    
    OP_LD           = 0x30,     // target = [ operand ]   // covers LDW, LDH, LDB
    OP_ST           = 0x31,     // [ operand ] = target   // covers STW, STH, STB
    OP_LDA          = 0x32,     // load word from absolute address
    OP_STA          = 0x33,     // store word to absolute adress
    OP_LDR          = 0x34,     // load word referenced
    OP_STC          = 0X35,     // store word conditional
    
    OP_LDPA         = 0x39,     // load physical address
    OP_PRB          = 0x3A,     // probe access
    OP_ITLB         = 0x3B,     // insert into TLB
    OP_PTLB         = 0x3C,     // remove from TLB
    OP_PCA          = 0x3D,     // purge and flush cache
    OP_DIAG         = 0x3E,     // diagnostics instruction, tbd.
    OP_RFI          = 0x3F      // return from interrupt
};

//------------------------------------------------------------------------------------------------------------
// During the instruction execution, there is a lot to check about the instructions defined. To speed up the
// process, each instruction and any special attribute to know about it is stored in a literal table. For
// each opCode there is a table entry which contains the opCode itself and flags that describe the
// instruction. These flags are used by the pipeline stages to identify characteristics of the instruction
// instead of long "if" or "switch" statements to test an instruction.
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
    PRIV_INSTR          = ( 1U << 7 ),
    READ_INSTR          = ( 1U << 8 ),
    WRITE_INSTR         = ( 1U << 9 )
   
};

//------------------------------------------------------------------------------------------------------------
// The instruction decoder needs to do a lot of checking on the opcode. Naturally. The following flags help
// to simplifiy this checking. Each instruction is classified with the relevant flags.
//
//------------------------------------------------------------------------------------------------------------
const struct opCodeInfo {
    
    char        mnemonic[ 8 ] ;
    uint8_t     opCode;
    uint32_t    flags;
    
} opCodeTab[ ] = {
    
    /* 0x00 */  { "BRK",    OP_BRK,     ( CTRL_INSTR ) },
    /* 0x01 */  { "LDIL",   OP_LDIL,    ( COMP_INSTR | REG_R_INSTR ) },
    /* 0x02 */  { "ADDIL",  OP_ADDIL,   ( COMP_INSTR | REG_R_INSTR ) },
    /* 0x03 */  { "LOD",    OP_LDO,     ( COMP_INSTR | REG_R_INSTR ) },
    /* 0x04 */  { "LSID",   OP_LSID,    ( COMP_INSTR | REG_R_INSTR ) },
    /* 0x05 */  { "EXTR",   OP_EXTR,    ( COMP_INSTR | REG_R_INSTR ) },
    /* 0x06 */  { "DEP",    OP_DEP,     ( COMP_INSTR | REG_R_INSTR ) },
    /* 0x07 */  { "DSR",    OP_DSR,     ( COMP_INSTR | REG_R_INSTR ) },
    /* 0x08 */  { "SHLA",   OP_SHLA,    ( COMP_INSTR | REG_R_INSTR ) },
    /* 0x09 */  { "CMR",    OP_CMR,     ( COMP_INSTR | REG_R_INSTR ) },
    /* 0x0A */  { "MR",     OP_MR,      ( CTRL_INSTR ) },
    /* 0x0B */  { "MST",    OP_MST,     ( CTRL_INSTR | PRIV_INSTR | REG_R_INSTR ) },
    /* 0x0C */  { "RSV_0C", 0x0C,       ( NO_FLAGS ) },
    /* 0x0D */  { "RSV_0D", 0x0D,       ( NO_FLAGS ) },
    /* 0x0E */  { "RSV_0E", 0x0E,       ( NO_FLAGS ) },
    /* 0x0F */  { "RSV_0F", 0x0F,       ( NO_FLAGS ) },
    
    /* 0x10 */  { "ADD",    OP_ADD,     ( COMP_INSTR | OP_MODE_INSTR | READ_INSTR | REG_R_INSTR ) },
    /* 0x11 */  { "ADC",    OP_ADC,     ( COMP_INSTR | OP_MODE_INSTR | READ_INSTR | REG_R_INSTR ) },
    /* 0x12 */  { "SUB",    OP_SUB,     ( COMP_INSTR | OP_MODE_INSTR | READ_INSTR | REG_R_INSTR ) },
    /* 0x13 */  { "SBC",    OP_SBC,     ( COMP_INSTR | OP_MODE_INSTR | READ_INSTR | REG_R_INSTR ) },
    /* 0x14 */  { "AND",    OP_AND,     ( COMP_INSTR | OP_MODE_INSTR | READ_INSTR | REG_R_INSTR ) },
    /* 0x15 */  { "OR",     OP_OR,      ( COMP_INSTR | OP_MODE_INSTR | READ_INSTR | REG_R_INSTR ) },
    /* 0x16 */  { "XOR",    OP_XOR,     ( COMP_INSTR | OP_MODE_INSTR | READ_INSTR | REG_R_INSTR ) },
    /* 0x17 */  { "CMP",    OP_CMP,     ( COMP_INSTR | OP_MODE_INSTR | READ_INSTR | REG_R_INSTR ) },
    /* 0x18 */  { "CMPU",   OP_CMPU,    ( COMP_INSTR | OP_MODE_INSTR | READ_INSTR | REG_R_INSTR ) },
    /* 0x19 */  { "RSV_19", 0x19,       ( NO_FLAGS ) },
    /* 0x1A */  { "RSV_1A", 0x1A,       ( NO_FLAGS ) },
    /* 0x1B */  { "RSV_1B", 0x1B,       ( NO_FLAGS ) },
    /* 0x1C */  { "RSV_1C", 0x1C,       ( NO_FLAGS ) },
    /* 0x1D */  { "RSV_1D", 0x1D,       ( NO_FLAGS ) },
    /* 0x1E */  { "RSV_1E", 0x1E,       ( NO_FLAGS ) },
    /* 0x1F */  { "RSV_1F", 0x1F,       ( NO_FLAGS ) },
    
    /* 0x20 */  { "B",      OP_B,       ( BRANCH_INSTR | REG_R_INSTR ) },
    /* 0x21 */  { "GATE",   OP_GATE,    ( CTRL_INSTR | BRANCH_INSTR | REG_R_INSTR ) },
    /* 0x22 */  { "BR",     OP_BR,      ( BRANCH_INSTR | REG_R_INSTR ) },
    /* 0x23 */  { "BV",     OP_BV,      ( BRANCH_INSTR | REG_R_INSTR ) },
    /* 0x24 */  { "BE",     OP_BE,      ( BRANCH_INSTR | REG_R_INSTR ) },
    /* 0x25 */  { "BVE",    OP_BVE,     ( BRANCH_INSTR | REG_R_INSTR ) },
    /* 0x26 */  { "CBR",    OP_CBR,     ( BRANCH_INSTR ) },
    /* 0x27 */  { "CBRU",   OP_CBR,     ( BRANCH_INSTR ) },
    /* 0x28 */  { "RSV_28", 0x28,       ( NO_FLAGS ) },
    /* 0x29 */  { "RSV_29", 0x29,       ( NO_FLAGS ) },
    /* 0x2A */  { "RSV_2A", 0x0A,       ( NO_FLAGS ) },
    /* 0x2B */  { "RSV_2B", 0x2B,       ( NO_FLAGS ) },
    /* 0x2C */  { "RSV_2C", 0x2C,       ( NO_FLAGS ) },
    /* 0x2D */  { "RSV_2D", 0x2D,       ( NO_FLAGS ) },
    /* 0x2E */  { "RSV_2E", 0x2E,       ( NO_FLAGS ) },
    /* 0x2F */  { "RSV_2F", 0x2F,       ( NO_FLAGS ) },
    
    /* 0x30 */  { "LD",     OP_LD,      ( LOAD_INSTR  | READ_INSTR | REG_R_INSTR ) },
    /* 0x31 */  { "ST",     OP_ST,      ( STORE_INSTR | WRITE_INSTR ) },
    /* 0x32 */  { "LDA",    OP_LDA,     ( LOAD_INSTR  | PRIV_INSTR | READ_INSTR | REG_R_INSTR  ) },
    /* 0x33 */  { "STA",    OP_STA,     ( STORE_INSTR | PRIV_INSTR | WRITE_INSTR ) },
    /* 0x34 */  { "LDR",    OP_LDR,     ( LOAD_INSTR  | READ_INSTR | REG_R_INSTR ) },
    /* 0x35 */  { "STC",    OP_STC,     ( STORE_INSTR | WRITE_INSTR) },
    /* 0x36 */  { "RSV_36", 0x36,       ( NO_FLAGS ) },
    /* 0x37 */  { "RSV_37", 0x37,       ( NO_FLAGS ) },
    /* 0x38 */  { "RSV_38", 0x38,       ( NO_FLAGS ) },
    /* 0x39 */  { "LDPA",   OP_LDPA,    ( LOAD_INSTR | PRIV_INSTR | REG_R_INSTR ) },
    /* 0x3A */  { "PRB",    OP_PRB,     ( CTRL_INSTR | REG_R_INSTR ) },
    /* 0x3B */  { "ITLB",   OP_ITLB,    ( CTRL_INSTR | PRIV_INSTR ) },
    /* 0x3C */  { "PTLB",   OP_PTLB,    ( CTRL_INSTR | PRIV_INSTR ) },
    /* 0x3C */  { "PCA",    OP_PCA,     ( CTRL_INSTR ) },
    /* 0x3D */  { "DIAG",   OP_DIAG,    ( CTRL_INSTR ) },
    /* 0x3F */  { "RFI",    OP_RFI,     ( CTRL_INSTR | PRIV_INSTR ) }
};

//------------------------------------------------------------------------------------------------------------
// The pipeline logic needs a kind of NOP instruction for stall and flush operations. We will pick an opCode
// that will do nothing. Currently, the BRK 0, 0 instruction is used for this purpose. TDB ...
//
//------------------------------------------------------------------------------------------------------------
const uint32_t NOP_INSTR = 0;
 
#endif
