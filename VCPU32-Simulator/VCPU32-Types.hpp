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
const uint32_t  MAX_PHYS_MEM_SIZE       = UINT32_MAX - ( UINT32_MAX / 16 );
const uint32_t  MAX_IO_MEM_SIZE         = UINT32_MAX / 16;

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
    PSTAGE_REG_ID_RID_X     = 11,
    
    PSTAGE_REG_ID_MA_CTRL   = 12,
    PSTAGE_REG_ID_EX_CTRL   = 13
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
// Operand mode codes. The operand in an instruction consists of an operand mode field and the mode
// depending arguments.
//
//------------------------------------------------------------------------------------------------------------
enum OpMode : uint32_t {
    
    OP_MODE_IMM             = 0x0,
    OP_MODE_ONE_REG         = 0x01,
    OP_MODE_TWO_REG         = 0x02,
    
    OP_MODE_REG_INDX_W      = 0x04,
    OP_MODE_REG_INDX_H      = 0x05,
    OP_MODE_REG_INDX_B      = 0x06,
    
    OP_MODE_INDX_W          = 0x08,
    OP_MODE_INDX_H          = 0x10,
    OP_MODE_INDX_B          = 0x18
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
    
    // group zero - register based, special
    
    OP_BRK          = 0x00,     // break for debug
    OP_LSID         = 0x01,     // load segement id
    OP_LDIL         = 0x02,     // load immediate left
    OP_ADDIL        = 0x03,     // add immediate left
    OP_EXTR         = 0x04,     // extract bit field of operand
    OP_DEP          = 0x05,     // extract bit field into operand
    OP_DSR          = 0x06,     // double register shift right
    OP_SHLA         = 0x07,     // shift left and add
    OP_CMR          = 0x0B,     // conditional move register or value
    OP_MR           = 0x09,     // move to or from a segment or control register
    OP_MST          = 0x0A,     // set or clear status bits
    
    OP_RSV_0B       = 0x0B,     // reserved
    OP_RSV_0C       = 0x0C,     // reserved
    OP_RSV_0D       = 0x0D,     // reserved
    OP_RSV_0E       = 0x0E,     // reserved
    OP_RSV_0F       = 0x0F,     // reserved
    
    // group one - opMode group
    
    OP_ADD          = 0x10,     // target = target + operand ;options for carry, ovl trap, etc.
    OP_SUB          = 0x11,     // target = target - operand ;options for carry, ovl trap, etc.
    OP_AND          = 0x12,     // target = target & operand ; option to negate the result
    OP_OR           = 0x13,     // target = target | operand ; option to negate the result
    OP_XOR          = 0x14,     // target = target ^ operand ; option to negate the result
    OP_CMP          = 0x15,     // subtract reg2 from reg1 and set target reg
    OP_LDO          = 0x16,     // load offset
    OP_RSV_17       = 0x17,     // reserved

    OP_LD           = 0x18,     // target = [ operand ]   // covers LDW, LDH, LDB
    OP_ST           = 0x19,     // [ operand ] = target   // covers STW, STH, STB
    OP_LDWR         = 0x1A,     // load word referenced
    OP_STWC         = 0X1B,     // store word conditional
    
    OP_RSV_1C       = 0x1C,     // reserved
    OP_RSV_1D       = 0x1D,     // reserved
    OP_LDPA         = 0x1E,     // load physical address
    OP_PRB          = 0x1F,     // probe access
    
    // group two - branch type group
    
    OP_B            = 0x20,     // branch
    OP_BL           = 0x21,     // branch and link
    OP_BR           = 0x22,     // branch register
    OP_BLR          = 0x23,     // branch and link register
    OP_BV           = 0x24,     // branch vectored
    OP_BVR          = 0x25,     // branch vectored register
    OP_BE           = 0x26,     // branch external
    OP_BLE          = 0x27,     // branch and link external
    OP_GATE         = 0x28,     // gateway instruction
    OP_CBR          = 0x29,     // compare and branch
    OP_TBR          = 0x2A,     // test and branch
    
