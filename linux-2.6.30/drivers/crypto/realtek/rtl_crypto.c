/*
 * -------------------------
 * Realtek Crypto Engine
 * -------------------------
 *
 * Realtek Crypto Engine Core Features:
 * 	- Authentication Only (md5/sha1/hmac-md5/hmac-sha1)
 * 	- Encryption/Decryption Only (des/3des/aes with ecb/cbc/ctr modes)
 *	- Authentication then Encryption/Decryption
 *	- Encryption/Decryption then Authentication
 *
 * TODO:
 * 	- Non-Blocking mode
 *
 * Realtek Crypto Engine API in Linux
 * 	- hash: md5/sha1
 * 	- blkcipher: des/des3_ede/aes
 * 	- blkcipher operation modes: ecb/cbc/ctr
 *
 * TODO:
 * 	- hmac
 * 	- aead transforms
 * 	- standard crypto api register procedure
 */

#include <net/rtl/rtl_types.h>
#include <net/rtl/rtl_glue.h>
#include <asm/rtl865x/rtl865xc_asicregs.h>
#include "rtl_ipsec.h"

#define RTL_CRYPTO_VERSION "v0.1"

static int __init rtl_crypto_init(void)
{
	printk("Realtek Crypto Engine %s\n", RTL_CRYPTO_VERSION);

	rtl_ipsecEngine_init(10, 2);

	rtl_ipsecSetOption(RTL_IPSOPT_SAWB, 0);

	return 0;
}

subsys_initcall(rtl_crypto_init);
