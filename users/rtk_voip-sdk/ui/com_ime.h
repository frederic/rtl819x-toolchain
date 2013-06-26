#ifndef __COM_IME_H__
#define __COM_IME_H__

typedef enum ime_id_s {
	IME_ID_ENGLISH,
#if defined( LANG_BIG5 ) || defined( LANG_GB2312 )
	IME_ID_PINYIN,
#endif
#ifdef LANG_BIG5
	IME_ID_PHONETIC,
#endif
#ifdef LANG_GB2312
	IME_ID_WUBIHUA,
#endif
	//IME_ID_UPPER_ENG,
	//IME_ID_LOWER_ENG,
	NUM_OF_INPUT_METHOD_EDITOR,
} ime_id_t;

extern void ActivateInputMethodEditor( ime_id_t ime );
extern void DeactivateInputMethodEditor( void );

#endif /* __COM_IME_H__ */
