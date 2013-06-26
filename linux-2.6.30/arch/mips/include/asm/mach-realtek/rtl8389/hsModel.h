#ifndef _HS_MODEL_H_
#define _HS_MODEL_H_

#include "rtl_types.h"
/* HSB (Header Stamp Before):
 * Software-friendly structure definition */
typedef struct hsb_param_s
{
	uint32 cfi:1;       /* CFI */
	uint32 patternMatch:2;   /* MAC Ingress pattern match key. */
	uint32 flowLabel:20;      /* IPv6 Flow Lable */
	uint32 dstPort:16;      /* TCP/UDP Destination Port */
	uint32 srcPort:16;     /* TCP/UDP Source Port */
	uint32 tcpFlags:8;   /* TCP Flags */
	uint32 ipProto:8;      /* IP Protocol */
	uint32 svid:12;     /* SVID */
	uint32 spri:3;      /* S Priority */
	uint32 rxDrop:1;     /* ??? */
	uint32 cpuTagif:1;     /* If CPU tagged */
	uint32 cpuIntPri:3;      /* The Internal Priority in CPU tag, used for HSA */
	uint32 cpuPortMask:28;     /* The Port mask in CPU tag, used for HSA */
	uint32 ethType:16;     /* EtherType or Length */
	uint32 ipv6MLD:1;  /* IPv6 MLD control packet */
	uint32 cpri:3;  	/* C Priority */
	uint32 cvid:12;        /* CVID */
	uint32 dip:32;        /* Destination IP Address */
	uint32 sip:32;		/* Source IP Address */
	uint8 dmac[6];      /* Destination MAC */
	uint8 smac[6];   /* Source MAC */
	uint32 tos:8;     /* TOS */
	uint32 cpuasp:1;     /* CPU Tag Assigns Internal Priority */
	uint32 cpuasdpm:1;    /* CPU Tag Assigns Destination Port Mask */
	uint32 cpuaspRmk:1;    /* CPU Tag Allows MAC Tx Remarking */
	uint32 ipv6:1;   	/* IPv6 packet (ver=6) */
	uint32 ipv4:1;    	/* IPv4 packet (ver=4) and length >= 20 bytes */
	uint32 pppoe:1;	/* PPPoE packet */
	uint32 stagif:1;     /* If SVID Tagged */
	uint32 ctagif:1;    /* If CVID Tagged (ethertype==0x8100) */
	uint32 frameType:2;  /* 0:Ethrnet;1:RFC1042;2-LLC_Other ??? */
	uint32 pktLen:14;  /* Packet Length (included CRC) */
	uint32 l4csok:1;  /* L4 Checksum OK */
	uint32 l3csok:1;  /* L3 Checksum OK */
	uint32 endPage:11;  /* the last page address of packet */
	uint32 startPage:11;  /* the first page address of packet  */
	uint32 startBank:3;  /* the first bank of packet */
	uint32 spa:5;  /* Source Port Address (physical) */
}hsb_param_t;

/* HSA (Header Stamp After):
 * Software-friendly structure definition */
typedef struct hsa_param_s
{
	uint32 newsvid:12;     /* New SVID */
	uint32 newcvid:12;     /* New CVID*/
	uint32 cpuTagif:1;     /* If CPU tagged */
	uint32 dpCnt:5;   /* Destination Port Count */
	uint32 rvid:12;   /* Relay VID */
	uint32 reason:16;      /* CPU reason */
	uint32 intPri:3;      /* Internal Priority */
	uint32 dpMask:29;     /* Destination Port Mask */
	uint32 mir1dpa:5;    /* Physical Port Address of the egress mirrored port 1 (30-ingress matched, 31-not hit) */
	uint32 mir0dpa:5;	/* Physical Port Address of the egress mirrored port 0 (30-ingress matched, 31-not hit) */
	uint32 cpuaspRmk:1;     /* CPU Tag Allows MAC Tx Remarking */
	uint32 ipv6:1;	    /* IPv6 packet (ver=6) */
	uint32 ipv4:1;	    /* IPv4 packet (ver=4)*/
	uint32 pppoe:1;  /* PPPoE packet */
	uint32 stagif:1;    /* If SVID Tagged */
	uint32 ctagif:1;     /* If CVID Tagged */
	uint32 frameType:2;   /* 0:Ethrnet;1:RFC1042;2-LLC_Other ??? */
	uint32 pktLen:14;   /* Packet Length (included CRC) */
	uint32 l4csok:1;      /* L4 Checksum OK */
	uint32 l3csok:1;  /* L3 Checksum OK */
	uint32 endPage:11;  /* the last page address of packet */
	uint32 startPage:11;     /* the first page address of packet  */
	uint32 startBank:3;   /* the first bank of packet */
	uint32 spa:5;   /* Source Port Address (physical) */
}hsa_param_t;


