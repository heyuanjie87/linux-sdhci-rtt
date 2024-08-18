/*
 * drivers/mmc/host/sdhci-cvi.c - CVITEK SDHCI Platform driver
 *
 * Copyright (c) 2013-2014, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef __SDHCI_CV_H
#define __SDHCI_CV_H

#include <linux-sdhci/compat.h>

#define MAX_TUNING_CMD_RETRY_COUNT 50
#define TUNE_MAX_PHCODE	128
#define TAP_WINDOW_THLD 20
#define DISPPLL_MHZ 1188
#define FPLL_MHZ 1500

#define TOP_BASE	0x03000000
#define OFFSET_SD_PWRSW_CTRL	0x1F4

#define PINMUX_BASE 0x03001000
#define CLKGEN_BASE 0x03002000


#define CVI_CV181X_SDHCI_VENDOR_OFFSET		0x200
#define CVI_CV181X_SDHCI_VENDOR_MSHC_CTRL_R	(CVI_CV181X_SDHCI_VENDOR_OFFSET + 0x0)
#define CVI_CV181X_SDHCI_PHY_TX_RX_DLY		(CVI_CV181X_SDHCI_VENDOR_OFFSET + 0x40)
#define CVI_CV181X_SDHCI_PHY_DS_DLY			(CVI_CV181X_SDHCI_VENDOR_OFFSET + 0x44)
#define CVI_CV181X_SDHCI_PHY_DLY_STS		(CVI_CV181X_SDHCI_VENDOR_OFFSET + 0x48)
#define CVI_CV181X_SDHCI_PHY_CONFIG			(CVI_CV181X_SDHCI_VENDOR_OFFSET + 0x4C)

#define SDHCI_GPIO_CD_DEBOUNCE_TIME	10
#define SDHCI_GPIO_CD_DEBOUNCE_DELAY_TIME	200

struct sdhci_cvi_host {
	struct sdhci_host *host;
	struct platform_device *pdev;
	void __iomem *core_mem; /* mmio address */

	struct mmc_host *mmc;

	void __iomem *topbase;
	void __iomem *pinmuxbase;
	void __iomem *clkgenbase;

	u8 final_tap;
	u8 sdio0_voltage_1_8_v;
};

#endif
