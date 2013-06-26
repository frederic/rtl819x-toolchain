#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <netinet/in.h>	// ntohs / ntohl 
#include "linux/config.h"
#include "ipc_internal.h"
#include "voip_ipc.h"
#include "voip_debug.h"

/*
 * This is host program to analyze data captured from /dev/voip/ipc. 
 *
 * Usage example: 
 *  - PC side: 
 *    netcat-x86 -l -p 1234 | ./ipc_arch_viewer
 *    netcat-x86 -l -p 1234 | ./voip_ipc/ipc_arch_viewer -v -id -ip -ir -lM -lR -T
 *  - target side:
 *    cat /dev/voip/ipc | nc [PC.IP] 1234
 */

typedef enum {
	// Mode select 
	OUTFMT_MODE_BINARY 		= 0x00000001,	// output binary data 
	OUTFMT_MODE_SIMPLE 		= 0x00000002,	// output human readable data 
	
	// OUTFMT_MODE_SIMPLE's options 
	OUTFMT_NO_ETH_HEADER 	= 0x00000010,	// no ethernet header (14 bytes) - ETHERNET_HEADER_SIZE
	OUTFMT_NO_TIMESTAMP		= 0x00000020,	// no timestamp (default)
	OUTFMT_PARSE_IPC		= 0x00000040,	// parse IPC content to output better look 
	
	// OUTFMT_PARSE_IPC's options 
	OUTFMT_NO_VOICE_CONT	= 0x00000100,	// no voice content 
	OUTFMT_NO_VOIP_VER		= 0x00000200,	// no VoIP version 
	OUTFMT_DOT_4BYTES		= 0x00000400,	// every 4 bytes, use '.' instead of ' ' 
	OUTFMT_LIMIT_200_DATA	= 0x00000800,	// limit 200 byte data output  
	OUTFMT_NO_HUGE_DATA		= 0x00001000,	// Huge data (500bytes) only print preceding 100 bytes 
	OUTFMT_NO_VOICE_PKT		= 0x00002000,	// no voice packet 
	OUTFMT_NO_CONT_DATA		= 0x00004000,	// no detail data 
	
	// OUTFMT_PARSE_IPC's highlight options 
	OUTFMT_HL_VOICE		= 0x00100000,	// highlight voice 
	OUTFMT_HL_T38		= 0x00200000,	// highlight T.38 
	OUTFMT_HL_CRTL		= 0x00400000,	// highlight ctrl/response
	OUTFMT_HL_EVENT		= 0x00800000,	// highlight event/ack
	OUTFMT_HL_MIRROR	= 0x01000000,	// highlight mirror/mirror_ack
	OUTFMT_HL_RPC		= 0x02000000,	// highlight RPC/RPC_ack
	
	// Global options 
	OUTFMT_VERBOSE		= 0x10000000,	// verbose mode 
	
} outfmt_t;

#define DEFAULT_OUTFTM	( 		\
				OUTFMT_MODE_SIMPLE | OUTFMT_NO_ETH_HEADER | OUTFMT_PARSE_IPC | \
				OUTFMT_NO_VOICE_CONT | OUTFMT_NO_VOIP_VER | \
				OUTFMT_NO_TIMESTAMP |	\
				OUTFMT_DOT_4BYTES | OUTFMT_LIMIT_200_DATA | \
				OUTFMT_NO_HUGE_DATA )

#define ETHERNET_HEADER_SIZE	14

