/*
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.                                                    
* 
* Program : The header file of realtek type definition
* Abstract :                                                           
* Author :              
* Revision 1.1.2.1  2009/02/25 05:59:23  pkshih
* [VoIP-rtl865x-1.0.4.36] System: Refine voip manager and voip_types
*
* Revision 1.3  2008/01/15 06:33:19  kennylin
* Sync with VoIP-8186 for SI3226.
*
* Revision 1.4  2007/05/28 02:54:57  pkshih
* IP phone LCD/Keypad support.
*
* Revision 1.3  2006/07/04 05:56:13  thlin
* modify code for RTCP use.
*
* Revision 1.2  2006/07/02 09:42:56  kennylin
* 8651 inital ok version
*
* Revision 1.1  2006/06/20 13:13:06  kennylin
* reorganize rtk_voip basing on Branch: merged-3way 2006-06-13
*
* Revision 1.1.1.1  2005/10/25 05:50:15  bruce
* SD4 VoIP ATA project! initial version!
*
* Revision 1.1  2005/09/12 15:00:15  sandro
*
*  Committing in .
*
*  Added Files:
*  	Makefile asicRegs.h assert.h dsp_main.c dsp_main.h
*  	voip_dsp.h mbuf.h myTypes.h pcm865x.c pcm_interface.h
*  	rtl_queue.h rtl_types.h types.h voip_support.h
*
* Revision 1.1  2005/07/21 10:57:18  sandro
*  Added Files:
*  	Makefile asicRegs.h assert.h dsp_main.c dsp_main.h dspapi.c
*  	dspapi.h voip_dsp.h mbuf.h myTypes.h pcm.h pcm865x.c
*  	rtl_queue.h rtl_types.h types.h voip_support.h
*
* Revision 1.1.1.1  2005/06/29 03:46:33  sandro
* Initial import.
*
* Revision 1.9  2005/01/10 03:21:43  yjlou
* *: always define __IRAM and __DRAM
*
* Revision 1.8  2004/07/23 13:42:45  tony
* *: remove all warning messages
*
* Revision 1.7  2004/07/05 08:25:32  chenyl
* +: define __IRAM, __DRAM for module test
*
* Revision 1.6  2004/07/04 15:04:55  cfliu
* +: add IRAM and DRAM
*
* Revision 1.5  2004/04/20 03:44:03  tony
* if disable define "RTL865X_OVER_KERNEL" and "RTL865X_OVER_LINUX", __KERNEL__ and __linux__ will be undefined.
*
* Revision 1.4  2004/03/19 13:13:35  cfliu
* Reorganize ROME driver local header files. Put all private data structures into different .h file corrsponding to its layering
* Rename printf, printk, malloc, free with rtlglue_XXX prefix
*
* Revision 1.3  2004/03/05 07:44:27  cfliu
* fix header file problem for ctype.h
*
* Revision 1.2  2004/03/03 10:40:38  yjlou
* *: commit for mergence the difference in rtl86xx_tbl/ since 2004/02/26.
*
* Revision 1.1  2004/02/25 14:26:33  chhuang
* *** empty log message ***
*
* Revision 1.3  2004/02/25 14:24:52  chhuang
* *** empty log message ***
*
* Revision 1.8  2003/12/10 06:30:12  tony
* add linux/config.h, disable define CONFIG_RTL865X_NICDRV2 in mbuf.c by default
*
* Revision 1.7  2003/12/03 14:25:43  cfliu
* change SIZE_T to _SIZE_T. Linux kernel seems to recognize _SIZE_T
*
* Revision 1.6  2003/10/01 12:29:02  tony
* #define DEBUG_P(args...) while(0);
*
* Revision 1.5  2003/10/01 10:31:47  tony
* solve all the compiler warnning messages in the board.c
*
* Revision 1.4  2003/09/30 06:07:50  orlando
* check in RTL8651BLDRV_V20_20030930
*
* Revision 1.30  2003/07/21 06:27:49  cfliu
* no message
*
* Revision 1.29  2003/04/30 15:32:30  cfliu
* move macros to types.h
*
* Revision 1.28  2003/03/13 10:29:22  cfliu
* Remove unused symbols
*
* Revision 1.27  2003/03/06 05:00:04  cfliu
* Move '#pragma ghs inlineprologue' to rtl_depend.h since it is compiler dependent
*
* Revision 1.26  2003/03/06 03:41:46  danwu
* Prevent compiler from generating internal sub-routine call code at the
*  function prologue and epilogue automatically
*
* Revision 1.25  2003/03/03 09:16:35  hiwu
* remove ip4a
*
* Revision 1.24  2003/02/18 10:04:06  jzchen
* Add ether_addr_t to compatable with protocol stack's ether_addr
*
* Revision 1.23  2003/01/21 05:59:51  cfliu
* add min, max, SETBITS, CLEARBITS, etc.
*
* Revision 1.22  2002/11/25 07:31:30  cfliu
* Remove _POSIX_SOURCE since it is cygwin specific
*
* Revision 1.21  2002/09/30 11:51:49  jzchen
* Add ASSERT_ISR for not print inside ISR
*
* Revision 1.20  2002/09/18 01:43:24  jzchen
* Add type limit definition
*
* Revision 1.19  2002/09/16 00:14:34  elvis
* remove struct posix_handle_t (change the handle type from
*  structure to uint32)
*
* Revision 1.18  2002/08/20 01:40:40  danwu
* Add definitions of ipaddr_t & macaddr_t.
*
* Revision 1.17  2002/07/30 04:36:30  danwu
* Add ASSERT_CSP.
*
* Revision 1.16  2002/07/19 06:47:30  cfliu
* Add _POSIX_SOURCE symbol
*
* Revision 1.15  2002/07/05 02:10:39  elvis
* Add new types for OSK
*
* Revision 1.14  2002/07/03 12:36:21  orlando
* <rtl_depend.h> will use type definitions. Has to be moved to
* be after the type declaration lines.
*
* Revision 1.13  2002/07/03 09:19:00  cfliu
* Removed all standard header files from source code. They would be included by <core/types.h>-><rtl_depend.h>
*
* Revision 1.12  2002/07/03 09:16:48  cfliu
* Removed all standard header files from source code. They would be included by <core/types.h>-><rtl_depend.h>
*
* Revision 1.11  2002/07/03 07:14:47  orlando
* Add "struct posix_handle_t_", used by POSIX module.
*
* Revision 1.9  2002/06/21 03:15:36  cfliu
* Add time.h for struct timeval
*
* Revision 1.8  2002/06/14 01:58:03  cfliu
* Move sa_family_t to socket
*
* Revision 1.7  2002/06/13 09:37:42  cfliu
* Move byte order conversion routines to socket
*
* Revision 1.6  2002/05/23 04:24:37  hiwu
* change memaddr_t to calladdr_t
*
* Revision 1.5  2002/05/13 10:15:16  hiwu
* add new type definition
*
* Revision 1.4  2002/05/09 05:21:51  cfliu
* Add parenthesis around swaps16, swapl32
*
* Revision 1.3  2002/04/30 03:07:34  orlando
* Remove UIxx_T definitions to conform with new
* naming conventions.
*
* Revision 1.2  2002/04/29 10:10:32  hiwu
* add NTOHS macro
*
* Revision 1.1.1.1  2002/04/26 08:53:53  orlando
* Initial source tree creation.
*
* Revision 1.9  2002/04/25 03:59:05  cfliu
* no message
*
* Revision 1.8  2002/04/08 08:08:04  hiwu
* initial version
*
*/