    OP_RSV_2B       = 0x2B,     // reserved
    OP_RSV_2C       = 0x2C,     // reserved
    OP_RSV_2D       = 0x2D,     // reserved
    OP_RSV_2E       = 0x2E,     // reserved
    OP_RSV_2F       = 0x2F,     // reserved
    
    // group three - special and control group
    
    OP_RSV_30       = 0x30,     // reserved
    OP_RSV_31       = 0x31,     // reserved
    OP_RSV_32       = 0x32,     // reserved
    OP_RSV_33       = 0x33,     // reserved
    OP_RSV_34       = 0x34,     // reserved
    OP_RSV_35       = 0x35,     // reserved
    OP_RSV_36       = 0x36,     // reserved
    OP_RSV_37       = 0x37,     // reserved
    OP_ITLB         = 0x38,     // insert into TLB
    OP_PTLB         = 0x39,     // remove from TLB
    OP_PCA          = 0x3A,     // purge and flush cache
    OP_DIAG         = 0x3B,     // diagnostics instruction, tbd.
    OP_LDWA         = 0x3C,     // load word from absolute address
    OP_LDWAX        = 0x3D,     // load word indexed from absolute address
    OP_STWA         = 0x3E,     // store word to absolute adress
    OP_RFI          = 0x3F      // return from interrupt
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
// ??? should that be a local table for the dissasembler only ?
//------------------------------------------------------------------------------------------------------------

const struct opCodeInfo {
    
    char        mnemonic[ 8 ] ;
    uint8_t     opCode;
    uint32_t    flags;
    
} opCodeTab[ ] = {
    
    /* 0x00 */  { "BRK",    OP_BRK,     ( CTRL_INSTR ) },
    /* 0x01 */  { "LSID",   OP_LSID,    ( COMP_INSTR | REG_R_INSTR ) },
    /* 0x02 */  { "LDIL",   OP_LDIL,    ( COMP_INSTR | REG_R_INSTR ) },
    /* 0x03 */  { "ADDIL",  OP_ADDIL,   ( COMP_INSTR | REG_R_INSTR ) },
    /* 0x04 */  { "EXTR",   OP_EXTR,    ( COMP_INSTR | REG_R_INSTR ) },
    /* 0x05 */  { "DEP",    OP_DEP,     ( COMP_INSTR | REG_R_INSTR ) },
    /* 0x06 */  { "DSR",    OP_DSR,     ( COMP_INSTR | REG_R_INSTR ) },
    /* 0x07 */  { "SHLA",   OP_SHLA,    ( COMP_INSTR | REG_R_INSTR ) },
    /* 0x08 */  { "CMR",    OP_CMR,     ( COMP_INSTR | REG_R_INSTR ) },
    /* 0x09 */  { "MR",     OP_MR,      ( CTRL_INSTR ) },
    /* 0x0A */  { "MST",    OP_MST,     ( CTRL_INSTR | PRIV_INSTR | REG_R_INSTR ) },
    /* 0x0B */  { "RSV_0B", OP_RSV_0B,  ( NO_FLAGS ) },
    /* 0x0C */  { "RSV_0C", OP_RSV_0C,  ( NO_FLAGS ) },
    /* 0x0D */  { "RSV_0D", OP_RSV_0D,  ( NO_FLAGS ) },
    /* 0x0E */  { "RSV_0E", OP_RSV_0E,  ( NO_FLAGS ) },
    /* 0x0F */  { "RSV_0F", OP_RSV_0F,  ( NO_FLAGS ) },
    
    /* 0x10 */  { "ADD",    OP_ADD,     ( COMP_INSTR | OP_MODE_INSTR | REG_R_INSTR ) },
    /* 0x11 */  { "SUB",    OP_SUB,     ( COMP_INSTR | OP_MODE_INSTR | REG_R_INSTR ) },
    /* 0x12 */  { "AND",    OP_AND,     ( COMP_INSTR | OP_MODE_INSTR | REG_R_INSTR ) },
    /* 0x13 */  { "OR",     OP_OR,      ( COMP_INSTR | OP_MODE_INSTR | REG_R_INSTR ) },
    /* 0x14 */  { "XOR",    OP_XOR,     ( COMP_INSTR | OP_MODE_INSTR | REG_R_INSTR ) },
    /* 0x15 */  { "CMP",    OP_CMP,     ( COMP_INSTR | OP_MODE_INSTR | REG_R_INSTR ) },
    /* 0x16 */  { "LOD",    OP_LDO,     ( COMP_INSTR | OP_MODE_INSTR | REG_R_INSTR ) },
    /* 0x17 */  { "RSV_17", OP_RSV_17,  ( COMP_INSTR | REG_R_INSTR ) },
    /* 0x18 */  { "LD",     OP_LD,      ( LOAD_INSTR  | OP_MODE_INSTR | REG_R_INSTR ) },
    /* 0x19 */  { "ST",     OP_ST,      ( STORE_INSTR | OP_MODE_INSTR ) },
    /* 0x1A */  { "LDWR",   OP_LDWR,    ( LOAD_INSTR  | OP_MODE_INSTR | REG_R_INSTR ) },
    /* 0x1B */  { "STWC",   OP_STWC,    ( STORE_INSTR | OP_MODE_INSTR ) },
    /* 0x1C */  { "RSV_1C", OP_RSV_1C,  ( NO_FLAGS ) },
    /* 0x1D */  { "RSV_1D", OP_RSV_1D,  ( NO_FLAGS ) },
    /* 0x1E */  { "LDPA",   OP_LDPA,    ( LOAD_INSTR | PRIV_INSTR | REG_R_INSTR ) },
    /* 0x1F */  { "PRB",    OP_PRB,     ( CTRL_INSTR | REG_R_INSTR ) },
    
    /* 0x20 */  { "B",      OP_B,       ( BRANCH_INSTR ) },
    /* 0x21 */  { "BL",     OP_BL,      ( BRANCH_INSTR | REG_R_INSTR ) },
    /* 0x22 */  { "BR",     OP_BR,      ( BRANCH_INSTR ) },
    /* 0x23 */  { "BLR",    OP_BLR,     ( BRANCH_INSTR | REG_R_INSTR ) },
    /* 0x24 */  { "BV",     OP_BV,      ( BRANCH_INSTR ) },
    /* 0x25 */  { "BVR",    OP_BVR,     ( BRANCH_INSTR ) },
    /* 0x26 */  { "BE",     OP_BE,      ( BRANCH_INSTR ) },
    /* 0x27 */  { "BLE",    OP_BLE,     ( BRANCH_INSTR | REG_R_INSTR ) },
    /* 0x28 */  { "GATE",   OP_GATE,    ( CTRL_INSTR | BRANCH_INSTR | REG_R_INSTR ) },
    /* 0x29 */  { "CBR",    OP_CBR,     ( BRANCH_INSTR ) },
    /* 0x2A */  { "TBR",    OP_TBR,     ( BRANCH_INSTR ) },
    /* 0x2B */  { "RSV_2B", OP_RSV_2B,  ( NO_FLAGS ) },
    /* 0x2C */  { "RSV_2C", OP_RSV_2C,  ( NO_FLAGS ) },
    /* 0x2D */  { "RSV_2D", OP_RSV_2D,  ( NO_FLAGS ) },
    /* 0x2E */  { "RSV_2E", OP_RSV_2E,  ( NO_FLAGS ) },
    /* 0x2F */  { "RSV_2F", OP_RSV_2F,  ( NO_FLAGS ) },
    
    /* 0x30 */  { "RSV_30", OP_RSV_30,  ( NO_FLAGS ) },
    /* 0x31 */  { "RSV_31", OP_RSV_31,  ( NO_FLAGS ) },
    /* 0x32 */  { "RSV_32", OP_RSV_32,  ( NO_FLAGS ) },
    /* 0x33 */  { "RSV_33", OP_RSV_33,  ( NO_FLAGS ) },
    /* 0x34 */  { "RSV_34", OP_RSV_34,  ( NO_FLAGS ) },
    /* 0x35 */  { "RSV_35", OP_RSV_35,  ( NO_FLAGS ) },
    /* 0x36 */  { "RSV_36", OP_RSV_36,  ( NO_FLAGS ) },
    /* 0x37 */  { "RSV_37", OP_RSV_37,  ( NO_FLAGS ) },
    
    /* 0x38 */  { "ITLB",   OP_ITLB,    ( CTRL_INSTR | PRIV_INSTR ) },
    /* 0x39 */  { "PTLB",   OP_PTLB,    ( CTRL_INSTR | PRIV_INSTR ) },
    /* 0x3A */  { "PCA",    OP_PCA,     ( CTRL_INSTR ) },
    /* 0x3B */  { "DIAG",   OP_DIAG,    ( CTRL_INSTR ) },
    /* 0x3C */  { "LDWA",   OP_LDWA,    ( LOAD_INSTR  | PRIV_INSTR | REG_R_INSTR  ) },
    /* 0x3D */  { "LDWAX",  OP_LDWAX,   ( LOAD_INSTR  | PRIV_INSTR | REG_R_INSTR  ) },
    /* 0x3E */  { "STWA",   OP_STWA,    ( STORE_INSTR | PRIV_INSTR ) },
    /* 0x3F */  { "RFI",    OP_RFI,     ( CTRL_INSTR | PRIV_INSTR ) }
};

//------------------------------------------------------------------------------------------------------------
// The pipeline logic needs a knd of NOP instruction for stall and flush operations. We will pick an opCode
// that will do nothing.
//
//------------------------------------------------------------------------------------------------------------
const uint32_t NOP_INSTR = 0; // ??? settle on one ...

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
// The immediate values of an instruction often are constructed from several fields which are concatenated.
// The routines "xxSyy" assemble a field. The "S" indicates the sign position. This is always bit 18 in the
// instrcution word. The field left of the "S" contains as the most significant bit the sign bit. The fiels
// "xx" left of the field containing the sign is placed right off the "yy" field. Together the form the
// value. If the sign bit is set, the concatenatd field is sign extended.
//
//------------------------------------------------------------------------------------------------------------
struct Instr {
  
public:
    
