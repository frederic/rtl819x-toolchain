/** \file vp_api.c
 * vp_api.c
 *
 *  This file contains the implementation of top level VoicePath API-II.
 *
 * Copyright (c) 2010, Zarlink Semiconductor, Inc.
 *
 * $Revision: 6423 $
 * $LastChangedDate: 2010-02-12 17:01:34 -0600 (Fri, 12 Feb 2010) $
 */

/* INCLUDES */
#include "vp_api.h"     /* Typedefs and function prototypes for API */

#include "vp_hal.h"
#include "vp_api_int.h" /* Device specific typedefs and function prototypes */
#include "sys_service.h"

#if defined (VP_CC_880_SERIES)
#include "vp880_api_int.h"
#endif

typedef void (*VpTempFuncPtrType) (void);

/* Macros for calling a device-specific API function using the pointer in the
   Device Context: */
#define VP_CALL_DEV_FUNC(func, args) \
    (((pDevCtx->funPtrsToApiFuncs.func) == VP_NULL) ? VP_STATUS_FUNC_NOT_SUPPORTED : (pDevCtx->funPtrsToApiFuncs.func) args )

/******************************************************************************
 *                     SYSTEM CONFIGURATION FUNCTIONS                         *
 ******************************************************************************/
/**
 * VpMakeDeviceObject()
 *  This function creates a device context using the information that is
 * provided. This funciton should be the first API function that should be
 * called. This function is like C++ constructor. If the passed device type is
 * not valid or the code for the device type is not compiled in, this function
 * returns error. Please see VP-API documentation for more information.
 *
 * Preconditions:
 *  The device context, device object pointers must be non zero and device type
 * must be valid. The type of device object should match with device type. The
 * deviceId must uniquely determine a chipselect for the device of interest in
 * HAL layer.
 *
 * Postconditions:
 *  The device context and device object are initialized and this function
 * returns success if context is created properly.
 */
VpStatusType
VpMakeDeviceObject(
    VpDeviceType deviceType,    /**< Device Type */
    VpDeviceIdType deviceId,    /**< Hardware chip select for this device */

    VpDevCtxType *pDevCtx,      /**< Device Context to be initialized by other
                                 * input
                                 */
    void *pDevObj)              /**< Device Object to be pointed to by device
                                 * context
                                 */
{
    uint8 i;
    VpTempFuncPtrType *funcPtr;
    VpStatusType status;
    VP_API_ENTER(None, VP_NULL, "MakeDeviceObject");

    /* Basic argument checking */
    if ((pDevObj == VP_NULL) || (pDevCtx == VP_NULL)) {
        VP_API_EXIT(None, VP_NULL, "MakeDeviceObject", VP_STATUS_INVALID_ARG);
        return VP_STATUS_INVALID_ARG;
    } else if ((deviceType != VP_DEV_VCP_SERIES) &&
        (deviceType != VP_DEV_VCP2_SERIES) &&
        (deviceType != VP_DEV_880_SERIES) &&
        (deviceType != VP_DEV_890_SERIES) &&
        (deviceType != VP_DEV_790_SERIES) &&
        (deviceType != VP_DEV_580_SERIES) &&
        (deviceType != VP_DEV_792_SERIES) &&
        (deviceType != VP_DEV_792_GROUP) &&
        (deviceType != VP_DEV_KWRAP)) {
        VP_API_EXIT(None, VP_NULL, "MakeDeviceObject", VP_STATUS_ERR_VTD_CODE);
        return VP_STATUS_ERR_VTD_CODE;
    }

    VpSysEnterCritical(deviceId, VP_CODE_CRITICAL_SEC);

    if(pDevCtx != VP_NULL) {
        /* User wants to create device context as well; First clear it */
        /* Initialize All the funciton pointers to zero */
        funcPtr = (VpTempFuncPtrType *)&pDevCtx->funPtrsToApiFuncs;
        for(i = 0; i < (sizeof(ApiFunctions) / sizeof(VpTempFuncPtrType)); i++){
            *funcPtr = VP_NULL;
            funcPtr++;
        }

        /* Initialize all the line context pointers to null */
        for(i = 0; i < VP_MAX_LINES_PER_DEVICE; i++) {
            pDevCtx->pLineCtx[i] = VP_NULL;
        }
    }

    pDevCtx->deviceType = deviceType;

    switch (deviceType) {
#if defined (VP_CC_VCP_SERIES)
        case VP_DEV_VCP_SERIES:
            ((VpVcpDeviceObjectType *)pDevObj)->deviceId = deviceId;
            status = VpMakeVcpDeviceObject(pDevCtx, pDevObj);
            break;
#endif

#if defined (VP_CC_VCP2_SERIES)
        case VP_DEV_VCP2_SERIES:
            ((VpVcp2DeviceObjectType *)pDevObj)->deviceId = deviceId;
            status = Vcp2MakeDeviceObject(pDevCtx, pDevObj);
            break;
#endif

#if defined (VP_CC_880_SERIES)
        case VP_DEV_880_SERIES:
            status = VpMakeVp880DeviceObject(pDevCtx, pDevObj);
            ((Vp880DeviceObjectType *)pDevObj)->deviceId = deviceId;
            ((Vp880DeviceObjectType *)pDevObj)->staticInfo.rcnPcn[0] = 0;
            break;
#endif

#if defined (VP_CC_890_SERIES)
        case VP_DEV_890_SERIES:
            status = VpMakeVp890DeviceObject(pDevCtx, pDevObj);
            ((Vp890DeviceObjectType *)pDevObj)->deviceId = deviceId;
            break;
#endif

#if defined (VP_CC_790_SERIES)
        case VP_DEV_790_SERIES:
            status = VpMakeVp790DeviceObject(pDevCtx, pDevObj);
            ((Vp790DeviceObjectType *)pDevObj)->deviceId = deviceId;
            break;
#endif

#if defined (VP_CC_792_SERIES)
        case VP_DEV_792_SERIES:
            status = Vp792MakeDeviceObject(pDevCtx, pDevObj);
            ((Vp792DeviceObjectType *)pDevObj)->deviceId = deviceId;
            break;
#endif

#if defined (VP_CC_792_GROUP)
        case VP_DEV_792_GROUP:
            pDevCtx->pDevObj = pDevObj;
            pDevCtx->funPtrsToApiFuncs.GetEvent = Vp792GroupGetEvent;
            ((Vp792GroupDeviceObjectType *)pDevObj)->deviceId = deviceId;
            status = VP_STATUS_SUCCESS;
            break;
#endif

#if defined (VP_CC_580_SERIES)
        case VP_DEV_580_SERIES:
            status = VpMakeVp580DeviceObject(pDevCtx, pDevObj);
            ((Vp580DeviceObjectType *)pDevObj)->deviceId = deviceId;
            break;
#endif

#if defined (VP_CC_KWRAP)
        case VP_DEV_KWRAP:
            status = VP_STATUS_FUNC_NOT_SUPPORTED;
            break;
#endif
        default:
            /*
             * This error check should be performed pointers are initialized, so
             * this code should never be executed -- error checking above
             */
            status = VP_STATUS_ERR_VTD_CODE;
            break;
    }
    VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);

    VP_API_EXIT(None, VP_NULL, "MakeDeviceObject", status);
    return status;
} /* VpMakeDeviceObject() */

/**
 * VpMakeDeviceCtx()
 *  This function links a device object to a device context and initializes
 * the function pointers.  This function must be called if VpMakeDeviceObject
 * has not been called with a valid device context. This function enables more
 * than one device context to be created for a shared device object referring to
 * to a given device. This feature is useful when more than one process is
 * controlling a given device.
 *
 * Preconditions:
 *  The device context and device object pointers must be non zero. Device
 * Object must be created before calling this function.
 *
 * Postconditions:
 *  The device context and device object are linked and the device context
 * function pointers are initialized.
 */
VpStatusType
VpMakeDeviceCtx(
    VpDeviceType deviceType,    /**< Device Type - must match type in pDevObj */
    VpDevCtxType *pDevCtx,      /**< Device Context to be initialized by other
                                 * input
                                 */
    void *pDevObj)              /**< Device Object to be pointed to by device
                                 * context
                                 */
{
    uint8 i;
    VpStatusType status;
    /* Pointer to a function pointer array (of same function type) */
    VpTempFuncPtrType *funcPtr;

    VP_API_ENTER(None, VP_NULL, "MakeDeviceCtx");

    /* Basic argument checking */
    if ((pDevCtx == VP_NULL) || (pDevObj == VP_NULL)) {
        VP_API_EXIT(None, VP_NULL, "MakeDeviceCtx", VP_STATUS_INVALID_ARG);
        return VP_STATUS_INVALID_ARG;
    }

    /* First Initialize all the funciton pointers to zero */
    funcPtr = (VpTempFuncPtrType *)&pDevCtx->funPtrsToApiFuncs;
    for(i = 0; i < (sizeof(ApiFunctions) / sizeof(VpTempFuncPtrType)); i++) {
        *funcPtr = VP_NULL;
        funcPtr++;
    }

    /* Initialize all the line context pointers to null */
    for(i = 0; i < VP_MAX_LINES_PER_DEVICE; i++) {
        pDevCtx->pLineCtx[i] = VP_NULL;
    }

    pDevCtx->deviceType = deviceType;

    switch (deviceType) {
#if defined (VP_CC_VCP_SERIES)
        case VP_DEV_VCP_SERIES:
            status = VpMakeVcpDeviceCtx(pDevCtx, pDevObj);
            break;
#endif

#if defined (VP_CC_VCP2_SERIES)
        case VP_DEV_VCP2_SERIES:
            status = Vcp2MakeDeviceCtx(pDevCtx, pDevObj);
            break;
#endif

#if defined (VP_CC_880_SERIES)
        case VP_DEV_880_SERIES:
            status = VpMakeVp880DeviceCtx(pDevCtx, pDevObj);
            break;
#endif

#if defined (VP_CC_890_SERIES)
        case VP_DEV_890_SERIES:
            status = VpMakeVp890DeviceCtx(pDevCtx, pDevObj);
            break;
#endif

#if defined (VP_CC_790_SERIES)
        case VP_DEV_790_SERIES:
            status = VpMakeVp790DeviceCtx(pDevCtx, pDevObj);
            break;
#endif

#if defined (VP_CC_792_SERIES)
        case VP_DEV_792_SERIES:
            status = Vp792MakeDeviceCtx(pDevCtx, pDevObj);
            break;
#endif

#if defined (VP_CC_580_SERIES)
        case VP_DEV_580_SERIES:
            status = VpMakeVp580DeviceCtx(pDevCtx, pDevObj);
            break;
#endif

#if defined (VP_CC_KWRAP)
        case VP_DEV_KWRAP:
            status = VP_STATUS_FUNC_NOT_SUPPORTED;
            break;
#endif
        default:
            status = VP_STATUS_ERR_VTD_CODE;
            break;
    }

    VP_API_EXIT(None, VP_NULL, "MakeDeviceCtx", status);
    return status;
} /* VpMakeDeviceCtx() */

/**
 * VpMakeLineObject()
 *  This function initializes a line context using the information that is
 * passed. This function is like a C++ constructor. It initializes the passed
 * line context and line object based on the paramters provided. The passed line
 * object type should match with the type of device object type. See VP-API
 * reference guide for more information.
 *
 * Preconditions:
 *  This function assumes device context has already been created and
 * initialized. This function should only be called after downloading the boot
 * image the device when applicable (like for VCP class of devices).
 *
 * Postconditions:
 *  This function initializes the line context/line object. Line related VP-API
 * functions can be called after calling this function.
 */
VpStatusType
VpMakeLineObject(
    VpTermType termType,
    uint8 channelId,
    VpLineCtxType *pLineCtx,
    void *pLineObj,
    VpDevCtxType *pDevCtx)
{
    VpStatusType status;
    VP_API_ENTER(VpDevCtxType, pDevCtx, "MakeLineObject");

    /* Basic argument checking */
    if ((pLineObj == VP_NULL) || (pDevCtx == VP_NULL) || (pLineCtx == VP_NULL)) {
        status = VP_STATUS_INVALID_ARG;
    } else {
        pLineCtx->pLineObj = VP_NULL;
        status = VP_CALL_DEV_FUNC(MakeLineObject, (termType, channelId, pLineCtx, pLineObj, pDevCtx));
    }

    VP_API_EXIT(VpDevCtxType, pDevCtx, "MakeLineObject", status);
    return status;
} /* VpMakeLineObject() */

/**
 * VpMakeLineCtx()
 *  This function makes the association from line context to line object and
 * device context. It must be called if VpMakeLineObject() is called with line
 * context set to VP_NULL. This function allows line objects and line contexts
 * to be created in separate steps. In multiprocess environment this function
 * allows associating more than one line contexts with one shared line object
 * for a given line.
 *
 * Preconditions:
 *  None of the arguments can be VP_NULL. Line object must be created before
 * calling this function.
 *
 * Postconditions:
 *  The line context is associated with the line object and device context.
 */
VpStatusType
VpMakeLineCtx(
    VpLineCtxType *pLineCtx,
    void *pLineObj,
    VpDevCtxType *pDevCtx)
{
    VpLineInfoType lineInfo;
    VpStatusType status;
    VP_API_ENTER(VpDevCtxType, pDevCtx, "MakeLineCtx");

    /* Basic argument checking */
    if ((pLineCtx == VP_NULL) || (pLineObj == VP_NULL) || (pDevCtx == VP_NULL)) {
        VP_API_EXIT(VpDevCtxType, pDevCtx, "MakeLineCtx", VP_STATUS_INVALID_ARG);
        return VP_STATUS_INVALID_ARG;
    }

    /*
     * Make the device context member of the line context point to the
     * device context passed. This links this line context with a specific
     * device context.
     */
    pLineCtx->pDevCtx = pDevCtx;

    /*
     * Make the line object member of this line context point to the passed
     * line object pointer
     */
    pLineCtx->pLineObj = pLineObj;

    /* Get channel id from line context */
    lineInfo.pLineCtx = pLineCtx;
    lineInfo.pDevCtx = VP_NULL;
    status = VpGetLineInfo(&lineInfo);

    /*
     * Make the indexed line context array in the device context point to the
     * line context passed.
     */
    if (status == VP_STATUS_SUCCESS) {
        pDevCtx->pLineCtx[lineInfo.channelId] = pLineCtx;
    }

    VP_API_EXIT(VpDevCtxType, pDevCtx, "MakeLineCtx", status);
    return status;
} /* VpMakeLineCtx() */

