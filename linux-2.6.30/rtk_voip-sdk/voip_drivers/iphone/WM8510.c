#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/delay.h>  	// udelay
#include "WM8510.h"
#include "base_gpio.h"
#include "base_i2c_WM8510.h"
//WM8510 default value
static WM8510_DEFAULT WM85X_DEF_VAL = {0, 0, 0, 0x50, 0, 0x140, 0, 0,
					  0, 0xff, 0x100, 0xff, 0x32, 0, 0, 0, 0, 0, 0x38,
					  0x0b, 0x32, 0, 0x08, 0x0c, 0x93, 0xe9, 0, 3, 0x10,
					  0x100, 0x02, 0, 0x39, 0};

//---------------------static function prototype-------------------------------//
static void mic_input_interface(unsigned char MICP2INPPGA, unsigned char MICN2INPPGA, 
						 		unsigned char MIC2_2INPPGA);
static void Analog_input_enable_ctrl(unsigned char device, unsigned char device_en);						 		
static void input_PGA_mute(unsigned char INPPGAMUTE);
static void input_PGA_2Boost(unsigned char PGABOOST);
static void mic2_input_resistor(unsigned char MIC2MODE, unsigned char MIC2EN);
static void mic2_2Boost(unsigned char MIC2_2BOOSTVOL);
static void micp_2Boost(unsigned char MICP2BOOSTVOL);
static void mic_bias_boost_setting(unsigned char MBVSEL);
static void ADC_oversample_rate(unsigned char ADCOSR);
static void ALC_mode_select(unsigned char ALCSEL, unsigned char ALCMODE);
static void ALC_max_min_gain(unsigned char ALCLVL, unsigned char ALCZC,
							 unsigned char ALCMAXGAIN, unsigned char ALCMINGAIN);
static void ALC_hold_time_setting(unsigned char ALCHLD);
static void ALC_decay_time_setting(unsigned char ALCDCY);
static void ALC_attack_time_setting(unsigned char ALCATK);
static void ALC_noise_gate_setting(unsigned char NGEN, unsigned char NGTH);
static void DAC_oversample_rate(unsigned char DACOSR);
static void DAC_de_emphasis_ctrl(unsigned char DEEMPH);
static void DAC_soft_mute_ctrl(unsigned char DACMU);
static void DAC_auto_mute_ctrl(unsigned char AMUTE);
static void DAC_limiter_boost_setting(unsigned char LIMBOOST);
static void analog_output_boost_ctrl(unsigned char device, unsigned char boost_en);
static void sidetone_attenuation_ctrl(unsigned char device, unsigned char enable);
static void Speaker_mixer_control(unsigned char input_select, unsigned char device_en, unsigned char SPKATTN);
static void Mono_mixer_control(unsigned char input_select, unsigned char device_en,
						unsigned char MONOATTN, unsigned char MONOMUTE);
static void Analog_output_enable_ctrl(unsigned char device, unsigned char device_en);
static void Power_management_ctrl(unsigned char device, unsigned char device_en);
static void Analog_output_resistance(unsigned char VROI);
static void Thermal_shutdown_ctrl(unsigned char TSDEN);
static void Audio_interface_setting(unsigned char FMT, unsigned char WL, unsigned char ADCLRSWAP, unsigned char DACLRSWAP);
static void Audio_companding_setting(unsigned char ADC_COMP, unsigned char DAC_COMP);
//static void Audio_interface_loopback_test(unsigned char LOOPBACK);
static void Audio_sample_frequency(unsigned char SR);
static void Audio_clock_setting(unsigned char MS, unsigned char CLKSEL, unsigned char MCLKDIV);
static void Power_VMID_resistor(unsigned char VMIDSEL);
static void WM8510_special_register(void);

int iphone_handfree;
//-----------------------------------------------------------------------------//

void WM8510_fake_read(unsigned char reg_number)
{
	unsigned char temp;
	if (reg_number <= 9 && reg_number > 0)
		temp = reg_number - 1;
	else if (reg_number > 9 && reg_number <= 11)
		temp = reg_number - 2;
	else if (reg_number > 13 && reg_number <= 15)
		temp = reg_number - 4;
	else if (reg_number > 23 && reg_number <= 25)
		temp = reg_number - 12;	
	else if (reg_number > 26 && reg_number <= 30)
		temp = reg_number - 13;
	else if (reg_number > 31 && reg_number <= 40)
		temp = reg_number - 14;
	else if (reg_number > 43 && reg_number <= 45)
		temp = reg_number - 17;
	else if (reg_number == 47)
		temp = reg_number - 18;
	else if (reg_number > 48 && reg_number <= 50)
		temp = reg_number - 19;
	else if (reg_number == 54)
		temp = reg_number - 22;
	else if (reg_number == 56)
		temp = reg_number - 23;
	else {
		printk("No such register number\n");
		temp = 255;
	}
	if (temp != 255) 										
		printk("register %d = 0x%x\n",reg_number,*(&WM85X_DEF_VAL.reg_1+temp));
	
	return;
}

void WM8510_software_reset(void)
{
	write_WM8510(0,0x07);
	printk("Reset WM8510 ");
#ifdef _WM8510_DEBUG_	
	printk(" write reg_0:0x07\n");
#endif
	return;
}

//Mic1 and Mic2 selection
static void mic_input_interface(unsigned char MICP2INPPGA, unsigned char MICN2INPPGA, 
						 unsigned char MIC2_2INPPGA)
{
	unsigned short buf;
	
	buf = MICP2INPPGA | (MICN2INPPGA<<1) | (MIC2_2INPPGA<<2);
#ifdef _WM8510_DEBUG_
	printk("Original reg_44=0x%x ",WM85X_DEF_VAL.reg_44);
#endif
	WM85X_DEF_VAL.reg_44 = (WM85X_DEF_VAL.reg_44 & 0x1f8) | buf;
	write_WM8510(44,WM85X_DEF_VAL.reg_44);
	if (MICP2INPPGA && MICN2INPPGA)
		printk("Analog input: MICP/N ");
	else
		printk("Analog input: MIC2	");
	
	if (MIC2_2INPPGA) {
		//set Vo = -Vi and enable MIC2 input buffer
		mic2_input_resistor(0,1);
	} else if (MIC2_2INPPGA == 0) {
		//set Vo = -Vi and disable MIC2 input buffer
		mic2_input_resistor(0,0);
	}

#ifndef _WM8510_DEBUG_
	printk("\n");
#endif
			
#ifdef _WM8510_DEBUG_
	printk("Modified reg_44=0x%x\n",WM85X_DEF_VAL.reg_44);
#endif	
	
	return;
}

//device:0->ADConverter, 1->InputMicPGA, 2->InputBoost.
//device_en:0->disconnect device, 1->connect device.
static void Analog_input_enable_ctrl(unsigned char device, unsigned char device_en)
{
#ifdef _WM8510_DEBUG_
	printk("Original reg_2=0x%x ",WM85X_DEF_VAL.reg_2);
#endif
	if (device_en) {
		if (device == 0) {
			printk("ADConverter enable ");
			WM85X_DEF_VAL.reg_2 = (WM85X_DEF_VAL.reg_2 & 0x1fe) | device_en;
			write_WM8510(2,WM85X_DEF_VAL.reg_2);
		} else if (device == 1) {
			printk("InputMicPGA enable ");
			WM85X_DEF_VAL.reg_2 = (WM85X_DEF_VAL.reg_2 & 0x1fb) | (device_en<<2);
			write_WM8510(2,WM85X_DEF_VAL.reg_2);
		} else if (device == 2) {
			printk("InputBoost enable ");
			WM85X_DEF_VAL.reg_2 = (WM85X_DEF_VAL.reg_2 & 0x1ef) | (device_en<<4);
			write_WM8510(2,WM85X_DEF_VAL.reg_2);
		}
	
	} else {
		if (device == 0) {
			printk("ADConverter disconnect ");
			WM85X_DEF_VAL.reg_2 = (WM85X_DEF_VAL.reg_2 & 0x1fe) | device_en;
			write_WM8510(2,WM85X_DEF_VAL.reg_2);
		} else if (device == 1) {
			printk("InputMicPGA disconnect ");
			WM85X_DEF_VAL.reg_2 = (WM85X_DEF_VAL.reg_2 & 0x1fb) | (device_en<<2);
			write_WM8510(2,WM85X_DEF_VAL.reg_2);
		} else if (device == 2) {
			printk("InputBoost disconnect ");
			WM85X_DEF_VAL.reg_2 = (WM85X_DEF_VAL.reg_2 & 0x1ef) | (device_en<<4);
			write_WM8510(2,WM85X_DEF_VAL.reg_2);
		}		
	}

#ifndef _WM8510_DEBUG_
	printk("\n");
#endif

#ifdef _WM8510_DEBUG_
	printk("Modified reg_2=0x%x\n",WM85X_DEF_VAL.reg_2);
#endif
	return;
}

//INPPGAMUTE:0->not mute, 1->mute(disconnect to input boost circuit).
static void input_PGA_mute(unsigned char INPPGAMUTE)
{

#ifdef _WM8510_DEBUG_
	printk("Original reg_45=0x%x ",WM85X_DEF_VAL.reg_45);
#endif	
	WM85X_DEF_VAL.reg_45 = (WM85X_DEF_VAL.reg_45 & 0x1bf) | (INPPGAMUTE<<6);
	write_WM8510(45,WM85X_DEF_VAL.reg_45);
	if (INPPGAMUTE)
		printk("Input PGA mute ");
	else
		printk("Input PGA not mute ");	

#ifndef _WM8510_DEBUG_
	printk("\n");
#endif

#ifdef _WM8510_DEBUG_
	printk("Modified reg_45=0x%x\n",WM85X_DEF_VAL.reg_45);
#endif
	return;
}

