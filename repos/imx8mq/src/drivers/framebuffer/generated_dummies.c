/*
 * \brief  Dummy definitions of Linux Kernel functions
 * \author Automatically generated file - do no edit
 * \date   2021-05-21
 */

#include <lx_emul.h>


#include <linux/ratelimit.h>

int ___ratelimit(struct ratelimit_state * rs,const char * func)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/thread_info.h>

void __check_object_size(const void * ptr,unsigned long n,bool to_user)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/clk-provider.h>

const char * __clk_get_name(const struct clk * clk)
{
	lx_emul_trace_and_stop(__func__);
}


#include <asm-generic/delay.h>

void __const_udelay(unsigned long xloops)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/kexec.h>

void __noclone __crash_kexec(struct pt_regs * regs)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/file.h>

unsigned long __fdget(unsigned int fd)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/hwspinlock.h>

int __hwspin_lock_timeout(struct hwspinlock * hwlock,unsigned int to,int mode,unsigned long * flags)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/hwspinlock.h>

void __hwspin_unlock(struct hwspinlock * hwlock,int mode,unsigned long * flags)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/bottom_half.h>

void __local_bh_enable_ip(unsigned long ip,unsigned int cnt)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/suspend.h>

void __pm_pr_dbg(bool defer,const char * fmt,...)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/sched/task.h>

void __put_task_struct(struct task_struct * tsk)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/freezer.h>

bool __refrigerator(bool check_kthr_stop)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/sched.h>

void __set_task_comm(struct task_struct * tsk,const char * buf,bool exec)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/sched.h>

pid_t __task_pid_nr_ns(struct task_struct * task,enum pid_type type,struct pid_namespace * ns)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/interrupt.h>

void __tasklet_schedule(struct tasklet_struct * t)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/fs.h>

void __unregister_chrdev(unsigned int major,unsigned int baseminor,unsigned int count,const char * name)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/wait_bit.h>

wait_queue_head_t * __var_waitqueue(void * p)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/vmalloc.h>

void * __vmalloc_node_flags_caller(unsigned long size,int node,gfp_t flags,void * caller)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/uio.h>

bool _copy_from_iter_full(void * addr,size_t bytes,struct iov_iter * i)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/uio.h>

size_t _copy_to_iter(const void * addr,size_t bytes,struct iov_iter * i)
{
	lx_emul_trace_and_stop(__func__);
}


extern unsigned int _parse_integer(const char * s,unsigned int base,unsigned long long * p);
unsigned int _parse_integer(const char * s,unsigned int base,unsigned long long * p)
{
	lx_emul_trace_and_stop(__func__);
}


extern const char * _parse_integer_fixup_radix(const char * s,unsigned int * base);
const char * _parse_integer_fixup_radix(const char * s,unsigned int * base)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/rwlock_api_smp.h>

void __lockfunc _raw_read_lock(rwlock_t * lock)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/rwlock_api_smp.h>

void __lockfunc _raw_read_unlock(rwlock_t * lock)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/kernel_stat.h>

void account_idle_ticks(unsigned long ticks)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/random.h>

void add_bootloader_randomness(const void * buf,unsigned int size)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/kobject.h>

int add_uevent_var(struct kobj_uevent_env * env,const char * format,...)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/amba/bus.h>

int amba_device_add(struct amba_device * dev,struct resource * parent)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/amba/bus.h>

struct amba_device * amba_device_alloc(const char * name,resource_size_t base,size_t size)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/amba/bus.h>

void amba_device_put(struct amba_device * dev)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/anon_inodes.h>

struct file * anon_inode_getfile(const char * name,const struct file_operations * fops,void * priv,int flags)
{
	lx_emul_trace_and_stop(__func__);
}


#include <asm/pgtable-prot.h>

bool arm64_use_ng_mappings;


#include <linux/async.h>

async_cookie_t async_schedule_node(async_func_t func,void * data,int node)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/async.h>

void async_synchronize_full(void)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/bsearch.h>