/**
 * VpFreeLineCtx()
 *  This function frees the association from line context to device context. It
 * must be called if the application is freeing the memory associated with the
 * line context or object.
 *
 * Preconditions:
 *  None.
 *
 * Postconditions:
 *  All areas of the API-II where this line context may be used is set to
 * VP_NULL.
 */
VpStatusType
VpFreeLineCtx(
    VpLineCtxType *pLineCtx)
{
    uint8 channelId;
    VpDevCtxType *pDevCtx;
    void *pLineObj, *pDevObj;
    VpDeviceIdType deviceId;    /**< Hardware chip select for this device */
    VP_API_ENTER(VpLineCtxType, pLineCtx, "VpFreeLineCtx");

    if (pLineCtx == VP_NULL) {
        VP_API_EXIT(VpLineCtxType, pLineCtx, "VpFreeLineCtx", VP_STATUS_INVALID_ARG);
        return VP_STATUS_INVALID_ARG;
    }

    pDevCtx = pLineCtx->pDevCtx;
    pDevObj = pDevCtx->pDevObj;
    pLineObj = pLineCtx->pLineObj;

    /*
     * Get the channel ID in the device context that is associated with this
     * line context.
     */

    switch(pDevCtx->deviceType) {
#if defined (VP_CC_VCP_SERIES)
        case VP_DEV_VCP_SERIES:
            channelId = ((VpVcpLineObjectType *)pLineObj)->channelId;
            deviceId = ((VpVcpDeviceObjectType *)pDevObj)->deviceId;
            break;
#endif

#if defined (VP_CC_VCP2_SERIES)
        case VP_DEV_VCP2_SERIES:
            channelId = ((VpVcp2LineObjectType *)pLineObj)->channelId;
            deviceId = ((VpVcp2DeviceObjectType *)pDevObj)->deviceId;
            break;
#endif

#if defined (VP_CC_880_SERIES)
        case VP_DEV_880_SERIES:
            channelId = ((Vp880LineObjectType *)pLineObj)->channelId;
            deviceId = ((Vp880DeviceObjectType *)pDevObj)->deviceId;
            break;
#endif

#if defined (VP_CC_890_SERIES)
        case VP_DEV_890_SERIES:
            channelId = ((Vp890LineObjectType *)pLineObj)->channelId;
            deviceId = ((Vp890DeviceObjectType *)pDevObj)->deviceId;
            break;
#endif

#if defined (VP_CC_790_SERIES)
        case VP_DEV_790_SERIES:
            channelId = ((Vp790LineObjectType *)pLineObj)->channelId;
            deviceId = ((Vp790DeviceObjectType *)pDevObj)->deviceId;
            break;
#endif

#if defined (VP_CC_792_SERIES)
        case VP_DEV_792_SERIES:
            channelId = ((Vp792LineObjectType *)pLineObj)->channelId;
            deviceId = ((Vp792DeviceObjectType *)pDevObj)->deviceId;
            break;
#endif

#if defined (VP_CC_580_SERIES)
        case VP_DEV_580_SERIES:
            channelId = ((Vp580LineObjectType *)pLineObj)->channelId;
            deviceId = ((Vp580DeviceObjectType *)pDevObj)->deviceId;
            break;
#endif

#if defined (VP_CC_KWRAP)
        case VP_DEV_KWRAP:
#if 0
            VP_API_EXIT(VpLineCtxType, pLineCtx, "VpFreeLineCtx", VP_STATUS_FUNC_NOT_SUPPORTED);
            return VP_STATUS_FUNC_NOT_SUPPORTED;
#endif
            channelId = ((VpKWrapLineObjectType *)pLineObj)->channelId;
            deviceId = ((VpKWrapDeviceObjectType *)pDevObj)->deviceId;
            break;

#endif

        default:
            VP_API_EXIT(VpLineCtxType, pLineCtx, "VpFreeLineCtx", VP_STATUS_INVALID_ARG);
            return VP_STATUS_INVALID_ARG;
    }

    /* Free the line context of this device */
    VpSysEnterCritical(deviceId, VP_CODE_CRITICAL_SEC);
    pDevCtx->pLineCtx[channelId] = VP_NULL;
    VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);

    VP_API_EXIT(VpLineCtxType, pLineCtx, "VpFreeLineCtx", VP_STATUS_SUCCESS);
    return VP_STATUS_SUCCESS;
} /* VpFreeLineCtx() */

/**
 * This function provides generic timer functionality.  Please see VP-API
 * documentation for more information.
 *
 * Preconditions:
 * This function assumes the passed line context is created and initialized.
 *
 * Postconditions:
 * Starts or cancels a timer.
 */
VpStatusType
VpGenTimerCtrl(
    VpLineCtxType *pLineCtx,
    VpGenTimerCtrlType timerCtrl,
    uint32 duration,
    uint16 handle)
{
    VpStatusType status;
    VP_API_ENTER(VpLineCtxType, pLineCtx, "GenTimerCtrl");

    /* Basic argument checking */
    if (pLineCtx == VP_NULL) {
        status = VP_STATUS_INVALID_ARG;
    } else {
        VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
        status = VP_CALL_DEV_FUNC(GenTimerCtrl, (pLineCtx, timerCtrl, duration, handle));
    }

    VP_API_EXIT(VpLineCtxType, pLineCtx, "GenTimerCtrl", status);
    return status;
} /* VpGenTimerCtrl() */

/*
 * VpGetDeviceInfo()
 * This function is used to obtain information about a device. Please see VP-API
 * documentation for more information.
 *
 * Preconditions:
 * This function assumes the passed line/device contexts are created and
 * initialized.
 *
 * Postconditions:
 * Returns the requested information.
 */
VpStatusType
VpGetDeviceInfo(
    VpDeviceInfoType *pDeviceInfo)
{
    VpLineCtxType *pLineCtx = VP_NULL;
    VpDevCtxType *pDevCtx;
    VpDeviceType devType;
    void *pDevObj;
    VpStatusType status = VP_STATUS_SUCCESS;

    /* Basic argument checking */
    if (pDeviceInfo == VP_NULL) {
        status = VP_STATUS_INVALID_ARG;
    } else {
        pLineCtx = pDeviceInfo->pLineCtx;
        pDevCtx = pDeviceInfo->pDevCtx;

        if ((pLineCtx == VP_NULL) && (pDevCtx == VP_NULL)) {
            status = VP_STATUS_INVALID_ARG;
        }
    }

    if (status != VP_STATUS_SUCCESS) {
        VP_API_EXIT(None, VP_NULL, "GetDeviceInfo", status);
        return status;
    }

    /* Extract device info from...*/
    if (pLineCtx != VP_NULL) {
        /* Line context */
        pDevCtx = pLineCtx->pDevCtx;
        pDeviceInfo->pDevCtx = pDevCtx;
    } else {
        /* Device context */
        pDevCtx = pDeviceInfo->pDevCtx;
        pDeviceInfo->pLineCtx = VP_NULL;
    }

    pDevObj = pDevCtx->pDevObj;
    devType = pDevCtx->deviceType;
    pDeviceInfo->deviceType = devType;
    pDeviceInfo->revCode = 0;
    pDeviceInfo->slacId = 0;
    pDeviceInfo->productCode = 0;
    pDeviceInfo->featureList.testLoadSwitch = VP_UNKNOWN;
    pDeviceInfo->featureList.internalTestTermination = VP_UNKNOWN;

    switch(devType) {
#if defined (VP_CC_VCP_SERIES)
        case VP_DEV_VCP_SERIES:
            pDeviceInfo->numLines =
                ((VpVcpDeviceObjectType *)pDevObj)->maxChannels;
            pDeviceInfo->deviceId =
                ((VpVcpDeviceObjectType *)pDevObj)->deviceId;
            break;
#endif

#if defined (VP_CC_VCP2_SERIES)
        case VP_DEV_VCP2_SERIES:
            pDeviceInfo->numLines =
                ((VpVcp2DeviceObjectType *)pDevObj)->numChannels;
            pDeviceInfo->deviceId =
                ((VpVcp2DeviceObjectType *)pDevObj)->deviceId;
            break;
#endif

#if defined (VP_CC_880_SERIES)
        case VP_DEV_880_SERIES:
            pDeviceInfo->numLines =
                ((Vp880DeviceObjectType *)pDevObj)->staticInfo.maxChannels;
            pDeviceInfo->deviceId =
                ((Vp880DeviceObjectType *)pDevObj)->deviceId;
            pDeviceInfo->revCode =
                ((Vp880DeviceObjectType *)pDevObj)->staticInfo.rcnPcn[0];
            pDeviceInfo->productCode =
                (uint16)((Vp880DeviceObjectType *)pDevObj)->staticInfo.rcnPcn[1];

            if (((Vp880DeviceObjectType *)pDevObj)->stateInt & VP880_HAS_TEST_LOAD_SWITCH) {
                pDeviceInfo->featureList.testLoadSwitch = VP_AVAILABLE;
            } else {
                pDeviceInfo->featureList.testLoadSwitch = VP_NOT_AVAILABLE;
            }

            /* The internal test termination is available when the test load
             * switch is not, as long as the product is newer than VC */
            if (pDeviceInfo->featureList.testLoadSwitch == VP_NOT_AVAILABLE &&
                pDeviceInfo->revCode > VP880_REV_VC)
            {
                pDeviceInfo->featureList.internalTestTermination = VP_AVAILABLE;
            } else {
                pDeviceInfo->featureList.internalTestTermination = VP_NOT_AVAILABLE;
            }
            break;
#endif

#if defined (VP_CC_890_SERIES)
        case VP_DEV_890_SERIES:
            pDeviceInfo->revCode =
                ((Vp890DeviceObjectType *)pDevObj)->staticInfo.rcnPcn[0];
            pDeviceInfo->productCode =
                (uint16)((Vp890DeviceObjectType *)pDevObj)->staticInfo.rcnPcn[1];

            pDeviceInfo->featureList.testLoadSwitch = VP_NOT_AVAILABLE;
            pDeviceInfo->featureList.internalTestTermination = VP_AVAILABLE;
            /*
             * This special test is done becuase the FXO is on physical line 1
             * (range [0:1]) but is a single channel device. So either the 890
             * specific library has to take care of all special instances where
             * channelId looping starts at 1 with maxChannels = 1 (normal looping
             * starts at 0), or we just have to change the number reported to
             * the application for maxChannels -- what is done here.
             */
            if(pDeviceInfo->productCode == (uint16)VP890_DEV_PCN_89010) {
                pDeviceInfo->numLines = 1;
            } else {
                pDeviceInfo->numLines =
                    ((Vp890DeviceObjectType *)pDevObj)->staticInfo.maxChannels;
            }

            pDeviceInfo->deviceId =
                ((Vp890DeviceObjectType *)pDevObj)->deviceId;
            break;
#endif

#if defined (VP_CC_790_SERIES)
        case VP_DEV_790_SERIES:
            pDeviceInfo->numLines =
                ((Vp790DeviceObjectType *)pDevObj)->staticInfo.maxChannels;
            pDeviceInfo->deviceId =
                ((Vp790DeviceObjectType *)pDevObj)->deviceId;
            pDeviceInfo->revCode =
                ((Vp790DeviceObjectType *)pDevObj)->staticInfo.rcnPcn[0];
            break;
#endif

#if defined (VP_CC_792_SERIES)
        case VP_DEV_792_SERIES:
            pDeviceInfo->numLines =
                ((Vp792DeviceObjectType *)pDevObj)->maxChannels;
            pDeviceInfo->deviceId =
                ((Vp792DeviceObjectType *)pDevObj)->deviceId;
            pDeviceInfo->slacId =
                ((Vp792DeviceObjectType *)pDevObj)->slacId;
            break;
#endif

#if defined (VP_CC_580_SERIES)
        case VP_DEV_580_SERIES:
            pDeviceInfo->numLines =
                ((Vp580DeviceObjectType *)pDevObj)->staticInfo.maxChannels;
            pDeviceInfo->deviceId =
                ((Vp580DeviceObjectType *)pDevObj)->deviceId;
            break;
#endif

#if defined (VP_CC_KWRAP)
        case VP_DEV_KWRAP:
            status = KWrapGetDeviceInfo(pDevCtx, pDeviceInfo);
            break;
#endif
        default:
            pDeviceInfo->numLines = 0;
            status = VP_STATUS_FAILURE;
            break;
    }

    if (pDeviceInfo->numLines > VP_MAX_LINES_PER_DEVICE) {
        pDeviceInfo->numLines = VP_MAX_LINES_PER_DEVICE;
    }

    VP_API_EXIT(None, VP_NULL, "GetDeviceInfo", status);
    return status;
} /* VpGetDeviceInfo() */

/**
 * VpGetLineInfo()
 *  This function is used to obtain information about a line. Please see VP-API
 * documentation for more information.
 *
 * Preconditions:
 *  This function assumes the line context and device context are already
 * created.
 *
 * Postconditions:
 *  Returns the requested information.
 */