//INPPGAVOL:PGA gain. INPPGAZC:0->zero cross disable, 1->zero cross enable.
void mic_gain_control(unsigned char INPPGAVOL, unsigned char INPPGAZC)
{
	unsigned short buf;
	
	//disable ALC function
	WM85X_DEF_VAL.reg_32 = (WM85X_DEF_VAL.reg_32 & 0x0ff);
	write_WM8510(32,WM85X_DEF_VAL.reg_32);
	//PGA not mute
	input_PGA_mute(0);
	//set PGA gain and zero cross 
	buf = INPPGAVOL | (INPPGAZC<<7);
#ifdef _WM8510_DEBUG_
	printk("Original reg_45=0x%x ",WM85X_DEF_VAL.reg_45);
#endif
	WM85X_DEF_VAL.reg_45 = (WM85X_DEF_VAL.reg_45 & 0x140) | buf;
	write_WM8510(45,WM85X_DEF_VAL.reg_45);
#ifdef _WM8510_DEBUG_
	printk("Modified reg_45=0x%x\n",WM85X_DEF_VAL.reg_45);
#endif	
	
	return;
}

//input PGA to boost circuit.PGABOOST:0->0dB, 1->20dB 
static void input_PGA_2Boost(unsigned char PGABOOST)
{
	//PGA not mute
	input_PGA_mute(0);
#ifdef _WM8510_DEBUG_
	printk("Original reg_47=0x%x ",WM85X_DEF_VAL.reg_47);
#endif
	WM85X_DEF_VAL.reg_47 = (WM85X_DEF_VAL.reg_47 & 0x0ff) | (PGABOOST<<8);
	write_WM8510(47,WM85X_DEF_VAL.reg_47);
#ifdef _WM8510_DEBUG_
	printk("Modified reg_47=0x%x\n",WM85X_DEF_VAL.reg_47);
#endif	

	return;
}

//MIC2MODE:0->Vo/Vi = -1, 1->Vo/Vi = ?.
//MIC2EN:0->mic2 input buffer disable, 1->mic2 input buffer enable.
static void mic2_input_resistor(unsigned char MIC2MODE, unsigned char MIC2EN)
{

#ifdef _WM8510_DEBUG_
	printk("Original reg_44=0x%x ",WM85X_DEF_VAL.reg_44);
#endif
	WM85X_DEF_VAL.reg_44 = (WM85X_DEF_VAL.reg_44 & 0x1f7) | (MIC2MODE<<3);
	write_WM8510(44,WM85X_DEF_VAL.reg_44);
#ifdef _WM8510_DEBUG_
	printk("Modified reg_44=0x%x\n",WM85X_DEF_VAL.reg_44);
#endif

#ifdef _WM8510_DEBUG_
	printk("Original reg_1=0x%x ",WM85X_DEF_VAL.reg_1);
#endif
	WM85X_DEF_VAL.reg_1 = (WM85X_DEF_VAL.reg_1 & 0x1bf) | (MIC2EN<<6);
	write_WM8510(1,WM85X_DEF_VAL.reg_1);
#ifdef _WM8510_DEBUG_
	printk("Modified reg_1=0x%x\n",WM85X_DEF_VAL.reg_1);
#endif

	return;
}

//Mic2 to Boost circuit.MIC2_2BOOSTVOL:0->disconnect, other->gain volume
static void mic2_2Boost(unsigned char MIC2_2BOOSTVOL)
{

#ifdef _WM8510_DEBUG_
	printk("Original reg_47=0x%x ",WM85X_DEF_VAL.reg_47);
#endif
	WM85X_DEF_VAL.reg_47 = (WM85X_DEF_VAL.reg_47 & 0x1f8) | MIC2_2BOOSTVOL;
	write_WM8510(47,WM85X_DEF_VAL.reg_47);
#ifdef _WM8510_DEBUG_
	printk("Modified reg_47=0x%x\n",WM85X_DEF_VAL.reg_47);
#endif

	return;
}

//Micp pin to Boost circuit.MICP2BOOSTVOL:0->disconnect, other->gain volume
static void micp_2Boost(unsigned char MICP2BOOSTVOL)
{

	//set MICP2INPPGA 0
	WM85X_DEF_VAL.reg_44 = (WM85X_DEF_VAL.reg_44 & 0x1fe);
	write_WM8510(44,WM85X_DEF_VAL.reg_44);
#ifdef _WM8510_DEBUG_
	printk("Original reg_47=0x%x ",WM85X_DEF_VAL.reg_47);
#endif
	WM85X_DEF_VAL.reg_47 = (WM85X_DEF_VAL.reg_47 & 0x18f) | (MICP2BOOSTVOL<<4);
	write_WM8510(47,WM85X_DEF_VAL.reg_47);
#ifdef _WM8510_DEBUG_
	printk("Modified reg_47=0x%x\n",WM85X_DEF_VAL.reg_47);
#endif
	
	return;
}

//MBVSEL(MICBIAS pin output voltage):0->0.9*AVDD, 1->0.65*AVDD
static void mic_bias_boost_setting(unsigned char MBVSEL)
{

#ifdef _WM8510_DEBUG_
	printk("Original reg_44=0x%x ",WM85X_DEF_VAL.reg_44);
#endif
	WM85X_DEF_VAL.reg_44 = (WM85X_DEF_VAL.reg_44 & 0x0ff) | (MBVSEL<<8);
	write_WM8510(44,WM85X_DEF_VAL.reg_44);
	
	if (MBVSEL)
		printk("Mic Bias voltage 0.65*AVDD ");
	else
		printk("Mic Bias voltage 0.9*AVDD ");	

#ifndef _WM8510_DEBUG_
	printk("\n");
#endif

#ifdef _WM8510_DEBUG_
	printk("Modified reg_44=0x%x\n",WM85X_DEF_VAL.reg_44);
#endif

	return;
}

//ADC oversample rate select. ADCOSR:0->64x, 1->128x.
static void ADC_oversample_rate(unsigned char ADCOSR)
{

#ifdef _WM8510_DEBUG_
	printk("Original reg_14=0x%x ",WM85X_DEF_VAL.reg_14);
#endif	
	WM85X_DEF_VAL.reg_14 = (WM85X_DEF_VAL.reg_14 & 0x1f7) | (ADCOSR<<3);
	write_WM8510(14,WM85X_DEF_VAL.reg_14);
	if (ADCOSR)
		printk("ADC oversample rate 128x ");
	else
		printk("ADC oversample rate 64x ");	

#ifndef _WM8510_DEBUG_
	printk("\n");
#endif

#ifdef _WM8510_DEBUG_
	printk("Modified reg_14=0x%x\n",WM85X_DEF_VAL.reg_14);
#endif

	return;
}

//ADC high pass filter control.HPFEN:0->disable hpf, 1->enable hpf. 
//HPFAPP:0->1st order, 1->2nd order.HPFCUT:cut-off frequency.
void ADC_high_pass_filter_ctrl(unsigned char HPFEN, unsigned char HPFAPP, unsigned char HPFCUT)
{

#ifdef _WM8510_DEBUG_
	printk("Original reg_14=0x%x ",WM85X_DEF_VAL.reg_14);
#endif
	WM85X_DEF_VAL.reg_14 = (WM85X_DEF_VAL.reg_14 & 0x00e) | (HPFEN<<8) | (HPFAPP<<7) | (HPFCUT<<4);
	write_WM8510(14,WM85X_DEF_VAL.reg_14);

	if (HPFEN)
		printk("High-pass filter enable ");
	else
		printk("High-pass filter disable ");	

#ifndef _WM8510_DEBUG_
	printk("\n");
#endif

#ifdef _WM8510_DEBUG_
	printk("Modified reg_14=0x%x\n",WM85X_DEF_VAL.reg_14);
#endif
	
	return;
}

