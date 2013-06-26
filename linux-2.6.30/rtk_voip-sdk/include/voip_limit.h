#ifndef __VOIP_LIMIT_H__
#define __VOIP_LIMIT_H__

// SIP media ID will be 0 or 1 
#define MAX_MID_NUM			2	

// rtp use mid 0/1, rtcp use 2/3 (IPC arch use only )
#define RTCP_MID_OFFSET		MAX_MID_NUM

#endif /* __VOIP_LIMIT_H__ */

