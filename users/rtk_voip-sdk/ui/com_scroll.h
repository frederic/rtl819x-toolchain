#ifndef __COM_SCROLL_H__
#define __COM_SCROLL_H__

#define SCROLL_WIDTH	5		/* scroll bar occupies 5 pixels */

typedef struct {
	unsigned int total;
	unsigned int current;
	/* 
	 * It assumes that there is only one item in screen, so if your screen has more than one 
	 * item, please let total minus items of screen and plus one. 
	 */
} scroll_t;

#ifdef MENU_ENABLE_SCROLLBAR

extern void ActivateScrollBar( const rect_t *pRect, const scroll_t *pScroll, int bRightSide );

extern void DeactivateScrollBar( void );

extern void DrawEntireScrollBar( void );

extern void RefreshEntireScrollBar( void );

extern void SetCurrentAndDrawScrollBar( unsigned int current );

#else

#define ActivateScrollBar( pRect, pScroll, bRightSide )		

#define DeactivateScrollBar()		

#define DrawEntireScrollBar()		

#define RefreshEntireScrollBar()		

#define SetCurrentAndDrawScrollBar( current )		

#endif


#endif /* __COM_SCROLL_H__ */
