/*
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.                                                    
* 
* Program : The header file of realtek type definition
* Abstract :                                                           
* Author :              
* $Id: rtl_types.h,v 1.1 2007-12-07 05:52:23 alva_zhang Exp $
* $Log: not supported by cvs2svn $
* Revision 1.1.1.1  2007/08/06 10:04:57  root
* Initial import source to CVS
*
*
*/


#ifndef _RTL_TYPES_H
#define _RTL_TYPES_H

#ifndef RTL865X_OVER_KERNEL
	#undef __KERNEL__
#endif

#ifndef RTL865X_OVER_LINUX
	#undef __linux__
#endif

/*
 * Internal names for basic integral types.  Omit the typedef if
 * not possible for a machine/compiler combination.
 */
#ifdef __linux__
#ifdef __KERNEL__
#include <linux/version.h>
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
#include <linux/config.h>
#endif
#include <linux/ctype.h>
#include <linux/module.h>
#include <linux/string.h>
#endif /*__KERNEL__*/
#endif /*__linux__*/

/* ===============================================================================
		IRAM / DRAM definition
    =============================================================================== */
#undef __DRAM_GEN
#undef __DRAM_FWD
#undef __DRAM_L2_FWD
#undef __DRAM_L34_FWD
#undef __DRAM_EXTDEV
#undef __DRAM_AIRGO
#undef __DRAM_RTKWLAN
#undef __DRAM_CRYPTO
#undef __DRAM_VOIP
#undef __DRAM_TX
#undef __DRAM

#undef __IRAM_GEN
#undef __IRAM_FWD
#undef __IRAM_L2_FWD
#undef __IRAM_L34_FWD
#undef __IRAM_EXTDEV
#undef __IRAM_AIRGO
#undef __IRAM_RTKWLAN
#undef __IRAM_CRYPTO
#undef __IRAM_VOIP
#undef __IRAM_TX
#undef __IRAM

#if defined(__linux__)&&defined(__KERNEL__)&&defined(CONFIG_RTL865X)
	#define __DRAM_GEN			__attribute__  ((section(".dram-gen")))
	#define __DRAM_FWD			__attribute__  ((section(".dram-fwd")))
	#define __DRAM_L2_FWD		__attribute__  ((section(".dram-l2-fwd")))
	#define __DRAM_L34_FWD	__attribute__  ((section(".dram-l34-fwd")))
	#define __DRAM_EXTDEV		__attribute__  ((section(".dram-extdev")))
	#define __DRAM_AIRGO		__attribute__  ((section(".dram-airgo")))
	#define __DRAM_RTKWLAN	__attribute__  ((section(".dram-rtkwlan")))
	#define __DRAM_CRYPTO		__attribute__  ((section(".dram-crypto")))
	#define __DRAM_VOIP			__attribute__  ((section(".dram-voip")))
	#define __DRAM_TX			__attribute__  ((section(".dram-tx")))
	#define __DRAM				__attribute__  ((section(".dram")))

	#define __IRAM_GEN			__attribute__  ((section(".iram-gen")))
	#define __IRAM_FWD			__attribute__  ((section(".iram-fwd")))
	#define __IRAM_L2_FWD		__attribute__  ((section(".iram-l2-fwd")))
	#define __IRAM_L34_FWD		__attribute__  ((section(".iram-l34-fwd")))
	#define __IRAM_EXTDEV		__attribute__  ((section(".iram-extdev")))
	#define __IRAM_AIRGO		__attribute__  ((section(".iram-airgo")))
	#define __IRAM_RTKWLAN		__attribute__  ((section(".iram-rtkwlan")))
	#define __IRAM_CRYPTO		__attribute__  ((section(".iram-crypto")))
	#define __IRAM_VOIP			__attribute__  ((section(".iram-voip")))
	#define __IRAM_TX			__attribute__  ((section(".iram-tx")))
	#define __IRAM				__attribute__  ((section(".iram")))
#else
	#define __DRAM_GEN
	#define __DRAM_FWD
	#define __DRAM_L2_FWD
	#define __DRAM_L34_FWD
	#define __DRAM_EXTDEV
	#define __DRAM_AIRGO
	#define __DRAM_RTKWLAN
	#define __DRAM_CRYPTO
	#define __DRAM_VOIP
	#define __DRAM_TX
	#define __DRAM

	#define __IRAM_GEN
	#define __IRAM_FWD
	#define __IRAM_L2_FWD
	#define __IRAM_L34_FWD
	#define __IRAM_EXTDEV
	#define __IRAM_AIRGO
	#define __IRAM_RTKWLAN
	#define __IRAM_CRYPTO
	#define __IRAM_VOIP
	#define __IRAM_TX
	#define __IRAM
#endif

/* ===============================================================================
		print macro
    =============================================================================== */
#if	defined(__linux__)&&defined(__KERNEL__)

	#define rtlglue_printf	printk

