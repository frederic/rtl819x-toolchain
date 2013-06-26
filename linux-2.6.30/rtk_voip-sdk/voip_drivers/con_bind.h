#ifndef __CON_BIND_H__
#define __CON_BIND_H__

#include "con_register.h"

typedef enum {
	BIND_POLICY_DECT_FXS_DAA,	// 8k: bind in odrder -  all DECT / all FXS / all DAA 
	BIND_POLICY_FXS_DAA_SHARE,	// 8k: FXS share bus with DAA 
	BIND_POLICY_FXS_ONLY,		// 8k: FXS only 
	
	BIND_POLICY_DECT_FXS_DAA_FULLY,	// 8k/16k: bind all FXS, and then all DAA 
	
	BIND_POLICY_AC_ONLY,		// 8k/16k: bind AudioCodec 
	
	BIND_POLICY_REG_ORDER,		// registered order 
	
	BIND_POLICY_FXS_DAA_SNDONLY,	// 8k: bind FXS/DAA for IPC host control SND 
} bind_policy_t;

extern int auto_bind_con_with_policy( voip_con_t voip_con[], int num, bind_policy_t policy );

#endif // __CON_BIND_H__

