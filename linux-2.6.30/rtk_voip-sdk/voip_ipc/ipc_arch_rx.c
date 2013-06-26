#include <linux/string.h>
#include "rtk_voip.h"
//#include "../voip_rx/rtk_trap.h"
#include "voip_types.h"
#include "voip_init.h"
#include "voip_ipc.h"
#include "voip_limit.h"
#include "voip_dev.h"
#include "voip_timer.h"

#include "../voip_manager/voip_mgr_netfilter.h"
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
#include "con_mux.h"
#include "../voip_manager/voip_mgr_events.h"
#elif defined CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP
#include "ipc_arch_help_dsp.h"
#include "../voip_drivers/t38_handler.h"
#endif

#include "ipc_internal.h"

#include "ipc_arch_tx.h"
#include "ipc_arch_rx.h"

#ifdef IPC_ARCH_DEBUG_DSP
unsigned int dsp_ctrl_rx_cnt = 0;
unsigned int dsp_rtp_rtcp_rx_cnt = 0;
unsigned int dsp_t38_rx_cnt = 0;
unsigned int dsp_ack_rx_cnt = 0;
#endif

#ifdef IPC_ARCH_DEBUG_HOST
unsigned int host_resp_rx_cnt[CONFIG_RTK_VOIP_DSP_DEVICE_NR] = {0};
unsigned int host_rtp_rtcp_rx_cnt[CONFIG_RTK_VOIP_DSP_DEVICE_NR] = {0};
unsigned int host_t38_rx_cnt[CONFIG_RTK_VOIP_DSP_DEVICE_NR] = {0};
unsigned int host_event_rx_cnt[CONFIG_RTK_VOIP_DSP_DEVICE_NR] = {0};
#endif


#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
extern void system_process_rtp_tx(unsigned char CH, unsigned int sid, const void *ptr_data, unsigned int data_len);
#endif

//static unsigned char* pRes_content = NULL;
//static unsigned short res_len = 0, resp_category = 0, resp_dsp_id = 0;
//unsigned int Fax_Modem_Event_from_pkt[CON_CH_NUM] = {0};
unsigned int Fax_DIS_TX_Event_from_pkt[CON_CH_NUM] = {0};
unsigned int Fax_DIS_RX_Event_from_pkt[CON_CH_NUM] = {0};
//unsigned char res_content[100]={0};

#if 0
unsigned short get_response_content(unsigned short* pDsp_Id, unsigned char** pCont, unsigned short* pCategory)
{
	*pDsp_Id = resp_dsp_id;
	*pCont = pRes_content;
	*pCategory = resp_category;
	return res_len;
}
#endif

/* Host get DSP's voice packets and then send out. */
static void ipc_voice2host_parser( uint16 dsp_cpuid, 
								const ipc_voice_pkt_t* ipc_voice_pkt)
{
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	unsigned int chid, sid, mid, len, isRtcp = 0;
	const void * header;
	const voice_content_t * const voice_content = 
				( voice_content_t * )ipc_voice_pkt ->voice_content;

#if 1
	len = ipc_voice_pkt ->voice_cont_len;
	chid = voice_content ->chid;
	mid = voice_content ->mid;
	header = voice_content ->voice;
#else	
	len = *(unsigned short*)pkt;
	chid = *(unsigned int*)((unsigned char*)pkt+2);
	sid = *(unsigned int*)((unsigned char*)pkt+6);
	header = (unsigned char*)pkt+10;
#endif
	//PRINT_R("v");
	
	/* Transfer DSP-ID, chid to Host chid */
	//if (sid >= (2*ChNumPerDsp[dsp_id])) // 2*CHNUM_PER_DSP is DSP's RTCP_SID_OFFSET
	if( mid >= RTCP_MID_OFFSET )
	{
		//sid = sid - (2*ChNumPerDsp[dsp_id]);
		mid -= RTCP_MID_OFFSET;
		isRtcp = 1;
	}

	//mid = API_GetMid_Dsp(chid, sid, dsp_id);
	//if (mid == 255)
	//	PRINT_R("voip_voice2host_parser: (%d, %d)\n", chid, sid);

	chid = API_get_Host_CH(dsp_cpuid, chid); 	// get Host chid	

	sid = API_GetSid(chid, mid);		// get Host sid
	if (sid == 255)
		PRINT_G("voip_voice2host_parser: (%d, %d)\n", chid, mid);
	
	if (isRtcp == 1)
	{
		sid += RTCP_SID_OFFSET;
		//PRINT_R("rtcp tx sid=%d\n", sid);
		//PRINT_G("len=%d\n", len-8);
	}
	
	system_process_rtp_tx(chid, sid, header, len - SIZE_OF_VOICE_CONT_HEADER);
	//PRINT_G("%d,%d\n", chid, len - SIZE_OF_VOICE_CONT_HEADER);
#endif	
}

