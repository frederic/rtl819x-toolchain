#ifndef __VE880_INT_H__
#define __VE880_INT_H__

#include "zarlinkCommon.h"

int Ve880CreateDevObj(
	/* Realtek */
	RTKDevType 				dev_type, 
	int						ch_id,
	RTKDevObj 				*pDevObj, 
	RTKLineObj 				LineObj[],

	/* Zarlink */
	VpDeviceIdType 			devId, 
	VpDeviceType  			vpDevType, 
	Vp880DeviceObjectType	*pVpDevObj, 	/* use 'void *' to support 880/890 */
	VpDevCtxType  			*pVpDevCtx, 
	Vp880LineObjectType  	pVpLineObj[],
	VpLineCtxType 			pVpLineCtx[]);

BOOL Ve880CreateLineObj(
	int ch_id, 
	int channelId, 	/* line# within a slic. usually 0 or 1 */
	RTKLineType	line_type,
	RTKLineObj *pLine,
	int law,
	unsigned int slot);

int Ve880Init(int pcm_mode);
int Ve880GetRev(RTKDevObj *pDev);

#endif /* __VE880_INT_H__ */


