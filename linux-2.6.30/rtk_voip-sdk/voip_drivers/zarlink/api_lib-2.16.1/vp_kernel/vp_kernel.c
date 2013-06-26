/** \file vp_kernel.c
 * vp_kernel.c
 *
 *  This file contains the implementation of the VP-API to the VP-API Kernel
 * driver.
 *
 * Copyright (c) 2010, Zarlink Semiconductor, Inc.
 *
 * $Revision: 6493 $
 * $LastChangedDate: 2010-02-19 14:56:38 -0600 (Fri, 19 Feb 2010) $
 */

/* INCLUDES */
#include    "vp_api.h"

#if defined (VP_CC_KWRAP)  /* Compile only if required */

#include    "vpapi_mod.h"
#include    "vp_kernel.h"
#include    "vp_api_int.h"
#include    "boot_common.h"

/*******************************************************************************
 *              Linux Kwrap specific API function prototypes
 ******************************************************************************/
/* Initialization functions */
static VpStatusType KWrapBootLoad(
    VpDevCtxType        *pDevCtx,
    VpBootStateType     state,
    VpImagePtrType      pImageBuffer,
    uint32              bufferSize,
    VpScratchMemType    *pScratchMem,
    VpBootModeType      validation);

static VpStatusType KWrapBootSlac(
    VpLineCtxType       *pLineCtx,
    VpDevCtxType        *pDevCtx,
    VpImagePtrType      pImageBuffer,
    uint32              bufferSize);

static VpStatusType KWrapInitDevice(
    VpDevCtxType        *pDevCtx,
    VpProfilePtrType    pDevProfile,
    VpProfilePtrType    pAcProfile,
    VpProfilePtrType    pDcProfile,
    VpProfilePtrType    pRingProfile,
    VpProfilePtrType    pFxoAcProfile,
    VpProfilePtrType    pFxoCfgProfile);

static VpStatusType KWrapInitLine(
    VpLineCtxType       *pLineCtx,
    VpProfilePtrType    pAcProfile,
    VpProfilePtrType    pDcProfile,
    VpProfilePtrType    pRingProfile);

static VpStatusType KWrapInitSlac(
    VpLineCtxType       *pLineCtx,
    VpProfilePtrType    pDevProfile,
    VpProfilePtrType    pAcProfile,
    VpProfilePtrType    pDcProfile,
    VpProfilePtrType    pRingProfile);

static VpStatusType KWrapConfigLine(
    VpLineCtxType       *pLineCtx,
    VpProfilePtrType    pAcProfile,
    VpProfilePtrType    pDcOrFxoProfile,
    VpProfilePtrType    pRingProfile);

static VpStatusType KWrapCalCodec(
    VpLineCtxType       *pLineCtx,
    VpDeviceCalType     mode);

static VpStatusType KWrapCalLn(
    VpLineCtxType       *pLineCtx);

static VpStatusType KWrapCal(
    VpLineCtxType       *pLineCtx,
    VpCalType           calType,
    void                *inputArgs);

static VpStatusType KWrapFreeRun(
    VpDevCtxType       *pDevCtx,
    VpFreeRunModeType  freeRunMode);

static VpStatusType KWrapInitMeter(
    VpLineCtxType       *pLineCtx,
    VpProfilePtrType    pMeterProfile);

static VpStatusType KWrapInitCustomTerm(
    VpDevCtxType        *pDevCtx,
    VpLineCtxType       *pLineCtx,
    VpProfilePtrType    pCustomTermProfile);

static VpStatusType KWrapInitProfile(
    VpDevCtxType        *pDevCtx,
    VpProfileType       type,
    VpProfilePtrType    pProfileIndex,
    VpProfilePtrType    pProfile);

static VpStatusType KWrapSoftReset(
    VpDevCtxType        *pDevCtx);

static VpStatusType KWrapSetBatteries(
    VpLineCtxType       *pLineCtx,
    VpBatteryModeType   battMode,
    VpBatteryValuesType *pBatt);

/* VP-API Control Functions */
static VpStatusType KWrapSetLineState(
    VpLineCtxType       *pLineCtx,
    VpLineStateType     state);

static VpStatusType KWrapSetLineTone(
    VpLineCtxType       *pLineCtx,
    VpProfilePtrType    pToneProfile,
    VpProfilePtrType    pCadProfile,
    VpDtmfToneGenType   *pDtmfControl);

static VpStatusType KWrapSetRelayState(
    VpLineCtxType       *pLineCtx,
    VpRelayControlType  rState);

static VpStatusType KWrapSetRelGain(
    VpLineCtxType       *pLineCtx,
    uint16              txLevel,
    uint16              rxLevel,
    uint16              handle);

static VpStatusType KWrapSendSignal(
    VpLineCtxType       *pLineCtx,
    VpSendSignalType    signalType,
    void                *pSignalData);

static VpStatusType KWrapDtmfDigitDet(
    VpLineCtxType       *pLineCtx,
    VpDigitType         digit,
    VpDigitSenseType    sense);

static VpStatusType KWrapStartMeter(
    VpLineCtxType       *pLineCtx,
    uint16              onTime,
    uint16              offTime,
    uint16              numMeters);

static VpStatusType KWrapStartMeter32Q(
    VpLineCtxType       *pLineCtx,
    uint32              minDelay,
    uint32              onTime,
    uint32              offTime,
    uint16              numMeters,
    uint16              eventRate);

static VpStatusType KWrapDeviceIoAccess(
    VpDevCtxType                *pDevCtx,
    VpDeviceIoAccessDataType    *pDeviceIoData);

static VpStatusType KWrapDeviceIoAccessExt(
    VpDevCtxType                *pDevCtx,
    VpDeviceIoAccessExtType     *pDeviceIoAccess);

static VpStatusType KWrapSelfTest(
    VpLineCtxType       *pLineCtx);

static VpStatusType KWrapFillTestBuf(
    VpLineCtxType       *pLineCtx,
    uint16              length,
    VpVectorPtrType     pData);

static VpStatusType KWrapLowLevelCmd(
    VpLineCtxType       *pLineCtx,
    uint8               *pCmdData,
    uint8               len,
    uint16              handle);

static VpStatusType KWrapLowLevelCmd16(
    VpLineCtxType       *pLineCtx,
    VpLowLevelCmdType   cmdType,
    uint16              *writeWords,
    uint8               numWriteWords,
    uint8               numReadWords,
    uint16              handle);

static VpStatusType KWrapSetBFilter(
    VpLineCtxType       *pLineCtx,
    VpBFilterModeType   bFiltMode,
    VpProfilePtrType    pAcProfile);

static VpStatusType KWrapLineIoAccess(
    VpLineCtxType       *pLineCtx,
    VpLineIoAccessType  *pLineIoAccess,
    uint16 handle);

static VpStatusType KWrapGenTimerCtrl(
    VpLineCtxType       *pLineCtx,
    VpGenTimerCtrlType  timerCtrl,
    uint32              duration,
    uint16              handle);


/* VP-API Status and Query Functions */
static bool KWrapGetEvent(
    VpDevCtxType        *pDevCtx,
    VpEventType         *pEvent);

static VpStatusType KWrapGetLineStatus(
    VpLineCtxType       *pLineCtx,
    VpInputType         input,
    bool                *pStatus);

static VpStatusType KWrapGetDeviceStatus(
    VpDevCtxType        *pDevCtx,
    VpInputType         input,
    uint32              *pDeviceStatus);

static VpStatusType KWrapGetDeviceStatusExt(
    VpDevCtxType        *pDevCtx,
    VpDeviceStatusType  *pDeviceStatus);

static VpStatusType KWrapGetLoopCond(
    VpLineCtxType       *pLineCtx,
    uint16              handle);

static VpStatusType KWrapGetOption(
    VpLineCtxType       *pLineCtx,
    VpDevCtxType        *pDevCtx,
    VpOptionIdType      option,
    uint16              handle);

static VpStatusType KWrapGetLineState(
    VpLineCtxType       *pLineCtx,
    VpLineStateType     *pCurrentState);

static VpStatusType KWrapFlushEvents(
    VpDevCtxType        *pDevCtx);

static VpStatusType KWrapGetResults(
    VpEventType         *pEvent,
    void                *pResults);

static VpStatusType KWrapClearResults(
    VpDevCtxType        *pDevCtx);

static VpStatusType KWrapCodeCheckSum(
    VpDevCtxType        *pDevCtx,
    uint16              handle);

static VpStatusType KWrapQuery(
    VpLineCtxType       *pLineCtx,
    VpQueryIdType       queryId,
    uint16              handle);

static VpStatusType KWrapGetRelayState(
    VpLineCtxType *pLineCtx,
    VpRelayControlType *pRstate);

static VpStatusType KWrapReadTestBuf(
    VpLineCtxType       *pLineCtx,
    uint16              length,
    VpVectorPtrType     pData);

static VpStatusType KWrapRegisterDump(
    VpDevCtxType        *pDevCtx);


/* VP-API Line Tests Functions */
static VpStatusType KWrapTestLine(
    VpLineCtxType       *pLineCtx,
    VpTestIdType        test,
    const void          *pArgs,
    uint16              handle);


