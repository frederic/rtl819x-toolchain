#include <string.h>
#include <stdio.h>
#include "ui_config.h"
#include "ui_vkey.h"
#include "ui_softkey.h"
#include "gs_lib.h"
#include "ioctl_softrtc.h"
#include "com_scroll.h"
#include "com_select.h"

/* ============================================================================ */
/* dynamic menu item buffer for easy usage. */
unsigned char DynamicMenuItemText[ MAX_NUM_OF_DYNAMIC_MENU_ITEM ][ MAX_LEN_OF_DYNAMIC_MENU_ITEM_TEXT + 1 ];

#define _M_DYNAMIC_ITEM( n )	{ DynamicMenuItemText[ n ], 0, NULL }

menu_item_t DynamicMenuItem[ MAX_NUM_OF_DYNAMIC_MENU_ITEM ] = {
	_M_DYNAMIC_ITEM( 0 ), _M_DYNAMIC_ITEM( 1 ), _M_DYNAMIC_ITEM( 2 ), _M_DYNAMIC_ITEM( 3 ), 
	_M_DYNAMIC_ITEM( 4 ), _M_DYNAMIC_ITEM( 5 ), _M_DYNAMIC_ITEM( 6 ), _M_DYNAMIC_ITEM( 7 ), 
	_M_DYNAMIC_ITEM( 8 ), _M_DYNAMIC_ITEM( 9 ), _M_DYNAMIC_ITEM( 10 ), _M_DYNAMIC_ITEM( 11 ), 
	_M_DYNAMIC_ITEM( 12 ), _M_DYNAMIC_ITEM( 13 ), _M_DYNAMIC_ITEM( 14 ), _M_DYNAMIC_ITEM( 15 ), 
};

#undef _M_DYNAMIC_ITEM

#if MAX_NUM_OF_DYNAMIC_MENU_ITEM != 16
#error "Need 16 initializer!"
#endif

static menu_select_t DynamicMenuSelect = {
	0,	/* attribute */
	0,	/* number of items */
	DynamicMenuItem,	/* point to items */
	NULL,	/* point to get item text function */
};

/* ============================================================================ */
//#define _DISCONTINUE_ITEMS	/* In a page, it can show item-9, item-1, item-2 .... */
#define _SCROLL_PAD_TAIL_SPACE	/* Add a space to tail of scrolling text */

typedef unsigned int ri_t, vi_t, cy_t;		/* ri: real index, vi: visual index distance, cy: count y-axis */
typedef unsigned int chi_t, chc_t;			/* chi: character index, chc: character count */

typedef union { 
	struct {
		unsigned long activate:1;
		unsigned long scrolling:1;		/* text horizontal scrolling */
		unsigned long scrollbar:1;		/* vertical scroll bar */
		unsigned long rightsb:1;		/* right side scroll bar */
	} b;
	unsigned long all;
} fSelectionWorking_t;

static fSelectionWorking_t fSelectionWorking = { { 0 } };	CT_ASSERT( sizeof( fSelectionWorking ) == sizeof( fSelectionWorking.all ) );
static menu_select_t msActivatedMenuSelect;

static ri_t riScreenTopItemIndex;
static vi_t viScreenItemIndex;
static cy_t cyScreenRowNum;

#ifdef _DISCONTINUE_ITEMS
???
#define	RiFromVi( vi )
#else
#define RiFromVi( vi )		( riScreenTopItemIndex + ( vi ) )
#endif

/* scrolling assistants */
#define SCROLL_HIGHLIGHT_ITEM_PERIOD	1000			/* 1 second */

#ifdef _SCROLL_PAD_TAIL_SPACE
static unsigned char szScrollHightlightText[ MAX_LENGTH_OF_ITEM_TEXT + 1 + 1 ];
#else
static unsigned char szScrollHightlightText[ MAX_LENGTH_OF_ITEM_TEXT + 1 ];
#endif
static chi_t chiScrollHighlightOffset;
static chc_t chcScrollHighlightLength;
static uptime_t tmScrollHighlightTime;

