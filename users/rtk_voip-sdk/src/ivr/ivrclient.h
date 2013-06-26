#ifndef __IVR_CLIENT_H__
#define __IVR_CLIENT_H__

/* Initialize IVR client */
extern void InitIvrClient( void );

/* After initialization, send constant table to server */
extern void SendGlobalConstantToIVR_IPC( int nMaxCodec, const int *pMapSupportedCodec );

/* IVR instruction processor */
extern int IsInstructionForIVR_IPC( ivr_ipc_msg_t *msg );

/* Once flash modified, notice IVR. */
extern void NoticeIVRUpdateFlash_IPC( void );


#endif /* __IVR_CLIENT_H__ */

