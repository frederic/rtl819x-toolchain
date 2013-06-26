#include <linux/time.h>

#ifdef CONFIG_RTL_ICTEST
#include "rtl_types.h"
#else
#include <net/rtl/rtl_types.h>
#include <net/rtl/rtl_glue.h>
#include <asm/rtl865x/rtl865xc_asicregs.h>
#endif

#include "md5.h"
#include "sha1.h"
#include "hmac.h"
#include "sw_sim/authSim.h"
#include "sw_sim/desSim.h"
#include "sw_sim/aesSim.h"
#include "sw_sim/aes.h"
#include "rtl_ipsec.h"

// address macro
#define PHYSICAL_ADDRESS(x) CPHYSADDR(x)
#define UNCACHED_ADDRESS(x) CKSEG1ADDR(x)
#define CACHED_ADDRESS(x) CKSEG0ADDR(x)

static uint32 rtl_seed = 0xDeadC0de;

uint32 rtlglue_getmstime( uint32* pTime )
{
    struct timeval tm;

    do_gettimeofday( &tm );
    tm.tv_sec = tm.tv_sec % 86400; /* to avoid overflow for 1000times, we wrap to the seconds in a day. */
    return *pTime = ( tm.tv_sec*1000 + tm.tv_usec/1000 );
}

void rtlglue_srandom( uint32 seed )
{
    rtl_seed = seed;
}

uint32 rtlglue_random( void )
{
    uint32 hi32, lo32;

    hi32 = (rtl_seed>>16)*19;
    lo32 = (rtl_seed&0xffff)*19+37;
    hi32 = hi32^(hi32<<16);
    return ( rtl_seed = (hi32^lo32) );
}

int32 ipsecSim(uint32 modeCrypto, uint32 modeAuth,
	uint8 *data, uint8 *pCryptResult,
	uint32 lenCryptoKey, uint8 *pCryptoKey, 
	uint32 lenAuthKey, uint8 *pAuthKey, 
	uint8 *pIv, uint8 *pPad, uint8 *pDigest,
	uint32 lenA2eo, uint32 lenEnl)
{
	if ((modeAuth != (uint32) -1) &&
		(modeCrypto != (uint32) -1))
	{
		if (modeCrypto & 0x04) // encrypt then auth
		{
			if (modeCrypto & 0x20) // aes 
				aesSim_aes(modeCrypto, data + lenA2eo, pCryptResult + lenA2eo,
					lenEnl, lenCryptoKey, pCryptoKey, pIv);
			else // des
				desSim_des(modeCrypto, data + lenA2eo, pCryptResult + lenA2eo,
					lenEnl,	pCryptoKey, pIv);

			memcpy(pCryptResult, data, lenA2eo);

			authSim(modeAuth, pCryptResult, lenA2eo + lenEnl,
				pAuthKey, lenAuthKey, pDigest);
		}
		else // auth then decrypt
		{
			authSim(modeAuth, data, lenA2eo + lenEnl,
				pAuthKey, lenAuthKey, pDigest);
		
			if (modeCrypto & 0x20) // aes
				aesSim_aes(modeCrypto, data + lenA2eo, pCryptResult + lenA2eo,
					lenEnl, lenCryptoKey, pCryptoKey, pIv);
			else // des
				desSim_des(modeCrypto, data + lenA2eo, pCryptResult + lenA2eo,
					lenEnl,	pCryptoKey, pIv);

			memcpy(pCryptResult, data, lenA2eo);
		}
	}
	else if (modeAuth != (uint32) -1) // auth only
	{
		authSim(modeAuth, data, lenA2eo + lenEnl,
			pAuthKey, lenAuthKey, pDigest);
	}
	else if (modeCrypto != (uint32) -1) // crypto only
	{
		if (modeCrypto & 0x20) // aes
			aesSim_aes(modeCrypto, data + lenA2eo, pCryptResult + lenA2eo,
				lenEnl, lenCryptoKey, pCryptoKey, pIv);
		else // des
			desSim_des(modeCrypto, data + lenA2eo, pCryptResult + lenA2eo,
				lenEnl,	pCryptoKey, pIv);
	}
	else
	{
		return FAILED;
	}

	return SUCCESS;
}

static int8 *cryptoModeString[] = {
	"CBC_DES",
	"CBC_3DES",
	"ECB_DES",
	"ECB_3DES",
	"CBC_AES",
	"NONE",
	"ECB_AES",
	"CTR_AES"
};

static int8 *authModeString[] = {
	"HASH_MD5",
	"HASH_SHA1",
	"HMAC_MD5",
	"HMAC_SHA1"
};

