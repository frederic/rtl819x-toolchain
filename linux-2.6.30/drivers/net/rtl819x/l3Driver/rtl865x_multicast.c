/*
 *
 *  Copyright (c) 2011 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#ifdef __linux__
#include <linux/config.h>
#include <linux/jiffies.h>
#include <linux/timer.h>
#include <linux/proc_fs.h>
#endif



#include <net/rtl/rtl_types.h>
#include <net/rtl/rtl_glue.h>
//#include "common/rtl_utils.h"
//#include "common/assert.h"

#ifdef CONFIG_RTL_LAYERED_ASIC_DRIVER
#include "AsicDriver/rtl865x_asicCom.h"
#include "AsicDriver/rtl865x_asicL3.h"
#else
#include "AsicDriver/rtl865xC_tblAsicDrv.h" 
#include "common/rtl865x_tblDrvPatch.h"
#endif

#include "AsicDriver/asicRegs.h"
#include "AsicDriver/asicTabs.h"

#include "common/rtl8651_tblDrvProto.h"

#include "common/rtl865x_eventMgr.h"
#include "common/rtl865x_vlan.h"
#include <net/rtl/rtl865x_netif.h>

#include "l3Driver/rtl865x_ip.h"

#ifdef RTL865X_TEST
#include <string.h>
#endif

#include <net/rtl/rtl865x_multicast.h>
#include <net/rtl/rtl865x_igmpsnooping.h>
/********************************************************/
/*			Multicast Related Global Variable			*/
/********************************************************/

static rtl865x_mcast_fwd_descriptor_t *rtl865x_mcastFwdDescPool=NULL;
static mcast_fwd_descriptor_head_t  free_mcast_fwd_descriptor_head;
static struct rtl865x_multicastTable mCastTbl;
static uint32 rtl865x_externalMulticastPortMask = 0;
#if defined(__linux__) && defined(__KERNEL__)
static struct timer_list rtl865x_mCastSysTimer;	/*igmp timer*/
#endif

static int32 _rtl865x_initMCastFwdDescPool(void)
{
	int32 i;


	MC_LIST_INIT(&free_mcast_fwd_descriptor_head);

	TBL_MEM_ALLOC(rtl865x_mcastFwdDescPool, rtl865x_mcast_fwd_descriptor_t,MAX_MCAST_FWD_DESCRIPTOR_CNT);
	
	if(rtl865x_mcastFwdDescPool!=NULL)
	{
		memset( rtl865x_mcastFwdDescPool, 0, MAX_MCAST_FWD_DESCRIPTOR_CNT * sizeof(rtl865x_mcast_fwd_descriptor_t));	
	}
	else
	{
		return FAILED;
	}
	
	for(i = 0; i<MAX_MCAST_FWD_DESCRIPTOR_CNT;i++)
	{
		MC_LIST_INSERT_HEAD(&free_mcast_fwd_descriptor_head, &rtl865x_mcastFwdDescPool[i], next);
	}
	
	return SUCCESS;
}

static rtl865x_mcast_fwd_descriptor_t *_rtl865x_allocMCastFwdDesc(void)
{
	rtl865x_mcast_fwd_descriptor_t *retDesc=NULL;
	retDesc = MC_LIST_FIRST(&free_mcast_fwd_descriptor_head);
	if(retDesc!=NULL)
	{
		MC_LIST_REMOVE(retDesc, next);
		memset(retDesc,0,sizeof(rtl865x_mcast_fwd_descriptor_t));
	}
	return retDesc;
}

static int32 _rtl865x_freeMCastFwdDesc(rtl865x_mcast_fwd_descriptor_t *descPtr)
{
	if(descPtr==NULL)
	{
		return SUCCESS;
	}
	memset(descPtr,0,sizeof(rtl865x_mcast_fwd_descriptor_t));
	MC_LIST_INSERT_HEAD(&free_mcast_fwd_descriptor_head, descPtr, next);
	
	return SUCCESS;
}

static int32 _rtl865x_flushMCastFwdDescChain(mcast_fwd_descriptor_head_t * descChainHead)
{
	rtl865x_mcast_fwd_descriptor_t * curDesc,*nextDesc;
	
	if(descChainHead==NULL)
	{
		return SUCCESS;
	}
	
	curDesc=MC_LIST_FIRST(descChainHead);
	while(curDesc)
	{
		nextDesc=MC_LIST_NEXT(curDesc, next );
		/*remove from the old descriptor chain*/
		MC_LIST_REMOVE(curDesc, next);
		/*return to the free descriptor chain*/
		_rtl865x_freeMCastFwdDesc(curDesc);
		curDesc = nextDesc;
	}

	return SUCCESS;
}



static int32 _rtl865x_mCastFwdDescEnqueue(mcast_fwd_descriptor_head_t * queueHead,
												rtl865x_mcast_fwd_descriptor_t * enqueueDesc)
{

	rtl865x_mcast_fwd_descriptor_t *newDesc;
	rtl865x_mcast_fwd_descriptor_t *curDesc,*nextDesc;
	if(queueHead==NULL)
	{
		return FAILED;
	}
	
	if(enqueueDesc==NULL)
	{
		return SUCCESS;
	}
	
	/*multicast forward descriptor is internal maintained,always alloc new one*/
	newDesc=_rtl865x_allocMCastFwdDesc();
	
	if(newDesc!=NULL)
	{
		memcpy(newDesc, enqueueDesc,sizeof(rtl865x_mcast_fwd_descriptor_t ));
		//memset(&(newDesc->next), 0, sizeof(MC_LIST_ENTRY(rtl865x_mcast_fwd_descriptor_s)));
		newDesc->next.le_next=NULL;
		newDesc->next.le_prev=NULL;
	}
	else
	{
		/*no enough memory*/
		return FAILED;
	}
	

	for(curDesc=MC_LIST_FIRST(queueHead);curDesc!=NULL;curDesc=nextDesc)
	{

		nextDesc=MC_LIST_NEXT(curDesc, next);
		
		/*merge two descriptor*/
	//	if((strcmp(curDesc->netifName,newDesc->netifName)==0) && (curDesc->vid==newDesc->vid))
		if(strcmp(curDesc->netifName,newDesc->netifName)==0)
		{	
			
			if(newDesc->descPortMask==0)
			{
				newDesc->descPortMask=curDesc->descPortMask;
			}
			MC_LIST_REMOVE(curDesc, next);
			_rtl865x_freeMCastFwdDesc(curDesc);
			
		}
	}

	/*not matched descriptor is found*/
	MC_LIST_INSERT_HEAD(queueHead, newDesc, next);

	return SUCCESS;
	
}


static int32 _rtl865x_mergeMCastFwdDescChain(mcast_fwd_descriptor_head_t * targetChainHead ,
													rtl865x_mcast_fwd_descriptor_t *srcChain)
{
	rtl865x_mcast_fwd_descriptor_t *curDesc;

	if(targetChainHead==NULL)
	{
		return FAILED;
	}
	
	for(curDesc=srcChain; curDesc!=NULL; curDesc=MC_LIST_NEXT(curDesc,next))
	{
		
		_rtl865x_mCastFwdDescEnqueue(targetChainHead, curDesc);
		
	}
	
	return SUCCESS;
}




static int32 _rtl865x_initMCastEntryPool(void)
{
	int32 index;
	rtl865x_tblDrv_mCast_t *multiCast_t;
	
	TBL_MEM_ALLOC(multiCast_t, rtl865x_tblDrv_mCast_t ,MAX_MCAST_TABLE_ENTRY_CNT);
	TAILQ_INIT(&mCastTbl.freeList.freeMultiCast);
	for(index=0; index<MAX_MCAST_TABLE_ENTRY_CNT; index++)
	{
		memset( &multiCast_t[index], 0, sizeof(rtl865x_tblDrv_mCast_t));
		TAILQ_INSERT_HEAD(&mCastTbl.freeList.freeMultiCast, &multiCast_t[index], nextMCast);
	}

	TBL_MEM_ALLOC(multiCast_t, rtl865x_tblDrv_mCast_t, RTL8651_MULTICASTTBL_SIZE);
	memset(multiCast_t, 0,RTL8651_MULTICASTTBL_SIZE* sizeof(rtl865x_tblDrv_mCast_t));
	mCastTbl.inuseList.mCastTbl = (void *)multiCast_t;

	for(index=0; index<RTL8651_MULTICASTTBL_SIZE; index++)
	{
		TAILQ_INIT(&mCastTbl.inuseList.mCastTbl[index]);
	}

	return SUCCESS;
}

