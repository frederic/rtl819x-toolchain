
#include "voip_types.h"


#ifdef __KERNEL__
	#define rtlglue_printf printk
	#define printf			printk
#else
	#define rtlglue_printf printf 
#endif

#ifdef FEATURE_C_MODEL
#else
#include <linux/config.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <asm/rtl8181.h>
#endif

//#define printk        printf
#define printfByPolling       printk
//#define sprintf Vsprintf

int32 rtlglue_getDrvMutex(void);
int32 rtlglue_reinitDrvMutex(void);
int32 rtlglue_getMbufMutex(void);
int32 rtlglue_reinitMbufMutex(void);

extern int test_drvMutex;
extern int test_mbufMutex;

#define rtlglue_drvMutexLock() \
	do { \
		test_drvMutex ++;\
	} while (0)
#define rtlglue_drvMutexUnlock()\
	do {\
		test_drvMutex --;\
		if (test_drvMutex < 0)\
		{\
			printf("%s (%d) Error: Driver Mutex Lock/Unlcok is not balance (%d).\n", __FUNCTION__, __LINE__, test_drvMutex);\
		}\
	} while (0)

#define rtlglue_mbufMutexLock() \
	do { \
		test_mbufMutex ++;\
	} while (0)
#define rtlglue_mbufMutexUnlock()\
	do {\
		test_mbufMutex --;\
		if (test_mbufMutex < 0)\
		{\
			printf("%s (%d) Error: Mbuffer Mutex Lock/Unlcok is not balance (%d).\n", __FUNCTION__, __LINE__, test_mbufMutex);\
		}\
	} while (0)


