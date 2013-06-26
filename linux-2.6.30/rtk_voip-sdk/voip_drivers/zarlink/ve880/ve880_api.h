
#ifndef __VE880_API_H__
#define __VE880_API_H__

#include "zarlinkCommon.h"

VpStatusType Ve880SetRingCadenceProfile(RTKLineObj *pLine, uint8 ring_cad);
VpStatusType Ve880SetImpedenceCountry(RTKLineObj *pLine, uint8 country);
VpStatusType Ve880SetFxsAcProfileByBand(RTKLineObj *pLine, int pcm_mode);

VpStatusType Ve880UpdIOState(RTKLineObj *pLine);
VpStatusType Ve880SetIOState(RTKLineObj *pLine, VPIO IO, int bHigh );
VpStatusType Ve880GetIOState(RTKLineObj *pLine, VPIO IO, int *bHigh);
VpStatusType Ve880SetIODir(RTKLineObj *pLine, VPIO IO, int bOut);

#if 0
VpProfilePtrType Ve880RingProfile(uint8 profileId);
VpProfilePtrType Ve880AcProfile(uint8 profileId);
#endif

/********** DAA Function **********/

#endif /* __VE880_API_H__*/