//ADC Notch filter control. NFEN:0->disable notch filter, 1->enable notch filter.
//NFA0 and NFA1:notch filter coefficient.
void ADC_notch_filter_ctrl(unsigned char NFEN, unsigned short NFA0, unsigned short NFA1)
{

#ifdef _WM8510_DEBUG_
	printk("Original reg_27=0x%x ",WM85X_DEF_VAL.reg_27);
#endif
	WM85X_DEF_VAL.reg_27 = (WM85X_DEF_VAL.reg_27 & 0x17f) | (NFEN<<7);
	write_WM8510(27,WM85X_DEF_VAL.reg_27);

	if (NFEN)
		printk("Notch filter enable ");
	else
		printk("Notch filter disable ");	

#ifndef _WM8510_DEBUG_
	printk("\n");
#endif

#ifdef _WM8510_DEBUG_
	printk("Modified reg_27=0x%x\n",WM85X_DEF_VAL.reg_27);
#endif
	if (NFEN) {
#ifdef _WM8510_DEBUG_
		printk("Original reg_27=0x%x ",WM85X_DEF_VAL.reg_27);
		printk("Original reg_28=0x%x ",WM85X_DEF_VAL.reg_28);
		printk("Original reg_29=0x%x ",WM85X_DEF_VAL.reg_29);
		printk("Original reg_30=0x%x\n",WM85X_DEF_VAL.reg_30);
#endif		
		WM85X_DEF_VAL.reg_27 = (WM85X_DEF_VAL.reg_27 & 0x180) | (NFA0>>7);
		write_WM8510(27,WM85X_DEF_VAL.reg_27);
		WM85X_DEF_VAL.reg_28 = (WM85X_DEF_VAL.reg_28 & 0x180) | (NFA0 & 0x7f);
		write_WM8510(28,WM85X_DEF_VAL.reg_28);
		WM85X_DEF_VAL.reg_29 = (WM85X_DEF_VAL.reg_29 & 0x180) | (NFA1>>7);
		write_WM8510(29,WM85X_DEF_VAL.reg_29);
		WM85X_DEF_VAL.reg_30 = (WM85X_DEF_VAL.reg_30 & 0x180) | (NFA1 & 0x7f) | (1<<8);
		write_WM8510(30,WM85X_DEF_VAL.reg_30);
#ifdef _WM8510_DEBUG_
		printk("Modified reg_27=0x%x ",WM85X_DEF_VAL.reg_27);
		printk("Modified reg_28=0x%x ",WM85X_DEF_VAL.reg_28);
		printk("Modified reg_29=0x%x ",WM85X_DEF_VAL.reg_29);
		printk("Modified reg_30=0x%x\n",WM85X_DEF_VAL.reg_30);
#endif	
	}
	

	return;
}

//ADC volume attenuate. ADCVOL:0->mute, other->gain attenuated
void ADC_volume_gain_ctrl(unsigned char ADCVOL)
{

#ifdef _WM8510_DEBUG_
	printk("Original reg_15=0x%x ",WM85X_DEF_VAL.reg_15);
#endif
	WM85X_DEF_VAL.reg_15 = (WM85X_DEF_VAL.reg_15 & 0x100) | ADCVOL;
	write_WM8510(15,WM85X_DEF_VAL.reg_15);
#ifdef _WM8510_DEBUG_
	printk("Modified reg_15=0x%x\n",WM85X_DEF_VAL.reg_15);
#endif

	return;
}

//ALCSEL:0->ALC function disable, 1->ALC function enable.
//ALCMODE:0->ALC mode, 1->Limited mode.
static void ALC_mode_select(unsigned char ALCSEL, unsigned char ALCMODE)
{

#ifdef _WM8510_DEBUG_
	printk("Original reg_32=0x%x ",WM85X_DEF_VAL.reg_32);
#endif
	WM85X_DEF_VAL.reg_32 = (WM85X_DEF_VAL.reg_32 & 0xff) | (ALCSEL<<8);
	write_WM8510(32,WM85X_DEF_VAL.reg_32);

	if (ALCSEL)
		printk("ALC function enable ");
	else
		printk("ALC function disable ");
#ifdef _WM8510_DEBUG_
	printk("Modified reg_32=0x%x\n",WM85X_DEF_VAL.reg_32);
#endif	
	
	if (ALCSEL) {
#ifdef _WM8510_DEBUG_
		printk("Original reg_34=0x%x ",WM85X_DEF_VAL.reg_34);
#endif
		WM85X_DEF_VAL.reg_34 = (WM85X_DEF_VAL.reg_34 & 0xff) | (ALCMODE<<8);
		write_WM8510(34,WM85X_DEF_VAL.reg_34);		

		if (ALCMODE)
			printk("ALC mode ");
		else
			printk("Limited mode ");
#ifdef _WM8510_DEBUG_
		printk("Modified reg_34=0x%x\n",WM85X_DEF_VAL.reg_34);
#endif		
	}

#ifndef _WM8510_DEBUG_
	printk("\n");
#endif
		
	return;
}

//ALCLVL:signal level at ADC input. ALCZC:0->zero cross disable, 1->zero cross enable.
//ALCMAXGAIN:ALC upper limit gain. ALCMINGAIN:ALC lower limit gain.
static void ALC_max_min_gain(unsigned char ALCLVL, unsigned char ALCZC,
							 unsigned char ALCMAXGAIN, unsigned char ALCMINGAIN)
{

#ifdef _WM8510_DEBUG_
	printk("Original reg_33=0x%x ",WM85X_DEF_VAL.reg_33);
#endif
	WM85X_DEF_VAL.reg_33 = (WM85X_DEF_VAL.reg_33 & 0x0f0) | ALCLVL | (ALCZC<<8);
	write_WM8510(33,WM85X_DEF_VAL.reg_33);
#ifdef _WM8510_DEBUG_	
	printk("Modified reg_33=0x%x\n",WM85X_DEF_VAL.reg_33);
#endif
		
#ifdef _WM8510_DEBUG_
	printk("Original reg_32=0x%x ",WM85X_DEF_VAL.reg_32);
#endif
	WM85X_DEF_VAL.reg_32 = (WM85X_DEF_VAL.reg_32 & 0x100) | (ALCMAXGAIN<<3) | (ALCMINGAIN);
	write_WM8510(32,WM85X_DEF_VAL.reg_32);
#ifdef _WM8510_DEBUG_	
	printk("Modified reg_32=0x%x\n",WM85X_DEF_VAL.reg_32);
#endif

	return;
}

//ALCHLD:ALC hold time before gain is incresed.
static void ALC_hold_time_setting(unsigned char ALCHLD)
{

#ifdef _WM8510_DEBUG_
	printk("Original reg_33=0x%x ",WM85X_DEF_VAL.reg_33);
#endif
	WM85X_DEF_VAL.reg_33 = (WM85X_DEF_VAL.reg_33 & 0x0f0) | (ALCHLD<<4);
	write_WM8510(33,WM85X_DEF_VAL.reg_33);
#ifdef _WM8510_DEBUG_	
	printk("Modified reg_33=0x%x\n",WM85X_DEF_VAL.reg_33);
#endif

	return;
}

//ALCDCY:ALC decay time(gain ramp-up)
static void ALC_decay_time_setting(unsigned char ALCDCY)
{

#ifdef _WM8510_DEBUG_
	printk("Original reg_34=0x%x ",WM85X_DEF_VAL.reg_34);
#endif
	WM85X_DEF_VAL.reg_34 = (WM85X_DEF_VAL.reg_34 & 0x10f) | (ALCDCY<<4);
	write_WM8510(34,WM85X_DEF_VAL.reg_34);
#ifdef _WM8510_DEBUG_	
	printk("Modified reg_34=0x%x\n",WM85X_DEF_VAL.reg_34);
#endif
	
	return;
}

//ALCATK:ALC attack time(gain ramp-down)
static void ALC_attack_time_setting(unsigned char ALCATK)
{

#ifdef _WM8510_DEBUG_
	printk("Original reg_34=0x%x ",WM85X_DEF_VAL.reg_34);
#endif
	WM85X_DEF_VAL.reg_34 = (WM85X_DEF_VAL.reg_34 & 0x1f0) | ALCATK;
	write_WM8510(34,WM85X_DEF_VAL.reg_34);
#ifdef _WM8510_DEBUG_	
	printk("Modified reg_34=0x%x\n",WM85X_DEF_VAL.reg_34);
#endif

	return;
}

//ALCLVL:signal level at ADC input. ALCZC:0->zero cross disable, 1->zero cross enable.
//ALCMAXGAIN:ALC upper limit gain. ALCMINGAIN:ALC lower limit gain.
//ALCHLD:ALC hold time before gain is incresed.
//ALCDCY:ALC decay time(gain ramp-up).
//ALCATK:ALC attack time(gain ramp-down).
void ALC_mode_setting(unsigned char ALCLVL, unsigned char ALCZC,
					  unsigned char ALCMAXGAIN, unsigned char ALCMINGAIN, unsigned char ALCHLD,
					  unsigned char ALCDCY, unsigned char ALCATK)
{
	//PGA not mute
	input_PGA_mute(0);
	//enable ALC function and select ALC mode
	ALC_mode_select(1,0);
	//set singal level, zero-cross, Max gain, Min gain.
	ALC_max_min_gain(ALCLVL, ALCZC, ALCMAXGAIN, ALCMINGAIN);
	//set ALC hold time
	ALC_hold_time_setting(ALCHLD);
	//set ALC decay time
	ALC_decay_time_setting(ALCDCY);
	//set ALC attack time
	ALC_attack_time_setting(ALCATK);
	return;
}

//ALCLVL:signal level at ADC input. ALCZC:0->zero cross disable, 1->zero cross enable.
//ALCMAXGAIN:ALC upper limit gain. ALCMINGAIN:ALC lower limit gain.
//ALCDCY:ALC decay time(gain ramp-up).
//ALCATK:ALC attack time(gain ramp-down).
void ALC_Limited_mode_setting(unsigned char ALCLVL, unsigned char ALCZC,
					  	  unsigned char ALCMAXGAIN, unsigned char ALCMINGAIN,
					  	  unsigned char ALCDCY, unsigned char ALCATK)
{
	//PGA not mute
	input_PGA_mute(0);
	//enable ALC function and select Limited mode
	ALC_mode_select(1,1);
	//set singal level, zero-cross, Max gain, Min gain.
	ALC_max_min_gain(ALCLVL, ALCZC, ALCMAXGAIN, ALCMINGAIN);
	//set ALC decay time
	ALC_decay_time_setting(ALCDCY);
	//set ALC attack time
	ALC_attack_time_setting(ALCATK);
	return;
}

