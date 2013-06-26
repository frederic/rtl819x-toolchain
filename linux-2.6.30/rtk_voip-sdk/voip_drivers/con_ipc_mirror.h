#ifndef __CON_IPC_MIRROR_H__
#define __CON_IPC_MIRROR_H__

#include "voip_types.h"
#include "voip_ipc.h"
#include "snd_mirror_define.h"

typedef union {	// data in mirror packets 
	mirror_slic_priv_t	slic;
	mirror_daa_priv_t	daa;	
} ipc_mirror_union_data_t;

typedef union {	// This is host side RAM variables 
	struct {
		uint32 f_ring_chk:1;		// do ring check 
		uint32 f_ring_occur:1;		// check return value == 1
		uint32 f_SendNTTCAR_chk:1;	// do NTTCAR check 
		
		uint8  CPC_pre_linefeed;	// DSP side can't provide, so host side backup 
		uint32 SendNTTCAR_check_timeout;	// NTTCAR check timeout value 
	} slic;
	struct {
		uint32 f_:1;
	} daa;
} ipc_mirror_help_state_t;

typedef struct {
	ipc_mirror_union_data_t udata;
	ipc_mirror_help_state_t hstate;
} ipc_mirror_priv_data_t;

#endif /* __CON_IPC_MIRROR_H__ */