static void ipc_voice2dsp_parser(const ipc_voice_pkt_t* ipc_voice_pkt)
{
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP
	int chid, mid, sid, len, flags;
	const void * packet;
	const voice_rtp2dsp_content_t * const voice_rtp2dsp_content = 
				( voice_rtp2dsp_content_t * )ipc_voice_pkt ->voice_content;

#if 1
	len = ipc_voice_pkt ->voice_cont_len;
	chid = voice_rtp2dsp_content ->chid;
	mid = voice_rtp2dsp_content ->mid;
	flags = voice_rtp2dsp_content ->flags;
	packet = voice_rtp2dsp_content ->voice;
#else	
	len = *(unsigned short*)pkt;
	chid = *(unsigned int*)((unsigned char*)pkt+2);
	sid = *(unsigned int*)((unsigned char*)pkt+6);
	flags = *(unsigned int*)((unsigned char*)pkt+10);
	packet = (unsigned char*)pkt+14;
#endif
	//PRINT_G("v");
	
	sid = API_GetSid( chid, mid );
	
	DSP_pktRx(chid, sid, packet, len - SIZE_OF_VOICE_RTP2DSP_CONT_HEADER, flags );
	//PRINT_G("%d,%d\n", chid, len - SIZE_OF_VOICE_RTP2DSP_CONT_HEADER);
#endif	
}

static void ipc_t38_to_host_parser(uint32 dsp_cpuid, const ipc_voice_pkt_t* ipc_voice_pkt)
{
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	int chid, sid, mid, len, isRtcp = 0;
	const void * header;
	const voice_content_t * const voice_content = 
				( voice_content_t * )ipc_voice_pkt ->voice_content;

#if 1
	len = ipc_voice_pkt ->voice_cont_len;
	chid = voice_content ->chid;
	mid = voice_content ->mid;
	header = voice_content ->voice;
#else	
	len = *(unsigned short*)pkt;
	chid = *(unsigned int*)((unsigned char*)pkt+2);
	sid = *(unsigned int*)((unsigned char*)pkt+6);
	header = (unsigned char*)pkt+10;
#endif
	//PRINT_R("v");
	
	/* Transfer DSP-ID, chid to Host chid */
	//if (sid >= (2*ChNumPerDsp[dsp_id]))
	if( mid >= RTCP_MID_OFFSET )
	{
		//sid = sid - (2*ChNumPerDsp[dsp_id]);
		mid -= RTCP_MID_OFFSET;
		isRtcp = 1;
	}
	//mid = API_GetMid_Dsp(chid, sid, dsp_id);
	chid = API_get_Host_CH(dsp_cpuid, chid); 	// get Host chid
	sid = API_GetSid(chid, mid);		// get Host sid
	if (isRtcp == 1)
		sid += RTCP_SID_OFFSET;
	
	system_process_rtp_tx(chid, sid, header, len - SIZE_OF_VOICE_CONT_HEADER);
	//PRINT_G("%d,%d\n", chid, len - SIZE_OF_VOICE_CONT_HEADER);
	//printk("R%d ", chid);
#endif
}

static void ipc_t38_to_dsp_parser(const ipc_voice_pkt_t* ipc_voice_pkt)
{
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP
	int chid, mid, sid, len;
	const void * packet;
	const voice_content_t * const voice_content = 
				( voice_content_t * )ipc_voice_pkt ->voice_content;

#if 1
	len = ipc_voice_pkt ->voice_cont_len;
	chid = voice_content ->chid;
	mid = voice_content ->mid;
	packet = voice_content ->voice;
#else	
	len = *(unsigned short*)pkt;
	chid = *(unsigned int*)((unsigned char*)pkt+2);
	sid = *(unsigned int*)((unsigned char*)pkt+6);
	packet = (unsigned char*)pkt+10;
#endif
	
	sid = API_GetSid( chid, mid );
	
	T38_API_PutPacket( chid, packet, len - SIZE_OF_VOICE_CONT_HEADER );
	//printk("r%d ", chid);
#endif
}