void * bsearch(const void * key,const void * base,size_t num,size_t size,cmp_func_t cmp)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/kernel.h>

void bust_spinlocks(int yes)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/sched/nohz.h>

void calc_load_nohz_stop(void)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/srcu.h>

void call_srcu(struct srcu_struct * ssp,struct rcu_head * rhp,rcu_callback_t func)
{
	lx_emul_trace_and_stop(__func__);
}


#include <drm/bridge/cdns-mhdp-common.h>

int cdns_hdmi_bind(struct platform_device * pdev,struct drm_encoder * encoder,struct cdns_mhdp_device * mhdp)
{
	lx_emul_trace_and_stop(__func__);
}


#include <drm/bridge/cdns-mhdp-common.h>

void cdns_hdmi_unbind(struct device * dev)
{
	lx_emul_trace_and_stop(__func__);
}


#include <drm/bridge/cdns-mhdp-common.h>

bool cdns_mhdp_check_alive(struct cdns_mhdp_device * mhdp)
{
	lx_emul_trace_and_stop(__func__);
}


#include <drm/bridge/cdns-mhdp-common.h>

u32 cdns_phy_reg_read(struct cdns_mhdp_device * mhdp,u32 addr)
{
	lx_emul_trace_and_stop(__func__);
}


#include <drm/bridge/cdns-mhdp-common.h>

int cdns_phy_reg_write(struct cdns_mhdp_device * mhdp,u32 addr,u32 val)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/clk.h>

int clk_set_parent(struct clk * clk,struct clk * parent)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/console.h>

void console_flush_on_panic(enum con_flush_mode mode)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/console.h>

void console_lock(void)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/console.h>

int console_trylock(void)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/console.h>

void console_unblank(void)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/console.h>

void console_unlock(void)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/debugfs.h>

ssize_t debugfs_attr_read(struct file * file,char __user * buf,size_t len,loff_t * ppos)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/debugfs.h>

ssize_t debugfs_attr_write(struct file * file,const char __user * buf,size_t len,loff_t * ppos)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/debugfs.h>

struct dentry * debugfs_create_bool(const char * name,umode_t mode,struct dentry * parent,bool * value)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/debugfs.h>

ssize_t debugfs_read_file_bool(struct file * file,char __user * user_buf,size_t count,loff_t * ppos)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/debugfs.h>

void debugfs_remove(struct dentry * dentry)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/debugfs.h>

ssize_t debugfs_write_file_bool(struct file * file,const char __user * user_buf,size_t count,loff_t * ppos)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/fs.h>

loff_t default_llseek(struct file * file,loff_t offset,int whence)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/fs.h>

struct file * dentry_open(const struct path * path,int flags,const struct cred * cred)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/pm_wakeirq.h>

int dev_pm_set_dedicated_wake_irq(struct device * dev,int irq)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/pm_wakeirq.h>

int dev_pm_set_wake_irq(struct device * dev,int irq)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/regulator/consumer.h>

struct regulator * devm_regulator_get_optional(struct device * dev,const char * id)
{
	lx_emul_trace_and_stop(__func__);
}


extern int devtmpfs_delete_node(struct device * dev);
int devtmpfs_delete_node(struct device * dev)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/dma-buf.h>

struct dma_buf_attachment * dma_buf_attach(struct dma_buf * dmabuf,struct device * dev)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/dma-buf.h>

void dma_buf_detach(struct dma_buf * dmabuf,struct dma_buf_attachment * attach)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/dma-buf.h>

struct dma_buf * dma_buf_export(const struct dma_buf_export_info * exp_info)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/dma-buf.h>

int dma_buf_fd(struct dma_buf * dmabuf,int flags)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/dma-buf.h>

struct dma_buf * dma_buf_get(int fd)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/dma-buf.h>

struct sg_table * dma_buf_map_attachment(struct dma_buf_attachment * attach,enum dma_data_direction direction)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/dma-buf.h>

void dma_buf_put(struct dma_buf * dmabuf)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/dma-buf.h>