static rtl865x_tblDrv_mCast_t * _rtl865x_allocMCastEntry(uint32 hashIndex)
{
	rtl865x_tblDrv_mCast_t *newEntry;
	newEntry=TAILQ_FIRST(&mCastTbl.freeList.freeMultiCast);
	if (newEntry == NULL)
	{
		return NULL;
	}		
	
	TAILQ_REMOVE(&mCastTbl.freeList.freeMultiCast, newEntry, nextMCast);

	
	/*initialize it*/
	if(MC_LIST_FIRST(&newEntry->fwdDescChain)!=NULL)
	{
		_rtl865x_flushMCastFwdDescChain(&newEntry->fwdDescChain);
	}
	MC_LIST_INIT(&newEntry->fwdDescChain);
	
	memset(newEntry, 0, sizeof(rtl865x_tblDrv_mCast_t));

	TAILQ_INSERT_TAIL(&mCastTbl.inuseList.mCastTbl[hashIndex], newEntry, nextMCast);
	
	return newEntry;
}

static int32 _rtl865x_flushMCastEntry(rtl865x_tblDrv_mCast_t *mCastEntry)
{
	if(mCastEntry==NULL)
	{
		return SUCCESS;
	}
	
	_rtl865x_flushMCastFwdDescChain(&mCastEntry->fwdDescChain);
	
	memset(mCastEntry, 0, sizeof(rtl865x_tblDrv_mCast_t));
	return SUCCESS;
}

static int32 _rtl865x_freeMCastEntry(rtl865x_tblDrv_mCast_t * mCastEntry, uint32 hashIndex)
{
	if(mCastEntry==NULL)
	{
		return SUCCESS;
	}
	
	TAILQ_REMOVE(&mCastTbl.inuseList.mCastTbl[hashIndex], mCastEntry, nextMCast);
	_rtl865x_flushMCastEntry(mCastEntry);
	TAILQ_INSERT_HEAD(&mCastTbl.freeList.freeMultiCast, mCastEntry, nextMCast);
	return SUCCESS;
}


static uint32 _rtl865x_doMCastEntrySrcVlanPortFilter(rtl865x_tblDrv_mCast_t *mCastEntry)
{
	rtl865x_mcast_fwd_descriptor_t * curDesc,*nextDesc;
	if(mCastEntry==NULL)
	{
		return SUCCESS;
	}
	
	for(curDesc=MC_LIST_FIRST(&mCastEntry->fwdDescChain);curDesc!=NULL;curDesc=nextDesc)
	{
		nextDesc=MC_LIST_NEXT(curDesc, next);
		{
			curDesc->fwdPortMask=curDesc->fwdPortMask & (~(1<<mCastEntry->port));
			if(curDesc->fwdPortMask==0)
			{
				/*remove from the old chain*/
				MC_LIST_REMOVE(curDesc, next);
				/*return to the free descriptor chain*/
				_rtl865x_freeMCastFwdDesc(curDesc);

			}
		}
		
	}

	return SUCCESS;
}

 
static uint32 rtl865x_genMCastEntryAsicFwdMask(rtl865x_tblDrv_mCast_t *mCastEntry)
{
	uint32 asicFwdPortMask=0;
	rtl865x_mcast_fwd_descriptor_t * curDesc;
	if(mCastEntry==NULL)
	{
		return 0;
	}
	
	MC_LIST_FOREACH(curDesc, &(mCastEntry->fwdDescChain), next)
	{
		if(curDesc->toCpu==0)
		{
			asicFwdPortMask|=(curDesc->fwdPortMask & ((1<<RTL8651_MAC_NUMBER)-1));
		}
		else
		{
			asicFwdPortMask|=( 0x01<<RTL8651_MAC_NUMBER);
		}
	}
	asicFwdPortMask = asicFwdPortMask & (~(1<<mCastEntry->port)); 
	return asicFwdPortMask;
}

static uint16 rtl865x_genMCastEntryCpuFlag(rtl865x_tblDrv_mCast_t *mCastEntry)
{
	uint16 cpuFlag=FALSE;
	rtl865x_mcast_fwd_descriptor_t * curDesc;
	if(mCastEntry==NULL)
	{
		return 0;
	}

	if(mCastEntry->cpuHold==TRUE)
	{
		cpuFlag=TRUE;
	}
	
	MC_LIST_FOREACH(curDesc, &(mCastEntry->fwdDescChain), next)
	{
		if(	(curDesc->toCpu==TRUE)	||
			(memcmp(curDesc->netifName, RTL_WLAN_NAME,4)==0)	)
		{
			cpuFlag=TRUE;
		}
	}
	
	return cpuFlag;
}

#if defined (CONFIG_RTL_IGMP_SNOOPING)
/*for linux bridge level igmp snooping usage*/
static uint32 rtl865x_getMCastEntryDescPortMask(rtl865x_tblDrv_mCast_t *mCastEntry)
{
	uint32 descPortMask=0;
	rtl865x_mcast_fwd_descriptor_t * curDesc;
	if(mCastEntry==NULL)
	{
		return 0;
	}
	
	MC_LIST_FOREACH(curDesc, &(mCastEntry->fwdDescChain), next)
	{
		descPortMask=descPortMask | curDesc->descPortMask;
	}
	
	return descPortMask;
}

#endif
/*=======================================
  * Multicast Table APIs
  *=======================================*/
#define RTL865X_MULTICASE_TABLE_APIs

static void  _rtl865x_setASICMulticastPortStatus(void) {
	uint32 index;

	for (index=0; index<RTL8651_PORT_NUMBER+rtl8651_totalExtPortNum; index++) {
		rtl8651_setAsicMulticastPortInternal(index, (rtl865x_externalMulticastPortMask&(1<<index))?FALSE:TRUE);
	}
}

void rtl865x_arrangeMulticastPortStatus(void) {

	rtl865x_externalMulticastPortMask=rtl865x_getExternalPortMask();
	_rtl865x_setASICMulticastPortStatus();
}

/*
@func int32	| rtl865x_addMulticastExternalPort	| API to add a hardware multicast external port.
@parm  uint32 | extPort	| External port number to be added. 
@rvalue SUCCESS	|Add hardware multicast external port successfully.
@rvalue FAILED	|Add hardware multicast external port failed.
*/
int32 rtl865x_addMulticastExternalPort(uint32 extPort)
{
	rtl865x_externalMulticastPortMask |= (1<<extPort);
	_rtl865x_setASICMulticastPortStatus();
	return SUCCESS;
}

/*
@func int32	| rtl865x_delMulticastExternalPort	| API to delete a hardware multicast external port.
@parm  uint32 | extPort	| External port number to be deleted.
@rvalue SUCCESS	|Delete external port successfully.
@rvalue FAILED	|Delete external port failed.
*/
int32 rtl865x_delMulticastExternalPort(uint32 extPort)
{
	rtl865x_externalMulticastPortMask &= ~(1<<extPort);
	_rtl865x_setASICMulticastPortStatus();
	return SUCCESS;
}

/*
@func int32	| rtl865x_setMulticastExternalPortMask	| API to set hardware multicast external port mask.
@parm  uint32 | extPortMask	| External port mask to be set.
@rvalue SUCCESS	|Set external port mask successfully.
@rvalue FAILED	|Set external port mask failed.
*/
int32 rtl865x_setMulticastExternalPortMask(uint32 extPortMask)
{
	rtl865x_externalMulticastPortMask =extPortMask;
	_rtl865x_setASICMulticastPortStatus();
	return SUCCESS;
}

/*
@func int32	| rtl865x_addMulticastExternalPortMask	| API to add hardware multicast external port mask.
@parm  uint32 | extPortMask	| External port mask to be added.
@rvalue SUCCESS	|Add external port mask successfully.
@rvalue FAILED	|Add external port mask failed.
*/
int32 rtl865x_addMulticastExternalPortMask(uint32 extPortMask)
{
	rtl865x_externalMulticastPortMask|= extPortMask;
	_rtl865x_setASICMulticastPortStatus();
	return SUCCESS;
}

/*
@func int32	| rtl865x_delMulticastExternalPortMask	|  API to delete hardware multicast external port mask.
@parm  uint32 | extPortMask	| External port mask to be deleted.
@rvalue SUCCESS	|Delete external port mask successfully.
@rvalue FAILED	|Delete external port mask failed.
*/
int32 rtl865x_delMulticastExternalPortMask(uint32 extPortMask)
{
	rtl865x_externalMulticastPortMask &= ~extPortMask;
	_rtl865x_setASICMulticastPortStatus();
	return SUCCESS;
}

