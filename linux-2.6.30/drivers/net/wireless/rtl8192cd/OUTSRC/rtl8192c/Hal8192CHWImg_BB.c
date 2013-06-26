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

#include "../odm_precomp.h"

#if (RTL8192C_HWIMG_SUPPORT == 1)
#define __ODMHWCONFIG_CALLEE_BB
//#include "../odm_HWConfigCallee.h"
static BOOLEAN
CheckCondition(
    const u4Byte  Condition,
    const u4Byte  Hex
    )
{
    u4Byte board = Hex & 0xFF;
    u4Byte interface = Hex & 0xFF00;
    u4Byte platform = Hex & 0xFF0000;
    u4Byte cond = Condition;

    cond = Condition & 0xFF;
    if ( (board & cond) == 0 && cond != 0x1F)
        return FALSE;

    cond = Condition & 0xFF00;
    cond = cond >> 8;
    if ( (interface & cond) == 0 && cond != 0x07)
        return FALSE;

    cond = Condition & 0xFF0000;
    cond = cond >> 16;
    if ( (platform & cond) == 0 && cond != 0x0F)
        return FALSE;
    return TRUE;
}

static u2Byte
odm_SetArrayPointerGetLength_8192C(
     IN   PDM_ODM_T  pDM_Odm,
     IN   u4Byte     ArrayLen,
     IN   pu4Byte    Array,
     OUT  pu4Byte    *gArrayPointer
     )
{
	u4Byte     hex = 0;
	u4Byte     i           = 0;
	u2Byte     count       = 0;
	pu4Byte    ptr_array   = NULL;
	u1Byte     platform    = pDM_Odm->SupportPlatform;
	u1Byte     interface   = pDM_Odm->SupportInterface;
	u1Byte     board       = pDM_Odm->BoardType;  
	ODM_AllocateMemory(pDM_Odm, (PVOID*)&ptr_array, sizeof(u4Byte) * RETURNED_ARRAY_SIZE);

	hex += board;
	hex += interface << 8;
	hex += platform << 16;
	hex += 0xFF000000;
	for (i = 0; i < ArrayLen; i += 2 )
	{
	    u4Byte v1 = Array[i];
	    u4Byte v2 = Array[i+1];

	    // this line is a line of pure_body
	    if ( v1 < 0xCDCDCDCD )
	    {
	        ptr_array[count++] = v1;
	        ptr_array[count++] = v2;
	        continue;
	    }
	    else
	    { // this line is the start of branch
	        if ( !CheckCondition(Array[i], hex) )
	        { // don't need the hw_body
	            i += 2;
	            v1 = Array[i];
	            v2 = Array[i+1];
	            while (v2 != 0xDEAD)
	            {
	                i += 2;
	                v1 = Array[i];
	                v2 = Array[i+1];
	            }
	        }
	    }
	}
	*gArrayPointer = ptr_array;
	return count;
}



/******************************************************************************
*                           AGC_TAB.TXT
******************************************************************************/

u4Byte Array_AGC_TAB_8192C[] = { 
		0x420, 0x00000080,
		0x423, 0x00000000,
		0x430, 0x00000000,
		0x431, 0x00000000,
		0x432, 0x00000000,
		0x433, 0x00000001,
		0x434, 0x00000004,
		0x435, 0x00000005,
		0x436, 0x00000006,
		0x437, 0x00000007,
		0x438, 0x00000000,
		0x439, 0x00000000,
		0x43A, 0x00000000,
		0x43B, 0x00000001,
		0x43C, 0x00000004,
		0x43D, 0x00000005,
		0x43E, 0x00000006,
		0x43F, 0x00000007,
		0x440, 0x0000005D,
		0x441, 0x00000001,
		0x442, 0x00000000,
		0x444, 0x00000015,
		0x445, 0x000000F0,
		0x446, 0x0000000F,
		0x447, 0x00000000,
		0x458, 0x00000041,
		0x459, 0x000000A8,
		0x45A, 0x00000072,
		0x45B, 0x000000B9,
		0x460, 0x00000088,
		0x461, 0x00000088,
		0x462, 0x00000006,
		0x463, 0x00000003,
		0x4C8, 0x00000004,
		0x4C9, 0x00000008,
		0x4CC, 0x00000002,
		0x4CD, 0x00000028,
		0x4CE, 0x00000001,
		0x500, 0x00000026,
		0x501, 0x000000A2,
		0x502, 0x0000002F,
		0x503, 0x00000000,
		0x504, 0x00000028,
		0x505, 0x000000A3,
		0x506, 0x0000005E,
		0x507, 0x00000000,
		0x508, 0x0000002B,
		0x509, 0x000000A4,
		0x50A, 0x0000005E,
		0x50B, 0x00000000,
		0x50C, 0x0000004F,
		0x50D, 0x000000A4,
		0x50E, 0x00000000,
		0x05F, 0x00000000,
		0x512, 0x0000001C,
		0x514, 0x0000000A,
		0x515, 0x00000010,
		0x516, 0x0000000A,
		0x517, 0x00000010,
		0x51A, 0x00000016,
		0x524, 0x0000000F,
		0x525, 0x0000004F,
		0x546, 0x00000020,
		0x547, 0x00000000,
		0x559, 0x00000002,
		0x55A, 0x00000002,
		0x55D, 0x000000FF,
		0x605, 0x00000030,
		0x608, 0x0000000E,
		0x609, 0x0000002A,
		0x652, 0x00000020,
		0x63C, 0x0000000A,
		0x63D, 0x0000000A,
		0x700, 0x00000021,
		0x701, 0x00000043,
		0x702, 0x00000065,
		0x703, 0x00000087,
		0x708, 0x00000021,
		0x709, 0x00000043,
		0x70A, 0x00000065,
		0x70B, 0x00000087,

};

