/** \file vp_api_dev_term.h
 * vp_api_dev_term.h
 *
 * This file contains declaration for VP-API Device and termination types.
 *
 * $Revision: 5442 $
 * $LastChangedDate: 2009-08-17 12:13:24 -0500 (Mon, 17 Aug 2009) $
 */

#ifndef VP_API_DEV_TERM_H
#define VP_API_DEV_TERM_H

/******************************************************************************
 *                                ENUMERATIONS                                *
 ******************************************************************************/
/* Valid Device Types supported by VP-API-II. For the corresponding part numbers
 * supported by each device type please see VP-API-II documentation.
 */
typedef enum
{
    VP_DEV_RSRVD1 = 0,
    VP_DEV_580_SERIES,
    VP_DEV_790_SERIES,
    VP_DEV_VCP_SERIES,
    VP_DEV_880_SERIES,
    VP_DEV_RSVD_SERIES,
    VP_DEV_VCP2_SERIES,
    VP_DEV_KWRAP,
    VP_DEV_890_SERIES,
    VP_DEV_792_SERIES,
    VP_DEV_792_GROUP,
    VP_NUM_DEV_TYPES,
    VP_DEV_ENUM_SIZE = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req. */
} VpDeviceType;

/**
 * Deprecated name (VCP) based on older name for VCP. All new applications
 * should use VCP instead
 */
#define VP_DEV_DVP_SERIES  VP_DEV_VCP_SERIES

/** Valid Termination Types (per line) supported by API-II */
typedef enum
{
    VP_TERM_FXS_GENERIC,    /* Generic FXS with no term type relays */
    VP_TERM_FXS_ISOLATE,    /* FXS with single isolation relay for T/R */

    VP_TERM_FXS_TITO_TL_R,  /**< FXS termination with SLIC = Le79232 or
                             * Le79252; with single control for test-in/
                             * test-out relays; with test load; with
                             * optional external ringing
                             */
    VP_TERM_FXS_CO_TL,      /**< FXS termination with SLIC = Le79232
                             * Le79242 or Le79252 and cutover relay and
                             * test load resistor */
    VP_TERM_FXS_75181,      /**< FXS termination with SLIC = Le79232,
                             * LCAS = Le75181
                             */
    VP_TERM_FXS_75282,      /**< FXS termination with SLIC = Le79232,
                             * LCAS = Le75282
                             */
    VP_TERM_FXS_RDT,        /**< FXS termination with SLIC = Le79232
                             * Le79242 or Le79252 and multipurpose relay
                             * that is used for ringing, disconnecting the
                             * loop and also for providing access to testout
                             * bus. */
    VP_TERM_FXS_RR,         /**< FXS termination with SLIC = Le79232
                             * Le79242 or Le79252 and relay for external
                             * ringing relay and reset relay. */
    VP_TERM_FXS_SPLITTER,
    VP_TERM_FXS_TO_TL,      /**< Le79252 With Test load and
                             * Test-out control */
    VP_TERM_FXS_LOW_PWR,

    VP_TERM_FXS_TI,         /**< FXS termination with Test-In relay */

    VP_TERM_FXS_SPLITTER_LP,    /**< FXS with Splitter Relay, Low-Power */

    VP_TERM_FXS_ISOLATE_LP,     /**< FXS with Splitter Relay, Low-Power */

    VP_TERM_FXO_GENERIC,
    VP_TERM_FXO_DISC,
    VP_TERM_FXO_RSRVD2,
    VP_TERM_FXO_RSRVD3,
    VP_TERM_FXO_RSRVD4,

    VP_TERM_FXS_CUSTOM,
    VP_TERM_FXO_CUSTOM,

    VP_NUM_TERM_TYPES,  /**< This value should be used for test purposes only.
                         * It is not garaunteed to equal the number of
                         * termination types defined
                         */

    VP_TERM_ENUM_SIZE = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req. */
} VpTermType;

#endif /* VP_API_DEV_TERM_H */