int32 rtl865x_getMulticastExternalPortMask(void)
{
	return rtl865x_externalMulticastPortMask ;
}

static inline void _rtl865x_patchPppoeWeak(rtl865x_tblDrv_mCast_t *mCast_t)
{
	rtl865x_mcast_fwd_descriptor_t * curDesc;
	uint32 netifType;
	/* patch: keep cache in software if one vlan's interface is pppoe */
	MC_LIST_FOREACH(curDesc, &(mCast_t->fwdDescChain), next)
	{
		if(rtl865x_getNetifType(curDesc->netifName, &netifType)==SUCCESS)
		{
			/*how about pptp,l2tp?*/
			if(netifType==IF_PPPOE)
			{
				mCast_t->flag |= RTL865X_MULTICAST_PPPOEPATCH_CPUBIT;
				return;
			}
		}
		
	}

	mCast_t->flag &= ~RTL865X_MULTICAST_PPPOEPATCH_CPUBIT;
}
#if 0
static int _rtl865x_checkMulticastEntryEqual(rtl865x_tblDrv_mCast_t * mCastEntry1, rtl865x_tblDrv_mCast_t * mCastEntry2)
{
	if((mCastEntry1==NULL) && (mCastEntry2==NULL))
	{
		return TRUE;
	}
	
	if((mCastEntry1==NULL) && (mCastEntry2!=NULL))
	{
		return FALSE;
	}

	if((mCastEntry1!=NULL) && (mCastEntry2==NULL))
	{
		return FALSE;
	}
	
	if(mCastEntry1->sip!=mCastEntry2->sip)
	{
		return FALSE;
	}

	if(mCastEntry1->dip!=mCastEntry2->dip)
	{
		return FALSE;
	}

	if(mCastEntry1->svid!=mCastEntry2->svid)
	{
		return FALSE;
	}
	
	if(mCastEntry1->port!=mCastEntry2->port)
	{
		return FALSE;
	}

	if(mCastEntry1->mbr!=mCastEntry2->mbr)
	{
		return FALSE;
	}
	
	if(mCastEntry1->cpu!=mCastEntry2->cpu)
	{
		return FALSE;
	}
	
	if(mCastEntry1->extIp!=mCastEntry2->extIp)
	{
		return FALSE;
	}

	if(mCastEntry1->flag!=mCastEntry2->flag)
	{
		return FALSE;
	}

	
	if(mCastEntry1->inAsic!=mCastEntry2->inAsic)
	{
		return FALSE;
	}			

	return TRUE;
}
#endif
#ifdef CONFIG_PROC_FS
static unsigned int mcastAddOpCnt=0;
unsigned int _rtl865x_getAddMcastOpCnt(void)
{
	return mcastAddOpCnt;
}

static unsigned int mcastDelOpCnt=0;
unsigned int _rtl865x_getDelMcastOpCnt(void)
{
	return mcastDelOpCnt;
}
#endif
/* re-select Multicast entry to ASIC for the index ""entryIndex */
static void _rtl865x_arrangeMulticast(uint32 entryIndex)
{
	rtl865x_tblAsicDrv_multiCastParam_t asic_mcast;
	rtl865x_tblDrv_mCast_t *mCast_t=NULL;
	rtl865x_tblDrv_mCast_t *select_t=NULL;
	rtl865x_tblDrv_mCast_t *swapOutEntry=NULL;
	int32 retval;
	
	TAILQ_FOREACH(mCast_t, &mCastTbl.inuseList.mCastTbl[entryIndex], nextMCast) 
	{
		if ((mCast_t->cpu == 0) && !(mCast_t->flag & RTL865X_MULTICAST_PPPOEPATCH_CPUBIT)) 
		{ /* Ignore cpu=1 */

			if(mCast_t->inAsic==TRUE)
			{
				if(swapOutEntry==NULL)
				{
					swapOutEntry=mCast_t;
				}
				else
				{
					/*impossible, two flow in one asic entry*/
					swapOutEntry->inAsic=FALSE;
					mCast_t->inAsic = FALSE;
				}
			}
			
			if (select_t) 
			{
#if 1
				if ((mCast_t->unKnownMCast==TRUE) && (select_t->unKnownMCast==TRUE))
				{
					/*unknown multicast, select the heavy load*/
					if (mCast_t->maxPPS > select_t->maxPPS)
					{
						select_t = mCast_t;
					}
				}
				else if ((mCast_t->unKnownMCast==FALSE) && (select_t->unKnownMCast==TRUE))
				{
					/*replace unknown multicast*/
					select_t = mCast_t;
				}
				else if((mCast_t->unKnownMCast==FALSE) && (select_t->unKnownMCast==FALSE))
				{
					/*select the heavy load*/
					if (mCast_t->maxPPS > select_t->maxPPS)
					{
						select_t = mCast_t;
					}
				}
				else if((mCast_t->unKnownMCast==TRUE) && (select_t->unKnownMCast==FALSE))
				{
					/*this  stream is unknown multicast,needn't change candidate*/
				}
#else
				if ((mCast_t->mbr==0) && (select_t->mbr==0))
				{
					/*unknown multicast, select the heavy load*/
					if (mCast_t->maxPPS > select_t->maxPPS)
					{
						select_t = mCast_t;
					}
				}
				else if ((mCast_t->mbr!=0) && (select_t->mbr==0))
				{
					/*replace unknown multicast*/
					select_t = mCast_t;
				}
				else if((mCast_t->mbr!=0) && (select_t->mbr!=0))
				{
					/*select the heavy load*/
					if (mCast_t->maxPPS > select_t->maxPPS)
					{
						select_t = mCast_t;
					}
				}
				else if((mCast_t->mbr==0) && (select_t->mbr!=0))
				{
					/*this  stream is unknown multicast,needn't change candidate*/
				}
#endif				
				
				
			}
			else 
			{
				select_t = mCast_t;
			}

			//printk("mCast_t->dip is 0x%x,mCast_t->inAsic is %d,mCast_t->maxPPS is %d\n\n\n",mCast_t->dip,mCast_t->inAsic,mCast_t->maxPPS);
		}
		else
		{
			mCast_t->inAsic = FALSE;	/* reset "inAsic" bit */
		} 


	}
	/*
	if(swapOutEntry)
	{
		printk("%s:%d,swapOutEntry->dip is 0x%x,swapOutEntry->mbr is 0x%x\n",__FUNCTION__,__LINE__,swapOutEntry->dip,swapOutEntry->mbr);

	}
	
	if (select_t) 
	{
		printk("%s:%d,select_t->dip is 0x%x,select_t->mbr is 0x%x\n",__FUNCTION__,__LINE__,select_t->dip,select_t->mbr);
	}
	*/
	if (select_t) 
	{
		if((swapOutEntry==NULL) ||(select_t==swapOutEntry))
		{
			select_t->age = RTL865X_MULTICAST_TABLE_ASIC_AGE;
			bzero(&asic_mcast, sizeof(rtl865x_tblAsicDrv_multiCastParam_t));
			memcpy(&asic_mcast, select_t, (uint32)&(((rtl865x_tblDrv_mCast_t *)0)->extIp));
			if (select_t->extIp)
			{
			
#ifdef CONFIG_RTL_LAYERED_DRIVER_L3
				int32 ipIdx;
				if(rtl865x_getIpIdxByExtIp(select_t->extIp, &ipIdx)==SUCCESS)
				{
					asic_mcast.extIdx=(uint16)ipIdx;
				}
#else
				asic_mcast.extIdx=0;
#endif
		
			}
			retval = rtl8651_setAsicIpMulticastTable(&asic_mcast);
			
#ifdef CONFIG_PROC_FS
			mcastAddOpCnt++;
#endif
			assert(retval == SUCCESS);
			if(retval==SUCCESS)
			{
				select_t->inAsic = TRUE;
			}
			else
			{
				select_t->inAsic = FALSE;
				rtl8651_delAsicIpMulticastTable(entryIndex);
#ifdef CONFIG_PROC_FS
				mcastDelOpCnt++;
#endif
			}
				
			assert(retval == SUCCESS);
			TAILQ_REMOVE(&mCastTbl.inuseList.mCastTbl[entryIndex], select_t, nextMCast);
			TAILQ_INSERT_HEAD(&mCastTbl.inuseList.mCastTbl[entryIndex], select_t, nextMCast);
		}
		else/*(swapOutEntry!=NULL) && (select_t!=swapOutEntry)*/
		{
			/*disable swap and only explicit joined mulicast flow can replace unknown multicast flow*/
			if((swapOutEntry->unKnownMCast==TRUE) &&(select_t->unKnownMCast==FALSE))
			{
				/*don't forget to set swapOutEntry's inAsic flag*/
				swapOutEntry->inAsic=FALSE;
				
				select_t->age = RTL865X_MULTICAST_TABLE_ASIC_AGE;
				bzero(&asic_mcast, sizeof(rtl865x_tblAsicDrv_multiCastParam_t));
				memcpy(&asic_mcast, select_t, (uint32)&(((rtl865x_tblDrv_mCast_t *)0)->extIp));

				if (select_t->extIp)
				{
#ifdef CONFIG_RTL_LAYERED_DRIVER_L3		
					int32 ipIdx;
					if(rtl865x_getIpIdxByExtIp(select_t->extIp, &ipIdx)==SUCCESS)
					{
						asic_mcast.extIdx=(uint16)ipIdx;
					}
#else
					asic_mcast.extIdx=0;
#endif
				}

				retval = rtl8651_setAsicIpMulticastTable(&asic_mcast);
#ifdef CONFIG_PROC_FS
				mcastAddOpCnt++;
#endif
				assert(retval == SUCCESS);
				if(retval==SUCCESS)
				{
					select_t->inAsic = TRUE;
				}
				else
				{
					select_t->inAsic = FALSE;
					rtl8651_delAsicIpMulticastTable(entryIndex);
#ifdef CONFIG_PROC_FS
					mcastDelOpCnt++;
#endif
				}
				
				TAILQ_REMOVE(&mCastTbl.inuseList.mCastTbl[entryIndex], select_t, nextMCast);
				TAILQ_INSERT_HEAD(&mCastTbl.inuseList.mCastTbl[entryIndex], select_t, nextMCast);

			}
			else
			{			
				if(swapOutEntry->inAsic == FALSE)
				{
					/*maybe something is wrong, we remove the asic entry*/
					rtl8651_delAsicIpMulticastTable(entryIndex);
#ifdef CONFIG_PROC_FS
					mcastDelOpCnt++;
#endif
				}
				
			}		
			
		}
		
	}
	else 	
	{
		if(swapOutEntry!=NULL)
		{
			swapOutEntry->inAsic=FALSE;
		}
		rtl8651_delAsicIpMulticastTable(entryIndex);
#ifdef CONFIG_PROC_FS
		mcastDelOpCnt++;
#endif
	}
}


