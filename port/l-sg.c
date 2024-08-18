#include "osdep/port.h"

struct scatterlist *__sdhci_sg_next(struct scatterlist *sg)
{
    if (sg_is_last(sg))
        return 0;

    sg ++;

    return sg;
}

size_t __sdhci_sg_copy_to_buffer(struct scatterlist *sgl, unsigned int nents,
			 void *buf, size_t buflen)
{
	pr_todo();
	return 0;
}

size_t __sdhci_sg_copy_from_buffer(struct scatterlist *sgl, unsigned int nents,
			   const void *buf, size_t buflen)
{
	pr_todo();
	return 0;
}

void __sdhci_sg_miter_stop(struct sg_mapping_iter *miter)
{
    pr_todo();
}

bool __sdhci_sg_miter_next(struct sg_mapping_iter *miter)
{
    pr_todo();
    return true;
}

void __sdhci_sg_miter_start(struct sg_mapping_iter *miter, struct scatterlist *sgl,
		    unsigned int nents, unsigned int flags)
{
     pr_todo();
}

void __sdhci_sg_init_one(struct scatterlist *sg, const void *buf, unsigned int buflen)
{
    __sdhci_sg_set_buf(sg, buf, buflen);
    sg->flags |= SG_END;
}

void __sdhci_sg_init_table(struct scatterlist *sgl, unsigned int nents)
{
    memset(sgl, 0, sizeof(*sgl) * nents);
    sgl[nents - 1].flags |= SG_END;
}

void __sdhci_sg_set_buf(struct scatterlist *sg, const void *buf,
			      unsigned int buflen)
{
    sg->pg.page_link = (unsigned long)buf & ~(0xfffUL);
    sg->offset = (unsigned long)buf & 0xfff;
    sg->length = buflen;
    sg->dma_address = 0;
}

struct page *__sdhci_sg_page(struct scatterlist *sg)
{
    return &sg->pg;
}
