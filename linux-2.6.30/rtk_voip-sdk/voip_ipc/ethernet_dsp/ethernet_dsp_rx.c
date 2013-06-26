#include <linux/string.h>
#include "rtk_voip.h"
#include "voip_types.h"
#include "voip_init.h"
#include "voip_ipc.h"

#include "ipc_arch_rx.h"

#ifdef CONFIG_RTK_VOIP_ETHERNET_DSP_IS_DSP
void ethernet_dsp_update_source_MAC( const ipc_ctrl_pkt_t *ipc_pkt )
{
	// Update Host sourc MAC addr
	static unsigned char update_host_mac = 1;
	extern unsigned char dec_mac_dsp2host[6];
	if (update_host_mac)
	{
		int i=0;
		
		//memcpy((unsigned char*)dec_mac_dsp2host, (unsigned char*)(eth_pkt+ETH_SRC_MAC_SHIFT), 6);
		memcpy((unsigned char*)dec_mac_dsp2host, ipc_pkt ->srcMAC, 6);
		update_host_mac = 0;
		printk("====> Update Host MAC addr: ");
		for (i=0; i<6; i++)
			printk("%x: ", dec_mac_dsp2host[i]);
		printk("\n");
	}
}
#endif

static void ethernet_dsp_L2_pkt_rx(unsigned char* eth_pkt, unsigned long size)
{
	// in some case, it will borrow to complete zero copy 
	ipc_pkt_rx_entry( ( ipc_ctrl_pkt_t * )eth_pkt, size );
}

static int __init ethernet_dsp_rx_init( void )
{
	extern void ( *voip_dsp_L2_pkt_rx_trap )( unsigned char* eth_pkt, unsigned long size );
	
	voip_dsp_L2_pkt_rx_trap = ethernet_dsp_L2_pkt_rx;
	
	return 0;
}

voip_initcall( ethernet_dsp_rx_init );

