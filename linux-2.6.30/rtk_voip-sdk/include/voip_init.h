#ifndef __VOIP_INIT_H__
#define __VOIP_INIT_H__

#include <linux/init.h>

#if defined(CONFIG_DEFAULTS_KERNEL_2_6) && !defined(MODULE)

/*
 * Execution precedence: 
 *  - kernel's initcall 		precedence 
 *  - voip_initcall_entry()		0
 *  - voip_initcall_bus()		1
 *  - voip_initcall_ioc_relay()	2
 *  - voip_initcall_snd_setup()	3	<-- setup for snd  
 *  - voip_initcall_snd_fts()	4	fixed timeslot SND, e.g. DECT 
 *  - voip_initcall_snd()		5
 *  - voip_initcall_ioc()		6
 *  - voip_initcall_dsp()		7
 *  - voip_initcall_con()		8
 *  ============================================= (binding ok!)
 *  - voip_initcall()			1	(*use this normally)
 *  - voip_initcall_proc() 		2
 *  - voip_initcall_led() 		1
 *  - voip_initcall_sync()		<--- the last one 
 */
#define voip_initcall_entry(fn)		__define_initcall("98",fn,98)
#define voip_initcall_bus(fn)		__define_initcall("98a",fn,98a)
#define voip_initcall_ioc_relay(fn)	__define_initcall("98b",fn,98a)
#define voip_initcall_snd_setup(fn)	__define_initcall("98c",fn,98b)
#define voip_initcall_snd_fts(fn)	__define_initcall("98d",fn,98b)
#define voip_initcall_snd(fn)		__define_initcall("98e",fn,98b)
#define voip_initcall_ioc(fn)		__define_initcall("98f",fn,98c)
#define voip_initcall_dsp(fn)		__define_initcall("98g",fn,98c)
#define voip_initcall_con(fn)		__define_initcall("98h",fn,98d)

#define voip_initcall(fn)			__define_initcall("99",fn,99)
#define voip_initcall_proc(fn)		__define_initcall("99a",fn,99a)
#define voip_initcall_led(fn)		__define_initcall("99a",fn,99a)
#define voip_initcall_sync(fn)		__define_initcall("99s",fn,99s)

#else

#define voip_initcall(fn)		module_init(fn)
#define voip_initcall_proc(fn)	module_init(fn)
#define voip_initcall_sync(fn)	module_init(fn)

#endif

/* rename to be more clear */
#define voip_exitcall(fn)		module_exit(fn)

#endif /* __VOIP_INIT_H__ */