static void RefreshEntireScreenOfMenuSelection( void );
static void UnHighlightSelectedItem( vi_t viScreenIndex );
static void HighlightSelectedItem( vi_t viScreenIndex, const unsigned char *pszHighlightText );
static void CheckIfNeedToScrollHighlightItem( const unsigned char *pszHighlightText );
static void ScrollHighlightItemText( void );
static const unsigned char *GetItemTextRelayOnItsAttrib( ri_t riIndex, unsigned char *pszBuffer );
static void MoveHighlightSelectedItem( vi_t viOldArrowIndex, vi_t viNewArrowIndex, const unsigned char *pszHighlightText );
static int MenuItemHandleOkKey( vi_t viIndex, ri_t riIndex );

#if MENU_ITEM_INDICATOR == 1
  #undef INVERSE_HIGHLIGHT
#else
  #define INVERSE_HIGHLIGHT	/* inverse highligh item or use a arrow */
#endif

#ifdef _TEXT_MODE
  #define MENU_LOC_X			1
  #define MENU_LOC_Y			0
  #define MENU_WIDTH			( VRAM_WIDTH_IN_TEXT_UNIT - MENU_LOC_X )
  #define MENU_HEIGHT			( VRAM_HEIGHT_IN_TEXT_UNIT - MENU_LOC_Y )
  #define MAX_MENU_ROW_NUM		MENU_HEIGHT
  #define MENU_TEXT_LENGTH		MENU_WIDTH
  #define ARROW_LOC_X			0
#else /* _GRAPH_MODE */
  #ifdef INVERSE_HIGHLIGHT
    #define MENU_LOC_X			0
  #else
    #define MENU_LOC_X			1
  #endif
  #define MENU_LOC_Y			0
  #define MENU_WIDTH			( VRAM_WIDTH_IN_TEXT_UNIT - MENU_LOC_X )
  #ifdef SOFTKEY_SUPPORT
    #define MENU_HEIGHT			( VRAM_HEIGHT_IN_TEXT_UNIT - MENU_LOC_Y - 1 )
  #else
    #define MENU_HEIGHT			( VRAM_HEIGHT_IN_TEXT_UNIT - MENU_LOC_Y )
  #endif
  #define MAX_MENU_ROW_NUM		MENU_HEIGHT
  //#define MAX_MENU_ROW_NUM		( MENU_HEIGHT > 2 ? 2 : MENU_HEIGHT )
  #define MENU_TEXT_LENGTH		MENU_WIDTH
  #define ARROW_LOC_X			0
#endif /* _TEXT_MODE */

#define ITEM_TEXT_LEN_TO_FIT_SCREEN		( VRAM_WIDTH_IN_TEXT_UNIT - MENU_LOC_X )
#define RIGHT_SIDE_SCROLL_BAR	/* use right side scroll bar, or left side */


