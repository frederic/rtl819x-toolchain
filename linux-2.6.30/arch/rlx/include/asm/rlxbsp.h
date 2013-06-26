/*
 * Copyright (c) 2006, Realtek Semiconductor Corp.
 *
 * rlxbsp.h:
 *   board module interface
 *
 * Tony Wu (tonywu@realtek.com.tw)
 * Oct. 30, 2006
 */

#ifndef _RLXBSP_H_
#define _RLXBSP_H_

/*
 * Function prototypes
 */
#ifndef _LANGUAGE_ASSEMBLY

#include <linux/linkage.h>

/* arch/rlx/bsp/serial.c */
extern void bsp_serial_init(void);

/* arch/rlx/bsp/time.c */
extern void bsp_timer_init(void);
extern void bsp_timer_ack(void);

/* arch/rlx/kernel/time.c */
extern int rlx_clockevent_init(int irq);

#endif

#endif
