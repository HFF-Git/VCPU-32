//------------------------------------------------------------------------------------------------------------
//
// VCPU32 - A 32-bit CPU - Simulator Environment Variables
//
//------------------------------------------------------------------------------------------------------------
// The test driver environment has a set of environment variables. They are simple name = value pairs for
// integers, booleans and strings.
//
//------------------------------------------------------------------------------------------------------------
//
// VCPU32 - A 32-bit CPU - Simulator Environment Variables
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
#include "VCPU32-Driver.hpp"
#include "VCPU32-Core.hpp"

//------------------------------------------------------------------------------------------------------------
// Local name space. We try to keep utility functions local to the file.
//
//------------------------------------------------------------------------------------------------------------
namespace {

//------------------------------------------------------------------------------------------------------------
// Environment variables. The simulator has a set of variables that control and display options for the
// simulator to set. Each variable has an assigned token name by which it can be referred to. The environment
// variable table is filled with reasonable defaults. A script executed at program start could of course set
// the variables to other values.
//
//------------------------------------------------------------------------------------------------------------
const int MAX_ENV_STR_SIZE = 256;

struct EnvTabEntry {
    
    char  name[ MAX_ENV_STR_SIZE ];
    TokId envId;
    TokId envTyp;
    bool  rOnly;
    
    union {
        
        TokId       tVal;
        int32_t     iVal;
        uint32_t    uVal;
        bool        bVal;
        char        *sVal;
    };
    
} envTab[ ] = {
    
    { .name = "FMT-DEF",
        .envId = ENV_FMT_DEF, .envTyp = ENV_TYP_TOK,  .rOnly = false,  .tVal = TOK_HEX   },
    { .name = "SHOW-CMD-CNT",
        .envId = ENV_SHOW_CMD_CNT,      .envTyp = ENV_TYP_BOOL, .rOnly = false,  .bVal = true      },
    { .name = "CMD-CNT",
        .envId = ENV_CMD_CNT,           .envTyp = ENV_TYP_INT,  .rOnly = true,   .iVal = 0         },
    { .name = "EXIT-CODE",
        .envId = ENV_EXIT_CODE,         .envTyp = ENV_TYP_INT,  .rOnly = false,  .iVal = 0         },
    { .name = "W-PER-LINE",
        .envId = ENV_WORDS_PER_LINE,    .envTyp = ENV_TYP_INT,  .rOnly = false,  .iVal = 8         },
    { .name = "VERSION",
        .envId = ENV_PROG_VERSION,      .envTyp = ENV_TYP_STR,  .rOnly = false,  .sVal = nullptr   },
    { .name = "PATCH_LEVEL",
        .envId = ENV_PROG_PATCH_LEVEL,  .envTyp = ENV_TYP_INT,  .rOnly = false,  .iVal = 0         },
    { .name = "STEP-IN-CLOCKS",
        .envId = ENV_STEP_IN_CLOCKS,    .envTyp = ENV_TYP_BOOL, .rOnly = false,  .bVal = false     },
    { .name = "PASS_CNT",
        .envId = ENV_PASS_CNT,          .envTyp = ENV_TYP_INT,  .rOnly = false,  .iVal = 0         },
    { .name = "FAIL_CNT",
        .envId = ENV_FAIL_CNT,          .envTyp = ENV_TYP_INT,  .rOnly = false,  .iVal = 0         },
    { .name = "I-TLB-SETS",
        .envId = ENV_I_TLB_SETS,        .envTyp = ENV_TYP_INT,  .rOnly = false,  .iVal = 1         },
    { .name = "I-TLB-SIZE",
        .envId = ENV_I_TLB_SIZE,        .envTyp = ENV_TYP_INT,  .rOnly = false,  .iVal = 1024      },
    { .name = "D-TLB-SETS",
        .envId = ENV_D_TLB_SETS,        .envTyp = ENV_TYP_INT,  .rOnly = false,  .iVal = 1         },
    { .name = "D-TLB-SIZE",
        .envId = ENV_D_TLB_SIZE,        .envTyp = ENV_TYP_INT,  .rOnly = false,  .iVal = 1024      },
    { .name = "I-CACHE-SETS",
        .envId = ENV_I_CACHE_SETS,      .envTyp = ENV_TYP_INT,  .rOnly = false,  .iVal = 1         },
    { .name = "I-CACHE-SIZE",
        .envId = ENV_I_CACHE_SIZE,      .envTyp = ENV_TYP_INT,  .rOnly = false,  .iVal = 1024      },
    { .name = "I-CACHE-LINE-SIZE",
        .envId = ENV_I_CACHE_SIZE,      .envTyp = ENV_TYP_INT,  .rOnly = false,  .iVal = 4         },
    { .name = "D-CACHE-SETS",
        .envId = ENV_D_CACHE_SETS,      .envTyp = ENV_TYP_INT,  .rOnly = false,  .iVal = 1         },
    { .name = "D-CACHE-SIZE",
        .envId = ENV_D_CACHE_SIZE,      .envTyp = ENV_TYP_INT,  .rOnly = false,  .iVal = 1024      },
    { .name = "D-CACHE-LINE-SIZE",
        .envId = ENV_D_CACHE_SIZE,      .envTyp = ENV_TYP_INT,  .rOnly = false,  .iVal = 4         },
    { .name = "MEM-SIZE",
        .envId = ENV_MEM_SIZE,          .envTyp = ENV_TYP_UINT, .rOnly = false,  .uVal = MAX_MEMORY_SIZE },
    { .name = "MEM-BANKS",
        .envId = ENV_MEM_BANKS,         .envTyp = ENV_TYP_INT,  .rOnly = false,  .iVal = 1           },
    { .name = "MEM-BANK-SIZE",
        .envId = ENV_MEM_BANK_SIZE,     .envTyp = ENV_TYP_UINT, .rOnly = false,  .uVal = MAX_MEMORY_SIZE },
    { .name = "WIN-MIN-ROWS",
        .envId = ENV_WIN_MIN_ROWS,      .envTyp = ENV_TYP_INT,  .rOnly = false,  .iVal = 24          },
    { .name = "WIN-TEXT-WIDTH",
        .envId = ENV_WIN_TX_WIDTH,      .envTyp = ENV_TYP_INT,  .rOnly = false,  .iVal = 90          }
};

const int ENV_TAB_SIZE  = sizeof( envTab ) / sizeof( *envTab );


//------------------------------------------------------------------------------------------------------------
// A local function to display an environment variable.
//
//------------------------------------------------------------------------------------------------------------
void displayEnvTabEntry( EnvTabEntry *entry ) {
    
    fprintf( stdout, "%-32s: ", entry -> name );
    
    if      ( entry -> envTyp == ENV_TYP_TOK )  fprintf( stdout, "%s\n", DrvCmds::tokIdToName( entry -> tVal ));
    else if ( entry -> envTyp == ENV_TYP_INT )  fprintf( stdout, "%d\n", entry -> iVal );
    else if ( entry -> envTyp == ENV_TYP_UINT ) fprintf( stdout, "%u\n", entry -> uVal );
    else if ( entry -> envTyp == ENV_TYP_BOOL ) fprintf( stdout, "%s\n", ( entry -> bVal ? "true" : "false" ));
    else if ( entry -> envTyp == ENV_TYP_STR ) {
        
        if ( entry -> sVal != nullptr ) fprintf( stdout, "%s\n", entry -> sVal );
        else fprintf( stdout, "null\n" );
    }
    else fprintf( stdout, "****\n" );
}

}; // namespace