static void _rtl865x_mCastEntryReclaim(void)
{
	uint32 index;
	uint32 freeCnt=0;
	uint32 asicFwdPortMask=0;
	uint32 needReArrange=FALSE;
	rtl865x_tblDrv_mCast_t *curMCastEntry, *nextMCastEntry;

	/*free unused software forward entry*/
	for(index=0; index<RTL8651_MULTICASTTBL_SIZE; index++) 
	{
		curMCastEntry = TAILQ_FIRST(&mCastTbl.inuseList.mCastTbl[index]);
		while (curMCastEntry)
		{
			nextMCastEntry = TAILQ_NEXT(curMCastEntry, nextMCast);
			if((curMCastEntry->inAsic==FALSE)  && (curMCastEntry->count==0))
			{
				_rtl865x_freeMCastEntry(curMCastEntry, index);
				freeCnt++;
			}
			curMCastEntry = nextMCastEntry;
		}
		
	}

	if(freeCnt>0)
	{
		return;
	}
	
	for(index=0; index<RTL8651_MULTICASTTBL_SIZE; index++) 
	{
		curMCastEntry = TAILQ_FIRST(&mCastTbl.inuseList.mCastTbl[index]);
		needReArrange=FALSE;
		while (curMCastEntry)
		{
			nextMCastEntry = TAILQ_NEXT(curMCastEntry, nextMCast);
			if(curMCastEntry->inAsic)
			{
				asicFwdPortMask=rtl865x_genMCastEntryAsicFwdMask(curMCastEntry);
				if(asicFwdPortMask==0) 
				{
					_rtl865x_freeMCastEntry(curMCastEntry, index);
					needReArrange=TRUE;
				}
			}
			curMCastEntry = nextMCastEntry;
		}
		
		if(needReArrange==TRUE)
		{
			_rtl865x_arrangeMulticast(index);
		}
	}

	return;
}
/*
@func rtl865x_tblDrv_mCast_t *	| rtl865x_findMCastEntry	|  API to find a hardware multicast entry.
@parm  ipaddr_t 	| mAddr	| Multicast stream destination group address. 
@parm  ipaddr_t	|  sip	| Multicast stream source ip address.
@parm  uint16		| svid	| Multicast stream input vlan index.
@parm  uint16 	| sport	| Multicast stream input port number.
*/
rtl865x_tblDrv_mCast_t *rtl865x_findMCastEntry(ipaddr_t mAddr, ipaddr_t sip, uint16 svid, uint16 sport)
{
	rtl865x_tblDrv_mCast_t *mCast_t;
	uint32 entry = rtl8651_ipMulticastTableIndex(sip, mAddr);
	TAILQ_FOREACH(mCast_t, &mCastTbl.inuseList.mCastTbl[entry], nextMCast) {
		if (mCast_t->dip==mAddr && mCast_t->sip==sip && mCast_t->svid==svid && mCast_t->port==sport)
		{
			if (mCast_t->inAsic == FALSE) 
			{
				mCast_t->age = RTL865X_MULTICAST_TABLE_AGE;
				mCast_t->count ++;
			}

			return mCast_t;
		}
		
		mCast_t->count ++;
		if(mCast_t->maxPPS<mCast_t->count)
		{
			/*update maxPPS*/
			mCast_t->maxPPS=mCast_t->count ;
		}
			
	}
	return (rtl865x_tblDrv_mCast_t *)NULL;	
}

/*
@func int32	| rtl865x_addMulticastEntry	|  API to add a hardwawre multicast forwarding entry.
@parm  ipaddr_t 	| mAddr	| Multicast flow Destination group address. 
@parm  ipaddr_t 	| sip	| Multicast flow source ip address. 
@parm  uint16 	| svid	| Multicast flow input vlan index. 
@parm  uint16		| sport	| Multicast flow input port number. 
@parm  rtl865x_mcast_fwd_descriptor_t *	| newFwdDescChain	| Multicast flow forwarding descriptor chain to be added. 
@parm  int32 	| flushOldChain	| Flag to indicate to flush old mulicast forwarding descriptor chain or not. 1 is to flush old chain, and 0 is not to. 
@parm  ipaddr_t 	| extIp	| External source ip address used when forward multicast data from lan to wan. 
@parm  int8	| toCpu	| Cpu forwarding flag, 1 is to forward multicast data by cpu,and  0 is not.
@parm  int8	| flag	| For future usage, set to 0 at present.
@rvalue SUCCESS	|Add hardware multicast forwarding entry successfully. 
@rvalue FAILED	|Add hardware multicast forwarding entry failed.
*/
int32 rtl865x_addMulticastEntry(ipaddr_t mAddr, ipaddr_t sip, uint16 svid, uint16 sport, 
									rtl865x_mcast_fwd_descriptor_t * newFwdDescChain, 
									int32 flushOldChain, ipaddr_t extIp, char cpuHold, uint8 flag)
{

	rtl865x_tblDrv_mCast_t *mCast_t;
	uint32 hashIndex = rtl8651_ipMulticastTableIndex(sip, mAddr);
	#if defined (CONFIG_RTL_IGMP_SNOOPING)
	struct rtl_groupInfo groupInfo;
	#endif
	/*windows xp upnp:239.255.255.0*/
	if(mAddr==0xEFFFFFFA)
	{
		return FAILED;
	}
#if 0
	/*reserved multicast address 224.0.0.x*/
	if((mAddr & 0xFFFFFF00) == 0xE0000000)
	{
		return FAILED;
	}
#endif	
	/*try to match hash line*/
	TAILQ_FOREACH(mCast_t, &mCastTbl.inuseList.mCastTbl[hashIndex], nextMCast) 
	{
		if (mCast_t->sip==sip && mCast_t->dip==mAddr && mCast_t->svid==svid && mCast_t->port==sport)
			break;
	}
	
	if (mCast_t == NULL) 
	{
		mCast_t=_rtl865x_allocMCastEntry(hashIndex);
		if (mCast_t == NULL)
		{
			_rtl865x_mCastEntryReclaim();
			mCast_t=_rtl865x_allocMCastEntry(hashIndex);
			if(mCast_t == NULL)
			{
				return FAILED;
			}
		}
		mCast_t->sip			= sip;
		mCast_t->dip			= mAddr;
		mCast_t->svid		= svid;
		mCast_t->port		= sport;
		mCast_t->mbr		= 0;
		mCast_t->count		= 0;
		mCast_t->maxPPS		= 0;
		mCast_t->inAsic		= FALSE;
	}
	
	if(flushOldChain)
	{
		_rtl865x_flushMCastFwdDescChain(&mCast_t->fwdDescChain);
		
	}
	
	_rtl865x_mergeMCastFwdDescChain(&mCast_t->fwdDescChain,newFwdDescChain);
	_rtl865x_doMCastEntrySrcVlanPortFilter(mCast_t);
	
	mCast_t->mbr			= rtl865x_genMCastEntryAsicFwdMask(mCast_t);
	mCast_t->extIp			= extIp;

	mCast_t->age			= RTL865X_MULTICAST_TABLE_AGE;
#if 0
	mCast_t->cpu			= (toCpu==TRUE? 1: 0);
#else
	mCast_t->cpuHold			= cpuHold;
	mCast_t->cpu 			= rtl865x_genMCastEntryCpuFlag(mCast_t);
#endif	
	mCast_t->flag			= flag;
	
	if (extIp)
		mCast_t->flag |= RTL865X_MULTICAST_EXTIP_SET;
	else
		mCast_t->flag &= ~RTL865X_MULTICAST_EXTIP_SET;
	#if defined (CONFIG_RTL_IGMP_SNOOPING)
	rtl_getGroupInfo(mAddr, &groupInfo);
	if(groupInfo.ownerMask==0)
	{
		mCast_t->unKnownMCast=TRUE;
	}
	else
	{
		mCast_t->unKnownMCast=FALSE;
	}
	#endif
	_rtl865x_patchPppoeWeak(mCast_t);
	_rtl865x_arrangeMulticast(hashIndex);
	return SUCCESS;	
}


