/** \file vp880_api.h
 * vp880_api.h
 *
 *  Header file that define all the commands for the Vp880 device.
 *
 * Copyright (c) 2010, Zarlink Semiconductor, Inc.
 *
 * $Revision: 7511 $
 * $LastChangedDate: 2010-07-16 13:32:46 -0500 (Fri, 16 Jul 2010) $
 */

#ifndef VP880_API_H
#define VP880_API_H

#include "vp_hal.h"
#include "vp_CSLAC_types.h"
#include "vp_api_common.h"

#define VP880_MAX_NUM_CHANNELS   2

#ifdef VP880_INCLUDE_TESTLINE_CODE
#include "vp_api_test.h"
  #ifdef VP880_EZ_MPI_PCM_COLLECT
  #include  "vp_pcm_compute.h"
  #endif
#endif

#define VP880_MAX_MPI_DATA  15   /* Max data from any MPI read command */
#define VP880_INT_SEQ_LEN       22

#ifndef VP880_DC_FEED_LEN
#define VP880_DC_FEED_LEN    0x02
#endif

#ifndef VP880_REGULATOR_PARAM_LEN
#define VP880_REGULATOR_PARAM_LEN   0x03
#endif

#ifndef VP880_LOOP_SUP_LEN
#define VP880_LOOP_SUP_LEN      0x04
#endif

#ifndef VP880_SIGA_PARAMS_LEN
#define VP880_SIGA_PARAMS_LEN       0x0B
#endif

#ifndef VP880_UL_SIGREG_LEN
#define VP880_UL_SIGREG_LEN         0x02
#endif

#ifndef VP880_ICR1_LEN
#define VP880_ICR1_LEN      0x04
#endif

#ifndef VP880_ICR2_LEN
#define VP880_ICR2_LEN      0x04
#endif

#ifndef VP880_ICR3_LEN
#define VP880_ICR3_LEN      0x04
#endif

#ifndef VP880_ICR4_LEN
#define VP880_ICR4_LEN      0x04
#endif

#ifndef VP880_ICR5_LEN
#define VP880_ICR5_LEN      0x02
#endif

#ifndef VP880_ICR6_LEN
#define VP880_ICR6_LEN      0x02
#endif

#ifndef VP880_DC_CAL_REG_LEN
#define VP880_DC_CAL_REG_LEN      0x02
#endif

#ifndef VP880_BAT_CALIBRATION_LEN
#define VP880_BAT_CALIBRATION_LEN 0x02
#endif

#ifndef VP880_REGULATOR_CTRL_LEN
#define VP880_REGULATOR_CTRL_LEN    0x01
#endif

#ifndef VP880_TX_PCM_BUFF_LEN
#define VP880_TX_PCM_BUFF_LEN 0x0E
#endif

#ifndef VP880_INT_SWREG_PARAM_LEN
#define VP880_INT_SWREG_PARAM_LEN   0x06
#endif

#ifndef VP880_DISN_LEN
#define VP880_DISN_LEN              0x01
#endif

#ifndef VP880_SYS_STATE_LEN
#define VP880_SYS_STATE_LEN     0x01
#endif

#ifndef VP880_OP_FUNC_LEN
#define VP880_OP_FUNC_LEN         0x01
#endif

#ifndef VP880_OP_COND_LEN
#define VP880_OP_COND_LEN         0x01
#endif

#ifndef VP880_CONV_CFG_LEN
#define VP880_CONV_CFG_LEN      0x01
#endif

#if defined (VP880_INCLUDE_TESTLINE_CODE)
/*
 * #defines used in unit conversion function
 * All units are based on the following calculations
 *      Internal voltage unit   = 480.0  / 2^31 Volts
 *      Internal current unit   = 240e-3 / 2^31 Amps
 *      Internal freq uint      = 12e3   / 2^31 Hz
 */
