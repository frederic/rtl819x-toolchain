/*
* Copyright c                  Realtek Semiconductor Corporation, 2008  
* All rights reserved.
* 
* Program : nexthop table driver
* Abstract : 
* Author : hyking (hyking_liu@realsil.com.cn)  
*/

/*      @doc RTL_LAYEREDDRV_API

        @module rtl865x_nexthop.c - RTL865x Home gateway controller Layered driver API documentation       |
        This document explains the API interface of the table driver module. Functions with rtl865x prefix
        are external functions.
        @normal Hyking Liu (Hyking_liu@realsil.com.cn) <date>

        Copyright <cp>2008 Realtek<tm> Semiconductor Cooperation, All Rights Reserved.

        @head3 List of Symbols |
        Here is a list of all functions and variables in this module.
        
        @index | RTL_LAYEREDDRV_API
*/
#include <net/rtl/rtl_types.h>
#include <net/rtl/rtl_glue.h>
#include <net/rtl/rtl865x_netif.h>
#include "common/rtl_errno.h"
#include "common/rtl865x_netif_local.h" 
#include "rtl865x_ip.h"
#include "rtl865x_nexthop.h"
//#include "rtl865x_ppp.h"
#include "rtl865x_ppp_local.h"
#include "rtl865x_route.h"
#include "rtl865x_arp.h"

#include "common/rtl865x_vlan.h"
#include "common/rtl865x_eventMgr.h" /*call back function....*/

#ifdef CONFIG_RTL_LAYERED_ASIC_DRIVER
#include "AsicDriver/rtl865x_asicCom.h"
#include "AsicDriver/rtl865x_asicL2.h"
#include "AsicDriver/rtl865x_asicL3.h"
#else
#include "AsicDriver/rtl865xC_tblAsicDrv.h"
#endif
#include "l2Driver/rtl865x_fdb.h"
#include <net/rtl/rtl865x_fdb_api.h>

static rtl865x_nextHopEntry_t *rtl865x_nxtHopTable;

#define NEXTHOP_TABLE_INDEX(entry)	(entry - rtl865x_nxtHopTable)
static int32 _rtl865x_nextHop_register_event(void);
static int32 _rtl865x_nextHop_unRegister_event(void);
static int32 _rtl865x_delNxtHop(uint32 attr, uint32 entryIdx);

static RTL_DECLARE_MUTEX(nxthop_sem);

#if 0
static void _print_softNxtHop(void)
{
	int32 i = 0;
	rtl865x_nextHopEntry_t *entry = rtl865x_nxtHopTable;
	printk("software nextHop table:\n");
	for(i = 0; i < NXTHOP_ENTRY_NUM; i++,entry++)
	{
		printk("idx(%d): valid(%d),dst(%s),nexthop(%d),nexthopType(%d),refCnt(%d)\n",i,entry->valid,entry->dstNetif->name,
			entry->nexthop,entry->nextHopType,entry->refCnt);
	}
}
#endif

/*
@func int32 | rtl865x_initNxtHopTable |initialize the nexthop table
@rvalue SUCCESS | success.
@rvalue FAILED | failed. system should reboot.
@comm	
*/
int32 rtl865x_initNxtHopTable(void)
{
	TBL_MEM_ALLOC(rtl865x_nxtHopTable, rtl865x_nextHopEntry_t, NXTHOP_ENTRY_NUM);	
	memset(rtl865x_nxtHopTable,0,sizeof(rtl865x_nextHopEntry_t)*NXTHOP_ENTRY_NUM);
	_rtl865x_nextHop_register_event();
	return SUCCESS;	
}

