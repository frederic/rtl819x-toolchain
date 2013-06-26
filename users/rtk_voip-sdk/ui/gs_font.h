#ifndef __GS_FONT_H__
#define __GS_FONT_H__

#ifndef _TEXT_MODE

typedef enum {
	ENG_FONT_ID_NORMAL,
	ENG_FONT_ID_VARIABLE,
	NUM_OF_ENG_FONT_ID,
} eng_font_id_t;

typedef enum {
	CHN_FONT_ID_NORMAL,
	CHN_FONT_ID_SMALL,
	NUM_OF_CHN_FONT_ID,
} chn_font_id_t;

extern void GS_InitializeFont( void );
extern const unsigned char *GS_GetFontBitmap( const unsigned char *pch, int *pnCh, int *pnWidth, int *pnHeight );

extern eng_font_id_t GS_SelectEngFont( eng_font_id_t id );
extern void GS_GetEngFontMaxSize( int *pWidth, int *pHeight );
extern void GS_GetChnFontMaxSize( int *pWidth, int *pHeight );
extern void GS_GetFontMaxSize( int *pWidth, int *pHeight );

#if defined( LANG_BIG5 ) || defined( LANG_GB2312 )
extern chn_font_id_t GS_SelectChnFont( chn_font_id_t id );
#else
static inline chn_font_id_t GS_SelectChnFont( chn_font_id_t id )	{ return CHN_FONT_ID_NORMAL; }
#endif

#else
#define GS_InitializeFont()
#endif /* !_TEXT_MODE */

#endif /* __GS_FONT_H__ */
