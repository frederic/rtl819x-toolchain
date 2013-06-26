/* This function is included in ivr_speech.c */

/*
 * Instruction to add a speech: 
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * 1. Generate a speech wave file
 * 2. Add its file name text_xxx to make_ivr_speech, and make
 *    a new ivr_speech.c
 * 3. Add ID:
 *    - text_xxx, in ivr_speech_wave_info[] of this file
 *    - IVR_TEXT_ID_xxx, in voip_params.h 
 * 4. Add ID conversion statement in GetWaveIndexFromTextChar().
 */

#include "voip_debug.h"
//#define CT_ASSERT( expr )		extern int __ct_assert[ 2 * ( expr ) - 1 ]

#define _M_IVR_SPEECH_WAVE_INFO( x )	{ sizeof( ivr_wave_ ##x ), ( unsigned char * )ivr_wave_ ##x }
const ivr_speech_wave_info_t ivr_speech_wave_info[] = {
	_M_IVR_SPEECH_WAVE_INFO( number_0 ),
	_M_IVR_SPEECH_WAVE_INFO( number_1 ),
	_M_IVR_SPEECH_WAVE_INFO( number_2 ),
	_M_IVR_SPEECH_WAVE_INFO( number_3 ),
	_M_IVR_SPEECH_WAVE_INFO( number_4 ),
	_M_IVR_SPEECH_WAVE_INFO( number_5 ),
	_M_IVR_SPEECH_WAVE_INFO( number_6 ),
	_M_IVR_SPEECH_WAVE_INFO( number_7 ),
	_M_IVR_SPEECH_WAVE_INFO( number_8 ),
	_M_IVR_SPEECH_WAVE_INFO( number_9 ),
	_M_IVR_SPEECH_WAVE_INFO( number_dot ),
	_M_IVR_SPEECH_WAVE_INFO( alphabet_a ),
	_M_IVR_SPEECH_WAVE_INFO( alphabet_b ),
	_M_IVR_SPEECH_WAVE_INFO( alphabet_c ),
	_M_IVR_SPEECH_WAVE_INFO( alphabet_d ),
	_M_IVR_SPEECH_WAVE_INFO( alphabet_e ),
	_M_IVR_SPEECH_WAVE_INFO( alphabet_f ),
	_M_IVR_SPEECH_WAVE_INFO( alphabet_g ),
	_M_IVR_SPEECH_WAVE_INFO( alphabet_h ),
	_M_IVR_SPEECH_WAVE_INFO( alphabet_i ),
	_M_IVR_SPEECH_WAVE_INFO( alphabet_j ),
	_M_IVR_SPEECH_WAVE_INFO( alphabet_k ),
	_M_IVR_SPEECH_WAVE_INFO( alphabet_l ),
	_M_IVR_SPEECH_WAVE_INFO( alphabet_m ),
	_M_IVR_SPEECH_WAVE_INFO( alphabet_n ),
	_M_IVR_SPEECH_WAVE_INFO( alphabet_o ),
	_M_IVR_SPEECH_WAVE_INFO( alphabet_p ),
	_M_IVR_SPEECH_WAVE_INFO( alphabet_q ),
	_M_IVR_SPEECH_WAVE_INFO( alphabet_r ),
	_M_IVR_SPEECH_WAVE_INFO( alphabet_s ),
	_M_IVR_SPEECH_WAVE_INFO( alphabet_t ),
	_M_IVR_SPEECH_WAVE_INFO( alphabet_u ),
	_M_IVR_SPEECH_WAVE_INFO( alphabet_v ),
	_M_IVR_SPEECH_WAVE_INFO( alphabet_w ),
	_M_IVR_SPEECH_WAVE_INFO( alphabet_x ),
	_M_IVR_SPEECH_WAVE_INFO( alphabet_y ),
	_M_IVR_SPEECH_WAVE_INFO( alphabet_z ),
	_M_IVR_SPEECH_WAVE_INFO( text_DHCP ),
	_M_IVR_SPEECH_WAVE_INFO( text_fixIP ),
	_M_IVR_SPEECH_WAVE_INFO( text_no_resource ),
	_M_IVR_SPEECH_WAVE_INFO( text_please_enter_number ),
	_M_IVR_SPEECH_WAVE_INFO( text_please_enter_password ),
	///<<&&ID1&&>>	/* DON'T remove this line, it helps wizard to generate ID. */
	//_M_IVR_SPEECH_WAVE_INFO( text_xxx ),
};
#undef _M_IVR_SPEECH_WAVE_INFO

#define SIZE_OF_IVR_SPEECH_INFO		( sizeof( ivr_speech_wave_info ) / sizeof( ivr_speech_wave_info[ 0 ] ) )

/*
 * If NUM_OF_IVR_SPEECH_WAVE is different from SIZE_OF_IVR_SPEECH_INFO,
 * some entries are missing.
 */
CT_ASSERT( NUM_OF_IVR_SPEECH_WAVE == SIZE_OF_IVR_SPEECH_INFO );

unsigned int GetWaveIndexFromTextChar( unsigned char ch )
{
	if( ch >= '0' && ch <= '9' ) {			/* number */
		return ( ch - '0' + IVR_SPEECH_ID_NUM_0 );
	} else if( ch >= 'a' && ch <= 'z' ) {	/* lower case alphabet */
		return ( ch - 'a' + IVR_SPEECH_ID_ALPHA_A );
	} else if( ch >= 'A' && ch <= 'Z' ) {	/* upper case alphabet */
		return ( ch - 'A' + IVR_SPEECH_ID_ALPHA_A );
	} else if( ch == '.' ) {				/* dot */
		return IVR_SPEECH_ID_DOT;
	} else {
		switch( ch ) {
		case IVR_TEXT_ID_DHCP:
			return IVR_SPEECH_ID_TXT_DHCP;
			break;
		case IVR_TEXT_ID_FIX_IP:
			return IVR_SPEECH_ID_TXT_FIX_IP;
			break;
		case IVR_TEXT_ID_NO_RESOURCE:
			return IVR_SPEECH_ID_TXT_NO_RESOURCE;
			break;
		case IVR_TEXT_ID_PLZ_ENTER_NUMBER:
			return IVR_SPEECH_ID_TXT_PLZ_ENTER_NUMBER;
			break;
		case IVR_TEXT_ID_PLEASE_ENTER_PASSWORD:
			return IVR_SPEECH_ID_TXT_PLEASE_ENTER_PASSWORD;
			break;
		///<<&&ID2&&>>	/* DON'T remove this line, it helps wizard to generate ID. */
		//case IVR_TEXT_ID_xxx:
		//	return IVR_SPEECH_ID_xxx;
		//	break;
		}
	}
	
	// TODO: unexpected text, how to do?
	return NUM_OF_IVR_SPEECH_WAVE - 1;
}

