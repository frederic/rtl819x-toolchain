#ifndef __IOCTL_NET_H__
#define __IOCTL_NET_H__

/* Get string of default gateway, and return value indicates parameter is valid or not. */
extern int GetDefaultGatewayText( unsigned char *pszGateway );

/* Get string of first DNS, and return value indicates parameter is valid or not. */
extern int GetFirstDNSText( unsigned char *pszDNS );

#endif /* __IOCTL_NET_H__ */