/*
@func int32	| rtl865x_delMulticastEntry	|  API to delete multicast forwarding entry related with a certain group address.
@parm  ipaddr_t 	| mcast_addr	| Group address to be mached in deleting hardware multicast forwarding entry. 
@rvalue SUCCESS	|Delete hardware multicast forwarding entry successfully. 
@rvalue FAILED	|Delete hardware multicast forwarding entry failed.
*/
int32 rtl865x_delMulticastEntry(ipaddr_t mcast_addr)
{

	rtl865x_tblDrv_mCast_t *mCastEntry, *nextMCastEntry;
	uint32 entry;
	uint32 deleteFlag=FALSE;

	for(entry=0; entry<RTL8651_MULTICASTTBL_SIZE; entry++) 
	{
		deleteFlag=FALSE;
		mCastEntry = TAILQ_FIRST(&mCastTbl.inuseList.mCastTbl[entry]);
		while (mCastEntry)
		{
			nextMCastEntry = TAILQ_NEXT(mCastEntry, nextMCast);
			if (!mcast_addr || mCastEntry->dip == mcast_addr) 
			{
				deleteFlag=TRUE;
				_rtl865x_freeMCastEntry(mCastEntry, entry);
			}
			
			mCastEntry = nextMCastEntry;
		}
		
		if(deleteFlag==TRUE)
		{
			_rtl865x_arrangeMulticast(entry);
		}
	}

	return SUCCESS;
}

#if 0
/*the following function maybe used in future*/

int32 rtl865x_addMulticastFwdDesc(ipaddr_t mcast_addr, rtl865x_mcast_fwd_descriptor_t * newFwdDesc)
{

	rtl865x_tblDrv_mCast_t *mCast_t;
	uint32 entry, matchedIdx = 0;
	uint32 oldFwdPortMask,newFwdPortMask;
	if(newFwdDesc==NULL)
	{
		return SUCCESS;
	}

	for (entry=0; entry< RTL8651_MULTICASTTBL_SIZE; entry++)
	{
		TAILQ_FOREACH(mCast_t, &mCastTbl.inuseList.mCastTbl[entry], nextMCast)
		{
			if (mCast_t->dip != mcast_addr)
				continue;

			oldFwdPortMask=mCast_t->mbr;

			_rtl865x_mergeMCastFwdDescChain(&mCast_t->fwdDescChain,newFwdDesc);
			_rtl865x_doMCastEntrySrcVlanPortFilter(mCast_t);
			
			mCast_t->mbr 		= rtl865x_genMCastEntryFwdMask(mCast_t);
			newFwdPortMask		= mCast_t->mbr ;
#ifndef RTL8651_MCAST_ALWAYS2UPSTREAM
			if (mCast_t->flag & RTL865X_MULTICAST_UPLOADONLY)
			{	/* remove upload term*/
				if(oldFwdPortMask!=newFwdPortMask)
				{
					mCast_t->flag &= ~RTL865X_MULTICAST_UPLOADONLY;
					/* we assume multicast member will NEVER in External interface, so we remove
					     external ip now */
					mCast_t->flag &= ~RTL865X_MULTICAST_EXTIP_SET;
					mCast_t->extIp= 0;
				}
			}
#endif /* RTL8651_MCAST_ALWAYS2UPSTREAM */

			_rtl865x_patchPppoeWeak(mCast_t);
			_rtl865x_arrangeMulticast(entry);
			matchedIdx = entry;
		}
	}

	if (matchedIdx) 
	{
		return SUCCESS;
	}
	return FAILED;
}

int32 rtl865x_delMulticastFwdDesc(ipaddr_t mcast_addr,  rtl865x_mcast_fwd_descriptor_t * deadFwdDesc)
{

	uint32 index;
	rtl865x_tblDrv_mCast_t  *mCastEntry, *nextMCastEntry;
	uint32 oldFwdPortMask,newFwdPortMask;
	
	for(index=0; index<RTL8651_MULTICASTTBL_SIZE; index++) 
	{

		for (mCastEntry = TAILQ_FIRST(&mCastTbl.inuseList.mCastTbl[index]); mCastEntry; mCastEntry = nextMCastEntry)
		{
			nextMCastEntry=TAILQ_NEXT(mCastEntry, nextMCast);
			
			if ((mcast_addr) && (mCastEntry->dip != mcast_addr))
			{
				continue;
			}
			
			oldFwdPortMask=mCastEntry->mbr;
		
			_rtl865x_subMCastFwdDescChain(&mCastEntry->fwdDescChain, deadFwdDesc);
			
			mCastEntry->mbr=rtl865x_genMCastEntryFwdMask(mCastEntry);
			newFwdPortMask=mCastEntry->mbr; 	
			if (mCastEntry->mbr == 0)
			{
				/*to-do:unknown multicast hardware blocking*/
				_rtl865x_freeMCastEntry(mCastEntry, index);
				mCastEntry=NULL;
				_rtl865x_arrangeMulticast(index);
			}
			else
			{
			
				_rtl865x_patchPppoeWeak(mCastEntry);
			}
			
		}
			
		_rtl865x_arrangeMulticast(index);
	}

	return SUCCESS;
}

int32 rtl865x_delMulticastUpStream(ipaddr_t mcast_addr, ipaddr_t sip, uint16 svid, uint16 sport)
{
	uint32 index;
	rtl865x_tblDrv_mCast_t *mCast_t;
	
	for(index=0; index<RTL8651_MULTICASTTBL_SIZE; index++) 
	{
		TAILQ_FOREACH(mCast_t, &mCastTbl.inuseList.mCastTbl[index], nextMCast) 
		{
			if ((!mcast_addr || mCast_t->dip == mcast_addr) && 
				(!sip || mCast_t->sip==sip) && 
				(!svid || mCast_t->svid==svid) && 
				mCast_t->port==sport)
			{
				_rtl865x_freeMCastEntry(mCast_t, index);
				_rtl865x_arrangeMulticast(index);
				return SUCCESS;
			}
		}
	}
	return FAILED;
}

