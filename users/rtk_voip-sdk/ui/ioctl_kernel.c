#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "ui_config.h"
#include "ioctl_kernel.h"
#include "ioctl_codec.h"	/* voice path */

/* included in rtk_voip/include/ */
//#include "type.h"
//#include "voip_params.h"
//#include "voip_control.h"
#include "voip_manager.h"
#include "voip_ioctl.h"

/* ************************************************************* */
/* Keypad Interface */
/* ************************************************************* */
int rtk_SetKeypadSetTarget( void )
{
	TstKeypadSetTarget stKeypadSetTarget;
	
	stKeypadSetTarget.cmd = KEYPAD_CMD_SET_TARGET;
	stKeypadSetTarget.pid = getpid();

    SETSOCKOPT(VOIP_MGR_CTL_KEYPAD, &stKeypadSetTarget, TstKeypadSetTarget, 1);
    
    return 0;
}

int rtk_SetKeypadUnsetTarget( void )
{
	TstKeypadSetTarget stKeypadSetTarget;
	
	stKeypadSetTarget.cmd = KEYPAD_CMD_SET_TARGET;
	stKeypadSetTarget.pid = 0;

    SETSOCKOPT(VOIP_MGR_CTL_KEYPAD, &stKeypadSetTarget, TstKeypadSetTarget, 1);
    
    return 0;
}

int rtk_SetKeypadSignalDebug( wkey_t wkey )
{
	TstKeypadSignalDebug stKeypadSignalDebug;
	
	stKeypadSignalDebug.cmd = KEYPAD_CMD_SIG_DEBUG;
	stKeypadSignalDebug.wkey = wkey;
	
	SETSOCKOPT(VOIP_MGR_CTL_KEYPAD, &stKeypadSignalDebug, TstKeypadSignalDebug, 1);

    return 0;
}

int rtk_SetKeypadReadKey( wkey_t *pWKey )
{
	TstKeypadReadKey stKeypadReadKey;
	
	stKeypadReadKey.cmd = KEYPAD_CMD_READ_KEY;
	
	SETSOCKOPT(VOIP_MGR_CTL_KEYPAD, &stKeypadReadKey, TstKeypadReadKey, 1);

	/* kernel will fill give key */
	*pWKey = stKeypadReadKey.wkey;
	
	if( stKeypadReadKey.validKey )
		return 0;	/* ok. */

    return -3;
}

int rtk_GetKeypadHookStatus( void )
{
	TstKeypadHookStatus	stKeypadHookStatus;
	
	stKeypadHookStatus.cmd = KEYPAD_CMD_HOOK_STATUS;
	
	SETSOCKOPT(VOIP_MGR_CTL_KEYPAD, &stKeypadHookStatus, stKeypadHookStatus, 1);
		
	return stKeypadHookStatus.status;
}

/* ************************************************************* */
/* PCM Interface */
/* ************************************************************* */
int rtk_eanblePCM(unsigned int chid, unsigned int val)
{
	extern int32 rtk_OnHookReInitKernelVariables( uint32 chid );
	
	static unsigned int nReference = 0;
	
	TstVoipCfg stVoipCfg;
	
	if( val ) {		/* enable */
		nReference ++;
		if( nReference > 1 ) 
			return 0;		/* has been enabled */
	} else {		/* disable */
		if( nReference ) {
			nReference --;
			if( nReference > 0 )
				return 0;	/* reference count > 0 */
		} else {
			debug_out( "PCM reference count less than zero!!!!\n" );
			//return 0;		/* let it disable PCM during boot time. */
		}
		
		rtk_OnHookReInitKernelVariables( IP_CHID );
	}
	
	/* 
	 * do ioctl only if enable and reference count == 1, or 
	 * disable and reference count == 0.
	 */

	stVoipCfg.ch_id = chid;
	stVoipCfg.enable = val;
	SETSOCKOPT(VOIP_MGR_PCM_CFG, &stVoipCfg, TstVoipCfg, 1);
#ifdef AUDIOCODES_VOIP	
	SETSOCKOPT(VOIP_MGR_SLIC_OFFHOOK_ACTION, &stVoipCfg, TstVoipCfg, 1);
#endif
	return 0;
}

