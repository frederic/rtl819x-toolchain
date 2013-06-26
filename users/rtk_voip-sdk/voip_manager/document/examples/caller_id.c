#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "voip_manager.h"

void ShowUsage(char *cmd)
{
	printf(
		"usage: %s <mode=0> <caller_id> <DTMF mode> <DTMF duration> <DTMF intergigit pause duration> \n" \
		"       %s <mode=1> <caller_id> <FSK area>  <FSK_type> \n" \
		"       %s <mode=2> [test_set]\n" \
		"  - mode => 0 is DTMF, 1 is FSK, 2 is DTMF predefined verfication\n" \
		"  - caller_id => caller id string\n" \
		"  - FSK area[2:0] => 0 BELLCORE, 1: ETSI, 2: BT, 3: NTT\n" \
		"  - FSK area[bit8]=> Auto Ring, 0: disable, 1: enable\n" \
		"  - FSK area[bit7]=> FSK date & time sync\n" \
		"  - FSK area[bit6]=> reverse polarity before caller id (For FSK)\n" \
		"  - FSK area[bit5]=> short ring before caller id (For FSK)\n" \
		"  - FSK area[bit4]=> dual alert tone before caller id (For FSK)\n" \
		"  - FSK area[bit3]=> caller id Prior Ring (FSK & DTMF)\n" \
		"  - FSK type => 1 is type-I, 2 is type-II\n"\
		"  - DTMF mode[1:0]=> Start digit, 0:A, 1:B, 2:C, 3:D\n" \
		"  - DTMF mode[3:2]=> End digit, 0:A, 1:B, 2:C, 3:D\n"	\
		"  - DTMF mode[4]=> Auto start/end digit send, 0:suto mode 1:non-auto\n" \
		"	(non-auto mode: DSP send caller ID string only. If caller ID need start/end digits, developer should add them to caller ID strings.)\n" \
		"  - DTMF mode[5]=> Auto Ring, 0: disable, 1: enable\n"	\
 		"  - DTMF mode[6]=> Before 1st Ring, 0: after 1st Ring, 1: before 1st Ring.\n"
		"  - DTMF mode[7]==> Auto SLIC action, 0: Disable, - 1: Enable.\n"
		"                    SLIC action, such as SLIC hook statue check, line polarity change (not include SLIC Ringing)\n"
 		"  - test_set[0] ==> test items are auto ring, and auto SLIC action\n"
 		"  - test_set[1] ==> test items are NO auto ring, and NO auto SLIC action\n"
		, cmd, cmd, cmd);
	exit(0);
}

static void CID_DTMF_predefined_verification( unsigned int test_set );

int main(int argc, char *argv[])
{
	unsigned int mode, dtmf_mode, digit_on, digit_pause, fsk_type, fsk_area;
	unsigned int test_set = 0xFFFFFF;

	if (argc < 2)
	{
		ShowUsage(argv[0]);
	}
	
	mode = atoi(argv[1]);
	switch (mode)
	{
	case 0:
		if( argc < 6 )
			goto label_show_usage;
			
		dtmf_mode = atoi(argv[3]);
		digit_on = atoi(argv[4]);
		digit_pause = atoi(argv[5]);
		
		// first ring by AP 
		if( ( dtmf_mode & 0x20 ) == 0 && ( dtmf_mode & 0x40 ) == 0 ) {
			// not auto ring, and DTMF after ring 
			rtk_SetRingFXS( 0, 1 );
			
			// wait ring complete 
			usleep( 2 * 1000 * 1000 );	// simple implement: delay 2 seconds 
			
			rtk_SetRingFXS( 0, 0 );
		}
		
		rtk_Set_CID_DTMF_MODE(0, dtmf_mode, digit_on, digit_pause, 300, 300); //pre_silence = 300ms, end_silence = 300ms
		rtk_Gen_Dtmf_CID(0, argv[2]);
		
		// ring by AP
		if( ( dtmf_mode & 0x20 ) == 0 ) {
			rtk_SetRingFXS( 0, 1 );
		}

		break;
	case 1:
		if (argc == 5)
		{
			fsk_area = atoi(argv[3]);
			
			//rtk_Set_CID_FSK_GEN_MODE(0, 1);
			
			if ((fsk_area&7) > CID_DTMF)
				printf("wrong FSK area => 0 BELLCORE, 1: ETSI, 2: BT, 3: NTT\n");
			else if(atoi(argv[4])!= 1 && atoi(argv[4])!= 2)
				printf("wrong fsk type: should be type-I(1) or type-II(2)\n");
			else
			{	
				fsk_type = atoi(argv[4])- 1 ;
				rtk_Set_FSK_Area(0, fsk_area);
				rtk_Gen_FSK_CID(0, argv[2], (void *) 0, (void *) 0, fsk_type/*FSK Type*/); // 
			}
		} else
			goto label_show_usage;
		break;
	case 2:
		if( argc >= 3 )
			test_set = atoi(argv[2]);
		CID_DTMF_predefined_verification( test_set );
		break;
	default:
label_show_usage:
		ShowUsage(argv[0]);
	}
	return 0;
}