char *mixModeString(int32 cryptoIdx, int32 authIdx)
{
	int len;
	static char modestr[128];

	len = 0;
	memset(modestr, 0, sizeof(modestr));

	if (cryptoIdx == -1) // auth only
	{
		if (authIdx >= 0 && authIdx <= 3)
			len += sprintf(&modestr[len], "%s", authModeString[authIdx]);
	}
	else if (authIdx == -1) // encrypt/decrypt only
	{
		if (cryptoIdx >= 0x0 && cryptoIdx <= 0x3)
			len += sprintf(&modestr[len], "DECRYPT_%s", cryptoModeString[cryptoIdx]);
		else if (cryptoIdx >= 0x4 && cryptoIdx <= 0x7)
			len += sprintf(&modestr[len], "ENCRYPT_%s", cryptoModeString[cryptoIdx - 0x4]);
		else if (cryptoIdx >= 0x20 && cryptoIdx <= 0x23)
			len += sprintf(&modestr[len], "DECRYPT_%s", cryptoModeString[cryptoIdx - 0x20 + 4]);
		else if (cryptoIdx >= 0x24 && cryptoIdx <= 0x27)
			len += sprintf(&modestr[len], "ENCRYPT_%s", cryptoModeString[cryptoIdx - 0x24 + 4]);
	}
	else if (cryptoIdx & 0x4) // encode first, then auth
	{
		if (cryptoIdx >= 0x4 && cryptoIdx <= 0x7)
			len += sprintf(&modestr[len], "ENCRYPT_%s", cryptoModeString[cryptoIdx - 0x4]);
		else if (cryptoIdx >= 0x24 && cryptoIdx <= 0x27)
			len += sprintf(&modestr[len], "ENCRYPT_%s", cryptoModeString[cryptoIdx - 0x24 + 4]);

		if (authIdx >= 0 && authIdx <= 3)
			len += sprintf(&modestr[len], ", then %s", authModeString[authIdx]);
	}
	else // auth first, then decode
	{
		if (authIdx >= 0 && authIdx <= 3)
			len += sprintf(&modestr[len], "%s", authModeString[authIdx]);

		if (cryptoIdx >= 0x0 && cryptoIdx <= 0x3)
			len += sprintf(&modestr[len], ", then DECRYPT_%s", cryptoModeString[cryptoIdx]);
		else if (cryptoIdx >= 0x20 && cryptoIdx <= 0x23)
			len += sprintf(&modestr[len], ", then DECRYPT_%s", cryptoModeString[cryptoIdx - 0x20 + 4]);
	}

	return modestr;
}

int32 mix8651bAsicThroughput(int32 round, int32 cryptoStart, int32 cryptoEnd, int32 authStart, int32 authEnd,
	int32 CryptoKeyLen, int32 AuthKeyLen, int32 pktLen, int32 a2eoLen)
{
	uint32 testRound, i, bps;
	uint32 sTime, eTime;
	int32 cryptoIdx, authIdx;
	uint8 *cur;
	int32 lenEnl;
	uint32 cntScatter = 0;
	rtl_ipsecScatter_t src[8];
	static uint8 bufAsic[32+MAX_PKTLEN+32];
	static uint8 _cryptoKey[32+256+32];
	static uint8 _authKey[32+256+32];
	static uint8 _iv[32+32+32]; /* 256-bits */
	static uint8 _pad[32+128+32]; /* ipad + opad */
	static uint8 _AsicDigest[32+20+32]; /* max digest length */
	static uint8 _AsicCrypt[32+MAX_PKTLEN+32];
	uint8 *pCryptoKey = (void *) UNCACHED_ADDRESS( &_cryptoKey[32] );
	uint8 *pAuthKey = (void *) UNCACHED_ADDRESS( &_authKey[32] );
	uint8 *pIv = (void *) UNCACHED_ADDRESS( &_iv[32] );
	uint8 *pPad = (void *) UNCACHED_ADDRESS( &_pad[32] );
	uint8 *pAsicDigest = &_AsicDigest[32];
	uint8 *pAsicCrypt = (void *) UNCACHED_ADDRESS(&_AsicCrypt[32]);
	
	rtlglue_printf("Evaluate 8651b throughput(round=%d,cryptoStart=%d,cryptoEnd=%d,authStart=%d,authEnd=%d,"
		"CryptoKeyLen=%d,AuthKeyLen=%d,pktLen=%d,a2eoLen=%d)\n", round, cryptoStart, cryptoEnd, authStart, authEnd,
		CryptoKeyLen, AuthKeyLen, pktLen, a2eoLen);

	/*  <--lenValid-->
	 *  | A2EO | ENL | APL
	 */
	lenEnl = pktLen - a2eoLen;

	if ( lenEnl <= 0 )
	{
		rtlglue_printf( "[IGNORE,lenEnl<=0]\n" );
		return FAILED;
	}

	/* Simple pattern for debug */
	/* build keys and iv */
	for( i = 0; i < CryptoKeyLen; i++ )
		pCryptoKey[i] = i;
	for( i = 0; i < AuthKeyLen; i++ )
		pAuthKey[i] = i;
	for( i = 0; i < 16; i++ )
		pIv[i] = i;
	for( i = 0; i < 128; i++ )
		pPad[i] = i;

	/* fill the test data */
	cur = (void *) UNCACHED_ADDRESS( &bufAsic[0+32] );
	for( i = 0; i < pktLen; i++ )
	{
		cur[i] = 0xff;
	}

	src[0].ptr = cur;
	src[0].len = pktLen;
	cntScatter = 1;

	for (cryptoIdx=cryptoStart; cryptoIdx<=cryptoEnd; cryptoIdx++) 
	{
		if ( cryptoIdx==-1 || // none
		     cryptoIdx==0x00 || cryptoIdx==0x01 || cryptoIdx==0x02 || cryptoIdx==0x03 || // des/3des decode
		     cryptoIdx==0x04 || cryptoIdx==0x05 || cryptoIdx==0x06 || cryptoIdx==0x07 || // des/3des encode
		     cryptoIdx==0x20 || cryptoIdx==0x22 || cryptoIdx==0x23 || // aes decode
		     cryptoIdx==0x24 || cryptoIdx==0x26 || cryptoIdx==0x27) // aes encode
		{
			/* valid mode, go on */
		}
		else
			continue;

		if ( cryptoIdx!=(uint32)-1 )
		{
			if ( cryptoIdx&0x20 )
			{ /* AES, ENL is the times of 16 bytes. */
				if ( lenEnl&0xf )
				{
					rtlglue_printf( "[IGNORE,ENL=0x%04x]\n", lenEnl );
					return FAILED;
				}
			}
			else
			{ /* DES, ENL is the times of 8 bytes. */
				if ( lenEnl&0x7 )
				{
					rtlglue_printf( "[IGNORE,ENL=0x%04x]\n", lenEnl );
					return FAILED;
				}
			}
		}

		if (cryptoIdx == 0x00 || cryptoIdx == 0x02 || cryptoIdx == 0x04 || cryptoIdx == 0x06)
			CryptoKeyLen = 8; // DES
		else if (cryptoIdx == 0x01 || cryptoIdx == 0x03 || cryptoIdx == 0x05 || cryptoIdx == 0x07)
			CryptoKeyLen = 24; // 3DES

		for (authIdx=authStart; authIdx<=authEnd; authIdx++) 
		{
			if ( authIdx!=(uint32)-1 )
			{
				if ( pktLen&3 )
				{
					/* Since A2EO and APL must be 4-byte times (ENL is 8/16-byte), lenValid must be 4-byte times.
					 * Otherwise, APL will be non-4-byte times. */
					rtlglue_printf( "[IGNORE,Valid=0x%04x]\n", pktLen );
					return FAILED;
				}
			}

			rtlglue_getmstime(&sTime);
			for(testRound=0; testRound<=round; testRound++) 
			{
				if (rtl_ipsecEngine(cryptoIdx, authIdx, cntScatter, src, pAsicCrypt,
					CryptoKeyLen, pCryptoKey, AuthKeyLen, pAuthKey, pIv, pPad, pAsicDigest, 
					a2eoLen, lenEnl) != SUCCESS)
				{
					rtlglue_printf("testRound=%d, rtl8651x_ipsecEngine(modeIdx=%d,authIdx=%d) failed... pktlen=%d\n",
						testRound, cryptoIdx, authIdx, pktLen);
					return FAILED;
				}
			}
			rtlglue_getmstime(&eTime);

			if ( eTime - sTime == 0 )
			{
				rtlglue_printf("round is too small to measure throughput, try larger round number!\n");
				return FAILED;
			}
			else
			{
				bps = pktLen*8*1000/((uint32)(eTime - sTime))*round;
				if(bps>1000000)
					rtlglue_printf("%s round %u len %u time %u throughput %u.%02u mbps\n",
						mixModeString(cryptoIdx, authIdx),
						round, pktLen, (uint32)(eTime - sTime),
						bps/1000000, (bps%1000000)/10000);
				else if(bps>1000)
					rtlglue_printf("%s round %u len %u time %u throughput %u.%02u kbps\n",
						mixModeString(cryptoIdx, authIdx),
						round, pktLen, (uint32)(eTime - sTime),
						bps/1000, (bps%1000)/10);
				else
					rtlglue_printf("%s round %u len %u time %u throughput %u bps\n",
						mixModeString(cryptoIdx, authIdx),
						round, pktLen, (uint32)(eTime - sTime),
						bps);			
			}
		}
	}

	return SUCCESS;
}

