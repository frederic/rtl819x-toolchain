#include <linux/string.h>
#include "rtk_voip.h"
#include "voip_feature.h"
#include "voip_init.h"
#include "voip_proc.h"
#include "voip_debug.h"
#include "con_register.h"

VoipFeature_t gVoipFeature = 0;


void __init voip_con_main_init_feature( voip_con_t voip_con[], int num )
{
	int i;
	voip_snd_t *p_snd;
	VoipFeature_t feature = 0;
	uint32 slic = 0, daa = 0, dect = 0, phone = 0;
	
	// platform 
	feature |= RTK_VOIP_PLATFORM_FEATURE;
	
	// misc 
	feature |= RTK_VOIP_ONE_ARM_ROUTER_FEATURE;
	
	// channel
	for( i = 0; i < num; i ++ ) {
		
		p_snd = voip_con[ i ].snd_ptr;
		
		if( p_snd == NULL )
			continue;
		
		switch( p_snd ->snd_type ) {
		case SND_TYPE_FXS:
			slic ++;
			break;
			
		case SND_TYPE_DAA:
		case SND_TYPE_VDAA:
			daa ++;
			break;
			
		case SND_TYPE_DECT:
			dect ++;
			break;
			
		case SND_TYPE_AC:
			//phone = 1;
			slic ++;	// old design, we should modify it later 
			break;
		
		default:
			break;
		}
		
	}
	
	feature |= ( ( slic  << SLIC_NUM_SHIFT  ) & SLIC_NUM_MASK );
	feature |= ( ( daa   << DAA_NUM_SHIFT   ) & DAA_NUM_MASK  );
	feature |= ( ( dect  << DECT_NUM_SHIFT  ) & DECT_NUM_MASK );
	feature |= ( ( phone << PHONE_NUM_SHIFT ) & PHONE_NUM_MASK );
	
	feature |= RTK_VOIP_DAA_TYPE_FEATURE;
	
	// dsp
	feature |= RTK_VOIP_IVR_FEATURE;
	feature |= RTK_VOIP_MW_REALTEK_FEATURE;
	feature |= RTK_VOIP_MW_AUDIOCODES_FEATURE;
	
	// arch
	feature |= RTK_VOIP_IPC_ARCH_FEATURE;
	feature |= RTK_VOIP_IPC_ARCH_TYPE_FEATURE;
	feature |= RTK_VOIP_IPC_ARCH_ROLE_FEATURE;
	feature |= RTK_VOIP_DSP_DEVICE_NUM_FEATURE;
	
	// codec 
	feature |= RTK_VOIP_CODEC_FEATURE;
	
	gVoipFeature = feature;
	
	BOOT_MSG("Get RTK VoIP Feature.\n");
}

// --------------------------------------------------------
// proc 
// --------------------------------------------------------

