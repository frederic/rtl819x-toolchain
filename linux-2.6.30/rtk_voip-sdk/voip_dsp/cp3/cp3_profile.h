/* Profile utilities based on COP3 */
/* Modiied from rtl8651 ROME driver */

#ifndef _CP3_PROFILE_H_
#define _CP3_PROFILE_H_

#include "voip_types.h"
#include "cp3_index.h"
#include <linux/string.h> // add by timlee for memset and memcpy 
#include <linux/kernel.h> // add by timlee for printk and sprintf

struct profile_stat_s {
	char *desc;
	uint64 maxCycle[4];
	uint64 accCycle[4];
	uint64 tempCycle[4];
	uint32 executedNum;
	uint32 hasTempCycle:1; /* true if tempCycle is valid. */
	uint32 per_count;
	int		valid;
};
typedef struct profile_stat_s profile_stat_t;


#ifdef FEATURE_COP3_PROFILE
int ProfileInit( void );
int ProfileEnterPoint( uint32 index );
int ProfileExitPoint( uint32 index );
int ProfileDump( uint32 start, uint32 end, uint32 period );
int ProfilePerDump( uint32 start, uint32 period );
#else
#define ProfileInit()			//ProfileInit
#define ProfileEnterPoint(x)	//ProfileEnterPoint
#define ProfileExitPoint(x)		//ProfileExitPoint
#define ProfileDump(x,y)		//ProfileDump
#define ProfilePerDump(x, y)	//ProfilePerDump
#endif

int ProfilePause( void );
int ProfileResume( void );
int ProfileGet( uint64 *pGet );

#endif/* _CP3_PROFILE_H_ */
