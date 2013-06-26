/*
 * ppp_mppe_mppc.c - MPPC/MPPE "compressor/decompressor" module.
 *
 * Copyright (c) 1994 ÁpáMagosái <mag@bunuel.tii.matav.hu>
 * Copyright (c) 1999 Tim Hockin, Cobalt Networks Inc. <thockin@cobaltnet.com>
 * Copyright (c) 2002-2004 Jan Dubiec <jdx@slackware.pl>
 * 
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, provided that the above copyright
 * notice appears in all copies. This software is provided without any
 * warranty, express or implied.
 *
 * The code is based on MPPE kernel module written by ÁpáMagosái and
 * Tim Hockin which can be found on http://planetmirror.com/pub/mppe/.
 * I have added MPPC and 56 bit session keys support in MPPE.
 *
 * WARNING! Although this is open source code, its usage in some countries
 * (in particular in the USA) may violate Stac Inc. patent for MPPC.
 *
 *  ==FILEVERSION 20040815==
 *
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <asm/scatterlist.h>
#include <linux/vmalloc.h>
#include <linux/crypto.h>

#include <linux/ppp_defs.h>
#include <linux/ppp-comp.h>

/*
 * SHA1 definitions and prototypes
 */
typedef struct {
    u32 state[5];
    u32 count[2];
    u8 buffer[64];
} SHA1_CTX;

#define SHA1_SIGNATURE_SIZE 20

static void SHA1_Init(SHA1_CTX *);
static void SHA1_Update(SHA1_CTX *, const u8 *, u32);
static void SHA1_Final(u8[SHA1_SIGNATURE_SIZE], SHA1_CTX *);

/*
 * State for a mppc/mppe "(de)compressor".
 */
struct ppp_mppe_state {
    struct crypto_tfm *arc4_tfm;
    u8		master_key[MPPE_MAX_KEY_LEN];
    u8		session_key[MPPE_MAX_KEY_LEN];
    u8		mppc;		/* do we use compression (MPPC)? */
    u8		mppe;		/* do we use encryption (MPPE)? */
    u8		keylen;		/* key length in bytes */
    u8		bitkeylen;	/* key length in bits */
    u16		ccount;		/* coherency counter */
    u16		bits;		/* MPPC/MPPE control bits */
    u8		stateless;	/* do we use stateless mode? */
    u8		nextflushed;	/* set A bit in the next outgoing packet;
				   used only by compressor*/
    u8		flushexpected;	/* drop packets until A bit is received;
				   used only by decompressor*/
    u8		*hist;		/* MPPC history */
    u16		*hash;		/* Hash table; used only by compressor */
    u16		histptr;	/* history "cursor" */
    int		unit;
    int		debug;
    int		mru;
    struct compstat stats;
};

#define MPPE_HIST_LEN		8192	/* MPPC history size */
#define MPPE_MAX_CCOUNT		0x0FFF	/* max. coherency counter value */

#define MPPE_BIT_FLUSHED	0x80	/* bit A */
#define MPPE_BIT_RESET		0x40	/* bit B */
#define MPPE_BIT_COMP		0x20	/* bit C */
#define MPPE_BIT_ENCRYPTED	0x10	/* bit D */

#define MPPE_SALT0		0xD1	/* values used in MPPE key derivation */
#define MPPE_SALT1		0x26	/* according to RFC3079 */
#define MPPE_SALT2		0x9E

#define MPPE_CCOUNT(x)		((((x)[4] & 0x0f) << 8) + (x)[5])
#define MPPE_BITS(x)		((x)[4] & 0xf0)
#define MPPE_CTRLHI(x)		((((x)->ccount & 0xf00)>>8)|((x)->bits))
#define MPPE_CTRLLO(x)		((x)->ccount & 0xff)

static inline void
setup_sg(struct scatterlist *sg, const void  *address, unsigned int length)
{
    sg[0].page_link = virt_to_page(address);
    sg[0].offset = offset_in_page(address);
    sg[0].length = length;
}

static inline void
arc4_setkey(struct ppp_mppe_state *state, const unsigned char *key,
	    const unsigned int keylen)
{
    crypto_cipher_setkey(state->arc4_tfm, key, keylen);
}

static inline void
arc4_encrypt(struct ppp_mppe_state *state, const unsigned char *in,
	     const unsigned int len, unsigned char *out)
{
    int i;
    for (i = 0; i < len; i++)
    {
       crypto_cipher_encrypt_one(state->arc4_tfm, out+i, in+i);
    }
}

#define arc4_decrypt arc4_encrypt

/*
 * Key Derivation, from RFC 3078, RFC 3079.
 * Equivalent to Get_Key() for MS-CHAP as described in RFC 3079.
 */
static void
GetNewKeyFromSHA(struct ppp_mppe_state *state, unsigned char *MasterKey,
		 unsigned char *SessionKey, unsigned long SessionKeyLength,
		 unsigned char *InterimKey)
{
    /*Pads used in key derivation */
    static const unsigned char  SHAPad1[40] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    static const unsigned char  SHAPad2[40] = {
	0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2,
	0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2,
	0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2,
	0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2
    };

    unsigned char Digest[SHA1_SIGNATURE_SIZE];
    SHA1_CTX Context;

    SHA1_Init(&Context);
    SHA1_Update(&Context, MasterKey, SessionKeyLength);
    SHA1_Update(&Context, SHAPad1, sizeof(SHAPad1));
    SHA1_Update(&Context, SessionKey, SessionKeyLength);
    SHA1_Update(&Context, SHAPad2, sizeof(SHAPad2));
    SHA1_Final(Digest, &Context);

    memcpy(InterimKey, Digest, SessionKeyLength);
}

static void
mppe_change_key(struct ppp_mppe_state *state, int initialize)
{
    unsigned char InterimKey[MPPE_MAX_KEY_LEN];

    GetNewKeyFromSHA(state, state->master_key, state->session_key,
		     state->keylen, InterimKey);
    if (initialize) {
	memcpy(state->session_key, InterimKey, state->keylen);
    } else {
	arc4_setkey(state, InterimKey, state->keylen);
	arc4_encrypt(state, InterimKey, state->keylen, state->session_key);
    }
    if (state->keylen == 8) {
	if (state->bitkeylen == 40) {
	    state->session_key[0] = MPPE_SALT0;
	    state->session_key[1] = MPPE_SALT1;
	    state->session_key[2] = MPPE_SALT2;
	} else {
	    state->session_key[0] = MPPE_SALT0;
	}
    }
    arc4_setkey(state, state->session_key, state->keylen);
}

/* increase 12-bit coherency counter */
static inline void
mppe_increase_ccount(struct ppp_mppe_state *state)
{
    state->ccount = (state->ccount + 1) & MPPE_MAX_CCOUNT;
    if (state->mppe) {
	if (state->stateless) {
	    mppe_change_key(state, 0);
	    state->nextflushed = 1;
	} else {
	    if ((state->ccount & 0xff) == 0xff) {
		mppe_change_key(state, 0);
	    }
	}
    }
}

