#include "osdep/port.h"

#include <mm_page.h>

static void _dma_sync(void *ptr, unsigned size,
                      enum dma_data_direction dir)
{
    if (dir == DMA_FROM_DEVICE)
        rt_hw_cpu_dcache_ops(RT_HW_CACHE_INVALIDATE, ptr, size);
    else
        rt_hw_cpu_dcache_ops(RT_HW_CACHE_FLUSH, ptr, size);
}

static void _sg_sync(struct scatterlist *sg, unsigned nents, enum dma_data_direction dir)
{
    for (unsigned i = 0; i < nents; i ++)
    {
        _dma_sync(sg_virt(&sg[i]), sg[i].length, dir);
    }
}

int __sdhci_dma_map_sg(struct device *dev, struct scatterlist *sg,
		unsigned nents, enum dma_data_direction dir)
{
    void *va;

    for (unsigned i = 0; i < nents; i ++)
    {
        va = sg_virt(sg);
        sg->dma_address = virt_to_phys(va);
        _dma_sync(va, sg->length, dir);
        sg ++;
    }

    return nents;
}

void __sdhci_dma_sync_sg_for_cpu(struct device *dev,
		struct scatterlist *sg, int nelems, enum dma_data_direction dir)
{
    _sg_sync(sg, nelems, dir);
}

int __sdhci_dmaengine_terminate_sync(struct dma_chan *chan)
{
	pr_todo();
	return 0;
}

void *__sdhci_dma_alloc_coherent(struct device *dev, size_t size,
		dma_addr_t *dma_handle, gfp_t gfp)
{
    void *v;

    v = rt_pages_alloc(rt_page_bits(size));
    if (v)
    {
        *dma_handle = virt_to_phys(v);
        v = ioremap_wt(*dma_handle, size);
    }

    return v;
}

void __sdhci_dma_free_coherent(struct device *dev, size_t size,
		void *cpu_addr, dma_addr_t dma_handle)
{
    pr_todo();
}

int __sdhci_dma_mapping_error(struct device *dev, dma_addr_t dma_addr)
{
	return 0;
}

dma_addr_t __sdhci_dma_map_single(struct device *dev, void *ptr,
		size_t size, enum dma_data_direction dir)
{
	pr_todo();
	return 0;
}

void __sdhci_dma_sync_single_for_device(struct device *dev, dma_addr_t addr,
        size_t size, enum dma_data_direction dir)
{
    pr_todo();
}


void __sdhci_dma_sync_single_for_cpu(struct device *dev, dma_addr_t addr, size_t size,
        enum dma_data_direction dir)
{
    pr_todo();
}

int __sdhci_dma_set_mask_and_coherent(struct device *dev, u64 mask)
{
    return 0;
}

void __sdhci_dma_unmap_sg_attrs(struct device *dev,
		struct scatterlist *sg, int nents, enum dma_data_direction dir,
		unsigned long attrs)
{
    if (dir == DMA_FROM_DEVICE)
    {
        struct scatterlist *sgtmp;
        unsigned i;

        for (i = 0, sgtmp = sg; i < nents; i ++, sgtmp ++)
        {
            if (sgtmp->flags & SG_REA)
            {
                rt_memcpy(sgtmp->old_addr, sg_virt(sgtmp), sgtmp->length);
                rt_free_align(sg_virt(sgtmp));
            }
        }
    }
}
