/** \file vp890_control.c
 * vp890_control.c
 *
 *  This file contains the implementation of the VP-API 890 Series
 *  Control Functions.
 *
 * Copyright (c) 2010, Zarlink Semiconductor, Inc.
 *
 * $Revision: 7491 $
 * $LastChangedDate: 2010-07-14 10:08:08 -0500 (Wed, 14 Jul 2010) $
 */

/* INCLUDES */
#include    "vp_api.h"

#if defined (VP_CC_890_SERIES)  /* Compile only if required */

#include    "vp_api_int.h"
#include    "vp890_api_int.h"

/* =================================
    Prototypes for Static Functions
   ================================= */
static void
Vp890RestartComplete(
    VpDevCtxType *pDevCtx);

static void
Vp890ProcessFxsLine(
    Vp890DeviceObjectType   *pDevObj,
    VpLineCtxType           *pLineCtx);

static void
Vp890ProcessFxoLine(
    Vp890DeviceObjectType   *pDevObj,
    VpLineCtxType           *pLineCtx);

void
Vp890AmpFreqRingDetect(
    VpLineCtxType *pLineCtx,
    VpCslacLineCondType tempAmpSt);

static void
Vp890ServiceInterrupts(
    VpDevCtxType            *pDevCtx);

static void
Vp890ServiceFxsInterrupts(
    VpLineCtxType           *pLineCtx);

static void
Vp890ServiceDevTimers(
    VpDevCtxType            *pDevCtx);

static void
Vp890ServiceFxsTimers(
    VpDevCtxType            *pDevCtx,
    VpLineCtxType           *pLineCtx,
    Vp890LineObjectType     *pLineObj,
    VpDeviceIdType          deviceId,
    uint8                   ecVal);

static void
Vp890ServiceFxoTimers(
    VpDevCtxType            *pDevCtx,
    VpLineCtxType           *pLineCtx,
    Vp890LineObjectType     *pLineObj,
    VpDeviceIdType          deviceId,
    uint8                   ecVal);

static void
Vp890FxoLoopCurrentMonitor(
    VpLineCtxType           *pLineCtx);

static void
Vp890FxoLowVoltageMonitor(
    VpLineCtxType           *pLineCtx);

static void
MakeLowVoltageCorrections(
    Vp890LineObjectType     *pLineObj,
    uint8                   *pIntReg);

static bool
Vp890FindSoftwareInterrupts(
    VpDevCtxType            *pDevCtx);

static bool
Vp890SetStateRinging(
    VpLineCtxType           *pLineCtx,
    VpLineStateType         state);

static bool
Vp890IsSupportedFxoState(
    VpLineStateType state);

static bool
Vp890IsSupportedFxsState(
    VpLineStateType state);

static void
Vp890GroundStartProc(
    bool gsMode,
    VpLineCtxType *pLineCtx,
    uint8 currentLineState,
    uint8 userByte);

static void
Vp890RunLPDisc(
    VpLineCtxType *pLineCtx,
    bool discMode,
    uint8 nextSlicByte);

static uint16
Vp890SetDiscTimers(
    Vp890DeviceObjectType *pDevObj);

static void
Vp890SetLP(
    bool lpMode,
    VpLineCtxType *pLineCtx);

static void
Vp890WriteLPExitRegisters(
    VpLineCtxType *pLineOtx,
    VpDeviceIdType deviceId,
    uint8 ecVal,
    uint8 *lineState);

static void
Vp890WriteLPEnterRegisters(
    VpLineCtxType *pLineOtx,
    VpDeviceIdType deviceId,
    uint8 ecVal,
    uint8 *lineState);

static bool
Vp890IsChnlUndrTst(
    Vp890DeviceObjectType *pDevObj,
    uint8 channelId);

static uint8
LineStateMap(
    VpLineStateType         state);

static void
Vp890ApplyInternalTestTerm(
    VpLineCtxType *pLineCtx);

static void
Vp890RemoveInternalTestTerm(
    VpLineCtxType *pLineCtx);

static void
CidCorrectionLookup(
    uint16                  voltage,
    uint16                  *gainFactor,
    uint8                   *dtgVal);

static VpStatusType
SetDeviceOption(
    VpDevCtxType            *pDevCtx,
    VpDeviceIdType          deviceId,
    VpOptionIdType          option,
    void                    *value);

static VpStatusType
SetLineOption(
    VpLineCtxType           *pLineCtx,
    VpDeviceIdType          deviceId,
    VpOptionIdType          option,
    void                    *value);

static void
SetOptionEventMask(
    Vp890DeviceObjectType   *pDevObj,
    Vp890LineObjectType     *pLineObj,
    VpDeviceIdType          deviceId,
    uint8                   ecVal,
    void                    *value);

/* Function called by SetOptionInternal for Event Masking only */
static void
MaskNonSupportedEvents(
    VpOptionEventMaskType   *pLineEventsMask,
    VpOptionEventMaskType   *pDevEventsMask);

/* Function called by SetOptionInternal to set tx and rx timeslot */
static VpStatusType
SetTimeSlot(
    VpLineCtxType           *pLineCtx,
    uint8                   txSlot,
    uint8                   rxSlot);

static void
PllRecovery(
    VpLineCtxType           *pLineCtx);

/**< Profile index for Generator A/B and C/D starting points (std tone) */
typedef enum
{
    VP890_SIGGEN_AB_START = 8,
    VP890_SIGGEN_CD_START = 16,
    VP890_SIGGEN_ENUM_SIZE = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} vp890_toneProfileParams;

/*******************************************************************************
 * Vp890ApiTick()
 *  This function should be called on a periodic basis or attached to an
 * interrupt.
 *
 * Arguments:
 *
 * Preconditions:
 *  The device must first be initialized.
 *
 * Postconditions:
 *  The value passed (by pointer) is set to TRUE if there is an updated event.
 * The user should call the GetEventStatus function to determine the cause of
 * the event (TRUE value set).  This function always returns the success code.
 ******************************************************************************/
VpStatusType
Vp890ApiTick(
    VpDevCtxType        *pDevCtx,
    bool                *pEventStatus)
{
    VpLineCtxType           *pLineCtx;
    Vp890DeviceObjectType   *pDevObj    = pDevCtx->pDevObj;
    Vp890LineObjectType     *pLineObj;
    VpDeviceIdType          deviceId    = pDevObj->deviceId;

    uint8                   channelId;
    uint8                   maxChan     = pDevObj->staticInfo.maxChannels;
    uint8                   ecVal;
    bool                    tempClkFault, tempBat1Fault, lineInTest, intServiced;

#ifdef CSLAC_SEQ_EN
    bool isSeqRunning = FALSE;
#endif

    uint16                  timeStampPre, tickAdder;

    *pEventStatus = FALSE;

    VpSysEnterCritical(deviceId, VP_CODE_CRITICAL_SEC);

    /*
     * Can't allow tick functions to proceed until Init Device function has
     * been called. Otherwise, "tickrate" is unknown and initally 0.
     */
    if (pDevObj->devProfileData.tickRate == 0) {
        VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
        return VP_STATUS_DEV_NOT_INITIALIZED;
    }

    /*
     * The timestamp is in 0.5mS increments, but the device tickrate is
     * something else. So increment by the scaled amount and detect rollover
     * by finding if the previous value is greater than the new value.
     */
    timeStampPre = pDevObj->timeStamp;
    tickAdder = pDevObj->devProfileData.tickRate / VP_CSLAC_TICKSTEP_0_5MS;
    pDevObj->timeStamp+=tickAdder;

    if (timeStampPre > pDevObj->timeStamp) {
        pDevObj->deviceEvents.signaling |= VP_DEV_EVID_TS_ROLLOVER;
    }

#if defined (VP890_INTERRUPT_LEVTRIG_MODE)
    VpSysEnableInt(deviceId);
#endif

    /*
     * Figure out if either channel is using WB mode and adjust the device
     * ec value as needed. The device object ec value will be used next to
     * go back and adjust both valid line object ec values. This "dual loop"
     * ckeck is done in case the second channel is in WB but not the first.
     */
    pDevObj->ecVal &= ~VP890_WIDEBAND_MODE;
    for(channelId=0; channelId < maxChan; channelId++ ) {
        pLineCtx = pDevCtx->pLineCtx[channelId];
        if (pLineCtx == VP_NULL) {
            continue;
        }
        pLineObj = pLineCtx->pLineObj;

        if (pLineObj->codec == VP_OPTION_WIDEBAND) {
            pDevObj->ecVal |= VP890_WIDEBAND_MODE;
        }
    }
    ecVal = pDevObj->ecVal;

    /*
     * Adjust the line object ec values based on previous determination
     * if Wideband mode is being used at the device level.
     */
    for(channelId=0; channelId < maxChan; channelId++ ) {
        pLineCtx = pDevCtx->pLineCtx[channelId];
        if (pLineCtx == VP_NULL) {
            continue;
        }
        pLineObj = pLineCtx->pLineObj;
        pLineObj->ecVal &= ~VP890_WIDEBAND_MODE;
        pLineObj->ecVal |= pDevObj->ecVal;

        ecVal |= pLineObj->ecVal;
    }

    if (!(pDevObj->stateInt & VP890_IS_FXO_ONLY)) {
        Vp890LowPowerMode(pDevCtx);
    }

    /* Vp890Service Device Timers */
    Vp890ServiceDevTimers(pDevCtx);

    /*
     * Preclear the lineInTest flag that is used to determine if the TX Buffer
     * needs to be read. It is always read if any line is in test.
     */
    lineInTest = FALSE;

    /* Vp890Service Line Timers and check to see if the sequencer needs to run */
    for(channelId=0; channelId < maxChan; channelId++ ) {
        pLineCtx = pDevCtx->pLineCtx[channelId];
        if (pLineCtx == VP_NULL) {
            continue;
        }
        pLineObj = pLineCtx->pLineObj;
        /* call the appropriate service timer */
        if ((pLineObj->status & VP890_IS_FXO)) {
            Vp890ServiceFxoTimers(pDevCtx, pLineCtx, pLineObj, deviceId, pLineObj->ecVal);
        } else {
            Vp890ServiceFxsTimers(pDevCtx, pLineCtx, pLineObj, deviceId, pLineObj->ecVal);

#ifdef VP890_INCLUDE_TESTLINE_CODE
            lineInTest = ( (Vp890IsChnlUndrTst(pDevObj, channelId) == TRUE) ||
                (pDevObj->currentTest.rdLoopTest == TRUE)) ? TRUE : lineInTest;
#endif
            if (pDevObj->stateInt & VP890_CAL_RELOAD_REQ) {
                Vp890UpdateCalValue(pLineCtx);
            }
        }
#ifdef CSLAC_SEQ_EN
        /* Evaluate if Cadencing is required */
        if ((pLineObj->cadence.status & VP_CADENCE_STATUS_ACTIVE) == VP_CADENCE_STATUS_ACTIVE) {
            VP_SEQUENCER(VpLineCtxType, pLineCtx, ("Line %d is Active", channelId));
            isSeqRunning = TRUE;
        }
#endif
    }

    pDevObj->stateInt &= ~VP890_CAL_RELOAD_REQ;


    /* Reset event pointers pointers */
    pDevObj->dynamicInfo.lastChan = 0;

#ifdef CSLAC_SEQ_EN
    if (isSeqRunning == TRUE) {
        VpServiceSeq(pDevCtx);
    }
#endif

    /*
     * Test the interrupt to see if there is a pending interrupt.  If there is,
     * read the interrupt registers (if running in an interrupt driven mode).
     * If running in polled mode, automatically read the interrupt/status
     * registers.
     */

#if defined (VP890_EFFICIENT_POLLED_MODE)
    /* Poll the device PIO-INT line */
    pDevObj->status.state |= (VpSysTestInt(deviceId) ? VP_DEV_PENDING_INT : 0x00);
#elif defined (VP890_SIMPLE_POLLED_MODE)
    pDevObj->status.state |= VP_DEV_PENDING_INT;
#endif

    /* Limit the number of interrupts serviced during one tick */
    pDevObj->status.numIntServiced = 2;
    intServiced = FALSE;

    /*
     * Read this buffer once per tick IF there is an active interrupt, or if
     * running line test or being forced to.
     */
    if ((pDevObj->status.state & VP_DEV_PENDING_INT)
     || (pDevObj->status.state & VP_DEV_FORCE_SIG_READ)
     || (lineInTest == TRUE)) {
        pDevObj->status.state &= ~VP_DEV_FORCE_SIG_READ;
        /* Collect data from the device test data buffer */
        VpMpiCmdWrapper(deviceId, ecVal, VP890_TEST_DATA_RD, VP890_TEST_DATA_LEN,
            pDevObj->testDataBuffer);
        pDevObj->status.state |= VP_DEV_TEST_BUFFER_READ;
    } else {
        pDevObj->status.state &= ~VP_DEV_TEST_BUFFER_READ;
    }

    /* Service all pending interrupts (up to 2) */
    while ((pDevObj->status.state & VP_DEV_PENDING_INT) &&
           (pDevObj->status.numIntServiced > 0)) {

        VpMpiCmdWrapper(deviceId, ecVal, VP890_UL_SIGREG_RD,
            VP890_UL_SIGREG_LEN, pDevObj->intReg);

        /*
         * Compare it with what we already know.  If different, generate
         * events and update the line status bits
         */
        intServiced = TRUE;
        Vp890ServiceInterrupts(pDevCtx);

        /*
         * If level triggered, the interrupt may have been disabled (to prevent
         * a flood of interrupts), so reenable it.
         */
    #if defined (VP890_INTERRUPT_LEVTRIG_MODE)
        VpSysEnableInt(deviceId);
    #endif

        /* Clear the current interrupt indication */
        pDevObj->status.state &= ~(VP_DEV_PENDING_INT);
        pDevObj->status.numIntServiced--;

        /*
         * If operating in Efficient Polled Mode, check to see if the interrupt
         * line is still indicating an active interrupt. If in simple polled mode,
         * repeat the loop and service interrupts (if anything is changed).
         */
    #if defined (VP890_EFFICIENT_POLLED_MODE)
        /* Poll the PIO-INT line */
        pDevObj->status.state |=
            (VpSysTestInt(deviceId) ? VP_DEV_PENDING_INT : 0x00);
    #elif defined (VP890_SIMPLE_POLLED_MODE)
        pDevObj->status.state |= VP_DEV_PENDING_INT;
    #endif
    }/* End while Interrupts*/

    /* Make sure Vp890ServiceInterrupts() is called at least once per tick to
     * keep the API line status up to date */
    if (intServiced == FALSE) {
        Vp890ServiceInterrupts(pDevCtx);
    }

    /* Update the dial pulse handler for lines that are set for pulse decode,
     * and process events for FXO lines */
    for (channelId = 0; channelId < maxChan; channelId++) {
        pLineCtx = pDevCtx->pLineCtx[channelId];
        if (pLineCtx != VP_NULL) {
            pLineObj = pLineCtx->pLineObj;
            if (!(pLineObj->status & VP890_IS_FXO)) {
                Vp890ProcessFxsLine(pDevObj, pLineCtx);
            } else {
                Vp890ProcessFxoLine(pDevObj, pLineCtx);
            }
        }
    }
    /*******************************************************
     *      HANDLE Clock Fail and Battery Fault Events     *
     *******************************************************/
    /* Get the current status of the clock fault bit */
    if (!(pDevObj->devTimer[VP_DEV_TIMER_WB_MODE_CHANGE] & VP_ACTIVATE_TIMER)) {
        tempClkFault = (pDevObj->intReg[0] & VP890_CFAIL_MASK) ? TRUE : FALSE;
        /*
         * Compare it with what we already know.  If different, generate event
         */
        if(tempClkFault ^ pDevObj->dynamicInfo.clkFault) {
            if (!(pDevObj->stateInt & VP890_FORCE_FREE_RUN)) {
                if (tempClkFault) {
                    /* Entering clock fault, possibly a system restart. */
                    Vp890FreeRun(pDevCtx, VP_FREE_RUN_START);

                    /*
                     * Clear the flag used to indicate that Vp890FreeRun() was
                     * called by the application -- because it wasn't.
                     */
                    pDevObj->stateInt &= ~VP890_FORCE_FREE_RUN;
                } else {
                    /*
                     * Exiting clock fault (note: this function does not affect
                     * VP890_FORCE_FREE_RUN flag).
                     */
                    Vp890RestartComplete(pDevCtx);
                }
            }

            pDevObj->dynamicInfo.clkFault = tempClkFault;
            pDevObj->deviceEvents.faults |= VP_DEV_EVID_CLK_FLT;
            pDevObj->eventHandle = pDevObj->timeStamp;
        }
    }

    /* Get the current status of the battery fault bit */
    tempBat1Fault = (pDevObj->intReg[0] & VP890_OCALMY_MASK) ? TRUE : FALSE;
    /*
     * Compare it with what we already know.  If different, generate event
     */
    if(tempBat1Fault ^ pDevObj->dynamicInfo.bat1Fault) {
        pDevObj->dynamicInfo.bat1Fault = tempBat1Fault;
        pDevObj->deviceEvents.faults |= VP_DEV_EVID_BAT_FLT;
        pDevObj->eventHandle = pDevObj->timeStamp;
    }

    /* Collect all event activity and report to the calling function */
    if (Vp890FindSoftwareInterrupts(pDevCtx)) {
        *pEventStatus = TRUE;
    }

    VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
    return VP_STATUS_SUCCESS;
} /* Vp890ApiTick() */

/**
 * Vp890LowPowerMode()
 *  This function is called when the device should be updated for Low Power
 * mode. It determines if the device can be put into low power mode and does
 * (if it can), sets a flag in the device object, and sets the device timer
 * for hook debounce.
 *
 * Preconditions:
 *
 * Postconditions:
 */
void
Vp890LowPowerMode(
    VpDevCtxType *pDevCtx)
{
    Vp890DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpDeviceIdType deviceId = pDevObj->deviceId;

    VpLineCtxType *pLineCtx;
    bool isValidCtx[VP890_MAX_NUM_CHANNELS] = {FALSE, FALSE};
    Vp890LineObjectType *pLineObj;
    bool lowPower;

    /*
     * FXS exists only on first channel or not at all (FXO only device). But for
     * FXO only device, this function is not called.
     */
    uint8 maxChannels = 1;
    uint8 channelId;
    uint8 ecVal;

#ifdef VP890_INCLUDE_TESTLINE_CODE
    /* We don't want to interact with the line in this state */
    if (pDevObj->currentTest.rdLoopTest == TRUE) {
        return;
    }
#endif

    /* Don't do anything if device is in calibration */
    if (pDevObj->status.state & VP_DEV_IN_CAL) {
        return;
    }

    for (channelId = 0; channelId < maxChannels; channelId++) {
        if (pDevCtx->pLineCtx[channelId] != VP_NULL) {
            pLineCtx = pDevCtx->pLineCtx[channelId];
            pLineObj = pLineCtx->pLineObj;

            /* Don't enter or exit LPM until all lines have been initialized. */
            if (!(pLineObj->status & VP890_INIT_COMPLETE)) {
                VP_LINE_STATE(VpLineCtxType, pLineCtx, ("Init Line Not Complete"));
                return;
            }
        }
    }

    if (pDevObj->stateInt & VP890_LINE0_LP) {
        lowPower = TRUE;
    } else {
        lowPower = FALSE;
    }

    /*
     * Determine which lines are valid in case we have to adjust their line
     * states. Consider "valid" only those lines that are FXS Low Power type.
     */
    for (channelId = 0; channelId < maxChannels; channelId++) {
        if (pDevCtx->pLineCtx[channelId] != VP_NULL) {
            pLineCtx = pDevCtx->pLineCtx[channelId];
            pLineObj = pLineCtx->pLineObj;
            if ((!(pLineObj->status & VP890_IS_FXO))
               && ((pLineObj->termType == VP_TERM_FXS_LOW_PWR) ||
                   (pLineObj->termType == VP_TERM_FXS_ISOLATE_LP) ||
                   (pLineObj->termType == VP_TERM_FXS_SPLITTER_LP))) {
                isValidCtx[channelId] = TRUE;
                if ((Vp890IsChnlUndrTst(pDevObj, channelId) == TRUE) ||
                    (pLineObj->status & VP890_LINE_LEAK)) {
                    lowPower = FALSE;
                }

                if ((pLineObj->lineTimers.timers.timer[VP_LINE_POLREV_DEBOUNCE]
                    & VP_ACTIVATE_TIMER) && (lowPower == TRUE)){
                    VP_LINE_STATE(VpLineCtxType, pLineCtx, ("Delay LP Enter for PolRev at time %d",
                        pDevObj->timeStamp));
                    return;
                }
            }
        }
    }

    if (lowPower == FALSE) {
        /*
         * Take the device out of low power mode and set channels to correct
         * states. Do not affect device or channels if change has already
         * been made.
         */

        for (channelId = 0; channelId < maxChannels; channelId++) {
            if (isValidCtx[channelId] == TRUE) {
                pLineCtx = pDevCtx->pLineCtx[channelId];
                pLineObj = pLineCtx->pLineObj;
                ecVal = pLineObj->ecVal;

                if (pLineObj->status & VP890_LOW_POWER_EN) {
                    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp890LowPowerMode+"));

                    VP_LINE_STATE(VpLineCtxType, pLineCtx, ("Exit LPM for Line %d",
                        channelId));

                    VpMpiCmdWrapper(deviceId, ecVal, VP890_REGULATOR_PARAM_WRT,
                        VP890_REGULATOR_PARAM_LEN, pDevObj->devProfileData.swParams);

                    Vp890SetLP(FALSE, pLineCtx);

                    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp890LowPowerMode-"));
                }
            }
        }
    } else {
        /*
         * We should be in low power mode because both lines can be put into
         * low power mode. Don't need to call Set Line State in this case for
         * each line because there are a limited number of API-II states that
         * can allow Low Power, and all required the SLIC state to be set to
         * Disconnect.
         */
        for (channelId = 0; channelId < maxChannels; channelId++) {
            if (isValidCtx[channelId] == TRUE) {
                pLineCtx = pDevCtx->pLineCtx[channelId];
                pLineObj = pLineCtx->pLineObj;
                ecVal = pLineObj->ecVal;

                if (!(pLineObj->status & VP890_LOW_POWER_EN)) {
                    uint8 swParams[VP890_REGULATOR_PARAM_LEN];

                    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp890LowPowerMode+"));

                    VP_LINE_STATE(VpLineCtxType, pLineCtx, ("Enter LPM for Line %d", channelId));

                    VpMpiCmdWrapper(deviceId, ecVal, VP890_REGULATOR_PARAM_RD,
                        VP890_REGULATOR_PARAM_LEN, swParams);
                    swParams[VP890_FLOOR_VOLTAGE_BYTE] &= ~VP890_FLOOR_VOLTAGE_MASK;
                    swParams[VP890_FLOOR_VOLTAGE_BYTE] |= 0x0D;   /* 70V */

                    VpMpiCmdWrapper(deviceId, ecVal, VP890_REGULATOR_PARAM_WRT,
                        VP890_REGULATOR_PARAM_LEN, swParams);

                    Vp890SetLP(TRUE, pLineCtx);
                    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp890LowPowerMode-"));
                }
            }
        }
    }
}

/**
 * Vp890SetLP()
 *  This function modifies the line/device to enter/exit LPM.
 *
 * Preconditions:
 *
 * Postconditions:
 */
void
Vp890SetLP(
    bool lpMode,
    VpLineCtxType *pLineCtx)
{
    Vp890LineObjectType *pLineObj = pLineCtx->pLineObj;

    uint8 ecVal = pLineObj->ecVal;

    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp890DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpDeviceIdType deviceId = pDevObj->deviceId;

    uint8 lineState[VP890_SYS_STATE_LEN];

    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp890SetLP+"));

    /* Need timer here to allow switcher to stabilize whether entering or
     * exiting LPM.
     */
    pDevObj->devTimer[VP_DEV_TIMER_LP_CHANGE] =
        (MS_TO_TICKRATE(VP890_PWR_SWITCH_DEBOUNCE, pDevObj->devProfileData.tickRate))
        | VP_ACTIVATE_TIMER;

    if (lpMode == FALSE) {
        VP_LINE_STATE(VpLineCtxType, pLineCtx, ("Taking Channel %d out of Low Power Mode at time %d User State %d",
            pLineObj->channelId, pDevObj->timeStamp, pLineObj->lineState.usrCurrent));

        pLineObj->status &= ~VP890_LOW_POWER_EN;

        if (pLineObj->lineState.usrCurrent != VP_LINE_DISCONNECT) {
            bool internalApiOperation = FALSE;

            Vp890SetLPRegisters(pDevObj, pLineObj, FALSE);

#ifdef CSLAC_SEQ_EN
            if ((pLineObj->cadence.status & (VP_CADENCE_STATUS_ACTIVE | VP_CADENCE_STATUS_SENDSIG)) ==
                (VP_CADENCE_STATUS_ACTIVE | VP_CADENCE_STATUS_SENDSIG)) {
                internalApiOperation = TRUE;
            }
#endif
            if ((pLineObj->lineState.usrCurrent == VP_LINE_STANDBY)
             && (internalApiOperation == FALSE)) {
                lineState[0] = VP890_SS_ACTIVE;
                Vp890WriteLPExitRegisters(pLineCtx, deviceId, ecVal,
                    lineState);
                pLineObj->nextSlicValue = VP890_SS_IDLE;
            } else {
                Vp890WriteLPExitRegisters(pLineCtx, deviceId, ecVal,
                    VP_NULL);
            }
        }

        /*
         * Disable LPM Exit sequence. When the Tracker Disable Time expires, it
         * will set the line to Disconnect and Disable the Tracker (ICR2).
         * Obviously, we no longer want to do this.
         */
        pLineObj->lineTimers.timers.timer[VP_LINE_TRACKER_DISABLE]
            &= ~VP_ACTIVATE_TIMER;
    } else {
        VP_LINE_STATE(VpLineCtxType, pLineCtx, ("Putting Channel %d in Low Power Mode at time %d",
            pLineObj->channelId, pDevObj->timeStamp));

        pLineObj->status |= VP890_LOW_POWER_EN;

        if (pLineObj->lineState.usrCurrent != VP_LINE_DISCONNECT) {
            /* Set timer to wait before making final changes. When this timer
             * expires, the following sequence is run:
             *
             *  - Set SLIC state to Disconnect
             *  - Write the following:
             *      icr2[VP890_ICR2_VOC_DAC_INDEX] |= VP890_ICR2_ILA_DAC;
             *      icr2[VP890_ICR2_VOC_DAC_INDEX] &= ~VP890_ICR2_VOC_DAC_SENSE;
             *      icr2[VP890_ICR2_VOC_DAC_INDEX+1] &= ~VP890_ICR2_ILA_DAC;
             */
            pLineObj->lineTimers.timers.timer[VP_LINE_TRACKER_DISABLE] =
                (MS_TO_TICKRATE(VP890_TRACKER_DISABLE_TIME,
                    pDevObj->devProfileData.tickRate)) | VP_ACTIVATE_TIMER;

            /*
             * We are entering LPM into VP_LINE_STANDBY. So the required ICR
             * values are not yet set in the line object (as they would be if
             * entering from VP_LINE_DISCONNECT).
             */
            lineState[0] = VP890_SS_DISCONNECT;

            Vp890SetLPRegisters(pDevObj, pLineObj, TRUE);
        } else {
            /*-----------------9/3/2009 2:53PM------------------
             * JEF: When is this code ever called?
             * --------------------------------------------------*/
            uint16 dischargeTime = Vp890SetDiscTimers(pDevObj);

            /*
             * We are entering LPM into VP_LINE_DISCONNECT. So the required ICR
             * values have been set in the line object, just need to force the
             * required sequence (Active w/polarity, then Disconnect).
             */
            lineState[0] = VP890_SS_ACTIVE;

            /* Set Discharge Time based on Supply Configuration. */
            if (dischargeTime < VP890_TRACKER_DISABLE_TIME) {
                dischargeTime = VP890_TRACKER_DISABLE_TIME;
            }

            /* Set timer to wait before making final changes. When this timer
             * expires, the following sequence is run:
             *
             *  - Set SLIC state to Disconnect
             *  - Write the following:
             *      icr2[VP890_ICR2_VOC_DAC_INDEX] |= VP890_ICR2_ILA_DAC;
             *      icr2[VP890_ICR2_VOC_DAC_INDEX] &= ~VP890_ICR2_VOC_DAC_SENSE;
             *      icr2[VP890_ICR2_VOC_DAC_INDEX+1] &= ~VP890_ICR2_ILA_DAC;
             */
            pLineObj->lineTimers.timers.timer[VP_LINE_DISCONNECT_EXIT] =
                (MS_TO_TICKRATE(dischargeTime, pDevObj->devProfileData.tickRate))
                | VP_ACTIVATE_TIMER;
        }

        Vp890WriteLPEnterRegisters(pLineCtx, deviceId, ecVal, lineState);
    }
    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp890SetLP-"));
}

/**
 * Vp890WriteLPExitRegisters()
 *  This function writes the ICR and State values to the device for LPM exit.
 *
 * Preconditions:
 *  None. Modification of line object data only.
 *
 * Postconditions:
 *  The device registers have been modified.
 */
void
Vp890WriteLPExitRegisters(
    VpLineCtxType *pLineCtx,
    VpDeviceIdType deviceId,
    uint8 ecVal,
    uint8 *lineState)
{
    Vp890LineObjectType *pLineObj = pLineCtx->pLineObj;

    VpMpiCmdWrapper(deviceId, ecVal, VP890_ICR3_WRT, VP890_ICR3_LEN,
        pLineObj->icr3Values);

    VpMpiCmdWrapper(deviceId, ecVal, VP890_ICR4_WRT, VP890_ICR4_LEN,
        pLineObj->icr4Values);

    if (lineState == VP_NULL) {
        VP_LINE_STATE(VpLineCtxType, pLineCtx, ("LP Exit: Writing LineState %d on Ch %d",
            pLineObj->lineState.currentState, pLineObj->channelId));

        Vp890SetFxsLineState(pLineCtx, pLineObj->lineState.currentState);
    } else {
        VP_LINE_STATE(VpLineCtxType, pLineCtx, ("LP Exit: Writing SLIC State 0x%02X on Ch %d",
            lineState[0], pLineObj->channelId));

        VpMpiCmdWrapper(deviceId, ecVal, VP890_SYS_STATE_WRT, VP890_SYS_STATE_LEN,
            lineState);
    }

    Vp890ProtectedWriteICR1(pLineObj, deviceId, pLineObj->icr1Values);

    VpMpiCmdWrapper(deviceId, ecVal, VP890_ICR2_WRT, VP890_ICR2_LEN,
        pLineObj->icr2Values);

    VP_LINE_STATE(VpLineCtxType, pLineCtx, ("LP Exit: Writing ICR3 0x%02X 0x%02X 0x%02X 0x%02X on Ch %d",
        pLineObj->icr3Values[0], pLineObj->icr3Values[1],
        pLineObj->icr3Values[2], pLineObj->icr3Values[3], pLineObj->channelId));

    VP_LINE_STATE(VpLineCtxType, pLineCtx, ("LP Exit: Writing ICR4 0x%02X 0x%02X 0x%02X 0x%02X on Ch %d",
        pLineObj->icr4Values[0], pLineObj->icr4Values[1],
        pLineObj->icr4Values[2], pLineObj->icr4Values[3], pLineObj->channelId));

    VP_LINE_STATE(VpLineCtxType, pLineCtx, ("LP Exit: Writing ICR1 0x%02X 0x%02X 0x%02X 0x%02X on Ch %d",
        pLineObj->icr1Values[0], pLineObj->icr1Values[1],
        pLineObj->icr1Values[2], pLineObj->icr1Values[3], pLineObj->channelId));

    VP_LINE_STATE(VpLineCtxType, pLineCtx, ("LP Exit: Writing ICR2 0x%02X 0x%02X 0x%02X 0x%02X on Ch %d",
        pLineObj->icr2Values[0], pLineObj->icr2Values[1],
        pLineObj->icr2Values[2], pLineObj->icr2Values[3], pLineObj->channelId));
}

/**
 * Vp890WriteLPEnterRegisters()
 *  This function writes the ICR and State values to the device for LPM enter.
 *
 * Preconditions:
 *  None. Modification of line object data only.
 *
 * Postconditions:
 *  The device registers have been modified.
 */
void
Vp890WriteLPEnterRegisters(
    VpLineCtxType *pLineCtx,
    VpDeviceIdType deviceId,
    uint8 ecVal,
    uint8 *lineState)
{
    Vp890LineObjectType *pLineObj = pLineCtx->pLineObj;

    Vp890ProtectedWriteICR1(pLineObj, deviceId, pLineObj->icr1Values);

    VpMpiCmdWrapper(deviceId, ecVal, VP890_ICR2_WRT, VP890_ICR2_LEN,
        pLineObj->icr2Values);

    VpMpiCmdWrapper(deviceId, ecVal, VP890_SYS_STATE_WRT, VP890_SYS_STATE_LEN,
        lineState);

    VpMpiCmdWrapper(deviceId, ecVal, VP890_ICR3_WRT, VP890_ICR3_LEN,
        pLineObj->icr3Values);

    VpMpiCmdWrapper(deviceId, ecVal, VP890_ICR4_WRT, VP890_ICR4_LEN,
        pLineObj->icr4Values);

    VP_LINE_STATE(VpLineCtxType, pLineCtx, ("LP Enter: Writing ICR1 0x%02X 0x%02X 0x%02X 0x%02X on Ch %d",
        pLineObj->icr1Values[0], pLineObj->icr1Values[1],
        pLineObj->icr1Values[2], pLineObj->icr1Values[3], pLineObj->channelId));

    VP_LINE_STATE(VpLineCtxType, pLineCtx, ("LP Enter: Writing ICR2 0x%02X 0x%02X 0x%02X 0x%02X on Ch %d",
        pLineObj->icr2Values[0], pLineObj->icr2Values[1],
        pLineObj->icr2Values[2], pLineObj->icr2Values[3], pLineObj->channelId));

    /* Set line to desired state */
    VP_LINE_STATE(VpLineCtxType, pLineCtx, ("LP Enter: Setting State 0x%02X on Ch %d",
        lineState[0], pLineObj->channelId));

    VP_LINE_STATE(VpLineCtxType, pLineCtx, ("LP Enter: Writing ICR3 0x%02X 0x%02X 0x%02X 0x%02X on Ch %d",
        pLineObj->icr3Values[0], pLineObj->icr3Values[1],
        pLineObj->icr3Values[2], pLineObj->icr3Values[3], pLineObj->channelId));

    VP_LINE_STATE(VpLineCtxType, pLineCtx, ("LP Enter: Writing ICR4 0x%02X 0x%02X 0x%02X 0x%02X on Ch %d",
        pLineObj->icr4Values[0], pLineObj->icr4Values[1],
        pLineObj->icr4Values[2], pLineObj->icr4Values[3], pLineObj->channelId));
}

/**
 * Vp890SetLPRegisters()
 *  This function modifies the line object ICR register values. It does not
 * write to the device.
 *
 * Preconditions:
 *  None. Modification of line object data only.
 *
 * Postconditions:
 *  The line object data (ICR values) have been modified.
 */
void
Vp890SetLPRegisters(
    Vp890DeviceObjectType *pDevObj,
    Vp890LineObjectType *pLineObj,
    bool lpModeTo)
{
    VP_API_FUNC_INT(None, VP_NULL, ("Vp890SetLPRegisters+"));

    if (lpModeTo == TRUE) {
        /* Set Line Bias, remove other bias control */
        pLineObj->icr1Values[VP890_ICR1_BIAS_OVERRIDE_LOCATION] =
            VP890_ICR1_LINE_BIAS_OVERRIDE;
        pLineObj->icr1Values[VP890_ICR1_BIAS_OVERRIDE_LOCATION+1] = 0x08;

        pLineObj->icr1Values[VP890_ICR1_RING_AND_DAC_LOCATION] &=
            ~VP890_ICR1_RING_BIAS_DAC_MASK;
        pLineObj->icr1Values[VP890_ICR1_RING_AND_DAC_LOCATION+1] &=
            ~VP890_ICR1_RING_BIAS_DAC_MASK;

        pLineObj->icr2Values[VP890_ICR2_SENSE_INDEX] =
            (VP890_ICR2_DAC_SENSE | VP890_ICR2_FEED_SENSE
           | VP890_ICR2_TIP_SENSE | VP890_ICR2_RING_SENSE);

        pLineObj->icr2Values[VP890_ICR2_SENSE_INDEX+1] =
            (VP890_ICR2_FEED_SENSE
            | VP890_ICR2_TIP_SENSE | VP890_ICR2_RING_SENSE);

        pLineObj->icr2Values[VP890_ICR2_SWY_CTRL_INDEX] =
            (VP890_ICR2_SWY_LIM_CTRL1 | VP890_ICR2_SWY_LIM_CTRL);

        if (pDevObj->stateInt & VP890_IS_HIGH_VOLTAGE) {
            pLineObj->icr2Values[VP890_ICR2_SWY_CTRL_INDEX+1] =
                (VP890_ICR2_SWY_LIM_CTRL1 | VP890_ICR2_SWY_LIM_CTRL);
        } else {
            pLineObj->icr2Values[VP890_ICR2_SWY_CTRL_INDEX+1] =
                (VP890_ICR2_SWY_LIM_CTRL1);
        }

        pLineObj->icr3Values[VP890_ICR3_LINE_CTRL_INDEX] |= VP890_ICR3_LINE_CTRL;
        pLineObj->icr3Values[VP890_ICR3_LINE_CTRL_INDEX+1] |= VP890_ICR3_LINE_CTRL;

        pLineObj->icr4Values[VP890_ICR4_SUP_INDEX] |=
            (VP890_ICR4_SUP_DAC_CTRL | VP890_ICR4_SUP_DET_CTRL
            | VP890_ICR4_SUP_POL_CTRL);
        pLineObj->icr4Values[VP890_ICR4_SUP_INDEX+1] |=
            (VP890_ICR4_SUP_DAC_CTRL | VP890_ICR4_SUP_DET_CTRL
            | VP890_ICR4_SUP_POL_CTRL);
    } else {
        pLineObj->icr3Values[VP890_ICR3_LINE_CTRL_INDEX] &= ~VP890_ICR3_LINE_CTRL;

        pLineObj->icr4Values[VP890_ICR4_SUP_INDEX] &=
            ~(VP890_ICR4_SUP_DAC_CTRL | VP890_ICR4_SUP_DET_CTRL
            | VP890_ICR4_SUP_POL_CTRL);

        /* Remove previously set SW control of ICR1 */
        pLineObj->icr1Values[VP890_ICR1_BIAS_OVERRIDE_LOCATION] &=
            ~VP890_ICR1_LINE_BIAS_OVERRIDE;

        pLineObj->icr2Values[VP890_ICR2_SENSE_INDEX] &=
            ~(VP890_ICR2_RING_SENSE | VP890_ICR2_TIP_SENSE |
              VP890_ICR2_DAC_SENSE | VP890_ICR2_FEED_SENSE);
    }

    VP_API_FUNC_INT(None, VP_NULL, ("Vp890SetLPRegisters-"));
}

