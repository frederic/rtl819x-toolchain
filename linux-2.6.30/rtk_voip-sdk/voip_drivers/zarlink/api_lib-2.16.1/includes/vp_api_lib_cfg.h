/** \file vp_api_lib_cfg.h
 * vp_api_lib_cfg.h
 *
 * This file contains the default options for various libraries. In general
 * the default options are same as top level API default options. However
 * VP-API provides a mechanism to define default options that is different
 * compared to top level default options. This file provides such a mechanism.
 *
 * Copyright (c) 2010, Zarlink Semiconductor, Inc.
 *
 * $Revision: 6419 $
 * $LastChangedDate: 2010-02-12 16:40:10 -0600 (Fri, 12 Feb 2010) $
 */

#ifndef VP_API_LIB_CFG_H
#define VP_API_LIB_CFG_H

/******************************************************************************
 * Library Specific default options                                           *
 *****************************************************************************/
/* The following section provides mechanisms to define default options for
 * individual VTD families that is different compared to VP-API top level
 * default settins.
 *
 * NOTE: Users are free to change individual library's default settings as long
 * as it is not needed by the API and the VTD understands it.
 */

#ifdef VP_CC_VCP_SERIES

/* Default Event Masks for VCP */
/* First, define the events that VCP does not understand or not needed */
#define VCP_EVCAT_FAULT_NOT_NEEDED \
        (VP_DEV_EVID_EVQ_OFL_FLT    | /* VCP does not understand */ \
         VP_DEV_EVID_WDT_FLT)        /* VCP does not understand */
/* Second, define the events that are specially needed for the VCP API */
#define VCP_EVCAT_FAULT_NEEDED (~0x0000)

/* Third, Define the composite events */
#define VCP_OPTION_DEFAULT_FAULT_EVENT_MASK  \
    ((VP_OPTION_DEFAULT_FAULT_EVENT_MASK  |  /* Top level API default */ \
     VCP_EVCAT_FAULT_NOT_NEEDED) &         /* Events not needed for VCP*/ \
     VCP_EVCAT_FAULT_NEEDED)                /* Events needed for VCP */

/* First, define the events that VCP does not understand or not needed */
#define VCP_EVCAT_SIGNALLING_NOT_NEEDED \
        (VP_LINE_EVID_US_TONE_DETECT | /* VCP does not understand */  \
         VP_LINE_EVID_DS_TONE_DETECT | /* VCP does not understand */  \
         VP_DEV_EVID_SEQUENCER)       /* VCP does not understand */
/* Second, define the events that are specially needed for the VCP API */
#define VCP_EVCAT_SIGNALLING_NEEDED (~0x0000)

/* Third, Define the composite events */
#define VCP_OPTION_DEFAULT_SIGNALING_EVENT_MASK  \
    ((VP_OPTION_DEFAULT_SIGNALING_EVENT_MASK  |  /* Top level API default */ \
     VCP_EVCAT_SIGNALLING_NOT_NEEDED) &         /* Events not needed for VCP*/ \
     VCP_EVCAT_SIGNALLING_NEEDED)                /* Events needed for VCP */

/* First, define the events that VCP does not understand or not needed */
#define VCP_EVCAT_RESPONSE_NOT_NEEDED \
        (VP_DEV_EVID_DEV_INIT_CMP ) /* VCP does not understand */
/* Second, define the events that are specially needed for the VCP API */
#define VCP_EVCAT_RESPONSE_NEEDED (~0x0000)

/* Third, Define the composite events */
#define VCP_OPTION_DEFAULT_RESPONSE_EVENT_MASK  \
    ((VP_OPTION_DEFAULT_RESPONSE_EVENT_MASK  |  /* Top level API default */ \
     VCP_EVCAT_RESPONSE_NOT_NEEDED) &         /* Events not needed for VCP*/ \
     VCP_EVCAT_RESPONSE_NEEDED)                /* Events needed for VCP */

#define VCP_OPTION_DEFAULT_FXO_EVENT_MASK    (0xffff)/* VCP does not
                                                      * understand */

#define VCP_OPTION_DEFAULT_PACKET_EVENT_MASK (0xffff)/* VCP does not
                                                      * understand */
#endif /* VP_CC_VCP_SERIES */


#ifdef VP_CC_VCP2_SERIES
/* Default option settings for VCP2:  These are the default values applied at
 * VpInitDevice() and/or VpInitLine().  If your application uses more than one
 * type of device (VTD) and you with to specify VCP2-specific defaults that
 * differ from the defaults specified in vp_api_cfg.h, do so here. */

#include "vcp2_api.h"

/* VCP2-specific default event masks: */
#define VCP2_DEFAULT_MASK_FAULT     (VP_OPTION_DEFAULT_FAULT_EVENT_MASK     | VCP2_INVALID_FAULT_EVENTS)
#define VCP2_DEFAULT_MASK_SIGNALING (VP_OPTION_DEFAULT_SIGNALING_EVENT_MASK | VCP2_INVALID_SIGNALING_EVENTS)
#define VCP2_DEFAULT_MASK_RESPONSE  (VP_OPTION_DEFAULT_RESPONSE_EVENT_MASK  | VCP2_INVALID_RESPONSE_EVENTS)
#define VCP2_DEFAULT_MASK_TEST      (VP_OPTION_DEFAULT_TEST_EVENT_MASK      | VCP2_INVALID_TEST_EVENTS)
#define VCP2_DEFAULT_MASK_PROCESS   (VP_OPTION_DEFAULT_PROCESS_EVENT_MASK   | VCP2_INVALID_PROCESS_EVENTS)
#define VCP2_DEFAULT_MASK_FXO       (VP_OPTION_DEFAULT_FXO_EVENT_MASK       | VCP2_INVALID_FXO_EVENTS)
#define VCP2_DEFAULT_MASK_PACKET    (VP_OPTION_DEFAULT_PACKET_EVENT_MASK    | VCP2_INVALID_PACKET_EVENTS)

#endif

#endif /* VP_API_LIB_CFG_H */

