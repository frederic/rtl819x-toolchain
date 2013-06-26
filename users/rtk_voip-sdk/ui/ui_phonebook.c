#include <stdio.h>
#include <string.h>
#include "ui_config.h"
#include "ui_host.h"
#include "ui_softkey.h"
#include "ui_phonebook.h"
#include "gs_lib.h"
#include "flash_rw_api.h"

#define PHONEBOOK_FIELD_OFFSET( type, field )		( ( unsigned long )MY_FIELD_OFFSET( ui_falsh_layout_t, phonebook ) +	\
													  ( unsigned long )MY_FIELD_OFFSET( type, field ) )
#define PHONEBOOK_RECORD_NAME_OFFSET( n )			( ( unsigned long )PHONEBOOK_FIELD_OFFSET( phonebook_info_t, records ) +	\
													  ( unsigned long )sizeof( phonebook_record_t ) * n +						\
													  ( unsigned long )MY_FIELD_OFFSET( phonebook_record_t, szName ) )
#define PHONEBOOK_RECORD_NUMBER_OFFSET( n )			( ( unsigned long )PHONEBOOK_FIELD_OFFSET( phonebook_info_t, records ) +	\
													  ( unsigned long )sizeof( phonebook_record_t ) * n +						\
													  ( unsigned long )MY_FIELD_OFFSET( phonebook_record_t, bcdNumber ) )
#define PHONEBOOK_RECORD_OFFSET( n )				( ( unsigned long )PHONEBOOK_FIELD_OFFSET( phonebook_info_t, records ) +	\
													  ( unsigned long )sizeof( phonebook_record_t ) * n )

/* ****************************************************************************
 * Phonebook structure 
 * **************************************************************************** */
static phonebook_record_t phonebookEditBuffer;		/* buffer for current editing */
static unsigned int viPhonebookEdit;				/* visual index to store current editing */

static unsigned int riFromViPhonebookRecordSort[ NUM_OF_PHONEBOOK_RECORD ];

static unsigned int nNumberOfPhonebookRecords;

/* ****************************************************************************
 * Phonebook assistant functions
 * **************************************************************************** */
void PhonebookBasicInitialization( int bClearContent )
{
	unsigned int i;

	if( bClearContent ) {

		nNumberOfPhonebookRecords = 0;

		for( i = 0; i < NUM_OF_PHONEBOOK_RECORD; i ++ )
			riFromViPhonebookRecordSort[ i ] = i;

		FlashWriteData( PHONEBOOK_FIELD_OFFSET( phonebook_info_t, nNumberOfPhonebookRecord ), 
					    &nNumberOfPhonebookRecords, sizeof( nNumberOfPhonebookRecords ) ); 

		FlashWriteData( PHONEBOOK_FIELD_OFFSET( phonebook_info_t, riFromViPhonebookRecordSort ), 
					    &riFromViPhonebookRecordSort, sizeof( riFromViPhonebookRecordSort ) ); 

		/* FlashValidateWriting() by caller */
	} else {

		FlashReadData( PHONEBOOK_FIELD_OFFSET( phonebook_info_t, nNumberOfPhonebookRecord ), 
					   &nNumberOfPhonebookRecords, sizeof( nNumberOfPhonebookRecords ) ); 

		FlashReadData( PHONEBOOK_FIELD_OFFSET( phonebook_info_t, riFromViPhonebookRecordSort ), 
					   &riFromViPhonebookRecordSort, sizeof( riFromViPhonebookRecordSort ) ); 
	}
}

static void GetPhonebookListItemTextFromVi( unsigned int viItem, unsigned char *pszItemText )
{
#if MAX_LEN_OF_PHONE_NAME > MAX_LENGTH_OF_ITEM_TEXT
	???	// truncate part of phone name
#else
	if( viItem >= NUM_OF_PHONEBOOK_RECORD )
		return;

	FlashReadData( PHONEBOOK_RECORD_NAME_OFFSET( riFromViPhonebookRecordSort[ viItem ] ), pszItemText, MAX_LEN_OF_PHONE_NAME + 1 );
#endif
}

static void ReadPhonebookRecordFromVi( unsigned int viItem, phonebook_record_t * pRecord )
{
	FlashReadData( PHONEBOOK_RECORD_OFFSET( riFromViPhonebookRecordSort[ viItem ] ), pRecord, sizeof( phonebook_record_t ) );
}

