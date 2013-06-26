/** \file vp890_common.c
 * vp890_common.c
 *
 * Copyright (c) 2010, Zarlink Semiconductor, Inc.
 *
 * $Revision: 6423 $
 * $LastChangedDate: 2010-02-12 17:01:34 -0600 (Fri, 12 Feb 2010) $
 */

#include    "vp_api.h"

#if defined (VP_CC_890_SERIES)  /* Compile only if required */

#include    "vp_api_int.h"
#include    "vp890_api_int.h"

static void
ConvertSignedFixed2Csd(
    int32                   fixed,
    uint8                   *csdBuf);


/*******************************************************************************
 * Vp890IsDevReady()
 * This function checks the state of the device against the device complete,
 * the device initilaztion in progress and the device in calibration flags.
 *
 * Arguments:
 *  state           -   current internal state of the device.
 *  checkCal        -   indication that the function needs to check for calibration
 *
 * Returns
 *  TRUE  - If device is ready
 *  FALSE - If the device is busy being initialized or calibrated.
 ******************************************************************************/
bool
Vp890IsDevReady(
    VpDeviceStateType   state,
    bool                checkCal)
{
    /* Proceed if device state is either in progress or complete */
    if (!(state & (VP_DEV_INIT_CMP | VP_DEV_INIT_IN_PROGRESS))) {
        VP_ERROR(None, VP_NULL, ("Vp890IsDevReady() - device not initilized!!!"));
        return FALSE;
    }
    return TRUE;
} /* Vp890IsDevReady() */


/*******************************************************************************
 * Vp890IsProfileValid()
 * This function checks the validity of a profile passed into the API via
 * a pointer or an index.
 *
 * Arguments:
 *  tableSize       -   size of the profile table being checked
 *  profEntry       -   value of the profile entry being checked
 *  profType        -   type of profile that is being checked
 *  pProfTable      -   pointer to profile table pointers profType
 *  pProfileInput   -   pointer to the profile being checked
 *  pProfileRslt    -   pointer to the resulting profile
 *
 * Preconditions:
 *
 * Postconditions:
 ******************************************************************************/
bool
Vp890IsProfileValid(
    VpProfileType       profType,
    int16               tableSize,
    uint16              profEntry,
    VpProfilePtrType    *pProfTable,
    VpProfilePtrType    pProfileInput,
    VpProfilePtrType    *pProfileRslt)
{
    int                 profIndex   = VpGetProfileIndex(pProfileInput);

    /* Fail if profile index is beyond legal table size */
    if (profIndex >= tableSize) {
        VP_ERROR(None, VP_NULL, ("IsProfileValid() - profIndex exceeds table size"));
        return FALSE;
    }

    /* Input profile is null, -- NULL is legal */
    if (pProfileInput == VP_PTABLE_NULL) {
        *pProfileRslt = VP_PTABLE_NULL;
        return TRUE;
    }

    if (profIndex < 0) {
        /* Is the input profile a vaild profile type? */
        if ( VpVerifyProfileType(profType, pProfileInput)) {
            *pProfileRslt = pProfileInput;
            return TRUE;
        }
    } else if (profIndex < tableSize) {
        /* Does the profile table contain a profile at the requested index? */
        if ((profEntry & (0x01 << profIndex))) {
            *pProfileRslt = pProfTable[profIndex];
            return TRUE;
        }
    }

    VP_ERROR(None, VP_NULL, ("IsProfileValid() - invalid profile"));
    return FALSE;
} /* Vp890IsProfileValid */


/*******************************************************************************
 * Vp890LoadCidBuffers()
 *  This function loads cid data into the line objects cid buffers
 *  If length is within the size of just the primary buffer size, then only
 *  fill the primary buffer. Otherwise (the length exceeds the size of the
 *  primary buffer size) "low fill" the primary buffer and max fill the
 *  secondary buffer. This has the affect of causing a CID Data event
 *  quickly and giving the application a maximum amount of time to refill
 *  the message buffer
 *
 * Arguments:
 *   length     -
 *   *pCid      -
 *   pCidData   -
 *
 * Preconditions:
 *  none are needed
 *
 * Postconditions:
 * Caller ID information is saved to a line objects primary and secondary
 * CID buffers.
 *
 ******************************************************************************/
void
Vp890LoadCidBuffers(
    uint8           length,
    VpCallerIdType  *pCid,
    uint8p          pCidData)
{
    uint8 byteCnt1, byteCnt2;

    if (length <= VP_SIZEOF_CID_MSG_BUFFER) {
        pCid->primaryMsgLen = length;
        pCid->secondaryMsgLen = 0;
    } else {
        pCid->primaryMsgLen = (length - VP_SIZEOF_CID_MSG_BUFFER);
        pCid->secondaryMsgLen = VP_SIZEOF_CID_MSG_BUFFER;
    }

    /*
     * Copy the message data to the primary API buffer. If we're here, there's
     * at least one byte of primary message data. So a check is not necessary
     */
    pCid->status |= VP_CID_PRIMARY_FULL;
    for (byteCnt1 = 0; byteCnt1 < pCid->primaryMsgLen; byteCnt1++) {
        pCid->primaryBuffer[byteCnt1] = pCidData[byteCnt1];
        pCid->cidCheckSum += pCidData[byteCnt1];
        pCid->cidCheckSum = pCid->cidCheckSum % 256;
    }

    /* Copy the message data to the secondary API buffer if there is any */
    if (pCid->secondaryMsgLen > 0) {
        pCid->status |= VP_CID_SECONDARY_FULL;
        for (byteCnt2 = 0; (byteCnt2 < pCid->secondaryMsgLen); byteCnt2++) {
            pCid->secondaryBuffer[byteCnt2] = pCidData[byteCnt2 + byteCnt1];
            pCid->cidCheckSum += pCidData[byteCnt2 + byteCnt1];
            pCid->cidCheckSum =  pCid->cidCheckSum % 256;
        }
    }
    return;
} /* Vp890LoadCidBuffers() */

#ifdef CSLAC_SEQ_EN

/*******************************************************************************
 * Vp890CommandInstruction()
 *  This function implements the Sequencer Command instruction for the Vp890
 * device type.
 *
 * Preconditions:
 *  The line must first be initialized and the sequencer data must be valid.
 *
 * Postconditions:
 *  The command instruction currently being pointed to by the sequencer
 * instruction passed is acted upon.  The sequencer may or may not be advanced,
 * depending on the specific command instruction being executed.
 ******************************************************************************/
