#include <rtthread.h>

#include <linux-sdhci/compat.h>

/*
 emmc:cv-emmc@4300000 {
  compatible = "cvitek,cv181x-emmc";
 // reg = <0x0 0x4300000 0x0 0x1000>;
 // reg-names = "core_mem";
  bus-width = <4>;
  non-removable;
  no-sdio;
  no-sd;
 // min-frequency = <400000>;
  max-frequency = <200000000>;
 // 64_addressing;
 // reset_tx_rx_phy;
  pll_index = <0x5>;
  pll_reg = <0x3002064>;
 };
*/

/*
	sd:cv-sd@4310000 {
		compatible = "cvitek,cv181x-sd";
		reg = <0x0 0x4310000 0x0 0x1000>;
		reg-names = "core_mem";
		bus-width = <4>;
		cap-sd-highspeed;
		cap-mmc-highspeed;
		sd-uhs-sdr12;
		sd-uhs-sdr25;
		sd-uhs-sdr50;
		sd-uhs-sdr104;
		no-sdio;
		no-mmc;
		src-frequency = <375000000>;
		min-frequency = <400000>;
		max-frequency = <200000000>;
		64_addressing;
		reset_tx_rx_phy;
		reset-names = "sdhci";
		pll_index = <0x6>;
		pll_reg = <0x3002070>;
		cvi-cd-gpios = <&porta 13 GPIO_ACTIVE_LOW>;
	};

*/

extern int sdhci_cvi_probe(struct platform_device *pdev);

#ifdef SDHCI_PLATFORM_CVITEK_EMMC_ENABLE
static struct device_node* emmc_of(void)
{
    static struct device_node _dn = {0};
    static struct property _pp[12] = {0};

    struct device_node *dn = &_dn;
    struct property *p = _pp;

	dn_pp_set_and_add_string(dn, p ++, "compatible", "cvitek,cv181x-emmc");
    dn_pp_set_and_add_u32(dn, p ++, "bus-width", 4);
    dn_pp_set_and_add_u32(dn, p ++, "max-frequency", 200000000);
    dn_pp_set_and_add_bool(dn, p ++, "non-removable");

    return dn;
}

static int _emmc_init(void)
{
	static struct platform_device pdev0 = {0};

    int ret;

	pdev0.irq = 34;
    pdev0.base = 0x4300000;
    pdev0.dev.of_node = emmc_of();
    pdev0.dev.init_name = "emmc";

	ret = sdhci_cvi_probe(&pdev0);

	return ret;
}
#else
static int _emmc_init(void)
{
    return 0;
}
#endif

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

static int _sd_init(void)
{
	static struct platform_device pdev0 = {0};

    int ret;

	pdev0.irq = 36;
    pdev0.base = 0x4310000;
    pdev0.dev.of_node = sd_of();
    pdev0.dev.init_name = "sd";

	ret = sdhci_cvi_probe(&pdev0);

	return ret;
}

#ifndef SDHCI_PLATFORM_CVITEK_AUTOSTART_OFF
INIT_DEVICE_EXPORT(_emmc_init);
INIT_DEVICE_EXPORT(_sd_init);
#else
static void sdhcistart(int argc, char **argv)
{
    if (argc == 2 && !rt_strcmp("sd", argv[1]))
    {
        _sd_init();
    }
    else
    {
        _emmc_init();
    }
}
MSH_CMD_EXPORT(sdhcistart, sdhci driver start);
#endif

int __sdhci_irq_hw_register(unsigned int irq, irq_handler_t handler, void *id)
{
    rt_hw_interrupt_install(irq, (rt_isr_handler_t)handler, id, "mmc");
    rt_hw_interrupt_umask(irq);

    return 0;
}
