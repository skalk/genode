
#include <linux/stop_machine.h>

int stop_machine(cpu_stop_fn_t fn,void * data,const struct cpumask * cpus)
{
	return (*fn)(data);
}
