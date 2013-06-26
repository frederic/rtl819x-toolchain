cmd_arch/rlx/bsp/irq.o := rsdk-linux-gcc -Wp,-MD,arch/rlx/bsp/.irq.o.d  -nostdinc -isystem /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/toolchain/rsdk-1.3.6-4181-EB-2.6.30-0.9.30/bin/../lib/gcc/mips-linux/3.4.6-1.3.6/include -Iinclude  -I/home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include -include include/linux/autoconf.h -D__KERNEL__ -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -Werror-implicit-function-declaration -fno-delete-null-pointer-checks -Os -ffunction-sections -mno-check-zero-division -mabi=32 -G 0 -mno-abicalls -fno-pic -pipe -msoft-float -ffreestanding -EB -UMIPSEB -U_MIPSEB -U__MIPSEB -U__MIPSEB__ -UMIPSEL -U_MIPSEL -U__MIPSEL -U__MIPSEL__ -DMIPSEB -D_MIPSEB -D__MIPSEB -D__MIPSEB__ -Iinclude/asm-rlx -Iarch/rlx/bsp/ -I/home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/mach-generic -D"VMLINUX_LOAD_ADDRESS=0x80000000" -fomit-frame-pointer -Wdeclaration-after-statement -I/home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/drivers/net/rtl819x/AsicDriver -I/home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/drivers/net/rtl819x/common -DCONFIG_RTL_819X_SWCORE   -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(irq)"  -D"KBUILD_MODNAME=KBUILD_STR(irq)"  -c -o arch/rlx/bsp/irq.o arch/rlx/bsp/irq.c