/*******************************************************************************
 * Vp890IsChnlUndrTst()
 *  This function determines if a particular line of a device is currently
 * running a test.
 *
 * Preconditions:
 *  None.
 *
 * Postconditions:
 *  Device not affected. Return value TRUE if the line is currently running a
 * test, FALSE otherwise.
 ******************************************************************************/
static bool
Vp890IsChnlUndrTst(
    Vp890DeviceObjectType *pDevObj,
    uint8 channelId)
{
#ifdef VP890_INCLUDE_TESTLINE_CODE
    if (pDevObj->currentTest.rdLoopTest == FALSE) {
        if ((TRUE == pDevObj->currentTest.prepared) &&
            (channelId == pDevObj->currentTest.channelId)) {
            return TRUE;
        }
    }
#endif
    return FALSE;
}

/**
 * Vp890FreeRun()
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
Vp890FreeRun(
    VpDevCtxType *pDevCtx,
    VpFreeRunModeType freeRunMode)
{
    VpLineCtxType *pLineCtx;
    Vp890DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    Vp890LineObjectType *pLineObj;
    VpDeviceIdType deviceId = pDevObj->deviceId;

    uint8 ecVal = VP890_EC_CH1;
    VpLineStateType lineState;
    uint8 powerMode[VP890_REGULATOR_CTRL_LEN];

    VP_API_FUNC(VpDevCtxType, pDevCtx, ("Vp890FreeRun Mode %d", freeRunMode));

    VpSysEnterCritical(deviceId, VP_CODE_CRITICAL_SEC);

    /*
     * Time value is passed in 500us increment. If timeOut = 0, only PCLK
     * recovery exits restart prepare operations. If less than one tick, force
     * a one tick timeout.
     */
    if (freeRunMode == VP_FREE_RUN_STOP) {
        Vp890RestartComplete(pDevCtx);
        /*
         * Clear the device as being forced into free run mode by application.
         * This allows PCLK fault detection to automatically enter/exit free
         * run mode.
         */
        pDevObj->stateInt &= ~VP890_FORCE_FREE_RUN;
        VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
        return VP_STATUS_SUCCESS;
    }

    /* Take the lines out of Ringing if necessary */
    pLineCtx = pDevCtx->pLineCtx[0];
    if (pLineCtx != VP_NULL) {
        pLineObj = pLineCtx->pLineObj;
        ecVal = pLineObj->ecVal;
        lineState = pLineObj->lineState.usrCurrent;

        if (lineState == VP_LINE_RINGING) {
            Vp890SetLineState(pLineCtx, VP_LINE_STANDBY);
        }
        if (lineState == VP_LINE_RINGING_POLREV) {
            Vp890SetLineState(pLineCtx, VP_LINE_STANDBY_POLREV);
        }
    }

    /* Load the free run timing, if available. Othewise just switch to HP mode */
    if (pDevObj->devProfileData.timingParamsFR[0] != 0x00) {
        VpMpiCmdWrapper(deviceId, ecVal, VP890_REGULATOR_TIMING_WRT,
            VP890_REGULATOR_TIMING_LEN, pDevObj->devProfileData.timingParamsFR);
    } else {
        pDevObj->devProfileData.swParams[VP890_SWY_AUTOPOWER_INDEX] |= VP890_SWY_AUTOPOWER_DIS;

        VpMpiCmdWrapper(deviceId, pDevObj->ecVal, VP890_REGULATOR_PARAM_WRT,
            VP890_REGULATOR_PARAM_LEN, pDevObj->devProfileData.swParams);

        /* Change the Switchers to High Power Mode */
        VpMpiCmdWrapper(deviceId, ecVal, VP890_REGULATOR_CTRL_RD,
            VP890_REGULATOR_CTRL_LEN, powerMode);
        powerMode[0] &= ~VP890_SWY_MODE_MASK;
        powerMode[0] |= VP890_SWY_HP;
        VpMpiCmdWrapper(deviceId, ecVal, VP890_REGULATOR_CTRL_WRT,
            VP890_REGULATOR_CTRL_LEN, powerMode);
    }

    /*
     * Mark the device as being forced into free run mode by application. This
     * prevents auto-recovery when PCLK is restored.
     */
    pDevObj->stateInt |= VP890_FORCE_FREE_RUN;

    VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
    return VP_STATUS_SUCCESS;
}

/**
 * Vp890RestartComplete()
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
Vp890RestartComplete(
    VpDevCtxType *pDevCtx)
{
    VpLineCtxType *pLineCtx;
    Vp890DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    Vp890LineObjectType *pLineObj;
    VpDeviceIdType deviceId = pDevObj->deviceId;
    uint8 ecVal = VP890_EC_CH1;
    uint8 powerMode[VP890_REGULATOR_CTRL_LEN];

    /* Make sure to use the correct EC value. */
    pLineCtx = pDevCtx->pLineCtx[0];
    if (pLineCtx != VP_NULL) {
        pLineObj = pLineCtx->pLineObj;
        ecVal = pLineObj->ecVal;
    }

    /* Restore normal timing, if available. Othewise just switch to LP mode */
    if (pDevObj->devProfileData.timingParamsFR[0] != 0x00) {
        /* Restore the original timings */
        VpMpiCmdWrapper(deviceId, ecVal, VP890_REGULATOR_TIMING_WRT,
            VP890_REGULATOR_TIMING_LEN, pDevObj->devProfileData.timingParams);
    } else {
        /* Change the Switchers to Low Power Mode */
        VpMpiCmdWrapper(deviceId, ecVal, VP890_REGULATOR_CTRL_RD,
            VP890_REGULATOR_CTRL_LEN, powerMode);
        powerMode[0] &= ~VP890_SWY_MODE_MASK;
        powerMode[0] |= VP890_SWY_LP;
        VpMpiCmdWrapper(deviceId, ecVal, VP890_REGULATOR_CTRL_WRT,
            VP890_REGULATOR_CTRL_LEN, powerMode);

        /* Relinquish switcher control. */
        pDevObj->devProfileData.swParams[VP890_SWY_AUTOPOWER_INDEX] &= ~VP890_SWY_AUTOPOWER_DIS;
        VpMpiCmdWrapper(deviceId, pDevObj->ecVal, VP890_REGULATOR_PARAM_WRT,
            VP890_REGULATOR_PARAM_LEN, pDevObj->devProfileData.swParams);
    }
}

/*******************************************************************************
 * Vp890ProcessFxsLine()
 *  This function should only be called by Vp890ApiTick on FXS lines. It
 * performs all line processing for operations that are Tick based
 *
 * Arguments:
 *
 * Preconditions:
 *  Conditions defined by purpose of Api Tick.
 *
 * Postconditions:
 *  The Api variables and events (as appropriate) for the line passed have been
 * updated.
 ******************************************************************************/
static void
Vp890ProcessFxsLine(
    Vp890DeviceObjectType   *pDevObj,
    VpLineCtxType           *pLineCtx)
{
    Vp890LineObjectType         *pLineObj   = pLineCtx->pLineObj;
    bool                        dpStatus[2] = {FALSE, FALSE};
    VpOptionEventMaskType       lineEvents1;
    VpOptionEventMaskType       lineEvents2;
    VpDialPulseDetectStatesType beforeState, afterState;

#ifdef CSLAC_SEQ_EN
    VpDeviceIdType              deviceId    = pDevObj->deviceId;
    uint8                       ecVal       = pLineObj->ecVal;
    uint8 cidParam[VP890_CID_PARAM_LEN];
    uint8 cidState;
#endif
    uint8 hookStatus = 0, i, validSamples;
    uint8 hookIncrement;
    bool dp2Valid;

    /* Skip processing during line test */
    if (Vp890IsChnlUndrTst(pDevObj, pLineObj->channelId) == TRUE) {
        return;
    }

    lineEvents1.signaling = 0;
    lineEvents2.signaling = 0;

    /* If the secondary pulse params are all 0 (default), mark them as invalid
     * so that they will not be used. */
    if (pDevObj->pulseSpecs2.breakMin == 0 &&
        pDevObj->pulseSpecs2.breakMax == 0 &&
        pDevObj->pulseSpecs2.makeMin == 0 &&
        pDevObj->pulseSpecs2.makeMax == 0 &&
#ifdef EXTENDED_FLASH_HOOK
        pDevObj->pulseSpecs2.onHookMin == 0 &&
#endif
        pDevObj->pulseSpecs2.interDigitMin == 0 &&
        pDevObj->pulseSpecs2.flashMin == 0 &&
        pDevObj->pulseSpecs2.flashMax == 0)
    {
        dp2Valid = FALSE;
    } else {
        dp2Valid = TRUE;
    }

    /*
     * If the line is configured for Dial Pulse Detection, run the Dial Pulse
     * detection code. Dial Pulse detection code will generate the appropriate
     * events
     */
    if(pLineObj->pulseMode == VP_OPTION_PULSE_DECODE_ON) {
        switch (pDevObj->txBufferDataRate) {
            case VP890_CC_500HZ_RATE:
                hookIncrement = 16;
                break;

            case VP890_CC_1KHZ_RATE:
                hookIncrement = 8;
                break;

            case VP890_CC_2KHZ_RATE:
                hookIncrement = 4;
                break;

            case VP890_CC_4KHZ_RATE:
                hookIncrement = 2;
                break;

            case VP890_CC_8KHZ_RATE:
                hookIncrement = 1;
                break;

            default:
                /* We should never reach here */
                hookIncrement = 16;
                break;
        }

        validSamples = ((pDevObj->testDataBuffer[VP890_TEST_DATA_LEN_INDEX]
            & VP890_TEST_DATA_LEN_MASK) >> 4);

        if (validSamples == 7) {
            validSamples = 6;
        }

        hookStatus = pDevObj->testDataBuffer[VP890_TEST_DATA_HK_INDEX]
            & VP890_TEST_DATA_HK_MASK;

        if (pLineObj->status & VP890_LOW_POWER_EN) {
            hookStatus = ~hookStatus;
        }

        beforeState = pLineObj->dpStruct.state;

        /* Adjust the end of the loop open or closed period if the test buffer
         * has been updated */
        if (pDevObj->status.state & VP_DEV_TEST_BUFFER_READ) {
            /*
            VP_HOOK(VpLineCtxType, pLineCtx, ("CH%d Validsamples %d, buffer %02X %02X %d%d%d%d%d%d\n",
                channelId,
                ((pDevObj->testDataBuffer[VP890_TEST_DATA_LEN_INDEX] & VP890_TEST_DATA_LEN_MASK) >> 4),
                pDevObj->txBuffer[0],
                pDevObj->txBuffer[1],
                (hookStatus & 0x20) == 0x20,
                (hookStatus & 0x10) == 0x10,
                (hookStatus & 0x08) == 0x08,
                (hookStatus & 0x04) == 0x04,
                (hookStatus & 0x02) == 0x02,
                (hookStatus & 0x01) == 0x01));
            */
            for (i = 1; i < (1 << validSamples); i <<= 1) {
                if (beforeState == VP_DP_DETECT_STATE_LOOP_CLOSE) {
                    if (!(hookStatus & i)) {
                        if (pLineObj->dpStruct.lc_time > hookIncrement) {
                            pLineObj->dpStruct.lc_time -= hookIncrement;
                        }
                        if (pLineObj->dpStruct2.lc_time > hookIncrement) {
                            pLineObj->dpStruct2.lc_time -= hookIncrement;
                        }
                    } else {
                        break;
                    }
                } else if (beforeState == VP_DP_DETECT_STATE_LOOP_OPEN) {
                    if (hookStatus & i) {
                        if (pLineObj->dpStruct.lo_time > hookIncrement) {
                            pLineObj->dpStruct.lo_time -= hookIncrement;
                        }
                        if (pLineObj->dpStruct2.lo_time > hookIncrement) {
                            pLineObj->dpStruct2.lo_time -= hookIncrement;
                        }
                    } else {
                        break;
                    }
                }
            }
        }

        /* Compensate for slow onhook detection */
        if (beforeState == VP_DP_DETECT_STATE_LOOP_CLOSE && !(pLineObj->dpStruct.hookSt)) {
            if (pLineObj->dpStruct.lc_time > VP890_PULSE_DETECT_ADJUSTMENT) {
                pLineObj->dpStruct.lc_time -= VP890_PULSE_DETECT_ADJUSTMENT;
            }
        }
        if (beforeState == VP_DP_DETECT_STATE_LOOP_CLOSE && !(pLineObj->dpStruct2.hookSt)) {
            if (pLineObj->dpStruct2.lc_time > VP890_PULSE_DETECT_ADJUSTMENT) {
                pLineObj->dpStruct2.lc_time -= VP890_PULSE_DETECT_ADJUSTMENT;
            }
        }

        dpStatus[0] = VpUpdateDP(pDevObj->devProfileData.tickRate,
            &pDevObj->pulseSpecs, &pLineObj->dpStruct, &lineEvents1);
        if (dp2Valid == TRUE) {
            dpStatus[1] = VpUpdateDP(pDevObj->devProfileData.tickRate,
                &pDevObj->pulseSpecs2, &pLineObj->dpStruct2, &lineEvents2);
        }
        afterState = pLineObj->dpStruct.state;
        /* Update the loop open and close times according to the hook change
         * within a tick */

        /* If the state changed, adjust the hook timings */
        if (beforeState != afterState) {
            if (pDevObj->status.state & VP_DEV_TEST_BUFFER_READ) {
                for (i = 1; i < (1 << validSamples); i <<= 1) {
                    if (afterState == VP_DP_DETECT_STATE_LOOP_CLOSE) {
                        if (hookStatus & i) {
                            pLineObj->dpStruct.lc_time += hookIncrement;
                            pLineObj->dpStruct2.lc_time += hookIncrement;
                        } else {
                            break;
                        }
                    } else if (afterState == VP_DP_DETECT_STATE_LOOP_OPEN) {
                        if (!(hookStatus & i)) {
                            pLineObj->dpStruct.lo_time += hookIncrement;
                            pLineObj->dpStruct2.lo_time += hookIncrement;
                        } else {
                            break;
                        }
                    }
                }
            }
            if (afterState == VP_DP_DETECT_STATE_LOOP_OPEN) {
                pLineObj->dpStruct.lo_time += VP890_PULSE_DETECT_ADJUSTMENT;
                pLineObj->dpStruct2.lo_time += VP890_PULSE_DETECT_ADJUSTMENT;
            }
        }

        /*
         * The state machines will not necessarily complete at the same time, so
         * keep track of each and when both are done, report a passing digit if
         * one exists, or invalid if no criteria was met.
         */
        if (dpStatus[0] == TRUE) {
            pLineObj->signaling1 = lineEvents1.signaling;
            pLineObj->lineEventHandle = pDevObj->timeStamp;

            if (!(pLineObj->lineEvents.signaling & VP_LINE_EVID_BREAK_MAX)) {
                pLineObj->status |= VP890_DP_SET1_DONE;
            }
        }

        if (dpStatus[1] == TRUE && dp2Valid == TRUE) {
            pLineObj->signaling2 = lineEvents2.signaling;
            pLineObj->lineEventHandle = pDevObj->timeStamp;

            if (!(pLineObj->lineEvents.signaling & VP_LINE_EVID_BREAK_MAX)) {
                pLineObj->status |= VP890_DP_SET2_DONE;
            }
        }

        /* Report events if:
         *  Both DP sets are done, OR
         *  Set 1 is done and set 2 is invalid */
        if ((pLineObj->status & VP890_DP_SET1_DONE) &&
            ((pLineObj->status & VP890_DP_SET2_DONE) ||
             dp2Valid == FALSE))
        {
            /* Use the results of DP set 1 if it detected a valid digit, or
             * if DP set 2 detected an invalid digit, or if set 2 is disabled */
            if (pLineObj->dpStruct.digits != -1 ||
                pLineObj->dpStruct2.digits == -1 ||
                dp2Valid == FALSE)
            {
                pLineObj->signalingData = pLineObj->dpStruct.digits;
                pLineObj->lineEvents.signaling |= pLineObj->signaling1;
                pLineObj->lineEventHandle = VP_DP_PARAM1;
            } else {
                pLineObj->signalingData = pLineObj->dpStruct2.digits;
                pLineObj->lineEvents.signaling |= pLineObj->signaling2;
                pLineObj->lineEventHandle = VP_DP_PARAM2;
            }

            if (pLineObj->signalingData == 0) {
                pLineObj->signalingData = pLineObj->lineEventHandle;
                pLineObj->lineEventHandle = pDevObj->timeStamp;
            }

            pLineObj->status &= ~(VP890_DP_SET1_DONE | VP890_DP_SET2_DONE);
            pLineObj->signaling1 = 0;
            pLineObj->signaling2 = 0;
        }
    }

#ifdef CSLAC_SEQ_EN
    /*
     * If Caller ID sequencer is in progress, update unless it's in a state of
     * suspension. If suspended, re-enable if device is in underrun (no more
     * data to transmit).
     */
    if ((pLineObj->callerId.status & VP_CID_IN_PROGRESS)
     || (pLineObj->suspendCid == TRUE)) {
        if (pLineObj->suspendCid == TRUE) {
            /*
             * Check to see if the Device Buffer is empty. If it is, continue
             * with CID.
             */
            VpMpiCmdWrapper(deviceId, ecVal, VP890_CID_PARAM_RD,
                VP890_CID_PARAM_LEN, cidParam);
            cidState = (cidParam[0] & VP890_CID_STATE_MASK);

            if ((cidState == VP890_CID_STATE_URUN)
             || (cidState == VP890_CID_STATE_IDLE)) {
                pLineObj->suspendCid = FALSE;
                cidParam[0] &= ~(VP890_CID_FRAME_BITS);
                cidParam[0] |= VP890_CID_DIS;

                Vp890MuteChannel(pLineCtx, FALSE);
                VpMpiCmdWrapper(deviceId, ecVal, VP890_CID_PARAM_WRT,
                    VP890_CID_PARAM_LEN, cidParam);
                VpCidSeq(pLineCtx);
            }
        } else {
            VpCidSeq(pLineCtx);
        }
    }

#endif

    return;
} /* Vp890ProcessFxsLine() */

/*******************************************************************************
 * Vp890ProcessFxoLine()
 * This function processes signaling information and generates events for an FXO
 * line.  This function deals only with persistent status signaling information,
 * so it only needs to be called once per tick.
 *
 * Arguments:   *pDevObj    - Device object ptr
 *              *pLineCtx   - Line context ptr
 *
 * Preconditions:   intReg in the device option must contain current signaling
 *                  register data.
 *
 * Postconditions:  FXO signaling data will be handled by generating events or
 *                  setting debounce/delay timers.
 ******************************************************************************/
static void
Vp890ProcessFxoLine(
    Vp890DeviceObjectType   *pDevObj,
    VpLineCtxType           *pLineCtx)
{
    Vp890LineObjectType   *pLineObj   = pLineCtx->pLineObj;
    VpDeviceIdType        deviceId    = pDevObj->deviceId;
    uint8                 channelId   = pLineObj->channelId;
    uint8                 ecVal       = pLineObj->ecVal;

    VpFXOTimerType        *pFxoTimers = &pLineObj->lineTimers.timers.fxoTimer;
    bool                  onHookState = TRUE;
    uint8                 intReg      = pDevObj->intReg[channelId];
    uint8                 lsdData;
    VpCslacLineCondType   tempAmpSt, tempDiscSt, tempPolRevSt, tempLiuSt, tempPohSt;
    VpCslacLineCondType   tempAcFault, tempDcFault;

    if(pLineObj->lineState.currentState == VP_LINE_FXO_LOOP_CLOSE
      || pLineObj->lineState.currentState == VP_LINE_FXO_TALK) {
        onHookState = FALSE;
    }

    /*
     * Ignore the detector for a period after the last hook state change, or
     * a longer period after the last hook state change AND if the previous
     * line condition was Ringing
     */
    if ((pFxoTimers->lastStateChange < VP890_FXO_ONHOOK_CHANGE_DEBOUNCE
            && onHookState == TRUE)
        || (pFxoTimers->lastStateChange < VP890_FXO_OFFHOOK_CHANGE_DEBOUNCE
            && onHookState == FALSE)
#ifdef CSLAC_SEQ_EN
        || ((pLineObj->cadence.status & VP_CADENCE_STATUS_ACTIVE)
            && (pLineObj->intSequence[VP_PROFILE_TYPE_LSB]
                == VP_PRFWZ_PROFILE_MOMENTARY_LOOP_OPEN_INT))
#endif
        ) {
        /* Middle of detector Mask. Skip this process */
    } else {

        /* Ringing */
        if(onHookState){
            if (intReg & VP890_RING_DET_MASK) {
                tempAmpSt = VP_CSLAC_RING_AMP_DET;
            } else {
                tempAmpSt = 0;
            }
        } else {
            tempAmpSt = 0;
        }

        if ((pLineObj->lineState.condition & VP_CSLAC_RING_AMP_DET) != tempAmpSt) {
            Vp890AmpFreqRingDetect(pLineCtx, tempAmpSt);
        }
        /* --Ringing */

        /* Polrev */
        /* Ignore while ringing, disconnected, or shortly after a polrev because
           when ringing occurs, several polrevs are seen before ringing is detected. */
        if((pLineObj->lineState.condition & VP_CSLAC_RINGING) != VP_CSLAC_RINGING
        && (pLineObj->lineState.condition & VP_CSLAC_RAW_DISC) != VP_CSLAC_RAW_DISC
         && pFxoTimers->timeLastPolRev > VP890_FXO_POLREV_SILENCE) {
            if (intReg & VP890_POL_MASK) {
                tempPolRevSt = VP_CSLAC_POLREV;
            } else {
                tempPolRevSt = 0;
            }
            if ((pLineObj->lineState.condition & VP_CSLAC_POLREV) != tempPolRevSt) {
                VP_INFO(Vp890DeviceObjectType, pDevObj, ("Signaling Register 0x%02X 0x%02X Channel %d",
                    pDevObj->intReg[0], pDevObj->intReg[1], channelId));

                /*
                 * Workaround - If we see a change in polrev status, force the line
                 * side device to update, then read sigreg again to make sure
                 */
                VpMpiCmdWrapper(deviceId, ecVal, VP890_LSD_CTL_RD, VP890_LSD_CTL_LEN, &lsdData);
                VpMpiCmdWrapper(deviceId, ecVal, VP890_LSD_CTL_WRT, VP890_LSD_CTL_LEN, &lsdData);
                /* LSD forced to update at this point, reread signaling register */
                VpMpiCmdWrapper(deviceId, ecVal, VP890_NO_UL_SIGREG_RD,
                    VP890_NO_UL_SIGREG_LEN, pDevObj->intReg);
                intReg = pDevObj->intReg[channelId];
                if (intReg & VP890_POL_MASK) {
                    tempPolRevSt = VP_CSLAC_POLREV;
                } else {
                    tempPolRevSt = 0;
                }
            }

            if ((pLineObj->lineState.condition & VP_CSLAC_POLREV) != tempPolRevSt) {
                pLineObj->lineState.condition &= ~VP_CSLAC_POLREV;
                pLineObj->lineState.condition |= tempPolRevSt;
                pLineObj->lineEventHandle = pDevObj->timeStamp;
                pFxoTimers->timeLastPolRev = 0;
                pLineObj->lineEvents.fxo |= VP_LINE_EVID_POLREV;
                if (tempPolRevSt) {
                    pLineObj->fxoData = VP_POLREV_REVERSE;
                } else {
                    pLineObj->fxoData = VP_POLREV_NORMAL;
                }
            }
        }
        /* --Polrev */

        /* LIU */
        /* Implement the low voltage disconnect/LIU distinction workaround
         * adjustments.  This will modify the contents of intReg */
        if (onHookState && pLineObj->lowVoltageDetection.enabled == TRUE) {
            MakeLowVoltageCorrections(pLineObj, &intReg);
        }
        /* Ignore while ringing, suspecting ringing, or disconnected */
        if(((pLineObj->lineState.condition & VP_CSLAC_RAW_DISC) != VP_CSLAC_RAW_DISC)
         && (!(pLineObj->lineState.condition & VP_CSLAC_RINGING))
         && (onHookState)
         && (pFxoTimers->ringOffDebounce > VP890_FXO_RING_OFF_DEBOUNCE_LIU)) {
            /* Trigger LIU off of either the LIU bit or the LDN bit.  This is
             * done so that the API will match the behavior of the 880 API.
             * The 880 FXO always reports LIU when it reports disconnect. */
            if ((intReg & VP890_LIU_MASK) || (intReg & VP890_LDN_MASK)) {
                tempLiuSt = VP_CSLAC_LIU;
            } else {
                tempLiuSt = 0;
            }

            if ((pLineObj->lineState.condition & VP_CSLAC_LIU) != tempLiuSt) {
                if (tempLiuSt) {
                    if (pFxoTimers->liuDebounce > VP890_FXO_LIU_DEBOUNCE) {
                        /* Start LIU debounce timer if not running */
                        /* With some DC biased ringing, LIU can show up
                           before even the first polrev */
                        pFxoTimers->liuDebounce = 0;
                    }
                } else {
                    if (pFxoTimers->liuDebounce > VP890_FXO_LIU_DEBOUNCE) {
                        /* Use the same timer for LNIU to even out pulse
                           width.  ServiceFxoTimers determines which event
                           to generate when the timer expires */
                        pFxoTimers->liuDebounce = 0;
                    }
                }
            } else if (pFxoTimers->liuDebounce <= VP890_FXO_LIU_DEBOUNCE) {
                    /* If LIU debounce timer is running, stop it */
                    pFxoTimers->liuDebounce = 255;
            }
        }
        /* --LIU */

        /* Disconnect and Feed */
        /* Don't process if currently or recently ringing.  Zero bias ringing
           will give false LDN=1 indications due to integration, and a parallel
           phone going offhook during ringing can also cause LDN=1 */
        if((!(pLineObj->lineState.condition & VP_CSLAC_RINGING))
        && (pFxoTimers->ringOffDebounce > VP890_FXO_RING_OFF_DEBOUNCE_DISC)) {
            /* The AC_FLT flag indicates overcurrent or current starve.  Treat
             * those as disconnect. */
            if ((intReg & VP890_LDN_MASK) ||
                (pLineObj->lineState.condition & VP_CSLAC_AC_FLT))
            {
                tempDiscSt = VP_CSLAC_RAW_DISC;
            } else {
                tempDiscSt = 0;
            }

            if ((pLineObj->lineState.condition & VP_CSLAC_RAW_DISC) != tempDiscSt) {
                pLineObj->lineState.condition &= ~VP_CSLAC_RAW_DISC;
                pLineObj->lineState.condition |= tempDiscSt;

                pLineObj->preDisconnect = tempDiscSt;
                pLineObj->lineTimers.timers.fxoTimer.disconnectDebounce =
                    MS_TO_TICKRATE(VP_FXO_DISCONNECT_DEBOUNCE,
                                    pDevObj->devProfileData.tickRate);

                VP_FXO_FUNC(VpLineCtxType, pLineCtx, ("Disconnect Change to %d at time %d",
                    tempDiscSt, pDevObj->timeStamp));

                /* If onhook and not in disconnect, start CID correction */
                if(onHookState) {
                    if (!(tempDiscSt)) {
                        /* Start the timer for caller ID correction */
                        pFxoTimers->cidCorrectionTimer = 0;
                    }
                }
            }
        }
        /* --Disconnect and Feed */

        /* Overvoltage */
        if (onHookState == TRUE) {
            if (intReg & VP890_OVIR_MASK) {
                tempDcFault = VP_CSLAC_DC_FLT;
            } else {
                tempDcFault = 0;
            }
            if ((pLineObj->lineState.condition & VP_CSLAC_DC_FLT) != tempDcFault) {
                pLineObj->lineState.condition &= ~VP_CSLAC_DC_FLT;
                pLineObj->lineState.condition |= tempDcFault;
                pLineObj->lineEventHandle = pDevObj->timeStamp;
                pLineObj->lineEvents.faults |= VP_LINE_EVID_DC_FLT;
            }
        }
        /* --Overvoltage */

        /* Overcurrent/POH */
        if (onHookState == FALSE) {
            uint16 absCurrent[VP890_PCM_BUF_SIZE];
            uint8 bufferVal;
            bool overCurrent = TRUE;

            if (pLineObj->currentMonitor.stateValue == VP890_CURRENT_MONITOR_NORMAL
                && !(pLineObj->status & VP890_LINE_IN_CAL))
            {
                /* Initialize values such that first sample forces an update */
                int16 minValue = 32767;
                int16 maxValue = -32767;
                int32 deltaMaxMin = 0;
                int32 averageCurrent = 0;
                for ( bufferVal = 0; bufferVal < VP890_PCM_BUF_SIZE; bufferVal++) {
                    absCurrent[bufferVal] = (pLineObj->currentMonitor.currentBuffer[bufferVal] < 0) ?
                        -pLineObj->currentMonitor.currentBuffer[bufferVal] :
                        pLineObj->currentMonitor.currentBuffer[bufferVal];

                    if (absCurrent[bufferVal] < 1230) {
                        overCurrent = FALSE;
                    }
                    averageCurrent += pLineObj->currentMonitor.currentBuffer[bufferVal];

                    if (minValue >  pLineObj->currentMonitor.currentBuffer[bufferVal]) {
                        minValue =  pLineObj->currentMonitor.currentBuffer[bufferVal];
                    }
                    if (maxValue <  pLineObj->currentMonitor.currentBuffer[bufferVal]) {
                        maxValue =  pLineObj->currentMonitor.currentBuffer[bufferVal];
                    }
                }
                /* Overcurrent Algorithm */
                if (overCurrent == TRUE) {
                    VP_FXO_FUNC(VpLineCtxType, pLineCtx, ("Possible Overcurrent or Current Starve"));
                    tempAcFault = VP_CSLAC_AC_FLT;
                } else {
                    tempAcFault = 0;
                }

                /* POH Algorithm */
                deltaMaxMin = maxValue;
                deltaMaxMin -=  minValue;
                averageCurrent /= (int32)VP890_PCM_BUF_SIZE;
                if (deltaMaxMin > 30) {
                    /*
                     * This is a period where a line transient occurred. Cannot
                     * be evaluated for POH, but triggers when a POH might have
                     * occurred.
                     */
                    pLineObj->currentMonitor.invalidData = TRUE;
                } else {
                    /*
                     * This is a period where line is stable. But if recovering
                     * from being stable, evaluate for POH.
                     */
                    if (pLineObj->currentMonitor.invalidData == TRUE) {
                        pLineObj->currentMonitor.invalidData = FALSE;

                        if (((averageCurrent > 0) && (pLineObj->currentMonitor.steadyStateAverage > 0))
                         || ((averageCurrent < 0) && (pLineObj->currentMonitor.steadyStateAverage < 0))) {
                            int32 absOld = (pLineObj->currentMonitor.steadyStateAverage < 0) ?
                                            -pLineObj->currentMonitor.steadyStateAverage :
                                            pLineObj->currentMonitor.steadyStateAverage;
                            int32 absNew = (averageCurrent < 0) ?
                                            -averageCurrent :
                                            averageCurrent;
                            int32 deltaOldNew = absOld - absNew;
                            int32 absDeltaOldNew = (deltaOldNew < 0) ? -deltaOldNew : deltaOldNew;

                            if (absDeltaOldNew > 50) {
                                /* A phone did something .. but what? */
                                if (deltaOldNew > 0) {
                                    tempPohSt = VP_CSLAC_POH;
                                    pLineObj->lineState.condition |= VP_CSLAC_LIU;
                                } else {
                                    tempPohSt = 0;
                                    pLineObj->lineState.condition &= ~VP_CSLAC_LIU;
                                }

                                /* --POH/Not-POH */
                                if ((pLineObj->lineState.condition & VP_CSLAC_POH) != tempPohSt) {
                                    pLineObj->lineState.condition &= ~VP_CSLAC_POH;
                                    pLineObj->lineState.condition |= tempPohSt;
                                    pLineObj->lineEventHandle = pDevObj->timeStamp;

                                    if (tempPohSt == VP_CSLAC_POH) {
                                        pLineObj->lineEvents.fxo |= VP_LINE_EVID_POH;
                                    } else {
                                        pLineObj->lineEvents.fxo |= VP_LINE_EVID_PNOH;
                                    }
                                }
                            }
                        }
                    }
                    pLineObj->currentMonitor.steadyStateAverage = averageCurrent;
                }
            } else {
                tempAcFault    = pLineObj->lineState.condition & VP_CSLAC_AC_FLT;
            }

            if((pLineObj->lineState.condition & VP_CSLAC_AC_FLT) != tempAcFault) {
                pLineObj->lineState.condition &= ~(VP_CSLAC_AC_FLT);
                pLineObj->lineState.condition |= tempAcFault;
            }
        } else {
            /* Clear this flag if the FXO is back onhook */
            pLineObj->lineState.condition &= ~(VP_CSLAC_AC_FLT);
        }
        /* --Overcurrent/POH */

    }

#ifdef VP890_FXO_DELAYED_RING_TRIP
    /* If ringing has stopped and there was a state change requested during
       ringing, service it now */
    if ((pLineObj->fxoRingStateFlag == 1)
     && (!(pLineObj->lineState.condition & VP_CSLAC_RINGING))) {
        pLineObj->fxoRingStateFlag = 0;
        Vp890SetFxoLineState(pLineCtx, pLineObj->fxoRingState);
    }
#endif
} /* Vp890ProcessFxoLine */

/*******************************************************************************
 * Vp890AmpFreqRingDetect()
 * This function processes the device level ringing detector and generates
 * appropriate event. Called only by Process FXO.
 *
 * Arguments:       *pLineCtx   - Line context ptr
 *                  tempAmp     - Indication of current ringing status.
 *
 * Preconditions:   intReg in the device option must contain current signaling
 *                  register data.
 *
 * Postconditions:  FXO signaling data will be handled by generating events or
 *                  setting debounce/delay timers.
 ******************************************************************************/
void
Vp890AmpFreqRingDetect(
    VpLineCtxType *pLineCtx,
    VpCslacLineCondType tempAmpSt)
{
    Vp890LineObjectType     *pLineObj   = pLineCtx->pLineObj;
    VpDevCtxType            *pDevCtx    = pLineCtx->pDevCtx;
    Vp890DeviceObjectType   *pDevObj    = pDevCtx->pDevObj;

    VpFXOTimerType          *pFxoTimers = &pLineObj->lineTimers.timers.fxoTimer;

    pLineObj->lineEventHandle = pDevObj->timeStamp;

    if (tempAmpSt) {
        VP_INFO(VpLineCtxType, pLineCtx, ("Ringing Detect at Time %d", pDevObj->timeStamp));
        pLineObj->lineState.condition |= VP_CSLAC_RING_AMP_DET;

        pLineObj->lineEvents.fxo |= VP_LINE_EVID_RING_ON;
        pLineObj->lineState.condition |= VP_CSLAC_RINGING;

        if (pFxoTimers->liuDebounce <= VP890_FXO_LIU_DEBOUNCE) {
            /* If LIU debounce timer is running, stop it */
            pFxoTimers->liuDebounce = 255;
        }

        /* Going from disconnect to 0-bias ringing, we won't see a change in
           LDN, but if we see ringing, there must be feed. */
        if (pLineObj->lineState.condition & VP_CSLAC_DISC) {
            pLineObj->lineEvents.fxo |= VP_LINE_EVID_FEED_EN;
            pLineObj->lineState.condition &= ~VP_CSLAC_DISC;
            pLineObj->lineEventHandle = pDevObj->timeStamp;
            /* When we get a feed enable event, start the timer for
             * caller ID correction */
            pFxoTimers->cidCorrectionTimer = 0;
        }

        /* Stop the disconnect timer and clear any raw disconnect indication */
        pLineObj->lineTimers.timers.fxoTimer.disconnectDebounce = 0;
        pLineObj->lineState.condition &= ~VP_CSLAC_RAW_DISC;

    } else {
        VP_INFO(Vp890DeviceObjectType, pDevObj, ("Ringing Remove at Time %d", pDevObj->timeStamp));
        pLineObj->lineState.condition &= ~VP_CSLAC_RING_AMP_DET;

        pLineObj->lineEvents.fxo |= VP_LINE_EVID_RING_OFF;
        pLineObj->lineState.condition &= ~VP_CSLAC_RINGING;
        pFxoTimers->ringOffDebounce = 0;
    }
}

