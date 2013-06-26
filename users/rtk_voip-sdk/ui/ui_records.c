#include "ui_config.h"
#include "ui_records.h"
#include "ui_host.h"
#include "flash_layout.h"
#include "flash_rw_api.h"
#include "gs_lib.h"
#include "res.h"
#include <string.h>
#include <stdio.h>

static unsigned char nUsedRecordsOfMissedCall;
static unsigned char nUsedRecordsOfIncomingCall;
static unsigned char nUsedRecordsOfOutgoingCall;
static unsigned char nSortRecordsOfMissedCall[ NUM_OF_CALL_RECORD_UNITS ];
static unsigned char nSortRecordsOfIncomingCall[ NUM_OF_CALL_RECORD_UNITS ];
static unsigned char nSortRecordsOfOutgoingCall[ NUM_OF_CALL_RECORD_UNITS ];

void LoadCallRecordsFromFlash( int bDefault )
{
	unsigned char i;

	if( bDefault ) {
		nUsedRecordsOfMissedCall = 0;
		nUsedRecordsOfIncomingCall = 0;
		nUsedRecordsOfOutgoingCall = 0;

		for( i = 0; i < NUM_OF_CALL_RECORD_UNITS; i ++ ) {
			nSortRecordsOfMissedCall[ i ] = i;
			nSortRecordsOfIncomingCall[ i ] = i;
			nSortRecordsOfOutgoingCall[ i ] = i;
		}

		FlashWriteOneByte( MY_FIELD_OFFSET( ui_falsh_layout_t, callrecord.missed.used ), 0 );
		FlashWriteOneByte( MY_FIELD_OFFSET( ui_falsh_layout_t, callrecord.incoming.used ), 0 );
		FlashWriteOneByte( MY_FIELD_OFFSET( ui_falsh_layout_t, callrecord.outgoing.used ), 0 );

		FlashWriteData( MY_FIELD_OFFSET( ui_falsh_layout_t, callrecord.missed.sort ), 
						nSortRecordsOfMissedCall, 
						MY_FIELD_SIZE( ui_falsh_layout_t, callrecord.missed.sort ) );
		FlashWriteData( MY_FIELD_OFFSET( ui_falsh_layout_t, callrecord.incoming.sort ), 
						nSortRecordsOfIncomingCall,
						MY_FIELD_SIZE( ui_falsh_layout_t, callrecord.incoming.sort ) );
		FlashWriteData( MY_FIELD_OFFSET( ui_falsh_layout_t, callrecord.outgoing.sort ), 
						nSortRecordsOfOutgoingCall, 
						MY_FIELD_SIZE( ui_falsh_layout_t, callrecord.outgoing.sort ) );	
		
		/* FlashValidateWriting() by caller */
	} else {
		nUsedRecordsOfMissedCall = FlashReadOneByte( 
									MY_FIELD_OFFSET( ui_falsh_layout_t, callrecord.missed.used ) );
		nUsedRecordsOfIncomingCall = FlashReadOneByte(
									MY_FIELD_OFFSET( ui_falsh_layout_t, callrecord.incoming.used ) );
		nUsedRecordsOfOutgoingCall = FlashReadOneByte(
									MY_FIELD_OFFSET( ui_falsh_layout_t, callrecord.outgoing.used ) );

		FlashReadData( MY_FIELD_OFFSET( ui_falsh_layout_t, callrecord.missed.sort ), 
					   nSortRecordsOfMissedCall, 
					   MY_FIELD_SIZE( ui_falsh_layout_t, callrecord.missed.sort ) );
		FlashReadData( MY_FIELD_OFFSET( ui_falsh_layout_t, callrecord.incoming.sort ), 
					   nSortRecordsOfIncomingCall,
					   MY_FIELD_SIZE( ui_falsh_layout_t, callrecord.incoming.sort ) );
		FlashReadData( MY_FIELD_OFFSET( ui_falsh_layout_t, callrecord.outgoing.sort ), 
					   nSortRecordsOfOutgoingCall, 
					   MY_FIELD_SIZE( ui_falsh_layout_t, callrecord.outgoing.sort ) );
	}
}