void ActivateMenuSelection( const menu_select_t *pMenuSelect, unsigned int nDefaultSelection )
{
	ri_t riMaxScreenTop;
#ifdef MENU_ENABLE_SCROLLBAR
	scroll_t scrollbar;
	rect_t rect;
#endif

	/* keep user specified data */
	fSelectionWorking.b.activate = 1;
	msActivatedMenuSelect = *pMenuSelect;

	/* screen layout */
	cyScreenRowNum = MAX_MENU_ROW_NUM;
	
	/* initialize variables */
	if( pMenuSelect ->nNumOfMenuItem <= cyScreenRowNum ) {
		riScreenTopItemIndex = 0;
		viScreenItemIndex = ( ( nDefaultSelection < pMenuSelect ->nNumOfMenuItem ) ?
								 nDefaultSelection : 0 );
	} else {
		riScreenTopItemIndex = ( ( nDefaultSelection < pMenuSelect ->nNumOfMenuItem ) ?
								 nDefaultSelection : 0 );
		viScreenItemIndex = 0;

#ifndef _DISCONTINUE_ITEMS
		riMaxScreenTop = pMenuSelect ->nNumOfMenuItem - cyScreenRowNum;

		if( riScreenTopItemIndex > riMaxScreenTop ) {
			riScreenTopItemIndex = riMaxScreenTop;
			viScreenItemIndex = nDefaultSelection - riScreenTopItemIndex;
		}
#endif
	}
	
	/* scroll bar part */
#ifdef MENU_ENABLE_SCROLLBAR
	if( msActivatedMenuSelect.nNumOfMenuItem > cyScreenRowNum ) {
		scrollbar.total = msActivatedMenuSelect.nNumOfMenuItem - cyScreenRowNum + 1;
		scrollbar.current = riScreenTopItemIndex;

  #ifdef RIGHT_SIDE_SCROLL_BAR	/* right side scroll bar */
		rect.left = VRAM_WIDTH - SCROLL_WIDTH;
		rect.right = VRAM_WIDTH;
		rect.top = MENU_LOC_Y * CHAR_HEIGHT;
		rect.bottom = rect.top + MENU_HEIGHT * CHAR_HEIGHT;

		fSelectionWorking.b.rightsb = 1;	/* right side scroll bar */
  #else	/* left side scroll bar */
		rect.left = 0;
		rect.right = SCROLL_WIDTH;
		rect.top = MENU_LOC_Y * CHAR_HEIGHT;
		rect.bottom = rect.top + MENU_HEIGHT * CHAR_HEIGHT;

		fSelectionWorking.b.rightsb = 0;	/* left side scroll bar */
  #endif
		fSelectionWorking.b.scrollbar = 1;

		ActivateScrollBar( &rect, &scrollbar, fSelectionWorking.b.rightsb );
	} else 
#endif /* MENU_ENABLE_SCROLLBAR */
	{
		fSelectionWorking.b.scrollbar = 0;
	}

	RefreshEntireScreenOfMenuSelection();
}

void ActivateDynamicMenuSelectionWithAttributes( unsigned int nNumberOfItems, unsigned int nDefaultSelection, unsigned long fMenuSelectAttrib )
{
	DynamicMenuSelect.fMenuSelectAttrib = fMenuSelectAttrib;
	DynamicMenuSelect.nNumOfMenuItem = nNumberOfItems;

	ActivateMenuSelection( &DynamicMenuSelect, nDefaultSelection );
}

void ActivateDynamicMenuSelection( unsigned int nNumberOfItems, unsigned int nDefaultSelection )
{
	ActivateDynamicMenuSelectionWithAttributes( nNumberOfItems, nDefaultSelection, 0 );
}

int DeactivateMenuSelection( void )
{
	unsigned int riRetIndex;

	if( !fSelectionWorking.b.activate )
		return 0;

	//fSelectionWorking.b.activate = 0;
	fSelectionWorking.all = 0;

	riRetIndex = riScreenTopItemIndex + viScreenItemIndex;

	if( riRetIndex >= msActivatedMenuSelect.nNumOfMenuItem )
		riRetIndex -= msActivatedMenuSelect.nNumOfMenuItem;
	
	DeactivateScrollBar();

	return riRetIndex;
}

void MenuSelectionTimerEvent( void )
{
	if( !fSelectionWorking.b.activate )
		return;

	if( fSelectionWorking.b.scrolling ) {
		if( CheckIfTimeoutInMillisecond( &tmScrollHighlightTime, SCROLL_HIGHLIGHT_ITEM_PERIOD ) == 0 )
			ScrollHighlightItemText();
	}
	

}