#define VP880_UNIT_MV                          4474L    /* mV unit */
#define VP880_UNIT_MVRMS                       6327L    /* mVrms unit */
#define VP880_UNIT_ADC_VAB                     -32768L  /* vab step from ADC */
#define VP880_UNIT_ADC_VAB_RMS                 46341L   /* vab step from ADC rms*/
#define VP880_UNIT_ADC_AC_VAB_RMS                664L     /* vab step from ADC Metallic Vrms*/
#define VP880_UNIT_DAC_RING                    21076L   /* step due to DAC in ringing*/
#define VP880_UNIT_TRACK_SWZ                23091612L  /* step used for tracking switching regulator Z */
#define VP880_UNIT_UA                          8948L    /* uA unit */
#define VP880_UNIT_ADC_IMET_NOR                16384L   /* met I from ADC in normal VC */
#define VP880_UNIT_ADC_IMET_NOR_JA             15756L   /* met I from ADC in normal JA*/
#define VP880_UNIT_ADC_IMET_RING               32768L   /* met I from ADC in ringing*/
#define VP880_UNIT_ADC_ILG                     11469L   /* long current from ADC*/
#define VP880_UNIT_DECHZ                       17896L   /* Deci Hz*/
#define VP880_UNIT_MHZ                         179L     /* mHz */
#define VP880_UNIT_FREQ                        65536L   /* siggen freq step */
#define VP880_UNIT_RAMP_TIME                   1365L    /* Ramp time (-1/3) in ms*/

#define VP880_UNIT_CONVERT(DATA, I_UNIT, O_UNIT)    (int32)(((int32)DATA * I_UNIT) / O_UNIT)

/* converts requested slope into SigGen Freq */
#define VP880_SLOPE_TO_FREQ(AMP,SLOPE)  (int16)(((int32)SLOPE * (int32)VP880_UNIT_RAMP_TIME) / (int32)AMP)

#define VP880_AC_RLOOP_MAX_BAT_LVL              75000L /* mV */
#define VP880_AC_RLOOP_MAX_TEST_LVL         ((VP880_AC_RLOOP_MAX_BAT_LVL - 5000) / 2)


#endif /*VP880_INCLUDE_TESTLINE_CODE*/

/**< Required Vp880 Device and Line Objects for user instantiation if a Vp880
 * device is used
 */

/**< Structure that defines the Vp880 Device Profile. Current as of the first
 * Device Profile version (ver 0).
 */
typedef struct {
    uint16  pcmClkRate;      /**< Used to verify valid TX/RX Timeslot setting */
    uint8   mClkMask;
    uint16  tickRate;        /**< Primary API-II tick for this device */
    uint8   devCfg1;
    uint8   clockSlot;
    bool    peakManagement;
} Vp880DeviceProfileType;

/**< Line Status types to minimize code space in line object (compared to each
 * status being maintined by a uint8 type)
 */
typedef enum {
    VP880_INIT_STATUS = 0x0000,

    VP880_IS_FXO = 0x0001,  /**< Set if the line is configured for FXO */

    VP880_SLS_CALL_FROM_API = 0x0002,   /**< Set if Set Line State is called
                                         * from an API function (e.g., cadence).
                                         */

    VP880_BAD_LOOP_SUP = 0x0004,    /**< Set when the Loop Supervision has been
                                     * changed in such a way inconsistent with
                                     * the user's specifications. This is done
                                     * in internal to the API to make some
                                     * functions work (e.g., Msg Wait Pulse).
                                     */

    VP880_UNBAL_RINGING = 0x0008,   /**< Set if this line uses unbal ringing */

    VP880_DP_SET1_DONE = 0x0010,    /**< Set when Dial Pulse detection machine
                                     * is "done" on the current dial pulse using
                                     * the "first" set of DP parameters
                                     */

    VP880_DP_SET2_DONE = 0x0020,    /**< Set when Dial Pulse detection machine
                                     * is "done" on the current dial pulse using
                                     * the 2nd set of DP parameters
                                     */

    VP880_LINE_IN_CAL = 0x0040,    /**< Set when line is calibrating */

    VP880_LOW_POWER_EN = 0x0080,    /**< Set when line is operating in low power
                                     * mode
                                     */

    VP880_LINE_LEAK = 0x0100,      /**< Set when leakage is detected on the line
                                    * such that low power mode is prevented.
                                    */
    VP880_HAL_DELAY = 0x0400,      /**< Set when MPI write is delayed. Should
                                    * only occur for LPM.
                                    */
    VP880_INIT_COMPLETE = 0x0800,  /**< Set when InitLine has been completed
                                    * on this line.
                                    */
    VP880_PREVIOUS_HOOK = 0x1000   /**< Set if Last Hook Event reported was
                                    * off-hook, cleared if last event was
                                    * on-hook.
                                    */

} Vp880LineStatusType;

