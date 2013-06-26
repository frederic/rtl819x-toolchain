#ifndef __IOCTL_KERNEL_H__

// FIXME: Which chid is belong to IP phone? dynamically?? 
#define IP_CHID		0
#define IP_SID		0

#include "voip_manager.h"

/* *********************************************************** */
/* Keypad function */
/* *********************************************************** */
/* Ask keypad driver to send a SIGUSR1 signal to this process. */
extern int rtk_SetKeypadSetTarget( void );

/* Ask keypad driver to unset target */
extern int rtk_SetKeypadUnsetTarget( void );

/* Debug purpose: make kernel signal and place a key in its buffer. */
extern int rtk_SetKeypadSignalDebug( wkey_t wkey );

/* Read key when we receive a signal */
extern int rtk_SetKeypadReadKey( wkey_t *pWKey );

/* Get hook status */
extern int rtk_GetKeypadHookStatus( void );

/* ************************************************************* */
/* PCM Interface */
/* ************************************************************* */
/* Enable / Disable PCM */
extern int rtk_eanblePCM(unsigned int chid, unsigned int val);

/* *********************************************************** */
/* LCD function */
/* *********************************************************** */

/* Display on/off, cursor on/off, cursor blink */
extern int rtk_SetLcdDisplayOnOff( unsigned char bDisplayOnOff,
							unsigned char bCursorOnOff,
							unsigned char bCursorBlink );

/* Move cursor position */
extern int rtk_SetLcdMoveCursorPosition( int x, int y );

/* Draw text (Textual mode) */
extern int rtk_SetLcdDrawText( int x, int y, unsigned char *pszText, int len );

/* Write Data (Graphic mode) */
extern int rtk_SetLcdWriteData( int start, const unsigned char *pdata, int len );

/* Write Data (Graphic mode & col2 type)*/
extern int rtk_SetLcdWriteData2( int page, int col, const unsigned char *pdata, int len );

/* Dirty MMAP */
extern int rtk_SetLcdDirtyMmap( int start, int len );

/* Dirty MMAP2 */
extern int rtk_SetLcdDirtyMmap2( int page, int col, int len, int rows );


/* ************************************************************* */
/* LED interface  */
/* ************************************************************* */
/* Set LED on/off */
extern int rtk_SetLedOnOff( unsigned long led );

/* ************************************************************* */
/* Codec interface  */
/* ************************************************************* */
/* Set codec register */
extern int rtk_Set_IPhone(unsigned int function_type, unsigned int reg, unsigned int value);

/* Set voice path */
//extern int rtk_SetVoicePath( vocpath_t vocpath );

/* ************************************************************* */
/* Misc interface  */
/* ************************************************************* */
extern int rtk_RetrieveMiscInfo( unsigned long *buildno, unsigned long *builddate );

#endif /* __IOCTL_KERNEL_H__ */

