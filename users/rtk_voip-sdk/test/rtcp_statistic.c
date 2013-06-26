#include "voip_manager.h"

int rtcp_main(int argc, char *argv[])
{
	TstVoipRtpStatistics stVoipRtpStatistics;
	TstVoipSessionStatistics stVoipSessionStatistics;
	
	if (argc == 3)
	{
		rtk_Get_Rtp_Statistics(atoi(argv[1]), atoi(argv[2]), &stVoipRtpStatistics);

		printf("Rx(byte) = %d\n", stVoipRtpStatistics.nRxRtpStatsCountByte);
		printf("Rx(pkt) = %d\n", stVoipRtpStatistics.nRxRtpStatsCountPacket);
		printf("Rx(lost pkt) = %d\n", stVoipRtpStatistics.nRxRtpStatsLostPacket);
		printf("Tx(byte) = %d\n", stVoipRtpStatistics.nTxRtpStatsCountByte);
		printf("Tx(pkt) = %d\n", stVoipRtpStatistics.nTxRtpStatsCountPacket);
	}
	else if (argc == 4)
	{
		rtk_Get_Session_Statistics( atoi(argv[1]), atoi(argv[2]), atoi(argv[3]), &stVoipSessionStatistics);

		printf("Rx Silence packet = %d\n", stVoipSessionStatistics.nRxSilencePacket);
		printf("Tx Silence packet = %d\n", stVoipSessionStatistics.nTxSilencePacket);
		printf("Average Playout Delay(unit:1ms) = %d\n", stVoipSessionStatistics.nAvgPlayoutDelay);
		printf("Current Jitter Buffer packet  = %d\n", stVoipSessionStatistics.nCurrentJitterBuf);
		printf("Early packet  = %d\n", stVoipSessionStatistics.nEarlyPacket);
		printf("Late packet  = %d\n", stVoipSessionStatistics.nLatePacket);
		printf("Silence speech  = %d\n", stVoipSessionStatistics.nSilenceSpeech);
	}
	else
	{
		printf("Usage error!\n");
		printf("To get RTP statistics: rtcp_statistic  chid  reset(0 or 1) \n");
		printf("To get Session statistics: rtcp_statistic  chid  sid(0 or 1)  reset(0 or 1) \n");

	}
	
	return 0;
}


