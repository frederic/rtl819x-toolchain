/*
** Copyright (c) 2008 by Silicon Laboratories
**
** $Id: vdaa.h 554 2009-03-25 23:11:55Z cdp@SILABS.COM $
**
** Vdaa.h
** Vdaa  VoiceDAA interface header file
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
** This is the header file for the main  VoiceDAA API and is used 
** in the  VoiceDAA demonstration code. 
**
** Dependancies:
** Customer Drivers
**
*/
#ifndef VDAA_INTF_H
#define VDAA_INTF_H

#include "si_voice_datatypes.h"
#include "si_voice.h"
/*
**
** VDAA INITIALIZATION FUNCTIONS
**
*/
/*
** This defines names for the PCM Data Format
*/
 typedef enum {
	A_LAW = 0,	/*	00 = A-Law. Signed magnitude data format */
	U_LAW = 1,	/*	01 = u-Law. Signed magnitude data format */
	LINEAR_16_BIT = 3	/*	11 = 16-bit linear (2s complement data format) */
}tPcmFormat;

/*
** This defines names for the phcf bits
*/
 typedef enum {
	PCLK_1_PER_BIT = 0,
	PCLK_2_PER_BIT = 1
}tPHCF;


/*
** This defines names for the tri bit
*/
 typedef enum {
	TRI_POS_EDGE = 0,
	TRI_NEG_EDGE = 1
}tTRI;

/*
** Defines structure for configuring pcm
*/
 typedef struct {
	tPcmFormat pcmFormat;
	tPHCF pcmHwy;	
	tTRI pcm_tri;
} vdaa_PCM_Cfg;

/*
** This defines names for the AC impedance range
*/
 typedef enum {
	AC_600,					/*	600 Ohms */
	AC_900,					/*	900 O */
	AC_270__750_150,		/*	270 O + (750 O || 150 nF) and 275 O + (780 O || 150 nF) */
	AC_220__820_120,		/*	220 O + (820 O || 120 nF) and 220 O + (820 O || 115 nF) */
	AC_370__620_310,		/*	370 O + (620 O || 310 nF) */
	AC_320__1050_230,		/*	320 O + (1050 O || 230 nF) */
	AC_370__820_110,		/*	370 O + (820 O || 110 nF) */
	AC_275__780_115,		/*	275 O + (780 O || 115 nF) */
	AC_120__820_110,		/*	120 O + (820 O || 110 nF) */
	AC_350__1000_210,		/*	350 O + (1000 O || 210 nF) */
	AC_200__680_100,		/*	200 O + (680 O || 100 nF) */
	AC_600__2160,			/*	600 O + 2.16 uF */
	AC_900__1000,			/*	900 O + 1 uF */
	AC_900__2160,			/*	900 O + 2.16 uF */
	AC_600__1000,			/*	600 O + 1 uF */
	AC_Global_impedance		/*	Global impedance */

}tAC_Z;

/*
** This defines names for the DC impedance range
*/
 typedef enum {
	 DC_50,					/*	50 Ohms dc termination is selected */
	 DC_800					/*	800 Ohms dc termination is selected */

 }tDC_Z;

 /*
** This defines names for the ringer impedance range
*/
 typedef enum {
	 RZ_MAX = 0,					
	 RZ_SYNTH = 1					

 }tRZ;

/*
** This defines names for the dc voltage adjust
*/
 typedef enum {
	 DCV3_1 = 0,
	 DCV3_2 = 1,
	 DCV3_35 = 2,
	 DCV3_5 = 3
 }tDCV;

/*
** This defines names for the minimum loop current
*/
 typedef enum {
	 MINI_10MA = 0,
	 MINI_12MA = 1,
	 MINI_14MA = 2,
	 MINI_16MA = 3
 }tMINI;

/*
** This defines names for the current limiting enable bit
*/
 typedef enum {
	 ILIM_DISABLED = 0,
	 ILIM_ENABLED = 1
 }tILIM;