pu4Byte gArrayPointer_AGC_TAB_8192C;

pu4Byte
ODM_GetArrayPointer_AGC_TAB_8192C(
     IN   PDM_ODM_T    pDM_Odm
     )
{
	return gArrayPointer_AGC_TAB_8192C;
}


u2Byte
ODM_GetArrayLength_AGC_TAB_8192C(
     IN   PDM_ODM_T    pDM_Odm
     )
{
	u2Byte len = odm_SetArrayPointerGetLength_8192C(pDM_Odm,
	                                             sizeof(Array_AGC_TAB_8192C)/sizeof(u4Byte),
	                                             Array_AGC_TAB_8192C,
	                                             &gArrayPointer_AGC_TAB_8192C
	                                            );
	return len;
}


void
ODM_ReadAndConfig_AGC_TAB_8192C(
 	IN   PDM_ODM_T  pDM_Odm
 	)
{
	u4Byte     hex         = 0;
	u4Byte     i           = 0;
	u2Byte     count       = 0;
	pu4Byte    ptr_array   = NULL;
	u1Byte     platform    = pDM_Odm->SupportPlatform;
	u1Byte     interface   = pDM_Odm->SupportInterface;
	u1Byte     board       = pDM_Odm->BoardType;  
	u4Byte     ArrayLen    = sizeof(Array_AGC_TAB_8192C)/sizeof(u4Byte);
	pu4Byte    Array       = Array_AGC_TAB_8192C;


	hex += board;
	hex += interface << 8;
	hex += platform << 16;
	hex += 0xFF000000;
	for (i = 0; i < ArrayLen; i += 2 )
	{
	    u4Byte v1 = Array[i];
	    u4Byte v2 = Array[i+1];
	
	    // this line is a line of pure_body
	    if ( v1 < 0xCDCDCDCD )
	    {
		    odm_ConfigBB_AGC_8192C(pDM_Odm, v1, bMaskDWord, v2);
		    continue;
	 	}
		else
		{ // this line is the start of branch
		    if ( !CheckCondition(Array[i], hex) )
		    { // don't need the hw_body
		        i += 2;
		        v1 = Array[i];
		        v2 = Array[i+1];
		        while (v2 != 0xDEAD && 
	                   v2 != 0xCDEF && 
	                   v2 != 0xCDCD)
		        {
		            i += 2;
		            v1 = Array[i];
		            v2 = Array[i+1];
		        }
		    }
		}
	}

}

/******************************************************************************
*                           AGC_TAB_1T.TXT
******************************************************************************/

u4Byte Array_AGC_TAB_1T_8192C[] = { 
		0x420, 0x00000080,
		0x423, 0x00000000,
		0x430, 0x00000000,
		0x431, 0x00000000,
		0x432, 0x00000000,
		0x433, 0x00000001,
		0x434, 0x00000004,
		0x435, 0x00000005,
		0x436, 0x00000006,
		0x437, 0x00000007,
		0x438, 0x00000000,
		0x439, 0x00000000,
		0x43A, 0x00000000,
		0x43B, 0x00000001,
		0x43C, 0x00000004,
		0x43D, 0x00000005,
		0x43E, 0x00000006,
		0x43F, 0x00000007,
		0x440, 0x0000005D,
		0x441, 0x00000001,
		0x442, 0x00000000,
		0x444, 0x00000015,
		0x445, 0x000000F0,
		0x446, 0x0000000F,
		0x447, 0x00000000,
		0x458, 0x00000041,
		0x459, 0x000000A8,
		0x45A, 0x00000072,
		0x45B, 0x000000B9,
		0x460, 0x00000088,
		0x461, 0x00000088,
		0x462, 0x00000006,
		0x463, 0x00000003,
		0x4C8, 0x00000004,
		0x4C9, 0x00000008,
		0x4CC, 0x00000002,
		0x4CD, 0x00000028,
		0x4CE, 0x00000001,
		0x500, 0x00000026,
		0x501, 0x000000A2,
		0x502, 0x0000002F,
		0x503, 0x00000000,
		0x504, 0x00000028,
		0x505, 0x000000A3,
		0x506, 0x0000005E,
		0x507, 0x00000000,
		0x508, 0x0000002B,
		0x509, 0x000000A4,
		0x50A, 0x0000005E,
		0x50B, 0x00000000,
		0x50C, 0x0000004F,
		0x50D, 0x000000A4,
		0x50E, 0x00000000,
		0x05F, 0x00000000,
		0x512, 0x0000001C,
		0x514, 0x0000000A,
		0x515, 0x00000010,
		0x516, 0x0000000A,
		0x517, 0x00000010,
		0x51A, 0x00000016,
		0x524, 0x0000000F,
		0x525, 0x0000004F,
		0x546, 0x00000020,
		0x547, 0x00000000,
		0x559, 0x00000002,
		0x55A, 0x00000002,
		0x55D, 0x000000FF,
		0x605, 0x00000030,
		0x608, 0x0000000E,
		0x609, 0x0000002A,
		0x652, 0x00000020,
		0x63C, 0x0000000A,
		0x63D, 0x0000000A,
		0x700, 0x00000021,
		0x701, 0x00000043,
		0x702, 0x00000065,
		0x703, 0x00000087,
		0x708, 0x00000021,
		0x709, 0x00000043,
		0x70A, 0x00000065,
		0x70B, 0x00000087,

};

pu4Byte gArrayPointer_AGC_TAB_1T_8192C;

pu4Byte
ODM_GetArrayPointer_AGC_TAB_1T_8192C(
     IN   PDM_ODM_T    pDM_Odm
     )
{
	return gArrayPointer_AGC_TAB_1T_8192C;
}


