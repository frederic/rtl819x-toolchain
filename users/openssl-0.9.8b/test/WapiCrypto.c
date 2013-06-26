//
//Add for WAPI authentication support 20090219
//

#include <stdio.h>

#if 1
//#include "stdafx.h"
//#include "NdisAdapterNT.h"
//#include "GlobalDef.h"
#include "Wapi.h"
#include "WapiCrypto.h"
//#include "RtlFunc.h"
#include "CustomOid.h"
//#include "Simple_UPnP.H"
//#include <winsock2.h>
//#include "Registry.h"

#include "../crypto/hmac/hmac.h"

void dumpDex(const unsigned char * buf, int bufLen)
{
	int i;

	for(i=0;i<bufLen;i++)
	{
		printf("%2x ",buf[i]);
		if((i>=20)&&(i/20==0))
		{
			printf("\n");
		}
	}
	printf("\n");
}

void dumpStr(const char * buf, int bufLen)
{
	int i;

	for(i=0;i<bufLen;i++)
	{
		printf("%c",buf[i]);
		if((i>=20)&&(i/20==0))
		{
			printf("\n");
		}
	}
	printf("\n");
}

/*********************************************************************
SMS4
*********************************************************************/
#ifndef LITTLE_ENDIAN
#define	 LITTLE_ENDIAN
#endif

const muint8 Sbox[256] = {
0xd6,0x90,0xe9,0xfe,0xcc,0xe1,0x3d,0xb7,0x16,0xb6,0x14,0xc2,0x28,0xfb,0x2c,0x05,
0x2b,0x67,0x9a,0x76,0x2a,0xbe,0x04,0xc3,0xaa,0x44,0x13,0x26,0x49,0x86,0x06,0x99,
0x9c,0x42,0x50,0xf4,0x91,0xef,0x98,0x7a,0x33,0x54,0x0b,0x43,0xed,0xcf,0xac,0x62,
0xe4,0xb3,0x1c,0xa9,0xc9,0x08,0xe8,0x95,0x80,0xdf,0x94,0xfa,0x75,0x8f,0x3f,0xa6,
0x47,0x07,0xa7,0xfc,0xf3,0x73,0x17,0xba,0x83,0x59,0x3c,0x19,0xe6,0x85,0x4f,0xa8,
0x68,0x6b,0x81,0xb2,0x71,0x64,0xda,0x8b,0xf8,0xeb,0x0f,0x4b,0x70,0x56,0x9d,0x35,
0x1e,0x24,0x0e,0x5e,0x63,0x58,0xd1,0xa2,0x25,0x22,0x7c,0x3b,0x01,0x21,0x78,0x87,
0xd4,0x00,0x46,0x57,0x9f,0xd3,0x27,0x52,0x4c,0x36,0x02,0xe7,0xa0,0xc4,0xc8,0x9e,
0xea,0xbf,0x8a,0xd2,0x40,0xc7,0x38,0xb5,0xa3,0xf7,0xf2,0xce,0xf9,0x61,0x15,0xa1,
0xe0,0xae,0x5d,0xa4,0x9b,0x34,0x1a,0x55,0xad,0x93,0x32,0x30,0xf5,0x8c,0xb1,0xe3,
0x1d,0xf6,0xe2,0x2e,0x82,0x66,0xca,0x60,0xc0,0x29,0x23,0xab,0x0d,0x53,0x4e,0x6f,
0xd5,0xdb,0x37,0x45,0xde,0xfd,0x8e,0x2f,0x03,0xff,0x6a,0x72,0x6d,0x6c,0x5b,0x51,
0x8d,0x1b,0xaf,0x92,0xbb,0xdd,0xbc,0x7f,0x11,0xd9,0x5c,0x41,0x1f,0x10,0x5a,0xd8,
0x0a,0xc1,0x31,0x88,0xa5,0xcd,0x7b,0xbd,0x2d,0x74,0xd0,0x12,0xb8,0xe5,0xb4,0xb0,
0x89,0x69,0x97,0x4a,0x0c,0x96,0x77,0x7e,0x65,0xb9,0xf1,0x09,0xc5,0x6e,0xc6,0x84,
0x18,0xf0,0x7d,0xec,0x3a,0xdc,0x4d,0x20,0x79,0xee,0x5f,0x3e,0xd7,0xcb,0x39,0x48
};

const muint32 CK[32] = {
	0x00070e15, 0x1c232a31, 0x383f464d, 0x545b6269,
	0x70777e85, 0x8c939aa1, 0xa8afb6bd, 0xc4cbd2d9,
	0xe0e7eef5, 0xfc030a11, 0x181f262d, 0x343b4249,
	0x50575e65, 0x6c737a81, 0x888f969d, 0xa4abb2b9,
	0xc0c7ced5, 0xdce3eaf1, 0xf8ff060d, 0x141b2229,
	0x30373e45, 0x4c535a61, 0x686f767d, 0x848b9299,
	0xa0a7aeb5, 0xbcc3cad1, 0xd8dfe6ed, 0xf4fb0209,
	0x10171e25, 0x2c333a41, 0x484f565d, 0x646b7279 };

