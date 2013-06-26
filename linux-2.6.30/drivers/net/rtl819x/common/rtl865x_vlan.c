/*
* Copyright c                  Realtek Semiconductor Corporation, 2008  
* All rights reserved.
* 
* Program : Vlan driver
* Abstract : 
* Author : hyking (hyking_liu@realsil.com.cn)  
*/
#include <net/rtl/rtl_types.h>
#include <net/rtl/rtl_glue.h>
#include "rtl_errno.h"
//#include "rtl_utils.h"
//#include "rtl_glue.h"
#include "rtl865x_vlan.h"
#ifdef CONFIG_RTL_LAYERED_ASIC_DRIVER
#include "AsicDriver/rtl865x_asicCom.h"
#else
#include "AsicDriver/rtl865xC_tblAsicDrv.h"
#endif
#include "rtl865x_eventMgr.h"


static rtl865x_vlan_entry_t *vlanTbl = NULL;

static RTL_DECLARE_MUTEX(vlan_sem);

static int32 _rtl865x_delVlan(uint16 vid);

static int32 _rtl865x_setAsicVlan(uint16 vid,rtl865x_vlan_entry_t *vlanEntry)
{
	int32 retval = FAILED;
	rtl865x_tblAsicDrv_vlanParam_t asicEntry;
	/*add this entry to asic table*/
	asicEntry.fid = vlanEntry->fid;
	asicEntry.memberPortMask = vlanEntry->memberPortMask;
	asicEntry.untagPortMask = vlanEntry->untagPortMask;
	retval = rtl8651_setAsicVlan(vid,&asicEntry);
	return retval;
}


static int32 _rtl865x_referVlan(uint16 vid)
{
	rtl865x_vlan_entry_t *entry;
	/* vid should be legal vlan ID */
	if(vid < 1 || vid > VLAN_NUMBER-1)
		return RTL_EINVALIDVLANID;

	entry = &vlanTbl[vid];
	if(entry->valid != 1)
		return RTL_EINVALIDVLANID;

	entry->refCnt++;
	return SUCCESS;
}

static int32 _rtl865x_deReferVlan(uint16 vid)
{
	rtl865x_vlan_entry_t *entry;
	/* vid should be legal vlan ID */
	if(vid < 1 || vid > VLAN_NUMBER-1)
		return RTL_EINVALIDVLANID;

	entry = &vlanTbl[vid];
	if(entry->valid != 1)
		return RTL_EINVALIDVLANID;

	entry->refCnt--;
	return SUCCESS;
}

int32 _rtl865x_addVlan(uint16 vid)
{
	int32 retval = FAILED;
	rtl865x_vlan_entry_t *entry;
	entry=&vlanTbl[vid];
	
	if(1 == entry->valid)
		return RTL_EVLANALREADYEXISTS;

	/*add new vlan entry*/
	memset(entry,0,sizeof(rtl865x_vlan_entry_t));
	entry->vid = vid;
	entry->valid = 1;
	entry->refCnt = 1;

	/*add this entry to asic table*/
	retval = _rtl865x_setAsicVlan(vid,entry);	
	
	return retval;
}


static int32 _rtl865x_delVlan(uint16 vid)
{
	int32 retval = FAILED;
	rtl865x_vlan_entry_t *vlanEntry,org;
	vlanEntry=&vlanTbl[vid];
	if(0 == vlanEntry->valid)
		return RTL_EINVALIDVLANID;
	/*
	if(vlanEntry->refCnt > 1)
	{
		printk("vid(%d),reference(%d)\n",vid,vlanEntry->refCnt);
		return RTL_EREFERENCEDBYOTHER;
	}
	*/

	memcpy(&org,vlanEntry,sizeof(rtl865x_vlan_entry_t));
	
	/*delete vlan entry*/
	vlanEntry->valid =  0;
	/*ignor other member...*/

	retval = rtl8651_delAsicVlan(vid);

	if(SUCCESS == retval)
	{
		/*if vlan entry is deleted, this information should be noticed by uplayer module*/
		#if 0
		do_eventAction(EV_DEL_VLAN, (void *)&org);
		#else
		rtl865x_raiseEvent(EVENT_DEL_VLAN, (void *)&org);
		#endif
	}
	
	return retval;
}

