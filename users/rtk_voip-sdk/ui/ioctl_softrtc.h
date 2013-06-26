#ifndef __IOCTL_SOFTRTC_H__
#define __IOCTL_SOFTRTC_H__

typedef unsigned long uptime_t;

/* Get current uptime */
extern uptime_t GetUptimeInMillisecond( void );

extern unsigned long GetUptimeInSecond( void );

/* If timeout return 0, otherwise return time to timeout */
extern uptime_t CheckIfTimeoutInMillisecond( uptime_t *pPrevUptime, 
												unsigned long nTimeoutMS );

#endif /* __IOCTL_SOFTRTC_H__ */

