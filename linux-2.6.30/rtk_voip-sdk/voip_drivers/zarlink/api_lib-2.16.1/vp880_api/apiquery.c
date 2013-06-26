/** \file apiquery.c
 * apiquery.c
 *
 *  This file contains the query functions used in the Vp880 device API.
 *
 * Copyright (c) 2010, Zarlink Semiconductor, Inc.
 *
 * $Revision: 6680 $
 * $LastChangedDate: 2010-03-15 16:01:52 -0500 (Mon, 15 Mar 2010) $
 */

#include "vp_api_cfg.h"

#if defined (VP_CC_880_SERIES)

/* Project Includes */
#include "vp_api_types.h"
#include "sys_service.h"
#include "vp_hal.h"
#include "vp_api_int.h"
#include "vp880_api.h"
#include "vp880_api_int.h"

#ifdef VP880_INCLUDE_TESTLINE_CODE
#include "vp_api_test.h"
#endif

/* Private Functions */
static uint16 Vp880CheckLineEvent(uint16 event, uint16 eventMask,
    VpEventCategoryType eventCat, Vp880LineObjectType *pLineObj);
static uint16 Vp880CheckDevEvent(uint16 event, uint16 eventMask,
    VpEventCategoryType eventCat, Vp880DeviceObjectType *pDevObj);

static void Vp880ServiceFxsTimers(VpLineCtxType *pLineCtx);
static void Vp880ServiceFxoTimers(VpLineCtxType *pLineCtx);

static VpStatusType
Vp880GetDeviceOption(VpDevCtxType *pDevCtx, VpOptionIdType option,
    uint16 handle);

static void Vp880ServiceLpChangeTimer(VpDevCtxType *pDevCtx);
static void Vp880ServiceDisconnectExitTimer(VpLineCtxType *pLineCtx);

/**
 * Vp880FindSoftwareInterrupts()
 *  This function checks for active non-masked device and line events.
 *
 * Preconditions:
 *  None.
 *
 * Postconditions:
 *  Returns true if there is an active, non-masked event on either the device
 * or on a line associated with the device.
 */
bool
Vp880FindSoftwareInterrupts(
    VpDevCtxType *pDevCtx)
{
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    Vp880LineObjectType *pLineObj;
    VpLineCtxType *pLineCtx;
    uint8 channelId;
    uint8 maxChan = pDevObj->staticInfo.maxChannels;

    VpOptionEventMaskType eventsMask = pDevObj->deviceEventsMask;
    VpOptionEventMaskType *pEvents = &(pDevObj->deviceEvents);

    /* First clear all device events that are masked */
    pEvents->faults &= ~(eventsMask.faults);
    pEvents->signaling &= ~(eventsMask.signaling);
    pEvents->response &= ~(eventsMask.response);
    pEvents->process &= ~(eventsMask.process);
    pEvents->test &= ~(eventsMask.test);
    pEvents->fxo &= ~(eventsMask.fxo);

    /* Evaluate if any events remain */
    if((pEvents->faults) || (pEvents->signaling) || (pEvents->response)
    || (pEvents->process) || (pEvents->test) || (pEvents->fxo)) {
        return TRUE;
    }

    for (channelId = 0; channelId < maxChan; channelId++) {
        pLineCtx = pDevCtx->pLineCtx[channelId];
        if(pLineCtx != VP_NULL) {
            pLineObj = pLineCtx->pLineObj;
            eventsMask = pLineObj->lineEventsMask;
            pEvents = &(pLineObj->lineEvents);

            /* Clear the line events that are masked */
            pEvents->faults &= ~(eventsMask.faults);
            pEvents->signaling &= ~(eventsMask.signaling);
            pEvents->response &= ~(eventsMask.response);
            pEvents->process &= ~(eventsMask.process);
            pEvents->test &= ~(eventsMask.test);
            pEvents->fxo &= ~(eventsMask.fxo);

            /* Evaluate if any events remain */
            if(pEvents->faults || pEvents->signaling || pEvents->response
            || pEvents->process || pEvents->test || pEvents->fxo) {
                return TRUE;
            }
        }
    }

    return FALSE;
}

/**
 * Vp880GetEvent()
 *  This function reports new events that occured on the device. This function
 * returns one event for each call to it. It should be called repeatedly until
 * no more events are reported for a specific device.  This function does not
 * access the device, it returns status from the phantom registers that are
 * maintained by the API tick routine.
 *
 * Preconditions:
 *  None. All error checking required is assumed to exist in common interface
 * file.
 *
 * Postconditions:
 *  Returns true if there is an active event for the device.
 */
bool
Vp880GetEvent(
    VpDevCtxType *pDevCtx,
    VpEventType *pEvent)    /**< Pointer to the results event structure */
{
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    Vp880LineObjectType *pLineObj;
    VpLineCtxType *pLineCtx;
    VpDeviceIdType deviceId = pDevObj->deviceId;

    uint8 i, eventCatLoop;
    uint8 chan;
    uint8 maxChan = pDevObj->staticInfo.maxChannels;

#ifdef VP880_INCLUDE_TESTLINE_CODE
#define EVENT_ARRAY_SIZE 6
#else
#define EVENT_ARRAY_SIZE 5
#endif

    uint16 eventArray[EVENT_ARRAY_SIZE];
    uint16 eventMaskArray[EVENT_ARRAY_SIZE];
    VpEventCategoryType eventCat[EVENT_ARRAY_SIZE] = {
        VP_EVCAT_FAULT,
        VP_EVCAT_SIGNALING,
        VP_EVCAT_RESPONSE,
        VP_EVCAT_PROCESS,
        VP_EVCAT_FXO
#ifdef VP880_INCLUDE_TESTLINE_CODE
       ,VP_EVCAT_TEST
#endif
    };

    VP_API_FUNC(VpDevCtxType, pDevCtx, ("Vp880GetEvent+"));

    pEvent->status = VP_STATUS_SUCCESS;
    pEvent->hasResults = FALSE;

    /* Initialize the arrays for device events */
    for (i = 0; i < EVENT_ARRAY_SIZE; i++) {
        switch(eventCat[i]) {
            case VP_EVCAT_FAULT:
                eventArray[i] = pDevObj->deviceEvents.faults;
                eventMaskArray[i] = pDevObj->deviceEventsMask.faults;
                break;

            case VP_EVCAT_SIGNALING:
                eventArray[i] = pDevObj->deviceEvents.signaling;
                eventMaskArray[i] = pDevObj->deviceEventsMask.signaling;
                break;

            case VP_EVCAT_RESPONSE:
                eventArray[i] = pDevObj->deviceEvents.response;
                eventMaskArray[i] = pDevObj->deviceEventsMask.response;
                break;

            case VP_EVCAT_PROCESS:
                eventArray[i] = pDevObj->deviceEvents.process;
                eventMaskArray[i] = pDevObj->deviceEventsMask.process;
                break;

            case VP_EVCAT_FXO:
                eventArray[i] = pDevObj->deviceEvents.fxo;
                eventMaskArray[i] = pDevObj->deviceEventsMask.fxo;
                break;

#ifdef VP880_INCLUDE_TESTLINE_CODE
            case VP_EVCAT_TEST:
                eventArray[i] = pDevObj->deviceEvents.test;
                eventMaskArray[i] = pDevObj->deviceEventsMask.test;
                break;
#endif

            default:
                /* This can only occur if there's a bug in this code */
                break;
        }
    }

    VpSysEnterCritical(deviceId, VP_CODE_CRITICAL_SEC);

    /* Look for active device events first */
    for (eventCatLoop = 0; eventCatLoop < EVENT_ARRAY_SIZE; eventCatLoop++) {
        pEvent->eventId = Vp880CheckDevEvent(eventArray[eventCatLoop],
            eventMaskArray[eventCatLoop], eventCat[eventCatLoop], pDevObj);
        if (pEvent->eventId != 0x0000) {
            pEvent->deviceId = deviceId;
            pEvent->channelId = 0;
            pEvent->eventCategory = eventCat[eventCatLoop];
            pEvent->pDevCtx = pDevCtx;
            pEvent->pLineCtx = VP_NULL;
            pEvent->parmHandle = pDevObj->eventHandle;
            pEvent->hasResults = FALSE;

            if (pEvent->eventCategory == VP_EVCAT_RESPONSE) {
                /*
                 * For the events that require a read operation, set the has
                 * results indicator in the event structure
                 */

                switch (pEvent->eventId) {
                    case VP_LINE_EVID_RD_OPTION:
                        pEvent->channelId = pDevObj->getResultsOption.chanId;
                        pEvent->pLineCtx = pDevCtx->pLineCtx[pEvent->channelId];
                        if (pEvent->pLineCtx != VP_NULL) {
                            Vp880LineObjectType *pLineObj = pEvent->pLineCtx->pLineObj;
                            pEvent->lineId = pLineObj->lineId;
                        }
                        pEvent->hasResults = TRUE;
                        pEvent->eventData = pDevObj->getResultsOption.optionType;
                        break;

                    case VP_DEV_EVID_IO_ACCESS_CMP:
                        pEvent->hasResults = TRUE;
                        break;

                    case VP_DEV_EVID_DEV_INIT_CMP:
                        pEvent->eventData = 1;
                        pEvent->channelId = 0;
                        break;

                    case VP_EVID_CAL_CMP:
                        pEvent->eventData = (uint16)pDevObj->responseData;
                        break;

                    default:
                        break;
                }
            }

            if (pEvent->eventCategory == VP_EVCAT_FAULT) {
                switch(pEvent->eventId) {
                    case VP_DEV_EVID_CLK_FLT:
                        pEvent->eventData =
                            (pDevObj->dynamicInfo.clkFault ? TRUE : FALSE);
                        break;

                    case VP_DEV_EVID_BAT_FLT:
                        if ((pDevObj->dynamicInfo.bat1Fault == TRUE)
                         || (pDevObj->dynamicInfo.bat2Fault == TRUE)
                         || (pDevObj->dynamicInfo.bat3Fault == TRUE)) {
                            pEvent->eventData = TRUE;
                        } else {
                            pEvent->eventData = FALSE;
                        }
                        break;

                    default:
                        break;
                }
            }
            VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
            VP_API_FUNC(VpDevCtxType, pDevCtx, ("Vp880GetEvent Error - Unknon Device Event Category"));
            return TRUE;
        }
    }

    /*
     * No device events, now look for Line events -- but make sure the line
     * context is valid before looking for a line object
     */
    for(chan = pDevObj->dynamicInfo.lastChan; chan < maxChan; chan++) {
        pLineCtx = pDevCtx->pLineCtx[chan];
        if (pLineCtx != VP_NULL) {
            pLineObj = pLineCtx->pLineObj;
            /* The line context is valid, create a line object and initialize
             * the event arrays for this line
             */
            for (i = 0; i < EVENT_ARRAY_SIZE; i++) {
                switch(eventCat[i]) {
                    case VP_EVCAT_FAULT:
                        eventArray[i] = pLineObj->lineEvents.faults;
                        eventMaskArray[i] = pLineObj->lineEventsMask.faults;
                        break;

                    case VP_EVCAT_SIGNALING:
                        eventArray[i] = pLineObj->lineEvents.signaling;
                        eventMaskArray[i] = pLineObj->lineEventsMask.signaling;
                        break;

                    case VP_EVCAT_RESPONSE:
                        eventArray[i] = pLineObj->lineEvents.response;
                        eventMaskArray[i] = pLineObj->lineEventsMask.response;
                        break;

                    case VP_EVCAT_PROCESS:
                        eventArray[i] = pLineObj->lineEvents.process;
                        eventMaskArray[i] = pLineObj->lineEventsMask.process;
                        break;

                    case VP_EVCAT_FXO:
                        eventArray[i] = pLineObj->lineEvents.fxo;
                        eventMaskArray[i] = pLineObj->lineEventsMask.fxo;
                        break;

#ifdef VP880_INCLUDE_TESTLINE_CODE
                    case VP_EVCAT_TEST:
                        eventArray[i] = pLineObj->lineEvents.test;
                        eventMaskArray[i] = pLineObj->lineEventsMask.test;
                        break;
#endif
                    default:
                        /* This can only occur if there's a bug in this code */
                        break;

                }
            }

            /* Check this line events */
            for (eventCatLoop = 0;
                 eventCatLoop < EVENT_ARRAY_SIZE;
                 eventCatLoop++) {
                pEvent->eventId = Vp880CheckLineEvent(eventArray[eventCatLoop],
                    eventMaskArray[eventCatLoop], eventCat[eventCatLoop],
                    pLineObj);

                if (pEvent->eventId != 0x0000) {
                    pEvent->deviceId = deviceId;
                    pEvent->channelId = chan;
                    pEvent->pLineCtx = pDevCtx->pLineCtx[chan];
                    pEvent->pDevCtx = pDevCtx;
                    pEvent->eventCategory = eventCat[eventCatLoop];
                    pEvent->parmHandle = pLineObj->lineEventHandle;
                    pEvent->lineId = pLineObj->lineId;
                    pEvent->hasResults = FALSE;

                    switch(pEvent->eventCategory) {
                        case VP_EVCAT_RESPONSE:
                            pEvent->eventData = (uint16)pLineObj->responseData;
                            switch(pEvent->eventId) {
                                case VP_LINE_EVID_LLCMD_RX_CMP:
                                    pEvent->eventData = pDevObj->mpiLen;
                                case VP_LINE_EVID_GAIN_CMP:
                                case VP_LINE_EVID_RD_LOOP:
                                    pEvent->hasResults = TRUE;
                                    break;

                                case VP_EVID_CAL_CMP:
                                    if (pLineObj->responseData == (uint8)VP_CAL_GET_SYSTEM_COEFF) {
                                        pEvent->eventData = pDevObj->mpiLen;
                                        pEvent->hasResults = TRUE;
                                        /*
                                         * Prevent future cal complete events from being
                                         * indicated as having results data.
                                         */
                                        pLineObj->responseData = (uint8)VP_CAL_ENUM_SIZE;
                                    }
                                    break;

                                default:
                                    break;
                            }
                            break;

                        case VP_EVCAT_SIGNALING:
                            if (pEvent->eventId == VP_LINE_EVID_DTMF_DIG) {
                                /*
                                 * Upper bits are used for the timestamp.
                                 * Lower bits are used for the digit and the
                                 * make/break bit.
                                 */
                                pEvent->eventData = (pDevObj->timeStamp << 5)
                                    | pLineObj->dtmfDigitSense;
                            } else {
                                pEvent->eventData = pLineObj->signalingData;

                                if (pEvent->eventId == VP_LINE_EVID_HOOK_OFF) {
                                    pLineObj->status |= VP880_PREVIOUS_HOOK;
                                } else if (pEvent->eventId == VP_LINE_EVID_HOOK_ON) {
                                    pLineObj->status &= ~VP880_PREVIOUS_HOOK;
                                }
                            }
                            break;

                        case VP_EVCAT_FXO:
                            pEvent->eventData = pLineObj->fxoData;
                            break;

                        case VP_EVCAT_PROCESS:
                            pEvent->eventData = pLineObj->processData;
                            break;

                        case VP_EVCAT_FAULT:
                            if (pEvent->eventId == VP_LINE_EVID_THERM_FLT) {
                                pEvent->eventData =
                                    (pLineObj->lineState.condition
                                        & VP_CSLAC_THERM_FLT) ? TRUE : FALSE;
                            }
                            break;

                        case VP_EVCAT_TEST:
                            if ( VP_LINE_EVID_TEST_CMP == pEvent->eventId) {
                                pEvent->eventData = pDevObj->testResults.testId;
                                pEvent->hasResults = TRUE;
                            }
                            break;

                        default:
                            break;
                    }

                    /*
                     * We're returning, so update the device last channel that
                     * was checked so we start at the next channel
                     */
                    pDevObj->dynamicInfo.lastChan = chan + 1;
                    if (pDevObj->dynamicInfo.lastChan >= maxChan) {
                        pDevObj->dynamicInfo.lastChan = 0;
                    }
                    VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
                    VP_API_FUNC(VpDevCtxType, pDevCtx, ("Vp880GetEvent Error - Unknon Line Event Category"));
                    return TRUE;
                }
            }
        }
    }
    VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);

    VP_API_FUNC(VpDevCtxType, pDevCtx, ("Vp880GetEvent-"));

    return FALSE;
} /* End Vp880GetEvent */

/**
 * Vp880CheckDevEvent()
 *  This function performs a check on active device events and compares the
 * event with the event mask.  The event is cleared, and if the event is
 * unmasked it gets returned to the calling function via the return value.
 *
 * Preconditions:
 *  None. This is an internal API function call only and it is assumed all error
 * checking necessary is performed by higher level functions.
 *
 * Postconditions:
 *  If the returned value is other than 0x0000, the event being returned is
 * cleared in the device object.
 */
uint16
Vp880CheckDevEvent(
    uint16 event,
    uint16 eventMask,
    VpEventCategoryType eventCat,
    Vp880DeviceObjectType *pDevObj)
{
    uint8 i;
    uint16 mask;

    VP_API_FUNC_INT(None, NULL, ("Vp880CheckDevEvent+"));

    for (i = 0, mask = 0x0001; i < 16; i++, (mask = mask << 1)) {
        /* Check to see if an event MAY be reported */
        if ((mask & event) != 0) {
            /*
             * Have to clear the device event so we don't report this event
             * again
             */
            switch(eventCat) {
                case VP_EVCAT_FAULT:
                    pDevObj->deviceEvents.faults &= (~mask);
                    break;

                case VP_EVCAT_SIGNALING:
                    pDevObj->deviceEvents.signaling &= (~mask);
                    break;

                case VP_EVCAT_RESPONSE:
                    pDevObj->deviceEvents.response &= (~mask);
                    break;

                case VP_EVCAT_PROCESS:
                    pDevObj->deviceEvents.process &= (~mask);
                    break;

                case VP_EVCAT_FXO:
                    pDevObj->deviceEvents.fxo &= (~mask);
                    break;

#ifdef VP880_INCLUDE_TESTLINE_CODE
                case VP_EVCAT_TEST:
                    pDevObj->deviceEvents.test &= (~mask);
                    break;
#endif
                default:
                    break;
            }

            /* If the event is not masked, return the event */
            if ((mask & eventMask) == 0) {
                VP_API_FUNC_INT(None, NULL, ("Vp880CheckDevEvent-"));
                return mask;
            }
        }
    }
    VP_API_FUNC_INT(None, NULL, ("Vp880CheckDevEvent-"));
    return 0x0000;
}

