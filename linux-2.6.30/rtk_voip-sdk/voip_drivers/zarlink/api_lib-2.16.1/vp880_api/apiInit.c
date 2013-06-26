/** \file apiInit.c
 * apiInit.c
 *
 * This file contains the line and device initialization functions for
 * the Vp880 device API.
 *
 * Copyright (c) 2010, Zarlink Semiconductor, Inc.
 *
 * $Revision: 6677 $
 * $LastChangedDate: 2010-03-15 15:26:33 -0500 (Mon, 15 Mar 2010) $
 */

#include "vp_api_cfg.h"

#if defined (VP_CC_880_SERIES)

/* INCLUDES */
#include "vp_api_types.h"
#include "vp_api.h"
#include "vp_api_int.h"
#include "vp880_api.h"
#include "vp880_api_int.h"
#include "vp_hal.h"
#include "sys_service.h"

/**< Vp880 Initalization Function Prototypes */
static VpStatusType
Vp880InitDevice(
    VpDevCtxType *pDevCtx,
    VpProfilePtrType pDevProfile,
    VpProfilePtrType pAcProfile,
    VpProfilePtrType pDcProfile,
    VpProfilePtrType pRingProfile,
    VpProfilePtrType pFxoAcProfile,
    VpProfilePtrType pFxoCfgProfile);

static VpStatusType
Vp880DeviceReset(
    Vp880DeviceObjectType *pDevObj);

static VpStatusType
Vp880InitLine(
    VpLineCtxType *pLineCtx,
    VpProfilePtrType pAcProfile,
    VpProfilePtrType pDcFeedOrFxoCfgProfile,
    VpProfilePtrType pRingProfile);

static VpStatusType
Vp880ConfigLine(
    VpLineCtxType *pLineCtx,
    VpProfilePtrType pAcProfile,
    VpProfilePtrType pDcFeedOrFxoCfgProfile,
    VpProfilePtrType pRingProfile);

static VpStatusType
Vp880InitProfile(
    VpDevCtxType *pDevCtx,
    VpProfileType type,
    VpProfilePtrType pProfileIndex,
    VpProfilePtrType pProfile);

static void
Vp880InitDeviceObject(
    Vp880DeviceObjectType *pDevObj);

static void
Vp880InitLineObject(
    Vp880LineObjectType *pLineObj);

static void
Vp880CopyDefaultFRProfile(
    Vp880DeviceObjectType *pDevObj);

typedef enum
{
    VP880_DEV_PROFILE_PCLK_MSB = 6,
    VP880_DEV_PROFILE_PCLK_LSB = 7,
    VP880_DEV_PROFILE_DEVCFG1 = 8,
    VP880_DEV_PROFILE_MCLK_CORR = 9,
    VP880_DEV_PROFILE_CLOCK_SLOT = 10,
    VP880_DEV_PROFILE_MAX_EVENTS = 11,
    VP880_DEV_PROFILE_TICKRATE_MSB = 12,
    VP880_DEV_PROFILE_TICKRATE_LSB = 13,
    VP880_DEV_PROFILE_SWITCHER_CMD = 14,
    VP880_DEV_PROFILE_SWITCHER_DATA0 = 15,
    VP880_DEV_PROFILE_SWITCHER_DATA1 = 16,
    VP880_DEV_PROFILE_SWITCHER_DATA2 = 17,

    VP880_ABS_DEV_PROFILE_YVOLT = 18,
    VP880_ABS_DEV_PROFILE_ZVOLT = 19,

    VP880_DEV_PROFILE_TRACKER_INT_SW_REG = 18,

    VP880_DEV_PROFILE_ABS_INT_SW_REG = 24,
    VP880_DEV_ENUM_SIZE = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} vp880_deviceProfileParams;

/**
 * VpMakeVp880DeviceObject()
 *  This function performs the main tasks of VpMakeDeviceObject() for Vp880 type
 * of devices.
 *
 * Preconditions:
 *  Same as VpMakeDeviceObject(), and in addition the deviceType pointed to by
 * pDevCtx should be Vp880 series type.
 *
 * Postconditions:
 *  VpAPI Function pointers for pDevCtx are initialized to Vp880 specific
 * functions.  This completes the function abstraction for "this" device.
 */
VpStatusType
VpMakeVp880DeviceObject(
    VpDevCtxType *pDevCtx,  /**< Device context to be initialized with function
                             * pointers
                             */
    Vp880DeviceObjectType *pDevObj) /**< Device object containing information
                                     * for the device pointed to by pDevCtx
                                     */
{
    Vp880InitDeviceObject(pDevObj);

    /* Initialize other elements in the device object */
    return VpMakeVp880DeviceCtx(pDevCtx, pDevObj);
}

/**
 * Vp880InitDeviceObject()
 *  This function initializes the Vp880 Device object data structure. It is
 * called only in this file .
 */
static void
Vp880InitDeviceObject(
    Vp880DeviceObjectType *pDevObj)
{
    uint16 objSize;
    uint8 *objPtr = (uint8 *)pDevObj;

    for (objSize = 0;
         objSize < (sizeof(Vp880DeviceObjectType) / sizeof(uint8));
         objSize++) {

        *objPtr = 0;
        objPtr++;
    }

    pDevObj->staticInfo.maxChannels = VP880_MAX_NUM_CHANNELS;

#ifdef VP_DEBUG
    pDevObj->debugSelectMask = VP_OPTION_DEFAULT_DEBUG_SELECT;
#endif
}

/**
 * Vp880InitLineObject()
 *  This function initializes the Vp880 Line Object data structure. It is
 * called only in this file .
 */
static void
Vp880InitLineObject(
    Vp880LineObjectType *pLineObj)
{
    uint16 objSize;
    uint8 *objPtr = (uint8 *)pLineObj;

    for (objSize = 0;
         objSize < (sizeof(Vp880LineObjectType) / sizeof(uint8));
         objSize++) {

        *objPtr = 0;
        objPtr++;
    }

#ifdef VP_DEBUG
    pLineObj->debugSelectMask = VP_OPTION_DEFAULT_DEBUG_SELECT;
#endif
}

/**
 * VpMakeVp880DeviceCtx()
 *  This function initializes the device context to handle Vp880 functionality.
 *
 * Preconditions:
 *  This function should be called after initializing the device object. This
 * function can be called more than once since it does not modify the contents
 * of the device object.
 *
 * Postconditions:
 *  Initializes device context to be able to handle Vp780 functionality.
 */
VpStatusType
VpMakeVp880DeviceCtx(
    VpDevCtxType *pDevCtx,          /**< Device Context to be initialized */
    Vp880DeviceObjectType *pDevObj) /**< Device Object that has been already
                                     * initialized
                                     */
{
    uint8 channelCount, maxChan;

    if((pDevCtx == VP_NULL) || (pDevObj == VP_NULL)) {
        return VP_STATUS_INVALID_ARG;
    }

    /* Initialize Device context */
    pDevCtx->pDevObj = pDevObj;
    pDevCtx->deviceType = VP_DEV_880_SERIES;

    /*
     * Initialize all of the line context pointers to null in the device context
     */
    maxChan = pDevObj->staticInfo.maxChannels;
    for (channelCount = 0; channelCount < maxChan; channelCount++) {
        pDevCtx->pLineCtx[channelCount] = VP_NULL;
    }

    /* Functions in apiInit.c */
    pDevCtx->funPtrsToApiFuncs.MakeLineObject = Vp880MakeLineObject;
    pDevCtx->funPtrsToApiFuncs.InitDevice = Vp880InitDevice;
    pDevCtx->funPtrsToApiFuncs.InitLine = Vp880InitLine;
    pDevCtx->funPtrsToApiFuncs.ConfigLine = Vp880ConfigLine;
    pDevCtx->funPtrsToApiFuncs.InitProfile = Vp880InitProfile;
    pDevCtx->funPtrsToApiFuncs.FreeRun = Vp880FreeRun;

#ifdef CSLAC_SEQ_EN
    pDevCtx->funPtrsToApiFuncs.InitRing = Vp880InitRing;
    pDevCtx->funPtrsToApiFuncs.InitCid = Vp880InitCid;
    pDevCtx->funPtrsToApiFuncs.InitMeter = VpCSLACInitMeter;
    pDevCtx->funPtrsToApiFuncs.DtmfDigitDetected = VpCSLACDtmfDigitDetected;
#endif

    pDevCtx->funPtrsToApiFuncs.ClearResults = VpCSLACClearResults;

    /* Functions in apicnt.c */
    pDevCtx->funPtrsToApiFuncs.SetLineState = Vp880SetLineState;
    pDevCtx->funPtrsToApiFuncs.SetLineTone = Vp880SetLineTone;
    pDevCtx->funPtrsToApiFuncs.SetRelGain = Vp880SetRelGain;
    pDevCtx->funPtrsToApiFuncs.SetRelayState = Vp880SetRelayState;

#ifdef CSLAC_SEQ_EN
    pDevCtx->funPtrsToApiFuncs.SendSignal = Vp880SendSignal;
    pDevCtx->funPtrsToApiFuncs.SendCid = Vp880SendCid;
    pDevCtx->funPtrsToApiFuncs.ContinueCid = Vp880ContinueCid;
    pDevCtx->funPtrsToApiFuncs.StartMeter = VpCSLACStartMeter;
#endif

    pDevCtx->funPtrsToApiFuncs.SetOption = Vp880SetOption;
    pDevCtx->funPtrsToApiFuncs.VirtualISR = Vp880VirtualISR;
    pDevCtx->funPtrsToApiFuncs.ApiTick = Vp880ApiTick;
    pDevCtx->funPtrsToApiFuncs.LowLevelCmd = Vp880LowLevelCmd;
    pDevCtx->funPtrsToApiFuncs.DeviceIoAccess = Vp880DeviceIoAccess;

    /* Functions in apiQuery.c */
    pDevCtx->funPtrsToApiFuncs.GetEvent = Vp880GetEvent;
    pDevCtx->funPtrsToApiFuncs.GetLineStatus = VpCSLACGetLineStatus;
    pDevCtx->funPtrsToApiFuncs.GetDeviceStatus = Vp880GetDeviceStatus;
    pDevCtx->funPtrsToApiFuncs.FlushEvents = Vp880FlushEvents;
    pDevCtx->funPtrsToApiFuncs.GetResults = Vp880GetResults;
    pDevCtx->funPtrsToApiFuncs.GetOption = Vp880GetOption;
    pDevCtx->funPtrsToApiFuncs.GetRelayState    = Vp880GetRelayState;

#if 1//def VP_DEBUG
    pDevCtx->funPtrsToApiFuncs.RegisterDump = Vp880RegisterDump;
#endif

    pDevCtx->funPtrsToApiFuncs.RegisterReadWrite = Vp880RegisterReadWrite;

    /* Functions in apiTestLine.c */
#ifdef VP880_INCLUDE_TESTLINE_CODE
    pDevCtx->funPtrsToApiFuncs.TestLineInt = Vp880TestLineInt;
    pDevCtx->funPtrsToApiFuncs.TestLineCallback = Vp880TestLineCallback;
    pDevCtx->funPtrsToApiFuncs.TestLine = Vp880TestLine;
#endif

    /* Functions in apiCal.c */
    pDevCtx->funPtrsToApiFuncs.CalCodec = Vp880CalCodec;
    pDevCtx->funPtrsToApiFuncs.CalLine = Vp880CalLine;
    pDevCtx->funPtrsToApiFuncs.Cal = Vp880Cal;

    return VP_STATUS_SUCCESS;
}

/**
 * VpMakeVp880LineObject()
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
Vp880MakeLineObject(
    VpTermType termType,
    uint8 channelId,
    VpLineCtxType *pLineCtx,
    void *pVoidLineObj,
    VpDevCtxType *pDevCtx)
{
    Vp880LineObjectType *pLineObj = pVoidLineObj;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpDeviceIdType deviceId = pDevObj->deviceId;
    Vp880DeviceStateIntType chanMap[] = {VP880_LINE0_IS_FXO, VP880_LINE1_IS_FXO};

    if (channelId >= pDevObj->staticInfo.maxChannels) {
        return VP_STATUS_INVALID_ARG;
    }

    VpSysEnterCritical(deviceId, VP_CODE_CRITICAL_SEC);

    Vp880InitLineObject(pLineObj);

    switch (termType) {
        case VP_TERM_FXO_GENERIC:
        case VP_TERM_FXO_DISC:
            /*
             * At this point, it is only a recommendation. We'll adjust this
             * when we determine the device type found in VpInitDevice()
             */
            if (pDevObj->stateInt & VP880_DEVICE_DETECTED) {
                if (chanMap[channelId] & pDevObj->stateInt) {
                    pLineObj->status |= VP880_IS_FXO;
                } else {
                    VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
                    return VP_STATUS_ERR_VTD_CODE;
                }
            } else {
                pLineObj->status |= VP880_IS_FXO;
            }

            pDevObj->stateInt |=
                ((channelId == 0) ? VP880_LINE0_LP : VP880_LINE1_LP);
            /*
             * Force the device object calibration structure to "think" that
             * the FXO line of the device is calibrated. This allows
             * retrieval of calibration structure from VpCal().
             */
            pDevObj->vp880SysCalData.ila40[channelId] = 1;
            break;

        case VP_TERM_FXS_GENERIC:
        case VP_TERM_FXS_ISOLATE:
        case VP_TERM_FXS_SPLITTER:
            pLineObj->status = VP880_INIT_STATUS;
            pDevObj->stateInt &=
                ((channelId == 0) ? ~VP880_LINE0_LP : ~VP880_LINE1_LP);
            break;

        case VP_TERM_FXS_LOW_PWR:
        case VP_TERM_FXS_ISOLATE_LP:
        case VP_TERM_FXS_SPLITTER_LP:
            pLineObj->status = VP880_INIT_STATUS;
            pDevObj->stateInt |=
                ((channelId == 0) ? VP880_LINE0_LP : VP880_LINE1_LP);
            break;

        default:
            VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
            return VP_STATUS_ERR_VTD_CODE;
    }

    pLineCtx->pLineObj = pLineObj;
    pLineCtx->pDevCtx = pDevCtx;

    pDevCtx->pLineCtx[channelId] = pLineCtx;
    pLineObj->channelId = channelId;
    pLineObj->termType = termType;

    VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);

    /* Everything else done by device/line specific functions */
    return VP_STATUS_SUCCESS;
}

/**
 * Vp880Init
 *  This function initializes the device, and (contrary to InitDevice) does
 * not initialize any channels. This function should be called internal to the
 * API only.
 *
 * Preconditions:
 *  The device context must be of a Vp880 device type.
 *
 * Postconditions:
 *  This function returns a failure code if the clock configuration is not set
 * correctly based on the device data set in InitDevice.
 */