/*
** This defines names for the ring detect interupt mode
*/
 typedef enum {
	 RDI_BEG_BURST = 0,
	 RDI_BEG_END_BURST = 1
 }tRDI;
/*
** This defines names for the on hook speed / spark quenching
*/
 typedef enum {
	 OHS_LESS_THAN_0_5MS = 0,
	 OHS_3MS = 1,
	 OHS_26MS = 0xE
 }tOHS;

 /*
** This defines names for the hbe bit
*/
 typedef enum {
	HYBRID_ENABLE = 1,
	HYBRID_DISABLE = 0
 }tHBE;

/*
** Defines structure for configuring impedence
*/
 typedef struct {
	tRZ rz;
	tDC_Z dcr; 
	tAC_Z acim;
	uInt8 hyb1;
	uInt8 hyb2;
	uInt8 hyb3;
	uInt8 hyb4;
	uInt8 hyb5;
	uInt8 hyb6;
	uInt8 hyb7;
	uInt8 hyb8;
	tDCV dcv;
	tMINI mini;
	tILIM ilim;
	tOHS ohs_sq;
	tHBE hbe;
} vdaa_Impedance_Cfg;

/*
** This defines names for the TX/RX Gain(2) or Attenuation(2) range
*/
 typedef enum {
	GN_0,				/*	0	dB gain or attenuation is applied to the transmit path. */
	GN_1,				/*	1	dB gain is applied to the transmit path. */
	GN_2,				/*	2	dB gain is applied to the transmit path. */
	GN_3,				/*	3	dB gain is applied to the transmit path. */
	GN_4,				/*	4	dB gain is applied to the transmit path. */
	GN_5,				/*	5	dB gain is applied to the transmit path. */
	GN_6,				/*	6	dB gain is applied to the transmit path. */
	GN_7,				/*	7	dB gain is applied to the transmit path. */
	GN_8,				/*	8	dB gain is applied to the transmit path. */
	GN_9,				/*	9	dB gain is applied to the transmit path. */
	GN_10,				/*	10	dB gain is applied to the transmit path. */
	GN_11,				/*	11	dB gain is applied to the transmit path. */
	GN_12,				/*	12	dB gain is applied to the transmit path. */
	ATTN_1,				/*	1	dB attenuation is applied to the transmit path. */
	ATTN_2,				/*	2	dB attenuation is applied to the transmit path. */
	ATTN_3,				/*	3	dB attenuation is applied to the transmit path. */
	ATTN_4,				/*	4	dB attenuation is applied to the transmit path. */
	ATTN_5,				/*	5	dB attenuation is applied to the transmit path. */
	ATTN_6,				/*	6	dB attenuation is applied to the transmit path. */
	ATTN_7,				/*	7	dB attenuation is applied to the transmit path. */
	ATTN_8,				/*	8	dB attenuation is applied to the transmit path. */
	ATTN_9,				/*	9	dB attenuation is applied to the transmit path. */
	ATTN_10,			/*	10	dB attenuation is applied to the transmit path. */
	ATTN_11,			/*	11	dB attenuation is applied to the transmit path. */
	ATTN_12,			/*	12	dB attenuation is applied to the transmit path. */
	ATTN_13,			/*	13	dB attenuation is applied to the transmit path. */
	ATTN_14,			/*	14	dB attenuation is applied to the transmit path. */
	ATTN_15				/*	15	dB attenuation is applied to the transmit path. */

 }tacgn_attn2;

