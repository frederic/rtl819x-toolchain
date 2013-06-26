/*
 * This file provide flash client interface to access flash, 
 * so application should link voip_flash_client.a. 
 * Also, include following .h files, and give a suitable path. 
 */
#ifndef __VOIP_FLASH_CLIENT_H__
#define __VOIP_FLASH_CLIENT_H__

#include "kernel_config.h"
#include "rtk_voip.h"
#include "voip_flash.h"

//------------------------------------------------------------------
//			VOIP FLASH Write
//------------------------------------------------------------------
#define VOIP_FLASH_WRITE_PATHNAME		"/web/voip_script.js"		// use for ipc 
#define VOIP_FLASH_WRITE_SERVER			0x2F			// use for ipc
#define VOIP_FLASH_WRITE_CLIENT_BASE	0x60			// use for ipc 

/**
 * @ingroup VOIP_CONFIG
 * Enumeration for Voip Flash Write IPC 
 */
enum _VOIP_FLASH_WRITE_IPC
{
	VFWM_CMD_WRITE = 1,	/* VFWM is short for 'Voip Flash Write Message' */
	VFWM_CMD_WRITE_ACK,
	VFWM_CMD_VALIDATE,
	VFWM_CMD_VALIDATE_ACK,
	VFWM_CMD_MIBSET,
	VFWM_CMD_MIBSET_ACK,
	VFWM_CMD_MIBGET,
	VFWM_CMD_MIBGET_ACK,
	VFWM_CMD_MIBUPDATE,
	VFWM_CMD_MIBUPDATE_ACK,	
	VFWM_CMD_NOP,
};

// ---------------------------------------------------------------
// Flash Write Message Structure 
// ---------------------------------------------------------------

#define MAX_WRITE_LENGTH		256
#define MAX_MIBMSG_ORDER		12		/* maximum 12 ( 2 ^ 12 = 4096 ) */
#define MAX_MIBMSG_ATOM			( 1 << MAX_MIBMSG_ORDER )/* MSGMAX = 8192; So, maximum is 4096 */
#define MAX_MIBMSG_MOD( x )		( ( x ) & ( MAX_MIBMSG_ATOM - 1 ) )
#define MAX_MIBMSG_ATOM_NUM		3
#define MAX_MIBMSG_LENGTH		( MAX_MIBMSG_ATOM * MAX_MIBMSG_ATOM_NUM )		/* 4096 * 3 = 12288 */
#define VFWM_PAYLOAD_SIZE( x )	( sizeof( x ) - sizeof( unsigned long ) )

typedef struct {
	unsigned long type;
	unsigned long len;
	cid_t         cid;
} VoipFlashWriteMsgHeader_t;

typedef struct {
	unsigned long type;	/* = VFWM_CMD_WRITE */
	unsigned long len;	/* ONLY include part of data_xxxx */
	cid_t         cid;
	unsigned long data_offset;
	unsigned long data_len;
	unsigned char data[ MAX_WRITE_LENGTH ];
} VoipFlashWriteMsgWrite_t;

typedef struct {
	unsigned long type;	/* = VFWM_CMD_WRITE_ACK */
	unsigned long len;
	cid_t         cid;
} VoipFlashWriteMsgWriteAck_t;

typedef struct {
	unsigned long type;	/* = VFWM_CMD_VALIDATE */
	unsigned long len;
	cid_t         cid;
} VoipFlashWriteMsgValidate_t;

typedef struct {
	unsigned long type;	/* = VFWM_CMD_VALIDATE_ACK */
	unsigned long len;
	cid_t         cid;
} VoipFlashWriteMsgValidateAck_t;

typedef struct {
	unsigned long type;	/* = VFWM_CMD_MIBSET */
	unsigned long len;
	cid_t         cid;
	unsigned long data_id;
	unsigned long data_len;
	unsigned long data_offset;	/* indicate n-st atom */
	unsigned char data[ MAX_MIBMSG_ATOM ];
} VoipFlashWriteMsgMibSet_t;

typedef struct {
	unsigned long type;	/* = VFWM_CMD_MIBSET_ACK  */
	unsigned long len;
	cid_t         cid;
	unsigned long result;	/* 0 indicates fail */
} VoipFlashWriteMsgMibSetAck_t;

typedef struct {
	unsigned long type;	/* = VFWM_CMD_MIBGET */
	unsigned long len;
	cid_t         cid;
	unsigned long data_id;
	unsigned long data_len;
	unsigned long data_offset;	/* indicate n-st atom */
} VoipFlashWriteMsgMibGet_t;