VpStatusType
Vp880Init(
    VpDevCtxType *pDevCtx)
{
    VpLineCtxType *pLineCtx;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpDeviceIdType deviceId = pDevObj->deviceId;

    uint8 data, chanId, maxChan = 0;
    uint8 intMaskData[] = {0x7F, 0xFF};
    uint8 clkNotStable;
    uint8 clkTestCount;

    VP_API_FUNC_INT(VpDevCtxType, pDevCtx, ("Vp880Init+"));

    /*
     * When the device timing is cleared, so are the lines associated with
     * this device
     */
    maxChan = pDevObj->staticInfo.maxChannels;
    for (chanId = 0; chanId < maxChan; chanId++) {
        /*
         * It is possible that any channel on a device is not used and therefore
         * points to a NULL pointer. Check it and jump over if this is the case
         * to the next line until all possible line numbers for this device have
         * been checked.
         */
        pLineCtx = pDevCtx->pLineCtx[chanId];
        if (pLineCtx != VP_NULL) {
            /*
             * If the device is "dumping" it's timers, the lines associated with
             * this device better not be relying on timers.
             */
            InitTimerVars(pLineCtx);
        }
    }

    /*
     * If the MPI Bus gets out of sequence for any reason, a HW reset command
     * will not work and this function may fail. To be sure a reset occurs, the
     * following sequence is required.
     */
    if(Vp880DeviceReset(pDevObj) != VP_STATUS_SUCCESS) {
        VP_ERROR(VpDevCtxType, pDevCtx, ("Device %d Failed to Reset Properly",
            pDevObj->deviceId));

        VP_API_FUNC_INT(VpDevCtxType, pDevCtx, ("Vp880Init-"));
        return VP_STATUS_FAILURE;
    }

    /*
     * Setup mclk. The MCLK mask set the mclk frequency, sets the mclk source
     * (the MCLK pin or the PCLK pin), and sets the interrupt pin output drive
     * mode (TTL or open collector)
     */
    data = pDevObj->devProfileData.devCfg1;
    VpMpiCmdWrapper(deviceId, VP880_EC_CH1, VP880_MCLK_CNT_WRT, VP880_MCLK_CNT_LEN,
        &data);

    /* Setup the Clock Fail Interrupt */
    VpMpiCmdWrapper(deviceId, VP880_EC_CH1, VP880_INT_MASK_WRT, VP880_INT_MASK_LEN,
        intMaskData);

    /*
     * Wait for the CFAIL bit to clear before proceding. If the CFAIL bit does
     * not clear after several trys, give up and return an error condition. Wait
     * between each read of the status register.
     */

    clkNotStable = VP880_CFAIL_MASK;
    clkTestCount = MAX_CFAIL_TEST;
    while(clkNotStable && (--clkTestCount) != 0) {
        VpSysWait(CFAIL_TEST_INTERVAL*10);
        VpMpiCmdWrapper(deviceId, VP880_EC_CH1, VP880_UL_SIGREG_RD,
            VP880_UL_SIGREG_LEN, pDevObj->intReg);
        clkNotStable = pDevObj->intReg[0] & VP880_CFAIL_MASK;
    }

    /*
     * The CFAIL bit did not clear so the part will not complete initialization.
     * Return error status to indicate failure.
     */
    if(clkNotStable) {
        pDevObj->deviceEvents.faults |= VP_DEV_EVID_CLK_FLT;

        VP_ERROR(VpDevCtxType, pDevCtx, ("Device %d Failed to Reset Clock Fault",
            pDevObj->deviceId));

        VP_API_FUNC_INT(VpDevCtxType, pDevCtx, ("Vp880Init-"));
        return VP_STATUS_FAILURE;
    }

    /* Setup interrupts back to default */
    intMaskData[0] = 0xFF;  /* Clear all Device Interrupt Masks */
    VpMpiCmdWrapper(deviceId, VP880_EC_CH1, VP880_INT_MASK_WRT, VP880_INT_MASK_LEN,
        intMaskData);

    /*
     * The PCM mask tells the device which clock edge to grab and xmit the
     * PCM data on and also which clock period LSB of the PCM data starts on
     */
    data = pDevObj->devProfileData.clockSlot;
    VpMpiCmdWrapper(deviceId, VP880_EC_CH1, VP880_XR_CS_WRT, VP880_XR_CS_LEN, &data);

    VP_API_FUNC_INT(VpDevCtxType, pDevCtx, ("Vp880Init-"));
    return VP_STATUS_SUCCESS;
} /* Vp880Init */

/**
 * Vp880DeviceReset
 *  This function resets the MPI buffer and HW reset of the device. The method
 * for doing this depends on the silicon revision due to I/O1 driver
 * requirements.
 *
 * Preconditions:
 *  None.
 *
 * Postconditions:
 *  The Device is reset, and I/O1 is in a safe condition.
 */
VpStatusType
Vp880DeviceReset(
    Vp880DeviceObjectType *pDevObj)
{
    VpDeviceIdType deviceId = pDevObj->deviceId;
    uint8 mpiCmdData;
    uint8 devicePcn, deviceRcn;

    uint8 mpiClear[] = {
        VP880_NO_OP_WRT, VP880_NO_OP_WRT, VP880_NO_OP_WRT, VP880_NO_OP_WRT, VP880_NO_OP_WRT,
        VP880_NO_OP_WRT, VP880_NO_OP_WRT, VP880_NO_OP_WRT, VP880_NO_OP_WRT, VP880_NO_OP_WRT,
        VP880_NO_OP_WRT, VP880_NO_OP_WRT, VP880_NO_OP_WRT, VP880_NO_OP_WRT, VP880_NO_OP_WRT,
        VP880_NO_OP_WRT
    };

    VP_API_FUNC_INT(None, VP_NULL, ("Vp880DeviceReset+"));

    /*
     * First, make sure the MPI buffer is cleared so we can write to the
     * device correctly prior to HW reset.
     */
    VpMpiCmdWrapper(deviceId, VP880_EC_CH1, VP880_NO_OP_WRT, 16, mpiClear);

    /*
     * Read revision code
     * If >= JA then force I/O as below
     * Force I/O1 to '1', and wait for (if present) the external relay to
     * open
     */
    VpMpiCmdWrapper(deviceId, VP880_EC_CH1, VP880_RCN_PCN_RD, VP880_RCN_PCN_LEN,
        pDevObj->staticInfo.rcnPcn);

    /* Force the revision code to at least JE to allow calibration */
    if ((pDevObj->staticInfo.rcnPcn[VP880_PCN_LOCATION] == VP880_DEV_PCN_88536) &&
        (pDevObj->staticInfo.rcnPcn[VP880_RCN_LOCATION] < VP880_REV_JE)) {
        pDevObj->staticInfo.rcnPcn[VP880_RCN_LOCATION] = VP880_REV_JE;
    }
    devicePcn = pDevObj->staticInfo.rcnPcn[VP880_PCN_LOCATION];
    deviceRcn = pDevObj->staticInfo.rcnPcn[VP880_RCN_LOCATION];

    /* MPI Failure if the PCN and RCN are both 0x00 or 0xFF */
    if (((devicePcn == 0xFF) && (deviceRcn == 0xFF)) ||
        ((devicePcn == 0x00) && (deviceRcn == 0x00))) {
        pDevObj->status.state &= ~(VP_DEV_INIT_IN_PROGRESS | VP_DEV_INIT_CMP);

        VP_ERROR(None, VP_NULL, ("Device %d Failed to Detect Revision/PCN Properly",
            pDevObj->deviceId));

        VP_API_FUNC_INT(None, VP_NULL, ("Vp880DeviceReset-"));
        return VP_STATUS_FAILURE;
    }

    if (pDevObj->staticInfo.rcnPcn[VP880_RCN_LOCATION] < VP880_REV_VC) {
        pDevObj->status.state &= ~(VP_DEV_INIT_IN_PROGRESS | VP_DEV_INIT_CMP);

        VP_ERROR(None, VP_NULL, ("Unsupported Silicon Revision %d",
            pDevObj->staticInfo.rcnPcn[VP880_RCN_LOCATION]));

        VP_API_FUNC_INT(None, VP_NULL, ("Vp880DeviceReset-"));
        return VP_STATUS_FAILURE;
    }

    if (pDevObj->staticInfo.rcnPcn[VP880_RCN_LOCATION] >= VP880_REV_JE) {
        mpiCmdData = VP880_IODATA_IO1;
        VP_INFO(None, NULL, ("3. Write IODATA 0x%02X on Both Channels",
            VP880_IODATA_IO1));

        VpMpiCmdWrapper(deviceId, (VP880_EC_CH1 | VP880_EC_CH2), VP880_IODATA_REG_WRT,
            VP880_IODATA_REG_LEN, &mpiCmdData);
        VpSysWait(24);  /* EMR's take 2-3 ms to open/close */
    }

    /* Proceed with normal device level reset and required delay */
    mpiCmdData = VP880_HW_RESET_WRT;
    VpMpiCmdWrapper(deviceId, VP880_EC_CH1, VP880_HW_RESET_WRT,
        VP880_HW_RESET_LEN, &mpiCmdData);
    VpSysWait(20);

    VP_API_FUNC_INT(None, VP_NULL, ("Vp880DeviceReset-"));

    return VP_STATUS_SUCCESS;
}

/**
 * Vp880InitDevice
 *  This function initializes the device and all lines associated with this
 * device (if line profiles are passed to this function). The device profile
 * passed must be valid otherwise an error code is returned and the device
 * remains in it's previously initialized state.
 *
 * Preconditions:
 *  None (device context is not NULL and is of Vp880 type, which is handled in
 * higher level software)
 *
 * Postconditions:
 *  This device is initialized to the configuration specified in the device
 * profile, and the FXS lines associated with this device are initialized by the
 * FXS specific AC, DC, and Ringing profiles passed, and the FXO lines
 * associated with this device are initialized by the FXO specific AC and Config
 * profiles passed.  If the FXO/FXS profiles are all NULL, then only the device
 * initialization occurs. This function returns an error code if the device
 * profile trying to be used for initialization is VP_PTABLE_NULL (either
 * passed or by a non-initialized index).
 */
