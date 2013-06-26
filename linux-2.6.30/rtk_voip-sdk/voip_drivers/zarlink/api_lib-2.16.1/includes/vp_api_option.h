/** \file vp_api_option.h
 * vp_api_option.h
 *
 * This file contains declaration associated with VP-API Options.
 *
 * Copyright (c) 2010, Zarlink Semiconductor, Inc.
 *
 * $Revision: 6493 $
 * $LastChangedDate: 2010-02-19 14:56:38 -0600 (Fri, 19 Feb 2010) $
 */

#ifndef VP_API_OPTION
#define VP_API_OPTION

#include "vp_api_types.h"
#include "vp_api_cfg.h"
#include "vp_api_event.h"

/* Option IDs.  (See Options chapter in VP-API-2 Reference Guide.) */
typedef uint16 VpOptionIdType;

/*
 * Line-specific option IDs begin with "VP_OPTION_ID_".  Device-specific
 * option IDs begin with "VP_DEVICE_OPTION_ID_".  When new option IDs are added,
 * the VpOptionValueType struct (below) must be updated accordingly.
 */
#define VP_DEVICE_OPTION_ID_PULSE               (0x00)
#define VP_DEVICE_OPTION_ID_CRITICAL_FLT        (0x01)
#define VP_OPTION_ID_ZERO_CROSS                 (0x02)
#define VP_DEVICE_OPTION_ID_RAMP2STBY           (0x03)
#define VP_OPTION_ID_PULSE_MODE                 (0x04)
#define VP_OPTION_ID_TIMESLOT                   (0x05)
#define VP_OPTION_ID_CODEC                      (0x06)
#define VP_OPTION_ID_PCM_HWY                    (0x07)
#define VP_OPTION_ID_LOOPBACK                   (0x08)
#define VP_OPTION_ID_LINE_STATE                 (0x09)
#define VP_OPTION_ID_EVENT_MASK                 (0x0A)
#define VP_OPTION_ID_RESERVED_1                 (0x0B)
#define VP_OPTION_ID_RING_CNTRL                 (0x0C)
#define VP_OPTION_ID_RESERVED_2                 (0x0D)
#define VP_OPTION_ID_DTMF_MODE                  (0x0E)
#define VP_DEVICE_OPTION_ID_DEVICE_IO           (0x0F)
#define VP_OPTION_ID_RESERVED_EVENT_MASK_VCP    (0x10)
#define VP_OPTION_ID_PCM_TXRX_CNTRL             (0x11)
#define VP_DEVICE_OPTION_ID_PULSE2              (0x12)
#define VP_OPTION_ID_LINE_IO_CFG                (0x13)
#define VP_DEVICE_OPTION_ID_DEV_IO_CFG          (0x14)
#define VP_OPTION_ID_DTMF_SPEC                  (0x15)
#define VP_DEVICE_OPTION_ID_PARK_MODE           (0x16)
#define VP_OPTION_ID_DCFEED_SLOPE               (0x17)
#define VP_OPTION_ID_SWITCHER_CTRL              (0x18)
#define VP_OPTION_ID_HOOK_DETECT_MODE           (0x19)

/* It is fine to add new option values in this gap. */
#define VP_OPTION_ID_PULSE                      (0x24)
#define VP_OPTION_ID_DEBUG_SELECT               (0x25)

#define VP_NUM_OPTION_IDS                       (0x26)

/** Parameters for dial pulse, flash, and on-hook */
typedef struct {
    uint16 breakMin;        /**< Minimum pulse break time (in 125uS) */
    uint16 breakMax;        /**< Maximum pulse break time (in 125uS) */
    uint16 makeMin;         /**< Minimum pulse make time (in 125uS) */
    uint16 makeMax;         /**< Maximum pulse make time (in 125uS) */
    uint16 interDigitMin;   /**< Minimum pulse interdigit time (in 125uS) */
    uint16 flashMin;        /**< Minimum flash break time (in 125uS) */
    uint16 flashMax;        /**< Maximum flash break time (in 125uS) */
#ifdef EXTENDED_FLASH_HOOK
    uint16 onHookMin;       /**< Minimum on-hook time (in 125uS) */
#endif
} VpOptionPulseType;