/*******************************************************************************
 * Vp890ServiceInterrupts()
 * This function goes through every line associated with the given device and
 * calls Vp890ServiceFxsInterrupts.
 *
 * Arguments: *pDevCtx - Device context ptr
 *
 * Preconditions: intReg in the device object should contain global signaling
 *                  data as read by Vp890ApiTick
 *
 * Postconditions:
 ******************************************************************************/
static void
Vp890ServiceInterrupts(
    VpDevCtxType    *pDevCtx)
{
    VpLineCtxType           *pLineCtx;
    Vp890DeviceObjectType   *pDevObj    = pDevCtx->pDevObj;
    Vp890LineObjectType     *pLineObj;
    uint8                   maxChan     = pDevObj->staticInfo.maxChannels;
    uint8                   channelId;

    for (channelId = 0; channelId < maxChan; channelId++) {
        pLineCtx = pDevCtx->pLineCtx[channelId];
        if (pLineCtx != VP_NULL) {
            pLineObj = pLineCtx->pLineObj;

            if (!(pLineObj->status & VP890_INIT_COMPLETE)) {
                continue;
            }

            if (!(pLineObj->status & VP890_IS_FXO)) {
                Vp890ServiceFxsInterrupts(pLineCtx);
            }
        }
    }
} /* Vp890ServiceInterrupts() */

/*******************************************************************************
 * Vp890ServiceFxsInterrupts()
 *  This function should only be called by Vp890ApiTick when an interrupt
 * occurs.
 *
 * Preconditions:
 *  The device must first be initialized.
 *
 * Postconditions:
 *  The Global Signaling Register is read and the data is stored in the device
 * object.  Depending on the dial pulse mode option set, the hook event (on/off)
 * is generated if a hook status changed.  This function will return
 * TRUE if an event has been generated.
 ******************************************************************************/
static void
Vp890ServiceFxsInterrupts(
    VpLineCtxType         *pLineCtx)
{
    VpDevCtxType          *pDevCtx  = pLineCtx->pDevCtx;
    Vp890DeviceObjectType *pDevObj  = pDevCtx->pDevObj;
    Vp890LineObjectType   *pLineObj = pLineCtx->pLineObj;

#ifdef CSLAC_SEQ_EN
    VpProfilePtrType      pCadence;
#endif
    uint16                *pTimerAry = pLineObj->lineTimers.timers.timer;
    uint8                 channelId = pLineObj->channelId;
    VpLineStateType       state     = pLineObj->lineState.currentState;
    VpLineStateType       usrState  = pLineObj->lineState.usrCurrent;

    VpCslacLineCondType   tempHookSt, tempGnkSt, tempThermFault;
    uint8                 loopSupervision[VP890_LOOP_SUP_LEN];
    VpDeviceIdType        deviceId  = pDevObj->deviceId;
    uint8                 ecVal     = pLineObj->ecVal;

    /* If debouncing for Ring Exit or Caller ID, ignore hook.
     * Otherwise process. */
    if ((pTimerAry[VP_LINE_CID_DEBOUNCE] & VP_ACTIVATE_TIMER)           ||
        (pTimerAry[VP_LINE_RING_EXIT_DEBOUNCE] & VP_ACTIVATE_TIMER)     ||
        (pTimerAry[VP_LINE_POLREV_DEBOUNCE] & VP_ACTIVATE_TIMER)        ||
        (pTimerAry[VP_LINE_DISCONNECT_EXIT] & VP_ACTIVATE_TIMER)        ||
        (pTimerAry[VP_LINE_CAL_LINE_TIMER] & VP_ACTIVATE_TIMER)         ||
        (pDevObj->devTimer[VP_DEV_TIMER_LP_CHANGE] & VP_ACTIVATE_TIMER) ||
        (pDevObj->status.state & VP_DEV_IN_CAL)

#ifdef CSLAC_SEQ_EN
     || ((pLineObj->cadence.status & VP_CADENCE_STATUS_ACTIVE)
     && (pLineObj->intSequence[VP_PROFILE_TYPE_LSB] ==
            VP_PRFWZ_PROFILE_FWD_DISC_INT))

     || ((pLineObj->cadence.status & VP_CADENCE_STATUS_ACTIVE)
     && (pLineObj->intSequence[VP_PROFILE_TYPE_LSB] ==
            VP_PRFWZ_PROFILE_TIP_OPEN_INT))
#endif
     || (pLineObj->lineState.calType != VP_CSLAC_CAL_NONE)
     || (state == VP_LINE_DISCONNECT)) {
        tempHookSt = (pLineObj->lineState.condition & VP_CSLAC_HOOK);
    } else {
        if (pLineObj->status & VP890_LOW_POWER_EN) {
            if (pDevObj->intReg[channelId] & VP890_HOOK_MASK) {
                tempHookSt = 0;
            } else {
                tempHookSt = VP_CSLAC_HOOK;
            }
        } else {
            if (pDevObj->intReg[channelId] & VP890_HOOK_MASK) {
                tempHookSt = VP_CSLAC_HOOK;
            } else {
                tempHookSt = 0;
            }
        }
    }

    if (pDevObj->intReg[channelId] & VP890_TEMPA_MASK) {
        tempThermFault = VP_CSLAC_THERM_FLT;
    } else {
        tempThermFault = 0;
    }

    if ((pDevObj->devTimer[VP_DEV_TIMER_LP_CHANGE] & VP_ACTIVATE_TIMER)
     || (pDevObj->status.state & VP_DEV_IN_CAL)
     || (state == VP_LINE_DISCONNECT)
     || (pLineObj->lineTimers.timers.timer[VP_LINE_DISCONNECT_EXIT]
         & VP_ACTIVATE_TIMER)) {
        tempGnkSt = (pLineObj->lineState.condition & VP_CSLAC_GKEY);
    } else {
        if (pDevObj->intReg[channelId] & VP890_GNK_MASK) {
            tempGnkSt = VP_CSLAC_GKEY;
        } else {
            tempGnkSt = 0;
        }
    }

    /* If the hook conditions changed, continue line processing */
    if ((pLineObj->lineState.condition & VP_CSLAC_HOOK) != tempHookSt) {
        pLineObj->lineState.condition &= ~VP_CSLAC_HOOK;
        pLineObj->lineState.condition |= tempHookSt;
        VpMemCpy(loopSupervision, pLineObj->loopSup, VP890_LOOP_SUP_LEN);
        if ((pLineObj->status & VP890_LOW_POWER_EN) && (tempHookSt)) {
            VP_HOOK(VpLineCtxType, pLineCtx, ("Off-Hook Detected in Low Power Mode on line %d time %d",
                channelId, pDevObj->timeStamp));
            if ((loopSupervision[VP890_LOOP_SUP_THRESH_BYTE] & 0x07)
                >= pLineObj->hookHysteresis) {
                loopSupervision[VP890_LOOP_SUP_THRESH_BYTE] -= pLineObj->hookHysteresis;
            } else {
                loopSupervision[VP890_LOOP_SUP_THRESH_BYTE] &= 0xF8;
            }

            /* Force line to feed state and start leaky line detection */
            pLineObj->lineState.currentState = VP_LINE_OHT;
            pDevObj->stateInt &= ~(VP890_LINE0_LP | VP890_LINE1_LP);

            pLineObj->lineState.condition |= VP_CSLAC_LINE_LEAK_TEST;
        } else {

#ifdef CSLAC_SEQ_EN
            /*
             * There was a sufficient hook activity to stop the active
             * CID -- unless the CID sequence knew this would happen and
             * set the debounce flag. In which case, let CID continue.
             */
            if (pLineObj->callerId.status & VP_CID_IN_PROGRESS) {
                if (pLineObj->callerId.status & VP_CID_IS_DEBOUNCE) {
                    /* Hook event is fully debounced and ready to go */
                    pLineObj->callerId.status &= ~VP_CID_IS_DEBOUNCE;
                } else {
                    VpCliStopCli(pLineCtx);
                    Vp890SetLineTone(pLineCtx, VP_PTABLE_NULL,
                        VP_PTABLE_NULL, VP_NULL);
                }
            }
#endif

            if (tempHookSt) {
                VP_HOOK(VpLineCtxType, pLineCtx, ("Off-Hook on Line %d at Time %d Low Power 0x%02X",
                    channelId, pDevObj->timeStamp, (pLineObj->status & VP890_LOW_POWER_EN)));
                if ((loopSupervision[VP890_LOOP_SUP_THRESH_BYTE] & 0x07)
                    >= pLineObj->hookHysteresis) {
                    loopSupervision[VP890_LOOP_SUP_THRESH_BYTE] -= pLineObj->hookHysteresis;
                } else {
                    loopSupervision[VP890_LOOP_SUP_THRESH_BYTE] &= 0xF8;
                }

                pLineObj->dpStruct.hookSt = TRUE;
                pLineObj->dpStruct2.hookSt = TRUE;

                pLineObj->leakyLineCnt = 0;
                pLineObj->status &= ~VP890_LINE_LEAK;

                if(pLineObj->pulseMode == VP_OPTION_PULSE_DECODE_OFF) {
                    pLineObj->lineTimers.timers.timer[VP_LINE_OFFHOOK_DELAY] =
                        MS_TO_TICKRATE(VP890_OFFHOOK_EVENT_DELAY, pDevObj->devProfileData.tickRate);
                    pLineObj->lineTimers.timers.timer[VP_LINE_OFFHOOK_DELAY]
                        |= VP_ACTIVATE_TIMER;
                } else {
                    VP_HOOK(VpLineCtxType, pLineCtx, ("Off-Hook on Line %d at Time %d Signaling 0x%02X 0x%02X User State %d",
                        channelId, pDevObj->timeStamp, pDevObj->intReg[0], pDevObj->intReg[1], usrState));
                }

#ifdef CSLAC_SEQ_EN
                /*
                 * If an off-hook is detected when the active cadence
                 * is a Message Waiting Pulse on the line, restore the
                 * line state.
                 */
                pCadence = pLineObj->cadence.pActiveCadence;
                if (pCadence != VP_PTABLE_NULL) {
                    if (pCadence[VP_PROFILE_TYPE_LSB] ==
                        VP_PRFWZ_PROFILE_MSG_WAIT_PULSE_INT) {
                        Vp890SetFxsLineState(pLineCtx, state);
                    }
                }
#endif
                /*
                 * If an off-hook is detected during the user set state
                 * of Ringing (incl. ringing and silent interval) while
                 * a test is running, don't allow the api to go to the
                 * ringtrip state
                 */
                if(Vp890IsChnlUndrTst(pDevObj, channelId) == TRUE) {
                    /* Do not change line state during test */
                } else  {
                    if ((usrState == VP_LINE_RINGING)
                     || (usrState == VP_LINE_RINGING_POLREV)) {
                        VP_LINE_STATE(VpLineCtxType, pLineCtx, ("Off-Hook During Ringing -- Going to State %d on Line %d at Time %d",
                            pLineObj->ringCtrl.ringTripExitSt, channelId, pDevObj->timeStamp));

                        Vp890SetLineState(pLineCtx,
                            pLineObj->ringCtrl.ringTripExitSt);
                    }
                }
            } else {
                VP_HOOK(VpLineCtxType, pLineCtx, ("On-Hook on Line %d at Time %d",
                    channelId, pDevObj->timeStamp));
                /* Restore the on-hook current threshold */
                loopSupervision[VP890_LOOP_SUP_THRESH_BYTE] =
                    pLineObj->loopSup[VP890_LOOP_SUP_THRESH_BYTE];

                pLineObj->dpStruct.hookSt = FALSE;
                pLineObj->dpStruct2.hookSt = FALSE;

                if ((pLineObj->pulseMode == VP_OPTION_PULSE_DECODE_OFF) &&
                    ((pLineObj->lineState.condition & VP_CSLAC_LINE_LEAK_TEST) == 0)) {
                    /*
                     * If this is the first time after initialization
                     * that we are checking for on-hook and it is
                     * on-hook, don't generate an interrupt
                     */
                    if (!(pLineObj->lineState.condition & VP_CSLAC_STATUS_VALID)) {
                        pLineObj->lineEvents.signaling |=
                            VP_LINE_EVID_HOOK_ON;
                        pLineObj->lineEventHandle = pDevObj->timeStamp;
                    }
                }

                if (((pLineObj->termType == VP_TERM_FXS_LOW_PWR) ||
                     (pLineObj->termType == VP_TERM_FXS_ISOLATE_LP) ||
                     (pLineObj->termType == VP_TERM_FXS_SPLITTER_LP))) {

                    VP_HOOK(VpLineCtxType, pLineCtx, ("User State %d Current State %d",
                        pLineObj->lineState.usrCurrent, pLineObj->lineState.currentState));

                    if (pLineObj->lineState.usrCurrent == VP_LINE_STANDBY) {
                        pLineObj->lineState.currentState = VP_LINE_STANDBY;
                        Vp890LLSetSysState(deviceId, pLineCtx, 0x00, FALSE);
                    }
                }
            }
        }
        /* Do not cache the value, actually it uses the cached value to restore it */
        if (pLineObj->hookHysteresis != 0) {
            VpMpiCmdWrapper(deviceId, ecVal, VP890_LOOP_SUP_WRT, VP890_LOOP_SUP_LEN, loopSupervision);
        }
    }

    /* If the gkey conditions changed, continue line processing */
    if((pLineObj->lineState.condition & VP_CSLAC_GKEY)
        != tempGnkSt) {

        if (tempGnkSt) {
            pLineObj->lineEvents.signaling |= VP_LINE_EVID_GKEY_DET;
            pLineObj->lineState.condition |= VP_CSLAC_GKEY;
        } else {
            pLineObj->lineEvents.signaling |= VP_LINE_EVID_GKEY_REL;
            pLineObj->lineState.condition &= ~(VP_CSLAC_GKEY);
        }
        pLineObj->lineEventHandle = pDevObj->timeStamp;
    }

    if((pLineObj->lineState.condition & VP_CSLAC_THERM_FLT) != tempThermFault) {
        pLineObj->lineEventHandle = pDevObj->timeStamp;
        pLineObj->lineState.condition &= ~(VP_CSLAC_THERM_FLT);
        pLineObj->lineState.condition |= tempThermFault;

        pLineObj->lineEvents.faults |= VP_LINE_EVID_THERM_FLT;

        if (tempThermFault == VP_CSLAC_THERM_FLT) {
#ifdef VP890_INCLUDE_TESTLINE_CODE
            if((Vp890IsChnlUndrTst(pDevObj, channelId) == TRUE) ||
               (pDevObj->currentTest.rdLoopTest == TRUE)) {
                pLineObj->lineEvents.test |= VP_LINE_EVID_ABORT;
            } else if (pDevObj->criticalFault.thermFltDiscEn == TRUE) {
#endif
                Vp890SetLineState(pLineCtx, VP_LINE_DISCONNECT);
#ifdef VP890_INCLUDE_TESTLINE_CODE
            }
#endif
        }
    }
} /* Vp890ServiceFxsInterrupts() */

/*******************************************************************************
 * Vp890ServiceDevTimers()
 * This function services device-level timers.
 *
 * Arguments:  *pDevCtx - Device context ptr
 *
 * Preconditions:  Sould be called once per tick by Vp890ApiTick.
 *
 * Postconditions: Device timers will be serviced.
 ******************************************************************************/
static void
Vp890ServiceDevTimers(
    VpDevCtxType            *pDevCtx)
{
    Vp890DeviceObjectType   *pDevObj    = pDevCtx->pDevObj;
    VpDevTimerType          devTimer;

    for (devTimer = 0; devTimer < VP_DEV_TIMER_LAST; devTimer++) {

        /* if timer is not active go to next timer */
        if (!(pDevObj->devTimer[devTimer] & VP_ACTIVATE_TIMER)) {
            continue;
        }

        /* get the bits associated only with the time of the timer */
        pDevObj->devTimer[devTimer] &= VP_TIMER_TIME_MASK;

        /* decrement the timer */
        if (pDevObj->devTimer[devTimer] > 0) {
            (pDevObj->devTimer[devTimer])--;
        }

        /* if time left on the timer, active it and move on to the next one */
        if (pDevObj->devTimer[devTimer] != 0) {
            pDevObj->devTimer[devTimer] |= VP_ACTIVATE_TIMER;
            continue;
        }

        if (VP_DEV_TIMER_TESTLINE == devTimer) {
#ifdef VP890_INCLUDE_TESTLINE_CODE
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
#endif
        } else if (VP_DEV_TIMER_LP_CHANGE == devTimer) {
            uint8 channelId;
            Vp890LineObjectType *pLineObj;
            VpLineCtxType *pLineCtx;
            bool failureMode = FALSE;

            /* Skip this process if we're in calibration */
            if (!(pDevObj->status.state & VP_DEV_IN_CAL)) {
                VP_HOOK(VpDevCtxType, pDevCtx, ("Signaling 0x%02X 0x%02X",
                    pDevObj->intReg[0], pDevObj->intReg[1]));
                for (channelId = 0; channelId < pDevObj->staticInfo.maxChannels; channelId++) {
                    pLineCtx = pDevCtx->pLineCtx[channelId];
                    if (pLineCtx != VP_NULL) {
                        bool lpTermType = FALSE;
                        pLineObj = pLineCtx->pLineObj;

                        if ((pLineObj->termType == VP_TERM_FXS_LOW_PWR) ||
                            (pLineObj->termType == VP_TERM_FXS_SPLITTER_LP) ||
                            (pLineObj->termType == VP_TERM_FXS_ISOLATE_LP)) {
                            lpTermType = TRUE;
                        }

                        if (lpTermType == TRUE) {
                            VP_HOOK(VpLineCtxType, pLineCtx, ("Last Hook State on line %d = %d LP Mode %d CurrentState %d",
                                channelId, (pLineObj->lineState.condition & VP_CSLAC_HOOK),
                                (pLineObj->status & VP890_LOW_POWER_EN), pLineObj->lineState.currentState));
                            if (pLineObj->lineState.currentState != VP_LINE_DISCONNECT) {
                                if (pLineObj->status & VP890_LOW_POWER_EN) {
                                    /*
                                     * If we're in LP Mode, then the line should be
                                     * detecting on-hook. All other conditions mean
                                     * there could be a leaky line.
                                     */
                                    if (((pLineObj->lineState.condition & VP_CSLAC_HOOK)
                                      && ((pDevObj->intReg[channelId]) & VP890_HOOK_MASK) != VP890_HOOK_MASK)) {
                                        failureMode = TRUE;
                                    }
                                    VP_HOOK(VpLineCtxType, pLineCtx, ("1. Failure Mode = %d -- Previous %d Current %d",
                                        failureMode,
                                        (pLineObj->lineState.condition & VP_CSLAC_HOOK),
                                        (pDevObj->intReg[channelId]) & VP890_HOOK_MASK));
                                } else {
                                    /*
                                     * If we're not in LP Mode, then the line should be
                                     * detecting off-hook and the signaling bit should
                                     * be high. Otherwise, error.
                                     */
                                    if ((pLineObj->lineState.condition & VP_CSLAC_HOOK)
                                      && (((pDevObj->intReg[channelId]) & VP890_HOOK_MASK) != VP890_HOOK_MASK)) {
                                        failureMode = TRUE;
                                    }
                                    VP_HOOK(VpLineCtxType, pLineCtx, ("2. Failure Mode = %d -- Previous %d Current %d",
                                        failureMode,
                                        (pLineObj->lineState.condition & VP_CSLAC_HOOK),
                                        (pDevObj->intReg[channelId]) & VP890_HOOK_MASK));
                                }
                            }
                        }

                        /*
                         * If the line was last seen off-hook and is now on-hook as a result
                         * of exiting LP Mode, it could be a leaky line.
                         */
                        if (failureMode == TRUE) {
                            if (pLineObj->leakyLineCnt >= 3) {
                                VP_HOOK(VpLineCtxType, pLineCtx, ("Flag Channel %d for Leaky Line at time %d Signaling 0x%02X LineState %d",
                                    channelId, pDevObj->timeStamp, (pDevObj->intReg[channelId] & VP890_HOOK_MASK),
                                    pLineObj->lineState.usrCurrent));

                                pLineObj->status |= VP890_LINE_LEAK;
                                pDevObj->stateInt &= ~(VP890_LINE0_LP | VP890_LINE1_LP);
                                pLineObj->lineEvents.faults |= VP_LINE_EVID_RES_LEAK_FLT;

                                /* Leaky Line Test is complete */
                                pLineObj->lineState.condition &= ~VP_CSLAC_LINE_LEAK_TEST;
                            } else {
                                VP_HOOK(VpLineCtxType, pLineCtx, ("Potential Leaky Line %d at time %d  ...retry",
                                    channelId, pDevObj->timeStamp));

                                /* Continue Leaky Line Test */
                                pLineObj->leakyLineCnt++;


                                /*
                                 * Make sure timer is restarted. This may
                                 * occur as a result of SetLineState(),
                                 * but it may not.
                                 */
                                pDevObj->devTimer[VP_DEV_TIMER_LP_CHANGE] =
                                    MS_TO_TICKRATE(VP890_PWR_SWITCH_DEBOUNCE,
                                        pDevObj->devProfileData.tickRate);
                                pDevObj->devTimer[VP_DEV_TIMER_LP_CHANGE] |=
                                    VP_ACTIVATE_TIMER;
                            }

                            /* Update the line state */
                            for (channelId = 0;
                                 channelId < pDevObj->staticInfo.maxChannels;
                                 channelId++) {
                                Vp890LineObjectType *pLineObjInt;

                                pLineCtx = pDevCtx->pLineCtx[channelId];
                                if (pLineCtx != VP_NULL) {
                                    pLineObjInt = pLineCtx->pLineObj;
                                    if ((pLineObj->termType == VP_TERM_FXS_LOW_PWR) ||
                                        (pLineObj->termType == VP_TERM_FXS_SPLITTER_LP) ||
                                        (pLineObj->termType == VP_TERM_FXS_ISOLATE_LP)) {
                                        VP_HOOK(VpLineCtxType, pLineCtx, ("1. Channel %d Current Linestate %d Current User Linestate %d",
                                            channelId, pLineObjInt->lineState.currentState, pLineObjInt->lineState.usrCurrent));

                                        Vp890SetFxsLineState(pLineCtx, pLineObjInt->lineState.usrCurrent);
                                    }
                                }
                            }
                        } else if (lpTermType == TRUE) {
                            /*
                             * No failure. Recover all hook status, line states
                             * and clear Leaky Line Test Flag
                             */

                            /* Leaky Line Test is complete */
                            pLineObj->lineState.condition &= ~VP_CSLAC_LINE_LEAK_TEST;

                            /*
                             * Low Power Mode exit simply sets the line
                             * to Active in order to maintain smooth
                             * line transients. This step is done to
                             * put the line into the user (API-II)
                             * defined state.
                             */
                            VP_LINE_STATE(VpLineCtxType, pLineCtx, ("LPM Timer: Current %d, User Current %d Channel %d",
                                pLineObj->lineState.currentState,
                                pLineObj->lineState.usrCurrent,
                                channelId));

                            if ((pLineObj->lineState.usrCurrent == VP_LINE_STANDBY)
                              && (!(pLineObj->status & VP890_LOW_POWER_EN))
                              && (pLineObj->calLineData.calDone == TRUE)) {     /* Must not occur during the calibration */
                                uint8 lineState[1] = {VP890_SS_IDLE};

                                VP_LINE_STATE(VpLineCtxType, pLineCtx, ("Setting Channel %d to 0x%02X State at time %d",
                                    channelId, lineState[0], pDevObj->timeStamp));
#ifdef CSLAC_SEQ_EN
                                if ((pLineObj->cadence.status & (VP_CADENCE_STATUS_ACTIVE | VP_CADENCE_STATUS_SENDSIG)) !=
                                    (VP_CADENCE_STATUS_ACTIVE | VP_CADENCE_STATUS_SENDSIG)) {
#endif
                                    VpMpiCmdWrapper(pDevObj->deviceId, pLineObj->ecVal, VP890_SYS_STATE_WRT,
                                        VP890_SYS_STATE_LEN, lineState);
#ifdef CSLAC_SEQ_EN
                                }
#endif
                            }

                            if ((pLineObj->lineState.condition & VP_CSLAC_HOOK)
                                && (pDevObj->intReg[channelId]) & VP890_HOOK_MASK) {

                                if ((pLineObj->lineState.condition & VP_CSLAC_HOOK) &&
                                    (pLineObj->status & VP890_LOW_POWER_EN)) {
                                    /* The line is on-hook */
                                    pLineObj->lineState.condition &= ~VP_CSLAC_HOOK;
                                } else {
                                    /* Valid off-hook */
                                    VP_HOOK(VpLineCtxType, pLineCtx, ("Valid Off-Hook on line %d at time %d",
                                        channelId, pDevObj->timeStamp));

                                    pLineObj->leakyLineCnt = 0;
                                    pLineObj->status &= ~VP890_LINE_LEAK;

                                    pLineObj->dpStruct.hookSt = TRUE;
                                    pLineObj->dpStruct2.hookSt = TRUE;

                                    if(pLineObj->pulseMode != VP_OPTION_PULSE_DECODE_OFF) {
                                        pLineObj->dpStruct.state = VP_DP_DETECT_STATE_LOOP_CLOSE;
                                        pLineObj->dpStruct.lc_time = 0;

                                        pLineObj->dpStruct2.state = VP_DP_DETECT_STATE_LOOP_CLOSE;
                                        pLineObj->dpStruct2.lc_time = 0;
                                    }

                                    pLineObj->lineEvents.signaling |=
                                        VP_LINE_EVID_HOOK_OFF;

                                    pLineObj->lineState.condition |= VP_CSLAC_HOOK;
                                    pLineObj->lineEventHandle = pDevObj->timeStamp;
                                }
                            }
                        }
                    }
                }
            }
        } else if (VP_DEV_TIMER_WB_MODE_CHANGE == devTimer) {
        }
    } /* Loop through all device timers */

    return;
}

/*******************************************************************************
 * Vp890ServiceFxsTimers()
 * This function services FXS-specific line timers.
 *
 * Arguments:   *pDevCtx    -   Device context ptr
 *              *pLineCtx   -   Line context ptr
 *              *pLineObj   -   Line object ptr
 *              deviceId    -   User-defined deviceId
 *              ecVal       -   Enable Channel value including wideband info
 *
 * Preconditions:  Sould be called once per tick by Vp890ApiTick.
 *
 * Postconditions: FXS line timers will be serviced.
 ******************************************************************************/
static void
Vp890ServiceFxsTimers(
    VpDevCtxType            *pDevCtx,
    VpLineCtxType           *pLineCtx,
    Vp890LineObjectType     *pLineObj,
    VpDeviceIdType          deviceId,
    uint8                   ecVal)
{
    Vp890DeviceObjectType   *pDevObj    = pDevCtx->pDevObj;
    VpLineTimerType         timerType;
    uint16                  *pLineTimer;

    for (timerType = 0; timerType < VP_LINE_TIMER_LAST;  timerType++) {
        pLineTimer = &pLineObj->lineTimers.timers.timer[timerType];

        /* if the timer is not active then skip to next timer */
        if (!((*pLineTimer) & VP_ACTIVATE_TIMER)) {
            continue;
        }

        /* get the bits associated only with the time of the timer */
        (*pLineTimer) &= VP_TIMER_TIME_MASK;

        /* decrement the timer */
        if ((*pLineTimer) > 0) {
            (*pLineTimer)--;
        }

        /* if time left on the timer, active it and move on to the next one */
        if (*pLineTimer != 0) {
            *pLineTimer |= VP_ACTIVATE_TIMER;
            continue;
        }

        /*  if any of the timers have expired then serivce them */
        switch(timerType) {
            case VP_LINE_RING_EXIT_DEBOUNCE:
                /*
                 * Allow correcttion of LPM when exiting Ringing due to
                 * initial delay provided in LPM code.
                 */
                if ((pLineObj->termType == VP_TERM_FXS_LOW_PWR) ||
                    (pLineObj->termType == VP_TERM_FXS_ISOLATE_LP) ||
                    (pLineObj->termType == VP_TERM_FXS_SPLITTER_LP)) {
                    Vp890LLSetSysState(deviceId, pLineCtx, 0x00, FALSE);
                }

            case VP_LINE_POLREV_DEBOUNCE:
                pDevObj->status.state |= VP_DEV_FORCE_SIG_READ;
                pDevObj->status.state |= VP_DEV_PENDING_INT;
                break;

            case VP_LINE_DISCONNECT_EXIT:
                if ((pLineObj->termType == VP_TERM_FXS_LOW_PWR) ||
                    (pLineObj->termType == VP_TERM_FXS_ISOLATE_LP) ||
                    (pLineObj->termType == VP_TERM_FXS_SPLITTER_LP)) {
                    VP_LINE_STATE(VpLineCtxType, pLineCtx, ("Disconnect Exit Timer: Setting Ch %d to State 0x%02X at time %d",
                        pLineObj->channelId, pLineObj->nextSlicValue, pDevObj->timeStamp));

                    if (pLineObj->lineState.usrCurrent == VP_LINE_DISCONNECT) {
                        pLineObj->icr2Values[VP890_ICR2_VOC_DAC_INDEX] |= VP890_ICR2_ILA_DAC;
                        pLineObj->icr2Values[VP890_ICR2_VOC_DAC_INDEX] &= ~VP890_ICR2_VOC_DAC_SENSE;
                        pLineObj->icr2Values[VP890_ICR2_VOC_DAC_INDEX+1] &= ~VP890_ICR2_ILA_DAC;
                    } else if (pLineObj->nextSlicValue == VP890_SS_DISCONNECT) {
                        pLineObj->icr2Values[VP890_ICR2_SENSE_INDEX] =
                            (VP890_ICR2_TIP_SENSE | VP890_ICR2_RING_SENSE
                            | VP890_ICR2_ILA_DAC | VP890_ICR2_FEED_SENSE);
                        pLineObj->icr2Values[VP890_ICR2_SENSE_INDEX+1] =
                            (VP890_ICR2_TIP_SENSE | VP890_ICR2_RING_SENSE
                           | VP890_ICR2_FEED_SENSE);
                    }

                    VP_LINE_STATE(VpLineCtxType, pLineCtx, ("Disconnect Exit Timer: ICR2 0x%02X 0x%02X 0x%02X 0x%02X Ch %d",
                        pLineObj->icr2Values[0], pLineObj->icr2Values[1],
                        pLineObj->icr2Values[2], pLineObj->icr2Values[3],
                        pLineObj->channelId));

                    VpMpiCmdWrapper(deviceId, ecVal, VP890_ICR2_WRT, VP890_ICR2_LEN,
                        pLineObj->icr2Values);

                    /*
                     * Release Tip and Ring Bias Override. Set Line Bias to
                     * normal values.
                     */
                    pLineObj->icr1Values[VP890_ICR1_BIAS_OVERRIDE_LOCATION] &=
                        ~VP890_ICR1_TIP_BIAS_OVERRIDE;
                    pLineObj->icr1Values[VP890_ICR1_BIAS_OVERRIDE_LOCATION+1] &=
                        ~(VP890_ICR1_TIP_BIAS_OVERRIDE | VP890_ICR1_LINE_BIAS_OVERRIDE);
                    pLineObj->icr1Values[VP890_ICR1_BIAS_OVERRIDE_LOCATION+1] |=
                        VP890_ICR1_LINE_BIAS_OVERRIDE_NORM;

                    pLineObj->icr1Values[VP890_ICR1_RING_BIAS_OVERRIDE_LOCATION] &=
                        ~VP890_ICR1_RING_BIAS_OVERRIDE;
                    Vp890ProtectedWriteICR1(pLineObj, deviceId,
                        pLineObj->icr1Values);

                    VpMpiCmdWrapper(deviceId, ecVal, VP890_SYS_STATE_WRT,
                        VP890_SYS_STATE_LEN, &pLineObj->nextSlicValue);
                }
                pDevObj->status.state |= VP_DEV_FORCE_SIG_READ;
                pDevObj->status.state |= VP_DEV_PENDING_INT;
                break;

            case VP_LINE_CAL_LINE_TIMER:
                VP_INFO(VpLineCtxType, pLineCtx, ("Running Internal Cal Line"));
                Vp890CalLineInt(pLineCtx);
                break;

            case VP_LINE_OFFHOOK_DELAY:
                VP_INFO(VpLineCtxType, pLineCtx, ("Off-Hook on Line %d at Time %d",
                    pLineObj->channelId, pDevObj->timeStamp));
                pLineObj->lineEvents.signaling |= VP_LINE_EVID_HOOK_OFF;
                pLineObj->lineEventHandle = pDevObj->timeStamp;
                break;

            case VP_LINE_TRACKER_DISABLE: {
                uint8 sysState[VP890_SYS_STATE_LEN] =
                    {VP890_SS_DISCONNECT};

                /* Set line to Disconnect */
                VP_LINE_STATE(VpLineCtxType, pLineCtx, ("Tracker Disable: Setting Ch %d to State 0x%02X at time %d",
                    pLineObj->channelId, pLineObj->nextSlicValue, pDevObj->timeStamp));

                VpMpiCmdWrapper(deviceId, ecVal, VP890_SYS_STATE_WRT,
                    VP890_SYS_STATE_LEN, sysState);

                pLineObj->icr2Values[VP890_ICR2_VOC_DAC_INDEX] |= VP890_ICR2_ILA_DAC;
                pLineObj->icr2Values[VP890_ICR2_VOC_DAC_INDEX] &= ~VP890_ICR2_VOC_DAC_SENSE;
                pLineObj->icr2Values[VP890_ICR2_VOC_DAC_INDEX+1] &= ~VP890_ICR2_ILA_DAC;

                VP_LINE_STATE(VpLineCtxType, pLineCtx, ("Tracker Disable: ICR2 0x%02X 0x%02X 0x%02X 0x%02X Ch %d",
                    pLineObj->icr2Values[0], pLineObj->icr2Values[1],
                    pLineObj->icr2Values[2], pLineObj->icr2Values[3],
                    pLineObj->channelId));

                VpMpiCmdWrapper(deviceId, ecVal, VP890_ICR2_WRT,
                    VP890_ICR2_LEN, pLineObj->icr2Values);
                break;
            }

            case VP_LINE_INTERNAL_TESTTERM_TIMER: {
                /* Apply new bias settings to keep tip/ring near battery. */
                uint8 icr1Reg[VP890_ICR1_LEN];
                icr1Reg[0] = 0xFF;
                icr1Reg[1] = 0x18;
                icr1Reg[2] = 0xFF;
                icr1Reg[3] = 0x04;
                VpMpiCmdWrapper(deviceId, ecVal, VP890_ICR1_WRT, VP890_ICR1_LEN,
                    icr1Reg);
                break;
            }

            default:
                break;
        }
    } /* Loop through all timerTypes for chanID */

    return;
} /* Vp890ServiceFxsTimers() */

/*******************************************************************************
 * Vp890ServiceFxoTimers()
 * This function services FXO-specific line timers.
 *
 * Arguments:   *pDevCtx    -   Device context ptr
 *              *pLineCtx   -   Line context ptr
 *              *pLineObj   -   Line object ptr
 *              deviceId    -   User-defined deviceId
 *              ecVal       -   Enable Channel value including wideband info
 *
 * Preconditions:  Sould be called once per tick by Vp890ApiTick.
 *
 * Postconditions: FXO line timers will be serviced.
 ******************************************************************************/