VpStatusType
Vp880InitDevice(
    VpDevCtxType *pDevCtx,
    VpProfilePtrType pDevProfile,   /**< The profile pointer for the device
                                     * configuration parameters
                                     */
    VpProfilePtrType pAcProfile,    /**< The profile pointer (or index) for
                                     * the AC characteristic to apply to the
                                     * FXS lines
                                     */
    VpProfilePtrType pDcProfile,    /**< The profile pointer (or index) for
                                     * the DC characteristic to apply to the
                                     * FXS lines
                                     */
    VpProfilePtrType pRingProfile,  /**< The profile pointer (or index) for
                                     * the Ringing characteristic to apply to
                                     * the FXS lines
                                     */
    VpProfilePtrType pFxoAcProfile, /**< The profile pointer (or index) for
                                     * the AC characteristic to apply to the
                                     * FXO lines
                                     */
    VpProfilePtrType pFxoCfgProfile)/**< The profile pointer for the FXO
                                     * specific supervision paramaters.
                                     */
{
    VpLineCtxType *pLineCtx;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpDeviceIdType deviceId = pDevObj->deviceId;
    Vp880LineObjectType *pLineObj;
    uint8 ecVal[] = {VP880_EC_CH1, VP880_EC_CH2};
    uint8 maxChan = pDevObj->staticInfo.maxChannels;
    VpProfilePtrType pDevProf;
    uint8 intSwParam[VP880_INT_SWREG_PARAM_LEN] =
        {0x5C, 0x4B, 0xC4, 0x4B, 0xC4, 0x4B};

#ifdef VP880_CURRENT_LIMIT
    uint8 intSwParamLimit[VP880_INT_SWREG_PARAM_LEN] =
        {0xB2, 0x00, 0xB1, 0x00, 0x60, 0x40};
#endif

    uint8 deviceRcn, chan, data;

    VpStatusType status = VP_STATUS_SUCCESS;
    uint8 devicePcn;

    int profIndex = VpGetProfileIndex(pDevProfile);

    VP_API_FUNC_INT(None, VP_NULL, ("Vp880InitDevice+"));

    /*
     * Get Profile Index returns -1 if the profile passed is a pointer or
     * of VP_PTABLE_NULL type. Otherwise it returns the index
     */

    if (profIndex < 0) {
        /*
         * A pointer is passed or VP_PTABLE_NULL.  If it's a pointer, make
         * sure the content is valid for the profile type.
         */
        if (pDevProfile != VP_PTABLE_NULL) {
            if(VpVerifyProfileType(VP_PROFILE_DEVICE, pDevProfile) != TRUE) {
                VP_API_FUNC_INT(None, VP_NULL, ("Vp880InitDevice-"));
                return VP_STATUS_ERR_PROFILE;
            }
        } else {
            VpSysEnterCritical(deviceId, VP_CODE_CRITICAL_SEC);
            pDevObj->status.state &= ~VP_DEV_INIT_IN_PROGRESS;
            VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
            VP_API_FUNC_INT(None, VP_NULL, ("Vp880InitDevice-"));
            return VP_STATUS_ERR_PROFILE;
        }
        pDevProf = pDevProfile;
    } else if (profIndex < VP_CSLAC_DEV_PROF_TABLE_SIZE) {
        pDevProf = pDevObj->devProfileTable.pDevProfileTable[profIndex];
    } else {
        VP_API_FUNC_INT(None, VP_NULL, ("Vp880InitDevice-"));
        return VP_STATUS_ERR_PROFILE;
    }

    VpSysEnterCritical(deviceId, VP_CODE_CRITICAL_SEC);

    /* Initialize the API's device status variables */
    if (pDevObj->status.state & VP_DEV_INIT_CMP) {
        pDevObj->status.state = (VP_DEV_WARM_REBOOT | VP_DEV_INIT_IN_PROGRESS);
    } else {
        pDevObj->status.state = VP_DEV_INIT_IN_PROGRESS;
    }
    pDevObj->timeStamp = 0;

    /* Initialize the API's device dynamic variables */
    pDevObj->dynamicInfo.lastChan = 0;
    pDevObj->dynamicInfo.bat1Fault = FALSE;
    pDevObj->dynamicInfo.bat2Fault = FALSE;
    pDevObj->dynamicInfo.bat3Fault = FALSE;
    pDevObj->dynamicInfo.clkFault = FALSE;

    /*
     * Reset the internal state information except for possibly previously
     * loaded calibration values.
     */
    pDevObj->stateInt &= VP880_SYS_CAL_COMPLETE;

    if (pDevProf != VP_PTABLE_NULL) {
        pDevObj->devProfileData.pcmClkRate =
            (uint16)(((pDevProf[VP880_DEV_PROFILE_PCLK_MSB] << 8) & 0xFF00)
                    | (pDevProf[VP880_DEV_PROFILE_PCLK_LSB] & 0x00FF));

        pDevObj->devProfileData.devCfg1 =
            (uint8)(pDevProf[VP880_DEV_PROFILE_DEVCFG1]);
        pDevObj->devProfileData.clockSlot =
            (uint8)(pDevProf[VP880_DEV_PROFILE_CLOCK_SLOT]);

        pDevObj->devProfileData.tickRate =
            (uint16)(((pDevProf[VP880_DEV_PROFILE_TICKRATE_MSB] << 8) & 0xFF00)
                    | (pDevProf[VP880_DEV_PROFILE_TICKRATE_LSB] & 0x00FF));

        if (pDevProf[VP_PROFILE_INDEX] & 0x01) {
            pDevObj->devProfileData.peakManagement = TRUE;
        } else {
            pDevObj->devProfileData.peakManagement = FALSE;
        }

        if (pDevProf[VP_PROFILE_VERSION] >= VP_CSLAC_DEV_PROFILE_VERSION_SW_CONFIG) {
            /* Cache the target switcher parameters */
            VpMemCpy(pDevObj->swParams, &pDevProf[VP880_DEV_PROFILE_SWITCHER_CMD+1],
                VP880_REGULATOR_PARAM_LEN);
        } else {
            pDevObj->status.state &=
                ~(VP_DEV_INIT_IN_PROGRESS | VP_DEV_INIT_CMP);

            VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);

            VP_ERROR(None, VP_NULL, ("Device %d Unsupported Device Profile Version",
                pDevObj->deviceId));

            VP_API_FUNC_INT(None, VP_NULL, ("Vp880InitDevice Unsupported Profile Version-"));
            return VP_STATUS_ERR_PROFILE;
        }
    }

    /* Initialize device */
    /*
     * If not successful, the Clock Fail bit did not clear so return error code
     */
    if ((status = Vp880Init(pDevCtx)) != VP_STATUS_SUCCESS) {
        pDevObj->status.state &= ~(VP_DEV_INIT_IN_PROGRESS | VP_DEV_INIT_CMP);
        VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
        VP_API_FUNC_INT(None, VP_NULL, ("Vp880InitDevice Init Failure-"));
        return status;
    }

    devicePcn = pDevObj->staticInfo.rcnPcn[VP880_PCN_LOCATION];
    deviceRcn = pDevObj->staticInfo.rcnPcn[VP880_RCN_LOCATION];

    /*
     * Verify that we recognize the device by limiting to the range of those
     * supported in the Vp880PcnType table. If not recognized (although may
     * be a valid Zarlink Semiconductor PN) return an error because the API-II
     * does not know how to handle it. More often, the error is occuring because
     * the hardware cannot talk to the device.
     */
    switch(devicePcn) {
        case VP880_DEV_PCN_88010:   /**< FXO */
        case VP880_DEV_PCN_88111:   /* FXS-Tracker */
        case VP880_DEV_PCN_88116:   /* FXS-Tracker - Wideband */
        case VP880_DEV_PCN_88131:   /* FXS-Tracker */
        case VP880_DEV_PCN_88136:   /* FXS-Tracker - Wideband */
            pDevObj->staticInfo.maxChannels = 1;
            maxChan = 1;
            pDevObj->stateInt |= VP880_IS_SINGLE_CHANNEL;
            /*
             * Force the device object calibration structure to "think" that
             * the second virtual line of the device is calibrated. This allows
             * retrieval of calibration structure from VpCal().
             */
            pDevObj->vp880SysCalData.ila40[1] = 1;
            break;

        case VP880_DEV_PCN_88211:   /* 2FXS-Tracker */
        case VP880_DEV_PCN_88216:   /* 2FXS-Tracker - Wideband */
        case VP880_DEV_PCN_88221:   /* 2FXS-ABS */
        case VP880_DEV_PCN_88226:   /* 2FXS-ABS - Wideband */
        case VP880_DEV_PCN_88231:   /* 2FXS-Tracker */
        case VP880_DEV_PCN_88236:   /* 2FXS-Tracker - Wideband */

        case VP880_DEV_PCN_88241:   /* 2FXS-ABS */
        case VP880_DEV_PCN_88246:   /* 2FXS-ABS - Wideband */

        case VP880_DEV_PCN_88311:   /* FXO/FXS-Tracker */
        case VP880_DEV_PCN_88331:   /* FXO/FXS-Tracker */

        case VP880_DEV_PCN_88506:  /* 2FXS-Tracker - Wideband Split Package*/
        case VP880_DEV_PCN_88536:   /* 2FXS-Tracker - Wideband, IP Block */
            pDevObj->staticInfo.maxChannels = 2;
            maxChan = 2;
            pDevObj->stateInt &= ~VP880_IS_SINGLE_CHANNEL;
            break;

        default:
            pDevObj->status.state &= ~(VP_DEV_INIT_IN_PROGRESS | VP_DEV_INIT_CMP);
            VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);

            VP_ERROR(None, VP_NULL, ("Device %d Revision/PCN Unknown",
                pDevObj->deviceId));

            VP_API_FUNC_INT(None, VP_NULL, ("Vp880InitDevice-"));
            return VP_STATUS_FAILURE;
    }

    pDevObj->stateInt |= VP880_DEVICE_DETECTED;

    /*
     * Make initial assumption that the device has lines such that low power
     * mode is in affect. The line controls will change as necessary.
     */
    pDevObj->stateInt |= (VP880_LINE0_LP | VP880_LINE1_LP);

    /* Check if device is Tracker, otherwise it's ABS or only FXO */
    if (devicePcn & VP880_TRACKER_MASK) {
        uint8 icr3Vals[VP880_ICR3_LEN];
        uint8 ecAll = (VP880_EC_CH1 | VP880_EC_CH2);

        /* Mark as non-ABS device type */
        pDevObj->stateInt &= ~VP880_IS_ABS;

        /* Initialize Tracker device sensitve items */
        /*
         * Configure the Switcher for Flyback or Buckboost per device
         * profile. If the device is in Buckboost mode, config the internal
         * switcher
         */
        if (pDevProf[VP_PROFILE_VERSION] >= VP_CSLAC_DEV_PROFILE_VERSION_INT_SW_CONFIG) {
            /* Write the internal switcher parameters */
            VpMpiCmdWrapper(deviceId, ecVal[0], VP880_INT_SWREG_PARAM_WRT,
                VP880_INT_SWREG_PARAM_LEN, (uint8 *)&pDevProf[VP880_DEV_PROFILE_TRACKER_INT_SW_REG]);

            if (pDevProf[VP_PROFILE_VERSION] >= VP_CSLAC_DEV_PROFILE_VERSION_INT_SW_CONFIG_FR) {
                /* Cache the internal switcher parameters for free run mode */
                VpMemCpy(pDevObj->intSwParamsFR, &pDevProf[VP880_DEV_PROFILE_TRACKER_INT_SW_REG +
                    VP880_INT_SWREG_PARAM_LEN], VP880_INT_SWREG_PARAM_LEN);
            } else {
                Vp880CopyDefaultFRProfile(pDevObj);
            }
        } else {
            if (pDevProf[VP880_DEV_PROFILE_SWITCHER_CMD+1] & 0x20) {
                VpMpiCmdWrapper(deviceId, ecVal[0], VP880_INT_SWREG_PARAM_WRT,
                    VP880_INT_SWREG_PARAM_LEN, intSwParam);

                /* No free run timing available -> save default ones */
                Vp880CopyDefaultFRProfile(pDevObj);
            }
        }

        VpMpiCmdWrapper(deviceId, ecVal[0], VP880_REGULATOR_PARAM_WRT,
            VP880_REGULATOR_PARAM_LEN,
            (VpProfileDataType *)(&pDevProf[VP880_DEV_PROFILE_SWITCHER_CMD+1]));

        if ((pDevObj->swParams[VP880_SWREG_RING_V_BYTE]
           & VP880_VOLTAGE_MASK) < VP880_VOLTAGE_60V) {
            pDevObj->swParams[VP880_SWREG_RING_V_BYTE] &= ~VP880_VOLTAGE_MASK;
            pDevObj->swParams[VP880_SWREG_RING_V_BYTE] |= VP880_VOLTAGE_60V;

            VpMpiCmdWrapper(deviceId, ecVal[0], VP880_REGULATOR_PARAM_WRT,
                VP880_REGULATOR_PARAM_LEN, pDevObj->swParams);
        }

        /* Device is reset, so ch1 and 2 are at same values for ICR's
         * NOTE: This also has to be done in VpInitLine() due to SW
         * Reset.
         */
        VpMpiCmdWrapper(deviceId, ecVal[0], VP880_ICR3_RD, VP880_ICR3_LEN,
            icr3Vals);
        icr3Vals[VP880_ICR3_LINE_CTRL_INDEX] |=  VP880_ICR3_VREF_CTRL;
        icr3Vals[VP880_ICR3_LINE_CTRL_INDEX+1] |=  VP880_ICR3_VREF_CTRL;
        VpMpiCmdWrapper(deviceId, ecAll, VP880_ICR3_WRT, VP880_ICR3_LEN, icr3Vals);
        VP_LINE_STATE(None, NULL, ("Init: ICR3 0x%02X 0x%02X 0x%02X 0x%02X Ch %d",
            icr3Vals[0], icr3Vals[1], icr3Vals[2], icr3Vals[3], ecAll));

        /*
         * Wait at least 5ms before turning the switchers on for Vref to
         * stabilize. We'll wait 10ms to be safe.
         */
        VpSysWait(80);

        /*
         * Enable the switchers in low power. The power mode is changed as
         * needed during normal operation.
         */
        data = VP880_SWY_LP | VP880_SWZ_LP;
        VpMpiCmdWrapper(deviceId, ecVal[0], VP880_REGULATOR_CTRL_WRT,
            VP880_REGULATOR_CTRL_LEN, &data);
    } else {
        if (devicePcn == VP880_DEV_PCN_88010) {
            /* FXO only devices */
            pDevObj->stateInt |= VP880_IS_FXO_ONLY;
        } else {
            /* Last choice is ABS type */
            pDevObj->stateInt |= VP880_IS_ABS;
        }

        if (pDevProf[VP_PROFILE_VERSION] >=
            VP_CSLAC_DEV_PROFILE_VERSION_INT_SW_CONFIG) {

            /* Write the internal switcher parameters */
            VpMpiCmdWrapper(deviceId, ecVal[0], VP880_INT_SWREG_PARAM_WRT,
                VP880_INT_SWREG_PARAM_LEN, (uint8 *)&pDevProf[VP880_DEV_PROFILE_ABS_INT_SW_REG]);

            if (pDevProf[VP_PROFILE_VERSION] >= VP_CSLAC_DEV_PROFILE_VERSION_INT_SW_CONFIG_FR) {
                /* Cache the internal switcher parameters for free run mode */
                VpMemCpy(pDevObj->intSwParamsFR, &pDevProf[VP880_DEV_PROFILE_ABS_INT_SW_REG +
                    VP880_INT_SWREG_PARAM_LEN], VP880_INT_SWREG_PARAM_LEN);
            } else {
                /* No free run timing available -> save default ones */
                Vp880CopyDefaultFRProfile(pDevObj);
            }

            /* Extract Y/Z Voltages. "0" if not set by Profile Wizard */
            pDevObj->yVolt = pDevProf[VP880_ABS_DEV_PROFILE_YVOLT];
            pDevObj->zVolt = pDevProf[VP880_ABS_DEV_PROFILE_ZVOLT];
        } else {
            /* No free run timing available -> save default ones */
            Vp880CopyDefaultFRProfile(pDevObj);
        }

        /*
         * Make sure lines are in disconnect state (recover from shutdown)
         * even if there are no line contexts associated with this device.
         */
        data = VP880_SS_DISCONNECT;
        for (chan = 0; chan < maxChan; chan++) {
            VpMpiCmdWrapper(deviceId, ecVal[chan], VP880_SYS_STATE_WRT,
                VP880_SYS_STATE_LEN, &data);
        }

        data = VP880_SWY_OFF;
        VpMpiCmdWrapper(deviceId, ecVal[0], VP880_REGULATOR_CTRL_WRT,
            VP880_REGULATOR_CTRL_LEN, &data);

#ifdef VP880_CURRENT_LIMIT
        /* Implement a user compile option to limit the switcher power */
        VpMpiCmdWrapper(deviceId, ecVal[0], VP880_INT_SWREG_PARAM_WRT,
            VP880_INT_SWREG_PARAM_LEN, intSwParamLimit);
#endif

        if (!(pDevObj->stateInt & VP880_IS_FXO_ONLY)) {
#ifdef VP880_AUTO_BAT_DETECT
            if ((status = Vp880AutoBatDetect(pDevObj,
                &pDevProf[VP880_DEV_PROFILE_SWITCHER_CMD+1])) != VP_STATUS_SUCCESS) {
                pDevObj->status.state &=
                    ~(VP_DEV_INIT_IN_PROGRESS | VP_DEV_INIT_CMP);

                VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
                VP_API_FUNC_INT(None, VP_NULL, ("Vp880InitDevice Exit Error %d-",
                    status));
                return status;
            }
#else
            VpMpiCmdWrapper(deviceId, ecVal[0], VP880_REGULATOR_PARAM_WRT,
                VP880_REGULATOR_PARAM_LEN,
                (uint8 *)(&pDevProf[VP880_DEV_PROFILE_SWITCHER_CMD+1]));

            /* Low power mode first, wait at least 20ms, then to med power. */
            data = VP880_SWY_LP | VP880_SWZ_LP;
            VpMpiCmdWrapper(deviceId, ecVal[0], VP880_REGULATOR_CTRL_WRT,
                VP880_REGULATOR_CTRL_LEN, &data);

            VpSysWait(160); /* 125us * 160 = 20ms */

            data = VP880_SWY_MP | VP880_SWZ_MP;
            VpMpiCmdWrapper(deviceId, ecVal[0], VP880_REGULATOR_CTRL_WRT,
                VP880_REGULATOR_CTRL_LEN, &data);
#endif
        }
    }

    /* No matter how they were provided, cache the internal switcher parameters */
    VpMpiCmdWrapper(deviceId, ecVal[0], VP880_INT_SWREG_PARAM_RD,
        VP880_INT_SWREG_PARAM_LEN, pDevObj->intSwParams);

    /*
     * Check for High Voltage Device and line test switch
     * Currently high voltage and test switch go hand in hand but may not in
     * the future that is why there are two bits but only a test for one.
     */
    if ((devicePcn & VP880_HV_MASK) == VP880_HV_MASK) {
        pDevObj->stateInt |= VP880_IS_HIGH_VOLTAGE;
        /* VE8820 does not support a test load but is high voltage */
        if (devicePcn != VP880_DEV_PCN_88506) {
            pDevObj->stateInt |= VP880_HAS_TEST_LOAD_SWITCH;
        }
    } else {
        if ((devicePcn == VP880_DEV_PCN_88226) && (deviceRcn > VP880_REV_VC)) {
            pDevObj->stateInt |= VP880_IS_HIGH_VOLTAGE;
        } else {
            pDevObj->stateInt &= ~VP880_IS_HIGH_VOLTAGE;
        }
        pDevObj->stateInt &= ~VP880_HAS_TEST_LOAD_SWITCH;
    }

#ifdef VP880_ALWAYS_USE_INTERNAL_TEST_TERMINATION
    /* If this option is defined, we will treat this device as if it has no
     * physical test load.  The internal test termination only works for
     * revisions newer than VC, so ignore this override if it can't be used */
    if (pDevObj->staticInfo.rcnPcn[VP880_RCN_LOCATION] > VP880_REV_VC) {
        pDevObj->stateInt &= ~VP880_HAS_TEST_LOAD_SWITCH;
    }
