
#ifndef __ZARLINKCOMMONDAA_H__
#define __ZARLINKCOMMONDAA_H__

#include "vp_api.h"
#include "Ve_profile.h"

#include "zarlinkCommon.h" 

/*
 *  Function declarations
 */

BOOL zarlinkGetFxoLineStatus(RTKLineObj *pLine, uint8 category);
unsigned char zarlinkGetFxoHookStatus(RTKLineObj *pLine);
VpStatusType zarlinkSetFxoLineState(RTKLineObj *pLine, VpLineStateType lineState);
VpStatusType zarlinkSetFxoLineOnHook(RTKLineObj *pLine);
VpStatusType zarlinkSetFxoLineOffHook(RTKLineObj *pLine);
VpStatusType zarlinkSetFxoLineOHT(RTKLineObj *pLine);
VpStatusType zarlinkSetFxoRingValidation(RTKLineObj *pLine);

#endif /* __ZARLINKCOMMONDAA_H__ */

