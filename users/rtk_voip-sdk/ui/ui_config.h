#ifndef __UI_CONFIG_H__
#define __UI_CONFIG_H__

#include "kernel_config.h"
#include "rtk_voip.h"

/* ------------------------------------------------------------ */
/* Version Declare */
#define SOFTWARE_VERSION			0x00010000
#define FLASH_VERSION				0x00010000
#define FLASH_SIGNATURE				0xa26ae567
#define UI_SOFT_VER					"1.0"	/* for menu display */

#define _DEBUG_MODE					/* undef it in release version */

/* ------------------------------------------------------------ */
/* Vendor Declare */
#define VENDOR_WCO					1
#define VENDOR_TCO					2
#define VENDOR_WCO2					3
#define VENDOR_BCO					4

#if defined( CONFIG_RTK_VOIP_GPIO_IPP_100 ) || defined( CONFIG_RTK_VOIP_GPIO_IPP_101 )
  #define KEYPAD_MAP_VENDOR			VENDOR_WCO
  #define LED_MAP_VENDOR			VENDOR_WCO
#elif defined( CONFIG_RTK_VOIP_GPIO_IPP_8972_V00 ) || defined( CONFIG_RTK_VOIP_GPIO_IPP_8972_V01 )
  #define KEYPAD_MAP_VENDOR			VENDOR_TCO
  #define LED_MAP_VENDOR			VENDOR_TCO
#elif defined( CONFIG_RTK_VOIP_GPIO_IPP_8952_V00 )
  #define KEYPAD_MAP_VENDOR			VENDOR_WCO2
  #define LED_MAP_VENDOR			VENDOR_WCO2
#elif defined( CONFIG_RTK_VOIP_GPIO_IPP_8972B_V00 )
  #define KEYPAD_MAP_VENDOR			VENDOR_BCO
  #define LED_MAP_VENDOR			VENDOR_BCO
#endif

/* ------------------------------------------------------------ */
/* Time Configuration */
#define SHOW_PROMPT_PERIOD			2000	/* 2 seconds */
#define DISCONNECT_PROMPT_PERIOD	5000	/* 5 seconds */
#define KEYPRESS_TONE_PERIOD		100		/* 0.1 second */
#define LED_BLINKING_PERIOD			500		/* 0.5 second */
#define IDLE_PERIOD_TO_DO_LONG_JOB	5000	/* 5 seconds */

/* ------------------------------------------------------------ */
/* Configuration */
#define _NEW_FXO_DESIGN				/* FXO and FXS play the same roles */

/* ------------------------------------------------------------ */
/* LCD definition */
#define LCD_MODULE_ID_MTB_F000132MNNSAR		1		/* 16 x 2 characters */
#define LCD_MODULE_ID_HT1650				2		/* 64 x 32 pixels */
#define LCD_MODULE_ID_RTS26151B				3		/* 128 x 64 pixels */
#define LCD_MODULE_ID_NT7534				4		/* 132 x 65 pixels */
#define LCD_MODULE_ID_EPL65132				5		/* 132 x 65 pixels */
#define LCD_MODULE_ID_SPLC780D				6		/* 16 x 2 characters */
#define LCD_MODULE_ID_xxxx					7

#if defined( CONFIG_RTK_VOIP_GPIO_IPP_100 ) || defined( CONFIG_RTK_VOIP_GPIO_IPP_101 )
  #define LCD_MODULE_ID				LCD_MODULE_ID_MTB_F000132MNNSAR
#elif defined( CONFIG_RTK_VOIP_GPIO_IPP_8972_V00 ) || defined( CONFIG_RTK_VOIP_GPIO_IPP_8972_V01 )
  #define LCD_MODULE_ID				LCD_MODULE_ID_HT1650
#elif defined( CONFIG_RTK_VOIP_GPIO_IPP_8952_V00 )
  #define LCD_MODULE_ID				LCD_MODULE_ID_EPL65132
#elif defined( CONFIG_RTK_VOIP_GPIO_IPP_8972B_V00 )
  #define LCD_MODULE_ID				LCD_MODULE_ID_SPLC780D
#endif

#if LCD_MODULE_ID == LCD_MODULE_ID_MTB_F000132MNNSAR || LCD_MODULE_ID == LCD_MODULE_ID_SPLC780D
  #define _HARDWARE_CURSOR
  #define _TEXT_MODE						/* Not support graphic function */
  #define MENU_ITEM_INDICATOR		1	/* use '>' as indicator. graphic mode can inverse background */
  #undef  MENU_ENABLE_SCROLLBAR
  #define CHAR_WIDTH				1
  #define CHAR_HEIGHT				1
  #define VRAM_WIDTH				16
  #define VRAM_HEIGHT				2
  #define VRAM_WIDTH_IN_TEXT_UNIT	VRAM_WIDTH
  #define VRAM_HEIGHT_IN_TEXT_UNIT	VRAM_HEIGHT
  #undef _VRAM_MMAP
