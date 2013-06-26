#include <linux/kernel.h>
#include <linux/timer.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/version.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
#include "bspchip.h"
#endif

//#include "Daa_api.h"
#include "rtk_voip.h"
#include "voip_types.h"
#include "voip_init.h"
//#ifdef CONFIG_RTK_VOIP_DRIVERS_SI3050
//#include "spi.h"
//#endif
//#if defined (CONFIG_RTK_VOIP_DRIVERS_SLIC_LE89116) || defined (CONFIG_RTK_VOIP_DRIVERS_SLIC_LE89316)
//#include "zarlink.h"
//#endif
#include "snd_mux_daa.h"
#include "con_mux.h"
#include "con_register.h"

#ifdef PULSE_DIAL_GEN

#define PULSE_DIAL_FIFO_SIZE 15
static char pulse_dial_fifo[CON_CH_NUM][PULSE_DIAL_FIFO_SIZE];//={0};
static uint8 pulse_dial_wp[CON_CH_NUM]={0},pulse_dial_rp[CON_CH_NUM]={0};

int pulse_dial_in_cch(uint32 ch_id, char input)
{

	if ( ( input < 0) || ( input > 9) )
	{
		//PRINT_R("Warning: error input %d for Pulse Dial FIFO\n", input);
		return -1;
	}
		
	if ((pulse_dial_wp[ch_id]+1)%PULSE_DIAL_FIFO_SIZE != pulse_dial_rp[ch_id])
	{
		if (input == 0)
			input = 10; // digit 0 = 10 pulse
			
		pulse_dial_fifo[ch_id][pulse_dial_wp[ch_id]] = input;
		pulse_dial_wp[ch_id] = (pulse_dial_wp[ch_id]+1) % PULSE_DIAL_FIFO_SIZE;
	}
	else
	{
		PRINT_R("Pulse Dial FIFO overflow,(%d)\n", ch_id);
	}

	return 0;
}

char pulse_dial_out_cch(uint32 ch_id)
{
	int output;

	if ( pulse_dial_wp[ch_id] == pulse_dial_rp[ch_id])
	{
		// FIFO empty
		return -1;
	}
	else
	{
		output = pulse_dial_fifo[ch_id][pulse_dial_rp[ch_id]];
                pulse_dial_rp[ch_id] = (pulse_dial_rp[ch_id]+1) % PULSE_DIAL_FIFO_SIZE;

                return output;
	}
}

int flush_pulse_dial_fifo(uint32 ch_id)
{
	int i;
	for (i=0; i< PULSE_DIAL_FIFO_SIZE; i++)
	{
		pulse_dial_fifo[ch_id][i] = -1;
	}
	pulse_dial_wp[ch_id] = 0;
	pulse_dial_rp[ch_id] = 0;
	
	return 0;
}

#endif


static unsigned int pulse_dial_flag[CON_CH_NUM]={0};	/* 0: disable 1: enable Pulse dial */

void DAA_Set_Dial_Mode(unsigned int cch, unsigned int mode)
{
	pulse_dial_flag[cch] = mode;
}

unsigned int DAA_Get_Dial_Mode(unsigned int cch)
{
	return pulse_dial_flag[cch];
}


#ifdef PULSE_DIAL_GEN // deinfe in rtk_voip.h


#ifdef CONFIG_RTK_VOIP_PULSE_DIAL_GEN_TIMER	// define in rtk_voip/Config.in

// For 10 pps, break time: 60ms, make time: 40ms
static unsigned int make_time = 40, break_time = 60;
// For 20 pps, break time: 30ms, make time: 20ms
//static unsigned int make_time = 2, break_time = 3;
static unsigned int interdigit_pause = 70;	// 700ms