//
// DTMF caller ID predefined test set (mode=2)
//
typedef enum {
	DTMF_DIGIT_A,
	DTMF_DIGIT_B,
	DTMF_DIGIT_C,
	DTMF_DIGIT_D,
} DTMF_DIGIT_t;

typedef struct {
	struct {
		unsigned int bAutoDigit:	1;
		DTMF_DIGIT_t nStartDigit:	2;
		DTMF_DIGIT_t nEndDigit:		2;
		
		unsigned int bAutoRing:			1;
		unsigned int bBeforeFirstRing:	1;
		
		unsigned int bAutoSlicAction:	1;
	} mode;
	
	struct {
		uint32 on;		// in unit of ms (suggest: 80ms)
		uint32 pause;	// in unit of ms (suggest: 80ms)
	} duration;
	
	struct {
		uint32 pre;		// in unit of ms (suggest: 300ms)
		uint32 end;		// in unit of ms (suggest: 300ms)
	} silence;
	
	struct {
		char string[ 64 ];
	} id;
} CID_DTMF_verify_t;

static void Run_CID_DTMF_verification( const CID_DTMF_verify_t *pCIDDTMF )
{
	unsigned int dtmf_mode;
	VoipEventID dsp_event;
	char log_cmd[ 256 ];
	static int log_seq = 0;
	static int random;
	SLICEVENT slic_event;
	const uint32 chid = 0;
	
	if( log_seq == 0 )
		random = ( int )time( NULL );

	// parameters for DSP 
	dtmf_mode = 0;
	dtmf_mode |= ( !pCIDDTMF ->mode.bAutoDigit ? 0x10 : 
						0x00 | ( pCIDDTMF ->mode.nStartDigit ) | 
								( pCIDDTMF ->mode.nEndDigit << 2 ) );
	dtmf_mode |= ( !pCIDDTMF ->mode.bAutoRing ? 0x00 :
						0x20 | ( pCIDDTMF ->mode.bBeforeFirstRing << 6 ) );
	dtmf_mode |= ( !pCIDDTMF ->mode.bAutoSlicAction ? 0x00 : 0x80 );
	
	// Detail content 
	printf( "Test setting:\n" );
	printf( "\tAuto digit: %s", pCIDDTMF ->mode.bAutoDigit ? "Yes" : "No" );
	if( pCIDDTMF ->mode.bAutoDigit ) {
		printf( "\t(Start=%c End=%c)\n", 'A' + pCIDDTMF ->mode.nStartDigit,
										'A' + pCIDDTMF ->mode.nEndDigit );
	} else
		printf( "\n" );
	
	printf( "\tAuto ring: %s, DTMF '%s' first ring\n", 
						pCIDDTMF ->mode.bAutoRing ? "Yes" : "No",
						pCIDDTMF ->mode.bBeforeFirstRing ? "before" : "after" );
	
	printf( "\tAuto SLIC action: %s\n", 
						pCIDDTMF ->mode.bAutoSlicAction ? "Yes" : "No" );
	
	printf( "\tDuration: on=%u pause=%u (ms)\n", 
							pCIDDTMF ->duration.on, pCIDDTMF ->duration.pause );
	
	printf( "\tSilence: pre=%u end=%u (ms)\n",
							pCIDDTMF ->silence.pre, pCIDDTMF ->silence.end );
	
	printf( "\tCaller ID: string=%s\n", pCIDDTMF ->id.string );
	
	printf( "\tdtmf_mode: %04X\n", dtmf_mode );
	
	fflush( stdout );
	
	printf( "Press any key to start...\n" );
	
	getchar();
	
	// start log 
	sprintf( log_cmd, "cat /dev/voip/pcmtx0 > "
				"pcmtx0_%02d_%04X-str%s-digit%d-%d-%d_"
				"ring%d-before%d_"
				"slic%d_"
				"silence%d-%d_"
				"duration%d-%d &", 
						log_seq ++, random & 0xFFFF, 
						pCIDDTMF ->id.string,
						pCIDDTMF ->mode.bAutoDigit, 
						pCIDDTMF ->mode.nStartDigit,
						pCIDDTMF ->mode.nEndDigit,
						
						pCIDDTMF ->mode.bAutoRing, pCIDDTMF ->mode.bBeforeFirstRing,
						
						pCIDDTMF ->mode.bAutoSlicAction,
						
						pCIDDTMF ->silence.pre, pCIDDTMF ->silence.end,
						
						pCIDDTMF ->duration.on, pCIDDTMF ->duration.pause
						);
	printf( "start log - %s\n", log_cmd );
	system( log_cmd );
	
	// flush all events fifo 
	rtk_Set_flush_fifo( chid );
	
	// part 1: [ring] by AP (optional)
	if( !pCIDDTMF ->mode.bAutoRing && !pCIDDTMF ->mode.bBeforeFirstRing ) {
		// not auto ring && DTMF not before first ring 
		// --> ring by AP && DTMF after first ring 
		rtk_SetRingFXS( chid, 1 );
		printf( "Turn on SLIC ring by AP\n" );
		
		// wait ring complete 
		usleep( 2 * 1000 * 1000 );	// simple implement: delay 2 seconds 
		
		rtk_SetRingFXS( chid, 0 );
		printf( "Turn off SLIC ring by AP\n" );
	}
	
	// part 2: silence pre + DTMF start + DTMF ID + DTMF end + silence end
	rtk_Set_CID_DTMF_MODE(chid, dtmf_mode, 
							pCIDDTMF ->duration.on, pCIDDTMF ->duration.pause, 
							pCIDDTMF ->silence.pre, pCIDDTMF ->silence.end ); //pre_silence = 300ms, end_silence = 300ms
	
	if( pCIDDTMF ->mode.bAutoSlicAction )
		rtk_Gen_Dtmf_CID( chid, pCIDDTMF ->id.string );
	else {
		rtk_enablePCM( chid, 1 ); // enable PCM before generating dtmf caller id
		
		// set cid CID String 
		rtk_Set_Dtmf_CID_String( chid, pCIDDTMF ->id.string );	
	}
	
	// wait for DTMF complete, or off-hook event 
	printf( "Wait for DTMF caller ID complete, or off-hook to break..." );
	fflush( stdout );
	
	while( 1 ) {
		// DTMF complete? 
		rtk_GetDspEvent( chid, 0, &dsp_event );
		
		if( dsp_event == VEID_DSP_DTMF_CLID_END ) {
			printf( "complete!!\n" );
			break;
		}

		// off-hook event to stop caller ID 
		if( !pCIDDTMF ->mode.bAutoSlicAction ) {
			rtk_GetSlicEvent( chid, &slic_event );
			
			if( slic_event == SLICEVENT_OFFHOOK ) {
				rtk_Stop_CID( chid, 2 );
				printf( "off-hook!!\n" );
				break;	// stop CID!! we can't recv complete event 
			}
		}
		
		usleep( 1 );	// light busy loop 
	}
	
	if( !pCIDDTMF ->mode.bAutoSlicAction ) {
		rtk_enablePCM( chid, 0 ); // disable PCM after generating dtmf caller id	
	}
	
	// part 3: ring by AP (optional)
	if( !pCIDDTMF ->mode.bAutoRing ) {
		rtk_SetRingFXS( chid, 1 );
		printf( "Turn on SLIC ring by AP\n" );
	}
	
	// stop log
	printf( "stop log\n" );
	system( "killall cat" );
	
	// wait moment 
	printf( "Wait 5 seconds, and then turn off ring...\n" );
	sleep( 5 );
	
	rtk_SetRingFXS( chid, 0 );
	printf( "Turn off SLIC ring by AP.... complete test item\n------\n" );
}

