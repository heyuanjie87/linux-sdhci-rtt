#ifndef _SDHCI_SG_H
#define _SDHCI_SG_H

struct page
{
    unsigned long page_link;
};

#define SG_END		0x02
#define SG_REA      0x04

struct scatterlist 
{
	struct page     pg;
	unsigned int	offset:24;
	unsigned int	flags:8;
	unsigned int	length;
	unsigned long	dma_address;
	void *old_addr;
};

struct sg_mapping_iter 
{
    unsigned			length;
	unsigned consumed;
	void			*addr;
};

#define SG_MITER_ATOMIC		(1 << 0)	 /* use kmap_atomic */
#define SG_MITER_TO_SG		(1 << 1)	/* flush back to phys on unmap */
#define SG_MITER_FROM_SG	(1 << 2)	/* nop */

struct scatterlist *__sdhci_sg_next(struct scatterlist *sg);
#define sg_next(s) __sdhci_sg_next(s)

#define for_each_sg(sglist, sg, nr, __i)	\
	for (__i = 0, sg = (sglist); __i < (nr); __i++, sg = sg_next(sg))

#define sg_dma_address(sg)	(((struct scatterlist*)(sg))->dma_address)
#define sg_dma_len(sg)		(((struct scatterlist*)(sg))->length)

#define sg_is_last(sg)		((sg)->flags & SG_END)

void __sdhci_sg_init_one(struct scatterlist *sg, const void *buf, unsigned int buflen);
#define sg_init_one(s, b, l) __sdhci_sg_init_one(s, b, l)
struct page *__sdhci_sg_page(struct scatterlist *sg);
#define sg_page(s) __sdhci_sg_page(s)
void __sdhci_sg_set_buf(struct scatterlist *sg, const void *buf,
			      unsigned int buflen);
#define sg_set_buf(s, b, l) __sdhci_sg_set_buf(s, b, l)
void __sdhci_sg_init_table(struct scatterlist *sgl, unsigned int nents);
#define sg_init_table(s, n) __sdhci_sg_init_table(s, n)
static inline void *sg_virt(struct scatterlist *sg)
{
    return (void*)(sg->pg.page_link + sg->offset);
}

size_t __sdhci_sg_copy_from_buffer(struct scatterlist *sgl, unsigned int nents,
			   const void *buf, size_t buflen);
#define sg_copy_from_buffer(s, n, b, l) __sdhci_sg_copy_from_buffer(s, n, b, l)
size_t __sdhci_sg_copy_to_buffer(struct scatterlist *sgl, unsigned int nents,
			 void *buf, size_t buflen);
#define sg_copy_to_buffer(s, n, b, l) __sdhci_sg_copy_to_buffer(s, n, b, l)
void __sdhci_sg_miter_start(struct sg_mapping_iter *miter, struct scatterlist *sgl,
		    unsigned int nents, unsigned int flags);
#define sg_miter_start(m, s, n, f) __sdhci_sg_miter_start(m, s, n, f)
void __sdhci_sg_miter_stop(struct sg_mapping_iter *miter);
#define sg_miter_stop(m) __sdhci_sg_miter_stop(m)
bool __sdhci_sg_miter_next(struct sg_mapping_iter *miter);
#define sg_miter_next(m) __sdhci_sg_miter_next(m)

#endif
