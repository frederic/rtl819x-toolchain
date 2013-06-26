/** \file apiCal.c
 * apiCal.c
 *
 * This file contains the line and device calibration functions for
 * the Vp880 device API.
 *
 * Copyright (c) 2010, Zarlink Semiconductor, Inc.
 *
 * $Revision: 6680 $
 * $LastChangedDate: 2010-03-15 16:01:52 -0500 (Mon, 15 Mar 2010) $
 */

#include "vp_api_cfg.h"

#if defined (VP_CC_880_SERIES)
#if defined (VP880_CAL_ENABLE)

/* INCLUDES */
#include "vp_api_types.h"
#include "vp_api.h"
#include "vp_api_int.h"
#include "vp880_api.h"
#include "vp880_api_int.h"
#include "vp_hal.h"
#include "sys_service.h"

/* Time to wait before starting calibration after state change */
#define VP880_MIN_CAL_WAIT_TIME (200)

#define VP880_ABV_INIT_WAIT     (10)

#define VP880_CAL_ABV_LONG      (200)
#define VP880_CAL_ABV_DELAY     (20)
#define VP880_CAL_ABV_ABS_INIT  (1000)  /* was 2500 */

/* Per Nigel W., sample 100ms with error <= 3 */
#define VP880_CAL_ABV_SAMPLE        (100)
#define VP880_CAL_ABV_SAMPLE_ERR    (3)
#define VP880_CAL_ABV_SAMPLE_MAX    (40)    /* Limit the number of tries */

#define VP880_CAL_VOC_SHORT         (100)    /**< Time for converter to get data */

#define VP880_CAL_VOC_LONG      (100)    /**< Time for line to stabilize */

#define VP880_IMT_AVERAGE_CNT   (1)

#define VP880_VAS_INIT_WAIT     (200)
#define VP880_VAS_MEAS_DELAY    (50)    /* (20) */

#define VP880_VAS_MEAS_ERR      (100)   /**< Times 1.817ua = 187uA */
#define VP880_VAS_OVERHEAD      (3000)  /**< 3V for Temperature Variation */
#define VP880_VAS_MAX           (14250) /**< 14.25V per silicon */

#define VP880_V_PCM_LSB         (7324)  /**< 7.324V per LSB from PCM Data */
#define VP880_V_SCALE           ((int32)10000) /**< Scale to put in 10mV */

#define VP880_V_1V_SCALE        ((int32)13654)  /**< Based on 7.324V per LSB from PCM Data */
#define VP880_V_1V_RANGE        (100)   /**< To scale back to PCM range used */

#define VP880_CAL_SET            (1)    /**< Minimum time between tick */

/* Functions that are called only inside this file. */
static void
Vp880CalInit(
     VpDevCtxType *pDevCtx);

static void
Vp880AbvInit(
    VpDevCtxType *pDevCtx);

static void
Vp880AbvInitAbs(
    VpDevCtxType *pDevCtx);

static void
Vp880AbvSetAdc(
    VpDevCtxType *pDevCtx);

static void
Vp880AbvSetAdcAbs(
    VpDevCtxType *pDevCtx);

static void
Vp880AbvStateChange(
    VpDevCtxType *pDevCtx);

static void
Vp880AbvStateChangeAbs(
    VpDevCtxType *pDevCtx);

static bool
Vp880AbvMeasure(
    VpDevCtxType *pDevCtx);

static bool
Vp880AbvMeasureAbs(
    VpDevCtxType *pDevCtx);

static int16
Vp880AdcSettling(
    VpDeviceIdType deviceId,
    uint8 ecVal,
    uint8 adcConfig);

/* Functions for Cal Line */
#ifdef VP880_CAL_ENABLE
static void
Vp880CalLineInit(
     VpLineCtxType *pLineCtx);
#endif

static VpStatusType
Vp880CalVas(
    VpLineCtxType *pLineCtx);

static void
Vp880VasPolRev(
    VpLineCtxType *pLineCtx);

static VpStatusType
Vp880CalVoc(
    VpLineCtxType *pLineCtx);

static void
Vp880VocMeasure(
    VpLineCtxType *pLineCtx);

static void
Vp880VocMeasureInvert(
    VpLineCtxType *pLineCtx);

static void
Vp880CalDone(
    VpLineCtxType *pLineCtx);

static void
Vp880VocInit(
    VpLineCtxType *pLineCtx);

static void
Vp880VocSetAdc(
    VpLineCtxType *pLineCtx);

static void
Vp880VocOffset(
    VpLineCtxType *pLineCtx);

static void
Vp880VocDone(
    VpLineCtxType *pLineCtx);

/**
 * Vp880CalLine()
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
Vp880CalLine(
    VpLineCtxType *pLineCtx)
{
#ifdef VP880_CAL_ENABLE
    Vp880LineObjectType *pLineObj = pLineCtx->pLineObj;
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpDeviceIdType deviceId = pDevObj->deviceId;
    uint16 tickRate = pDevObj->devProfileData.tickRate;
    VpLineStateType currentState = pLineObj->lineState.currentState;
    uint8 channelId = pLineObj->channelId;
    uint8 ecVal[] = {VP880_EC_CH1, VP880_EC_CH2};
    uint8 opCond[VP880_OP_COND_LEN];
    uint16 minWaitTime = VP880_MIN_CAL_WAIT_TIME;

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
        VP_CALIBRATION(VpLineCtxType, pLineCtx, ("Return NOT_INITIALIZED from Vp880CalLine() Channel %d",
            channelId));
        return VP_STATUS_DEV_NOT_INITIALIZED;
    }

    VP_CALIBRATION(VpLineCtxType, pLineCtx, ("Running Cal Line on Channel %d at time %d",
        pLineObj->channelId, pDevObj->timeStamp));

    VpSysEnterCritical(deviceId, VP_CODE_CRITICAL_SEC);

    if (pDevObj->vp880SysCalData.ila40[channelId]) {
        pLineObj->lineEvents.response |= VP_EVID_CAL_CMP;
        pLineObj->lineState.calType = VP_CSLAC_CAL_NONE;
        VP_CALIBRATION(VpLineCtxType, pLineCtx, ("Calibration Previously Done. Cal Line Complete"));
        VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
        return VP_STATUS_SUCCESS;
    }

    if (pDevObj->staticInfo.rcnPcn[VP880_RCN_LOCATION] < VP880_REV_JE) {
        pLineObj->lineEvents.response |= VP_EVID_CAL_CMP;
        pLineObj->lineState.calType = VP_CSLAC_CAL_NONE;
        VP_CALIBRATION(VpLineCtxType, pLineCtx, ("Version %d (Pre %d): Cal Line Complete",
            pDevObj->staticInfo.rcnPcn[VP880_RCN_LOCATION], VP880_REV_JE));
        VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
        return VP_STATUS_SUCCESS;
    }

    pLineObj->status |= VP880_LINE_IN_CAL;

    /* Save off the current user state that will be restored later */
    pLineObj->calLineData.usrState = pLineObj->lineState.usrCurrent;

    Vp880CalLineInit(pLineCtx);

    if ((pLineObj->termType == VP_TERM_FXS_LOW_PWR) ||
        (pLineObj->termType == VP_TERM_FXS_ISOLATE_LP) ||
        (pLineObj->termType == VP_TERM_FXS_SPLITTER_LP)) {
        uint16 lpDiscTime = Vp880SetDiscTimers(pDevObj);
        minWaitTime = (minWaitTime > lpDiscTime) ? minWaitTime : lpDiscTime;
    }

    pLineObj->lineTimers.timers.timer[VP_LINE_CAL_LINE_TIMER] =
        (MS_TO_TICKRATE(minWaitTime, tickRate)) | VP_ACTIVATE_TIMER;

    switch(pLineObj->lineState.calType) {
        case VP_CSLAC_CAL_NONE:
            VP_CALIBRATION(VpLineCtxType, pLineCtx, ("Cal Line -- VP_CSLAC_CAL_NONE Channel %d",
                channelId));
            pLineObj->calLineData.reversePol = FALSE;

            /*
             * Since we're starting with VOC, check to make sure it can
             * be run.
             */
            switch(currentState) {
                case VP_LINE_STANDBY:
                case VP_LINE_STANDBY_POLREV:
                    Vp880SetLineState(pLineCtx, VP_LINE_OHT);
                    break;

                case VP_LINE_OHT:
                case VP_LINE_OHT_POLREV:
                    Vp880SetLineState(pLineCtx, VP_LINE_STANDBY);
                    Vp880SetLineState(pLineCtx, VP_LINE_OHT);
                    break;

                default:
                    pLineObj->lineState.calType = VP_CSLAC_CAL_NONE;
                    pLineObj->lineTimers.timers.timer[VP_LINE_CAL_LINE_TIMER] &=
                        ~VP_ACTIVATE_TIMER;
                    pLineObj->status &= ~VP880_LINE_IN_CAL;
                    VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
                    return VP_STATUS_INVALID_ARG;
            }

            pLineObj->calLineData.calState =  VP880_CAL_INIT;
            pLineObj->lineState.calType = VP_CSLAC_CAL_VOC;
            break;

        default:
            VP_CALIBRATION(VpLineCtxType, pLineCtx, ("Cal Line -- BUSY Channel %d", channelId));
            pLineObj->lineTimers.timers.timer[VP_LINE_CAL_LINE_TIMER] &=
                ~VP_ACTIVATE_TIMER;
            VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
            return VP_STATUS_DEVICE_BUSY;
    }

    /* Reprogram the Operating Conditions Register, affected by Set Line State */
    opCond[0] = (VP880_CUT_TXPATH | VP880_CUT_RXPATH | VP880_HIGH_PASS_DIS);
    VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_OP_COND_WRT, VP880_OP_COND_LEN,
        opCond);

    pLineObj->lineState.calType = VP_CSLAC_CAL_VOC;

    /*
     * Make sure if the control/query code is updating the device that it use
     * a state that will allow calibration measurements. This also allows time
     * for the state to transition before setting the converter configuration.
     */
    pLineObj->nextSlicValue = VP880_SS_ACTIVE;

    VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);

    return VP_STATUS_SUCCESS;
#else
    return VP_STATUS_FUNC_NOT_SUPPORTED;
#endif
}

/**
 * Vp880CalCodec()
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
Vp880CalCodec(
    VpLineCtxType *pLineCtx,
    VpDeviceCalType mode)
{
#ifdef VP880_CAL_ENABLE

    Vp880LineObjectType *pLineObj = pLineCtx->pLineObj;
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpDeviceIdType deviceId = pDevObj->deviceId;
    uint8 maxChannels = pDevObj->staticInfo.maxChannels;
    uint8 chanNum;
    VpLineStateType currentState;

    VpSysEnterCritical(deviceId, VP_CODE_CRITICAL_SEC);

    if ((pDevObj->staticInfo.rcnPcn[VP880_RCN_LOCATION] < VP880_REV_JE) ||
        (pDevObj->stateInt & VP880_SYS_CAL_COMPLETE)) {
        pDevObj->deviceEvents.response |= VP_EVID_CAL_CMP;
        pDevObj->responseData = VP_CAL_SUCCESS;
        VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
        return VP_STATUS_SUCCESS;
    }

    if (mode == VP_DEV_CAL_NBUSY) {
        for (chanNum = 0; chanNum < maxChannels; chanNum++) {
            if (pDevCtx->pLineCtx[chanNum] != VP_NULL) {
                pLineObj = pLineCtx->pLineObj;
                currentState = pLineObj->lineState.currentState;

                if (pLineObj->status & VP880_IS_FXO) {
                    if ((currentState == VP_LINE_FXO_TALK)
                     || (currentState == VP_LINE_FXO_LOOP_CLOSE)) {
                        pDevObj->deviceEvents.response |= VP_EVID_CAL_BUSY;
                        pDevObj->deviceEvents.response &= ~VP_EVID_CAL_CMP;
                        VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
                        return VP_STATUS_SUCCESS;
                    }
                } else {
                    if ((currentState != VP_LINE_STANDBY)
                     && (currentState != VP_LINE_DISCONNECT)) {
                        pDevObj->deviceEvents.response |= VP_EVID_CAL_BUSY;
                        pDevObj->deviceEvents.response &= ~VP_EVID_CAL_CMP;
                        VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
                        return VP_STATUS_SUCCESS;
                    }
                }
            }
        }
    }

    pDevObj->status.state |= VP_DEV_IN_CAL;

    if (pDevObj->stateInt & VP880_IS_ABS) { /* Start for ABS Device */
        if (Vp880SetCalFlags(pDevObj) == TRUE) {
            Vp880CalCodecInt(pDevCtx);
        }
    } else {    /* Start for Tracker Device */
        pDevObj->status.state |= VP_DEV_ABV_CAL;
        Vp880CalCodecInt(pDevCtx);
    }

    VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
    return VP_STATUS_SUCCESS;
#else
    return VP_STATUS_FUNC_NOT_SUPPORTED;
#endif
}

/**
 * Vp880SetCalFlags() -- ABS Only Function
 *  This function sets the calibration flags to define start of calibration and
 * calibration function to start with for the ABS device.
 *
 * Preconditions:
 *  The device must be created and initialized before calling this function.
 * This function should be called only by API-II internal functions, generally
 * CalCodec and InitDevice.
 *
 * Postconditions:
 *  Flags in the device object are set appropriately for the silicon revision.
 */
bool
Vp880SetCalFlags(
    Vp880DeviceObjectType *pDevObj)
{
    bool calStatus = FALSE;

    if (pDevObj->staticInfo.rcnPcn[VP880_RCN_LOCATION] >= VP880_REV_JE) {
        /* Later than VC requires ABV and ABS calibration, start with ABV */
        pDevObj->status.state |= VP_DEV_ABV_CAL_ABS;
        calStatus = TRUE;
    } else if (pDevObj->staticInfo.rcnPcn[VP880_RCN_LOCATION] == VP880_REV_VC) {
        /* VC requires ABS calibration only. */
        pDevObj->status.state |= VP_DEV_ABS_BAT_CAL;
        calStatus = TRUE;
    } else {
        /* No calibration required for earlier than VC silicon */
    }

    if (calStatus == TRUE) {
        pDevObj->status.state |= VP_DEV_IN_CAL;
        pDevObj->calData.calState = VP880_CAL_INIT;
    } else {
        pDevObj->status.state &= ~VP_DEV_IN_CAL;
        pDevObj->calData.calState &= ~VP880_CAL_INIT;
    }
    return calStatus;
}


/**
 * Vp880CalCodecInt()
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
Vp880CalCodecInt(
    VpDevCtxType *pDevCtx)
{
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;

    /*
     * Run appropriate calibration currently set. Note that the same calibration
     * function may be run for either Tracker or ABS devices.
     */
    if (pDevObj->status.state & VP_DEV_ABV_CAL) {
        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("Calling ABV Cal inside Vp880CalCodecInt() for Tracker"));
        Vp880CalAbv(pDevCtx);
        return VP_STATUS_SUCCESS;
    }

    if (pDevObj->status.state & VP_DEV_ABS_BAT_CAL) {
        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("Calling ABS Cal inside Vp880CalCodecInt()"));
        Vp880AbsCalibration(pDevCtx);
        return VP_STATUS_SUCCESS;
    }

    if (pDevObj->status.state & VP_DEV_ABV_CAL_ABS) {
        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("Calling ABS Cal inside Vp880CalCodecInt() for ABS"));
        Vp880CalAbvAbsDev(pDevCtx);
        return VP_STATUS_SUCCESS;
    }

    return VP_STATUS_SUCCESS;
}

/**
 * Vp880AbsCalibration() -- ABS Only Function
 *  This function is called only through Vp880ApiTick() to run an ABS battery
 * calibrary algorithm. The result is a computed DC offset for each polartiy
 * state.
 *
 * Preconditions:
 *  The device must first be initialized.
 *
 * Postconditions:
 *  Upon completion, the Init Device Complete event is generated and the device
 * is considered initialized. The offset values for each line in both normal and
 * reverse polarity are stored in the device object. Note that the offset is
 * independent of the line object since it is a function only of the physical
 * silicon.
 *
 * Note regarding Low Power Mode:
 *  This function can set the System State (SLIC State) directly without any
 * problem because it all occurs prior to lines being initialized.
 */
void
Vp880AbsCalibration(
    VpDevCtxType *pDevCtx)
{
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    uint8 ecVal[] = {VP880_EC_CH1, VP880_EC_CH2};
    VpDeviceIdType deviceId = pDevObj->deviceId;
    uint8 maxChan = pDevObj->staticInfo.maxChannels;
    uint8 cmdData[VP880_MAX_MPI_DATA];
    uint8 switcherData[VP880_REGULATOR_CTRL_LEN];
    uint8 firstValue[VP880_DC_CAL_REG_LEN] = {0xF4, 0xE4};
    uint8 lastValue[VP880_DC_CAL_REG_LEN] = {0x00, 0x02};
    uint8 icr2Values[VP880_ICR2_LEN] = {0xC0, 0x00, 0x00, 0x00};
    uint8 chan;
    uint16 tickRate = pDevObj->devProfileData.tickRate;

    /*
     * To save on memory, the calibration state in terms of the state machine
     * state, channel, and which polarity is being tested are all stored in
     * a single byte. The mask for each parameter as follows.
     */
#define ABS_DC_CAL_STATE_BITS       0x3F
#define ABS_DC_CAL_CHAN_BITS        0x80
#define ABS_DC_CAL_POLREV_BITS      0x40

    /* State values corresponding to what the state is "doing" */
#define ABS_DC_CAL_INIT_STATE       0
#define ABS_DC_CAL_CONNECT_STATE    1
#define ABS_DC_CAL_MEAS_ADJ_STATE   2

#define ABS_CAL_INITIAL_DELAY   30
#define ABS_CAL_SAMPLE_DELAY    5

    uint8 channelId = ((pDevObj->calState & ABS_DC_CAL_CHAN_BITS) >> 7);

    /* If set, then running line in reverse polarity */
    uint8 revPolTest = (pDevObj->calState & ABS_DC_CAL_POLREV_BITS);

    uint8 chanNum;

    bool complete = FALSE;

    switch(pDevObj->calState & ABS_DC_CAL_STATE_BITS) {

        case ABS_DC_CAL_INIT_STATE:
            VP_CALIBRATION(VpDevCtxType, pDevCtx, ("ABS_DC_CAL_INIT_STATE at Time %d",
                pDevObj->timeStamp));

            VpMpiCmdWrapper(deviceId, ecVal[0], VP880_REGULATOR_CTRL_RD,
                VP880_REGULATOR_CTRL_LEN, pDevObj->cachedSwCtrl);

            /* Steps 1 and first part of 2 */
            switcherData[0] = VP880_SWY_OFF;
            VpMpiCmdWrapper(deviceId, ecVal[0], VP880_REGULATOR_CTRL_WRT,
                VP880_REGULATOR_CTRL_LEN, switcherData);

            cmdData[0] = VP880_SS_DISCONNECT;
            for (chanNum = 0; chanNum < maxChan; chanNum++) {
                VP_CALIBRATION(VpDevCtxType, pDevCtx, ("Setting Channel %d to DISCONNECT at time %d",
                    chanNum, pDevObj->timeStamp));

                VpMpiCmdWrapper(deviceId, ecVal[chanNum], VP880_SYS_STATE_WRT,
                    VP880_SYS_STATE_LEN, cmdData);

                VP_CALIBRATION(VpDevCtxType, pDevCtx, ("Writing ICR2 Channel %d to 0x%02X 0x%02X 0x%02X 0x%02X",
                    chanNum, icr2Values[0], icr2Values[1], icr2Values[2], icr2Values[3]));

                VpMpiCmdWrapper(deviceId, ecVal[chanNum], VP880_ICR2_WRT,
                    VP880_ICR2_LEN, icr2Values);
            }

            for (chanNum = 0; chanNum < maxChan; chanNum++) {
                VpMpiCmdWrapper(deviceId, ecVal[chanNum], VP880_DC_CAL_REG_WRT,
                    VP880_DC_CAL_REG_LEN, firstValue);
            }

            pDevObj->calState &= ~ABS_DC_CAL_STATE_BITS;
            pDevObj->calState |= ABS_DC_CAL_CONNECT_STATE;
            pDevObj->devTimer[VP_DEV_TIMER_ABSCAL] =
                MS_TO_TICKRATE(ABS_CAL_INITIAL_DELAY, tickRate) | VP_ACTIVATE_TIMER;
            pDevObj->calState |= 0x80;  /* Start with Channel 2 */
            break;

        case ABS_DC_CAL_CONNECT_STATE:
            VP_CALIBRATION(VpDevCtxType, pDevCtx, ("ABS_DC_CAL_CONNECT_STATE for Chan %d Polarity %d",
                channelId, revPolTest));

            VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_DC_CAL_REG_WRT,
                VP880_DC_CAL_REG_LEN, firstValue);

            /* Last part of 2 and Step 3 */
            cmdData[0] = (revPolTest) ?
                VP880_SS_ACTIVE_MID_BAT_PR : VP880_SS_ACTIVE_MID_BAT;

            VP_CALIBRATION(VpDevCtxType, pDevCtx, ("Setting Channel %d to State 0x%02X at time %d",
                channelId, cmdData[0], pDevObj->timeStamp));

            VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_SYS_STATE_WRT,
                VP880_SYS_STATE_LEN, cmdData);

            pDevObj->calState &= ~ABS_DC_CAL_STATE_BITS;
            pDevObj->calState |= ABS_DC_CAL_MEAS_ADJ_STATE;
            pDevObj->devTimer[VP_DEV_TIMER_ABSCAL] =
                MS_TO_TICKRATE(ABS_CAL_INITIAL_DELAY, tickRate) | VP_ACTIVATE_TIMER;
            break;

        case ABS_DC_CAL_MEAS_ADJ_STATE:
            /* Most cases, we're returning. */
            pDevObj->devTimer[VP_DEV_TIMER_ABSCAL] =
                MS_TO_TICKRATE(ABS_CAL_SAMPLE_DELAY,
                pDevObj->devProfileData.tickRate) | VP_ACTIVATE_TIMER;

            VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_DC_CAL_REG_RD,
                VP880_DC_CAL_REG_LEN, cmdData);

            VP_CALIBRATION(VpDevCtxType, pDevCtx, ("ABS_DC_CAL_MEAS_ADJ_STATE for Chan %d Polarity %d Value 0x%02X 0x%02X",
                channelId, revPolTest, cmdData[0], cmdData[1]));

            if (cmdData[VP880_DC_CAL_BLIM_INDEX] & VP880_DC_CAL_BLIM) {
                /* Good. Save this value */
                if (revPolTest) {
                    /* Saving the polarity reversal information */
                    pDevObj->vp880SysCalData.absPolRevCal[channelId] =
                        cmdData[VP880_DC_CAL_ABS_INDEX] & 0xF0;

                    VP_CALIBRATION(VpDevCtxType, pDevCtx, ("Saving PolRev 0x%02X for Ch %d",
                        pDevObj->vp880SysCalData.absPolRevCal[channelId], channelId));
                } else {
                    /* Saving the normal polarity information */
                    pDevObj->vp880SysCalData.absNormCal[channelId] =
                        cmdData[VP880_DC_CAL_ABS_INDEX] & 0xF0;

                    VP_CALIBRATION(VpDevCtxType, pDevCtx, ("Saving Normal 0x%02X for Ch %d",
                        pDevObj->vp880SysCalData.absNormCal[channelId], channelId));
                }

                /* Determine if there's anything else to do */
                switch (channelId) {
                    case 1:
                        if (revPolTest) {
                            /* Change to channel 1, Normal Polarity */
                            pDevObj->calState |= ABS_DC_CAL_CHAN_BITS;
                            pDevObj->calState &= ~ABS_DC_CAL_POLREV_BITS;
                            pDevObj->calState &= 0x7F;

                            VP_CALIBRATION(VpDevCtxType, pDevCtx, ("Changing to Normal Polarity for Channel 1 0x%02X",
                                pDevObj->calState));
                        } else {
                            /* Change to Reverse Polarity */
                            VP_CALIBRATION(VpDevCtxType, pDevCtx, ("Changing to PolRev, Channel %d 0x%02X",
                                channelId, pDevObj->calState));
                            pDevObj->calState |= ABS_DC_CAL_POLREV_BITS;
                        }

                        /* Repeat with new connections */
                        pDevObj->calState &= ~ABS_DC_CAL_STATE_BITS;
                        pDevObj->calState |= ABS_DC_CAL_CONNECT_STATE;
                        break;

                    case 0:
                        if (revPolTest) {
                            /* Done */
                            VP_CALIBRATION(VpDevCtxType, pDevCtx, ("Cal Complete For Device %d", deviceId));
                            complete = TRUE;
                        } else {
                            /* Change to Reverse Polarity */
                            pDevObj->calState |= ABS_DC_CAL_POLREV_BITS;

                            /* Repeat with new connections */
                            pDevObj->calState &= ~ABS_DC_CAL_STATE_BITS;
                            pDevObj->calState |= ABS_DC_CAL_CONNECT_STATE;

                            VP_CALIBRATION(VpDevCtxType, pDevCtx, ("Changing to PolRev for Channel %d 0x%02X",
                                channelId, pDevObj->calState));
                        }
                        break;

                    default:
                        /* Error */
                        VP_ERROR(VpDevCtxType, pDevCtx, ("ChannelId Error %d", channelId));
                        complete = TRUE;
                        break;
                }
            } else {
                /* Change the current offset and try again */
                if (cmdData[VP880_DC_CAL_ABS_INDEX] & 0x80) {
                    if ((cmdData[VP880_DC_CAL_ABS_INDEX] & 0xF0) == 0x80) {
                        cmdData[VP880_DC_CAL_ABS_INDEX] = 0;
                    } else {
                        cmdData[VP880_DC_CAL_ABS_INDEX] -= 16;
                    }
                } else {
                    if ((cmdData[VP880_DC_CAL_ABS_INDEX] & 0xF0) == 0x70) {
                        /*
                         * Something wrong happened. Restore back to 0 and end
                         * algorithm.
                         */
                        complete = TRUE;
                        VP_ERROR(VpDevCtxType, pDevCtx, ("Calibration Algorithm Error 0x%02X",
                            pDevObj->status.state));
                    } else {
                        cmdData[VP880_DC_CAL_ABS_INDEX] += 16;
                    }
                }
                VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_DC_CAL_REG_WRT,
                    VP880_DC_CAL_REG_LEN, cmdData);

                VP_CALIBRATION(VpDevCtxType, pDevCtx, ("Adjusting Offset 0x%02X",
                    cmdData[0]));
            }
            break;

        default:
            /* oops. shouldn't be here. */
            VP_ERROR(VpDevCtxType, pDevCtx, ("Calibration Case Error %d",
                (pDevObj->calState & ABS_DC_CAL_STATE_BITS)));
            complete = TRUE;
            break;
    }

    if (complete == TRUE) {
        /* We're done. Disable timers */
        pDevObj->devTimer[VP_DEV_TIMER_ABSCAL] = 0;

        cmdData[0] = VP880_SS_DISCONNECT;
        for (chanNum = 0; chanNum < maxChan; chanNum++) {
            VP_CALIBRATION(VpDevCtxType, pDevCtx, ("Setting Channel %d to VP880_SS_DISCONNECT at time %d",
                cmdData[0], pDevObj->timeStamp));

            VpMpiCmdWrapper(deviceId, ecVal[chanNum], VP880_SYS_STATE_WRT,
                VP880_SYS_STATE_LEN, cmdData);

            VpMpiCmdWrapper(deviceId, ecVal[chanNum], VP880_ICR2_RD,
                VP880_ICR2_LEN, icr2Values);
            icr2Values[VP880_ICR2_FEED_CTRL_INDEX+1] &= ~VP880_ICR2_FEED_CTRL;

            VP_CALIBRATION(VpDevCtxType, pDevCtx, ("Cal Complete: Writing ICR2 Channel %d to 0x%02X 0x%02X 0x%02X 0x%02X",
                chanNum, icr2Values[0], icr2Values[1], icr2Values[2], icr2Values[3]));

            VpMpiCmdWrapper(deviceId, ecVal[chanNum], VP880_ICR2_WRT,
                VP880_ICR2_LEN, icr2Values);
        }

        VpMpiCmdWrapper(deviceId, (ecVal[0] | ecVal[1]), VP880_DC_CAL_REG_WRT,
            VP880_DC_CAL_REG_LEN, lastValue);
        /* Cache the ICR6 values for existing line objects */
        for (chan = 0; chan < pDevObj->staticInfo.maxChannels; chan++) {
            VpLineCtxType *pLineCtx = pDevCtx->pLineCtx[chan];
            if (pLineCtx != VP_NULL) {
                Vp880LineObjectType *pLineObj = pLineCtx->pLineObj;
                VpMemCpy(pLineObj->icr6Values, lastValue, VP880_DC_CAL_REG_LEN);
            }
        }

        VpMpiCmdWrapper(deviceId, ecVal[0], VP880_REGULATOR_CTRL_WRT,
            VP880_REGULATOR_CTRL_LEN, pDevObj->cachedSwCtrl);

        if (pDevObj->status.state & VP_DEV_INIT_IN_PROGRESS) {
            pDevObj->deviceEvents.response |= VP_DEV_EVID_DEV_INIT_CMP;
        } else {
            pDevObj->deviceEvents.response |= VP_EVID_CAL_CMP;
        }

        /* Restore device mode to test buffer if exists */
        if (pDevObj->staticInfo.rcnPcn[VP880_RCN_LOCATION] > VP880_REV_VC) {
            uint8 deviceMode[VP880_DEV_MODE_LEN];

            VpMpiCmdWrapper(deviceId, (ecVal[0] | ecVal[1]), VP880_DEV_MODE_RD,
                VP880_DEV_MODE_LEN, deviceMode);
            deviceMode[0] |= VP880_DEV_MODE_TEST_DATA;
            VpMpiCmdWrapper(deviceId, (ecVal[0] | ecVal[1]), VP880_DEV_MODE_WRT,
                VP880_DEV_MODE_LEN, deviceMode);
        }

        pDevObj->status.state &= ~(VP_DEV_INIT_IN_PROGRESS | VP_DEV_IN_CAL);
        pDevObj->calState = VP880_CAL_INIT;
        pDevObj->status.state &= ~VP_DEV_ABS_BAT_CAL;
    }
}

