/* crypto/ecdh/ecdhtest.c */
/* ====================================================================
 * Copyright 2002 Sun Microsystems, Inc. ALL RIGHTS RESERVED.
 *
 * The Elliptic Curve Public-Key Crypto Library (ECC Code) included
 * herein is developed by SUN MICROSYSTEMS, INC., and is contributed
 * to the OpenSSL project.
 *
 * The ECC Code is licensed pursuant to the OpenSSL open source
 * license provided below.
 *
 * The ECDH software is originally written by Douglas Stebila of
 * Sun Microsystems Laboratories.
 *
 */
/* ====================================================================
 * Copyright (c) 1998-2003 The OpenSSL Project.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit. (http://www.openssl.org/)"
 *
 * 4. The names "OpenSSL Toolkit" and "OpenSSL Project" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. For written permission, please contact
 *    openssl-core@openssl.org.
 *
 * 5. Products derived from this software may not be called "OpenSSL"
 *    nor may "OpenSSL" appear in their names without prior written
 *    permission of the OpenSSL Project.
 *
 * 6. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit (http://www.openssl.org/)"
 *
 * THIS SOFTWARE IS PROVIDED BY THE OpenSSL PROJECT ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE OpenSSL PROJECT OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * ====================================================================
 *
 * This product includes cryptographic software written by Eric Young
 * (eay@cryptsoft.com).  This product includes software written by Tim
 * Hudson (tjh@cryptsoft.com).
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../e_os.h"

#include <openssl/opensslconf.h>	/* for OPENSSL_NO_ECDH */
#include <openssl/crypto.h>
#include <openssl/bio.h>
#include <openssl/bn.h>
#include <openssl/objects.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <openssl/err.h>

//Added for test
//CustomOid.h Wapi.h WapiCrypto.h WapiCrypto.c WapiSupplicant.c
#include "WapiCrypto.c"
#include "WapiSupplicant.c"
#include "../crypto/ec/ec_lcl.h"

//For test
#define NOISY

//For wapi
//#include <stdio.h>
//#include <sys/types.h> 
//#include <sys/stat.h> 
#include <fcntl.h>
//#include <sys/dir.h>
//#include <string.h>

//For wapi
//input from wapi udp_sock
#define TMP_PEER_PUB_KEY "/var/tmp/peerPubKey"
#define ADDID_FILE "/var/tmp/ADDID"
#define NAE_FILE "/var/tmp/Nae"
#define NASUE_FILE "/var/tmp/Nasue"
//output to wapi udp_sock
#define TMP_LOCAL_PUB_KEY "/var/tmp/localPubKey"
#define BK_FILE "/var/tmp/BK"
#define NEXT_AUTH_SEED "/var/tmp/nextAuthSeed"
#define BKID_FILE "/var/tmp/BKID"
//use this now!!!
#define KEY_FOR_BK "/var/tmp/key4bk"

#define SUCCESS 0
#define FAILED -1

/*
*   function description: store str (length is strLen) into tmpFile
*
*  parameters:
*  tmpFile (input): output filename
*  str (input): string to store
*  strLen (input): length of string
*
*  return 0: success; return -1: failed
*/
static int storeStr2File(const char * tmpFile, const unsigned char * str, const int strLen)
{
	int fd;

	int ret, toRet;

	//initial
	fd=-1;

	if(tmpFile==NULL)
	{
		printf("%s(%d),tmpFile is null.\n",__FUNCTION__,__LINE__);//Added for test
		toRet=FAILED;
		goto err;
	}
	if(str==NULL)
	{
		printf("%s(%d),str is null.\n",__FUNCTION__,__LINE__);//Added for test
		toRet=FAILED;
		goto err;
	}
	
	fd=open(tmpFile, O_WRONLY | O_CREAT | O_TRUNC);
	if ( fd == -1 ) 
	{
		printf("open %s error.\n", tmpFile);//Added for test
		toRet=FAILED;
		goto err;
	}

	ret=write(fd, (void *)str, strLen);
//	printf("%s(%d),ret=%d, strLen=%d\n",__FUNCTION__,__LINE__,ret, strLen);//Added for test
	if((ret==FAILED)||(ret< strLen))
	{
		printf("%s(%d),error: write file failed.\n",__FUNCTION__,__LINE__);//Added for test
		toRet=FAILED;
		goto err;
	}

	toRet=SUCCESS;

err:
	if(fd!=-1)
		close(fd);
	
	return toRet;
}