static void DeletePhonebookRecordFromVi( unsigned int viDeleteItem )
{
	unsigned int riTargetItem;

	if( viDeleteItem == nNumberOfPhonebookRecords - 1 ) {
		/* delete last one */
	} else {
		/* delete middle one */
		riTargetItem = riFromViPhonebookRecordSort[ viDeleteItem ];

		memcpy( &riFromViPhonebookRecordSort[ viDeleteItem ], 
				&riFromViPhonebookRecordSort[ viDeleteItem + 1 ],
				( nNumberOfPhonebookRecords - viDeleteItem - 1 ) * sizeof( riFromViPhonebookRecordSort[ 0 ] ) );

		riFromViPhonebookRecordSort[ nNumberOfPhonebookRecords - 1 ] = riTargetItem;

		FlashWriteData( PHONEBOOK_FIELD_OFFSET( phonebook_info_t, riFromViPhonebookRecordSort ) +
						MY_FIELD_SIZE( phonebook_info_t, riFromViPhonebookRecordSort[ 0 ] ) * viDeleteItem, 
						&riFromViPhonebookRecordSort[ viDeleteItem ], 
						MY_FIELD_SIZE( phonebook_info_t, riFromViPhonebookRecordSort[ 0 ] ) * ( nNumberOfPhonebookRecords - viDeleteItem ) ); 
	}

	nNumberOfPhonebookRecords --;

	FlashWriteData( PHONEBOOK_FIELD_OFFSET( phonebook_info_t, nNumberOfPhonebookRecord ), 
				    &nNumberOfPhonebookRecords, sizeof( nNumberOfPhonebookRecords ) ); 
				    
	FlashValidateWriting( 0 );
}

static void DeleteAllPhonebookRecord( void )
{
	PhonebookBasicInitialization( 1 /* clear */ );
	
	FlashValidateWriting( 0 );
}

static void UpdatePhonebookRecordFromEditingBuffer( void )
{
	unsigned int viInsert, riTargetIndex;
	unsigned char szName[ MAX_LEN_OF_PHONE_NAME + 1 ];

	if( viPhonebookEdit == nNumberOfPhonebookRecords ) {
		/* new a record */
		for( viInsert = 0; viInsert < nNumberOfPhonebookRecords; viInsert ++ ) {
			GetPhonebookListItemTextFromVi( viInsert, szName );
			if( strcmp( ( char * )szName, ( char * )phonebookEditBuffer.szName ) > 0 )
				break;
		}
		
		nNumberOfPhonebookRecords ++;
		riTargetIndex = riFromViPhonebookRecordSort[ viPhonebookEdit ];

		FlashWriteData( PHONEBOOK_RECORD_OFFSET( riTargetIndex ), 
					    &phonebookEditBuffer, sizeof( phonebookEditBuffer ) ); 
		
		FlashWriteData( PHONEBOOK_FIELD_OFFSET( phonebook_info_t, nNumberOfPhonebookRecord ), 
					    &nNumberOfPhonebookRecords, sizeof( nNumberOfPhonebookRecords ) );

		if( viInsert != nNumberOfPhonebookRecords - 1 ) {
			/* not last one --> do insert */		
			memmove( &riFromViPhonebookRecordSort[ viInsert + 1 ], &riFromViPhonebookRecordSort[ viInsert ],
					( nNumberOfPhonebookRecords - viInsert - 1 ) * sizeof( riFromViPhonebookRecordSort[ 0 ] ) );

			riFromViPhonebookRecordSort[ viInsert ] = riTargetIndex;

			FlashWriteData( PHONEBOOK_FIELD_OFFSET( phonebook_info_t, riFromViPhonebookRecordSort ) +
							MY_FIELD_SIZE( phonebook_info_t, riFromViPhonebookRecordSort[ 0 ] ) * viInsert, 
							&riFromViPhonebookRecordSort[ viInsert ], 
							MY_FIELD_SIZE( phonebook_info_t, riFromViPhonebookRecordSort[ 0 ] ) * ( nNumberOfPhonebookRecords - viInsert ) ); 
		}

		/* Make viPhonebookEdit to keep current editing */
		viPhonebookEdit = viInsert;
	} else {
		/* modify */
		riTargetIndex = riFromViPhonebookRecordSort[ viPhonebookEdit ];

		FlashWriteData( PHONEBOOK_RECORD_OFFSET( riTargetIndex ), 
					    &phonebookEditBuffer, sizeof( phonebookEditBuffer ) ); 		
	}
	
	FlashValidateWriting( 0 );
}

/* ****************************************************************************
 * Phonebook share states
 * **************************************************************************** */