/**
 * Vp880CalAbv() -- Tracker Only Function
 *  This function initiates a calibration operation for Absolute Switcher
 *  circuits associated with all the lines of a device. See VP-API reference
 *  guide for more information. SWYV SWZV are global for every Channels
 *  Line must be in Disconnect state before to start the Calibration
 *
 * Preconditions:
 *  The device and line context must be created and initialized before calling
 * this function.
 *
 * Postconditions:
 *  This function generates an event upon completing the requested action.
 */
VpStatusType
Vp880CalAbv(
     VpDevCtxType *pDevCtx)
{
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpDeviceIdType deviceId = pDevObj->deviceId;
    VpStatusType  status = VP_STATUS_SUCCESS;
    uint8 ecVal;
    uint8 fxsChannels;
    bool calCleanup = FALSE;

    if ((pDevObj->stateInt & VP880_IS_SINGLE_CHANNEL)
     || (pDevObj->stateInt & VP880_LINE1_IS_FXO)) {
        ecVal = VP880_EC_CH1;
        fxsChannels = 1;
    } else {
        ecVal = VP880_EC_CH1 | VP880_EC_CH2;
        fxsChannels = 2;
    }

    if (pDevObj->calData.calState == VP880_CAL_INIT
        || pDevObj->calData.calState == VP880_CAL_EXIT) {
        VP_CALIBRATION(VpDevCtxType, pDevCtx,("Vp880CalAbv:  - Setting to Vp880CalInit"));

        pDevObj->calData.calState = VP880_CAL_INIT;
        Vp880CalInit(pDevCtx);
    }

    switch(pDevObj->calData.calState) {
        case VP880_CAL_INIT:
            VP_CALIBRATION(VpDevCtxType, pDevCtx,("Vp880CalAbv: - Running Vp880AbvInit"));
            Vp880AbvInit(pDevCtx);
            break;

        case VP880_CAL_ADC:
            VP_CALIBRATION(VpDevCtxType, pDevCtx,("Vp880CalAbv: - Running Vp880AbvSetAdc"));
            Vp880AbvSetAdc(pDevCtx);
            break;

        case VP880_CAL_STATE_CHANGE:
            VP_CALIBRATION(VpDevCtxType, pDevCtx,("Vp880CalAbv: - Running Vp880AbvStateChange"));
            Vp880AbvStateChange(pDevCtx);
            break;

        case VP880_CAL_MEASURE:
            VP_CALIBRATION(VpDevCtxType, pDevCtx,("Vp880CalAbv - Running Vp880AbvMeasure"));
            Vp880AbvMeasure(pDevCtx);
            break;

        case VP880_CAL_DONE:
            VP_CALIBRATION(VpDevCtxType, pDevCtx,("ABV Cal Done"));
            calCleanup = TRUE;
            pDevObj->responseData = VP_CAL_SUCCESS;
            break;

        case VP880_CAL_ERROR:
            /* Fall through intentional */
        default:
            VP_ERROR(VpDevCtxType, pDevCtx,("Vp880CalAbv: ERROR - Cal Done"));
            calCleanup = TRUE;
            status = VP_STATUS_FAILURE;
            pDevObj->responseData = VP_CAL_FAILURE;
            break;
    }

    if (calCleanup == TRUE) {
        uint8 convCfg[VP880_CONV_CFG_LEN];

        pDevObj->calData.calState = VP880_CAL_EXIT;
        if (pDevObj->status.state & VP_DEV_INIT_IN_PROGRESS) {
            pDevObj->deviceEvents.response |= VP_DEV_EVID_DEV_INIT_CMP;
            pDevObj->status.state |= VP_DEV_INIT_CMP;
        } else {
            pDevObj->deviceEvents.response |= VP_EVID_CAL_CMP;
        }
        pDevObj->status.state &= ~(VP_DEV_INIT_IN_PROGRESS | VP_DEV_IN_CAL);
        pDevObj->status.state &= ~VP_DEV_ABV_CAL;

        /* Restore device mode to test buffer if exists */
        if (pDevObj->staticInfo.rcnPcn[VP880_RCN_LOCATION] > VP880_REV_VC) {
            uint8 deviceMode[VP880_DEV_MODE_LEN];

            VpMpiCmdWrapper(deviceId, ecVal, VP880_DEV_MODE_RD,
                VP880_DEV_MODE_LEN, deviceMode);
            deviceMode[0] |= VP880_DEV_MODE_TEST_DATA;
            VpMpiCmdWrapper(deviceId, ecVal, VP880_DEV_MODE_WRT,
                VP880_DEV_MODE_LEN, deviceMode);
        }

        /* Reset Line states */
        VP_CALIBRATION(VpDevCtxType, pDevCtx,("Calibration Cleanup -- Setting Channel 0 to State 0x%02X at time %d",
            pDevObj->calData.abvData.sysState[0][0], pDevObj->timeStamp));
        VP_CALIBRATION(VpDevCtxType, pDevCtx,("Calibration Cleanup -- Setting Channel 1 to State 0x%02X at time %d",
            pDevObj->calData.abvData.sysState[1][0], pDevObj->timeStamp));

        VpMpiCmdWrapper(deviceId, VP880_EC_CH1, VP880_SYS_STATE_WRT, VP880_SYS_STATE_LEN,
            &pDevObj->calData.abvData.sysState[0][0]);
        VpMpiCmdWrapper(deviceId, VP880_EC_CH2, VP880_SYS_STATE_WRT, VP880_SYS_STATE_LEN,
            &pDevObj->calData.abvData.sysState[1][0]);

        VpMpiCmdWrapper(deviceId, VP880_EC_CH1, VP880_ICR2_WRT, VP880_ICR2_LEN,
            &pDevObj->calData.abvData.icr2[0][0]);
        VpMpiCmdWrapper(deviceId, VP880_EC_CH2, VP880_ICR2_WRT, VP880_ICR2_LEN,
            &pDevObj->calData.abvData.icr2[1][0]);

        VP_CALIBRATION(VpDevCtxType, pDevCtx,("Calibration Cleanup -- Channel 0 Writing ICR2 to 0x%02X 0x%02X 0x%02X 0x%02X",
            pDevObj->calData.abvData.icr2[0][0],
            pDevObj->calData.abvData.icr2[0][1],
            pDevObj->calData.abvData.icr2[0][2],
            pDevObj->calData.abvData.icr2[0][3]));

        VP_CALIBRATION(VpDevCtxType, pDevCtx,("Calibration Cleanup -- Channel 1 Writing ICR2 to 0x%02X 0x%02X 0x%02X 0x%02X",
            pDevObj->calData.abvData.icr2[1][0],
            pDevObj->calData.abvData.icr2[1][1],
            pDevObj->calData.abvData.icr2[1][2],
            pDevObj->calData.abvData.icr2[1][3]));

        VpMpiCmdWrapper(deviceId, VP880_EC_CH1, VP880_ICR3_WRT, VP880_ICR3_LEN,
            &pDevObj->calData.abvData.icr3[0][0]);
        VpMpiCmdWrapper(deviceId, VP880_EC_CH2, VP880_ICR3_WRT, VP880_ICR3_LEN,
            &pDevObj->calData.abvData.icr3[1][0]);

        VpMpiCmdWrapper(deviceId, VP880_EC_CH1, VP880_ICR4_WRT, VP880_ICR4_LEN,
            &pDevObj->calData.abvData.icr4[0][0]);

        VP_CALIBRATION(VpDevCtxType, pDevCtx,("Calibration Cleanup -- Channel 0 Writing ICR4 to 0x%02X 0x%02X 0x%02X 0x%02X",
            pDevObj->calData.abvData.icr4[0][0],
            pDevObj->calData.abvData.icr4[0][1],
            pDevObj->calData.abvData.icr4[0][2],
            pDevObj->calData.abvData.icr4[0][3]));

        VpMpiCmdWrapper(deviceId, VP880_EC_CH2, VP880_ICR4_WRT, VP880_ICR4_LEN,
            &pDevObj->calData.abvData.icr4[1][0]);

        VP_CALIBRATION(VpDevCtxType, pDevCtx,("Calibration Cleanup -- Channel 1 Writing ICR4 to 0x%02X 0x%02X 0x%02X 0x%02X",
            pDevObj->calData.abvData.icr4[1][0],
            pDevObj->calData.abvData.icr4[1][1],
            pDevObj->calData.abvData.icr4[1][2],
            pDevObj->calData.abvData.icr4[1][3]));

        /* Restore Converter Configuration */
        convCfg[0] = (VP880_METALLIC_AC_V | pDevObj->txBufferDataRate);
        VpMpiCmdWrapper(deviceId, (VP880_EC_CH1 | VP880_EC_CH2), VP880_CONV_CFG_WRT,
            VP880_CONV_CFG_LEN, convCfg);
    }

    return status;
}

/**
 * Vp880AbvInit() -- Tracker (ABV Cal) only Function
 *  This function initiates a calibration operation for ABV
 * associated with all the lines of a device. See VP-API reference guide for
 * more information.

 * Preconditions:
 *  The device and line context must be created and initialized before calling
 * this function.
 *
 * Postconditions:
 *  This function generates an event upon completing the requested action.
 */
void
Vp880AbvInit(
    VpDevCtxType *pDevCtx)
{
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpDeviceIdType deviceId = pDevObj->deviceId;
    uint8 disnVal[VP880_DISN_LEN] = {0x00};
    uint8 channelId;

    uint8 isrpMods[VP880_INT_SWREG_PARAM_LEN] = {
        0x00, 0x40, 0x00, 0x40, 0x00, 0x40
    };

    uint8 icr3[VP880_ICR3_LEN] = {0x30, 0x20, 0x00, 0x00};
    uint8 icr4[VP880_ICR4_LEN] = {0x01, 0x01, 0x00, 0x00};
    uint8 icr2[VP880_ICR2_LEN] = {0x00, 0xEC, 0x2C, 0x2C};

    uint8 data, ecVal;
    uint8 swCal[VP880_BAT_CALIBRATION_LEN];

    /*
     * Initialize and use to measure each channels offset and voltage using
     * same functions.
     */
    pDevObj->calData.abvData.passCnt = 0;

    /* Read (for restore) internal switcher parameters */
    VpMpiCmdWrapper(deviceId, VP880_EC_CH1, VP880_INT_SWREG_PARAM_RD,
        VP880_INT_SWREG_PARAM_LEN, pDevObj->calData.abvData.isrpData);

    /* Channel specific registers to restore at end of calibration */
    for (channelId = 0; channelId < pDevObj->staticInfo.maxChannels; channelId++) {
        ecVal = (channelId == 0) ? VP880_EC_CH1 : VP880_EC_CH2;

        /* Save off current slic state */
        VpMpiCmdWrapper(deviceId, ecVal, VP880_SYS_STATE_RD, VP880_SYS_STATE_LEN,
            &pDevObj->calData.abvData.sysState[channelId][0]);

        /*
         * Disable switcher by setting duty cycle = 0. Global, so only need
         * to do once.
         */
        if (channelId == 0) {
            VpMpiCmdWrapper(deviceId, ecVal, VP880_INT_SWREG_PARAM_WRT,
                VP880_INT_SWREG_PARAM_LEN, isrpMods);
        }

        /* Clear existing correction factors */
        VpMpiCmdWrapper(deviceId, ecVal, VP880_BAT_CALIBRATION_RD,
            VP880_BAT_CALIBRATION_LEN, swCal);
        swCal[0] &= ~(VP880_BAT_CAL_SWCAL_MASK);
        VpMpiCmdWrapper(deviceId, ecVal, VP880_BAT_CALIBRATION_WRT,
            VP880_BAT_CALIBRATION_LEN, swCal);

        VpMpiCmdWrapper(deviceId, ecVal, VP880_ICR2_RD, VP880_ICR2_LEN,
            &pDevObj->calData.abvData.icr2[channelId][0]);

        VP_CALIBRATION(VpDevCtxType, pDevCtx,("Saving ICR2 0x%02X 0x%02X 0x%02X 0x%02X Ch %d",
            pDevObj->calData.abvData.icr2[channelId][0],
            pDevObj->calData.abvData.icr2[channelId][1],
            pDevObj->calData.abvData.icr2[channelId][2],
            pDevObj->calData.abvData.icr2[channelId][3],
            channelId));

        VpMpiCmdWrapper(deviceId, ecVal, VP880_ICR3_RD, VP880_ICR3_LEN,
            &pDevObj->calData.abvData.icr3[channelId][0]);
        VP_CALIBRATION(VpDevCtxType, pDevCtx,("Saving ICR3 0x%02X 0x%02X 0x%02X 0x%02X Ch %d",
            pDevObj->calData.abvData.icr3[channelId][0],
            pDevObj->calData.abvData.icr3[channelId][1],
            pDevObj->calData.abvData.icr3[channelId][2],
            pDevObj->calData.abvData.icr3[channelId][3],
            channelId));

        VpMpiCmdWrapper(deviceId, ecVal, VP880_ICR4_RD, VP880_ICR4_LEN,
            &pDevObj->calData.abvData.icr4[channelId][0]);

        VpMpiCmdWrapper(deviceId, ecVal, VP880_DISN_RD, VP880_DISN_LEN,
            &pDevObj->calData.abvData.disnVal[channelId][0]);
    }

    if ((pDevObj->stateInt & VP880_IS_SINGLE_CHANNEL)
     || (pDevObj->stateInt & VP880_LINE1_IS_FXO)) {
        ecVal = VP880_EC_CH1;
    } else {
        ecVal = VP880_EC_CH1 | VP880_EC_CH2;
    }

    /* Disable the switchers */
    VP_CALIBRATION(VpDevCtxType, pDevCtx,("Writing ICR2 0x%02X 0x%02X 0x%02X 0x%02X BOTH channels",
        icr2[0], icr2[1], icr2[2], icr2[3]));
    VpMpiCmdWrapper(deviceId, ecVal, VP880_ICR2_WRT, VP880_ICR2_LEN, icr2);

    /* Force sink supply current to reduce voltage */
    data = VP880_SS_ACTIVE;
    VP_CALIBRATION(VpDevCtxType, pDevCtx,("Calibration: Setting Ch %d to State 0x%02X at time %d",
        channelId, data, pDevObj->timeStamp));
    VpMpiCmdWrapper(deviceId, ecVal, VP880_SYS_STATE_WRT, VP880_SYS_STATE_LEN,
        &data);

    /* Enable line control to access VBAT sense */
    VpMpiCmdWrapper(deviceId, ecVal, VP880_ICR3_WRT, VP880_ICR3_LEN, icr3);
    VP_CALIBRATION(VpDevCtxType, pDevCtx,("1. Calibration: Write ICR3 0x%02X 0x%02X 0x%02X 0x%02X Ch %d",
        icr3[0], icr3[1], icr3[2], icr3[3], channelId));

    /* Enable ADC */
    VpMpiCmdWrapper(deviceId, ecVal, VP880_ICR4_WRT, VP880_ICR4_LEN, icr4);
    VP_CALIBRATION(VpDevCtxType, pDevCtx,("Calibration: Write ICR4 0x%02X 0x%02X 0x%02X 0x%02X Ch %d",
        icr4[0], icr4[1], icr4[2], icr4[3], channelId));

    /* Set compression to Linear Mode and default AC Coefficients */
    data = VP880_LINEAR_CODEC;
    VpMpiCmdWrapper(deviceId, ecVal, VP880_CODEC_REG_WRT, VP880_CODEC_REG_LEN,
        &data);

    /* Cut TX/RX PCM and disable HPF */
    data = (VP880_CUT_TXPATH | VP880_CUT_RXPATH | VP880_HIGH_PASS_DIS);
    VpMpiCmdWrapper(deviceId, ecVal, VP880_OP_COND_WRT, VP880_OP_COND_LEN,
        &data);

    /* Set DISN = 0. */
    VpMpiCmdWrapper(deviceId, ecVal, VP880_DISN_WRT, VP880_DISN_LEN, disnVal);

    /* Wait at least 100ms before collecting data */
    pDevObj->devTimer[VP_DEV_TIMER_ABV_CAL] =
    MS_TO_TICKRATE(VP880_CAL_ABV_LONG,
        pDevObj->devProfileData.tickRate) | VP_ACTIVATE_TIMER;

    /* Advance state to measure ADC offset */
    pDevObj->calData.calState = VP880_CAL_STATE_CHANGE;
} /* end Vp880AbvInit */

/**
 * Vp880AbvInitAbs() -- ABS Only Function
 *  This function initiates a calibration operation for ABV associated with all
 * the lines of an ABS device. See VP-API reference guide for more information.

 * Preconditions:
 *  The device and line context must be created and initialized before calling
 * this function.
 *
 * Postconditions:
 *  This function generates an event upon completing the requested action.
 */
void
Vp880AbvInitAbs(
    VpDevCtxType *pDevCtx)
{
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpDeviceIdType deviceId = pDevObj->deviceId;
    uint8 disnVal[VP880_DISN_LEN] = {0x00};
    uint8 channelId, ecVal;
    uint8 data;
    uint8 swCal[VP880_BAT_CALIBRATION_LEN] = {0x00, 0x10};

    /*
     * Initialize and use to measure each channels offset and voltage using
     * same functions.
     */
    pDevObj->calData.abvData.passCnt = 0;

    VpMpiCmdWrapper(deviceId, pDevObj->ecVal, VP880_REGULATOR_PARAM_WRT,
        VP880_REGULATOR_PARAM_LEN, pDevObj->swParams);
    pDevObj->stateInt &= (uint32)(~(VP880_SWZ_DECAY_CMP | VP880_SWY_DECAY_CMP));

    /* Device Mode */
    VpMpiCmdWrapper(deviceId, VP880_EC_CH1, VP880_DEV_MODE_RD, VP880_DEV_MODE_LEN,
        &data);
    data &= ~(VP880_DEV_MODE_TEST_DATA);
    VpMpiCmdWrapper(deviceId, VP880_EC_CH1, VP880_DEV_MODE_WRT, VP880_DEV_MODE_LEN,
        &data);

    /* Channel specific registers to restore at end of calibration */
    for (channelId = 0; channelId < pDevObj->staticInfo.maxChannels; channelId++) {
        ecVal = (channelId == 0) ? VP880_EC_CH1 : VP880_EC_CH2;

        /* Clear existing correction factors */
        VpMpiCmdWrapper(deviceId, ecVal, VP880_BAT_CALIBRATION_WRT,
            VP880_BAT_CALIBRATION_LEN, swCal);

        VpMpiCmdWrapper(deviceId, ecVal, VP880_SYS_STATE_RD, VP880_SYS_STATE_LEN,
            &pDevObj->calData.abvData.sysState[channelId][0]);

        VpMpiCmdWrapper(deviceId, ecVal, VP880_DISN_RD, VP880_DISN_LEN,
            &pDevObj->calData.abvData.disnVal[channelId][0]);

        VpMpiCmdWrapper(deviceId, ecVal, VP880_OP_FUNC_RD, VP880_OP_FUNC_LEN,
            &pDevObj->calData.abvData.opFunc[channelId][0]);

        VpMpiCmdWrapper(deviceId, ecVal, VP880_OP_COND_RD, VP880_OP_COND_LEN,
            &pDevObj->calData.abvData.opCond[channelId][0]);

        VpMpiCmdWrapper(deviceId, ecVal, VP880_CONV_CFG_RD, VP880_CONV_CFG_LEN,
            &pDevObj->calData.abvData.converterCfg[channelId][0]);
    }

    /* Set for Linear Mode and disable AC Coefficients */
    data = VP880_LINEAR_CODEC;
    VpMpiCmdWrapper(deviceId, (VP880_EC_CH1 | VP880_EC_CH2), VP880_OP_FUNC_WRT,
        VP880_OP_FUNC_LEN, &data);

    /* Cut TX/RX PCM and disable HPF */
    data = (VP880_CUT_TXPATH | VP880_CUT_RXPATH | VP880_HIGH_PASS_DIS);
    VpMpiCmdWrapper(deviceId, (VP880_EC_CH1 | VP880_EC_CH2), VP880_OP_COND_WRT,
        VP880_OP_COND_LEN,  &data);

    /* Set DISN = 0. */
    VpMpiCmdWrapper(deviceId, (VP880_EC_CH1 | VP880_EC_CH2), VP880_DISN_WRT,
        VP880_DISN_LEN, disnVal);

    /*
     * Disable Switchers and wait for discharge. Typically, 2.5 seconds for a
     * warm re-cal
     */
    data = (VP880_SWY_OFF | VP880_SWZ_OFF);
    VpMpiCmdWrapper(deviceId, (VP880_EC_CH1 | VP880_EC_CH2), VP880_REGULATOR_CTRL_WRT,
        VP880_REGULATOR_CTRL_LEN, &data);

    /* Force sink supply current to reduce voltage. */
    data = VP880_SS_ACTIVE;
    VP_CALIBRATION(VpDevCtxType, pDevCtx, ("Calibration: Setting BOTH LINES to State 0x%02X at time %d",
        data, pDevObj->timeStamp));
    VpMpiCmdWrapper(deviceId, (VP880_EC_CH1 | VP880_EC_CH2), VP880_SYS_STATE_WRT,
        VP880_SYS_STATE_LEN, &data);

    pDevObj->devTimer[VP_DEV_TIMER_ABV_CAL] =
    MS_TO_TICKRATE(VP880_CAL_ABV_ABS_INIT,
        pDevObj->devProfileData.tickRate) | VP_ACTIVATE_TIMER;

    /* Advance state to measure ADC offset */
    pDevObj->calData.calState = VP880_CAL_STATE_CHANGE;
} /* end Vp880AbvInitAbs */

/**
 * Vp880AbvStateChange () -- Tracker (ABV Cal) only function
 *  This function changes the line state and sets the converter configuration
 * in order to give time for the converter to stabilize before taking the first
 * set of data.
 *
 * Preconditions:
 *  The device and line context must be created and initialized before calling
 * this function.
 *
 * Postconditions:
 */
void
Vp880AbvStateChange(
    VpDevCtxType *pDevCtx)
{
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpDeviceIdType deviceId = pDevObj->deviceId;
    uint8 data, ecVal;
    uint8 converterCfg[VP880_CONV_CFG_LEN] = {VP880_SWITCHER_Y};

    if ((pDevObj->stateInt & VP880_IS_SINGLE_CHANNEL)
     || (pDevObj->stateInt & VP880_LINE1_IS_FXO)) {
        ecVal = VP880_EC_CH1;
    } else {
        ecVal = VP880_EC_CH1 | VP880_EC_CH2;
    }

    /* Disconnect to enable switcher circuitry, codec activated */
    data = (VP880_SS_DISCONNECT | VP880_SS_ACTIVATE_MASK);
    VP_CALIBRATION(VpDevCtxType, pDevCtx,("Calibration: Setting ALL_LINES to State 0x%02X at time %d",
        data, pDevObj->timeStamp));
    VpMpiCmdWrapper(deviceId, ecVal, VP880_SYS_STATE_WRT, VP880_SYS_STATE_LEN,
        &data);

    /* Don't care about the data, just force the converter configuration */
    VpMpiCmdWrapper(deviceId, VP880_EC_CH1, VP880_CONV_CFG_WRT, VP880_CONV_CFG_LEN,
        converterCfg);

    /* Allow the converter to stabilize */
    pDevObj->devTimer[VP_DEV_TIMER_ABV_CAL] =
        MS_TO_TICKRATE(VP880_CAL_ABV_LONG, /*VP880_CAL_ABV_DELAY,*/
        pDevObj->devProfileData.tickRate) | VP_ACTIVATE_TIMER;

    pDevObj->calData.calState =  VP880_CAL_ADC;
} /* end Vp880AbvStateChange */

