#pragma once

#include <rtthread.h>
#include <ipc/workqueue.h>

struct work_struct;

typedef void (*work_func_t)(struct work_struct *work);

#define workqueue_struct rt_workqueue

struct work_struct
{
    struct rt_work _wk;
    work_func_t func;

    void *priv;
};

void __sdhci_init_work(struct work_struct *w, work_func_t f);

#define INIT_WORK(_work, _func) __sdhci_init_work(_work, _func)
struct workqueue_struct *__sdhci_alloc_workqueue(const char *fmt,
					 unsigned int flags,
					 int max_active, ...);
#define alloc_workqueue __sdhci_alloc_workqueue

bool __sdhci_queue_work(struct workqueue_struct *wq,
			      struct work_struct *work);
#define queue_work(w, k) __sdhci_queue_work(w, k)

void __sdhci_destroy_workqueue(struct workqueue_struct *wq);
#define destroy_workqueue(w) __sdhci_destroy_workqueue(w)