static void AddOneCallRecord( const unsigned char *pszPhonenumber, datetime_t datetime, /* input */
								unsigned long offset,	/* flash offset */
								unsigned char *pSort, unsigned char *pUsed	/* related variables */
								)
{
	unsigned char used = *pUsed;
	unsigned char i, j;
	unsigned char index;
	unsigned char szSrcPhonenumber[ MAX_LEN_OF_PHONENUMBER + 1 ];
	unsigned char temp;
	call_records_unit_t unit;

	/* search for the same phone number */
	for( i = 0; i < used; i ++ ) {

		index = pSort[ i ];

		FlashReadData( offset + MY_FIELD_OFFSET( call_records_fold_t, unit[ index ].szPhonenumber ),
						szSrcPhonenumber, 
						MY_FIELD_SIZE( call_records_fold_t, unit[ index ].szPhonenumber ) );

		if( strcmp( ( const char * )szSrcPhonenumber, ( const char * )pszPhonenumber ) == 0 )
			break;
	}

	if( i < used ) {	/* found! add datetime */

		index = pSort[ i ];

		FlashReadData( offset + MY_FIELD_OFFSET( call_records_fold_t, unit[ index ].datetime ),
						unit.datetime,
						MY_FIELD_SIZE( call_records_fold_t, unit[ index ].datetime ) );

#if NUM_OF_CALL_RECORD_DATETIME > 1
		for( j = NUM_OF_CALL_RECORD_DATETIME - 1; j > 0; j -- )
			unit.datetime[ j ] = unit.datetime[ j - 1 ];
#endif
		
		unit.datetime[ 0 ] = datetime;

		/* bring this unit to be first one */
		for( j = i; j > 0; j -- )
			pSort[ j ] = pSort[ j - 1 ];

		pSort[ 0 ] = index;

		/* write unit */
		FlashWriteData( offset + MY_FIELD_OFFSET( call_records_fold_t, unit[ index ].datetime ),
						unit.datetime,
						MY_FIELD_SIZE( call_records_fold_t, unit[ index ].datetime ) );

		/* write sort */
		FlashWriteData( offset + MY_FIELD_OFFSET( call_records_fold_t, sort ),
						pSort, 
						MY_FIELD_SIZE( call_records_fold_t, sort ) );

	} else {			/* not found! get a free space, and add a new record */

		/* get a free space */
		if( used >= NUM_OF_CALL_RECORD_UNITS ) {	/* full */

			index = temp = pSort[ NUM_OF_CALL_RECORD_UNITS - 1 ];

			for( j = NUM_OF_CALL_RECORD_UNITS - 1; j > 0; j -- )
				pSort[ j ] = pSort[ j - 1 ];

			pSort[ 0 ] = temp;

		} else {

			index = temp = pSort[ used ];

			for( j = used; j > 0; j -- )
				pSort[ j ] = pSort[ j - 1 ];

			pSort[ 0 ] = temp;

			( *pUsed ) ++;
		}

		/* add a new record at index 0 */
		unit.datetime[ 0 ] = datetime;

#if NUM_OF_CALL_RECORD_DATETIME > 1
		for( j = 1; j < NUM_OF_CALL_RECORD_DATETIME; j ++ )
			unit.datetime[ j ].all = 0;
#endif

		/* write a unit */
		FlashWriteData( offset + MY_FIELD_OFFSET( call_records_fold_t, unit[ index ].szPhonenumber ),
						pszPhonenumber, 
						MY_FIELD_SIZE( call_records_fold_t, unit[ index ].szPhonenumber ) );

		FlashWriteData( offset + MY_FIELD_OFFSET( call_records_fold_t, unit[ index ].datetime ),
						unit.datetime,
						MY_FIELD_SIZE( call_records_fold_t, unit[ index ].datetime ) );

		/* write sort */
		FlashWriteData( offset + MY_FIELD_OFFSET( call_records_fold_t, sort ),
						pSort, 
						MY_FIELD_SIZE( call_records_fold_t, sort ) );

		/* write used number */
		FlashWriteOneByte( offset + MY_FIELD_OFFSET( call_records_fold_t, used ), *pUsed );
	}
	
	FlashValidateWriting( 0 );
}