//************************************************************************************************************
// Object methods.
//************************************************************************************************************


//------------------------------------------------------------------------------------------------------------
// Object creator.
//
//------------------------------------------------------------------------------------------------------------
DrvEnv::DrvEnv( VCPU32Globals *glb ) {
    
    this -> glb = glb;
}

//------------------------------------------------------------------------------------------------------------
// ENV functions to get the type, attribute, and to get and set value.
//
//------------------------------------------------------------------------------------------------------------
int DrvEnv::getEnvTabSize( ) {
    
    return ( ENV_TAB_SIZE );
}

TokId DrvEnv::lookupEnvTokId( char *str, TokId def ) {
    
    size_t len = strlen( str );
    
    if (( len == 0 ) || ( len > MAX_ENV_STR_SIZE )) return( def );
    
    char tmpStr[ MAX_ENV_STR_SIZE ];
    strncpy( tmpStr, str, len + 1 );
    
    for ( size_t i = 0; i < len; i++ ) tmpStr[ i ] = (char) toupper((int) str[ i ] );
    
    for ( int i = 0; i < ENV_TAB_SIZE; i++ ) {
        
        if ( strcmp( tmpStr, envTab[ i ].name ) == 0 ) return( envTab[ i ].envId );
    }
    
    return( def );
}

TokId DrvEnv::getEnvType( TokId envId ) {
    
    for ( int i = 0; i < ENV_TAB_SIZE; i++ ) {
        
        if ( envTab[ i ].envId == envId ) return( envTab[ i ].envTyp );
    }
    
    return( TOK_NIL );
}

bool DrvEnv::isReadOnly( TokId envId ) {
    
    for ( int i = 0; i < ENV_TAB_SIZE; i++ ) {
        
        if ( envTab[ i ].envId == envId ) return( envTab[ i ].rOnly );
    }
    
    return( true );
}

