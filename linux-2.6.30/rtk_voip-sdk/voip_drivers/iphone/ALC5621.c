/*
 *	file name: ALC5621.c
 *
 */

#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/delay.h>  	// udelay
#include "ALC5621.h"
#include "base_i2c_ALC5621.h"

#define DEBUGPRINT_ALC5621	printk
#define ALC5621_POLLING_GPIO
#define TC_SET

#if defined( CONFIG_RTK_VOIP_GPIO_IPP_8972_V00 ) || defined( CONFIG_RTK_VOIP_GPIO_IPP_8972_V01 )
  #define RIGHT_PHASE
  #define HANDSET_MIC2
#elif defined( CONFIG_RTK_VOIP_GPIO_IPP_8952_V00 ) || defined( CONFIG_RTK_VOIP_GPIO_IPP_8972B_V00 ) || defined( CONFIG_RTK_VOIP_GPIO_IPP_8972B_V99 )
  #define LEFT_PHASE
  #define HANDSET_MIC1
#else
	???
#endif

#ifdef RIGHT_PHASE
  #define SL1( l1, r1 )								l1, r1
  #define SL2( l1, l2, r1, r2 )						l1, l2, r1, r2
  #define SL3( l1, l2, l3, r1, r2, r3 )				l1, l2, l3, r1, r2, r3
  #define SL4( l1, l2, l3, l4, r1, r2, r3, r4 )		l1, l2, l3, l4, r1, r2, r3, r4
#elif defined( LEFT_PHASE )
  #define SL1( l1, r1 )								r1, l1
  #define SL2( l1, l2, r1, r2 )						r1, r2, l1, l2 
  #define SL3( l1, l2, l3, r1, r2, r3 )				r1, r2, r3, l1, l2, l3 
  #define SL4( l1, l2, l3, l4, r1, r2, r3, r4 )		r1, r2, r3, r4, l1, l2, l3, l4 
#else
  ???
#endif

#ifdef HANDSET_MIC2
  #define MIC_SL1( mic1, mic2 )						mic1, mic2
#elif defined( HANDSET_MIC1 )
  #define MIC_SL1( mic1, mic2 )						mic2, mic1
#endif

//---------------------static function prototype-------------------------------//

static void ADDA_oversample_rate(unsigned short int ADDA_OSR);
static void Audio_interface_setting(unsigned short int I2S_MOD_SEL,unsigned short int FMT, unsigned short int WL, unsigned short int ADCLRSWAP, unsigned short int DACLRSWAP);
static void Speaker_volume_control(unsigned short int SP_L_MUTE, unsigned short int SP_L_DEZERO,unsigned short int SP_L_SOFTMUTE, unsigned short int SP_L_VOL,
				unsigned short int SP_R_MUTE, unsigned short int SP_R_DEZERO,unsigned short int SP_R_SOFTMUTE, unsigned short int SP_R_VOL);
static void Headphone_volume_control(unsigned short int HP_L_MUTE, unsigned short int HP_L_DEZERO,unsigned short int HP_L_SOFTMUTE, unsigned short int HP_L_VOL,
				unsigned short int HP_R_MUTE, unsigned short int HP_R_DEZERO,unsigned short int HP_R_SOFTMUTE, unsigned short int HP_R_VOL);
static void Auxout_volume_control(unsigned short int AUXO_L_MUTE, unsigned short int AUXO_L_DEZERO,unsigned short int AUXO_L_SOFTMUTE, unsigned short int AUXO_L_VOL,
				unsigned short int AUXO_R_MUTE, unsigned short int AUXO_R_DEZERO,unsigned short int AUXO_R_SOFTMUTE, unsigned short int AUXO_R_VOL);
static void Auxin_volume_control(unsigned short int AUXI2HP_MUTE, unsigned short int AUXI2SPK_MUTE,unsigned short int AUXI2MONO_MUTE, unsigned short int AUXI_L_VOL,
				unsigned short int AUXI_R_VOL);
static void Linein_volume_control(unsigned short int LI2HP_MUTE, unsigned short int LI2SPK_MUTE,unsigned short int LI2MONO_MUTE, unsigned short int LI_L_VOL,
				unsigned short int LI_R_VOL);
static void Stereo_dac_volume_control(unsigned short int DACMUTE, unsigned short int DAC_L_VOL, unsigned short int DAC_R_VOL);
static void MIC_volume_control(unsigned short int MIC1_VOL, unsigned short int MIC2_VOL);
static void MIC_routing_control(unsigned short int MIC1_ROUTE_SET, unsigned short int MIC2_ROUTE_SET);
static void ADC_record_gain_control(unsigned short int ADC2MIXER, unsigned short int ADC_L_VOL, unsigned short int ADC_L_DEZERO, unsigned short int ADC_R_VOL, unsigned short int ADC_R_DEZERO);
static void ADC_mixer_control(unsigned short int ADCREC_L_MUTE, unsigned short int ADCREC_R_MUTE);
static void Output_mixer_control(unsigned short int SETTING);
static void Microphone_control(unsigned short int SETTING);
static void ADDA_clock_control(unsigned short int I2S_PRE_DIV, unsigned short int I2S_SCLK_DIV, unsigned short int I2S_WCLK_DIV_PRE, 
				unsigned short int I2S_WCLK_DIV, unsigned short int ADDA_FILTER_CLK );
static void Power_management_a1(unsigned short int SETTING);
static void Power_management_a2(unsigned short int SETTING);
static void Power_management_a3(unsigned short int SETTING);
static void Additional_control(unsigned short int SETTING);
static void Global_clock_control(unsigned short int SETTING);
static void PLL_control(unsigned short int PLL_N_CODE, unsigned short int PLL_M_BYPASS, unsigned short int PLL_K_CODE,
				unsigned short int PLL_M_CODE);
static void GPIO_Output_control(unsigned short int GPIO_OUT_HL);
static void GPIO_config(unsigned short int GPIO_CONF);
static unsigned short int GPIO_status(void);
static void PIN_sharing(unsigned short int SETTING);
static void Misc_control(unsigned short int SETTING);
static unsigned short int Vendor_ID1(void);
static unsigned short int Vendor_ID2(void);

