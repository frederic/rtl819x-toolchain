#include <linux/kernel.h>
#include <linux/errno.h>
#include <crypto/aes.h>

#include <net/rtl/rtl_types.h>
#include <net/rtl/rtl_glue.h>
#include <asm/rtl865x/rtl865xc_asicregs.h>
#include "rtl_ipsec.h"
#include "rtl_crypto_helper.h"

//#define CONFIG_RTK_VOIP_DBG

void rtl_crypto_hexdump(unsigned char *buf, unsigned int len)
{
	print_hex_dump(KERN_CONT, "", DUMP_PREFIX_OFFSET,
			16, 1,
			buf, len, false);
}

int rtl_hash_init_ctx(struct crypto_tfm *tfm, struct rtl_hash_ctx *ctx)
{
	const char *algname = crypto_tfm_alg_name(tfm);

	if (strcmp(algname, "md5") == 0)
		ctx->mode = 0x00;
	else if (strcmp(algname, "sha1") == 0)
		ctx->mode = 0x01;
	else
		ctx->mode = -1;
	
	ctx->data = (u8 *) kmalloc(MAX_PKTLEN, GFP_KERNEL);
	ctx->length = 0;
	return 0;
}

int rtl_hash_update(struct rtl_hash_ctx *ctx, u8 *data, u32 length)
{
	memcpy(ctx->data + ctx->length, data, length);
	ctx->length += length;
	return 0;
}

int rtl_hash_final(struct rtl_hash_ctx *ctx, u8 *out)
{
	int ret;

	ret = rtl_hash_digest(ctx, ctx->data, ctx->length, out);
	kfree(ctx->data);
	return ret;
}

int rtl_hash_digest(struct rtl_hash_ctx *ctx, u8 *data, u32 length, u8 *out)
{
	rtl_ipsecScatter_t scatter[1];

	scatter[0].len = length;
	scatter[0].ptr = (void *) CKSEG1ADDR(data);
	
	/*
		int32 rtl_ipsecEngine(uint32 modeCrypto, uint32 modeAuth, 
			uint32 cntScatter, rtl_ipsecScatter_t *scatter, void *pCryptResult,
			uint32 lenCryptoKey, void* pCryptoKey, 
			uint32 lenAuthKey, void* pAuthKey, 
			void* pIv, void* pPad, void* pDigest,
			uint32 a2eo, uint32 enl)
	*/
	return rtl_ipsecEngine(-1, ctx->mode, 1, scatter, NULL,
		0, NULL,
		0, NULL,
		NULL, NULL, out,
		0, length);
}

int rtl_cipher_init_ctx(struct crypto_tfm *tfm,
	struct rtl_cipher_ctx *ctx)
{
	const char *algname = crypto_tfm_alg_name(tfm);

	memset(ctx, 0, sizeof(*ctx));

	if (strcmp(algname, "cbc(des)") == 0)
		ctx->mode = 0x00;
	else if (strcmp(algname, "cbc(des3_ede)") == 0)
		ctx->mode = 0x01;
	else if (strcmp(algname, "ecb(des)") == 0)
		ctx->mode = 0x02;
	else if (strcmp(algname, "ecb(des3_ede)") == 0)
		ctx->mode = 0x03;
	else if (strcmp(algname, "cbc(aes)") == 0)
		ctx->mode = 0x20;
	else if (strcmp(algname, "ecb(aes)") == 0)
		ctx->mode = 0x22;
	else if (strcmp(algname, "ctr(aes)") == 0)
		ctx->mode = 0x23;
	else
		ctx->mode = -1;

#ifdef CONFIG_RTK_VOIP_DBG
	printk("%s: alg=%s, driver=%s, mode=%x\n", __FUNCTION__,
		crypto_tfm_alg_name(tfm), 
		crypto_tfm_alg_driver_name(tfm),
		ctx->mode
	);
#endif

	ctx->aes_dekey = &ctx->__aes_dekey[32]; // cache align issue
	return 0;
}

int	rtl_cipher_setkey(struct crypto_cipher *cipher, 
	struct rtl_cipher_ctx *ctx, const u8 *key, unsigned int keylen)
{
	ctx->key = (u8 *) key;
	ctx->key_length = keylen;