u2Byte
ODM_GetArrayLength_AGC_TAB_1T_8192C(
     IN   PDM_ODM_T    pDM_Odm
     )
{
	u2Byte len = odm_SetArrayPointerGetLength_8192C(pDM_Odm,
	                                             sizeof(Array_AGC_TAB_1T_8192C)/sizeof(u4Byte),
	                                             Array_AGC_TAB_1T_8192C,
	                                             &gArrayPointer_AGC_TAB_1T_8192C
	                                            );
	return len;
}


void
ODM_ReadAndConfig_AGC_TAB_1T_8192C(
 	IN   PDM_ODM_T  pDM_Odm
 	)
{
	u4Byte     hex         = 0;
	u4Byte     i           = 0;
	u2Byte     count       = 0;
	pu4Byte    ptr_array   = NULL;
	u1Byte     platform    = pDM_Odm->SupportPlatform;
	u1Byte     interface   = pDM_Odm->SupportInterface;
	u1Byte     board       = pDM_Odm->BoardType;  
	u4Byte     ArrayLen    = sizeof(Array_AGC_TAB_1T_8192C)/sizeof(u4Byte);
	pu4Byte    Array       = Array_AGC_TAB_1T_8192C;


	hex += board;
	hex += interface << 8;
	hex += platform << 16;
	hex += 0xFF000000;
	for (i = 0; i < ArrayLen; i += 2 )
	{
	    u4Byte v1 = Array[i];
	    u4Byte v2 = Array[i+1];
	
	    // this line is a line of pure_body
	    if ( v1 < 0xCDCDCDCD )
	    {
		    odm_ConfigBB_AGC_8192C(pDM_Odm, v1, bMaskDWord, v2);
		    continue;
	 	}
		else
		{ // this line is the start of branch
		    if ( !CheckCondition(Array[i], hex) )
		    { // don't need the hw_body
		        i += 2;
		        v1 = Array[i];
		        v2 = Array[i+1];
		        while (v2 != 0xDEAD && 
	                   v2 != 0xCDEF && 
	                   v2 != 0xCDCD)
		        {
		            i += 2;
		            v1 = Array[i];
		            v2 = Array[i+1];
		        }
		    }
		}
	}

}

/******************************************************************************
*                           AGC_TAB_2G.TXT
******************************************************************************/

u4Byte Array_AGC_TAB_2G_8192C[] = { 
		0x420, 0x00000080,
		0x423, 0x00000000,
		0x430, 0x00000000,
		0x431, 0x00000000,
		0x432, 0x00000000,
		0x433, 0x00000001,
		0x434, 0x00000004,
		0x435, 0x00000005,
		0x436, 0x00000006,
		0x437, 0x00000007,
		0x438, 0x00000000,
		0x439, 0x00000000,
		0x43A, 0x00000000,
		0x43B, 0x00000001,
		0x43C, 0x00000004,
		0x43D, 0x00000005,
		0x43E, 0x00000006,
		0x43F, 0x00000007,
		0x440, 0x0000005D,
		0x441, 0x00000001,
		0x442, 0x00000000,
		0x444, 0x00000015,
		0x445, 0x000000F0,
		0x446, 0x0000000F,
		0x447, 0x00000000,
		0x458, 0x00000041,
		0x459, 0x000000A8,
		0x45A, 0x00000072,
		0x45B, 0x000000B9,
		0x460, 0x00000088,
		0x461, 0x00000088,
		0x462, 0x00000006,
		0x463, 0x00000003,
		0x4C8, 0x00000004,
		0x4C9, 0x00000008,
		0x4CC, 0x00000002,
		0x4CD, 0x00000028,
		0x4CE, 0x00000001,
		0x500, 0x00000026,
		0x501, 0x000000A2,
		0x502, 0x0000002F,
		0x503, 0x00000000,
		0x504, 0x00000028,
		0x505, 0x000000A3,
		0x506, 0x0000005E,
		0x507, 0x00000000,
		0x508, 0x0000002B,
		0x509, 0x000000A4,
		0x50A, 0x0000005E,
		0x50B, 0x00000000,
		0x50C, 0x0000004F,
		0x50D, 0x000000A4,
		0x50E, 0x00000000,
		0x05F, 0x00000000,
		0x512, 0x0000001C,
		0x514, 0x0000000A,
		0x515, 0x00000010,
		0x516, 0x0000000A,
		0x517, 0x00000010,
		0x51A, 0x00000016,
		0x524, 0x0000000F,
		0x525, 0x0000004F,
		0x546, 0x00000020,
		0x547, 0x00000000,
		0x559, 0x00000002,
		0x55A, 0x00000002,
		0x55D, 0x000000FF,
		0x605, 0x00000030,
		0x608, 0x0000000E,
		0x609, 0x0000002A,
		0x652, 0x00000020,
		0x63C, 0x0000000A,
		0x63D, 0x0000000A,
		0x700, 0x00000021,
		0x701, 0x00000043,
		0x702, 0x00000065,
		0x703, 0x00000087,
		0x708, 0x00000021,
		0x709, 0x00000043,
		0x70A, 0x00000065,
		0x70B, 0x00000087,

};

pu4Byte gArrayPointer_AGC_TAB_2G_8192C;

pu4Byte
ODM_GetArrayPointer_AGC_TAB_2G_8192C(
     IN   PDM_ODM_T    pDM_Odm
     )
{
	return gArrayPointer_AGC_TAB_2G_8192C;
}


u2Byte
ODM_GetArrayLength_AGC_TAB_2G_8192C(
     IN   PDM_ODM_T    pDM_Odm
     )
{
	u2Byte len = odm_SetArrayPointerGetLength_8192C(pDM_Odm,
	                                             sizeof(Array_AGC_TAB_2G_8192C)/sizeof(u4Byte),
	                                             Array_AGC_TAB_2G_8192C,
	                                             &gArrayPointer_AGC_TAB_2G_8192C
	                                            );
	return len;
}