void dma_buf_unmap_attachment(struct dma_buf_attachment * attach,struct sg_table * sg_table,enum dma_data_direction direction)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/dma-buf.h>

void dma_buf_vunmap(struct dma_buf * dmabuf,void * vaddr)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/dma-mapping.h>

int dma_direct_map_sg(struct device * dev,struct scatterlist * sgl,int nents,enum dma_data_direction dir,unsigned long attrs)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/dma-mapping.h>

void dma_direct_unmap_sg(struct device * dev,struct scatterlist * sgl,int nents,enum dma_data_direction dir,unsigned long attrs)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/dma-fence.h>

int dma_fence_add_callback(struct dma_fence * fence,struct dma_fence_cb * cb,dma_fence_func_t func)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/dma-fence-chain.h>

int dma_fence_chain_find_seqno(struct dma_fence ** pfence,uint64_t seqno)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/dma-fence-chain.h>

void dma_fence_chain_init(struct dma_fence_chain * chain,struct dma_fence * prev,struct dma_fence * fence,uint64_t seqno)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/dma-fence-chain.h>

const struct dma_fence_ops dma_fence_chain_ops;


#include <linux/dma-fence-chain.h>

struct dma_fence * dma_fence_chain_walk(struct dma_fence * fence)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/dma-fence.h>

signed long dma_fence_default_wait(struct dma_fence * fence,bool intr,signed long timeout)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/dma-fence.h>

struct dma_fence * dma_fence_get_stub(void)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/dma-fence.h>

void dma_fence_init(struct dma_fence * fence,const struct dma_fence_ops * ops,spinlock_t * lock,u64 context,u64 seqno)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/dma-fence.h>

void dma_fence_release(struct kref * kref)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/dma-fence.h>

bool dma_fence_remove_callback(struct dma_fence * fence,struct dma_fence_cb * cb)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/dma-fence.h>

int dma_fence_signal(struct dma_fence * fence)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/dma-fence.h>

signed long dma_fence_wait_timeout(struct dma_fence * fence,bool intr,signed long timeout)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/dma-mapping.h>

int dma_get_sgtable_attrs(struct device * dev,struct sg_table * sgt,void * cpu_addr,dma_addr_t dma_addr,size_t size,unsigned long attrs)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/dma-mapping.h>

int dma_mmap_attrs(struct device * dev,struct vm_area_struct * vma,void * cpu_addr,dma_addr_t dma_addr,size_t size,unsigned long attrs)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/dma-resv.h>

void dma_resv_fini(struct dma_resv * obj)
{
	lx_emul_trace_and_stop(__func__);
}


extern void dpm_sysfs_remove(struct device * dev);
void dpm_sysfs_remove(struct device * dev)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/irq.h>

struct irq_chip dummy_irq_chip;


#include <linux/printk.h>

asmlinkage __visible void dump_stack(void)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/reboot.h>

void emergency_restart(void)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/syscalls.h>

struct trace_event_functions enter_syscall_print_funcs;


#include <linux/errname.h>

const char * errname(int err)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/syscalls.h>

struct trace_event_class event_class_syscall_enter;


#include <linux/syscalls.h>

struct trace_event_class event_class_syscall_exit;


#include <linux/syscalls.h>

struct trace_event_functions exit_syscall_print_funcs;


#include <linux/fb.h>

void fb_deferred_io_cleanup(struct fb_info * info)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/fb.h>

void fb_deferred_io_init(struct fb_info * info)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/fb.h>

void fb_set_suspend(struct fb_info * info,int state)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/fb.h>

ssize_t fb_sys_read(struct fb_info * info,char __user * buf,size_t count,loff_t * ppos)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/fb.h>

ssize_t fb_sys_write(struct fb_info * info,const char __user * buf,size_t count,loff_t * ppos)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/file.h>

void fd_install(unsigned int fd,struct file * file)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/kobject.h>

struct kobject *firmware_kobj;


#include <linux/fs.h>

loff_t fixed_size_llseek(struct file * file,loff_t offset,int whence,loff_t size)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/file.h>

