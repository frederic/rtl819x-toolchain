/*
  *  RTL8197B header file of hcd
  *
  *	Copyright (C)2008, Realtek Semiconductor Corp. All rights reserved.
  *
  *	$Id: hcd.h,v 1.3 2010/02/05 07:21:45 marklee Exp $
  */

/*================================================================*/

#ifndef INCLUDE_SYSINIT_H
#define INCLUDE_SYSINIT_H

#define BR_UTL							("brctl")
#define IFCONFG							("ifconfig")
#define IF_BR							("br0")
#define IF_ETH							("eth0")
#define IF_WLAN							("wlan0")

//MII port means the Inband port between HOST and SLAVE
//usually in 8196c it's eth0, 8198 = eth1 .
//#define IF_MII							("eth1")
#define IF_MII							("eth0")


#define INBAND_NETIF  ("br0")
#define INBAND_MAC ("001234567899")

#define RTK_MBSSID_SUPPORT 1
#define RTK_MAX_MBSSID 4

#define ALL_ZERO_MAC_ADDR	"\x0\x0\x0\x0\x0\x0"


/*================================================================*/
/* Macro Definitions */

#define DISPLAY_BANNER \
	printf("\n\n%s %s %s\n\n", PROG_NAME, VERSION_STR, COPYRIGHT)

#define ADD_BR_INTERFACE(br, dev, addr, mac, phy) { \
	if (mac) { \
		sprintf(tmpbuf, "%s addif %s %s", BR_UTL, br, dev); \
		system(tmpbuf); \
	} \
	if (phy) { \
		if (memcmp(addr, ALL_ZERO_MAC_ADDR, 6)) { \
			sprintf(tmpbuf, "%s %s hw ether %02x%02x%02x%02x%02x%02x", IFCONFG, \
				dev, addr[0], addr[1],addr[2], addr[3],addr[4], addr[5]); \
			system(tmpbuf); \
		}\
		sprintf(tmpbuf, "%s %s up", IFCONFG, dev); \
		system(tmpbuf);\
	} \
}

#define ADD_BR(br) { \
	sprintf(tmpbuf, "%s addbr %s", BR_UTL, br); \
	system(tmpbuf); \
	sprintf(tmpbuf, "%s %s up", IFCONFG, br); \
	system(tmpbuf);	\
}

#define DEL_BR(br) { \
	sprintf(tmpbuf, "%s delbr %s", BR_UTL, br); \
	system(tmpbuf); \
}

#define DISABLE_STP(br) { \
	sprintf(tmpbuf, "%s setfd %s 0", BR_UTL, br); \
	system(tmpbuf); \
	sprintf(tmpbuf, "%s stp %s 0", BR_UTL, br); \
	system(tmpbuf); \
}

#define DEL_BR_INTERFACE(br, dev, mac, phy) { \
	if (phy) { \
		sprintf(tmpbuf, "%s %s down", IFCONFG, dev); \
		system(tmpbuf); \
	} \
	if (mac) { \
		sprintf(tmpbuf, "%s delif %s %s", BR_UTL, br, dev); \
		system(tmpbuf); \
	} \
}
#endif // INCLUDE_SYSINIT_H