/* RAW HSB: Raw structure to access ASIC.
 * The structure is directly mapped to ASIC, however, it is not friendly for software. */
typedef struct hsb_table_s
{
#ifdef _BIG_ENDIAN
	uint32 sel_hsb:1;        /* W0[31] */
	uint32 valid_hsb:1;      /* W0[30] */
	uint32 reserved:7;       /* W0[29:23] */
	uint32 cfi:1;            /* W0[22] */
	uint32 patternmatch:2;   /* W0[21:20] */	
	uint32 flowlabel:20;     /* W0[19:0] */	
#else
	uint32 flowlabel:20;     /* W0[19:0] */	
	uint32 patternmatch:2;   /* W0[21:20] */	
	uint32 cfi:1;            /* W0[22] */
	uint32 resevered:7;      /* W0[29:23] */
	uint32 valid_hsb:1;      /* W0[30] */
	uint32 sel_hsb:1;        /* W0[31] */
#endif

#ifdef _BIG_ENDIAN
	uint32 dstport:16;       /* W1[31:16] */
	uint32 srcport:16;       /* W1[15:0] */
#else
	uint32 srcport:16;       /* W1[15:0] */
	uint32 dstport:16;       /* W1[31:16] */
#endif

#ifdef _BIG_ENDIAN
	uint32 tcpflags:8;       /* W2[31:24] */
	uint32 ipproto:8;        /* W2[23:16] */
	uint32 svid:12;          /* W2[15:4] */
	uint32 spri:3;           /* W2[3:1] */	
	uint32 rxdrop:1;         /* W2[0] */		
#else
	uint32 rxdrop:1;         /* W2[0] */		
	uint32 spri:3;           /* W2[3:1] */	
	uint32 svid:12;          /* W2[15:4] */
	uint32 ipproto:8;        /* W2[23:16] */
	uint32 tcpflags:8;       /* W2[31:24] */
#endif

#ifdef _BIG_ENDIAN
	uint32 cputagif:1;       /* W3[31] */
	uint32 cpuintpri:3;      /* W3[30:28] */
	uint32 cpuportmask:28;   /* W3[27:0] */
#else
	uint32 cpuportmask:28;   /* W3[27:0] */
	uint32 cpuintpri:3;      /* W3[30:28] */
	uint32 cputagif:1;       /* W3[31] */
#endif

#ifdef _BIG_ENDIAN
	uint32 ethertype:16;     /* W4[31:16] */
	uint32 ipv6mld:1;        /* W4[15] */
	uint32 cpri:3;           /* W4[14:12] */
	uint32 cvid:12;          /* W4[11:0] */	
#else
	uint32 cvid:12;          /* W4[11:0] */	
	uint32 cpri:3;           /* W4[14:12] */
	uint32 ipv6mld:1;        /* W4[15] */
	uint32 ethertype:16;     /* W4[31:16] */
#endif

 	uint32 dip;              /* W5[31:0] */ 
 	uint32 sip;              /* W6[31:0] */
 	uint32 dmac47_16;        /* W7[31:0] */
 
#ifdef _BIG_ENDIAN
	uint32 dmac15_0:16;      /* W8[31:16] */
	uint32 smac47_32:16;     /* W8[15:0] */
#else
	uint32 smac47_32:16;     /* W8[15:0] */
	uint32 dmac15_0:16;      /* W8[31:16] */
#endif

 	uint32 smac31_0;         /* W9[31:0] */
 
#ifdef _BIG_ENDIAN
	uint32 tos:8;            /* W10[31:24] */
	uint32 cpuasp:1;        /* W10[23] */
	uint32 cpuasdpm:1;       /* W10[22] */
	uint32 cpuasprmk:1;     /* W10[21] */
	uint32 ipv6:1;           /* W10[20] */
	uint32 ipv4:1;           /* W10[19] */
	uint32 pppoe:1;          /* W10[18] */	
	uint32 stagif:1;         /* W10[17] */	
	uint32 ctagif:1;         /* W10[16] */	
	uint32 frametype:2;      /* W10[15:14] */		
	uint32 pktlen:14;        /* W10[13:0] */		
#else
	uint32 pktlen:14;        /* W10[13:0] */		
	uint32 frametype:2;      /* W10[15:14] */		
	uint32 ctagif:1;         /* W10[16] */	
	uint32 stagif:1;         /* W10[17] */	
	uint32 pppoe:1;          /* W10[18] */	
	uint32 ipv4:1;           /* W10[19] */
	uint32 ipv6:1;           /* W10[20] */
	uint32 cpuasprmk:1;     /* W10[21] */
	uint32 cpuasdpm:1;       /* W10[22] */
	uint32 cpuasp:1;        /* W10[23] */
	uint32 tos:8;            /* W10[31:24] */
#endif

#ifdef _BIG_ENDIAN
	uint32 l4csok:1;         /* W11[31] */
	uint32 l3csok:1;         /* W11[30] */
	uint32 endpage:11;       /* W11[29:19] */
	uint32 startpage:11;     /* W11[18:8] */
	uint32 startbank:3;      /* W11[7:5] */
	uint32 spa:5;            /* W11[4:0] */	
#else
	uint32 spa:5;            /* W11[4:0] */	
	uint32 startbank:3;      /* W11[7:5] */
	uint32 startpage:11;     /* W11[18:8] */
	uint32 endpage:11;       /* W11[29:19] */
	uint32 l3csok:1;         /* W11[30] */
	uint32 l4csok:1;         /* W11[31] */
#endif
} hsb_table_t;