#if defined (VP880_INCLUDE_TESTLINE_CODE)
/* Definitions for Test arguments */
typedef union {
    VpTestConcludeType          conclude;
    VpTestOpenVType             openV;
    VpTestDcRLoopType           dcRloop;
    VpTestAcRLoopType           acRloop;
    VpTest3EleResAltResType     resFltAlt;
    VpTest3EleCapAltResType     capAlt;
    VpTestLoopCondType          loopCond;
    VpTestLoopbackType          loopback;
    VpTestRampType              ramp;
    VpTestRampInitType          rampInit;
} Vp880TestArgsType;

/* Used to configure the device for a target current */
typedef struct {
    bool success;
    int16 polarity;
    int16 state;
    int16 nullI;
    int16 metI;
    uint8 prevSigCtrl;
    uint8 prevSlacState;
    uint8 prevVpGain;
    uint8 prevDcFeed[VP880_DC_FEED_LEN];
} Vp880TargetCurrentType;

typedef struct {
    uint8 slacState;
    uint8 vpGain;
    uint8 opCond;
    uint8 opFunction;
    uint8 icr2[VP880_ICR2_LEN];
    uint8 icr3[VP880_ICR3_LEN];
    uint8 icr4[VP880_ICR4_LEN];
    uint8 icr6[VP880_ICR6_LEN];
} Vp880LineTestCalType;

typedef struct {
    uint8 adcState;
    int16 nextState; /**< Used when a pcm collection routine is started */

    Vp880TestArgsType testArgs; /**< Saved test input arguments of current test */

    uint8 opCond;                           /**< Saved Operation Condition */
    uint8 opFunction;                       /**< Saved Operation Functions */
    uint8 sigCtrl;                          /**< Signal Generator Control */
    uint8 slacState;                        /**< Saved Slac State */
    uint8 sysConfig;                        /** Saved System Configurations */
    uint8 vpGain;                           /**< Voice Path Gain */
    uint8 switchReg[VP880_REGULATOR_PARAM_LEN]; /**< Switching Reg Parameters */
    uint8 dcFeed[VP880_DC_FEED_LEN];        /**< Saved DC Feed Parameters */
    uint8 disn;                             /**< Digital Imped. Scaling Network */
    uint8 SwRegCtrl;                        /**< Switching Regulator Control */

    uint8 icr1[VP880_ICR1_LEN];
    uint8 icr2[VP880_ICR2_LEN];
    uint8 icr3[VP880_ICR3_LEN];
    uint8 icr4[VP880_ICR4_LEN];
    uint8 icr5[VP880_ICR5_LEN];
    uint8 icr6[VP880_ICR6_LEN];
    uint8 lpSuper[VP880_LOOP_SUP_LEN];      /**< Saved Loop Sup. Parameters */
    uint8 sigGenAB[VP880_SIGA_PARAMS_LEN]; /**< Saved Signal Generator A & B*/

    /* lg res flt uses this to bump up the battery */
    uint8 batCal[VP880_BAT_CALIBRATION_LEN];

    /* used for collecting PCM data */
    bool pcmRequest;        /** < indication that pcm data was requested */
    VpPcmOperationMaskType operationMask;

    VpPcmOperationResultsType pcmResults; /** < stores the pcm operation results */

    /* Used for setting up a current measurment */
    Vp880TargetCurrentType targetI;

    /* Used for common setup functions */
    uint16 commonSetupState;

    /* Used for storing line event mask data */
    VpOptionEventMaskType preTestEventMask;

    /* Used for saving and restoring registers during calibration */
    Vp880LineTestCalType calRegs;

    /* Used for resflt lg speed up*/
    uint16 speedupTime;
    int16 previousAvg;
    int16 vabComputed;
    uint8 loopCnt;
    bool compensate;
    bool lowGain;

    /* Used in the capacitance test */
    int16 adcSampleBuffer[52];
    uint8 requestedSamples;
    uint8 saveConvConfig[VP880_CONV_CFG_LEN];
    bool xtraBuffer;

    /* The following members are for EZ mode calculations only*/
#ifdef VP880_EZ_MPI_PCM_COLLECT
    VpPcmComputeTempType ezPcmTemp;
#endif

} Vp880TestHeapType;

