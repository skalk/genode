/*
 * \brief  Dummy definitions of Linux Kernel functions - handled manually
 * \author Stefan Kalkowski
 * \date   2021-03-31
 */

/*
 * Copyright (C) 2021 Genode Labs GmbH
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 */

#include <lx_emul.h>


unsigned long __must_check __arch_clear_user(void __user *to, unsigned long n);
unsigned long __must_check __arch_clear_user(void __user *to, unsigned long n)
{
	lx_emul_trace_and_stop(__func__);
}


unsigned long __must_check __arch_copy_from_user(void *to, const void __user *from, unsigned long n);
unsigned long __must_check __arch_copy_from_user(void *to, const void __user *from, unsigned long n)
{
	lx_emul_trace_and_stop(__func__);
}


unsigned long __must_check __arch_copy_to_user(void __user *to, const void *from, unsigned long n);
unsigned long __must_check __arch_copy_to_user(void __user *to, const void *from, unsigned long n)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/cpuhotplug.h>

int __cpuhp_setup_state(enum cpuhp_state state,const char * name,bool invoke,int (* startup)(unsigned int cpu),int (* teardown)(unsigned int cpu),bool multi_instance)
{
	lx_emul_trace(__func__);
	return 0;
}


#include <asm-generic/percpu.h>

unsigned long __per_cpu_offset[NR_CPUS] = {};


#include <linux/pm_runtime.h>

void __pm_runtime_disable(struct device * dev,bool check_resume)
{
	lx_emul_trace(__func__);
}


#include <linux/pm_runtime.h>

void __pm_runtime_use_autosuspend(struct device * dev,bool use)
{
	lx_emul_trace(__func__);
}


#include <linux/pm_runtime.h>

int __pm_runtime_set_status(struct device * dev,unsigned int status)
{
	lx_emul_trace(__func__);
	return 0;
}


#include <linux/serial_core.h>

const struct earlycon_id *__earlycon_table[] = { NULL };


#include <linux/serial_core.h>

const struct earlycon_id *__earlycon_table_end[] = { NULL };


#include <asm-generic/sections.h>

char __start_once[] = { 0 };
char __end_once[] = { 0 };


#include <linux/i2c.h>

int __i2c_first_dynamic_bus_num;
EXPORT_SYMBOL_GPL(__i2c_first_dynamic_bus_num);

DECLARE_RWSEM(__i2c_board_lock);
EXPORT_SYMBOL_GPL(__i2c_board_lock);

LIST_HEAD(__i2c_board_list);
EXPORT_SYMBOL_GPL(__i2c_board_list);


#include <linux/pm_runtime.h>

int __pm_runtime_suspend(struct device * dev,int rpmflags)
{
	lx_emul_trace(__func__);
	return 0;
}


#include <linux/fs.h>

int __register_chrdev(unsigned int major, unsigned int baseminor,unsigned int count,const char * name,const struct file_operations * fops)
{
	lx_emul_trace(__func__);
	return 0;
}


#include <linux/of_reserved_mem.h>
#include <linux/mod_devicetable.h>

const struct of_device_id __reservedmem_of_table[] = {};


#include <asm-generic/sections.h>

char __start_rodata[] = {};
char __end_rodata[]   = {};


#include <linux/percpu-defs.h>

notrace void __this_cpu_preempt_check(const char * op)
{
	lx_emul_trace(__func__);
}


#include <linux/random.h>

void add_interrupt_randomness(int irq,int irq_flags)
{
	lx_emul_trace(__func__);
}


#include <linux/random.h>

int add_random_ready_callback(struct random_ready_callback * rdy)
{
	lx_emul_trace(__func__);
	return 0;
}


#include <linux/fs.h>

struct inode * alloc_anon_inode(struct super_block * s)
{
	return kmalloc(sizeof(struct inode), GFP_KERNEL);
}


#include <linux/dma-mapping.h>

void arch_setup_dma_ops(struct device * dev,u64 dma_base,u64 size,const struct iommu_ops * iommu,bool coherent)
{
	lx_emul_trace(__func__);
}


#include <linux/dma-mapping.h>

void arch_teardown_dma_ops(struct device * dev)
{
	lx_emul_trace(__func__);
}


#include <linux/init.h>

char __initdata boot_command_line[] = { 0 };


#include <linux/rcupdate.h>

void call_rcu(struct rcu_head * head,rcu_callback_t func)
{
	lx_emul_trace(__func__);
}


#include <linux/clk.h>

int clk_notifier_register(struct clk * clk,struct notifier_block * nb)
{
	lx_emul_trace(__func__);
	return 0;
}


#include <linux/printk.h>

int console_printk[] = { 0 };


#include <linux/debugfs.h>

struct dentry * debugfs_create_dir(const char * name,struct dentry * parent)
{
	return NULL;
}


