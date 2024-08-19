# 简介
linux sdhci驱动适配到RT-Thread

# 适配说明
适配SDHCI平台相关部分需对linux中相应代码进行少量修改。
把平台代码中原来包含的<linux/xx.h>替换为<linux-sdhci/sdhci.h>，
初始化代码中只需包含<linux-sdhci/compat.h>。
要适配你自己的平台可以参考`example`和`paltform`目录。

## 必须提供的参数
```
    static struct platform_device pdev0 = {0};

    pdev0.irq = 36;                        //中断号
    pdev0.base = 0x4310000;                //设备基地址
    pdev0.dev.of_node = `构造的设备树`;     //传给sdhci的配置参数
    pdev0.dev.init_name = "sd";            //设备名称，不设置则为"mmc"
```

## 必须提供的中断注册函数
```
int __sdhci_irq_hw_register(unsigned int irq, irq_handler_t handler, void *id)
{
    /* 不支持下面接口的平台，你需要自己处理 */
    rt_hw_interrupt_install(irq, (rt_isr_handler_t)handler, id, "mmc");
    rt_hw_interrupt_umask(irq);

    return 0;
}
```

## 设备树构造
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
    dn_pp_set_and_add_bool(dn, p ++, "no-1-8-v"); //对于不支持1.8v的板子需加此参数

    return dn;
}
```

# 对原版sdhci代码的修改说明

## 1. sdhci_request_done()
在执行`dma_unmap_sg()`前执行了`spin_unlock_irqrestore`然后`*_save()`。

* 原因:
对于存在cache的cpu在进行dma传输前需要更新cache，
由于上层传下来的数据地址以及数据长度可能是非cacheline对齐的，
这可能给系统带来致命错误。为了解决这个问题驱动层对非对齐`读`(`写`不需要)
重新分配了对齐的缓冲区，当传输完成后拷贝数据再释放这段内存。
所以在`dma_unmap_sg()`中可能释放内存。
原版代码对整个过程进行`spinlock_irq_save()`这导致内存释放出错。


# 已知问题

* 未测试过SDMA
* 未测试PIO(不用DMA)
* 未支持nommu cpu