/*
*   function description: read str (length is strLen) from tmpFile
*
*  parameters:
*  str (out): string read from tmpFile
*  strLen (input): length of string
*  tmpFile (input): input filename
*
*  return 0: success; return -1: failed
*/
static int readFile2Str(unsigned char * str, int * strLen, const char * tmpFile)
{
	int fd;
	int ret, toRet;
	unsigned char buffer[300];

	//initial
	fd=-1;

	if(tmpFile==NULL)
	{
		printf("%s(%d),tmpFile is null.\n",__FUNCTION__,__LINE__);//Added for test
		toRet=FAILED;
		goto err;
	}
	if(str==NULL)
	{
		printf("%s(%d),str is null.\n",__FUNCTION__,__LINE__);//Added for test
		toRet=FAILED;
		goto err;
	}
	if(strLen==NULL)
	{
		printf("%s(%d),strLen is null.\n",__FUNCTION__,__LINE__);//Added for test
		toRet=FAILED;
		goto err;
	}
	
	fd=open(tmpFile, O_RDONLY);
	if ( fd == -1 ) 
	{
		printf("open %s error.\n", tmpFile);//Added for test
		toRet=FAILED;
		goto err;
	}

	ret=read(fd, (void *)buffer, sizeof(buffer));
//	printf("%s(%d),ret=%d\n",__FUNCTION__,__LINE__,ret);//Added for test
	if(ret==FAILED)
	{
		printf("%s(%d),error: read file failed.\n",__FUNCTION__,__LINE__);//Added for test
		toRet=FAILED;
		goto err;
	}

	memcpy(str, buffer, ret);
	* strLen=ret;

	toRet=SUCCESS;

err:
	if(fd!=-1)
		close(fd);
	
	return toRet;
}

#ifdef OPENSSL_NO_ECDH
int main(int argc, char *argv[])
{
    printf("No ECDH support\n");
    return(0);
}
#else
#include <openssl/ec.h>
#include <openssl/ecdh.h>

#ifdef OPENSSL_SYS_WIN16
#define MS_CALLBACK	_far _loadds
#else
#define MS_CALLBACK
#endif

static const char rnd_seed[] = "string to make the random number generator think it has entropy";

#if 0
static const int KDF1_SHA1_len = 20;
static void *KDF1_SHA1(const void *in, size_t inlen, void *out, size_t *outlen)
	{
#ifndef OPENSSL_NO_SHA
	if (*outlen < SHA_DIGEST_LENGTH)
		return NULL;
	else
		*outlen = SHA_DIGEST_LENGTH;
	return SHA1(in, inlen, out);
#else
	return NULL;
#endif
	}

//For wapi
static const int KDF1_SHA256_len = 32;
static void *KDF1_SHA256(const void *in, size_t inlen, void *out, size_t *outlen)
	{
#ifndef OPENSSL_NO_SHA
	if (*outlen < SHA256_DIGEST_LENGTH)
		return NULL;
	else
		*outlen = SHA256_DIGEST_LENGTH;
	return SHA256(in, inlen, out);
#else
	return NULL;
#endif
	}
#endif

