// SPDX-License-Identifier: GPL-2.0-only
/*
 * sdhci-pltfm.c Support for SDHCI platform devices
 * Copyright (c) 2009 Intel Corporation
 *
 * Copyright (c) 2007, 2011 Freescale Semiconductor, Inc.
 * Copyright (c) 2009 MontaVista Software, Inc.
 *
 * Authors: Xiaobo Xie <X.Xie@freescale.com>
 *	    Anton Vorontsov <avorontsov@ru.mvista.com>
 */

/* Supports:
 * SDHCI platform devices
 *
 * Inspired by sdhci-pci.c, by Pierre Ossman
 */

#include "sdhci-pltfm.h"

static const struct sdhci_ops sdhci_pltfm_ops = {
	.set_clock = sdhci_set_clock,
	.set_bus_width = sdhci_set_bus_width,
	.reset = sdhci_reset,
	.set_uhs_signaling = sdhci_set_uhs_signaling,
};

static bool sdhci_wp_inverted(struct device *dev)
{
	if (device_property_present(dev, "sdhci,wp-inverted") ||
	    device_property_present(dev, "wp-inverted"))
		return true;

	/* Old device trees don't have the wp-inverted property. */
#ifdef CONFIG_PPC
	return machine_is(mpc837x_rdb) || machine_is(mpc837x_mds);
#else
	return false;
#endif /* CONFIG_PPC */
}

void sdhci_get_compatibility(struct platform_device *pdev) {}

void sdhci_get_property(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct sdhci_host *host = platform_get_drvdata(pdev);
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	u32 bus_width;

	if (device_property_present(dev, "sdhci,auto-cmd12"))
		host->quirks |= SDHCI_QUIRK_MULTIBLOCK_READ_ACMD12;

	if (device_property_present(dev, "sdhci,1-bit-only") ||
	    (device_property_read_u32(dev, "bus-width", &bus_width) == 0 &&
	    bus_width == 1))
		host->quirks |= SDHCI_QUIRK_FORCE_1_BIT_DATA;

	if (sdhci_wp_inverted(dev))
		host->quirks |= SDHCI_QUIRK_INVERTED_WRITE_PROTECT;

	if (device_property_present(dev, "broken-cd"))
		host->quirks |= SDHCI_QUIRK_BROKEN_CARD_DETECTION;

	if (device_property_present(dev, "no-1-8-v"))
		host->quirks2 |= SDHCI_QUIRK2_NO_1_8_V;

	sdhci_get_compatibility(pdev);

	device_property_read_u32(dev, "clock-frequency", &pltfm_host->clock);

	if (device_property_present(dev, "keep-power-in-suspend"))
		host->mmc->pm_caps |= MMC_PM_KEEP_POWER;

	if (device_property_read_bool(dev, "wakeup-source") ||
	    device_property_read_bool(dev, "enable-sdio-wakeup")) /* legacy */
		host->mmc->pm_caps |= MMC_PM_WAKE_SDIO_IRQ;
}
EXPORT_SYMBOL_GPL(sdhci_get_property);

struct sdhci_host *sdhci_pltfm_init(struct platform_device *pdev,
				    const struct sdhci_pltfm_data *pdata,
				    size_t priv_size)
{
	struct sdhci_host *host;
	void __iomem *ioaddr;
	int irq, ret;

	ioaddr = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(ioaddr)) {
		ret = PTR_ERR(ioaddr);
		goto err;
	}

	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		ret = irq;
		goto err;
	}

	host = sdhci_alloc_host(&pdev->dev,
		sizeof(struct sdhci_pltfm_host) + priv_size);

	if (IS_ERR(host)) {
		ret = PTR_ERR(host);
		goto err;
	}

	host->ioaddr = ioaddr;
	host->irq = irq;
	host->hw_name = dev_name(&pdev->dev);
	if (pdata && pdata->ops)
		host->ops = pdata->ops;
	else
		host->ops = &sdhci_pltfm_ops;
	if (pdata) {
		host->quirks = pdata->quirks;
		host->quirks2 = pdata->quirks2;
	}

	platform_set_drvdata(pdev, host);

	return host;
err:
	dev_err(&pdev->dev, "%s failed %d\n", __func__, ret);
	return ERR_PTR(ret);
}
EXPORT_SYMBOL_GPL(sdhci_pltfm_init);

void sdhci_pltfm_free(struct platform_device *pdev)
{
	struct sdhci_host *host = platform_get_drvdata(pdev);

	sdhci_free_host(host);
}
EXPORT_SYMBOL_GPL(sdhci_pltfm_free);
