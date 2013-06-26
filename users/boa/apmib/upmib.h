#ifndef INCLUDE_UPMIB_H
#define INCLUDE_UPMIB_H
#include "apmib.h"
#include "mibtbl.h"

/*
 * The 'id' in 'struct upmib' is the same as the defination in apmib.h.
 *eg:it should be 'MIB_MIB_VER' according to "#define MIB_MIB_VER 655 " in apmib.h
 *
 * The 'name' in 'struct upmib' is the same as the defination in mibdef.h
 *eg:it should be 'MIB_VER' according to "MIBDEF(unsigned char,   mibVer, , MIB_VER,    BYTE_T, APMIB_T, 5, 0)" 
 *defined in mibdef.h
 *
 * If you want to update MIB current setting value,you must change MIB VERSION in 'UPMIB_T update_mib[]' first.
 * Add the new MIB in 'UPMIB_T new_mib'. 
 * Add the MIB that you want to update its value in 'UPMIB_T update_mib[]'.
 *
 */


 
typedef struct upmib{
        int id;				
        unsigned char name[32]; 	
	unsigned char value[128];	//the mib value
}UPMIB_T, *UPMIB_Tp;


 
UPMIB_T update_mib[] = {
//add mib info here
        {MIB_MIB_VER,           "MIB_VER",              "1"},
        {0}
};
 
UPMIB_T new_mib[] = {
//add mib info here
        {0}
};


#endif