/* allocate space for a MPPE/MPPC (de)compressor.  */
/*   comp != 0 -> init compressor */
/*   comp = 0 -> init decompressor */
static void *
mppe_alloc(unsigned char *options, int opt_len, int comp)
{
    struct ppp_mppe_state *state;
    u8* fname;

    fname = comp ? "mppe_comp_alloc" : "mppe_decomp_alloc";

    /*  
 	* Hack warning - additionally to the standard MPPC/MPPE configuration
	* options, pppd passes to the (de)copressor 8 or 16 byte session key.
 	* Therefore options[1] contains MPPC/MPPE configuration option length
 	* (CILEN_MPPE = 6), but the real options length, depending on the key
	* length, is 6+8 or 6+16.
 	*/
    if (opt_len < CILEN_MPPE) {
	printk(KERN_WARNING "%s: wrong options length: %u\n", fname, opt_len);
	return NULL;
    }

    if (options[0] != CI_MPPE || options[1] != CILEN_MPPE ||
	(options[2] & ~MPPE_STATELESS) != 0 ||
	options[3] != 0 || options[4] != 0 ||
	(options[5] & ~(MPPE_128BIT|MPPE_56BIT|MPPE_40BIT|MPPE_MPPC)) != 0 ||
	(options[5] & (MPPE_128BIT|MPPE_56BIT|MPPE_40BIT|MPPE_MPPC)) == 0) {
	printk(KERN_WARNING "%s: options rejected: o[0]=%02x, o[1]=%02x, "
	       "o[2]=%02x, o[3]=%02x, o[4]=%02x, o[5]=%02x\n", fname, options[0],
	       options[1], options[2], options[3], options[4], options[5]);
	return NULL;
    }

    state = (struct ppp_mppe_state *)kmalloc(sizeof(*state), GFP_KERNEL);
    if (state == NULL) {
	printk(KERN_ERR "%s: cannot allocate space for %scompressor\n", fname,
	       comp ? "" : "de");
	return NULL;
    }
    memset(state, 0, sizeof(struct ppp_mppe_state));

    state->mppc = options[5] & MPPE_MPPC;	/* Do we use MPPC? */
    state->mppe = options[5] & (MPPE_128BIT | MPPE_56BIT |
	MPPE_40BIT);				/* Do we use MPPE? */

    if (state->mppc) {
	/* allocate MPPC history */
	state->hist = (u8*)vmalloc(2*MPPE_HIST_LEN*sizeof(u8));
	if (state->hist == NULL) {
	    kfree(state);
	    printk(KERN_ERR "%s: cannot allocate space for MPPC history\n",
		   fname);
	    return NULL;
	}

	/* allocate hashtable for MPPC compressor */
	if (comp) {
	    state->hash = (u16*)vmalloc(MPPE_HIST_LEN*sizeof(u16));
	    if (state->hash == NULL) {
		vfree(state->hist);
		kfree(state);
		printk(KERN_ERR "%s: cannot allocate space for MPPC history\n",
		       fname);
		return NULL;
	    }
	}
    }

    if (state->mppe) { /* specific for MPPE */
	/* Load ARC4 algorithm */
	state->arc4_tfm = crypto_alloc_base("arc4", 0, 0);
	if (state->arc4_tfm == NULL) {
	    vfree(state->hash);
	    vfree(state->hist);
	    kfree(state);
	    printk(KERN_ERR "%s: cannot load ARC4 module\n", fname);
	    return NULL;
	}

	memcpy(state->master_key, options+CILEN_MPPE, MPPE_MAX_KEY_LEN);
	memcpy(state->session_key, state->master_key, MPPE_MAX_KEY_LEN);
	/* initial key generation is done in mppe_init() */
    }

    return (void *) state;
}

static void *
mppe_comp_alloc(unsigned char *options, int opt_len)
{
    return mppe_alloc(options, opt_len, 1);
}

static void *
mppe_decomp_alloc(unsigned char *options, int opt_len)
{
    return mppe_alloc(options, opt_len, 0);
}

/* cleanup the (de)compressor */
static void
mppe_comp_free(void *arg)
{
    struct ppp_mppe_state *state = (struct ppp_mppe_state *) arg;

    if (state != NULL) {
	if (state->mppe) {
	    if (state->arc4_tfm != NULL)
		crypto_free_tfm(state->arc4_tfm);
	}
	if (state->hist != NULL)
	    vfree(state->hist);
	if (state->hash != NULL)
	    vfree(state->hash);
	kfree(state);
    }
}

/* init MPPC/MPPE (de)compresor */
/*   comp != 0 -> init compressor */
/*   comp = 0 -> init decompressor */
static int
mppe_init(void *arg, unsigned char *options, int opt_len, int unit,
	  int hdrlen, int mru, int debug, int comp)
{
    struct ppp_mppe_state *state = (struct ppp_mppe_state *) arg;
    u8* fname;

    fname = comp ? "mppe_comp_init" : "mppe_decomp_init";

    if (opt_len < CILEN_MPPE) {
	if (debug)
	    printk(KERN_WARNING "%s: wrong options length: %u\n",
		   fname, opt_len);
	return 0;
    }

    if (options[0] != CI_MPPE || options[1] != CILEN_MPPE ||
	(options[2] & ~MPPE_STATELESS) != 0 ||
	options[3] != 0 || options[4] != 0 ||
	(options[5] & ~(MPPE_56BIT|MPPE_128BIT|MPPE_40BIT|MPPE_MPPC)) != 0 ||
	(options[5] & (MPPE_56BIT|MPPE_128BIT|MPPE_40BIT|MPPE_MPPC)) == 0) {
	if (debug)
	    printk(KERN_WARNING "%s: options rejected: o[0]=%02x, o[1]=%02x, "
		   "o[2]=%02x, o[3]=%02x, o[4]=%02x, o[5]=%02x\n", fname,
		   options[0], options[1], options[2], options[3], options[4],
		   options[5]);
	return 0;
    }

    if ((options[5] & ~MPPE_MPPC) != MPPE_128BIT &&
	(options[5] & ~MPPE_MPPC) != MPPE_56BIT &&
	(options[5] & ~MPPE_MPPC) != MPPE_40BIT &&
	(options[5] & MPPE_MPPC) != MPPE_MPPC) {
	if (debug)
	    printk(KERN_WARNING "%s: don't know what to do: o[5]=%02x\n",
		   fname, options[5]);
	return 0;
    }

    state->mppc = options[5] & MPPE_MPPC;	/* Do we use MPPC? */
    state->mppe = options[5] & (MPPE_128BIT | MPPE_56BIT |
	MPPE_40BIT);				/* Do we use MPPE? */
    state->stateless = options[2] & MPPE_STATELESS; /* Do we use stateless mode? */

    switch (state->mppe) {
    case MPPE_40BIT:     /* 40 bit key */
	state->keylen = 8;
	state->bitkeylen = 40;
	break;
    case MPPE_56BIT:     /* 56 bit key */
	state->keylen = 8;
	state->bitkeylen = 56;
	break;
    case MPPE_128BIT:    /* 128 bit key */
	state->keylen = 16;
	state->bitkeylen = 128;
	break;
    default:
	state->keylen = 0;
	state->bitkeylen = 0;
    }

    state->ccount = MPPE_MAX_CCOUNT;
    state->bits = 0;
    state->unit  = unit;
    state->debug = debug;
    state->histptr = MPPE_HIST_LEN;
    if (state->mppc) {	/* reset history if MPPC was negotiated */
	memset(state->hist, 0, 2*MPPE_HIST_LEN*sizeof(u8));
    }

    if (state->mppe) { /* generate initial session keys */
	mppe_change_key(state, 1);
    }

    if (comp) { /* specific for compressor */
	state->nextflushed = 1;
    } else { /* specific for decompressor */
	state->mru = mru;
	state->flushexpected = 1;
    }

    return 1;
}