#endif

    /* Check for devices that have an FXO on line 0 and/or line 1*/
    switch (devicePcn) {
        case VP880_DEV_PCN_88010:
        case VP880_DEV_PCN_88311:
        case VP880_DEV_PCN_88331:
            pDevObj->stateInt |= VP880_LINE0_IS_FXO;
            break;

        default:
            break;
    }

    /* Check for Wideband Mode support */
    if ((devicePcn == VP880_DEV_PCN_88506) ||
        ((devicePcn & (VP880_CODEC_MASK | VP880_WIDEBAND_MASK))
         == (VP880_CODEC_MASK | VP880_WIDEBAND_MASK))) {
        pDevObj->stateInt |= VP880_WIDEBAND;
    }

    /* Check for Cal Circuit */
    if (VP880_REV_VA == deviceRcn) {
        /* none of the rev 1 devices have cal circuit */
        pDevObj->stateInt &= ~VP880_HAS_CALIBRATE_CIRCUIT;

    } else if (VP880_REV_VC == deviceRcn) {
        if (pDevObj->stateInt & VP880_IS_SINGLE_CHANNEL) {
            /* none of the single channel rev 2 devices have a cal circuit */
            pDevObj->stateInt &= ~VP880_HAS_CALIBRATE_CIRCUIT;
        } else {
            pDevObj->stateInt |= VP880_HAS_CALIBRATE_CIRCUIT;
        }
    } else {
        /* all other revs should have cal circuit */
        pDevObj->stateInt |= VP880_HAS_CALIBRATE_CIRCUIT;
    }

    /* Check for Relay Protection Required */
    switch(deviceRcn) {
        case VP880_DEV_PCN_88506:
        case VP880_DEV_PCN_88536:
            pDevObj->stateInt |= VP880_RLY_PROT_REQ;
            break;

        default:
            if(deviceRcn > VP880_REV_VC) {
                pDevObj->stateInt |= VP880_RLY_PROT_REQ;
            }
            break;
    }

    /* Initialize each channel */
    for (chan = 0; chan < maxChan; chan++) {
        /*
         * For Init Line to work, the device cannot be non-initialized because
         * the init line function tries to set the line state.  Therefore,
         * temporarily set the device init flag to TRUE then immediately after
         * line init, set back to FALSE until device init is complete
         */
        pLineCtx = pDevCtx->pLineCtx[chan];
        if (pLineCtx != VP_NULL) {
            pLineObj = pLineCtx->pLineObj;

            if (pLineObj->status & VP880_IS_FXO) {
                status = Vp880InitLine(pLineCtx, pFxoAcProfile, pFxoCfgProfile,
                    VP_PTABLE_NULL);
            } else {
                status = Vp880InitLine(pLineCtx, pAcProfile, pDcProfile,
                    pRingProfile);
#ifdef VP880_INCLUDE_TESTLINE_CODE
                /* initialize the calibration coeffs */
                pDevObj->calOffsets[chan].nullOffset = 0;
                pDevObj->calOffsets[chan].vabOffset = 0;
                pDevObj->calOffsets[chan].vahOffset = 0;
                pDevObj->calOffsets[chan].vbhOffset = 0;
#endif /* VP880_INCLUDE_TESTLINE_CODE */
            }

            if (status != VP_STATUS_SUCCESS) {
                pDevObj->status.state &=
                    ~(VP_DEV_INIT_IN_PROGRESS | VP_DEV_INIT_CMP);

                VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
                VP_API_FUNC_INT(None, VP_NULL, ("Vp880InitDevice Exit Error %d-",
                    status));
                return status;
            }
        }
    }

    status = VpImplementDefaultSettings(pDevCtx, VP_NULL);

    /*
     * This clears the Init Line Events and any other erroneous event that
     * may have been created due to initialization
     */
    Vp880FlushEvents(pDevCtx);

#ifdef VP880_INCLUDE_TESTLINE_CODE
    /*
     * This clears the Test structure
     */
    pDevObj->currentTest.prepared = FALSE;
    pDevObj->currentTest.testState = -1;
    pDevObj->currentTest.testId = VP_NUM_TEST_IDS;
#endif /* VP880_INCLUDE_TESTLINE_CODE */

    if (status == VP_STATUS_SUCCESS) {
        if (pDevObj->stateInt & VP880_SYS_CAL_COMPLETE) {
            VP_CALIBRATION(None, VP_NULL, ("Calibration Previously Complete: Device 0x%08lX RCN %d",
                pDevObj->stateInt, pDevObj->staticInfo.rcnPcn[VP880_RCN_LOCATION]));
            pDevObj->status.state &= ~VP_DEV_IN_CAL;
            pDevObj->status.state &= ~(VP_DEV_INIT_IN_PROGRESS);
            pDevObj->deviceEvents.response |= VP_DEV_EVID_DEV_INIT_CMP;
        } else {
#ifdef VP880_CAL_ENABLE
            if (!(pDevObj->status.state & VP_DEV_WARM_REBOOT)) {
                VP_CALIBRATION(None, VP_NULL, ("Cal Required: Device 0x%08lX RCN %d",
                    pDevObj->stateInt, pDevObj->staticInfo.rcnPcn[VP880_RCN_LOCATION]));

                pDevObj->status.state |= VP_DEV_IN_CAL;
                pDevObj->calData.calState = VP880_CAL_INIT;

                if (pDevObj->stateInt & VP880_IS_ABS) { /* Start for ABS Device */
                    if (Vp880SetCalFlags(pDevObj) == TRUE) {
                        Vp880CalCodecInt(pDevCtx);
                    } else {
                        pDevObj->status.state &= ~VP_DEV_IN_CAL;
                        pDevObj->status.state &= ~(VP_DEV_INIT_IN_PROGRESS);
                        pDevObj->deviceEvents.response |= VP_DEV_EVID_DEV_INIT_CMP;
                    }
                } else {    /* Start for Tracker Device if JE (JA 8827x) silicon */
                    if (pDevObj->staticInfo.rcnPcn[VP880_RCN_LOCATION] >= VP880_REV_JE) {
                        pDevObj->status.state |= VP_DEV_ABV_CAL;
                        Vp880CalCodecInt(pDevCtx);
                    } else {
                        pDevObj->status.state &= ~VP_DEV_IN_CAL;
                        pDevObj->status.state &= ~(VP_DEV_INIT_IN_PROGRESS);
                        pDevObj->deviceEvents.response |= VP_DEV_EVID_DEV_INIT_CMP;
                    }
                }
            } else {
                VP_CALIBRATION(None, VP_NULL, ("Warm Reboot Detected: Device 0x%08lX RCN %d",
                    pDevObj->stateInt, pDevObj->staticInfo.rcnPcn[VP880_RCN_LOCATION]));

                pDevObj->status.state &= ~VP_DEV_IN_CAL;
                pDevObj->status.state &= ~(VP_DEV_INIT_IN_PROGRESS);
                pDevObj->deviceEvents.response |= VP_DEV_EVID_DEV_INIT_CMP;

                VpMpiCmdWrapper(deviceId, VP880_EC_CH1, VP880_BAT_CALIBRATION_WRT,
                    VP880_BAT_CALIBRATION_LEN, pDevObj->calData.abvData.switcherAdjust[0]);

                VpMpiCmdWrapper(deviceId, VP880_EC_CH2, VP880_BAT_CALIBRATION_WRT,
                    VP880_BAT_CALIBRATION_LEN, pDevObj->calData.abvData.switcherAdjust[1]);
            }
#else
            VP_CALIBRATION(None, VP_NULL, ("\n\rCal Required, Disabled at Compile: Device 0x%04X RCN %d",
                pDevObj->stateInt, pDevObj->staticInfo.rcnPcn[VP880_RCN_LOCATION]));
            pDevObj->status.state &= ~VP_DEV_IN_CAL;
            pDevObj->status.state &= ~(VP_DEV_INIT_IN_PROGRESS);
            pDevObj->deviceEvents.response |= VP_DEV_EVID_DEV_INIT_CMP;
#endif
        }
    }

    /*
     * Success, Failure, or Calibration started -- we're not in "InitDevice"
     * function anymore. So normal rules apply.
     */
    pDevObj->status.state |= VP_DEV_INIT_CMP;

    VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);

    VP_API_FUNC_INT(None, VP_NULL, ("Vp880InitDevice Exit Normal: State 0x%04X Event 0x%04X-",
        pDevObj->status.state, pDevObj->deviceEvents.response));
    return status;
} /* Vp880InitDevice */

/**
 * Vp880InitLine
 *  This function initializes a line of a device with the specified parameters
 * and API default values. It is a "Line Reset".
 *
 * Preconditions:
 *  The device associated with this line must be initialized.
 *
 * Postconditions:
 *  The line pointed to be the line context passed is initialized with the
 * profile data specified.  This function returns the success code if the device
 * associated with this line is initialized.
 */
