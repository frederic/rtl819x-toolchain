#ifndef __CON_IPC_HELP_HOST_H__
#define __CON_IPC_HELP_HOST_H__

#include "voip_types.h"
#include "con_register.h"

extern void ipc_assign_dsp_cpuid_cch( voip_con_t voip_con[], int num );
extern void ipc_construct_host_cch_invert_table( voip_con_t voip_con[] );
extern uint32 ipc_get_host_cch_from_invert_table( uint32 dsp_cpuid, uint32 dsp_cch );
extern void ipc_print_host_cch_invert_table( void );


#endif /* __CON_IPC_HELP_HOST_H__ */