typedef struct {
    Vp880TestHeapType *pTestHeap;
    uint8 testHeapId;

    uint8 channelId;    /**< Channel # for "this" line on the device.  Indexed
                         * starting at 0, should not exceed the max number of
                         * lines supported by the device - 1 (max = 2, then
                         * channelId = {0, 1}
                         */

    bool prepared;       /**< indicates if the current test is prepared */
    bool preparing;      /**< indicates if the test prepare is complete */
    bool concluding;     /**< indicates that the device is concluding a test */
    bool rdLoopTest;     /***< LT_TID_RD_LOOP_COND specific */
    VpTestIdType testId; /** < indicates the test currently running */

    int16 testState;     /**< maintains the currnt state of the current TestId */
    uint16 handle;

} Vp880CurrentTestType;

typedef struct {
    int16 nullOffset;
    int16 vabOffset;
    int16 vahOffset;
    int16 valOffset;
    int16 vbhOffset;
    int16 vblOffset;
    int16 imtOffset;
    int16 ilgOffset;
} Vp880CalOffCoeffs;

#endif /*VP880_INCLUDE_TESTLINE_CODE*/

typedef enum {
    VP880_CAL_INIT = 0,
    VP880_CAL_ERROR,
    VP880_CAL_OFFSET,
    VP880_CAL_MEASURE,
    VP880_CAL_FIRST_IMT_SET,
    VP880_CAL_FIRST_IMT_READ,
    VP880_CAL_FIRST_ILG_READ,
    VP880_CAL_FIRST_VOC_READ,
    VP880_CAL_SIGEN_A_PHASE1,
    VP880_CAL_SIGEN_A_PHASE2,
    VP880_CAL_SIGEN_A_PHASE3,
    VP880_CAL_FIRST_VAG_READ,
    VP880_CAL_FIRST_VBG_READ,
    VP880_CAL_IMT_OFFSET_SET,
    VP880_CAL_IMT_OFFSET_READ,
    VP880_CAL_ILG_OFFSET_READ,
    VP880_CAL_IMT_OFFSET_SET_INVERT,
    VP880_CAL_IMT_OFFSET_READ_INVERT,
    VP880_CAL_ILG_OFFSET_READ_INVERT,
    VP880_CAL_VOC_READ_INVERT,
    VP880_CAL_VAG_READ_INVERT,
    VP880_CAL_VBG_READ_INVERT,
    VP880_CAL_IMT_OFFSET_SET_DONE,
    VP880_CAL_IMT_OFFSET_SET_DONE_PRE,
    VP880_CAL_IMT_OFFSET_READ_DONE,
    VP880_CAL_ILG_OFFSET_READ_DONE,
    VP880_CAL_VOC_READ_DONE,
    VP880_CAL_ADC,
    VP880_CAL_STATE_CHANGE,
    VP880_CAL_INVERT_POL,
    VP880_CONVERTER_CHECK,
    VP880_CAL_DONE,
    VP880_CAL_EXIT
} Vp880CalState;

typedef struct {
    int16 imtPrev;
    bool secondPass;
} Vp880VasCalData;

typedef struct {
    int16 ilgNorm;

    /* This is to be backward compatible with VVA P1.3.0 */
    int16 ilgOffsetNorm;
} Vp880IlgCalData;

typedef struct {
    int16 vocNorm;
    int16 vocRev;

    /* This is to be backward compatible with VVA P1.3.0 */
    int16 vocOffsetNorm;
    int16 vocOffsetRev;
} Vp880VocCalData;

typedef struct {
    int16 vagNorm;
    int16 vagRev;

    /* This is to be compatible with VVA P1.3.0 */
    int16 vagOffsetNorm;
    int16 vagOffsetRev;
} Vp880VagCalData;

/* This is to be compatible with VVA P1.3.0 */
typedef struct {
    int16 vbgOffsetNorm;
    int16 vbgOffsetRev;
} Vp880VbgCalData;

/* This is to be compatible with VVA P1.3.0 */
typedef struct {
    int16 ilaOffsetNorm;
} Vp880IlaCalData;

