/*
  *  RTL8197B header file of hcd
  *
  *	Copyright (C)2008, Realtek Semiconductor Corp. All rights reserved.
  *
  *	$Id: hcd.h,v 1.9 2009/04/15 03:51:41 michael Exp $
  */

/*================================================================*/

#ifndef INCLUDE_HCD_H
#define INCLUDE_HCD_H


/*================================================================*/
/* Compiling Flags */

#define DB_ERR							// defined to print out error message
//#define DB_MSG						// defined to print out debug message
//#define ACCESS_WLAN_IF		            // defined to access wlan interface
#define CMD_LINE						// define d to include cmd line interface
#define JUMP_CMD						// defined to add jump ioctl cmd, for development purpose


/*================================================================*/
/* Constant Definitions */

#define VERSION_STR					"v0.2"
#define PROG_NAME					("hcd")
#define COPYRIGHT					("Copyright (c) 2009 Realtek Semiconductor Corp.")
#define DEV_NAME					("/dev/rtl_mdio")
#define BIT(x)						(1 << (x))

#define PID_FILE					("/var/hcd.pid")
#define CMD_FILE					("/tmp/hcd.cmd")

#define BR_UTL						("brctl")
#define IFCONFG						("ifconfig")
#define IF_BR						("br0")
#define IF_ETH						("eth0")
#define IF_MII						("eth1")
#ifdef RT_WLAN
#define IF_WLAN						("ra0")
#else
#define IF_WLAN						("wlan0")
#endif

// command for set_scr
#define INIT_BR					BIT(0)
#define INIT_ETH_MAC				BIT(1)
#define INIT_ETH_PHY				BIT(2)
#define INIT_MII_MAC				BIT(3) // isolation
#define INIT_WLAN_MAC			   	BIT(4)
#define INIT_WLAN_PHY			    	BIT(5)
#define INIT_MII_CLK			    	BIT(6) // SelMIIClk

// return code of command
#define RET_OK					0
#define RET_CANNOT_ACCESS			1
#define RET_INVALID_CMD_ID			2
#define RET_INVALID_RANGE			3
#define RET_NOT_NOW				4
#define RET_IOCTL_ERR				5
#define RET_NOT_SUPPORT_NOW	        	6
#define RET_INVALID_ARG				7

#define ALL_ZERO_MAC_ADDR	"\x0\x0\x0\x0\x0\x0"

// argument keyword
#define LOAD_DAEMON				        ("daemon")
#define SET_MIB						("set_mib")
#define GET_MIB						("get_mib")
#define ACT_REQ						("do_request")
#define DUMP_ALL_MIB				    	("dump_all_mib")
#define SET_SCR						("set_scr")
#ifdef JUMP_CMD
#define JUMP_ADDR					("jump")
#endif


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

#ifdef DB_MSG
	#define DEBUG_OUT(fmt, args...)		printf("%s_%s: "fmt, PROG_NAME, __FUNCTION__, ## args)
#else
	#define DEBUG_OUT(fmt, args...)
#endif

#ifdef DB_ERR
	#define DEBUG_ERR(fmt, args...)		printf("%s_%s_ERR: "fmt, PROG_NAME, __FUNCTION__, ## args)
#else
	#define DEBUG_ERR(fmt, args...)
#endif


/*================================================================*/
/* External routines */

extern int do_mdio_ioctl(int id, void *data);

#endif // INCLUDE_HCD_H

