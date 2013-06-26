#ifndef _WAPI_CRYPTO_H
#define _WAPI_CRYPTO_H

#include <string.h>
#include <stdio.h>
#include <openssl/x509.h>
#include <openssl/ec.h>
#include <openssl/ecdsa.h>
#include <openssl/ecdh.h>
#include <openssl/objects.h>
#include <openssl/err.h>

#define ENCRYPT  0     
#define DECRYPT  1  

//Added by zj
#define NPROT_COPY_MEM(_pDest, _pSrc, _size)\
	memcpy(_pDest, _pSrc, _size)

#define TRACE printf

typedef int INT;
typedef unsigned int UINT;
typedef unsigned char UCHAR;
typedef unsigned short USHORT;
typedef char BOOLEAN;
typedef unsigned long ULONG;
typedef int HANDLE;

#if 1
void HMAC_SHA256(unsigned char *text, UINT text_len, unsigned char *key, UINT key_len, unsigned char *digest, UINT *digest_len);
void KD_HMAC_SHA256(unsigned char *text,unsigned int text_len,unsigned char *key, unsigned int key_len,unsigned char *output,unsigned int length);

//void WapiGeneratePublicAndPrivateKey(EC_KEY *eckey,unsigned char * publicKey, unsigned char *publicKeyLength);

//void WapiECDSASign(unsigned char*data, int  srcLen,EC_KEY *eckey, unsigned char*out, unsigned short *OutLength);

//int   WapiECDSAVerify(unsigned char*src,int srcLen, unsigned char*signature,unsigned short signatureLen,unsigned char *publicKey,int publicKeyLen);
int   WapiECDSAVerify2(unsigned char*src,int srcLen, unsigned char*signature,unsigned short signatureLen,EC_KEY *eckey);


void WapiECDHComputeBMK(EC_KEY *a,unsigned char *peerPublicKey,unsigned char peerPublicKeyLength,unsigned char *bkbuf,unsigned char *bklen);

 void WapiGetPublicKey(EC_KEY *a, unsigned char *publick_key, size_t public_key_len);

 void testEdch(int nid, unsigned char *peerPublicKey);

 //void WapiGetPubKeyFromCertificate(X509  *x,void *pSecData);

void WAPIX509GetSubjectName(X509 *x, char *buf, int len, unsigned char *contentbuf,int *buflength);

void WAPIX509GetIssuerName(X509 *x, char *buf, int len, unsigned char *contentbuf,int *buflength);
#endif

void dumpDex(const unsigned char * buf, int bufLen);

void dumpStr(const char * buf, int bufLen);

void test_wapi(void);

#endif