static int
mppe_comp_init(void *arg, unsigned char *options, int opt_len, int unit,
	       int hdrlen, int debug)
{
    return mppe_init(arg, options, opt_len, unit, hdrlen, 0, debug, 1);
}


static int
mppe_decomp_init(void *arg, unsigned char *options, int opt_len, int unit,
		 int hdrlen, int mru, int debug)
{
    return mppe_init(arg, options, opt_len, unit, hdrlen, mru, debug, 0);
}

static void
mppe_comp_reset(void *arg)
{
    struct ppp_mppe_state *state = (struct ppp_mppe_state *)arg;

    if (state->debug)
	printk(KERN_DEBUG "%s%d: resetting MPPC/MPPE compressor\n",
	       __FUNCTION__, state->unit);

    state->nextflushed = 1;
    if (state->mppe)
	arc4_setkey(state, state->session_key, state->keylen);
}

static void
mppe_decomp_reset(void *arg)
{
    /* When MPPC/MPPE is in use, we shouldn't receive any CCP Reset-Ack.
 *        But when we receive such a packet, we just ignore it. */
    return;
}

static void
mppe_stats(void *arg, struct compstat *stats)
{
    struct ppp_mppe_state *state = (struct ppp_mppe_state *)arg;

    *stats = state->stats;
}

/***************************/
/**** Compression stuff ****/
/***************************/
/* inserts 1 to 8 bits into the output buffer */
static inline void putbits8(u8 *buf, u32 val, const u32 n, u32 *i, u32 *l)
{
    buf += *i;
    if (*l >= n) {
	*l = (*l) - n;
	val <<= *l;
	*buf = *buf | (val & 0xff);
	if (*l == 0) {
	    *l = 8;
	    (*i)++;
	    *(++buf) = 0;
	}
    } else {
	(*i)++;
	*l = 8 - n + (*l);
	val <<= *l;
	*buf = *buf | ((val >> 8) & 0xff);
	*(++buf) = val & 0xff;
    }
}

/* inserts 9 to 16 bits into the output buffer */
static inline void putbits16(u8 *buf, u32 val, const u32 n, u32 *i, u32 *l)
{
    buf += *i;
    if (*l >= n - 8) {
	(*i)++;
	*l = 8 - n + (*l);
	val <<= *l;
	*buf = *buf | ((val >> 8) & 0xff);
	*(++buf) = val & 0xff;
	if (*l == 0) {
	    *l = 8;
	    (*i)++;
	    *(++buf) = 0;
	}
    } else {
	(*i)++; (*i)++;
	*l = 16 - n + (*l);
	val <<= *l;
	*buf = *buf | ((val >> 16) & 0xff);
	*(++buf) = (val >> 8) & 0xff;
	*(++buf) = val & 0xff;
    }
}

/* inserts 17 to 24 bits into the output buffer */
static inline void putbits24(u8 *buf, u32 val, const u32 n, u32 *i, u32 *l)
{
    buf += *i;
    if (*l >= n - 16) {
	(*i)++; (*i)++;
	*l = 16 - n + (*l);
	val <<= *l;
	*buf = *buf | ((val >> 16) & 0xff);
	*(++buf) = (val >> 8) & 0xff;
	*(++buf) = val & 0xff;
	if (*l == 0) {
	    *l = 8;
	    (*i)++;
	    *(++buf) = 0;
	}
    } else {
	(*i)++; (*i)++; (*i)++;
	*l = 24 - n + (*l);
	val <<= *l;
	*buf = *buf | ((val >> 24) & 0xff);
	*(++buf) = (val >> 16) & 0xff;
	*(++buf) = (val >> 8) & 0xff;
	*(++buf) = val & 0xff;
    }
}

