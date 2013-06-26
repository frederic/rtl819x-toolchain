#ifndef __COM_SELECT_H__
#define __COM_SELECT_H__

#define MAX_LENGTH_OF_ITEM_TEXT			50	/* especially used for scrolling */

#define MAX_NUM_OF_DYNAMIC_MENU_ITEM			16
#define MAX_LEN_OF_DYNAMIC_MENU_ITEM_TEXT		20	/* should smaller than 50 (exclusive null terminator) */

/* menu item */
typedef enum {
	ITEM_STATUS_VALUE_0 = 0,		/* 0-255 are reserved for item value */
	ITEM_STATUS_VALUE_1 = 1,
	ITEM_STATUS_VALUE_2 = 2,
	ITEM_STATUS_VALUE_3 = 3,
	ITEM_STATUS_VALUE_4 = 4,
	ITEM_STATUS_VALUE_5 = 5,
	ITEM_STATUS_VALUE_6 = 6,
	ITEM_STATUS_VALUE_7 = 7,
	ITEM_STATUS_VALUE_8 = 8,
	ITEM_STATUS_VALUE_9 = 9,
	ITEM_STATUS_VALUE_10 = 10,
	ITEM_STATUS_VALUE_99 = 99,
	ITEM_STATUS_VALUE_100 = 100,
	ITEM_STATUS_VALUE_255 = 255,	/* 0-255 are reserved for item value */
	ITEM_STATUS_NOT_FEED_KEY = 0xFFE0,	/* key will process by state key processor */
	ITEM_STATUS_FALSE,
	ITEM_STATUS_TRUE,
} isItemStatus_t;

typedef enum {
	ITEM_OPER_GET_VALUE,
	ITEM_OPER_SWITCH_VALUE,
} ioItemOperate_t;

typedef isItemStatus_t ( *pFnItemGetItemStatus_t )( ioItemOperate_t ioItemOperate );
typedef void ( *pFnItemGetItemText_t )( unsigned char *pszItemText );

typedef void * pFnGeneral_t;

#define ITEM_ATT_OWNER_HANDLE_OK_KEY	0x00000001	/* bit 0: use callback function to handle 'OK' key -> pFnItemGetItemStatus_t */
#define ITEM_ATT_GET_TEXT_FUNC			0x00000002	/* bit 1: use callback function to get item text -> pFnItemGetItemText_t */

typedef struct menu_item_s {
	const unsigned char *	pszItemText;
	unsigned long			fMenuItemAttrib;
	pFnGeneral_t			pFnItemGetItemGeneral;	/* It is possible cast to pFnItemGetItemStatus_t or pFnItemGetItemText_t */
} menu_item_t;	/* Its instances have to be CONST */

/* menu select */
typedef void ( *pFnSelectGetItemText_t )( unsigned int idxItem, unsigned char *pszItemText );

#define SELECT_ATTRIB_GET_TEXT_FUNC			0x00000001	/* bit 0: use callback function to get item text */
#define SELECT_ATTRIB_SOFTKEY_MASK			0x000000F0	/* bit 4-7: softkey */
#define SELECT_ATTRIB_SOFTKEY_OK_BACK		0x00000000	/* bit 4-7=0, [OK, Back] */
#define SELECT_ATTRIB_SOFTKEY_DETAIL_BACK	0x00000010	/* bit 4-7=1, [Detail, Back] */
#define SELECT_ATTRIB_SOFTKEY_BACK			0x00000020	/* bit 4-7=2, [, Back] */

typedef struct menu_select_s {
	unsigned long				fMenuSelectAttrib;
	unsigned int				nNumOfMenuItem;
	const menu_item_t *			pMenuItem;
	pFnSelectGetItemText_t		pFnSelectGetItemText;
} menu_select_t;

extern void ActivateMenuSelection( const menu_select_t *pMenuSelect, unsigned int nDefaultSelection );
extern void ActivateDynamicMenuSelection( unsigned int nNumberOfItems, unsigned int nDefaultSelection );
extern void ActivateDynamicMenuSelectionWithAttributes( unsigned int nNumberOfItems, unsigned int nDefaultSelection, unsigned long fMenuSelectAttrib );
extern int DeactivateMenuSelection( void );

extern menu_item_t DynamicMenuItem[ MAX_NUM_OF_DYNAMIC_MENU_ITEM ];
extern unsigned char DynamicMenuItemText[ MAX_NUM_OF_DYNAMIC_MENU_ITEM ][ MAX_LEN_OF_DYNAMIC_MENU_ITEM_TEXT + 1 ];

#endif /* __COM_SELECT_H__ */

