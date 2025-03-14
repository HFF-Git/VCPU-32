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
#include "VCPU32-SimVersion.h"
#include "VCPU32-Types.h"
#include "VCPU32-SimDeclarations.h"
#include "VCPU32-SimTables.h"
#include "VCPU32-Core.h"

//------------------------------------------------------------------------------------------------------------
// Local name space. We try to keep utility functions local to the file.  None so far.
//
//------------------------------------------------------------------------------------------------------------
namespace {

}; // namespace


//************************************************************************************************************
//************************************************************************************************************
//
// Object methods.
//
//************************************************************************************************************
//************************************************************************************************************

//------------------------------------------------------------------------------------------------------------
// There predefined and user defined variables. Predefined variables are created at program start and
// initialized. They are marked predefined and optional readonly by the ENV command. Also, their type cannot
// be changed by a new value of a different type.
//
// User defined variables can be changed in type and value. They are by definition read and write enabled
// and can also be removed.
//------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------
// The ENV variable object. The table is dynamically allocated, the HWM and limit pointer are used to manage
// the search and entry add and remove functions.
//
//------------------------------------------------------------------------------------------------------------
SimEnv::SimEnv( VCPU32Globals *glb, uint32_t size ) {
   
    table       = (SimEnvTabEntry *) calloc( size, sizeof( SimEnvTabEntry ));
    hwm         = table;
    limit       = &table[ size ];
    this -> glb = glb;
}

//------------------------------------------------------------------------------------------------------------
// "setEnvVar" is a set of function signatures that modify an ENV variable value. If the variable is a
// predefined variable, the readOnly option is checked as well as that the variable type matches. A user
// defined variable is by definition read/write enabled and the type changes based on the type of the
// value set. If the variable is not found, a new variable will be allocated. One more thing. If the ENV
// variable type is string and we set a value, the old string is deallocated.
//
//------------------------------------------------------------------------------------------------------------
void SimEnv::setEnvVar( char *name, int iVal ) {
    
    int index = lookupEntry( name );
    
    if ( index >= 0 ) {
        
        SimEnvTabEntry *ptr = &table[ index ];
        
        if (( ptr -> predefined ) && ( ptr -> typ != TYP_NUM )) throw ( ERR_ENV_VALUE_EXPR );
        if (( ptr -> typ == TYP_STR ) && ( ptr -> strVal != nullptr )) free( ptr -> strVal );
         
        ptr -> typ    = TYP_NUM;
        ptr -> iVal   = iVal;
    }
    else enterEnvVar( name, ((int) iVal ));
}

void SimEnv::setEnvVar( char *name, uint32_t uVal ) {
    
    int index = lookupEntry( name );
    
    if ( index >= 0 ) {
        
        SimEnvTabEntry *ptr = &table[ index ];
        
        if (( ptr -> predefined ) && ( ptr -> typ != TYP_NUM )) throw ( ERR_ENV_VALUE_EXPR );
        if (( ptr -> typ == TYP_STR ) && ( ptr -> strVal != nullptr )) free( ptr -> strVal );
         
        ptr -> typ  = TYP_NUM;
        ptr -> uVal = uVal;
    }
    else enterEnvVar( name, ((uint32_t) uVal ));
}

void SimEnv::setEnvVar( char *name, bool bVal )  {
    
    int index = lookupEntry( name );
    
    if ( index >= 0 ) {
        
        SimEnvTabEntry *ptr = &table[ index ];
        
        if (( ptr -> predefined ) && ( ptr -> typ != TYP_NUM )) throw ( ERR_ENV_VALUE_EXPR );
        if (( ptr -> typ == TYP_STR ) && ( ptr -> strVal != nullptr )) free( ptr -> strVal );
         
        ptr -> typ  = TYP_BOOL;
        ptr -> bVal = bVal;
    }
    else enterEnvVar( name, ((bool) bVal ));
}

void SimEnv::setEnvVar( char *name, uint32_t seg, uint32_t ofs )  {
   
    int index = lookupEntry( name );
    
    if ( index >= 0 ) {
        
        SimEnvTabEntry *ptr = &table[ index ];
        
        if (( ptr -> predefined ) && ( ptr -> typ != TYP_NUM )) throw ( ERR_ENV_VALUE_EXPR );
        if (( ptr -> typ == TYP_STR ) && ( ptr -> strVal != nullptr )) free( ptr -> strVal );
         
        ptr -> typ  = TYP_EXT_ADR;
        ptr -> seg  = seg;
        ptr -> ofs  = ofs;
    }
    else enterEnvVar( name, ((uint32_t) seg), ((uint32_t) ofs ));
}