static int
mppc_compress(struct ppp_mppe_state *state, unsigned char *ibuf,
	      unsigned char *obuf, int isize, int osize)
{
    u32 olen, off, len, idx, i, l;
    u8 *hist, *sbuf, *p, *q, *r, *s;

    /*  
  	At this point, to avoid possible buffer overflow caused by packet
  	expansion during/after compression,  we should make sure that
  	osize >= (((isize*9)/8)+1)+2, but we don't do that because in
  	ppp_generic.c we simply allocate bigger obuf.

  	Maximum MPPC packet expansion is 12.5%. This is the worst case when
  	all octets in the input buffer are >= 0x80 and we cannot find any
  	repeated tokens. Additionally we have to reserve 2 bytes for MPPE/MPPC
  	status bits and coherency counter.
  	*/

    hist = state->hist + MPPE_HIST_LEN;
    /* check if there is enough room at the end of the history */
    if (state->histptr + isize >= 2*MPPE_HIST_LEN) {
	state->bits |= MPPE_BIT_RESET;
	state->histptr = MPPE_HIST_LEN;
	memcpy(state->hist, hist, MPPE_HIST_LEN);
    }
    /* add packet to the history; isize must be <= MPPE_HIST_LEN */
    sbuf = state->hist + state->histptr;
    memcpy(sbuf, ibuf, isize);
    state->histptr += isize;

    /* compress data */
    r = sbuf + isize;
    *obuf = olen = i = 0;
    l = 8;
    while (i < isize - 2) {
	s = q = sbuf + i;
	idx = ((40543*((((s[0]<<4)^s[1])<<4)^s[2]))>>4) & 0x1fff;
	p = hist + state->hash[idx];
	state->hash[idx] = (u16) (s - hist);
	off = s - p;
	if (off > MPPE_HIST_LEN - 1 || off < 1 || *p++ != *s++ || *p++ != *s++ ||
	    *p++ != *s++) {
	    /* no match found; encode literal byte */
	    if (ibuf[i] < 0x80) {		/* literal byte < 0x80 */
		putbits8(obuf, (u32) ibuf[i], 8, &olen, &l);
	    } else {				/* literal byte >= 0x80 */
		putbits16(obuf, (u32) (0x100|(ibuf[i]&0x7f)), 9, &olen, &l);
	    }
	    ++i;
	    continue;
	}
	if (r - q >= 64) {
	    *p++ != *s++ || *p++ != *s++ || *p++ != *s++ || *p++ != *s++ ||
	    *p++ != *s++ || *p++ != *s++ || *p++ != *s++ || *p++ != *s++ ||
	    *p++ != *s++ || *p++ != *s++ || *p++ != *s++ || *p++ != *s++ ||
	    *p++ != *s++ || *p++ != *s++ || *p++ != *s++ || *p++ != *s++ ||
	    *p++ != *s++ || *p++ != *s++ || *p++ != *s++ || *p++ != *s++ ||
	    *p++ != *s++ || *p++ != *s++ || *p++ != *s++ || *p++ != *s++ ||
	    *p++ != *s++ || *p++ != *s++ || *p++ != *s++ || *p++ != *s++ ||
	    *p++ != *s++ || *p++ != *s++ || *p++ != *s++ || *p++ != *s++ ||
	    *p++ != *s++ || *p++ != *s++ || *p++ != *s++ || *p++ != *s++ ||
	    *p++ != *s++ || *p++ != *s++ || *p++ != *s++ || *p++ != *s++ ||
	    *p++ != *s++ || *p++ != *s++ || *p++ != *s++ || *p++ != *s++ ||
	    *p++ != *s++ || *p++ != *s++ || *p++ != *s++ || *p++ != *s++ ||
	    *p++ != *s++ || *p++ != *s++ || *p++ != *s++ || *p++ != *s++ ||
	    *p++ != *s++ || *p++ != *s++ || *p++ != *s++ || *p++ != *s++ ||
	    *p++ != *s++ || *p++ != *s++ || *p++ != *s++ || *p++ != *s++ ||
	    *p++ != *s++;
	    if (s - q == 64) {
		p--; s--;
		while((*p++ == *s++) && (s < r) && (p < q));
	    }
	} else {
	    while((*p++ == *s++) && (s < r) && (p < q));
	}
	len = s - q - 1;
	i += len;

	/* at least 3 character match found; code data */
	/* encode offset */
	if (off < 64) {			/* 10-bit offset; 0 <= offset < 64 */
	    putbits16(obuf, 0x3c0|off, 10, &olen, &l);
	} else if (off < 320) {		/* 12-bit offset; 64 <= offset < 320 */
	    putbits16(obuf, 0xe00|(off-64), 12, &olen, &l);
	} else if (off < 8192) {	/* 16-bit offset; 320 <= offset < 8192 */
	    putbits16(obuf, 0xc000|(off-320), 16, &olen, &l);
	} else {
	    /* This shouldn't happen; we return 0 what means "packet expands",
 	    and we send packet uncompressed. */
	    if (state->debug)
		printk(KERN_DEBUG "%s%d: wrong offset value: %d\n",
		       __FUNCTION__, state->unit, off);
	    return 0;
	}
	/* encode length of match */
	if (len < 4) {			/* length = 3 */
	    putbits8(obuf, 0, 1, &olen, &l);
	} else if (len < 8) {		/* 4 <= length < 8 */
	    putbits8(obuf, 0x08|(len&0x03), 4, &olen, &l);
	} else if (len < 16) {		/* 8 <= length < 16 */
	    putbits8(obuf, 0x30|(len&0x07), 6, &olen, &l);
	} else if (len < 32) {		/* 16 <= length < 32 */
	    putbits8(obuf, 0xe0|(len&0x0f), 8, &olen, &l);
	} else if (len < 64) {		/* 32 <= length < 64 */
	    putbits16(obuf, 0x3c0|(len&0x1f), 10, &olen, &l);
	} else if (len < 128) {		/* 64 <= length < 128 */
	    putbits16(obuf, 0xf80|(len&0x3f), 12, &olen, &l);
	} else if (len < 256) {		/* 128 <= length < 256 */
	    putbits16(obuf, 0x3f00|(len&0x7f), 14, &olen, &l);
	} else if (len < 512) {		/* 256 <= length < 512 */
	    putbits16(obuf, 0xfe00|(len&0xff), 16, &olen, &l);
	} else if (len < 1024) {	/* 512 <= length < 1024 */
	    putbits24(obuf, 0x3fc00|(len&0x1ff), 18, &olen, &l);
	} else if (len < 2048) {	/* 1024 <= length < 2048 */
	    putbits24(obuf, 0xff800|(len&0x3ff), 20, &olen, &l);
	} else if (len < 4096) {	/* 2048 <= length < 4096 */
	    putbits24(obuf, 0x3ff000|(len&0x7ff), 22, &olen, &l);
	} else if (len < 8192) {	/* 4096 <= length < 8192 */
	    putbits24(obuf, 0xffe000|(len&0xfff), 24, &olen, &l);
	} else {
	    /* This shouldn't happen; we return 0 what means "packet expands",
 	    and send packet uncompressed. */
	    if (state->debug)
		printk(KERN_DEBUG "%s%d: wrong length of match value: %d\n",
		       __FUNCTION__, state->unit, len);
	    return 0;
	}
    }

    /* Add remaining octets to the output */
    while(isize - i > 0) {
	if (ibuf[i] < 0x80) {	/* literal byte < 0x80 */
	    putbits8(obuf, (u32) ibuf[i++], 8, &olen, &l);
	} else {		/* literal byte >= 0x80 */
	    putbits16(obuf, (u32) (0x100|(ibuf[i++]&0x7f)), 9, &olen, &l);
	}
    }
    /* Reset unused bits of the last output octet */
    if ((l != 0) && (l != 8)) {
	putbits8(obuf, 0, l, &olen, &l);
    }

    return (int) olen;
}

