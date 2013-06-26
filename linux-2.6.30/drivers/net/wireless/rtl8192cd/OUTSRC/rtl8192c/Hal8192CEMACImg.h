#ifndef __INC_HAL8192CE_FW_IMG_H
#define __INC_HAL8192CE_FW_IMG_H

/*Created on  2011/ 6/16,  6: 8*/

#if (RTL8192CE_HWIMG_SUPPORT == 1)

#define Rtl8192CETSMCImgArrayLength 14950
extern u1Byte Rtl8192CEFwTSMCImgArray[Rtl8192CETSMCImgArrayLength];
#define Rtl8192CEUMCImgArrayLength 14950
extern u1Byte Rtl8192CEFwUMCImgArray[Rtl8192CEUMCImgArrayLength];
#define Rtl8192CEUMCBCutImgArrayLength 14930
extern u1Byte Rtl8192CEFwUMCBCutImgArray[Rtl8192CEUMCBCutImgArrayLength];
#define Rtl8192CEPHY_REG_2TArrayLength 374
extern u4Byte Rtl8192CEPHY_REG_2TArray[Rtl8192CEPHY_REG_2TArrayLength];
#define Rtl8192CEPHY_REG_1TArrayLength 374
extern u4Byte Rtl8192CEPHY_REG_1TArray[Rtl8192CEPHY_REG_1TArrayLength];
#define Rtl8192CEPHY_ChangeTo_1T1RArrayLength 1
extern u4Byte Rtl8192CEPHY_ChangeTo_1T1RArray[Rtl8192CEPHY_ChangeTo_1T1RArrayLength];
#define Rtl8192CEPHY_ChangeTo_1T2RArrayLength 1
extern u4Byte Rtl8192CEPHY_ChangeTo_1T2RArray[Rtl8192CEPHY_ChangeTo_1T2RArrayLength];
#define Rtl8192CEPHY_ChangeTo_2T2RArrayLength 1
extern u4Byte Rtl8192CEPHY_ChangeTo_2T2RArray[Rtl8192CEPHY_ChangeTo_2T2RArrayLength];
#define Rtl8192CEPHY_REG_Array_PGLength 336
extern u4Byte Rtl8192CEPHY_REG_Array_PG[Rtl8192CEPHY_REG_Array_PGLength];
#define Rtl8192CEPHY_REG_Array_MPLength 4
extern u4Byte Rtl8192CEPHY_REG_Array_MP[Rtl8192CEPHY_REG_Array_MPLength];
#define Rtl8192CERadioA_2TArrayLength 282
extern u4Byte Rtl8192CERadioA_2TArray[Rtl8192CERadioA_2TArrayLength];
#define Rtl8192CERadioB_2TArrayLength 78
extern u4Byte Rtl8192CERadioB_2TArray[Rtl8192CERadioB_2TArrayLength];
#define Rtl8192CERadioA_1TArrayLength 282
extern u4Byte Rtl8192CERadioA_1TArray[Rtl8192CERadioA_1TArrayLength];
#define Rtl8192CERadioB_1TArrayLength 1
extern u4Byte Rtl8192CERadioB_1TArray[Rtl8192CERadioB_1TArrayLength];
#define Rtl8192CERadioB_GM_ArrayLength 1
extern u4Byte Rtl8192CERadioB_GM_Array[Rtl8192CERadioB_GM_ArrayLength];
#define Rtl8192CEMAC_2T_ArrayLength 172
extern u4Byte Rtl8192CEMAC_2T_Array[Rtl8192CEMAC_2T_ArrayLength];
#define Rtl8192CEMACPHY_Array_PGLength 1
extern u4Byte Rtl8192CEMACPHY_Array_PG[Rtl8192CEMACPHY_Array_PGLength];
#define Rtl8192CEAGCTAB_2TArrayLength 320
extern u4Byte Rtl8192CEAGCTAB_2TArray[Rtl8192CEAGCTAB_2TArrayLength];
#define Rtl8192CEAGCTAB_1TArrayLength 320
extern u4Byte Rtl8192CEAGCTAB_1TArray[Rtl8192CEAGCTAB_1TArrayLength];

