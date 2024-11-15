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
#include "VCPU32-Types.h"
#include "VCPU32-Driver.h"
#include "VCPU32-Core.h"

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
// ??? wouldn't it be nice to have also user defined variables ?
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
    { .name = "ECHO-CMD-INPUT",
        .envId = ENV_ECHO_CMD,          .envTyp = ENV_TYP_BOOL, .rOnly = false,  .bVal = false     },
    { .name = "EXIT-CODE",
        .envId = ENV_EXIT_CODE,         .envTyp = ENV_TYP_INT,  .rOnly = false,  .iVal = 0         },
    { .name = "W-PER-LINE",
        .envId = ENV_WORDS_PER_LINE,    .envTyp = ENV_TYP_INT,  .rOnly = false,  .iVal = 8         },
    { .name = "VERSION",
        .envId = ENV_PROG_VERSION,      .envTyp = ENV_TYP_STR,  .rOnly = false,  .sVal = nullptr   },
    { .name = "GIT-BRANCH",
        .envId = ENV_GIT_BRANCH,        .envTyp = ENV_TYP_STR,  .rOnly = false,  .sVal = nullptr   },
    { .name = "PATCH_LEVEL",
        .envId = ENV_PROG_PATCH_LEVEL,  .envTyp = ENV_TYP_INT,  .rOnly = false,  .iVal = 0         },
    { .name = "STEP-IN-CLOCKS",
        .envId = ENV_STEP_IN_CLOCKS,    .envTyp = ENV_TYP_BOOL, .rOnly = false,  .bVal = false     },
    { .name = "SHOW-PSTAGE-INFO",
        .envId = ENV_SHOW_PSTAGE_INFO,  .envTyp = ENV_TYP_BOOL, .rOnly = false,  .bVal = false     },
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


//------------------------------------------------------------------------------------------------------------
// ??? if we have predefined and user defined ENV variables, this module needs to be rewritten.
// ??? shall we just use these variables by name  and not by token Id ?
//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------
// There predefined and user defined variables. Predefined variables are created at program start and
// initialized. They are marked predefined and optional readonly by the ENV command. Also, their type cannot
// be changed by a new value of a different type.
//
// User defined variables can be changed in type and value. They are by definition read and write enabled
// and can also be removed.
//------------------------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------------------------
// Environment table entry, Each environment variable has a name, a couple of flags and the value, which
// depends on the value type. Most types record their value directly in the entry. A string value will be
// dynamically allocated.
//
//------------------------------------------------------------------------------------------------------------
struct EnvTabEntry_n {
    
    char    name[ 32 ]  = { 0 };
    bool    valid       = false;
    bool    predefined  = false;
    bool    readOnly    = false;
    TypeId  typ         = TYP_NIL;
    
    union {
        
        struct {    TokId       tok;    };
        struct {    bool        bVal;   };
        struct {    uint32_t    uVal;   };
        struct {    int32_t     iVal;   };
        struct {    char        *str;   };
        
        struct {    uint16_t seg;   uint32_t ofs;   };
    };
};

//------------------------------------------------------------------------------------------------------------
// Environment table. The simulator has a global table where all variables are kept. It is a simple array
// with a high water mark concept. The table will be allocated at simulator start.
//
//------------------------------------------------------------------------------------------------------------
struct EnvTab_n {
    
    EnvTab_n( uint32_t size );
    
    uint8_t         displayEnvTable( );
    uint8_t         displayEnvTableEntry( char *name );
    uint8_t         setupPredefined( );
    
    uint8_t         setEnvVar( char *name, int iVal );
    uint8_t         setEnvVar( char *name, uint32_t uVal );
    uint8_t         setEnvVar( char *name, bool bVal );
    uint8_t         setEnvVar( char *name, uint32_t seg, uint32_t ofs );
    uint8_t         setEnvVar( char *name, char *str );
    uint8_t         setEnvVar( char *name, TokId tokVal );
    
