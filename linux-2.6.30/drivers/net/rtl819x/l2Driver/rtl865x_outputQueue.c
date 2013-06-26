/*
 *
 *  Copyright (c) 2011 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#include <linux/config.h>
#include <net/rtl/rtl_types.h>
#include <net/rtl/rtl_glue.h>
#include <common/rtl8651_tblDrvProto.h>
#include <common/rtl865x_eventMgr.h>
#include <common/rtl865x_vlan.h>
#include <net/rtl/rtl865x_netif.h>
#include <common/rtl865x_netif_local.h>
#include <net/rtl/rtl865x_outputQueue.h>
//#include "assert.h"
//#include "rtl_utils.h"
#include <common/rtl_errno.h>
#if defined (CONFIG_RTL_LOCAL_PUBLIC)
#include <l3Driver/rtl865x_localPublic.h>
#endif

#ifdef CONFIG_RTL_LAYERED_ASIC_DRIVER
#include <AsicDriver/asicRegs.h>
#include <AsicDriver/rtl865x_asicCom.h>
#include <AsicDriver/rtl865x_asicL2.h>
#else
#include <AsicDriver/asicRegs.h>
#include <AsicDriver/rtl8651_tblAsicDrv.h>
#endif

#if	defined(CONFIG_RTL_HW_QOS_SUPPORT)
#include <net/pkt_cls.h>
#endif

uint8	netIfNameArray[NETIF_NUMBER][IFNAMSIZ] = {{0}};
static int8	(*rtl865x_compFunc)(rtl865x_qos_t	*entry1, rtl865x_qos_t	*entry2);
static uint8	priorityMatrix[RTL8651_OUTPUTQUEUE_SIZE][TOTAL_VLAN_PRIORITY_NUM] = 
								{{0,0,0,0,0,0,0,0},	
								{0,0,0,0,5,5,5,5},	
								{0,0,0,0,1,1,5,5},
								{0,0,0,1,2,2,5,5},
								{0,0,0,1,2,3,5,5},
								{0,0,1,2,3,4,5,5}};

static int32	queueMatrix[RTL8651_OUTPUTQUEUE_SIZE][RTL8651_OUTPUTQUEUE_SIZE] =
								{{0, -1, -1, -1, -1, -1},
								{0, -1, -1, -1, -1, 5},
								{0, 4, -1, -1, -1, 6},
								{0, 3, 4, -1, -1, 6},
								{0, 3, 4, 5, -1, 6},
								{0, 2, 3, 4, 5 ,6}};

static uint8    priorityDecisionArray[] = {	2,		/* port base */
									8,		/*         802.1p base */ 
#if defined (CONFIG_RTK_VOIP_QOS) 
									8,		/*         dscp base */                   
#else
									4,		/*         dscp base */                   
#endif
									8,		/*         acl base */    
									8		/* nat base */
								};

static uint32	defPriority[NETIF_NUMBER] = {0};
static uint32	queueNumber[NETIF_NUMBER] = {0};
static uint32	priority2HandleMapping[NETIF_NUMBER][TOTAL_VLAN_PRIORITY_NUM] = {{0}};
static rtl_qos_mark_info_t	mark2Priority[NETIF_NUMBER][MAX_MARK_NUM_PER_DEV] = {{{0}}};

rtl865x_qos_rule_t		*rtl865x_qosRuleHead = NULL;
extern int hw_qos_init_netlink(void);
static int32 _rtl865x_qosArrangeRuleByNetif(uint8 *netIfName);
	
int32 rtl865x_qosSetBandwidth(uint8 *netIfName, uint32 bps)
{
	uint32	memberPort, wanMemberPort;
	rtl865x_netif_local_t	*netIf, *wanNetIf;
	rtl865x_vlan_entry_t	*vlan;
	uint32	port;
	uint32	asicBandwidth;
	uint32	wanPortAsicBandwidth;

	netIf = _rtl865x_getNetifByName(netIfName);
	if(netIf == NULL)
		return FAILED;
	vlan = _rtl8651_getVlanTableEntry(netIf->vid);
	if(vlan == NULL)
		return FAILED;

	memberPort = vlan->memberPortMask;

	///////////////////////////////////////////////
	/*	Egress bandwidth granularity was 64Kbps	*/
	asicBandwidth = bps>>EGRESS_BANDWIDTH_GRANULARITY_BITLEN;
	if (asicBandwidth>0 && (bps&(1<<(EGRESS_BANDWIDTH_GRANULARITY_BITLEN-1)))!=0)
	{
		asicBandwidth++;
	}

#if defined(CONFIG_RTL_PUBLIC_SSID)
	if(strcmp(netIfName,RTL_GW_WAN_DEVICE_NAME) == 0)
#else
	if(strcmp(netIfName,RTL_DRV_WAN0_NETIF_NAME)==0)
#endif
	{
		//Adjust for wan port egress asic bandwidth
		asicBandwidth+=3;
	}
	if(strcmp(netIfName,RTL_DRV_LAN_NETIF_NAME)==0)
	{
		//Adjust for lan port egress asic bandwidth
		asicBandwidth++;
	}
	////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////
	/*	Ingress bandwidth granularity was 16Kbps	*/
	wanPortAsicBandwidth=bps>>INGRESS_BANDWIDTH_GRANULARITY_BITLEN;
	if (wanPortAsicBandwidth>0 && (bps&(1<<(INGRESS_BANDWIDTH_GRANULARITY_BITLEN-1)))!=0)
	{
		wanPortAsicBandwidth++;
	}

	//Adjust for wan port ingress asic bandwidth
	wanPortAsicBandwidth+=5;

	if(strcmp(netIfName,RTL_DRV_LAN_NETIF_NAME)==0)
	{
		//To set wan port ingress asic bandwidth
#if defined(CONFIG_RTL_PUBLIC_SSID)
		wanNetIf= _rtl865x_getNetifByName(RTL_GW_WAN_DEVICE_NAME);
#else
		wanNetIf= _rtl865x_getNetifByName(RTL_DRV_WAN0_NETIF_NAME);
#endif
		if(wanNetIf != NULL)
		{
			wanMemberPort=rtl865x_getVlanPortMask(wanNetIf->vid);
			for(port=0;port<=CPU;port++)
			{
				if(((1<<port)&wanMemberPort)==0)
					continue;
				rtl8651_setAsicPortIngressBandwidth(port,wanPortAsicBandwidth);
			}
		}
	}
	////////////////////////////////////////////////////////////////
	
	rtl865xC_lockSWCore();
	for(port=0;port<=CPU;port++)
	{
		if(((1<<port)&memberPort)==0)
			continue;
		rtl8651_setAsicPortEgressBandwidth(port, asicBandwidth);
	}
	rtl865xC_waitForOutputQueueEmpty();
	rtl8651_resetAsicOutputQueue();
	rtl865xC_unLockSWCore();
	return SUCCESS;
}

int32 rtl865x_qosFlushBandwidth(uint8 *netIfName)
{
	uint32	memberPort, wanMemberPort;
	rtl865x_netif_local_t	*netIf, *wanNetIf;
	rtl865x_vlan_entry_t	*vlan;
	uint32	port;

	//If flush lan interface qos rules, wan port ingress bandwidth should be set to full speed.
	if(strcmp(netIfName,RTL_DRV_LAN_NETIF_NAME)==0)
	{
#if defined(CONFIG_RTL_PUBLIC_SSID)
		wanNetIf= _rtl865x_getNetifByName(RTL_GW_WAN_DEVICE_NAME);
#else
		wanNetIf= _rtl865x_getNetifByName(RTL_DRV_WAN0_NETIF_NAME);
#endif
		if(wanNetIf != NULL)
		{
			wanMemberPort=rtl865x_getVlanPortMask(wanNetIf->vid);
			for(port=0;port<=CPU;port++)
			{
				if(((1<<port)&wanMemberPort)==0)
					continue;
				rtl8651_setAsicPortIngressBandwidth(port,0);
			}
		}
	}

	netIf = _rtl865x_getNetifByName(netIfName);
	if(netIf == NULL)
		return FAILED;
	vlan = _rtl8651_getVlanTableEntry(netIf->vid);
	if(vlan == NULL)
		return FAILED;

	memberPort = vlan->memberPortMask;

	rtl865xC_lockSWCore();
	for(port=0;port<=CPU;port++)
	{
		if(((1<<port)&memberPort)==0)
			continue;

		rtl8651_setAsicPortEgressBandwidth(port, APR_MASK>>APR_OFFSET);
	}

	rtl865x_raiseEvent(EVENT_FLUSH_QOSRULE, NULL);
	
	rtl865xC_waitForOutputQueueEmpty();
	rtl8651_resetAsicOutputQueue();
	rtl865xC_unLockSWCore();
	return SUCCESS;
}
#if 0
int32 rtl865x_qosGetPriorityByHandle(uint8 *priority, uint32 handle)
{
	int	j;

	for(j=0; j < TOTAL_VLAN_PRIORITY_NUM; j++)
	{
		if (priority2HandleMapping[j] == handle)
		{
			*priority = j;
			break;
		}
	}
	if (j<TOTAL_VLAN_PRIORITY_NUM)
		return SUCCESS;
	else
		return FAILED;
}
#else
static int32 _rtl865x_qosGetPriorityByHandle(int32 idx, uint32 handle)
{
	int	j;

	for(j=0; j < TOTAL_VLAN_PRIORITY_NUM; j++)
	{
		if (priority2HandleMapping[idx][j] == handle)
		{
			return j;
		}
	}

	return 0;
}
#endif

