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
//#include <openssl/bio.h>
#include <openssl/bn.h>
#include <openssl/objects.h>
#include <openssl/rand.h>
//#include <openssl/sha.h>
//#include <openssl/err.h>

//Added for test
//CustomOid.h Wapi.h WapiCrypto.h WapiCrypto.c WapiSupplicant.c
//#include "WapiCrypto.c"
//#include "WapiSupplicant.c"
//#include "../crypto/ec/ec_lcl.h"
#include <openssl/evp.h>
#include <openssl/x509.h>

//For wapi
//#include <stdio.h>
//#include <sys/types.h> 
//#include <sys/stat.h> 
#include <fcntl.h>
//#include <sys/dir.h>
//#include <string.h>

#define TRACE printf

typedef int INT;
typedef unsigned int UINT;
typedef unsigned char UCHAR;
typedef unsigned short USHORT;
typedef char BOOLEAN;
typedef unsigned long ULONG;
typedef int HANDLE;

#if 1
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
#endif

//For signature and signature verify now
#define TMP_MSG 				"/var/tmp/msg.txt"			//source data
#define TMP_SIG					"/var/tmp/sig.txt"				//signature(input) -- for signature verify
#define TMP_SIG_VERIFY_RES		"/var/tmp/sigVerifyRes.txt"		//signature verify result: if this file exist, verify OK; else verify wrong.
#define TMP_SIG_OUT				"/var/tmp/sig_out.txt"				//signature(output) -- for signature

#define MAX_MSG_LEN 2400
#define MAX_CERT_LEN 1000
#define MAX_BUF_LEN 2000

#define SUCCESS 0
#define FAILED -1

#define FALSE 0
#define TRUE 1

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
	//unsigned char buffer[MAX_MSG_LEN];
	//unsigned char *buffer=NULL;

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

	//ret=read(fd, (void *)buffer, sizeof(buffer));
	ret=read(fd, (void *)str, MAX_MSG_LEN);
//	printf("%s(%d),ret=%d\n",__FUNCTION__,__LINE__,ret);//Added for test
	if(ret==FAILED)
	{
		printf("%s(%d),error: read file failed.\n",__FUNCTION__,__LINE__);//Added for test
		toRet=FAILED;
		goto err;
	}

	//memcpy(str, buffer, ret);
	* strLen=ret;

	toRet=SUCCESS;

err:
	if(fd!=-1)
		close(fd);
	
	return toRet;
}

#ifdef OPENSSL_NO_ECDSA
int main(int argc, char *argv[])
{
    printf("No ECDSA support\n");
    return(0);
}
#else
#include <openssl/ec.h>
#include <openssl/ecdsa.h>

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

////////////////////////
//for wapi
#if 1
static void  WapiECDSAResultDecode(unsigned char* src, int srcLen, unsigned char*decodeOut,int *decodeOutLen)
{
	int outLen;
	int	getLen = 0;
//	int   i;
	int  tempLen;
	int	tempValue;

	getLen ++;
//check Length is ok or not
	getLen++;
	outLen = 0;
	if(src[getLen] == 0x02)
	{
		getLen ++;
		tempLen = src[getLen];
		if(src[getLen] == 0x19)
		{
			if(src[getLen+1]!=0)
				TRACE("error case \n");
			getLen+=2;
			tempLen -= 1;
			
		}
		else if(src[getLen] <= 0x17)
		{
			tempValue = 0x18-src[getLen];
			memset(decodeOut+outLen,0,tempValue);
			outLen += tempValue;
			getLen += 1;
		}
		else
		{
			getLen += 1;
		}

		memcpy(decodeOut+outLen,src+getLen,tempLen);
		outLen += tempLen;
		getLen += tempLen;
	}

	if(src[getLen] == 0x02)
	{
		getLen ++;
		tempLen = src[getLen];
		if(src[getLen] == 0x19)
		{
			if(src[getLen+1]!=0)
				TRACE("error case \n");
			getLen+=2;
			tempLen -=1;
			
		}
		else if(src[getLen] <= 0x17)
		{
			tempValue = 0x18-src[getLen];
			memset(decodeOut+outLen,0,tempValue);
			outLen += tempValue;
			getLen += 1;
		}
		else
		{
			getLen += 1;
		}

		memcpy(decodeOut+outLen,src+getLen,tempLen);
		outLen += tempLen;
		getLen += tempLen;
	}

	*decodeOutLen = outLen;
}

#else
static void  WapiECDSAResultDecode(unsigned char* src, int srcLen, unsigned char*decodeOut,int *decodeOutLen)
{
	int outLen;
	int	getLen = 0;
//	int   i;
	int  tempLen;

	//printf("%s(%d).\n",__FUNCTION__,__LINE__);//Added for test
	getLen ++;
//check Length is ok or not
	getLen++;
	outLen = 0;
	if(src[getLen] == 0x02)
	{
		getLen ++;
		tempLen = src[getLen];
		if(src[getLen] == 0x19)
		{
			if(src[getLen+1]!=0)
				TRACE("error case \n");
			getLen+=2;
			tempLen -= 1;
			
		}
		else
		{
			getLen += 1;
		}

		memcpy(decodeOut,src+getLen,tempLen);
		outLen += tempLen;
		getLen += tempLen;
	}

	if(src[getLen] == 0x02)
	{
		getLen ++;
		tempLen = src[getLen];
		if(src[getLen] == 0x19)
		{
			if(src[getLen+1]!=0)
				TRACE("error case \n");
			getLen+=2;
			tempLen -=1;
			
		}
		else
		{
			getLen += 1;
		}

		memcpy(decodeOut+outLen,src+getLen,tempLen);
		outLen += tempLen;
		getLen += tempLen;
	}

	//printf("%s(%d).\n",__FUNCTION__,__LINE__);//Added for test
	*decodeOutLen = outLen;
}
#endif

/**************************************************************************
 Input :
    src/srcLen------------the data content/length of calculate the digest
    publicKey/publicKeyLen-------------the public Key for verify
    ecKey---------------------------used to do ECDSA
    out/outLen-----------------------output signature
**************************************************************************/
 
