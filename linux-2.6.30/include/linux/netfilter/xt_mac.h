#ifndef _XT_MAC_H
#define _XT_MAC_H

#define MAC_SRC		0x01	/* Match source MAC address */
#define MAC_DST		0x02	/* Match destination MAC address */
#if defined(CONFIG_RTL_MAC_FILTER_CARE_INPORT)
#define INPORT_FLAG    0x04
#endif
#define MAC_SRC_INV		0x10	/* Negate the condition */
#define MAC_DST_INV		0x20	/* Negate the condition */

struct xt_mac{
    unsigned char macaddr[ETH_ALEN];
};

struct xt_mac_info {
   struct xt_mac srcaddr;
   struct xt_mac dstaddr;
//    int invert;
    u_int8_t flags;
#if defined(CONFIG_RTL_MAC_FILTER_CARE_INPORT)
    u_int8_t inPortMask;
#endif
};

#endif /*_XT_MAC_H*/
