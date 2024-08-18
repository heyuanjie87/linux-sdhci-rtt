#include "osdep/port.h"

#include <rtthread.h>

static void *_alloc(size_t n, size_t size, gfp_t flags)
{
    void *p;

    if (flags & __GFP_ZERO)
        p = rt_calloc(n, size);
    else
        p = rt_malloc(n * size);

    return p;
}

void *__sdhci_kvzalloc(size_t size, gfp_t flags)
{
    return _alloc(1, size, flags | __GFP_ZERO);
}

void *__sdhci_devm_kzalloc(struct device *dev, size_t size, gfp_t gfp)
{
    return _alloc(1, size, gfp | __GFP_ZERO);
}

void *__sdhci_devm_kmalloc(struct device *dev, size_t size, gfp_t gfp)
{
    return _alloc(1, size, gfp);
}
