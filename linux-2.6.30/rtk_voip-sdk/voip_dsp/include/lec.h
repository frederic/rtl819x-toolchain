#if !defined(_LEC_H_)
#define _LEC_H_

#include "../../include/voip_types.h"

/* LEC Mode Mask */
#define LEC             0x01	/* LEC */
#define LEC_NLP   	0x02	/* LEC+NLP */
#define LEC_NLP_MUTE  	0x04	/* LEC+NLP_mute */
#define LEC_NLP_CNG     0x08	/* LEC+NLP_CNG */
#define LEC_NLP_SHIFT   0x10	/* LEC+NLP_shift */

#ifdef CONFIG_RTK_VOIP_MODULE
#define DMEN_LEC	0
#define ASM_LEC		1
#define DMEN_STACK_LEC  0
#else
#define DMEN_LEC	0
#define ASM_LEC		1
#define DMEN_STACK_LEC  0
#endif

/* lec.c function calls prototype */

/*  sensitivity range :3 ~ 8
	The smaller sensitivity value, the large NLP threshold, and the worse double talk performance.
	The larger sensitivity value, the smaller NLP threshold, and better double talk performance.But the worse LEC performance.
	Suggested value is 6.
*/
void LEC_DT_Config(int dt_sensitivity);

void LEC_NLP_Config(int nlp_sensitivity);

void LEC_re_init(unsigned char chid);

void LEC_g168_init(unsigned char chid, unsigned char type);

void LEC_g168(char chid, const Word16 *pRin, Word16 *pSin, Word16 *pEx);

int LEC_g168_enable(char chid);

int LEC_g168_disable(char chid);

void LEC_g168_set_TailLength(unsigned int chid, unsigned int tail_length);

#ifdef SUPPORT_RTCP_XR
unsigned int LEC_g168_Get_ERLE(unsigned int chid);
#endif

int LEC_g168_vbd_auto(char chid, int vbd_high, int vbd_low, int lec_bk);

int LEC_g168_set_nlp(unsigned char chid, unsigned char mode);

#endif

