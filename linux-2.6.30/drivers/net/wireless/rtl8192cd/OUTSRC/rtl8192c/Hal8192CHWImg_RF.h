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
#ifndef __INC_RF_8192C_HW_IMG_H
#define __INC_RF_8192C_HW_IMG_H

#define RETURNED_ARRAY_SIZE 20000
static BOOLEAN CheckCondition(const u4Byte Condition, const u4Byte Hex);
static u2Byte odm_SetArrayPointerGetLength_8192C(PDM_ODM_T pDM_Odm, u4Byte ArrayLen, pu4Byte Array, pu4Byte *gArrayPointer);

/******************************************************************************
*                           RadioA.TXT
******************************************************************************/

pu4Byte
ODM_GetArrayPointer_RadioA_8192C(
     IN   PDM_ODM_T    pDM_Odm
);
u2Byte
ODM_GetArrayLength_RadioA_8192C(
     IN   PDM_ODM_T    pDM_Odm
);
extern  u4Byte  Array_RadioA_8192C[];

extern  pu4Byte gArrayPointer_RadioA_8192C;

void
ODM_ReadAndConfig_RadioA_8192C(
	IN   PDM_ODM_T  pDM_Odm
);

/******************************************************************************
*                           RadioA_1T.TXT
******************************************************************************/

pu4Byte
ODM_GetArrayPointer_RadioA_1T_8192C(
     IN   PDM_ODM_T    pDM_Odm
);
u2Byte
ODM_GetArrayLength_RadioA_1T_8192C(
     IN   PDM_ODM_T    pDM_Odm
);
extern  u4Byte  Array_RadioA_1T_8192C[];

extern  pu4Byte gArrayPointer_RadioA_1T_8192C;

void
ODM_ReadAndConfig_RadioA_1T_8192C(
	IN   PDM_ODM_T  pDM_Odm
);

/******************************************************************************
*                           RadioA_2T.TXT
******************************************************************************/

pu4Byte
ODM_GetArrayPointer_RadioA_2T_8192C(
     IN   PDM_ODM_T    pDM_Odm
);
u2Byte
ODM_GetArrayLength_RadioA_2T_8192C(
     IN   PDM_ODM_T    pDM_Odm
);
extern  u4Byte  Array_RadioA_2T_8192C[];

extern  pu4Byte gArrayPointer_RadioA_2T_8192C;

void
ODM_ReadAndConfig_RadioA_2T_8192C(
	IN   PDM_ODM_T  pDM_Odm
);

/******************************************************************************
*                           RadioB.TXT
******************************************************************************/

pu4Byte
ODM_GetArrayPointer_RadioB_8192C(
     IN   PDM_ODM_T    pDM_Odm
);
u2Byte
ODM_GetArrayLength_RadioB_8192C(
     IN   PDM_ODM_T    pDM_Odm
);
extern  u4Byte  Array_RadioB_8192C[];

extern  pu4Byte gArrayPointer_RadioB_8192C;

void
ODM_ReadAndConfig_RadioB_8192C(
	IN   PDM_ODM_T  pDM_Odm
);

/******************************************************************************
*                           RadioB_1T.TXT
******************************************************************************/

pu4Byte
ODM_GetArrayPointer_RadioB_1T_8192C(
     IN   PDM_ODM_T    pDM_Odm
);
u2Byte
ODM_GetArrayLength_RadioB_1T_8192C(
     IN   PDM_ODM_T    pDM_Odm
);
extern  u4Byte  Array_RadioB_1T_8192C[];

extern  pu4Byte gArrayPointer_RadioB_1T_8192C;

void
ODM_ReadAndConfig_RadioB_1T_8192C(
	IN   PDM_ODM_T  pDM_Odm
);

/******************************************************************************
*                           RadioB_2T.TXT
******************************************************************************/

pu4Byte
ODM_GetArrayPointer_RadioB_2T_8192C(
     IN   PDM_ODM_T    pDM_Odm
);
u2Byte
ODM_GetArrayLength_RadioB_2T_8192C(
     IN   PDM_ODM_T    pDM_Odm
);
extern  u4Byte  Array_RadioB_2T_8192C[];

extern  pu4Byte gArrayPointer_RadioB_2T_8192C;

void
ODM_ReadAndConfig_RadioB_2T_8192C(
	IN   PDM_ODM_T  pDM_Odm
);

#endif
#endif // end of HWIMG_SUPPORT