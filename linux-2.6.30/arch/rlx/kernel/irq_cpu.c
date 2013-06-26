/*
 * Copyright 2001 MontaVista Software Inc.
 * Author: Jun Sun, jsun@mvista.com or jsun@junsun.net
 *
 * Copyright (C) 2001 Ralf Baechle
 * Copyright (C) 2005  MIPS Technologies, Inc.  All rights reserved.
 *      Author: Maciej W. Rozycki <macro@mips.com>
 *
 * This file define the irq handler for MIPS CPU interrupts.
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */

/*
 * Almost all MIPS CPUs define 8 interrupt sources.  They are typically
 * level triggered (i.e., cannot be cleared from CPU; must be cleared from
 * device).  The first two are software interrupts which we don't really
 * use or support.  The last one is usually the CPU timer interrupt if
 * counter register is present or, for CPUs with an external FPU, by
 * convention it's the FPU exception interrupt.
 *
 * Don't even think about using this on SMP.  You have been warned.
 *
 * This file exports one global function:
 *	void rlx_cpu_irq_init(int irq_base);
 */
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>

#include <asm/irq_cpu.h>
#include <asm/rlxregs.h>
#include <asm/system.h>
#include <net/rtl/rtl_types.h>

#include "bspchip.h"

__IRAM_GEN
static inline void unmask_rlx_irq(unsigned int irq)
{
 	set_c0_status(0x100 << (irq - BSP_IRQ_CPU_BASE));
	irq_enable_hazard();
}

__IRAM_GEN
static inline void mask_rlx_irq(unsigned int irq)
{
	clear_c0_status(0x100 << (irq - BSP_IRQ_CPU_BASE));
	irq_disable_hazard();
}

static struct irq_chip rlx_cpu_irq_controller = {
	.name		= "RLX",
	.ack		= mask_rlx_irq,
	.mask		= mask_rlx_irq,
	.mask_ack	= mask_rlx_irq,
	.unmask		= unmask_rlx_irq,
	.eoi		= unmask_rlx_irq,
};

void __init rlx_cpu_irq_init(int irq_base)
{
	int i;

	/* Mask interrupts. */
	clear_c0_status(ST0_IM);
	clear_c0_cause(CAUSEF_IP);

	for (i = irq_base + 2; i < irq_base + BSP_IRQ_CPU_NUM; i++)
		set_irq_chip_and_handler(i, &rlx_cpu_irq_controller, handle_percpu_irq);

	#if 0
	/* enable global interrupt mask */
	REG32(BSP_GIMR) = BSP_TC0_IE | BSP_UART0_IE ;

	#ifdef CONFIG_SERIAL_RTL8198_UART1
	REG32(BSP_GIMR)	|= BSP_UART1_IE;
	#endif
	
	#if defined(CONFIG_RTL8192CD)
	#if (defined(CONFIG_RTL_8196C) || defined(CONFIG_RTL_8196CT) || defined(CONFIG_RTL_8196CS) || !defined(CONFIG_RTL_92D_DMDP))
	REG32(BSP_GIMR) |= (BSP_PCIE_IE);
	#endif
	#if defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN_D) || (defined(CONFIG_RTL_8198)&&defined(CONFIG_RTL_92D_SUPPORT))
	REG32(BSP_GIMR) |= (BSP_PCIE2_IE);
	#endif
	#endif
	
	#if defined(CONFIG_USB)
	REG32(BSP_GIMR) |= BSP_USB_H_IE;
	#endif
	
	#if defined(CONFIG_RTL_819X) || defined(CONFIG_RTL_ICTEST_SWITCH) || defined(CONFIG_RTL_865X_ETH)
	REG32(BSP_GIMR) |= (BSP_SW_IE);
	#endif
	
	#if defined(CONFIG_RTL_NFBI_MDIO)
	REG32(BSP_GIMR) |= BSP_NFBI_IE;
	#endif

	#ifdef CONFIG_RTL_8198_NFBI_BOARD
	setup_reboot_addr(0x80700000);
	#endif
	
	#if defined(CONFIG_RTK_VOIP)
	REG32(BSP_GIMR) |= (BSP_PCM_IE | BSP_I2S_IE);
	REG32(BSP_GIMR) |= (BSP_GPIO_ABCD_IE | BSP_GPIO_EFGH_IE); 
	#endif
	#endif
}