/**
 * Vp880CheckLineEvent()
 *  This function performs a check on active line events and compares the
 * event with the event mask.  The event is cleared, and if the event is
 * unmasked it gets returned to the calling function via the return value.
 *
 * Preconditions:
 *  None. This is an internal API function call only and it is assumed all error
 * checking necessary is performed by higher level functions.
 *
 * Postconditions:
 *  If the returned value is other than 0x0000, the event being returned is
 * cleared in the line object.
 */
uint16
Vp880CheckLineEvent(
    uint16 event,
    uint16 eventMask,
    VpEventCategoryType eventCat,
    Vp880LineObjectType *pLineObj)
{
    uint8 i;
    uint16 mask;

    VP_API_FUNC_INT(None, NULL, ("Vp880CheckLineEvent+"));

    for (i = 0, mask = 0x0001; i < 16; i++, (mask = mask << 1)) {
        /* Check to see if an event MAY be reported */
        if ((mask & event) != 0) {
            /*
             * Have to clear the line event so we don't report this event
             * again
             */
            switch(eventCat) {
                case VP_EVCAT_FAULT:
                    pLineObj->lineEvents.faults &= (~mask);
                    break;

                case VP_EVCAT_SIGNALING:
                    pLineObj->lineEvents.signaling &= (~mask);
                    break;

                case VP_EVCAT_RESPONSE:
                    pLineObj->lineEvents.response &= (~mask);
                    break;

                case VP_EVCAT_PROCESS:
                    pLineObj->lineEvents.process &= (~mask);
                    break;

                case VP_EVCAT_FXO:
                    pLineObj->lineEvents.fxo &= (~mask);
                    break;

#ifdef VP880_INCLUDE_TESTLINE_CODE
               case VP_EVCAT_TEST:
                    pLineObj->lineEvents.test &= (~mask);
                    break;
#endif

                default:
                    break;
            }

            /* If the event is not masked, return the event */
            if ((mask & eventMask) == 0) {
                VP_API_FUNC_INT(None, NULL, ("Vp880CheckLineEvent-"));
                return mask;
            }
        }
    }
    VP_API_FUNC_INT(None, NULL, ("Vp880CheckLineEvent-"));
    return 0x0000;
}

/**
 * Vp880GetOption()
 *  This function accesses the option being requested, fills the device object
 * with the data to be returned, and sets the Read Option complete event.
 *
 * Preconditions:
 *  None. All error checking required is assumed to exist in common interface
 * file.
 *
 * Postconditions:
 *  The device object is filled with the results of the option type being
 * requested and the Read Option Event flag is set.  This function returns the
 * success code if the option type being requested is supported.
 */
VpStatusType
Vp880GetOption(
    VpLineCtxType *pLineCtx,
    VpDevCtxType *pDevCtx,
    VpOptionIdType option,
    uint16 handle)
{
    Vp880LineObjectType *pLineObj;
    Vp880DeviceObjectType *pDevObj;
    VpStatusType status = VP_STATUS_SUCCESS;
    VpGetResultsOptionsDataType *pOptionData;

    uint8 channelId, txSlot, rxSlot;
    VpDeviceIdType deviceId;
    uint8 tempLoopBack[VP880_LOOPBACK_LEN];
    uint8 tempSysConfig;
    uint8 ecVal;

    if (pLineCtx != VP_NULL) {
        VpDevCtxType *pDevCtxLocal = pLineCtx->pDevCtx;
        pDevObj = pDevCtxLocal->pDevObj;
        deviceId = pDevObj->deviceId;
        pLineObj = pLineCtx->pLineObj;
        ecVal = pLineObj->ecVal;
        channelId = pLineObj->channelId;
        pOptionData = &(pDevObj->getResultsOption.optionData);

        VP_API_FUNC(None, NULL, ("Vp880GetOption (Line)+"));

        if (pDevObj->deviceEvents.response & VP880_READ_RESPONSE_MASK) {
            VP_API_FUNC(VpLineCtxType, pLineCtx, ("Vp880GetOption (Line) Error - VP_STATUS_DEVICE_BUSY"));
            return VP_STATUS_DEVICE_BUSY;
        }

        /* Do not allow FXS specific options on an FXO line */
        if (pLineObj->status & VP880_IS_FXO) {
            switch(option) {
                case VP_OPTION_ID_ZERO_CROSS:
                case VP_OPTION_ID_PULSE_MODE:
                case VP_OPTION_ID_LINE_STATE:
                case VP_OPTION_ID_RING_CNTRL:
                    VP_API_FUNC(VpLineCtxType, pLineCtx, ("Vp880GetOption (Line) Error - VP_STATUS_INVALID_ARG"));
                    return VP_STATUS_INVALID_ARG;
                default:
                    break;
            }
        }

        /*
         * If this function can be executed, we will either access the MPI
         * and/or shared data. So it is best to label the entire function as
         * Code Critical so the data being accessed cannot be changed while
         * trying to be accessed
         */
        VpSysEnterCritical(deviceId, VP_CODE_CRITICAL_SEC);

        pDevObj->getResultsOption.chanId = channelId;

        switch (option) {
            /* Line Options */
            case VP_OPTION_ID_PULSE_MODE:
                pOptionData->pulseModeOption = pLineObj->pulseMode;
                break;

            case VP_OPTION_ID_TIMESLOT:
                VpMpiCmdWrapper(deviceId, ecVal, VP880_TX_TS_RD,
                    VP880_TX_TS_LEN, &txSlot);

                VpMpiCmdWrapper(deviceId, ecVal, VP880_RX_TS_RD,
                    VP880_RX_TS_LEN, &rxSlot);

                pOptionData->timeSlotOption.tx = (txSlot & VP880_TX_TS_MASK);

                if (pDevObj->staticInfo.rcnPcn[1] == VP880_DEV_PCN_88536) {
                    pOptionData->timeSlotOption.tx++;
                }

                pOptionData->timeSlotOption.rx = (rxSlot & VP880_RX_TS_MASK);
                break;

            case VP_OPTION_ID_CODEC:
                pOptionData->codecOption = pLineObj->codec;
                break;

            case VP_OPTION_ID_PCM_HWY:
                pOptionData->pcmHwyOption = VP_OPTION_HWY_A;
                break;

            case VP_OPTION_ID_LOOPBACK:
                /* Timeslot loopback via loopback register */
                VpMpiCmdWrapper(deviceId, ecVal, VP880_LOOPBACK_RD,
                    VP880_LOOPBACK_LEN, tempLoopBack);

                if ((tempLoopBack[0] & VP880_INTERFACE_LOOPBACK_EN) ==
                     VP880_INTERFACE_LOOPBACK_EN) {
                    pOptionData->loopBackOption = VP_OPTION_LB_TIMESLOT;
                } else {
                    pOptionData->loopBackOption = VP_OPTION_LB_OFF;
                }
                break;

            case VP_OPTION_ID_LINE_STATE:
                /* Battery control is automatic, so force it */
                pOptionData->lineStateOption.bat = VP_OPTION_BAT_AUTO;

                /* Smooth/Abrupt PolRev is controlled in the device */
                VpMpiCmdWrapper(deviceId, ecVal, VP880_SS_CONFIG_RD,
                    VP880_SS_CONFIG_LEN, &tempSysConfig);

                if (tempSysConfig & VP880_SMOOTH_PR_EN) {
                    pOptionData->lineStateOption.battRev = FALSE;
                } else {
                    pOptionData->lineStateOption.battRev = TRUE;
                }
                break;

            case VP_OPTION_ID_EVENT_MASK:
                /*
                 * In SetOption(), we force all line-specific bits in the
                 * deviceEventsMask to zero.  Likewise, we force all device-
                 * specific bits in the lineEventsMask to zero.  This allows
                 * us to simply OR the two together here.
                 */
                pOptionData->eventMaskOption.faults =
                    pLineObj->lineEventsMask.faults |
                    pDevObj->deviceEventsMask.faults;
                pOptionData->eventMaskOption.signaling =
                    pLineObj->lineEventsMask.signaling |
                    pDevObj->deviceEventsMask.signaling;
                pOptionData->eventMaskOption.response =
                    pLineObj->lineEventsMask.response |
                    pDevObj->deviceEventsMask.response;
                pOptionData->eventMaskOption.test =
                    pLineObj->lineEventsMask.test |
                    pDevObj->deviceEventsMask.test;
                pOptionData->eventMaskOption.process =
                    pLineObj->lineEventsMask.process |
                    pDevObj->deviceEventsMask.process;
                pOptionData->eventMaskOption.fxo =
                    pLineObj->lineEventsMask.fxo |
                    pDevObj->deviceEventsMask.fxo;
                break;

            case VP_OPTION_ID_ZERO_CROSS:
                pOptionData->zeroCross = pLineObj->ringCtrl.zeroCross;
                break;

            case VP_OPTION_ID_RING_CNTRL:
                pOptionData->ringControlOption = pLineObj->ringCtrl;
                break;

            case VP_OPTION_ID_PCM_TXRX_CNTRL:
                pOptionData->pcmTxRxCtrl = pLineObj->pcmTxRxCtrl;
                break;

            case VP_DEVICE_OPTION_ID_PULSE:
            case VP_DEVICE_OPTION_ID_PULSE2:
            case VP_DEVICE_OPTION_ID_CRITICAL_FLT:
            case VP_DEVICE_OPTION_ID_DEVICE_IO:
                status = Vp880GetDeviceOption(pDevCtxLocal, option, handle);
                break;

            default:
                status = VP_STATUS_OPTION_NOT_SUPPORTED;
                break;
        }
        VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
        VP_API_FUNC(VpLineCtxType, pLineCtx, ("Vp880GetOption (Line)-"));
    } else {
        pDevObj = pDevCtx->pDevObj;
        deviceId = pDevObj->deviceId;

        VpSysEnterCritical(deviceId, VP_CODE_CRITICAL_SEC);
        status = Vp880GetDeviceOption(pDevCtx, option, handle);
        VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
    }

    if (status == VP_STATUS_SUCCESS) {
        VpSysEnterCritical(deviceId, VP_CODE_CRITICAL_SEC);

        pDevObj->getResultsOption.optionType = option;
        pDevObj->deviceEvents.response |= VP_LINE_EVID_RD_OPTION;
        pDevObj->eventHandle = handle;

        VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
    }

    return status;
}

/**
 * Vp880GetDeviceOption()
 *  This function accesses the option being requested, fills the device object
 * with the data to be returned, and sets the Read Option complete event.
 *
 * Functions calling this function have to make sure Enter/Exit critical are
 * called around this function.
 *
 * Preconditions:
 *  None. All error checking required is assumed to exist in common interface
 * file.
 *
 * Postconditions:
 *  The device object is filled with the results of the option type being
 * requested and the Read Option Event flag is set.  This function returns the
 * success code if the option type being requested is supported.
 */
VpStatusType
Vp880GetDeviceOption(
    VpDevCtxType *pDevCtx,
    VpOptionIdType option,
    uint16 handle)
{
    VpStatusType status = VP_STATUS_SUCCESS;

    VpLineCtxType *pLineCtx;
    Vp880LineObjectType *pLineObj;

    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpGetResultsOptionsDataType *pOptionData;
    VpDeviceIdType deviceId;

    uint8 maxChan = pDevObj->staticInfo.maxChannels;
    uint8 channelId;

    uint8 ecVal = pDevObj->ecVal;
    uint8 ioDirection[2] = {0x00, 0x00};

    VP_API_FUNC(VpDevCtxType, pDevCtx, ("Vp880GetOption (Device)+"));

    /*
     * Upper layer checks to be sure that either device context or line
     * context pointers are not null -- so the device context is not null
     * in this case.
     */
    pDevObj = pDevCtx->pDevObj;
    deviceId = pDevObj->deviceId;
    pOptionData = &(pDevObj->getResultsOption.optionData);

    if (pDevObj->deviceEvents.response & VP880_READ_RESPONSE_MASK) {
        VP_API_FUNC(VpDevCtxType, pDevCtx, ("Vp880GetOption (Device) Error - VP_STATUS_DEVICE_BUSY"));
        return VP_STATUS_DEVICE_BUSY;
    }

    for (channelId = 0; channelId < maxChan; channelId++) {
        pLineCtx = pDevCtx->pLineCtx[channelId];
        if (pLineCtx != VP_NULL) {
            pLineObj = pLineCtx->pLineObj;
        }
    }

    switch (option) {
        case VP_DEVICE_OPTION_ID_PULSE:
            pOptionData->pulseTypeOption = pDevObj->pulseSpecs;
            break;

        case VP_DEVICE_OPTION_ID_PULSE2:
            pOptionData->pulseTypeOption = pDevObj->pulseSpecs2;
            break;

        case VP_DEVICE_OPTION_ID_CRITICAL_FLT:
            pOptionData->criticalFaultOption = pDevObj->criticalFault;
            break;

        case VP_DEVICE_OPTION_ID_DEVICE_IO: {
                uint8 pinCnt;
                uint16 bitMask;
                uint8 ecValMod[] = {VP880_EC_CH1, VP880_EC_CH2};

                uint8 regMask[VP880_MAX_PINS_PER_LINE] = {0x00,
                    VP880_IODIR_IO2_OUTPUT, /* 0x04 */
                    VP880_IODIR_IO3_OUTPUT, /* 0x08 */
                    VP880_IODIR_IO4_OUTPUT, /* 0x10 */
                    VP880_IODIR_IO5_OUTPUT, /* 0x20 */
                    VP880_IODIR_IO6_OUTPUT, /* 0x40 */
                };

                ecVal = pDevObj->ecVal;

                /* VE8830 Chip set does not have I/O pins */
                if (pDevObj->staticInfo.rcnPcn[1] == VP880_DEV_PCN_88536) {
                    VP_API_FUNC(VpDevCtxType, pDevCtx, ("Vp880GetOption (Device) Error - VP880_DEV_PCN_88536: VP_STATUS_OPTION_NOT_SUPPORTED"));
                    return VP_STATUS_OPTION_NOT_SUPPORTED;
                }


               /*
                 * Preclear so only 'OR' operation for output and open drain
                 * indications are set.
                 */
                pOptionData->deviceIo.outputTypePins_63_32 = 0;
                pOptionData->deviceIo.directionPins_63_32 = 0;
                pOptionData->deviceIo.outputTypePins_31_0 = 0;
                pOptionData->deviceIo.directionPins_31_0 = 0;

                /* Get the current device IO control information */
                for (channelId = 0; channelId < maxChan; channelId++) {
                    VpMpiCmdWrapper(deviceId, (ecVal | ecValMod[channelId]),
                        VP880_IODIR_REG_RD, VP880_IODIR_REG_LEN,
                        &ioDirection[channelId]);
                }

                for (channelId = 0; channelId < maxChan; channelId++) {
                    for (pinCnt = 0; pinCnt < VP880_MAX_PINS_PER_LINE; pinCnt++) {
                        bitMask = (1 << (channelId + 2 * pinCnt));

                        if (pinCnt == 0) {
                            /* I/O-1 has a "type" of output to be determined. */
                            if (ioDirection[channelId] & VP880_IODIR_IO1_OPEN_DRAIN) {
                                pOptionData->deviceIo.outputTypePins_31_0 |= bitMask;
                                pOptionData->deviceIo.directionPins_31_0 |= bitMask;
                            } else if (ioDirection[channelId] & VP880_IODIR_IO1_OUTPUT) {
                                pOptionData->deviceIo.directionPins_31_0 |= bitMask;
                            }
                        } else {
                            /*
                             * All other pins are driven output only. Just need
                             * to determine IF they are configured for output.
                             */
                            if (ioDirection[channelId] & regMask[pinCnt]) {
                                pOptionData->deviceIo.directionPins_31_0 |= bitMask;
                            }
                        }
                    }
                }
            }
            break;

        case VP_OPTION_ID_PULSE_MODE:
        case VP_OPTION_ID_TIMESLOT:
        case VP_OPTION_ID_CODEC:
        case VP_OPTION_ID_PCM_HWY:
        case VP_OPTION_ID_LOOPBACK:
        case VP_OPTION_ID_LINE_STATE:
        case VP_OPTION_ID_ZERO_CROSS:
        case VP_OPTION_ID_RING_CNTRL:
        case VP_OPTION_ID_PCM_TXRX_CNTRL:
        case VP_OPTION_ID_EVENT_MASK:
            status = VP_STATUS_INVALID_ARG;
            VP_API_FUNC(VpDevCtxType, pDevCtx, ("Vp880GetOption (Device) Error - VP_STATUS_INVALID_ARG"));
            break;

       default:
            status = VP_STATUS_OPTION_NOT_SUPPORTED;
            VP_API_FUNC(VpDevCtxType, pDevCtx, ("Vp880GetOption (Device) Error - VP_STATUS_OPTION_NOT_SUPPORTED"));
            break;
    }
    VP_API_FUNC(VpDevCtxType, pDevCtx, ("Vp880GetOption (Device)-"));
    return status;
}

/**
 * Vp880GetDeviceStatus()
 *  This function returns the status of all lines on a device for the type being
 * requested.
 *
 * Preconditions:
 *  None. All error checking required is assumed to exist in common interface
 * file.
 *
 * Postconditions:
 *  The location pointed to by the uint32 pointer passed is set (on a per line
 * basis) to either '1' if the status if TRUE on the given line, or '0' if the
 * status is FALSE on the given line for the status being requested.
 */
VpStatusType
Vp880GetDeviceStatus(
    VpDevCtxType *pDevCtx,
    VpInputType input,
    uint32 *pDeviceStatus)
{
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    uint8 maxChan = pDevObj->staticInfo.maxChannels;
    uint8 channelId;
    bool status = FALSE;
    VpLineCtxType *pLineCtx;

    VP_API_FUNC(VpDevCtxType, pDevCtx, ("Vp880GetDeviceStatus+"));

    *pDeviceStatus = 0;

    for (channelId = 0; channelId < maxChan; channelId++) {
        pLineCtx = pDevCtx->pLineCtx[channelId];

        if(pLineCtx != VP_NULL) {
            VpCSLACGetLineStatus(pLineCtx, input, &status);
        } else {
            status = FALSE;
        }
        *pDeviceStatus |= (((status == TRUE) ? 1 : 0) << channelId);
    }
    VP_API_FUNC(VpDevCtxType, pDevCtx, ("Vp880GetDeviceStatus-"));
    return VP_STATUS_SUCCESS;
}

/**
 * Vp880FlushEvents()
 *  This function clears out all events on the device and all events on all
 * lines associated with the device passed.
 *
 * Preconditions:
 *  None. All error checking required is assumed to exist in common interface
 * file.
 *
 * Postconditions:
 *  All active device events are cleared, and all active line events associated
 * with this device are cleared.
 */