static const struct {
	outfmt_t bit;
	int indent;			// help display, max=2 (also used by cmd)
	const char *cmd;	// enable cmd char 
	const char *meaning;
} fmt_detail[] = {

	// Mode select 
	{ OUTFMT_MODE_BINARY,	0, "b", "Binary output mode" },
	{ OUTFMT_MODE_SIMPLE,	0, "s", "Readable output mode" },
	
	// OUTFMT_MODE_SIMPLE's options 
	{ OUTFMT_NO_ETH_HEADER,	1, "e", "No eth header" },
	{ OUTFMT_NO_TIMESTAMP,	1, "t", "No timestamp" },
	{ OUTFMT_PARSE_IPC,		1, "P", "Parse IPC to be fancy" }, 
	
	// OUTFMT_PARSE_IPC's options 
	{ OUTFMT_NO_VOICE_CONT,	2, "ic", "No voice content" },
	{ OUTFMT_NO_VOIP_VER,	2, "iv", "No VoIP version" },
	{ OUTFMT_DOT_4BYTES,	2, "iD", "Print dot per 4 bytes" }, 
	{ OUTFMT_LIMIT_200_DATA,2, "iL", "Limit 200 bytes data" },
	{ OUTFMT_NO_HUGE_DATA,	2, "ih", "Discard huge data" },
	{ OUTFMT_NO_VOICE_PKT,	2, "ip", "No voice packet" },
	{ OUTFMT_NO_CONT_DATA,	2, "ir", "No detail content data" },
	
	// OUTFMT_PARSE_IPC's highlight options 
	{ OUTFMT_HL_VOICE,	2, "lV", "Highlight voice" }, 
	{ OUTFMT_HL_T38,	2, "lT", "Highlight T.38" }, 
	{ OUTFMT_HL_CRTL,	2, "lC", "Highlight ctrl/response" }, 
	{ OUTFMT_HL_EVENT,	2, "lE", "Highlight event/ack" }, 
	{ OUTFMT_HL_MIRROR,	2, "lM", "Highlight mirror/mirror_ack" }, 
	{ OUTFMT_HL_RPC,	2, "lR", "Highlight RPC/RPC_ack" }, 
	
	// Global options 
	{ OUTFMT_VERBOSE,	0, "v", "Verbose mode" },

};
#define SIZE_OF_FMT_DETAIL		( sizeof( fmt_detail ) / sizeof( fmt_detail[ 0 ] ) )

static inline char INVCASE( char ch )
{
	if( ( uint8 )ch >= ( uint8 )'a' &&
		( uint8 )ch <= ( uint8 )'z' )
	{
		return ch - ( uint8 )'a' + ( uint8 )'A';
	}
	
	if( ( uint8 )ch >= ( uint8 )'A' &&
		( uint8 )ch <= ( uint8 )'Z' )
	{
		return ch - ( uint8 )'A' + ( uint8 )'a';
	}
	
	return ch;
}

static inline char chrlwr( char ch )
{
	if( ( uint8 )ch >= ( uint8 )'A' &&
		( uint8 )ch <= ( uint8 )'Z' ) 
	{
		return ch - ( uint8 )'A' + ( uint8 )'a';
	}
	
	return ch;
}

static void viewer_help( void )
{
	int i, j;
	
	printf( "ipc_arch_viewer:\n" );
	
	for( i = 0; i < SIZE_OF_FMT_DETAIL; i ++ ) {
		
		// print indent 
		for( j = 0; j <= fmt_detail[ i ].indent; j ++ )
			printf( "\t" );
		
		// command: 
		switch( fmt_detail[ i ].indent ) {
		case 0:
			printf( "-%s:", fmt_detail[ i ].cmd );
			break;
		case 1:
			printf( "-%s/-%c:", fmt_detail[ i ].cmd, 
								INVCASE( fmt_detail[ i ].cmd[ 0 ] ) );
			break;
		case 2:
			printf( "-%s/-%c%c:", fmt_detail[ i ].cmd, 
								fmt_detail[ i ].cmd[ 0 ], 
								INVCASE( fmt_detail[ i ].cmd[ 1 ] ) );			
			break;
		}
		
		// detail (default) 
		printf( " %s", fmt_detail[ i ].meaning );
		
		// (default:xxx)
		if( fmt_detail[ i ].bit & DEFAULT_OUTFTM )
			printf( "(default:-%s)\n", fmt_detail[ i ].cmd );
		else
			printf( "\n" );
	}
	
	printf( "	-h: help screen\n"
			"\n"
			"Usage example:\n"
			"	PC:     netcat-x86 -l -p 1234 | ./voip_ipc/ipc_arch_viewer -v -id -ip -lM -lR -T\n"
			"	TARGET: cat /dev/voip/ipc | nc [PC.IP] 1234\n"
			);
}