void
ODM_ReadAndConfig_AGC_TAB_2G_8192C(
 	IN   PDM_ODM_T  pDM_Odm
 	)
{
	u4Byte     hex         = 0;
	u4Byte     i           = 0;
	u2Byte     count       = 0;
	pu4Byte    ptr_array   = NULL;
	u1Byte     platform    = pDM_Odm->SupportPlatform;
	u1Byte     interface   = pDM_Odm->SupportInterface;
	u1Byte     board       = pDM_Odm->BoardType;  
	u4Byte     ArrayLen    = sizeof(Array_AGC_TAB_2G_8192C)/sizeof(u4Byte);
	pu4Byte    Array       = Array_AGC_TAB_2G_8192C;


	hex += board;
	hex += interface << 8;
	hex += platform << 16;
	hex += 0xFF000000;
	for (i = 0; i < ArrayLen; i += 2 )
	{
	    u4Byte v1 = Array[i];
	    u4Byte v2 = Array[i+1];
	
	    // this line is a line of pure_body
	    if ( v1 < 0xCDCDCDCD )
	    {
		    odm_ConfigBB_AGC_8192C(pDM_Odm, v1, bMaskDWord, v2);
		    continue;
	 	}
		else
		{ // this line is the start of branch
		    if ( !CheckCondition(Array[i], hex) )
		    { // don't need the hw_body
		        i += 2;
		        v1 = Array[i];
		        v2 = Array[i+1];
		        while (v2 != 0xDEAD && 
	                   v2 != 0xCDEF && 
	                   v2 != 0xCDCD)
		        {
		            i += 2;
		            v1 = Array[i];
		            v2 = Array[i+1];
		        }
		    }
		}
	}

}

/******************************************************************************
*                           AGC_TAB_2T.TXT
******************************************************************************/

u4Byte Array_AGC_TAB_2T_8192C[] = { 
		0x420, 0x00000080,
		0x423, 0x00000000,
		0x430, 0x00000000,
		0x431, 0x00000000,
		0x432, 0x00000000,
		0x433, 0x00000001,
		0x434, 0x00000004,
		0x435, 0x00000005,
		0x436, 0x00000006,
		0x437, 0x00000007,
		0x438, 0x00000000,
		0x439, 0x00000000,
		0x43A, 0x00000000,
		0x43B, 0x00000001,
		0x43C, 0x00000004,
		0x43D, 0x00000005,
		0x43E, 0x00000006,
		0x43F, 0x00000007,
		0x440, 0x0000005D,
		0x441, 0x00000001,
		0x442, 0x00000000,
		0x444, 0x00000015,
		0x445, 0x000000F0,
		0x446, 0x0000000F,
		0x447, 0x00000000,
		0x458, 0x00000041,
		0x459, 0x000000A8,
		0x45A, 0x00000072,
		0x45B, 0x000000B9,
		0x460, 0x00000088,
		0x461, 0x00000088,
		0x462, 0x00000006,
		0x463, 0x00000003,
		0x4C8, 0x00000004,
		0x4C9, 0x00000008,
		0x4CC, 0x00000002,
		0x4CD, 0x00000028,
		0x4CE, 0x00000001,
		0x500, 0x00000026,
		0x501, 0x000000A2,
		0x502, 0x0000002F,
		0x503, 0x00000000,
		0x504, 0x00000028,
		0x505, 0x000000A3,
		0x506, 0x0000005E,
		0x507, 0x00000000,
		0x508, 0x0000002B,
		0x509, 0x000000A4,
		0x50A, 0x0000005E,
		0x50B, 0x00000000,
		0x50C, 0x0000004F,
		0x50D, 0x000000A4,
		0x50E, 0x00000000,
		0x05F, 0x00000000,
		0x512, 0x0000001C,
		0x514, 0x0000000A,
		0x515, 0x00000010,
		0x516, 0x0000000A,
		0x517, 0x00000010,
		0x51A, 0x00000016,
		0x524, 0x0000000F,
		0x525, 0x0000004F,
		0x546, 0x00000020,
		0x547, 0x00000000,
		0x559, 0x00000002,
		0x55A, 0x00000002,
		0x55D, 0x000000FF,
		0x605, 0x00000030,
		0x608, 0x0000000E,
		0x609, 0x0000002A,
		0x652, 0x00000020,
		0x63C, 0x0000000A,
		0x63D, 0x0000000A,
		0x700, 0x00000021,
		0x701, 0x00000043,
		0x702, 0x00000065,
		0x703, 0x00000087,
		0x708, 0x00000021,
		0x709, 0x00000043,
		0x70A, 0x00000065,
		0x70B, 0x00000087,

};

pu4Byte gArrayPointer_AGC_TAB_2T_8192C;

pu4Byte
ODM_GetArrayPointer_AGC_TAB_2T_8192C(
     IN   PDM_ODM_T    pDM_Odm
     )
{
	return gArrayPointer_AGC_TAB_2T_8192C;
}


u2Byte
ODM_GetArrayLength_AGC_TAB_2T_8192C(
     IN   PDM_ODM_T    pDM_Odm
     )
{
	u2Byte len = odm_SetArrayPointerGetLength_8192C(pDM_Odm,
	                                             sizeof(Array_AGC_TAB_2T_8192C)/sizeof(u4Byte),
	                                             Array_AGC_TAB_2T_8192C,
	                                             &gArrayPointer_AGC_TAB_2T_8192C
	                                            );
	return len;
}


