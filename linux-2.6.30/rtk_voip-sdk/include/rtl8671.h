


#ifndef __RTL8671__
#define __RTL8671__

#include <asm/addrspace.h>


#if 1
#include "voip_addrspace.h"
#else
#define Virtual2Physical(x)		(((int)x) & 0x1fffffff)
#define Physical2Virtual(x)		(((int)x) | 0x80000000)
#define Virtual2NonCache(x)		(((int)x) | 0x20000000)
#define Physical2NonCache(x)		(((int)x) | 0xa0000000)
#endif

#if 0
#define RTL8181_REG_BASE (0xBD010000)

#define rtl_inb(offset) (*(volatile unsigned char *)(RTL8181_REG_BASE+offset))
#define rtl_inw(offset) (*(volatile unsigned short *)(RTL8181_REG_BASE+offset))
#define rtl_inl(offset) (*(volatile unsigned long *)(RTL8181_REG_BASE+offset))

#define rtl_outb(offset,val)	(*(volatile unsigned char *)(RTL8181_REG_BASE+offset) = val)
#define rtl_outw(offset,val)	(*(volatile unsigned short *)(RTL8181_REG_BASE+ offset) = val)
#define rtl_outl(offset,val)	(*(volatile unsigned long *)(RTL8181_REG_BASE+offset) = val)
#endif
#undef BIT
#ifdef BIT
#error	"BIT define occurred earlier elsewhere!\n"
#endif

#define BIT(x)	( 1 << (x))

#endif