VpStatusType
Vp890CommandInstruction(
    VpLineCtxType           *pLineCtx,
    VpProfilePtrType        pSeqData)
{
    Vp890LineObjectType     *pLineObj = pLineCtx->pLineObj;
    VpDevCtxType            *pDevCtx  = pLineCtx->pDevCtx;
    Vp890DeviceObjectType   *pDevObj  = pDevCtx->pDevObj;
    VpDeviceIdType          deviceId  = pDevObj->deviceId;

    uint8                   ecVal     = pLineObj->ecVal;
    uint8                   channelId = pLineObj->channelId;

    uint8                   lineState;
    uint8                   lsConfig[VP890_LOOP_SUP_LEN];
    uint16                  tempFreq, tempLevel;

    /*
     * We know the current value "pSeqData[0]" is 0, now we need to determine if
     * the next command is generator control operator followed by time, or a
     * Line state command -- No other options supported
     */
    VP_SEQUENCER(VpLineCtxType, pLineCtx, ("890 Command 0x%02X", pSeqData[0]));

    switch (pSeqData[0] & VP_SEQ_SUBTYPE_MASK) {
        case VP_SEQ_SUBCMD_SIGGEN:
            /* Set the generator enable bits to 0 without affecting the rest */
            pLineObj->sigGenCtrl[0] &= (VP890_GEN_ALLOFF | (~VP890_GEN_EN_MASK));
            pLineObj->sigGenCtrl[0] &= ~VP890_GEN_BIAS;

            /* Get the signal generator bits and set. */
            pLineObj->sigGenCtrl[0] |= ((pSeqData[1] & 0x01) ?  VP890_GENA_EN : 0);
            pLineObj->sigGenCtrl[0] |= ((pSeqData[1] & 0x02) ?  VP890_GENB_EN : 0);
            pLineObj->sigGenCtrl[0] |= ((pSeqData[1] & 0x04) ?  VP890_GENC_EN : 0);
            pLineObj->sigGenCtrl[0] |= ((pSeqData[1] & 0x08) ?  VP890_GEND_EN : 0);

            VpMpiCmdWrapper(deviceId, ecVal, VP890_GEN_CTRL_WRT,
                VP890_GEN_CTRL_LEN, pLineObj->sigGenCtrl);
            break;

        case VP_SEQ_SUBCMD_LINE_STATE:
            switch(pSeqData[1]) {
                case VP_PROFILE_CADENCE_STATE_STANDBY:
                    Vp890SetFxsLineState(pLineCtx, VP_LINE_STANDBY);
                    break;

                case VP_PROFILE_CADENCE_STATE_POLREV_STANDBY:
                    Vp890SetFxsLineState(pLineCtx, VP_LINE_STANDBY_POLREV);
                    break;

                case VP_PROFILE_CADENCE_STATE_TALK:
                    Vp890SetFxsLineState(pLineCtx, VP_LINE_TALK);
                    break;

                case VP_PROFILE_CADENCE_STATE_ACTIVE:
                    Vp890SetFxsLineState(pLineCtx, VP_LINE_ACTIVE);
                    break;

                case VP_PROFILE_CADENCE_STATE_OHT:
                    Vp890SetFxsLineState(pLineCtx, VP_LINE_OHT);
                    break;

                case VP_PROFILE_CADENCE_STATE_DISCONNECT:
                    Vp890SetFxsLineState(pLineCtx, VP_LINE_DISCONNECT);
                    break;

                case VP_PROFILE_CADENCE_STATE_RINGING:
                    Vp890SetFxsLineState(pLineCtx, VP_LINE_RINGING);
                    break;

                case VP_PROFILE_CADENCE_STATE_POLREV_RINGING:
                    Vp890SetFxsLineState(pLineCtx, VP_LINE_RINGING_POLREV);
                    break;

                case VP_PROFILE_CADENCE_STATE_POLREV_ACTIVE:
                    Vp890SetFxsLineState(pLineCtx, VP_LINE_ACTIVE_POLREV);
                    break;

                case VP_PROFILE_CADENCE_STATE_POLREV_TALK:
                    Vp890SetFxsLineState(pLineCtx, VP_LINE_TALK_POLREV);
                    break;

                case VP_PROFILE_CADENCE_STATE_POLREV_OHT:
                    Vp890SetFxsLineState(pLineCtx, VP_LINE_OHT_POLREV);
                    break;

                case VP_PROFILE_CADENCE_STATE_FXO_LOOP_OPEN:
                    Vp890SetFxoLineState(pLineCtx, VP_LINE_FXO_LOOP_OPEN);
                    break;

                case VP_PROFILE_CADENCE_STATE_FXO_OHT:
                    Vp890SetFxoLineState(pLineCtx, VP_LINE_FXO_OHT);
                    break;

                case VP_PROFILE_CADENCE_STATE_FXO_LOOP_CLOSE:
                    Vp890SetFxoLineState(pLineCtx, VP_LINE_FXO_LOOP_CLOSE);
                    break;

                case VP_PROFILE_CADENCE_STATE_FXO_LOOP_TALK:
                    Vp890SetFxoLineState(pLineCtx, VP_LINE_FXO_TALK);
                    break;

                case VP_PROFILE_CADENCE_STATE_MSG_WAIT_NORM:
                case VP_PROFILE_CADENCE_STATE_MSG_WAIT_POLREV:
                    VpMemCpy(lsConfig, pLineObj->loopSup, VP890_LOOP_SUP_LEN);
                    if (lsConfig[VP890_LOOP_SUP_RT_MODE_BYTE]
                        & VP890_RING_TRIP_AC) {
                        if (!(pLineObj->status & VP890_BAD_LOOP_SUP)) {
                            pLineObj->status |= VP890_BAD_LOOP_SUP;
                        }

                        /* Force DC Trip */
                        lsConfig[VP890_LOOP_SUP_RT_MODE_BYTE] &=
                            ~VP890_RING_TRIP_AC;
                        VpMpiCmdWrapper(deviceId, ecVal, VP890_LOOP_SUP_WRT,
                            VP890_LOOP_SUP_LEN, lsConfig);
                    }

                    lineState =
                        (pSeqData[1] == VP_PROFILE_CADENCE_STATE_MSG_WAIT_NORM) ?
                        VP890_SS_BALANCED_RINGING :
                        VP890_SS_BALANCED_RINGING_PR;

                    Vp890LLSetSysState(deviceId, pLineCtx, lineState, TRUE);
                    break;

                default:
                    VP_ERROR(VpLineCtxType, pLineCtx, ("Vp890CommandInstruction() - Invalid sequencer state command"));
                    return VP_STATUS_INVALID_ARG;
            }
            break;

        case VP_SEQ_SUBCMD_START_CID:
        case VP_SEQ_SUBCMD_WAIT_ON:
            if (pLineObj->pCidProfileType1 != VP_PTABLE_NULL) {
                pLineObj->callerId.pCliProfile = pLineObj->pCidProfileType1;

                pLineObj->callerId.status |= VP_CID_IN_PROGRESS;
                if ((pSeqData[0] & VP_SEQ_SUBTYPE_MASK) == VP_SEQ_SUBCMD_WAIT_ON) {
                    VP_CID(VpLineCtxType, pLineCtx, ("890 Command WAIT_ON_CID"));
                    pLineObj->callerId.status |= VP_CID_WAIT_ON_ACTIVE;
                } else {
                    VP_CID(VpLineCtxType, pLineCtx, ("890 Command START_CID"));
                }
                pLineObj->callerId.cliTimer = 1;

                pLineObj->callerId.cliIndex = 0;
                pLineObj->callerId.cliMPIndex = 0;
                pLineObj->callerId.cliMSIndex = 0;

                pLineObj->callerId.status &= ~VP_CID_SIG_B_VALID;

                pLineObj->callerId.status |= VP_CID_PRIMARY_IN_USE;
                pLineObj->callerId.status &= ~VP_CID_SECONDARY_IN_USE;
            }
            break;

        case VP_SEQ_SUBCMD_RAMP_GENERATORS:
            tempFreq = (pLineObj->cadence.regData[3] << 8);
            tempFreq |= pLineObj->cadence.regData[4];

            tempLevel = (pLineObj->cadence.regData[5] << 8);
            tempLevel |= pLineObj->cadence.regData[6];

            if (pLineObj->cadence.isFreqIncrease == TRUE) {
                /* Check if we're at or above the max frequency */
                if (tempFreq >= pLineObj->cadence.stopFreq) {
                    pLineObj->cadence.isFreqIncrease = FALSE;
                    tempFreq -= pLineObj->cadence.freqStep;
                } else {
                    tempFreq += pLineObj->cadence.freqStep;
                }
            } else {
                if (tempFreq <
                    (pLineObj->cadence.startFreq - pLineObj->cadence.freqStep)) {
                    pLineObj->cadence.isFreqIncrease = TRUE;
                    tempFreq += pLineObj->cadence.freqStep;
                } else {
                    tempFreq -= pLineObj->cadence.freqStep;
                }
            }
            pLineObj->cadence.regData[3] = (tempFreq >> 8) & 0xFF;
            pLineObj->cadence.regData[4] = tempFreq & 0xFF;

            /*
             * Check if we're at or above the max level, but make sure we don't
             * wrap around
             */
            if (tempLevel <
                (pLineObj->cadence.stopLevel -
                    ((tempLevel * pLineObj->cadence.levelStep) / 10000))) {

                tempLevel += ((tempLevel * pLineObj->cadence.levelStep) / 10000);

                pLineObj->cadence.regData[5] = (tempLevel >> 8) & 0xFF;
                pLineObj->cadence.regData[6] = tempLevel & 0xFF;
            }
            VpMpiCmdWrapper(deviceId, ecVal, VP890_SIGAB_PARAMS_WRT,
                VP890_SIGAB_PARAMS_LEN, pLineObj->cadence.regData);
            break;
        default:
            VP_ERROR(VpLineCtxType, pLineCtx, ("Vp890CommandInstruction() - Invalid sequencer command"));
            return VP_STATUS_INVALID_ARG;
    }

    /*
     * Check to see if there is more sequence data, and if so, move the
     * sequence pointer to the next command. Otherwise, end this cadence
     */
    pLineObj->cadence.index+=2;
    VP_SEQUENCER(VpLineCtxType, pLineCtx, ("New Index %d Length %d",
        pLineObj->cadence.index, pLineObj->cadence.length));

    if (pLineObj->cadence.index <
       (pLineObj->cadence.length + VP_PROFILE_LENGTH + 1)) {
        pSeqData+=2;
        pLineObj->cadence.pCurrentPos = pSeqData;

        VP_SEQUENCER(VpLineCtxType, pLineCtx, ("Continue Cadence..."));
    } else {
        VP_SEQUENCER(VpLineCtxType, pLineCtx, ("End Cadence"));

        switch(pLineObj->cadence.pActiveCadence[VP_PROFILE_TYPE_LSB]) {

            case VP_PRFWZ_PROFILE_RINGCAD:
                pLineObj->lineEvents.process |= VP_LINE_EVID_RING_CAD;
                pLineObj->processData = VP_RING_CAD_DONE;
                break;

            case VP_PRFWZ_PROFILE_TONECAD:
                pLineObj->lineEvents.process |= VP_LINE_EVID_TONE_CAD;
                break;

            case VP_PRFWZ_PROFILE_HOOK_FLASH_DIG_GEN:
                pLineObj->lineEvents.process |= VP_LINE_EVID_SIGNAL_CMP;
                pLineObj->processData = VP_SENDSIG_HOOK_FLASH;
                break;

            case VP_PRFWZ_PROFILE_DIAL_PULSE_DIG_GEN:
                pLineObj->lineEvents.process |= VP_LINE_EVID_SIGNAL_CMP;
                pLineObj->processData = VP_SENDSIG_PULSE_DIGIT;
                break;

            case VP_PRFWZ_PROFILE_MOMENTARY_LOOP_OPEN_INT:
                pLineObj->lineEvents.process |= VP_LINE_EVID_SIGNAL_CMP;
                pLineObj->processData = VP_SENDSIG_MOMENTARY_LOOP_OPEN;
                if (pDevObj->intReg[channelId] & VP890_LIU_MASK) {
                    pLineObj->lineEventHandle = 1;
                } else {
                    pLineObj->lineEventHandle = 0;
                }
                VpMpiCmdWrapper(deviceId, ecVal, VP890_LOOP_SUP_WRT,
                    VP890_LOOP_SUP_LEN, pLineObj->loopSup);
                break;

            case VP_PRFWZ_PROFILE_DTMF_DIG_GEN:
                pLineObj->lineEvents.process |= VP_LINE_EVID_SIGNAL_CMP;
                pLineObj->processData = VP_SENDSIG_DTMF_DIGIT;
                Vp890MuteChannel(pLineCtx, FALSE);
                break;

            case VP_PRFWZ_PROFILE_MSG_WAIT_PULSE_INT:
                pLineObj->lineEvents.process |= VP_LINE_EVID_SIGNAL_CMP;
                pLineObj->processData = VP_SENDSIG_MSG_WAIT_PULSE;
                VpSetLineState(pLineCtx, pLineObj->lineState.usrCurrent);
                break;

            default:
                break;

        }
        pLineObj->cadence.status = VP_CADENCE_RESET_VALUE;
        pLineObj->cadence.pActiveCadence = VP_PTABLE_NULL;
    }

    return VP_STATUS_SUCCESS;
} /* Vp890CommandInstruction() */