/* RAW HSA: Raw structure to access ASIC.
 * The structure is directly mapped to ASIC, however, it is not friendly for software. */
typedef struct hsa_table_s
{
#ifdef _BIG_ENDIAN
	uint32 hsa_busy:1;       /* W0[31] */
	uint32 reserved:5;       /* W0[30:26] */
	uint32 newsvid:12;      /* W0[25:14] */
	uint32 newcvid:12;        /* W0[13:2] */
	uint32 cputagif:1;       /* W0[1] */ 
	uint32 dpcnt4_4:1;       /* W0[0] */ 	
#else
	uint32 dpcnt4_4:1;       /* W0[0] */ 	
	uint32 cputagif:1;       /* W0[1] */ 
	uint32 newcvid:12;       /* W0[13:2] */
	uint32 newsvid:12;       /* W0[25:14] */
	uint32 reserved:5;       /* W0[30:26] */
	uint32 hsa_busy:1;       /* W0[31] */
#endif

#ifdef _BIG_ENDIAN
 	uint32 dpcnt3_0:4;       /* W1[31:28] */  
 	uint32 rvid:12;          /* W1[27:16] */  
 	uint32 reason:16;        /* W1[15:0] */  	
#else
 	uint32 reason:16;        /* W1[15:0] */  	
 	uint32 rvid:12;          /* W1[27:16] */  
 	uint32 dpcnt3_0:4;       /* W1[31:28] */  
#endif

#ifdef _BIG_ENDIAN
 	uint32 intpri:3;         /* W2[31:29] */  
 	uint32 dpmask:29;        /* W2[28:0] */  
#else
 	uint32 dpmask:29;        /* W2[28:0] */  
 	uint32 intpri:3;         /* W2[31:29] */  
#endif

#ifdef _BIG_ENDIAN
	uint32 mir1dpa:5;        /* W3[31:27] */
	uint32 mir0dpa:5;        /* W3[26:22] */
	uint32 cpuasdprmk:1;     /* W3[21] */
	uint32 ipv6:1;           /* W3[20] */
	uint32 ipv4:1;           /* W3[19] */
	uint32 pppoe:1;          /* W3[18] */
	uint32 stagif:1;         /* W3[17] */
	uint32 ctagif:1;         /* W3[16] */
	uint32 frametype:2;      /* W3[15:14] */	
	uint32 pktlen:14;        /* W3[13:0] */	 
#else
	uint32 pktlen:14;        /* W3[13:0] */	 
	uint32 frametype:2;      /* W3[15:14] */	
	uint32 ctagif:1;         /* W3[16] */
	uint32 stagif:1;         /* W3[17] */
	uint32 pppoe:1;          /* W3[18] */
	uint32 ipv4:1;           /* W3[19] */
	uint32 ipv6:1;           /* W3[20] */
	uint32 cpuasdprmk:1;     /* W3[21] */
	uint32 mir0dpa:5;        /* W3[26:22] */
	uint32 mir1dpa:5;        /* W3[31:27] */
#endif

#ifdef _BIG_ENDIAN
	uint32 l4csok:1;         /* W4[31] */
	uint32 l3csok:1;         /* W4[30] */ 
	uint32 endpage:11;       /* W4[29:19] */ 
	uint32 startpage:11;     /* W4[18:8] */ 
	uint32 startbank:3;      /* W4[7:5] */ 
	uint32 spa:5;            /* W4[4:0] */ 	
#else
	uint32 spa:5;            /* W4[4:0] */ 	
	uint32 startbank:3;      /* W4[7:5] */ 
	uint32 startpage:11;     /* W4[18:8] */ 
	uint32 endpage:11;       /* W4[29:19] */ 
	uint32 l3csok:1;         /* W4[30] */ 
	uint32 l4csok:1;         /* W4[31] */
#endif
} hsa_table_t;



extern int32 modelGetHsb(hsb_param_t* hsb);
extern int32 modelSetHsb(hsb_param_t* hsb);
extern int32 modelGetHsa(hsa_param_t* hsa);
extern int32 modelSetHsa(hsa_param_t* hsa);

#endif/*_HS_MODEL_H_*/

