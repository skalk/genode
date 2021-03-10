TARGET   = imx8mq_fb_drv
REQUIRES = arm_v8a
LIBS     = base blit
SRC_CC   = main.cc i2c.cc reset.cc
SRC_C    = dummies.c fb.c i2c_imx.c
SRC_C   += $(notdir $(wildcard $(PRG_DIR)/generated_dummies.c))

INC_DIR  = $(PRG_DIR)
CC_OPT_i2c_imx = -DKBUILD_MODFILE='"i2c_imx"' -DKBUILD_BASENAME='"i2c_imx"' -DKBUILD_MODNAME='"i2c_imx"'


#
# Lx_emul + Lx_kit definitions
#

SRC_CC  += lx_emul/alloc.cc
SRC_CC  += lx_emul/clock.cc
SRC_CC  += lx_emul/debug.cc
SRC_CC  += lx_emul/init.cc
SRC_CC  += lx_emul/io_mem.cc
SRC_CC  += lx_emul/irq.cc
SRC_CC  += lx_emul/log.cc
SRC_CC  += lx_emul/task.cc
SRC_CC  += lx_emul/time.cc

SRC_C   += lx_emul/clocksource.c
SRC_C   += lx_emul/irqchip.c
SRC_C   += lx_emul/start.c
SRC_C   += lx_emul/shadow/arch/arm64/kernel/smp.c
SRC_C   += lx_emul/shadow/arch/arm64/mm/ioremap.c
SRC_C   += lx_emul/shadow/drivers/base/power/common.c
SRC_C   += lx_emul/shadow/drivers/base/power/main.c
SRC_C   += lx_emul/shadow/drivers/base/power/runtime.c
SRC_C   += lx_emul/shadow/drivers/clk/clk.c
SRC_C   += lx_emul/shadow/drivers/clk/clkdev.c
SRC_C   += lx_emul/shadow/kernel/cpu.c
SRC_C   += lx_emul/shadow/kernel/dma/mapping.c
SRC_C   += lx_emul/shadow/kernel/irq/spurious.c
SRC_C   += lx_emul/shadow/kernel/ksysfs.c
SRC_C   += lx_emul/shadow/kernel/kthread.c
SRC_C   += lx_emul/shadow/kernel/locking/spinlock.c
SRC_C   += lx_emul/shadow/kernel/printk/printk.c
SRC_C   += lx_emul/shadow/kernel/rcu/srcutree.c
SRC_C   += lx_emul/shadow/kernel/rcu/tree.c
SRC_C   += lx_emul/shadow/kernel/sched/core.c
SRC_C   += lx_emul/shadow/kernel/smp.c
SRC_C   += lx_emul/shadow/kernel/softirq.c
SRC_C   += lx_emul/shadow/kernel/stop_machine.c
SRC_C   += lx_emul/shadow/lib/devres.c
SRC_C   += lx_emul/shadow/lib/smp_processor_id.c
SRC_C   += lx_emul/shadow/mm/memblock.c
SRC_C   += lx_emul/shadow/mm/percpu.c
SRC_C   += lx_emul/shadow/mm/slab_common.c
SRC_C   += lx_emul/shadow/mm/slub.c

SRC_CC  += lx_kit/console.cc
SRC_CC  += lx_kit/device.cc
SRC_CC  += lx_kit/env.cc
SRC_CC  += lx_kit/init.cc
SRC_CC  += lx_kit/memory.cc
SRC_CC  += lx_kit/scheduler.cc
SRC_CC  += lx_kit/task.cc
SRC_CC  += lx_kit/timeout.cc
SRC_S   += lx_kit/spec/arm_64/setjmp.S

INC_DIR += $(REP_DIR)/src/include
INC_DIR += $(REP_DIR)/src/include/spec/arm_64
INC_DIR += $(REP_DIR)/src/include/lx_emul/shadow

vpath % $(REP_DIR)/src/lib


#
# Linux kernel definitions
#

# FIXME: when port is available, change to CONTRIB_DIR := $(call select_from_ports,linux)
CONTRIB_DIR := /home/sk/src/genode/contrib/linux-7f785aec84b4be2960e4ef7f91a385ba68cad776

INC_DIR  += $(CONTRIB_DIR)/arch/arm64/include
INC_DIR  += $(CONTRIB_DIR)/arch/arm64/include/generated
INC_DIR  += $(CONTRIB_DIR)/include
INC_DIR  += $(CONTRIB_DIR)/arch/arm64/include/uapi
INC_DIR  += $(CONTRIB_DIR)/arch/arm64/include/generated/uapi
INC_DIR  += $(CONTRIB_DIR)/include/uapi
INC_DIR  += $(CONTRIB_DIR)/include/generated/uapi
INC_DIR  += $(CONTRIB_DIR)/scripts/dtc/libfdt