void
ODM_ReadAndConfig_AGC_TAB_2T_8192C(
 	IN   PDM_ODM_T  pDM_Odm
 	)
{
	u4Byte     hex         = 0;
	u4Byte     i           = 0;
	u2Byte     count       = 0;
	pu4Byte    ptr_array   = NULL;
	u1Byte     platform    = pDM_Odm->SupportPlatform;
	u1Byte     interface   = pDM_Odm->SupportInterface;
	u1Byte     board       = pDM_Odm->BoardType;  
	u4Byte     ArrayLen    = sizeof(Array_AGC_TAB_2T_8192C)/sizeof(u4Byte);
	pu4Byte    Array       = Array_AGC_TAB_2T_8192C;


	hex += board;
	hex += interface << 8;
	hex += platform << 16;
	hex += 0xFF000000;
	for (i = 0; i < ArrayLen; i += 2 )
	{
	    u4Byte v1 = Array[i];
	    u4Byte v2 = Array[i+1];
	
	    // this line is a line of pure_body
	    if ( v1 < 0xCDCDCDCD )
	    {
		    odm_ConfigBB_AGC_8192C(pDM_Odm, v1, bMaskDWord, v2);
		    continue;
	 	}
		else
		{ // this line is the start of branch
		    if ( !CheckCondition(Array[i], hex) )
		    { // don't need the hw_body
		        i += 2;
		        v1 = Array[i];
		        v2 = Array[i+1];
		        while (v2 != 0xDEAD && 
	                   v2 != 0xCDEF && 
	                   v2 != 0xCDCD)
		        {
		            i += 2;
		            v1 = Array[i];
		            v2 = Array[i+1];
		        }
		    }
		}
	}

}

/******************************************************************************
*                           AGC_TAB_5G.TXT
******************************************************************************/

u4Byte Array_AGC_TAB_5G_8192C[] = { 
		0x420, 0x00000080,
		0x423, 0x00000000,
		0x430, 0x00000000,
		0x431, 0x00000000,
		0x432, 0x00000000,
		0x433, 0x00000001,
		0x434, 0x00000004,
		0x435, 0x00000005,
		0x436, 0x00000006,
		0x437, 0x00000007,
		0x438, 0x00000000,
		0x439, 0x00000000,
		0x43A, 0x00000000,
		0x43B, 0x00000001,
		0x43C, 0x00000004,
		0x43D, 0x00000005,
		0x43E, 0x00000006,
		0x43F, 0x00000007,
		0x440, 0x0000005D,
		0x441, 0x00000001,
		0x442, 0x00000000,
		0x444, 0x00000015,
		0x445, 0x000000F0,
		0x446, 0x0000000F,
		0x447, 0x00000000,
		0x458, 0x00000041,
		0x459, 0x000000A8,
		0x45A, 0x00000072,
		0x45B, 0x000000B9,
		0x460, 0x00000088,
		0x461, 0x00000088,
		0x462, 0x00000006,
		0x463, 0x00000003,
		0x4C8, 0x00000004,
		0x4C9, 0x00000008,
		0x4CC, 0x00000002,
		0x4CD, 0x00000028,
		0x4CE, 0x00000001,
		0x500, 0x00000026,
		0x501, 0x000000A2,
		0x502, 0x0000002F,
		0x503, 0x00000000,
		0x504, 0x00000028,
		0x505, 0x000000A3,
		0x506, 0x0000005E,
		0x507, 0x00000000,
		0x508, 0x0000002B,
		0x509, 0x000000A4,
		0x50A, 0x0000005E,
		0x50B, 0x00000000,
		0x50C, 0x0000004F,
		0x50D, 0x000000A4,
		0x50E, 0x00000000,
		0x05F, 0x00000000,
		0x512, 0x0000001C,
		0x514, 0x0000000A,
		0x515, 0x00000010,
		0x516, 0x0000000A,
		0x517, 0x00000010,
		0x51A, 0x00000016,
		0x524, 0x0000000F,
		0x525, 0x0000004F,
		0x546, 0x00000020,
		0x547, 0x00000000,
		0x559, 0x00000002,
		0x55A, 0x00000002,
		0x55D, 0x000000FF,
		0x605, 0x00000030,
		0x608, 0x0000000E,
		0x609, 0x0000002A,
		0x652, 0x00000020,
		0x63C, 0x0000000A,
		0x63D, 0x0000000A,
		0x700, 0x00000021,
		0x701, 0x00000043,
		0x702, 0x00000065,
		0x703, 0x00000087,
		0x708, 0x00000021,
		0x709, 0x00000043,
		0x70A, 0x00000065,
		0x70B, 0x00000087,

};

pu4Byte gArrayPointer_AGC_TAB_5G_8192C;

pu4Byte
ODM_GetArrayPointer_AGC_TAB_5G_8192C(
     IN   PDM_ODM_T    pDM_Odm
     )
{
	return gArrayPointer_AGC_TAB_5G_8192C;
}


u2Byte
ODM_GetArrayLength_AGC_TAB_5G_8192C(
     IN   PDM_ODM_T    pDM_Odm
     )
{
	u2Byte len = odm_SetArrayPointerGetLength_8192C(pDM_Odm,
	                                             sizeof(Array_AGC_TAB_5G_8192C)/sizeof(u4Byte),
	                                             Array_AGC_TAB_5G_8192C,
	                                             &gArrayPointer_AGC_TAB_5G_8192C
	                                            );
	return len;
}