static void WapiECDSASign(unsigned char*data, int  srcLen,EC_KEY *eckey, unsigned char*out, unsigned short *OutLength)
{
	  unsigned char digest[32];
	  //EC_builtin_curve *curves = NULL;
	 size_t  crv_len = 0, n = 0;
	// EC_GROUP *group;
	 unsigned char *signature = NULL; 
	 unsigned int sig_len;
	 int   ret =  0;
	unsigned char decodeOut[60];
	int			decodeOutLen;

	
	 *OutLength=0;//Initial
	 
	 SHA256(data,srcLen,digest);
//	 TRACE("====>WapiECDSASign\n");

	//printf("%s(%d), eckey=%p, eckey->group=%p.\n",__FUNCTION__,__LINE__, eckey, eckey->group);//Added for test
	//if (EC_GROUP_get_degree(EC_KEY_get0_group(eckey)) < 160)
	//{
	//		goto builtin_err;
	//}

	//printf("%s(%d).\n",__FUNCTION__,__LINE__);//Added for test

	if (!EC_KEY_check_key(eckey))
	{
		TRACE(" check key fail\n");
		goto builtin_err;
	}

	//printf("%s(%d).\n",__FUNCTION__,__LINE__);//Added for test

	sig_len = ECDSA_size(eckey);

	if ((signature = (unsigned char *)OPENSSL_malloc(sig_len)) == NULL)
			goto builtin_err;
        if (!ECDSA_sign(0, digest, 32, signature, &sig_len, eckey))
	{
			TRACE("signed  failed\n");
			goto builtin_err;
	}

	//printf("%s(%d).\n",__FUNCTION__,__LINE__);//Added for test
      WapiECDSAResultDecode(signature,sig_len,decodeOut,&decodeOutLen);
	  //printf("%s(%d).\n",__FUNCTION__,__LINE__);//Added for test

	memcpy(out,decodeOut,decodeOutLen);
	*OutLength = decodeOutLen;

builtin_err:	
	//printf("%s(%d).\n",__FUNCTION__,__LINE__);//Added for test
	
	if (signature)
		OPENSSL_free(signature);

//	TRACE("<====WapiECDSASign\n");

 
}
 
/**************************************************************************
return value:
    1-----------fail
    0-----------success
 Input :
    src/srcLen------------the data content/length of calculate the digest
    signature/signatureLen-------------the signature received From Packet
    publicKey/publicKeyLen-------------the public Key for verify
    ecKey---------------------------used to do ECDSA
**************************************************************************/
static int   WapiECDSAVerify(unsigned char*src,int srcLen, unsigned char*signature,unsigned short signatureLen,EC_KEY  *eckey)
{
	 unsigned char digestFromPacket[32];
//	 unsigned char digestFromECDSA[32];
	 size_t  n = 0;
	 const EC_GROUP *group;
	 int  ret =  0;
	 int result = 0;
//	 int  i;
	 int	pubkeyLen;
	 unsigned char pubKey[100],*p;

//	 TRACE("===========>WapiECDSAVerify\n");
	 SHA256(src,srcLen,digestFromPacket);

	 group = EC_KEY_get0_group(eckey);

	 //printf("%s(%d),--------\n",__FUNCTION__,__LINE__);//Added for test	
	 if (EC_GROUP_get_degree(EC_KEY_get0_group(eckey)) < 160)
	 /* drop the curve */ 
	 {	 	
	 	TRACE("curve error \n");
	  	goto end;
	 }

	//printf("%s(%d),--------\n",__FUNCTION__,__LINE__);//Added for test	
	p =  pubKey;
	pubkeyLen = i2o_ECPublicKey(eckey, (unsigned char * * )&p);

	//printf("%s(%d),--------\n",__FUNCTION__,__LINE__);//Added for test	
	//dumpDex(pubKey, pubkeyLen);
	//printf("%s(%d),--------\n",__FUNCTION__,__LINE__);//Added for test	

	//printf("%s(%d),--------\n",__FUNCTION__,__LINE__);//Added for test	
	 if (ECDSA_verify2(0, digestFromPacket, 32, signature, signatureLen, eckey) != 1)
	 {
	    		TRACE(" failed to verify signature \n");
	  		 result = 1;
			 goto end;
	}

//	TRACE("======>WapiECDSAVerify successful \n");
	 result = 0;

end:
	 return result; 
 
}

static void WapiGetDataFromPemX509(unsigned char *buf,int len,unsigned char *x509Buf,int *x509Len,int *x509GetLen)
{
	//unsigned char   buf_x509[MAX_CERT_LEN];
	unsigned char   *buf_x509=NULL;
	int			x509_len;
	int			bOK;
	EVP_ENCODE_CTX ctx;
	int			k,l,i,j;

#if 1
	//To malloc memory
	buf_x509=(unsigned char *)malloc(MAX_CERT_LEN);
	if(buf_x509==NULL)
	{
		printf("%s(%d), malloc failed.\n",__FUNCTION__,__LINE__);//Added for test
		return;
	}
#endif
	
	for(i=11;i<len;)
	{
			
		i++;
		bOK = 0;
		x509_len = 0;
		if (strncmp((char *)&buf[i],"-----\n",6) == 0)
		{
			for(j=i+6;j<len;j++)
			{
				buf_x509[x509_len] = buf[j]; 
				x509_len ++;
				if(strncmp((char *)&buf[j],"-----END ",9) == 0)
				{
						bOK = 1;
						break;
				}
			
			}
			
		}
		else
		{
			continue;
		}
		if(bOK == 1)
		{
			*x509GetLen = j;
			break;
		}
	}
					

	EVP_DecodeInit(&ctx);
	l=EVP_DecodeUpdate(&ctx,buf_x509,&x509_len,buf_x509,x509_len);
	if (l < 0)
	{
		printf("EVP Encode error \n");	
	}
	l=EVP_DecodeFinal(&ctx,&(buf_x509[x509_len]),&k);
	if (l < 0)
	{
		printf("EVP final  error %d  \n",k);
	}
	x509_len+=k;

	//printf("x509_len in sub function %d \n",x509_len);

	*x509Len = x509_len;
	memcpy(x509Buf,buf_x509,x509_len);

#if 1
	if(buf_x509!=NULL)
		free(buf_x509);
#endif

}



 

