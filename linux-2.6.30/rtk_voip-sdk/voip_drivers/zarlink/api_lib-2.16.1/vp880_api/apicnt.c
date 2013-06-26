/** \file apicnt.c
 * apicnt.c
 *
 *  This file contains the control functions for the Vp880 device API.
 *
 * Copyright (c) 2010, Zarlink Semiconductor, Inc.
 *
 * $Revision: 7529 $
 * $LastChangedDate: 2010-07-21 15:03:32 -0500 (Wed, 21 Jul 2010) $
 */

#include "vp_api_cfg.h"

#if defined (VP_CC_880_SERIES)

/* INCLUDES */
#include "vp_api_types.h"
#include "vp_hal.h"
#include "vp_api_int.h"
#include "vp880_api.h"
#include "vp880_api_int.h"
#include "sys_service.h"

/**< Profile index for Generator A/B and C/D starting points (std tone) */
typedef enum
{
    VP880_SIGGEN_AB_START = 8,
    VP880_SIGGEN_CD_START = 16,
    VP880_SIGGEN_ENUM_SIZE = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} vp880_toneProfileParams;

/**< Functions called by Set Line State to abstract device states used for ABS
 * and non ABS devices (if they are different). Set line state then operates
 * on device state only (abstracted from API-II line state).
 */
static uint8
Vp880GetLineStateNonABS(
    VpLineCtxType *pLineCtx,
    VpLineStateType state);

static void
Vp880WriteLPEnterRegisters(
    VpLineCtxType *pLineOtx,
    VpDeviceIdType deviceId,
    uint8 ecVal,
    uint8 *lineState);

VpStatusType
Vp880SetFxoState(
    VpLineCtxType *pLineCtx,
    VpLineStateType state);

static bool
Vp880SetStateRinging(
    VpLineCtxType *pLineCtx,
    VpLineStateType state);

static void
Vp880ApplyInternalTestTerm(
    VpLineCtxType *pLineCtx);

static void
Vp880RemoveInternalTestTerm(
    VpLineCtxType *pLineCtx);

/**< Function called by Set Option only. Implements the options specified by
 * the user. The calling function implements the Device/Line control. If a line
 * option is set and a device option is passed, the calling function will call
 * this function once for each line and pass it the line contexts. Therefore,
 * this function will only be subjected to either a device context and device
 * option, or a line context and a line option.
 */
static VpStatusType
Vp880SetOptionInternal(
    VpLineCtxType *pLineCtx,
    VpDevCtxType *pDevCtx,
    VpOptionIdType option,
    void *value);

/* Function called by SetOptionInternal for Event Masking only */
static void
Vp880MaskNonSupportedEvents(
    VpOptionEventMaskType *pLineEventsMask,
    VpOptionEventMaskType *pDevEventsMask);

/* Function called by SetOptionInternal to set tx and rx timeslot */
static VpStatusType
Vp880SetTimeSlot(
    VpLineCtxType *pLineCtx,
    uint8 txSlot,
    uint8 rxSlot);

/**< Function called by Api Tick only. Processes the line passed for Api
 * Tick based operations
 */
static VpStatusType
Vp880ProcessFxsLine(
    Vp880DeviceObjectType *pDevObj,
    VpLineCtxType *pLineCtx);

static bool
Vp880OnHookMgmt(
    Vp880DeviceObjectType *pDevObj,
    VpLineCtxType *pLineCtx,
    uint8 ecVal);

static bool
Vp880OffHookMgmt(
    Vp880DeviceObjectType *pDevObj,
    VpLineCtxType *pLineCtx,
    uint8 ecVal);

static bool
Vp880ProcessFxoLine(
    Vp880DeviceObjectType *pDevObj,
    VpLineCtxType *pLineCtx);

#ifdef LEGACY_RINGING_DETECTION
static void
Vp880LowRingFreqDetect(
    VpLineCtxType *pLineCtx,
    VpCslacLineCondType *tempRingingSt,
    VpCslacLineCondType *tempPolRevSt,
    bool *retFlag);
#endif

/**< Function called by Api only to determine if a particular line is currently
 * running a test.
 */
static bool Vp880IsChnlUndrTst(
    Vp880DeviceObjectType *pDevObj,
    uint8 channelId);

bool
Vp880IsSupportedFxsState(
    VpLineStateType state);

/**
 * Vp880IsSupportedFxsState()
 *  This function checks to see if the state passed is a supproted FXS state of
 * the 880 API
 *
 * Preconditions:
 *  None.
 *
 * Postconditions:
 *  None.
 */