#else

#define Rtl8192CETSMCImgArrayLength 1
extern u1Byte Rtl8192CEFwTSMCImgArray[Rtl8192CETSMCImgArrayLength];
#define Rtl8192CEUMCImgArrayLength 1
extern u1Byte Rtl8192CEFwUMCImgArray[Rtl8192CEUMCImgArrayLength];
#define Rtl8192CEUMCBCutImgArrayLength 1
extern u1Byte Rtl8192CEFwUMCBCutImgArray[Rtl8192CEUMCBCutImgArrayLength];
#define Rtl8192CEPHY_REG_2TArrayLength 1
extern u4Byte Rtl8192CEPHY_REG_2TArray[Rtl8192CEPHY_REG_2TArrayLength];
#define Rtl8192CEPHY_REG_1TArrayLength 1
extern u4Byte Rtl8192CEPHY_REG_1TArray[Rtl8192CEPHY_REG_1TArrayLength];
#define Rtl8192CEPHY_ChangeTo_1T1RArrayLength 1
extern u4Byte Rtl8192CEPHY_ChangeTo_1T1RArray[Rtl8192CEPHY_ChangeTo_1T1RArrayLength];
#define Rtl8192CEPHY_ChangeTo_1T2RArrayLength 1
extern u4Byte Rtl8192CEPHY_ChangeTo_1T2RArray[Rtl8192CEPHY_ChangeTo_1T2RArrayLength];
#define Rtl8192CEPHY_ChangeTo_2T2RArrayLength 1
extern u4Byte Rtl8192CEPHY_ChangeTo_2T2RArray[Rtl8192CEPHY_ChangeTo_2T2RArrayLength];
#define Rtl8192CEPHY_REG_Array_PGLength 1
extern u4Byte Rtl8192CEPHY_REG_Array_PG[Rtl8192CEPHY_REG_Array_PGLength];
#define Rtl8192CEPHY_REG_Array_MPLength 1
extern u4Byte Rtl8192CEPHY_REG_Array_MP[Rtl8192CEPHY_REG_Array_MPLength];
#define Rtl8192CERadioA_2TArrayLength 1
extern u4Byte Rtl8192CERadioA_2TArray[Rtl8192CERadioA_2TArrayLength];
#define Rtl8192CERadioB_2TArrayLength 1
extern u4Byte Rtl8192CERadioB_2TArray[Rtl8192CERadioB_2TArrayLength];
#define Rtl8192CERadioA_1TArrayLength 1
extern u4Byte Rtl8192CERadioA_1TArray[Rtl8192CERadioA_1TArrayLength];
#define Rtl8192CERadioB_1TArrayLength 1
extern u4Byte Rtl8192CERadioB_1TArray[Rtl8192CERadioB_1TArrayLength];
#define Rtl8192CERadioB_GM_ArrayLength 1
extern u4Byte Rtl8192CERadioB_GM_Array[Rtl8192CERadioB_GM_ArrayLength];
#define Rtl8192CEMAC_2T_ArrayLength 1
extern u4Byte Rtl8192CEMAC_2T_Array[Rtl8192CEMAC_2T_ArrayLength];
#define Rtl8192CEMACPHY_Array_PGLength 1
extern u4Byte Rtl8192CEMACPHY_Array_PG[Rtl8192CEMACPHY_Array_PGLength];
#define Rtl8192CEAGCTAB_2TArrayLength 1
extern u4Byte Rtl8192CEAGCTAB_2TArray[Rtl8192CEAGCTAB_2TArrayLength];
#define Rtl8192CEAGCTAB_1TArrayLength 1
extern u4Byte Rtl8192CEAGCTAB_1TArray[Rtl8192CEAGCTAB_1TArrayLength];

#endif //#if (RTL8192CE_HWIMG_SUPPORT == 1)

#endif //__INC_HAL8192CE_FW_IMG_H
