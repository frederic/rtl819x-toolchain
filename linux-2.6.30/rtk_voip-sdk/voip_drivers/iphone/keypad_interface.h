#ifndef __KEYPAD_INTERFACE_H__
#define __KEYPAD_INTERFACE_H__

/* ----------------------------------------------------------- */
/* For user space ioctl */
/* ----------------------------------------------------------- */

/* handle user space ioctl */
extern void do_keypad_opt_ctl( void *pUser, unsigned int len );

/* ----------------------------------------------------------- */
/* For keypad debouncing V2 */
/* ----------------------------------------------------------- */

/*
 * Keyscan and hook detection may be interrupt or polling mode. 
 * They can summerize as following:
 *
 *       keyscan       hook         key_num for hook polling 
 *       POLLING       POLLING		count of keyscan_core()
 *       INT           POLLING		KEY_NUM_HOOK_ONLY
 *       INT           INT			count of keyscan_core()
 */

typedef enum {
	DBST_INIT,			/* initial state */
	DBST_NORMAL_PRESS,
	DBST_LONG_PRESS,
	DBST_MULTI_PRESS,
	DBST_DONE,
} dbstate_t;

/* After driver do keyscan, it call this routine */
/* 
 * NOTE: In some case, we may do hook polling only, so we fill 
 *       key_num with KEY_NUM_HOOK_ONLY. 
 *       An example is that keyscan is in interrupt mode, but hook isn't.
 */
#define KEY_NUM_HOOK_ONLY	9999

extern dbstate_t keypad_scan_result( unsigned int key_num, unsigned char key1 );

/* In interrupt mode, it may stop timer and enable interrupt by this */
extern int check_keypad_debounce_complete( void );
extern int check_hook_debounce_complete( void );

#endif /* __KEYPAD_INTERFACE_H__ */