/**
 * Vp880AbvStateChangeAbs() -- ABS Only Function
 *  This function TBD
 *
 * Preconditions:
 *  The device and line context must be created and initialized before calling
 * this function.
 *
 * Postconditions:
 */
void
Vp880AbvStateChangeAbs(
    VpDevCtxType *pDevCtx)
{
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpDeviceIdType deviceId = pDevObj->deviceId;

    uint8 converterCfg[VP880_CONV_CFG_LEN];

    /* First iteration VP880_SWITCHER_Y -> Ch1 AND VP880_SWITCHER_Z -> Ch2 */
    /* Second iteration VP880_SWITCHER_Y -> Ch2 AND VP880_SWITCHER_Z -> Ch1 */
    /* Third iteration VP880_XBR -> Ch1 AND VP880_XBR -> Ch2 */
    if ((pDevObj->calData.abvData.passCnt & 0xC0) == 0x00) {
        /* Don't care about the data, just force the converter configuration */
        converterCfg[0] = VP880_SWITCHER_Y;
        VpMpiCmdWrapper(deviceId, VP880_EC_CH1, VP880_CONV_CFG_WRT, VP880_CONV_CFG_LEN,
            converterCfg);

        converterCfg[0] = VP880_SWITCHER_Z;
        VpMpiCmdWrapper(deviceId, VP880_EC_CH2, VP880_CONV_CFG_WRT, VP880_CONV_CFG_LEN,
            converterCfg);
    } else if ((pDevObj->calData.abvData.passCnt & 0xC0) == 0x40) {
        /* Don't care about the data, just force the converter configuration */
        converterCfg[0] = VP880_SWITCHER_Y;
        VpMpiCmdWrapper(deviceId, VP880_EC_CH2, VP880_CONV_CFG_WRT, VP880_CONV_CFG_LEN,
            converterCfg);

        converterCfg[0] = VP880_SWITCHER_Z;
        VpMpiCmdWrapper(deviceId, VP880_EC_CH1, VP880_CONV_CFG_WRT, VP880_CONV_CFG_LEN,
            converterCfg);
    } else {    /* ((pDevObj->calData.abvData.passCnt & 0xC0) == 0x80) */
        /* Don't care about the data, just force the converter configuration */
        converterCfg[0] = VP880_XBR;
        VpMpiCmdWrapper(deviceId, VP880_EC_CH1, VP880_CONV_CFG_WRT, VP880_CONV_CFG_LEN,
            converterCfg);

        VpMpiCmdWrapper(deviceId, VP880_EC_CH2, VP880_CONV_CFG_WRT, VP880_CONV_CFG_LEN,
            converterCfg);
    }

    /* Allow the converter to stabilize */
    pDevObj->devTimer[VP_DEV_TIMER_ABV_CAL] = MS_TO_TICKRATE(VP880_CAL_ABV_LONG,
        pDevObj->devProfileData.tickRate) | VP_ACTIVATE_TIMER;

    pDevObj->calData.calState =  VP880_CAL_ADC;
} /* end Vp880AbvStateChangeAbs */

/**
 * Vp880AbvSetAdc() -- Tracker (ABV Cal) only function
 *  This function set the converter to read the right pcm set the right state
 * machine
 *
 * Preconditions:
 *  The device and line context must be created and initialized before calling
 * this function.
 *
 * Postconditions:
 */
void
Vp880AbvSetAdc(
    VpDevCtxType *pDevCtx)
{
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpDeviceIdType deviceId = pDevObj->deviceId;
    uint8 ecVal;
    uint8 swYZ[VP880_REGULATOR_PARAM_LEN];
    bool abvSetAdcDone = FALSE;
    uint8 converterCfg[VP880_CONV_CFG_LEN] = {VP880_SWITCHER_Z};

    if ((pDevObj->stateInt & VP880_IS_SINGLE_CHANNEL)
     || (pDevObj->stateInt & VP880_LINE1_IS_FXO)) {
        ecVal = VP880_EC_CH1;
    } else {
        ecVal = VP880_EC_CH1 | VP880_EC_CH2;
    }

    /* Now we'll switch to channel specific measurements */
    /* Read SWY from first channel, SWZ from second channel */
    if (pDevObj->calData.abvData.passCnt == 0) {
        pDevObj->vp880SysCalData.swyOffset[0] =
            Vp880AdcSettling(deviceId, VP880_EC_CH1, VP880_SWITCHER_Y);

        /* This is done to be compatible with VVA P1.3.0 */
        pDevObj->calData.abvData.swyOffset[0] =
            pDevObj->vp880SysCalData.swyOffset[0];

        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("Chan 0 Offset (10mV): SWY %d",
            ((int16)(pDevObj->vp880SysCalData.swyOffset[0] * VP880_V_PCM_LSB / VP880_V_SCALE))));

        if (ecVal == VP880_EC_CH1) {
            abvSetAdcDone = TRUE;
        } else {
            VpMpiCmdWrapper(deviceId, VP880_EC_CH2, VP880_CONV_CFG_WRT,
                VP880_CONV_CFG_LEN, converterCfg);

            /* Wait for converter data to settle */
            pDevObj->devTimer[VP_DEV_TIMER_ABV_CAL] =
                MS_TO_TICKRATE(VP880_CAL_ABV_DELAY,
                    pDevObj->devProfileData.tickRate) | VP_ACTIVATE_TIMER;

            pDevObj->calData.abvData.passCnt++;
        }
    } else if (pDevObj->calData.abvData.passCnt == 1) {
        pDevObj->vp880SysCalData.swzOffset[1] =
            Vp880AdcSettling(deviceId, VP880_EC_CH2, VP880_SWITCHER_Z);

        /* This is done to be compatible with VVA P1.3.0 */
        pDevObj->calData.abvData.swzOffset[1] =
            pDevObj->vp880SysCalData.swzOffset[1];

        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("Chan 1 Offset (10mV): SWZ %d",
            ((int16)(pDevObj->vp880SysCalData.swzOffset[1] * VP880_V_PCM_LSB / VP880_V_SCALE))));

        converterCfg[0] = VP880_SWITCHER_Y;
        VpMpiCmdWrapper(deviceId, VP880_EC_CH1, VP880_CONV_CFG_WRT, VP880_CONV_CFG_LEN,
            converterCfg);

        converterCfg[0] = VP880_SWITCHER_Z;
        VpMpiCmdWrapper(deviceId, VP880_EC_CH2, VP880_CONV_CFG_WRT, VP880_CONV_CFG_LEN,
            converterCfg);

        /* Wait for converter data to settle */
        pDevObj->devTimer[VP_DEV_TIMER_ABV_CAL] = MS_TO_TICKRATE(VP880_CAL_ABV_DELAY,
            pDevObj->devProfileData.tickRate) | VP_ACTIVATE_TIMER;

        pDevObj->calData.abvData.passCnt++;
    } else {
        pDevObj->vp880SysCalData.swzOffset[0] =
            Vp880AdcSettling(deviceId, VP880_EC_CH1, VP880_SWITCHER_Z);

        /* This is done to be compatible with VVA P1.3.0 */
        pDevObj->calData.abvData.swzOffset[0] =
            pDevObj->vp880SysCalData.swzOffset[0];

        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("Chan 0 Offset: SWZ %d",
            ((pDevObj->vp880SysCalData.swzOffset[0] * VP880_V_PCM_LSB) / 10000)));

        pDevObj->vp880SysCalData.swyOffset[1] =
            Vp880AdcSettling(deviceId, VP880_EC_CH2, VP880_SWITCHER_Y);

        /* This is done to be compatible with VVA P1.3.0 */
        pDevObj->calData.abvData.swyOffset[1] =
            pDevObj->vp880SysCalData.swyOffset[1];

        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("Chan 1 Offset: SWY %d",
            ((pDevObj->vp880SysCalData.swyOffset[1] * VP880_V_PCM_LSB) / 10000)));

        abvSetAdcDone = TRUE;
    }

    if (abvSetAdcDone == TRUE) {
        /*
         * Restore internal switcher parameters to take voltage measurement. This
         * is a global register so it doesn't matter which EC value is used.
         */
        VpMpiCmdWrapper(deviceId, VP880_EC_CH1, VP880_INT_SWREG_PARAM_WRT,
            VP880_INT_SWREG_PARAM_LEN, pDevObj->calData.abvData.isrpData);

        pDevObj->calData.calSet = pDevObj->swParams[VP880_SWREG_RING_V_BYTE];

        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("Vp880AbvInit: ABV Set Value Target %d",
            ((pDevObj->calData.calSet * 5) + 5)));

        /*
         * Copy the Ringing Voltage to the Floor Voltage, everything else
         * directly from the device profile
         */
        swYZ[0] = pDevObj->swParams[0];

        swYZ[VP880_SWY_LOCATION] =
            (pDevObj->swParams[VP880_SWZ_LOCATION] & VP880_VOLTAGE_MASK);
        swYZ[VP880_SWY_LOCATION] |=
            (pDevObj->swParams[VP880_SWY_LOCATION] & ~VP880_VOLTAGE_MASK);

        swYZ[2] = pDevObj->swParams[2];

        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("Vp880AbvInit: swYZ: 0x%02X 0x%02X 0x%02X",
            swYZ[0], swYZ[1], swYZ[2]));

        /* Program switcher floor voltage to target ringing voltage */
        VpMpiCmdWrapper(deviceId, VP880_EC_CH1, VP880_REGULATOR_PARAM_WRT,
            VP880_REGULATOR_PARAM_LEN, swYZ);

        /* Things will take time to settle after programming switcher. */
        pDevObj->devTimer[VP_DEV_TIMER_ABV_CAL] = MS_TO_TICKRATE(VP880_CAL_ABV_LONG,
            pDevObj->devProfileData.tickRate) | VP_ACTIVATE_TIMER;

        pDevObj->calData.calState = VP880_CAL_MEASURE;
    }
} /* end Vp880AbvSetAdc  */

/**
 * Vp880AbvSetAdcAbs() -- ABS Only Function
 *  This function set the converter to read the right pcm set the right state
 * machine
 *
 * Preconditions:
 *  The device and line context must be created and initialized before calling
 * this function.
 *
 * Postconditions:
 */
void
Vp880AbvSetAdcAbs(
    VpDevCtxType *pDevCtx)
{
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpDeviceIdType deviceId = pDevObj->deviceId;
    int16 sw1OffsetNew, sw2OffsetNew;
    uint8 sw1, sw2;
    int16 *sw1Offset, *sw2Offset;

    if ((pDevObj->calData.abvData.passCnt & 0xC0) == 0x00) {
        sw1Offset = (int16*)&pDevObj->vp880SysCalData.swyOffset[0];
        sw2Offset = (int16*)&pDevObj->vp880SysCalData.swzOffset[1];
        sw1 = VP880_SWITCHER_Y;
        sw2 = VP880_SWITCHER_Z;
    } else if ((pDevObj->calData.abvData.passCnt & 0xC0) == 0x40) {
        sw1Offset = (int16*)&pDevObj->vp880SysCalData.swzOffset[0];
        sw2Offset = (int16*)&pDevObj->vp880SysCalData.swyOffset[1];
        sw1 = VP880_SWITCHER_Z;
        sw2 = VP880_SWITCHER_Y;
    } else { /* ((pDevObj->calData.abvData.passCnt & 0xC0) == 0x80) */
        sw1Offset = (int16*)&pDevObj->vp880SysCalData.swxbOffset[0];
        sw2Offset = (int16*)&pDevObj->vp880SysCalData.swxbOffset[1];
        sw1 = VP880_XBR;
        sw2 = VP880_XBR;
    }

    /* Read SWY from first channel, SWZ from second channel */
    if ((pDevObj->calData.abvData.passCnt & 0x3F) == 0) {
        /*
         * Take first measurement, than increment the pass counter to track
         * how long it takes to settle. The time it takes will determine an
         * offset correction to the final value.
         */
        *sw1Offset = Vp880AdcSettling(deviceId, VP880_EC_CH1, sw1);

        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("1. Chan 0 Offset, Sequence %d: %d",
            pDevObj->calData.abvData.passCnt >> 6, ((*sw1Offset * VP880_V_PCM_LSB) / 10000)));

        *sw2Offset = Vp880AdcSettling(deviceId, VP880_EC_CH2, sw2);

        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("1. Chan 1 Offset, Sequence %d: %d",
            pDevObj->calData.abvData.passCnt >> 6, ((*sw2Offset * VP880_V_PCM_LSB) / 10000)));

        pDevObj->calData.abvData.passCnt++;
    } else {
        sw1OffsetNew = Vp880AdcSettling(deviceId, VP880_EC_CH1, sw1);

        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("%d. Chan 0 Offset, Sequence %d: %d",
            (pDevObj->calData.abvData.passCnt & 0x3F) + 1,
            pDevObj->calData.abvData.passCnt >> 6, ((sw1OffsetNew * VP880_V_PCM_LSB) / 10000)));

        sw2OffsetNew = Vp880AdcSettling(deviceId, VP880_EC_CH2, sw2);

        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("%d. Chan 1 Offset, Sequence %d: %d",
            (pDevObj->calData.abvData.passCnt & 0x3F) + 1,
            pDevObj->calData.abvData.passCnt >> 6, ((sw2OffsetNew * VP880_V_PCM_LSB) / 10000)));

        /* Repeat until delta between two samples is less than max error */
        if (ABS(sw1OffsetNew - *sw1Offset) <= VP880_CAL_ABV_SAMPLE_ERR) {
            pDevObj->stateInt |= VP880_SWY_DECAY_CMP;
        }

        if (ABS(sw2OffsetNew - *sw2Offset) <= VP880_CAL_ABV_SAMPLE_ERR) {
            pDevObj->stateInt |= VP880_SWZ_DECAY_CMP;
        }

        if ((pDevObj->stateInt & (VP880_SWZ_DECAY_CMP | VP880_SWY_DECAY_CMP)) ==
            (VP880_SWZ_DECAY_CMP | VP880_SWY_DECAY_CMP)) {

            if ((pDevObj->calData.abvData.passCnt & 0xC0) == 0x00) {
                pDevObj->calData.abvData.passCnt = 0x40;
                pDevObj->calData.calState = VP880_CAL_STATE_CHANGE;
            } else if ((pDevObj->calData.abvData.passCnt & 0xC0) == 0x40) {
                pDevObj->calData.abvData.passCnt = 0x80;
                pDevObj->calData.calState = VP880_CAL_STATE_CHANGE;
            } else {
                VP_CALIBRATION(VpDevCtxType, pDevCtx, ("-- Offset VP880_SWITCHER_Y Ch0 = %d",
                    pDevObj->vp880SysCalData.swyOffset[0]));
                VP_CALIBRATION(VpDevCtxType, pDevCtx, ("-- Offset VP880_SWITCHER_Y Ch1 = %d",
                    pDevObj->vp880SysCalData.swyOffset[1]));
                VP_CALIBRATION(VpDevCtxType, pDevCtx, ("-- Offset VP880_SWITCHER_Z Ch0 = %d",
                    pDevObj->vp880SysCalData.swzOffset[0]));
                VP_CALIBRATION(VpDevCtxType, pDevCtx, ("-- Offset VP880_SWITCHER_Z Ch1 = %d",
                    pDevObj->vp880SysCalData.swzOffset[1]));
                VP_CALIBRATION(VpDevCtxType, pDevCtx, ("-- Offset VP880_XBR Ch0        = %d",
                    pDevObj->vp880SysCalData.swxbOffset[0]));
                VP_CALIBRATION(VpDevCtxType, pDevCtx, ("-- Offset VP880_XBR Ch1        = %d",
                    pDevObj->vp880SysCalData.swxbOffset[1]));
                pDevObj->calData.calState = VP880_CAL_MEASURE;
            }
        } else {
            pDevObj->calData.abvData.passCnt++;

            /*
             * Error is exceeded between consecutive values. Terminate the loop
             * if we reached max number of iterations.
             */
            if ((pDevObj->calData.abvData.passCnt & 0x3F) >= VP880_CAL_ABV_SAMPLE_MAX) {
                if ((pDevObj->calData.abvData.passCnt & 0xC0) == 0x00) {
                    pDevObj->calData.abvData.passCnt = 0x40;
                    pDevObj->calData.calState = VP880_CAL_STATE_CHANGE;
                } else if ((pDevObj->calData.abvData.passCnt & 0xC0) == 0x40) {
                    pDevObj->calData.abvData.passCnt = 0x80;
                    pDevObj->calData.calState = VP880_CAL_STATE_CHANGE;
                } else {
                    pDevObj->calData.calState = VP880_CAL_MEASURE;
                }
            }
        }
        *sw1Offset = sw1OffsetNew;
        *sw2Offset = sw2OffsetNew;
    }

    if (pDevObj->calData.calState == VP880_CAL_MEASURE) {
        uint8 data = (VP880_SWY_LP | VP880_SWZ_LP);

        /* Check if we were able to collapse the battery (>7.3V) */
        /* if not set the calibration factors to 0 */
        if (ABS(pDevObj->vp880SysCalData.swyOffset[0]) > 1370) {
            VP_CALIBRATION(VpDevCtxType, pDevCtx, ("Vp880AbvSetAdcAbs(): Impossible to collapse the battery, %d",
                ABS(pDevObj->vp880SysCalData.swyOffset[0])));
            pDevObj->vp880SysCalData.swyOffset[0] = 0;
            pDevObj->vp880SysCalData.swyOffset[1] = 0;
            pDevObj->vp880SysCalData.swzOffset[0] = 0;
            pDevObj->vp880SysCalData.swzOffset[1] = 0;
            pDevObj->vp880SysCalData.swxbOffset[0] = 0;
            pDevObj->vp880SysCalData.swxbOffset[1] = 0;
        }

        pDevObj->calData.abvData.initChange = TRUE;

        /* Re-enable the switchers for target measurement */
        VpMpiCmdWrapper(deviceId, (VP880_EC_CH1 | VP880_EC_CH2),
            VP880_REGULATOR_CTRL_WRT, VP880_REGULATOR_CTRL_LEN, &data);
    }

    /* Things will take time to settle after programming switcher. */
    pDevObj->devTimer[VP_DEV_TIMER_ABV_CAL] = MS_TO_TICKRATE(VP880_CAL_ABV_SAMPLE,
        pDevObj->devProfileData.tickRate) | VP_ACTIVATE_TIMER;

} /* end Vp880AbvSetAdcAbs */

/**
 * Vp880AbvMeasure() -- Tracker (ABV Cal) only function
 *  This function read switcher value and compare with the value read from the
 * pcm data if the value is bigger than 1.25v this function will make a
 * correction  voltage.
 *
 * Preconditions:
 *  The device and line context must be created and initialized before calling
 * this function.
 *
 * Postconditions:
 *  Battery calibration registers are adjusted. Channel specific registers are
 * restored.
 */
bool
Vp880AbvMeasure(
    VpDevCtxType *pDevCtx)
{
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpDeviceIdType deviceId = pDevObj->deviceId;
    uint8 ecVal;
    int32 abvError, abvTarget;
    uint8 channelId;

    if ((pDevObj->stateInt & VP880_IS_SINGLE_CHANNEL)
     || (pDevObj->stateInt & VP880_LINE1_IS_FXO)) {
        ecVal = VP880_EC_CH1;
    } else {
        ecVal = VP880_EC_CH1 | VP880_EC_CH2;
    }

    /* Now we'll switch to channel specific measurements */
    /* Read SWY from first channel */
    pDevObj->calData.abvData.swyVolt[0] =
        Vp880AdcSettling(deviceId, VP880_EC_CH1, VP880_SWITCHER_Y);

    VP_CALIBRATION(VpDevCtxType, pDevCtx, ("Chan 0 Voltage: SWY %d",
        ((pDevObj->calData.abvData.swyVolt[0] * VP880_V_PCM_LSB) / 10000)));

    /* Now have all data necessary to compute error and adjust channel 0 */
    abvTarget = (pDevObj->calData.calSet * 5) + 5;   /* Gets it to V scale */
    abvTarget *= 1000;
    abvTarget *= 1000;
    abvTarget /= VP880_V_PCM_LSB;   /* Now we're scaled to the PCM data */

    abvError = abvTarget -
        (pDevObj->calData.abvData.swyVolt[0]
       - pDevObj->vp880SysCalData.swyOffset[0]
       - 41);   /* 41 = ~300mV at 7.324mV scale */

    pDevObj->vp880SysCalData.abvError[0] = (((int16)abvError * VP880_V_PCM_LSB) / 10000);

    VP_CALIBRATION(VpDevCtxType, pDevCtx, ("1. Chan 0 Voltage Error: SWY %d Target Converted %d",
        pDevObj->vp880SysCalData.abvError[0], (((int16)abvTarget * VP880_V_PCM_LSB) / 10000)));

    /* Write the correction value to CH1 register. Steps in 1.25V increment */
    Vp880BatteryCalAdjust(pDevObj, VP880_EC_CH1);

    if (ecVal == (VP880_EC_CH1 | VP880_EC_CH2)) {
        /* Read SWZ voltages from second channel */
        pDevObj->calData.abvData.swzVolt[1] =
            Vp880AdcSettling(deviceId, VP880_EC_CH2, VP880_SWITCHER_Z);

        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("Chan 1 Voltage: SWZ %d",
            ((pDevObj->calData.abvData.swzVolt[1] * VP880_V_PCM_LSB) / 10000)));

        abvError = abvTarget -
            (pDevObj->calData.abvData.swzVolt[1]
           - pDevObj->vp880SysCalData.swzOffset[1]
           - 41);   /* 41 = ~300mV at 7.324mV scale */

        pDevObj->vp880SysCalData.abvError[1] = (((int16)abvError * VP880_V_PCM_LSB) / 10000);

        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("Chan 1 (SWZ): Offset %d Voltage %d Error Raw %d Error Converted %d",
            pDevObj->vp880SysCalData.swzOffset[1],
            pDevObj->calData.abvData.swzVolt[1],
            (int16)abvError,
            pDevObj->vp880SysCalData.abvError[1]));

        /* Write the correction value to CH2 register. Steps in 1.25V increment */
        Vp880BatteryCalAdjust(pDevObj, VP880_EC_CH2);
    }

    /* Restore Switching Regulator Parameters */
    VpMpiCmdWrapper(deviceId, ecVal, VP880_REGULATOR_PARAM_WRT,
        VP880_REGULATOR_PARAM_LEN, pDevObj->swParams);

    /* Channel specific registers to restore at end of calibration */
    for (channelId = 0; channelId < pDevObj->staticInfo.maxChannels; channelId++) {
        ecVal = (channelId == 0) ? VP880_EC_CH1 : VP880_EC_CH2;

        VpMpiCmdWrapper(deviceId, ecVal, VP880_ICR2_WRT,
            VP880_ICR2_LEN, &pDevObj->calData.abvData.icr2[channelId][0]);

        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("Calibration Restore: ICR2 0x%02X 0x%02X 0x%02X 0x%02X Ch %d",
            pDevObj->calData.abvData.icr2[channelId][0],
            pDevObj->calData.abvData.icr2[channelId][1],
            pDevObj->calData.abvData.icr2[channelId][2],
            pDevObj->calData.abvData.icr2[channelId][3],
            channelId));

        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("Calibration Restore: ICR3 0x%02X 0x%02X 0x%02X 0x%02X Ch %d",
            pDevObj->calData.abvData.icr3[channelId][0],
            pDevObj->calData.abvData.icr3[channelId][1],
            pDevObj->calData.abvData.icr3[channelId][2],
            pDevObj->calData.abvData.icr3[channelId][3],
            channelId));

        VpMpiCmdWrapper(deviceId, ecVal, VP880_ICR3_WRT,
            VP880_ICR3_LEN,  &pDevObj->calData.abvData.icr3[channelId][0]);

        VpMpiCmdWrapper(deviceId, ecVal, VP880_ICR4_WRT,
            VP880_ICR4_LEN,  &pDevObj->calData.abvData.icr4[channelId][0]);

        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("Calibration Restore: ICR4 0x%02X 0x%02X 0x%02X 0x%02X Ch %d",
            pDevObj->calData.abvData.icr4[channelId][0],
            pDevObj->calData.abvData.icr4[channelId][1],
            pDevObj->calData.abvData.icr4[channelId][2],
            pDevObj->calData.abvData.icr4[channelId][3],
            channelId));

        VpMpiCmdWrapper(deviceId, ecVal, VP880_DISN_WRT, VP880_DISN_LEN,
            &pDevObj->calData.abvData.disnVal[channelId][0]);
    }

    pDevObj->calData.calState = VP880_CAL_DONE;

    /* Things will take time to settle. */
    pDevObj->devTimer[VP_DEV_TIMER_ABV_CAL] = MS_TO_TICKRATE(VP880_CAL_ABV_LONG,
        pDevObj->devProfileData.tickRate) | VP_ACTIVATE_TIMER;

    return TRUE;
} /*end Vp880AbvMeasure */

/**
 * Vp880BatteryCalAdjust() -- Tracker and ABS Function
 *  This function computes the battery calibration error and adjust the device
 * register and object content.
 *
 * Preconditions:
 *  The device and line context must be created and initialized before calling
 * this function.
 *
 * Postconditions:
 *  Battery calibration registers are adjusted. Device object is updated.
 */