EXTERN VpStatusType KWrapGetDevLineSummary(
    VpApiModRegSumInfoType *pSummaryInfo)
{
    int fd = open(VPAPI_MOD_FILE_DES, O_RDWR);
    if (fd == -1) {
        return VP_STATUS_FAILURE;
    }

    /* need to add the device to the driver */
    if (ioctl(fd, VPAPI_MOD_IOG_GET_REG_SUMMARY, pSummaryInfo) < 0) {
        close(fd);
        return VP_STATUS_FAILURE;
    }

    close(fd);
    return pSummaryInfo->status;
}

EXTERN VpStatusType KWrapGetLineIdInfo(
    const VpLineIdType      lineId,
    VpKWrapLineIdInfoType   *pLineIdInfo)
{
    VpApiModGetLineIdInfoType ioLineIdInfo = {
        .lineId = lineId
    };

    int fd = open(VPAPI_MOD_FILE_DES, O_RDWR);
    if (fd == -1) {
        return VP_STATUS_FAILURE;
    }

    /* need to add the device to the driver */
    if (ioctl(fd, VPAPI_MOD_IOX_GET_LINE_ID_INFO, &ioLineIdInfo) < 0) {
        close(fd);
        return VP_STATUS_FAILURE;
    }

    close(fd);

    pLineIdInfo->termType     = ioLineIdInfo.termType;
    pLineIdInfo->channelId    = ioLineIdInfo.channelId;
    pLineIdInfo->deviceType   = ioLineIdInfo.deviceType;
    pLineIdInfo->deviceId     = ioLineIdInfo.deviceId;
    return ioLineIdInfo.status;
}


EXTERN VpStatusType KWrapCloseDevice(
    VpDevCtxType            *pDevCtx)
{
    VpKWrapDeviceObjectType *pDevObj = NULL;

    if (pDevCtx == VP_NULL) {
        return VP_STATUS_INVALID_ARG;
    }

    pDevObj = pDevCtx->pDevObj;

    if (close(pDevObj->fileDes) < 0) {
        return VP_STATUS_FAILURE;
    }

    return VP_STATUS_SUCCESS;
}

EXTERN VpStatusType KWrapOpenDevice(
    const VpDeviceType      deviceType,
    const VpDeviceIdType    deviceId,
    const uint16            numFifoBytes,
    const uint16            userData,
    VpDevCtxType            *pDevCtx,
    VpKWrapDeviceObjectType *pDevObj)
{
    VpApiModMkDevObjType ioMkDevObj = {
        .deviceId = deviceId,
        .deviceType = deviceType,
        .numFifoBytes = numFifoBytes,
        .userData = userData
    };

    /* redundant checking of the object and context */
    if ((pDevCtx == VP_NULL) || (pDevObj == NULL)) {
        return VP_STATUS_INVALID_ARG;
    }

    /* Initialize the members of device context */
    pDevObj->deviceId = deviceId;
    pDevCtx->pDevObj = pDevObj;
    pDevCtx->deviceType = VP_DEV_KWRAP;

    /* Open the device driver*/
    pDevObj->fileDes = open(VPAPI_MOD_FILE_DES, O_RDWR);
    if (-1 == pDevObj->fileDes) {
        return VP_STATUS_FAILURE;
    }

    /* need to set the size of the fifo here using ioctl */

    /* need to add the device to the driver */
    if (ioctl(pDevObj->fileDes, VPAPI_MOD_IOX_MK_DEV_OBJ, &ioMkDevObj) < 0) {
        close(pDevObj->fileDes);
        return VP_STATUS_FAILURE;
    }

    /* check status of VP-API command */
    if (VP_STATUS_SUCCESS != ioMkDevObj.status) {
        return ioMkDevObj.status;
    }

    /* Initialization function pointers */
    pDevCtx->funPtrsToApiFuncs.BootLoad             = KWrapBootLoad;
    pDevCtx->funPtrsToApiFuncs.BootSlac             = KWrapBootSlac;
    pDevCtx->funPtrsToApiFuncs.InitDevice           = KWrapInitDevice;
    pDevCtx->funPtrsToApiFuncs.InitLine             = KWrapInitLine;
    pDevCtx->funPtrsToApiFuncs.InitSlac             = KWrapInitSlac;
    pDevCtx->funPtrsToApiFuncs.ConfigLine           = KWrapConfigLine;
    pDevCtx->funPtrsToApiFuncs.CalCodec             = KWrapCalCodec;
    pDevCtx->funPtrsToApiFuncs.CalLine              = KWrapCalLn;
    pDevCtx->funPtrsToApiFuncs.Cal                  = KWrapCal;
    pDevCtx->funPtrsToApiFuncs.FreeRun              = KWrapFreeRun;
    pDevCtx->funPtrsToApiFuncs.InitMeter            = KWrapInitMeter;
    pDevCtx->funPtrsToApiFuncs.InitCustomTerm       = KWrapInitCustomTerm;
    pDevCtx->funPtrsToApiFuncs.InitProfile          = KWrapInitProfile;
    pDevCtx->funPtrsToApiFuncs.SoftReset            = KWrapSoftReset;
    pDevCtx->funPtrsToApiFuncs.SetBatteries         = KWrapSetBatteries;

    /* Control function pointers */
    pDevCtx->funPtrsToApiFuncs.SetLineState         = KWrapSetLineState;
    pDevCtx->funPtrsToApiFuncs.SetLineTone          = KWrapSetLineTone;
    pDevCtx->funPtrsToApiFuncs.SetRelayState        = KWrapSetRelayState;
    pDevCtx->funPtrsToApiFuncs.SetRelGain           = KWrapSetRelGain;
    pDevCtx->funPtrsToApiFuncs.SendSignal           = KWrapSendSignal;
    pDevCtx->funPtrsToApiFuncs.DtmfDigitDetected    = KWrapDtmfDigitDet;
    pDevCtx->funPtrsToApiFuncs.StartMeter           = KWrapStartMeter;
    pDevCtx->funPtrsToApiFuncs.StartMeter32Q        = KWrapStartMeter32Q;
    pDevCtx->funPtrsToApiFuncs.SetOption            = KWrapSetOption;
    pDevCtx->funPtrsToApiFuncs.DeviceIoAccess       = KWrapDeviceIoAccess;
    pDevCtx->funPtrsToApiFuncs.DeviceIoAccessExt    = KWrapDeviceIoAccessExt;
    pDevCtx->funPtrsToApiFuncs.SelfTest             = KWrapSelfTest;
    pDevCtx->funPtrsToApiFuncs.FillTestBuf          = KWrapFillTestBuf;
    pDevCtx->funPtrsToApiFuncs.LowLevelCmd          = KWrapLowLevelCmd;
    pDevCtx->funPtrsToApiFuncs.LowLevelCmd16        = KWrapLowLevelCmd16;
    pDevCtx->funPtrsToApiFuncs.SetBFilter           = KWrapSetBFilter;
    pDevCtx->funPtrsToApiFuncs.LineIoAccess         = KWrapLineIoAccess;
    pDevCtx->funPtrsToApiFuncs.GenTimerCtrl         = KWrapGenTimerCtrl;

    /* Status and Query function pointers */
    pDevCtx->funPtrsToApiFuncs.GetEvent             = KWrapGetEvent;
    pDevCtx->funPtrsToApiFuncs.GetLineStatus        = KWrapGetLineStatus;
    pDevCtx->funPtrsToApiFuncs.GetLineState         = KWrapGetLineState;
    pDevCtx->funPtrsToApiFuncs.GetDeviceStatus      = KWrapGetDeviceStatus;
    pDevCtx->funPtrsToApiFuncs.GetDeviceStatusExt   = KWrapGetDeviceStatusExt;
    pDevCtx->funPtrsToApiFuncs.GetLoopCond          = KWrapGetLoopCond;
    pDevCtx->funPtrsToApiFuncs.GetOption            = KWrapGetOption;
    pDevCtx->funPtrsToApiFuncs.FlushEvents          = KWrapFlushEvents;
    pDevCtx->funPtrsToApiFuncs.GetResults           = KWrapGetResults;
    pDevCtx->funPtrsToApiFuncs.ClearResults         = KWrapClearResults;
    pDevCtx->funPtrsToApiFuncs.CodeCheckSum         = KWrapCodeCheckSum;
    pDevCtx->funPtrsToApiFuncs.Query                = KWrapQuery;
    pDevCtx->funPtrsToApiFuncs.GetRelayState        = KWrapGetRelayState;
    pDevCtx->funPtrsToApiFuncs.ReadTestBuf          = KWrapReadTestBuf;
    pDevCtx->funPtrsToApiFuncs.RegisterDump         = KWrapRegisterDump;

    pDevCtx->funPtrsToApiFuncs.TestLine             = KWrapTestLine;

    return VP_STATUS_SUCCESS;
}