/*
** This defines names for the TX/RX Gain(3) or Attenuation(3) range
*/
 typedef enum {
	GN_0_0,				/*	0.0	dB gain or attenuation is applied to the transmit path. */
	GN_0_1,				/*	0.1	dB gain is applied to the transmit path. */
	GN_0_2,				/*	0.2	dB gain is applied to the transmit path. */
	GN_0_3,				/*	0.3	dB gain is applied to the transmit path. */
	GN_0_4,				/*	0.4	dB gain is applied to the transmit path. */
	GN_0_5,				/*	0.5	dB gain is applied to the transmit path. */
	GN_0_6,				/*	0.6	dB gain is applied to the transmit path. */
	GN_0_7,				/*	0.7	dB gain is applied to the transmit path. */
	GN_0_8,				/*	0.8	dB gain is applied to the transmit path. */
	GN_0_9,				/*	0.9	dB gain is applied to the transmit path. */
	GN_1_0,				/*	1.0	dB gain is applied to the transmit path. */
	GN_1_1,				/*	1.1	dB gain is applied to the transmit path. */
	GN_1_2,				/*	1.2	dB gain is applied to the transmit path. */
	GN_1_3,				/*	1.3	dB gain is applied to the transmit path. */
	GN_1_4,				/*	1.4	dB gain is applied to the transmit path. */
	GN_1_5,				/*	1.5	dB gain is applied to the transmit path. */
	ATTN_0_1,			/*	0.1	dB attenuation is applied to the transmit path. */
	ATTN_0_2,			/*	0.2	dB attenuation is applied to the transmit path. */
	ATTN_0_3,			/*	0.3	dB attenuation is applied to the transmit path. */
	ATTN_0_4,			/*	0.4	dB attenuation is applied to the transmit path. */
	ATTN_0_5,			/*	0.5	dB attenuation is applied to the transmit path. */
	ATTN_0_6,			/*	0.6	dB attenuation is applied to the transmit path. */
	ATTN_0_7,			/*	0.7	dB attenuation is applied to the transmit path. */
	ATTN_0_8,			/*	0.8	dB attenuation is applied to the transmit path. */
	ATTN_0_9,			/*	0.9	dB attenuation is applied to the transmit path. */
	ATTN_1_0,			/*	1.0	dB attenuation is applied to the transmit path. */
	ATTN_1_1,			/*	1.1	dB attenuation is applied to the transmit path. */
	ATTN_1_2,			/*	1.2	dB attenuation is applied to the transmit path. */
	ATTN_1_3,			/*	1.3	dB attenuation is applied to the transmit path. */
	ATTN_1_4,			/*	1.4	dB attenuation is applied to the transmit path. */
	ATTN_1_5			/*	1.5	dB attenuation is applied to the transmit path. */

 }tacgn_attn3;

/*
** Defines structure for configuring audio gain
*/

 typedef struct {
	uInt8		mute;
	uInt8		gainCtrl2;
	tacgn_attn2	acgain2;
	uInt8		gainCtrl3;
	tacgn_attn3	acgain3;
	uInt8		callProgress;

} vdaa_audioGain_Cfg;


/*
** This defines names for the ring delay setting
*/
 typedef enum {
	 RDLY_0MS = 0,
	 RDLY_256MS = 1,
	 RDLY_512MS = 2,
	 RDLY_768MS = 3,
	 RDLY_1024MS = 4,
	 RDLY_1280MS = 5,
	 RDLY_1536MS = 6,
	 RDLY_1792MS = 7
 }tRDLY;

/*
** This defines names for the ring timeouts
*/
 typedef enum {
	 RTO_128MS = 1,
	 RTO_256MS = 2,
	 RTO_384MS = 3,
	 RTO_512MS = 4,
	 RTO_640MS = 5,
	 RTO_768MS = 6,
	 RTO_896MS = 7,
	 RTO_1024MS = 8,
	 RTO_1152MS = 9,
	 RTO_1280MS = 10,
	 RTO_1408MS = 11,
	 RTO_1536MS = 12,
	 RTO_1664MS = 13,
	 RTO_1792MS = 14,
	 RTO_1920MS = 15
 }tRTO;

 /*
** This defines names for the ring timeouts
*/
 typedef enum {
	 RCC_100MS = 0,
	 RCC_150MS = 1,
	 RCC_200MS = 2,
	 RCC_256MS = 3,
	 RCC_384MS = 4,
	 RCC_512MS = 5,
	 RCC_640MS = 6,
	 RCC_1024MS = 7
 }tRCC;

  /*
** This defines names for the ring validation modes
*/
 typedef enum {
	 RNGV_DISABLED = 0,
	 RNGV_ENABLED = 1
 }tRNGV;