static void GetStateAndParamAfterPhonebookEdit( state_back_level_t level, ui_state_t *pState, int *pParam )
{
	int help;

	UI_PeekStateStack( level, pState, &help );

	switch( help ) {
	case PHONEBOOK_EDIT_HELP_LIST_NO_RECORD:
		*pParam = 0;
		break;

	case PHONEBOOK_EDIT_HELP_ACT_ADD:
		*pParam = PHONEBOOK_ACT_ID_ADD;
		break;

	case PHONEBOOK_EDIT_HELP_ACT_MODIFY:
		*pParam = PHONEBOOK_ACT_ID_MODIFY;
		break;

	default:
		debug_out( "Phonebook edit has no help??\n" );
	}
}

void SetupPhonebookEditNameFrame( int param )
{
	params_for_single_line_editor_t edit_params;

	GS_DrawOffScreenAndClearScreen();

	/* Draw Title */
	GS_TextOut( 0, 0, szEnterName, 5 );

	/* Activate editor */
	edit_params.nDefaultTextLength = strlen( ( char * )phonebookEditBuffer.szName );
	edit_params.pszDefaultText = phonebookEditBuffer.szName;
	edit_params.nMaxTextLength = MAX_LEN_OF_PHONE_NAME;
	edit_params.tTextType = TEXT_TYPE_NORMAL;
	edit_params.fParams.all = 0;

	ActivateTextEditor( EDITOR_ID_SINGLE_LINE, ( const params_for_editor_t * )&edit_params );

	/* Configuration softkey */
	SoftkeyStandardConfiguration_OkBack();

	/* Activate IME */
#if defined( LANG_BIG5 ) || defined( LANG_GB2312 )
	ActivateInputMethodEditor( IME_ID_PINYIN );
#else
	ActivateInputMethodEditor( IME_ID_ENGLISH );
#endif

	GS_DrawOnScreen( NULL );
	GS_SetCursorStatus( CURSOR_STATUS_ON );
}

void StatePhonebookEditNameOnKey( unsigned char key )	/* UI_STATE_PHONEBOOK_EDIT_NAME */
{
	ui_state_t originalState;
	int param;

	if( key == VKEY_CANCEL ) {
		GetStateAndParamAfterPhonebookEdit( STATE_BACK_LEVEL_ONE, &originalState, &param );
		UI_StateTransition( originalState, param );
	} else if( key == VKEY_OK ) {
		if( GetTextEditorTextContent( phonebookEditBuffer.szName, MAX_LEN_OF_PHONE_NAME ) == 0 ) {
			SetupPromptFrame( szEmptyIsNotAllow );
			UI_StateTransitionToPrompt( UI_STATE_PHONEBOOK_EDIT_NAME, 0 );
		} else
			UI_StateTransition( UI_STATE_PHONEBOOK_EDIT_NUMBER, 0 );
	}
}

void SetupPhonebookEditNumberFrame( int param )
{
	params_for_single_line_editor_t edit_params;

	GS_DrawOffScreenAndClearScreen();

	/* Draw Title */
	GS_TextOut( 0, 0, szEnterNumber, RES_strlen( szEnterNumber ) );

	/* Activate editor */
	edit_params.nDefaultTextLength = 0;		/* ignore if BCD */
	edit_params.pszDefaultText = phonebookEditBuffer.bcdNumber;
	edit_params.nMaxTextLength = MAX_LEN_OF_PHONE_NUMBER;
	edit_params.tTextType = TEXT_TYPE_BCD;
	edit_params.fParams.all = 0;

	ActivateTextEditor( EDITOR_ID_SINGLE_LINE, ( const params_for_editor_t * )&edit_params );

	SoftkeyStandardConfiguration_OkBack();

	GS_DrawOnScreen( NULL );
	GS_SetCursorStatus( CURSOR_STATUS_ON );
}

void StatePhonebookEditNumberOnKey( unsigned char key )	/* UI_STATE_PHONEBOOK_EDIT_NUMBER */
{
	/* Cancel key is processed by UI_KeypadInput() */
	//ui_state_t originalState;
	//int param;

	if( key == VKEY_OK ) {
		// TODO: save name and state transition to edit number
		if( GetTextEditorTextContent( phonebookEditBuffer.bcdNumber, BCD_LEN_OF_PHONE_NUMBER ) == 0 ) {
			SetupPromptFrame( szEmptyIsNotAllow );
			UI_StateTransitionToPrompt( UI_STATE_PHONEBOOK_EDIT_NUMBER, 0 );
		} else {
			UpdatePhonebookRecordFromEditingBuffer();

			SetupPromptFrame( szOK );
#if 0
			//GetStateAndParamAfterPhonebookEdit( STATE_BACK_LEVEL_TWO, &originalState, &param );

			//UI_StateTransitionToPrompt( originalState, param );
#else
			UI_StateTransitionToPrompt( UI_STATE_PHONEBOOK_LIST, viPhonebookEdit );
#endif			
		}
	}
}