int DAA_PulseDial_Gen_Cfg(unsigned int pps, unsigned int make_duration, unsigned int interdigit_duration)
{
	//int r;
	
	if ( (pps != 10) && (pps != 20) )
	{
		PRINT_R("Pulse Dial Gen Config Error, %dPPS is not supported\n", pps);
		return -1;
	}
	
	//PRINT_MSG("PPS=%d, make time=%d, interdigit pause=%d\n", pps, make_duration, interdigit_duration);
	
	if (pps == 10)
	{
		make_time = make_duration;
		interdigit_pause = interdigit_duration/10;

		
		break_time = 100 - make_time;
		
		if (break_time <= 0)
		{
			make_time = 40;
			break_time = 60;
			PRINT_R("Pulse Dial Gen Config Error, make_duration:%d larger than 100 msec for %d PPS\n", make_duration, pps);
			return -1;
		}
	}
	else if (pps == 20)
	{
		make_time = make_duration;
		interdigit_pause = interdigit_duration/10;
		
		break_time = 50 - make_time;
		
		if (break_time <= 0)
		{
			make_time = 20;
			break_time = 30;
			PRINT_R("Pulse Dial Gen Config Error, make_duration:%d larger than 50 msec for %d PPS\n", make_duration, pps);
			return -1;
		}
	}
	
	PRINT_MSG("PPS=%d, make time=%d, break time= %d, interdigit pause=%d\n", pps, make_time, break_time, interdigit_duration);
	
	return 0;
}

extern void timer1_enable(void);
extern void timer1_disable(void);
//extern void bus_set_tx_mute(unsigned char chid, int enable);
//extern void DisableDspInPcmIsr_cch(uint32 chid);
//extern void RestoreDspInPcmIsr_cch(uint32 chid);


/*************** For Pulse Dial generation ***************/
static int interdigit_time[CON_CH_NUM] = {0};
static int pulse_make_time[CON_CH_NUM] = {0};
static int pulse_break_time[CON_CH_NUM] = {0};
static int pulse_gen_start[CON_CH_NUM] = {0};
static int pulse_ONH[CON_CH_NUM] = {0}, pulse_OFH[CON_CH_NUM] = {0};
static char pulse_event[CON_CH_NUM] = {0}, pulse_count[CON_CH_NUM] = {0};

void DAA_PollPulseGenFifo(const voip_con_t *p_con)
{
	const uint32 cch = p_con ->cch;
	voip_dsp_t *p_dsp;
	
	if (pulse_gen_start[cch] == 0)
	{
		if (interdigit_time[cch] < (interdigit_pause))
			interdigit_time[cch]++;
		else
		{
			pulse_event[cch] = pulse_dial_out_cch(cch);
			if (pulse_event[cch] != -1)
			{
				//PRINT_G("Out!\n");
				pulse_gen_start[cch] = 1;
				
				pulse_break_time[cch] = 0;
				pulse_make_time[cch] = 0;
				pulse_count[cch] = 0;
				pulse_ONH[cch] = 0;
				pulse_OFH[cch] = 0;
				
				//DisableDspInPcmIsr_cch(cch);	// Disable the DSP in ISR for the channel which gen pulse dial
				p_dsp = p_con ->dsp_ptr;
				p_dsp ->dsp_ops ->disable_dsp_in_ISR( p_dsp );
				//bus_set_tx_mute(chid, 1); 	// Mute the pcm tx when pulse dial generation				
				p_con ->con_ops ->bus_fifo_set_tx_mute( p_con, 1 );
				timer1_enable();		// Enable timer 1 (1 msec) for pulse dial generation
			}
		}
	}
}

