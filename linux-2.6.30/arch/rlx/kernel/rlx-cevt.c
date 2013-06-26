/*
 *  DS1287 clockevent driver
 *
 *  Copyright (C) 2008  Yoichi Yuasa <yoichi_yuasa@tripeaks.co.jp>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */
#include <linux/clockchips.h>
#include <linux/init.h>
#include <linux/interrupt.h>

#include <asm/time.h>

#ifdef CONFIG_RTL_TIMER_ADJUSTMENT
#include <net/rtl/rtl_types.h>
#include <rtl865xc_asicregs.h>
#endif

#if defined(CONFIG_RTL_WTDOG)
	int is_fault=0; // kernel fault flag
#endif

#ifdef CONFIG_RTL_TIMER_ADJUSTMENT
#define MHZ 100
#define TIMER0_ADJUSTMENT_THRESHOLD 20


//static unsigned long tuRemainder=0;   /* unit is the same as TC0DATA, say "timer unit". */
static unsigned long tuRemainder=0;
#ifdef CONFIG_RTL8196C_REVISION_B
static unsigned long previousTC1CNT=0xFFFFFFF0; /* Previous value of Timer 1 */
#else
static unsigned long previousTC1CNT=0xFFFFFF00; /* Previous value of Timer 1 */
#endif
static u64 timer0AccJiffies=0;
static u64 timer1AccJiffies=0; /* accumulated jiffies, unit is the same as jiffies, 1/HZ. */
static unsigned int timer0IntCnt=0;
#define TICK_SIZE	(tick_nsec / 1000)
unsigned long rtl865x_getTimer1PassedJiffies(void)
{
	unsigned long jifPassed=0;
	unsigned long currTC1CNT;
	unsigned long tc0data;

	//tc0data = READ_MEM32(TC0DATA)>>TCD_OFFSET;
	tc0data =((MHZ * 250) / HZ);
	/* compute passed time since last time executed this function */
	#ifdef CONFIG_RTL8196C_REVISION_B
	currTC1CNT = READ_MEM32(TC1CNT) & 0xfffffff0;
	#else
	currTC1CNT = READ_MEM32(TC1CNT) & 0xffffff00;
	#endif

#if defined(CONFIG_RTL_819X)
	/*
		In RTL865xC, timer / counter is incremental
	*/
	#ifdef CONFIG_RTL8196C_REVISION_B
	if ( previousTC1CNT <= currTC1CNT )
	{
		/* No wrap happend. */
		tuRemainder += (currTC1CNT-previousTC1CNT)>>4; /* how many units are passed since last check? */
	}
	else
	{
		/* Timer1 wrapped!! */
		tuRemainder += (currTC1CNT+(0xfffffff0-previousTC1CNT)+(0x1<<4))>>4; /* how many units are passed since last check? */
	}
	#else
	if ( previousTC1CNT <= currTC1CNT )
	{
		/* No wrap happend. */
		tuRemainder += (currTC1CNT-previousTC1CNT)>>TCD_OFFSET; /* how many units are passed since last check? */
	}
	else
	{
		/* Timer1 wrapped!! */
		tuRemainder += (currTC1CNT+(0xffffff00-previousTC1CNT)+(0x1<<TCD_OFFSET))>>TCD_OFFSET; /* how many units are passed since last check? */
	}
	#endif
#endif
	previousTC1CNT = currTC1CNT; /* keep TC1CNT value for next time check */

	/* If tc0data is zero, it means 'time is frozen.' */
	if ( tc0data == 0 )
	{
		jifPassed = 0;
	}
	else
	{
		jifPassed = tuRemainder / tc0data;
		tuRemainder = tuRemainder % tc0data;
	}
	
	timer1AccJiffies += jifPassed;
 	return jifPassed;
}
#endif

int rlx_timer_state(void)
{
  return 0;
}

int rlx_timer_set_base_clock(unsigned int hz)
{
  return 0;
}

static int rlx_timer_set_next_event(unsigned long delta,
				 struct clock_event_device *evt)
{
  return -EINVAL;
}