/* ****************************************************************************
 * Phonebook states
 * **************************************************************************** */
void SetupPhonebookListFrame( int param )
{
	menu_select_t phonebookListSelect = {
		SELECT_ATTRIB_GET_TEXT_FUNC,		/* attribute */
		nNumberOfPhonebookRecords,	/* number of items */	/* (it is a variable) */
		NULL,	/* point to items */
		GetPhonebookListItemTextFromVi,	/* point to get item text function */
	};
		
	if( nNumberOfPhonebookRecords == 0 )
		/* No record */
		SetupFrameWithCentralizedStringAndSoftkey( szPhonebookHasNoRecord, SOFTKEY_LR_YES_NO );
	else {
		ActivateMenuSelection( &phonebookListSelect, param );
	}
}

void StatePhonebookListOnKey( unsigned char key )	/* UI_STATE_PHONEBOOK_LIST */
{
	ui_state_t state;
	int help;
	
	if( key == VKEY_CANCEL ) {
		UI_PeekStateStack( STATE_BACK_LEVEL_ONE, &state, &help );

		if( state == UI_STATE_MENU_ACT )
			UI_StateTransition( UI_STATE_MENU_ACT, help );
		else
			UI_StateBack( STATE_BACK_LEVEL_ONE );
	} else if( key == VKEY_OK ) {
		if( nNumberOfPhonebookRecords == 0 ) {
			/* add a new record */
			viPhonebookEdit = nNumberOfPhonebookRecords;
			phonebookEditBuffer.szName[ 0 ] = '\x0';
			phonebookEditBuffer.bcdNumber[ 0 ] = '\xFF';

			UI_StateTransitionWithBackHelp( UI_STATE_PHONEBOOK_EDIT_NAME, 0, 
											PHONEBOOK_EDIT_HELP_LIST_NO_RECORD /* back help */ );
		} else {
			/* action */
			nSelectedItem = DeactivateMenuSelection();

			viPhonebookEdit = nSelectedItem;

			UI_StateTransition( UI_STATE_PHONEBOOK_ACT, 0 );
		}
	}
}

void SetupPhonebookActFrame( int param )
{
	static const menu_item_t phonebookActItems[] = {
		{ szAdd, 0, NULL },
		{ szModify, 0, NULL },
		{ szDelete, 0, NULL },
		{ szDeleteAll, 0, NULL },
		{ szStatus, 0, NULL },
	};
	
	static const menu_select_t phonebookActSelect = {
		0,		/* attribute */
		sizeof( phonebookActItems ) / sizeof( phonebookActItems[ 0 ] ),	/* number of items */
		phonebookActItems,	/* point to items */
		NULL,	/* point to get item text function */
	};
	
	ActivateMenuSelection( &phonebookActSelect, param );
}

void StatePhonebookActOnKey( unsigned char key )	/* UI_STATE_PHONEBOOK_ACT */
{
	if( key == VKEY_CANCEL ) {
#if 1
		UI_StateTransition( UI_STATE_PHONEBOOK_LIST, viPhonebookEdit );
#else
		UI_StateBack( STATE_BACK_LEVEL_ONE );
#endif
	} else if( key == VKEY_OK ) {
		nSelectedItem = DeactivateMenuSelection();

		switch( nSelectedItem ) {
		case PHONEBOOK_ACT_ID_ADD:
			if( nNumberOfPhonebookRecords >= NUM_OF_PHONEBOOK_RECORD ) {
				/* full */
				SetupPromptFrame( szPhonebookIsFull );
				UI_StateTransitionToPrompt( UI_STATE_PHONEBOOK_ACT, PHONEBOOK_ACT_ID_ADD );
			} else {
				viPhonebookEdit = nNumberOfPhonebookRecords;
				phonebookEditBuffer.szName[ 0 ] = '\x0';
				phonebookEditBuffer.bcdNumber[ 0 ] = '\xFF';

				UI_StateTransitionWithBackHelp( UI_STATE_PHONEBOOK_EDIT_NAME, 0, 
												PHONEBOOK_EDIT_HELP_ACT_ADD /* back help */ );
			}
			break;

		case PHONEBOOK_ACT_ID_MODIFY:
			ReadPhonebookRecordFromVi( viPhonebookEdit, &phonebookEditBuffer );

			UI_StateTransitionWithBackHelp( UI_STATE_PHONEBOOK_EDIT_NAME, 0, 
											PHONEBOOK_EDIT_HELP_ACT_MODIFY /* back help */ );
			break;

		case PHONEBOOK_ACT_ID_DEL:
			UI_StateTransition( UI_STATE_PHONEBOOK_ACT_DEL, 0 );
			break;

		case PHONEBOOK_ACT_ID_DEL_ALL:
			UI_StateTransition( UI_STATE_PHONEBOOK_ACT_DEL_ALL, 0 );
			break;

		case PHONEBOOK_ACT_ID_STATUS:
			UI_StateTransition( UI_STATE_PHONEBOOK_ACT_STATUS, 0 );
			break;

		}
	}
}

