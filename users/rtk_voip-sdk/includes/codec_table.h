#ifndef __CODEC_TABLE_H__
#define __CODEC_TABLE_H__

#include "kernel_config.h"
#include "rtk_voip.h"

#ifdef CONFIG_RTK_VOIP_G7231
#define NUM_G723_CODEC_TABLE		1
#else
#define NUM_G723_CODEC_TABLE		0
#endif

#ifdef CONFIG_RTK_VOIP_G729AB
#define NUM_G729_CODEC_TABLE		1
#else
#define NUM_G729_CODEC_TABLE		0
#endif

#ifdef CONFIG_RTK_VOIP_G726
#define NUM_G726_CODEC_TABLE		4	/* 16/24/32/40 k */
#else
#define NUM_G726_CODEC_TABLE		0
#endif

#ifdef CONFIG_RTK_VOIP_GSMFR
#define NUM_GSMFR_CODEC_TABLE		1
#else
#define NUM_GSMFR_CODEC_TABLE		0
#endif

#ifdef CONFIG_RTK_VOIP_ILBC
#define NUM_ILBC_CODEC_TABLE        1
#else
#define NUM_ILBC_CODEC_TABLE        0
#endif

#ifdef CONFIG_RTK_VOIP_G722
#define NUM_G722_CODEC_TABLE        1
#else
#define NUM_G722_CODEC_TABLE        0
#endif

#ifdef CONFIG_RTK_VOIP_SPEEX_NB
#define NUM_SPEEX_NB_CODEC_TABLE        1
#else
#define NUM_SPEEX_NB_CODEC_TABLE        0
#endif

#ifdef CONFIG_RTK_VOIP_G7111
#define NUM_G7111_CODEC_TABLE		2
#else
#define NUM_G7111_CODEC_TABLE		0
#endif

#define NUM_CODEC_TABLE				( 2 /* 711 */ + 			\
									  NUM_G723_CODEC_TABLE + 	\
									  NUM_G729_CODEC_TABLE +	\
									  NUM_G726_CODEC_TABLE +	\
									  NUM_GSMFR_CODEC_TABLE +	\
									  NUM_ILBC_CODEC_TABLE +	\
									  NUM_G722_CODEC_TABLE +	\
									  NUM_SPEEX_NB_CODEC_TABLE + \
									  NUM_G7111_CODEC_TABLE )

/* ****************************************************************** 
 * Assistant Definition
 * ****************************************************************** */
#define CT_ASSERT( expr )		extern int __ct_assert[ 2 * ( expr ) - 1 ]

#endif /* __CODEC_TABLE_H__ */

