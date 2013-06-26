cmd_arch/rlx/bsp/vmlinux.lds := rsdk-linux-gcc -E -Wp,-MD,arch/rlx/bsp/.vmlinux.lds.d  -nostdinc -isystem /home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/toolchain/rsdk-1.5.5-5281-EB-2.6.30-0.9.30.3-110714/bin/../lib/gcc/mips-linux/4.4.5-1.5.5p4/include -Iinclude  -I/home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include -include include/linux/autoconf.h -D__KERNEL__   -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -Werror-implicit-function-declaration -fno-delete-null-pointer-checks -Os -ffunction-sections  -mno-check-zero-division -mabi=32 -G 0 -mno-abicalls -fno-pic -pipe -msoft-float -ffreestanding -EB -UMIPSEB -U_MIPSEB -U__MIPSEB -U__MIPSEB__ -UMIPSEL -U_MIPSEL -U__MIPSEL -U__MIPSEL__ -DMIPSEB -D_MIPSEB -D__MIPSEB -D__MIPSEB__ -Iinclude/asm-rlx -Iarch/rlx/bsp/ -I/home/keith_huang/11n/rlx/patch_area/rtl819x-SDK-v32_v321_v3211_322_3221/rtl819x/linux-2.6.30/arch/rlx/include/asm/mach-generic -D"VMLINUX_LOAD_ADDRESS=0x80000000" -D"LOADADDR=0x80000000" -D"JIFFIES=jiffies_64 + 4" -D"DATAOFFSET=0" -D"ZZ_DRAMSIZE_ZZ=8k" -D"ZZ_IRAMSIZE_ZZ=8k" -D"ZZ_L2RAMSIZE_ZZ=32k" -P -C -Urlx -D__ASSEMBLY__ -o arch/rlx/bsp/vmlinux.lds arch/rlx/bsp/vmlinux.lds.S

deps_arch/rlx/bsp/vmlinux.lds := \
  arch/rlx/bsp/vmlinux.lds.S \
    $(wildcard include/config/rtk/voip.h) \
    $(wildcard include/config/rtl8192se.h) \
    $(wildcard include/config/mapped/kernel.h) \
    $(wildcard include/config/blk/dev/initrd.h) \
  include/asm/asm-offsets.h \
  include/asm-generic/vmlinux.lds.h \
    $(wildcard include/config/hotplug.h) \
    $(wildcard include/config/hotplug/cpu.h) \
    $(wildcard include/config/memory/hotplug.h) \
    $(wildcard include/config/ftrace/mcount/record.h) \
    $(wildcard include/config/trace/branch/profiling.h) \
    $(wildcard include/config/profile/all/branches.h) \
    $(wildcard include/config/event/tracer.h) \
    $(wildcard include/config/tracing.h) \
    $(wildcard include/config/ftrace/syscalls.h) \
    $(wildcard include/config/function/graph/tracer.h) \
    $(wildcard include/config/generic/bug.h) \
    $(wildcard include/config/pm/trace.h) \
  include/linux/section-names.h \

arch/rlx/bsp/vmlinux.lds: $(deps_arch/rlx/bsp/vmlinux.lds)

$(deps_arch/rlx/bsp/vmlinux.lds):