/*******************************************************************************
 * Vp890FSKGeneratorReady()
 *  This function is used for Caller ID to determine if the FSK generator is
 *  ready to accept another byte. It uses the device caller ID state machine
 *  and signaling (caller ID status) register. This function should be called
 *  from an API internal function only.
 *
 * Prototype is in vp_api_int.h
 *
 * Arguments:
 *
 * Preconditions:
 *
 * Returns:
 *  TRUE if the FSK generator for Caller ID can accept a byte, FALSE otherwise.
 ******************************************************************************/
bool
Vp890FSKGeneratorReady(
    VpLineCtxType         *pLineCtx)
{
    VpDevCtxType          *pDevCtx  = pLineCtx->pDevCtx;
    Vp890DeviceObjectType *pDevObj  = pDevCtx->pDevObj;
    Vp890LineObjectType   *pLineObj = pLineCtx->pLineObj;
    VpDeviceIdType        deviceId  = pDevObj->deviceId;
    uint8                 ecVal     = pLineObj->ecVal;
    uint8                 cidState;
    uint8                 cidParam[VP890_CID_PARAM_LEN];

    /* Check the Generator State */
    VpMpiCmdWrapper(deviceId, ecVal, VP890_CID_PARAM_RD,
        VP890_CID_PARAM_LEN, cidParam);
    cidState = (cidParam[0] & VP890_CID_STATE_MASK);

    /* Check to see if the device is ready to accept (more) CID data */
    if (cidState == VP890_CID_STATE_FULL_D) {
        VP_CID(VpLineCtxType, pLineCtx, ("890 FSK Generator FULL"));
        return FALSE;
    } else {
        VP_CID(VpLineCtxType, pLineCtx, ("890 FSK Generator READY"));
        return TRUE;
    }
} /* Vp890FSKGeneratorReady() */

