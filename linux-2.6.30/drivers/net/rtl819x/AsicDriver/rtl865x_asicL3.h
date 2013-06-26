/*
* Copyright c                  Realtek Semiconductor Corporation, 2009  
* All rights reserved.
* 
* Program : Switch table Layer3 route driver,following features are included:
*	Route/Multicast
* Abstract :
* Author : hyking (hyking_liu@realsil.com.cn)  
*/

#ifndef RTL865X_ASICL3_H
#define RTL865X_ASICL3_H

#define RTL8651_IPTABLE_SIZE			16
#define RTL8651_PPPOETBL_SIZE			8
#define RTL8651_NEXTHOPTBL_SIZE		32
#define RTL8651_ROUTINGTBL_SIZE		8
#define RTL8651_ARPTBL_SIZE			512

#if defined(CONFIG_RTL_8196E)

#define RTL8651_MULTICASTTBL_SIZE			256
#define RTL8651_IPMULTICASTTBL_SIZE		256
#define RTL8651_IPMCAST_CAM_SIZE			32

#elif defined (CONFIG_RTL8196C_REVISION_B) || defined (CONFIG_RTL8198_REVISION_B) || defined(CONFIG_RTL_819XD)
#define RTL8651_IPMULTICASTTBL_SIZE		128
#define RTL8651_MULTICASTTBL_SIZE		128
#define RTL8651_IPMCAST_CAM_SIZE			32

#else
#define RTL8651_IPMULTICASTTBL_SIZE		64
#define RTL8651_MULTICASTTBL_SIZE		64
#endif

typedef struct {
#ifndef _LITTLE_ENDIAN
    /* word 0 */
    ipaddr_t        internalIP;
    /* word 1 */
    ipaddr_t        externalIP;
    /* word 2 */
    uint32          reserv0     : 24;
    uint32          nextHop     : 5;
    uint32          isLocalPublic   : 1;
    uint32          isOne2One       : 1;
    uint32          valid       : 1;
#else /*_LITTLE_ENDIAN*/
    /* word 0 */
    ipaddr_t        internalIP;
    /* word 1 */
    ipaddr_t        externalIP;
    /* word 2 */
    uint32          valid       : 1;
    uint32          isOne2One       : 1;
    uint32          isLocalPublic   : 1;
    uint32          nextHop     : 5;
    uint32          reserv0     : 24;
#endif /*_LITTLE_ENDIAN*/
    /* word 3 */
    uint32          reservw3;
    /* word 4 */
    uint32          reservw4;
    /* word 5 */
    uint32          reservw5;
    /* word 6 */
    uint32          reservw6;
    /* word 7 */
    uint32          reservw7;
} rtl8651_tblAsic_extIpTable_t;

typedef struct {
#ifndef _LITTLE_ENDIAN
    /* word 0 */
    uint16          reserv0     : 13;
    uint16          ageTime     : 3;
    uint16          sessionID;
#else /*_LITTLE_ENDIAN*/
    /* word 0 */
    uint16          sessionID;
    uint16          ageTime     : 3;
    uint16          reserv0     : 13;
#endif /*_LITTLE_ENDIAN*/
    /* word 1 */
    uint32          reservw1;
    /* word 2 */
    uint32          reservw2;
    /* word 3 */
    uint32          reservw3;
    /* word 4 */
    uint32          reservw4;
    /* word 5 */
    uint32          reservw5;
    /* word 6 */
    uint32          reservw6;
    /* word 7 */
    uint32          reservw7;
} rtl8651_tblAsic_pppoeTable_t;



typedef struct {
#ifndef _LITTLE_ENDIAN
    /* word 0 */
    uint32          reserv0     : 11;
    uint32          nextHop     : 10;
    uint32          PPPoEIndex  : 3;
    uint32          dstnetif     : 3;
    uint32          IPIndex     : 4;
    uint32          type        : 1;
#else
    /* word 0 */
    uint32          type        : 1;
    uint32          IPIndex     : 4;
    uint32          dstnetif      : 3;
    uint32          PPPoEIndex  : 3;
    uint32          nextHop     : 10;
    uint32          reserv0     : 11;
#endif /*_LITTLE_ENDIAN*/
    /* word 1 */
    uint32          reservw1;
    /* word 2 */
    uint32          reservw2;
    /* word 3 */
    uint32          reservw3;
    /* word 4 */
    uint32          reservw4;
    /* word 5 */
    uint32          reservw5;
    /* word 6 */
    uint32          reservw6;
    /* word 7 */
    uint32          reservw7;
} rtl865xc_tblAsic_nextHopTable_t;

