#include <linux/proc_fs.h>
#include <linux/init.h>
#include <linux/module.h>
#include "rtk_voip.h"
#include "voip_types.h"
#include "voip_init.h"

#ifdef IPC_ARCH_DEBUG_HOST
extern unsigned int host_ctrl_tx_cnt[];
extern unsigned int host_rtp_rtcp_tx_cnt[];
extern unsigned int host_t38_tx_cnt[];
extern unsigned int host_ack_tx_cnt[];

extern unsigned int host_resp_rx_cnt[];
extern unsigned int host_rtp_rtcp_rx_cnt[];
extern unsigned int host_t38_rx_cnt[];
extern unsigned int host_event_rx_cnt[];
#endif

#ifdef IPC_ARCH_DEBUG_DSP
extern unsigned int dsp_ctrl_rx_cnt;
extern unsigned int dsp_rtp_rtcp_rx_cnt;
extern unsigned int dsp_t38_rx_cnt;
extern unsigned int dsp_ack_rx_cnt;

extern unsigned int dsp_resp_tx_cnt;
extern unsigned int dsp_rtp_rtcp_tx_cnt;
extern unsigned int dsp_event_tx_cnt;
extern unsigned int dsp_t38_tx_cnt;
#endif

int get_ethernet_dsp_info( char *buf, char **start, off_t off, int count, int *eof, void *data )
{
#ifdef IPC_ARCH_DEBUG_HOST
	int i;
#endif

#ifdef IPC_ARCH_DEBUG_HOST
	printk("\n* Host Pakcet Tx Info:\n");
	for (i=0; i<CONFIG_RTK_VOIP_DSP_DEVICE_NR; i++)
	{
		printk("** To DSP#%d:\n", i);
		printk("  - Ctrl tx cnt: %d\n", host_ctrl_tx_cnt[i]);
		printk("  - Rtp/Rtcp tx cnt: %d\n", host_rtp_rtcp_tx_cnt[i]);
		printk("  - T38 tx cnt: %d\n", host_t38_tx_cnt[i]);
		printk("  - Ack Tx cnt: %d\n", host_ack_tx_cnt[i]);
	}
	
	printk("\n* Host Pakcet Rx Info:\n");
	for (i=0; i<CONFIG_RTK_VOIP_DSP_DEVICE_NR; i++)
	{
		printk("** From DSP#%d:\n", i);
		printk("  - Resp rx cnt: %d\n", host_resp_rx_cnt[i]);
		printk("  - Rtp/Rtcp rx cnt: %d\n", host_rtp_rtcp_rx_cnt[i]);
		printk("  - T38 rx cnt: %d\n", host_t38_rx_cnt[i]);
		printk("  - Event rx cnt: %d\n", host_event_rx_cnt[i]);
	}
#endif

#ifdef IPC_ARCH_DEBUG_DSP
	printk("\n* DSP Packet Rx Info:\n");
	printk(" - Ctrl rx cnt: %d\n", dsp_ctrl_rx_cnt);
	printk(" - Rtp/Rtcp  rx cnt: %d\n", dsp_rtp_rtcp_rx_cnt);
	printk(" - T38  rx cnt: %d\n", dsp_t38_rx_cnt);
	printk(" - Ack rx cnt: %d\n", dsp_ack_rx_cnt);
	
	printk("\n* DSP Packet Tx Info:\n");
	printk(" - Resp tx cnt: %d\n", dsp_resp_tx_cnt);
	printk(" - Rtp/Rtcp  tx cnt: %d\n", dsp_rtp_rtcp_tx_cnt);
	printk(" - T38  tx cnt: %d\n", dsp_t38_tx_cnt);
	printk(" - Event tx cnt: %d\n", dsp_event_tx_cnt);
#endif
	
	return -1;
}


int __init ethernet_dsp_info_init( void )
{
	create_proc_read_entry( "ethernet_dsp_info",0, NULL, get_ethernet_dsp_info, NULL );
	return  0;
}

void __exit ethernet_dsp_info_exit( void )
{
	remove_proc_entry( "ethernet_dsp_info", NULL );
}

#ifndef CONFIG_RTK_VOIP_MODULE
voip_initcall(ethernet_dsp_info_init);
voip_exitcall(ethernet_dsp_info_exit);
#endif