bool
Vp880IsSupportedFxsState(
    VpLineStateType state)
{
    switch (state) {
        case VP_LINE_STANDBY:
        case VP_LINE_ACTIVE:
        case VP_LINE_ACTIVE_POLREV:
        case VP_LINE_TIP_OPEN:
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
 * Vp880SetLineState()
 *  This function is the API-II wrapper function for Set Line State - Internal
 * for the Vp880 API.
 *
 * Preconditions:
 *  Same as Vp880SetLineStateInt()
 *
 * Postconditions:
 *  Same as Vp880SetLineStateInt()
 */
VpStatusType
Vp880SetLineState(
    VpLineCtxType *pLineCtx,
    VpLineStateType state)
{
    Vp880LineObjectType *pLineObj = pLineCtx->pLineObj;
    VpStatusType status;
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpDeviceIdType deviceId = pDevObj->deviceId;

    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880SetLineState+"));

    /* Proceed if device state is either in progress or complete */
    if (pDevObj->status.state & (VP_DEV_INIT_CMP | VP_DEV_INIT_IN_PROGRESS)) {
    } else {
        VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880SetLineState-"));
        return VP_STATUS_DEV_NOT_INITIALIZED;
    }

    /*
     * Do not proceed if the device calibration is in progress. This could
     * damage the device.
     */
    if (pDevObj->status.state & VP_DEV_IN_CAL) {
        VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880SetLineState-"));
        return VP_STATUS_DEV_NOT_INITIALIZED;
    }

    VpSysEnterCritical(deviceId, VP_CODE_CRITICAL_SEC);
    if(state == pLineObj->lineState.usrCurrent) {
        VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
        VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880SetLineState-"));
        return VP_STATUS_SUCCESS;
    }

    VP_LINE_STATE(VpLineCtxType, pLineCtx, ("Setting Device %d Channel %d to State %d at time %d",
        pDevObj->deviceId, pLineObj->channelId, state, pDevObj->timeStamp));

    /* Clear the "called from API" flag. This affects the cadencer */
    pLineObj->status &= ~(VP880_SLS_CALL_FROM_API);

    /*
     * Special FXS handling to prevent setting the line to ringing if
     * off-hook
     */
    if ((pLineObj->lineState.condition & VP_CSLAC_HOOK)
     && ((state == VP_LINE_RINGING_POLREV) || (state == VP_LINE_RINGING))) {
        /*
         * Go to Ring Trip Exit state instead, which could be ringing -- but
         * that's up to the application.
         */
        state = pLineObj->ringCtrl.ringTripExitSt;
    }

    /* Stop the entering ringing timer and restore swRegParam */
    if (pDevObj->ringParams.channelArray[pLineObj->channelId] == TRUE) {
        pDevObj->ringParams.channelArray[pLineObj->channelId] = FALSE;

        if (pDevObj->ringParams.channelArray[0] == FALSE) {
            if ((pDevObj->ringParams.swRegParam[1] & VP880_SWY_AUTOPOWER_MASK) ==
                VP880_SWY_AUTOPOWER_DIS) {
                pDevObj->ringParams.swRegParam[1] |= VP880_SWY_AUTOPOWER_DIS;
            } else {
                pDevObj->ringParams.swRegParam[1] &= ~VP880_SWY_AUTOPOWER_DIS;
            }
        }
        if (pDevObj->ringParams.channelArray[1] == FALSE) {
            if ((pDevObj->ringParams.swRegParam[1] & VP880_SWZ_AUTOPOWER_MASK) ==
                VP880_SWZ_AUTOPOWER_DIS) {
                pDevObj->ringParams.swRegParam[1] |= VP880_SWZ_AUTOPOWER_DIS;
            } else {
                pDevObj->ringParams.swRegParam[1] &= ~VP880_SWZ_AUTOPOWER_DIS;
            }
        }
        if ((pDevObj->ringParams.channelArray[0] == FALSE) &&
            (pDevObj->ringParams.channelArray[1] == FALSE)) {
            VP_LINE_STATE(VpLineCtxType, pLineCtx,("Vp880LimitInRushCurrent(): kill the timer"));
            pDevObj->devTimer[VP_DEV_TIMER_ENTER_RINGING] = 0;
        }
        VpMpiCmdWrapper(deviceId, pLineObj->channelId + 1, VP880_REGULATOR_PARAM_WRT,
            VP880_REGULATOR_PARAM_LEN, pDevObj->ringParams.swRegParam);
        VpMemCpy(pDevObj->swParams, pDevObj->ringParams.swRegParam, VP880_REGULATOR_PARAM_LEN);
    }

    status = Vp880SetLineStateInt(pLineCtx, state);
    if (status == VP_STATUS_SUCCESS) {
        /*
         * Reset the "Count" for leaky line conditions because there are some
         * normal state change conditions that will increment the count, therefore
         * causing exit of LP for non-leaky line
         */
        pLineObj->leakyLineCnt = 0;

        VP_LINE_STATE(VpLineCtxType, pLineCtx, ("Adjusting User State to %d on channel %d",
            state, pLineObj->channelId));

        pLineObj->lineState.usrCurrent = state;
    }

    /*
     * Set the "called from API" flag. Convenience for API functions so setting
     * this flag does not need to occur in multiple locations
     */
    pLineObj->status |= VP880_SLS_CALL_FROM_API;

    VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880SetLineState-"));

    return status;
}

/**
 * Vp880SetLineStateInt()
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
 */
VpStatusType
Vp880SetLineStateInt(
    VpLineCtxType *pLineCtx,    /**< Line context to change line state on */
    VpLineStateType state)      /**< The desired line state to set */
{
    uint8 userByte, currentStateByte, mpiData;
    uint8 mpiByte = 0;
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    VpStatusType status = VP_STATUS_SUCCESS;

    Vp880LineObjectType *pLineObj = pLineCtx->pLineObj;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    uint8 ecVal = pLineObj->ecVal;

    VpDeviceIdType deviceId = pDevObj->deviceId;
    VpLineStateType currentState = pLineObj->lineState.currentState;

    bool disconnectTimerSet = FALSE;
    bool feedToDisable = FALSE;
    bool polarityInversion = FALSE;

#ifdef CSLAC_SEQ_EN
    bool disableTones = TRUE;
#endif

    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880SetLineStateInt+"));

    /* Modify Operating conditions ONLY if not running calibration */
    if (pLineObj->lineState.calType == VP_CSLAC_CAL_NONE) {
        /*
         * Read the status of the Operating Conditions register so we can change
         * only the TX and RX if the line state is a non-communication mode.
         * This also performs the line type/state verification.
         */
        VpMpiCmdWrapper(deviceId, ecVal, VP880_OP_COND_RD, VP880_OP_COND_LEN,
            &mpiData);
        mpiData &= ~(VP880_CUT_TXPATH | VP880_CUT_RXPATH);
        mpiData &= ~(VP880_HIGH_PASS_DIS | VP880_OPCOND_RSVD_MASK);

        status = Vp880GetTxRxPcmMode(pLineObj, state, &mpiByte);
        if (status == VP_STATUS_SUCCESS) {
            mpiData |= mpiByte;
        } else {
            VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880SetLineStateInt-"));
            return status;
        }

        VP_LINE_STATE(VpLineCtxType, pLineCtx, ("2. Writing 0x%02X to Operating Conditions", mpiData));
        VpMpiCmdWrapper(deviceId, ecVal, VP880_OP_COND_WRT, VP880_OP_COND_LEN,
            &mpiData);
    }

#ifdef CSLAC_SEQ_EN
    /* We're no longer in the middle of a time function */
    pLineObj->cadence.status &= ~VP_CADENCE_STATUS_MID_TIMER;
    pLineObj->cadence.timeRemain = 0;
#endif
    /*
     * If this function is called by the application, stop the cadencer and
     * reset the Loop Supervision if it is incorrect
     */

    if (!(pLineObj->status & VP880_SLS_CALL_FROM_API)) {
        uint8 sigGenCtrl[VP880_GEN_CTRL_LEN];
#ifdef CSLAC_SEQ_EN

        /* If we're in the middle of active cadence, terminate it */
        if ((pLineObj->cadence.status & VP_CADENCE_STATUS_METERING)
         && (pLineObj->cadence.status & VP_CADENCE_STATUS_ACTIVE)) {
            pLineObj->lineEvents.process |= VP_LINE_EVID_MTR_ABORT;
            pLineObj->processData = pLineObj->cadence.meteringBurst;
        }
#endif
        if (pLineObj->status & VP880_BAD_LOOP_SUP) {
            pLineObj->status &= ~(VP880_BAD_LOOP_SUP);
            VpMpiCmdWrapper(deviceId, ecVal, VP880_LOOP_SUP_WRT,
                VP880_LOOP_SUP_LEN, pLineObj->loopSup);
        }

        /*
         * Disable tones and cadencing if going to a state that prevents it and
         * in all cases for the FXO line
         */
        switch(state) {
            case VP_LINE_STANDBY:
            case VP_LINE_STANDBY_POLREV:
            case VP_LINE_TIP_OPEN:
            case VP_LINE_DISCONNECT:
            case VP_LINE_RINGING:
            case VP_LINE_RINGING_POLREV:
            case VP_LINE_FXO_LOOP_OPEN:
            case VP_LINE_FXO_LOOP_CLOSE:
            case VP_LINE_FXO_RING_GND:
            case VP_LINE_FXO_OHT:
            case VP_LINE_FXO_TALK:
              /*
                * Disable signal generator A/B/C/D before making any changes and
                * stop previous cadences.
                */
                VpMpiCmdWrapper(deviceId, ecVal, VP880_GEN_CTRL_RD,
                   VP880_GEN_CTRL_LEN, sigGenCtrl);
                sigGenCtrl[0] &= ~(VP880_GEN_ALLON | VP880_GEN_CTRL_EN_BIAS);
                VpMpiCmdWrapper(deviceId, ecVal, VP880_GEN_CTRL_WRT, VP880_GEN_CTRL_LEN,
                   sigGenCtrl);
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

    /* FXO TYPE LINE HANDLING */
    if (pLineObj->status & VP880_IS_FXO) {
        status = Vp880SetFxoState(pLineCtx, state);
        if (status == VP_STATUS_SUCCESS) {
            pLineObj->lineState.previous = currentState;
            pLineObj->lineState.currentState = state;
        }
    } else {  /* FXS Handling */
        if (pDevObj->stateInt & VP880_IS_ABS) {
            userByte = Vp880GetLineStateABS(pLineCtx, state);
        } else {
            userByte = Vp880GetLineStateNonABS(pLineCtx, state);
        }

        if (userByte == 0xFF) {
            VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880SetLineStateInt-"));
            return VP_STATUS_INVALID_ARG;
        }

        /* Modify userByte depending on the current polarity */
        VpMpiCmdWrapper(deviceId, ecVal, VP880_SYS_STATE_RD,
            VP880_SYS_STATE_LEN, &currentStateByte);
#ifdef CSLAC_SEQ_EN
        if (pLineObj->cadence.pActiveCadence != VP_NULL) {
            if ((pLineObj->cadence.status &
                (VP_CADENCE_STATUS_ACTIVE | VP_CADENCE_STATUS_IGNORE_POLARITY)) ==
                (VP_CADENCE_STATUS_ACTIVE | VP_CADENCE_STATUS_IGNORE_POLARITY)) {

                userByte &= ~VP880_SS_POLARITY_MASK;
                userByte |= (currentStateByte & VP880_SS_POLARITY_MASK);
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
            if (Vp880SetStateRinging(pLineCtx, state) == TRUE) {
                VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880SetLineStateInt-"));
                return VP_STATUS_SUCCESS;
            }
        }
        if (((pLineObj->termType == VP_TERM_FXS_LOW_PWR) ||
            (pLineObj->termType == VP_TERM_FXS_ISOLATE_LP) ||
            (pLineObj->termType == VP_TERM_FXS_SPLITTER_LP))
            && (pLineObj->lineTimers.timers.timer[VP_LINE_DISCONNECT_EXIT]
                & VP_ACTIVATE_TIMER)) {
            pLineObj->nextSlicValue = userByte;

            if ((pLineObj->lineState.currentState == VP_LINE_STANDBY) &&
               ((state != VP_LINE_STANDBY) && (state != VP_LINE_DISCONNECT))) {
                /*
                 * Disconnect Exit to LPM-Standby was started but we're now
                 * changing to non-LPM. Need to correct ICR values.
                 */
                Vp880SetLPRegisters(pLineObj, FALSE);
                Vp880WriteLPExitRegisters(pLineCtx, deviceId, pLineObj->ecVal,
                    &userByte);
                Vp880LowPowerMode(pDevCtx);
            }
        }

        /*
         * Enable Disconnect Recovery time for hook status if going FROM
         * Disconnect to a state that can detect off-hook
         */
        if (pLineObj->lineState.currentState == VP_LINE_DISCONNECT) {
            /* Coming from Disconnect...*/
            switch (state) {
                case VP_LINE_DISCONNECT:
                case VP_LINE_RING_OPEN:
                    break;

                default:
                    /* ..going to a state that can detect some feed condition */
                    if (!(pLineObj->lineState.condition & VP_CSLAC_LINE_LEAK_TEST)) {
                        VP_LINE_STATE(VpLineCtxType, pLineCtx, ("Setting Disconnect Recovery Timer on channel %d at time %d line state conditions: 0x%04X",
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
        if ((state == VP_LINE_DISCONNECT)
         || (state == VP_LINE_TIP_OPEN)
         || (state == VP_LINE_RING_OPEN)) {
            feedToDisable = TRUE;
        }

        /*
         * Set Polarity Reverse timer if the SLIC is changing polarity. Exclude
         * Disconnect type recovery conditions since a timer is set above for
         * those conditions.
         */
        if ((currentStateByte & VP880_SS_POLARITY_MASK)
         != (userByte & VP880_SS_POLARITY_MASK)) {
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

                if (userByte & VP880_SS_POLARITY_MASK) {
                    VpMpiCmdWrapper(deviceId, ecVal, VP880_DC_FEED_WRT,
                        VP880_DC_FEED_LEN, pLineObj->calLineData.dcFeedPr);
                } else {
                    VpMpiCmdWrapper(deviceId, ecVal, VP880_DC_FEED_WRT,
                        VP880_DC_FEED_LEN, pLineObj->calLineData.dcFeed);
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

        VP_LINE_STATE(VpLineCtxType, pLineCtx, ("In Vp880SetLineStateInt() Current state %d, next State %d Chan %d at time %d",
            currentState, state, pLineObj->channelId, pDevObj->timeStamp));
        if (((pLineObj->termType == VP_TERM_FXS_LOW_PWR) ||
             (pLineObj->termType == VP_TERM_FXS_ISOLATE_LP) ||
             (pLineObj->termType == VP_TERM_FXS_SPLITTER_LP))
             /*
              * If going from/to Disconnect/Standby in LPM, states are the same
              * so don't continue.
              */
         && ((state == VP_LINE_DISCONNECT) || (currentState == VP_LINE_DISCONNECT))) {
            if (currentState == state) {
                VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880SetLineStateInt-"));
                return VP_STATUS_SUCCESS;
            } else {
                if (currentState == VP_LINE_TIP_OPEN) {
                    Vp880GroundStartProc(FALSE, pLineCtx, currentStateByte, userByte);
                }

                if (state == VP_LINE_DISCONNECT) {
                    Vp880RunLPDisc(pLineCtx, TRUE, userByte);
                } else {
                    Vp880RunLPDisc(pLineCtx, FALSE, userByte);
                }
            }
        } else {
            if ((userByte & VP880_SS_LINE_FEED_MASK) == VP880_SS_TIP_OPEN) {
                Vp880GroundStartProc(TRUE, pLineCtx, currentStateByte, userByte);
            } else {
                Vp880GroundStartProc(FALSE, pLineCtx, currentStateByte, userByte);
            }
        }
    }

    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880SetLineStateInt-"));

    return status;
}

/**
 * Vp880RunLPDisc()
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
Vp880RunLPDisc(
    VpLineCtxType *pLineCtx,
    bool discMode,
    uint8 nextSlicByte)
{
    Vp880LineObjectType *pLineObj = pLineCtx->pLineObj;
    uint8 channelId = pLineObj->channelId;
    uint8 ecVal = pLineObj->ecVal;

    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpDeviceIdType deviceId = pDevObj->deviceId;

    /* devState "set" means the other line is in a LPM state. */
    Vp880DeviceStateIntType devState = (channelId == 0) ?
        (pDevObj->stateInt & VP880_LINE1_LP) :
        (pDevObj->stateInt & VP880_LINE0_LP);

    /*
     * myDevState "set" means *this* line is in a LPM state prior to calling
     * this function.
     */
    Vp880DeviceStateIntType myDevState = (channelId == 0) ?
        (pDevObj->stateInt & VP880_LINE0_LP) :
        (pDevObj->stateInt & VP880_LINE1_LP);

    /*
     * Enter/Exit Disconnect uses Active (w/Polarity) to cause fast charge or
     * discharge of the line.
     */
    uint8 lineState[] = {VP880_SS_ACTIVE};

    bool lineInTest = Vp880IsChnlUndrTst(pDevObj, channelId);
    uint16 leakyLine = pLineObj->status & VP880_LINE_LEAK;
    bool hookStatus;

    VpCSLACGetLineStatus(pLineCtx, VP_INPUT_RAW_HOOK, &hookStatus);
    leakyLine = (hookStatus == FALSE) ? leakyLine : VP880_LINE_LEAK ;

    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880RunLPDisc+"));

    /*
     * The other line is not really in LPM if this line is in test or has a
     * leaky line condition.
     */
    devState = (lineInTest == TRUE) ?  0 : devState;
    devState = ((leakyLine == VP880_LINE_LEAK) ? 0 : devState);

    if (discMode == TRUE) {        /* Entering Disconnect */
        VP_LINE_STATE(VpLineCtxType, pLineCtx, ("Entering Disconnect on Chan %d time %d",
            channelId, pDevObj->timeStamp));

        /*
         * There are three cases to consider when entering Disconnect:
         *     1. This line is coming from LPM-Standby and other line is in LPM.
         *     2. This line is coming from a non-LPM state and other line is in LPM.
         *     3. This line is coming from any state and other line is not in LPM.
         *        Condition #3 occurs also if this line has a resistive leak.
         *
         *  All cases require the ICR values modified to disable the switcher.
          */

        /*
         * Step 1: Program all ICR registers including Disable Switcher. Note
         * these are writing to cached values only, not to the device -- yet.
         */

        Vp880SetLPRegisters(pLineObj, TRUE);
        pLineObj->icr2Values[VP880_ICR2_SENSE_INDEX] &= ~VP880_ICR2_ILA_DAC;
        pLineObj->icr2Values[VP880_ICR2_SENSE_INDEX] |= VP880_ICR2_VOC_DAC_SENSE;
        pLineObj->icr2Values[VP880_ICR2_SENSE_INDEX+1] &= ~VP880_ICR2_VOC_DAC_SENSE;

        pLineObj->icr2Values[VP880_ICR2_SWY_CTRL_INDEX] |= VP880_ICR2_SWY_CTRL_EN;
        pLineObj->icr2Values[VP880_ICR2_SWY_CTRL_INDEX+1] &= ~VP880_ICR2_SWY_CTRL_EN;

        /*
         * Always start by disabling the switcher. Remaining steps will be done
         * either in this function (immediate) or delayed. In all cases, the
         * final state of Disconnect is delayed in order to allow the supply
         * to discharge once disabled.
         */
        VP_LINE_STATE(VpLineCtxType, pLineCtx, ("Entering Disconnect: Channel %d: Writing ICR2 0x%02X 0x%02X 0x%02X 0x%02X",
            pLineObj->channelId,
            pLineObj->icr2Values[0], pLineObj->icr2Values[1],
            pLineObj->icr2Values[2], pLineObj->icr2Values[3]));

        VpMpiCmdWrapper(deviceId, ecVal, VP880_ICR2_WRT, VP880_ICR2_LEN,
            pLineObj->icr2Values);

        /*
         * Force Tip and Ring Bias Override and set values to max. This forces
         * values toward ground. Temporarily remove the line bias override until
         * after the SLIC state has been set.
         */

        pLineObj->icr1Values[VP880_ICR1_BIAS_OVERRIDE_LOCATION] |=
            (VP880_ICR1_TIP_BIAS_OVERRIDE);
        pLineObj->icr1Values[VP880_ICR1_BIAS_OVERRIDE_LOCATION+1] |=
            (VP880_ICR1_TIP_BIAS_OVERRIDE);
        pLineObj->icr1Values[VP880_ICR1_BIAS_OVERRIDE_LOCATION] &=
            ~VP880_ICR1_LINE_BIAS_OVERRIDE;

        pLineObj->icr1Values[VP880_ICR1_RING_BIAS_OVERRIDE_LOCATION] |=
            VP880_ICR1_RING_BIAS_OVERRIDE;
        pLineObj->icr1Values[VP880_ICR1_RING_BIAS_OVERRIDE_LOCATION+1] |=
            VP880_ICR1_RING_BIAS_OVERRIDE;

        VP_LINE_STATE(VpLineCtxType, pLineCtx, ("Entering Disconnect: Channel %d: Writing ICR1 0x%02X 0x%02X 0x%02X 0x%02X",
            pLineObj->channelId,
            pLineObj->icr1Values[0], pLineObj->icr1Values[1],
            pLineObj->icr1Values[2], pLineObj->icr1Values[3]));

        Vp880ProtectedWriteICR1(pLineObj, deviceId, pLineObj->icr1Values);

        if ((devState) && (myDevState)) {
            /*
             * 1. This line is coming from LPM-Standby and other line is in LPM.
             *      Step 1: Program all ICR registers including Disable Switcher. - done
             *      Step 2: Set line to Active (for fast discharge)
             *      Step 3: Delay, then set line to VP_LINE_DISCONNECT. - outside
             *              this if statement.
             */
            VP_LINE_STATE(VpLineCtxType, pLineCtx, ("Entering Disconnect: Channel %d: Case 1 State 0x%02X",
                pLineObj->channelId, lineState[0]));

            /*
             * Step 2: Line is in LPM-Standby (i.e., Disconnect) so need to
             * set to Active to cause fast discharge
             */

            Vp880UpdateBufferChanSel(pDevObj, channelId, lineState[0]);
            VpMpiCmdWrapper(deviceId, ecVal, VP880_SYS_STATE_WRT,
                VP880_SYS_STATE_LEN, lineState);
        } else if ((devState) && (!myDevState)) {
            /*
             * 2. This line is coming from a non-LPM state and other line is in LPM.
             *      Step 1: Program all ICR registers including Disable Switcher. - done
             *      Step 2: Set line to Active w/Polarity (for fast discharge)
             *      Step 3: Delay, then set device to LPM.
             */

            /*
             * Step 2: Set line to Active w/Polarity (for fast discharge). Note
             * that the state may already be in a condition that will allow fast
             * discharge, but it could also be in Tip Open or Ringing or "other"
             * so we just want to be sure it's in a state that we know.
             */

            VpMpiCmdWrapper(deviceId, ecVal, VP880_SYS_STATE_RD,
                VP880_SYS_STATE_LEN, lineState);

            lineState[0] &= ~VP880_SS_LINE_FEED_MASK;
            lineState[0] |= VP880_SS_ACTIVE;

            VP_LINE_STATE(VpLineCtxType, pLineCtx, ("Entering Disconnect: Channel %d: Case 2 State 0x%02X",
                pLineObj->channelId, lineState[0]));

            Vp880UpdateBufferChanSel(pDevObj, channelId, lineState[0]);
            VpMpiCmdWrapper(deviceId, ecVal, VP880_SYS_STATE_WRT,
                VP880_SYS_STATE_LEN, lineState);

            pLineObj->nextSlicValue = VP880_SS_DISCONNECT;
            pLineObj->nextSlicValue |= ((lineInTest == TRUE) ? VP880_SS_ACTIVATE_MASK : 0);

            /* Step 3: Delay, then set device to LPM. */
            pLineObj->status |= VP880_HAL_DELAY;
            VP_LINE_STATE(VpLineCtxType, pLineCtx, ("Delaying Disconnect Enter Channel %d at time %d from Line State 0x%04X Other Line 0x%04X",
                channelId, pDevObj->timeStamp, myDevState, devState));
        } else {
            /*
             * 3. This line is coming from any state and other line is not in LPM.
             *      Step 1: Program all ICR registers including Disable Switcher. - done
             *      Step 2: Set line to Active w/polarity (for fast discharge)
             *      Step 3: Delay, then set line to VP_LINE_DISCONNECT. - outside
             *              this if statement.
             */

            /* Step 2: Set line to Active w/polarity (for fast discharge) */
            VpMpiCmdWrapper(deviceId, ecVal, VP880_SYS_STATE_RD,
                VP880_SYS_STATE_LEN, lineState);

            lineState[0] &= ~VP880_SS_LINE_FEED_MASK;
            lineState[0] |= VP880_SS_ACTIVE;

            VP_LINE_STATE(VpLineCtxType, pLineCtx, ("Entering Disconnect: Channel %d: Case 3 State 0x%02X",
                pLineObj->channelId, lineState[0]));

            Vp880UpdateBufferChanSel(pDevObj, channelId, lineState[0]);
            VpMpiCmdWrapper(deviceId, ecVal, VP880_SYS_STATE_WRT,
                VP880_SYS_STATE_LEN, lineState);

            /*
             * This writes the ICR AND System State Register. No delay will
             * occur (except the delay at end of Disonnect timer.
             */
            Vp880WriteLPEnterRegisters(pLineCtx, deviceId, ecVal,
                lineState);
        }

        if ((devState) && (!myDevState)) {
            /* Only condition where LPM tick is handling final sequence. */
        } else {
            /* Step 3: Delay, then set line to VP_LINE_DISCONNECT. */
            pLineObj->nextSlicValue = VP880_SS_DISCONNECT;
            pLineObj->nextSlicValue |= ((lineInTest == TRUE) ? VP880_SS_ACTIVATE_MASK : 0);

            /* Set Discharge Time based on Supply Configuration. */
            pLineObj->lineTimers.timers.timer[VP_LINE_DISCONNECT_EXIT] =
                (MS_TO_TICKRATE(Vp880SetDiscTimers(pDevObj),
                pDevObj->devProfileData.tickRate)) | VP_ACTIVATE_TIMER;
        }

        Vp880LLSetSysState(deviceId, pLineCtx, VP880_SS_DISCONNECT, FALSE);

        /* Restore the line bias override */
        pLineObj->icr1Values[VP880_ICR1_BIAS_OVERRIDE_LOCATION] |=
            VP880_ICR1_LINE_BIAS_OVERRIDE;
        pLineObj->icr1Values[VP880_ICR1_BIAS_OVERRIDE_LOCATION+1] |=
            VP880_ICR1_LINE_BIAS_OVERRIDE;
        Vp880ProtectedWriteICR1(pLineObj, deviceId, pLineObj->icr1Values);

    } else {    /* Exiting Disconnect */
        VP_LINE_STATE(VpLineCtxType, pLineCtx, ("Recovering Chan %d from DISCONNECT at time %d with value 0x%02X",
            channelId, pDevObj->timeStamp, nextSlicByte));

        /*
         * There are four cases to consider when exiting Disconnect:
         *     1. This line is going to VP_LINE_STANDBY and other line is in LPM.
         *     2. This line is going to VP_LINE_STANDBY and other line is not in LPM.
         *     3. This line is going to non-LPM state and other line is in LPM.
         *     4. This line is going to non-LPM state and other line is not in LPM.
         */
        pLineObj->icr2Values[VP880_ICR2_VOC_DAC_INDEX] &= ~VP880_ICR2_VOC_DAC_SENSE;
        pLineObj->icr2Values[VP880_ICR2_SWY_CTRL_INDEX] &= ~VP880_ICR2_SWY_CTRL_EN;
        pLineObj->icr2Values[VP880_ICR2_SENSE_INDEX] &= ~VP880_ICR2_ILA_DAC;

        if ((nextSlicByte == VP880_SS_IDLE) && (devState)) {
            uint8 swParams[VP880_REGULATOR_PARAM_LEN];

            VP_LINE_STATE(VpLineCtxType, pLineCtx, ("Case 1: Channel %d Line Status 0x%04X",
                channelId, pLineObj->status));

            /*
             * 1. This line is going to VP_LINE_STANDBY and other line is in LPM.
             *      Step 1: Enable Switcher.
             *      Step 2: Set line to Active (for fast charge)
             *      Step 3: Set line to SLIC-Disconnect (end of timer)
             */
            pLineObj->icr2Values[VP880_ICR2_SENSE_INDEX] =
                (VP880_ICR2_TIP_SENSE | VP880_ICR2_RING_SENSE);

            VpMpiCmdWrapper(deviceId, ecVal, VP880_REGULATOR_PARAM_RD,
                VP880_REGULATOR_PARAM_LEN, swParams);
            if ((swParams[VP880_FLOOR_VOLTAGE_BYTE] & 0x0D) != 0x0D) {
                swParams[VP880_FLOOR_VOLTAGE_BYTE] &= ~VP880_FLOOR_VOLTAGE_MASK;
                swParams[VP880_FLOOR_VOLTAGE_BYTE] |= 0x0D;   /* 70V */
                VpMpiCmdWrapper(deviceId, ecVal, VP880_REGULATOR_PARAM_WRT,
                    VP880_REGULATOR_PARAM_LEN, swParams);
                /*
                 * If we have to turn on the supply here, wait longer than the
                 * normal disconnect exit time. Normal time is based only on
                 * line stability into a 5REN load. Added time for switcher to
                 * fully charge the supply caps/line.
                 */
                pDevObj->devTimer[VP_DEV_TIMER_LP_CHANGE] =
                    (MS_TO_TICKRATE(Vp880SetDiscTimers(pDevObj),
                     pDevObj->devProfileData.tickRate)) | VP_ACTIVATE_TIMER;
            }

            Vp880WriteLPEnterRegisters(pLineCtx, deviceId, ecVal, lineState);

            if (pLineObj->status & VP880_LINE_LEAK) {
                pLineObj->nextSlicValue = VP880_SS_IDLE;
            } else {
                pLineObj->nextSlicValue = VP880_SS_DISCONNECT;
                /*
                 * Done entering LPM (exept for final state setting, so set
                 * line flag so the hook bit is not mis-interpreted.
                 */
                pLineObj->status |= VP880_LOW_POWER_EN;
            }
        } else if ((nextSlicByte == VP880_SS_IDLE) && (!(devState))) {

            /*
             * 2. This line is going to VP_LINE_STANDBY and other line is not in LPM.
             *      Step 1: Enable Switcher on this line
             *      Step 2: Set line to Active (for fast charge)
             *      Step 3: Set remaining ICR registers for LPM exit settings.
             *      Step 4: Set line to SLIC-IDLE (end of timer)
             */
            VpMpiCmdWrapper(deviceId, ecVal, VP880_ICR2_WRT, VP880_ICR2_LEN,
                pLineObj->icr2Values);

            VP_LINE_STATE(VpLineCtxType, pLineCtx, ("Case 2: Channel %d: Writing ICR2 0x%02X 0x%02X 0x%02X 0x%02X",
                pLineObj->channelId,
                pLineObj->icr2Values[0], pLineObj->icr2Values[1],
                pLineObj->icr2Values[2], pLineObj->icr2Values[3]));

            VP_LINE_STATE(VpLineCtxType, pLineCtx, ("Case 2: Setting Ch %d to State 0x%02X at time %d",
                channelId, lineState[0], pDevObj->timeStamp));

            Vp880UpdateBufferChanSel(pDevObj, channelId, lineState[0]);
            VpMpiCmdWrapper(deviceId, ecVal, VP880_SYS_STATE_WRT,
                VP880_SYS_STATE_LEN, lineState);

            pLineObj->nextSlicValue = nextSlicByte;

            VP_LINE_STATE(VpLineCtxType, pLineCtx, ("Force User State to VP_LINE_STANDBY on channel %d",
                pLineObj->channelId));

            pLineObj->lineState.usrCurrent = VP_LINE_STANDBY;

            Vp880SetLP(FALSE, pLineCtx);
        } else if ((nextSlicByte != VP880_SS_IDLE) && (devState)) {
            /*
             * 3. This line is going to non-LPM state and other line is in LPM.
             *      Step 1: Do nothing. Set the device flag to start LPM exit.
             */
            VP_LINE_STATE(VpLineCtxType, pLineCtx, ("Case 3: Delaying Disconnect Exit Channel %d at time %d Slic State 0x%02X",
                channelId, pDevObj->timeStamp, nextSlicByte));
            Vp880LLSetSysState(deviceId, pLineCtx, nextSlicByte, FALSE);
        } else {
            /*
             * 4. This line is going to non-LPM state and other line is not in LPM.
             *      Step 1: Enable Switcher on this line
             *      Step 2: Set line to Active w/Polarity (for fast charge)
             *      Step 3: Set remaining ICR registers for LPM exit settings.
             *      Step 4: Set line to new SLIC state (end of timer)
             */
            Vp880SetLPRegisters(pLineObj, FALSE);   /* ICR1, 3, 4 correct */

            VpMpiCmdWrapper(deviceId, ecVal, VP880_ICR2_WRT, VP880_ICR2_LEN,
                pLineObj->icr2Values);

            VP_LINE_STATE(VpLineCtxType, pLineCtx, ("Case 4: Channel %d: Writing ICR2 0x%02X 0x%02X 0x%02X 0x%02X",
                pLineObj->channelId,
                pLineObj->icr2Values[0], pLineObj->icr2Values[1],
                pLineObj->icr2Values[2], pLineObj->icr2Values[3]));

            /* Perform the SLIC Line State change */
            lineState[0] |= (VP880_SS_POLARITY_MASK & nextSlicByte);
            pLineObj->nextSlicValue = nextSlicByte;

            VP_LINE_STATE(VpLineCtxType, pLineCtx, ("Case 4: Setting Ch %d to State 0x%02X at time %d",
                channelId, lineState[0], pDevObj->timeStamp));

            Vp880UpdateBufferChanSel(pDevObj, channelId, lineState[0]);
            VpMpiCmdWrapper(deviceId, ecVal, VP880_SYS_STATE_WRT,
                VP880_SYS_STATE_LEN, lineState);

            /* Complete Remaining Disconnect Exit Sequence */
            Vp880WriteLPExitRegisters(pLineCtx, deviceId, ecVal, lineState);
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

    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880RunLPDisc-"));
}

/**
 * Vp880SetDiscTimers()
 *  This function provides the value for Disconnect Timing using LPM.
 *
 * Preconditions:
 *  None.
 *
 * Postconditions:
 *  None. Returns a value in ms.
 */
uint16
Vp880SetDiscTimers(
    Vp880DeviceObjectType *pDevObj)
{
    /*
     * Only two options -- Fixed Ringing (long discharge), or Tracked Ringing
     * (short discharge).
     */
    if (pDevObj->swParams[VP880_REGULATOR_TRACK_INDEX]
        & VP880_REGULATOR_FIXED_RING) {
        /*
         * Longest is using "fixed" ringing mode because the external
         * capacitors are generally very large.
         */
        return VP880_FIXED_TRACK_DISABLE_TIME;
    } else {
        return VP880_INVERT_BOOST_DISABLE_TIME;
    }
}

/**
 * Vp880SetStateRinging()
 *  This function starts cadence and non-cadence ringing, as well as ringing
 * exit.
 *
 * Preconditions:
 *  None. Calling function must know that this code should execute.
 *
 * Postconditions:
 *  Line object is modified if ringing cadence or exiting (timers). If not
 * cadencing and ringing is started, then return TRUE. Otherwise return FALSE.
 */
bool
Vp880SetStateRinging(
    VpLineCtxType *pLineCtx,
    VpLineStateType state)
{
    Vp880LineObjectType *pLineObj = pLineCtx->pLineObj;
    uint8 ecVal = pLineObj->ecVal;
    uint8 channelId = pLineObj->channelId;

    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpDeviceIdType deviceId = pDevObj->deviceId;

#ifdef CSLAC_SEQ_EN
    VpProfilePtrType pProfile;
#endif
    uint8 tempRingCtrlData = VP880_GEN_ALLOFF;

    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880SetStateRinging+"));

    if ((state == VP_LINE_RINGING) || (state == VP_LINE_RINGING_POLREV)) {
#ifdef CSLAC_SEQ_EN
        pLineObj->cadence.pActiveCadence = pLineObj->pRingingCadence;
        /*
         * We're entering a ringing state, so determine if we need to
         * cadence. If we're not cadencing, this is "always on", so we can
         * disable the currently active cadence sequence and immediately
         * implement the ringing state change.
         */
        pProfile = pLineObj->cadence.pActiveCadence;
        if (pProfile == VP_PTABLE_NULL) {
            pLineObj->cadence.status = VP_CADENCE_RESET_VALUE;
        } else {
            /*
             * We have a non-null cadence. If the cadence was not previously
             * started, we'll start it here and let the sequencer take over.
             * Otherwise, it was previously started and this state change is
             * at the request of the sequencer.
             */
            if (!(pLineObj->cadence.status & VP_CADENCE_STATUS_ACTIVE)) {
                /* We have a cadence and are just starting it */
                pLineObj->cadence.status |= VP_CADENCE_STATUS_ACTIVE;
                pLineObj->cadence.index = VP_PROFILE_TYPE_SEQUENCER_START;
                pLineObj->cadence.pCurrentPos =
                    &pProfile[VP_PROFILE_TYPE_SEQUENCER_START];
                pLineObj->cadence.length = pProfile[VP_PROFILE_LENGTH];
                pLineObj->cadence.status &=
                    ~VP_CADENCE_STATUS_IGNORE_POLARITY;
                pLineObj->cadence.status |=
                    (pProfile[VP_PROFILE_MPI_LEN] & 0x01) ?
                        VP_CADENCE_STATUS_IGNORE_POLARITY : 0;

                /* Nullify any internal sequence so that the API doesn't think
                 * that an internal sequence of some sort is running */
                pLineObj->intSequence[VP_PROFILE_TYPE_LSB] =
                    VP_PRFWZ_PROFILE_NONE;

                VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880SetStateRinging-"));
                return TRUE;
            }
        }
#endif
        /*
         * Cadencing already called or null cadence.  We're ready to set
         * the line to the Ringing State but we have to first make sure
         * that the signal generator parameters in the device are setup
         * for the ringing profile
         */

        VpMpiCmdWrapper(deviceId, ecVal, VP880_GEN_CTRL_WRT,
            VP880_GEN_CTRL_LEN, &tempRingCtrlData);

        if (pLineObj->ringingParams != VP_PTABLE_NULL) {
            int16 biasErr;
            uint8 bias[VP880_RINGER_PARAMS_LEN];

            VpMemCpy(bias, pLineObj->ringingParams, VP880_RINGER_PARAMS_LEN);
            biasErr = (int16)((((uint16)(pLineObj->ringingParams[1]) << 8) & 0xFF00) +
                ((uint16)(pLineObj->ringingParams[2]) & 0x00FF));
            if (state == VP_LINE_RINGING) {
                /* Normal polarity */
                biasErr -= ((pDevObj->vp880SysCalData.sigGenAError[channelId][0] -
                    pDevObj->vp880SysCalData.vocOffset[channelId][VP880_NORM_POLARITY]) * 16 / 10);
            } else {
                /* Reverse polarity */
                biasErr += ((pDevObj->vp880SysCalData.sigGenAError[channelId][0] -
                    pDevObj->vp880SysCalData.vocOffset[channelId][VP880_REV_POLARITY]) * 16 / 10);
            }
            bias[1] = (uint8)((biasErr >> 8) & 0x00FF);
            bias[2] = (uint8)(biasErr & 0x00FF);

            VpMpiCmdWrapper(deviceId, ecVal, VP880_RINGER_PARAMS_WRT,
                VP880_RINGER_PARAMS_LEN, bias);
        }

        /*
         * Disable the ABS Ringing Exit Timer if previously running. Enable
         * device HPM
         */
        if (pDevObj->stateInt & VP880_IS_ABS) {
            uint8 regCtrl = VP880_SWY_HP | VP880_SWZ_HP;

            pDevObj->devTimer[VP_DEV_TIMER_EXIT_RINGING] = 0;
            VpMpiCmdWrapper(deviceId, ecVal, VP880_REGULATOR_CTRL_WRT,
                VP880_REGULATOR_CTRL_LEN, &regCtrl);
        }

#ifdef CSLAC_SEQ_EN
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
    }

    /*
     * If we are exiting ringing and still on-hook at the time, set the
     * Ringing exit timer
     */
    VP_LINE_STATE(VpLineCtxType, pLineCtx,("Ringing Control for channel %d: CurrentState %d, UserState %d NextState %d",
        channelId, pLineObj->lineState.currentState, pLineObj->lineState.usrCurrent, state));

    switch (pLineObj->lineState.currentState) {
        case VP_LINE_RINGING_POLREV:
        case VP_LINE_RINGING:
            switch(state) {
                case VP_LINE_RINGING_POLREV:
                case VP_LINE_RINGING:
                    break;

                default:
                    if (!(pLineObj->lineState.condition & VP_CSLAC_HOOK)) {
                        pLineObj->lineTimers.timers.timer[VP_LINE_RING_EXIT_DEBOUNCE] =
                            MS_TO_TICKRATE(pLineObj->ringCtrl.ringExitDbncDur / 8,
                                pDevObj->devProfileData.tickRate);

                        if (pLineObj->ringCtrl.ringExitDbncDur) {
                            pLineObj->lineTimers.timers.timer[VP_LINE_RING_EXIT_DEBOUNCE]
                                |= VP_ACTIVATE_TIMER;
                        }
                    }

                    /*
                     * For ABS devices, start the ringing exit timer used to
                     * trigger Switcher Medium Power Enterance. OK if this was
                     * already set. Simply causes it to continue...
                    */
                    if (pDevObj->stateInt & VP880_IS_ABS) {
                        pDevObj->devTimer[VP_DEV_TIMER_EXIT_RINGING] =
                            (MS_TO_TICKRATE(VP_DEV_TIMER_EXIT_RINGING_SAMPLE,
                            pDevObj->devProfileData.tickRate)) | VP_ACTIVATE_TIMER;
                    }

#ifdef CSLAC_SEQ_EN
                    /*
                     * If we're in an active Ringing Cadence, and ready to
                     * go into another state, generate the Ringing Event
                     * and indicate that this is the Ringing Off event
                     */
                    if (pLineObj->cadence.status & VP_CADENCE_STATUS_ACTIVE) {
                        pProfile = pLineObj->cadence.pActiveCadence;
                        if (pProfile[VP_PROFILE_TYPE_LSB] ==
                             VP_PRFWZ_PROFILE_RINGCAD) {
                            pLineObj->lineEvents.process
                                |= VP_LINE_EVID_RING_CAD;
                            pLineObj->processData = VP_RING_CAD_BREAK;
                        }
                    }
#endif
                    break;
            }
            break;

        default:
            break;
    }

    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880SetStateRinging-"));

    return FALSE;
}

/**
 * Vp880SetFxoState()
 *  This function sets the line state for an FXO line.
 *
 * Preconditions:
 *  See VpSetLineState()
 *
 * Postconditions:
 *  Returns success code if the channel can be set and the line state.
 */
VpStatusType
Vp880SetFxoState(
    VpLineCtxType *pLineCtx,    /**< Line context to change line state on */
    VpLineStateType state)      /**< The desired line state to set */
{
    Vp880LineObjectType *pLineObj = pLineCtx->pLineObj;
    uint8 ecVal = pLineObj->ecVal;

    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpDeviceIdType deviceId = pDevObj->deviceId;

    VpLineStateType currentState = pLineObj->lineState.currentState;
    uint8 fxoCidLine;
    uint8 userByte, userByteBefore;

    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880SetFxoState+"));

    /*
     * FXO is straightforward, just set as defined by termination type since
     * we already know it's not an unsupported state (except error maybe)
     */
    VpMpiCmdWrapper(deviceId, ecVal, VP880_IODATA_REG_RD,
        VP880_IODATA_REG_LEN, &userByteBefore);
    userByte = userByteBefore;

    if ((pLineObj->termType == VP_TERM_FXO_GENERIC)
     || (pLineObj->termType == VP_TERM_FXO_DISC)) {
        /* Pre-clear both bits for convenience */
        if (pLineObj->termType == VP_TERM_FXO_DISC) {
            fxoCidLine = VP880_IODATA_IO3;
        } else {
            fxoCidLine = VP880_FXO_CID_LINE;
        }

        userByte &= ~(VP880_IODATA_IO1 | fxoCidLine);

        switch(state) {
            case VP_LINE_FXO_TALK:
                /* IO3/IO1 = 00, so ok */
                break;

            case VP_LINE_FXO_OHT:
                /* IO3/IO1 = 01, so set IO1 to 1 */
                userByte |= VP880_IODATA_IO1;
                break;

            case VP_LINE_FXO_LOOP_CLOSE:
                /* IO3/IO1 = 10, so set IO3 to 1 */
                userByte |= fxoCidLine;
                break;

            case VP_LINE_FXO_LOOP_OPEN:
                /* IO3/IO1 = 11, so set IO3 and IO1 to 1 */
                userByte |= (fxoCidLine | VP880_IODATA_IO1);
                break;

            default:
                /* This should be redundant from TX/RX PCM code section above */
                return VP_STATUS_INVALID_ARG;
        }

        VP_FXO_FUNC(VpLineCtxType, pLineCtx, ("Set FXO State %d Register 0x%02X at time %d",
            state, userByte, pDevObj->timeStamp));

       /* Set the loop open/close bit */
        VpMpiCmdWrapper(deviceId, ecVal, VP880_IODATA_REG_WRT,
            VP880_IODATA_REG_LEN, &userByte);

    } else {
        VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880SetFxoState-"));
        return VP_STATUS_INVALID_ARG;
    }

    if ( ((state == VP_LINE_FXO_TALK) || (state == VP_LINE_FXO_LOOP_CLOSE))
      && ((currentState == VP_LINE_FXO_OHT) || (currentState == VP_LINE_FXO_LOOP_OPEN)) ) {
        VP_FXO_FUNC(VpLineCtxType, pLineCtx, ("Last State Change Timer Set at %d",
            pDevObj->timeStamp));

        pLineObj->lineTimers.timers.fxoTimer.lastStateChange = 0;
    }

    if ( ((state == VP_LINE_FXO_OHT) || (state == VP_LINE_FXO_LOOP_OPEN))
      && ((currentState == VP_LINE_FXO_TALK) || (currentState == VP_LINE_FXO_LOOP_CLOSE)) ) {
        VP_FXO_FUNC(VpLineCtxType, pLineCtx, ("Last State Change Timer Set at %d",
            pDevObj->timeStamp));

        pLineObj->lineTimers.timers.fxoTimer.lastStateChange = 0;
    }

    /* Set the FXO CODEC Mode */
    userByte = (VP880_FXO_ACTIVATE_CODEC | VP880_FXO_SUPERVISION_EN);

    /* Perform the FXO Line State change */
    VpMpiCmdWrapper(deviceId, ecVal, VP880_SYS_STATE_WRT,
        VP880_SYS_STATE_LEN, &userByte);

    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880SetFxoState-"));

    return VP_STATUS_SUCCESS;
}

/**
 * Vp880GroundStartProc()
 *  This function implements the Ground Start procedures when entering or
 * exiting a ground start state. It should be called only by SetLineState
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
Vp880GroundStartProc(
    bool gsMode,
    VpLineCtxType *pLineCtx,
    uint8 currentLineState,
    uint8 userByte)
{
    Vp880LineObjectType *pLineObj = pLineCtx->pLineObj;
    uint8 ecVal = pLineObj->ecVal;

    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpDeviceIdType deviceId = pDevObj->deviceId;

    uint8 sysStateCfg[VP880_SS_CONFIG_LEN];

    uint8 beforeState = (currentLineState & VP880_SS_LINE_FEED_MASK);
    uint8 afterState =  (userByte & VP880_SS_LINE_FEED_MASK);

    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880GroundStartProc+"));

    if (gsMode == TRUE) {
        /*
         * Implement "Return to Ground Start Idle" part of Ground Start
         * workaround for VoicePort Devices
         */
        pLineObj->icr1Values[VP880_ICR1_RING_AND_DAC_LOCATION] |=
            VP880_ICR1_RING_AND_DAC_B2_3;
        pLineObj->icr1Values[VP880_ICR1_RING_AND_DAC_LOCATION+1] |=
            VP880_ICR1_RING_AND_DAC_B2_3;

        if (beforeState == VP880_SS_DISCONNECT) {
            pLineObj->icr1Values[VP880_ICR1_BIAS_OVERRIDE_LOCATION] &=
                ~0x0F;
            pLineObj->icr2Values[VP880_ICR2_SENSE_INDEX] &=
                ~(VP880_ICR2_DAC_SENSE | VP880_ICR2_FEED_SENSE);

            VP_LINE_STATE(VpLineCtxType, pLineCtx, ("Ground Start Enter: Write ICR2 0x%02X 0x%02X 0x%02X 0x%02X Ch %d",
                pLineObj->icr2Values[0], pLineObj->icr2Values[1],
                pLineObj->icr2Values[2], pLineObj->icr2Values[3], pLineObj->channelId));

            VpMpiCmdWrapper(deviceId, ecVal, VP880_ICR2_WRT,
                VP880_ICR2_LEN, pLineObj->icr2Values);
        }

        VP_LINE_STATE(VpLineCtxType, pLineCtx, ("Ground Start Enter: Write ICR1 0x%02X 0x%02X 0x%02X 0x%02X Ch %d",
            pLineObj->icr1Values[0], pLineObj->icr1Values[1],
            pLineObj->icr1Values[2], pLineObj->icr1Values[3], pLineObj->channelId));

        Vp880ProtectedWriteICR1(pLineObj, deviceId, pLineObj->icr1Values);

        if (pDevObj->stateInt & VP880_IS_ABS) {
            /*
             * Disable Auto Battery Switch so Ring stays near -48V for a
             * Ring to Ground
             */
            VpMpiCmdWrapper(deviceId, ecVal, VP880_SS_CONFIG_RD,
                VP880_SS_CONFIG_LEN, sysStateCfg);
            sysStateCfg[0] |= VP880_AUTO_BAT_SWITCH_DIS;
            VpMpiCmdWrapper(deviceId, ecVal, VP880_SS_CONFIG_WRT,
                VP880_SS_CONFIG_LEN, sysStateCfg);
        }

        /* Set Polarity of input to Gkey Detector */
        pLineObj->icr4Values[VP880_ICR4_GKEY_DET_LOCATION] |= VP880_ICR4_GKEY_DET;
        pLineObj->icr4Values[VP880_ICR4_GKEY_DET_LOCATION+1] &= ~VP880_ICR4_GKEY_DET;

        VP_LINE_STATE(VpLineCtxType, pLineCtx, ("Ground Start Enter: Write ICR4 0x%02X 0x%02X 0x%02X 0x%02X Ch %d",
            pLineObj->icr4Values[0], pLineObj->icr4Values[1],
            pLineObj->icr4Values[2], pLineObj->icr4Values[3], pLineObj->channelId));

        VpMpiCmdWrapper(deviceId, ecVal, VP880_ICR4_WRT,
            VP880_ICR4_LEN, pLineObj->icr4Values);

        /*
         * Perform the SLIC state change, controlled by logic that determines
         * if low power mode can be used.
         */
        Vp880LLSetSysState(deviceId, pLineCtx, userByte, TRUE);

        /*
         * Connect Longitudinal Loop Control to ILout Pin Override
         * After this point, the Hook Bit indicates Ground Key
         */
        pLineObj->icr3Values[VP880_ICR3_LONG_LOOP_CTRL_LOCATION] |=
            VP880_ICR3_LONG_LOOP_CONTROL;
        pLineObj->icr3Values[VP880_ICR3_LONG_LOOP_CTRL_LOCATION+1] &=
            ~VP880_ICR3_LONG_LOOP_CONTROL;

        VP_LINE_STATE(VpLineCtxType, pLineCtx, ("Ground Start Enter: Write ICR3 0x%02X 0x%02X 0x%02X 0x%02X Ch %d",
            pLineObj->icr3Values[0], pLineObj->icr3Values[1],
            pLineObj->icr3Values[2], pLineObj->icr3Values[3], pLineObj->channelId));

        VpMpiCmdWrapper(deviceId, ecVal, VP880_ICR3_WRT,
            VP880_ICR3_LEN, pLineObj->icr3Values);

        /*
         * Enable a (30ms) State Change Mask to allow for the line to stabilize
         * before considering valid Ring Ground detection. This is normally
         * set in the DC Profile for Gkey Debounce. However, in LPM the Gkey
         * indication can come from the Hook or Gkey but Hook debounce can be
         * much shorter than Gkey.
         */
        if ((pLineObj->termType == VP_TERM_FXS_LOW_PWR) ||
            (pLineObj->termType == VP_TERM_FXS_ISOLATE_LP) ||
            (pLineObj->termType == VP_TERM_FXS_SPLITTER_LP)) {
            pLineObj->lineTimers.timers.timer[VP_LINE_GND_START_TIMER] =
                (MS_TO_TICKRATE(VP880_LPM_GND_START_TIME,
                    pDevObj->devProfileData.tickRate)) | VP_ACTIVATE_TIMER;
        }
    } else {
        if (((pLineObj->termType == VP_TERM_FXS_LOW_PWR) ||
            (pLineObj->termType == VP_TERM_FXS_ISOLATE_LP) ||
            (pLineObj->termType == VP_TERM_FXS_SPLITTER_LP)) &&
            (pLineObj->status & VP880_LOW_POWER_EN)) {
            VP_LINE_STATE(VpLineCtxType, pLineCtx, ("Adjusting LP Mode on channel %d, NOT writing to device",
                pLineObj->channelId));
            Vp880LLSetSysState(deviceId, pLineCtx, userByte, FALSE);
        } else {
            VP_LINE_STATE(VpLineCtxType, pLineCtx, ("Adjusting LP Mode on channel %d, writing to device",
                pLineObj->channelId));
            Vp880LLSetSysState(deviceId, pLineCtx, userByte, TRUE);
        }

        if ((pLineObj->termType != VP_TERM_FXS_LOW_PWR) &&
            (pLineObj->termType != VP_TERM_FXS_ISOLATE_LP) &&
            (pLineObj->termType != VP_TERM_FXS_SPLITTER_LP)) {
            if (beforeState != afterState) {
                bool writeIcrReg = FALSE;
                if (beforeState == VP880_SS_DISCONNECT) {
                    pLineObj->icr1Values[VP880_ICR1_BIAS_OVERRIDE_LOCATION] &=
                        ~0x0F;

                    pLineObj->icr2Values[VP880_ICR2_SENSE_INDEX] &=
                        ~(VP880_ICR2_DAC_SENSE | VP880_ICR2_FEED_SENSE);
                    writeIcrReg = TRUE;
                } else if (afterState == VP880_SS_DISCONNECT) {
                    pLineObj->icr1Values[VP880_ICR1_BIAS_OVERRIDE_LOCATION] |=
                        0x0F;
                    pLineObj->icr1Values[VP880_ICR1_BIAS_OVERRIDE_LOCATION+1] &= 0xF8;
                    pLineObj->icr1Values[VP880_ICR1_BIAS_OVERRIDE_LOCATION+1] |= 0x08;

                    pLineObj->icr2Values[VP880_ICR2_SENSE_INDEX] |=
                        (VP880_ICR2_DAC_SENSE | VP880_ICR2_FEED_SENSE);
                    pLineObj->icr2Values[VP880_ICR2_SENSE_INDEX+1] &= ~VP880_ICR2_DAC_SENSE;
                    pLineObj->icr2Values[VP880_ICR2_SENSE_INDEX+1] |= VP880_ICR2_FEED_SENSE;
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

                    Vp880ProtectedWriteICR1(pLineObj, deviceId,
                        pLineObj->icr1Values);
                    VpMpiCmdWrapper(deviceId, ecVal, VP880_ICR2_WRT,
                        VP880_ICR2_LEN, pLineObj->icr2Values);
                }
            }
        }

        /*
         * We are transferring from Tip Open to some other state. Need to remove
         * the Ground Start workarounds
         */
        if ((currentLineState & VP880_SS_LINE_FEED_MASK) == VP880_SS_TIP_OPEN) {

            if (pDevObj->stateInt & VP880_IS_ABS) {
                /* Re-enable Auto Battery switch only on VC and later silicon */
                if (pDevObj->staticInfo.rcnPcn[VP880_RCN_LOCATION] >= VP880_REV_VC) {
                    VpMpiCmdWrapper(deviceId, ecVal, VP880_SS_CONFIG_RD,
                        VP880_SS_CONFIG_LEN, sysStateCfg);
                    sysStateCfg[0] &= ~VP880_AUTO_BAT_SWITCH_DIS;
                    VpMpiCmdWrapper(deviceId, ecVal, VP880_SS_CONFIG_WRT,
                        VP880_SS_CONFIG_LEN, sysStateCfg);
                }
            }

            pLineObj->icr1Values[VP880_ICR1_RING_AND_DAC_LOCATION] &=
                ~VP880_ICR1_RING_AND_DAC_B2_3;
            pLineObj->icr1Values[VP880_ICR1_RING_AND_DAC_LOCATION+1] |=
                VP880_ICR1_RING_AND_DAC_B2_3;
            VP_LINE_STATE(VpLineCtxType, pLineCtx, ("Ground Start Exit: Write ICR1 0x%02X 0x%02X 0x%02X 0x%02X Ch %d",
                pLineObj->icr1Values[0], pLineObj->icr1Values[1],
                pLineObj->icr1Values[2], pLineObj->icr1Values[3], pLineObj->channelId));

            Vp880ProtectedWriteICR1(pLineObj, deviceId, pLineObj->icr1Values);

            /*
             * Set a timer for 210ms before disabling the DC bias. This
             * results in retaining the Tip voltage near ground for a time
             * long enough for the CPE to detect the Tip-Gnd voltage and
             * remove/apply the Ring Grnd/Hook
             */
            pLineObj->lineTimers.timers.timer[VP_LINE_GND_START_TIMER] =
                (MS_TO_TICKRATE(VP880_GND_START_TIME,
                    pDevObj->devProfileData.tickRate)) | VP_ACTIVATE_TIMER;
        }
    }

    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880GroundStartProc-"));
}

/**
 * Vp880SetRelGain
 *  This function adjusts the GR and GX values for a given channel of a given
 * device.  It multiplies the profile values by a factor from 0.0 to 4.0.  The
 * adjustment factors are specified in the txLevel and rxLevel parameters,
 * which are 2.14 fixed-point numbers.
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
 */
VpStatusType
Vp880SetRelGain(
    VpLineCtxType *pLineCtx,    /**< Line context to change gains on */
    uint16 txLevel,             /**< Adjustment to line's relative Tx level */
    uint16 rxLevel,             /**< Adjustment to line's relative Rx level */
    uint16 handle)              /**< Handle value returned with the event */
{
    Vp880LineObjectType *pLineObj = pLineCtx->pLineObj;
    Vp880DeviceObjectType *pDevObj = pLineCtx->pDevCtx->pDevObj;
    VpDeviceIdType deviceId = pDevObj->deviceId;
    VpRelGainResultsType *relGainResults = &pDevObj->relGainResults;
    uint8 ecVal = pLineObj->ecVal;
    uint32 gxInt, grInt;
    uint8 gainCSD[VP880_GX_GAIN_LEN];

    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880SetRelGain+"));

    /* Proceed if device state is either in progress or complete */
    if (pDevObj->status.state & (VP_DEV_INIT_CMP | VP_DEV_INIT_IN_PROGRESS)) {
    } else {
        VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880SetRelGain-"));
        return VP_STATUS_DEV_NOT_INITIALIZED;
    }

    /*
     * Do not proceed if the device calibration is in progress. This could
     * damage the device.
     */
    if (pDevObj->status.state & VP_DEV_IN_CAL) {
        VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880SetRelGain-"));
        return VP_STATUS_DEV_NOT_INITIALIZED;
    }

    VpSysEnterCritical(deviceId, VP_CODE_CRITICAL_SEC);

    if (pDevObj->deviceEvents.response & VP880_READ_RESPONSE_MASK) {
        VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
        VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880SetRelGain-"));
        return VP_STATUS_DEVICE_BUSY;
    }

    relGainResults->gResult = VP_GAIN_SUCCESS;

    /* Multiply the profile gain values by the requested adjustments. */
    gxInt = (uint32)pLineObj->gain.gxInt * txLevel / 16384;
    grInt = (uint32)pLineObj->gain.grInt * rxLevel / 16384;

    /* If overflow or underflow occurred, generate out-of-range result. */
    /* Requirement: 1.0 <= gxInt <= 4.0 */
    if ((gxInt < (uint32)0x4000) || (gxInt > (uint32)0x10000)) {
        VP_WARNING(VpLineCtxType, pLineCtx, ("Vp880SetRelGain(): %u * %cxLevel / 16384 = %u, %s is %u",
            (unsigned)pLineObj->gain.gxInt, 't', (unsigned)gxInt,
            (gxInt < (uint32)0x4000) ? "minimum" : "maximum",
            (gxInt < (uint32)0x4000) ? 0x4000U : 0x10000U));

        relGainResults->gResult |= VP_GAIN_GX_OOR;
        gxInt = pLineObj->gain.gxInt;
    }
    /* Requirement: 0.25 <= grInt <= 1.0 */
    if ((grInt < (uint32)0x1000) || (grInt > (uint32)0x4000)) {
        VP_WARNING(VpLineCtxType, pLineCtx, ("Vp880SetRelGain(): %u * %cxLevel / 16384 = %u, %s is %u",
            (unsigned)pLineObj->gain.grInt, 'r', (unsigned)grInt,
            (grInt < (uint32)0x1000) ? "minimum" : "maximum",
            (gxInt < (uint32)0x1000) ? 0x1000U : 0x4000U));

        relGainResults->gResult |= VP_GAIN_GR_OOR;
        grInt = pLineObj->gain.grInt;
    }

    /*
     * Write adjusted gain values to the device, and remember them for
     * VpGetResults().
     */
    VpConvertFixed2Csd((uint16)(gxInt - 0x4000), gainCSD);
    relGainResults->gxValue = ((uint16)gainCSD[0] << 8) + gainCSD[1];
    VpMpiCmdWrapper(deviceId, ecVal, VP880_GX_GAIN_WRT, VP880_GX_GAIN_LEN,
        gainCSD);

    VpConvertFixed2Csd((uint16)grInt, gainCSD);
    relGainResults->grValue = ((uint16)gainCSD[0] << 8) + gainCSD[1];
    VpMpiCmdWrapper(deviceId, ecVal, VP880_GR_GAIN_WRT, VP880_GR_GAIN_LEN,
        gainCSD);

    /* Generate the gain-complete event. */
    pLineObj->lineEvents.response |= VP_LINE_EVID_GAIN_CMP;
    pLineObj->lineEventHandle = handle;

    VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);

    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880SetRelGain-"));

    return VP_STATUS_SUCCESS;
}

/**
 * Vp880MuteChannel()
 *  This function disables or enables the PCM highway for the selected line and
 * should only be called by API internal functions.
 *
 * Preconditions:
 *  The line context must be valid (i.e., pointing to a valid Vp880 line object
 * type).
 *
 * Postconditions:
 *  If mode is TRUE the TX/RX path is cut. If FALSE, the TX/RX path is enabled
 * according to the current line state and mode used for talk states.
 */
void
Vp880MuteChannel(
    VpLineCtxType *pLineCtx,    /**< Line affected */
    bool mode)                  /**< TRUE = Disable TX/RX, FALSE = enable */
{
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;

    Vp880LineObjectType *pLineObj = pLineCtx->pLineObj;

    uint8 ecVal = pLineObj->ecVal;
    VpDeviceIdType deviceId = pDevObj->deviceId;
    uint8 preState, postState;
    uint8 mpiByte = 0;

    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880MuteChannel+"));

    /*
     * Read the status of the Operating Conditions register so we can change
     * only the TX and RX if the line state is a non-communication mode.
     */
    VpMpiCmdWrapper(deviceId, ecVal, VP880_OP_COND_RD, VP880_OP_COND_LEN,
        &preState);
    postState = preState;
    postState &= ~(VP880_CUT_TXPATH | VP880_CUT_RXPATH);
    postState &= ~(VP880_HIGH_PASS_DIS | VP880_OPCOND_RSVD_MASK);

    /*
     * If disabling, simple. Otherwise enable based on the current line state
     * and the state of the "talk" option. The "talk" option is maintained in
     * the line object and abstracted in Vp880GetTxRxMode() function
     */

    Vp880GetTxRxPcmMode(pLineObj, pLineObj->lineState.currentState, &mpiByte);

    if (mode == TRUE) {
        /*
         * If awaiting DTMF detection, enable TX, disable RX. This is higher
         * priority than Mute mode. Otherwise, disable both TX and RX.
         */
        postState |= VP880_CUT_RXPATH;  /* Mute == TRUE always cuts RX path */
#ifdef CSLAC_SEQ_EN
        if (!(pLineObj->callerId.status & VP_CID_AWAIT_TONE)) {
#endif
            /* Not awaiting tone, TX Path is disabled as well */
            postState |= VP880_CUT_TXPATH;
#ifdef CSLAC_SEQ_EN
        }
#endif
    } else {
        /*
         * It's possible that a Mute off is occuring because of end of DTMF
         * detection, or end of data generation, or end of Mute period. However,
         * we only need to check if Mute On is still enabled since DTMF
         * detection will not occur while data is being generated.
         */
#ifdef CSLAC_SEQ_EN
        if (pLineObj->callerId.status & VP_CID_MUTE_ON) {
            /*
             * Some "other" operation completed, but we're still in a Mute On
             * period.
             */
            postState |= (VP880_CUT_RXPATH | VP880_CUT_TXPATH);
        } else  {
#endif
            postState |= mpiByte;
#ifdef CSLAC_SEQ_EN
        }
#endif
    }

    if (postState != preState) {
        VP_INFO(VpLineCtxType, pLineCtx, ("3. Writing 0x%02X to Operating Conditions",
            postState));

        VpMpiCmdWrapper(deviceId, ecVal, VP880_OP_COND_WRT,
            VP880_OP_COND_LEN, &postState);
    }
    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880MuteChannel-"));

    return;
}

/**
 * Vp880GetTxRxPcmMode()
 *  This function returns the TX/RX PCM bits for the PCM (enable/disable) mode
 * corresponding to the state passed. The results should be or'-ed with the
 * bits set to 0 prior to calling this function.
 *
 * Preconditions:
 *  None. Mapping function only.
 *
 * Postconditions:
 *  None. Mapping function only.
 */
VpStatusType
Vp880GetTxRxPcmMode(
    Vp880LineObjectType *pLineObj,
    VpLineStateType state,  /**< The state associating with PCM mode */
    uint8 *mpiByte) /**< Device Specific byte */
{
    VP_API_FUNC_INT(None, VP_NULL, ("Vp880GetTxRxPcmMode+"));

    switch(pLineObj->pcmTxRxCtrl) {
        case VP_OPTION_PCM_BOTH:
            *mpiByte = 0x00;
            break;

        case VP_OPTION_PCM_RX_ONLY:
            *mpiByte = VP880_CUT_TXPATH;
            break;

        case VP_OPTION_PCM_TX_ONLY:
            *mpiByte = VP880_CUT_RXPATH;
            break;

        case VP_OPTION_PCM_ALWAYS_ON:
            *mpiByte = 0x00;
            return VP_STATUS_SUCCESS;

        default:
            *mpiByte = 0x00;
            break;
    }

    switch(state) {
        /* Non-Talk States */
        case VP_LINE_STANDBY:
        case VP_LINE_STANDBY_POLREV:
        case VP_LINE_TIP_OPEN:
        case VP_LINE_ACTIVE:
        case VP_LINE_ACTIVE_POLREV:
        case VP_LINE_DISCONNECT:
        case VP_LINE_RINGING:
        case VP_LINE_RINGING_POLREV:
            if (pLineObj->status & VP880_IS_FXO) {
                VP_API_FUNC_INT(None, VP_NULL, ("Vp880GetTxRxPcmMode-"));
                return VP_STATUS_INVALID_ARG;
            }
            *mpiByte |= (VP880_CUT_TXPATH | VP880_CUT_RXPATH);
            break;

        case VP_LINE_FXO_LOOP_OPEN:
        case VP_LINE_FXO_LOOP_CLOSE:
        case VP_LINE_FXO_RING_GND:
            if (!(pLineObj->status & VP880_IS_FXO)) {
                VP_API_FUNC_INT(None, VP_NULL, ("Vp880GetTxRxPcmMode-"));
                return VP_STATUS_INVALID_ARG;
            }
            *mpiByte |= (VP880_CUT_TXPATH | VP880_CUT_RXPATH);
            break;

        /* Talk States */
        case VP_LINE_TALK:
        case VP_LINE_TALK_POLREV:
        case VP_LINE_OHT:
        case VP_LINE_OHT_POLREV:
            if (pLineObj->status & VP880_IS_FXO) {
                VP_API_FUNC_INT(None, VP_NULL, ("Vp880GetTxRxPcmMode-"));
                return VP_STATUS_INVALID_ARG;
            }
            break;

        case VP_LINE_FXO_OHT:
        case VP_LINE_FXO_TALK:
            if (!(pLineObj->status & VP880_IS_FXO)) {
                VP_API_FUNC_INT(None, VP_NULL, ("Vp880GetTxRxPcmMode-"));
                return VP_STATUS_INVALID_ARG;
            }
            break;

        default:
            break;
    }
    VP_API_FUNC_INT(None, VP_NULL, ("Vp880GetTxRxPcmMode-"));

    return VP_STATUS_SUCCESS;
}

/**
 * Vp880GetLineStateABS
 *  Locally used function by Vp880SetLineState to get the line state byte used
 * for ABS devices.
 *
 * Preconditions:
 *  None. State to byte mapping only.
 *
 * Postconditions:
 *  Returns the byte that should be used in the device System State register
 * for the API State passed.
 */
uint8
Vp880GetLineStateABS(
    VpLineCtxType *pLineCtx,
    VpLineStateType state)
{
    Vp880LineObjectType *pLineObj = pLineCtx->pLineObj;
    uint8 ecVal = pLineObj->ecVal;
    uint8 channelId = pLineObj->channelId;

    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpDeviceIdType deviceId = pDevObj->deviceId;

    uint8 returnVal = 0xFF;
    uint8 dcCalVal[VP880_DC_CAL_REG_LEN];

    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880GetLineStateABS+"));

    if (pDevObj->staticInfo.rcnPcn[VP880_RCN_LOCATION] >= VP880_REV_VC) {
        VpMpiCmdWrapper(deviceId, ecVal, VP880_DC_CAL_REG_RD,
            VP880_DC_CAL_REG_LEN, dcCalVal);
    }

    switch(state) {
        case VP_LINE_STANDBY:
            returnVal = VP880_SS_IDLE;
            dcCalVal[0] &= 0x0F;
            dcCalVal[0] |= pDevObj->vp880SysCalData.absNormCal[channelId];
            break;

        case VP_LINE_TIP_OPEN:
            returnVal = VP880_SS_TIP_OPEN;
            break;

        case VP_LINE_ACTIVE:
        case VP_LINE_TALK:
            returnVal = VP880_SS_ACTIVE;
            dcCalVal[0] &= 0x0F;
            dcCalVal[0] |= pDevObj->vp880SysCalData.absNormCal[channelId];
            break;

        case VP_LINE_ACTIVE_POLREV:
        case VP_LINE_TALK_POLREV:
        case VP_LINE_STANDBY_POLREV:    /* Idle Polrev does not work */
            returnVal = VP880_SS_ACTIVE_POLREV;
            dcCalVal[0] &= 0x0F;
            dcCalVal[0] |= pDevObj->vp880SysCalData.absPolRevCal[channelId];
            break;

        case VP_LINE_OHT:
            returnVal = VP880_SS_ACTIVE_MID_BAT;
            dcCalVal[0] &= 0x0F;
            dcCalVal[0] |= pDevObj->vp880SysCalData.absNormCal[channelId];
            break;

        case VP_LINE_OHT_POLREV:
            returnVal = VP880_SS_ACTIVE_MID_BAT_PR;
            dcCalVal[0] &= 0x0F;
            dcCalVal[0] |= pDevObj->vp880SysCalData.absPolRevCal[channelId];
            break;

        case VP_LINE_DISCONNECT:
            returnVal = VP880_SS_DISCONNECT;
            break;

        case VP_LINE_RINGING:
            if (pLineObj->status & VP880_UNBAL_RINGING) {
                returnVal = VP880_SS_UNBALANCED_RINGING;
            } else {
                returnVal = VP880_SS_BALANCED_RINGING;
            }
            dcCalVal[0] &= 0x0F;
            dcCalVal[0] |= pDevObj->vp880SysCalData.absNormCal[channelId];
            break;

        case VP_LINE_RINGING_POLREV:
            if (pLineObj->status & VP880_UNBAL_RINGING) {
                returnVal = VP880_SS_UNBALANCED_RINGING_PR;
            } else {
                returnVal = VP880_SS_BALANCED_RINGING_PR;
            }
            dcCalVal[0] &= 0x0F;
            dcCalVal[0] |= pDevObj->vp880SysCalData.absPolRevCal[channelId];
            break;

        default:
            break;
    }

    if (pDevObj->staticInfo.rcnPcn[VP880_RCN_LOCATION] >= VP880_REV_VC) {
        VpMpiCmdWrapper(deviceId, ecVal, VP880_DC_CAL_REG_WRT,
            VP880_DC_CAL_REG_LEN, dcCalVal);
        VpMemCpy(pLineObj->icr6Values, dcCalVal, VP880_DC_CAL_REG_LEN);
    }

    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880GetLineStateABS-"));

    return returnVal;
}

/**
 * Vp880GetLineStateNonABS
 *  Locally used function by Vp880SetLineState to get the line state byte used
 * for non-ABS devices.
 *
 * Preconditions:
 *  None. State to byte mapping only.
 *
 * Postconditions:
 *  Returns the byte that should be used in the device System State register
 * for the API State passed.
 */
uint8
Vp880GetLineStateNonABS(
    VpLineCtxType *pLineCtx,
    VpLineStateType state)
{
    Vp880LineObjectType *pLineObj = pLineCtx->pLineObj;

    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880GetLineStateNonABS+"));

    switch(state) {
        case VP_LINE_STANDBY:
            VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880GetLineStateNonABS-"));
            return VP880_SS_IDLE;

        case VP_LINE_TIP_OPEN:
            VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880GetLineStateNonABS-"));
            return VP880_SS_TIP_OPEN;

        case VP_LINE_ACTIVE:
        case VP_LINE_TALK:
        case VP_LINE_OHT:
            VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880GetLineStateNonABS-"));
            return VP880_SS_ACTIVE;

        case VP_LINE_ACTIVE_POLREV:
        case VP_LINE_TALK_POLREV:
        case VP_LINE_OHT_POLREV:
        case VP_LINE_STANDBY_POLREV:    /* Idle Polrev does not work */
            VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880GetLineStateNonABS-"));
            return VP880_SS_ACTIVE_POLREV;

        case VP_LINE_DISCONNECT:
            VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880GetLineStateNonABS-"));
            return VP880_SS_DISCONNECT;

        case VP_LINE_RINGING:
            if (pLineObj->status & VP880_UNBAL_RINGING) {
                VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880GetLineStateNonABS-"));
                return VP880_SS_UNBALANCED_RINGING;
            } else {
                VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880GetLineStateNonABS-"));
                return VP880_SS_BALANCED_RINGING;
            }

        case VP_LINE_RINGING_POLREV:
            if (pLineObj->status & VP880_UNBAL_RINGING) {
                VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880GetLineStateNonABS-"));
                return VP880_SS_UNBALANCED_RINGING_PR;
            } else {
                VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880GetLineStateNonABS-"));
                return VP880_SS_BALANCED_RINGING_PR;
            }

        default:
            break;
    }
    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880GetLineStateNonABS-"));

    return 0xFF;
}

/**
 * Vp880SetLineTone()
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
 */
VpStatusType
Vp880SetLineTone(
    VpLineCtxType *pLineCtx,
    VpProfilePtrType pToneProfile,  /**< A pointer to a tone profile, or an
                                     * index into the profile table for the tone
                                     * to put on the line.
                                     */
    VpProfilePtrType pCadProfile,   /**< A pointer to a tone cadence profile, or
                                     * an index into the profile table for the
                                     * tone cadence to put on the line.
                                     */
    VpDtmfToneGenType *pDtmfControl)    /**< Indicates to send a DTMF tone
                                         * (either upstream or downstream) if
                                         * this parameter is not VP_NULL AND
                                         * the tone specified is VP_PTABLE_NULL
                                         */
{
    Vp880LineObjectType *pLineObj = pLineCtx->pLineObj;
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpProfilePtrType pToneProf = VP_PTABLE_NULL;
    VpProfilePtrType pCadProf = VP_PTABLE_NULL;

    VpDigitType digit = VP_DIG_NONE;
    VpDirectionType direction = VP_DIRECTION_INVALID;

    uint8 ecVal = pLineObj->ecVal;

    uint8 opCond, sigGenCtrl, sigGenABCount;
    uint8 mpiByte = 0;

    /* Initialize SigGen A/B values to 0 */
    uint8 sigGenAB[VP880_SIGAB_PARAMS_LEN] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    uint8 sigGenABOffset = 3;  /* Starting point after filler bytes */

    int toneIndex = VpGetProfileIndex(pToneProfile);
    int cadenceIndex = VpGetProfileIndex(pCadProfile);
    uint16 profIndex;   /* Used for bit masking the profile index table */

    VpDeviceIdType deviceId = pDevObj->deviceId;

    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880SetLineTone+"));

    /* Proceed if device state is either in progress or complete */
    if (pDevObj->status.state & (VP_DEV_INIT_CMP | VP_DEV_INIT_IN_PROGRESS)) {
    } else {
        VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880SetLineTone-"));
        return VP_STATUS_DEV_NOT_INITIALIZED;
    }

    /*
     * Do not proceed if the device calibration is in progress. This could
     * damage the device.
     */
    if (pDevObj->status.state & VP_DEV_IN_CAL) {
        VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880SetLineTone-"));
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
                VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880SetLineTone-"));
                return VP_STATUS_ERR_PROFILE;
            }
        }
        pToneProf = pToneProfile;
    } else if (toneIndex < VP_CSLAC_TONE_PROF_TABLE_SIZE) {
        profIndex = (uint16)toneIndex;
        pToneProf = pDevObj->devProfileTable.pToneProfileTable[profIndex];
        if (!(pDevObj->profEntry.toneProfEntry & (0x0001 << profIndex))) {
            /* The profile is invalid -- error. */
            VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880SetLineTone-"));
            return VP_STATUS_ERR_PROFILE;
        }
    } else {
        VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880SetLineTone-"));
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
                VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880SetLineTone-"));
                return VP_STATUS_ERR_PROFILE;
            }
        }
        pCadProf = pCadProfile;
    } else if (cadenceIndex < VP_CSLAC_TONE_CADENCE_PROF_TABLE_SIZE) {
        pCadProf = pDevObj->devProfileTable.pToneCadProfileTable[cadenceIndex];
        if (!(pDevObj->profEntry.toneCadProfEntry & (0x0001 << cadenceIndex))) {
            /* The profile is invalid -- error. */
            VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880SetLineTone-"));
            return VP_STATUS_ERR_PROFILE;
        }
    } else {
        VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880SetLineTone-"));
        return VP_STATUS_ERR_PROFILE;
    }

    if (pDtmfControl != VP_NULL) {
        digit = pDtmfControl->toneId;
        if (VpIsDigit(digit) == FALSE) {
            VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880SetLineTone-"));
            return VP_STATUS_INVALID_ARG;
        }

        direction = pDtmfControl->dir;
        if (direction != VP_DIRECTION_DS) {
            VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880SetLineTone-"));
            return VP_STATUS_INVALID_ARG;
        }
    }

    /* All input parameters are valid. */
    VpSysEnterCritical(deviceId, VP_CODE_CRITICAL_SEC);

    if (pLineObj->status & VP880_BAD_LOOP_SUP) {
        pLineObj->status &= ~(VP880_BAD_LOOP_SUP);
        VpMpiCmdWrapper(deviceId, ecVal, VP880_LOOP_SUP_WRT,
            VP880_LOOP_SUP_LEN, pLineObj->loopSup);
    }

    /*
     * Disable signal generator A/B/C/D before making any changes and stop
     * previous cadences
     */
    VpMpiCmdWrapper(deviceId, ecVal, VP880_GEN_CTRL_RD, VP880_GEN_CTRL_LEN,
        &sigGenCtrl);
    sigGenCtrl &= ~(VP880_GEN_ALLON | VP880_GEN_CTRL_EN_BIAS);
    VpMpiCmdWrapper(deviceId, ecVal, VP880_GEN_CTRL_WRT, VP880_GEN_CTRL_LEN,
        &sigGenCtrl);

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
    VpMpiCmdWrapper(deviceId, ecVal, VP880_OP_COND_RD, VP880_OP_COND_LEN,
        &opCond);
    opCond &= ~(VP880_HIGH_PASS_DIS | VP880_OPCOND_RSVD_MASK);

    /*
     * If tone profile is NULL, and either the pDtmfControl is NULL or it's
     * "digit" member is "Digit None", then shutoff the tone generators, stop
     * any active cadencing and restore the filter coefficients if they need
     * to be. Also, re-enable the audio path if it was disabled by a previous
     * DTMF generation command
     */
    if (((pToneProf == VP_PTABLE_NULL) && (pDtmfControl == VP_NULL))
     || ((pToneProf == VP_PTABLE_NULL) && (digit == VP_DIG_NONE))) {
        /*
         * Update the TX/RX Path enable/disable ONLY if not running CID. The CID
         * sequence itself manages TX/RX path control
         */
#ifdef CSLAC_SEQ_EN
        if (!(pLineObj->callerId.status & VP_CID_IN_PROGRESS)) {
#endif
            /*
             * Pre-Or the bits and get the correct values based on the current
             * line state, then update the device.
             */
            opCond &= ~(VP880_CUT_TXPATH | VP880_CUT_RXPATH);
            Vp880GetTxRxPcmMode(pLineObj, pLineObj->lineState.currentState, &mpiByte);
            opCond |= mpiByte;

            VP_INFO(VpLineCtxType, pLineCtx, ("4. Writing 0x%02X to Operating Conditions",
                opCond));
            VpMpiCmdWrapper(deviceId, ecVal, VP880_OP_COND_WRT,
                VP880_OP_COND_LEN, &opCond);
#ifdef CSLAC_SEQ_EN
        }
#endif

        VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
        VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880SetLineTone-"));
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
        Vp880SetDTMFGenerators(pLineCtx, VP_CID_NO_CHANGE, digit);

        /*
         * Disable only the receive path since disabling the transmit path
         * also may generate noise upstream (e.g., an unterminated, but
         * assigned timeslot
         */
        opCond |= VP880_CUT_RXPATH;

        VP_INFO(VpLineCtxType, pLineCtx, ("5. Writing 0x%02X to Operating Conditions",
            opCond));
        VpMpiCmdWrapper(deviceId, ecVal, VP880_OP_COND_WRT,
            VP880_OP_COND_LEN, &opCond);

        /* Enable only generator A/B */
        VpMpiCmdWrapper(deviceId, ecVal, VP880_GEN_CTRL_RD,
            VP880_GEN_CTRL_LEN, &sigGenCtrl);
        sigGenCtrl |= (VP880_GENB_EN | VP880_GENA_EN);
        sigGenCtrl &= ~VP880_GEN_CTRL_EN_BIAS;
        VpMpiCmdWrapper(deviceId, ecVal, VP880_GEN_CTRL_WRT,
            VP880_GEN_CTRL_LEN, &sigGenCtrl);

        VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
        VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880SetLineTone-"));
        return VP_STATUS_SUCCESS;
    }

#ifdef CSLAC_SEQ_EN
    /* If we're here, we're sending a Tone, not DTMF */
    if ((pCadProf != VP_PTABLE_NULL)
     && ((pCadProf[VP880_TONE_TYPE] == VP880_HOWLER_TONE)
      || (pCadProf[VP880_TONE_TYPE] == VP880_AUS_HOWLER_TONE)
      || (pCadProf[VP880_TONE_TYPE] == VP880_NTT_HOWLER_TONE))) {

        uint8 sigGenCD[VP880_SIGCD_PARAMS_LEN] = {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };

        if (pCadProf[VP880_TONE_TYPE] == VP880_HOWLER_TONE) {

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
        } else if (pCadProf[VP880_TONE_TYPE] == VP880_AUS_HOWLER_TONE) {
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
        VpMpiCmdWrapper(deviceId, ecVal, VP880_SIGCD_PARAMS_WRT,
            VP880_SIGCD_PARAMS_LEN, sigGenCD);

        /* Program A/B */
        VpMpiCmdWrapper(deviceId, ecVal, VP880_SIGAB_PARAMS_WRT,
            VP880_SIGAB_PARAMS_LEN, sigGenAB);

        /* Set the parameters in the line object for cadence use */
        for (sigGenABCount = 0; sigGenABCount < VP880_SIGAB_PARAMS_LEN;
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
             sigGenABCount < VP880_SIGAB_PARAMS_LEN;
             sigGenABCount++, profIndex++) {
            sigGenAB[sigGenABCount] =
                pToneProf[VP880_SIGGEN_AB_START + profIndex];
        }

        VpMpiCmdWrapper(deviceId, ecVal, VP880_SIGAB_PARAMS_WRT,
            VP880_SIGAB_PARAMS_LEN, sigGenAB);
        VpMpiCmdWrapper(deviceId, ecVal, VP880_SIGCD_PARAMS_WRT,
            VP880_SIGCD_PARAMS_LEN,
            (VpProfileDataType *)(&pToneProf[VP880_SIGGEN_CD_START]));

        VpMpiCmdWrapper(deviceId, ecVal, VP880_GEN_CTRL_RD,
            VP880_GEN_CTRL_LEN, &sigGenCtrl);
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
        sigGenCtrl |= VP880_GEN_ALLON;
        sigGenCtrl &= ~VP880_GEN_CTRL_EN_BIAS;
        VpMpiCmdWrapper(deviceId, ecVal, VP880_GEN_CTRL_WRT,
            VP880_GEN_CTRL_LEN, &sigGenCtrl);
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

        /* Nullify any internal sequence so that the API doesn't think
         * that an internal sequence of some sort is running */
        pLineObj->intSequence[VP_PROFILE_TYPE_LSB] = VP_PRFWZ_PROFILE_NONE;
    }
#endif

    VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880SetLineTone-"));
    return VP_STATUS_SUCCESS;
}

/**
 * Vp880SetRelayState()
 *  This function controls the state of controlled relays for the VP880 device.
 *
 * Preconditions:
 *  Device/Line context should be created and initialized. For applicable
 * devices bootload should be performed before calling the function.
 *
 * Postconditions:
 *  The indicated relay state is set for the given line.
 */
VpStatusType
Vp880SetRelayState(
    VpLineCtxType *pLineCtx,
    VpRelayControlType rState)
{
    Vp880LineObjectType *pLineObj = pLineCtx->pLineObj;
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpDeviceIdType deviceId = pDevObj->deviceId;

    uint8 ecVal = pLineObj->ecVal;

    uint8 ioDirection[VP880_IODIR_REG_LEN];
    uint8 ioData[VP880_IODATA_REG_LEN];

    /*
     * In case this function fails, make sure the line object is unchanged by
     * pre-saving it's current value. First step prior to 100% verifiying if
     * this function will be success is to pre-clear the relay control bits.
     */
    uint8 preIcr1Values = pLineObj->icr1Values[VP880_ICR1_TEST_LOAD_LOCATION];

    VpSysEnterCritical(deviceId, VP_CODE_CRITICAL_SEC);

    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880SetRelayState+"));

    /* Proceed if device state is either in progress or complete */
    if (pDevObj->status.state & (VP_DEV_INIT_CMP | VP_DEV_INIT_IN_PROGRESS)) {
    } else {
        VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
        VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880SetRelayState-"));
        return VP_STATUS_DEV_NOT_INITIALIZED;
    }

    /*
     * Do not proceed if the device calibration is in progress. This could
     * damage the device.
     */
    if (pDevObj->status.state & VP_DEV_IN_CAL) {
        VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
        VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880SetRelayState-"));
        return VP_STATUS_DEV_NOT_INITIALIZED;
    }

    /* Handle the VP_RELAY_BRIDGED_TEST case for devices that do not have a
     * physical test load.  Use the internal test termination algorithm instead.
     * If the VP880_ALWAYS_USE_INTERNAL_TEST_TERMINATION option is defined in
     * vp_api_cfg.h the HAS_TEST_LOAD_SWITCH flag will always be cleared at
     * initialization so that this method will always be used.  The internal
     * test termination is only supported for revisions newer than VC. */
    if (rState == VP_RELAY_BRIDGED_TEST &&
        !(pDevObj->stateInt & VP880_HAS_TEST_LOAD_SWITCH) &&
        pDevObj->staticInfo.rcnPcn[VP880_RCN_LOCATION] > VP880_REV_VC)
    {
        Vp880ApplyInternalTestTerm(pLineCtx);

        pLineObj->relayState = VP_RELAY_BRIDGED_TEST;

        VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
        return VP_STATUS_SUCCESS;
    }

    /* If the internal test termination is currently applied and we're going
     * to a state other than BRIDGED_TEST, restore the internal settings */
    if (pLineObj->internalTestTermApplied == TRUE &&
        rState != VP_RELAY_BRIDGED_TEST)
    {
        Vp880RemoveInternalTestTerm(pLineCtx);
    }

    /* Read registers that may have partial modifications */
    VpMpiCmdWrapper(deviceId, ecVal, VP880_IODIR_REG_RD,
        VP880_IODIR_REG_LEN, ioDirection);

    VpMpiCmdWrapper(deviceId, ecVal, VP880_IODATA_REG_RD,
        VP880_IODATA_REG_LEN, ioData);

    /* Always set the value for Test Load Control. */
    pLineObj->icr1Values[VP880_ICR1_TEST_LOAD_LOCATION-1] |=
        VP880_ICR1_TEST_LOAD_MASK;

    /* Preclear test load bits */
    pLineObj->icr1Values[VP880_ICR1_TEST_LOAD_LOCATION]
        &= ~(VP880_ICR1_TEST_LOAD_MASK);

    switch (pLineObj->termType) {
        case VP_TERM_FXS_GENERIC:
        case VP_TERM_FXS_LOW_PWR:
            switch (rState) {
                case VP_RELAY_BRIDGED_TEST:
                    if (!(pDevObj->stateInt & VP880_HAS_TEST_LOAD_SWITCH)) {

                        /* Restore values prior to caling this function. */
                        pLineObj->icr1Values[VP880_ICR1_TEST_LOAD_LOCATION] = preIcr1Values;

                        VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
                        VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880SetRelayState-"));
                        return VP_STATUS_INVALID_ARG;
                    }

                    pLineObj->icr1Values[VP880_ICR1_TEST_LOAD_LOCATION]
                        |= VP880_ICR1_TEST_LOAD_METALLIC;

                    /* Test Load bits pre-cleared. Can pick up at Normal */

                case VP_RELAY_NORMAL:
                    break;

                default:
                    /* Restore values prior to caling this function. */
                    pLineObj->icr1Values[VP880_ICR1_TEST_LOAD_LOCATION] = preIcr1Values;

                    VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
                    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880SetRelayState-"));
                    return VP_STATUS_INVALID_ARG;
            }
            break;

        case VP_TERM_FXS_ISOLATE:
        case VP_TERM_FXS_ISOLATE_LP:
        case VP_TERM_FXS_SPLITTER:
        case VP_TERM_FXS_SPLITTER_LP:
            ioDirection[0] &= ~VP880_IODIR_IO1_MASK;
            ioData[0] &= ~(VP880_IODATA_IO1);

            switch (rState) {
                case VP_RELAY_BRIDGED_TEST:
                    if (!(pDevObj->stateInt & VP880_HAS_TEST_LOAD_SWITCH)) {
                        /* Restore values prior to caling this function. */
                        pLineObj->icr1Values[VP880_ICR1_TEST_LOAD_LOCATION] = preIcr1Values;

                        VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
                        VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880SetRelayState-"));
                        return VP_STATUS_INVALID_ARG;
                    }

                    pLineObj->icr1Values[VP880_ICR1_TEST_LOAD_LOCATION]
                        |= VP880_ICR1_TEST_LOAD_METALLIC;

                    /* Test Load bits pre-cleared. Can pick up at Normal */

                case VP_RELAY_NORMAL:
                    if (pDevObj->staticInfo.rcnPcn[VP880_RCN_LOCATION] <= VP880_REV_VC) {
                        ioDirection[0] |= VP880_IODIR_IO1_OPEN_DRAIN;
                        if ((pLineObj->termType == VP_TERM_FXS_ISOLATE) ||
                            (pLineObj->termType == VP_TERM_FXS_ISOLATE_LP)) {
                            /* Dir = open drain, Set Data = 0 */
                            ioData[0] &= ~VP880_IODATA_IO1;
                        } else {
                            /* Dir = open drain, Set Data = 1 */
                            ioData[0] |= VP880_IODATA_IO1;
                        }
                    } else {
                        ioData[0] |= VP880_IODATA_IO1;
                        if ((pLineObj->termType == VP_TERM_FXS_ISOLATE) ||
                            (pLineObj->termType == VP_TERM_FXS_ISOLATE_LP)) {
                            /* Dir = output, Set Data = 1 */
                            ioDirection[0] |= VP880_IODIR_IO1_OUTPUT;
                        } else {
                            /* Dir = open drain, Set Data = 1 */
                            ioDirection[0] |= VP880_IODIR_IO1_OPEN_DRAIN;
                        }
                    }
                    break;

                case VP_RELAY_RESET:
                    if (pDevObj->staticInfo.rcnPcn[VP880_RCN_LOCATION] <= VP880_REV_VC) {
                        ioDirection[0] |= VP880_IODIR_IO1_OPEN_DRAIN;
                        if ((pLineObj->termType == VP_TERM_FXS_ISOLATE) ||
                            (pLineObj->termType == VP_TERM_FXS_ISOLATE_LP)) {
                            /* Dir = open drain, Set Data = 1 */
                            ioData[0] |= VP880_IODATA_IO1;
                        } else {
                            /* Dir = open drain, Set Data = 0 */
                            ioData[0] &= ~VP880_IODATA_IO1;
                        }
                    } else {
                        ioData[0] |= VP880_IODATA_IO1;
                        if ((pLineObj->termType == VP_TERM_FXS_ISOLATE) ||
                            (pLineObj->termType == VP_TERM_FXS_ISOLATE_LP)) {
                            /* Dir = open drain, Set Data = 1 */
                            ioDirection[0] |= VP880_IODIR_IO1_OPEN_DRAIN;
                        } else {
                            /* Dir = output, Set Data = 1 */
                            ioDirection[0] |= VP880_IODIR_IO1_OUTPUT;
                        }
                    }
                    break;

                default:
                    /* Restore values prior to caling this function. */
                    pLineObj->icr1Values[VP880_ICR1_TEST_LOAD_LOCATION] = preIcr1Values;

                    VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
                    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880SetRelayState-"));
                    return VP_STATUS_INVALID_ARG;
            }
            break;

        default:
            /* Restore values prior to caling this function. */
            pLineObj->icr1Values[VP880_ICR1_TEST_LOAD_LOCATION] = preIcr1Values;

            VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
            VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880SetRelayState-"));
            return VP_STATUS_INVALID_ARG;
    }

    VP_LINE_STATE(VpLineCtxType, pLineCtx, ("1. Write IODATA 0x%02X on Ch %d", ioData[0], pLineObj->channelId));

    VpMpiCmdWrapper(deviceId, ecVal, VP880_IODATA_REG_WRT,
        VP880_IODATA_REG_LEN, ioData);

    VP_LINE_STATE(VpLineCtxType, pLineCtx, ("2. Write IODIR 0x%02X on Channel %d",
        ioDirection[0], pLineObj->channelId));
    VpMpiCmdWrapper(deviceId, ecVal, VP880_IODIR_REG_WRT,
        VP880_IODIR_REG_LEN, ioDirection);

    VP_LINE_STATE(VpLineCtxType, pLineCtx, ("3. Write ICR1 0x%02X 0x%02X 0x%02X 0x%02X Ch %d",
        pLineObj->icr1Values[0], pLineObj->icr1Values[1],
        pLineObj->icr1Values[2], pLineObj->icr1Values[3], pLineObj->channelId));

    Vp880ProtectedWriteICR1(pLineObj, deviceId, pLineObj->icr1Values);

    pLineObj->relayState = rState;

    VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);

    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880SetRelayState-"));
    return VP_STATUS_SUCCESS;
}

/**
 * Vp880ApplyInternalTestTerm()
 *  Configures ICR settings for the internal test termination algorithm, which
 * is used instead of a physical test load for devices which do not have one.
 * The internal test termination works by internally shorting tip and ring.
 */
static void
Vp880ApplyInternalTestTerm(
    VpLineCtxType *pLineCtx)
{
    Vp880LineObjectType *pLineObj = pLineCtx->pLineObj;
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpDeviceIdType deviceId = pDevObj->deviceId;
    uint8 ecVal = pLineObj->ecVal;
    uint16 timerDelay;

    uint8 icr1Reg[VP880_ICR1_LEN];

    if (pLineObj->internalTestTermApplied == TRUE) {
        return;
    }

    VpSysEnterCritical(deviceId, VP_CODE_CRITICAL_SEC);

    /* Disconnect VAB sensing */
    pLineObj->icr6Values[VP880_DC_CAL_CUT_INDEX] |= VP880_C_TIP_SNS_CUT;
    pLineObj->icr6Values[VP880_DC_CAL_CUT_INDEX] |= VP880_C_RING_SNS_CUT;
    VpMpiCmdWrapper(deviceId, ecVal, VP880_ICR6_WRT, VP880_ICR6_LEN,
        pLineObj->icr6Values);

    /* Reverse the polarity of the ground key detector to disable ground
     * key event */
    pLineObj->icr4Values[VP880_ICR4_GKEY_DET_LOCATION] |=
        VP880_ICR4_GKEY_POL;
    pLineObj->icr4Values[VP880_ICR4_GKEY_DET_LOCATION+1] |=
        VP880_ICR4_GKEY_POL;
    VpMpiCmdWrapper(deviceId, ecVal, VP880_ICR4_WRT, VP880_ICR4_LEN,
        pLineObj->icr4Values);

    VP_LINE_STATE(VpLineCtxType, pLineCtx, ("Vp880ApplyInternalTestTerm: Write ICR4 0x%02X 0x%02X 0x%02X 0x%02X Ch %d",
        pLineObj->icr4Values[0], pLineObj->icr4Values[1],
        pLineObj->icr4Values[2], pLineObj->icr4Values[3], pLineObj->channelId));

    /* Forcing the SLIC DC bias for 200ms helps collapse the battery
     * voltage, especially for fixed tracking designs.  We're taking over
     * ICR1 completely here.  Other parts of the code will set
     * pLineObj->icr1Values but will not actually write to the register
     * while this relay state is active.  See Vp880WriteICR1().  When we
     * leave this relay state, we will restore pLineObj->icr1Values */
    if (pDevObj->stateInt & VP880_IS_ABS) {
        icr1Reg[0] = 0xFF;
        icr1Reg[1] = 0x68;
        icr1Reg[2] = 0xFF;
        icr1Reg[3] = 0x06;
    } else {
        icr1Reg[0] = 0xFF;
        icr1Reg[1] = 0xFF;
        icr1Reg[2] = 0xFF;
        icr1Reg[3] = 0x0F;
    }
    VpMpiCmdWrapper(deviceId, ecVal, VP880_ICR1_WRT, VP880_ICR1_LEN, icr1Reg);

    /* Start a timer to change the ICR1 settings later to make tip and ring
     * outputs high impedance so that they tend to pull to battery. */
    if ((pDevObj->stateInt & VP880_IS_ABS) ||
        !(pDevObj->swParams[VP880_REGULATOR_TRACK_INDEX]
             & VP880_REGULATOR_FIXED_RING))
    {
        /* Use a short delay for ABS and non-fixed tracking devices */
        timerDelay = VP880_INTERNAL_TESTTERM_SETTLING_TIME_SHORT;
    } else {
        /* Use a longer delay for fixed tracking devices */
        timerDelay = VP880_INTERNAL_TESTTERM_SETTLING_TIME_LONG;
    }
    pLineObj->lineTimers.timers.timer[VP_LINE_INTERNAL_TESTTERM_TIMER] =
        MS_TO_TICKRATE(timerDelay, pDevObj->devProfileData.tickRate);
    pLineObj->lineTimers.timers.timer[VP_LINE_INTERNAL_TESTTERM_TIMER]
        |= VP_ACTIVATE_TIMER;

    pLineObj->internalTestTermApplied = TRUE;

    VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
} /* Vp880ApplyInternalTestTerm() */

/**
 * Vp880RemoveInternalTestTerm()
 *  This function reverts the settings that control the internal test
 * termination.
 */
static void
Vp880RemoveInternalTestTerm(
    VpLineCtxType *pLineCtx)
{
    Vp880LineObjectType *pLineObj = pLineCtx->pLineObj;
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpDeviceIdType deviceId = pDevObj->deviceId;
    uint8 ecVal = pLineObj->ecVal;

    VpSysEnterCritical(deviceId, VP_CODE_CRITICAL_SEC);

    /* Restore VAB sensing */
    pLineObj->icr6Values[VP880_DC_CAL_CUT_INDEX] &= ~VP880_C_TIP_SNS_CUT;
    pLineObj->icr6Values[VP880_DC_CAL_CUT_INDEX] &= ~VP880_C_RING_SNS_CUT;
    VpMpiCmdWrapper(deviceId, ecVal, VP880_ICR6_WRT, VP880_ICR6_LEN,
        pLineObj->icr6Values);

    /* Restore ground key polarity setting */
    pLineObj->icr4Values[VP880_ICR4_GKEY_DET_LOCATION] &=
        ~VP880_ICR4_GKEY_POL;
    pLineObj->icr4Values[VP880_ICR4_GKEY_DET_LOCATION+1] &=
        ~VP880_ICR4_GKEY_POL;
    VpMpiCmdWrapper(deviceId, ecVal, VP880_ICR4_WRT, VP880_ICR4_LEN,
        pLineObj->icr4Values);

    VP_LINE_STATE(VpLineCtxType, pLineCtx, ("Vp880RemoveInternalTestTerm: Write ICR4 0x%02X 0x%02X 0x%02X 0x%02X Ch %d",
        pLineObj->icr4Values[0], pLineObj->icr4Values[1],
        pLineObj->icr4Values[2], pLineObj->icr4Values[3], pLineObj->channelId));

    /* Restore ICR1 to the cached value in pLineObj->icr1Values */
    VpMpiCmdWrapper(deviceId, ecVal, VP880_ICR1_WRT, VP880_ICR1_LEN,
        pLineObj->icr1Values);

    /* Deactivate the timer in case it is still running */
    pLineObj->lineTimers.timers.timer[VP_LINE_INTERNAL_TESTTERM_TIMER]
        &= ~VP_ACTIVATE_TIMER;

    pLineObj->internalTestTermApplied = FALSE;

    VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
} /* Vp880RemoveInternalTestTerm() */

/**
 * Vp880SetDTMFGenerators()
 *  This function sets signal generator A/B for DTMF tone generation.
 *
 * Preconditions:
 *  The line must first be initialized.
 *
 * Postconditions:
 *  The signal generators A/B are set to the DTMF frequencies and level required
 * by the digit passed.
 */
VpStatusType
Vp880SetDTMFGenerators(
    VpLineCtxType *pLineCtx,
    VpCidGeneratorControlType mode,
    VpDigitType digit)
{
    Vp880LineObjectType *pLineObj = pLineCtx->pLineObj;
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;

    uint8 ecVal = pLineObj->ecVal;
    uint8 sigGenCtrl[VP880_GEN_CTRL_LEN] = {VP880_GEN_ALLOFF};
    VpDeviceIdType deviceId = pDevObj->deviceId;

#ifdef CSLAC_SEQ_EN
    uint8 sigByteCount;
    uint8 sigOffset = VP_CID_PROFILE_FSK_PARAM_LEN + 2;
#endif

    uint8 sigGenABParams[VP880_SIGAB_PARAMS_LEN] = {
        0x00, 0x00, 0x00,  /* RSVD */
        0x00, 0x00, /* Replace with required column Frequency */
        0x1C, 0x32, /* Level = -10dBm */
        0x00, 0x00, /* Replace with required row Frequency */
        0x1C, 0x32  /* Level = -10dBm */
    };

    uint8 columnFreqs[] = {
        0x0C, 0xE5,    /* 1209Hz (1, 4, 7, *) */
        0x0E, 0x40,    /* 1336Hz (2, 5, 8, 0) */
        0x0F, 0xC1,    /* 1477Hz (3, 6, 9, #) */
        0x11, 0x6B     /* 1633Hz (A, B, C, D) */
    };

    uint8 rowFreqs[] = {
        0x07, 0x6F,    /* 697Hz (1, 2, 3, A) */
        0x08, 0x36,    /* 770Hz (4, 5, 6, B) */
        0x09, 0x16,    /* 852Hz (7, 8, 9, C) */
        0x0A, 0x09     /* 941Hz (*, 0, #, D) */
    };

    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880SetDTMFGenerators+"));

#ifdef CSLAC_SEQ_EN
    /*
     * If we're generating caller ID data set the levels based on the data in
     * the CID profile
     */
    if ((pLineObj->callerId.status & VP_CID_IN_PROGRESS) &&
        (pLineObj->callerId.pCliProfile != VP_PTABLE_NULL)) {
        for (sigByteCount = 0; sigByteCount < (VP880_SIGAB_PARAMS_LEN - 3);
             sigByteCount++) {
            sigGenABParams[sigByteCount+3] =
                pLineObj->callerId.pCliProfile[sigOffset + sigByteCount];
        }
    } else {
#endif
        /*
         * If it's an FXO line then the DTMF high and low frequency levels are
         * specified in the FXO/Dialing Profile, cached in the line object.
         */
        if (pLineObj->status & VP880_IS_FXO) {
            sigGenABParams[5] = pLineObj->digitGenStruct.dtmfHighFreqLevel[0];
            sigGenABParams[6] = pLineObj->digitGenStruct.dtmfHighFreqLevel[1];
            sigGenABParams[9] = pLineObj->digitGenStruct.dtmfLowFreqLevel[0];
            sigGenABParams[10] = pLineObj->digitGenStruct.dtmfLowFreqLevel[1];
        }
#ifdef CSLAC_SEQ_EN
    }
#endif

    /* Set the Column Freqs first */
    switch(digit) {
        case 1:
        case 4:
        case 7:
        case VP_DIG_ASTER:
            sigGenABParams[3] = columnFreqs[0];
            sigGenABParams[4] = columnFreqs[1];
            break;

        case 2:
        case 5:
        case 8:
        case VP_DIG_ZERO:
            sigGenABParams[3] = columnFreqs[2];
            sigGenABParams[4] = columnFreqs[3];
            break;

        case 3:
        case 6:
        case 9:
        case VP_DIG_POUND:
            sigGenABParams[3] = columnFreqs[4];
            sigGenABParams[4] = columnFreqs[5];
            break;

        case VP_DIG_A:
        case VP_DIG_B:
        case VP_DIG_C:
        case VP_DIG_D:
            sigGenABParams[3] = columnFreqs[6];
            sigGenABParams[4] = columnFreqs[7];
            break;

        default:
            VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880SetDTMFGenerators-"));
            return VP_STATUS_INVALID_ARG;
    }

    /* Now set the row freqs */
    switch(digit) {
        case 1:
        case 2:
        case 3:
        case VP_DIG_A:
            sigGenABParams[7] = rowFreqs[0];
            sigGenABParams[8] = rowFreqs[1];
            break;

        case 4:
        case 5:
        case 6:
        case VP_DIG_B:
            sigGenABParams[7] = rowFreqs[2];
            sigGenABParams[8] = rowFreqs[3];
            break;

        case 7:
        case 8:
        case 9:
        case VP_DIG_C:
            sigGenABParams[7] = rowFreqs[4];
            sigGenABParams[8] = rowFreqs[5];
            break;

        case VP_DIG_ASTER:
        case VP_DIG_ZERO:
        case VP_DIG_POUND:
        case VP_DIG_D:
            sigGenABParams[7] = rowFreqs[6];
            sigGenABParams[8] = rowFreqs[7];
            break;

        default:
            VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880SetDTMFGenerators-"));
            return VP_STATUS_INVALID_ARG;
    }

    VpMpiCmdWrapper(deviceId, ecVal, VP880_SIGAB_PARAMS_WRT,
        VP880_SIGAB_PARAMS_LEN, sigGenABParams);

    /*
     * If there is no change to generator control required, it is assumed to be
     * set properly prior to this function call.
     */
    if (mode != VP_CID_NO_CHANGE) {
        /*
         * For DTMF CID, the data passed may be message data, a keyed character
         * (e.g., Mark, Channel Seizure), or End of Transmission. If it's End
         * of Transmission, disable the DTMF generators immediately. Otherwise,
         * enable the DTMF generators
         */
        if ((mode == VP_CID_GENERATOR_DATA)
         || (mode == VP_CID_GENERATOR_KEYED_CHAR)) {
            sigGenCtrl[0] |= (VP880_GENA_EN | VP880_GENB_EN);

            /* Setup the line timer for the on-time for DTMF CID */
            pLineObj->lineTimers.timers.timer[VP_LINE_TIMER_CID_DTMF] =
                MS_TO_TICKRATE(VP_CID_DTMF_ON_TIME,
                    pDevObj->devProfileData.tickRate);

            pLineObj->lineTimers.timers.timer[VP_LINE_TIMER_CID_DTMF]
                |= VP_ACTIVATE_TIMER;
#ifdef CSLAC_SEQ_EN
            pLineObj->callerId.dtmfStatus |= VP_CID_ACTIVE_ON_TIME;
#endif
        }

        VpMpiCmdWrapper(deviceId, ecVal, VP880_GEN_CTRL_WRT,
            VP880_GEN_CTRL_LEN, sigGenCtrl);
    }
    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880SetDTMFGenerators-"));
    return VP_STATUS_SUCCESS;
}

/**
 * Vp880SetOption()
 *  This function determines how to process the Option based on pDevCtx,
 * pLineCtx, and option type.  The actual options are implemented in
 * Vp880SetOptionInternal
 *
 * Preconditions:
 *  The line must first be initialized if a line context is passed, or the
 * device must be initialized if a device context is passed.
 *
 * Postconditions:
 *  The option specified is implemented either on the line, or on the device, or
 * on all lines associated with the device (see the API Reference Guide for
 * details).
 */
VpStatusType
Vp880SetOption(
    VpLineCtxType *pLineCtx,
    VpDevCtxType *pDevCtx,
    VpOptionIdType option,
    void *value)
{
    uint8 channelId;
    Vp880DeviceObjectType *pDevObj;
    VpStatusType status = VP_STATUS_INVALID_ARG;

    VpDevCtxType *pDevCtxLocal;
    VpLineCtxType *pLineCtxLocal;
    Vp880LineObjectType *pLineObj;
    VpDeviceIdType deviceId;
    bool onlyFXO = TRUE;

    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880SetOption+"));

    if (pDevCtx != VP_NULL) {
        pDevObj = pDevCtx->pDevObj;
        deviceId = pDevObj->deviceId;

        /* Proceed if device state is either in progress or complete */
        if (pDevObj->status.state & (VP_DEV_INIT_CMP | VP_DEV_INIT_IN_PROGRESS)) {
        } else {
            VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880SetOption-"));
            return VP_STATUS_DEV_NOT_INITIALIZED;
        }

        /*
         * Do not proceed if the device calibration is in progress. This could
         * damage the device.
         */
        if (pDevObj->status.state & VP_DEV_IN_CAL) {
            VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880SetOption-"));
            return VP_STATUS_DEV_NOT_INITIALIZED;
        }

        VpSysEnterCritical(deviceId, VP_CODE_CRITICAL_SEC);

        /*
         * Valid Device Context, we already know Line context is NULL (higher
         * layer SW, process on device if device option, or process on all lines
         * associated with device if line option
         */
        switch (option) {
            case VP_OPTION_ID_EVENT_MASK:  /* Line and Device */
                Vp880SetOptionInternal(VP_NULL, pDevCtx, option, value);

            /* Line Options */
            case VP_OPTION_ID_ZERO_CROSS:
            case VP_OPTION_ID_PULSE_MODE:
            case VP_OPTION_ID_TIMESLOT:
            case VP_OPTION_ID_CODEC:
            case VP_OPTION_ID_PCM_HWY:
            case VP_OPTION_ID_LOOPBACK:
            case VP_OPTION_ID_LINE_STATE:
            case VP_OPTION_ID_RING_CNTRL:
            case VP_OPTION_ID_PCM_TXRX_CNTRL:
                /*
                 * Loop through all of the valid channels associated with this
                 * device. Init status variable in case there are currently no
                 * line contexts associated with this device
                 */
                status = VP_STATUS_SUCCESS;
                for (channelId = 0; channelId < pDevObj->staticInfo.maxChannels; channelId++) {
                    pLineCtxLocal = pDevCtx->pLineCtx[channelId];

                    if (pLineCtxLocal == VP_NULL) {
                        continue;
                    }

                    if ((option == VP_OPTION_ID_ZERO_CROSS) ||
                        (option == VP_OPTION_ID_PULSE_MODE) ||
                        (option == VP_OPTION_ID_LINE_STATE) ||
                        (option == VP_OPTION_ID_RING_CNTRL)){
                        uint8 lastChannel = (pDevObj->staticInfo.maxChannels - 1);

                        pLineObj = pLineCtxLocal->pLineObj;

                        /* This device has at least 1 FXS, SetOption will succeed */
                        if (!(pLineObj->status & VP880_IS_FXO)) {
                            onlyFXO = FALSE;
                            status = Vp880SetOptionInternal(pLineCtxLocal, VP_NULL, option, value);
                        /* Only FXO on this device */
                        } else if ((onlyFXO == TRUE) && (channelId == lastChannel)) {
                            status = VP_STATUS_OPTION_NOT_SUPPORTED;
                        /* Just bailout in case there is at least 1 FXS on this device */
                        } else {
                            break;
                        }
                    } else {
                        status = Vp880SetOptionInternal(pLineCtxLocal, VP_NULL, option, value);
                    }

                    if (VP_STATUS_SUCCESS != status) {
                        VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
                        VP_API_FUNC_INT(VpLineCtxType, pLineCtxLocal, ("Vp880SetOption-"));
                        return status;
                    }
                }
                break;
            default:
                /*
                 * Device option, or option unknown option.  Handle in lower
                 * layer
                 */
                status = Vp880SetOptionInternal(VP_NULL, pDevCtx, option, value);
                break;
        }
    } else {
        /*
         * Line context must be valid, device context is NULL, proceed as
         * normal
         */
        pDevCtxLocal = pLineCtx->pDevCtx;
        pDevObj = pDevCtxLocal->pDevObj;
        deviceId = pDevObj->deviceId;

        /* Proceed if device state is either in progress or complete */
        if (pDevObj->status.state & (VP_DEV_INIT_CMP | VP_DEV_INIT_IN_PROGRESS)) {
        } else {
            VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880SetOption-"));
            return VP_STATUS_DEV_NOT_INITIALIZED;
        }

        /*
         * Do not proceed if the device calibration is in progress. This could
         * damage the device.
         */
        if (pDevObj->status.state & VP_DEV_IN_CAL) {
            VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880SetOption-"));
            return VP_STATUS_DEV_NOT_INITIALIZED;
        }

        VpSysEnterCritical(deviceId, VP_CODE_CRITICAL_SEC);
        status = Vp880SetOptionInternal(pLineCtx, VP_NULL, option, value);
    }

    VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);

    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880SetOption-"));
    return status;
}

/**
 * Vp880SetOptionInternal()
 *  This function implements on the Vp880 device the options specified from
 * Vp880SetOption().  No other function should call this function.
 *
 * Preconditions:
 *  See Vp880SetOption()
 *
 * Postconditions:
 *  See Vp880SetOption()
 */
VpStatusType
Vp880SetOptionInternal(
    VpLineCtxType *pLineCtx,
    VpDevCtxType *pDevCtx,
    VpOptionIdType option,
    void *value)
{
    VpDevCtxType *pDevCtxLocal;
    VpLineCtxType *pLineCtxLocal;

    VpStatusType status = VP_STATUS_SUCCESS;

    Vp880LineObjectType *pLineObj;
    Vp880DeviceObjectType *pDevObj;
    uint8 tempData[VP880_INT_MASK_LEN], channelId, txSlot, rxSlot;

    VpDeviceIdType deviceId;

    VpOptionDeviceIoType deviceIo;

    uint8 maxChan, mpiData;
    uint8 mpiByte = 0;
    uint8 ioDirection[2] = {0x00, 0x00};
    uint8 tempSysConfig[VP880_SS_CONFIG_LEN];
    uint8 tempLoopBack[VP880_LOOPBACK_LEN];
    uint8 ecVal;
    uint16 eventMask;

    VpOptionEventMaskType *pEventsMask, *pNewEventsMask;

    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880SetOptionInternal+"));

    if (pLineCtx != VP_NULL) {
        pDevCtxLocal = pLineCtx->pDevCtx;
        pDevObj = pDevCtxLocal->pDevObj;
        deviceId = pDevObj->deviceId;
        pLineObj = pLineCtx->pLineObj;
        channelId = pLineObj->channelId;
        ecVal = pLineObj->ecVal;

        switch (option) {
            /* Line Options */
            case VP_OPTION_ID_PULSE_MODE:
                if (pLineObj->status & VP880_IS_FXO) {
                    status = VP_STATUS_OPTION_NOT_SUPPORTED;
                    break;
                }

                if (pLineObj->pulseMode != *((VpOptionPulseModeType *)value)) {
                    pLineObj->pulseMode = *((VpOptionPulseModeType *)value);
                    VpInitDP(&pLineObj->dpStruct);
                    VpInitDP(&pLineObj->dpStruct2);
                }
                break;

            case VP_OPTION_ID_TIMESLOT:
                txSlot = ((VpOptionTimeslotType *)value)->tx;
                rxSlot = ((VpOptionTimeslotType *)value)->rx;
                status = Vp880SetTimeSlot(pLineCtx, txSlot, rxSlot);
                break;

            case VP_OPTION_ID_CODEC:
                status = Vp880SetCodec(pLineCtx, *((VpOptionCodecType *)value));
                break;

            case VP_OPTION_ID_PCM_HWY:
                if (*((VpOptionPcmHwyType *)value) != VP_OPTION_HWY_A) {
                    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880SetOptionInternal-"));
                    return VP_STATUS_INVALID_ARG;
                }
                break;

            case VP_OPTION_ID_LOOPBACK:
                /* Timeslot loopback via loopback register */
                VpMpiCmdWrapper(deviceId, ecVal, VP880_LOOPBACK_RD,
                    VP880_LOOPBACK_LEN, tempLoopBack);

                switch(*((VpOptionLoopbackType *)value)) {
                    case VP_OPTION_LB_TIMESLOT:
                        tempLoopBack[0] |= VP880_INTERFACE_LOOPBACK_EN;
                        break;

                    case VP_OPTION_LB_OFF:
                        tempLoopBack[0] &= ~(VP880_INTERFACE_LOOPBACK_EN);
                        break;

                    case VP_OPTION_LB_DIGITAL:
                    default:
                        VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880SetOptionInternal-"));
                        return VP_STATUS_INVALID_ARG;
                }
                VP_INFO(VpLineCtxType, pLineCtx,("Writing Op Cond (Loopback) 0x%02X", tempLoopBack[0]));

                VpMpiCmdWrapper(deviceId, ecVal, VP880_LOOPBACK_WRT,
                    VP880_LOOPBACK_LEN, tempLoopBack);
                break;

            case VP_OPTION_ID_LINE_STATE:
                /* Option does not apply to FXO */
                if (pLineObj->status & VP880_IS_FXO) {
                    status = VP_STATUS_OPTION_NOT_SUPPORTED;
                    break;
                }

                /*
                 * Only supports one type of battery control, so make sure it
                 * is set correctly. If not, return error otherwise continue
                 */
                if (((VpOptionLineStateType *)value)->bat
                    != VP_OPTION_BAT_AUTO) {
                    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880SetOptionInternal-"));
                    return VP_STATUS_INVALID_ARG;
                }

                VpMpiCmdWrapper(deviceId, ecVal, VP880_SS_CONFIG_RD,
                    VP880_SS_CONFIG_LEN, tempSysConfig);
                if (((VpOptionLineStateType *)value)->battRev == TRUE) {
                    tempSysConfig[0] &= ~(VP880_SMOOTH_PR_EN);
                } else {
                    tempSysConfig[0] |= VP880_SMOOTH_PR_EN;
                }
                VpMpiCmdWrapper(deviceId, ecVal, VP880_SS_CONFIG_WRT,
                    VP880_SS_CONFIG_LEN, tempSysConfig);
                break;

            case VP_OPTION_ID_EVENT_MASK:
                pNewEventsMask = (VpOptionEventMaskType *)value;

                /* Zero out the line-specific bits before setting the
                 * deviceEventsMask in the device object. */
                pEventsMask = &pDevObj->deviceEventsMask;
                pEventsMask->faults =
                    pNewEventsMask->faults & VP_EVCAT_FAULT_DEV_EVENTS;
                pEventsMask->signaling =
                    pNewEventsMask->signaling & VP_EVCAT_SIGNALING_DEV_EVENTS;
                pEventsMask->response =
                    pNewEventsMask->response & VP_EVCAT_RESPONSE_DEV_EVENTS;
                pEventsMask->test =
                    pNewEventsMask->test & VP_EVCAT_TEST_DEV_EVENTS;
                pEventsMask->process =
                    pNewEventsMask->process & VP_EVCAT_PROCESS_DEV_EVENTS;
                pEventsMask->fxo =
                    pNewEventsMask->fxo & VP_EVCAT_FXO_DEV_EVENTS;

                /* Zero out the device-specific bits before setting the
                 * lineEventsMask in the line object. */
                pEventsMask = &pLineObj->lineEventsMask;
                pEventsMask->faults =
                    pNewEventsMask->faults & ~VP_EVCAT_FAULT_DEV_EVENTS;
                pEventsMask->signaling =
                    pNewEventsMask->signaling & ~VP_EVCAT_SIGNALING_DEV_EVENTS;
                pEventsMask->response =
                    pNewEventsMask->response & ~VP_EVCAT_RESPONSE_DEV_EVENTS;
                pEventsMask->test =
                    pNewEventsMask->test & ~VP_EVCAT_TEST_DEV_EVENTS;
                pEventsMask->process =
                    pNewEventsMask->process & ~VP_EVCAT_PROCESS_DEV_EVENTS;
                pEventsMask->fxo =
                    pNewEventsMask->fxo & ~VP_EVCAT_FXO_DEV_EVENTS;

                /* Unmask the unmaskable */
                VpImplementNonMaskEvents(&pLineObj->lineEventsMask,
                    &pDevObj->deviceEventsMask);

                /* Mask those events that the VP880 API-II cannot generate */
                Vp880MaskNonSupportedEvents(&pLineObj->lineEventsMask,
                    &pDevObj->deviceEventsMask);

                /*
                 * The next code section prevents the device from interrupting
                 * the processor if all of the events associated with the
                 * specific hardware interrupt are masked
                 */
                VpMpiCmdWrapper(deviceId, ecVal, VP880_INT_MASK_RD,
                    VP880_INT_MASK_LEN, tempData);

                /* Keep Clock Fault Interrupt Enabled for auto-free run mode. */
                tempData[0] &= ~VP880_CFAIL_MASK;

                if (pDevObj->deviceEventsMask.faults & VP_DEV_EVID_CLK_FLT) {
                    tempData[0] &= ~VP880_CFAIL_MASK;
                }

                if (!(pLineObj->status & VP880_IS_FXO)) {  /* Line is FXS */
                    /* Mask off the FXO events */
                    pLineObj->lineEventsMask.fxo |= VP_EVCAT_FXO_MASK_ALL;

                    /*
                     * Never mask the thermal fault interrupt otherwise the
                     * actual thermal fault may not be seen by the VP-API-II.
                     */
                    tempData[channelId] &= ~VP880_TEMPA1_MASK;

                    /*
                     * Never mask the hook interrupt otherwise interrupt modes
                     * of the VP-API-II for LPM types won't work -- hook status
                     * is never updated, leaky line never properly detected.
                     */
                    tempData[channelId] &= ~VP880_HOOK1_MASK;

                    /*
                     * Never mask the gkey interrupt otherwise interrupt modes
                     * of the VP-API-II won't support "get line status"
                     * correctly.
                     */
                    tempData[channelId] &= ~VP880_GNK1_MASK;

                    /* Implement Operation Note 8 on errata notice V103 */
                    tempData[channelId] &= ~(VP880_OCALMY_MASK);
                } else {  /* Line is FXO */
                    /* Mask off the FXS events */
                    pLineObj->lineEventsMask.signaling
                        |= VP880_FXS_SIGNALING_EVENTS;

                    /* Evaluate for fxo events */
                    eventMask = pLineObj->lineEventsMask.fxo;
                    if ((eventMask & VP_LINE_EVID_LIU)
                     && (eventMask & VP_LINE_EVID_LNIU)) {
                        tempData[channelId] |= VP880_LIU1_MASK;
                    } else {
                        tempData[channelId] &= ~VP880_LIU1_MASK;
                    }

                    if ((eventMask & VP_LINE_EVID_RING_ON)
                     && (eventMask & VP_LINE_EVID_RING_OFF)) {
                        tempData[channelId] |= VP880_RING1_DET_MASK;
                        if (eventMask & VP_LINE_EVID_POLREV) {
                            tempData[channelId] |= VP880_POL1_MASK;
                        }
                    } else {
                        tempData[channelId] &= ~VP880_RING1_DET_MASK;
                        tempData[channelId] &= ~VP880_POL1_MASK;
                    }

                    if ((eventMask & VP_LINE_EVID_DISCONNECT)
                     && (eventMask & VP_LINE_EVID_RECONNECT)
                     && (eventMask & VP_LINE_EVID_FEED_DIS)
                     && (eventMask & VP_LINE_EVID_FEED_EN)) {
                        tempData[channelId] |= VP880_DISC1_MASK;
                    } else {
                        tempData[channelId] &= ~VP880_DISC1_MASK;
                    }
                }
                VpMpiCmdWrapper(deviceId, ecVal, VP880_INT_MASK_WRT,
                    VP880_INT_MASK_LEN, tempData);
                break;

            case VP_OPTION_ID_ZERO_CROSS:
                /* Option does not apply to FXO */
                if (pLineObj->status & VP880_IS_FXO) {
                    status = VP_STATUS_OPTION_NOT_SUPPORTED;
                    break;
                }

                VpMpiCmdWrapper(deviceId, ecVal, VP880_SS_CONFIG_RD,
                    VP880_SS_CONFIG_LEN, tempSysConfig);
                if (*(VpOptionZeroCrossType *)value == VP_OPTION_ZC_NONE) {
                    tempSysConfig[0] |= VP880_ZXR_DIS;
                } else {
                    tempSysConfig[0] &= ~(VP880_ZXR_DIS);
                }
                VpMpiCmdWrapper(deviceId, ecVal, VP880_SS_CONFIG_WRT,
                    VP880_SS_CONFIG_LEN, tempSysConfig);

                pLineObj->ringCtrl.zeroCross = *((VpOptionZeroCrossType *)value);
                break;

            case VP_OPTION_ID_RING_CNTRL: {
                VpOptionRingControlType TempRingCtrl = *((VpOptionRingControlType *)value);

                /* Option does not apply to FXO */
                if (pLineObj->status & VP880_IS_FXO) {
                    status = VP_STATUS_OPTION_NOT_SUPPORTED;
                    break;
                }

                if (Vp880IsSupportedFxsState(TempRingCtrl.ringTripExitSt) == FALSE) {
                    return VP_STATUS_INVALID_ARG;
                }

                pLineObj->ringCtrl = *((VpOptionRingControlType *)value);

                VpMpiCmdWrapper(deviceId, ecVal, VP880_SS_CONFIG_RD,
                    VP880_SS_CONFIG_LEN, tempSysConfig);
                if (pLineObj->ringCtrl.zeroCross == VP_OPTION_ZC_NONE) {
                    tempSysConfig[0] |= VP880_ZXR_DIS;
                } else {
                    tempSysConfig[0] &= ~(VP880_ZXR_DIS);
                }

                VpMpiCmdWrapper(deviceId, ecVal, VP880_SS_CONFIG_WRT,
                    VP880_SS_CONFIG_LEN, tempSysConfig);
                break;
            }

            case VP_OPTION_ID_PCM_TXRX_CNTRL:
                pLineObj->pcmTxRxCtrl = *((VpOptionPcmTxRxCntrlType *)value);
                VpMpiCmdWrapper(deviceId, ecVal, VP880_OP_COND_RD,
                    VP880_OP_COND_LEN, &mpiData);
                mpiData &= ~(VP880_CUT_TXPATH | VP880_CUT_RXPATH);
                mpiData &= ~(VP880_HIGH_PASS_DIS | VP880_OPCOND_RSVD_MASK);

                Vp880GetTxRxPcmMode(pLineObj, pLineObj->lineState.currentState,
                    &mpiByte);
                mpiData |= mpiByte;

                VP_INFO(VpLineCtxType, pLineCtx, ("6. Writing 0x%02X to Operating Conditions",
                    mpiData));
                VpMpiCmdWrapper(deviceId, ecVal, VP880_OP_COND_WRT,
                    VP880_OP_COND_LEN, &mpiData);
                break;

#ifdef VP_DEBUG
            case VP_OPTION_ID_DEBUG_SELECT:
                /* Update the debugSelectMask in the Line Object. */
                pLineObj->debugSelectMask = *(uint32 *)value;
                break;
#endif
            case VP_DEVICE_OPTION_ID_PULSE:
            case VP_DEVICE_OPTION_ID_PULSE2:
            case VP_DEVICE_OPTION_ID_CRITICAL_FLT:
            case VP_DEVICE_OPTION_ID_DEVICE_IO:
                status = VP_STATUS_INVALID_ARG;
                break;

            default:
                status = VP_STATUS_OPTION_NOT_SUPPORTED;
                break;
        }
    } else {
        pDevObj = pDevCtx->pDevObj;
        deviceId = pDevObj->deviceId;
        maxChan = pDevObj->staticInfo.maxChannels;
        ecVal = pDevObj->ecVal;

        switch (option) {
            case VP_DEVICE_OPTION_ID_PULSE:
                if (pDevObj->stateInt & VP880_IS_FXO_ONLY) {
                    status = VP_STATUS_OPTION_NOT_SUPPORTED;
                    break;
                }
                pDevObj->pulseSpecs = *((VpOptionPulseType *)value);
                break;

            case VP_DEVICE_OPTION_ID_PULSE2:
                if (pDevObj->stateInt & VP880_IS_FXO_ONLY) {
                    status = VP_STATUS_OPTION_NOT_SUPPORTED;
                    break;
                }
                pDevObj->pulseSpecs2 = *((VpOptionPulseType *)value);
                break;

            case VP_DEVICE_OPTION_ID_CRITICAL_FLT:
                if (pDevObj->stateInt & VP880_IS_FXO_ONLY) {
                    status = VP_STATUS_OPTION_NOT_SUPPORTED;
                    break;
                }

                pDevObj->criticalFault = *((VpOptionCriticalFltType *)value);

                if ((pDevObj->criticalFault.acFltDiscEn == TRUE)
                 || (pDevObj->criticalFault.dcFltDiscEn == TRUE)) {
                    pDevObj->criticalFault.acFltDiscEn = FALSE;
                    pDevObj->criticalFault.dcFltDiscEn = FALSE;
                    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880SetOptionInternal-"));
                    return VP_STATUS_INVALID_ARG;
                }

                for (channelId = 0; channelId < maxChan; channelId++) {
                    pLineCtxLocal = pDevCtx->pLineCtx[channelId];
                    if (pLineCtxLocal != VP_NULL) {
                        pLineObj = pLineCtxLocal->pLineObj;

                        if (!(pLineObj->status & VP880_IS_FXO)) {
                            ecVal = pLineObj->ecVal;
                            VpMpiCmdWrapper(deviceId, ecVal, VP880_SS_CONFIG_RD,
                                VP880_SS_CONFIG_LEN, tempSysConfig);

                            if (pDevObj->criticalFault.thermFltDiscEn == TRUE) {
                                tempSysConfig[0] |= VP880_ATFS_EN;
                            } else {
                                tempSysConfig[0] &= ~VP880_ATFS_EN;
                            }
                            VpMpiCmdWrapper(deviceId, ecVal,
                                VP880_SS_CONFIG_WRT, VP880_SS_CONFIG_LEN,
                                tempSysConfig);
                        }
                    }
                }
                break;

            case VP_DEVICE_OPTION_ID_DEVICE_IO: {
                uint8 ioTypeReq[2] = {0x00, 0x00};
                uint8 ecMod[] = {VP880_EC_CH1, VP880_EC_CH2};

                /* VE8830 Chip set does not have I/O pins */
                if (pDevObj->staticInfo.rcnPcn[1] == VP880_DEV_PCN_88536) {
                    return VP_STATUS_OPTION_NOT_SUPPORTED;
                }

                deviceIo = *(VpOptionDeviceIoType *)(value);

                /*
                 * Read the current direction pins and create a local array
                 * that matches what the input is requesting.
                 */
                for (channelId = 0; channelId < maxChan; channelId++) {
                    uint8 pinCnt = 0;

                    VpMpiCmdWrapper(deviceId, (ecVal | ecMod[channelId]),
                        VP880_IODIR_REG_RD, VP880_IODIR_REG_LEN,
                        &ioDirection[channelId]);

                    for (pinCnt = 0; pinCnt < VP880_MAX_PINS_PER_LINE; pinCnt++) {
                        if (deviceIo.directionPins_31_0 & (1 << (channelId + 2 * pinCnt))) {
                            if (pinCnt == 0) {
                                ioTypeReq[channelId] |=
                                    ((deviceIo.outputTypePins_31_0 & (1 << (channelId + 2 * pinCnt)))
                                    ? VP880_IODIR_IO1_OPEN_DRAIN : VP880_IODIR_IO1_OUTPUT);
                            } else {
                                ioTypeReq[channelId] |= (VP880_IODIR_IO2_OUTPUT << (pinCnt - 1));
                            }
                        } else {
                            /*
                             * This is here for show only. Input is 0, so no
                             * OR operation is needed.
                             */
                            /*  ioTypeReq[channelId] |= VP880_IODIR_IO1_INPUT; */
                        }
                    }

                    /* Protect the I/O lines dedictated to termination types */
                    pLineCtxLocal = pDevCtx->pLineCtx[channelId];

                    if (pLineCtxLocal != VP_NULL) {
                        uint8 fxoMask;
                        pLineObj = pLineCtxLocal->pLineObj;
                        switch (pLineObj->termType) {
                            case VP_TERM_FXO_GENERIC:
                            case VP_TERM_FXO_DISC:
                                fxoMask = (VP880_FXO_CID_LINE == VP880_IODATA_IO2)
                                    ? VP880_IODIR_IO2_MASK : VP880_IODIR_IO3_MASK;

                                ioTypeReq[channelId] &= ~fxoMask;
                                ioTypeReq[channelId] |= (ioDirection[channelId] & fxoMask);

                                /*
                                 * No break required becuase FXO also has I/O1
                                 * dedicated.
                                 */

                            case VP_TERM_FXS_ISOLATE:
                            case VP_TERM_FXS_ISOLATE_LP:
                            case VP_TERM_FXS_SPLITTER:
                                ioTypeReq[channelId] &= ~VP880_IODIR_IO1_MASK;
                                ioTypeReq[channelId] |= (ioDirection[channelId] & VP880_IODIR_IO1_MASK);
                                break;

                            default:
                                break;
                        }
                    }
                }

                /* Set the current device IO control information */
                for (channelId = 0; channelId < maxChan; channelId++) {
                    VP_INFO(VpLineCtxType, pLineCtx, ("1. Write IODIR 0x%02X on Channel %d",
                        ioTypeReq[channelId], channelId));

                    VpMpiCmdWrapper(deviceId, (ecVal | ecMod[channelId]),
                        VP880_IODIR_REG_WRT, VP880_IODIR_REG_LEN,
                        &ioTypeReq[channelId]);
                }
                }
                break;

#ifdef VP_DEBUG
            case VP_OPTION_ID_DEBUG_SELECT:
                /* Update the debugSelectMask in the Device Object. */
                pDevObj->debugSelectMask = *(uint32 *)value;
                break;
#endif

            default:
                status = VP_STATUS_OPTION_NOT_SUPPORTED;
                break;
        }
    }
    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880SetOptionInternal-"));

    return status;
}

/**
 * Vp880MaskNonSupportedEvents()
 *  This function masks the events that are not supported by the VP880 API-II.
 * It should only be called by SetOptionInternal when event masks are being
 * modified.
 *
 * Preconditions:
 *  None. Utility function to modify event structures only.
 *
 * Postconditions:
 *  Event structures passed are modified with masked bits for non-supported
 * VP880 API-II events.
 */
void
Vp880MaskNonSupportedEvents(
    VpOptionEventMaskType *pLineEventsMask, /**< Line Events Mask to modify for
                                             * non-masking
                                             */
    VpOptionEventMaskType *pDevEventsMask)  /**< Device Events Mask to modify
                                             * for non-masking
                                             */
{
    VP_API_FUNC_INT(None, VP_NULL, ("Vp880MaskNonSupportedEvents+"));
    pLineEventsMask->faults |= VP880_NONSUPPORT_FAULT_EVENTS;
    pLineEventsMask->signaling |= VP880_NONSUPPORT_SIGNALING_EVENTS;
    pLineEventsMask->response |= VP880_NONSUPPORT_RESPONSE_EVENTS;
    pLineEventsMask->test |= VP880_NONSUPPORT_TEST_EVENTS;
    pLineEventsMask->process |= VP880_NONSUPPORT_PROCESS_EVENTS;
    pLineEventsMask->fxo |= VP880_NONSUPPORT_FXO_EVENTS;

    pDevEventsMask->faults |= VP880_NONSUPPORT_FAULT_EVENTS;
    pDevEventsMask->signaling |= VP880_NONSUPPORT_SIGNALING_EVENTS;
    pDevEventsMask->response |= VP880_NONSUPPORT_RESPONSE_EVENTS;
    pDevEventsMask->test |= VP880_NONSUPPORT_TEST_EVENTS;
    pDevEventsMask->process |= VP880_NONSUPPORT_PROCESS_EVENTS;
    pDevEventsMask->fxo |= VP880_NONSUPPORT_FXO_EVENTS;
    VP_API_FUNC_INT(None, VP_NULL, ("Vp880MaskNonSupportedEvents-"));
    return;
}

/**
 * Vp880DeviceIoAccess()
 *  This function is used to access device IO pins of the Vp880. See API-II
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
Vp880DeviceIoAccess(
    VpDevCtxType *pDevCtx,
    VpDeviceIoAccessDataType *pDeviceIoData)
{
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;

    VpLineCtxType *pLineCtx;
    Vp880LineObjectType *pLineObj;

    bool isDedicatedPins = FALSE;

    VpDeviceIdType deviceId = pDevObj->deviceId;
    uint8 ecVal;
    uint8 chanNum, maxChan;
    uint8 ioDataReg[2] = {0x00, 0x00};  /* IO Status from each channel */

    /*
     * tempIoData and tempIoMask are representations of the device content to be
     * written.
     */
    uint8 tempIoData[2] = {0x00, 0x00};
    uint8 tempIoMask[2] = {0x00, 0x00};

    VpDeviceIoAccessDataType *pAccessData =
        &(pDevObj->getResultsOption.optionData.deviceIoData);

    /* VE8830 Chip set does not have I/O pins */
    if (pDevObj->staticInfo.rcnPcn[1] == VP880_DEV_PCN_88536) {
        return VP_STATUS_FUNC_NOT_SUPPORTED;
    }

    VP_API_FUNC_INT(VpDevCtxType, pDevCtx, ("Vp880DeviceIoAccess+"));

    maxChan = pDevObj->staticInfo.maxChannels;

    /* Proceed if device state is either in progress or complete */
    if (pDevObj->status.state & (VP_DEV_INIT_CMP | VP_DEV_INIT_IN_PROGRESS)) {
    } else {
        VP_API_FUNC_INT(VpDevCtxType, pDevCtx, ("Vp880DeviceIoAccess-"));
        return VP_STATUS_DEV_NOT_INITIALIZED;
    }

    /*
     * Do not proceed if the device calibration is in progress. This could
     * damage the device.
     */
    if (pDevObj->status.state & VP_DEV_IN_CAL) {
        VP_API_FUNC_INT(VpDevCtxType, pDevCtx, ("Vp880DeviceIoAccess-"));
        return VP_STATUS_DEV_NOT_INITIALIZED;
    }

    VpSysEnterCritical(deviceId, VP_CODE_CRITICAL_SEC);

    for (chanNum = 0; chanNum < maxChan; chanNum++) {
        uint16 dataMask;
        uint8 loopCnt;
        uint16 tempData;
        for (loopCnt = 0; loopCnt < 6; loopCnt++) {
            dataMask = 0x01;
            dataMask = (dataMask << (chanNum + 2 * loopCnt));

            tempData = 0;
            tempData = (uint16)(pDeviceIoData->accessMask_31_0 & dataMask);
            tempIoMask[chanNum] |= (uint8)(tempData >> (chanNum + loopCnt));

            tempData = 0;
            tempData = (uint16)(pDeviceIoData->deviceIOData_31_0 & dataMask);

            tempIoData[chanNum] |= (uint8)(tempData >> (chanNum + loopCnt));
        }
    }

    /* Read the current state of the IO lines */
    for (chanNum = 0; chanNum < maxChan; chanNum++) {
        pLineCtx = pDevCtx->pLineCtx[chanNum];
        if (pLineCtx != VP_NULL) {
            pLineObj = pLineCtx->pLineObj;
            ecVal = pLineObj->ecVal;

            /* Protect the CID line for FXO type */
            if ((pLineObj->status & VP880_IS_FXO)
             || (pLineObj->termType == VP_TERM_FXO_DISC)) {
                if (tempIoMask[chanNum] & VP880_FXO_CID_LINE) {
                    VP_INFO(VpLineCtxType, pLineCtx, ("Dedicated Pin Error"));
                    isDedicatedPins = TRUE;
                }
                tempIoMask[chanNum] &= ~VP880_FXO_CID_LINE;
            } else {    /* Force Data [2:4] to 0 for FXS */
                tempIoData[chanNum] &= (VP880_IODATA_IO1 | VP880_IODATA_IO2);
            }

            /* Protect access to I/O1 if FXO or Relay Type terminations */
            if ((pLineObj->status & VP880_IS_FXO)
             || (pLineObj->termType == VP_TERM_FXS_ISOLATE)
             || (pLineObj->termType == VP_TERM_FXS_ISOLATE_LP)
             || (pLineObj->termType == VP_TERM_FXS_SPLITTER)) {

                if (tempIoMask[chanNum] & VP880_IODATA_IO1) {
                    VP_INFO(VpLineCtxType, pLineCtx, ("Dedicated Pin Error"));
                    isDedicatedPins = TRUE;
                }
                tempIoMask[chanNum] &= ~VP880_IODATA_IO1;
            }
        } else {
            VP_INFO(None, NULL, ("VpDeviceIoAccess: NULL Line Found on Ch %d",
                chanNum));
            ecVal = pDevObj->ecVal;
            ecVal |= ((chanNum == 0) ? VP880_EC_CH1 : VP880_EC_CH2);
        }

        /* Read the IO Data, whether a line exists or not */
        VpMpiCmdWrapper(deviceId, ecVal, VP880_IODATA_REG_RD,
            VP880_IODATA_REG_LEN, &ioDataReg[chanNum]);
    }

    *pAccessData = *pDeviceIoData;

    if (pDeviceIoData->accessType == VP_DEVICE_IO_WRITE) {
        for (chanNum = 0; chanNum < maxChan; chanNum++) {
            uint8 tempData = ioDataReg[chanNum];
            ecVal = pDevObj->ecVal;
            ecVal |= ((chanNum == 0) ? VP880_EC_CH1 : VP880_EC_CH2);

            tempData &= ~tempIoMask[chanNum];
            tempData |= (tempIoMask[chanNum] & tempIoData[chanNum]);

            VP_INFO(None, NULL, ("VpDeviceIoAccess: Write IODATA 0x%02X on Ch %d",
                tempData, chanNum));

            VpMpiCmdWrapper(deviceId, ecVal, VP880_IODATA_REG_WRT,
                VP880_IODATA_REG_LEN, &tempData);
        }
    } else {    /* VP_DEVICE_IO_READ */
        isDedicatedPins = FALSE;

        pAccessData->deviceIOData_31_0 = 0;
        pAccessData->deviceIOData_63_32 = 0;

        for (chanNum = 0; chanNum < maxChan; chanNum++) {
            uint8 loopCnt;
            uint32 tempIoRdData;
            uint16 dataMask;

            for (loopCnt = 0; loopCnt < 6; loopCnt++) {
                dataMask = 0x01;
                dataMask = (dataMask << loopCnt);

                /* Extract the bit we're after in this loop */
                tempIoRdData = ioDataReg[chanNum];

                /* This is the location per the device. Move to API location */
                tempIoRdData &= dataMask;
                tempIoRdData = (tempIoRdData << (chanNum + loopCnt));

                /* Mask off ONLY the bit being provided in this loop */
                dataMask = 0x01;
                dataMask = (dataMask << (chanNum + 2 * loopCnt));
                tempIoRdData &= dataMask;

                pAccessData->deviceIOData_31_0 |= tempIoRdData;
            }
        }
        pAccessData->deviceIOData_31_0 &= (uint16)pDeviceIoData->accessMask_31_0;
    }

    pDevObj->deviceEvents.response |= VP_DEV_EVID_IO_ACCESS_CMP;

    VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);

    VP_API_FUNC_INT(VpDevCtxType, pDevCtx, ("Vp880DeviceIoAccess-"));
    return ((isDedicatedPins == TRUE) ? VP_STATUS_DEDICATED_PINS : VP_STATUS_SUCCESS);
}

/**
 * Vp880SetCodec()
 *  This function sets the codec mode on the line specified.
 *
 * Preconditions:
 *  The line must first be initialized.
 *
 * Postconditions:
 *  The codec mode on the line is set.  This function returns the success code
 * if the codec mode specified is supported.
 */
VpStatusType
Vp880SetCodec(
    VpLineCtxType *pLineCtx,
    VpOptionCodecType codec)    /* Encoding, as defined by LineCodec typedef */
{
    Vp880LineObjectType *pLineObj = pLineCtx->pLineObj;
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpDeviceIdType deviceId = pDevObj->deviceId;

    uint8 codecReg;
    uint8 ecVal = pLineObj->ecVal;

    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880SetCodec+"));

    /* Basic error checking */
    if ((codec != VP_OPTION_LINEAR) && (codec != VP_OPTION_ALAW)
     && (codec != VP_OPTION_MLAW) && (codec != VP_OPTION_WIDEBAND)) {
        VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880SetCodec-"));
        return VP_STATUS_INVALID_ARG;
    }

    if ((codec == VP_OPTION_WIDEBAND)
     && (!(pDevObj->stateInt & VP880_WIDEBAND))) {
        VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880SetCodec-"));
        return VP_STATUS_INVALID_ARG;
    }

    /* Adjust the EC value for Wideband mode as needed */
    ecVal &= ~VP880_WIDEBAND_MODE;
    ecVal |= ((codec == VP_OPTION_WIDEBAND) ? VP880_WIDEBAND_MODE : 0);

    /*
     * Wideband requires 1/2 rate reduction in device programmed rate to
     * maintain the same real sample rate.
     */
    if(((pLineObj->codec == VP_OPTION_WIDEBAND) && (codec != VP_OPTION_WIDEBAND))
    || ((pLineObj->codec != VP_OPTION_WIDEBAND) && (codec == VP_OPTION_WIDEBAND))) {
        uint8 converterCfg[VP880_CONV_CFG_LEN];
        uint8 newValue;

        pDevObj->devTimer[VP_DEV_TIMER_WB_MODE_CHANGE] =
            MS_TO_TICKRATE(VP_WB_CHANGE_MASK_TIME,
            pDevObj->devProfileData.tickRate) | VP_ACTIVATE_TIMER;

        VpMpiCmdWrapper(deviceId, ecVal, VP880_CONV_CFG_RD, VP880_CONV_CFG_LEN,
            converterCfg);
        converterCfg[0] &= ~VP880_CC_RATE_MASK;

        /* Adjust the pcm buffer update rate based on the tickrate and CODEC */
        if(pDevObj->devProfileData.tickRate <=160) {
            newValue = ((codec == VP_OPTION_WIDEBAND) ? VP880_CC_4KHZ_RATE : VP880_CC_8KHZ_RATE);
        } else if(pDevObj->devProfileData.tickRate <=320){
            newValue = ((codec == VP_OPTION_WIDEBAND) ? VP880_CC_2KHZ_RATE : VP880_CC_4KHZ_RATE);
        } else if(pDevObj->devProfileData.tickRate <=640){
            newValue = ((codec == VP_OPTION_WIDEBAND) ? VP880_CC_1KHZ_RATE : VP880_CC_2KHZ_RATE);
        } else if(pDevObj->devProfileData.tickRate <=1280){
            newValue = ((codec == VP_OPTION_WIDEBAND) ? VP880_CC_500HZ_RATE : VP880_CC_1KHZ_RATE);
        } else {
            newValue = VP880_CC_500HZ_RATE;
        }

        pDevObj->txBufferDataRate = newValue;
        converterCfg[0] |= newValue;
        /*
         * If channel is going to Wideband mode, we can immediately update the
         * device object. But if leaving Wideband mode, we have to let the tick
         * manage it because the other line may still be in Wideband mode.
         */
        if (codec == VP_OPTION_WIDEBAND) {
            pDevObj->ecVal |= VP880_WIDEBAND_MODE;
        }

        VpMpiCmdWrapper(deviceId, ecVal, VP880_CONV_CFG_WRT, VP880_CONV_CFG_LEN,
            converterCfg);
    }

    /* Read the current state of the codec register */
    VpMpiCmdWrapper(deviceId, ecVal, VP880_CODEC_REG_RD,
        VP880_CODEC_REG_LEN, &codecReg);

    /* Enable the desired CODEC mode */
    switch(codec) {
        case VP_OPTION_LINEAR:      /* 16 bit linear PCM */
        case VP_OPTION_WIDEBAND:    /* Wideband asumes Linear PCM */
            codecReg |= VP880_LINEAR_CODEC;
            break;

        case VP_OPTION_ALAW:                /* A-law PCM */
            codecReg &= ~(VP880_LINEAR_CODEC | VP880_ULAW_CODEC);
            break;

        case VP_OPTION_MLAW:                /* u-law PCM */
            codecReg |= VP880_ULAW_CODEC;
            codecReg &= ~(VP880_LINEAR_CODEC);
            break;

        default:
            /* Cannot reach here.  Error checking at top */
            break;
    } /* Switch */

    VpMpiCmdWrapper(deviceId, ecVal, VP880_CODEC_REG_WRT,
        VP880_CODEC_REG_LEN, &codecReg);

    pLineObj->codec = codec;
    pLineObj->ecVal = ecVal;

    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880SetCodec-"));

    return VP_STATUS_SUCCESS;
}

/**
 * Vp880SetTimeSlot()
 *  This function set the RX and TX timeslot for a device channel. Valid
 * timeslot numbers start at zero. The upper bound is system dependent.
 *
 * Preconditions:
 *  The line must first be initialized.
 *
 * Postconditions:
 *  The timeslots on the line are set.  This function returns the success code
 * if the timeslot numbers specified are within the range of the device based on
 * the PCLK rate.
 */
VpStatusType
Vp880SetTimeSlot(
    VpLineCtxType *pLineCtx,
    uint8 txSlot,       /**< The TX PCM timeslot */
    uint8 rxSlot)       /**< The RX PCM timeslot */
{
    Vp880LineObjectType *pLineObj = pLineCtx->pLineObj;
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpDeviceIdType deviceId = pDevObj->deviceId;

    uint8 ecVal = pLineObj->ecVal;

    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880SetTimeSlot+"));

    /* Validate the tx time slot value */
    if(txSlot >= pDevObj->devProfileData.pcmClkRate/64) {
        VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880SetTimeSlot-"));
        return VP_STATUS_INVALID_ARG;
    }

    /* Validate the rx time slot value */
    if(rxSlot >= pDevObj->devProfileData.pcmClkRate/64) {
        VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880SetTimeSlot-"));
        return VP_STATUS_INVALID_ARG;
    }

    if (pDevObj->staticInfo.rcnPcn[1] == VP880_DEV_PCN_88536) {
        if (txSlot == 0) {
            VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880SetTimeSlot-"));
            return VP_STATUS_INVALID_ARG;
        }
        txSlot--;
    }

    VpMpiCmdWrapper(deviceId, ecVal, VP880_TX_TS_WRT,
        VP880_TX_TS_LEN, &txSlot);

    VpMpiCmdWrapper(deviceId, ecVal, VP880_RX_TS_WRT,
        VP880_RX_TS_LEN, &rxSlot);

    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880SetTimeSlot-"));
    return VP_STATUS_SUCCESS;
}

/**
 * Vp880VirtualISR()
 *  This function is called everytime the device causes an interrupt
 *
 * Preconditions
 *  A device interrupt has just occured
 *
 * Postcondition
 *  This function should be called from the each device's ISR.
 *  This function could be inlined to improve ISR performance.
 */
VpStatusType
Vp880VirtualISR(
    VpDevCtxType *pDevCtx)
{
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpDeviceIdType deviceId = pDevObj->deviceId;

    VP_API_FUNC_INT(VpDevCtxType, pDevCtx, ("Vp880VirtualISR+"));

#if defined (VP880_INTERRUPT_LEVTRIG_MODE)
    VpSysDisableInt(deviceId);
#endif
    /* Device Interrupt Received */
    VpSysEnterCritical(deviceId, VP_CODE_CRITICAL_SEC);
    pDevObj->status.state |= VP_DEV_PENDING_INT;
    VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);

    VP_API_FUNC_INT(VpDevCtxType, pDevCtx, ("Vp880VirtualISR-"));

    return VP_STATUS_SUCCESS;
}

/**
 * Vp880ApiTick()
 *  This function should be called on a periodic basis or attached to an
 * interrupt.
 *
 * Preconditions:
 *  The device must first be initialized.
 *
 * Postconditions:
 *  The value passed (by pointer) is set to TRUE if there is an updated event.
 * The user should call the GetEventStatus function to determine the cause of
 * the event (TRUE value set).  This function always returns the success code.
 */
VpStatusType
Vp880ApiTick(
    VpDevCtxType *pDevCtx,
    bool *pEventStatus)
{
    VpLineCtxType *pLineCtx;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    Vp880LineObjectType *pLineObj;
    VpDeviceIdType deviceId = pDevObj->deviceId;
    uint8 ecVal;

    uint8 channelId;
    uint8 maxChan = pDevObj->staticInfo.maxChannels;
    bool tempClkFault, tempBat1Fault, tempBat2Fault, lineInTest;
    bool intServCalled = FALSE;
    uint16 timeStampPre, tickAdder;

#ifdef CSLAC_SEQ_EN
    bool isSeqRunning = FALSE;
#endif

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

#if defined (VP880_INTERRUPT_LEVTRIG_MODE)
    VpSysEnableInt(deviceId);
#endif

    /* Ensure that device is initialized */
    if (!(pDevObj->status.state & VP_DEV_INIT_CMP)) {
        if (Vp880FindSoftwareInterrupts(pDevCtx)) {
            *pEventStatus = TRUE;
        }

        VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
        return VP_STATUS_SUCCESS;
    }

    /*
     * Figure out if either channel is using WB mode and adjust the device
     * ec value as needed. The device object ec value will be used next to
     * go back and adjust both valid line object ec values. This "dual loop"
     * ckeck is done in case the second channel is in WB but not the first.
     */
    pDevObj->ecVal &= ~VP880_WIDEBAND_MODE;
    for(channelId=0; channelId < maxChan; channelId++ ) {
        pLineCtx = pDevCtx->pLineCtx[channelId];
        if (pLineCtx == VP_NULL) {
            continue;
        }
        pLineObj = pLineCtx->pLineObj;

        if (pLineObj->codec == VP_OPTION_WIDEBAND) {
            pDevObj->ecVal |= VP880_WIDEBAND_MODE;
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
        pLineObj->ecVal &= ~VP880_WIDEBAND_MODE;
        pLineObj->ecVal |= pDevObj->ecVal;

        ecVal |= pLineObj->ecVal;
    }

    Vp880LowPowerMode(pDevCtx);

    /* Service API Timers */
    Vp880ServiceTimers(pDevCtx);

    if (pDevObj->status.state & VP_DEV_IN_CAL) {
        VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
        return VP_STATUS_SUCCESS;
   }

    /* Reset event pointers pointers */
    pDevObj->dynamicInfo.lastChan = 0;

    for (channelId = 0; channelId < maxChan; channelId++) {
        pLineCtx = pDevCtx->pLineCtx[channelId];
        if (pLineCtx != VP_NULL) {
            pLineObj = pLineCtx->pLineObj;

#ifdef CSLAC_SEQ_EN
            /* Evaluate if Cadencing is required */
            if (pLineObj->cadence.status & VP_CADENCE_STATUS_ACTIVE) {
                isSeqRunning = TRUE;
            }
#endif
            if (pDevObj->stateInt & VP880_CAL_RELOAD_REQ) {
                Vp880UpdateCalValue(pLineCtx);
            }
        }
    }
    pDevObj->stateInt &= ~VP880_CAL_RELOAD_REQ;

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

#if defined (VP880_EFFICIENT_POLLED_MODE)
    /* Poll the device PIO-INT line */
    pDevObj->status.state |=
        (VpSysTestInt(deviceId) ? VP_DEV_PENDING_INT : 0x00);
#elif defined (VP880_SIMPLE_POLLED_MODE)
    pDevObj->status.state |= VP_DEV_PENDING_INT;
#endif

    /*
     * Adjust the EC value for Wideband mode as needed and set the line test
     * flag if any line is under test.
     */
    lineInTest = FALSE;
    for (channelId = 0; channelId < maxChan; channelId++) {
        pLineCtx = pDevCtx->pLineCtx[channelId];
        if (pLineCtx != VP_NULL) {
            lineInTest = (Vp880IsChnlUndrTst(pDevObj, channelId) == TRUE) ?
                TRUE : lineInTest;
        }
    }
#if defined (VP880_INCLUDE_TESTLINE_CODE)
    /*
     * Also want to consider a line in test if running Read Loop Conditions.
     * But if in Read Loop Conditions, the function "..IsChn" returns FALSE.
     */
    lineInTest = ((pDevObj->currentTest.rdLoopTest == TRUE) ? TRUE : lineInTest);
#endif

    /* Limit the number of interrupts serviced during one tick */
    pDevObj->status.numIntServiced = 2;

    /*
     * Read this buffer once per tick IF there is an active interrupt, or if
     * running line test or being forced to.
     */
    if (pDevObj->staticInfo.rcnPcn[VP880_RCN_LOCATION] > VP880_REV_VC) {
        if ((pDevObj->status.state & VP_DEV_PENDING_INT)
         || (pDevObj->status.state & VP_DEV_FORCE_SIG_READ)
         || (lineInTest == TRUE)) {
            pDevObj->status.state &= ~VP_DEV_FORCE_SIG_READ;
            VpMpiCmdWrapper(deviceId, ecVal, VP880_TX_PCM_BUFF_RD,
                VP880_TX_PCM_BUFF_LEN, pDevObj->txBuffer);
            pDevObj->status.state |= VP_DEV_TEST_BUFFER_READ;
        } else {
            pDevObj->status.state &= ~VP_DEV_TEST_BUFFER_READ;
        }
    } else {
        pDevObj->status.state &= ~VP_DEV_TEST_BUFFER_READ;
    }

    /* Service all pending interrupts (up to 2) */
    while ((pDevObj->status.state & VP_DEV_PENDING_INT)
        && (pDevObj->status.numIntServiced > 0)) {
        VpMpiCmdWrapper(deviceId, ecVal, VP880_UL_SIGREG_RD,
            VP880_UL_SIGREG_LEN, pDevObj->intReg);

        /*******************************************************
         *         HANDLE Clock Fail Events                    *
         *******************************************************/
        if (!(pDevObj->devTimer[VP_DEV_TIMER_WB_MODE_CHANGE] & VP_ACTIVATE_TIMER)) {
            /* Get the current status of the fault bit */
            tempClkFault = (pDevObj->intReg[0] & VP880_CFAIL_MASK) ? TRUE : FALSE;
            /*
             * Compare it with what we already know.  If different, generate
             * events and update the line status bits
             */
            if(tempClkFault ^ pDevObj->dynamicInfo.clkFault) {
                if (!(pDevObj->stateInt & VP880_FORCE_FREE_RUN)) {
                    if (tempClkFault) {
                        /* Entering clock fault, possibly a system restart. */
                        Vp880FreeRun(pDevCtx, VP_FREE_RUN_START);

                        /*
                         * Clear the flag used to indicate that Vp880FreeRun() was
                         * called by the application -- because it wasn't.
                         */
                        pDevObj->stateInt &= ~VP880_FORCE_FREE_RUN;
                    } else {
                        /*
                         * Exiting clock fault (note: this function does not affect
                         * VP880_FORCE_FREE_RUN flag).
                         */
                        Vp880RestartComplete(pDevCtx);
                    }
                }

                pDevObj->dynamicInfo.clkFault = tempClkFault;
                pDevObj->deviceEvents.faults |= VP_DEV_EVID_CLK_FLT;
            }
        }

        /* Get the current status of the first battery fault bit */
        tempBat1Fault = (pDevObj->intReg[0] & VP880_OCALMY_MASK) ? TRUE : FALSE;
        tempBat2Fault = (pDevObj->intReg[0] & VP880_OCALMZ_MASK) ? TRUE : FALSE;

        /* If line 1 is FXO, the Y supply is ignored */
        pLineCtx = pDevCtx->pLineCtx[0];
        if (pLineCtx != VP_NULL) {
            pLineObj = pLineCtx->pLineObj;
            if (!(pLineObj->status & VP880_IS_FXO)) {
                if(tempBat1Fault ^ pDevObj->dynamicInfo.bat1Fault) {
                    pDevObj->dynamicInfo.bat1Fault = tempBat1Fault;
                    pDevObj->deviceEvents.faults |= VP_DEV_EVID_BAT_FLT;
                }
            }
        }

        /* If line 2 is FXO, the Z supply is ignored */
        pLineCtx = pDevCtx->pLineCtx[1];
        if (pLineCtx != VP_NULL) {
            pLineObj = pLineCtx->pLineObj;
            if (!(pLineObj->status & VP880_IS_FXO)) {
                if(tempBat2Fault ^ pDevObj->dynamicInfo.bat2Fault) {
                    pDevObj->dynamicInfo.bat2Fault = tempBat2Fault;
                    pDevObj->deviceEvents.faults |= VP_DEV_EVID_BAT_FLT;
                }
            }
        }

        /*
         * Compare it with what we already know.  If different, generate
         * events and update the line status bits
         */
        intServCalled = TRUE;
        Vp880ServiceInterrupts(pDevCtx);

        /*
         * If level triggered, the interrupt may have been disabled (to prevent
         * a flood of interrupts), so reenable it.
         */
    #if defined (VP880_INTERRUPT_LEVTRIG_MODE)
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
    #if defined (VP880_EFFICIENT_POLLED_MODE)
        /* Poll the PIO-INT line */
        pDevObj->status.state |=
            (VpSysTestInt(deviceId) ? VP_DEV_PENDING_INT : 0x00);
    #elif defined (VP880_SIMPLE_POLLED_MODE)
        pDevObj->status.state |= VP_DEV_PENDING_INT;
    #endif
    }/* End while Interrupts*/

    /* Make sure Vp880ServiceInterrupts() is called at least once per tick to
     * keep the API line status up to date */
    if (intServCalled == FALSE) {
        Vp880ServiceInterrupts(pDevCtx);
    }

    /* Update the dial pulse handler for lines that are set for pulse decode */
    for (channelId = 0; channelId < maxChan; channelId++) {
        pLineCtx = pDevCtx->pLineCtx[channelId];
        if (pLineCtx != VP_NULL) {
            pLineObj = pLineCtx->pLineObj;
            if (!(pLineObj->status & VP880_IS_FXO)) {
                Vp880ProcessFxsLine(pDevObj, pLineCtx);
            } else {
                Vp880ProcessFxoLine(pDevObj, pLineCtx);
            }
        }
    }

    /* Collect all event activity and report to the calling function */
    if (Vp880FindSoftwareInterrupts(pDevCtx)) {
        *pEventStatus = TRUE;
    }

    VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
    return VP_STATUS_SUCCESS;
}

/**
 * Vp880ProcessFxsLine()
 *  This function should only be called by Vp880ApiTick on FXS lines. It
 * performs all line processing for operations that are Tick based
 *
 * Preconditions:
 *  Conditions defined by purpose of Api Tick.
 *
 * Postconditions:
 *  The Api variables and events (as appropriate) for the line passed have been
 * updated.
 */
VpStatusType
Vp880ProcessFxsLine(
    Vp880DeviceObjectType *pDevObj,
    VpLineCtxType *pLineCtx)
{
    Vp880LineObjectType *pLineObj = pLineCtx->pLineObj;
    uint8 channelId = pLineObj->channelId;
    bool dpStatus[2] = {FALSE, FALSE};
    VpOptionEventMaskType lineEvents1;
    VpOptionEventMaskType lineEvents2;
    VpDialPulseDetectStatesType beforeState, afterState;
    uint8 hookStatus = 0, i, validSamples;
    uint8 hookIncrement;
    bool dp2Valid;

#ifdef CSLAC_SEQ_EN
    uint8 cidParam[VP880_CID_PARAM_LEN];
    uint8 cidState;
#endif

    /* Skip processing during line test */
    if (Vp880IsChnlUndrTst(pDevObj, channelId) == TRUE) {
        return VP_STATUS_SUCCESS;
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
        pDevObj->pulseSpecs2.flashMax == 0) {

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
            case VP880_CC_500HZ_RATE:
                hookIncrement = 16;
                break;

            case VP880_CC_1KHZ_RATE:
                hookIncrement = 8;
                break;

            case VP880_CC_2KHZ_RATE:
                hookIncrement = 4;
                break;

            case VP880_CC_4KHZ_RATE:
                hookIncrement = 2;
                break;

            case VP880_CC_8KHZ_RATE:
                hookIncrement = 1;
                break;

            default:
                /* We should never reach here */
                hookIncrement = 16;
                break;
        }

        validSamples = ((pDevObj->txBuffer[VP880_TX_BUF_HOOK_MSB_INDEX]
            & VP880_TX_BUF_LEN_MASK) >> 4);

        if (validSamples == 7) {
            validSamples = 6;
        }

        if (channelId == 0) {
            hookStatus = pDevObj->txBuffer[VP880_TX_BUF_HOOK_LSB_INDEX]
                & VP880_TX_BUF_HOOK_CHAN1_MASK;
        } else {
            hookStatus = (pDevObj->txBuffer[VP880_TX_BUF_HOOK_MSB_INDEX]
                & VP880_TX_BUF_HOOK_MSB_MASK) << 2;
            hookStatus |= ((pDevObj->txBuffer[VP880_TX_BUF_HOOK_LSB_INDEX]
                & VP880_TX_BUF_HOOK_CHAN2_MASK) >> 6);
        }

        if (pLineObj->status & VP880_LOW_POWER_EN) {
            hookStatus = ~hookStatus;
        }

        beforeState = pLineObj->dpStruct.state;

        /* Adjust the end of the loop open or closed period if the test buffer
         * has been updated.  VP_DEV_TEST_BUFFER_READ will only be set if the
         * device revision is > VC */
        if (pDevObj->status.state & VP_DEV_TEST_BUFFER_READ) {
            /*
            VP_HOOK(VpLineCtxType, pLineCtx, ("CH%d Validsamples %d, buffer %02X %02X %d%d%d%d%d%d\n",
                channelId,
                ((pDevObj->txBuffer[VP880_TX_BUF_HOOK_MSB_INDEX] & VP880_TX_BUF_LEN_MASK) >> 4),
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
            if (pLineObj->dpStruct.lc_time > VP880_PULSE_DETECT_ADJUSTMENT) {
                pLineObj->dpStruct.lc_time -= VP880_PULSE_DETECT_ADJUSTMENT;
            }
        }
        if (beforeState == VP_DP_DETECT_STATE_LOOP_CLOSE && !(pLineObj->dpStruct2.hookSt)) {
            if (pLineObj->dpStruct2.lc_time > VP880_PULSE_DETECT_ADJUSTMENT) {
                pLineObj->dpStruct2.lc_time -= VP880_PULSE_DETECT_ADJUSTMENT;
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
                pLineObj->dpStruct.lo_time += VP880_PULSE_DETECT_ADJUSTMENT;
                pLineObj->dpStruct2.lo_time += VP880_PULSE_DETECT_ADJUSTMENT;
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
                pLineObj->status |= VP880_DP_SET1_DONE;
            }
        }

        if (dpStatus[1] == TRUE && dp2Valid == TRUE) {
            pLineObj->signaling2 = lineEvents2.signaling;
            pLineObj->lineEventHandle = pDevObj->timeStamp;

            if (!(pLineObj->lineEvents.signaling & VP_LINE_EVID_BREAK_MAX)) {
                pLineObj->status |= VP880_DP_SET2_DONE;
            }
        }

        /* Report events if:
         *  Both DP sets are done, OR
         *  Set 1 is done and set 2 is invalid */
        if ((pLineObj->status & VP880_DP_SET1_DONE) &&
            ((pLineObj->status & VP880_DP_SET2_DONE) ||
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

            pLineObj->status &= ~(VP880_DP_SET1_DONE | VP880_DP_SET2_DONE);
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

            VpDeviceIdType deviceId = pDevObj->deviceId;
            uint8 ecVal = pLineObj->ecVal;

            /*
             * Check to see if the Device Buffer is empty. If it is, continue
             * with CID.
             */
            VpMpiCmdWrapper(deviceId, ecVal, VP880_CID_PARAM_RD,
                VP880_CID_PARAM_LEN, cidParam);
            cidState = (cidParam[0] & VP880_CID_STATE_MASK);

            if ((cidState == VP880_CID_STATE_URUN)
             || (cidState == VP880_CID_STATE_IDLE)) {
                pLineObj->suspendCid = FALSE;
                cidParam[0] &= ~(VP880_CID_FRAME_BITS);
                cidParam[0] |= VP880_CID_DIS;

                Vp880MuteChannel(pLineCtx, FALSE);
                VP_CID(VpLineCtxType, pLineCtx, ("Writing 0x%02X to CID Params", cidParam[0]));
                VpMpiCmdWrapper(deviceId, ecVal, VP880_CID_PARAM_WRT,
                    VP880_CID_PARAM_LEN, cidParam);
                VpCidSeq(pLineCtx);
            }
        } else {
            VpCidSeq(pLineCtx);
        }
    }
#endif

    return VP_STATUS_SUCCESS;
}

/**
 * Vp880ServiceInterrupts()
 *  This function should only be called by Vp880ApiTick when an interrupt
 * occurs.
 *
 * Preconditions:
 *  The device must first be initialized.
 *
 * Postconditions:
 *  The Global Signaling Register is read and the data is stored in the device
 * object.  Depending on the dial pulse mode option set, the hook event (on/off)
 * is generated if a hook status changed.  All FXO events are reported by this
 * function (i.e., no other processing necessary). This function will return
 * TRUE if an event has been generated.
 */
bool
Vp880ServiceInterrupts(
    VpDevCtxType *pDevCtx)
{
    VpLineCtxType *pLineCtx;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    Vp880LineObjectType *pLineObj;

    VpDeviceIdType deviceId = pDevObj->deviceId;
    uint8 ecVal;

    uint8 channelId;
    VpCslacLineCondType tempHookSt, tempGnkSt, tempThermFault;
    VpLineStateType state;

    bool retFlag = FALSE;
    bool freezeGkey;
    uint8 maxChannels = pDevObj->staticInfo.maxChannels;

    for (channelId = 0; channelId < maxChannels; channelId++) {
        pLineCtx = pDevCtx->pLineCtx[channelId];

        if (pLineCtx != VP_NULL) {
            pLineObj = pLineCtx->pLineObj;

            if (!(pLineObj->status & VP880_INIT_COMPLETE)) {
                continue;
            }

            ecVal = pLineObj->ecVal;
            state = pLineObj->lineState.currentState;

            if (!(pLineObj->status & VP880_IS_FXO)) {
                freezeGkey = FALSE;

                /*
                 * If debouncing for Ring Exit or Caller ID, ignore hook.
                 * Otherwise process.
                 */
                if ((pLineObj->lineTimers.timers.timer[VP_LINE_CID_DEBOUNCE] & VP_ACTIVATE_TIMER)
                 || (pLineObj->lineTimers.timers.timer[VP_LINE_RING_EXIT_DEBOUNCE] & VP_ACTIVATE_TIMER)
                 || (pLineObj->lineTimers.timers.timer[VP_LINE_POLREV_DEBOUNCE] & VP_ACTIVATE_TIMER)
                 || (pLineObj->lineTimers.timers.timer[VP_LINE_DISCONNECT_EXIT] & VP_ACTIVATE_TIMER)
                 || (pDevObj->devTimer[VP_DEV_TIMER_LP_CHANGE] & VP_ACTIVATE_TIMER)
                 || (pLineObj->lineTimers.timers.timer[VP_LINE_CAL_LINE_TIMER] & VP_ACTIVATE_TIMER)
                 || (pLineObj->lineState.calType != VP_CSLAC_CAL_NONE)
                 || (pDevObj->status.state & VP_DEV_IN_CAL)
                 || (pLineObj->lineTimers.timers.timer[VP_LINE_TRACKER_DISABLE] & VP_ACTIVATE_TIMER)
                 || (pLineObj->lineTimers.timers.timer[VP_LINE_GND_START_TIMER] & VP_ACTIVATE_TIMER)

#ifdef CSLAC_SEQ_EN
                 || ((pLineObj->cadence.status & VP_CADENCE_STATUS_ACTIVE)
                  && (pLineObj->intSequence[VP_PROFILE_TYPE_LSB] == VP_PRFWZ_PROFILE_FWD_DISC_INT))
                 || ((pLineObj->cadence.status & VP_CADENCE_STATUS_ACTIVE)
                  && (pLineObj->intSequence[VP_PROFILE_TYPE_LSB] == VP_PRFWZ_PROFILE_TIP_OPEN_INT))
#endif
                 || ((state == VP_LINE_DISCONNECT))
                 || ((state == VP_LINE_TIP_OPEN))) {
                    tempHookSt = (pLineObj->lineState.condition & VP_CSLAC_HOOK);
                } else {
                    if (pLineObj->status & VP880_LOW_POWER_EN) {
                        if (pDevObj->intReg[channelId] & VP880_HOOK1_MASK) {
                            tempHookSt = 0;
                        } else {
                            tempHookSt = VP_CSLAC_HOOK;
                        }
                    } else {
                        if (pDevObj->intReg[channelId] & VP880_HOOK1_MASK) {
                            tempHookSt = VP_CSLAC_HOOK;
                        } else {
                            tempHookSt = 0;
                        }
                    }
                }

                if (pDevObj->intReg[channelId] & VP880_TEMPA1_MASK) {
                    tempThermFault = VP_CSLAC_THERM_FLT;
                } else {
                    tempThermFault = 0;
                }

                if ((pDevObj->devTimer[VP_DEV_TIMER_LP_CHANGE] & VP_ACTIVATE_TIMER)
                 || (pLineObj->lineState.calType != VP_CSLAC_CAL_NONE)
                 || (state == VP_LINE_DISCONNECT)
                 || (pLineObj->lineTimers.timers.timer[VP_LINE_DISCONNECT_EXIT]  & VP_ACTIVATE_TIMER)
                 || (pLineObj->lineTimers.timers.timer[VP_LINE_RING_EXIT_DEBOUNCE] & VP_ACTIVATE_TIMER)
                 || (pLineObj->lineTimers.timers.timer[VP_LINE_GND_START_TIMER]  & VP_ACTIVATE_TIMER)) {
                    tempGnkSt = (pLineObj->lineState.condition & VP_CSLAC_GKEY);
                    freezeGkey = TRUE;
                } else {
                    if (pDevObj->intReg[channelId] & VP880_GNK1_MASK) {
                        tempGnkSt = VP_CSLAC_GKEY;
                    } else {
                        tempGnkSt = 0;
                    }
                }

                /*
                 * We "think" we know what Hook and Gkey are now, but it's
                 * possible the API-II is in the middle of the VoicePort Ground
                 * Start workaround. Check for the conditions where what is
                 * detected MUST be a Ground Key and not a Hook
                 */
                if ((!(pDevObj->devTimer[VP_DEV_TIMER_LP_CHANGE] & VP_ACTIVATE_TIMER))
                 && (freezeGkey == FALSE)) {
                    if ((state == VP_LINE_TIP_OPEN)
                     || (pLineObj->lineTimers.timers.timer[VP_LINE_GND_START_TIMER] & VP_ACTIVATE_TIMER)) {
                        uint8 currentHook = (pDevObj->intReg[channelId] & VP880_HOOK1_MASK);
                        tempGnkSt = (currentHook || tempGnkSt) ? VP_CSLAC_GKEY : 0;
                        tempHookSt = 0;
                    }
                }

                /* If the hook conditions changed, continue line processing */
                if((pLineObj->lineState.condition & VP_CSLAC_HOOK) != tempHookSt) {
                    pLineObj->lineState.condition &= ~VP_CSLAC_HOOK;
                    pLineObj->lineState.condition |= tempHookSt;

                    /* Apply the hysteresis on the hook threshold (if available) */
                    if (pLineObj->hookHysteresis != 0) {
                        uint8 loopSupervision[VP880_LOOP_SUP_LEN];

                        VpMemCpy(loopSupervision, pLineObj->loopSup, VP880_LOOP_SUP_LEN);
                        if ((loopSupervision[VP880_LOOP_SUP_LIU_THRESH_BYTE]
                            & VP880_LOOP_SUP_LIU_THRESH_BITS) >= pLineObj->hookHysteresis) {
                            loopSupervision[VP880_LOOP_SUP_LIU_THRESH_BYTE] -=
                                pLineObj->hookHysteresis;
                        } else {
                            loopSupervision[VP880_LOOP_SUP_LIU_THRESH_BYTE] &=
                                ~VP880_LOOP_SUP_LIU_THRESH_BITS;
                        }
                        VpMpiCmdWrapper(deviceId, ecVal, VP880_LOOP_SUP_WRT,
                            VP880_LOOP_SUP_LEN, loopSupervision);
                    }

                    if ((pLineObj->status & VP880_LOW_POWER_EN) && tempHookSt
#ifdef VP880_INCLUDE_TESTLINE_CODE
                     && (pDevObj->currentTest.rdLoopTest == FALSE)
#endif
                    ){
                        VP_HOOK(VpLineCtxType, pLineCtx, ("Off-Hook Detected in Low Power Mode on line %d time %d UserState %d Current State %d Status 0x%04X",
                            channelId, pDevObj->timeStamp, pLineObj->lineState.usrCurrent, pLineObj->lineState.currentState, pLineObj->status));

                        if ((pLineObj->lineState.calType == VP_CSLAC_CAL_NONE) &&
                            (Vp880IsChnlUndrTst(pDevObj, channelId) == FALSE)) {
                            /* Force line to feed state and start leaky line detection */
                            pLineObj->lineState.currentState = VP_LINE_OHT;
                            pDevObj->stateInt &= ~((channelId == 0) ? VP880_LINE0_LP : VP880_LINE1_LP);

                            pLineObj->lineState.condition |= VP_CSLAC_LINE_LEAK_TEST;
                        }
                        break;
                    }

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
                            Vp880SetLineTone(pLineCtx, VP_PTABLE_NULL,
                                VP_PTABLE_NULL, VP_NULL);
                        }
                    }
#endif

                    if (tempHookSt) {
                        if (Vp880OffHookMgmt(pDevObj, pLineCtx, ecVal) == TRUE) {
                            retFlag = TRUE;
                        }
                    } else {
                        if (Vp880OnHookMgmt(pDevObj, pLineCtx, ecVal) == TRUE) {
                            retFlag = TRUE;
                        }
                    }
                }

                /* If the gkey conditions changed, continue line processing */
                if((pLineObj->lineState.condition & VP_CSLAC_GKEY) != tempGnkSt) {
                    VP_HOOK(VpLineCtxType, pLineCtx, ("GKEY Change to %d on Ch %d Time %d",
                        tempGnkSt, channelId, pDevObj->timeStamp));

                    if (tempGnkSt) {
                        pLineObj->lineEvents.signaling |= VP_LINE_EVID_GKEY_DET;
                        pLineObj->lineState.condition |= VP_CSLAC_GKEY;
                    } else {
                        pLineObj->lineEvents.signaling |= VP_LINE_EVID_GKEY_REL;
                        pLineObj->lineState.condition &= ~(VP_CSLAC_GKEY);
                    }
                    retFlag = TRUE;
                    pLineObj->lineEventHandle = pDevObj->timeStamp;
                }

                if((pLineObj->lineState.condition & VP_CSLAC_THERM_FLT)
                    != tempThermFault) {
                    pLineObj->lineEventHandle = pDevObj->timeStamp;
                    pLineObj->lineState.condition &= ~(VP_CSLAC_THERM_FLT);
                    pLineObj->lineState.condition |= tempThermFault;

                    pLineObj->lineEvents.faults |= VP_LINE_EVID_THERM_FLT;
                    retFlag = TRUE;

                    if (tempThermFault == VP_CSLAC_THERM_FLT) {
#ifdef VP880_INCLUDE_TESTLINE_CODE
                        if((Vp880IsChnlUndrTst(pDevObj, channelId) == TRUE)
                          || (pDevObj->currentTest.rdLoopTest == TRUE)) {
                            pLineObj->lineEvents.test |= VP_LINE_EVID_ABORT;
                        } else if (pDevObj->criticalFault.thermFltDiscEn == TRUE) {
#endif
                            Vp880SetLineState(pLineCtx, VP_LINE_DISCONNECT);
#ifdef VP880_INCLUDE_TESTLINE_CODE
                        }
#endif
                    }
                }
            }
        }
    }

    return retFlag;
}

/**
 * Vp880OffHookMgmt()
 *  This function manages the device and API behavior when an off-hook is
 * detected from the device AFTER all normal debounce timers have expired.
 *
 * Preconditions:
 *  This function is called internally by the API-II only.
 *
 * Postconditions:
 *  Device and API is updated accordingly.
 */
bool
Vp880OffHookMgmt(
    Vp880DeviceObjectType *pDevObj,
    VpLineCtxType *pLineCtx,
    uint8 ecVal)
{
    Vp880LineObjectType *pLineObj = pLineCtx->pLineObj;
    uint8 channelId = pLineObj->channelId;

    bool retFlag = FALSE;

    VpLineStateType usrState = pLineObj->lineState.usrCurrent;

#ifdef CSLAC_SEQ_EN
    VpProfilePtrType pCadence;
#endif

    VP_HOOK(VpLineCtxType, pLineCtx, ("Off-Hook on Line %d at Time %d Low Power 0x%02X",
        channelId, pDevObj->timeStamp, (pLineObj->status & VP880_LOW_POWER_EN)));

    pLineObj->dpStruct.hookSt = TRUE;
    pLineObj->dpStruct2.hookSt = TRUE;

    pLineObj->leakyLineCnt = 0;
    pLineObj->status &= ~VP880_LINE_LEAK;

    if(pLineObj->pulseMode == VP_OPTION_PULSE_DECODE_OFF) {
        pLineObj->lineEvents.signaling |= VP_LINE_EVID_HOOK_OFF;
        pLineObj->lineEventHandle = pDevObj->timeStamp;
        retFlag = TRUE;
    }

#ifdef CSLAC_SEQ_EN
    /*
     * If an off-hook is detected when the active cadence is a Message Waiting
     * Pulse on the line, restore the line state.
     */
    pCadence = pLineObj->cadence.pActiveCadence;
    if (pCadence != VP_PTABLE_NULL) {
        VpLineStateType state = pLineObj->lineState.currentState;
        if (pCadence[VP_PROFILE_TYPE_LSB] == VP_PRFWZ_PROFILE_MSG_WAIT_PULSE_INT) {
            Vp880SetLineState(pLineCtx, state);
        }
    }
#endif
    /*
     * If an off-hook is detected during the user set state of Ringing (incl.
     * ringing and silent interval) while a test is running, don't allow the api
     * to go to the ringtrip state
     */
    if(Vp880IsChnlUndrTst(pDevObj, channelId) == TRUE) {
        /* Do not change line state during test */
    } else {
        if ((usrState == VP_LINE_RINGING) || (usrState == VP_LINE_RINGING_POLREV)) {
            Vp880SetLineState(pLineCtx,  pLineObj->ringCtrl.ringTripExitSt);

            /* If ringtrip occurs (ringing then offhook), debounce the hook bit */
            pLineObj->lineTimers.timers.timer[VP_LINE_RING_EXIT_DEBOUNCE] =
                MS_TO_TICKRATE(VP880_RING_TRIP_DEBOUNCE, pDevObj->devProfileData.tickRate);
            pLineObj->lineTimers.timers.timer[VP_LINE_RING_EXIT_DEBOUNCE]
                |= VP_ACTIVATE_TIMER;
        }
    }
    return retFlag;
}

/**
 * Vp880OnHookMgmt()
 *  This function manages the device and API behavior when an on-hook is
 * detected from the device AFTER all normal debounce timers have expired.
 *
 * Preconditions:
 *  This function is called internally by the API-II only.
 *
 * Postconditions:
 *  Device and API is updated accordingly.
 */
bool
Vp880OnHookMgmt(
    Vp880DeviceObjectType *pDevObj,
    VpLineCtxType *pLineCtx,
    uint8 ecVal)
{
    Vp880LineObjectType *pLineObj = pLineCtx->pLineObj;

    bool retFlag = FALSE;
    uint8 slacState;

    VpDeviceIdType deviceId = pDevObj->deviceId;

    VP_HOOK(VpLineCtxType, pLineCtx, ("On-Hook on Line %d at Time %d",
        pLineObj->channelId, pDevObj->timeStamp));

    /* Restore the initial threshold */
    if (pLineObj->hookHysteresis != 0) {
        VpMpiCmdWrapper(deviceId, ecVal, VP880_LOOP_SUP_WRT, VP880_LOOP_SUP_LEN,
            pLineObj->loopSup);
    }

    pLineObj->dpStruct.hookSt = FALSE;
    pLineObj->dpStruct2.hookSt = FALSE;

    if(pLineObj->pulseMode == VP_OPTION_PULSE_DECODE_OFF) {
        /*
         * If this is the first time after initialization
         * that we are checking for on-hook and it is
         * on-hook, don't generate an interrupt
         */
        if (!(pLineObj->lineState.condition & VP_CSLAC_STATUS_VALID)) {
            pLineObj->lineEvents.signaling |= VP_LINE_EVID_HOOK_ON;
            pLineObj->lineEventHandle = pDevObj->timeStamp;
            retFlag = TRUE;
        }
    }
    if (((pLineObj->termType == VP_TERM_FXS_LOW_PWR) ||
         (pLineObj->termType == VP_TERM_FXS_ISOLATE_LP) ||
         (pLineObj->termType == VP_TERM_FXS_SPLITTER_LP))) {

        VP_HOOK(VpLineCtxType, pLineCtx, ("User State %d Current State %d",
            pLineObj->lineState.usrCurrent, pLineObj->lineState.currentState));

        if (pLineObj->lineState.usrCurrent == VP_LINE_STANDBY) {
            pLineObj->lineState.currentState = VP_LINE_STANDBY;
            Vp880LLSetSysState(deviceId, pLineCtx, 0x00, FALSE);
        }
    }

    VpMpiCmdWrapper(deviceId, ecVal, VP880_SYS_STATE_RD, VP880_SYS_STATE_LEN,
        &slacState);

    if (pDevObj->stateInt & VP880_IS_ABS) {
        switch(slacState & VP880_SS_LINE_FEED_MASK) {
            /*
             * Feed states where the SLIC needs to be put into high battery mode to
             * optimize feed conditions and transient response.
             */
            case (VP880_SS_ACTIVE & VP880_SS_LINE_FEED_MASK):
            case (VP880_SS_IDLE & VP880_SS_LINE_FEED_MASK):
            case (VP880_SS_ACTIVE_MID_BAT & VP880_SS_LINE_FEED_MASK):
                slacState &= ~VP880_SS_LINE_FEED_MASK;
                if(pLineObj->lineState.usrCurrent == VP_LINE_STANDBY) {
                    slacState |= VP880_SS_IDLE;
                } else {
                    slacState |= VP880_SS_ACTIVE_MID_BAT;
                }
                Vp880LLSetSysState(deviceId, pLineCtx, slacState, TRUE);
                break;

            default:
                /*
                 * Another state that either should not cause off-hook detection,
                 * or state that is handled by API-II functionality (e.g.,
                 * Ring Trip).
                 */
                break;
        }
    }

    return retFlag;
}

/**
 * Vp880LLSetSysState()
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
 */
void
Vp880LLSetSysState(
    VpDeviceIdType deviceId,
    VpLineCtxType *pLineCtx,
    uint8 lineState,
    bool writeToDevice)
{
    Vp880LineObjectType *pLineObj = pLineCtx->pLineObj;
    uint8 ecVal = pLineObj->ecVal;
    uint8 channelId = pLineObj->channelId;
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    uint8 lineStatePre[VP880_SYS_STATE_LEN];
    bool lineIsFxs = FALSE;

    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880LLSetSysState+"));

    VpMpiCmdWrapper(deviceId, ecVal, VP880_SYS_STATE_RD,
        VP880_SYS_STATE_LEN, lineStatePre);

    if (!(pLineObj->status & VP880_IS_FXO)) {
        lineIsFxs = TRUE;

        if ((Vp880IsChnlUndrTst(pDevObj, channelId) == TRUE) ||
            (pLineObj->status & VP880_LINE_IN_CAL)) {
            pDevObj->stateInt &=
                ((channelId == 0) ? ~VP880_LINE0_LP : ~VP880_LINE1_LP);
            VP_LINE_STATE(VpLineCtxType, pLineCtx,("3. Clearing LP flag for channel %d",channelId));
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

            if ((lpTermType == TRUE) && (hookStatus == FALSE) && (!(pLineObj->status & VP880_LINE_LEAK))) {
                if ((pLineObj->lineState.currentState == VP_LINE_DISCONNECT) ||
                    (pLineObj->lineState.currentState == VP_LINE_STANDBY)) {

                    if (((lineStatePre[0] & VP880_SS_LINE_FEED_MASK) != VP880_SS_FEED_BALANCED_RINGING)
                     && ((lineStatePre[0] & VP880_SS_LINE_FEED_MASK) != VP880_SS_FEED_UNBALANCED_RINGING)) {

                        pDevObj->stateInt |=
                            ((channelId == 0) ? VP880_LINE0_LP : VP880_LINE1_LP);

                        VP_LINE_STATE(VpLineCtxType, pLineCtx,("1. Setting LP flag for channel %d Hook Status %d",
                            channelId, hookStatus));
                    } else {
                        VP_LINE_STATE(VpLineCtxType, pLineCtx,("1. Delay Setting LP flag for channel %d",channelId));
                    }
                } else {
                    pDevObj->stateInt &=
                        ((channelId == 0) ? ~VP880_LINE0_LP : ~VP880_LINE1_LP);
                    VP_LINE_STATE(VpLineCtxType, pLineCtx,("1. Clearing LP flag for channel %d Device Status 0x%08lX",
                        channelId, pDevObj->stateInt));
                }
            } else {
                pDevObj->stateInt &=
                    ((channelId == 0) ? ~VP880_LINE0_LP : ~VP880_LINE1_LP);
                VP_LINE_STATE(VpLineCtxType, pLineCtx,("2. Clearing LP flag for channel %d Device Status 0x%08lX",
                    channelId, pDevObj->stateInt));
            }
        }
    }

    /* Device Write Override: setting flags without a device write */
    if (writeToDevice == FALSE) {
        VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880LLSetSysState-"));
        return;
    }

    if (((lineStatePre[0] & VP880_SS_LINE_FEED_MASK) == VP880_SS_DISCONNECT)
      && (lineIsFxs == TRUE)) {
        pLineObj->nextSlicValue = lineState;

        if ((pDevObj->staticInfo.rcnPcn[VP880_RCN_LOCATION] <= VP880_REV_VC)) {
            lineState = VP880_SS_TIP_OPEN;

            VP_LINE_STATE(VpLineCtxType, pLineCtx, ("2. Setting Chan %d to TIP_OPEN at Time %d",
                channelId, pDevObj->timeStamp));

            pLineObj->lineTimers.timers.timer[VP_LINE_PING_TIMER] =
                MS_TO_TICKRATE(VP880_PING_TIME,
                    pDevObj->devProfileData.tickRate);

            pLineObj->lineTimers.timers.timer[VP_LINE_PING_TIMER]
                |= VP_ACTIVATE_TIMER;
        }

        VP_LINE_STATE(VpLineCtxType, pLineCtx, ("2. Setting Chan %d to System State 0x%02X at Time %d",
            channelId, lineState, pDevObj->timeStamp));

        Vp880UpdateBufferChanSel(pDevObj, channelId, lineState);
        if ((lineState == VP880_SS_BALANCED_RINGING) && (IN_RUSH_CYCLE_TOTAL > 0) &&
            (pDevObj->devProfileData.peakManagement == TRUE)) {
            Vp880LimitInRushCurrent(pDevObj, ecVal, FALSE);
        } else {
            VpMpiCmdWrapper(deviceId, ecVal, VP880_SYS_STATE_WRT,
                VP880_SYS_STATE_LEN, &lineState);
        }
    } else {
        if (pLineObj->lineTimers.timers.timer[VP_LINE_PING_TIMER] & VP_ACTIVATE_TIMER) {
            pLineObj->nextSlicValue = lineState;
        } else {
            VP_LINE_STATE(VpLineCtxType, pLineCtx, ("1. Setting Chan %d to System State 0x%02X at Time %d",
                channelId, lineState, pDevObj->timeStamp));

            Vp880UpdateBufferChanSel(pDevObj, channelId, lineState);
            if ((lineState == VP880_SS_BALANCED_RINGING) && (IN_RUSH_CYCLE_TOTAL > 0) &&
                (pDevObj->devProfileData.peakManagement == TRUE)) {
                Vp880LimitInRushCurrent(pDevObj, ecVal, FALSE);
            } else {
                VpMpiCmdWrapper(deviceId, ecVal, VP880_SYS_STATE_WRT,
                    VP880_SYS_STATE_LEN, &lineState);
            }
        }
    }
    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880LLSetSysState-"));
}

/**
 * Vp880LowPowerMode()
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
Vp880LowPowerMode(
    VpDevCtxType *pDevCtx)
{
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpDeviceIdType deviceId = pDevObj->deviceId;

    VpLineCtxType *pLineCtx;
    bool isValidCtx[VP880_MAX_NUM_CHANNELS] = {FALSE, FALSE};
    Vp880LineObjectType *pLineObj;
    bool lowPower;

    uint8 maxChannels = pDevObj->staticInfo.maxChannels;
    uint8 channelId;
    uint8 ecVal[] = {VP880_EC_CH1, VP880_EC_CH2};

#ifdef VP880_INCLUDE_TESTLINE_CODE
    /* We don't want to interact with the line in this state */
    if (pDevObj->currentTest.rdLoopTest == TRUE) {
        return;
    }
#endif

    /* Don't do anything if device is in calibration */
    if (pDevObj->status.state & VP_DEV_IN_CAL) {
        return;
    }

    /*
     * Low Power is only possible if both lines can use low power mode.
     * Otherwise, exit.
     */
    if ((pDevObj->stateInt & (VP880_LINE0_LP | VP880_LINE1_LP)) !=
        (VP880_LINE0_LP | VP880_LINE1_LP)) {
        lowPower = FALSE;
    } else {
        lowPower = TRUE;
    }

    /*
     * Determine which lines are valid in case we have to adjust their line
     * states. Consider "valid" only those lines that are FXS Low Power type.
     */
    for (channelId = 0; channelId < maxChannels; channelId++) {
        if (pDevCtx->pLineCtx[channelId] != VP_NULL) {
            pLineCtx = pDevCtx->pLineCtx[channelId];
            pLineObj = pLineCtx->pLineObj;
            ecVal[channelId] = pLineObj->ecVal;

            if ((!(pLineObj->status & VP880_IS_FXO)) &&
                ((pLineObj->termType == VP_TERM_FXS_LOW_PWR) ||
                 (pLineObj->termType == VP_TERM_FXS_ISOLATE_LP) ||
                 (pLineObj->termType == VP_TERM_FXS_SPLITTER_LP))) {
                bool hookStatus = FALSE;
                isValidCtx[channelId] = TRUE;
                VpCSLACGetLineStatus(pLineCtx, VP_INPUT_RAW_HOOK, &hookStatus);

                if ((Vp880IsChnlUndrTst(pDevObj, channelId) == TRUE) ||
                    (pLineObj->status & VP880_LINE_LEAK) ||
                    (hookStatus == TRUE)) {
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
                ecVal[channelId] = pLineObj->ecVal;

                if (pLineObj->status & VP880_LOW_POWER_EN) {
                    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880LowPowerMode+"));

                    VP_LINE_STATE(VpLineCtxType, pLineCtx, ("Exit LPM for Line %d Device Status 0x%08lX Line Status 0x%04X",
                        channelId, pDevObj->stateInt, pLineObj->status));

                    VpMpiCmdWrapper(deviceId, ecVal[0], VP880_REGULATOR_PARAM_WRT,
                        VP880_REGULATOR_PARAM_LEN, pDevObj->swParams);

                    Vp880SetLP(FALSE, pLineCtx);

                    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880LowPowerMode-"));
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
                ecVal[channelId] = pLineObj->ecVal;

                if (!(pLineObj->status & VP880_LOW_POWER_EN)) {
                    uint8 swParams[VP880_REGULATOR_PARAM_LEN];

                    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880LowPowerMode+"));

                    VP_LINE_STATE(VpLineCtxType, pLineCtx, ("Enter LPM for Line %d Device Status 0x%08lX Line Status 0x%04X",
                        channelId, pDevObj->stateInt, pLineObj->status));

                    VpMpiCmdWrapper(deviceId, ecVal[0], VP880_REGULATOR_PARAM_RD,
                        VP880_REGULATOR_PARAM_LEN, swParams);
                    swParams[VP880_FLOOR_VOLTAGE_BYTE] &= ~VP880_FLOOR_VOLTAGE_MASK;
                    swParams[VP880_FLOOR_VOLTAGE_BYTE] |= 0x0D;   /* 70V */

                    VpMpiCmdWrapper(deviceId, ecVal[0], VP880_REGULATOR_PARAM_WRT,
                        VP880_REGULATOR_PARAM_LEN, swParams);

                    Vp880SetLP(TRUE, pLineCtx);
                    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880LowPowerMode-"));
                }
            }
        }
    }
}

/**
 * Vp880SetLP()
 *  This function modifies the line/device to enter/exit LPM.
 *
 * Preconditions:
 *
 * Postconditions:
 */
void
Vp880SetLP(
    bool lpMode,
    VpLineCtxType *pLineCtx)
{
    Vp880LineObjectType *pLineObj = pLineCtx->pLineObj;
    uint8 ecVal = pLineObj->ecVal;

    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpDeviceIdType deviceId = pDevObj->deviceId;

    uint8 lineState[VP880_SYS_STATE_LEN];

    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880SetLP+"));

    /*
     * Need timer here to allow switcher to stabilize whether entering or
     * exiting LPM.
     */
    pDevObj->devTimer[VP_DEV_TIMER_LP_CHANGE] =
        (MS_TO_TICKRATE(VP880_PWR_SWITCH_DEBOUNCE, pDevObj->devProfileData.tickRate))
        | VP_ACTIVATE_TIMER;

    if (lpMode == FALSE) {
        VP_LINE_STATE(VpLineCtxType, pLineCtx, ("Taking Channel %d out of Low Power Mode at time %d User State %d",
            pLineObj->channelId, pDevObj->timeStamp, pLineObj->lineState.usrCurrent));

        pLineObj->status &= ~VP880_LOW_POWER_EN;

        if (pLineObj->lineState.usrCurrent != VP_LINE_DISCONNECT) {
            bool internalApiOperation = FALSE;

            Vp880SetLPRegisters(pLineObj, FALSE);

#ifdef CSLAC_SEQ_EN
            if ((pLineObj->cadence.status & (VP_CADENCE_STATUS_ACTIVE | VP_CADENCE_STATUS_SENDSIG)) ==
                (VP_CADENCE_STATUS_ACTIVE | VP_CADENCE_STATUS_SENDSIG)) {
                internalApiOperation = TRUE;
            }
#endif

            if ((pLineObj->lineState.usrCurrent == VP_LINE_STANDBY)
             && (internalApiOperation == FALSE)) {
                lineState[0] = VP880_SS_ACTIVE;
                Vp880WriteLPExitRegisters(pLineCtx, deviceId, ecVal,
                    lineState);
                pLineObj->nextSlicValue = VP880_SS_IDLE;
            } else {
                Vp880WriteLPExitRegisters(pLineCtx, deviceId, ecVal,
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

        pLineObj->status |= VP880_LOW_POWER_EN;

        if (pLineObj->lineState.usrCurrent != VP_LINE_DISCONNECT) {
            /* Set timer to wait before making final changes. When this timer
             * expires, the following sequence is run:
             *
             *  - Set SLIC state to Disconnect
             *  - Write the following:
             *      icr2[VP880_ICR2_VOC_DAC_INDEX] |= VP880_ICR2_ILA_DAC;
             *      icr2[VP880_ICR2_VOC_DAC_INDEX] &= ~VP880_ICR2_VOC_DAC_SENSE;
             *      icr2[VP880_ICR2_VOC_DAC_INDEX+1] &= ~VP880_ICR2_ILA_DAC;
             */
            pLineObj->lineTimers.timers.timer[VP_LINE_TRACKER_DISABLE] =
                (MS_TO_TICKRATE(VP880_TRACKER_DISABLE_TIME,
                    pDevObj->devProfileData.tickRate)) | VP_ACTIVATE_TIMER;

            /*
             * We are entering LPM into VP_LINE_STANDBY. So the required ICR
             * values are not yet set in the line object (as they would be if
             * entering from VP_LINE_DISCONNECT).
             */
            lineState[0] = VP880_SS_DISCONNECT;

            Vp880SetLPRegisters(pLineObj, TRUE);
        } else {
            uint16 dischargeTime = Vp880SetDiscTimers(pDevObj);

            /*
             * We are entering LPM into VP_LINE_DISCONNECT. So the required ICR
             * values have been set in the line object, just need to force the
             * required sequence (Active w/polarity, then Disconnect).
             */
            lineState[0] = VP880_SS_ACTIVE;
            pLineObj->status &= ~VP880_HAL_DELAY;
            pLineObj->nextSlicValue = VP880_SS_DISCONNECT;

            /* Set Discharge Time based on Supply Configuration. */
            if (dischargeTime < VP880_TRACKER_DISABLE_TIME) {
                dischargeTime = VP880_TRACKER_DISABLE_TIME;
            }

            /* Set timer to wait before making final changes. When this timer
             * expires, the following sequence is run:
             *
             *  - Set SLIC state to Disconnect
             *  - Write the following:
             *      icr2[VP880_ICR2_VOC_DAC_INDEX] |= VP880_ICR2_ILA_DAC;
             *      icr2[VP880_ICR2_VOC_DAC_INDEX] &= ~VP880_ICR2_VOC_DAC_SENSE;
             *      icr2[VP880_ICR2_VOC_DAC_INDEX+1] &= ~VP880_ICR2_ILA_DAC;
             */
            pLineObj->lineTimers.timers.timer[VP_LINE_DISCONNECT_EXIT] =
                (MS_TO_TICKRATE(dischargeTime, pDevObj->devProfileData.tickRate))
                | VP_ACTIVATE_TIMER;
        }

        Vp880WriteLPEnterRegisters(pLineCtx, deviceId, ecVal,
            lineState);
    }
    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880SetLP-"));
}

/**
 * Vp880WriteLPExitRegisters()
 *  This function writes the ICR and State values to the device for LPM exit.
 *
 * Preconditions:
 *  None. Modification of line object data only.
 *
 * Postconditions:
 *  The device registers have been modified.
 */
void
Vp880WriteLPExitRegisters(
    VpLineCtxType *pLineCtx,
    VpDeviceIdType deviceId,
    uint8 ecVal,
    uint8 *lineState)
{
    Vp880LineObjectType *pLineObj = pLineCtx->pLineObj;

    VpMpiCmdWrapper(deviceId, ecVal, VP880_ICR3_WRT, VP880_ICR3_LEN,
        pLineObj->icr3Values);

    VpMpiCmdWrapper(deviceId, ecVal, VP880_ICR4_WRT, VP880_ICR4_LEN,
        pLineObj->icr4Values);

    if (lineState == VP_NULL) {
        VP_LINE_STATE(VpLineCtxType, pLineCtx, ("LP Exit: Writing LineState %d on Ch %d",
            pLineObj->lineState.currentState, pLineObj->channelId));

        Vp880SetLineStateInt(pLineCtx, pLineObj->lineState.currentState);
    } else {
        VP_LINE_STATE(VpLineCtxType, pLineCtx, ("LP Exit: Writing SLIC State 0x%02X on Ch %d",
            lineState[0], pLineObj->channelId));

        Vp880UpdateBufferChanSel(pLineCtx->pDevCtx->pDevObj, pLineObj->channelId, lineState[0]);
        VpMpiCmdWrapper(deviceId, ecVal, VP880_SYS_STATE_WRT, VP880_SYS_STATE_LEN,
            lineState);
    }

    Vp880ProtectedWriteICR1(pLineObj, deviceId, pLineObj->icr1Values);

    VpMpiCmdWrapper(deviceId, ecVal, VP880_ICR2_WRT, VP880_ICR2_LEN,
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
 * Vp880WriteLPEnterRegisters()
 *  This function writes the ICR and State values to the device for LPM enter.
 *
 * Preconditions:
 *  None. Modification of line object data only.
 *
 * Postconditions:
 *  The device registers have been modified.
 */
void
Vp880WriteLPEnterRegisters(
    VpLineCtxType *pLineCtx,
    VpDeviceIdType deviceId,
    uint8 ecVal,
    uint8 *lineState)
{
    Vp880LineObjectType *pLineObj = pLineCtx->pLineObj;

    Vp880ProtectedWriteICR1(pLineObj, deviceId, pLineObj->icr1Values);

    VpMpiCmdWrapper(deviceId, ecVal, VP880_ICR2_WRT, VP880_ICR2_LEN,
        pLineObj->icr2Values);

    Vp880UpdateBufferChanSel(pLineCtx->pDevCtx->pDevObj, pLineObj->channelId, lineState[0]);
    VpMpiCmdWrapper(deviceId, ecVal, VP880_SYS_STATE_WRT, VP880_SYS_STATE_LEN,
        lineState);

    VpMpiCmdWrapper(deviceId, ecVal, VP880_ICR3_WRT, VP880_ICR3_LEN,
        pLineObj->icr3Values);

    VpMpiCmdWrapper(deviceId, ecVal, VP880_ICR4_WRT, VP880_ICR4_LEN,
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
 * Vp880SetLPRegisters()
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
Vp880SetLPRegisters(
    Vp880LineObjectType *pLineObj,
    bool lpModeTo)
{
    VP_API_FUNC_INT(None, VP_NULL, ("Vp880SetLPRegisters+"));

    if (lpModeTo == TRUE) {
        pLineObj->icr1Values[VP880_ICR1_BIAS_OVERRIDE_LOCATION] =
            VP880_ICR1_LINE_BIAS_OVERRIDE;
        pLineObj->icr1Values[VP880_ICR1_BIAS_OVERRIDE_LOCATION+1] = 0x08;

        pLineObj->icr1Values[VP880_ICR1_RING_AND_DAC_LOCATION] &=
            ~VP880_ICR1_RING_BIAS_DAC_MASK;
        pLineObj->icr1Values[VP880_ICR1_RING_AND_DAC_LOCATION+1] &=
            ~VP880_ICR1_RING_BIAS_DAC_MASK;

        pLineObj->icr2Values[VP880_ICR2_SENSE_INDEX] =
            (VP880_ICR2_DAC_SENSE | VP880_ICR2_FEED_SENSE
           | VP880_ICR2_TIP_SENSE | VP880_ICR2_RING_SENSE);

        pLineObj->icr2Values[VP880_ICR2_SENSE_INDEX+1] =
            (VP880_ICR2_FEED_SENSE
           | VP880_ICR2_TIP_SENSE | VP880_ICR2_RING_SENSE);

        pLineObj->icr2Values[VP880_ICR2_SWY_CTRL_INDEX] =
            (VP880_ICR2_SWY_LIM_CTRL1 | VP880_ICR2_SWY_LIM_CTRL);
        pLineObj->icr2Values[VP880_ICR2_SWY_CTRL_INDEX+1] =
            (VP880_ICR2_SWY_LIM_CTRL1 | VP880_ICR2_SWY_LIM_CTRL);

        pLineObj->icr3Values[VP880_ICR3_LINE_CTRL_INDEX] |= VP880_ICR3_LINE_CTRL;
        pLineObj->icr3Values[VP880_ICR3_LINE_CTRL_INDEX+1] |= VP880_ICR3_LINE_CTRL;

        pLineObj->icr4Values[VP880_ICR4_SUP_INDEX] |=
            (VP880_ICR4_SUP_DAC_CTRL | VP880_ICR4_SUP_DET_CTRL
            | VP880_ICR4_SUP_POL_CTRL);
        pLineObj->icr4Values[VP880_ICR4_SUP_INDEX+1] |=
            (VP880_ICR4_SUP_DAC_CTRL | VP880_ICR4_SUP_DET_CTRL
            | VP880_ICR4_SUP_POL_CTRL);

    } else {
        pLineObj->icr3Values[VP880_ICR3_LINE_CTRL_INDEX] &= ~VP880_ICR3_LINE_CTRL;

        pLineObj->icr4Values[VP880_ICR4_SUP_INDEX] &=
            ~(VP880_ICR4_SUP_DAC_CTRL | VP880_ICR4_SUP_DET_CTRL
            | VP880_ICR4_SUP_POL_CTRL);

        /* Remove previously set SW control of ICR1 */
        pLineObj->icr1Values[VP880_ICR1_BIAS_OVERRIDE_LOCATION] &=
            ~VP880_ICR1_LINE_BIAS_OVERRIDE;

        pLineObj->icr2Values[VP880_ICR2_SENSE_INDEX] &=
            ~(VP880_ICR2_RING_SENSE | VP880_ICR2_TIP_SENSE |
              VP880_ICR2_DAC_SENSE | VP880_ICR2_FEED_SENSE);
    }

    VP_API_FUNC_INT(None, VP_NULL, ("Vp880SetLPRegisters-"));
}

#ifdef LEGACY_RINGING_DETECTION
/**
 * Vp880LowRingFreqDetect()
 *  This function should only be called by Vp880ProcessFxoLine. It performs
 * Ringing and PolRev detection on lines configured to detect ringing
 * frequencies below what the device can support.
 *
 * Preconditions:
 *  Conditions defined by purpose of Vp880ProcessFxoLine.
 *
 * Postconditions:
 *  The Api variables and events (as appropriate) for the line passed have been
 * updated. Return Flag passed is set to TRUE if an event is pending.
 */
void
Vp880LowRingFreqDetect(
    VpLineCtxType *pLineCtx,
    VpCslacLineCondType *tempRingingSt,
    VpCslacLineCondType *tempPolRevSt,
    bool *retFlag)
{
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880LineObjectType *pLineObj = pLineCtx->pLineObj;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    uint8 channelId = pLineObj->channelId;
    uint16 polRevPeriod;

    if (*tempPolRevSt !=
        (pLineObj->lineState.condition & VP_CSLAC_POLREV)) {
        pLineObj->lineState.condition &= ~VP_CSLAC_POLREV;
        pLineObj->lineState.condition |= *tempPolRevSt;
        pLineObj->lineState.condition &= ~VP_CSLAC_POLREV_REPORTED;

        /*
         * Capture the period of the last two pol revs. Used for
         * Ringing Detection
         */
        pLineObj->lineTimers.timers.fxoTimer.prevHighToLowTime =
            ((pLineObj->lineTimers.timers.fxoTimer.timePrevPolRev
            + pLineObj->lineTimers.timers.fxoTimer.timeLastPolRev)
            / 4);

        pLineObj->lineTimers.timers.fxoTimer.timePrevPolRev =
            pLineObj->lineTimers.timers.fxoTimer.timeLastPolRev;
        pLineObj->lineTimers.timers.fxoTimer.timeLastPolRev = 0;
    }

    if (pDevObj->intReg[channelId] & VP880_LIU1_MASK) {
        pLineObj->lineState.condition |= VP_CSLAC_LIU;
    } else {
        pLineObj->lineState.condition &= ~VP_CSLAC_LIU;
        pLineObj->lineTimers.timers.fxoTimer.lastNotLiu =
            pLineObj->ringDetMax * 2;
    }

    polRevPeriod =
        pLineObj->lineTimers.timers.fxoTimer.prevHighToLowTime;

    if ((pLineObj->lineTimers.timers.fxoTimer.timePrevPolRev / 4)
        < pLineObj->ringDetMax) {
        if (pLineObj->lineState.condition & VP_CSLAC_POLREV) {
            pLineObj->fxoData = VP_POLREV_REVERSE;
        } else {
            pLineObj->fxoData = VP_POLREV_NORMAL;
        }
    }

    /* Evaluate the detectors */
    /*
     * If the LIU Threshold has been exceeded, it's definitely not
     * PolRev, but may be ringing. If it has been completely
     * debounced, then Ringing is removed if we previously had
     * Ringing.
     */
    if (pLineObj->lineTimers.timers.fxoTimer.lastNotLiu) {
        /*
         * The threshold has been exceeded for a period within the
         * debounce interval. Check on Ringing condition.
         */

        if ((pLineObj->lineTimers.timers.fxoTimer.timeLastPolRev / 4)
            > pLineObj->ringDetMax) {
            /*
             * This occurs because we had a recent LIU threshold,
             * but the frequency is not correct. No action other
             * than clearing Ringing state is necessary.
             */
            *tempRingingSt = 0;
            if (pLineObj->lineState.condition & VP_CSLAC_POLREV) {
                pLineObj->fxoData = VP_POLREV_REVERSE;
            } else {
                pLineObj->fxoData = VP_POLREV_NORMAL;
            }
        } else if ((polRevPeriod <= pLineObj->ringDetMax)
                && (polRevPeriod >= pLineObj->ringDetMin)) {
            *tempRingingSt = VP_CSLAC_RINGING;
        } else {
            /*
             * This prevents compiler warning because it forces
             * the value to an initialized state
             */
            *tempRingingSt =
                (pLineObj->lineState.condition & VP_CSLAC_RINGING);
        }
    } else {
        *tempRingingSt = 0;

        /* We were not ringing, so process for polrev event */
        if (!(pLineObj->lineState.condition & VP_CSLAC_RINGING)) {
            /*
             * Require a 5ms delay (plus LIU 2ms the debounce time)
             * from previous polrev to occur before allowing it to
             * be detected as "Not Ringing". This gives time for
             * most ringing signals to exceed the LIU threshold.
             */
            if ((pLineObj->lineTimers.timers.fxoTimer.timeLastPolRev / 4) >= 5) {
                if (!(pLineObj->lineState.condition & VP_CSLAC_POLREV_REPORTED)) {
                    pLineObj->lineState.condition |= VP_CSLAC_POLREV_REPORTED;

                    /*
                     * Based on how Ringing behaves, we could get out
                     * of sync w.r.t., PolRev. So don't report an event
                     * unless the PolRev changed.
                     */
                    if (pLineObj->lineState.condition & VP_CSLAC_POLREV) {
                        if (pLineObj->fxoData != VP_POLREV_REVERSE) {
                            pLineObj->fxoData = VP_POLREV_REVERSE;
                            pLineObj->lineEventHandle = pDevObj->timeStamp;
                            pLineObj->lineEvents.fxo |= VP_LINE_EVID_POLREV;
                            *retFlag = TRUE;
                            pLineObj->preRingPolRev = VP_POLREV_REVERSE;
                        }
                    } else {
                        if (pLineObj->fxoData != VP_POLREV_NORMAL) {
                            pLineObj->fxoData = VP_POLREV_NORMAL;
                            pLineObj->lineEventHandle = pDevObj->timeStamp;
                            pLineObj->lineEvents.fxo |= VP_LINE_EVID_POLREV;
                            pLineObj->preRingPolRev = VP_POLREV_NORMAL;
                            *retFlag = TRUE;
                        }
                    }
                }
            }
        }
    }
}

/**
 * Vp880ProcessFxoLine()
 *  This function should only be called by Vp880ServiceInterrupts on FXO lines.
 * It performs all line processing for operations that are Tick based
 *
 * Preconditions:
 *  Conditions defined by purpose of Api Tick.
 *
 * Postconditions:
 *  The Api variables and events (as appropriate) for the line passed have been
 * updated.
 */
bool
Vp880ProcessFxoLine(
    Vp880DeviceObjectType *pDevObj,
    VpLineCtxType *pLineCtx)
{
    Vp880LineObjectType *pLineObj = pLineCtx->pLineObj;
    bool retFlag = FALSE;
    VpCslacLineCondType tempRingingSt, tempDiscSt, tempPolRevSt, tempLiu;
    uint8 channelId = pLineObj->channelId, discMask = 0;
    VpLineStateType currentState = pLineObj->lineState.currentState;

    /*
     * Ignore the detector for a period after the last hook state change, or
     * a longer period after the last hook state change AND if the previous
     * line condition was Ringing
     */
    if ((pLineObj->lineTimers.timers.fxoTimer.lastStateChange
         < VP_FXO_STATE_CHANGE_DEBOUNCE) ||
        ((pLineObj->lineTimers.timers.fxoTimer.lastStateChange
         < VP_FXO_RING_TRIP_DEBOUNCE)
      && (pLineObj->lineState.condition & VP_CSLAC_RINGING))
#ifdef CSLAC_SEQ_EN
      || ((pLineObj->cadence.status & VP_CADENCE_STATUS_ACTIVE)
       && (pLineObj->intSequence[VP_PROFILE_TYPE_LSB]
            == VP_PRFWZ_PROFILE_MOMENTARY_LOOP_OPEN_INT))
#endif
            ) {
        /* Middle of detector Mask. Skip this process */
    } else {
        if ((pLineObj->termType == VP_TERM_FXO_DISC)
        && ((currentState == VP_LINE_FXO_TALK)
         || (currentState == VP_LINE_FXO_LOOP_CLOSE))) {
            discMask = pDevObj->intReg[channelId] & VP880_IO2_1_MASK;
        } else {
            discMask = pDevObj->intReg[channelId] & VP880_DISC1_MASK;
        }

        if (discMask) {
            tempDiscSt = VP_CSLAC_DISC;
        } else {
            tempDiscSt = 0;
        }

        if (pDevObj->intReg[channelId] & VP880_POL1_MASK) {
            tempPolRevSt = VP_CSLAC_POLREV;
        } else {
            tempPolRevSt = 0;
        }

        if (pLineObj->ringDetMax >= VP880_MAX_RING_DET_PERIOD) {
            Vp880LowRingFreqDetect(pLineCtx, &tempRingingSt, &tempPolRevSt,
                &retFlag);
        } else {
            if (pDevObj->intReg[channelId] & VP880_LIU1_MASK) {
                tempLiu = VP_CSLAC_LIU;
            } else {
                tempLiu = 0;
            }

            if (tempLiu != (pLineObj->lineState.condition & VP_CSLAC_LIU)) {
                pLineObj->lineState.condition &= ~VP_CSLAC_LIU;
                pLineObj->lineState.condition |= tempLiu;

                pLineObj->lineEventHandle = pDevObj->timeStamp;
                pLineObj->lineEvents.fxo |=
                    ((tempLiu) ? VP_LINE_EVID_LIU : VP_LINE_EVID_LNIU);
                retFlag = TRUE;
            }

            if (pDevObj->intReg[channelId] & VP880_RING1_DET_MASK) {
                tempRingingSt = VP_CSLAC_RINGING;
            } else {
                tempRingingSt = 0;
            }

            if (tempPolRevSt !=
                (pLineObj->lineState.condition & VP_CSLAC_POLREV)) {
                pLineObj->lineState.condition &= ~VP_CSLAC_POLREV;
                pLineObj->lineState.condition |= tempPolRevSt;
                pLineObj->lineState.condition &= ~VP_CSLAC_POLREV_REPORTED;

                if ((tempRingingSt != VP_CSLAC_RINGING)
                 && ((pLineObj->lineTimers.timers.fxoTimer.timeLastPolRev / 4)
                    > pLineObj->ringDetMax)) {
                    pLineObj->lineEventHandle = pDevObj->timeStamp;
                    pLineObj->lineEvents.fxo |= VP_LINE_EVID_POLREV;
                    retFlag = TRUE;
                    pLineObj->preRingPolRev =
                        (pLineObj->lineState.condition & VP_CSLAC_POLREV) ?
                        VP_POLREV_REVERSE : VP_POLREV_NORMAL;
                }

                pLineObj->lineTimers.timers.fxoTimer.timeLastPolRev = 0;

                if (pLineObj->lineState.condition & VP_CSLAC_POLREV) {
                    pLineObj->fxoData = VP_POLREV_REVERSE;
                } else {
                    pLineObj->fxoData = VP_POLREV_NORMAL;
                }
            }
        }

        /*
         * Our cached state is inconsistent with recently detected conditions.
         * Generate the event.
         */
        if ((pLineObj->lineState.condition & VP_CSLAC_RINGING) != tempRingingSt) {
            pLineObj->lineEventHandle = pDevObj->timeStamp;

            if (tempRingingSt) {
                pLineObj->lineEvents.fxo |= VP_LINE_EVID_RING_ON;
                pLineObj->lineState.condition |= VP_CSLAC_RINGING;
            } else {
                pLineObj->lineEvents.fxo |= VP_LINE_EVID_RING_OFF;
                pLineObj->lineState.condition &= ~(VP_CSLAC_RINGING);

                if (pLineObj->preRingPolRev != pLineObj->fxoData) {
                    pLineObj->preRingPolRev = pLineObj->fxoData;
                    pLineObj->lineEvents.fxo |= VP_LINE_EVID_POLREV;
                }
            }
            retFlag = TRUE;
        }

         /* If the feed conditions changed, continue line processing */
        if((pLineObj->lineState.condition & VP_CSLAC_DISC) != tempDiscSt) {
            /*
             * Update actual line condition, even if not reporting an
             * event
             */
            if (tempDiscSt) {
                pLineObj->lineState.condition |= VP_CSLAC_DISC;
            } else {
                pLineObj->lineState.condition &= ~(VP_CSLAC_DISC);
            }

            /*
             * If line has been stable, update the pre-Disconnect value that
             * is used at end of timer to determine if event should be
             * generated.
             */
            if (pLineObj->lineTimers.timers.fxoTimer.disconnectDebounce == 0) {
                pLineObj->preDisconnect = tempDiscSt;
            }

            /* Line status changed. Therefore, reset timer */
            pLineObj->lineTimers.timers.fxoTimer.disconnectDebounce =
                VP_FXO_DISCONNECT_DEBOUNCE;

            /*
             * Immediate disconnect changes detected do not result in
             * API-II event. So retFlag remains as set previously.
             */
        } else {
            /*
             * If the disconnect signal came back to the current state, stop
             * the debounce count
             */
            if (pLineObj->termType == VP_TERM_FXO_DISC) {
                pLineObj->lineTimers.timers.fxoTimer.noCount = TRUE;
                pLineObj->lineTimers.timers.fxoTimer.fxoDiscIO2Change = 0;
            }
        }
    } /* end interrupt detect */
    return retFlag;
} /* end Vp880ProcessFxoLine */
/* end of LEGACY_RINGING_DETECTION */
#else

/**
 * Vp880ProcessFxoLine()
 *  This function should only be called by Vp880ServiceInterrupts on FXO lines.
 * It performs all line processing for operations that are Tick based
 *
 * Preconditions:
 *  Conditions defined by purpose of Api Tick.
 *
 * Postconditions:
 *  The Api variables and events (as appropriate) for the line passed have been
 * updated.
 */
bool
Vp880ProcessFxoLine(
    Vp880DeviceObjectType *pDevObj,
    VpLineCtxType *pLineCtx)
{
    Vp880LineObjectType *pLineObj = pLineCtx->pLineObj;
    bool retFlag = FALSE;
    VpCslacLineCondType tempRingingSt, tempDiscSt, tempPolRevSt, tempLiu;
    uint8 channelId = pLineObj->channelId, discMask = 0;
    VpLineStateType currentState = pLineObj->lineState.currentState;
    VpFXOTimerType *pFxoTimers = &pLineObj->lineTimers.timers.fxoTimer;

    /*
     * Ignore the detector for a period after the last hook state change, or
     * a longer period after the last hook state change AND if the previous
     * line condition was Ringing
     */
    if ((pFxoTimers->lastStateChange < VP_FXO_STATE_CHANGE_DEBOUNCE) ||
        ((pFxoTimers->lastStateChange < VP_FXO_RING_TRIP_DEBOUNCE)
      && (pLineObj->lineState.condition & VP_CSLAC_RINGING))
#ifdef CSLAC_SEQ_EN
      || ((pLineObj->cadence.status & VP_CADENCE_STATUS_ACTIVE)
       && (pLineObj->intSequence[VP_PROFILE_TYPE_LSB]
            == VP_PRFWZ_PROFILE_MOMENTARY_LOOP_OPEN_INT))
#endif
            ) {
        /* Middle of detector Mask. Skip this process */
    } else {
        if ((pLineObj->termType == VP_TERM_FXO_DISC)
        && ((currentState == VP_LINE_FXO_TALK)
         || (currentState == VP_LINE_FXO_LOOP_CLOSE))) {
            discMask = pDevObj->intReg[channelId] & VP880_IO2_1_MASK;
        } else {
            discMask = pDevObj->intReg[channelId] & VP880_DISC1_MASK;
        }

        tempDiscSt = (discMask) ? VP_CSLAC_RAW_DISC : 0;

        tempPolRevSt = (pDevObj->intReg[channelId] & VP880_POL1_MASK) ? VP_CSLAC_POLREV : 0;
        tempRingingSt = (pDevObj->intReg[channelId] & VP880_RING1_DET_MASK) ? VP_CSLAC_RINGING : 0;

        /*
         * Ringing Detector is unstable in presence of Disconnect, and the
         * Disconnect is filtered for generally 28-40ms. So if currently
         * detecting Disconnect or just "recently" stopped detecting Disconnect,
         * prevent Ringing events and Ring_On status.
         */
        if ((pLineObj->lineState.condition & (VP_CSLAC_LIU | VP_CSLAC_RAW_DISC | VP_CSLAC_DISC)) ||
            (pFxoTimers->disconnectDebounce > 0)) {
            tempRingingSt = 0;
        }

        if ((currentState == VP_LINE_FXO_TALK)
         || (currentState == VP_LINE_FXO_LOOP_CLOSE)) {
            tempLiu = (pLineObj->lineState.condition & VP_CSLAC_LIU);
        } else {
            tempLiu = (pDevObj->intReg[channelId] & VP880_LIU1_MASK)
                ? VP_CSLAC_LIU : 0;
        }

        /*
         * Multiple PolRevs can/will be detected prior to ringing but not after
         * (ringing "off" occurs when the line is stable). So we have to know
         * what the polarity was if a PolRev event was generated prior to
         * ringing, so we generate a corresponding correction event post ringing.
         */
        if((pLineObj->lineState.condition & VP_CSLAC_RINGING) != tempRingingSt) {
            pLineObj->lineEventHandle = pDevObj->timeStamp;
            if (tempRingingSt) {
                pLineObj->lineEvents.fxo |= VP_LINE_EVID_RING_ON;
                pLineObj->lineState.condition |= VP_CSLAC_RINGING;
            } else {
                pLineObj->lineEvents.fxo |= VP_LINE_EVID_RING_OFF;
                pLineObj->lineState.condition &= ~(VP_CSLAC_RINGING);

                VP_FXO_FUNC(VpLineCtxType, pLineCtx, ("Ringing Stop. Pre-Ring PolRev %d Current %d",
                    pLineObj->preRingPolRev, tempPolRevSt));

                if (((pLineObj->preRingPolRev == VP_POLREV_NORMAL) && (tempPolRevSt))
                || ((pLineObj->preRingPolRev == VP_POLREV_REVERSE) && (!(tempPolRevSt)))) {
                    /*
                     * PolRev is out of sync due to transitions prior to ringing
                     * detect. Correct it.
                     */
                    tempPolRevSt = (tempPolRevSt) ? 0 : VP_CSLAC_POLREV;
                }
            }
            retFlag = TRUE;
        }

        if (pLineObj->lineState.condition & VP_CSLAC_RINGING) {
            /* Not PolRev events if ringing is detected. */
            tempPolRevSt = (pLineObj->lineState.condition & VP_CSLAC_POLREV);
        }

        if ((tempPolRevSt != (pLineObj->lineState.condition & VP_CSLAC_POLREV))) {
            pLineObj->lineState.condition &= ~VP_CSLAC_POLREV;
            pLineObj->lineState.condition |= tempPolRevSt;
            pLineObj->lineState.condition &= ~VP_CSLAC_POLREV_REPORTED;

            if ((pFxoTimers->timeLastPolRev / 4) > pLineObj->ringDetMax) {
                pLineObj->lineEventHandle = pDevObj->timeStamp;
                pLineObj->lineEvents.fxo |= VP_LINE_EVID_POLREV;
                retFlag = TRUE;
                pLineObj->preRingPolRev =
                    (pLineObj->lineState.condition & VP_CSLAC_POLREV) ?
                    VP_POLREV_REVERSE : VP_POLREV_NORMAL;
            }

            pFxoTimers->timeLastPolRev = 0;

            if (pLineObj->lineState.condition & VP_CSLAC_POLREV) {
                pLineObj->fxoData = VP_POLREV_REVERSE;
            } else {
                pLineObj->fxoData = VP_POLREV_NORMAL;
            }
        }

        if (tempLiu != (pLineObj->lineState.condition & VP_CSLAC_LIU)) {
            VP_FXO_FUNC(VpLineCtxType, pLineCtx, ("LIU Change to %d at time %d",
                tempLiu, pDevObj->timeStamp));

            pLineObj->lineState.condition &= ~VP_CSLAC_LIU;
            pLineObj->lineState.condition |= tempLiu;

            pLineObj->lineEventHandle = pDevObj->timeStamp;
            pLineObj->lineEvents.fxo |=
                ((tempLiu) ? VP_LINE_EVID_LIU : VP_LINE_EVID_LNIU);

            if (!(tempLiu)) {
                /*
                 * Line status changed. Therefore, reset timer to prevent
                 * Ringing events.
                 */
                pFxoTimers->disconnectDebounce = VP_FXO_DISCONNECT_DEBOUNCE;
            }

            retFlag = TRUE;
        }

        /* If the feed conditions changed, continue line processing */
        if((pLineObj->lineState.condition & VP_CSLAC_RAW_DISC) != tempDiscSt) {
            VP_FXO_FUNC(VpLineCtxType, pLineCtx, ("RAW_DISC Change to %d at time %d",
                tempDiscSt, pDevObj->timeStamp));
            /*
             * Update actual line condition, even if not reporting an
             * event
             */
            if (tempDiscSt) {
                pLineObj->lineState.condition |= VP_CSLAC_RAW_DISC;
            } else {
                pLineObj->lineState.condition &= ~VP_CSLAC_RAW_DISC;
            }

            /*
             * Line status changed. Therefore, reset timer and update
             * pre-disconnect value (condition we think line is really in)
             */
            pFxoTimers->disconnectDebounce = VP_FXO_DISCONNECT_DEBOUNCE;
            pLineObj->preDisconnect = tempDiscSt;

            /*
             * Immediate disconnect changes detected do not result in
             * API-II event. So retFlag remains as set previously.
             */
        } else {
            /*
             * If the disconnect signal came back to the current state, stop
             * the debounce count
             */
            if (pLineObj->termType == VP_TERM_FXO_DISC) {
                pFxoTimers->noCount = TRUE;
                pFxoTimers->fxoDiscIO2Change = 0;
            }
        }
    } /* end interrupt detect */
    return retFlag;
} /* end Vp880ProcessFxoLine */
#endif

/**
 * Vp880IsChnlUndrTst()
 *  This function determines if a particular line of a device is currently
 * running a test.
 *
 * Preconditions:
 *  None.
 *
 * Postconditions:
 *  Device not affected. Return value TRUE if the line is currently running a
 * test, FALSE otherwise.
 */
static bool
Vp880IsChnlUndrTst(
    Vp880DeviceObjectType *pDevObj,
    uint8 channelId)
{
#ifdef VP880_INCLUDE_TESTLINE_CODE
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
 * Vp880LimitInRushCurrent()
 *  This function reduces the in-rush current during the battery transition
 *
 * Preconditions:
 *  None.
 *
 * Postconditions:
 *  Slac state is in ringing
 */
void
Vp880LimitInRushCurrent(
    Vp880DeviceObjectType *pDevObj,
    uint8 ecVal,    /* 0x00, 0x80, 0x01, 0x02, 0x81, 0x82 */
    bool callback)  /* TRUE if called from device timer, FALSE if called by Set Line State */
{
#if (IN_RUSH_CYCLE_TOTAL > 0)
    VpDeviceIdType deviceId = pDevObj->deviceId;
    uint8 swRegParam[3], regCtrl, ssConf, sStateRing, sState, channelId;

    if (callback == FALSE) {    /* Must be valid channel */
        channelId = ((ecVal & VP880_EC_BITS_MASK) >> 1);

        VP_LINE_STATE(None, NULL,("Vp880LimitInRushCurrent(): Initialization"));

        /* Limit only in fixed ringing */
        if (pDevObj->ringParams.channelArray[channelId] == FALSE) {
            VpMpiCmdWrapper(deviceId, pDevObj->ecVal, VP880_REGULATOR_PARAM_RD,
                VP880_REGULATOR_PARAM_LEN, swRegParam);
            VpMemCpy(pDevObj->ringParams.swRegParam, swRegParam, VP880_REGULATOR_PARAM_LEN);
        } else {
            VpMemCpy(swRegParam, pDevObj->ringParams.swRegParam, VP880_REGULATOR_PARAM_LEN);
        }

        pDevObj->ringParams.channelArray[channelId] = TRUE;

        /* The in-rush current limitation applies only in fixed voltage */
        /* Assuming that SWY is for VP880_EC_CH1 and SWZ is for VP880_EC_CH2 */
        if ((((ecVal & VP880_EC_BITS_MASK) == VP880_EC_CH1) &&
            (((swRegParam[0] & VP880_YRING_TRACK_MASK) == VP880_YRING_TRACK_EN) ||
             ((pDevObj->stateInt & VP880_LINE0_IS_FXO) == VP880_LINE0_IS_FXO))) ||
            (((ecVal & VP880_EC_BITS_MASK) == VP880_EC_CH2) &&
            (((swRegParam[0] & VP880_ZRING_TRACK_MASK) == VP880_ZRING_TRACK_EN) ||
             (pDevObj->stateInt & VP880_LINE1_IS_FXO)))) {

            sState = VP880_SS_BALANCED_RINGING;
            VpMpiCmdWrapper(deviceId, ecVal, VP880_SYS_STATE_WRT, VP880_SYS_STATE_LEN, &sState);
            VP_LINE_STATE(None, NULL,("Vp880LimitInRushCurrent(): Tracking enable, set and exit"));
            return;
        } else {
            if (pDevObj->ringParams.channelArray[0] == TRUE) {
                swRegParam[1] |= VP880_SWY_AUTOPOWER_DIS;
            }

            if (pDevObj->ringParams.channelArray[1] == TRUE) {
                swRegParam[2] |= VP880_SWZ_AUTOPOWER_DIS;
            }

            VpMpiCmdWrapper(deviceId, pDevObj->ecVal, VP880_REGULATOR_PARAM_WRT,
                VP880_REGULATOR_PARAM_LEN, swRegParam);
            VpMemCpy(pDevObj->swParams, swRegParam, VP880_REGULATOR_PARAM_LEN);
        }

        /* Set the battery to medium power */
        VpMpiCmdWrapper(deviceId, pDevObj->ecVal, VP880_REGULATOR_CTRL_RD,
            VP880_REGULATOR_CTRL_LEN, &regCtrl);
        if (channelId == 0) {
            regCtrl &= ~VP880_SWY_MODE_MASK;
            regCtrl |= VP880_SWY_MP;
        } else {
            regCtrl &= ~VP880_SWZ_MODE_MASK;
            regCtrl |= VP880_SWZ_MP;
        }
        VpMpiCmdWrapper(deviceId, pDevObj->ecVal, VP880_REGULATOR_CTRL_WRT,
            VP880_REGULATOR_CTRL_LEN, &regCtrl);

        /* Enable ZXR and set the slac state in balanced ringing */
        VpMpiCmdWrapper(deviceId, ecVal, VP880_SS_CONFIG_RD, VP880_SS_CONFIG_LEN, &ssConf);
        if ((ssConf & VP880_ZXR_MASK) == VP880_ZXR_DIS) {
            ssConf &= ~VP880_ZXR_DIS;
            VpMpiCmdWrapper(deviceId, ecVal, VP880_SS_CONFIG_WRT, VP880_SS_CONFIG_LEN, &ssConf);
        }

        sStateRing = VP880_SS_BALANCED_RINGING;
        VpMpiCmdWrapper(deviceId, ecVal, VP880_SYS_STATE_WRT, VP880_SYS_STATE_LEN, &sStateRing);

        pDevObj->ringParams.stage[channelId] = 0;
        pDevObj->devTimer[VP_DEV_TIMER_ENTER_RINGING] = (MS_TO_TICKRATE(10,
            pDevObj->devProfileData.tickRate)) | VP_ACTIVATE_TIMER;
    } else {    /* callback - no valid channel */
        VP_LINE_STATE(None, NULL,("Vp880LimitInRushCurrent(): callback"));

        for (channelId = 0 ; channelId < VP880_MAX_NUM_CHANNELS ; channelId++) {
            uint8 ecValCh = ((channelId == 0) ? VP880_EC_CH1 : VP880_EC_CH2);

            if (pDevObj->ringParams.channelArray[channelId] == TRUE) {
                pDevObj->ringParams.stage[channelId]++;

                if ((pDevObj->ringParams.stage[channelId] <= IN_RUSH_CYCLE_TOTAL) &&
                    (pDevObj->ringParams.stage[channelId] >
                    (IN_RUSH_CYCLE_TOTAL - IN_RUSH_CYCLE_END))) {
                    /* Charge the battery capacitor in medium power and allow ringing */
                    VP_LINE_STATE(None, NULL,("Vp880LimitInRushCurrent(): Finish Settling %d",
                        pDevObj->ringParams.stage[channelId]));
                    pDevObj->devTimer[VP_DEV_TIMER_ENTER_RINGING] = (MS_TO_TICKRATE(10,
                        pDevObj->devProfileData.tickRate)) | VP_ACTIVATE_TIMER;
                } else if (pDevObj->ringParams.stage[channelId] < IN_RUSH_CYCLE_TOTAL) {
                    /* Charge the battery capacitor in medium power and avoid ringing */
                    VP_LINE_STATE(None, NULL,("Vp880LimitInRushCurrent(): Settling %d",
                        pDevObj->ringParams.stage[channelId]));
                    sState = VP880_SS_ACTIVE;
                    VpMpiCmdWrapper(deviceId, (ecValCh | pDevObj->ecVal), VP880_SYS_STATE_WRT,
                        VP880_SYS_STATE_LEN, &sState);

                    sState = VP880_SS_BALANCED_RINGING;
                    VpMpiCmdWrapper(deviceId, (ecValCh | pDevObj->ecVal), VP880_SYS_STATE_WRT,
                        VP880_SYS_STATE_LEN, &sState);

                    pDevObj->devTimer[VP_DEV_TIMER_ENTER_RINGING] = (MS_TO_TICKRATE(10,
                        pDevObj->devProfileData.tickRate)) | VP_ACTIVATE_TIMER;
                } else {
                    VP_LINE_STATE(None, NULL,("Vp880LimitInRushCurrent(): Conclusion"));

                    /* Reset the regulator in original mode */
                    pDevObj->ringParams.channelArray[channelId] = FALSE;
                    VpMemCpy(swRegParam, pDevObj->ringParams.swRegParam, VP880_REGULATOR_PARAM_LEN);

                    if (pDevObj->ringParams.channelArray[0] == FALSE) {
                        if ((swRegParam[1] & VP880_SWY_AUTOPOWER_MASK) == VP880_SWY_AUTOPOWER_DIS) {
                            swRegParam[1] |= VP880_SWY_AUTOPOWER_DIS;
                        } else {
                            swRegParam[1] &= ~VP880_SWY_AUTOPOWER_DIS;
                        }
                    }

                    if (pDevObj->ringParams.channelArray[1] == FALSE) {
                        if ((swRegParam[1] & VP880_SWZ_AUTOPOWER_MASK) == VP880_SWZ_AUTOPOWER_DIS) {
                            swRegParam[1] |= VP880_SWZ_AUTOPOWER_DIS;
                        } else {
                            swRegParam[1] &= ~VP880_SWZ_AUTOPOWER_DIS;
                        }
                    }

                    if ((pDevObj->ringParams.channelArray[0] == FALSE) &&
                        (pDevObj->ringParams.channelArray[1] == FALSE)) {
                        VP_LINE_STATE(None, NULL,("Vp880LimitInRushCurrent(): kill the timer"));
                        pDevObj->devTimer[VP_DEV_TIMER_ENTER_RINGING] = 0;
                    }
                    VpMpiCmdWrapper(deviceId, pDevObj->ecVal, VP880_REGULATOR_PARAM_WRT,
                            VP880_REGULATOR_PARAM_LEN, swRegParam);
                    VpMemCpy(pDevObj->swParams, swRegParam, VP880_REGULATOR_PARAM_LEN);

                    /* Set the battery to high power */
                    VpMpiCmdWrapper(deviceId, pDevObj->ecVal, VP880_REGULATOR_CTRL_RD,
                        VP880_REGULATOR_CTRL_LEN, &regCtrl);
                    regCtrl |= ((channelId == 0) ? VP880_SWY_HP : VP880_SWZ_HP);
                    VpMpiCmdWrapper(deviceId, pDevObj->ecVal, VP880_REGULATOR_CTRL_WRT,
                        VP880_REGULATOR_CTRL_LEN, &regCtrl);
                }
            }
        }
    }
#endif
}

/**
 * Vp880LowLevelCmd()
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
 */
VpStatusType
Vp880LowLevelCmd(
    VpLineCtxType *pLineCtx,
    uint8 *pCmdData,
    uint8 len,
    uint16 handle)
{
    Vp880LineObjectType *pLineObj = pLineCtx->pLineObj;
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpDeviceIdType deviceId = pDevObj->deviceId;

    uint8 mpiIndex;

    uint8 ecVal = pLineObj->ecVal;

    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880LowLevelCmd+"));

    if (pDevObj->deviceEvents.response & VP880_READ_RESPONSE_MASK) {
        VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880LowLevelCmd-"));
        return VP_STATUS_DEVICE_BUSY;
    }

    VpSysEnterCritical(deviceId, VP_CODE_CRITICAL_SEC);
    if(pCmdData[0] & 0x01) { /* Read Command */
        VpMpiCmdWrapper(deviceId, ecVal, pCmdData[0], len,
            &(pDevObj->mpiData[0]));
        pDevObj->mpiLen = len;
        pLineObj->lineEvents.response |= VP_LINE_EVID_LLCMD_RX_CMP;
    } else {
        VpMpiCmdWrapper(deviceId, ecVal, pCmdData[0], len, &pCmdData[1]);
        for (mpiIndex = 0; mpiIndex < len; mpiIndex++) {
            pDevObj->mpiData[mpiIndex] = pCmdData[mpiIndex];
        }
        pLineObj->lineEvents.response |= VP_LINE_EVID_LLCMD_TX_CMP;
    }
    pLineObj->lineEventHandle = handle;

    VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);

    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("Vp880LowLevelCmd-"));

    return VP_STATUS_SUCCESS;
}

/**
 * Vp880UpdateBufferChanSel()
 *  This function sets the test buffer channel selection so that the hook bits
 * in the test buffer are valid for any channel in the active state.
 *
 * In order for any of the hook bits to be valid, the codec must be activated
 * for the channel selected by CBS in the device mode register.  This function
 * changes CBS so that if the given channel is being activated, that channel
 * is selected.  If the given channel is being deactivated, the other channel is
 * selected.
 *
 * This function should be called just before activating or deactivating the
 * codec for any channel.
 *
 * Arguments:
 *  channelId  -  The channel that is being changed
 *  sysState   -  Value of the system state register that is going to be
 *                programmed.  Contains the codec activate/deactivate bit.
 */
void
Vp880UpdateBufferChanSel(
    Vp880DeviceObjectType *pDevObj,
    uint8 channelId,
    uint8 sysState)
{
    VpDeviceIdType deviceId = pDevObj->deviceId;
    uint8 ecVal = (VP880_EC_CH1 | VP880_EC_CH2 | pDevObj->ecVal);

    uint8 devMode;
    bool activated;

    if (pDevObj->staticInfo.rcnPcn[VP880_RCN_LOCATION] <= VP880_REV_VC) {
        /* No test buffer to worry about in older revs */
        return;
    }

#ifdef VP880_INCLUDE_TESTLINE_CODE
    /* Do nothing if either channel is under test */
    if (Vp880IsChnlUndrTst(pDevObj, 0) || Vp880IsChnlUndrTst(pDevObj, 1)) {
        return;
    }
#endif

    if ((sysState & VP880_SS_ACTIVATE_MASK) == VP880_SS_ACTIVATE_MASK) {
        activated = TRUE;
    } else {
        activated = FALSE;
    }

    VpMpiCmdWrapper(deviceId, ecVal, VP880_DEV_MODE_RD, VP880_DEV_MODE_LEN,
        &devMode);
    devMode &= ~VP880_DEV_MODE_CHAN_MASK;

    if (channelId == 0 && activated == TRUE) {
        /* If channel 0 was activated, select channel 0 */
        devMode |= VP880_DEV_MODE_CHAN0_SEL;

    } else if (channelId == 0 && activated == FALSE) {
        /* If channel 0 was deactivated, select channel 1 */
        devMode |= VP880_DEV_MODE_CHAN1_SEL;

    } else if (channelId == 1 && activated == TRUE) {
        /* If channel 1 was activated, select channel 1 */
        devMode |= VP880_DEV_MODE_CHAN1_SEL;

    } else if (channelId == 1 && activated == FALSE) {
        /* If channel 1 was deactivated, select channel 0 */
        devMode |= VP880_DEV_MODE_CHAN0_SEL;
    }
    VpMpiCmdWrapper(deviceId, ecVal, VP880_DEV_MODE_WRT, VP880_DEV_MODE_LEN,
        &devMode);
}

/*******************************************************************************
 * Vp880ProtectedWriteICR1()
 *  This function is a wrapper for writing to ICR1.  If the internal test
 * termination is applied, ICR1 must not be changed, so this function copies
 * the data into the line object cache and returns without writing anything to
 * the device.  If the internal test termination is not applied, the write
 * is performed.
 *
 * Note: This function must be called from within a critical section.
 ******************************************************************************/
void
Vp880ProtectedWriteICR1(
    Vp880LineObjectType *pLineObj,
    VpDeviceIdType deviceId,
    uint8 *icr1Values)
{
    uint8 ecVal = pLineObj->ecVal;

    pLineObj->icr1Values[0] = icr1Values[0];
    pLineObj->icr1Values[1] = icr1Values[1];
    pLineObj->icr1Values[2] = icr1Values[2];
    pLineObj->icr1Values[3] = icr1Values[3];

    if (pLineObj->internalTestTermApplied == TRUE) {
        return;
    } else {
        VpMpiCmdWrapper(deviceId, ecVal, VP880_ICR1_WRT, VP880_ICR1_LEN,
            icr1Values);
    }
}

#endif