//NGEN:0->noise gate function disable, 1->noise gate function enable.
//MGTH:noise gate threshold.
static void ALC_noise_gate_setting(unsigned char NGEN, unsigned char NGTH)
{
	
#ifdef _WM8510_DEBUG_
	printk("Original reg_35=0x%x ",WM85X_DEF_VAL.reg_35);
#endif	
	WM85X_DEF_VAL.reg_35 = (WM85X_DEF_VAL.reg_35 & 0x1f7) | (NGEN<<3);
	write_WM8510(35,WM85X_DEF_VAL.reg_35);
	
	if (NGEN)
		printk("Noise gate function enable ");
	else
		printk("Noise gate function disable ");	
#ifdef _WM8510_DEBUG_
	printk("Modified reg_35=0x%x\n",WM85X_DEF_VAL.reg_35);
#endif
	if (NGEN) {
		if ((WM85X_DEF_VAL.reg_32 & 0x100) && ((WM85X_DEF_VAL.reg_34 & 0x100) == 0)) {
#ifdef _WM8510_DEBUG_
			printk("Original reg_35=0x%x ",WM85X_DEF_VAL.reg_35);
#endif	
			WM85X_DEF_VAL.reg_35 = (WM85X_DEF_VAL.reg_35 & 0x1f8) | NGTH;
			write_WM8510(35,WM85X_DEF_VAL.reg_35);
#ifdef _WM8510_DEBUG_			
			printk("Modified reg_35=0x%x\n",WM85X_DEF_VAL.reg_35);
#endif
		} else
			printk("ALC function disable or ALC in limited mode\n");
	}

#ifndef _WM8510_DEBUG_
	printk("\n");
#endif
	
	return;
}

//DAC oversample rate select. DACOSR:0->64x, 1->128x.
static void DAC_oversample_rate(unsigned char DACOSR)
{

#ifdef _WM8510_DEBUG_
	printk("Original reg_10=0x%x ",WM85X_DEF_VAL.reg_10);
#endif	
	WM85X_DEF_VAL.reg_10 = (WM85X_DEF_VAL.reg_10 & 0x1f7) | (DACOSR<<3);
	write_WM8510(10,WM85X_DEF_VAL.reg_10);
	if (DACOSR)
		printk("DAC oversample rate 128x ");
	else
		printk("DAC oversample rate 64x ");	
#ifdef _WM8510_DEBUG_
	printk("Modified reg_10=0x%x\n",WM85X_DEF_VAL.reg_10);
#endif

#ifndef _WM8510_DEBUG_
	printk("\n");
#endif

	return;
}

//DAC volume attenuate. DACVOL:0->mute, other->gain attenuate.
void DAC_volume_gain_ctrl(unsigned char DACVOL)
{

#ifdef _WM8510_DEBUG_
	printk("Original reg_11=0x%x ",WM85X_DEF_VAL.reg_11);
#endif	
	WM85X_DEF_VAL.reg_11 = (WM85X_DEF_VAL.reg_11 & 0x100) | DACVOL;
	write_WM8510(11,WM85X_DEF_VAL.reg_11);
#ifdef _WM8510_DEBUG_	
	printk("Modified reg_11=0x%x\n",WM85X_DEF_VAL.reg_11);
#endif
	return;
}

//DEEMPH:0->no de-emphasis, 1->32kHz, 2->44.1kHz, 3->48kHz
static void DAC_de_emphasis_ctrl(unsigned char DEEMPH)
{

#ifdef _WM8510_DEBUG_
	printk("Original reg_10=0x%x ",WM85X_DEF_VAL.reg_10);
#endif	
	WM85X_DEF_VAL.reg_10 = (WM85X_DEF_VAL.reg_10 & 0x1cf) | (DEEMPH<<4);
	write_WM8510(10,WM85X_DEF_VAL.reg_10);
#ifdef _WM8510_DEBUG_	
	printk("Modified reg_10=0x%x\n",WM85X_DEF_VAL.reg_10);
#endif
	return;
}	
	
//DAC soft mute. DACMU:0->not mute, 1->mute
static void DAC_soft_mute_ctrl(unsigned char DACMU)	
{

#ifdef _WM8510_DEBUG_
	printk("Original reg_10=0x%x ",WM85X_DEF_VAL.reg_10);
#endif	
	WM85X_DEF_VAL.reg_10 = (WM85X_DEF_VAL.reg_10 & 0x1bf) | (DACMU<<6);
	write_WM8510(10,WM85X_DEF_VAL.reg_10);
	if (DACMU)
		printk("DAC soft mute enable ");
	else
		printk("DAC soft mute disable ");
		
#ifdef _WM8510_DEBUG_	
	printk("Modified reg_10=0x%x\n",WM85X_DEF_VAL.reg_10);
#endif

#ifndef _WM8510_DEBUG_
	printk("\n");
#endif

	return;
}	

//Auto mute function when 1024 consecutive zeros are detected.
//AMUTE:0->disable auto mute, 1->enable auto mute
static void DAC_auto_mute_ctrl(unsigned char AMUTE)
{

#ifdef _WM8510_DEBUG_
	printk("Original reg_10=0x%x ",WM85X_DEF_VAL.reg_10);
#endif	
	WM85X_DEF_VAL.reg_10 = (WM85X_DEF_VAL.reg_10 & 0x1fb) | (AMUTE<<2);
	write_WM8510(10,WM85X_DEF_VAL.reg_10);
	if (AMUTE)
		printk("DAC auto mute enable ");
	else
		printk("DAC auto mute disable ");
			
#ifdef _WM8510_DEBUG_	
	printk("Modified reg_10=0x%x\n",WM85X_DEF_VAL.reg_10);
#endif

#ifndef _WM8510_DEBUG_
	printk("\n");
#endif

	return;
}	

//DAC limiter enable or not. LIMEN:0->disable, 1->enable.
//Signal level be operated. LIMLVL:signal dB.
//Limiter attack time. LIMATK:time.
//Limiter decay time. LIMDCY:time.
void DAC_limiter_control(unsigned char LIMEN, unsigned char LIMLVL, 
						 unsigned char LIMATK, unsigned char LIMDCY)
{

#ifdef _WM8510_DEBUG_
	printk("Original reg_24=0x%x ",WM85X_DEF_VAL.reg_24);
#endif	
	WM85X_DEF_VAL.reg_24 = (WM85X_DEF_VAL.reg_24 & 0xff) | (LIMEN<<8);
	write_WM8510(24,WM85X_DEF_VAL.reg_24);
	
	if (LIMEN)
		printk("DAC limiter enable ");
	else
		printk("DAC limiter disable ");
#ifdef _WM8510_DEBUG_
	printk("Modified reg_24=0x%x\n",WM85X_DEF_VAL.reg_24);
#endif

	if (LIMEN) {
#ifdef _WM8510_DEBUG_
	printk("Original reg_24=0x%x ",WM85X_DEF_VAL.reg_24);
	printk("Original reg_25=0x%x\n",WM85X_DEF_VAL.reg_25);
#endif	
	WM85X_DEF_VAL.reg_24 = (WM85X_DEF_VAL.reg_24 & 0x100) | LIMATK | (LIMDCY<<4);
	write_WM8510(24,WM85X_DEF_VAL.reg_24);
	WM85X_DEF_VAL.reg_25 = (WM85X_DEF_VAL.reg_25 & 0x18f) | (LIMLVL<<4);
	write_WM8510(25,WM85X_DEF_VAL.reg_25);
#ifdef _WM8510_DEBUG_
	printk("Modified reg_24=0x%x ",WM85X_DEF_VAL.reg_24);
	printk("Modified reg_25=0x%x\n",WM85X_DEF_VAL.reg_25);
#endif	
	}

#ifndef _WM8510_DEBUG_
	printk("\n");
#endif

	return;
}

//Limiter volume boost.LIMBOOST:gain.
static void DAC_limiter_boost_setting(unsigned char LIMBOOST)
{

#ifdef _WM8510_DEBUG_
	printk("Original reg_25=0x%x ",WM85X_DEF_VAL.reg_25);
#endif	
	WM85X_DEF_VAL.reg_25 = (WM85X_DEF_VAL.reg_25 & 0x1f0) | LIMBOOST;
	write_WM8510(25,WM85X_DEF_VAL.reg_25);
#ifdef _WM8510_DEBUG_
	printk("Modified reg_25=0x%x\n",WM85X_DEF_VAL.reg_25);
#endif

	return;
}

//device:0->speaker, 1->mono. boost_en:0->disable boost, 1->enable boost.
static void analog_output_boost_ctrl(unsigned char device, unsigned char boost_en)
{
#ifdef _WM8510_DEBUG_
	printk("Original reg_49=0x%x ",WM85X_DEF_VAL.reg_49);
	printk("Original reg_1=0x%x ",WM85X_DEF_VAL.reg_1);
#endif
	if (device) {
		printk("Mono boost ");
		WM85X_DEF_VAL.reg_49 = (WM85X_DEF_VAL.reg_49 & 0x1f7) | (boost_en<<3);
		write_WM8510(49,WM85X_DEF_VAL.reg_49);
	} else {
		printk("Speaker boost ");
		WM85X_DEF_VAL.reg_49 = (WM85X_DEF_VAL.reg_49 & 0x1fb) | (boost_en<<2);
		write_WM8510(49,WM85X_DEF_VAL.reg_49);
	}
	//Depending on boost_en, set BUFDCOPEN.
	WM85X_DEF_VAL.reg_1 = (WM85X_DEF_VAL.reg_1 & 0xff) | (boost_en<<8);
	write_WM8510(1,WM85X_DEF_VAL.reg_1);
	if (boost_en)
		printk("enable ");
	else
		printk("disable ");	
#ifdef _WM8510_DEBUG_
	printk("Modified reg_49=0x%x ",WM85X_DEF_VAL.reg_49);
	printk("Modified reg_1=0x%x\n",WM85X_DEF_VAL.reg_1);
#endif

#ifndef _WM8510_DEBUG_
	printk("\n");
#endif

	return;
}