int iphone_handfree;
void AI_AO_select(unsigned char type);
//-----------------------------------------------------------------------------//

void ALC5621_fake_read(unsigned char reg_number)
{

	printk("register %xh = 0x%x\n", reg_number, read_ALC5621(reg_number));

}

void ALC5621_software_reset(void)
{
	write_ALC5621(0,20);
	printk("Reset ALC5621 ");

}


//ADDA oversample rate select. ADDA_OSR:0->64x, 1->128x.
static void ADDA_oversample_rate(unsigned short int ADDA_OSR)
{
	unsigned short int buf;
	
	buf=read_ALC5621(0x36);

	DEBUGPRINT_ALC5621("Original reg_36h=0x%x \n",buf);
	
	
	buf = (buf & 0xfffe) | (ADDA_OSR);
	write_ALC5621(0x36, buf );
	
	if(ADDA_OSR)
		DEBUGPRINT_ALC5621("ADDA oversample rate 128x \n");
	else
		DEBUGPRINT_ALC5621("ADDA oversample rate 64x \n");

	DEBUGPRINT_ALC5621("Modified reg_36h=0x%x\n", buf);

}

//I2S_MOD_SEL: i2s data mode: 0->Master, 1:Slave
//The polarity of FRAME sync. and BCLK is kept normal.
//FMT:0->I2S format, 1->right justified, 2->left justified, 3->DSP/PCM mode.
//WL(data length):0->16 bits, 1->20 bits, 2->24 bits, 3->32 bits.
//ADCLRSWAP:0->ADC data appear in "left" phase of FRAME, 1->ADC data appear in "right" phase of FRAME.
//DACLRSWAP:0->DAC data appear in "left" phase of FRAME, 1->DAC data appear in "right" phase of FRAME.
static void Audio_interface_setting(unsigned short int I2S_MOD_SEL,unsigned short int FMT, unsigned short int WL, unsigned short int ADCLRSWAP, unsigned short int DACLRSWAP)
{
	unsigned short int buf;

	buf=read_ALC5621(0x34);

	DEBUGPRINT_ALC5621("Original reg_34h=0x%x \n",buf);
	
	buf = (I2S_MOD_SEL << 15) | (PCM_MODE <<14) | (BCLK_POLARITY <<7) | (ADCLRSWAP <<5) | (DACLRSWAP <<4)
		 | (WL <<2) | (FMT);

	write_ALC5621(0x34, buf);

	if (I2S_MOD_SEL ==0)
		DEBUGPRINT_ALC5621("i2s data mode is Master,");
	else if(I2S_MOD_SEL ==1)
		DEBUGPRINT_ALC5621("i2s data mode is Slave,");

	if(PCM_MODE == 0)
		DEBUGPRINT_ALC5621("pcm mode is A,");
	else
		DEBUGPRINT_ALC5621("pcm mode is B,");

	if (!BCLK_POLARITY)
		DEBUGPRINT_ALC5621("BCLK polarity is normal \n");
	else
		DEBUGPRINT_ALC5621("BCLK polarity is inverted \n");

	if (ADCLRSWAP)
		DEBUGPRINT_ALC5621("ADC data appear in 'right' phase of FRAME, ");
	else
		DEBUGPRINT_ALC5621("ADC data appear in 'left' phase of FRAME, ");

	if (DACLRSWAP)
		DEBUGPRINT_ALC5621("DAC data appear in 'right' phase of FRAME, \n");
	else
		DEBUGPRINT_ALC5621("DAC data appear in 'left' phase of FRAME, \n");

	if (FMT == 0)
		DEBUGPRINT_ALC5621("Audio interface is I2S format, ");
	else if (FMT == 1)
		DEBUGPRINT_ALC5621("Audio interface is Right Justified, ");
	else if (FMT == 2)
		DEBUGPRINT_ALC5621("Audio interface is Left Justified, ");
	else if (FMT == 3)
		DEBUGPRINT_ALC5621("Audio interface is DSP/PCM mode, ");

	if (WL == 0)
		DEBUGPRINT_ALC5621("Data length 16 bits, \n");
	else if (WL == 1)
		DEBUGPRINT_ALC5621("Data length 20 bits, \n");
	else if (WL == 2)
		DEBUGPRINT_ALC5621("Data length 24 bits, \n");
	else if (WL == 3)
		DEBUGPRINT_ALC5621("Data length 32 bits, \n");
		
	printk("Modified reg_34h=0x%x\n",buf);


	return;
}

//SP_L_MUTE: 0-> spk left on, 1-> mute spk left
//SP_L_DEZERO: 0-> spk left disable zero cross detect, 1-> spk left enalbe zero cross detect
//SP_L_SOFTMUTE: 0->spk left softmute disable, 1-> spk left softmute enable
//SP_L_VOL[4:0]: 00h->0dB, 1Fh->-46.5dB
//SP_R_MUTE: 0-> spk right on, 1-> mute spk right
//SP_R_DEZERO: 0-> spk right disable zero cross detect, 1-> spk right enalbe zero cross detect
//SP_R_SOFTMUTE: 0->spk right softmute disable, 1-> spk right softmute enable
//SP_R_VOL[4:0]: 00h->0dB, 1Fh->-46.5dB
static void Speaker_volume_control(unsigned short int SP_L_MUTE, unsigned short int SP_L_DEZERO,unsigned short int SP_L_SOFTMUTE, unsigned short int SP_L_VOL,
				unsigned short int SP_R_MUTE, unsigned short int SP_R_DEZERO,unsigned short int SP_R_SOFTMUTE, unsigned short int SP_R_VOL)
{
	unsigned short int buf;

	buf=read_ALC5621(0x02);

	DEBUGPRINT_ALC5621("Original reg_02h=0x%x \n",buf);

	buf = (SP_L_MUTE<<15)|(SP_L_DEZERO<<14)|(SP_L_SOFTMUTE<<13)| (SP_L_VOL<<8)
	     |(SP_R_MUTE<<7) |(SP_R_DEZERO<<6) |(SP_R_SOFTMUTE<<5)| SP_R_VOL;

	write_ALC5621(0x02, buf);

	printk("Modified reg_02h=0x%x\n",buf);


	return;

}