VpStatusType
VpGetLineInfo(
    VpLineInfoType *pLineInfo)
{
    VpStatusType status = VP_STATUS_SUCCESS;

    if (
        (pLineInfo == VP_NULL) ||
        ((pLineInfo->pDevCtx == VP_NULL) && (pLineInfo->pLineCtx == VP_NULL))
    ) {
        //VP_API_ENTER(None, VP_NULL, "GetLineInfo");
        //VP_API_EXIT(None, VP_NULL, "GetLineInfo", VP_STATUS_INVALID_ARG);
        return VP_STATUS_INVALID_ARG;
    } else if (pLineInfo->pLineCtx != VP_NULL) {
        //VP_API_ENTER(VpLineCtxType, pLineInfo->pLineCtx, "GetLineInfo");

        if (pLineInfo->pDevCtx != VP_NULL) {
            //VP_API_EXIT(VpLineCtxType, pLineInfo->pLineCtx, "GetLineInfo", VP_STATUS_INVALID_ARG);
            return VP_STATUS_INVALID_ARG;
        }
        pLineInfo->pDevCtx = pLineInfo->pLineCtx->pDevCtx;
    } else /* (pLineInfo->pLineCtx == VP_NULL) && (pLineInfo->pDevCtx != VP_NULL) */ {
        uint8 channelId = pLineInfo->channelId;
        //VP_API_ENTER(VpDevCtxType, pLineInfo->pDevCtx, "GetLineInfo");

        if (channelId >= VP_MAX_LINES_PER_DEVICE) {
            status = VP_STATUS_INVALID_ARG;
        } else {
            pLineInfo->pLineCtx = pLineInfo->pDevCtx->pLineCtx[channelId];
            if (pLineInfo->pLineCtx == VP_NULL) {
                status = VP_STATUS_INVALID_LINE;
            }
        }

        if (status != VP_STATUS_SUCCESS) {
            //VP_API_EXIT(VpDevCtxType, pLineInfo->pDevCtx, "GetLineInfo", status);
            return status;
        }
    }

    /* Got valid Line Context, Device Context.  Get the other info from the Line Object. */
    switch (pLineInfo->pDevCtx->deviceType) {

#if defined (VP_CC_880_SERIES)
        case VP_DEV_880_SERIES: {
            Vp880LineObjectType *pLineObj = (Vp880LineObjectType *)pLineInfo->pLineCtx->pLineObj;
            pLineInfo->channelId = pLineObj->channelId;
            pLineInfo->termType = pLineObj->termType;
            pLineInfo->lineId = pLineObj->lineId;
            break;
        }
#endif

#if defined (VP_CC_890_SERIES)
        case VP_DEV_890_SERIES: {
            Vp890LineObjectType *pLineObj = (Vp890LineObjectType *)pLineInfo->pLineCtx->pLineObj;
            pLineInfo->channelId = pLineObj->channelId;
            pLineInfo->termType = pLineObj->termType;
            pLineInfo->lineId = pLineObj->lineId;
            break;
        }
#endif

#if defined (VP_CC_790_SERIES)
        case VP_DEV_790_SERIES: {
            Vp790LineObjectType *pLineObj = (Vp790LineObjectType *)pLineInfo->pLineCtx->pLineObj;
            pLineInfo->channelId = pLineObj->channelId;
            pLineInfo->termType = pLineObj->termType;
            pLineInfo->lineId = pLineObj->lineId;
            break;
        }
#endif

#if defined (VP_CC_792_SERIES)
        case VP_DEV_792_SERIES: {
            Vp792LineObjectType *pLineObj = (Vp792LineObjectType *)pLineInfo->pLineCtx->pLineObj;
            pLineInfo->channelId = pLineObj->channelId;
            pLineInfo->termType = pLineObj->termType;
            pLineInfo->lineId = pLineObj->lineId;
            break;
        }
#endif

#if defined (VP_CC_580_SERIES)
        case VP_DEV_580_SERIES: {
            Vp580LineObjectType *pLineObj = (Vp580LineObjectType *)pLineInfo->pLineCtx->pLineObj;
            pLineInfo->channelId = pLineObj->channelId;
            pLineInfo->termType = pLineObj->termType;
            pLineInfo->lineId = pLineObj->lineId;
            break;
        }
#endif

#if defined (VP_CC_VCP_SERIES)
        case VP_DEV_VCP_SERIES: {
            VpVcpLineObjectType *pLineObj = (VpVcpLineObjectType *)pLineInfo->pLineCtx->pLineObj;
            pLineInfo->channelId = pLineObj->channelId;
            pLineInfo->termType = pLineObj->termType;
            pLineInfo->lineId = pLineObj->lineId;
            break;
        }
#endif

#if defined (VP_CC_VCP2_SERIES)
        case VP_DEV_VCP2_SERIES: {
            VpVcp2LineObjectType *pLineObj = (VpVcp2LineObjectType *)pLineInfo->pLineCtx->pLineObj;
            pLineInfo->channelId = pLineObj->channelId;
            pLineInfo->termType = pLineObj->termType;
            pLineInfo->lineId = pLineObj->lineId;
            break;
        }
#endif

#if defined (VP_CC_KWRAP)
        case VP_DEV_KWRAP: {
            VpKWrapLineObjectType *pLineObj = (VpKWrapLineObjectType *)pLineInfo->pLineCtx->pLineObj;
            pLineInfo->channelId = pLineObj->channelId;
            pLineInfo->termType = pLineObj->termType;
            pLineInfo->lineId = pLineObj->lineId;
            break;
        }
#endif

        default:
            status = VP_STATUS_INVALID_ARG;
    }

    //VP_API_EXIT(VpLineCtxType, pLineInfo->pLineCtx, "GetLineInfo", status);
    return status;
} /* VpGetLineInfo() */


/******************************************************************************
 *                        INITIALIZATION FUNCTIONS                            *
 ******************************************************************************/
/*
 * VpBootLoad()
 * This function is used to download the boot image to the device.
 * This function must be called upon power on reset for VCP classes
 * of devices to download boot image for these devices. This function is
 * not applicable for CSLAC class of devices. See VP-API-II documentation
 * for more information about this function.
 *
 * Preconditions:
 * Device context must be created before calling this function.
 *
 * Postconditions:
 * This function downloads the boot image to the part and configures the part
 * to start excecuting the image that was downloaded.
 */
VpStatusType
VpBootLoad(
    VpDevCtxType *pDevCtx,
    VpBootStateType state,
    VpImagePtrType pImageBuffer,
    uint32 bufferSize,
    VpScratchMemType *pScratchMem,
    VpBootModeType validation)
{
    VpStatusType status;
    VP_API_ENTER(VpDevCtxType, pDevCtx, "BootLoad");

    if (pDevCtx == VP_NULL) {
        status = VP_STATUS_INVALID_ARG;
    } else {
        status = VP_CALL_DEV_FUNC(BootLoad, (pDevCtx, state, pImageBuffer, bufferSize, pScratchMem, validation));
    }

    VP_API_EXIT(VpDevCtxType, pDevCtx, "BootLoad", status);
    return status;
} /* VpBootLoad() */

/*
 * VpBootSlac()
 * This function is used to download the boot image to one or all SLACs
 * controlled by a VCP device without affecting the code on the VCP or other
 * SLACs.
 *
 * Preconditions:
 * Either pLineCtx or pDevCtx must be valid.  The other must be VP_NULL.
 *
 * Postconditions:
 * If pLineCtx is not VP_NULL, the SLAC associated with the given line will be
 * reset and loaded with the image provided.  If pDevCtx is not VP_NULL, all
 * SLACs associated with the given VCP device will be reset and loaded.
 */
VpStatusType
VpBootSlac(
    VpLineCtxType *pLineCtx,
    VpDevCtxType *pDevCtxParam,
    VpImagePtrType pImageBuffer,
    uint32 bufferSize)
{
    VpDevCtxType *pDevCtx;
    VpStatusType status = VP_STATUS_SUCCESS;
    bool singleSlac = (pLineCtx != VP_NULL);

    /* Basic argument checking */
    if (singleSlac) {
        VP_API_ENTER(VpLineCtxType, pLineCtx, "BootSlac");
        if (pDevCtxParam != VP_NULL) {
            status = VP_STATUS_INVALID_ARG;
        }
        pDevCtx = pLineCtx->pDevCtx;
    } else {
        pDevCtx = pDevCtxParam;
        VP_API_ENTER(VpDevCtxType, pDevCtx, "BootSlac");
        if (pDevCtx == VP_NULL) {
            status = VP_STATUS_INVALID_ARG;
        }
    }
    if (pImageBuffer == VP_NULL) {
        status = VP_STATUS_INVALID_ARG;
    }

    if (status == VP_STATUS_SUCCESS) {
        status = VP_CALL_DEV_FUNC(BootSlac, (pLineCtx, pDevCtxParam, pImageBuffer, bufferSize));
    }

#if (VP_CC_DEBUG_SELECT & VP_DBG_API_FUNC)
    if (singleSlac) {
        VP_API_EXIT(VpLineCtxType, pLineCtx, "BootSlac", status);
    } else {
        VP_API_EXIT(VpDevCtxType, pDevCtx, "BootSlac", status);
    }
#endif

    return status;
} /* VpBootSlac() */

/*
 * VpInitDevice()
 *  This function calibrates the device and initializes all lines (for which
 * line context has been created and intialized) on the device with the AC, DC,
 * and Ringing parameters passed. See VP-API reference guide for more
 * information.
 *
 * Preconditions:
 *  This function should be called only after creating and initializing the
 * device context and line context (atleast for those lines which need service).
 *
 * Postconditions:
 *  Device is calibrated and all lines (for whom line context has been created
 * and initialized) associated with this device are initialized with the AC, DC,
 * and Ringing Paramaters passed (DC and Ringing apply to FXS type lines only).
 */
VpStatusType
VpInitDevice(
    VpDevCtxType *pDevCtx,          /**< Pointer to device context for the
                                     * device that will be initialized
                                     */
    VpProfilePtrType pDevProfile,   /**< Pointer to Device Profile */
    VpProfilePtrType pAcProfile,    /**< Pointer to AC Profile that is applied
                                     * to all FXS lines on this device
                                     */
    VpProfilePtrType pDcProfile,    /**< Pointer to DC Profile that is applied
                                     * to all FXS lines on this device
                                     */
    VpProfilePtrType pRingProfile,  /**< Pointer to Ringing Profile that is
                                     * applied to all FXS lines on this device
                                     */
    VpProfilePtrType pFxoAcProfile, /**< Pointer to AC Profile that is applied
                                     * to all FXO lines on this device
                                     */
    VpProfilePtrType pFxoCfgProfile)/**< Pointer to Config Profile that is
                                     * applied to all FXO lines on this device
                                     */
{
    VpStatusType status;
    VP_API_ENTER(VpDevCtxType, pDevCtx, "InitDevice");

    /* Basic argument checking */
    if (pDevCtx == VP_NULL) {
        status = VP_STATUS_INVALID_ARG;
    } else {
        status = VP_CALL_DEV_FUNC(InitDevice, (pDevCtx, pDevProfile, pAcProfile, pDcProfile, pRingProfile, pFxoAcProfile, pFxoCfgProfile));
    }

    VP_API_EXIT(VpDevCtxType, pDevCtx, "InitDevice", status);
    return status;
} /* VpInitDevice() */

/*
 * VpInitSlac()
 *  This function performs the same operations on a per-SLAC basis that
 * VpInitDevice() does for all devices. See VpInitDevice() for details.
 *
 * Preconditions:
 *  The device and line context must be created and initialized and the
 * controller device must be initialized (via VpInitDevice).
 *
 * Postconditions:
 *  SLAC is calibrated and all lines associated with this SLAC are initialized
 * with the AC, DC, and Ringing Paramaters passed.
 */
VpStatusType
VpInitSlac(
    VpLineCtxType *pLineCtx,         /**< Pointer any line context associated
                                     * with the SLAC being initialized.
                                     */
    VpProfilePtrType pDevProfile,   /**< Pointer to Device Profile */
    VpProfilePtrType pAcProfile,    /**< Pointer to AC Profile that is applied
                                     * to all FXS lines on this SLAC
                                     */
    VpProfilePtrType pDcProfile,    /**< Pointer to DC Profile that is applied
                                     * to all FXS lines on this SLAC
                                     */
    VpProfilePtrType pRingProfile)  /**< Pointer to Ringing Profile that is
                                     * applied to all FXS lines on this SLAC
                                     */
{
    VpStatusType status;
    VP_API_ENTER(VpLineCtxType, pLineCtx, "InitSlac");

    /* Basic argument checking */
    if (pLineCtx == VP_NULL) {
        status = VP_STATUS_INVALID_ARG;
    } else {
        VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
        status = VP_CALL_DEV_FUNC(InitSlac, (pLineCtx, pDevProfile, pAcProfile, pDcProfile, pRingProfile));
    }

    VP_API_EXIT(VpLineCtxType, pLineCtx, "InitSlac", status);
    return status;
} /* VpInitSlac() */

/**
 * VpInitLine()
 *  This function initializes the line with the AC, DC, and Ringing parameters
 * passed. It resets the line, loads the default options, performs calibration
 * (if applicable) and it also results in an event being generated.
 * See VP-API reference guide for more information.
 *
 * Preconditions:
 *  The device and line context must be created and initialized also device must
 * be initialized (via VpInitDevice).
 *
 * Postconditions:
 *  The line is initialized with the AC, DC, and Ringing parameters passed.
 * DC and Ringing parameters apply to FXS lines only.
 */
VpStatusType
VpInitLine(
    VpLineCtxType *pLineCtx,
    VpProfilePtrType pAcProfile,
    VpProfilePtrType pDcFeedOrFxoCfgProfile,
    VpProfilePtrType pRingProfile)
{
    VpStatusType status;
    VP_API_ENTER(VpLineCtxType, pLineCtx, "InitLine");

    /* Basic argument checking */
    if (pLineCtx == VP_NULL) {
        status = VP_STATUS_INVALID_ARG;
    } else {
        VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
        status = VP_CALL_DEV_FUNC(InitLine, (pLineCtx, pAcProfile, pDcFeedOrFxoCfgProfile, pRingProfile));
    }

    VP_API_EXIT(VpLineCtxType, pLineCtx, "InitLine", status);
    return status;
} /* VpInitLine() */

/*
 * VpFreeRun()
 *  This function prepares the devices and lines for a system restart.
 *
 * Preconditions:
 *  This function should be called only after creating and initializing the
 * device context and line context (atleast for those lines which need service).
 *
 * Postconditions:
 *  Device and lines are in a system restart "ready" state.
 */
VpStatusType
VpFreeRun(
    VpDevCtxType *pDevCtx,          /**< Pointer to device context for the
                                     * device that will be initialized
                                     */
    VpFreeRunModeType freeRunMode)
{
    VpStatusType status;
    VP_API_ENTER(VpDevCtxType, pDevCtx, "FreeRun");

    /* Basic argument checking */
    if (pDevCtx == VP_NULL) {
        status = VP_STATUS_INVALID_ARG;
    } else {
        status = VP_CALL_DEV_FUNC(FreeRun, (pDevCtx, freeRunMode));
    }

    VP_API_EXIT(VpDevCtxType, pDevCtx, "FreeRun", status);
    return status;
} /* VpFreeRun() */

/**
 * VpConfigLine()
 *  This function initializes the line with the AC, DC, and Ringing parameters
 * passed. See VP-API reference guide for more information.
 *
 * Preconditions:
 *  The device and line context must be created and initialized also device must
 * be initialized (via VpInitDevice).
 *
 * Postconditions:
 *  The line is initialized with the AC, DC, and Ringing parameters passed.
 * DC and Ringing parameters apply to FXS lines only.
 */