VpStatusType
Vp880InitLine(
    VpLineCtxType *pLineCtx,
    VpProfilePtrType pAcProfile,    /**< Pointer to AC coefficient data or
                                     * profile index to be applied to this line.
                                     */

    VpProfilePtrType pDcOrFxoProfile,   /**< Pointer to DC Feed (FXS) or Cfg
                                         * (FX0) profile or profile index to be
                                         * applied to this line.
                                         */

    VpProfilePtrType pRingProfile)  /**< Pointer to Ringing profile or profile
                                     * index to apply to this line
                                     */
{
    Vp880LineObjectType *pLineObj = pLineCtx->pLineObj;
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;

    uint8 channelId = pLineObj->channelId;
    uint8 ecValMap[]  = {VP880_EC_CH1, VP880_EC_CH2};
    uint8 ecVal       = ecValMap[channelId];

    uint8 alwaysOn[VP880_CADENCE_TIMER_LEN] = {0x3F, 0xFF, 0x00, 0x00};

#ifdef CSLAC_SEQ_EN
    uint8 seqByte;
#endif

    uint8 ringingParamCount;
    uint8 defaultRingParams[VP880_RINGER_PARAMS_LEN] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    VpProfileDataType dcDefaultProf[] = {
        0x00, 0x01, 0x01, 0x0A, 0x00, 0x08, 0xC2, 0x1B, 0x84, 0xB3, 0x05, 0xC6,
        0x13, 0x08
    };

    VpProfileDataType fxoDefaultProf[] =
    {
        /* FXO/Dialing Profile */
        0x00, 0xFE, 0x00, 0x12, 0x00, 0x00, 0x00, 0x27, 0x00, 0x28, 0x00, 0x78,
        0x0C, 0x08, 0x00, 0x28, 0xEB, 0x79, 0x04, 0x03, 0x26, 0x3A
    };

    /*
     * IO Direction and Control used to restore the device IO to the state
     * set prior to the channel Software Reset
     */
    uint8 ioDirection[VP880_IODIR_REG_LEN];
    uint8 ioData[VP880_IODATA_REG_LEN];
    uint8 swCal[VP880_BAT_CALIBRATION_LEN];

    VpStatusType status = VP_STATUS_SUCCESS;
    uint8 data;
    VpDeviceIdType deviceId = pDevObj->deviceId;
    uint8 fxoCidLine;

    /* Proceed if device state is either in progress or complete */
    if (pDevObj->status.state & (VP_DEV_INIT_CMP | VP_DEV_INIT_IN_PROGRESS)) {
    } else {
        return VP_STATUS_DEV_NOT_INITIALIZED;
    }

    /*
     * Do not proceed if the device calibration is in progress. This could
     * damage the device.
     */
    if (pDevObj->status.state & VP_DEV_IN_CAL) {
        return VP_STATUS_DEV_NOT_INITIALIZED;
    }

    VpSysEnterCritical(deviceId, VP_CODE_CRITICAL_SEC);
    pLineObj->ecVal = ecVal;
    pLineObj->status &= ~VP880_INIT_COMPLETE;

    /*
     * For init purposes, mark the line as uncalibrated untill provided with
     * a DC profile and either calibrated with VpCalLine() or provided
     * calibrated values with VpCal().
     */
    pLineObj->calLineData.calDone = FALSE;

#ifdef CSLAC_SEQ_EN
    for (seqByte = 0; seqByte < VP880_INT_SEQ_LEN; seqByte++) {
        pLineObj->intSequence[seqByte] = 0x00;
    }

    pLineObj->callerId.status = 0x00;
    pLineObj->suspendCid = FALSE;
#endif
    pLineObj->leakyLineCnt = 0;    /* Used only for LP Mode */

    pLineObj->status &= ~(VP880_BAD_LOOP_SUP);

    pLineObj->pRingingCadence = VP_PTABLE_NULL;
    pLineObj->pCidProfileType1 = VP_PTABLE_NULL;

    /* Initialize cached transmit and receive gains for SetRelGain to 1.0. */
    pLineObj->gain.gxInt = 0x4000;
    pLineObj->gain.grInt = 0x4000;

    /* Inititialize API line state variables */
    if (pLineObj->status & VP880_IS_FXO) {
        pLineObj->lineState.currentState = VP_LINE_FXO_LOOP_OPEN;
        pLineObj->lineState.previous = VP_LINE_FXO_LOOP_OPEN;
    } else {
        pLineObj->lineState.currentState = VP_LINE_DISCONNECT;
        pLineObj->lineState.previous = VP_LINE_DISCONNECT;
        pLineObj->lineState.usrCurrent = VP_LINE_DISCONNECT;
    }

    /* Force a line state check and update hook information */
    pLineObj->lineState.condition = VP_CSLAC_STATUS_INVALID;
    pLineObj->dpStruct.hookSt = FALSE;
    pLineObj->dpStruct2.hookSt = FALSE;

    VpInitDP(&pLineObj->dpStruct);
    VpInitDP(&pLineObj->dpStruct2);

    /*
     * Cache the ICR values to avoid read/delay/write operation and to minimize
     * device access. It's possible to have a read conflict with the device such
     * that read-modify-write would cause an error. Write only operation does not
     * have this problem.
     */
    pLineObj->icr1Values[0] = 0x00;
    pLineObj->icr1Values[1] = 0x00;
    pLineObj->icr1Values[2] = 0x00;
    pLineObj->icr1Values[3] = 0x00;

    pLineObj->icr2Values[0] = 0x00;
    pLineObj->icr2Values[1] = 0x00;
    pLineObj->icr2Values[2] = 0x00;
    pLineObj->icr2Values[3] = 0x00;

    pLineObj->icr3Values[0] = 0x00;
    pLineObj->icr3Values[1] = 0x00;
    pLineObj->icr3Values[2] = 0x00;
    pLineObj->icr3Values[3] = 0x00;

    pLineObj->icr4Values[0] = 0x00;
    pLineObj->icr4Values[1] = 0x00;
    pLineObj->icr4Values[2] = 0x00;
    pLineObj->icr4Values[3] = 0x00;

    /* It is possible that CalCodec set ABS calibration values in ICR6 before
     * this point, so read these from the device */
    VpMpiCmdWrapper(deviceId, ecVal, VP880_ICR6_RD,
        VP880_ICR6_LEN, pLineObj->icr6Values);

    /* Force a codec update */
    pLineObj->codec = VP_NUM_OPTION_CODEC_TYPE_IDS;

    pLineObj->internalTestTermApplied = FALSE;

    /*
     * Read the IO direction and data for the device IO that will be affected
     * by a software reset. Also read the calibrated battery voltage.
     */
    VpMpiCmdWrapper(deviceId, ecVal, VP880_IODIR_REG_RD,
        VP880_IODIR_REG_LEN, ioDirection);

    VpMpiCmdWrapper(deviceId, ecVal, VP880_IODATA_REG_RD,
        VP880_IODATA_REG_LEN, ioData);

    VpMpiCmdWrapper(deviceId,ecVal, VP880_BAT_CALIBRATION_RD,
        VP880_BAT_CALIBRATION_LEN, swCal);

    /*
     * Workaround for I/O1 relay driver. Problem is the I/O1 pin does not have
     * proper voltage clamps to protect against normal voltage spikes that
     * occur as a result of transitioning to Input or Open Drain on a driven
     * relay coil. So this workaround will check to see if the pin is currently
     * being used to close an external relay, and force it open. Then it has
     * to wait for 3ms for the relay to fully open (coil to fully discharge).
     */
    if ((pDevObj->stateInt & VP880_RLY_PROT_REQ)
      && ((pLineObj->termType == VP_TERM_FXS_ISOLATE)
      ||  (pLineObj->termType == VP_TERM_FXS_ISOLATE_LP)
      ||  (pLineObj->termType == VP_TERM_FXS_SPLITTER))) {

        if((pLineObj->termType == VP_TERM_FXS_ISOLATE) ||
           (pLineObj->termType == VP_TERM_FXS_ISOLATE_LP)) {
            Vp880SetRelayState(pLineCtx, VP_RELAY_NORMAL);
        } else {
            Vp880SetRelayState(pLineCtx, VP_RELAY_RESET);
        }

        /* Wait for relay coil to fully discharge */
        VpSysWait(24);
    }

    /* Software reset the channel */
    VpMpiCmdWrapper(deviceId, ecVal, VP880_SW_RESET_WRT, VP880_SW_RESET_LEN,
        &data);
    VpSysWait(3);

    /* Restore the previous device IO direction and control */
    VP_INFO(VpLineCtxType, pLineCtx, ("5. Write IODATA 0x%02X on Channel %d",
        ioData[0], channelId));

    VpMpiCmdWrapper(deviceId, ecVal, VP880_IODATA_REG_WRT,
        VP880_IODATA_REG_LEN, ioData);

    VP_INFO(VpLineCtxType, pLineCtx, ("3. Write IODIR 0x%02X on Channel %d",
        ioDirection[0], channelId));

    VpMpiCmdWrapper(deviceId, ecVal, VP880_IODIR_REG_WRT,
        VP880_IODIR_REG_LEN, ioDirection);

    VpMpiCmdWrapper(deviceId, ecVal, VP880_BAT_CALIBRATION_WRT,
        VP880_BAT_CALIBRATION_LEN, swCal);

    /*
     * Operating Conditions - Remove all loopbacks, connect TX/RX PCM Hwy
     * Note that TX/RX PCM Highway is set when Set Linestate function is
     * called.
     */
    data = VP880_NORMAL_OP_COND_MODE;
    VpMpiCmdWrapper(deviceId, ecVal, VP880_OP_COND_WRT, VP880_OP_COND_LEN,
        &data);

    /* Disable the internal device cadencer .. done in the API */
    VpMpiCmdWrapper(deviceId, ecVal, VP880_CADENCE_TIMER_WRT,
        VP880_CADENCE_TIMER_LEN, alwaysOn);

    /* Start the channel out in the standby state or loop open (if FXO)  */
    if (pLineObj->status & VP880_IS_FXO) {
        pLineObj->lineTimers.type = VP_CSLAC_FXO_TIMER;

        /* Disable auto system state control */
        data = VP880_AUTO_SSC_DIS;
        VpMpiCmdWrapper(deviceId, ecVal, VP880_SS_CONFIG_WRT,
            VP880_SS_CONFIG_LEN, &data);

        if (pLineObj->termType == VP_TERM_FXO_DISC) {
            fxoCidLine = VP880_IODATA_IO3;
        } else {
            fxoCidLine = VP880_FXO_CID_LINE;
        }

        data &= ~VP880_IODIR_IO1_MASK;
        data = (VP880_IODIR_IO1_OUTPUT | (fxoCidLine << 1));

#ifndef LEGACY_RINGING_DETECTION
        data |= VP880_IODIR_EXPDT_MASK;    /* Ringing detector on IO4 */
        VpMpiCmdWrapper(deviceId, ecVal, VP880_IODIR_REG_WRT,
            VP880_IODIR_REG_LEN, &data);
#endif

        pLineObj->digitGenStruct.dtmfOnTime = VP_FXO_DTMF_ON_DEFAULT;
        pLineObj->digitGenStruct.dtmfOffTime = VP_FXO_DTMF_OFF_DEFAULT;
        pLineObj->digitGenStruct.breakTime = VP_FXO_PULSE_BREAK_DEFAULT;
        pLineObj->digitGenStruct.makeTime = VP_FXO_PULSE_MAKE_DEFAULT;
        pLineObj->digitGenStruct.flashTime = VP_FXO_FLASH_HOOK_DEFAULT;
        pLineObj->digitGenStruct.dpInterDigitTime = VP_FXO_INTERDIG_DEFAULT;
        pLineObj->digitGenStruct.dtmfHighFreqLevel[0] = 0x1C;
        pLineObj->digitGenStruct.dtmfHighFreqLevel[1] = 0x32;
        pLineObj->digitGenStruct.dtmfLowFreqLevel[0] = 0x1C;
        pLineObj->digitGenStruct.dtmfLowFreqLevel[1] = 0x32;

        if (pDcOrFxoProfile == VP_NULL) {
            status = Vp880ConfigLine(pLineCtx, pAcProfile, fxoDefaultProf,
                VP_PTABLE_NULL);
        } else {
            status = Vp880ConfigLine(pLineCtx, pAcProfile, pDcOrFxoProfile,
                VP_PTABLE_NULL);
        }

        if (status != VP_STATUS_SUCCESS) {
            VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
            return status;
        }
        /* Activate Codec and enable Supervision */
        Vp880SetLineStateInt(pLineCtx, VP_LINE_FXO_LOOP_OPEN);
        pLineObj->lineState.usrCurrent = VP_LINE_FXO_LOOP_OPEN;
    } else {
        pLineObj->lineTimers.type = VP_CSLAC_FXS_TIMER;

        /*
         * Enable Auto Bat Switch (ABS), Disable Auto-Battery Shutdown (Tracker)
         * and disable Auto State Control (both)
         */
        data = VP880_AUTO_SSC_DIS;

        if (pDevObj->stateInt & VP880_IS_ABS) {
            /* An ABS device can't be LowPower, otherwise it will burn the line module */
            if ((pLineObj->termType == VP_TERM_FXS_LOW_PWR) ||
                (pLineObj->termType == VP_TERM_FXS_SPLITTER_LP) ||
                (pLineObj->termType == VP_TERM_FXS_ISOLATE_LP)) {
                pLineObj->termType = VP_TERM_FXS_GENERIC;
                pDevObj->stateInt &= ~VP880_LINE0_LP;
                pDevObj->stateInt &= ~VP880_LINE1_LP;
                VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
                return VP_STATUS_ERR_VTD_CODE;
            }
        }

        VpMpiCmdWrapper(deviceId, ecVal, VP880_SS_CONFIG_WRT,
            VP880_SS_CONFIG_LEN, &data);

        pLineObj->onHookTicks = -1;

        /* Complete all other non device senstive items */

        /* Initialize default values for Ringing */
        for (ringingParamCount = 0;
             ringingParamCount < VP880_RINGER_PARAMS_LEN;
             ringingParamCount++) {
            pLineObj->ringingParams[ringingParamCount] =
                defaultRingParams[ringingParamCount];
        }
        pLineObj->status &= ~(VP880_UNBAL_RINGING);

        if (pDevObj->staticInfo.rcnPcn[VP880_RCN_LOCATION] > VP880_REV_VC) {
            uint8 converterCfg[VP880_CONV_CFG_LEN];
            uint8 deviceMode[VP880_DEV_MODE_LEN];

            /* Set the pcm buffer update rate based on the tickrate */
            if(pDevObj->devProfileData.tickRate <=160) {
                converterCfg[0] = VP880_CC_8KHZ_RATE;
                pDevObj->txBufferDataRate = VP880_CC_8KHZ_RATE;

            } else if(pDevObj->devProfileData.tickRate <=320){
                converterCfg[0] = VP880_CC_4KHZ_RATE;
                pDevObj->txBufferDataRate = VP880_CC_4KHZ_RATE;

            } else if(pDevObj->devProfileData.tickRate <=640){
                converterCfg[0] = VP880_CC_2KHZ_RATE;
                pDevObj->txBufferDataRate = VP880_CC_2KHZ_RATE;

            } else if(pDevObj->devProfileData.tickRate <=1280){
                converterCfg[0] = VP880_CC_1KHZ_RATE;
                pDevObj->txBufferDataRate = VP880_CC_1KHZ_RATE;
            } else {
                converterCfg[0] = VP880_CC_500HZ_RATE;
                pDevObj->txBufferDataRate = VP880_CC_500HZ_RATE;
            }

            VpMpiCmdWrapper(deviceId, ecVal, VP880_CONV_CFG_WRT,
                VP880_CONV_CFG_LEN, converterCfg);

            VpMpiCmdWrapper(deviceId, ecVal, VP880_DEV_MODE_RD,
                VP880_DEV_MODE_LEN, deviceMode);
            deviceMode[0] |= VP880_DEV_MODE_TEST_DATA;
            VpMpiCmdWrapper(deviceId, ecVal, VP880_DEV_MODE_WRT,
                VP880_DEV_MODE_LEN, deviceMode);
        }

#ifdef VP880_CURRENT_LIMIT
        pLineObj->icr2Values[VP880_ICR2_SWY_LIM_INDEX] |= VP880_ICR2_SWY_LIM_CTRL;
        pLineObj->icr2Values[VP880_ICR2_SWY_LIM_INDEX+1] &= ~VP880_ICR2_SWY_LIM_CTRL;
#endif

        if (!(pDevObj->stateInt & VP880_IS_ABS)) {
            /*
             * Tracker Workaround:
             * SW Reset takes this back to "all zeros" so we don't need a
             * read/modify/write.
             */
            pLineObj->icr3Values[VP880_ICR3_LINE_CTRL_INDEX] = VP880_ICR3_VREF_CTRL;
            pLineObj->icr3Values[VP880_ICR3_LINE_CTRL_INDEX+1] = VP880_ICR3_VREF_CTRL;

            VpMpiCmdWrapper(deviceId, ecVal, VP880_ICR3_WRT, VP880_ICR3_LEN,
                pLineObj->icr3Values);
            VP_LINE_STATE(VpLineCtxType, pLineCtx, ("Line Init: ICR3 0x%02X 0x%02X 0x%02X 0x%02X Ch %d",
                pLineObj->icr3Values[0], pLineObj->icr3Values[1],
                pLineObj->icr3Values[2], pLineObj->icr3Values[3], channelId));

#ifndef VP880_CURRENT_LIMIT
            /* Eliminate use of 50V clamp for all conditions */
            pLineObj->icr2Values[VP880_ICR2_SWY_LIM_INDEX] |=
                (VP880_ICR2_SWY_LIM_CTRL1 | VP880_ICR2_SWY_LIM_CTRL);
            pLineObj->icr2Values[VP880_ICR2_SWY_LIM_INDEX+1] |=
                (VP880_ICR2_SWY_LIM_CTRL1);

            /* If High Voltage Device, fix clamp for 150V also */
            if (pDevObj->stateInt & VP880_IS_HIGH_VOLTAGE) {
                pLineObj->icr2Values[VP880_ICR2_SWY_LIM_INDEX+1] |=
                    (VP880_ICR2_SWY_LIM_CTRL);
            }
#endif
            if ((pLineObj->termType == VP_TERM_FXS_LOW_PWR) ||
                (pLineObj->termType == VP_TERM_FXS_ISOLATE_LP) ||
                (pLineObj->termType == VP_TERM_FXS_SPLITTER_LP)) {
                Vp880SetLPRegisters(pLineObj, TRUE);
                pLineObj->status |= VP880_LOW_POWER_EN;
            }
        }

        VP_LINE_STATE(VpLineCtxType, pLineCtx, ("Line Init: ICR2 0x%02X 0x%02X 0x%02X 0x%02X Ch %d",
            pLineObj->icr2Values[0], pLineObj->icr2Values[1],
            pLineObj->icr2Values[2], pLineObj->icr2Values[3], channelId));

        VpMpiCmdWrapper(deviceId, ecVal, VP880_ICR2_WRT, VP880_ICR2_LEN,
            pLineObj->icr2Values);

        if (pDcOrFxoProfile == VP_PTABLE_NULL) {
            status = Vp880ConfigLine(pLineCtx, pAcProfile, dcDefaultProf,
                pRingProfile);
        } else {
            status = Vp880ConfigLine(pLineCtx, pAcProfile, pDcOrFxoProfile,
                pRingProfile);
        }

        if (status != VP_STATUS_SUCCESS) {
            VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
            return status;
        }

        /* Set to Disconnect */
        Vp880SetLineStateInt(pLineCtx, VP_LINE_DISCONNECT);
        pLineObj->lineState.usrCurrent = VP_LINE_DISCONNECT;
    }

    status = VpImplementDefaultSettings(VP_NULL, pLineCtx);

    Vp880SetRelayState(pLineCtx, VP_RELAY_NORMAL);

    /* Post the line init complete event if status is succesfull */
    if (status == VP_STATUS_SUCCESS) {
        pLineObj->lineEvents.response |= VP_LINE_EVID_LINE_INIT_CMP;
        pLineObj->status |= VP880_INIT_COMPLETE;
    }

#ifdef CSLAC_SEQ_EN
    InitCadenceVars(pLineCtx);
#endif

    InitTimerVars(pLineCtx);
    if (pLineObj->status & VP880_IS_FXO) {
        pLineObj->lineTimers.timers.fxoTimer.disconnectDebounce = VP_FXO_DISCONNECT_DEBOUNCE;
    }

    VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);

    return status;
} /* Vp880InitLine */

/**
 * Vp880ConfigLine
 *  This function reloads a line of a device with the specified parameters.
 *
 * Preconditions:
 *  The device associated with this line must be initialized.
 *
 * Postconditions:
 *  The line pointed to be the line context passed is initialized with the
 * profile data specified.  This function returns the success code if the device
 * associated with this line is initialized.
 */