EXTERN VpStatusType KWrapOpenLine(
    const VpTermType        termType,
    const uint8             channelId,
    const VpLineIdType      lineId,
    VpLineCtxType           *pLineCtx,
    VpKWrapLineObjectType   *pLineObj,
    VpDevCtxType            *pDevCtx)
{
    VpKWrapDeviceObjectType     *pDevObj = pDevCtx->pDevObj;

    /* init with const data */
    VpApiModMkLnObjType ioMkLnObj = {
        .channelId  = channelId,
        .termType   = termType,
        .lineId     = lineId
    };

    /* Basic error checking */
    if((pLineObj == VP_NULL) || (pDevCtx == VP_NULL) || (pLineCtx == VP_NULL)) {
        return VP_STATUS_INVALID_ARG;
    }

    /* issue the corresponding ioctl */
    if (ioctl(pDevObj->fileDes, VPAPI_MOD_IOX_MK_LN_OBJ, &ioMkLnObj) < 0) {
        return VP_STATUS_FAILURE;
    }

    /* check status of VP-API command */
    if (VP_STATUS_SUCCESS != ioMkLnObj.status) {
        return ioMkLnObj.status;
    }

    /* Initialize line context */
    pLineCtx->pLineObj  = pLineObj;
    pLineCtx->pDevCtx   = pDevCtx;

    /* Establish the link between device context to line context */
    pDevCtx->pLineCtx[channelId] = pLineCtx;

    /* Initialize line object */
    pLineObj->channelId = channelId;
    pLineObj->lineRegNum = ioMkLnObj.lineRegNum;
    pLineObj->termType = termType;
    pLineObj->lineId = lineId;

    return VP_STATUS_SUCCESS;
}

EXTERN VpStatusType KWrapGetDeviceInfo(
    VpDevCtxType *pDevCtx,
    VpDeviceInfoType *pDeviceInfo)
{
    VpKWrapDeviceObjectType *pDevObj = pDevCtx->pDevObj;

    /* load up the kwrap struct */
    VpApiModGetDevInfoType ioDevInfo = {
        .pDeviceInfo = pDeviceInfo
    };

    /* issue the corresponding ioctl */
    if (ioctl(pDevObj->fileDes, VPAPI_MOD_IOX_GET_DEV_INFO, &ioDevInfo) < 0) {
        return VP_STATUS_FAILURE;
    }

    return ioDevInfo.status;
}  /* KwrapGetDeviceInfo() */

EXTERN VpStatusType KWrapMapLineId(
    VpLineCtxType       *pLineCtx,
    VpLineIdType        lineId)
{
    VpKWrapDeviceObjectType *pDevObj = (pLineCtx->pDevCtx)->pDevObj;
    VpKWrapLineObjectType *pLineObj = pLineCtx->pLineObj;

    /* load up the kwrap struct */
    VpApiModMapLineIdType ioLineId = {
        .lineRegNum = pLineObj->lineRegNum,
        .lineId     = lineId
    };

    /* issue the corresponding ioctl */
    if (ioctl(pDevObj->fileDes, VPAPI_MOD_IOX_MAP_LINE_ID, &ioLineId) < 0) {
        return VP_STATUS_FAILURE;
    }

    return ioLineId.status;
}  /* KWrapMapLineId() */


EXTERN VpStatusType KWrapMapSlacId(
    VpDevCtxType        *pDevCtx,
    uint8               slacId)
{
    VpKWrapDeviceObjectType *pDevObj = pDevCtx->pDevObj;

    /* load up the kwrap struct */
    VpApiModMapSlacIdType ioSlacId = {
        .slacId = slacId
    };

    /* issue the corresponding ioctl */
    if (ioctl(pDevObj->fileDes, VPAPI_MOD_IOX_MAP_SLAC_ID, &ioSlacId) < 0) {
        return VP_STATUS_FAILURE;
    }

    return ioSlacId.status;
}  /* KWrapMapSlacId() */


/******************************************************************************
 *                   KWRAP Specific Initialization Functions                    *
 ******************************************************************************/
static VpStatusType KWrapBootLoad(
    VpDevCtxType        *pDevCtx,
    VpBootStateType     state,
    VpImagePtrType      pImageBuffer,
    uint32              bufferSize,
    VpScratchMemType    *pScratchMem,
    VpBootModeType      validation)
{
    VpKWrapDeviceObjectType *pDevObj = pDevCtx->pDevObj;

    /* init with const data */
    VpApiModBootLoadType ioBootLoadInfo = {
        .state    = state,
        .pImageBuffer   = pImageBuffer,
        .bufferSize     = bufferSize,
        .pScratchMem    = pScratchMem,
        .validation     = validation
    };

    /* issue the corresponding ioctl */
    if (ioctl(pDevObj->fileDes, VPAPI_MOD_IOX_BOOT_LOAD, &ioBootLoadInfo) < 0) {
        return VP_STATUS_FAILURE;
    }

    return ioBootLoadInfo.status;
}

static VpStatusType KWrapBootSlac(
    VpLineCtxType       *pLineCtx,
    VpDevCtxType        *pDevCtx,
    VpImagePtrType      pImageBuffer,
    uint32              bufferSize)
{

    if (pDevCtx != VP_NULL) {
        /* init all slacs */
        VpKWrapDeviceObjectType *pDevObj = pDevCtx->pDevObj;

        /* init with const data */
        VpApiModBootSlacType ioBootSlacInfo = {
            .lineRequest    = FALSE,
            .pImageBuffer   = pImageBuffer,
            .bufferSize     = bufferSize
        };

        /* issue the corresponding ioctl */
        if (ioctl(pDevObj->fileDes, VPAPI_MOD_IOX_BOOT_SLAC, &ioBootSlacInfo) < 0) {
            return VP_STATUS_FAILURE;
        }
        return ioBootSlacInfo.status;

    } else if (pLineCtx != VP_NULL) {
        /* init one slac */
        VpKWrapDeviceObjectType *pDevObj = (pLineCtx->pDevCtx)->pDevObj;
        VpKWrapLineObjectType *pLineObj = pLineCtx->pLineObj;

        /* init with const data */
        VpApiModBootSlacType ioBootSlacInfo = {
            .lineRequest    = TRUE,
            .lineRegNum     = pLineObj->lineRegNum,
            .pImageBuffer   = pImageBuffer,
            .bufferSize     = bufferSize
        };

        /* issue the corresponding ioctl */
        if (ioctl(pDevObj->fileDes, VPAPI_MOD_IOX_BOOT_SLAC, &ioBootSlacInfo) < 0) {
            return VP_STATUS_FAILURE;
        }
        return ioBootSlacInfo.status;

    } else {
        return VP_STATUS_INVALID_ARG;
    }
}

static VpStatusType KWrapInitDevice(
    VpDevCtxType        *pDevCtx,
    VpProfilePtrType    pDevProfile,
    VpProfilePtrType    pAcProfile,
    VpProfilePtrType    pDcProfile,
    VpProfilePtrType    pRingProfile,
    VpProfilePtrType    pFxoAcProfile,
    VpProfilePtrType    pFxoCfgProfile)
{
    VpKWrapDeviceObjectType *pDevObj = pDevCtx->pDevObj;

    /* init with const data */
    VpApiModInitDeviceType ioInitDevInfo = {
        .pDevProfile    = pDevProfile,
        .pAcProfile     = pAcProfile,
        .pDcProfile     = pDcProfile,
        .pRingProfile   = pRingProfile,
        .pFxoAcProfile  = pFxoAcProfile,
        .pFxoCfgProfile = pFxoCfgProfile
    };

    /* issue the corresponding ioctl */
    if (ioctl(pDevObj->fileDes, VPAPI_MOD_IOX_INIT_DEV, &ioInitDevInfo) < 0) {
        return VP_STATUS_FAILURE;
    }

    return ioInitDevInfo.status;
}

static VpStatusType KWrapInitLine(
    VpLineCtxType       *pLineCtx,
    VpProfilePtrType    pAcProfile,
    VpProfilePtrType    pDcProfile,
    VpProfilePtrType    pRingProfile)
{
    VpKWrapDeviceObjectType *pDevObj = (pLineCtx->pDevCtx)->pDevObj;
    VpKWrapLineObjectType *pLineObj = pLineCtx->pLineObj;

    /* init with const data */
    VpApiModInitLnType ioInitLineInfo = {
        .lineRegNum     = pLineObj->lineRegNum,
        .pAcProfile     = pAcProfile,
        .pDcProfile     = pDcProfile,
        .pRingProfile   = pRingProfile
    };

    /* issue the corresponding ioctl */
    if (ioctl(pDevObj->fileDes, VPAPI_MOD_IOX_INIT_LN, &ioInitLineInfo) < 0) {
        return VP_STATUS_FAILURE;
    }

    return ioInitLineInfo.status;
}