/* ************************************************************* */
/* LCD Interface  */
/* ************************************************************* */
int rtk_SetLcdDisplayOnOff( unsigned char bDisplayOnOff,
							unsigned char bCursorOnOff,
							unsigned char bCursorBlink )
{
	TstLcmDisplayOnOff stLcmDisplayOnOff;
	
	stLcmDisplayOnOff.cmd = LCM_CMD_DISPLAY_ON_OFF;
	stLcmDisplayOnOff.bDisplayOnOff = ( bDisplayOnOff ? 1 : 0 );
	stLcmDisplayOnOff.bCursorOnOff = ( bCursorOnOff ? 1 : 0 );
	stLcmDisplayOnOff.bCursorBlink = ( bCursorBlink ? 1 : 0 );
	
	SETSOCKOPT(VOIP_MGR_CTL_LCM, &stLcmDisplayOnOff, TstLcmDisplayOnOff, 1);
	
    return 0;
}

int rtk_SetLcdMoveCursorPosition( int x, int y )
{
	TstLcmMoveCursorPosition stLcmMoveCursorPosition;
	
	stLcmMoveCursorPosition.cmd = LCM_CMD_MOVE_CURSOR_POS;
	stLcmMoveCursorPosition.x = x;
	stLcmMoveCursorPosition.y = y;
	
	SETSOCKOPT(VOIP_MGR_CTL_LCM, &stLcmMoveCursorPosition, TstLcmMoveCursorPosition, 1);
	
    return 0;
}

int rtk_SetLcdDrawText( int x, int y, unsigned char *pszText, int len )
{
	TstLcmDrawText stLcmDrawText;
	
	stLcmDrawText.cmd = LCM_CMD_DRAW_TEXT;
	stLcmDrawText.x = x;
	stLcmDrawText.y = y;
	if( len > MAX_LEN_OF_DRAW_TEXT )
		len = MAX_LEN_OF_DRAW_TEXT;
	memcpy( stLcmDrawText.szText, pszText, len );
	stLcmDrawText.len = len;
	
	SETSOCKOPT(VOIP_MGR_CTL_LCM, &stLcmDrawText, TstLcmDrawText, 1);
	
    return 0;	
}

int rtk_SetLcdWriteData( int start, const unsigned char *pdata, int len )
{
	TstLcmWriteData stLcmWriteData;
	
	stLcmWriteData.cmd = LCM_CMD_WRITE_DATA;
	stLcmWriteData.start = start;
	if( len > MAX_LEN_OF_WRITE_DATA )
		len = MAX_LEN_OF_WRITE_DATA;
	memcpy( stLcmWriteData.pixels, pdata, len );
	stLcmWriteData.len = len;
	
	SETSOCKOPT(VOIP_MGR_CTL_LCM, &stLcmWriteData, TstLcmWriteData, 1);
	
    return 0;	
}

int rtk_SetLcdWriteData2( int page, int col, const unsigned char *pdata, int len )
{
	TstLcmWriteData2 stLcmWriteData2;
	
	stLcmWriteData2.cmd = LCM_CMD_WRITE_DATA2;
	stLcmWriteData2.page = page;
	stLcmWriteData2.col = col;
	if( len > MAX_LEN_OF_WRITE_DATA2 )
		len = MAX_LEN_OF_WRITE_DATA2;
	memcpy( stLcmWriteData2.pixels, pdata, len );
	stLcmWriteData2.len = len;
	
	SETSOCKOPT(VOIP_MGR_CTL_LCM, &stLcmWriteData2, TstLcmWriteData2, 1);
	
    return 0;	
}

int rtk_SetLcdDirtyMmap( int start, int len )
{
	TstLcmDirtyMmap stLcmDirtyMmap;
	
	stLcmDirtyMmap.cmd = LCM_CMD_DIRTY_MMAP;
	stLcmDirtyMmap.start = start;
	stLcmDirtyMmap.len = len;
	
	SETSOCKOPT(VOIP_MGR_CTL_LCM, &stLcmDirtyMmap, TstLcmDirtyMmap, 1);
	
    return 0;	
}

