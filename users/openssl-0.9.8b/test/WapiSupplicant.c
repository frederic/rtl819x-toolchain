//
//Add for WAPI authentication support 20090219
//


//#include "stdafx.h"
//#include "NdisAdapterNT.h"
//#include "GlobalDef.h"
#include "WapiCrypto.h"
#include "Wapi.h"
//#include "RtlFunc.h"
#include "CustomOid.h"
//#include "Simple_UPnP.H"
//#include <winsock2.h>
//#include "Registry.h"
//#include "Wsc.h"
#include <openssl/ec.h>
#include <openssl/x509.h>
#include <openssl/pem.h>

#if !defined __int64
#  define __int64 long long
#endif   

typedef   unsigned int   u32;
typedef   unsigned __int64  u64;
typedef   unsigned char		u8;

struct sha256_state {
	u64 length;
	u32 state[8], curlen;
	unsigned char buf[64];
};

const unsigned long K[64] = {
	0x428a2f98UL, 0x71374491UL, 0xb5c0fbcfUL, 0xe9b5dba5UL, 0x3956c25bUL,
	0x59f111f1UL, 0x923f82a4UL, 0xab1c5ed5UL, 0xd807aa98UL, 0x12835b01UL,
	0x243185beUL, 0x550c7dc3UL, 0x72be5d74UL, 0x80deb1feUL, 0x9bdc06a7UL,
	0xc19bf174UL, 0xe49b69c1UL, 0xefbe4786UL, 0x0fc19dc6UL, 0x240ca1ccUL,
	0x2de92c6fUL, 0x4a7484aaUL, 0x5cb0a9dcUL, 0x76f988daUL, 0x983e5152UL,
	0xa831c66dUL, 0xb00327c8UL, 0xbf597fc7UL, 0xc6e00bf3UL, 0xd5a79147UL,
	0x06ca6351UL, 0x14292967UL, 0x27b70a85UL, 0x2e1b2138UL, 0x4d2c6dfcUL,
	0x53380d13UL, 0x650a7354UL, 0x766a0abbUL, 0x81c2c92eUL, 0x92722c85UL,
	0xa2bfe8a1UL, 0xa81a664bUL, 0xc24b8b70UL, 0xc76c51a3UL, 0xd192e819UL,
	0xd6990624UL, 0xf40e3585UL, 0x106aa070UL, 0x19a4c116UL, 0x1e376c08UL,
	0x2748774cUL, 0x34b0bcb5UL, 0x391c0cb3UL, 0x4ed8aa4aUL, 0x5b9cca4fUL,
	0x682e6ff3UL, 0x748f82eeUL, 0x78a5636fUL, 0x84c87814UL, 0x8cc70208UL,
	0x90befffaUL, 0xa4506cebUL, 0xbef9a3f7UL, 0xc67178f2UL
};


/* Various logical functions */
#define RORc(x, y) \
( ((((unsigned long) (x) & 0xFFFFFFFFUL) >> (unsigned long) ((y) & 31)) | \
   ((unsigned long) (x) << (unsigned long) (32 - ((y) & 31)))) & 0xFFFFFFFFUL)
#define Ch(x,y,z)       (z ^ (x & (y ^ z)))
#define Maj(x,y,z)      (((x | y) & z) | (x & y)) 
#define S(x, n)         RORc((x), (n))
#define R(x, n)         (((x)&0xFFFFFFFFUL)>>(n))
#define Sigma0(x)       (S(x, 2) ^ S(x, 13) ^ S(x, 22))
#define Sigma1(x)       (S(x, 6) ^ S(x, 11) ^ S(x, 25))
#define Gamma0(x)       (S(x, 7) ^ S(x, 18) ^ R(x, 3))
#define Gamma1(x)       (S(x, 17) ^ S(x, 19) ^ R(x, 10))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define WPA_GET_BE32(a) ((((u32) (a)[0]) << 24) | (((u32) (a)[1]) << 16) | \
			 (((u32) (a)[2]) << 8) | ((u32) (a)[3]))

#define WPA_PUT_BE32(a, val)				\
	do {						\
		(a)[0] = (u8) (((u32) (val)) >> 24);	\
		(a)[1] = (u8) (((u32) (val)) >> 16);	\
		(a)[2] = (u8) (((u32) (val)) >> 8);	\
		(a)[3] = (u8) (((u32) (val)) & 0xff);	\
	} while (0)

#define WPA_PUT_BE64(a, val)				\
	do {						\
		(a)[0] = (u8) (((u64) (val)) >> 56);	\
		(a)[1] = (u8) (((u64) (val)) >> 48);	\
		(a)[2] = (u8) (((u64) (val)) >> 40);	\
		(a)[3] = (u8) (((u64) (val)) >> 32);	\
		(a)[4] = (u8) (((u64) (val)) >> 24);	\
		(a)[5] = (u8) (((u64) (val)) >> 16);	\
		(a)[6] = (u8) (((u64) (val)) >> 8);	\
		(a)[7] = (u8) (((u64) (val)) & 0xff);	\
	} while (0)

int sha256_compress(struct sha256_state *md, unsigned char *buf)
{
	u32 S[8], W[64], t0, t1;
	u32  t;
	int i;

	/* copy state into S */
	for (i = 0; i < 8; i++) {
		S[i] = md->state[i];
	}

	/* copy the state into 512-bits into W[0..15] */
	for (i = 0; i < 16; i++)
		W[i] = WPA_GET_BE32(buf + (4 * i));

	/* fill W[16..63] */
	for (i = 16; i < 64; i++) {
		W[i] = Gamma1(W[i - 2]) + W[i - 7] + Gamma0(W[i - 15]) +
			W[i - 16];
	}        

	/* Compress */
#define RND(a,b,c,d,e,f,g,h,i)                          \
	t0 = h + Sigma1(e) + Ch(e, f, g) + K[i] + W[i];	\
	t1 = Sigma0(a) + Maj(a, b, c);			\
	d += t0;					\
	h  = t0 + t1;

	for (i = 0; i < 64; ++i) {
		RND(S[0], S[1], S[2], S[3], S[4], S[5], S[6], S[7], i);
		t = S[7]; S[7] = S[6]; S[6] = S[5]; S[5] = S[4]; 
		S[4] = S[3]; S[3] = S[2]; S[2] = S[1]; S[1] = S[0]; S[0] = t;
	}

	/* feedback */
	for (i = 0; i < 8; i++) {
		md->state[i] = md->state[i] + S[i];
	}
	return 0;
}




/* Initialize the hash state */
void sha256_init(struct sha256_state *md)
{
	md->curlen = 0;
	md->length = 0;
	md->state[0] = 0x6A09E667UL;
	md->state[1] = 0xBB67AE85UL;
	md->state[2] = 0x3C6EF372UL;
	md->state[3] = 0xA54FF53AUL;
	md->state[4] = 0x510E527FUL;
	md->state[5] = 0x9B05688CUL;
	md->state[6] = 0x1F83D9ABUL;
	md->state[7] = 0x5BE0CD19UL;
}


static int sha256_process(struct sha256_state *md, const unsigned char *in,
			  unsigned long inlen)
{
	unsigned long n;
#define block_size 64

	if (md->curlen > sizeof(md->buf))
		return -1;

	while (inlen > 0) {
		if (md->curlen == 0 && inlen >= block_size) {
			if (sha256_compress(md, (unsigned char *) in) < 0)
				return -1;
			md->length += block_size * 8;
			in += block_size;
			inlen -= block_size;
		} else {
			n = MIN(inlen, (block_size - md->curlen));
			memcpy(md->buf + md->curlen, in, n);
			md->curlen += n;
			in += n;
			inlen -= n;
			if (md->curlen == block_size) {
				if (sha256_compress(md, md->buf) < 0)
					return -1;
				md->length += 8 * block_size;
				md->curlen = 0;
			}
		}
	}

	return 0;
}

int sha256_done(struct sha256_state *md, unsigned char *out)
{
	int i;

	if (md->curlen >= sizeof(md->buf))
		return -1;

	/* increase the length of the message */
	md->length += md->curlen * 8;

	/* append the '1' bit */
	md->buf[md->curlen++] = (unsigned char) 0x80;

	/* if the length is currently above 56 bytes we append zeros
	 * then compress.  Then we can fall back to padding zeros and length
	 * encoding like normal.
	 */
	if (md->curlen > 56) {
		while (md->curlen < 64) {
			md->buf[md->curlen++] = (unsigned char) 0;
		}
		sha256_compress(md, md->buf);
		md->curlen = 0;
	}

	/* pad upto 56 bytes of zeroes */
	while (md->curlen < 56) {
		md->buf[md->curlen++] = (unsigned char) 0;
	}

	/* store length */
	WPA_PUT_BE64(md->buf + 56, md->length);
	sha256_compress(md, md->buf);

	/* copy output */
	for (i = 0; i < 8; i++)
		WPA_PUT_BE32(out + (4 * i), md->state[i]);

	return 0;
}


/**
 * sha256_vector - SHA256 hash for data vector
 * @num_elem: Number of elements in the data vector
 * @addr: Pointers to the data areas
 * @len: Lengths of the data blocks
 * @mac: Buffer for the hash
 */
void sha256_vector(size_t num_elem, const unsigned char **addr, int *len,
		unsigned char *mac)
{
	struct sha256_state ctx;
	size_t i;

	sha256_init(&ctx);
	for (i = 0; i < num_elem; i++)
		sha256_process(&ctx, addr[i], len[i]);
	sha256_done(&ctx, mac);
}


void X509GetNamet(unsigned char*x509DerBuf, int x509Len, unsigned char * IssueName,int *issueNameLen,unsigned char*serialNum,int *serialNumLen,unsigned char*subjectName, int*subjectNameLen)
{
	int getLen = 0;
	int serialNumLength, issuerNameLength,subjectNameLength;
	
	
}


