#include "rtk_voip.h"
#include "voip_types.h"
#include "voip_limit.h"
#include "voip_init.h"
#include "voip_proc.h"


#if !defined (AUDIOCODES_VOIP)

/*
 * 3 channel example:
 *  
 * chid     mid           sid (=chid+mid*3)
 *          rtp rtcp      rtp rtcp
 * ---------------------------------
 *  0       0/1 2/3       0/3 6/9
 *  1       0/1 2/3       1/4 7/10
 *  2       0/1 2/3       2/5 8/11
 */

typedef struct _channel_config_s
{
#ifdef SUPPORT_3WAYS_AUDIOCONF
	uint32 isConference;
#endif
	uint32 tranSessId;
	
} channel_config_t;

// --------------------------------------------------------
// transfer ID - regular function
// --------------------------------------------------------

static channel_config_t chanInfo[MAX_DSP_RTK_CH_NUM];

extern int dsp_rtk_ch_num;
extern int dsp_rtk_ss_num;

uint32 API_GetSid(uint32 chid, uint32 mid) 
{
	uint32 sid;
	
	if(chid >= dsp_rtk_ch_num || mid >= MAX_MID_NUM )
		return SESSION_NULL;

	sid = chid + mid * dsp_rtk_ch_num;
	
	return sid;
}

uint32 API_GetMid(uint32 chid, uint32 sid) 
{
	// sid = chid + mid * __ch_num
	uint32 mid;
	
	if( chid >= dsp_rtk_ch_num || sid >= dsp_rtk_ss_num )
		return SESSION_NULL;
	
	for( mid = 0; mid < MAX_MID_NUM; mid ++ ) {
		if( sid == chid + mid * dsp_rtk_ch_num )
			return mid;
	}
	
	return SESSION_NULL;
}

uint32 API_CloseSid(uint32 chid, uint32 mid) 
{
	return API_GetSid( chid, mid );
}

uint32 API_OpenSid(uint32 chid, uint32 mid) 
{
	return API_GetSid( chid, mid );	// the same 
}

/* session */
uint32 chanInfo_GetChannelbySession(uint32 sid) 
{
	uint32 chid;
	
	if(sid >= dsp_rtk_ss_num)
		return CHANNEL_NULL;
	
	for( chid = sid; chid >= dsp_rtk_ch_num; chid -= dsp_rtk_ch_num ); 
	
	return chid;
}

uint32 chanInfo_IsSessionFull(void) 
{
	return 1;
}

void InitChanInfo(uint32 chid) 
{
#ifdef SUPPORT_3WAYS_AUDIOCONF
	chanInfo[chid].isConference = 0;
#endif
	chanInfo[chid].tranSessId = chid;
}

#ifdef SUPPORT_3WAYS_AUDIOCONF
uint32 chanInfo_SetConference(uint32 chid, uint32 bEnable) 
{
	chanInfo[chid].isConference = bEnable;
	//gnet_debug = 1;
	return 0;
}

uint32 chanInfo_IsConference(uint32 chid) 
{
	return chanInfo[chid].isConference;
}
#endif

uint32 chanInfo_IsActiveSession(uint32 chid, uint32 sid) 
{
	if( chanInfo_GetChannelbySession( sid ) 
		!= chid ) 
	{
		printk( "Not match!! chid=%d, sid=%d\n", chid, sid );
		return 0;	// check fail 
	}
	
	if(chanInfo[chid].isConference)
	{
		return 1;
	}
	else
	{
		if(chanInfo[chid].tranSessId == sid)
			return 1;
	}
	return 0;
}

int32 chanInfo_SetTranSessionID(uint32 chid, uint32 sid) 
{
	if( chid >= dsp_rtk_ch_num )
		return FAILED;
	
	if( sid >= dsp_rtk_ss_num )	// handle case of "sid == 255"
		sid = chid;
	
	chanInfo[chid].tranSessId = sid;

	return SUCCESS;
}

int32 chanInfo_CloseSessionID(uint32 chid, uint32 sid) 
{
	return SUCCESS;
}


uint32 chanInfo_GetTranSessionID(uint32 chid) 
{
	return chanInfo[chid].tranSessId;
}

uint32 chanInfo_GetRegSessionNum(uint32 chid) 
{
	return MAX_MID_NUM;
}

uint32 chanInfo_GetRegSessionID(uint32 chid, uint32 reg_idx) 
{
	return API_GetSid( chid, reg_idx );
}

int32 chanInfo_GetRegSessionRank(uint32 chid, uint32 rank[]) 
{
	int i;
	
	for( i = 0; i < MAX_MID_NUM; i ++ )
		rank[ i ] = API_GetSid( chid, i );
	
	return SUCCESS;
}

// --------------------------------------------------------
// proc 
// --------------------------------------------------------

int voip_dsp_channel_info_read_proc( char *buf, char **start, off_t off, int count, int *eof, void *data )
{
	int n = 0;
	int i;
	
	n += sprintf( buf + n, "DCH  isConf  TrasnSID\n" );
	n += sprintf( buf + n, "---------------------\n" );
	
	for( i = 0; i < MAX_DSP_RTK_CH_NUM; i ++ ) {
		n += sprintf( buf + n, "%-3d    %d     %d\n", i, 
#ifdef SUPPORT_3WAYS_AUDIOCONF
					chanInfo[ i ].isConference, 
#else
					0,
#endif
					chanInfo[ i ].tranSessId );
	}
	
	*eof = 1;
	return n;
}

int __init voip_dsp_channel_info_proc_init( void )
{
	create_proc_read_entry( PROC_VOIP_DIR "/channel_info", 0, NULL, voip_dsp_channel_info_read_proc, NULL );
	
	return 0;
}

void __exit voip_dsp_channel_info_proc_exit( void )
{
	remove_proc_entry( PROC_VOIP_DIR "/channel_info", NULL );
}

voip_initcall_proc( voip_dsp_channel_info_proc_init );
voip_exitcall( voip_dsp_channel_info_proc_exit );

#else

uint32 chanInfo_GetTranSessionID(uint32 chid) {return 2*chid;}

#endif // AUDIOCODES_VOIP