//device:0->speaker, 1->mono. enable:0->disable attenuation, 1->enable attenuation.
static void sidetone_attenuation_ctrl(unsigned char device, unsigned char enable)
{
#ifdef _WM8510_DEBUG_
	printk("Original reg_40=0x%x ",WM85X_DEF_VAL.reg_40);
#endif
	if (device) {
		printk("Mono mixer sidetone atten. ");
		WM85X_DEF_VAL.reg_40 = (WM85X_DEF_VAL.reg_40 & 0x1fb) | (enable<<2);
		write_WM8510(40,WM85X_DEF_VAL.reg_40);
	} else {
		printk("Speaker mixer sidetone atten. ");
		WM85X_DEF_VAL.reg_40 = (WM85X_DEF_VAL.reg_40 & 0x1fd) | (enable<<1);
		write_WM8510(40,WM85X_DEF_VAL.reg_40);
	}
	if (enable)
		printk("enable ");
	else
		printk("disable ");
#ifdef _WM8510_DEBUG_
	printk("Modified reg_40=0x%x\n",WM85X_DEF_VAL.reg_40);
#endif

#ifndef _WM8510_DEBUG_
	printk("\n");
#endif

	return;
}

//Input of speaker mixer select.input_select:0->DAC, 1->Bypass, 2->MIC2.
//device_en:0->disconnect, 1->connect.
//SPKATTN:0->disable attenuation, 1->enable attenuation.
static void Speaker_mixer_control(unsigned char input_select, unsigned char device_en, unsigned char SPKATTN)
{
#ifdef _WM8510_DEBUG_
	printk("Original reg_50=0x%x ",WM85X_DEF_VAL.reg_50);
#endif
	if (input_select == 0) {
		printk("DAC2SpeakerMixer ");
		WM85X_DEF_VAL.reg_50 = (WM85X_DEF_VAL.reg_50 & 0x1fe) | device_en;
		write_WM8510(50,WM85X_DEF_VAL.reg_50);
	} else if (input_select == 1) {
		printk("Bypass2SpeakerMixer ");
		WM85X_DEF_VAL.reg_50 = (WM85X_DEF_VAL.reg_50 & 0x1fd) | (device_en<<1);
		write_WM8510(50,WM85X_DEF_VAL.reg_50);
	} else if (input_select == 2) {
		printk("Mic2_2SpeakerMixer ");
		WM85X_DEF_VAL.reg_50 = (WM85X_DEF_VAL.reg_50 & 0x1df) | (device_en<<5);
		write_WM8510(50,WM85X_DEF_VAL.reg_50);
	}
	if (device_en)
		printk("connect ");
	else
		printk("disconnect ");	
#ifdef _WM8510_DEBUG_
	printk("Modified reg_50=0x%x\n",WM85X_DEF_VAL.reg_50);
#endif

#ifndef _WM8510_DEBUG_
	printk("\n");
#endif
	
	//set sidetone attenuation
	sidetone_attenuation_ctrl(0,SPKATTN);

	return;
}

//SPKMUTE:0->speaker output not mute, 1->speaker output mute.
//SPKVOL:volume gain. SPKZC:0->change gain immediately, 1->change gain on zero cross.
void Speaker_volume_control(unsigned char SPKMUTE, unsigned char SPKVOL, unsigned char SPKZC)
{

#ifdef _WM8510_DEBUG_
	printk("Original reg_54=0x%x ",WM85X_DEF_VAL.reg_54);
#endif
	WM85X_DEF_VAL.reg_54 = (WM85X_DEF_VAL.reg_54 & 0x1bf) | (SPKMUTE<<6);
	write_WM8510(54,WM85X_DEF_VAL.reg_54);
	if (!SPKMUTE) {
		printk("Speaker output enable ");
		WM85X_DEF_VAL.reg_54 = (WM85X_DEF_VAL.reg_54 & 0x140) | (SPKZC<<7) | SPKVOL;
		write_WM8510(54,WM85X_DEF_VAL.reg_54);
	} else
		printk("Speaker output mute ");

#ifdef _WM8510_DEBUG_
	printk("Modified reg_54=0x%x\n",WM85X_DEF_VAL.reg_54);
#endif

#ifndef _WM8510_DEBUG_
	printk("\n");
#endif

	return;
}

//Input of Mono mixer select.input_select:0->DAC, 1->Bypass, 2->MIC2.
//device_en:0->disconnect, 1->connect.
//MONOATTN:0->disable attenuation, 1->enable attenuation.
//MONOMUTE:0->Mono output not mute, 1->Mono output mute.
static void Mono_mixer_control(unsigned char input_select, unsigned char device_en,
						unsigned char MONOATTN, unsigned char MONOMUTE)

{

#ifdef _WM8510_DEBUG_
	printk("Original reg_56=0x%x ",WM85X_DEF_VAL.reg_56);
#endif
	if (input_select == 0) {
		printk("DAC2MonoMixer ");
		WM85X_DEF_VAL.reg_56 = (WM85X_DEF_VAL.reg_56 & 0x1fe) | device_en;
		write_WM8510(56,WM85X_DEF_VAL.reg_56);
	} else if (input_select == 1) {
		printk("Bypass2MonoMixer ");
		WM85X_DEF_VAL.reg_56 = (WM85X_DEF_VAL.reg_56 & 0x1fd) | (device_en<<1);
		write_WM8510(56,WM85X_DEF_VAL.reg_56);
	} else if (input_select == 2) {
		printk("Mic2_2MonoMixer ");
		WM85X_DEF_VAL.reg_56 = (WM85X_DEF_VAL.reg_56 & 0x1fb) | (device_en<<2);
		write_WM8510(56,WM85X_DEF_VAL.reg_56);
	}	
	if (device_en)
		printk("connect ");
	else
		printk("disconnect ");
		
	if (MONOMUTE) {
		printk("Mono output mute ");
		WM85X_DEF_VAL.reg_56 = (WM85X_DEF_VAL.reg_56 & 0x1bf) | (MONOMUTE<<6);
		write_WM8510(56,WM85X_DEF_VAL.reg_56);
	} else {
		printk("Mono output enable ");
		WM85X_DEF_VAL.reg_56 = (WM85X_DEF_VAL.reg_56 & 0x1bf) | (MONOMUTE<<6);
		write_WM8510(56,WM85X_DEF_VAL.reg_56);
	}


#ifdef _WM8510_DEBUG_
	printk("Modified reg_56=0x%x\n",WM85X_DEF_VAL.reg_56);
#endif

#ifndef _WM8510_DEBUG_
	printk("\n");
#endif

	//set sidetone attenuation
	sidetone_attenuation_ctrl(1,MONOATTN);
	return;
}

//device:0:->DAConverter, 1->SpeakerMixer, 2->MonoMixer, 3->SpeakerOutputP, 4->SpeakerOutputN, 5->MonoOutput.
//device_en:0->device disconnect, 1->device connect.
static void Analog_output_enable_ctrl(unsigned char device, unsigned char device_en) 
{

#ifdef _WM8510_DEBUG_
	printk("Original reg_3=0x%x ",WM85X_DEF_VAL.reg_3);
#endif
	if (device_en) {
		if (device == 0) {
			printk("DAConverter enable ");
			WM85X_DEF_VAL.reg_3 = (WM85X_DEF_VAL.reg_3 & 0x1fe) | device_en;
			write_WM8510(3,WM85X_DEF_VAL.reg_3);
		} else if (device == 1) {
			printk("SpeakerMixer enable ");
			WM85X_DEF_VAL.reg_3 = (WM85X_DEF_VAL.reg_3 & 0x1fb) | (device_en<<2);
			write_WM8510(3,WM85X_DEF_VAL.reg_3);		
		} else if (device == 2) {
			printk("MonoMixer enable ");
			WM85X_DEF_VAL.reg_3 = (WM85X_DEF_VAL.reg_3 & 0x1f7) | (device_en<<3);
			write_WM8510(3,WM85X_DEF_VAL.reg_3);		
		} else if (device == 3) {
			printk("SpeakerOutputP enable ");
			WM85X_DEF_VAL.reg_3 = (WM85X_DEF_VAL.reg_3 & 0x1df) | (device_en<<5);
			write_WM8510(3,WM85X_DEF_VAL.reg_3);
		} else if (device == 4) {
			printk("SpeakerOutputN enable ");
			WM85X_DEF_VAL.reg_3 = (WM85X_DEF_VAL.reg_3 & 0x1bf) | (device_en<<6);
			write_WM8510(3,WM85X_DEF_VAL.reg_3);
		} else if (device == 5) {
			printk("MonoOutput enable ");
			WM85X_DEF_VAL.reg_3 = (WM85X_DEF_VAL.reg_3 & 0x17f) | (device_en<<7);
			write_WM8510(3,WM85X_DEF_VAL.reg_3);
		}
	} else {
		if (device == 0) {
			printk("DAConverter disconnect ");
			WM85X_DEF_VAL.reg_3 = (WM85X_DEF_VAL.reg_3 & 0x1fe) | device_en;
			write_WM8510(3,WM85X_DEF_VAL.reg_3);
		} else if (device == 1) {
			printk("SpeakerMixer disconnect ");
			WM85X_DEF_VAL.reg_3 = (WM85X_DEF_VAL.reg_3 & 0x1fb) | (device_en<<2);
			write_WM8510(3,WM85X_DEF_VAL.reg_3);		
		} else if (device == 2) {
			printk("MonoMixer disconnect ");
			WM85X_DEF_VAL.reg_3 = (WM85X_DEF_VAL.reg_3 & 0x1f7) | (device_en<<3);
			write_WM8510(3,WM85X_DEF_VAL.reg_3);		
		} else if (device == 3) {
			printk("SpeakerOutputP disconnect ");
			WM85X_DEF_VAL.reg_3 = (WM85X_DEF_VAL.reg_3 & 0x1df) | (device_en<<5);
			write_WM8510(3,WM85X_DEF_VAL.reg_3);
		} else if (device == 4) {
			printk("SpeakerOutputN disconnect ");
			WM85X_DEF_VAL.reg_3 = (WM85X_DEF_VAL.reg_3 & 0x1bf) | (device_en<<6);
			write_WM8510(3,WM85X_DEF_VAL.reg_3);
		} else if (device == 5) {
			printk("MonoOutput disconnect ");
			WM85X_DEF_VAL.reg_3 = (WM85X_DEF_VAL.reg_3 & 0x17f) | (device_en<<7);
			write_WM8510(3,WM85X_DEF_VAL.reg_3);
		}	
	}

#ifdef _WM8510_DEBUG_
	printk("Modified reg_3=0x%x\n",WM85X_DEF_VAL.reg_3);
#endif

#ifndef _WM8510_DEBUG_
	printk("\n");
#endif

	return;
}