void getPubKey(X509  *x)
{
	EVP_PKEY *pkey=NULL;
	int		i;
	unsigned char	buf[512];
	int n;
	const EC_POINT *public_key;
	BIGNUM  *pub_key=NULL;
	BN_CTX  *ctx=NULL;
	const EC_GROUP *group;

	pkey = EVP_PKEY_new();
	
	pkey=X509_get_pubkey(x);
	if (pkey == NULL)
	{
		printf("%12sUnable to load Public Key\n");
	}
	else
	if (pkey->type == EVP_PKEY_RSA)
	{
			printf("RSA Public Key: \n");
			BN_num_bits(pkey->pkey.rsa->n);
							
			n=BN_bn2bin(pkey->pkey.rsa->n,&buf[0]);

			printf("bytes number %d \n",n);

			for(i=0;i<n;i++)
			{
				printf("%02x   ",buf[i]);
			}


			//RSA_print(bp,pkey->pkey.rsa,16);
	}
	else 
	if (pkey->type == EVP_PKEY_EC)
	{
		public_key = EC_KEY_get0_public_key( pkey->pkey.ec);
		group = EC_KEY_get0_group(pkey->pkey.ec);
		pub_key = EC_POINT_point2bn(group, public_key,EC_KEY_get_conv_form(pkey->pkey.ec), NULL, ctx);
		n=BN_bn2bin(pub_key,&buf[0]);
		printf("bytes number %d \n",n);
		for(i=0;i<n;i++)
		{
			printf("%02x   ",buf[i]);
		}

	}
	else
		printf("%12sUnknown Public Key:\n");
	EVP_PKEY_free(pkey);

}

void getSerialNum(X509  *x
)

{
	ASN1_INTEGER *bs;
	long l;

	printf("get Serial Num \n");
	
	bs=X509_get_serialNumber(x);

	printf("length %d \n",bs->length);

	for(l=0;l<bs->length;l++)
		printf("%02X	",*(bs->data+l));
	
}