void fput(struct file * file)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/fb.h>

void framebuffer_release(struct fb_info * info)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/percpu.h>

void free_percpu(void __percpu * ptr)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/freezer.h>

bool freezing_slow_path(struct task_struct * p)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/uidgid.h>

gid_t from_kgid(struct user_namespace * targ,kgid_t kgid)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/uidgid.h>

uid_t from_kuid(struct user_namespace * targ,kuid_t kuid)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/uidgid.h>

uid_t from_kuid_munged(struct user_namespace * targ,kuid_t kuid)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/kernel.h>

void ftrace_dump(enum ftrace_dump_mode oops_dump_mode)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/property.h>

struct fwnode_handle * fwnode_create_software_node(const struct property_entry * properties,const struct fwnode_handle * parent)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/property.h>

void fwnode_remove_software_node(struct fwnode_handle * fwnode)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/bug.h>

void generic_bug_clear_once(void)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/file.h>

int get_unused_fd_flags(unsigned flags)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/gpio/consumer.h>

int gpiod_get_direction(struct gpio_desc * desc)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/gpio/consumer.h>

int gpiod_get_value_cansleep(const struct gpio_desc * desc)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/gpio/consumer.h>

void gpiod_set_value(struct gpio_desc * desc,int value)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/gpio/consumer.h>

int gpiod_to_irq(const struct gpio_desc * desc)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/uuid.h>

const u8 guid_index[16] = {};


#include <linux/capability.h>

bool has_capability_noaudit(struct task_struct * t,int cap)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/sched/isolation.h>

bool housekeeping_enabled(enum hk_flags flags)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/hwspinlock.h>

int hwspin_lock_free(struct hwspinlock * hwlock)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/hwspinlock.h>

struct hwspinlock * hwspin_lock_request_specific(unsigned int id)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/pseudo_fs.h>

struct pseudo_fs_context * init_pseudo(struct fs_context * fc,unsigned long magic)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/wait_bit.h>

void init_wait_var_entry(struct wait_bit_queue_entry * wbq_entry,void * var,int flags)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/init.h>

bool initcall_debug;


#include <linux/ioport.h>

int insert_resource(struct resource * parent,struct resource * new)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/sched.h>

void io_schedule_finish(int token)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/sched.h>

int io_schedule_prepare(void)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/sched.h>

long __sched io_schedule_timeout(long timeout)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/ioport.h>

struct resource iomem_resource;


#include <linux/ioport.h>

struct resource ioport_resource;


#include <asm/hardirq.h>

	unsigned long irq_err_count;


extern bool irq_pm_check_wakeup(struct irq_desc * desc);
bool irq_pm_check_wakeup(struct irq_desc * desc)
{
	lx_emul_trace_and_stop(__func__);
}


extern bool irq_wait_for_poll(struct irq_desc * desc);
bool irq_wait_for_poll(struct irq_desc * desc)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/irq_work.h>

void irq_work_tick(void)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/property.h>

bool is_software_node(const struct fwnode_handle * fwnode)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/jiffies.h>

unsigned long volatile __cacheline_aligned_in_smp __jiffy_arch_data jiffies;


#include <linux/kgdb.h>

atomic_t			kgdb_active;


#include <linux/kgdb.h>

void kgdb_panic(const char * msg)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/fs.h>

void kill_anon_super(struct super_block * sb)
{
	lx_emul_trace_and_stop(__func__);
}


#include <asm/memory.h>

u64			kimage_voffset;


#include <linux/slab.h>

void kmem_cache_destroy(struct kmem_cache * s)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/slab.h>

void kmem_cache_free(struct kmem_cache * s,void * x)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/kmsg_dump.h>

void kmsg_dump(enum kmsg_dump_reason reason)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/kobject.h>

int kobject_synth_uevent(struct kobject * kobj,const char * buf,size_t count)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/string.h>

int kstrtobool(const char * s,bool * res)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/kernel.h>