/********************************************************************
 *          input
 *         |     |
 *         v     v
 *       SWenc HWenc
 *         |     |
 *         Compare
 *         |     |
 *         v     v
 *       SWdec HWdec
 *         |     |
 *         Compare
 *
 * notes: total length (in scatters) = lenA2eo + lenEnl
 ********************************************************************/
static int32 ipsec_GeneralApiTestItem(uint32 modeCrypto, uint32 modeAuth,
	uint32 cntScatter, rtl_ipsecScatter_t *scatter, void *pCryptResult,
	uint32 lenCryptoKey, void *pCryptoKey, 
	uint32 lenAuthKey, void *pAuthKey, 
	void *pIv, void *pPad, void *pDigest,
	uint32 lenA2eo, uint32 lenEnl)
{
	int i;
	int32 retval;
	uint8 *cur;
	uint32 data_len;
	static uint8 sw_orig[MAX_PKTLEN];
	static uint8 sw_enc[MAX_PKTLEN];
	static uint8 sw_dec[MAX_PKTLEN];
	static uint8 sw_digest[SHA_DIGEST_LENGTH];
	void *pDecryptoKey;
	uint32 sawb;

	rtl_ipsecGetOption(RTL_IPSOPT_SAWB, &sawb);
	if (!sawb && pCryptResult == NULL)
	{
		printk("no pCryptResult && no sawb!\n");
		return FAILED;
	}

	if (modeAuth == (uint32)-1 && modeCrypto == (uint32)-1)
	{
		// do nothing, skip it
		return 1;
	}

	// -1:NONE
	// 0:HASH_MD5,1:HASH_SHA1,2:HMAC_MD5,3:HMAC_SHA1
	if (modeAuth != (uint32)-1 && modeAuth > 3)
	{
		// auth type check failed
		return 2;
	}

	// -1:NONE
	// 0x00:CBC_DES,0x01:CBC_3DES,0x02:ECB_DES,0x03:ECB_3DES
	// 0x20:CBC_AES,0x22:ECB_AES,0x23:CTR_AES
	if (!(modeCrypto == (uint32)-1 ||
		modeCrypto==0x00 ||
		modeCrypto==0x01 ||
		modeCrypto==0x02 ||
		modeCrypto==0x03 ||
		modeCrypto==0x20 ||
		modeCrypto==0x22 ||
		modeCrypto==0x23))
	{
		// crypto type check failed
		return 3;
	}

	// 0,2: DES_CBC or DES_ECB
	// 1,3: 3DES_CBC or 3DES_ECB 
	// 20,22,23: AES_CBC or AES_ECB or AES_CTR
	if (((modeCrypto == 0 || modeCrypto == 2) && lenCryptoKey != 8) ||
		((modeCrypto == 1 || modeCrypto == 3) && lenCryptoKey != 24) ||
		((modeCrypto == 0x20 || modeCrypto == 0x22 || modeCrypto == 0x23) &&
			(lenCryptoKey < 16 || lenCryptoKey > 32)))
	{
		// crypto length check failed
		return 4;
	}

	// mix mode check
	if (modeCrypto != (uint32)-1 && modeAuth >= 0 && modeAuth <= 1)
	{
		// crypto + hash is not support
		return 5;
	}

	if (modeCrypto != (uint32)-1)
	{
		if (modeCrypto & 0x20) /* AES, ENL is the times of 16 bytes. */
		{
			if (lenEnl & 0xf)
				return 6;
		}
		else /* DES, ENL is the times of 8 bytes. */
		{
			if (lenEnl & 0x7)
				return 7;
		}
	}

	if ( lenA2eo & 3 )
	{
		/* A2EO must be 4-byte times */
		return 8;
	}

#if 0
	if (modeAuth != (uint32)-1)
	{
		if ((lenA2eo + lenEnl) & 3)
		{
			/* Since A2EO and APL must be 4-byte times (ENL is 8/16-byte), lenValid must be 4-byte times.
			 * Otherwise, APL will be non-4-byte times. */
			return 1;
		}
	}
#endif

	/*---------------------------------------------------------------------
	 *                                 SW ENC/Auth 
	 *---------------------------------------------------------------------*/

	// scatter to continuous buffer for ipsecSim
	data_len  = 0;
	for (i = 0; i < cntScatter; i++)
	{
		if (scatter[i].ptr == NULL)
		{
			printk("%s():%d Invalid scatter pointer: %p\n",
				__FUNCTION__, __LINE__, scatter[i].ptr);
			return FAILED;
		}

		memcpy(&sw_orig[data_len], scatter[i].ptr, scatter[i].len);
		data_len += scatter[i].len;
	}

	retval = ipsecSim(modeCrypto|4/*enc*/, modeAuth, sw_orig, sw_enc,
		lenCryptoKey, pCryptoKey, lenAuthKey, pAuthKey,
		pIv, pPad, sw_digest, lenA2eo, lenEnl);

	if (retval != SUCCESS)
	{
		return FAILED; // IPSec Simulator Engine unable to encrypt is mandatory testing failed
	}

	/*---------------------------------------------------------------------
	 *                                 ASIC ENC/Auth 
	 *---------------------------------------------------------------------*/
	retval = rtl_ipsecEngine(modeCrypto|4/*enc*/, modeAuth,
		cntScatter, scatter, pCryptResult,
		lenCryptoKey, pCryptoKey, lenAuthKey, pAuthKey,
		pIv, pPad, pDigest, lenA2eo, lenEnl);

	if (retval != SUCCESS)
	{
		return FAILED; // IPSec Engine unable to encrypt is mandatory testing failed
	}
	
	/*---------------------------------------------------------------------
	 *                                 Compare ENC/Auth
	 *---------------------------------------------------------------------*/
	if (modeCrypto != (uint32) -1)
	{
		if (sawb) // use sawb, ASIC write back to scatter buffer
		{
			cur = sw_enc;
			for (i = 0; i < cntScatter; i++)
			{
				if (memcmp(cur, scatter[i].ptr, scatter[i].len) != 0)
				{
					printk("ENC DIFF![%d]crypt:%x auth:%x CKey:%d AKey:%d lenA2eo:%d\n",
						i, modeCrypto, modeAuth, lenCryptoKey, lenAuthKey, lenA2eo);
					memDump(cur, scatter[i].len, "Software");
					memDump(scatter[i].ptr, scatter[i].len, "ASIC");
					return FAILED;
				}
				cur += scatter[i].len;
			}
		}
		else // not use sawb, ASIC write to continuous buffer pCryptResult
		{
			if (memcmp(sw_enc + lenA2eo, pCryptResult + lenA2eo, lenEnl) != 0)
			{
				printk("ENC DIFF![%d]crypt:%x auth:%x CKey:%d AKey:%d lenA2eo:%d\n",
					i, modeCrypto, modeAuth, lenCryptoKey, lenAuthKey, lenA2eo);
				memDump(sw_enc + lenA2eo, lenEnl, "Software");
				memDump(pCryptResult + lenA2eo, lenEnl, "ASIC");
				return FAILED;
			}
		}
	}

	if (modeAuth != (uint32) -1)
	{
		if (memcmp(sw_digest, pDigest, modeAuth&1?20/*SHA1*/:16/*MD5*/ ) != 0) 
		{
			printk("ENC DIGEST! crypt:%x auth:%x CKey:%d AKey:%d lenA2eo:%d\n",
				modeCrypto, modeAuth, lenCryptoKey, lenAuthKey, lenA2eo);
			memDump(sw_digest, modeAuth&1?20/*SHA1*/:16/*MD5*/, "Software");
			memDump(pDigest, modeAuth&1?20/*SHA1*/:16/*MD5*/, "ASIC");
			return FAILED;
		}
	}

	/* Since no crypto employed, no need to hash again. */
	if (modeCrypto == (uint32) -1)
		return SUCCESS;

	/* AES_CTR mode need not verify. */
	if (modeCrypto == 0x23/*AES_CTR*/)
		return SUCCESS;

	/*---------------------------------------------------------------------
	 *                                 SW Auth/DEC
	 *---------------------------------------------------------------------*/
	retval = ipsecSim(modeCrypto/*dec*/, modeAuth, sw_enc, sw_dec,
		lenCryptoKey, pCryptoKey, lenAuthKey, pAuthKey,
		pIv, pPad, sw_digest, lenA2eo, lenEnl);

	if (retval != SUCCESS)
	{
		return FAILED; // IPSec Simulator Engine unable to decrypt is mandatory testing failed
	}

	/*---------------------------------------------------------------------
	 *                                 ASIC Auth/DEC
	 *---------------------------------------------------------------------*/
	// create de-key for ASIC
	if (modeCrypto == 0x20/*AES_CBC*/ || modeCrypto == 0x22/*AES_ECB*/)
	{
		AES_KEY aes_key;
		static uint8 __key_sch[32 + 256/8 + 32];
		static uint8 *key_sch = &__key_sch[32];

		/* IC accept the de-key in reverse order. */
		AES_set_encrypt_key(pCryptoKey, lenCryptoKey*8, &aes_key);
		switch (lenCryptoKey)
		{
			case 128/8:
				memcpy((void *) UNCACHED_ADDRESS(key_sch), &aes_key.rd_key[4*10], 16);
				break;
			case 192/8:
				memcpy((void *) UNCACHED_ADDRESS(key_sch), &aes_key.rd_key[4*12], 16);
				memcpy((void *) UNCACHED_ADDRESS(&key_sch[16]), &aes_key.rd_key[4*11+2], 8);
				break;
			case 256/8:
				memcpy((void *) UNCACHED_ADDRESS(key_sch), &aes_key.rd_key[4*14], 16);
				memcpy((void *) UNCACHED_ADDRESS(&key_sch[16]), &aes_key.rd_key[4*13], 16);
				break;
			default:
				printk("%s():%d unknown lenCryptoKey = %d\n",
					__FUNCTION__, __LINE__, lenCryptoKey);
				return FAILED;
		}
		pDecryptoKey = (void *) UNCACHED_ADDRESS(key_sch);
	}
	else
	{
		pDecryptoKey = pCryptoKey;
	}

	// if sawb disabled,
	// create scatter buffer from continuous software buffer
	if (!sawb) 
	{
		cur = pCryptResult;
		for (i = 0; i < cntScatter; i++)
		{
			memcpy(scatter[i].ptr, cur, scatter[i].len);
			cur += scatter[i].len;
		}
	}

	retval = rtl_ipsecEngine(modeCrypto/*dec*/, modeAuth,
		cntScatter, scatter, pCryptResult,
		lenCryptoKey, pDecryptoKey, lenAuthKey, pAuthKey,
		pIv, pPad, pDigest, lenA2eo, lenEnl);

	if (retval != SUCCESS)
	{
		return FAILED; // IPSec Engine unable to encrypt is mandatory testing failed
	}
	
	/*---------------------------------------------------------------------
	 *                                 Compare Auth/DEC
	 *---------------------------------------------------------------------*/
	if (modeAuth != (uint32) -1)
	{
		if (memcmp(sw_digest, pDigest, modeAuth&1?20/*SHA1*/:16/*MD5*/) != 0)
		{
			printk("DEC DIGEST! crypt:%x auth:%x CKey:%d AKey:%d lenA2eo:%d\n",
				modeCrypto, modeAuth, lenCryptoKey, lenAuthKey, lenA2eo);
			memDump(sw_digest, modeAuth&1?20/*SHA1*/:16/*MD5*/, "Orig");
			memDump(pDigest, modeAuth&1?20/*SHA1*/:16/*MD5*/, "Software");
			return FAILED;
		}
	}

	if (modeCrypto != (uint32) -1)
	{
		if (memcmp(sw_orig + lenA2eo, sw_dec + lenA2eo, lenEnl) != 0)
		{
			printk("DEC DIFF![%d]crypt:%x auth:%x CKey:%d AKey:%d lenA2eo:%d\n",
				i, modeCrypto, modeAuth, lenCryptoKey, lenAuthKey, lenA2eo);
			memDump(sw_orig + lenA2eo, lenEnl, "Orig");
			memDump(sw_dec + lenA2eo, lenEnl, "Software");
			return FAILED;
		}

		if (sawb) // use sawb, ASIC write back to scatter buffer
		{
			cur = sw_dec;
			for (i = 0; i < cntScatter; i++)
			{
				if (memcmp(cur, scatter[i].ptr, scatter[i].len) != 0)
				{
					printk("DEC DIFF![%d]crypt:%x auth:%x CKey:%d AKey:%d lenA2eo:%d\n",
						i, modeCrypto, modeAuth, lenCryptoKey, lenAuthKey, lenA2eo);
					memDump(cur, scatter[i].len, "Software");
					memDump(scatter[i].ptr, scatter[i].len, "ASIC");
					return FAILED;
				}
				cur += scatter[i].len;
			}
		}
		else // not use sawb, ASIC write to continuous buffer pCryptResult
		{
			if (memcmp(sw_dec + lenA2eo, pCryptResult + lenA2eo, lenEnl) != 0)
			{
				printk("DEC DIFF![%d]crypt:%x auth:%x CKey:%d AKey:%d lenA2eo:%d\n",
					i, modeCrypto, modeAuth, lenCryptoKey, lenAuthKey, lenA2eo);
				memDump(sw_dec + lenA2eo, lenEnl, "Software");
				memDump(pCryptResult + lenA2eo, lenEnl, "ASIC");
				return FAILED;
			}
		}
	}

	return SUCCESS;
}