static int test_ecdh_curve(int nid, const char *text, BN_CTX *ctx, BIO *out)
	{
	EC_KEY *a=NULL;
	EC_KEY *b=NULL;
	BIGNUM *x_a=NULL, *y_a=NULL,
	       *x_b=NULL, *y_b=NULL;
	char buf[12];
	unsigned char *abuf=NULL,*bbuf=NULL;
	int i,alen,blen,aout,bout,ret, toRet;
	const EC_GROUP *group;

//	unsigned char priKeyBuf[1000];
	//char * priKeyBuf=NULL;
//	int privKeyBufLen;
//	FILE * fp;
//	unsigned char *p1,*p2,*pp;
//	int pp_len;
//	char name[1000];
//	char header[1000];

	//For test
	unsigned char  b_pubkey[300],*b1;
	 int			b_pubkey_len;
	 unsigned char  a_pubkey[300],*a1;
	 int			a_pubkey_len;

	 unsigned char ab_buf1[300];
	 unsigned char ab_len1;
	 unsigned char ab_buf2[300];
	 unsigned char ab_len2;

//	int res;

	a = EC_KEY_new_by_curve_name(nid);
	b = EC_KEY_new_by_curve_name(nid);
	if (a == NULL || b == NULL)
		goto err;

	group = EC_KEY_get0_group(a);

	if ((x_a=BN_new()) == NULL) goto err;
	if ((y_a=BN_new()) == NULL) goto err;
	if ((x_b=BN_new()) == NULL) goto err;
	if ((y_b=BN_new()) == NULL) goto err;

	if (!EC_KEY_generate_key(a)) goto err;
	
	if (EC_METHOD_get_field_type(EC_GROUP_method_of(group)) == NID_X9_62_prime_field) 
		{
		if (!EC_POINT_get_affine_coordinates_GFp(group,
			EC_KEY_get0_public_key(a), x_a, y_a, ctx)) goto err;
		}
	else
		{
		if (!EC_POINT_get_affine_coordinates_GF2m(group,
			EC_KEY_get0_public_key(a), x_a, y_a, ctx)) goto err;
		}

	if (!EC_KEY_generate_key(b)) goto err;

	if (EC_METHOD_get_field_type(EC_GROUP_method_of(group)) == NID_X9_62_prime_field) 
		{
		if (!EC_POINT_get_affine_coordinates_GFp(group, 
			EC_KEY_get0_public_key(b), x_b, y_b, ctx)) goto err;
		}
	else
		{
		if (!EC_POINT_get_affine_coordinates_GF2m(group, 
			EC_KEY_get0_public_key(b), x_b, y_b, ctx)) goto err;
		}

	///////////////////////
	//For testing
	//alen=KDF1_SHA256_len;
	alen=128;
	abuf=(unsigned char *)OPENSSL_malloc(alen);
	//aout=ECDH_compute_key(abuf,alen,EC_KEY_get0_public_key(b),a,KDF1_SHA256);
	aout=ECDH_compute_key(abuf,alen,EC_KEY_get0_public_key(b),a,NULL);
	//printf("%s(%d), aout=%d\n",__FUNCTION__,__LINE__, aout);//Added for test
	//////////////////////////////////

	///////////////////////
	//For testing
	//blen=KDF1_SHA256_len;
	blen=128;
	bbuf=(unsigned char *)OPENSSL_malloc(blen);
//	bout=ECDH_compute_key(bbuf,blen,EC_KEY_get0_public_key(a),b,KDF1_SHA256);
	bout=ECDH_compute_key(bbuf,blen,EC_KEY_get0_public_key(a),b,NULL);
	//printf("%s(%d), bout=%d\n",__FUNCTION__,__LINE__, bout);//Added for test
	///////////////////////////////

	if ((aout < 4) || (bout != aout) || (memcmp(abuf,bbuf,aout) != 0))
		{
#ifndef NOISY
		BIO_printf(out, " failed\n\n");
		BIO_printf(out, "key a:\n");
		BIO_printf(out, "private key: ");
		BN_print(out, EC_KEY_get0_private_key(a));
		BIO_printf(out, "\n");
		BIO_printf(out, "public key (x,y): ");
		BN_print(out, x_a);
		BIO_printf(out, ",");
		BN_print(out, y_a);
		BIO_printf(out, "\nkey b:\n");
		BIO_printf(out, "private key: ");
		BN_print(out, EC_KEY_get0_private_key(b));
		BIO_printf(out, "\n");
		BIO_printf(out, "public key (x,y): ");
		BN_print(out, x_b);
		BIO_printf(out, ",");
		BN_print(out, y_b);
		BIO_printf(out, "\n");
		BIO_printf(out, "generated key a: ");
		for (i=0; i<bout; i++)
			{
			sprintf(buf, "%02X", bbuf[i]);
			BIO_puts(out, buf);
			}
		BIO_printf(out, "\n");
		BIO_printf(out, "generated key b: ");
		for (i=0; i<aout; i++)
			{
			sprintf(buf, "%02X", abuf[i]);
			BIO_puts(out,buf);
			}
		BIO_printf(out, "\n");
#endif
		fprintf(stderr,"Error in ECDH routines\n");
		toRet=FAILED;
		goto err;
		}
	else
		{
#ifndef NOISY
		BIO_printf(out, " ok\n");
#endif
		}
	///////////////////////////////
	//To store local generated tmp pub key
	a1 = a_pubkey;
	 a_pubkey_len = i2o_ECPublicKey(a, (unsigned char * * )&a1);
//	 printf("%s(%d), a_pubkey_len=%d\n",__FUNCTION__,__LINE__, a_pubkey_len);//Added for test!!!!!
//	 dumpDex(a_pubkey, a_pubkey_len);
	//To store local tmp pub key into tmp file
	ret=storeStr2File(TMP_LOCAL_PUB_KEY, a_pubkey, a_pubkey_len);
	//printf("%s(%d),ret=%d\n",__FUNCTION__,__LINE__,ret);//Added for test
	if(ret==FAILED)
	{
		printf("%s(%d), storeStr2File failed.\n",__FUNCTION__,__LINE__);//Added for test
		toRet=FAILED;
		goto err;
	}

	//To get peer tmp pub key
	//To initial
	memset(b_pubkey, 0, sizeof(b_pubkey));
	b_pubkey_len=0;
	//To get peer pub key from tmp file
	ret=readFile2Str(b_pubkey, &b_pubkey_len, TMP_PEER_PUB_KEY);
//	printf("%s(%d),ret=%d, b_pubkey_len=%d\n",__FUNCTION__,__LINE__,ret, b_pubkey_len);//Added for test
	if(ret==FAILED)
	{
		printf("%s(%d), readFile2Str failed.\n",__FUNCTION__,__LINE__);//Added for test
		toRet=FAILED;
		goto err;
	}
//	dumpDex(b_pubkey, b_pubkey_len);
//	printf("----------------------%s(%d)---------\n",__FUNCTION__,__LINE__);//Added for test
	
	 
	WapiECDHComputeBMK(a, b_pubkey, b_pubkey_len, ab_buf1, &ab_len1);
//	printf("%s(%d), ab_len1=%d\n",__FUNCTION__,__LINE__, ab_len1);//Added for test!!!!
//	dumpDex(ab_buf1, ab_len1);
//	printf("----------------------%s(%d)---------\n",__FUNCTION__,__LINE__);//Added for test
	ret=storeStr2File(KEY_FOR_BK, ab_buf1, ab_len1);
	//printf("%s(%d),ret=%d\n",__FUNCTION__,__LINE__,ret);//Added for test
	if(ret==FAILED)
	{
		printf("%s(%d), storeStr2File failed.\n",__FUNCTION__,__LINE__);//Added for test
		toRet=FAILED;
		goto err;
	 }

	   toRet=SUCCESS;
//	   printf("----------------------%s(%d)---------\n",__FUNCTION__,__LINE__);//Added for teste
	//End for test
err:
	ERR_print_errors_fp(stderr);

	if (abuf != NULL) OPENSSL_free(abuf);
	if (bbuf != NULL) OPENSSL_free(bbuf);
	if (x_a) BN_free(x_a);
	if (y_a) BN_free(y_a);
	if (x_b) BN_free(x_b);
	if (y_b) BN_free(y_b);
	if (b) EC_KEY_free(b);
	if (a) EC_KEY_free(a);
	return(toRet);
	}

