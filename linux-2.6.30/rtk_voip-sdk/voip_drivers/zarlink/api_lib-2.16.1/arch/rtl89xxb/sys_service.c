/** \file sys_service.c
 * sys_service.c
 *
 *  This file implements the required system services for the API-II using a
 * Linux OS running on the UVB.  The user should replace the functions provided
 * here with the equivalent based on their OS and hardware.
 *
 * Copyright (c) 2005, Legerity Inc.
 * All rights reserved
 *
 * This software is the property of Legerity , Inc. Please refer to the
 * Non Disclosure Agreement (NDA) that you have signed for more information
 * on legal obligations in using, modifying or distributing this file.
 */
#include <linux/interrupt.h>
#include "vp_api_types.h"
#include "sys_service.h"
/*
 * VpSysWait() function implementation is needed only for CSLAC devices
 * (880, 790). For other devices this function could be commented.
 */
void
VpSysWait(
    uint8 time)  /* Time specified in increments of 125uS (e.g. 4 = 500uS) */
{
    /* Blocking delay function added here based on OS */
}
void VpSysSemaphoreInit(int semaphoreId)
{
}

/*
 * VpSysEnterCritical(), VpSysExitCritical():
 *
 *  These functions allow for disabling interrupts while executing nonreentrant
 * portions of VoicePath API code. Note that the following implementations of
 * enter/exit critical section functions are simple implementations. These
 * functions could be expanded (if required) to handle different critical
 * section types differently.
 *
 * Params:
 *  VpDeviceIdType deviceId: Device Id (chip select ID)
 *  VpCriticalSecType: Critical section type
 *
 * Return:
 *  Number of critical sections currently entered for the device.
 */

#ifdef CONFIG_DEFAULTS_KERNEL_2_6
int flags;
spinlock_t VpSysSpinLock;
#else
static unsigned long flags = 0;
#endif
 
uint8
VpSysEnterCritical(
    VpDeviceIdType deviceId,
    VpCriticalSecType criticalSecType)
{
    /*
     * Code to check if semaphore can be taken, and if so increment. Block if
     * semaphore cannot be taken for this process ID.
     */
	
#ifdef CONFIG_DEFAULTS_KERNEL_2_6
    spin_lock_irqsave(VpSysSpinLock,flags); //for Linux 2.6 style migration
#else
	save_flags(flags); cli();
#endif
    /*
     * criticalDepth++;
     * return criticalDepth;
     */
    /* Prevent compiler from generating error */
    return 0;
} /* VpSysEnterCritical() */

uint8
VpSysExitCritical(
    VpDeviceIdType deviceId,
    VpCriticalSecType criticalSecType)
{
    /* Code to decrement semaphore */
    
#ifdef CONFIG_DEFAULTS_KERNEL_2_6
    spin_unlock_irqrestore(VpSysSpinLock,flags); 
#else
    restore_flags(flags);
#endif
    /*
     * criticalDepth--;
     * return criticalDepth;
     */
    /* Prevent compiler from generating error */
    return 0;
} /* VpSysExitCritical() */

/**
 * VpSysDisableInt(), VpSysEnableInt(), and VpSysTestInt()
 *
 *  These functions are used by the CSLAC device family for interrupt driven
 * polling modes. These are called by the API to detect when a non-masked
 * device status has changed.  If using SIMPLE_POLL mode, these functions do not
 * require implementation.
 *
 * Preconditions:
 *  None. The implementation of these functions is architecture dependent.
 *
 * Postconditions:
 *  VpSysDisableInt() - The interrupt associated with the deviceId passed is
 * disabled.
 *
 * VpSysEnableInt() - The interrupt associated with the deviceId passed is
 * enabled.
 *
 * VpSysTestInt() - The return value is TRUE if an interrupt occurred, otherwise
 * return FALSE.
 *
 * These functions are needed only for CSLAC devices
 * (880, 790). For other devices these functions could be commented.
 *
 */
void
VpSysDisableInt(
    VpDeviceIdType deviceId)
{
    return;
}
void
VpSysEnableInt(
    VpDeviceIdType deviceId)
{
    return;
}
bool
VpSysTestInt(
    VpDeviceIdType deviceId)
{
    return FALSE;
}
/**
 * VpSysDtmfDetEnable(), VpSysDtmfDetDisable()
 *
 *  These functions are used by the CSLAC device family for devices that do not
 * internally detect DTMF. It is used for Caller ID type-II and is provided to
 * enable external DTMF detection.
 *
 * Preconditions:
 *  None. The implementation of these functions is application dependent.
 *
 * Postconditions:
 *  VpSysDtmfDetEnable() - The device/channel resource for DTMF detection is
 * enabled.
 *
 *  VpSysDtmfDetDisable() - The device/channel resource for DTMF detection is
 * disabled.
 *
 * These functions are needed only for CSLAC devices
 * (880, 790). For other devices these functions could be commented.
 *
 */
void
VpSysDtmfDetEnable(
    VpDeviceIdType deviceId,
    uint8 channelId)
{
}
void
VpSysDtmfDetDisable(
    VpDeviceIdType deviceId,
    uint8 channelId)
{
}
/*
 * The following functions VpSysTestHeapAcquire(),  VpSysTestHeapRelease()
 * VpSysPcmCollectAndProcess() and are needed only for CSLAC devices
 * (880). For other devices these functions could be commented. Please see
 * the LineTest API documentation for function requirements.
 */
void *
VpSysTestHeapAcquire(
    uint8 *pHeapId)
{
    return VP_NULL;
} /* VpSysTestHeapAcquire() */
bool
VpSysTestHeapRelease(
    uint8 heapId)
{
    return TRUE;
} /* VpSysTestHeapRelease() */
void
VpSysPcmCollectAndProcess(
    void *pLineCtx,
    VpDeviceIdType deviceId,
    uint8 channelId,
    uint8 startTimeslot,
    uint16 operationTime,
    uint16 settlingTime,
    uint16 operationMask)
{
} /* VpSysPcmCollectAndProcess() */