/*
** This defines names for the rfwe bit
*/
 typedef enum {
	 RFWE_HALF_WAVE = 0,
	 RFWE_FULL_WAVE = 1,
	 RFWE_RNGV_RING_ENV = 0,
	 RFWE_RNGV_THRESH_CROSS = 1
 }tRFWE;

 /*
** This defines names for the rt and rt2 bit
*/
 typedef enum {
	 RT__13_5VRMS_16_5VRMS = 0,
	 RT__19_35VRMS_23_65VRMS = 1,
	 RT__40_5VRMS_49_5VRMS = 3
 }tRT;

  /*
** This defines names for the rt and rt2 bit
*/
 typedef enum {
	 RGDT_ACTIVE_LOW = 0,
	 RGDT_ACTIVE_HI = 1
 }tRPOL;

/*
** Defines structure for configuring ring detect config
*/

typedef struct {
	tRDLY rdly;
	tRT rt;
	uInt8 rmx;
	tRTO rto;
	tRCC rcc;
	tRNGV rngv;
	uInt8 ras;
	tRFWE rfwe;
	tRDI rdi;
	tRPOL rpol;
} vdaa_Ring_Detect_Cfg;


/*
** This defines names for the interrupts
*/
 typedef enum {
	POLI,
	TGDI,
	LCSOI,
	DODI,
	BTDI,
	FTDI,
	ROVI,
	RDTI,
	CVI		/* Current/Voltage Interrupt REGISTER#44 */
}vdaaInt;


/*
** Defines structure of interrupt data
*/
typedef struct {
	vdaaInt *irqs;
	uInt8 number;
} vdaaIntType;

/*
** This defines names for the idl bit
*/
 typedef enum {
	 IDL_DISABLED = 0,
	 IDL_ENABLED = 1
 }tIDL;

 /*
** This defines names for the ddl bit
*/
 typedef enum {
	 DDL_NORMAL_OPERATION = 0,
	 DDL_PCM_LOOPBACK = 1
 }tDDL;

/*
** Defines structure for configuring Loop Back
*/
typedef struct {
	tIDL isoDigLB;
	tDDL digDataLB;
} vdaa_Loopback_Cfg;


/*
** This defines names for the interrupt pin modes
*/
 typedef enum {
	 INTE_ANALOG = 0,
	 INTE_INTERRUPT = 1
 }tInte;

 /*
** This defines names for the interrupt pin polarities
*/
 typedef enum {
	 INTE_ACTIVE_LOW = 0,
	 INTE_ACTIVE_HIGH = 1
 }tIntePol;

/*
** This defines names for the pwm settings
*/
 typedef enum {
	PWM_DELTA_SIGMA = 0,
	PWM_CONVENTIONAL_16KHZ = 1,
	PWM_CONVENTIONAL_32KHZ = 2
 }tPwmMode;


/*
** Defines structure for configuring daa general parameters
*/
typedef struct {
	tInte inte;
	tIntePol intp;
	tPwmMode pwmm;
	BOOLEAN pwmEnable;
} vdaa_General_Cfg;

typedef enum {
	VDAA_BIT_SET = 1,
	VDAA_BIT_CLEAR = 0
} tVdaaBit;

/*
** Defines structure for daa current status (ring detect/hook stat)
*/
typedef struct {
	tVdaaBit ringDetectedNeg;
	tVdaaBit ringDetectedPos;
	tVdaaBit ringDetected;
	tVdaaBit offhook;
	tVdaaBit onhookLineMonitor;
} vdaaRingDetectStatusType;