VpStatusType
VpConfigLine(
    VpLineCtxType *pLineCtx,
    VpProfilePtrType pAcProfile,
    VpProfilePtrType pDcFeedOrFxoCfgProfile,
    VpProfilePtrType pRingProfile)
{
    VpStatusType status;
    VP_API_ENTER(VpLineCtxType, pLineCtx, "ConfigLine");

    /* Basic argument checking */
    if (pLineCtx == VP_NULL) {
        status = VP_STATUS_INVALID_ARG;
    } else {
        VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
        status = VP_CALL_DEV_FUNC(ConfigLine, (pLineCtx, pAcProfile, pDcFeedOrFxoCfgProfile, pRingProfile));
    }

    VP_API_EXIT(VpLineCtxType, pLineCtx, "ConfigLine", status);
    return status;
} /* VpConfigLine() */

/**
 * VpSetBFilter()
 *  This function enables the B-Filter and programs it with the B-Filter values
 * provided in the ac profile, or disables the B-Filter.
 *
 * Preconditions:
 *  The line must be created and initialized before calling this function.
 *
 * Postconditions:
 *  The B-Filter is programmed to either disabled values or to the values passed
 * in the AC-Profile.
 */
VpStatusType
VpSetBFilter(
    VpLineCtxType *pLineCtx,
    VpBFilterModeType bFiltMode,
    VpProfilePtrType pAcProfile)
{
    VpStatusType status;
    VP_API_ENTER(VpLineCtxType, pLineCtx, "SetBFilter");

    /* Basic argument checking */
    if (
        (pLineCtx == VP_NULL) ||
        ((bFiltMode == VP_BFILT_EN) && (pAcProfile == VP_NULL))
    ) {
        status = VP_STATUS_INVALID_ARG;
    } else {
        VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
        status = VP_CALL_DEV_FUNC(SetBFilter, (pLineCtx, bFiltMode, pAcProfile));
    }

    VP_API_EXIT(VpLineCtxType, pLineCtx, "SetBFilter", status);
    return status;
} /* VpSetBFilter() */

/**
 * VpSetBatteries()
 *  This function sets the battery values in the device, which for some devices
 * may result in improved feed performance.
 *
 * Preconditions:
 *  The device must be created and initialized before calling this function.
 *
 * Postconditions:
 *  The device is programmed to use the battery values provided, or programmed
 * to use measured battery voltages.
 */
VpStatusType
VpSetBatteries(
    VpLineCtxType *pLineCtx,
    VpBatteryModeType battMode,
    VpBatteryValuesType *pBatt)
{
    VpStatusType status;
    VP_API_ENTER(VpLineCtxType, pLineCtx, "SetBatteries");

    /* Basic argument checking */
    if (
        (pLineCtx == VP_NULL) ||
        ((battMode == VP_BFILT_EN) && (pBatt == VP_NULL))
    ) {
        status = VP_STATUS_INVALID_ARG;
    } else {
        VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
        status = VP_CALL_DEV_FUNC(SetBatteries, (pLineCtx, battMode, pBatt));
    }

    VP_API_EXIT(VpLineCtxType, pLineCtx, "SetBatteries", status);
    return status;
} /* VpSetBatteries() */

/**
 * VpCalCodec()
 *  This function initiates a calibration operation for analog circuits
 * associated with all the lines of a device. See VP-API reference guide for
 * more information.

 * Preconditions:
 *  The device and line context must be created and initialized before calling
 * this function.
 *
 * Postconditions:
 *  This function generates an event upon completing the requested action.
 */
VpStatusType
VpCalCodec(
    VpLineCtxType *pLineCtx,
    VpDeviceCalType mode)
{
    VpStatusType status;
    VP_API_ENTER(VpLineCtxType, pLineCtx, "CalCodec");

    /* Basic argument checking */
    if (
        (pLineCtx == VP_NULL) ||
        ((mode != VP_DEV_CAL_NOW) && (mode != VP_DEV_CAL_NBUSY))
    ) {
       status = VP_STATUS_INVALID_ARG;
    } else {
        VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
        status = VP_CALL_DEV_FUNC(CalCodec, (pLineCtx, mode));
    }

    VP_API_EXIT(VpLineCtxType, pLineCtx, "CalCodec", status);
    return status;
} /* VpCalCodec() */


/**
 * VpCalLine()
 *  This function initiates a calibration operation for analog circuits
 * associated with a given line. See VP-API reference guide for more
 * information.

 * Preconditions:
 *  The device and line context must be created and initialized before calling
 * this function.
 *
 * Postconditions:
 *  This function generates an event upon completing the requested action.
 */
VpStatusType
VpCalLine(
    VpLineCtxType *pLineCtx)
{
    VpStatusType status;
    VP_API_ENTER(VpLineCtxType, pLineCtx, "CalLine");

    /* Basic argument checking */
    if (pLineCtx == VP_NULL) {
        status = VP_STATUS_INVALID_ARG;
    } else {
        VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
        status = VP_CALL_DEV_FUNC(CalLine, (pLineCtx));
    }

    VP_API_EXIT(VpLineCtxType, pLineCtx, "CalLine", status);
    return status;
} /* VpCalLine() */

/**
 * VpCal()
 *  This function runs a selected calibration option.
 *
 * Preconditions:
 *  The device and line context must be created and initialized before calling
 * this function.
 *
 * Postconditions:
 *  This function generates an event upon completing the requested action. Event
 * indicates if calibration was successfull and if results are available.
 */
VpStatusType
VpCal(
    VpLineCtxType *pLineCtx,
    VpCalType calType,
    void *inputArgs)
{
    VpStatusType status;
    VP_API_ENTER(VpLineCtxType, pLineCtx, "Cal");

    /* Basic argument checking */
    if (pLineCtx == VP_NULL) {
        status = VP_STATUS_INVALID_ARG;
    } else {
        VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
        status = VP_CALL_DEV_FUNC(Cal, (pLineCtx, calType, inputArgs));
    }

    VP_API_EXIT(VpLineCtxType, pLineCtx, "Cal", status);
    return status;
} /* VpCal() */

/**
 * VpInitRing()
 *  This function is used to initialize Ringing and CID profile to a given line.
 * See VP-API reference guide for more information.
 *
 * Preconditions:
 *  The line context and device context should be created initialized. The boot
 * image should be downloaded before calling this function (for applicable
 * devices).
 *
 * Postconditions:
 *  Applies the Caller ID and Ringing profile.
 */
VpStatusType
VpInitRing(
    VpLineCtxType *pLineCtx,        /**< Line context */
    VpProfilePtrType pCadProfile,   /**< Ringing cadence profile */
    VpProfilePtrType pCidProfile)   /**< Caller ID profile */
{
    VpStatusType status;
    VP_API_ENTER(VpLineCtxType, pLineCtx, "InitRing");

    /* Basic argument checking */
    if (pCadProfile == VP_NULL && pCidProfile != VP_NULL) {
        /* It doesn't make sense to load a caller ID profile with
         * no ring cadence */
        VP_ERROR(VpLineCtxType, pLineCtx, ("VpInitRing(): Cannot use a NULL ring cadence with a non-NULL caller ID profile"));
        status = VP_STATUS_INVALID_ARG;
    } else if (pLineCtx == VP_NULL) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("VpInitRing(): Invalid NULL pLineCtx"));
        status = VP_STATUS_INVALID_ARG;
    } else {
        VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
        status = VP_CALL_DEV_FUNC(InitRing, (pLineCtx, pCadProfile, pCidProfile));
    }

    VP_API_EXIT(VpLineCtxType, pLineCtx, "InitRing", status);
    return status;
} /* VpInitRing() */

/**
 * VpInitCid()
 *  This function is used to send caller ID information. See VP-API reference
 * guide for more information.
 *
 * Preconditions:
 *  The device and line context must be created and initialized before calling
 * this function. This function needs to be called before placing the line in to
 * ringing state.
 *
 * Postconditions:
 *  This function transmits the given CID information on the line (when the line
 * is placed in the ringing state).
 */
VpStatusType
VpInitCid(
    VpLineCtxType *pLineCtx,
    uint8 length,
    uint8p pCidData)
{
    VpStatusType status;
    VP_API_ENTER(VpLineCtxType, pLineCtx, "InitCid");

    /* Basic argument checking */
    if (pLineCtx == VP_NULL) {
        status = VP_STATUS_INVALID_ARG;
    } else {
        VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
        status = VP_CALL_DEV_FUNC(InitCid, (pLineCtx, length, pCidData));
    }

    VP_API_EXIT(VpLineCtxType, pLineCtx, "InitCid", status);
    return status;
} /* VpInitCid() */

/**
 * VpInitMeter()
 *  This function is used to initialize metering parameters. See VP-API
 * reference guide for more information.
 *
 * Preconditions:
 *  The device and line context must be created and initialized before calling
 * this function. This function needs to be called before placing the line in to
 * ringing state.
 *
 * Postconditions:
 *  This function initializes metering parameters as per given profile.
 */
VpStatusType
VpInitMeter(
    VpLineCtxType *pLineCtx,
    VpProfilePtrType pMeterProfile)
{
    VpStatusType status;
    VP_API_ENTER(VpLineCtxType, pLineCtx, "InitMeter");

    /* Basic argument checking */
    if (pLineCtx == VP_NULL) {
        status = VP_STATUS_INVALID_ARG;
    } else if (pMeterProfile == VP_NULL) {
        /* If metering profile is null, there is nothing to do. */
        status = VP_STATUS_SUCCESS;
    } else {
        VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
        status = VP_CALL_DEV_FUNC(InitMeter, (pLineCtx, pMeterProfile));
    }

    VP_API_EXIT(VpLineCtxType, pLineCtx, "InitMeter", status);
    return status;
} /* VpInitMeter() */

/**
 * VpInitCustomTerm()
 *  This function is used to initialize the I/O control for a custom termination
 * type.
 *
 * Preconditions:
 *  The device and line context must be created and initialized before calling
 * this function. It is required to be called for term type CUSTOM before calling
 * VpInitDevice()/VpInitLine().
 *
 * Postconditions:
 *  This function initializes I/O control parameters as per given profile. When
 * called with device context only, all custom lines on the device are affected
 * by the profile passed. When a line context only is passed, then only that
 * line context is affected.
 */
VpStatusType
VpInitCustomTermType(
    VpDevCtxType *pDevCtxParam,
    VpLineCtxType *pLineCtx,
    VpProfilePtrType pCustomTermProfile)
{
    VpDevCtxType *pDevCtx;
    VpStatusType status = VP_STATUS_SUCCESS;
    bool singleLine = (pLineCtx != VP_NULL);

    /* Basic argument checking */
    if (singleLine) {
        VP_API_ENTER(VpLineCtxType, pLineCtx, "InitCustomTermType");
        if (pDevCtxParam != VP_NULL) {
            status = VP_STATUS_INVALID_ARG;
        }
        pDevCtx = pLineCtx->pDevCtx;
    } else {
        VP_API_ENTER(VpDevCtxType, pDevCtxParam, "InitCustomTermType");
        pDevCtx = pDevCtxParam;
        if (pDevCtx == VP_NULL) {
            status = VP_STATUS_INVALID_ARG;
        }
    }

    if (status == VP_STATUS_SUCCESS) {
        status = VP_CALL_DEV_FUNC(InitCustomTerm, (pDevCtxParam, pLineCtx, pCustomTermProfile));
    }

#if (VP_CC_DEBUG_SELECT & VP_DBG_API_FUNC)
    if (singleLine) {
        VP_API_EXIT(VpLineCtxType, pLineCtx, "InitCustomTermType", status);
    } else {
        VP_API_EXIT(VpDevCtxType, pDevCtxParam, "InitCustomTermType", status);
    }
#endif

    return status;
} /* VpInitCustomTermType() */

/**
 * VpInitProfile()
 *  This function is used to initialize profile tables in VCP. See
 * VP-API reference guide for more information.
 *
 * Preconditions:
 *  The device and line context must be created and initialized before calling
 * this function.
 *
 * Postconditions:
 *  Stores the given profile at the specified index of the profile table.
 */
VpStatusType
VpInitProfile(
    VpDevCtxType *pDevCtx,
    VpProfileType type,
    VpProfilePtrType pProfileIndex,
    VpProfilePtrType pProfile)
{
    VpStatusType status;
    VP_API_ENTER(VpDevCtxType, pDevCtx, "InitProfile");

    /* Basic argument checking */
    if (pDevCtx == VP_NULL || pProfileIndex == VP_PTABLE_NULL) {
        status = VP_STATUS_INVALID_ARG;
    } else {
        status = VP_CALL_DEV_FUNC(InitProfile, (pDevCtx, type, pProfileIndex, pProfile));
    }

    VP_API_EXIT(VpDevCtxType, pDevCtx, "InitProfile", status);
    return status;
} /* VpInitProfile() */

/**
 * VpSoftReset()
 *  This function resets VCP without requiring another image load.
 * See VP-API reference guide for more information.
 *
 * Preconditions:
 *  The device and must be created and initialized before calling this function.
 *
 * Postconditions:
 *  The part is reset.
 */
VpStatusType
VpSoftReset(
    VpDevCtxType *pDevCtx)
{
    VpStatusType status;
    VP_API_ENTER(VpDevCtxType, pDevCtx, "SoftReset");

    /* Basic argument checking */
    if (pDevCtx == VP_NULL) {
        status = VP_STATUS_INVALID_ARG;
    } else {
        status = VP_CALL_DEV_FUNC(SoftReset, (pDevCtx));
    }

    VP_API_EXIT(VpDevCtxType, pDevCtx, "SoftReset", status);
    return status;
} /* VpSoftReset() */

/******************************************************************************
 *                        CONTROL FUNCTIONS                                   *
 ******************************************************************************/
/**
 * VpSetLineState()
 *  This function sets a given line to indicated state. See VP-API-II
 * documentation for more information about this function.
 *
 * Preconditions:
 *  Device/Line context should be created and initialized. For applicable
 * devices bootload should be performed before calling the function.
 *
 * Postconditions:
 *  The indicated line is set to indicated line state.
 */
VpStatusType
VpSetLineState(
    VpLineCtxType *pLineCtx,
    VpLineStateType state)
{
    VpStatusType status;
    VP_API_ENTER(VpLineCtxType, pLineCtx, "SetLineState");

    /* Basic argument checking */
    if (pLineCtx == VP_NULL) {
        status = VP_STATUS_INVALID_ARG;
    } else {
        VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
        status = VP_CALL_DEV_FUNC(SetLineState, (pLineCtx, state));
    }

    VP_API_EXIT(VpLineCtxType, pLineCtx, "SetLineState", status);
    return status;
} /* VpSetLineState() */


/**
 * VpSetLineTone()
 *  This function is used to set a tone on a given line. See VP-API-II
 * documentation for more information about this function.
 *
 * Preconditions:
 *  Device/Line context should be created and initialized. For applicable
 * devices bootload should be performed before calling the function.
 *
 * Postconditions:
 *  Starts/Stops tone generation for a given line.
 */