typedef struct {
    uint16 breakMin;        /**< Minimum pulse break time (in 125uS) */
    uint16 breakMax;        /**< Maximum pulse break time (in 125uS) */
    uint16 makeMin;         /**< Minimum pulse make time (in 125uS) */
    uint16 makeMax;         /**< Maximum pulse make time (in 125uS) */
    uint16 interDigitMin;   /**< Minimum pulse interdigit time (in 125uS) */
    uint16 flashMin;        /**< Minimum flash break time (in 125uS) */
    uint16 flashMax;        /**< Maximum flash break time (in 125uS) */
    uint16 onHookMin;       /**< Minimum on-hook time (in 125uS) */
    uint16 offHookMin;      /**< Minimum off-hook time (in 125uS) */
} VpOptionLinePulseType;

/** Method for line control when critical faults are detected */
typedef struct {
    /**< The line is set to disconnect when the specified fault is active and
     * the "En" bit is set TRUE
     */
    bool acFltDiscEn;           /**< AC fault detected */
    bool dcFltDiscEn;           /**< DC fault detected */
    bool thermFltDiscEn;        /**< Thermal fault detected */
} VpOptionCriticalFltType;

/** Method for zero-cross control */
typedef enum {
    VP_OPTION_ZC_M4B,   /**< Zero-Cross On - Make before break */
    VP_OPTION_ZC_B4M,   /**< Zero-Cross On - Break before make */
    VP_OPTION_ZC_NONE,   /**< Turn Zero-Cross control off */
    VP_OPTION_ZC_ENUM_RSVD    = FORCE_SIGNED_ENUM,
    VP_OPTION_ZC_ENUM_SIZE = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req. */
} VpOptionZeroCrossType;

/** Dial Pulse decode enable/disable */
typedef enum {
    VP_OPTION_PULSE_DECODE_OFF, /**< Disable Pulse Decode */
    VP_OPTION_PULSE_DECODE_ON,   /**< Enable Pulse Decode */
    VP_OPTION_PULSE_ENUM_SIZE = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} VpOptionPulseModeType;

/** Transmit/Receive Timeslot setting (timeslot and control) */
typedef struct {
    uint8 tx;   /**< 8-bit TX timeslot */
    uint8 rx;   /**< 8-bit RX timeslot */
} VpOptionTimeslotType;

typedef enum {
    VP_OPTION_ALAW,                 /**< Select G.711 A-Law PCM encoding */
    VP_OPTION_MLAW,                 /**< Select G.711 Mu-Law PCM encoding */
    VP_OPTION_LINEAR,               /**< Select Linear PCM encoding */
    VP_OPTION_WIDEBAND,             /**< Select Wideband PCM encoding */
    VP_NUM_OPTION_CODEC_TYPE_IDS,   /**< Select Linear PCM encoding */
    VP_OPTION_CODEC_ENUM_SIZE = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} VpOptionCodecType;

/** PCM Highway Selection (B valid on select devices only) */
typedef enum {
    VP_OPTION_HWY_A,            /**< Select the 'A' PCM Highway */
    VP_OPTION_HWY_B,            /**< Select the 'B' PCM Highway */
    VP_OPTION_HWY_TX_A_RX_B,    /**< Transmit on Highway A, receive on B */
    VP_OPTION_HWY_TX_B_RX_A,    /**< Transmit on Highway A, receive on A */
    VP_OPTION_HWY_TX_AB_RX_A,   /**< Transmit on Highway A and B, receive on A */
    VP_OPTION_HWY_TX_AB_RX_B,   /**< Transmit on Highway A and B, receive on B */
    VP_OPTION_HWY_ENUM_SIZE = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} VpOptionPcmHwyType;

