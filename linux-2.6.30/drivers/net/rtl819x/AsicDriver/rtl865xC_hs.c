#include <net/rtl/rtl_types.h>
#include <net/rtl/rtl_glue.h>
//#include "assert.h"
#include "asicRegs.h"
//#include "rtl_utils.h"
//#include "rtl8651_debug.h"
#include "rtl865xC_hs.h"


static int32 convertHsbToSoftware( hsb_t* rawHsb, hsb_param_t* hsb )
{
	/* bit-to-bit mapping */
	hsb->spa = rawHsb->spa;
	hsb->trigpkt = rawHsb->trigpkt;
 	hsb->len = rawHsb->len;
	hsb->vid = rawHsb->vid;
	hsb->tagif = rawHsb->tagif;
	hsb->pppoeif = rawHsb->pppoeif;
	hsb->sip = rawHsb->sip29_0 | (rawHsb->sip31_30<<30);
	hsb->sprt = rawHsb->sprt;
	hsb->dip = rawHsb->dip13_0 | (rawHsb->dip31_14<<14);
	hsb->dprt = rawHsb->dprt13_0 | (rawHsb->dprt15_14<<14);
	hsb->ipptl = rawHsb->ipptl;
	hsb->ipfg = rawHsb->ipfg;
	hsb->iptos = rawHsb->iptos;
	hsb->tcpfg = rawHsb->tcpfg;
	hsb->type = rawHsb->type;
	hsb->patmatch = rawHsb->patmatch;
	hsb->ethtype = rawHsb->ethtype;
#if 1 /* Since the endian is reversed, we must translate it. */
	hsb->da[5] = rawHsb->da14_0;
	hsb->da[4] = (rawHsb->da14_0>>8) | (rawHsb->da46_15<<7);
	hsb->da[3] = rawHsb->da46_15>>1;
	hsb->da[2] = rawHsb->da46_15>>9;
	hsb->da[1] = rawHsb->da46_15>>17;
	hsb->da[0] = (rawHsb->da46_15>>25) | (rawHsb->da47_47<<7);
	hsb->sa[5] = rawHsb->sa30_0;
	hsb->sa[4] = rawHsb->sa30_0>>8;
	hsb->sa[3] = rawHsb->sa30_0>>16;
	hsb->sa[2] = (rawHsb->sa30_0>>24) | (rawHsb->sa47_31<<7);
	hsb->sa[1] = rawHsb->sa47_31>>1;
	hsb->sa[0] = rawHsb->sa47_31>>9;
#else
	hsb->da[0] = rawHsb->da14_0;
	hsb->da[1] = (rawHsb->da14_0>>8) | (rawHsb->da46_15<<7);
	hsb->da[2] = rawHsb->da46_15>>1;
	hsb->da[3] = rawHsb->da46_15>>9;
	hsb->da[4] = rawHsb->da46_15>>17;
	hsb->da[5] = (rawHsb->da46_15>>25) | (rawHsb->da47_47<<7);
	hsb->sa[0] = rawHsb->sa30_0;
	hsb->sa[1] = rawHsb->sa30_0>>8;
	hsb->sa[2] = rawHsb->sa30_0>>16;
	hsb->sa[3] = (rawHsb->sa30_0>>24) | (rawHsb->sa47_31<<7);
	hsb->sa[4] = rawHsb->sa47_31>>1;
	hsb->sa[5] = rawHsb->sa47_31>>9;
#endif
	hsb->hiprior = rawHsb->hiprior;
	hsb->snap = rawHsb->snap;
	hsb->udpnocs = rawHsb->udpnocs;
	hsb->ttlst = rawHsb->ttlst;
	hsb->dirtx = rawHsb->dirtx;
	hsb->l3csok = rawHsb->l3csok;
	hsb->l4csok = rawHsb->l4csok;
	hsb->ipfo0_n = rawHsb->ipfo0_n;
	hsb->llcothr = rawHsb->llcothr;
	hsb->urlmch = rawHsb->urlmch;
	hsb->extspa = rawHsb->extspa;
	hsb->extl2 = rawHsb->extl2;
	hsb->linkid = rawHsb->linkid;
	hsb->pppoeid = rawHsb->pppoeid;
	return SUCCESS;
}


