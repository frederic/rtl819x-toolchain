#include <linux/proc_fs.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#ifdef CONFIG_ARCH_CPU_WMPU
#include <linux/ptrace.h>
#endif
#include "rtk_voip.h"
#include "voip_init.h"
#include "voip_proc.h"

#if defined (CONFIG_AUDIOCODES_VOIP) && defined (CONFIG_RTK_VOIP_DRIVERS_IP_PHONE) || defined (CONFIG_RTK_VOIP_DECT_UART_SUPPORT)
int rtk_dbg_level = RTK_DBG_ERROR;
#else
//int rtk_dbg_level = RTK_DBG_INFO;
int rtk_dbg_level = RTK_DBG_ERROR;
#endif

static int __init rtk_voip_init(void)
{
	PRINT_MSG("rtk_voip_init\n");
	return 0;
}

static void __exit rtk_voip_exit(void)
{
	PRINT_MSG("rtk_voip_exit\n");
}

#ifdef CONFIG_RTK_VOIP_DRIVERS_11N_MP
int PCM_Check_Any_ChanEnabled(void)
{
	return 0;
}
#endif

voip_initcall(rtk_voip_init);
voip_exitcall(rtk_voip_exit);



#ifdef CONFIG_ARCH_CPU_WMPU
static unsigned char test[2];

void memwatch_test_init(unsigned char* ptr)
{
	struct wmpu_addr addr;
	struct wmpu_addr *p = &addr;

#if 0
	p->start = test;
	p->end = test;
#else
	p->start = ptr;
	p->end = ptr;
#endif
	p->attr = WMPU_DW;
	ptrace_wmpu_wp(p);
}

void memwatch_test_run(unsigned char* ptr)
{
#if 0
	test[0] = 0xFF;
#else
	*ptr = 0xFF;
#endif
}
#endif


static int voip_debug_read(char *page, char **start, off_t off,
        int count, int *eof, void *data)
{
	int chid, sid;
	int n;

	if( off ) {	/* In our case, we write out all data at once. */
		*eof = 1;
		return 0;
	}

	n = sprintf(page, "VoIP Debug Information:\n");
	n += sprintf(page+n, "  - VoIP Debug Message Level: %d\n", rtk_dbg_level);

	*eof = 1;
	return n;
}

static int voip_debug_write(struct file *file, const char *buffer, 
                               unsigned long count, void *data)
{
	char tmp[128];
	int dbg_val;

	if (count < 2)
		return -EFAULT;

	if (buffer && !copy_from_user(tmp, buffer, 128)) {
		sscanf(tmp, "%d", &dbg_val);

		printk("dbg_val = %d\n", dbg_val);

		if (dbg_val <= RTK_DBG_MAX)
			rtk_dbg_level = dbg_val;
		
#ifdef CONFIG_ARCH_CPU_WMPU
		if (dbg_val == 80)
			memwatch_test_init();
		else if (dbg_val == 81)
			memwatch_test_run();
#endif
	}

	return count;
}

static int __init voip_proc_debug_init(void)
{
	struct proc_dir_entry *voip_debug_proc;

	voip_debug_proc = create_voip_proc_rw_entry(PROC_VOIP_DIR "/debug", 0, NULL,
						voip_debug_read, voip_debug_write);
	
	if (voip_debug_proc == NULL)
	{
		printk("voip_debug_proc NULL!! \n");
		return -1;
	}
	
	return 0;
}

static void __exit voip_proc_debug_exit( void )
{
	remove_voip_proc_entry( PROC_VOIP_DIR "/debug", NULL );
}

voip_initcall_proc( voip_proc_debug_init );
voip_exitcall( voip_proc_debug_exit );

#ifndef CONFIG_PRINTK
// printk wrapper
int __wrap_printk(const char *fmt, ...)
{
#if defined(CONFIG_PANIC_PRINTK) || defined(CONFIG_PRINTK_FUNC)
    va_list args;
    int r;

    va_start(args, fmt);
#ifdef CONFIG_PANIC_PRINTK
    r = vprintk(fmt, args);
#else
	r = scrlog_vprintk(fmt, args);
#endif
    va_end(args);

    return r;
#else
	return 0;
#endif
}
#endif