void DAA_PulseGenProcess_msec_con( const voip_con_t * const p_con )
{
	int cch = p_con ->cch;
	voip_snd_t * const p_snd = p_con ->snd_ptr;
	voip_dsp_t * p_dsp;
	
	if( Is_DAA_snd( p_snd ) )
		return;
	
	if (pulse_count[cch] < pulse_event[cch])
	{
		if ( pulse_OFH[cch] == 0 )
		{
			if ( pulse_ONH[cch] == 0 )
			{
				//DAA_On_Hook(chid);
				p_snd ->daa_ops ->DAA_On_Hook( p_snd );
				pulse_ONH[cch] = 1;
				//PRINT_R("N\n");
			}
			else
			{
				//PRINT_R("%d, %d\n",pulse_break_time[chid], break_time);
				if (pulse_break_time[cch] < (break_time))
				{
					pulse_break_time[cch]++;
					//PRINT_Y("b:%d\n", pulse_break_time[chid]);
				}
				
				if (pulse_break_time[cch] == (break_time))
				{
					pulse_ONH[cch] = 0;
					pulse_break_time[cch] = 0;
					//PRINT_Y("$b\n");
				}
			}
		}
			
		if ( pulse_ONH[cch] == 0 )
		{
			//PRINT_R("2\n");
			if ( pulse_OFH[cch] == 0 ) 
			{
				//DAA_Off_Hook(chid);
				p_snd ->daa_ops ->DAA_Off_Hook( p_snd );
				pulse_OFH[cch] = 1;
				//PRINT_R("F\n");
			}
			else 
			{
				if (pulse_make_time[cch] < (make_time-1))
				{
					pulse_make_time[cch]++;
					//PRINT_Y("m:%d\n", pulse_make_time[chid]);
				}
				
				if (pulse_make_time[cch] == (make_time-1))
				{
					pulse_OFH[cch] = 0;
					pulse_make_time[cch] = 0;
					pulse_count[cch]++;
					//PRINT_Y("$m\n");
				}			
			}
		}
	}
	else
	{
		pulse_gen_start[cch] = 0;
		pulse_count[cch] = 0;
		interdigit_time[cch] = 0;
		
		timer1_disable();		// Disable timer 1 (1 msec) for pulse dial generation
		//RestoreDspInPcmIsr_cch(cch);	// Restore the DSP in ISR for the channel which gen pulse dial
		p_dsp = p_con ->dsp_ptr;
		p_dsp ->dsp_ops ->restore_dsp_in_ISR( p_dsp );
		//bus_set_tx_mute(chid, 0);	// Un-mute the pcm tx when pulse dial generation
		p_con ->con_ops ->bus_fifo_set_tx_mute( p_con, 0 );
		
		
		//PRINT_Y("End\n");
	}
}


void DAA_PulseGenKill_con( voip_con_t * p_con )
{
	extern int flush_pulse_dial_fifo(uint32 ch_id);
	const uint32 cch = p_con ->cch;
	voip_dsp_t *p_dsp;
	
	flush_pulse_dial_fifo(cch);
	
	timer1_disable();		// Disable timer 1 (1 msec) for pulse dial generation
	//RestoreDspInPcmIsr_cch(cch);	// Restore the DSP in ISR for the channel which gen pulse dial
	p_dsp = p_con ->dsp_ptr;
	p_dsp ->dsp_ops ->restore_dsp_in_ISR( p_dsp );
	//bus_set_tx_mute(cch, 0);	// Un-mute the pcm tx when pulse dial generation
	p_con ->con_ops ->bus_fifo_set_tx_mute( p_con, 0 );
	
	pulse_gen_start[cch] = 0;
	pulse_count[cch] = 0;
	interdigit_time[cch] = 0;
}

#else //!CONFIG_RTK_VOIP_PULSE_DIAL_GEN_TIMER

???	// chmap FIXME: 
	
// For 10 pps, break time: 60ms, make time: 40ms
static unsigned int make_time = 4, break_time = 6;
// For 20 pps, break time: 30ms, make time: 20ms
//static unsigned int make_time = 2, break_time = 3;
static unsigned int interdigit_pause = 70;	// 700ms
static unsigned char delay_off = 0 ,delay_on = 0;

