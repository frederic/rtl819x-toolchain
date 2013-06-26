/****************************************************************************** 
* 
* Copyright(c) 2007 - 2011 Realtek Corporation. All rights reserved. 
* 
* This program is free software; you can redistribute it and/or modify it 
* under the terms of version 2 of the GNU General Public License as 
* published by the Free Software Foundation. 
* 
* This program is distributed in the hope that it will be useful, but WITHOUT 
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for 
* more details. 
* 
* You should have received a copy of the GNU General Public License along with 
* this program; if not, write to the Free Software Foundation, Inc., 
* 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA 
* 
* 
******************************************************************************/

#ifndef __INC_HAL8192CE_MAC_IMG_H
#define __INC_HAL8192CE_MAC_IMG_H

/*Created on  2011/10/ 5,  8: 2*/

/*
#if (DM_ODM_SUPPORT_TYPE == ODM_MP)
	#include <Mp_Precomp.h>
#elif (DM_ODM_SUPPORT_TYPE == ODM_CE)
	#include "../odm_types.h"
#endif
*/
//#include "../Mp_Precomp.h"


#if (RTL8192CE_HWIMG_SUPPORT == 1)

#define Rtl8192CEMAC_2T_ArrayLength 172
extern const u4Byte Rtl8192CEMAC_2T_Array[Rtl8192CEMAC_2T_ArrayLength];
#define Rtl8192CEMACPHY_Array_PGLength 1
extern const u4Byte Rtl8192CEMACPHY_Array_PG[Rtl8192CEMACPHY_Array_PGLength];
#define Rtl8192CEAGCTAB_2TArrayLength 320
extern const u4Byte Rtl8192CEAGCTAB_2TArray[Rtl8192CEAGCTAB_2TArrayLength];
#define Rtl8192CEAGCTAB_1TArrayLength 320
extern const u4Byte Rtl8192CEAGCTAB_1TArray[Rtl8192CEAGCTAB_1TArrayLength];

#else

#define Rtl8192CEMAC_2T_ArrayLength 1
extern const u1Byte Rtl8192CEFwMAC_2T_Array[Rtl8192CEMAC_2T_ArrayLength];
#define Rtl8192CEMACPHY_Array_PGLength 1
extern const u1Byte Rtl8192CEFwMACPHY_Array_PG[Rtl8192CEMACPHY_Array_PGLength];
#define Rtl8192CEAGCTAB_2TArrayLength 1
extern const u1Byte Rtl8192CEFwAGCTAB_2TArray[Rtl8192CEAGCTAB_2TArrayLength];
#define Rtl8192CEAGCTAB_1TArrayLength 1
extern const u4Byte Rtl8192CEAGCTAB_1TArray[Rtl8192CEAGCTAB_1TArrayLength];

#endif //#if (RTL8192CE_HWIMG_SUPPORT == 1)

#endif //__INC_HAL8192CE_FW_IMG_H