int KeyOwnByMenuSelection( unsigned char key )
{
	rect_t rect;
	unsigned int nTotalItem;
	vi_t viOldArrowIndex;

	if( !fSelectionWorking.b.activate )
		return 0;		/* not my key */

	nTotalItem = msActivatedMenuSelect.nNumOfMenuItem;
	viOldArrowIndex = viScreenItemIndex;
	
	if( key == VKEY_UP ) {
		
		if( nTotalItem == 1 )	/* only one item */
			goto label_feed_this_key;
				
		if( viScreenItemIndex == 0 ) {
			if( nTotalItem <= cyScreenRowNum ) {
				/* move cursor from first item to last one (in the same page) */
				viScreenItemIndex = nTotalItem - 1;
				goto label_move_arrow_to_last_item;
			} else if( riScreenTopItemIndex == 0 ) {
				/* first page -> move to last page */
#ifdef _DISCONTINUE_ITEMS
				riScreenTopItemIndex = nTotalItem - 1;
#else
				riScreenTopItemIndex = nTotalItem - cyScreenRowNum;
				viScreenItemIndex = cyScreenRowNum - 1;
#endif
				goto label_update_entire_screen;
			} else {
				/* move one item up */
				riScreenTopItemIndex --;
				goto label_scroll_down_and_update_entire_screen;
			}
		} else {
			viScreenItemIndex --;
			goto label_move_arrow_up;
		}
	
	} else if( key == VKEY_DOWN ) {

		if( nTotalItem == 1 )	/* only one item */
			goto label_feed_this_key;
		
		if( viScreenItemIndex == cyScreenRowNum - 1 ) {
			if( nTotalItem <= cyScreenRowNum ) {
				/* move cursor from last item to first one (in the same page) */
				viScreenItemIndex = 0;
				goto label_move_arrow_to_first_item;
			} 
				/* last page -> move to first page */			
#ifdef _DISCONTINUE_ITEMS
			else if( riScreenTopItemIndex == nTotalItem - 1 ) {
				riScreenTopItemIndex = 0;
				goto label_update_entire_screen;
			}
#else
			else if( riScreenTopItemIndex == nTotalItem - cyScreenRowNum ) {
				riScreenTopItemIndex = 0;
				viScreenItemIndex = 0;
				goto label_update_entire_screen;
			}
#endif
			else {
				/* move one item down */
				riScreenTopItemIndex ++;
				goto label_scroll_up_and_update_entire_screen;
			}
		} else if( nTotalItem <= cyScreenRowNum && viScreenItemIndex == nTotalItem - 1 ) {
			/* last item, and all items in the same page */
			viScreenItemIndex = 0;
			goto label_move_arrow_to_first_item;
		} else {
			viScreenItemIndex ++;
			goto label_move_arrow_down;
		}
	} else if( key == VKEY_OK ) {
		if( MenuItemHandleOkKey( viScreenItemIndex, riScreenTopItemIndex + viScreenItemIndex ) )
			;
		else
			return 0;
	} else
		return 0;


label_scroll_up_and_update_entire_screen:
label_scroll_down_and_update_entire_screen:
label_update_entire_screen:
	RefreshEntireScreenOfMenuSelection();
	return 1;

	/* update arrow only */
label_move_arrow_up:
label_move_arrow_down:
label_move_arrow_to_last_item:
label_move_arrow_to_first_item:
	
	GS_DrawOffScreen();

#ifdef INVERSE_HIGHLIGHT
	rect.left = MENU_LOC_X * CHAR_WIDTH;
#else
	rect.left = ARROW_LOC_X * CHAR_WIDTH;
#endif

#ifdef INVERSE_HIGHLIGHT
	rect.right = VRAM_WIDTH;
#else
	if( fSelectionWorking.b.scrolling )
		rect.right = VRAM_WIDTH_IN_TEXT_UNIT * CHAR_WIDTH;
	else
		rect.right = ( ARROW_LOC_X + 1) * CHAR_WIDTH;
#endif

	if( viOldArrowIndex > viScreenItemIndex ) {
		rect.top = viScreenItemIndex * CHAR_HEIGHT;
		rect.bottom = ( viOldArrowIndex + 1 ) * CHAR_HEIGHT;
	} else {
		rect.top = viOldArrowIndex * CHAR_HEIGHT;
		rect.bottom = ( viScreenItemIndex + 1 ) * CHAR_HEIGHT;
	}	

	MoveHighlightSelectedItem( viOldArrowIndex, viScreenItemIndex, NULL /* fetch by callee */ );
		
	GS_DrawOnScreen( &rect );

label_feed_this_key:
	return 1;	/* my key */
}

