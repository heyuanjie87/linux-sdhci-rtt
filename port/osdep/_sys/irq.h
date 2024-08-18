#ifndef _SDHCI_IRQ_H
#define _SDHCI_IRQ_H

#include "types.h"

enum irqreturn 
{
	IRQ_NONE		= (0 << 0),
	IRQ_HANDLED		= (1 << 0),
	IRQ_WAKE_THREAD		= (1 << 1),
};
typedef enum irqreturn irqreturn_t;

typedef irqreturn_t (*irq_handler_t)(int, void *);

#define IRQF_SHARED		0x00000080

#define local_irq_save(flags)     {flags = (unsigned long)rt_hw_interrupt_disable();}
#define local_irq_restore(flags)  rt_hw_interrupt_enable((rt_base_t)flags)

const void *__sdhci_free_irq(unsigned int irq, void *d);
#define free_irq(i, d) __sdhci_free_irq(i, d)

int __sdhci_request_threaded_irq(unsigned int irq, irq_handler_t handler,
                         irq_handler_t thread_fn,
                         unsigned long irqflags, const char *name, void *dev_id);
#define request_threaded_irq(i, h, t, f, n, d) __sdhci_request_threaded_irq(i, h, t, f, n, d)

#endif
