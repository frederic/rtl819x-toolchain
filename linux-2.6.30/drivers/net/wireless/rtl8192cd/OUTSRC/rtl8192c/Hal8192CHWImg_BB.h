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
#ifndef __INC_BB_8192C_HW_IMG_H
#define __INC_BB_8192C_HW_IMG_H

#define RETURNED_ARRAY_SIZE 20000
static BOOLEAN CheckCondition(const u4Byte Condition, const u4Byte Hex);
static u2Byte odm_SetArrayPointerGetLength_8192C(PDM_ODM_T pDM_Odm, u4Byte ArrayLen, pu4Byte Array, pu4Byte *gArrayPointer);

/******************************************************************************
*                           AGC_TAB.TXT
******************************************************************************/

pu4Byte
ODM_GetArrayPointer_AGC_TAB_8192C(
     IN   PDM_ODM_T    pDM_Odm
);
u2Byte
ODM_GetArrayLength_AGC_TAB_8192C(
     IN   PDM_ODM_T    pDM_Odm
);
extern  u4Byte  Array_AGC_TAB_8192C[];

extern  pu4Byte gArrayPointer_AGC_TAB_8192C;

void
ODM_ReadAndConfig_AGC_TAB_8192C(
	IN   PDM_ODM_T  pDM_Odm
);

/******************************************************************************
*                           AGC_TAB_1T.TXT
******************************************************************************/

pu4Byte
ODM_GetArrayPointer_AGC_TAB_1T_8192C(
     IN   PDM_ODM_T    pDM_Odm
);
u2Byte
ODM_GetArrayLength_AGC_TAB_1T_8192C(
     IN   PDM_ODM_T    pDM_Odm
);
extern  u4Byte  Array_AGC_TAB_1T_8192C[];

extern  pu4Byte gArrayPointer_AGC_TAB_1T_8192C;

void
ODM_ReadAndConfig_AGC_TAB_1T_8192C(
	IN   PDM_ODM_T  pDM_Odm
);

/******************************************************************************
*                           AGC_TAB_2G.TXT
******************************************************************************/

pu4Byte
ODM_GetArrayPointer_AGC_TAB_2G_8192C(
     IN   PDM_ODM_T    pDM_Odm
);
u2Byte
ODM_GetArrayLength_AGC_TAB_2G_8192C(
     IN   PDM_ODM_T    pDM_Odm
);
extern  u4Byte  Array_AGC_TAB_2G_8192C[];

extern  pu4Byte gArrayPointer_AGC_TAB_2G_8192C;

void
ODM_ReadAndConfig_AGC_TAB_2G_8192C(
	IN   PDM_ODM_T  pDM_Odm
);

/******************************************************************************
*                           AGC_TAB_2T.TXT
******************************************************************************/

pu4Byte
ODM_GetArrayPointer_AGC_TAB_2T_8192C(
     IN   PDM_ODM_T    pDM_Odm
);
u2Byte
ODM_GetArrayLength_AGC_TAB_2T_8192C(
     IN   PDM_ODM_T    pDM_Odm
);
extern  u4Byte  Array_AGC_TAB_2T_8192C[];

extern  pu4Byte gArrayPointer_AGC_TAB_2T_8192C;

void
ODM_ReadAndConfig_AGC_TAB_2T_8192C(
	IN   PDM_ODM_T  pDM_Odm
);

/******************************************************************************
*                           AGC_TAB_5G.TXT
******************************************************************************/

pu4Byte
ODM_GetArrayPointer_AGC_TAB_5G_8192C(
     IN   PDM_ODM_T    pDM_Odm
);
u2Byte
ODM_GetArrayLength_AGC_TAB_5G_8192C(
     IN   PDM_ODM_T    pDM_Odm
);
extern  u4Byte  Array_AGC_TAB_5G_8192C[];

extern  pu4Byte gArrayPointer_AGC_TAB_5G_8192C;

void
ODM_ReadAndConfig_AGC_TAB_5G_8192C(
	IN   PDM_ODM_T  pDM_Odm
);

/******************************************************************************
*                           PHY_REG.TXT
******************************************************************************/

pu4Byte
ODM_GetArrayPointer_PHY_REG_8192C(
     IN   PDM_ODM_T    pDM_Odm
);
u2Byte
ODM_GetArrayLength_PHY_REG_8192C(
     IN   PDM_ODM_T    pDM_Odm
);
extern  u4Byte  Array_PHY_REG_8192C[];

extern  pu4Byte gArrayPointer_PHY_REG_8192C;

void
ODM_ReadAndConfig_PHY_REG_8192C(
	IN   PDM_ODM_T  pDM_Odm
);

/******************************************************************************
*                           PHY_REG_1T.TXT
******************************************************************************/

pu4Byte
ODM_GetArrayPointer_PHY_REG_1T_8192C(
     IN   PDM_ODM_T    pDM_Odm
);
u2Byte
ODM_GetArrayLength_PHY_REG_1T_8192C(
     IN   PDM_ODM_T    pDM_Odm
);
extern  u4Byte  Array_PHY_REG_1T_8192C[];

extern  pu4Byte gArrayPointer_PHY_REG_1T_8192C;

void
ODM_ReadAndConfig_PHY_REG_1T_8192C(
	IN   PDM_ODM_T  pDM_Odm
);

/******************************************************************************
*                           PHY_REG_2T.TXT
******************************************************************************/

pu4Byte
ODM_GetArrayPointer_PHY_REG_2T_8192C(
     IN   PDM_ODM_T    pDM_Odm
);
u2Byte
ODM_GetArrayLength_PHY_REG_2T_8192C(
     IN   PDM_ODM_T    pDM_Odm
);
extern  u4Byte  Array_PHY_REG_2T_8192C[];

extern  pu4Byte gArrayPointer_PHY_REG_2T_8192C;

void
ODM_ReadAndConfig_PHY_REG_2T_8192C(
	IN   PDM_ODM_T  pDM_Odm
);

#endif
#endif // end of HWIMG_SUPPORT