#ifndef __UI_FLAGS_H__
#define __UI_FLAGS_H__

/* flags */
typedef union {		/* runtime assistant flags */
	struct {
		unsigned long	hookStatus:1;		/* 1: off-hook, 0: on-hook */
		unsigned long	tonePlaying:1;		/* 1: tone is playing */
		unsigned long	keyStopTonePluse:1;	/* 1: stop tone and unset flag */
		unsigned long	animatePlaying:1;	/* 1: animation is playing */
		unsigned long	ledBlinking:1;		/* 1: led is blinking */
		unsigned long	pingRequest:1;		/* 1: now ping a ip addr */
		unsigned long	handfree:1;			/* 1: use speaker instead of receiver */
		unsigned long	adjustMic:1;		/* 1: +/- adjust mic instead of spaker */
		unsigned long	lsrProcPair:1;		/* 1: verify lsr procedure to be pair (for debug purpose) */
		unsigned long	sipDisplay:1;		/* 1: display SIP register status */
		unsigned long	sipRegister:1;		/* 1: SIP registered */
		unsigned long	promptSoftkey:1;	/* 1: softkey has been set in prompt state */
	} b;
	unsigned long all;
} fHostFlags_t;

extern fHostFlags_t fHostFlags;

typedef union {		/* flags for temporal usage, so that give them slight responsibility. */
	struct {
		unsigned long	connDial:1;			/* 1: it is going to dial, help to jump to UI_STATE_IN_CONN_DIAL (a temporal variable) */
		unsigned long	connDialLine:1;		/* 0: Line 1, 1: line 2 (ONLY used by UI_STATE_IN_CONN_DIAL) */
		unsigned long	connDialTransfer:1;	/* 0: dial 2nd, 1: transfer (ONLY used by UI_STATE_IN_CONN_DIAL) */
		unsigned long	unsetDiscDisplay:1;	/* 1: unset disc display and multi-line */
	} b;
	unsigned long all;
} fTempFlags_t;

extern fTempFlags_t fTempFlags;

typedef union {		/* flags for long jobs, such as physical flash writing. */
	struct {
		unsigned long 	flashWrite:1;		/* physical flash writing */
	} b;
	unsigned long all;
} fLongJobFlags_t;

extern fLongJobFlags_t fLongJobFlags;

typedef union {		/* flags for phone call line */
	struct {
		unsigned long	active:1;			/* it is active line */ /* In our design, only one of lines is active, except conference. */
		unsigned long	contact:1;			/* incomingCall bit is valid, also this line is in use */
		unsigned long	incomingCall:1;		/* incoming or outgoing call */
		unsigned long	waiting:1;			/* incoming call waiting (another line is in connection) */
		unsigned long	interactIncoming:1;	/* user interact (answer/reject) with incoming call */
		unsigned long	connected:1;		/* it was ever in connection state */
		unsigned long	hold:1;				/* I announce hold req */
		unsigned long	held:1;				/* receiver held ind */
		unsigned long	switching:1;		/* line switching so deny other action */
		unsigned long	disconnected:1;		/* it is disconnected */
		unsigned long	discDisplay:1;		/* display disconnection in CONNECTION state */
	} b;
	unsigned long all;
} fLineFlags_t;

extern fLineFlags_t fLineFlags[];

typedef union {		/* flags for phone call */
	struct {
		unsigned long	volumePromptDial:1;	/* adjust volume in dial state */
		unsigned long	volumePrompt:1;		/* adjust volume in connection state */
		unsigned long	multiline:1;		/* now multi-line working */
		unsigned long	conference:1;		/* conference */
		unsigned long	transfer:1;			/* call transfer */
	} b;
	unsigned long all;
} fCallFlags_t;

extern fCallFlags_t fCallFlags;

#endif /* __UI_FLAGS_H__ */