typedef struct {
    uint8 passCnt;
    bool initChange;    /**< As needed, set to TRUE when a state is changed in
                         * the calibration state machine.
                         */

    int16 swyVolt[2]; /**< One per channel used to measure SWY Voltage */
    int16 swzVolt[2]; /**< One per channel used to measure SWZ Voltage */

    uint8 isrpData[VP880_INT_SWREG_PARAM_LEN];
    uint8 icr2[2][VP880_ICR2_LEN];
    uint8 icr3[2][VP880_ICR3_LEN];
    uint8 icr4[2][VP880_ICR4_LEN];
    uint8 disnVal[2][VP880_DISN_LEN];
    uint8 sysState[2][VP880_SYS_STATE_LEN];
    uint8 opFunc[2][VP880_OP_FUNC_LEN];
    uint8 opCond[2][VP880_OP_COND_LEN];
    uint8 converterCfg[2][VP880_CONV_CFG_LEN];
    uint8 switcherAdjust[2][VP880_BAT_CALIBRATION_LEN];

    /* This is to be compatible with VVA P1.3.0 */
    int16 swyOffset[2];
    int16 swzOffset[2];

    /* This is to be compatible with VVA P1.3.0 but not currently measured. */
    int16 swxbOffset[2];
} Vp880AbvCalData;

typedef struct {
    Vp880VasCalData vasData;
    Vp880IlgCalData ilgData;
    Vp880VocCalData vocData;
    Vp880VagCalData vagData;

    /* This is be compatible with VVA P1.3.0 */
    Vp880VbgCalData vbgData;
    Vp880IlaCalData ilaData;
} Vp880CalTypeData;

typedef struct {
    bool calDone;           /**< TRUE when calibration has been performed on
                             * this line
                             */
    bool reversePol;        /**< TRUE when line is in reverse polarity */

    Vp880CalTypeData typeData;

    uint8 codecReg;
    uint8 icr2Values[VP880_ICR2_LEN];

    uint8 dcFeedRef[VP880_DC_FEED_LEN]; /**< Customer settings per profile */
    uint8 dcFeed[VP880_DC_FEED_LEN];    /**< Normal polarity calibrated values */
    uint8 dcFeedPr[VP880_DC_FEED_LEN];  /**< Reverse polarity calibrated values */

    Vp880CalState calState;
    VpLineStateType usrState;

    uint8 disnVal[VP880_DISN_LEN];
    bool forceCalDataWrite;

    /* Signal generator calibration temporary backup */
    uint8 sigGenA[VP880_SIGA_PARAMS_LEN];
    uint8 sysState[VP880_SYS_STATE_LEN];
    uint8 calReg[VP880_DC_CAL_REG_LEN];
} Vp880CalLineData;

typedef struct {
    Vp880AbvCalData abvData;
    Vp880CalState calState;
    int16 calSet;   /* Target values for calibration */
} Vp880CalDeviceData;

typedef struct {
    bool channelArray[VP880_MAX_NUM_CHANNELS];
    uint8 stage[VP880_MAX_NUM_CHANNELS];
    uint8 swRegParam[VP880_REGULATOR_PARAM_LEN];
} Vp880RingParams;