static VpStatusType KWrapInitSlac(
    VpLineCtxType       *pLineCtx,
    VpProfilePtrType    pDevProfile,
    VpProfilePtrType    pAcProfile,
    VpProfilePtrType    pDcProfile,
    VpProfilePtrType    pRingProfile)
{
    VpKWrapDeviceObjectType *pDevObj = (pLineCtx->pDevCtx)->pDevObj;
    VpKWrapLineObjectType *pLineObj = pLineCtx->pLineObj;

    /* init with const data */
    VpApiModInitSlacType ioInitSlacInfo = {
        .lineRegNum     = pLineObj->lineRegNum,
        .pDevProfile    = pDevProfile,
        .pAcProfile     = pAcProfile,
        .pDcProfile     = pDcProfile,
        .pRingProfile   = pRingProfile
    };

    /* issue the corresponding ioctl */
    if (ioctl(pDevObj->fileDes, VPAPI_MOD_IOX_INIT_SLAC, &ioInitSlacInfo) < 0) {
        return VP_STATUS_FAILURE;
    }

    return ioInitSlacInfo.status;
}

static VpStatusType KWrapConfigLine(
    VpLineCtxType       *pLineCtx,
    VpProfilePtrType    pAcProfile,
    VpProfilePtrType    pDcOrFxoProfile,
    VpProfilePtrType    pRingProfile)
{
    VpKWrapDeviceObjectType *pDevObj = (pLineCtx->pDevCtx)->pDevObj;
    VpKWrapLineObjectType *pLineObj = pLineCtx->pLineObj;

    /* init with const data */
    VpApiModCfgLnType ioCfgLineInfo = {
        .lineRegNum     = pLineObj->lineRegNum,
        .pAcProfile     = pAcProfile,
        .pDcOrFxoProfile= pDcOrFxoProfile,
        .pRingProfile   = pRingProfile
    };

    /* issue the corresponding ioctl */
    if (ioctl(pDevObj->fileDes, VPAPI_MOD_IOX_CFG_LN, &ioCfgLineInfo) < 0) {
        return VP_STATUS_FAILURE;
    }

    return ioCfgLineInfo.status;
}

static VpStatusType KWrapCalCodec(
    VpLineCtxType       *pLineCtx,
    VpDeviceCalType     mode)
{
    VpKWrapDeviceObjectType *pDevObj = (pLineCtx->pDevCtx)->pDevObj;
    VpKWrapLineObjectType *pLineObj = pLineCtx->pLineObj;

    /* init with const data */
    VpApiModCalCodecType ioCalCodec = {
        .lineRegNum     = pLineObj->lineRegNum,
        .mode           = mode
    };

    /* issue the corresponding ioctl */
    if (ioctl(pDevObj->fileDes, VPAPI_MOD_IOX_CAL_CODEC, &ioCalCodec) < 0) {
        return VP_STATUS_FAILURE;
    }

    return ioCalCodec.status;


}

static VpStatusType KWrapCalLn(
    VpLineCtxType       *pLineCtx)
{
    VpKWrapDeviceObjectType *pDevObj = (pLineCtx->pDevCtx)->pDevObj;
    VpKWrapLineObjectType *pLineObj = pLineCtx->pLineObj;

    /* init with const data */
    VpApiModCalLnType ioCalLine = {
        .lineRegNum     = pLineObj->lineRegNum,
    };

    /* issue the corresponding ioctl */
    if (ioctl(pDevObj->fileDes, VPAPI_MOD_IOX_CAL_LN, &ioCalLine) < 0) {
        return VP_STATUS_FAILURE;
    }

    return ioCalLine.status;
}

static VpStatusType KWrapCal(
    VpLineCtxType       *pLineCtx,
    VpCalType           calType,
    void                *inputArgs)
{
    VpKWrapDeviceObjectType *pDevObj = (pLineCtx->pDevCtx)->pDevObj;
    VpKWrapLineObjectType *pLineObj = pLineCtx->pLineObj;

    /* init with const data */
    VpApiModCalType ioCal = {
        .lineRegNum = pLineObj->lineRegNum,
        .calType    = calType,
        .inputArgs  = inputArgs,
    };

    /* issue the corresponding ioctl */
    if (ioctl(pDevObj->fileDes, VPAPI_MOD_IOX_CAL, &ioCal) < 0) {
        return VP_STATUS_FAILURE;
    }

    return ioCal.status;
}

static VpStatusType KWrapFreeRun(
    VpDevCtxType       *pDevCtx,
    VpFreeRunModeType  freeRunMode)
{
    VpKWrapDeviceObjectType *pDevObj = pDevCtx->pDevObj;

    /* restart prepare with const data */
    VpFreeRunType ioFreeRun = {
        .freeRunMode  = freeRunMode,
    };

    /* issue the corresponding ioctl */
    if (ioctl(pDevObj->fileDes, VPAPI_MOD_IOX_FREE_RUN, &ioFreeRun) < 0) {
        return VP_STATUS_FAILURE;
    }

    return ioFreeRun.status;
}

static VpStatusType KWrapInitMeter(
    VpLineCtxType       *pLineCtx,
    VpProfilePtrType    pMeterProfile)
{
    VpKWrapDeviceObjectType *pDevObj = (pLineCtx->pDevCtx)->pDevObj;
    VpKWrapLineObjectType *pLineObj = pLineCtx->pLineObj;

    /* init with const data */
    VpApiModInitMtrType ioInitMeter = {
        .lineRegNum     = pLineObj->lineRegNum,
        .pMeterProfile  = pMeterProfile
    };

    /* issue the corresponding ioctl */
    if (ioctl(pDevObj->fileDes, VPAPI_MOD_IOX_INIT_MTR, &ioInitMeter) < 0) {
        return VP_STATUS_FAILURE;
    }

    return ioInitMeter.status;
}

static VpStatusType KWrapInitCustomTerm(
    VpDevCtxType        *pDevCtx,
    VpLineCtxType       *pLineCtx,
    VpProfilePtrType    pCustomTermProfile)
{
    VpKWrapDeviceObjectType *pDevObj = (pLineCtx->pDevCtx)->pDevObj;
    VpKWrapLineObjectType *pLineObj = pLineCtx->pLineObj;

    /* init with const data */
    VpApiModInitCstmTermType ioInitCustom = {
        .lineRegNum             = pLineObj->lineRegNum,
        .pCustomTermProfile     = pCustomTermProfile
    };

    /* issue the corresponding ioctl */
    if (ioctl(pDevObj->fileDes, VPAPI_MOD_IOCTL_INIT_CSTM_TERM, &ioInitCustom) < 0) {
        return VP_STATUS_FAILURE;
    }

    return ioInitCustom.status;

}

static VpStatusType KWrapInitProfile(
    VpDevCtxType        *pDevCtx,
    VpProfileType       type,
    VpProfilePtrType    pProfileIndex,
    VpProfilePtrType    pProfile)
{
    VpKWrapDeviceObjectType *pDevObj = pDevCtx->pDevObj;

    /* init with const data */
    VpApiModInitProfType ioInitProfInfo = {
        .type           = type,
        .pProfileIndex  = pProfileIndex,
        .pProfile       = pProfile
    };

    /* issue the corresponding ioctl */
    if (ioctl(pDevObj->fileDes, VPAPI_MOD_IOX_INIT_PROF, &ioInitProfInfo) < 0) {
        return VP_STATUS_FAILURE;
    }

    return ioInitProfInfo.status;
}

static VpStatusType KWrapSoftReset(
    VpDevCtxType                *pDevCtx)
{
    VpKWrapDeviceObjectType *pDevObj = pDevCtx->pDevObj;

    /* init with const data */
    VpApiModSftRstType ioSoftReset;

    /* issue the corresponding ioctl */
    if (ioctl(pDevObj->fileDes, VPAPI_MOD_IOX_SFT_RST, &ioSoftReset) < 0) {
        return VP_STATUS_FAILURE;
    }

    return ioSoftReset.status;
}

static VpStatusType KWrapSetBatteries(
    VpLineCtxType       *pLineCtx,
    VpBatteryModeType   battMode,
    VpBatteryValuesType *pBatt)
{
    VpKWrapDeviceObjectType *pDevObj = (pLineCtx->pDevCtx)->pDevObj;
    VpKWrapLineObjectType *pLineObj = pLineCtx->pLineObj;

    /* init with const data */
    VpApiModSetBatType ioSetBatt = {
        .lineRegNum     = pLineObj->lineRegNum,
        .battMode       = battMode,
        .pBatt          = pBatt
    };

    /* issue the corresponding ioctl */
    if (ioctl(pDevObj->fileDes, VPAPI_MOD_IOX_SET_BATT, &ioSetBatt) < 0) {
        return VP_STATUS_FAILURE;
    }

    return ioSetBatt.status;
}


static VpStatusType KWrapSetLineState(
    VpLineCtxType       *pLineCtx,
    VpLineStateType     state)
{
    VpKWrapDeviceObjectType *pDevObj = (pLineCtx->pDevCtx)->pDevObj;
    VpKWrapLineObjectType *pLineObj = pLineCtx->pLineObj;

    /* init with const data */
    VpApiModSetLnStType ioSetLineState = {
        .lineRegNum     = pLineObj->lineRegNum,
        .state          = state,
    };

    /* issue the corresponding ioctl */
    if (ioctl(pDevObj->fileDes, VPAPI_MOD_IOX_SET_LN_ST, &ioSetLineState) < 0) {
        return VP_STATUS_FAILURE;
    }

    return ioSetLineState.status;
}