int
mppe_compress(void *arg, unsigned char *ibuf, unsigned char *obuf,
	      int isize, int osize)
{
    struct ppp_mppe_state *state = (struct ppp_mppe_state *) arg;
    int proto, olen, complen, off;
    unsigned char *wptr;

    /* Check that the protocol is in the range we handle. */
    proto = PPP_PROTOCOL(ibuf);
    if (proto < 0x0021 || proto > 0x00fa)
	return 0;

    wptr = obuf;
    /* Copy over the PPP header */
    wptr[0] = PPP_ADDRESS(ibuf);
    wptr[1] = PPP_CONTROL(ibuf);
    wptr[2] = PPP_COMP >> 8;
    wptr[3] = PPP_COMP;
    wptr += PPP_HDRLEN + (MPPE_OVHD / 2); /* Leave two octets for MPPE/MPPC bits */

    /* 
     * In ver. 0.99 protocol field was compressed. Deflate and BSD compress
     * do PFC before actual compression, RCF2118 and RFC3078 are not precise
     * on this topic so I decided to do PFC. Unfortunately this change caused
     * incompatibility with older/other MPPE/MPPC modules. I have received
     * a lot of complaints from unexperienced users so I have decided to revert
     * to previous state, i.e. the protocol field is sent uncompressed now.
     * Although this may be changed in the future.
     *
     * Receiving side (mppe_decompress()) still accepts packets with compressed
     * and uncompressed protocol field so you shouldn't get "Unsupported protocol
     * 0x2145 received" messages anymore.
     */
    //off = (proto > 0xff) ? 2 : 3; /* PFC - skip first protocol byte if 0 */
    off = 2;

    ibuf += off;

    mppe_increase_ccount(state);

    if (state->nextflushed) {
	state->bits |= MPPE_BIT_FLUSHED;
	state->nextflushed = 0;
	if (state->mppe && !state->stateless) {
	    /*
 	     * If this is the flag packet, the key has been already changed in
 	     * mppe_increase_ccount() so we dont't do it once again.
 	     */
	    if ((state->ccount & 0xff) != 0xff) {
		arc4_setkey(state, state->session_key, state->keylen);
	    }
	}
	if (state->mppc) { /* reset history */
	    state->bits |= MPPE_BIT_RESET;
	    state->histptr = MPPE_HIST_LEN;
	    memset(state->hist + MPPE_HIST_LEN, 0, MPPE_HIST_LEN*sizeof(u8));
	}
    }

    if (state->mppc && !state->mppe) { /* Do only compression */
	complen = mppc_compress(state, ibuf, wptr, isize - off,
				osize - PPP_HDRLEN - (MPPE_OVHD / 2));
	/*
 	 * TODO: Implement an heuristics to handle packet expansion in a smart
 	 * way. Now, when a packet expands, we send it as uncompressed and
 	 * when next packet is sent we have to reset compressor's history.
 	 * Maybe it would be better to send such packet as compressed in order
 	 * to keep history's continuity.
 	 */
	if ((complen > isize) || (complen > osize - PPP_HDRLEN) ||
	    (complen == 0)) {
	    /* packet expands */
	    state->nextflushed = 1;
	    memcpy(wptr, ibuf, isize - off);
	    olen = isize - (off - 2) + MPPE_OVHD;
	    (state->stats).inc_bytes += olen;
	    (state->stats).inc_packets++;
	} else {
	    state->bits |= MPPE_BIT_COMP;
	    olen = complen + PPP_HDRLEN + (MPPE_OVHD / 2);
	    (state->stats).comp_bytes += olen;
	    (state->stats).comp_packets++;
	}
    } else { /* Do encryption with or without compression */
	state->bits |= MPPE_BIT_ENCRYPTED;
	if (!state->mppc && state->mppe) { /* Do only encryption */
	    /* read from ibuf, write to wptr, adjust for PPP_HDRLEN */
	    arc4_encrypt(state, ibuf, isize - off, wptr);
	    olen = isize - (off - 2) + MPPE_OVHD;
	    (state->stats).inc_bytes += olen;
	    (state->stats).inc_packets++;
	} else { /* Do compression and then encryption - RFC3078 */
	    complen = mppc_compress(state, ibuf, wptr, isize - off,
				    osize - PPP_HDRLEN - (MPPE_OVHD / 2));
	    /*
 	     * TODO: Implement an heuristics to handle packet expansion in a smart
 	     * way. Now, when a packet expands, we send it as uncompressed and
 	     * when next packet is sent we have to reset compressor's history.
 	     * Maybe it would be good to send such packet as compressed in order
 	     * to keep history's continuity.
 	     */
	    if ((complen > isize) || (complen > osize - PPP_HDRLEN) ||
		(complen == 0)) {
		/* packet expands */
		state->nextflushed = 1;
		arc4_encrypt(state, ibuf, isize - off, wptr);
		olen = isize - (off - 2) + MPPE_OVHD;
		(state->stats).inc_bytes += olen;
		(state->stats).inc_packets++;
	    } else {
		state->bits |= MPPE_BIT_COMP;
		/* Hack warning !!! RC4 implementation which we use does
 		   encryption "in place" - it means that input and output
 		   buffers can be *the same* memory area. Therefore we don't
 		   need to use a temporary buffer. But be careful - other
 		   implementations don't have to be so nice.
 		   I used to use ibuf as temporary buffer here, but it led
 		   packet sniffers into error. Thanks to Wilfried Weissmann
 		   for pointing that. */
		arc4_encrypt(state, wptr, complen, wptr);
		olen = complen + PPP_HDRLEN + (MPPE_OVHD / 2);
		(state->stats).comp_bytes += olen;
		(state->stats).comp_packets++;
	    }
	}
    }

    /* write status bits and coherency counter into the output buffer */
    wptr = obuf + PPP_HDRLEN;
    wptr[0] = MPPE_CTRLHI(state);
    wptr[1] = MPPE_CTRLLO(state);

    state->bits = 0;

    (state->stats).unc_bytes += isize;
    (state->stats).unc_packets++;

    return olen;
}

/***************************/
/*** Decompression stuff ***/
/***************************/
static inline u32 getbits(const u8 *buf, const u32 n, u32 *i, u32 *l)
{
    static const u32 m[] = {0x00, 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff};
    u32 res, ol;

    ol = *l;
    if (*l >= n) {
	*l = (*l) - n;
	res = (buf[*i] & m[ol]) >> (*l);
	if (*l == 0) {
	    *l = 8;
	    (*i)++;
	}
    } else {
	*l = 8 - n + (*l);
	res = (buf[(*i)++] & m[ol]) << 8;
	res = (res | buf[*i]) >> (*l);
    }

    return res;
}

static inline u32 getbyte(const u8 *buf, const u32 i, const u32 l)
{
    if (l == 8) {
	return buf[i];
    } else {
	return (((buf[i] << 8) | buf[i+1]) >> l) & 0xff;
    }
}

static inline void lamecopy(u8 *dst, u8 *src, u32 len)
{
    while (len--)
	*dst++ = *src++;
}