void
ODM_ReadAndConfig_AGC_TAB_5G_8192C(
 	IN   PDM_ODM_T  pDM_Odm
 	)
{
	u4Byte     hex         = 0;
	u4Byte     i           = 0;
	u2Byte     count       = 0;
	pu4Byte    ptr_array   = NULL;
	u1Byte     platform    = pDM_Odm->SupportPlatform;
	u1Byte     interface   = pDM_Odm->SupportInterface;
	u1Byte     board       = pDM_Odm->BoardType;  
	u4Byte     ArrayLen    = sizeof(Array_AGC_TAB_5G_8192C)/sizeof(u4Byte);
	pu4Byte    Array       = Array_AGC_TAB_5G_8192C;


	hex += board;
	hex += interface << 8;
	hex += platform << 16;
	hex += 0xFF000000;
	for (i = 0; i < ArrayLen; i += 2 )
	{
	    u4Byte v1 = Array[i];
	    u4Byte v2 = Array[i+1];
	
	    // this line is a line of pure_body
	    if ( v1 < 0xCDCDCDCD )
	    {
		    odm_ConfigBB_AGC_8192C(pDM_Odm, v1, bMaskDWord, v2);
		    continue;
	 	}
		else
		{ // this line is the start of branch
		    if ( !CheckCondition(Array[i], hex) )
		    { // don't need the hw_body
		        i += 2;
		        v1 = Array[i];
		        v2 = Array[i+1];
		        while (v2 != 0xDEAD && 
	                   v2 != 0xCDEF && 
	                   v2 != 0xCDCD)
		        {
		            i += 2;
		            v1 = Array[i];
		            v2 = Array[i+1];
		        }
		    }
		}
	}

}

/******************************************************************************
*                           PHY_REG.TXT
******************************************************************************/

u4Byte Array_PHY_REG_8192C[] = { 
		0x420, 0x00000080,
		0x423, 0x00000000,
		0x430, 0x00000000,
		0x431, 0x00000000,
		0x432, 0x00000000,
		0x433, 0x00000001,
		0x434, 0x00000004,
		0x435, 0x00000005,
		0x436, 0x00000006,
		0x437, 0x00000007,
		0x438, 0x00000000,
		0x439, 0x00000000,
		0x43A, 0x00000000,
		0x43B, 0x00000001,
		0x43C, 0x00000004,
		0x43D, 0x00000005,
		0x43E, 0x00000006,
		0x43F, 0x00000007,
		0x440, 0x0000005D,
		0x441, 0x00000001,
		0x442, 0x00000000,
		0x444, 0x00000015,
		0x445, 0x000000F0,
		0x446, 0x0000000F,
		0x447, 0x00000000,
		0x458, 0x00000041,
		0x459, 0x000000A8,
		0x45A, 0x00000072,
		0x45B, 0x000000B9,
		0x460, 0x00000088,
		0x461, 0x00000088,
		0x462, 0x00000006,
		0x463, 0x00000003,
		0x4C8, 0x00000004,
		0x4C9, 0x00000008,
		0x4CC, 0x00000002,
		0x4CD, 0x00000028,
		0x4CE, 0x00000001,
		0x500, 0x00000026,
		0x501, 0x000000A2,
		0x502, 0x0000002F,
		0x503, 0x00000000,
		0x504, 0x00000028,
		0x505, 0x000000A3,
		0x506, 0x0000005E,
		0x507, 0x00000000,
		0x508, 0x0000002B,
		0x509, 0x000000A4,
		0x50A, 0x0000005E,
		0x50B, 0x00000000,
		0x50C, 0x0000004F,
		0x50D, 0x000000A4,
		0x50E, 0x00000000,
		0x05F, 0x00000000,
		0x512, 0x0000001C,
		0x514, 0x0000000A,
		0x515, 0x00000010,
		0x516, 0x0000000A,
		0x517, 0x00000010,
		0x51A, 0x00000016,
		0x524, 0x0000000F,
		0x525, 0x0000004F,
		0x546, 0x00000020,
		0x547, 0x00000000,
		0x559, 0x00000002,
		0x55A, 0x00000002,
		0x55D, 0x000000FF,
		0x605, 0x00000030,
		0x608, 0x0000000E,
		0x609, 0x0000002A,
		0x652, 0x00000020,
		0x63C, 0x0000000A,
		0x63D, 0x0000000A,
		0x700, 0x00000021,
		0x701, 0x00000043,
		0x702, 0x00000065,
		0x703, 0x00000087,
		0x708, 0x00000021,
		0x709, 0x00000043,
		0x70A, 0x00000065,
		0x70B, 0x00000087,

};

pu4Byte gArrayPointer_PHY_REG_8192C;

pu4Byte
ODM_GetArrayPointer_PHY_REG_8192C(
     IN   PDM_ODM_T    pDM_Odm
     )
{
	return gArrayPointer_PHY_REG_8192C;
}


u2Byte
ODM_GetArrayLength_PHY_REG_8192C(
     IN   PDM_ODM_T    pDM_Odm
     )
{
	u2Byte len = odm_SetArrayPointerGetLength_8192C(pDM_Odm,
	                                             sizeof(Array_PHY_REG_8192C)/sizeof(u4Byte),
	                                             Array_PHY_REG_8192C,
	                                             &gArrayPointer_PHY_REG_8192C
	                                            );
	return len;
}


void
ODM_ReadAndConfig_PHY_REG_8192C(
 	IN   PDM_ODM_T  pDM_Odm
 	)
{
	u4Byte     hex         = 0;
	u4Byte     i           = 0;
	u2Byte     count       = 0;
	pu4Byte    ptr_array   = NULL;
	u1Byte     platform    = pDM_Odm->SupportPlatform;
	u1Byte     interface   = pDM_Odm->SupportInterface;
	u1Byte     board       = pDM_Odm->BoardType;  
	u4Byte     ArrayLen    = sizeof(Array_PHY_REG_8192C)/sizeof(u4Byte);
	pu4Byte    Array       = Array_PHY_REG_8192C;


	hex += board;
	hex += interface << 8;
	hex += platform << 16;
	hex += 0xFF000000;
	for (i = 0; i < ArrayLen; i += 2 )
	{
	    u4Byte v1 = Array[i];
	    u4Byte v2 = Array[i+1];
	
	    // this line is a line of pure_body
	    if ( v1 < 0xCDCDCDCD )
	    {
		   	odm_ConfigBB_PHY_8192C(pDM_Odm, v1, bMaskDWord, v2);
		    continue;
	 	}
		else
		{ // this line is the start of branch
		    if ( !CheckCondition(Array[i], hex) )
		    { // don't need the hw_body
		        i += 2;
		        v1 = Array[i];
		        v2 = Array[i+1];
		        while (v2 != 0xDEAD && 
	                   v2 != 0xCDEF && 
	                   v2 != 0xCDCD)
		        {
		            i += 2;
		            v1 = Array[i];
		            v2 = Array[i+1];
		        }
		    }
		}
	}

}