typedef struct {
    uint8 channelId;    /**< Channel # for "this" line on the device.  Indexed
                         * starting at 0, should not exceed the max number of
                         * lines supported by the device - 1 (max = 2, then
                         * channelId = {0, 1}
                         */

    uint8 ecVal;

    VpTermType termType;    /**< Termination type */

    Vp880LineStatusType status; /**< Keeps track of several line state/config */

#ifdef CSLAC_SEQ_EN
    VpCallerIdType callerId;    /**< Caller ID related information */
    VpSeqDataType cadence;      /**< Sequencer related information */
    VpCidSeqDataType cidSeq;    /**< CID Sequencer related information */

    bool suspendCid;

    /**< Array to control internally run sequences */
    VpProfileDataType intSequence[VP880_INT_SEQ_LEN];
#endif

    VpDialPulseDetectType dpStruct; /**< Used on FXS lines for detecting pulse
                                     * digits
                                     */

    VpDialPulseDetectType dpStruct2;/**< Used on FXS lines for detecting pulse
                                     * digits using 2nd set of dp specs.
                                     */

    VpDigitGenerationDataType digitGenStruct;   /**< Used on FXO lines for
                                                 * generating pulse digits
                                                 */

    VpOptionEventMaskType lineEventsMask;
    VpOptionEventMaskType lineEvents;

    uint16 signaling1;
    uint16 signaling2;
    uint8 signalingData;    /**< Holds data for Signaling events on this line */

    VpOptionPulseModeType pulseMode;

    uint8 fxoData;          /**< Holds data for FXO events on this line */
    uint8 preRingPolRev;    /**< The polarity detected prior to ring detect */
    uint8 preDisconnect;    /**< The disconnect state prior to timer start */

    uint8 processData;      /**< Holds data for Process events on this line */

    uint8 responseData;     /**< Holds data for Response events on this line */

    VpCslacTimerStruct lineTimers; /**< Timers for "this" line */
    VpApiIntLineStateType lineState;    /**< Line state info used for state
                                         * transition handling and recall
                                         */
    uint8 nextSlicValue;

    /*
     * Holds user definition for Loop Supervision configuration when
     * "badLoopSup" is TRUE
     */
    uint8 loopSup[VP880_LOOP_SUP_LEN];

    /*
     * Array to hold ringing parameters used in the Signal Generator.  This is
     * needed when signal generator A is set to a tone, then set back to ringing
     * without the user re-specifying the ringing profile
     */
    uint8 ringingParams[VP880_SIGA_PARAMS_LEN];

    VpProfilePtrType pRingingCadence;   /**< Currently used ringing cadence on
                                         * this line
                                         */

    VpProfilePtrType pMeterProfile;     /**< Currently used metering profile on
                                         * this line
                                         */

    VpProfilePtrType pCidProfileType1;  /**< Currently used caller ID profile
                                         * on this line for sequenced cid
                                         */

    uint16 lineEventHandle; /**< Line specific event handle information */

    VpOptionRingControlType ringCtrl;

    VpOptionPcmTxRxCntrlType pcmTxRxCtrl;   /* Defines how the PCM highway is
                                             * set for "talk" linestates
                                             */

    int8 onHookTicks;   /**< Used to count duration since last on-hook used for
                         * ABS/Clare errata
                         */

    uint16 dtmfDigitSense;          /**< Used to hold the DTMF digit reported
                                     * with VpDtmfDigitDetected() until the
                                     * VP_LINE_EVID_DTMF_DIG is generated.
                                     */

    VpRelayControlType relayState;   /**< Used to hold current line relay state */

    VpLineIdType lineId;    /**< Application provided value for mapping a line to
                             * a line context
                             */
    struct {
        uint16 gxInt;       /**< Cached GX register, in 2.14 int format */
        uint16 grInt;       /**< Cached GR register, in 2.14 int format */
    } gain;

    uint8 ringDetMax;   /**< Stores the user specified maximum ringing detect
                         * period for FXO lines. This value may be outside the
                         * device range, in which case the SW will implement
                         * upper period detection
                         */
    uint8 ringDetMin;   /**< Stores the user specified maximum ringing detect
                         * period for FXO lines that is within the device range.
                         * This value is used as "minimum" that is detected by
                         * SW. Actual minimum period is supported by the device
                         * period detector itself.
                         */

    Vp880CalLineData calLineData;

    VpOptionCodecType codec;

    uint8 icr1Values[VP880_ICR1_LEN];   /**< Cached to minimize device access */
    uint8 icr2Values[VP880_ICR2_LEN];   /**< Cached to minimize device access */
    uint8 icr3Values[VP880_ICR3_LEN];   /**< Cached to minimize device access */
    uint8 icr4Values[VP880_ICR4_LEN];   /**< Cached to minimize device access */
    uint8 icr6Values[VP880_ICR6_LEN];   /**< Cached to minimize device access */

    uint8 leakyLineCnt; /*
                         * Keeps track of # of times off-hook was detected (LP Mode)
                         * that was not further verified after exiting LP Mode. Reset
                         * when off-hook is verified.
                         */
    /* Loop supervision parameters */
    uint8 hookHysteresis;

    bool internalTestTermApplied;

#ifdef VP_DEBUG
    /* For runtime enabling of debug output: */
    uint32 debugSelectMask;
#endif

} Vp880LineObjectType;