VpStatusType
Vp880ConfigLine(
    VpLineCtxType *pLineCtx,
    VpProfilePtrType pAcProfile,    /**< Pointer to AC coefficient data or
                                     * profile index to be applied to this line.
                                     */

    VpProfilePtrType pDcOrFxoProfile,   /**< Pointer to DC Feed (FXS) or Cfg
                                         * (FX0) profile or profile index to be
                                         * applied to this line.
                                         */

    VpProfilePtrType pRingProfile)  /**< Pointer to Ringing profile or profile
                                     * index to apply to this line
                                     */
{
    Vp880LineObjectType *pLineObj = pLineCtx->pLineObj;
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;

    uint8 ecVal = pLineObj->ecVal;
    uint8 channelId = pLineObj->channelId;
    uint8 loopSuperParams, profileIndex;
    uint8 sysStateConfig[VP880_SS_CONFIG_LEN];

    VpProfileDataType *pMpiData;

    VpProfilePtrType pAcProf = VP_PTABLE_NULL;
    VpProfilePtrType pDcFxoCfgProf = VP_PTABLE_NULL;
    VpProfilePtrType pRingProf = VP_PTABLE_NULL;

    int profIndex;
    uint8 ringTypeByte;

    uint8 data;
    VpDeviceIdType deviceId = pDevObj->deviceId;

    uint8 gainCSD[VP880_GR_GAIN_LEN];

    /*
     * Default value used if non provided. Note Ringing Detect 17-33Hz at 1/2
     * period due to 2x pulse from ringing detect input.
     */
#ifdef LEGACY_RINGING_DETECTION
    uint8 fxoLoopThreshLow[VP880_LOOP_SUP_LEN] = {0x1C, 0xE1, 0x79, 0xEB};
    uint8 fxoLoopThreshHigh[VP880_LOOP_SUP_LEN] = {0x19, 0xF4, 0x79, 0xEB};
#endif

    uint8 fxoLoopThresh[VP880_LOOP_SUP_LEN] = {0x19, 0xF4, 0x38, 0x78};

    uint8 vocThresh;
    uint8 icr5Speedup[VP880_ICR5_LEN];

    /* Proceed if device state is either in progress or complete */
    if (pDevObj->status.state & (VP_DEV_INIT_CMP | VP_DEV_INIT_IN_PROGRESS)) {
    } else {
        return VP_STATUS_DEV_NOT_INITIALIZED;
    }

    /*
     * Do not proceed if the device calibration is in progress. This could
     * damage the device.
     */
    if (pDevObj->status.state & VP_DEV_IN_CAL) {
        return VP_STATUS_DEV_NOT_INITIALIZED;
    }

    profIndex = VpGetProfileIndex(pAcProfile);
    if (profIndex < 0) {
        /*
         * A pointer is passed or VP_PTABLE_NULL.  If it's a pointer, make
         * sure the content is valid for the profile type.
         */
        if (pAcProfile != VP_PTABLE_NULL) {
            if(VpVerifyProfileType(VP_PROFILE_AC, pAcProfile) != TRUE) {
                return VP_STATUS_ERR_PROFILE;
            }
        }
        /* If we're here, it's a valid profile pointer -- even if NULL */
        pAcProf = pAcProfile;
    } else if (profIndex < VP_CSLAC_AC_PROF_TABLE_SIZE) {
        pAcProf = pDevObj->devProfileTable.pAcProfileTable[profIndex];
        if (!(pDevObj->profEntry.acProfEntry & (0x01 << profIndex))) {
            return VP_STATUS_ERR_PROFILE;
        }
    } else {
        return VP_STATUS_ERR_PROFILE;
    }

    profIndex = VpGetProfileIndex(pDcOrFxoProfile);
    if (profIndex < 0) {
        /*
         * A pointer is passed or VP_PTABLE_NULL.  If it's a pointer, make
         * sure the content is valid for the profile type.
         */
        if (pDcOrFxoProfile != VP_PTABLE_NULL) {
            if (pLineObj->status & VP880_IS_FXO) {
                if (VpVerifyProfileType(VP_PROFILE_FXO_CONFIG, pDcOrFxoProfile)
                     != TRUE) {
                    return VP_STATUS_ERR_PROFILE;
                }
            } else {
                if (VpVerifyProfileType(VP_PROFILE_DC, pDcOrFxoProfile)
                    != TRUE) {
                    return VP_STATUS_ERR_PROFILE;
                }
            }
        }
        /* If we're here, it's a valid profile pointer -- even if NULL */
        pDcFxoCfgProf = pDcOrFxoProfile;
    } else {
        if (pLineObj->status & VP880_IS_FXO) {
            if (profIndex < VP_CSLAC_FXO_CONFIG_PROF_TABLE_SIZE) {
                pDcFxoCfgProf =
                    pDevObj->devProfileTable.pFxoConfigProfileTable[profIndex];

                if (!(pDevObj->profEntry.fxoConfigProfEntry
                    & (0x01 << profIndex))) {
                    return VP_STATUS_ERR_PROFILE;
                }
            } else {
                return VP_STATUS_ERR_PROFILE;
            }

     } else {
            if (profIndex < VP_CSLAC_DC_PROF_TABLE_SIZE) {
                pDcFxoCfgProf =
                    pDevObj->devProfileTable.pDcProfileTable[profIndex];

                if (!(pDevObj->profEntry.dcProfEntry & (0x01 << profIndex))) {
                    return VP_STATUS_ERR_PROFILE;
                }
            } else {
                return VP_STATUS_ERR_PROFILE;
            }
        }
    }

    profIndex = VpGetProfileIndex(pRingProfile);
    if (profIndex < 0) {
        /*
         * A pointer is passed or VP_PTABLE_NULL.  If it's a pointer, make
         * sure the content is valid for the profile type.
         */
        if (pRingProfile != VP_PTABLE_NULL) {
            if(VpVerifyProfileType(VP_PROFILE_RING, pRingProfile) != TRUE) {
                return VP_STATUS_ERR_PROFILE;
            }
        }
        /* If we're here, it's a valid profile pointer -- even if NULL */
        pRingProf = pRingProfile;
    } else if (profIndex < VP_CSLAC_RINGING_PROF_TABLE_SIZE) {
        pRingProf = pDevObj->devProfileTable.pRingingProfileTable[profIndex];
        if (!(pDevObj->profEntry.ringingProfEntry & (0x01 << profIndex))) {
            return VP_STATUS_ERR_PROFILE;
        }
    } else {
        return VP_STATUS_ERR_PROFILE;
    }

    VpSysEnterCritical(deviceId, VP_CODE_CRITICAL_SEC);

    /* Load AC Coefficients */
    if (pAcProf != VP_PTABLE_NULL) {
        profileIndex = VP_PROFILE_MPI_LEN + 1;
        pMpiData = (VpProfileDataType *)(&pAcProfile[profileIndex]);
        VpMpiCmdWrapper(deviceId, ecVal, NOOP_CMD,
            pAcProfile[VP_PROFILE_MPI_LEN], pMpiData);

        /* Operating Functions - Use loaded coefficients */
        VpMpiCmdWrapper(deviceId, ecVal, VP880_OP_FUNC_RD,
            VP880_OP_FUNC_LEN, &data);
        data |= VP880_ENABLE_LOADED_COEFFICIENTS;
        VpMpiCmdWrapper(deviceId, ecVal, VP880_OP_FUNC_WRT,
            VP880_OP_FUNC_LEN, &data);

        /* Update cached transmit and receive gains for SetRelGain */
        VpMpiCmdWrapper(deviceId, ecVal, VP880_GX_GAIN_RD,
            VP880_GX_GAIN_LEN, gainCSD);
        pLineObj->gain.gxInt = 0x4000 + VpConvertCsd2Fixed(gainCSD);

        VpMpiCmdWrapper(deviceId, ecVal, VP880_GR_GAIN_RD,
            VP880_GR_GAIN_LEN, gainCSD);
        pLineObj->gain.grInt = VpConvertCsd2Fixed(gainCSD);
    }

    if (pLineObj->status & VP880_IS_FXO) {
        /* Configure an FXO line type */
        if (pDcFxoCfgProf != VP_PTABLE_NULL) {

#ifdef LEGACY_RINGING_DETECTION
            /*
             * Force device Ringing Detector unless the minimum frequency is
             * lower than device can support.
             */
            if (pDcFxoCfgProf[VP_FXO_DIALING_PROFILE_RING_PERIOD_MAX_ACT]
             >= VP880_MAX_RING_DET_PERIOD) {
                /* Cache the "Low Freq" loop supervision register content */
                VpMemCpy(fxoLoopThresh, fxoLoopThreshLow, VP880_LOOP_SUP_LEN);
            } else {
                /* Cache the "High Freq" loop supervision register content */
                VpMemCpy(fxoLoopThresh, fxoLoopThreshHigh, VP880_LOOP_SUP_LEN);
             }

            profileIndex = VP_FXO_DIALING_PROFILE_DISC_VOLTAGE_MIN;
            loopSuperParams = (pDcFxoCfgProf[profileIndex] << 3);
            loopSuperParams &= 0x38;
            fxoLoopThresh[0] &= ~(0x38);
            fxoLoopThresh[0] |= loopSuperParams;

            /*
             * Use the profile parameters for Ringing Detect minimum ONLY if
             * the minimum ringing detect frequency is within range of device.
             * Otherwise, force LIU/Ring Detect to 60V.
             */
            if (pDcFxoCfgProf[VP_FXO_DIALING_PROFILE_RING_PERIOD_MAX_ACT]
             < VP880_MAX_RING_DET_PERIOD) {
                profileIndex = VP_FXO_DIALING_PROFILE_RING_VOLTAGE_MIN;
                loopSuperParams =
                    (pDcFxoCfgProf[profileIndex] & VP880_LOOP_SUP_LIU_THRESH_BITS);
                fxoLoopThresh[0] &= ~(VP880_LOOP_SUP_LIU_THRESH_BITS);
                fxoLoopThresh[0] |= loopSuperParams;
            }

            fxoLoopThresh[VP880_RING_PERIOD_MIN_INDEX] =
                pDcFxoCfgProf[VP_FXO_DIALING_PROFILE_RING_PERIOD_MIN];

            /*
             * Cache the Minimum Ringing Detect Period that is implemented in
             * SW.
             */
            pLineObj->ringDetMin =
                pDcFxoCfgProf[VP_FXO_DIALING_PROFILE_RING_PERIOD_MIN];
            pLineObj->ringDetMin /= 4;

            profileIndex = VP_FXO_DIALING_PROFILE_RING_PERIOD_MAX_ACT;
            pLineObj->ringDetMax = pDcFxoCfgProf[profileIndex];

            profileIndex = VP_FXO_DIALING_PROFILE_RING_PERIOD_MAX;
            fxoLoopThresh[3] = pDcFxoCfgProf[profileIndex];
#else
            uint8 tempRegValue;

            profileIndex = VP_FXO_DIALING_PROFILE_DISC_VOLTAGE_MIN;
            loopSuperParams = (pDcFxoCfgProf[profileIndex] << 3);
            loopSuperParams &= 0x38;
            fxoLoopThresh[0] &= ~(0x38);
            fxoLoopThresh[0] |= loopSuperParams;
            fxoLoopThresh[0] |= VP880_RING_DETECT_PERIOD_ONLY;

            profileIndex = VP_FXO_DIALING_PROFILE_LIU_THRESHOLD_MIN;
            loopSuperParams =
                (pDcFxoCfgProf[profileIndex] & VP880_LOOP_SUP_LIU_THRESH_BITS);
            fxoLoopThresh[0] &= ~(VP880_LOOP_SUP_LIU_THRESH_BITS);
            fxoLoopThresh[0] |= loopSuperParams;

            profileIndex = VP_FXO_DIALING_PROFILE_RING_PERIOD_MIN;

#ifdef VP880_FXO_FULL_WAVE_RINGING
            tempRegValue = pDcFxoCfgProf[profileIndex] / 2;
#else
            tempRegValue = pDcFxoCfgProf[profileIndex];
#endif

            fxoLoopThresh[VP880_RING_PERIOD_MIN_INDEX] = tempRegValue;

            /*
             * Cache the Minimum Ringing Detect Period that is implemented in
             * SW.
             */
            pLineObj->ringDetMin = pDcFxoCfgProf[profileIndex];
            pLineObj->ringDetMin /= 4;

            profileIndex = VP_FXO_DIALING_PROFILE_RING_PERIOD_MAX_ACT;
            pLineObj->ringDetMax = pDcFxoCfgProf[profileIndex];

            profileIndex = VP_FXO_DIALING_PROFILE_RING_PERIOD_MAX;

#ifdef VP880_FXO_FULL_WAVE_RINGING
            tempRegValue = pDcFxoCfgProf[profileIndex] / 2;
#else
            tempRegValue = pDcFxoCfgProf[profileIndex];
#endif

            fxoLoopThresh[3] = tempRegValue;
#endif

            if (pLineObj->ringDetMax == 0) {
                pLineObj->ringDetMax = fxoLoopThresh[3] / 4;
            }

            profileIndex = VP_FXO_DIALING_PROFILE_DTMF_ON_MSB;
            pLineObj->digitGenStruct.dtmfOnTime =
                (pDcFxoCfgProf[profileIndex] << 8)&0xFF00;

            profileIndex = VP_FXO_DIALING_PROFILE_DTMF_ON_LSB;
            pLineObj->digitGenStruct.dtmfOnTime |=  pDcFxoCfgProf[profileIndex];

            profileIndex = VP_FXO_DIALING_PROFILE_DTMF_OFF_MSB;
            pLineObj->digitGenStruct.dtmfOffTime =
                (pDcFxoCfgProf[profileIndex] << 8)&0xFF00;

            profileIndex = VP_FXO_DIALING_PROFILE_DTMF_OFF_LSB;
            pLineObj->digitGenStruct.dtmfOffTime |= pDcFxoCfgProf[profileIndex];

            profileIndex = VP_FXO_DIALING_PROFILE_PULSE_BREAK;
            pLineObj->digitGenStruct.breakTime = pDcFxoCfgProf[profileIndex];

            profileIndex = VP_FXO_DIALING_PROFILE_PULSE_MAKE;
            pLineObj->digitGenStruct.makeTime = pDcFxoCfgProf[profileIndex];

            profileIndex = VP_FXO_DIALING_PROFILE_FLASH_HOOK_MSB;
            pLineObj->digitGenStruct.flashTime =
                (pDcFxoCfgProf[profileIndex] << 8)&0xFF00;

            profileIndex = VP_FXO_DIALING_PROFILE_FLASH_HOOK_LSB;
            pLineObj->digitGenStruct.flashTime |= pDcFxoCfgProf[profileIndex];

            profileIndex = VP_FXO_DIALING_PROFILE_INTERDIGIT_MSB;
            pLineObj->digitGenStruct.dpInterDigitTime =
                (pDcFxoCfgProf[profileIndex] << 8)&0xFF00;

            profileIndex = VP_FXO_DIALING_PROFILE_INTERDIGIT_LSB;
            pLineObj->digitGenStruct.dpInterDigitTime =
                pDcFxoCfgProf[profileIndex];

            profileIndex = VP_PROFILE_VERSION;
            if (pDcFxoCfgProf[profileIndex] >= VP_CSLAC_FXO_VERSION_DTMF_LEVEL) {
                profileIndex = VP_FXO_DIALING_PROFILE_DTMF_HIGH_LVL_MSB;
                pLineObj->digitGenStruct.dtmfHighFreqLevel[0] =
                    pDcFxoCfgProf[profileIndex];

                profileIndex = VP_FXO_DIALING_PROFILE_DTMF_HIGH_LVL_LSB;
                pLineObj->digitGenStruct.dtmfHighFreqLevel[1] =
                    pDcFxoCfgProf[profileIndex];

                profileIndex = VP_FXO_DIALING_PROFILE_DTMF_LOW_LVL_MSB;
                pLineObj->digitGenStruct.dtmfLowFreqLevel[0] =
                    pDcFxoCfgProf[profileIndex];

                profileIndex = VP_FXO_DIALING_PROFILE_DTMF_LOW_LVL_LSB;
                pLineObj->digitGenStruct.dtmfLowFreqLevel[1] =
                    pDcFxoCfgProf[profileIndex];
            } else {
                pLineObj->digitGenStruct.dtmfHighFreqLevel[0] = 0x1C;
                pLineObj->digitGenStruct.dtmfHighFreqLevel[1] = 0x32;
                pLineObj->digitGenStruct.dtmfLowFreqLevel[0] = 0x1C;
                pLineObj->digitGenStruct.dtmfLowFreqLevel[1] = 0x32;
            }

            fxoLoopThresh[VP880_LIU_DBNC_INDEX] &= ~VP880_LIU_DBNC_MASK;
            fxoLoopThresh[VP880_LIU_DBNC_INDEX] |=
                ((fxoLoopThresh[VP880_RING_PERIOD_MIN_INDEX] >> 3) & VP880_LIU_DBNC_MASK);

            if (!(fxoLoopThresh[VP880_RING_PERIOD_MIN_INDEX] & VP880_RING_PERIOD_1MS)) {
                if ((fxoLoopThresh[VP880_LIU_DBNC_INDEX] & VP880_LIU_DBNC_MASK) > 0) {
                    fxoLoopThresh[VP880_LIU_DBNC_INDEX]--;
                }
            }

            VpMpiCmdWrapper(deviceId, ecVal, VP880_LOOP_SUP_WRT,
                VP880_LOOP_SUP_LEN, fxoLoopThresh);

            /* Cache the loop supervision register content */
            VpMemCpy(pLineObj->loopSup, fxoLoopThresh, VP880_LOOP_SUP_LEN);
        }

        /* Cache this so we don't have to read it all the time */
        VpMemCpy(fxoLoopThresh, pLineObj->loopSup, VP880_LOOP_SUP_LEN);
        pLineObj->lineTimers.timers.fxoTimer.maxPeriod = fxoLoopThresh[3];
    } else {
        /* Configure an FXS line type */

        /* Ringing changed if profile passed */
        if (pRingProf != VP_PTABLE_NULL) {
            uint8 slacState;
            uint8 tempRingPr[255];
            int16 biasErr;

            /*
             * Ringing Profile May affect the system state register, so read
             * what it is before the profile, and set it back to all values
             * except what can change in the profile
             */
            VpMpiCmdWrapper(deviceId, ecVal, VP880_SS_CONFIG_RD,
                VP880_SS_CONFIG_LEN, sysStateConfig);

            profileIndex = VP_PROFILE_MPI_LEN + 1;
            pMpiData = (VpProfileDataType *)&pRingProf[profileIndex];

            VpMemCpy(tempRingPr, pMpiData, pRingProf[VP_PROFILE_MPI_LEN]);

            biasErr = (int16)((((uint16)(tempRingPr[2]) << 8) & 0xFF00) +
                ((uint16)(tempRingPr[3]) & 0x00FF));

            /* Apply the offset calibration to the BIAS */
            VpMpiCmdWrapper(deviceId, ecVal, VP880_SYS_STATE_RD,
                VP880_SYS_STATE_LEN, &slacState);
            if ((slacState & VP880_SS_POLARITY_MASK) == 0x00) {
                /* Normal polarity */
                biasErr -= ((pDevObj->vp880SysCalData.sigGenAError[channelId][0] -
                    pDevObj->vp880SysCalData.vocOffset[channelId][VP880_NORM_POLARITY]) * 16 / 10);
            } else {
                /* Reverse polarity */
                biasErr += ((pDevObj->vp880SysCalData.sigGenAError[channelId][0] -
                    pDevObj->vp880SysCalData.vocOffset[channelId][VP880_REV_POLARITY]) * 16 / 10);
            }
            tempRingPr[2] = (uint8)((biasErr >> 8) & 0x00FF);
            tempRingPr[3] = (uint8)(biasErr & 0x00FF);

            VpMpiCmdWrapper(deviceId, ecVal, NOOP_CMD,
                pRingProf[VP_PROFILE_MPI_LEN], tempRingPr);

            VpMemCpy(pLineObj->ringingParams, &pRingProf[profileIndex + 1],
                VP880_RINGER_PARAMS_LEN);

            ringTypeByte = pRingProf[VP_PROFILE_MPI_LEN +
                pRingProf[VP_PROFILE_MPI_LEN] + VP_PROFILE_RING_TYPE_OFFSET];

            pLineObj->status &= ~(VP880_UNBAL_RINGING);
            pLineObj->status |= (ringTypeByte ? VP880_UNBAL_RINGING : 0x0000);

            /*
             * Nothing in this register should be allowed to change, but the
             * Ringing profile may have changed this value to be compatible
             * with other device profiles. So correct it.
             */
            VpMpiCmdWrapper(deviceId, ecVal, VP880_SS_CONFIG_WRT,
                VP880_SS_CONFIG_LEN, sysStateConfig);
        }

        /* Set Loop Supervision and DC Feed */
        if (pDcFxoCfgProf != VP_PTABLE_NULL) {
            profileIndex = VP_PROFILE_MPI_LEN + 1;
            pMpiData = (VpProfileDataType *)&pDcFxoCfgProf[profileIndex];

            if (pDevObj->stateInt & VP880_IS_ABS) { /* ABS Workaround */
                vocThresh = VP880_VOC_51V;

                /* If the VOC set >= theshold, disable longitudinal clamps */
                if ((pDcFxoCfgProf[VP880_VOC_PROFILE_POSITION] & VP880_VOC_MASK)
                    >= vocThresh) {
                    pLineObj->icr3Values[VP880_ICR3_LINE_CTRL_INDEX]
                        &= ~VP880_ICR3_SAT_LIM_25_CTRL;
                    pLineObj->icr3Values[VP880_ICR3_LONG_UNCLAMP_INDEX]
                        |= VP880_ICR3_LONG_UNCLAMP;
                    pLineObj->icr3Values[VP880_ICR3_LONG_UNCLAMP_INDEX+1]
                        |= VP880_ICR3_LONG_UNCLAMP;
                } else {
                    pLineObj->icr3Values[VP880_ICR3_LINE_CTRL_INDEX]
                        |= VP880_ICR3_SAT_LIM_25_CTRL;
                    pLineObj->icr3Values[VP880_ICR3_LINE_CTRL_INDEX+1]
                        &= ~VP880_ICR3_SAT_LIM_25_CTRL;
                    pLineObj->icr3Values[VP880_ICR3_LONG_UNCLAMP_INDEX]
                        &= ~VP880_ICR3_LONG_UNCLAMP;
                }
            } else { /* Tracker Workaround */
                /* If the VOC set > 48V, disable longitudinal clamps */
                if ((pDcFxoCfgProf[VP880_VOC_PROFILE_POSITION] & VP880_VOC_MASK)
                    > VP880_VOC_48V) {
                    /* Disable longitudinal clamps */
                    pLineObj->icr3Values[VP880_ICR3_LONG_UNCLAMP_INDEX]
                        |= VP880_ICR3_LONG_UNCLAMP;
                    pLineObj->icr3Values[VP880_ICR3_LONG_UNCLAMP_INDEX+1]
                        |= VP880_ICR3_LONG_UNCLAMP;

                    /* Remove Workaround from other conditions */
                    pLineObj->icr3Values[VP880_ICR3_LONG_FIXED_INDEX]
                        &= ~VP880_ICR3_LONG_FIXED;
                } else {
                    /* If the VOC <= 48V, enable longitudinal clamps */
                    pLineObj->icr3Values[VP880_ICR3_LONG_FIXED_INDEX]
                        |= VP880_ICR3_LONG_FIXED;
                    pLineObj->icr3Values[VP880_ICR3_LONG_FIXED_INDEX+1]
                        &= ~VP880_ICR3_LONG_FIXED;

                    /* Remove Workaround from other conditions */
                    pLineObj->icr3Values[VP880_ICR3_LONG_UNCLAMP_INDEX]
                        &= ~VP880_ICR3_LONG_UNCLAMP;
                }
            }
            if (pDcFxoCfgProf[VP_PROFILE_VERSION] >= 1) {
                /* This profile contains a hook hysteresis value */
                pLineObj->hookHysteresis = pDcFxoCfgProf[VP880_HOOK_HYST_POSITION];
            } else {
                pLineObj->hookHysteresis = 0;
            }
            VP_LINE_STATE(VpLineCtxType, pLineCtx, ("Config Line: Channel %d: Writing ICR2 0x%02X 0x%02X 0x%02X 0x%02X",
                channelId, pLineObj->icr2Values[0], pLineObj->icr2Values[1],
                pLineObj->icr2Values[2], pLineObj->icr2Values[3]));

            VpMpiCmdWrapper(deviceId, ecVal, VP880_ICR2_WRT, VP880_ICR2_LEN,
                pLineObj->icr2Values);

            VP_LINE_STATE(VpLineCtxType, pLineCtx, ("ICR3 Workaround: 0x%02X 0x%02X 0x%02X 0x%02X Ch %d",
                pLineObj->icr3Values[0], pLineObj->icr3Values[1],
                pLineObj->icr3Values[2], pLineObj->icr3Values[3], channelId));

            VpMpiCmdWrapper(deviceId, ecVal, VP880_ICR3_WRT, VP880_ICR3_LEN,
                pLineObj->icr3Values);

            /* Cache the loop supervision register content */
            VpMemCpy(pLineObj->loopSup, &pMpiData[1], VP880_LOOP_SUP_LEN);

            /* If off-hook -> apply the hysteresis */
            if ((pLineObj->lineState.condition & VP_CSLAC_HOOK) == VP_CSLAC_HOOK) {
                if ((pMpiData[1] & VP880_LOOP_SUP_LIU_THRESH_BITS) >= pLineObj->hookHysteresis) {
                    pMpiData[1] -= pLineObj->hookHysteresis;
                } else {
                    pMpiData[1] &= ~VP880_LOOP_SUP_LIU_THRESH_BITS;
                }
            }

            /* Write the Profile Data */
            VpMpiCmdWrapper(deviceId, ecVal, NOOP_CMD,
                pDcFxoCfgProf[VP_PROFILE_MPI_LEN], pMpiData);

            if (pDevObj->staticInfo.rcnPcn[VP880_RCN_LOCATION] >= VP880_REV_JE) {
                VpMpiCmdWrapper(deviceId, ecVal, VP880_ICR5_RD,
                    VP880_ICR5_LEN, icr5Speedup);

                icr5Speedup[VP880_ICR5_FEED_HOLD_INDEX]
                    &= ~VP880_ICR5_FEED_HOLD_MASK;

                /* Device value is x + 18mA, so threshold is > 35mA */
                if ((pDcFxoCfgProf[VP880_ILA_PROFILE_POSITION] & VP880_ILA_MASK)
                    > 17) {
                    icr5Speedup[VP880_ICR5_FEED_HOLD_INDEX] |= 0xF0;
                } else {
                    icr5Speedup[VP880_ICR5_FEED_HOLD_INDEX] |= 0xA0;
                }
                VpMpiCmdWrapper(deviceId, ecVal, VP880_ICR5_WRT,
                    VP880_ICR5_LEN, icr5Speedup);
            }

            /* Copy Feed for Calibration reference */
            VP_INFO(VpLineCtxType, pLineCtx, ("Copying DC Feed Reference in VpConfigLine()"));
            VpMpiCmdWrapper(deviceId, ecVal, VP880_DC_FEED_RD,
                VP880_DC_FEED_LEN, pLineObj->calLineData.dcFeedRef);

            /*
             * Update the line object and device (dc feed register) for
             * calibrated values if possible.
             */
            Vp880UpdateCalValue(pLineCtx);
        }
    }

    VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
    return VP_STATUS_SUCCESS;
}