CC_C_OPT += -std=gnu89 -include $(CONTRIB_DIR)/include/linux/kconfig.h
CC_C_OPT += -include $(CONTRIB_DIR)/include/linux/compiler_types.h
CC_C_OPT += -D__KERNEL__ -DCONFIG_CC_HAS_K_CONSTRAINT=1
CC_C_OPT += -DKASAN_SHADOW_SCALE_SHIFT=3
CC_C_OPT += -Wall -Wundef -Werror=strict-prototypes -Wno-trigraphs
CC_C_OPT += -Werror=implicit-function-declaration -Werror=implicit-int
CC_C_OPT += -Wno-format-security -Wno-psabi
CC_C_OPT += -Wno-frame-address -Wno-format-truncation -Wno-format-overflow
CC_C_OPT += -Wframe-larger-than=2048 -Wno-unused-but-set-variable -Wimplicit-fallthrough
CC_C_OPT += -Wno-unused-const-variable -Wdeclaration-after-statement -Wvla
CC_C_OPT += -Wno-pointer-sign -Wno-stringop-truncation -Wno-array-bounds -Wno-stringop-overflow
CC_C_OPT += -Wno-restrict -Wno-maybe-uninitialized -Werror=date-time
CC_C_OPT += -Werror=incompatible-pointer-types -Werror=designated-init
CC_C_OPT += -Wno-packed-not-aligned  

INC_DIR  += $(CONTRIB_DIR)/drivers/base/regmap

# Linux objects needed by whole initialization + device tree paring + device construction
LX_OBJECTS  = $(wildcard $(CONTRIB_DIR)/drivers/base/bus.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/drivers/base/class.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/drivers/base/component.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/drivers/base/core.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/drivers/base/dd.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/drivers/base/devres.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/drivers/base/driver.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/drivers/base/platform.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/drivers/base/property.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/drivers/base/regmap/*.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/drivers/clk/clk-devres.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/drivers/dma/dmaengine.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/drivers/dma/of-dma.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/drivers/of/*.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/drivers/irqchip/irqchip.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/kernel/irq/chip.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/kernel/irq/devres.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/kernel/irq/handle.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/kernel/irq/irqdesc.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/kernel/irq/irqdomain.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/kernel/irq/manage.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/kernel/irq/resend.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/kernel/locking/mutex.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/kernel/locking/osq_lock.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/kernel/locking/rtmutex.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/kernel/locking/rwsem.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/kernel/notifier.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/kernel/panic.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/kernel/sched/clock.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/kernel/sched/completion.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/kernel/sched/swait.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/kernel/sched/wait.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/kernel/time/clockevents.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/kernel/time/clocksource.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/kernel/time/hrtimer.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/kernel/time/jiffies.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/kernel/time/ntp.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/kernel/time/sched_clock.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/kernel/time/tick-*.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/kernel/time/time*.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/kernel/workqueue.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/lib/bitmap.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/lib/cpumask.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/lib/crc32.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/lib/ctype.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/lib/dynamic_debug.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/lib/debug_locks.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/lib/fdt*.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/lib/find_bit.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/lib/hexdump.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/lib/hweight.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/lib/idr.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/lib/irq_regs.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/lib/kasprintf.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/lib/klist.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/lib/kobject.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/lib/list_debug.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/lib/list_sort.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/lib/radix-tree.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/lib/rbtree.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/lib/sort.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/lib/string.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/lib/timerqueue.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/lib/vsprintf.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/lib/xarray.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/mm/mempool.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/mm/util.o)

LX_ASM      = $(wildcard $(CONTRIB_DIR)/arch/arm64/lib/mem*.S)
LX_ASM     += $(wildcard $(CONTRIB_DIR)/arch/arm64/lib/str*.S)

# Display subsystem related stuff
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/drivers/gpu/drm/*.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/drivers/gpu/drm/bridge/*.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/drivers/gpu/drm/imx/*.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/drivers/gpu/drm/imx/dcss/*.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/drivers/gpu/drm/mxsfb/*.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/drivers/gpu/drm/panel/*.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/drivers/i2c/i2c-core-base.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/drivers/i2c/i2c-core-of.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/drivers/i2c/i2c-core-smbus.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/drivers/irqchip/irq-imx-irqsteer.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/drivers/phy/freescale/phy-fsl-imx8-mipi-dphy.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/drivers/phy/phy-core.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/drivers/phy/phy-core-mipi-dphy.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/drivers/video/backlight/backlight.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/drivers/video/fbdev/core/fbcmap.o)
LX_OBJECTS += $(wildcard $(CONTRIB_DIR)/drivers/video/of_display_timing.o)

# Turn Linux objects into source files
LX_REL_OBJ = $(LX_OBJECTS:$(CONTRIB_DIR)/%=%)
SRC_C     += $(LX_REL_OBJ:%.o=%.c))
SRC_S     += $(LX_ASM:$(CONTRIB_DIR)/%=%)

vpath %.c $(CONTRIB_DIR)
vpath %.S $(CONTRIB_DIR)

# Define per-compilation-unit CC_OPT defines needed by MODULE* macros in Linux
define CC_OPT_LX_RULES =
CC_OPT_$(1) = -DKBUILD_MODFILE='"$(1)"' -DKBUILD_BASENAME='"$(notdir $(1))"' -DKBUILD_MODNAME='"$(notdir $(1))"'
endef

$(foreach file,$(LX_REL_OBJ),$(eval $(call CC_OPT_LX_RULES,$(file:%.o=%))))
