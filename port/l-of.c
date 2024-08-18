#include "osdep/port.h"

static struct property *__find_property(const struct device_node *np,
                                        const char *name,
                                        int *lenp)
{
    struct property *pp;

    if (!np)
        return NULL;

    for (pp = np->properties; pp; pp = pp->next)
    {
        if (strcmp(pp->name, name) == 0)
        {
            if (lenp)
                *lenp = pp->length;
            break;
        }
    }

    return pp;
}

bool __sdhci_of_property_read_bool(const struct device_node *np,
                                   const char *propname)
{
    struct property *prop = __find_property(np, propname, NULL);

    return prop ? true : false;
}

bool __sdhci_device_property_present(struct device *dev, const char *propname)
{
    return of_property_read_bool(dev->of_node, propname);
}

bool __sdhci_device_property_read_bool(struct device *dev, const char *propname)
{
    return of_property_read_bool(dev->of_node, propname);
}

int __sdhci_of_property_read_u32(const struct device_node *np,
                                 const char *propname, u32 *val)
{
    struct property *prop = __find_property(np, propname, NULL);

    if (!prop)
        return -EINVAL;

    *val = (u32)prop->value;

    return 0;
}

int __sdhci_device_property_read_u32(struct device *dev,
                                     const char *propname, u32 *val)
{
    return of_property_read_u32(dev->of_node, propname, val);
}

int __sdhci_of_property_read_u64(const struct device_node *np,
                                 const char *propname, u64 *val)
{
    struct property *prop = __find_property(np, propname, NULL);

    if (!prop)
        return -EINVAL;

    *val = (u64)prop->value;

    return 0;
}

int __sdhci_of_alias_get_id(struct device_node *np, const char *stem)
{
    pr_todo();
    return 0;
}

const struct of_device_id *__sdhci_of_match_device(const struct of_device_id *matches, const struct device *dev)
{
    const struct of_device_id *id = NULL;

    if (matches && dev)
    {
        struct property *prop = __find_property(dev->of_node, "compatible", NULL);

        if (prop)
        {
            while (matches->compatible)
            {
                if (rt_strcmp(matches->compatible, prop->str) == 0)
                {
                    id = matches;
                    break;
                }

                matches ++;
            }
        }
    }

    return id;
}

void dn_pp_set_and_add_u32(struct device_node *dn, struct property *p,
                           const char *name, u32 val)
{
    struct property *pre = dn->properties;

    dn->properties = p;
    p->next = pre;
    p->name = (char *)name;
    p->value = val;
    p->length = 4;
}

void dn_pp_set_and_add_bool(struct device_node *dn, struct property *p,
                            const char *name)
{
    struct property *pre = dn->properties;

    dn->properties = p;
    p->next = pre;
    p->name = (char *)name;
    p->value = 0;
    p->length = rt_strlen(name);
}

void dn_pp_set_and_add_string(struct device_node *dn, struct property *p,
                            const char *name, const char *val)
{
    struct property *pre = dn->properties;

    dn->properties = p;
    p->next = pre;
    p->name = (char *)name;
    p->str = val;
    p->length = rt_strlen(val);
}