VpStatusType
Vp880FlushEvents(
    VpDevCtxType *pDevCtx)
{
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpDeviceIdType deviceId = pDevObj->deviceId;

    VpLineCtxType *pLineCtx;
    Vp880LineObjectType *pLineObj;
    uint8 channelId;
    uint8 maxChan = pDevObj->staticInfo.maxChannels;

    VP_API_FUNC(VpDevCtxType, pDevCtx, ("Vp880FlushEvents+"));

    VpSysEnterCritical(deviceId, VP_CODE_CRITICAL_SEC);

    pDevObj->deviceEvents.faults = 0;
    pDevObj->deviceEvents.signaling = 0;
    pDevObj->deviceEvents.response = 0;
    pDevObj->deviceEvents.test = 0;
    pDevObj->deviceEvents.process = 0;
    pDevObj->deviceEvents.fxo = 0;

    for (channelId = 0; channelId < maxChan; channelId++) {
        pLineCtx = pDevCtx->pLineCtx[channelId];
        if(pLineCtx != VP_NULL) {
            pLineObj = pLineCtx->pLineObj;

            pLineObj->lineEvents.faults = 0;
            pLineObj->lineEvents.signaling = 0;
            pLineObj->lineEvents.response = 0;
            pLineObj->lineEvents.test = 0;
            pLineObj->lineEvents.process = 0;
            pLineObj->lineEvents.fxo = 0;
        }
    }

    VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);

    VP_API_FUNC(VpDevCtxType, pDevCtx, ("Vp880FlushEvents"));
    return VP_STATUS_SUCCESS;
}

/**
 * Vp880GetResults()
 *  This function fills the results structure passed with the results data found
 * from the event that caused new results.
 *
 * Preconditions:
 *  None. All error checking required is assumed to exist in common interface
 * file.
 *
 * Postconditions:
 *  If the event structure passed provides the event catagory and ID for a valid
 * results type to read, then the structure passed is filled with the results
 * data.  This function returns the success code if the event catagory and ID is
 * supported by the device.
 */
VpStatusType
Vp880GetResults(
    VpEventType *pEvent,
    void *pResults)
{
    VpDevCtxType *pDevCtx = pEvent->pDevCtx;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpDeviceIdType deviceId = pDevObj->deviceId;
    VpStatusType status = VP_STATUS_SUCCESS;
    uint8 commandByte;
    uint8 *pMpiData;
    uint8 mpiDataLen = pDevObj->mpiLen;

    VpGetResultsOptionsDataType *pOptionData =
        &(pDevObj->getResultsOption.optionData);

    VP_API_FUNC(VpDevCtxType, pDevCtx, ("Vp880GetResults+"));

    VpSysEnterCritical(deviceId, VP_CODE_CRITICAL_SEC);

    switch(pEvent->eventCategory) {
        case VP_EVCAT_RESPONSE:
            switch (pEvent->eventId) {
                case VP_LINE_EVID_LLCMD_RX_CMP:
                    pMpiData = (uint8 *)pResults;
                    for (commandByte = 0;
                         commandByte < mpiDataLen;
                         commandByte++) {
                        pMpiData[commandByte] = pDevObj->mpiData[commandByte];
                    }
                    break;

                case VP_LINE_EVID_GAIN_CMP:
                    *(VpRelGainResultsType *)pResults =
                        pDevObj->relGainResults;
                    break;

                case VP_DEV_EVID_IO_ACCESS_CMP:
                    *((VpDeviceIoAccessDataType *)pResults) =
                        pOptionData->deviceIoData;
                    break;

                case VP_LINE_EVID_RD_OPTION:
                    switch(pDevObj->getResultsOption.optionType) {
                        case VP_DEVICE_OPTION_ID_PULSE:
                            *(VpOptionPulseType *)pResults =
                                pDevObj->pulseSpecs;
                            break;

                        case VP_DEVICE_OPTION_ID_PULSE2:
                            *(VpOptionPulseType *)pResults =
                                pDevObj->pulseSpecs2;
                            break;

                        case VP_DEVICE_OPTION_ID_CRITICAL_FLT:
                            *(VpOptionCriticalFltType *)pResults =
                                pOptionData->criticalFaultOption;
                            break;

                        case VP_DEVICE_OPTION_ID_DEVICE_IO:
                            *(VpOptionDeviceIoType *)pResults =
                                pOptionData->deviceIo;
                            break;

                        case VP_OPTION_ID_RING_CNTRL:
                            *(VpOptionRingControlType *)pResults =
                                pOptionData->ringControlOption;
                            break;

                        case VP_OPTION_ID_ZERO_CROSS:
                            *(VpOptionZeroCrossType *)pResults =
                                pOptionData->zeroCross;
                            break;

                        case VP_OPTION_ID_PULSE_MODE:
                            *((VpOptionPulseModeType *)pResults) =
                                pOptionData->pulseModeOption;
                            break;

                        case VP_OPTION_ID_TIMESLOT:
                            *(VpOptionTimeslotType *)pResults =
                                pOptionData->timeSlotOption;
                            break;

                        case VP_OPTION_ID_CODEC:
                            *((VpOptionCodecType *)pResults) =
                                pOptionData->codecOption;
                            break;

                        case VP_OPTION_ID_PCM_HWY:
                            *((VpOptionCodecType *)pResults) =
                                pOptionData->pcmHwyOption;
                            break;

                        case VP_OPTION_ID_LOOPBACK:
                            *((VpOptionLoopbackType *)pResults) =
                                pOptionData->loopBackOption;
                            break;

                        case VP_OPTION_ID_LINE_STATE:
                            *((VpOptionLineStateType *)pResults) =
                                pOptionData->lineStateOption;
                            break;

                        case VP_OPTION_ID_EVENT_MASK:
                            *((VpOptionEventMaskType *)pResults) =
                                pOptionData->eventMaskOption;
                            break;

                        case VP_OPTION_ID_PCM_TXRX_CNTRL:
                            *((VpOptionPcmTxRxCntrlType *)pResults) =
                                pOptionData->pcmTxRxCtrl;
                            break;

                        default:
                            status = VP_STATUS_INVALID_ARG;
                            break;
                    }
                    break;

                case VP_EVID_CAL_CMP:
                    if (pResults == VP_NULL) {
                        status = VP_STATUS_INVALID_ARG;
                    } else {
                        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("Vp880GetResults - VP_EVID_CAL_CMP"));

                        pMpiData = (uint8 *)pResults;
                        pMpiData[VP_PROFILE_TYPE_MSB] = VP_DEV_880_SERIES;
                        pMpiData[VP_PROFILE_TYPE_LSB] = VP_PRFWZ_PROFILE_CAL;
                        pMpiData[VP_PROFILE_INDEX] = 0;
                        pMpiData[VP_PROFILE_LENGTH] = VP880_CAL_STRUCT_SIZE + 2;
                        pMpiData[VP_PROFILE_VERSION] = 0;
                        pMpiData[VP_PROFILE_MPI_LEN] = 0;
                        commandByte = VP_PROFILE_DATA_START;

                        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("ABV Y Error %d Size %d",
                            pDevObj->vp880SysCalData.abvError[0], sizeof(pDevObj->vp880SysCalData.abvError[0])));
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.abvError[0] >> 8) & 0xFF);
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.abvError[0]) & 0xFF);

                        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("ABV Z Error %d Size %d",
                            pDevObj->vp880SysCalData.abvError[1], sizeof(pDevObj->vp880SysCalData.abvError[1])));
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.abvError[1] >> 8) & 0xFF);
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.abvError[1]) & 0xFF);

                        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("VOC Offset Norm Ch 0 %d Size %d",
                            pDevObj->vp880SysCalData.vocOffset[0][0], sizeof(pDevObj->vp880SysCalData.vocOffset[0][0])));
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.vocOffset[0][0] >> 8) & 0xFF);
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.vocOffset[0][0]) & 0xFF);

                        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("VOC Error Norm Ch 0 %d Size %d",
                            pDevObj->vp880SysCalData.vocError[0][0], sizeof(pDevObj->vp880SysCalData.vocError[0][0])));
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.vocError[0][0] >> 8) & 0xFF);
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.vocError[0][0]) & 0xFF);

                        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("VOC Offset Rev Ch 0 %d Size %d",
                            pDevObj->vp880SysCalData.vocOffset[0][1], sizeof(pDevObj->vp880SysCalData.vocOffset[0][1])));
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.vocOffset[0][1] >> 8) & 0xFF);
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.vocOffset[0][1]) & 0xFF);

                        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("VOC Error Rev Ch 0 %d Size %d",
                            pDevObj->vp880SysCalData.vocError[0][1], sizeof(pDevObj->vp880SysCalData.vocError[0][1])));
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.vocError[0][1] >> 8) & 0xFF);
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.vocError[0][1]) & 0xFF);

                        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("VOC Offset Norm Ch 1 %d Size %d",
                            pDevObj->vp880SysCalData.vocOffset[1][0], sizeof(pDevObj->vp880SysCalData.vocOffset[1][0])));
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.vocOffset[1][0] >> 8) & 0xFF);
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.vocOffset[1][0]) & 0xFF);

                        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("VOC Error Norm Ch 1 %d Size %d",
                            pDevObj->vp880SysCalData.vocError[1][0], sizeof(pDevObj->vp880SysCalData.vocError[1][0])));
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.vocError[1][0] >> 8) & 0xFF);
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.vocError[1][0]) & 0xFF);

                        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("VOC Offset Rev Ch 1 %d Size %d",
                            pDevObj->vp880SysCalData.vocOffset[1][1], sizeof(pDevObj->vp880SysCalData.vocOffset[1][1])));
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.vocOffset[1][1] >> 8) & 0xFF);
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.vocOffset[1][1]) & 0xFF);

                        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("VOC Error Rev Ch 1 %d Size %d",
                            pDevObj->vp880SysCalData.vocError[1][1], sizeof(pDevObj->vp880SysCalData.vocError[1][1])));
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.vocError[1][1] >> 8) & 0xFF);
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.vocError[1][1]) & 0xFF);

                        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("SigGenA Norm Error Ch 0 %d Size %d",
                            pDevObj->vp880SysCalData.sigGenAError[0][0], sizeof(pDevObj->vp880SysCalData.sigGenAError[0][0])));
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.sigGenAError[0][0] >> 8) & 0xFF);
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.sigGenAError[0][0]) & 0xFF);

                        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("SigGenA Rev Error Ch 0 %d Size %d",
                            pDevObj->vp880SysCalData.sigGenAError[0][1], sizeof(pDevObj->vp880SysCalData.sigGenAError[0][1])));
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.sigGenAError[0][1] >> 8) & 0xFF);
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.sigGenAError[0][1]) & 0xFF);

                        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("SigGenA Norm Error Ch 1 %d Size %d",
                            pDevObj->vp880SysCalData.sigGenAError[1][0], sizeof(pDevObj->vp880SysCalData.sigGenAError[1][0])));
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.sigGenAError[1][0] >> 8) & 0xFF);
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.sigGenAError[1][0]) & 0xFF);

                        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("SigGenA Rev Error Ch 1 %d Size %d",
                            pDevObj->vp880SysCalData.sigGenAError[1][1], sizeof(pDevObj->vp880SysCalData.sigGenAError[1][1])));
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.sigGenAError[1][1] >> 8) & 0xFF);
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.sigGenAError[1][1]) & 0xFF);

                        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("ILA-20mA Ch 0 %d Size %d",
                            pDevObj->vp880SysCalData.ila20[0], sizeof(pDevObj->vp880SysCalData.ila20[0])));
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.ila20[0] >> 8) & 0xFF);
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.ila20[0]) & 0xFF);

                        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("ILA-20mA Ch 1 %d Size %d",
                            pDevObj->vp880SysCalData.ila20[1], sizeof(pDevObj->vp880SysCalData.ila20[1])));
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.ila20[1] >> 8) & 0xFF);
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.ila20[1]) & 0xFF);

                        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("ILA-25mA Ch 0 %d Size %d",
                            pDevObj->vp880SysCalData.ila25[0], sizeof(pDevObj->vp880SysCalData.ila25[0])));
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.ila25[0] >> 8) & 0xFF);
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.ila25[0]) & 0xFF);

                        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("ILA-25mA Ch 1 %d Size %d",
                            pDevObj->vp880SysCalData.ila25[1], sizeof(pDevObj->vp880SysCalData.ila25[1])));
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.ila25[1] >> 8) & 0xFF);
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.ila25[1]) & 0xFF);

                        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("ILA-32mA Ch 0 %d Size %d",
                            pDevObj->vp880SysCalData.ila32[0], sizeof(pDevObj->vp880SysCalData.ila32[0])));
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.ila32[0] >> 8) & 0xFF);
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.ila32[0]) & 0xFF);

                        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("ILA-32mA Ch 1 %d Size %d",
                            pDevObj->vp880SysCalData.ila32[1], sizeof(pDevObj->vp880SysCalData.ila32[1])));
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.ila32[1] >> 8) & 0xFF);
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.ila32[1]) & 0xFF);

                        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("ILA-40mA Ch 0 %d Size %d",
                            pDevObj->vp880SysCalData.ila40[0], sizeof(pDevObj->vp880SysCalData.ila40[0])));
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.ila40[0] >> 8) & 0xFF);
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.ila40[0]) & 0xFF);

                        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("ILA-40mA Ch 1 %d Size %d",
                            pDevObj->vp880SysCalData.ila40[1], sizeof(pDevObj->vp880SysCalData.ila40[1])));
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.ila40[1] >> 8) & 0xFF);
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.ila40[1]) & 0xFF);

                        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("ILA Offset Norm Ch 0 %d Size %d",
                            pDevObj->vp880SysCalData.ilaOffsetNorm[0], sizeof(pDevObj->vp880SysCalData.ilaOffsetNorm[0])));
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.ilaOffsetNorm[0] >> 8) & 0xFF);
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.ilaOffsetNorm[0]) & 0xFF);

                        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("ILA Offset Norm Ch 1 %d Size %d",
                            pDevObj->vp880SysCalData.ilaOffsetNorm[1], sizeof(pDevObj->vp880SysCalData.ilaOffsetNorm[1])));
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.ilaOffsetNorm[1] >> 8) & 0xFF);
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.ilaOffsetNorm[1]) & 0xFF);

                        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("ILG Offset Norm Ch 0 %d Size %d",
                            pDevObj->vp880SysCalData.ilgOffsetNorm[0], sizeof(pDevObj->vp880SysCalData.ilgOffsetNorm[0])));
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.ilgOffsetNorm[0] >> 8) & 0xFF);
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.ilgOffsetNorm[0]) & 0xFF);

                        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("ILG Offset Norm Ch 1 %d Size %d",
                            pDevObj->vp880SysCalData.ilgOffsetNorm[1], sizeof(pDevObj->vp880SysCalData.ilgOffsetNorm[1])));
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.ilgOffsetNorm[1] >> 8) & 0xFF);
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.ilgOffsetNorm[1]) & 0xFF);

                        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("VAS Norm Ch 0 %d Size %d",
                            pDevObj->vp880SysCalData.vas[0][0], sizeof(pDevObj->vp880SysCalData.vas[0][0])));
                        pMpiData[commandByte++] = pDevObj->vp880SysCalData.vas[0][0];

                        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("VAS Rev Ch 0 %d Size %d",
                            pDevObj->vp880SysCalData.vas[0][1], sizeof(pDevObj->vp880SysCalData.vas[0][1])));
                        pMpiData[commandByte++] = pDevObj->vp880SysCalData.vas[0][1];

                        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("VAS Norm Ch 1 %d Size %d",
                            pDevObj->vp880SysCalData.vas[1][0], sizeof(pDevObj->vp880SysCalData.vas[1][0])));
                        pMpiData[commandByte++] = pDevObj->vp880SysCalData.vas[1][0];

                        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("VAS Rev Ch 1 %d Size %d",
                            pDevObj->vp880SysCalData.vas[1][1], sizeof(pDevObj->vp880SysCalData.vas[1][1])));
                        pMpiData[commandByte++] = pDevObj->vp880SysCalData.vas[1][1];

                        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("VAG Offset Norm Ch 0 %d Size %d",
                            pDevObj->vp880SysCalData.vagOffsetNorm[0], sizeof(pDevObj->vp880SysCalData.vagOffsetNorm[0])));
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.vagOffsetNorm[0] >> 8) & 0xFF);
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.vagOffsetNorm[0]) & 0xFF);

                        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("VAG Offset Norm Ch 1 %d Size %d",
                            pDevObj->vp880SysCalData.vagOffsetNorm[1], sizeof(pDevObj->vp880SysCalData.vagOffsetNorm[1])));
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.vagOffsetNorm[1] >> 8) & 0xFF);
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.vagOffsetNorm[1]) & 0xFF);

                        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("VAG Offset Rev Ch 0 %d Size %d",
                            pDevObj->vp880SysCalData.vagOffsetRev[0], sizeof(pDevObj->vp880SysCalData.vagOffsetRev[0])));
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.vagOffsetRev[0] >> 8) & 0xFF);
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.vagOffsetRev[0]) & 0xFF);

                        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("VAG Offset Rev Ch 1 %d Size %d",
                            pDevObj->vp880SysCalData.vagOffsetRev[1], sizeof(pDevObj->vp880SysCalData.vagOffsetRev[1])));
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.vagOffsetRev[1] >> 8) & 0xFF);
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.vagOffsetRev[1]) & 0xFF);

                        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("VBG Offset Norm Ch 0 %d Size %d",
                            pDevObj->vp880SysCalData.vbgOffsetNorm[0], sizeof(pDevObj->vp880SysCalData.vbgOffsetNorm[0])));
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.vbgOffsetNorm[0] >> 8) & 0xFF);
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.vbgOffsetNorm[0]) & 0xFF);

                        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("VBG Offset Norm Ch 1 %d Size %d",
                            pDevObj->vp880SysCalData.vbgOffsetNorm[1], sizeof(pDevObj->vp880SysCalData.vbgOffsetNorm[1])));
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.vbgOffsetNorm[1] >> 8) & 0xFF);
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.vbgOffsetNorm[1]) & 0xFF);

                        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("VBG Offset Rev Ch 0 %d Size %d",
                            pDevObj->vp880SysCalData.vbgOffsetRev[0], sizeof(pDevObj->vp880SysCalData.vbgOffsetRev[0])));
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.vbgOffsetRev[0] >> 8) & 0xFF);
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.vbgOffsetRev[0]) & 0xFF);

                        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("VBG Offset Rev Ch 1 %d Size %d",
                            pDevObj->vp880SysCalData.vbgOffsetRev[1], sizeof(pDevObj->vp880SysCalData.vbgOffsetRev[1])));
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.vbgOffsetRev[1] >> 8) & 0xFF);
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.vbgOffsetRev[1]) & 0xFF);

                        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("Auto-Bat Switch Normal Ch 0 %d Size %d",
                            pDevObj->vp880SysCalData.absNormCal[0], sizeof(pDevObj->vp880SysCalData.absNormCal[0])));
                        pMpiData[commandByte++] = pDevObj->vp880SysCalData.absNormCal[0];

                        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("Auto-Bat Switch Rev Ch 0 %d Size %d",
                            pDevObj->vp880SysCalData.absPolRevCal[0], sizeof(pDevObj->vp880SysCalData.absPolRevCal[0])));
                        pMpiData[commandByte++] = pDevObj->vp880SysCalData.absPolRevCal[0];

                        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("Auto-Bat Switch Normal Ch 1 %d Size %d",
                            pDevObj->vp880SysCalData.absNormCal[1], sizeof(pDevObj->vp880SysCalData.absNormCal[1])));
                        pMpiData[commandByte++] = pDevObj->vp880SysCalData.absNormCal[1];

                        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("Auto-Bat Switch Rev Ch 1 %d Size %d",
                            pDevObj->vp880SysCalData.absPolRevCal[1], sizeof(pDevObj->vp880SysCalData.absPolRevCal[1])));
                        pMpiData[commandByte++] = pDevObj->vp880SysCalData.absPolRevCal[1];

                        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("SWY Offset Ch 0 %d Size %d",
                            pDevObj->vp880SysCalData.swyOffset[0], sizeof(pDevObj->vp880SysCalData.swyOffset[0])));
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.swyOffset[0] >> 8) & 0xFF);
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.swyOffset[0]) & 0xFF);

                        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("SWY Offset Ch 1 %d Size %d",
                            pDevObj->vp880SysCalData.swyOffset[1], sizeof(pDevObj->vp880SysCalData.swyOffset[1])));
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.swyOffset[1] >> 8) & 0xFF);
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.swyOffset[1]) & 0xFF);

                        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("SWZ Offset Ch 0 %d Size %d",
                            pDevObj->vp880SysCalData.swzOffset[0], sizeof(pDevObj->vp880SysCalData.swzOffset[0])));
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.swzOffset[0] >> 8) & 0xFF);
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.swzOffset[0]) & 0xFF);

                        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("SWZ Offset Ch 1 %d Size %d",
                            pDevObj->vp880SysCalData.swzOffset[1], sizeof(pDevObj->vp880SysCalData.swzOffset[1])));
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.swzOffset[1] >> 8) & 0xFF);
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.swzOffset[1]) & 0xFF);

                        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("XB Offset Ch 0 %d Size %d",
                            pDevObj->vp880SysCalData.swxbOffset[0], sizeof(pDevObj->vp880SysCalData.swxbOffset[0])));
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.swxbOffset[0] >> 8) & 0xFF);
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.swxbOffset[0]) & 0xFF);

                        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("XB Offset Ch 1 %d Size %d",
                            pDevObj->vp880SysCalData.swxbOffset[1], sizeof(pDevObj->vp880SysCalData.swxbOffset[1])));
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.swxbOffset[1] >> 8) & 0xFF);
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.swxbOffset[1]) & 0xFF);

                        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("Tip Cap Ch 0 %li Size %d",
                            pDevObj->vp880SysCalData.tipCapCal[0], sizeof(pDevObj->vp880SysCalData.tipCapCal[0])));
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.tipCapCal[0] >> 24) & 0xFF);
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.tipCapCal[0] >> 16) & 0xFF);
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.tipCapCal[0] >> 8) & 0xFF);
                        pMpiData[commandByte++] = (uint8)(pDevObj->vp880SysCalData.tipCapCal[0] & 0xFF);

                        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("Tip Cap Ch 1 %li Size %d",
                            pDevObj->vp880SysCalData.tipCapCal[1], sizeof(pDevObj->vp880SysCalData.tipCapCal[1])));
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.tipCapCal[1] >> 24) & 0xFF);
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.tipCapCal[1] >> 16) & 0xFF);
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.tipCapCal[1] >> 8) & 0xFF);
                        pMpiData[commandByte++] = (uint8)(pDevObj->vp880SysCalData.tipCapCal[1] & 0xFF);

                        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("Ring Cap Ch 0 %li Size %d",
                            pDevObj->vp880SysCalData.ringCapCal[0], sizeof(pDevObj->vp880SysCalData.ringCapCal[0])));
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.ringCapCal[0] >> 24) & 0xFF);
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.ringCapCal[0] >> 16) & 0xFF);
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.ringCapCal[0] >> 8) & 0xFF);
                        pMpiData[commandByte++] = (uint8)(pDevObj->vp880SysCalData.ringCapCal[0] & 0xFF);

                        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("Ring Cap Ch 1 %li Size %d",
                            pDevObj->vp880SysCalData.ringCapCal[1], sizeof(pDevObj->vp880SysCalData.ringCapCal[1])));
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.ringCapCal[1] >> 24) & 0xFF);
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.ringCapCal[1] >> 16) & 0xFF);
                        pMpiData[commandByte++] = (uint8)((pDevObj->vp880SysCalData.ringCapCal[1] >> 8) & 0xFF);
                        pMpiData[commandByte++] = (uint8)(pDevObj->vp880SysCalData.ringCapCal[1] & 0xFF);

                        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("Final Command Byte Value %d", commandByte));
                    }
                    break;

                default:
                    status = VP_STATUS_INVALID_ARG;
                    break;
            }
            break;