int32 rtl_qosGetPriorityByMark(uint8 *netIfName, int32 mark)
{
	int	netifIdx, i;

	//Especil for ppp0 which attach at eth1
	netifIdx = _rtl865x_getNetifIdxByNameExt(netIfName);

	if (queueNumber[netifIdx] <=1)
		return defPriority[netifIdx];
	
	for(i=0;i<MAX_MARK_NUM_PER_DEV;i++)
	{
		if (mark2Priority[netifIdx][i].mark==mark)
			return mark2Priority[netifIdx][i].priority;
	}

	return defPriority[netifIdx];
}

int32 rtl_qosGetPriorityByVid(int32 vid, int32 mark)
{
	int	netifIdx, i;

	netifIdx = _rtl865x_getNetifIdxByVid(vid);
	if (queueNumber[netifIdx] <=1)
		return defPriority[netifIdx];
	for(i=0;i<MAX_MARK_NUM_PER_DEV;i++)
	{
		if (mark2Priority[netifIdx][i].mark==mark)
			return mark2Priority[netifIdx][i].priority;
	}

	return defPriority[netifIdx];
}

int32 rtl_qosSetPriorityByMark(uint8 *netIfName, int32 mark, int32 handler, int32 enable)
{
	int	netifIdx, i;

	//Especil for ppp0 which attach at eth1
	netifIdx = _rtl865x_getNetifIdxByNameExt(netIfName);

	for(i=0;i<MAX_MARK_NUM_PER_DEV;i++)
	{
		if (mark2Priority[netifIdx][i].mark==mark)
		{
			if (enable==TRUE)
			{
				mark2Priority[netifIdx][i].priority = _rtl865x_qosGetPriorityByHandle(netifIdx, handler);
			}
			else
			{
				mark2Priority[netifIdx][i].mark = 0;
				mark2Priority[netifIdx][i].priority = 0;
			}
			break;
		}
	}

	if (i==MAX_MARK_NUM_PER_DEV&&enable==TRUE)
	{
		for(i=0;i<MAX_MARK_NUM_PER_DEV;i++)
		{
			if (mark2Priority[netifIdx][i].mark==0)
			{
				mark2Priority[netifIdx][i].mark = mark;
				mark2Priority[netifIdx][i].priority = _rtl865x_qosGetPriorityByHandle(netifIdx, handler);
				break;
			}
		}

		if (i==MAX_MARK_NUM_PER_DEV)
		{
			return FAILED;
		}
	}
	return SUCCESS;
}

#if 1
/*
	_rtl865x_compare2Entry Return Value:
		>0		means entry1 > entry2
		<0		means entry1 < entry2
		=0		means entry1 = entry2
*/
static int8 _rtl865x_compare2Entry(rtl865x_qos_t	*entry1, rtl865x_qos_t	*entry2)
{
#if 0
	if (entry1->handle>entry2->handle)
		return 1;
	else if (entry1->handle<entry2->handle)
		return -1;
	else
		return 0;
#else
	if (entry1->bandwidth<entry2->bandwidth)
		return 1;
	else if (entry1->bandwidth>entry2->bandwidth)
		return -1;
	else
		return 0;
#endif
}

int32	rtl865x_registerQosCompFunc(int8 (*p_cmpFunc)(rtl865x_qos_t	*entry1, rtl865x_qos_t	*entry2))
{
	if (p_cmpFunc==NULL)
		rtl865x_compFunc = _rtl865x_compare2Entry;
	else
		rtl865x_compFunc = p_cmpFunc;

	return SUCCESS;
}

static int my_gcd(int numA, int numB)
{
	int	tmp;
	int	divisor;

	if (numA<numB)
	{
		tmp = numA;
		numA = numB;
		numB = tmp;
	}

	divisor = numA%numB;
	while(divisor)
	{
		numA = numB;
		numB = divisor;
		divisor = numA%numB;
	}

	return numB;
}

static struct net_device *rtl865x_getDevByName(char *devName)
{
	struct net_device * dev=NULL;

	if(devName != NULL)
	{
		dev=dev_get_by_name(&init_net, devName);
	}
	
	return dev;
}

static int32 _rtl865x_qosArrangeQueue(rtl865x_qos_t *qosInfo)
{
	uint32	queueOrder[RTL8651_OUTPUTQUEUE_SIZE] = {0};
	uint32	entry;
	int32	nStart, nEnd;
	int32	mStart, mEnd;
	int32	i=0, j=0, cnt=0;
	int32	queueNum;
	int32	qosMarkNumIdx;
	uint32	tmpHandle;
	struct net_device *qosDev, * tmpDev;
	rtl865x_qos_t	*outputQueue;
	rtl865x_qos_t	tmp_qosInfo[RTL8651_OUTPUTQUEUE_SIZE];
	const int32	idx = _rtl865x_getNetifIdxByNameExt(qosInfo->ifname);

	/*	Process the queue type & ratio	*/
	{
		int			divisor;

		if ((qosInfo[0].flags&QOS_DEF_QUEUE)!=0)
			divisor = qosInfo[0].ceil;
		else
			divisor = qosInfo[0].bandwidth;

		for(queueNum=0; queueNum<RTL8651_OUTPUTQUEUE_SIZE; queueNum++)
		{
			if ((qosInfo[queueNum].flags&QOS_VALID_MASK)==0)
				break;
			/*	Currently, we set all queue as WFQ		*/
			if ((qosInfo[queueNum].flags&QOS_TYPE_MASK)==QOS_TYPE_WFQ)
			{
				if ((qosInfo[queueNum].flags&QOS_DEF_QUEUE)!=0)
				{
					qosInfo[queueNum].bandwidth=qosInfo[queueNum].ceil;
				}
				divisor = my_gcd(qosInfo[queueNum].bandwidth, divisor);
			}
		}

		/*	process WFQ type ratio	*/
		{
			if (divisor)
			{
				int	maxBandwidth;
				int	queueNumBackup;
				maxBandwidth = 0;
				queueNumBackup = queueNum;

				while(queueNum>0)
				{
					if ((qosInfo[queueNum-1].flags&QOS_TYPE_MASK) 
						==QOS_TYPE_WFQ)
					{
						qosInfo[queueNum-1].bandwidth = (qosInfo[queueNum-1].bandwidth/divisor);
						if (maxBandwidth<qosInfo[queueNum-1].bandwidth)
							maxBandwidth = qosInfo[queueNum-1].bandwidth;
					}
					queueNum--;
				}

				if (maxBandwidth>EGRESS_WFQ_MAX_RATIO)
				{
					queueNum = queueNumBackup;
					divisor = (maxBandwidth/EGRESS_WFQ_MAX_RATIO)
						+ ((maxBandwidth%EGRESS_WFQ_MAX_RATIO)>(EGRESS_WFQ_MAX_RATIO>>1)?1:0);
					while(queueNum>0)
					{
						if ((qosInfo[queueNum-1].flags&QOS_TYPE_MASK) 
							==QOS_TYPE_WFQ)
						{
							qosInfo[queueNum-1].bandwidth = 
								(qosInfo[queueNum-1].bandwidth/divisor);
							
							if (qosInfo[queueNum-1].bandwidth==0)
								qosInfo[queueNum-1].bandwidth = 1;
							else if (qosInfo[queueNum-1].bandwidth>EGRESS_WFQ_MAX_RATIO)
								qosInfo[queueNum-1].bandwidth = EGRESS_WFQ_MAX_RATIO;
						}
						queueNum--;
					}
				}
			}
		}

		divisor = 0;
		for(queueNum=0; queueNum<RTL8651_OUTPUTQUEUE_SIZE; queueNum++)
		{
			if ((qosInfo[queueNum].flags&QOS_DEF_QUEUE)!=0)
			{
				qosInfo[queueNum].bandwidth = 1;
			}
			else if ((qosInfo[queueNum].flags&QOS_VALID_MASK)!=0)
			{
				continue;
			}
			break;
		}
	}
	
	nStart = nEnd = mStart = mEnd = 0;	/*	reserver 0 for default queue	*/
	queueOrder[0] = 0;
	outputQueue = qosInfo;
	
	for(entry=0; entry<RTL8651_OUTPUTQUEUE_SIZE; entry++, outputQueue++)
	{
		if ((outputQueue->flags&QOS_VALID_MASK)==0)
			break;
		
		/*	rtlglue_printf("index %d, queueType %d.\n", entry, outputQueue->queueType); */
		if ((outputQueue->flags&QOS_TYPE_MASK)==QOS_TYPE_WFQ) 
		{
			/*	Do not exceed the max value: 1 ~ 128	*/
			if (outputQueue->bandwidth>((WEIGHT0_MASK>>WEIGHT0_OFFSET)+1))
				outputQueue->bandwidth = (WEIGHT0_MASK>>WEIGHT0_OFFSET)+1;

			/*	this is a NQueue entry	*/
			{
				/*	process m Queue */
				if (mEnd>mStart)
				{
					i = mEnd-1;
					while(i>=mStart)
					{
						queueOrder[i+1] = queueOrder[i];
						i--;
					}
				}
				mEnd++;
				mStart++;	

				/*	process n Queue */
				i = nEnd;
				{
					while(i>nStart)
					{
						if(rtl865x_compFunc(outputQueue, &outputQueue[queueOrder[i-1]])>0)
						{
							queueOrder[i] = queueOrder[i-1];
							i--;
							continue;
						}
						break;
					}
				}
				nEnd++;
				queueOrder[i] = entry;
			}			
		} else if ((outputQueue->flags&QOS_TYPE_MASK)==QOS_TYPE_STR)
		{
			i = mEnd;
			{
				while(i>mStart)
				{
					if (rtl865x_compFunc(outputQueue, &outputQueue[queueOrder[i-1]])>0)
					{
						queueOrder[i] = queueOrder[i-1];
						i--;
						continue;
					}
					break;
				}
			}
			mEnd++;
			queueOrder[i] = entry;
		}
	}

	queueNumber[idx] = mEnd;
	
	queueNum = 1;
	for(i=0;i<NETIF_NUMBER;i++)
	{
		if (queueNum<queueNumber[i])
			queueNum = queueNumber[i];
	}

	memset((void*)tmp_qosInfo, 0, RTL8651_OUTPUTQUEUE_SIZE*sizeof(rtl865x_qos_t));

	/*	Record the priority <-> handle mapping relationship	*/
	for(i=0;i<mEnd;i++)
	{
		cnt = -1;
		for(j=0;j<RTL8651_OUTPUTQUEUE_SIZE;j++)
		{
			if (queueMatrix[queueNum-1][j]>=0)
				cnt++;
			else
				continue;

			if (cnt==i)
			{
				memcpy(&tmp_qosInfo[j], &qosInfo[queueOrder[i]], sizeof(rtl865x_qos_t));
				priority2HandleMapping[idx][queueMatrix[queueNum-1][j]] = tmp_qosInfo[j].handle;
				tmp_qosInfo[j].queueId = j;
			}
		}
	}

	memcpy(qosInfo, tmp_qosInfo, RTL8651_OUTPUTQUEUE_SIZE*sizeof(rtl865x_qos_t));

	/*	Set default priority	*/
	for(i=0;i<RTL8651_OUTPUTQUEUE_SIZE;i++)
	{
		if (!(qosInfo[i].flags&QOS_DEF_QUEUE))
			continue;
		
		for(j=0; j < TOTAL_VLAN_PRIORITY_NUM; j++)
		{
			if (priority2HandleMapping[idx][j] == qosInfo[i].handle)
				break;
		}

		/*	If we do not find the default queue priority
		  *	just keep the default priority 0
		  */
		if (j==TOTAL_VLAN_PRIORITY_NUM)
			j = 0;

		/*	Set default queue priority	*/
		defPriority[idx] = j;
	}

	//To update mark2Priority
	qosDev=rtl865x_getDevByName(qosInfo->ifname);
	if(qosDev)
	{
		for(qosMarkNumIdx=0;qosMarkNumIdx<MAX_MARK_NUM_PER_DEV;qosMarkNumIdx++)
		{
			if(mark2Priority[idx][qosMarkNumIdx].mark != 0)
			{
				if(tc_getHandleByKey(mark2Priority[idx][qosMarkNumIdx].mark, &tmpHandle, qosDev, &tmpDev) == 0)
				{
					mark2Priority[idx][qosMarkNumIdx].priority=_rtl865x_qosGetPriorityByHandle(idx, tmpHandle);
				}
				else
				{
					mark2Priority[idx][qosMarkNumIdx].mark=0;
				}
			}
		}

		dev_put(qosDev);
	}
	
	return SUCCESS;
}
#endif