	// setup aes-dekey
	if (ctx->mode >= 0 && ctx->mode & 0x20)
	{
		struct crypto_aes_ctx *aes_ctx;
		u8 *aes_dekey = ctx->aes_dekey;
		u32 *round_key;

		aes_ctx = crypto_tfm_ctx(crypto_cipher_tfm(cipher));
		round_key = aes_ctx->key_enc;

		// IC accept the de-key in reverse order
		switch (aes_ctx->key_length)
		{
			case 128/8:
				((u32*) aes_dekey)[0] = cpu_to_le32(round_key[4 * 10 + 0]);
				((u32*) aes_dekey)[1] = cpu_to_le32(round_key[4 * 10 + 1]);
				((u32*) aes_dekey)[2] = cpu_to_le32(round_key[4 * 10 + 2]);
				((u32*) aes_dekey)[3] = cpu_to_le32(round_key[4 * 10 + 3]);
				break;
			case 192/8:
				((u32*) aes_dekey)[0] = cpu_to_le32(round_key[4 * 12 + 0]);
				((u32*) aes_dekey)[1] = cpu_to_le32(round_key[4 * 12 + 1]);
				((u32*) aes_dekey)[2] = cpu_to_le32(round_key[4 * 12 + 2]);
				((u32*) aes_dekey)[3] = cpu_to_le32(round_key[4 * 12 + 3]);
				((u32*) aes_dekey)[4] = cpu_to_le32(round_key[4 * 11 + 2]);
				((u32*) aes_dekey)[5] = cpu_to_le32(round_key[4 * 11 + 3]);
				break;
			case 256/8:
				((u32*) aes_dekey)[0] = cpu_to_le32(round_key[4 * 14 + 0]);
				((u32*) aes_dekey)[1] = cpu_to_le32(round_key[4 * 14 + 1]);
				((u32*) aes_dekey)[2] = cpu_to_le32(round_key[4 * 14 + 2]);
				((u32*) aes_dekey)[3] = cpu_to_le32(round_key[4 * 14 + 3]);
				((u32*) aes_dekey)[4] = cpu_to_le32(round_key[4 * 13 + 0]);
				((u32*) aes_dekey)[5] = cpu_to_le32(round_key[4 * 13 + 1]);
				((u32*) aes_dekey)[6] = cpu_to_le32(round_key[4 * 13 + 2]);
				((u32*) aes_dekey)[7] = cpu_to_le32(round_key[4 * 13 + 3]);
				break;
			default:
				printk("%s: unknown aes key length=%d\n",
					__FUNCTION__, aes_ctx->key_length);
				return -EINVAL;
		}
	}

	return 0;
}

int rtl_cipher_crypt(struct crypto_cipher *cipher, u8 bEncrypt,
	struct rtl_cipher_ctx *ctx, u8 *src, unsigned int nbytes, u8 *iv, u8 *dst)
{
	unsigned int bsize = crypto_cipher_blocksize(cipher);
	u8 *key = bEncrypt ? ctx->key : ctx->mode & 0x20 ? ctx->aes_dekey : ctx->key;
	rtl_ipsecScatter_t scatter[1];
	u32 flag_encrypt = bEncrypt ? 4 : 0;
	int err;

#ifdef CONFIG_RTK_VOIP_DBG
	printk("%s: src=%p, len=%d, blk=%d, key=%p, iv=%p, dst=%p\n", __FUNCTION__,
		src, nbytes, bsize, key, iv, dst);

	rtl_crypto_hexdump((void *) src, nbytes);
	rtl_crypto_hexdump((void *) key, ctx->key_length);
	rtl_crypto_hexdump((void *) iv, bsize);
#endif

	dma_cache_wback((u32) src, nbytes);
	dma_cache_wback((u32) key, ctx->key_length);
	dma_cache_wback((u32) iv, bsize);

	scatter[0].len = (nbytes / bsize) * bsize;
	scatter[0].ptr = (void *) CKSEG1ADDR(src);

	/*
		int32 rtl_ipsecEngine(uint32 modeCrypto, uint32 modeAuth, 
			uint32 cntScatter, rtl_ipsecScatter_t *scatter, void *pCryptResult,
			uint32 lenCryptoKey, void* pCryptoKey, 
			uint32 lenAuthKey, void* pAuthKey, 
			void* pIv, void* pPad, void* pDigest,
			uint32 a2eo, uint32 enl)
	*/
	err = rtl_ipsecEngine(ctx->mode | flag_encrypt,
		-1, 1, scatter,
		(void *) CKSEG1ADDR(dst),
		ctx->key_length, (void *) CKSEG1ADDR(key),
		0, NULL,
		(void *) CKSEG1ADDR(iv), NULL, NULL,
		0, scatter[0].len);

	if (unlikely(err))
		printk("%s: rtl_ipsecEngine failed\n", __FUNCTION__);

	dma_cache_inv((u32) dst, nbytes);
#ifdef CONFIG_RTK_VOIP_DBG
	printk("result:\n");
	rtl_crypto_hexdump(dst, nbytes);
#endif

	// return handled bytes, even err! (for blkcipher_walk)
	return nbytes - scatter[0].len;
}

