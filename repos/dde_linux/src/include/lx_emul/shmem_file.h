/*
 * \brief  Shared-memory file utility
 * \author Christian Helmuth
 * \author Josef Söntgen
 * \date   2023-12-04
 *
 * This utility implements limited shared-memory file semantics as required by
 * Linux graphics drivers (e.g., intel_fb and lima_gpu_drv)
 */

/*
 * Copyright (C) 2023 Genode Labs GmbH
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 */

#include <lx_emul/shared_dma_buffer.h>
#include <linux/shmem_fs.h>

struct shmem_file_buffer
{
	struct genode_shared_dataspace *dataspace;
	void *addr;
	struct page *pages;
};


struct file *shmem_file_setup(char const *name, loff_t size,
                              unsigned long flags)
{
	struct file *f;
	struct inode *inode;
	struct address_space *mapping;
	struct shmem_file_buffer *private_data;
	loff_t const nrpages = DIV_ROUND_UP(size, PAGE_SIZE);

	if (!size)
		return (struct file*)ERR_PTR(-EINVAL);

	f = kzalloc(sizeof (struct file), 0);
	if (!f) {
		return (struct file*)ERR_PTR(-ENOMEM);
	}

	inode = kzalloc(sizeof (struct inode), 0);
	if (!inode) {
		goto err_inode;
	}

	mapping = kzalloc(sizeof (struct address_space), 0);
	if (!mapping) {
		goto err_mapping;
	}

	private_data = kzalloc(sizeof (struct shmem_file_buffer), 0);
	if (!private_data) {
		goto err_private_data;
	}

	private_data->dataspace = lx_emul_shared_dma_buffer_allocate(nrpages * PAGE_SIZE);
	if (!private_data->dataspace)
		goto err_private_data_addr;

	private_data->addr = lx_emul_shared_dma_buffer_virt_addr(private_data->dataspace);
	private_data->pages = lx_emul_virt_to_page(private_data->addr);

	mapping->private_data = private_data;
	mapping->nrpages = nrpages;

	inode->i_mapping = mapping;

	atomic_long_set(&f->f_count, 1);
	f->f_inode    = inode;
	f->f_mapping  = mapping;
	f->f_flags    = flags;
	f->f_mode     = OPEN_FMODE(flags);
	f->f_mode    |= FMODE_OPENED;

	return f;

err_private_data_addr:
	kfree(private_data);
err_private_data:
	kfree(mapping);
err_mapping:
	kfree(inode);
err_inode:
	kfree(f);
	return (struct file*)ERR_PTR(-ENOMEM);
}


struct page *shmem_read_mapping_page_gfp(struct address_space *mapping,
                                         pgoff_t index, gfp_t gfp)
{
	struct page *p;
	struct shmem_file_buffer *private_data;

	if (index > mapping->nrpages)
		return NULL;

	private_data = mapping->private_data;

	p = private_data->pages;
	return (p + index);
}


#include <linux/pagevec.h>

void __pagevec_release(struct pagevec * pvec)
{
	/* XXX check if we have to call release_pages */
	pagevec_reinit(pvec);
}


#include <linux/file.h>

static void _free_file(struct file *file)
{
	struct inode *inode;
	struct address_space *mapping;
	struct shmem_file_buffer *private_data;

	mapping      = file->f_mapping;
	inode        = file->f_inode;

	if (mapping) {
		private_data = mapping->private_data;

		lx_emul_shared_dma_buffer_free(private_data->dataspace);

		kfree(private_data);
		kfree(mapping);
	}

	kfree(inode);
	kfree(file->f_path.dentry);
	kfree(file);
}


void fput(struct file *file)
{
	if (!file)
		return;

	if (atomic_long_sub_and_test(1, &file->f_count)) {
		_free_file(file);
	}
}