static void WapiGetPubKeyFromCertificate(X509  *x,EC_KEY *eckey)
{
	 
	 const EC_GROUP *group;
	 EC_POINT 	*pub_key = NULL;
	 X509_CINF	*ci;
	 unsigned char buf[500],*p,*q;
	 int		len;
	 BOOLEAN		bnewPubKey = FALSE;
//	 int			i;

//	TRACE("==========>WapiGetPubKeyFromCertificate \n");
//get group
	group = EC_KEY_get0_group(eckey);

//get pub key from x ===> unsigned char
	ci = x->cert_info;
	p=buf;
	if(ci->key->public_key != NULL)
	{
		len = i2c_ASN1_BIT_STRING(ci->key->public_key,&p);
	}
	else
	{
		TRACE("null case \n");
//		goto end;
		return;
	}

//initialize eckey->pub_key
	if(EC_KEY_get0_public_key(eckey) == NULL)
	{
//		TRACE("pub key == NULL case \n");
		bnewPubKey = TRUE;
		pub_key = EC_POINT_new(group);
		if(pub_key == NULL)
			TRACE("Unable to allocate ec_point \n");
		EC_KEY_set_public_key(eckey, pub_key);
	}
	else
	{
//		TRACE("get pub key \n");
		bnewPubKey = FALSE;
		pub_key = (EC_POINT *)EC_KEY_get0_public_key(eckey);
	}
	 
//set public key
	q = buf + 1;

	o2i_ECPublicKey(&eckey,( const unsigned char * * )&q, len -1);
	
//free memory
	if((bnewPubKey) && (pub_key != EC_KEY_get0_public_key(eckey)))
		EC_POINT_free(pub_key);
//end:
//	TRACE("<==========WapiGetPubKeyFromCertificate \n");

}


static void WapiGetPrivateKeyFromCertificate(unsigned char *private_key,int key_len,EC_KEY *eckey)
{
	 
	 BIGNUM   *privKey;
	 BOOLEAN		bnewPrivateKey = FALSE;
	 int				i;

//	TRACE("==========>WapiGetPrivateKeyFromCertificate \n");


//initialize eckey->pub_key
	if(EC_KEY_get0_private_key(eckey) == NULL)
	{
//		TRACE("private key == NULL case \n");
		bnewPrivateKey = TRUE;
		privKey = BN_new();
		if(privKey == NULL)
			TRACE("Unable to allocate ec_point \n");
		EC_KEY_set_private_key(eckey, privKey);
	}
	else
	{
//		TRACE("get pub key \n");
		bnewPrivateKey = FALSE;
		privKey = (BIGNUM *)EC_KEY_get0_private_key(eckey);
	}
	 
//set private key
	if(!BN_bin2bn(private_key,key_len,(BIGNUM *)EC_KEY_get0_private_key(eckey)))
		TRACE("transvert fail \n");

//	for(i=0;i<6;i++)
//		TRACE("%02x ", EC_KEY_get0_private_key(eckey)->d[i]);

		
//free memory
	if((bnewPrivateKey) && (privKey != EC_KEY_get0_private_key(eckey)))
		BN_free(privKey);

//	TRACE("<==========WapiGetPrivateKeyFromCertificate \n");

}


#if 1

/*
*    return 0: success; return -1: failed
*/
static int readPrivKeyFile2Eckey(EC_KEY * eckey, unsigned char * privKeyFile)
{
	//PWAPI_ASUE_OPEN_CONTEXT           pASUE_Open_context;
	X509                    *xASUE=NULL;
  	FILE                    *fpASUE=NULL;
//	 ASN1_INTEGER *bs;
	 //long l;
//	 UCHAR         pNamebuf[500];
//	 int                 Name_len;
	int                 length=0;
	int			len;
	unsigned char bufASUEPrivateKey[500];
	int			ASUEX509Len,ASUEPrivateKeyLen,ASUEGetLen,ASUEGetLen1;
	int			i;
	unsigned char  *p,*pX509;
//	unsigned char  buf[MAX_BUF_LEN];
	unsigned char  *buf=NULL;
//	unsigned char bufASUEX509[MAX_CERT_LEN];
	unsigned char *bufASUEX509=NULL;
//	unsigned char temp[MAX_CERT_LEN];
	unsigned char *temp=NULL;
	int        tempLen;
	int toRet;
		
       // pASUE_Open_context=(PWAPI_ASUE_OPEN_CONTEXT)pArg;

//get current path
	//memset(currentPath,0,256);
	//GetModuleFileName(NULL,currentPath,256);		
	//(_tcsrchr(currentPath,_T('\\')))[1]=0;
	//strcat(currentPath,"ASUE.cer");
	
    /*  read asue  certificate	*/

	//fpASUE = fopen(currentPath, "r");
	fpASUE = fopen(privKeyFile, "r");
	
	if (!fpASUE) 
	{
		TRACE("open file fail!\n");	
		toRet=-1;
		goto err;
	}
//	TRACE("open file 1 ok!\n");

	//To malloc memory
	buf=(unsigned char *)malloc(MAX_BUF_LEN);
	if(buf==NULL)
	{
		printf("%s(%d), malloc failed.\n",__FUNCTION__,__LINE__);//Added for test
		toRet=-1;
		goto err;
	}

	len = fread(buf, 1, MAX_BUF_LEN, fpASUE);

	xASUE = X509_new();
	if(xASUE==NULL)
	{
		printf("%s(%d), X509_new failed.\n",__FUNCTION__,__LINE__);//Added for test
		toRet=-1;
		goto err;
	}

	//To malloc memory
	bufASUEX509=(unsigned char *)malloc(MAX_CERT_LEN);
	if(bufASUEX509==NULL)
	{
		printf("%s(%d), malloc failed.\n",__FUNCTION__,__LINE__);//Added for test
		toRet=-1;
		goto err;
	}

	for(i=0;i<len;)
	if (strncmp((char *)&buf[i],"-----BEGIN ",11) == 0)
	{
		WapiGetDataFromPemX509(buf,len,bufASUEX509,&ASUEX509Len,&ASUEGetLen);
		break;
		
	}

	p = bufASUEX509;
	d2i_X509(&xASUE, (const unsigned char **)&p, ASUEX509Len);



	for(i=ASUEX509Len;i<len;i++)
	{
		if (strncmp((char *)&buf[i],"-----BEGIN ",11) == 0)
		{
			pX509 = &buf[i];
			WapiGetDataFromPemX509(pX509,(len-ASUEX509Len),bufASUEPrivateKey,&ASUEPrivateKeyLen,&ASUEGetLen1);
			break;
		}
	}

//	if(fpASUE)
//		fclose(fpASUE);

	//TRACE("decode file 1 ok!\n");

	//get publick key  && private key from Certificate
	//eckey = EC_KEY_new_by_curve_name(NID_X9_62_prime192v1);

	if((xASUE == NULL) || (eckey == NULL))
	{
		TRACE("=====>error1\n");
		toRet=-1;
		goto err;
	}

	WapiGetPubKeyFromCertificate(xASUE,eckey);

	if(eckey == NULL)
	{
		TRACE("=====>error2\n");
		toRet=-1;
		goto err;
	}
	WapiGetPrivateKeyFromCertificate(bufASUEPrivateKey+7,24,eckey);

	//To malloc memory
	temp=(unsigned char *)malloc(MAX_CERT_LEN);
	if(temp==NULL)
	{
		printf("%s(%d), malloc failed.\n",__FUNCTION__,__LINE__);//Added for test
		toRet=-1;
		goto err;
	}
	
	p = temp;
	tempLen = i2o_ECPublicKey(eckey, (unsigned char * * )&p);
	//printf("%s(%d), tempLen=%d.\n",__FUNCTION__,__LINE__, tempLen);//Added for test

	//X509_free(xASU);
//	X509_free(xASUE);

	toRet=0;
err:
	if(fpASUE!=NULL)
		fclose(fpASUE);

	if(xASUE!=NULL)
		X509_free(xASUE);

	if(buf!=NULL)
		free(buf);

	if(bufASUEX509!=NULL)
		free(bufASUEX509);

	if(temp!=NULL)
		free(temp);
	
	return toRet;
	
}

