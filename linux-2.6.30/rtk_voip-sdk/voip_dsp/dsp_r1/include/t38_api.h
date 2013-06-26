#ifndef __T38_API_H__
#define __T38_API_H__

typedef enum {
	T38_MAX_RATE_2400	= 0, 
	T38_MAX_RATE_4800	= 1, 
	T38_MAX_RATE_7200	= 2, 
	T38_MAX_RATE_9600	= 3, 
	T38_MAX_RATE_12000	= 4, 
	T38_MAX_RATE_14400	= 5, 
} T38MaxBitRate_t;

typedef enum {
	T38_ECC_TYPE_NONE			= 0, 
	T38_ECC_TYPE_REDUNDANCY		= 1, 
	T38_ECC_TYPE_FEC			= 2, 
} T38FaxUdpEC_t;

typedef struct t38Capability_s {
    unsigned short T38FaxVersion;
	T38MaxBitRate_t T38MaxBitRate;
	unsigned short T38FaxFillBitRemoval;	// trun or false
	unsigned short T38FaxTrancodingMMR;		// trun or false
	unsigned short T38FaxTranscodingJBIG;	// trun or false
	unsigned short T38FaxRateManagement;	// trun or false
	unsigned short T38FaxMaxBuffer;			// quantity
	unsigned short T38FaxMaxDatagram;		// quantity
	T38FaxUdpEC_t T38FaxUdpEC;
} t38Capability_t;

typedef struct t38Param_s {
	short pECC_Signal;	/* # of Redundant frames to use for T.30 signalling frames */
						/* (max 7, default 5) */
	short pECC_Data;	/* # of Redundant frames to use for image data packets frames */
						/* (max 2, default 2) */
	uint32 nPreambleDelayThres;	/* in unit of 10ms, so ( 3 * 1000 / 10 ) indicates 3 sec [default value]. */
	short nImageFrameSize;	/* 10, 20, 30 or 40 [default: 40] */
	short nMaxBuffer;	/* 200~600ms; if out of range, use default [default: 500] */
	
	short nRateManagement;	/* 1: do not pass tcf data, 2: pass tcf data [default: 2] */
	T38MaxBitRate_t nMaxRate;	/* rate limit [default: T38_MAX_RATE_14400] */
	short pEnableECM;	/* 1: enable, 0: disable [default: 1] */
	short bForceTrainZero;	/* 1: enable, 0: disable [default: 0] */
	short bEnableSpoof;	/* 1: enable, 0: disable [default: 1] */
	short nDuplicateNum;	/* # of Duplicate packet to use for T.38 */
						/* (max 2, default 0) */
} t38Param_t;

#define T38_DEFAULT_PARAM_LIST() 		\
						{ 5, 2, ( 0 * 1000 / 10 ), 40, 500, \
						2, T38_MAX_RATE_14400, 1, 0, 1, 0 }

extern void T38_API_Capability( t38Capability_t *pCapability );
extern void T38_API_Initialize( uint32 chid, const t38Param_t *pParam );
extern uint32 T38_API_EncodeDecodeProcess( uint32 chid, 
								const unsigned short *pPcmInBuffer,
								unsigned short *pPcmOutBuffer,
								void *pPacketOutBuffer,
								int32 *pOutputBufferSize );
extern uint32 T38_API_PutPacket( uint32 chid, const unsigned char *pPacketInBuffer, uint32 nPacketInSize );

#endif /* __T38_API_H__ */