static void rlx_timer_set_mode(enum clock_event_mode mode,
			    struct clock_event_device *evt)
{
  return;
}

static void rlx_timer_event_handler(struct clock_event_device *dev)
{
}

static struct clock_event_device rlx_clockevent = {
	.name		= "rlx timer",
	.features	= CLOCK_EVT_FEAT_PERIODIC,
	.set_next_event	= rlx_timer_set_next_event,
	.set_mode	= rlx_timer_set_mode,
	.event_handler	= rlx_timer_event_handler,
};

static irqreturn_t rlx_timer_interrupt(int irq, void *dev_id)
{
	struct clock_event_device *cd = &rlx_clockevent;
    	extern void bsp_timer_ack(void);

#if defined(CONFIG_RTL_WTDOG)
    	if (!is_fault){
  #ifdef CONFIG_RTK_VOIP
		extern int bBspWatchdog;
		
		*(volatile unsigned long *)(0xB800311c) |= 
					( bBspWatchdog ? ( 1 << 23 ) : ( ( 1 << 23 ) | ( 0xA5 << 24 ) ) );
  #else
		*(volatile unsigned long *)(0xB800311c) |=  1 << 23;
  #endif
	}else {
  #ifdef CONFIG_RTK_VOIP
		// run gdb cause Break instruction exception and call do_bp()
		extern int bBspWatchdog;
		
		if( !bBspWatchdog )
			is_fault = 0;
		else
  #endif
		{
		// quick fix for warn reboot fail issue
#if defined(CONFIG_RTL8192SE) || defined(CONFIG_RTL8192CD) || defined(CONFIG_RTL8192E)
#if !defined(CONFIG_RTL865X_PANAHOST) && !defined(CONFIG_RTL8197B_PANA)
		extern void force_stop_wlan_hw(void);
		force_stop_wlan_hw();
#endif
#endif
		local_irq_disable();	
		*(volatile unsigned long *)(0xB800311c)=0; /*this is to enable 865xc watch dog reset*/
		for(;;);
	    }
	}
#endif

#if defined(CONFIG_RTL_TIMER_ADJUSTMENT)
	/*notice:do_timer(1) will be called in tick_periodic()*/
	timer0AccJiffies++;
	timer0IntCnt++;
	if((timer0IntCnt%TIMER0_ADJUSTMENT_THRESHOLD)==0)
	{
		rtl865x_getTimer1PassedJiffies();
		if(timer1AccJiffies>timer0AccJiffies)
		{
			/*to compensate system jiffied, because timer0's interrupt may be disabled by nic driver*/
			//do_timer((timer1AccJiffies-timer0AccJiffies)+1);
			//printk("%s:%d,diff jiffies is %u\n",__FUNCTION__,__LINE__,(unsigned int)(timer1AccJiffies-timer0AccJiffies));
			do_timer((timer1AccJiffies-timer0AccJiffies));
			timer0AccJiffies=timer1AccJiffies;
		}
		else
		{
			/*for timer1 start later than timer0*/		
			if(timer1AccJiffies<timer0AccJiffies)
			{
				timer1AccJiffies=timer0AccJiffies;
			}
			//do_timer(1);
		}
	}
	else
	{
		//do_timer(1);
	}
#endif

	/* Ack the RTC interrupt. */
    	bsp_timer_ack();

	cd->event_handler(cd);
	return IRQ_HANDLED;
}

static struct irqaction rlx_irqaction = {
	.handler	= rlx_timer_interrupt,
	.flags		= IRQF_DISABLED | IRQF_PERCPU | IRQF_TIMER,
	.name		= "rlx timer",
};

int __init rlx_clockevent_init(int irq)
{
	struct clock_event_device *cd;

	cd = &rlx_clockevent;
	cd->rating = 100;
	cd->irq = irq;
	clockevent_set_clock(cd, 32768);
	cd->max_delta_ns = clockevent_delta2ns(0x7fffffff, cd);
	cd->min_delta_ns = clockevent_delta2ns(0x300, cd);
	cd->cpumask = cpumask_of(0);

	clockevents_register_device(&rlx_clockevent);

	return setup_irq(irq, &rlx_irqaction);
}
