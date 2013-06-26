/*
** Copyright ?2008 by Silicon Laboratories
**
** $Id: vdaa_constants.c 208 2008-11-07 16:27:34Z lajordan@SILABS.COM $
**
** vdaa_constants.c
** VoiceDAA example presets
**
** Author(s): 
** naqamar, laj
**
** Distributed by: 
** Silicon Laboratories, Inc
**
** This file contains proprietary information.	 
** No dissemination allowed without prior written permission from
** Silicon Laboratories, Inc.
**
** File Description:
** This file is used 
** in the VoiceDAA demonstration code. 
**
**
*/
#include "vdaa.h"

//#define TIME_250		/* Timing Counetr for Reset() */

vdaa_General_Cfg Vdaa_General_Configuration  = {
INTE_INTERRUPT, /*Interrupt enable*/
INTE_ACTIVE_LOW, /*Interrupt pin polarity*/
PWM_DELTA_SIGMA, /*PWM Mode*/
FALSE /*PWM enable*/
};

vdaa_PCM_Cfg Vdaa_PCM_Presets [] ={
    { A_LAW, PCLK_1_PER_BIT,  TRI_POS_EDGE},
    { U_LAW, PCLK_1_PER_BIT,  TRI_POS_EDGE},
    { LINEAR_16_BIT, PCLK_1_PER_BIT,  TRI_POS_EDGE}
};


vdaa_Ring_Detect_Cfg Vdaa_Ring_Detect_Presets[] ={

      { RDLY_256MS,RT__13_5VRMS_16_5VRMS,/*rmx*/ 0x000000,RTO_128MS,RCC_200MS,
		  RNGV_ENABLED, /*ras*/0x000000,RFWE_RNGV_THRESH_CROSS , RDI_BEG_BURST, RGDT_ACTIVE_LOW}
};

vdaa_audioGain_Cfg Vdaa_audioGain_Presets[] ={

	{0x0, 0x0, 0x03, 0x1, 0x01, 0x7F}
};

 vdaa_Impedance_Cfg Vdaa_Impedance_Presets [] = {
	 {RZ_MAX,DC_50,AC_600,	0,0,0,0,0,0,0,0,DCV3_1,MINI_10MA,ILIM_DISABLED,
		 OHS_LESS_THAN_0_5MS,HYBRID_ENABLE}

};

 vdaa_Loopback_Cfg Vdaa_Loopback_Presets [] = {
	 {IDL_DISABLED, DDL_NORMAL_OPERATION}
 };