extern struct dentry * debugfs_create_file(const char * name,umode_t mode,struct dentry * parent,void * data,const struct file_operations * fops);
struct dentry * debugfs_create_file(const char * name,umode_t mode,struct dentry * parent,void * data,const struct file_operations * fops)
{
	return NULL;
}


extern struct dentry * debugfs_create_file_unsafe(const char * name,umode_t mode,struct dentry * parent,void * data,const struct file_operations * fops);
struct dentry * debugfs_create_file_unsafe(const char * name,umode_t mode,struct dentry * parent,void * data,const struct file_operations * fops)
{
	return NULL;
}


#include <linux/debugfs.h>

bool debugfs_initialized(void)
{
	lx_emul_trace(__func__);
	return true;
}


#include <linux/pm_wakeirq.h>

void dev_pm_clear_wake_irq(struct device * dev)
{
	lx_emul_trace(__func__);
}


#include <linux/pm_wakeup.h>

int device_init_wakeup(struct device * dev,bool enable)
{
	lx_emul_trace(__func__);
	return 0;
}


#include <linux/gpio/consumer.h>
#include <uapi/linux/synclink.h>

struct gpio_desc * __must_check devm_gpiod_get(struct device * dev,const char * con_id,enum gpiod_flags flags)
{
	static struct gpio_desc gpiod;
	return &gpiod;
}


#include <linux/gpio/consumer.h>

struct gpio_desc * __must_check devm_gpiod_get_optional(struct device * dev,const char * con_id,enum gpiod_flags flags)
{
	lx_emul_trace(__func__);
	return NULL;
}


#include <linux/mux/consumer.h>
#include <linux/mux/driver.h>

struct mux_control * devm_mux_control_get(struct device * dev,const char * mux_name)
{
	static struct mux_control ret;
	lx_emul_trace(__func__);
	return &ret;
}


#include <linux/regulator/consumer.h>

int devm_regulator_bulk_get(struct device * dev,int num_consumers,struct regulator_bulk_data * consumers)
{
	lx_emul_trace(__func__);
	return 0;
}


#include <linux/regulator/consumer.h>

struct regulator * devm_regulator_get(struct device * dev,const char * id)
{
	lx_emul_trace(__func__);
	return NULL;
}


#include <linux/atomic.h>
#include <linux/dma-fence.h>

u64 dma_fence_context_alloc(unsigned num)
{
	static atomic64_t dma_fence_context_counter = ATOMIC64_INIT(1);
	WARN_ON(!num);
	return atomic64_add_return(num, &dma_fence_context_counter) - num;
}


#include <linux/dma-mapping.h>

int dma_supported(struct device *dev, u64 mask)
{
	return 1;
}


extern int dpm_sysfs_add(struct device * dev);
int dpm_sysfs_add(struct device * dev)
{
	return 0;
}


#include <linux/fb.h>

int fb_get_options(const char * name,char ** option)
{
	*option = "";
	return 0;
}


#include <linux/logic_pio.h>

struct logic_pio_hwaddr * find_io_range_by_fwnode(struct fwnode_handle * fwnode)
{
	lx_emul_trace(__func__);
	return NULL;
}


#include <linux/random.h>

void get_random_bytes(void * buf,int nbytes)
{
	lx_emul_trace(__func__);
}


#include <linux/random.h>

int __must_check get_random_bytes_arch(void * buf,int nbytes)
{
	lx_emul_trace(__func__);
	return 0;
}


#include <linux/tracepoint-defs.h>

const struct trace_print_flags gfpflag_names[]  = { {0,NULL}};


#include <linux/sched/isolation.h>

const struct cpumask * housekeeping_cpumask(enum hk_flags flags)
{
	static struct cpumask mask;
	lx_emul_trace(__func__);
	return &mask;
}


#include <linux/utsname.h>
#include <linux/user_namespace.h>

struct user_namespace init_user_ns;


#include <linux/fs.h>

void iput(struct inode * inode)
{
	kfree(inode);
}


extern void irq_add_debugfs_entry(unsigned int irq,struct irq_desc * desc);
void irq_add_debugfs_entry(unsigned int irq,struct irq_desc * desc)
{
	lx_emul_trace(__func__);
}


extern void irq_pm_install_action(struct irq_desc * desc,struct irqaction * action);
void irq_pm_install_action(struct irq_desc * desc,struct irqaction * action)
{
	lx_emul_trace(__func__);
}


extern void irq_pm_remove_action(struct irq_desc * desc,struct irqaction * action);
void irq_pm_remove_action(struct irq_desc * desc,struct irqaction * action)
{
	lx_emul_trace(__func__);
}


#include <linux/kernfs.h>