//device:0->unused I/O tie off buffer, 1->Analog amplifier bias, 2->Mic bias.
//device_en:0->disable, 1->enable.
static void Power_management_ctrl(unsigned char device, unsigned char device_en)
{

#ifdef _WM8510_DEBUG_
	printk("Original reg_1=0x%x ",WM85X_DEF_VAL.reg_1);
#endif
	if (device_en) {
		if (device == 0) {
			printk("Unused I/O tie-off buffer enable ");
			WM85X_DEF_VAL.reg_1 = (WM85X_DEF_VAL.reg_1 & 0x1fb) | (device_en<<2);
			write_WM8510(1,WM85X_DEF_VAL.reg_1);
		} else if (device == 1) {
			printk("Analog amplifier bias enable ");
			WM85X_DEF_VAL.reg_1 = (WM85X_DEF_VAL.reg_1 & 0x1f7) | (device_en<<3);
			write_WM8510(1,WM85X_DEF_VAL.reg_1);
		} else if (device == 2) {
			printk("MIC bias enable ");
			WM85X_DEF_VAL.reg_1 = (WM85X_DEF_VAL.reg_1 & 0x1ef) | (device_en<<4);
			write_WM8510(1,WM85X_DEF_VAL.reg_1);
		}	
	} else {
		if (device == 0) {
			printk("Unused I/O tie-off buffer disable ");
			WM85X_DEF_VAL.reg_1 = (WM85X_DEF_VAL.reg_1 & 0x1fb) | (device_en<<2);
			write_WM8510(1,WM85X_DEF_VAL.reg_1);
		} else if (device == 1) {
			printk("Analog amplifier bias disable ");
			WM85X_DEF_VAL.reg_1 = (WM85X_DEF_VAL.reg_1 & 0x1f7) | (device_en<<3);
			write_WM8510(1,WM85X_DEF_VAL.reg_1);
		} else if (device == 2) {
			printk("MIC bias disable ");
			WM85X_DEF_VAL.reg_1 = (WM85X_DEF_VAL.reg_1 & 0x1ef) | (device_en<<4);
			write_WM8510(1,WM85X_DEF_VAL.reg_1);
		}		
	}

#ifdef _WM8510_DEBUG_
	printk("Modified reg_1=0x%x\n",WM85X_DEF_VAL.reg_1);
#endif

#ifndef _WM8510_DEBUG_
	printk("\n");
#endif

	return;
}

//VROI:0->resistance about 1k ohm, 1->resistance about 30k ohm.
static void Analog_output_resistance(unsigned char VROI)
{

#ifdef _WM8510_DEBUG_
	printk("Original reg_49=0x%x ",WM85X_DEF_VAL.reg_49);
#endif
	WM85X_DEF_VAL.reg_49 = (WM85X_DEF_VAL.reg_49 & 0x1fe) | VROI;
	write_WM8510(49,WM85X_DEF_VAL.reg_49);
	if (VROI)
		printk("Output resistance is 30k ohm ");
	else
		printk("Output resistance is 1k ohm ");
			
#ifdef _WM8510_DEBUG_
	printk("Modified reg_49=0x%x\n",WM85X_DEF_VAL.reg_49);
#endif

#ifndef _WM8510_DEBUG_
	printk("\n");
#endif

	return;
}

//TSDEN:0->thermal shutdown protection disable, 1->thermal shutdown protection enable.
static void Thermal_shutdown_ctrl(unsigned char TSDEN)
{

#ifdef _WM8510_DEBUG_
	printk("Original reg_49=0x%x ",WM85X_DEF_VAL.reg_49);
#endif
	WM85X_DEF_VAL.reg_49 = (WM85X_DEF_VAL.reg_49 & 0x1fd) | (TSDEN<<1);
	write_WM8510(49,WM85X_DEF_VAL.reg_49);
	if (TSDEN)
		printk("Thermal shutdown protection enable ");
	else
		printk("Thermal shutdown protection disable ");

#ifdef _WM8510_DEBUG_
	printk("Modified reg_49=0x%x\n",WM85X_DEF_VAL.reg_49);
#endif

#ifndef _WM8510_DEBUG_
	printk("\n");
#endif

	return;
}

//The polarity of FRAME sync. and BCLK is kept normal.
//FMT:0->right justified, 1->left justified, 2->I2S format, 3->DSP/PCM mode.
//WL(data length):0->16 bits, 1->20 bits, 2->24 bits, 3->32 bits.
//ADCLRSWAP:0->ADC data appear in "left" phase of FRAME, 1->ADC data appear in "right" phase of FRAME.
//DACLRSWAP:0->DAC data appear in "left" phase of FRAME, 1->DAC data appear in "right" phase of FRAME.
static void Audio_interface_setting(unsigned char FMT, unsigned char WL, unsigned char ADCLRSWAP, unsigned char DACLRSWAP)
{

#ifdef _WM8510_DEBUG_
	printk("Original reg_4=0x%x ",WM85X_DEF_VAL.reg_4);
#endif
	
	WM85X_DEF_VAL.reg_4 = (WM85X_DEF_VAL.reg_4 & 0x1e7) | (FMT<<3);
	write_WM8510(4,WM85X_DEF_VAL.reg_4);
	if (FMT == 0)
		printk("Audio interface is Right Justified, ");
	else if (FMT == 1)
		printk("Audio interface is Left Justified, ");
	else if (FMT == 2)
		printk("Audio interface is I2S format, ");
	else if (FMT == 3)
		printk("Audio interface is DSP/PCM mode, ");

	WM85X_DEF_VAL.reg_4 = (WM85X_DEF_VAL.reg_4 & 0x19f) | (WL<<5);
	write_WM8510(4,WM85X_DEF_VAL.reg_4);
	if (WL == 0)
		printk("Data length 16 bits, ");
	else if (WL == 1)
		printk("Data length 20 bits, ");
	else if (WL == 2)
		printk("Data length 24 bits, ");
	else if (WL == 3)
		printk("Data length 32 bits, ");
		
	WM85X_DEF_VAL.reg_4 = (WM85X_DEF_VAL.reg_4 & 0x1fd) | (ADCLRSWAP<<1);
	write_WM8510(4,WM85X_DEF_VAL.reg_4);
	if (ADCLRSWAP)
		printk("ADC data appear in 'right' phase of FRAME, ");
	else
		printk("ADC data appear in 'left' phase of FRAME, ");
	
	WM85X_DEF_VAL.reg_4 = (WM85X_DEF_VAL.reg_4 & 0x1fb) | (DACLRSWAP<<2);
	write_WM8510(4,WM85X_DEF_VAL.reg_4);
	if (DACLRSWAP)
		printk("DAC data appear in 'right' phase of FRAME, ");
	else
		printk("DAC data appear in 'left' phase of FRAME, ");
				
	WM85X_DEF_VAL.reg_4 = (WM85X_DEF_VAL.reg_4 & 0x17f) | (FRAME_POLARITY<<7);
	write_WM8510(4,WM85X_DEF_VAL.reg_4);
	if (!FRAME_POLARITY)
		printk("FRAME polarity is normal ");
	else
		printk("FRAME polarity is inverted ");
	
	WM85X_DEF_VAL.reg_4 = (WM85X_DEF_VAL.reg_4 & 0x0ff) | (BCLK_POLARITY<<8);
	write_WM8510(4,WM85X_DEF_VAL.reg_4);
	if (!BCLK_POLARITY)
		printk("BCLK polarity is normal ");
	else
		printk("BCLK polarity is inverted ");		
#ifdef _WM8510_DEBUG_
	printk("Modified reg_4=0x%x\n",WM85X_DEF_VAL.reg_4);
#endif

#ifndef _WM8510_DEBUG_
	printk("\n");
#endif

	return;
}