typedef enum {
    VP880_IS_ABS = 0x0001,                  /**< Set when the device is ABS type */
    VP880_RLY_PROT_REQ = 0x0002,            /**< Set when the device requires relay protction on I/O 1 */
    VP880_HAS_TEST_LOAD_SWITCH = 0x0004,    /**< Set when test load swith is available */
    VP880_HAS_CALIBRATE_CIRCUIT = 0x0008,   /**< Set when test cal circuit is available */

    VP880_IS_HIGH_VOLTAGE = 0x0010,         /**< Set when device is a high voltage device */
    VP880_IS_SINGLE_CHANNEL = 0x0020,       /**< Set when a single channel device is found*/
    VP880_DEVICE_DETECTED = 0x0040,         /**< Set when the device is detected */
    VP880_LINE0_IS_FXO = 0x0080,            /**< Set if device detection indicates line0 as FXO */

    VP880_LINE1_IS_FXO = 0x0100,            /**< Set if device detection indicates line1 as FXO */
    VP880_WIDEBAND = 0x0200,                /**< Set if device supports Wideband mode */
    VP880_LINE0_LP = 0x0400,                /**< Set if line 0 allows low power mode */
    VP880_LINE1_LP = 0x0800,                /**< Set if line 1 allows low power mode */

    VP880_IS_FXO_ONLY = 0x1000,             /**< Set when the device contains only FXO lines */
    VP880_SYS_CAL_COMPLETE = 0x2000,        /**< Set when the system calibration structure has been initialied */
    VP880_CAL_RELOAD_REQ = 0x4000,          /**< Set when the line calibration values need to be reloaded. */
    VP880_FORCE_FREE_RUN = 0x8000,          /**< Set when app calls VpFreeRun() (start), cleared when called with stop.
                                             * This prevents the VP-API-II from automatically exiting free run mode
                                             * upon PCLK recovery.
                                             */
    VP880_SWY_DECAY_CMP = 0x10000,
    VP880_SWZ_DECAY_CMP = 0x20000
} Vp880DeviceStateIntType;

#define VP880_SYS_CAL_POLARITY_LENGTH  2
#define VP880_SYS_CAL_CHANNEL_LENGTH   2

/* Contains calibration error values -- in +/-10mv LSB */
typedef struct {
    int16 abvError[2];

    int16 vocOffset[VP880_SYS_CAL_CHANNEL_LENGTH][VP880_SYS_CAL_POLARITY_LENGTH];
    int16 vocError[VP880_SYS_CAL_CHANNEL_LENGTH][VP880_SYS_CAL_POLARITY_LENGTH];

    int16 sigGenAError[VP880_SYS_CAL_CHANNEL_LENGTH][VP880_SYS_CAL_POLARITY_LENGTH];

    int16 ila20[VP880_SYS_CAL_CHANNEL_LENGTH];
    int16 ila25[VP880_SYS_CAL_CHANNEL_LENGTH];
    int16 ila32[VP880_SYS_CAL_CHANNEL_LENGTH];
    int16 ila40[VP880_SYS_CAL_CHANNEL_LENGTH];

    int16 ilaOffsetNorm[VP880_SYS_CAL_CHANNEL_LENGTH];
    int16 ilgOffsetNorm[VP880_SYS_CAL_CHANNEL_LENGTH];

    /* Used for Tracker only */
    uint8 vas[VP880_SYS_CAL_CHANNEL_LENGTH][VP880_SYS_CAL_POLARITY_LENGTH];

    int16 vagOffsetNorm[VP880_SYS_CAL_CHANNEL_LENGTH];
    int16 vagOffsetRev[VP880_SYS_CAL_CHANNEL_LENGTH];
    int16 vbgOffsetNorm[VP880_SYS_CAL_CHANNEL_LENGTH];
    int16 vbgOffsetRev[VP880_SYS_CAL_CHANNEL_LENGTH];

    /* Used for ABS only */
    uint8 absNormCal[VP880_SYS_CAL_CHANNEL_LENGTH];
    uint8 absPolRevCal[VP880_SYS_CAL_CHANNEL_LENGTH];

    int16 swyOffset[VP880_SYS_CAL_CHANNEL_LENGTH];     /**< Used to hold SWY Offset */
    int16 swzOffset[VP880_SYS_CAL_CHANNEL_LENGTH];     /**< Used to hold SWZ Offset */
    int16 swxbOffset[VP880_SYS_CAL_CHANNEL_LENGTH];    /**< Used to hold SWXB Offset */

    /* Used for capacitance line test only */
    int32 tipCapCal[VP880_SYS_CAL_CHANNEL_LENGTH];
    int32 ringCapCal[VP880_SYS_CAL_CHANNEL_LENGTH];
} Vp880SysCalResultsType;