VpStatusType
VpSetLineTone(
    VpLineCtxType *pLineCtx,
    VpProfilePtrType pToneProfile,
    VpProfilePtrType pCadProfile,
    VpDtmfToneGenType *pDtmfControl)
{
    VpStatusType status;
    VP_API_ENTER(VpLineCtxType, pLineCtx, "SetLineTone");

    /* Basic argument checking */
    if (pLineCtx == VP_NULL) {
        status = VP_STATUS_INVALID_ARG;
    } else {
        VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
        status = VP_CALL_DEV_FUNC(SetLineTone, (pLineCtx, pToneProfile, pCadProfile, pDtmfControl));
    }

    VP_API_EXIT(VpLineCtxType, pLineCtx, "SetLineTone", status);
    return status;
} /* VpSetLineTone() */

/**
 * VpSetRelayState()
 *  This function controls the state of VTD controlled relays. See VP-API-II
 * documentation for more information about this function.
 *
 * Preconditions:
 *  Device/Line context should be created and initialized. For applicable
 * devices bootload should be performed before calling the function.
 *
 * Postconditions:
 *  The indicated relay state is set for the given line.
 */
VpStatusType
VpSetRelayState(
    VpLineCtxType *pLineCtx,
    VpRelayControlType rState)
{
    VpStatusType status;
    VP_API_ENTER(VpLineCtxType, pLineCtx, "SetRelayState");

    /* Basic argument checking */
    if (pLineCtx == VP_NULL) {
        status = VP_STATUS_INVALID_ARG;
    } else {
        VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
        status = VP_CALL_DEV_FUNC(SetRelayState, (pLineCtx, rState));
    }

    VP_API_EXIT(VpLineCtxType, pLineCtx, "SetRelayState", status);
    return status;
} /* VpSetRelayState() */

/**
 * VpSetRelGain()
 *  This function adjusts the transmit and receive path relative gains. See
 * VP-API-II documentation for more information about this function.
 *
 * Preconditions:
 *  Device/Line context should be created and initialized. For applicable
 * devices bootload should be performed before calling the function.
 *
 * Postconditions:
 *  The requested relative gains will be applied.
 */
VpStatusType
VpSetRelGain(
    VpLineCtxType *pLineCtx,
    uint16 txLevel,
    uint16 rxLevel,
    uint16 handle)
{
    VpStatusType status;
    VP_API_ENTER(VpLineCtxType, pLineCtx, "SetRelGain");

    /* Basic argument checking */
    if (pLineCtx == VP_NULL) {
        status = VP_STATUS_INVALID_ARG;
    } else {
        VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
        status = VP_CALL_DEV_FUNC(SetRelGain, (pLineCtx, txLevel, rxLevel, handle));
    }

    VP_API_EXIT(VpLineCtxType, pLineCtx, "SetRelGain", status);
    return status;
} /* VpSetRelGain() */

/**
 * VpSendSignal()
 *  This function is used to send a signal on a line. The signal type is
 * specified by the type parameter and the parameters associated with the signal
 * type are specified by the structure pointer passed.
 *
 * Preconditions:
 *  Device/Line context should be created and initialized. For applicable
 * devices bootload should be performed before calling the function.
 *
 * Postconditions:
 *  Applies a signal to the line.
 */
VpStatusType
VpSendSignal(
    VpLineCtxType *pLineCtx,
    VpSendSignalType signalType,
    void *pSignalData)
{
    VpStatusType status;
    VP_API_ENTER(VpLineCtxType, pLineCtx, "SendSignal");

    /* Basic argument checking */
    if (pLineCtx == VP_NULL) {
        status = VP_STATUS_INVALID_ARG;
    } else {
        VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
        status = VP_CALL_DEV_FUNC(SendSignal, (pLineCtx, signalType, pSignalData));
    }

    VP_API_EXIT(VpLineCtxType, pLineCtx, "SendSignal", status);
    return status;
} /* VpSendSignal() */

/**
 * VpSendCid()
 *  This function may be used to send Caller ID information on-demand. See
 * VP-API-II documentation for more information about this function.
 *
 * Preconditions:
 *  Device/Line context should be created and initialized. For applicable
 * devices bootload should be performed before calling the function.
 *
 * Postconditions:
 * Caller ID information is transmitted on the line.
 */
VpStatusType
VpSendCid(
    VpLineCtxType *pLineCtx,
    uint8 length,
    VpProfilePtrType pCidProfile,
    uint8p pCidData)
{
    VpStatusType status;
    VP_API_ENTER(VpLineCtxType, pLineCtx, "SendCid");

    /* Basic argument checking */
    if (pLineCtx == VP_NULL) {
        status = VP_STATUS_INVALID_ARG;
    } else {
        VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
        status = VP_CALL_DEV_FUNC(SendCid, (pLineCtx, length, pCidProfile, pCidData));
    }

    VP_API_EXIT(VpLineCtxType, pLineCtx, "SendCid", status);
    return status;
} /* VpSendCid() */

/**
 * VpContinueCid()
 *  This function is called to provide more caller ID data (in response to
 * Caller ID data event from the VP-API). See VP-API-II  documentation
 * for more information about this function.
 *
 * Preconditions:
 *  Device/Line context should be created and initialized. For applicable
 * devices bootload should be performed before calling the function.
 *
 * Postconditions:
 *  Continues to transmit Caller ID information on the line.
 */
VpStatusType
VpContinueCid(
    VpLineCtxType *pLineCtx,
    uint8 length,
    uint8p pCidData)
{
    VpStatusType status;
    VP_API_ENTER(VpLineCtxType, pLineCtx, "ContinueCid");

    /* Basic argument checking */
    if (pLineCtx == VP_NULL) {
        status = VP_STATUS_INVALID_ARG;
    } else {
        VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
        status = VP_CALL_DEV_FUNC(ContinueCid, (pLineCtx, length, pCidData));
    }

    VP_API_EXIT(VpLineCtxType, pLineCtx, "ContinueCid", status);
    return status;
} /* VpContinueCid() */


/**
 * VpStartMeter()
 *  This function starts(can also abort) metering pulses on the line. See
 * VP-API-II documentation for more information about this function.
 *
 * Preconditions:
 *  Device/Line context should be created and initialized. For applicable
 * devices bootload should be performed before calling the function.
 *
 * Postconditions:
 *  Metering pulses are transmitted on the line.
 */
VpStatusType
VpStartMeter(
    VpLineCtxType *pLineCtx,
    uint16 onTime,
    uint16 offTime,
    uint16 numMeters)
{
    VpStatusType status;
    VP_API_ENTER(VpLineCtxType, pLineCtx, "StartMeter");

    /* Basic argument checking */
    if (pLineCtx == VP_NULL) {
        status = VP_STATUS_INVALID_ARG;
    } else {
        VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
        status = VP_CALL_DEV_FUNC(StartMeter, (pLineCtx, onTime, offTime, numMeters));
    }

    VP_API_EXIT(VpLineCtxType, pLineCtx, "StartMeter", status);
    return status;
} /* VpStartMeter() */

/**
 * VpStartMeter32Q()
 *  This function starts(can also abort) metering pulses on the line. See
 * VP-API-II documentation for more information about this function.  This
 * version of the function supports 32-bit minDelay, onTime, and offTime
 * parameters at 1ms increments.
 *
 * Preconditions:
 *  Device/Line context should be created and initialized. For applicable
 * devices bootload should be performed before calling the function.
 *
 * Postconditions:
 *  Metering pulses are transmitted on the line.
 */
VpStatusType
VpStartMeter32Q(
    VpLineCtxType *pLineCtx,
    uint32 minDelay,
    uint32 onTime,
    uint32 offTime,
    uint16 numMeters,
    uint16 eventRate)
{
    VpStatusType status;
    VP_API_ENTER(VpLineCtxType, pLineCtx, "StartMeter32Q");

    /* Basic argument checking */
    if (pLineCtx == VP_NULL) {
        status = VP_STATUS_INVALID_ARG;
    } else {
        VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
        status = VP_CALL_DEV_FUNC(StartMeter32Q, (pLineCtx, minDelay, onTime, offTime, numMeters, eventRate));
    }

    VP_API_EXIT(VpLineCtxType, pLineCtx, "StartMeter32Q", status);
    return status;
} /* VpStartMeter32Q() */

/* undocumented function used only by the Test Library: */
bool
VpReadCalFlag(
    VpLineCtxType *pLineCtx)
{
    bool retval;

    /* Basic argument checking */
    if (pLineCtx == VP_NULL) {
        retval = FALSE;
    } else switch (pLineCtx->pDevCtx->deviceType) {

#ifdef VP_CC_VCP2_SERIES
        case VP_DEV_VCP2_SERIES:
            retval = Vcp2ReadCalFlag(pLineCtx);
            break;
#endif

#if defined (VP_CC_KWRAP)
        case VP_DEV_KWRAP:
            retval = KWrapReadCalFlag(pLineCtx);
            break;
#endif

        default:
            retval = FALSE;
    }

    return retval;
}


/* undocumented function used only by the Test Library: */
void
VpSetCalFlag(
    VpLineCtxType *pLineCtx)
{
    /* Basic argument checking */
    if (pLineCtx == VP_NULL) {
        return;
    } else switch (pLineCtx->pDevCtx->deviceType) {

#ifdef VP_CC_VCP2_SERIES
        case VP_DEV_VCP2_SERIES:
            Vcp2SetCalFlag(pLineCtx);
            break;
#endif

#if defined (VP_CC_KWRAP)
        case VP_DEV_KWRAP:
            KWrapSetCalFlag(pLineCtx);
            break;
#endif
        default:
            break;
    }

    return;
}

/**
 * VpSetOption()
 *  This function is used to set various options that are supported by VP-API.
 * For a detailed description of how this function can be used to set device,
 * line specific, device specific options and to various types of options
 * please see VP-API user guide.
 *
 * Preconditions:
 *  Device/Line context should be created and initialized. For applicable
 * devices bootload should be performed before calling the function.
 *
 * Postconditions:
 *  This function sets the requested option.
 */
VpStatusType
VpSetOption(
    VpLineCtxType *pLineCtx,
    VpDevCtxType *pDevCtxParam,
    VpOptionIdType optionId,
    void *value)
{
    VpDevCtxType *pDevCtx;
    VpStatusType status = VP_STATUS_SUCCESS;
    bool singleLine = (pLineCtx != VP_NULL);

    /* Basic argument checking */
    if (singleLine) {
        VP_API_FUNC(VpLineCtxType, pLineCtx, ("VpSetOption(%s):", VpGetString_VpOptionIdType(optionId)));
        if (pDevCtxParam != VP_NULL) {
            status = VP_STATUS_INVALID_ARG;
        }
        pDevCtx = pLineCtx->pDevCtx;
    } else {
        pDevCtx = pDevCtxParam;
        if (pDevCtx == VP_NULL) {

#ifdef VP_DEBUG
            /* Special case: We need a way of modifying this global variable. */
            if (optionId == VP_OPTION_ID_DEBUG_SELECT) {
                VP_API_FUNC(None, VP_NULL, ("VpSetOption(%s):", VpGetString_VpOptionIdType(VP_OPTION_ID_DEBUG_SELECT)));
#ifdef VP_CC_KWRAP
                KWrapSetOption(VP_NULL, VP_NULL, VP_OPTION_ID_DEBUG_SELECT, value);
#endif
                vpDebugSelectMask = *(uint32 *)value;
                VP_API_EXIT(None, VP_NULL, "SetOption", VP_STATUS_SUCCESS);
                return VP_STATUS_SUCCESS;
            }
#endif

            status = VP_STATUS_INVALID_ARG;
        }
        VP_API_FUNC(VpDevCtxType, pDevCtxParam, ("VpSetOption(%s):", VpGetString_VpOptionIdType(optionId)));
    }

    if (status == VP_STATUS_SUCCESS) {
        status = VP_CALL_DEV_FUNC(SetOption, (pLineCtx, pDevCtxParam, optionId, value));
    }

#if (VP_CC_DEBUG_SELECT & VP_DBG_API_FUNC)
    if (singleLine) {
        VP_API_EXIT(VpLineCtxType, pLineCtx, "SetOption", status);
    } else {
        VP_API_EXIT(VpDevCtxType, pDevCtx, "SetOption", status);
    }
#endif

    return status;
} /* VpSetOption() */

/**
 * VpDeviceIoAccess()
 *  This function is used to access device IO pins of the VTD. See VP-API-II
 * documentation for more information about this function.
 *
 * Preconditions:
 *  Device/Line context should be created and initialized. For applicable
 * devices bootload should be performed before calling the function.
 *
 * Postconditions:
 *  Reads/Writes from device IO pins.
 */
VpStatusType
VpDeviceIoAccess(
    VpDevCtxType *pDevCtx,
    VpDeviceIoAccessDataType *pDeviceIoData)
{
    VpStatusType status;
    VP_API_ENTER(VpDevCtxType, pDevCtx, "DeviceIoAccess");

    /* Basic argument checking */
    if (pDevCtx == VP_NULL) {
        status = VP_STATUS_INVALID_ARG;
    } else {
        status = VP_CALL_DEV_FUNC(DeviceIoAccess, (pDevCtx, pDeviceIoData));
    }

    VP_API_EXIT(VpDevCtxType, pDevCtx, "DeviceIoAccess", status);
    return status;
} /* VpDeviceIoAccess() */

/**
 * VpDeviceIoAccessExt()
 *  This function is used to access device IO pins of the VTD. See VP-API-II
 * documentation for more information about this function.
 *
 * Preconditions:
 *  Device/Line context should be created and initialized. For applicable
 * devices bootload should be performed before calling the function.
 *
 * Postconditions:
 *  Reads/Writes from device IO pins.
 */
VpStatusType
VpDeviceIoAccessExt(
    VpDevCtxType *pDevCtx,
    VpDeviceIoAccessExtType *pDeviceIoAccess)
{
    VpStatusType status;
    VP_API_ENTER(VpDevCtxType, pDevCtx, "DeviceIoAccessExt");

    /* Basic argument checking */
    if ((pDevCtx == VP_NULL) || (pDeviceIoAccess == VP_NULL)) {
        status = VP_STATUS_INVALID_ARG;
    } else {
        status = VP_CALL_DEV_FUNC(DeviceIoAccessExt, (pDevCtx, pDeviceIoAccess));
    }

    VP_API_EXIT(VpDevCtxType, pDevCtx, "DeviceIoAccessExt", status);
    return status;
} /* VpDeviceIoAccessExt() */