/*
@func int32 | rtl865x_reinitNxtHopTable |reinitialize the nexthop table
@rvalue SUCCESS | success.
@comm	
*/
int32 rtl865x_reinitNxtHopTable(void)
{
	int32 i;
	_rtl865x_nextHop_unRegister_event();

	for(i = 0; i < NXTHOP_ENTRY_NUM; i++)
	{
		if(rtl865x_nxtHopTable[i].valid)
			_rtl865x_delNxtHop(NEXTHOP_L3,i);
	}
	
	_rtl865x_nextHop_register_event();
	return SUCCESS;
}
#if defined CONFIG_RTL_LOCAL_PUBLIC
extern  int rtl865x_getLocalPublicArpMapping(unsigned int ip, rtl865x_arpMapping_entry_t * arp_mapping);
#endif
static int32 _rtl865x_synNxtHopToAsic(rtl865x_nextHopEntry_t *entry_t)
{
	rtl865x_tblAsicDrv_nextHopParam_t asic;
	ether_addr_t reservedMac;
	rtl865x_tblAsicDrv_l2Param_t asic_l2;
	rtl865x_netif_local_t *dstNetif;
	uint32 fid = 0;
	uint32 columIdx = 0;
	int32 retval = 0;
	int32 ipIdx = 0;

	retval = rtl865x_getReserveMacAddr(&reservedMac);
	
	bzero(&asic, sizeof(rtl865x_tblAsicDrv_nextHopParam_t));

	if(entry_t == NULL)
	{
		return RTL_EINVALIDINPUT;
	}
	
	if (entry_t->nextHopType == IF_ETHER)
	{
		rtl865x_arpMapping_entry_t arp_t;
		int32 ret_arpFound = FAILED;

		/*if the arp info of nexthop is not found, reserved to cpu Mac is used for trap packet to CPU*/
#if defined (CONFIG_RTL_LOCAL_PUBLIC)
		if(entry_t->nexthop)
		{
			ret_arpFound = rtl865x_getArpMapping(entry_t->nexthop,&arp_t);
			if(ret_arpFound!=SUCCESS)
			{
				/*try to match local pulbic ip*/
				ret_arpFound = rtl865x_getLocalPublicArpMapping(entry_t->nexthop, &arp_t);
				
			}
		}
#else
		if(entry_t->nexthop)
			ret_arpFound = rtl865x_getArpMapping(entry_t->nexthop,&arp_t);
#endif		
		
		rtl865x_getVlanFilterDatabaseId(entry_t->dstNetif->vid,&fid);
		retval = rtl865x_Lookup_fdb_entry(fid, (ret_arpFound == SUCCESS)? &arp_t.mac : &reservedMac, FDB_DYNAMIC, &columIdx,&asic_l2);
		
		asic.nextHopRow = rtl8651_filterDbIndex( (ret_arpFound == SUCCESS)? &arp_t.mac : &reservedMac, fid );
		asic.nextHopColumn = (retval == SUCCESS)? columIdx: 0;

		asic.isPppoe = FALSE;
		asic.pppoeIdx = 0;
	}
	else
	{	
		/*session based interface type*/
		rtl865x_ppp_t pppoe;		
		int32 pppidx = 0;

		memset(&pppoe,0,sizeof(rtl865x_ppp_t));
		
		if(entry_t->nexthop)
			retval = rtl865x_getPppBySessionId(entry_t->nexthop,&pppoe);

		rtl865x_getVlanFilterDatabaseId(entry_t->dstNetif->vid,&fid);
		

		retval =rtl865x_Lookup_fdb_entry(fid, (pppoe.valid)? &pppoe.server_mac : &reservedMac, FDB_DYNAMIC, &columIdx,&asic_l2);
			
		asic.nextHopRow = rtl8651_filterDbIndex( (pppoe.valid)? &pppoe.server_mac : &reservedMac, fid);
		asic.nextHopColumn = (pppoe.valid)? columIdx: 0;
		asic.isPppoe = (pppoe.type == IF_PPPOE)? TRUE: FALSE;

		retval = rtl865x_getPppIdx(&pppoe, &pppidx);

		//printk("%s(%d): pppoeIdx(%d), pppoeType(%d), pppoevalid(%d),pppoeSid(%d)\n",__FUNCTION__,__LINE__,pppidx,pppoe.type,pppoe.valid,pppoe.sessionId);
		asic.pppoeIdx	= (pppoe.type == IF_PPPOE)? pppidx: 0;
	}
	
	if(entry_t->dstNetif->is_slave == 1)
	{
		dstNetif = entry_t->dstNetif->master;

		if(dstNetif == NULL)
			dstNetif = _rtl865x_getDefaultWanNetif();
	}
	else
		dstNetif = entry_t->dstNetif;

	if(dstNetif == NULL)
		printk("_%s(%d), BUG!!!!!!",__FUNCTION__,__LINE__);
	
	asic.dvid 			= dstNetif->asicIdx;

	if(entry_t->srcIp)
		retval = rtl865x_getIpIdxByExtIp(entry_t->srcIp, &ipIdx);
	
	asic.extIntIpIdx	= entry_t->srcIp? ipIdx: 0;

	//printk("%s(%d), entryIdx(%d),asic.isPPPoe(%d),asic.pppoeIdx(%d),asic.dvid(%d)\n", __FUNCTION__,__LINE__,entry_t->entryIndex,asic.isPppoe,asic.pppoeIdx,asic.dvid);
	rtl8651_setAsicNextHopTable(entry_t->entryIndex,  &asic);
	
	return SUCCESS;
}


