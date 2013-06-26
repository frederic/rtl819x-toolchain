#ifndef __LEVEL_TRACKING_H__
#define __LEVEL_TRACKING_H__

/* reset singla/noise cluster */
extern void reset_level_tracking_cluster( uint32 sid );

/* get singla/noise cluster */
extern void get_level_tracking_cluster( uint32 sid, 
						uint32 *in_signal_q16, uint32 *in_noise_q16,
						uint32 *out_signal_q16, uint32 *out_noise_q16 );

/* turn on/off silence notification */
extern void enable_level_tracking_silence_notify( uint32 sid, uint32 enable );

/* set silence notification threshold (ms) */
extern void set_level_tracking_silence_notify_threshold( uint32 sid, uint32 period );

/* log energy in/out level */
extern void record_level_tracking( uint32 sid,
									uint32 energy_in, uint32 energy_out );



#endif // __LEVEL_TRACKING_H__ 

