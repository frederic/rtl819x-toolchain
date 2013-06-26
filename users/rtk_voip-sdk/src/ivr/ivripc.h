#ifndef __IVR_IPC_H__
#define __IVR_IPC_H__

#include "voip_flash.h"

#define IVRSERVER_SO	1		// experimental .so version 

#ifndef IVRSERVER_SO
#define IVR_SERVER_PATHNAME		"/bin/ivrserver"
#define IVR_SERVER_VERSION		1
#define IVR_CLIENT_KEY_OFFSET	0xEF45
#endif

#define LINE_MAX_LEN	100		/* Don't modify */
#define DNS_LEN			40		/* Don't modify */

/* =============================================================== */
/* client -> server message */
#define IVR_IPC_MSG_SEND_GLOBAL		1	/* After initialization, global constant are given */
#define IVR_IPC_MSG_DO_IVR_INS		2	/* User input a SUFFIX '#' (INITIAL '#' will not send this message) */
#define IVR_IPC_MSG_UPDATE_FLASH	3	/* Once flash data modified. */

typedef struct do_ivr_ins_s {		/* IVR_IPC_MSG_DO_IVR_INS */
    int chid;
    int sid;
    char dial_initial_hash;
    int	digit_index;
    char dial_code[LINE_MAX_LEN];
    char login_id[DNS_LEN];
} do_ivr_ins_t;

typedef struct send_global_s {		/* IVR_IPC_MSG_SEND_GLOBAL */
	int mapSupportedCodec[ _CODEC_MAX ];
	int nMaxCodec;
} send_global_t;

typedef struct update_flash_s {		/* IVR_IPC_MSG_UPDATE_FLASH */
	int reserved;
} update_flash_t;

typedef struct ivr_ipc_msg_s {
    long int message_type;
    union {
    	do_ivr_ins_t	do_ivr_ins;
    	send_global_t	send_global;
    	update_flash_t	update_flash;
    };
} ivr_ipc_msg_t;

/* =============================================================== */
/* server -> client message */
#define IVR_IPC_RET_NOT_IVR_INS		0	/* Not IVR instruction */
#define IVR_IPC_RET_BUSY_TONE		1	/* IVR instruction, and play busy tone continuously. */
#define IVR_IPC_RET_VOICE_CFG		2	/* IVR instruction, and voice a configuration */

typedef struct ivr_ipc_ret_s {
    long int message_type;
    int ret;
} ivr_ipc_ret_t;

#define IVR_RET_DATA_SIZE		sizeof( int )
#endif /* __IVR_IPC_H__ */