static int
mppc_decompress(struct ppp_mppe_state *state, unsigned char *ibuf,
		unsigned char *obuf, int isize, int osize)
{
    u32 olen, off, len, bits, val, sig, i, l;
    u8 *history, *s;

    history = state->hist + state->histptr;
    olen = len = i = 0;
    l = 8;
    bits = isize * 8;
    while (bits >= 8) {
	val = getbyte(ibuf, i++, l);
	if (val < 0x80) {		/* literal byte < 0x80 */
	    if (state->histptr < 2*MPPE_HIST_LEN) {
		/* copy uncompressed byte to the history */
		(state->hist)[(state->histptr)++] = (u8) val;
	    } else {
		/* buffer overflow; drop packet */
		if (state->debug)
		    printk(KERN_ERR "%s%d: trying to write outside history "
			   "buffer\n", __FUNCTION__, state->unit);
		return DECOMP_ERROR;
	    }
	    olen++;
	    bits -= 8;
	    continue;
	}

	sig = val & 0xc0;
	if (sig == 0x80) {		/* literal byte >= 0x80 */
	    if (state->histptr < 2*MPPE_HIST_LEN) {
		/* copy uncompressed byte to the history */
		(state->hist)[(state->histptr)++] = 
		    (u8) (0x80|((val&0x3f)<<1)|getbits(ibuf, 1 , &i ,&l));
	    } else {
		/* buffer overflow; drop packet */
		if (state->debug)
		    printk(KERN_ERR "%s%d: trying to write outside history "
			   "buffer\n", __FUNCTION__, state->unit);
		return DECOMP_ERROR;
	    }
	    olen++;
	    bits -= 9;
	    continue;
	}

	/* Not a literal byte so it must be an (offset,length) pair */
	/* decode offset */
	sig = val & 0xf0;
	if (sig == 0xf0) {		/* 10-bit offset; 0 <= offset < 64 */
	    off = (((val&0x0f)<<2)|getbits(ibuf, 2 , &i ,&l));
	    bits -= 10;
	} else {
	    if (sig == 0xe0) {		/* 12-bit offset; 64 <= offset < 320 */
		off = ((((val&0x0f)<<4)|getbits(ibuf, 4 , &i ,&l))+64);
		bits -= 12;
	    } else {
		if ((sig&0xe0) == 0xc0) {/* 16-bit offset; 320 <= offset < 8192 */
		    off = ((((val&0x1f)<<8)|getbyte(ibuf, i++, l))+320);
		    bits -= 16;
		    if (off > MPPE_HIST_LEN - 1) {
			if (state->debug)
			    printk(KERN_DEBUG "%s%d: too big offset value: %d\n",
				   __FUNCTION__, state->unit, off);
			return DECOMP_ERROR;
		    }
		} else {		/* this shouldn't happen */
		    if (state->debug)
			printk(KERN_DEBUG "%s%d: cannot decode offset value\n",
			       __FUNCTION__, state->unit);
		    return DECOMP_ERROR;
		}
	    }
	}
	/* decode length of match */
	val = getbyte(ibuf, i, l);
	if ((val & 0x80) == 0x00) {			/* len = 3 */
	    len = 3;
	    bits--;
	    getbits(ibuf, 1 , &i ,&l);
	} else if ((val & 0xc0) == 0x80) {		/* 4 <= len < 8 */
	    len = 0x04 | ((val>>4) & 0x03);
	    bits -= 4;
	    getbits(ibuf, 4 , &i ,&l);
	} else if ((val & 0xe0) == 0xc0) {		/* 8 <= len < 16 */
	    len = 0x08 | ((val>>2) & 0x07);
	    bits -= 6;
	    getbits(ibuf, 6 , &i ,&l);
	} else if ((val & 0xf0) == 0xe0) {		/* 16 <= len < 32 */
	    len = 0x10 | (val & 0x0f);
	    bits -= 8;
	    i++;
	} else {
	    bits -= 8;
	    val = (val << 8) | getbyte(ibuf, ++i, l);
	    if ((val & 0xf800) == 0xf000) {		/* 32 <= len < 64 */
		len = 0x0020 | ((val >> 6) & 0x001f);
		bits -= 2;
		getbits(ibuf, 2 , &i ,&l);
	    } else if ((val & 0xfc00) == 0xf800) {	/* 64 <= len < 128 */
		len = 0x0040 | ((val >> 4) & 0x003f);
		bits -= 4;
		getbits(ibuf, 4 , &i ,&l);
	    } else if ((val & 0xfe00) == 0xfc00) {	/* 128 <= len < 256 */
		len = 0x0080 | ((val >> 2) & 0x007f);
		bits -= 6;
		getbits(ibuf, 6 , &i ,&l);
	    } else if ((val & 0xff00) == 0xfe00) {	/* 256 <= len < 512 */
		len = 0x0100 | (val & 0x00ff);
		bits -= 8;
		i++;
	    } else {
		bits -= 8;
		val = (val << 8) | getbyte(ibuf, ++i, l);
		if ((val & 0xff8000) == 0xff0000) {	/* 512 <= len < 1024 */
		    len = 0x000200 | ((val >> 6) & 0x0001ff);
		    bits -= 2;
		    getbits(ibuf, 2 , &i ,&l);
		} else if ((val & 0xffc000) == 0xff8000) {/* 1024 <= len < 2048 */
		    len = 0x000400 | ((val >> 4) & 0x0003ff);
		    bits -= 4;
		    getbits(ibuf, 4 , &i ,&l);
		} else if ((val & 0xffe000) == 0xffc000) {/* 2048 <= len < 4096 */
		    len = 0x000800 | ((val >> 2) & 0x0007ff);
		    bits -= 6;
		    getbits(ibuf, 6 , &i ,&l);
		} else if ((val & 0xfff000) == 0xffe000) {/* 4096 <= len < 8192 */
		    len = 0x001000 | (val & 0x000fff);
		    bits -= 8;
		    i++;
		} else {				/* this shouldn't happen */
		    if (state->debug)
			printk(KERN_DEBUG "%s%d: wrong length code: 0x%X\n",
			       __FUNCTION__, state->unit, val);
		    return DECOMP_ERROR;
		}
	    }
	}
	s = state->hist + state->histptr;
	state->histptr += len;
	olen += len;
	if (state->histptr < 2*MPPE_HIST_LEN) {
	    /* copy uncompressed bytes to the history */

	    /* In some cases len may be greater than off. It means that memory
 	     * areas pointed by s and s-off overlap. I had used memmove() here
 	     * because I thought that it acts as libc's version. Unfortunately,
 	     * I was wrong. :-) I got strange errors sometimes. Wilfried suggested
 	     * using of byte by byte copying here and strange errors disappeared.
 	     */
	    lamecopy(s, s - off, len);
	} else {
	    /* buffer overflow; drop packet */
	    if (state->debug)
		printk(KERN_ERR "%s%d: trying to write outside history "
		       "buffer\n", __FUNCTION__, state->unit);
	    return DECOMP_ERROR;
	}
    }

    /* Do PFC decompression */
    len = olen;
    if ((history[0] & 0x01) != 0) {
	obuf[0] = 0;
	obuf++;
	len++;
    }

    if (len <= osize) {
	/* copy uncompressed packet to the output buffer */
	memcpy(obuf, history, olen);
    } else {
	/* buffer overflow; drop packet */
	if (state->debug)
	    printk(KERN_ERR "%s%d: too big uncompressed packet: %d\n",
		   __FUNCTION__, state->unit, len + (PPP_HDRLEN / 2));
	return DECOMP_ERROR;
    }

    return (int) len;
}

