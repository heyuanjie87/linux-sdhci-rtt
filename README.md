# 简介
linux sdhci驱动适配到RT-Thread

# 适配说明
适配SDHCI平台相关部分只需对linux中相应代码进行少量修改。
把原来包含的<linux/xx.h>替换为<linux-sdhci/sdhci.h>。
在驱动初始化代码中你只需要包含<linux-sdhci/compat.h>
·example·目录下放置了驱动初始化示例。

必须提供的参数：
```
	static struct platform_device pdev0 = {0};

	pdev0.irq = 36;
    pdev0.base = 0x4310000;
    pdev0.dev.of_node = `构造的设备树`;
    pdev0.dev.init_name = "sd";

```

必须提供的中断注册函数:
```
int __sdhci_irq_hw_register(unsigned int irq, irq_handler_t handler, void *id)
{
    /* 不支持下面接口的平台，你需要自己处理 */
    rt_hw_interrupt_install(irq, (rt_isr_handler_t)handler, id, "mmc");
    rt_hw_interrupt_umask(irq);

    return 0;
}
```

由于当前不支持读取实际的设备树你可以按如下方法构造设备树
```
static struct device_node* sd_of(void)
{
    static struct device_node _dn = {0};
    static struct property _pp[12] = {0};

    struct device_node *dn = &_dn;
    struct property *p = _pp;

	dn_pp_set_and_add_string(dn, p ++, "compatible", "cvitek,cv181x-sd");
    dn_pp_set_and_add_u32(dn, p ++, "bus-width", 4);
    dn_pp_set_and_add_u32(dn, p ++, "max-frequency", 200000000);
    dn_pp_set_and_add_bool(dn, p ++, "cap-sd-highspeed");
    dn_pp_set_and_add_bool(dn, p ++, "no-1-8-v");

    return dn;
}
```

# 已知问题

* 未测试过SDMA
* 未测试PIO(不用DMA)
* 未支持nommu cpu