int32 ipsec_GeneralApiTest(uint32 round,
	int32 lenStart, int32 lenEnd,
	int32 offsetStart, int32 offsetEnd,
	int32 cryptoStart, int32 cryptoEnd,
	int32 authStart, int32 authEnd,
	int32 cryptoKeyStart, int32 cryptoKeyEnd,
	int32 authKeyStart, int32 authKeyEnd,
	int32 a2eoStart, int32 a2eoEnd)
{
	int i;
	int32 roundIdx;
	int32 pktLen, offset;
	int32 modeCrypto, modeAuth;
	int32 lenCryptoKey, lenAuthKey;
	int32 lenA2eo;
	int32 ret = SUCCESS;
	static uint8 _cryptoKey[32 + 32 + 32]; // 32 for AES-256
	static uint8 _cryptoIv[32 + 16 + 32]; // 16 for AES
	static uint8 _authKey[32 + SHA_DIGEST_LENGTH + 32]; // 20 for SHA1
	static uint8 _pad[32 + 128 + 32]; // 128 for ipad and opad 
	static uint8 _asic_orig[32 + MAX_PKTLEN + 32];
	static uint8 _asic_enc[32 + MAX_PKTLEN + 32];
	static uint8 _asic_digest[32 + SHA_DIGEST_LENGTH + 32]; // 20 for SHA1
	static uint8 *cryptoKey, *cryptoIv, *authKey, *pad;
	static uint8 *asic_orig, *asic_enc, *asic_digest;
	rtl_ipsecScatter_t scatter;

	cryptoKey = (void *) UNCACHED_ADDRESS(&_cryptoKey[32]);
	cryptoIv = (void *) UNCACHED_ADDRESS(&_cryptoIv[32]);
	authKey = (void *) UNCACHED_ADDRESS(&_authKey[32]);
	pad = (void *) UNCACHED_ADDRESS(&_pad[32]);
	asic_orig = (void *) UNCACHED_ADDRESS(&_asic_orig[32]);
	asic_enc = (void *) UNCACHED_ADDRESS(&_asic_enc[32]);
	asic_digest = (void *) UNCACHED_ADDRESS(&_asic_digest[32]);

	for(i=0; i<24; i++)
		cryptoKey[i] = 0x01;

	for(i=0; i<8; i++)
		cryptoIv[i] = 0x01;

	for (roundIdx=0; roundIdx<round; roundIdx++)
	{
		for (pktLen=lenStart; pktLen<=lenEnd; pktLen+=8) 
		{
			for (offset=offsetStart; offset<=offsetEnd; offset++) 
			{
				for (i=0; i<pktLen; i++)
					asic_orig[i + offset] = (i & 0xFF);

				scatter.len = pktLen;
				scatter.ptr = &asic_orig[offset];

				for (modeCrypto=cryptoStart; modeCrypto<=cryptoEnd; modeCrypto++)
				{
					for (modeAuth=authStart; modeAuth<=authEnd; modeAuth++)
					{
						for (lenCryptoKey=cryptoKeyStart; lenCryptoKey<=cryptoKeyEnd; lenCryptoKey+=8)
						{
							for (lenAuthKey=authKeyStart; lenAuthKey<=authKeyEnd; lenAuthKey+=4)
							{
								for (lenA2eo=a2eoStart; lenA2eo<=a2eoEnd; lenA2eo+=4)
								{
									ret = ipsec_GeneralApiTestItem(modeCrypto, modeAuth,
										1, &scatter, asic_enc,
										lenCryptoKey, cryptoKey,
										lenAuthKey, authKey,
										cryptoIv, NULL/*pad*/, asic_digest,
										lenA2eo, pktLen - lenA2eo);

									if (ret == FAILED)
									{
										printk("Len:%d Offser:%d Crypto:0x%x Auth:0x%x CKey:%d AKey:%d A2EO:%d [FAILED]\n",
											pktLen, offset, modeCrypto, modeAuth, lenCryptoKey, lenAuthKey, lenA2eo);
										return FAILED;
									}
									else if (ret == 1)
									{
//										printk("Len:%d Offset:%d Crypto:0x%x Auth:0x%x CKey:%d AKey:%d A2EO:%d [SKIP]\n",
//											pktLen, offset, modeCrypto, modeAuth, lenCryptoKey, lenAuthKey, lenA2eo);
									}
									else
									{
										printk("Len:%d Offser:%d Crypto:0x%x Auth:0x%x CKey:%d AKey:%d A2EO:%d [SUCCESS]\n",
											pktLen, offset, modeCrypto, modeAuth, lenCryptoKey, lenAuthKey, lenA2eo);
									}
								}

								if (modeAuth == -1) // no auth, don't care auth key len
									break;
							}

							if (modeCrypto == -1) // no crypto, don't care crypto key len
								break;
						}
					}
				}
			}
		}
	}
				
	printk("\nGeneral API Test Finished.\n");
	return SUCCESS;	
}