int32 rtl865x_qosProcessQueue(uint8 *netIfName, rtl865x_qos_t *qosInfo)
{
	uint32	memberPort;
	rtl865x_netif_local_t	*netIf;
	rtl865x_vlan_entry_t	*vlan;
	uint32	port, queue, i;
	uint32	queueNum;
	rtl865x_qos_t		*tmp_info;
	int32	asicBandwidth;

	if (qosInfo==NULL)
		return FAILED;
	
	netIf = _rtl865x_getNetifByName(netIfName);
	if(netIf == NULL)
		return FAILED;
	vlan = _rtl8651_getVlanTableEntry(netIf->vid);
	if(vlan == NULL)
		return FAILED;

	rtl865xC_lockSWCore();
	rtl865x_closeQos(netIfName);
	
	memberPort = vlan->memberPortMask;
	tmp_info = qosInfo;
	queueNum = 0;
	
	_rtl865x_qosArrangeQueue(qosInfo);
	_rtl865x_qosArrangeRuleByNetif(netIfName);

	/*	Since we use napt base priority, 
	*	the queue number of each network interface should be the same.
	*	So we had to select the max queue number to set to each netif.
	*/
	queueNum = 1;
	for(i=0;i<NETIF_NUMBER;i++)
	{
		if (queueNum<queueNumber[i])
			queueNum = queueNumber[i];
	}

	for(port=0;port<=CPU;port++)
	{
		if(((1<<port)&memberPort)==0)
			continue;

		rtl8651_setAsicOutputQueueNumber(port, queueNum);

		for (queue=0;queue<RTL8651_OUTPUTQUEUE_SIZE;queue++)
		{
			if ((qosInfo[queue].flags&QOS_VALID_MASK)==0 || qosInfo[queue].ceil==0)	/*	un-used queue	*/
				continue;

			/*	Egress bandwidth granularity was 64Kbps	*/
			asicBandwidth = ((qosInfo[queue].ceil)>>(EGRESS_BANDWIDTH_GRANULARITY_BITLEN)) - 1;
			if ((qosInfo[queue].ceil)&(1<<(EGRESS_BANDWIDTH_GRANULARITY_BITLEN-1)))
				asicBandwidth += 1;
#if 0
			if (asicBandwidth<EGRESS_WFQ_MAX_RATIO && ((qosInfo[queue].ceil<<3)&(1<<(EGRESS_BANDWIDTH_GRANULARITY_BITLEN-1)))!=0)
			{
				asicBandwidth++;
			}
#endif

#if 0
			if(qosInfo[queue].bandwidth==1)
			{
				//Default queue
				rtl8651_setAsicQueueRate(port, qosInfo[queue].queueId, 
					PPR_MASK>>PPR_OFFSET, 
					L1_MASK>>L1_OFFSET, 
					asicBandwidth);
			}
			else
#endif
			if ((qosInfo[queue].flags&QOS_TYPE_MASK)==QOS_TYPE_STR)
			{
				rtl8651_setAsicQueueRate(port, qosInfo[queue].queueId, 
					PPR_MASK>>PPR_OFFSET, 
					L1_MASK>>L1_OFFSET, 
					asicBandwidth);
				rtl8651_setAsicQueueWeight(port, qosInfo[queue].queueId, STR_PRIO, 0);
			}
			else
			{
				rtl8651_setAsicQueueRate(port, qosInfo[queue].queueId, 
					1>>PPR_OFFSET, 
					L1_MASK>>L1_OFFSET, 
					asicBandwidth);
				rtl8651_setAsicQueueWeight(port, qosInfo[queue].queueId, WFQ_PRIO, qosInfo[queue].bandwidth-1);
			}
		}
	}

	rtl865xC_waitForOutputQueueEmpty();
	rtl8651_resetAsicOutputQueue();
	rtl865xC_unLockSWCore();

	return SUCCESS;
}

int32 rtl865x_qosArrangeRuleByNetif(void)
{
	int		i;

	for(i=0;i<NETIF_NUMBER;i++)
	{
		if (netIfNameArray[i][0]!=0)
		{
			rtl865x_flush_allAcl_fromChain(netIfNameArray[i], RTL865X_ACL_QOS_USED0, RTL865X_ACL_INGRESS);
			rtl865x_flush_allAcl_fromChain(netIfNameArray[i], RTL865X_ACL_QOS_USED1, RTL865X_ACL_INGRESS);
		}
	}

	for(i=0;i<NETIF_NUMBER;i++)
	{
		if (netIfNameArray[i][0]!=0)
		{
			_rtl865x_qosArrangeRuleByNetif(netIfNameArray[i]);
		}
	}

	return SUCCESS;
}