void SimEnv::setEnvVar( char *name, char *str )  {
   
    int index = lookupEntry( name );
    
    if ( index >= 0 ) {
        
        SimEnvTabEntry *ptr = &table[ index ];
        
        if (( ptr -> predefined ) && ( ptr -> typ != TYP_STR )) throw ( ERR_ENV_VALUE_EXPR );
        if (( ptr -> typ == TYP_STR ) && ( ptr -> strVal != nullptr )) free( ptr -> strVal );
        
        ptr -> typ = TYP_STR;
        ptr -> strVal = (char *) malloc( strlen( str ) + 1 );
        strcpy( ptr -> strVal, str );
    }
    else enterEnvVar( name, str );
}

//------------------------------------------------------------------------------------------------------------
// Environment variables getter functions. Just look up the entry and return the value. If the entry does not
// exist, we return an optional default.
//
//------------------------------------------------------------------------------------------------------------
bool SimEnv::getEnvVarBool( char *name, bool def ) {
    
    int index = lookupEntry( name );
    
    if ( index >= 0 )   return( table[ index ].bVal );
    else                return ( def );
}

int  SimEnv::getEnvVarInt( char *name, int def ) {
    
    int index = lookupEntry( name );
    
    if ( index >= 0 )   return( table[ index ].iVal );
    else                return ( def );
}

uint32_t  SimEnv::getEnvVarUint( char *name, uint32_t def ) {
    
    int index = lookupEntry( name );
    
    if ( index >= 0 )   return( table[ index ].uVal );
    else                return ( def );
}

uint32_t  SimEnv::getEnvVarExtAdrSeg( char *name, uint32_t def ) {
    
    int index = lookupEntry( name );
    
    if ( index >= 0 )   return( table[ index ].seg );
    else                return ( def );
}

uint32_t  SimEnv::getEnvVarExtAdrOfs( char *name, uint32_t def ) {
    
    int index = lookupEntry( name );
    
    if ( index >= 0 )   return( table[ index ].ofs );
    else                return ( def );
}

char *SimEnv::getEnvVarStr( char *name, char *def ) {
    
    int index = lookupEntry( name );
    
    if ( index >= 0 )   return( table[ index ].strVal );
    else                return (def );
}

//------------------------------------------------------------------------------------------------------------
// Remove a user defined ENV variable. If the ENV variable is predefined it is an error. If the ENV variable
// type is a string, free the string space. The entry is marked invalid, i.e. free. Finally, if the entry
// was at the high water mark, adjust the HWM.
//
//------------------------------------------------------------------------------------------------------------
void SimEnv::removeEnvVar( char *name ) {
    
    int index = lookupEntry( name );
    
    if ( index >= 0 ) {
        
        SimEnvTabEntry *ptr = &table[ index ];
        
        if ( ptr -> predefined ) throw( ERR_ENV_PREDEFINED );
        if (( ptr -> typ == TYP_STR ) && ( ptr -> strVal != nullptr )) free( ptr -> strVal );
        
        ptr -> valid    = false;
        ptr -> typ      = TYP_NIL;
        
        if ( ptr == hwm - 1 ) {
            
            while (( ptr >= table ) && ( ptr -> valid )) hwm --;
        }
    }
    else throw ( ERR_ENV_VAR_NOT_FOUND );
}

//------------------------------------------------------------------------------------------------------------
// A set of helper function to enter a variable. The variable can be a predefined or a user defined one. If
// it is a predefined variable, the readonly flag marks the variable read only for the ENV command.
//
//------------------------------------------------------------------------------------------------------------
void SimEnv::enterEnvVar( char *name, int32_t iVal, bool predefined, bool rOnly ) {
    
    int index = findFreeEntry( );
    
    if ( index >= 0 ) {
    
        SimEnvTabEntry tmp;
        strcpy ( tmp.name, name );
        tmp.typ         = TYP_NUM;
        tmp.valid       = true;
        tmp.predefined  = predefined;
        tmp.readOnly    = rOnly;
        tmp.iVal        = iVal;
        table[ index ]  = tmp;
    }
    else throw( ERR_ENV_TABLE_FULL );
}

void SimEnv::enterEnvVar( char *name, uint32_t uVal, bool predefined, bool rOnly ) {
    
    int index = findFreeEntry( );
    
    if ( index >= 0 ) {
    
        SimEnvTabEntry tmp;
        strcpy ( tmp.name, name );
        tmp.typ         = TYP_NUM;
        tmp.valid       = true;
        tmp.predefined  = predefined;
        tmp.readOnly    = rOnly;
        tmp.uVal        = uVal;
        table[ index ]  = tmp;
    }
    else throw( ERR_ENV_TABLE_FULL );
}

