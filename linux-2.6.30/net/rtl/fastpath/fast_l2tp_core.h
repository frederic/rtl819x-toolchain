#ifndef FAST_L2TP_CORE_H
#define FAST_L2TP_CORE_H

#include <linux/skbuff.h>

int __init fast_l2tp_init(void);
void __exit fast_l2tp_exit(void);
void event_ppp_dev_down(const char *name);
int fast_l2tp_to_wan(void *skb);
void fast_l2tp_rx(void *skb);
void l2tp_tx_id(void *skb);
void (*l2tp_tx_id_hook)(void *skb);
unsigned long get_fast_l2tp_lastxmit(void);
int fast_l2tp_fw;
#endif