int main(int argc, char *argv[])
	{
	BN_CTX *ctx=NULL;
	int ret=1;
	BIO *out;

//	printf("%s(%d)\n",__FUNCTION__,__LINE__);//Added for test

//	test_wapi();//Added for test


	CRYPTO_malloc_debug_init();
	CRYPTO_dbg_set_options(V_CRYPTO_MDEBUG_ALL);
	CRYPTO_mem_ctrl(CRYPTO_MEM_CHECK_ON);

#ifdef OPENSSL_SYS_WIN32
	CRYPTO_malloc_init();
#endif

	RAND_seed(rnd_seed, sizeof rnd_seed);

	out=BIO_new(BIO_s_file());
	if (out == NULL) EXIT(1);
	BIO_set_fp(out,stdout,BIO_NOCLOSE);

	if ((ctx=BN_CTX_new()) == NULL) goto err;

	/* NIST PRIME CURVES TESTS */
	if (test_ecdh_curve(NID_X9_62_prime192v1, "NIST Prime-Curve P-192", ctx, out)==FAILED) goto err;
	
	ret = 0;

err:
	ERR_print_errors_fp(stderr);
	if (ctx) BN_CTX_free(ctx);
	BIO_free(out);
	CRYPTO_cleanup_all_ex_data();
	ERR_remove_state(0);
	CRYPTO_mem_leaks_fp(stderr);
	EXIT(ret);
	return(ret);
	}
#endif