static VpStatusType KWrapSetLineTone(
    VpLineCtxType       *pLineCtx,
    VpProfilePtrType    pToneProfile,
    VpProfilePtrType    pCadProfile,
    VpDtmfToneGenType   *pDtmfControl)
{
    VpKWrapDeviceObjectType *pDevObj = (pLineCtx->pDevCtx)->pDevObj;
    VpKWrapLineObjectType *pLineObj = pLineCtx->pLineObj;

    /* init with const data */
    VpApiModSetLnTnType ioSetLineTone = {
        .lineRegNum     = pLineObj->lineRegNum,
        .pToneProfile   = pToneProfile,
        .pCadProfile    = pCadProfile,
        .pDtmfControl   = pDtmfControl
    };

    /* issue the corresponding ioctl */
    if (ioctl(pDevObj->fileDes, VPAPI_MOD_IOX_SET_LN_TN, &ioSetLineTone) < 0) {
        return VP_STATUS_FAILURE;
    }

    return ioSetLineTone.status;
}

static VpStatusType KWrapSetRelayState(
    VpLineCtxType       *pLineCtx,
    VpRelayControlType  rState)
{
    VpKWrapDeviceObjectType *pDevObj = (pLineCtx->pDevCtx)->pDevObj;
    VpKWrapLineObjectType *pLineObj = pLineCtx->pLineObj;

    /* init with const data */
    VpApiModSetRlyStType ioSetRelayState = {
        .lineRegNum     = pLineObj->lineRegNum,
        .rState         = rState,
    };

    /* issue the corresponding ioctl */
    if (ioctl(pDevObj->fileDes, VPAPI_MOD_IOX_SET_RLY_ST, &ioSetRelayState) < 0) {
        return VP_STATUS_FAILURE;
    }

    return ioSetRelayState.status;
}

static VpStatusType KWrapSetRelGain(
    VpLineCtxType       *pLineCtx,
    uint16              txLevel,
    uint16              rxLevel,
    uint16              handle)
{
    VpKWrapDeviceObjectType *pDevObj = (pLineCtx->pDevCtx)->pDevObj;
    VpKWrapLineObjectType *pLineObj = pLineCtx->pLineObj;

    /* init with const data */
    VpApiModSetRelGnType ioSetRelGain = {
        .lineRegNum     = pLineObj->lineRegNum,
        .txLevel        = txLevel,
        .rxLevel        = rxLevel,
        .handle         = handle
    };

    /* issue the corresponding ioctl */
    if (ioctl(pDevObj->fileDes, VPAPI_MOD_IOX_SET_REL_GN, &ioSetRelGain) < 0) {
        return VP_STATUS_FAILURE;
    }

    return ioSetRelGain.status;
}

static VpStatusType KWrapSendSignal(
    VpLineCtxType       *pLineCtx,
    VpSendSignalType    signalType,
    void                *pSignalData)
{
    VpKWrapDeviceObjectType *pDevObj = (pLineCtx->pDevCtx)->pDevObj;
    VpKWrapLineObjectType *pLineObj = pLineCtx->pLineObj;

    /* init with const data */
    VpApiModSendSigType ioSendSignal = {
        .lineRegNum     = pLineObj->lineRegNum,
        .signalType     = signalType,
        .pSignalData    = pSignalData
    };

    /* issue the corresponding ioctl */
    if (ioctl(pDevObj->fileDes, VPAPI_MOD_IOX_SEND_SIG, &ioSendSignal) < 0) {
        return VP_STATUS_FAILURE;
    }

    return ioSendSignal.status;
}

static VpStatusType KWrapDtmfDigitDet(
    VpLineCtxType       *pLineCtx,
    VpDigitType         digit,
    VpDigitSenseType    sense)
{
    VpKWrapDeviceObjectType *pDevObj = (pLineCtx->pDevCtx)->pDevObj;
    VpKWrapLineObjectType *pLineObj = pLineCtx->pLineObj;

    /* init with const data */
    VpApiModDtmfDetectType ioDtmfDigit = {
        .lineRegNum     = pLineObj->lineRegNum,
        .digit          = digit,
        .sense          = sense
    };

    /* issue the corresponding ioctl */
    if (ioctl(pDevObj->fileDes, VPAPI_MOD_IOX_DTMF_DETECT, &ioDtmfDigit) < 0) {
        return VP_STATUS_FAILURE;
    }

    return ioDtmfDigit.status;
}

static VpStatusType KWrapStartMeter(
    VpLineCtxType       *pLineCtx,
    uint16              onTime,
    uint16              offTime,
    uint16              numMeters)
{
    VpKWrapDeviceObjectType *pDevObj = (pLineCtx->pDevCtx)->pDevObj;
    VpKWrapLineObjectType *pLineObj = pLineCtx->pLineObj;

    /* init with const data */
    VpApiModStartMtrType ioStartMeter = {
        .lineRegNum     = pLineObj->lineRegNum,
        .onTime         = onTime,
        .offTime        = offTime,
        .numMeters      = numMeters
    };

    /* issue the corresponding ioctl */
    if (ioctl(pDevObj->fileDes, VPAPI_MOD_IOX_START_MTR, &ioStartMeter) < 0) {
        return VP_STATUS_FAILURE;
    }

    return ioStartMeter.status;
}

static VpStatusType KWrapStartMeter32Q(
    VpLineCtxType       *pLineCtx,
    uint32              minDelay,
    uint32              onTime,
    uint32              offTime,
    uint16              numMeters,
    uint16              eventRate)
{
    VpKWrapDeviceObjectType *pDevObj = (pLineCtx->pDevCtx)->pDevObj;
    VpKWrapLineObjectType *pLineObj = pLineCtx->pLineObj;

    /* init with const data */
    VpApiModStartMtr32QType ioStartMeter32Q = {
        .lineRegNum     = pLineObj->lineRegNum,
        .minDelay       = minDelay,
        .onTime         = onTime,
        .offTime        = offTime,
        .numMeters      = numMeters,
        .eventRate      = eventRate
    };

    /* issue the corresponding ioctl */
    if (ioctl(pDevObj->fileDes, VPAPI_MOD_IOX_START_MTR32Q, &ioStartMeter32Q) < 0) {
        return VP_STATUS_FAILURE;
    }

    return ioStartMeter32Q.status;
}

EXTERN VpStatusType KWrapSetOption(
    VpLineCtxType       *pLineCtx,
    VpDevCtxType        *pDevCtx,
    VpOptionIdType      option,
    void                *pValue)
{

    if ((pDevCtx == NULL) && (pLineCtx == NULL) && (option == VP_OPTION_ID_DEBUG_SELECT)) {
        /*
         * special case:
         * need to set a variable that is not specific to any device
         * or line object. This variable is for debug only.
         */
        /* init with const data */
        VpApiModSetOptionType ioSetOption = {
            .lineRequest    = FALSE,
            .lineRegNum     = -1,
            .option         = option,
            .pValue         = pValue
        };

        int fd = open(VPAPI_MOD_FILE_DES, O_RDWR);
        if (fd == -1) {
            return VP_STATUS_FAILURE;
        }

        if (ioctl(fd, VPAPI_MOD_IOX_SET_OPTION, &ioSetOption) < 0) {
            close(fd);
            return VP_STATUS_FAILURE;
        }

        close(fd);
        return ioSetOption.status;

    } else if (pDevCtx != VP_NULL) {

        /* send the set option at the device level */
        VpKWrapDeviceObjectType *pDevObj = pDevCtx->pDevObj;

        /* init with const data */
        VpApiModSetOptionType ioSetOption = {
            .lineRequest    = FALSE,
            .option         = option,
            .pValue         = pValue
        };

        /* issue the corresponding ioctl */
        if (ioctl(pDevObj->fileDes, VPAPI_MOD_IOX_SET_OPTION, &ioSetOption) < 0) {
            return VP_STATUS_FAILURE;
        }

        return ioSetOption.status;

    } else if (pLineCtx != VP_NULL) {
        /* send the set option at the line level */
        VpKWrapDeviceObjectType *pDevObj = (pLineCtx->pDevCtx)->pDevObj;
        VpKWrapLineObjectType *pLineObj = pLineCtx->pLineObj;


        /* init with const data */
        VpApiModSetOptionType ioSetOption = {
            .lineRequest    = TRUE,
            .lineRegNum     = pLineObj->lineRegNum,
            .option         = option,
            .pValue         = pValue
        };

        /* issue the corresponding ioctl */
        if (ioctl(pDevObj->fileDes, VPAPI_MOD_IOX_SET_OPTION, &ioSetOption) < 0) {
            return VP_STATUS_FAILURE;
        }

        return ioSetOption.status;
    } else {
        return VP_STATUS_INVALID_ARG;
    }
}