static void MoveHighlightSelectedItem( vi_t viOldArrowIndex, vi_t viNewArrowIndex, const unsigned char *pszHighlightText )
{
	unsigned char szItemTextBuffer[ MAX_LENGTH_OF_ITEM_TEXT + 1 ];

	if( !pszHighlightText )
		pszHighlightText = GetItemTextRelayOnItsAttrib( RiFromVi( viNewArrowIndex ), szItemTextBuffer );

	UnHighlightSelectedItem( viOldArrowIndex );
	HighlightSelectedItem( viNewArrowIndex, pszHighlightText );
}

static void HighlightSelectedItem( vi_t viScreenIndex, const unsigned char *pszHighlightText )
{
#ifdef INVERSE_HIGHLIGHT
	rect_t rect;
#endif

#ifdef INVERSE_HIGHLIGHT
	rect.left = MENU_LOC_X * CHAR_WIDTH;
	rect.right = VRAM_WIDTH;
	rect.top = ( MENU_LOC_Y + viScreenIndex ) * CHAR_HEIGHT;
	rect.bottom = rect.top + CHAR_HEIGHT;

	if( fSelectionWorking.b.scrollbar ) {
		if( fSelectionWorking.b.rightsb )
			rect.right -= SCROLL_WIDTH;
		else
			rect.left += SCROLL_WIDTH;
	}

	GS_DrawSolidRectangleInverse_rect( &rect );
#else
	GS_TextOut( ARROW_LOC_X, MENU_LOC_Y + viScreenIndex, ( unsigned char * )">", 1 );
#endif

	if( pszHighlightText )
		CheckIfNeedToScrollHighlightItem( pszHighlightText );
}

static void UnHighlightSelectedItem( vi_t viScreenIndex )
{
#ifndef _TEXT_MODE
	rect_t rect;

	rect.left = MENU_LOC_X * CHAR_WIDTH;
	rect.right = VRAM_WIDTH;
	rect.top = ( MENU_LOC_Y + viScreenIndex ) * CHAR_HEIGHT;
	rect.bottom = rect.top + CHAR_HEIGHT;
#endif

#ifdef INVERSE_HIGHLIGHT
	if( fSelectionWorking.b.scrollbar ) {
		if( fSelectionWorking.b.rightsb )
			rect.right -= SCROLL_WIDTH;
		else
			rect.left += SCROLL_WIDTH;
	}

	GS_DrawSolidRectangleInverse_rect( &rect );
#endif

	/* restore scrolling text */
	if( fSelectionWorking.b.scrolling ) {
#ifdef _TEXT_MODE
		GS_TextOut( MENU_LOC_X, MENU_LOC_Y + viScreenIndex, szScrollHightlightText, MENU_TEXT_LENGTH );
#else
		GS_DrawText( rect.left, rect.top, szScrollHightlightText, MENU_TEXT_LENGTH );
#endif
	}

	/* clear arrow */
#ifndef INVERSE_HIGHLIGHT
	GS_TextOut( ARROW_LOC_X, MENU_LOC_Y + viScreenIndex, ( unsigned char * )" ", 1 );
#endif
}

static void CheckIfNeedToScrollHighlightItem( const unsigned char *pszHighlightText )
{
	chcScrollHighlightLength = strlen( ( const char * )pszHighlightText );

	if( chcScrollHighlightLength > ITEM_TEXT_LEN_TO_FIT_SCREEN ) {
		fSelectionWorking.b.scrolling = 1;
		chiScrollHighlightOffset = 0;

		if( chcScrollHighlightLength > MAX_LENGTH_OF_ITEM_TEXT )
			chcScrollHighlightLength = MAX_LENGTH_OF_ITEM_TEXT;

		memcpy( szScrollHightlightText, pszHighlightText, chcScrollHighlightLength );

#ifdef _SCROLL_PAD_TAIL_SPACE
		szScrollHightlightText[ chcScrollHighlightLength ++ ] = ' ';
#endif /* _SCROLL_PAD_TAIL_SPACE */

		szScrollHightlightText[ chcScrollHighlightLength ] = '\x0';

		tmScrollHighlightTime = GetUptimeInMillisecond();
	} else
		fSelectionWorking.b.scrolling = 0;
}

