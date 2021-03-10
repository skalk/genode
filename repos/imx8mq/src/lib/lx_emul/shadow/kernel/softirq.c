#include <linux/interrupt.h>

#define CREATE_TRACE_POINTS
#include <trace/events/irq.h>

#include <lx_emul/debug.h>

irq_cpustat_t irq_stat;

int __init __weak arch_probe_nr_irqs(void)
{
	return 0;
}

int __init __weak arch_early_irq_init(void)
{
	return 0;
}

unsigned int __weak arch_dynirq_lower_bound(unsigned int from)
{
	return from;
}

static struct softirq_action actions[NR_SOFTIRQS];

void open_softirq(int nr, void (* action)(struct softirq_action *))
{
	if (nr >= NR_SOFTIRQS) {
		printk("Error: %s nr=%d exceeds softirq limit\n", __func__, nr);
		return;
	}

	actions[nr].action = action;
}


void raise_softirq(unsigned int nr)
{
	if (nr >= NR_SOFTIRQS || !actions[nr].action) { return; }

	actions[nr].action(&actions[nr]);
}