#ifdef VP880_INCLUDE_TESTLINE_CODE
        case VP_EVCAT_TEST:
            switch (pEvent->eventId) {
                case VP_LINE_EVID_TEST_CMP:
                    *((VpTestResultType *)pResults) = pDevObj->testResults;
                    break;

                 default:
                    break;
            }
#endif
            break;
        default:
            status = VP_STATUS_INVALID_ARG;
            break;
    }

    VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);

    VP_API_FUNC(VpDevCtxType, pDevCtx, ("Vp880GetResults-"));
    return status;
}

/**
 * Vp880ServiceTimers()
 *  This function services active API timers on all channels of deviceId.
 *
 * Preconditions:
 *  This Function must be called from the ApiTick function once per device.
 *
 * Postconditions:
 *  All Active Timers have been serviced.
 */
bool
Vp880ServiceTimers(
    VpDevCtxType *pDevCtx)
{
    VpLineCtxType *pLineCtx;
    Vp880LineObjectType *pLineObj;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    uint16 tempTimer;
    VpDevTimerType devTimerType;

    uint8 channelId;
    uint8 maxChan = pDevObj->staticInfo.maxChannels;
    bool retFlag = FALSE;

    uint8 ecVal;

    for (devTimerType = 0; devTimerType < VP_DEV_TIMER_LAST; devTimerType++) {
        if (pDevObj->devTimer[devTimerType] & VP_ACTIVATE_TIMER) {

            /* get the bits associated with the timer into a temp variable */
            tempTimer = (pDevObj->devTimer[devTimerType] & VP_TIMER_TIME_MASK);

            /* decrement the timer */
            if (tempTimer > 0) {
                tempTimer--;
            }

            /* reset the device timer to the new value*/
            pDevObj->devTimer[devTimerType] = tempTimer;

            /* if the timer has expired then run the timer code */
            if (pDevObj->devTimer[devTimerType] == 0) {

                VP_LINE_STATE(None, NULL, ("Device timer %d Expired at %d",
                    devTimerType, pDevObj->timeStamp));

                switch(devTimerType) {
                    case VP_DEV_TIMER_EXIT_RINGING: {
                            bool disableHpm = TRUE;
                            uint8 lineState[VP880_SYS_STATE_LEN];
                            VpDeviceIdType deviceId = pDevObj->deviceId;

                            for (channelId = 0; channelId < maxChan; channelId++) {
                                pLineCtx = pDevCtx->pLineCtx[channelId];
                                if (pLineCtx != VP_NULL) {
                                    pLineObj = pLineCtx->pLineObj;
                                    if (!(pLineObj->status & VP880_IS_FXO)) {
                                        ecVal = pLineObj->ecVal;
                                        VpMpiCmdWrapper(deviceId, ecVal, VP880_SYS_STATE_RD,
                                            VP880_SYS_STATE_LEN, lineState);

                                        switch (lineState[0] & VP880_SS_LINE_FEED_MASK) {
                                            case  VP880_SS_FEED_BALANCED_RINGING:
                                            case  VP880_SS_FEED_UNBALANCED_RINGING:
                                                disableHpm = FALSE;
                                                break;

                                            default:
                                                if (lineState[0] & VP880_SS_RING_EXIT_MASK) {
                                                    disableHpm = FALSE;
                                                }
                                                break;
                                        }
                                    }
                                }
                            }
                            ecVal = pDevObj->ecVal;
                            if (disableHpm == TRUE) {
                                uint8 regCtrl = VP880_SWY_MP | VP880_SWZ_MP;
                                VpMpiCmdWrapper(deviceId, ecVal,
                                    VP880_REGULATOR_CTRL_WRT,
                                    VP880_REGULATOR_CTRL_LEN, &regCtrl);
                            } else {
                                pDevObj->devTimer[VP_DEV_TIMER_EXIT_RINGING] =
                                    (MS_TO_TICKRATE(VP_DEV_TIMER_EXIT_RINGING_SAMPLE,
                                    pDevObj->devProfileData.tickRate)) | VP_ACTIVATE_TIMER;
                            }
                        }
                        break;

                    case VP_DEV_TIMER_CLKFAIL:
                        retFlag = TRUE;
                        break;

                    case VP_DEV_TIMER_LP_CHANGE:
                        Vp880ServiceLpChangeTimer(pDevCtx);
                        break;

#ifdef VP880_INCLUDE_TESTLINE_CODE
                    case VP_DEV_TIMER_TESTLINE:
                        {
                            const void *pTestArgs =
                                (const void*)&pDevObj->currentTest.pTestHeap->testArgs;
                            uint8 testChanId = pDevObj->currentTest.channelId;
                            VpTestLineIntFuncPtrType testline =
                                pDevCtx->funPtrsToApiFuncs.TestLineInt;
                            if ((testline != VP_NULL)
                              && (pDevObj->currentTest.testState != -1)) {
                                /*
                                 * if the TestLineInt function exists and the
                                 * current test state has not been set back to
                                 * -1 by test conclude before the timer expired
                                 * then run the call back
                                 */

                                testline(
                                    pDevCtx->pLineCtx[testChanId],
                                    pDevObj->currentTest.testId,
                                    pTestArgs,
                                    pDevObj->currentTest.handle,
                                    TRUE);
                            }
                        }
                        break;

#endif /* VP880_INCLUDE_TESTLINE_CODE */

                    case VP_DEV_TIMER_ABV_CAL:
                        if (pDevObj->stateInt & VP880_IS_ABS) {
                            Vp880CalAbvAbsDev(pDevCtx);
                        } else {
                            Vp880CalAbv(pDevCtx);
                        }
                        break;

                    case VP_DEV_TIMER_ABSCAL:
                        Vp880AbsCalibration(pDevCtx);
                        break;

                    case VP_DEV_TIMER_ENTER_RINGING:
                        Vp880LimitInRushCurrent(pDevObj, pDevObj->ecVal, TRUE);
                        break;

                    default:
                        break;
                } /* Switch (timerType) */

                /*
                 * This is a check to make sure that one of the call back
                 * functions has not reset the value of the devTimer. If
                 * the call back function has not then just clear the timer bits
                 * if it has then we need to enable the activate mask.
                 */
                if (pDevObj->devTimer[devTimerType] == 0) {
                    pDevObj->devTimer[devTimerType] &= ~(VP_ACTIVATE_TIMER);
                } else {
                    pDevObj->devTimer[devTimerType] |= VP_ACTIVATE_TIMER;
                }
            } else { /* If timer has not expired */
                pDevObj->devTimer[devTimerType] |= VP_ACTIVATE_TIMER;
            }
        } /* if timerType is active     */
    } /* Loop through all device timers */

    /* Iterate through the channels until all timers are serviced */
    for(channelId=0; channelId < maxChan; channelId++ ) {
        pLineCtx = pDevCtx->pLineCtx[channelId];
        if (pLineCtx != VP_NULL) {
            pLineObj = pLineCtx->pLineObj;

            if (!(pLineObj->status & VP880_IS_FXO)) {
                Vp880ServiceFxsTimers(pLineCtx);
            } else {
                Vp880ServiceFxoTimers(pLineCtx);
            }
        } /* Line Context Check */
    } /* Loop through channels until no more tests */

    return retFlag;
} /* Vp880ServiceTimers() */

/**
 * Vp880ServiceLpChangeTimer()
 *  This function services the active Low Power Change Device timer.
 *
 * Preconditions:
 *  This Function must be called from the ApiTick function once per device.
 *
 * Postconditions:
 *  The LP Change timer has been serviced.
 */
void
Vp880ServiceLpChangeTimer(
    VpDevCtxType *pDevCtx)
{
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    uint8 channelId;
    uint8 maxChan = pDevObj->staticInfo.maxChannels;
    VpDeviceIdType deviceId = pDevObj->deviceId;
    Vp880LineObjectType *pLineObj;
    VpLineCtxType *pLineCtx;

    uint8 ecVal;

    for (channelId = 0; channelId < maxChan; channelId++) {
        pLineCtx = pDevCtx->pLineCtx[channelId];
        if (pLineCtx != VP_NULL) {
            bool failureMode = FALSE;
            pLineObj = pLineCtx->pLineObj;
            ecVal = pLineObj->ecVal;

            if ((pLineObj->lineState.currentState != VP_LINE_DISCONNECT) &&
                (pLineObj->lineState.currentState != VP_LINE_TIP_OPEN) &&
                (pLineObj->lineState.currentState != VP_LINE_RINGING) &&
                (!(pLineObj->status & VP880_LINE_IN_CAL))) {

                VP_HOOK(VpLineCtxType, pLineCtx, ("Signaling 0x%02X 0x%02X",
                    pDevObj->intReg[0], pDevObj->intReg[1]));

                VP_HOOK(VpLineCtxType, pLineCtx, ("Last Hook State on line %d = %d LP Mode %d UserState %d",
                    channelId, (pLineObj->lineState.condition & VP_CSLAC_HOOK),
                    (pLineObj->status & VP880_LOW_POWER_EN),
                    pLineObj->lineState.usrCurrent));

               if (pLineObj->status & VP880_LOW_POWER_EN) {
                    /*
                     * If we're in LP Mode, then the line should be detecting
                     * on-hook. All other conditions mean there could be a leaky
                     * line.
                     */
                    if ((pLineObj->lineState.condition & VP_CSLAC_HOOK)
                      && (!(pDevObj->intReg[channelId]) & VP880_HOOK1_MASK)) {
                        failureMode = TRUE;
                    }
                } else {
                    /*
                     * If we're not in LP Mode, then the line should be
                     * detecting off-hook and the signaling bit should be high.
                     * Otherwise, error.
                     */

                    if ((pLineObj->lineState.condition & VP_CSLAC_HOOK)
                      && (!(pDevObj->intReg[channelId]) & VP880_HOOK1_MASK)) {
                        failureMode = TRUE;
                     }
                }
            }

            /*
             * If the line was last seen off-hook and is now on-hook as a result
             * of exiting LP Mode, it could be a leaky line.
             */
            if (failureMode == TRUE) {
                /* Nigel wants this test to run 3 times */
                if (pLineObj->leakyLineCnt >= 2) {
                    VP_HOOK(VpLineCtxType, pLineCtx, ("Flag Channel %d for Leaky Line at time %d Signaling 0x%02X LineState %d",
                        channelId, pDevObj->timeStamp, (pDevObj->intReg[channelId] & VP880_HOOK1_MASK),
                        pLineObj->lineState.usrCurrent));

                    pLineObj->status |= VP880_LINE_LEAK;
                    pDevObj->stateInt &=
                        ((channelId == 0) ? ~VP880_LINE0_LP : ~VP880_LINE1_LP);
                    pLineObj->lineEvents.faults |= VP_LINE_EVID_RES_LEAK_FLT;

                    /* Leak test is complete */
                    pLineObj->lineState.condition &= ~VP_CSLAC_LINE_LEAK_TEST;
                } else {
                    VP_HOOK(VpLineCtxType, pLineCtx, ("Potential Leaky Line %d at time %d  ...retry",
                        channelId, pDevObj->timeStamp));

                    pLineObj->leakyLineCnt++;

                    /* Leak test is still in progress */
                    /*
                     * Make sure timer is restarted. This may occur as a result
                     * of SetLineState(), but it may not.
                     */
                    pDevObj->devTimer[VP_DEV_TIMER_LP_CHANGE] =
                        MS_TO_TICKRATE(VP880_PWR_SWITCH_DEBOUNCE,
                            pDevObj->devProfileData.tickRate);
                    pDevObj->devTimer[VP_DEV_TIMER_LP_CHANGE] |=
                        VP_ACTIVATE_TIMER;
                }

                /* Update the line state */
                for (channelId = 0;
                     channelId < pDevObj->staticInfo.maxChannels;
                     channelId++) {
                    Vp880LineObjectType *pLineObjInt;

                    pLineCtx = pDevCtx->pLineCtx[channelId];
                    if (pLineCtx != VP_NULL) {
                        pLineObjInt = pLineCtx->pLineObj;
                        VP_HOOK(VpLineCtxType, pLineCtx, ("1. Channel %d Current Linestate %d Current User Linestate %d",
                            channelId, pLineObjInt->lineState.currentState, pLineObjInt->lineState.usrCurrent));

                        Vp880SetLineStateInt(pLineCtx, pLineObjInt->lineState.usrCurrent);
                    }
                }
            } else {
                /*
                 * No failure. Recover all hook status, line states, and clear
                 * Leaky Line Test Flag
                 */

                /* Leak test is complete */
                pLineObj->lineState.condition &= ~VP_CSLAC_LINE_LEAK_TEST;

                /*
                 * Low Power Mode exit simply sets the line to Active in order
                 * to maintain smooth line transients. This step is done to
                 * put the line into the user (API-II) defined state.
                 */
                VP_LINE_STATE(VpLineCtxType, pLineCtx, ("LPM Timer: Current %d, User Current %d Channel %d",
                    pLineObj->lineState.currentState,
                    pLineObj->lineState.usrCurrent,
                    channelId));

                if ((pLineObj->lineState.usrCurrent == VP_LINE_STANDBY)
                  && (!(pLineObj->status & VP880_LOW_POWER_EN))
                  && (pLineObj->calLineData.calDone == TRUE)) {     /* Must not occur during the calibration */
                    uint8 lineState[1] = {VP880_SS_IDLE};

                    VP_LINE_STATE(VpLineCtxType, pLineCtx, ("Setting Channel %d to 0x%02X State at time %d",
                        channelId, lineState[0], pDevObj->timeStamp));
#ifdef CSLAC_SEQ_EN
                    if ((pLineObj->cadence.status & (VP_CADENCE_STATUS_ACTIVE | VP_CADENCE_STATUS_SENDSIG)) !=
                        (VP_CADENCE_STATUS_ACTIVE | VP_CADENCE_STATUS_SENDSIG)) {
#endif
                        Vp880UpdateBufferChanSel(pDevObj, channelId, lineState[0]);
                        VpMpiCmdWrapper(deviceId, ecVal, VP880_SYS_STATE_WRT,
                            VP880_SYS_STATE_LEN, lineState);
#ifdef CSLAC_SEQ_EN
                    }
#endif
                }

                if ((pLineObj->lineState.condition & VP_CSLAC_HOOK)
                    && (pDevObj->intReg[channelId]) & VP880_HOOK1_MASK) {
                    if ((pLineObj->lineState.condition & VP_CSLAC_HOOK) &&
                        (pLineObj->status & VP880_LOW_POWER_EN)) {
                        /* The line is on-hook */
                        pLineObj->lineState.condition &= ~VP_CSLAC_HOOK;
                    } else {
                        /* Valid off-hook */
                        VP_HOOK(VpLineCtxType, pLineCtx, ("Valid Off-Hook on line %d at time %d",
                            channelId, pDevObj->timeStamp));

                        pLineObj->leakyLineCnt = 0;
                        pLineObj->status &= ~VP880_LINE_LEAK;

                        pLineObj->dpStruct.hookSt = TRUE;
                        pLineObj->dpStruct2.hookSt = TRUE;

                        if(pLineObj->pulseMode != VP_OPTION_PULSE_DECODE_OFF) {
                            pLineObj->dpStruct.state = VP_DP_DETECT_STATE_LOOP_CLOSE;
                            pLineObj->dpStruct.lc_time = 0;

                            pLineObj->dpStruct2.state = VP_DP_DETECT_STATE_LOOP_CLOSE;
                            pLineObj->dpStruct2.lc_time = 0;
                        }

                        if (!(pLineObj->status & VP880_PREVIOUS_HOOK)) {
                            pLineObj->lineEvents.signaling |= VP_LINE_EVID_HOOK_OFF;
                            pLineObj->lineEventHandle = pDevObj->timeStamp;
                        }

                        pLineObj->lineState.condition |= VP_CSLAC_HOOK;
                    }
                }
            }
        }
    }
}