int32 rtl865x_delMulticastByVid(uint32 vid)
{
	uint16 sport;
	uint32 sportMask;
	rtl865x_mcast_fwd_descriptor_t vlanFwdDesc;
	memset(&vlanFwdDesc,0,sizeof(rtl865x_mcast_fwd_descriptor_t));
	
	/* delete all upstream related to vid */
	sport = 0;
	sportMask=rtl865x_getVlanPortMask(vid);
	while (sportMask) 
	{
		if (sportMask & 1)
		{
			rtl865x_delMulticastUpStream(0, 0, vid, sport);
		}
		
		sportMask = sportMask >> 1;
		sport ++;
	}
	
	/* delete all downstream related to vid*/
	vlanFwdDesc.vid=vid;
	vlanFwdDesc.fwdPortMask=rtl865x_getVlanPortMask(vid);
	rtl865x_delMulticastFwdDesc(0, &vlanFwdDesc);

	return FAILED;
}

int32 rtl865x_delMulticastByPort(uint32 port)
{

	rtl865x_mcast_fwd_descriptor_t portFwdDesc;
	memset(&portFwdDesc,0,sizeof(rtl865x_mcast_fwd_descriptor_t));
	
	/* delete all upstream related to this port */
	rtl865x_delMulticastUpStream(0, 0, 0, port);

	/* delete all downstream related to this port*/
	portFwdDesc.vid=0;
	portFwdDesc.fwdPortMask=1<<port;
	rtl865x_delMulticastFwdDesc(0, &portFwdDesc);

	return SUCCESS;
}

int32 rtl865x_setMGroupAttribute(ipaddr_t groupIp, int8 toCpu)
{
	uint32 index;
	rtl865x_tblDrv_mCast_t *mCast_t;

	for(index=0; index<RTL8651_MULTICASTTBL_SIZE; index++) 
	{
		TAILQ_FOREACH(mCast_t, &mCastTbl.inuseList.mCastTbl[index], nextMCast) 
		{
			if (mCast_t->dip == groupIp)
			{
				mCast_t->cpu = (toCpu==TRUE? 1: 0);
			}
		}
		_rtl865x_arrangeMulticast(index);
	}
	return SUCCESS;
}


static int32 _rtl865x_subMCastFwdDescChain(mcast_fwd_descriptor_head_t * targetChainHead,rtl865x_mcast_fwd_descriptor_t *srcChain)
{
	rtl865x_mcast_fwd_descriptor_t *curDesc;
	if(targetChainHead==NULL)
	{
		return FAILED;
	}
	
	for(curDesc=srcChain; curDesc!=NULL; curDesc=MC_LIST_NEXT(curDesc,next))
	{
		_rtl865x_mCastFwdDescDequeue(targetChainHead, curDesc);
	}

	return SUCCESS;
}

static int32 _rtl865x_mCastFwdDescDequeue(mcast_fwd_descriptor_head_t * queueHead,rtl865x_mcast_fwd_descriptor_t * dequeueDesc)
{
	rtl865x_mcast_fwd_descriptor_t *curDesc,*nextDesc;
	
	if(queueHead==NULL)
	{
		return FAILED;
	}
	
	if(dequeueDesc==NULL)
	{
		return FAILED;
	}

	for(curDesc=MC_LIST_FIRST(queueHead);curDesc!=NULL;curDesc=nextDesc)
	{
		nextDesc=MC_LIST_NEXT(curDesc, next );
		if((strcmp(curDesc->netifName,dequeueDesc->netifName)==0) ||
			((dequeueDesc->vid==0 ) ||(curDesc->vid==dequeueDesc->vid)))
		{
			curDesc->fwdPortMask &= (~dequeueDesc->fwdPortMask);
			if(curDesc->fwdPortMask==0)
			{
				/*remove from the old descriptor chain*/
				MC_LIST_REMOVE(curDesc, next);
				/*return to the free descriptor chain*/
				_rtl865x_freeMCastFwdDesc(curDesc);

			}

			return SUCCESS;
		}
	}

	/*never reach here*/
	return SUCCESS;
}

#endif