int
mppe_decompress(void *arg, unsigned char *ibuf, int isize,
		unsigned char *obuf, int osize)
{
    struct ppp_mppe_state *state = (struct ppp_mppe_state *)arg;
    int seq, bits, uncomplen;

    if (isize <= PPP_HDRLEN + MPPE_OVHD) {
	if (state->debug) {
	    printk(KERN_DEBUG "%s%d: short packet (len=%d)\n",  __FUNCTION__,
		   state->unit, isize);
	}
	return DECOMP_ERROR;
    }

    /* Get coherency counter and control bits from input buffer */
    seq = MPPE_CCOUNT(ibuf);
    bits = MPPE_BITS(ibuf);

    if (state->stateless) {
	/* RFC 3078, sec 8.1. */
	mppe_increase_ccount(state);
	if ((seq != state->ccount) && state->debug)
	    printk(KERN_DEBUG "%s%d: bad sequence number: %d, expected: %d\n",
		   __FUNCTION__, state->unit, seq, state->ccount);
	while (seq != state->ccount)
	    mppe_increase_ccount(state);
    } else {
	/* RFC 3078, sec 8.2. */
	if (state->flushexpected) { /* discard state */
	    if ((bits & MPPE_BIT_FLUSHED)) { /* we received expected FLUSH bit */
		while (seq != state->ccount)
		    mppe_increase_ccount(state);
		state->flushexpected = 0;
	    } else /* drop packet*/
		return DECOMP_ERROR;
	} else { /* normal state */
	    mppe_increase_ccount(state);
	    if (seq != state->ccount) {
		/* Packet loss detected, enter the discard state. */
		if (state->debug)
		    printk(KERN_DEBUG "%s%d: bad sequence number: %d, expected: %d\n",
			   __FUNCTION__, state->unit, seq, state->ccount);
		state->flushexpected = 1;
		return DECOMP_ERROR;
	    }
	}
	if (state->mppe && (bits & MPPE_BIT_FLUSHED)) {
	    arc4_setkey(state, state->session_key, state->keylen);
	}
    }

    if (state->mppc && (bits & (MPPE_BIT_FLUSHED | MPPE_BIT_RESET))) {
	state->histptr = MPPE_HIST_LEN;
	if ((bits & MPPE_BIT_FLUSHED)) {
	    memset(state->hist + MPPE_HIST_LEN, 0, MPPE_HIST_LEN*sizeof(u8));
	} else
	    if ((bits & MPPE_BIT_RESET)) {
		memcpy(state->hist, state->hist + MPPE_HIST_LEN, MPPE_HIST_LEN);
	    }
    }

    /* Fill in the first part of the PPP header. The protocol field
       comes from the decompressed data. */
    obuf[0] = PPP_ADDRESS(ibuf);
    obuf[1] = PPP_CONTROL(ibuf);
    obuf += PPP_HDRLEN / 2;

    if (state->mppe) { /* process encrypted packet */
	if ((bits & MPPE_BIT_ENCRYPTED)) {
	    /* OK, packet encrypted, so decrypt it */
	    if (state->mppc && (bits & MPPE_BIT_COMP)) {
		/* Hack warning !!! RC4 implementation which we use does
 		   decryption "in place" - it means that input and output
 		   buffers can be *the same* memory area. Therefore we don't
 		   need to use a temporary buffer. But be careful - other
 		   implementations don't have to be so nice. */
		arc4_decrypt(state, ibuf + PPP_HDRLEN +	(MPPE_OVHD / 2), isize -
			     PPP_HDRLEN - (MPPE_OVHD / 2), ibuf + PPP_HDRLEN +
			     (MPPE_OVHD / 2));
		uncomplen = mppc_decompress(state, ibuf + PPP_HDRLEN +
					    (MPPE_OVHD / 2), obuf, isize -
					    PPP_HDRLEN - (MPPE_OVHD / 2),
					    osize - (PPP_HDRLEN / 2));
		if (uncomplen == DECOMP_ERROR) {
		    state->flushexpected = 1;
		    return DECOMP_ERROR;
		}
		uncomplen += PPP_HDRLEN / 2;
		(state->stats).comp_bytes += isize;
		(state->stats).comp_packets++;
	    } else {
		uncomplen = isize - MPPE_OVHD;
		/* Decrypt the first byte in order to check if it is
 		   compressed or uncompressed protocol field */
		arc4_decrypt(state, ibuf + PPP_HDRLEN +	(MPPE_OVHD / 2), 1, obuf);
		/* Do PFC decompression */
		if ((obuf[0] & 0x01) != 0) {
		    obuf[1] = obuf[0];
		    obuf[0] = 0;
		    obuf++;
		    uncomplen++;
		}
		/* And finally, decrypt the rest of the frame. */
		arc4_decrypt(state, ibuf + PPP_HDRLEN +	(MPPE_OVHD / 2) + 1,
			     isize - PPP_HDRLEN - (MPPE_OVHD / 2) - 1, obuf + 1);
		(state->stats).inc_bytes += isize;
		(state->stats).inc_packets++;
	    }
	} else { /* this shouldn't happen */
	    if (state->debug)
		printk(KERN_ERR "%s%d: encryption negotiated but not an "
		       "encrypted packet received\n", __FUNCTION__, state->unit);
	    mppe_change_key(state, 0);
	    state->flushexpected = 1;
	    return DECOMP_ERROR;
	}
    } else {
	if (state->mppc) { /* no MPPE, only MPPC */
	    if ((bits & MPPE_BIT_COMP)) {
		uncomplen = mppc_decompress(state, ibuf + PPP_HDRLEN +
					    (MPPE_OVHD / 2), obuf, isize -
					    PPP_HDRLEN - (MPPE_OVHD / 2),
					    osize - (PPP_HDRLEN / 2));
		if (uncomplen == DECOMP_ERROR) {
		    state->flushexpected = 1;
		    return DECOMP_ERROR;
		}
		uncomplen += PPP_HDRLEN / 2;
		(state->stats).comp_bytes += isize;
		(state->stats).comp_packets++;
	    } else {
		memcpy(obuf, ibuf + PPP_HDRLEN + (MPPE_OVHD / 2), isize -
		       PPP_HDRLEN - (MPPE_OVHD / 2));
		uncomplen = isize - MPPE_OVHD;
		(state->stats).inc_bytes += isize;
		(state->stats).inc_packets++;
	    }
	} else { /* this shouldn't happen */
	    if (state->debug)
		printk(KERN_ERR "%s%d: error - not an  MPPC or MPPE frame "
		       "received\n", __FUNCTION__, state->unit);
	    state->flushexpected = 1;
	    return DECOMP_ERROR;
	}
    }

    (state->stats).unc_bytes += uncomplen;
    (state->stats).unc_packets++;

    return uncomplen;
}


/************************************************************
 *  * Module interface table
 *   ************************************************************/

/* These are in ppp_generic.c */
extern int  ppp_register_compressor   (struct compressor *cp);
extern void ppp_unregister_compressor (struct compressor *cp);

/*
 * Functions exported to ppp_generic.c.
 *
 * In case of MPPC/MPPE there is no need to process incompressible data
 * because such a data is sent in MPPC/MPPE frame. Therefore the (*incomp)
 * callback function isn't needed.
 */
struct compressor ppp_mppe = {
    .compress_proto =	CI_MPPE,
    .comp_alloc =	mppe_comp_alloc,
    .comp_free =	mppe_comp_free,
    .comp_init =	mppe_comp_init,
    .comp_reset =	mppe_comp_reset,
    .compress =		mppe_compress,
    .comp_stat =	mppe_stats,
    .decomp_alloc =	mppe_decomp_alloc,
    .decomp_free =	mppe_comp_free,
    .decomp_init =	mppe_decomp_init,
    .decomp_reset =	mppe_decomp_reset,
    .decompress =	mppe_decompress,
    .incomp =		NULL,
    .decomp_stat =	mppe_stats,
    .owner =		THIS_MODULE
};

/************************************************************
 * Module support routines
 ************************************************************/

int __init mppe_module_init(void)
{
    int answer = ppp_register_compressor(&ppp_mppe);
    if (answer == 0) {
	printk(KERN_INFO "MPPE/MPPC encryption/compression module registered\n");
    }
    return answer;
}

void __exit mppe_module_cleanup(void)
{
    ppp_unregister_compressor(&ppp_mppe);
    printk(KERN_INFO "MPPE/MPPC encryption/compression module unregistered\n");
}

module_init(mppe_module_init);
module_exit(mppe_module_cleanup);

MODULE_AUTHOR("Jan Dubiec <jdx@slackware.pl>");
MODULE_DESCRIPTION("MPPE/MPPC encryption/compression module for Linux");
MODULE_VERSION("1.1");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_ALIAS("ppp-compress-" __stringify(CI_MPPE));

/*
 * SHA1 implementation
 */
static void SHA1_Transform(u32[5], const u8[64]);

#define rol(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))

/* blk0() and blk() perform the initial expand. */
/* I got the idea of expanding during the round function from SSLeay */
#if defined(__LITTLE_ENDIAN) || defined(_LITTLE_ENDIAN)
#define blk0(i) (block->l[i] = (rol(block->l[i],24)&0xFF00FF00) \
    |(rol(block->l[i],8)&0x00FF00FF))
#elif defined(__BIG_ENDIAN) || defined(_BIG_ENDIAN)
#define blk0(i) block->l[i]
#else
#error Endianness not defined
#endif
#define blk(i) (block->l[i&15] = rol(block->l[(i+13)&15]^block->l[(i+8)&15] \
    ^block->l[(i+2)&15]^block->l[i&15],1))