/**
 * Vp880ServiceFxsTimers()
 *  This function services active FXS API timers.
 *
 * Preconditions:
 *  This Function must be called from the ApiTick function once per device.
 *
 * Postconditions:
 *  All Active FXS Timers for the current line have been serviced.
 */
void
Vp880ServiceFxsTimers(
    VpLineCtxType *pLineCtx)
{
    Vp880LineObjectType *pLineObj = pLineCtx->pLineObj;
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpDeviceIdType deviceId = pDevObj->deviceId;

    uint8 channelId = pLineObj->channelId;
    uint8 ecVal = pLineObj->ecVal;

    VpLineTimerType lineTimerType;
    uint16 tempTimer;

    for (lineTimerType = 0; lineTimerType < VP_LINE_TIMER_LAST;
         lineTimerType++) {

        if (pLineObj->lineTimers.timers.timer[lineTimerType]
          & VP_ACTIVATE_TIMER) {

            tempTimer = (pLineObj->lineTimers.timers.timer[lineTimerType]
                & VP_TIMER_TIME_MASK);

            if (tempTimer > 0) {
                tempTimer--;
            }

            pLineObj->lineTimers.timers.timer[lineTimerType] = tempTimer;

            if (tempTimer == 0) {
                pLineObj->lineTimers.timers.timer[lineTimerType] &=
                    ~(VP_ACTIVATE_TIMER);

                /* Perform appropriate action based on timerType */
                switch (lineTimerType) {
                    case VP_LINE_RING_EXIT_DEBOUNCE:
                        /*
                         * Allow correcttion of LPM when exiting Ringing due to
                         * initial delay provided in LPM code.
                         */
                        if ((pLineObj->termType == VP_TERM_FXS_LOW_PWR) ||
                            (pLineObj->termType == VP_TERM_FXS_ISOLATE_LP) ||
                            (pLineObj->termType == VP_TERM_FXS_SPLITTER_LP)) {
                            Vp880LLSetSysState(deviceId, pLineCtx, 0x00, FALSE);
                        }

                    case VP_LINE_CID_DEBOUNCE:
                    case VP_LINE_POLREV_DEBOUNCE:
                        pDevObj->status.state |= VP_DEV_FORCE_SIG_READ;
                        pDevObj->status.state |= VP_DEV_PENDING_INT;
                        break;

                    case VP_LINE_DISCONNECT_EXIT:
                        Vp880ServiceDisconnectExitTimer(pLineCtx);
                        break;

                    case VP_LINE_GND_START_TIMER:
                        if (pLineObj->lineState.currentState != VP_LINE_TIP_OPEN) {
                            pLineObj->icr3Values[VP880_ICR3_LONG_LOOP_CTRL_LOCATION] |=
                                VP880_ICR3_LONG_LOOP_CONTROL;
                            pLineObj->icr3Values[VP880_ICR3_LONG_LOOP_CTRL_LOCATION+1] |=
                                VP880_ICR3_LONG_LOOP_CONTROL;

                            VP_LINE_STATE(VpLineCtxType, pLineCtx, ("1. Ground Key Timer: Write ICR3 0x%02X 0x%02X 0x%02X 0x%02X Ch %d Time %d",
                                pLineObj->icr3Values[0], pLineObj->icr3Values[1],
                                pLineObj->icr3Values[2], pLineObj->icr3Values[3],
                                channelId, pDevObj->timeStamp));
                            VpMpiCmdWrapper(deviceId, ecVal,
                                VP880_ICR3_WRT, VP880_ICR3_LEN, pLineObj->icr3Values);

                            VpSysWait(1);

                            pLineObj->icr4Values[VP880_ICR4_GKEY_DET_LOCATION] |=
                                VP880_ICR4_GKEY_DET;
                            pLineObj->icr4Values[VP880_ICR4_GKEY_DET_LOCATION+1] |=
                                VP880_ICR4_GKEY_DET;

                            VP_LINE_STATE(VpLineCtxType, pLineCtx, ("1. Ground Key Timer: Write ICR4 0x%02X 0x%02X 0x%02X 0x%02X Ch %d Time %d",
                                pLineObj->icr4Values[0], pLineObj->icr4Values[1],
                                pLineObj->icr4Values[2], pLineObj->icr4Values[3],
                                channelId, pDevObj->timeStamp));
                            VpMpiCmdWrapper(deviceId, ecVal,
                                VP880_ICR4_WRT, VP880_ICR4_LEN, pLineObj->icr4Values);

                            VpSysWait(1);

                            pLineObj->icr3Values[VP880_ICR3_LONG_LOOP_CTRL_LOCATION] &=
                                ~VP880_ICR3_LONG_LOOP_CONTROL;

                            VP_LINE_STATE(VpLineCtxType, pLineCtx, ("2. Ground Key Timer: Write ICR3 0x%02X 0x%02X 0x%02X 0x%02X Ch %d",
                                pLineObj->icr3Values[0], pLineObj->icr3Values[1],
                                pLineObj->icr3Values[2], pLineObj->icr3Values[3], channelId));

                            VpMpiCmdWrapper(deviceId, ecVal,
                                VP880_ICR3_WRT, VP880_ICR3_LEN, pLineObj->icr3Values);

                            pLineObj->icr4Values[VP880_ICR4_GKEY_DET_LOCATION] &=
                                ~VP880_ICR4_GKEY_DET;

                            VP_LINE_STATE(VpLineCtxType, pLineCtx, ("2. Ground Key Timer: Write ICR4 0x%02X 0x%02X 0x%02X 0x%02X Ch %d",
                                pLineObj->icr4Values[0], pLineObj->icr4Values[1],
                                pLineObj->icr4Values[2], pLineObj->icr4Values[3], channelId));

                            VpMpiCmdWrapper(deviceId, ecVal,
                                VP880_ICR4_WRT, VP880_ICR4_LEN, pLineObj->icr4Values);

                            if (((pLineObj->termType == VP_TERM_FXS_LOW_PWR) ||
                                 (pLineObj->termType == VP_TERM_FXS_ISOLATE_LP) ||
                                 (pLineObj->termType == VP_TERM_FXS_SPLITTER_LP)) &&
                                (pLineObj->status & VP880_LOW_POWER_EN)) {

                                pLineObj->lineTimers.timers.timer[VP_LINE_POLREV_DEBOUNCE] =
                                    (MS_TO_TICKRATE(VP880_LPM_GND_START_TIME,
                                        pDevObj->devProfileData.tickRate)) | VP_ACTIVATE_TIMER;
                            }
                        }
                        break;

                    case VP_LINE_CAL_LINE_TIMER:
                        Vp880CalLineInt(pLineCtx);
                        break;

                   case VP_LINE_PING_TIMER:
                        if (((pDevObj->stateInt & (VP880_LINE0_LP | VP880_LINE1_LP))
                            == (VP880_LINE0_LP | VP880_LINE1_LP))
                         && (!(pLineObj->status & VP880_LINE_LEAK))
                         && (pLineObj->lineState.usrCurrent == VP_LINE_STANDBY)) {
                            pLineObj->nextSlicValue = VP880_SS_DISCONNECT;
                        }

                        VP_LINE_STATE(VpLineCtxType, pLineCtx, ("\n\r3. PING TIMER -- Setting Chan %d to Value 0x%02X at Time %d",
                            channelId, pLineObj->nextSlicValue, pDevObj->timeStamp));

                        Vp880UpdateBufferChanSel(pDevObj, channelId, pLineObj->nextSlicValue);
                        VpMpiCmdWrapper(deviceId, ecVal, VP880_SYS_STATE_WRT,
                            VP880_SYS_STATE_LEN, &pLineObj->nextSlicValue);
                        break;

                    case VP_LINE_TRACKER_DISABLE:
                        {
                            uint8 sysState[VP880_SYS_STATE_LEN] =
                                {VP880_SS_DISCONNECT};

                            /* Set line to Disconnect */
                            VP_LINE_STATE(VpLineCtxType, pLineCtx, ("Tracker Disable: Setting Ch %d to State 0x%02X at time %d",
                                channelId, sysState[0], pDevObj->timeStamp));

                            Vp880UpdateBufferChanSel(pDevObj, channelId, sysState[0]);
                            VpMpiCmdWrapper(deviceId, ecVal, VP880_SYS_STATE_WRT,
                                VP880_SYS_STATE_LEN, sysState);

                            pLineObj->icr2Values[VP880_ICR2_VOC_DAC_INDEX] |= VP880_ICR2_ILA_DAC;
                            pLineObj->icr2Values[VP880_ICR2_VOC_DAC_INDEX] &= ~VP880_ICR2_VOC_DAC_SENSE;
                            pLineObj->icr2Values[VP880_ICR2_VOC_DAC_INDEX+1] &= ~VP880_ICR2_ILA_DAC;

                            VP_LINE_STATE(VpLineCtxType, pLineCtx, ("Tracker Disable: ICR2 0x%02X 0x%02X 0x%02X 0x%02X Ch %d",
                                pLineObj->icr2Values[0], pLineObj->icr2Values[1],
                                pLineObj->icr2Values[2], pLineObj->icr2Values[3],
                                channelId));

                            VpMpiCmdWrapper(deviceId, ecVal, VP880_ICR2_WRT,
                                VP880_ICR2_LEN, pLineObj->icr2Values);
                        }
                        break;

                    case VP_LINE_INTERNAL_TESTTERM_TIMER: {
                        /* Apply new bias settings to keep tip/ring near battery. */
                        uint8 icr1Reg[VP880_ICR1_LEN];
                        icr1Reg[0] = 0xFF;
                        icr1Reg[1] = 0x18;
                        icr1Reg[2] = 0xFF;
                        icr1Reg[3] = 0x04;
                        VpMpiCmd(deviceId, ecVal, VP880_ICR1_WRT,
                            VP880_ICR1_LEN, icr1Reg);
                        break;
                    }

                    default:
                        break;
                } /* Switch (timerType) */
            } else { /* If timer has not expired, keep it going */
                pLineObj->lineTimers.timers.timer[lineTimerType]
                    |= VP_ACTIVATE_TIMER;
            }
        } /* if timerType is active     */
    } /* Loop through all timerTypes for chanID */

    return;
}

/**
 * Vp880ServiceDisconnectExitTimer()
 *  This function services active Disconnect Exit API timers.
 *
 * Preconditions:
 *  This Function must be called from the ApiTick function once per device.
 *
 * Postconditions:
 *  Active Disconnect Exit Timers for the current line have been serviced.
 */
void
Vp880ServiceDisconnectExitTimer(
    VpLineCtxType *pLineCtx)
{
    Vp880LineObjectType *pLineObj = pLineCtx->pLineObj;
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;

    VpDeviceIdType deviceId = pDevObj->deviceId;

    uint8 channelId = pLineObj->channelId;
    uint8 ecVal = pLineObj->ecVal;

    if ((pLineObj->termType == VP_TERM_FXS_LOW_PWR) ||
        (pLineObj->termType == VP_TERM_FXS_ISOLATE_LP) ||
        (pLineObj->termType == VP_TERM_FXS_SPLITTER_LP)) {
        if(pLineObj->lineState.calType != VP_CSLAC_CAL_NONE) {
            pLineObj->nextSlicValue &= VP880_SS_POLARITY_MASK;
            pLineObj->nextSlicValue |= VP880_SS_ACTIVE;
        }

        VP_LINE_STATE(VpLineCtxType, pLineCtx, ("Disconnect Exit Timer: Setting Ch %d to State 0x%02X at time %d",
            channelId, pLineObj->nextSlicValue, pDevObj->timeStamp));

        if (pLineObj->lineState.currentState == VP_LINE_DISCONNECT) {
            pLineObj->icr2Values[VP880_ICR2_VOC_DAC_INDEX] |= VP880_ICR2_ILA_DAC;
            pLineObj->icr2Values[VP880_ICR2_VOC_DAC_INDEX] &= ~VP880_ICR2_VOC_DAC_SENSE;
            pLineObj->icr2Values[VP880_ICR2_VOC_DAC_INDEX+1] &= ~VP880_ICR2_ILA_DAC;
        } else if (pLineObj->nextSlicValue == VP880_SS_DISCONNECT) {
            uint8 swParams[VP880_REGULATOR_PARAM_LEN];
            VpMpiCmdWrapper(deviceId, ecVal, VP880_REGULATOR_PARAM_RD,
                VP880_REGULATOR_PARAM_LEN, swParams);
            if ((swParams[VP880_FLOOR_VOLTAGE_BYTE] & 0x0D) != 0x0D) {
                swParams[VP880_FLOOR_VOLTAGE_BYTE] &= ~VP880_FLOOR_VOLTAGE_MASK;
                swParams[VP880_FLOOR_VOLTAGE_BYTE] |= 0x0D;   /* 70V */
                VpMpiCmdWrapper(deviceId, ecVal, VP880_REGULATOR_PARAM_WRT,
                    VP880_REGULATOR_PARAM_LEN, swParams);
            }

            pLineObj->icr2Values[VP880_ICR2_SENSE_INDEX] =
                (VP880_ICR2_TIP_SENSE | VP880_ICR2_RING_SENSE
                | VP880_ICR2_ILA_DAC | VP880_ICR2_FEED_SENSE);
            pLineObj->icr2Values[VP880_ICR2_SENSE_INDEX+1] =
                (VP880_ICR2_TIP_SENSE | VP880_ICR2_RING_SENSE
               | VP880_ICR2_FEED_SENSE);
        }

        VP_LINE_STATE(VpLineCtxType, pLineCtx, ("Disconnect Exit Timer: ICR2 0x%02X 0x%02X 0x%02X 0x%02X Ch %d",
            pLineObj->icr2Values[0], pLineObj->icr2Values[1],
            pLineObj->icr2Values[2], pLineObj->icr2Values[3],
            channelId));

        VpMpiCmdWrapper(deviceId, ecVal, VP880_ICR2_WRT,
            VP880_ICR2_LEN, pLineObj->icr2Values);

        /*
         * Release Tip and Ring Bias Override. Set Line Bias to
         * normal values.
         */
        pLineObj->icr1Values[VP880_ICR1_BIAS_OVERRIDE_LOCATION] &=
            ~VP880_ICR1_TIP_BIAS_OVERRIDE;
        pLineObj->icr1Values[VP880_ICR1_BIAS_OVERRIDE_LOCATION+1] &=
            ~(VP880_ICR1_TIP_BIAS_OVERRIDE | VP880_ICR1_LINE_BIAS_OVERRIDE);
        pLineObj->icr1Values[VP880_ICR1_BIAS_OVERRIDE_LOCATION+1] |=
            VP880_ICR1_LINE_BIAS_OVERRIDE_NORM;

        pLineObj->icr1Values[VP880_ICR1_RING_BIAS_OVERRIDE_LOCATION] &=
            ~VP880_ICR1_RING_BIAS_OVERRIDE;
        VpMpiCmdWrapper(deviceId, ecVal, VP880_ICR1_WRT, VP880_ICR1_LEN,
            pLineObj->icr1Values);

        if (pLineObj->nextSlicValue == VP880_SS_TIP_OPEN) {
            Vp880GroundStartProc(TRUE, pLineCtx, 0x00,
                pLineObj->nextSlicValue);
        } else {
            Vp880UpdateBufferChanSel(pDevObj, channelId, pLineObj->nextSlicValue);
            VpMpiCmdWrapper(deviceId, ecVal, VP880_SYS_STATE_WRT,
                VP880_SYS_STATE_LEN, &pLineObj->nextSlicValue);
        }
    }
    pDevObj->status.state |= VP_DEV_FORCE_SIG_READ;
    pDevObj->status.state |= VP_DEV_PENDING_INT;
}