void SimEnv::enterEnvVar( char *name, bool bVal, bool predefined, bool rOnly ) {
    
    int index = findFreeEntry( );
    
    if ( index >= 0 ) {
    
        SimEnvTabEntry tmp;
        strcpy ( tmp.name, name );
        tmp.typ         = TYP_BOOL;
        tmp.valid       = true;
        tmp.predefined  = predefined;
        tmp.readOnly    = rOnly;
        tmp.bVal        = bVal;
        table[ index ]  = tmp;
    }
    else throw( ERR_ENV_TABLE_FULL );
}

void SimEnv::enterEnvVar( char *name, char *str, bool predefined, bool rOnly ) {
    
    int index = findFreeEntry( );
    
    if ( index >= 0 ) {
        
        SimEnvTabEntry tmp;
        strcpy ( tmp.name, name );
        tmp.valid       = true;
        tmp.typ         = TYP_STR;
        tmp.predefined  = predefined;
        tmp.readOnly    = rOnly;
        tmp.strVal      = (char *) calloc( strlen( str ), sizeof( char ));
        strcpy( tmp.strVal, str );
        table[ index ]  = tmp;
    }
    else throw( ERR_ENV_TABLE_FULL );
}

void SimEnv::enterEnvVar( char *name, uint32_t seg, uint32_t ofs, bool predefined, bool rOnly ) {
    
    int index = findFreeEntry( );
    
    if ( index >= 0 ) {
    
        SimEnvTabEntry tmp;
        strcpy ( tmp.name, name );
        tmp.typ         = TYP_EXT_ADR;
        tmp.valid       = true;
        tmp.predefined  = predefined;
        tmp.readOnly    = rOnly;
        tmp.seg         = seg;
        tmp.ofs         = ofs;
        table[ index ]  = tmp;
    }
    else throw( ERR_ENV_TABLE_FULL );
}

//------------------------------------------------------------------------------------------------------------
// Utility functions to return variable attributes.
//
//------------------------------------------------------------------------------------------------------------
bool SimEnv::isValid( char *name ) {
    
    int index = lookupEntry( name );
    
    if ( index >= 0 ) return( table[ index ].valid );
    else return( false );
}

bool SimEnv::isReadOnly( char *name ) {
    
    int index = lookupEntry( name );
    
    if ( index >= 0 ) return( table[ index ].readOnly );
    else return( false );
}

bool SimEnv::isPredefined( char *name ) {
    
    int index = lookupEntry( name );
    
    if ( index >= 0 ) return( table[ index ].predefined );
    else return( false );
}

SimEnvTabEntry *SimEnv::getEnvVarEntry( char *name ) {
    
    int index = lookupEntry( name );
    if ( index >= 0 ) return( &table[ index ] );
    else return( nullptr );
}

