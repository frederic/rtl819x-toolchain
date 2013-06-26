#ifndef __STR_MIB_H__
#define __STR_MIB_H__

#include "mib_def.h"

/*
 * ====================================================================
 * Functions that convert ID to string (strdup)
 * ====================================================================
 */
extern char *strdup_Enable( enable_t enable );

extern char *strdup_SignalingProtocol( signaling_protocol_t SignallingProtocol );

extern char *strdup_DTMFMethod( DTMF_method_t DTMFMethod );

extern char *strdup_Transport( transport_t Transport );

extern char *strdup_LineStatus( line_status_t LineStatus );

/*
 * ====================================================================
 * Functions that convert string to ID
 * ====================================================================
 */
extern int str2id_Enable( const char *pszEnable, enable_t *pidEnable );

extern int str2id_SignalingProtocol( const char *pszSignalingProtocol, signaling_protocol_t *pidSignalingProtocol );

extern int str2id_DTMFMethod( const char *pszDTMFMethod, DTMF_method_t *pidDTMFMethod );

extern int str2id_Transport( const char *pszTransport, transport_t *pidTransport );

extern int str2id_LineStatus( const char *pszLineStatus, line_status_t *pidLineStatus );

#endif /* __STR_MIB_H__ */