/****************************************
 * By 'crypto mix rand' command
 *   random key/iv/mode/len/offset/data
 ****************************************/
int32 ipsec_GeneralApiRandTest(uint32 seed, uint32 round) 
{
	int i;
	int32 roundIdx;
	int32 pktLen, offset;
	int32 modeCrypto, modeAuth;
	int32 lenCryptoKey, lenAuthKey;
	int32 lenA2eo, lenEnl;
	int32 ret = SUCCESS;
	int32 modeCrypto_val[8] = {-1, 0x00, 0x01, 0x02, 0x03, 0x20, 0x22, 0x23};
	int32 modeAuth_val[5] = {-1, 0, 1, 2, 3};
	int32 CryptoKey_val[3] = {16, 24, 32};
	static uint8 _cryptoKey[32 + 32 + 32]; // 32 for AES-256
	static uint8 _cryptoIv[32 + 16 + 32]; // 16 for AES
	static uint8 _authKey[32 + SHA_DIGEST_LENGTH + 32]; // 20 for SHA1
	static uint8 _pad[32 + 128 + 32]; // 128 for ipad and opad 
	static uint8 _asic_orig[32 + MAX_PKTLEN + 32];
	static uint8 _asic_enc[32 + MAX_PKTLEN + 32];
	static uint8 _asic_digest[32 + SHA_DIGEST_LENGTH + 32]; // 20 for SHA1
	static uint8 *cryptoKey, *cryptoIv, *authKey, *pad;
	static uint8 *asic_orig, *asic_enc, *asic_digest;
	rtl_ipsecScatter_t scatter;

	cryptoKey = (void *) UNCACHED_ADDRESS(&_cryptoKey[32]);
	cryptoIv = (void *) UNCACHED_ADDRESS(&_cryptoIv[32]);
	authKey = (void *) UNCACHED_ADDRESS(&_authKey[32]);
	pad = (void *) UNCACHED_ADDRESS(&_pad[32]);
	asic_orig = (void *) UNCACHED_ADDRESS(&_asic_orig[32]);
	asic_enc = (void *) UNCACHED_ADDRESS(&_asic_enc[32]);
	asic_digest = (void *) UNCACHED_ADDRESS(&_asic_digest[32]);

	rtlglue_srandom(seed);

	for (roundIdx=0; roundIdx<round; roundIdx++) 
	{
		modeCrypto = modeCrypto_val[rtlglue_random() % 8];
		modeAuth = modeAuth_val[rtlglue_random() % 5];

		if (modeCrypto == 0x00 || modeCrypto == 0x02)
			lenCryptoKey = 8; // DES
		else if (modeCrypto == 0x01 || modeCrypto == 0x03)
			lenCryptoKey = 24; // 3DES
		else
			lenCryptoKey = CryptoKey_val[rtlglue_random() % 3];

		lenAuthKey = rtlglue_random() & 0x3ff;

		for(i=0; i<24; i++)
			cryptoKey[i] = rtlglue_random() & 0xFF;
		for(i=0; i<8; i++)
			cryptoIv[i] = rtlglue_random() & 0xFF;

		if (modeCrypto == -1 && modeAuth == -1)
		{
			continue;
		}
		else if (modeCrypto == -1) // auth only
		{
			lenA2eo = 0; // lenA2eo must 0 if not mix mode

			if (modeAuth & 0x2) // HMAC
				pktLen = rtlglue_random() % (0x3FC0 - 9 - HMAC_MAX_MD_CBLOCK) + 1; // 1..16247
			else // HASH only
				pktLen = rtlglue_random() % (0x3FC0 - 9) + 1; // 1..16311
		}
		else if (modeAuth == -1) // crypto only
		{
			lenA2eo = 0; // lenA2eo must 0 if not mix mode

			if (modeCrypto & 0x20)	// aes
				pktLen = (rtlglue_random() + 16) & 0x3FF0; /* 16 ~ 16368 */   
			else // des
				pktLen = (rtlglue_random() + 8) & 0x3FF8; /* 8 ~ 16376 */       
		} 
		else // mix mode
		{
			modeAuth |= 0x2; // support hmac only if mix mode

#if 1
			lenA2eo = rtlglue_random() & 0xFC; // a2eo must 4 byte aligna
			if (lenA2eo > 200) lenA2eo = 200;
#else
			lenA2eo = rtlglue_random() & 0x3C; // a2eo must 4 byte aligna
#endif
			lenEnl = rtlglue_random() % (0x3FC0 - 9 - HMAC_MAX_MD_CBLOCK - lenA2eo) + 1; /* 1..16247 - a2eo*/

			if (modeCrypto & 0x20)	// aes
				lenEnl = lenEnl & 0x3FF0;
			else // des
				lenEnl = lenEnl & 0x3FF8;

			pktLen = lenEnl + lenA2eo;
		}

		offset = rtlglue_random() & 0x7;

		for(i=0; i<pktLen; i++) //8-byte for IV and 4-byte for DMA_addr (0-3) byte offset test
			asic_orig[i+offset] = rtlglue_random() & 0xFF;
		
		scatter.len = pktLen;
		scatter.ptr = &asic_orig[offset];

		ret = ipsec_GeneralApiTestItem(modeCrypto, modeAuth,
			1, &scatter, asic_enc,
			lenCryptoKey, cryptoKey,
			lenAuthKey, authKey,
			cryptoIv, NULL/*pad*/, asic_digest,
			lenA2eo, pktLen - lenA2eo);

		if (ret == FAILED)
		{
			printk("Len:%d Offser:%d Crypto:0x%x Auth:0x%x CKey:%d AKey:%d A2EO:%d [FAILED]\n",
				pktLen, offset, modeCrypto, modeAuth, lenCryptoKey, lenAuthKey, lenA2eo);
			return FAILED;
		}
		else if (ret > 0)
		{
			printk("Len:%d Offser:%d Crypto:0x%x Auth:0x%x CKey:%d AKey:%d A2EO:%d [SKIP %d]\n",
				pktLen, offset, modeCrypto, modeAuth, lenCryptoKey, lenAuthKey, lenA2eo, ret);
		}
		else
		{
			printk("Len:%d Offser:%d Crypto:0x%x Auth:0x%x CKey:%d AKey:%d A2EO:%d [SUCCESS]\n",
				pktLen, offset, modeCrypto, modeAuth, lenCryptoKey, lenAuthKey, lenA2eo);
		}
	}

	printk("\nGeneral API Random Test Finished.\n");
	return SUCCESS;	
}

