#include "rtk_voip.h"
#include "voip_types.h"
#include "voip_timer.h"


static void Function_Event_Polling_Use_Timer(void *data)
{
	/*
	 * IPC host side doesn't process SLIC, DAA hook and so on, 
	 * because these status may reference to DSP's result. 
	 */
#ifdef CONFIG_RTK_VOIP_CON_POLLING_PROCESS
	extern void START_Event_Polling_Use_Timer(unsigned long data);
#else
	extern void Polling_Mirror_SND(unsigned long data);
#endif

#ifdef CONFIG_RTK_VOIP_CON_POLLING_PROCESS
	START_Event_Polling_Use_Timer( data );
#else 
	// Get hook status (keep in buffer), and mirror to DSP 
	Polling_Mirror_SND( data );
#endif
	
}

void Init_Event_Polling_Use_Timer( unsigned long data )
{
	register_timer_10ms( Function_Event_Polling_Use_Timer, ( void * )data );
	
	PRINT_MSG("Add Event Timer For Polling\n");
}