static void CID_DTMF_predefined_verification( unsigned int test_set )
{
	CID_DTMF_verify_t CID_DTMF;
	//const char *pstr;
	
#define M_AUTO_DIGIT( on, start, end )		CID_DTMF.mode.bAutoDigit = on;	\
											CID_DTMF.mode.nStartDigit = start;	\
											CID_DTMF.mode.nEndDigit = end;
#define M_AUTO_RING( on, before )			CID_DTMF.mode.bAutoRing = on;	\
											CID_DTMF.mode.bBeforeFirstRing = before;
#define M_AUTO_SLIC_ACT( on )				CID_DTMF.mode.bAutoSlicAction = on;
#define M_DURATION( on1, pause1 )			CID_DTMF.duration.on = on1;	\
											CID_DTMF.duration.pause = pause1;
#define M_SILENCE( pre1, end1 )				CID_DTMF.silence.pre = pre1;	\
											CID_DTMF.silence.end = end1;
#define M_ID_STRING( str )					strcpy( CID_DTMF.id.string, str );
	
	/*
	 * DTMF caller ID verfication 
	 * --------------------------
	 *
	 * format of DTMF caller ID: 
	 *   [ring] + silence pre + DTMF start + DTMF ID + DTMF end + silence end + ring 
	 *                          (      duration on / off       )
	 * 
	 * 
	 */
	
	system( "killall solar_monitor" );
	system( "killall solar" );
	
	printf( "CID DTMF predefined verfication\n" );
	printf( "Test set are:\n" );
	if( test_set & 0x01 )
		printf( "\t(A: Auto ring with long digits)\n" );
	if( test_set & 0x02 )
		printf( "\t(B: AP ring with long digits)\n" );	
	
#if 1	// Auto ring 
	if( ( test_set & 0x01 ) == 0 )
		goto label_auto_ring_test_set_done;
	
	// A0. ring + 'A' + '886852963741' + 'C' + ring 
	M_AUTO_DIGIT( 1, DTMF_DIGIT_A, DTMF_DIGIT_C );
	M_AUTO_RING( 1, 0 );
	M_AUTO_SLIC_ACT( 1 );
	M_DURATION( 80, 80 );
	M_SILENCE( 300, 300 );
	M_ID_STRING( "886852963741" );
	Run_CID_DTMF_verification( &CID_DTMF );
	
	// A1. 'A' + '886852963741' + 'C' + ring 
	M_AUTO_DIGIT( 1, DTMF_DIGIT_A, DTMF_DIGIT_C );
	M_AUTO_RING( 1, 1 );
	M_AUTO_SLIC_ACT( 1 );
	M_DURATION( 80, 80 );
	M_SILENCE( 300, 300 );
	M_ID_STRING( "886852963741" );
	Run_CID_DTMF_verification( &CID_DTMF );
	
	// A2. ring + 'A886852963741C' + ring 
	M_AUTO_DIGIT( 0, DTMF_DIGIT_A, DTMF_DIGIT_C );
	M_AUTO_RING( 1, 0 );
	M_AUTO_SLIC_ACT( 1 );
	M_DURATION( 80, 80 );
	M_SILENCE( 300, 300 );
	M_ID_STRING( "A886852963741C" );
	Run_CID_DTMF_verification( &CID_DTMF );
	
	// A3. ring + '886852963741' + ring 
	M_AUTO_DIGIT( 0, DTMF_DIGIT_A, DTMF_DIGIT_C );
	M_AUTO_RING( 1, 0 );
	M_AUTO_SLIC_ACT( 1 );
	M_DURATION( 80, 80 );
	M_SILENCE( 300, 300 );
	M_ID_STRING( "886852963741" );
	Run_CID_DTMF_verification( &CID_DTMF );
	
	// A4. ring + 'A' + '886852963741' + 'C' + ring 
	// cmp with A0 - duration ( 150, 60 )
	M_AUTO_DIGIT( 1, DTMF_DIGIT_A, DTMF_DIGIT_C );
	M_AUTO_RING( 1, 0 );
	M_AUTO_SLIC_ACT( 1 );
	M_DURATION( 150, 60 );
	M_SILENCE( 300, 300 );
	M_ID_STRING( "886852963741" );
	Run_CID_DTMF_verification( &CID_DTMF );	

	// A5. ring + 'A' + '886852963741' + 'C' + ring 
	// cmp with A0 - silence ( 500, 200 )
	M_AUTO_DIGIT( 1, DTMF_DIGIT_A, DTMF_DIGIT_C );
	M_AUTO_RING( 1, 0 );
	M_AUTO_SLIC_ACT( 1 );
	M_DURATION( 80, 80 );
	M_SILENCE( 500, 200 );
	M_ID_STRING( "886852963741" );
	Run_CID_DTMF_verification( &CID_DTMF );	
		
label_auto_ring_test_set_done:
	;
#endif

#if 1	// AP ring 
	if( ( test_set & 0x02 ) == 0 )
		goto label_AP_ring_test_set_done;
	
	// B0. ring + 'A' + '0088635780211' + 'C' + ring 
	M_AUTO_DIGIT( 1, DTMF_DIGIT_A, DTMF_DIGIT_C );
	M_AUTO_RING( 0, 0 );
	M_AUTO_SLIC_ACT( 0 );
	M_DURATION( 80, 80 );
	M_SILENCE( 300, 300 );
	M_ID_STRING( "0088635780211" );
	Run_CID_DTMF_verification( &CID_DTMF );
	
	// B1. 'A' + '0088635780211' + 'C' + ring 
	M_AUTO_DIGIT( 1, DTMF_DIGIT_A, DTMF_DIGIT_C );
	M_AUTO_RING( 0, 1 );
	M_AUTO_SLIC_ACT( 0 );
	M_DURATION( 80, 80 );
	M_SILENCE( 300, 300 );
	M_ID_STRING( "0088635780211" );
	Run_CID_DTMF_verification( &CID_DTMF );
	
	// B2. ring + 'A0088635780211C' + ring 
	M_AUTO_DIGIT( 0, DTMF_DIGIT_A, DTMF_DIGIT_C );
	M_AUTO_RING( 0, 0 );
	M_AUTO_SLIC_ACT( 0 );
	M_DURATION( 80, 80 );
	M_SILENCE( 300, 300 );
	M_ID_STRING( "A0088635780211C" );
	Run_CID_DTMF_verification( &CID_DTMF );

	// B3. ring + '0088635780211' + ring 
	M_AUTO_DIGIT( 0, DTMF_DIGIT_A, DTMF_DIGIT_C );
	M_AUTO_RING( 0, 0 );
	M_AUTO_SLIC_ACT( 0 );
	M_DURATION( 80, 80 );
	M_SILENCE( 300, 300 );
	M_ID_STRING( "0088635780211" );
	Run_CID_DTMF_verification( &CID_DTMF );

	// B4. ring + 'A' + '0088635780211' + 'C' + ring 
	// cmp with B0 - duration ( 150, 60 )
	M_AUTO_DIGIT( 1, DTMF_DIGIT_A, DTMF_DIGIT_C );
	M_AUTO_RING( 0, 0 );
	M_AUTO_SLIC_ACT( 0 );
	M_DURATION( 150, 60 );
	M_SILENCE( 300, 300 );
	M_ID_STRING( "0088635780211" );
	Run_CID_DTMF_verification( &CID_DTMF );
	
	// B5. ring + 'A' + '0088635780211' + 'C' + ring 
	// cmp with B0 - silence ( 500, 200 )
	M_AUTO_DIGIT( 1, DTMF_DIGIT_A, DTMF_DIGIT_C );
	M_AUTO_RING( 0, 0 );
	M_AUTO_SLIC_ACT( 0 );
	M_DURATION( 80, 80 );
	M_SILENCE( 500, 200 );
	M_ID_STRING( "0088635780211" );
	Run_CID_DTMF_verification( &CID_DTMF );
	
label_AP_ring_test_set_done:
	;
#endif	
}