void rtl_ipsec_test(int test)
{
	static int seed = 0;

	if (test == 10)
	{
		rtl_ipsecEngine_init(8, 2);
	}
	else if (test == 99)
	{
		rtl_ipsec_info();
	}
	else
	// int32 round, int32 cryptoStart, int32 cryptoEnd, int32 authStart, int32 authEnd,
	// int32 CryptoKeyLen, int32 AuthKeyLen, int32 pktLen, int32 a2eoLen
	switch (test)
	{
	case 11: // DES & 3DES throughput
		mix8651bAsicThroughput(10000, 0, 7, 0, 3, 0, 8, 1400, 16);
		break;

	case 12: // AES 128 throughput
		mix8651bAsicThroughput(10000, 0x20, 0x27, 2, 3, 16, 8, 1408, 16);
		break;

	case 13: // Auth 192 throughput
		mix8651bAsicThroughput(10000, 0x20, 0x27, 2, 3, 24, 8, 1408, 16);
		break;

	case 14: // Auth 256 throughput
		mix8651bAsicThroughput(10000, 0x20, 0x27, 2, 3, 32, 8, 1408, 16);
		break;

	case 15: // DES test
		ipsec_GeneralApiTest(1, 1408, 1408, 0, 0, 0, 3, 0, 3, 8, 24, 16, 16, 16, 16);
		break;

	case 16: // AES test
		ipsec_GeneralApiTest(1, 1408, 1408, 0, 0, 0x20, 0x23, 0, 3, 16, 32, 16, 16, 16, 16);
		break;

	case 19: // Random test
		ipsec_GeneralApiRandTest(seed++, 10000000);
		break;

	case 21: // debug test
	{
		g_rtl_ipsec_dbg = 1;
		ipsec_GeneralApiRandTest(seed++, 1);
		g_rtl_ipsec_dbg = 0;
		break;
	}

	case 31: // test vector
	{
		static uint8 _cryptoKey[32 + 32 + 32]; // 32 for AES-256
		static uint8 _cryptoIv[32 + 16 + 32]; // 16 for AES
		static uint8 _asic_orig[32 + MAX_PKTLEN + 32];
		static uint8 _asic_enc[32 + MAX_PKTLEN + 32];
		static uint8 *cryptoKey, *cryptoIv;
		static uint8 *asic_orig, *asic_enc;
		rtl_ipsecScatter_t scatter[1];
		char *key = "\x06\xa9\x21\x40\x36\xb8\xa1\x5b"
			  "\x51\x2e\x03\xd5\x34\x12\x00\x06";
		int klen = 16;
		char *iv = "\x3d\xaf\xba\x42\x9d\x9e\xb4\x30"
			  "\xb4\x22\xda\x80\x2c\x9f\xac\x41";
		char *input = "\xe3\x53\x77\x9c\x10\x79\xae\xb8"
			  "\x27\x08\x94\x2d\xbe\x77\x18\x1a";
		int ilen = 16;
		char *result = "Single block msg";
		AES_KEY aes_key;

		cryptoKey = (void *) UNCACHED_ADDRESS(&_cryptoKey[32]);
		cryptoIv = (void *) UNCACHED_ADDRESS(&_cryptoIv[32]);
		asic_orig = (void *) UNCACHED_ADDRESS(&_asic_orig[32]);
		asic_enc = (void *) UNCACHED_ADDRESS(&_asic_enc[32]);

		AES_set_encrypt_key(key, klen*8, &aes_key);
		memcpy((void *) cryptoKey, &aes_key.rd_key[4*10], 16);
		memDump(aes_key.rd_key, 11*4*4, "key");

		memcpy(cryptoIv, iv, 16);
		memcpy(asic_orig, input, ilen);

		scatter[0].len = ilen;
		scatter[0].ptr = input;

		g_rtl_ipsec_dbg = 1;
		rtl_ipsecEngine(0x20, -1, 1, scatter, _asic_enc,
			klen, cryptoKey,
			0, NULL,
			cryptoIv, NULL, NULL,
			0, ilen
		);
		g_rtl_ipsec_dbg = 0;

		if (memcmp(scatter[0].ptr, result, ilen) == 0)
			printk("aes test vector success!\n");
		else
			printk("aes test vector failed!\n");

		break;
	}

	default:
		break;
	}
}