void getDataFromCert(unsigned char *buf,int len,unsigned char *x509Buf,int *x509Len,int *x509GetLen)
{
	unsigned char   buf_x509[1000];
	int			x509_len;
	int			bOK;
	EVP_ENCODE_CTX ctx;
	int			k,l,m,i,j;
	for(i=11;i<len;)
	{
			
		i++;
		bOK = 0;
		x509_len = 0;
		if (strncmp(&buf[i],"-----\n",6) == 0)
		{
			printf("1111111111111  %d \n",i);
			for(j=i+6;j<len;j++)
			{
				buf_x509[x509_len] = buf[j]; 
				x509_len ++;
				if(strncmp(&buf[j],"-----END ",9) == 0)
				{
						printf("333333333333 \n");
						printf("%s  \n",buf_x509);
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
			printf("OUT OUT \n");
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

	printf("x509_len in sub function %d \n",x509_len);

	*x509Len = x509_len;
	memcpy(x509Buf,buf_x509,x509_len);

	

}

#if 0
//return 0: success
//return -1: error
int    ReadCertificate(const char * certName, BIO * out)
{
	X509                    *x;
    FILE                    *fp;
	unsigned char			buf[5000], *p,*q;
    int                     len,ret,j;
    //BIO                      *b;
	int  i,len1;
EVP_PKEY *pkey=NULL;
X509_CINF *ci;
unsigned char			buf1[500];
ASN1_BIT_STRING		key[1000];
	unsigned char 	temp[1000];
	EC_KEY			*eckey = NULL;
	int		klen;
	char  psbuf[PEM_BUFSIZE];
	char name[1000],header[1000],*p2,*p3,*p4;
	BIGNUM    * priv_key;

	EC_GROUP  * group;

	EC_POINT   *pub_key;

	BIGNUM  *public_key=NULL;

	unsigned char   buf_x509[1000];
	int			x509_len;
	int			bOK;
	EVP_ENCODE_CTX ctx;
	int			k,l,m;
	int			x509_get_len;
	unsigned char  *pX509;
	unsigned char   buf_x5091[1000];
	int			x509_len1;
	int			x509_get_len1;
	char			namebuf[300];
	int			nameLen;

	unsigned char	 src[689];
	int			srcLen;
	unsigned char  sha256_out1[32],sha256_out2[32];

	WAPI_SEC_DATA      pubKey;

	for(i=0;i<689;i++)
		src[i] = i+1;
	srcLen = 689;
	p=src;

#if 0
	sha256_vector(1,&p,&srcLen,sha256_out1);
	SHA256(src,689,sha256_out2);

	printf("sha2561out\n");
	for(i=0;i<32;i++)
	{
		printf("%02x   ",sha256_out1[i]);
		if((i+1) % 16==0)
			printf("\n");
	}

	printf("sha2562out\n");
	for(i=0;i<32;i++)
	{
		printf("%02x   ",sha256_out2[i]);
		if((i+1) % 16==0)
			printf("\n");
	}
#endif
	
#if 1

    /* example.cer为DER编码的数字证书	*/

	fp = fopen(certName, "r");
	if (!fp) 
	{
		printf("open file fail!\n");	
		return -1;
	}
	
	printf("open file ok!\n");

#if 0
	len = fread(buf, 1, 5000, fp);

	x509_len = 0;
	for(i=0;i<len;)
	if (strncmp(buf,"-----BEGIN ",11) == 0)
	{
		printf("---------------============\n");
		getDataFromCert(buf,len,buf_x509,&x509_len,&x509_get_len);
		break;
		
	}

	printf("x509_len %d \n",x509_len);
	printf("x509_get_len %d \n",x509_get_len);

	for(i=0;i<x509_len;i++)
	{
		printf("%02x  ",buf_x509[i]);
		if((i+1) % 16 == 0)
			printf("\n");
	}

	for(i=x509_get_len;i<len;i++)
	{
		if (strncmp(&buf[i],"-----BEGIN ",11) == 0)
		{
			printf("---------------============\n");
			pX509 = &buf[i];
			getDataFromCert(pX509,(len-x509_get_len),buf_x5091,&x509_len1,&x509_get_len1);
			break;
		}
	}

	printf("x509_len  1  %d \n",x509_len1);
	printf("x509_get_len %d \n",x509_get_len1);

	for(i=0;i<x509_len1;i++)
	{
		printf("%02x  ",buf_x5091[i]);
		if((i+1) % 16 == 0)
			printf("\n");
	}


	//b=BIO_new(BIO_s_file());
	// 关联BIO结构跟相关的file
//	BIO_set_fp(b,stdout,BIO_NOCLOSE);


	// 为X509 struct分配空间

	
	// 将buf的数据存到X509 (DER to Internal)
#endif

#endif

#if 1
	x = X509_new();
	if(x==NULL)
	{
		printf("%s(%d): X509_new is null.\n",__FUNCTION__,__LINE__);//Added for test
		return -1;
	}
	
	x = PEM_read_X509(fp, NULL, 0, NULL);

	if(x==NULL)
	{
		printf("%s(%d): PEM_read_X509 return null.\n",__FUNCTION__,__LINE__);//Added for test
		return -1;
	}
	//X509_print(out,x);//Added for test

	WapiGetPubKeyFromCertificate(x,(void *)&pubKey);
	printf("%s(%d): pub_key.Length=%d\n",__FUNCTION__,__LINE__,pubKey.Length);//Added for test
//	dumpStr(pubKey.Content,pubKey.Length);
//	printf("%s(%d): pub_key.Length=%d\n",__FUNCTION__,__LINE__,pubKey.Content);//Added for test
	printf("%s(%d)\n",__FUNCTION__,__LINE__);//Added for test

	

	p2 = name;
	p3 = header;

	printf("%s(%d)\n",__FUNCTION__,__LINE__);//Added for test
	PEM_read(fp,&p2,&p3,&p,(long *)&len);
	printf("%s(%d),len=%d\n",__FUNCTION__,__LINE__,len);//Added for test

#if 0
	eckey = EC_KEY_new_by_curve_name(NID_X9_62_prime192v1);
	group = EC_KEY_get0_group(eckey);

	if (EC_KEY_get0_private_key(eckey) == NULL)
	{
		printf("%s(%d),EC_KEY_get0_private_key return null.\n",__FUNCTION__,__LINE__);//Added for test
		priv_key = BN_new();
		if (priv_key == NULL)
			goto err;
	}
	else
	{

		priv_key = EC_KEY_get0_private_key(eckey);
        }

        if (!BN_bin2bn(p+7, 24, priv_key))
		goto err;

	BN_print(out,priv_key);
	printf("\n");

#if 0
	for(i=0;i<6;i++)
		printf("%x   ",priv_key->d[i]);
	
	printf("\n");
#endif

	//p=temp;

	//klen = i2d_ECPrivateKey(eckey,&p);

	//klen = i2o_ECPublicKey(eckey,&p);

	//for(i=0;i<klen;i++)
	//	printf("%02x  ",temp[i]);

	//printf("klen  %d \n",klen);

#if 0
	printf("1111111%s  \n",p2);

	printf("22222222%s  \n",p3);

	for(i=0;i<len;i++)
		printf("%02x  ",p[i]);


	printf("len %d \n",len);



	printf("decode file ok!\n");

	

	

	// 以X509的格式打印
	//ret=X509_print(b,x);
//test
	printf("tttttttttttttt \n");
	//getSerialNum(x);
	//X509GetNamet(X509_get_subject_name(x),NULL,0);
	//getPubKey(x);
#endif

	ci=x->cert_info;

	q=buf1;

	if(ci->key->public_key != NULL)
	{
		printf("%s(%d), ci->key->public_key is not null\n",__FUNCTION__,__LINE__);//Added for test
		len1 = i2c_ASN1_BIT_STRING(ci->key->public_key,&q);
	}

	if(EC_KEY_get0_public_key(eckey) == NULL)
	{
		printf("public key == NULL \n");
		pub_key = EC_POINT_new(group);
		if(pub_key == NULL)
			goto err;
	}
	else
	{
	 	printf("get pub key \n");
		pub_key = EC_KEY_get0_public_key(eckey);
	}


	EC_KEY_set_public_key(eckey, pub_key);

	q = buf1+1;

	o2i_ECPublicKey(&eckey, (const unsigned char * * )&q, len1-1);

	for(i=0;i<50;i++)
		temp[i] = 0;


	p = temp;

	klen = i2o_ECPublicKey(eckey, &p);


	printf("p  addr  %x  \n",p);

	printf("temp addr %x \n",temp);
	
	printf("klen =============== %d \n",klen);

	for(i=0;i<klen;i++)
		printf("%02x  ",temp[i]);

	printf("\n");


	//len = i2d_X509(x,&p);

#if 0
	printf("len %d \n",len);

	for(i=0;i<len;i++)
	{
		printf("%02x  ",buf[i]);
		if((i+1) % 16 == 0)
			printf("\n");
	}

	printf("len1 %d \n",len1);
#endif

#endif	

err:	
#endif
	//BIO_free(b);

	X509_free(x);

	fclose(fp);
	return 0;
}
#endif

//return 0: success
//return -1: failed
int wapiCertVerify0(const char * user_cer,const char * as_cer,BIO * out)
{
	X509 *x=NULL;
	X509 *x_user=NULL;
	FILE *fp=NULL;
	FILE *fp_user=NULL;

	EVP_PKEY *pkey=NULL;

	int ret,res_verify;

	//To get as public key from as_cer file
	fp = fopen(as_cer, "r");
	if (!fp) 
	{
		printf("open as_cer file fail!\n");	//Added for test
		ret=-1;
		goto err;
	}
	
	printf("open as_cer file ok!\n");//Added for test

	x = X509_new();
	if(x==NULL)
	{
		printf("%s(%d): x X509_new is null.\n",__FUNCTION__,__LINE__);//Added for test
		ret=-1;
		goto err;
	}
	
	x = PEM_read_X509(fp, NULL, 0, NULL);
	if(x==NULL)
	{
		printf("%s(%d): x PEM_read_X509 return null.\n",__FUNCTION__,__LINE__);//Added for test
		ret=-1;
		goto err;
	}
	X509_print(out,x);//Added for test

	pkey=X509_get_pubkey(x);
	if (pkey == NULL)
	 {
	  	printf("Unable to load Public Key\n");//Added for test
		goto err;
	 }

	//To load user_cer as x509
	fp_user= fopen(user_cer, "r");
	if (!fp_user) 
	{
		printf("open user_cer file fail!\n");	//Added for test
		ret=-1;
		goto err;
	}
	
	printf("open user_cer file ok!\n");//Added for test

	x_user= X509_new();
	if(x_user==NULL)
	{
		printf("%s(%d): x_user X509_new is null.\n",__FUNCTION__,__LINE__);//Added for test
		ret=-1;
		goto err;
	}
	
	x_user = PEM_read_X509(fp_user, NULL, 0, NULL);
	if(x_user==NULL)
	{
		printf("%s(%d): x_user PEM_read_X509 return null.\n",__FUNCTION__,__LINE__);//Added for test
		ret=-1;
		goto err;
	}
	X509_print(out,x_user);//Added for test

	res_verify=X509_verify(x_user, pkey);//return 1: success; return 0: failed
	printf("%s(%d): res_verify=%d.\n",__FUNCTION__,__LINE__,res_verify);//Added for test
	if(res_verify!=1)
	{
		printf("%s(%d): X509_verify failed.\n",__FUNCTION__,__LINE__);//Added for test
		ret=-1;
		goto err;
	}

	ret=0;
	
err:
	if(pkey)
	 	EVP_PKEY_free(pkey);
	
	if(x)
		X509_free(x);

	if(fp)
		fclose(fp);

	if(x_user)
		X509_free(x_user);

	if(fp_user)
		fclose(fp_user);
	
	return ret;
}


#if 0
//return 0: success
//return -1: failed
int wapiCertVerify(const char * user_cer,const char * as_cer,BIO * out)
{
	X509 *x=NULL;
	X509 *x_user=NULL;
	FILE *fp=NULL;
	FILE *fp_user=NULL;
	int i;

#if 0
	EVP_PKEY *pkey=NULL;
#endif
	WAPI_SEC_DATA      pubKey;

	int ret,res_verify;

	char name[1000],header[1000],src[1000],*p2,*p3,*p;
	char *srcH, *sigH;
	unsigned short srcLen,sigLen;
	int len;
	int nameLen,headerLen;
	

#if 1
	//To get as public key from as_cer file
	fp = fopen(as_cer, "r");
	if (!fp) 
	{
		printf("open as_cer file fail!\n");	//Added for test
		ret=-1;
		goto err;
	}
	
	printf("open as_cer file ok!\n");//Added for test

	x = X509_new();
	if(x==NULL)
	{
		printf("%s(%d): x X509_new is null.\n",__FUNCTION__,__LINE__);//Added for test
		ret=-1;
		goto err;
	}
	
	x = PEM_read_X509(fp, NULL, 0, NULL);
	if(x==NULL)
	{
		printf("%s(%d): x PEM_read_X509 return null.\n",__FUNCTION__,__LINE__);//Added for test
		ret=-1;
		goto err;
	}
//	X509_print(out,x);//Added for test

	WapiGetPubKeyFromCertificate(x,(void *)&pubKey);
	printf("%s(%d): pub_key.Length=%d\n",__FUNCTION__,__LINE__,pubKey.Length);//Added for test
//	dumpStr(pubKey.Content,pubKey.Length);
//	printf("%s(%d): pub_key.Length=%d\n",__FUNCTION__,__LINE__,pubKey.Content);//Added for test
#endif

#if 0
	pkey=X509_get_pubkey(x);
	if (pkey == NULL)
	 {
	  	printf("Unable to load Public Key\n");//Added for test
		goto err;
	 }
#endif

#if 1
	//To load user_cer as x509
	fp_user= fopen(user_cer, "r");
	if (!fp_user) 
	{
		printf("open user_cer file fail!\n");	//Added for test
		ret=-1;
		goto err;
	}
	
	printf("open user_cer file ok!\n");//Added for test


	x_user= X509_new();
	if(x_user==NULL)
	{
		printf("%s(%d): x_user X509_new is null.\n",__FUNCTION__,__LINE__);//Added for test
		ret=-1;
		goto err;
	}
	
	x_user = PEM_read_X509(fp_user, NULL, 0, NULL);
	if(x_user==NULL)
	{
		printf("%s(%d): x_user PEM_read_X509 return null.\n",__FUNCTION__,__LINE__);//Added for test
		ret=-1;
		goto err;
	}

	printf("%s(%d): x_user->signature->length=%d.\n",__FUNCTION__,__LINE__,x_user->signature->length);//Added for test
#if 0
	//X509_print(out,x_user);//Added for test
	if(X509_signature_print(out, x_user->sig_alg, x_user->signature) <= 0)
	{
		printf("%s(%d): x_user X509_signature_print errorl.\n",__FUNCTION__,__LINE__);//Added for test
		ret=-1;
		goto err;
	}
#endif

	//if(fp)
	//	fclose(fp);
#endif

#if 0
	fp_user= fopen(user_cer, "r");
	if (!fp_user) 
	{
		printf("open user_cer file fail!\n");	//Added for test
		ret=-1;
		goto err;
	}
	
	printf("Again open user_cer file ok!\n");//Added for test
#endif

	//To read cert
	memset(name,0,1000);
	memset(header,0,1000);
	p2 = name;
	p3 = header;
	p=src;
	
	PEM_read(fp_user,&p2,&p3,&p,(long *)&len);

	nameLen=strlen(name);
	headerLen=strlen(header);
	printf("%s(%d),len=%d,nameLen=%d,headerLen=%d\n",__FUNCTION__,__LINE__,len,nameLen,headerLen);//Added for test
	dumpDex((const unsigned char *)p,len);
	printf("\n%s(%d),len=%d,nameLen=%d,headerLen=%d\n",__FUNCTION__,__LINE__,len,nameLen,headerLen);//Added for test

	//printf("%s(%d),name=0x%x\n",__FUNCTION__,__LINE__,name);//Added for test
	//printf("%s(%d),header=0x%x\n",__FUNCTION__,__LINE__,header);//Added for test

	//printf("%s(%d),name=%s\n",__FUNCTION__,__LINE__,name);//Added for test
	//printf("%s(%d),header=%s\n",__FUNCTION__,__LINE__,header);//Added for test
	fclose(fp);

	//To get as public key from as_cer file
	fp = fopen("/var/tmp/debug_wapi2", "a+");
	if (!fp) 
	{
		printf("open /var/tmp/debug_wapi2  fail!\n");	//Added for test
		ret=-1;
		goto err;
	}
	
	printf("open debug_wapi2 file ok!\n");//Added for test
	//strcpy(name,"wapi_user_priv_key");
	//strcpy(header,"header_of_priv_key_zj");
	memset(name,0,1000);
	memset(header,0,1000);
	//p2 = name;
	//p3 = header;
	
	PEM_write(fp, name, header, p, (long)len);

	fclose(fp);
	

#if 0
	//To read private key
	p2 = name;
	p3 = header;
	
	PEM_read(fp_user,&p2,&p3,&p,(long *)&len);
	printf("%s(%d),len=%d\n",__FUNCTION__,__LINE__,len);//Added for test
	dumpDex((const unsigned char *)p,len);
	printf("%s(%d),len=%d\n",__FUNCTION__,__LINE__,len);//Added for test
#endif

#if 0
	srcH=p;
	for(i=0;i<100;i++)
	{
		printf("%s(%d),i=%d\n",__FUNCTION__,__LINE__,i);//Added for test
		srcLen=(unsigned short)len-x_user->signature->length-i;//-17 for test
		sigH=&p[srcLen+i];
		sigLen=(unsigned short)x_user->signature->length;
		res_verify=WapiECDSAVerify((unsigned char *)srcH, srcLen,(unsigned char *)sigH,sigLen,pubKey.Content,(int)pubKey.Length);
		//printf("%s(%d),srcLen=%d,sigLen=%d, pubKey.Length=%d, res_verify=%d\n",__FUNCTION__,__LINE__,srcLen,sigLen, pubKey.Length,res_verify);//Added for test
		if(res_verify==0)
		{
			printf("%s(%d),srcLen=%d,sigLen=%d, pubKey.Length=%d, res_verify=%d\n",__FUNCTION__,__LINE__,srcLen,sigLen, pubKey.Length,res_verify);//Added for test
			break;
		}
	}
#endif

#if 0
	printf("%s(%d),len=%d\n\n",__FUNCTION__,__LINE__);//Added for test
	dumpDex((const unsigned char *)srcH,(int)srcLen);
	printf("%s(%d),len=%d\n\n",__FUNCTION__,__LINE__);//Added for test
	dumpDex((const unsigned char *)sigH,(int)sigLen);
	printf("%s(%d),len=%d\n\n",__FUNCTION__,__LINE__);//Added for test
	dumpDex((const unsigned char *)pubKey.Content,(int)pubKey.Length);
	printf("%s(%d),len=%d\n\n",__FUNCTION__,__LINE__);//Added for test
#endif

#if 0
	res_verify=X509_verify(x_user, pkey);//return 1: success; return 0: failed
	printf("%s(%d): res_verify=%d.\n",__FUNCTION__,__LINE__,res_verify);//Added for test
	if(res_verify!=1)
	{
		printf("%s(%d): X509_verify failed.\n",__FUNCTION__,__LINE__);//Added for test
		ret=-1;
		goto err;
	}

#endif

	ret=0;
	
err:

#if 0
	if(pkey)
	 	EVP_PKEY_free(pkey);
#endif
	
	if(x)
		X509_free(x);

	if(fp)
		fclose(fp);

	if(x_user)
		X509_free(x_user);

	if(fp_user)
		fclose(fp_user);
	
	return ret;
}
#endif

#if 0
int    ReadCertificate(const char * certName, BIO * out)
{
      // PWAPI_ASUE_OPEN_CONTEXT           pASUE_Open_context;
	X509                    *x;
  	FILE                    *fp;
	unsigned char			buf[5120], *p;
  	int                     len;
	 ASN1_INTEGER *bs;
	 //long l;
	 UCHAR         pNamebuf[1024];
	 int                 Name_len;
	 int                 length=0;

	WAPI_SEC_DATA      pub_key;

	unsigned long errL;

	 //EC_POINT pub_key;
	//BIGNUM priv_key;

        //pASUE_Open_context=(PWAPI_ASUE_OPEN_CONTEXT)pArg;

	BIO_puts(out,"Testing ReadCertificate ... ");
	//printf("================%s(%d):================\n",__FUNCTION__,__LINE__);//Added for test
    /*  certificate	*/
	if(certName==NULL)
	{
		printf("%s(%d): certName is null.\n",__FUNCTION__,__LINE__);//Added for test
		return -1;
	}
	
	fp = fopen(certName, "rb");
	if (!fp) 
	{
		printf("open file fail!\n");	
		return -1;
	}
	printf("open file ok!\n");

	len = fread(buf, 1, 5120, fp);

	if(d2i_X509!=NULL)
		printf("%s(%d): d2i_X509=0x%x\n",__FUNCTION__,__LINE__,len,d2i_X509);//Added for test
		
	x=NULL;
	d2i_X509_fp(fp,&x);
	if(x==NULL)
	{
		errL=ERR_get_error();
		printf("%s(%d): d2i_X509_fp return null.error=0x%x\n",__FUNCTION__,__LINE__,errL);//Added for test
		return -1;
	}
		
	fclose(fp);

	//printf("%s(%d): [0]len=%d.\n",__FUNCTION__,__LINE__,len);//Added for test
	//dumpDex((void *)buf,len);
	
	printf("%s(%d): [0]len=%d.\n",__FUNCTION__,__LINE__,len);//Added for test
	dumpStr((void *)buf,len);
   
	p = buf;
	//  allocate memory for X509 struct
#if 0
	x = X509_new();
	if(x==NULL)
	{
		printf("%s(%d): X509_new is null.\n",__FUNCTION__,__LINE__);//Added for test
		return -1;
	}
#endif

	//x=NULL;

	//printf("%s(%d): d2i_X509=0x%x\n",__FUNCTION__,__LINE__,len,d2i_X509);//Added for test

#if 0
	x=d2i_X509(NULL, (const unsigned char **)&p, len);
	//  copy buf to X509 (DER to Internal)
	//if(!d2i_X509(&x, &p, len))
	//if(!d2i_X509(&x, (const unsigned char **)&p, len))
	if(x==NULL)
	{
		printf("%s(%d): d2i_X509 return null.\n",__FUNCTION__,__LINE__);//Added for test
		return -1;
	}
#endif

	//TRACE("decode file ok!\n");
	printf("%s(%d): len=%d.\n",__FUNCTION__,__LINE__,len);//Added for test

        //Certificate entity
       // pASUE_Open_context->ASUE_Certificate.Flag=1;
	 //pASUE_Open_context->ASUE_Certificate.Length=len;
	 //memcpy(pASUE_Open_context->ASUE_Certificate.Content,buf,len);

#if 0
       //ASUE identity	   
       WAPIX509GetSubjectName(x, NULL, 0,pNamebuf,&Name_len);	
	printf("%s(%d): Name_len=%d.\n",__FUNCTION__,__LINE__,Name_len);//Added for test
	//memcpy(pASUE_Open_context->ASUE_Identity.Content,pNamebuf,Name_len);  
	memset(pNamebuf,0,Name_len);
	//length=length+Name_len;
	 
	 WAPIX509GetIssuerName(x, NULL, 0,pNamebuf,&Name_len);
	 printf("%s(%d): Name_len=%d.\n",__FUNCTION__,__LINE__,Name_len);//Added for test
	 //memcpy(pASUE_Open_context->ASUE_Identity.Content+length,pNamebuf,Name_len);  
	 //length=length+Name_len;
	   
         bs=X509_get_serialNumber(x);
        // memcpy(pASUE_Open_context->ASUE_Identity.Content+length,bs->data,bs->length);
	  //length=length+bs->length;
	 
        // pASUE_Open_context->ASUE_Identity.Flag=1;
         //pASUE_Open_context->ASUE_Identity.Length=length;

	//get publick key from Certificate
	WapiGetPubKeyFromCertificate(x,(void *)&pub_key);
		printf("%s(%d): pub_key.Length=%d\n",__FUNCTION__,__LINE__,pub_key.Length);//Added for test
#endif

#if 1	   
	printf("%s(%d)--------------\n",__FUNCTION__,__LINE__);//Added for test
#if 0
	out=BIO_new(BIO_s_file());
	if(b==NULL)
	{
		printf("%s(%d): BIO_new is null\n",__FUNCTION__,__LINE__);//Added for test
		return -1;
	}
	BIO_set_fp(b,stdout,BIO_NOCLOSE);
#endif
	// 
	//X509_print(out,x);
	//BIO_free(b);
#endif

	X509_free(x);

	BIO_puts(out,"End testing ReadCertificate.");

	return 0;

}
#endif

#if 0
int    ReadCertificateASUE(void *pArg)
{
         PWAPI_ASUE_OPEN_CONTEXT           pASUE_Open_context;
	X509                    *x;
  	FILE                    *fp;
	unsigned char			buf[5000], *p;
  	int                     len;
	 ASN1_INTEGER *bs;
	 //long l;
	 UCHAR         pNamebuf[500];
	 int                 Name_len;
	int                 length=0;

        pASUE_Open_context=(PWAPI_ASUE_OPEN_CONTEXT)pArg;

    /*  certificate	*/

	fp = fopen("example.cer", "rb");
	if (!fp) 
	{
		printf("open file fail!\n");	
		return -1;
	}
	printf("open file ok!\n");

	len = fread(buf, 1, 5000, fp);
	fclose(fp);
   
	p = buf;
	//  allocate memory for X509 struct
	x = X509_new();
	//  copy buf to X509 (DER to Internal)
	d2i_X509(&x, (const unsigned char **)&p, len);

	TRACE("decode file ok!\n");

        //Certificate entity
        pASUE_Open_context->ASUE_Certificate.Flag=1;
	 pASUE_Open_context->ASUE_Certificate.Length=len;
	 memcpy(pASUE_Open_context->ASUE_Certificate.Content,buf,len);

       //ASUE identity	   
       WAPIX509GetSubjectName(x, NULL, 0,pNamebuf,&Name_len);	
	memcpy(pASUE_Open_context->ASUE_Identity.Content,pNamebuf,Name_len);  
	memset(pNamebuf,0,Name_len);
	length=length+Name_len;
	 
	 WAPIX509GetIssuerName(x, NULL, 0,pNamebuf,&Name_len);
	 memcpy(pASUE_Open_context->ASUE_Identity.Content+length,pNamebuf,Name_len);  
	 length=length+Name_len;
	   
         bs=X509_get_serialNumber(x);
         memcpy(pASUE_Open_context->ASUE_Identity.Content+length,bs->data,bs->length);
	  length=length+bs->length;
	 
         pASUE_Open_context->ASUE_Identity.Flag=1;
         pASUE_Open_context->ASUE_Identity.Length=length;

	//get publick key from Certificate
	WapiGetPubKeyFromCertificate(x,&pASUE_Open_context->Public_Key);

#if 0	   
	b=BIO_new(BIO_s_file());
	// 
	BIO_set_fp(b,stdout,BIO_NOCLOSE);
	// 
	ret=X509_print(b,x);
	BIO_free(b);
#endif

	X509_free(x);

	return 0;

}
/*
*  Handle time out
*/
BOOLEAN    HandleIdleState(void *pArg,void *pASUEINSTANCE)
{      
	TRACE("Enter handle idle state or BK USK establish state!\n");
 	PWAPI_ASUE_INSTANCE         pASUE_Instance;
	PWAPI_OPEN_CONTEXT           pWAPI_Context;
	PWAPI_ASUE_OPEN_CONTEXT    pASUE_Open_Context;
         unsigned long                          Time_out;
	//unsigned long                          current_time;
	//int                                            i,k;
	int							i;
         BOOLEAN                                bRecv_result;
	UCHAR                                   RxBuffer[2000];
	USHORT                                 RxLength;
	//UCHAR                                  Associate_state[100];	 
	UCHAR                                  subtype;
	UCHAR                                  buf[100];		 
	UINT                  bresult;

         pWAPI_Context=(PWAPI_OPEN_CONTEXT)pArg;
	pASUE_Open_Context=(PWAPI_ASUE_OPEN_CONTEXT)pWAPI_Context->wapiASUEContext;
         pASUE_Instance=(PWAPI_ASUE_INSTANCE)pASUEINSTANCE;

	Time_out=1000;
		 
	  // query connect state  0:connect  1 :disconnect
	/*	IORequest(0, OID_GEN_MEDIA_CONNECT_STATUS, (char*)Associate_state, 100);	 
	if(Associate_state[0]!=0)     
	 	return FALSE;
	*/	

	if(pASUE_Instance->ASUE_Current_State==ST_WAPI_ASUE_USKA_ESTABLISH)
	{
	      // Start usk key receive function
		memset(buf,0,100);
		buf[0]=0x1;
		buf[1]=0x1;
		buf[2]=0x0;
		if(pASUE_Instance->bAE_USK_Update||pASUE_Instance->bASUE_USK_Update)
			buf[3]=0x1;
		else
			buf[3]=0x0;
		memcpy(buf+4,pASUE_Instance->destAddress,6);
		memcpy(buf+10,pASUE_Instance->ASUE_Unicast_EncryptKey,16);
		memcpy(buf+26,pASUE_Instance->ASUE_Unicast_Integrity_VerifyKey,16);
		memcpy(buf+42,&pASUE_Instance->ASUE_USKID,1);
		if(!(pASUE_Instance->bNetworkType))  //IBSS
		{        	
	        	  	if(Compare_address(pASUE_Instance->destAddress,pASUE_Instance->CurrentAddress,6)>0)
				IORequest(1, OID_RT_WAPI_SET_KEY, (char*)buf, 100);
		}
		 else{
			   	IORequest(1, OID_RT_WAPI_SET_KEY, (char*)buf,100);
		}

		// Exit if usk update finish
		if(pASUE_Instance->bAE_USK_Update||pASUE_Instance->bASUE_USK_Update)
		   		return TRUE;
		 
	}
	 
	 if(!pASUE_Instance->bfail)
	 {	 		 
		for(i=0;i<30;i++)
		{
			if(adapter->bUIExist)
			{
				
				pASUE_Instance->bfail = TRUE;
				pASUE_Instance->ASUE_Current_State = ST_WAPI_ASUE_IDLE;
				return FALSE;
			}
			
			bRecv_result = WapiReceivePacket(pWAPI_Context, pASUE_Instance->destAddress, RxBuffer, &RxLength, Time_out, pASUE_Open_Context->FragThreshold, 0);

#if 0			
			TRACE("WAPI receive packet result:%d\n",bRecv_result);
			for (k=0;k<RxLength;k++)
			{
				TRACE("  %02x ",RxBuffer[k]);
				if((k+1)%16==0)
					TRACE("\n");
				
			}
			TRACE("\n");
#endif
			
			bresult=0;
			if(bRecv_result)
			{  
				TRACE("IN handle idle state recved packet subtype:%d\n",RxBuffer[3]);
					       //subtype
				subtype=RxBuffer[3];
				switch(subtype)
				{
		                  case      3 :       //active authentication
					bresult=WapiRecvActivateAuthenticationPacket(pASUE_Instance,RxBuffer,RxLength);
					break;
				  case    5 :      //access authenticate response
					bresult=WapiRecvAccessAuthenticateResponse(pASUE_Instance,RxBuffer,RxLength);
				        break;
				  case    8 :      //unicast   key  requst
					bresult=WapiRecvUnicastKeyAggrementRequest(pASUE_Instance,RxBuffer,RxLength);
					break;
				  case    10  :     //unicast  key  Confirm
					bresult=WapiRecvUnicastKeyAggrementConfirm(pASUE_Instance,RxBuffer,RxLength);
					break;
				   case    11 :    //  multicast key notice
					bresult=WapiRecvMulticastKeyNotification(pASUE_Instance,RxBuffer,RxLength);
					break;		
				   default:	
					break;				
				}			
						    
				TRACE("Handle  exit for :bresult  %d\n",bresult);
				if(bresult!=0)
					break;				
						    }						
						
				   Sleep(20);
			 	
			 	}
			 
	 	}
	 
	if(i==30||pASUE_Instance->bfail)
	 {
	 	//disconnect
	 	TRACE("fail to authentication or usk bk msk update ,disconnect\n");
		IORequest(1, OID_802_11_DISASSOCIATE, (char *) buf, 100);
		//memset(pASUE_Instance,0,sizeof(WAPI_ASUE_INSTANCE));
		return TRUE;
	 }
	 else
		return FALSE;

}




void    HandleSendAccessAuthenticateRequest(void *pArg,void *pASUEINSTANCE)
{         
	    PWAPI_ASUE_INSTANCE         pASUE_Instance;
	    PWAPI_OPEN_CONTEXT           pWAPI_Context;
	    PWAPI_ASUE_OPEN_CONTEXT    pASUE_Open_Context;
             unsigned long                          Time_out;
	    unsigned long                          current_time;
	    unsigned long                    	 Default_Time_out;
             int                                            i;
             BOOLEAN                                bRecv_result;
	    UCHAR                                   RxBuffer[2000];
	    USHORT                                 RxLength;
	    UINT                                      Result;

	    pWAPI_Context=(PWAPI_OPEN_CONTEXT)pArg;
	    pASUE_Open_Context=(PWAPI_ASUE_OPEN_CONTEXT)pWAPI_Context->wapiASUEContext;
             pASUE_Instance=(PWAPI_ASUE_INSTANCE)pASUEINSTANCE;

	    Default_Time_out=30000;
	
 	    for(i=0;i<Max_send_Count;i++)
 	    {
 			if(adapter->bUIExist)
 			{
 				pASUE_Instance->ASUE_Current_State=ST_WAPI_ASUE_IDLE;
		     		pASUE_Instance->bfail=TRUE;
				return;
 			}
				
		       Result=0;
 	 	       current_time=Get_Current_Time();
 		       Time_out=Default_Time_out-(current_time-pASUE_Instance->Last_Send_Time);
		       bRecv_result = WapiReceivePacket(pWAPI_Context, pASUE_Instance->destAddress, RxBuffer, &RxLength, Time_out, pASUE_Open_Context->FragThreshold, 0);
      		        if(bRecv_result)
      		       {
      			       if(RxBuffer[3]==5)
      			       {
				    Result=WapiRecvAccessAuthenticateResponse(pASUE_Instance,RxBuffer,RxLength);
				    if(Result!=0)
      			  	     	   break;
      			       }
      			  	
      			}
 			else
 		         {
 			  	WapiSendAccessAuthenticateRequest(pASUE_Instance);
 			}
 	 	
 	    }
 	    if(i==Max_send_Count)
 	    {
 	    	     pASUE_Instance->ASUE_Current_State=ST_WAPI_ASUE_IDLE;
		     pASUE_Instance->bfail=TRUE;
		     TRACE("==>Send Access authenticate request exceed retry time , disconnect!\n");
 	    }
		
}

void   HandleSendUnicastKeyAggrementResponse(void *pArg,void *pASUEINSTANCE)
{      
	    PWAPI_ASUE_INSTANCE         pASUE_Instance;
	    PWAPI_OPEN_CONTEXT           pWAPI_Context;
	    PWAPI_ASUE_OPEN_CONTEXT    pASUE_Open_Context;
             unsigned long                          Time_out;
	    unsigned long                          current_time;
	    unsigned long                       Default_Time_out;
             int                                            i;
             BOOLEAN                                bRecv_result;
	    UCHAR                                   RxBuffer[2000];
	    USHORT                                 RxLength;
	    UINT                                      Result=0;
	    pWAPI_Context=(PWAPI_OPEN_CONTEXT)pArg;
	    pASUE_Open_Context=(PWAPI_ASUE_OPEN_CONTEXT)pWAPI_Context->wapiASUEContext;
             pASUE_Instance=(PWAPI_ASUE_INSTANCE)pASUEINSTANCE;

	
	    TRACE("==>Enter Handle Send USK Aggrement Response!\n");
	    Default_Time_out=1000;
	    for(i=0;i<Max_send_Count;i++)
	    {
	    		if(adapter->bUIExist)
 			{
 				pASUE_Instance->ASUE_Current_State=ST_WAPI_ASUE_IDLE;
		     		pASUE_Instance->bfail=TRUE;
				return;
 			}
			Result=0;
	 	         current_time=Get_Current_Time();
		         Time_out=Default_Time_out-(current_time-pASUE_Instance->Last_Send_Time);
		         bRecv_result = WapiReceivePacket(pWAPI_Context, pASUE_Instance->destAddress, RxBuffer, &RxLength, Time_out, pASUE_Open_Context->FragThreshold, 0);
     		         if(bRecv_result)
     			{
     			  	     if(RxBuffer[3]==10)
     			  	     {
						Result=WapiRecvUnicastKeyAggrementConfirm(pASUE_Instance,RxBuffer,RxLength);
						if(Result!=0)
     			  	     	   		break;
     			  	     }
     			  	
     			 }
			 else
			 {
			       	WapiSendUnicastKeyAggrementResponse(pASUE_Instance);
			  }
	 	
	     }
	     TRACE("Max send num:%d\n",i);
 	      if(i==Max_send_Count)
 	      {
 	    	         pASUE_Instance->ASUE_Current_State=ST_WAPI_ASUE_IDLE;
			pASUE_Instance->bfail=TRUE;
		         TRACE("==>Send Access authenticate request exceed retry time , disconnect!\n");
 	      }
	
	TRACE("<==Exit Handle Send USK Aggrement Response!\n");
}

BOOLEAN    HandMulticastKeyResponseSent(void *pArg,void *pASUEINSTANCE)
{      
	TRACE("Enter handle multicast key response sent!\n");
	PWAPI_ASUE_INSTANCE         pASUE_Instance;
	PWAPI_OPEN_CONTEXT           pWAPI_Context;
	PWAPI_ASUE_OPEN_CONTEXT    pASUE_Open_Context;
	unsigned long                          Time_out;
//	int                                            i;
	BOOLEAN                                bRecv_result;
	UCHAR                                   RxBuffer[2000];
	USHORT                                 RxLength;
	//UCHAR                                  buf;		 
	UINT                  bresult;
	
	pWAPI_Context=(PWAPI_OPEN_CONTEXT)pArg;
	pASUE_Open_Context=(PWAPI_ASUE_OPEN_CONTEXT)pWAPI_Context->wapiASUEContext;
	pASUE_Instance=(PWAPI_ASUE_INSTANCE)pASUEINSTANCE;
	
	Time_out=1000;
	//Sleep(200);
	//after multicast key response sent , we try to sent multicast response packet if we received msk notice packet
	// for AE may drop our msk response packet!

		if(adapter->bUIExist)
		{
			pASUE_Instance->ASUE_Current_State=ST_WAPI_ASUE_IDLE;
			pASUE_Instance->bfail = TRUE;
			return TRUE;
		}

		bRecv_result = WapiReceivePacket(pWAPI_Context, pASUE_Instance->destAddress, RxBuffer, &RxLength, Time_out, pASUE_Open_Context->FragThreshold, 0);
		
		bresult=0;
		if(bRecv_result)
		{  
			TRACE("IN handle idle state recved packet subtype:%d\n",RxBuffer[3]);
			//subtype
			if(RxBuffer[3]==11)
			{
				WapiRecvMulticastKeyNotification(pASUE_Instance,RxBuffer,RxLength);
				return FALSE;

			}
						
		}
		else   	//if we don't  receive msk notice packet  ; it means that msk establish
		{		
		TRACE("==>Exit  handle multicast key response sent!\n");
		pASUE_Instance->ASUE_Current_State=ST_WAPI_ASUE_IDLE;
		memcpy(pASUE_Instance->Seckey_notice_flag,pASUE_Instance->Seckey_notice_flag_reserved,16);
		return TRUE;
		}	
	
	
}


/*
 *	thread for one authentication instance of WAPI supplicant. 
 *    lifecycle is one authentication process period.
 */
unsigned long __stdcall WapiSupplicantAuthInstanceThread(void * pArg)
{
         PWAPI_OPEN_CONTEXT             pWAPI_Context;
         PWAPI_ASUE_OPEN_CONTEXT   pASUE_open_contex;
	PWAPI_ASUE_INSTANCE           pASUE_INSTANCE;
	BOOLEAN                               bexit;
         BOOLEAN                              bresult;		
	UCHAR                                   Associate_state[100];         
//	UCHAR                                 iobuf;
	UCHAR                                  BKlabel[12];	
	UCHAR                                  keybuf[100];
//         UCHAR                                    temp[1000];		
	UCHAR                                     i;
//	int						len;

	
		
       TRACE("==>Enter supplicant thread!\n");
       pASUE_INSTANCE=(PWAPI_ASUE_INSTANCE)pArg;
	pASUE_INSTANCE->bASUE_InstanceThread_Exist = FALSE;
	pWAPI_Context=(PWAPI_OPEN_CONTEXT)pASUE_INSTANCE->pWAPI_OPEN_CONTEXT;
	pASUE_open_contex=(PWAPI_ASUE_OPEN_CONTEXT)pWAPI_Context->wapiASUEContext;           

		
	// wait to be connected to bss
	for(i=0;i<200;i++)
	{
		if(adapter->bUIExist)
 		{
			pASUE_INSTANCE->bInProcess=FALSE;
			pASUE_INSTANCE->bASUE_InstanceThread_Exist = TRUE;
			return 0;
 		}
		IORequest(0, OID_GEN_MEDIA_CONNECT_STATUS, (char*)Associate_state, 100);
			   //  0:connect  1 :disconnect
		if(Associate_state[0]==0)               
	                break;
		 Sleep(200);
	}
	if(i==200)
	{
			pASUE_INSTANCE->bInProcess=FALSE;
			TRACE("==>Exit supplicant thread when not associate!\n");
			pASUE_INSTANCE->bASUE_InstanceThread_Exist = TRUE;
			return 0;
	}

		//*********generate private key*******//        

		if((pWAPI_Context->bWAPI_Certificate) && (pASUE_INSTANCE->bFirst_Authenticate || pASUE_INSTANCE->bBK_Update))
		{
		 	 WapiGeneratePublicAndPrivateKey(pASUE_INSTANCE->pTemp_Privatekey_EDCH,pASUE_INSTANCE->ASUE_sec_data.Content,&pASUE_INSTANCE->ASUE_sec_data.Length);
		}
	
		//******************//

		 // network type: 1:  BSS  0: IBSS  
	/*         IORequest(0, OID_RT_GET_CONNECT_STATE, (char * )&iobuf, 1);      //iobuf value 1 :bss 2:IBSS 
               if(iobuf==1)
			pASUE_open_contex->bNetworkType=TRUE;
	      else
			pASUE_open_contex->bNetworkType=FALSE;
	      pASUE_INSTANCE->bNetworkType=pASUE_open_contex->bNetworkType;
		*/

	      // initialize
	      pASUE_INSTANCE->SeqNum=0;
	      pASUE_INSTANCE->FragThreshold=pASUE_open_contex->FragThreshold;
	      pASUE_INSTANCE->bInProcess=TRUE;
	      pASUE_INSTANCE->bfail=FALSE;

		
              //Current Address
              memcpy(pASUE_INSTANCE->CurrentAddress,pASUE_open_contex->CurrentAddress,6);

	      //preshared key  Calculate BK;     
	     if(pWAPI_Context->bWAPI_PSK) 
	     {
			memcpy(pASUE_INSTANCE->ASUE_BK,pASUE_open_contex->Preshared_BK,16);
			memcpy(BKlabel,pASUE_INSTANCE->destAddress,6);
			memcpy(BKlabel+6,pASUE_INSTANCE->CurrentAddress,6);
			KD_HMAC_SHA256(BKlabel, 12, pASUE_INSTANCE->ASUE_BK, 16, keybuf, 16);
			memcpy(pASUE_INSTANCE->ASUE_BKID,keybuf,16);
#if 0
			TRACE("BKID:\n");
			for(i=0;i<16;i++)
			{
				TRACE("%02x ",keybuf[i]);
			}
			TRACE("\n");
#endif
			i=0;
				
	       }

		 //WAPI Information 		 		
	/*	if(pWAPI_Context->bInfrastructure)  //1      // 1: bss     0: IBSS
		{
	IORequest(0, OID_RT_WAPI_GET_INFO, (char *)temp, 512);    
				pASUE_INSTANCE->WAPI_INFO_From_asso_request.Length=temp[0];
				memcpy(pASUE_INSTANCE->WAPI_INFO_From_asso_request.Content,temp+1,temp[0]);				
				pASUE_INSTANCE->WAPI_INFO_From_asso_response.Length=temp[1+temp[0]];
				memcpy(	pASUE_INSTANCE->WAPI_INFO_From_asso_response.Content,temp+temp[0]+2,temp[1+temp[0]]);
 
		}
		else
		{
	  memset(temp,0,1000);
				IORequest(0, OID_RT_WAPI_GET_INFO, (char *)temp, 1000);    
				pASUE_INSTANCE->WAPI_INFO_From_asso_response.Length=temp[0];
				memcpy(	pASUE_INSTANCE->WAPI_INFO_From_asso_response.Content,temp+1,temp[0]);
	  TRACE(" wapi information begin :%d\n ",temp[0]);
	  for (i=0;i<pASUE_INSTANCE->WAPI_INFO_From_asso_response.Length;i++)
	  {
	  TRACE("  %02x ",pASUE_INSTANCE->WAPI_INFO_From_asso_response.Content[i]);
						if((i+1)%16==0)
						TRACE("\n");
						
		}
						  TRACE("\n");
	}*/
	pASUE_INSTANCE->bNetworkType=pWAPI_Context->bInfrastructure;


		//set WPI PN
		/*for(k=0;k<8;k++)
			{			        
				pASUE_INSTANCE->WPIMSTDataPN[2*k]=0x36;
				pASUE_INSTANCE->WPIMSTDataPN[2*k+1]= 0x5c;
			}*/


		// state machine implementation	   	
   
	bexit=FALSE;
	
	do{       
		  TRACE("Enter do while for ASUE THREAD\n");
		  if(adapter->bUIExist)
 		{
			pASUE_INSTANCE->bInProcess=FALSE;
			pASUE_INSTANCE->bASUE_InstanceThread_Exist = TRUE;
			return 0;
 		}
	    	  switch(pASUE_INSTANCE->ASUE_Current_State)
	     	 {
	     	 	case    ST_WAPI_ASUE_IDLE:
			case    ST_WAPI_ASUE_BKSA_ESTABLISH:
			case    ST_WAPI_ASUE_USKA_ESTABLISH:
					bresult=HandleIdleState(pWAPI_Context,pASUE_INSTANCE);
					if(bresult)
					  bexit=TRUE;
				  else
					  bexit=FALSE;
					break;
			case   ST_WAPI_ASUE_ACTIVE_AUTHENTICATION_RCVD:
					WapiSendAccessAuthenticateRequest(pASUE_INSTANCE);
					break;
			case    ST_WAPI_ASUE_ACCESS_AUTHENTICATE_REQ_SNT:
					HandleSendAccessAuthenticateRequest(pWAPI_Context,pASUE_INSTANCE);
					break;
			case    ST_WAPI_ASUE_ACCESS_AUTHENTICATE_RSP_RCVD	:	
					pASUE_INSTANCE->ASUE_Current_State=ST_WAPI_ASUE_BKSA_ESTABLISH;
					break;
			case    ST_WAPI_ASUE_USK_AGGREMENT_REQ_RCVD:
				  WAPIASUEStartUSKupdate(pASUE_INSTANCE);
			case    ST_WAPI_ASUE_USK_AGGREMENT_RSP_SNT:
					HandleSendUnicastKeyAggrementResponse(pWAPI_Context,pASUE_INSTANCE);
					break;
			case    ST_WAPI_ASUE_USK_AGGREMENT_CONFIRM_RCVD:
				  TRACE("Supplicant instance: state usk establish!\n");
					pASUE_INSTANCE->ASUE_Current_State=ST_WAPI_ASUE_USKA_ESTABLISH;
					break;
			case    ST_WAPI_ASUE_MSK_NOTIFICATION_RCVD:
			case    ST_WAPI_ASUE_MSK_RSP_SNT:
				  bresult=HandMulticastKeyResponseSent(pWAPI_Context,pASUE_INSTANCE);
				  if(bresult)
					  bexit=TRUE;
				  else
					  bexit=FALSE;
				  break;
			     	  
			  default  :
					break;

	        	}
		
	}  while(!bexit);

	//exit set	
	pASUE_INSTANCE->ASUE_Current_State=ST_WAPI_ASUE_IDLE;
	pASUE_INSTANCE->bInProcess=FALSE;	
	pASUE_INSTANCE->SeqNum=0;
	pASUE_INSTANCE->bfail=FALSE;
	pASUE_INSTANCE->bASUE_USK_Update=FALSE;
	pASUE_INSTANCE->bBK_Update=FALSE;
	pASUE_INSTANCE->bFirst_Authenticate=FALSE;
	TRACE("==>Exit supplicant thread !\n");
	pASUE_INSTANCE->bASUE_InstanceThread_Exist = TRUE;
	return 1;
}


/*
 *	main process for WAPI supplicant. 
 *    lifecycle is one connected period.
 */
unsigned long __stdcall WapiSupplicantProcess(void * pArg)
{

	// allocate memory for our resources
	PWAPI_ASUE_OPEN_CONTEXT             pASUE_Open_Contex;
	PWAPI_OPEN_CONTEXT                       pWAPI_Open_Contex;
	
	UCHAR                                               DestAddress[6];
	BOOLEAN                                            bcreate_thread=FALSE;
	BOOLEAN                                            bresult;
	BOOLEAN                                              bcompare_result;
//	int                                                        i,k;
	int												i;
	int                                                       num_address;
	WAPI_ASUE_STATE                                ASUE_State;
	char  *preSharedKeyLabel="preshared key expansion for authentication and key negotiation";
	
	UCHAR                                                  Buf[500];   
	UCHAR                                                  keybuf[100];
	int                                                         Length=0;
	DWORD                                                 Thread_id;
	UCHAR                                                   zero_address[6];
     
	unsigned int                                                  FragThreshold;	 
	WAPI_PACKET_STATE                                     PeerAddress_State;
	char *					defaultSsid = "ANY";
	USHORT                                                   QuireNum=0;
//	UCHAR                                                     buf0[100],buf1[100];
	UCHAR					buf0[100];

	int   nid;
	EC_builtin_curve *curves = NULL;
	size_t		crv_len = 0;

	  //Initialize  Allocate memory 
         pASUE_Open_Contex=(PWAPI_ASUE_OPEN_CONTEXT)malloc(sizeof(WAPI_ASUE_OPEN_CONTEXT));
	  NPROT_ZERO_MEM(pASUE_Open_Contex, sizeof(WAPI_ASUE_OPEN_CONTEXT));

         pWAPI_Open_Contex=(PWAPI_OPEN_CONTEXT)pArg;
      
         pWAPI_Open_Contex->wapiASUEContext=pASUE_Open_Contex;
	   
	//Current Address
	IORequest(0, 	OID_RT_PRO_READ_MAC_ADDRESS, (char *)pASUE_Open_Contex->CurrentAddress, 6); 
	
	//ECDH  Parameter
	pASUE_Open_Contex->EDCH_Parameter.Flag=0x1;
	pASUE_Open_Contex->EDCH_Parameter.Length = 0x000b;
	pASUE_Open_Contex->EDCH_Parameter.Content[0] = 0x06;
	pASUE_Open_Contex->EDCH_Parameter.Content[1] = 0x09;
	pASUE_Open_Contex->EDCH_Parameter.Content[2] = 0x2a;
	pASUE_Open_Contex->EDCH_Parameter.Content[3] = 0x80;
	pASUE_Open_Contex->EDCH_Parameter.Content[4] = 0x1c;
	pASUE_Open_Contex->EDCH_Parameter.Content[5] = 0xd7;
	pASUE_Open_Contex->EDCH_Parameter.Content[6] = 0x63;
	pASUE_Open_Contex->EDCH_Parameter.Content[7] = 0x01;
	pASUE_Open_Contex->EDCH_Parameter.Content[8] = 0x01;
	pASUE_Open_Contex->EDCH_Parameter.Content[9] = 0x02;
	pASUE_Open_Contex->EDCH_Parameter.Content[10] = 0x01;


	
	//Query fragthreshold;
	IORequest(0, OID_GEN_MAXIMUM_FRAME_SIZE , (char*) &FragThreshold, 4);	
       	pASUE_Open_Contex->FragThreshold=FragThreshold+24;

		
       	memset(zero_address,0,6);  //for search idle instance by address

	//current address
	IORequest(0, OID_RT_PRO_READ_MAC_ADDRESS, (char *)pASUE_Open_Contex->CurrentAddress, 6);


	//Initialize exist instance thread
	for (i=0;i<MAX_ASUE_INSTANCE_NUM;i++)
	{
		pASUE_Open_Contex->ASUE_Instance[i].bASUE_InstanceThread_Exist = TRUE;
	}


	//preshared key mode: calculate BK
	if(pWAPI_Open_Contex->bWAPI_PSK)
	{
		memcpy(pASUE_Open_Contex->PassPhraseKey,pWAPI_Open_Contex->passphrase,pWAPI_Open_Contex->passphrase_len);
		pASUE_Open_Contex->PassPhraseKeyLength=pWAPI_Open_Contex->passphrase_len;
		KD_HMAC_SHA256((UCHAR*)preSharedKeyLabel, strlen(preSharedKeyLabel),pASUE_Open_Contex->PassPhraseKey,pASUE_Open_Contex->PassPhraseKeyLength,keybuf,16);
		memcpy(pASUE_Open_Contex->Preshared_BK,keybuf,16);

#if 0		
		for(i=0;i<strlen(preSharedKeyLabel);i++)
		{
			TRACE("%02x ",preSharedKeyLabel[i]);
		}
		TRACE("\n");
#endif
		i=0;		
#if 0
		TRACE("Passphrase key:\n");
		for(i=0;i<pASUE_Open_Contex->PassPhraseKeyLength;i++)
		{
			TRACE("%02x ",pASUE_Open_Contex->PassPhraseKey[i]);
		}
		TRACE("\n");
#endif
		i=0;		
#if 0
		TRACE("BK:\n");
		for(i=0;i<16;i++)
		{
			TRACE("%02x ",pASUE_Open_Contex->Preshared_BK[i]);
		}
		TRACE("\n");
#endif
		i=0;
		
	}
	else
	{
//Initialize curves EDSCA
 		crv_len = EC_get_builtin_curves(NULL, 0);
		if (curves == NULL)
		{
			TRACE( "malloc curves error\n");
			goto err;
		}
		if (!EC_get_builtin_curves(curves, crv_len))
		{
			TRACE("unable to get internal curves\n");
			goto err;
		}
		nid = curves[13].nid;

//allocate memory for ECDH Key
		for(i=0;i<MAX_ASUE_INSTANCE_NUM;i++)
		{
			pASUE_Open_Contex->ASUE_Instance[i].pPrivate_Public_Key_For_EDSCA= EC_KEY_new_by_curve_name(nid);
			if(pASUE_Open_Contex->ASUE_Instance[i].pPrivate_Public_Key_For_EDSCA == NULL)
			{
				TRACE("Unable to allocate memory for  EDSCA \n");
				goto err;
			}

			pASUE_Open_Contex->ASUE_Instance[i].pTemp_Privatekey_EDCH = EC_KEY_new_by_curve_name(NID_X9_62_prime192v1);
			if(pASUE_Open_Contex->ASUE_Instance[i].pTemp_Privatekey_EDCH == NULL)
			{
				TRACE("Unable to allocate memory for EDCH \n");
				goto err;
			}
			
		}

//read certificate 
		ReadCertificateASUE(pASUE_Open_Contex);
	}


	TRACE("After initialize in Suppilicant \n");
		
	//create our authentication instance thread if need
	do
       	{

		 if (adapter->bUIExist)
				 	break;
       		bcreate_thread=FALSE;
		memset(Buf,0,500);
		IORequest(0,OID_WAPI_ASUE_CREATE_THREAD,(char*)Buf,500);
		TRACE("==>ASUE: Wait for create thread!\n");
		//OID struct num of address  /state/address....../state/address

		if(Buf[0]==0)			//Buf[0] :address num
			bresult=FALSE;			
		else
		{
			bresult=TRUE;
#if 0
				TRACE("The OID_WAPI_ASUE_CREATE_THREAD value:%d\n",Buf[0]);

				for(k=0;k<8;k++)
				{
					TRACE("%02x  ",Buf[k]);
				}
				TRACE("\n");	
#endif
		   }

		 if(adapter->bUIExist)
			break;

		
		if(bresult)
		{
			     Length=2;
			     for(num_address=0;num_address<Buf[0];num_address++)
			     {
				     Length=Length+num_address*7;
			              //check destAddress to create thread
			              memcpy(DestAddress,Buf+Length,6);		              
						  
					for (i=0;i<MAX_ASUE_INSTANCE_NUM;i++)
					{
						  //bcompare_result= eqMacAddr(DestAddress,pASUE_Open_Contex->ASUE_Instance[i].destAddress);
						  bcompare_result=Compare_Two(DestAddress, pASUE_Open_Contex->ASUE_Instance[i].destAddress, 6);
		                              if(bcompare_result)
		                              {
							PeerAddress_State=(WAPI_PACKET_STATE)Buf[num_address*7+1];	
							TRACE("compare result : i:%d  peer address state:%d\n",i,PeerAddress_State);
		                                              switch(PeerAddress_State)
		                                              {
		                                                   		 case WAPI_RCEV_PACKET:
											ASUE_State=pASUE_Open_Contex->ASUE_Instance[i].ASUE_Current_State;									
											if(ASUE_State==ST_WAPI_ASUE_IDLE&&(!pASUE_Open_Contex->ASUE_Instance[i].bInProcess))
											{
												TRACE("==>ASUE Start authenticate or psk\n");
												bcreate_thread=TRUE;
											}
											break;
											
									   case WAPI_START_BK_UPDATE:
									  	         TRACE("==>ASUE Start BK update\n");
										   	ASUE_State=ST_WAPI_ASUE_ACTIVE_AUTHENTICATION_RCVD;
									      		pASUE_Open_Contex->ASUE_Instance[i].ASUE_Current_State=ASUE_State;
											pASUE_Open_Contex->ASUE_Instance[i].bBK_Update=TRUE;
										   	bcreate_thread=TRUE;
										      break;
											  
									  case WAPI_START_USK_UPDATE : 
									  	        TRACE("==>ASUE Start USK update\n");
										   	ASUE_State=ST_WAPI_ASUE_USK_AGGREMENT_REQ_RCVD;
									      		pASUE_Open_Contex->ASUE_Instance[i].ASUE_Current_State=ASUE_State;
											pASUE_Open_Contex->ASUE_Instance[i].bASUE_USK_Update=TRUE;
										   	bcreate_thread=TRUE;
										 	    break;
									   case  WAPI_START_STA_LEAVE:
									   	        TRACE("==>sta leave : don't create thread\n");
										   	memset(&pASUE_Open_Contex->ASUE_Instance[i],0,  sizeof(WAPI_ASUE_INSTANCE) );							   
										       	bcreate_thread=FALSE;
											break;		   	
									  default:
											bcreate_thread=FALSE;
											break;
														
		                                            }
			   
							break;
							
		                                    }
							
					}
					if(i==MAX_ASUE_INSTANCE_NUM)
					{
						  bcreate_thread=TRUE;
						  for(i=0;i<MAX_ASUE_INSTANCE_NUM;i++)
						  {
						  	  bcompare_result = eqMacAddr(zero_address, pASUE_Open_Contex->ASUE_Instance[i].destAddress);
							  if(bcompare_result)
							  {
							  	TRACE("==>ASUE create a thread for the first time authentication\n");
								pASUE_Open_Contex->ASUE_Instance[i].bFirst_Authenticate=TRUE;
								break;
							  }
						  }

					}	

					if(bcreate_thread)
					{
						pASUE_Open_Contex->ASUE_Instance[i].pWAPI_OPEN_CONTEXT=pWAPI_Open_Contex;
						memcpy(pASUE_Open_Contex->ASUE_Instance[i].destAddress,DestAddress,6);
						pASUE_Open_Contex->Instance_id=i;
					       CreateThread(NULL,0,WapiSupplicantAuthInstanceThread,&pASUE_Open_Contex->ASUE_Instance[i],0,&Thread_id);	
					}
	
			}
	    }


	   if(adapter->bUIExist)
			break;

            Sleep(200);    //sleep 2s for requery  


	   IORequest(0, OID_GEN_MEDIA_CONNECT_STATUS, (char*)buf0, 100);    // 0:connect   1:disconnect
	    if(pWAPI_Open_Contex->bInfrastructure)
	    {
			if(buf0[0]!=0)
			{
				do
				{
						if(!pASUE_Open_Contex->ASUE_Instance[pASUE_Open_Contex->Instance_id].bASUE_InstanceThread_Exist)
							Sleep(50);
						else
							break;
				}while(TRUE);
				memset(&pASUE_Open_Contex->ASUE_Instance[pASUE_Open_Contex->Instance_id],0, sizeof(WAPI_ASUE_INSTANCE) );							   
				pASUE_Open_Contex->ASUE_Instance[pASUE_Open_Contex->Instance_id].bASUE_InstanceThread_Exist  = TRUE;

			}
		}
	    else
	    {
			if(buf0[0]!=0)
			for(i=0;i<MAX_ASUE_INSTANCE_NUM;i++)
			{
				do
				{
						TRACE("do while for waiting ASUE Exist Child Thread \n");
						if(!pASUE_Open_Contex->ASUE_Instance[i].bASUE_InstanceThread_Exist)
							Sleep(50);
						else
							break;
				}while(TRUE);
				memset(&pASUE_Open_Contex->ASUE_Instance[i],0, sizeof(WAPI_ASUE_INSTANCE) );
				pASUE_Open_Contex->ASUE_Instance[i].bASUE_InstanceThread_Exist = TRUE;

			}
	    }


		if(((adapter->ssidLength== strlen(defaultSsid)) && (!memcmp(adapter->ssid,defaultSsid,strlen(defaultSsid)))) ||(RT_IsUnplug() == TRUE))
		{
				break;
		}

		
	/*	QuireNum=QuireNum+1;
		if(QuireNum==0xffff)
		{
			QuireNum=0;
		}
		else if(QuireNum>20)
		{
			memset(buf0,0,100);
			memset(buf1,0,100);
			if(pWAPI_Open_Contex->bInfrastructure)
			{
				IORequest(0, OID_GEN_MEDIA_CONNECT_STATUS, (char*)buf0, 100);  // 0:connect   1:disconnect
				if(buf0[0]!=0)
					break;
			}
			else
			{
				
				IORequest(0, OID_GEN_MEDIA_CONNECT_STATUS, (char*)buf0, 100);
				IORequest(0, OID_RT_GET_CONNECT_STATE, (char*)buf1, 100);          // 1: infrastructure connect  2:IBSS  
				TRACE("Media connect state:%d,IBSS:%d\n",buf0[0],buf1[0]); 
				if((buf0[0]!=0)&&(buf1[0]!=2))
					break;
				
       	}
		}*/
		
       	} while(TRUE);

	// idle cycle (do some sleep and exit if disconnectd),
	// and create authentication instance thread if need (e.g. update keys)
	


	// free memory for our resources
err:	
	if (curves)
		OPENSSL_free(curves);

	for(i=0;i<MAX_ASUE_INSTANCE_NUM;i++)
	{
		if(pASUE_Open_Contex->ASUE_Instance[i].pTemp_Privatekey_EDCH != NULL)
		{
			EC_KEY_free(pASUE_Open_Contex->ASUE_Instance[i].pTemp_Privatekey_EDCH);
		}

		if(pASUE_Open_Contex->ASUE_Instance[i].pPrivate_Public_Key_For_EDSCA != NULL)
		{
			EC_KEY_free(pASUE_Open_Contex->ASUE_Instance[i].pPrivate_Public_Key_For_EDSCA);
		}
	}
	
	
	do
	{
		TRACE("============>ASUE ASUE ASUE start free \n");
		for(i=0;i<MAX_ASUE_INSTANCE_NUM;i++)
		{
			if(!pASUE_Open_Contex->ASUE_Instance[i].bASUE_InstanceThread_Exist)
			{
				TRACE("i  %d \n",i);
				break;
			}
		}
		if(i==MAX_ASUE_INSTANCE_NUM)
		{
			TRACE("free asue memory \n");

			if(pASUE_Open_Contex)
			{
				free(pASUE_Open_Contex);
				pWAPI_Open_Contex->bASUEinProcess=FALSE;
				pWAPI_Open_Contex->wapiASUEContext = NULL;
				pASUE_Open_Contex = NULL;
			}
			break;
		}
		else
		{
			Sleep(50);
		}
	}while(TRUE);
	
	
	if(!pWAPI_Open_Contex->bAEinProcess)
	{
		TRACE("free open context in supplicant \n");
		free(pWAPI_Open_Contex);
		pWAPI_Open_Contex = NULL;
	}

	// exit main process thread
	TRACE("==>Exit supplicant process\n");
	return 1;

}
#endif