void AddRecordOfMissedCall( const unsigned char *pszPhonenumber, datetime_t datetime )
{
	AddOneCallRecord( pszPhonenumber, datetime, 
						MY_FIELD_OFFSET( ui_falsh_layout_t, callrecord.missed ), 
						nSortRecordsOfMissedCall, &nUsedRecordsOfMissedCall );
}

void AddRecordOfIncomingCall( const unsigned char *pszPhonenumber, datetime_t datetime )
{
	AddOneCallRecord( pszPhonenumber, datetime, 
						MY_FIELD_OFFSET( ui_falsh_layout_t, callrecord.incoming ), 
						nSortRecordsOfIncomingCall, &nUsedRecordsOfIncomingCall );
}

void AddRecordOfOutgoingCall( const unsigned char *pszPhonenumber, datetime_t datetime )
{
	AddOneCallRecord( pszPhonenumber, datetime, 
						MY_FIELD_OFFSET( ui_falsh_layout_t, callrecord.outgoing ), 
						nSortRecordsOfOutgoingCall, &nUsedRecordsOfOutgoingCall );
}


/* ============================================================================ */
/* State Frames */
/* ============================================================================ */

unsigned int GetTheLastOutgoingCallRecord( unsigned char *pszPhonenumber )
{
	unsigned int index;

	if( nUsedRecordsOfOutgoingCall == 0 )
		return 0;	/* No record */

	index = nSortRecordsOfOutgoingCall[ 0 ];	/* recent outgoing call */

	FlashReadData(	MY_FIELD_OFFSET( ui_falsh_layout_t, callrecord.outgoing ) +
					MY_FIELD_OFFSET( call_records_fold_t, unit[ index ].szPhonenumber ),
					pszPhonenumber, 
					MY_FIELD_SIZE( call_records_fold_t, unit[ index ].szPhonenumber ) );

	return nUsedRecordsOfOutgoingCall;
}

static void PutCallRecordsToDynamicMenuItem( unsigned char nRecords, 
											 unsigned long fold_offset,	/* flash offset */
											 const unsigned char *pSort )
{
	unsigned int i;
	unsigned char index;
	unsigned char szSrcPhonenumber[ MAX_LEN_OF_PHONENUMBER + 1 ];

	for( i = 0; i < nRecords; i ++ ) { 
		DynamicMenuItem[ i ].fMenuItemAttrib = 0;
		DynamicMenuItem[ i ].pFnItemGetItemGeneral = NULL;

		index = pSort[ i ];

		FlashReadData( fold_offset + MY_FIELD_OFFSET( call_records_fold_t, unit[ index ].szPhonenumber ),
						szSrcPhonenumber, 
						MY_FIELD_SIZE( call_records_fold_t, unit[ index ].szPhonenumber ) );

		if( szSrcPhonenumber[ 0 ] == '\x0' )
			strcpy( ( char * )DynamicMenuItemText[ i ], ( const char * )szNoname );
		else {
			memcpy( ( char * )DynamicMenuItemText[ i ], ( const char * )szSrcPhonenumber, MAX_LEN_OF_DYNAMIC_MENU_ITEM_TEXT );
			DynamicMenuItemText[ i ][ MAX_LEN_OF_DYNAMIC_MENU_ITEM_TEXT ] = '\x0';
		}
	}
}

void SetupCallRecordMissedFrame( int param )
{
	if( nUsedRecordsOfMissedCall == 0 ) {

		SetupFrameWithCentralizedStringAndSoftkey( szNoRecord, SOFTKEY_R_BACK );

		return;
	}

	PutCallRecordsToDynamicMenuItem( nUsedRecordsOfMissedCall, 
									 MY_FIELD_OFFSET( ui_falsh_layout_t, callrecord.missed ), 
									 nSortRecordsOfMissedCall );

	ActivateDynamicMenuSelectionWithAttributes( nUsedRecordsOfMissedCall /* number of items */, param /* default select */, SELECT_ATTRIB_SOFTKEY_DETAIL_BACK );
}