void SetupPhonebookActAddFrame( int param )
{
	params_for_single_line_editor_t edit_params;

	GS_DrawOffScreenAndClearScreen();

	/* Draw Title */
	GS_TextOut( 0, 0, szEnterName, 5 );

	/* Activate editor */
	edit_params.nDefaultTextLength = 0;
	edit_params.nMaxTextLength = MAX_LENGTH_OF_SINGLE_LINE_EDITOR;
	edit_params.tTextType = TEXT_TYPE_NORMAL;
	edit_params.fParams.all = 0;

	ActivateTextEditor( EDITOR_ID_SINGLE_LINE, ( const params_for_editor_t * )&edit_params );

	/* Configuration softkey */
	SoftkeyStandardConfiguration_OkBack();

	/* Activate IME */
	ActivateInputMethodEditor( IME_ID_ENGLISH );

	GS_DrawOnScreen( NULL );
	GS_SetCursorStatus( CURSOR_STATUS_ON );
}

void StatePhonebookActAddOnKey( unsigned char key )	/* UI_STATE_PHONEBOOK_ACT_ADD */
{
}

void SetupPhonebookActModifyFrame( int param )
{
}

void StatePhonebookActModifyOnKey( unsigned char key )	/* UI_STATE_PHONEBOOK_ACT_MODIFY */
{
}

void SetupPhonebookActDelFrame( int param )
{
	GS_DrawOffScreenAndClearScreen();

	GS_TextOutInCenter( 0, szDelete );

	GS_TextOutInCenter( 1, szQSure );

	SoftkeyStandardConfiguration_YesNo();
	
	GS_DrawOnScreen( NULL );
}

void StatePhonebookActDelOnKey( unsigned char key )	/* UI_STATE_PHONEBOOK_ACT_DEL */
{
	/* Cancel key is processed by UI_KeypadInput() */
	if( key == VKEY_OK ) {
		DeletePhonebookRecordFromVi( viPhonebookEdit );

		SetupPromptFrame( szOK );

		UI_StateTransitionToPrompt( UI_STATE_PHONEBOOK_LIST, 
									( ( viPhonebookEdit >= nNumberOfPhonebookRecords ) ?
									  ( nNumberOfPhonebookRecords - 1 ) :
									  viPhonebookEdit ) );

	}
}

void SetupPhonebookActDelAllFrame( int param )
{
	GS_DrawOffScreenAndClearScreen();

	GS_TextOutInCenter( 0, szDeleteAll );

	GS_TextOutInCenter( 1, szQSure );

	SoftkeyStandardConfiguration_YesNo();

	GS_DrawOnScreen( NULL );
}

void StatePhonebookActDelAllOnKey( unsigned char key )	/* UI_STATE_PHONEBOOK_ACT_DEL_ALL */
{
	/* Cancel key is processed by UI_KeypadInput() */
	if( key == VKEY_OK ) {
		DeleteAllPhonebookRecord();

		SetupPromptFrame( szOK );
		UI_StateTransitionToPrompt( UI_STATE_PHONEBOOK_LIST, 0 );
	}
}

void SetupPhonebookActStatusFrame( int param )
{
	unsigned char szBuffer[ 16 ];

	GS_DrawOffScreenAndClearScreen();

	/* used record */
	sprintf( ( char * )szBuffer, ( char * )szUsedFormat, nNumberOfPhonebookRecords );
	GS_TextOut( 0, 0, szBuffer, strlen( ( char * )szBuffer ) );

	/* free record */
	sprintf( ( char * )szBuffer, ( char * )szFreeFormat, NUM_OF_PHONEBOOK_RECORD - nNumberOfPhonebookRecords );
	GS_TextOut( 0, 1, szBuffer, strlen( ( char * )szBuffer ) );

	SoftkeyStandardConfiguration_Back();

	GS_DrawOnScreen( NULL );
}

void StatePhonebookActStatusOnKey( unsigned char key )	/* UI_STATE_PHONEBOOK_ACT_STATUS */
{
	/* Cancel key is processed by UI_KeypadInput() */
	if( key == VKEY_OK )
		UI_StateBack( STATE_BACK_LEVEL_ONE  );
}