/******************************************************************************
*                           PHY_REG_1T.TXT
******************************************************************************/

u4Byte Array_PHY_REG_1T_8192C[] = { 
		0x420, 0x00000080,
		0x423, 0x00000000,
		0x430, 0x00000000,
		0x431, 0x00000000,
		0x432, 0x00000000,
		0x433, 0x00000001,
		0x434, 0x00000004,
		0x435, 0x00000005,
		0x436, 0x00000006,
		0x437, 0x00000007,
		0x438, 0x00000000,
		0x439, 0x00000000,
		0x43A, 0x00000000,
		0x43B, 0x00000001,
		0x43C, 0x00000004,
		0x43D, 0x00000005,
		0x43E, 0x00000006,
		0x43F, 0x00000007,
		0x440, 0x0000005D,
		0x441, 0x00000001,
		0x442, 0x00000000,
		0x444, 0x00000015,
		0x445, 0x000000F0,
		0x446, 0x0000000F,
		0x447, 0x00000000,
		0x458, 0x00000041,
		0x459, 0x000000A8,
		0x45A, 0x00000072,
		0x45B, 0x000000B9,
		0x460, 0x00000088,
		0x461, 0x00000088,
		0x462, 0x00000006,
		0x463, 0x00000003,
		0x4C8, 0x00000004,
		0x4C9, 0x00000008,
		0x4CC, 0x00000002,
		0x4CD, 0x00000028,
		0x4CE, 0x00000001,
		0x500, 0x00000026,
		0x501, 0x000000A2,
		0x502, 0x0000002F,
		0x503, 0x00000000,
		0x504, 0x00000028,
		0x505, 0x000000A3,
		0x506, 0x0000005E,
		0x507, 0x00000000,
		0x508, 0x0000002B,
		0x509, 0x000000A4,
		0x50A, 0x0000005E,
		0x50B, 0x00000000,
		0x50C, 0x0000004F,
		0x50D, 0x000000A4,
		0x50E, 0x00000000,
		0x05F, 0x00000000,
		0x512, 0x0000001C,
		0x514, 0x0000000A,
		0x515, 0x00000010,
		0x516, 0x0000000A,
		0x517, 0x00000010,
		0x51A, 0x00000016,
		0x524, 0x0000000F,
		0x525, 0x0000004F,
		0x546, 0x00000020,
		0x547, 0x00000000,
		0x559, 0x00000002,
		0x55A, 0x00000002,
		0x55D, 0x000000FF,
		0x605, 0x00000030,
		0x608, 0x0000000E,
		0x609, 0x0000002A,
		0x652, 0x00000020,
		0x63C, 0x0000000A,
		0x63D, 0x0000000A,
		0x700, 0x00000021,
		0x701, 0x00000043,
		0x702, 0x00000065,
		0x703, 0x00000087,
		0x708, 0x00000021,
		0x709, 0x00000043,
		0x70A, 0x00000065,
		0x70B, 0x00000087,

};

pu4Byte gArrayPointer_PHY_REG_1T_8192C;

pu4Byte
ODM_GetArrayPointer_PHY_REG_1T_8192C(
     IN   PDM_ODM_T    pDM_Odm
     )
{
	return gArrayPointer_PHY_REG_1T_8192C;
}


u2Byte
ODM_GetArrayLength_PHY_REG_1T_8192C(
     IN   PDM_ODM_T    pDM_Odm
     )
{
	u2Byte len = odm_SetArrayPointerGetLength_8192C(pDM_Odm,
	                                             sizeof(Array_PHY_REG_1T_8192C)/sizeof(u4Byte),
	                                             Array_PHY_REG_1T_8192C,
	                                             &gArrayPointer_PHY_REG_1T_8192C
	                                            );
	return len;
}


void
ODM_ReadAndConfig_PHY_REG_1T_8192C(
 	IN   PDM_ODM_T  pDM_Odm
 	)
{
	u4Byte     hex         = 0;
	u4Byte     i           = 0;
	u2Byte     count       = 0;
	pu4Byte    ptr_array   = NULL;
	u1Byte     platform    = pDM_Odm->SupportPlatform;
	u1Byte     interface   = pDM_Odm->SupportInterface;
	u1Byte     board       = pDM_Odm->BoardType;  
	u4Byte     ArrayLen    = sizeof(Array_PHY_REG_1T_8192C)/sizeof(u4Byte);
	pu4Byte    Array       = Array_PHY_REG_1T_8192C;


	hex += board;
	hex += interface << 8;
	hex += platform << 16;
	hex += 0xFF000000;
	for (i = 0; i < ArrayLen; i += 2 )
	{
	    u4Byte v1 = Array[i];
	    u4Byte v2 = Array[i+1];
	
	    // this line is a line of pure_body
	    if ( v1 < 0xCDCDCDCD )
	    {
		   	odm_ConfigBB_PHY_8192C(pDM_Odm, v1, bMaskDWord, v2);
		    continue;
	 	}
		else
		{ // this line is the start of branch
		    if ( !CheckCondition(Array[i], hex) )
		    { // don't need the hw_body
		        i += 2;
		        v1 = Array[i];
		        v2 = Array[i+1];
		        while (v2 != 0xDEAD && 
	                   v2 != 0xCDEF && 
	                   v2 != 0xCDCD)
		        {
		            i += 2;
		            v1 = Array[i];
		            v2 = Array[i+1];
		        }
		    }
		}
	}

}