#define Rotl(_x, _y) (((_x) << (_y)) | ((_x) >> (32 - (_y))))

#define ByteSub(_A) (Sbox[(_A) >> 24 & 0xFF] << 24 ^ \
                     Sbox[(_A) >> 16 & 0xFF] << 16 ^ \
                     Sbox[(_A) >>  8 & 0xFF] <<  8 ^ \
                     Sbox[(_A) & 0xFF])

#define L1(_B) ((_B) ^ Rotl(_B, 2) ^ Rotl(_B, 10) ^ Rotl(_B, 18) ^ Rotl(_B, 24))
#define L2(_B) ((_B) ^ Rotl(_B, 13) ^ Rotl(_B, 23))



#if 1

void WapiSMS4ForMNKEncrypt(UCHAR *key,UCHAR *IV,UCHAR *input, UINT inputLength,UCHAR *output, UCHAR *outputLength, UINT CryptFlag)
{
	UINT     	blockNum,i,j;
	UINT	remainder;
	muint8	blockIn[16],blockOut[16];
	muint32	rk[32];
	//int temp;

	*outputLength = 0;
	remainder = inputLength % 16;
	blockNum = inputLength/16;
	if(remainder !=0)
		blockNum++;

	if(remainder !=0)
	{
		for(j= inputLength;j<16*blockNum;j++)
		{
			input[j] = 0;					
		}
	}

//	TRACE("block num %d \n", 	blockNum);
//	TRACE("remainder  %d \n",  remainder);

#if 0	
	TRACE("key \n");
	for(temp=0;temp<16;temp++)
	{
		TRACE("%02x    ",key[temp]);
	}
	TRACE("\nend of key \n");

	TRACE("IV \n");
	for(temp=0;temp<16;temp++)
	{
		TRACE("%02x    ",IV[temp]);
	}
	TRACE("\nend of IV\n");
#endif

	memcpy(blockIn,IV,16);
	SMS4KeyExt((muint8 *)key, rk,CryptFlag);

	for(i=0;i<blockNum;i++)
	{
		//TRACE("=====> %d \n",i);
		SMS4Crypt(blockIn, blockOut, rk);
		*outputLength+=16;

		for(j=0;j<16;j++)
		{
			output[i*16+j] = input[i*16+j] ^ blockOut[j];
		}

		memcpy(blockIn,blockOut,16);
	}
		
}



void SMS4Crypt(muint8 *Input, muint8 *Output, muint32 *rk)
{
	 muint32 r, mid, x0, x1, x2, x3, *p;
	 p = (muint32 *)Input;
	 x0 = p[0];
	 x1 = p[1];
	 x2 = p[2];
	 x3 = p[3];
#ifdef LITTLE_ENDIAN
	 x0 = Rotl(x0, 16); x0 = ((x0 & 0x00FF00FF) << 8) ^ ((x0 & 0xFF00FF00) >> 8);
	 x1 = Rotl(x1, 16); x1 = ((x1 & 0x00FF00FF) << 8) ^ ((x1 & 0xFF00FF00) >> 8);
	 x2 = Rotl(x2, 16); x2 = ((x2 & 0x00FF00FF) << 8) ^ ((x2 & 0xFF00FF00) >> 8);
	 x3 = Rotl(x3, 16); x3 = ((x3 & 0x00FF00FF) << 8) ^ ((x3 & 0xFF00FF00) >> 8);
#endif
	 for (r = 0; r < 32; r += 4)
	 {
		  mid = x1 ^ x2 ^ x3 ^ rk[r + 0];
		  mid = ByteSub(mid);
		  x0 ^= L1(mid);
		  mid = x2 ^ x3 ^ x0 ^ rk[r + 1];
		  mid = ByteSub(mid);
		  x1 ^= L1(mid);
		  mid = x3 ^ x0 ^ x1 ^ rk[r + 2];
		  mid = ByteSub(mid);
		  x2 ^= L1(mid);
		  mid = x0 ^ x1 ^ x2 ^ rk[r + 3];
		  mid = ByteSub(mid);
		  x3 ^= L1(mid);
	 }
#ifdef LITTLE_ENDIAN
	 x0 = Rotl(x0, 16); x0 = ((x0 & 0x00FF00FF) << 8) ^ ((x0 & 0xFF00FF00) >> 8);
	 x1 = Rotl(x1, 16); x1 = ((x1 & 0x00FF00FF) << 8) ^ ((x1 & 0xFF00FF00) >> 8);
	 x2 = Rotl(x2, 16); x2 = ((x2 & 0x00FF00FF) << 8) ^ ((x2 & 0xFF00FF00) >> 8);
	 x3 = Rotl(x3, 16); x3 = ((x3 & 0x00FF00FF) << 8) ^ ((x3 & 0xFF00FF00) >> 8);
#endif
	 p = (muint32 *)Output;
	 p[0] = x3;
	 p[1] = x2;
	 p[2] = x1;
	 p[3] = x0;
}