struct kernfs_node * kernfs_find_and_get_ns(struct kernfs_node * parent,const char * name,const void * ns)
{
	return NULL;
}


#include <linux/kernfs.h>

void kernfs_get(struct kernfs_node * kn) { }


#include <linux/kernfs.h>

void kernfs_put(struct kernfs_node * kn) { }


#include <linux/kobject.h>

int kobject_uevent(struct kobject * kobj,enum kobject_action action)
{
	lx_emul_trace(__func__);
	return 0;
}


#include <linux/kernel_stat.h>
#include <linux/stat.h>

struct kernel_stat kstat;


#include <linux/mux/consumer.h>

int mux_control_try_select(struct mux_control * mux,unsigned int state)
{
	lx_emul_trace(__func__);
	return 0;
}


#include <linux/topology.h>

int numa_node = 0;


#include <linux/clk/clk-conf.h>

int of_clk_set_defaults(struct device_node * node,bool clk_supplier)
{
	lx_emul_trace(__func__);
	return 0;
}


#include <linux/of_iommu.h>

const struct iommu_ops * of_iommu_configure(struct device * dev,struct device_node * master_np)
{
	static struct iommu_ops ops;
	lx_emul_trace(__func__);
	return &ops;
}


#include <linux/kernel.h>

int oops_in_progress = 0;		/* If set, an oops, panic(), BUG() or die() is in progress */


#include <linux/tracepoint-defs.h>

const struct trace_print_flags pageflag_names[] = { {0,NULL}};


#include <linux/kernel.h>

bool parse_option_str(const char *str, const char *option)
{
	lx_emul_trace(__func__);
	return false;
}


#include <linux/pci.h>

unsigned long pci_address_to_pio(phys_addr_t addr)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/pinctrl/devinfo.h>

int pinctrl_bind_pins(struct device * dev)
{
	lx_emul_trace(__func__);
	return 0;
}


#include <linux/pinctrl/devinfo.h>

int pinctrl_init_done(struct device * dev)
{
	lx_emul_trace(__func__);
	return 0;
}


#include <linux/pm_runtime.h>

void pm_runtime_enable(struct device * dev)
{
	lx_emul_trace(__func__);
}


#include <linux/pm_runtime.h>

void pm_runtime_no_callbacks(struct device * dev)
{
	lx_emul_trace(__func__);
}


#include <linux/pm_runtime.h>

void pm_runtime_set_autosuspend_delay(struct device * dev,int delay)
{
	lx_emul_trace(__func__);
}


#include <linux/proc_fs.h>

struct proc_dir_entry { int dummy; };

struct proc_dir_entry * proc_create_seq_private(const char * name,umode_t mode,struct proc_dir_entry * parent,const struct seq_operations * ops,unsigned int state_size,void * data)
{
	static struct proc_dir_entry ret;
	lx_emul_trace(__func__);
	return &ret;
}


#include <linux/proc_fs.h>

struct proc_dir_entry * proc_mkdir(const char * name,struct proc_dir_entry * parent)
{
	lx_emul_trace(__func__);
	return NULL;
}


#include <linux/proc_fs.h>

struct proc_dir_entry * proc_symlink(const char * name,struct proc_dir_entry * parent,const char * dest)
{
	lx_emul_trace(__func__);
	return NULL;
}


extern void register_handler_proc(unsigned int irq,struct irqaction * action);
void register_handler_proc(unsigned int irq,struct irqaction * action)
{
	lx_emul_trace(__func__);
}


extern void register_irq_proc(unsigned int irq,struct irq_desc * desc);
void register_irq_proc(unsigned int irq,struct irq_desc * desc)
{
	lx_emul_trace(__func__);
}


#include <linux/syscore_ops.h>

void register_syscore_ops(struct syscore_ops * ops)
{
	lx_emul_trace(__func__);
}


#include <linux/regulator/consumer.h>

struct regulator * regulator_get_optional(struct device * dev,const char * id)
{
	lx_emul_trace(__func__);
	return NULL;
}


#include <linux/fs.h>
#include <linux/mount.h>

int simple_pin_fs(struct file_system_type * type, struct vfsmount ** mount, int * count)
{
	*mount = kmalloc(sizeof(struct vfsmount), GFP_KERNEL);
	return 0;
}


#include <linux/fs.h>

void simple_release_fs(struct vfsmount ** mount,int * count)
{
	kfree(*mount);
}


#include <linux/sys_soc.h>

const struct soc_device_attribute * soc_device_match(const struct soc_device_attribute * matches)
{
	if (!matches) { return NULL; }

	for (; matches->machine && matches->family &&
	       matches->revision && matches->soc_id; matches++) {
		if (strcmp(matches->soc_id, "i.MX8MQ") == 0) { return matches; }
	}

	return NULL;
}


#include <linux/property.h>