int DAA_PulseDial_Gen_Cfg(unsigned int pps, unsigned int make_duration, unsigned int interdigit_duration)
{
	int r;
	
	if ( (pps != 10) && (pps != 20) )
	{
		PRINT_R("Pulse Dial Gen Config Error, %dPPS is not supported\n", pps);
		return -1;
	}
	
	//PRINT_MSG("PPS=%d, make time=%d, interdigit pause=%d\n", pps, make_duration, interdigit_duration);
	
	r = make_duration%10;
	
	if (pps == 10)
	{
		make_time = make_duration/10;
		interdigit_pause = interdigit_duration/10 - 5;
		if (r > 5)
		{
			delay_off = 10 - r;
			delay_on = 0;
			make_time++;
		}
		else if ( r <= 5)
		{
			delay_off = 0;
			delay_on = r;
		}
		else if (r == 0)
		{
			delay_off = 0;
			delay_on = 0;
		}
		
		break_time = 10 - make_time;
					
		if (break_time <= 0)
		{
			make_time = 4;
			break_time = 6;
			delay_off = 0;
			delay_on = 0;
			PRINT_R("Pulse Dial Gen Config Error, make_duration:%d larger than 100 msec for %d PPS\n", make_duration, pps);
			return -1;
		}
	}
	else if (pps == 20)
	{
		make_time = make_duration/10;
		interdigit_pause = interdigit_duration/10 - 3;
		if (r > 5)
		{
			delay_off = 10 - r;
			delay_on = 0;
			make_time++;
		}
		else if ( r <= 5)
		{
			delay_off = 0;
			delay_on = r;
		}
		else if (r == 0)
		{
			delay_off = 0;
			delay_on = 0;
		}
		
		break_time = 5 - make_time;
					
		if (break_time <= 0)
		{
			make_time = 2;
			break_time = 3;
			delay_off = 0;
			delay_on = 0;
			PRINT_R("Pulse Dial Gen Config Error, make_duration:%d larger than 50 msec for %d PPS\n", make_duration, pps);
			return -1;
		}
	}
	
	PRINT_MSG("PPS=%d, make time=%d, break time= %d, interdigit pause=%d\n", pps, make_time*10, break_time*10, interdigit_duration);
	
	return 0;
}

/*************** For Pulse Dial generation ***************/
static int interdigit_time[CON_CH_NUM] = {0};
static int pulse_make_time[CON_CH_NUM] = {0};
static int pulse_break_time[CON_CH_NUM] = {0};
static int pulse_gen_start[CON_CH_NUM] = {0};
static int pulse_ONH[CON_CH_NUM] = {0}, pulse_OFH[CON_CH_NUM] = {0};
static char pulse_event[CON_CH_NUM] = {0}, pulse_count[CON_CH_NUM] = {0};

void DAA_PulseGenProcess(unsigned int chid)
{
	if (pulse_gen_start[chid] == 0)
	{
		if (interdigit_time[chid] < (interdigit_pause))
			interdigit_time[chid]++;
		else
		{
			pulse_event[chid] = pulse_dial_out_cch(chid);
			if (pulse_event[chid] != -1)
			{
				//PRINT_G("Out!\n");
				pulse_gen_start[chid] = 1;
				
				pulse_break_time[chid] = 0;
				pulse_make_time[chid] = 0;
				pulse_count[chid] = 0;
				pulse_ONH[chid] = 0;
				pulse_OFH[chid] = 0;
			}
		}
	}
	
	if (pulse_gen_start[chid] == 1)
	{
		if (pulse_count[chid] < pulse_event[chid])
		{
			if ( pulse_OFH[chid] == 0 )
			{
				if ( pulse_ONH[chid] == 0 )
				{
					if (delay_on != 0)
						mdelay(delay_on);
					DAA_On_Hook(chid);
					pulse_ONH[chid] = 1;
					//PRINT_R("N\n");
				}
				else
				{
					//PRINT_R("%d, %d\n",pulse_break_time[chid], break_time);
					if (pulse_break_time[chid] < (break_time))
					{
						pulse_break_time[chid]++;
						//PRINT_Y("b:%d\n", pulse_break_time[chid]);
					}
					
					if (pulse_break_time[chid] == (break_time))
					{
						pulse_ONH[chid] = 0;
						pulse_break_time[chid] = 0;
						//PRINT_Y("$b\n");
					}
				}
			}
				
			if ( pulse_ONH[chid] == 0 )
			{
				//PRINT_R("2\n");
				if ( pulse_OFH[chid] == 0 ) 
				{
					if(delay_off!=0)
						mdelay(delay_off);
					DAA_Off_Hook(chid);
					pulse_OFH[chid] = 1;
					//PRINT_R("F\n");
				}
				else 
				{
					if (pulse_make_time[chid] < (make_time-1))
					{
						pulse_make_time[chid]++;
						//PRINT_Y("m:%d\n", pulse_make_time[chid]);
					}
					
					if (pulse_make_time[chid] == (make_time-1))
					{
						pulse_OFH[chid] = 0;
						pulse_make_time[chid] = 0;
						pulse_count[chid]++;
						//PRINT_Y("$m\n");
					}			
				}
			}
		}
		else
		{
			pulse_gen_start[chid] = 0;
			pulse_count[chid] = 0;
			interdigit_time[chid] = 0;
			//PRINT_Y("End\n");
		}

	}
}

