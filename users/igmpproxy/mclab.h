/*
**  smcroute - static multicast routing control 
**  Copyright (C) 2001 Carsten Schill <carsten@cschill.de>
**
**  This program is free software; you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation; either version 2 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program; if not, write to the Free Software
**  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
**
**  $Id: mclab.h,v 1.1.1.1 2007-08-06 10:04:43 root Exp $	
**
*/

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <net/if.h>		/* ifreq struct         */

#define USE_LINUX_IN_H
#ifdef USE_LINUX_IN_H
# include <linux/in.h>
# include <linux/mroute.h>
# include <linux/ip.h>
# include <linux/igmp.h>
#else
# include <netinet/in.h>
#endif

typedef u_int8_t   uint8;
typedef u_int16_t  uint16;
typedef u_int32_t  uint32;

#define MIN( a, b ) ((a) < (b) ? (a) : (b))
#define MAX( a, b ) ((a) < (b) ? (b) : (a))
#define VCMC( Vc )  (sizeof( Vc ) / sizeof( (Vc)[ 0 ] ))
#define VCEP( Vc )  (&(Vc)[ VCMC( Vc ) ])

#define MAX_MC_VIFS    MAXVIFS     // !!! check this const in the specific includes
// #define NO_VIF_IX  MAXVIFS       /* invalid VIF index (32) */

struct IfDesc {
  char Name[ sizeof( ((struct ifreq *)NULL)->ifr_name ) ];
  struct in_addr InAdr;          /* == 0 for non IP interfaces */            
  short Flags;
  int vif_idx;
  int sock;
};

/* ifvc.c
 */
#define MAX_IF         MAXVIFS+8     // max. number of interfaces recognized 
void buildIfVc( void );
struct IfDesc *getIfByName( const char *IfName );
struct IfDesc *getIfByIx( unsigned Ix );

/* mroute-api.c
 */
struct MRouteDesc {
  struct in_addr OriginAdr;
  // Kaohj
  struct in_addr SubsAdr; // addr of the subscriber
  struct in_addr McAdr;
  short InVif;
  uint8 TtlVc[ MAX_MC_VIFS ];
};

// IGMP socket as interface for the mrouted API
// - receives the IGMP messages
extern int MRouterFD;

int enableMRouter( void );
void disableMRouter( void );
int addVIF( struct IfDesc *Dp );
int addMRoute( struct MRouteDesc * Dp );
int delMRoute( struct MRouteDesc * Dp );
int getVifIx( struct IfDesc *IfDp );

/* ipc.c
 */
int initIpcServer( void );
struct CmdPkt *readIpcServer( uint8 Bu[], int BuSz );
int initIpcClient( void );
int  sendIpc( const void *Bu, int Sz );
int  readIpc( uint8 Bu[], int BuSz );
void cleanIpc( void );

/* cmdpkt.c
 */
struct CmdPkt {
  unsigned PktSz;
  uint16   Cmd;
  uint16   ParCn;  
  // 'ParCn' * '\0' terminated strings + '\0'
};

#define MX_CMDPKT_SZ 1024   // CmdPkt size including appended strings

void *buildCmdPkt( char Cmd, const char *ArgVc[], int ParCn );
const char *convCmdPkt2MRouteDesc( struct MRouteDesc *MrDp, const struct CmdPkt *PktPt );

/* lib.c
 */
char *fmtInAdr( char *St, struct in_addr InAdr );
char *fmtSockAdr( char *St, const struct sockaddr_in *SockAdrPt );
int getInAdr( uint32 *InAdrPt, uint16 *PortPt, char *St );

/* mcgroup.c
 */
int joinMcGroup( int UdpSock, const char *IfName, struct in_addr McAdr );
int leaveMcGroup( int UdpSock, const char *IfName, struct in_addr McAdr );

/* syslog.c
 */
extern int  Log2Stderr;    // Log threshold for stderr, LOG_WARNING .... LOG_DEBUG 
extern int  LogLastServerity;     // last logged serverity
extern int  LogLastErrno;         // last logged errno value
extern char LogLastMsg[ 128 ];    // last logged message



void log( int Serverity, int Errno, const char *FmtSt, ... );

/* udpsock.c
 */
int openUdpSocket( uint32 PeerInAdr, uint16 PeerPort );









