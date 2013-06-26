#include <stdio.h>
#include "ui_config.h"
#include "gs_types.h"
#include "gs_shape.h"
#include "gs_lib.h"
#include "com_scroll.h"

#ifdef MENU_ENABLE_SCROLLBAR

static struct {
	struct {
		unsigned int active:1;			/* enable scroll bar */
		unsigned int rightSide:1;		/* place on right or left side */
	} b;
	rect_t rectUpdate;	/* scroll bar rectangle for updating */
	rect_t rectClear;
	rect_t rectFrame;
	scroll_t n;
} varScroll;


void ActivateScrollBar( const rect_t *pRect, const scroll_t *pScroll, int bRightSide )
{
	if( pRect ->left + SCROLL_WIDTH != pRect ->right ) {
		debug_out( "Invalid scrollbar rectangle!!\n" );
		return;
	}

	if( pScroll ->total > pScroll ->current ) {
	} else {
		debug_out( "Invalid scrollbar range!!\n" );
		return;
	}

	varScroll.rectUpdate = *pRect;
	varScroll.rectClear = *pRect;		varScroll.rectClear.right --;	varScroll.rectClear.bottom --;
	varScroll.rectFrame = *pRect;		varScroll.rectFrame.right --;	varScroll.rectFrame.bottom --;
	varScroll.n = *pScroll;

	varScroll.b.rightSide = bRightSide;

	if( varScroll.b.rightSide )
		varScroll.rectFrame.left ++;
	else
		varScroll.rectFrame.right --;
	
	varScroll.b.active = 1;
}

void DeactivateScrollBar( void )
{
	varScroll.b.active = 0;
}

void DrawEntireScrollBar( void )
{
	int nTotalPixels;
	int nTickPixels;
	int nTickY;

	if( varScroll.b.active == 0 )
		return;

	/* clear scroll bar region */
	GS_DrawSolidRectangle_rect( &varScroll.rectClear, 0 );

	/* draw frame */
	GS_DrawRectangle_rect( &varScroll.rectFrame );

	/* draw tick */
	nTotalPixels = varScroll.rectFrame.bottom - varScroll.rectFrame.top - 1;
	nTickPixels = nTotalPixels / varScroll.n.total;
	if( nTickPixels == 0 )
		nTickPixels = 1;

	if( varScroll.n.current == varScroll.n.total - 1 )
		nTickY = nTotalPixels - nTickPixels;
	else
		nTickY = nTotalPixels * varScroll.n.current / varScroll.n.total;

	GS_DrawVerticalLine( varScroll.rectFrame.left + 1, varScroll.rectFrame.top + 1 + nTickY, nTickPixels, 1 );
	GS_DrawVerticalLine( varScroll.rectFrame.left + 2, varScroll.rectFrame.top + 1 + nTickY, nTickPixels, 1 );
}

void RefreshEntireScrollBar( void )
{
	GS_DrawOffScreen();

	DrawEntireScrollBar();

	GS_DrawOnScreen( &varScroll.rectUpdate );
}

void SetCurrentAndDrawScrollBar( unsigned int current )
{
	if( varScroll.n.total > current ) 
		;
	else {
		debug_out( "Incorrect 'current' setting\n" );
		return;
	}

	varScroll.n.current = current;
	DrawEntireScrollBar();
}


#endif /* !_TEXT_MODE */