void DAA_PulseGenKill(unsigned int chid)
{
	extern int flush_pulse_dial_fifo(uint32 ch_id);
	
	flush_pulse_dial_fifo(chid);
	
	pulse_gen_start[chid] = 0;
	pulse_count[chid] = 0;
	interdigit_time[chid] = 0;
}

#endif // CONFIG_RTK_VOIP_PULSE_DIAL_GEN_TIMER
#endif // PULSE_DIAL_GEN

#ifdef CONFIG_RTK_VOIP_PULSE_DIAL_GEN_TIMER
#if defined (CONFIG_RTK_VOIP_DRIVERS_PCM865xC) || defined (CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY) || defined (CONFIG_RTK_VOIP_DRIVERS_PCM89xxC)
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
static irqreturn_t timer1_interrupt(int32 irq, void *dev_instance)
#else
static void timer1_interrupt(int32 irq, void *dev_instance, struct pt_regs *regs)
#endif
{
	unsigned int status;
	
	status = *(volatile unsigned long *)(0xB8003114); //read TCIR
	
	if (status & 0x10000000)    // timer 1 interrupt pending
    	{
	        *(volatile unsigned long *)(0xB8003114) = *(volatile unsigned long *)(0xB8003114) | 0x10000000; // claer timer1 pending interrupt
	        
	        //do pulse dial gen process
	        DAA_PulseGenProcess_msec();
   	}
   	
#ifdef CONFIG_DEFAULTS_KERNEL_2_6
	return IRQ_HANDLED;
#endif
}
#endif
#endif


#ifdef CONFIG_RTK_VOIP_PULSE_DIAL_GEN_TIMER
#if defined (CONFIG_RTK_VOIP_DRIVERS_PCM865xC) || defined (CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY) || defined(CONFIG_RTK_VOIP_DRIVERS_PCM89xxC)
static __init int timer1_init( void )
{
	int result;
	static char timer1_devices[] = "Timer1";
	char* timer1_dev = timer1_devices;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
	result = request_irq(BSP_TC1_IRQ, timer1_interrupt, IRQF_DISABLED, timer1_dev, timer1_dev);// NULL OK
#else
	result = request_irq(TIMER1_IRQ, timer1_interrupt, SA_INTERRUPT, timer1_dev, timer1_dev);// NULL OK
#endif
	if(result)
	{
		printk("Can't request IRQ for Timer1. %d\n", result);
		//PERR(KERN_ERR "Can't request IRQ for channel %d.\n", ch);
		return -1;
			
	}
	printk("Request IRQ for Timer1 OK!.\n");
	
	return 0;
}

voip_initcall( timer1_init );
//voip_exitcall( timer1_exit );
#endif
#endif