/*
*    return 0: success; return -1: failed
*/
static int readPubKey2Eckey(EC_KEY * eckey, unsigned char * certFile)
{
	X509                    *xASU=NULL;
  	FILE                    *fpASU=NULL;
//	 ASN1_INTEGER *bs;
	 //long l;
//	 UCHAR         pNamebuf[500];
//	 int                 Name_len;
	int                 length=0;
	int			len;
	int			ASUX509Len,ASUGetLen;
	int			i;
	unsigned char  *p,*pX509;
//	unsigned char  buf[MAX_BUF_LEN];
	unsigned char  *buf=NULL;
//	unsigned char bufASUX509[MAX_CERT_LEN];
	unsigned char *bufASUX509=NULL;
//	unsigned char temp[MAX_CERT_LEN];
	unsigned char *temp=NULL;
	int        tempLen;
	int toRet;
	
	 /*  read  certificate	*/
   	fpASU = fopen(certFile, "r");
	if (!fpASU) 
	{
		TRACE("open file fail!\n");	
		toRet=-1;
		goto err;
	}

	//To malloc memory
	buf=(unsigned char *)malloc(MAX_BUF_LEN);
	if(buf==NULL)
	{
		printf("%s(%d), malloc failed.\n",__FUNCTION__,__LINE__);//Added for test
		toRet=-1;
		goto err;
	}
	
	len = fread(buf, 1, MAX_BUF_LEN, fpASU);

//	TRACE("open file 2 ok!\n");
	
	xASU = X509_new();
	if(xASU==NULL)
	{
		printf("%s(%d), X509_new failed.\n",__FUNCTION__,__LINE__);//Added for test
		toRet=-1;
		goto err;
	}

	//To malloc memory
	bufASUX509=(unsigned char *)malloc(MAX_CERT_LEN);
	if(bufASUX509==NULL)
	{
		printf("%s(%d), malloc failed.\n",__FUNCTION__,__LINE__);//Added for test
		toRet=-1;
		goto err;
	}

	for(i=0;i<len;)
	if (strncmp((char *)&buf[i],"-----BEGIN ",11) == 0)
	{
		WapiGetDataFromPemX509(buf,len,bufASUX509,&ASUX509Len,&ASUGetLen);
		break;
		
	}

	p = bufASUX509;
	d2i_X509(&xASU, (const unsigned char **)&p, ASUX509Len);

//	fclose(fpASU);
   	
//	TRACE("decode file 2 ok!\n");

//get public key

	//eckey = EC_KEY_new_by_curve_name(NID_X9_62_prime192v1);

	if((xASU == NULL) ||(eckey == NULL))
	{
		TRACE("=====>error3\n");
		toRet=-1;
		goto err;
	}


	WapiGetPubKeyFromCertificate(xASU,eckey);

	//To malloc memory
	temp=(unsigned char *)malloc(MAX_CERT_LEN);
	if(temp==NULL)
	{
		printf("%s(%d), malloc failed.\n",__FUNCTION__,__LINE__);//Added for test
		toRet=-1;
		goto err;
	}
	
	p = temp;
	tempLen = i2o_ECPublicKey(eckey, (unsigned char * * )&p);
	//printf("%s(%d), tempLen=%d.\n",__FUNCTION__,__LINE__, tempLen);//Added for test

//	X509_free(xASU);

	toRet=0;
err:
	if(fpASU!=NULL)
		fclose(fpASU);

	if(buf!=NULL)
		free(buf);

	if(xASU!=NULL)
		X509_free(xASU);

	if(bufASUX509!=NULL)
		free(bufASUX509);

	if(temp!=NULL)
		free(temp);
	
	return toRet;
}

#endif

