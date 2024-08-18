#ifndef _SDHCI_DMA_H
#define _SDHCI_DMA_H

#include "types.h"

struct device;
struct scatterlist;

struct dma_chan 
{
	const char *name;
};

enum dma_data_direction
{
	DMA_BIDIRECTIONAL = 0,
	DMA_TO_DEVICE = 1,
	DMA_FROM_DEVICE = 2,
	DMA_NONE = 3,
};

#define DMA_BIT_MASK(n)	(((n) == 64) ? ~0ULL : ((1ULL<<(n))-1))

int __sdhci_dma_set_mask_and_coherent(struct device *dev, u64 mask);
#define dma_set_mask_and_coherent(d, m) __sdhci_dma_set_mask_and_coherent(d, m) 
void __sdhci_dma_free_coherent(struct device *dev, size_t size,
		void *cpu_addr, dma_addr_t dma_handle);
#define dma_free_coherent(d, s, c, h) __sdhci_dma_free_coherent(d, s, c, h)
void *__sdhci_dma_alloc_coherent(struct device *dev, size_t size,
		dma_addr_t *dma_handle, gfp_t gfp);
#define dma_alloc_coherent(d, s, h, g) __sdhci_dma_alloc_coherent(d, s, h, g)
dma_addr_t __sdhci_dma_map_single(struct device *dev, void *ptr,
		size_t size, enum dma_data_direction dir);
#define dma_map_single(d, p, s, r) __sdhci_dma_map_single(d, p, s, r)
int __sdhci_dma_map_sg(struct device *dev, struct scatterlist *sg,
		unsigned nents, enum dma_data_direction dir);
void __sdhci_dma_unmap_sg_attrs(struct device *dev,
		struct scatterlist *sg, int nents, enum dma_data_direction dir,
		unsigned long attrs);

#define dma_map_sg(d, s, n, r) __sdhci_dma_map_sg(d, s, n, r)
#define dma_unmap_sg(d, s, n, r) __sdhci_dma_unmap_sg_attrs(d, s, n, r, 0)

void __sdhci_dma_sync_sg_for_cpu(struct device *dev,
		struct scatterlist *sg, int nelems, enum dma_data_direction dir);
#define dma_sync_sg_for_cpu(d, s, n, r) __sdhci_dma_sync_sg_for_cpu(d, s, n, r)
int __sdhci_dma_mapping_error(struct device *dev, dma_addr_t dma_addr);
#define dma_mapping_error(d, a) __sdhci_dma_mapping_error(d, a)
int __sdhci_dmaengine_terminate_sync(struct dma_chan *chan);
#define dmaengine_terminate_sync(c) __sdhci_dmaengine_terminate_sync(c)
void __sdhci_dma_sync_single_for_cpu(struct device *dev, dma_addr_t addr, size_t size,
		enum dma_data_direction dir);
#define dma_sync_single_for_cpu(d, a, s, r) __sdhci_dma_sync_single_for_cpu(d, a, s, r)
void __sdhci_dma_sync_single_for_device(struct device *dev, dma_addr_t addr,
		size_t size, enum dma_data_direction dir);
#define dma_sync_single_for_device(d, a, s, r) __sdhci_dma_sync_single_for_device(d, a, s, r)

#endif
