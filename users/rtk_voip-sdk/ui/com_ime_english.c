#include <stdio.h>
#include "ui_config.h"
#include "ui_vkey.h"
#include "com_ime_def.h"
#include "com_ime_english.h"


/*
 * IME_ID_ENGLISH  
 */
static const unsigned char cyclic_keys_for_key0[] = {
	'0', ' ',
};
static const unsigned char cyclic_keys_for_key1[] = {
	'1', '@', '-', '_', '%', '$', '&', 
};
static const unsigned char cyclic_keys_for_key2[] = {
	'A', 'B', 'C', 'a', 'b', 'c', '2',
};
static const unsigned char cyclic_keys_for_key3[] = {
	'D', 'E', 'F', 'd', 'e', 'f', '3',
};
static const unsigned char cyclic_keys_for_key4[] = {
	'G', 'H', 'I', 'g', 'h', 'i', '4',
};
static const unsigned char cyclic_keys_for_key5[] = {
	'J', 'K', 'L', 'j', 'k', 'l', '5',
};
static const unsigned char cyclic_keys_for_key6[] = {
	'M', 'N', 'O', 'm', 'n', 'o', '6',
};
static const unsigned char cyclic_keys_for_key7[] = {
	'P', 'Q', 'R', 'S', 'p', 'q', 'r', 's', '7',
};
static const unsigned char cyclic_keys_for_key8[] = {
	'T', 'U', 'V', 't', 'u', 'v', '8',
};
static const unsigned char cyclic_keys_for_key9[] = {
	'W', 'X', 'Y', 'Z', 'w', 'x', 'y', 'z', '9',
};
static const unsigned char cyclic_keys_for_key_star[] = {
	'*',
};
static const unsigned char cyclic_keys_for_key_pound[] = {
	'#',
};

#define M_CYCLIC_KEYS_SET( k )		{ k, sizeof( k ) / sizeof( k[ 0 ] ), }

static const cyclic_keys_set_t englishCyclicKeysSet[] = {
	M_CYCLIC_KEYS_SET( cyclic_keys_for_key0 ),
	M_CYCLIC_KEYS_SET( cyclic_keys_for_key1 ),
	M_CYCLIC_KEYS_SET( cyclic_keys_for_key2 ),
	M_CYCLIC_KEYS_SET( cyclic_keys_for_key3 ),
	M_CYCLIC_KEYS_SET( cyclic_keys_for_key4 ),
	M_CYCLIC_KEYS_SET( cyclic_keys_for_key5 ),
	M_CYCLIC_KEYS_SET( cyclic_keys_for_key6 ),
	M_CYCLIC_KEYS_SET( cyclic_keys_for_key7 ),
	M_CYCLIC_KEYS_SET( cyclic_keys_for_key8 ),
	M_CYCLIC_KEYS_SET( cyclic_keys_for_key9 ),
	M_CYCLIC_KEYS_SET( cyclic_keys_for_key_star ),
	M_CYCLIC_KEYS_SET( cyclic_keys_for_key_pound ),
};

#undef M_CYCLIC_KEYS_SET

void EnglishImeInitialization( void )
{
	pEnglishImeVars ->preStrokeKey = 0;
	pEnglishImeVars ->preEditorCmd = EDITOR_CMD_NORMAL;

	/* don't initialize follows can also work correctly. */
	pEnglishImeVars ->preStrokeKeyTime = 0;
	pEnglishImeVars ->pCyclicKeysSet = NULL;
	pEnglishImeVars ->nthKeyInCyclic = 0;
}

void EnglishImeTermination( void )
{
	if( pEnglishImeVars ->preEditorCmd == EDITOR_CMD_CANDIDATE ||
		pEnglishImeVars ->preEditorCmd == EDITOR_CMD_REPLACE )
	{
		pEnglishImeVars ->preEditorCmd = EDITOR_CMD_ACCEPT;

		SendTextToTextEditor( 0 /* key (ignore) */, EDITOR_CMD_ACCEPT );
	}
}

int GetEnglishImeCyclicKeyIndex( unsigned char key )
{
	if( key >= VKEY_0 && key <= VKEY_9 )
		return key - VKEY_0;
	else if( key == VKEY_STAR )
		return 10;
	else if( key == VKEY_POUND )
		return 11;

	return -1;		/* not included in cyclic key */
}