static void ipc_control_parser( ipc_ctrl_pkt_t * ipc_ctrl_pkt )
{
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP
	unsigned short category = 0, length = 0, seq_no = 0;
	unsigned char * pContent = NULL;
	extern int do_voip_mgr_set_ctl(unsigned short cmd, unsigned char* para, unsigned short length, unsigned short seq_no);

#if 1
	category = ipc_ctrl_pkt ->category;
	seq_no = ipc_ctrl_pkt ->sequence;
	length = ipc_ctrl_pkt ->cont_len;
	pContent = ipc_ctrl_pkt ->content;
#else	
	category = *(unsigned short*)(pkt);
	seq_no = *(unsigned short*)((unsigned char*)pkt+2);
	length = *(unsigned short*)((unsigned char*)pkt+4);
	pContent = (unsigned char*)pkt+6;
#endif
	
	do_voip_mgr_set_ctl(category, pContent, length, seq_no);
#endif
}

static void ipc_response_parser(unsigned short dsp_id, const ipc_ctrl_pkt_t * ipc_ctrl_pkt)
{
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	uint16 category, length, seq_no;
	const unsigned char* pContent = NULL;
	extern int host_check_response_and_fill_content( 
			uint16 category, uint16 seq_no,
			const unsigned char *content, uint16 cont_len,
			unsigned short dsp_id );

	category = ipc_ctrl_pkt ->category;
	seq_no = ipc_ctrl_pkt ->sequence;
	length = ipc_ctrl_pkt ->cont_len;
	pContent = ipc_ctrl_pkt ->content;
	
	//PRINT_R("(%d,%d)(%d,%d)\n", category, seq_no, stCheckResponse.category, stCheckResponse.seqNo);
	// Check if received response pkt match with the send control pkt.
	if( host_check_response_and_fill_content( category, seq_no,
								pContent, length, dsp_id ) < 0 )
	{
		//PRINT_G("Get Resp(%d)\n", category);
	} 

#if 0	// pkshih: move into host_check_response() 
	resp_dsp_id = dsp_id;
	
	if (length > 0) // copy_to_user
	{
		
#if 0		
		// send ack packet which the same with category and seq_no of the response pkt
		TstTxPktCtrl pktCtrl;
		pktCtrl.seq_no = seq_no;
		pktCtrl.resend_flag = 0;
		ipc_pkt_tx_final(category, IPC_PROT_ACK, NULL, 0, &pktCtrl, NULL);
#endif
		pRes_content = pContent;
		res_len = length;
		resp_category = category;
	}
	else
	{
		pRes_content = NULL;
		res_len = 0;
	}
#endif 
#endif
}

static void ipc_event_parser(uint32 dsp_cpuid, const ipc_ctrl_pkt_t * ipc_ctrl_pkt)
{

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST

	uint16 category, length, seq_no;
	const unsigned char * pContent = NULL;
	TstTxPktCtrl pktCtrl;
	TstVoipEvent event;
	
	typedef struct {
		uint64 history;		// bit 0, 1, 2, ... represents time @0, -1, -2, ...
		uint16 seq_no;
	} received_t;
	
	static received_t received[ CONFIG_RTK_VOIP_DSP_DEVICE_NR ] = { { 0, 0 } };
	received_t *p_received = ( dsp_cpuid < CONFIG_RTK_VOIP_DSP_DEVICE_NR ?
								&received[ dsp_cpuid ] : NULL );
	
	int16 delta;
	
	category = ipc_ctrl_pkt ->category;
	seq_no = ipc_ctrl_pkt ->sequence;
	length = ipc_ctrl_pkt ->cont_len;
	pContent = ipc_ctrl_pkt ->content;
	
	event = *( ( const TstVoipEvent * )pContent );
	
	//PRINT_R("cat=%d, len=%d\n", category, length);
	
	// compare with received history 
	delta = seq_no - p_received ->seq_no;
	
	if( delta > 0 ) {	// newer one 
		p_received ->history <<= delta;
		p_received ->history |= 0x01;	// bit 0 indicates current 
		p_received ->seq_no = seq_no;
	} else {	// older one 
		delta *= -1;	// absolute value 
		
		if( delta >= sizeof( p_received ->history ) * 8 ) {
			// not exist in history... 
		} else if( p_received ->history & ( 1 << delta ) ) {
			// in history 
			PRINT_R("Get the same event pkt with seqno=%d\n", seq_no);
			
			// re-send ack packet which the same with category and seq_no of the event pkt
			goto label_send_event_ACK;
		} else {
			// out of order? save to history 
			p_received ->history |= ( 1 << delta );
		}
	}
	
#if 0
	int i;
	
	printk( "ipc_event_parser:\n" );
	
	for( i = 0; i < length; i ++ ) {
		printk( "%02X, ", *( pContent + i ) );
	}
	printk( "\n" );
#endif
	
	// convert event channel 
	event.ch_id = API_get_Host_CH( dsp_cpuid, event.ch_id );
	
	// write to event manager 
	voip_mgr_event_in_packed( &event );
	
	// addition flags setting 
	switch( event.id ) {
	case VEID_FAXMDM_FAX_DIS_TX:
		Fax_DIS_TX_Event_from_pkt[ event.ch_id ] = 1;
		break;
		
	case VEID_FAXMDM_FAX_DIS_RX:
		Fax_DIS_RX_Event_from_pkt[ event.ch_id ] = 1;
		break;
	
	default:
		break;
	}
	
	// send ACK packet which the same with category and seq_no of the event pkt
label_send_event_ACK:
	pktCtrl.dsp_cpuid = dsp_cpuid;
	pktCtrl.seq_no = seq_no;
	pktCtrl.resend_flag = 0;
	ipc_pkt_tx_final(category, IPC_PROT_ACK, NULL, 0, &pktCtrl, NULL);	
#endif
}