/**
 * Vp880ServiceFxoTimers()
 *  This function services active FXO API timers.
 *
 * Preconditions:
 *  This Function must be called from the ApiTick function once per device.
 *
 * Postconditions:
 *  All Active FXO Timers for the current line have been serviced.
 */
void
Vp880ServiceFxoTimers(
    VpLineCtxType *pLineCtx)
{
    Vp880LineObjectType *pLineObj = pLineCtx->pLineObj;
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpFXOTimerType *pFxoTimer = &pLineObj->lineTimers.timers.fxoTimer;
    uint16 tickAdder =
        pDevObj->devProfileData.tickRate / (VP_CSLAC_TICKSTEP_0_5MS / 2);

    /* Increment the time since polrev was observed */
    if (pFxoTimer->timeLastPolRev  < (0x7FFF - tickAdder)) {
        /*
         * The time is in 0.25mS increments, but the device
         * tickrate is something else. Increment by the scaled
         * amount.
         */
        pFxoTimer->timeLastPolRev += tickAdder;
    } else {
        /* Max limit the value of last polrev value */
        pFxoTimer->timeLastPolRev = 0x7FFF;
    }

    /* Set tick adder for 1ms increments */
    tickAdder =
        pDevObj->devProfileData.tickRate / (VP_CSLAC_TICKSTEP_0_5MS * 2);

    if (((uint16)pFxoTimer->lastStateChange + tickAdder) > 255) {
        pFxoTimer->lastStateChange = 255;
    } else {
        pFxoTimer->lastStateChange += tickAdder;
    }

    if (((uint16)pFxoTimer->lastNotLiu - tickAdder) <= 0) {
        pFxoTimer->lastNotLiu = 0;
    } else {
        pFxoTimer->lastNotLiu-=tickAdder;
    }

    if (((uint16)pFxoTimer->fxoDiscIO2Change - tickAdder) <= 0) {
        pFxoTimer->fxoDiscIO2Change = 0;
    } else {
        pFxoTimer->fxoDiscIO2Change -= tickAdder;
    }

    if (pFxoTimer->disconnectDebounce >= tickAdder) {
        pFxoTimer->disconnectDebounce -= tickAdder;

        if (pFxoTimer->disconnectDebounce ==0) {
            if (pLineObj->preDisconnect ==
                (pLineObj->lineState.condition & VP_CSLAC_RAW_DISC)) {
                /*
                 * There is a change that persisted longer than the
                 * debounce interval. Evaluate and generate event
                 */
                pLineObj->lineEventHandle = pDevObj->timeStamp;

                switch(pLineObj->lineState.usrCurrent) {
                    case VP_LINE_FXO_TALK:
                    case VP_LINE_FXO_LOOP_CLOSE:
                        if (pLineObj->preDisconnect == VP_CSLAC_RAW_DISC) {
                            if (!(pLineObj->lineState.condition & VP_CSLAC_DISC)) {
                                pLineObj->lineState.condition |= VP_CSLAC_DISC;
                                pLineObj->lineEvents.fxo |= VP_LINE_EVID_DISCONNECT;
                            }
                        } else {
                            if (pLineObj->lineState.condition & VP_CSLAC_DISC) {
                                pLineObj->lineState.condition &= ~VP_CSLAC_DISC;
                                pLineObj->lineEvents.fxo |= VP_LINE_EVID_RECONNECT;
                            }
                        }
                        break;

                    default:
                        if (pLineObj->preDisconnect == VP_CSLAC_RAW_DISC) {
                            if (!(pLineObj->lineState.condition & VP_CSLAC_DISC)) {
                                pLineObj->lineState.condition |= VP_CSLAC_DISC;
                                pLineObj->lineEvents.fxo |= VP_LINE_EVID_FEED_DIS;
                            }
                        } else {
                            if (pLineObj->lineState.condition & VP_CSLAC_DISC) {
                                pLineObj->lineState.condition &= ~VP_CSLAC_DISC;
                                pLineObj->lineEvents.fxo |= VP_LINE_EVID_FEED_EN;
                            }
                        }
                        break;
                }
            }
        }
    }
}

/**
 * Vp880FSKGeneratorReady()
 *  This function is used for Caller ID to determine if the FSK generator is
 * ready to accept another byte. It uses the device caller ID state machine
 * and signaling (caller ID status) register. This function should be called
 * from an API internal function only.
 *
 * Returns:
 *  TRUE if the FSK generator for Caller ID can accept a byte, FALSE otherwise.
 */
bool
Vp880FSKGeneratorReady(
    VpLineCtxType *pLineCtx)
{
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    Vp880LineObjectType *pLineObj = pLineCtx->pLineObj;
    uint8 cidParam[VP880_CID_PARAM_LEN];
    VpDeviceIdType deviceId = pDevObj->deviceId;
    uint8 ecVal = pLineObj->ecVal;
    uint8 cidState;
    uint8 devRev = pDevObj->staticInfo.rcnPcn[VP880_RCN_LOCATION];

    /* Check the Generator State */
    VpMpiCmdWrapper(deviceId, ecVal, VP880_CID_PARAM_RD,
        VP880_CID_PARAM_LEN, cidParam);
    cidState = (cidParam[0] & VP880_CID_STATE_MASK);

    VP_CID(VpLineCtxType, pLineCtx, ("CID Param 0x%02X, State 0x%02X", cidParam[0], cidState));

    /* Check to see if the device is ready to accept (more) CID data */
    if (devRev <= VP880_REV_VC) {
        if (cidState == VP880_CID_STATE_FULL) {
            return FALSE;
        } else {
            return TRUE;
        }
    } else {
        if (cidState == VP880_CID_STATE_FULL_D) {
            return FALSE;
        } else {
            return TRUE;
        }
    }
}

/**
 * Vp880GetRelayState()
 *  This function returns the current relay state of VP880 device.
 *
 * Preconditions:
 *  Device/Line context should be created and initialized.
 *
 * Postconditions:
 *  The indicated relay state is returned for the given line.
 */
VpStatusType
Vp880GetRelayState(
    VpLineCtxType           *pLineCtx,
    VpRelayControlType      *pRstate)
{
    VpDevCtxType            *pDevCtx;
    Vp880DeviceObjectType   *pDevObj;
    Vp880LineObjectType     *pLineObj;
    VpDeviceIdType          deviceId;

    if(pLineCtx == VP_NULL) {
        return VP_STATUS_INVALID_ARG;
    }

    pDevCtx = pLineCtx->pDevCtx;
    pLineObj = pLineCtx->pLineObj;
    pDevObj = pDevCtx->pDevObj;
    deviceId = pDevObj->deviceId;

    VpSysEnterCritical(deviceId, VP_CODE_CRITICAL_SEC);
    *pRstate = pLineObj->relayState;
    VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);

    return VP_STATUS_SUCCESS;
}

#if 1//def VP_DEBUG
/**
 * Vp880RegisterDump()
 *  Dump all known 880 device and line specific registers (for debug purposes).
 *
 * Returns:
 */
VpStatusType
Vp880RegisterDump(
    VpDevCtxType *pDevCtx)
{
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpDeviceIdType deviceId = pDevObj->deviceId;

    uint8 channelId;

    uint8 intSwRegParam[VP880_INT_SWREG_PARAM_LEN];
    uint8 swYZ[VP880_REGULATOR_PARAM_LEN];
    uint8 devType[VP880_DEVTYPE_LEN];
    uint8 testReg1[VP880_TEST_REG1_LEN];
    uint8 testReg2[VP880_TEST_REG2_LEN];
    uint8 txRxSlot[VP880_XR_CS_LEN];
    uint8 devCfg[VP880_DCR_LEN];
    uint8 opMode[VP880_OP_MODE_LEN];
    uint8 sigReg[VP880_NO_UL_SIGREG_LEN];
    uint8 intMask[VP880_INT_MASK_LEN];
    uint8 intRev[VP880_REV_INFO_LEN];
    uint8 swCtrl[VP880_REGULATOR_CTRL_LEN];
    uint8 devMode[VP880_DEV_MODE_LEN];

    uint8 registerIndex;

#ifdef VP_ERROR
#undef VP_ERROR
#define VP_ERROR(objType, pObj, printf_args) printk printf_args
#endif

    VpMpiCmdWrapper(deviceId, VP880_EC_CH1, VP880_DEVTYPE_RD, VP880_DEVTYPE_LEN, devType);
    VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_DEVTYPE_RD                (0x%02X) ", VP880_DEVTYPE_RD));
    for (registerIndex = 0; registerIndex < VP880_DEVTYPE_LEN; registerIndex++) {
        VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", devType[registerIndex]));
    }

    VpMpiCmdWrapper(deviceId, VP880_EC_CH1, VP880_DEV_MODE_RD, VP880_DEV_MODE_LEN, devMode);
    VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_DEV_MODE_RD               (0x%02X) ", VP880_DEV_MODE_RD));
    for (registerIndex = 0; registerIndex < VP880_DEV_MODE_LEN; registerIndex++) {
        VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", devMode[registerIndex]));
    }

    VpMpiCmdWrapper(deviceId, VP880_EC_CH1, VP880_TEST_REG1_RD, VP880_TEST_REG1_LEN, testReg1);
    VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_TEST_REG1_RD              (0x%02X) ", VP880_TEST_REG1_RD));
    for (registerIndex = 0; registerIndex < VP880_TEST_REG1_LEN; registerIndex++) {
        VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", testReg1[registerIndex]));
    }

    VpMpiCmdWrapper(deviceId, VP880_EC_CH1, VP880_TEST_REG2_RD, VP880_TEST_REG2_LEN, testReg2);
    VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_TEST_REG2_RD              (0x%02X) ", VP880_TEST_REG2_RD));
    for (registerIndex = 0; registerIndex < VP880_TEST_REG2_LEN; registerIndex++) {
        VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", testReg1[registerIndex]));
    }

    VpMpiCmdWrapper(deviceId, VP880_EC_CH1, VP880_XR_CS_RD, VP880_XR_CS_LEN, txRxSlot);
    VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_XR_CS_RD                  (0x%02X) ", VP880_XR_CS_RD));
    for (registerIndex = 0; registerIndex < VP880_XR_CS_LEN; registerIndex++) {
        VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", txRxSlot[registerIndex]));
    }

    VpMpiCmdWrapper(deviceId, VP880_EC_CH1, VP880_DCR_RD, VP880_DCR_LEN, devCfg);
    VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_DCR_RD                    (0x%02X) ", VP880_DCR_RD));
    for (registerIndex = 0; registerIndex < VP880_DCR_LEN; registerIndex++) {
        VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", devCfg[registerIndex]));
    }

    VpMpiCmdWrapper(deviceId, VP880_EC_CH1, VP880_OP_MODE_RD, VP880_OP_MODE_LEN, opMode);
    VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_OP_MODE_RD                (0x%02X) ", VP880_OP_MODE_RD));
    for (registerIndex = 0; registerIndex < VP880_OP_MODE_LEN; registerIndex++) {
        VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", opMode[registerIndex]));
    }

    VpMpiCmdWrapper(deviceId, VP880_EC_CH1, VP880_NO_UL_SIGREG_RD, VP880_NO_UL_SIGREG_LEN, sigReg);
    VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_NO_UL_SIGREG_RD           (0x%02X) ", VP880_NO_UL_SIGREG_RD));
    for (registerIndex = 0; registerIndex < VP880_NO_UL_SIGREG_LEN; registerIndex++) {
        VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", sigReg[registerIndex]));
    }

    VpMpiCmdWrapper(deviceId, VP880_EC_CH1, VP880_INT_MASK_RD, VP880_INT_MASK_LEN, intMask);
    VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_INT_MASK_RD               (0x%02X) ", VP880_INT_MASK_RD));
    for (registerIndex = 0; registerIndex < VP880_INT_MASK_LEN; registerIndex++) {
        VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", intMask[registerIndex]));
    }

    VpMpiCmdWrapper(deviceId, VP880_EC_CH1, VP880_REV_INFO_RD, VP880_REV_INFO_LEN, intRev);
    VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_REV_INFO_RD               (0x%02X) ", VP880_REV_INFO_RD));
    for (registerIndex = 0; registerIndex < VP880_REV_INFO_LEN; registerIndex++) {
        VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", intRev[registerIndex]));
    }

    VpMpiCmdWrapper(deviceId, VP880_EC_CH1, VP880_REGULATOR_PARAM_RD, VP880_REGULATOR_PARAM_LEN, swYZ);
    VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_REGULATOR_PARAM_RD        (0x%02X) ", VP880_REGULATOR_PARAM_RD));
    for (registerIndex = 0; registerIndex < VP880_REGULATOR_PARAM_LEN; registerIndex++) {
        VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", swYZ[registerIndex]));
    }

    VpMpiCmdWrapper(deviceId, VP880_EC_CH1, VP880_REGULATOR_CTRL_RD, VP880_REGULATOR_CTRL_LEN, swCtrl);
    VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_REGULATOR_CTRL_RD         (0x%02X) ", VP880_REGULATOR_CTRL_RD));
    for (registerIndex = 0; registerIndex < VP880_REGULATOR_CTRL_LEN; registerIndex++) {
        VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", swCtrl[registerIndex]));
    }

    VpMpiCmdWrapper(deviceId, VP880_EC_CH1, VP880_INT_SWREG_PARAM_RD, VP880_INT_SWREG_PARAM_LEN, intSwRegParam);
    VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_INT_SWREG_PARAM_RD        (0x%02X) ", VP880_INT_SWREG_PARAM_RD));
    for (registerIndex = 0; registerIndex < VP880_INT_SWREG_PARAM_LEN; registerIndex++) {
        VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", intSwRegParam[registerIndex]));
    }

    for (channelId = 0; channelId < pDevObj->staticInfo.maxChannels; channelId++) {
        uint8 sysState[VP880_SYS_STATE_LEN];
        uint8 sysStateCfg[VP880_SS_CONFIG_LEN];
        uint8 convCfg[VP880_CONV_CFG_LEN];

        uint8 icr1[VP880_ICR1_LEN];
        uint8 icr2[VP880_ICR2_LEN];
        uint8 icr3[VP880_ICR3_LEN];
        uint8 icr4[VP880_ICR4_LEN];
        uint8 icr5[VP880_ICR5_LEN];
        uint8 dcCal[VP880_DC_CAL_REG_LEN];

        uint8 disn[VP880_DISN_LEN];
        uint8 vpGain[VP880_VP_GAIN_LEN];
        uint8 grGain[VP880_GR_GAIN_LEN];
        uint8 gxGain[VP880_GX_GAIN_LEN];

        uint8 x[VP880_X_FILTER_LEN];
        uint8 r[VP880_R_FILTER_LEN];

        uint8 b1[VP880_B1_FILTER_LEN];
        uint8 b2[VP880_B2_FILTER_LEN];

        uint8 z1[VP880_Z1_FILTER_LEN];
        uint8 z2[VP880_Z2_FILTER_LEN];

        uint8 opFunc[VP880_CODEC_REG_LEN];
        uint8 opCond[VP880_OP_COND_LEN];

        uint8 ioData[VP880_IODATA_REG_LEN];
        uint8 ioDir[VP880_IODIR_REG_LEN];
        uint8 batCal[VP880_BAT_CALIBRATION_LEN];

        uint8 txSlot[VP880_TX_TS_LEN];
        uint8 rxSlot[VP880_RX_TS_LEN];

        uint8 dcFeed[VP880_DC_FEED_LEN];
        uint8 loopSup[VP880_LOOP_SUP_LEN];

        uint8 meteringParams[VP880_METERING_PARAM_LEN];

        uint8 sigABParams[VP880_SIGA_PARAMS_LEN];
        uint8 sigCDParams[VP880_SIGCD_PARAMS_LEN];

        uint8 cadenceParams[VP880_CADENCE_TIMER_LEN];
        uint8 cidParams[VP880_CID_PARAM_LEN];
        uint8 ecVal = (channelId == 0) ? VP880_EC_CH1 : VP880_EC_CH2;

        VP_ERROR(VpDevCtxType, pDevCtx,("\n\nCHANNEL %d", channelId));

        VpMpiCmdWrapper(deviceId, ecVal, VP880_VP_GAIN_RD, VP880_VP_GAIN_LEN, vpGain);
        VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_VP_GAIN_RD            (0x%02X) ", VP880_VP_GAIN_RD));
        for (registerIndex = 0; registerIndex < VP880_VP_GAIN_LEN; registerIndex++) {
            VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", vpGain[registerIndex]));
        }

        VpMpiCmdWrapper(deviceId, ecVal, VP880_IODATA_REG_RD, VP880_IODATA_REG_LEN, ioData);
        VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_IODATA_REG_RD         (0x%02X) ", VP880_IODATA_REG_RD));
        for (registerIndex = 0; registerIndex < VP880_IODATA_REG_LEN; registerIndex++) {
            VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", ioData[registerIndex]));
        }

        VpMpiCmdWrapper(deviceId, ecVal, VP880_IODIR_REG_RD, VP880_IODIR_REG_LEN, ioDir);
        VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_IODIR_REG_RD          (0x%02X) ", VP880_IODIR_REG_RD));
        for (registerIndex = 0; registerIndex < VP880_IODIR_REG_LEN; registerIndex++) {
            VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", ioDir[registerIndex]));
        }

        VpMpiCmdWrapper(deviceId, ecVal, VP880_BAT_CALIBRATION_RD, VP880_BAT_CALIBRATION_LEN, batCal);
        VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_BAT_CALIBRATION_RD    (0x%02X) ", VP880_BAT_CALIBRATION_RD));
        for (registerIndex = 0; registerIndex < VP880_BAT_CALIBRATION_LEN; registerIndex++) {
            VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", batCal[registerIndex]));
        }

        VpMpiCmdWrapper(deviceId, ecVal, VP880_SYS_STATE_RD, VP880_SYS_STATE_LEN, sysState);
        VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_SYS_STATE_RD          (0x%02X) ", VP880_SYS_STATE_RD));
        for (registerIndex = 0; registerIndex < VP880_SYS_STATE_LEN; registerIndex++) {
            VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", sysState[registerIndex]));
        }

        VpMpiCmdWrapper(deviceId, ecVal, VP880_SS_CONFIG_RD, VP880_SS_CONFIG_LEN, sysStateCfg);
        VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_SS_CONFIG_RD          (0x%02X) ", VP880_SS_CONFIG_RD));
        for (registerIndex = 0; registerIndex < VP880_SS_CONFIG_LEN; registerIndex++) {
            VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", sysStateCfg[registerIndex]));
        }

        VpMpiCmdWrapper(deviceId, ecVal, VP880_CONV_CFG_RD, VP880_CONV_CFG_LEN, convCfg);
        VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_CONV_CFG_RD           (0x%02X) ", VP880_CONV_CFG_RD));
        for (registerIndex = 0; registerIndex < VP880_CONV_CFG_LEN; registerIndex++) {
            VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", convCfg[registerIndex]));
        }

        VpMpiCmdWrapper(deviceId, ecVal, VP880_CODEC_REG_RD, VP880_CODEC_REG_LEN, opFunc);
        VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_CODEC_REG_RD          (0x%02X) ", VP880_CODEC_REG_RD));
        for (registerIndex = 0; registerIndex < VP880_CODEC_REG_LEN; registerIndex++) {
            VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", opFunc[registerIndex]));
        }

        VpMpiCmdWrapper(deviceId, ecVal, VP880_OP_COND_RD, VP880_OP_COND_LEN, opCond);
        VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_OP_COND_RD            (0x%02X) ", VP880_OP_COND_RD));
        for (registerIndex = 0; registerIndex < VP880_OP_COND_LEN; registerIndex++) {
            VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", opCond[registerIndex]));
        }

        VpMpiCmdWrapper(deviceId, ecVal, VP880_DISN_RD, VP880_DISN_LEN, disn);
        VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_DISN_RD               (0x%02X) ", VP880_DISN_RD));
        for (registerIndex = 0; registerIndex < VP880_DISN_LEN; registerIndex++) {
            VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", disn[registerIndex]));
        }

        VpMpiCmdWrapper(deviceId, ecVal, VP880_GX_GAIN_RD, VP880_GX_GAIN_LEN, gxGain);
        VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_GX_GAIN_RD            (0x%02X) ", VP880_GX_GAIN_RD));
        for (registerIndex = 0; registerIndex < VP880_GX_GAIN_LEN; registerIndex++) {
            VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", gxGain[registerIndex]));
        }

        VpMpiCmdWrapper(deviceId, ecVal, VP880_GR_GAIN_RD, VP880_GR_GAIN_LEN, grGain);
        VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_GR_GAIN_RD            (0x%02X) ", VP880_GR_GAIN_RD));
        for (registerIndex = 0; registerIndex < VP880_GR_GAIN_LEN; registerIndex++) {
            VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", grGain[registerIndex]));
        }

        VpMpiCmdWrapper(deviceId, ecVal, VP880_B1_FILTER_RD, VP880_B1_FILTER_LEN, b1);
        VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_B1_FILTER_RD          (0x%02X) ", VP880_B1_FILTER_RD));
        for (registerIndex = 0; registerIndex < VP880_B1_FILTER_LEN; registerIndex++) {
            VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", b1[registerIndex]));
        }

        VpMpiCmdWrapper(deviceId, ecVal, VP880_B2_FILTER_RD, VP880_B2_FILTER_LEN, b2);
        VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_B2_FILTER_RD          (0x%02X) ", VP880_B2_FILTER_RD));
        for (registerIndex = 0; registerIndex < VP880_B2_FILTER_LEN; registerIndex++) {
            VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", b2[registerIndex]));
        }

        VpMpiCmdWrapper(deviceId, ecVal, VP880_X_FILTER_RD, VP880_X_FILTER_LEN, x);
        VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_X_FILTER_RD           (0x%02X) ", VP880_X_FILTER_RD));
        for (registerIndex = 0; registerIndex < VP880_X_FILTER_LEN; registerIndex++) {
            VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", x[registerIndex]));
        }

        VpMpiCmdWrapper(deviceId, ecVal, VP880_R_FILTER_RD, VP880_R_FILTER_LEN, r);
        VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_R_FILTER_RD           (0x%02X) ", VP880_R_FILTER_RD));
        for (registerIndex = 0; registerIndex < VP880_R_FILTER_LEN; registerIndex++) {
            VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", r[registerIndex]));
        }

        VpMpiCmdWrapper(deviceId, ecVal, VP880_Z1_FILTER_RD, VP880_Z1_FILTER_LEN, z1);
        VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_Z1_FILTER_RD          (0x%02X) ", VP880_Z1_FILTER_RD));
        for (registerIndex = 0; registerIndex < VP880_Z1_FILTER_LEN; registerIndex++) {
            VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", z1[registerIndex]));
        }

        VpMpiCmdWrapper(deviceId, ecVal, VP880_Z2_FILTER_RD, VP880_Z2_FILTER_LEN, z2);
        VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_Z2_FILTER_RD          (0x%02X) ", VP880_Z2_FILTER_RD));
        for (registerIndex = 0; registerIndex < VP880_Z2_FILTER_LEN; registerIndex++) {
            VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", z2[registerIndex]));
        }

        VpMpiCmdWrapper(deviceId, ecVal, VP880_TX_TS_RD, VP880_TX_TS_LEN, txSlot);
        VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_TX_TS_RD              (0x%02X) ", VP880_TX_TS_RD));
        for (registerIndex = 0; registerIndex < VP880_TX_TS_LEN; registerIndex++) {
            VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", txSlot[registerIndex]));
        }

        VpMpiCmdWrapper(deviceId, ecVal, VP880_RX_TS_RD, VP880_RX_TS_LEN, rxSlot);
        VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_RX_TS_RD              (0x%02X) ", VP880_RX_TS_RD));
        for (registerIndex = 0; registerIndex < VP880_RX_TS_LEN; registerIndex++) {
            VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", rxSlot[registerIndex]));
        }

        VpMpiCmdWrapper(deviceId, ecVal, VP880_DC_FEED_RD, VP880_DC_FEED_LEN, dcFeed);
        VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_DC_FEED_RD            (0x%02X) ", VP880_DC_FEED_RD));
        for (registerIndex = 0; registerIndex < VP880_DC_FEED_LEN; registerIndex++) {
            VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", dcFeed[registerIndex]));
        }

        VpMpiCmdWrapper(deviceId, ecVal, VP880_LOOP_SUP_RD, VP880_LOOP_SUP_LEN, loopSup);
        VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_LOOP_SUP_RD           (0x%02X) ", VP880_LOOP_SUP_RD));
        for (registerIndex = 0; registerIndex < VP880_LOOP_SUP_LEN; registerIndex++) {
            VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", loopSup[registerIndex]));
        }

        VpMpiCmdWrapper(deviceId, ecVal, VP880_METERING_PARAM_RD, VP880_METERING_PARAM_LEN, meteringParams);
        VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_METERING_PARAM_RD     (0x%02X) ", VP880_METERING_PARAM_RD));
        for (registerIndex = 0; registerIndex < VP880_METERING_PARAM_LEN; registerIndex++) {
            VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", meteringParams[registerIndex]));
        }

        VpMpiCmdWrapper(deviceId, ecVal, VP880_SIGA_PARAMS_RD, VP880_SIGA_PARAMS_LEN, sigABParams);
        VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_SIGA_PARAMS_RD        (0x%02X) ", VP880_SIGA_PARAMS_RD));
        for (registerIndex = 0; registerIndex < VP880_SIGA_PARAMS_LEN; registerIndex++) {
            VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", sigABParams[registerIndex]));
        }

        VpMpiCmdWrapper(deviceId, ecVal, VP880_SIGCD_PARAMS_RD, VP880_SIGCD_PARAMS_LEN, sigCDParams);
        VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_SIGCD_PARAMS_RD       (0x%02X) ", VP880_SIGCD_PARAMS_RD));
        for (registerIndex = 0; registerIndex < VP880_SIGCD_PARAMS_LEN; registerIndex++) {
            VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", sigCDParams[registerIndex]));
        }

        VpMpiCmdWrapper(deviceId, ecVal, VP880_CADENCE_TIMER_RD, VP880_CADENCE_TIMER_LEN, cadenceParams);
        VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_CADENCE_TIMER_RD      (0x%02X) ", VP880_CADENCE_TIMER_RD));
        for (registerIndex = 0; registerIndex < VP880_CADENCE_TIMER_LEN; registerIndex++) {
            VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", cadenceParams[registerIndex]));
        }

        VpMpiCmdWrapper(deviceId, ecVal, VP880_CID_PARAM_RD, VP880_CID_PARAM_LEN, cidParams);
        VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_CID_PARAM_RD          (0x%02X) ", VP880_CID_PARAM_RD));
        for (registerIndex = 0; registerIndex < VP880_CID_PARAM_LEN; registerIndex++) {
            VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", cidParams[registerIndex]));
        }

        VpMpiCmdWrapper(deviceId, ecVal, VP880_ICR1_RD, VP880_ICR1_LEN, icr1);
        VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_ICR1_RD               (0x%02X) ", VP880_ICR1_RD));
        for (registerIndex = 0; registerIndex < VP880_ICR1_LEN; registerIndex++) {
            VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", icr1[registerIndex]));
        }

        VpMpiCmdWrapper(deviceId, ecVal, VP880_ICR2_RD, VP880_ICR2_LEN, icr2);
        VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_ICR2_RD               (0x%02X) ", VP880_ICR2_RD));
        for (registerIndex = 0; registerIndex < VP880_ICR2_LEN; registerIndex++) {
            VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", icr2[registerIndex]));
        }

        VpMpiCmdWrapper(deviceId, ecVal, VP880_ICR3_RD, VP880_ICR3_LEN, icr3);
        VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_ICR3_RD               (0x%02X) ", VP880_ICR3_RD));
        for (registerIndex = 0; registerIndex < VP880_ICR3_LEN; registerIndex++) {
            VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", icr3[registerIndex]));
        }

        VpMpiCmdWrapper(deviceId, ecVal, VP880_ICR4_RD, VP880_ICR4_LEN, icr4);
        VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_ICR4_RD               (0x%02X) ", VP880_ICR4_RD));
        for (registerIndex = 0; registerIndex < VP880_ICR4_LEN; registerIndex++) {
            VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", icr4[registerIndex]));
        }

        VpMpiCmdWrapper(deviceId, ecVal, VP880_ICR5_RD, VP880_ICR5_LEN, icr5);
        VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_ICR5_RD               (0x%02X) ", VP880_ICR5_RD));
        for (registerIndex = 0; registerIndex < VP880_ICR5_LEN; registerIndex++) {
            VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", icr5[registerIndex]));
        }

        VpMpiCmdWrapper(deviceId, ecVal, VP880_DC_CAL_REG_RD, VP880_DC_CAL_REG_LEN, dcCal);
        VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_DC_CAL_REG_RD         (0x%02X) ", VP880_DC_CAL_REG_RD));
        for (registerIndex = 0; registerIndex < VP880_DC_CAL_REG_LEN; registerIndex++) {
            VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", dcCal[registerIndex]));
        }
    }
    return VP_STATUS_SUCCESS;
}
#endif