void StateCallRecordMissedOnKey( unsigned char key )	/* UI_STATE_CALL_RECORD_MISSED */
{
	if( key == VKEY_CANCEL )
		UI_StateBack( STATE_BACK_LEVEL_ONE );
	else if( nUsedRecordsOfMissedCall ) {
		if( key == VKEY_OK ) {
			nSelectedItem = DeactivateMenuSelection();

			UI_StateTransitionWithBackHelp( UI_STATE_CALL_RECORD_MISSED_DETAIL, nSelectedItem, nSelectedItem );
		}
	}
}

void SetupCallRecordIncomingFrame( int param )
{
	if( nUsedRecordsOfIncomingCall == 0 ) {

		SetupFrameWithCentralizedStringAndSoftkey( szNoRecord, SOFTKEY_R_BACK );

		return;
	}

	PutCallRecordsToDynamicMenuItem( nUsedRecordsOfIncomingCall, 
									 MY_FIELD_OFFSET( ui_falsh_layout_t, callrecord.incoming ), 
									 nSortRecordsOfIncomingCall );

	ActivateDynamicMenuSelectionWithAttributes( nUsedRecordsOfIncomingCall /* number of items */, param /* default select */, SELECT_ATTRIB_SOFTKEY_DETAIL_BACK );
}

void StateCallRecordIncomingOnKey( unsigned char key )	/* UI_STATE_CALL_RECORD_INCOMING */
{
	if( key == VKEY_CANCEL )
		UI_StateBack( STATE_BACK_LEVEL_ONE );
	else if( nUsedRecordsOfIncomingCall ) {
		if( key == VKEY_OK ) {
			nSelectedItem = DeactivateMenuSelection();

			UI_StateTransitionWithBackHelp( UI_STATE_CALL_RECORD_INCOMING_DETAIL, nSelectedItem, nSelectedItem );
		}
	}
}

void SetupCallRecordOutgoingFrame( int param )
{
	if( nUsedRecordsOfOutgoingCall == 0 ) {

		SetupFrameWithCentralizedStringAndSoftkey( szNoRecord, SOFTKEY_R_BACK );

		return;
	}

	PutCallRecordsToDynamicMenuItem( nUsedRecordsOfOutgoingCall, 
									 MY_FIELD_OFFSET( ui_falsh_layout_t, callrecord.outgoing ), 
									 nSortRecordsOfOutgoingCall );

	ActivateDynamicMenuSelectionWithAttributes( nUsedRecordsOfOutgoingCall /* number of items */, param /* default select */, SELECT_ATTRIB_SOFTKEY_DETAIL_BACK );
}

void StateCallRecordOutgoingOnKey( unsigned char key )	/* UI_STATE_CALL_RECORD_OUTGOING */
{
	if( key == VKEY_CANCEL )
		UI_StateBack( STATE_BACK_LEVEL_ONE );
	else if( nUsedRecordsOfOutgoingCall ) {
		if( key == VKEY_OK ) {
			nSelectedItem = DeactivateMenuSelection();

			UI_StateTransitionWithBackHelp( UI_STATE_CALL_RECORD_OUTGOING_DETAIL, nSelectedItem, nSelectedItem );
		}
	}
}