void
Vp880BatteryCalAdjust(
    Vp880DeviceObjectType *pDevObj,
    uint8 ecVal)
{
    uint8 channelId = ((ecVal == VP880_EC_CH1) ? 0 : 1);
    VpDeviceIdType deviceId = pDevObj->deviceId;
    uint8 swCal[VP880_BAT_CALIBRATION_LEN];
    int16 abvError = pDevObj->vp880SysCalData.abvError[channelId];

    uint16 swCalError;

    VpMpiCmdWrapper(deviceId, ecVal, VP880_BAT_CALIBRATION_RD,
        VP880_BAT_CALIBRATION_LEN, swCal);

    swCal[0] &= ~(VP880_BAT_CAL_SWCAL_MASK);

    /* Conversion from 7.324mV to 1.25V */
    swCalError = (ABS(abvError) / 171);
    if (((ABS(abvError) + 85) /  171) > swCalError) {
        swCalError+=1;
    }
    swCalError = (swCalError > 3) ? 3 : swCalError;
    swCal[0] |= (swCalError << 3);

    /*
     * Positive error means voltage is too low (not negative enough).
     * Positive adjustment makes the battery voltage more negative.
     */
    swCal[0] |= (abvError > 0) ? 0 : VP880_BAT_CAL_SWCAL_SIGN;

    VP_CALIBRATION(None, VP_NULL, ("Ch %d: Battery Calibration Correction 0x%02X 0x%02X",
        channelId, swCal[0], swCal[1]));

    VpMpiCmdWrapper(deviceId, (pDevObj->ecVal | ecVal), VP880_BAT_CALIBRATION_WRT,
        VP880_BAT_CALIBRATION_LEN, swCal);

    pDevObj->calData.abvData.switcherAdjust[channelId][0] = swCal[0];
    pDevObj->calData.abvData.switcherAdjust[channelId][1] = swCal[1];
}

/**
 * Vp880AbvMeasureAbs() -- ABS Only Function
 *  This is the last functional step for ABV calibration on ABS devices. It
 * takes the SWY and SWZ measurements, computes the error, makes the adjustment,
 * and restores line registers to values prior to running calibration.
 *
 * Preconditions:
 *  The device and line context must be created and initialized before calling
 * this function.
 *
 * Postconditions:
 *  Battery calibration registers are adjusted. Channel specific registers are
 * restored.
 */
bool
Vp880AbvMeasureAbs(
    VpDevCtxType *pDevCtx)
{
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpDeviceIdType deviceId = pDevObj->deviceId;
    uint8 ecVal;
    uint8 channelId;

    if (pDevObj->calData.abvData.initChange == TRUE) {
        /* Make sure converters are configured correctly */
        uint8 converterCfg[VP880_CONV_CFG_LEN];
        uint8 data = (VP880_SWY_HP | VP880_SWZ_HP);

        /* Don't care about the data, just force the converter configuration */
        converterCfg[0] = VP880_SWITCHER_Y;
        VpMpiCmdWrapper(deviceId, VP880_EC_CH1, VP880_CONV_CFG_WRT, VP880_CONV_CFG_LEN,
            converterCfg);

        converterCfg[0] = VP880_SWITCHER_Z;
        VpMpiCmdWrapper(deviceId, VP880_EC_CH2, VP880_CONV_CFG_WRT, VP880_CONV_CFG_LEN,
            converterCfg);

        /* Re-adjust switchers for target power control */
        VpMpiCmdWrapper(deviceId, (VP880_EC_CH1 | VP880_EC_CH2),
            VP880_REGULATOR_CTRL_WRT, VP880_REGULATOR_CTRL_LEN, &data);

        pDevObj->calData.abvData.initChange = FALSE;
    } else {
        int16 targetVoltY, targetVoltZ;
        uint8 data;

        pDevObj->calData.abvData.swyVolt[0] =
            Vp880AdcSettling(deviceId, VP880_EC_CH1, VP880_SWITCHER_Y);

        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("Chan 0 Voltage: SWY %d",
            ((pDevObj->calData.abvData.swyVolt[0] * VP880_V_PCM_LSB) / 10000)));

        pDevObj->calData.abvData.swzVolt[0] =
            Vp880AdcSettling(deviceId, VP880_EC_CH2, VP880_SWITCHER_Z);

        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("Chan 1 Voltage: SWZ %d",
            ((pDevObj->calData.abvData.swzVolt[0] * VP880_V_PCM_LSB) / 10000)));

        /* Compute Errors and make corrections */
        targetVoltY = (pDevObj->swParams[VP880_SWY_LOCATION] & VP880_VOLTAGE_MASK);
        targetVoltZ = (pDevObj->swParams[VP880_SWZ_LOCATION] & VP880_VOLTAGE_MASK);

        Vp880AbvMakeAdjustment(pDevObj, &targetVoltY, &targetVoltZ);

        /* Channel specific registers to restore at end of calibration */
        for (channelId = 0;
             channelId < pDevObj->staticInfo.maxChannels;
             channelId++) {

            ecVal = (channelId == 0) ? VP880_EC_CH1 : VP880_EC_CH2;

            VP_CALIBRATION(VpDevCtxType, pDevCtx, ("Calibration Restore: Setting Ch %d to State 0x%02X at time %d",
                channelId, data, pDevObj->timeStamp));

            VpMpiCmdWrapper(deviceId, ecVal, VP880_SYS_STATE_WRT, VP880_SYS_STATE_LEN,
                &pDevObj->calData.abvData.sysState[channelId][0]);

            VpMpiCmdWrapper(deviceId, ecVal, VP880_DISN_WRT, VP880_DISN_LEN,
                &pDevObj->calData.abvData.disnVal[channelId][0]);

            VpMpiCmdWrapper(deviceId, ecVal, VP880_OP_FUNC_WRT, VP880_OP_FUNC_LEN,
                &pDevObj->calData.abvData.opFunc[channelId][0]);

            VpMpiCmdWrapper(deviceId, ecVal, VP880_OP_COND_WRT, VP880_OP_COND_LEN,
                &pDevObj->calData.abvData.opCond[channelId][0]);

            VpMpiCmdWrapper(deviceId, ecVal, VP880_CONV_CFG_WRT, VP880_CONV_CFG_LEN,
                &pDevObj->calData.abvData.converterCfg[channelId][0]);
        }

        /* Device Mode */
        VpMpiCmdWrapper(deviceId, VP880_EC_CH1, VP880_DEV_MODE_RD, VP880_DEV_MODE_LEN,
            &data);
        data |= VP880_DEV_MODE_TEST_DATA;
        VpMpiCmdWrapper(deviceId, VP880_EC_CH1, VP880_DEV_MODE_WRT, VP880_DEV_MODE_LEN,
            &data);

        pDevObj->calData.calState = VP880_CAL_DONE;
    }

    /* Things will take time to settle. */
    pDevObj->devTimer[VP_DEV_TIMER_ABV_CAL] = MS_TO_TICKRATE(VP880_CAL_ABV_SAMPLE,
        pDevObj->devProfileData.tickRate) | VP_ACTIVATE_TIMER;

    return TRUE;
} /* end Vp880AbvMeasureAbs */

/**
 * Vp880AbvMakeAdjustment() -- ABS Only Function
 *  This function computes the measured error of ABV voltage and uses some
 * logic based on decay time to determine the device correction.
 *
 * Preconditions:
 *  The device and line context must be created and initialized before calling
 * this function.
 *
 * Postconditions:
 *  Battery calibration registers are adjusted.
 */
void
Vp880AbvMakeAdjustment(
    Vp880DeviceObjectType *pDevObj,
    int16 *targetVoltY,
    int16 *targetVoltZ)
{
    VpDeviceIdType deviceId = pDevObj->deviceId;
    int32 abvError, abvTarget, errorScaled;
    uint8 swParams[VP880_REGULATOR_PARAM_LEN];
    uint8 data = (VP880_SWY_MP | VP880_SWZ_MP);
    uint8 channelId;
    uint8 swCal[VP880_BAT_CALIBRATION_LEN];
    uint16 swCalError;

    /*
     * Offset correction is 600mV if decay took a long time, -300mV if it
     * occurred rapidly.
     */
    int8 offsetCorrection =
        ((pDevObj->calData.abvData.passCnt & 0x3F) > 2) ? 82 : -41;

    VpMpiCmdWrapper(deviceId, pDevObj->ecVal, VP880_REGULATOR_PARAM_RD,
        VP880_REGULATOR_PARAM_LEN, swParams);

    for (channelId = 0;
         channelId < pDevObj->staticInfo.maxChannels;
         channelId++) {
        uint8 ecVal, swIndex;
        int16 fineVoltTarget, coarseVoltTarget;
        int16 measVolt, measOffset;

        if (channelId == 0) {
            ecVal = VP880_EC_CH1;
            fineVoltTarget = pDevObj->yVolt;
            measVolt = pDevObj->calData.abvData.swyVolt[0];
            measOffset = pDevObj->vp880SysCalData.swyOffset[0];
            coarseVoltTarget = *targetVoltY;
            swIndex = VP880_SWY_LOCATION;
        } else {
            ecVal = VP880_EC_CH2;
            fineVoltTarget = pDevObj->zVolt;
            measVolt = pDevObj->calData.abvData.swzVolt[0];
            measOffset = pDevObj->vp880SysCalData.swzOffset[0];
            coarseVoltTarget = *targetVoltZ;
            swIndex = VP880_SWZ_LOCATION;
        }

        if (fineVoltTarget) {
            abvTarget = fineVoltTarget;
        } else {
            abvTarget = (coarseVoltTarget * 5) + 5;   /* Gets it to V scale */
        }
        abvTarget *= 1000;
        abvTarget *= 1000;
        abvTarget /= VP880_V_PCM_LSB;   /* Now we're scaled to the PCM data */

        /* If "reload" is in progress, we're being told what the error is. */
        if (pDevObj->stateInt & VP880_CAL_RELOAD_REQ) {
            abvError = (pDevObj->vp880SysCalData.abvError[channelId] * 10000 / VP880_V_PCM_LSB);
        } else {
            abvError = abvTarget - (measVolt - measOffset - offsetCorrection);
            /* Save the computed total error used if/when parameters are changed. */
            pDevObj->vp880SysCalData.abvError[channelId] =
                (((int16)abvError * VP880_V_PCM_LSB) / 10000);
        }

        /*
         * Start adjustment assuming it's in +/-5V type range. This is readjusted if
         * a +/-5V coarse adjustment is made, but does not affect the previously
         * saved "total" error value.
         */
        errorScaled = pDevObj->vp880SysCalData.abvError[channelId];

        /* errorScaled = (((int16)abvError * VP880_V_PCM_LSB) / 10000); */
        VP_CALIBRATION(None, NULL, ("2. Chan %d Voltage Error: SW%s %d (10mV), Target Converted %d (10mV) Offset Correction %d",
            channelId,
            ((channelId == 0) ? "Y" : "Z"),
            (int16)errorScaled, (((int16)abvTarget * VP880_V_PCM_LSB) / 10000), offsetCorrection));

        /* If the error is more than 10V, we can't adjust this supply */
        if ((ABS(errorScaled) < 1000)) {
            /*
             * If the coarse error requires a decrease/increase, make sure the current
             * setting can go down/up by 1 step and adjust the error.
             */
            /* Positive Error means the Battery voltage is not negative enough */
            /* Negative Error means the Battery voltage is too negative */

            if (((errorScaled * 10) < -4375) &&
                (swParams[swIndex] & VP880_VOLTAGE_MASK)) {
                swParams[swIndex]--;
                abvError += 683;   /* Fixed PCM Value for 5V */
                errorScaled = (((int16)abvError * VP880_V_PCM_LSB) / 10000);
                VP_CALIBRATION(None, NULL, ("Adjusted Switcher %s Down to 0x%02X. New Error %d",
                    ((channelId == 0) ? "Y" : "Z"),
                    swParams[swIndex],
                    (int16)errorScaled));
            } else if (((errorScaled * 10) > 4375) &&
                       ((swParams[swIndex] & VP880_VOLTAGE_MASK) < VP880_VOLTAGE_MASK)) {
                swParams[swIndex]++;
                abvError -= 683;   /* Fixed PCM Value for 5V */
                errorScaled = (((int16)abvError * VP880_V_PCM_LSB) / 10000);
                VP_CALIBRATION(None, NULL, ("Adjusted Switcher Up to 0x%02X. New Error %d",
                    swParams[swIndex], (int16)errorScaled));
            } else {
                VP_CALIBRATION(None, NULL, ("No Switcher Adjustment"));
            }

            if ((ABS(errorScaled) * 10) < 4375) {
                /* Write the correction value to CH1 register. Steps in 1.25V increment */
                VpMpiCmdWrapper(deviceId, (pDevObj->ecVal | ecVal),
                    VP880_BAT_CALIBRATION_RD, VP880_BAT_CALIBRATION_LEN, swCal);
                swCal[0] &= ~(VP880_BAT_CAL_SWCAL_MASK);

                /* Conversion from 7.324mV to 1.25V */
                swCalError = (ABS(abvError) / 171);
                if (((ABS(abvError) + 85) /  171) > swCalError) {
                    swCalError+=1;
                }
                swCalError = (swCalError > 3) ? 3 : swCalError;
                swCal[0] |= (swCalError << 3);

                /*
                 * Positive error means voltage is too low (not negative enough). Positive
                 * adjustment makes the battery voltage more negative.
                 */
                swCal[0] |= (abvError > 0) ? 0 : VP880_BAT_CAL_SWCAL_SIGN;

                VP_CALIBRATION(None, NULL, ("Ch %d: Battery Calibration Correction 0x%02X 0x%02X",
                    channelId, swCal[0], swCal[1]));

                VpMpiCmdWrapper(deviceId, (pDevObj->ecVal | ecVal),
                    VP880_BAT_CALIBRATION_WRT, VP880_BAT_CALIBRATION_LEN, swCal);
                pDevObj->calData.abvData.switcherAdjust[channelId][0] = swCal[0];
                pDevObj->calData.abvData.switcherAdjust[channelId][1] = swCal[1];
            }
        } else {
            VP_CALIBRATION(None, NULL, ("Channel %d Cannot Control Switcher - Setting for Nominal Voltage",
                channelId));

            /*
             * Device is at nominal voltage IF the 1V parameter is not set. If
             * it is set, fineVoltage is always equal to or higher than the
             * coarse setting. So in 1V steps, it is always adjustable in the
             * channel correction register (never requires coarse adjustment).
             */
            if (fineVoltTarget) {
                /* channelAdjust in 10mV steps, same as 1.25V adjustments */
                int16 channelAdjust = 100*(fineVoltTarget - ((coarseVoltTarget * 5) + 5));

                /* Remainder to determine if round-up/down */
                int16 remainder = channelAdjust % 125;

                /* Get the rounded down step... */
                uint8 stepSize = (channelAdjust - remainder)/ 125;

                VP_CALIBRATION(None, NULL, ("Channel %d Adjust %d Remainder %d Step %d - Fine Voltage %d  Coarse Setting %d" ,
                    channelId, channelAdjust, remainder, stepSize, fineVoltTarget, coarseVoltTarget));

                /* Round as needed */
                stepSize += (remainder <= 62) ? 0 : 1;

                VP_CALIBRATION(None, NULL, ("Adjusted Step %d", stepSize));

                /* Write the correction value to CH1 register. Steps in 1.25V increment */
                VpMpiCmdWrapper(deviceId, (pDevObj->ecVal | ecVal),
                    VP880_BAT_CALIBRATION_RD, VP880_BAT_CALIBRATION_LEN, swCal);
                swCal[0] &= ~(VP880_BAT_CAL_SWCAL_MASK);
                swCal[0] |= ((stepSize << 3) & VP880_BAT_CAL_SWCAL_MASK);

                VP_CALIBRATION(None, NULL, ("Adjusting Regsiter to 0x%02X", swCal[0]));

                VpMpiCmdWrapper(deviceId, (pDevObj->ecVal | ecVal),
                    VP880_BAT_CALIBRATION_WRT, VP880_BAT_CALIBRATION_LEN, swCal);
            }

            pDevObj->calData.abvData.switcherAdjust[channelId][0] = 0;
            pDevObj->calData.abvData.switcherAdjust[channelId][1] = 0;
        }
    }

    VP_CALIBRATION(None, NULL, ("Writing 0x%02X 0x%02X 0x%02X to Switcher" ,
        swParams[0], swParams[1], swParams[2]));

    VpMpiCmdWrapper(deviceId, (pDevObj->ecVal | VP880_EC_CH1),
        VP880_REGULATOR_PARAM_WRT, VP880_REGULATOR_PARAM_LEN, swParams);

    /* Re-adjust switchers for target power control */
    VpMpiCmdWrapper(deviceId, (pDevObj->ecVal | VP880_EC_CH1 | VP880_EC_CH2),
        VP880_REGULATOR_CTRL_WRT, VP880_REGULATOR_CTRL_LEN, &data);
} /* end Vp880AbvMakeAdjustment */

/**
 * Vp880CalAbvAbsDev() -- ABS Only Function
 *  This function initiates a calibration operation for Absolute Switcher
 * circuits associated with all the lines of a device. See VP-API reference
*  guide for more information. SWYV SWZV are global for every Channels
 * Line must be in Disconnect state before to start the Calibration
 *
 * Preconditions:
 *  The device and line context must be created and initialized before calling
 * this function.
 *
 * Postconditions:
 *  This function generates an event upon completing the requested action.
 */
VpStatusType
Vp880CalAbvAbsDev(
     VpDevCtxType *pDevCtx)
{
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpDeviceIdType deviceId = pDevObj->deviceId;
    VpStatusType  status = VP_STATUS_SUCCESS;
    uint8 ecVal;
    uint8 fxsChannels;
    bool calCleanup = FALSE;

    ecVal = VP880_EC_CH1 | VP880_EC_CH2;
    fxsChannels = 2;

    if (pDevObj->calData.calState == VP880_CAL_INIT
        || pDevObj->calData.calState == VP880_CAL_EXIT) {
        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("Vp880CalAbvAbsDev:  - Setting to Vp880CalInit"));

        pDevObj->calData.calState = VP880_CAL_INIT;
    }

    switch(pDevObj->calData.calState) {
        case VP880_CAL_INIT:
            VP_CALIBRATION(VpDevCtxType, pDevCtx, ("Vp880CalAbv: - Running Vp880AbvInitAbs"));
            Vp880AbvInitAbs(pDevCtx);
            break;

        case VP880_CAL_ADC:
            VP_CALIBRATION(VpDevCtxType, pDevCtx, ("Vp880CalAbv: - Running Vp880AbvSetAdcAbs"));
            Vp880AbvSetAdcAbs(pDevCtx);
            break;

        case VP880_CAL_STATE_CHANGE:
            VP_CALIBRATION(VpDevCtxType, pDevCtx, ("Vp880CalAbv: - Running Vp880AbvStateChangeAbs"));
            Vp880AbvStateChangeAbs(pDevCtx);
            break;

        case VP880_CAL_MEASURE:
            VP_CALIBRATION(VpDevCtxType, pDevCtx, ("Vp880CalAbv - Running Vp880AbvMeasureAbs"));
            Vp880AbvMeasureAbs(pDevCtx);
            break;

        case VP880_CAL_DONE:
            VP_CALIBRATION(VpDevCtxType, pDevCtx, ("ABV Cal Done for ABS"));
            calCleanup = TRUE;
            pDevObj->responseData = VP_CAL_SUCCESS;
            break;

        case VP880_CAL_ERROR:
            /* Fall through intentional */
        default:
            VP_ERROR(VpDevCtxType, pDevCtx, ("Vp880CalAbvAbsDev: ERROR - Cal Done"));
            calCleanup = TRUE;
            status = VP_STATUS_FAILURE;
            pDevObj->responseData = VP_CAL_FAILURE;
            break;
    }

    if (calCleanup == TRUE) {
        /* Reset Line states */
        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("Vp880CalAbvAbsDev: Setting Ch 0 to State 0x%02X at time %d",
            pDevObj->calData.abvData.sysState[0][0], pDevObj->timeStamp));
        VP_CALIBRATION(VpDevCtxType, pDevCtx, ("Vp880CalAbvAbsDev: Setting Ch 1 to State 0x%02X at time %d",
            pDevObj->calData.abvData.sysState[1][0], pDevObj->timeStamp));

        VpMpiCmdWrapper(deviceId, VP880_EC_CH1, VP880_SYS_STATE_WRT, VP880_SYS_STATE_LEN,
            &pDevObj->calData.abvData.sysState[0][0]);
        VpMpiCmdWrapper(deviceId, VP880_EC_CH2, VP880_SYS_STATE_WRT, VP880_SYS_STATE_LEN,
            &pDevObj->calData.abvData.sysState[1][0]);

        pDevObj->status.state &= ~VP_DEV_ABV_CAL_ABS;

        if (status != VP_STATUS_FAILURE) {
            /* Move on to ABS Battery Switch Calibration */
            pDevObj->status.state |= VP_DEV_ABS_BAT_CAL;
            pDevObj->calState = ABS_DC_CAL_INIT_STATE;
            pDevObj->calData.calState = VP880_CAL_EXIT;

            pDevObj->devTimer[VP_DEV_TIMER_ABSCAL] =
                MS_TO_TICKS_ROUND_UP(1, pDevObj->devProfileData.tickRate) | VP_ACTIVATE_TIMER;
        } else {

            if (pDevObj->status.state & VP_DEV_INIT_IN_PROGRESS) {
                pDevObj->deviceEvents.response |= VP_DEV_EVID_DEV_INIT_CMP;
            } else {
                pDevObj->deviceEvents.response |= VP_EVID_CAL_CMP;
            }

            /* Restore device mode to test buffer if exists */
            if (pDevObj->staticInfo.rcnPcn[VP880_RCN_LOCATION] > VP880_REV_VC) {
                uint8 deviceMode[VP880_DEV_MODE_LEN];

                VpMpiCmdWrapper(deviceId, ecVal, VP880_DEV_MODE_RD,
                    VP880_DEV_MODE_LEN, deviceMode);
                deviceMode[0] |= VP880_DEV_MODE_TEST_DATA;
                VpMpiCmdWrapper(deviceId, ecVal, VP880_DEV_MODE_WRT,
                    VP880_DEV_MODE_LEN, deviceMode);
            }
            pDevObj->status.state &= ~(VP_DEV_INIT_IN_PROGRESS | VP_DEV_IN_CAL);
            pDevObj->calState = VP880_CAL_INIT;
        }
    }

    return status;
}

/**
 * Vp880CalInit()
 *  This function initiates a calibration operation for VOC
 * associated with all the lines of a device. See VP-API reference guide for
 * more information.

 * Preconditions:
 *  The device and line context must be created and initialized before calling
 * this function.
 *
 * Postconditions:
 *  This function generates an event upon completing the requested action.
 */
void
Vp880CalInit(
     VpDevCtxType *pDevCtx)
{
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    uint8 channelId = 0;
    uint8 ecVal[] = {VP880_EC_CH1, VP880_EC_CH2};
    VpDeviceIdType deviceId = pDevObj->deviceId;
    uint8 data;

    /* Set for Linear Mode and disable AC Coefficients */
    data = VP880_LINEAR_CODEC;
    VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_CODEC_REG_WRT, VP880_CODEC_REG_LEN,
        &data);

    /* Cut TX/RX PCM and disable HPF */
    data = (VP880_CUT_TXPATH | VP880_CUT_RXPATH | VP880_HIGH_PASS_DIS);
    VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_OP_COND_WRT, VP880_OP_COND_LEN,
        &data);

    /* Device Mode */
    VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_DEV_MODE_RD, VP880_DEV_MODE_LEN,
        &data);
    data &= ~(VP880_DEV_MODE_TEST_DATA);
    VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_DEV_MODE_WRT, VP880_DEV_MODE_LEN,
        &data);
}

/**
 * Vp880CalLineInit()
 *  This function initiates a calibration operation for VOC
 * associated with all the lines of a device. See VP-API reference guide for
 * more information.

 * Preconditions:
 *  The device and line context must be created and initialized before calling
 * this function.
 *
 * Postconditions:
 *  This function generates an event upon completing the requested action.
 */
void
Vp880CalLineInit(
     VpLineCtxType *pLineCtx)
{
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    Vp880LineObjectType *pLineObj = pLineCtx->pLineObj;
    uint8 channelId = pLineObj->channelId;
    uint8 ecVal[] = {VP880_EC_CH1, VP880_EC_CH2};
    VpDeviceIdType deviceId = pDevObj->deviceId;
    uint8 data;
    uint8 disnVal[VP880_DISN_LEN] = {0x00};

    /*
     * Clear flag to indicate Calibration is NOT done. Init calibration type
     * (i.e., what algorithm to start with) and state (step inside the starting
     * algorithm.
     */
    pLineObj->calLineData.calDone = FALSE;
    pLineObj->calLineData.calState = VP880_CAL_INIT;

     /* Save PCM For Linear Mode and disable AC Coefficients */
    VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_OP_FUNC_RD, VP880_OP_FUNC_LEN,
        &pLineObj->calLineData.codecReg);

    /* Set compression to Linear Mode */
    data = VP880_LINEAR_CODEC;
    VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_CODEC_REG_WRT, VP880_CODEC_REG_LEN,
        &data);

    /* Cut TX/RX PCM and disable HPF */
    data = (VP880_CUT_TXPATH | VP880_CUT_RXPATH | VP880_HIGH_PASS_DIS);
    VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_OP_COND_WRT, VP880_OP_COND_LEN,
        &data);

    VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_DISN_RD, VP880_DISN_LEN,
        pLineObj->calLineData.disnVal);
    VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_DISN_WRT, VP880_DISN_LEN, disnVal);

    /* Device Mode */
    VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_DEV_MODE_RD, VP880_DEV_MODE_LEN,
        &data);
    data &= ~(VP880_DEV_MODE_TEST_DATA);
    VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_DEV_MODE_WRT, VP880_DEV_MODE_LEN,
        &data);
}

/**
 * Vp880AdcSettling()
 *  This function read ADC/PCM and set the converter register
 *
 * Preconditions:
 *  The device and line context must be created and initialized before calling
 * this function.
 *
 * Postconditions:
 *  This function return the value pcm red.
 */
int16
Vp880AdcSettling(
    VpDeviceIdType deviceId,
    uint8 ecVal,
    uint8 adcConfig)
{
    uint8 xdataTemp[VP880_TX_PCM_DATA_LEN];
    int16 tempNew;
    uint8 devMode[VP880_DEV_MODE_LEN];

    VpMpiCmdWrapper(deviceId, ecVal, VP880_DEV_MODE_RD, VP880_DEV_MODE_LEN, devMode);

    /*
     * If the device mode was changed (by other channel), need to change it back
     * so data can be taken.
     */
    if (devMode[0] & VP880_DEV_MODE_TEST_DATA) {
        devMode[0] &= ~(VP880_DEV_MODE_TEST_DATA);
        VpMpiCmdWrapper(deviceId, ecVal, VP880_DEV_MODE_WRT, VP880_DEV_MODE_LEN,
            devMode);
    }

    /* read twice */
    VpMpiCmdWrapper(deviceId, ecVal, VP880_TX_PCM_DATA_RD, VP880_TX_PCM_DATA_LEN,
        xdataTemp);
    VpMpiCmdWrapper(deviceId, ecVal, VP880_TX_PCM_DATA_RD, VP880_TX_PCM_DATA_LEN,
        xdataTemp);
    tempNew = ((xdataTemp[0] << 8) | xdataTemp[1]);

    VP_CALIBRATION(None, NULL,("Vp880AdcSettling(adcConfig = 0x%02X): AdcPcm %d ecVal %d",
        adcConfig, tempNew, ecVal));
    return tempNew;
}