void SMS4KeyExt(muint8 *Key, muint32 *rk, muint32 CryptFlag)
{
	 muint32 r, mid, x0, x1, x2, x3, *p;
	 p = (muint32 *)Key;
	 x0 = p[0];
	 x1 = p[1];
	 x2 = p[2];
	 x3 = p[3];
#ifdef LITTLE_ENDIAN
	 x0 = Rotl(x0, 16); x0 = ((x0 & 0xFF00FF) << 8) ^ ((x0 & 0xFF00FF00) >> 8);
	 x1 = Rotl(x1, 16); x1 = ((x1 & 0xFF00FF) << 8) ^ ((x1 & 0xFF00FF00) >> 8);
	 x2 = Rotl(x2, 16); x2 = ((x2 & 0xFF00FF) << 8) ^ ((x2 & 0xFF00FF00) >> 8);
	 x3 = Rotl(x3, 16); x3 = ((x3 & 0xFF00FF) << 8) ^ ((x3 & 0xFF00FF00) >> 8);
#endif
	 x0 ^= 0xa3b1bac6;
	 x1 ^= 0x56aa3350;
	 x2 ^= 0x677d9197;
	 x3 ^= 0xb27022dc;
	 for (r = 0; r < 32; r += 4)
	 {
		  mid = x1 ^ x2 ^ x3 ^ CK[r + 0];
		  mid = ByteSub(mid);
		  rk[r + 0] = x0 ^= L2(mid);
		  mid = x2 ^ x3 ^ x0 ^ CK[r + 1];
		  mid = ByteSub(mid);
		  rk[r + 1] = x1 ^= L2(mid);
		  mid = x3 ^ x0 ^ x1 ^ CK[r + 2];
		  mid = ByteSub(mid);
		  rk[r + 2] = x2 ^= L2(mid);
		  mid = x0 ^ x1 ^ x2 ^ CK[r + 3];
		  mid = ByteSub(mid);
		  rk[r + 3] = x3 ^= L2(mid);
	 }
	 if (CryptFlag == DECRYPT)
	 {
	 	  for (r = 0; r < 16; r++)
	 	  	 mid = rk[r], rk[r] = rk[31 - r], rk[31 - r] = mid;
	 }
}


/************************************************************
generate radom number
************************************************************/
#if 0
void
GenerateRandomData(UCHAR * data, UINT len)
{
	 SYSTEMTIME stime;                         
	UINT i, num;

	GetLocalTime(&stime);
	srand((UINT)stime.wSecond%100);

	for (i=0; i<(len/4); i++) {
		num = rand();
		*((UINT *)data) = num;
		data += 4;
	}
	
	i = len % 4;
	if (i > 0) {
		num = rand();
		NPROT_COPY_MEM(data, &num, i);
	}
}
#endif

/****************************************************************
HMAC_SHA256
*****************************************************************/
void HMAC_SHA256(unsigned char *text, UINT text_len, unsigned char *key, UINT key_len, unsigned char *digest, UINT *digest_len)
{
    unsigned int    outlen;
    unsigned char   out[EVP_MAX_MD_SIZE];
    const EVP_MD   *md;
//    int x;
	md= EVP_sha256();
	HMAC(md,key,key_len,text,text_len,out,&outlen);
//              for(x=0;x<outlen;x++)
//                  printf("%02x  ",out[x]);
//                  printf("\n");
	NPROT_COPY_MEM(digest,out,outlen);
	*digest_len=outlen;
}

/******************************************************************
KD_HMAC_SHA256
*******************************************************************/
void KD_HMAC_SHA256(unsigned char *text,unsigned int text_len,unsigned char *key, unsigned int key_len,unsigned char *output,unsigned int length)
{
	unsigned int i;
	unsigned int shalength = SHA256_DIGEST_LENGTH;
	for(i=0;i<length/SHA256_DIGEST_LENGTH;i++,length-=SHA256_DIGEST_LENGTH)
	{
		HMAC_SHA256(text,text_len,key,key_len,&output[i*SHA256_DIGEST_LENGTH],&shalength);
		text = &output[i*SHA256_DIGEST_LENGTH];
		text_len=SHA256_DIGEST_LENGTH;
	}
	if(length>0)
		HMAC_SHA256(text,text_len,key,key_len,&output[i*SHA256_DIGEST_LENGTH],&length);
}
/**********************************************************************

**********************************************************************/
void WapiGeneratePublicAndPrivateKey(EC_KEY *eckey,unsigned char * publicKey, unsigned char *publicKeyLength)
{
	size_t len;
	int result;
	
	result = EC_KEY_generate_key(eckey);
		
	len = EC_POINT_point2oct(EC_KEY_get0_group(eckey), EC_KEY_get0_public_key(eckey), 
			POINT_CONVERSION_COMPRESSED, publicKey,(size_t)(*publicKeyLength), NULL);

 
}
/**************************************************************************
 Input :
    src/srcLen------------the data content/length of calculate the digest
    publicKey/publicKeyLen-------------the public Key for verify
    ecKey---------------------------used to do ECDSA
    out/outLen-----------------------output signature
**************************************************************************/

