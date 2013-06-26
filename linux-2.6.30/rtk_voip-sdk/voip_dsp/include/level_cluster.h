#ifndef __LEVEL_CLUSTER_H__
#define __LEVEL_CLUSTER_H__

#include "voip_types.h"

// initial level cluster variable (encode input / decode output) 
extern void init_level_cluster( uint32 sid );

// record energy (encode input / decode output) 
extern void record_level_cluster( uint32 sid,
									uint32 energy_in, uint32 energy_out );

// get signal/noise energy (encode input / decode output) 
extern int get_level_cluster( uint32 sid, 
						uint32 *in_signal_lv, uint32 *in_noise_lv,
						uint32 *out_signal_lv, uint32 *out_noise_lv
							 );

#endif /* __LEVEL_CLUSTER_H__ */