static int32 _rtl865x_arrangeNxtHop(uint32 start, uint32 num)
{
	int32 idx;
	rtl865x_nextHopEntry_t *entry = NULL;

	if(start < 0 || start + num >= NXTHOP_ENTRY_NUM)
		return RTL_EINVALIDINPUT;
	for(idx = start; idx < start + num; idx++)
	{
		entry = &rtl865x_nxtHopTable[idx];

		if(entry->valid)
			_rtl865x_synNxtHopToAsic(entry);
	}
	
	return SUCCESS;
}

static int32 _rtl865x_addNxtHop(uint32 attr, void *ref_ptr, rtl865x_netif_local_t *netif, uint32 nexthop,uint32 srcIp)
{
	int entryIdx;
	rtl865x_nextHopEntry_t *entry = NULL, *entry1 = NULL;
	rtl865x_route_t *rt_t = NULL;

	/*
	  * NOTE:
	  * parameter description:
	  * (1) attr: why need to add the nexthop entry? NEXTHOP_L3 or NEXTHOP_DEFREDIRECT_ACL?
	  * (2) ref_ptr: when attr = NEXTHOP_L3, ref_ptr point to a route structure,
	  *				   attr = NEXTHOP_DEFREDIRECT_ACL, ref_ptr = NULL,
	  *				   attr = others, ref_ptr = NULL
	  * (3) netif: destination network interface
	  * (4) nexthop: 
	  *		a) netif->if_type == IF_ETHER, nexthop = nexthop ip address,
	  *		b) netif->if_type == session based type, nexthop = session Id,
	  *
	  * following case should be NOTED now:
	  * (1) ETHERNET type network interface:
	  *	 a) If nexthop != NULL , it means the entry is added for:
	  *		nexthop ip&mac information, nextHop = arp entry of nexthop ip address.
	  *	  b) If nexthop == 0, use default route's nexthop or nexthop TOCPU
	  *
	  * (2) PPPoE/PPTP/L2TP type network interface:
	  *	  The "nexthop" will explicitly specify the PPPoE session (PPTP/L2TP session).
	  */

	if(netif == NULL)
		return RTL_EINVALIDINPUT;
	
	/* Allocate an empty entry for new one */
	/*Note: all nexthop entry referenced by L3 must be 2 entries aligned(reference l3 Datasheet)*/
	for(entryIdx = 0; entryIdx < NXTHOP_ENTRY_NUM; entryIdx++)
	{
		if(rtl865x_nxtHopTable[entryIdx].valid == 0)
		{
			switch(attr)
			{
				case NEXTHOP_L3:
					if( entryIdx%2 == 0 && (entryIdx + 1) < NXTHOP_ENTRY_NUM &&  rtl865x_nxtHopTable[entryIdx+1].valid == 0)
					{
						entry = &rtl865x_nxtHopTable[entryIdx];
						goto found;
					}
					break;
					
				case NEXTHOP_DEFREDIRECT_ACL:
					entry = &rtl865x_nxtHopTable[entryIdx];
					goto found;
					break;
					
				default:
					printk("attr(%d) is not support.....\n",attr);
					break;
			}
		}
	}

	/*if not found proper nextHop entry, return*/
	entry = NULL;

found:
	if(entry == NULL)
		return RTL_ENOFREEBUFFER;

	entry->valid = 1;
	entry->dstNetif = netif;
	entry->entryIndex = entryIdx;
	entry->nextHopType = netif->if_type;
	entry->srcIp = srcIp;
	entry->refCnt = 1;
	entry->flag = attr;

	switch(netif->if_type)
	{
		case IF_ETHER:
			entry->nexthop = nexthop;
			break;
		case IF_PPPOE:
		case IF_PPTP:
		case IF_L2TP:
			/*nexthop is sessionId*/
			entry->nexthop = nexthop;
			break;
		
	}

	if(attr == NEXTHOP_L3)
	{
		entry1 = &rtl865x_nxtHopTable[entryIdx+1];
		memcpy(entry1,entry,sizeof(rtl865x_nextHopEntry_t));
		entry1->entryIndex = entryIdx + 1;

		_rtl865x_arrangeNxtHop(entryIdx, 2);

		/*entry1 used netif,update reference netif*/
		rtl865x_referNetif(netif->name);
		/*entry1 used pppoe!, update reference pppoe*/
		if((entry1->nextHopType == IF_PPPOE)
			|| (entry1->nextHopType == IF_PPTP)
			|| (entry1->nextHopType == IF_L2TP)
			)
			rtl865x_referPpp(nexthop);
		
		/*FIXME_hyking:lazy, update the route information right here....*/
		rt_t = (rtl865x_route_t *)ref_ptr;
		rt_t ->un.nxthop.nxtHopSta = entryIdx;
		rt_t ->un.nxthop.nxtHopEnd = entryIdx + 1;
	}
	else
		_rtl865x_arrangeNxtHop(entryIdx, 1);
	
	/*update reference dstnetif&pppoe arp?*/
	rtl865x_referNetif(netif->name);
	if((entry->nextHopType == IF_PPPOE)
			|| (entry->nextHopType == IF_PPTP)
			|| (entry->nextHopType == IF_L2TP)
			)
		rtl865x_referPpp(nexthop);
	
	return SUCCESS;
	
}