/**
 * VpLineIoAccess()
 *  This function is used to access the IO pins of the VTD associated with a
 * particular line. See VP-API-II documentation for more information about
 * this function.
 *
 * Preconditions:
 *  Device/Line context should be created and initialized. For applicable
 * devices bootload should be performed before calling the function.
 *
 * Postconditions:
 *  Reads/Writes from line IO pins.
 */
VpStatusType
VpLineIoAccess(
    VpLineCtxType *pLineCtx,
    VpLineIoAccessType *pLineIoAccess,
    uint16 handle)
{
    VpStatusType status;
    VP_API_ENTER(VpLineCtxType, pLineCtx, "LineIoAccess");

    /* Basic argument checking */
    if (pLineCtx == VP_NULL) {
        status = VP_STATUS_INVALID_ARG;
    } else {
        VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
        status = VP_CALL_DEV_FUNC(LineIoAccess, (pLineCtx, pLineIoAccess, handle));
    }

    VP_API_EXIT(VpLineCtxType, pLineCtx, "LineIoAccess", status);
    return status;
} /* VpLineIoAccess() */

/**
 * VpVirtualISR()
 *  This function should be called from the interrupt context for CSLAC devices
 * upon a CSLAC device interrupt. See VP-API-II documentation for more
 * information about this function.
 *
 * Preconditions:
 *  Device/Line context should be created and initialized. For applicable
 * devices bootload should be performed before calling the function.
 *
 * Postconditions:
 *  Updates API internal variables to indicate an interrupt has occured.
 */
VpStatusType
VpVirtualISR(
    VpDevCtxType *pDevCtx)
{
    VpVirtualISRFuncPtrType VirtualISR;

    if (pDevCtx == VP_NULL) {
        return VP_STATUS_INVALID_ARG;
    }

    VirtualISR = pDevCtx->funPtrsToApiFuncs.VirtualISR;

    if (VirtualISR == VP_NULL) {
        return VP_STATUS_FUNC_NOT_SUPPORTED;
    } else {
        return VirtualISR(pDevCtx);
    }
} /* VpVirtualISR() */

/**
 * VpApiTick()
 *  This function should be called at regular intervals of time for CSLAC
 * devices. This function call is used to update timing related aspects of
 * VP-API. See VP-API-II documentation for more information.
 *
 * Preconditions:
 *  Device/Line context should be created and initialized.
 *
 * Postconditions:
 *  Updates necessary internal variables.
 */
VpStatusType
VpApiTick(
    VpDevCtxType *pDevCtx,
    bool *pEventStatus)
{
    VpApiTickFuncPtrType ApiTick;

    if (pDevCtx == VP_NULL) {
        return VP_STATUS_INVALID_ARG;
    }

    ApiTick = pDevCtx->funPtrsToApiFuncs.ApiTick;

    if (ApiTick == VP_NULL) {
        return VP_STATUS_FUNC_NOT_SUPPORTED;
    } else {
        return ApiTick(pDevCtx, pEventStatus);
    }
} /* VpApiTick() */

/**
 * VpLowLevelCmd()
 *  This function provides a by-pass mechanism for the VP-API. THE USE
 * OF THIS FUNCTION BY THE APPLICATION CODE IS STRONGLY DISCOURAGED. THIS
 * FUNCTION CALL BREAKS THE SYNCHRONIZATION BETWEEN THE VP-API AND THE
 * VTD. See VP-API-II documentation for more information about this function.
 *
 * Preconditions:
 *  Device/Line context should be created and initialized. For applicable
 * devices bootload should be performed before calling the function.
 *
 * Postconditions:
 *  Performs the indicated low-level command access.
 */
VpStatusType
VpLowLevelCmd(
    VpLineCtxType *pLineCtx,
    uint8 *pCmdData,
    uint8 len,
    uint16 handle)
{
    VpStatusType status;
    VP_API_ENTER(VpLineCtxType, pLineCtx, "LowLevelCmd");

    /* Basic argument checking */
    if (pLineCtx == VP_NULL) {
        status = VP_STATUS_INVALID_ARG;
    } else {
        VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
        status = VP_CALL_DEV_FUNC(LowLevelCmd, (pLineCtx, pCmdData, len, handle));
    }

    VP_API_EXIT(VpLineCtxType, pLineCtx, "LowLevelCmd", status);
    return status;
} /* VpLowLevelCmd() */

/**
 * VpLowLevelCmd16()
 *  This function provides a by-pass mechanism for the VP-API. THE USE
 * OF THIS FUNCTION BY THE APPLICATION CODE IS STRONGLY DISCOURAGED. THIS
 * FUNCTION CALL BREAKS THE SYNCHRONIZATION BETWEEN THE VP-API AND THE
 * VTD. See VP-API-II documentation for more information about this function.
 *
 * Preconditions:
 *  Device/Line context should be created and initialized. For applicable
 * devices bootload should be performed before calling the function.
 *
 * Postconditions:
 *  Performs the indicated low-level command access.
 */
VpStatusType
VpLowLevelCmd16(
    VpLineCtxType *pLineCtx,
    VpLowLevelCmdType cmdType,
    uint16 *writeWords,
    uint8 numWriteWords,
    uint8 numReadWords,
    uint16 handle)
{
    VpStatusType status;
    VP_API_ENTER(VpLineCtxType, pLineCtx, "LowLevelCmd16");

    /* Basic argument checking */
    if (pLineCtx == VP_NULL) {
        status = VP_STATUS_INVALID_ARG;
    } else {
        VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
        status = VP_CALL_DEV_FUNC(LowLevelCmd16, (pLineCtx, cmdType, writeWords, numWriteWords, numReadWords, handle));
    }

    VP_API_EXIT(VpLineCtxType, pLineCtx, "LowLevelCmd16", status);
    return status;
} /* VpLowLevelCmd16() */


/******************************************************************************
 *                        STATUS AND QUERY FUNCTIONS                          *
 ******************************************************************************/
/**
 * VpGetEvent()
 *  This function is used to get a event from a given device. See VP-API-II
 * documentation for more information about this function.
 *
 * Preconditions:
 *  This function can be called upon detecting an interrupt from interrupt
 * context or this function could be called to poll events. This function
 * assumes passed device context has been initialized.
 *
 * Postconditions:
 *  Returns TRUE is there is a new event. Information about this event is
 * filled in to the event pointer (if not zero). If either of its arguments is
 * zero this function returns FALSE.
 *
 */
bool
VpGetEvent(
    VpDevCtxType *pDevCtx,
    VpEventType *pEvent)
{
    bool gotEvent;
    VpGetEventFuncPtrType GetEvent;
    VP_API_ENTER(VpDevCtxType, pDevCtx, "GetEvent");

    /* Basic argument checking */
    if (pEvent == VP_NULL) {
        VP_ERROR(None, VP_NULL, ("pEvent = VP_NULL in VpGetEvent()"));
        return FALSE;
    } else if (pDevCtx == VP_NULL) {
        pEvent->status = VP_STATUS_INVALID_ARG;
        gotEvent = FALSE;
    } else {
        GetEvent = pDevCtx->funPtrsToApiFuncs.GetEvent;
        if (GetEvent == VP_NULL) {
            pEvent->status = VP_STATUS_FUNC_NOT_SUPPORTED;
            gotEvent = FALSE;
        } else {
            gotEvent = GetEvent(pDevCtx, pEvent);
        }
    }

#if (VP_CC_DEBUG_SELECT & VP_DBG_API_FUNC)
    if (pEvent->status == VP_STATUS_SUCCESS) {
        if (gotEvent) {
            if (pEvent->pLineCtx != VP_NULL) {
                VP_API_FUNC(VpLineCtxType, pEvent->pLineCtx, ("Vp%s() = %s", "GetEvent", "TRUE"));
            } else {
                VP_API_FUNC(VpDevCtxType, pEvent->pDevCtx, ("Vp%s() = %s", "GetEvent", "TRUE"));
            }
        } else {
            VP_API_FUNC(VpDevCtxType, pDevCtx, ("Vp%s() = %s", "GetEvent", "FALSE"));
        }
    } else {
        VP_API_FUNC(VpDevCtxType, pDevCtx, ("Vp%s(): status = %s", "GetEvent", VpGetString_VpStatusType(pEvent->status)));
    }
#endif

#if (VP_CC_DEBUG_SELECT & VP_DBG_EVENT)
    if ((pEvent->status == VP_STATUS_SUCCESS) && gotEvent) {
        VpDisplayEvent(pEvent);
    }
#endif

    return gotEvent;
} /* VpGetEvent() */

/**
 * VpGetLineStatus()
 *  This function is used to obtain status of a given line with respect to a
 * certain type of line condition. See VP-API-II documentation for more
 * information about this function.
 *
 * Preconditions:
 *  Device/Line context should be created and initialized. For applicable
 * devices bootload should be performed before calling the function.
 *
 * Postconditions:
 * Returns status information about the line condition that was asked for.
 */
VpStatusType
VpGetLineStatus(
    VpLineCtxType *pLineCtx,
    VpInputType input,
    bool *pStatus)
{
    VpStatusType status;
    //VP_API_ENTER(VpLineCtxType, pLineCtx, "GetLineStatus");

    /* Basic argument checking */
    if (pLineCtx == VP_NULL) {
        status = VP_STATUS_INVALID_ARG;
    } else {
        VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
        status = VP_CALL_DEV_FUNC(GetLineStatus, (pLineCtx, input, pStatus));
    }

    //VP_API_EXIT(VpLineCtxType, pLineCtx, "GetLineStatus", status);
    return status;
} /* VpGetLineStatus() */

/**
 * VpGetDeviceStatus()
 *  This function is used to obtain status of all lines with respect to a
 * certain type of line condition. See VP-API-II documentation for more
 * information about this function.
 *
 * Preconditions:
 *  Device/Line context should be created and initialized. For applicable
 * devices bootload should be performed before calling the function.
 *
 * Postconditions:
 *  Returns status information about the line condition that was asked for.
 */
VpStatusType
VpGetDeviceStatus(
    VpDevCtxType *pDevCtx,
    VpInputType input,
    uint32 *pDeviceStatus)
{
    VpStatusType status;
    VP_API_ENTER(VpDevCtxType, pDevCtx, "GetDeviceStatus");

    /* Basic argument checking */
    if ((pDevCtx == VP_NULL) || (pDeviceStatus == VP_NULL)) {
        status = VP_STATUS_INVALID_ARG;
    } else {
        status = VP_CALL_DEV_FUNC(GetDeviceStatus, (pDevCtx, input, pDeviceStatus));
    }

    VP_API_EXIT(VpDevCtxType, pDevCtx, "GetDeviceStatus", status);
    return status;
} /* VpGetDeviceStatus() */

/**
 * VpGetDeviceStatusExt()
 *  This function is used to obtain status of all lines with respect to a
 * certain type of line condition. See VP-API-II documentation for more
 * information about this function.
 *
 * Preconditions:
 *  Device/Line context should be created and initialized. For applicable
 * devices bootload should be performed before calling the function.
 *
 * Postconditions:
 *  Returns status information about the line condition that was asked for.
 */
VpStatusType
VpGetDeviceStatusExt(
    VpDevCtxType *pDevCtx,
    VpDeviceStatusType *pDeviceStatus)
{
    VpStatusType status;
    VP_API_ENTER(VpDevCtxType, pDevCtx, "GetDeviceStatusExt");

    /* Basic argument checking */
    if ((pDevCtx == VP_NULL) || (pDeviceStatus == VP_NULL)) {
        status = VP_STATUS_INVALID_ARG;
    } else {
        status = VP_CALL_DEV_FUNC(GetDeviceStatusExt, (pDevCtx, pDeviceStatus));
    }

    VP_API_EXIT(VpDevCtxType, pDevCtx, "GetDeviceStatusExt", status);
    return status;
} /* VpGetDeviceStatusExt() */

/**
 * VpGetLoopCond()
 *  This function is used to obtain the telephone loop condition for a given
 * line. See VP-API-II documentation for more information about this function.
 *
 * Preconditions:
 *  Device/Line context should be created and initialized. For applicable
 * devices bootload should be performed before calling the function.
 *
 * Postconditions:
 *  This function starts the necessary actions to obtain the loop condition.
 */
VpStatusType
VpGetLoopCond(
    VpLineCtxType *pLineCtx,
    uint16 handle)
{
    VpStatusType status;
    VP_API_ENTER(VpLineCtxType, pLineCtx, "GetLoopCond");

    /* Basic argument checking */
    if (pLineCtx == VP_NULL) {
        status = VP_STATUS_INVALID_ARG;
    } else {
        VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
        status = VP_CALL_DEV_FUNC(GetLoopCond, (pLineCtx, handle));
    }

    VP_API_EXIT(VpLineCtxType, pLineCtx, "GetLoopCond", status);
    return status;
} /* VpGetLoopCond() */

/**
 * VpGetOption()
 *  This function is used to read an option. See VP-API-II documentation for
 * more information about this function.
 *
 * Preconditions:
 *  Device/Line context should be created and initialized. For applicable
 * devices bootload should be performed before calling the function.
 *
 * Postconditions:
 *  Starts the necessary action to be able to read the requested option.
 */
VpStatusType
VpGetOption(
    VpLineCtxType *pLineCtx,
    VpDevCtxType *pDevCtxParam,
    VpOptionIdType option,
    uint16 handle)
{
    VpDevCtxType *pDevCtx;
    VpStatusType status = VP_STATUS_SUCCESS;
    bool lineSpecific = (pLineCtx != VP_NULL);

    /* Basic argument checking */
    if (lineSpecific) {
        VP_API_ENTER(VpLineCtxType, pLineCtx, "GetOption");
        if (pDevCtxParam != VP_NULL) {
            status = VP_STATUS_INVALID_ARG;
        }
        pDevCtx = pLineCtx->pDevCtx;
    } else {
        pDevCtx = pDevCtxParam;
        VP_API_ENTER(VpDevCtxType, pDevCtx, "GetOption");
        if (pDevCtx == VP_NULL) {
            status = VP_STATUS_INVALID_ARG;
        }
    }

    if (status == VP_STATUS_SUCCESS) {
        status = VP_CALL_DEV_FUNC(GetOption, (pLineCtx, pDevCtxParam, option, handle));
    }

#if (VP_CC_DEBUG_SELECT & VP_DBG_API_FUNC)
    if (lineSpecific) {
        VP_API_EXIT(VpLineCtxType, pLineCtx, "GetOption", status);
    } else {
        VP_API_EXIT(VpDevCtxType, pDevCtx, "GetOption", status);
    }
#endif

    return status;
} /* VpGetOption() */