static void
Vp890ServiceFxoTimers(
    VpDevCtxType            *pDevCtx,
    VpLineCtxType           *pLineCtx,
    Vp890LineObjectType     *pLineObj,
    VpDeviceIdType          deviceId,
    uint8                   ecVal)
{
    Vp890DeviceObjectType   *pDevObj    = pDevCtx->pDevObj;
    VpFXOTimerType          *pFxoTimers = &pLineObj->lineTimers.timers.fxoTimer;
    uint8                   stateByte;

    /* Set tick adder for 1ms increments */
    uint16 tickAdder = pDevObj->devProfileData.tickRate / (VP_CSLAC_TICKSTEP_0_5MS * 2);

    /* Increment the time since polrev was observed */
    if (pFxoTimers->timeLastPolRev < (0x7FFF - tickAdder)) {
        /*
         * The time is in 1mS increments, but the device tickrate is
         * something else. Increment by the scaled amount.
         */
        pFxoTimers->timeLastPolRev += tickAdder;
    } else {
        /* Max limit the value of last polrev value */
        pFxoTimers->timeLastPolRev = 0x7FFF;
    }

    if (!(pLineObj->status & VP890_LINE_IN_CAL)) {
        if ((pFxoTimers->lastStateChange + tickAdder) >= (0x7FFF - tickAdder)) {
            pFxoTimers->lastStateChange = 0x7FFF;
        } else {
            pFxoTimers->lastStateChange += tickAdder;
            /*
             * If we tried to set the line state to loop closed, but it automatically
             *  got kicked back to loop open, set it back to loop closed
             */
            if ((pLineObj->lineState.currentState == VP_LINE_FXO_LOOP_CLOSE
                 || pLineObj->lineState.currentState == VP_LINE_FXO_TALK)
                && pFxoTimers->lastStateChange < (VP890_FXO_OFFHOOK_CHANGE_DEBOUNCE
                 + tickAdder))
            {
                VpMpiCmdWrapper(deviceId, ecVal, VP890_SYS_STATE_RD, VP890_SYS_STATE_LEN, &stateByte);

                if ((stateByte & VP890_SS_FXO_STATE_MASK) != VP890_SS_FXO_OFFHOOK) {
                    /* We think we're offhook, but we really got reset to onhook.
                       Set state back to what we want it to be */
                    Vp890SetFxoLineState(pLineCtx, pLineObj->lineState.currentState);
                }
            }
        }
    }

    if (((uint16)pFxoTimers->ringOffDebounce + tickAdder) > 255) {
        pFxoTimers->ringOffDebounce = 255;
    } else {
        pFxoTimers->ringOffDebounce += tickAdder;
    }

    /* Disconnect Debounce timer */
    if (pFxoTimers->ringOffDebounce > VP890_FXO_RING_OFF_DEBOUNCE_DISC) {
        if (pLineObj->lineTimers.timers.fxoTimer.disconnectDebounce >= 1) {
            pLineObj->lineTimers.timers.fxoTimer.disconnectDebounce -= 1;

            if (pLineObj->lineTimers.timers.fxoTimer.disconnectDebounce == 0) {

                VP_FXO_FUNC(VpLineCtxType, pLineCtx, ("Disconnect debounce timer expired at ts %d PreCond 0x%04X Post 0x%04X",
                    pDevObj->timeStamp, pLineObj->preDisconnect,
                    (pLineObj->lineState.condition & VP_CSLAC_RAW_DISC)));

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
                                    VP_FXO_FUNC(VpLineCtxType, pLineCtx, ("Starting Disconnect Duration with %d at time %d",
                                        pLineObj->lineTimers.timers.fxoTimer.disconnectDuration, pDevObj->timeStamp));
                                }
                                pLineObj->lineTimers.timers.fxoTimer.disconnectDuration =
                                    MS_TO_TICKRATE(VP_FXO_DISC_TO_LOOP_OPEN,
                                        pDevObj->devProfileData.tickRate);
                            } else {
                                if (pLineObj->lineState.condition & VP_CSLAC_DISC) {
                                    pLineObj->lineState.condition &= ~VP_CSLAC_DISC;
                                    pLineObj->lineEvents.fxo |= VP_LINE_EVID_RECONNECT;
                                    VP_FXO_FUNC(VpLineCtxType, pLineCtx, ("Clearing Disconnect Duration at time %d",
                                        pDevObj->timeStamp));
                                }
                                pLineObj->lineTimers.timers.fxoTimer.disconnectDuration = 0;
                            }
                            break;

                        default:
                            if (pLineObj->preDisconnect == VP_CSLAC_RAW_DISC) {
                                if (!(pLineObj->lineState.condition & VP_CSLAC_DISC)) {
                                    pLineObj->lineState.condition |= VP_CSLAC_DISC;
                                    pLineObj->lineEvents.fxo |= VP_LINE_EVID_FEED_DIS;
                                }
                                pLineObj->lineTimers.timers.fxoTimer.disconnectDuration =
                                    MS_TO_TICKRATE(VP_FXO_DISC_TO_LOOP_OPEN,
                                        pDevObj->devProfileData.tickRate);
                                VP_FXO_FUNC(VpLineCtxType, pLineCtx, ("Starting Disconnect Duration with %d at time %d",
                                    pLineObj->lineTimers.timers.fxoTimer.disconnectDuration, pDevObj->timeStamp));
                            } else {
                                if (pLineObj->lineState.condition & VP_CSLAC_DISC) {
                                    pLineObj->lineState.condition &= ~VP_CSLAC_DISC;
                                    pLineObj->lineEvents.fxo |= VP_LINE_EVID_FEED_EN;
                                }
                                pLineObj->lineTimers.timers.fxoTimer.disconnectDuration = 0;
                                VP_FXO_FUNC(VpLineCtxType, pLineCtx, ("Clearing Disconnect Duration at time %d",
                                    pDevObj->timeStamp));
                            }
                            break;
                    }
                }
            }
        }
    }

    /* Line In Use debounce timer */
    if (((uint16)pFxoTimers->liuDebounce + tickAdder) > 255) {
        pFxoTimers->liuDebounce = 255;
    } else {
        pFxoTimers->liuDebounce += tickAdder;
        if (pFxoTimers->liuDebounce >= VP890_FXO_LIU_DEBOUNCE
            && pFxoTimers->liuDebounce < (VP890_FXO_LIU_DEBOUNCE + tickAdder)) {
            /* If this timer expires, we know that the LIU state needs to be
               changed to the opposite of what it currently is, so we use the
               current condition to determine which event we generate. */
            if (pLineObj->lineState.condition & VP_CSLAC_LIU) {
                /* Report the event as long as we're not in disconnect */
                if ((pLineObj->lineState.condition & VP_CSLAC_RAW_DISC)
                    != VP_CSLAC_RAW_DISC) {
                    pLineObj->lineState.condition &= ~VP_CSLAC_LIU;
                    pLineObj->lineEventHandle = pDevObj->timeStamp;
                    pLineObj->lineEvents.fxo |= VP_LINE_EVID_LNIU;
                }
            } else {
                /* OK to report this regardless of disconnect */
                pLineObj->lineState.condition |= VP_CSLAC_LIU;
                pLineObj->lineEventHandle = pDevObj->timeStamp;
                pLineObj->lineEvents.fxo |= VP_LINE_EVID_LIU;
            }
        }
    }

    /* Ringing detection timer */
    /* If this timer expires, ringing has ended. */
    if (((uint16)pFxoTimers->ringTimer + tickAdder) > 255) {
        pFxoTimers->ringTimer = 255;
    } else {
        pFxoTimers->ringTimer += tickAdder;
        if ((pFxoTimers->ringTimer >= VP890_FXO_RING_MAX)
         && (pFxoTimers->ringTimer < (VP890_FXO_RING_MAX + tickAdder))) {
            VP_INFO(VpLineCtxType, pLineCtx, ("Ring Timer %d Expires At %d", pFxoTimers->ringTimer, VP890_FXO_RING_MAX));
            if (pLineObj->lineState.condition & VP_CSLAC_RINGING) {
                pLineObj->lineEventHandle = pDevObj->timeStamp;
                pLineObj->lineEvents.fxo |= VP_LINE_EVID_RING_OFF;
                pLineObj->lineState.condition &= ~VP_CSLAC_RINGING;
                pFxoTimers->ringOffDebounce = 0;
            }
        }
    }

    /* Caller ID Correction Timer */
    if (((uint16)pFxoTimers->cidCorrectionTimer + tickAdder) > 255) {
        pFxoTimers->cidCorrectionTimer = 255;
    } else {
        pFxoTimers->cidCorrectionTimer += tickAdder;

        /* When the timer reaches the CID correction starting time, check some
         * conditions then set the converter to start measuring avg voltage */
        if (pFxoTimers->cidCorrectionTimer >= VP890_FXO_CID_CORRECTION_START
            && pFxoTimers->cidCorrectionTimer < (VP890_FXO_CID_CORRECTION_START + tickAdder))
        {
            /* Have to make sure the channel is not under test, is void of
             * detections, and is in an onhook state. */
            if (!Vp890IsChnlUndrTst(pDevObj, pLineObj->channelId)
                && Vp890IsDevReady(pDevObj->status.state, TRUE)
                && (pLineObj->lineState.condition & (VP_CSLAC_RINGING
                                                    | VP_CSLAC_LIU
                                                    | VP_CSLAC_RAW_DISC
                                                    | VP_CSLAC_RING_AMP_DET)) == 0
                && (pLineObj->lineState.currentState == VP_LINE_FXO_LOOP_OPEN
                 || pLineObj->lineState.currentState == VP_LINE_FXO_OHT))
            {
                /* Start measuring average line voltage */
                uint8 convCfg;
                VpMpiCmdWrapper(deviceId, ecVal, VP890_CONV_CFG_RD,
                    VP890_CONV_CFG_LEN, &convCfg);

                convCfg &= ~VP890_CONV_CONNECT_BITS;
                convCfg |= VP890_AVG_ONHOOK_V;

                VpMpiCmdWrapper(deviceId, ecVal, VP890_CONV_CFG_WRT,
                    VP890_CONV_CFG_LEN, &convCfg);
                pLineObj->cidCorrectionCtr = 0;
            } else {
                /* We can't take the measurement now, so restart the process */
                pFxoTimers->cidCorrectionTimer = 0;
            }
        }
        /* When the timer reaches the CID correction ending time, check the
         * conditions again, then read the avg voltage result and set the gain */
        if (pFxoTimers->cidCorrectionTimer >= VP890_FXO_CID_CORRECTION_END
            && pFxoTimers->cidCorrectionTimer < (VP890_FXO_CID_CORRECTION_END + tickAdder))
        {
            /* Have to make sure the channel is not under test, is void of
             * detections, and is in an onhook state. */
            if (!Vp890IsChnlUndrTst(pDevObj, pLineObj->channelId)
                && Vp890IsDevReady(pDevObj->status.state, TRUE)
                && (pLineObj->lineState.condition & (VP_CSLAC_RINGING
                                                    | VP_CSLAC_LIU
                                                    | VP_CSLAC_RAW_DISC
                                                    | VP_CSLAC_RING_AMP_DET)) == 0
                && (pLineObj->lineState.currentState == VP_LINE_FXO_LOOP_OPEN
                    || pLineObj->lineState.currentState == VP_LINE_FXO_OHT))
            {
                /* Read measurement */
                uint8 data[VP890_TX_PCM_DATA_LEN];
                int8 sample;
                uint16 voltage;
                VpMpiCmdWrapper(deviceId, ecVal, VP890_TX_PCM_DATA_RD,
                        VP890_TX_PCM_DATA_LEN, data);
                /* Measurement data is 7 bits, so we have to sign extend it */
                sample = (data[1] & VP890_FXO_LINE_V_MASK) |
                        ((data[1] & VP890_FXO_LINE_V_SIGN_BIT) << 1);

                /* We use cidCorrectionCtr and cidCorrectionSample to make sure
                 * the line voltage is stable and that we don't make our adjustment
                 * based on a reading that includes some kind of glitch on the line.
                 * We look for 3 successive unchanging samples before making the
                 * correction calculation.
                 */
                if (sample == pLineObj->cidCorrectionSample) {
                    pLineObj->cidCorrectionCtr++;
                } else {
                    pLineObj->cidCorrectionCtr = 1;
                }
                pLineObj->cidCorrectionSample = sample;
                if (pLineObj->cidCorrectionCtr == 3) {
                    /* Measurement is signed and in units of 1.28 volts.  We need an
                     * absolute value in a format where 32 = 1.28V and 25 = 1V */
                    voltage = 32 * (sample < 0 ? -(sample) : sample);
                    CidCorrectionLookup(voltage, &pLineObj->gxCidLevel, &pLineObj->cidDtg);
                    VP_INFO(VpLineCtxType, pLineCtx, ("CallerID Correction - Voltage: %d (%dV)  GX factor: %d  DTG: %d",
                        voltage, voltage/25, pLineObj->gxCidLevel, pLineObj->cidDtg));
                    Vp890SetRelGainInt(pLineCtx);
                } else {
                    /* The timing here COULD skip a sample, but we don't care */
                    pFxoTimers->cidCorrectionTimer = VP890_FXO_CID_CORRECTION_START;
                }
            } else {
                /* We can't take the measurement now, so restart the process */
                pFxoTimers->cidCorrectionTimer = 0;
            }
        }
    }

    /* B Filter Calibration Timer */
    /* When this timer expires, call Vp890CalBFilterInt */
    if (((uint16)pFxoTimers->bCalTimer + tickAdder) > 255) {
        pFxoTimers->bCalTimer = 255;
    } else {
        if ((pFxoTimers->bCalTimer >= VP890_BFILT_SAMPLE_TIME)
         && (pFxoTimers->bCalTimer < (VP890_BFILT_SAMPLE_TIME + tickAdder))) {
            if (pLineObj->status & VP890_LINE_IN_CAL) {
                Vp890CalBFilterInt(pLineCtx);
            }
        }
        pFxoTimers->bCalTimer += tickAdder;
    }

    /* B Filter Measurement Timer */
    /* When this timer expires, call Vp890CalMeasureBFilterInt */
    if (((uint16)pFxoTimers->measureBFilterTimer + tickAdder) > 255) {
        pFxoTimers->measureBFilterTimer = 255;
    } else {
        if ((pFxoTimers->measureBFilterTimer >= VP890_BFILT_SAMPLE_TIME)
         && (pFxoTimers->measureBFilterTimer < (VP890_BFILT_SAMPLE_TIME + tickAdder))) {
            if (pLineObj->status & VP890_LINE_IN_CAL) {
                Vp890CalMeasureBFilterInt(pLineCtx);
            }
        }
        pFxoTimers->measureBFilterTimer += tickAdder;
    }

    if (pFxoTimers->pllRecovery != 0) {
        pFxoTimers->pllRecovery--;
        if (pFxoTimers->pllRecovery == 0) {
            PllRecovery(pLineCtx);
        }
    }

    if (!(pLineObj->status & VP890_LINE_IN_CAL)) {
        /* Current Monitor Timer already set in "ticks" */
        if (pFxoTimers->currentMonitorTimer != 0) {
            pFxoTimers->currentMonitorTimer--;

            if (pFxoTimers->currentMonitorTimer == 0) {
                Vp890FxoLoopCurrentMonitor(pLineCtx);
            }
        }
    }

    if (pLineObj->lineTimers.timers.fxoTimer.disconnectDuration > 0) {
        pLineObj->lineTimers.timers.fxoTimer.disconnectDuration--;
        if (pLineObj->lineTimers.timers.fxoTimer.disconnectDuration == 0) {
            if ((pLineObj->lineState.usrCurrent == VP_LINE_FXO_TALK)
             || (pLineObj->lineState.usrCurrent == VP_LINE_FXO_LOOP_CLOSE)) {

                VP_FXO_FUNC(VpLineCtxType, pLineCtx, ("Disconnect Duration timeout time %d",
                    pDevObj->timeStamp));

                Vp890SetLineState(pLineCtx, VP_LINE_FXO_LOOP_OPEN);
            }
        }
    }

    /* Workaround for low voltage disconnect/LIU distinction */
    if (!(pLineObj->status & VP890_LINE_IN_CAL) &&
          pLineObj->lowVoltageDetection.enabled == TRUE) {
        /* Timer already set in "ticks" */
        if (pFxoTimers->lowVoltageTimer != 0) {
            pFxoTimers->lowVoltageTimer--;

            if (pFxoTimers->lowVoltageTimer == 0) {
                Vp890FxoLowVoltageMonitor(pLineCtx);
            }
        }
    }

    return;
} /* Vp890ServiceFxoTimers() */

/*******************************************************************************
 * Vp890FxoLoopCurrentMonitor()
 *  This function measures the current on the FXO line while in loop close and
 * detecting line current.
 *
 * Preconditions:
 *  None.
 *
 * Postconditions:
 *  Line object is updated with buffer containing loop current values.
 ******************************************************************************/
static void
Vp890FxoLoopCurrentMonitor(
    VpLineCtxType *pLineCtx)
{
    VpDevCtxType            *pDevCtx    = pLineCtx->pDevCtx;
    Vp890DeviceObjectType   *pDevObj    = pDevCtx->pDevObj;
    Vp890LineObjectType     *pLineObj   = pLineCtx->pLineObj;
    uint16                  pcmData;
    uint8                   pcmRegister[VP890_TX_PCM_DATA_LEN];
    uint8                   daaCtrl[VP890_LSD_CTL_LEN];
    uint8                   ecVal       = pLineObj->ecVal;
    VpDeviceIdType          deviceId    = pDevObj->deviceId;

#undef FXO_LOOP_MONITOR_DEBUG

#ifdef FXO_LOOP_MONITOR_DEBUG
    VP_FXO_FUNC(VpLineCtxType, pLineCtx, ("Vp890FXOLoopCurrentMonitor() - state %d at %d (0x%04X)",
        pLineObj->currentMonitor.stateValue, pDevObj->timeStamp, pDevObj->timeStamp));
#endif

    VpMpiCmdWrapper(deviceId, ecVal, VP890_TX_PCM_DATA_RD, VP890_TX_PCM_DATA_LEN,
        pcmRegister);

#ifdef FXO_LOOP_MONITOR_DEBUG
    VP_FXO_FUNC(VpLineCtxType, pLineCtx, ("PCM Data 0x%02X 0x%02X", pcmRegister[0], pcmRegister[1]));
#endif

    pcmData = pcmRegister[0];
    pcmData = (pcmData << 8) & 0xFF00;
    pcmData |= pcmRegister[1];

    switch(pLineObj->currentMonitor.stateValue) {
        case VP890_CURRENT_MONITOR_DISABLED:
            pLineObj->currentMonitor.currentBuffer[0] = (int16)pcmData;

            pLineObj->currentMonitor.offsetMeasurements = 1;

            pLineObj->lineTimers.timers.fxoTimer.currentMonitorTimer =
                MS_TO_TICKRATE(140, pDevObj->devProfileData.tickRate);

            pLineObj->currentMonitor.stateValue = VP890_CURRENT_MONITOR_OFFSET;
            break;

        case VP890_CURRENT_MONITOR_OFFSET: {
            uint8 i;
            int16 min;
            int16 max;

            /* Insert new sample */
            for (i = (VP890_CM_OFFSET_NUM - 1); i > 0; i--) {
                pLineObj->currentMonitor.currentBuffer[i] =
                    pLineObj->currentMonitor.currentBuffer[i - 1];
            }
            pLineObj->currentMonitor.currentBuffer[0] = (int16)pcmData;

            pLineObj->currentMonitor.offsetMeasurements++;

            /* Need to fill up the samples before checking anything */
            if (pLineObj->currentMonitor.offsetMeasurements < VP890_CM_OFFSET_NUM) {
                pLineObj->lineTimers.timers.fxoTimer.currentMonitorTimer =
                    MS_TO_TICKRATE(140, pDevObj->devProfileData.tickRate);

                pLineObj->currentMonitor.stateValue = VP890_CURRENT_MONITOR_OFFSET;
                break;
            }

            /* Find the min/max of the samples */
            min = pLineObj->currentMonitor.currentBuffer[0];
            max = pLineObj->currentMonitor.currentBuffer[0];
            for(i = 1; i < VP890_CM_OFFSET_NUM; i++) {
                if (pLineObj->currentMonitor.currentBuffer[i] < min) {
                    min = pLineObj->currentMonitor.currentBuffer[i];
                }
                if (pLineObj->currentMonitor.currentBuffer[i] > max) {
                    max = pLineObj->currentMonitor.currentBuffer[i];
                }
            }

            VP_FXO_FUNC(VpLineCtxType, pLineCtx, ("Current monitor offset min %d max %d", min, max));

            /* If the range of variation is within our tolerance, use the
             * average as the offset and move on */
            if (max - min < VP890_CM_OFFSET_TOLERANCE &&
                max - min > -VP890_CM_OFFSET_TOLERANCE)
            {
                int32 sum = 0;
                for (i = 0; i < VP890_CM_OFFSET_NUM; i++) {
                    sum += pLineObj->currentMonitor.currentBuffer[i];
                }
                pLineObj->currentMonitor.currentOffset = sum / VP890_CM_OFFSET_NUM;

                VP_FXO_FUNC(VpLineCtxType, pLineCtx, ("Current monitor offset %04hX", pLineObj->currentMonitor.currentOffset));

                /* Re-Enable the Current Sense Path */
                VpMpiCmdWrapper(deviceId, ecVal, VP890_LSD_CTL_RD, VP890_LSD_CTL_LEN, daaCtrl);
                daaCtrl[0] &= ~VP890_LSD_CSE_DIS;
                VpMpiCmdWrapper(deviceId, ecVal, VP890_LSD_CTL_WRT, VP890_LSD_CTL_LEN, daaCtrl);

                pLineObj->lineTimers.timers.fxoTimer.currentMonitorTimer =
                    MS_TO_TICKRATE(400, pDevObj->devProfileData.tickRate);

                pLineObj->currentMonitor.stateValue = VP890_CURRENT_MONITOR_BUFFER0;
                break;
            }

            /*
             * Give up and use the last sample for the offset if we've
             * exceeded the offset sample limit
             */
            if (pLineObj->currentMonitor.offsetMeasurements > VP890_CM_OFFSET_NUM_LIMIT) {

                pLineObj->currentMonitor.currentOffset = (int16)pcmData;

                VP_FXO_FUNC(VpLineCtxType, pLineCtx, ("Current monitor offset didn't converge, using 0"));

                /* Re-Enable the Current Sense Path */
                VpMpiCmdWrapper(deviceId, ecVal, VP890_LSD_CTL_RD, VP890_LSD_CTL_LEN, daaCtrl);
                daaCtrl[0] &= ~VP890_LSD_CSE_DIS;
                VpMpiCmdWrapper(deviceId, ecVal, VP890_LSD_CTL_WRT, VP890_LSD_CTL_LEN, daaCtrl);

                pLineObj->lineTimers.timers.fxoTimer.currentMonitorTimer =
                    MS_TO_TICKRATE(400, pDevObj->devProfileData.tickRate);

                pLineObj->currentMonitor.stateValue = VP890_CURRENT_MONITOR_BUFFER0;
                break;
            }

            /* Otherwise, continue offset measurements */
            pLineObj->lineTimers.timers.fxoTimer.currentMonitorTimer =
                MS_TO_TICKRATE(140, pDevObj->devProfileData.tickRate);

            pLineObj->currentMonitor.stateValue = VP890_CURRENT_MONITOR_OFFSET;
        } break;

        case VP890_CURRENT_MONITOR_BUFFER0:
        case VP890_CURRENT_MONITOR_BUFFER1:
        case VP890_CURRENT_MONITOR_BUFFER2:
        case VP890_CURRENT_MONITOR_BUFFER3:
        case VP890_CURRENT_MONITOR_BUFFER4: {
                uint8 bufferIndex = pLineObj->currentMonitor.stateValue - VP890_CURRENT_MONITOR_BUFFER0;
                pLineObj->currentMonitor.currentBuffer[bufferIndex] = (int16)pcmData;
                pLineObj->currentMonitor.currentBuffer[bufferIndex] -=
                    pLineObj->currentMonitor.currentOffset;

                pLineObj->lineTimers.timers.fxoTimer.currentMonitorTimer =
                    MS_TO_TICKRATE(140, pDevObj->devProfileData.tickRate);

                pLineObj->currentMonitor.stateValue++;
        } break;

        case VP890_CURRENT_MONITOR_NORMAL: {
                uint8 buffCnt;

#ifdef FXO_LOOP_MONITOR_DEBUG
                VP_FXO_FUNC(VpLineCtxType, pLineCtx, ("Vp890FXOLoopCurrentMonitor() - offSet %d",
                    pLineObj->currentMonitor.currentOffset));
#endif

                for (buffCnt = (VP890_PCM_BUF_SIZE - 1); buffCnt > 0; buffCnt--) {
                    pLineObj->currentMonitor.currentBuffer[buffCnt] =
                        pLineObj->currentMonitor.currentBuffer[buffCnt - 1];

#ifdef FXO_LOOP_MONITOR_DEBUG
                    VP_FXO_FUNC(VpLineCtxType, pLineCtx, ("Vp890FXOLoopCurrentMonitor() - Buffer %d, Value %d",
                        buffCnt, pLineObj->currentMonitor.currentBuffer[buffCnt]));
#endif
                }

                pLineObj->currentMonitor.currentBuffer[0] = (int16)pcmData;
                pLineObj->currentMonitor.currentBuffer[0] -=
                    pLineObj->currentMonitor.currentOffset;

#ifdef FXO_LOOP_MONITOR_DEBUG
                VP_FXO_FUNC(VpLineCtxType, pLineCtx, ("Vp890FXOLoopCurrentMonitor() - Buffer 0, Value %d",
                    pLineObj->currentMonitor.currentBuffer[0]));
#endif

                pLineObj->lineTimers.timers.fxoTimer.currentMonitorTimer =
                    MS_TO_TICKRATE(140, pDevObj->devProfileData.tickRate);
            }
            break;
        default:
            break;
    }

    return;
}

/*******************************************************************************
 * Vp890FxoLowVoltageMonitor()
 *  This function measures the voltage on the FXO line while in loop open and
 * gathers information used to adjust the LIU and disconnect detection
 *
 * Preconditions:
 *  This function assumes that the converter config is set to measure
 * average on hook line voltage.  We never set it to anything else while
 * onhook, so this should always be true.
 *
 * Postconditions:
 *  Line object is updated with information for making the LIU and disconnect
 * adjustments.
 ******************************************************************************/
static void
Vp890FxoLowVoltageMonitor(
    VpLineCtxType *pLineCtx)
{
    VpDevCtxType            *pDevCtx    = pLineCtx->pDevCtx;
    Vp890DeviceObjectType   *pDevObj    = pDevCtx->pDevObj;
    Vp890LineObjectType     *pLineObj   = pLineCtx->pLineObj;
    uint8                   ecVal       = pLineObj->ecVal;
    VpDeviceIdType          deviceId    = pDevObj->deviceId;

    uint8 data[VP890_TX_PCM_DATA_LEN];

    VpMpiCmdWrapper(deviceId, ecVal, VP890_TX_PCM_DATA_RD,
            VP890_TX_PCM_DATA_LEN, data);

    if (data[1] == 0) {
        /* Incremement the count of samples indicating true disconnect, and
         * stop counting at the limit. */
        if (pLineObj->lowVoltageDetection.numDisc < VP890_FXO_LOW_VOLTAGE_DISC_COUNT) {
            pLineObj->lowVoltageDetection.numDisc++;
        }

        /* Clear out any count of samples of not-disconnect */
        pLineObj->lowVoltageDetection.numNotDisc = 0;

    } else {
        /* If we're currently indicating disconnect, count the number of samples
         * that indicate NOT disconnect. */
        if (pLineObj->lowVoltageDetection.numDisc >= VP890_FXO_LOW_VOLTAGE_DISC_COUNT) {
            pLineObj->lowVoltageDetection.numNotDisc++;

            /* If this count reaches its limit, clear the disconnect count.
             * Also reset this count because it is only used when the disconnect
             * count is full. */
            if (pLineObj->lowVoltageDetection.numNotDisc >= VP890_FXO_LOW_VOLTAGE_NOTDISC_COUNT) {
                pLineObj->lowVoltageDetection.numDisc = 0;
                pLineObj->lowVoltageDetection.numNotDisc = 0;
                VP_FXO_FUNC(VpLineCtxType, pLineCtx, ("Cleared true disconnect"));
            }
        }
    }

    /* Set the timer for the next measurement. */
    pLineObj->lineTimers.timers.fxoTimer.lowVoltageTimer =
        MS_TO_TICKRATE(VP890_FXO_LOW_VOLTAGE_TIMER_LEN, pDevObj->devProfileData.tickRate);

}

/*******************************************************************************
 * MakeLowVoltageCorrections()
 *  This function makes corrections to the signaling register buffer based on
 * voltage readings made by Vp890FxoLowVoltageMonitor().
 *
 * Preconditions:
 *  None.
 *
 * Postconditions:
 *  The LIU and LDN bits in 'intReg' are adjusted based on voltage measurements.
 ******************************************************************************/
static void
MakeLowVoltageCorrections(
    Vp890LineObjectType *pLineObj,
    uint8 *pIntReg)
{
    if ((*pIntReg & VP890_LDN_MASK) &&
        (pLineObj->lowVoltageDetection.numDisc < VP890_FXO_LOW_VOLTAGE_DISC_COUNT))
    {
        /* The device is indicating LDN, but the voltage measurements have not
         * confirmed a zero voltage.  Instead of 0, we must be seeing some small
         * voltage, so change the bits to indicate LIU instead of disconnect. */
        *pIntReg &= ~VP890_LDN_MASK;
        *pIntReg |= VP890_LIU_MASK;
    }
}

/*******************************************************************************
 * Vp890FindSoftwareInterrupts()
 *  This function checks for active non-masked device and line events.
 *
 * Preconditions:
 *  None.
 *
 * Postconditions:
 *  Returns true if there is an active, non-masked event on either the device
 * or on a line associated with the device.
 ******************************************************************************/
static bool
Vp890FindSoftwareInterrupts(
    VpDevCtxType            *pDevCtx)
{
    Vp890DeviceObjectType   *pDevObj    = pDevCtx->pDevObj;
    Vp890LineObjectType     *pLineObj;
    VpLineCtxType           *pLineCtx;
    uint8                   channelId;
    uint8                   maxChan     = pDevObj->staticInfo.maxChannels;

    VpOptionEventMaskType   eventsMask  = pDevObj->deviceEventsMask;
    VpOptionEventMaskType   *pEvents    = &(pDevObj->deviceEvents);

    /* First clear all device events that are masked */
    pEvents->faults     &= ~(eventsMask.faults);
    pEvents->signaling  &= ~(eventsMask.signaling);
    pEvents->response   &= ~(eventsMask.response);
    pEvents->process    &= ~(eventsMask.process);
    pEvents->test       &= ~(eventsMask.test);
    pEvents->fxo        &= ~(eventsMask.fxo);

    /* Evaluate if any device events remain */
    if (pEvents->faults     ||
        pEvents->signaling  ||
        pEvents->response   ||
        pEvents->process    ||
        pEvents->test       ||
        pEvents->fxo) {
            return TRUE;
    }

    for (channelId = 0; channelId < maxChan; channelId++) {
        pLineCtx = pDevCtx->pLineCtx[channelId];
        if(pLineCtx != VP_NULL) {
            pLineObj = pLineCtx->pLineObj;
            eventsMask = pLineObj->lineEventsMask;
            pEvents = &(pLineObj->lineEvents);

            /* Clear the line events that are masked */
            pEvents->faults     &= ~(eventsMask.faults);
            pEvents->signaling  &= ~(eventsMask.signaling);
            pEvents->response   &= ~(eventsMask.response);
            pEvents->process    &= ~(eventsMask.process);
            pEvents->test       &= ~(eventsMask.test);
            pEvents->fxo        &= ~(eventsMask.fxo);

            /* Clear all FXO events during PLL Recovery */
            if (pLineObj->lineTimers.timers.fxoTimer.pllRecovery) {
                pEvents->fxo = 0;
            }

            /* Evaluate if any line events remain */
            if (pEvents->faults     ||
                pEvents->signaling  ||
                pEvents->response   ||
                pEvents->process    ||
                pEvents->test       ||
                pEvents->fxo) {
                    return TRUE;
            }
        }
    }

    return FALSE;
} /* Vp890FindSoftwareInterrupts() */

/*******************************************************************************
 * Vp890VirtualISR()
 *  This function is called everytime the device causes an interrupt
 *
 * Preconditions
 *  A device interrupt has just occured
 *
 * Postcondition
 *  This function should be called from the each device's ISR.
 *  This function could be inlined to improve ISR performance.
 ******************************************************************************/
VpStatusType
Vp890VirtualISR(
    VpDevCtxType *pDevCtx)
{
    Vp890DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpDeviceIdType deviceId = pDevObj->deviceId;

#if defined (VP890_INTERRUPT_LEVTRIG_MODE)
    VpSysDisableInt(deviceId);
#endif
    /* Device Interrupt Received */
    VpSysEnterCritical(deviceId, VP_CODE_CRITICAL_SEC);
    pDevObj->status.state |= VP_DEV_PENDING_INT;
    VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);

    return VP_STATUS_SUCCESS;
} /* Vp890VirtualISR() */

/*******************************************************************************

* Vp890SetLineState()
 *  This function is the API-II wrapper function for Vp890SetFxsLineState and
 * Vp890SetFxoLineState.
 *
 * Preconditions:
 *  Same as Vp890SetFxsLineState() / Vp890SetFxoLineState()
 *
 * Postconditions:
 *  Same as Vp890SetFxsLineState() / Vp890SetFxoLineState()
 ******************************************************************************/
VpStatusType
Vp890SetLineState(
    VpLineCtxType           *pLineCtx,
    VpLineStateType         state)
{
    Vp890LineObjectType     *pLineObj   = pLineCtx->pLineObj;
    VpStatusType            status;
    VpDevCtxType            *pDevCtx    = pLineCtx->pDevCtx;
    Vp890DeviceObjectType   *pDevObj    = pDevCtx->pDevObj;
    VpDeviceIdType          deviceId    = pDevObj->deviceId;

    VP_API_FUNC(VpLineCtxType, pLineCtx, ("+Vp890SetLineState() State %d", state));

    /* Proceed if device state is either in progress or complete */
    /* Get out if device state is not ready */
    if (!Vp890IsDevReady(pDevObj->status.state, TRUE)) {
        VP_API_FUNC(VpLineCtxType, pLineCtx, ("-Vp890SetLineState() - Device Not Ready"));
        return VP_STATUS_DEV_NOT_INITIALIZED;
    }

    VP_LINE_STATE(VpLineCtxType, pLineCtx, ("Setting Channel %d to State %d",
        pLineObj->channelId, state));

    VpSysEnterCritical(deviceId, VP_CODE_CRITICAL_SEC);

    if(state == pLineObj->lineState.usrCurrent) {
        VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
        VP_API_FUNC(VpLineCtxType, pLineCtx, ("-Vp890SetLineState() - Same State"));
        return VP_STATUS_SUCCESS;
    }

    if (pLineObj->status & VP890_IS_FXO) {
        if (Vp890IsSupportedFxoState(state) == FALSE) {
            VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
            VP_API_FUNC(VpLineCtxType, pLineCtx, ("-Vp890SetLineState() - Unsupported FXO State"));
            return VP_STATUS_INVALID_ARG;
        }
    } else {
        if (Vp890IsSupportedFxsState(state) == FALSE) {
            VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
            VP_API_FUNC(VpLineCtxType, pLineCtx, ("-Vp890SetLineState() - Unsupported FXS State"));
            return VP_STATUS_INVALID_ARG;
        }
    }

    /* Clear the "called from API" flag. This affects the cadencer */
    pLineObj->status &= ~(VP890_SLS_CALL_FROM_API);

    if (pLineObj->status & VP890_IS_FXO) {
#ifdef VP890_FXO_DELAYED_RING_TRIP
        /*
         * If line is FXO and is currently detecting ringing, set a flag to change
         * state after ringing is over.  Also make sure the voicepath is cut off
         * so that the ringing signal won't be sent out as voice
         */
        if (pLineObj->lineState.condition & VP_CSLAC_RINGING) {
            pLineObj->fxoRingStateFlag = 1;
            pLineObj->fxoRingState = state;
            status = VP_STATUS_SUCCESS;
        } else {
            status = Vp890SetFxoLineState(pLineCtx, state);
        }
#else
            status = Vp890SetFxoLineState(pLineCtx, state);
#endif
    } else {
        /*
         * Special FXS handling to prevent setting the line to ringing if
         * off-hook
         */
        if ((pLineObj->lineState.condition & VP_CSLAC_HOOK)
         && ((state == VP_LINE_RINGING_POLREV) || (state == VP_LINE_RINGING))) {
            state = pLineObj->ringCtrl.ringTripExitSt;
        }

        status = Vp890SetFxsLineState(pLineCtx, state);
    }

    if (status == VP_STATUS_SUCCESS) {
        /*
         * Reset the "Count" for leaky line conditions because there are some
         * normal state change conditions that will increment the count, therefore
         * causing exit of LP for non-leaky line
         */
        pLineObj->leakyLineCnt = 0;
        pLineObj->lineState.usrCurrent = state;
    }

    /*
     * Set the "called from API" flag. Convenience for API functions so setting
     * this flag does not need to occur in multiple locations
     */
    pLineObj->status |= VP890_SLS_CALL_FROM_API;

    VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);

    VP_API_FUNC(VpLineCtxType, pLineCtx, ("-Vp890SetLineState()"));

    return status;
} /* Vp890SetLineState() */

/*******************************************************************************
 * Vp890SetFxsLineState()
 *  This function sets the line state for a given channel of a given device. The
 * valid line states are defined in the VpLineState type.
 *
 * Preconditions:
 *  The line must first be initialized prior to setting the line state.  The
 * state must be a valid line state as defined in the VpLineState type.
 *
 * Postconditions:
 *  Returns success code if the channel can be set and the line state is valid
 * for the type of line specified by the line context.  If successfull, the line
 * specified by the line context is set to the line state specified.
 ******************************************************************************/