/*
** This is the main  VoiceDAA channel object
*/
typedef struct	{
		SiVoiceDeviceType* deviceId;
		uInt8 channel;
		uInt8 channelType;
		int error;
		int debugMode;
		int channelEnable;
} vdaaChanType;

/*
** This is the main  VoiceDAA interface object pointer
*/
typedef vdaaChanType *vdaaChanType_ptr;

/*
** Defines initialization data structures
*/
typedef struct {
	uInt8 address;
	uInt8 initValue;
} vdaaRegInit;

/* Voice DAA Hook states */
enum {
VDAA_DIG_LOOPBACK,
VDAA_ONHOOK,
VDAA_OFFHOOK,
VDAA_ONHOOK_MONITOR
};

/*
** These are the error codes for ProSLIC/VoiceDAA failures
*/
typedef enum {
	ERR_VDAA_NONE,
	ERR_VDAA_SPIFAIL,
	ERR_VDAA_INVALID_INPUT,
	ERR_VDAA_BROADCAST_FAIL,
	ERR_VDAA_CHANNEL_TYPE,
	ERR_VDAA_ILOOP_OVERLOAD
} vdaaErrorCode;



int Vdaa_createDevice (SiVoiceDeviceType **pDevice);
int Vdaa_createChannel (vdaaChanType_ptr *pVdaa);
int Vdaa_destroyChannel (vdaaChanType_ptr *pVdaa);
int Vdaa_destroyDevice (SiVoiceDeviceType **pDevice);

int Vdaa_SWInitChan (vdaaChanType_ptr pVdaa,int32 channel,int chipType, SiVoiceDeviceType*deviceObj,SiVoiceControlInterfaceType *pCtrlIntf);
int Vdaa_setSWDebugMode (vdaaChanType_ptr pVdaa, int32 debugEn);
int Vdaa_getErrorFlag (vdaaChanType_ptr pVdaa, int32*error);
int Vdaa_clearErrorFlag (vdaaChanType_ptr pVdaa);
int Vdaa_setChannelEnable (vdaaChanType_ptr pVdaa, int32 chanEn);
int Vdaa_getChannelEnable (vdaaChanType_ptr pVdaa, int32* chanEn);
int Vdaa_PrintDebugData (vdaaChanType *pVdaa);
int Vdaa_VerifyControlInterface (vdaaChanType *pVdaa);
/*
** Function: VDAA_Init
**
** Description: 
** Initializes the Vdaa
**
** Input Parameters: 
** pVdaa: pointer to VDAA object
**
** Return:
** none
*/
int Vdaa_Init (vdaaChanType_ptr *pVdaa,int size);


/*
** Function: VDAA_EnableInterrupts
**
** Description: 
** Enables interrupts
**
** Input Parameters: 
** hVdaa: pointer to Vdaa object
** size: Array Length
** 
** Return:
** 
*/
int Vdaa_EnableInterrupts (vdaaChanType *pVdaa);




/*
** Function: VDAA_RingDetectSetup
**
** Description: 
** configure ringing
**
** Input Parameters: 
** pVdaa: pointer to Vdaa object
** pRingSetup: pointer to ringing config structure
**
** Return:
** none
*/
int Vdaa_RingDetectSetup (vdaaChanType *pVdaa, int32 preset);


/*
** Function: VDAA_ImpedanceSetup
**
** Description: 
** configure impedence synthesis
**
** Input Parameters: 
** pVdaa: pointer to Vdaa object
** pZynth: pointer to impedance config structure
**
** Return:
** none
*/
int Vdaa_ImpedanceSetup (vdaaChanType *pVdaa, int32 preset);

/*
** Function: VDAA_LoopbackSetup
**
** Description: 
** configure loopback
**
** Input Parameters: 
** pVdaa: pointer to Vdaa object
** preset: index to preset table
**
** Return:
** none
*/
int Vdaa_LoopbackSetup (vdaaChanType *pVdaa, int32 preset);