static int voip_feature_read_proc( char *buf, char **start, off_t off, int count, int *eof, void *data )
{
#if ( ( PLATFORM_MASK >> RTK_VOIP_PLATFORM_SHIFT ) != 0x0C ) ||	\
	( ( PLATFORM_TYPE_MASK >> RTK_VOIP_PLATFORM_SHIFT ) != 0x03 )
#error "Re-write platform_str[]"
#endif

	const char * const platform_str[] = {
		"8186-8186V",		//8186 SoC - 8186V
		"8186-8186PV",		//8186 SoC - 8186PV
		"8186-RESERVED1",
		"8186-RESERVED2",
		"865x-8651",		//865x Soc - 8651
		"865x-865xC",		//865x Soc - 865xC
		"865x-RESERVED1",
		"865x-RESERVED2",
		"867x-8671",		//867x Soc - 8671
		"867x-8672",		//867x Soc - 8672
		"867x-8676",		//867x Soc - 8676
		"867x-RESERVED1",
		"89xx-8972B_8982B",	//89xx Soc - 8972B or 8982B
		"89xx-89xxC",		//89xx Soc - 89xxC	
		"89xx-RESERVED1",	//89xx Soc	
		"89xx-RESERVED2",	//89xx Soc	
	};
	
	int n = 0;
	int i;
	uint8 *pch;

	if( off ) {	/* In our case, we write out all data at once. */
		*eof = 1;
		return 0;
	}
	
	// dump feature 	
	n += sprintf( buf + n, "gVoipFeature = " );
	
	for( pch = ( uint8 * )&gVoipFeature, i = 0; i < sizeof( gVoipFeature ); i ++ )
	{
		n += sprintf( buf + n, "%02X ", *pch ++ );
	}
	
	n += sprintf( buf + n, "\n" );
	
	// human readable info
	n += sprintf( buf + n, "Platform: %s (%X)\n", 
				platform_str[ ( uint32 )( ( gVoipFeature & RTK_VOIP_PLATFORM_MASK ) >> RTK_VOIP_PLATFORM_SHIFT ) ], 
				( uint32 )( ( gVoipFeature & RTK_VOIP_PLATFORM_MASK ) >> RTK_VOIP_PLATFORM_SHIFT ) );
	
	n += sprintf( buf + n, "One Arm Router: %u\n", !!( gVoipFeature & ONE_ARM_ROUTER_SUPPORT ) );
	// ------------- 32 bits boundary 
	
	n += sprintf( buf + n, "Channel:\n" );

	n += sprintf( buf + n, "\tIP phone: %u\n", RTK_VOIP_PHONE_NUM( gVoipFeature ) );
	n += sprintf( buf + n, "\tDECT: %u\n", RTK_VOIP_DECT_NUM( gVoipFeature ) );
	n += sprintf( buf + n, "\tSLIC: %u\n", RTK_VOIP_SLIC_NUM( gVoipFeature ) );
	n += sprintf( buf + n, "\tDAA:  %u (type=%u)\n", RTK_VOIP_DAA_NUM( gVoipFeature ), ( uint32 )( ( gVoipFeature & DAA_TYPE_MASK ) >> DAA_TYPE_SHIFT ) );
	
	n += sprintf( buf + n, "DSP:\n" );
	n += sprintf( buf + n, "\tIVR: %u\n", !!( gVoipFeature & IVR_SUPPORT ) );
	n += sprintf( buf + n, "\tMW: %s %s\n", 
			( RTK_VOIP_MW_CHK_IS_REALTEK( gVoipFeature ) ? "Realtek" : "" ), 
			( RTK_VOIP_MW_CHK_IS_AUDIOCODES( gVoipFeature ) ? "Audiocodes" : "" ) );
	
	n += sprintf( buf + n, "Arch:\n" );
	n += sprintf( buf + n, "\tIPC: %s\n", ( RTK_VOIP_CHECK_IS_IPC_ARCH( gVoipFeature ) ? "Yes" : "Standalone" ) );
	n += sprintf( buf + n, "\t\tType: %s\n", 
			( RTK_VOIP_CHECK_IS_IPC_ARCH( gVoipFeature ) ?
				( RTK_VOIP_CHECK_IS_IPC_ETHERNETDSP( gVoipFeature ) ? "Ethernet DSP" : "Coprocessor DSP" ) :
				"NA" ) );
	n += sprintf( buf + n, "\t\tRole: %s\n", 
			( RTK_VOIP_CHECK_IS_IPC_ARCH( gVoipFeature ) ?
				( RTK_VOIP_CHECK_IS_IPC_HOST( gVoipFeature ) ? "Host" : "DSP" ) :
				"NA" ) );
	n += sprintf( buf + n, "\tDSP Number: %u (IPC host only)\n", RTK_VOIP_DSP_DEVICE_NUMBER( gVoipFeature ) );
	
	n += sprintf( buf + n, "Codec:\n" );
	n += sprintf( buf + n, "\tG.711.1:	%u\n", !!( gVoipFeature & CODEC_G7111_SUPPORT ) );
	n += sprintf( buf + n, "\tG.722:	%u\n", !!( gVoipFeature & CODEC_G722_SUPPORT ) );
	n += sprintf( buf + n, "\tG.723:	%u\n", !!( gVoipFeature & CODEC_G723_SUPPORT ) );
	n += sprintf( buf + n, "\tG.726:	%u\n", !!( gVoipFeature & CODEC_G726_SUPPORT ) );
	n += sprintf( buf + n, "\tG.729:	%u\n", !!( gVoipFeature & CODEC_G729_SUPPORT ) );
	n += sprintf( buf + n, "\tGSM-FR:	%u\n", !!( gVoipFeature & CODEC_GSMFR_SUPPORT ) );
	n += sprintf( buf + n, "\tAMR:	%u\n", !!( gVoipFeature & CODEC_AMR_SUPPORT ) );
	n += sprintf( buf + n, "\tiLBC:	%u\n", !!( gVoipFeature & CODEC_iLBC_SUPPORT ) );
	n += sprintf( buf + n, "\tT.38:	%u\n", !!( gVoipFeature & CODEC_T38_SUPPORT ) );
	n += sprintf( buf + n, "\tSpeex-NB:	%u\n", !!( gVoipFeature & CODEC_SPEEX_NB_SUPPORT ) );
	
	*eof = 1;
	return n;
}

static int __init voip_feature_proc_init( void )
{
	create_proc_read_entry( PROC_VOIP_DIR "/feature", 0, NULL, voip_feature_read_proc, NULL );
	
	return 0;
}

static void __exit voip_feature_proc_exit( void )
{
	remove_proc_entry( PROC_VOIP_DIR "/feature", NULL );
}

voip_initcall_proc( voip_feature_proc_init );
voip_exitcall( voip_feature_proc_exit );

