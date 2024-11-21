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
#include "VCPU32-Version.h"
#include "VCPU32-Types.h"
#include "VCPU32-Driver.h"
#include "VCPU32-DrvTables.h"
#include "VCPU32-Core.h"

//------------------------------------------------------------------------------------------------------------
// Local name space. We try to keep utility functions local to the file.
//
//------------------------------------------------------------------------------------------------------------
namespace {

//------------------------------------------------------------------------------------------------------------
// A little helper function to upshift a string in place.
//
//------------------------------------------------------------------------------------------------------------
void upshiftStr( char *str ) {
    
    size_t len = strlen( str );
    
    if ( len > 0 ) {
        
        for ( size_t i = 0; i < len; i++ ) str[ i ] = (char) toupper((int) str[ i ] );
    }
}

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
DrvEnv::DrvEnv( VCPU32Globals *glb, uint32_t size ) {
   
    table       = (DrvEnvTabEntry *) calloc( size, sizeof( DrvEnvTabEntry ));
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
uint8_t DrvEnv::setEnvVar( char *name, int iVal ) {
    
    int index = lookupEntry( name );
    
    if ( index >= 0 ) {
        
        DrvEnvTabEntry *ptr = &table[ index ];
        
        if (( ptr -> predefined ) && ( ptr -> typ != TYP_NUM )) return ( 99 );
        if ( ptr -> typ == TYP_STR )                            free( ptr -> str );
         
        ptr -> typ  = TYP_NUM;
        ptr -> iVal = iVal;
        return( NO_ERR );
    }
    else return enterEnvVar( name, ((int) iVal ));
}

uint8_t DrvEnv::setEnvVar( char *name, uint32_t uVal ) {
    
    int index = lookupEntry( name );
    
    if ( index >= 0 ) {
        
        DrvEnvTabEntry *ptr = &table[ index ];
        
        if (( ptr -> predefined ) && ( ptr -> typ != TYP_NUM )) return ( 99 );
        if ( ptr -> typ == TYP_STR )                            free( ptr -> str );
         
        ptr -> typ  = TYP_NUM;
        ptr -> uVal = uVal;
        return( NO_ERR );
    }
    else return enterEnvVar( name, ((uint32_t) uVal ));
}

uint8_t DrvEnv::setEnvVar( char *name, bool bVal )  {
    
    int index = lookupEntry( name );
    
    if ( index >= 0 ) {
        
        DrvEnvTabEntry *ptr = &table[ index ];
        
        if (( ptr -> predefined ) && ( ptr -> typ != TYP_NUM )) return ( 99 );
        if ( ptr -> typ == TYP_STR )                            free( ptr -> str );
         
        ptr -> typ  = TYP_BOOL;
        ptr -> bVal = bVal;
        return( NO_ERR );
    }
    else return enterEnvVar( name, ((bool) bVal ));
}

uint8_t DrvEnv::setEnvVar( char *name, uint32_t seg, uint32_t ofs )  {
   
    int index = lookupEntry( name );
    
    if ( index >= 0 ) {
        
        DrvEnvTabEntry *ptr = &table[ index ];
        
        if (( ptr -> predefined ) && ( ptr -> typ != TYP_NUM )) return ( 99 );
        if ( ptr -> typ == TYP_STR )                            free( ptr -> str );
         
        ptr -> typ  = TYP_EXT_ADR;
        ptr -> seg = seg;
        ptr -> ofs = ofs;
        return( NO_ERR );
    }
    else return enterEnvVar( name, ((uint32_t) seg), ((uint32_t) ofs ));
}

uint8_t DrvEnv::setEnvVar( char *name, char *str )  {
   
    int index = lookupEntry( name );
    
    if ( index >= 0 ) {
        
        DrvEnvTabEntry *ptr = &table[ index ];
        
        if (( ptr -> predefined ) && ( ptr -> typ != TYP_STR )) return ( 99 );
        if (( ptr -> typ == TYP_STR ) && ( ptr -> str != nullptr )) free( ptr -> str );
        
        ptr -> typ  = TYP_STR;
        ptr -> str = (char *) malloc( strlen( str ) + 1 );
        strcpy( ptr -> str, str );
        return( NO_ERR );
    }
    else return enterEnvVar( name, str );
}

//------------------------------------------------------------------------------------------------------------
// Environemnt vriables getter functions. Just look up th entry and return the value. If the entry does not
// exist, we return an optional default.
//
//------------------------------------------------------------------------------------------------------------
bool DrvEnv::getEnvVarBool( char *name, bool def ) {
    
    int index = lookupEntry( name );
    
    if ( index >= 0 )   return( table[ index ].bVal );
    else                return ( def );
}

int  DrvEnv::getEnvVarInt( char *name, int def ) {
    
    int index = lookupEntry( name );
    
    if ( index >= 0 )   return( table[ index ].iVal );
    else                return ( def );
}

uint32_t  DrvEnv::getEnvVarUint( char *name, uint32_t def ) {
    
    int index = lookupEntry( name );
    
    if ( index >= 0 )   return( table[ index ].uVal );
    else                return ( def );
}

uint32_t  DrvEnv::getEnvVarExtAdrSeg( char *name, uint32_t def ) {
    
    int index = lookupEntry( name );
    
    if ( index >= 0 )   return( table[ index ].seg );
    else                return ( def );
    
}

uint32_t  DrvEnv::getEnvVarExtAdrOfs( char *name, uint32_t def ) {
    
    int index = lookupEntry( name );
    
    if ( index >= 0 )   return( table[ index ].ofs );
    else                return ( def );
    
}

char *DrvEnv::getEnvVarStr( char *name, char *def ) {
    
    int index = lookupEntry( name );
    
    if ( index >= 0 )   return( table[ index ].str );
    else                return (def );
}

//------------------------------------------------------------------------------------------------------------
// Remove a user defined ENV variable. If the ENV variable is predefined it is an error. If the ENV variable
// type is a string, free the string space. The entry is marked invalid, i.e. free. Finally, if the entry
// was at the high water mark, adjust the HWM.
//
//------------------------------------------------------------------------------------------------------------
uint8_t DrvEnv::removeEnvVar( char *name ) {
    
    int index = lookupEntry( name );
    
    if ( index >= 0 ) {
        
        DrvEnvTabEntry *ptr = &table[ index ];
        
        if ( ptr -> predefined ) return( 99 );
        if (( ptr -> typ == TYP_STR ) && ( ptr -> str != nullptr )) free( ptr -> str );
        
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
uint8_t DrvEnv::enterEnvVar( char *name, int32_t iVal, bool predefined, bool readOnly ) {
    
    int index = findFreeEntry( );
    
    if ( index >= 0 ) {
    
        DrvEnvTabEntry tmp;
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

uint8_t DrvEnv::enterEnvVar( char *name, uint32_t uVal, bool predefined, bool readOnly ) {
    
    int index = findFreeEntry( );
    
    if ( index >= 0 ) {
    
        DrvEnvTabEntry tmp;
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

uint8_t DrvEnv::enterEnvVar( char *name, bool bVal, bool predefined, bool readOnly ) {
    
    int index = findFreeEntry( );
    
    if ( index >= 0 ) {
    
        DrvEnvTabEntry tmp;
        strcpy ( tmp.name, name );
        tmp.typ         = TYP_BOOL;
        tmp.valid       = true;
        tmp.predefined  = predefined;
        tmp.readOnly    = readOnly;
        tmp.bVal        = bVal;
        table[ index ]  = tmp;
        return( NO_ERR );
    }
    else return( 99 );
}

uint8_t DrvEnv::enterEnvVar( char *name, char *str, bool predefined, bool readOnly ) {
    
    int index = findFreeEntry( );
    
    if ( index >= 0 ) {
        
        DrvEnvTabEntry tmp;
        strcpy ( tmp.name, name );
        tmp.valid       = true;
        tmp.typ         = TYP_STR;
        tmp.predefined  = predefined;
        tmp.readOnly    = readOnly;
        
        tmp.str         = (char *) calloc( strlen( str ), sizeof( char ));
        strcpy( tmp.str, str );
        
        table[ index ]  = tmp;
        return( NO_ERR );
    }
    else return( 99 );
}

uint8_t DrvEnv::enterEnvVar( char *name, uint32_t seg, uint32_t ofs, bool predefined, bool readOnly ) {
    
    int index = findFreeEntry( );
    
    if ( index >= 0 ) {
    
        DrvEnvTabEntry tmp;
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
// Utility functions to return variable attributes.
//
//------------------------------------------------------------------------------------------------------------
bool DrvEnv::isValid( char *name ) {
    
    int index = lookupEntry( name );
    
    if ( index >= 0 ) return( table[ index ].valid );
    else return( false );
}

bool DrvEnv::isReadOnly( char *name ) {
    
    int index = lookupEntry( name );
    
    if ( index >= 0 ) return( table[ index ].readOnly );
    else return( false );
}

bool DrvEnv::isPredefined( char *name ) {
    
    int index = lookupEntry( name );
    
    if ( index >= 0 ) return( table[ index ].predefined );
    else return( false );
}

DrvEnvTabEntry  *DrvEnv::getEnvVarEntry( char *name ) {
    
    int index = lookupEntry( name );
    if ( index >= 0 ) return( &table[ index ] );
    else return( nullptr );
}

//------------------------------------------------------------------------------------------------------------
// Look a variable. We just do a linear search up to the HWM. SIf not found a -1 is returned. Straightforward.
//
//------------------------------------------------------------------------------------------------------------
int DrvEnv::lookupEntry( char *name ) {
    
    DrvEnvTabEntry *entry = table;
    
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
int DrvEnv::findFreeEntry( ) {
    
    DrvEnvTabEntry *entry = table;
    
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
uint8_t DrvEnv::displayEnvTable( ) {
    
    DrvEnvTabEntry *entry = table;
    
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
uint8_t DrvEnv::displayEnvTableEntry( char *name ) {
    
    int index = lookupEntry( name );
    
    if ( index >= 0 )   return( displayEnvTableEntry( &table[ index ] ));
    else                return( 99 );
}

//------------------------------------------------------------------------------------------------------------
// Display the ENV entry.
//
// ??? what about the uVal variables.... TYP_NUM is not correct ....
//------------------------------------------------------------------------------------------------------------
uint8_t DrvEnv::displayEnvTableEntry( DrvEnvTabEntry *entry ) {
    
    fprintf( stdout, "%-32s", entry -> name );
    
    switch ( entry -> typ ) {
            
        case TYP_NUM:       fprintf( stdout, "NUM:     %i", entry -> iVal ); break;
        case TYP_EXT_ADR:   fprintf( stdout, "EXT_ADR: 0x%04x.0x%08x", entry -> seg, entry -> ofs );
            
        case TYP_STR: {
            
            // ??? any length checks ?
            fprintf( stdout, "STR:     \"%s\"", entry -> str );
            
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
// Enter the predefind entries.
//
//------------------------------------------------------------------------------------------------------------
uint8_t DrvEnv::setupPredefined( ) {
    
    uint8_t rStat = NO_ERR;
    
    if ( rStat == NO_ERR ) enterEnvVar((char *)  ENV_PROG_VERSION, (char *) VERSION, true, false );
    if ( rStat == NO_ERR ) enterEnvVar((char *)  ENV_GIT_BRANCH, (char *) GIT_BRANCH, true, false );
    if ( rStat == NO_ERR ) enterEnvVar((char *)  ENV_PROG_PATCH_LEVEL, (int) PATCH_LEVEL, true, false );
    
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
    
    // ??? should TRUE and FALSE rather be a predefine ?
    // ??? this would only make sense when teh expression evaluator looks for identifiers here too ....
    
    return( rStat );
}
