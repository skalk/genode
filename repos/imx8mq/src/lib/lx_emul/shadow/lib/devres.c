#include <linux/device.h>
#include <lx_emul/io_mem.h>

void __iomem *devm_ioremap_resource(struct device *dev,
                                    const struct resource *res)
{
	return lx_emul_io_mem_map(res->start, resource_size(res));
}


void __iomem *devm_ioremap(struct device *dev, resource_size_t offset,
                           resource_size_t size)
{
	return lx_emul_io_mem_map(offset, size);
}
