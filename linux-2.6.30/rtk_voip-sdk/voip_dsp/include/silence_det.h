#ifndef __SILENCE_DET_H__
#define __SILENCE_DET_H__

// set silence threshold once! 
// energy_neg = 0 (louder)~127. This is a negative value, and was given from web 
// period (ms), and 1000 is suggested (10 seconds)
extern void set_silence_det_threshold( uint32 sid, uint32 energy_neg, uint32 period );

// enable/disable silence (T.38, V.152 VBD, and so on)
extern void enable_silence_det( uint32 sid, int enable );

// record silence 
extern void record_silence_det( uint32 sid, uint32 energy_in, uint32 energy_out );


#endif /* __SILENCE_DET_H__ */

