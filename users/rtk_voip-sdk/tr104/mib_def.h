#ifndef __MIB_DEF_H__
#define __MIB_DEF_H__

typedef enum {
	TR104_DISABLE,
	TR104_QUIESCENT0,
	TR104_ENABLE,
} enable_t;

typedef enum {
	SIP,
	MGCP,
	MGCP_NCS,
	H_248,
	H_323,
} signaling_protocol_t;

typedef enum {
	RFC2833=0,
	SIP_INFO,
	IN_BAND
} DTMF_method_t;

typedef enum {
	UDP,
	TCP,
	TLS,
	SCTP,
} transport_t;

typedef enum {
	UP,
	INITIALIZING,
	REGISTERING,
	UNREGISTERING,
	ERROR,
	TESTING,
	QUIESCENT1,
	TR104_DISABLED,
} line_status_t;

#endif /* __MIB_DEF_H__ */

