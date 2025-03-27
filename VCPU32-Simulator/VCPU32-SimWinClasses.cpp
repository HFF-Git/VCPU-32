//------------------------------------------------------------------------------------------------------------
//
// VCPU32 - A 32-bit CPU - Simulator window classes
//
//------------------------------------------------------------------------------------------------------------
// This module contains all of the mothods for the different windows that the simlulator supports. The
// exception is the command window, which is in a separate file. A window generally consist of a banner line,
// shown in inverse video and a nuber of body lines.
//
//------------------------------------------------------------------------------------------------------------
//
// VCPU32 - A 32-bit CPU - Simulator window classes
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
#include "VCPU32-Types.h"
#include "VCPU32-Core.h"
#include "VCPU32-SimDeclarations.h"
#include "VCPU32-SimTables.h"

//------------------------------------------------------------------------------------------------------------
//
//  Windows:
//
//  Program Regs    -> PS
//  General Regs    -> GR
//  Special Regs    -> CR
//  Pipeline Regs   -> PL
//  Statistics      -> ST
//  Program Code    -> PC
//  TLB             -> IT, DT
//  T-Controller    -> ITR, DTR
//  Cache           -> IC, DC, UC
//  C-Controller    -> ICR, DCR, UCR
//  Text Window     -> TX
//  User Defined    -> UW
//
//  Combine the window command with the window to form the command to type.
//  Example: PSE -> enable general regs window.
//  Note: not all combinations are allowed.
//
//------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------
// Local name space. We try to keep utility functions local to the file.
//
//------------------------------------------------------------------------------------------------------------
namespace {

//-----------------------------------------------------------------------------------------------------------
// Little bit field helpers.
//
//-----------------------------------------------------------------------------------------------------------
bool getBit( uint32_t arg, int pos ) {
    
    return(( arg & ( 1U << ( 31 - ( pos % 32 )))) ? 1 : 0 );
}

uint32_t getBitField( uint32_t arg, int pos, int len, bool sign = false ) {
    
    pos = pos % 32;
    len = len % 32;
    
    uint32_t tmpM = ( 1U << len ) - 1;
    uint32_t tmpA = arg >> ( 31 - pos );
    
    if ( sign ) return( tmpA | ( ~ tmpM ));
    else        return( tmpA & tmpM );
}

//------------------------------------------------------------------------------------------------------------
// Routine for creating the access rights string. It consists of the page access and the two privilege
// levels.
//
//------------------------------------------------------------------------------------------------------------
void buildAccessRightsStr( char *bufStr, int bufLen, uint8_t type, uint8_t privLev1, uint8_t privLev2 ) {
    
    switch( type ) {
            
        case ACC_READ_ONLY:     snprintf( bufStr, bufLen, "[ro:%1d:%1d]", privLev1, privLev2 ); break;
        case ACC_READ_WRITE:    snprintf( bufStr, bufLen, "[rw:%1d:%1d]", privLev1, privLev2 ); break;
        case ACC_EXECUTE:       snprintf( bufStr, bufLen, "[ex:%1d:%1d]", privLev1, privLev2 ); break;
        default:                snprintf( bufStr, bufLen, "[xx:%1d:%1d]", privLev1, privLev2 ); break;
    }
}

}; // namespace


//***********************************************************************************************************
//***********************************************************************************************************
//
// Methods for the Program State Window class.
//
//***********************************************************************************************************
//***********************************************************************************************************


//------------------------------------------------------------------------------------------------------------
// Object creator.
//
//------------------------------------------------------------------------------------------------------------
SimWinProgState::SimWinProgState( VCPU32Globals *glb ) : SimWin( glb ) { }

//------------------------------------------------------------------------------------------------------------
// The default values are the initial settings when windows is brought up the first time, or for the WDEF
// command.
//
//------------------------------------------------------------------------------------------------------------
void SimWinProgState::setDefaults( ) {
    
    setRadix( glb -> env -> getEnvVarInt((char *) ENV_RDX_DEFAULT ));
    setDefColumns( 12 + ( 8 * 11 ), 16 );
    setDefColumns( 12 + ( 8 * 13 ), 8  );
    setDefColumns( 12 + ( 8 * 11 ), 10 );
    setColumns( getDefColumns( getRadix( )));
    setRows( 4 );

    setWinType( WT_PS_WIN );
    setEnable( true );
}

//------------------------------------------------------------------------------------------------------------
// The window overrides the setRadix method to set the column width according to the radix chosen.
//
//------------------------------------------------------------------------------------------------------------
void SimWinProgState::setRadix( int rdx ) {
    
    SimWin::setRadix( rdx );
    setColumns( getDefColumns( getRadix( )));
}

//------------------------------------------------------------------------------------------------------------
// Each window consist of a headline and a body. The banner line is always shown in inverse and contains
// summary or head data for the window. The program state banner lists the instruction address and the
// status word.
//
//------------------------------------------------------------------------------------------------------------
void SimWinProgState::drawBanner( ) {
    
    uint32_t fmtDesc = FMT_BOLD | FMT_INVERSE | FMT_ALIGN_LFT ;
    
    setWinCursor( 1, 1 );
    printTextField((char *) "Program State", ( fmtDesc ), 16 );
    
    printTextField((char *) "Seg:", fmtDesc, 5 );
    printNumericField( glb -> cpu -> getReg( RC_FD_PSTAGE, PSTAGE_REG_ID_PSW_0 ) & 0xFFFF, fmtDesc | FMT_HALF_WORD, 8 );
    printTextField((char *) "Ofs:", fmtDesc, 5 );
    printNumericField( glb -> cpu -> getReg( RC_FD_PSTAGE, PSTAGE_REG_ID_PSW_1 ), fmtDesc, 12 );
    printTextField((char *) "ST:", fmtDesc, 4 );
    
    uint32_t stat = glb -> cpu -> getReg( RC_FD_PSTAGE, PSTAGE_REG_ID_PSW_0 );
    
    printTextField(( getBit( stat, ST_MACHINE_CHECK )) ? (char *) "M" : (char *) "m", fmtDesc );
    printTextField(( getBit( stat, ST_EXECUTION_LEVEL )) ? (char *) "X" : (char *) "x", fmtDesc );
    printTextField(( getBit( stat, ST_CODE_TRANSLATION_ENABLE )) ? (char *) "C" : (char *) "c", fmtDesc );
    printTextField(( getBit( stat, ST_NULLIFY )) ? (char *) "N" : (char *) "n", fmtDesc );
    printTextField(( getBit( stat, ST_DIVIDE_STEP )) ? (char *) "V" : (char *) "v", fmtDesc );
    printTextField(( getBit( stat, ST_CARRY )) ? (char *) "C" : (char *) "c", fmtDesc );
    
    printTextField(( getBit( stat, ST_REC_COUNTER )) ? (char *) "R" : (char *) "r", fmtDesc );
    printTextField(( getBit( stat, ST_SINGLE_STEP )) ? (char *) "Z" : (char *) "z", fmtDesc );
    printTextField(( getBit( stat, ST_DATA_TRANSLATION_ENABLE )) ? (char *) "D" : (char *) "d", fmtDesc );
    printTextField(( getBit( stat, ST_PROTECT_ID_CHECK_ENABLE )) ? (char *) "P" : (char *) "p", fmtDesc );
    printTextField(( getBit( stat, ST_INTERRUPT_ENABLE )) ? (char *) "E" : (char *) "e", fmtDesc );
    
    padLine( fmtDesc );
    printRadixField( fmtDesc | FMT_LAST_FIELD );
}

