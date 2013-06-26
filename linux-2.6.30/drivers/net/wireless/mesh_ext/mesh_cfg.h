#ifndef _MESH_CFG_H_
#define _MESH_CFG_H_

#ifdef CONFIG_RTK_MESH 

#define PU_A4A6 // use A4/A6 to update proxy table
#define PU_STANDARD // use proxy update protocol

#ifdef PU_STANDARD
#define PU_STANDARD_RX // send proxy update message when receving a data frame
#define PU_STANDARD_SME // send proxy update message when assoc
#endif

#ifdef MESH_USE_METRICOP
#define MESH_METRIC_PERIOD_UPDATE 60*HZ // 1 mins (jiffies)
#define MESH_METRIC_TIMEOUT_FRAME 1000 // jiffies
#define MESH_METRIC_MAX_RETRY  3
#define MESH_METRIC_UNSPECIFIED 20000 // if metric = 0, response this value; if exceeding MAX_RETRY, update with this value
#endif

// TEST and DEBUG sub-section
//
// The following settings (DBG_NCTU_MESH, _11s_TEST_MODE_ and _11s_TRACE_MODE_) could
// be removed for release version.
//

#define SIMPLE_CH_UNI_PROTOCOL
 
#ifdef SIMPLE_CH_UNI_PROTOCOL
#define _11S_SS_COUNT1 3
#define _11S_SS_COUNT2 10
#endif

//#define D_ACL 	//tsananiu
//#define FREDDY

#if defined(CONFIG_RTL8192SE)
#define SIGNINTX rtl8192SE_signin_txdesc
#else
#define SIGNINTX signin_txdesc
#endif

#if 0
#define _11s_TRACE_MODE_
#ifndef _11s_TEST_MODE_
#undef _11s_TRACE_MODE_
#endif
#endif

// If DBG_NCTU_MESH is disabled, the corresponding flags in
// br_input.c, arp.c, icmp.c, skbuff.c, skbuff.h, netdevice.h and eth865x.c
// should NOT be enabled!
// #define DBG_NCTU_MESH 1

//
// End of TEST and DEBUG sub-section

#endif
#endif	// _MESH_CFG_H
