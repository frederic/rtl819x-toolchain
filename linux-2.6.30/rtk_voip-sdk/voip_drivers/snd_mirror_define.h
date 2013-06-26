#ifndef __SND_MIRROR_DEFINE_H__
#define __SND_MIRROR_DEFINE_H__

#include "voip_types.h"

/*
 * Structures in this file are used to mirror data in packets. 
 */

typedef struct {
	uint16 hook_status;		// RAM 0
	uint16 ringing;			// RAM 1
	uint16 SendNTTCAR_chk;	// RAM 2
	uint16 verbose;			// RAM 3: debug purpose 
} mirror_slic_priv_t;

typedef struct {
	uint16 hook_status;		// RAM 0
	uint16 pol_rev_det;		// RAM 1
	uint16 bat_dropout_det;	// RAM 2
	uint16 pos_neg_ring;	// RAM 3
	uint16 polarity;		// RAM 4: hook_status == 1
	uint16 line_voltage;	// RAM 5: hook_status == 1
	uint16 verbose;			// RAM 6: debug purpose 
} mirror_daa_priv_t;

#endif /* __SND_MIRROR_DEFINE_H__ */