//------------------------------------------------------------------------------------------------------------
// Look a variable. We just do a linear search up to the HWM. If not found a -1 is returned. Straightforward.
//
//------------------------------------------------------------------------------------------------------------
int SimEnv::lookupEntry( char *name ) {
    
    SimEnvTabEntry *entry = table;
    
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
int SimEnv::findFreeEntry( ) {
    
    SimEnvTabEntry *entry = table;
    
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
uint8_t SimEnv::displayEnvTable( ) {
    
    SimEnvTabEntry *entry = table;
    
    while ( entry < hwm ) {
        
        if ( entry -> valid ) displayEnvTableEntry( entry );
        entry ++;
    }
    
    return( NO_ERR );
}

//------------------------------------------------------------------------------------------------------------
// Display a ENV entry by name.
//
//------------------------------------------------------------------------------------------------------------
uint8_t SimEnv::displayEnvTableEntry( char *name ) {
    
    int index = lookupEntry( name );
    
    if ( index >= 0 )   return( displayEnvTableEntry( &table[ index ] ));
    else                return( 99 );
}

//------------------------------------------------------------------------------------------------------------
// Display the ENV entry.
//
// ??? what about the uVal variables.... TYP_NUM is not correct ....
//------------------------------------------------------------------------------------------------------------
uint8_t SimEnv::displayEnvTableEntry( SimEnvTabEntry *entry ) {
    
    fprintf( stdout, "%-32s", entry -> name );
    
    switch ( entry -> typ ) {
            
        case TYP_NUM:       fprintf( stdout, "NUM:     %i", entry -> iVal ); break;
        case TYP_EXT_ADR:   fprintf( stdout, "EXT_ADR: 0x%04x.0x%08x", entry -> seg, entry -> ofs ); break;
            
        case TYP_STR: {
            
            // ??? any length checks ?
            fprintf( stdout, "STR:     \"%s\"", entry -> strVal );
            
        } break;
            
        case TYP_BOOL:      {
            
            if ( entry -> bVal ) fprintf( stdout, "BOOL:    TRUE");
            else                 fprintf( stdout, "BOOL:    FALSE"); break;
        
        } break;
        
        default: printf( "Unknown type" );
    }
    
    fprintf( stdout, "\n" );
    return( NO_ERR );
}


//------------------------------------------------------------------------------------------------------------
// Enter the predefined entries.
//
//------------------------------------------------------------------------------------------------------------
void SimEnv::setupPredefined( ) {
    
    uint8_t rStat = NO_ERR;
    
    if ( rStat == NO_ERR ) enterEnvVar((char *)  ENV_TRUE, (bool) true, true, true );
    if ( rStat == NO_ERR ) enterEnvVar((char *)  ENV_FALSE, (bool) false, true, true );
    
    if ( rStat == NO_ERR ) enterEnvVar((char *)  ENV_PROG_VERSION, (char *) SIM_VERSION, true, false );
    if ( rStat == NO_ERR ) enterEnvVar((char *)  ENV_GIT_BRANCH, (char *) SIM_GIT_BRANCH, true, false );
    if ( rStat == NO_ERR ) enterEnvVar((char *)  ENV_PATCH_LEVEL, (int) SIM_PATCH_LEVEL, true, false );
    
    if ( rStat == NO_ERR ) enterEnvVar((char *)  ENV_SHOW_CMD_CNT, true, true, false );
    if ( rStat == NO_ERR ) enterEnvVar((char *)  ENV_CMD_CNT, (int) 0, true, true );
    if ( rStat == NO_ERR ) enterEnvVar((char *)  ENV_ECHO_CMD_INPUT, false, true, false );
    if ( rStat == NO_ERR ) enterEnvVar((char *)  ENV_EXIT_CODE, (int) 0, true, false );
    
    if ( rStat == NO_ERR ) enterEnvVar((char *)  ENV_RDX_DEFAULT, (int) 16, true, false );
    if ( rStat == NO_ERR ) enterEnvVar((char *)  ENV_WORDS_PER_LINE, (int) 8, true, false );
    if ( rStat == NO_ERR ) enterEnvVar((char *)  ENV_SHOW_PSTAGE_INFO, false, true, false );
    if ( rStat == NO_ERR ) enterEnvVar((char *)  ENV_STEP_IN_CLOCKS, false, true, false );
    
    if ( rStat == NO_ERR ) enterEnvVar((char *)  ENV_I_TLB_SETS, (int) 1, true, false );
    if ( rStat == NO_ERR ) enterEnvVar((char *)  ENV_I_TLB_SIZE, (int) 1024, true, false );
    if ( rStat == NO_ERR ) enterEnvVar((char *)  ENV_D_TLB_SETS, (int) 1, true, false );
    if ( rStat == NO_ERR ) enterEnvVar((char *)  ENV_D_TLB_SIZE, (int) 1024, true, false );
    
    if ( rStat == NO_ERR ) enterEnvVar((char *)  ENV_I_CACHE_SETS, (int) 1, true, false );
    if ( rStat == NO_ERR ) enterEnvVar((char *)  ENV_I_CACHE_SIZE, (int) 1024, true, false );
    if ( rStat == NO_ERR ) enterEnvVar((char *)  ENV_I_CACHE_LINE_SIZE, (int) 4, true, false );
    
    if ( rStat == NO_ERR ) enterEnvVar((char *)  ENV_D_CACHE_SETS, (int) 1, true, false );
    if ( rStat == NO_ERR ) enterEnvVar((char *)  ENV_D_CACHE_SIZE, (int) 1024, true, false );
    if ( rStat == NO_ERR ) enterEnvVar((char *)  ENV_D_CACHE_LINE_SIZE, (int) 4, true, false );
    
    if ( rStat == NO_ERR ) enterEnvVar((char *)  ENV_MEM_SIZE, (uint32_t) MAX_MEMORY_SIZE, true, false );
    if ( rStat == NO_ERR ) enterEnvVar((char *)  ENV_MEM_BANKS, (int) 1, true, false );
    if ( rStat == NO_ERR ) enterEnvVar((char *)  ENV_MEM_BANK_SIZE, (uint32_t) MAX_MEMORY_SIZE, true, false );
    
    if ( rStat == NO_ERR ) enterEnvVar((char *)  ENV_WIN_MIN_ROWS, (int) 24, true, false );
    if ( rStat == NO_ERR ) enterEnvVar((char *)  ENV_WIN_TEXT_LINE_WIDTH, (int) 90, true, false );
}