//------------------------------------------------------------------------------------------------------------
// Each window consist of a banner and a body. The body lines are displayed after the banner line. The
// program state window body lists the general and segment registers.
//
//------------------------------------------------------------------------------------------------------------
void SimWinProgState::drawBody( ) {
    
    uint32_t fmtDesc = FMT_DEF_ATTR;
    
    setWinCursor( 2, 1 );
    printTextField((char *) "GR0=", ( fmtDesc | FMT_BOLD | FMT_ALIGN_LFT ), 6 );
    
    for ( int i = 0; i < 4; i++ ) {
        
        printNumericField( glb -> cpu -> getReg( RC_GEN_REG_SET, i ), fmtDesc );
        printTextField((char *) " ", fmtDesc );
    }
       
    printTextField((char *) "GR4=", ( fmtDesc | FMT_BOLD | FMT_ALIGN_LFT ), 6 );
    
    for ( int i = 4; i < 8; i++ ) {
        
        printNumericField( glb -> cpu -> getReg( RC_GEN_REG_SET, i ), fmtDesc );
        printTextField((char *) " ", fmtDesc );
    }
       
    padLine( fmtDesc );
    
    setWinCursor( 3, 1 );
    printTextField((char *) "GR8=", ( fmtDesc | FMT_BOLD | FMT_ALIGN_LFT ), 6 );
    
    for ( int i = 8; i < 12; i++ ) {
        
        printNumericField( glb -> cpu -> getReg( RC_GEN_REG_SET, i ), fmtDesc );
        printTextField((char *) " " );
    }
        
    printTextField((char *) "GR12=", ( fmtDesc | FMT_BOLD | FMT_ALIGN_LFT ), 6 );
    
    for ( int i = 12; i < 16; i++ ) {
        
        printNumericField( glb -> cpu -> getReg( RC_GEN_REG_SET, i ), fmtDesc );
        printTextField((char *) " " );
    }
        
    padLine( fmtDesc );
    
    setWinCursor( 4, 1 );
    printTextField((char *) "SR0=", ( fmtDesc | FMT_BOLD | FMT_ALIGN_LFT ), 6 );
    
    for ( int i = 0; i < 4; i++ ) {
        
        printNumericField( glb -> cpu -> getReg( RC_SEG_REG_SET, i ), fmtDesc );
        printTextField((char *) " " );
    }
        
    printTextField((char *) "SR4=", ( fmtDesc | FMT_BOLD | FMT_ALIGN_LFT ), 6 );
    
    for ( int i = 4; i < 8; i++ ) {
    
        printNumericField( glb -> cpu -> getReg( RC_SEG_REG_SET, i ), fmtDesc );
        printTextField((char *) " " );
    }
        
    padLine( fmtDesc );
}

//***********************************************************************************************************
//***********************************************************************************************************
//
// Methods for the special register window class.
//
//***********************************************************************************************************
//***********************************************************************************************************

//------------------------------------------------------------------------------------------------------------
// Object constructor.
//
//------------------------------------------------------------------------------------------------------------
SimWinSpecialRegs::SimWinSpecialRegs( VCPU32Globals *glb )  : SimWin( glb ) { }

//------------------------------------------------------------------------------------------------------------
// The default values are the initial settings when windows is brought up the first time, or for the WDEF
// command.
//
//------------------------------------------------------------------------------------------------------------
void SimWinSpecialRegs::setDefaults( ) {
    
    setRadix( glb -> env -> getEnvVarInt((char *) ENV_RDX_DEFAULT ));
    setDefColumns( 12 + ( 8 * 11 ), 16 );
    setDefColumns( 12 + ( 8 * 13 ), 8 );
    setDefColumns( 12 + ( 8 * 11 ), 10 );
    setColumns( getDefColumns( getRadix( )));
    setRows( 5 );

    setWinType( WT_CR_WIN );
    setEnable( false );
}

//------------------------------------------------------------------------------------------------------------
// The window overrides the setRadix method to set the column width according to the radix chosen.
//
//------------------------------------------------------------------------------------------------------------
void SimWinSpecialRegs::setRadix( int rdx ) {
    
    SimWin::setRadix( rdx );
    setColumns( getDefColumns( getRadix( )));
}

//------------------------------------------------------------------------------------------------------------
// The banner line for the special register window.
//
//------------------------------------------------------------------------------------------------------------
void SimWinSpecialRegs::drawBanner( ) {
    
    uint32_t fmtDesc = FMT_BOLD | FMT_INVERSE;
    
    setWinCursor( 1, 1 );
    printTextField((char *) "Special Reg", ( fmtDesc | FMT_ALIGN_LFT ), 16 );
    padLine( fmtDesc );
    printRadixField( fmtDesc | FMT_LAST_FIELD );
}

//------------------------------------------------------------------------------------------------------------
// Each window consist of a banner and a body. The body lines are displayed after the banner line. We
// currently just display all control registers. A later version may print the registers a bit more formatted
// with respect to their content.
//
//------------------------------------------------------------------------------------------------------------
void SimWinSpecialRegs::drawBody( ) {
    
    uint32_t fmtDesc = FMT_ALIGN_LFT;
    
    setWinCursor( 2, 1 );
    printTextField((char *) "CR0=  ", ( fmtDesc | FMT_BOLD ));
    
    for ( int i = 0; i < 4; i++ ) {
        
        printNumericField( glb -> cpu -> getReg( RC_CTRL_REG_SET, i ), fmtDesc );
        printTextField((char *) " " );
    }
   
    printTextField((char *) "CR4=  ", ( fmtDesc | FMT_BOLD ));
    
    for ( int i = 4; i < 8; i++ ) {
        
        printNumericField( glb -> cpu -> getReg( RC_CTRL_REG_SET, i ), fmtDesc );
        printTextField((char *) " " );
    }
        
    padLine( fmtDesc );
    
    setWinCursor( 3, 1 );
    printTextField((char *) "CR8=  ", ( fmtDesc | FMT_BOLD ));
    
    for ( int i = 8; i < 12; i++ ) {
        
        printNumericField( glb -> cpu -> getReg( RC_CTRL_REG_SET, i ), fmtDesc );
        printTextField((char *) " " );
    }
   
    printTextField((char *) "CR12= ", ( fmtDesc | FMT_BOLD ));
    
    for ( int i = 12; i < 16; i++ ) {
        
        printNumericField( glb -> cpu -> getReg( RC_CTRL_REG_SET, i ), fmtDesc );
        printTextField((char *) " " );
    }
    
    padLine( fmtDesc );
    
    setWinCursor( 4, 1 );
    printTextField((char *) "CR16= ", ( fmtDesc | FMT_BOLD ));
    
    for ( int i = 16; i < 20; i++ ) {
        
        printNumericField( glb -> cpu -> getReg( RC_CTRL_REG_SET, i ), fmtDesc );
        printTextField((char *) " " );
    }
    
    printTextField((char *) "CR20= ", ( fmtDesc | FMT_BOLD ));
    
    
    for ( int i = 20; i < 24; i++ ) {
        
        printNumericField( glb -> cpu -> getReg( RC_CTRL_REG_SET, i ), fmtDesc );
        printTextField((char *) " " );
    }
    
    padLine( fmtDesc );
    
    setWinCursor( 5, 1 );
    printTextField((char *) "CR24= ", ( fmtDesc | FMT_BOLD ));
    
    for ( int i = 24; i < 28; i++ ) {
        
        printNumericField( glb -> cpu -> getReg( RC_CTRL_REG_SET, i ), fmtDesc );
        printTextField((char *) " " );
    }
   
    printTextField((char *) "CR28= ", ( fmtDesc | FMT_BOLD ));
    
    for ( int i = 28; i < 32; i++ ) {
        
        printNumericField( glb -> cpu -> getReg( RC_CTRL_REG_SET, i ), fmtDesc );
        printTextField((char *) " " );
    }
    
    padLine( fmtDesc );
}

//***********************************************************************************************************
//***********************************************************************************************************
//
// Methods for the CPU24 pipeline register window class.
//
//***********************************************************************************************************
//***********************************************************************************************************

//------------------------------------------------------------------------------------------------------------
// Object constructor.
//
//------------------------------------------------------------------------------------------------------------
SimWinPipeLineRegs::SimWinPipeLineRegs( VCPU32Globals *glb ) : SimWin( glb ) { }

//------------------------------------------------------------------------------------------------------------
// The default values are the initial settings when windows is brought up the first time, or for the WDEF
// command.
//
//------------------------------------------------------------------------------------------------------------
void SimWinPipeLineRegs::setDefaults( ) {
    
    setRadix( glb -> env -> getEnvVarInt((char *) ENV_RDX_DEFAULT ));
    setDefColumns( 84, 16 );
    setDefColumns( 106, 8 );
    setDefColumns( 84, 10 );
    setColumns( getDefColumns( getRadix( )));
    setRows( 4 );
    
    setWinType( WT_PL_WIN );
    setEnable( false );
}

