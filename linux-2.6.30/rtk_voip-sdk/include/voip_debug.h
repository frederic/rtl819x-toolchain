#ifndef __VOIP_DEBUG_H
#define __VOIP_DEBUG_H

#ifdef __KERNEL__
#include <linux/kernel.h>	// for debug message
#include "voip_timer.h"
#endif

#ifndef MODULE_NAME
#define MODULE_NAME ""
#endif

#define DBG_PRINT_MAX		1024

#define AC_RESET			"\x1B[0m"	/* clears all colors and styles (to white on black) */
#define AC_FORE_RED			"\x1B[31m"	/* foreground red */
#define AC_FORE_GREEN		"\x1B[32m"	/* foreground green */
#define AC_FORE_YELLOW		"\x1B[33m"	/* foreground yellow */
#define AC_FORE_BlUE		"\x1B[34m"	/* foreground blue */

#define PRINT_R(fmt, args...)		printk("\x1B[31m" fmt "\x1B[0m", ## args)
#define PRINT_G(fmt, args...)		printk("\x1B[32m" fmt "\x1B[0m", ## args)
#define PRINT_B(fmt, args...)		printk("\x1B[34m" fmt "\x1B[0m", ## args)
#define PRINT_Y(fmt, args...)		printk("\x1B[33m" fmt "\x1B[0m", ## args)

#define RTK_KERN_ERROR 		KERN_ERR "\x1b[31m"
#define RTK_KERN_WARNING	KERN_WARNING "\x1b[33m"
#define RTK_KERN_INFO 		KERN_INFO "\x1b[32m"
#define RTK_KERN_TRACE 		KERN_DEBUG "\x1b[36m"

#define __PRINT(lv, name, fmt, args...) 									\
do { 																		\
	if (rtk_dbg_level == 99)												\
		printk(RTK_KERN_##lv "[" __FILE__ ":%d]\x1B[0m " fmt, 				\
			__LINE__, ##args);												\
	else if (rtk_dbg_level >= RTK_DBG_##lv)									\
		printk(RTK_KERN_##lv "[%010lu][%3s]\x1B[0m " fmt,				    \
			timetick-TIMETICK_SEED, name, ##args);							\
} while (0)
#define DBG_ERROR(fmt, args...)		__PRINT(ERROR, MODULE_NAME, fmt, ##args)
#define DBG_WARNING(fmt, args...)	__PRINT(WARNING, MODULE_NAME, fmt, ##args)
#define DBG_INFO(fmt, args...)		__PRINT(INFO, MODULE_NAME, fmt, ##args)
#define DBG_TRACE(fmt, args...)		__PRINT(TRACE, MODULE_NAME, fmt, ##args)

#define PRINT_MSG(fmt, args...) 	DBG_INFO(fmt, ##args)
#define PRINT_DBG(fmt, args...) 	DBG_TRACE(fmt, ##args)

#define DBG_PRINT(lv, name, fmt, args...)			\
do {												\
	switch (lv)										\
	{												\
		case RTK_DBG_ERROR:							\
			__PRINT(ERROR, name, fmt, ##args);		\
			break;									\
		case RTK_DBG_WARNING:						\
			__PRINT(WARNING, name, fmt, ##args);	\
			break;									\
		case RTK_DBG_INFO:							\
			__PRINT(INFO, name, fmt, ##args);		\
			break;									\
		case RTK_DBG_TRACE:							\
			__PRINT(TRACE, name, fmt, ##args);		\
			break;									\
		default:									\
			printk(KERN_ERR "Invalid RTK_PRINT\n");	\
			break;									\
	}												\
} while (0)

#ifdef ENABLE_BOOT_MSG
#define BOOT_MSG(fmt, args...) DBG_ERROR(fmt, ##args)
#else
#define BOOT_MSG(fmt, args...)
#endif

/**
 * @ingroup VOIP_DEBUG
 * Enumeration for debug level
 */
enum  {
	RTK_DBG_ERROR = 0,
	RTK_DBG_WARNING,
	RTK_DBG_INFO,
	RTK_DBG_TRACE,
	RTK_DBG_MAX
};

/**
 * @brief Structure for debug message 
 * @param level Debug level
 * @param module Module name 
 * @param msg Print message 
 * @see do_mgr_VOIP_MGR_PRINT()
 */
typedef struct {
	int level;
	char module[8];
	char msg[DBG_PRINT_MAX];
	int ret_val;
} rtk_print_cfg;


typedef struct {

	char cp3_counter1;
	char cp3_counter2;
	char cp3_counter3;
	char cp3_counter4;	
	int cp3_dump_period;
	int bCp3Count_PCM_ISR;
	int bCp3Count_PCM_RX;
	int bCp3Count_PCM_HANDLER;
	int bCp3Count_LEC;
	int bCp3Count_G711Enc;
	int bCp3Count_G711Dec;
	int bCp3Count_G729Enc;
	int bCp3Count_G729Dec;
	int bCp3Count_G7231Enc;
	int bCp3Count_G7231Dec;
	int bCp3Count_G726Enc;
	int bCp3Count_G726Dec;
	int bCp3Count_G722Enc;
	int bCp3Count_G722Dec;
	int bCp3Count_GSMFREnc;
	int bCp3Count_GSMFRDec;
	int bCp3Count_iLBC20Enc;
	int bCp3Count_iLBC20Dec;
	int bCp3Count_iLBC30Enc;
	int bCp3Count_iLBC30Dec;
	int bCp3Count_T38Enc;
	int bCp3Count_T38Dec;
	int bCp3Count_AMRNBEnc;
	int bCp3Count_AMRNBDec;
	int bCp3Count_SpeexNBEnc;
	int bCp3Count_SpeexNBDec;
	int bCp3Count_G7111NBEnc;
	int bCp3Count_G7111NBDec;
	int bCp3Count_G7111WBEnc;
	int bCp3Count_G7111WBDec;
	int bCp3Count_Temp200;
	int bCp3Count_Temp201;
	int bCp3Count_Temp202;
	int bCp3Count_Temp203;
	int bCp3Count_Temp204;
	int bCp3Count_Temp205;
	int bCp3Count_Temp206;
	int bCp3Count_Temp207;
	int bCp3Count_Temp208;
	int bCp3Count_Temp209;
	int bCp3Count_Temp210;
	int bCp3Count_Temp211;
	int bCp3Count_Temp212;
	int bCp3Count_Temp213;
	int bCp3Count_Temp214;
	int bCp3Count_Temp215;
	int bCp3Count_Temp216;
	int bCp3Count_Temp217;
	int bCp3Count_Temp218;
	int bCp3Count_Temp219;
	//int ret_val;
} st_CP3_VoIP_param;

extern int rtk_dbg_level;

// compiler time assert macro 
#define CT_ASSERT( expr )	extern int __ct_assert[ 2 * ( expr ) - 1 ] __attribute__ ((__unused__))

#endif
