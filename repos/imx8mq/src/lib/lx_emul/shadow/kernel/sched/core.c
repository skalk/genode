#include <asm/preempt.h>
#include <linux/preempt.h>
#include <linux/sched.h>
#include <linux/sched/debug.h>
#include <linux/sched/stat.h>
#include <linux/sched/nohz.h>

#include <lx_emul/debug.h>
#include <lx_emul/task.h>

#include <../kernel/sched/sched.h>

void set_user_nice(struct task_struct * p, long nice)
{
	p->static_prio = NICE_TO_PRIO(nice);
	p->prio        = p->static_prio;
	p->normal_prio = p->static_prio;
	lx_emul_task_priority(p, p->static_prio);
}


int set_cpus_allowed_ptr(struct task_struct * p,
                         const struct cpumask * new_mask)
{
	return 0;
}


static int
try_to_wake_up(struct task_struct *p, unsigned int state, int wake_flags)
{
	if (!(p->state & state)) { return 0; }

	if (p != lx_emul_task_get_current()) {
		lx_emul_task_unblock(p); }

	p->state = TASK_WAKING;
	return 1;
}


int wake_up_process(struct task_struct * p)
{
	return try_to_wake_up(p, TASK_NORMAL, 0);
}


int default_wake_function(wait_queue_entry_t *curr,
                          unsigned mode,
                          int wake_flags,
                          void *key)
{
	return try_to_wake_up(curr->private, mode, wake_flags);
}


asmlinkage __visible void __sched schedule(void)
{
	lx_emul_task_schedule(current->state != TASK_RUNNING);
}


void preempt_count_add(int val)
{
	current_thread_info()->preempt.count += val;
}


void preempt_count_sub(int val)
{
	current_thread_info()->preempt.count -= val;
}


asmlinkage __visible void __sched notrace preempt_schedule(void)
{
	if (likely(!preemptible())) return;
	schedule();
}


asmlinkage __visible void __sched notrace preempt_schedule_notrace(void)
{
	if (likely(!preemptible())) return;
	schedule();
}


unsigned long nr_iowait_cpu(int cpu)
{
	return 0;
}


void scheduler_tick(void)
{
	sched_clock_tick();
}


#ifdef CONFIG_SMP
#ifdef CONFIG_NO_HZ_COMMON

int get_nohz_timer_target(void)
{
	return 0;
}

#endif
#endif