//------------------------------------------------------------------------------------------------------------
// The window overrides the setRadix method to set the column width according to the radix chosen.
//
//------------------------------------------------------------------------------------------------------------
void SimWinPipeLineRegs::setRadix( int rdx ) {
    
    SimWin::setRadix( rdx );
    setColumns( getDefColumns( getRadix( )));
}

//------------------------------------------------------------------------------------------------------------
// The banner line for the pipeline register window. We show the cycle counter in the banner.
//
//------------------------------------------------------------------------------------------------------------
void SimWinPipeLineRegs::drawBanner( ) {
    
    uint32_t fmtDesc = FMT_BOLD | FMT_INVERSE;
    
    setWinCursor( 1, 1 );
    printTextField((char *) "Pipeline", ( fmtDesc | FMT_ALIGN_LFT ), 16 );
    
    printTextField((char *) "ClockSteps: ", fmtDesc );
    printNumericField( glb -> cpu -> stats.clockCntr, fmtDesc );
    
    padLine( fmtDesc );
    printRadixField( fmtDesc | FMT_LAST_FIELD );
}

//------------------------------------------------------------------------------------------------------------
// The pipeline window shows the pipeline registers of the three stages.
//
// ??? can we also print entire lines using pritChars ?
//------------------------------------------------------------------------------------------------------------
void SimWinPipeLineRegs::drawBody( ) {
    
    uint32_t fmtDesc = FMT_DEF_ATTR;
    
    setWinCursor( 2, 1 );
    
    if ( glb -> cpu -> getReg( RC_FD_PSTAGE, PSTAGE_REG_STALLED ) == 1 )
        
        printTextField((char *) "FD(s):", ( fmtDesc | FMT_ALIGN_LFT | FMT_BOLD ), 8 );
    else
        printTextField((char *) "FD:   ", ( fmtDesc | FMT_ALIGN_LFT | FMT_BOLD ), 8 );
    
    printTextField((char *) "PSW:",  ( fmtDesc | FMT_ALIGN_LFT ), 5 );
    printNumericField( getBitField( glb -> cpu -> getReg( RC_FD_PSTAGE, PSTAGE_REG_ID_PSW_0 ), 15, 16 ),
                      ( fmtDesc | FMT_HALF_WORD ));
    printTextField(( char *) ":", ( fmtDesc | FMT_ALIGN_LFT ));
    printNumericField( getBitField( glb -> cpu -> getReg( RC_FD_PSTAGE, PSTAGE_REG_ID_PSW_0 ), 31, 16 ),
                      ( fmtDesc | FMT_HALF_WORD ));
    printTextField(( char *) "." );
    printNumericField( glb -> cpu -> getReg( RC_FD_PSTAGE, PSTAGE_REG_ID_PSW_1 ) );
    padLine( fmtDesc );
    
    setWinCursor( 3, 1 );
    
    if ( glb -> cpu -> getReg( RC_MA_PSTAGE, PSTAGE_REG_STALLED ) == 1 )
        
        printTextField((char *) "MA(s):", ( fmtDesc | FMT_ALIGN_LFT | FMT_BOLD ), 8 );
    else
        printTextField((char *) "MA:   ", ( fmtDesc | FMT_ALIGN_LFT | FMT_BOLD ), 8 );
    
    printTextField((char *) "PSW:",  ( fmtDesc | FMT_ALIGN_LFT ), 5 );
    printNumericField( getBitField( glb -> cpu -> getReg( RC_MA_PSTAGE, PSTAGE_REG_ID_PSW_0 ), 15, 16 ),
                      ( fmtDesc | FMT_HALF_WORD ));
    printTextField(( char *) ":", ( fmtDesc | FMT_ALIGN_LFT ));
    printNumericField( getBitField( glb -> cpu -> getReg( RC_MA_PSTAGE, PSTAGE_REG_ID_PSW_0 ), 31, 16 ),
                      ( fmtDesc | FMT_HALF_WORD ));
    printTextField(( char *) "." );
    printNumericField( glb -> cpu -> getReg( RC_MA_PSTAGE, PSTAGE_REG_ID_PSW_1 ) );
    padLine( fmtDesc );
    
    printTextField(( char *) "  I: " );
    printNumericField( glb -> cpu -> getReg( RC_MA_PSTAGE, PSTAGE_REG_ID_INSTR ));
    printTextField(( char *) "  A: " );
    printNumericField( glb -> cpu -> getReg( RC_MA_PSTAGE, PSTAGE_REG_ID_VAL_A ));
    printTextField(( char *) "  B: " );
    printNumericField( glb -> cpu -> getReg( RC_MA_PSTAGE, PSTAGE_REG_ID_VAL_B ));
    printTextField(( char *) "  X: " );
    printNumericField( glb -> cpu -> getReg( RC_MA_PSTAGE, PSTAGE_REG_ID_VAL_X ));
    padLine( fmtDesc );
    
    setWinCursor( 4, 1 );
    
    if ( glb -> cpu -> getReg( RC_EX_PSTAGE, PSTAGE_REG_STALLED ) == 1 )
        printTextField((char *) "EX(s):", ( fmtDesc | FMT_ALIGN_LFT | FMT_BOLD ), 8 );
    else
        printTextField((char *) "EX:   ", ( fmtDesc | FMT_ALIGN_LFT | FMT_BOLD ), 8 );
    
    printTextField((char *) "PSW:",  ( fmtDesc | FMT_ALIGN_LFT ), 5 );
    printNumericField( getBitField( glb -> cpu -> getReg( RC_EX_PSTAGE, PSTAGE_REG_ID_PSW_0 ), 15, 16 ),
                      ( fmtDesc | FMT_HALF_WORD ));
    printTextField(( char *) ":", ( fmtDesc | FMT_ALIGN_LFT ));
    printNumericField( getBitField( glb -> cpu -> getReg( RC_EX_PSTAGE, PSTAGE_REG_ID_PSW_0 ), 31, 16 ),
                      ( fmtDesc | FMT_HALF_WORD ));
    printTextField(( char *) "." );
    printNumericField( glb -> cpu -> getReg( RC_EX_PSTAGE, PSTAGE_REG_ID_PSW_1 ) );
    padLine( fmtDesc );
    
    printTextField(( char *) "  I: " );
    printNumericField( glb -> cpu -> getReg( RC_EX_PSTAGE, PSTAGE_REG_ID_INSTR ));
    printTextField(( char *) "  A: " );
    printNumericField( glb -> cpu -> getReg( RC_EX_PSTAGE, PSTAGE_REG_ID_VAL_A ));
    printTextField(( char *) "  B: " );
    printNumericField( glb -> cpu -> getReg( RC_EX_PSTAGE, PSTAGE_REG_ID_VAL_B ));
    printTextField(( char *) "  X: " );
    printNumericField( glb -> cpu -> getReg( RC_EX_PSTAGE, PSTAGE_REG_ID_VAL_X ));
    padLine( fmtDesc );
}

//***********************************************************************************************************
//***********************************************************************************************************
//
// Methods for the CPU24 statistics window class.
//
//***********************************************************************************************************
//***********************************************************************************************************

//------------------------------------------------------------------------------------------------------------
// Object constructor.
//
//------------------------------------------------------------------------------------------------------------
SimWinStatistics::SimWinStatistics( VCPU32Globals *glb ) : SimWin( glb ) { }

//------------------------------------------------------------------------------------------------------------
// The default values are the initial settings when windows is brought up the first time, or for the WDEF
// command.
//
//------------------------------------------------------------------------------------------------------------
void SimWinStatistics::setDefaults( ) {
    
    setRadix( glb -> env -> getEnvVarInt((char *) ENV_RDX_DEFAULT ));
    setWinType( WT_ST_WIN );
    setEnable( false );
    setRows( 4 );
    setColumns( 84 );
    setDefColumns( 84 );
}