static const unsigned char *GetItemTextRelayOnItsAttrib( ri_t riIndex, unsigned char *pszBuffer )
{
	/* pszBuffer used only if SELECT_ATTRIB_GET_TEXT_FUNC, and its size has to be (MAX_LENGTH_OF_ITEM_TEXT+1) */
	unsigned int index;
	unsigned int len;
	isItemStatus_t is;
	unsigned int freeSpace;
	unsigned int t, t2;

	if( msActivatedMenuSelect.fMenuSelectAttrib & SELECT_ATTRIB_GET_TEXT_FUNC ) {
		/* get item text by callback function */
		( *msActivatedMenuSelect.pFnSelectGetItemText )( riIndex, pszBuffer );

		return pszBuffer;
	} else if( msActivatedMenuSelect.pMenuItem[ riIndex ].fMenuItemAttrib & ITEM_ATT_OWNER_HANDLE_OK_KEY ) {
		/* get item text and its status */
		// FIXME: if length is too large.... 
		len = strlen( ( const char * )msActivatedMenuSelect.pMenuItem[ riIndex ].pszItemText );

		if( len > MAX_LENGTH_OF_ITEM_TEXT )
			memcpy( pszBuffer, msActivatedMenuSelect.pMenuItem[ riIndex ].pszItemText, MAX_LENGTH_OF_ITEM_TEXT );
		else
			memcpy( pszBuffer, msActivatedMenuSelect.pMenuItem[ riIndex ].pszItemText, len );
		
		/* get item value */
		is = ( *( pFnItemGetItemStatus_t )msActivatedMenuSelect.pMenuItem[ riIndex ].pFnItemGetItemGeneral )( ITEM_OPER_GET_VALUE );

		/* prepare space for [Y] or [N] */
		freeSpace = VRAM_WIDTH_IN_TEXT_UNIT - 3 - MENU_LOC_X;	/* -3 for '[Y]', -1 for cursor */
																/* MENU_LOC_X == -1 */
		if( is >= ITEM_STATUS_VALUE_10 && is <= ITEM_STATUS_VALUE_99 )
			freeSpace -= 1;
		else if( is >= ITEM_STATUS_VALUE_100 && is <= ITEM_STATUS_VALUE_255 )
			freeSpace -= 2;

		/* move index to draw [xxx] */
		if( len > freeSpace )
			index = freeSpace;	
		else {
			for( index = len; index < freeSpace; index ++ )
				pszBuffer[ index ] = ' ';
		}

		pszBuffer[ index ++ ] = '[';

		switch( is ) {
		case ITEM_STATUS_FALSE:
			pszBuffer[ index ++ ] = 'N';
			break;

		case ITEM_STATUS_TRUE:
			pszBuffer[ index ++ ] = 'Y';
			break;

		default:
			if( is >= ITEM_STATUS_VALUE_0 && is <= ITEM_STATUS_VALUE_9 ) {
				pszBuffer[ index ++ ] = '0' + is - ITEM_STATUS_VALUE_0;
			} else if( is >= ITEM_STATUS_VALUE_10 && is <= ITEM_STATUS_VALUE_99 ) {
				t = ( is - ITEM_STATUS_VALUE_0 ) / 10;
				pszBuffer[ index ++ ] = '0' + t;
				pszBuffer[ index ++ ] = '0' + ( is - ITEM_STATUS_VALUE_0 ) - t * 10;
			} else if( is >= ITEM_STATUS_VALUE_100 && is <= ITEM_STATUS_VALUE_255 ) {
				t = ( is - ITEM_STATUS_VALUE_0 ) / 100;
				pszBuffer[ index ++ ] = '0' + t;
				t2 = ( ( is - ITEM_STATUS_VALUE_0 ) - t * 100 ) / 10;
				pszBuffer[ index ++ ] = '0' + t2;
				pszBuffer[ index ++ ] = '0' + ( is - ITEM_STATUS_VALUE_0 ) - t * 100 - t2 * 10;
			} else
				pszBuffer[ index ++ ] = '*';
			break;
		}

		pszBuffer[ index ++ ] = ']';
		pszBuffer[ index ++ ] = '\x0';

		return pszBuffer;
	} else if( msActivatedMenuSelect.pMenuItem[ riIndex ].fMenuItemAttrib & ITEM_ATT_GET_TEXT_FUNC ) {
		/* get item text through callback function */
		( *( pFnItemGetItemText_t )msActivatedMenuSelect.pMenuItem[ riIndex ].pFnItemGetItemGeneral )( pszBuffer );
		
		return pszBuffer;
	} else {
		/* get item text from static table */
		return ( &( msActivatedMenuSelect.pMenuItem[ riIndex ] ) ) ->pszItemText;
	}
}

