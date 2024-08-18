#ifndef _SDHCI_TIME_H
#define _SDHCI_TIME_H

#include "types.h"

typedef int64_t ktime_t;

#define NSEC_PER_USEC 1000L
#define NSEC_PER_SEC 1000000000L
#define NSEC_PER_MSEC 1000000L

struct timer_list
{
	struct rt_timer tmr;
	void (*function)(struct timer_list *);
	u32 flags;
};

void __sdhci_init_timer(struct timer_list *timer,
						void (*func)(struct timer_list *), unsigned int flags,
						const char *name);
#define timer_setup(timer, callback, flags) __sdhci_init_timer(timer, callback, flags, "tmr")
#define del_timer_sync(t) __sdhci_del_timer(t)
int __sdhci_del_timer(struct timer_list *timer);
int __sdhci_mod_timer(struct timer_list *timer, unsigned long expires);

#define del_timer(t) __sdhci_del_timer(t)
#define mod_timer(t, e) __sdhci_mod_timer(t, e)

unsigned int __sdhci_jiffies_to_msecs(const unsigned long j);
#define jiffies_to_msecs(j) __sdhci_jiffies_to_msecs(j)

unsigned long __sdhci_msecs_to_jiffies(const unsigned int m);
#define msecs_to_jiffies(m) __sdhci_msecs_to_jiffies(m)

extern unsigned __sdhci_hz(void);
#define HZ __sdhci_hz()

extern unsigned long __sdhci_jiffies(void);
#define jiffies __sdhci_jiffies()
unsigned long __sdhci_nsecs_to_jiffies(u64 n);
#define nsecs_to_jiffies(n) __sdhci_nsecs_to_jiffies(n)

#define ktime_add_ns(kt, nsval) ((kt) + (nsval))
bool ktime_after(const ktime_t cmp1, const ktime_t cmp2);
ktime_t __sdhci_ktime_get(void);
#define ktime_get() __sdhci_ktime_get()

ktime_t ktime_add_ms(const ktime_t kt, const u64 msec);
int ktime_compare(const ktime_t cmp1, const ktime_t cmp2);
ktime_t ktime_add_us(const ktime_t kt, const u64 usec);

void __sdhci_mdelay(unsigned long x);
#define mdelay(x) __sdhci_mdelay(x)
void __sdhci_msleep(unsigned int msecs);
#define msleep(m) __sdhci_msleep(m)

void __sdhci_usleep_range(unsigned long min, unsigned long max);
#define usleep_range(i, a) __sdhci_usleep_range(i, a)

extern void __sdhci_udelay(unsigned long u);

#define udelay(u) __sdhci_udelay(u)

#endif