/*
*    return 0: success; return -1: failed
*/
static int wapiSign(unsigned char *msgFile, unsigned char * privKeyFile, unsigned char * outFile)
{
	int ret, toRet;
	unsigned char outSig[100];
//	unsigned char msgBuf[MAX_MSG_LEN]; 
	unsigned char *msgBuf=NULL;
	int msgLen;
	unsigned short outSigLen;
	EC_KEY *ecKey=NULL;

	BIGNUM *x_a=NULL, *y_a=NULL;
	const EC_GROUP *group;

	BN_CTX *ctx=NULL;

	//printf("%s(%d).\n",__FUNCTION__,__LINE__);//Added for test
	//To initial ec key
	if ((ctx=BN_CTX_new()) == NULL) goto err;
	
	ecKey = EC_KEY_new_by_curve_name(NID_X9_62_prime192v1);//For wapi
	if (ecKey== NULL)
		goto err;

	group = EC_KEY_get0_group(ecKey);

	if ((x_a=BN_new()) == NULL) goto err;
	if ((y_a=BN_new()) == NULL) goto err;

	if (!EC_KEY_generate_key(ecKey)) goto err;
	
	if (EC_METHOD_get_field_type(EC_GROUP_method_of(group)) == NID_X9_62_prime_field) 
		{
		if (!EC_POINT_get_affine_coordinates_GFp(group,
			EC_KEY_get0_public_key(ecKey), x_a, y_a, ctx)) goto err;
		}
	else
		{
		if (!EC_POINT_get_affine_coordinates_GF2m(group,
			EC_KEY_get0_public_key(ecKey), x_a, y_a, ctx)) goto err;
		}
	//End initial ec key

	//printf("%s(%d).\n",__FUNCTION__,__LINE__);//Added for test

	//To malloc memory
	msgBuf=(unsigned char *)malloc(MAX_MSG_LEN);
	if(msgBuf==NULL)
	{
		printf("%s(%d), malloc failed.\n",__FUNCTION__,__LINE__);//Added for test
		toRet=FAILED;
		goto err;
	}

	ret=readFile2Str(msgBuf, &msgLen, msgFile);
	//printf("%s(%d), ret=%d, msgLen=%d.\n",__FUNCTION__,__LINE__,ret,msgLen);//Added for test
	if(ret==FAILED)
	{
		printf("%s(%d), readFile2Str failed.\n",__FUNCTION__,__LINE__);//Added for test
		toRet=FAILED;
		goto err;
	}

	ret=readPrivKeyFile2Eckey(ecKey, privKeyFile);
	if(ret==FAILED)
	{
		printf("%s(%d), readPrivKeyFile2Eckey failed.\n",__FUNCTION__,__LINE__);//Added for test
		toRet=FAILED;
		goto err;
	}

	//printf("%s(%d).\n",__FUNCTION__,__LINE__);//Added for test
	WapiECDSASign(msgBuf, msgLen, ecKey, outSig, &outSigLen);
	//printf("-------%s(%d), msgLen=%d, outSigLen=%d.----------\n",__FUNCTION__,__LINE__,msgLen,outSigLen);//Added for test
	if(outSigLen==0)
	{
		printf("%s(%d), WapiECDSASign failed.\n",__FUNCTION__,__LINE__);//Added for test
		toRet=FAILED;
		goto err;
	}
	//WapiECDSASign(unsigned char*data, int  srcLen,EC_KEY *eckey, unsigned char*out, unsigned short *OutLength)

	ret=storeStr2File(outFile, outSig, outSigLen);
	if(ret==FAILED)
	{
		printf("%s(%d), storeStr2File failed.\n",__FUNCTION__,__LINE__);//Added for test
		toRet=FAILED;
		goto err;
	}
	//printf("%s(%d), outSigLen=%d, outFile=%s.\n",__FUNCTION__,__LINE__,outSigLen, outFile);//Added for test

	toRet=SUCCESS;

err:
	if (ctx) BN_CTX_free(ctx);
	if (x_a) BN_free(x_a);
	if (y_a) BN_free(y_a);
	if (ecKey) EC_KEY_free(ecKey);

	if(msgBuf!=NULL)
		free(msgBuf);
	
	return toRet;
}

/*
*    parameters:
*    msgFile(input): msg file name
*    sigFile(input): signature file name
*    certFile(input): cert file name (to get public key from this cert)
*    verifyResult(output): 1--wrong sign; 0--sign verify OK.
*
*    return 0: success; return -1: failed
*/
static int wapiSignVerify(unsigned char*msgFile, unsigned char*sigFile, unsigned char * certFile, int* verifyResult)
{
	int ret, toRet;
	unsigned char sig[100];
//	unsigned char msgBuf[MAX_MSG_LEN];
	unsigned char *msgBuf=NULL;
	int msgLen, tmpLen;
	unsigned short sigLen;
	EC_KEY *ecKey=NULL;

	BIGNUM *x_a=NULL, *y_a=NULL;
	const EC_GROUP *group;

	BN_CTX *ctx=NULL;

	//printf("%s(%d).\n",__FUNCTION__,__LINE__);//Added for test
	*verifyResult=1;//Initial
	
	//To initial ec key
	if ((ctx=BN_CTX_new()) == NULL) goto err;
	
	ecKey = EC_KEY_new_by_curve_name(NID_X9_62_prime192v1);//For wapi
	if (ecKey== NULL)
		goto err;

	group = EC_KEY_get0_group(ecKey);

	if ((x_a=BN_new()) == NULL) goto err;
	if ((y_a=BN_new()) == NULL) goto err;

	if (!EC_KEY_generate_key(ecKey)) goto err;
	
	if (EC_METHOD_get_field_type(EC_GROUP_method_of(group)) == NID_X9_62_prime_field) 
		{
		if (!EC_POINT_get_affine_coordinates_GFp(group,
			EC_KEY_get0_public_key(ecKey), x_a, y_a, ctx)) goto err;
		}
	else
		{
		if (!EC_POINT_get_affine_coordinates_GF2m(group,
			EC_KEY_get0_public_key(ecKey), x_a, y_a, ctx)) goto err;
		}
	//End initial ec key

	//To malloc memory
	msgBuf=(unsigned char *)malloc(MAX_MSG_LEN);
	if(msgBuf==NULL)
	{
		printf("%s(%d), malloc failed.\n",__FUNCTION__,__LINE__);//Added for test
		toRet=FAILED;
		goto err;
	}

	ret=readFile2Str(msgBuf, &msgLen, msgFile);
	//printf("%s(%d), ret=%d, msgLen=%d, msgFile=%s.\n",__FUNCTION__,__LINE__,ret,msgLen, msgFile);//Added for test
	if(ret==FAILED)
	{
		printf("%s(%d), readFile2Str failed.\n",__FUNCTION__,__LINE__);//Added for test
		toRet=FAILED;
		goto err;
	}

	ret=readFile2Str(sig, &tmpLen, sigFile);
	sigLen=(unsigned short)tmpLen;
	//printf("%s(%d), ret=%d, tmpLen=%d, sigLen=%d, sigFile=%s.\n",__FUNCTION__,__LINE__,ret,tmpLen,sigLen, sigFile);//Added for test
	if(ret==FAILED)
	{
		printf("%s(%d), readFile2Str failed.\n",__FUNCTION__,__LINE__);//Added for test
		toRet=FAILED;
		goto err;
	}

	ret=readPubKey2Eckey(ecKey, certFile);
	if(ret==FAILED)
	{
		printf("%s(%d), readPubKey2Eckey failed.\n",__FUNCTION__,__LINE__);//Added for test
		toRet=FAILED;
		goto err;
	}

	ret=WapiECDSAVerify(msgBuf, msgLen, sig, sigLen, ecKey);
	//printf("%s(%d), ret=%d.\n",__FUNCTION__,__LINE__, ret);//Added for test
//	WapiECDSAVerify(unsigned char*src,int srcLen, unsigned char*signature,unsigned short signatureLen,EC_KEY  *eckey)

	* verifyResult=ret;
		
	toRet=SUCCESS;

err:
	if (ctx) BN_CTX_free(ctx);
	if (x_a) BN_free(x_a);
	if (y_a) BN_free(y_a);
	if (ecKey) EC_KEY_free(ecKey);

	if(msgBuf!=NULL)
		free(msgBuf);
	
	return toRet;
}