static VpStatusType KWrapDeviceIoAccess(
    VpDevCtxType                *pDevCtx,
    VpDeviceIoAccessDataType    *pDeviceIoData)
{

    VpKWrapDeviceObjectType *pDevObj = pDevCtx->pDevObj;

    /* init with const data */
    VpApiModDevIoAccType ioDeviceIo = {
        .pDeviceIoData  = pDeviceIoData
    };

    /* issue the corresponding ioctl */
    if (ioctl(pDevObj->fileDes, VPAPI_MOD_IOX_DEV_IO_ACC, &ioDeviceIo) < 0) {
        return VP_STATUS_FAILURE;
    }
    return ioDeviceIo.status;
}

static VpStatusType KWrapDeviceIoAccessExt(
    VpDevCtxType            *pDevCtx,
    VpDeviceIoAccessExtType *pDeviceIoAccess)
{
    VpKWrapDeviceObjectType *pDevObj = pDevCtx->pDevObj;

    /* init with const data */
    VpApiModDevIoAccExtType ioDeviceIoExt = {
        .pDeviceIoAccess    = pDeviceIoAccess
    };

    /* issue the corresponding ioctl */
    if (ioctl(pDevObj->fileDes, VPAPI_MOD_IOX_DEV_IO_ACC_EXT, &ioDeviceIoExt) < 0) {
        return VP_STATUS_FAILURE;
    }

    return ioDeviceIoExt.status;
}

static VpStatusType KWrapSelfTest(
    VpLineCtxType       *pLineCtx)

{
    VpKWrapDeviceObjectType *pDevObj = (pLineCtx->pDevCtx)->pDevObj;
    VpKWrapLineObjectType *pLineObj = pLineCtx->pLineObj;

    /* init with const data */
    VpApiModSelfTestType ioSelfTest = {
        .lineRegNum     = pLineObj->lineRegNum,
    };

    /* issue the corresponding ioctl */
    if (ioctl(pDevObj->fileDes, VPAPI_MOD_IOX_SELF_TEST, &ioSelfTest) < 0) {
        return VP_STATUS_FAILURE;
    }

    return ioSelfTest.status;
}

static VpStatusType KWrapLowLevelCmd(
    VpLineCtxType       *pLineCtx,
    uint8               *pCmdData,
    uint8               len,
    uint16              handle)

{
    VpKWrapDeviceObjectType *pDevObj = (pLineCtx->pDevCtx)->pDevObj;
    VpKWrapLineObjectType *pLineObj = pLineCtx->pLineObj;

    /* init with const data */
    VpApiModLowLvlCmdType lowLvlCmd = {
        .lineRegNum     = pLineObj->lineRegNum,
        .pCmdData       = pCmdData,
        .len            = len,
        .handle         = handle
    };

    /* issue the corresponding ioctl */
    if (ioctl(pDevObj->fileDes, VPAPI_MOD_IOX_LOW_LVL_CMD, &lowLvlCmd) < 0) {
        return VP_STATUS_FAILURE;
    }

    return lowLvlCmd.status;
}

static VpStatusType KWrapLowLevelCmd16(
    VpLineCtxType       *pLineCtx,
    VpLowLevelCmdType   cmdType,
    uint16              *writeWords,
    uint8               numWriteWords,
    uint8               numReadWords,
    uint16              handle)

{
    VpKWrapDeviceObjectType *pDevObj = (pLineCtx->pDevCtx)->pDevObj;
    VpKWrapLineObjectType *pLineObj = pLineCtx->pLineObj;

    /* init with const data */
    VpApiModLowLvlCmd16Type lowLvlCmd16 = {
        .lineRegNum     = pLineObj->lineRegNum,
        .cmdType        = cmdType,
        .pWriteWords    = writeWords,
        .numWriteWords  = numWriteWords,
        .numReadWords   = numReadWords,
        .handle         = handle
    };

    /* issue the corresponding ioctl */
    if (ioctl(pDevObj->fileDes, VPAPI_MOD_IOX_LOW_LVL_CMD16, &lowLvlCmd16) < 0) {
        return VP_STATUS_FAILURE;
    }

    return lowLvlCmd16.status;
}

static VpStatusType KWrapSetBFilter(
    VpLineCtxType       *pLineCtx,
    VpBFilterModeType   bFiltMode,
    VpProfilePtrType    pAcProfile)
{

    VpKWrapDeviceObjectType *pDevObj = (pLineCtx->pDevCtx)->pDevObj;
    VpKWrapLineObjectType *pLineObj = pLineCtx->pLineObj;

    /* init with const data */
    VpApiModSetBFilterType ioSetBFilter = {
        .lineRegNum     = pLineObj->lineRegNum,
        .bFiltMode      = bFiltMode,
        .pAcProfile     = pAcProfile
    };

    /* issue the corresponding ioctl */
    if (ioctl(pDevObj->fileDes, VPAPI_MOD_IOX_SET_B_FILTER, &ioSetBFilter) < 0) {
        return VP_STATUS_FAILURE;
    }

    return ioSetBFilter.status;
}


static VpStatusType KWrapFillTestBuf(
    VpLineCtxType       *pLineCtx,
    uint16              length,
    VpVectorPtrType     pData)

{
    VpKWrapDeviceObjectType *pDevObj = (pLineCtx->pDevCtx)->pDevObj;
    VpKWrapLineObjectType *pLineObj = pLineCtx->pLineObj;

    /* init with const data */
    VpApiModFillBuffType ioTestBuff = {
        .lineRegNum     = pLineObj->lineRegNum,
        .length         = length,
        .pData          = pData
    };

    /* issue the corresponding ioctl */
    if (ioctl(pDevObj->fileDes, VPAPI_MOD_IOX_FILL_BUFF, &ioTestBuff) < 0) {
        return VP_STATUS_FAILURE;
    }

    return ioTestBuff.status;
}

static VpStatusType KWrapLineIoAccess(
    VpLineCtxType       *pLineCtx,
    VpLineIoAccessType  *pLineIoAccess,
    uint16              handle)
{
    VpKWrapDeviceObjectType *pDevObj = (pLineCtx->pDevCtx)->pDevObj;
    VpKWrapLineObjectType *pLineObj = pLineCtx->pLineObj;

    /* init with const data */
    VpApiModLnIoAccType ioLineIoExt = {
        .lineRegNum     = pLineObj->lineRegNum,
        .handle         = handle,
        .pLineIoAccess  = pLineIoAccess
    };

    /* issue the corresponding ioctl */
    if (ioctl(pDevObj->fileDes, VPAPI_MOD_IOX_LN_IO_ACC, &ioLineIoExt) < 0) {
        return VP_STATUS_FAILURE;
    }

    return ioLineIoExt.status;
}

static VpStatusType KWrapGenTimerCtrl(
    VpLineCtxType       *pLineCtx,
    VpGenTimerCtrlType  timerCtrl,
    uint32              duration,
    uint16              handle)
{
    VpKWrapDeviceObjectType *pDevObj = (pLineCtx->pDevCtx)->pDevObj;
    VpKWrapLineObjectType *pLineObj = pLineCtx->pLineObj;

    /* init with const data */
    VpApiModGenTimerCtrlType ioGenTimerCtrl = {
        .lineRegNum     = pLineObj->lineRegNum,
        .duration       = duration,
        .timerCtrl      = timerCtrl,
        .handle         = handle
    };

    /* issue the corresponding ioctl */
    if (ioctl(pDevObj->fileDes, VPAPI_MOD_IOX_GEN_TIMER_CTRL, &ioGenTimerCtrl) < 0) {
        return VP_STATUS_FAILURE;
    }

    return ioGenTimerCtrl.status;
}

static bool KWrapGetEvent(
    VpDevCtxType        *pDevCtx,
    VpEventType         *pEvent)
{
    VpKWrapDeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpApiModGetEventType    ioGetEvent;
    int8 channelId;

    ioGetEvent.fromFifo = FALSE;
    ioGetEvent.pEvent = pEvent;

    /* issue the corresponding ioctl */
    if (ioctl(pDevObj->fileDes, VPAPI_MOD_IOX_GET_EVENT, &ioGetEvent) < 0) {
        return FALSE;
    }

    /* modify the return data that is specific to the kernel */
    channelId = ioGetEvent.pEvent->channelId;
    if (ioGetEvent.pEvent->pDevCtx != NULL) {
        ioGetEvent.pEvent->pDevCtx = pDevCtx;
    }
    if (ioGetEvent.pEvent->pLineCtx != NULL) {
        ioGetEvent.pEvent->pLineCtx = pDevCtx->pLineCtx[channelId];
    }
    return ioGetEvent.newEvent;
}

static bool GetEventFromFifo(
    VpDevCtxType        *pDevCtx,
    VpEventType         *pEvent)
{
    VpKWrapDeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpApiModGetEventType    ioGetEvent;
    int8 channelId;

    ioGetEvent.fromFifo = TRUE;
    ioGetEvent.pEvent = pEvent;

    /* issue the corresponding ioctl */
    if (ioctl(pDevObj->fileDes, VPAPI_MOD_IOX_GET_EVENT, &ioGetEvent) < 0) {
        return FALSE;
    }

    /* modify the return data that is specific to the kernel */
    channelId = ioGetEvent.pEvent->channelId;
    if (ioGetEvent.pEvent->pDevCtx != NULL) {
        ioGetEvent.pEvent->pDevCtx = pDevCtx;
    }
    if (ioGetEvent.pEvent->pLineCtx != NULL) {
        ioGetEvent.pEvent->pLineCtx = pDevCtx->pLineCtx[channelId];
    }
    return ioGetEvent.newEvent;
}