/*******************************************************************************
 * Vp890CliGetEncodedByte()
 *  This function returns an encoded byte of data that is suitable for writing
 *  the FSK generator (device dependent).
 *
 * Preconditions
 *  Must have a valid CLI packet in to work from.
 *
 * Postconditions
 *  The per-channel caller ID buffer will be updated with encoded data.
 *
 ******************************************************************************/
bool
Vp890CliGetEncodedByte(
    VpLineCtxType           *pLineCtx,
    uint8                   *pByte)
{
    Vp890LineObjectType     *pLineObj       = pLineCtx->pLineObj;
    VpOptionEventMaskType   *pLineEvents    = &(pLineObj->lineEvents);
    VpCallerIdType          *pCidStruct     = &(pLineObj->callerId);

    uint8                   nextByte        = '\0';

    uint8                   checkSumIndex   = VP_CID_PROFILE_FSK_PARAM_LEN
                + pLineObj->callerId.pCliProfile[VP_CID_PROFILE_FSK_PARAM_LEN]
                + VP_CID_PROFILE_CHECKSUM_OFFSET_LSB;

    if (pLineObj->callerId.status & VP_CID_MID_CHECKSUM) {
        pLineObj->callerId.status &= ~VP_CID_MID_CHECKSUM;
        pCidStruct->status |= VP_CID_END_OF_MSG;
        *pByte = '\0';
        return FALSE;
    }

    /* Check to determine which buffer is in use to index the message data */
    if (pCidStruct->status & VP_CID_PRIMARY_IN_USE) {
        /*
         * If the index is at the length of the buffer, we need to switch
         * buffers if there is more data
         */
        if (pCidStruct->cliMPIndex >= pCidStruct->primaryMsgLen) {
            /*
             * At the end of the Primary Buffer. Flag an event and indicate to
             * the API that this buffer is no longer being used and we can
             * accept more data
             */
            pCidStruct->status &= ~VP_CID_PRIMARY_IN_USE;
            pCidStruct->status &= ~VP_CID_PRIMARY_FULL;

            if (pCidStruct->status & VP_CID_SECONDARY_FULL) {
                pLineEvents->process |= VP_LINE_EVID_CID_DATA;
                pLineObj->processData = VP_CID_DATA_NEED_MORE_DATA;

                pCidStruct->status |= VP_CID_SECONDARY_IN_USE;
                pCidStruct->cliMSIndex = 1;
                *pByte = pCidStruct->secondaryBuffer[0];
                nextByte = pCidStruct->secondaryBuffer[1];

                VP_CID(VpLineCtxType, pLineCtx, ("890 FSK Buffer -- Switching to Secondary 0x%02X",
                    *pByte));
            } else {
                if (pLineObj->callerId.pCliProfile[checkSumIndex]) {
                    *pByte = (uint8)(~pLineObj->callerId.cidCheckSum + 1);
                    pLineObj->callerId.status |= VP_CID_MID_CHECKSUM;

                    VP_CID(VpLineCtxType, pLineCtx, ("1. 890 FSK Buffer -- From Primary 0x%02X",
                        *pByte));
                } else {
                    *pByte = '\0';
                }
            }
        } else {
            *pByte = pCidStruct->primaryBuffer[pCidStruct->cliMPIndex];

            /* Get the next byte to be sent after the current byte */
            if ((pCidStruct->cliMPIndex+1) >= pCidStruct->primaryMsgLen) {
                if (pCidStruct->status & VP_CID_SECONDARY_FULL) {
                    nextByte = pCidStruct->secondaryBuffer[0];

                    VP_CID(VpLineCtxType, pLineCtx, ("890 FSK Buffer -- From Secondary 0x%02X",
                        *pByte));
                }
            } else {
                nextByte =
                    pCidStruct->primaryBuffer[pCidStruct->cliMPIndex+1];

                    VP_CID(VpLineCtxType, pLineCtx, ("2. 890 FSK Buffer -- From Primary 0x%02X",
                        *pByte));
            }
        }
        pCidStruct->cliMPIndex++;
    } else if (pCidStruct->status & VP_CID_SECONDARY_IN_USE) {
        /*
         * If the index is at the length of the buffer, we need to switch
         * buffers if there is more data
         */
        if (pCidStruct->cliMSIndex >= pCidStruct->secondaryMsgLen) {
            /*
             * At the end of the Secondary Buffer. Flag an event and indicate to
             * the API that this buffer is no longer being used and is empty
             */
            pLineEvents->process |= VP_LINE_EVID_CID_DATA;
            pLineObj->processData = VP_CID_DATA_NEED_MORE_DATA;

            pCidStruct->status &= ~VP_CID_SECONDARY_IN_USE;
            pCidStruct->status &= ~VP_CID_SECONDARY_FULL;

            if (pCidStruct->status & VP_CID_PRIMARY_FULL) {
                pLineEvents->process |= VP_LINE_EVID_CID_DATA;
                pLineObj->processData = VP_CID_DATA_NEED_MORE_DATA;

                pCidStruct->status |= VP_CID_PRIMARY_IN_USE;
                pCidStruct->cliMPIndex = 1;
                *pByte = pCidStruct->primaryBuffer[0];
                nextByte = pCidStruct->primaryBuffer[1];
                VP_CID(VpLineCtxType, pLineCtx, ("890 FSK Buffer -- Switching to Primary 0x%02X",
                    *pByte));
            } else {
                /* There is no more data in either buffer */
                if (pLineObj->callerId.pCliProfile[checkSumIndex]) {
                    *pByte = (uint8)(~pLineObj->callerId.cidCheckSum + 1);
                    pLineObj->callerId.status |= VP_CID_MID_CHECKSUM;
                    VP_CID(VpLineCtxType, pLineCtx, ("890 FSK Buffer -- Checksum 0x%02X",
                        *pByte));
                } else {
                    *pByte = '\0';
                }
            }
        } else {
            *pByte = pCidStruct->secondaryBuffer[pCidStruct->cliMSIndex];

            /* Get the next byte to be sent after the current byte */
            if ((pCidStruct->cliMSIndex+1) >= pCidStruct->secondaryMsgLen) {
                if (pCidStruct->status & VP_CID_PRIMARY_FULL) {
                    nextByte = pCidStruct->primaryBuffer[0];
                    VP_CID(VpLineCtxType, pLineCtx, ("3. 890 FSK Buffer -- From Primary 0x%02X",
                        *pByte));
                }
            } else {
                nextByte =
                    pCidStruct->secondaryBuffer[pCidStruct->cliMSIndex+1];
                    VP_CID(VpLineCtxType, pLineCtx, ("2. 890 FSK Buffer -- From Secondary 0x%02X",
                        *pByte));
            }
        }
        pCidStruct->cliMSIndex++;
    }

    if ((!(pCidStruct->status & VP_CID_PRIMARY_IN_USE))
     && (!(pCidStruct->status & VP_CID_SECONDARY_IN_USE))) {
        if(pCidStruct->status & VP_CID_MID_CHECKSUM) {
            return TRUE;
        } else {
            return FALSE;
        }
    }

    if ((nextByte == '\0')
    && (!(pLineObj->callerId.pCliProfile[checkSumIndex]))) {
        VP_CID(VpLineCtxType, pLineCtx, ("890 Caller ID -- EOM"));
        pCidStruct->status |= VP_CID_END_OF_MSG;
    }

    return TRUE;
} /* Vp890CliGetEncodedByte() */

