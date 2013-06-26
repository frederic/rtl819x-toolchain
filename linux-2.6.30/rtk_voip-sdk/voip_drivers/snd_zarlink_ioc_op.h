#ifndef __SND_ZARLINK_IOC_OP_H__
#define __SND_ZARLINK_IOC_OP_H__

#include "voip_types.h"
#include "con_register.h"

#include "zarlink_api.h"

// Give IO a initial state 
extern void InitializeZarlinkIO( voip_ioc_t *this );

// zarlink's ioc priv
typedef struct {
	VPIO IO;
	void *snd_priv;
} zarlink_ioc_priv_t;

extern ioc_ops_t ioc_led_ops_zarlink;
extern ioc_ops_t ioc_relay_ops_zarlink;

// VPIO list 
extern VPIO zarlink_VPIO_list[];
extern int  zarlink_VPIO_list_num;

#endif /* __SND_ZARLINK_IOC_OP_H__ */