/******************************************************************************
*                           PHY_REG_2T.TXT
******************************************************************************/

u4Byte Array_PHY_REG_2T_8192C[] = { 
		0x420, 0x00000080,
		0x423, 0x00000000,
		0x430, 0x00000000,
		0x431, 0x00000000,
		0x432, 0x00000000,
		0x433, 0x00000001,
		0x434, 0x00000004,
		0x435, 0x00000005,
		0x436, 0x00000006,
		0x437, 0x00000007,
		0x438, 0x00000000,
		0x439, 0x00000000,
		0x43A, 0x00000000,
		0x43B, 0x00000001,
		0x43C, 0x00000004,
		0x43D, 0x00000005,
		0x43E, 0x00000006,
		0x43F, 0x00000007,
		0x440, 0x0000005D,
		0x441, 0x00000001,
		0x442, 0x00000000,
		0x444, 0x00000015,
		0x445, 0x000000F0,
		0x446, 0x0000000F,
		0x447, 0x00000000,
		0x458, 0x00000041,
		0x459, 0x000000A8,
		0x45A, 0x00000072,
		0x45B, 0x000000B9,
		0x460, 0x00000088,
		0x461, 0x00000088,
		0x462, 0x00000006,
		0x463, 0x00000003,
		0x4C8, 0x00000004,
		0x4C9, 0x00000008,
		0x4CC, 0x00000002,
		0x4CD, 0x00000028,
		0x4CE, 0x00000001,
		0x500, 0x00000026,
		0x501, 0x000000A2,
		0x502, 0x0000002F,
		0x503, 0x00000000,
		0x504, 0x00000028,
		0x505, 0x000000A3,
		0x506, 0x0000005E,
		0x507, 0x00000000,
		0x508, 0x0000002B,
		0x509, 0x000000A4,
		0x50A, 0x0000005E,
		0x50B, 0x00000000,
		0x50C, 0x0000004F,
		0x50D, 0x000000A4,
		0x50E, 0x00000000,
		0x05F, 0x00000000,
		0x512, 0x0000001C,
		0x514, 0x0000000A,
		0x515, 0x00000010,
		0x516, 0x0000000A,
		0x517, 0x00000010,
		0x51A, 0x00000016,
		0x524, 0x0000000F,
		0x525, 0x0000004F,
		0x546, 0x00000020,
		0x547, 0x00000000,
		0x559, 0x00000002,
		0x55A, 0x00000002,
		0x55D, 0x000000FF,
		0x605, 0x00000030,
		0x608, 0x0000000E,
		0x609, 0x0000002A,
		0x652, 0x00000020,
		0x63C, 0x0000000A,
		0x63D, 0x0000000A,
		0x700, 0x00000021,
		0x701, 0x00000043,
		0x702, 0x00000065,
		0x703, 0x00000087,
		0x708, 0x00000021,
		0x709, 0x00000043,
		0x70A, 0x00000065,
		0x70B, 0x00000087,

};

pu4Byte gArrayPointer_PHY_REG_2T_8192C;

pu4Byte
ODM_GetArrayPointer_PHY_REG_2T_8192C(
     IN   PDM_ODM_T    pDM_Odm
     )
{
	return gArrayPointer_PHY_REG_2T_8192C;
}


u2Byte
ODM_GetArrayLength_PHY_REG_2T_8192C(
     IN   PDM_ODM_T    pDM_Odm
     )
{
	u2Byte len = odm_SetArrayPointerGetLength_8192C(pDM_Odm,
	                                             sizeof(Array_PHY_REG_2T_8192C)/sizeof(u4Byte),
	                                             Array_PHY_REG_2T_8192C,
	                                             &gArrayPointer_PHY_REG_2T_8192C
	                                            );
	return len;
}


void
ODM_ReadAndConfig_PHY_REG_2T_8192C(
 	IN   PDM_ODM_T  pDM_Odm
 	)
{
	u4Byte     hex         = 0;
	u4Byte     i           = 0;
	u2Byte     count       = 0;
	pu4Byte    ptr_array   = NULL;
	u1Byte     platform    = pDM_Odm->SupportPlatform;
	u1Byte     interface   = pDM_Odm->SupportInterface;
	u1Byte     board       = pDM_Odm->BoardType;  
	u4Byte     ArrayLen    = sizeof(Array_PHY_REG_2T_8192C)/sizeof(u4Byte);
	pu4Byte    Array       = Array_PHY_REG_2T_8192C;


	hex += board;
	hex += interface << 8;
	hex += platform << 16;
	hex += 0xFF000000;
	for (i = 0; i < ArrayLen; i += 2 )
	{
	    u4Byte v1 = Array[i];
	    u4Byte v2 = Array[i+1];
	
	    // this line is a line of pure_body
	    if ( v1 < 0xCDCDCDCD )
	    {
		   	odm_ConfigBB_PHY_8192C(pDM_Odm, v1, bMaskDWord, v2);
		    continue;
	 	}
		else
		{ // this line is the start of branch
		    if ( !CheckCondition(Array[i], hex) )
		    { // don't need the hw_body
		        i += 2;
		        v1 = Array[i];
		        v2 = Array[i+1];
		        while (v2 != 0xDEAD && 
	                   v2 != 0xCDEF && 
	                   v2 != 0xCDCD)
		        {
		            i += 2;
		            v1 = Array[i];
		            v2 = Array[i+1];
		        }
		    }
		}
	}

}

#endif // end of HWIMG_SUPPORT