/**
 * Vp880CalLineInt()
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
Vp880CalLineInt(
    VpLineCtxType *pLineCtx)
{
    Vp880LineObjectType *pLineObj = pLineCtx->pLineObj;

    switch(pLineObj->lineState.calType) {
        case VP_CSLAC_CAL_VOC:
            VP_CALIBRATION(VpLineCtxType, pLineCtx,("Running VOC Calibration on line %d",
                pLineObj->channelId));
            Vp880CalVoc(pLineCtx);
            break;

        case VP_CSLAC_CAL_VAS:
            VP_CALIBRATION(VpLineCtxType, pLineCtx,("Running VAS Calibration on line %d",
                pLineObj->channelId));
            Vp880CalVas(pLineCtx);
            break;

        case VP_CSLAC_CAL_CLEANUP:
        default:
            pLineObj->lineState.calType = VP_CSLAC_CAL_NONE;
            pLineObj->lineEvents.response |= VP_EVID_CAL_CMP;
            pLineObj->calLineData.calDone = TRUE;
            pLineObj->status &= ~VP880_LINE_IN_CAL;
            break;
    }

    return VP_STATUS_SUCCESS;
}

/**
 * Vp880VocMeasure()
 *  This function read switcher value and compare with
 *  the value read from the pcm data if the value is bigger than 1.25v
 * this function will make a correction  voltage .
 *
 */
void
Vp880VocMeasure(
    VpLineCtxType *pLineCtx)
{
    Vp880LineObjectType *pLineObj = pLineCtx->pLineObj;
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpDeviceIdType deviceId = pDevObj->deviceId;
    uint8 ecVal[] = {VP880_EC_CH1, VP880_EC_CH2};
    uint8 channelId = pLineObj->channelId;
    int16 imtValue, ilgValue;
    uint8 sysState[VP880_SYS_STATE_LEN];
    uint8 dcCal[VP880_DC_CAL_REG_LEN];
    uint8 converterCfg[VP880_CONV_CFG_LEN];

    switch (pLineObj->calLineData.calState) {
        case VP880_CAL_IMT_OFFSET_SET:
            converterCfg[0] = VP880_METALLIC_DC_I;

            /* Force the converter change. Don't care about data yet. */
            VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_CONV_CFG_WRT,
                VP880_CONV_CFG_LEN, converterCfg);

            pLineObj->calLineData.calState = VP880_CAL_IMT_OFFSET_READ;
            pLineObj->lineTimers.timers.timer[VP_LINE_CAL_LINE_TIMER] =
                MS_TO_TICKRATE(VP880_CAL_VOC_SHORT,
                    pDevObj->devProfileData.tickRate) | VP_ACTIVATE_TIMER;
            return;

        case VP880_CAL_IMT_OFFSET_READ:
            if (pLineObj->calLineData.reversePol == FALSE) {

                /* Read the metallic current. */
                imtValue = Vp880AdcSettling(deviceId, ecVal[channelId],
                    VP880_METALLIC_DC_I);

                VP_CALIBRATION(VpLineCtxType, pLineCtx, ("ILA OffsetNorm %d Channel %d",
                    imtValue, channelId));
                pDevObj->vp880SysCalData.ilaOffsetNorm[channelId] = imtValue;

                /* This is to be compatible with VVA P1.3.0 */
                pLineObj->calLineData.typeData.ilaData.ilaOffsetNorm = imtValue;
            } else {
                /*
                 * The error in reverse polarity is less than what can actually be
                 * measured. So it's better to ignore what is being read and assume
                 * the part is "perfect".
                 */
            }

            converterCfg[0] = VP880_LONGITUDINAL_DC_I;

            /* Force the converter change. Don't care about data yet. */
            VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_CONV_CFG_WRT,
                VP880_CONV_CFG_LEN, converterCfg);

            pLineObj->calLineData.calState = VP880_CAL_ILG_OFFSET_READ;
            pLineObj->lineTimers.timers.timer[VP_LINE_CAL_LINE_TIMER] =
                MS_TO_TICKRATE(VP880_CAL_VOC_SHORT,
                    pDevObj->devProfileData.tickRate) | VP_ACTIVATE_TIMER;
            return;

        case VP880_CAL_ILG_OFFSET_READ:
            if (pLineObj->calLineData.reversePol == FALSE) {

                /* Read the longitudinal current. */
                ilgValue = Vp880AdcSettling(deviceId, ecVal[channelId],
                    VP880_LONGITUDINAL_DC_I);

                VP_CALIBRATION(VpLineCtxType, pLineCtx, ("ILG OffsetNorm %d Channel %d",
                    ilgValue, channelId));
                pDevObj->vp880SysCalData.ilgOffsetNorm[channelId] = ilgValue;

                /* This is to be compatible with VVA P1.3.0 */
                pLineObj->calLineData.typeData.ilgData.ilgOffsetNorm = ilgValue;
            } else {
                /*
                 * The error in reverse polarity is less than what can actually be
                 * measured. So it's better to ignore what is being read and assume
                 * the part is "perfect".
                 */
            }
            break;

        default:
            break;
    }

    VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_SYS_STATE_RD,
        VP880_SYS_STATE_LEN, sysState);

    /* We're changing the polarity. So set flag as it will be. */
    if (sysState[0] & VP880_SS_POLARITY_MASK) {
        pLineObj->calLineData.reversePol = FALSE;
        sysState[0] &= ~VP880_SS_POLARITY_MASK;
    } else {
        pLineObj->calLineData.reversePol = TRUE;
        sysState[0] |= VP880_SS_POLARITY_MASK;
    }

    VP_CALIBRATION(VpLineCtxType, pLineCtx, ("Vp880VocMeasure: Setting Ch %d to State 0x%02X at time %d",
        channelId, sysState[0], pDevObj->timeStamp));

    Vp880UpdateBufferChanSel(pDevObj, channelId, sysState[0]);
    VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_SYS_STATE_WRT,
        VP880_SYS_STATE_LEN, sysState);

    /* Re-Disable TRDC Sense */
    VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_DC_CAL_REG_RD,
        VP880_DC_CAL_REG_LEN, dcCal);

    if (pDevObj->stateInt & VP880_IS_ABS) {
        dcCal[0] &= VP880_DC_CAL_ABS_MASK;
        dcCal[1] &= VP880_DCCAL_BAT_SW_HYST_MASK;
        dcCal[1] |= (VP880_C_RING_SNS_CUT | VP880_C_TIP_SNS_CUT);
    } else {
        dcCal[1] |= (VP880_C_RING_SNS_CUT | VP880_C_TIP_SNS_CUT | VP880_DCCAL_BAT_SW_HYST_5V);
    }
    dcCal[0] |= VP880_DC_CAL_OFFSET_OVRD;
    dcCal[1] |= VP880_DC_CAL_DIS_INPUT_OFFSET;

    VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_DC_CAL_REG_WRT,
        VP880_DC_CAL_REG_LEN, dcCal);

    /* Advance to Next State */
    pLineObj->calLineData.calState = VP880_CAL_IMT_OFFSET_SET_INVERT;

    /*
     * Need to wait for line to settle before changing converter. Time is
     * based on polarity change which is "known" in API-II.
     */
    pLineObj->lineTimers.timers.timer[VP_LINE_CAL_LINE_TIMER] =
        MS_TO_TICKRATE(VP_POLREV_DEBOUNCE_TIME,
            pDevObj->devProfileData.tickRate) | VP_ACTIVATE_TIMER;

    return;
} /* end Vp880VocMeasure */

/**
 * Vp880VocMeasureInvert()
 *  This function read switcher value and compare with
 *  the value read from the pcm data if the value is bigger than 1.25v
 * this function will make a correction  voltage .
 *
 */
void
Vp880VocMeasureInvert(
    VpLineCtxType *pLineCtx)
{
    Vp880LineObjectType *pLineObj = pLineCtx->pLineObj;
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpDeviceIdType deviceId = pDevObj->deviceId;
    uint8 ecVal[] = {VP880_EC_CH1, VP880_EC_CH2};
    uint8 channelId = pLineObj->channelId;
    int16 imtValue, ilgValue, vocValue, vagValue, vbgValue;
    uint8 dcCal[VP880_DC_CAL_REG_LEN];
    uint8 converterCfg[VP880_CONV_CFG_LEN];

    switch (pLineObj->calLineData.calState) {
        case VP880_CAL_IMT_OFFSET_SET_INVERT:
            converterCfg[0] = VP880_METALLIC_DC_I;

            /* Force the converter change. Don't care about data yet. */
            VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_CONV_CFG_WRT,
                VP880_CONV_CFG_LEN, converterCfg);

            pLineObj->calLineData.calState = VP880_CAL_IMT_OFFSET_READ_INVERT;

            pLineObj->lineTimers.timers.timer[VP_LINE_CAL_LINE_TIMER] =
                MS_TO_TICKRATE(VP880_CAL_VOC_SHORT,
                    pDevObj->devProfileData.tickRate) | VP_ACTIVATE_TIMER;
            return;

        case VP880_CAL_IMT_OFFSET_READ_INVERT:
            imtValue = Vp880AdcSettling(deviceId, ecVal[channelId],
                VP880_METALLIC_DC_I);
            converterCfg[0] = VP880_LONGITUDINAL_DC_I;

            /* Force the converter change. Don't care about data yet. */
            VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_CONV_CFG_WRT,
                VP880_CONV_CFG_LEN, converterCfg);

            pLineObj->calLineData.calState = VP880_CAL_ILG_OFFSET_READ_INVERT;

            pLineObj->lineTimers.timers.timer[VP_LINE_CAL_LINE_TIMER] =
                MS_TO_TICKRATE(VP880_CAL_VOC_SHORT,
                    pDevObj->devProfileData.tickRate) | VP_ACTIVATE_TIMER;
            return;

        case VP880_CAL_ILG_OFFSET_READ_INVERT:
            ilgValue = Vp880AdcSettling(deviceId, ecVal[channelId],
                VP880_LONGITUDINAL_DC_I);

            if (pLineObj->calLineData.reversePol == FALSE) {
                VP_CALIBRATION(VpLineCtxType, pLineCtx, ("ILG Norm %d Channel %d",
                    ilgValue, channelId));
                pLineObj->calLineData.typeData.ilgData.ilgNorm = ilgValue;
            } else {
                /*
                 * Don't care about ilg reverse polarity. Device is better than
                 * it can measure.
                 */
            }

            converterCfg[0] = VP880_METALLIC_DC_V;

            /* Force the converter change. Don't care about data yet. */
            VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_CONV_CFG_WRT,
                VP880_CONV_CFG_LEN, converterCfg);

            pLineObj->calLineData.calState = VP880_CAL_VOC_READ_INVERT;

            pLineObj->lineTimers.timers.timer[VP_LINE_CAL_LINE_TIMER] =
                MS_TO_TICKRATE(VP880_CAL_VOC_SHORT,
                    pDevObj->devProfileData.tickRate) | VP_ACTIVATE_TIMER;
            return;

        case VP880_CAL_VOC_READ_INVERT:
            vocValue = Vp880AdcSettling(deviceId, ecVal[channelId],
                VP880_METALLIC_DC_V);

            if (pLineObj->calLineData.reversePol == FALSE) {
                VP_CALIBRATION(VpLineCtxType, pLineCtx, ("1. VOC OffsetNorm (in 10mV) %d Channel %d",
                    (int16)(vocValue * VP880_V_PCM_LSB/VP880_V_SCALE), channelId));

                pDevObj->vp880SysCalData.vocOffset[channelId][VP880_NORM_POLARITY] = vocValue;

                /* This is to be backward compatible with VVA P1.3.0 */
                pLineObj->calLineData.typeData.vocData.vocOffsetNorm = vocValue;
            } else {
                VP_CALIBRATION(VpLineCtxType, pLineCtx, ("2. VOC OffsetRev (in 10mV) %d Channel %d",
                    (int16)(vocValue * VP880_V_PCM_LSB/VP880_V_SCALE), channelId));
                pDevObj->vp880SysCalData.vocOffset[channelId][VP880_REV_POLARITY] = vocValue;

                /* This is to be backward compatible with VVA P1.3.0 */
                pLineObj->calLineData.typeData.vocData.vocOffsetRev = vocValue;
            }

            converterCfg[0] = VP880_TIP_TO_GND_V;

            /* Force the converter change. Don't care about data yet. */
            VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_CONV_CFG_WRT,
                VP880_CONV_CFG_LEN, converterCfg);

            pLineObj->calLineData.calState = VP880_CAL_VAG_READ_INVERT;

            pLineObj->lineTimers.timers.timer[VP_LINE_CAL_LINE_TIMER] =
                MS_TO_TICKRATE(VP880_CAL_VOC_SHORT,
                    pDevObj->devProfileData.tickRate) | VP_ACTIVATE_TIMER;
            return;

        case VP880_CAL_VAG_READ_INVERT:
            vagValue = Vp880AdcSettling(deviceId, ecVal[channelId],
                VP880_TIP_TO_GND_V);

            if (pLineObj->calLineData.reversePol == FALSE) {
                VP_CALIBRATION(VpLineCtxType, pLineCtx, ("VAG OffsetNorm %d Channel %d",
                    vagValue, channelId));
                pDevObj->vp880SysCalData.vagOffsetNorm[channelId] = vagValue;

                /* This is to be backward compatible with VVA P1.3.0 */
                pLineObj->calLineData.typeData.vagData.vagOffsetNorm = vagValue;
            } else {
                VP_CALIBRATION(VpLineCtxType, pLineCtx, ("VAG OffsetRev %d Channel %d",
                    vagValue, channelId));
                pDevObj->vp880SysCalData.vagOffsetRev[channelId] = vagValue;

                /* This is to be backward compatible with VVA P1.3.0 */
                pLineObj->calLineData.typeData.vagData.vagOffsetRev = vagValue;
            }

            converterCfg[0] = VP880_RING_TO_GND_V;

            /* Force the converter change. Don't care about data yet. */
            VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_CONV_CFG_WRT,
                VP880_CONV_CFG_LEN, converterCfg);

            pLineObj->calLineData.calState = VP880_CAL_VBG_READ_INVERT;

            pLineObj->lineTimers.timers.timer[VP_LINE_CAL_LINE_TIMER] =
                MS_TO_TICKRATE(VP880_CAL_VOC_SHORT,
                    pDevObj->devProfileData.tickRate) | VP_ACTIVATE_TIMER;
            return;

        case VP880_CAL_VBG_READ_INVERT:
            vbgValue = Vp880AdcSettling(deviceId, ecVal[channelId],
                VP880_RING_TO_GND_V);

            if (pLineObj->calLineData.reversePol == FALSE) {
                VP_CALIBRATION(VpLineCtxType, pLineCtx, ("VBG OffsetNorm %d Channel %d",
                    vbgValue, channelId));
                pDevObj->vp880SysCalData.vbgOffsetNorm[channelId] = vbgValue;

                /* This is to compatible with VVA P1.3.0 */
                pLineObj->calLineData.typeData.vbgData.vbgOffsetNorm = vbgValue;
            } else {
                VP_CALIBRATION(VpLineCtxType, pLineCtx, ("VBG OffsetRev %d Channel %d",
                    vbgValue, channelId));
                pDevObj->vp880SysCalData.vbgOffsetRev[channelId] = vbgValue;

                /* This is to compatible with VVA P1.3.0 */
                pLineObj->calLineData.typeData.vbgData.vbgOffsetRev = vbgValue;
            }
            break;

        default:
            break;
    }

    /* Re-Enable TRDC Sense */
    VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_DC_CAL_REG_RD,
        VP880_DC_CAL_REG_LEN, dcCal);
    dcCal[1] &= ~(VP880_C_RING_SNS_CUT | VP880_C_TIP_SNS_CUT);
    dcCal[1] |= VP880_DC_CAL_DIS_INPUT_OFFSET;
    VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_DC_CAL_REG_WRT,
        VP880_DC_CAL_REG_LEN, dcCal);

    /* Return to VOC feed, speedup enabled */
    VP_CALIBRATION(VpLineCtxType, pLineCtx, ("CAL Restore: Channel %d: Writing ICR2 0x%02X 0x%02X 0x%02X 0x%02X",
        channelId,
        pLineObj->calLineData.icr2Values[0],
        pLineObj->calLineData.icr2Values[1],
        pLineObj->calLineData.icr2Values[2],
        pLineObj->calLineData.icr2Values[3]));

    VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_ICR2_WRT, VP880_ICR2_LEN,
        pLineObj->calLineData.icr2Values);

    /* Advance to Final States */
    pLineObj->calLineData.calState = VP880_CAL_IMT_OFFSET_SET_DONE;

    /* Need to wait for line to settle before changing converter. */
    pLineObj->lineTimers.timers.timer[VP_LINE_CAL_LINE_TIMER] =
        MS_TO_TICKRATE(VP880_CAL_VOC_SHORT,
            pDevObj->devProfileData.tickRate) | VP_ACTIVATE_TIMER;

    return;
} /* end Vp880VocMeasureInvert */

/**
 * Vp880CalVas()
 *  This function determines the optimal VAS value for both normal and reverse
 * polarity on the line specified by the line context. It should only be called
 * by API-II internal functions.
 *
 * Preconditions:
 *  The device and line context must be created and initialized before calling
 * this function.
 *
 * Postconditions:
 *  VAS values for normal and reverse polarity are computed and stored in the
 * line object. The line state is returned to state it was in prior to starting
 * calibration.
 */