static int MenuItemHandleOkKey( vi_t viIndex, ri_t riIndex )
{
	unsigned char szMenuItem[ MAX_LENGTH_OF_ITEM_TEXT + 1 ];
	const unsigned char *pszItem;
#ifndef _TEXT_MODE
	int x;
#endif

	/* ITEM_ATT_OWNER_HANDLE_OK_KEY?? */
	if( ( msActivatedMenuSelect.pMenuItem == NULL ) ||
		( ( msActivatedMenuSelect.pMenuItem[ riIndex ].fMenuItemAttrib & 
														ITEM_ATT_OWNER_HANDLE_OK_KEY ) == 0 ) )
	{
		return 0;
	}

	/* switch value */
	if( ( *( pFnItemGetItemStatus_t )msActivatedMenuSelect.pMenuItem[ riIndex ].pFnItemGetItemGeneral )( ITEM_OPER_SWITCH_VALUE )
		== ITEM_STATUS_NOT_FEED_KEY )
	{
		return 0;
	}
	
	/* get text */
	pszItem = GetItemTextRelayOnItsAttrib( riIndex, szMenuItem );

	/* draw */
#ifdef _TEXT_MODE
	GS_TextOut( MENU_LOC_X, MENU_LOC_Y + viIndex, pszItem, strlen( ( const char * )pszItem ) );
#else
	x = MENU_LOC_X * CHAR_WIDTH;

	if( fSelectionWorking.b.scrollbar && !fSelectionWorking.b.rightsb )
		x += SCROLL_WIDTH;

	GS_DrawText( x, ( MENU_LOC_Y + viIndex ) * CHAR_HEIGHT, 
					pszItem, strlen( ( const char * )pszItem ) );
#endif

	return 1;
}