/*******************************************************************************
 * Vp890CtrlSetCliTone()
 *  This function is called by the API internally to enable or disable the
 *  signal generator used for Caller ID.
 *
 * Preconditions:
 *  The line context must be valid (pointing to a Vp890 line object type
 *
 * Postconditions:
 *  The signal generator used for CID tones is enabled/disabled indicated by
 *  the mode parameter passed.
 *
 ******************************************************************************/
VpStatusType
Vp890CtrlSetCliTone(
    VpLineCtxType         *pLineCtx,
    bool                  mode)
{
    Vp890LineObjectType   *pLineObj = pLineCtx->pLineObj;
    VpDevCtxType          *pDevCtx  = pLineCtx->pDevCtx;
    Vp890DeviceObjectType *pDevObj  = pDevCtx->pDevObj;
    VpDeviceIdType        deviceId  = pDevObj->deviceId;
    uint8                 ecVal     = pLineObj->ecVal;

    /*
     * This function should only be called when the Caller ID sequence is
     * generating an alerting tone. We're using the C/D generators, so disable
     * A/B and enable C/D only (if mode == TRUE).
     */
    pLineObj->sigGenCtrl[0] &= ~(VP890_GEN_ALLON);
    pLineObj->sigGenCtrl[0] &= ~VP890_GEN_BIAS;

    if (mode == TRUE) {
        pLineObj->sigGenCtrl[0] |= (VP890_GENC_EN | VP890_GEND_EN);
    }
    VpMpiCmdWrapper(deviceId, ecVal, VP890_GEN_CTRL_WRT, VP890_GEN_CTRL_LEN,
        pLineObj->sigGenCtrl);

    return VP_STATUS_SUCCESS;
} /* Vp890CtrlSetCliTone() */

/*******************************************************************************
 * Vp890CtrlSetFSKGen()
 *  This function is called by the CID sequencer executed internally by the API
 *
 * Preconditions:
 *  The line context must be valid (pointing to a VP890 line object type
 *
 * Postconditions:
 *  The data indicated by mode and data is applied to the line. Mode is used
 * to indicate whether the data is "message", or a special character. The
 * special characters are "channel siezure" (alt. 1/0), "mark" (all 1), or
 * "end of transmission".
 *
 ******************************************************************************/
void
Vp890CtrlSetFSKGen(
    VpLineCtxType               *pLineCtx,
    VpCidGeneratorControlType   mode,
    uint8                       data)
{
    Vp890LineObjectType   *pLineObj = pLineCtx->pLineObj;
    VpDevCtxType          *pDevCtx  = pLineCtx->pDevCtx;
    Vp890DeviceObjectType *pDevObj  = pDevCtx->pDevObj;
    VpDeviceIdType        deviceId  = pDevObj->deviceId;
    uint8                 ecVal     = pLineObj->ecVal;
    bool                  moreData  = TRUE;
    uint8                 fskParam[VP890_CID_PARAM_LEN];

    VpMpiCmdWrapper(deviceId, ecVal, VP890_CID_PARAM_RD,
        VP890_CID_PARAM_LEN, fskParam);
    fskParam[0] &= ~(VP890_CID_FRAME_BITS);

    switch(mode) {
        case VP_CID_SIGGEN_EOT:
            if (data == 0) {
                /* Stop Transmission Immediately */
                Vp890MuteChannel(pLineCtx, FALSE);
                fskParam[0] |= VP890_CID_DIS;
            } else {
                /* Wait until the device is complete */
                pLineObj->suspendCid = TRUE;
            }
            moreData = FALSE;
            break;

        case VP_CID_GENERATOR_DATA:
            Vp890MuteChannel(pLineCtx, TRUE);

            fskParam[0] |= (VP890_CID_FB_START_0 | VP890_CID_FB_STOP_1);
            fskParam[0] &= ~(VP890_CID_DIS);
            if ((pLineObj->callerId.status & VP_CID_END_OF_MSG) ||
                (pLineObj->callerId.status & VP_CID_MID_CHECKSUM)) {
                fskParam[0] |= VP890_CID_EOM;
            } else {
                fskParam[0] &= ~(VP890_CID_EOM);
            }
            break;

        case VP_CID_GENERATOR_KEYED_CHAR:
            Vp890MuteChannel(pLineCtx, TRUE);
            fskParam[0] &= ~(VP890_CID_EOM | VP890_CID_DIS);

            switch(data) {
                case VP_FSK_CHAN_SEIZURE:
                    fskParam[0] |=
                        (VP890_CID_FB_START_0 | VP890_CID_FB_STOP_1);
                    break;

                case VP_FSK_MARK_SIGNAL:
                    fskParam[0] |=
                        (VP890_CID_FB_START_1 | VP890_CID_FB_STOP_1);
                    break;
                default:
                    break;
            }
            break;

        default:
            break;
    }

    VpMpiCmdWrapper(deviceId, ecVal, VP890_CID_PARAM_WRT,
        VP890_CID_PARAM_LEN, fskParam);

    if (moreData == TRUE) {
        VP_CID(VpLineCtxType, pLineCtx, ("890 Caller ID Data 0x%02X",
            data));

        VpMpiCmdWrapper(deviceId, ecVal, VP890_CID_DATA_WRT,
            VP890_CID_DATA_LEN, &data);
    }

    return;
} /* Vp890CtrlSetFSKGen() */

#endif /* CSLAC_SEQ_EN */

/*******************************************************************************
 * Vp890SetDTMFGenerators()
 *  This function sets signal generator A/B for DTMF tone generation.
 *
 * Preconditions:
 *  The line must first be initialized.
 *
 * Postconditions:
 *  The signal generators A/B are set to the DTMF frequencies and level required
 * by the digit passed.
 ******************************************************************************/
