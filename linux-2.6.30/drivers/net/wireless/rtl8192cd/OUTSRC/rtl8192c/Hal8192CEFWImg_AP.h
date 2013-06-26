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

#ifndef __INC_HAL8192CE_FW_IMG_H
#define __INC_HAL8192CE_FW_IMG_H


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

#define Rtl8192CETSMCImgArrayLength 11614
extern const u1Byte Rtl8192CEFwTSMCImgArray[Rtl8192CETSMCImgArrayLength];
#define Rtl8192CEUMCImgArrayLength 11746
extern const u1Byte Rtl8192CEFwUMCImgArray[Rtl8192CEUMCImgArrayLength];
#define Rtl8192CEUMCBCutImgArrayLength 11614
extern const u1Byte Rtl8192CEFwUMCBCutImgArray[Rtl8192CEUMCBCutImgArrayLength];

#else

#define Rtl8192CETSMCImgArrayLength 1
extern const u1Byte Rtl8192CEFwTSMCImgArray[Rtl8192CETSMCImgArrayLength];
#define Rtl8192CEUMCImgArrayLength 1
extern const u1Byte Rtl8192CEFwUMCImgArray[Rtl8192CEUMCImgArrayLength];
#define Rtl8192CEUMCBCutImgArrayLength 1
extern const u1Byte Rtl8192CEFwUMCBCutImgArray[Rtl8192CEUMCBCutImgArrayLength];

#endif //#if (RTL8192CE_HWIMG_SUPPORT == 1)

#endif //__INC_HAL8192CE_FW_IMG_H
