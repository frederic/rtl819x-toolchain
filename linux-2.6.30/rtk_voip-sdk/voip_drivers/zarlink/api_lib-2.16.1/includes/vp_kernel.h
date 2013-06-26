/*
 * vp_kernel.h
 *
 * This file declares the VCP Host Bus Interface layer.
 *
 * Copyright (c) 2010, Zarlink Semiconductor, Inc.
 *
 * $Revision: 6419 $
 * $LastChangedDate: 2010-02-12 16:40:10 -0600 (Fri, 12 Feb 2010) $
 */


#ifndef _VP_KERNEL_H
#define _VP_KERNEL_H

#include <stdarg.h>     /* Header file required for variable args */
#include <sys/types.h>  /* Header files required for open() */
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>     /* Header file required for close() */
#include <sys/ioctl.h>  /* Header file required for ioctl() */
#include <sys/select.h> /* Header file required for poll/select */


#include <vpapi_mod.h> /* Header file required for api kernel mod */

/* Kernel specific Device Object */
typedef struct {
    VpDeviceIdType      deviceId;       /* Device Id indication */

    int                 fileDes;        /* File descriptor */

    VpModDevRegNumType  devRegNum;       /* Vp-api module regisrtation number
                                         * associated with this deviceId
                                         */
} VpKWrapDeviceObjectType;

/* Kernel specific Line Object */
typedef struct {

    uint8               channelId;

    VpLineIdType        lineId;         /* Application provided value for mapping a
                                         * line to a line context */

    VpModLineRegNumType lineRegNum;     /* Vp-api module regisrtation number
                                         * associated with this lineId
                                         */

    VpTermType          termType;       /* Termination type for this line */

} VpKWrapLineObjectType;

typedef enum KwrapCidLineState {
    KWRAP_LINE_STATE_SAME           = CID_LINE_STATE_SAME,
    KWRAP_LINE_STATE_RINGING        = CID_LINE_STATE_RINGING,
    KWRAP_LINE_STATE_RINGING_POLREV = CID_LINE_STATE_RINGING_POLREV,
    KWRAP_LINE_STATE_ENUM_RSVD      = FORCE_SIGNED_ENUM,
    KWRAP_LINE_STATE_ENUM_SIZE      = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req. */
} KwrapCidLineStateType;

typedef struct VpKWrapLineIdInfo {
    VpTermType                  termType;
    uint8                       channelId;
    VpDeviceType                deviceType;
    VpDeviceIdType              deviceId;
} VpKWrapLineIdInfoType;

EXTERN VpStatusType KWrapGetDeviceInfo(
    VpDevCtxType        *pDevCtx,
    VpDeviceInfoType    *pDeviceInfo);

EXTERN VpStatusType KWrapGetLineIdInfo(
    const VpLineIdType      lineId,
    VpKWrapLineIdInfoType   *pLineIdInfo);

EXTERN VpStatusType KWrapGetDevLineSummary(
    VpApiModRegSumInfoType *pSummaryInfo);

EXTERN VpStatusType KWrapCloseDevice(
    VpDevCtxType            *pDevCtx);

EXTERN VpStatusType KWrapOpenDevice(
    const VpDeviceType      deviceType,
    const VpDeviceIdType    deviceId,
    const uint16            numFifoBytes,
    const uint16            userData,
    VpDevCtxType            *pDevCtx,
    VpKWrapDeviceObjectType *pDevObj);

EXTERN VpStatusType KWrapOpenLine(
    const VpTermType        termType,
    const uint8             channelId,
    const VpLineIdType      lineId,
    VpLineCtxType           *pLineCtx,
    VpKWrapLineObjectType   *pLineObj,
    VpDevCtxType            *pDevCtx);

EXTERN VpStatusType KWrapMapLineId(
    VpLineCtxType       *pLineCtx,
    VpLineIdType        lineId);

EXTERN int KWrapAddDevsToFdSet(
    uint8               numDevs,
    VpDevCtxType        *pDevCtx[],
    fd_set              *pFdSet);

EXTERN bool KWrapGetEventsForFdSet(
    VpEventType         *pEvent,
    uint8               numDevs,
    VpDevCtxType        *pDevCtx[],
    fd_set              *pFdSet);

EXTERN bool KWrapGetEventBlocking(
    time_t              sec,
    suseconds_t         usec,
    VpEventType         *pEvent,
    uint8               numDevs,
    VpDevCtxType        *pDevCtx[]);

EXTERN VpStatusType KWrapConfigRing(
    struct VpLineCtxType *pLineCtx,
    VpProfilePtrType pRingProfile,
    VpProfilePtrType pRingCadProfile);

EXTERN VpStatusType KWrapSendCid(
    struct VpLineCtxType *pLineCtx,
    VpProfilePtrType pCidProfile,
    uint8 cidMsgLen,
    uint8 *pCidMsgData,
    KwrapCidLineStateType cidLineState);

EXTERN bool KWrapReadCalFlag(
    VpLineCtxType *pLineCtx);

EXTERN void KWrapSetCalFlag(
    VpLineCtxType *pLineCtx);

/*
 *  KWrapSetOption -
 *      Must be here because it is possible to enable debug before device and/or
 *      line contexts are created.
 */
EXTERN VpStatusType KWrapSetOption(
    VpLineCtxType       *pLineCtx,
    VpDevCtxType        *pDevCtx,
    VpOptionIdType      option,
    void                *pValue);


#endif /* _VP_KERNEL_H */
