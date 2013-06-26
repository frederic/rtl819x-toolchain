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

#if (RTL8192C_HWIMG_SUPPORT == 1)
#ifndef __INC_FW_8192C_HW_IMG_H
#define __INC_FW_8192C_HW_IMG_H

#include "../odm_precomp.h"


/******************************************************************************
*                           rtl8192cfw.TXT
******************************************************************************/

pu4Byte
ODM_GetArrayPointer_rtl8192cfw_8192C(
     IN   PDM_ODM_T    pDM_Odm
);
u2Byte
ODM_GetArrayLength_rtl8192cfw_8192C(
     IN   PDM_ODM_T    pDM_Odm
);
extern  u4Byte  Array_rtl8192cfw_8192C[];

extern  pu4Byte gArrayPointer_rtl8192cfw_8192C;

void
ODM_ReadAndConfig_rtl8192cfw_8192C(
	IN   PDM_ODM_T  pDM_Odm
);

/******************************************************************************
*                           rtl8192cfwT.TXT
******************************************************************************/

pu4Byte
ODM_GetArrayPointer_rtl8192cfwT_8192C(
     IN   PDM_ODM_T    pDM_Odm
);
u2Byte
ODM_GetArrayLength_rtl8192cfwT_8192C(
     IN   PDM_ODM_T    pDM_Odm
);
extern  u4Byte  Array_rtl8192cfwT_8192C[];

extern  pu4Byte gArrayPointer_rtl8192cfwT_8192C;

void
ODM_ReadAndConfig_rtl8192cfwT_8192C(
	IN   PDM_ODM_T  pDM_Odm
);

/******************************************************************************
*                           rtl8192cfwT_.TXT
******************************************************************************/

pu4Byte
ODM_GetArrayPointer_rtl8192cfwT__8192C(
     IN   PDM_ODM_T    pDM_Odm
);
u2Byte
ODM_GetArrayLength_rtl8192cfwT__8192C(
     IN   PDM_ODM_T    pDM_Odm
);
extern  u4Byte  Array_rtl8192cfwT__8192C[];

extern  pu4Byte gArrayPointer_rtl8192cfwT__8192C;

void
ODM_ReadAndConfig_rtl8192cfwT__8192C(
	IN   PDM_ODM_T  pDM_Odm
);

/******************************************************************************
*                           rtl8192cfwU.TXT
******************************************************************************/

pu4Byte
ODM_GetArrayPointer_rtl8192cfwU_8192C(
     IN   PDM_ODM_T    pDM_Odm
);
u2Byte
ODM_GetArrayLength_rtl8192cfwU_8192C(
     IN   PDM_ODM_T    pDM_Odm
);
extern  u4Byte  Array_rtl8192cfwU_8192C[];

extern  pu4Byte gArrayPointer_rtl8192cfwU_8192C;

void
ODM_ReadAndConfig_rtl8192cfwU_8192C(
	IN   PDM_ODM_T  pDM_Odm
);

/******************************************************************************
*                           rtl8192cfwU_B.TXT
******************************************************************************/

pu4Byte
ODM_GetArrayPointer_rtl8192cfwU_B_8192C(
     IN   PDM_ODM_T    pDM_Odm
);
u2Byte
ODM_GetArrayLength_rtl8192cfwU_B_8192C(
     IN   PDM_ODM_T    pDM_Odm
);
extern  u4Byte  Array_rtl8192cfwU_B_8192C[];

extern  pu4Byte gArrayPointer_rtl8192cfwU_B_8192C;

void
ODM_ReadAndConfig_rtl8192cfwU_B_8192C(
	IN   PDM_ODM_T  pDM_Odm
);

/******************************************************************************
*                           rtl8192cfw_test.TXT
******************************************************************************/

pu4Byte
ODM_GetArrayPointer_rtl8192cfw_test_8192C(
     IN   PDM_ODM_T    pDM_Odm
);
u2Byte
ODM_GetArrayLength_rtl8192cfw_test_8192C(
     IN   PDM_ODM_T    pDM_Odm
);
extern  u4Byte  Array_rtl8192cfw_test_8192C[];

extern  pu4Byte gArrayPointer_rtl8192cfw_test_8192C;

void
ODM_ReadAndConfig_rtl8192cfw_test_8192C(
	IN   PDM_ODM_T  pDM_Odm
);

#endif
#endif // end of HWIMG_SUPPORT