static void RefreshEntireScreenOfMenuSelection( void )
{
	cy_t cyRowNumToScreen;
	ri_t riIndex;
	cy_t cyI;
	int nTextLen;
	unsigned int nTotalItem;
	unsigned char szItemTextBuffer[ MAX_LENGTH_OF_ITEM_TEXT + 1 ];
	const unsigned char *pszItemText;
#ifndef _TEXT_MODE
	int x;
#endif

	GS_DrawOffScreenAndClearScreen();
	
	/* how many row to show on screen */
	cyRowNumToScreen = msActivatedMenuSelect.nNumOfMenuItem;
	if( cyRowNumToScreen > cyScreenRowNum )
		cyRowNumToScreen = cyScreenRowNum;

	nTotalItem = msActivatedMenuSelect.nNumOfMenuItem;	

	for( cyI = 0, riIndex = riScreenTopItemIndex; 
							cyI < cyRowNumToScreen; cyI ++, riIndex ++ ) 
	{
		if( riIndex == nTotalItem )
			riIndex = 0;

		/* pszItemText may equal to szItemTextBuffer, or point to a const string */
		pszItemText = GetItemTextRelayOnItsAttrib( riIndex, szItemTextBuffer );

		/* fit text length */
		nTextLen = strlen( ( char * )pszItemText );
		if( nTextLen > MENU_TEXT_LENGTH )
			nTextLen = MENU_TEXT_LENGTH;

		/* draw text */
#ifdef _TEXT_MODE
		GS_TextOut( MENU_LOC_X, MENU_LOC_Y + cyI, pszItemText, nTextLen );
#else
		x = MENU_LOC_X * CHAR_WIDTH;

		if( fSelectionWorking.b.scrollbar && !fSelectionWorking.b.rightsb )
			x += SCROLL_WIDTH;

		GS_DrawText( x, ( MENU_LOC_Y + cyI ) * CHAR_HEIGHT, pszItemText, nTextLen );
#endif

		/* highlight this one */
		if( cyI == viScreenItemIndex )
			HighlightSelectedItem( viScreenItemIndex, pszItemText );
	}

	switch( msActivatedMenuSelect.fMenuSelectAttrib & SELECT_ATTRIB_SOFTKEY_MASK ) {
	case SELECT_ATTRIB_SOFTKEY_OK_BACK:
		SoftkeyStandardConfiguration_OkBack();
		break;

	case SELECT_ATTRIB_SOFTKEY_DETAIL_BACK:
		SoftkeyStandardConfiguration_DetailBack();
		break;

	case SELECT_ATTRIB_SOFTKEY_BACK:
		SoftkeyStandardConfiguration_Back();
		break;
	}

	SetCurrentAndDrawScrollBar( riScreenTopItemIndex );

	GS_DrawOnScreen( NULL );
}

static void ScrollHighlightItemText()
{
	int lenFirstPart;
#ifndef _TEXT_MODE
	rect_t rect;

	rect.left = MENU_LOC_X * CHAR_WIDTH;
	rect.right = VRAM_WIDTH;
	rect.top = ( MENU_LOC_Y + viScreenItemIndex ) * CHAR_HEIGHT;
	rect.bottom = rect.top + CHAR_HEIGHT;
#endif

	GS_DrawOffScreen();

#ifdef INVERSE_HIGHLIGHT
	if( fSelectionWorking.b.scrollbar ) {
		if( fSelectionWorking.b.rightsb )
			rect.right -= SCROLL_WIDTH;
		else
			rect.left += SCROLL_WIDTH;
	}

	GS_DrawSolidRectangleInverse_rect( &rect );
#endif

	if( ++ chiScrollHighlightOffset >= chcScrollHighlightLength )
		chiScrollHighlightOffset = 0;

	if( chiScrollHighlightOffset + MENU_TEXT_LENGTH <= chcScrollHighlightLength ) {
		/* draw continue text */
#ifdef _TEXT_MODE
		GS_TextOut( MENU_LOC_X, MENU_LOC_Y + viScreenItemIndex, 
					 szScrollHightlightText + chiScrollHighlightOffset, MENU_TEXT_LENGTH );
#else
		GS_DrawText( rect.left, rect.top, 
					 szScrollHightlightText + chiScrollHighlightOffset, MENU_TEXT_LENGTH );
#endif
	} else {
		/* draw a separate text */
		lenFirstPart = chcScrollHighlightLength - chiScrollHighlightOffset;
#ifdef _TEXT_MODE
		GS_TextOut( MENU_LOC_X, MENU_LOC_Y + viScreenItemIndex, 
					 szScrollHightlightText + chiScrollHighlightOffset, lenFirstPart );
		GS_TextOut( MENU_LOC_X + lenFirstPart, MENU_LOC_Y + viScreenItemIndex, 
					 szScrollHightlightText, MENU_TEXT_LENGTH - lenFirstPart );
#else
		GS_DrawText( rect.left, rect.top, 
					 szScrollHightlightText + chiScrollHighlightOffset, lenFirstPart );
		GS_DrawText( rect.left + lenFirstPart * CHAR_WIDTH, rect.top, 
					 szScrollHightlightText, MENU_TEXT_LENGTH - lenFirstPart );
#endif
	}

#ifdef INVERSE_HIGHLIGHT
	GS_DrawSolidRectangleInverse_rect( &rect );
#endif

	GS_DrawOnScreen( NULL );
}