static int32 _rtl865x_addVlanPortMember(uint16 vid, uint32 portMask)
{
	int32 retval = FAILED;
	rtl865x_vlan_entry_t *vlanEntry;
	
	vlanEntry = &vlanTbl[vid];
	if(vlanEntry->valid == 0)
		return RTL_EINVALIDVLANID;

	/*add member port*/
	vlanEntry->memberPortMask |= portMask;
	vlanEntry->untagPortMask |= portMask;

	/*update this entry to asic table*/
	retval = _rtl865x_setAsicVlan(vid,vlanEntry);

	return retval;
	
	
}


static int32 _rtl865x_delVlanPortMember(uint16 vid, uint32 portMask)
{
	int32 retval = FAILED;
	rtl865x_vlan_entry_t *vlanEntry;
	//rtl865x_tblAsicDrv_vlanParam_t asicEntry;
	
	vlanEntry = &vlanTbl[vid];
	if(vlanEntry->valid == 0)
		return RTL_EINVALIDVLANID;

	/*add member port*/
	vlanEntry->memberPortMask &= ~portMask;
	vlanEntry->untagPortMask &=~portMask;

	if(vlanEntry->memberPortMask == 0)
		vlanEntry->valid = 0;

	/*update this entry to asic table*/
	retval = _rtl865x_setAsicVlan(vid,vlanEntry);

	return retval;
	
	
}


static int32 _rtl865x_setVlanPortTag(uint16 vid, uint32 portMask, uint8 tag)
{
	int32 retval = FAILED;
	rtl865x_vlan_entry_t *vlanEntry;
	
	vlanEntry = &vlanTbl[vid];
	if(vlanEntry->valid == 0)
		return RTL_EINVALIDVLANID;

	if(tag == 0)
		vlanEntry->untagPortMask |= vlanEntry->memberPortMask & portMask;
	else
		vlanEntry->untagPortMask &=~(vlanEntry->memberPortMask & portMask);

	/*update this entry to asic table*/
	retval = _rtl865x_setAsicVlan(vid,vlanEntry);

	return retval;
	
}

static int32 _rtl865x_setVlanFID(uint16 vid, uint32 fid)
{
	int32 retval = FAILED;
	rtl865x_vlan_entry_t *vlanEntry;

	if(fid >= RTL865X_FDB_NUMBER)
		return RTL_EINVALIDFID;
	
	vlanEntry = &vlanTbl[vid];
	if(vlanEntry->valid == 0)
		return RTL_EINVALIDVLANID;

	vlanEntry->fid = fid;		

	/*update this entry to asic table*/
	retval = _rtl865x_setAsicVlan(vid,vlanEntry);

	return retval;
}


static int32 _rtl865x_getVlanFilterDatabaseId(uint16 vid, uint32 *fid)
{
	int32 retval = 0;
	
	if(vid < 1 || vid > VLAN_NUMBER -1)
		return RTL_EINVALIDVLANID;
	
	if(vlanTbl[vid].valid == 1)
	{
		*fid = vlanTbl[vid].fid;
		retval = SUCCESS;
	}
	else
	{
		printk("%s(%d):the vlan is invalid!!!BUG!!!!\n",__FUNCTION__,__LINE__);
		retval = FAILED;
	}

	return retval;
	
}
rtl865x_vlan_entry_t *_rtl8651_getVlanTableEntry(uint16 vid)
{
	if(vlanTbl[vid].valid == 1)
		return &vlanTbl[vid];
	return NULL;
}

/*
@func int32 | rtl865x_referVlan | reference a VLAN entry.
@parm uint16 | vid | VLAN ID.
@rvalue SUCCESS | Success.
@rvalue RTL_EINVALIDVLANID | Invalid VLAN ID.
@comm
if a vlan entry is referenced, please call this API.
*/
int32 rtl865x_referVlan(uint16 vid)
{
	int32 retval = FAILED;
	unsigned long flags;
	//rtl_down_interruptible(&vlan_sem);
	local_irq_save(flags);
	retval = _rtl865x_referVlan(vid);
	//rtl_up(&vlan_sem);
	local_irq_restore(flags);
	return retval;
}