static int32 _rtl865x_qosArrangeRuleByNetif(uint8 *netIfName)
{
	rtl865x_qos_rule_t	*qosRule;
	int32			priority;
	int32			i;
	const int32	idx = _rtl865x_getNetifIdxByNameExt(netIfName);
	int32			tmp_idx;

	for(qosRule = rtl865x_qosRuleHead; qosRule; qosRule=qosRule->next)
	{
		if (qosRule->handle==0)
			continue;
		if ((priority=_rtl865x_qosGetPriorityByHandle(idx, qosRule->handle))==0)
			continue;

		if(qosRule->outIfname[0]!='\0')
		{
			/*	assigned egress netif	*/
			tmp_idx = _rtl865x_getNetifIdxByNameExt(qosRule->outIfname);
			if (tmp_idx!=idx)
				continue;
		}

#if	defined(CONFIG_RTL_QOS_8021P_SUPPORT)
		if (qosRule->rule->ruleType_==RTL865X_ACL_802D1P)
		{
			rtl8651_setAsicDot1qAbsolutelyPriority(qosRule->rule->vlanTagPri_, priority);
			continue;
		}
#endif
		{
			qosRule->rule->priority_ = priority;
			if (qosRule->inIfname[0]!='\0')
			{
				/*	assigned ingress netif	*/
				rtl865x_del_acl(qosRule->rule, qosRule->inIfname, RTL865X_ACL_QOS_USED0);
				rtl865x_add_acl(qosRule->rule, qosRule->inIfname, RTL865X_ACL_QOS_USED0);
				#if defined (CONFIG_RTL_LOCAL_PUBLIC)
				#if defined(CONFIG_RTL_PUBLIC_SSID)
				if (memcmp(RTL_GW_WAN_DEVICE_NAME, qosRule->inIfname, 4)==0
					&&qosRule->rule->direction_==RTL_ACL_INGRESS)
				#else
				if (memcmp(RTL_DRV_WAN0_NETIF_NAME, qosRule->inIfname, 4)==0
					&&qosRule->rule->direction_==RTL_ACL_INGRESS)
				#endif
				{
					rtl_checkLocalPublicNetifIngressRule(qosRule->rule);
				}
				#endif
			}
			else
			{
				/*	do not assigned ingress netif	*/
				for(i=0;i<NETIF_NUMBER;i++)
				{
					if (netIfNameArray[i][0]!=0)
					{
						tmp_idx = _rtl865x_getNetifIdxByNameExt(netIfNameArray[i]);
						if (tmp_idx>=0&&tmp_idx<NETIF_NUMBER&&tmp_idx!=idx)
						{
							rtl865x_del_acl(qosRule->rule, netIfNameArray[i], RTL865X_ACL_QOS_USED0);
							rtl865x_add_acl(qosRule->rule, netIfNameArray[i], RTL865X_ACL_QOS_USED0);
						}
					}
				}
				#if defined (CONFIG_RTL_LOCAL_PUBLIC)
				if (qosRule->rule->direction_==RTL_ACL_INGRESS)
				{
					rtl_checkLocalPublicNetifIngressRule(qosRule->rule);
				}
				#endif
			}
		}
	}

	/*	Add default priority	*/
	{
		rtl865x_AclRule_t	aclRule;
		int	i;
		
		memset(&aclRule, 0, sizeof(rtl865x_AclRule_t));
		aclRule.actionType_ = RTL865X_ACL_PRIORITY;
		aclRule.pktOpApp_ = RTL865X_ACL_ALL_LAYER;
		aclRule.priority_ = defPriority[idx];
		i = 0;
		for(i=0;i<NETIF_NUMBER;i++)
		{
			if (netIfNameArray[i][0]!=0)
			{
				tmp_idx = _rtl865x_getNetifIdxByNameExt(netIfNameArray[i]);
				if (tmp_idx>=0&&tmp_idx<NETIF_NUMBER&&tmp_idx!=idx)
				{
					rtl865x_add_acl(&aclRule, netIfNameArray[i], RTL865X_ACL_QOS_USED1);
				}
			}
		}
	}

	rtl865x_raiseEvent(EVENT_CHANGE_QOSRULE, NULL);
	
	return SUCCESS;
}

int32 rtl865x_qosAddMarkRule(rtl865x_qos_rule_t *rule)
{
	rtl865x_AclRule_t *qosAclRule;
	int				i;

	qosAclRule = kmalloc(sizeof(rtl865x_AclRule_t), GFP_ATOMIC);
	if (qosAclRule==NULL)
	{
		return RTL_ENOFREEBUFFER;
	}
	memcpy(qosAclRule, rule->rule, sizeof(rtl865x_AclRule_t));
	qosAclRule->pktOpApp_ = RTL865X_ACL_ALL_LAYER;
	qosAclRule->actionType_ = RTL865X_ACL_PRIORITY;
	qosAclRule->aclIdx = 0;

	if (rtl865x_qosRuleHead==NULL)
	{
		rtl865x_qosRuleHead = rtl_malloc(sizeof(rtl865x_qos_rule_t));
		if (rtl865x_qosRuleHead==NULL)
		{
			rtl_free(qosAclRule);
			return RTL_ENOFREEBUFFER;
		}
		memcpy(rtl865x_qosRuleHead, rule, sizeof(rtl865x_qos_rule_t));
		rtl865x_qosRuleHead->rule = qosAclRule;
		rtl865x_qosRuleHead->next = NULL;
	}
	else
	{
		rtl865x_qos_rule_t	*qosRule, *lastQosRule;

		qosRule = rtl_malloc(sizeof(rtl865x_qos_rule_t));
		if (qosRule==NULL)
		{
			rtl_free(qosAclRule);
			return RTL_ENOFREEBUFFER;
		}
		lastQosRule = rtl865x_qosRuleHead;
		while(lastQosRule->next)
			lastQosRule = lastQosRule->next;

		lastQosRule->next = qosRule;
		memcpy(qosRule, rule, sizeof(rtl865x_qos_rule_t));
		qosRule->rule = qosAclRule;
		qosRule->next = NULL;
	}

	for(i=0;i<NETIF_NUMBER;i++)
	{
		if (netIfNameArray[i][0]!=0)
		{
			_rtl865x_qosArrangeRuleByNetif(netIfNameArray[i]);
		}
	}
	
	return SUCCESS;
}

int32 rtl865x_qosCheckNaptPriority(rtl865x_AclRule_t *qosRule)
{
	rtl865x_AclRule_t		*rule_p;

	rule_p = rtl865x_matched_layer4_aclChain(netIfNameArray[qosRule->netifIdx_], RTL865X_ACL_QOS_USED0, RTL865X_ACL_INGRESS, qosRule);

	if (!rule_p)
		rule_p = rtl865x_matched_layer2_aclChain(netIfNameArray[qosRule->netifIdx_], RTL865X_ACL_QOS_USED0, RTL865X_ACL_INGRESS, qosRule);

	if (rule_p)
	{
		qosRule->priority_ = rule_p->priority_;
		qosRule->aclIdx = rule_p->aclIdx;
		qosRule->upDown_=rule_p->upDown_;
		return SUCCESS;
	}
	else
	{
		qosRule->priority_ = defPriority[qosRule->netifIdx_];
		return FAILED;
	}
}

int32 rtl865x_qosFlushMarkRule(void)
{
	rtl865x_qos_rule_t	*qosRule;
	int	i;

	while(rtl865x_qosRuleHead)
	{
		qosRule = rtl865x_qosRuleHead->next;
		rtl_free(rtl865x_qosRuleHead->rule);
		rtl_free(rtl865x_qosRuleHead);
		rtl865x_qosRuleHead = qosRule;
	}

	for(i=0;i<NETIF_NUMBER;i++)
	{
		if (netIfNameArray[i][0]!=0)
		{
			rtl865x_flush_allAcl_fromChain(netIfNameArray[i], RTL865X_ACL_QOS_USED0, RTL865X_ACL_INGRESS);
			rtl865x_flush_allAcl_fromChain(netIfNameArray[i], RTL865X_ACL_QOS_USED1, RTL865X_ACL_INGRESS);
			_rtl865x_qosArrangeRuleByNetif(netIfNameArray[i]);
		}
	}

	rtl865x_raiseEvent(EVENT_FLUSH_QOSRULE, NULL);
	
	return SUCCESS;
}

#if defined (CONFIG_RTL_HW_QOS_SUPPORT)	// sync from voip customer for multiple ppp
int32 rtl865x_qosFlushMarkRuleByDev(uint8 *netIfName)
{
	int i;
	rtl865x_qosFlushMarkRule();
	for(i=0;i<NETIF_NUMBER;i++)
	{
		if (netIfNameArray[i][0]!=0 && (strcmp(netIfName,netIfNameArray[i])!=0))
		{
			_rtl865x_qosArrangeRuleByNetif(netIfNameArray[i]);
		}
	}

	return SUCCESS;
}

int32 rtl865x_qosRearrangeRule(void)
{
	int i;
	rtl865x_qosFlushMarkRule();
	for(i=0;i<NETIF_NUMBER;i++)
	{
		if (netIfNameArray[i][0]!=0)
		{
			_rtl865x_qosArrangeRuleByNetif(netIfNameArray[i]);
		}
	}

	return SUCCESS;
}
#endif

int32 rtl865x_closeQos(uint8 *netIfName)
{
	uint32	memberPort;
	rtl865x_netif_local_t	*netIf;
	rtl865x_vlan_entry_t	*vlan;
	uint32	i, port;
	const int32	idx = _rtl865x_getNetifIdxByNameExt(netIfName);
	int32		tmp_idx;

	netIf = _rtl865x_getNetifByName(netIfName);
	if(netIf == NULL)
		return FAILED;
	vlan = _rtl8651_getVlanTableEntry(netIf->vid);
	if(vlan == NULL)
		return FAILED;
	/* clear all acl rules */
#if	defined(CONFIG_RTL_QOS_8021P_SUPPORT)
	rtl8651_flushAsicDot1qAbsolutelyPriority();
#endif
	for(i=0;i<NETIF_NUMBER;i++)
	{
		if (netIfNameArray[i][0]!=0)
		{
			tmp_idx = _rtl865x_getNetifIdxByNameExt(netIfNameArray[i]);
			if (tmp_idx>=0&&tmp_idx<NETIF_NUMBER&&tmp_idx!=idx)
			{
				rtl865x_flush_allAcl_fromChain(netIfNameArray[i], RTL865X_ACL_QOS_USED0, RTL865X_ACL_INGRESS);
				rtl865x_flush_allAcl_fromChain(netIfNameArray[i], RTL865X_ACL_QOS_USED1, RTL865X_ACL_INGRESS);
			}
		}
	}

	
	defPriority[idx] = 0;
	queueNumber[idx] = 0;
	memset(priority2HandleMapping[idx], 0, TOTAL_VLAN_PRIORITY_NUM*sizeof(int));
	for(i=0;i<MAX_MARK_NUM_PER_DEV;i++)
		mark2Priority[idx][i].priority=0;	//Only clear priority here, mark need to be stored here.

	/* keep the rule of RTL865X_ACL_QOS_USED2 untouched */

	memberPort = vlan->memberPortMask;

	rtl865xC_lockSWCore();
	for(port=0;port<=CPU;port++)
	{
		if(((1<<port)&memberPort)==0)
			continue;
		
		for(i=0;i<RTL8651_OUTPUTQUEUE_SIZE;i++)
		{
			rtl8651_setAsicQueueWeight(port, i, STR_PRIO, 0);
			rtl8651_setAsicQueueRate(port,i, PPR_MASK>>PPR_OFFSET, L1_MASK>>L1_OFFSET,APR_MASK>>APR_OFFSET);
		}

		rtl8651_setAsicOutputQueueNumber(port, 1);
	}

	rtl865xC_waitForOutputQueueEmpty();
	rtl8651_resetAsicOutputQueue();
	rtl865xC_unLockSWCore();
	return SUCCESS;
}

