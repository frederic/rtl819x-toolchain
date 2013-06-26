#ifndef __VOIP_ADDRSPACE_H__
#define __VOIP_ADDRSPACE_H__

#include "rtk_voip.h"

// ---- NO_SPECIAL_ADDRSPACE decision 

#define NO_SPECIAL_ADDRSPACE	1

#ifdef CONFIG_RTK_VOIP_2_PHYSICAL_OFFSET
#if CONFIG_RTK_VOIP_2_PHYSICAL_OFFSET > 0
#undef NO_SPECIAL_ADDRSPACE
#endif
#endif

// ---- use NO_SPECIAL_ADDRSPACE to choose macro 

#ifndef NO_SPECIAL_ADDRSPACE

#define Virtual2Logical(x)		(((unsigned long)x) & 0x1fffffff)
#define Logical2Virtual(x)		(((unsigned long)x) | 0x80000000)
#define Logical2NonCache(x)		(((unsigned long)x) | 0xa0000000)

#define Logical2Physical(x)		(((unsigned long)x) + CONFIG_RTK_VOIP_2_PHYSICAL_OFFSET)
#define Physical2Logical(x)		(((unsigned long)x) - CONFIG_RTK_VOIP_2_PHYSICAL_OFFSET)

#define Virtual2Physical(x)		Logical2Physical(Virtual2Logical(x))
#define Physical2Virtual(x)		Logical2Virtual(Physical2Logical(x))
#define Virtual2NonCache(x)		(((unsigned long)x) | 0x20000000)
#define Physical2NonCache(x)	Logical2NonCache(Physical2Logical(x))

#else

#define Virtual2Physical(x)		(((unsigned long)x) & 0x1fffffff)
#define Physical2Virtual(x)		(((unsigned long)x) | 0x80000000)
#define Virtual2NonCache(x)		(((unsigned long)x) | 0x20000000)
#define Physical2NonCache(x)	(((unsigned long)x) | 0xa0000000)

#endif

#endif /* __VOIP_ADDRSPACE_H__ */