VpStatusType
Vp880CalVas(
    VpLineCtxType *pLineCtx)
{
    Vp880LineObjectType *pLineObj = pLineCtx->pLineObj;
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpDeviceIdType deviceId = pDevObj->deviceId;
    uint8 ecVal[] = {VP880_EC_CH1, VP880_EC_CH2};
    uint8 channelId = pLineObj->channelId;
    uint8 convCfg[VP880_CONV_CFG_LEN];
    uint8 dcFeed[VP880_DC_FEED_LEN];
    uint8 imtAvgLoopCnt;

    int16 imtNew[VP880_IMT_AVERAGE_CNT];
    int32 imtSumAvg;

    uint16 imtErr;
    uint16 vasValue;
    uint8 icr2Mods[VP880_ICR2_LEN];
    uint8 deviceMode[VP880_DEV_MODE_LEN];
    uint8 sysState[VP880_SYS_STATE_LEN];

    switch(pLineObj->calLineData.calState) {
        case VP880_CAL_INIT:
            VP_CALIBRATION(VpLineCtxType, pLineCtx,("VAS: Cal Init Channel %d", channelId));
            pLineObj->calLineData.typeData.vasData.secondPass = FALSE;

            /* Read system State and keep Pol sign */
            VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_SYS_STATE_RD,
                VP880_SYS_STATE_LEN, sysState);

            /*
             * Save current dc calibration data. This value is adjusted by cal
             * algorithms to finalize a set that is programmed in the device
             * by Set Line State. It will contain adjusted VOC values for each
             * polarity.
             */
            if (sysState[0] & VP880_SS_POLARITY_MASK) {
                pLineObj->calLineData.reversePol = TRUE;
                dcFeed[0] = pLineObj->calLineData.dcFeedPr[0];
                dcFeed[1] = pLineObj->calLineData.dcFeedPr[1];
            } else {
                pLineObj->calLineData.reversePol = FALSE;
                dcFeed[0] = pLineObj->calLineData.dcFeed[0];
                dcFeed[1] = pLineObj->calLineData.dcFeed[1];
            }

            VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_ICR2_RD, VP880_ICR2_LEN,
                icr2Mods);

            /* Set Battery and Feed for High-Speed switching */
            icr2Mods[2] |= 0xCC;
            icr2Mods[3] |= 0xCC;

            VP_CALIBRATION(VpLineCtxType, pLineCtx, ("CAL High-Speed : Channel %d: Writing ICR2 0x%02X 0x%02X 0x%02X 0x%02X",
                channelId, icr2Mods[0], icr2Mods[1], icr2Mods[2], icr2Mods[3]));

            VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_ICR2_WRT, VP880_ICR2_LEN,
                icr2Mods);

            /* Set VAS to minimum */
            VpCSLACSetVas(dcFeed, 0);
            VP_CALIBRATION(VpLineCtxType, pLineCtx, ("1. VAS Writing 0x%04X to DC Feed Channel %d",
                (((dcFeed[0] << 8) & 0xFF00) | dcFeed[1]), channelId));
            VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_DC_FEED_WRT,
                VP880_DC_FEED_LEN, dcFeed);

            /* Make sure samples are collected from the PCM Test Buffer */
            VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_DEV_MODE_RD,
                VP880_DEV_MODE_LEN, deviceMode);
            deviceMode[0] &= ~VP880_DEV_MODE_TEST_DATA;
            VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_DEV_MODE_WRT,
                VP880_DEV_MODE_LEN, deviceMode);

            /*
             * Start timers to wait for supply to settle before taking the
             * first measurement
             */
            pLineObj->lineTimers.timers.timer[VP_LINE_CAL_LINE_TIMER] =
               MS_TO_TICKRATE(VP880_VAS_INIT_WAIT, pDevObj->devProfileData.tickRate);

            pLineObj->lineTimers.timers.timer[VP_LINE_CAL_LINE_TIMER] |=
               VP_ACTIVATE_TIMER;

            /* Increment the state machine */
            pLineObj->calLineData.calState = VP880_CONVERTER_CHECK;
            break;

        case VP880_CONVERTER_CHECK:
            VP_CALIBRATION(VpLineCtxType, pLineCtx, ("VAS Cal: Converter Check Channel %d",
                channelId));

            VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_CONV_CFG_RD,
                VP880_CONV_CFG_LEN, convCfg);

            if ((convCfg[0] & VP880_CONV_CONNECT_BITS) != VP880_METALLIC_DC_I) {
                VP_CALIBRATION(VpLineCtxType, pLineCtx, ("Converter Check Fail Channel %d",
                    channelId));

                /*
                 * The device internally changed the converter configuration.
                 * So we're in a bad condition, no need to measure. Increase
                 * VAS and try again.
                 */
                VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_DC_FEED_RD,
                    VP880_DC_FEED_LEN, dcFeed);

                vasValue = VP880_VAS_CONVERSION(dcFeed[0], dcFeed[1]);
                VpCSLACSetVas(dcFeed, vasValue + 750);

                if (vasValue >= (VP880_VAS_MAX - VP880_VAS_OVERHEAD)) {
                    /*
                     * If we're at the maximum VAS value, then that's the
                     * best we can and no sense continuing.
                     */
                    VpCSLACSetVas(dcFeed, VP880_VAS_MAX);
                    VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_DC_FEED_WRT,
                        VP880_DC_FEED_LEN, dcFeed);

                    pLineObj->calLineData.calState = VP880_CAL_DONE;
                    VP_CALIBRATION(VpLineCtxType, pLineCtx, ("Giving Up on Converter Check Channel %d",
                        channelId));
                } else {
                    VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_DC_FEED_WRT,
                        VP880_DC_FEED_LEN, dcFeed);

                    convCfg[0] = VP880_METALLIC_DC_I;
                    VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_CONV_CFG_WRT,
                        VP880_CONV_CFG_LEN, convCfg);

                    VP_CALIBRATION(VpLineCtxType, pLineCtx, ("2. VAS (%d) - VOC Reg 0x%02X 0x%02X Channel %d",
                        vasValue, dcFeed[0], dcFeed[1], channelId));
                }
            } else {
                pLineObj->calLineData.calState = VP880_CAL_OFFSET;
            }

            pLineObj->lineTimers.timers.timer[VP_LINE_CAL_LINE_TIMER] =
               MS_TO_TICKRATE(VP880_VAS_MEAS_DELAY,
                pDevObj->devProfileData.tickRate);

            pLineObj->lineTimers.timers.timer[VP_LINE_CAL_LINE_TIMER] |=
               VP_ACTIVATE_TIMER;
            break;

        case VP880_CAL_OFFSET:
            VP_CALIBRATION(VpLineCtxType, pLineCtx,("VAS Cal: Cal Offset Channel %d",
                channelId));

            imtSumAvg = 0;
            for (imtAvgLoopCnt = 0;
                 imtAvgLoopCnt < VP880_IMT_AVERAGE_CNT;
                 imtAvgLoopCnt++) {

                imtNew[imtAvgLoopCnt] = Vp880AdcSettling(deviceId,
                    ecVal[channelId], VP880_METALLIC_DC_I);

                imtSumAvg += imtNew[imtAvgLoopCnt];
            }
            imtSumAvg /= VP880_IMT_AVERAGE_CNT;
            pLineObj->calLineData.typeData.vasData.imtPrev = (int16)imtSumAvg;

            VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_DC_FEED_RD,
                VP880_DC_FEED_LEN, dcFeed);

            vasValue = VP880_VAS_CONVERSION(dcFeed[0], dcFeed[1]);
            VpCSLACSetVas(dcFeed, vasValue + 750);

            VP_CALIBRATION(VpLineCtxType, pLineCtx, ("3. VAS (%d) - VOC Reg 0x%02X 0x%02X Channel %d",
                vasValue, dcFeed[0], dcFeed[1], channelId));

            VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_DC_FEED_WRT,
                VP880_DC_FEED_LEN, dcFeed);

            if (vasValue >= (VP880_VAS_MAX - VP880_VAS_OVERHEAD)) {
                /*
                 * If we're at the maximum VAS value, then that's the
                 * best we can and no sense continuing.
                 */
                VP_CALIBRATION(VpLineCtxType, pLineCtx, ("1. Max Overhead Reached Channel %d",
                    channelId));

                VpCSLACSetVas(dcFeed, VP880_VAS_MAX);
                VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_DC_FEED_WRT,
                    VP880_DC_FEED_LEN, dcFeed);

                Vp880VasPolRev(pLineCtx);
            } else {
                pLineObj->calLineData.calState = VP880_CAL_MEASURE;
            }
            pLineObj->lineTimers.timers.timer[VP_LINE_CAL_LINE_TIMER] =
                MS_TO_TICKRATE(VP880_VAS_MEAS_DELAY,
                pDevObj->devProfileData.tickRate);

            pLineObj->lineTimers.timers.timer[VP_LINE_CAL_LINE_TIMER] |=
                VP_ACTIVATE_TIMER;
            break;

        case VP880_CAL_MEASURE:
            VP_CALIBRATION(VpLineCtxType, pLineCtx,("VAS: Cal Measure Channel %d", channelId));

            imtSumAvg = 0;
            for (imtAvgLoopCnt = 0;
                 imtAvgLoopCnt < VP880_IMT_AVERAGE_CNT;
                 imtAvgLoopCnt++) {

                imtNew[imtAvgLoopCnt] = Vp880AdcSettling(deviceId,
                    ecVal[channelId], VP880_METALLIC_DC_I);

                imtSumAvg += imtNew[imtAvgLoopCnt];
            }
            imtSumAvg /= VP880_IMT_AVERAGE_CNT;

            imtNew[0] = (int16)imtSumAvg;
            imtErr = (imtNew[0] >= pLineObj->calLineData.typeData.vasData.imtPrev)
                ? (imtNew[0] - pLineObj->calLineData.typeData.vasData.imtPrev)
                : (pLineObj->calLineData.typeData.vasData.imtPrev - imtNew[0]);

            VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_DC_FEED_RD,
                VP880_DC_FEED_LEN, dcFeed);
            vasValue = VP880_VAS_CONVERSION(dcFeed[0], dcFeed[1]);

            if (imtErr > VP880_VAS_MEAS_ERR) {
                pLineObj->calLineData.typeData.vasData.imtPrev = imtNew[0];

                VpCSLACSetVas(dcFeed, vasValue + 750);
                VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_DC_FEED_WRT,
                    VP880_DC_FEED_LEN, dcFeed);

                VP_CALIBRATION(VpLineCtxType, pLineCtx, ("3. VAS (%d) - VOC Reg 0x%02X 0x%02X Channel %d",
                    vasValue, dcFeed[0], dcFeed[1], channelId));

                pLineObj->lineTimers.timers.timer[VP_LINE_CAL_LINE_TIMER] =
                    MS_TO_TICKRATE(VP880_VAS_MEAS_DELAY,
                        pDevObj->devProfileData.tickRate);

                pLineObj->lineTimers.timers.timer[VP_LINE_CAL_LINE_TIMER] |=
                    VP_ACTIVATE_TIMER;

                if (vasValue >= (VP880_VAS_MAX - VP880_VAS_OVERHEAD)) {
                    /*
                     * If we're at the maximum VAS value, then that's the
                     * best we can and no sense continuing.
                     */
                    VP_CALIBRATION(VpLineCtxType, pLineCtx, ("2. Max Overhead Reached Channel %d",
                        channelId));

                    VpCSLACSetVas(dcFeed, VP880_VAS_MAX);
                    VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_DC_FEED_WRT,
                        VP880_DC_FEED_LEN, dcFeed);

                    if (pLineObj->calLineData.typeData.vasData.secondPass == FALSE) {
                        pLineObj->calLineData.typeData.vasData.secondPass = TRUE;
                        Vp880VasPolRev(pLineCtx);
                        pLineObj->lineTimers.timers.timer[VP_LINE_CAL_LINE_TIMER] =
                            MS_TO_TICKRATE(VP880_VAS_MEAS_DELAY,
                                pDevObj->devProfileData.tickRate);

                        pLineObj->lineTimers.timers.timer[VP_LINE_CAL_LINE_TIMER] |=
                            VP_ACTIVATE_TIMER;

                        /* Set VAS to minimum */
                        VpCSLACSetVas(dcFeed, 0);
                        VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_DC_FEED_WRT,
                            VP880_DC_FEED_LEN, dcFeed);
                    } else {
                        pLineObj->calLineData.calState = VP880_CAL_DONE;
                    }
                }
            } else {
                VpCSLACSetVas(dcFeed, vasValue + VP880_VAS_OVERHEAD);

                if (pLineObj->calLineData.reversePol == FALSE) {
                    pLineObj->calLineData.reversePol = TRUE;
                    pLineObj->calLineData.dcFeed[0] = dcFeed[0];
                    pLineObj->calLineData.dcFeed[1] = dcFeed[1];

                    VP_CALIBRATION(VpLineCtxType, pLineCtx, ("VAS Normal Polarity Cal OK -- Saving 0x%02X 0x%02X Channel %d",
                        pLineObj->calLineData.dcFeed[0],
                        pLineObj->calLineData.dcFeed[1],
                        channelId));

                    dcFeed[0] = pLineObj->calLineData.dcFeedPr[0];
                    dcFeed[1] = pLineObj->calLineData.dcFeedPr[1];
                } else {
                    pLineObj->calLineData.reversePol = FALSE;
                    pLineObj->calLineData.dcFeedPr[0] = dcFeed[0];
                    pLineObj->calLineData.dcFeedPr[1] = dcFeed[1];

                    VP_CALIBRATION(VpLineCtxType, pLineCtx, ("VAS Reverse Polarity Cal OK -- Saving 0x%02X 0x%02X Channel %d",
                        pLineObj->calLineData.dcFeedPr[0],
                        pLineObj->calLineData.dcFeedPr[1],
                        channelId));

                    dcFeed[0] = pLineObj->calLineData.dcFeed[0];
                    dcFeed[1] = pLineObj->calLineData.dcFeed[1];
                }

                if (pLineObj->calLineData.typeData.vasData.secondPass == FALSE) {
                    pLineObj->calLineData.typeData.vasData.secondPass = TRUE;
                    Vp880VasPolRev(pLineCtx);
                    pLineObj->lineTimers.timers.timer[VP_LINE_CAL_LINE_TIMER] =
                        MS_TO_TICKRATE(VP880_VAS_MEAS_DELAY,
                            pDevObj->devProfileData.tickRate);

                    pLineObj->lineTimers.timers.timer[VP_LINE_CAL_LINE_TIMER] |=
                        VP_ACTIVATE_TIMER;

                    /* Set VAS to minimum */
                    VpCSLACSetVas(dcFeed, 0);
                    VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_DC_FEED_WRT,
                        VP880_DC_FEED_LEN, dcFeed);
                } else {
                    pLineObj->calLineData.calState = VP880_CAL_DONE;
                }
            }
            break;

        case VP880_CAL_INVERT_POL:
            VP_CALIBRATION(VpLineCtxType, pLineCtx,("VAS: Reversing Polarity Channel %d", channelId));
            Vp880SetLineStateInt(pLineCtx,
                VpGetReverseState(pLineObj->lineState.currentState));

            /* Set VAS to minimum */
            VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_DC_FEED_RD,
                VP880_DC_FEED_LEN, pLineObj->calLineData.dcFeed);

            dcFeed[0] = pLineObj->calLineData.dcFeed[0];
            dcFeed[1] = pLineObj->calLineData.dcFeed[1];

            VpCSLACSetVas(dcFeed, 0);
            VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_DC_FEED_WRT,
                VP880_DC_FEED_LEN, dcFeed);
            VP_CALIBRATION(VpLineCtxType, pLineCtx, ("1. VAS - VOC Reg 0x%02X 0x%02X Channel %d",
                dcFeed[0], dcFeed[1], channelId));

            /*
             * Start timers to wait for supply to settle before taking the
             * measurement
             */
            pLineObj->lineTimers.timers.timer[VP_LINE_CAL_LINE_TIMER] =
                MS_TO_TICKRATE(VP_POLREV_DEBOUNCE_TIME,
                    pDevObj->devProfileData.tickRate);

            pLineObj->lineTimers.timers.timer[VP_LINE_CAL_LINE_TIMER] |=
               VP_ACTIVATE_TIMER;

            /* Increment the state machine */
            pLineObj->calLineData.calState = VP880_CONVERTER_CHECK;
            break;

        default:
            VP_CALIBRATION(VpLineCtxType, pLineCtx,("VAS Cal Done Channel %d", channelId));
            pLineObj->calLineData.calState = VP880_CAL_DONE;
            break;
    }

    if (pLineObj->calLineData.calState == VP880_CAL_DONE) {
        VP_CALIBRATION(VpLineCtxType, pLineCtx,("VAS: Cal Completion Channel %d", channelId));

        VP_CALIBRATION(VpLineCtxType, pLineCtx, ("VAS Restore: ICR2 0x%02X 0x%02X 0x%02X 0x%02X Ch %d",
            pLineObj->calLineData.icr2Values[0],
            pLineObj->calLineData.icr2Values[1],
            pLineObj->calLineData.icr2Values[2],
            pLineObj->calLineData.icr2Values[3],
            channelId));

        VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_ICR2_WRT, VP880_ICR2_LEN,
            pLineObj->calLineData.icr2Values);

        /*
         * Both dcFeed and dcFeedPr contain the same maximum VAS setting that
         * is required for both normal and polarity reversal conditions to
         * prevent saturation condition. So using either value in the line
         * object is ok.
         */
        VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_DC_FEED_WRT,
            VP880_DC_FEED_LEN, pLineObj->calLineData.dcFeed);

        VP_CALIBRATION(VpLineCtxType, pLineCtx, ("5. VAS - VOC Normal 0x%02X 0x%02X Reverse 0x%02X 0x%02X Channel %d",
            pLineObj->calLineData.dcFeed[0],
            pLineObj->calLineData.dcFeed[1],
            pLineObj->calLineData.dcFeedPr[0],
            pLineObj->calLineData.dcFeedPr[1],
            channelId));

        /* Second byte contains the lower two bits of VAS */
        pDevObj->vp880SysCalData.vas[channelId][VP880_NORM_POLARITY] =
            ((pLineObj->calLineData.dcFeed[1] >> 6) & 0x3);

        /* First byte contains the upper two bits of VAS */
        pDevObj->vp880SysCalData.vas[channelId][VP880_NORM_POLARITY] |=
            ((pLineObj->calLineData.dcFeed[0] << 2) & 0xC);

        /* Second byte contains the lower two bits of VAS */
        pDevObj->vp880SysCalData.vas[channelId][VP880_REV_POLARITY] =
            ((pLineObj->calLineData.dcFeedPr[1] >> 6) & 0x3);

        /* First byte contains the upper two bits of VAS */
        pDevObj->vp880SysCalData.vas[channelId][VP880_REV_POLARITY] |=
            ((pLineObj->calLineData.dcFeedPr[0] << 2) & 0xC);

        pLineObj->responseData = VP_CAL_SUCCESS;

        Vp880CalDone(pLineCtx);

        /* Hold this off until cleanup is complete */
        pLineObj->lineEvents.response &= ~VP_EVID_CAL_CMP;
        pLineObj->calLineData.calDone = FALSE;
        pLineObj->lineState.calType = VP_CSLAC_CAL_CLEANUP;

        /*
         * Holdoff the event and "normal" VP-API-II function behavior until line
         * conditions have stabilized
         */
        pLineObj->lineTimers.timers.timer[VP_LINE_CAL_LINE_TIMER] =
            (MS_TO_TICKRATE(VP880_MIN_CAL_WAIT_TIME,
                pDevObj->devProfileData.tickRate)) | VP_ACTIVATE_TIMER;
    }

    return VP_STATUS_SUCCESS;
}

/**
 * Vp880VasPolRev()
 *  This function inverts polarity of line and sets reversePol flag in line
 * object (calibration) appropriately.
 *
 * Preconditions:
 *  The device and line context must be created and initialized before calling
 * this function.
 *
 * Postconditions:
 *  Line polarity is reversed. Line Object updated.
 */
void
Vp880VasPolRev(
    VpLineCtxType *pLineCtx)
{
    Vp880LineObjectType *pLineObj = pLineCtx->pLineObj;
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpDeviceIdType deviceId = pDevObj->deviceId;
    uint8 ecVal[] = {VP880_EC_CH1, VP880_EC_CH2};
    uint8 channelId = pLineObj->channelId;
    uint8 sysState[VP880_SYS_STATE_LEN];

    /* Read system State */
    VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_SYS_STATE_RD,
        VP880_SYS_STATE_LEN, sysState);

    if (sysState[0] & VP880_SS_POLARITY_MASK) {
        sysState[0] &= ~VP880_SS_POLARITY_MASK;
        pLineObj->calLineData.reversePol = FALSE;
    } else {
        sysState[0] |= VP880_SS_POLARITY_MASK;
        pLineObj->calLineData.reversePol = TRUE;
    }

    VP_CALIBRATION(VpLineCtxType, pLineCtx, ("Vp880VasPolRev: Setting Ch %d to State 0x%02X at time %d",
        channelId, sysState[0], pDevObj->timeStamp));

    Vp880UpdateBufferChanSel(pDevObj, channelId, sysState[0]);
    VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_SYS_STATE_WRT,
        VP880_SYS_STATE_LEN, sysState);
}

/**
 * Vp880CalVoc()
 *  This function sets the VOC values in dcFeed as specified by the device
 * dc feed register, with VOS value passed. It does not actually access the
 * device, just simply computes the correct hex values for the dc feed reg.
 *
 * Preconditions:
 *  None. Helper function only.
 *
 * Postconditions:
 *  Line not affected. Values in dcFeed contain the VOC values passed.
 */

VpStatusType
Vp880CalVoc(
    VpLineCtxType *pLineCtx)
{
    Vp880LineObjectType *pLineObj = pLineCtx->pLineObj;
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;

    VP_CALIBRATION(VpLineCtxType, pLineCtx, ("Chan %d VOC Cal State %d",
        pLineObj->channelId, pLineObj->calLineData.calState));

    switch(pLineObj->calLineData.calState) {
        case VP880_CAL_INIT:
            Vp880VocInit(pLineCtx);
            break;

        case VP880_CAL_ADC:
            Vp880VocSetAdc(pLineCtx);
            break;

        case VP880_CAL_OFFSET:
        case VP880_CAL_FIRST_IMT_SET:
        case VP880_CAL_FIRST_IMT_READ:
        case VP880_CAL_FIRST_ILG_READ:
        case VP880_CAL_FIRST_VOC_READ:
        case VP880_CAL_SIGEN_A_PHASE1:
        case VP880_CAL_SIGEN_A_PHASE2:
        case VP880_CAL_SIGEN_A_PHASE3:
        case VP880_CAL_FIRST_VAG_READ:
        case VP880_CAL_FIRST_VBG_READ:
            Vp880VocOffset(pLineCtx);
            break;

        case VP880_CAL_MEASURE:
        case VP880_CAL_IMT_OFFSET_SET:
        case VP880_CAL_IMT_OFFSET_READ:
        case VP880_CAL_ILG_OFFSET_READ:
            Vp880VocMeasure(pLineCtx);
            break;

        case VP880_CAL_INVERT_POL:
        case VP880_CAL_IMT_OFFSET_SET_INVERT:
        case VP880_CAL_IMT_OFFSET_READ_INVERT:
        case VP880_CAL_ILG_OFFSET_READ_INVERT:
        case VP880_CAL_VOC_READ_INVERT:
        case VP880_CAL_VAG_READ_INVERT:
        case VP880_CAL_VBG_READ_INVERT:
            Vp880VocMeasureInvert(pLineCtx);
            break;

        case VP880_CAL_DONE:
        case VP880_CAL_IMT_OFFSET_SET_DONE:
        case VP880_CAL_IMT_OFFSET_READ_DONE:
        case VP880_CAL_ILG_OFFSET_READ_DONE:
        case VP880_CAL_VOC_READ_DONE:
            Vp880VocDone(pLineCtx);
            break;

        case VP880_CAL_ERROR:
            VP_CALIBRATION(VpLineCtxType, pLineCtx,("VOC Cal Error Channel %d", pLineObj->channelId));
            pLineObj->responseData = VP_CAL_FAILURE;
            Vp880VocDone(pLineCtx);
            Vp880CalDone(pLineCtx);

            pLineObj->calLineData.calState = VP880_CAL_EXIT;
            pLineObj->lineState.calType = VP_CSLAC_CAL_NONE;

            pLineObj->lineTimers.timers.timer[VP_LINE_CAL_LINE_TIMER] =
                MS_TO_TICKRATE((VP880_CAL_SET),
                    pDevObj->devProfileData.tickRate) | VP_ACTIVATE_TIMER;
            break;

        default:
            pLineObj->calLineData.calState = VP880_CAL_INIT;
            pLineObj->lineState.calType = VP_CSLAC_CAL_NONE;
            break;
    }

    return VP_STATUS_SUCCESS;

} /*end Vp880CalVoc */

/**
 * Vp880CalDone()
 *  This function initiates a calibration operation for VOC
 * associated with all the lines of a device. See VP-API reference guide for
 * more information.

 * Preconditions:
 *  The device and line context must be created and initialized before calling
 * this function.
 *
 * Postconditions:
 *  This function generates an event upon completing the requested action.
 */
void
Vp880CalDone(
    VpLineCtxType *pLineCtx)
{
    Vp880LineObjectType *pLineObj = pLineCtx->pLineObj;
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    uint8 ecVal[] = {VP880_EC_CH1, VP880_EC_CH2};
    uint8 channelId = pLineObj->channelId;
    uint8 otherChan = (channelId == 0) ? 1 : 0;
    VpDeviceIdType deviceId = pDevObj->deviceId;
    uint8 data;
    uint8 convCfg[VP880_CONV_CFG_LEN];

    VP_CALIBRATION(VpLineCtxType, pLineCtx,("Calibration Done Channel %d", channelId));

    /* Set use of programmed coefficients and Codec Mode */
    data = VP880_ENABLE_LOADED_COEFFICIENTS;
    VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_OP_FUNC_WRT, VP880_OP_FUNC_LEN,
        &data);
    Vp880SetCodec(pLineCtx, pLineObj->codec);

    /* Cut TX/RX PCM and enable HPF */
    data = (VP880_CUT_TXPATH | VP880_CUT_RXPATH);
    VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_OP_COND_WRT, VP880_OP_COND_LEN,
        &data);

    /* Restore DISN */
    VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_DISN_WRT, VP880_DISN_LEN,
        pLineObj->calLineData.disnVal);


    /* Restore Device Mode */
    VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_DEV_MODE_RD, VP880_DEV_MODE_LEN,
        &data);
    data |= VP880_DEV_MODE_TEST_DATA;
    VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_DEV_MODE_WRT, VP880_DEV_MODE_LEN,
        &data);

    /* Connect A/D to AC T/R Input, restore device sample rate */
    convCfg[0] = (VP880_METALLIC_AC_V | pDevObj->txBufferDataRate);
    VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_CONV_CFG_WRT, VP880_CONV_CFG_LEN,
        convCfg);

    pLineObj->status &= ~VP880_LINE_IN_CAL;

    /* Restore Line State */
    Vp880SetLineState(pLineCtx, pLineObj->calLineData.usrState);

    if (pLineObj->responseData == VP_CAL_SUCCESS) {
        pLineObj->calLineData.calDone = TRUE;
    }

    pLineObj->lineState.calType = VP_CSLAC_CAL_NONE;
    pLineObj->lineEvents.response |= VP_EVID_CAL_CMP;

    if (pDevObj->vp880SysCalData.ila40[otherChan]) {
        pDevObj->stateInt |= VP880_SYS_CAL_COMPLETE;
    }

    /*
     * Force an update on the line then wait at least one "tick" before
     * reporting the event. This prevents a false-hook event using 5REN FCC
     * load.
     */
    Vp880LowPowerMode(pDevCtx);
}

/**
 * Vp880VocInit()
 *  This function initiates a calibration operation for VOC
 * associated with all the lines of a device. See VP-API reference guide for
 * more information.

 * Preconditions:
 *  The device and line context must be created and initialized before calling
 * this function.
 *
 * Postconditions:
 *  This function generates an event upon completing the requested action.
 */
void
Vp880VocInit(
    VpLineCtxType *pLineCtx)
{
    Vp880LineObjectType *pLineObj = pLineCtx->pLineObj;
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    uint8 ecVal[] = {VP880_EC_CH1, VP880_EC_CH2};
    uint8 channelId = pLineObj->channelId;
    VpDeviceIdType deviceId = pDevObj->deviceId;
    uint8 converterCfg[VP880_CONV_CFG_LEN]  = {VP880_METALLIC_DC_V};
    uint8 icr2[VP880_ICR2_LEN] = {0x00, 0x00, 0xC0, 0xC0};
    uint8 xdataTemp[VP880_TX_PCM_DATA_LEN];
    int16 tempNew;
    uint8 dcFeed20mA[VP880_DC_FEED_LEN];

    VpMemCpy(pLineObj->calLineData.icr2Values, pLineObj->icr2Values,
        VP880_ICR2_LEN);

    VP_CALIBRATION(VpLineCtxType, pLineCtx, ("Vp880VocInit: Saving Off Chan %d ICR2 0x%02X 0x%02X 0x%02X 0x%02X",
        channelId,
        pLineObj->calLineData.icr2Values[0], pLineObj->calLineData.icr2Values[1],
        pLineObj->calLineData.icr2Values[2], pLineObj->calLineData.icr2Values[3]));

    /* Calibration to start at 20ma, incremented to 25, 32, 40mA later */
    dcFeed20mA[0] = pLineObj->calLineData.dcFeedRef[0];
    dcFeed20mA[1] = (pLineObj->calLineData.dcFeedRef[1] & ~VP880_ILA_MASK) + 2;

    VP_CALIBRATION(VpLineCtxType, pLineCtx, ("Vp880VocInit: Register 0x%02X 0x%02X Channel %d",
        dcFeed20mA[0], dcFeed20mA[1], channelId));

    VpMemCpy(pLineObj->calLineData.dcFeed, pLineObj->calLineData.dcFeedRef,
        VP880_DC_FEED_LEN);

    VpMemCpy(pLineObj->calLineData.dcFeedPr, pLineObj->calLineData.dcFeedRef,
        VP880_DC_FEED_LEN);

    VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_DC_FEED_WRT, VP880_DC_FEED_LEN,
        dcFeed20mA);

    if (pDevObj->stateInt & VP880_IS_ABS) {
        VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_ICR2_WRT, VP880_ICR2_LEN,
            icr2);
    }

    VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_TX_PCM_DATA_RD,
        VP880_TX_PCM_DATA_LEN, xdataTemp);
    tempNew = ( (xdataTemp[0] << 8) | xdataTemp[1]);
    VP_CALIBRATION(VpLineCtxType, pLineCtx,("2. Vp880AdcSettling(adcConfig = 0x%02X): AdcPcm %d ecVal %d",
        converterCfg[0], tempNew, ecVal[channelId]));

    /* Sense VAB Voltage. Can't use the value yet, so need to wait. */
    VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_CONV_CFG_WRT, VP880_CONV_CFG_LEN,
        converterCfg);

    /* Advanced to state for ADC calibration */
    pLineObj->calLineData.calState = VP880_CAL_ADC;

    /* Start timer to cause internal line calibration function to execute */
    pLineObj->lineTimers.timers.timer[VP_LINE_CAL_LINE_TIMER] =
        MS_TO_TICKRATE(VP880_CAL_VOC_SHORT,
            pDevObj->devProfileData.tickRate) | VP_ACTIVATE_TIMER;
} /* end Vp880VocInit */

/**
 * Vp880VocSetAdc ()
 *  This function set the converter to read the right pcm
 *  and set the right state machine , takes care for Pol Rev
 *
 * Preconditions:
 *  The device and line context must be created and initialized before calling
 * this function.
 *
 * Postconditions:
 *  This function generates an event upon completing the requested action.
 */
void
Vp880VocSetAdc(
    VpLineCtxType *pLineCtx)
{
    Vp880LineObjectType *pLineObj = pLineCtx->pLineObj;
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpDeviceIdType deviceId = pDevObj->deviceId;
    uint8 ecVal[] = {VP880_EC_CH1, VP880_EC_CH2};
    uint8 channelId = pLineObj->channelId;
    int16 vocVolt;
    uint8 dcCal[VP880_DC_CAL_REG_LEN];

    /* Read the Normal VOC Value raw from PCM */
    vocVolt = Vp880AdcSettling(deviceId, ecVal[channelId], VP880_METALLIC_DC_V);

    if (pLineObj->calLineData.reversePol == FALSE) {
        VP_CALIBRATION(VpLineCtxType, pLineCtx, ("1. VOC Norm (10mV): %d Channel %d",
            (int16)(vocVolt  * VP880_V_PCM_LSB/VP880_V_SCALE), channelId));
         pLineObj->calLineData.typeData.vocData.vocNorm = vocVolt;
    } else {
        VP_CALIBRATION(VpLineCtxType, pLineCtx, ("1. VOC Rev: %d Channel %d",
            (int16)(vocVolt  * VP880_V_PCM_LSB/VP880_V_SCALE), channelId));
        pLineObj->calLineData.typeData.vocData.vocRev = vocVolt;
    }

    /* Setup channel for Normal ILA and VAB offset read. Allow VAB to collapse */
    /* Disable TRDC sense -- forces ILA when on-hook */
    VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_DC_CAL_REG_RD,
        VP880_DC_CAL_REG_LEN, dcCal);

    if (pDevObj->stateInt & VP880_IS_ABS) {
        dcCal[0] &= VP880_DC_CAL_ABS_MASK;
        dcCal[1] &= VP880_DCCAL_BAT_SW_HYST_MASK;
        dcCal[1] |= (VP880_C_RING_SNS_CUT | VP880_C_TIP_SNS_CUT);
    } else {
        dcCal[1] |= (VP880_C_RING_SNS_CUT | VP880_C_TIP_SNS_CUT | VP880_DCCAL_BAT_SW_HYST_5V);
    }
    dcCal[0] |= VP880_DC_CAL_OFFSET_OVRD;
    dcCal[1] |= VP880_DC_CAL_DIS_INPUT_OFFSET;

    VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_DC_CAL_REG_WRT,
        VP880_DC_CAL_REG_LEN, dcCal);

    /* Advance to next state, where we can read Metalic Current */
    pLineObj->calLineData.calState = VP880_CAL_FIRST_IMT_SET;

    /*
     * Wait for line to settle before making converter configuration change.
     * Otherwise, it can change back to T/R AC voltage.
     */
    pLineObj->lineTimers.timers.timer[VP_LINE_CAL_LINE_TIMER] =
        MS_TO_TICKRATE(VP880_CAL_VOC_LONG,
            pDevObj->devProfileData.tickRate) | VP_ACTIVATE_TIMER;
} /* end Vp880VocSetAdc */

/**
 * Vp880VocOffset ()
 *
 *  This function computes and save the offset value for the ABV calibration
 *
 * Preconditions:
 *  The device and line context must be created and initialized before calling
 * this function.
 *
 * Postconditions:
 *
 */