int __init rtl865x_initOutputQueue(uint8 **netIfName)
{
	int	i,j;

	rtl865xC_lockSWCore();
	for(i=0;i<RTL8651_OUTPUTQUEUE_SIZE;i++)
	{
		for(j=0;j<TOTAL_VLAN_PRIORITY_NUM;j++)
			rtl8651_setAsicPriorityToQIDMappingTable(i+1, j, priorityMatrix[i][j]);
	}
#if 0
	for(i=CPU;i<=MULTEXT;i++)
	{
		/* mapping all priority to cpu port output queue 0	*/
		for(j=0;j<TOTAL_VLAN_PRIORITY_NUM;j++)
			if(j < 4)
				rtl8651_setAsicCPUPriorityToQIDMappingTable(i, j, 0);
			else
				rtl8651_setAsicCPUPriorityToQIDMappingTable(i,j,5);
	}
#endif
		for(j=0;j<8;j++)
			rtl8651_setAsicCPUPriorityToQIDMappingTable(CPU, j, j<4?0:5);
	
	rtl8651_setAsicPriorityDecision(priorityDecisionArray[PORT_BASE], 
		priorityDecisionArray[D1P_BASE], priorityDecisionArray[DSCP_BASE], 
		priorityDecisionArray[ACL_BASE], priorityDecisionArray[NAT_BASE]);

#if	defined(CONFIG_RTL_QOS_8021P_SUPPORT)
	rtl8651_flushAsicDot1qAbsolutelyPriority();
#endif

	hw_qos_init_netlink();

	for(i =0; i < RTL8651_OUTPUTQUEUE_SIZE; i++)
	{
		for(j=PHY0;j<=CPU;j++)
		{
			/* 1. If disable hw queue flow control, issues as follows:
			  * 	1) 1Gbps ether port to 100Mbps ether port chariot traffic not stable.
			  * 	2) hw qos not so precise
			  * 2. If enable hw queue flow control, note as follows:
			  *	1) for hw qos and using 2 chariot tcp downlink traffic,
			  *      the no-matched traffic throughput will be pull-down because of the hw queue flow control.
			  *
			  * So we enable hw queue flow control as default setting at present.
			  * 2011-04-02, zj
			  */
			if (rtl8651_setAsicQueueFlowControlConfigureRegister(j, i, TRUE)!=SUCCESS)
			{
				QOS_DEBUGP("Set Queue Flow Control Para Error.\n");
			}
		}
	}

	rtl865xC_waitForOutputQueueEmpty();
	rtl8651_resetAsicOutputQueue();
	rtl865xC_unLockSWCore();
	
	memcpy(netIfNameArray, netIfName, NETIF_NUMBER*IFNAMSIZ);

	for(i=0;i<NETIF_NUMBER;i++)
	{
		if (netIfNameArray[i][0]!=0)
		{
			rtl865x_regist_aclChain(netIfNameArray[i], RTL865X_ACL_QOS_USED0, RTL865X_ACL_INGRESS);
			rtl865x_regist_aclChain(netIfNameArray[i], RTL865X_ACL_QOS_USED1, RTL865X_ACL_INGRESS);
		}
	}

	memset(defPriority, 0, NETIF_NUMBER*sizeof(uint32));
	memset(queueNumber, 0, NETIF_NUMBER*sizeof(uint32));
	memset(priority2HandleMapping, 0, NETIF_NUMBER*TOTAL_VLAN_PRIORITY_NUM*sizeof(int));
	memset(mark2Priority, 0, NETIF_NUMBER*MAX_MARK_NUM_PER_DEV*sizeof(rtl_qos_mark_info_t));

	rtl865x_qosRuleHead = NULL;
	if (rtl865x_compFunc==NULL)
		rtl865x_compFunc = _rtl865x_compare2Entry;
	
	return SUCCESS;
}

void __exit rtl865x_exitOutputQueue(void)
{
	int	i;

	for(i=0;i<NETIF_NUMBER;i++)
	{
		if (netIfNameArray[i][0]!=0)
		{
			rtl865x_unRegist_aclChain(netIfNameArray[i], RTL865X_ACL_QOS_USED0, RTL865X_ACL_INGRESS);
			rtl865x_unRegist_aclChain(netIfNameArray[i], RTL865X_ACL_QOS_USED1, RTL865X_ACL_INGRESS);
			rtl865x_qosFlushBandwidth(netIfNameArray[i]);
			rtl865x_closeQos(netIfNameArray[i]);
		}
	}
}

#if defined (CONFIG_RTL_HW_QOS_SUPPORT) && defined(CONFIG_RTL_QOS_PATCH)
void rtl865x_reinitOutputQueuePatchForQoS(uint32 qosEnabled)
{
	int32 port, queue;
	int32 i;
	int32 tmp_idx;
	rtl865x_AclRule_t	aclRule;

	rtl865xC_lockSWCore();
	
	if(qosEnabled == TRUE)
	{
		/* qos enabled */
		for(i=0;i<NETIF_NUMBER;i++)
		{
			queueNumber[i]=DEF_QUEUE_NUM;
		}
		for ( port = PHY0 ; port <= CPU ; port ++ )
		{
			rtl8651_setAsicOutputQueueNumber(port, DEF_QUEUE_NUM);
			
			for ( queue = QUEUE0 ; queue <= QUEUE5 ; queue ++ )
			{				
				rtl8651_setAsicQueueWeight(port, queue, STR_PRIO, 0);
				rtl8651_setAsicQueueRate(port,queue, PPR_MASK>>PPR_OFFSET, L1_MASK>>L1_OFFSET,APR_MASK>>APR_OFFSET);
	  		}
		}

	}
	else
	{
		/* qos disabled */
		for(i=0;i<NETIF_NUMBER;i++)
		{
			queueNumber[i]=MAX_QOS_PATCH_QUEUE_NUM;
		}

		for ( port = PHY0 ; port <= CPU ; port ++ )
		{				
			rtl8651_setAsicOutputQueueNumber(port, MAX_QOS_PATCH_QUEUE_NUM);
			
			for ( queue = QUEUE0 ; queue <= QUEUE5 ; queue ++ )
			{
				if((queue == QUEUE0) || (queue == QUEUE5))
					rtl8651_setAsicQueueRate(port, queue,PPR_MASK>>PPR_OFFSET, L1_MASK>>L1_OFFSET, APR_MASK>>APR_OFFSET);	// full speed
			}
		}

		// Add ACL rule for tcp dport(80) highest priority 7 to accelarate webpage access when heavy traffic load
		memset(&aclRule, 0, sizeof(rtl865x_AclRule_t));
		aclRule.ruleType_ = RTL865X_ACL_TCP;	
		aclRule.tcpSrcPortUB_=65535;
		aclRule.tcpSrcPortLB_=0;
		aclRule.tcpDstPortUB_=80;
		aclRule.tcpDstPortLB_=80;				
		aclRule.actionType_ = RTL865X_ACL_PRIORITY;
		aclRule.pktOpApp_ = RTL865X_ACL_ALL_LAYER;
		aclRule.priority_ = 7;
		i = 0;
		for(i=0;i<NETIF_NUMBER;i++)	//For all interfaces
		{
			if (netIfNameArray[i][0]!=0)
			{
				tmp_idx = _rtl865x_getNetifIdxByNameExt(netIfNameArray[i]);
				if (tmp_idx>=0&&tmp_idx<NETIF_NUMBER)
				{
					rtl865x_add_acl(&aclRule, netIfNameArray[i], RTL865X_ACL_QOS_USED1);
				}
			}
		}
	}
	
	rtl865xC_waitForOutputQueueEmpty();
	rtl8651_resetAsicOutputQueue();
	rtl865xC_unLockSWCore();
}
#endif

