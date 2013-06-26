#ifndef __GS_DEF_H__
#define __GS_DEF_H__

/* This file is only used by gs_xxx */
typedef union fDrawOffScreen_s {		/* flags for draw off screen */
	struct {
		unsigned long	on:1;			/* Draw off screen */
		unsigned long	supervisor:1;	/* Supervisor mode */
	} b;
	unsigned long all;
} fDrawOffScreen_t;

extern fDrawOffScreen_t fDrawOffScreen;

#endif /* !__GS_DEF_H__ */
