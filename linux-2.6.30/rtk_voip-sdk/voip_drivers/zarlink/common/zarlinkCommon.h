#ifndef __ZARLINKCOMMON_H__
#define __ZARLINKCOMMON_H__

#include "vp_api_int.h"
#include "vp_api_option.h"
#include "voip_debug.h" /* for PRINT_R PRINT_MSG */
#include "rtk_voip.h"	/* for SLIC_NUM, DAA_NUM */
#include "voip_timer.h"

/* for 89xx series. TODO need check 88xx series */
#define RING_PROFILE_CAD_ON_H_IDX		10
#define RING_PROFILE_CAD_ON_L_IDX		11
#define DEVICE_PROFILE_TICK_RATE_IDX	12
#define RING_PROFILE_CAD_OFF_H_IDX		14
#define RING_PROFILE_CAD_OFF_L_IDX		15

/* TODO add CONFIG_RTK_VOIP_SLIC_NUM support in KConfig */
/* (compatible with older design) */
#define ZARLINK_SLIC_DEV_NUM 	MAX(SLIC_CH_NUM,DAA_CH_NUM)
#define ZARLINK_SLIC_CH_NUM 	(SLIC_CH_NUM + DAA_CH_NUM)
#define ZARLINK_FXS_LINE_NUM	SLIC_CH_NUM
#define ZARLINK_FXO_LINE_NUM	DAA_CH_NUM

#define ASSERT_zarlink(st) if (st != VP_STATUS_SUCCESS)\
	PRINT_ERR(st,"Call DEV_FUNC return fail")

#define PRINT_ERR(st,str) PRINT_R("Error %d (%d:%d) %s %s\n",st,pLine->pDev->dev_id,pLine->ch_id,__FUNCTION__,str)


typedef enum {
	DEV_UNKNOWN,
	DEV_FXS,
	DEV_FXO,
	DEV_FXSFXS,
	DEV_FXSFXO,
	DEV_LAST 
} RTKDevType;

typedef enum {
	LINE_UNKNOWN,
	LINE_FXS,
	LINE_FXO
} RTKLineType;

typedef enum {
	LINE_S_NOT_INIT,
	LINE_S_IDLE,
	LINE_S_READY,
} RTKLineState;

typedef enum {
	DEV_S_NOT_INIT,
	DEV_S_IDLE,
	DEV_S_READY,
} RTKDevState;

typedef enum {
    VPIO_IO1 = 0x01,  
    VPIO_IO2 = 0x02,
    VPIO_IO3 = 0x04,
    VPIO_IO4 = 0x08,
} VPIO;

#define GPIO_NEW_MASK 0x0000FFFF // not apply yet
#define GPIO_CUR_MASK 0xFFFF0000 // have applied. appy every 10ms refer to api_tick

typedef struct ZarlinkDevObj_t RTKDevObj;
typedef struct RTKLineObj_t RTKLineObj; 

struct ZarlinkDevObj_t{
	RTKDevState				dev_st;
	unsigned int			dev_id;
	unsigned int 			max_line;
	RTKDevType 				dev_type;
	RTKLineObj				*pLine[VP_MAX_LINES_PER_DEVICE];

	unsigned int			gpio_dir;
	unsigned int			gpio_dat;

	VpDeviceType			VpDevType;

	/* Will be Vp880DeviceObjectType or Vp890DeviceObjectType */
	void  					*pDevObj; 

	VpDevCtxType          	*pDevCtx; 

	/* Ring Cadence. TODO Should change to line base*/
	unsigned int			cad_on_ms;
	unsigned int			cad_off_ms;

	/* profile */
	VpProfilePtrType 		pDev_profile;
	VpProfilePtrType 		pAC_profile;
	VpProfilePtrType 		pDC_profile; 		/* DC profile        */
	VpProfilePtrType 		pRing_profile;		/* Ring amp and freq */
	VpProfilePtrType 		pACFxoLC_profile;
	VpProfilePtrType 		pFxoDial_profile;

	VpProfilePtrType 		pRing_cad_usr_profile;

	/* Device api function */
	unsigned char (*SendNTTCAR_Check)(RTKLineObj *pLine, unsigned long time_out);
	unsigned char (*GetFxsHookStatus)(RTKLineObj *pLine, int from_polling_timer);
	unsigned char (*GetLineState)(RTKLineObj *pLine);
	unsigned char (*CheckFxsRing)(RTKLineObj *pLine);

