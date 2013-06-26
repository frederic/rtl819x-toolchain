/*
** Copyright (c) 2011 Silicon Laboratories, Inc.
** 2011-02-09 18:53:30
**
** Si3217x ProSLIC API Configuration Tool Version 2.4.0
*/


#ifndef VDAA_CONSTANTS_H
#define VDAA_CONSTANTS_H

/** Vdaa_Country Presets */
enum {
	COU_USA,
	COU_GERMANY,
	COU_CHINA,
	COU_AUSTRALIA
};

/** Vdaa_Audio_Gain Presets */
enum {
	AUDIO_GAIN_0DB,
	AUDIO_ATTEN_4DB,
	AUDIO_ATTEN_6DB,
	AUDIO_ATTEN_11DB
};

/** Vdaa_Ring_Validation Presets */
enum {
	RING_DET_NOVAL_LOWV,
	RING_DET_VAL_HIGHV
};

/** Vdaa_PCM Presets */
enum {
	PCM_8ULAW,
	PCM_8ALAW,
	PCM_16LIN
};

/** Vdaa_Hybrid Presets */
enum {
	HYB_600_0_0_500FT_24AWG,
	HYB_270_750_150_500FT_24AWG,
	HYB_200_680_100_500FT_24AWG,
	HYB_220_820_120_500FT_24AWG
};

#endif