/*
@func int32 | rtl865x_deReferVlan | dereference a VLAN entry.
@parm uint16 | vid | VLAN ID.
@rvalue SUCCESS | Success.
@rvalue RTL_EINVALIDVLANID | Invalid VLAN ID.
@comm
if a vlan entry is dereferenced, please call this API.
NOTE: rtl865x_deReferVlan should be called after rtl865x_referVlan.
*/
int32 rtl865x_deReferVlan(uint16 vid)
{
	int32 retval = FAILED;
	unsigned long flags;	
	//rtl_down_interruptible(&vlan_sem);
	local_irq_save(flags);
	retval = _rtl865x_deReferVlan(vid);
	//rtl_up(&vlan_sem);
	local_irq_restore(flags);
	return retval;
}

/*
@func int32 | rtl865x_addVlan | Add a VLAN.
@parm uint16 | vid | VLAN ID.
@rvalue SUCCESS | Success.
@rvalue RTL_EINVALIDVLANID | Invalid VLAN ID.
@rvalue RTL_EVLANALREADYEXISTS | Vlan already exists.
*/
int32 rtl865x_addVlan(uint16 vid)
{
	int32 retval = FAILED;
	unsigned long flags;
	/* vid should be legal vlan ID */
	if(vid < 1 || vid > VLAN_NUMBER-1)
		return RTL_EINVALIDVLANID;

	//rtl_down_interruptible(&vlan_sem);
	local_irq_save(flags);
	retval = _rtl865x_addVlan(vid);
	//rtl_up(&vlan_sem);
	local_irq_restore(flags);
	return retval;
		
}

/*
@func int32 | rtl865x_delVlan | Delete a VLAN.
@parm uint16 | vid | VLAN ID.
@rvalue SUCCESS | Success.
@rvalue RTL_EINVALIDVLANID | Invalid VLAN ID.
@rvalue RTL_EREFERENCEDBYOTHER | the Vlan is referenced by other,please delete releated table entry first.
*/
int32 rtl865x_delVlan(uint16 vid)
{
	int32 retval = FAILED;
	unsigned long flags;	
	/* vid should be legal vlan ID */
	if(vid < 1 || vid > VLAN_NUMBER -1)
		return RTL_EINVALIDVLANID;
	
	//rtl_down_interruptible(&vlan_sem);
	local_irq_save(flags);
	retval = _rtl865x_delVlan(vid);	
	//rtl_up(&vlan_sem);
	local_irq_restore(flags);
	return retval;
}

/*
@func int32 | rtl865x_addVlanPortMember | configure vlan member port
@parm uint16 | vid | VLAN ID.
@parm uint32 | portMask | Port mask.
@rvalue SUCCESS | Success.
@rvalue RTL_EINVALIDVLANID | Invalid VLAN ID.
@comm
the parm portMask is the MASK of port. bit0 mapping to physical port 0,bit1 mapping to physical port 1.
*/
int32 rtl865x_addVlanPortMember(uint16 vid, uint32 portMask)
{
	int32 retval = FAILED;
	unsigned long flags;
	/* vid should be legal vlan ID */
	if(vid < 1 || vid > VLAN_NUMBER -1)
		return RTL_EINVALIDVLANID;

	//rtl_down_interruptible(&vlan_sem);
	local_irq_save(flags);
	retval = _rtl865x_addVlanPortMember(vid,portMask);	
	//rtl_up(&vlan_sem);
	local_irq_restore(flags);
	return retval;
}

/*
@func int32 | rtl865x_delVlanPortMember | delete vlan's member port
@parm uint16 | vid | VLAN ID.
@parm uint32 | portMask | Port mask.
@rvalue SUCCESS | Success.
@rvalue RTL_EINVALIDVLANID | Invalid VLAN ID.
@comm
the parm portMask is the MASK of port. bit0 mapping to physical port 0,bit1 mapping to physical port 1.
*/
int32 rtl865x_delVlanPortMember(uint16 vid,uint32 portMask)
{
	int32 retval = FAILED;
	unsigned long flags;
	/* vid should be legal vlan ID */
	if(vid < 1 || vid > VLAN_NUMBER -1)
		return RTL_EINVALIDVLANID;
	
	//rtl_down_interruptible(&vlan_sem);
	local_irq_save(flags);
	retval = _rtl865x_delVlanPortMember(vid,portMask);	
	//rtl_up(&vlan_sem);
	local_irq_restore(flags);

	return retval;
	
}