int software_node_notify(struct device * dev,unsigned long action)
{
	lx_emul_trace(__func__);
	return 0;
}


#include <linux/srcutree.h>

void synchronize_srcu(struct srcu_struct * ssp)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/sysfs.h>

int sysfs_create_bin_file(struct kobject * kobj,const struct bin_attribute * attr)
{
	return 0;
}


#include <linux/sysfs.h>

int sysfs_create_dir_ns(struct kobject * kobj,const void * ns)
{
	//printk("Create SYSFS dir ns %s %p\n", kobject_name(kobj), ns);
	return 0;
}


#include <linux/sysfs.h>

int sysfs_create_file_ns(struct kobject * kobj,const struct attribute * attr,const void * ns)
{
	//printk("Create SYSFS file ns %s\n", kobject_name(kobj));
	return 0;
}


#include <linux/sysfs.h>

int sysfs_create_groups(struct kobject * kobj,const struct attribute_group ** groups)
{
	return 0;
}


#include <linux/sysfs.h>

int sysfs_create_link(struct kobject * kobj,struct kobject * target,const char * name)
{
	//printk("Create SYSFS link %s\n", name);
	return 0;
}


#include <linux/sysfs.h>

void sysfs_remove_link(struct kobject * kobj,const char * name) { }


extern void unregister_handler_proc(unsigned int irq,struct irqaction * action);
void unregister_handler_proc(unsigned int irq,struct irqaction * action)
{
	lx_emul_trace(__func__);
}


#include <linux/timekeeper_internal.h>

void update_vsyscall(struct timekeeper * tk)
{
	lx_emul_trace(__func__);
}


#include <linux/tracepoint-defs.h>

const struct trace_print_flags vmaflag_names[]  = { {0,NULL}};


#include <linux/mm.h>

bool is_vmalloc_addr(const void * x)
{
	lx_emul_trace(__func__);
	return false;
}


#include <linux/kernel_stat.h>

void account_process_tick(struct task_struct * p,int user_tick)
{
	lx_emul_trace(__func__);
}


#include <linux/sched/loadavg.h>

void calc_global_load(unsigned long ticks)
{
	lx_emul_trace(__func__);
}


#include <linux/capability.h>

bool capable(int cap)
{
	lx_emul_trace(__func__);
	return true;
}


extern int devtmpfs_create_node(struct device * dev);
int devtmpfs_create_node(struct device * dev)
{
	lx_emul_trace(__func__);
	return 0;
}


#include <linux/dma-resv.h>

void dma_resv_init(struct dma_resv * obj)
{
	lx_emul_trace(__func__);
}


#include <linux/gpio/consumer.h>

void gpiod_set_value_cansleep(struct gpio_desc * desc,int value)
{
	lx_emul_trace(__func__);
}


#include <linux/kobject.h>

int kobject_uevent_env(struct kobject * kobj,enum kobject_action action,char * envp_ext[])
{
	lx_emul_trace(__func__);
	return 0;
}


#include <linux/profile.h>

void profile_tick(int type)
{
	lx_emul_trace(__func__);
}


#include <linux/rcutree.h>

void rcu_irq_enter_irqson(void)
{
	lx_emul_trace(__func__);
}


#include <linux/rcutree.h>

void rcu_irq_exit_irqson(void)
{
	lx_emul_trace(__func__);
}


#include <linux/rcupdate.h>

void rcu_sched_clock_irq(int user)
{
	lx_emul_trace(__func__);
}


#include <linux/sysrq.h>

int register_sysrq_key(int key,struct sysrq_key_op * op_p)
{
	lx_emul_trace(__func__);
	return 0;
}


#include <linux/regulator/consumer.h>

int regulator_enable(struct regulator * regulator)
{
	lx_emul_trace(__func__);
	return 0;
}


#include <linux/posix-timers.h>

void run_posix_cpu_timers(void)
{
	lx_emul_trace(__func__);
}


#include <linux/shmem_fs.h>

struct file * shmem_file_setup(const char * name,loff_t size,unsigned long flags)
{
	static struct file file;
	lx_emul_trace(__func__);
	return &file;
}


#include <linux/clk.h>

struct clk * of_clk_get(struct device_node * np,int index)
{
	lx_emul_trace(__func__);
	return NULL;
}


#include <linux/hwspinlock.h>

int of_hwspin_lock_get_id(struct device_node * np,int index)
{
	lx_emul_trace(__func__);
	return -ENOENT;
}


#include <linux/moduleparam.h>

char * parse_args(const char * doing,char * args,const struct kernel_param * params,unsigned num,s16 min_level,s16 max_level,void * arg,int (* unknown)(char * param,char * val,const char * doing,void * arg))
{
	lx_emul_trace(__func__);
	return NULL;
}