    uint8_t         getEnvVarTokId( char *name, TokId *arg );
    uint8_t         getEnvVarBool( char *name, bool *arg );
    uint8_t         getEnvVarInt( char *name, int *arg );
    uint8_t         getEnvVarUint( char *name, uint32_t *arg );
    uint8_t         getEnvVarAdr( char *name, uint32_t *arg );
    uint8_t         getEnvVarExtAdr( char *name, uint32_t *seg, uint32_t *ofs );
    
    uint8_t         removeEnvVar( char *name );
    
    private:
    
    int             lookupEntry( char *name );
    int             findFreeEntry( );
    
    uint8_t         enterEnvVar( char *name, int32_t iVal, bool predefined = false, bool readOnly = false );
    uint8_t         enterEnvVar( char *name, uint32_t uVal, bool predefined = false, bool readOnly = false );
    uint8_t         enterEnvVar( char *name, char *str, bool predefined = false, bool readOnly = false );
    uint8_t         enterEnvVar( char *name, uint32_t seg, uint32_t ofs, bool predefined = false, bool readOnly = false );
    
    uint8_t         displayEnvTableEntry( EnvTabEntry_n *entry );
    
    EnvTabEntry_n   *table;
    EnvTabEntry_n   *hwm;
    EnvTabEntry_n   *limit;
};

//------------------------------------------------------------------------------------------------------------
// The ENV variable object. The table is dynamically allocated, the HWM and limit pointer are used to manage
// the search and entry add and remove functions.
//
//------------------------------------------------------------------------------------------------------------
EnvTab_n::EnvTab_n( uint32_t size ) {
   
    table   = (EnvTabEntry_n *) calloc( size, sizeof( EnvTabEntry_n ));
    hwm     = table;
    limit   = &table[ size ];
}

//------------------------------------------------------------------------------------------------------------
// "setEnvVar" is a set of function signatures that modify an ENV variable value. If the variable is a
// predefined variable, the readOnly option is checked as well as that the variable type matches. A user
// defined variable is by definition read/write enabled and the type changes based on the type of the
// value set. If the variable is not found, a new variable will be allocated. One more thing. If the ENV
// variable type is string and we set a value, the old string is deallocated.
//
//------------------------------------------------------------------------------------------------------------
uint8_t EnvTab_n::setEnvVar( char *name, int iVal ) {
    
    int index = lookupEntry( name );
    
    if ( index >= 0 ) {
        
        EnvTabEntry_n *ptr = &table[ index ];
        
        if (( ptr -> predefined ) && ( ptr -> typ != TYP_NUM )) return ( 99 );
        if (( ptr -> predefined ) && ( ptr -> readOnly ))       return ( 99 );
        if ( ptr -> typ == TYP_STR )                            free( ptr -> str );
         
        ptr -> typ  = TYP_NUM;
        ptr -> iVal = iVal;
        return( NO_ERR );
    }
    else return enterEnvVar( name, iVal );
}

uint8_t EnvTab_n::setEnvVar( char *name, uint32_t uVal ) {
    
    int index = lookupEntry( name );
    
    if ( index >= 0 ) {
        
        EnvTabEntry_n *ptr = &table[ index ];
        
        if (( ptr -> predefined ) && ( ptr -> typ != TYP_NUM )) return ( 99 );
        if (( ptr -> predefined ) && ( ptr -> readOnly ))       return ( 99 );
        if ( ptr -> typ == TYP_STR )                            free( ptr -> str );
         
        ptr -> typ  = TYP_NUM;
        ptr -> uVal = uVal;
        return( NO_ERR );
    }
    else return enterEnvVar( name, uVal );
}

uint8_t EnvTab_n::setEnvVar( char *name, bool bVal )  {
    
    int index = lookupEntry( name );
    
    if ( index >= 0 ) {
        
        EnvTabEntry_n *ptr = &table[ index ];
        
        if (( ptr -> predefined ) && ( ptr -> typ != TYP_NUM )) return ( 99 );
        if (( ptr -> predefined ) && ( ptr -> readOnly ))       return ( 99 );
        if ( ptr -> typ == TYP_STR )                            free( ptr -> str );
         
        ptr -> typ  = TYP_BOOL;
        ptr -> bVal = bVal;
        return( NO_ERR );
    }
    else return enterEnvVar( name, bVal );
}

