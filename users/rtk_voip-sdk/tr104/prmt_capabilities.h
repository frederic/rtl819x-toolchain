#ifndef __PRMT_VOICE_CAPABILITIES_H__
#define __PRMT_VOICE_CAPABILITIES_H__

#include "prmt_igd.h"

//extern struct sCWMP_ENTITY tCapabilitiesEntity[];
extern struct CWMP_NODE tCapabilitiesEntityNodes[];
extern struct CWMP_LEAF tCapabilitiesEntityLeaves[];

typedef struct lstCodecs_s {
	const char *pszCodec;
	unsigned int nBitRate;
	const char *pszPacketizationPeriod;
	unsigned int bSilenceSupression;
} lstCodecs_t;

extern const lstCodecs_t lstCodecs[];

#endif /* __PRMT_VOICE_CAPABILITIES_H__ */