//------------------------------------------------------------------------------------------------------------
// The banner line for the statistics window.
//
//------------------------------------------------------------------------------------------------------------
void SimWinStatistics::drawBanner( ) {
    
    uint32_t fmtDesc = FMT_BOLD | FMT_INVERSE;
    
    setWinCursor( 1, 1 );
    printTextField((char *) "Statistics", ( fmtDesc | FMT_ALIGN_LFT ), 16) ;
    printTextField((char *) "ClockSteps: ", fmtDesc );
    printNumericField( glb -> cpu -> stats.clockCntr, fmtDesc );
    padLine( fmtDesc );
    printRadixField( fmtDesc | FMT_LAST_FIELD );
}

//------------------------------------------------------------------------------------------------------------
// The data window for the statistics.
//
// ??? work in progress...
//------------------------------------------------------------------------------------------------------------
void SimWinStatistics::drawBody( ) {
    
    uint32_t fmtDesc = FMT_DEF_ATTR;
    
    setWinCursor( 2, 1 );
    
    printTextField((char *) "... ", fmtDesc | FMT_ALIGN_LFT, 32 );
    padLine( fmtDesc );
}

//***********************************************************************************************************
//***********************************************************************************************************
//
// Methods for the physical memory window class.
//
//***********************************************************************************************************
//***********************************************************************************************************

//------------------------------------------------------------------------------------------------------------
// Object constructor.
//
//------------------------------------------------------------------------------------------------------------
SimWinAbsMem::SimWinAbsMem( VCPU32Globals *glb ) : SimWinScrollable( glb ) { }

//------------------------------------------------------------------------------------------------------------
// The default values are the initial settings when windows is brought up the first time, or for the WDEF
// command. The memory window is a window where the number of lines to display can be set. However, the
// minimum is the default number of lines.
//
//------------------------------------------------------------------------------------------------------------
void SimWinAbsMem::setDefaults( ) {
    
    setRadix( glb -> env -> getEnvVarInt((char *) ENV_RDX_DEFAULT ));
    setDefColumns( 12 + ( 8 * 11 ), 16 );
    setDefColumns( 14 + ( 8 * 13 ), 8 );
    setDefColumns( 12 + ( 8 * 11 ), 10 );
    setColumns( getDefColumns( getRadix( )));
    
    setWinType( WT_PM_WIN );
    setEnable( false );
    setRows( 5 );
    setHomeItemAdr( 0 );
    setCurrentItemAdr( 0 );
    setLineIncrement( 8 * 4 );
    setLimitItemAdr( 0 );
}

//------------------------------------------------------------------------------------------------------------
// The window overrides the setRadix method to set the column width according to the radix chosen.
//
//------------------------------------------------------------------------------------------------------------
void SimWinAbsMem::setRadix( int rdx ) {
    
    SimWin::setRadix( rdx );
    setColumns( getDefColumns( getRadix( )));
}

//------------------------------------------------------------------------------------------------------------
// The banner line shows the item address, which is the current absolute physical memory address where the
// window body will start to display. We also need to set the item address limit. This is however always the
// maximum value UINT_MAX, as absolute memory is up to 4G bytes. The drawLine method will actually check that
// the offset passed is valid and invoke the correct absolute memory portion handler.
//
//------------------------------------------------------------------------------------------------------------
void SimWinAbsMem::drawBanner( ) {
    
    uint32_t    fmtDesc     = FMT_BOLD | FMT_INVERSE;
    uint32_t    currentAdr  = getCurrentItemAdr( );
    CpuMem      *physMem    = glb -> cpu -> physMem;
    CpuMem      *pdcMem     = glb -> cpu -> pdcMem;
    CpuMem      *ioMem      = glb -> cpu -> ioMem;
    bool        isCurrent   = glb -> winDisplay -> isCurrentWin( getWinIndex( ));
  
    setWinCursor( 1, 1 );
    printWindowIdField( getWinStack( ), getWinIndex( ), isCurrent, fmtDesc );
    
    if (( physMem != nullptr ) && ( physMem -> validAdr( currentAdr ))) {
        
        printTextField((char *) "Main Memory ", ( fmtDesc | FMT_ALIGN_LFT ), 16 );
    }
    else if (( pdcMem != nullptr ) && ( pdcMem -> validAdr( currentAdr ))) {
        
        printTextField((char *) "PDC Memory ", ( fmtDesc | FMT_ALIGN_LFT ), 16 );
    }
    else if (( ioMem != nullptr ) && ( ioMem -> validAdr( currentAdr ))) {
        
        printTextField((char *) "IO Memory ", ( fmtDesc | FMT_ALIGN_LFT ), 16 );
    }
    else printTextField((char *) "**** Memory ", ( fmtDesc | FMT_ALIGN_LFT ), 16 );
    
    printTextField((char *) "Current " );
    printNumericField( getCurrentItemAdr( ));
    printTextField((char *) "  Home: " );
    printNumericField(  getHomeItemAdr( ));
    padLine( fmtDesc );
    printRadixField( fmtDesc | FMT_LAST_FIELD );
    
    setLimitItemAdr( UINT_MAX );
}

//------------------------------------------------------------------------------------------------------------
// A scrollable window needs to implement a routine for displaying a row. We are passed the item address and
// need to map this to the actual meaning of the particular window. The "itemAdr" value is the byte offset
// into physical memory, the line increment is 8 * 4 = 32 bytes. We show eight words.
//
//------------------------------------------------------------------------------------------------------------
void SimWinAbsMem::drawLine( uint32_t itemAdr ) {
    
    uint32_t    fmtDesc     = FMT_DEF_ATTR;
    uint32_t    limit       = getLineIncrement( ) - 1;
    CpuMem      *physMem    = glb -> cpu -> physMem;
    CpuMem      *pdcMem     = glb -> cpu -> pdcMem;
    CpuMem      *ioMem      = glb -> cpu -> ioMem;
    
    printNumericField( itemAdr, fmtDesc );
    printTextField((char *) ": ", fmtDesc );
    
    for ( int i = 0; i < limit; i = i + 4 ) {
        
        uint32_t ofs = itemAdr + i;
        
        if (( physMem != nullptr ) && ( physMem -> validAdr( ofs ))) {
            
            printNumericField( physMem -> getMemDataWord( ofs ), fmtDesc );
        }
        else if (( pdcMem != nullptr ) && ( pdcMem -> validAdr( ofs ))) {
            
            printNumericField( pdcMem -> getMemDataWord( ofs ), fmtDesc );
        }
        else if (( ioMem != nullptr ) && ( ioMem -> validAdr( ofs ))) {
            
            printNumericField( ioMem -> getMemDataWord( ofs ), fmtDesc );
        }
        else printNumericField( 0, fmtDesc | FMT_INVALID_NUM );
        
        printTextField((char *) " " );
    }
}

//***********************************************************************************************************
//***********************************************************************************************************
//
// Methods for the code memory window class.
//
//***********************************************************************************************************
//***********************************************************************************************************

//------------------------------------------------------------------------------------------------------------
// Object constructor. We create a dfisassembler object for displaying the decoded instructions.
//
// ??? check math for wond scrolling, debug: 0xf000000 + forward ....
// ??? need to rework for virtual addresses ? We need to work with segment and offset !!!!
//------------------------------------------------------------------------------------------------------------
SimWinCode::SimWinCode( VCPU32Globals *glb ) : SimWinScrollable( glb ) {
    
    disAsm = new SimDisAsm( );
}

//------------------------------------------------------------------------------------------------------------
// The default values are the initial settings when windows is brought up the first time, or for the WDEF
// command. The code memory window is a window where the number of lines to display can be set. However,
// the minimum is the default number of lines.
//
//------------------------------------------------------------------------------------------------------------
void SimWinCode::setDefaults( ) {
    
    setRadix( glb -> env -> getEnvVarInt((char *) ENV_RDX_DEFAULT ));
    setColumns( 84 );
    setDefColumns( 84 );
    setRows( 9 );
    setHomeItemAdr( 0 );
    setCurrentItemAdr( glb -> cpu -> getReg( RC_FD_PSTAGE, PSTAGE_REG_ID_PSW_1 ));
    setLineIncrement( 4 );
    setLimitItemAdr( UINT_MAX );
    setWinType( WT_PC_WIN );
    setEnable( false );
}

