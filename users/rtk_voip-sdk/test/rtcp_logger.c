#include "voip_manager.h"

int rtcp_logger_main(int argc, char *argv[])
{
	TstVoipRtcpLogger stVoipRtcpLogger;
	
	if (argc == 3)
	{
		rtk_GetRtcpLogger(atoi(argv[1]), atoi(argv[2]), &stVoipRtcpLogger);
		
		printf( "chid = %d, sid = %d\n", stVoipRtcpLogger.ch_id, stVoipRtcpLogger.m_id );
		
		printf( "TX:\n" );
		printf( "	packet count: %u\n", stVoipRtcpLogger.TX_packet_count );
		printf( "	loss rate: max=%u, min=%u, avg=%u, cur=%u (1/256)\n",
						stVoipRtcpLogger.TX_loss_rate_max,
						stVoipRtcpLogger.TX_loss_rate_min,
						stVoipRtcpLogger.TX_loss_rate_avg,
						stVoipRtcpLogger.TX_loss_rate_cur );
		printf( "	jitter: max=%u, min=%u, avg=%u, cur=%u (timestamp)\n",
						stVoipRtcpLogger.TX_jitter_max,
						stVoipRtcpLogger.TX_jitter_min,
						stVoipRtcpLogger.TX_jitter_avg,
						stVoipRtcpLogger.TX_jitter_cur );
		printf( "	round trip delay: max=%u, min=%u, avg=%u, cur=%u (ms)\n",
						stVoipRtcpLogger.TX_round_trip_max,
						stVoipRtcpLogger.TX_round_trip_min,
						stVoipRtcpLogger.TX_round_trip_avg,
						stVoipRtcpLogger.TX_round_trip_cur );
		printf( "	MOS-LQ: max=%u, min=%u, avg=%u, cur=%u\n",
						stVoipRtcpLogger.TX_MOS_LQ_max,
						stVoipRtcpLogger.TX_MOS_LQ_min,
						stVoipRtcpLogger.TX_MOS_LQ_avg,
						stVoipRtcpLogger.TX_MOS_LQ_cur );
		printf( "	MOS-LQ*10: max=%u, min=%u, avg=%u, cur=%u\n",
						stVoipRtcpLogger.TX_MOS_LQ_max_x10,
						stVoipRtcpLogger.TX_MOS_LQ_min_x10,
						stVoipRtcpLogger.TX_MOS_LQ_avg_x10,
						stVoipRtcpLogger.TX_MOS_LQ_cur_x10 );

		printf( "RX:\n" );
		printf( "	packet count: %u\n", stVoipRtcpLogger.RX_packet_count );
		printf( "	loss rate: max=%u, min=%u, avg=%u, cur=%u (1/256)\n",
						stVoipRtcpLogger.RX_loss_rate_max,
						stVoipRtcpLogger.RX_loss_rate_min,
						stVoipRtcpLogger.RX_loss_rate_avg,
						stVoipRtcpLogger.RX_loss_rate_cur );
		printf( "	jitter: max=%u, min=%u, avg=%u, cur=%u (timestamp)\n",
						stVoipRtcpLogger.RX_jitter_max,
						stVoipRtcpLogger.RX_jitter_min,
						stVoipRtcpLogger.RX_jitter_avg,
						stVoipRtcpLogger.RX_jitter_cur );
		printf( "	round trip delay: max=%u, min=%u, avg=%u, cur=%u (ms)\n",
						stVoipRtcpLogger.RX_round_trip_max,
						stVoipRtcpLogger.RX_round_trip_min,
						stVoipRtcpLogger.RX_round_trip_avg,
						stVoipRtcpLogger.RX_round_trip_cur );
		printf( "	MOS-LQ: max=%u, min=%u, avg=%u, avg*10=%u, cur=%u\n",
						stVoipRtcpLogger.RX_MOS_LQ_max,
						stVoipRtcpLogger.RX_MOS_LQ_min,
						stVoipRtcpLogger.RX_MOS_LQ_avg,
						stVoipRtcpLogger.RX_MOS_LQ_avg_x10,
						stVoipRtcpLogger.RX_MOS_LQ_cur );
	}
	else
	{
		printf("Usage error!\n");
		printf("To get Session logger: rtcp_logger  chid  sid(0 or 1)  \n");

	}
	
	return 0;
}