#if defined(CONFIG_RTL_PROC_DEBUG)
int32 rtl865x_show_allQosAcl(void)
{
	rtl865x_qos_rule_t	*qosRule;
	rtl865x_AclRule_t *rule;
	int8 *actionT[] = { "permit", "redirect to ether", "drop", "to cpu", "legacy drop", 
					"drop for log", "mirror", "redirect to pppoe", "default redirect", "mirror keep match", 
					"drop rate exceed pps", "log rate exceed pps", "drop rate exceed bps", "log rate exceed bps","priority "
					};

	for(qosRule = rtl865x_qosRuleHead;qosRule;qosRule=qosRule->next)
	{
		rule = qosRule->rule;
		switch(rule->ruleType_)
		{
			case RTL865X_ACL_MAC:
				printk(" [%d] rule type: %s   rule action: %s\n", rule->aclIdx, "Ethernet", actionT[rule->actionType_]);
				printk("\tether type: %x   ether type mask: %x\n", rule->typeLen_, rule->typeLenMask_);
				printk("\tDMAC: %x:%x:%x:%x:%x:%x  DMACM: %x:%x:%x:%x:%x:%x\n",
						rule->dstMac_.octet[0], rule->dstMac_.octet[1], rule->dstMac_.octet[2],
						rule->dstMac_.octet[3], rule->dstMac_.octet[4], rule->dstMac_.octet[5],
						rule->dstMacMask_.octet[0], rule->dstMacMask_.octet[1], rule->dstMacMask_.octet[2],
						rule->dstMacMask_.octet[3], rule->dstMacMask_.octet[4], rule->dstMacMask_.octet[5]
						);
				
				printk( "\tSMAC: %x:%x:%x:%x:%x:%x  SMACM: %x:%x:%x:%x:%x:%x\n",
						rule->srcMac_.octet[0], rule->srcMac_.octet[1], rule->srcMac_.octet[2],
						rule->srcMac_.octet[3], rule->srcMac_.octet[4], rule->srcMac_.octet[5],
						rule->srcMacMask_.octet[0], rule->srcMacMask_.octet[1], rule->srcMacMask_.octet[2],
						rule->srcMacMask_.octet[3], rule->srcMacMask_.octet[4], rule->srcMacMask_.octet[5]
					);
				break;

			case RTL865X_ACL_IP:
				printk(" [%d] rule type: %s   rule action: %s\n", rule->aclIdx, "IP", actionT[rule->actionType_]);
				printk( "\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (rule->dstIpAddr_>>24),
						((rule->dstIpAddr_&0x00ff0000)>>16), ((rule->dstIpAddr_&0x0000ff00)>>8),
						(rule->dstIpAddr_&0xff), (rule->dstIpAddrMask_>>24), ((rule->dstIpAddrMask_&0x00ff0000)>>16),
						((rule->dstIpAddrMask_&0x0000ff00)>>8), (rule->dstIpAddrMask_&0xff)
						);
				printk("\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (rule->srcIpAddr_>>24),
						((rule->srcIpAddr_&0x00ff0000)>>16), ((rule->srcIpAddr_&0x0000ff00)>>8),
						(rule->srcIpAddr_&0xff), (rule->srcIpAddrMask_>>24), ((rule->srcIpAddrMask_&0x00ff0000)>>16),
						((rule->srcIpAddrMask_&0x0000ff00)>>8), (rule->srcIpAddrMask_&0xff)
						);
				printk("\tTos: %x   TosM: %x   ipProto: %x   ipProtoM: %x   ipFlag: %x   ipFlagM: %x\n",
						rule->tos_, rule->tosMask_, rule->ipProto_, rule->ipProtoMask_, rule->ipFlag_, rule->ipFlagMask_
					);
				
				printk("\t<FOP:%x> <FOM:%x> <http:%x> <httpM:%x> <IdentSdip:%x> <IdentSdipM:%x> \n",
						rule->ipFOP_, rule->ipFOM_, rule->ipHttpFilter_, rule->ipHttpFilterM_, rule->ipIdentSrcDstIp_,
						rule->ipIdentSrcDstIpM_
						);
				printk( "\t<DF:%x> <MF:%x>\n", rule->ipDF_, rule->ipMF_); 
					break;
					
			case RTL865X_ACL_IP_RANGE:
				printk(" [%d] rule type: %s   rule action: %s\n", rule->aclIdx, "IP Range", actionT[rule->actionType_]);
				printk("\tdipU: %d.%d.%d.%d dipL: %d.%d.%d.%d\n", (rule->dstIpAddr_>>24),
						((rule->dstIpAddr_&0x00ff0000)>>16), ((rule->dstIpAddr_&0x0000ff00)>>8),
						(rule->dstIpAddr_&0xff), (rule->dstIpAddrMask_>>24), ((rule->dstIpAddrMask_&0x00ff0000)>>16),
						((rule->dstIpAddrMask_&0x0000ff00)>>8), (rule->dstIpAddrMask_&0xff)
						);
				printk("\tsipU: %d.%d.%d.%d sipL: %d.%d.%d.%d\n", (rule->srcIpAddr_>>24),
						((rule->srcIpAddr_&0x00ff0000)>>16), ((rule->srcIpAddr_&0x0000ff00)>>8),
						(rule->srcIpAddr_&0xff), (rule->srcIpAddrMask_>>24), ((rule->srcIpAddrMask_&0x00ff0000)>>16),
						((rule->srcIpAddrMask_&0x0000ff00)>>8), (rule->srcIpAddrMask_&0xff)
						);
				printk("\tTos: %x   TosM: %x   ipProto: %x   ipProtoM: %x   ipFlag: %x   ipFlagM: %x\n",
						rule->tos_, rule->tosMask_, rule->ipProto_, rule->ipProtoMask_, rule->ipFlag_, rule->ipFlagMask_
						);
				printk("\t<FOP:%x> <FOM:%x> <http:%x> <httpM:%x> <IdentSdip:%x> <IdentSdipM:%x> \n",
						rule->ipFOP_, rule->ipFOM_, rule->ipHttpFilter_, rule->ipHttpFilterM_, rule->ipIdentSrcDstIp_,
						rule->ipIdentSrcDstIpM_
						);
					printk("\t<DF:%x> <MF:%x>\n", rule->ipDF_, rule->ipMF_); 
					break;			
			case RTL865X_ACL_ICMP:
				printk(" [%d] rule type: %s   rule action: %s\n", rule->aclIdx, "ICMP", actionT[rule->actionType_]);
				printk("\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (rule->dstIpAddr_>>24),
						((rule->dstIpAddr_&0x00ff0000)>>16), ((rule->dstIpAddr_&0x0000ff00)>>8),
						(rule->dstIpAddr_&0xff), (rule->dstIpAddrMask_>>24), ((rule->dstIpAddrMask_&0x00ff0000)>>16),
						((rule->dstIpAddrMask_&0x0000ff00)>>8), (rule->dstIpAddrMask_&0xff)
						);
				printk("\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (rule->srcIpAddr_>>24),
						((rule->srcIpAddr_&0x00ff0000)>>16), ((rule->srcIpAddr_&0x0000ff00)>>8),
						(rule->srcIpAddr_&0xff), (rule->srcIpAddrMask_>>24), ((rule->srcIpAddrMask_&0x00ff0000)>>16),
						((rule->srcIpAddrMask_&0x0000ff00)>>8), (rule->srcIpAddrMask_&0xff)
						);
				printk("\tTos: %x   TosM: %x   type: %x   typeM: %x   code: %x   codeM: %x\n",
						rule->tos_, rule->tosMask_, rule->icmpType_, rule->icmpTypeMask_, 
						rule->icmpCode_, rule->icmpCodeMask_);
				break;
			case RTL865X_ACL_ICMP_IPRANGE:
				printk(" [%d] rule type: %s   rule action: %s\n", rule->aclIdx, "ICMP IP RANGE", actionT[rule->actionType_]);
				printk("\tdipU: %d.%d.%d.%d dipL: %d.%d.%d.%d\n", (rule->dstIpAddr_>>24),
						((rule->dstIpAddr_&0x00ff0000)>>16), ((rule->dstIpAddr_&0x0000ff00)>>8),
						(rule->dstIpAddr_&0xff), (rule->dstIpAddrMask_>>24), ((rule->dstIpAddrMask_&0x00ff0000)>>16),
						((rule->dstIpAddrMask_&0x0000ff00)>>8), (rule->dstIpAddrMask_&0xff)
						);
				printk("\tsipU: %d.%d.%d.%d sipL: %d.%d.%d.%d\n", (rule->srcIpAddr_>>24),
						((rule->srcIpAddr_&0x00ff0000)>>16), ((rule->srcIpAddr_&0x0000ff00)>>8),
						(rule->srcIpAddr_&0xff), (rule->srcIpAddrMask_>>24), ((rule->srcIpAddrMask_&0x00ff0000)>>16),
						((rule->srcIpAddrMask_&0x0000ff00)>>8), (rule->srcIpAddrMask_&0xff)
						);
				printk("\tTos: %x   TosM: %x   type: %x   typeM: %x   code: %x   codeM: %x\n",
						rule->tos_, rule->tosMask_, rule->icmpType_, rule->icmpTypeMask_, 
						rule->icmpCode_, rule->icmpCodeMask_);
				break;
			case RTL865X_ACL_IGMP:
				printk(" [%d] rule type: %s   rule action: %s\n", rule->aclIdx, "IGMP", actionT[rule->actionType_]);
				printk("\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (rule->dstIpAddr_>>24),
						((rule->dstIpAddr_&0x00ff0000)>>16), ((rule->dstIpAddr_&0x0000ff00)>>8),
						(rule->dstIpAddr_&0xff), (rule->dstIpAddrMask_>>24), ((rule->dstIpAddrMask_&0x00ff0000)>>16),
						((rule->dstIpAddrMask_&0x0000ff00)>>8), (rule->dstIpAddrMask_&0xff)
						);
				printk("\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (rule->srcIpAddr_>>24),
						((rule->srcIpAddr_&0x00ff0000)>>16), ((rule->srcIpAddr_&0x0000ff00)>>8),
						(rule->srcIpAddr_&0xff), (rule->srcIpAddrMask_>>24), ((rule->srcIpAddrMask_&0x00ff0000)>>16),
						((rule->srcIpAddrMask_&0x0000ff00)>>8), (rule->srcIpAddrMask_&0xff)
						);
				printk("\tTos: %x   TosM: %x   type: %x   typeM: %x\n", rule->tos_, rule->tosMask_,
						rule->igmpType_, rule->igmpTypeMask_
						);
				break;


			case RTL865X_ACL_IGMP_IPRANGE:
				printk(" [%d] rule type: %s   rule action: %s\n", rule->aclIdx, "IGMP IP RANGE", actionT[rule->actionType_]);
				printk("\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (rule->dstIpAddr_>>24),
						((rule->dstIpAddr_&0x00ff0000)>>16), ((rule->dstIpAddr_&0x0000ff00)>>8),
						(rule->dstIpAddr_&0xff), (rule->dstIpAddrMask_>>24), ((rule->dstIpAddrMask_&0x00ff0000)>>16),
						((rule->dstIpAddrMask_&0x0000ff00)>>8), (rule->dstIpAddrMask_&0xff)
						);
				printk("\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (rule->srcIpAddr_>>24),
						((rule->srcIpAddr_&0x00ff0000)>>16), ((rule->srcIpAddr_&0x0000ff00)>>8),
						(rule->srcIpAddr_&0xff), (rule->srcIpAddrMask_>>24), ((rule->srcIpAddrMask_&0x00ff0000)>>16),
						((rule->srcIpAddrMask_&0x0000ff00)>>8), (rule->srcIpAddrMask_&0xff)
						);
				printk("\tTos: %x   TosM: %x   type: %x   typeM: %x\n", rule->tos_, rule->tosMask_,
						rule->igmpType_, rule->igmpTypeMask_
						);
				break;

			case RTL865X_ACL_TCP:
				printk(" [%d] rule type: %s   rule action: %s\n", rule->aclIdx, "TCP", actionT[rule->actionType_]);
				printk("\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (rule->dstIpAddr_>>24),
						((rule->dstIpAddr_&0x00ff0000)>>16), ((rule->dstIpAddr_&0x0000ff00)>>8),
						(rule->dstIpAddr_&0xff), (rule->dstIpAddrMask_>>24), ((rule->dstIpAddrMask_&0x00ff0000)>>16),
						((rule->dstIpAddrMask_&0x0000ff00)>>8), (rule->dstIpAddrMask_&0xff)
						);
				printk("\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (rule->srcIpAddr_>>24),
						((rule->srcIpAddr_&0x00ff0000)>>16), ((rule->srcIpAddr_&0x0000ff00)>>8),
						(rule->srcIpAddr_&0xff), (rule->srcIpAddrMask_>>24), ((rule->srcIpAddrMask_&0x00ff0000)>>16),
						((rule->srcIpAddrMask_&0x0000ff00)>>8), (rule->srcIpAddrMask_&0xff)
						);
				printk("\tTos:%x  TosM:%x  sportL:%d  sportU:%d  dportL:%d  dportU:%d\n",
						rule->tos_, rule->tosMask_, rule->tcpSrcPortLB_, rule->tcpSrcPortUB_,
						rule->tcpDstPortLB_, rule->tcpDstPortUB_
						);
				printk("\tflag: %x  flagM: %x  <URG:%x> <ACK:%x> <PSH:%x> <RST:%x> <SYN:%x> <FIN:%x>\n",
						rule->tcpFlag_, rule->tcpFlagMask_, rule->tcpURG_, rule->tcpACK_,
						rule->tcpPSH_, rule->tcpRST_, rule->tcpSYN_, rule->tcpFIN_
						);
				break;
			case RTL865X_ACL_TCP_IPRANGE:
					printk(" [%d] rule type: %s   rule action: %s\n", rule->aclIdx, "TCP IP RANGE", actionT[rule->actionType_]);
					printk("\tdipU: %d.%d.%d.%d dipL: %d.%d.%d.%d\n", (rule->dstIpAddr_>>24),
						((rule->dstIpAddr_&0x00ff0000)>>16), ((rule->dstIpAddr_&0x0000ff00)>>8),
						(rule->dstIpAddr_&0xff), (rule->dstIpAddrMask_>>24), ((rule->dstIpAddrMask_&0x00ff0000)>>16),
						((rule->dstIpAddrMask_&0x0000ff00)>>8), (rule->dstIpAddrMask_&0xff)
						);
					printk("\tsipU: %d.%d.%d.%d sipL: %d.%d.%d.%d\n", (rule->srcIpAddr_>>24),
						((rule->srcIpAddr_&0x00ff0000)>>16), ((rule->srcIpAddr_&0x0000ff00)>>8),
						(rule->srcIpAddr_&0xff), (rule->srcIpAddrMask_>>24), ((rule->srcIpAddrMask_&0x00ff0000)>>16),
						((rule->srcIpAddrMask_&0x0000ff00)>>8), (rule->srcIpAddrMask_&0xff)
						);
					printk("\tTos:%x  TosM:%x  sportL:%d  sportU:%d  dportL:%d  dportU:%d\n",
						rule->tos_, rule->tosMask_, rule->tcpSrcPortLB_, rule->tcpSrcPortUB_,
						rule->tcpDstPortLB_, rule->tcpDstPortUB_
						);
					printk("\tflag: %x  flagM: %x  <URG:%x> <ACK:%x> <PSH:%x> <RST:%x> <SYN:%x> <FIN:%x>\n",
						rule->tcpFlag_, rule->tcpFlagMask_, rule->tcpURG_, rule->tcpACK_,
						rule->tcpPSH_, rule->tcpRST_, rule->tcpSYN_, rule->tcpFIN_
					);
				break;

			case RTL865X_ACL_UDP:
				printk(" [%d] rule type: %s   rule action: %s\n", rule->aclIdx,"UDP", actionT[rule->actionType_]);
				printk("\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (rule->dstIpAddr_>>24),
						((rule->dstIpAddr_&0x00ff0000)>>16), ((rule->dstIpAddr_&0x0000ff00)>>8),
						(rule->dstIpAddr_&0xff), (rule->dstIpAddrMask_>>24), ((rule->dstIpAddrMask_&0x00ff0000)>>16),
						((rule->dstIpAddrMask_&0x0000ff00)>>8), (rule->dstIpAddrMask_&0xff)
						);
				printk("\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (rule->srcIpAddr_>>24),
						((rule->srcIpAddr_&0x00ff0000)>>16), ((rule->srcIpAddr_&0x0000ff00)>>8),
						(rule->srcIpAddr_&0xff), (rule->srcIpAddrMask_>>24), ((rule->srcIpAddrMask_&0x00ff0000)>>16),
						((rule->srcIpAddrMask_&0x0000ff00)>>8), (rule->srcIpAddrMask_&0xff)
						);
				printk("\tTos:%x  TosM:%x  sportL:%d  sportU:%d  dportL:%d  dportU:%d\n",
						rule->tos_, rule->tosMask_, rule->udpSrcPortLB_, rule->udpSrcPortUB_,
						rule->udpDstPortLB_, rule->udpDstPortUB_
						);
				break;				
			case RTL865X_ACL_UDP_IPRANGE:
				printk(" [%d] rule type: %s   rule action: %s\n", rule->aclIdx, "UDP IP RANGE", actionT[rule->actionType_]);
				printk("\tdipU: %d.%d.%d.%d dipL: %d.%d.%d.%d\n", (rule->dstIpAddr_>>24),
						((rule->dstIpAddr_&0x00ff0000)>>16), ((rule->dstIpAddr_&0x0000ff00)>>8),
						(rule->dstIpAddr_&0xff), (rule->dstIpAddrMask_>>24), ((rule->dstIpAddrMask_&0x00ff0000)>>16),
						((rule->dstIpAddrMask_&0x0000ff00)>>8), (rule->dstIpAddrMask_&0xff)
						);
				printk("\tsipU: %d.%d.%d.%d sipL: %d.%d.%d.%d\n", (rule->srcIpAddr_>>24),
						((rule->srcIpAddr_&0x00ff0000)>>16), ((rule->srcIpAddr_&0x0000ff00)>>8),
						(rule->srcIpAddr_&0xff), (rule->srcIpAddrMask_>>24), ((rule->srcIpAddrMask_&0x00ff0000)>>16),
						((rule->srcIpAddrMask_&0x0000ff00)>>8), (rule->srcIpAddrMask_&0xff)
						);
				printk("\tTos:%x  TosM:%x  sportL:%d  sportU:%d  dportL:%d  dportU:%d\n",
						rule->tos_, rule->tosMask_, rule->udpSrcPortLB_, rule->udpSrcPortUB_,
						rule->udpDstPortLB_, rule->udpDstPortUB_
					);
				break;				

			
			case RTL865X_ACL_SRCFILTER:
				printk(" [%d] rule type: %s   rule action: %s\n", rule->aclIdx, "Source Filter", actionT[rule->actionType_]);
				printk("\tSMAC: %x:%x:%x:%x:%x:%x  SMACM: %x:%x:%x:%x:%x:%x\n", 
						rule->srcFilterMac_.octet[0], rule->srcFilterMac_.octet[1], rule->srcFilterMac_.octet[2], 
						rule->srcFilterMac_.octet[3], rule->srcFilterMac_.octet[4], rule->srcFilterMac_.octet[5],
						rule->srcFilterMacMask_.octet[0], rule->srcFilterMacMask_.octet[1], rule->srcFilterMacMask_.octet[2],
						rule->srcFilterMacMask_.octet[3], rule->srcFilterMacMask_.octet[4], rule->srcFilterMacMask_.octet[5]
						);
				printk("\tsvidx: %d   svidxM: %x   sport: %d   sportM: %x   ProtoType: %x\n",
						rule->srcFilterVlanIdx_, rule->srcFilterVlanIdxMask_, rule->srcFilterPort_, rule->srcFilterPortMask_,
						(rule->srcFilterIgnoreL3L4_==TRUE? 2: (rule->srcFilterIgnoreL4_ == 1? 1: 0))
						);
				printk("\tsip: %d.%d.%d.%d   sipM: %d.%d.%d.%d\n", (rule->srcFilterIpAddr_>>24),
						((rule->srcFilterIpAddr_&0x00ff0000)>>16), ((rule->srcFilterIpAddr_&0x0000ff00)>>8),
						(rule->srcFilterIpAddr_&0xff), (rule->srcFilterIpAddrMask_>>24),
						((rule->srcFilterIpAddrMask_&0x00ff0000)>>16), ((rule->srcFilterIpAddrMask_&0x0000ff00)>>8),
						(rule->srcFilterIpAddrMask_&0xff)
						);
				printk("\tsportL: %d   sportU: %d\n", rule->srcFilterPortLowerBound_, rule->srcFilterPortUpperBound_);
				break;

			case RTL865X_ACL_SRCFILTER_IPRANGE:
				printk(" [%d] rule type: %s   rule action: %s\n", rule->aclIdx, "Source Filter(IP RANGE)", actionT[rule->actionType_]);
				printk("\tSMAC: %x:%x:%x:%x:%x:%x  SMACM: %x:%x:%x:%x:%x:%x\n", 
						rule->srcFilterMac_.octet[0], rule->srcFilterMac_.octet[1], rule->srcFilterMac_.octet[2], 
						rule->srcFilterMac_.octet[3], rule->srcFilterMac_.octet[4], rule->srcFilterMac_.octet[5],
						rule->srcFilterMacMask_.octet[0], rule->srcFilterMacMask_.octet[1], rule->srcFilterMacMask_.octet[2],
						rule->srcFilterMacMask_.octet[3], rule->srcFilterMacMask_.octet[4], rule->srcFilterMacMask_.octet[5]
						);
				printk("\tsvidx: %d   svidxM: %x   sport: %d   sportM: %x   ProtoType: %x\n",
						rule->srcFilterVlanIdx_, rule->srcFilterVlanIdxMask_, rule->srcFilterPort_, rule->srcFilterPortMask_,
						(rule->srcFilterIgnoreL3L4_==TRUE? 2: (rule->srcFilterIgnoreL4_ == 1? 1: 0))
						);
				printk("\tsipU: %d.%d.%d.%d   sipL: %d.%d.%d.%d\n", (rule->srcFilterIpAddr_>>24),
						((rule->srcFilterIpAddr_&0x00ff0000)>>16), ((rule->srcFilterIpAddr_&0x0000ff00)>>8),
						(rule->srcFilterIpAddr_&0xff), (rule->srcFilterIpAddrMask_>>24),
						((rule->srcFilterIpAddrMask_&0x00ff0000)>>16), ((rule->srcFilterIpAddrMask_&0x0000ff00)>>8),
						(rule->srcFilterIpAddrMask_&0xff)
						);
				printk("\tsportL: %d   sportU: %d\n", rule->srcFilterPortLowerBound_, rule->srcFilterPortUpperBound_);
				break;

			case RTL865X_ACL_DSTFILTER:
				printk(" [%d] rule type: %s   rule action: %s\n", rule->aclIdx, "Deatination Filter", actionT[rule->actionType_]);
				printk("\tDMAC: %x:%x:%x:%x:%x:%x  DMACM: %x:%x:%x:%x:%x:%x\n", 
						rule->dstFilterMac_.octet[0], rule->dstFilterMac_.octet[1], rule->dstFilterMac_.octet[2], 
						rule->dstFilterMac_.octet[3], rule->dstFilterMac_.octet[4], rule->dstFilterMac_.octet[5],
						rule->dstFilterMacMask_.octet[0], rule->dstFilterMacMask_.octet[1], rule->dstFilterMacMask_.octet[2],
						rule->dstFilterMacMask_.octet[3], rule->dstFilterMacMask_.octet[4], rule->dstFilterMacMask_.octet[5]
						);
				printk("\tdvidx: %d   dvidxM: %x  ProtoType: %x   dportL: %d   dportU: %d\n",
						rule->dstFilterVlanIdx_, rule->dstFilterVlanIdxMask_, 
						(rule->dstFilterIgnoreL3L4_==TRUE? 2: (rule->dstFilterIgnoreL4_ == 1? 1: 0)), 
						rule->dstFilterPortLowerBound_, rule->dstFilterPortUpperBound_
						);
				printk("\tdip: %d.%d.%d.%d   dipM: %d.%d.%d.%d\n", (rule->dstFilterIpAddr_>>24),
						((rule->dstFilterIpAddr_&0x00ff0000)>>16), ((rule->dstFilterIpAddr_&0x0000ff00)>>8),
						(rule->dstFilterIpAddr_&0xff), (rule->dstFilterIpAddrMask_>>24),
						((rule->dstFilterIpAddrMask_&0x00ff0000)>>16), ((rule->dstFilterIpAddrMask_&0x0000ff00)>>8),
						(rule->dstFilterIpAddrMask_&0xff)
						);
				break;
			case RTL865X_ACL_DSTFILTER_IPRANGE:
				printk(" [%d] rule type: %s   rule action: %s\n", rule->aclIdx, "Deatination Filter(IP Range)", actionT[rule->actionType_]);
				printk("\tDMAC: %x:%x:%x:%x:%x:%x  DMACM: %x:%x:%x:%x:%x:%x\n", 
						rule->dstFilterMac_.octet[0], rule->dstFilterMac_.octet[1], rule->dstFilterMac_.octet[2], 
						rule->dstFilterMac_.octet[3], rule->dstFilterMac_.octet[4], rule->dstFilterMac_.octet[5],
						rule->dstFilterMacMask_.octet[0], rule->dstFilterMacMask_.octet[1], rule->dstFilterMacMask_.octet[2],
						rule->dstFilterMacMask_.octet[3], rule->dstFilterMacMask_.octet[4], rule->dstFilterMacMask_.octet[5]
						);
				printk("\tdvidx: %d   dvidxM: %x  ProtoType: %x   dportL: %d   dportU: %d\n",
						rule->dstFilterVlanIdx_, rule->dstFilterVlanIdxMask_, 
						(rule->dstFilterIgnoreL3L4_==TRUE? 2: (rule->dstFilterIgnoreL4_ == 1? 1: 0)), 
						rule->dstFilterPortLowerBound_, rule->dstFilterPortUpperBound_
						);
				printk("\tdipU: %d.%d.%d.%d   dipL: %d.%d.%d.%d\n", (rule->dstFilterIpAddr_>>24),
						((rule->dstFilterIpAddr_&0x00ff0000)>>16), ((rule->dstFilterIpAddr_&0x0000ff00)>>8),
						(rule->dstFilterIpAddr_&0xff), (rule->dstFilterIpAddrMask_>>24),
						((rule->dstFilterIpAddrMask_&0x00ff0000)>>16), ((rule->dstFilterIpAddrMask_&0x0000ff00)>>8),
						(rule->dstFilterIpAddrMask_&0xff)
					);
				break;

				default:
					printk("rule->ruleType_(0x%x)\n", rule->ruleType_);

		}	

		switch (rule->actionType_) 
		{
			case RTL865X_ACL_PERMIT:
			case RTL865X_ACL_REDIRECT_ETHER:
			case RTL865X_ACL_DROP:
			case RTL865X_ACL_TOCPU:
			case RTL865X_ACL_LEGACY_DROP:
			case RTL865X_ACL_DROPCPU_LOG:
			case RTL865X_ACL_MIRROR:
			case RTL865X_ACL_REDIRECT_PPPOE:
			case RTL865X_ACL_MIRROR_KEEP_MATCH:
				printk("\tnetifIdx: %d   pppoeIdx: %d   l2Idx:%d  ", rule->netifIdx_, rule->pppoeIdx_, rule->L2Idx_);
				break;

			case RTL865X_ACL_PRIORITY: 
				printk("\tprioirty: %d   ",  rule->priority_) ;
				break;
				
			case RTL865X_ACL_DEFAULT_REDIRECT:
				printk("\tnextHop:%d  ",  rule->nexthopIdx_);
				break;

			case RTL865X_ACL_DROP_RATE_EXCEED_PPS:
			case RTL865X_ACL_LOG_RATE_EXCEED_PPS:
			case RTL865X_ACL_DROP_RATE_EXCEED_BPS:
			case RTL865X_ACL_LOG_RATE_EXCEED_BPS:
				printk("\tratelimitIdx: %d  ",  rule->ratelimtIdx_);
				break;
			default: 
				;
			
		}
		printk("pktOpApp: %d	handler:0x%x Mark: %d\n",  rule->pktOpApp_, qosRule->handle, qosRule->mark);
		printk("InDev: %s	OutDev: %s\n", qosRule->inIfname[0]==0?"NULL":qosRule->inIfname, qosRule->outIfname[0]==0?"NULL":qosRule->outIfname);
		
		rule = rule->next;
	}
	
	return SUCCESS;

}
#endif

