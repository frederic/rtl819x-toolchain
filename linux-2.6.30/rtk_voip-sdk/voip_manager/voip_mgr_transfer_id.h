#ifndef __VOIP_MGR_TRANSFER_ID_H__
#define __VOIP_MGR_TRANSFER_ID_H__

#include "voip_types.h"

extern uint32 API_OpenSid(uint32 chid, uint32 mid);
extern uint32 API_GetSid(uint32 chid, uint32 mid);
extern uint32 API_GetMid(uint32 chid, uint32 sid);
extern uint32 API_CloseSid(uint32 chid, uint32 mid);

#endif /* __VOIP_MGR_TRANSFER_ID_H__ */