#elif LCD_MODULE_ID == LCD_MODULE_ID_HT1650
  #undef _HARDWARE_CURSOR		/* No hardware cursor */
  #undef _TEXT_MODE				/* Support graphic function */
  #define LCD_COL_ORIENTED		/* column oriented (type 1) */
  #define LCD_BPP					1	/* 1 bit per pixel */
  #define MENU_ITEM_INDICATOR		1	/* use '>' as indicator. graphic mode can inverse background */
  #undef  MENU_ENABLE_SCROLLBAR
  #define CHAR_WIDTH				5	/* English character width */
  #define CHAR_HEIGHT				8	/* English character height */
  #define VRAM_WIDTH_IN_TEXT_UNIT	16
  #define VRAM_HEIGHT_IN_TEXT_UNIT	2
  #define VRAM_WIDTH				( VRAM_WIDTH_IN_TEXT_UNIT * CHAR_WIDTH )
  #define VRAM_HEIGHT				( VRAM_HEIGHT_IN_TEXT_UNIT * CHAR_HEIGHT )
  #undef _VRAM_MMAP
#elif LCD_MODULE_ID == LCD_MODULE_ID_RTS26151B
  #undef _HARDWARE_CURSOR		/* No hardware cursor */
  #undef _TEXT_MODE				/* Support graphic function */
  #define LCD_ROW_ORIENTED		/* row oriented */
  #define LCD_BPP					1	/* 1 bit per pixel */
  #define MENU_ITEM_INDICATOR		1	/* use '>' as indicator. graphic mode can inverse background */
  #define MENU_ENABLE_SCROLLBAR
  #define CHAR_WIDTH				8	/* English character width */
  #define CHAR_HEIGHT				16	/* English character height */
  #define VRAM_WIDTH_IN_TEXT_UNIT	16
  #define VRAM_HEIGHT_IN_TEXT_UNIT	4
  #define VRAM_WIDTH				( VRAM_WIDTH_IN_TEXT_UNIT * CHAR_WIDTH )
  #define VRAM_HEIGHT				( VRAM_HEIGHT_IN_TEXT_UNIT * CHAR_HEIGHT )
  #undef _VRAM_MMAP
#elif LCD_MODULE_ID == LCD_MODULE_ID_NT7534 || LCD_MODULE_ID == LCD_MODULE_ID_EPL65132
  #undef _HARDWARE_CURSOR		/* No hardware cursor */
  #undef _TEXT_MODE				/* Support graphic function */
  #define LCD_COL2_ORIENTED		/* column oriented (type 2) */
  #define LCD_BPP					1	/* 1 bit per pixel */
  #define MENU_ITEM_INDICATOR		0	/* use '>' as indicator. graphic mode can inverse background */
  #define MENU_ENABLE_SCROLLBAR
  #define CHAR_WIDTH				8	/* English character width */
  #define CHAR_HEIGHT				16	/* English character height */
  #define VRAM_WIDTH_IN_TEXT_UNIT	16
  #define VRAM_HEIGHT_IN_TEXT_UNIT	4
  #define VRAM_WIDTH				132
  #define VRAM_HEIGHT				65
  #define VRAM_MMAP					2	/* mmap type 2 (associated with LCD_COL2_ORIENTED) */
  #define VRAM_MMAP_SIZE			4096	/* must be power of 4k */
  //#define LANG_BIG5
  #define LANG_GB2312
#elif LCD_MODULE_ID == LCD_MODULE_ID_xxxx
	???
#else
	???
#endif /* LCD_MODULE_ID */

/* ------------------------------------------------------------ */
/* Keypad definition */
#if KEYPAD_MAP_VENDOR == VENDOR_WCO2
  #define SOFTKEY_SUPPORT			/* Using softkey has to satisfy with key and large LCD. */
#endif

/* ------------------------------------------------------------ */
/* LED definition */
#if LED_MAP_VENDOR == VENDOR_BCO
  #define NO_LINE_LED				/* There is no line LED indicator */
#endif

/* ------------------------------------------------------------ */
/* Assistant macro */
#ifdef _DEBUG_MODE
  #define debug_out			printf
#else
  static inline void debug_out( const char *format, ... ) {}
#endif

#ifndef CT_ASSERT
#define CT_ASSERT( expr )	extern char ct_assert[ 2 * ( expr ) - 1 ]
#endif

#endif /* __UI_CONFIG_H__ */

