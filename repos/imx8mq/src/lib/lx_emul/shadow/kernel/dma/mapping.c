#include <lx_emul/alloc.h>
#include <lx_emul/debug.h>

#include <linux/dma-mapping.h>

void * dma_alloc_attrs(struct device * dev,
                       size_t          size,
                       dma_addr_t    * dma_handle,
                       gfp_t           flag,
                       unsigned long   attrs)
{
	if (dev && dev->dma_mem) {
		printk("We do not support device DMA memory yet!");
		lx_emul_trace_and_stop(__func__);
	}

	if (size < 4096) {
		printk("We do not support DMA memory smaller than page size yet!");
		lx_emul_trace_and_stop(__func__);
	}

	return lx_emul_alloc_dma(size, (void**)dma_handle);
}


void dma_free_attrs(struct device * dev,
                    size_t          size,
                    void          * cpu_addr,
                    dma_addr_t      dma_handle,
                    unsigned long   attrs)
{
	lx_emul_free(cpu_addr);
}


int dma_set_mask(struct device *dev, u64 mask)
{
	mask = (dma_addr_t)mask;

	if (!dev->dma_mask || !dma_supported(dev, mask))
		return -EIO;

	*dev->dma_mask = mask;
	return 0;
}


int dma_set_coherent_mask(struct device *dev, u64 mask)
{
	mask = (dma_addr_t)mask;

	if (!dma_supported(dev, mask))
		return -EIO;

	dev->coherent_dma_mask = mask;
	return 0;
}