/** Loopback option selection */
typedef enum {
    VP_OPTION_LB_OFF,           /**< All loopbacks off */

    /* Following loopback options are supported for CSLAC and VCP only */
    VP_OPTION_LB_TIMESLOT,      /**< Perform a timeslot loopback */
    VP_OPTION_LB_DIGITAL,       /**< Perform a full-digital loopback */
    VP_OPTION_LB_CHANNELS,      /**< Connects FXO to FXS line on same device */

    VP_NUM_LB_OPTIONS,
    VP_OPTION_LB_ENUM_SIZE = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} VpOptionLoopbackType;

/* DevNotes: The names of the following type need to be changed to make use
 * of mapped battery names like Bat1, Bat2 ...*/
/** Active Line State battery supply selection */
typedef enum {
    VP_OPTION_BAT_AUTO,     /**< Automatic Batery selection */
    VP_OPTION_BAT_HIGH,     /**< Use High Batery */
    VP_OPTION_BAT_LOW,      /**< Use Low Batery */
    VP_OPTION_BAT_BOOST,    /**< Include Positive Batery */
    VP_NUM_OPTION_BAT_IDS,
    VP_OPTION_BAT_ENUM_SIZE = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} VpOptionBatType;

/** Active Line State battery supply selection */
typedef struct {
    bool battRev;       /**< Smooth/Abrupt Battery Reversal (TRUE = abrupt) */

    VpOptionBatType bat;    /**< Battery selection for Active line state */
} VpOptionLineStateType;

/** Ring control option */
typedef enum {
    VP_LINE_STANDBY,        /**< Low power line feed state */
    VP_LINE_TIP_OPEN,       /**< Tip open circuit state */
    VP_LINE_ACTIVE,         /**< Line Feed w/out VF */
    VP_LINE_ACTIVE_POLREV,  /**< Polarity Reversal Line Feed w/out VF */
    VP_LINE_TALK,           /**< Normal off-hook Active State; Voice Enabled */
    VP_LINE_TALK_POLREV,    /**< Normal Active with reverse polarity;
                             *   Voice Enabled */

    VP_LINE_OHT,            /**< On-Hook tranmission state */
    VP_LINE_OHT_POLREV,     /**< Polarity Reversal On-Hook tranmission state */

    VP_LINE_DISCONNECT,     /**< Denial of service */
    VP_LINE_RINGING,        /**< Ringing state */
    VP_LINE_RINGING_POLREV, /**< Ringing w/Polarity Reversal */

    VP_LINE_FXO_OHT,        /**< FXO Line providing Loop Open w/VF */
    VP_LINE_FXO_LOOP_OPEN,  /**< FXO Line providing Loop Open w/out VF */
    VP_LINE_FXO_LOOP_CLOSE, /**< FXO Line providing Loop Close w/out VF */
    VP_LINE_FXO_TALK,       /**< FXO Line providing Loop Close w/VF */
    VP_LINE_FXO_RING_GND,   /**< FXO Line providing Ring Ground (GS only)*/

    VP_LINE_STANDBY_POLREV, /**< Low power polrev line feed state */
    VP_LINE_PARK,           /**< Park mode */
    VP_LINE_RING_OPEN,      /**< Ring open */
    VP_LINE_HOWLER,         /**< Howler */
    VP_LINE_TESTING,        /**< Testing */
    VP_LINE_DISABLED,       /**< Disabled */
    VP_LINE_NULLFEED,       /**< Null-feed */

    VP_NUM_LINE_STATES,
    VP_LINE_STATE_ENUM_RSVD = FORCE_SIGNED_ENUM,
    VP_LINE_STATE_ENUM_SIZE = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} VpLineStateType;

typedef struct {
    VpOptionZeroCrossType zeroCross;    /**< LCAS zero cross control */

    uint16 ringExitDbncDur; /**< Ringing Exit Debounce Duration; Used during end
                             * of ON periods of ringing cadences; 125uS
                             * resolution
                             */

    VpLineStateType ringTripExitSt; /**< State to automatically switch to upon
                                     * ring trip
                                     */
}  VpOptionRingControlType;

