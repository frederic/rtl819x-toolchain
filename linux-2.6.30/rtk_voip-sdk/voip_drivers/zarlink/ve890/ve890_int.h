#ifndef __VE890_INT_H__
#define __VE890_INT_H__

#include "zarlinkCommon.h"

int Ve890CreateDevObj(
	/* Realtek */
	RTKDevType 				dev_type, 
	int						ch_id,
	RTKDevObj 				*pDevObj, 
	RTKLineObj 				LineObj[],

	/* Zarlink */
	VpDeviceIdType 			devId, 
	VpDeviceType  			vpDevType, 
	Vp890DeviceObjectType	*pVpDevObj, 	/* use 'void *' to support 880/890 */
	VpDevCtxType  			*pVpDevCtx, 
	Vp890LineObjectType  	pVpLineObj[],
	VpLineCtxType 			pVpLineCtx[]);

BOOL Ve890CreateLineObj(
	int ch_id, 
	int channelId, 	/* line# within a slic. usually 0 or 1 */
	RTKLineType	line_type,
	RTKLineObj *pLine,
	int law,
	unsigned int slot );

int Ve890Init(int pcm_mode);
int Ve890GetRev(RTKDevObj *pDev);

#endif /* __VE890_INT_H__ */


