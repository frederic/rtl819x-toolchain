cmd_arch/rlx/bsp/prom.o := rsdk-linux-gcc -Wp,-MD,arch/rlx/bsp/.prom.o.d  -nostdinc -isystem /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/toolchain/rsdk-1.3.6-4181-EB-2.6.30-0.9.30/bin/../lib/gcc/mips-linux/3.4.6-1.3.6/include -Iinclude  -I/home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include -include include/linux/autoconf.h -D__KERNEL__ -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -Werror-implicit-function-declaration -fno-delete-null-pointer-checks -Os -ffunction-sections -mno-check-zero-division -mabi=32 -G 0 -mno-abicalls -fno-pic -pipe -msoft-float -ffreestanding -EB -UMIPSEB -U_MIPSEB -U__MIPSEB -U__MIPSEB__ -UMIPSEL -U_MIPSEL -U__MIPSEL -U__MIPSEL__ -DMIPSEB -D_MIPSEB -D__MIPSEB -D__MIPSEB__ -Iinclude/asm-rlx -Iarch/rlx/bsp/ -I/home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/mach-generic -D"VMLINUX_LOAD_ADDRESS=0x80000000" -fomit-frame-pointer -Wdeclaration-after-statement -I/home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/drivers/net/rtl819x/AsicDriver -I/home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/drivers/net/rtl819x/common -DCONFIG_RTL_819X_SWCORE   -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(prom)"  -D"KBUILD_MODNAME=KBUILD_STR(prom)"  -c -o arch/rlx/bsp/prom.o arch/rlx/bsp/prom.c

deps_arch/rlx/bsp/prom.o := \
  arch/rlx/bsp/prom.c \
    $(wildcard include/config/rtl/819x.h) \
    $(wildcard include/config/early/printk.h) \
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
  include/linux/kernel.h \
    $(wildcard include/config/lbd.h) \
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
  include/linux/stddef.h \
  include/linux/types.h \
    $(wildcard include/config/uid16.h) \
    $(wildcard include/config/phys/addr/t/64bit.h) \
    $(wildcard include/config/64bit.h) \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/types.h \
  include/asm-generic/int-ll64.h \
  include/asm-generic/bitsperlong.h \
  include/linux/posix_types.h \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/posix_types.h \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/sgidefs.h \
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
    $(wildcard include/config/rtl/8196e.h) \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/barrier.h \
    $(wildcard include/config/cpu/has/sync.h) \
    $(wildcard include/config/cpu/has/wb.h) \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/bug.h \
    $(wildcard include/config/bug.h) \
  include/asm-generic/bug.h \
    $(wildcard include/config/generic/bug.h) \
    $(wildcard include/config/generic/bug/relative/pointers.h) \
    $(wildcard include/config/debug/bugverbose.h) \
    $(wildcard include/config/smp.h) \
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
  include/linux/string.h \
    $(wildcard include/config/binary/printf.h) \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/string.h \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/bootinfo.h \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/setup.h \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/addrspace.h \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/mach-generic/spaces.h \
    $(wildcard include/config/32bit.h) \
    $(wildcard include/config/dma/noncoherent.h) \
  include/linux/const.h \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/page.h \
  include/linux/pfn.h \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/io.h \
  include/asm-generic/iomap.h \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/pgtable-bits.h \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/processor.h \
    $(wildcard include/config/cpu/has/sleep.h) \
    $(wildcard include/config/arch/suspend/possible.h) \
    $(wildcard include/config/rtl8197b/pana.h) \
    $(wildcard include/config/rtl/8196c.h) \
    $(wildcard include/config/rtl/819xd.h) \
  include/linux/cpumask.h \
    $(wildcard include/config/disable/obsolete/cpumask/functions.h) \
    $(wildcard include/config/hotplug/cpu.h) \
    $(wildcard include/config/cpumask/offstack.h) \
    $(wildcard include/config/debug/per/cpu/maps.h) \
  include/linux/threads.h \
    $(wildcard include/config/nr/cpus.h) \
    $(wildcard include/config/base/small.h) \
  include/linux/bitmap.h \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/cachectl.h \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/rlxregs.h \
    $(wildcard include/config/cpu/rlx4281.h) \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/system.h \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/cmpxchg.h \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/mach-generic/ioremap.h \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/mach-generic/mangle-port.h \
    $(wildcard include/config/swap/io/space.h) \
  include/asm-generic/memory_model.h \
    $(wildcard include/config/flatmem.h) \
    $(wildcard include/config/discontigmem.h) \
    $(wildcard include/config/sparsemem/vmemmap.h) \
    $(wildcard include/config/sparsemem.h) \
  include/asm-generic/getorder.h \
  /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/rlxbsp.h \
  arch/rlx/bsp/bspcpu.h \
  arch/rlx/bsp/bspchip.h \
    $(wildcard include/config/fpga/platform.h) \
    $(wildcard include/config/rtl/92d/support.h) \
    $(wildcard include/config/rtl/usb/ip/host/speedup.h) \

arch/rlx/bsp/prom.o: $(deps_arch/rlx/bsp/prom.o)

$(deps_arch/rlx/bsp/prom.o):