//------------------------------------------------------------------------------------------------------------
// The banner for the code window shows the code address. It would be nice to automatically scroll the window
// for the single step command. We detect this by examining the current command and adjust the current item
// address to scroll to the next lines to show.
//
//------------------------------------------------------------------------------------------------------------
void SimWinCode::drawBanner( ) {
    
    uint32_t    fmtDesc             = FMT_BOLD | FMT_INVERSE;
    uint32_t    currentItemAdr      = getCurrentItemAdr( );
    uint32_t    currentItemAdrLimit = currentItemAdr + (( getRows( ) - 1 ) * getLineIncrement( ));
    uint32_t    currentIaOfs        = (int) glb -> cpu -> getReg( RC_FD_PSTAGE, PSTAGE_REG_ID_PSW_1  );
    SimTokId    currentCmd          = glb -> cmdWin -> getCurrentCmd( );
    bool        isCurrent           = glb -> winDisplay -> isCurrentWin( getWinIndex( ));
    bool        hasIaOfsAdr         = (( currentIaOfs >= currentItemAdr ) && ( currentIaOfs <= currentItemAdrLimit ));
    
    if (( currentCmd == CMD_STEP ) && ( hasIaOfsAdr )) {
        
        if      ( currentIaOfs >= currentItemAdrLimit ) winJump( currentIaOfs );
        else if ( currentIaOfs < currentItemAdr )       winJump( currentIaOfs );
    }
    
    setWinCursor( 1, 1 );
    printWindowIdField( getWinStack( ), getWinIndex( ), isCurrent, fmtDesc );
    printTextField((char *) "Code Memory ", ( fmtDesc | FMT_ALIGN_LFT ), 16 );
    printTextField((char *) "Current: " );
    printNumericField( getCurrentItemAdr( ));
    printTextField((char *) "  Home: " );
    printNumericField(  getHomeItemAdr( ));
    padLine( fmtDesc );
    printRadixField( fmtDesc | FMT_LAST_FIELD );
}

//------------------------------------------------------------------------------------------------------------
// A scrollable window needs to implement a routine for displaying a row. We are passed the item address and
// need to map this to the actual meaning of the particular window. The disassembled format is printed in
// two parts, the first is the instruction and options, the second is the target and operand field. We make
// sure that both parts are nicely aligned.
//
//------------------------------------------------------------------------------------------------------------
void SimWinCode::drawLine( uint32_t itemAdr ) {
    
    uint32_t    fmtDesc         = FMT_DEF_ATTR;
    bool        plWinEnabled    = glb -> winDisplay -> isWinEnabled( PL_REG_WIN );
    CpuMem      *physMem        = glb -> cpu -> physMem;
    CpuMem      *pdcMem         = glb -> cpu -> pdcMem;
    CpuMem      *ioMem          = glb -> cpu -> ioMem;
    uint32_t    instr           = 0xFFFFFFFF;
    char        buf[ 128 ]      = { 0 };
    
    if (( physMem != nullptr ) && ( physMem -> validAdr( itemAdr ))) {
        
        instr = physMem -> getMemDataWord( itemAdr );
    }
    else if (( pdcMem != nullptr ) && ( pdcMem -> validAdr( itemAdr ))) {
        
        instr = pdcMem -> getMemDataWord( itemAdr );
    }
    else if (( ioMem != nullptr ) && ( ioMem -> validAdr( itemAdr ))) {
        
        instr = ioMem -> getMemDataWord( itemAdr );
    }

    printNumericField( itemAdr, fmtDesc | FMT_ALIGN_LFT, 12 );
  
    if (( plWinEnabled ) && ( itemAdr == glb -> cpu -> getReg( RC_FD_PSTAGE, PSTAGE_REG_ID_PSW_1  ))) {
            
        printTextField((char *) "(fd)>", fmtDesc, 5 );
    }
    else if (( plWinEnabled ) && ( itemAdr == glb -> cpu -> getReg( RC_MA_PSTAGE, PSTAGE_REG_ID_PSW_1  ))) {
            
        printTextField((char *) "(ma) ", fmtDesc, 5 );
    }
    else if (( plWinEnabled ) && ( itemAdr == glb -> cpu -> getReg( RC_EX_PSTAGE, PSTAGE_REG_ID_PSW_1  ))) {
            
        printTextField((char *) "(ex) ", fmtDesc, 5 );
    }
    else if ( itemAdr == glb -> cpu -> getReg( RC_FD_PSTAGE, PSTAGE_REG_ID_PSW_1 )) {
        
        printTextField((char *) "    >", fmtDesc, 5 );
    }
    else {
        
        printTextField((char *) "     ", fmtDesc, 5 );
    }
   
    printNumericField( instr, fmtDesc | FMT_ALIGN_LFT, 12 );
    
    int pos          = getWinCursorCol( );
    int opCodeField  = disAsm -> getOpCodeOptionsFieldWidth( );
    int operandField = disAsm -> getOpCodeOptionsFieldWidth( );
    
    clearField( opCodeField );
    disAsm -> formatOpCodeAndOptions( buf, sizeof( buf ), instr );
    printText( buf, (int) strlen( buf ));
    setWinCursor( 0, pos + opCodeField );
    
    clearField( operandField );
    disAsm -> formatTargetAndOperands( buf, sizeof( buf ), instr );
    printText( buf, (int) strlen( buf ));
    setWinCursor( 0, pos + opCodeField + operandField );
    padLine( );
}

//***********************************************************************************************************
//***********************************************************************************************************
//
// Methods for the TLB class.
//
//***********************************************************************************************************
//***********************************************************************************************************

//------------------------------------------------------------------------------------------------------------
// Object constructor. All we do is to remember what kind of TLB this is.
//
//------------------------------------------------------------------------------------------------------------
SimWinTlb::SimWinTlb( VCPU32Globals *glb, int winType ) : SimWinScrollable( glb ) {
    
    this -> winType = winType;
}

//------------------------------------------------------------------------------------------------------------
// We have a function to set reasonable default values for the window. The default values are the initial
// settings when windows is brought up the first time, or for the WDEF command. The TLB window is a window
// where the number of lines to display can be set. However, the minimum is the default number of lines.
//
//------------------------------------------------------------------------------------------------------------
void SimWinTlb::setDefaults( ) {
    
    setRadix( glb -> env -> getEnvVarInt((char *) ENV_RDX_DEFAULT ));
    setDefColumns( 84, 16 );
    setDefColumns( 102, 8 );
    setDefColumns( 84, 10 );
    setColumns( getDefColumns( getRadix( )));
    setWinType( winType );
    setEnable( false );
    setRows( 5 );
    setCurrentItemAdr( 0 );
    setLineIncrement( 1 );
    setLimitItemAdr( 0 );
    
    if      ( winType == WT_ITLB_WIN ) tlb = glb -> cpu -> iTlb;
    else if ( winType == WT_DTLB_WIN ) tlb = glb -> cpu -> dTlb;
}

//------------------------------------------------------------------------------------------------------------
// The window overrides the setRadix method to set the column width according to the radix chosen.
//
//------------------------------------------------------------------------------------------------------------
void SimWinTlb::setRadix( int rdx ) {
    
    SimWin::setRadix( rdx );
    setColumns( getDefColumns( getRadix( )));
}

//------------------------------------------------------------------------------------------------------------
// Each window consist of a banner and a body. The banner line is always shown in inverse and contains
// summary or head data for the window. We also need to set the item address limit. As this can change with
// some commands outside the windows system, better set it every time.
//
//------------------------------------------------------------------------------------------------------------
void SimWinTlb::drawBanner( ) {
    
    uint32_t    fmtDesc     = FMT_BOLD | FMT_INVERSE;
    bool        isCurrent   = glb -> winDisplay -> isCurrentWin( getWinIndex( ));
    
    setWinCursor( 1, 1 );
    printWindowIdField( getWinStack( ), getWinIndex( ), isCurrent, fmtDesc );
    
    if      ( winType == WT_ITLB_WIN ) printTextField((char *) "I-TLB ", ( fmtDesc | FMT_ALIGN_LFT ), 16 );
    else if ( winType == WT_DTLB_WIN ) printTextField((char *) "D-TLB ", ( fmtDesc | FMT_ALIGN_LFT ), 16 );
    else                               printTextField((char *) "***** ", ( fmtDesc | FMT_ALIGN_LFT ), 16 );
    
    printTextField((char *) "Current: " );
    printNumericField( getCurrentItemAdr( ));
    printTextField((char *) "  Home: " );
    printNumericField(  getHomeItemAdr( ));
    padLine( fmtDesc );
    printRadixField( fmtDesc | FMT_LAST_FIELD );
    
    setLimitItemAdr( tlb -> getTlbSize( ));
}

