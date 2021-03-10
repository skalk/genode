#include <asm/io.h>
#include <lx_emul/io_mem.h>

void __iomem * __ioremap(phys_addr_t phys_addr, size_t size, pgprot_t prot)
{
	return lx_emul_io_mem_map(phys_addr, size);
}


void iounmap(volatile void __iomem * io_addr) { }