//HP_L_MUTE: 0-> headphone left on, 1-> mute headphone left
//HP_L_DEZERO: 0-> headphone left disable zero cross detect, 1-> headphone left enalbe zero cross detect
//HP_L_SOFTMUTE: 0->headphone left softmute disable, 1-> headphone left softmute enable
//HP_L_VOL[4:0]: 00h->0dB, 1Fh->-46.5dB
//HP_R_MUTE: 0-> headphone right on, 1-> mute headphone right
//HP_R_DEZERO: 0-> headphone right disable zero cross detect, 1-> headphone right enalbe zero cross detect
//HP_R_SOFTMUTE: 0->headphone right softmute disable, 1-> headphone right softmute enable
//HP_R_VOL[4:0]: 00h->0dB, 1Fh->-46.5dB
static void Headphone_volume_control(unsigned short int HP_L_MUTE, unsigned short int HP_L_DEZERO,unsigned short int HP_L_SOFTMUTE, unsigned short int HP_L_VOL,
				unsigned short int HP_R_MUTE, unsigned short int HP_R_DEZERO,unsigned short int HP_R_SOFTMUTE, unsigned short int HP_R_VOL)
{
	unsigned short int buf;

	buf=read_ALC5621(0x04);

	DEBUGPRINT_ALC5621("Original reg_04h=0x%x \n",buf);

	buf = (HP_L_MUTE<<15)|(HP_L_DEZERO<<14)|(HP_L_SOFTMUTE<<13)| (HP_L_VOL<<8)
	     |(HP_R_MUTE<<7) |(HP_R_DEZERO<<6) |(HP_R_SOFTMUTE<<5)| HP_R_VOL;

	write_ALC5621(0x04, buf);

	printk("Modified reg_04h=0x%x\n",buf);


	return;

}

//AUXO_L_MUTE: 0-> auxout left on, 1-> mute auxout left
//AUXO_L_DEZERO: 0-> auxout left disable zero cross detect, 1-> auxout left enalbe zero cross detect
//AUXO_L_SOFTMUTE: 0->auxout left softmute disable, 1-> auxout left softmute enable
//AUXO_L_VOL[4:0]: 00h->0dB, 1Fh->-46.5dB
//AUXO_R_MUTE: 0-> auxout right on, 1-> mute auxout right
//AUXO_R_DEZERO: 0-> auxout right disable zero cross detect, 1-> auxout right enalbe zero cross detect
//AUXO_R_SOFTMUTE: 0->auxout right softmute disable, 1-> auxout right softmute enable
//AUXO_R_VOL[4:0]: 00h->0dB, 1Fh->-46.5dB
static void Auxout_volume_control(unsigned short int AUXO_L_MUTE, unsigned short int AUXO_L_DEZERO,unsigned short int AUXO_L_SOFTMUTE, unsigned short int AUXO_L_VOL,
				unsigned short int AUXO_R_MUTE, unsigned short int AUXO_R_DEZERO,unsigned short int AUXO_R_SOFTMUTE, unsigned short int AUXO_R_VOL)
{
	unsigned short int buf;

	buf=read_ALC5621(0x06);

	DEBUGPRINT_ALC5621("Original reg_06h=0x%x \n",buf);

	buf = (AUXO_L_MUTE<<15)|(AUXO_L_DEZERO<<14)|(AUXO_L_SOFTMUTE<<13)| (AUXO_L_VOL<<8)
	     |(AUXO_R_MUTE<<7) |(AUXO_R_DEZERO<<6) |(AUXO_R_SOFTMUTE<<5)| AUXO_R_VOL;

	write_ALC5621(0x06, buf);

	printk("Modified reg_06h=0x%x\n",buf);


	return;

}

//AUXI2HP_MUTE: 0->auxin output to headphone mixer, 1-> mute auxin output to headphone mixer
//AUXI2SPK_MUTE: 0-> auxin output to speaker mixer, 1-> mute auxin output to speaker mixer
//AUXI2MONO_MUTE: 0-> auxin output to mono mixer, 1-> mute auxin output to mono mixer
//AUXI_L_VOL:00h->+12dB, 08h->0db, 1fh -34.5db, 1.5db step
//AUXI_R_VOL:00h->+12dB, 08h->0db, 1fh -34.5db, 1.5db step
static void Auxin_volume_control(unsigned short int AUXI2HP_MUTE, unsigned short int AUXI2SPK_MUTE,unsigned short int AUXI2MONO_MUTE, unsigned short int AUXI_L_VOL,
				unsigned short int AUXI_R_VOL)
{
	unsigned short int buf;

	buf=read_ALC5621(0x08);

	DEBUGPRINT_ALC5621("Original reg_08h=0x%x \n",buf);

	buf = (AUXI2HP_MUTE<<15)|(AUXI2SPK_MUTE<<14)|(AUXI2MONO_MUTE<<13)| (AUXI_L_VOL<<8)
	     | AUXI_R_VOL;

	write_ALC5621(0x08, buf);

	printk("Modified reg_08h=0x%x\n",buf);


	return;

}

//LI2HP_MUTE: 0->line_in output to headphone mixer, 1-> mute line_in output to headphone mixer
//LI2SPK_MUTE: 0-> line_in output to speaker mixer, 1-> mute line_in output to speaker mixer
//LI2MONO_MUTE: 0-> line_in output to mono mixer, 1-> mute line_in output to mono mixer
//LI_L_VOL:00h->+12dB, 08h->0db, 1fh -34.5db, 1.5db step
//LI_R_VOL:00h->+12dB, 08h->0db, 1fh -34.5db, 1.5db step
static void Linein_volume_control(unsigned short int LI2HP_MUTE, unsigned short int LI2SPK_MUTE,unsigned short int LI2MONO_MUTE, unsigned short int LI_L_VOL,
				unsigned short int LI_R_VOL)
{
	unsigned short int buf;

	buf=read_ALC5621(0x0a);

	DEBUGPRINT_ALC5621("Original reg_0ah=0x%x \n",buf);

	buf = (LI2HP_MUTE<<15)|(LI2SPK_MUTE<<14)|(LI2MONO_MUTE<<13)| (LI_L_VOL<<8)
	     | LI_R_VOL;

	write_ALC5621(0x0a, buf);

	printk("Modified reg_0ah=0x%x\n",buf);


	return;

}