static int parse_parameters( int argc, char *argv[], outfmt_t *outfmt )
{
	char ch1, ch2, found_ch;
	int i, j;
	int done, found;
	
	for( i = 1; i < argc; i ++ ) {
		
		if( argv[ i ][ 0 ] != '-' )
			continue;
		
		ch1 = argv[ i ][ 1 ];
		ch2 = argv[ i ][ 2 ];
		
		switch( ch1 ) {
		case 's':	// output mode - simple 
			*outfmt |= OUTFMT_MODE_SIMPLE;
			*outfmt &= ~OUTFMT_MODE_BINARY;
			break;
		
		case 'b':	// output mode - binary  
			*outfmt |= OUTFMT_MODE_BINARY;
			*outfmt &= ~OUTFMT_MODE_SIMPLE;
			break;
		
		case 'v':	// verbose mode 
			*outfmt |= OUTFMT_VERBOSE;
			break;
			
		case 'h':
			viewer_help();
			return 1;
			break;
			
		default:
			goto label_parse_table;
			break;
		}
		
		continue;

label_parse_table:
		done = 0;
		
		for( j = 0; j < SIZE_OF_FMT_DETAIL; j ++ ) {
			
			found = 0;
			
			// check one char cmd 
			switch( fmt_detail[ j ].indent ) {
			case 1:
				if( chrlwr( ch1 ) == chrlwr( fmt_detail[ j ].cmd[ 0 ] ) ) {
					found = 1;
					found_ch = ch1;
					done = 1;					
				}
				break;
				
			case 2:
				if( ch1 == fmt_detail[ j ].cmd[ 0 ] &&
					chrlwr( ch2 ) == chrlwr( fmt_detail[ j ].cmd[ 1 ] ) ) 
				{
					found = 2;
					found_ch = ch2;
					// add fmt and continue to scan others 
				}
				break;
			}
			
			// not found 
			if( !found )
				continue;
			
			// add fmt 
			if( found_ch == fmt_detail[ j ].cmd[ found - 1 ] )
			{
				// positive 
				*outfmt |= fmt_detail[ j ].bit;
			} else {
				// negative 
				*outfmt &= ~fmt_detail[ j ].bit;
			}
			
		} // j
	} // i 
	
	return 0;
}

static void detail_outfmt_parameters( outfmt_t outfmt )
{
	int i;
	
	for( i = 0; i < SIZE_OF_FMT_DETAIL; i ++ ) {

		printf( "%s: %s\n", fmt_detail[ i ].meaning, 
							( ( outfmt & fmt_detail[ i ].bit ) ? "yes" : "no" ) );
	}
	
	printf( "\n" );
}