VpStatusType
Vp890SetFxsLineState(
    VpLineCtxType           *pLineCtx,
    VpLineStateType         state)
{
    VpDevCtxType            *pDevCtx    = pLineCtx->pDevCtx;
    Vp890LineObjectType     *pLineObj   = pLineCtx->pLineObj;
    Vp890DeviceObjectType   *pDevObj    = pDevCtx->pDevObj;
    uint8                   ecVal       = pLineObj->ecVal;

    uint8                   userByte, currentStateByte, mpiData, mpiByte;
    VpStatusType            status;
    VpDeviceIdType          deviceId    = pDevObj->deviceId;
    VpLineStateType         currentState = pLineObj->lineState.currentState;

    bool                    disconnectTimerSet = FALSE;
    bool                    feedToDisable = FALSE;
    bool                    polarityInversion = FALSE;

#ifdef CSLAC_SEQ_EN
    bool                    disableTones = TRUE;
#endif

    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("+Vp890SetFxsLineState()"));
    VP_LINE_STATE(VpLineCtxType, pLineCtx, ("+Vp890SetFxsLineState() State %d", state));

    /*
     * Read the status of the Operating Conditions register so we can change
     * only the TX and RX if the line state is a non-communication mode.
     * This also performs the line type/state verification.
     */
    VpMpiCmdWrapper(deviceId, ecVal, VP890_OP_COND_RD, VP890_OP_COND_LEN, &mpiData);
    VP_INFO(VpLineCtxType, pLineCtx, ("13.a Reading 0x%02X from Operating Conditions",
        mpiData));

    mpiData &= ~(VP890_CUT_TXPATH | VP890_CUT_RXPATH);
    mpiData &= ~(VP890_HIGH_PASS_DIS | VP890_OPCOND_RSVD_MASK);

    status = Vp890GetTxRxPcmMode(pLineObj, state, &mpiByte);
    if (status == VP_STATUS_SUCCESS) {
        mpiData |= mpiByte;
    } else {
        VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("-Vp890SetFxsLineState() FAILURE"));
        VP_LINE_STATE(VpLineCtxType, pLineCtx, ("-Vp890SetFxsLineState() FAILURE"));
        return status;
    }

    VP_LINE_STATE(VpLineCtxType, pLineCtx, ("13.b Writing 0x%02X to Operating Conditions",
        mpiData));
    VpMpiCmdWrapper(deviceId, ecVal, VP890_OP_COND_WRT, VP890_OP_COND_LEN,
        &mpiData);

#ifdef CSLAC_SEQ_EN
    /* We're no longer in the middle of a time function */
    pLineObj->cadence.status &= ~VP_CADENCE_STATUS_MID_TIMER;
    pLineObj->cadence.timeRemain = 0;
#endif

    /*
     * If this function is called by the application, stop the cadencer and
     * reset the Loop Supervision if it is incorrect. Disable all tones if going
     * to a state that does not support tone generation.
     */
    if (!(pLineObj->status & VP890_SLS_CALL_FROM_API)) {
#ifdef CSLAC_SEQ_EN
        /* If we're in the middle of active cadence, terminate it */
        if ((pLineObj->cadence.status & VP_CADENCE_STATUS_METERING)
         && (pLineObj->cadence.status & VP_CADENCE_STATUS_ACTIVE)) {
            pLineObj->lineEvents.process |= VP_LINE_EVID_MTR_ABORT;
            pLineObj->processData = pLineObj->cadence.meteringBurst;
        }
#endif
        /* Correct the loop supervision if currently incorrect. */
        if (pLineObj->status & VP890_BAD_LOOP_SUP) {
            pLineObj->status &= ~(VP890_BAD_LOOP_SUP);
            VpMpiCmdWrapper(deviceId, ecVal, VP890_LOOP_SUP_WRT,
                VP890_LOOP_SUP_LEN, pLineObj->loopSup);
        }

        /* Disable tones and cadencing if going to a state that prevents it */
        switch(state) {
            case VP_LINE_STANDBY:
            case VP_LINE_DISCONNECT:
            case VP_LINE_RINGING:
            case VP_LINE_RINGING_POLREV:
            case VP_LINE_STANDBY_POLREV:
                /*
                 * Disable signal generator A/B/C/D before making any changes and stop
                 * previous cadences
                 */
                pLineObj->sigGenCtrl[0] &= ~VP890_GEN_ALLON;
                pLineObj->sigGenCtrl[0] &= ~VP890_GEN_BIAS;
                VpMpiCmdWrapper(deviceId, ecVal, VP890_GEN_CTRL_WRT, VP890_GEN_CTRL_LEN,
                    pLineObj->sigGenCtrl);
                break;

            default:
                /* Stop also if coming from Ringing */
                if ((pLineObj->lineState.usrCurrent != VP_LINE_RINGING) &&
                    (pLineObj->lineState.usrCurrent != VP_LINE_RINGING_POLREV)) {
#ifdef CSLAC_SEQ_EN
                    /* Keep tones/cadencing running */
                    disableTones = FALSE;
#endif
                }
                break;
        }

#ifdef CSLAC_SEQ_EN
        if (disableTones == TRUE) {
            pLineObj->cadence.status = VP_CADENCE_RESET_VALUE;
            pLineObj->cadence.pActiveCadence = VP_PTABLE_NULL;
        }

        /*  If the user is changing the line state, we should stop callerId */
        if (pLineObj->callerId.status & VP_CID_IN_PROGRESS) {
            VpCliStopCli(pLineCtx);
        }
#endif
    }

    userByte = LineStateMap(state);

    /* Modify userByte depending on the current polarity */
    VpMpiCmdWrapper(deviceId, ecVal, VP890_SYS_STATE_RD, VP890_SYS_STATE_LEN,
        &currentStateByte);
#ifdef CSLAC_SEQ_EN
    if (pLineObj->cadence.pActiveCadence != VP_NULL) {
        if ((pLineObj->cadence.status &
            (VP_CADENCE_STATUS_ACTIVE | VP_CADENCE_STATUS_IGNORE_POLARITY)) ==
            (VP_CADENCE_STATUS_ACTIVE | VP_CADENCE_STATUS_IGNORE_POLARITY)) {

            userByte &= ~VP890_SS_POLARITY_MASK;
            userByte |= (currentStateByte & VP890_SS_POLARITY_MASK);
        }
    }
#endif
    if ((state == VP_LINE_RINGING) || (state == VP_LINE_RINGING_POLREV)
     || (pLineObj->lineState.currentState == VP_LINE_RINGING)
     || (pLineObj->lineState.currentState == VP_LINE_RINGING_POLREV)) {
        /*
         * Set State Ringing Returns TRUE if the line has NOT actually been
         * changed. So return at this point results in preventing the
         * internal line state values to be updated until the line is
         * changed by the cadencer.
         */
        if (Vp890SetStateRinging(pLineCtx, state) == TRUE) {
            VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp890SetLineStateInt-"));
            return VP_STATUS_SUCCESS;
        }
    }

    if (((pLineObj->termType == VP_TERM_FXS_LOW_PWR) ||
         (pLineObj->termType == VP_TERM_FXS_ISOLATE_LP) ||
         (pLineObj->termType == VP_TERM_FXS_SPLITTER_LP))
        && (pLineObj->lineTimers.timers.timer[VP_LINE_DISCONNECT_EXIT]
            & VP_ACTIVATE_TIMER)) {
        pLineObj->nextSlicValue = userByte;
    }

    /*
     * Enable Disconnect Recovery time for hook status if going FROM
     * Disconnect to a state that can detect off-hook
     */
    if (pLineObj->lineState.currentState == VP_LINE_DISCONNECT) {
        /* Coming from Disconnect...*/
        switch (state) {
            case VP_LINE_DISCONNECT:
                break;

            default:
                /* ..going to a state that can detect feed */
                if (!(pLineObj->lineState.condition & VP_CSLAC_LINE_LEAK_TEST)) {
                    VP_LINE_STATE(VpLineCtxType, pLineCtx, ("Setting Disconnect Recovery Timer on channel %d at time %d status 0x%04X",
                        pLineObj->channelId, pDevObj->timeStamp, pLineObj->lineState.condition));

                    pLineObj->lineTimers.timers.timer[VP_LINE_DISCONNECT_EXIT] =
                        MS_TO_TICKRATE(VP_DISCONNECT_RECOVERY_TIME,
                            pDevObj->devProfileData.tickRate);

                    pLineObj->lineTimers.timers.timer[VP_LINE_DISCONNECT_EXIT] |=
                        VP_ACTIVATE_TIMER;

                    disconnectTimerSet = TRUE;
                }
                break;
        }
    }

    /*
     * Set the disable flag if disabling feed. This prevents the
     * polrev timer from starting.
     */
    if (state == VP_LINE_DISCONNECT) {
        feedToDisable = TRUE;
    }

    /*
     * Set Polarity Reverse timer if the SLIC is changing polarity. Exclude
     * Disconnect type recovery conditions since a timer is set above for
     * those conditions.
     */
    if ((currentStateByte & VP890_SS_POLARITY_MASK)
     != (userByte & VP890_SS_POLARITY_MASK)) {
        /*
         * Set this to mark the condition that requires loading of new
         * calibration coefficients.
         */
        polarityInversion = TRUE;

        /*
         * Timer is set if not exiting disconnect and staying in feed
         * conditions.
         */
        if ((disconnectTimerSet == FALSE) && (feedToDisable == FALSE)) {
            VP_LINE_STATE(VpLineCtxType, pLineCtx, ("Setting Polarity Reversal Timer on channel %d at time %d status 0x%04X",
                pLineObj->channelId, pDevObj->timeStamp, pLineObj->lineState.condition));

            pLineObj->lineTimers.timers.timer[VP_LINE_POLREV_DEBOUNCE] =
                 MS_TO_TICKRATE(VP_POLREV_DEBOUNCE_TIME,
                     pDevObj->devProfileData.tickRate);

             pLineObj->lineTimers.timers.timer[VP_LINE_POLREV_DEBOUNCE]
                 |= VP_ACTIVATE_TIMER;
        }
    }

    if (pLineObj->calLineData.calDone == TRUE) {
        if ((polarityInversion == TRUE) ||
            (pLineObj->calLineData.forceCalDataWrite == TRUE)) {
            pLineObj->calLineData.forceCalDataWrite = FALSE;

            if (userByte & VP890_SS_POLARITY_MASK) {
                VpMpiCmdWrapper(deviceId, ecVal, VP890_DC_FEED_WRT,
                    VP890_DC_FEED_LEN, pLineObj->calLineData.dcFeedPr);
            } else {
                VpMpiCmdWrapper(deviceId, ecVal, VP890_DC_FEED_WRT,
                    VP890_DC_FEED_LEN, pLineObj->calLineData.dcFeed);
            }
        }
    }

    /*
     * Update the line object previous and current line state variables.
     * Note that this is not updated with ringing cadence until the line is
     * actually set to ringing (i.e., not when the application sets the line
     * to ringing with (possibly) a non-ringing state via the ringing
     * cadence.
     */
    pLineObj->lineState.previous = currentState;
    pLineObj->lineState.currentState = state;

    if (((pLineObj->termType == VP_TERM_FXS_LOW_PWR) ||
         (pLineObj->termType == VP_TERM_FXS_ISOLATE_LP) ||
         (pLineObj->termType == VP_TERM_FXS_SPLITTER_LP))
         /*
          * If going from/to Disconnect/Standby in LPM, states are the same
          * so don't continue.
          */
     && ((state == VP_LINE_DISCONNECT) || (currentState == VP_LINE_DISCONNECT))) {
        if (currentState == state) {
            VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("-Vp890SetFxsLineState() SAME STATE"));
            return VP_STATUS_SUCCESS;
        } else {
            if (state == VP_LINE_DISCONNECT) {
                Vp890RunLPDisc(pLineCtx, TRUE, userByte);
            } else {
                Vp890RunLPDisc(pLineCtx, FALSE, userByte);
            }
        }
    } else {
        Vp890GroundStartProc(FALSE, pLineCtx, currentStateByte, userByte);
    }

    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("-Vp890SetFxsLineState() NORMAL"));
    VP_LINE_STATE(VpLineCtxType, pLineCtx, ("-Vp890SetFxsLineState() NORMAL"));

    return VP_STATUS_SUCCESS;
} /* SetFxsLineState() */

/**
 * Vp890IsSupportedFxsState()
 *  This function checks to see if the state passed is a supproted FXS state of
 * the 890 API
 *
 * Preconditions:
 *  None.
 *
 * Postconditions:
 *  None.
 */
bool
Vp890IsSupportedFxsState(
    VpLineStateType state)
{
    switch (state) {
        case VP_LINE_STANDBY:
        case VP_LINE_ACTIVE:
        case VP_LINE_ACTIVE_POLREV:
        case VP_LINE_TALK:
        case VP_LINE_TALK_POLREV:
        case VP_LINE_OHT:
        case VP_LINE_OHT_POLREV:
        case VP_LINE_DISCONNECT:
        case VP_LINE_RINGING:
        case VP_LINE_RINGING_POLREV:
        case VP_LINE_STANDBY_POLREV:
            return TRUE;

        default:
            return FALSE;
    }
}

/**
 * Vp890IsSupportedFxoState()
 *  This function checks to see if the state passed is a supproted FXO state of
 * the 890 API
 *
 * Preconditions:
 *  None.
 *
 * Postconditions:
 *  None.
 */
bool
Vp890IsSupportedFxoState(
    VpLineStateType state)
{
    switch (state) {
        case VP_LINE_FXO_OHT:
        case VP_LINE_FXO_LOOP_OPEN:
        case VP_LINE_FXO_LOOP_CLOSE:
        case VP_LINE_FXO_TALK:
            return TRUE;

        default:
            return FALSE;
    }
}

/**
 * Vp890RunLPDisc()
 *  This function implements the Disconnect Enter/Exit for Low Power Mode.
 *
 * Preconditions:
 *  None. Calling function must know that this code should execute.
 *
 * Postconditions:
 *  Initial procedures are performed and timers set to enter or exit Disconnect
 * state for Low Power termination type.
 */
void
Vp890RunLPDisc(
    VpLineCtxType *pLineCtx,
    bool discMode,
    uint8 nextSlicByte)
{
    Vp890LineObjectType     *pLineObj   = pLineCtx->pLineObj;
    uint8                   ecVal       = pLineObj->ecVal;

    VpDevCtxType            *pDevCtx    = pLineCtx->pDevCtx;
    Vp890DeviceObjectType   *pDevObj    = pDevCtx->pDevObj;
    VpDeviceIdType          deviceId    = pDevObj->deviceId;
    uint8                   channelId   = pLineObj->channelId;

    /*
     * myDevState "set" means *this* line is in a LPM state prior to calling
     * this function.
     */
    Vp890DeviceStateIntType myDevState = (channelId == 0) ?
        (pDevObj->stateInt & VP890_LINE0_LP) :
        (pDevObj->stateInt & VP890_LINE1_LP);

    bool hookStatus;
    Vp890LineStatusType leakyLine = pLineObj->status & VP890_LINE_LEAK;

    /*
     * Enter/Exit Disconnect uses Active (w/Polarity) to cause fast charge or
     * discharge of the line.
     */
    uint8 lineState[]  = {VP890_SS_ACTIVE};

    VpCSLACGetLineStatus(pLineCtx, VP_INPUT_RAW_HOOK, &hookStatus);
    leakyLine = (hookStatus == FALSE) ? leakyLine : VP890_LINE_LEAK ;

    VP_LINE_STATE(VpLineCtxType, pLineCtx, ("+Vp890RunLPDisc() NextByte 0x%02X", nextSlicByte));

    if ((discMode == TRUE)
     || ((nextSlicByte == VP890_SS_IDLE) && (!(leakyLine)))) {
        /*
         * Either entering Disconnect or Exiting into Standby. Either case, set
         * floor voltage to -70V for LPM T/R voltage.
         */
        uint8 swParams[VP890_REGULATOR_PARAM_LEN];
        VpMpiCmdWrapper(deviceId, ecVal, VP890_REGULATOR_PARAM_RD,
            VP890_REGULATOR_PARAM_LEN, swParams);
        swParams[VP890_FLOOR_VOLTAGE_BYTE] &= ~VP890_FLOOR_VOLTAGE_MASK;
        swParams[VP890_FLOOR_VOLTAGE_BYTE] |= 0x0D;   /* 70V */
        VpMpiCmdWrapper(deviceId, ecVal, VP890_REGULATOR_PARAM_WRT,
            VP890_REGULATOR_PARAM_LEN, swParams);
    }

    if (discMode == TRUE) {        /* Entering Disconnect */
        VP_LINE_STATE(VpLineCtxType, pLineCtx, ("Entering Disconnect on Chan %d time %d",
            channelId, pDevObj->timeStamp));

        /*
         * There are two cases to consider when entering Disconnect:
         *     1. This line is coming from LPM-Standby
         *     2. This line is coming from a non-LPM state
         *
         *  All cases require the ICR values modified to disable the switcher.
         */

        /*
         * Step 1: Program all ICR registers including Disable Switcher. Note
         * these are writing to cached values only, not to the device -- yet.
         */
        Vp890SetLPRegisters(pDevObj, pLineObj, TRUE);
        pLineObj->icr2Values[VP890_ICR2_SENSE_INDEX] &= ~VP890_ICR2_ILA_DAC;
        pLineObj->icr2Values[VP890_ICR2_SENSE_INDEX] |= VP890_ICR2_VOC_DAC_SENSE;
        pLineObj->icr2Values[VP890_ICR2_SENSE_INDEX+1] &= ~VP890_ICR2_VOC_DAC_SENSE;

        pLineObj->icr2Values[VP890_ICR2_SWY_CTRL_INDEX] |= VP890_ICR2_SWY_CTRL_EN;
        pLineObj->icr2Values[VP890_ICR2_SWY_CTRL_INDEX+1] &= ~VP890_ICR2_SWY_CTRL_EN;

        VP_LINE_STATE(VpLineCtxType, pLineCtx, ("Entering Disconnect: Channel %d: Writing ICR2 0x%02X 0x%02X 0x%02X 0x%02X",
            pLineObj->channelId,
            pLineObj->icr2Values[0], pLineObj->icr2Values[1],
            pLineObj->icr2Values[2], pLineObj->icr2Values[3]));

        VpMpiCmdWrapper(deviceId, ecVal, VP890_ICR2_WRT, VP890_ICR2_LEN,
            pLineObj->icr2Values);

        /*
         * Force Tip and Ring Bias Override and set values to max. This forces
         * values toward ground. Temporarily disable line bias override until
         * after the line state has been set.
         */
        pLineObj->icr1Values[VP890_ICR1_BIAS_OVERRIDE_LOCATION] |=
            VP890_ICR1_TIP_BIAS_OVERRIDE;
        pLineObj->icr1Values[VP890_ICR1_BIAS_OVERRIDE_LOCATION+1] |=
            VP890_ICR1_TIP_BIAS_OVERRIDE;
        pLineObj->icr1Values[VP890_ICR1_BIAS_OVERRIDE_LOCATION] &=
            ~VP890_ICR1_LINE_BIAS_OVERRIDE;

        pLineObj->icr1Values[VP890_ICR1_RING_BIAS_OVERRIDE_LOCATION] |=
            VP890_ICR1_RING_BIAS_OVERRIDE;
        pLineObj->icr1Values[VP890_ICR1_RING_BIAS_OVERRIDE_LOCATION+1] |=
            VP890_ICR1_RING_BIAS_OVERRIDE;

        Vp890ProtectedWriteICR1(pLineObj, deviceId, pLineObj->icr1Values);

        if (myDevState) {
            /*
             * State is ACITVE coming from Standy. No need to modify.
             */
        } else {
            /* State is Polarity conditioned coming from any other state. */
            VpMpiCmdWrapper(deviceId, ecVal, VP890_SYS_STATE_RD, VP890_SYS_STATE_LEN,
                lineState);

            lineState[0] &= ~VP890_SS_LINE_FEED_MASK;
            lineState[0] |= VP890_SS_ACTIVE;
        }

        VpMpiCmdWrapper(deviceId, ecVal, VP890_SYS_STATE_WRT, VP890_SYS_STATE_LEN,
            lineState);

        /* Restore line bias override */
        pLineObj->icr1Values[VP890_ICR1_BIAS_OVERRIDE_LOCATION] |=
            VP890_ICR1_LINE_BIAS_OVERRIDE;
        pLineObj->icr1Values[VP890_ICR1_BIAS_OVERRIDE_LOCATION+1] |=
            VP890_ICR1_LINE_BIAS_OVERRIDE;
        Vp890ProtectedWriteICR1(pLineObj, deviceId, pLineObj->icr1Values);

        VpMpiCmdWrapper(deviceId, ecVal, VP890_ICR3_WRT, VP890_ICR3_LEN,
            pLineObj->icr3Values);

        VpMpiCmdWrapper(deviceId, ecVal, VP890_ICR4_WRT, VP890_ICR4_LEN,
            pLineObj->icr4Values);

        pLineObj->nextSlicValue = VP890_SS_DISCONNECT;

        /* Set Discharge Time based on Supply Configuration. */
        pLineObj->lineTimers.timers.timer[VP_LINE_DISCONNECT_EXIT] =
            (MS_TO_TICKRATE(Vp890SetDiscTimers(pDevObj),
            pDevObj->devProfileData.tickRate)) | VP_ACTIVATE_TIMER;

    } else {    /* Exiting Disconnect */
        VP_LINE_STATE(VpLineCtxType, pLineCtx, ("Recovering Chan %d from DISCONNECT at time %d with value 0x%02X",
            pLineObj->channelId, pDevObj->timeStamp, nextSlicByte));

        /*
         * There are two cases to consider when exiting Disconnect:
         *     1. This line is going to VP_LINE_STANDBY.
         *     2. This line is going to non-LPM state.
         */
        pLineObj->icr2Values[VP890_ICR2_VOC_DAC_INDEX] &= ~VP890_ICR2_VOC_DAC_SENSE;
        pLineObj->icr2Values[VP890_ICR2_SWY_CTRL_INDEX] &= ~VP890_ICR2_SWY_CTRL_EN;
        pLineObj->icr2Values[VP890_ICR2_SENSE_INDEX] &= ~VP890_ICR2_ILA_DAC;

        /*
         * Disable Tip and Ring bias control completely. Set SLIC bias back
         * to normal values.
         */
        pLineObj->icr1Values[VP890_ICR1_BIAS_OVERRIDE_LOCATION] &=
            ~(VP890_ICR1_TIP_BIAS_OVERRIDE);
        pLineObj->icr1Values[VP890_ICR1_BIAS_OVERRIDE_LOCATION+1] &=
            ~(VP890_ICR1_TIP_BIAS_OVERRIDE | VP890_ICR1_LINE_BIAS_OVERRIDE);
        pLineObj->icr1Values[VP890_ICR1_BIAS_OVERRIDE_LOCATION+1] |=
            (VP890_ICR1_LINE_BIAS_OVERRIDE_NORM);
        pLineObj->icr1Values[VP890_ICR1_RING_BIAS_OVERRIDE_LOCATION] &=
            ~VP890_ICR1_RING_BIAS_OVERRIDE;

        if ((nextSlicByte == VP890_SS_IDLE) && (!(leakyLine))) {
            /*
             * 1. This line is going to LPM-VP_LINE_STANDBY.
             *      Step 1: Enable Switcher and set to 70V floor.
             *      Step 2: Set line to Active (for fast charge)
             *      Step 3: Set line to SLIC-Disconnect (end of timer)
             */
            Vp890SetLPRegisters(pDevObj, pLineObj, TRUE);

            pLineObj->icr2Values[VP890_ICR2_SENSE_INDEX] =
                (VP890_ICR2_TIP_SENSE | VP890_ICR2_RING_SENSE);

            VP_LINE_STATE(VpLineCtxType, pLineCtx, ("Case 1: Channel %d: Writing ICR2 0x%02X 0x%02X 0x%02X 0x%02X",
                pLineObj->channelId,
                pLineObj->icr2Values[0], pLineObj->icr2Values[1],
                pLineObj->icr2Values[2], pLineObj->icr2Values[3]));

            VpMpiCmdWrapper(deviceId, ecVal, VP890_ICR2_WRT, VP890_ICR2_LEN,
                pLineObj->icr2Values);

            VP_LINE_STATE(VpLineCtxType, pLineCtx, ("Case 1: Setting Ch %d to State 0x%02X at time %d",
                channelId, lineState[0], pDevObj->timeStamp));

            pLineObj->nextSlicValue = VP890_SS_DISCONNECT;

            VP_LINE_STATE(VpLineCtxType, pLineCtx, ("Disconnect Exit: Channel %d: State 0x%02X",
                pLineObj->channelId, lineState[0]));

            VpMpiCmdWrapper(deviceId, ecVal, VP890_SYS_STATE_WRT, VP890_SYS_STATE_LEN,
                lineState);

            VP_LINE_STATE(VpLineCtxType, pLineCtx, ("Disconnect Exit: Channel %d: Writing ICR1 0x%02X 0x%02X 0x%02X 0x%02X",
                pLineObj->channelId,
                pLineObj->icr1Values[0], pLineObj->icr1Values[1],
                pLineObj->icr1Values[2], pLineObj->icr1Values[3]));

            Vp890ProtectedWriteICR1(pLineObj, deviceId, pLineObj->icr1Values);

            /*
             * This prevents LPM Exit Code from running since it is no longer
             * necessary.
             */
            pLineObj->status |= VP890_LOW_POWER_EN;
            pDevObj->stateInt |= (VP890_LINE0_LP | VP890_LINE1_LP);
        } else {
            /*
             * 2. This line is going to non-LPM state, so take care of all LPM
             *    exit handling here.
             *      Step 1: Enable Switcher on this line
             *      Step 2: Set line to Active w/Polarity (for fast charge)
             *      Step 3: Set remaining ICR registers for LPM exit settings.
             *      Step 4: Set line to new SLIC state (end of timer)
             */
            Vp890SetLPRegisters(pDevObj, pLineObj, FALSE);   /* ICR3 and 4 correct */

            /* Update state to match target polarity. */
            lineState[0] |= (VP890_SS_POLARITY_MASK & nextSlicByte);
            pLineObj->nextSlicValue = nextSlicByte;

            /* Restore Supply voltage. */
            VpMpiCmdWrapper(deviceId, ecVal, VP890_REGULATOR_PARAM_WRT,
                VP890_REGULATOR_PARAM_LEN, pDevObj->devProfileData.swParams);

            /* Run LPM Exit Sequence */
            Vp890WriteLPExitRegisters(pLineCtx, deviceId, ecVal, lineState);

            /*
             * This prevents LPM Exit Code from running since it is no longer
             * necessary.
             */
            pLineObj->status &= ~VP890_LOW_POWER_EN;
            pDevObj->stateInt &= ~(VP890_LINE0_LP | VP890_LINE1_LP);
        }

        /*
         * Disable LPM Exit sequence if for some reason it was previously started
         * (e.g., application quickly went from Standby-OHT-Disconnect). When
         * the Tracker Disable Time expires, it will set the line to Disconnect
         * and Disable the Tracker (ICR2). While some of the steps may be what
         * we're also trying to do, Disconnect handling is done with the
         * Disconnect Exit (which also is Disoconnect Enter) timer.
         */
        pLineObj->lineTimers.timers.timer[VP_LINE_TRACKER_DISABLE]
            &= ~VP_ACTIVATE_TIMER;
    }

    VP_LINE_STATE(VpLineCtxType, pLineCtx, ("-Vp890RunLPDisc()"));
}

/**
 * Vp890GroundStartProc()
 *  This function implements enter and exit from Ground Start procedures
 *
 * Preconditions:
 *  None. Calling function must know that this code should execute.
 *
 * Postconditions:
 *  Procedures according to operational notes are implemented for enter/exit
 * a ground start state. A timer is set when exiting ground start to delay
 * disable of DC bias.
 */
void
Vp890GroundStartProc(
    bool gsMode,
    VpLineCtxType *pLineCtx,
    uint8 currentLineState,
    uint8 userByte)
{
    Vp890LineObjectType     *pLineObj   = pLineCtx->pLineObj;
    uint8                   ecVal       = pLineObj->ecVal;

    VpDevCtxType            *pDevCtx    = pLineCtx->pDevCtx;
    Vp890DeviceObjectType   *pDevObj    = pDevCtx->pDevObj;
    VpDeviceIdType          deviceId    = pDevObj->deviceId;
    uint8                   beforeState = (currentLineState & VP890_SS_LINE_FEED_MASK);
    uint8                   afterState  =  (userByte & VP890_SS_LINE_FEED_MASK);

    VP_LINE_STATE(VpLineCtxType, pLineCtx, ("+Vp890GroundStartProc() userByte 0x%02X", userByte));

    if (((pLineObj->termType == VP_TERM_FXS_LOW_PWR) ||
         (pLineObj->termType == VP_TERM_FXS_ISOLATE_LP) ||
         (pLineObj->termType == VP_TERM_FXS_SPLITTER_LP)) &&
        (pLineObj->status & VP890_LOW_POWER_EN)) {
        VP_LINE_STATE(VpLineCtxType, pLineCtx, ("Adjusting LP Mode on channel %d, NOT writing to device",
            pLineObj->channelId));
        Vp890LLSetSysState(deviceId, pLineCtx, userByte, FALSE);
    } else {
        VP_LINE_STATE(VpLineCtxType, pLineCtx, ("Adjusting LP Mode on channel %d, writing to device",
            pLineObj->channelId));
        Vp890LLSetSysState(deviceId, pLineCtx, userByte, TRUE);
    }

    if ((pLineObj->termType != VP_TERM_FXS_LOW_PWR) &&
        (pLineObj->termType != VP_TERM_FXS_ISOLATE_LP) &&
        (pLineObj->termType != VP_TERM_FXS_SPLITTER_LP)) {
        if (beforeState != afterState) {
            bool writeIcrReg = FALSE;
            if (beforeState == VP890_SS_DISCONNECT) {
                pLineObj->icr1Values[VP890_ICR1_BIAS_OVERRIDE_LOCATION] &=
                    ~0x0F;

                pLineObj->icr2Values[VP890_ICR2_SENSE_INDEX] &=
                    ~(VP890_ICR2_DAC_SENSE | VP890_ICR2_FEED_SENSE);
                writeIcrReg = TRUE;
            } else if (afterState == VP890_SS_DISCONNECT) {
                pLineObj->icr1Values[VP890_ICR1_BIAS_OVERRIDE_LOCATION] |=
                    0x0F;
                pLineObj->icr1Values[VP890_ICR1_BIAS_OVERRIDE_LOCATION+1] &= 0xF8;
                pLineObj->icr1Values[VP890_ICR1_BIAS_OVERRIDE_LOCATION+1] |= 0x08;

                pLineObj->icr2Values[VP890_ICR2_SENSE_INDEX] |=
                    (VP890_ICR2_DAC_SENSE | VP890_ICR2_FEED_SENSE);
                pLineObj->icr2Values[VP890_ICR2_SENSE_INDEX+1] &= ~VP890_ICR2_DAC_SENSE;
                pLineObj->icr2Values[VP890_ICR2_SENSE_INDEX+1] |= VP890_ICR2_FEED_SENSE;
                writeIcrReg = TRUE;
            }

            if (writeIcrReg == TRUE) {
                VP_LINE_STATE(VpLineCtxType, pLineCtx, ("Ground Start Exit (disconnect): Write ICR1 Before %d After %d 0x%02X 0x%02X 0x%02X 0x%02X Ch %d",
                    beforeState, afterState,
                    pLineObj->icr1Values[0], pLineObj->icr1Values[1],
                    pLineObj->icr1Values[2], pLineObj->icr1Values[3],
                    pLineObj->channelId));

                VP_LINE_STATE(VpLineCtxType, pLineCtx, ("Ground Start Exit (disconnect): Write ICR2 Before %d After %d 0x%02X 0x%02X 0x%02X 0x%02X Ch %d",
                    beforeState, afterState,
                    pLineObj->icr2Values[0], pLineObj->icr2Values[1],
                    pLineObj->icr2Values[2], pLineObj->icr2Values[3],
                    pLineObj->channelId));

                Vp890ProtectedWriteICR1(pLineObj, deviceId, pLineObj->icr1Values);
                VpMpiCmdWrapper(deviceId, ecVal, VP890_ICR2_WRT, VP890_ICR2_LEN,
                    pLineObj->icr2Values);
            }
        }
    }
    VP_LINE_STATE(VpLineCtxType, pLineCtx, ("-Vp890GroundStartProc()"));

}

/*******************************************************************************
 * SetFxoLineState()
 *  This function sets the line state for a given channel of a given device. The
 * valid line states are defined in the VpLineState type.
 *
 * Preconditions:
 *  The line must first be initialized prior to setting the line state.  The
 * state must be a valid line state as defined in the VpLineState type.
 *
 * Postconditions:
 *  Returns success code if the channel can be set and the line state is valid
 * for the type of line specified by the line context.  If successfull, the line
 * specified by the line context is set to the line state specified.
 ******************************************************************************/
VpStatusType
Vp890SetFxoLineState(
    VpLineCtxType           *pLineCtx,
    VpLineStateType         state)
{
    VpDevCtxType            *pDevCtx  = pLineCtx->pDevCtx;
    Vp890DeviceObjectType   *pDevObj  = pDevCtx->pDevObj;
    VpDeviceIdType          deviceId  = pDevObj->deviceId;
    Vp890LineObjectType     *pLineObj = pLineCtx->pLineObj;

    VpStatusType            status;
    uint8                   chanId    = pLineObj->channelId;
    uint8                   ecVal     = pLineObj->ecVal;
    uint8                   stateByte, stateBytePre;
    uint8                   opCondData, opCondByte, gainData, opFuncData;
    uint8                   intData[VP890_INT_MASK_LEN];
    uint8                   daaCtrl[VP890_LSD_CTL_LEN];

    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("+Vp890SetFxoLineState()"));

    VpMpiCmdWrapper(deviceId, ecVal, VP890_INT_MASK_RD, VP890_INT_MASK_LEN, intData);
    VpMpiCmdWrapper(deviceId, ecVal, VP890_SYS_STATE_RD, VP890_SYS_STATE_LEN, &stateByte);
    stateBytePre = stateByte;
    stateByte &= ~(VP890_SS_FXO_STATE_MASK | VP890_SS_SPE_MASK);

    /* If the line state is set to OHT, we must enable CID gain correction and
     * disable the programmed X filter */
    VpMpiCmdWrapper(deviceId, ecVal, VP890_VP_GAIN_RD, VP890_VP_GAIN_LEN, &gainData);
    gainData &= ~VP890_CIDCOR_MASK;
    VpMpiCmdWrapper(deviceId, ecVal, VP890_OP_FUNC_RD, VP890_OP_FUNC_LEN, &opFuncData);
    opFuncData |= VP890_ENABLE_X;

    /*
     * Read the status of the Operating Conditions register so we can change
     * only the TX and RX if the line state is a non-communication mode.
     * This also performs the line type/state verification.
     */
    VpMpiCmdWrapper(deviceId, ecVal, VP890_OP_COND_RD, VP890_OP_COND_LEN, &opCondData);
    opCondData &= ~(VP890_CUT_TXPATH | VP890_CUT_RXPATH);
    opCondData &= ~(VP890_HIGH_PASS_DIS | VP890_OPCOND_RSVD_MASK);
    status = Vp890GetTxRxPcmMode(pLineObj, state, &opCondByte);
    if (status == VP_STATUS_SUCCESS) {
        opCondData |= opCondByte;
    } else {
        return status;
    }

    /* Does the CODEC have to be in Active for simple on/off-hook? */
    switch(state) {
        case VP_LINE_FXO_OHT:
            stateByte |= VP890_SS_SPE_MASK;
            gainData |= VP890_CIDCOR_MASK;
            opFuncData &= ~VP890_ENABLE_X;
        case VP_LINE_FXO_LOOP_OPEN:
            /* The LIU and RING_DET interrupt masks share the same bits as POH
             * and POH_SIGN.  They will be masked when the FXO goes offhook, so
             * we need to restore them for LIU and RING events when onhook. */
            intData[chanId] &= ~VP890_LIU_MASK;
            intData[chanId] &= ~VP890_RING_DET_MASK;
            if ( (pLineObj->lineEventsMask.fxo & VP_LINE_EVID_LIU) &&
                 (pLineObj->lineEventsMask.fxo & VP_LINE_EVID_LNIU) ) {
                intData[chanId] |= VP890_LIU_MASK;
            }
            if ( (pLineObj->lineEventsMask.fxo & VP_LINE_EVID_RING_ON) &&
                 (pLineObj->lineEventsMask.fxo & VP_LINE_EVID_RING_OFF) ) {
                intData[chanId] |= VP890_RING_DET_MASK;
            }

            stateByte |= VP890_SS_FXO_ONHOOK;
            VP_LINE_STATE(VpLineCtxType, pLineCtx, ("Vp890SetFxoLineState() - Onhook at %d (0x%04X)",
                pDevObj->timeStamp, pDevObj->timeStamp));
            /*-----------------7/24/2008 6:18PM-----------------
             * Disable the current monitor and restore DAA configuration if
             * on-hook occurs in the middle of calibration phase.
             * --------------------------------------------------*/
            pLineObj->lineTimers.timers.fxoTimer.currentMonitorTimer = 0;
            if (pLineObj->currentMonitor.stateValue == VP890_CURRENT_MONITOR_OFFSET ||
                pLineObj->currentMonitor.stateValue == VP890_CURRENT_MONITOR_DISABLED)
            {
                VpMpiCmdWrapper(deviceId, ecVal, VP890_LSD_CTL_RD, VP890_LSD_CTL_LEN, daaCtrl);
                daaCtrl[0] &= ~VP890_LSD_CSE_DIS;
                VpMpiCmdWrapper(deviceId, ecVal, VP890_LSD_CTL_WRT, VP890_LSD_CTL_LEN, daaCtrl);
            }
            pLineObj->currentMonitor.stateValue = VP890_CURRENT_MONITOR_DISABLED;

            /* Turn on the low voltage monitor if needed */
            if (pLineObj->lowVoltageDetection.enabled == TRUE) {
                pLineObj->lowVoltageDetection.numDisc = 0;
                pLineObj->lowVoltageDetection.numNotDisc = 0;
                pLineObj->lineTimers.timers.fxoTimer.lowVoltageTimer =
                    MS_TO_TICKRATE(VP890_FXO_LOW_VOLTAGE_TIMER_LEN, pDevObj->devProfileData.tickRate);
                VP_FXO_FUNC(VpLineCtxType, pLineCtx, ("Enable low voltage timer %d\n",
                    pLineObj->lineTimers.timers.fxoTimer.lowVoltageTimer));
            }
            break;

        case VP_LINE_FXO_LOOP_CLOSE:
        case VP_LINE_FXO_TALK:
            /* Mask the POH and POH_SIGN bits because we don't use them, and
             * they can toggle frequently, causing excessive interrupts */
            intData[chanId] |= VP890_POH_SIGN_MASK;
            intData[chanId] |= VP890_POH_DET_MASK;

            stateByte |= VP890_SS_FXO_OFFHOOK;

            /*-----------------7/24/2008 6:23PM-----------------
             * Start calibration phase of current monitor.
             * --------------------------------------------------*/
            VpMpiCmdWrapper(deviceId, ecVal, VP890_LSD_CTL_RD, VP890_LSD_CTL_LEN, daaCtrl);
            daaCtrl[0] |= VP890_LSD_CSE_DIS;
            VpMpiCmdWrapper(deviceId, ecVal, VP890_LSD_CTL_WRT, VP890_LSD_CTL_LEN, daaCtrl);

            pLineObj->lineTimers.timers.fxoTimer.currentMonitorTimer =
                MS_TO_TICKRATE(500, pDevObj->devProfileData.tickRate);
            pLineObj->currentMonitor.currentOffset = 0;

            /* Turn off the low voltage monitor */
            pLineObj->lineTimers.timers.fxoTimer.lowVoltageTimer = 0;

            VP_LINE_STATE(VpLineCtxType, pLineCtx, ("Vp890SetFxoLineState() - Offhook at %d (0x%04X), timer %d",
                pDevObj->timeStamp, pDevObj->timeStamp, pLineObj->lineTimers.timers.fxoTimer.currentMonitorTimer));
         break;

        case VP_LINE_FXO_RING_GND:
        default:
            VP_ERROR(VpLineCtxType, pLineCtx, ("Vp890SetFxoLineState() - Invalid FXO State"));
            return VP_STATUS_INVALID_ARG;
    }
    VP_LINE_STATE(VpLineCtxType, pLineCtx, ("14. Writing 0x%02X to Operating Conditions",
        opCondData));
    VpMpiCmdWrapper(deviceId, ecVal, VP890_OP_COND_WRT, VP890_OP_COND_LEN, &opCondData);

    VpMpiCmdWrapper(deviceId, ecVal, VP890_VP_GAIN_WRT, VP890_VP_GAIN_LEN, &gainData);

    VP_LINE_STATE(VpLineCtxType, pLineCtx, ("FXO Set Line State Operating Functions 0x%02X",
        opFuncData));
    VpMpiCmdWrapper(deviceId, ecVal, VP890_OP_FUNC_WRT, VP890_OP_FUNC_LEN, &opFuncData);

    /* Start the lastStateChange timer and set the line state */
    VpMpiCmdWrapper(deviceId, ecVal, VP890_SYS_STATE_WRT, VP890_SYS_STATE_LEN, &stateByte);
    if (state == VP_LINE_FXO_OHT) {
        /* For OHT, the SS register needs to be written twice to force SPE=1.
           The device state machine forces SPE=0 when it transitions between
           major states, so writing this a second time will override that. */
        VpMpiCmdWrapper(deviceId, ecVal, VP890_SYS_STATE_WRT, VP890_SYS_STATE_LEN, &stateByte);
    }
    VpMpiCmdWrapper(deviceId, ecVal, VP890_INT_MASK_WRT, VP890_INT_MASK_LEN, intData);

    pLineObj->lineTimers.timers.fxoTimer.lastStateChange = 0;

    pLineObj->lineState.previous = pLineObj->lineState.currentState;
    pLineObj->lineState.currentState = state;

    /* This will apply the callerID gain correction factor if we changed to
     * an onhook state, or remove it if we changed to an offhook state. */
    Vp890SetRelGainInt(pLineCtx);

    if ((stateBytePre & VP890_SS_FXO_STATE_BITS) == VP890_SS_FXO_DISCONNECT) {
        /*
         * Kick off the FXO Disconnect to OHT workaround. If coming from
         * Disconnect, the next state must be OHT -- unless someone accessed
         * the device directly. Then, they're on their own...
         */
       if (pLineObj->lineTimers.timers.fxoTimer.pllRecovery == 0) {
            /* Start at very next tick */
           pLineObj->lineTimers.timers.fxoTimer.pllRecovery = 1;
           pLineObj->pllRecoveryState = VP890_PLL_RECOVERY_ST_DISABLE;
       }
    }

    return VP_STATUS_SUCCESS;
} /* SetFxoLineState() */

