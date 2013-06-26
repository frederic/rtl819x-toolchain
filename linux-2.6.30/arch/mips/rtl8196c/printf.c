/*
 *
 *  Copyright (c) 2011 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <asm/io.h>
#include <asm/system.h>

#include <prom.h>
#include <platform.h>

void  __init prom_console_init(void)
{
   /* 8 bits, 1 stop bit, no parity. */
   REG8(UART0_LCR) = CHAR_LEN_8 | ONE_STOP | PARITY_DISABLE;

   /* Reset/Enable the FIFO */
   REG8(UART0_FCR) = FCR_EN | RXRST | TXRST | CHAR_TRIGGER_14;

   /* Disable All Interrupts */
   REG8(UART0_IER) = 0x00000000;

   /* Enable Divisor Latch */
   REG8(UART0_LCR) |= DLAB;

   /* Set Divisor */
   REG8(UART0_DLL) = (SYSCLK / (BAUDRATE * 16) - 1) & 0x00FF;
   REG8(UART0_DLM) = ((SYSCLK / (BAUDRATE * 16) - 1) & 0xFF00) >> 8;

   /* Disable Divisor Latch */
   REG8(UART0_LCR) &= (~DLAB);
}

int prom_putchar(char c)
{
   unsigned int busy_cnt = 0;

   do
   {
      /* Prevent Hanging */
      if (busy_cnt++ >= 30000)
      {
         /* Reset Tx FIFO */
         REG8(UART0_FCR) = TXRST | CHAR_TRIGGER_14;
         return 0;
      }
   } while ((REG8(UART0_LSR) & LSR_THRE) == TxCHAR_AVAIL);

   /* Send Character */
   REG8(UART0_THR) = c;

   return 1;
}
