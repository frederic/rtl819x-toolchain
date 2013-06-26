#ifndef RTL_IPSEC_H
#define RTL_IPSEC_H

// modeCrypto
#define _MD_NOCRYPTO 			((uint32)-1)
#define _MD_CBC					(0)
#define _MD_ECB					(1<<1)
#define _MD_CTR					((1<<0)|(1<<1))
#define _MASK_CRYPTOTHENAUTH	(1<<2)
#define _MASK_CRYPTOAES			(1<<5)
#define _MASK_CRYPTODECRYPTO	(1<<2)
#define _MASK_CRYPTO3DESDES		(1<<0)
#define _MASK_CBCECBCTR			((1<<0)|(1<<1))
#define _MASK_ECBCBC			(1<<1)

// modeAuth
#define _MD_NOAUTH				((uint32)-1)
#define _MASK_AUTHSHA1MD5		(1<<0)
#define _MASK_AUTHHMAC			(1<<1)
#define _MASK_AUTH_IOPAD_READY	(1<<2)

#define MAX_PKTLEN (1<<14)

//Bit 0: 0:DES 1:3DES
//Bit 1: 0:CBC 1:ECB
//Bit 2: 0:Decrypt 1:Encrypt
#define DECRYPT_CBC_DES					0x00
#define DECRYPT_CBC_3DES				0x01
#define DECRYPT_ECB_DES					0x02
#define DECRYPT_ECB_3DES				0x03
#define ENCRYPT_CBC_DES					0x04
#define ENCRYPT_CBC_3DES				0x05
#define ENCRYPT_ECB_DES					0x06
#define ENCRYPT_ECB_3DES				0x07
#define RTL8651_CRYPTO_NON_BLOCKING		0x08
#define RTL8651_CRYPTO_GENERIC_DMA		0x10
#define DECRYPT_CBC_AES					0x20
#define DECRYPT_ECB_AES					0x22
#define DECRYPT_CTR_AES					0x23
#define ENCRYPT_CBC_AES					0x24
#define ENCRYPT_ECB_AES					0x26
#define ENCRYPT_CTR_AES					0x27

//Bit 0: 0:MD5 1:SHA1
//Bit 1: 0:Hash 1:HMAC
#define HASH_MD5		0x00
#define HASH_SHA1		0x01
#define HMAC_MD5		0x02
#define HMAC_SHA1		0x03

#if 0
#ifdef RTL_IPSEC_DEBUG
#define assert(expr) \
	do { \
		if(!(expr)) { \
			printk( "%s:%d: assert(%s)\n", \
			__FILE__,__LINE__,#expr); \
	        } \
	} while (0)
#else
#define assert(expr) do {} while (0)
#endif
#endif

/*
 *  ipsec engine supports scatter list: your data can be stored in several segments those are not continuous.
 *  Each scatter points to one segment of data.
 */
typedef struct rtl_ipsecScatter_s {
	int32 len;
	void* ptr;
} rtl_ipsecScatter_t;

enum RTL_IPSEC_OPTION
{
	RTL_IPSOPT_LBKM, /* loopback mode */
	RTL_IPSOPT_SAWB, /* Source Address Write Back */
	RTL_IPSOPT_DMBS, /* Dest Max Burst Size */
	RTL_IPSOPT_SMBS, /* Source Max Burst Size */
};

/**************************************************************************
 * Data Structure for Descriptor
 **************************************************************************/
typedef struct rtl_ipsec_source_s
{
	uint32 own:1;
	uint32 eor:1;
	uint32 fs:1;
	uint32 resv1:1;
	uint32 ms:2;
	uint32 kam:3;
	uint32 aeskl:2;
	uint32 trides:1;
	uint32 cbc:1;
	uint32 ctr:1;
	uint32 hmac:1;
	uint32 md5:1;
	uint32 resv2:2;
	uint32 sbl:14;
	uint32 resv3:8;
	uint32 a2eo:8;
	uint32 resv4:2;
	uint32 enl:14;
	uint32 resv5:8;
	uint32 apl:8;
	uint32 resv6:16;
	uint32 sdbp;
} rtl_ipsec_source_t;

typedef struct rtl_ipsec_dest_s
{
	uint32 own:1;
	uint32 eor:1;
	uint32 resv1:16;
	uint32 dbl:14;
	uint32 ddbp;
	uint32 resv2;
	uint32 icv[5];
} rtl_ipsec_dest_t;

int32 rtl_ipsecEngine_init(uint32 descNum, int8 mode32Bytes);
int32 rtl_ipsecEngine_exit(void);

int32 rtl_ipsecEngine(uint32 modeCrypto, uint32 modeAuth, 
	uint32 cntScatter, rtl_ipsecScatter_t *scatter, void *pCryptResult,
	uint32 lenCryptoKey, void* pCryptoKey, 
	uint32 lenAuthKey, void* pAuthKey, 
	void* pIv, void* pPad, void* pDigest,
	uint32 a2eo, uint32 enl);

int32 rtl_ipsecGetOption(enum RTL_IPSEC_OPTION option, uint32* value);
int32 rtl_ipsecSetOption(enum RTL_IPSEC_OPTION option, uint32 value);

rtl_ipsec_source_t *get_rtl_ipsec_ipssdar(void);
rtl_ipsec_dest_t *get_rtl_ipsec_ipsddar(void);

void rtl_ipsec_info(void);

extern int g_rtl_ipsec_dbg;

#endif