uint8_t EnvTab_n::setEnvVar( char *name, uint32_t seg, uint32_t ofs )  {
    
    int index = lookupEntry( name );
    
    if ( index >= 0 ) {
        
        EnvTabEntry_n *ptr = &table[ index ];
        
        if (( ptr -> predefined ) && ( ptr -> typ != TYP_NUM )) return ( 99 );
        if (( ptr -> predefined ) && ( ptr -> readOnly ))       return ( 99 );
        if ( ptr -> typ == TYP_STR )                            free( ptr -> str );
         
        ptr -> typ  = TYP_EXT_ADR;
        ptr -> seg = seg;
        ptr -> ofs = ofs;
        return( NO_ERR );
    }
    else return enterEnvVar( name, seg, ofs );
}

uint8_t EnvTab_n::setEnvVar( char *name, char *str )  {
    
    int index = lookupEntry( name );
    
    if ( index >= 0 ) {
        
        EnvTabEntry_n *ptr = &table[ index ];
        
        if (( ptr -> predefined ) && ( ptr -> typ != TYP_NUM )) return ( 99 );
        if (( ptr -> predefined ) && ( ptr -> readOnly ))       return ( 99 );
        if ( ptr -> typ == TYP_STR )                            free( ptr -> str );
         
        ptr -> typ  = TYP_STR;
        strcpy( ptr -> str, str );
        return( NO_ERR );
    }
    else return enterEnvVar( name, str );
}

uint8_t EnvTab_n::setEnvVar( char *name, TokId tokVal )  {
    
    return( NO_ERR );
}

//------------------------------------------------------------------------------------------------------------
// Remove a user defined ENV variable. If the ENV variable is predefined it is an error. If the ENV variable
// type is a string, free the string space. The entry is marked invalid, i.e. free. Finally, if the entry
// was at the high water mark, adjust the HWM.
//
//------------------------------------------------------------------------------------------------------------
uint8_t EnvTab_n::removeEnvVar( char *name ) {
    
    int index = lookupEntry( name );
    
    if ( index >= 0 ) {
        
        EnvTabEntry_n *ptr = &table[ index ];
        
        if ( ptr -> predefined ) return( 99 );
        if ( ptr -> typ == TYP_STR ) free( ptr -> str );
        
        ptr -> valid    = false;
        ptr -> typ      = TYP_NIL;
        
        if ( ptr == hwm - 1 ) {
            
            while (( ptr >= table ) && ( ptr -> valid )) hwm --;
        }
        
        return( NO_ERR );
    }
    else return ( 99 );
}

//------------------------------------------------------------------------------------------------------------
// A set of helper function to enter a variable. The variable can be a predefined or a user defined one. If
// it is a predefined variable, the readonly flag marks the variable read only for the ENV command.
//
//------------------------------------------------------------------------------------------------------------
uint8_t EnvTab_n::enterEnvVar( char *name, int32_t iVal, bool predefined, bool readOnly ) {
    
    int index = findFreeEntry( );
    
    if ( index <= 0 ) {
    
        EnvTabEntry_n tmp;
        strcpy ( tmp.name, name );
        tmp.typ         = TYP_NUM;
        tmp.valid       = true;
        tmp.predefined  = predefined;
        tmp.readOnly    = readOnly;
        tmp.iVal        = iVal;
        table[ index ]  = tmp;
        return( NO_ERR );
    }
    else return( 99 );
}

uint8_t EnvTab_n::enterEnvVar( char *name, uint32_t uVal, bool predefined, bool readOnly ) {
    
    int index = findFreeEntry( );
    
    if ( index <= 0 ) {
    
        EnvTabEntry_n tmp;
        strcpy ( tmp.name, name );
        tmp.typ         = TYP_NUM;
        tmp.valid       = true;
        tmp.predefined  = predefined;
        tmp.readOnly    = readOnly;
        tmp.uVal        = uVal;
        table[ index ]  = tmp;
        return( NO_ERR );
    }
    else return( 99 );
}