/** DTMF detection option */
typedef enum {
    VP_OPTION_DTMF_DECODE_OFF,      /**< Disable DTMF Digit Decode */
    VP_OPTION_DTMF_DECODE_ON,       /**< Enable DTMF Digit  Decode */
    VP_OPTION_DTMF_GET_STATUS,      /**< Do not change anything; Just get the
                                     *   DTMF status  */
    VP_NUM_OPTION_DTMF_IDS,
    VP_OPTION_DTMF_ENUM_SIZE = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} VpOptionDtmfModeControlType;

/* Device I/O Option related definitions */
typedef enum {
    VP_IO_INPUT_PIN,                /* Configure GPIO pin as input pin */
    VP_IO_OUTPUT_PIN,               /* Configure GPIO pin as output pin */
    VP_IO_DIR_ENUM_SIZE = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} VpDeviceIoDirectionType;

typedef enum {
    VP_OUTPUT_DRIVEN_PIN,           /* Configure as TTL/CMOS output pin */
    VP_OUTPUT_OPEN_PIN,             /* Configure as open collector/drain
                                     * output pin */
    VP_OUTPUT_TYPE_ENUM_SIZE = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} VpDeviceOutputPinType;

typedef struct {
    uint32 directionPins_31_0;      /* Device specific IO pin direction
                                     * (Pins 0 - 31) */
    uint32 directionPins_63_32;     /* Device specific IO pin direction
                                     * (Pins 32 - 63) */
    uint32 outputTypePins_31_0;     /* Output pin type (Pins 0 - 31) */
    uint32 outputTypePins_63_32;    /* Output pin type (Pins 32 - 63) */
} VpOptionDeviceIoType;

/* Definition for line I/O config option */
typedef struct {
    uint8 direction;
    uint8 outputType;
} VpOptionLineIoConfigType;

/* Definition for device I/O config option */
typedef struct {
    VpOptionLineIoConfigType lineIoConfig[VP_MAX_LINES_PER_DEVICE];
} VpOptionDeviceIoConfigType;

typedef enum {
    VP_OPTION_PCM_BOTH,             /* Enable both PCM transmit and receive
                                     * paths */
    VP_OPTION_PCM_RX_ONLY,          /* Enable PCM receive path only */
    VP_OPTION_PCM_TX_ONLY,          /* Enable PCM transmit path only */
    VP_OPTION_PCM_ALWAYS_ON,        /* Prevents disabling of PCM path */
    VP_PCM_TXRX_CNTRL_ENUM_RSVD    = FORCE_SIGNED_ENUM,
    VP_PCM_TXRX_CNTRL_ENUM_SIZE=FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} VpOptionPcmTxRxCntrlType;

/** Direction Specification */
typedef enum {
    VP_DIRECTION_DS,
    VP_DIRECTION_US,
    VP_DIRECTION_INVALID,   /**< Used by the API to determine if the direction
                             * field is valid */
    VP_DIRECTION_ENUM_SIZE = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} VpDirectionType;

#define VP_LINE_FLAG_BYTES ((VP_MAX_LINES_PER_DEVICE + 7) / 8)

/** DTMF detection option control */
typedef struct {
    VpOptionDtmfModeControlType dtmfControlMode; /**< DTMF detection
                                                  * Enable/Disable */
    VpDirectionType direction;                  /**< Detection direction */
    uint32 dtmfDetectionSetting;                /**< Indicates the DTMF
                                                 *   detection setting for first
                                                 *    32 lines */
    uint8 dtmfResourcesRemaining;               /**< DTMF decoder resources
                                                 *   remaining */
    uint8 dtmfDetectionEnabled[VP_LINE_FLAG_BYTES];
                                                /**< DTMF detection setting for
                                                 *   lines 7-0, 15-8, etc. */
} VpOptionDtmfModeType;

/** Regional DTMF Specs */
typedef enum {
    VP_OPTION_DTMF_SPEC_ATT,    /* Q.24 AT&T */
    VP_OPTION_DTMF_SPEC_NTT,    /* Q.24 NTT */
    VP_OPTION_DTMF_SPEC_AUS,    /* Q.24 Australian */
    VP_OPTION_DTMF_SPEC_BRZL,   /* Q.24 Brazilian */
    VP_OPTION_DTMF_SPEC_ETSI    /* ETSI ES 201 235-3 v1.3.1 */
} VpOptionDtmfSpecType;

