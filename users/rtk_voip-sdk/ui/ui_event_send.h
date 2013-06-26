#ifndef __UI_EVENT_SEND_H__
#define __UI_EVENT_SEND_H__

typedef unsigned long ocf_t;		/* outgoing call flags */

#define OCF_NORMAL		0x0000
#define OCF_NONE		0x0000
#define OCF_CONNECTED	0x0001	/* first line is connected */
#define OCF_TRANSFER	0x0002	/* do call transfer */
#define OCF_FXO			0x0004	/* dial to local FXO */

#endif /* __UI_EVENT_SEND_H__ */