VpStatusType
Vp890SetDTMFGenerators(
    VpLineCtxType               *pLineCtx,
    VpCidGeneratorControlType   mode,
    VpDigitType                 digit)
{
    Vp890LineObjectType   *pLineObj = pLineCtx->pLineObj;
    VpDevCtxType          *pDevCtx  = pLineCtx->pDevCtx;
    Vp890DeviceObjectType *pDevObj  = pDevCtx->pDevObj;
    VpDeviceIdType        deviceId  = pDevObj->deviceId;
    uint8                 ecVal     = pLineObj->ecVal;

#ifdef CSLAC_SEQ_EN
    uint8 sigByteCount;
    uint8 sigOffset = VP_CID_PROFILE_FSK_PARAM_LEN + 2;
#endif

    uint8 sigGenCDParams[VP890_SIGCD_PARAMS_LEN] = {
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

#ifdef CSLAC_SEQ_EN
    /*
     * If we're generating caller ID data set the levels based on the data in
     * the CID profile
     */
    if ((pLineObj->callerId.status & VP_CID_IN_PROGRESS) &&
        (pLineObj->callerId.pCliProfile != VP_PTABLE_NULL)) {
        for (sigByteCount = 0; sigByteCount < (VP890_SIGCD_PARAMS_LEN - 3);
             sigByteCount++) {
            sigGenCDParams[sigByteCount+3] =
                pLineObj->callerId.pCliProfile[sigOffset + sigByteCount];
        }
    } else {
#endif
        /*
         * If it's an FXO line then the DTMF high and low frequency levels are
         * specified in the FXO/Dialing Profile, cached in the line object.
         */
        if (pLineObj->status & VP890_IS_FXO) {
            sigGenCDParams[2] = pLineObj->digitGenStruct.dtmfHighFreqLevel[0];
            sigGenCDParams[3] = pLineObj->digitGenStruct.dtmfHighFreqLevel[1];
            sigGenCDParams[6] = pLineObj->digitGenStruct.dtmfLowFreqLevel[0];
            sigGenCDParams[7] = pLineObj->digitGenStruct.dtmfLowFreqLevel[1];
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
            sigGenCDParams[0] = columnFreqs[0];
            sigGenCDParams[1] = columnFreqs[1];
            break;

        case 2:
        case 5:
        case 8:
        case VP_DIG_ZERO:
            sigGenCDParams[0] = columnFreqs[2];
            sigGenCDParams[1] = columnFreqs[3];
            break;

        case 3:
        case 6:
        case 9:
        case VP_DIG_POUND:
            sigGenCDParams[0] = columnFreqs[4];
            sigGenCDParams[1] = columnFreqs[5];
            break;

        case VP_DIG_A:
        case VP_DIG_B:
        case VP_DIG_C:
        case VP_DIG_D:
            sigGenCDParams[0] = columnFreqs[6];
            sigGenCDParams[1] = columnFreqs[7];
            break;

        default:
            VP_ERROR(VpLineCtxType, pLineCtx, ("Vp890SetDTMFGenerators() - Invalid digit"));
            return VP_STATUS_INVALID_ARG;
    }

    /* Now set the row freqs */
    switch(digit) {
        case 1:
        case 2:
        case 3:
        case VP_DIG_A:
            sigGenCDParams[4] = rowFreqs[0];
            sigGenCDParams[5] = rowFreqs[1];
            break;

        case 4:
        case 5:
        case 6:
        case VP_DIG_B:
            sigGenCDParams[4] = rowFreqs[2];
            sigGenCDParams[5] = rowFreqs[3];
            break;

        case 7:
        case 8:
        case 9:
        case VP_DIG_C:
            sigGenCDParams[4] = rowFreqs[4];
            sigGenCDParams[5] = rowFreqs[5];
            break;

        case VP_DIG_ASTER:
        case VP_DIG_ZERO:
        case VP_DIG_POUND:
        case VP_DIG_D:
            sigGenCDParams[4] = rowFreqs[6];
            sigGenCDParams[5] = rowFreqs[7];
            break;

        default:
            VP_ERROR(VpLineCtxType, pLineCtx, ("Vp890SetDTMFGenerators() - Invalid digit"));
            return VP_STATUS_INVALID_ARG;
    }

    VpMpiCmdWrapper(deviceId, ecVal, VP890_SIGCD_PARAMS_WRT,
        VP890_SIGCD_PARAMS_LEN, sigGenCDParams);

    /*
     * If there is no change to generator control required, it is assumed to be
     * set properly prior to this function call.
     */
    if (mode != VP_CID_NO_CHANGE) {
        pLineObj->sigGenCtrl[0] = VP890_GEN_ALLOFF;

        /*
         * For DTMF CID, the data passed may be message data, a keyed character
         * (e.g., Mark, Channel Seizure), or End of Transmission. If it's End
         * of Transmission, disable the DTMF generators immediately. Otherwise,
         * enable the DTMF generators
         */
        if ((mode == VP_CID_GENERATOR_DATA)
         || (mode == VP_CID_GENERATOR_KEYED_CHAR)) {
            pLineObj->sigGenCtrl[0] |= (VP890_GENC_EN | VP890_GEND_EN);

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
        pLineObj->sigGenCtrl[0] &= ~VP890_GEN_BIAS;
        VpMpiCmdWrapper(deviceId, ecVal, VP890_GEN_CTRL_WRT, VP890_GEN_CTRL_LEN,
            pLineObj->sigGenCtrl);
    }
    return VP_STATUS_SUCCESS;
} /* Vp890SetDTMFGenerators */

/*******************************************************************************
 * Vp890MuteChannel()
 *  This function disables or enables the PCM highway for the selected line and
 * should only be called by API internal functions.
 *
 * Arguments:
 *  pLineCtx    - Line affected
 *  mode        - TRUE = Disable TX/RX, FALSE = enable
 *
 * Preconditions:
 *  The line context must be valid (i.e., pointing to a valid Vp890 line object
 * type).
 *
 * Postconditions:
 *  If mode is TRUE the TX/RX path is cut. If FALSE, the TX/RX path is enabled
 * according to the current line state and mode used for talk states.
 ******************************************************************************/
void
Vp890MuteChannel(
    VpLineCtxType         *pLineCtx,
    bool                  mode)
{
    VpDevCtxType          *pDevCtx  = pLineCtx->pDevCtx;
    Vp890DeviceObjectType *pDevObj  = pDevCtx->pDevObj;
    Vp890LineObjectType   *pLineObj = pLineCtx->pLineObj;
    uint8                 ecVal     = pLineObj->ecVal;
    VpDeviceIdType        deviceId  = pDevObj->deviceId;

    uint8                 preState, postState, mpiByte;

    /*
     * Read the status of the Operating Conditions register so we can change
     * only the TX and RX if the line state is a non-communication mode.
     */
    VpMpiCmdWrapper(deviceId, ecVal, VP890_OP_COND_RD, VP890_OP_COND_LEN,
        &preState);
    postState = preState;
    postState &= ~(VP890_CUT_TXPATH | VP890_CUT_RXPATH);
    postState &= ~(VP890_HIGH_PASS_DIS | VP890_OPCOND_RSVD_MASK);

    /*
     * If disabling, simple. Otherwise enable based on the current line state
     * and the state of the "talk" option. The "talk" option is maintained in
     * the line object and abstracted in GetTxRxMode() function
     */

    Vp890GetTxRxPcmMode(pLineObj, pLineObj->lineState.currentState, &mpiByte);

    if (mode == TRUE) {
        /*
         * If awaiting DTMF detection, enable TX, disable RX. This is higher
         * priority than Mute mode. Otherwise, disable both TX and RX.
         */
        postState |= VP890_CUT_RXPATH;  /* Mute == TRUE always cuts RX path */
#ifdef CSLAC_SEQ_EN
        if (!(pLineObj->callerId.status & VP_CID_AWAIT_TONE)) {
#endif
            /* Not awaiting tone, TX Path is disabled as well */
            postState |= VP890_CUT_TXPATH;
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
            postState |= (VP890_CUT_RXPATH | VP890_CUT_TXPATH);
        } else  {
#endif
            postState |= mpiByte;
#ifdef CSLAC_SEQ_EN
        }
#endif
    }

    if (postState != preState) {
        VP_INFO(VpLineCtxType, pLineCtx, ("\n\r10. Writing 0x%02X to Op Conditions",
            postState));

        VpMpiCmdWrapper(deviceId, ecVal, VP890_OP_COND_WRT, VP890_OP_COND_LEN,
            &postState);
    }
    return;
} /* Vp890MuteChannel() */

/*******************************************************************************
 * Vp890GetTxRxPcmMode()
 *  This function returns the TX/RX PCM bits for the PCM (enable/disable) mode
 * corresponding to the state passed. The results should be or'-ed with the
 * bits set to 0 prior to calling this function.
 *
 * Arguments:
 *  state   - The state associating with PCM mode
 *  mpiByte - Device Specific byte
 *
 * Preconditions:
 *  None. Mapping function only.
 *
 * Postconditions:
 *  None. Mapping function only.
 ******************************************************************************/
VpStatusType
Vp890GetTxRxPcmMode(
    Vp890LineObjectType *pLineObj,
    VpLineStateType     state,
    uint8               *mpiByte)
{
    if (VP_OPTION_PCM_RX_ONLY == pLineObj->pcmTxRxCtrl) {
         *mpiByte = VP890_CUT_TXPATH;
    } else if (VP_OPTION_PCM_TX_ONLY == pLineObj->pcmTxRxCtrl) {
        *mpiByte = VP890_CUT_RXPATH;
    } else if (VP_OPTION_PCM_ALWAYS_ON == pLineObj->pcmTxRxCtrl) {
        *mpiByte = 0x00;
        return VP_STATUS_SUCCESS;
    } else {
        *mpiByte = 0x00;
    }

    switch(state) {
        /* Non-Talk States */
        case VP_LINE_STANDBY:
        case VP_LINE_STANDBY_POLREV:
        case VP_LINE_ACTIVE:
        case VP_LINE_ACTIVE_POLREV:
        case VP_LINE_DISCONNECT:
        case VP_LINE_RINGING:
        case VP_LINE_RINGING_POLREV:
            if (pLineObj->status & VP890_IS_FXO) {
                VP_ERROR(Vp890LineObjectType, pLineObj, ("Vp890GetTxRxPcmMode() - Invalid FXO state %02X", state));
                return VP_STATUS_INVALID_ARG;
            }
            *mpiByte |= (VP890_CUT_TXPATH | VP890_CUT_RXPATH);
            break;

        case VP_LINE_FXO_LOOP_OPEN:
        case VP_LINE_FXO_LOOP_CLOSE:
        case VP_LINE_FXO_RING_GND:
            if (!(pLineObj->status & VP890_IS_FXO)) {
                VP_ERROR(Vp890LineObjectType, pLineObj, ("Vp890GetTxRxPcmMode() - Invalid FXS state %02X", state));
                return VP_STATUS_INVALID_ARG;
            }
            *mpiByte |= (VP890_CUT_TXPATH | VP890_CUT_RXPATH);
            break;

        /* Talk States */
        case VP_LINE_TALK:
        case VP_LINE_TALK_POLREV:
        case VP_LINE_OHT:
        case VP_LINE_OHT_POLREV:
            if (pLineObj->status & VP890_IS_FXO) {
                VP_ERROR(Vp890LineObjectType, pLineObj, ("Vp890GetTxRxPcmMode() - Invalid FXO state %02X", state));
                return VP_STATUS_INVALID_ARG;
            }
            break;

        case VP_LINE_FXO_OHT:
        case VP_LINE_FXO_TALK:
            if (!(pLineObj->status & VP890_IS_FXO)) {
                VP_ERROR(Vp890LineObjectType, pLineObj, ("Vp890GetTxRxPcmMode() - Invalid FXS state %02X", state));
                return VP_STATUS_INVALID_ARG;
            }
            break;

        case VP_LINE_TIP_OPEN:
            return VP_STATUS_INVALID_ARG;

        default:
            break;
    }
    return VP_STATUS_SUCCESS;
} /* Vp890GetTxRxPcmMode() */

/*******************************************************************************
 * Vp890SetRelGainInt()
 *  This function adjusts the GR and GX values for a given channel of a given
 * device.  It multiplies the profile values by a user-specified factor and
 * possibly a callerID correction factor determined elsewhere in the API.  The
 * DTG bit of the VP_GAIN register will also be set for callerID correction.
 *
 * Arguments:
 *  *pLineCtx  -  Line context to change gains on
 *  rxLevel    -  Adjustment to line's relative Rx level
 *
 * Preconditions:
 *
 * Postconditions:
 *  Returns error if device is not initialized.
 *  GX and GR will be
 ******************************************************************************/
VpStatusType
Vp890SetRelGainInt(
    VpLineCtxType   *pLineCtx)
{
    Vp890LineObjectType     *pLineObj       = pLineCtx->pLineObj;
    Vp890DeviceObjectType   *pDevObj        = pLineCtx->pDevCtx->pDevObj;
    VpDeviceIdType          deviceId        = pDevObj->deviceId;
    VpRelGainResultsType    *relGainResults = &pDevObj->relGainResults;
    uint8                   ecVal           = pLineObj->ecVal;
    uint32                  gxInt, grInt;
    uint8                   gainCSD[VP890_GX_GAIN_LEN];
    uint8                   vpGainData;

    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("+SetRelGainInt()"));

    /* Get out if device state is not ready */
    if (!Vp890IsDevReady(pDevObj->status.state, TRUE)) {
        return VP_STATUS_DEV_NOT_INITIALIZED;
    }

    relGainResults->gResult = VP_GAIN_SUCCESS;

    /* Multiply the profile gain values by the user's adjustments. */
    gxInt = (uint32)pLineObj->gxBase * pLineObj->gxUserLevel / 16384;
    grInt = (uint32)pLineObj->grBase * pLineObj->grUserLevel / 16384;

    /* If overflow or underflow occurred, generate out-of-range result. */
    /* Requirement: 1.0 <= gxInt <= 4.0 */
    if ((gxInt < (uint32)0x500) || (gxInt > (uint32)0x10000)) {
        relGainResults->gResult |= VP_GAIN_GX_OOR;
        gxInt = pLineObj->gxBase;
    }
    /* Requirement: 0.25 <= grInt <= 1.0 */
    if ((grInt < (uint32)0x1000) || (grInt > (uint32)0x4000)) {
        relGainResults->gResult |= VP_GAIN_GR_OOR;
        grInt = pLineObj->grBase;
    }

    /* If this function is being called for an on-hook FXO line, we need to
     * apply the caller ID correction factor - a previously computed value
     * that is set to 1 by default - and set the Digital Transmit Gain bit */
    if (pLineObj->status & VP890_IS_FXO) {
        VpMpiCmdWrapper(deviceId, ecVal, VP890_VP_GAIN_RD, VP890_VP_GAIN_LEN, &vpGainData);
        vpGainData &= ~VP890_DTG_MASK;

        if (pLineObj->lineState.currentState == VP_LINE_FXO_LOOP_OPEN
          || pLineObj->lineState.currentState == VP_LINE_FXO_OHT) {
            gxInt = (uint32)gxInt * pLineObj->gxCidLevel / 16384;
            /* Requirement: 1.0 <= gxInt < 5.0
             * If outside of this range, set it to the limit */
            if (gxInt >= (uint32)0x14000) {
                gxInt = (uint32)0x13FFF;
            } else if (gxInt < (uint32)0x500) {
                gxInt = 0x500;
            }
            /* Apply the DTG value calculated for callerId correction */
            vpGainData |= pLineObj->cidDtg;
        } else {
            vpGainData |= pLineObj->userDtg;
        }
        VpMpiCmdWrapper(deviceId, ecVal, VP890_VP_GAIN_WRT, VP890_VP_GAIN_LEN, &vpGainData);
    }
    /*
     * Write adjusted gain values to the device, and remember them for
     * VpGetResults().
     */
    ConvertSignedFixed2Csd((int32)gxInt - 0x4000, gainCSD);
    relGainResults->gxValue = ((uint16)gainCSD[0] << 8) + gainCSD[1];
    VpMpiCmdWrapper(deviceId, ecVal, VP890_GX_GAIN_WRT, VP890_GX_GAIN_LEN,
        gainCSD);

    ConvertSignedFixed2Csd((int32)grInt, gainCSD);
    relGainResults->grValue = ((uint16)gainCSD[0] << 8) + gainCSD[1];
    VpMpiCmdWrapper(deviceId, ecVal, VP890_GR_GAIN_WRT, VP890_GR_GAIN_LEN,
        gainCSD);


    return VP_STATUS_SUCCESS;
} /* Vp890SetRelGainInt() */

/*******************************************************************************
 * ConvertSignedFixed2Csd()
 *  This function returns a four-nibble CSD (canonical signed digit) number
 * whose value matches (as nearly as possible) the supplied signed 2.14
 * fixed-point number.
 *
 * Preconditions:
 *
 * Postconditions:
 *  The CSD number will be placed into a two-byte array (high byte first) at
 * the address specified in the csdBuf parameter.
 ******************************************************************************/
static void
ConvertSignedFixed2Csd(
    int32 fixed,
    uint8 *csdBuf)
{
#define CSD_NIBBLES 4
    uint16 error, power, greaterPower, smallerPower, distGreater, distSmaller;
    uint16 C, m, result;
    int32 sum = 0;
    int8 n, gp, sp;

    /* Data structure for holding the four terms composing the CSD number. */
    typedef struct {
        bool sign;
        int power;
    } term;
    term t[CSD_NIBBLES + 1];

    t[0].power = 0;
    t[0].sign = 0;

    /*
     * Split the 2.14 value into a sum of powers of 2,
     *   s1 * 2^p1  +  s2 * 2^p2  +  s3 * 2^p3  +  s4 * 2^p4
     * where for term x,
     *   sx = 1 or -1,
     *   px <= 0.
     */
    for (n = 1; n <= CSD_NIBBLES; n++) {

        if (sum == fixed) break;

        /*
         * If current sum is less than actual value, then the next term
         * should be added; otherwise the next term should be
         * subtracted.
         */
        if (sum < fixed) {
            t[n].sign = 0;
            error = fixed - sum;
        } else {
            t[n].sign = 1;
            error = sum - fixed;
        }

        /* If error > 1, then term = +/-1. */
        if (error > 0x4000) {
            t[n].power = 0;
        } else {

            /*
             * Calculate greaterPower = the smallest power of 2 greater
             * than error.  Calculate smallerPower = the largest power
             * of 2 less than error.
             */
            greaterPower = 0x4000; gp = 0;
            for (power = 0x2000; power > error; power >>= 1) {
                greaterPower >>= 1; gp--;
            }
            smallerPower = greaterPower >> 1; sp = gp - 1;

            /*
             * Is error closer to greaterPower or smallerPower?
             * Whichever is closer, choose that for the value of the
             * next term.
             */
            distGreater = greaterPower - error;
            distSmaller = error - smallerPower;
            if (distGreater < distSmaller) {
                t[n].power = gp;
            } else {
                t[n].power = sp;
            }

            /*
             * The power of this term can differ from the power of the
             * previous term by no more than 7.
             */
            if (t[n - 1].power - t[n].power > 7) {
                t[n].power = t[n - 1].power - 7;
            }
        }

        /* Add or subtract the term to the sum, depending on sign. */
        if (t[n].sign == 0) {
            sum += (uint16)1 << (14 + t[n].power);
        } else {
            sum -= (uint16)1 << (14 + t[n].power);
        }
    }

    /*
     * If we reached the exact value with terms left over, fill these
     * extra terms with dummy values which don't affect the CSD value.
     */
    while (n <= CSD_NIBBLES) {
        if (n == 1) {
            t[1] = t[0];
            t[2].power = 0;
            t[2].sign = 1;
            n += 2;
        } else {
            /*
             * Increase the number of terms by replacing the last term
             * with two new terms whose sum is the old term.
             */
            if (t[n - 1].power == t[n - 2].power) {
                t[n - 1].power--;
                t[n] = t[n - 1];
            } else {
                t[n] = t[n - 1];
                t[n - 1].power++;
                t[n].sign = !(t[n - 1].sign);
            }
            n++;
        }
    }

    /* Compute nibble values from the terms. */
    result = 0;
    for (n = 1; n <= CSD_NIBBLES; n++) {
        int8 bitPos = (n - 1) * 4;
        C = (t[n].sign != t[n - 1].sign);
        m = -(t[n].power - t[n - 1].power);
        result |= (C << (bitPos + 3)) | (m << bitPos);
    }

    /* Split the uint16 result into high and low bytes. */
    csdBuf[0] = (uint8)(result >> 8);
    csdBuf[1] = (uint8)(result & 0xFF);
} /* ConvertFixed2Csd() */


/*******************************************************************************
 * Vp890ProtectedWriteICR1()
 *  This function is a wrapper for writing to ICR1.  If the internal test
 * termination is applied, ICR1 must not be changed, so this function copies
 * the data into the line object cache and returns without writing anything to
 * the device.  If the internal test termination is not applied, the write
 * is performed.
 *
 * Note: This function must be called from within a critical section.
 ******************************************************************************/
void
Vp890ProtectedWriteICR1(
    Vp890LineObjectType *pLineObj,
    VpDeviceIdType deviceId,
    uint8 *icr1Values)
{
    pLineObj->icr1Values[0] = icr1Values[0];
    pLineObj->icr1Values[1] = icr1Values[1];
    pLineObj->icr1Values[2] = icr1Values[2];
    pLineObj->icr1Values[3] = icr1Values[3];
    if (pLineObj->internalTestTermApplied == TRUE) {
        return;
    } else {
        VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_ICR1_WRT, VP890_ICR1_LEN,
            icr1Values);
    }
}



#endif /* VP_CC_890_SERIES */
