#include <stdio.h>
#include "ui_config.h"
#include "ioctl_kernel.h"
#include "ioctl_codec.h"

int SetVolumeOfDAC( unsigned char volmic, unsigned char volspk )
{
	/* volume range 0 ~ MAX_VOLUME_VALUE */
	static unsigned char pre_volmic = 0;
	static unsigned char pre_volspk = 0;

	if( ( pre_volmic && ( pre_volmic == volmic ) ) &&
		( pre_volspk && ( pre_volspk == volspk ) ) )
	{
		return 0;
	}

	pre_volmic = volmic;
	pre_volspk = volspk;

	debug_out( "Set DAC volume: (%u,%u)\n", volmic, volspk );

#ifdef _DSP_VOLUME_CONTROLLER
	/*
	 * 0: mute
	 * 1: -31dB
	 * 32: 0dB
	 * 63: 31dB
	 */
	// TODO: In final version, we adjust speaker volume only. 
	//return rtk_Set_Voice_Gain( IP_CHID, ( int )volume - 32, ( int )volume - 32 );
	//return rtk_Set_Voice_Gain( IP_CHID, 0, ( int )volume - 32 );
	return rtk_Set_Voice_Gain( IP_CHID, ( int )volspk - 32, ( int )volmic - 32 );
#else
	/* 
	 * 0000 0000: mute
	 * 0000 0001: -127 dB
	 * 0000 0010: -126.5 dB
	 * ... 0.5 dB steps up to 
	 * 1111 1111: 0 dB <-- default value 
	 */

	return rtk_Set_IPhone( 	0	/* write */, 
							11	/* R11: DAC Digital Volume */, 
							volume /* volume */ );
#endif /* _DSP_VOLUME_CONTROLLER */
}

#if 0
//unsigned char GetDefaultVolumeOfDAC( void )
//{
//	/* NOTE: It get *DEFAULT* volume, so it will NOT up to date. */
//	unsigned int volume;
//
//	rtk_Set_IPhone( 	1	/* write */, 
//						11	/* R11: DAC Digital Volume */, 
//						 /* volume */ );
//
//	return ( unsigned char )volume;
//}
#endif

int SetCodecVoicePath( vpMicSpeacker_t path )
{
	extern int rtk_SetVoicePath( vpMicSpeacker_t path );
	
	static vpMicSpeacker_t pre_path = NUM_OF_VOC_PATH;
	
	if( pre_path == path )
		return 0;
	
	pre_path = path;
	
	debug_out( "Voice Path: %u\n", path );
	
	return rtk_SetVoicePath( path );
}

