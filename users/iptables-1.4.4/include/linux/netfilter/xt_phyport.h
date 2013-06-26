#ifndef _XT_PHYPORT_H
#define _XT_PHYPORT_H

#define PORT_SRC		0x01	/* Match source phy port number*/
#define PORT_DST		0x02	/* Match destination port number */
#define PORT_SRC_INV	0x10	/* Negate the condition */
#define PORT_DST_INV	0x20	/* Negate the condition */

struct xt_phyport_info {
   u_int8_t srcport;
   u_int8_t dstport;
    u_int8_t flags;
};

#endif /*_XT_PHYPORT_H*/

