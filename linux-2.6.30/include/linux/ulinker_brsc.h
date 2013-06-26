#ifndef __LINUX_USB_ULINKER_BRSC_H
#define __LINUX_USB_ULINKER_BRSC_H

#define FROM_USB 0
#define FROM_ETH 1
#define FROM_WLAN 2

struct brsc_cache 
{
	struct net_device * dev;
	unsigned char addr[6];	
};

struct brsc_counter_s {
	unsigned long tx_in;
	unsigned long tx_eth_sc;
	unsigned long tx_wlan_sc;
	unsigned long tx_ok;
	
	unsigned long tx_unknown_cdc_filter;
	unsigned long tx_req_full;
	unsigned long tx_realloc_header_fail;
	unsigned long tx_skb_expand_full;
	unsigned long tx_ep_queue_err;
	unsigned long tx_req_full_recover;
	
	unsigned long rx_alloc_fail;
	unsigned long rx_ep_queue_fail;
	unsigned long rx_ep_queue_fail2;
	unsigned long rx_ep_queue_ok;
	
	unsigned long rx_complete_in;
	unsigned long rx_complete_err;
	unsigned long rx_complete_sc;
	unsigned long rx_complete_normal;
	unsigned long rx_complete_connreset;
	unsigned long rx_complete_shutdown;
	unsigned long rx_complete_connabort;
	unsigned long rx_complete_overflow;
	unsigned long rx_complete_other_err;

	unsigned long otg_status_fail;
	unsigned long otg_inepint;
	unsigned long otg_outepintr;
};

extern struct brsc_counter_s brsc_counter;
extern struct brsc_cache cached_usb;
extern struct net_device *brsc_get_cached_dev(char from_usb, unsigned char *da);
extern void brsc_cache_dev(int from, struct net_device *dev, unsigned char *da);
extern struct net_device *pre_get_shortcut_dev(struct sk_buff *skb);

#if 0
  #define BDBG_BRSC(format, arg...) 	  \
	  panic_printk(format , ## arg)
#elif 0
  #define BDBG_BRSC(format, arg...)		\
	  printk(format , ## arg)
#else
  #define BDBG_BRSC(format, arg...)
#endif


#if 0
  #define BDBG_TASKLET(format, arg...) 	  \
	  panic_printk(format , ## arg)
#elif 0
  #define BDBG_TASKLET(format, arg...)		\
	  printk(format , ## arg)
#else
  #define BDBG_TASKLET(format, arg...)
#endif

#if defined(CONFIG_RTL_ULINKER_BRSC)
#define ULINKER_BRSC_RECOVER_TX_REQ 1
#else
#define ULINKER_BRSC_RECOVER_TX_REQ 0
#endif


#define ULINKER_BRSC_COUNTER 0
#if ULINKER_BRSC_COUNTER
	#define BRSC_COUNTER_UPDATE(X) \
		do { brsc_counter.X++; } while(0);
#else
	#define BRSC_COUNTER_UPDATE(x)
#endif

#if 0
  #define BDBG_GADGET_MODE_SWITCH(format, arg...) 	  \
	  printk(format , ## arg)
#else
  #define BDBG_GADGET_MODE_SWITCH(format, arg...)
#endif


#endif	/* __LINUX_USB_ULINKER_BRSC_H */