    static inline bool getBit( uint32_t arg, int pos ) {
        
        return( arg & ( 1U << ( 31 - ( pos % 32 ))));
    }
    
    static inline void setBit( uint32_t *arg, int pos ) {
       
        *arg |= ( 1U << ( 31 - ( pos % 32 )));
    }
    
    static inline void clearBit( uint32_t *arg, int pos ) {
        
        *arg &= ~( 1U << ( 31 - ( pos % 32 )));
    }
    
    static inline uint32_t getBitField( uint32_t arg, int pos, int len, bool sign = false ) {
        
        pos = pos % 32;
        len = len % 32;
        
        uint32_t tmpM = ( 1U << len ) - 1;
        uint32_t tmpA = arg >> ( 31 - pos );
        
        if ( sign ) return( tmpA | ( ~ tmpM ));
        else        return( tmpA & tmpM );
    }
    
    static inline void setBitField( uint32_t *arg, int pos, int len, uint32_t val ) {
        
        pos = pos % 32;
        len = len % 32;
        
        uint32_t tmpM = ( 1U << len ) - 1;
        
        val = ( val & tmpM ) << ( 31 - pos );
        
        *arg = ( *arg & ( ~tmpM )) | val;
    }
    
    static inline uint32_t signExtend( uint32_t arg, int len ) {
        
        len = len % 32;
        
        uint32_t tmpM = ( 1U << len ) - 1;
        bool     sign = arg & ( 1U << ( 31 - len ));
        
        if ( sign ) return( arg |= ~ tmpM );
        else        return( arg &= tmpM );
    }
    