//------------------------------------------------------------------------------------------------------------
// Each window consist of a banner and a body. The body lines are displayed after the banner line. The
// number of lines can vary. A line represents an entry in the respective TLB.
//
//------------------------------------------------------------------------------------------------------------
void SimWinTlb::drawLine( uint32_t index ) {
    
    uint32_t  fmtDesc = FMT_DEF_ATTR;
    
    printNumericField( index, fmtDesc );
    printTextField((char *) ":[", fmtDesc );
    
    if ( index > tlb -> getTlbSize( )) {
  
        printTextField((char *) "Invalid TLB index", fmtDesc );
        printTextField((char *) "]", fmtDesc );
        padLine( );
    }
    else {
        
        TlbEntry   *tEntry  = tlb -> getTlbEntry( index );
        char            tmpBuf[ 32 ];
        
        printTextField((( tEntry -> tValid( )) ? (char *) "V" : (char *) "v" ), fmtDesc );
        printTextField((( tEntry -> tDirty( )) ? (char *) "D" : (char *) "d" ), fmtDesc );
        printTextField((( tEntry -> tTrapPage( )) ? (char *) "P" : (char *) "p" ), fmtDesc );
        printTextField((( tEntry -> tTrapDataPage( )) ? (char *) "D" : (char *) "d" ), fmtDesc );
        printTextField((char *) "]", fmtDesc );
        
        buildAccessRightsStr( tmpBuf, 32, tEntry ->tPageType( ), tEntry -> tPrivL1( ), tEntry -> tPrivL2( ));
        printTextField((char *) " ACC:", fmtDesc );
        printTextField( tmpBuf, fmtDesc );
        printTextField((char *) " PID:", fmtDesc );
        printNumericField( tEntry -> tSegId( ), fmtDesc| FMT_HALF_WORD );
        printTextField((char *) " VPN:", fmtDesc );
        printNumericField( tEntry -> vpnHigh, fmtDesc );
        printTextField((char *) ".", fmtDesc );
        printNumericField( tEntry -> vpnLow, fmtDesc );
        printTextField((char *) " PPN:", fmtDesc );
        printNumericField( tEntry -> tPhysPage( ), fmtDesc );
    }
}

//***********************************************************************************************************
//***********************************************************************************************************
//
// Methods for the Cache class.
//
//***********************************************************************************************************
//***********************************************************************************************************

//------------------------------------------------------------------------------------------------------------
// Object constructor. All we do is to remember what kind of Cache this is.
//
//------------------------------------------------------------------------------------------------------------
SimWinCache::SimWinCache( VCPU32Globals *glb, int winType ) : SimWinScrollable( glb ) {
    
    this -> winType = winType;
}

//------------------------------------------------------------------------------------------------------------
// We have a function to set reasonable default values for the window. The default values are the initial
// settings when windows is brought up the first time, or for the WDEF command. The TLB window is a window
// where the number of lines to display can be set. However, the minimum is the default number of lines.
//
//------------------------------------------------------------------------------------------------------------
void SimWinCache::setDefaults( ) {
    
    if      ( winType == WT_ICACHE_WIN )    cPtr = glb -> cpu -> iCacheL1;
    else if ( winType == WT_DCACHE_WIN )    cPtr = glb -> cpu -> dCacheL1;
    else if ( winType == WT_UCACHE_WIN )    cPtr = glb -> cpu -> uCacheL2;
    
    uint32_t wordsPerBlock = cPtr -> getBlockSize( ) / 4;
    
    setRadix( glb -> env -> getEnvVarInt((char *) ENV_RDX_DEFAULT ));
    setDefColumns( 36 + ( wordsPerBlock * 11 ), 16 );
    setDefColumns( 36 + ( wordsPerBlock * 13 ), 8 );
    setDefColumns( 36 + ( wordsPerBlock * 11 ), 10 );
    setColumns( getDefColumns( getRadix( )));
    setRows( 6 );
    setWinType( winType );
    setEnable( false );
    setCurrentItemAdr( 0 );
    setLineIncrement( 1 );
    setLimitItemAdr( 0 );
    winToggleVal = 0;
}

//------------------------------------------------------------------------------------------------------------
// The window overrides the setRadix method to set the column width according to the radix chosen.
//
//------------------------------------------------------------------------------------------------------------
void SimWinCache::setRadix( int rdx ) {
    
    SimWin::setRadix( rdx );
    setColumns( getDefColumns( getRadix( )));
}

//------------------------------------------------------------------------------------------------------------
// We allow for toggling through the sets if the cache is an n-way associative cache.
//
//------------------------------------------------------------------------------------------------------------
void SimWinCache::toggleWin( ) {
    
    uint32_t blockSize   = cPtr -> getBlockSets( );
    winToggleVal = ( winToggleVal + 1 ) % blockSize;
}
    
//------------------------------------------------------------------------------------------------------------
// Each window consist of a headline and a body. The banner line is always shown in inverse and contains
// summary or head data for the window. We also need to set the item address limit. As this can change with
// some commands outside the windows system, better set it every time.
//
//------------------------------------------------------------------------------------------------------------
void SimWinCache::drawBanner( ) {
    
    uint32_t    fmtDesc     = FMT_BOLD | FMT_INVERSE;
    bool        isCurrent   = glb -> winDisplay -> isCurrentWin( getWinIndex( ));
    
    setWinCursor( 1, 1 );
    printWindowIdField( getWinStack( ), getWinIndex( ), isCurrent, fmtDesc );
    
    if      ( winType == WT_ICACHE_WIN ) printTextField((char *) "I-Cache (L1) ", ( fmtDesc | FMT_ALIGN_LFT ), 16 );
    else if ( winType == WT_DCACHE_WIN ) printTextField((char *) "D-Cache (L1)", ( fmtDesc | FMT_ALIGN_LFT ), 16 );
    else if ( winType == WT_UCACHE_WIN ) printTextField((char *) "U-Cache (L2)", ( fmtDesc | FMT_ALIGN_LFT ), 16 );
    else                                 printTextField((char *) "******* ", ( fmtDesc | FMT_ALIGN_LFT ), 16 );
  
    setLimitItemAdr( cPtr -> getBlockEntries( ));
    printTextField((char *) "Set: " );
    printNumericField( winToggleVal, ( fmtDesc | FMT_HALF_WORD ));
    printTextField((char *) " Current: " );
    printNumericField( getCurrentItemAdr( ));
    printTextField((char *) "  Home: " );
    printNumericField(  getHomeItemAdr( ));
    padLine( fmtDesc );
    printRadixField( fmtDesc | FMT_LAST_FIELD );
}

//------------------------------------------------------------------------------------------------------------
// The draw line methods for the cache lists a cache entry. There are various cache line sizes. And there
// are up to two sets of cache data.
//
//------------------------------------------------------------------------------------------------------------
void SimWinCache::drawLine( uint32_t index ) {
    
    uint32_t  fmtDesc   = FMT_DEF_ATTR;
 
    if ( index > cPtr -> getBlockEntries( )) {
  
        printNumericField( index, fmtDesc );
        printTextField((char *) ":[", fmtDesc );
        printTextField((char *) "Invalid Cache index", fmtDesc );
        printTextField((char *) "]", fmtDesc );
        padLine( );
    }
    else {
        
        MemTagEntry *tagPtr         = cPtr -> getMemTagEntry( index, winToggleVal );
        uint32_t    *dataPtr        = (uint32_t *) cPtr -> getMemBlockEntry( index, winToggleVal );
        uint32_t    wordsPerBlock   = cPtr -> getBlockSize( ) / 4;
      
        printNumericField( index, fmtDesc );
        printTextField((char *) ":[", fmtDesc );
        printTextField((( tagPtr -> valid ) ? (char *) "V" : (char *) "v" ), fmtDesc );
        printTextField((( tagPtr -> dirty ) ? (char *) "D" : (char *) "d" ), fmtDesc );
        printTextField((char *) "] (", fmtDesc );
        printNumericField( tagPtr -> tag, fmtDesc );
        printTextField((char *) ") ", fmtDesc );
        
        for ( uint32_t i = 0; i < wordsPerBlock; i++ ) {
          
            printNumericField( dataPtr[ i ], fmtDesc );
            printTextField((char *) " " );
        }
    }
}