static void print_fancy_ipc_pkt( const ipc_ctrl_pkt_t * ipc_pkt, outfmt_t outfmt )
{
	static const struct { 
		uint8 protocol;
		const char *name;
		outfmt_t highlight;
		const char *color;
	} protocol_string[] = {
		{ IPC_PROT_VOICE_TO_DSP,	"VO_H2D",	OUTFMT_HL_VOICE,	AC_FORE_RED },
		{ IPC_PROT_VOICE_TO_HOST,	"VO_D2H",	OUTFMT_HL_VOICE,	AC_FORE_RED },
		{ IPC_PROT_CTRL,			"CTRL",		OUTFMT_HL_CRTL,		AC_FORE_GREEN },
		{ IPC_PROT_RESP,			"RESP",		OUTFMT_HL_CRTL,		AC_FORE_GREEN },
		{ IPC_PROT_EVENT,			"EVENT",	OUTFMT_HL_EVENT,	AC_FORE_GREEN },
		{ IPC_PROT_ACK,				"ACK",		OUTFMT_HL_EVENT,	AC_FORE_GREEN },
		{ IPC_PROT_T38_TO_DSP,		"T38_H2D",	OUTFMT_HL_T38,		AC_FORE_RED },
		{ IPC_PROT_T38_TO_HOST,		"T38_D2H",	OUTFMT_HL_T38,		AC_FORE_RED },
		{ IPC_PROT_MIRROR,			"MIRROR",	OUTFMT_HL_MIRROR,	AC_FORE_YELLOW },
		{ IPC_PROT_MIRROR_ACK,		"MIR_ACK",	OUTFMT_HL_MIRROR,	AC_FORE_YELLOW },
		{ IPC_PROT_RPC,				"RPC",		OUTFMT_HL_RPC,		AC_FORE_BlUE },
		{ IPC_PROT_RPC_ACK,			"RPC_ACK",	OUTFMT_HL_RPC,		AC_FORE_BlUE },
	};
	#define SIZE_OF_PROT_STRING		( sizeof( protocol_string ) / sizeof( protocol_string[ 0 ] ) )
	
	int i;
	//int len;
	
	// byte out - common function 
	int byte_len = 0;	// may < 0 
	const uint8 *byte_ptr;
	
	const ipc_ctrl_pkt_t * const ipc_ctrl_pkt = ( const ipc_ctrl_pkt_t * )ipc_pkt;
	const ipc_voice_pkt_t * const ipc_voice_pkt = ( const ipc_voice_pkt_t * )ipc_pkt;
	const voice_content_t * const voice_content = ( const voice_content_t * )ipc_voice_pkt ->voice_content;
	const voice_rtp2dsp_content_t * const voice_rtp2dsp_content = ( const voice_rtp2dsp_content_t * )ipc_voice_pkt ->voice_content;
	const mirror_all_content_t * const mirror_all_content = ( const mirror_all_content_t * )ipc_ctrl_pkt ->content;
	const rpc_content_t * const rpc_content = ( const rpc_content_t * )ipc_ctrl_pkt ->content;
	
	// is voice packet ? 
	if( outfmt & OUTFMT_NO_VOICE_PKT ) {
		switch( ipc_pkt ->protocol ) {
		case IPC_PROT_VOICE_TO_DSP:
		case IPC_PROT_VOICE_TO_HOST:
		case IPC_PROT_T38_TO_DSP:
		case IPC_PROT_T38_TO_HOST:
			printf( "\x1B[1k\x0D" );	// erase & move to home 
			return;
			break;
		}
	}
	
	// ethernet header 
	if( outfmt & OUTFMT_NO_ETH_HEADER ) 
		goto label_basic_info;
	
	printf( "DST:" );
	for( i = 0; i < 6; i ++ )
		printf( "%02X", ipc_pkt ->dstMAC[ i ] );
	
	printf( "SRC:" );
	for( i = 0; i < 6; i ++ )
		printf( "%02X", ipc_pkt ->srcMAC[ i ] );
	
	printf( "TYPE:" );
	printf( "%04X ", ntohs( ipc_pkt ->ethType ) );
	
	// basic info 
label_basic_info:
	
	printf( "cpuid=%u ", ntohs( ipc_pkt ->dsp_cpuid ) );
	
	for( i = 0; i < SIZE_OF_PROT_STRING; i ++ )
		if( ipc_pkt ->protocol == protocol_string[ i ].protocol )
			break;
	
	if( i < SIZE_OF_PROT_STRING ) {
		if( outfmt & protocol_string[ i ].highlight ) {
			printf( "%s%-7s " AC_RESET, 
								protocol_string[ i ].color, 
								protocol_string[ i ].name );
		} else
			printf( "%-7s ", protocol_string[ i ].name );
	} else
		printf( "PROTUND " );
	
	if( !( outfmt & OUTFMT_NO_VOIP_VER ) )
		printf( "ver=%u ", ipc_pkt ->voip_ver );
	
	// content 
	switch( ipc_pkt ->protocol ) {
	case IPC_PROT_VOICE_TO_DSP:		//    host --> dsp 
		printf( "len=%u ", ntohs( ipc_voice_pkt ->voice_cont_len ) );
		printf( "cch=%u mid=%u flag=%u ", 
							ntohs( voice_rtp2dsp_content ->chid ), 
							ntohs( voice_rtp2dsp_content ->mid ), 
							ntohs( voice_rtp2dsp_content ->flags ) );
		
		if( ( outfmt & OUTFMT_NO_VOICE_CONT ) == 0 ) {
			byte_ptr = voice_rtp2dsp_content ->voice;
			byte_len = ntohs( ipc_voice_pkt ->voice_cont_len );
		} 
		break;

	case IPC_PROT_VOICE_TO_HOST:	//    host <-- dsp 
	case IPC_PROT_T38_TO_DSP:		//    host --> dsp 
	case IPC_PROT_T38_TO_HOST:		//    host <-- dsp 
		printf( "len=%u ", ntohs( ipc_voice_pkt ->voice_cont_len ) );
		printf( "cch=%u mid=%u ", 
							ntohs( voice_content ->chid ), 
							ntohs( voice_content ->mid ) );
		
		if( ( outfmt & OUTFMT_NO_VOICE_CONT ) == 0 ) {
			byte_ptr = voice_content ->voice;
			byte_len = ntohs( ipc_voice_pkt ->voice_cont_len );
		} 
		break;
		
	case IPC_PROT_CTRL:				// a) host --> dsp 
	case IPC_PROT_RESP:				// a) host <-- dsp 
	case IPC_PROT_EVENT:			// b) host <-- dsp 
	case IPC_PROT_ACK:				// b) host --> dsp 
	case IPC_PROT_MIRROR:			// c) host <-- dsp
	case IPC_PROT_MIRROR_ACK:		// c) host --> dsp
		printf( "cate=%u seq=%u len=%u ", 
				ntohs( ipc_ctrl_pkt ->category ),
				ntohs( ipc_ctrl_pkt ->sequence ),
				ntohs( ipc_ctrl_pkt ->cont_len ) );
		
		if( ipc_pkt ->protocol == IPC_PROT_MIRROR ||
			ipc_pkt ->protocol == IPC_PROT_MIRROR_ACK )
		{
			if( ipc_ctrl_pkt ->cont_len > 0 )
				printf( "cch=%u ", ntohs( mirror_all_content ->cch ) );
				
			i = SIZE_OF_MIRROR_CONT_PLUS_HEADER;
		} else
			i = 0;
		
		byte_ptr = ipc_ctrl_pkt ->content + i;
		byte_len = ntohs( ipc_ctrl_pkt ->cont_len ) - i;	// len may < 0
		
		break;
		
	case IPC_PROT_RPC:				// d) host <-- dsp
	case IPC_PROT_RPC_ACK:			// d) host --> dsp	
		printf( "cate=%u seq=%u len=%u ", 
				ntohs( ipc_ctrl_pkt ->category ),
				ntohs( ipc_ctrl_pkt ->sequence ),
				ntohs( ipc_ctrl_pkt ->cont_len ) );
		
		printf( "cch=%u ops=%u ", 
							ntohs( rpc_content ->cch ), 
							ntohs( rpc_content ->ops_offset ) );
		
		byte_ptr = rpc_content ->data;
		byte_len = ntohs( ipc_ctrl_pkt ->cont_len ) -
				// minus RPC header 
				( ( uint32 )rpc_content ->data - ( uint32 )rpc_content );
		
		break;
	}
	
	// byte output 
	for( i = 0; i < byte_len; i ++ ) {
		
		if( ( outfmt & OUTFMT_NO_CONT_DATA ) )
			break;
		
		if( ( outfmt & OUTFMT_DOT_4BYTES ) &&
			( ( i & 0x03 ) == 0x03 ) ) {
			printf( "%02X.", *byte_ptr ++ );
		} else
			printf( "%02X ", *byte_ptr ++ );
		
		// discard 1 - only preceding 200 bytes 
		if( ( outfmt & OUTFMT_LIMIT_200_DATA ) &&
			( i > 200 ) )
		{
			printf( "...... " );
			break;
		}
		
		// discard 2 - over 500 bytes huge data, only preceding 100 bytes 
		if( ( outfmt & OUTFMT_NO_HUGE_DATA ) &&
			( i > 100 ) && ( byte_len > 500 ) )
		{
			printf( "...... " );
			break;			
		}
	}
	
	printf( "\n" );	
}