int kstrtoint(const char * s,unsigned int base,int * res)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/kernel.h>

int kstrtoll(const char * s,unsigned int base,long long * res)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/kernel.h>

int kstrtouint(const char * s,unsigned int base,unsigned int * res)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/kernel.h>

int kstrtoull(const char * s,unsigned int base,unsigned long long * res)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/kthread.h>

void * kthread_data(struct task_struct * task)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/kthread.h>

bool kthread_should_stop(void)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/kthread.h>

int kthread_stop(struct task_struct * k)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/llist.h>

bool llist_add_batch(struct llist_node * new_first,struct llist_node * new_last,struct llist_head * head)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/logic_pio.h>

unsigned long logic_pio_trans_hwaddr(struct fwnode_handle * fwnode,resource_size_t addr,resource_size_t size)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/kallsyms.h>

int lookup_symbol_name(unsigned long addr,char * symname)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/delay.h>

unsigned long lpj_fine;


#include <linux/parser.h>

bool match_wildcard(const char * pattern,const char * str)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/memblock.h>

int __init_memblock memblock_add(phys_addr_t base,phys_addr_t size)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/memblock.h>

phys_addr_t __init_memblock memblock_find_in_range(phys_addr_t start,phys_addr_t end,phys_addr_t size,phys_addr_t align)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/memblock.h>

int __init_memblock memblock_free(phys_addr_t base,phys_addr_t size)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/memblock.h>

int __init_memblock memblock_mark_hotplug(phys_addr_t base,phys_addr_t size)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/memblock.h>

int __init_memblock memblock_remove(phys_addr_t base,phys_addr_t size)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/memblock.h>

int __init_memblock memblock_reserve(phys_addr_t base,phys_addr_t size)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/string.h>

ssize_t memory_read_from_buffer(void * to,size_t count,loff_t * ppos,const void * from,size_t available)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/mux/consumer.h>

int mux_control_deselect(struct mux_control * mux)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/irq.h>

struct irq_chip no_irq_chip;


#include <linux/fs.h>

loff_t no_llseek(struct file * file,loff_t offset,int whence)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/nodemask.h>

nodemask_t node_states[NR_NODE_STATES] = {};


#include <asm/numa.h>

cpumask_var_t node_to_cpumask_map[MAX_NUMNODES] = {};


#include <linux/fs.h>

loff_t noop_llseek(struct file * file,loff_t offset,int whence)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/irq.h>

void note_interrupt(struct irq_desc * desc,irqreturn_t action_ret)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/nodemask.h>

unsigned int nr_node_ids;


#include <linux/serial_core.h>

int __init of_setup_earlycon(const struct earlycon_id * match,unsigned long node,const char * options)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/smp.h>

void on_each_cpu(void (* func)(void * info),void * info,int wait)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/highuid.h>

int overflowuid;


#include <linux/reboot.h>

enum reboot_mode panic_reboot_mode;


#include <linux/genhd.h>

struct device_type part_type;


#include <linux/pci.h>

struct bus_type pci_bus_type;


#include <linux/initrd.h>

unsigned long phys_initrd_size;


#include <linux/initrd.h>

phys_addr_t phys_initrd_start;


#include <asm/memory.h>

s64			physvirt_offset;


#include <linux/pid.h>

struct task_struct * pid_task(struct pid * pid,enum pid_type type)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/pid.h>

pid_t pid_vnr(struct pid * pid)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/pm.h>

int pm_generic_freeze(struct device * dev)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/pm.h>

int pm_generic_poweroff(struct device * dev)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/pm.h>

int pm_generic_restore(struct device * dev)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/pm.h>

int pm_generic_resume(struct device * dev)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/pm.h>

int pm_generic_suspend(struct device * dev)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/pm.h>

int pm_generic_thaw(struct device * dev)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/printk.h>

int printk_deferred(const char * fmt,...)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/printk.h>

void printk_safe_flush_on_panic(void)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/proc_fs.h>