//ADC_COMP:0->linear, 2->u-law, 3->A-law.
//DAC_COMP:0->linear, 2->u-law, 3->A-law.
static void Audio_companding_setting(unsigned char ADC_COMP, unsigned char DAC_COMP)
{

#ifdef _WM8510_DEBUG_
	printk("Original reg_5=0x%x ",WM85X_DEF_VAL.reg_5);
#endif
	WM85X_DEF_VAL.reg_5 = (WM85X_DEF_VAL.reg_5 & 0x1f9) | (ADC_COMP<<1);
	write_WM8510(5,WM85X_DEF_VAL.reg_5);
	if (ADC_COMP == 0)
		printk("ADC companding mode is linear ");
	else if (ADC_COMP == 2)
		printk("ADC companding mode is u-law ");	
	else if (ADC_COMP == 3)
		printk("ADC companding mode is A-law ");
	
	WM85X_DEF_VAL.reg_5 = (WM85X_DEF_VAL.reg_5 & 0x1e7) | (DAC_COMP<<3);
	write_WM8510(5,WM85X_DEF_VAL.reg_5);
	if (DAC_COMP == 0)
		printk("DAC companding mode is linear ");
	else if (DAC_COMP == 2)
		printk("DAC companding mode is u-law ");	
	else if (DAC_COMP == 3)
		printk("DAC companding mode is A-law ");

#ifdef _WM8510_DEBUG_
	printk("Modified reg_5=0x%x\n",WM85X_DEF_VAL.reg_5);
#endif

#ifndef _WM8510_DEBUG_
	printk("\n");
#endif

	return;
}

// linear: 0 , a-law: 1, u-law: 2.
static unsigned char convert_table[]={LINEAR_MODE, A_LAW_MODE, u_LAW_MODE}; 
static void Set_wm8510_companding_mode(int law)
{
	if (law < 3)
		Audio_companding_setting(convert_table[law], convert_table[law]);
	return;
}

//LOOPBACK:0->no loopback, 1->loopback enable.
void Audio_interface_loopback_test(unsigned char LOOPBACK)
{

#ifdef _WM8510_DEBUG_
	printk("Original reg_5=0x%x ",WM85X_DEF_VAL.reg_5);
#endif
	WM85X_DEF_VAL.reg_5 = (WM85X_DEF_VAL.reg_5 & 0x1fe) | LOOPBACK;
	write_WM8510(5,WM85X_DEF_VAL.reg_5);
	if (LOOPBACK)
		printk("Digital loopback enable ");
	else
		printk("Digital loopback disable ");	
	
#ifdef _WM8510_DEBUG_
	printk("Modified reg_5=0x%x\n",WM85X_DEF_VAL.reg_5);
#endif

#ifndef _WM8510_DEBUG_
	printk("\n");
#endif

	return;
}

//SR:0->48kHz, 1->32kHz, 2->24kHz, 3->16kHz, 4->12kHz, 5->8kHz.
static void Audio_sample_frequency(unsigned char SR)
{

#ifdef _WM8510_DEBUG_
	printk("Original reg_7=0x%x ",WM85X_DEF_VAL.reg_7);
#endif
	WM85X_DEF_VAL.reg_7 = (WM85X_DEF_VAL.reg_7 & 0x1f1) | (SR<<1);
	write_WM8510(7,WM85X_DEF_VAL.reg_7);
	if (SR == 0)
		printk("Audio sample frequency:48kHz "); 
	else if (SR == 1)
		printk("Audio sample frequency:32kHz "); 
	else if (SR == 2)
		printk("Audio sample frequency:24kHz "); 
	else if (SR == 3)
		printk("Audio sample frequency:16kHz "); 
	else if (SR == 4)
		printk("Audio sample frequency:12kHz "); 
	else if (SR == 5)
		printk("Audio sample frequency:8kHz "); 
	
#ifdef _WM8510_DEBUG_
	printk("Modified reg_7=0x%x\n",WM85X_DEF_VAL.reg_7);
#endif

#ifndef _WM8510_DEBUG_
	printk("\n");
#endif

	return;
}

//MS:0->BCLK and FRAME are inputs, 1->BCLK and FRAME are outputs.
//CLKSEL:0->MCLK, 1->PLL output.
//MCLKDIV:0->MCLK/1, 1->MCLK/1.5, 2->MCLK/2, 3->MCLK/3, 4->MCLK/4, 5->MCLK/6, 6->MCLK/8, 7->MCLK/12.
static void Audio_clock_setting(unsigned char MS, unsigned char CLKSEL, unsigned char MCLKDIV)
{

#ifdef _WM8510_DEBUG_
	printk("Original reg_6=0x%x ",WM85X_DEF_VAL.reg_6);
#endif
	WM85X_DEF_VAL.reg_6 = (WM85X_DEF_VAL.reg_6 & 0x1fe) | MS;
	write_WM8510(6,WM85X_DEF_VAL.reg_6);
	if (MS)
		printk("BCLK and FRAME are outputs ");
	else
		printk("BCLK and FRAME are inputs ");	

	WM85X_DEF_VAL.reg_6 = (WM85X_DEF_VAL.reg_6 & 0x0ff) | (CLKSEL<<8);
	write_WM8510(6,WM85X_DEF_VAL.reg_6);
	if (CLKSEL)
		printk("PLL output ");
	else
		printk("MCLK ");
	
	WM85X_DEF_VAL.reg_6 = (WM85X_DEF_VAL.reg_6 & 0x11f) | (MCLKDIV<<5);
	write_WM8510(6,WM85X_DEF_VAL.reg_6);
	switch (MCLKDIV) {
		case 0:
			printk("MCLK/1 ");
		break;	
		case 1:
			printk("MCLK/1.5 ");
		break;
		case 2:
			printk("MCLK/2 ");
		break;
		case 3:
			printk("MCLK/3 ");
		break;
		case 4:
			printk("MCLK/4 ");
		break;
		case 5:
			printk("MCLK/6 ");
		break;
		case 6:
			printk("MCLK/8 ");
		break;
		case 7:
			printk("MCLK/12 ");
		break;
	} 	
#ifdef _WM8510_DEBUG_
	printk("Modified reg_6=0x%x\n",WM85X_DEF_VAL.reg_6);
#endif

#ifndef _WM8510_DEBUG_
	printk("\n");
#endif

	return;
}

//VMIDSEL:0->open circuit(off), 1->75k ohm, 2->300k ohm(the slowest startup), 3->2.5k ohm(the fastest startup).
static void Power_VMID_resistor(unsigned char VMIDSEL)
{

#ifdef _WM8510_DEBUG_
	printk("Original reg_1=0x%x ",WM85X_DEF_VAL.reg_1);
#endif
	WM85X_DEF_VAL.reg_1 = (WM85X_DEF_VAL.reg_1 & 0x1fc) | VMIDSEL;
	write_WM8510(1,WM85X_DEF_VAL.reg_1);
	if (VMIDSEL == 0)
		printk("VMID open circuit ");
	else if (VMIDSEL == 1)
		printk("VMID connects 75k ohm resistor ");
	else if (VMIDSEL == 2)
		printk("VMID connects 300k ohm resistor ");
	else if (VMIDSEL == 3)
		printk("VMID connects 2.5k ohm resistor ");
					
#ifdef _WM8510_DEBUG_
	printk("Modified reg_1=0x%x\n",WM85X_DEF_VAL.reg_1);
#endif

#ifndef _WM8510_DEBUG_
	printk("\n");
#endif

	return;
}

