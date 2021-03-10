#include <lx_emul/task.h>
#include <lx_kit/env.h>
#include <lx_kit/task.h>

extern "C" struct task_struct * lx_emul_task_get_current(void)
{
	return (struct task_struct*)
		Lx_kit::env().scheduler.current().lx_task();
}


extern "C"
void lx_emul_task_create(struct task_struct * task,
                         const char         * name,
                         int               (* threadfn)(void * data),
                         void               * data)
{
	new (Lx_kit::env().heap) Lx_kit::Task(threadfn,
	                                      data,
	                                      (void*)task, name,
	                                      Lx_kit::env().scheduler,
	                                      Lx_kit::Task::NORMAL);
}


extern "C" void lx_emul_task_unblock(struct task_struct * t)
{
	Lx_kit::env().scheduler.task((void*)t).unblock();
}


extern "C" void lx_emul_task_priority(struct task_struct * t,
                                      unsigned long prio)
{
	Lx_kit::env().scheduler.task((void*)t).priority(prio);
}


extern "C" void lx_emul_task_schedule(int block)
{
	Lx_kit::Task & task = Lx_kit::env().scheduler.current();
	if (block) { task.block(); }
	task.schedule();
}