#define VP880_CAL_STRUCT_SIZE   (40*sizeof(int16) + 8*sizeof(uint8) + 4*sizeof(int32))

typedef struct {
    VpDeviceIdType deviceId;    /**< Device chip select ID defined by user */
    VpDeviceStaticInfoType staticInfo;  /**< Info that will not change during
                                         * runtime
                                         */
    VpDeviceDynamicInfoType dynamicInfo;    /**< Info that will change during
                                             * runtime
                                             */
    /*
     * Variety of common device level status such as init/not init, init in
     * progress, test buffer read y/n?, etc.. Internally used by the VP-API-II
     * to communicate device state.
     */
    VpCSLACDeviceStatusType status;

    /*
     * Similar to common device level status, this is for 880 specific type of
     * device level information. Such as ABS/Tracker, LPM lines, etc..
     *
     * This works on the Vp880DeviceStateIntType values
     */
    uint32 stateInt;

    VpOptionEventMaskType deviceEventsMask;
    VpOptionEventMaskType deviceEvents;

    /*
     * Two sets of dial pulse specifications are provide to support NTT dial
     * pulse detection windows of 10pps and 20pps while excluding 15pps
     */
    VpOptionPulseType pulseSpecs;
    VpOptionPulseType pulseSpecs2;

    VpOptionCriticalFltType criticalFault;

    uint16 devTimer[VP_DEV_TIMER_LAST];
    uint8 timerChan[2];     /**< Channel ID associated with device timer */
    uint16 timerHandle[2];  /**< Event Handle associated with device timer */

    Vp880DeviceProfileType devProfileData;
    VpCSLACDeviceProfileTableType devProfileTable;
    VpCSLACProfileTableEntryType profEntry;

    /**< State of signaling interrupt register - length 2 bytes */
    uint8 intReg[VP880_UL_SIGREG_LEN];  /**< Holds signaling data info for the
                                         * device
                                         */

    uint8 mpiData[VP880_MAX_MPI_DATA];  /**< Buffer for MPI Low level reads to
                                         * hold maximum amount of MPI data that
                                         * is possible
                                         */

    uint8 mpiLen;       /**< Length of data to be copied into mpiData buffer */

    uint16 eventHandle;  /** Application defined event handle */
    uint16 timeStamp;   /**< Used to track event timing. Increment by ticks */

    uint8 responseData; /**< Holds data for Response events on the device */

    VpGetResultsOptionsType getResultsOption;
    VpRelGainResultsType relGainResults;

    /* Testing structure */
    VpTestResultType testResults;
#if defined (VP880_INCLUDE_TESTLINE_CODE)
    Vp880CurrentTestType currentTest;

    /**< Used to hold calibration offset coeffs. One per channel */
    Vp880CalOffCoeffs calOffsets[VP880_MAX_NUM_CHANNELS];

#endif /* VP880_INCLUDE_TESTLINE_CODE */

    /*
     * Used to hold battery switch calibration offset. One per channel, per
     * polarity
     */
    uint8 cachedSwCtrl[VP880_REGULATOR_CTRL_LEN];
    uint8 calState;
    Vp880CalDeviceData calData;

    /*
     * Used to get better hook granularity and pcm buffered data.
     */
    uint8 txBuffer[VP880_TX_PCM_BUFF_LEN];
    uint8 txBufferDataRate;

    uint8 swParams[VP880_REGULATOR_PARAM_LEN];
    uint8 intSwParams[VP880_INT_SWREG_PARAM_LEN];
    uint8 intSwParamsFR[VP880_INT_SWREG_PARAM_LEN];

    uint8 yVolt;    /* Y-Switcher Target in 1V step */
    uint8 zVolt;    /* Z-Switcher Target in 1V step */

#if defined (VP_CC_880_SERIES) || defined (VP_CC_KWRAP)
    Vp880SysCalResultsType vp880SysCalData;
#endif

    /* used to store the in-rush current function data */
    Vp880RingParams ringParams;

#ifdef VP_DEBUG
    /* For runtime enabling of debug output: */
    uint32 debugSelectMask;
#endif

    /* Used to hold WB information */
    uint8 ecVal;

} Vp880DeviceObjectType;

#endif  /**< vp880_api.h */