static struct {
	struct {
		uint64 bytes;
		uint32 packets;
	} all, tx, rx;
	struct {
		uint32 bytes;
	} temp;
} summary;

#define INC_TOTAL_BYTES()		{ summary.all.bytes ++; summary.temp.bytes ++; }
#define INC_TOTAL_PACKET()		{ summary.all.packets ++; }
#define INC_TX_PACKET()			{ summary.tx.packets ++; summary.tx.bytes += summary.temp.bytes; summary.temp.bytes = 0; }
#define INC_RX_PACKET()			{ summary.rx.packets ++; summary.rx.bytes += summary.temp.bytes; summary.temp.bytes = 0; }

static void int_signal_handler( int sig )
{
	// handle SIGINT
	
	printf( "\n" );
	printf( " Dir   Bytes   Pkts               \n" );
	printf( "----- ------- ------ -------------\n" );
	printf( " All  %7llu %6u\n", 
					summary.all.bytes,
					summary.all.packets );
	printf( " TX   %7llu %6u\n", 
					summary.tx.bytes,
					summary.tx.packets );
	printf( " RX   %7llu %6u\n", 
					summary.rx.bytes,
					summary.rx.packets );
	
	exit( 0 );
}

typedef enum {
	FSM_NONE,
	FSM_PREAMBLE,	// 4 bytes 
	FSM_TIMESTAMP,	// 4 bytes 
	FSM_LENGTH,		// 4 bytes 
	FSM_DATA,		// variables size (give warning if larger than MAX_READ_DATA_LENGTH) 
	FSM_DONE,
} fsm_t;

