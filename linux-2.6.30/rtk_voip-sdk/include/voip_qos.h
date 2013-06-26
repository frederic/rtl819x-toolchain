#ifndef __VOIP_QOS_H__
#define __VOIP_QOS_H__

#ifdef CONFIG_RTK_VOIP_QOS

#ifndef BIT		// BIT may be defined in kernel's header file  
#define BIT(x)  (1<<(x))
#endif

#define VOIP_QOS_INTERRUPT_RX_TASKLET BIT(0)

#define VOIP_QOS_RX_HW_HIGH_QUEUE BIT(4)
#define VOIP_QOS_RX_SW_HIGH_QUEUE BIT(5)

#define VOIP_QOS_TX_HW_HIGH_QUEUE BIT(8)
#define VOIP_QOS_TX_SW_HIGH_QUEUE BIT(9)
#define VOIP_QOS_TX_DISABLE_FC BIT(10)

#define VOIP_QOS_LOCAL_SESSION_RESERVE BIT(12)

#define VOIP_QOS_DROP_BIG_TRAFFIC BIT(16)

#endif

#endif /* __VOIP_QOS_H__ */