/*
 * Vp880UpdateCalValue()
 *  This function loads the device with calibration values provided by VpCal()
 * "Apply System Coefficient" process.
 *
 * Preconditions:
 *  System calibration values provided or calibration previously run.
 *
 * Postconditions:
 *  The device is loaded per the applied calibration values.
 */
bool
Vp880UpdateCalValue(
    VpLineCtxType *pLineCtx)
{
    Vp880LineObjectType *pLineObj = pLineCtx->pLineObj;
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpDeviceIdType deviceId = pDevObj->deviceId;

    uint8 ecVal = pLineObj->ecVal;
    uint8 channelId = pLineObj->channelId;

    bool calStatus = FALSE;

    if ((pLineObj->calLineData.dcFeedRef[0] == 0x00) || (pLineObj->status & VP880_IS_FXO)) {
        return calStatus;
    }

    /*
     * Update the target DC Feed registers used after calibration has
     * been run. So we don't need to replace VAS (0 only if calibration
     * not run, but then corrected during calibration).
     */
    pLineObj->calLineData.dcFeed[0] &= ~VP880_VOC_VALUE_MASK;
    pLineObj->calLineData.dcFeed[0] |=
        (pLineObj->calLineData.dcFeedRef[0] & VP880_VOC_VALUE_MASK);

    pLineObj->calLineData.dcFeedPr[0] &= ~VP880_VOC_VALUE_MASK;
    pLineObj->calLineData.dcFeedPr[0] |=
        (pLineObj->calLineData.dcFeedRef[0] & VP880_VOC_VALUE_MASK);

    pLineObj->calLineData.dcFeedPr[1] &= ~VP880_ILA_MASK;
    pLineObj->calLineData.dcFeedPr[1] |=
        (pLineObj->calLineData.dcFeedRef[1] & VP880_ILA_MASK);

    pLineObj->calLineData.dcFeed[1] &= ~VP880_ILA_MASK;
    pLineObj->calLineData.dcFeed[1] |=
        (pLineObj->calLineData.dcFeedRef[1] & VP880_ILA_MASK);

    /*
     * Adjust for the errors if previously calibrated. The ILA function
     * returns "TRUE" if an adjustment was made meaning calibration
     * done previously, FALSE if not.
     */
    if (Vp880AdjustIla(pLineCtx, (pLineObj->calLineData.dcFeedRef[1] & VP880_ILA_MASK)) == TRUE) {
        uint8 currentStateByte[VP880_SYS_STATE_LEN];
        uint16 vasVoltScale;
        calStatus = TRUE;

        pLineObj->calLineData.calDone = TRUE;

        VpMpiCmdWrapper(deviceId, ecVal, VP880_SYS_STATE_RD, VP880_SYS_STATE_LEN,
            currentStateByte);

        Vp880AdjustVoc(pLineCtx, ((pLineObj->calLineData.dcFeedRef[0] >> 2) & 0x7), TRUE);

        if (!(pDevObj->stateInt & VP880_IS_ABS)) { /* Tracker only */
            /* Set VAS to device calibrated values */
            vasVoltScale = (3000 + (uint16)pDevObj->vp880SysCalData.vas[channelId][VP880_NORM_POLARITY] * 750);
            VpCSLACSetVas(pLineObj->calLineData.dcFeed, vasVoltScale);

            vasVoltScale = (3000 + (uint16)pDevObj->vp880SysCalData.vas[channelId][VP880_REV_POLARITY] * 750);
            VpCSLACSetVas(pLineObj->calLineData.dcFeedPr, vasVoltScale);

            /*
             * BatteryCalAdjust() takes care of the device object wideband 'OR'
             * operation to the provided ec value as needed.
             */
            Vp880BatteryCalAdjust(pDevObj, VP880_EC_CH1);
            Vp880BatteryCalAdjust(pDevObj, VP880_EC_CH2);
        } else {
            VpLineStateType lineState;
            int16 targetVoltY, targetVoltZ;

            VpGetLineState(pLineCtx, &lineState);
            Vp880GetLineStateABS(pLineCtx, lineState);

            /* Compute Errors and make corrections */
            targetVoltY = (pDevObj->swParams[VP880_SWY_LOCATION] & VP880_VOLTAGE_MASK);
            targetVoltZ = (pDevObj->swParams[VP880_SWZ_LOCATION] & VP880_VOLTAGE_MASK);

            Vp880AbvMakeAdjustment(pDevObj, &targetVoltY, &targetVoltZ);
        }

        if (currentStateByte[0] & VP880_SS_POLARITY_MASK) {
            VpMpiCmdWrapper(deviceId, ecVal, VP880_DC_FEED_WRT,
                VP880_DC_FEED_LEN, pLineObj->calLineData.dcFeedPr);
        } else {
            VpMpiCmdWrapper(deviceId, ecVal, VP880_DC_FEED_WRT,
                VP880_DC_FEED_LEN, pLineObj->calLineData.dcFeed);
        }
    }
    return calStatus;
}

/**
 * Vp880InitProfile()
 *  This function is used to initialize profile tables in Vp880.
 *
 * Preconditions:
 *  The device associated with this line must be initialized.
 *
 * Postconditions:
 *  Stores the given profile at the specified index of the profile table.
 */