//***********************************************************************************************************
//***********************************************************************************************************
//
// Methods for the memory object register class.
//
//***********************************************************************************************************
//***********************************************************************************************************

//------------------------------------------------------------------------------------------------------------
// Object constructor. All we do is to remember what kind of memory object this is.
//
//------------------------------------------------------------------------------------------------------------
SimWinMemController::SimWinMemController( VCPU32Globals *glb, int winType ) : SimWin( glb ) {
    
    this -> winType = winType;
}

//------------------------------------------------------------------------------------------------------------
// Set up reasonable defaults and the reference to the actual memory object.
//
//------------------------------------------------------------------------------------------------------------
void SimWinMemController::setDefaults( ) {
    
    if      ( winType == WT_ICACHE_S_WIN )  cPtr = glb -> cpu -> iCacheL1;
    else if ( winType == WT_DCACHE_S_WIN )  cPtr = glb -> cpu -> dCacheL1;
    else if ( winType == WT_UCACHE_S_WIN )  cPtr = glb -> cpu -> uCacheL2;
    else if ( winType == WT_MEM_S_WIN    )  cPtr = glb -> cpu -> physMem;
    
    setRadix( glb -> env -> getEnvVarInt((char *) ENV_RDX_DEFAULT ));
    setDefColumns( 84, 16 );
    setDefColumns( 108, 8 );
    setDefColumns( 84, 10 );
    setColumns( getDefColumns( getRadix( )));
    setWinType( winType );
    setEnable( false );
    setRows(( winType == WT_MEM_S_WIN ) ? 3 : 4 );
}

//------------------------------------------------------------------------------------------------------------
// Draw the memory object banner. We will display the static configuration data for the memory object.
//
//------------------------------------------------------------------------------------------------------------
void SimWinMemController::drawBanner( ) {
    
    uint32_t    fmtDesc     = FMT_BOLD | FMT_INVERSE;
    bool        isCurrent   = glb -> winDisplay -> isCurrentWin( getWinIndex( ));
    
    setWinCursor( 1, 1 );
    printWindowIdField( getWinStack( ), getWinIndex( ), isCurrent, fmtDesc );
    
    if      ( winType == WT_ICACHE_S_WIN )
        printTextField((char *) "I-Cache (L1)", ( fmtDesc | FMT_ALIGN_LFT ), 16 );
    else if ( winType == WT_DCACHE_S_WIN )
        printTextField((char *) "D-Cache (L1)", ( fmtDesc | FMT_ALIGN_LFT ), 16 );
    else if ( winType == WT_UCACHE_S_WIN )
        printTextField((char *) "U-Cache (L2)", ( fmtDesc | FMT_ALIGN_LFT ), 16 );
    else if ( winType == WT_MEM_S_WIN )
        printTextField((char *) "MEM Reg Set", ( fmtDesc | FMT_ALIGN_LFT ), 16 );
    else if ( winType == WT_PDC_S_WIN )
        printTextField((char *) "PdcMEM Reg Set", ( fmtDesc | FMT_ALIGN_LFT ), 16 );
    else if ( winType == WT_IO_S_WIN )
        printTextField((char *) "IoMEM Reg Set", ( fmtDesc | FMT_ALIGN_LFT ), 16 );
    else
        printTextField((char *) "******* ", ( fmtDesc | FMT_ALIGN_LFT ), 16 );
    
    printTextField((char *) "Range: " );
    printNumericField( cPtr -> getMemCtrlReg( MC_REG_START_ADR ), fmtDesc );
    printTextField((char *) ":" );
    printNumericField( cPtr -> getMemCtrlReg( MC_REG_END_ADR ), fmtDesc );
    
    printTextField((char *) ", Blocks: " );
    printNumericField( cPtr -> getMemCtrlReg( MC_REG_BLOCK_ENTRIES ), fmtDesc );
    printTextField((char *) ":", fmtDesc );
    printNumericField( cPtr -> getMemCtrlReg( MC_REG_BLOCK_SIZE ), ( fmtDesc | FMT_HALF_WORD ));
    
    if (( winType != WT_MEM_S_WIN ) && ( winType != WT_PDC_S_WIN ) && ( winType != WT_IO_S_WIN )) {
        
        printTextField((char *) ", Sets: " );
        printNumericField( cPtr -> getMemCtrlReg( MC_REG_SETS ), ( fmtDesc | FMT_HALF_WORD ));
    }
    
    padLine( fmtDesc );
    printRadixField( fmtDesc | FMT_LAST_FIELD );
}

//------------------------------------------------------------------------------------------------------------
// Display the memory object machine state and the actual requests.
//
//------------------------------------------------------------------------------------------------------------
void SimWinMemController::drawBody( ) {
    
    uint32_t fmtDesc = FMT_DEF_ATTR;
    
    setWinCursor( 2, 1 );
    
    printTextField((char *) "State:", ( fmtDesc | FMT_ALIGN_LFT ), 10 );
    printTextField( cPtr -> getMemOpStr( cPtr -> getMemCtrlReg( MC_REG_STATE )),
                   ( fmtDesc | FMT_ALIGN_LFT ), 20 );
    
    setWinCursor( 3, 1 );
    printTextField((char *) "Request:", ( fmtDesc | FMT_ALIGN_LFT ));
   
    if (( winType == WT_MEM_S_WIN ) || ( winType == WT_PDC_S_WIN ) || ( winType == WT_IO_S_WIN )) {
        
        setWinCursor( 3, 11 );
        printTextField((char *) "Adr:", ( fmtDesc | FMT_ALIGN_LFT ));
        printNumericField( cPtr -> getMemCtrlReg( MC_REG_REQ_OFS ));
        printTextField((char *) "  Len: ", fmtDesc );
        printNumericField( cPtr -> getMemCtrlReg( MC_REG_REQ_LEN ), ( fmtDesc | FMT_HALF_WORD ));
        printTextField((char * ) "  Pri: ", ( fmtDesc | FMT_ALIGN_LFT | FMT_HALF_WORD ));
        printNumericField( cPtr -> getMemCtrlReg( MC_REG_REQ_PRI ));
        printTextField((char * ) "  Lat: ", ( fmtDesc | FMT_ALIGN_LFT | FMT_HALF_WORD ));
        printNumericField( cPtr -> getMemCtrlReg( MC_REG_REQ_LATENCY ));
    }
    else {
        
        setWinCursor( 3, 11 );
        printTextField((char *) "Seg:ofs:", ( fmtDesc | FMT_ALIGN_LFT ));
        printNumericField( cPtr -> getMemCtrlReg( MC_REG_REQ_SEG ));
        printTextField((char *) ":", fmtDesc );
        printNumericField( cPtr -> getMemCtrlReg( MC_REG_REQ_OFS ));
        printTextField((char *) "   Tag: ", fmtDesc );
        printNumericField( cPtr -> getMemCtrlReg( MC_REG_REQ_TAG ));
        printTextField((char *) "  Len: ", fmtDesc );
        printNumericField( cPtr -> getMemCtrlReg( MC_REG_REQ_LEN ), ( fmtDesc | FMT_HALF_WORD ));
        
        setWinCursor( 4, 11 );
        printTextField((char * ) "Pri: ", ( fmtDesc | FMT_ALIGN_LFT | FMT_HALF_WORD ));
        printNumericField( cPtr -> getMemCtrlReg( MC_REG_REQ_PRI ));
        printTextField((char * ) "  Lat: ", ( fmtDesc | FMT_ALIGN_LFT | FMT_HALF_WORD ));
        printNumericField( cPtr -> getMemCtrlReg( MC_REG_REQ_LATENCY ));
        printTextField((char * ) "  tSet: ", ( fmtDesc | FMT_ALIGN_LFT | FMT_HALF_WORD ));
        printNumericField( cPtr -> getMemCtrlReg( MC_REG_REQ_BLOCK_SET ));
        printTextField((char * ) "  tBlock: ", ( fmtDesc | FMT_ALIGN_LFT ));
        printNumericField( cPtr -> getMemCtrlReg( MC_REG_REQ_BLOCK_INDEX ));
    }
}