    static inline uint32_t lowSignExtend32( uint32_t arg, int len ) {
        
        len = len % 32;
        
        uint32_t tmpM = ( 1U << ( len - 1 )) - 1;
        bool     sign = arg % 2;
        
        arg = arg >> 1;
        
        if ( sign ) return( arg |= ~ tmpM );
        else        return( arg &= tmpM );
    }
   
    static inline uint32_t  opCodeField( uint32_t instr )           { return( getBitField( instr, 5,  6  )); }
    static inline uint32_t  regRIdField( uint32_t instr )           { return( getBitField( instr, 9,  4  )); }
    static inline uint32_t  regAIdField( uint32_t instr )           { return( getBitField( instr, 27, 4  )); }
    static inline uint32_t  regBIdField( uint32_t instr )           { return( getBitField( instr, 31, 4  )); }
    static inline uint32_t  opModeField( uint32_t instr )           { return( getBitField( instr, 17, 5  )); }
    static inline uint32_t  opSegSelectField( uint32_t instr )      { return( getBitField( instr, 19, 2  )); }
    static inline uint32_t  cmpCondField( uint32_t instr )          { return( getBitField( instr, 12, 3  )); }
    static inline uint32_t  cmrCondField( uint32_t instr )          { return( getBitField( instr, 12, 3  )); }
    static inline uint32_t  cbrCondField( uint32_t instr )          { return( getBitField( instr, 8,  3  )); }
    static inline uint32_t  tbrCondField( uint32_t instr )          { return( getBitField( instr, 8,  3  )); }
    static inline uint32_t  arithOpFlagField( uint32_t instr )      { return( getBitField( instr, 12, 3  )); }
    static inline uint32_t  boolOpFlagField( uint32_t instr )       { return( getBitField( instr, 12, 3  )); }
    static inline uint32_t  extrDepLenField( uint32_t instr )       { return( getBitField( instr, 22, 5  )); }
    static inline uint32_t  extrDepPosField( uint32_t instr )       { return( getBitField( instr, 27, 5  )); }
    static inline uint32_t  dsrSaAmtField( uint32_t instr )         { return( getBitField( instr, 22, 5  )); }
    static inline bool      shlaUseImmField( uint32_t instr )       { return( getBitField( instr, 22, 2  )); }
    static inline uint32_t  shlaSaField( uint32_t instr )           { return( getBitField( instr, 22, 2  )); }
    static inline uint32_t  mrArgField( uint32_t instr )            { return( getBitField( instr, 31, 6  )); }
    static inline uint32_t  mstModeField( uint32_t instr )          { return( getBitField( instr, 11, 2  )); }
    static inline uint32_t  mstArgField( uint32_t instr )           { return( getBitField( instr, 31, 6  )); }
    static inline bool      brkInfo1Field( uint32_t instr )         { return( getBitField( instr, 9,  4  )); }
    static inline bool      brkInfo2Field( uint32_t instr )         { return( getBitField( instr, 31, 16 )); }
    