typedef struct {
#ifndef _LITTLE_ENDIAN
    /* word 0 */
    uint32          reserv0     : 11;
    uint32          nextHop     : 10;
    uint32          PPPoEIndex  : 3;
    uint32          dstVid      : 3;
    uint32          IPIndex     : 4;
    uint32          type        : 1;
#else
    /* word 0 */
    uint32          type        : 1;
    uint32          IPIndex     : 4;
    uint32          dstVid      : 3;
    uint32          PPPoEIndex  : 3;
    uint32          nextHop     : 10;
    uint32          reserv0     : 11;
#endif /*_LITTLE_ENDIAN*/
    /* word 1 */
    uint32          reservw1;
    /* word 2 */
    uint32          reservw2;
    /* word 3 */
    uint32          reservw3;
    /* word 4 */
    uint32          reservw4;
    /* word 5 */
    uint32          reservw5;
    /* word 6 */
    uint32          reservw6;
    /* word 7 */
    uint32          reservw7;
} rtl8651_tblAsic_nextHopTable_t;



typedef struct {
#ifndef _LITTLE_ENDIAN
    /* word 0 */
    ipaddr_t        IPAddr;

    /* word 1 */
    union {
        struct {
            uint32          reserv0     : 3;
            uint32		  ARPIpIdx	: 3;
            uint32          ARPEnd      : 6;			
            uint32          ARPStart    : 6;
            uint32          netif         : 3;
            uint32          isDMZ      : 1;			
            uint32          internal   : 1;
            uint32          process     : 3;
            uint32          valid       : 1;			
            uint32          IPMask      : 5;
        } ARPEntry;
        struct {
            uint32          reserv0     : 8;
            uint32          nextHop     : 10;
            uint32          netif         : 3;
            uint32          isDMZ      : 1;
            uint32          internal   : 1;
            uint32          process     : 3;			
            uint32          valid       : 1;
            uint32          IPMask      : 5;
        } L2Entry;
        struct {

            uint32          reserv0     : 5;
            uint32          PPPoEIndex  : 3;
            uint32          nextHop     : 10;
	     uint32          netif         : 3;						
            uint32          isDMZ      : 1;
            uint32          internal   : 1;
            uint32          process     : 3;
            uint32          valid       : 1;
            uint32          IPMask      : 5;
        } PPPoEEntry;
        struct {
            uint32          reserv0     : 4;
            uint32          IPDomain    : 3;
            uint32          nhAlgo      : 2;
            uint32          nhNxt       : 5;
            uint32          nhStart     : 4;
            uint32          nhNum       : 3;
	     uint32          isDMZ      : 1;
	     uint32          internal   : 1;
	     uint32          process     : 3;
	     uint32          valid       : 1;
            uint32          IPMask      : 5;			
        } NxtHopEntry;

    } linkTo;
#else /*_LITTLE_ENDIAN*/

    /* word 0 */
    ipaddr_t        IPAddr;
    /* word 1 */
    union {
        struct {
            uint32          IPMask      : 5;
            uint32          valid       : 1;
            uint32          process     : 3;
            uint32          internal   : 1;
            uint32          isDMZ      : 1;
			
            uint32          netif         : 3;
            uint32          ARPStart    : 6;
            uint32          ARPEnd      : 6;
            uint32		  ARPIpIdx	: 3;
            uint32          reserv0     : 3;
        } ARPEntry;
        struct {
            uint32          IPMask      : 5;
            uint32          valid       : 1;
            uint32          process     : 3;
            uint32          internal   : 1;
            uint32          isDMZ      : 1;

            uint32          netif         : 3;			
            uint32          nextHop     : 10;
            uint32          reserv0     : 8;
        } L2Entry;
        struct {
            uint32          IPMask      : 5;
            uint32          valid       : 1;
            uint32          process     : 3;
            uint32          internal   : 1;
            uint32          isDMZ      : 1;
			
	     uint32          netif         : 3;			
            uint32          nextHop     : 10;
            uint32          PPPoEIndex  : 3;
            uint32          reserv0     : 5;
        } PPPoEEntry;
        struct {
            uint32          IPMask      : 5;
            uint32          valid       : 1;
            uint32          process     : 3;
            uint32          internal   : 1;
            uint32          isDMZ      : 1;

		
            uint32          nhNum       : 3;
            uint32          nhStart     : 4;
            uint32          nhNxt       : 5;
            uint32          nhAlgo      : 2;
            uint32          IPDomain    : 3;
            uint32          reserv0     : 4;
        } NxtHopEntry;
    } linkTo;
#endif /*_LITTLE_ENDIAN*/
    /* word 2 */
    uint32          reservw2;
    /* word 3 */
    uint32          reservw3;
    /* word 4 */
    uint32          reservw4;
    /* word 5 */
    uint32          reservw5;
    /* word 6 */
    uint32          reservw6;
    /* word 7 */
    uint32          reservw7;
} rtl865xc_tblAsic_l3RouteTable_t;