static int32 rtl865x_multicastCallbackFn(void *param)
{
	#if defined (CONFIG_RTL_IGMP_SNOOPING)
	uint32 index;
	uint32 oldDescPortMask,newDescPortMask;/*for device decriptor forwarding usage*/
	
	uint32 oldAsicFwdPortMask,newAsicFwdPortMask;/*for physical port forwarding usage*/
	uint32 oldCpuFlag,newCpuFlag;
	
	rtl_multicastEventContext_t mcastEventContext;

	rtl865x_mcast_fwd_descriptor_t newFwdDesc;
	struct rtl_multicastDataInfo multicastDataInfo;
	struct rtl_multicastFwdInfo multicastFwdInfo;
	rtl865x_tblDrv_mCast_t  *mCastEntry,*nextMCastEntry;
	struct rtl_multicastDeviceInfo_s bridgeMCastDev;

	struct rtl_groupInfo groupInfo;
	int32 retVal=FAILED;

	if(param==NULL)
	{
		return EVENT_CONTINUE_EXECUTE;
	}
	memcpy(&mcastEventContext,param,sizeof(rtl_multicastEventContext_t));
	/*check device name's validity*/
	if(strlen(mcastEventContext.devName)==0)
	{
		return EVENT_CONTINUE_EXECUTE;
	}
	#ifdef CONFIG_RTL865X_MUTLICAST_DEBUG
	printk("%s:%d,mcastEventContext.devName is %s, mcastEventContext.groupAddr is 0x%x,mcastEventContext.sourceAdd is 0x%x,mcastEventContext.portMask is 0x%x\n",__FUNCTION__,__LINE__,mcastEventContext.devName, mcastEventContext.groupAddr[0], mcastEventContext.sourceAddr[0], mcastEventContext.portMask);
	#endif
	/*case 1:this is multicast event from bridge(br0) */
	/*sync wlan and ethernet*/
	//hyking:[Fix me] the RTL_BR_NAME...
#ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
    if(memcmp(mcastEventContext.devName,RTL_BR_NAME,3)==0 || memcmp(mcastEventContext.devName,RTL_BR1_NAME,3)==0)
#else
	if(memcmp(mcastEventContext.devName,RTL_BR_NAME,3)==0)
#endif
	{

		for (index=0; index< RTL8651_MULTICASTTBL_SIZE; index++)
		{
			for (mCastEntry = TAILQ_FIRST(&mCastTbl.inuseList.mCastTbl[index]); mCastEntry; mCastEntry = nextMCastEntry)
			{
				nextMCastEntry=TAILQ_NEXT(mCastEntry, nextMCast);
				
				if ((mcastEventContext.groupAddr!=0) && (mCastEntry->dip != mcastEventContext.groupAddr[0]))
				{
					continue;
				}

				rtl_getGroupInfo(mCastEntry->dip, &groupInfo);
				if(groupInfo.ownerMask==0)
				{
					mCastEntry->unKnownMCast=TRUE;
				}
				else
				{
					mCastEntry->unKnownMCast=FALSE;
				}

				oldDescPortMask=rtl865x_getMCastEntryDescPortMask( mCastEntry);			
				
				/*sync with control plane*/
				memset(&newFwdDesc, 0 ,sizeof(rtl865x_mcast_fwd_descriptor_t));
				strcpy(newFwdDesc.netifName,mcastEventContext.devName);
				multicastDataInfo.ipVersion=4;
				multicastDataInfo.sourceIp[0]=  mCastEntry->sip;
				multicastDataInfo.groupAddr[0]= mCastEntry->dip;
				retVal= rtl_getMulticastDataFwdInfo(mcastEventContext.moduleIndex, &multicastDataInfo, &multicastFwdInfo);
				

				if(retVal!=SUCCESS)
				{
					continue;
				}
				
				retVal= rtl_getIgmpSnoopingModuleDevInfo(mcastEventContext.moduleIndex, &bridgeMCastDev);
				if(retVal!=SUCCESS)
				{
					continue;
				}
				newDescPortMask=multicastFwdInfo.fwdPortMask;
				if(	(oldDescPortMask != newDescPortMask) &&
					(	((newDescPortMask & bridgeMCastDev.swPortMask)!=0) ||
						(((oldDescPortMask & bridgeMCastDev.swPortMask) !=0) && ((newDescPortMask & bridgeMCastDev.swPortMask)==0)))	)
				{
					/*this multicast entry should be re-generated at linux protocol stack bridge level*/
					_rtl865x_freeMCastEntry(mCastEntry, index);
					_rtl865x_arrangeMulticast(index);
				}
				
			}
		}
		
		return EVENT_CONTINUE_EXECUTE;
	}
			
	/*case 2:this is multicast event from ethernet (eth0)*/
	/*update ethernet forwarding port mask*/
#ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
       if(memcmp(mcastEventContext.devName,"eth*",4)==0 || memcmp(mcastEventContext.devName,RTL_PS_ETH_NAME_ETH2,4)==0)
#else
	if(memcmp(mcastEventContext.devName,"eth*",4)==0)
#endif
	{
		#ifdef CONFIG_RTL865X_MUTLICAST_DEBUG
		printk("%s:%d,multicast event from ethernet (%s),mcastEventContext.groupAddr[0] is 0x%x\n",__FUNCTION__,__LINE__,mcastEventContext.devName,mcastEventContext.groupAddr[0]);
		#endif
		
		for (index=0; index< RTL8651_MULTICASTTBL_SIZE; index++)
		{
			for (mCastEntry = TAILQ_FIRST(&mCastTbl.inuseList.mCastTbl[index]); mCastEntry; mCastEntry = nextMCastEntry)
			{
				nextMCastEntry=TAILQ_NEXT(mCastEntry, nextMCast);
				
				if ((mcastEventContext.groupAddr!=0) && (mCastEntry->dip != mcastEventContext.groupAddr[0]))
				{
					continue;
				}
				
				memset(&newFwdDesc, 0 ,sizeof(rtl865x_mcast_fwd_descriptor_t));
				strcpy(newFwdDesc.netifName,mcastEventContext.devName);

				/*save old multicast entry forward port mask*/
				oldAsicFwdPortMask=mCastEntry->mbr;
				oldCpuFlag=mCastEntry->cpu;

				/*sync with control plane*/
				multicastDataInfo.ipVersion=4;
				multicastDataInfo.sourceIp[0]=  mCastEntry->sip;
				multicastDataInfo.groupAddr[0]= mCastEntry->dip;
				retVal= rtl_getMulticastDataFwdInfo(mcastEventContext.moduleIndex, &multicastDataInfo, &multicastFwdInfo);

				newFwdDesc.fwdPortMask=multicastFwdInfo.fwdPortMask & (~(1<<mCastEntry->port));
				newFwdDesc.toCpu=multicastFwdInfo.cpuFlag;
			
				/*update/replace old forward descriptor*/
				
				_rtl865x_mergeMCastFwdDescChain(&mCastEntry->fwdDescChain,&newFwdDesc);
				mCastEntry->mbr 		= rtl865x_genMCastEntryAsicFwdMask(mCastEntry);
				mCastEntry->cpu		= rtl865x_genMCastEntryCpuFlag(mCastEntry);
				
				newAsicFwdPortMask	= mCastEntry->mbr ;
				newCpuFlag			=mCastEntry->cpu;
				
				#ifdef CONFIG_RTL865X_MUTLICAST_DEBUG
				printk("%s:%d,oldAsicFwdPortMask is %d,newAsicFwdPortMask is %d\n",__FUNCTION__,__LINE__,oldAsicFwdPortMask,newAsicFwdPortMask);
				#endif
				
#ifndef RTL8651_MCAST_ALWAYS2UPSTREAM
				if (mCastEntry->flag & RTL865X_MULTICAST_UPLOADONLY)
				{	/* remove upload term*/
					if((newAsicFwdPortMask!=0) && (newAsicFwdPortMask!=oldAsicFwdPortMask))
					{
						mCastEntry->flag &= ~RTL865X_MULTICAST_UPLOADONLY;
						/* we assume multicast member will NEVER in External interface, so we remove
						     external ip now */
						mCastEntry->flag &= ~RTL865X_MULTICAST_EXTIP_SET;
						mCastEntry->extIp= 0;
					}
				}
#endif /* RTL8651_MCAST_ALWAYS2UPSTREAM */

				rtl_getGroupInfo(mCastEntry->dip, &groupInfo);
				if(groupInfo.ownerMask==0)
				{
					mCastEntry->unKnownMCast=TRUE;
				}
				else
				{
					mCastEntry->unKnownMCast=FALSE;
				}
	
				
				if((oldCpuFlag != newCpuFlag)||(newAsicFwdPortMask!=oldAsicFwdPortMask)) 
				{
					_rtl865x_patchPppoeWeak(mCastEntry);
					
					/*reset inAsic flag to re-select or re-write this hardware asic entry*/
					if(newAsicFwdPortMask==0)
					{
						_rtl865x_freeMCastEntry(mCastEntry, index);
					}

					_rtl865x_arrangeMulticast(index);
				}
			}

				
			
		}
	}
	#endif
	return EVENT_CONTINUE_EXECUTE;
}

static int32 _rtl865x_multicastUnRegisterEvent(void)
{
	rtl865x_event_Param_t eventParam;

	eventParam.eventLayerId=DEFAULT_LAYER3_EVENT_LIST_ID;
	eventParam.eventId=EVENT_UPDATE_MCAST;
	eventParam.eventPriority=0;
	eventParam.event_action_fn=rtl865x_multicastCallbackFn;
	rtl865x_unRegisterEvent(&eventParam);

	return SUCCESS;
}

static int32 _rtl865x_multicastRegisterEvent(void)
{
	rtl865x_event_Param_t eventParam;

	eventParam.eventLayerId=DEFAULT_LAYER3_EVENT_LIST_ID;
	eventParam.eventId=EVENT_UPDATE_MCAST;
	eventParam.eventPriority=0;
	eventParam.event_action_fn=rtl865x_multicastCallbackFn;
	rtl865x_registerEvent(&eventParam);

	return SUCCESS;
}


void _rtl865x_timeUpdateMulticast(uint32 secPassed)
{

	rtl865x_tblDrv_mCast_t *mCast_t, *nextMCast_t;
	uint32 entry;
	uint32 needReArrange=FALSE;
	/* check to Aging and HW swapping */
	for (entry=0; entry< RTL8651_MULTICASTTBL_SIZE; entry++) {
		needReArrange=FALSE;
		mCast_t = TAILQ_FIRST(&mCastTbl.inuseList.mCastTbl[entry]);
		while (mCast_t) {
			/*save the next entry first*/
			nextMCast_t=TAILQ_NEXT(mCast_t, nextMCast);
			
			if (mCast_t->inAsic == TRUE)
			{
				/* Entry is in the ASIC */
				if (mCast_t->age <= secPassed) 
				{
					if(mCast_t->mbr==0)
					{
						_rtl865x_freeMCastEntry(mCast_t, entry);
						needReArrange=TRUE;
					}
					else
					{
						mCast_t->age = RTL865X_MULTICAST_TABLE_AGE;
					}
				}
				else
				{
					mCast_t->age -= secPassed;
				}
			}
			else 
			{
				/* Entry is not in the ASIC */
				if(mCast_t->maxPPS<mCast_t->count)
				{
					mCast_t->maxPPS=mCast_t->count ;
				}
				
				if(mCast_t->maxPPS>0)
				{
					mCast_t->maxPPS--;
				}
				
				if (mCast_t->age <= secPassed)
				{ /* aging out */
					_rtl865x_freeMCastEntry(mCast_t, entry);
				}
				else 
				{
					mCast_t->age -= secPassed;
				}
			}
			
			mCast_t->count = 0;
			mCast_t = nextMCast_t;
		}

		if(needReArrange==TRUE)
		{
			_rtl865x_arrangeMulticast(entry);
		}
	
	}
}

#if defined(__linux__) && defined(__KERNEL__)
static void _rtl865x_mCastSysTimerExpired(uint32 expireDada)
{

	_rtl865x_timeUpdateMulticast(1);
	mod_timer(&rtl865x_mCastSysTimer, jiffies+HZ);
	
}

static void _rtl865x_initMCastSysTimer(void)
{

	init_timer(&rtl865x_mCastSysTimer);
	rtl865x_mCastSysTimer.data=rtl865x_mCastSysTimer.expires;
	rtl865x_mCastSysTimer.expires=jiffies+HZ;
	rtl865x_mCastSysTimer.function=(void*)_rtl865x_mCastSysTimerExpired;
	add_timer(&rtl865x_mCastSysTimer);
}

static void _rtl865x_destroyMCastSysTimer(void)
{
	del_timer(&rtl865x_mCastSysTimer);
}

#endif

