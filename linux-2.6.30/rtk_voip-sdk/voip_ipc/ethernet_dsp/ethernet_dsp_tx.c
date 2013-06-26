#include <linux/string.h>
#include <linux/version.h>
#include <linux/netdevice.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include "rtk_voip.h"
#include "voip_types.h"
#include "voip_init.h"
#include "voip_ipc.h"

//dsp to host des MAC addr
unsigned char dec_mac_dsp2host[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
//host to dsp des MAC addr
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
static const unsigned char dec_mac_host2dsp0[6] = {0x02, 0xE0, 0x4C, 0x89, 0x72, 0xB0}; // dsp-0
static const unsigned char dec_mac_host2dsp1[6] = {0x02, 0xE0, 0x4C, 0x89, 0x72, 0xB1}; // dsp-1
static const unsigned char dec_mac_host2dsp2[6] = {0x02, 0xE0, 0x4C, 0x89, 0x72, 0xB2}; // dsp-2
static const unsigned char dec_mac_host2dsp3[6] = {0x02, 0xE0, 0x4C, 0x89, 0x72, 0xB3}; // dsp-3
#endif
/* Note: Should be identical to the DSP MAC addr setting: 
	refer to AP/pana/hcd/hcd.c function ethernet_dsp_set_mac()
*/

static const unsigned short eth_type = 0x8899;
static struct net_device *eth0_dev;

void ethernet_dsp_start_xmit( void *ipc_priv )
{
	struct sk_buff * const skb = ipc_priv;
	
#if 0
	skb->dev->hard_start_xmit(skb, eth0_dev);
#else
	skb->priority = 7;
	dev_queue_xmit(skb);
#endif
}

void ethernet_dsp_fill_tx_frame_header( ipc_ctrl_pkt_t *ipc_pkt,
								const TstTxPktCtrl* txCtrl )
{
	/* setup ethernet header (DA, SA, type) and payload of the skb */
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	//memcpy(ipc_pkt ->dstMAC, dec_mac_host, 6);
	if (txCtrl->dsp_cpuid == 0)
		memcpy(ipc_pkt ->dstMAC, dec_mac_host2dsp0, 6);
	else if (txCtrl->dsp_cpuid == 1)
		memcpy(ipc_pkt ->dstMAC, dec_mac_host2dsp1, 6);		/* destination eth addr */
	else if (txCtrl->dsp_cpuid == 2)
		memcpy(ipc_pkt ->dstMAC, dec_mac_host2dsp2, 6);
	else if (txCtrl->dsp_cpuid == 3)
		memcpy(ipc_pkt ->dstMAC, dec_mac_host2dsp3, 6);
	else
	{
		//memcpy(skb->data, dec_mac_host2dsp0, 6);
		//PRINT_R("No set for dec_mac_addr, cmd= %d\n", category);
		PRINT_R("No set for dec_mac_addr\n");
	}
#elif defined CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP
	memcpy(ipc_pkt ->dstMAC, dec_mac_dsp2host, 6);
#endif
	memcpy(ipc_pkt ->srcMAC, eth0_dev->dev_addr, 6); 				/* source ether addr */
	
	ipc_pkt ->ethType = htons(eth_type);	/* packet type ID field */
}

ipc_ctrl_pkt_t *ethernet_dsp_tx_allocate( unsigned int *pkt_len, 
											void **ipc_priv )
{
	struct sk_buff *skb;
	unsigned int skb_len = *pkt_len;
	
	if (eth0_dev == NULL)
		return NULL;
	
	if (skb_len < 64) {
		skb_len = *pkt_len = 64;
	}
		
	skb = alloc_skb(skb_len , GFP_ATOMIC);
	if (skb == NULL)
	{
		printk("ethernet_dsp_tx_allocate :skb_alloc return NULL.\n");
		return NULL;
	}
	
	skb->len = skb_len;
	skb->dev = eth0_dev;
	memset(skb->data, 0, skb->len);
	
	*ipc_priv = skb;
	
	return ( ipc_ctrl_pkt_t * )skb ->data;
}

static int __init rtk_voip_ethernet_dsp_init(void)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24))
	extern struct net init_net;//linux global variable
	eth0_dev = (struct net_device *)__dev_get_by_name(&init_net, "eth0");
#else
	eth0_dev = (struct net_device *)__dev_get_by_name("eth0");
#endif
	
	if (eth0_dev != NULL)
		PRINT_Y("Get the eth0 dev successfully.\n");
	else
		PRINT_Y("Get the eth0 dev NULL.\n");
		
	return 0;
}

static void __exit rtk_voip_ethernet_dsp_exit(void)
{
	eth0_dev = NULL;
}

voip_initcall(rtk_voip_ethernet_dsp_init);
voip_exitcall(rtk_voip_ethernet_dsp_exit);