//DACMUTE[2:0]: bit2 0-> volume output to headphone, 1-> mute volumte output to headphone mixer
//DACMUTE[2:0]: bit1 0-> volume output to speak, 1-> mute volumte output to speak mixer
//DACMUTE[2:0]: bit0 0-> volume output to mono, 1-> mute volumte output to mono mixer
//DAC_L_VOL[4:0]:left dac volume 00h->+12dB, 08h->0db, 1fh -34.5db, 1.5db step
//DAC_R_VOL[4:0]:right dac volume 00h->+12dB, 08h->0db, 1fh -34.5db, 1.5db step
static void Stereo_dac_volume_control(unsigned short int DACMUTE, unsigned short int DAC_L_VOL, unsigned short int DAC_R_VOL)
{
	unsigned short int buf;

	buf=read_ALC5621(0x0c);

	DEBUGPRINT_ALC5621("Original reg_0Ch=0x%x \n",buf);

	buf = (DACMUTE <<13) | (DAC_L_VOL<<8) | DAC_R_VOL;

	write_ALC5621(0x0c, buf);

	printk("Modified reg_0ch=0x%x\n",buf);


	return;

}

//DAC_L_VOL[4:0]:left dac volume 00h->+12dB, 08h->0db, 1fh -34.5db, 1.5db step
//DAC_R_VOL[4:0]:right dac volume 00h->+12dB, 08h->0db, 1fh -34.5db, 1.5db step
static void Stereo_dac_volume_control_VOL( unsigned short int DAC_L_VOL, unsigned short int DAC_R_VOL)
{
	unsigned short int buf;

	buf=read_ALC5621(0x0c);

	DEBUGPRINT_ALC5621("Original reg_0Ch=0x%x \n",buf);

	buf &= 0xe000;
	buf |= (DAC_L_VOL<<8) | DAC_R_VOL;

	write_ALC5621(0x0c, buf);

	printk("Modified reg_0ch=0x%x\n",buf);


	return;

}

//DACMUTE[2:0]: bit2 0-> volume output to headphone, 1-> mute volumte output to headphone mixer
//DACMUTE[2:0]: bit1 0-> volume output to speak, 1-> mute volumte output to speak mixer
//DACMUTE[2:0]: bit0 0-> volume output to mono, 1-> mute volumte output to mono mixer
void Stereo_dac_volume_control_MUTE(unsigned short int DACMUTE)
{
	unsigned short int buf;

	buf=read_ALC5621(0x0c);

	DEBUGPRINT_ALC5621("Original reg_0Ch=0x%x \n",buf);

	buf &= 0x1fff;
	buf |= (DACMUTE <<13);

	write_ALC5621(0x0c, buf);

	printk("Modified reg_0ch=0x%x\n",buf);


	return;

}

//MIC1_VOL[4:0]: 00h->+12dB, 08h->0dB, 1Fh->-34.5dB
//MIC2_VOL[4:0]: 00h->+12dB, 08h->0dB, 1Fh->-34.5dB
static void MIC_volume_control(unsigned short int MIC1_VOL, unsigned short int MIC2_VOL)
{
	unsigned short int buf;

	buf=read_ALC5621(0x0e);

	DEBUGPRINT_ALC5621("Original reg_0eh=0x%x \n",buf);

	buf = (MIC1_VOL<<8) | MIC2_VOL;

	write_ALC5621(0x0e, buf);

	printk("Modified reg_0eh=0x%x\n",buf);


	return;

}


static void MIC_routing_control(unsigned short int MIC1_ROUTE_SET, unsigned short int MIC2_ROUTE_SET)
{

	unsigned short int buf;

	buf=read_ALC5621(0x10);

	DEBUGPRINT_ALC5621("Original reg_10h=0x%x \n",buf);

	buf = (MIC1_ROUTE_SET<<12) | (MIC2_ROUTE_SET<<4);

	write_ALC5621(0x10, buf);

	printk("Modified reg_10h=0x%x\n",buf);


	return;

}


//ADC2MIXER[3:0]: bit3 0-> adc left output to headphone mixer, 1-> mute adc left output to headphone mixer
//ADC2MIXER[3:0]: bit2 0-> adc right output to headphone mixer, 1-> mute adc right output to headphone mixer
//ADC2MIXER[3:0]: bit1 0-> adc left output to mono mixer, 1-> mute adc left output to mono mixer
//ADC2MIXER[3:0]: bit0 0-> adc right output to mono mixer, 1-> mute adc right output to mono mixer
//ADC_L_VOL[4:0]: 00h->-16.5db, 0Bh->0dB, 1fh->+30dB
//ADC_L_DEZERO: 0-> disable detect zero cross, 1->enable detect zero cross
//ADC_R_VOL[4:0]: 00h->-16.5db, 0Bh->0dB, 1fh->+30dB
//ADC_R_DEZERO: 0-> disable detect zero cross, 1->enable detect zero cross
static void ADC_record_gain_control(unsigned short int ADC2MIXER, unsigned short int ADC_L_VOL, unsigned short int ADC_L_DEZERO, unsigned short int ADC_R_VOL, unsigned short int ADC_R_DEZERO)
{
	unsigned short int buf;

	buf=read_ALC5621(0x12);

	DEBUGPRINT_ALC5621("Original reg_12h=0x%x \n",buf);

	buf = (ADC2MIXER <<12) | (ADC_L_VOL<<7) | (ADC_L_DEZERO<<6) | (ADC_R_DEZERO<<5) | ADC_R_VOL;

	write_ALC5621(0x12, buf);

	printk("Modified reg_12h=0x%x\n",buf);


	return;

}


