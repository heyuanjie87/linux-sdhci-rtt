#include <rtthread.h>

#include <string.h>

#include "osdep/port.h"

static int fls(unsigned int x)
{
	int r = 32;

	if (!x)
		return 0;
	if (!(x & 0xffff0000u)) {
		x <<= 16;
		r -= 16;
	}
	if (!(x & 0xff000000u)) {
		x <<= 8;
		r -= 8;
	}
	if (!(x & 0xf0000000u)) {
		x <<= 4;
		r -= 4;
	}
	if (!(x & 0xc0000000u)) {
		x <<= 2;
		r -= 2;
	}
	if (!(x & 0x80000000u)) {
		x <<= 1;
		r -= 1;
	}
	return r;
}

int __sdhci_ilog2(int v)
{
    return fls(v) - 1;
}

void *__sdhci_kmap_atomic(struct page *page)
{
	pr_todo();
    return (void*)page->page_link;
}

void __sdhci_kunmap_atomic(struct page *page)
{
}

unsigned int __sdhci_swiotlb_max_segment(void)
{
	return 0;
}

void __iomem *
__sdhci_devm_platform_ioremap_resource(struct platform_device *pdev,
			       unsigned int index)
{
    return ioremap(pdev->base, 0x1000);
}

const char *__sdhci_dev_name(const struct device *dev)
{
	if (dev && dev->init_name)
	    return dev->init_name;

	return "";
}

#include <ioremap.h>
void *__sdhci_ioremap(phys_addr_t offset, size_t size)
{
    return rt_ioremap((void*)offset, size);
}

void *__sdhci_ioremap_wt(phys_addr_t offset, size_t size)
{
    return rt_ioremap_wt((void*)offset, size);
}

void __iounmap(volatile void __iomem *addr)
{
    rt_iounmap(addr);
}

#include <mm_aspace.h>
unsigned long __sdhci_virt_to_phys(volatile void *address)
{
    return (unsigned long)rt_kmem_v2p((void*)address);
}