#if 0
void WapiECDSASign(unsigned char*data, int  srcLen,EC_KEY *eckey, unsigned char*out, unsigned short *OutLength)
{
	 unsigned char digest[32];
	 EC_builtin_curve *curves = NULL;
	 size_t  crv_len = 0, n = 0;
	 EC_GROUP *group;
	 unsigned char *signature = NULL; 
	 unsigned int sig_len;
	 int  nid, ret =  0;
	 
	 SHA256(data,srcLen,digest);
	 
	 crv_len = EC_get_builtin_curves(NULL, 0);
	 curves = (EC_builtin_curve*)OPENSSL_malloc(sizeof(EC_builtin_curve) * crv_len);
	 
	 if (curves == NULL)
	 {
	  	TRACE("curves malloc error\n");
	  	goto builtin_err;
	 }
	 
	 if (!EC_get_builtin_curves(curves, crv_len))
	 {
	  	TRACE("unable to get internal curves\n");
	  	goto builtin_err;
	 }
	 
	 nid = curves[13].nid;
	 group = EC_GROUP_new_by_curve_name(nid);
	  if (group == NULL)
	  {
	  	TRACE("group malloc error \n");
	  	goto builtin_err;
	  }
	 
	 if (EC_KEY_set_group(eckey, group) == 0)
	 {
	 	TRACE("set group error \n");
	   	goto builtin_err;
	 }
	 EC_GROUP_free(group);
	 
	 if (EC_GROUP_get_degree(EC_KEY_get0_group(eckey)) < 160)
	 {
	 	TRACE("get group from key error \n");
	   	goto builtin_err;
	 }
	 
	 sig_len = ECDSA_size(eckey);
	 
	 if ((signature = (unsigned char *)OPENSSL_malloc(sig_len)) == NULL)
	 {
	 	TRACE("Unable to malloc signature \n");
	   	goto builtin_err;
	 }

	 printf("%s(%d)\n",__FUNCTION__,__LINE__);//Added for test
	 if (!ECDSA_sign(0, digest, 32, signature, &sig_len, eckey))
	 {
	   	TRACE(" signature failed \n");
	   	goto builtin_err;
	 }

	 printf("%s(%d),sig_len=%d\n",__FUNCTION__,__LINE__,sig_len);//Added for test
	 
	 memcpy(out,signature,sig_len);
	 *OutLength = sig_len;

	 printf("%s(%d)\n",__FUNCTION__,__LINE__);//Added for test
	 
builtin_err: 
	if(group)
	{
		EC_GROUP_free(group);
		group=NULL;
	}
	 printf("%s(%d)\n",__FUNCTION__,__LINE__);//Added for test
	if (signature)
	{
	  	OPENSSL_free(signature);
		signature=NULL;
	}
	 printf("%s(%d)\n",__FUNCTION__,__LINE__);//Added for test
	if (curves)
	{
	  	OPENSSL_free(curves);
		curves=NULL;
	}
	printf("%s(%d)\n",__FUNCTION__,__LINE__);//Added for test
 
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
int   WapiECDSAVerify(unsigned char*src,int srcLen, unsigned char*signature,unsigned short signatureLen,unsigned char *publicKey,int publicKeyLen)
{
	 unsigned char digestFromPacket[32];
	 unsigned char digestFromECDSA[32];
	 EC_builtin_curve *curves = NULL;
	 size_t crv_len = 0, n = 0;
	 EC_GROUP *group = NULL;
	 int  nid, ret =  0;
	 EC_POINT *srvr_ecpoint = NULL;
	 BN_CTX *bn_ctx = NULL;
	 unsigned char dirt, offset;
	 int result = 0;
	 EC_KEY  *eckey=NULL;

	 //int sig_len;
	 
	 crv_len = EC_get_builtin_curves(NULL, 0);
	 curves = (EC_builtin_curve*)OPENSSL_malloc(sizeof(EC_builtin_curve) * crv_len);
	 if (curves == NULL)
	 {
	  	TRACE("malloc error\n");
	  	goto builtin_err;
	 }
	 if (!EC_get_builtin_curves(curves, crv_len))
	 {
	  	TRACE( "unable to get internal curves\n");
	  	goto builtin_err;
	 }

	 nid = curves[13].nid;
	 group = EC_GROUP_new_by_curve_name(nid);

	 eckey = EC_KEY_new_by_curve_name(nid);
	 if(eckey == NULL)
	 {
	  	TRACE("unable to allocate eckey \n");
		goto builtin_err;
	 }
	 
	 if (group == NULL)
	 {
	 	TRACE("Unable to new group \n");
	   	goto builtin_err;
	 }
	 if (EC_GROUP_get_degree(EC_KEY_get0_group(eckey)) < 160)
	 /* drop the curve */ 
	 {	 	
	 	TRACE("curve error \n");
	  	goto builtin_err;
	 }
	 
	 //set public key
	 srvr_ecpoint = EC_POINT_new(group);
	 bn_ctx = BN_CTX_new();
	 if(srvr_ecpoint == NULL || bn_ctx == NULL)
	 { 
	  	TRACE("unable to new EC_POINT \n");
	  	goto builtin_err;
	 }
	 
	 if(EC_POINT_oct2point(group, srvr_ecpoint,publicKey, publicKeyLen, bn_ctx) == 0)
	 {
	   	TRACE("unable to convert octect to point \n");
	    	goto builtin_err;
	 }
	 
	 if(EC_KEY_set_public_key(eckey, srvr_ecpoint)==0)
	 {
	 	TRACE("unable to EC_KEY_set_public_key \n");
	    	goto builtin_err;
	 }

	 /* check key */
	if (!EC_KEY_check_key(eckey))
	{
		TRACE("EC_KEY_check_key failed \n");
	    	goto builtin_err;
	}

	 //sig_len = ECDSA_size(eckey);
	 //printf("%s(%d),sig_len=%d,signatureLen=%d\n",__FUNCTION__,__LINE__,sig_len,signatureLen);//Added for test
	 
	 //offset = signature[10] % signatureLen;
	 //dirt   = signature[11];
	 //signature[offset] ^= dirt ? dirt : 1; 

#if 1
	SHA256(src,srcLen,digestFromPacket);
	 if (ECDSA_verify(0, digestFromPacket, 32, signature, signatureLen, eckey) != 1)
	 {
	    		TRACE(" failed to verify signature \n");
	  		 result = 1;
	   		goto builtin_err;
	 }

	 result = 0;
#endif

#if 0	 
	 if (ECDSA_verify(0, digestFromECDSA, 32, signature, signatureLen, eckey) != 1)
	 {
	    		TRACE(" failed to verify signature \n");
	  		 result = 1;
	   		goto builtin_err;
	 }
	 
	 SHA256(src,srcLen,digestFromPacket);
	 
	 if(memcmp(digestFromPacket,digestFromECDSA,32))
	 {
	  	result = 1;
	 }
	 else
	 {
	  	result = 0;
	 }
#endif
	 
builtin_err: 
	 if(curves)
	  	OPENSSL_free(curves);
	 if(eckey)
	 	EC_KEY_free(eckey);
	 if(bn_ctx) 
	 	BN_CTX_free(bn_ctx);
	 if(srvr_ecpoint) 
	 	EC_POINT_free(srvr_ecpoint);
	 srvr_ecpoint = NULL;
	 if(group)
	 	EC_GROUP_free(group);
	 return result; 
 
}

 #endif
 
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
int   WapiECDSAVerify2(unsigned char*src,int srcLen, unsigned char*signature,unsigned short signatureLen,EC_KEY *eckey)
{
	 unsigned char digestFromPacket[32];
	 unsigned char digestFromECDSA[32];
	 EC_builtin_curve *curves = NULL;
	 size_t crv_len = 0, n = 0;
	 EC_GROUP *group = NULL;
	 int  nid, ret =  0;
	 //EC_POINT *srvr_ecpoint = NULL;
	// BN_CTX *bn_ctx = NULL;
	 unsigned char dirt, offset;
	 int result = 0;
	 //EC_KEY  *eckey=NULL;

	 //int sig_len;
	 
	 crv_len = EC_get_builtin_curves(NULL, 0);
	 curves = (EC_builtin_curve*)OPENSSL_malloc(sizeof(EC_builtin_curve) * crv_len);
	 if (curves == NULL)
	 {
	  	TRACE("malloc error\n");
	  	goto builtin_err;
	 }
	 if (!EC_get_builtin_curves(curves, crv_len))
	 {
	  	TRACE( "unable to get internal curves\n");
	  	goto builtin_err;
	 }

	 nid = curves[13].nid;
	 group = EC_GROUP_new_by_curve_name(nid);

	 //eckey = EC_KEY_new_by_curve_name(nid);
	 if(eckey == NULL)
	 {
	  	TRACE("unable to allocate eckey \n");
		goto builtin_err;
	 }
	 
	 if (group == NULL)
	 {
	 	TRACE("Unable to new group \n");
	   	goto builtin_err;
	 }
	 if (EC_GROUP_get_degree(EC_KEY_get0_group(eckey)) < 160)
	 /* drop the curve */ 
	 {	 	
	 	TRACE("curve error \n");
	  	goto builtin_err;
	 }
	 
	 //set public key
	 //srvr_ecpoint = EC_POINT_new(group);
	 //bn_ctx = BN_CTX_new();
	// if(srvr_ecpoint == NULL || bn_ctx == NULL)
	 //if(bn_ctx == NULL)
	 //{ 
	  //	TRACE("unable to new EC_POINT \n");
	  	//goto builtin_err;
	 //}

/*
	 if(EC_POINT_oct2point(group, srvr_ecpoint,publicKey, publicKeyLen, bn_ctx) == 0)
	 {
	   	TRACE("unable to convert octect to point \n");
	    	goto builtin_err;
	 }
	 
	 if(EC_KEY_set_public_key(eckey, srvr_ecpoint)==0)
	 {
	 	TRACE("unable to EC_KEY_set_public_key \n");
	    	goto builtin_err;
	 }
*/

	 /* check key */
	if (!EC_KEY_check_key(eckey))
	{
		TRACE("EC_KEY_check_key failed \n");
	    	goto builtin_err;
	}

	 //sig_len = ECDSA_size(eckey);
	 //printf("%s(%d),sig_len=%d,signatureLen=%d\n",__FUNCTION__,__LINE__,sig_len,signatureLen);//Added for test
	 
	 //offset = signature[10] % signatureLen;
	 //dirt   = signature[11];
	 //signature[offset] ^= dirt ? dirt : 1; 

#if 1
	SHA256(src,srcLen,digestFromPacket);
	 if (ECDSA_verify(0, digestFromPacket, 32, signature, signatureLen, eckey) != 1)
	 {
	    		TRACE(" failed to verify signature \n");
	  		 result = 1;
	   		goto builtin_err;
	 }

	 result = 0;
#endif

#if 0	 
	 if (ECDSA_verify(0, digestFromECDSA, 32, signature, signatureLen, eckey) != 1)
	 {
	    		TRACE(" failed to verify signature \n");
	  		 result = 1;
	   		goto builtin_err;
	 }
	 
	 SHA256(src,srcLen,digestFromPacket);
	 
	 if(memcmp(digestFromPacket,digestFromECDSA,32))
	 {
	  	result = 1;
	 }
	 else
	 {
	  	result = 0;
	 }
#endif
	 
builtin_err: 
	 if(curves)
	  	OPENSSL_free(curves);
	// if(eckey)
	 //	EC_KEY_free(eckey);
	// if(bn_ctx) 
	// 	BN_CTX_free(bn_ctx);
	// if(srvr_ecpoint) 
	 //	EC_POINT_free(srvr_ecpoint);
	 //srvr_ecpoint = NULL;
	 if(group)
	 	EC_GROUP_free(group);
	 return result; 
 
}


#if 0
void WapiECDHComputeBMK(EC_KEY *a,unsigned char *peerPublicKey,unsigned char peerPublicKeyLength,unsigned char *bkbuf,unsigned char *bklen)
{
	 BIGNUM *x_a=NULL, *y_a=NULL,*x_b=NULL, *y_b=NULL;
	 const EC_GROUP *group;
	 EC_POINT *srvr_ecpoint = NULL;
	 BN_CTX *bn_ctx = NULL;
	 int aout,alen;
	 unsigned char *abuf=NULL;
	 BN_CTX *ctx = NULL;
	 EC_KEY *b=NULL;
	 
	 group = EC_KEY_get0_group(a);
	 
	 TRACE("\nBegin ECDH test!\n");
	 
	 if ((x_a=BN_new()) == NULL) goto err;
	 if ((y_a=BN_new()) == NULL) goto err;
	 if ((x_b=BN_new()) == NULL) goto err;
	 if ((y_b=BN_new()) == NULL) goto err;

	 b = EC_KEY_new_by_curve_name(NID_X9_62_prime192v1);
	 
	 srvr_ecpoint = EC_POINT_new(group);
	 ctx = BN_CTX_new();
	 bn_ctx = BN_CTX_new();
	 if(srvr_ecpoint == NULL || bn_ctx == NULL ||ctx ==NULL || b==NULL)
	 { 
	  	TRACE("unable to new EC_POINT \n");
	  	goto err;
	 }
	 
	 if(EC_POINT_oct2point(group, srvr_ecpoint,peerPublicKey, peerPublicKeyLength, bn_ctx) == 0)
	 {
	  	TRACE("unable to convert octect to point \n");
	  	goto err;
	 }
	 
	printf("%s(%d)\n",__FUNCTION__,__LINE__);//Added for test
	 EC_KEY_set_public_key(b, srvr_ecpoint);
	 
	 if (EC_METHOD_get_field_type(EC_GROUP_method_of(group)) == NID_X9_62_prime_field) 
	 {
	 	 if (!EC_POINT_get_affine_coordinates_GFp(group,EC_KEY_get0_public_key(a), x_a, y_a, ctx)) 
	  		goto err;
	 }
	 else
	 {
	 	 if (!EC_POINT_get_affine_coordinates_GF2m(group,EC_KEY_get0_public_key(a), x_a, y_a, ctx)) 
	  		goto err;
	 }
	 

	 if (EC_METHOD_get_field_type(EC_GROUP_method_of(group)) == NID_X9_62_prime_field) 
	 {
	  	if (!EC_POINT_get_affine_coordinates_GFp(group, EC_KEY_get0_public_key(b), x_b, y_b, ctx))
			goto err;
	  }
	 else
	 {
	  	if (!EC_POINT_get_affine_coordinates_GF2m(group, EC_KEY_get0_public_key(b), x_b, y_b, ctx)) 
			goto err;
	 }

	 printf("%s(%d)\n",__FUNCTION__,__LINE__);//Added for test
	 alen=128;
	 abuf=(unsigned char *)OPENSSL_malloc(alen);
	 aout=ECDH_compute_key(abuf,alen,EC_KEY_get0_public_key(b),a,NULL);
	 TRACE("finish ECDH compute key, out length = %d\n", aout);

	printf("%s(%d)\n",__FUNCTION__,__LINE__);//Added for test
	 memcpy(bkbuf,abuf,aout);
	 *bklen=(unsigned char)aout;
	 printf("%s(%d)\n",__FUNCTION__,__LINE__);//Added for test
	 
err:
	 ERR_print_errors_fp(stderr);
	 
	 if(bn_ctx) 
	 	BN_CTX_free(bn_ctx);
	if(ctx)
		BN_CTX_free(ctx);
	 
	 if(srvr_ecpoint) 
	 	EC_POINT_free(srvr_ecpoint);
	 srvr_ecpoint = NULL;

	 if(b)
	 	EC_KEY_free(b);
	 
	 if (abuf != NULL) OPENSSL_free(abuf);
	 if (x_a) BN_free(x_a);
	 if (y_a) BN_free(y_a);
	 if (x_b) BN_free(x_b);
	 if (y_b) BN_free(y_b);

	 printf("%s(%d)\n",__FUNCTION__,__LINE__);//Added for test
 
}
#endif

#if 1
void WapiECDHComputeBMK(EC_KEY *a,unsigned char *peerPublicKey,unsigned char peerPublicKeyLength,unsigned char *bkbuf,unsigned char *bklen)
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


 void WapiGetPublicKey(EC_KEY *a, unsigned char *publick_key, size_t public_key_len)
{

	size_t len;
	len = EC_POINT_point2oct(EC_KEY_get0_group(a), EC_KEY_get0_public_key(a), 
			POINT_CONVERSION_COMPRESSED, publick_key,public_key_len, NULL);
}

/*
void testEdch(int nid, unsigned char *peerPublicKey)
{
 EC_KEY *src=NULL;
 EC_KEY *peer=NULL;
 
 src = EC_KEY_new_by_curve_name(nid);
 peer = EC_KEY_new_by_curve_name(nid);
 
 EC_KEY_generate_key(src);
 ECDHCompute(nid,src,peer,peerPublicKey);
 
 if (peer) EC_KEY_free(peer);
 if (src) EC_KEY_free(src);
}*/
 
 
#if 0
void WapiGetPubKeyFromCertificate(X509  *x,void *pSecData)
{
	 EVP_PKEY *pkey=NULL;
	 int  i;
	 unsigned char buf[512];
	 int n;
	 PWAPI_SEC_DATA      pPubkey;
	 const EC_POINT *public_key;
	 BIGNUM  *pub_key=NULL;
	 BN_CTX  *ctx=NULL;
	 const EC_GROUP *group;

	 if ((ctx=BN_CTX_new()) == NULL) 
	 {
	 	TRACE("non ctx \n");
	 	goto err;
	 }

	 pPubkey=(PWAPI_SEC_DATA)pSecData;
	 
	 pkey=X509_get_pubkey(x);
	 if (pkey == NULL)
	 {
	  	TRACE("Unable to load Public Key\n");
		goto err;
	 }
	 else
	 if (pkey->type == EVP_PKEY_RSA)
	 {
		   printf("RSA Public Key: \n");
		   BN_num_bits(pkey->pkey.rsa->n);
		       
		   n=BN_bn2bin(pkey->pkey.rsa->n,&buf[0]);
		 
		   TRACE("bytes number %d \n",n);
		 
		   for(i=0;i<n;i++)
		   {
		    TRACE("%02x   ",buf[i]);
		   }
	 }
	 else 
	 if (pkey->type == EVP_PKEY_EC)
	 {
		public_key = EC_KEY_get0_public_key( pkey->pkey.ec);
		group = EC_KEY_get0_group(pkey->pkey.ec);
		pub_key = EC_POINT_point2bn(group, public_key,EC_KEY_get_conv_form(pkey->pkey.ec), NULL, ctx);
		n=BN_bn2bin(pub_key,&buf[0]);
		TRACE("bytes number %d \n",n);
		for(i=0;i<n;i++)
		{
			TRACE("%02x   ",buf[i]);
		}

	 }
	 else
	 printf("Unknown Public Key:\n");

	 pPubkey->Length=n;
	 memcpy( pPubkey->Content,buf,n);	

err:
	if (ctx) 
		BN_CTX_free(ctx);
	if(pkey)
	 	EVP_PKEY_free(pkey);
	 
 
}
#endif

void 
WAPIX509GetIssuerName(X509 *x, char *buf, int len, unsigned char *contentbuf,int *buflength)
{
	 X509_NAME_ENTRY *ne;
	 int i;
	 int n,l1,num,type;
	 const char *s;
	 unsigned char *q;
	 BUF_MEM *b=NULL;
	 	 
	 char tmp_buf[80];
	 int count = 0;
	 char  out[1000];
	 
	 memset(out,0,1000);
	 
	 if (buf == NULL)
	  {
	  if ((b=BUF_MEM_new()) == NULL) goto err;
	  if (!BUF_MEM_grow(b,200)) goto err;
	  b->data[0]='\0';
	  len=200;
	  }
	 if (X509_get_issuer_name(x) == NULL)
	 {
	     if(b)
	  {
	  buf=b->data;
	  OPENSSL_free(b);
	  }
	     }
	 
	 for (i=0; i<sk_X509_NAME_ENTRY_num(X509_get_issuer_name(x)->entries); i++)
	  {
	  ne=sk_X509_NAME_ENTRY_value(X509_get_issuer_name(x)->entries,i);
	  n=OBJ_obj2nid(ne->object);
	  if ((n == NID_undef) || ((s=OBJ_nid2sn(n)) == NULL))
	   {
	   i2t_ASN1_OBJECT(tmp_buf,sizeof(tmp_buf),ne->object);
	   s=tmp_buf;
	   }
	  l1=strlen(s);
	 
	  type=ne->value->type;
	  num=ne->value->length;
	  q=ne->value->data;
	 
	  
	  memcpy(out+count,s,strlen(s));
	  count +=strlen(s);
	 
	  out[count] = '=';
	  count++;
	 
	  memcpy(out+count,ne->value->data,ne->value->length);
	  count +=ne->value->length;
	 
	 
	  
	  }

	 *buflength=count;
	 memcpy(contentbuf,out,count);
	 err:
	 X509err(X509_F_X509_NAME_ONELINE,ERR_R_MALLOC_FAILURE);
	 if (b != NULL) BUF_MEM_free(b);
}


void 
WAPIX509GetSubjectName(X509 *x, char *buf, int len, unsigned char *contentbuf,int *buflength)
{
	 X509_NAME_ENTRY *ne;
	 int i;
	 int n,l1,num,type;
	 const char *s;
	 unsigned char *q;
	 BUF_MEM *b=NULL;
	 	 
	 char tmp_buf[80];
	 int count = 0;
	 char  out[1000];
	 
	 memset(out,0,1000);
	 
	 if (buf == NULL)
	  {
	  if ((b=BUF_MEM_new()) == NULL) goto err;
	  if (!BUF_MEM_grow(b,200)) goto err;
	  b->data[0]='\0';
	  len=200;
	  }
	 if (X509_get_subject_name(x) == NULL)
	 {
	     if(b)
	  {
	  buf=b->data;
	  OPENSSL_free(b);
	  }
	     }
	 
	 for (i=0; i<sk_X509_NAME_ENTRY_num(X509_get_subject_name(x)->entries); i++)
	  {
	  ne=sk_X509_NAME_ENTRY_value(X509_get_subject_name(x)->entries,i);
	  n=OBJ_obj2nid(ne->object);
	  if ((n == NID_undef) || ((s=OBJ_nid2sn(n)) == NULL))
	   {
	   i2t_ASN1_OBJECT(tmp_buf,sizeof(tmp_buf),ne->object);
	   s=tmp_buf;
	   }
	  l1=strlen(s);
	 
	  type=ne->value->type;
	  num=ne->value->length;
	  q=ne->value->data;
	 
	  
	  memcpy(out+count,s,strlen(s));
	  count +=strlen(s);
	 
	  out[count] = '=';
	  count++;
	 
	  memcpy(out+count,ne->value->data,ne->value->length);
	  count +=ne->value->length;
	 
	 
	  
	  }

	 *buflength=count;
	 memcpy(contentbuf,out,count);
	 err:
	 X509err(X509_F_X509_NAME_ONELINE,ERR_R_MALLOC_FAILURE);
	 if (b != NULL) BUF_MEM_free(b);
}
#endif

void test_wapi()
{
	printf("######%s(%d)#######\n",__FUNCTION__,__LINE__);//Added for test
}



#endif