//ADCREC_L_MUTE[6:0]: bit6 0-> MIC1 input left mixer, 1-> MIC1 mute left mixer
//ADCREC_L_MUTE[6:0]: bit5 0-> MIC2 input left mixer, 1-> MIC2 mute left mixer
//ADCREC_L_MUTE[6:0]: bit4 0-> LINE_IN_L input left mixer, 1-> LINE_IN_L mute left mixer
//ADCREC_L_MUTE[6:0]: bit3 0-> AUX_IN_L input left mixer, 1-> AUX_IN_L mute left mixer
//ADCREC_L_MUTE[6:0]: bit2 0-> Headphone mixer Left input left mixer, 1-> Headphone mixer left mute left mixer
//ADCREC_L_MUTE[6:0]: bit1 0-> speaker mixer input left mixer, 1-> speaker mixer mute left mixer
//ADCREC_L_MUTE[6:0]: bit0 0-> mono mixer input left mixer, 1-> mono mixer mute left mixer
//ADCREC_R_MUTE[6:0]: bit6 0-> MIC1 input right mixer, 1-> MIC1 mute right mixer
//ADCREC_R_MUTE[6:0]: bit5 0-> MIC2 input right mixer, 1-> MIC2 mute right mixer
//ADCREC_R_MUTE[6:0]: bit4 0-> LINE_IN_R input right mixer, 1-> LINE_IN_R mute right mixer
//ADCREC_R_MUTE[6:0]: bit3 0-> AUX_IN_R input right mixer, 1-> AUX_IN_R mute right mixer
//ADCREC_R_MUTE[6:0]: bit2 0-> Headphone mixer right input left mixer, 1-> Headphone mixer right mute right mixer
//ADCREC_R_MUTE[6:0]: bit1 0-> speaker mixer input right mixer, 1-> speaker mixer mute right mixer
//ADCREC_R_MUTE[6:0]: bit0 0-> mono mixer input right mixer, 1-> mono mixer mute right mixer
static void ADC_mixer_control(unsigned short int ADCREC_L_MUTE, unsigned short int ADCREC_R_MUTE)
{

	unsigned short int buf;

	buf=read_ALC5621(0x14);

	DEBUGPRINT_ALC5621("Original reg_14h=0x%x \n",buf);

	buf = (ADCREC_L_MUTE <<8) |  ADCREC_R_MUTE;

	write_ALC5621(0x14, buf);

	printk("Modified reg_14h=0x%x\n",buf);


	return;

}

static void Output_mixer_control(unsigned short int SETTING)
{
	unsigned short int buf;

	buf=read_ALC5621(0x1c);

	DEBUGPRINT_ALC5621("Original reg_1ch=0x%x \n",buf);

	buf = SETTING;

	write_ALC5621(0x1c, buf);

	printk("Modified reg_1Ch=0x%x\n",buf);

	return;

}

static void Microphone_control(unsigned short int SETTING)
{
	unsigned short int buf;

	buf=read_ALC5621(0x22);

	DEBUGPRINT_ALC5621("Original reg_22h=0x%x \n",buf);

	buf = SETTING;

	write_ALC5621(0x22, buf);

	printk("Modified reg_22h=0x%x\n",buf);

	return;

}



static void ADDA_clock_control(unsigned short int I2S_PRE_DIV, unsigned short int I2S_SCLK_DIV, unsigned short int I2S_WCLK_DIV_PRE, 
				unsigned short int I2S_WCLK_DIV, unsigned short int ADDA_FILTER_CLK )
{
	unsigned short int buf;

	buf=read_ALC5621(0x36);

	DEBUGPRINT_ALC5621("Original reg_36h=0x%x \n",buf);

	buf = (buf&1) | (I2S_PRE_DIV<<12) | (I2S_SCLK_DIV<<9)| (I2S_WCLK_DIV_PRE<<5)| (I2S_WCLK_DIV<<2)| (ADDA_FILTER_CLK<<1);

	write_ALC5621(0x36, buf);

	printk("Modified reg_36h=0x%x\n",buf);

	return;

}

static void Compand_control(unsigned short int SETTING)
{
	unsigned short int buf;

	buf=read_ALC5621(0x38);

	DEBUGPRINT_ALC5621("Original reg_38h=0x%x \n",buf);

	buf = SETTING;

	write_ALC5621(0x38, buf);

	printk("Modified reg_38h=0x%x\n",buf);

	return;


}


static void Power_management_a1(unsigned short int SETTING)
{
	unsigned short int buf;

	buf=read_ALC5621(0x3a);

	DEBUGPRINT_ALC5621("Original reg_3ah=0x%x \n",buf);

	buf = SETTING;

	write_ALC5621(0x3a, buf);

	printk("Modified reg_3ah=0x%x\n",buf);

	return;

}

static void Power_management_a2(unsigned short int SETTING)
{
	unsigned short int buf;

	buf=read_ALC5621(0x3c);

	DEBUGPRINT_ALC5621("Original reg_3ch=0x%x \n",buf);

	buf = SETTING;

	write_ALC5621(0x3c, buf);

	printk("Modified reg_3ch=0x%x\n",buf);

	return;

}

static void Power_management_a3(unsigned short int SETTING)
{
	unsigned short int buf;

	buf=read_ALC5621(0x3e);

	DEBUGPRINT_ALC5621("Original reg_3eh=0x%x \n",buf);

	buf = SETTING;

	write_ALC5621(0x3e, buf);

	printk("Modified reg_3eh=0x%x\n",buf);

	return;

}

static void Additional_control(unsigned short int SETTING)
{
	unsigned short int buf;

	buf=read_ALC5621(0x40);

	DEBUGPRINT_ALC5621("Original reg_40h=0x%x \n",buf);

	buf = SETTING;

	write_ALC5621(0x40, buf);

	printk("Modified reg_40h=0x%x\n",buf);

	return;

}