/* Adds all API device file descriptors to the given fd_set.
 * Returns the highest fd value added to the set (always >= 0). */
EXTERN int KWrapAddDevsToFdSet(
    uint8               numDevs,
    VpDevCtxType        *pDevCtx[],
    fd_set              *pFdSet)
{
    int maxFd = 0;
    uint8 i = 0;

    /* Nothing to do, return 0 */
    if (pDevCtx == NULL || numDevs == 0) {
        return 0;
    }

    /* get all of the file descriptors and add them to the set */
    for(i = 0; i < numDevs; i++) {
        int fd;

        if (pDevCtx[i] == NULL) {
            continue;
        }
        fd = ((VpKWrapDeviceObjectType *)(pDevCtx[i]->pDevObj))->fileDes;

        FD_SET(fd, pFdSet);
        maxFd = MAX(fd, maxFd);
    }

    return maxFd;
}

EXTERN bool KWrapGetEventsForFdSet(
    VpEventType         *pEvent,
    uint8               numDevs,
    VpDevCtxType        *pDevCtx[],
    fd_set              *pFdSet)
{
    bool hasEvent = FALSE;
    uint8 i = 0;

    if (pDevCtx == NULL || numDevs == 0) {
        return FALSE;
    }

    /* determine which device has the event */
    for(i = 0; i < numDevs; i++) {
        int fd;

        if (pDevCtx[i] == NULL) {
            continue;
        }

        fd = ((VpKWrapDeviceObjectType *)(pDevCtx[i]->pDevObj))->fileDes;

        /* get the event and get out */
        if ( FD_ISSET(fd, pFdSet) ) {
            hasEvent = GetEventFromFifo(pDevCtx[i], pEvent);
            break;
        }
    }

    return hasEvent;
}

EXTERN bool KWrapGetEventBlocking(
    time_t              sec,
    suseconds_t         usec,
    VpEventType         *pEvent,
    uint8               numDevs,
    VpDevCtxType        *pDevCtx[])
{
    struct timeval tickTimer;
    struct timeval *pTimeout = NULL;
    fd_set fdSet;

    int maxFd = 0;
    int selRtnVal;

    /* setup a timer if requested */
    if ((sec != 0) || (usec != 0)) {
        tickTimer.tv_sec = sec;
        tickTimer.tv_usec = usec;
        pTimeout = &tickTimer;
    }

    /* Zero-out the file descriptor set */
    FD_ZERO(&fdSet);

    /* get all of the file descriptors and add them to the set */
    maxFd = KWrapAddDevsToFdSet(numDevs, pDevCtx, &fdSet);

    /* if no device contexts were passed in then get out */
    if (maxFd == 0) {
        return FALSE;
    }

    /* Perform select() - Block until something happens */
    selRtnVal = select(maxFd + 1, &fdSet, NULL, NULL, pTimeout);

    if (selRtnVal < 0) {
        /* error condition */
        return FALSE;
    } else if (selRtnVal == 0) {
        /* timeout condtion */
        return FALSE;
    }

    return KWrapGetEventsForFdSet(pEvent, numDevs, pDevCtx, &fdSet);
}

static VpStatusType KWrapGetLineStatus(
    VpLineCtxType       *pLineCtx,
    VpInputType         input,
    bool                *pStatus)
{
    VpKWrapDeviceObjectType *pDevObj = (pLineCtx->pDevCtx)->pDevObj;
    VpKWrapLineObjectType *pLineObj = pLineCtx->pLineObj;

    /* init with const data */
    VpApiModGetLnStatusType ioGetLineStatus = {
        .lineRegNum     = pLineObj->lineRegNum,
        .input          = input,
        .pStatus        = pStatus
    };

    /* issue the corresponding ioctl */
    if (ioctl(pDevObj->fileDes, VPAPI_MOD_IOX_GET_LN_STATUS, &ioGetLineStatus) < 0) {
        return VP_STATUS_FAILURE;
    }

    return ioGetLineStatus.status;
}

static VpStatusType KWrapGetDeviceStatus(
    VpDevCtxType        *pDevCtx,
    VpInputType         input,
    uint32              *pDeviceStatus)
{
    VpKWrapDeviceObjectType *pDevObj = pDevCtx->pDevObj;

    /* init with const data */
    VpApiModGetDevStatusType ioGetDevStatus = {
        .input          = input,
        .pDeviceStatus  = pDeviceStatus
    };

    /* issue the corresponding ioctl */
    if (ioctl(pDevObj->fileDes, VPAPI_MOD_IOX_GET_DEV_STATUS, &ioGetDevStatus) < 0) {
        return VP_STATUS_FAILURE;
    }

    return ioGetDevStatus.status;
}

static VpStatusType KWrapGetDeviceStatusExt(
    VpDevCtxType       *pDevCtx,
    VpDeviceStatusType  *pDeviceStatus)
{
    VpKWrapDeviceObjectType *pDevObj = pDevCtx->pDevObj;

    /* init with const data */
    VpApiModGetDevStatusExtType ioGetDevStatusExt = {
        .pDeviceStatus  = pDeviceStatus
    };

    /* issue the corresponding ioctl */
    if (ioctl(pDevObj->fileDes, VPAPI_MOD_IOX_GET_DEV_STATUS_EXT, &ioGetDevStatusExt) < 0) {
        return VP_STATUS_FAILURE;
    }

    return ioGetDevStatusExt.status;
}

static VpStatusType KWrapGetLoopCond(
    VpLineCtxType       *pLineCtx,
    uint16              handle)
{
    VpKWrapDeviceObjectType *pDevObj = (pLineCtx->pDevCtx)->pDevObj;
    VpKWrapLineObjectType *pLineObj = pLineCtx->pLineObj;

    /* init with const data */
    VpApiModSetLnStType ioGetLoopCond = {
        .lineRegNum     = pLineObj->lineRegNum,
        .state          = handle,
    };

    /* issue the corresponding ioctl */
    if (ioctl(pDevObj->fileDes, VPAPI_MOD_IOX_GET_LP_COND, &ioGetLoopCond) < 0) {
        return VP_STATUS_FAILURE;
    }

    return ioGetLoopCond.status;
}

static VpStatusType KWrapGetOption(
    VpLineCtxType       *pLineCtx,
    VpDevCtxType        *pDevCtx,
    VpOptionIdType      option,
    uint16              handle)
{
    if (pDevCtx != VP_NULL) {
        /* send the set option at the device level */
        VpKWrapDeviceObjectType *pDevObj = pDevCtx->pDevObj;

        /* init with const data */
        VpApiModGetOptionType ioGetOption = {
            .lineRequest    = FALSE,
            .option         = option,
            .handle         = handle,
        };

        /* issue the corresponding ioctl */
        if (ioctl(pDevObj->fileDes, VPAPI_MOD_IOX_GET_OPTION, &ioGetOption) < 0) {
            return VP_STATUS_FAILURE;
        }

        return ioGetOption.status;

    } else if (pLineCtx != VP_NULL) {
        /* send the set option at the line level */
        VpKWrapDeviceObjectType *pDevObj = (pLineCtx->pDevCtx)->pDevObj;
        VpKWrapLineObjectType *pLineObj = pLineCtx->pLineObj;

        /* init with const data */
        VpApiModGetOptionType ioGetOption = {
            .lineRequest    = TRUE,
            .lineRegNum     = pLineObj->lineRegNum,
            .option         = option,
            .handle         = handle,
        };

        /* issue the corresponding ioctl */
        if (ioctl(pDevObj->fileDes, VPAPI_MOD_IOX_GET_OPTION, &ioGetOption) < 0) {
            return VP_STATUS_FAILURE;
        }

        return ioGetOption.status;
    } else {
        return VP_STATUS_INVALID_ARG;
    }
}

static VpStatusType KWrapGetLineState(
    VpLineCtxType       *pLineCtx,
    VpLineStateType     *pCurrentState)
{
    VpKWrapDeviceObjectType *pDevObj = (pLineCtx->pDevCtx)->pDevObj;
    VpKWrapLineObjectType *pLineObj = pLineCtx->pLineObj;

    /* init with const data */
    VpApiModGetLnStType ioGetLineState = {
        .lineRegNum     = pLineObj->lineRegNum,
        .pCurrentState  = pCurrentState,
    };

    /* issue the corresponding ioctl */
    if (ioctl(pDevObj->fileDes, VPAPI_MOD_IOX_GET_LN_ST, &ioGetLineState) < 0) {
        return VP_STATUS_FAILURE;
    }

    return ioGetLineState.status;
}