/**
 * Vp890SetDiscTimers()
 *  This function provides the value for Disconnect Timing using LPM.
 *
 * Preconditions:
 *  None.
 *
 * Postconditions:
 *  None. Returns a value in ms.
 */
uint16
Vp890SetDiscTimers(
    Vp890DeviceObjectType *pDevObj)
{
    return VP890_FIXED_TRACK_DISABLE_TIME;
}

/*******************************************************************************
 * Vp890SetStateRinging()
 *  This function starts cadence and non-cadence ringing, as well as ringing
 * exit.
 *
 * Preconditions:
 *  None. Calling function must know that this code should execute.
 *
 * Postconditions:
 *  Line object is modified if ringing cadence or exiting (timers). If not
 * cadencing and ringing is started, then return TRUE. Otherwise return FALSE.
 ******************************************************************************/
static bool
Vp890SetStateRinging(
    VpLineCtxType           *pLineCtx,
    VpLineStateType         state)
{
    Vp890LineObjectType     *pLineObj       = pLineCtx->pLineObj;
    uint8                   ecVal           = pLineObj->ecVal;

    VpDevCtxType            *pDevCtx        = pLineCtx->pDevCtx;
    Vp890DeviceObjectType   *pDevObj        = pDevCtx->pDevObj;
    VpDeviceIdType          deviceId        = pDevObj->deviceId;
    uint8                   tempRingCtrlData = VP890_GEN_CTRL_EN_BIAS;

    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("+Vp890SetStateRinging()"));

    /*
     * No matter what the current line state is,
     * go to ringing if the input state is ringing
     */
    if ((state == VP_LINE_RINGING) || (state == VP_LINE_RINGING_POLREV)) {
#ifdef CSLAC_SEQ_EN
        VpProfilePtrType pProfile;
        pLineObj->cadence.pActiveCadence = pLineObj->pRingingCadence;
        pProfile = pLineObj->cadence.pActiveCadence;
        /*
         * We're entering a ringing state, so determine if we need to
         * cadence. If we're not cadencing, this is "always on", so we can
         * disable the currently active cadence sequence and immediately
         * implement the ringing state change.
         */
        if (pProfile == VP_PTABLE_NULL) {
            pLineObj->cadence.status = VP_CADENCE_RESET_VALUE;
        } else if (!(pLineObj->cadence.status & VP_CADENCE_STATUS_ACTIVE)) {
            /*
             * We have a non-null cadence and the candence is still active.
             * If the cadence was not previously started, then start it here
             * and let the sequencer take over. Otherwise, the sequencer was
             * previously started and this state change is at the request of
             * the sequencer.
             */

            /* We have a cadence and are just starting it */
            pLineObj->cadence.status |= VP_CADENCE_STATUS_ACTIVE;
            pLineObj->cadence.index = VP_PROFILE_TYPE_SEQUENCER_START;
            pLineObj->cadence.pCurrentPos = &pProfile[VP_PROFILE_TYPE_SEQUENCER_START];
            pLineObj->cadence.length = pProfile[VP_PROFILE_LENGTH];
            pLineObj->cadence.status &= ~VP_CADENCE_STATUS_IGNORE_POLARITY;
            pLineObj->cadence.status |= (pProfile[VP_PROFILE_MPI_LEN] & 0x01) ?
                VP_CADENCE_STATUS_IGNORE_POLARITY : 0;

            /* Nullify any internal sequence so that the API doesn't think that
             * an internal sequence of some sort is running */
            pLineObj->intSequence[VP_PROFILE_TYPE_LSB] = VP_PRFWZ_PROFILE_NONE;

            return TRUE;

        }
        /*
         * If we're in an active Ringing Cadence, and ready to go into the
         * Ringing state, generate the Ringing Event and indicate that this
         * is the Ringing On event
         */
        if (pLineObj->cadence.status & VP_CADENCE_STATUS_ACTIVE) {
            if (pProfile[VP_PROFILE_TYPE_LSB] == VP_PRFWZ_PROFILE_RINGCAD) {
                pLineObj->lineEvents.process |= VP_LINE_EVID_RING_CAD;
                pLineObj->processData = VP_RING_CAD_MAKE;
            }
        }
#endif
        /*
         * Cadencing already called or null cadence.  We're ready to set
         * the line to the Ringing State but we have to first make sure
         * that the signal generator parameters in the device are setup
         * for the ringing profile
         */
        tempRingCtrlData &= ~VP890_GEN_BIAS;
        VpMpiCmdWrapper(deviceId, ecVal, VP890_GEN_CTRL_WRT,
            VP890_GEN_CTRL_LEN, &tempRingCtrlData);

        if (pLineObj->ringingParams != VP_PTABLE_NULL) {
            int16 biasErr;
            uint8 bias[VP890_RINGER_PARAMS_LEN];

            VpMemCpy(bias, pLineObj->ringingParams, VP890_RINGER_PARAMS_LEN);
            biasErr = (int16)((((uint16)(pLineObj->ringingParams[1]) << 8) & 0xFF00) +
                ((uint16)(pLineObj->ringingParams[2]) & 0x00FF));
            if (state == VP_LINE_RINGING) {
                /* Normal polarity */
                biasErr -= ((pDevObj->vp890SysCalData.sigGenAError[pLineObj->channelId][0] -
                    pDevObj->vp890SysCalData.vocOffset[pLineObj->channelId][VP890_NORM_POLARITY]) *
                    16 / 10);
            } else {
                /* Reverse polarity */
                biasErr += ((pDevObj->vp890SysCalData.sigGenAError[pLineObj->channelId][0] -
                    pDevObj->vp890SysCalData.vocOffset[pLineObj->channelId][VP890_REV_POLARITY]) *
                    16 / 10);
            }
            bias[1] = (uint8)((biasErr >> 8) & 0x00FF);
            bias[2] = (uint8)(biasErr & 0x00FF);

            VpMpiCmdWrapper(deviceId, ecVal, VP890_RINGER_PARAMS_WRT,
                VP890_RINGER_PARAMS_LEN, bias);
        }
    } else if ((VP_LINE_RINGING_POLREV == pLineObj->lineState.currentState) ||
               (VP_LINE_RINGING        == pLineObj->lineState.currentState)) {
        /*
         * If we are currently in a ringing state and are trying to set the device
         * into a non-ringing state, the external capacitance of the line
         * may cause false hook events once the state transition is made.
         * In order to avoid false hooks, we set up the ring exit hook debounce timer.
         */
        if (!(pLineObj->lineState.condition & VP_CSLAC_HOOK)) {

            pLineObj->lineTimers.timers.timer[VP_LINE_RING_EXIT_DEBOUNCE] =
                MS_TO_TICKRATE(pLineObj->ringCtrl.ringExitDbncDur / 8,
                    pDevObj->devProfileData.tickRate);

            if (pLineObj->ringCtrl.ringExitDbncDur) {
                pLineObj->lineTimers.timers.timer[VP_LINE_RING_EXIT_DEBOUNCE]
                    |= VP_ACTIVATE_TIMER;
            }
        } else {
            /* If ringtrip occurs (ringing then offhook), debounce the hook bit */
            pLineObj->lineTimers.timers.timer[VP_LINE_RING_EXIT_DEBOUNCE] =
                MS_TO_TICKRATE(VP890_RING_TRIP_DEBOUNCE, pDevObj->devProfileData.tickRate);
            pLineObj->lineTimers.timers.timer[VP_LINE_RING_EXIT_DEBOUNCE]
                |= VP_ACTIVATE_TIMER;
        }

#ifdef CSLAC_SEQ_EN
        /*
         * If we're in an active Ringing Cadence, and ready to go into another
         * state, generate the Ringing Event and indicate that this is the
         * Ringing Off event
         */
        if (pLineObj->cadence.status & VP_CADENCE_STATUS_ACTIVE) {
            VpProfilePtrType pProfile = pLineObj->cadence.pActiveCadence;
            if (pProfile[VP_PROFILE_TYPE_LSB] == VP_PRFWZ_PROFILE_RINGCAD) {
                pLineObj->lineEvents.process |= VP_LINE_EVID_RING_CAD;
                pLineObj->processData = VP_RING_CAD_BREAK;
            }
        }
#endif
    }

    return FALSE;
} /* Vp890SetStateRinging() */

/*******************************************************************************
 * Vp890LLSetSysState()
 *  This function writes to the System State register and based on the state
 * being set, determines if low power mode can or cannot be used, or if the line
 * is recovering from Disconnect to a Feed state. In the latter case, a timer
 * is set to transition through Tip Open first to prevent "ping" of the phone.
 * In this case, the final state is set when the timer expires.
 *
 * Preconditions:
 *  This function is called internally by the API-II only.
 *
 * Postconditions:
 *  The System State Register is updated with either the value passed, or
 * Tip Open. If low power mode termination type exists, a flag in the device
 * object indicating that low power can or cannot be used is modified. If coming
 * out of Disconnect, Tip Open is written and a timer is set.
 ******************************************************************************/
void
Vp890LLSetSysState(
    VpDeviceIdType deviceId,
    VpLineCtxType *pLineCtx,
    uint8 lineState,
    bool writeToDevice)
{
    Vp890LineObjectType *pLineObj = VP_NULL;
    uint8 ecVal;
    VpDevCtxType *pDevCtx = VP_NULL;
    Vp890DeviceObjectType *pDevObj = VP_NULL;
    uint8 lineStatePre[VP890_SYS_STATE_LEN];
    bool lineIsFxs = FALSE;
    uint8 channelId;

    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("+Vp890LLSetSysState() SlicState 0x%02X Write %d",
        lineState, writeToDevice));

    if (pLineCtx != VP_NULL) {
        pLineObj = pLineCtx->pLineObj;
        pDevCtx = pLineCtx->pDevCtx;
        pDevObj = pDevCtx->pDevObj;
        ecVal = pLineObj->ecVal;
        channelId = pLineObj->channelId;

        VpMpiCmdWrapper(deviceId, ecVal, VP890_SYS_STATE_RD, VP890_SYS_STATE_LEN,
            lineStatePre);

        if (!(pLineObj->status & VP890_IS_FXO)) {
            lineIsFxs = TRUE;
            if ((Vp890IsChnlUndrTst(pDevObj, channelId) == TRUE) ||
                (pLineObj->status & VP890_LINE_IN_CAL)) {
                pDevObj->stateInt &= ~(VP890_LINE0_LP | VP890_LINE1_LP);
                VP_LINE_STATE(VpLineCtxType, pLineCtx, ("3. Clearing LP flag for channel %d CurrentState %d UserState %d",
                    channelId, pLineObj->lineState.currentState,
                    pLineObj->lineState.usrCurrent));
            } else {
                bool lpTermType = FALSE;
                bool hookStatus = FALSE;

                if (((pLineObj->termType == VP_TERM_FXS_LOW_PWR) ||
                     (pLineObj->termType == VP_TERM_FXS_ISOLATE_LP) ||
                     (pLineObj->termType == VP_TERM_FXS_SPLITTER_LP))) {
                    lpTermType = TRUE;
                    VpCSLACGetLineStatus(pLineCtx, VP_INPUT_RAW_HOOK, &hookStatus);
                }
                hookStatus = (pLineObj->lineState.currentState == VP_LINE_STANDBY) ? hookStatus : FALSE;

                if ((lpTermType == TRUE) && (hookStatus == FALSE) && (!(pLineObj->status & VP890_LINE_LEAK))) {
                    if ((pLineObj->lineState.currentState == VP_LINE_DISCONNECT) ||
                        (pLineObj->lineState.currentState == VP_LINE_STANDBY)) {

                        if (((lineStatePre[0] & VP890_SS_LINE_FEED_MASK) != VP890_SS_FEED_BALANCED_RINGING)
                         && ((lineStatePre[0] & VP890_SS_LINE_FEED_MASK) != VP890_SS_FEED_UNBALANCED_RINGING)) {
                            pDevObj->stateInt |= (VP890_LINE0_LP | VP890_LINE1_LP);
                            VP_LINE_STATE(VpLineCtxType, pLineCtx,("1. 890-Setting LP flag for channel %d",channelId));
                        } else {
                            VP_LINE_STATE(VpLineCtxType, pLineCtx,("1. Delay Setting LP flag for channel %d",channelId));
                        }
                    } else {
                        pDevObj->stateInt &= ~(VP890_LINE0_LP | VP890_LINE1_LP);
                        VP_LINE_STATE(VpLineCtxType, pLineCtx,("1. Clearing LP flag for channel %d Device Status 0x%04X",
                            channelId, pDevObj->stateInt));
                    }
                } else {
                    pDevObj->stateInt &= ~(VP890_LINE0_LP | VP890_LINE1_LP);
                    VP_LINE_STATE(VpLineCtxType, pLineCtx, ("2. Clearing LP flag for channel %d Status 0x%04X",
                        channelId, pLineObj->status));
                }
            }
        }
    } else {
        ecVal = VP890_EC_CH1;
        channelId = 0;

        VpMpiCmdWrapper(deviceId, ecVal, VP890_SYS_STATE_RD, VP890_SYS_STATE_LEN,
            lineStatePre);
    }

    /* Device Write Override: setting flags without a device write */
    if (writeToDevice == FALSE) {
        VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("-Vp890LLSetSysState()"));
        VP_LINE_STATE(VpLineCtxType, pLineCtx, ("-Vp890LLSetSysState()"));
        return;
    }

    if (((lineStatePre[0] & VP890_SS_LINE_FEED_MASK) == VP890_SS_DISCONNECT)
      && (lineIsFxs == TRUE)) {
        pLineObj->nextSlicValue = lineState;

        VP_LINE_STATE(VpLineCtxType, pLineCtx, ("2. Setting Chan %d to System State 0x%02X at Time %d",
            channelId, lineState, pDevObj->timeStamp));

        VpMpiCmdWrapper(deviceId, ecVal, VP890_SYS_STATE_WRT, VP890_SYS_STATE_LEN,
            &lineState);
    } else {
        VP_LINE_STATE(VpLineCtxType, pLineCtx, ("1. Setting Chan %d to System State 0x%02X at Time %d",
            channelId, lineState, pDevObj->timeStamp));

        VpMpiCmdWrapper(deviceId, ecVal, VP890_SYS_STATE_WRT, VP890_SYS_STATE_LEN,
            &lineState);
    }

    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("-Vp890LLSetSysState()"));
}

/*******************************************************************************
 * LineStateMap
 *  Locally used function by SetFxsLineState and SetFxoLineState to map an API
 * line state to a register-level line state
 *
 * Preconditions:
 *  None. State to byte mapping only.
 *
 * Postconditions:
 *  Returns the byte that should be used in the device System State register
 * for the API State passed.
 ******************************************************************************/
static uint8
LineStateMap(
    VpLineStateType state)
{
    VP_API_FUNC_INT(None, VP_NULL, ("+LineStateMap()"));

    switch(state) {
        case VP_LINE_STANDBY:
            return VP890_SS_IDLE;
        case VP_LINE_TIP_OPEN:
            return VP890_SS_TIP_OPEN;

        case VP_LINE_ACTIVE:
        case VP_LINE_TALK:
        case VP_LINE_OHT:
            return VP890_SS_ACTIVE;

        case VP_LINE_ACTIVE_POLREV:
        case VP_LINE_TALK_POLREV:
        case VP_LINE_OHT_POLREV:
            return VP890_SS_ACTIVE_POLREV;

        case VP_LINE_STANDBY_POLREV:
            return VP890_SS_IDLE_POLREV;

        case VP_LINE_DISCONNECT:
            return VP890_SS_DISCONNECT;
        case VP_LINE_RINGING:
            return VP890_SS_BALANCED_RINGING;

        case VP_LINE_RINGING_POLREV:
            return VP890_SS_BALANCED_RINGING_PR;

        case VP_LINE_FXO_OHT:
        case VP_LINE_FXO_LOOP_OPEN:
            return VP890_SS_FXO_ONHOOK;
        case VP_LINE_FXO_LOOP_CLOSE:
        case VP_LINE_FXO_TALK:
            return VP890_SS_FXO_OFFHOOK;

        default:
            return VP890_SS_DISCONNECT;
    }
} /* LineStateMap() */

/*******************************************************************************
 * Vp890SetLineTone()
 *  This function sets the line tone with the cadence specified on the line.
 *
 * Preconditions:
 *  The line must first be initialized.
 *
 * Postconditions:
 *  The tone specified by the tone profile is sent on the line at the cadence
 * specified by the cadence profile.  If the tone is NULL, all line tones are
 * removed.  If the cadence is NULL, the cadence is set to "Always On".  This
 * function returns the success code if the tone cadence is a valid tone cadence
 * and the tone profile is a valid tone profile, or in the case where the user
 * passes in profile indexes, if the tone/cadence indexes are within the range
 * of the device.
 ******************************************************************************/
VpStatusType
Vp890SetLineTone(
    VpLineCtxType       *pLineCtx,
    VpProfilePtrType    pToneProfile,  /**< A pointer to a tone profile, or an
                                        * index into the profile table for the tone
                                        * to put on the line.
                                        */
    VpProfilePtrType    pCadProfile,   /**< A pointer to a tone cadence profile, or
                                        * an index into the profile table for the
                                        * tone cadence to put on the line.
                                        */
    VpDtmfToneGenType   *pDtmfControl) /**< Indicates to send a DTMF tone
                                        * (either upstream or downstream) if
                                        * this parameter is not VP_NULL AND
                                        * the tone specified is VP_PTABLE_NULL
                                        */
{
    Vp890LineObjectType   *pLineObj = pLineCtx->pLineObj;
    VpDevCtxType          *pDevCtx  = pLineCtx->pDevCtx;
    Vp890DeviceObjectType *pDevObj  = pDevCtx->pDevObj;
    VpProfilePtrType      pToneProf = VP_PTABLE_NULL;
    VpProfilePtrType      pCadProf  = VP_PTABLE_NULL;

    VpDigitType           digit     = VP_DIG_NONE;
    VpDirectionType       direction = VP_DIRECTION_INVALID;

    uint8                 ecVal     = pLineObj->ecVal;

    uint8 opCond, sigGenABCount, mpiByte;
    uint8 converterCfg[VP890_CONV_CFG_LEN];

    /* Initialize SigGen A/B values to 0 */
    uint8 sigGenAB[VP890_SIGAB_PARAMS_LEN] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    uint8 sigGenABOffset = 3;  /* Starting point after filler bytes */

    int toneIndex = VpGetProfileIndex(pToneProfile);
    int cadenceIndex = VpGetProfileIndex(pCadProfile);
    uint16 profIndex;   /* Used for bit masking the profile index table */

    VpDeviceIdType deviceId = pDevObj->deviceId;

    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("+Vp890SetLineTone()"));

    /* Get out if device state is not ready */
    if (!Vp890IsDevReady(pDevObj->status.state, TRUE)) {
        return VP_STATUS_DEV_NOT_INITIALIZED;
    }

    /*
     * Get Profile Index returns -1 if the profile passed is a pointer or
     * of VP_PTABLE_NULL type. Otherwise it returns the index
     */

    /* Verify a good profile (index or pointer) for the tone */
    if (toneIndex < 0) {
        /*
         * A pointer is passed or VP_PTABLE_NULL.  If it's a pointer, make
         * sure the content is valid for the profile type.
         */
        if (pToneProfile != VP_PTABLE_NULL) {
            if(VpVerifyProfileType(VP_PROFILE_TONE, pToneProfile) != TRUE) {
                VP_ERROR(VpLineCtxType, pLineCtx, ("Vp890SetLineTone() - Invalid tone profile"));
                return VP_STATUS_ERR_PROFILE;
            }
        }
        pToneProf = pToneProfile;
    } else if (toneIndex < VP_CSLAC_TONE_PROF_TABLE_SIZE) {
        profIndex = (uint16)toneIndex;
        pToneProf = pDevObj->devProfileTable.pToneProfileTable[profIndex];
        if (!(pDevObj->profEntry.toneProfEntry & (0x0001 << profIndex))) {
            /* The profile is invalid -- error. */
            VP_ERROR(VpLineCtxType, pLineCtx, ("Vp890SetLineTone() - Invalid tone profile"));
            return VP_STATUS_ERR_PROFILE;
        }
    } else {
        VP_ERROR(VpLineCtxType, pLineCtx, ("Vp890SetLineTone() - Invalid tone profile"));
        return VP_STATUS_ERR_PROFILE;
    }

    /* Verify a good profile (index or pointer) for the cadence */
    if (cadenceIndex < 0) {
        /*
         * A pointer is passed or VP_PTABLE_NULL.  If it's a pointer, make
         * sure the content is valid for the profile type.
         */
        if (pCadProfile != VP_PTABLE_NULL) {
            if(VpVerifyProfileType(VP_PROFILE_TONECAD, pCadProfile) != TRUE) {
                VP_ERROR(VpLineCtxType, pLineCtx, ("Vp890SetLineTone() - Invalid cadence profile"));
                return VP_STATUS_ERR_PROFILE;
            }
        }
        pCadProf = pCadProfile;
    } else if (cadenceIndex < VP_CSLAC_TONE_CADENCE_PROF_TABLE_SIZE) {
        pCadProf = pDevObj->devProfileTable.pToneCadProfileTable[cadenceIndex];
        if (!(pDevObj->profEntry.toneCadProfEntry & (0x0001 << cadenceIndex))) {
            /* The profile is invalid -- error. */
            VP_ERROR(VpLineCtxType, pLineCtx, ("Vp890SetLineTone() - Invalid cadence profile"));
            return VP_STATUS_ERR_PROFILE;
        }
    } else {
        VP_ERROR(VpLineCtxType, pLineCtx, ("Vp890SetLineTone() - Invalid cadence profile"));
        return VP_STATUS_ERR_PROFILE;
    }

    if (pDtmfControl != VP_NULL) {
        digit = pDtmfControl->toneId;
        if (VpIsDigit(digit) == FALSE) {
            return VP_STATUS_INVALID_ARG;
        }

        direction = pDtmfControl->dir;
        if (direction != VP_DIRECTION_DS) {
            return VP_STATUS_INVALID_ARG;
        }
    }

    /* All input parameters are valid. */
    VpSysEnterCritical(deviceId, VP_CODE_CRITICAL_SEC);

    if (pLineObj->status & VP890_BAD_LOOP_SUP) {
        pLineObj->status &= ~(VP890_BAD_LOOP_SUP);
        VpMpiCmdWrapper(deviceId, ecVal, VP890_LOOP_SUP_WRT,
            VP890_LOOP_SUP_LEN, pLineObj->loopSup);
    }

    /*
     * Disable signal generator A/B/C/D before making any changes and stop
     * previous cadences
     */
    pLineObj->sigGenCtrl[0] &= ~VP890_GEN_ALLON;
    VpMpiCmdWrapper(deviceId, ecVal, VP890_GEN_CTRL_WRT, VP890_GEN_CTRL_LEN,
        pLineObj->sigGenCtrl);

#ifdef CSLAC_SEQ_EN
    if (!(pLineObj->callerId.status & VP_CID_IN_PROGRESS)) {
        pLineObj->cadence.pActiveCadence = pCadProf;
        pLineObj->cadence.status = VP_CADENCE_RESET_VALUE;

        /* We're no longer in the middle of a time function */
        pLineObj->cadence.status &= ~VP_CADENCE_STATUS_MID_TIMER;
        pLineObj->cadence.timeRemain = 0;
    }
#endif

    /* We'll almost for sure update this register, so read it now */
    VpMpiCmdWrapper(deviceId, ecVal, VP890_OP_COND_RD, VP890_OP_COND_LEN,
        &opCond);
    opCond &= ~(VP890_HIGH_PASS_DIS | VP890_OPCOND_RSVD_MASK);

    /*
     * If tone profile is NULL, and either the pDtmfControl is NULL or it's
     * "digit" member is "Digit None", then shutoff the tone generators, stop
     * any active cadencing and restore the filter coefficients if they need
     * to be. Also, re-enable the audio path if it was disabled by a previous
     * DTMF generation command
     */
    if (((pToneProf == VP_PTABLE_NULL) && (pDtmfControl == VP_NULL))
     || ((pToneProf == VP_PTABLE_NULL) && (digit == VP_DIG_NONE))) {
        /* Shutoff all tones and stop the cadencing -- already done */

        if (!(pLineObj->status & VP890_IS_FXO)) {
            /* Restore the normal audio path */
            VpMpiCmdWrapper(deviceId, ecVal, VP890_CONV_CFG_RD,
                VP890_CONV_CFG_LEN, converterCfg);

            converterCfg[0] &= ~VP890_CONV_CONNECT_BITS;
            converterCfg[0] |= VP890_METALLIC_AC_V;

            VpMpiCmdWrapper(deviceId, ecVal, VP890_CONV_CFG_WRT,
                VP890_CONV_CFG_LEN, converterCfg);
        }

#ifdef CSLAC_SEQ_EN
        if (!(pLineObj->callerId.status & VP_CID_IN_PROGRESS)) {
#endif
            /*
             * Pre-Or the bits and get the correct values based on the current
             * line state, then update the device
             */
            opCond &= ~(VP890_CUT_TXPATH | VP890_CUT_RXPATH);
            Vp890GetTxRxPcmMode(pLineObj, pLineObj->lineState.currentState, &mpiByte);
            opCond |= mpiByte;
            VP_INFO(VpLineCtxType, pLineCtx, ("9. Writing 0x%02X to Operating Conditions",
                opCond));

            VpMpiCmdWrapper(deviceId, ecVal, VP890_OP_COND_WRT, VP890_OP_COND_LEN,
                &opCond);
#ifdef CSLAC_SEQ_EN
        }
#endif

        VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
        return VP_STATUS_SUCCESS;
    }

    /*
     * If we're here, we're sending some tone.  If it's DTMF, we can stop the
     * active cadencer, set the time to "always on" (since the application will
     * tell us when to start/stop).
     *
     * If "direction" is some value other than the initialized value, then
     * the dtmf structure is passed and not NULL
     */
    if (direction != VP_DIRECTION_INVALID) {
#ifdef CSLAC_SEQ_EN
        /* Disable currently active cadence */
        pLineObj->cadence.status = VP_CADENCE_RESET_VALUE;
#endif
        /* Update the DTMF Generators and make the downstream connection */
        Vp890SetDTMFGenerators(pLineCtx, VP_CID_NO_CHANGE, digit);

        if (!(pLineObj->status & VP890_IS_FXO)) {
            VpMpiCmdWrapper(deviceId, ecVal, VP890_CONV_CFG_RD,
                VP890_CONV_CFG_LEN, converterCfg);
            converterCfg[0] &= ~VP890_CONV_CONNECT_BITS;
            converterCfg[0] |= VP890_METALLIC_AC_V;

            VpMpiCmdWrapper(deviceId, ecVal, VP890_CONV_CFG_WRT,
                VP890_CONV_CFG_LEN, converterCfg);
        }

        /*
         * Disable only the receive path since disabling the transmit path
         * also may generate noise upstream (e.g., an unterminated, but
         * assigned timeslot
         */
        opCond |= VP890_CUT_RXPATH;
        VP_INFO(VpLineCtxType, pLineCtx, ("15. Writing 0x%02X to Operating Conditions",
            opCond));
        VpMpiCmdWrapper(deviceId, ecVal, VP890_OP_COND_WRT, VP890_OP_COND_LEN,
            &opCond);

        /* Enable only generator C/D */
        pLineObj->sigGenCtrl[0] |= (VP890_GEND_EN | VP890_GENC_EN);
        pLineObj->sigGenCtrl[0] &= ~VP890_GEN_BIAS;
        VpMpiCmdWrapper(deviceId, ecVal, VP890_GEN_CTRL_WRT, VP890_GEN_CTRL_LEN,
            pLineObj->sigGenCtrl);

        VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
        return VP_STATUS_SUCCESS;
    }

#ifdef CSLAC_SEQ_EN
    /* If we're here, we're sending a Tone, not DTMF */
    if ((pCadProf != VP_PTABLE_NULL)
     && ((pCadProf[VP890_TONE_TYPE] == VP890_HOWLER_TONE)
      || (pCadProf[VP890_TONE_TYPE] == VP890_AUS_HOWLER_TONE)
      || (pCadProf[VP890_TONE_TYPE] == VP890_NTT_HOWLER_TONE))) {
        uint8 sigGenCD[VP890_SIGCD_PARAMS_LEN] = {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };

        if (pCadProf[VP890_TONE_TYPE] == VP890_HOWLER_TONE) {
            uint16 ukHowler[] = {
                0x08, 0x88, 0x07, 0xDB, /* Start Freqency/Amplitude (800Hz,-15dBm0) */
                0x01, 0x04, 0x00, 0x30, /* Step Frequency/Amplitude */
                0x22, 0x22, 0x7F, 0xFF  /* Stop Frequency/Amplitude (3200Hz, +3.14dBm0) */
            };

            /* Save the parameters for use in the cadencer */
            pLineObj->cadence.startFreq = ukHowler[0] << 8;
            pLineObj->cadence.startFreq |= ukHowler[1];

            pLineObj->cadence.startLevel = ukHowler[2] << 8;
            pLineObj->cadence.startLevel |= ukHowler[3];

            pLineObj->cadence.freqStep = ukHowler[4] << 8;
            pLineObj->cadence.freqStep |= ukHowler[5];

            pLineObj->cadence.levelStep = ukHowler[6] << 8;
            pLineObj->cadence.levelStep |= ukHowler[7];

            pLineObj->cadence.stopFreq = ukHowler[8] << 8;
            pLineObj->cadence.stopFreq |= ukHowler[9];

            pLineObj->cadence.stopLevel = ukHowler[10] << 8;
            pLineObj->cadence.stopLevel |= ukHowler[11];

            sigGenAB[3] = ukHowler[0];
            sigGenAB[4] = ukHowler[1];

            sigGenAB[5] = ukHowler[2];
            sigGenAB[6] = ukHowler[3];
        } else if (pCadProf[VP890_TONE_TYPE] == VP890_AUS_HOWLER_TONE) {
            uint16 ausHowler[] = {
                0x10, 0x00, 0x07, 0xDB, /* Start Freqency/Amplitude (1500Hz,-15dBm0) */
                0x00, 0xB8, 0x00, 0x18, /* Step Frequency/Amplitude  */
                0x22, 0x22, 0x7F, 0xFF  /* Stop Frequency/Amplitude (3200Hz, +3.14dBm0) */
            };

            /* Save the parameters for use in the cadencer */
            pLineObj->cadence.startFreq = ausHowler[0] << 8;
            pLineObj->cadence.startFreq |= ausHowler[1];

            pLineObj->cadence.startLevel = ausHowler[2] << 8;
            pLineObj->cadence.startLevel |= ausHowler[3];

            pLineObj->cadence.freqStep = ausHowler[4] << 8;
            pLineObj->cadence.freqStep |= ausHowler[5];

            pLineObj->cadence.levelStep = ausHowler[6] << 8;
            pLineObj->cadence.levelStep |= ausHowler[7];

            pLineObj->cadence.stopFreq = ausHowler[8] << 8;
            pLineObj->cadence.stopFreq |= ausHowler[9];

            pLineObj->cadence.stopLevel = ausHowler[10] << 8;
            pLineObj->cadence.stopLevel |= ausHowler[11];

            sigGenAB[3] = ausHowler[0];
            sigGenAB[4] = ausHowler[1];

            sigGenAB[5] = ausHowler[2];
            sigGenAB[6] = ausHowler[3];
        } else {
            uint16 nttHowler[] = {
                0x04, 0x44, 0x07, 0xDB, /* Start Freqency/Amplitude (400Hz,-15dBm0) */
                0x00, 0x00, 0x00, 0x40, /* Step Frequency/Amplitude  */
                0x04, 0x44, 0x7F, 0xFF  /* Stop Frequency/Amplitude (400Hz, +3.14dBm0) */
            };

            /* Save the parameters for use in the cadencer */
            pLineObj->cadence.startFreq = nttHowler[0] << 8;
            pLineObj->cadence.startFreq |= nttHowler[1];


            pLineObj->cadence.startLevel = nttHowler[2] << 8;
            pLineObj->cadence.startLevel |= nttHowler[3];

            pLineObj->cadence.freqStep = nttHowler[4] << 8;
            pLineObj->cadence.freqStep |= nttHowler[5];

            pLineObj->cadence.levelStep = nttHowler[6] << 8;
            pLineObj->cadence.levelStep |= nttHowler[7];

            pLineObj->cadence.stopFreq = nttHowler[8] << 8;
            pLineObj->cadence.stopFreq |= nttHowler[9];

            pLineObj->cadence.stopLevel = nttHowler[10] << 8;
            pLineObj->cadence.stopLevel |= nttHowler[11];

            sigGenAB[3] = nttHowler[0];
            sigGenAB[4] = nttHowler[1];

            sigGenAB[5] = nttHowler[2];
            sigGenAB[6] = nttHowler[3];
        }

        /* Set the signal generator A parameters to the initial value. */
        pLineObj->cadence.isFreqIncrease = TRUE;

        /* Make sure C/D are cleared */
        VpMpiCmdWrapper(deviceId, ecVal, VP890_SIGCD_PARAMS_WRT,
            VP890_SIGCD_PARAMS_LEN, sigGenCD);

        /* Program A/B */
        VpMpiCmdWrapper(deviceId, ecVal, VP890_SIGAB_PARAMS_WRT,
            VP890_SIGAB_PARAMS_LEN, sigGenAB);

        /* Set the parameters in the line object for cadence use */
        for (sigGenABCount = 0; sigGenABCount < VP890_SIGAB_PARAMS_LEN;
             sigGenABCount++) {
            pLineObj->cadence.regData[sigGenABCount] = sigGenAB[sigGenABCount];
        }
    } else {
#endif
        /*
         * Send the signal generator parameters to the device and enable the
         * Tone Generators -- add in the first 3 bytes (all 0x00)
         */
        for (sigGenABCount = sigGenABOffset, profIndex = 0;
             sigGenABCount < VP890_SIGAB_PARAMS_LEN;
             sigGenABCount++, profIndex++) {
            sigGenAB[sigGenABCount] =
                pToneProf[VP890_SIGGEN_AB_START + profIndex];
        }

        VpMpiCmdWrapper(deviceId, ecVal, VP890_SIGAB_PARAMS_WRT,
            VP890_SIGAB_PARAMS_LEN, sigGenAB);
        VpMpiCmdWrapper(deviceId, ecVal, VP890_SIGCD_PARAMS_WRT,
            VP890_SIGCD_PARAMS_LEN,
            (VpProfileDataType *)(&pToneProf[VP890_SIGGEN_CD_START]));

#ifdef CSLAC_SEQ_EN
    }