uint8_t EnvTab_n::enterEnvVar( char *name, char *str, bool predefined, bool readOnly ) {
    
    int index = findFreeEntry( );
    
    if ( index <= 0 ) {
        
        EnvTabEntry_n tmp;
        strcpy ( tmp.name, name );
        tmp.valid       = true;
        tmp.typ         = TYP_STR;
        tmp.predefined  = predefined;
        tmp.readOnly    = readOnly;
        
        char *tmpStr = (char *) calloc( strlen( str ), sizeof( char ));
        strcpy( tmpStr, str );
        
        table[ index ]  = tmp;
        return( NO_ERR );
    }
    else return( 99 );
}

uint8_t EnvTab_n::enterEnvVar( char *name, uint32_t seg, uint32_t ofs, bool predefined, bool readOnly ) {
    
    int index = findFreeEntry( );
    
    if ( index <= 0 ) {
    
        EnvTabEntry_n tmp;
        strcpy ( tmp.name, name );
        tmp.typ         = TYP_EXT_ADR;
        tmp.valid       = true;
        tmp.predefined  = predefined;
        tmp.readOnly    = readOnly;
        tmp.seg         = seg;
        tmp.ofs         = ofs;
        table[ index ]  = tmp;
        return( NO_ERR );
    }
    else return( 99 );
}

//------------------------------------------------------------------------------------------------------------
// Look a variable. We just do a linear search up to the HWM. SIf not found a -1 is returned. Straightforward.
//
//------------------------------------------------------------------------------------------------------------
int EnvTab_n::lookupEntry( char *name ) {
    
    EnvTabEntry_n *entry = table;
    
    while ( entry < hwm ) {
        
        if (( entry -> valid ) && ( strcmp( entry -> name, name ) == 0 )) return((int) ( entry - table ));
        else entry ++;
    }
    
    return( -1 );
}

//------------------------------------------------------------------------------------------------------------
// Find a free slot for a variable. First we look for a free entry in the range up to the HWM. If there is
// none, we try to increase the HWM. If all fails, the table is full.
//
//------------------------------------------------------------------------------------------------------------
int EnvTab_n::findFreeEntry( ) {
    
    EnvTabEntry_n *entry = table;
    
    while ( entry < hwm ) {
        
        if ( ! entry -> valid ) return((int) ( entry - table ));
        else entry ++;
    }
    
    if ( hwm < limit ) {
        
        hwm ++;
        return((int) ( entry - table ));
    }
    else return( 99 );
}

//------------------------------------------------------------------------------------------------------------
// List the entire ENV table up to the high water mark.
//
//------------------------------------------------------------------------------------------------------------
uint8_t EnvTab_n::displayEnvTable( ) {
    
    EnvTabEntry_n *entry = table;
    
    while ( entry < hwm ) {
        
        if ( entry -> valid ) displayEnvTableEntry( entry );
    }
    
    return( NO_ERR );
}

//------------------------------------------------------------------------------------------------------------
// Display a ENV entry by name.
//
//------------------------------------------------------------------------------------------------------------
uint8_t EnvTab_n::displayEnvTableEntry( char *name ) {
    
    int index = lookupEntry( name );
    
    if ( index >= 0 )   return( displayEnvTableEntry( &table[ index ] ));
    else                return( 99 );
}

//------------------------------------------------------------------------------------------------------------
// Display the ENV entry.
//
// ??? what to do about token types ? ( FMT, etc. ) ( perhaps time to make the fmtID a numeric value ... :-( )
//------------------------------------------------------------------------------------------------------------
uint8_t EnvTab_n::displayEnvTableEntry( EnvTabEntry_n *entry ) {
    
    printf( "%32s", entry -> name );
    
    switch ( entry -> typ ) {
            
        case TYP_NUM:       fprintf( stdout, "%i", entry -> iVal ); break;
        case TYP_EXT_ADR:   fprintf( stdout, "0x%04x.0x%08x", entry -> seg, entry -> ofs );
            
        case TYP_STR: {
            
            fprintf( stdout, "%s", entry -> str );
            
        } break;
            
        case TYP_BOOL:      {
            
            if ( entry -> bVal ) fprintf( stdout, "TRUE");
            else                 fprintf( stdout, "FALSE"); break;
        
        } break;
        
        default: printf( "Unknown type" );
    }
 
    return( NO_ERR );
}