int  EnglishImeKeyProcessor( unsigned char key )
{
	int key_index;

	if( pEnglishImeVars ->preEditorCmd == EDITOR_CMD_CANDIDATE ||
		pEnglishImeVars ->preEditorCmd == EDITOR_CMD_REPLACE )
	{
		if( pEnglishImeVars ->preStrokeKey == key ) {
			/* cyclic key */

			/* next one */
			pEnglishImeVars ->nthKeyInCyclic ++;
			if( pEnglishImeVars ->nthKeyInCyclic >= pEnglishImeVars ->pCyclicKeysSet ->nNumberOfKeys )
				pEnglishImeVars ->nthKeyInCyclic = 0;

			/* tell editor to replace */
			pEnglishImeVars ->preEditorCmd = EDITOR_CMD_REPLACE;

			SendTextToTextEditor( 
				pEnglishImeVars ->pCyclicKeysSet ->pCyclicKeys[ pEnglishImeVars ->nthKeyInCyclic ],
				EDITOR_CMD_REPLACE );

			pEnglishImeVars ->preStrokeKeyTime = GetUptimeInMillisecond();

			goto laebl_english_ime_key_process_done;
		} else if( key == VKEY_BACKSPACE ) {
			/* cancel candidate */
			pEnglishImeVars ->preEditorCmd = EDITOR_CMD_CANCEL;

			SendTextToTextEditor( 0 /* key (ignore) */, EDITOR_CMD_CANCEL );

			goto laebl_english_ime_key_process_done;
		} else {
			/* accept previous + new key */
			pEnglishImeVars ->preEditorCmd = EDITOR_CMD_ACCEPT;

			SendTextToTextEditor( 0 /* key (ignore) */, EDITOR_CMD_ACCEPT );

			goto label_english_ime_key_process_new_key;
		}
	} else {
		/* new key */
label_english_ime_key_process_new_key:

		key_index = GetEnglishImeCyclicKeyIndex( key );

		/* Not a cyclic key */
		if( key_index == -1 ) {

			// TODO: is a function key?
			goto label_english_ime_key_process_not_my_key;
		}
		
		/* Save current key. */
		pEnglishImeVars ->preStrokeKey = key;
		pEnglishImeVars ->preStrokeKeyTime = GetUptimeInMillisecond();

		pEnglishImeVars ->nthKeyInCyclic = 0;
		pEnglishImeVars ->pCyclicKeysSet = &englishCyclicKeysSet[ key_index ];

		if( englishCyclicKeysSet[ key_index ].nNumberOfKeys == 1 ) {

			pEnglishImeVars ->preEditorCmd = EDITOR_CMD_NORMAL;

			SendTextToTextEditor( englishCyclicKeysSet[ key_index ].pCyclicKeys[ 0 ], 
									EDITOR_CMD_NORMAL );
		} else {

			pEnglishImeVars ->preEditorCmd = EDITOR_CMD_CANDIDATE;

			SendTextToTextEditor( 
				pEnglishImeVars ->pCyclicKeysSet ->pCyclicKeys[ pEnglishImeVars ->nthKeyInCyclic ]/* key */, 
				EDITOR_CMD_CANDIDATE );
		}
	}

laebl_english_ime_key_process_done:
	return 1;

label_english_ime_key_process_not_my_key:
	return 0;	/* not my key */
}

void EnglishImeTimerProcessor( void )
{
	if( pEnglishImeVars ->preEditorCmd == EDITOR_CMD_CANDIDATE ||
		pEnglishImeVars ->preEditorCmd == EDITOR_CMD_REPLACE )
	{
		if( CheckIfTimeoutInMillisecond( &pEnglishImeVars ->preStrokeKeyTime, 
										 ON_SPOT_EDITING_ACCEPT_TIME ) == 0 )
		{
			pEnglishImeVars ->preEditorCmd = EDITOR_CMD_ACCEPT;

			SendTextToTextEditor( 0 /* key (ignore) */, EDITOR_CMD_ACCEPT );
		}
	}
}