    static inline uint32_t  immOfsSignField( uint32_t instr )       { return( getBit( instr, 18 )); }  // phase out...
    static inline uint32_t  extrSignField( uint32_t instr )         { return( getBit( instr, 10 )); }
    static inline uint32_t  depZeroField( uint32_t instr )          { return( getBit( instr, 10 )); }
    static inline uint32_t  depImmField( uint32_t instr )           { return( getBit( instr, 12 )); }
    static inline uint32_t  extrDepSaOptField( uint32_t instr )     { return( getBit( instr, 11 )); }
    static inline uint32_t  dsrSaOptField( uint32_t instr )         { return( getBit( instr, 11 )); }
    static inline bool      useCarryField( uint32_t instr )         { return( getBit( instr, 10 )); }
    static inline bool      logicalOpField( uint32_t instr )        { return( getBit( instr, 11 )); }
    static inline bool      trapOvlField( uint32_t instr )          { return( getBit( instr, 12 )); }
    static inline bool      negateResField( uint32_t instr )        { return( getBit( instr, 10 )); }
    static inline bool      complRegBField( uint32_t instr )        { return( getBit( instr, 11 )); }
    static inline bool      extrSignedField( uint32_t instr )       { return( getBit( instr, 10 )); }
    static inline bool      depInZeroField( uint32_t instr )        { return( getBit( instr, 10 )); }
    static inline bool      depImmOptField( uint32_t instr )        { return( getBit( instr, 11 )); }
    static inline bool      mrZeroField( uint32_t instr )           { return( getBit( instr, 10 )); }
    static inline bool      mrMovDirField( uint32_t instr )         { return( getBit( instr, 11 )); }
    static inline bool      mrRegTypeField( uint32_t instr )        { return( getBit( instr, 12 )); }
    static inline bool      prbAdrModeField( uint32_t instr )       { return( getBit( instr, 10 )); }
    static inline bool      prbRwAccField( uint32_t instr )         { return( getBit( instr, 11 )); }
    static inline bool      ldpaAdrModeField( uint32_t instr )      { return( getBit( instr, 10 )); }
    static inline bool      tlbAdrModeField( uint32_t instr )       { return( getBit( instr, 10 )); }
    static inline bool      tlbKindField( uint32_t instr )          { return( getBit( instr, 11 )); }
    static inline bool      tlbArgModeField( uint32_t instr )       { return( getBit( instr, 12 )); }
    static inline bool      pcaAdrModeField( uint32_t instr )       { return( getBit( instr, 10 )); }
    static inline bool      pcaKindField( uint32_t instr )          { return( getBit( instr, 11 )); }
    static inline bool      pcaPurgeFlushField( uint32_t instr )    { return( getBit( instr, 12 )); }
    
