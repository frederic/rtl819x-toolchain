#ifndef __RTCP_XR_RLE_H__
#define __RTCP_XR_RLE_H__

#include "voip_types.h"
#include "rtpTypesXR.h"

extern uint32 RunLengthEncode( uint32 *src, uint32 bits, RtcpXRChunk *chunk );

#endif /* __RTCP_XR_RLE_H__ */