struct proc_dir_entry * proc_create(const char * name,umode_t mode,struct proc_dir_entry * parent,const struct proc_ops * proc_ops)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/pid.h>

void put_pid(struct pid * pid)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/file.h>

void put_unused_fd(unsigned int fd)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/interrupt.h>

void raise_softirq_irqoff(unsigned int nr)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/reboot.h>

enum reboot_mode reboot_mode;


#include <linux/refcount.h>

bool refcount_dec_and_mutex_lock(refcount_t * r,struct mutex * lock)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/refcount.h>

bool refcount_dec_not_one(refcount_t * r)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/refcount.h>

void refcount_warn_saturate(refcount_t * r,enum refcount_saturation_type t)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/regulator/consumer.h>

int regulator_bulk_disable(int num_consumers,struct regulator_bulk_data * consumers)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/regulator/consumer.h>

int regulator_bulk_enable(int num_consumers,struct regulator_bulk_data * consumers)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/regulator/consumer.h>

int regulator_disable(struct regulator * regulator)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/regulator/consumer.h>

void regulator_put(struct regulator * regulator)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/firmware.h>

void release_firmware(const struct firmware * fw)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/ioport.h>

int release_resource(struct resource * old)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/firmware.h>

int request_firmware(const struct firmware ** firmware_p,const char * name,struct device * device)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/reset.h>

int reset_control_assert(struct reset_control * rstc)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/sched/rt.h>

void rt_mutex_setprio(struct task_struct * p,struct task_struct * pi_task)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/init.h>

char *saved_command_line;


#include <linux/sched.h>

int sched_setscheduler_nocheck(struct task_struct * p,int policy,const struct sched_param * param)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/sched.h>

void __sched schedule_preempt_disabled(void)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/seq_file.h>

void seq_escape(struct seq_file * m,const char * s,const char * esc)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/seq_file.h>

loff_t seq_lseek(struct file * file,loff_t offset,int whence)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/seq_file.h>

int seq_open_private(struct file * filp,const struct seq_operations * ops,int psize)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/seq_file.h>

void seq_printf(struct seq_file * m,const char * f,...)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/seq_file.h>

void seq_puts(struct seq_file * m,const char * s)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/seq_file.h>

ssize_t seq_read(struct file * file,char __user * buf,size_t size,loff_t * ppos)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/seq_file.h>

int seq_release_private(struct inode * inode,struct file * file)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/seq_file.h>

void seq_vprintf(struct seq_file * m,const char * f,va_list args)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/seq_file.h>

int seq_write(struct seq_file * seq,const void * data,size_t len)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/scatterlist.h>

void sg_free_table(struct sg_table * table)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/scatterlist.h>

struct scatterlist * sg_next(struct scatterlist * sg)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/mm.h>

void show_mem(unsigned int filter,nodemask_t * nodemask)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/sched/debug.h>

void show_state_filter(unsigned long state_filter)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/fs.h>

int simple_attr_open(struct inode * inode,struct file * file,int (* get)(void *,u64 *),int (* set)(void *,u64),const char * fmt)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/fs.h>

int simple_attr_release(struct inode * inode,struct file * file)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/fs.h>

int simple_open(struct inode * inode,struct file * file)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/fs.h>

ssize_t simple_read_from_buffer(void __user * to,size_t count,loff_t * ppos,const void * from,size_t available)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/seq_file.h>

int single_open(struct file * file,int (* show)(struct seq_file *,void *),void * data)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/seq_file.h>

int single_release(struct inode * inode,struct file * file)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/siphash.h>

u64 siphash_1u64(const u64 first,const siphash_key_t * key)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/smp.h>

int smp_call_function_single(int cpu,smp_call_func_t func,void * info,int wait)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/smp.h>

void smp_send_stop(void)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/kallsyms.h>

int sprint_backtrace(char * buffer,unsigned long address)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/kallsyms.h>

int sprint_symbol(char * buffer,unsigned long address)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/kallsyms.h>

int sprint_symbol_no_offset(char * buffer,unsigned long address)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/jump_label.h>