/*
** Function: VDAA_AudioGainSetup
**
** Description: 
** configure audio gains
**
** Input Parameters: 
** pVdaa: pointer to Vdaa object
** pAudio: pointer to audio gains config structure
**
** Return:
** none
*/
int Vdaa_TXAudioGainSetup (vdaaChanType *pVdaa, int32 preset);
int Vdaa_RXAudioGainSetup (vdaaChanType *pVdaa, int32 preset);


/*
** Function: VDAA_PCMSetup
**
** Description: 
** configure pcm
**
** Input Parameters: 
** pVdaa: pointer to Vdaa object
** rxcount: receive counter
** txcount: transmit counter
** Return:
** none
*/
int Vdaa_PCMSetup (vdaaChanType *pVdaa,int32 preset, int32 pcm_enable);
int Vdaa_PCMTimeSlotSetup (vdaaChanType *pVdaa, uInt16 rxcount, uInt16 txcount);




/*
** Function: VDAA_StartPCM
**
** Description: 
** Starts PCM
**
** Input Parameters: 
** pVdaa: pointer to Vdaa object
**
** Return:
** none
*/
int Vdaa_PCMStart (vdaaChanType *pVdaa);


/*
** Function: VDAA_StopPCM
**
** Description: 
** Disables PCM
**
** Input Parameters: 
** pVdaa: pointer to Vdaa object
**
** Return:
** none
*/
int Vdaa_PCMStop (vdaaChanType *pVdaa);
/*
** Function: VDAA_Reset
**
** Description: 
** Resets the VDaa
**
** Input Parameters: 
** pVdaa: pointer to VDAA object
**
** Return:
** none
*/
int Vdaa_Reset (vdaaChanType *pVdaa);
/*
** Function: Vdaa_ReadLinefeedStatus
**
** Description: 
** Read LIne Feed Status
**
** Input Parameters: 
** pVdaa: pointer to VDAA object, 
** vloop: pointer to Voltage Value
** iloop: pointer to Current Value
**
** Return:
** none
*/
int Vdaa_ReadLinefeedStatus (vdaaChanType *pVdaa,int8 *vloop, int16 *iloop);

/*
** Function: Vdaa_ReadRingDetectStatus
**
** Description: 
** Read Ring Detect Status
**
** Input Parameters: 
** pVdaa: pointer to VDAA object, 
** pStatus: pointer to status structure
**
** Return:
** none
*/
int Vdaa_ReadReadRingDetectStatus (vdaaChanType *pVdaa,vdaaRingDetectStatusType *pStatus);

/*
** Function: Vdaa_InitBroadcast
**
** Description: 
** Initiate Broadcast on all channels
**
** Input Parameters: 
** pVdaa: pointer to VDAA object, 
**
** Return:
** none
*/
int Vdaa_InitBroadcast (vdaaChanType_ptr pVdaa);
/*
** Function: Vdaa_GetInterrupts
**
** Description: 
** Get Interrupts
**
** Input Parameters: 
** pIntData: pointer to Active Interrupts , 
** pVdaa: pointer to VDAA object
**
** Return:
** none
*/
int Vdaa_GetInterrupts (vdaaChanType *pVdaa,vdaaIntType *pIntData);
/*
** Function: Vdaa_ClearInterrupts
**
** Description: 
** Clear already set Interrupts
**
** Input Parameters: 
** pVdaa: pointer to VDAA object
**
** Return:
** none
*/
int Vdaa_ClearInterrupts (vdaaChanType *pVdaa);
/*
** Function: Vdaa_SetHookStatus
**
** Description: 
** Set DAA hook status
**
** Input Parameters: 
** pVdaa: pointer to VDAA object
** newHookStatus: parameter for one of the four hook status 
** 
** Return:
** none
*/
int Vdaa_SetHookStatus (vdaaChanType *pVdaa,uInt8 newHookStatus);

/* TODO */
int Vdaa_ADCCal (vdaaChanType_ptr pVdaa, int32 size);
#endif