/**< The following types are for Park Mode options */
typedef struct {
    uint16 discTime;    /**< Specified in 500ms increments, up to 8 seconds */
    uint16 standbyTime; /**< Specified in 100ms increments, up to 8 seconds */
} VpOptionParkModeType;


/** Hook detection modes  */
typedef enum {
    VP_OPTION_HOOKDET_NORMAL         = 0,  /* normal hook detection behavior */
    VP_OPTION_HOOKDET_DISC_IS_ONHOOK = 1,  /* in the VP_LINE_DISCONNECT or VP_LINE_DISABLED
                                              state, the hook status is always considered
                                              to be on-hook */
    VP_OPTION_HOOKDET_ENUM_SIZE = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} VpOptionHookDetectModeType;

/* The following struct can be passed to VpGetResults() if the option ID is not
   known at compile time, to ensure that the buffer is large enough regardless
   of the option type. */
typedef union {
    VpOptionPulseType pulse;                   /* VP_DEVICE_OPTION_ID_PULSE        */
                                               /* VP_DEVICE_OPTION_ID_PULSE2       */
    VpOptionCriticalFltType criticalFlt;       /* VP_DEVICE_OPTION_ID_CRITICAL_FLT */
    VpOptionZeroCrossType zeroCross;           /* VP_OPTION_ID_ZERO_CROSS          */
    uint16 ramp2stby;                          /* VP_DEVICE_OPTION_ID_RAMP2STBY    */
    VpOptionPulseModeType pulseMode;           /* VP_OPTION_ID_PULSE_MODE          */
    VpOptionTimeslotType timeslot;             /* VP_OPTION_ID_TIMESLOT            */
    VpOptionCodecType codec;                   /* VP_OPTION_ID_CODEC               */
    VpOptionPcmHwyType pcmHwy;                 /* VP_OPTION_ID_PCM_HWY             */
    VpOptionLoopbackType loopback;             /* VP_OPTION_ID_LOOPBACK            */
    VpOptionLineStateType lineState;           /* VP_OPTION_ID_LINE_STATE          */
    VpOptionEventMaskType eventMask;           /* VP_OPTION_ID_EVENT_MASK          */
    VpOptionRingControlType ringControl;       /* VP_OPTION_ID_RING_CNTRL          */
    VpOptionDtmfModeType dtmfMode;             /* VP_OPTION_ID_DTMF_MODE           */
    VpOptionDeviceIoType deviceIo;             /* VP_DEVICE_OPTION_ID_DEVICE_IO    */
    VpOptionPcmTxRxCntrlType pcmTxRxCntrl;     /* VP_OPTION_ID_PCM_TXRX_CNTRL      */
    VpOptionDeviceIoConfigType deviceIoConfig; /* VP_DEVICE_OPTION_ID_DEV_IO_CFG   */
    VpOptionLineIoConfigType lineIoConfig;     /* VP_OPTION_ID_LINE_IO_CFG         */
    VpOptionDtmfSpecType dtmfSpec;             /* VP_OPTION_ID_DTMF_SPEC           */
    VpOptionParkModeType parkMode;             /* VP_DEVICE_OPTION_ID_PARK_MODE    */
    VpOptionLinePulseType linePulse;           /* VP_OPTION_ID_PULSE               */
    uint16 dcFeedSlope;                        /* VP_OPTION_ID_DCFEED_SLOPE        */
    bool switcherCtrl;                         /* VP_OPTION_ID_SWITCHER_CTRL       */
    uint32 debugSelect;                        /* VP_OPTION_ID_DEBUG_SELECT        */
    VpOptionHookDetectModeType hookDetectMode; /* VP_OPTION_ID_HOOK_DETECT_MODE    */
} VpOptionValueType;

#endif /* VP_API_OPTION */
