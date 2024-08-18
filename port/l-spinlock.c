#include "osdep/port.h"

void __sdhci_spin_lock_init(spinlock_t *l)
{
    rt_spin_lock_init(l);
}

void __sdhci_spin_lock(spinlock_t *l)
{
    rt_spin_lock(l);
}

void __sdhci_spin_unlock(spinlock_t *l)
{
    rt_spin_unlock(l);
}

void __sdhci_spin_irqsave(spinlock_t *l, unsigned long *f)
{
    *f = rt_spin_lock_irqsave(l);
}

void __sdhci_spin_irqrestore(spinlock_t *l, unsigned long f)
{
    rt_spin_unlock_irqrestore(l, f);
}
