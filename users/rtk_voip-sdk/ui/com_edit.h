#ifndef __COM_EDIT_H__
#define __COM_EDIT_H__

/* Editor Selection */
typedef enum editor_id_s {
	EDITOR_ID_SINGLE_LINE,
//	EDITOR_ID_MULTI_LINE,
//	EDITOR_ID_IP,
	NUM_OF_TEXT_EDITOR,
} editor_id_t;

/* Edit Command */
typedef enum editor_cmd_s {
	EDITOR_CMD_NORMAL,		/* add text to buffer */
	/* for wide-char */
	EDITOR_CMD_NOT_REFRESH,	/* refresh until receive next character */
	/* on spot editing */
	EDITOR_CMD_CANDIDATE,	/* this text is a candidate */
	EDITOR_CMD_REPLACE,		/* replace candidate */
	EDITOR_CMD_ACCEPT,		/* accept candidate (no text) */
	EDITOR_CMD_CANCEL,		/* cancel candidate (no text) */
} editor_cmd_t;

/* Parameters for editors */
typedef enum text_type_s {
	TEXT_TYPE_NORMAL,	/* accept all keys and text from IME */
	TEXT_TYPE_BCD,		/* accept 0 ~ 9 and * # only */
	TEXT_TYPE_IP,		/* accept 0 ~ 9 and * interpret as . */	/* normal text instead of BCD */
} text_type_t;

typedef union params_flags_s {
	struct {
		unsigned char keyBypass:1;	/* key is own by editor, but still bypass */
	} b;
	unsigned long all;
} params_flags_t;

typedef struct params_for_single_line_editor_s {
	text_type_t			tTextType;
	int					nDefaultTextLength;
	unsigned char *		pszDefaultText;
	int					nMaxTextLength;		/* should less than MAX_LENGTH_OF_SINGLE_LINE_EDITOR */
	params_flags_t		fParams;
} params_for_single_line_editor_t;

typedef union params_for_editor_s {
	params_for_single_line_editor_t single_line;
} params_for_editor_t;

#define MAX_LENGTH_OF_SINGLE_LINE_EDITOR		15	/* exclusive null terminator */

extern void ActivateTextEditor( editor_id_t editor_id, const params_for_editor_t *pParams );
extern void DeactivateTextEditor( void );
extern int  SendTextToTextEditor( unsigned char text, editor_cmd_t cmd );
extern int	SendWideTextToTextEditor( unsigned char text1, unsigned char text2 );
extern int  KeyOwnByTextEditor( unsigned char key );
extern int  GetTextEditorTextContent( unsigned char *pszText, int lenText );

#endif /* __COM_EDIT_H__ */