/*
@func int32	| rtl865x_initMulticast	|  Init hardware ip multicast module.
@parm  rtl865x_mCastConfig_t *	| mCastConfigPtr	| Pointer of hardware multicast configuration. 
@rvalue SUCCESS	|Initialize successfully.
@rvalue FAILED	|Initialize failed.
*/
int32 rtl865x_initMulticast(rtl865x_mCastConfig_t * mCastConfigPtr)
{
	_rtl865x_multicastUnRegisterEvent();
	_rtl865x_initMCastEntryPool();
	_rtl865x_initMCastFwdDescPool();
	rtl865x_setMulticastExternalPortMask(0);
	if(mCastConfigPtr!=NULL)
	{
		rtl865x_setMulticastExternalPortMask(mCastConfigPtr->externalPortMask);
	}
	#if defined(__linux__) && defined(__KERNEL__)
	_rtl865x_initMCastSysTimer();
	#endif
	rtl8651_setAsicMulticastMTU(1522); 
	rtl8651_setAsicMulticastEnable(TRUE);
	rtl865x_setAsicMulticastAging(TRUE);
	_rtl865x_multicastRegisterEvent();
	return SUCCESS;
}

/*
@func int32	| rtl865x_reinitMulticast	|  Re-init hardware ip multicast module.
@rvalue SUCCESS	|Re-initialize successfully.
@rvalue FAILED	|Re-initialize failed.
*/
int32 rtl865x_reinitMulticast(void)
{
	_rtl865x_multicastUnRegisterEvent();
	/*delete all multicast entry*/
	rtl8651_setAsicMulticastEnable(FALSE);
	rtl865x_delMulticastEntry(0);
	
	#if defined(__linux__) && defined(__KERNEL__)
	_rtl865x_destroyMCastSysTimer();
	_rtl865x_initMCastSysTimer();
	#endif
	
	/*regfster twice won't cause any side-effect, 
	because event management module will handle duplicate event issue*/
	rtl8651_setAsicMulticastMTU(1522); 
	rtl8651_setAsicMulticastEnable(TRUE);
	rtl865x_setAsicMulticastAging(TRUE);
	_rtl865x_multicastRegisterEvent();
	return SUCCESS;
}	



#ifdef CONFIG_PROC_FS
extern int32 rtl8651_getAsicMulticastSpanningTreePortState(uint32 port, uint32 *portState);
int32 rtl_dumpSwMulticastInfo(void)
{
	uint32 mCastMtu=0;
	uint32 mCastEnable=FALSE;
	uint32 index;
	int8 isInternal;
	uint32 portStatus;
	uint32 internalPortMask=0;
	uint32 externalPortMask=0;
	int32 ret=FAILED;
	
	rtl865x_tblDrv_mCast_t *mCast_t, *nextMCast_t;
	rtl865x_mcast_fwd_descriptor_t *curDesc,*nextDesc;
	uint32 entry;
	uint32 cnt;
	printk("----------------------------------------------------\n");
	printk("Asic Operation Layer :%d\n", rtl8651_getAsicOperationLayer());
	
	ret=rtl8651_getAsicMulticastEnable(&mCastEnable);
	if(ret==SUCCESS)
	{
		printk("Asic Multicast Table:%s\n", (mCastEnable==TRUE)?"Enable":"Disable");
	}
	else
	{
		printk("Read Asic Multicast Table Enable Bit Error\n");
	}
	
	ret=rtl8651_getAsicMulticastMTU(&mCastMtu); 
	if(ret==SUCCESS)
	{
		printk("Asic Multicast MTU:%d\n", mCastMtu);
	}
	else
	{
		printk("Read Asic Multicast MTU Error\n");
	}
	
	for (index=0; index<RTL8651_PORT_NUMBER+rtl8651_totalExtPortNum; index++)
	{
		ret=rtl8651_getAsicMulticastPortInternal(index, &isInternal);
		if(ret==SUCCESS)
		{
			if(isInternal==TRUE)
			{
				internalPortMask |= 1<<index;
			}
			else
			{
				externalPortMask |= 1<<index;
			}
		}
	
	}

	printk("Internal Port Mask:0x%x\nExternal Port Mask:0x%x\n", internalPortMask,externalPortMask);
	printk("----------------------------------------------------\n");
	printk("Multicast STP State:\n");
	for (index=0; index<RTL8651_PORT_NUMBER+rtl8651_totalExtPortNum; index++)
	{
		ret= rtl8651_getAsicMulticastSpanningTreePortState(index, &portStatus);
		if(ret==SUCCESS)
		{
			printk("port[%d]:",index);
			if(portStatus==RTL8651_PORTSTA_DISABLED)
			{
				printk("disabled\n");
			}
			else if(portStatus==RTL8651_PORTSTA_BLOCKING)
			{
				printk("blocking\n");
			}
			else if(portStatus==RTL8651_PORTSTA_LEARNING)
			{
				printk("learning\n");
			}
			else if(portStatus==RTL8651_PORTSTA_FORWARDING)
			{
				printk("forwarding\n");
			}
		}
		
	}
	printk("----------------------------------------------------\n");
	printk("Software Multicast Table:\n");
	/* check to Aging and HW swapping */
	for (entry=0; entry< RTL8651_MULTICASTTBL_SIZE; entry++) {
		mCast_t = TAILQ_FIRST(&mCastTbl.inuseList.mCastTbl[entry]);
		while (mCast_t) {
			/*save the next entry first*/
			nextMCast_t=TAILQ_NEXT(mCast_t, nextMCast);
			printk("\t[%2d]  dip:%d.%d.%d.%d, sip:%d.%d.%d.%d, mbr:0x%x, svid:%d, spa:%d, \n", entry,
				mCast_t->dip>>24, (mCast_t->dip&0x00ff0000)>>16, (mCast_t->dip&0x0000ff00)>>8, (mCast_t->dip&0xff), 
				mCast_t->sip>>24, (mCast_t->sip&0x00ff0000)>>16, (mCast_t->sip&0x0000ff00)>>8, (mCast_t->sip&0xff),
				mCast_t->mbr,mCast_t->svid, mCast_t->port);
			printk("\t      extIP:0x%x,age:%d, cpu:%d, maxPPS:%d, inAsic:%d, (%s)\n", 
				mCast_t->extIp,mCast_t->age, mCast_t->cpu,mCast_t->maxPPS,mCast_t->inAsic,mCast_t->unKnownMCast?"UnknownMCast":"KnownMCast");
			
			cnt=0;
			curDesc=MC_LIST_FIRST(&mCast_t->fwdDescChain);
			while(curDesc)
			{
				nextDesc=MC_LIST_NEXT(curDesc, next );
				printk("\t      netif(%s),descPortMask(0x%x),toCpu(%d),fwdPortMask(0x%x)\n",curDesc->netifName,curDesc->descPortMask,curDesc->toCpu,curDesc->fwdPortMask);
				curDesc = nextDesc;
			}
			
			printk("\n");
			mCast_t = nextMCast_t;
		}
		
	}

	return SUCCESS;
}
#endif

int rtl865x_genVirtualMCastFwdDescriptor(unsigned int forceToCpu, uint32 fwdPortMask, rtl865x_mcast_fwd_descriptor_t *fwdDescriptor)
{
	
	if(fwdDescriptor==NULL)
	{
		return FAILED;
	}
	
	memset(fwdDescriptor, 0, sizeof(rtl865x_mcast_fwd_descriptor_t ));
	fwdDescriptor->toCpu=forceToCpu;
	fwdDescriptor->fwdPortMask=fwdPortMask;
	return SUCCESS;

}

int rtl865x_blockMulticastFlow(unsigned int srcVlanId, unsigned int srcPort,unsigned int srcIpAddr, unsigned int destIpAddr)
{
	rtl865x_mcast_fwd_descriptor_t fwdDescriptor;
	rtl865x_tblDrv_mCast_t * existMCastEntry=NULL;
	existMCastEntry=rtl865x_findMCastEntry(destIpAddr, srcIpAddr, (uint16)srcVlanId, (uint16)srcPort);
	if(existMCastEntry!=NULL)
	{
		if(existMCastEntry->mbr==0)
		{
			return SUCCESS;
		}
	}
	memset(&fwdDescriptor, 0, sizeof(rtl865x_mcast_fwd_descriptor_t ));
	rtl865x_addMulticastEntry(destIpAddr, srcIpAddr, (unsigned short)srcVlanId, (unsigned short)srcPort, &fwdDescriptor, TRUE, 0, 0, 0);
	return SUCCESS;
}

