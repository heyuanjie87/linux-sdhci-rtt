#include "osdep/port.h"

static void work_func(struct rt_work *work, void *work_data)
{
    struct work_struct *w = (struct work_struct *) work_data;

    w->func(w);
}

void __sdhci_init_work(struct work_struct *w, work_func_t f)
{
    w->func = f;
    rt_work_init(&w->_wk, work_func, w);
}


struct workqueue_struct *__sdhci_alloc_workqueue(const char *fmt,
					 unsigned int flags,
					 int max_active, ...)
{
    struct workqueue_struct *wq;

    wq = rt_workqueue_create(fmt, RT_SYSTEM_WORKQUEUE_STACKSIZE, 20);

    return wq;
}

bool __sdhci_queue_work(struct workqueue_struct *wq,
			      struct work_struct *work)
{
    int ret;

    ret = rt_workqueue_submit_work(wq, &work->_wk, 0);

    return (ret == 0);
}

void __sdhci_destroy_workqueue(struct workqueue_struct *wq)
{
    rt_workqueue_destroy(wq);
}