/*
@func uint32 | rtl865x_getVlanPortMask | get the member portMask of a vlan
@parm uint16 | vid | VLAN ID.
@comm
if the retrun value is zero, it means vlan entry is invalid or no member port in this vlan.
*/
uint32 rtl865x_getVlanPortMask(uint32 vid)
{
	rtl865x_vlan_entry_t *vlanEntry;
	
	if((vid < 1) || (vid > VLAN_NUMBER -1))
	{
		return 0;
	}
	
	vlanEntry = &vlanTbl[vid];

	if(vlanEntry->valid == 0)
	{
		return 0;
	}	
	
	return vlanEntry->memberPortMask;
}


/*
@func int32 | rtl865x_setVlanPortTag | configure member port vlan tag attribute
@parm uint16 | vid | VLAN ID.
@parm uint32 | portMask | Port mask.
@parm uint8 | portMask | vlantag or untag.
@rvalue SUCCESS | Success.
@rvalue RTL_EINVALIDVLANID | Invalid VLAN ID.
@comm
the parm portMask is the MASK of port. bit0 mapping to physical port 0,bit1 mapping to physical port 1.
parm tag is used to indicated physical port is vlantag or untag. value 1 means vlan tagged, and vlan 0 means vlan untagged.
*/
int32 rtl865x_setVlanPortTag(uint16 vid,uint32 portMask,uint8 tag)
{
	int32 retval = FAILED;
	unsigned long flags;	
	/* vid should be legal vlan ID */
	if(vid < 1 || vid > VLAN_NUMBER -1)
		return RTL_EINVALIDVLANID;
	
	//rtl_down_interruptible(&vlan_sem);
	local_irq_save(flags);
	retval = _rtl865x_setVlanPortTag(vid,portMask,tag);	
	//rtl_up(&vlan_sem);
	local_irq_restore(flags);
	return retval;
}

/*
@func int32 | rtl865x_setVlanFilterDatabase | configure the filter database for a vlan.
@parm uint16 | vid | VLAN ID.
@parm uint32 | fid | filter data base ID.
@rvalue SUCCESS | Success.
@rvalue RTL_EINVALIDVLANID | Invalid VLAN ID.
@rvalue RTL_EINVALIDFID | Invalid filter database ID.
@comm
in realtek 865x, 4 filter databases are support.
if you want to configure SVL for all vlan, please set the fid of vlan is same.
default configure is SVL.
*/
int32 rtl865x_setVlanFilterDatabase(uint16 vid, uint32 fid)
{
	int32 retval = FAILED;
	unsigned long flags;
	/* vid should be legal vlan ID */
	if(vid < 1 || vid > VLAN_NUMBER -1)
		return RTL_EINVALIDVLANID;
	
	//rtl_down_interruptible(&vlan_sem);//Lock resource
	local_irq_save(flags);
	retval = _rtl865x_setVlanFID(vid,fid);	
	//rtl_up(&vlan_sem);
	local_irq_restore(flags);

	return retval;	
}

/*
@func int32 | rtl865x_getVlanFilterDatabaseId | get the vlan's filter database ID.
@parm uint16 | vid | VLAN ID.
@parm uint32 | fid | filter data base ID.
@rvalue SUCCESS | Success.
@rvalue RTL_EINVALIDVLANID | Invalid VLAN ID.
@comm
*/
int32 rtl865x_getVlanFilterDatabaseId(uint16 vid, uint32 *fid)
{
	int32 retval = FAILED;
	retval = _rtl865x_getVlanFilterDatabaseId(vid, fid);

	return retval;
}

/*
@func int32 | rtl865x_initVlanTable | initialize vlan table.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failed,system should be reboot.
*/
int32 rtl865x_initVlanTable(void)
{	
	TBL_MEM_ALLOC(vlanTbl, rtl865x_vlan_entry_t, VLAN_NUMBER);	
	memset(vlanTbl,0,sizeof(rtl865x_vlan_entry_t)*VLAN_NUMBER);

	return SUCCESS;	
}

/*
@func int32 | rtl865x_reinitVlantable | initialize vlan table.
@rvalue SUCCESS | Success.
*/
int32 rtl865x_reinitVlantable(void)
{
	uint16 i;
	for(i = 0; i < VLAN_NUMBER; i++)
	{
		if(vlanTbl[i].valid)
		{
			_rtl865x_delVlan(i);
		}
	}
	return SUCCESS;
}

