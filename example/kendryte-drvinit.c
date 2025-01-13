#include <rtthread.h>

#include <linux-sdhci/compat.h>

/*
        mmc_sd1: sdhci1@91581000 {
            compatible = "kendryte,k230-dw-mshc";
            reg = <0x0 0x91581000 0x0 0x1000>;
            interrupt-parent = <&intc>;
            interrupts = <144>;
            interrupt-names = "sdhci1irq";
            clocks = <&dummy_sd>,<&dummy_sd>;
            clock-names = "core", "bus";
            max-frequency = <50000000>;
            bus-width = <4>;
            sdhci,auto-cmd12;
            status = "disabled";
        };

*/

extern int dwcmshc_probe(struct platform_device *pdev);

static struct device_node *sd1_of(void)
{
    static struct device_node _dn = {0};
    static struct property _pp[12] = {0};

    struct device_node *dn = &_dn;
    struct property *p = _pp;

    dn_pp_set_and_add_string(dn, p++, "compatible", "kendryte,k230-dw-mshc");
    dn_pp_set_and_add_u32(dn, p++, "bus-width", 4);
    dn_pp_set_and_add_u32(dn, p++, "max-frequency", 50000000);
    dn_pp_set_and_add_bool(dn, p++, "no-1-8-v");

    return dn;
}

static int _sd1_init(void)
{
    static struct platform_device pdev0 = {0};

    int ret;

    pdev0.irq = 144;
    pdev0.base = 0x91581000;
    pdev0.dev.of_node = sd1_of();
    pdev0.dev.init_name = "sd1";

    ret = dwcmshc_probe(&pdev0);

    return ret;
}

int __sdhci_irq_hw_register(unsigned int irq, irq_handler_t handler, void *id)
{
    rt_hw_interrupt_install(irq, (rt_isr_handler_t)handler, id, "mmc");
    rt_hw_interrupt_umask(irq);

    return 0;
}