#ifndef _RTL_TYPES_H
#define _RTL_TYPES_H

#ifndef RTL865X_OVER_KERNEL
	#ifdef __KERNEL__
		#define __RESTORE_KERNEL_DEF__
	#endif
	#undef __KERNEL__
#endif

#ifndef RTL865X_OVER_LINUX
	#undef __linux__
#endif

/*
 * Internal names for basic integral types.  Omit the typedef if
 * not possible for a machine/compiler combination.
 */
#if 1 /*    __linux__ is only defined in RTL865x platform   */
//#ifdef __linux__
 #ifdef __KERNEL__
#include <linux/config.h>
#include <linux/ctype.h>
#include <linux/module.h>
#include <linux/string.h>
#ifdef CONFIG_RTL865X
	#ifndef __DRAM
	#define  __DRAM  __attribute__  ((section(".dram")))
	#endif
	#ifndef __IRAM
	#define  __IRAM  __attribute__  ((section(".iram")))
	#endif
#endif
#endif /*__KERNEL__*/
#endif /*__linux__*/

#if 1 /*    __linux__ is only defined in RTL865x platform   */
//#ifdef __linux__
	#ifdef __KERNEL__
	   #define  rtlglue_printf printk
	#endif
#elif RTL865X_TEST
   #include <ctype.h>
   #define rtlglue_printf printf
   #define  __DRAM  
   #define  __IRAM
#endif

//typedef  long  int   Word32;
//typedef  short int   Word16;	
//typedef  short int   Flag;	

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
#define __offsetof(type, field) ((size_t)(&((type *)0)->field))
#endif

#ifndef offsetof
#define offsetof(type, field) __offsetof(type, field)
#endif

#define ASSERT_ISR(x) if(!(x)) {while(1);}
#define RTL_STATIC_INLINE   static __inline__

#define ASSERT_CSP(x) if (!(x)) {rtlglue_printf("\nAssert Fail: %s %d", __FILE__, __LINE__); while(1);}

#if defined(DRTL_TBLDRV)||defined(RTL865X_TEST)
//only ROME driver and module test code need to include this header file
#include "rtl_glue.h"
#endif
 
//#ifndef true
//#define true			(0 == 0)
//#define false		(0 != 0)
//#endif

#ifdef __RESTORE_KERNEL_DEF__
#define __KERNEL__
#endif

#endif