int32 virtualMacGetHsb( hsb_param_t* hsb )
{
	hsb_t rawHsb;
	int32 ret = SUCCESS;

	{ /* Word-Access */
		uint32 *pSrc, *pDst;
		uint32 i;

		/* We must assert structure size is the times of 4-bytes. */
		if ( (sizeof(rawHsb)%4) != 0 ) RTL_BUG( "sizeof(rawHsb) is not the times of 4-bytes." );

		pSrc = (uint32*)HSB_BASE;
		pDst = (uint32*)&rawHsb;
		for( i = 0; i < sizeof(rawHsb); i+=4 )
		{
			*pDst = READ_MEM32((uint32)pSrc);
			pSrc++;
			pDst++;
		}
	}

	convertHsbToSoftware( &rawHsb, hsb );
	return ret;
}

int32 convertHsaToSoftware( hsa_t* rawHsa, hsa_param_t* hsa )
{
	/* bit-to-bit mapping */
#if 1 /* Since the endian is reversed, we must translate it. */
	hsa->nhmac[5] = rawHsa->nhmac0;
	hsa->nhmac[4] = rawHsa->nhmac1;
	hsa->nhmac[3] = rawHsa->nhmac2;
	hsa->nhmac[2] = rawHsa->nhmac3;
	hsa->nhmac[1] = rawHsa->nhmac4;
	hsa->nhmac[0] = rawHsa->nhmac5;
#else
	hsa->nhmac[0] = rawHsa->nhmac0;
	hsa->nhmac[1] = rawHsa->nhmac1;
	hsa->nhmac[2] = rawHsa->nhmac2;
	hsa->nhmac[3] = rawHsa->nhmac3;
	hsa->nhmac[4] = rawHsa->nhmac4;
	hsa->nhmac[5] = rawHsa->nhmac5;
#endif
	hsa->trip = rawHsa->trip15_0 | (rawHsa->trip31_16<<16);
	hsa->port = rawHsa->port;
	hsa->l3csdt = rawHsa->l3csdt;
	hsa->l4csdt = rawHsa->l4csdt;
	hsa->egif = rawHsa->egif;
	hsa->l2tr = rawHsa->l2tr;
	hsa->l34tr = rawHsa->l34tr;
	hsa->dirtxo = rawHsa->dirtxo;
	hsa->typeo = rawHsa->typeo;
	hsa->snapo = rawHsa->snapo;
	hsa->rxtag = rawHsa->rxtag;
	hsa->dvid = rawHsa->dvid;
	hsa->pppoeifo = rawHsa->pppoeifo;
	hsa->pppidx = rawHsa->pppidx;
	hsa->leno = rawHsa->leno5_0|(rawHsa->leno14_6<<6);
	hsa->l3csoko = rawHsa->l3csoko;
	hsa->l4csoko = rawHsa->l4csoko;
	hsa->frag = rawHsa->frag;
	hsa->lastfrag = rawHsa->lastfrag;
	hsa->ipmcastr = rawHsa->ipmcastr;
	hsa->svid = rawHsa->svid;
	hsa->fragpkt = rawHsa->fragpkt;
	hsa->ttl_1if = rawHsa->ttl_1if4_0|(rawHsa->ttl_1if5_5<<5)|(rawHsa->ttl_1if8_6<<6);
	hsa->dpc = rawHsa->dpc;
	hsa->spao = rawHsa->spao;
	hsa->hwfwrd = rawHsa->hwfwrd;
	hsa->dpext = rawHsa->dpext;
	hsa->spaext = rawHsa->spaext;
	hsa->why2cpu = rawHsa->why2cpu13_0|(rawHsa->why2cpu15_14<<14);
	hsa->spcp = rawHsa->spcp;
	hsa->dvtag = rawHsa->dvtag;
	hsa->difid = rawHsa->difid;
	hsa->linkid = rawHsa->linkid;
	hsa->siptos = rawHsa->siptos;
	hsa->dp = rawHsa->dp6_0;
	hsa->priority = rawHsa->priority;
	return SUCCESS;
}

int32 virtualMacGetHsa( hsa_param_t* hsa )
{
	hsa_t rawHsa;
	int32 ret = SUCCESS;

	{ /* Word-Access */
		uint32 *pSrc, *pDst;
		uint32 i;

		/* We must assert structure size is the times of 4-bytes. */
		if ( (sizeof(rawHsa)%4) != 0 ) RTL_BUG( "sizeof(rawHsa) is not the times of 4-bytes." );

		pSrc = (uint32*)HSA_BASE;
		pDst = (uint32*)&rawHsa;
		for( i = 0; i < sizeof(rawHsa); i+=4 )
		{
			*pDst = READ_MEM32((uint32)pSrc);
			pSrc++;
			pDst++;
		}
	}

	convertHsaToSoftware( &rawHsa, hsa );
	return ret;
}