VpStatusType
Vp880InitProfile(
    VpDevCtxType *pDevCtx,
    VpProfileType type,
    VpProfilePtrType pProfileIndex,
    VpProfilePtrType pProfile)
{
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpDeviceIdType deviceId = pDevObj->deviceId;
    VpStatusType status = VP_STATUS_SUCCESS;

    uint8 profIndex8;   /* Used for 8-bit profile table masking */
    uint16 profIndex16; /* Used for 16-bit profile table masking */

    /*
     * If the profile data is an index, indicated by Get Profile Index return
     * value of > -1, return an error (cannot init an indexed entry with an
     * index).
     */
    int profileIndex = VpGetProfileIndex(pProfile);

    if (profileIndex >= 0) {
        return VP_STATUS_INVALID_ARG;
    }

    /*
     * If pProfileIndex is -1, the profile is of pointer type and invalid,
     * otherwise it is an index.  If it's an index, make sure the range is
     * valid.
     */
    profileIndex = VpGetProfileIndex(pProfileIndex);
    if (profileIndex < 0) {
        return VP_STATUS_INVALID_ARG;
    }

    profIndex8 = (uint8)profileIndex;
    profIndex16 = (uint16)profileIndex;

    VpSysEnterCritical(deviceId, VP_CODE_CRITICAL_SEC);
    /*
     * The correct types are passed, but check to make sure the specific profile
     * type being initialized is valid as well as the index value
     */
    switch(type) {
        case VP_PROFILE_DEVICE:
            if (profIndex8 >= VP_CSLAC_DEV_PROF_TABLE_SIZE) {
                status = VP_STATUS_INVALID_ARG;
            } else {
                if(VpVerifyProfileType(VP_PROFILE_DEVICE, pProfile) == TRUE) {
                    pDevObj->devProfileTable.pDevProfileTable[profIndex8] =
                        pProfile;
                    /*
                     * If the profile is null, then clear the flag in the
                     * profile entry table to indicate that this profile is no
                     * longer valid.
                     */
                    if (pProfile == VP_PTABLE_NULL) {
                        pDevObj->profEntry.devProfEntry &=
                            ~(0x01 << profIndex8);
                    } else {
                        pDevObj->profEntry.devProfEntry |=
                            (0x01 << profIndex8);
                    }
                } else {
                    status = VP_STATUS_ERR_PROFILE;
                }
            }
            break;

        case VP_PROFILE_AC:
            if (profIndex8 >= VP_CSLAC_AC_PROF_TABLE_SIZE) {
                status = VP_STATUS_INVALID_ARG;
            } else {
                if(VpVerifyProfileType(VP_PROFILE_AC, pProfile) == TRUE) {
                    pDevObj->devProfileTable.pAcProfileTable[profIndex8] =
                        pProfile;
                    /*
                     * If the profile is null, then clear the flag in the
                     * profile entry table to indicate that this profile is no
                     * longer valid.
                     */
                    if (pProfile == VP_PTABLE_NULL) {
                        pDevObj->profEntry.acProfEntry &=
                            ~(0x01 << profIndex8);
                    } else {
                        pDevObj->profEntry.acProfEntry |=
                            (0x01 << profIndex8);
                    }
                } else {
                    status = VP_STATUS_ERR_PROFILE;
                }
            }
            break;

        case VP_PROFILE_DC:
            if (profIndex8 >= VP_CSLAC_DC_PROF_TABLE_SIZE) {
                status = VP_STATUS_INVALID_ARG;
            } else {
                if(VpVerifyProfileType(VP_PROFILE_DC, pProfile) == TRUE) {
                    pDevObj->devProfileTable.pDcProfileTable[profIndex8] =
                        pProfile;
                    /*
                     * If the profile is null, then clear the flag in the
                     * profile entry table to indicate that this profile is no
                     * longer valid.
                     */
                    if (pProfile == VP_PTABLE_NULL) {
                        pDevObj->profEntry.dcProfEntry &=
                            ~(0x01 << profIndex8);
                    } else {
                        pDevObj->profEntry.dcProfEntry |=
                            (0x01 << profIndex8);
                    }
                } else {
                    status = VP_STATUS_ERR_PROFILE;
                }
            }
            break;

        case VP_PROFILE_RING:
            if (profIndex8 >= VP_CSLAC_RINGING_PROF_TABLE_SIZE) {
                status = VP_STATUS_INVALID_ARG;
            } else {
                if(VpVerifyProfileType(VP_PROFILE_RING, pProfile) == TRUE) {
                    pDevObj->devProfileTable.pRingingProfileTable[profIndex8] =
                        pProfile;
                    /*
                     * If the profile is null, then clear the flag in the
                     * profile entry table to indicate that this profile is no
                     * longer valid.
                     */
                    if (pProfile == VP_PTABLE_NULL) {
                        pDevObj->profEntry.ringingProfEntry &=
                            ~(0x01 << profIndex8);
                    } else {
                        pDevObj->profEntry.ringingProfEntry |=
                            (0x01 << profIndex8);
                    }
                } else {
                    status = VP_STATUS_ERR_PROFILE;
                }
            }
            break;

        case VP_PROFILE_RINGCAD:
            if (profIndex8 >= VP_CSLAC_RING_CADENCE_PROF_TABLE_SIZE) {
                status = VP_STATUS_INVALID_ARG;
            } else {
                if(VpVerifyProfileType(VP_PROFILE_RINGCAD, pProfile) == TRUE) {
                    pDevObj->devProfileTable.pRingingCadProfileTable[profIndex8] =
                        pProfile;
                    /*
                     * If the profile is null, then clear the flag in the
                     * profile entry table to indicate that this profile is no
                     * longer valid.
                     */
                    if (pProfile == VP_PTABLE_NULL) {
                        pDevObj->profEntry.ringCadProfEntry &=
                            ~(0x01 << profIndex8);
                    } else {
                        pDevObj->profEntry.ringCadProfEntry |=
                            (0x01 << profIndex8);
                    }
                } else {
                    status = VP_STATUS_ERR_PROFILE;
                }
            }
            break;

        case VP_PROFILE_TONE:
            if (profIndex16 >= VP_CSLAC_TONE_PROF_TABLE_SIZE) {
                status = VP_STATUS_INVALID_ARG;
            } else {
                if(VpVerifyProfileType(VP_PROFILE_TONE, pProfile) == TRUE) {
                    pDevObj->devProfileTable.pToneProfileTable[profIndex16] =
                        pProfile;
                    /*
                     * If the profile is null, then clear the flag in the
                     * profile entry table to indicate that this profile is no
                     * longer valid.
                     */
                    if (pProfile == VP_PTABLE_NULL) {
                        pDevObj->profEntry.toneProfEntry &=
                            ~(0x01 << profIndex16);
                    } else {
                        pDevObj->profEntry.toneProfEntry |=
                            (0x01 << profIndex16);
                    }
                } else {
                    status = VP_STATUS_ERR_PROFILE;
                }
            }
            break;

        case VP_PROFILE_TONECAD:
            if (profIndex16 >= VP_CSLAC_TONE_CADENCE_PROF_TABLE_SIZE) {
                status = VP_STATUS_INVALID_ARG;
            } else {
                if(VpVerifyProfileType(VP_PROFILE_TONECAD, pProfile) == TRUE) {
                    pDevObj->devProfileTable.pToneCadProfileTable[profIndex16] =
                        pProfile;
                    /*
                     * If the profile is null, then clear the flag in the
                     * profile entry table to indicate that this profile is no
                     * longer valid.
                     */
                    if (pProfile == VP_PTABLE_NULL) {
                        pDevObj->profEntry.toneCadProfEntry &=
                            ~(0x01 << profIndex16);
                    } else {
                        pDevObj->profEntry.toneCadProfEntry |=
                            (0x01 << profIndex16);
                    }
                } else {
                    status = VP_STATUS_ERR_PROFILE;
                }
            }
            break;

        case VP_PROFILE_METER:
            if (profIndex8 >= VP_CSLAC_METERING_PROF_TABLE_SIZE) {
                status = VP_STATUS_INVALID_ARG;
            } else {
                if(VpVerifyProfileType(VP_PROFILE_METER, pProfile) == TRUE) {
                    pDevObj->devProfileTable.pMeteringProfileTable[profIndex8] =
                        pProfile;
                    /*
                     * If the profile is null, then clear the flag in the
                     * profile entry table to indicate that this profile is no
                     * longer valid.
                     */
                    if (pProfile == VP_PTABLE_NULL) {
                        pDevObj->profEntry.meterProfEntry &=
                            ~(0x01 << profIndex8);
                    } else {
                        pDevObj->profEntry.meterProfEntry |=
                            (0x01 << profIndex8);
                    }
                } else {
                    status = VP_STATUS_ERR_PROFILE;
                }
            }
            break;

        case VP_PROFILE_CID:
            if (profIndex8 >= VP_CSLAC_CALLERID_PROF_TABLE_SIZE) {
                status = VP_STATUS_INVALID_ARG;
            } else {
                if(VpVerifyProfileType(VP_PROFILE_CID, pProfile) == TRUE) {
                    pDevObj->devProfileTable.pCallerIdProfileTable[profIndex8] =
                        pProfile;
                    /*
                     * If the profile is null, then clear the flag in the
                     * profile entry table to indicate that this profile is no
                     * longer valid.
                     */
                    if (pProfile == VP_PTABLE_NULL) {
                        pDevObj->profEntry.cidCadProfEntry &=
                            ~(0x01 << profIndex8);
                    } else {
                        pDevObj->profEntry.cidCadProfEntry |=
                            (0x01 << profIndex8);
                    }
                } else {
                    status = VP_STATUS_ERR_PROFILE;
                }
            }
            break;

        case VP_PROFILE_FXO_CONFIG:
            if (profIndex8 >= VP_CSLAC_FXO_CONFIG_PROF_TABLE_SIZE) {
                status = VP_STATUS_INVALID_ARG;
            } else {
                if(VpVerifyProfileType(VP_PROFILE_FXO_CONFIG, pProfile) == TRUE) {
                    pDevObj->devProfileTable.pFxoConfigProfileTable[profIndex8] =
                        pProfile;
                    /*
                     * If the profile is null, then clear the flag in the
                     * profile entry table to indicate that this profile is no
                     * longer valid.
                     */
                    if (pProfile == VP_PTABLE_NULL) {
                        pDevObj->profEntry.fxoConfigProfEntry &=
                            ~(0x01 << profIndex8);
                    } else {
                        pDevObj->profEntry.fxoConfigProfEntry |=
                            (0x01 << profIndex8);
                    }
                } else {
                    status = VP_STATUS_ERR_PROFILE;
                }
            }
            break;

        default:
            status = VP_STATUS_INVALID_ARG;
            break;
    }

    VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);

    return status;
} /* Vp880InitProfile() */

/**
 * Vp880FreeRun()
 *  This function is called by the application when it wants to prepare the
 * system for a restart, or by the VP-API-II internally when a clock fault or
 * other "sign" of a restart is detected.
 *
 * Preconditions:
 *  Conditions defined by purpose of Api Tick.
 *
 * Postconditions:
 *  Device and line are in states 'ready" for a system reboot to occur. Lines
 * are set to VP_LINE_STANDBY if previously ringing.
 */
VpStatusType
Vp880FreeRun(
    VpDevCtxType *pDevCtx,
    VpFreeRunModeType freeRunMode)
{
    VpLineCtxType *pLineCtx;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    Vp880LineObjectType *pLineObj;
    VpDeviceIdType deviceId = pDevObj->deviceId;

    uint8 maxChan = pDevObj->staticInfo.maxChannels;

    uint8 ecVal = pDevObj->ecVal;
    VpLineStateType lineState;
    uint8 powerMode[VP880_REGULATOR_CTRL_LEN];

    uint8 channelId;

    VP_API_FUNC(VpDevCtxType, pDevCtx, ("Vp880FreeRun Mode %d", freeRunMode));

    VpSysEnterCritical(deviceId, VP_CODE_CRITICAL_SEC);

    /*
     * Time value is passed in 500us increment. If timeOut = 0, only PCLK
     * recovery exits restart prepare operations. If less than one tick, force
     * a one tick timeout.
     */
    if (freeRunMode == VP_FREE_RUN_STOP) {
        Vp880RestartComplete(pDevCtx);
        /*
         * Clear the device as being forced into free run mode by application.
         * This allows PCLK fault detection to automatically enter/exit free
         * run mode.
         */
        pDevObj->stateInt &= ~VP880_FORCE_FREE_RUN;

        VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
        return VP_STATUS_SUCCESS;
    }

    /* Take the lines out of Ringing if necessary */
    for (channelId = 0; channelId < maxChan; channelId++) {
        pLineCtx = pDevCtx->pLineCtx[channelId];
        if (pLineCtx != VP_NULL) {
            pLineObj = pLineCtx->pLineObj;

            if (pLineObj->status & VP880_LINE_IN_CAL) {
                lineState = pLineObj->calLineData.usrState;
            } else {
                lineState = pLineObj->lineState.usrCurrent;
            }
            if (lineState == VP_LINE_RINGING) {
                Vp880SetLineState(pLineCtx, VP_LINE_STANDBY);
            }
            if (lineState == VP_LINE_RINGING_POLREV) {
                Vp880SetLineState(pLineCtx, VP_LINE_STANDBY_POLREV);
            }
        }
    }

    /*
     * Load the free run timing, if available. Otherwise just force the switcher
     * to HP mode and take control.
     */
    if (pDevObj->intSwParamsFR[0] != 0x00) {
        VpMpiCmdWrapper(deviceId, ecVal, VP880_INT_SWREG_PARAM_WRT,
            VP880_INT_SWREG_PARAM_LEN, pDevObj->intSwParamsFR);
    } else {
        /* Force control of the power mode */
        pDevObj->swParams[VP880_SWY_AUTOPOWER_INDEX] |= VP880_SWY_AUTOPOWER_DIS;
        pDevObj->swParams[VP880_SWZ_AUTOPOWER_INDEX] |= VP880_SWZ_AUTOPOWER_DIS;

        VpMpiCmdWrapper(deviceId, pDevObj->ecVal, VP880_REGULATOR_PARAM_WRT,
            VP880_REGULATOR_PARAM_LEN, pDevObj->swParams);

        /* Change the Switchers to High Power Mode */
        powerMode[0] = VP880_SWY_HP | VP880_SWZ_HP;
        VpMpiCmdWrapper(deviceId, ecVal, VP880_REGULATOR_CTRL_WRT,
            VP880_REGULATOR_CTRL_LEN, powerMode);
    }

    /*
     * Mark the device as being forced into free run mode by application. This
     * prevents auto-recovery when PCLK is restored.
     */
    pDevObj->stateInt |= VP880_FORCE_FREE_RUN;

    VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
    return VP_STATUS_SUCCESS;
}

/**
 * Vp880RestartComplete()
 *  This function is called by the VP-API-II internally when a clock fault is
 * removed.
 *
 * Preconditions:
 *  Conditions defined by purpose of Api Tick.
 *
 * Postconditions:
 *  Device and line are in states recovered from a reboot.
 */
void
Vp880RestartComplete(
    VpDevCtxType *pDevCtx)
{
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpDeviceIdType deviceId = pDevObj->deviceId;
    uint8 ecVal = pDevObj->ecVal;
    uint8 powerMode[VP880_REGULATOR_CTRL_LEN];

    /*
     * Restore original timing if they were changed, otherwise change back the
     * power mode and relinquish control.
     */
    if (pDevObj->intSwParamsFR[0] != 0x00) {
        /* Restore the original timings */
        VpMpiCmdWrapper(deviceId, ecVal, VP880_INT_SWREG_PARAM_WRT,
            VP880_INT_SWREG_PARAM_LEN, pDevObj->intSwParams);
    } else {
        /* Change the Switchers to the original Power Mode */
        if (pDevObj->stateInt & VP880_IS_ABS) {
            powerMode[0] = VP880_SWY_MP | VP880_SWY_MP;
        } else {
            powerMode[0] = VP880_SWY_LP | VP880_SWZ_LP;
        }
        VpMpiCmdWrapper(deviceId, ecVal, VP880_REGULATOR_CTRL_WRT,
            VP880_REGULATOR_CTRL_LEN, powerMode);

        /* Relinquish control of the power mode */
        pDevObj->swParams[VP880_SWY_AUTOPOWER_INDEX] &= ~VP880_SWY_AUTOPOWER_DIS;
        pDevObj->swParams[VP880_SWZ_AUTOPOWER_INDEX] &= ~VP880_SWZ_AUTOPOWER_DIS;

        VpMpiCmdWrapper(deviceId, pDevObj->ecVal, VP880_REGULATOR_PARAM_WRT,
            VP880_REGULATOR_PARAM_LEN, pDevObj->swParams);
    }
}

static void
Vp880CopyDefaultFRProfile(
    Vp880DeviceObjectType *pDevObj)
{
    /*
     * The 1st timing value can't be 0x00, so it's a marker value -> no profile
     * available
     */
    uint8 intSwParamsFR_DFLT[VP880_INT_SWREG_PARAM_LEN] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    /* ABS free run default profile */
    uint8 intSwParamsFR_ABS[VP880_INT_SWREG_PARAM_LEN] = {
        0x2C, 0x40, 0x2C, 0x40, 0x2C, 0x40
    };

    if (pDevObj->stateInt & VP880_IS_ABS) {
        VpMemCpy(pDevObj->intSwParamsFR, intSwParamsFR_ABS, VP880_INT_SWREG_PARAM_LEN);
    } else {
        VpMemCpy(pDevObj->intSwParamsFR, intSwParamsFR_DFLT, VP880_INT_SWREG_PARAM_LEN);
    }
}

#endif