static int32 _rtl865x_delNxtHop(uint32 attr, uint32 entryIdx)
{
	rtl865x_nextHopEntry_t *entry;
	//int32 retval = 0;

	if(entryIdx >= NXTHOP_ENTRY_NUM)
		return RTL_EINVALIDINPUT;

	entry = &rtl865x_nxtHopTable[entryIdx];

	if(entry->valid == 0)
		return RTL_EENTRYNOTFOUND;

	if(entry->refCnt > 1)
	{
		printk("%s(%d),refcnt(%d)\n",__FUNCTION__,__LINE__,entry->refCnt);
		return SUCCESS;
	}

	/*now delete the entry*/
	//if(entry->srcIp_t)
		//retval = rtl865x_delIp(entry->srcIp_t->extIp);

	if((entry->nextHopType == IF_PPPOE)
			|| (entry->nextHopType == IF_PPTP)
			|| (entry->nextHopType == IF_L2TP)
			)
	{
		rtl865x_deReferPpp(entry->nexthop);
		
	}

	rtl865x_deReferNetif(entry->dstNetif->name);	
	memset(entry,0,sizeof(rtl865x_nextHopEntry_t));
	
	/*update asic nextHop table*/
	//_rtl865x_arrangeNxtHop(entryIdx,1);

	return SUCCESS;
	
}