int rtk_SetLcdDirtyMmap2( int page, int col, int len, int rows )
{
	TstLcmDirtyMmap2 stLcmDirtyMmap2;
	
	stLcmDirtyMmap2.cmd = LCM_CMD_DIRTY_MMAP2;
	stLcmDirtyMmap2.page = page;
	stLcmDirtyMmap2.col = col;
	stLcmDirtyMmap2.len = len;
	stLcmDirtyMmap2.rows = rows;
	
	SETSOCKOPT(VOIP_MGR_CTL_LCM, &stLcmDirtyMmap2, TstLcmDirtyMmap2, 1);
	
    return 0;	
}

/* ************************************************************* */
/* LED interface  */
/* ************************************************************* */
int rtk_SetLedOnOff( unsigned long led )
{
	TstLedCtl ledCtl;

	ledCtl.led = led;

	SETSOCKOPT( VOIP_MGR_CTL_LED, &ledCtl, TstLedCtl, 1 );

	return 0;
}

/* ************************************************************* */
/* Codec interface  */
/* ************************************************************* */
int rtk_Set_IPhone(unsigned int function_type, unsigned int reg, unsigned int value)
{
	IPhone_test iphone;
	
	iphone.function_type = function_type;
	iphone.reg = reg;
	iphone.value = value;
	SETSOCKOPT(VOIP_MGR_IPHONE_TEST, &iphone, IPhone_test, 1);
	return 0;
}

/* make sure AP and kernel IDs are identical. */
CT_ASSERT( VOC_PATH_MIC1_SPEAKER == VPATH_MIC1_SPEAKER );
CT_ASSERT( VOC_PATH_MIC2_MONO == VPATH_MIC2_MONO );
CT_ASSERT( VOC_PATH_SPEAKER_ONLY == VPATH_SPEAKER_ONLY );
CT_ASSERT( VOC_PATH_MONO_ONLY == VPATH_MONO_ONLY );

int rtk_SetVoicePath( vpMicSpeacker_t vocpath )
{	
	TstVoicePath_t stVoicePath;
	
	stVoicePath.vpath = vocpath;
	SETSOCKOPT(VOIP_MGR_CTL_VOICE_PATH, &stVoicePath, TstVoicePath_t, 1);
	
	return 0;
}

/* ************************************************************* */
/* Clone functions of voip_manager  */
/* ************************************************************* */
int32 rtk_SetPlayTone(uint32 chid, uint32 sid, DSPCODEC_TONE nTone, uint32 bFlag,
	DSPCODEC_TONEDIRECTION Path)
{
    TstVoipPlayToneConfig cfg;

	/* RING is incoming ring tone, and RINGING is ring back tone. */
    //if (nTone == DSPCODEC_TONE_RING)
    //    nTone = DSPCODEC_TONE_RINGING;

    cfg.ch_id = chid;
    cfg.m_id = sid;
    cfg.nTone = nTone;
    cfg.bFlag = bFlag;
    cfg.path = Path;

    SETSOCKOPT(VOIP_MGR_SETPLAYTONE, &cfg, TstVoipPlayToneConfig, 1);

    return 0;
}

int32 rtk_Set_Voice_Gain(uint32 chid, int spk_gain, int mic_gain)
{
	TstVoipValue stVoipValue;

	stVoipValue.ch_id = chid;   
	stVoipValue.value = spk_gain;
	stVoipValue.value1 = mic_gain;
	
	SETSOCKOPT(VOIP_MGR_SET_VOICE_GAIN, &stVoipValue, TstVoipValue, 1);

	return 0;   
}

/* ************************************************************* */
/* Port solar functions to UI */
/* ************************************************************* */
int32 rtk_OnHookReInitKernelVariables( uint32 chid )
{
	TstVoipCfg stVoipCfg;
	
	stVoipCfg.ch_id = chid;

	SETSOCKOPT(VOIP_MGR_ON_HOOK_RE_INIT, &stVoipCfg, TstVoipCfg, 1);
	
	return 0;
}

/* ************************************************************* */
/* Misc interface  */
/* ************************************************************* */
int rtk_RetrieveMiscInfo( unsigned long *buildno, unsigned long *builddate )
//int32 rtk_RetrieveMiscInfo( uint32 *buildno, uint32 *builddate )
{
	TstMiscCtl_t stMiscCtl;

	SETSOCKOPT(VOIP_MGR_CTL_MISC, &stMiscCtl, TstMiscCtl_t, 1);

	*buildno = stMiscCtl.buildno;
	*builddate = stMiscCtl.builddate;

	return 0;
}
