#ifndef __IPC_INTERNAL_H__
#define __IPC_INTERNAL_H__

#include "voip_types.h"

typedef struct
{
	uint16 category;
	uint16 seqNo;
	unsigned int needResponse;
	// response content (TX process allocate / RX process fill)
	uint8 *content;
	uint16 cont_len;
	uint16 cont_dsp_cpuid;
	// TODO: Add a mutex to protect TX/RX race 
}
TstCheckResponse;

typedef struct
{
	uint16 category;
	uint16 seqNo;
	unsigned int needAck;
}
TstCheckAck;

// log data preamble 
#define LOG_IPC_TX_PREAMBLE		"IPCT"
#define LOG_IPC_RX_PREAMBLE		"IPCR"

#endif /* __IPC_INTERNAL_H__ */