void
Vp880VocOffset(
    VpLineCtxType *pLineCtx)
{
    Vp880LineObjectType *pLineObj = pLineCtx->pLineObj;
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpDeviceIdType deviceId = pDevObj->deviceId;
    uint8 ecVal[] = {VP880_EC_CH1, VP880_EC_CH2};
    uint8 channelId = pLineObj->channelId;
    int16 imtValue, ilgValue, vabOffset, vagOffset, vbgOffset;
    uint8 icr2Mods[VP880_ICR2_LEN];
    uint8 dcCal[VP880_DC_CAL_REG_LEN];
    uint8 converterCfg[VP880_CONV_CFG_LEN];

    switch (pLineObj->calLineData.calState) {
        case VP880_CAL_FIRST_IMT_SET:
            converterCfg[0] = VP880_METALLIC_DC_I;

            /* Force the converter change. Don't care about data yet. */
            VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_CONV_CFG_WRT,
                VP880_CONV_CFG_LEN, converterCfg);

            /* Update the state */
            pLineObj->calLineData.calState = VP880_CAL_FIRST_IMT_READ;

            /* Wait for converter and data to stabilize */
            pLineObj->lineTimers.timers.timer[VP_LINE_CAL_LINE_TIMER] =
                MS_TO_TICKRATE(VP880_CAL_VOC_SHORT,
                    pDevObj->devProfileData.tickRate) | VP_ACTIVATE_TIMER;
            return;

        case VP880_CAL_FIRST_IMT_READ:
            /* First read the metallic current. Should be "high". */
            imtValue = Vp880AdcSettling(deviceId, ecVal[channelId],
                VP880_METALLIC_DC_I);

            if (pLineObj->calLineData.reversePol == FALSE) {
                uint8 dcFeedRegister[VP880_DC_FEED_LEN];

                VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_DC_FEED_RD,
                    VP880_DC_FEED_LEN, dcFeedRegister);
                dcFeedRegister[1] &= ~VP880_ILA_MASK;

                if (pDevObj->vp880SysCalData.ila20[channelId] == 0) {
                    pDevObj->vp880SysCalData.ila20[channelId] = imtValue;
                    dcFeedRegister[1] |= (VP880_ILA_MASK & 7);
                    VP_CALIBRATION(VpLineCtxType, pLineCtx, ("ILA PCM Measured 20mA: %d Converted (10uA) %d Channel %d",
                        imtValue, (uint16)(imtValue * 100 / VP880_ILA_SCALE_1MA), channelId));
                } else if (pDevObj->vp880SysCalData.ila25[channelId] == 0) {
                    pDevObj->vp880SysCalData.ila25[channelId] = imtValue;
                    dcFeedRegister[1] |= (VP880_ILA_MASK & 14);
                    VP_CALIBRATION(VpLineCtxType, pLineCtx, ("ILA Measured 25mA: %d Converted (10uA) %d Channel %d",
                        imtValue, (uint16)(imtValue * 100 / VP880_ILA_SCALE_1MA), channelId));
                } else if (pDevObj->vp880SysCalData.ila32[channelId] == 0) {
                    pDevObj->vp880SysCalData.ila32[channelId] = imtValue;
                    dcFeedRegister[1] |= (VP880_ILA_MASK & 22);
                    VP_CALIBRATION(VpLineCtxType, pLineCtx, ("ILA Measured 32mA: %d Converted (10uA) %d Channel %d",
                        imtValue, (uint16)(imtValue * 100 / VP880_ILA_SCALE_1MA), channelId));
                } else if (pDevObj->vp880SysCalData.ila40[channelId] == 0) {
                    pDevObj->vp880SysCalData.ila40[channelId] = imtValue;
                    dcFeedRegister[1] = pLineObj->calLineData.dcFeedRef[1];
                    VP_CALIBRATION(VpLineCtxType, pLineCtx, ("ILA Measured 40mA: %d Converted (10uA) %d Channel %d",
                        imtValue, (uint16)(imtValue * 100 / VP880_ILA_SCALE_1MA), channelId));
                }

                VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_DC_FEED_WRT,
                    VP880_DC_FEED_LEN, dcFeedRegister);

                /* Wait for converter to stabilize */
                pLineObj->lineTimers.timers.timer[VP_LINE_CAL_LINE_TIMER] =
                    MS_TO_TICKRATE(VP880_CAL_VOC_SHORT,
                        pDevObj->devProfileData.tickRate) | VP_ACTIVATE_TIMER;

                /* Continue to ILG only if all ILA calibration done */
                if (pDevObj->vp880SysCalData.ila40[channelId] == 0) {
                    return;
                }
            }

            converterCfg[0] = VP880_LONGITUDINAL_DC_I;

            /* Force the converter change. Don't care about data yet. */
            VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_CONV_CFG_WRT,
                VP880_CONV_CFG_LEN, converterCfg);

            /* Update the state */
            pLineObj->calLineData.calState = VP880_CAL_FIRST_ILG_READ;
            return;

        case VP880_CAL_FIRST_ILG_READ:
            /* First read the longitudinal current. */
            ilgValue = Vp880AdcSettling(deviceId, ecVal[channelId],
                VP880_LONGITUDINAL_DC_I);
            if (pLineObj->calLineData.reversePol == FALSE) {
                VP_CALIBRATION(VpLineCtxType, pLineCtx, ("ILG Norm: %d Channel %d",
                    ilgValue, channelId));
                pLineObj->calLineData.typeData.ilgData.ilgNorm = ilgValue;
            } else {
                /*
                 * Don't care about ilg in reverse polarity. It's not calibrated
                 * because device drive capability is better than the device
                 * measurement capability.
                 */
            }

            converterCfg[0] = VP880_METALLIC_DC_V;

            /* Force the converter change. Don't care about data yet. */
            VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_CONV_CFG_WRT,
                VP880_CONV_CFG_LEN, converterCfg);

            /* Update the state */
            pLineObj->calLineData.calState = VP880_CAL_FIRST_VOC_READ;

            /* Wait for converter to stabilize */
            pLineObj->lineTimers.timers.timer[VP_LINE_CAL_LINE_TIMER] =
                MS_TO_TICKRATE(VP880_CAL_VOC_SHORT,
                    pDevObj->devProfileData.tickRate) | VP_ACTIVATE_TIMER;
            return;

        case VP880_CAL_FIRST_VOC_READ: {
            uint8 sigGenA[VP880_SIGA_PARAMS_LEN] =  /* set almost 0V */
                {0x00, 0x00, 0x00, 0x0A, 0xAB, 0x00, 0x02, 0x00, 0x00 ,0x00, 0x00};
            uint8 slacState[VP880_SYS_STATE_LEN] = {VP880_SS_BALANCED_RINGING};
            uint8 calReg[VP880_DC_CAL_REG_LEN] = {0x00, 0x02};

            /* Read the VAB Offset Voltage (should be 0 at this point) */
            vabOffset = Vp880AdcSettling(deviceId, ecVal[channelId],
                VP880_METALLIC_DC_V);
            if (pLineObj->calLineData.reversePol == FALSE) {
                VP_CALIBRATION(VpLineCtxType, pLineCtx, ("VOC Norm Offset (10mV): %d Channel %d",
                    (int16)(vabOffset  * VP880_V_PCM_LSB/VP880_V_SCALE), channelId));
                pDevObj->vp880SysCalData.vocOffset[channelId][VP880_NORM_POLARITY] = vabOffset;

                /* This is to be backward compatible with VVA P1.3.0 */
                pLineObj->calLineData.typeData.vocData.vocOffsetNorm = vabOffset;
            } else {
                VP_CALIBRATION(VpLineCtxType, pLineCtx, ("VOC Rev Offset (10mV): %d Channel %d",
                    (int16)(vabOffset  * VP880_V_PCM_LSB/VP880_V_SCALE), channelId));
                pDevObj->vp880SysCalData.vocOffset[channelId][VP880_REV_POLARITY] = vabOffset;

                /* This is to be backward compatible with VVA P1.3.0 */
                pLineObj->calLineData.typeData.vocData.vocOffsetRev = vabOffset;
            }

            /* Save the registers */
            VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_SIGA_PARAMS_RD,
                VP880_SIGA_PARAMS_LEN, pLineObj->calLineData.sigGenA);
            VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_SYS_STATE_RD,
                VP880_SYS_STATE_LEN, pLineObj->calLineData.sysState);
            VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_DC_CAL_REG_RD,
                VP880_DC_CAL_REG_LEN, pLineObj->calLineData.calReg);

            /* Update the state to be able to calibrate the signal generator */
            VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_DC_CAL_REG_WRT,
                VP880_DC_CAL_REG_LEN, calReg);
            VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_SIGA_PARAMS_WRT,
                VP880_SIGA_PARAMS_LEN, sigGenA);
            VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_SYS_STATE_WRT,
                VP880_SYS_STATE_LEN, slacState);

            /* Update the state */
            pLineObj->calLineData.calState = VP880_CAL_SIGEN_A_PHASE1;

            /* Wait for converter to stabilize */
            pLineObj->lineTimers.timers.timer[VP_LINE_CAL_LINE_TIMER] =
                MS_TO_TICKRATE(VP880_CAL_VOC_SHORT,
                    pDevObj->devProfileData.tickRate) | VP_ACTIVATE_TIMER;
            return;
        }

        case VP880_CAL_SIGEN_A_PHASE1:
            converterCfg[0] = VP880_METALLIC_DC_V;
            /* Force the converter change. Don't care about data yet. */
            VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_CONV_CFG_WRT,
                VP880_CONV_CFG_LEN, converterCfg);

            /* Update the state */
            pLineObj->calLineData.calState = VP880_CAL_SIGEN_A_PHASE2;

            /* Wait for converter to stabilize */
            pLineObj->lineTimers.timers.timer[VP_LINE_CAL_LINE_TIMER] =
                MS_TO_TICKRATE(VP880_CAL_VOC_SHORT,
                pDevObj->devProfileData.tickRate) | VP_ACTIVATE_TIMER;
            return;

        case VP880_CAL_SIGEN_A_PHASE2:
            vabOffset = Vp880AdcSettling(deviceId, ecVal[channelId], VP880_METALLIC_DC_V);

            pDevObj->vp880SysCalData.sigGenAError[channelId][0] = vabOffset;

            VP_CALIBRATION(VpLineCtxType, pLineCtx, ("SigGenA Offset (10mV): %d Channel %d",
                (int16)((pDevObj->vp880SysCalData.sigGenAError[channelId][VP880_NORM_POLARITY] -
                pDevObj->vp880SysCalData.vocOffset[channelId][VP880_NORM_POLARITY]) *
                VP880_V_PCM_LSB/VP880_V_SCALE), channelId));

            /* restore the registers */
            VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_SIGA_PARAMS_WRT,
                VP880_SIGA_PARAMS_LEN, pLineObj->calLineData.sigGenA);
            VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_SYS_STATE_WRT,
                VP880_SYS_STATE_LEN, pLineObj->calLineData.sysState);
            VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_DC_CAL_REG_WRT,
                VP880_DC_CAL_REG_LEN, pLineObj->calLineData.calReg);

            /* Update the state */
            pLineObj->calLineData.calState = VP880_CAL_SIGEN_A_PHASE3;

            /* Wait for converter to stabilize */
            pLineObj->lineTimers.timers.timer[VP_LINE_CAL_LINE_TIMER] =
                MS_TO_TICKRATE(VP880_CAL_VOC_SHORT,
                pDevObj->devProfileData.tickRate) | VP_ACTIVATE_TIMER;
            return;

        case VP880_CAL_SIGEN_A_PHASE3:
            converterCfg[0] = VP880_TIP_TO_GND_V;
            /* Force the converter change. Don't care about data yet. */
            VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_CONV_CFG_WRT,
                VP880_CONV_CFG_LEN, converterCfg);

            /* Update the state */
            pLineObj->calLineData.calState = VP880_CAL_FIRST_VAG_READ;

            /* Wait for converter to stabilize */
            pLineObj->lineTimers.timers.timer[VP_LINE_CAL_LINE_TIMER] =
                MS_TO_TICKRATE(VP880_CAL_VOC_SHORT,
                pDevObj->devProfileData.tickRate) | VP_ACTIVATE_TIMER;
            return;

        case VP880_CAL_FIRST_VAG_READ:
            /* Read the VAG Offset Voltage */
            vagOffset = Vp880AdcSettling(deviceId, ecVal[channelId],
                VP880_TIP_TO_GND_V);
            if (pLineObj->calLineData.reversePol == FALSE) {
                VP_CALIBRATION(VpLineCtxType, pLineCtx, ("VAG Norm Offset: %d Channel %d",
                    vagOffset, channelId));
                pDevObj->vp880SysCalData.vagOffsetNorm[channelId] = vagOffset;

                /* This is to be backward compatible with VVA P1.3.0 */
                pLineObj->calLineData.typeData.vagData.vagOffsetNorm = vagOffset;
            } else {
                VP_CALIBRATION(VpLineCtxType, pLineCtx, ("VAG Rev Offset: %d Channel %d",
                    vagOffset, channelId));
                pDevObj->vp880SysCalData.vagOffsetRev[channelId] = vagOffset;

                /* This is to be backward compatible with VVA P1.3.0 */
                pLineObj->calLineData.typeData.vagData.vagOffsetRev = vagOffset;
            }

            converterCfg[0] = VP880_RING_TO_GND_V;

            /* Force the converter change. Don't care about data yet. */
            VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_CONV_CFG_WRT,
                VP880_CONV_CFG_LEN, converterCfg);

            /* Update the state */
            pLineObj->calLineData.calState = VP880_CAL_FIRST_VBG_READ;

            /* Wait for converter to stabilize */
            pLineObj->lineTimers.timers.timer[VP_LINE_CAL_LINE_TIMER] =
                MS_TO_TICKRATE(VP880_CAL_VOC_SHORT,
                    pDevObj->devProfileData.tickRate) | VP_ACTIVATE_TIMER;
            return;

        case VP880_CAL_FIRST_VBG_READ:
            /* Read the VBG Offset Voltage */
            vbgOffset = Vp880AdcSettling(deviceId, ecVal[channelId],
                VP880_RING_TO_GND_V);
            if (pLineObj->calLineData.reversePol == FALSE) {
                VP_CALIBRATION(VpLineCtxType, pLineCtx, ("VBG Norm Offset: %d Channel %d",
                    vbgOffset, channelId));
                pDevObj->vp880SysCalData.vbgOffsetNorm[channelId] = vbgOffset;

                /* This is to compatible with VVA P1.3.0 */
                pLineObj->calLineData.typeData.vbgData.vbgOffsetNorm = vbgOffset;
            } else {
                VP_CALIBRATION(VpLineCtxType, pLineCtx, ("VBG Rev Offset: %d Channel %d",
                    vbgOffset, channelId));
                pDevObj->vp880SysCalData.vbgOffsetRev[channelId] = vbgOffset;

                /* This is to compatible with VVA P1.3.0 */
                pLineObj->calLineData.typeData.vbgData.vbgOffsetRev = vbgOffset;
            }
            break;

        default:
            break;
    }


    /*
     * Disable the VOC DAC, re-enable the TRDC sense. Then need to wait before
     * switching converter.
     */
    VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_ICR2_RD, VP880_ICR2_LEN,
        icr2Mods);

    VP_CALIBRATION(VpLineCtxType, pLineCtx, ("CAL VOC Offset: Channel %d: Read ICR2 0x%02X 0x%02X 0x%02X 0x%02X",
        channelId, icr2Mods[0], icr2Mods[1], icr2Mods[2], icr2Mods[3]));

    icr2Mods[VP880_ICR2_VOC_DAC_INDEX] |= VP880_ICR2_VOC_DAC_SENSE;
    icr2Mods[VP880_ICR2_VOC_DAC_INDEX+1] &= ~VP880_ICR2_VOC_DAC_SENSE;
    VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_ICR2_WRT, VP880_ICR2_LEN,
        icr2Mods);

    VP_CALIBRATION(VpLineCtxType, pLineCtx, ("CAL VOC Offset: Channel %d: Writing ICR2 0x%02X 0x%02X 0x%02X 0x%02X",
        channelId, icr2Mods[0], icr2Mods[1], icr2Mods[2], icr2Mods[3]));

    VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_DC_CAL_REG_RD,
        VP880_DC_CAL_REG_LEN, dcCal);
    dcCal[1] &= ~(VP880_C_RING_SNS_CUT | VP880_C_TIP_SNS_CUT);
    dcCal[1] |= VP880_DC_CAL_DIS_INPUT_OFFSET;
    VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_DC_CAL_REG_WRT,
        VP880_DC_CAL_REG_LEN, dcCal);

    /* Advance to next state */
    pLineObj->calLineData.calState = VP880_CAL_IMT_OFFSET_SET;

    /*
     * Wait for line to stabilize before attempting converter change. Otherwise
     * it will go back to T/R AC Voltage.
     */
    pLineObj->lineTimers.timers.timer[VP_LINE_CAL_LINE_TIMER] =
        MS_TO_TICKRATE(VP880_CAL_VOC_LONG,
            pDevObj->devProfileData.tickRate) | VP_ACTIVATE_TIMER;
} /* end Vp880VocOffset */

/**
 * Vp880VocDone()
 *  This function end the calibration operation for VOC associated with all the
 * lines of a device. See VP-API reference guide for more information.
 *
 * Preconditions:
 *  The device and line context must be created and initialized before calling
 * this function.
 *
 * Postconditions:
 *  This function generates an event upon completing the requested action.
 */
void
Vp880VocDone(
    VpLineCtxType *pLineCtx)
{
    Vp880LineObjectType *pLineObj = pLineCtx->pLineObj;
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpDeviceIdType deviceId = pDevObj->deviceId;
    uint8 ecVal[] = {VP880_EC_CH1, VP880_EC_CH2};
    uint8 channelId = pLineObj->channelId;
    int16 imtValue, ilgValue, vocValue;

    uint8 converterCfg[VP880_CONV_CFG_LEN];
    uint8 dcCal[VP880_DC_CAL_REG_LEN];

    switch (pLineObj->calLineData.calState) {
        case VP880_CAL_IMT_OFFSET_SET_DONE:
            converterCfg[0] = VP880_METALLIC_DC_I;

            /* Force the converter change. Don't care about data yet. */
            VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_CONV_CFG_WRT,
                VP880_CONV_CFG_LEN, converterCfg);

            pLineObj->calLineData.calState = VP880_CAL_IMT_OFFSET_READ_DONE;

            pLineObj->lineTimers.timers.timer[VP_LINE_CAL_LINE_TIMER] =
                MS_TO_TICKRATE(VP880_CAL_VOC_SHORT,
                    pDevObj->devProfileData.tickRate) | VP_ACTIVATE_TIMER;
            return;

        case VP880_CAL_IMT_OFFSET_READ_DONE:
            if (pLineObj->calLineData.reversePol == FALSE) {
                imtValue = Vp880AdcSettling(deviceId, ecVal[channelId],
                    VP880_METALLIC_DC_I);
                VP_CALIBRATION(VpLineCtxType, pLineCtx, ("ILA OffsetNorm %d Channel %d",
                    imtValue, channelId));
                pDevObj->vp880SysCalData.ilaOffsetNorm[channelId] = imtValue;

                /* This is to be compatible with VVA P1.3.0 */
                pLineObj->calLineData.typeData.ilaData.ilaOffsetNorm = imtValue;
            } else {
                /*
                 * The error in reverse polarity is less than what can actually
                 * be measured. So it's better to ignore what is being read and
                 * assume the part is "perfect".
                 */
            }

            converterCfg[0] = VP880_LONGITUDINAL_DC_I;

            /* Force the converter change. Don't care about data yet. */
            VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_CONV_CFG_WRT,
                VP880_CONV_CFG_LEN, converterCfg);

            pLineObj->calLineData.calState = VP880_CAL_ILG_OFFSET_READ_DONE;

            pLineObj->lineTimers.timers.timer[VP_LINE_CAL_LINE_TIMER] =
                MS_TO_TICKRATE(VP880_CAL_VOC_SHORT,
                    pDevObj->devProfileData.tickRate) | VP_ACTIVATE_TIMER;
            return;

        case VP880_CAL_ILG_OFFSET_READ_DONE:
            if (pLineObj->calLineData.reversePol == FALSE) {
                ilgValue = Vp880AdcSettling(deviceId, ecVal[channelId],
                    VP880_METALLIC_DC_I);
                VP_CALIBRATION(VpLineCtxType, pLineCtx, ("ILG OffsetNorm %d Channel %d",
                    ilgValue, channelId));
                pDevObj->vp880SysCalData.ilgOffsetNorm[channelId] = ilgValue;

                /* This is to be compatible with VVA P1.3.0 */
                pLineObj->calLineData.typeData.ilgData.ilgOffsetNorm = ilgValue;
            } else {
                /*
                 * The error in reverse polarity is less than what can actually
                 * be measured. So it's better to ignore what is being read and
                 * assume the part is "perfect".
                 */
            }

            converterCfg[0] = VP880_METALLIC_DC_V;

            /* Force the converter change. Don't care about data yet. */
            VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_CONV_CFG_WRT,
                VP880_CONV_CFG_LEN, converterCfg);

            pLineObj->calLineData.calState = VP880_CAL_VOC_READ_DONE;

            pLineObj->lineTimers.timers.timer[VP_LINE_CAL_LINE_TIMER] =
                MS_TO_TICKRATE(VP880_CAL_VOC_SHORT,
                    pDevObj->devProfileData.tickRate) | VP_ACTIVATE_TIMER;
            return;

        case VP880_CAL_VOC_READ_DONE:
            vocValue = Vp880AdcSettling(deviceId, ecVal[channelId],
                VP880_METALLIC_DC_V);

            if (pLineObj->calLineData.reversePol == FALSE) {
                VP_CALIBRATION(VpLineCtxType, pLineCtx, ("2. VOC Norm (10mV): %d Channel %d",
                    (int16)(vocValue  * VP880_V_PCM_LSB/VP880_V_SCALE), channelId));

                pLineObj->calLineData.typeData.vocData.vocNorm = vocValue;
            } else {
                VP_CALIBRATION(VpLineCtxType, pLineCtx, ("2. VOC Rev (10mV): %d Channel %d",
                    (int16)(vocValue  * VP880_V_PCM_LSB/VP880_V_SCALE), channelId));
                pLineObj->calLineData.typeData.vocData.vocRev = vocValue;
            }
            break;

        default:
            break;
    }

    /* Make adjustments */
    Vp880AdjustVoc(pLineCtx, ((pLineObj->calLineData.dcFeedRef[0] >> 2) & 0x7), FALSE);
    Vp880AdjustIla(pLineCtx, (pLineObj->calLineData.dcFeedRef[1] & VP880_ILA_MASK));

    /* Move on to VAS Calibration... */
    pLineObj->lineState.calType = VP_CSLAC_CAL_VAS;

    if (pDevObj->stateInt & VP880_IS_ABS) {
        /* ...at state done if ABS device */
        pLineObj->calLineData.calState = VP880_CAL_DONE;
    } else {
        pLineObj->calLineData.calState = VP880_CAL_INIT;
    }

    /* Disable TDC and RDC pin input offset control override */
    VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_DC_CAL_REG_RD,
        VP880_DC_CAL_REG_LEN, dcCal);
    dcCal[0] &= ~VP880_DC_CAL_OFFSET_OVRD;
    dcCal[1] &= ~VP880_DC_CAL_DIS_INPUT_OFFSET;
    VpMpiCmdWrapper(deviceId, ecVal[channelId], VP880_DC_CAL_REG_WRT,
        VP880_DC_CAL_REG_LEN, dcCal);

    pLineObj->lineTimers.timers.timer[VP_LINE_CAL_LINE_TIMER] =
       (MS_TO_TICKRATE(VP880_VAS_INIT_WAIT,
           pDevObj->devProfileData.tickRate)) | VP_ACTIVATE_TIMER;

} /* end Vp880VOCDone */

/**
 * Vp880AdjustIla()
 *  This function adjusts the line object data for the adjusted ILA value. No
 * changes are made however if ILA calibration was not previously done.
 *
 * Preconditions:
 *  The device and line context must be created and initialized before calling
 * this function.
 *
 * Postconditions:
 *  If previous calibration done, return TRUE and adjust line object data.
 * Otherwise, return FALSE and no line object change made.
 */
bool
Vp880AdjustIla(
    VpLineCtxType *pLineCtx,
    uint8 targetIla)
{
    Vp880LineObjectType *pLineObj = pLineCtx->pLineObj;
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    uint8 channelId = pLineObj->channelId;
    int32 imtMeasured = 0;
    uint8 imtTarget;

    int16 ilaError, imtActual;
    uint8 ilaAdjust;

    if (targetIla < 4) {            /* 18mA to < 22mA */
        imtMeasured = pDevObj->vp880SysCalData.ila20[channelId];
        imtTarget = 20;
    } else if (targetIla < 10) {    /* 23mA to < 28mA */
        imtMeasured = pDevObj->vp880SysCalData.ila25[channelId];
        imtTarget = 25;
    } else if (targetIla < 18) {    /* 29mA to < 36mA */
        imtMeasured = pDevObj->vp880SysCalData.ila32[channelId];
        imtTarget = 32;
    } else {                        /* 36mA and higher */
        imtMeasured = pDevObj->vp880SysCalData.ila40[channelId];
        imtTarget = 40;
    }

    if (imtMeasured == 0) {
        return FALSE;
    }

    imtActual = imtMeasured - pDevObj->vp880SysCalData.ilaOffsetNorm[channelId];
    ilaError = imtActual - (imtTarget * VP880_ILA_SCALE_1MA);

    /* ILA Scale: 500uA = 273 at PCM */
    if (ABS(ilaError) >= (VP880_ILA_SCALE_1MA / 2)) {
        uint8 tempIlaValue = pLineObj->calLineData.dcFeed[VP880_ILA_INDEX] & VP880_ILA_MASK;
        int8 tempLowValue = (int8)tempIlaValue;
        ilaAdjust = ((ABS(ilaError)+(VP880_ILA_SCALE_1MA / 2)) / VP880_ILA_SCALE_1MA);
        if (ilaError < 0) {
            tempIlaValue += ilaAdjust;
            if (tempIlaValue <= VP880_ILA_MASK) {
                pLineObj->calLineData.dcFeed[VP880_ILA_INDEX] += ilaAdjust;
            } else {
                pLineObj->calLineData.dcFeed[VP880_ILA_INDEX] |= VP880_ILA_MASK;
            }
        } else {
            tempLowValue -= ilaAdjust;
            if (tempLowValue >= 0) {
                pLineObj->calLineData.dcFeed[VP880_ILA_INDEX] -= ilaAdjust;
            } else {
                pLineObj->calLineData.dcFeed[VP880_ILA_INDEX] &= ~VP880_ILA_MASK;
            }
        }
    }

    VP_CALIBRATION(VpLineCtxType, pLineCtx, ("Chan %d: ILA Actual Norm (10uA) %d",
        channelId,
        (int16)(imtActual * 100 / VP880_ILA_SCALE_1MA)));

    VP_CALIBRATION(VpLineCtxType, pLineCtx, ("Chan %d: ILA Target (10uA) %d ILA Error Norm (10uA) %d",
        channelId,
        (int16)(((targetIla + 18) * 100)),
        (int16)(ilaError * 100 / VP880_ILA_SCALE_1MA)));

    return TRUE;
}