//undefined register?
static void WM8510_special_register(void)
{
	write_WM8510(18,0x12c);
	write_WM8510(19,0x2c);
	write_WM8510(20,0x2c);
	write_WM8510(21,0x2c);
	write_WM8510(22,0x2c);
	return;
}
//device:0->speaker, 1->mono.
//output_boost_stage:0->output DC level is 0.5*AVDD, 1->output DC level is 0.75*AVDD
//void WM8510_init(unsigned char device, unsigned char output_boost_stage)
void WM8510_init(int law)
{
	unsigned int i;
	//reset WM8510
	WM8510_software_reset();
	
	//enable speaker output boost.
#if defined(CONFIG_RTK_VOIP_GPIO_IPP_100)
	analog_output_boost_ctrl(SPEAKER_OUTPUT_BOOST,1);
#elif defined(CONFIG_RTK_VOIP_GPIO_IPP_101)
	analog_output_boost_ctrl(SPEAKER_OUTPUT_BOOST,0);
#endif
	//enable mono output boost
#if defined(CONFIG_RTK_VOIP_GPIO_IPP_100)
	analog_output_boost_ctrl(MONO_OUTPUT_BOOST,0);
#elif defined(CONFIG_RTK_VOIP_GPIO_IPP_101)
	analog_output_boost_ctrl(MONO_OUTPUT_BOOST,1);
#endif
	//enable Bias voltage
	Power_management_ctrl(ANALOG_OP_BIAS,1);	
	//set BUFIOEN = 1
	Power_management_ctrl(BUFFER_I_O_TIE,1);
	//set fast startup
	Power_VMID_resistor(3);
	//enable thermal shutdown protection
	Thermal_shutdown_ctrl(1);
	//unused analog output resistor.1k ohm
	Analog_output_resistance(0);
	//wait 500ms.	
//#ifdef __kernel_used__
	mdelay(500);
//#endif
#ifdef __test_program_used__
	for (i=0;i<100000;i++);
#endif	
	//set audio interface
	Audio_interface_setting(DSP_PCM_MODE, DATA_L16, ADC_LEFT_PHASE, DAC_LEFT_PHASE);	
	//set companding
	//Audio_companding_setting(u_LAW_MODE, u_LAW_MODE);
	Set_wm8510_companding_mode(law);
	//set clock (BCLK)
	Audio_clock_setting(BCLK_FRAME_INPUTS, CLKSEL_MCLK, 0);
	//set audio sample rate(FRAME SYNC.)
	Audio_sample_frequency(AUDIO_FS_8kHZ);
	//set ADConverter oversample rate
	ADC_oversample_rate(ADCOSR_128x);
	//set DAConverter oversample rate
	DAC_oversample_rate(DACOSR_128x);
	
	//set DAC gain 0dB
	DAC_volume_gain_ctrl(0xff);
	//enable DAC
	Analog_output_enable_ctrl(DACONVERTER, 1);
	//set speaker gain 0dB
	Speaker_volume_control(0, 0x39, 1);
	//select the input of speaker mixer
	Speaker_mixer_control(DAC2SPEAKER, 1, 1);
	//Bypass circuit input speaker mixer
#if defined(CONFIG_RTK_VOIP_GPIO_IPP_100)
	//Speaker_mixer_control(BYPASS2SPEAKER, 1, 1);
#elif defined(CONFIG_RTK_VOIP_GPIO_IPP_101)
	Speaker_mixer_control(BYPASS2SPEAKER, 1, 1);    //enable hand set sidetone
#endif
	//enable speaker mixer
	Analog_output_enable_ctrl(SPEAKER_MIXER, 1);
	//enable output stage
	Analog_output_enable_ctrl(SPEAKER_OUTPUT_P, 0);
	Analog_output_enable_ctrl(SPEAKER_OUTPUT_N, 0);
	//select the input of mono mixer
	Mono_mixer_control(DAC2MONO, 1, 1, 0);
	//Bypass circuit input mono mixer
#if defined(CONFIG_RTK_VOIP_GPIO_IPP_100)
	Mono_mixer_control(BYPASS2MONO, 1, 1, 0);    //enable hand set sidetone
#elif defined(CONFIG_RTK_VOIP_GPIO_IPP_101)
	//Mono_mixer_control(BYPASS2MONO, 1, 1, 0);
#endif
	//enable mono mixer
	Analog_output_enable_ctrl(MONO_MIXER, 1);
	//enable output stage
	Analog_output_enable_ctrl(MONO_OUTPUT, 1);
	//unmute DAC
	DAC_soft_mute_ctrl(0);
	
	//set ADC gain 0dB
	ADC_volume_gain_ctrl(0xff);
	//enable ADC 
	Analog_input_enable_ctrl(ADCONVERTER, 1);
	//set input PGA connect and set PGA2Boost gain 0dB
	input_PGA_2Boost(0);
	//enable input Boost stage
	Analog_input_enable_ctrl(INPUT_BOOST, 1);
	//set input PGA gain 0dB
	//mic_gain_control(0x3f, 1);
	//mic_gain_control(0x20, 1);
	mic_gain_control(0x30, 1);
	//mic_gain_control(0x10, 1);
	//enable mic input high pass filter.
	ADC_high_pass_filter_ctrl(1, 1, 4);
	//enable input PGA
	Analog_input_enable_ctrl(INPUT_MIC_PGA, 1);
	//select MICP/N
	mic_input_interface(0, 0, 1);
	//enable mic bias,and set bias 0.9*AVDD
	Power_management_ctrl(MIC_BIAS, 1);
	mic_bias_boost_setting(0); 
	
	WM8510_special_register();	
	return;
}

void AI_AO_select(unsigned char type)
{
/* IPP ver.100 handset (MIC2_MONO) using  MONO_OUTPUT, MIC1 P/N
	       handfree (MIC1_SPEAKER) using SPEAKER_OUTPUT_P, SPEAKER_OUTPUT_N, MIC2 mono

   IPP ver.101 handset (MIC2_MONO) using SPEAKER_OUTPUT_P, SPEAKER_OUTPUT_N, MIC1 P/N
	       handfree (MIC1_SPEAKER) using MONO_OUTPUT, MIC2 mono
 */

#if defined(CONFIG_RTK_VOIP_GPIO_IPP_100)
	if (type == MIC1_SPEAKER) {
		mic_input_interface(0, 0, 1);		/* MIC2 */
		input_PGA_2Boost(0);	/* set PGA2Boost gain 20dB */
		//disable MONO
		Analog_output_enable_ctrl(MONO_OUTPUT, 0);
		//select speaker
		Analog_output_enable_ctrl(SPEAKER_OUTPUT_P, 1);
		Analog_output_enable_ctrl(SPEAKER_OUTPUT_N, 1);
		Speaker_volume_control(0, 0x39, 1);
		mic_gain_control(0x3f, 1);
		//Speaker_volume_control(0, 0x30, 1);
		iphone_handfree=1;
	} else if (type == MIC2_MONO) {
		Speaker_volume_control(0, 0x30, 1);
		mic_gain_control(0x30, 1);
		mic_input_interface(1, 1, 0);		/* MIC1 */
		input_PGA_2Boost(0);	/* set PGA2Boost gain 0dB */
		//disable speaker
		Analog_output_enable_ctrl(SPEAKER_OUTPUT_P, 0);
		Analog_output_enable_ctrl(SPEAKER_OUTPUT_N, 0);  
		//select MONO
		Analog_output_enable_ctrl(MONO_OUTPUT, 1);
		iphone_handfree=0;

	} else if (type == SPEAKER_ONLY) {
		Speaker_volume_control(0, 0x39, 1);
		//deselect MIC1/MIC2
		mic_input_interface(0, 0, 0);
		input_PGA_2Boost(0);	/* set PGA2Boost gain 0dB */
		//deselect MONO
		Analog_output_enable_ctrl(MONO_OUTPUT, 0);
		//enable speaker
		Analog_output_enable_ctrl(SPEAKER_OUTPUT_P, 1);
		Analog_output_enable_ctrl(SPEAKER_OUTPUT_N, 1);  
		iphone_handfree=0;
	} else if (type == MONO_ONLY) {
		//deselect MIC1/MIC2
		mic_input_interface(0, 0, 0);
		input_PGA_2Boost(0);	/* set PGA2Boost gain 0dB */
		//select MONO
		Analog_output_enable_ctrl(MONO_OUTPUT, 1);
		//disable speaker
		Analog_output_enable_ctrl(SPEAKER_OUTPUT_P, 0);
		Analog_output_enable_ctrl(SPEAKER_OUTPUT_N, 0);  
		iphone_handfree=0;

#elif defined(CONFIG_RTK_VOIP_GPIO_IPP_101)

	if (type == MIC2_MONO) {
		mic_input_interface(1, 1, 0);	/* MIC1 */
		input_PGA_2Boost(0);	/* set PGA2Boost gain 20dB */
		//disable MONO
		Analog_output_enable_ctrl(MONO_OUTPUT, 0);
		//select speaker
		Analog_output_enable_ctrl(SPEAKER_OUTPUT_P, 1);
		Analog_output_enable_ctrl(SPEAKER_OUTPUT_N, 1);
		Speaker_volume_control(0, 0x30, 1);
		mic_gain_control(0x30, 1);
		iphone_handfree=0;
	} else if (type == MIC1_SPEAKER) {
		//Speaker_volume_control(0, 0x39, 1);	//handfree using MONO_OUTPUT, this command 
							//change SPEAKER_OUTPUT_P SPEAKER_OUTPUT_N gain.
		mic_gain_control(0x35, 1);
		mic_input_interface(0, 0, 1);	/* MIC2 */
		input_PGA_2Boost(0);	/* set PGA2Boost gain 0dB */
		//disable speaker
		Analog_output_enable_ctrl(SPEAKER_OUTPUT_P, 0);
		Analog_output_enable_ctrl(SPEAKER_OUTPUT_N, 0);  
		//select MONO
		Analog_output_enable_ctrl(MONO_OUTPUT, 1);
		iphone_handfree=1;  
	} else if (type == MONO_ONLY) {
		//deselect MIC1/MIC2
		mic_input_interface(0, 0, 0);
		input_PGA_2Boost(0);	/* set PGA2Boost gain 0dB */
		//deselect MONO
		Analog_output_enable_ctrl(MONO_OUTPUT, 0);
		//enable speaker
		Analog_output_enable_ctrl(SPEAKER_OUTPUT_P, 1);
		Analog_output_enable_ctrl(SPEAKER_OUTPUT_N, 1);  
		iphone_handfree=0;
	} else if (type == SPEAKER_ONLY) {
		//Speaker_volume_control(0, 0x39, 1);	//handfree using MONO_OUTPUT, this command 
							//change SPEAKER_OUTPUT_P SPEAKER_OUTPUT_N gain.
		//deselect MIC1/MIC2
		mic_input_interface(0, 0, 0);
		input_PGA_2Boost(0);	/* set PGA2Boost gain 0dB */
		//select MONO
		Analog_output_enable_ctrl(MONO_OUTPUT, 1);
		//disable speaker
		Analog_output_enable_ctrl(SPEAKER_OUTPUT_P, 0);
		Analog_output_enable_ctrl(SPEAKER_OUTPUT_N, 0);  
		iphone_handfree=0;
#endif

	} else 
		printk("No such input and output.\n");
	return;
}