//***********************************************************************************************************
//***********************************************************************************************************
//
// Methods for the text window class.
//
//***********************************************************************************************************
//***********************************************************************************************************

//------------------------------------------------------------------------------------------------------------
// Object constructor. We are passed the globals and the file path. All we do right now is to remember the
// file name. The text window has a destructor method as well. We need to close a potentially opened file.
//
//------------------------------------------------------------------------------------------------------------
SimWinText::SimWinText( VCPU32Globals *glb, char *fName ) : SimWinScrollable( glb ) {
    
    if ( fName != nullptr ) strcpy( fileName, fName );
    else throw ( ERR_EXPECTED_FILE_NAME );
}

SimWinText:: ~SimWinText( ) {
    
    if ( textFile != nullptr ) {
        
        fclose( textFile );
    }
}

//------------------------------------------------------------------------------------------------------------
// The default values are the initial settings when windows is brought up the first time, or for the WDEF
// command.
//
//------------------------------------------------------------------------------------------------------------
void SimWinText::setDefaults( ) {
    
    setWinType( WT_TEXT_WIN );
    setEnable( true );
    setRows( 11 );
    
    int txWidth = glb -> env -> getEnvVarInt((char *) ENV_WIN_TEXT_LINE_WIDTH );
    setRadix( txWidth );
    setDefColumns( txWidth );
    
    setRadix( 10 );
    setCurrentItemAdr( 0 );
    setLineIncrement( 1 );
    setLimitItemAdr( 1 );
}

//------------------------------------------------------------------------------------------------------------
// The banner line for the text window. It contains the open file name and the current line and home line
// number. The file path may be a bit long for listing it completely, so we will truncate it on the left
// side. The routine will print the the filename, and the position into the file. The banner method also sets
// the preliminary line size of the window. This value is used until we know the actual number of lines in
// the file. Lines shown on the display start with one, internally we start at zero.
//
//------------------------------------------------------------------------------------------------------------
void SimWinText::drawBanner( ) {
    
    uint32_t    fmtDesc     = FMT_BOLD | FMT_INVERSE;
    bool        isCurrent   = glb -> winDisplay -> isCurrentWin( getWinIndex( ));
    
    setWinCursor( 1, 1 );
    printWindowIdField( getWinStack( ), getWinIndex( ), isCurrent, fmtDesc );
    printTextField((char *) "Text: ", ( fmtDesc | FMT_ALIGN_LFT ));
    printTextField((char *) fileName, ( fmtDesc | FMT_ALIGN_LFT | FMT_TRUNC_LFT ), 48 );
    printTextField((char *) "  Line: " );
    printNumericField( getCurrentItemAdr( ) + 1, ( fmtDesc | FMT_HALF_WORD ));
    printTextField((char *) "  Home: " );
    printNumericField(  getHomeItemAdr( ) + 1, ( fmtDesc | FMT_HALF_WORD ));
    padLine( fmtDesc );
}

//------------------------------------------------------------------------------------------------------------
// The draw line method for the text file window. We print the file content line by line. A line consists of
// the line number followed by the text. This routine will first check whether the file is already open.
// If we cannot open the file, we would now print an error message into the screen. This is also the time
// where we actually figure out how many lines are on the file so that we can set the limitItemAdr field
// of the window object.
//
//------------------------------------------------------------------------------------------------------------
void SimWinText::drawLine( uint32_t index ) {
    
    uint32_t    fmtDesc = FMT_DEF_ATTR;
    char        lineBuf[ MAX_TEXT_LINE_SIZE ];
    int         lineSize = 0;
  
    if ( openTextFile( )) {
  
        lineSize = readTextFileLine( index + 1, lineBuf, sizeof( lineBuf ));
        if ( lineSize > 0 ) {
            
            printNumericField( index + 1, ( fmtDesc | FMT_HALF_WORD ));
            printTextField((char *) ": " );
            printTextField( lineBuf, fmtDesc, lineSize );
            padLine( );
        }
        else padLine( );
    }
    else printTextField((char *) "Error opening the text file", fmtDesc );
}

//------------------------------------------------------------------------------------------------------------
// "openTextFile" is called every time we want to print a line. If the file is not opened yet, it will be
// now and while we are at it, we will also count the source lines for setting the limit in the scrollable
// window. All will be remembered of course.
//
//------------------------------------------------------------------------------------------------------------
bool SimWinText::openTextFile( ) {
    
    if ( textFile == nullptr ) {
        
        textFile = fopen( fileName, "r" );
        if ( textFile != nullptr ) {
           
            while( ! feof( textFile )) {
                
                if( fgetc( textFile ) == '\n' ) fileSizeLines++;
            }
            
            lastLinePos = 0;
            rewind( textFile );
            setLimitItemAdr( fileSizeLines );
        }
    }
    
    return( textFile != nullptr );
}

//------------------------------------------------------------------------------------------------------------
// "readTextFileLine" will get a line from the text file. Unfortunately, we do not have a line concept in a
// text file. In the worst case, we read from the beginning of the file, counting the lines read. To speed
// up a little, we remember the last line position read. If the requested line position is larger than the
// last position, we just read ahead. If it is smaller, no luck, we start to read from zero until we match.
// If equal, we just re-read the current line.
//
//------------------------------------------------------------------------------------------------------------
int SimWinText::readTextFileLine( int linePos, char *lineBuf, int bufLen  ) {
 
    if ( textFile != nullptr ) {
        
        if ( linePos > lastLinePos ) {
            
            while ( lastLinePos < linePos ) {
                
                lastLinePos ++;
                fgets( lineBuf, bufLen, textFile );
            }
        }
        else if ( linePos < lastLinePos ) {
            
            lastLinePos = 0;
            rewind( textFile );
            
            while ( lastLinePos < linePos ) {
                
                lastLinePos ++;
                fgets( lineBuf, bufLen, textFile );
            }
        }
        else fgets( lineBuf, bufLen, textFile );
            
        lineBuf[ strcspn( lineBuf, "\r\n") ] = 0;
        return((int) strlen ( lineBuf ));
    }
    else return ( 0 );
}

//***********************************************************************************************************
//***********************************************************************************************************
//
// Methods for the console window class.
//
//***********************************************************************************************************
//***********************************************************************************************************

//------------------------------------------------------------------------------------------------------------
// Object constructor / Destructor.
//
//------------------------------------------------------------------------------------------------------------
SimWinConsole::SimWinConsole( VCPU32Globals *glb ) : SimWin( glb ) {
    
}

SimWinConsole:: ~SimWinConsole( ) {
    
}

//------------------------------------------------------------------------------------------------------------
// The default values are the initial settings when windows is brought up the first time, or for the WDEF
// command.
//
//------------------------------------------------------------------------------------------------------------
void SimWinConsole::setDefaults( ) {
    
    setRadix( glb -> env -> getEnvVarInt((char *) ENV_RDX_DEFAULT ));
    setRows( 11 );
    setColumns( 80 );
    setDefColumns( 80 );
    setWinType( WT_CONSOLE_WIN );
    setEnable( true );
}

//------------------------------------------------------------------------------------------------------------
// The banner line for command window.
//
//------------------------------------------------------------------------------------------------------------
void SimWinConsole::drawBanner( ) {
    
    uint32_t fmtDesc = FMT_BOLD | FMT_INVERSE;
    
    setWinCursor( 1, 1 );
    printTextField((char *) "Commands ", fmtDesc );
    padLine( fmtDesc );
}

//------------------------------------------------------------------------------------------------------------
// The body lines of the command window are displayed after the banner line. We will never draw in this
// window via the window routines. The body is the terminal scroll area. What we do however, is to reset
// any character drawing attribute.
//
//------------------------------------------------------------------------------------------------------------
void SimWinConsole::drawBody( ) {
    
    setFieldAtributes( FMT_DEF_ATTR );
}