static void Global_clock_control(unsigned short int SETTING)
{
	unsigned short int buf;

	buf=read_ALC5621(0x42);

	DEBUGPRINT_ALC5621("Original reg_42h=0x%x \n",buf);

	buf = SETTING;

	write_ALC5621(0x42, buf);

	printk("Modified reg_42h=0x%x\n",buf);

	return;

}

//PLL_N_CODE[7:0]
//PLL_M_BYPASS: 0-> no bypass, 1-> bypass
//PLL_K_CODE[2:0]
//PLL_M_CODE[3:0]
//Fout=(MCLK*(N+2))/((M+2)*(K+2))   {typical k=2}
static void PLL_control(unsigned short int PLL_N_CODE, unsigned short int PLL_M_BYPASS, unsigned short int PLL_K_CODE,
				unsigned short int PLL_M_CODE)
{
	unsigned short int buf;

	buf=read_ALC5621(0x44);

	DEBUGPRINT_ALC5621("Original reg_44h=0x%x \n",buf);

	buf = (PLL_N_CODE<<8) | (PLL_M_BYPASS<<7) | (PLL_K_CODE<<4) | PLL_M_CODE;

	write_ALC5621(0x44, buf);

	printk("Modified reg_44h=0x%x\n",buf);

	return;


}


//GPIO_OUT_HL: 0-> drive low, 1-> drive high
static void GPIO_Output_control(unsigned short int GPIO_OUT_HL)
{
	unsigned short int buf;

	buf=read_ALC5621(0x4A);

	DEBUGPRINT_ALC5621("Original reg_4Ah=0x%x \n",buf);

	buf = GPIO_OUT_HL <<1;

	write_ALC5621(0x4a, buf);

	printk("Modified reg_4ah=0x%x\n",buf);

	return;

}

//GPIO_CONF: 0-> output, 1-> intput
static void GPIO_config(unsigned short int GPIO_CONF)
{
	unsigned short int buf;

	buf=read_ALC5621(0x4c);

	DEBUGPRINT_ALC5621("Original reg_4ch=0x%x \n",buf);

	buf = (buf & 0xfffd)|  (GPIO_CONF <<1);

	write_ALC5621(0x4c, buf);

	printk("Modified reg_4ch=0x%x\n",buf);

	return;

}

static unsigned short int GPIO_status(void)
{

	unsigned short int buf;

	buf=read_ALC5621(0x54);

	//DEBUGPRINT_ALC5621("Original reg_54h=0x%x \n",buf);

	buf= (buf & 2) >>1;
	
	return buf;
}

static void PIN_sharing(unsigned short int SETTING)
{
	unsigned short int buf;

	buf=read_ALC5621(0x56);

	DEBUGPRINT_ALC5621("Original reg_56h=0x%x \n",buf);

	buf = SETTING;

	write_ALC5621(0x56, buf);

	printk("Modified reg_56h=0x%x\n",buf);

	return;

}

static void Misc_control(unsigned short int SETTING)
{
	unsigned short int buf;

	buf=read_ALC5621(0x5e);

	DEBUGPRINT_ALC5621("Original reg_5eh=0x%x \n",buf);

	buf = SETTING;

	write_ALC5621(0x5e, buf);

	printk("Modified reg_5eh=0x%x\n",buf);

	return;

}


static unsigned short int Vendor_ID1(void)
{

	unsigned short int buf;

	buf=read_ALC5621(0x7c);

	DEBUGPRINT_ALC5621("Original reg_7ch=0x%x \n",buf);

	return buf;
}

static unsigned short int Vendor_ID2(void)
{

	unsigned short int buf;

	buf=read_ALC5621(0x7e);

	DEBUGPRINT_ALC5621("Original reg_7eh=0x%x \n",buf);

	return buf;
}


static void dump_all_iis_reg(void)
{
	unsigned short int i,j;


	for (i=0;i<0x80;i+=2)
	{
		for (j=0;j<100*80;j++);

		ALC5621_fake_read(i);

	}

}