bool static_key_initialized;


#include <linux/string_helpers.h>

int string_escape_mem(const char * src,size_t isz,char * dst,size_t osz,unsigned int flags,const char * only)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/string_helpers.h>

int string_unescape(char * src,char * dst,size_t size,unsigned int flags)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/printk.h>

int suppress_printk;


#include <linux/sync_file.h>

struct sync_file * sync_file_create(struct dma_fence * fence)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/sync_file.h>

struct dma_fence * sync_file_get_fence(int fd)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/fb.h>

void sys_copyarea(struct fb_info * p,const struct fb_copyarea * area)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/fb.h>

void sys_fillrect(struct fb_info * p,const struct fb_fillrect * rect)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/fb.h>

void sys_imageblit(struct fb_info * p,const struct fb_image * image)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/sysfs.h>

void sysfs_delete_link(struct kobject * kobj,struct kobject * targ,const char * name)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/sysfs.h>

void sysfs_notify(struct kobject * kobj,const char * dir,const char * attr)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/sysfs.h>

void sysfs_remove_dir(struct kobject * kobj)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/sysfs.h>

void sysfs_remove_file_ns(struct kobject * kobj,const struct attribute * attr,const void * ns)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/sysfs.h>

void sysfs_remove_groups(struct kobject * kobj,const struct attribute_group ** groups)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/freezer.h>

atomic_t system_freezing_cnt;	/* nr of freezing conds in effect */


#include <linux/task_work.h>

int task_work_add(struct task_struct * task,struct callback_head * work,bool notify)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/task_work.h>

struct callback_head * task_work_cancel(struct task_struct * task,task_work_func_t func)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/clockchips.h>

void tick_broadcast(const struct cpumask * mask)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/hrtimer.h>

void timerfd_clock_was_set(void)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/nmi.h>

notrace void touch_softlockup_watchdog(void)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/nmi.h>

notrace void touch_softlockup_watchdog_sched(void)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/trace_events.h>

enum print_line_t trace_handle_return(struct trace_seq * s)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/trace_events.h>

const char * trace_print_flags_seq(struct trace_seq * p,const char * delim,unsigned long flags,const struct trace_print_flags * flag_array)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/trace_events.h>

const char * trace_print_symbols_seq(struct trace_seq * p,unsigned long val,const struct trace_print_flags * symbol_array)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/trace_events.h>

int trace_raw_output_prep(struct trace_iterator * iter,struct trace_event * trace_event)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/trace_seq.h>

void trace_seq_printf(struct trace_seq * s,const char * fmt,...)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/vt_kern.h>

void unblank_screen(void)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/fb.h>

void unregister_framebuffer(struct fb_info * fb_info)
{
	lx_emul_trace_and_stop(__func__);
}


extern void unregister_irq_proc(unsigned int irq,struct irq_desc * desc);
void unregister_irq_proc(unsigned int irq,struct irq_desc * desc)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/sysrq.h>

int unregister_sysrq_key(int key,struct sysrq_key_op * op_p)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/uuid.h>

const u8 uuid_index[16] = {};


#include <asm/memory.h>

u64			vabits_actual;


#include <linux/vmalloc.h>

void vfree(const void * addr)
{
	lx_emul_trace_and_stop(__func__);
}


#include <video/videomode.h>

void videomode_from_timing(const struct display_timing * dt,struct videomode * vm)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/mm.h>

pgprot_t vm_get_page_prot(unsigned long vm_flags)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/vmalloc.h>

void * vzalloc(unsigned long size)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/sched/wake_q.h>

void wake_q_add(struct wake_q_head * head,struct task_struct * task)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/sched/wake_q.h>

void wake_q_add_safe(struct wake_q_head * head,struct task_struct * task)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/sched/nohz.h>

void wake_up_nohz_cpu(int cpu)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/sched/wake_q.h>

void wake_up_q(struct wake_q_head * head)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/wait_bit.h>

void wake_up_var(void * var)
{
	lx_emul_trace_and_stop(__func__);
}