static VpStatusType KWrapFlushEvents(
    VpDevCtxType        *pDevCtx)
{
    VpKWrapDeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpApiModClrRsltType ioFlushEvents;

    /* issue the corresponding ioctl */
    if (ioctl(pDevObj->fileDes, VPAPI_MOD_IOX_FLSH_EVNT, &ioFlushEvents) < 0) {
        return FALSE;
    }

    return ioFlushEvents.status;
}


static VpStatusType KWrapGetResults(
    VpEventType         *pEvent,
    void                *pResults)
{
    VpKWrapDeviceObjectType *pDevObj = (pEvent->pDevCtx)->pDevObj;

    /* init with const data */
    VpApiModGetRsltType ioGetResults = {
        .pEvent         = pEvent,
        .pResults       = pResults
    };

    /* issue the corresponding ioctl */
    if (ioctl(pDevObj->fileDes, VPAPI_MOD_IOX_GET_RSLT, &ioGetResults) < 0) {
        return FALSE;
    }

    return ioGetResults.status;
}

static VpStatusType KWrapClearResults(
    VpDevCtxType        *pDevCtx)
{
    VpKWrapDeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpApiModClrRsltType ioClearResults;

    /* issue the corresponding ioctl */
    if (ioctl(pDevObj->fileDes, VPAPI_MOD_IOX_CLR_RSLT, &ioClearResults) < 0) {
        return FALSE;
    }

    return ioClearResults.status;
}


static VpStatusType KWrapCodeCheckSum(
    VpDevCtxType *pDevCtx,
    uint16 handle)
{
    VpKWrapDeviceObjectType *pDevObj = pDevCtx->pDevObj;

    /* init with const data */
    VpApiModCodeCkSumType ioCodeCkSum = {
        .handle  = handle
    };

    /* issue the corresponding ioctl */
    if (ioctl(pDevObj->fileDes, VPAPI_MOD_IOX_CODE_CK_SUM, &ioCodeCkSum) < 0) {
        return VP_STATUS_FAILURE;
    }

    return ioCodeCkSum.status;
}

static VpStatusType KWrapQuery(
    VpLineCtxType       *pLineCtx,
    VpQueryIdType       queryId,
    uint16              handle)
{
    VpKWrapDeviceObjectType *pDevObj = (pLineCtx->pDevCtx)->pDevObj;
    VpKWrapLineObjectType *pLineObj = pLineCtx->pLineObj;

    /* init with const data */
    VpApiModQueryType ioQuery = {
        .lineRegNum     = pLineObj->lineRegNum,
        .queryId        = queryId,
        .handle         = handle
    };

    /* issue the corresponding ioctl */
    if (ioctl(pDevObj->fileDes, VPAPI_MOD_IOX_QUERY, &ioQuery) < 0) {
        return VP_STATUS_FAILURE;
    }

    return ioQuery.status;
}

static VpStatusType KWrapGetRelayState(
    VpLineCtxType *pLineCtx,
    VpRelayControlType *pRstate)
{
    VpKWrapDeviceObjectType *pDevObj = (pLineCtx->pDevCtx)->pDevObj;
    VpKWrapLineObjectType *pLineObj = pLineCtx->pLineObj;

    /* init with const data */
    VpApiModGetRelayStType ioGetRelayState = {
        .lineRegNum     = pLineObj->lineRegNum,
        .pRstate        = pRstate,
    };

    /* issue the corresponding ioctl */
    if (ioctl(pDevObj->fileDes, VPAPI_MOD_IOX_GET_RELAY_ST, &ioGetRelayState) < 0) {
        return VP_STATUS_FAILURE;
    }

    return ioGetRelayState.status;
}

static VpStatusType KWrapReadTestBuf(
    VpLineCtxType       *pLineCtx,
    uint16              length,
    VpVectorPtrType     pData)

{
    VpKWrapDeviceObjectType *pDevObj = (pLineCtx->pDevCtx)->pDevObj;
    VpKWrapLineObjectType *pLineObj = pLineCtx->pLineObj;

    /* init with const data */
    VpApiModReadBuffType ioTestBuff = {
        .lineRegNum     = pLineObj->lineRegNum,
        .length         = length,
        .pData          = pData
    };

    /* issue the corresponding ioctl */
    if (ioctl(pDevObj->fileDes, VPAPI_MOD_IOX_READ_BUFF, &ioTestBuff) < 0) {
        return VP_STATUS_FAILURE;
    }

    return ioTestBuff.status;
}

static VpStatusType KWrapRegisterDump(
    VpDevCtxType        *pDevCtx)
{
    VpKWrapDeviceObjectType *pDevObj = pDevCtx->pDevObj;

    VpApiModRegisterDumpType ioRegisterDumpInfo;

    /* issue the corresponding ioctl */
    if (ioctl(pDevObj->fileDes, VPAPI_MOD_IOX_REGISTER_DUMP, &ioRegisterDumpInfo) < 0) {
        return VP_STATUS_FAILURE;
    }

    return ioRegisterDumpInfo.status;
}


/* VP-API Line Tests Functions */
static VpStatusType KWrapTestLine(
    VpLineCtxType       *pLineCtx,
    VpTestIdType        test,
    const void          *pArgs,
    uint16              handle)
{
    VpKWrapDeviceObjectType *pDevObj = (pLineCtx->pDevCtx)->pDevObj;
    VpKWrapLineObjectType *pLineObj = pLineCtx->pLineObj;

    /* init with const data */
    VpApiModTestLnType ioTestLine = {
        .lineRegNum     = pLineObj->lineRegNum,
        .test           = test,
        .pArgs          = pArgs,
        .handle         = handle
    };

    /* issue the corresponding ioctl */
    if (ioctl(pDevObj->fileDes, VPAPI_MOD_IOCTL_TEST_LN, &ioTestLine) < 0) {
        return VP_STATUS_FAILURE;
    }

    return ioTestLine.status;
}

EXTERN VpStatusType KWrapConfigRing(
    struct VpLineCtxType *pLineCtx,
    VpProfilePtrType pRingProfile,
    VpProfilePtrType pRingCadProfile)
{
    VpKWrapDeviceObjectType *pDevObj = (pLineCtx->pDevCtx)->pDevObj;
    VpKWrapLineObjectType *pLineObj = pLineCtx->pLineObj;

    /* init with const data */
    VpApiModCfgRingType ioCfgRing = {
        .lineRegNum       = pLineObj->lineRegNum,
        .pRingProfile     = pRingProfile,
        .pRingCadProfile  = pRingCadProfile
    };

    /* issue the corresponding ioctl */
    if (ioctl(pDevObj->fileDes, VPAPI_MOD_IOCTL_CFG_RING, &ioCfgRing) < 0) {
        return VP_STATUS_FAILURE;
    }

    return ioCfgRing.status;
}

EXTERN VpStatusType KWrapSendCid(
    struct VpLineCtxType *pLineCtx,
    VpProfilePtrType pCidProfile,
    uint8 cidMsgLen,
    uint8 *pCidMsgData,
    KwrapCidLineStateType cidLineState)
{
    VpKWrapDeviceObjectType *pDevObj = (pLineCtx->pDevCtx)->pDevObj;
    VpKWrapLineObjectType *pLineObj = pLineCtx->pLineObj;

    /* init with const data */
    VpApiModSendCidType ioSendCid = {
        .lineRegNum      = pLineObj->lineRegNum,
        .pCidProfile     = pCidProfile,
        .cidMsgLen       = cidMsgLen,
        .pCidMsgData     = pCidMsgData,
        .lineState       = cidLineState
    };

    /* issue the corresponding ioctl */
    if (ioctl(pDevObj->fileDes, VPAPI_MOD_IOCTL_SEND_CID, &ioSendCid) < 0) {
        return VP_STATUS_FAILURE;
    }

    return ioSendCid.status;
}


EXTERN bool KWrapReadCalFlag(
    VpLineCtxType *pLineCtx)
{
    VpKWrapDeviceObjectType *pDevObj = (pLineCtx->pDevCtx)->pDevObj;
    VpKWrapLineObjectType *pLineObj = pLineCtx->pLineObj;

    /* init with const data */
    VpApiModReadCalType ioReadCalFlag = {
        .lineRegNum      = pLineObj->lineRegNum,
    };

    /* issue the corresponding ioctl */
    if (ioctl(pDevObj->fileDes, VPAPI_MOD_IOCTL_READ_CAL_FLAG, &ioReadCalFlag) < 0) {
        return FALSE;
    }

    return ioReadCalFlag.flag;

}

EXTERN void KWrapSetCalFlag(
    VpLineCtxType *pLineCtx)
{

    VpKWrapDeviceObjectType *pDevObj = (pLineCtx->pDevCtx)->pDevObj;
    VpKWrapLineObjectType *pLineObj = pLineCtx->pLineObj;

    /* init with const data */
    VpApiModSetCalType ioSetCalFlag = {
        .lineRegNum      = pLineObj->lineRegNum,
    };

    /* issue the corresponding ioctl */
    if (ioctl(pDevObj->fileDes, VPAPI_MOD_IOCTL_SET_CAL_FLAG, &ioSetCalFlag) < 0) {
        return;
    }

    return;
}


#endif /* VP_DEV_KWRAP */
