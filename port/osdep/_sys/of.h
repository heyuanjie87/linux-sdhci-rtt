#ifndef _SDHCI_OF_H
#define _SDHCI_OF_H

#include "types.h"

struct device;

struct property
{
    char *name;
    int length;
    union
    {
        unsigned long value;
        const char *str;
    };

    struct property *next;
};

struct device_node
{
    const char *name;
    struct property *properties;
};

struct of_device_id
{
    const char *compatible;
    const void *data;
};

void dn_pp_set_and_add_u32(struct device_node *dn, struct property *p,
                           const char *name, u32 val);

void dn_pp_set_and_add_bool(struct device_node *dn, struct property *p,
                            const char *name);

void dn_pp_set_and_add_string(struct device_node *dn, struct property *p,
                            const char *name, const char *val);

bool __sdhci_of_property_read_bool(const struct device_node *np,
                                   const char *propname);
#define of_property_read_bool(d, p) __sdhci_of_property_read_bool(d, p)

int __sdhci_of_property_read_u32(const struct device_node *np,
                                 const char *propname, u32 *val);
#define of_property_read_u32(d, p, v) __sdhci_of_property_read_u32(d, p, v)

int __sdhci_of_property_read_u64(const struct device_node *np,
                                 const char *propname, u64 *val);
#define of_property_read_u64(d, p, v) __sdhci_of_property_read_u64(d, p, v)

bool __sdhci_device_property_read_bool(struct device *dev, const char *propname);
#define device_property_read_bool(d, p) __sdhci_device_property_read_bool(d, p)

int __sdhci_device_property_read_u32(struct device *dev,
                                     const char *propname, u32 *val);
#define device_property_read_u32(d, p, v) __sdhci_device_property_read_u32(d, p, v)

bool __sdhci_device_property_present(struct device *dev, const char *propname);
#define device_property_present(d, p) __sdhci_device_property_present(d, p)

int __sdhci_of_alias_get_id(struct device_node *np, const char *stem);
#define of_alias_get_id(d, s) __sdhci_of_alias_get_id(d, s)

const struct of_device_id *__sdhci_of_match_device(const struct of_device_id *matches, const struct device *dev);
#define of_match_device(a, b) __sdhci_of_match_device(a, b)

#endif