#if 1
static void WapiECDHComputeBMK(EC_KEY *a,unsigned char *peerPublicKey,unsigned char peerPublicKeyLength,unsigned char *bkbuf,unsigned char *bklen)
{
	 BIGNUM *x_a=NULL, *y_a=NULL,*x_b=NULL, *y_b=NULL;
	 const EC_GROUP *group;
	 int aout,alen;
	 unsigned char *abuf=NULL;
	 BN_CTX *ctx = NULL;
	 EC_KEY *b=NULL;
	 unsigned char * p;
	 unsigned char  b_pubkey[100],*b1;
	 int			b_pubkey_len;
	 unsigned char  a_pubkey[100],*a1;
	 int			a_pubkey_len;
	 
	 group = EC_KEY_get0_group(a);
	 
//	 TRACE("\nBegin ECDH test!\n");

	 b = EC_KEY_new_by_curve_name(NID_X9_62_prime192v1);

//	 printf("%s(%d)\n",__FUNCTION__,__LINE__);//Added for test
	 ctx = BN_CTX_new();
	 if(ctx ==NULL || b==NULL)
	 { 
	  	TRACE("unable to new ctx and ec key \n");
	  	goto err;
	 }

//	 printf("%s(%d)\n",__FUNCTION__,__LINE__);//Added for test
	 EC_KEY_generate_key(b);
	 p = peerPublicKey;
//	 printf("%s(%d)\n",__FUNCTION__,__LINE__);//Added for test
	 o2i_ECPublicKey(&b,(const unsigned char * * )&p,peerPublicKeyLength);
//	 printf("%s(%d)\n",__FUNCTION__,__LINE__);//Added for test

/*
	 a1 = a_pubkey;
	 a_pubkey_len = i2o_ECPublicKey(a, (unsigned char * * )&a1);

	 printf("%s(%d),a_pubkey_len=%d\n",__FUNCTION__,__LINE__, a_pubkey_len);//Added for test
	 dumpDex(a_pubkey, a_pubkey_len);

	 b1 = b_pubkey;
	 b_pubkey_len = i2o_ECPublicKey(b, (unsigned char * * )&b1);

	 printf("%s(%d),b_pubkey_len=%d\n",__FUNCTION__,__LINE__, b_pubkey_len);//Added for test
	 dumpDex(b_pubkey, b_pubkey_len);
*/
	 	 
	 alen=128;
	 abuf=(unsigned char *)OPENSSL_malloc(alen);
	 if(abuf==NULL)
	 {
//	 	printf("%s(%d)\n",__FUNCTION__,__LINE__);//Added for test
	 	goto err;
	 }
	 aout=ECDH_compute_key(abuf,alen,EC_KEY_get0_public_key(b),a,NULL);

//	printf("%s(%d), aout=%d\n",__FUNCTION__,__LINE__, aout);//Added for test
	 memcpy(bkbuf,abuf,aout);
	 *bklen=(unsigned char)aout;
	 
err:
//	printf("%s(%d)\n",__FUNCTION__,__LINE__);//Added for test
	if(ctx)
		BN_CTX_free(ctx);
	 
	 if(b)
	 	EC_KEY_free(b);
	 
	 if (abuf != NULL) OPENSSL_free(abuf);
//	 printf("%s(%d)\n",__FUNCTION__,__LINE__);//Added for test
 
}
#endif


static int test_ecdh_curve(int nid, const char *text, BN_CTX *ctx)
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
		fprintf(stderr,"Error in ECDH routines\n");
		toRet=FAILED;
		goto err;
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