VpStatusType
VpGetLineState(
    VpLineCtxType *pLineCtx,
    VpLineStateType *pCurrentState)
{
    VpDevCtxType *pDevCtx;
    VpStatusType status = VP_STATUS_SUCCESS;
    VP_API_ENTER(VpLineCtxType, pLineCtx, "GetLineState");

    /* Basic argument checking */
    if (pLineCtx == VP_NULL) {
        VP_API_EXIT(VpLineCtxType, pLineCtx, "GetLineState", VP_STATUS_INVALID_ARG);
        return VP_STATUS_INVALID_ARG;
    }

    pDevCtx = pLineCtx->pDevCtx;
    switch (pDevCtx->deviceType) {

#ifdef VP_CC_VCP_SERIES
        case VP_DEV_VCP_SERIES:
#endif
#ifdef VP_CC_VCP2_SERIES
        case VP_DEV_VCP2_SERIES:
#endif
#if defined(VP_CC_VCP_SERIES) || defined(VP_CC_VCP2_SERIES)
            status = VP_CALL_DEV_FUNC(GetLineState, (pLineCtx, pCurrentState));
            break;
#endif

#if defined (VP_CC_790_SERIES)
        case VP_DEV_790_SERIES: {
            Vp790LineObjectType *pLineObj = pLineCtx->pLineObj;
            *pCurrentState = pLineObj->lineState.usrCurrent;
            break;
        }
#endif

#if defined (VP_CC_880_SERIES)
        case VP_DEV_880_SERIES: {
            Vp880LineObjectType *pLineObj = pLineCtx->pLineObj;
            if (pLineObj->status & VP880_LINE_IN_CAL) {
                *pCurrentState = pLineObj->calLineData.usrState;
            } else {
                *pCurrentState = pLineObj->lineState.usrCurrent;
            }
            break;
        }
#endif

#if defined (VP_CC_890_SERIES)
        case VP_DEV_890_SERIES: {
            Vp890LineObjectType *pLineObj = pLineCtx->pLineObj;
            *pCurrentState = pLineObj->lineState.usrCurrent;
            break;
        }
#endif

#if defined (VP_CC_580_SERIES)
        case VP_DEV_580_SERIES: {
            Vp580LineObjectType *pLineObj = pLineCtx->pLineObj;
            *pCurrentState = pLineObj->lineState.usrCurrent;
            break;
        }
#endif

#if defined (VP_CC_792_SERIES)
        case VP_DEV_792_SERIES: {
            Vp792LineObjectType *pLineObj = pLineCtx->pLineObj;
            *pCurrentState = pLineObj->currentState;
            break;
        }
#endif

#if defined (VP_CC_KWRAP)
        case VP_DEV_KWRAP: {
            VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
            status = VP_CALL_DEV_FUNC(GetLineState, (pLineCtx, pCurrentState));
            break;
        }
#endif

        default:
            status = VP_STATUS_FUNC_NOT_SUPPORTED;

    }

    VP_API_EXIT(VpLineCtxType, pLineCtx, "GetLineState", status);
    return status;
}

/* Vincent: This API is writting for getting internal line state. 
 * It's due to RING-OFF state doesn'f show in usrCurrent. 
 */

VpStatusType
VpGetLineStateINT(
    VpLineCtxType *pLineCtx,
    VpLineStateType *pCurrentState)
{
    VpDevCtxType *pDevCtx;
    VpStatusType status = VP_STATUS_SUCCESS;
    VP_API_ENTER(VpLineCtxType, pLineCtx, "GetLineStateINT");

    /* Basic argument checking */
    if (pLineCtx == VP_NULL) {
        VP_API_EXIT(VpLineCtxType, pLineCtx, "GetLineStateINT", VP_STATUS_INVALID_ARG);
        return VP_STATUS_INVALID_ARG;
    }

    pDevCtx = pLineCtx->pDevCtx;
    switch (pDevCtx->deviceType) {

#if defined (VP_CC_880_SERIES)
        case VP_DEV_880_SERIES: {
            Vp880LineObjectType *pLineObj = pLineCtx->pLineObj;
            if (pLineObj->status & VP880_LINE_IN_CAL) {
                *pCurrentState = pLineObj->calLineData.usrState;
            } else {
                //*pCurrentState = pLineObj->lineState.usrCurrent;
            	*pCurrentState = pLineObj->lineState.currentState;
            }
            break;
        }
#endif

#if defined (VP_CC_890_SERIES)
        case VP_DEV_890_SERIES: {
            Vp890LineObjectType *pLineObj = pLineCtx->pLineObj;
            //*pCurrentState = pLineObj->lineState.usrCurrent;
            *pCurrentState = pLineObj->lineState.currentState;
            break;
        }
#endif

        default:
            status = VP_STATUS_FUNC_NOT_SUPPORTED;

    }

    VP_API_EXIT(VpLineCtxType, pLineCtx, "GetLineStateINT", status);
    return status;
}

/**
 * VpFlushEvents()
 *  This function is used to flush all outstanding events from the VTD's event
 * queue. See VP-API-II documentation for more information about this function.
 *
 * Preconditions:
 *  Device/Line context should be created and initialized. For applicable
 * devices bootload should be performed before calling the function.
 *
 * Postconditions:
 *  Clears all outstanding events from event queue.
 */
VpStatusType
VpFlushEvents(
    VpDevCtxType *pDevCtx)
{
    VpStatusType status;
    VP_API_ENTER(VpDevCtxType, pDevCtx, "FlushEvents");

    /* Basic argument checking */
    if (pDevCtx == VP_NULL) {
        status = VP_STATUS_INVALID_ARG;
    } else {
        status = VP_CALL_DEV_FUNC(FlushEvents, (pDevCtx));
    }

    VP_API_EXIT(VpDevCtxType, pDevCtx, "FlushEvents", status);
    return status;
} /* VpFlushEvents() */

/**
 * VpGetResults()
 *  This function retrives the result based on a previous operation. For more
 * information see VP-API-II user guide.
 *
 * Preconditions:
 *  This function can be called upon detecting an event for which there could
 * be associated data. The results pointer must provide enough storage space
 * the type of result that is anticipated.
 *
 * Postconditions:
 *  It fills in the results based on the event to location provided by results
 * pointer. This function returns an error if either of its arguments is zero.
 */
VpStatusType
VpGetResults(
    VpEventType *pEvent,
    void *pResults)
{
    VpDevCtxType *pDevCtx;
    VpStatusType status;

    if (pEvent == VP_NULL) {
        return VP_STATUS_INVALID_ARG;
    }

#if (VP_CC_DEBUG_SELECT & VP_DBG_API_FUNC)
    if (pEvent == VP_NULL) {
        VP_API_ENTER(None, VP_NULL, "GetResults");
        VP_API_EXIT(None, VP_NULL, "GetResults", VP_STATUS_INVALID_ARG);
        return VP_STATUS_INVALID_ARG;
    } else if (pEvent->pLineCtx == VP_NULL) {
        VP_API_ENTER(VpDevCtxType, pEvent->pDevCtx, "GetResults");
    } else {
        VP_API_ENTER(VpLineCtxType, pEvent->pLineCtx, "GetResults");
    }
#endif

    pDevCtx = pEvent->pDevCtx;
    status = VP_CALL_DEV_FUNC(GetResults, (pEvent, pResults));

#if (VP_CC_DEBUG_SELECT & VP_DBG_API_FUNC)
    if (pEvent->pLineCtx != VP_NULL) {
        VP_API_EXIT(VpLineCtxType, pEvent->pLineCtx, "GetResults", status);
    } else {
        VP_API_EXIT(VpDevCtxType, pEvent->pDevCtx, "GetResults", status);
    }
#endif

#if (VP_CC_DEBUG_SELECT & VP_DBG_EVENT)
    if ((pResults != NULL) && (status == VP_STATUS_SUCCESS)) {
        VpDisplayResults(pEvent, pResults);
    }
#endif

    return status;
} /* VpGetResults() */

/**
 * VpClearResults()
 *  This function is used to clear results associated with an event.
 *
 * Preconditions:
 *  Device/Line context should be created and initialized. For applicable
 * devices bootload should be performed before calling the function.
 *
 * Postconditions:
 *  Clears results entry (only one at the top of the results queue) that is
 * outstanding and waiting to be read.
 */
VpStatusType
VpClearResults(
    VpDevCtxType *pDevCtx)
{
    VpStatusType status;
    VP_API_ENTER(VpDevCtxType, pDevCtx, "ClearResults");

    /* Basic argument checking */
    if (pDevCtx == VP_NULL) {
        status = VP_STATUS_INVALID_ARG;
    } else {
        status = VP_CALL_DEV_FUNC(ClearResults, (pDevCtx));
    }

    VP_API_EXIT(VpDevCtxType, pDevCtx, "ClearResults", status);
    return status;
} /* VpClearResults() */


/**
 * VpGetRelayState()
 *  This function returns the value of the lines relay state.
 *
 * Preconditions:
 *  Device/Line context should be created and initialized. For applicable
 * devices bootload should be performed before calling the function.
 *
 * Postconditions:
 *  The indicated relay state is set for the given line.
 */
VpStatusType
VpGetRelayState(
    VpLineCtxType *pLineCtx,
    VpRelayControlType *pRstate)
{
    VpStatusType status;
    VP_API_ENTER(VpLineCtxType, pLineCtx, "GetRelayState");

    /* Basic argument checking */
    if (pLineCtx == VP_NULL) {
        status = VP_STATUS_INVALID_ARG;
    } else {
        VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
        status = VP_CALL_DEV_FUNC(GetRelayState, (pLineCtx, pRstate));
    }

    VP_API_EXIT(VpLineCtxType, pLineCtx, "GetRelayState", status);
    return status;
} /* VpSetRelayState() */


/**
 * VpRegisterDump()
 *  This function is used to dump the content of all device registers.
 *
 * Preconditions:
 *  Device context should be created.
 *
 * Postconditions:
 *  Debug output of "all" registers.
 */
VpStatusType
VpRegisterDump(
    VpDevCtxType *pDevCtx)
{
    VpStatusType status;
    VP_API_ENTER(VpDevCtxType, pDevCtx, "RegisterDump");

    /* Basic argument checking */
    if (pDevCtx == VP_NULL) {
        status = VP_STATUS_INVALID_ARG;
    } else {
        status = VP_CALL_DEV_FUNC(RegisterDump, (pDevCtx));
    }

    VP_API_EXIT(VpDevCtxType, pDevCtx, "RegisterDump", status);
    return status;
} /* VpRegisterDump() */

/**
 * VpRegisterReadWrite()
 *  This function is used to read/write the content of specific device registers.
 *
 * Preconditions:
 *  Device context should be created.
 *
 * Postconditions:
 *  Debug read/write of "specific" registers.
 */
VpStatusType
VpRegisterReadWrite(
	VpLineCtxType   *pLineCtx,
	uint32			reg,
	uint8			*len,
	uint8 			*regdata
)
{
    VpStatusType status;

    VP_API_ENTER(VpLineCtxType, pLineCtx, "RegisterReadWrite");
    /* Basic argument checking */
    if (pLineCtx == VP_NULL) {
        status = VP_STATUS_INVALID_ARG;
    } else {
        VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
        status = VP_CALL_DEV_FUNC(RegisterReadWrite, (pLineCtx, reg, len, regdata));
    }

    VP_API_EXIT(VpLineCtxType, pLineCtx, "RegisterReadWrite", status);
    return status;
} /* VpRegisterReadWrite() */

/**
 * VpDtmfDigitDetected()
 *  This function is used to set a value in the API-II indicating that a DTMF
 * digit was detected in an external application/process.
 *
 * Preconditions:
 *  Device/Line context should be created and initialized. For applicable
 * devices bootload should be performed before calling the function.
 *
 * Postconditions:
 *  A value in the API-II is set which indicates the digit detected. The most
 * recent value is stored.
 */
VpStatusType
VpDtmfDigitDetected(
    VpLineCtxType *pLineCtx,
    VpDigitType digit,
    VpDigitSenseType sense)
{
    VpStatusType status;
    VP_API_ENTER(VpLineCtxType, pLineCtx, "DtmfDigitDetected");

    /* Basic argument checking */
    if ((pLineCtx == VP_NULL) || (VpIsDigit(digit) == FALSE)) {
        status = VP_STATUS_INVALID_ARG;
    } else {
        VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
        status = VP_CALL_DEV_FUNC(DtmfDigitDetected, (pLineCtx, digit, sense));
    }

    VP_API_EXIT(VpLineCtxType, pLineCtx, "DtmfDigitDetected", status);
    return status;
}

/******************************************************************************
 *                            TEST FUNCTIONS                                  *
 ******************************************************************************/
/**
 * VpTestLine()
 *  This function performs the requested test. For more information see
 * VP-API-II user guide.
 *
 * Preconditions:
 *  Device/Line context should be created and initialized. For applicable
 * devices bootload should be performed before calling the function.
 *
 * Postconditions:
 *  Starts/Stops requested test actions.
 */
VpStatusType
VpTestLine(
    VpLineCtxType *pLineCtx,
    VpTestIdType test,
    const void *pArgs,
    uint16 handle)
{
    VpStatusType status;
    VP_API_ENTER(VpLineCtxType, pLineCtx, "TestLine");

    /* Basic argument checking */
    if (pLineCtx == VP_NULL) {
        status = VP_STATUS_INVALID_ARG;
    } else {
        VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
        status = VP_CALL_DEV_FUNC(TestLine, (pLineCtx, test, pArgs, handle));
    }

    VP_API_EXIT(VpLineCtxType, pLineCtx, "TestLine", status);
    return status;
} /* VpTestLine() */

/**
 * VpTestLineCallback()
 * This function is normally called by an outside application after collecting
 * and processing PCM data requested by the previous PcmCollect routine. If
 * the API is operating in EZ mode this function is actually called from within
 * the API by the VpEzPcmCallback() function.
 *
 * The results structure pointed to by the pResults argument are copied into
 * the TestHeap for later use.
 *
 * Parameters:
 *  pLineCtx    - pointer to the line context
 *  pResults    - pointer to results from the pcmCollect system service layer
 */
