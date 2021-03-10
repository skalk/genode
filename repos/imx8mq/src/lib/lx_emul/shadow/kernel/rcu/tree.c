#include <linux/types.h>

extern void __rcu_read_lock(void);
void __rcu_read_lock(void) { }


extern void __rcu_read_unlock(void);
void __rcu_read_unlock(void) { }

int rcu_needs_cpu(u64 basemono, u64 *nextevt)
{
	return 0;
}