typedef struct {
#ifndef _LITTLE_ENDIAN
    /* word 0 */
    uint32          reserv0     : 16;
    uint32          aging:5;
    uint32          nextHop     : 10;
    uint32          valid       : 1;
#else /*_LITTLE_ENDIAN*/
    /* word 0 */
    uint32          valid       : 1;
    uint32          nextHop     : 10;
    uint32		  aging:5;
    uint32          reserv0     : 21;
#endif /*_LITTLE_ENDIAN*/
    /* word 1 */
    uint32          reservw1;
    /* word 2 */
    uint32          reservw2;
    /* word 3 */
    uint32          reservw3;
    /* word 4 */
    uint32          reservw4;
    /* word 5 */
    uint32          reservw5;
    /* word 6 */
    uint32          reservw6;
    /* word 7 */
    uint32          reservw7;
} rtl865xc_tblAsic_arpTable_t;

typedef struct {
#if defined (CONFIG_RTL8196C_REVISION_B) || defined (CONFIG_RTL8198_REVISION_B) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)
    /* word 0 */
    ipaddr_t        srcIPAddr;
    /* word 1 */
    uint32          srcPort      : 4;
    uint32          destIPAddrLsbs : 28;

    /* word 2*/
    uint32          reserv0     : 14;
    uint32          ageTime     : 3;
    uint32          toCPU       : 1;
    uint32          valid       : 1;
    uint32          extIPIndex  : 4;
    uint32          portList    : 9;

    /* word 3 */
    uint32          reservw3;
    /* word 4 */
    uint32          reservw4;
    /* word 5 */
    uint32          reservw5;
    /* word 6 */
    uint32          reservw6;
    /* word 7 */
    uint32          reservw7;

#else
#ifndef _LITTLE_ENDIAN
    /* word 0 */
    ipaddr_t        srcIPAddr;
    /* word 1 */
    uint32          srcVidL      : 4;
    uint32          destIPAddrLsbs : 28;

    /* word 2*/
    uint32          reserv0     : 2;
    uint32  	  extIPIndexH : 1;
    uint32          ageTime     : 3;
    uint32          extPortList : 3;
    uint32          srcPortExt  : 1;
    uint32          toCPU       : 1;
    uint32          valid       : 1;
    uint32          extIPIndex  : 3;
    uint32          portList    : 6;
    uint32          srcPort    : 3;
    uint32		  srcVidH: 8;
#else
    /* word 0 */
    ipaddr_t        srcIPAddr;
    /* word 1 */
    uint32          destIPAddrLsbs : 28;
    uint32          srcVidL      :4 ;
    /* word 2*/
    uint32		  srcVidH:8;
    uint32          srcPort   : 3;
    uint32          portList   : 6;
    uint32          extIPIndex  : 3;
    uint32          valid       : 1;
    uint32          toCPU       : 1;
    uint32          srcPortExt  : 1;
    uint32          extPortList : 3;
    uint32          ageTime     : 3;
    uint32  	  extIPIndexH : 1;
    uint32          reserv0     : 2;

#endif /*_LITTLE_ENDIAN*/
    /* word 3 */
    uint32          reservw3;
    /* word 4 */
    uint32          reservw4;
    /* word 5 */
    uint32          reservw5;
    /* word 6 */
    uint32          reservw6;
    /* word 7 */
    uint32          reservw7;
#endif
} rtl865xc_tblAsic_ipMulticastTable_t;

typedef struct rtl865x_tblAsicDrv_extIntIpParam_s {
	    ipaddr_t 	extIpAddr;
	    ipaddr_t 	intIpAddr;
	    uint32 		nhIndex; //index of next hop table
	    uint32 		localPublic:1,
	           		nat:1;
} rtl865x_tblAsicDrv_extIntIpParam_t;

typedef struct rtl865x_tblAsicDrv_pppoeParam_s {
	uint16 sessionId;
	uint16 age;
} rtl865x_tblAsicDrv_pppoeParam_t;