void ALC5621_init(int law)
{

	unsigned int i;

	ALC5621_software_reset();


	//Misc_control(0x8440);
#if 1
	Global_clock_control(0);
#else
	Global_clock_control(0xc000);
#endif

	//Audio_interface_setting(I2S_MODE_SLAVE, DSP_PCM_MODE, DATA_L16, ADC_LEFT_PHASE, DAC_LEFT_PHASE);
#ifdef RIGHT_PHASE
    #ifdef CONFIG_RTK_VOIP_DRIVERS_IIS
	Audio_interface_setting(I2S_MODE_SLAVE, I2S_FORMAT, DATA_L16, ADC_RIGHT_PHASE, DAC_RIGHT_PHASE);
    #else
	Audio_interface_setting(I2S_MODE_SLAVE, DSP_PCM_MODE, DATA_L16, ADC_RIGHT_PHASE, DAC_RIGHT_PHASE);
    #endif
#elif defined( LEFT_PHASE )
    #ifdef CONFIG_RTK_VOIP_DRIVERS_IIS
	Audio_interface_setting(I2S_MODE_SLAVE, I2S_FORMAT, DATA_L16, ADC_LEFT_PHASE, ADC_LEFT_PHASE);
    #else
	Audio_interface_setting(I2S_MODE_SLAVE, DSP_PCM_MODE, DATA_L16, ADC_LEFT_PHASE, DAC_LEFT_PHASE);
    #endif
#endif
	//Audio_interface_setting(I2S_MODE_SLAVE, I2S_FORMAT, DATA_L16, ADC_RIGHT_PHASE, DAC_RIGHT_PHASE);

	ADDA_oversample_rate(ADDA_OSR_128x);

	//ADDA_clock_control(3, 6, 0, 4, ADDA_FILTER_CLK_384FS);

#if 0
	//ADDA_clock_control(3, 6, 3, 2, ADDA_FILTER_CLK_384FS);
	ADDA_clock_control(3, 5, 3, 2, ADDA_FILTER_CLK_384FS);
#else
	ADDA_clock_control(0, 5, 3, 2, ADDA_FILTER_CLK_256FS);

	//PLL_control(62, 0, 2, 0);
#endif
	//Headphone_volume_control(0, 0, 0, 0, 1, 0, 0, 0);
	//Headphone_volume_control(0, 0, 0, 0, 0, 0, 0, 0);

	//Stereo_dac_volume_control(3, 8, 8);
	//Stereo_dac_volume_control(3, 4, 8);
	///Stereo_dac_volume_control(0, 0, 0);
	//Stereo_dac_volume_control(0, 8, 8);
	//Stereo_dac_volume_control(0, 0x1f, 0x00);
	//Stereo_dac_volume_control(7, 0x1f, 0x00);

	//MIC_routing_control(0xe, 0xe);
	MIC_routing_control(MIC_SL1(0xF, 0x0));
	//MIC_routing_control(0x6, 0xe);

#ifdef TC_SET
	MIC_volume_control(0, 0);
#else
	MIC_volume_control(8, 8);
#endif

	//ADC_record_gain_control(0xf, 0x15, 0, 0, 0xB);
	//ADC_record_gain_control(0x0, 0x1f, 0, 0x1f, 0x0);
	///ADC_record_gain_control(0xC, 0x1f, 0, 0x1f, 0x0);
	//ADC_record_gain_control(0xf, 0x1f, 0, 0x1f, 0x0);
	ADC_record_gain_control(0xf, SL2( 0x0b, 0, 0x0b, 0x0 ) );

	///ADC_mixer_control(0x3f, 0x7f);
	ADC_mixer_control(0x7f, 0x7f);
	//ADC_mixer_control(0x7b, 0x7f);

	//Output_mixer_control(0xc200);
	///Output_mixer_control(0xc300);
#ifdef ENABLE_3RD_SET
	Output_mixer_control(0x8bc0);
#else
	Output_mixer_control(0x8b00);
#endif
	//Output_mixer_control(0x8740);

	//Microphone_control(0x0510);
	//Microphone_control(0x0010);
	//Microphone_control(0x0010);
#ifdef TC_SET
	Microphone_control(0x0010);	
#else
	Microphone_control(0x0d00);
#endif

	//Speaker_volume_control(0, 0, 0, 0, 0, 0, 0, 0);
	Speaker_volume_control(0, 0, 0, 0, 0, 0, 1, 0);	// use speaker mixer, so don't need to swap argu. 
	///Additional_control(0xdc04);
	Additional_control(0x5300);
	///Auxout_volume_control(0, 0, 0, 0, 0, 0, 0, 0);
#ifdef ENABLE_3RD_SET
	Auxout_volume_control(SL4(0, 0, 0, 0, 0, 0, 0, 0));
#else
	Auxout_volume_control(SL4(1, 0, 0, 0, 1, 0, 0, 0));
#endif

	write_ALC5621_hidden(0x39, 0x9000);

	///Power_management_a3(0x8000);
#ifdef ENABLE_3RD_SET
	Power_management_a3(0xfAff);
#else
	Power_management_a3(0x9A03);
#endif

	///Headphone_volume_control(1, 0, 0, 0, 1, 0, 0, 0);
	Headphone_volume_control(SL4(1, 0, 0, 0, 0, 0, 0, 0));

	///Power_management_a1(0x0100);
	Power_management_a1(0x8830);

	///Power_management_a2(0x2000);
	//Power_management_a2(0xA7FB);
#ifdef ENABLE_3RD_SET
	Power_management_a2(0x27FF);
#else
	Power_management_a2(0x27FB);
#endif

	///Power_management_a3(0x8600);
#ifdef ENABLE_3RD_SET
	Power_management_a3(0xfAff);
#else
	Power_management_a3(0x9A03);
#endif

#ifdef RIGHT_PHASE
	Misc_control(0x0208);
#elif defined( LEFT_PHASE )
	Misc_control(0x0204);
#endif

	//volatile unsigned int get_timer_jiffies(void);
	//prom_printf("start_time:%d\n",get_timer_jiffies());
	for(i=0;i<10000000;i++);
	//prom_printf("end_time:%d\n",get_timer_jiffies());

#ifndef CONFIG_RTK_VOIP_DRIVERS_IIS
	if (law == 0) // linear
		Compand_control(0x0);
	else if (law == 1) //a-law
		Compand_control(0xA);
	else //u-law
		Compand_control(0x5);
#else
	Compand_control(0x0);
#endif

	///Power_management_a1(0x8820);
#ifdef ENABLE_3RD_SET
	Power_management_a1(0x8833);
#else
	Power_management_a1(0x8830);
#endif

#ifdef RIGHT_PHASE
	Misc_control(0x8208);
#elif defined( LEFT_PHASE )
	Misc_control(0x8204);
#endif
	///Headphone_volume_control(0, 0, 0, 0, 0, 0, 0, 0);
	Headphone_volume_control(SL4(1, 0, 0, 0, 0, 0, 0, 0));

	//Power_management_a1(0x8820);
	//Power_management_a1(0x8870);
	//Power_management_a1(0x8920);
	//Power_management_a1(0xc877);

	//Power_management_a2(0xe6a2);
	///Power_management_a2(0xf7bf);
	//Power_management_a2(0xA7FB);
#ifdef ENABLE_3RD_SET
	Power_management_a2(0x27FF);
#else
	Power_management_a2(0x27FB);
#endif 

	//Power_management_a3(0x840a);
	///Power_management_a3(0xf6ff);
#ifdef ENABLE_3RD_SET
	Power_management_a3(0xf6ff);
#else
	Power_management_a3(0x960f);
#endif
	
	AI_AO_select( MIC2_MONO );

#if 1
	// init GPIO for hook detection 
	PIN_sharing( 0 );	// no sharing 
	
	GPIO_config( 1 );	// input 
#endif
	
	dump_all_iis_reg();
#ifdef CONFIG_RTK_VOIP_GPIO_IPP_8972B_V99
	AI_AO_select(MIC1_SPEAKER);
#endif
}