static unsigned int PutCallRecordDetailToDynamicMenuItem( unsigned int nSelected,
									/* flash offset */	  unsigned long fold_offset,
														  const unsigned char *pSort )
{
	unsigned int i;
	unsigned char index;
	call_records_unit_t unit;
	datetime_t datetime;

	/* read whole unit */
	index = pSort[ nSelected ];

	FlashReadData( fold_offset + MY_FIELD_OFFSET( call_records_fold_t, unit[ index ] ),
					&unit, 
					MY_FIELD_SIZE( call_records_fold_t, unit[ index ] ) );

	/* Line 1: display phonenumber */
	DynamicMenuItem[ 0 ].fMenuItemAttrib = 0;
	DynamicMenuItem[ 0 ].pFnItemGetItemGeneral = NULL;

	if( unit.szPhonenumber[ 0 ] == '\x0' )
		strcpy( ( char * )DynamicMenuItemText[ 0 ], ( const char * )szNoname );
	else {
		memcpy( ( char * )DynamicMenuItemText[ 0 ], ( const char * )unit.szPhonenumber, MAX_LEN_OF_DYNAMIC_MENU_ITEM_TEXT );
		DynamicMenuItemText[ 0 ][ MAX_LEN_OF_DYNAMIC_MENU_ITEM_TEXT ] = '\x0';
	}

	/* Line 2 ~ n: display datetime */
	for( i = 1; i <= NUM_OF_CALL_RECORD_DATETIME; i ++ ) {

		datetime = unit.datetime[ i - 1 ];

		if( datetime.all == 0 )
			break;

		DynamicMenuItem[ i ].fMenuItemAttrib = 0;
		DynamicMenuItem[ i ].pFnItemGetItemGeneral = NULL;

		MakeShortDateTimeString( DynamicMenuItemText[ i ], datetime );
	}

	return i;
}

void SetupCallRecordMissedDetailFrame( int param )
{
	unsigned int num;

	num = PutCallRecordDetailToDynamicMenuItem( param, 
												MY_FIELD_OFFSET( ui_falsh_layout_t, callrecord.missed ),
												nSortRecordsOfMissedCall );

	ActivateDynamicMenuSelectionWithAttributes( num /* number of items */, 0 /* default select */, SELECT_ATTRIB_SOFTKEY_BACK );
}

void StateCallRecordMissedDetailOnKey( unsigned char key )	/* UI_STATE_CALL_RECORD_MISSED_DETAIL */
{
	ui_state_t state;
	int help;

	if( key == VKEY_CANCEL ) {
		UI_PeekStateStack( STATE_BACK_LEVEL_ONE, &state, &help );

		if( state != UI_STATE_CALL_RECORD_MISSED )
			debug_out( "Missed detail: Not expected state.\n" );
		else 
			UI_StateTransition( UI_STATE_CALL_RECORD_MISSED, help );
	}
}

void SetupCallRecordIncomingDetailFrame( int param )
{
	unsigned int num;

	num = PutCallRecordDetailToDynamicMenuItem( param, 
												MY_FIELD_OFFSET( ui_falsh_layout_t, callrecord.incoming ),
												nSortRecordsOfIncomingCall );

	ActivateDynamicMenuSelectionWithAttributes( num /* number of items */, 0 /* default select */, SELECT_ATTRIB_SOFTKEY_BACK );
}

void StateCallRecordIncomingDetailOnKey( unsigned char key )	/* UI_STATE_CALL_RECORD_INCOMING_DETAIL */
{
	ui_state_t state;
	int help;

	if( key == VKEY_CANCEL ) {
		UI_PeekStateStack( STATE_BACK_LEVEL_ONE, &state, &help );

		if( state != UI_STATE_CALL_RECORD_INCOMING )
			debug_out( "Incoming detail: Not expected state.\n" );
		else 
			UI_StateTransition( UI_STATE_CALL_RECORD_INCOMING, help );
	}
}

void SetupCallRecordOutgoingDetailFrame( int param )
{
	unsigned int num;

	num = PutCallRecordDetailToDynamicMenuItem( param, 
												MY_FIELD_OFFSET( ui_falsh_layout_t, callrecord.outgoing ),
												nSortRecordsOfOutgoingCall );

	ActivateDynamicMenuSelectionWithAttributes( num /* number of items */, 0 /* default select */, SELECT_ATTRIB_SOFTKEY_BACK );
}

void StateCallRecordOutgoingDetailOnKey( unsigned char key )	/* UI_STATE_CALL_RECORD_OUTGOING_DETAIL */
{
	ui_state_t state;
	int help;

	if( key == VKEY_CANCEL ) {
		UI_PeekStateStack( STATE_BACK_LEVEL_ONE, &state, &help );

		if( state != UI_STATE_CALL_RECORD_OUTGOING )
			debug_out( "Outgoing detail: Not expected state.\n" );
		else 
			UI_StateTransition( UI_STATE_CALL_RECORD_OUTGOING, help );
	}
}