typedef struct {
	unsigned long type;	/* = VFWM_CMD_MIBGET_ACK */
	unsigned long len;
	cid_t         cid;
	unsigned long result;	/* 0 indicates fail */
	unsigned long data_id;
	unsigned long data_len;
	unsigned long data_offset;	/* indicate n-st atom */
	unsigned char data[ MAX_MIBMSG_ATOM ];
} VoipFlashWriteMsgMibGetAck_t;

typedef struct {
	unsigned long type;	/* = VFWM_CMD_MIBUPDATE */
	unsigned long len;
	cid_t         cid;
	unsigned long update_type;
} VoipFlashWriteMsgMibUpdate_t;

typedef struct {
	unsigned long type;	/* = VFWM_CMD_MIBUPDATE_ACK  */
	unsigned long len;
	cid_t         cid;
	unsigned long result;	/* 0 indicates fail */
} VoipFlashWriteMsgMibUpdateAck_t;

typedef struct {
	unsigned long type;	/* = VFWM_CMD_NOP */
} VoipFlashWriteMsgNop_t;
	
typedef union {
	VoipFlashWriteMsgHeader_t		header;
	VoipFlashWriteMsgWrite_t		write;
	VoipFlashWriteMsgWriteAck_t		writeAck;
	VoipFlashWriteMsgValidate_t		validate;
	VoipFlashWriteMsgValidateAck_t	validateAck;
	VoipFlashWriteMsgMibSet_t		mibset;
	VoipFlashWriteMsgMibSetAck_t	mibsetAck;
	VoipFlashWriteMsgMibGet_t		mibget;
	VoipFlashWriteMsgMibGetAck_t	mibgetAck;
	VoipFlashWriteMsgMibUpdate_t	mibUpdate;
	VoipFlashWriteMsgMibUpdateAck_t	mibUpdateAck;
	VoipFlashWriteMsgNop_t			nop;
} VoipFlashWriteMsg_t;

/* We place prototype of these three functions in voip_flash.h to be compatible with old code. */
//int voip_flash_client_init(voip_flash_share_t **ppVoIPShare);
//void voip_flash_client_close(void);
//int voip_flash_client_update(void);

#ifdef SUPPORT_VOIP_FLASH_WRITE
extern int voip_flash_client_write( unsigned long offset, unsigned long len, const unsigned char *pData, unsigned int bIgnoreIntr );
extern int voip_flash_client_validate_writing( unsigned int bIgnoreIntr );
/*+++++add by Jack for tr-069 configuration+++++*/
extern int cmd_reboot();
extern int va_cmd(const char *cmd, int num, int dowait, ...);
extern int do_cmd(const char *filename, char *argv [], int dowait);
/*-----end-----*/
#endif 

#ifdef SUPPORT_VOIP_FLASH_WRITE
  #ifdef CONFIG_RTK_VOIP_PACKAGE_8186
/* These functions are platform dependent, but we implement them in 8186 now. */
/* Thus, we rename them to fit mib usage convention. */
#define mib_client_set( id, data, len )	mib_set_8186( id, data, len )
#define mib_client_get( id, data, len )	mib_get_8186( id, data, len )
#define mib_client_update( type )		mib_update_8186( type )

extern int mib_set_8186( unsigned long id, void *data, unsigned long data_len );
extern int mib_get_8186( unsigned long id, void *data, unsigned long data_len );
extern int mib_update_8186( CONFIG_DATA_T /*unsigned long*/ update_type );

/* In order to be compatible with all platforms. */
static inline int mib_set_8186_byte( unsigned long id, unsigned char *data )
{
	int temp;
	temp = *data;
	return mib_set_8186( id, &temp, sizeof( temp ) );
}

static inline int mib_set_8186_word( unsigned long id, unsigned short *data )
{
	int temp;
	temp = *data;
	return mib_set_8186( id, &temp, sizeof( temp ) );
}

static inline int mib_get_8186_byte( unsigned long id, unsigned char *data )
{
	int ret, temp;
	ret = mib_get_8186( id, &temp, sizeof( temp ) );
	*data = ( unsigned char )( unsigned int )temp;
	return ret;
}

static inline int mib_get_8186_word( unsigned long id, unsigned short *data )
{
	int ret, temp;
	ret = mib_get_8186( id, &temp, sizeof( temp ) );
	*data = ( unsigned short )( unsigned int )temp;
	return ret;
}
  #endif /* CONFIG_RTK_VOIP_PACKAGE_8186 */
#endif /* SUPPORT_VOIP_FLASH_WRITE */

#endif /* __VOIP_FLASH_CLIENT_H__ */