/*
*    return 0: success; return -1: failed
*/
static int Pem2DerFunc(unsigned char* inFile, unsigned char* outFile)
{
	int i, bufLen, outBufLen, getLen, ret, toRet;
	unsigned char buf[MAX_BUF_LEN];
	unsigned char outBuf[MAX_BUF_LEN];

	//Initial
	bufLen=0;
	outBufLen=0;
	getLen=0;

	memset(buf,0,sizeof(buf));
	memset(outBuf,0,sizeof(outBuf));
	
	ret=readFile2Str(buf, &bufLen, inFile);
	if((ret==FAILED) || (bufLen<=0)||(bufLen>=MAX_BUF_LEN))
	{
		printf("%s(%d), failed here,ret=%d, bufLen=%d\n",__FUNCTION__,__LINE__,ret, bufLen);//Added for test
		toRet=FAILED;
		goto err;
	}

	for(i=0;i<bufLen;i++)
	{
		if (strncmp((char *)&buf[i],"-----BEGIN ",11) == 0)
		{
			WapiGetDataFromPemX509(buf,bufLen,outBuf,&outBufLen,&getLen);
//			printf("%s(%d), bufLen=%d, outBufLen=%d, getLen=%d\n",__FUNCTION__,__LINE__, bufLen,outBufLen,getLen);//Added for test
			break;
		}
	}

	if(outBufLen>0)
	{
		ret=storeStr2File(outFile, outBuf, outBufLen);
		if(ret==FAILED)
		{
			printf("%s(%d), storeStr2File failed.\n",__FUNCTION__,__LINE__);//Added for test
			toRet=FAILED;
			goto err;
		}
	}
	else
	{
		printf("%s(%d), failed here, outBufLen=%d\n",__FUNCTION__,__LINE__,outBufLen);//Added for test
		toRet=FAILED;
		goto err;
	}

	toRet=SUCCESS;
err:
	return toRet;
}


