#ifndef _ALC5621_H_
#define _ALC5621_H_



/*-----------------ALC5621.c function prototype-----------*/
void ALC5621_fake_read(unsigned char reg_number);
void ALC5621_software_reset(void);




void ALC5621_init( int law );

void ALC5621_loopback( unsigned char on_off );


/*-----------------Audio interface reg0x34-----------------------*/
#define I2S_MOD_MASTER		0
#define I2S_MODE_SLAVE		1

#define ADC_LEFT_PHASE		0
#define ADC_RIGHT_PHASE		1
#define DAC_LEFT_PHASE		0
#define DAC_RIGHT_PHASE		1

#define I2S_FORMAT		0
#define RIGHT_JUSTIFIED		1
#define LEFT_JUSTIFIED		2
#define DSP_PCM_MODE		3
#define DATA_L16			0
#define DATA_L20			1
#define DATA_L24			2
#define DATA_L32			3

#ifndef CONFIG_RTK_VOIP_DRIVERS_IIS
#define BCLK_POLARITY		1	//0->normal, 1->inverted
#else
#define BCLK_POLARITY		0	//0->normal, 1->inverted
#endif
#define PCM_MODE		0	//pcm mode select : 0->mode A, 1->mode B

/*-------------------------------------------------------*/


#define BCLK_FRAME_INPUTS	0
#define BCLK_FRAME_OUTPUTS	1
#define CLKSEL_MCLK			0
#define CLKSEL_PLL			1

/*-------------------- reg 0x36  -----------------------------------*/
#define ADDA_OSR_64x			0
#define ADDA_OSR_128x			1

#define ADDA_FILTER_CLK_256FS		0
#define ADDA_FILTER_CLK_384FS		1

/*-------------------------------------------------------*/



#define MIC1_SPEAKER		0
#define MIC2_MONO   		1
#define SPEAKER_ONLY		2
#define MONO_ONLY		3


#endif	//_ALC5621_H_