void AI_AO_select(unsigned char type)
{
	if (type == MIC2_MONO) {
#ifdef ENABLE_3RD_SET
		Power_management_a2(0x27FF);
#else
		Power_management_a2(0x27FB);
#endif
		iphone_handfree=0;
#if 0
		//Output_mixer_control(0xC200);
		///Output_mixer_control(0xC300);
		Output_mixer_control(0x8b00);
		//ADC_mixer_control(0x5f, 0x7f);
		// ADC_mixer_control(0x00, 0x00);
		//ADC_mixer_control(0x7f, 0x7f);
		ADC_mixer_control(0x1f, 0x1f);
#else
  #ifdef TC_SET
		//Stereo_dac_volume_control(0, 0x1f, 0x00);		// left: -34.5db, right: +0db  
		Stereo_dac_volume_control_VOL(SL1(0xc, 0x0b));
		MIC_routing_control(MIC_SL1(0xF, 0x6));
		ADC_record_gain_control(0xf, SL2(0x0b, 0, 0x18, 0x0));
  #else
		Stereo_dac_volume_control(0, 0x1f, 0x08);	// left: -34.5db, right: +0db  
  #endif
		Output_mixer_control(0x8300);	// Speaker: none, HP: HP mixer  
  #ifdef HANDSET_MIC2
		ADC_mixer_control(SL1(0x7f, 0x5f));	// MIC: right MIC2 
  #elif defined( HANDSET_MIC1 )
		ADC_mixer_control(SL1(0x7f, 0x3f));	// MIC: right MIC1 
  #endif
#endif

	} else if (type == MIC1_SPEAKER) {
#ifdef ENABLE_3RD_SET
		Power_management_a2(0xA7FF);
#else
		Power_management_a2(0xA7FB);
#endif
		iphone_handfree=1;  

		Speaker_volume_control(1, 0, 0, 0, 1, 0, 1, 0); // use speaker mixer, so don't need to swap argu. 
		
		//Stereo_dac_volume_control(0, 0x1f, 0x00);	// left: -34.5db, right: +12db
		Stereo_dac_volume_control_VOL(SL1(0x08, 0x09));
		//Stereo_dac_volume_control_VOL(0x8, 0x04);
  #ifdef TC_SET
		MIC_routing_control(MIC_SL1(0x7, 0xE));
		ADC_record_gain_control(0xf, SL2(0x0b, 0, 0x1b, 0x0));
  #endif
		//Output_mixer_control(0x8400);
		Output_mixer_control(0x8800);	// Speaker: speaker mixer, HP: none 
		//ADC_mixer_control(0x3f, 0x7f);
  #ifdef HANDSET_MIC2
		ADC_mixer_control(SL1(0x7f, 0x3f));	// MIC: right MIC1 
  #elif defined( HANDSET_MIC1 )
		ADC_mixer_control(SL1(0x7f, 0x5f));	// MIC: right MIC2 
  #endif
		
		Speaker_volume_control(0, 0, 0, 0, 0, 0, 1, 0); // use speaker mixer, so don't need to swap argu. 

#ifdef CONFIG_AUDIOCODES_VOIP	// calibration for AEC of ACMW.
		printk("\nALC5621 calibration for AEC...\n");
		write_ALC5621(0xc, 0x606);
		write_ALC5621(0x12, 0xff97);
#endif

	} else if (type == MONO_ONLY) {
#ifdef ENABLE_3RD_SET
		Power_management_a2(0x27FF);
#else
		Power_management_a2(0x27FB);
#endif
		iphone_handfree=0;
		
  #ifdef TC_SET
		//Stereo_dac_volume_control(0, 0x1f, 0x00);	// left: -34.5db, right: +0db  
		Stereo_dac_volume_control_VOL(SL1(0x8, 0x08));
		MIC_routing_control(MIC_SL1(0xF, 0x6));
		ADC_record_gain_control(0xf, SL2(0x0b, 0, 0x0b, 0x0));
  #else
		Stereo_dac_volume_control(0, 0x1f, 0x08);	// left: -34.5db, right: +0db  
  #endif
		Output_mixer_control(0x8300);	// Speaker: none, HP: HP mixer  
		ADC_mixer_control(SL1(0x7f, 0x7f));	// Turn off all MIC  
		
	} else if (type == SPEAKER_ONLY) {
#ifdef ENABLE_3RD_SET
		Power_management_a2(0xA7FF);
#else
		Power_management_a2(0xA7FB);
#endif
		iphone_handfree=0;

		//Stereo_dac_volume_control(0, 0x1f, 0x00);	// left: -34.5db, right: +12db  
		Stereo_dac_volume_control_VOL(SL1(0x8, 0x08));
  #ifdef TC_SET
		MIC_routing_control(MIC_SL1(0x7, 0xE));
		ADC_record_gain_control(0xf, SL2(0x0b, 0, 0x0b, 0x0));
  #endif
		Output_mixer_control(0x8800);	// Speaker: speaker mixer, HP: none 
		ADC_mixer_control(SL1(0x7f, 0x7f));	// Turn off all MIC  

	} else 
		DEBUGPRINT_ALC5621("No such input and output.\n");
	
	return;
}

unsigned char ALC5621_GetGpioStatus( void )
{
#if 1
	return ( unsigned char )GPIO_status();
#else
	unsigned short status;
	
	status = GPIO_status();
	
	write_ALC5621( 0x54, status & ~2 );

	return ( unsigned char )status;
#endif
}

void ALC5621_loopback( unsigned char on_off )
{
	if( !on_off ) {
		printk( "ALC5621_loopback off (MIC2_MONO)\n" );
		AI_AO_select( MIC2_MONO );
		return;
	}
	
	printk( "ALC5621_loopback on\n" );
	
	MIC_routing_control( 0x0, 0x0 );
	
	ADC_mixer_control( 0x00, 0x00 );
	
	Output_mixer_control( 0x8B00 );
	
	ADC_record_gain_control( 0, 0x0B, 0, 0x0B, 0 );
	
	Speaker_volume_control(0, 0, 0, 0, 0, 0, 1, 0);
	
	Stereo_dac_volume_control( 0, 0x08, 0x08 );
	
	Power_management_a2(0xA7FB);
}

