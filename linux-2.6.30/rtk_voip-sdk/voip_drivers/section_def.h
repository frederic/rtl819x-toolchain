#ifndef __SECTION_DEF_H__
#define __SECTION_DEF_H__

#include "rtk_voip.h"

#if defined (AUDIOCODES_VOIP)

#define __pcmIsr00	__attribute__ ((section(".text.pcmisr.page_1.0")))
#define __pcmIsr01	__attribute__ ((section(".text.pcmisr.page_1.1")))
#define __pcmIsr02	__attribute__ ((section(".text.pcmisr.page_1.2")))

 #define __pcmifdata00	__attribute__ ((section(".data.pcmif.page_0.0"))) 
 #define __pcmifdata01	__attribute__ ((section(".data.pcmif.page_0.1"))) 
 #define __pcmifdata02	__attribute__ ((section(".data.pcmif.page_0.2")))  
 #define __pcmifdata03	__attribute__ ((section(".data.pcmif.page_0.3"))) 
 #define __pcmifdata04	__attribute__ ((section(".data.pcmif.page_0.4"))) 
 #define __pcmifdata05	__attribute__ ((section(".data.pcmif.page_0.5"))) 
 #define __pcmifdata06	__attribute__ ((section(".data.pcmif.page_0.6"))) 
 #define __pcmifdata07	__attribute__ ((section(".data.pcmif.page_0.7"))) 
 #define __pcmifdata08	__attribute__ ((section(".data.pcmif.page_0.8"))) 
 #define __pcmifdata09	__attribute__ ((section(".data.pcmif.page_0.9"))) 
 #define __pcmifdata10	__attribute__ ((section(".data.pcmif.page_0.10"))) 
 #define __pcmifdata11	__attribute__ ((section(".data.pcmif.page_0.11"))) 
 #define __pcmifdata12	__attribute__ ((section(".data.pcmif.page_0.12"))) 
 #define __pcmifdata13	__attribute__ ((section(".data.pcmif.page_0.13"))) 
 #define __pcmifdata14	__attribute__ ((section(".data.pcmif.page_0.14")))  
 #define __pcmifdata15	__attribute__ ((section(".data.pcmif.page_0.15")))  
 #define __pcmifdata16	__attribute__ ((section(".data.pcmif.page_0.16")))
 #define __pcmifdata17	__attribute__ ((section(".data.pcmif.page_0.14")))  
 #define __pcmifdata18	__attribute__ ((section(".data.pcmif.page_0.18")))  
 #define __pcmifdata19	__attribute__ ((section(".data.pcmif.page_0.19")))
 #define __pcmifdata20	__attribute__ ((section(".data.pcmif.page_0.20")))

 #define __pcmImem	__attribute__ ((section(".speedup.text")))

#else

 #define __pcmIsr00		/*__attribute__ ((section(".bus_pcm_isr")))*/
 #define __pcmIsr01		/*__attribute__ ((section(".bus_pcm_isr")))*/
 #define __pcmIsr02		/*__attribute__ ((section(".bus_pcm_isr")))*/

 #define __pcmifdata00
 #define __pcmifdata01
 #define __pcmifdata02
 #define __pcmifdata03
 #define __pcmifdata04
 #define __pcmifdata05
 #define __pcmifdata06
 #define __pcmifdata07
 #define __pcmifdata08
 #define __pcmifdata09
 #define __pcmifdata10
 #define __pcmifdata11
 #define __pcmifdata12
 #define __pcmifdata13
 #define __pcmifdata14
 #define __pcmifdata15
 #define __pcmifdata16
 #define __pcmifdata17
 #define __pcmifdata18
 #define __pcmifdata19
 #define __pcmifdata20

#endif

#endif /* __SECTION_DEF_H__ */