    static inline uint32_t  segSelect( uint32_t arg )               { return( getBitField( arg, 1,  2  )); }
    static inline uint32_t  ofsSelect( uint32_t arg )               { return( getBitField( arg, 31, 30 )); }
    
    static inline uint32_t immGenPosLenLowSign( uint32_t instr, int pos, int len ) {
        
        pos = pos % 32;
        len = len % 32;
        
        return( lowSignExtend32( Instr::getBitField( instr, pos, len ), len ));
    }
    
    static inline uint32_t immLeftField( uint32_t instr ) {
        
        return( getBitField( instr, 31, 22 ));
    }
    
    static inline uint32_t immRightField( uint32_t instr ) {
        
        return( getBitField( instr, 31, 10 ));
    }
    
    static inline uint32_t add32( uint32_t arg1, uint32_t arg2 ) {
        
        return ( arg1 + arg2 );
    }
    
    static uint32_t mapOpModeToIndexReg( uint32_t opMode ) {
        
        if      (( opMode >= 8 ) && ( opMode <= 15 )) return( opMode );
        else if (( opMode >= 16 ) && ( opMode <= 23 )) return( opMode - 8 );
        else if (( opMode >= 24 ) && ( opMode <= 31 )) return( opMode - 16 );
        else return( 0 );
    }
    
    static uint32_t dataLenForOpMode( uint32_t opMode ) {
        
        if      (( opMode >= 8 ) && ( opMode <= 15 )) return( 4 );
        else if (( opMode >= 16 ) && ( opMode <= 23 )) return( 2 );
        else if (( opMode >= 24 ) && ( opMode <= 31 )) return( 1 );
        else return( 0 );
    }
    
};
 
#endif