/* (R0+R1), R2, R3, R4 are the different operations used in SHA1 */
#define R0(v,w,x,y,z,i) z+=((w&(x^y))^y)+blk0(i)+0x5A827999+rol(v,5);w=rol(w,30);
#define R1(v,w,x,y,z,i) z+=((w&(x^y))^y)+blk(i)+0x5A827999+rol(v,5);w=rol(w,30);
#define R2(v,w,x,y,z,i) z+=(w^x^y)+blk(i)+0x6ED9EBA1+rol(v,5);w=rol(w,30);
#define R3(v,w,x,y,z,i) z+=(((w|x)&y)|(w&x))+blk(i)+0x8F1BBCDC+rol(v,5);w=rol(w,30);
#define R4(v,w,x,y,z,i) z+=(w^x^y)+blk(i)+0xCA62C1D6+rol(v,5);w=rol(w,30);

/* Hash a single 512-bit block. This is the core of the algorithm. */
static void
SHA1_Transform(u32 state[5], const u8 buffer[64])
{
    u32 a, b, c, d, e;
    typedef union {
	u8 c[64];
	u32 l[16];
    } CHAR64LONG16;
    CHAR64LONG16 *block;

#ifdef SHA1HANDSOFF
    static u8 workspace[64];
    block = (CHAR64LONG16 *) workspace;
    memcpy(block, buffer, 64);
#else
    block = (CHAR64LONG16 *) buffer;
#endif
    /* Copy context->state[] to working vars */
    a = state[0];
    b = state[1];
    c = state[2];
    d = state[3];
    e = state[4];
    /* 4 rounds of 20 operations each. Loop unrolled. */
    R0(a,b,c,d,e, 0); R0(e,a,b,c,d, 1); R0(d,e,a,b,c, 2); R0(c,d,e,a,b, 3);
    R0(b,c,d,e,a, 4); R0(a,b,c,d,e, 5); R0(e,a,b,c,d, 6); R0(d,e,a,b,c, 7);
    R0(c,d,e,a,b, 8); R0(b,c,d,e,a, 9); R0(a,b,c,d,e,10); R0(e,a,b,c,d,11);
    R0(d,e,a,b,c,12); R0(c,d,e,a,b,13); R0(b,c,d,e,a,14); R0(a,b,c,d,e,15);
    R1(e,a,b,c,d,16); R1(d,e,a,b,c,17); R1(c,d,e,a,b,18); R1(b,c,d,e,a,19);
    R2(a,b,c,d,e,20); R2(e,a,b,c,d,21); R2(d,e,a,b,c,22); R2(c,d,e,a,b,23);
    R2(b,c,d,e,a,24); R2(a,b,c,d,e,25); R2(e,a,b,c,d,26); R2(d,e,a,b,c,27);
    R2(c,d,e,a,b,28); R2(b,c,d,e,a,29); R2(a,b,c,d,e,30); R2(e,a,b,c,d,31);
    R2(d,e,a,b,c,32); R2(c,d,e,a,b,33); R2(b,c,d,e,a,34); R2(a,b,c,d,e,35);
    R2(e,a,b,c,d,36); R2(d,e,a,b,c,37); R2(c,d,e,a,b,38); R2(b,c,d,e,a,39);
    R3(a,b,c,d,e,40); R3(e,a,b,c,d,41); R3(d,e,a,b,c,42); R3(c,d,e,a,b,43);
    R3(b,c,d,e,a,44); R3(a,b,c,d,e,45); R3(e,a,b,c,d,46); R3(d,e,a,b,c,47);
    R3(c,d,e,a,b,48); R3(b,c,d,e,a,49); R3(a,b,c,d,e,50); R3(e,a,b,c,d,51);
    R3(d,e,a,b,c,52); R3(c,d,e,a,b,53); R3(b,c,d,e,a,54); R3(a,b,c,d,e,55);
    R3(e,a,b,c,d,56); R3(d,e,a,b,c,57); R3(c,d,e,a,b,58); R3(b,c,d,e,a,59);
    R4(a,b,c,d,e,60); R4(e,a,b,c,d,61); R4(d,e,a,b,c,62); R4(c,d,e,a,b,63);
    R4(b,c,d,e,a,64); R4(a,b,c,d,e,65); R4(e,a,b,c,d,66); R4(d,e,a,b,c,67);
    R4(c,d,e,a,b,68); R4(b,c,d,e,a,69); R4(a,b,c,d,e,70); R4(e,a,b,c,d,71);
    R4(d,e,a,b,c,72); R4(c,d,e,a,b,73); R4(b,c,d,e,a,74); R4(a,b,c,d,e,75);
    R4(e,a,b,c,d,76); R4(d,e,a,b,c,77); R4(c,d,e,a,b,78); R4(b,c,d,e,a,79);
    /* Add the working vars back into context.state[] */
    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
    state[4] += e;
    /* Wipe variables */
#ifdef SHA1HANDSOFF
    memset(&workspace, 0, sizeof(workspace));
#endif
    a = b = c = d = e = 0;
}

/* SHA1Init - Initialize new context */
static void
SHA1_Init(SHA1_CTX *context)
{
    /* SHA1 initialization constants */
    context->state[0] = 0x67452301;
    context->state[1] = 0xEFCDAB89;
    context->state[2] = 0x98BADCFE;
    context->state[3] = 0x10325476;
    context->state[4] = 0xC3D2E1F0;
    context->count[0] = context->count[1] = 0;
}

/* Run your data through this. */
static void
SHA1_Update(SHA1_CTX *context, const u8 *data, u32 len)
{
    u32 i, j;

    j = (context->count[0] >> 3) & 63;
    if ((context->count[0] += len << 3) < (len << 3)) context->count[1]++;
    context->count[1] += (len >> 29);
    if ((j + len) > 63) {
	memcpy(&context->buffer[j], data, (i = 64-j));
	SHA1_Transform(context->state, context->buffer);
	for ( ; i + 63 < len; i += 64) {
	    SHA1_Transform(context->state, &data[i]);
	}
	j = 0;
    }
    else
	i = 0;

    memcpy(&context->buffer[j], &data[i], len - i);
}

/* Add padding and return the message digest. */
static void
SHA1_Final(u8 digest[20], SHA1_CTX *context)
{
    u32 i, j;
    u8 finalcount[8];

    for (i = 0; i < 8; i++) {
        finalcount[i] = (u8) ((context->count[(i >= 4 ? 0 : 1)]
         >> ((3-(i & 3)) * 8) ) & 255);  /* Endian independent */
    }
    SHA1_Update(context, (u8 *) "\200", 1);
    while ((context->count[0] & 504) != 448) {
	SHA1_Update(context, (u8 *) "\0", 1);
    }
    SHA1_Update(context, finalcount, 8);  /* Should cause a SHA1Transform() */
    for (i = 0; i < 20; i++) {
	digest[i] = (u8) ((context->state[i>>2] >> ((3-(i & 3)) * 8) ) & 255);
    }
    /* Wipe variables */
    i = j = 0;
    memset(context->buffer, 0, 64);
    memset(context->state, 0, 20);
    memset(context->count, 0, 8);
    memset(&finalcount, 0, 8);
#ifdef SHA1HANDSOFF  /* make SHA1Transform overwrite it's own static vars */
    SHA1Transform(context->state, context->buffer);
#endif
}