#if defined(CONFIG_RTL_MULTIPLE_WAN)
static int32 _rtl865x_synNxtHop_by_arp_entry(rtl865x_nextHopEntry_t *entry_t,rtl865x_arpMapping_entry_t *arp)
{
	rtl865x_tblAsicDrv_nextHopParam_t asic;
	ether_addr_t reservedMac;
	ether_addr_t zero_mac ={{0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
	rtl865x_tblAsicDrv_l2Param_t asic_l2;
	rtl865x_netif_local_t *dstNetif;
	uint32 fid = 0;
	uint32 columIdx = 0;
	int32 retval = 0;
	int32 ipIdx = 0;

	retval = rtl865x_getReserveMacAddr(&reservedMac);
	
	bzero(&asic, sizeof(rtl865x_tblAsicDrv_nextHopParam_t));

	if(entry_t == NULL || arp == NULL)
	{
		return RTL_EINVALIDINPUT;
	}
	
	if (entry_t->nextHopType == IF_ETHER)
	{
		int mac_valid = 0;
		mac_valid = memcmp(arp->mac.octet,zero_mac.octet,ETHER_ADDR_LEN);
		
		rtl865x_getVlanFilterDatabaseId(entry_t->dstNetif->vid,&fid);
		retval = rtl865x_Lookup_fdb_entry(fid, (mac_valid)? &arp->mac : &reservedMac, FDB_DYNAMIC, &columIdx,&asic_l2);
		
		asic.nextHopRow = rtl8651_filterDbIndex( (mac_valid)? &arp->mac : &reservedMac, fid );
		asic.nextHopColumn = (retval == SUCCESS)? columIdx: 0;

		asic.isPppoe = FALSE;
		asic.pppoeIdx = 0;
	}
	else
	{	
		/*session based interface type*/
		rtl865x_ppp_t pppoe;		
		int32 pppidx = 0;

		memset(&pppoe,0,sizeof(rtl865x_ppp_t));
		
		if(entry_t->nexthop)
			retval = rtl865x_getPppBySessionId(entry_t->nexthop,&pppoe);

		rtl865x_getVlanFilterDatabaseId(entry_t->dstNetif->vid,&fid);
		

		retval =rtl865x_Lookup_fdb_entry(fid, (pppoe.valid)? &pppoe.server_mac : &reservedMac, FDB_DYNAMIC, &columIdx,&asic_l2);
			
		asic.nextHopRow = rtl8651_filterDbIndex( (pppoe.valid)? &pppoe.server_mac : &reservedMac, fid);
		asic.nextHopColumn = (pppoe.valid)? columIdx: 0;
		asic.isPppoe = (pppoe.type == IF_PPPOE)? TRUE: FALSE;

		retval = rtl865x_getPppIdx(&pppoe, &pppidx);

		//printk("%s(%d): pppoeIdx(%d), pppoeType(%d), pppoevalid(%d),pppoeSid(%d)\n",__FUNCTION__,__LINE__,pppidx,pppoe.type,pppoe.valid,pppoe.sessionId);
		asic.pppoeIdx	= (pppoe.type == IF_PPPOE)? pppidx: 0;
	}
	
	if(entry_t->dstNetif->is_slave == 1)
	{
		dstNetif = entry_t->dstNetif->master;

		if(dstNetif == NULL)
			dstNetif = _rtl865x_getDefaultWanNetif();
	}
	else
		dstNetif = entry_t->dstNetif;

	if(dstNetif == NULL)
		printk("_%s(%d), BUG!!!!!!",__FUNCTION__,__LINE__);
	
	asic.dvid			= dstNetif->asicIdx;

	if(entry_t->srcIp)
		retval = rtl865x_getIpIdxByExtIp(entry_t->srcIp, &ipIdx);
	
	asic.extIntIpIdx	= entry_t->srcIp? ipIdx: 0;

	//printk("%s(%d), entryIdx(%d),asic.isPPPoe(%d),asic.pppoeIdx(%d),asic.dvid(%d)\n", __FUNCTION__,__LINE__,entry_t->entryIndex,asic.isPppoe,asic.pppoeIdx,asic.dvid);
	rtl8651_setAsicNextHopTable(entry_t->entryIndex,  &asic);
	
	return SUCCESS;
}

int32 rtl865x_eventHandle_addArp_for_multiWan(void *param)
{
	rtl865x_arpMapping_entry_t *arp;
	rtl865x_nextHopEntry_t *entry;
	int32 i;
	
	if(param == NULL)
		return EVENT_CONTINUE_EXECUTE;

	arp = (rtl865x_arpMapping_entry_t *)param;
	entry = rtl865x_nxtHopTable;
		
	for(i = 0; i < NXTHOP_ENTRY_NUM; i++,entry++)
	{
		if(entry->valid && entry->nextHopType == IF_ETHER)
		{
			/*update nexthop*/
			if(entry->nexthop == arp->ip)
			{				
				_rtl865x_synNxtHop_by_arp_entry(entry,arp);
			}
		}		
	}
	return EVENT_CONTINUE_EXECUTE;
}

int32 rtl865x_eventHandle_delArp_for_multiWan(void *param)
{
	rtl865x_arpMapping_entry_t *arp;
	rtl865x_nextHopEntry_t *entry;
	int32 i;

	if(param == NULL)
		return EVENT_CONTINUE_EXECUTE;

	arp = (rtl865x_arpMapping_entry_t *)param;	
	entry = rtl865x_nxtHopTable;
	
	//set mac to 0 for sync nexthop
	memset(arp->mac.octet,0,ETHER_ADDR_LEN);
	
	for(i = 0; i < NXTHOP_ENTRY_NUM; i++,entry++)
	{
		if(entry->valid && entry->nextHopType == IF_ETHER)
		{
			/*update nexthop*/
			if(entry->nexthop == arp->ip)
				_rtl865x_synNxtHop_by_arp_entry(entry,arp);
		}		
	}
	return EVENT_CONTINUE_EXECUTE;
}

#endif

static int32 _rtl865x_eventHandle_addArp(void *param)
{
	rtl865x_arpMapping_entry_t *arp;
	rtl865x_nextHopEntry_t *entry;
	int32 i;

	if(param == NULL)
		return EVENT_CONTINUE_EXECUTE;

	arp = (rtl865x_arpMapping_entry_t *)param;

	entry = rtl865x_nxtHopTable;
	for(i = 0; i < NXTHOP_ENTRY_NUM; i++,entry++)
	{
		if(entry->valid && entry->nextHopType == IF_ETHER)
		{
			/*update nexthop*/
			if(entry->nexthop == arp->ip)
				_rtl865x_synNxtHopToAsic(entry);
		}		
	}
	return EVENT_CONTINUE_EXECUTE;
}

static int32 _rtl865x_eventHandle_delArp(void *param)
{
	rtl865x_arpMapping_entry_t *arp;
	rtl865x_nextHopEntry_t *entry;
	int32 i;

	if(param == NULL)
		return EVENT_CONTINUE_EXECUTE;

	arp = (rtl865x_arpMapping_entry_t *)param;
	
	entry = rtl865x_nxtHopTable;
	for(i = 0; i < NXTHOP_ENTRY_NUM; i++,entry++)
	{
		if(entry->valid && entry->nextHopType == IF_ETHER)
		{
			/*update nexthop*/
			if(entry->nexthop == arp->ip)
				_rtl865x_synNxtHopToAsic(entry);
		}		
	}
	return EVENT_CONTINUE_EXECUTE;
}

static int32 _rtl865x_eventHandle_delPpp(void *param)
{
	rtl865x_ppp_t *pppoe;
	rtl865x_nextHopEntry_t *entry;
	int32 i;

	if(param == NULL)
		return EVENT_CONTINUE_EXECUTE;

	pppoe = (rtl865x_ppp_t *)param;

	entry = rtl865x_nxtHopTable;
	for(i = 0; i < NXTHOP_ENTRY_NUM; i++,entry++)
	{
		if(entry->valid )
			if((entry->nextHopType == IF_PPPOE)
			|| (entry->nextHopType == IF_PPTP)
			|| (entry->nextHopType == IF_L2TP)
			)
			{
				/*update nexthop*/
				if(entry->nexthop == pppoe->sessionId)
				{
					/*update the action(TOCPU)*/
					entry->nexthop  = 0;
					_rtl865x_synNxtHopToAsic(entry);
				}
			}		
	}

	return EVENT_CONTINUE_EXECUTE;
}

static int32 _rtl865x_nextHop_register_event(void)
{
	rtl865x_event_Param_t eventParam;
	eventParam.eventLayerId=DEFAULT_LAYER3_EVENT_LIST_ID;
	eventParam.eventId=EVENT_DEL_ARP;
	eventParam.eventPriority=0;
	eventParam.event_action_fn=_rtl865x_eventHandle_delArp;
	rtl865x_registerEvent(&eventParam);

	memset(&eventParam,0,sizeof(rtl865x_event_Param_t));	
	eventParam.eventLayerId=DEFAULT_LAYER3_EVENT_LIST_ID;
	eventParam.eventId=EVENT_ADD_ARP;
	eventParam.eventPriority=0;
	eventParam.event_action_fn=_rtl865x_eventHandle_addArp;
	rtl865x_registerEvent(&eventParam);

	memset(&eventParam,0,sizeof(rtl865x_event_Param_t));	
	eventParam.eventLayerId=DEFAULT_LAYER3_EVENT_LIST_ID;
	eventParam.eventId=EVENT_DEL_PPP;
	eventParam.eventPriority=0;
	eventParam.event_action_fn=_rtl865x_eventHandle_delPpp;
	rtl865x_registerEvent(&eventParam);
	
	return SUCCESS;

}

static int32 _rtl865x_nextHop_unRegister_event(void)
{
	rtl865x_event_Param_t eventParam;
	eventParam.eventLayerId=DEFAULT_LAYER3_EVENT_LIST_ID;
	eventParam.eventId=EVENT_DEL_ARP;
	eventParam.eventPriority=0;
	eventParam.event_action_fn=_rtl865x_eventHandle_delArp;
	rtl865x_unRegisterEvent(&eventParam);

	memset(&eventParam,0,sizeof(rtl865x_event_Param_t));	
	eventParam.eventLayerId=DEFAULT_LAYER3_EVENT_LIST_ID;
	eventParam.eventId=EVENT_ADD_ARP;
	eventParam.eventPriority=0;
	eventParam.event_action_fn=_rtl865x_eventHandle_addArp;
	rtl865x_unRegisterEvent(&eventParam);

	memset(&eventParam,0,sizeof(rtl865x_event_Param_t));	
	eventParam.eventLayerId=DEFAULT_LAYER3_EVENT_LIST_ID;
	eventParam.eventId=EVENT_DEL_PPP;
	eventParam.eventPriority=0;
	eventParam.event_action_fn=_rtl865x_eventHandle_delPpp;
	rtl865x_unRegisterEvent(&eventParam);
	
	return SUCCESS;

}


/*
@func int32 | rtl865x_addNxtHop |add a nexthop entry
@parm uint32 | attr | attribute. NEXTHOP_L3/NEXTHOP_REDIRECT.
@parm void* | ref_ptr | entry pointer who refer this nexthop entry.
@parm rtl865x_netif_local_t* | netif | network interface.
@parm uint32 | nexthop | nexthop. ip address when linktype is ethernet, session id when linktype is ppp session based.
@rvalue SUCCESS | success.
@rvalue FAILED | failed.
@comm	
*/
int32 rtl865x_addNxtHop(uint32 attr, void *ref_ptr, rtl865x_netif_local_t *netif, uint32 nexthop,uint32 srcIp)
{
	int32 ret = FAILED;
	unsigned long flags;	
	//rtl_down_interruptible(&nxthop_sem);	
	local_irq_save(flags);
	ret = _rtl865x_addNxtHop(attr,ref_ptr,netif,nexthop,srcIp);	
	//rtl_up(&nxthop_sem);	
	local_irq_restore(flags);
	return ret;
}

/*
@func int32 | rtl865x_delNxtHop |delete nexthop entry
@parm uint32 | attr | attribute. NEXTHOP_L3/NEXTHOP_REDIRECT.
@parm uint32 | entryIdx | entry index.
@rvalue SUCCESS | success.
@rvalue FAILED | failed.
@comm	
*/
int32 rtl865x_delNxtHop(uint32 attr, uint32 entryIdx)
{
	int32 retval = FAILED;
	unsigned long flags;	
	//rtl_down_interruptible(&nxthop_sem);
	local_irq_save(flags);
	retval = _rtl865x_delNxtHop(attr,entryIdx);
	//rtl_up(&nxthop_sem);
	local_irq_restore(flags);
	return retval;	
}

#if defined (CONFIG_RTL_LOCAL_PUBLIC) || defined(CONFIG_RTL_MULTIPLE_WAN)
int32 rtl865x_getNxtHopIdx(uint32 attr, rtl865x_netif_local_t *netif, uint32 nexthop)
{
	rtl865x_nextHopEntry_t *entry;
	int32 i;
	
	entry = rtl865x_nxtHopTable;
	for(i = 0; i < NXTHOP_ENTRY_NUM; i++,entry++)
	{
		if(	(entry->valid) &&
			(entry->flag == attr)&&
			(entry->dstNetif == netif) &&
			(entry->nexthop== nexthop))
		{
			return i;
		}		
	}
	return -1;
}

#endif

