#ifndef _WM8510_H_
#define _WM8510_H_

//#define _WM8510_DEBUG_

/*-----------------WM8510 data structure-----------------*/
typedef struct {
		unsigned short reg_1;
		unsigned short reg_2;
		unsigned short reg_3;
		unsigned short reg_4;
		unsigned short reg_5;
		unsigned short reg_6;
		unsigned short reg_7;
		unsigned short reg_8;
		unsigned short reg_10;
		unsigned short reg_11;
		unsigned short reg_14;
		unsigned short reg_15;
		unsigned short reg_24;
		unsigned short reg_25;
		unsigned short reg_27;
		unsigned short reg_28;
		unsigned short reg_29;
		unsigned short reg_30;
		unsigned short reg_32;
		unsigned short reg_33;
		unsigned short reg_34;
		unsigned short reg_35;
		unsigned short reg_36;
		unsigned short reg_37;
		unsigned short reg_38;
		unsigned short reg_39;
		unsigned short reg_40;
		unsigned short reg_44;
		unsigned short reg_45;
		unsigned short reg_47;
		unsigned short reg_49;
		unsigned short reg_50;
		unsigned short reg_54;
		unsigned short reg_56;
} WM8510_DEFAULT;		
/*-------------------------------------------------------*/

/*-----------------WM8510.C function prototype-----------*/
void WM8510_fake_read(unsigned char reg_number);
void WM8510_software_reset(void);
void mic_gain_control(unsigned char INPPGAVOL, unsigned char INPPGAZC);
void ADC_high_pass_filter_ctrl(unsigned char HPFEN, unsigned char HPFAPP, unsigned char HPFCUT);
void ADC_notch_filter_ctrl(unsigned char NFEN, unsigned short NFA0, unsigned short NFA1);
void ADC_volume_gain_ctrl(unsigned char ADCVOL);
void ALC_mode_setting(unsigned char ALCLVL, unsigned char ALCZC,
					  unsigned char ALCMAXGAIN, unsigned char ALCMINGAIN, unsigned char ALCHLD,
					  unsigned char ALCDCY, unsigned char ALCATK);
void ALC_Limited_mode_setting(unsigned char ALCLVL, unsigned char ALCZC,
					  	  unsigned char ALCMAXGAIN, unsigned char ALCMINGAIN,
					  	  unsigned char ALCDCY, unsigned char ALCATK);
void DAC_volume_gain_ctrl(unsigned char DACVOL);
void DAC_limiter_control(unsigned char LIMEN, unsigned char LIMLVL, 
						 unsigned char LIMATK, unsigned char LIMDCY);
void Speaker_volume_control(unsigned char SPKMUTE, unsigned char SPKVOL, unsigned char SPKZC);
void WM8510_init(int law);
/*-------------------------------------------------------*/

/*-----------------Power management control--------------*/
#define BUFFER_I_O_TIE		0
#define ANALOG_OP_BIAS		1
#define MIC_BIAS			2
/*-------------------------------------------------------*/

/*-----------------Analog input enable-------------------*/
#define ADCONVERTER			0
#define INPUT_MIC_PGA		1
#define INPUT_BOOST			2
/*-------------------------------------------------------*/

/*-----------------Notch filter coefficient--------------*/
#define NOTCH_FILTER_A0
#define NOTCH_FILTER_A1

/*-------------------------------------------------------*/

/*-----------------Output Boost control coeff.-----------*/
#define SPEAKER_OUTPUT_BOOST	0
#define MONO_OUTPUT_BOOST		1
/*-------------------------------------------------------*/

/*-----------------Speaker mixer control coeff.----------*/
#define DAC2SPEAKER 		0
#define BYPASS2SPEAKER		1
#define MIC2_2SPEAKER		2
/*-------------------------------------------------------*/

/*-----------------Mono mixer control coeff.----------*/
#define DAC2MONO 			0
#define BYPASS2MONO			1
#define MIC2_2MONO			2
/*-------------------------------------------------------*/

/*-----------------Analog output enable-------------------*/
#define DACONVERTER			0
#define SPEAKER_MIXER		1
#define MONO_MIXER			2
#define SPEAKER_OUTPUT_P	3
#define SPEAKER_OUTPUT_N	4
#define MONO_OUTPUT			5
/*-------------------------------------------------------*/

/*-----------------Audio interface-----------------------*/
#define ADC_LEFT_PHASE		0
#define ADC_RIGHT_PHASE		1
#define DAC_LEFT_PHASE		0
#define DAC_RIGHT_PHASE		1
#define RIGHT_JUSTIFIED		0
#define LEFT_JUSTIFIED		1
#define I2S_FORMAT			2
#define DSP_PCM_MODE		3
#define DATA_L16			0
#define DATA_L20			1
#define DATA_L24			2
#define DATA_L32			3
#define FRAME_POLARITY		0	//0->normal, 1->inverted
#define BCLK_POLARITY		0	//0->normal, 1->inverted
#define LINEAR_MODE			0
#define u_LAW_MODE			2
#define A_LAW_MODE			3
#define LOOPBACK_TEST		1	//0->no loopback, 1->loopback enable
#define AUDIO_FS_48kHZ		0
#define AUDIO_FS_32kHZ		1
#define AUDIO_FS_24kHZ		2
#define AUDIO_FS_16kHZ		3
#define AUDIO_FS_12kHZ		4
#define AUDIO_FS_8kHZ		5
#define BCLK_FRAME_INPUTS	0
#define BCLK_FRAME_OUTPUTS	1
#define CLKSEL_MCLK			0
#define CLKSEL_PLL			1
#define ADCOSR_64x			0
#define ADCOSR_128x			1
#define DACOSR_64x			0
#define DACOSR_128x			1
/*-------------------------------------------------------*/

#define MIC1_SPEAKER		0
#define MIC2_MONO   		1
#define SPEAKER_ONLY		2
#define MONO_ONLY			3

#endif	//_WM8510_H_