VpStatusType
VpTestLineCallback(
    VpLineCtxType *pLineCtx,
    VpPcmOperationResultsType *pResults)
{
    VpStatusType status;
    VP_API_ENTER(VpLineCtxType, pLineCtx, "TestLineCallback");

    /* Basic argument checking */
    if (pLineCtx == VP_NULL) {
        status = VP_STATUS_INVALID_ARG;
    } else {
        VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
        status = VP_CALL_DEV_FUNC(TestLineCallback, (pLineCtx, pResults));
    }

    VP_API_EXIT(VpLineCtxType, pLineCtx, "TestLineCallback", status);
    return status;
} /* VpTestLineCallback() */

/**
 * VpCodeCheckSum()
 *  This function performs a checksum calculation over the loaded code (for
 * program and data memory). For more information see VP-API-II user guide.
 *
 * Preconditions:
 *  Device/Line context should be created and initialized. For applicable
 * devices bootload should be performed before calling the function.
 *
 * Postconditions:
 *  Initiates a checksum verification action.
 */
VpStatusType
VpCodeCheckSum(
    VpDevCtxType *pDevCtx,
    uint16 handle)
{
    VpStatusType status;
    VP_API_ENTER(VpDevCtxType, pDevCtx, "CodeCheckSum");

    /* Basic argument checking */
    if (pDevCtx == VP_NULL) {
        status = VP_STATUS_INVALID_ARG;
    } else {
        status = VP_CALL_DEV_FUNC(CodeCheckSum, (pDevCtx, handle));
    }

    VP_API_EXIT(VpDevCtxType, pDevCtx, "CodeCheckSum", status);
    return status;
} /* VpCodeCheckSum() */

/**
 * VpSelfTest()
 *  This function performs a self test on a given line or for all lines in the
 * device. For more information see VP-API-II user guide.
 *
 * Preconditions:
 *  Device/Line context should be created and initialized. For applicable
 * devices bootload should be performed before calling the function.
 *
 * Postconditions:
 *  Initiates self test.
 */
VpStatusType
VpSelfTest(
    VpLineCtxType *pLineCtx)
{
    VpStatusType status;
    VP_API_ENTER(VpLineCtxType, pLineCtx, "SelfTest");

    /* Basic argument checking */
    if (pLineCtx == VP_NULL) {
        status = VP_STATUS_INVALID_ARG;
    } else {
        VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
        status = VP_CALL_DEV_FUNC(SelfTest, (pLineCtx));
    }

    VP_API_EXIT(VpLineCtxType, pLineCtx, "SelfTest", status);
    return status;
} /* VpSelfTest() */

/**
 * VpFillTestBuf()
 *  This function is used to load test vector data in to the device internal
 * memory. For more information see VP-API-II user guide.
 *
 * Preconditions:
 *  Device/Line context should be created and initialized. For applicable
 * devices bootload should be performed before calling the function.
 *
 * Postconditions:
 *  Loads the given test data in to the device.
 */
VpStatusType
VpFillTestBuf(
    VpLineCtxType *pLineCtx,
    uint16 length,
    VpVectorPtrType pData)
{
    VpStatusType status;
    VP_API_ENTER(VpLineCtxType, pLineCtx, "FillTestBuf");

    /* Basic argument checking */
    if (pLineCtx == VP_NULL) {
        status = VP_STATUS_INVALID_ARG;
    } else {
        VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
        status = VP_CALL_DEV_FUNC(FillTestBuf, (pLineCtx, length, pData));
    }

    VP_API_EXIT(VpLineCtxType, pLineCtx, "FillTestBuf", status);
    return status;

} /* VpFillTestBuf() */

/**
 * VpReadTestBuf()
 *
 *
 * Preconditions:
 *  Device/Line context should be created and initialized. For applicable
 * devices bootload should be performed before calling the function.
 *
 * Postconditions:
 *  Loads test data from the device into the given buffer.
 */
VpStatusType
VpReadTestBuf(
    VpLineCtxType *pLineCtx,
    uint16 length,
    VpVectorPtrType pData)
{
    VpStatusType status = VP_STATUS_SUCCESS;

    VP_API_ENTER(VpLineCtxType, pLineCtx, "ReadTestBuf");
    /* Basic argument checking */
    if (pLineCtx == VP_NULL) {
        status = VP_STATUS_INVALID_ARG;
    } else {
        VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
        status = VP_CALL_DEV_FUNC(ReadTestBuf, (pLineCtx, length, pData));
    }
    VP_API_EXIT(VpLineCtxType, pLineCtx, "ReadTestBuf", status);
    return status;

} /* VpReadTestBuf() */


/**
 * VpMapLineId()
 *  This function assignes a lineId to a line object. It is used by the
 * application when mapping a device and channel Id to a specific line
 * context is not convenient (or easy). The value of lineId is reported
 * in the event structure.
 *
 * Preconditions:
 *  The line context and device object pointers must be non zero. Device
 * Object must be created before calling this function.
 *
 * Postconditions:
 *  The device context and device object are linked and the device context
 * function pointers are initialized.
 */
VpStatusType
VpMapLineId(
    VpLineCtxType *pLineCtx,   /**< Line Context to map to lineId */
    VpLineIdType lineId)       /**< Value assigned as line Id */
{
    VpDevCtxType *pDevCtx;
    VpDeviceIdType deviceId;
    void *pLineObj, *pDevObj;
    VpDeviceType devType;

    VpStatusType status = VP_STATUS_SUCCESS;
    VP_API_ENTER(VpLineCtxType, pLineCtx, "MapLineId");

    /* Basic argument checking */
    if (pLineCtx == VP_NULL) {
        VP_API_EXIT(VpLineCtxType, pLineCtx, "MapLineId", VP_STATUS_INVALID_ARG);
        return VP_STATUS_INVALID_ARG;
    }

    pDevCtx = pLineCtx->pDevCtx;
    pLineObj = pLineCtx->pLineObj;

    devType = pDevCtx->deviceType;
    pDevObj = pDevCtx->pDevObj;

    switch (devType) {
#if defined (VP_CC_VCP_SERIES)
        case VP_DEV_VCP_SERIES:
            deviceId = ((VpVcpDeviceObjectType *)pDevObj)->deviceId;
            VpSysEnterCritical(deviceId, VP_CODE_CRITICAL_SEC);
            ((VpVcpLineObjectType *)pLineObj)->lineId = lineId;
            VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
            break;
#endif

#if defined (VP_CC_VCP2_SERIES)
        case VP_DEV_VCP2_SERIES:
            deviceId = ((VpVcp2DeviceObjectType *)pDevObj)->deviceId;
            VpSysEnterCritical(deviceId, VP_CODE_CRITICAL_SEC);
            ((VpVcp2LineObjectType *)pLineObj)->lineId = lineId;
            VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
            break;
#endif

#if defined (VP_CC_880_SERIES)
        case VP_DEV_880_SERIES:
            deviceId = ((Vp880DeviceObjectType *)pDevObj)->deviceId;
            VpSysEnterCritical(deviceId, VP_CODE_CRITICAL_SEC);
            ((Vp880LineObjectType *)pLineObj)->lineId = lineId;
            VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
            break;
#endif

#if defined (VP_CC_890_SERIES)
        case VP_DEV_890_SERIES:
            deviceId = ((Vp890DeviceObjectType *)pDevObj)->deviceId;
            VpSysEnterCritical(deviceId, VP_CODE_CRITICAL_SEC);
            ((Vp890LineObjectType *)pLineObj)->lineId = lineId;
            VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
            break;
#endif


#if defined (VP_CC_790_SERIES)
        case VP_DEV_790_SERIES:
            deviceId = ((Vp790DeviceObjectType *)pDevObj)->deviceId;
            VpSysEnterCritical(deviceId, VP_CODE_CRITICAL_SEC);
            ((Vp790LineObjectType *)pLineObj)->lineId = lineId;
            VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
            break;
#endif

#if defined (VP_CC_792_SERIES)
        case VP_DEV_792_SERIES:
            deviceId = ((Vp792DeviceObjectType *)pDevObj)->deviceId;
            VpSysEnterCritical(deviceId, VP_CODE_CRITICAL_SEC);
            ((Vp792LineObjectType *)pLineObj)->lineId = lineId;
            VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
            break;
#endif

#if defined (VP_CC_580_SERIES)
        case VP_DEV_580_SERIES:
            deviceId = ((Vp580DeviceObjectType *)pDevObj)->deviceId;
            VpSysEnterCritical(deviceId, VP_CODE_CRITICAL_SEC);
            ((Vp580LineObjectType *)pLineObj)->lineId = lineId;
            VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
            break;
#endif

#if defined (VP_CC_KWRAP)
        case VP_DEV_KWRAP:
            status = VP_STATUS_FUNC_NOT_SUPPORTED;
            break;
#endif

        default:
            deviceId = 0;
            status = VP_STATUS_ERR_VTD_CODE;
    }

    VP_API_EXIT(VpLineCtxType, pLineCtx, "MapLineId", status);
    return status;
} /* VpMapLineId() */

/**
 * VpMapSlacId()
 *  This function assignes a slacId to a device object. It is used by the
 * application when mapping a SLAC ID to a specific device context using a
 * common deviceId. The value of slacId is reported in the event structure.
 *
 * Preconditions:
 *  The device context and device object pointers must be non zero. Device
 *  Object must be created before calling this function.
 *
 * Postconditions:
 *  The device context and device object are linked and the device context
 * function pointers are initialized.
 */
VpStatusType
VpMapSlacId(
    VpDevCtxType *pDevCtx,   /**< Device Context to map to lineId */
    uint8 slacId)            /**< Value assigned as slac Id */
{
    VpStatusType status = VP_STATUS_SUCCESS;
    VP_API_ENTER(VpDevCtxType, pDevCtx, "MapSlacId");

    /* Basic argument checking */
    if (pDevCtx == VP_NULL) {
        VP_API_EXIT(None, VP_NULL, "MapSlacId", VP_STATUS_INVALID_ARG);
        return VP_STATUS_INVALID_ARG;
    }

    switch (pDevCtx->deviceType) {

#if defined (VP_CC_792_SERIES)
        case VP_DEV_792_SERIES: {
            Vp792DeviceObjectType *pDevObj = pDevCtx->pDevObj;
            VpDeviceIdType deviceId = pDevObj->deviceId;
            if (VP792_MAX_SLAC_ID < slacId) {
                status = VP_STATUS_INVALID_ARG;
                break;
            }
            VpSysEnterCritical(deviceId, VP_CODE_CRITICAL_SEC);
            pDevObj->slacId = slacId;
            VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
            break;
        }
#endif

        default:
            status = VP_STATUS_FUNC_NOT_SUPPORTED;
    }

    VP_API_EXIT(VpDevCtxType, pDevCtx, "MapSlacId", status);
    return status;
}

/**
 * VpQuery()
 *  This function queries a SLAC register for the specified line.
 * For more information see VP-API-II user guide.
 *
 * Preconditions:
 *  Device/Line context should be created and initialized. For applicable
 * devices bootload should be performed before calling the function.
 *
 * Postconditions:
 *  Initiates SLAC register read operation.
 */
VpStatusType
VpQuery(
    VpLineCtxType *pLineCtx,
    VpQueryIdType queryId,
    uint16 handle)
{
    VpStatusType status;
    VP_API_ENTER(VpLineCtxType, pLineCtx, "Query");

    /* Basic argument checking */
    if (pLineCtx == VP_NULL) {
        status = VP_STATUS_INVALID_ARG;
    } else {
        VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
        status = VP_CALL_DEV_FUNC(Query, (pLineCtx, queryId, handle));
    }

    VP_API_EXIT(VpLineCtxType, pLineCtx, "Query", status);
    return status;
} /* VpSelfTest() */

/**
 * VpGetProfileField()
 *  This function extracts a field from a profile.  For more information see
 * the VP-API-II Reference Guide.
 *
 * Preconditions:
 *  pProfile must point to a valid profile struct (not VP_NULL, and not a
 * profile index).
 *  The 'device' and 'fieldId' members of the pField struct must be initialized.
 *
 * Postconditions:
 *  The 'data' member of the pField struct contains the value of the requested
 * field.
 */
VpStatusType
VpGetProfileField(
    VpDevCtxType *pDevCtx,
    VpProfilePtrType pProfile,
    VpProfileFieldType *pField)
{
    VpStatusType status;
    VP_API_ENTER(VpDevCtxType, pDevCtx, "GetProfileField");

    /* Basic argument checking */
    if (pDevCtx == VP_NULL) {
        VP_ERROR(VpDevCtxType, pDevCtx, ("VpGetProfileField(): %s.", "Can't accept VP_NULL for profile pointer."));
        status = VP_STATUS_INVALID_ARG;
    } else if (VpGetProfileIndex(pProfile) != -1) {
        VP_ERROR(VpDevCtxType, pDevCtx, ("VpGetProfileField(): %s.", "Can't accept an index for profile pointer."));
        status = VP_STATUS_INVALID_ARG;
    } else {
        status = VP_CALL_DEV_FUNC(GetProfileField, (pDevCtx, pProfile, pField));
    }

    VP_API_EXIT(None, VP_NULL, "GetProfileField", status);
    return status;
} /* VpGetProfileField() */

/**
 * VpSetProfileField()
 *  This function sets a field in a profile.  For more information see
 * the VP-API-II Reference Guide.
 *
 * Preconditions:
 *  pProfile must point to a valid profile struct (not VP_NULL, and not a
 * profile index).
 *  The 'device' and 'fieldId' members of the pField struct must be initialized.
 *  The 'data' member of the pField struct must be initialized to the desired
 * value.
 *
 * Postconditions:
 *  The pProfile struct is updated, assigning the specified value to the
 * specified field.
 */
VpStatusType
VpSetProfileField(
    VpDevCtxType *pDevCtx,
    VpProfilePtrType pProfile,
    VpProfileFieldType *pField)
{
    VpStatusType status;
    VP_API_ENTER(VpDevCtxType, pDevCtx, "SetProfileField");

    /* Basic argument checking */
    if (pDevCtx == VP_NULL) {
        VP_ERROR(VpDevCtxType, pDevCtx, ("VpSetProfileField(): %s.", "Can't accept VP_NULL for profile pointer."));
        status = VP_STATUS_INVALID_ARG;
    } else if (VpGetProfileIndex(pProfile) != -1) {
        VP_ERROR(VpDevCtxType, pDevCtx, ("VpSetProfileField(): %s.", "Can't accept an index for profile pointer."));
        status = VP_STATUS_INVALID_ARG;
    } else {
        status = VP_CALL_DEV_FUNC(GetProfileField, (pDevCtx, pProfile, pField));
    }

    VP_API_EXIT(None, VP_NULL, "SetProfileField", status);
    return status;
} /* VpSetProfileField() */
