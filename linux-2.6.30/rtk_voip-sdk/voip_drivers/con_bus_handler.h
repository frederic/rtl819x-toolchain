#ifndef __CON_BUS_HANDLER_H__
#define __CON_BUS_HANDLER_H__

// con_main call this to init bus handler tasklet 
extern void bus_handler_tasklet_init( void );

// bus ISR call this to schedule bus handler tasklet 
extern void isr_schedule_bus_handler( void );

// dsp call this to schedule bus handler tasklet again 
extern void dsp_schedule_bus_handler( void );

// when system is going to shutdown, we use this to kill handler 
extern void kill_bus_handler( void );


#endif /* __CON_BUS_HANDLER_H__ */

