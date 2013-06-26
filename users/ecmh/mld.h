
// Wrappers so we don't have to change the copied stuff ;)
#define __u8 uint8_t
#define __u16 uint16_t

// Determine Endianness
#if BYTE_ORDER == LITTLE_ENDIAN
	/* 1234 machines */
	#define __LITTLE_ENDIAN_BITFIELD 1
#elif BYTE_ORDER == BIG_ENDIAN
	/* 4321 machines */
	#define __BIG_ENDIAN_BITFIELD 1
# define WORDS_BIGENDIAN 1
#elif BYTE_ORDER == PDP_ENDIAN
	/* 3412 machines */
#error PDP endianness not supported yet!
#else
#error unknown endianness!
#endif

// Per RFC
struct mld1 {
        __u8		type;
        __u8		code;
        __u16		csum;
        __u16		mrc;
        __u16		resv1;
        struct in6_addr	mca;
};

#define ICMP6_V2_MEMBERSHIP_REPORT	206	/* MLDv2 Report */
#define MLDv2_LISTENER_REPORT		206	/* MLDv2 Report */

/* From linux/net/ipv6/mcast.c */

/*
 *  These header formats should be in a separate include file, but icmpv6.h
 *  doesn't have in6_addr defined in all cases, there is no __u128, and no
 *  other files reference these.
 *
 *                      +-DLS 4/14/03
 */

/* Multicast Listener Discovery version 2 headers */

struct mld2_grec {
        __u8            grec_type;
        __u8            grec_auxwords;
        __u16           grec_nsrcs;
        struct in6_addr grec_mca;
        struct in6_addr grec_src[0];
};

struct mld2_report {
        __u8    type;
        __u8    resv1;
        __u16   csum;
        __u16   resv2;
        __u16   ngrec;
        struct mld2_grec grec[0];
};

struct mld2_query {
        __u8 type;
        __u8 code;
        __u16 csum;
        __u16 mrc;
        __u16 resv1;
        struct in6_addr mca;
#if defined(__LITTLE_ENDIAN_BITFIELD)
        __u8 qrv:3,
             suppress:1,
             resv2:4;
#elif defined(__BIG_ENDIAN_BITFIELD)
        __u8 resv2:4,
             suppress:1,
             qrv:3;
#else
#error "Please fix <asm/byteorder.h>"
#endif
        __u8 qqic;
        __u16 nsrcs;
        struct in6_addr srcs[0];
};

#define IGMP6_UNSOLICITED_IVAL  (10*HZ)
#define MLD_QRV_DEFAULT         2

#define MLD_V1_SEEN(idev) ((idev)->mc_v1_seen && \
                time_before(jiffies, (idev)->mc_v1_seen))

#define MLDV2_MASK(value, nb) ((nb)>=32 ? (value) : ((1<<(nb))-1) & (value))
#define MLDV2_EXP(thresh, nbmant, nbexp, value) \
        ((value) < (thresh) ? (value) : \
        ((MLDV2_MASK(value, nbmant) | (1<<(nbmant+nbexp))) << \
        (MLDV2_MASK((value) >> (nbmant), nbexp) + (nbexp))))

#define MLDV2_QQIC(value) MLDV2_EXP(0x80, 4, 3, value)
#define MLDV2_MRC(value) MLDV2_EXP(0x8000, 12, 3, value)