/**
 * Vp880AdjustVoc()
 *  This function adjusts the line object data for the adjusted VOC value. No
 * changes are made however if VOC calibration was not previously done.
 *
 * Preconditions:
 *  The device and line context must be created and initialized before calling
 * this function.
 *
 * Postconditions:
 *  If previous calibration done, return TRUE and adjust line object data.
 * Otherwise, return FALSE and no line object change made.
 */
bool
Vp880AdjustVoc(
    VpLineCtxType *pLineCtx,
    uint8 targetVoc,
    bool previousCal)
{
    Vp880LineObjectType *pLineObj = pLineCtx->pLineObj;
    uint8 channelId = pLineObj->channelId;
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;

    int16 vocActual, vocActualRev;
    int32 pcmTargetVoc;

    int16 vocErrorList[NUM_POLARITY];
    uint8 vocErrIndex;

    uint8 *dcFeedByte[NUM_POLARITY];

    dcFeedByte[VP880_NORM_POLARITY] = &(pLineObj->calLineData.dcFeed[0]);
    dcFeedByte[VP880_REV_POLARITY] = &(pLineObj->calLineData.dcFeedPr[0]);

    pcmTargetVoc = (int32)(targetVoc * 3);
    pcmTargetVoc += 36;
    pcmTargetVoc *= VP880_V_1V_SCALE;
    pcmTargetVoc /= VP880_V_1V_RANGE;

    if (previousCal == FALSE) {
        vocActual =
            pLineObj->calLineData.typeData.vocData.vocNorm
          - pDevObj->vp880SysCalData.vocOffset[channelId][VP880_NORM_POLARITY];

        vocActualRev =
            pLineObj->calLineData.typeData.vocData.vocRev
          - pDevObj->vp880SysCalData.vocOffset[channelId][VP880_REV_POLARITY];

        /*
         * Target is always positive. Normal feed is positive. Negative error means
         * voltage is too low (magnitude), positive means too high (magnitude).
         */
        pDevObj->vp880SysCalData.vocError[channelId][VP880_NORM_POLARITY] = (vocActual - (int16)pcmTargetVoc);

        VP_CALIBRATION(VpLineCtxType, pLineCtx, ("(In 10mV): VOC Norm %d Rev %d OffsetNorm %d OffsetRev %d Channel %d",
            (int16)(pLineObj->calLineData.typeData.vocData.vocNorm  * VP880_V_PCM_LSB/VP880_V_SCALE),
            (int16)(pLineObj->calLineData.typeData.vocData.vocRev  * VP880_V_PCM_LSB/VP880_V_SCALE),
            (int16)(pDevObj->vp880SysCalData.vocOffset[channelId][VP880_NORM_POLARITY]  * VP880_V_PCM_LSB/VP880_V_SCALE),
            (int16)(pDevObj->vp880SysCalData.vocOffset[channelId][VP880_REV_POLARITY]  * VP880_V_PCM_LSB/VP880_V_SCALE),
            channelId));

        VP_CALIBRATION(VpLineCtxType, pLineCtx, ("Chan %d: VOC (10mV) Actual Norm %d Rev Norm %d",
            channelId,
            (int16)(vocActual  * VP880_V_PCM_LSB/VP880_V_SCALE),
            (int16)(vocActualRev  * VP880_V_PCM_LSB/VP880_V_SCALE)));

        /*
         * Target is always positive. Reverse feed is negative. Negative error means
         * voltage is too low (magnitude), positive means too high (magnitude).
         */
        pDevObj->vp880SysCalData.vocError[channelId][VP880_REV_POLARITY] = (-vocActualRev - (int16)pcmTargetVoc);

        VP_CALIBRATION(VpLineCtxType, pLineCtx, ("Chan %d: VOC Target %d VOC Error Norm %d Error Rev %d",
            channelId,
            (int16)((targetVoc * 3 + 36) * 100),
            (int16)(pDevObj->vp880SysCalData.vocError[channelId][VP880_NORM_POLARITY]  * VP880_V_PCM_LSB/VP880_V_SCALE),
            (int16)(pDevObj->vp880SysCalData.vocError[channelId][VP880_REV_POLARITY]  * VP880_V_PCM_LSB/VP880_V_SCALE)));

        VP_CALIBRATION(VpLineCtxType, pLineCtx, ("Chan %d: DC Feed Values Normal Before 0x%02X 0x%02X",
            channelId, pLineObj->calLineData.dcFeed[0],
            pLineObj->calLineData.dcFeed[1]));

        VP_CALIBRATION(VpLineCtxType, pLineCtx, ("Chan %d: DC Feed Values Reverse Before 0x%02X 0x%02X",
            channelId, pLineObj->calLineData.dcFeedPr[0],
            pLineObj->calLineData.dcFeedPr[1]));
    }

    /*
     * Adjust if error is more than 1/2 a step size for each parameter based
     * on PCM scale.
     */
    vocErrorList[VP880_NORM_POLARITY] = pDevObj->vp880SysCalData.vocError[channelId][VP880_NORM_POLARITY];
    vocErrorList[VP880_REV_POLARITY] = pDevObj->vp880SysCalData.vocError[channelId][VP880_REV_POLARITY];

    for (vocErrIndex = 0; vocErrIndex < NUM_POLARITY; vocErrIndex++) {
        /* VOC Scale: 1.5V = 204.8 at PCM. Adjust to account for bit shift */

        if (ABS(vocErrorList[vocErrIndex]) >= 205) {
            if (vocErrorList[vocErrIndex] < 0) {
                /* Error is low, so need to increase VOC */

                /* Saturate the value, to prevent the rollover */
                if ((*dcFeedByte[vocErrIndex] & VP880_VOC_MASK) !=
                    VP880_VOC_MASK) {

                    /* Not saturated within scale. So can adjust up */
                    *dcFeedByte[vocErrIndex] += 0x04;

                } else if ((*dcFeedByte[vocErrIndex] & VP880_VOC_LOW_RANGE) ==
                    VP880_VOC_LOW_RANGE) {

                    /*
                     * Saturated within scale, but not within device. Change
                     * scale (moves up 3V) and clear incremental values or we'll
                     *  end up at the top of the high range.
                     */
                    *dcFeedByte[vocErrIndex] &= ~VP880_VOC_MASK;
                    *dcFeedByte[vocErrIndex] &= ~VP880_VOC_LOW_RANGE;
                }
            } else {
                /* Error is high, so need to decrease VOC */

                /* Saturate the value, to prevent the rollover */
                if ((*dcFeedByte[vocErrIndex] & VP880_VOC_MASK) != 0x00) {
                    /* Not saturated within scale. So can adjust down */
                    *dcFeedByte[vocErrIndex] -= 0x04;
                } else if ((*dcFeedByte[vocErrIndex] & VP880_VOC_LOW_RANGE) !=
                    VP880_VOC_LOW_RANGE) {

                    /*
                     * Saturated within scale, but not within device. Change
                     * scale (moves down 3V) and max incremental values or we'll
                     *  end up at the bottom of the low range.
                     */
                    *dcFeedByte[vocErrIndex] |= VP880_VOC_MASK;
                    *dcFeedByte[vocErrIndex] |= VP880_VOC_LOW_RANGE;
                }
            }
        }
    }

    VP_CALIBRATION(VpLineCtxType, pLineCtx, ("Chan %d: DC Feed Values Normal After 0x%02X 0x%02X",
        channelId, pLineObj->calLineData.dcFeed[0],
        pLineObj->calLineData.dcFeed[1]));

    VP_CALIBRATION(VpLineCtxType, pLineCtx, ("Chan %d: DC Feed Values Reverse After 0x%02X 0x%02X",
        channelId, pLineObj->calLineData.dcFeedPr[0],
        pLineObj->calLineData.dcFeedPr[1]));

    return TRUE;
}

/**
 * Vp880Cal()
 *  This function calibrates a selected block of the device/line.
 *
 * Preconditions:
 *  The device and line context must be created and initialized before calling
 * this function.
 *
 * Postconditions:
 *  This function generates an event upon completing the requested action.
 */
VpStatusType
Vp880Cal(
    VpLineCtxType       *pLineCtx,
    VpCalType           calType,
    void                *inputArgs)
{
    Vp880LineObjectType *pLineObj = pLineCtx->pLineObj;
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpDeviceIdType deviceId = pDevObj->deviceId;
    VpStatusType status = VP_STATUS_SUCCESS;
    uint8 profileIndex;
    uint8 *profileData;

    VpSysEnterCritical(deviceId, VP_CODE_CRITICAL_SEC);

    switch(calType) {
        case VP_CAL_GET_SYSTEM_COEFF:
            if (pDevObj->stateInt & VP880_SYS_CAL_COMPLETE) {
                /* Data length is header (6 bytes) +  880 calibration data */
                pDevObj->mpiLen = 6 + VP880_CAL_STRUCT_SIZE;

                pLineObj->responseData = (uint8)VP_CAL_GET_SYSTEM_COEFF;
                pLineObj->lineEvents.response |= VP_EVID_CAL_CMP;
            } else {
                status = VP_STATUS_LINE_NOT_CONFIG;
            }
            break;

        case VP_CAL_APPLY_SYSTEM_COEFF:
            profileData = (uint8 *)inputArgs;
            if ((profileData == VP_NULL) ||
                (profileData[VP_PROFILE_TYPE_LSB] != VP_PRFWZ_PROFILE_CAL) ||
                (profileData[VP_PROFILE_TYPE_MSB] != VP_DEV_880_SERIES)) {
                VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
                return VP_STATUS_INVALID_ARG;
            }

            if (profileData[VP_PROFILE_LENGTH] < (VP880_CAL_STRUCT_SIZE + 2)) {
                VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
                return VP_STATUS_INVALID_ARG;
            }

            profileIndex = VP_PROFILE_DATA_START;

            pDevObj->vp880SysCalData.abvError[0] = VpConvertToInt16(&profileData[profileIndex]);
            profileIndex+=2;
            VP_CALIBRATION(VpDevCtxType, pDevCtx, ("ABV Y Error %d", pDevObj->vp880SysCalData.abvError[0]));

            pDevObj->vp880SysCalData.abvError[1] = VpConvertToInt16(&profileData[profileIndex]);
            profileIndex+=2;
            VP_CALIBRATION(VpDevCtxType, pDevCtx, ("ABV Z Error %d", pDevObj->vp880SysCalData.abvError[1]));

            pDevObj->vp880SysCalData.vocOffset[0][0] = VpConvertToInt16(&profileData[profileIndex]);
            profileIndex+=2;
            VP_CALIBRATION(VpDevCtxType, pDevCtx, ("VOC Offset Norm Ch 0 %d", pDevObj->vp880SysCalData.vocOffset[0][0]));

            pDevObj->vp880SysCalData.vocError[0][0] = VpConvertToInt16(&profileData[profileIndex]);
            profileIndex+=2;
            VP_CALIBRATION(VpDevCtxType, pDevCtx, ("VOC Error Norm Ch 0 %d", pDevObj->vp880SysCalData.vocError[0][0]));

            pDevObj->vp880SysCalData.vocOffset[0][1] = VpConvertToInt16(&profileData[profileIndex]);
            profileIndex+=2;
            VP_CALIBRATION(VpDevCtxType, pDevCtx, ("VOC Offset Rev Ch 0 %d", pDevObj->vp880SysCalData.vocOffset[0][1]));

            pDevObj->vp880SysCalData.vocError[0][1] = VpConvertToInt16(&profileData[profileIndex]);
            profileIndex+=2;
            VP_CALIBRATION(VpDevCtxType, pDevCtx, ("VOC Error Rev Ch 0 %d", pDevObj->vp880SysCalData.vocError[0][1]));

            pDevObj->vp880SysCalData.vocOffset[1][0] = VpConvertToInt16(&profileData[profileIndex]);
            profileIndex+=2;
            VP_CALIBRATION(VpDevCtxType, pDevCtx, ("VOC Offset Norm Ch 1 %d", pDevObj->vp880SysCalData.vocOffset[1][0]));

            pDevObj->vp880SysCalData.vocError[1][0] = VpConvertToInt16(&profileData[profileIndex]);
            profileIndex+=2;
            VP_CALIBRATION(VpDevCtxType, pDevCtx, ("VOC Error Norm Ch 1 %d", pDevObj->vp880SysCalData.vocError[1][0]));

            pDevObj->vp880SysCalData.vocOffset[1][1] = VpConvertToInt16(&profileData[profileIndex]);
            profileIndex+=2;
            VP_CALIBRATION(VpDevCtxType, pDevCtx, ("VOC Offset Rev Ch 1 %d", pDevObj->vp880SysCalData.vocOffset[1][1]));

            pDevObj->vp880SysCalData.vocError[1][1] = VpConvertToInt16(&profileData[profileIndex]);
            profileIndex+=2;
            VP_CALIBRATION(VpDevCtxType, pDevCtx, ("VOC Error Rev Ch 1 %d", pDevObj->vp880SysCalData.vocError[1][1]));

            pDevObj->vp880SysCalData.sigGenAError[0][0] = VpConvertToInt16(&profileData[profileIndex]);
            profileIndex+=2;
            VP_CALIBRATION(VpDevCtxType, pDevCtx, ("SigGenA Norm Ch 0 Error %d", pDevObj->vp880SysCalData.sigGenAError[0][0]));

            pDevObj->vp880SysCalData.sigGenAError[0][1] = VpConvertToInt16(&profileData[profileIndex]);
            profileIndex+=2;
            VP_CALIBRATION(VpDevCtxType, pDevCtx, ("SigGenA Rev Ch 0 Error %d", pDevObj->vp880SysCalData.sigGenAError[0][1]));

            pDevObj->vp880SysCalData.sigGenAError[1][0] = VpConvertToInt16(&profileData[profileIndex]);
            profileIndex+=2;
            VP_CALIBRATION(VpDevCtxType, pDevCtx, ("SigGenA Norm Ch 1 Error %d", pDevObj->vp880SysCalData.sigGenAError[1][0]));

            pDevObj->vp880SysCalData.sigGenAError[1][1] = VpConvertToInt16(&profileData[profileIndex]);
            profileIndex+=2;
            VP_CALIBRATION(VpDevCtxType, pDevCtx, ("SigGenA Rev Ch 1 Error %d", pDevObj->vp880SysCalData.sigGenAError[1][1]));

            pDevObj->vp880SysCalData.ila20[0] = VpConvertToInt16(&profileData[profileIndex]);
            profileIndex+=2;
            VP_CALIBRATION(VpDevCtxType, pDevCtx, ("ILA 20mA Ch 0 %d", pDevObj->vp880SysCalData.ila20[0]));

            pDevObj->vp880SysCalData.ila20[1] = VpConvertToInt16(&profileData[profileIndex]);
            profileIndex+=2;
            VP_CALIBRATION(VpDevCtxType, pDevCtx, ("ILA 20mA Ch 1 %d", pDevObj->vp880SysCalData.ila20[1]));

            pDevObj->vp880SysCalData.ila25[0] = VpConvertToInt16(&profileData[profileIndex]);
            profileIndex+=2;
            VP_CALIBRATION(VpDevCtxType, pDevCtx, ("ILA 25mA Ch 0 %d", pDevObj->vp880SysCalData.ila25[0]));

            pDevObj->vp880SysCalData.ila25[1] = VpConvertToInt16(&profileData[profileIndex]);
            profileIndex+=2;
            VP_CALIBRATION(VpDevCtxType, pDevCtx, ("ILA 25mA Ch 1 %d", pDevObj->vp880SysCalData.ila25[1]));

            pDevObj->vp880SysCalData.ila32[0] = VpConvertToInt16(&profileData[profileIndex]);
            profileIndex+=2;
            VP_CALIBRATION(VpDevCtxType, pDevCtx, ("ILA 32mA Ch 0 %d", pDevObj->vp880SysCalData.ila32[0]));

            pDevObj->vp880SysCalData.ila32[1] = VpConvertToInt16(&profileData[profileIndex]);
            profileIndex+=2;
            VP_CALIBRATION(VpDevCtxType, pDevCtx, ("ILA 32mA Ch 1 %d", pDevObj->vp880SysCalData.ila32[1]));

            pDevObj->vp880SysCalData.ila40[0] = VpConvertToInt16(&profileData[profileIndex]);
            profileIndex+=2;
            VP_CALIBRATION(VpDevCtxType, pDevCtx, ("ILA 40mA Ch 0 %d", pDevObj->vp880SysCalData.ila40[0]));

            pDevObj->vp880SysCalData.ila40[1] = VpConvertToInt16(&profileData[profileIndex]);
            profileIndex+=2;
            VP_CALIBRATION(VpDevCtxType, pDevCtx, ("ILA 40mA Ch 1 %d", pDevObj->vp880SysCalData.ila40[1]));

            pDevObj->vp880SysCalData.ilaOffsetNorm[0] = VpConvertToInt16(&profileData[profileIndex]);
            profileIndex+=2;
            VP_CALIBRATION(VpDevCtxType, pDevCtx, ("ILA Offset Ch 0 %d", pDevObj->vp880SysCalData.ilaOffsetNorm[0]));

            pDevObj->vp880SysCalData.ilaOffsetNorm[1] = VpConvertToInt16(&profileData[profileIndex]);
            profileIndex+=2;
            VP_CALIBRATION(VpDevCtxType, pDevCtx, ("ILA Offset Ch 1 %d", pDevObj->vp880SysCalData.ilaOffsetNorm[1]));

            pDevObj->vp880SysCalData.ilgOffsetNorm[0] = VpConvertToInt16(&profileData[profileIndex]);
            profileIndex+=2;
            VP_CALIBRATION(VpDevCtxType, pDevCtx, ("ILG Offset Ch 0 %d", pDevObj->vp880SysCalData.ilgOffsetNorm[0]));

            pDevObj->vp880SysCalData.ilgOffsetNorm[1] = VpConvertToInt16(&profileData[profileIndex]);
            profileIndex+=2;
            VP_CALIBRATION(VpDevCtxType, pDevCtx, ("ILG Offset Ch 1 %d", pDevObj->vp880SysCalData.ilgOffsetNorm[1]));

            pDevObj->vp880SysCalData.vas[0][0] = profileData[profileIndex++];
            VP_CALIBRATION(VpDevCtxType, pDevCtx, ("VAS Norm Ch 0 %d", pDevObj->vp880SysCalData.vas[0][0]));

            pDevObj->vp880SysCalData.vas[0][1] = profileData[profileIndex++];
            VP_CALIBRATION(VpDevCtxType, pDevCtx, ("VAS Rev Ch 0 %d", pDevObj->vp880SysCalData.vas[0][1]));

            pDevObj->vp880SysCalData.vas[1][0] = profileData[profileIndex++];
            VP_CALIBRATION(VpDevCtxType, pDevCtx, ("VAS Norm Ch 1 %d", pDevObj->vp880SysCalData.vas[1][0]));

            pDevObj->vp880SysCalData.vas[1][1] = profileData[profileIndex++];
            VP_CALIBRATION(VpDevCtxType, pDevCtx, ("VAS Rev Ch 1 %d", pDevObj->vp880SysCalData.vas[1][1]));

            pDevObj->vp880SysCalData.vagOffsetNorm[0] = VpConvertToInt16(&profileData[profileIndex]);
            profileIndex+=2;
            VP_CALIBRATION(VpDevCtxType, pDevCtx, ("VAG Offset Norm Ch 0 %d", pDevObj->vp880SysCalData.vagOffsetNorm[0]));

            pDevObj->vp880SysCalData.vagOffsetNorm[1] = VpConvertToInt16(&profileData[profileIndex]);
            profileIndex+=2;
            VP_CALIBRATION(VpDevCtxType, pDevCtx, ("VAG Offset Norm Ch 1 %d", pDevObj->vp880SysCalData.vagOffsetNorm[1]));

            pDevObj->vp880SysCalData.vagOffsetRev[0] = VpConvertToInt16(&profileData[profileIndex]);
            profileIndex+=2;
            VP_CALIBRATION(VpDevCtxType, pDevCtx, ("VAG Offset Rev Ch 0 %d", pDevObj->vp880SysCalData.vagOffsetRev[0]));

            pDevObj->vp880SysCalData.vagOffsetRev[1] = VpConvertToInt16(&profileData[profileIndex]);
            profileIndex+=2;
            VP_CALIBRATION(VpDevCtxType, pDevCtx, ("VAG Offset Rev Ch 1 %d", pDevObj->vp880SysCalData.vagOffsetRev[1]));

            pDevObj->vp880SysCalData.vbgOffsetNorm[0] = VpConvertToInt16(&profileData[profileIndex]);
            profileIndex+=2;
            VP_CALIBRATION(VpDevCtxType, pDevCtx, ("VBG Offset Norm Ch 0 %d", pDevObj->vp880SysCalData.vbgOffsetNorm[0]));

            pDevObj->vp880SysCalData.vbgOffsetNorm[1] = VpConvertToInt16(&profileData[profileIndex]);
            profileIndex+=2;
            VP_CALIBRATION(VpDevCtxType, pDevCtx, ("VBG Offset Norm Ch 1 %d", pDevObj->vp880SysCalData.vbgOffsetNorm[1]));

            pDevObj->vp880SysCalData.vbgOffsetRev[0] = VpConvertToInt16(&profileData[profileIndex]);
            profileIndex+=2;
            VP_CALIBRATION(VpDevCtxType, pDevCtx, ("VBG Offset Rev Ch 0 %d", pDevObj->vp880SysCalData.vbgOffsetRev[0]));

            pDevObj->vp880SysCalData.vbgOffsetRev[1] = VpConvertToInt16(&profileData[profileIndex]);
            profileIndex+=2;
            VP_CALIBRATION(VpDevCtxType, pDevCtx, ("VBG Offset Rev Ch 1 %d", pDevObj->vp880SysCalData.vbgOffsetRev[1]));

            pDevObj->vp880SysCalData.absNormCal[0] = profileData[profileIndex++];
            VP_CALIBRATION(VpDevCtxType, pDevCtx, ("Auto-Bat Switch Norm Ch 0 Cal %d", pDevObj->vp880SysCalData.absNormCal[0]));

            pDevObj->vp880SysCalData.absPolRevCal[0] = profileData[profileIndex++];
            VP_CALIBRATION(VpDevCtxType, pDevCtx, ("Auto-Bat Switch Rev Ch 0 Cal %d", pDevObj->vp880SysCalData.absPolRevCal[0]));

            pDevObj->vp880SysCalData.absNormCal[1] = profileData[profileIndex++];
            VP_CALIBRATION(VpDevCtxType, pDevCtx, ("Auto-Bat Switch Norm Ch 1 Cal %d", pDevObj->vp880SysCalData.absNormCal[1]));

            pDevObj->vp880SysCalData.absPolRevCal[1] = profileData[profileIndex++];
            VP_CALIBRATION(VpDevCtxType, pDevCtx, ("Auto-Bat Switch Rev Ch 1 Cal %d", pDevObj->vp880SysCalData.absPolRevCal[1]));

            pDevObj->vp880SysCalData.swyOffset[0] = VpConvertToInt16(&profileData[profileIndex]);
            profileIndex+=2;
            VP_CALIBRATION(VpDevCtxType, pDevCtx, ("SWY Offset Ch 0 %d", pDevObj->vp880SysCalData.swyOffset[0]));

            pDevObj->vp880SysCalData.swyOffset[1] = VpConvertToInt16(&profileData[profileIndex]);
            profileIndex+=2;
            VP_CALIBRATION(VpDevCtxType, pDevCtx, ("SWY Offset Ch 1 %d", pDevObj->vp880SysCalData.swyOffset[1]));

            pDevObj->vp880SysCalData.swzOffset[0] = VpConvertToInt16(&profileData[profileIndex]);
            profileIndex+=2;
            VP_CALIBRATION(VpDevCtxType, pDevCtx, ("SWZ Offset Ch 0 %d", pDevObj->vp880SysCalData.swzOffset[0]));

            pDevObj->vp880SysCalData.swzOffset[1] = VpConvertToInt16(&profileData[profileIndex]);
            profileIndex+=2;
            VP_CALIBRATION(VpDevCtxType, pDevCtx, ("SWZ Offset Ch 1 %d", pDevObj->vp880SysCalData.swzOffset[1]));

            pDevObj->vp880SysCalData.swxbOffset[0] = VpConvertToInt16(&profileData[profileIndex]);
            profileIndex+=2;
            VP_CALIBRATION(VpDevCtxType, pDevCtx, ("XB Offset Ch 0 %d", pDevObj->vp880SysCalData.swxbOffset[0]));

            pDevObj->vp880SysCalData.swxbOffset[1] = VpConvertToInt16(&profileData[profileIndex]);
            profileIndex+=2;
            VP_CALIBRATION(VpDevCtxType, pDevCtx, ("XB Offset Ch 1 %d", pDevObj->vp880SysCalData.swxbOffset[1]));

            pDevObj->vp880SysCalData.tipCapCal[0] = VpConvertToInt32(&profileData[profileIndex]);
            profileIndex+=4;
            VP_CALIBRATION(VpDevCtxType, pDevCtx, ("Tip Cap Ch 0 %li", pDevObj->vp880SysCalData.tipCapCal[0]));

            pDevObj->vp880SysCalData.tipCapCal[1] = VpConvertToInt32(&profileData[profileIndex]);
            profileIndex+=4;
            VP_CALIBRATION(VpDevCtxType, pDevCtx, ("Tip Cap Ch 1 %li", pDevObj->vp880SysCalData.tipCapCal[1]));

            pDevObj->vp880SysCalData.ringCapCal[0] = VpConvertToInt32(&profileData[profileIndex]);
            profileIndex+=4;
            VP_CALIBRATION(VpDevCtxType, pDevCtx, ("Ring Cap Ch 0 %li", pDevObj->vp880SysCalData.ringCapCal[0]));

            pDevObj->vp880SysCalData.ringCapCal[1] = VpConvertToInt32(&profileData[profileIndex]);
            profileIndex+=4;
            VP_CALIBRATION(VpDevCtxType, pDevCtx, ("Ring Cap Ch 1 %li", pDevObj->vp880SysCalData.ringCapCal[1]));

            VP_CALIBRATION(VpDevCtxType, pDevCtx, ("Calibration Data Length - %d", profileIndex));

            pDevObj->stateInt |= (VP880_SYS_CAL_COMPLETE | VP880_CAL_RELOAD_REQ);

            pLineObj->lineEvents.response |= VP_EVID_CAL_CMP;
            pLineObj->responseData = VP_CAL_SUCCESS;
            break;

        default:
            status = VP_STATUS_INVALID_ARG;
            break;
    }

    VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);

    return status;
}
#endif /* VP880_CAL_ENABLE */
#endif /* VP_CC_880_SERIES */

