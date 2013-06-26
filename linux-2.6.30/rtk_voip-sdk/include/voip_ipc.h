#ifndef __VOIP_IPC_H__
#define __VOIP_IPC_H__

#ifdef CONFIG_RTK_VOIP_IPC_ARCH

#include "voip_types.h"

// protocol 
enum {
	IPC_PROT_VOICE_TO_DSP = 0x30,	//    host --> dsp 
	IPC_PROT_VOICE_TO_HOST,			//    host <-- dsp 
	IPC_PROT_CTRL,					// a) host --> dsp 
	IPC_PROT_RESP,					// a) host <-- dsp 
	IPC_PROT_EVENT,					// b) host <-- dsp 
	IPC_PROT_ACK,					// b) host --> dsp 
	IPC_PROT_T38_TO_DSP,			//    host --> dsp 
	IPC_PROT_T38_TO_HOST,			//    host <-- dsp 
	IPC_PROT_MIRROR,				// c) host <-- dsp
	IPC_PROT_MIRROR_ACK,			// c) host --> dsp
	IPC_PROT_RPC,					// d) host <-- dsp
	IPC_PROT_RPC_ACK,				// d) host --> dsp	
};

// IPC category  
enum {	// Its domain is the same with VOIP_MGR_xxx 
	// protocol = IPC_PROT_EVENT (host <-- dsp) / IPC_PROT_ACK. 
	IPC_EVT_VOIP_EVENT = 3000,
	
	// protocol = IPC_PROT_MIRROR (host -->dsp)
	IPC_MIR_SLIC_ALL = 3100, 		// mirror all SLIC data 
	IPC_MIR_DAA_ALL  = 3200,		// mirror all DAA data 
	
	// protocol = IPC_PROT_RPC 
	IPC_RPC_SLIC = 3300,
	IPC_RPC_DAA,
	
};

#pragma pack( 1 )

typedef struct {
	uint8	dstMAC[ 6 ];	// 0
	uint8	srcMAC[ 6 ];	// 6
	uint16	ethType;		// 12 = 0x8899
	// --------------------
	uint16	dsp_cpuid;		// 14 = cpu ID 
	uint16	dummy;			// 16
	uint8	protocol;		// 18 = ipc type (ctrl = IPC_PROT_CTRL/IPC_PROT_RESP/IPC_PROT_EVENT/IPC_PROT_ACK)
	uint8	voip_ver;		// 19            (ctrl = IPC_PROT_MIRROR/IPC_PROT_MIRROR_ACK)
	// --------------------
	uint16	category;		// 20 = VOIP_MGR_ / IPC_EVT_ / IPC_MIR_ / IPC_RPC_
	uint16	sequence;		// 22
	uint16	cont_len;		// 24
	uint8	content[ 2 ];	// 26 (content[2] has no special meaning) 
} ipc_ctrl_pkt_t;

typedef struct {
	uint8	dstMAC[ 6 ];	// 0
	uint8	srcMAC[ 6 ];	// 6
	uint16	ethType;		// 12 = 0x8899
	// --------------------
	uint16	dsp_cpuid;		// 14 = cpu ID 
	uint16	dummy;			// 16
	uint8	protocol;		// 18 = ipc type (voice = IPC_PROT_VOICE_TO_HOST/IPC_PROT_VOICE_TO_DSP/IPC_PROT_T38_TO_HOST/IPC_PROT_T38_TO_DSP)
	uint8	voip_ver;		// 19
	// --------------------
	uint16	voice_cont_len;		// 20
	uint16	voice_content[ 2 ];	// 22
} ipc_voice_pkt_t;

typedef struct {
	uint16 chid;
	uint16 mid;
	uint8  voice[ 1 ];
} voice_content_t;	// union with ipc_voice_pkt_t ->voice_content

typedef struct {	// when protocol = IPC_PROT_VOICE_TO_DSP
	uint16 chid;
	uint16 mid;
	uint16 flags;
	uint8  voice[ 1 ];
} voice_rtp2dsp_content_t;	// union with ipc_voice_pkt_t ->voice_content

typedef struct {	// when protocol = IPC_PROT_MIRROR
	uint16 cch;
	uint16 data[ 1 ];		// NOTE: please make all data type are 'uint16' 
} mirror_all_content_t;	// union with ipc_ctrl_pkt_t ->content

typedef struct {	// when protocol = IPC_PROT_RPC
	uint16 cch;
	uint16 ops_offset;	// field offset in fxs/daa structure 
	uint8  data[ 1 ];	// explain by send/recv function 
} rpc_content_t;	// union with ipc_ctrl_pkt_t ->content

#pragma pack( )

// get size from the field of DSP_ID to specified one (included)
#define SIZE_CTRL_FROM_HEAD( fld )		\
	(									\
	sizeof( ( ( ( ipc_ctrl_pkt_t * )0 ) ->fld ) ) +			\
	( ( uint32 )&( ( ( ipc_ctrl_pkt_t * )0 ) ->fld ) ) 		\
	)

#define SIZE_VOICE_FROM_HEAD( fld )		\
	(									\
	sizeof( ( ( ( ipc_voice_pkt_t * )0 ) ->fld ) ) +		\
	( ( uint32 )&( ( ( ipc_voice_pkt_t * )0 ) ->fld ) ) 	\
	)

// get voice content header size 
#define SIZE_OF_VOICE_CONT_HEADER				/* 8 */		\
	(														\
	( uint32 )&( ( ( voice_content_t * )0 ) ->voice )		\
	)

#define SIZE_OF_VOICE_RTP2DSP_CONT_HEADER		/* 12 */	\
	(														\
	( uint32 )&( ( ( voice_rtp2dsp_content_t * )0 ) ->voice )	\
	)

// get mirror additional data header (cch)
#define SIZE_OF_MIRROR_CONT_PLUS_HEADER						\
	(														\
	( uint32 )&( ( ( mirror_all_content_t * )0 ) ->data )	\
	)

typedef struct
{
	//int host_cch;
	int dsp_cpuid;
	int dsp_cch;	// in some case need this (IPC_PROT_MIRROR)
	int seq_no;			// IPC_PROT_CTRL/IPC_PROT_EVENT: if resend = 1, tx use seq_no. otherwise, tx fill seq_no 
	int resend_flag;	// IPC_PROT_RESP/IPC_PROT_ACK/IPC_PROT_MIRROR_ACK: tx always use seq_no 
}
TstTxPktCtrl;

#endif /* CONFIG_RTK_VOIP_IPC_ARCH */

#endif /* __VOIP_IPC_H__ */