int main(int argc, char *argv[])
{
	// FSM_PREAMBLE
	#define PREAMBLE_LENGTH		4
	const char *szTxPreamble = LOG_IPC_TX_PREAMBLE;
	const char *szRxPreamble = LOG_IPC_RX_PREAMBLE;
	int bMatchTxPreamble = 0, bMatchRxPreamble = 0;
	int idxMatchPreamble = 0;
	
	// FSM_TIMESTAMP
	#define TIMESTAMP_LENGTH	4
	unsigned long timestamp = 0;
	int timestampShift = 0;
	
	// FSM_LENGTH
	unsigned long dataLength = 0;
	int lengthShift = 0;
	
	// FSM_DATA
	#define MAX_READ_DATA_LENGTH	2048
	unsigned char readData[ MAX_READ_DATA_LENGTH ];
	unsigned long readDataLength = 0;
	
	// settings 
	outfmt_t outfmt = DEFAULT_OUTFTM;
	
	int ch;	
	fsm_t fsm = FSM_NONE, prev_fsm = FSM_NONE;
	
	// register SIGINT signal 
	signal( SIGINT, int_signal_handler );
	
	// parse parameters 
	if( parse_parameters( argc, argv, &outfmt ) )
		return 0;
	
	// detail output format 
	if( outfmt & OUTFMT_VERBOSE )
		detail_outfmt_parameters( outfmt );
	
	while( ( ch = getchar() ) != EOF ) {
		
		INC_TOTAL_BYTES();
		
		// FSM 
		switch( fsm ) {
		case FSM_NONE:
		case FSM_DONE:
			if( ch == szTxPreamble[ 0 ] ) {
				bMatchTxPreamble = 1;
				idxMatchPreamble = 1;
				fsm = FSM_PREAMBLE;
			}
			
			if( ch == szRxPreamble[ 0 ] ) {
				bMatchRxPreamble = 1;
				idxMatchPreamble = 1;
				fsm = FSM_PREAMBLE;
			}
			break;
			
		case FSM_PREAMBLE:
			if( bMatchTxPreamble && ch != szTxPreamble[ idxMatchPreamble ] ) {
				bMatchTxPreamble = 0;
			}
			
			if( bMatchRxPreamble && ch != szRxPreamble[ idxMatchPreamble ] ) {
				bMatchRxPreamble = 0;
			}
			
			if( bMatchTxPreamble == 0 && bMatchRxPreamble == 0 ) {
				fsm = FSM_NONE;
			} else {
				idxMatchPreamble ++;
				
				if( idxMatchPreamble >= PREAMBLE_LENGTH ) {
					fsm = FSM_TIMESTAMP;
					
					timestamp = 0;
					timestampShift = 24;	// big endian data 
				}
			}
				
			break;
		
		case FSM_TIMESTAMP:
			timestamp |= ( ( unsigned long )ch << timestampShift );
			
			timestampShift -= 8;
			
			if( timestampShift < 0 ) {
				fsm = FSM_LENGTH;
				dataLength = 0;
				lengthShift = 24;	// big endian data 			
			}
			
			break;
		
		case FSM_LENGTH:
			dataLength |= ( ( unsigned long )ch << lengthShift );
			
			lengthShift -= 8;
			
			if( lengthShift < 0 ) {
				
				if( dataLength > MAX_READ_DATA_LENGTH )
					fprintf( stderr, "dataLength > %d !!!\n", MAX_READ_DATA_LENGTH );
				
				fsm = FSM_DATA;
				readDataLength = 0;
			}
			
			break;
			
		case FSM_DATA:
			readData[ readDataLength ] = ( unsigned char )ch;
			readDataLength ++;
			
			if( readDataLength >= dataLength ) {
				fsm = FSM_DONE;
				INC_TOTAL_PACKET();
				
				if( bMatchTxPreamble ) {
					INC_TX_PACKET();
				} else {
					INC_RX_PACKET();
				}
			}
			
			break;
		}
		
		// output 1) human readable format 
		if( ( outfmt & OUTFMT_MODE_SIMPLE ) == 0 )
			goto label_outfmt_is_binary;
			
		switch( fsm ) {
		case FSM_NONE:
		case FSM_PREAMBLE:
			break;
		
		case FSM_TIMESTAMP:
			if( prev_fsm == FSM_PREAMBLE ) {
				if( bMatchTxPreamble )
					printf( "TX: " );
				else if( bMatchRxPreamble )
					printf( "RX: " );
				else
					printf( "XX: " );
			}
			break;

		case FSM_LENGTH:
			if( ( prev_fsm == FSM_TIMESTAMP ) &&
				( ( outfmt & OUTFMT_NO_TIMESTAMP ) == 0 ) )
			{
				printf( "%02lu.%02lu ", ( timestamp / 100 ) % 100
										, timestamp % 100 );
			}
			break;
			
		case FSM_DATA:
			if( prev_fsm == FSM_LENGTH )
				printf( "len=%-3lu ", dataLength );
			else {
				if( outfmt & OUTFMT_PARSE_IPC )
					;	// print when FSM_DONE
				else if( ( outfmt & OUTFMT_NO_ETH_HEADER ) == 0 || 
						 ( readDataLength > ETHERNET_HEADER_SIZE ) )
				{
					printf( "%02X ", ch );
				}
			}
			break;
			
		case FSM_DONE:
			if( prev_fsm == FSM_DATA ) {
				if( outfmt & OUTFMT_PARSE_IPC )
					print_fancy_ipc_pkt( ( const ipc_ctrl_pkt_t * )readData, outfmt );
				else
					printf( "%02X\n", ch );
			}
			break;
		}
		
		goto label_next;
		
		// output 2) binary format 
label_outfmt_is_binary:
		
		printf( "%02X ", ch );
		
		switch( fsm ) {
		case FSM_NONE:
		case FSM_PREAMBLE:
		case FSM_TIMESTAMP:
		case FSM_LENGTH:
		case FSM_DATA:
			break;
			
		case FSM_DONE:
			if( prev_fsm == FSM_DATA )
				printf( "\n" );
			break;
		}
		
		// save fsm 
label_next:
		prev_fsm = fsm;
	}
	
	return 0;
}

