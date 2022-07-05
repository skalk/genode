/*
 * \brief  Dummy definitions of Linux Kernel functions
 * \author Automatically generated file - do no edit
 * \date   2022-07-04
 */

#include <lx_emul.h>


#include <linux/cpumask.h>

atomic_t __num_online_cpus;


//#include <asm-generic/percpu.h>

unsigned long __per_cpu_offset[NR_CPUS] = { 0UL };


struct srcu_struct;
extern int __srcu_read_lock(struct srcu_struct * ssp);
int __srcu_read_lock(struct srcu_struct * ssp)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/spinlock.h>

void __lockfunc _raw_read_lock(rwlock_t * lock)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/cpu.h>

void cpu_hotplug_disable(void)
{
	lx_emul_trace(__func__);
}


#include <linux/cpu.h>

void cpu_hotplug_enable(void)
{
	lx_emul_trace(__func__);
}


#include <linux/cpumask.h>

unsigned int cpumask_next(int n,const struct cpumask * srcp)
{
	lx_emul_trace(__func__);
	return n + 1;
}


#include <linux/cpumask.h>

int cpumask_next_and(int n,const struct cpumask * src1p,const struct cpumask * src2p)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/sched.h>

void do_set_cpus_allowed(struct task_struct * p,const struct cpumask * new_mask)
{
	lx_emul_trace(__func__);
}


#include <linux/sched/isolation.h>

const struct cpumask * housekeeping_cpumask(enum hk_flags flags)
{
	static struct cpumask ret;
	lx_emul_trace(__func__);
	return &ret;
}


#include <linux/sched/isolation.h>

bool housekeeping_enabled(enum hk_flags flags)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/sched.h>

int idle_cpu(int cpu)
{
	lx_emul_trace(__func__);
	return 1;
}


#include <linux/sched/nohz.h>

void nohz_balance_enter_idle(int cpu)
{
	lx_emul_trace(__func__);
}


#include <linux/cpumask.h>

unsigned int nr_cpu_ids = 1;


#include <linux/prandom.h>

u32 prandom_u32(void)
{
	lx_emul_trace_and_stop(__func__);
}


extern void quiet_vmstat(void);
void quiet_vmstat(void)
{
	lx_emul_trace(__func__);
}


#include <linux/rcutree.h>

noinstr void rcu_irq_enter(void)
{
	lx_emul_trace(__func__);
}


#include <linux/rcutree.h>

void noinstr rcu_irq_exit(void)
{
	lx_emul_trace(__func__);
}


#include <linux/rcutree.h>

void rcu_softirq_qs(void)
{
	lx_emul_trace(__func__);
}


extern void synchronize_srcu(struct srcu_struct * ssp);
void synchronize_srcu(struct srcu_struct * ssp)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/sched/nohz.h>

void wake_up_nohz_cpu(int cpu)
{
	lx_emul_trace(__func__);
}


int cpu_number = 0;

DEFINE_PER_CPU(void *, hardirq_stack_ptr);
DEFINE_PER_CPU(bool, hardirq_stack_inuse);


#include <linux/interrupt.h>

DEFINE_PER_CPU_SHARED_ALIGNED(irq_cpustat_t, irq_stat);
EXPORT_PER_CPU_SYMBOL(irq_stat);

extern void rcu_read_unlock_strict(void);
void rcu_read_unlock_strict(void)
{
	lx_emul_trace(__func__);
}


DEFINE_PER_CPU_READ_MOSTLY(unsigned long, this_cpu_off) = 0;
EXPORT_PER_CPU_SYMBOL(this_cpu_off);