#endif

#ifdef CSLAC_SEQ_EN
    if (pCadProf == VP_PTABLE_NULL) {
        /*
         * If a tone is being actived due to caller ID, then do not stop the
         * cadencer
         */
        if (!(pLineObj->callerId.status & VP_CID_IN_PROGRESS)) {
            pLineObj->cadence.status = VP_CADENCE_RESET_VALUE;
            pLineObj->cadence.index = VP_PROFILE_TYPE_SEQUENCER_START;
        }
#endif
        pLineObj->sigGenCtrl[0] |= VP890_GEN_ALLON;
        pLineObj->sigGenCtrl[0] &= ~VP890_GEN_BIAS;
        VpMpiCmdWrapper(deviceId, ecVal, VP890_GEN_CTRL_WRT, VP890_GEN_CTRL_LEN,
            pLineObj->sigGenCtrl);
#ifdef CSLAC_SEQ_EN
    } else {
        pLineObj->cadence.pCurrentPos =
            &(pCadProf[VP_PROFILE_TYPE_SEQUENCER_START]);
        pLineObj->cadence.status |= VP_CADENCE_STATUS_ACTIVE;
        pLineObj->cadence.length = pCadProf[VP_PROFILE_LENGTH];
        pLineObj->cadence.index = VP_PROFILE_TYPE_SEQUENCER_START;
        pLineObj->cadence.status &= ~VP_CADENCE_STATUS_IGNORE_POLARITY;
        pLineObj->cadence.status |= (pCadProf[VP_PROFILE_MPI_LEN] & 0x01) ?
            VP_CADENCE_STATUS_IGNORE_POLARITY : 0;

        /* Nullify any internal sequence so that the API doesn't think that
         * an internal sequence of some sort is running */
        pLineObj->intSequence[VP_PROFILE_TYPE_LSB] = VP_PRFWZ_PROFILE_NONE;
    }
#endif

    VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
    return VP_STATUS_SUCCESS;
} /* Vp890SetLineTone() */


/*******************************************************************************
 * Vp890SetRelayState()
 *  This function controls the state of controlled relays for the VP890 device.
 *
 * Preconditions:
 *  Device/Line context should be created and initialized. For applicable
 * devices bootload should be performed before calling the function.
 *
 * Postconditions:
 *  The indicated relay state is set for the given line.
 ******************************************************************************/
VpStatusType
Vp890SetRelayState(
    VpLineCtxType       *pLineCtx,
    VpRelayControlType  rState)
{
    Vp890LineObjectType     *pLineObj   = pLineCtx->pLineObj;
    VpDevCtxType            *pDevCtx    = pLineCtx->pDevCtx;
    Vp890DeviceObjectType   *pDevObj    = pDevCtx->pDevObj;
    VpDeviceIdType          deviceId    = pDevObj->deviceId;
    uint8                   ecVal       = pLineObj->ecVal;

    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("+Vp890SetRelayState()"));

    VpSysEnterCritical(deviceId, VP_CODE_CRITICAL_SEC);

    /* Get out if device state is not ready */
    if (!Vp890IsDevReady(pDevObj->status.state, TRUE)) {
        VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
        return VP_STATUS_DEV_NOT_INITIALIZED;
    }

    /* Handle the VP_RELAY_BRIDGED_TEST case, which is available to all term
     * types.  Due to the lack of an actual test load for 890, this relay
     * state is implemented by the internal test termination algorithm */
    if (rState == VP_RELAY_BRIDGED_TEST)
    {
        Vp890ApplyInternalTestTerm(pLineCtx);

        pLineObj->relayState = VP_RELAY_BRIDGED_TEST;

        VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
        return VP_STATUS_SUCCESS;
    }

    /* If the internal test termination is currently applied and we're going
     * to a state other than BRIDGED_TEST, restore the internal settings */
    if (pLineObj->internalTestTermApplied == TRUE &&
        rState != VP_RELAY_BRIDGED_TEST)
    {
        Vp890RemoveInternalTestTerm(pLineCtx);
    }

    /* Check term type */
    if ((pLineObj->termType == VP_TERM_FXS_GENERIC) ||
        (pLineObj->termType == VP_TERM_FXO_GENERIC) ||
        (pLineObj->termType == VP_TERM_FXS_LOW_PWR))
    {
        /* Check requested relay state */
        if (rState != VP_RELAY_NORMAL) {
            VP_ERROR(VpLineCtxType, pLineCtx, ("Vp890SetRelayState() - Invalid relay state 0x%02X",
                rState));
            VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
            return VP_STATUS_INVALID_ARG;
        }
    } else if ((pLineObj->termType == VP_TERM_FXS_SPLITTER_LP) ||
               (pLineObj->termType == VP_TERM_FXS_SPLITTER) ||
               (pLineObj->termType == VP_TERM_FXS_ISOLATE) ||
               (pLineObj->termType == VP_TERM_FXS_ISOLATE_LP))
    {

        uint8 ioData[VP890_IODATA_REG_LEN];
        uint8 ioDir[VP890_IODIR_REG_LEN];

        VpMpiCmdWrapper(deviceId, ecVal, VP890_IODATA_REG_RD, VP890_IODATA_REG_LEN,
            ioData);
        VpMpiCmdWrapper(deviceId, ecVal, VP890_IODIR_REG_RD, VP890_IODIR_REG_LEN,
            ioDir);

        ioData[0] &= ~VP890_IODATA_IO1;

        ioDir[0] &= ~VP890_IODIR_IO1_MASK;
        ioDir[0] |= VP890_IODIR_IO1_OUTPUT;

        switch(rState) {
            case VP_RELAY_NORMAL:
                /* Normal: Create High on Isolate, Low on Splitter */
                if ((pLineObj->termType == VP_TERM_FXS_ISOLATE) ||
                    (pLineObj->termType == VP_TERM_FXS_ISOLATE_LP)) {
                    ioData[0] |= VP890_IODATA_IO1;
                } else {
                    ioData[0] &= ~VP890_IODATA_IO1;
                }
                break;

            case VP_RELAY_RESET:
                /* Normal: Create Low on Isolate, High on Splitter */
                if ((pLineObj->termType == VP_TERM_FXS_ISOLATE) ||
                    (pLineObj->termType == VP_TERM_FXS_ISOLATE_LP)) {
                    ioData[0] &= ~VP890_IODATA_IO1;
                } else {
                    ioData[0] |= VP890_IODATA_IO1;
                }
                break;

            default:
                VP_ERROR(VpLineCtxType, pLineCtx, ("Vp890SetRelayState() - Invalid relay state 0x%02X",
                    rState));
                VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
                return VP_STATUS_INVALID_ARG;
        }

        VpMpiCmdWrapper(deviceId, ecVal, VP890_IODIR_REG_WRT, VP890_IODIR_REG_LEN,
            ioDir);

        VpMpiCmdWrapper(deviceId, ecVal, VP890_IODATA_REG_WRT, VP890_IODATA_REG_LEN,
            ioData);
    } else {
        VP_ERROR(VpLineCtxType, pLineCtx, ("Vp890SetRelayState() - Invalid termination type 0x%02X",
            pLineObj->termType));
        VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
        return VP_STATUS_INVALID_ARG;
    }

    pLineObj->relayState = rState;

    VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
    return VP_STATUS_SUCCESS;
} /* Vp890SetRelayState() */


/*******************************************************************************
 * Vp890ApplyInternalTestTerm()
 *  Configures ICR settings for the internal test termination algorithm, which
 * is used instead of a physical test load for devices which do not have one.
 * The internal test termination works by internally shorting tip and ring.
 ******************************************************************************/
static void
Vp890ApplyInternalTestTerm(
    VpLineCtxType *pLineCtx)
{
    Vp890LineObjectType     *pLineObj   = pLineCtx->pLineObj;
    VpDevCtxType            *pDevCtx    = pLineCtx->pDevCtx;
    Vp890DeviceObjectType   *pDevObj    = pDevCtx->pDevObj;
    VpDeviceIdType          deviceId    = pDevObj->deviceId;
    uint8                   ecVal       = pLineObj->ecVal;

    uint8 icr1Reg[VP890_ICR1_LEN];

    if (pLineObj->internalTestTermApplied == TRUE) {
        return;
    }

    VpSysEnterCritical(deviceId, VP_CODE_CRITICAL_SEC);

    /* Disconnect VAB sensing */
    pLineObj->dcCalValues[VP890_DC_CAL_CUT_INDEX] |= VP890_C_TIP_SNS_CUT;
    pLineObj->dcCalValues[VP890_DC_CAL_CUT_INDEX] |= VP890_C_RING_SNS_CUT;
    VpMpiCmdWrapper(deviceId, ecVal, VP890_DC_CAL_REG_WRT, VP890_DC_CAL_REG_LEN,
        pLineObj->dcCalValues);

    /* Reverse the polarity of the ground key detector to disable ground
     * key event */
    pLineObj->icr4Values[VP890_ICR4_GKEY_DET_LOCATION] |=
        VP890_ICR4_GKEY_POL;
    pLineObj->icr4Values[VP890_ICR4_GKEY_DET_LOCATION+1] |=
        VP890_ICR4_GKEY_POL;
    VpMpiCmdWrapper(deviceId, ecVal, VP890_ICR4_WRT, VP890_ICR4_LEN,
        pLineObj->icr4Values);

    /* Forcing the SLIC DC bias for 200ms helps collapse the battery
     * voltage, especially for fixed tracking designs.  We're taking over
     * ICR1 completely here.  Other parts of the code will set
     * pLineObj->icr1Values but will not actually write to the register
     * while this relay state is active.  See Vp890ProtectedWriteICR1().  When
     * we leave this relay state, we will restore pLineObj->icr1Values */
    icr1Reg[0] = 0xFF;
    icr1Reg[1] = 0xFF;
    icr1Reg[2] = 0xFF;
    icr1Reg[3] = 0x0F;
    VpMpiCmdWrapper(deviceId, ecVal, VP890_ICR1_WRT, VP890_ICR1_LEN, icr1Reg);

    /* Start a timer to change the ICR1 settings later to make tip and ring
     * outputs high impedance so that they tend to pull to battery. */
    pLineObj->lineTimers.timers.timer[VP_LINE_INTERNAL_TESTTERM_TIMER] =
        MS_TO_TICKRATE(VP890_INTERNAL_TESTTERM_SETTLING_TIME,
            pDevObj->devProfileData.tickRate);
    pLineObj->lineTimers.timers.timer[VP_LINE_INTERNAL_TESTTERM_TIMER]
        |= VP_ACTIVATE_TIMER;

    pLineObj->internalTestTermApplied = TRUE;

    VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
} /* Vp890ApplyInternalTestTerm() */

/*******************************************************************************
 * Vp890RemoveInternalTestTerm()
 *  This function reverts the settings that control the internal test
 * termination.
 ******************************************************************************/
static void
Vp890RemoveInternalTestTerm(
    VpLineCtxType *pLineCtx)
{
    Vp890LineObjectType     *pLineObj   = pLineCtx->pLineObj;
    VpDevCtxType            *pDevCtx    = pLineCtx->pDevCtx;
    Vp890DeviceObjectType   *pDevObj    = pDevCtx->pDevObj;
    VpDeviceIdType          deviceId    = pDevObj->deviceId;
    uint8                   ecVal       = pLineObj->ecVal;

    VpSysEnterCritical(deviceId, VP_CODE_CRITICAL_SEC);

    /* Restore VAB sensing */
    pLineObj->dcCalValues[VP890_DC_CAL_CUT_INDEX] &= ~VP890_C_TIP_SNS_CUT;
    pLineObj->dcCalValues[VP890_DC_CAL_CUT_INDEX] &= ~VP890_C_RING_SNS_CUT;
    VpMpiCmdWrapper(deviceId, ecVal, VP890_DC_CAL_REG_WRT, VP890_DC_CAL_REG_LEN,
        pLineObj->dcCalValues);

    /* Restore ground key polarity setting */
    pLineObj->icr4Values[VP890_ICR4_GKEY_DET_LOCATION] &=
        ~VP890_ICR4_GKEY_POL;
    pLineObj->icr4Values[VP890_ICR4_GKEY_DET_LOCATION+1] &=
        ~VP890_ICR4_GKEY_POL;
    VpMpiCmdWrapper(deviceId, ecVal, VP890_ICR4_WRT, VP890_ICR4_LEN,
        pLineObj->icr4Values);

    /* Restore ICR1 to the cached value in pLineObj->icr1Values */
    VpMpiCmdWrapper(deviceId, ecVal, VP890_ICR1_WRT, VP890_ICR1_LEN,
        pLineObj->icr1Values);

    /* Deactivate the timer in case it is still running */
    pLineObj->lineTimers.timers.timer[VP_LINE_INTERNAL_TESTTERM_TIMER]
        &= ~VP_ACTIVATE_TIMER;

    pLineObj->internalTestTermApplied = FALSE;

    VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
} /* Vp890RemoveInternalTestTerm() */

/*******************************************************************************
 * Vp890SetRelGain()
 *  This function adjusts the GR and GX values for a given channel of a given
 * device.  It multiplies the profile values by a factor from 1.0 to 4.0 (GX) or
 * from 0.25 to 1.0 (GR).  The adjustment factors are specified in the txLevel
 * and rxLevel parameters, which are 2.14 fixed-point numbers.
 *
 * Arguments:
 *  *pLineCtx  -  Line context to change gains on
 *  txLevel    -  Adjustment to line's relative Tx level
 *  rxLevel    -  Adjustment to line's relative Rx level
 *  handle     -  Handle value returned with the event
 *
 * Preconditions:
 *  The line must first be initialized prior to adjusting the gains.  Any
 * pre-existing results must be cleared by calling VpGetResults() before
 * calling this function.
 *
 * Postconditions:
 *  Returns error if device is not initialized or results are not cleared.
 * Otherwise, generates a VE_LINE_EVID_GAIN_CMP event and saves results in
 * the device object for later retrieval by VpGetResults().
 ******************************************************************************/
VpStatusType
Vp890SetRelGain(
    VpLineCtxType   *pLineCtx,
    uint16          txLevel,
    uint16          rxLevel,
    uint16          handle)
{
    Vp890LineObjectType     *pLineObj   = pLineCtx->pLineObj;
    Vp890DeviceObjectType   *pDevObj    = pLineCtx->pDevCtx->pDevObj;
    VpDeviceIdType          deviceId    = pDevObj->deviceId;
    VpStatusType            status;

    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("+Vp890SetRelGain()"));

    VpSysEnterCritical(deviceId, VP_CODE_CRITICAL_SEC);

    if (pDevObj->deviceEvents.response & VP890_READ_RESPONSE_MASK) {
        VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
        VP_WARNING(VpLineCtxType, pLineCtx, ("Vp890SetRelGain() - Waiting to clear previous read"));
        return VP_STATUS_DEVICE_BUSY;
    }

    pLineObj->gxUserLevel = txLevel;
    pLineObj->grUserLevel = rxLevel;

    status = Vp890SetRelGainInt(pLineCtx);

    if (status == VP_STATUS_SUCCESS){
        /* Generate the gain-complete event. */
        pLineObj->lineEvents.response |= VP_LINE_EVID_GAIN_CMP;
        pLineObj->lineEventHandle = handle;
    }

    VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
    return status;
} /* Vp890SetRelGain() */

/*******************************************************************************
 * CidCorrectionLookup()
 *  This function does a linear interpolation table lookup to find a caller ID
 * gain correction factor based on a measured line voltage.
 *
 * Arguments:
 *  voltage - Measured voltage on a scale where 1V = 25 units and

           1.28V = 32 units.

 * Returns:
 *  If the given voltage is outside the range in the table, returns the nearest
 * gain value in the table.
 *  If the given voltage matches a table entry, returns the gain factor of that
 * entry.
 *  If the given voltage is between two entries, returns a gain factor
 * interpolated from the nearest two entries.
 ******************************************************************************/
static void
CidCorrectionLookup(
    uint16                  voltage,
    uint16                  *gainFactor,
    uint8                   *dtgVal)
{
    /* Lookup table for FXO callerID gain adjustments based on line voltage.
     * Voltage measurements are in units of 1.28, or 32/25, so we use a voltage
     * representation where 1V = 25 units and 1.28V = 32 units.
     * If you add/remove elements, make sure to change VP890_CID_CORRECT_TABLE_SIZE
     */
#define VP890_CID_CORRECTION_DTG12_TABLE_SIZE 13
    Vp890CidCorrectionPair tableDtg12[VP890_CID_CORRECTION_DTG12_TABLE_SIZE] =
    {   {/* 48.6 V */ 1216, 16843 /* Gain factor 1.028 */},
        {/* 51.2 V */ 1280, 18661 /* Gain factor 1.139 */},
        {/* 53.8 V */ 1344, 20578 /* Gain factor 1.256 */},
        {/* 56.3 V */ 1408, 22577 /* Gain factor 1.378 */},
        {/* 58.9 V */ 1472, 24674 /* Gain factor 1.506 */},
        {/* 61.4 V */ 1536, 26870 /* Gain factor 1.640 */},
        {/* 64.0 V */ 1600, 29164 /* Gain factor 1.780 */},
        {/* 66.6 V */ 1664, 31539 /* Gain factor 1.925 */},
        {/* 69.1 V */ 1728, 34013 /* Gain factor 2.076 */},
        {/* 71.7 V */ 1792, 36569 /* Gain factor 2.232 */},
        {/* 74.2 V */ 1856, 39240 /* Gain factor 2.395 */},
        {/* 76.8 V */ 1920, 41992 /* Gain factor 2.563 */},
        {/* 79.4 V */ 1984, 44827 /* Gain factor 2.736 */}
    };
#define VP890_CID_CORRECTION_DTG6_TABLE_SIZE 12
    Vp890CidCorrectionPair tableDtg6[VP890_CID_CORRECTION_DTG6_TABLE_SIZE] =
    {   {/* 34.6 V */  864, 16957 /* Gain factor 1.035 */},
        {/* 35.8 V */  896, 18235 /* Gain factor 1.113 */},
        {/* 37.1 V */  928, 19562 /* Gain factor 1.194 */},
        {/* 38.4 V */  960, 20939 /* Gain factor 1.278 */},
        {/* 39.7 V */  992, 22364 /* Gain factor 1.365 */},
        {/* 41.0 V */ 1024, 23822 /* Gain factor 1.454 */},
        {/* 42.2 V */ 1056, 25346 /* Gain factor 1.547 */},
        {/* 43.5 V */ 1088, 26903 /* Gain factor 1.642 */},
        {/* 44.8 V */ 1120, 28508 /* Gain factor 1.740 */},
        {/* 46.1 V */ 1152, 30163 /* Gain factor 1.841 */},
        {/* 47.4 V */ 1184, 31850 /* Gain factor 1.944 */},
        {/* 48.6 V */ 1216, 33604 /* Gain factor 2.051 */}
    };
#define VP890_CID_CORRECTION_DTG0_TABLE_SIZE 6
    Vp890CidCorrectionPair tableDtg0[VP890_CID_CORRECTION_DTG0_TABLE_SIZE] =
    {   {/* 28.2 V */  704, 22479 /* Gain factor 1.372 */},
        {/* 29.4 V */  736, 24560 /* Gain factor 1.499 */},
        {/* 30.7 V */  768, 26739 /* Gain factor 1.632 */},
        {/* 32.0 V */  800, 29016 /* Gain factor 1.771 */},
        {/* 33.3 V */  832, 31392 /* Gain factor 1.916 */},
        {/* 34.6 V */  864, 33849 /* Gain factor 2.066 */}
    };

    Vp890CidCorrectionPair *pTable;
    uint8 tableSize;
    uint8 index;

    VP_API_FUNC_INT(None, VP_NULL, ("+CidCorrectionLookup()"));

    /* Choose which table and DTG setting to use based on the voltage range */
    if (voltage < tableDtg6[0].volts) {
        pTable = tableDtg0;
        tableSize = VP890_CID_CORRECTION_DTG0_TABLE_SIZE;
        *dtgVal = VP890_DTG_0DB;
    } else if (voltage < tableDtg12[0].volts) {
        pTable = tableDtg6;
        tableSize = VP890_CID_CORRECTION_DTG6_TABLE_SIZE;
        *dtgVal = VP890_DTG_6DB;
    } else {
        pTable = tableDtg12;
        tableSize = VP890_CID_CORRECTION_DTG12_TABLE_SIZE;
        *dtgVal = VP890_DTG_12DB;
    }

    if (voltage <= pTable[0].volts) {
        /* Voltage is equal to or less than the lowest entry in the table, so
         * return the gain of the lowest entry */
        *gainFactor = pTable[0].gain;
        return;
    }
    for (index = 0; index < tableSize-1; index++){
        if (voltage < pTable[index+1].volts) {
            /* Linear Interpolation
             * vm = voltage     v[i] = pTable[index].volts
             * g = gain result  g[i] = pTable[index].gain
             * Formula: g = (( vm - v[i] ) / ( v[i+1] - v[i] )) * ( g[i+1] - g[i] ) + g[i]
             * Terms:   g = ((   termA   ) / (      termB    )) * (     termC     ) + g[i]
             * Fixed point reorder:  g = ( termC / termB ) * termA + g[i]
             */
            uint16 termA, termB, termC;
            termA = voltage - pTable[index].volts;
            termB = pTable[index+1].volts - pTable[index].volts;
            termC = pTable[index+1].gain - pTable[index].gain;

            *gainFactor = ( termC / termB ) * termA + pTable[index].gain;
            return;
        }
    }
    /* Voltage is equal to or beyond the highest entry in the table, so return
     * the gain of the highest entry */
    *gainFactor = pTable[index].gain;
    return;
} /* CidCorrectionLookup() */

/*******************************************************************************
 * Vp890SetOption()
 *  This function determines how to process the Option based on pDevCtx,
 *  pLineCtx, and option type.  The actual options are implemented in static
 *  functions SetDeviceOption and SetLineOption.
 *
 * Preconditions:
 *  The line must first be initialized if a line context is passed, or the
 * device must be initialized if a device context is passed.
 *
 * Postconditions:
 *  The option specified is implemented either on the line, or on the device, or
 * on all lines associated with the device (see the API Reference Guide for
 ******************************************************************************/
VpStatusType
Vp890SetOption(
    VpLineCtxType           *pLineCtx,
    VpDevCtxType            *pDevCtx,
    VpOptionIdType          option,
    void                    *value)
{
    Vp890DeviceObjectType   *pDevObj;
    VpDeviceIdType          deviceId;
    VpDevCtxType            *pDevCtxLocal;
    VpStatusType            status          = VP_STATUS_INVALID_ARG;

    VP_API_FUNC_INT(None, VP_NULL, ("+Vp890SetOption()"));

    /* Error checking.  Either device or line context must be valid, but
     * not both. */
    if (pDevCtx != VP_NULL) {
        if (pLineCtx != VP_NULL) {
            return VP_STATUS_INVALID_ARG;
        }
    } else {
        if (pLineCtx == VP_NULL) {
            return VP_STATUS_INVALID_ARG;
        }
    }

    /* Get Device Object based on input param */
    if (pDevCtx != VP_NULL) {
        pDevObj = pDevCtx->pDevObj;
    } else {
        pDevCtxLocal = pLineCtx->pDevCtx;
        pDevObj = pDevCtxLocal->pDevObj;
    }

    deviceId = pDevObj->deviceId;

    /* Get out if device state is not ready */
    if (!Vp890IsDevReady(pDevObj->status.state, TRUE)) {
        return VP_STATUS_DEV_NOT_INITIALIZED;
    }
    VpSysEnterCritical(deviceId, VP_CODE_CRITICAL_SEC);

    if (pDevCtx != VP_NULL) {
        status = SetDeviceOption(pDevCtx, deviceId, option, value);
    } else {
        status = SetLineOption(pLineCtx, deviceId, option, value);
    }

    VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);

    return status;
} /* Vp890SetOption() */

/*******************************************************************************
 * SetDeviceOption()
 *  This function determines how to process Options based on pDevCtx. Any
 *  options that are line specific, loop through all existing lines
 *  associated with pDevCtx calling SetLineOption() with the LineCtx
 *  associated with that line.
 *
 *  Any options that are device specific, Carry out their task here.
 *
 * Preconditions:
 *  The device associated with this line must be initialized.
 *
 * Postconditions:
 *  This function returns the success code if the option associated with this
 *  function completes without issues.
 ******************************************************************************/
static VpStatusType
SetDeviceOption(
    VpDevCtxType        *pDevCtx,
    VpDeviceIdType      deviceId,
    VpOptionIdType      option,
    void                *value)
{
    Vp890DeviceObjectType   *pDevObj    = pDevCtx->pDevObj;
    VpLineCtxType           *pLineCtx;
    Vp890LineObjectType     *pLineObj;
    VpStatusType            status      = VP_STATUS_SUCCESS;
    VpOptionDeviceIoType    deviceIo;
    uint8                   tempSysConfig[VP890_SS_CONFIG_LEN];
    uint8                   ioDirection = 0;
    uint8                   ecVal = 0;
    uint8                   chanId;
    bool                    onlyFXO = TRUE;

    VP_API_FUNC_INT(VpDevCtxType, pDevCtx, ("+SetDeviceOption()"));

    /*
     * Valid Device Context, we already know Line context is NULL (higher
     * layer SW, process on device if device option, or process on all lines
     * associated with device if line option
     */
    switch (option) {
        /* Line Options */
        case VP_OPTION_ID_EVENT_MASK:  /* Affects Line and Device */
        case VP_OPTION_ID_ZERO_CROSS:
        case VP_OPTION_ID_PULSE_MODE:
        case VP_OPTION_ID_TIMESLOT:
        case VP_OPTION_ID_CODEC:
        case VP_OPTION_ID_PCM_HWY:
        case VP_OPTION_ID_LOOPBACK:
        case VP_OPTION_ID_LINE_STATE:
        case VP_OPTION_ID_RING_CNTRL:
        case VP_OPTION_ID_PCM_TXRX_CNTRL:
        case VP_OPTION_ID_SWITCHER_CTRL:
            /*
             * Loop through all of the valid channels associated with this
             * device. Init status variable in case there are currently no
             * line contexts associated with this device
             */
            status = VP_STATUS_SUCCESS;
            for (chanId = 0; chanId < pDevObj->staticInfo.maxChannels; chanId++) {

                pLineCtx = pDevCtx->pLineCtx[chanId];

                if (pLineCtx == VP_NULL) {
                    continue;
                }

                if ((option == VP_OPTION_ID_ZERO_CROSS) ||
                    (option == VP_OPTION_ID_PULSE_MODE) ||
                    (option == VP_OPTION_ID_LINE_STATE) ||
                    (option == VP_OPTION_ID_RING_CNTRL) ||
                    (option == VP_OPTION_ID_SWITCHER_CTRL)){
                    uint8 lastChannel = (pDevObj->staticInfo.maxChannels - 1);

                    pLineObj = pLineCtx->pLineObj;

                    /* This device has at least 1 FXS, SetOption will succeed */
                    if (!(pLineObj->status & VP890_IS_FXO)) {
                        onlyFXO = FALSE;
                        status = SetLineOption(pLineCtx, deviceId, option, value);
                    /* Only FXO on this device */
                    } else if ((onlyFXO == TRUE) && (chanId == lastChannel)) {
                        status = VP_STATUS_OPTION_NOT_SUPPORTED;
                    /* Just bailout in case there is at least 1 FXS on this device */
                    } else {
                        break;
                    }
                } else {
                    status = SetLineOption(pLineCtx, deviceId, option, value);
                }

                if (VP_STATUS_SUCCESS != status) {
                    return status;
                }
            }
            break;

        case VP_DEVICE_OPTION_ID_PULSE:
            if (pDevObj->stateInt & VP890_IS_FXO_ONLY) {
                status = VP_STATUS_OPTION_NOT_SUPPORTED;
                break;
            }
            pDevObj->pulseSpecs = *((VpOptionPulseType *)value);
            break;

        case VP_DEVICE_OPTION_ID_PULSE2:
            if (pDevObj->stateInt & VP890_IS_FXO_ONLY) {
                status = VP_STATUS_OPTION_NOT_SUPPORTED;
                break;
            }
            pDevObj->pulseSpecs2 = *((VpOptionPulseType *)value);
            break;

        case VP_DEVICE_OPTION_ID_CRITICAL_FLT:
            if (pDevObj->stateInt & VP890_IS_FXO_ONLY) {
                status = VP_STATUS_OPTION_NOT_SUPPORTED;
                break;
            }

            pDevObj->criticalFault = *((VpOptionCriticalFltType *)value);

            if ((pDevObj->criticalFault.acFltDiscEn == TRUE)
             || (pDevObj->criticalFault.dcFltDiscEn == TRUE)) {
                pDevObj->criticalFault.acFltDiscEn = FALSE;
                pDevObj->criticalFault.dcFltDiscEn = FALSE;
                return VP_STATUS_INVALID_ARG;
            }

            for (chanId = 0; chanId < pDevObj->staticInfo.maxChannels; chanId++) {
                pLineCtx = pDevCtx->pLineCtx[chanId];
                if (pLineCtx == VP_NULL) {
                    continue;
                }
                pLineObj = pLineCtx->pLineObj;
                if (pLineObj->status & VP890_IS_FXO) {
                    continue;
                }
                VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_SS_CONFIG_RD,
                    VP890_SS_CONFIG_LEN, tempSysConfig);

                if (pDevObj->criticalFault.thermFltDiscEn == TRUE) {
                    tempSysConfig[0] |= VP890_ATFS_EN;
                } else {
                    tempSysConfig[0] &= ~(VP890_ATFS_EN);
                }
                VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_SS_CONFIG_WRT,
                    VP890_SS_CONFIG_LEN, tempSysConfig);
            }
            break;

        case VP_DEVICE_OPTION_ID_DEVICE_IO:
            VP_API_FUNC_INT(VpDevCtxType, pDevCtx, ("+SetDeviceOption() - OPTION_ID_DEVICE_IO"));

            deviceIo = *(VpOptionDeviceIoType *)(value);
            ioDirection = deviceIo.directionPins_31_0 << 1;
            ioDirection &= ~VP890_IODIR_IO1_MASK;

            /* Default 00 is input */
            if ((deviceIo.directionPins_31_0 & 1) == VP_IO_OUTPUT_PIN) {
                /* Driven output only is supported. */
                ioDirection |= VP890_IODIR_IO1_OUTPUT;
            }

            for(chanId=0; chanId < pDevObj->staticInfo.maxChannels; chanId++ ) {
                pLineCtx = pDevCtx->pLineCtx[chanId];
                if (pLineCtx != VP_NULL) {
                    pLineObj = pLineCtx->pLineObj;
                    ecVal |= pLineObj->ecVal;
                    if ((pLineObj->termType == VP_TERM_FXS_SPLITTER_LP)
                     || (pLineObj->termType == VP_TERM_FXS_SPLITTER)
                     || (pLineObj->termType == VP_TERM_FXS_ISOLATE)) {
                        /* Splitter/Isolate I/O1 direction must be unchanged */
                        /*
                         * Note that this works only if the line contexts are
                         * associated with the device context passed to this
                         * function -- which is not a requirement.
                         */
                        uint8 localIoDir[VP890_IODIR_REG_LEN];
                        VpMpiCmdWrapper(deviceId, ecVal, VP890_IODIR_REG_RD, VP890_IODIR_REG_LEN,
                            localIoDir);

                        ioDirection &= ~VP890_IODIR_IO1_MASK;
                        ioDirection |= (localIoDir[0] & VP890_IODIR_IO1_MASK);
                    }
                }
            }
            VpMpiCmdWrapper(deviceId, ecVal, VP890_IODIR_REG_WRT, VP890_IODIR_REG_LEN,
                &ioDirection);
            VP_API_FUNC_INT(VpDevCtxType, pDevCtx, (" -SetDeviceOption() - OPTION_ID_DEVICE_IO"));
            break;

#ifdef VP_DEBUG
        case VP_OPTION_ID_DEBUG_SELECT:
            /* Update the debugSelectMask in the Device Object. */
            pDevObj->debugSelectMask = *(uint32 *)value;
            break;
#endif

        default:
            status = VP_STATUS_OPTION_NOT_SUPPORTED;
            VP_ERROR(VpDevCtxType, pDevCtx, ("SetDeviceOption() - Device option not supported 0x%02X",
                option));
            break;
    }

    return status;
} /* SetDeviceOption() */

/*******************************************************************************
 * SetLineOption()
 * This function ...
 *
 * Arguments:
 *
 * Preconditions:
 *
 * Postconditions:
 ******************************************************************************/