/**
 * Vp880RegisterReadWrite()
 *  Read/Write all known 880 device and line specific registers (for debug purposes).
 *
 * Returns:
 */
VpStatusType
Vp880RegisterReadWrite(
	VpLineCtxType   *pLineCtx,
	uint32			reg,
	uint8			*len,
	uint8			*regdata)
{
    Vp880DeviceObjectType *pDevObj = pLineCtx->pDevCtx->pDevObj;
	Vp880LineObjectType *pLineObj = pLineCtx->pLineObj;
    VpDeviceIdType deviceId = pDevObj->deviceId;

    uint8 channelId;
    uint8 registerIndex;

#ifdef VP_ERROR
#undef VP_ERROR
//#define VP_ERROR(objType, pObj, printf_args) printk printf_args
#define VP_ERROR(objType, pObj, printf_args) 
#endif

	if ((reg==VP880_DEVTYPE_RD))
	{
		VpMpiCmdWrapper(deviceId, VP880_EC_CH1, VP880_DEVTYPE_RD, VP880_DEVTYPE_LEN, regdata);
		VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_DEVTYPE_RD            (0x%02X) ", VP880_DEVTYPE_RD));
		for (registerIndex = 0; registerIndex < VP880_DEVTYPE_LEN; registerIndex++) {
			VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", regdata[registerIndex]));
		}
		*len = VP880_DEVTYPE_LEN;
	}

	if ((reg==VP880_DEV_MODE_RD) || (reg==VP880_DEV_MODE_WRT))
	{
		VpMpiCmdWrapper(deviceId, VP880_EC_CH1, reg, VP880_DEV_MODE_LEN, regdata);
		VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_DEV_MODE_RD           (0x%02X) ", VP880_DEV_MODE_RD));
		for (registerIndex = 0; registerIndex < VP880_DEV_MODE_LEN; registerIndex++) {
			VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", regdata[registerIndex]));
		}
		*len = VP880_DEV_MODE_LEN;
	}

	if ((reg==VP880_TEST_REG1_RD) || (reg==VP880_TEST_REG1_WRT))
	{
		VpMpiCmdWrapper(deviceId, VP880_EC_CH1, reg, VP880_TEST_REG1_LEN, regdata);
		VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_TEST_REG1_RD          (0x%02X) ", VP880_TEST_REG1_RD));
		for (registerIndex = 0; registerIndex < VP880_TEST_REG1_LEN; registerIndex++) {
			VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", regdata[registerIndex]));
		}
		*len = VP880_TEST_REG1_LEN;
	}

	if ((reg==VP880_TEST_REG2_RD) || (reg==VP880_TEST_REG2_WRT))
	{
		VpMpiCmdWrapper(deviceId, VP880_EC_CH1, reg, VP880_TEST_REG2_LEN, regdata);
		VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_TEST_REG2_RD          (0x%02X) ", VP880_TEST_REG2_RD));
		for (registerIndex = 0; registerIndex < VP880_TEST_REG2_LEN; registerIndex++) {
			VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", regdata[registerIndex]));
		}
		*len = VP880_TEST_REG2_LEN;
	}

	if ((reg==VP880_XR_CS_RD) || (reg==VP880_XR_CS_WRT))
	{
		VpMpiCmdWrapper(deviceId, VP880_EC_CH1, reg, VP880_XR_CS_LEN, regdata);
		VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_XR_CS_RD              (0x%02X) ", VP880_XR_CS_RD));
		for (registerIndex = 0; registerIndex < VP880_XR_CS_LEN; registerIndex++) {
			VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", regdata[registerIndex]));
		}
		*len = VP880_XR_CS_LEN;
	}

	if ((reg==VP880_DCR_RD) || (reg==VP880_DCR_WRT))
	{
		VpMpiCmdWrapper(deviceId, VP880_EC_CH1, reg, VP880_DCR_LEN, regdata);
		VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_DCR_RD                (0x%02X) ", VP880_DCR_RD));
		for (registerIndex = 0; registerIndex < VP880_DCR_LEN; registerIndex++) {
			VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", regdata[registerIndex]));
		}
		*len = VP880_DCR_LEN;
	}

	if ((reg==VP880_OP_MODE_RD) || (reg==VP880_OP_MODE_WRT))
	{
		VpMpiCmdWrapper(deviceId, VP880_EC_CH1, reg, VP880_OP_MODE_LEN, regdata);
		VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_OP_MODE_RD            (0x%02X) ", VP880_OP_MODE_RD));
		for (registerIndex = 0; registerIndex < VP880_OP_MODE_LEN; registerIndex++) {
			VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", regdata[registerIndex]));
		}
		*len = VP880_OP_MODE_LEN;
	}

	if ((reg==VP880_NO_UL_SIGREG_RD))
	{
		VpMpiCmdWrapper(deviceId, VP880_EC_CH1, reg, VP880_NO_UL_SIGREG_LEN, regdata);
		VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_NO_UL_SIGREG_RD       (0x%02X) ", VP880_NO_UL_SIGREG_RD));
		for (registerIndex = 0; registerIndex < VP880_NO_UL_SIGREG_LEN; registerIndex++) {
			VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", regdata[registerIndex]));
		}
		*len = VP880_NO_UL_SIGREG_LEN;
	}

	if ((reg==VP880_INT_MASK_RD) || (reg==VP880_INT_MASK_WRT))
	{
		VpMpiCmdWrapper(deviceId, VP880_EC_CH1, reg, VP880_INT_MASK_LEN, regdata);
		VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_INT_MASK_RD           (0x%02X) ", VP880_INT_MASK_RD));
		for (registerIndex = 0; registerIndex < VP880_INT_MASK_LEN; registerIndex++) {
			VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", regdata[registerIndex]));
		}
		*len = VP880_INT_MASK_LEN;
	}

	if ((reg==VP880_REV_INFO_RD))
	{
		VpMpiCmdWrapper(deviceId, VP880_EC_CH1, reg, VP880_REV_INFO_LEN, regdata);
		VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_REV_INFO_RD           (0x%02X) ", VP880_REV_INFO_RD));
		for (registerIndex = 0; registerIndex < VP880_REV_INFO_LEN; registerIndex++) {
			VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", regdata[registerIndex]));
		}
		*len = VP880_REV_INFO_LEN;
	}

	if ((reg==VP880_REGULATOR_PARAM_RD) || (reg==VP880_REGULATOR_PARAM_WRT))
	{
		VpMpiCmdWrapper(deviceId, VP880_EC_CH1, reg, VP880_REGULATOR_PARAM_LEN, regdata);
		VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_REGULATOR_PARAM_RD    (0x%02X) ", VP880_REGULATOR_PARAM_RD));
		for (registerIndex = 0; registerIndex < VP880_REGULATOR_PARAM_LEN; registerIndex++) {
			VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", regdata[registerIndex]));
		}
		*len = VP880_REGULATOR_PARAM_LEN;
	}

	if ((reg==VP880_REGULATOR_CTRL_RD) || (reg==VP880_REGULATOR_CTRL_WRT))
	{
		VpMpiCmdWrapper(deviceId, VP880_EC_CH1, reg, VP880_REGULATOR_CTRL_LEN, regdata);
		VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_REGULATOR_CTRL_RD     (0x%02X) ", VP880_REGULATOR_CTRL_RD));
		for (registerIndex = 0; registerIndex < VP880_REGULATOR_CTRL_LEN; registerIndex++) {
			VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", regdata[registerIndex]));
		}
		*len = VP880_REGULATOR_CTRL_LEN;
	}

	if ((reg==VP880_INT_SWREG_PARAM_RD) || (reg==VP880_INT_SWREG_PARAM_WRT))
	{
		VpMpiCmdWrapper(deviceId, VP880_EC_CH1, reg, VP880_INT_SWREG_PARAM_LEN, regdata);
		VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_INT_SWREG_PARAM_RD    (0x%02X) ", VP880_INT_SWREG_PARAM_RD));
		for (registerIndex = 0; registerIndex < VP880_INT_SWREG_PARAM_LEN; registerIndex++) {
			VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", regdata[registerIndex]));
		}
		*len = VP880_INT_SWREG_PARAM_LEN;
	}

		/* Line specific register */
		//for (channelId = 0; channelId < pDevObj->staticInfo.maxChannels; channelId++) {
	{
			uint8 ecVal = (pLineObj->channelId == 0) ? VP880_EC_CH1 : VP880_EC_CH2;

			VP_ERROR(VpDevCtxType, pDevCtx,("\nCHANNEL %d", pLineObj->channelId));

		if ((reg==VP880_VP_GAIN_RD) || (reg==VP880_VP_GAIN_WRT))
		{
			VpMpiCmdWrapper(deviceId, ecVal, reg, VP880_VP_GAIN_LEN, regdata);
			VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_VP_GAIN_RD        (0x%02X) ", VP880_VP_GAIN_RD));
			for (registerIndex = 0; registerIndex < VP880_VP_GAIN_LEN; registerIndex++) {
				VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", regdata[registerIndex]));
			}
		*len = VP880_VP_GAIN_LEN;
		}

		if ((reg==VP880_IODATA_REG_RD) || (reg==VP880_IODATA_REG_WRT))
		{
			VpMpiCmdWrapper(deviceId, ecVal, reg, VP880_IODATA_REG_LEN, regdata);
			VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_IODATA_REG_RD     (0x%02X) ", VP880_IODATA_REG_RD));
			for (registerIndex = 0; registerIndex < VP880_IODATA_REG_LEN; registerIndex++) {
				VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", regdata[registerIndex]));
			}
			*len = VP880_IODATA_REG_LEN;
		}

		if ((reg==VP880_IODIR_REG_RD) || (reg==VP880_IODIR_REG_WRT))
		{
			VpMpiCmdWrapper(deviceId, ecVal, reg, VP880_IODIR_REG_LEN, regdata);
			VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_IODIR_REG_RD      (0x%02X) ", VP880_IODIR_REG_RD));
			for (registerIndex = 0; registerIndex < VP880_IODIR_REG_LEN; registerIndex++) {
				VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", regdata[registerIndex]));
			}
			*len = VP880_IODIR_REG_LEN;
		}

		if ((reg==VP880_BAT_CALIBRATION_RD) || (reg==VP880_BAT_CALIBRATION_WRT))
		{
			VpMpiCmdWrapper(deviceId, ecVal, reg, VP880_BAT_CALIBRATION_LEN, regdata);
			VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_BAT_CALIBRATION_RD(0x%02X) ", VP880_BAT_CALIBRATION_RD));
			for (registerIndex = 0; registerIndex < VP880_BAT_CALIBRATION_LEN; registerIndex++) {
				VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", regdata[registerIndex]));
			}
			*len = VP880_BAT_CALIBRATION_LEN;
		}

		if ((reg==VP880_SYS_STATE_RD) || (reg==VP880_SYS_STATE_WRT))
		{
			VpMpiCmdWrapper(deviceId, ecVal, reg, VP880_SYS_STATE_LEN, regdata);
			VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_SYS_STATE_RD      (0x%02X) ", VP880_SYS_STATE_RD));
			for (registerIndex = 0; registerIndex < VP880_SYS_STATE_LEN; registerIndex++) {
				VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", regdata[registerIndex]));
			}
			*len = VP880_SYS_STATE_LEN;
		}

		if ((reg==VP880_SS_CONFIG_RD) || (reg==VP880_SS_CONFIG_WRT))
		{
			VpMpiCmdWrapper(deviceId, ecVal, reg, VP880_SS_CONFIG_LEN, regdata);
			VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_SS_CONFIG_RD      (0x%02X) ", VP880_SS_CONFIG_RD));
			for (registerIndex = 0; registerIndex < VP880_SS_CONFIG_LEN; registerIndex++) {
				VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", regdata[registerIndex]));
			}
			*len = VP880_SS_CONFIG_LEN;
		}

		if ((reg==VP880_CONV_CFG_RD) || (reg==VP880_CONV_CFG_WRT))
		{
			VpMpiCmdWrapper(deviceId, ecVal, reg, VP880_CONV_CFG_LEN, regdata);
			VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_CONV_CFG_RD       (0x%02X) ", VP880_CONV_CFG_RD));
			for (registerIndex = 0; registerIndex < VP880_CONV_CFG_LEN; registerIndex++) {
				VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", regdata[registerIndex]));
			}
			*len = VP880_CONV_CFG_LEN;
		}

		if ((reg==VP880_CODEC_REG_RD) || (reg==VP880_CODEC_REG_WRT))
		{
			VpMpiCmdWrapper(deviceId, ecVal, reg, VP880_CODEC_REG_LEN, regdata);
			VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_CODEC_REG_RD      (0x%02X) ", VP880_CODEC_REG_RD));
			for (registerIndex = 0; registerIndex < VP880_CODEC_REG_LEN; registerIndex++) {
				VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", regdata[registerIndex]));
			}
			*len = VP880_CODEC_REG_LEN;
		}

		if ((reg==VP880_OP_COND_RD) || (reg==VP880_OP_COND_WRT))
		{
			VpMpiCmdWrapper(deviceId, ecVal, reg, VP880_OP_COND_LEN, regdata);
			VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_OP_COND_RD        (0x%02X) ", VP880_OP_COND_RD));
			for (registerIndex = 0; registerIndex < VP880_OP_COND_LEN; registerIndex++) {
				VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", regdata[registerIndex]));
			}
			*len = VP880_OP_COND_LEN;
		}

		if ((reg==VP880_DISN_RD) || (reg==VP880_DISN_WRT))
		{
			VpMpiCmdWrapper(deviceId, ecVal, reg, VP880_DISN_LEN, regdata);
			VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_DISN_RD           (0x%02X) ", VP880_DISN_RD));
			for (registerIndex = 0; registerIndex < VP880_DISN_LEN; registerIndex++) {
				VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", regdata[registerIndex]));
			}
			*len = VP880_DISN_LEN;
		}

		if ((reg==VP880_GX_GAIN_RD) || (reg==VP880_GX_GAIN_WRT))
		{
			VpMpiCmdWrapper(deviceId, ecVal, reg, VP880_GX_GAIN_LEN, regdata);
			VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_GX_GAIN_RD        (0x%02X) ", VP880_GX_GAIN_RD));
			for (registerIndex = 0; registerIndex < VP880_GX_GAIN_LEN; registerIndex++) {
				VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", regdata[registerIndex]));
			}
			*len = VP880_GX_GAIN_LEN;
		}

		if ((reg==VP880_GR_GAIN_RD) || (reg==VP880_GR_GAIN_WRT))
		{
			VpMpiCmdWrapper(deviceId, ecVal, reg, VP880_GR_GAIN_LEN, regdata);
			VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_GR_GAIN_RD        (0x%02X) ", VP880_GR_GAIN_RD));
			for (registerIndex = 0; registerIndex < VP880_GR_GAIN_LEN; registerIndex++) {
				VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", regdata[registerIndex]));
			}
			*len = VP880_GR_GAIN_LEN;
		}

		if ((reg==VP880_B1_FILTER_RD) || (reg==VP880_B1_FILTER_WRT))
		{
			VpMpiCmdWrapper(deviceId, ecVal, reg, VP880_B1_FILTER_LEN, regdata);
			VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_B1_FILTER_RD      (0x%02X) ", VP880_B1_FILTER_RD));
			for (registerIndex = 0; registerIndex < VP880_B1_FILTER_LEN; registerIndex++) {
				VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", regdata[registerIndex]));
			}
			*len = VP880_B1_FILTER_LEN;
		}

		if ((reg==VP880_B2_FILTER_RD) || (reg==VP880_B2_FILTER_WRT))
		{
			VpMpiCmdWrapper(deviceId, ecVal, reg, VP880_B2_FILTER_LEN, regdata);
			VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_B2_FILTER_RD      (0x%02X) ", VP880_B2_FILTER_RD));
			for (registerIndex = 0; registerIndex < VP880_B2_FILTER_LEN; registerIndex++) {
				VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", regdata[registerIndex]));
			}
			*len = VP880_B2_FILTER_LEN;
		}

		if ((reg==VP880_X_FILTER_RD) || (reg==VP880_X_FILTER_WRT))
		{
			VpMpiCmdWrapper(deviceId, ecVal, reg, VP880_X_FILTER_LEN, regdata);
			VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_X_FILTER_RD       (0x%02X) ", VP880_X_FILTER_RD));
			for (registerIndex = 0; registerIndex < VP880_X_FILTER_LEN; registerIndex++) {
				VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", regdata[registerIndex]));
			}
			*len = VP880_X_FILTER_LEN;
		}

		if ((reg==VP880_R_FILTER_RD) || (reg==VP880_R_FILTER_WRT))
		{
			VpMpiCmdWrapper(deviceId, ecVal, reg, VP880_R_FILTER_LEN, regdata);
			VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_R_FILTER_RD       (0x%02X) ", VP880_R_FILTER_RD));
			for (registerIndex = 0; registerIndex < VP880_R_FILTER_LEN; registerIndex++) {
				VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", regdata[registerIndex]));
			}
			*len = VP880_R_FILTER_LEN;
		}

		if ((reg==VP880_Z1_FILTER_RD) || (reg==VP880_Z1_FILTER_WRT))
		{
			VpMpiCmdWrapper(deviceId, ecVal, reg, VP880_Z1_FILTER_LEN, regdata);
			VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_Z1_FILTER_RD      (0x%02X) ", VP880_Z1_FILTER_RD));
			for (registerIndex = 0; registerIndex < VP880_Z1_FILTER_LEN; registerIndex++) {
				VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", regdata[registerIndex]));
			}
			*len = VP880_Z1_FILTER_LEN;
		}

		if ((reg==VP880_Z2_FILTER_RD) || (reg==VP880_Z2_FILTER_WRT))
		{
			VpMpiCmdWrapper(deviceId, ecVal, reg, VP880_Z2_FILTER_LEN, regdata);
			VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_Z2_FILTER_RD      (0x%02X) ", VP880_Z2_FILTER_RD));
			for (registerIndex = 0; registerIndex < VP880_Z2_FILTER_LEN; registerIndex++) {
				VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", regdata[registerIndex]));
			}
			*len = VP880_Z2_FILTER_LEN;
		}

		if ((reg==VP880_TX_TS_RD) || (reg==VP880_TX_TS_WRT))
		{
			VpMpiCmdWrapper(deviceId, ecVal, reg, VP880_TX_TS_LEN, regdata);
			VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_TX_TS_RD          (0x%02X) ", VP880_TX_TS_RD));
			for (registerIndex = 0; registerIndex < VP880_TX_TS_LEN; registerIndex++) {
				VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", regdata[registerIndex]));
			}
			*len = VP880_TX_TS_LEN;
		}

		if ((reg==VP880_RX_TS_RD) || (reg==VP880_RX_TS_WRT))
		{
			VpMpiCmdWrapper(deviceId, ecVal, reg, VP880_RX_TS_LEN, regdata);
			VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_RX_TS_RD          (0x%02X) ", VP880_RX_TS_RD));
			for (registerIndex = 0; registerIndex < VP880_RX_TS_LEN; registerIndex++) {
				VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", regdata[registerIndex]));
			}
			*len = VP880_RX_TS_LEN;
		}

		if ((reg==VP880_DC_FEED_RD) || (reg==VP880_DC_FEED_WRT))
		{
			VpMpiCmdWrapper(deviceId, ecVal, reg, VP880_DC_FEED_LEN, regdata);
			VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_DC_FEED_RD        (0x%02X) ", VP880_DC_FEED_RD));
			for (registerIndex = 0; registerIndex < VP880_DC_FEED_LEN; registerIndex++) {
				VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", regdata[registerIndex]));
			}
			*len = VP880_DC_FEED_LEN;
		}

		if ((reg==VP880_LOOP_SUP_RD) || (reg==VP880_LOOP_SUP_WRT))
		{
			VpMpiCmdWrapper(deviceId, ecVal, reg, VP880_LOOP_SUP_LEN, regdata);
			VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_LOOP_SUP_RD       (0x%02X) ", VP880_LOOP_SUP_RD));
			for (registerIndex = 0; registerIndex < VP880_LOOP_SUP_LEN; registerIndex++) {
				VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", regdata[registerIndex]));
			}
			*len = VP880_LOOP_SUP_LEN;
		}

		if ((reg==VP880_METERING_PARAM_RD) || (reg==VP880_METERING_PARAM_WRT))
		{
			VpMpiCmdWrapper(deviceId, ecVal, reg, VP880_METERING_PARAM_LEN, regdata);
			VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_METERING_PARAM_RD (0x%02X) ", VP880_METERING_PARAM_RD));
			for (registerIndex = 0; registerIndex < VP880_METERING_PARAM_LEN; registerIndex++) {
				VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", regdata[registerIndex]));
			}
			*len = VP880_METERING_PARAM_LEN;
		}

		if ((reg==VP880_SIGA_PARAMS_RD) || (reg==VP880_SIGA_PARAMS_WRT))
		{
			VpMpiCmdWrapper(deviceId, ecVal, reg, VP880_SIGA_PARAMS_LEN, regdata);
			VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_SIGA_PARAMS_RD    (0x%02X) ", VP880_SIGA_PARAMS_RD));
			for (registerIndex = 0; registerIndex < VP880_SIGA_PARAMS_LEN; registerIndex++) {
				VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", regdata[registerIndex]));
			}
			*len = VP880_SIGA_PARAMS_LEN;
		}

		if ((reg==VP880_SIGCD_PARAMS_RD) || (reg==VP880_SIGCD_PARAMS_WRT))
		{
			VpMpiCmdWrapper(deviceId, ecVal, reg, VP880_SIGCD_PARAMS_LEN, regdata);
			VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_SIGCD_PARAMS_RD   (0x%02X) ", VP880_SIGCD_PARAMS_RD));
			for (registerIndex = 0; registerIndex < VP880_SIGCD_PARAMS_LEN; registerIndex++) {
				VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", regdata[registerIndex]));
			}
			*len = VP880_SIGCD_PARAMS_LEN;
		}

		if ((reg==VP880_CADENCE_TIMER_RD) || (reg==VP880_CADENCE_TIMER_WRT))
		{
			VpMpiCmdWrapper(deviceId, ecVal, reg, VP880_CADENCE_TIMER_LEN, regdata);
			VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_CADENCE_TIMER_RD  (0x%02X) ", VP880_CADENCE_TIMER_RD));
			for (registerIndex = 0; registerIndex < VP880_CADENCE_TIMER_LEN; registerIndex++) {
				VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", regdata[registerIndex]));
			}
			*len = VP880_CADENCE_TIMER_LEN;
		}

		if ((reg==VP880_CID_PARAM_RD) || (reg==VP880_CID_PARAM_WRT))
		{
			VpMpiCmdWrapper(deviceId, ecVal, reg, VP880_CID_PARAM_LEN, regdata);
			VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_CID_PARAM_RD      (0x%02X) ", VP880_CID_PARAM_RD));
			for (registerIndex = 0; registerIndex < VP880_CID_PARAM_LEN; registerIndex++) {
				VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", regdata[registerIndex]));
			}
			*len = VP880_CID_PARAM_LEN;
		}

		if ((reg==VP880_ICR1_RD) || (reg==VP880_ICR1_WRT))
		{
			VpMpiCmdWrapper(deviceId, ecVal, reg, VP880_ICR1_LEN, regdata);
			VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_ICR1_RD           (0x%02X) ", VP880_ICR1_RD));
			for (registerIndex = 0; registerIndex < VP880_ICR1_LEN; registerIndex++) {
				VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", regdata[registerIndex]));
			}
			*len = VP880_ICR1_LEN;
		}

		if ((reg==VP880_ICR2_RD) || (reg==VP880_ICR2_WRT))
		{
			VpMpiCmdWrapper(deviceId, ecVal, reg, VP880_ICR2_LEN, regdata);
			VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_ICR2_RD           (0x%02X) ", VP880_ICR2_RD));
			for (registerIndex = 0; registerIndex < VP880_ICR2_LEN; registerIndex++) {
				VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", regdata[registerIndex]));
			}
			*len = VP880_ICR2_LEN;
		}

		if ((reg==VP880_ICR3_RD) || (reg==VP880_ICR3_WRT))
		{
			VpMpiCmdWrapper(deviceId, ecVal, reg, VP880_ICR3_LEN, regdata);
			VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_ICR3_RD           (0x%02X) ", VP880_ICR3_RD));
			for (registerIndex = 0; registerIndex < VP880_ICR3_LEN; registerIndex++) {
				VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", regdata[registerIndex]));
			}
			*len = VP880_ICR3_LEN;
		}

		if ((reg==VP880_ICR4_RD) || (reg==VP880_ICR4_WRT))
		{
			VpMpiCmdWrapper(deviceId, ecVal, reg, VP880_ICR4_LEN, regdata);
			VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_ICR4_RD           (0x%02X) ", VP880_ICR4_RD));
			for (registerIndex = 0; registerIndex < VP880_ICR4_LEN; registerIndex++) {
				VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", regdata[registerIndex]));
			}
			*len = VP880_ICR4_LEN;
		}

		if ((reg==VP880_ICR5_RD) || (reg==VP880_ICR5_WRT))
		{
			VpMpiCmdWrapper(deviceId, ecVal, reg, VP880_ICR5_LEN, regdata);
			VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_ICR5_RD           (0x%02X) ", VP880_ICR5_RD));
			for (registerIndex = 0; registerIndex < VP880_ICR5_LEN; registerIndex++) {
				VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", regdata[registerIndex]));
			}
			*len = VP880_ICR5_LEN;
		}

		if ((reg==VP880_DC_CAL_REG_RD) || (reg==VP880_DC_CAL_REG_WRT))
		{
			VpMpiCmdWrapper(deviceId, ecVal, reg, VP880_DC_CAL_REG_LEN, regdata);
			VP_ERROR(VpDevCtxType, pDevCtx,("\nVP880_DC_CAL_REG_RD     (0x%02X) ", VP880_DC_CAL_REG_RD));
			for (registerIndex = 0; registerIndex < VP880_DC_CAL_REG_LEN; registerIndex++) {
				VP_ERROR(VpDevCtxType, pDevCtx,("0x%02X ", regdata[registerIndex]));
			}
			*len = VP880_DC_CAL_REG_LEN;
		}
	}

    return VP_STATUS_SUCCESS;
}
#endif