TokId DrvEnv::getEnvValTok( TokId envId ) {
    
    for ( int i = 0; i < ENV_TAB_SIZE; i++ ) {
        
        if (( envTab[ i ].envId == envId ) && ( envTab[ i ].envTyp == ENV_TYP_TOK ))
            return( envTab[ i ].tVal );
    }
    
    return( TOK_NIL );
}

int32_t DrvEnv::getEnvValInt( TokId envId ) {
    
    for ( int i = 0; i < ENV_TAB_SIZE; i++ ) {
        
        if (( envTab[ i ].envId == envId ) && ( envTab[ i ].envTyp == ENV_TYP_INT ))
            return( envTab[ i ].iVal );
    }
    
    return( 0 );
}

uint32_t DrvEnv::getEnvValUInt( TokId envId ) {
    
    for ( int i = 0; i < ENV_TAB_SIZE; i++ ) {
        
        if (( envTab[ i ].envId == envId ) && ( envTab[ i ].envTyp == ENV_TYP_INT ))
            return( envTab[ i ].uVal );
    }
    
    return( 0 );
}

bool DrvEnv::getEnvValBool( TokId envId ) {
    
    for ( int i = 0; i < ENV_TAB_SIZE; i++ ) {
        
        if (( envTab[ i ].envId == envId ) && ( envTab[ i ].envTyp == ENV_TYP_BOOL ))
            return( envTab[ i ].bVal );
    }
    
    return( false );
}

char *DrvEnv::getEnvValStr( TokId envId ) {
    
    for ( int i = 0; i < ENV_TAB_SIZE; i++ ) {
        
        if (( envTab[ i ].envId == envId ) && ( envTab[ i ].envTyp == ENV_TYP_STR ))
            return( envTab[ i ].sVal );
    }
    
    return((char *) "" );
}

void DrvEnv::setEnvVal( TokId envId, TokId val ) {
    
    for ( int i = 0; i < ENV_TAB_SIZE; i++ ) {
        
        if (( envTab[ i ].envId == envId ) && ( envTab[ i ].envTyp == ENV_TYP_TOK )) {
            
            envTab[ i ].tVal  = val;
        }
    }
}

void DrvEnv::setEnvVal( TokId envId, int32_t val ) {
    
    for ( int i = 0; i < ENV_TAB_SIZE; i++ ) {
        
        if (( envTab[ i ].envId == envId ) && ( envTab[ i ].envTyp == ENV_TYP_INT ))
            envTab[ i ].iVal = val;
    }
}

void DrvEnv::setEnvVal( TokId envId, uint32_t val ) {
    
    for ( int i = 0; i < ENV_TAB_SIZE; i++ ) {
        
        if (( envTab[ i ].envId == envId ) && ( envTab[ i ].envTyp == ENV_TYP_UINT ))
            envTab[ i ].uVal = val;
    }
}

void DrvEnv::setEnvVal( TokId envId, bool val ) {
    
    for ( int i = 0; i < ENV_TAB_SIZE; i++ ) {
        
        if (( envTab[ i ].envId == envId ) && ( envTab[ i ].envTyp == ENV_TYP_BOOL ))
            envTab[ i ].bVal = val;
    }
}

void DrvEnv::setEnvVal( TokId envId, char *str ) {
    
    for ( int i = 0; i < ENV_TAB_SIZE; i++ ) {
        
        if (( envTab[ i ].envId == envId ) && ( envTab[ i ].envTyp == ENV_TYP_STR )) {
            
            if ( envTab[ i ].sVal != nullptr ) free((char *) envTab[ i ].sVal );
            
            size_t len = strlen( str );
            if ( len > MAX_ENV_STR_SIZE ) len = MAX_ENV_STR_SIZE;
            
            char *tmpStr = (char *) calloc( len + 1, 1 );
            strcpy((char *) tmpStr, str );
            
            envTab[ i ].sVal = tmpStr;
        }
    }
}

//------------------------------------------------------------------------------------------------------------
// Finally, we want to list the ENV table as well as an individual entry.
//
//------------------------------------------------------------------------------------------------------------
uint8_t DrvEnv::displayEnvTable( ) {
    
    for ( int i = 0; i < ENV_TAB_SIZE; i++ ) ::displayEnvTabEntry( &envTab[ i ] );
    return( 0 );
}

uint8_t DrvEnv::displayEnvTabEntry( TokId envId ) {
    
    for ( int i = 0; i < ENV_TAB_SIZE; i++ ) {
        
        if ( envTab[ i ].envId == envId  ) {
            
            ::displayEnvTabEntry( &envTab[ i ] );
            return( 1 );
        }
    }
    
    return( 0 );
}