deps_arch/rlx/bsp/irq.o := \
  arch/rlx/bsp/irq.c \
    $(wildcard include/config/rtl/usb/otg.h) \
    $(wildcard include/config/rtk/voip.h) \
    $(wildcard include/config/pcie/power/saving.h) \
    $(wildcard include/config/rtl/819x.h) \
    $(wildcard include/config/rtl/8198/nfbi/board.h) \
    $(wildcard include/config/usb.h) \
    $(wildcard include/config/dwc/otg.h) \
    $(wildcard include/config/arch/suspend/possible.h) \
    $(wildcard include/config/rtl819x/suspend/check/interrupt.h) \
  include/linux/errno.h \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/errno.h \
  include/asm-generic/errno-base.h \
  include/linux/init.h \
    $(wildcard include/config/modules.h) \
    $(wildcard include/config/hotplug.h) \
  include/linux/compiler.h \
    $(wildcard include/config/trace/branch/profiling.h) \
    $(wildcard include/config/profile/all/branches.h) \
    $(wildcard include/config/enable/must/check.h) \
    $(wildcard include/config/enable/warn/deprecated.h) \
  include/linux/compiler-gcc.h \
    $(wildcard include/config/arch/supports/optimized/inlining.h) \
    $(wildcard include/config/optimize/inlining.h) \
  include/linux/compiler-gcc3.h \
  include/linux/section-names.h \
  include/linux/stringify.h \
  include/linux/kernel_stat.h \
    $(wildcard include/config/generic/hardirqs.h) \
  include/linux/smp.h \
    $(wildcard include/config/smp.h) \
    $(wildcard include/config/use/generic/smp/helpers.h) \
    $(wildcard include/config/debug/preempt.h) \
  include/linux/types.h \
    $(wildcard include/config/uid16.h) \
    $(wildcard include/config/lbd.h) \
    $(wildcard include/config/phys/addr/t/64bit.h) \
    $(wildcard include/config/64bit.h) \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/types.h \
  include/asm-generic/int-ll64.h \
  include/asm-generic/bitsperlong.h \
  include/linux/posix_types.h \
  include/linux/stddef.h \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/posix_types.h \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/sgidefs.h \
  include/linux/list.h \
    $(wildcard include/config/debug/list.h) \
  include/linux/poison.h \
  include/linux/prefetch.h \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/processor.h \
    $(wildcard include/config/cpu/has/sleep.h) \
    $(wildcard include/config/rtl8197b/pana.h) \
    $(wildcard include/config/rtl/8196c.h) \
    $(wildcard include/config/rtl/819xd.h) \
    $(wildcard include/config/rtl/8196e.h) \
  include/linux/cpumask.h \
    $(wildcard include/config/disable/obsolete/cpumask/functions.h) \
    $(wildcard include/config/hotplug/cpu.h) \
    $(wildcard include/config/cpumask/offstack.h) \
    $(wildcard include/config/debug/per/cpu/maps.h) \
  include/linux/kernel.h \
    $(wildcard include/config/preempt/voluntary.h) \
    $(wildcard include/config/debug/spinlock/sleep.h) \
    $(wildcard include/config/prove/locking.h) \
    $(wildcard include/config/printk.h) \
    $(wildcard include/config/panic/printk.h) \
    $(wildcard include/config/dynamic/debug.h) \
    $(wildcard include/config/ring/buffer.h) \
    $(wildcard include/config/tracing.h) \
    $(wildcard include/config/numa.h) \
    $(wildcard include/config/ftrace/mcount/record.h) \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/toolchain/rsdk-1.3.6-4181-EB-2.6.30-0.9.30/bin/../lib/gcc/mips-linux/3.4.6-1.3.6/include/stdarg.h \
  include/linux/linkage.h \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/linkage.h \
  include/linux/bitops.h \
    $(wildcard include/config/generic/find/first/bit.h) \
    $(wildcard include/config/generic/find/last/bit.h) \
    $(wildcard include/config/generic/find/next/bit.h) \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/bitops.h \
    $(wildcard include/config/cpu/has/llsc.h) \
    $(wildcard include/config/cpu/rlx4181.h) \
    $(wildcard include/config/cpu/rlx5181.h) \
    $(wildcard include/config/cpu/rlx5281.h) \
    $(wildcard include/config/cpu/has/radiax.h) \
  include/linux/irqflags.h \
    $(wildcard include/config/trace/irqflags.h) \
    $(wildcard include/config/irqsoff/tracer.h) \
    $(wildcard include/config/preempt/tracer.h) \
    $(wildcard include/config/trace/irqflags/support.h) \
  include/linux/typecheck.h \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/irqflags.h \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/hazards.h \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/cpu-features.h \
    $(wildcard include/config/cpu/has/ejtag.h) \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/cpu.h \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/cpu-info.h \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/cache.h \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/mach-generic/kmalloc.h \
    $(wildcard include/config/dma/coherent.h) \
  arch/rlx/bsp/bspcpu.h \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/barrier.h \
    $(wildcard include/config/cpu/has/sync.h) \
    $(wildcard include/config/cpu/has/wb.h) \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/bug.h \
    $(wildcard include/config/bug.h) \
  include/asm-generic/bug.h \
    $(wildcard include/config/generic/bug.h) \
    $(wildcard include/config/generic/bug/relative/pointers.h) \
    $(wildcard include/config/debug/bugverbose.h) \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/byteorder.h \
    $(wildcard include/config/cpu/big/endian.h) \
  include/linux/byteorder/big_endian.h \
  include/linux/swab.h \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/swab.h \
  include/linux/byteorder/generic.h \
  include/asm-generic/bitops/non-atomic.h \
  include/asm-generic/bitops/fls64.h \
  include/asm-generic/bitops/ffz.h \
  include/asm-generic/bitops/find.h \
  include/asm-generic/bitops/sched.h \
  include/asm-generic/bitops/hweight.h \
  include/asm-generic/bitops/ext2-non-atomic.h \
  include/asm-generic/bitops/le.h \
  include/asm-generic/bitops/ext2-atomic.h \
  include/asm-generic/bitops/minix.h \
  include/linux/log2.h \
    $(wildcard include/config/arch/has/ilog2/u32.h) \
    $(wildcard include/config/arch/has/ilog2/u64.h) \
  include/linux/ratelimit.h \
  include/linux/param.h \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/param.h \
    $(wildcard include/config/hz.h) \
  include/linux/dynamic_debug.h \
  include/linux/threads.h \
    $(wildcard include/config/nr/cpus.h) \
    $(wildcard include/config/base/small.h) \
  include/linux/bitmap.h \
  include/linux/string.h \
    $(wildcard include/config/binary/printf.h) \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/string.h \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/cachectl.h \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/rlxregs.h \
    $(wildcard include/config/cpu/rlx4281.h) \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/system.h \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/addrspace.h \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/mach-generic/spaces.h \
    $(wildcard include/config/32bit.h) \
    $(wildcard include/config/dma/noncoherent.h) \
  include/linux/const.h \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/cmpxchg.h \
  include/linux/percpu.h \
    $(wildcard include/config/have/dynamic/per/cpu/area.h) \
  include/linux/preempt.h \
    $(wildcard include/config/preempt.h) \
    $(wildcard include/config/preempt/notifiers.h) \
  include/linux/thread_info.h \
    $(wildcard include/config/compat.h) \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/thread_info.h \
    $(wildcard include/config/kernel/stack/size/order.h) \
    $(wildcard include/config/debug/stack/usage.h) \
  include/linux/slab.h \
    $(wildcard include/config/slab/debug.h) \
    $(wildcard include/config/debug/objects.h) \
    $(wildcard include/config/slub.h) \
    $(wildcard include/config/slob.h) \
    $(wildcard include/config/debug/slab.h) \
  include/linux/gfp.h \
    $(wildcard include/config/zone/dma.h) \
    $(wildcard include/config/zone/dma32.h) \
    $(wildcard include/config/highmem.h) \
  include/linux/mmzone.h \
    $(wildcard include/config/force/max/zoneorder.h) \
    $(wildcard include/config/memory/hotplug.h) \
    $(wildcard include/config/sparsemem.h) \
    $(wildcard include/config/arch/populates/node/map.h) \
    $(wildcard include/config/discontigmem.h) \
    $(wildcard include/config/flat/node/mem/map.h) \
    $(wildcard include/config/cgroup/mem/res/ctlr.h) \
    $(wildcard include/config/have/memory/present.h) \
    $(wildcard include/config/need/node/memmap/size.h) \
    $(wildcard include/config/need/multiple/nodes.h) \
    $(wildcard include/config/have/arch/early/pfn/to/nid.h) \
    $(wildcard include/config/flatmem.h) \
    $(wildcard include/config/sparsemem/extreme.h) \
    $(wildcard include/config/nodes/span/other/nodes.h) \
    $(wildcard include/config/holes/in/zone.h) \
    $(wildcard include/config/arch/has/holes/memorymodel.h) \
  include/linux/spinlock.h \
    $(wildcard include/config/debug/spinlock.h) \
    $(wildcard include/config/generic/lockbreak.h) \
    $(wildcard include/config/debug/lock/alloc.h) \
  include/linux/bottom_half.h \
  include/linux/spinlock_types.h \
  include/linux/spinlock_types_up.h \
  include/linux/lockdep.h \
    $(wildcard include/config/lockdep.h) \
    $(wildcard include/config/lock/stat.h) \
  include/linux/spinlock_up.h \
  include/linux/spinlock_api_up.h \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/atomic.h \
  include/asm-generic/atomic-long.h \
  include/linux/wait.h \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/current.h \
  include/linux/cache.h \
    $(wildcard include/config/arch/has/cache/line/size.h) \
  include/linux/numa.h \
    $(wildcard include/config/nodes/shift.h) \
  include/linux/seqlock.h \
  include/linux/nodemask.h \
  include/linux/pageblock-flags.h \
    $(wildcard include/config/hugetlb/page.h) \
    $(wildcard include/config/hugetlb/page/size/variable.h) \
  include/linux/bounds.h \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/page.h \
  include/linux/pfn.h \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/io.h \
  include/asm-generic/iomap.h \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/pgtable-bits.h \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/mach-generic/ioremap.h \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/mach-generic/mangle-port.h \
    $(wildcard include/config/swap/io/space.h) \
  include/asm-generic/memory_model.h \
    $(wildcard include/config/sparsemem/vmemmap.h) \
  include/asm-generic/getorder.h \
  include/linux/memory_hotplug.h \
    $(wildcard include/config/have/arch/nodedata/extension.h) \
    $(wildcard include/config/memory/hotremove.h) \
  include/linux/notifier.h \
  include/linux/mutex.h \
    $(wildcard include/config/debug/mutexes.h) \
  include/linux/rwsem.h \
    $(wildcard include/config/rwsem/generic/spinlock.h) \
  include/linux/rwsem-spinlock.h \
  include/linux/srcu.h \
  include/linux/topology.h \
    $(wildcard include/config/sched/smt.h) \
    $(wildcard include/config/sched/mc.h) \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/topology.h \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/mach-generic/topology.h \
  include/asm-generic/topology.h \
  include/linux/mmdebug.h \
    $(wildcard include/config/debug/vm.h) \
    $(wildcard include/config/debug/virtual.h) \
  include/linux/slab_def.h \
    $(wildcard include/config/kmemtrace.h) \
  include/trace/kmemtrace.h \
  include/linux/tracepoint.h \
    $(wildcard include/config/tracepoints.h) \
  include/linux/rcupdate.h \
    $(wildcard include/config/classic/rcu.h) \
    $(wildcard include/config/tree/rcu.h) \
    $(wildcard include/config/preempt/rcu.h) \
  include/linux/completion.h \
  include/linux/rcuclassic.h \
    $(wildcard include/config/rcu/cpu/stall/detector.h) \
  include/linux/kmalloc_sizes.h \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/percpu.h \
  include/asm-generic/percpu.h \
    $(wildcard include/config/have/setup/per/cpu/area.h) \
  include/linux/percpu-defs.h \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/irq.h \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/mach-generic/irq.h \
    $(wildcard include/config/i8259.h) \
    $(wildcard include/config/irq/cpu.h) \
    $(wildcard include/config/irq/cpu/rm7k.h) \
    $(wildcard include/config/irq/cpu/rm9k.h) \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/cputime.h \
  include/asm-generic/cputime.h \
  include/linux/time.h \
  include/linux/math64.h \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/div64.h \
  include/asm-generic/div64.h \
  include/linux/jiffies.h \
  include/linux/timex.h \
    $(wildcard include/config/no/hz.h) \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/timex.h \
  include/linux/irq.h \
    $(wildcard include/config/s390.h) \
    $(wildcard include/config/irq/per/cpu.h) \
    $(wildcard include/config/irq/release/method.h) \
    $(wildcard include/config/intr/remap.h) \
    $(wildcard include/config/generic/pending/irq.h) \
    $(wildcard include/config/proc/fs.h) \
    $(wildcard include/config/sparse/irq.h) \
    $(wildcard include/config/numa/migrate/irq/desc.h) \
    $(wildcard include/config/generic/hardirqs/no//do/irq.h) \
    $(wildcard include/config/cpumasks/offstack.h) \
  include/linux/irqreturn.h \
  include/linux/irqnr.h \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/ptrace.h \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/isadep.h \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/irq_regs.h \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/hw_irq.h \
  include/linux/signal.h \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/signal.h \
    $(wildcard include/config/trad/signals.h) \
  include/asm-generic/signal-defs.h \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/sigcontext.h \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/siginfo.h \
  include/asm-generic/siginfo.h \
  include/linux/sched.h \
    $(wildcard include/config/sched/debug.h) \
    $(wildcard include/config/detect/softlockup.h) \
    $(wildcard include/config/detect/hung/task.h) \
    $(wildcard include/config/core/dump/default/elf/headers.h) \
    $(wildcard include/config/bsd/process/acct.h) \
    $(wildcard include/config/taskstats.h) \
    $(wildcard include/config/audit.h) \
    $(wildcard include/config/inotify/user.h) \
    $(wildcard include/config/epoll.h) \
    $(wildcard include/config/posix/mqueue.h) \
    $(wildcard include/config/keys.h) \
    $(wildcard include/config/user/sched.h) \
    $(wildcard include/config/sysfs.h) \
    $(wildcard include/config/schedstats.h) \
    $(wildcard include/config/task/delay/acct.h) \
    $(wildcard include/config/fair/group/sched.h) \
    $(wildcard include/config/rt/group/sched.h) \
    $(wildcard include/config/blk/dev/io/trace.h) \
    $(wildcard include/config/x86/ptrace/bts.h) \
    $(wildcard include/config/sysvipc.h) \
    $(wildcard include/config/auditsyscall.h) \
    $(wildcard include/config/rt/mutexes.h) \
    $(wildcard include/config/task/xacct.h) \
    $(wildcard include/config/cpusets.h) \
    $(wildcard include/config/cgroups.h) \
    $(wildcard include/config/futex.h) \
    $(wildcard include/config/fault/injection.h) \
    $(wildcard include/config/latencytop.h) \
    $(wildcard include/config/function/graph/tracer.h) \
    $(wildcard include/config/have/unstable/sched/clock.h) \
    $(wildcard include/config/preempt/bkl.h) \
    $(wildcard include/config/group/sched.h) \
    $(wildcard include/config/mm/owner.h) \
  include/linux/capability.h \
    $(wildcard include/config/security/file/capabilities.h) \
  include/linux/rbtree.h \
  include/linux/mm_types.h \
    $(wildcard include/config/split/ptlock/cpus.h) \
    $(wildcard include/config/want/page/debug/flags.h) \
    $(wildcard include/config/mmu.h) \
    $(wildcard include/config/mmu/notifier.h) \
  include/linux/auxvec.h \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/auxvec.h \
  include/linux/prio_tree.h \
  include/linux/page-debug-flags.h \
    $(wildcard include/config/page/poisoning.h) \
    $(wildcard include/config/page/debug/something/else.h) \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/mmu.h \
  include/linux/sem.h \
  include/linux/ipc.h \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/ipcbuf.h \
  include/linux/kref.h \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/sembuf.h \
  include/linux/path.h \
  include/linux/pid.h \
  include/linux/proportions.h \
  include/linux/percpu_counter.h \
  include/linux/seccomp.h \
    $(wildcard include/config/seccomp.h) \
  include/linux/rtmutex.h \
    $(wildcard include/config/debug/rt/mutexes.h) \
  include/linux/plist.h \
    $(wildcard include/config/debug/pi/list.h) \
  include/linux/resource.h \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/resource.h \
  include/asm-generic/resource.h \
  include/linux/timer.h \
    $(wildcard include/config/timer/stats.h) \
    $(wildcard include/config/debug/objects/timers.h) \
  include/linux/ktime.h \
    $(wildcard include/config/ktime/scalar.h) \
  include/linux/debugobjects.h \
    $(wildcard include/config/debug/objects/free.h) \
  include/linux/hrtimer.h \
    $(wildcard include/config/high/res/timers.h) \
  include/linux/task_io_accounting.h \
    $(wildcard include/config/task/io/accounting.h) \
  include/linux/kobject.h \
  include/linux/sysfs.h \
  include/linux/latencytop.h \
  include/linux/cred.h \
    $(wildcard include/config/security.h) \
  include/linux/key.h \
    $(wildcard include/config/sysctl.h) \
  include/linux/sysctl.h \
    $(wildcard include/config/rtl/nf/conntrack/garbage/new.h) \
  include/linux/aio.h \
    $(wildcard include/config/aio.h) \
  include/linux/workqueue.h \
  include/linux/aio_abi.h \
  include/linux/uio.h \
  include/linux/interrupt.h \
    $(wildcard include/config/pm/sleep.h) \
    $(wildcard include/config/generic/irq/probe.h) \
    $(wildcard include/config/debug/shirq.h) \
  include/linux/hardirq.h \
    $(wildcard include/config/virt/cpu/accounting.h) \
  include/linux/ftrace_irq.h \
    $(wildcard include/config/ftrace/nmi/enter.h) \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/hardirq.h \
  include/linux/irq_cpustat.h \
  include/linux/ioport.h \
  include/linux/random.h \
  include/linux/ioctl.h \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/ioctl.h \
  include/asm-generic/ioctl.h \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/bootinfo.h \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/setup.h \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/irq_cpu.h \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/irq_vec.h \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/rlxbsp.h \
  include/net/rtl/rtl_types.h \
    $(wildcard include/config/rtl865x/nicdrv2.h) \
    $(wildcard include/config/rtl/dynamic/iram/mapping/for/wapi.h) \
    $(wildcard include/config/rtl/ulinker/brsc.h) \
    $(wildcard include/config/rtl/eth/priv/skb.h) \
    $(wildcard include/config/rtl8196c/eth/iot.h) \
    $(wildcard include/config/mp/psd/support.h) \
    $(wildcard include/config/rtl8196c/green/ethernet.h) \
    $(wildcard include/config/rtl/8196c/esd.h) \
    $(wildcard include/config/rtl/8198.h) \
    $(wildcard include/config/rtl/8198/esd.h) \
    $(wildcard include/config/rtl/8197d/dyn/thr.h) \
    $(wildcard include/config/rtl/819xdt.h) \
    $(wildcard include/config/rtl/log/debug.h) \
  include/linux/version.h \
  include/linux/module.h \
    $(wildcard include/config/modversions.h) \
    $(wildcard include/config/unused/symbols.h) \
    $(wildcard include/config/kallsyms.h) \
    $(wildcard include/config/markers.h) \
    $(wildcard include/config/module/unload.h) \
  include/linux/stat.h \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/stat.h \
  include/linux/kmod.h \
  include/linux/elf.h \
  include/linux/elf-em.h \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/elf.h \
  include/linux/moduleparam.h \
    $(wildcard include/config/alpha.h) \
    $(wildcard include/config/ia64.h) \
    $(wildcard include/config/ppc64.h) \
  include/linux/marker.h \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/local.h \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/module.h \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/uaccess.h \
  arch/rlx/bsp/bspchip.h \
    $(wildcard include/config/fpga/platform.h) \
    $(wildcard include/config/rtl/92d/support.h) \
    $(wildcard include/config/rtl/usb/ip/host/speedup.h) \
  include/linux/proc_fs.h \
    $(wildcard include/config/proc/devicetree.h) \
    $(wildcard include/config/proc/kcore.h) \
  include/linux/fs.h \
    $(wildcard include/config/dnotify.h) \
    $(wildcard include/config/quota.h) \
    $(wildcard include/config/inotify.h) \
    $(wildcard include/config/debug/writecount.h) \
    $(wildcard include/config/file/locking.h) \
    $(wildcard include/config/block.h) \
    $(wildcard include/config/fs/xip.h) \
    $(wildcard include/config/migration.h) \
  include/linux/limits.h \
  include/linux/kdev_t.h \
  include/linux/dcache.h \
  include/linux/rculist.h \
  include/linux/radix-tree.h \
  include/linux/semaphore.h \
  include/linux/fiemap.h \
  include/linux/quota.h \
  include/linux/dqblk_xfs.h \
  include/linux/dqblk_v1.h \
  include/linux/dqblk_v2.h \
  include/linux/dqblk_qtree.h \
  include/linux/nfs_fs_i.h \
  include/linux/nfs.h \
  include/linux/sunrpc/msg_prot.h \
  include/linux/fcntl.h \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/fcntl.h \
  include/asm-generic/fcntl.h \
  include/linux/err.h \
  include/linux/magic.h \

arch/rlx/bsp/irq.o: $(deps_arch/rlx/bsp/irq.o)

$(deps_arch/rlx/bsp/irq.o):
