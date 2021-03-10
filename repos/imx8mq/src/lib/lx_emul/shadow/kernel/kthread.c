
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/slab.h>

#include <lx_emul/task.h>

struct task_struct * kthread_create_on_node(int (* threadfn)(void * data),
                                            void * data,
                                            int node,
                                            const char namefmt[],...)
{
	struct task_struct *task;
	va_list args;
	char name[TASK_COMM_LEN];

	va_start(args, namefmt);
	task = kmalloc(sizeof(struct task_struct), GFP_KERNEL);
	vsnprintf(name, sizeof(name), namefmt, args);
	//set_task_comm(task, name);
	lx_emul_task_create(task, name, threadfn, data);
	va_end(args);
	return task;
}


void kthread_bind_mask(struct task_struct * p,
                       const struct cpumask * mask) { }