typedef struct rtl865x_tblAsicDrv_nextHopParam_s {
	uint32 nextHopRow;
	uint32 nextHopColumn;
	uint32 pppoeIdx;
	uint32 dvid;	//note: dvid means DVID index here! hyking
	uint32 extIntIpIdx;
	uint32 isPppoe:1;
} rtl865x_tblAsicDrv_nextHopParam_t;

typedef struct rtl865x_tblAsicDrv_routingParam_s {
	    ipaddr_t ipAddr;
	    ipaddr_t ipMask;
	    uint32 process; //0: pppoe, 1:direct, 2:indirect, 4:Strong CPU, 5:napt nexthop
	    uint32 vidx;
	    uint32 arpStart;
	    uint32 arpEnd;
	    uint32 arpIpIdx; /* for RTL8650B C Version Only */
	    uint32 nextHopRow;
	    uint32 nextHopColumn;
	    uint32 pppoeIdx;
	    uint32 nhStart; //exact index
	    uint32 nhNum; //exact number
	    uint32 nhNxt;
	    uint32 nhAlgo;
	    uint32 ipDomain;
	    uint16 	internal:1,
		DMZFlag:1;
		
 	    uint32 netif;
} rtl865x_tblAsicDrv_routingParam_t;

typedef struct rtl865x_tblAsicDrv_arpParam_s {
	uint32 nextHopRow;
	uint32 nextHopColumn;
	uint32 aging;	
} rtl865x_tblAsicDrv_arpParam_t;

typedef struct rtl865x_tblAsicDrv_multiCastParam_s {
	ipaddr_t	sip;
	ipaddr_t	dip;
	uint16	svid;
	uint16	port;
	uint32	mbr;
	uint16	age;
	uint16	cpu;
	uint16	extIdx;
} rtl865x_tblAsicDrv_multiCastParam_t;


/*arp*/
int32 rtl8651_setAsicArp(uint32 index, rtl865x_tblAsicDrv_arpParam_t *arpp);
int32 rtl8651_delAsicArp(uint32 index);
int32 rtl8651_getAsicArp(uint32 index, rtl865x_tblAsicDrv_arpParam_t *arpp);

/*ip*/
int32 rtl8651_setAsicExtIntIpTable(uint32 index, rtl865x_tblAsicDrv_extIntIpParam_t *extIntIpp);
int32 rtl8651_delAsicExtIntIpTable(uint32 index);
int32 rtl8651_getAsicExtIntIpTable(uint32 index, rtl865x_tblAsicDrv_extIntIpParam_t *extIntIpp);

/*pppoe*/
int32 rtl8651_setAsicPppoe(uint32 index, rtl865x_tblAsicDrv_pppoeParam_t *pppoep);
int32 rtl8651_getAsicPppoe(uint32 index, rtl865x_tblAsicDrv_pppoeParam_t *pppoep);

/*nexthop*/
int32 rtl8651_setAsicNextHopTable(uint32 index, rtl865x_tblAsicDrv_nextHopParam_t *nextHopp);
int32 rtl8651_getAsicNextHopTable(uint32 index, rtl865x_tblAsicDrv_nextHopParam_t *nextHopp);

/*L3 routing*/
int32 rtl8651_setAsicRouting(uint32 index, rtl865x_tblAsicDrv_routingParam_t *routingp);
int32 rtl8651_delAsicRouting(uint32 index);
int32 rtl8651_getAsicRouting(uint32 index, rtl865x_tblAsicDrv_routingParam_t *routingp);

/*multicast*/
uint32 rtl8651_ipMulticastTableIndex(ipaddr_t srcAddr, ipaddr_t dstAddr);
int32 rtl8651_setAsicIpMulticastTable(rtl865x_tblAsicDrv_multiCastParam_t *mCast_t);
int32 rtl8651_delAsicIpMulticastTable(uint32 index);
int32 rtl8651_getAsicIpMulticastTable(uint32 index, rtl865x_tblAsicDrv_multiCastParam_t *mCast_t);
int32 rtl8651_setAsicMulticastPortInternal(uint32 port, int8 isInternal);
int32 rtl8651_getAsicMulticastPortInternal(uint32 port, int8 *isInternal);
int32 rtl8651_setAsicMulticastMTU(uint32 mcastMTU);
int32 rtl8651_getAsicMulticastMTU(uint32 *mcastMTU);
int32 rtl8651_setAsicMulticastEnable(uint32 enable);
int32 rtl8651_getAsicMulticastEnable(uint32 *enable);
int32 rtl865x_setAsicMulticastAging(uint32 enable);



#endif