	VpStatusType (*SetFxsPcmMode)(RTKLineObj *pLine, int pcm_mode);
	VpStatusType (*SetFxsAcProfileByBand)(RTKLineObj *pLine, int pcm_mode);
	VpStatusType (*SetRingCadenceProfile)(RTKLineObj *pLine, uint8 ring_cad);
	VpStatusType (*SetRingCadence)(RTKLineObj *pLine, 
					unsigned short on_msec, unsigned short off_msec);
	VpStatusType (*SetRingFreqAmp)(RTKLineObj *pLine, uint8 profile);
	VpStatusType (*FxsRing)(RTKLineObj *pLine, uint8 enable);
	VpStatusType (*SendNTTCAR)(RTKLineObj *pLine);

	VpStatusType (*SetLineState)(RTKLineObj *pLine, VpLineStateType state);
	VpStatusType (*SetOHT)(RTKLineObj *pLine, uint8 reversal);
	VpStatusType (*SetLineOpen)(RTKLineObj *pLine);
	VpStatusType (*SetPcmTxOnly)(RTKLineObj *pLine, int enable);
	VpStatusType (*SetPcmRxOnly)(RTKLineObj *pLine, int enable);
	VpStatusType (*SetImpedenceCountry)(RTKLineObj *pLine, uint8 country);
	VpStatusType (*SetImpedence)(RTKLineObj *pLine, uint16 preset);
	VpStatusType (*TxGainAdjust)(RTKLineObj *pLine, int gain);  
	VpStatusType (*RxGainAdjust)(RTKLineObj *pLine, int gain);  


	/* Zarlink GPIO function */
	VpStatusType (*SetIODir)  (RTKLineObj *pLine, VPIO IO, uint32 bOut);
	VpStatusType (*SetIOState)(RTKLineObj *pLine, VPIO IO, uint32 bHigh);
	VpStatusType (*GetIOState)(RTKLineObj *pLine, VPIO IO, uint32 *bHigh);
	VpStatusType (*UpdIOState)(RTKLineObj *pLine);

	/********** DAA Function **********/
	BOOL (*GetFxoLineStatus)(RTKLineObj *pLine, uint8 category);
	unsigned char (*GetFxoHookStatus)(RTKLineObj *pLine);
	VpStatusType (*SetFxoLineState)(RTKLineObj *pLine, VpLineStateType lineState);
	VpStatusType (*SetFxoLineOnHook)(RTKLineObj *pLine);
	VpStatusType (*SetFxoLineOffHook)(RTKLineObj *pLine);
	VpStatusType (*SetFxoLineOHT)(RTKLineObj *pLine);
	VpStatusType (*SetFxoRingValidation)(RTKLineObj *pLine);

	/* Debug function */
	VpStatusType (*DumpDevReg)(RTKDevObj *pDev);
	VpStatusType (*RWDevReg)(RTKLineObj *pLine, unsigned int reg, unsigned char *len, char *regdata);
};

struct RTKLineObj_t{
 	RTKLineType				line_st;
	unsigned int 			ch_id;
	unsigned int			slot_tx;
	unsigned int			slot_rx;

	unsigned int 			channelId; 		/* usually 0 or 1 */
	unsigned int 			hook_st; 		/* usually 0 or 1 */
	unsigned char			pcm_law_save;	/* pcm_mode */

	RTKLineType 			line_type;
	RTKDevObj				*pDev;

	/* Will be Vp880LineObjectType or Vp890LineObjectType */
	void					*pLineObj;

	VpLineCtxType           *pLineCtx;
	VpOptionCodecType 		codec_type;		/* convert from rtk pcm_mode */

	/* profile */
	unsigned char			AC_country;
	VpProfilePtrType 		pAC_profile;
	VpProfilePtrType 		pDCfxo_profile; /* DC or FXO profile */
	VpProfilePtrType 		pRing_profile;
	VpProfilePtrType 		pRing_cad_profile;

	unsigned int			TxGainAdj; 		
	unsigned int			RxGainAdj;		
};

#define RTK_CALL_DEV_FUNC(func, args) \
    (((pLine->pDev->func) == VP_NULL) ? VP_STATUS_FUNC_NOT_SUPPORTED : (pLine->pDev->func) args )

#endif /* __ZARLINKCOMMON_H__ */