#else	/* defined(__linux__)&&defined(__KERNEL__) */

#ifdef	RTL865X_TEST
	#include <ctype.h>
#endif	/* RTL865X_TEST */

#define rtlglue_printf	printf

#endif	/* defined(__linux__)&&defined(__KERNEL__) */

/* ===============================================================================
		Type definition
    =============================================================================== */

typedef unsigned long long	uint64;
typedef long long		int64;
typedef unsigned int	uint32;
typedef int			int32;
typedef unsigned short	uint16;
typedef short			int16;
typedef unsigned char	uint8;
typedef char			int8;


typedef uint32		memaddr;	
typedef uint32          ipaddr_t;
typedef struct {
    uint16      mac47_32;
    uint16      mac31_16;
    uint16      mac15_0;
} macaddr_t;

#define ETHER_ADDR_LEN				6
typedef struct ether_addr_s {
	uint8 octet[ETHER_ADDR_LEN];
} ether_addr_t;



#ifndef NULL
#define NULL 0
#endif
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#ifndef SUCCESS
#define SUCCESS 	0
#endif
#ifndef FAILED
#define FAILED -1
#endif



#define DEBUG_P(args...) while(0);
#ifndef OK
#define OK		0
#endif
#ifndef NOT_OK
#define NOT_OK  1
#endif

#ifndef CLEARBITS
#define CLEARBITS(a,b)	((a) &= ~(b))
#endif

#ifndef SETBITS
#define SETBITS(a,b)		((a) |= (b))
#endif

#ifndef ISSET
#define ISSET(a,b)		(((a) & (b))!=0)
#endif

#ifndef ISCLEARED
#define ISCLEARED(a,b)	(((a) & (b))==0)
#endif

#ifndef max
#define max(a,b)  (((a) > (b)) ? (a) : (b))
#endif			   /* max */

#ifndef min
#define min(a,b)  (((a) < (b)) ? (a) : (b))
#endif			   /* min */

//round down x to multiple of y.  Ex: ROUNDDOWN(20, 7)=14
#ifndef ROUNDDOWN
#define	ROUNDDOWN(x, y)	(((x)/(y))*(y))
#endif

//round up x to multiple of y. Ex: ROUNDUP(11, 7) = 14
#ifndef ROUNDUP
#define	ROUNDUP(x, y)	((((x)+((y)-1))/(y))*(y))  /* to any y */
#endif

#ifndef ROUNDUP2
#define	ROUNDUP2(x, y)	(((x)+((y)-1))&(~((y)-1))) /* if y is powers of two */
#endif

#ifndef ROUNDUP4
#define	ROUNDUP4(x)		((1+(((x)-1)>>2))<<2)
#endif

#ifndef IS4BYTEALIGNED
#define IS4BYTEALIGNED(x)	 ((((x) & 0x3)==0)? 1 : 0)
#endif

#ifndef __offsetof
#define __offsetof(type, field) ((unsigned long)(&((type *)0)->field))
#endif

#ifndef offsetof
#define offsetof(type, field) __offsetof(type, field)
#endif

#ifndef RTL_PROC_CHECK
#define RTL_PROC_CHECK(expr, success) \
	do {\
			int __retval; \
			if ((__retval = (expr)) != (success))\
			{\
				rtlglue_printf("ERROR >>> [%s]:[%d] failed -- return value: %d\n", __FUNCTION__,__LINE__, __retval);\
				return __retval; \
			}\
		}while(0)
#endif

#ifndef RTL_STREAM_SAME
#define RTL_STREAM_SAME(s1, s2) \
	((strlen(s1) == strlen(s2)) && (strcmp(s1, s2) == 0))
#endif

#define ASSERT_ISR(x) if(!(x)) {while(1);}
#define RTL_STATIC_INLINE   static __inline__

#define ASSERT_CSP(x) if (!(x)) {rtlglue_printf("\nAssert Fail: %s %d", __FILE__, __LINE__); while(1);}

#if defined(RTL865X_TEST) || defined(RTL865X_MODEL_USER)
/* Only model code needs to define the following code. */
typedef struct { } spinlock_t;
typedef uint8 u8;
typedef uint16 u16;
typedef uint32 u32;
#endif
 
#if defined(DRTL_TBLDRV)||defined(RTL865X_TEST)
//only ROME driver and module test code need to include this header file
#include "rtl_glue.h"
#endif

#if defined(RTL865X_TEST)||defined(RTL865X_MODEL_USER)
#define UNCACHE_MASK		0
#define UNCACHE(addr)		(addr)
#define CACHED(addr)			((uint32)(addr))
#else
#define UNCACHE_MASK		0x20000000
#define UNCACHE(addr)		((UNCACHE_MASK)|(uint32)(addr))
#define CACHED(addr)			((uint32)(addr) & ~(UNCACHE_MASK))
#endif


#endif

