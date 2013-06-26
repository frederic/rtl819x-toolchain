#ifndef __LED_H__
#define __LED_H__

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8186
#if 0		/* 0:V210 EV Board ,1:E_version*/
#define _V210_Ed_ 
#else
#define _V210_EV_BOARD_
#endif
/* =================== For V210 EV Board LED Control ======================== */
#define GPAB_DIR  *((volatile unsigned int *)0xbd010124)
#define GPAB_DATA  *((volatile unsigned int *)0xbd010120)
#endif

#ifdef CONFIG_RTK_VOIP_GPIO_8962
#define PEFGH_CR  *((volatile unsigned int *)0xb800351c)
#define PEFGH_Ptype  *((volatile unsigned int *)0xb8003520)
#define PEFGH_DIR  *((volatile unsigned int *)0xb8003524)
#define PEFGH_DAT  *((volatile unsigned int *)0xb8003528)
#endif 

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8671
#define GPA_DIR  *((volatile unsigned int *)0xb9c01000)
#define GPA_DATA  *((volatile unsigned int *)0xb9c01004)
#define GPB_DIR  *((volatile unsigned int *)0xb9c01010)
#define GPB_DATA  *((volatile unsigned int *)0xb9c01014)

#endif

#define LED_BLINKING_FREQ	5	//50ms

/* led.c function prototype */

// FXS
void fxs_led_state(unsigned int chid, unsigned int state);

// FXO
#ifdef CONFIG_RTK_VOIP_DRIVERS_FXO
void fxo_led_state(unsigned int chid, unsigned int state);
#endif

// SIP
void sip_led_state(unsigned int state);

void led_state_watcher(unsigned int chid);

//void LED_Init(void);
/* ========================================================================== */

#endif // __LED_H__

