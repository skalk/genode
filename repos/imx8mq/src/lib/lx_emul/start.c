/*
 * \brief  Linux Kernel initialization
 * \author Stefan Kalkowski
 * \date   2021-03-16
 */

/*
 * Copyright (C) 2021 Genode Labs GmbH
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 */

#include <lx_emul/init.h>
#include <lx_emul/irq.h>
#include <lx_emul/time.h>

#include <asm/irq_regs.h>
#include <linux/cpu.h>
#include <linux/delay.h>
#include <linux/init_task.h>
#include <linux/interrupt.h>
#include <linux/irqchip.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/slab.h>
#include <linux/tick.h>
#include <linux/of.h>
#include <linux/of_clk.h>
#include <linux/of_fdt.h>

/* definitions in drivers/base/base.h */
extern int devices_init(void);
extern int buses_init(void);
extern int classes_init(void);
extern int platform_bus_init(void);

enum system_states system_state;

static int kernel_init(void * args)
{
	workqueue_init();

	/* the following calls are from driver_init() of drivers/base/init.c */
	devices_init();
	buses_init();
	classes_init();
	of_core_init();
	platform_bus_init();

	lx_emul_initcalls();

	system_state = SYSTEM_RUNNING;
	lx_emul_task_schedule(true);
	return 0;
}


static void timer_loop(void)
{
	//ktime_t delta_next;
	//u64     next_timer_ns;

	for (;;) {
		tick_nohz_idle_enter();
		//tick_nohz_idle_stop_tick();
		//next_timer_ns = tick_nohz_get_sleep_length(&delta_next);
		//rcu_idle_enter();
		//sched_idle_set_state(target_state);
		//time_start = ns_to_ktime(local_clock());
		//printk("sleep %lld microseconds\n", next_timer_ns / 1000);
		lx_emul_task_schedule(true);
		lx_emul_time_handle();
		//sched_clock_idle_wakeup_event();
		//tick_nohz_idle_restart_tick();
		tick_nohz_idle_exit();
	}
}


int lx_emul_init_task_function(void * dtb)
{
	/* Set dummy task registers used in IRQ and time handling */
	static struct pt_regs regs;
	set_irq_regs(&regs);

	/**
	 * Here we do the minimum normally done start_kernel() of init/main.c
	 */

	/* calls from setup_arch of arch/arm64/kernel/setup.c */
	early_init_dt_scan(dtb);
	unflatten_device_tree();

	jump_label_init();
	kmem_cache_init();
	radix_tree_init();
	workqueue_init_early();

	early_irq_init();
	irqchip_init();

	tick_init();
	init_timers();
	hrtimers_init();
	timekeeping_init();

	/* arch/arm64/kernel/time.c */
	//of_clk_init(NULL);
	lx_emul_time_init(); /* replaces timer_probe() */
	tick_setup_hrtimer_broadcast();
	lpj_fine = 1000000 / HZ;
	/* arch/arm64/kernel/time.c end */

	sched_clock_init();

	system_state = SYSTEM_SCHEDULING;

	kthread_create(kernel_init, NULL, "init");

	lx_emul_task_schedule(false);

	timer_loop();

	return 0;
}


struct task_struct init_task = {
	.state           = 0,
	.usage           = REFCOUNT_INIT(2),
	.flags           = PF_KTHREAD,
	.prio            = MAX_PRIO - 20,
	.static_prio     = MAX_PRIO - 20,
	.normal_prio     = MAX_PRIO - 20,
	.policy          = SCHED_NORMAL,
	.cpus_ptr        = &init_task.cpus_mask,
	.cpus_mask       = CPU_MASK_ALL,
	.nr_cpus_allowed = 1,
	.mm              = NULL,
	.active_mm       = NULL,
	.tasks           = LIST_HEAD_INIT(init_task.tasks),
	.real_parent     = &init_task,
	.parent          = &init_task,
	.children        = LIST_HEAD_INIT(init_task.children),
	.sibling         = LIST_HEAD_INIT(init_task.sibling),
	.group_leader    = &init_task,
	.comm            = INIT_TASK_COMM,
	.thread          = INIT_THREAD,
	.pending         = {
		.list   = LIST_HEAD_INIT(init_task.pending.list),
		.signal = {{0}}
	},
	.blocked         = {{0}},
};
void * lx_emul_init_task_struct = &init_task;