/*
*    return 0: success; return -1: failed
*/
static int Der2PemFunc(unsigned char* inFile, unsigned char* outFile)
{
	int k, l, bufLen, outBufLen, outLen, ret, toRet;
	unsigned char buf[MAX_BUF_LEN];
	unsigned char outBuf[MAX_BUF_LEN];
	EVP_ENCODE_CTX ctx;

	//Initial
	k=0;
	l=0;
	bufLen=0;
	outBufLen=0;
	outLen=0;

	memset(buf,0,sizeof(buf));
	memset(outBuf,0,sizeof(outBuf));
	
	ret=readFile2Str(buf, &bufLen, inFile);
	if((ret==FAILED) || (bufLen<=0)||(bufLen>=MAX_BUF_LEN))
	{
		printf("%s(%d), failed here,ret=%d, bufLen=%d\n",__FUNCTION__,__LINE__,ret, bufLen);//Added for test
		toRet=FAILED;
		goto err;
	}

	/////////////////////
	EVP_EncodeInit(&ctx);
	EVP_EncodeUpdate(&ctx,outBuf,&outBufLen,buf,bufLen);
	if ((outBufLen < 0)||(outBufLen>=MAX_BUF_LEN))
	{
		printf("%s(%d),EVP Encode error, outBufLen=%d \n", __FUNCTION__,__LINE__, outBufLen);
		toRet=FAILED;
		goto err;
	}
	EVP_EncodeFinal(&ctx,&(outBuf[outBufLen]),&k);
	if ((k < 0)||((k+outBufLen)>=MAX_BUF_LEN))
	{
		printf("%s(%d),EVP final  error outBufLen=%d, k=%d  \n", __FUNCTION__,__LINE__, outBufLen, k);
		toRet=FAILED;
		goto err;
	}
	outBufLen+=k;
	/////////////////////////

	if(outBufLen>0)
	{
		//To add cert begin and cert end into cert
		memset(buf,0,sizeof(buf));
		sprintf(buf, "%s", "-----BEGIN CERTIFICATE-----\n");
		outLen=28;
		
		outBuf[outBufLen]=0;
		sprintf(&buf[outLen],"%s", outBuf);
		outLen+=outBufLen;
		
		sprintf(&buf[outLen],"%s", "-----END CERTIFICATE-----\n");
		outLen+=26;
		
		//ret=storeStr2File(outFile, outBuf, outBufLen);
		ret=storeStr2File(outFile, buf, outLen);
		if(ret==FAILED)
		{
			printf("%s(%d), storeStr2File failed.\n",__FUNCTION__,__LINE__);//Added for test
			toRet=FAILED;
			goto err;
		}
	}
	else
	{
		printf("%s(%d), failed here, outBufLen=%d\n",__FUNCTION__,__LINE__,outBufLen);//Added for test
		toRet=FAILED;
		goto err;
	}

	toRet=SUCCESS;
err:
	return toRet;
}
//End for wapi
////////////////////////////////////////////////////
int main(int argc, char *argv[])
	{
	BN_CTX *ctx=NULL;
	int ret=-1;

	char toVerify=0, toSignature=0, toEcdhtest=0,pem2der=0, der2pem=0;
	char *msgFile= NULL, *sigFile=NULL, *keyFile=NULL, *sigOutFile=NULL, *inFile=NULL, *outFile=NULL;
	int verifyResult;
	char tmpBuf[100];

	//printf("%s(%d)\n",__FUNCTION__,__LINE__);//Added for test

	//Initial for wapi signature verify
	sprintf(tmpBuf, "rm -f %s 2>/dev/null", TMP_SIG_VERIFY_RES);
	system(tmpBuf);


	CRYPTO_malloc_debug_init();
	CRYPTO_dbg_set_options(V_CRYPTO_MDEBUG_ALL);
	CRYPTO_mem_ctrl(CRYPTO_MEM_CHECK_ON);

#ifdef OPENSSL_SYS_WIN32
	CRYPTO_malloc_init();
#endif

	RAND_seed(rnd_seed, sizeof rnd_seed);

	if ((ctx=BN_CTX_new()) == NULL) goto err;

	//To parse arguments
	argc--;
	argv++;
	while (argc > 0)
	{
		if ((*argv)[0] != '-') break;

		 if (strcmp(*argv,"-verify") == 0)
		{
			toVerify=1;//To do signature verify
		}
		else	 if (strcmp(*argv,"-signature") == 0)
		{
			toSignature=1;//To do message signature
		}
		else if (strcmp(*argv,"-ecdhtest") == 0)
		{
			toEcdhtest=1;//To do message signature
		}
		else if (strcmp(*argv,"-pem2der") == 0)
		{
			pem2der=1;//To do message signature
		}
		else if (strcmp(*argv,"-der2pem") == 0)
		{
			der2pem=1;//To do message signature
		}
		else
		{
			if((toVerify==1)||(toSignature==1))
			{
				if (strcmp(*argv,"-m") == 0)
				{
					//message file
					if (--argc < 1) break;
					msgFile=*(++argv);
				}
				else if (strcmp(*argv,"-s") == 0)
				{
					//(input)signature file for signature verify
					if (--argc < 1) break;
					sigFile=*(++argv);
				}
				else if (strcmp(*argv,"-k") == 0)
				{
					//signature file
					if (--argc < 1) break;
					keyFile=*(++argv);
				}
				else if (strcmp(*argv,"-o") == 0)
				{
					//(output)signature file for signature
					if (--argc < 1) break;
					sigOutFile=*(++argv);
				}
				else
					break;
			}
			else if((pem2der==1)||(der2pem==1))
			{
				if (strcmp(*argv,"-in") == 0)
				{
					//input file
					if (--argc < 1) break;
					inFile=*(++argv);
				}
				else if (strcmp(*argv,"-out") == 0)
				{
					//outFile
					if (--argc < 1) break;
					outFile=*(++argv);
				}
				else
					break;
			}
			else
				break;
		}
		
		argc--;
		argv++;
	}

#if 0
	if(toVerify==toSignature)
	{
		printf("Error: must use one of -verify and -signature");//Added for test
		goto err;
	}
#endif

	if ((argc > 0) && (argv[0][0] == '-')) /* bad option */
	{
		printf("unknown option '%s'\n",*argv);
		printf("Usage:\n");//Added for test
		printf("ecdsatest <options>\n");//Added for test
		printf("options are\n");//Added for test
		printf("-verify            to do message signature verify\n");//Added for test
		printf("-signature       to do message signature\n");//Added for test
		printf("    -m msgFile      input source message file\n");//Added for test
		printf("    -s sigFile         input source message's signature file -- for signature verify\n");//Added for test
		printf("    -k keyFile        input a wapi cert (X.509 V3 pem format), which contain public key\n");//Added for test
		printf("    -o sigOutFile    output source message's signature -- for signature\n");//Added for test
		printf("-ecdhtest       to do ecdh compute key for wapi bk\n");//Added for test
		printf("-pem2der        to transform wapi cert format from PEM to DER\n");//Added for test
		printf("-der2pem        to transform wapi cert format from DER to PEM\n");//Added for test
		printf("    -in inFile          input file\n");//Added for test
		printf("    -out outFile      output file\n");//Added for test
		goto err;
	}
	//End parse arguments

	if(toSignature==1)
	{
//		printf("%s(%d): toSignature, msgFile=%s, keyFile=%s, TMP_SIG_OUT=%s\n",__FUNCTION__,__LINE__, msgFile, keyFile, TMP_SIG_OUT);//Added for test
		ret=wapiSign(msgFile, keyFile, TMP_SIG_OUT);
		if(ret==FAILED)
		{
			printf("%s(%d): sign failed\n",__FUNCTION__,__LINE__);//Added for test
			goto err;
		}
//		printf("%s(%d): Signature done!\n",__FUNCTION__,__LINE__);//Added for test
	}
	else if(toVerify==1)
	{
//		printf("%s(%d): toVerify, msgFile=%s, sigFile=%s, keyFile=%s\n",__FUNCTION__,__LINE__, msgFile, sigFile, keyFile);//Added for test
		ret=wapiSignVerify(msgFile, sigFile, keyFile, &verifyResult);
//		printf("%s(%d): verify sign, ret=%d, verifyResult=%d\n",__FUNCTION__,__LINE__, ret, verifyResult);//Added for test
		if(ret==FAILED)
		{
			printf("%s(%d): verify sign failed\n",__FUNCTION__,__LINE__);//Added for test
			goto err;
		}
		if(verifyResult==0)
               {
                       sprintf(tmpBuf, "echo \"%d\" > %s", verifyResult, TMP_SIG_VERIFY_RES);
                       system(tmpBuf);
               }
	}
	else if(toEcdhtest==1)
	{
//		printf("%s(%d): toEcdhtest=%d\n",__FUNCTION__,__LINE__,toEcdhtest);//Added for test
		//To do ecdh compute key for wapi bk
		if (test_ecdh_curve(NID_X9_62_prime192v1, "NIST Prime-Curve P-192", ctx)==FAILED)
		{
			printf("%s(%d): test_ecdh_curve failed\n",__FUNCTION__,__LINE__);//Added for test
			goto err;
		}
	}
	else if(pem2der==1)
	{
//		printf("%s(%d): pem2der=%d, inFile=%s, outFile=%s\n",__FUNCTION__,__LINE__,pem2der,inFile, outFile);//Added for test
		ret=Pem2DerFunc(inFile,outFile);
		if(ret==FAILED)
		{
			printf("%s(%d): Pem2DerFunc failed\n",__FUNCTION__,__LINE__);//Added for test
			goto err;
		}
	}
	else if(der2pem==1)
	{
//		printf("%s(%d): der2pem=%d,inFile=%s, outFile=%s\n",__FUNCTION__,__LINE__,der2pem,inFile, outFile);//Added for test
		ret=Der2PemFunc(inFile,outFile);
		if(ret==FAILED)
		{
			printf("%s(%d): Der2PemFunc failed\n",__FUNCTION__,__LINE__);//Added for test
			goto err;
		}
	}
	else
	{
		printf("%s(%d): ecdsates cmd options not support!\n",__FUNCTION__,__LINE__);//Added for test
	}

//	printf("%s(%d): ok!!!!!!!!!!--------\n",__FUNCTION__,__LINE__);//Added for test

	ret = 0;

err:
//	printf("%s(%d)--------\n",__FUNCTION__,__LINE__);//Added for test
	ERR_print_errors_fp(stderr);
	if (ctx) BN_CTX_free(ctx);
	CRYPTO_cleanup_all_ex_data();
	ERR_remove_state(0);
	CRYPTO_mem_leaks_fp(stderr);
	EXIT(ret);
	return(ret);
}
#endif