static VpStatusType
SetLineOption(
    VpLineCtxType           *pLineCtx,
    VpDeviceIdType          deviceId,
    VpOptionIdType          option,
    void                    *value)
{
    VpDevCtxType            *pDevCtxLocal   = pLineCtx->pDevCtx;
    Vp890LineObjectType     *pLineObj       = pLineCtx->pLineObj;
    Vp890DeviceObjectType   *pDevObj        = pDevCtxLocal->pDevObj;
    VpStatusType            status          = VP_STATUS_SUCCESS;

    uint8                   ecVal           = pLineObj->ecVal;
    uint8                   txSlot, rxSlot;
    uint8                   mpiData, mpiByte;
    uint8                   tempSysConfig[VP890_SS_CONFIG_LEN];
    uint8                   tempLoopBack[VP890_LOOPBACK_LEN];
    uint8                   devMode[VP890_DEV_MODE_LEN];

    VP_API_FUNC_INT(VpDeviceIdType, &deviceId, ("+SetLineOption()"));

    switch (option) {
        /* Device Options - invalid arg */
        case VP_DEVICE_OPTION_ID_PULSE:
        case VP_DEVICE_OPTION_ID_PULSE2:
        case VP_DEVICE_OPTION_ID_CRITICAL_FLT:
        case VP_DEVICE_OPTION_ID_DEVICE_IO:
            return VP_STATUS_INVALID_ARG;

        /* Line Options */
        case VP_OPTION_ID_PULSE_MODE:
            /* Option does not apply to FXO */
            if (pLineObj->status & VP890_IS_FXO) {
                status = VP_STATUS_OPTION_NOT_SUPPORTED;
                break;
            }
            pLineObj->pulseMode = *((VpOptionPulseModeType *)value);
            VpInitDP(&pLineObj->dpStruct);
            VpInitDP(&pLineObj->dpStruct2);
            break;

        case VP_OPTION_ID_TIMESLOT:
            txSlot = ((VpOptionTimeslotType *)value)->tx;
            rxSlot = ((VpOptionTimeslotType *)value)->rx;
            status = SetTimeSlot(pLineCtx, txSlot, rxSlot);
            break;

        case VP_OPTION_ID_CODEC:
            status = Vp890SetCodec(pLineCtx, *((VpOptionCodecType *)value));
            break;

        case VP_OPTION_ID_PCM_HWY:
            if (*((VpOptionPcmHwyType *)value) != VP_OPTION_HWY_A) {
                VP_ERROR(VpLineCtxType, pLineCtx, ("SetLineOption() - Invalid PCM Highway option"));
                return VP_STATUS_INVALID_ARG;
            }
            break;

        case VP_OPTION_ID_LOOPBACK:
            /* Timeslot loopback via loopback register */
            VpMpiCmdWrapper(deviceId, ecVal, VP890_LOOPBACK_RD,
                VP890_LOOPBACK_LEN, tempLoopBack);
            VpMpiCmdWrapper(deviceId, ecVal, VP890_DEV_MODE_RD,
                VP890_DEV_MODE_LEN, devMode);

            switch(*((VpOptionLoopbackType *)value)) {
                case VP_OPTION_LB_TIMESLOT:
                    tempLoopBack[0] |= VP890_INTERFACE_LOOPBACK_EN;
                    devMode[0] &= ~(VP890_DEV_MODE_CH21PT | VP890_DEV_MODE_CH12PT);
                    break;

                case VP_OPTION_LB_OFF:
                    tempLoopBack[0] &= ~(VP890_INTERFACE_LOOPBACK_EN);
                    devMode[0] &= ~(VP890_DEV_MODE_CH21PT | VP890_DEV_MODE_CH12PT);
                    break;

                case VP_OPTION_LB_CHANNELS:
                    tempLoopBack[0] &= ~(VP890_INTERFACE_LOOPBACK_EN);
                    devMode[0] |= (VP890_DEV_MODE_CH21PT | VP890_DEV_MODE_CH12PT);
                    break;

                case VP_OPTION_LB_DIGITAL:
                default:
                    VP_ERROR(VpLineCtxType, pLineCtx, ("SetLineOption() - Invalid Loopback option"));
                    return VP_STATUS_INVALID_ARG;
            }

            VP_INFO(VpLineCtxType, pLineCtx, ("1. Loopback 0x%02X", tempLoopBack[0]));
            VpMpiCmdWrapper(deviceId, ecVal, VP890_LOOPBACK_WRT,
                VP890_LOOPBACK_LEN, tempLoopBack);
            VpMpiCmdWrapper(deviceId, ecVal, VP890_DEV_MODE_WRT,
                VP890_DEV_MODE_LEN, devMode);
            break;

        case VP_OPTION_ID_LINE_STATE:
            /* Option does not apply to FXO */
            if (pLineObj->status & VP890_IS_FXO) {
                status = VP_STATUS_OPTION_NOT_SUPPORTED;
                break;
            }
            /*
             * Only supports one type of battery control, so make sure it
             * is set correctly. If not, return error otherwise continue
             */
            if (((VpOptionLineStateType *)value)->bat != VP_OPTION_BAT_AUTO) {
                VP_ERROR(VpLineCtxType, pLineCtx, ("SetLineOption() - Invalid battery option"));
                return VP_STATUS_INVALID_ARG;
            }

            VpMpiCmdWrapper(deviceId, ecVal, VP890_SS_CONFIG_RD,
                VP890_SS_CONFIG_LEN, tempSysConfig);

            if (((VpOptionLineStateType *)value)->battRev == TRUE) {
                tempSysConfig[0] &= ~(VP890_SMOOTH_PR_EN);
            } else {
                tempSysConfig[0] |= VP890_SMOOTH_PR_EN;
            }
            VpMpiCmdWrapper(deviceId, ecVal, VP890_SS_CONFIG_WRT,
                VP890_SS_CONFIG_LEN, tempSysConfig);
            break;

        case VP_OPTION_ID_EVENT_MASK:
             SetOptionEventMask(pDevObj, pLineObj, deviceId, ecVal, value);
             break;

        case VP_OPTION_ID_ZERO_CROSS:
            /* Option does not apply to FXO */
            if (pLineObj->status & VP890_IS_FXO) {
                status = VP_STATUS_OPTION_NOT_SUPPORTED;
                break;
            }
            VpMpiCmdWrapper(deviceId, ecVal, VP890_SS_CONFIG_RD,
                VP890_SS_CONFIG_LEN, tempSysConfig);

            if (*(VpOptionZeroCrossType *)value == VP_OPTION_ZC_NONE) {
                tempSysConfig[0] |= VP890_ZXR_DIS;
            } else {
                tempSysConfig[0] &= ~(VP890_ZXR_DIS);
            }
            VpMpiCmdWrapper(deviceId, ecVal, VP890_SS_CONFIG_WRT,
                VP890_SS_CONFIG_LEN, tempSysConfig);

            pLineObj->ringCtrl.zeroCross = *((VpOptionZeroCrossType *)value);
            break;

        case VP_OPTION_ID_RING_CNTRL: {
            VpOptionRingControlType ringCtrl = *((VpOptionRingControlType *)value);

            /* Option does not apply to FXO */
            if (pLineObj->status & VP890_IS_FXO) {
                status = VP_STATUS_OPTION_NOT_SUPPORTED;
                break;
            }

            if (Vp890IsSupportedFxsState(ringCtrl.ringTripExitSt) == FALSE) {
                return VP_STATUS_INVALID_ARG;
            }

            pLineObj->ringCtrl = ringCtrl;

            VpMpiCmdWrapper(deviceId, ecVal, VP890_SS_CONFIG_RD,
                VP890_SS_CONFIG_LEN, tempSysConfig);

            if (pLineObj->ringCtrl.zeroCross == VP_OPTION_ZC_NONE) {
                tempSysConfig[0] |= VP890_ZXR_DIS;
            } else {
                tempSysConfig[0] &= ~(VP890_ZXR_DIS);
            }

            VpMpiCmdWrapper(deviceId, ecVal, VP890_SS_CONFIG_WRT,
                VP890_SS_CONFIG_LEN, tempSysConfig);
            }
            break;

        case VP_OPTION_ID_PCM_TXRX_CNTRL:
            pLineObj->pcmTxRxCtrl = *((VpOptionPcmTxRxCntrlType *)value);
            VpMpiCmdWrapper(deviceId, ecVal, VP890_OP_COND_RD, VP890_OP_COND_LEN,
                &mpiData);

            VP_INFO(VpLineCtxType, pLineCtx, ("16.a Data Read 0x%02X from Operating Conditions",
                mpiData));

            mpiData &= ~(VP890_CUT_TXPATH | VP890_CUT_RXPATH);
            mpiData &= ~(VP890_HIGH_PASS_DIS | VP890_OPCOND_RSVD_MASK);

            Vp890GetTxRxPcmMode(pLineObj, pLineObj->lineState.currentState, &mpiByte);
            mpiData |= mpiByte;
            VP_INFO(VpLineCtxType, pLineCtx, ("16.b Writing 0x%02X to Operating Conditions",
                mpiData));
            VpMpiCmdWrapper(deviceId, ecVal, VP890_OP_COND_WRT, VP890_OP_COND_LEN,
                &mpiData);
            break;

        case VP_OPTION_ID_SWITCHER_CTRL: {
            bool shutDownEn = *((bool *)value);
            uint8 ssConfig[VP890_SS_CONFIG_LEN];

            /* Option does not apply to FXO */
            if (pLineObj->status & VP890_IS_FXO) {
                status = VP_STATUS_OPTION_NOT_SUPPORTED;
                break;
            }

            VpMpiCmdWrapper(deviceId, ecVal, VP890_SS_CONFIG_RD, VP890_SS_CONFIG_LEN, ssConfig);

            if (shutDownEn == TRUE) {
                ssConfig[0] |= VP890_AUTO_BAT_SHUTDOWN_EN;
            } else {
                ssConfig[0] &= ~VP890_AUTO_BAT_SHUTDOWN_EN;
            }

            VpMpiCmdWrapper(deviceId, ecVal, VP890_SS_CONFIG_WRT, VP890_SS_CONFIG_LEN, ssConfig);
            }
            break;

#ifdef VP_DEBUG
        case VP_OPTION_ID_DEBUG_SELECT:
            /* Update the debugSelectMask in the Line Object. */
            pLineObj->debugSelectMask = *(uint32 *)value;
            break;
#endif

        default:
            status = VP_STATUS_OPTION_NOT_SUPPORTED;
            VP_ERROR(VpLineCtxType, pLineCtx, ("SetLineOption() - Line option not supported 0x%02X",
                option));
            break;
    }

    return status;
} /* SetLineOption() */

/*******************************************************************************
 * SetOptionEventMask()
 * This function ...
 *
 * Arguments:
 *
 * Preconditions:
 *
 * Postconditions:
 ******************************************************************************/
static void
SetOptionEventMask(
    Vp890DeviceObjectType   *pDevObj,
    Vp890LineObjectType     *pLineObj,
    VpDeviceIdType          deviceId,
    uint8                   ecVal,
    void                    *value)
{
    uint8                   chanId      = pLineObj->channelId;
    VpOptionEventMaskType   *pEventsMask, *pNewEventsMask;
    uint16                  eventMask;
    uint8                   tempData[VP890_INT_MASK_LEN];

    VP_API_FUNC_INT(VpDeviceIdType, &deviceId, ("+SetOptionEventMask()"));

    pNewEventsMask = (VpOptionEventMaskType *)value;

    /* Zero out the line-specific bits before setting the
     * deviceEventsMask in the device object. */
    pEventsMask = &pDevObj->deviceEventsMask;

    pEventsMask->faults     = pNewEventsMask->faults    & VP_EVCAT_FAULT_DEV_EVENTS;
    pEventsMask->signaling  = pNewEventsMask->signaling & VP_EVCAT_SIGNALING_DEV_EVENTS;
    pEventsMask->response   = pNewEventsMask->response  & VP_EVCAT_RESPONSE_DEV_EVENTS;
    pEventsMask->test       = pNewEventsMask->test      & VP_EVCAT_TEST_DEV_EVENTS;
    pEventsMask->process    = pNewEventsMask->process   & VP_EVCAT_PROCESS_DEV_EVENTS;
    pEventsMask->fxo        = pNewEventsMask->fxo       & VP_EVCAT_FXO_DEV_EVENTS;

    /* Zero out the device-specific bits before setting the
     * lineEventsMask in the line object. */
    pEventsMask = &pLineObj->lineEventsMask;

    pEventsMask->faults     = pNewEventsMask->faults    & ~VP_EVCAT_FAULT_DEV_EVENTS;
    pEventsMask->signaling  = pNewEventsMask->signaling & ~VP_EVCAT_SIGNALING_DEV_EVENTS;
    pEventsMask->response   = pNewEventsMask->response  & ~VP_EVCAT_RESPONSE_DEV_EVENTS;
    pEventsMask->test       = pNewEventsMask->test      & ~VP_EVCAT_TEST_DEV_EVENTS;
    pEventsMask->process    = pNewEventsMask->process   & ~VP_EVCAT_PROCESS_DEV_EVENTS;
    pEventsMask->fxo        = pNewEventsMask->fxo       & ~VP_EVCAT_FXO_DEV_EVENTS;

    /* Unmask the unmaskable defined in vp_api_common.c */
    VpImplementNonMaskEvents(&pLineObj->lineEventsMask, &pDevObj->deviceEventsMask);

    /* Mask those events that the VP890 API-II cannot generate */
    MaskNonSupportedEvents(&pLineObj->lineEventsMask, &pDevObj->deviceEventsMask);

    /*
     * The next code section prevents the device from interrupting
     * the processor if all of the events associated with the
     * specific hardware interrupt are masked
     */
    VpMpiCmdWrapper(deviceId, ecVal, VP890_INT_MASK_RD, VP890_INT_MASK_LEN, tempData);

    /* Keep Clock Fault Interrupt Enabled to support auto-free run mode. */
    tempData[0] &= ~VP890_CFAIL_MASK;

    if (!(pLineObj->status & VP890_IS_FXO)) {
    /* Line is FXS */
        /* Mask off the FXO events */
        pLineObj->lineEventsMask.fxo |= VP_EVCAT_FXO_MASK_ALL;

        /*
         * Never mask the thermal fault interrupt otherwise the
         * actual thermal fault may not be seen by the VP-API-II.
         */
        tempData[chanId] &= ~VP890_TEMPA_MASK;

        /* Evaluate for signaling events */
        eventMask = pLineObj->lineEventsMask.signaling;

        /*
         * Never mask the hook interrupt otherwise interrupt modes
         * of the VP-API-II for LPM types won't work -- hook status
         * is never updated, leaky line never properly detected.
         */
        tempData[chanId] &= ~VP890_HOOK_MASK;

        /*
         * Never mask the gkey interrupt otherwise interrupt modes
         * of the VP-API-II won't support "get line status"
         * correctly.
         */
        tempData[chanId] &= ~VP890_GNK_MASK;

        /* Disable Overcurrent Alaram on device until debounce put in place */
        tempData[chanId] &= ~(VP890_OCALMY_MASK);
    } else {
    /* Line is FXO */
        /* Mask off the FXS events */
        pLineObj->lineEventsMask.signaling |= VP890_FXS_SIGNALING_EVENTS;

        /* Evaluate for fxo events */
        eventMask = pLineObj->lineEventsMask.fxo;

        if ((eventMask & VP_LINE_EVID_RING_ON)
         && (eventMask & VP_LINE_EVID_RING_OFF)) {
            tempData[chanId] |= VP890_RING_DET_MASK;
        } else {
            tempData[chanId] &= ~VP890_RING_DET_MASK;
        }

        if ((eventMask & VP_LINE_EVID_LIU)
         && (eventMask & VP_LINE_EVID_LNIU)) {
            tempData[chanId] |= VP890_LIU_MASK;
        } else {
            tempData[chanId] &= ~VP890_LIU_MASK;
        }

        if (eventMask & VP_LINE_EVID_POLREV) {
            tempData[chanId] |= VP890_POL_MASK;
        } else {
            tempData[chanId] &= ~VP890_POL_MASK;
        }

        if ((eventMask & VP_LINE_EVID_DISCONNECT)
         && (eventMask & VP_LINE_EVID_RECONNECT)
         && (eventMask & VP_LINE_EVID_FEED_DIS)
         && (eventMask & VP_LINE_EVID_FEED_EN)) {
            tempData[chanId] |= VP890_LDN_MASK;
        } else {
            tempData[chanId] &= ~VP890_LDN_MASK;
        }

        /* Evaluate for line faults events */
        eventMask = pLineObj->lineEventsMask.faults;
        if (eventMask & VP_LINE_EVID_AC_FLT) {
            tempData[chanId] |= VP890_OVIR_MASK;
        } else {
            tempData[chanId] &= ~VP890_OVIR_MASK;
        }

        /* Workaround:  If VISTAT is masked, changing the FXO state can clear
         * existing interrupts.  Force it to be unmasked to avoid this.
         *
         * Specific error case:  Start with FXO offhook and disconnected.
         * Reconnect feed, this will pull the interrupt low.  Go onhook, and
         * the interrupt will go back to high if VISTAT is masked. */
        tempData[chanId] &= ~VP890_VISTAT_MASK;
    }
    VpMpiCmdWrapper(deviceId, ecVal, VP890_INT_MASK_WRT, VP890_INT_MASK_LEN, tempData);

    return;
} /* SetOptionEventMask() */

/*******************************************************************************
 * MaskNonSupportedEvents()
 *  This function masks the events that are not supported by the VP890 API-II.
 * It should only be called by SetOptionInternal when event masks are being
 * modified.
 *
 * Arguments:
 *   pLineEventsMask - Line Events Mask to modify for non-masking
 *   pDevEventsMask  - Device Events Mask to modify for non-masking
 *
 * Preconditions:
 *  None. Utility function to modify event structures only.
 *
 * Postconditions:
 *  Event structures passed are modified with masked bits for non-supported
 * VP890 API-II events.
 ******************************************************************************/
void
MaskNonSupportedEvents(
    VpOptionEventMaskType *pLineEventsMask,
    VpOptionEventMaskType *pDevEventsMask)
{
    VP_API_FUNC_INT(None, VP_NULL, ("+MaskNonSupportedEvents()"));

    pLineEventsMask->faults     |= VP890_NONSUPPORT_FAULT_EVENTS;
    pLineEventsMask->signaling  |= VP890_NONSUPPORT_SIGNALING_EVENTS;
    pLineEventsMask->response   |= VP890_NONSUPPORT_RESPONSE_EVENTS;
    pLineEventsMask->test       |= VP890_NONSUPPORT_TEST_EVENTS;
    pLineEventsMask->process    |= VP890_NONSUPPORT_PROCESS_EVENTS;
    pLineEventsMask->fxo        |= VP890_NONSUPPORT_FXO_EVENTS;

    pDevEventsMask->faults      |= VP890_NONSUPPORT_FAULT_EVENTS;
    pDevEventsMask->signaling   |= VP890_NONSUPPORT_SIGNALING_EVENTS;
    pDevEventsMask->response    |= VP890_NONSUPPORT_RESPONSE_EVENTS;
    pDevEventsMask->test        |= VP890_NONSUPPORT_TEST_EVENTS;
    pDevEventsMask->process     |= VP890_NONSUPPORT_PROCESS_EVENTS;
    pDevEventsMask->fxo         |= VP890_NONSUPPORT_FXO_EVENTS;

    return;
} /* MaskNonSupportedEvents() */

/*******************************************************************************
 * Vp890SetCodec()
 *  This function sets the codec mode on the line specified.
 *
 * Arguments:
 *   pLineCtx   - Line Context
 *   codec      - Encoding, as defined by LineCodec typedef
 *
 * Preconditions:
 *  The line must first be initialized.
 *
 * Postconditions:
 *  The codec mode on the line is set.  This function returns the success code
 * if the codec mode specified is supported.
 ******************************************************************************/
VpStatusType
Vp890SetCodec(
    VpLineCtxType           *pLineCtx,
    VpOptionCodecType       codec)
{
    Vp890LineObjectType     *pLineObj = pLineCtx->pLineObj;
    VpDevCtxType            *pDevCtx  = pLineCtx->pDevCtx;
    Vp890DeviceObjectType   *pDevObj  = pDevCtx->pDevObj;
    VpDeviceIdType          deviceId  = pDevObj->deviceId;

    uint8                   codecReg;
    uint8                   ecVal     = pLineObj->ecVal;

    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("+Vp890SetCodec()"));

    /* Basic error checking */
    if ((codec != VP_OPTION_LINEAR) &&
        (codec != VP_OPTION_ALAW)   &&
        (codec != VP_OPTION_MLAW)   &&
        (codec != VP_OPTION_WIDEBAND)) {

        VP_ERROR(VpLineCtxType, pLineCtx, ("Vp890SetCodec() - Invalid codec"));
        return VP_STATUS_INVALID_ARG;
    }

    /* Adjust the EC value for Wideband mode as needed */
    ecVal &= ~VP890_WIDEBAND_MODE;
    ecVal |= ((codec == VP_OPTION_WIDEBAND) ? VP890_WIDEBAND_MODE : 0);

    /*
     * Wideband requires 1/2 rate reduction in device programmed rate to
     * maintain the same real sample rate.
     */
    if(((pLineObj->codec == VP_OPTION_WIDEBAND) && (codec != VP_OPTION_WIDEBAND))
    || ((pLineObj->codec != VP_OPTION_WIDEBAND) && (codec == VP_OPTION_WIDEBAND))) {
        uint8 converterCfg[VP890_CONV_CFG_LEN];
        uint8 newValue;

        pDevObj->devTimer[VP_DEV_TIMER_WB_MODE_CHANGE] =
            MS_TO_TICKRATE(VP_WB_CHANGE_MASK_TIME,
            pDevObj->devProfileData.tickRate) | VP_ACTIVATE_TIMER;

        VpMpiCmdWrapper(deviceId, ecVal, VP890_CONV_CFG_RD, VP890_CONV_CFG_LEN,
            converterCfg);
        converterCfg[0] &= ~VP890_CC_RATE_MASK;

        VP_LINE_STATE(VpLineCtxType, pLineCtx, ("CODEC Change to %sWideband Mode at Time %d",
            ((codec != VP_OPTION_WIDEBAND) ? "non-" : ""), pDevObj->timeStamp));

        /* Adjust the pcm buffer update rate based on the tickrate and CODEC */
        if(pDevObj->devProfileData.tickRate <=160) {
            newValue = ((codec == VP_OPTION_WIDEBAND) ? VP890_CC_4KHZ_RATE : VP890_CC_8KHZ_RATE);
        } else if(pDevObj->devProfileData.tickRate <=320){
            newValue = ((codec == VP_OPTION_WIDEBAND) ? VP890_CC_2KHZ_RATE : VP890_CC_4KHZ_RATE);
        } else if(pDevObj->devProfileData.tickRate <=640){
            newValue = ((codec == VP_OPTION_WIDEBAND) ? VP890_CC_1KHZ_RATE : VP890_CC_2KHZ_RATE);
        } else if(pDevObj->devProfileData.tickRate <=1280){
            newValue = ((codec == VP_OPTION_WIDEBAND) ? VP890_CC_500HZ_RATE : VP890_CC_1KHZ_RATE);
        } else {
            newValue = VP890_CC_500HZ_RATE;
        }

        pDevObj->txBufferDataRate = newValue;
        converterCfg[0] |= newValue;

       /*
         * If channel is going to Wideband mode, we can immediately update the
         * device object. But if leaving Wideband mode, we have to let the tick
         * manage it because the other line may still be in Wideband mode.
         */
        if (codec == VP_OPTION_WIDEBAND) {
            pDevObj->ecVal |= VP890_WIDEBAND_MODE;
        }

        VpMpiCmdWrapper(deviceId, ecVal, VP890_CONV_CFG_WRT, VP890_CONV_CFG_LEN,
            converterCfg);
    }

    /* Read the current state of the codec register */
    VpMpiCmdWrapper(deviceId, ecVal, VP890_CODEC_REG_RD, VP890_CODEC_REG_LEN, &codecReg);

    /* Enable the desired CODEC mode */
    switch(codec) {
        /* Wideband same CODEC mode as Linear PCM */
        case VP_OPTION_WIDEBAND:
        case VP_OPTION_LINEAR:      /* 16 bit linear PCM */
            codecReg |= VP890_LINEAR_CODEC;
            break;

        case VP_OPTION_ALAW:                /* A-law PCM */
            codecReg &= ~(VP890_LINEAR_CODEC | VP890_ULAW_CODEC);
            break;

        case VP_OPTION_MLAW:                /* u-law PCM */
            codecReg |= VP890_ULAW_CODEC;
            codecReg &= ~(VP890_LINEAR_CODEC);
            break;

        default:
            /* Cannot reach here.  Error checking at top */
            break;
    } /* Switch */

    VP_INFO(VpLineCtxType, pLineCtx, ("1. Writing 0x%02X to Operating Functions EC 0x%02X Device %d",
        codecReg, ecVal, deviceId));
    VpMpiCmdWrapper(deviceId, ecVal, VP890_CODEC_REG_WRT, VP890_CODEC_REG_LEN, &codecReg);

    pLineObj->codec = codec;
    pLineObj->ecVal = ecVal;

    return VP_STATUS_SUCCESS;
} /* Vp890SetCodec() */

/*******************************************************************************
 * SetTimeSlot()
 *  This function set the RX and TX timeslot for a device channel. Valid
 * timeslot numbers start at zero. The upper bound is system dependent.
 *
 * Arguments:
 *   pLineCtx   - Line Context
 *   txSlot     - The TX PCM timeslot
 *   rxSlot     - The RX PCM timeslot
 *
 * Preconditions:
 *  The line must first be initialized.
 *
 * Postconditions:
 *  The timeslots on the line are set.  This function returns the success code
 * if the timeslot numbers specified are within the range of the device based on
 * the PCLK rate.
 ******************************************************************************/
static VpStatusType
SetTimeSlot(
    VpLineCtxType           *pLineCtx,
    uint8                   txSlot,
    uint8                   rxSlot)
{
    Vp890LineObjectType     *pLineObj = pLineCtx->pLineObj;
    VpDevCtxType            *pDevCtx  = pLineCtx->pDevCtx;
    Vp890DeviceObjectType   *pDevObj  = pDevCtx->pDevObj;
    VpDeviceIdType          deviceId  = pDevObj->deviceId;

    uint8                   ecVal     = pLineObj->ecVal;

    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("+SetTimeSlot()"));

    /* device only supports 127 time slots */
    if ((VP890_TX_TS_MAX < txSlot) || (VP890_RX_TS_MAX < rxSlot)) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("SetTimeSlot() - Time slot out of range.  rx %d, tx %d",
            rxSlot, txSlot));
        return VP_STATUS_INVALID_ARG;
    }

    /* Validate the tx time slot value */
    if(txSlot >= pDevObj->devProfileData.pcmClkRate / 64) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("SetTimeSlot() - Bad Tx time slot value"));
        return VP_STATUS_INVALID_ARG;
    }

    /* Validate the rx time slot value */
    if(rxSlot >= pDevObj->devProfileData.pcmClkRate / 64) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("SetTimeSlot() - Bad Rx time slot value"));
        return VP_STATUS_INVALID_ARG;
    }

    VpMpiCmdWrapper(deviceId, ecVal, VP890_TX_TS_WRT, VP890_TX_TS_LEN, &txSlot);

    VpMpiCmdWrapper(deviceId, ecVal, VP890_RX_TS_WRT, VP890_RX_TS_LEN, &rxSlot);

    return VP_STATUS_SUCCESS;
} /* SetTimeSlot() */

/*******************************************************************************
 * Vp890DeviceIoAccess()
 *  This function is used to access device IO pins of the Vp890. See API-II
 * documentation for more information about this function.
 *
 * Preconditions:
 *  Device/Line context should be created and initialized. For applicable
 * devices bootload should be performed before calling the function.
 *
 * Postconditions:
 *  Reads/Writes from device IO pins.
 ******************************************************************************/
VpStatusType
Vp890DeviceIoAccess(
    VpDevCtxType             *pDevCtx,
    VpDeviceIoAccessDataType *pDeviceIoData)
{
    Vp890DeviceObjectType    *pDevObj   = pDevCtx->pDevObj;
    VpLineCtxType            *pLineCtx;
    Vp890LineObjectType      *pLineObj;
    VpDeviceIdType           deviceId   = pDevObj->deviceId;
    uint8                    ecVal      = 0;
    uint8                    ioData;
    uint8                    chanId;
    VpStatusType             status     = VP_STATUS_SUCCESS;

    VpDeviceIoAccessDataType *pAccessData =
        &(pDevObj->getResultsOption.optionData.deviceIoData);

    VP_API_FUNC_INT(VpDevCtxType, pDevCtx, ("+Vp890DeviceIoAccess()"));

    /* Get out if device state is not ready */
    if (!Vp890IsDevReady(pDevObj->status.state, TRUE)) {
        return VP_STATUS_DEV_NOT_INITIALIZED;
    }

    VpSysEnterCritical(deviceId, VP_CODE_CRITICAL_SEC);

    /* Get wideband mode info from each channel's ecVal. */
    for(chanId=0; chanId < pDevObj->staticInfo.maxChannels; chanId++ ) {
        pLineCtx = pDevCtx->pLineCtx[chanId];
        if (pLineCtx != VP_NULL) {
            pLineObj = pLineCtx->pLineObj;
            ecVal |= pLineObj->ecVal;

            if ((pLineObj->termType == VP_TERM_FXS_SPLITTER_LP)
             || (pLineObj->termType == VP_TERM_FXS_SPLITTER)
             || (pLineObj->termType == VP_TERM_FXS_ISOLATE)) {
                if ((pDeviceIoData->accessMask_31_0 & 0x1)
                 && (pDeviceIoData->accessType == VP_DEVICE_IO_WRITE)) {
                    status = VP_STATUS_DEDICATED_PINS;
                }
                /* Can't let I/O1 Change on Splitter/Isolate Types */
                pDeviceIoData->accessMask_31_0 &= 0xfffffffe;
            }
        }
    }

    *pAccessData = *pDeviceIoData;

    VpMpiCmdWrapper(deviceId, ecVal, VP890_IODATA_REG_RD, VP890_IODATA_REG_LEN,
        &ioData);
    if (pDeviceIoData->accessType == VP_DEVICE_IO_READ) {
        pAccessData->deviceIOData_31_0 =
            ioData & pDeviceIoData->accessMask_31_0;
        pAccessData->deviceIOData_63_32 = 0;
    } else { /* pDeviceIoData->accessType == VP_DEVICE_IO_WRITE */
        ioData &= ~(pDeviceIoData->accessMask_31_0 & VP890_IODATA_IOMASK);
        ioData |= pDeviceIoData->deviceIOData_31_0 &
            (pDeviceIoData->accessMask_31_0 & VP890_IODATA_IOMASK);
        VpMpiCmdWrapper(deviceId, ecVal, VP890_IODATA_REG_WRT, VP890_IODATA_REG_LEN,
            &ioData);
    }
    pDevObj->deviceEvents.response |= VP_DEV_EVID_IO_ACCESS_CMP;

    VP_API_FUNC_INT(VpDevCtxType, pDevCtx, (" -Vp890DeviceIoAccess()"));
    VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
    return status;
} /* Vp890DeviceIoAccess() */

/*******************************************************************************
 * Vp890LowLevelCmd()
 *  This function provides direct MPI access to the line/device.
 *
 * Preconditions:
 *  The device associated with the line, and the line must first be initialized.
 *
 * Postconditions:
 *  The command data is passed over the MPI bus and affects only the line passed
 * if the command is line specific, and an event is generated.  If a read
 * command is performed, the user must read the results or flush events.  This

 * function returns the success code if the device is not already in a state
 * where the results must be read.
 ******************************************************************************/
VpStatusType
Vp890LowLevelCmd(
    VpLineCtxType       *pLineCtx,
    uint8               *pCmdData,
    uint8               len,
    uint16              handle)
{
    Vp890LineObjectType     *pLineObj = pLineCtx->pLineObj;
    VpDevCtxType            *pDevCtx  = pLineCtx->pDevCtx;
    Vp890DeviceObjectType   *pDevObj  = pDevCtx->pDevObj;
    VpDeviceIdType          deviceId  = pDevObj->deviceId;
    uint8                   ecVal     = pLineObj->ecVal;

    int i; /* For-loop var */

    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("+Vp890LowLevelCmd()"));

    if (pLineObj->lineEvents.response & VP890_READ_RESPONSE_MASK) {
        VP_WARNING(VpLineCtxType, pLineCtx, ("Vp890LowLevelCmd() - Waiting to clear previous read"));
        return VP_STATUS_DEVICE_BUSY;
    }

    VpSysEnterCritical(deviceId, VP_CODE_CRITICAL_SEC);
    if(pCmdData[0] & 0x01) { /* Read Command */
        VpMpiCmdWrapper(deviceId, ecVal, pCmdData[0], len, &(pDevObj->mpiData[0]));
        pDevObj->mpiLen = len;
        pLineObj->lineEvents.response |= VP_LINE_EVID_LLCMD_RX_CMP;
    } else {
        VpMpiCmdWrapper(deviceId, ecVal, pCmdData[0], len, &pCmdData[1]);
        for (i = 0; i < len; i++) {
            pDevObj->mpiData[i] = pCmdData[i];
        }
        pLineObj->lineEvents.response |= VP_LINE_EVID_LLCMD_TX_CMP;
    }
    pLineObj->lineEventHandle = handle;

    VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);

    return VP_STATUS_SUCCESS;
} /* Vp890LowLevelCmd() */

/*******************************************************************************
 * PllRecovery()
 *  This function is part of the software fix for the PLL recovery which appears
 * when the FXO line comes out of Disconnect. This solution monitors ICR1
 * for PLL detection, and when fails "flips the switch" to force a PLL restart.
 * This function is called from ServiceFxoTimers when timer pllRecovery expires.
 ******************************************************************************/
static void
PllRecovery(
    VpLineCtxType           *pLineCtx)
{
    Vp890LineObjectType     *pLineObj = pLineCtx->pLineObj;
    VpDevCtxType            *pDevCtx  = pLineCtx->pDevCtx;
    Vp890DeviceObjectType   *pDevObj  = pDevCtx->pDevObj;
    VpDeviceIdType          deviceId  = pDevObj->deviceId;
    uint8                   ecVal     = pLineObj->ecVal;
    uint8                   icr1Data[VP890_ICR1_LEN];

    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("+PllRecovery()"));

    /* Every state requires read of ICR1 */
    VpMpiCmdWrapper(deviceId, ecVal, VP890_ICR1_RD, VP890_ICR1_LEN, icr1Data);

    /* We're done if the PLL started ok, or if we corrected it */
    if ((icr1Data[0] == 0x00)
     && ((icr1Data[VP890_SUB_ST_IDX] & VP890_SUB_ST_MASK) == VP890_SUB_ST_FXO_ON_HOOK_SUP)) {
        /* Done. */

        if (pLineObj->pllRecoveryState != VP890_PLL_RECOVERY_ST_RESET) {
            VP_FXO_FUNC(VpLineCtxType, pLineCtx, ("PLL OK at time %d Starting Final Delay",
                pDevObj->timeStamp));

            pLineObj->pllRecoveryState = VP890_PLL_RECOVERY_ST_RESET;
            Vp890SetFxoLineState(pLineCtx, pLineObj->lineState.usrCurrent);

            /*
             * Final Delay to allow device level debouncers to stabilize under
             * worst case loop-open condition. The "140" comes from 140ms device
             * level filtering.
             */
            pLineObj->lineTimers.timers.fxoTimer.pllRecovery =
                MS_TO_TICKRATE((VP_FXO_DISCONNECT_DEBOUNCE+140),
                pDevObj->devProfileData.tickRate);
            return;
        }

        /*
         * Generate the Device Init Complete Event if this occurred as a result
         * of device level initialization. Clear the line initialization flag
         * as well in this case, but don't generate the line init complete event
         * (it is implied by device init complete).
         * Generate the Line Init Complete Event if this occurred as a result of
         * Line Init but not Device Init (which calls Line Init).
         */
        if (!(pDevObj->status.state & VP_DEV_INIT_CMP)) {
            VP_FXO_FUNC(None, NULL, ("Called From Init Device on Device %d. Generating Event at time %d.",
                deviceId, pDevObj->timeStamp));
            pDevObj->deviceEvents.response |= VP_DEV_EVID_DEV_INIT_CMP;
            pDevObj->status.state |= VP_DEV_INIT_CMP;
        } else if (!(pLineObj->status & VP890_INIT_COMPLETE)) {
            VP_FXO_FUNC(None, NULL, ("Called From Init Line on Device %d Channel %d. Generating Event at time %d.",
                deviceId, pLineObj->channelId, pDevObj->timeStamp));

            pLineObj->lineEvents.response |= VP_LINE_EVID_LINE_INIT_CMP;
        }
        pLineObj->status |= VP890_INIT_COMPLETE;

        return;
    }

    VP_FXO_FUNC(None, NULL, ("PLL FAILED -- 0x%02X 0x%02X 0x%02X 0x%02X at time %d",
        icr1Data[0], icr1Data[1], icr1Data[2], icr1Data[3], pDevObj->timeStamp));

    if(pLineObj->pllRecoveryState == VP890_PLL_RECOVERY_ST_RESET) {
        pLineObj->pllRecoveryState = VP890_PLL_RECOVERY_ST_DISABLE;
    }

    switch(pLineObj->pllRecoveryState) {
        case VP890_PLL_RECOVERY_ST_DISABLE:    {
            uint8 icr1allOff[] = {0xC0, 0x00, 0x00, 0x04};
            uint8 icr1analogEn[] = {0xC0, 0x80, 0x00, 0x04};

            VpMpiCmdWrapper(deviceId, ecVal, VP890_ICR1_WRT, VP890_ICR1_LEN, icr1allOff);
            VpMpiCmdWrapper(deviceId, ecVal, VP890_ICR1_WRT, VP890_ICR1_LEN, icr1analogEn);

            pLineObj->pllRecoveryState = VP890_PLL_RECOVERY_ST_ENABLE_1;
            pLineObj->lineTimers.timers.fxoTimer.pllRecovery =
                MS_TO_TICKRATE(VP890_PLL_RECOVER_INIT_DELAY,
                pDevObj->devProfileData.tickRate);
            }
            break;

        case VP890_PLL_RECOVERY_ST_ENABLE_1: {
            uint8 icr1rstOff[] = {0xC0, 0xC0, 0x00, 0x04};

            VpMpiCmdWrapper(deviceId, ecVal, VP890_ICR1_WRT, VP890_ICR1_LEN, icr1rstOff);

            pLineObj->pllRecoveryState = VP890_PLL_RECOVERY_ST_ENABLE_2;
            pLineObj->lineTimers.timers.fxoTimer.pllRecovery =
                MS_TO_TICKRATE(VP890_PLL_RECOVER_MEAS_DELAY_1,
                    pDevObj->devProfileData.tickRate);
            }
            break;

        case VP890_PLL_RECOVERY_ST_ENABLE_2: {
            uint8 icr1normal[] = {0x00, 0xC0, 0x00, 0x04};

            VpMpiCmdWrapper(deviceId, ecVal, VP890_ICR1_WRT, VP890_ICR1_LEN, icr1normal);

            pLineObj->pllRecoveryState = VP890_PLL_RECOVERY_ST_MEASURE;
            pLineObj->lineTimers.timers.fxoTimer.pllRecovery =
                MS_TO_TICKRATE(VP890_PLL_RECOVER_MEAS_DELAY_2,
                    pDevObj->devProfileData.tickRate);
            }
            break;

        case VP890_PLL_RECOVERY_ST_MEASURE:
            /*
             * If we're here, the device is in a failed state. Just trap on a
             * maximum number of tries.
             */
            if (pLineObj->pllRecoverAttempts < 10) {
                pLineObj->pllRecoverAttempts++;
                pLineObj->pllRecoveryState = VP890_PLL_RECOVERY_ST_DISABLE;

                /* Come back at very next tick */
                pLineObj->lineTimers.timers.fxoTimer.pllRecovery = 1;
            } else {
                /*
                 * Failed. Don't come back, but let the init complete events
                 * be generated.
                */
                if (!(pDevObj->status.state & VP_DEV_INIT_CMP)) {
                    VP_FXO_FUNC(None, NULL, ("Called From Init Device on Device %d. Generating Event.",
                        deviceId));
                    pDevObj->deviceEvents.response |= VP_DEV_EVID_DEV_INIT_CMP;
                    pDevObj->status.state |= VP_DEV_INIT_CMP;
                    pLineObj->status |= VP890_INIT_COMPLETE;
                } else if (!(pLineObj->status & VP890_INIT_COMPLETE)) {
                    VP_FXO_FUNC(None, NULL, ("Called From Init Line on Device %d Channel %d. Generating Event.",
                        deviceId, pLineObj->channelId));

                    pLineObj->lineEvents.response |= VP_LINE_EVID_LINE_INIT_CMP;
                    pLineObj->status |= VP890_INIT_COMPLETE;
                }
            }
            break;

        default:
            break;
    }
}

#endif /* VP_CC_890_SERIES */