static void ipc_ack_parser(const ipc_ctrl_pkt_t * ipc_ctrl_pkt)
{
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP
	extern void clean_event_cheak_ack(unsigned short category, unsigned short rev_seq);

	unsigned short category, seq_no;
	//extern TstCheckAck stCheckAck;

#if 1
	category = ipc_ctrl_pkt ->category;
	seq_no = ipc_ctrl_pkt ->sequence;
#else	
	category = *(unsigned short*)(pkt);
	seq_no = *(unsigned short*)((unsigned char*)pkt+2);
#endif
	//PRINT_G("Get Ack(%d)\n", category);

	clean_event_cheak_ack(category, seq_no);
#endif	
}

void ipc_pkt_rx_entry( ipc_ctrl_pkt_t * ipc_pkt, uint32 ipc_pkt_len )
{
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP
	extern void ethernet_dsp_update_source_MAC( const ipc_ctrl_pkt_t *ipc_pkt );
	extern void ipc_mirror_parser( ipc_ctrl_pkt_t *ipc_ctrl );
	extern void ipc_rpc_ack_parser( ipc_ctrl_pkt_t *ipc_ctrl );
#endif
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	extern void ipc_rpc_parser( ipc_ctrl_pkt_t *ipc_ctrl );
	extern void ipc_mirror_ack_parser( const ipc_ctrl_pkt_t *ipc_ctrl );
#endif

	unsigned short pltype;
	uint32 dsp_cpuid;
	
	// provide simple access 
	ipc_ctrl_pkt_t * const ipc_ctrl_pkt = ipc_pkt;
	const ipc_voice_pkt_t * const ipc_voice_pkt = ( const ipc_voice_pkt_t * )ipc_pkt;
	
	// log TX data 
	if( ddinst_write( VOIP_DEV_IPC, LOG_IPC_RX_PREAMBLE, 4 ) >= 0 )
	{
		const uint32 now = timetick;
		
		ddinst_write( VOIP_DEV_IPC, ( const char * )&now, 4 );
		ddinst_write( VOIP_DEV_IPC, ( const char * )&ipc_pkt_len, 4 );
		ddinst_write( VOIP_DEV_IPC, ( const char * )ipc_pkt, ipc_pkt_len );
	}
	
	//dsp_id = *(unsigned short*)((unsigned char*)eth_pkt+DSP_ID_SHIFT);
	dsp_cpuid = ipc_pkt ->dsp_cpuid;
	//printk("dsp_id=%d\n", dsp_id);

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP
	if (dsp_cpuid != Get_DSP_CPUID())
		return;
#endif
	
#ifdef CONFIG_RTK_VOIP_ETHERNET_DSP_IS_DSP
	ethernet_dsp_update_source_MAC( ipc_pkt );
#endif

	//pltype = *(unsigned char*)((unsigned char*)eth_pkt+PROTOCOL_SHIFT);
	pltype = ipc_pkt ->protocol;
	
#if 0
	if( pltype != 0x31 ) {
		int i;
		
		printk("pt%x\n", pltype);
		
		for( i = 0; i < 30; i ++ )
			printk( "%02X ", *( ( unsigned char * )ipc_pkt + i ) );
		
		printk( "\n" );
	}
#endif
	
	switch(pltype)
	{
		case IPC_PROT_VOICE_TO_HOST://DSP -> Host
			//voip_voice2host_parser(dsp_id, (unsigned char*)eth_pkt+VOICE_CONT_LEN_SHIFT);
			ipc_voice2host_parser( dsp_cpuid, ipc_voice_pkt );
#ifdef IPC_ARCH_DEBUG_HOST
			host_rtp_rtcp_rx_cnt[dsp_cpuid]++;
#endif
			//PRINT_G("V\n");
			break;
			
		case IPC_PROT_VOICE_TO_DSP://Host -> DSP
			//voip_voice2dsp_parser((unsigned char*)eth_pkt+VOICE_CONT_LEN_SHIFT);
			ipc_voice2dsp_parser( ipc_voice_pkt );
#ifdef IPC_ARCH_DEBUG_DSP
			dsp_rtp_rtcp_rx_cnt++;
#endif
			//PRINT_G("V\n");
			break;
		
		case IPC_PROT_T38_TO_HOST:
			//voip_t38_to_host_parser(dsp_id, (unsigned char*)eth_pkt+VOICE_CONT_LEN_SHIFT);
			ipc_t38_to_host_parser(dsp_cpuid, ipc_voice_pkt);
#ifdef IPC_ARCH_DEBUG_HOST
			host_t38_rx_cnt[dsp_cpuid]++;
#endif
			//PRINT_G("38->H\n");
			break;
		
		case IPC_PROT_T38_TO_DSP:
			//voip_t38_to_dsp_parser((unsigned char*)eth_pkt+VOICE_CONT_LEN_SHIFT);
			ipc_t38_to_dsp_parser(ipc_voice_pkt);
#ifdef IPC_ARCH_DEBUG_DSP
			dsp_t38_rx_cnt++;
#endif
			//PRINT_G("38->D\n");
			break;
		
		case IPC_PROT_CTRL:
			//voip_control_parser((unsigned char*)eth_pkt+CATEGORY_SHIFT);
			ipc_control_parser(ipc_ctrl_pkt);
#ifdef IPC_ARCH_DEBUG_DSP
			dsp_ctrl_rx_cnt++;
#endif
			//PRINT_G("C\n");
			break;
			
		case IPC_PROT_RESP:
			//voip_response_parser(dsp_id, (unsigned char*)eth_pkt+CATEGORY_SHIFT);
			ipc_response_parser(dsp_cpuid, ipc_ctrl_pkt);
#ifdef IPC_ARCH_DEBUG_HOST
			host_resp_rx_cnt[dsp_cpuid]++;
#endif
			//PRINT_G("R\n");
			break;
			
		case IPC_PROT_EVENT:
			//voip_event_parser(dsp_id, (unsigned char*)eth_pkt+CATEGORY_SHIFT);
			ipc_event_parser(dsp_cpuid, ipc_ctrl_pkt);
#ifdef IPC_ARCH_DEBUG_HOST
			host_event_rx_cnt[dsp_cpuid]++;
#endif
			//PRINT_G("E\n");
			break;
			
		case IPC_PROT_ACK:
			//PRINT_G("Get Ack\n");
			//voip_ack_parser((unsigned char*)eth_pkt+CATEGORY_SHIFT);
			ipc_ack_parser(ipc_ctrl_pkt);
#ifdef IPC_ARCH_DEBUG_DSP
			dsp_ack_rx_cnt++;
#endif
			break;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP				
		case IPC_PROT_MIRROR:
			ipc_mirror_parser( ipc_ctrl_pkt );
#ifdef IPC_ARCH_DEBUG_DSP
			dsp_ack_rx_cnt++;
#endif
			break;
#endif
			
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
		case IPC_PROT_MIRROR_ACK:
			ipc_mirror_ack_parser( ipc_ctrl_pkt );
#ifdef IPC_ARCH_DEBUG_HOST
			host_event_rx_cnt[dsp_cpuid]++;
#endif
			break;
#endif

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
		case IPC_PROT_RPC:
			ipc_rpc_parser( ipc_ctrl_pkt );
#ifdef IPC_ARCH_DEBUG_HOST
			host_event_rx_cnt[dsp_cpuid]++;
#endif
			break;
#endif
			
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP				
		case IPC_PROT_RPC_ACK:
			ipc_rpc_ack_parser( ipc_ctrl_pkt );
#ifdef IPC_ARCH_DEBUG_DSP
			dsp_ack_rx_cnt++;
#endif
			break;
#endif

		//default:
			//PRINT_R("?? @ %s, %s, line-%d\n", __FUNCTION__, __FILE__, __LINE__);
			//break;
	}

}

