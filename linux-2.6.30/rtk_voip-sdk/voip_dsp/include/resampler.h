#ifndef RESAMPLER_H
#define RESAMPLER_H

#include "rtk_voip.h"

#if 1
#define SUPPORT_QUALITY_0_TO_3		1	// use less memory
#else
#define SUPPORT_QUALITY_0_TO_10		0	// use more memory
#endif

#ifdef CONFIG_RTK_VOIP_WIDEBAND_SUPPORT
// +MAX_VOIP_CH_NUM for IVR use 
// +MAX_VOIP_CH_NUM for PCM RX use 
// +MAX_VOIP_CH_NUM for pcm_hdr TX use 
// +MAX_VOIP_CH_NUM for T38 handler 
// +MAX_VOIP_CH_NUM for PCM RX after LEC use 
#define MAX_NB_CHANNELS		( MAX_DSP_RTK_SS_NUM + MAX_DSP_RTK_CH_NUM * 5 )	
#define IVR_RESAMPLER_OFFSET	MAX_DSP_RTK_SS_NUM
#define PCMRX_RESAMPLER_OFFSET	( IVR_RESAMPLER_OFFSET + 1 * MAX_DSP_RTK_CH_NUM )
#define PCMHDR_RESAMPLER_OFFSET	( IVR_RESAMPLER_OFFSET + 2 * MAX_DSP_RTK_CH_NUM )
#define T38HDR_RESAMPLER_OFFSET	( IVR_RESAMPLER_OFFSET + 3 * MAX_DSP_RTK_CH_NUM )
#define PCMRX_LEC_RESAMPLER_OFFSET ( IVR_RESAMPLER_OFFSET + 4 * MAX_DSP_RTK_CH_NUM )
#else
#define MAX_NB_CHANNELS		MAX_DSP_RTK_SS_NUM	// number for channel support for resampler
#endif

#define RESAMPLER_QUALITY_MAX		10
#define RESAMPLER_QUALITY_MIN		0
#define RESAMPLER_QUALITY_DEFAULT	4
#define RESAMPLER_QUALITY_VOIP		3
#define RESAMPLER_QUALITY_DESKTOP	5

enum {
   RESAMPLER_ERR_SUCCESS         = 0,
   RESAMPLER_ERR_ALLOC_FAILED    = 1,
   RESAMPLER_ERR_BAD_STATE       = 2,
   RESAMPLER_ERR_INVALID_ARG     = 3,
   RESAMPLER_ERR_PTR_OVERLAP     = 4,
   
   RESAMPLER_ERR_MAX_ERROR
};

typedef struct ResamplerState_ ResamplerState;

typedef int (*resampler_basic_func)(ResamplerState *, uint32_t , const int16_t *, uint32_t *, int16_t *, uint32_t *);


struct ResamplerState_ {
   uint32_t in_rate;
   uint32_t out_rate;
   uint32_t num_rate;
   uint32_t den_rate;
   
   int    quality;
   uint32_t nb_channels;
   uint32_t filt_len;
   uint32_t mem_alloc_size;
   int     	int_advance;
   int          frac_advance;
   //float  cutoff;
   uint32_t oversample;
   int          initialised;
   int          started;
   
   /* These are per-channel */
   int32_t  last_sample[MAX_NB_CHANNELS];
   uint32_t samp_frac_num[MAX_NB_CHANNELS];
   uint32_t magic_samples[MAX_NB_CHANNELS];
   
#ifdef SUPPORT_QUALITY_0_TO_3
#if SUPPORT_QUALITY_0_TO_3
   int16_t mem[MAX_NB_CHANNELS*96];//support for max. rank 3
#endif
#endif
#ifdef SUPPORT_QUALITY_0_TO_10
#if SUPPORT_QUALITY_0_TO_10
   int16_t mem[MAX_NB_CHANNELS*512]; //support for max. rank 10
#endif
#endif
   int16_t *sinc_table;
   uint32_t sinc_table_length;
   resampler_basic_func resampler_ptr;
         
   int    in_stride;
   int    out_stride;
} ;


/** Create a new resampler with integer input and output rates.
 * nb_channels: Number of channels to be processed
 * in_rate: Input sampling rate (integer number of Hz).
 * out_rate: Output sampling rate (integer number of Hz).
 * quality: Resampling quality between 0 and 10.
 * return: resampler state.
 */
ResamplerState *resampler_init(uint32_t nb_channels, 
                                          uint32_t in_rate, 
                                          uint32_t out_rate, 
                                          int quality,
                                          int *err);

ResamplerState *resampler_init_frac(uint32_t nb_channels, 
                                               uint32_t ratio_num, 
                                               uint32_t ratio_den, 
                                               uint32_t in_rate, 
                                               uint32_t out_rate, 
                                               int quality,
                                               int *err);

int resampler_set_quality(ResamplerState *st, 
                                 int quality);


int resampler_set_rate_frac(ResamplerState *st, 
                                   uint32_t ratio_num, 
                                   uint32_t ratio_den, 
                                   uint32_t in_rate, 
                                   uint32_t out_rate);


/** Resample an int array. The input and output buffers must *not* overlap.
 * st: Resampler state
 * channel_index: Index of the channel to process for the multi-channel 
 * in: Input buffer
 * in_len: Number of input samples in the input buffer. Returns the number of samples processed
 * out: Output buffer
 * out_len: Size of the output buffer. Returns the number of samples written
 */
int resampler_process_int(ResamplerState *st, 
                                 uint32_t channel_index, 
                                 const int16_t *in, 
                                 uint32_t *in_len, 
                                 int16_t *out, 
                                 uint32_t *out_len);

static inline 
int resampler_process_int_ex( ResamplerState *st, 
                                 uint32_t channel_index, 
                                 const int16_t *in, 
                                 uint32_t in_len_n, 
                                 int16_t *out, 
                                 uint32_t out_len_n,
                                 const char *err_format)
{
	uint32_t in_len = in_len_n;
	uint32_t out_len = out_len_n;
	int err;
	
	err = resampler_process_int( st, channel_index, in, &in_len, out, &out_len );
	
	if( err )
		printk( err_format, err );
	
	return err;
}

/********************** API Usage ***********************
for (;;)
{

  1. Init the resampler state for up/down sampling.
  Ex:
		if (init_flag[chid] == 0)
		{
			resampler_up = resampler_init(chid+1, 8000, 16000, 1, &err);
			resampler_down = resampler_init(chid+1, 16000, 8000, 1, &err1);
			init_flag[chid] = 1;
		}
  2. Process Up/Down sampling.
  Ex:
  		// Up Sampling 
		in_length = 80;
		out_length = 160;
					
		err = resampler_process_int(resampler_up, chid, in, &in_length, upbuf, &out_length);

		if (err != RESAMPLER_ERR_SUCCESS)	
		{
			printk("Up error = %d\n", err);
		}
													
		// Down Sampling 
		in_length = 160;
		out_length = 80;	
																	
		err1 = resampler_process_int(resampler_down, chid, upbuf, &in_length, down_buf, &out_length);

		if (err1 != RESAMPLER_ERR_SUCCESS)	
		{
			printk("Down error = %d\n", err1);
		}
}

*********************************************************/

extern ResamplerState* pResampler_down_st;
extern ResamplerState* pResampler_up_st;

#endif
