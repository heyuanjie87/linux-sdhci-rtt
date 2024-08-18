/*
 * drivers/mmc/host/sdhci-cv.c - CVITEK SDHCI Platform driver
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

#include <linux-sdhci/sdhci.h>
#include "sdhci-cv181x.h"

#define BOUNDARY_OK(addr, len) \
	((addr | (SZ_128M - 1)) == ((addr + len - 1) | (SZ_128M - 1)))

static void sdhci_cvi_reset_helper(struct sdhci_host *host, u8 mask)
{
	// disable Intr before reset
	sdhci_writel(host, 0, SDHCI_INT_ENABLE);
	sdhci_writel(host, 0, SDHCI_SIGNAL_ENABLE);

	sdhci_reset(host, mask);

	sdhci_writel(host, host->ier, SDHCI_INT_ENABLE);
	sdhci_writel(host, host->ier, SDHCI_SIGNAL_ENABLE);
}

static void sdhci_cv181x_emmc_reset(struct sdhci_host *host, u8 mask)
{
	u16 ctrl_2;
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_cvi_host *cvi_host = sdhci_pltfm_priv(pltfm_host);

	pr_debug("%s mask = 0x%x\n", __func__, mask);
	sdhci_cvi_reset_helper(host, mask);

	//reg_0x200[0] = 1 for mmc
	sdhci_writel(host,
			 sdhci_readl(host, CVI_CV181X_SDHCI_VENDOR_MSHC_CTRL_R) | BIT(0),
			 CVI_CV181X_SDHCI_VENDOR_MSHC_CTRL_R);

	ctrl_2 = sdhci_readw(host, SDHCI_HOST_CONTROL2);
	ctrl_2 &= SDHCI_CTRL_UHS_MASK;
	if (ctrl_2 == SDHCI_CTRL_UHS_SDR104) {
		//reg_0x200[1] = 0
		sdhci_writel(host,
			sdhci_readl(host, CVI_CV181X_SDHCI_VENDOR_MSHC_CTRL_R) & ~(BIT(1)),
			CVI_CV181X_SDHCI_VENDOR_MSHC_CTRL_R);
		//reg_0x24c[0] = 0
		sdhci_writel(host,
			sdhci_readl(host, CVI_CV181X_SDHCI_PHY_CONFIG) & ~(BIT(0)),
			CVI_CV181X_SDHCI_PHY_CONFIG);
		//reg_0x240[22:16] = tap reg_0x240[9:8] = 1 reg_0x240[6:0] = 0
		sdhci_writel(host,
			(BIT(8) | ((cvi_host->final_tap & 0x7F) << 16)),
			CVI_CV181X_SDHCI_PHY_TX_RX_DLY);
	} else {
		//Reset as DS/HS setting.
		//reg_0x200[1] = 1
		sdhci_writel(host,
			sdhci_readl(host, CVI_CV181X_SDHCI_VENDOR_MSHC_CTRL_R) | BIT(1),
			CVI_CV181X_SDHCI_VENDOR_MSHC_CTRL_R);
		//reg_0x24c[0] = 1
		sdhci_writel(host,
			sdhci_readl(host, CVI_CV181X_SDHCI_PHY_CONFIG) | BIT(0),
			CVI_CV181X_SDHCI_PHY_CONFIG);
		//reg_0x240[25:24] = 1 reg_0x240[22:16] = 0 reg_0x240[9:8] = 1 reg_0x240[6:0] = 0
		sdhci_writel(host, 0x1000100, CVI_CV181X_SDHCI_PHY_TX_RX_DLY);
	}
}

static unsigned int sdhci_cvi_general_get_max_clock(struct sdhci_host *host)
{
	return 375000000;
}

static void sdhci_cvi_emmc_voltage_switch(struct sdhci_host *host)
{
}

static void sdhci_cvi_general_set_uhs_signaling(struct sdhci_host *host, unsigned int uhs)
{
	struct mmc_host *mmc = host->mmc;
	u16 ctrl_2;

	ctrl_2 = sdhci_readw(host, SDHCI_HOST_CONTROL2);
	/* Select Bus Speed Mode for host */
	ctrl_2 &= ~SDHCI_CTRL_UHS_MASK;
	switch (uhs) {
	case MMC_TIMING_UHS_SDR12:
		ctrl_2 |= SDHCI_CTRL_UHS_SDR12;
		break;
	case MMC_TIMING_UHS_SDR25:
		ctrl_2 |= SDHCI_CTRL_UHS_SDR25;
		break;
	case MMC_TIMING_UHS_SDR50:
		ctrl_2 |= SDHCI_CTRL_UHS_SDR50;
		break;
	case MMC_TIMING_MMC_HS200:
	case MMC_TIMING_UHS_SDR104:
		ctrl_2 |= SDHCI_CTRL_UHS_SDR104;
		break;
	case MMC_TIMING_UHS_DDR50:
	case MMC_TIMING_MMC_DDR52:
		ctrl_2 |= SDHCI_CTRL_UHS_DDR50;
		break;
	}

	/*
	 * When clock frequency is less than 100MHz, the feedback clock must be
	 * provided and DLL must not be used so that tuning can be skipped. To
	 * provide feedback clock, the mode selection can be any value less
	 * than 3'b011 in bits [2:0] of HOST CONTROL2 register.
	 */
	if (host->clock <= 100000000 &&
	    (uhs == MMC_TIMING_MMC_HS400 ||
	     uhs == MMC_TIMING_MMC_HS200 ||
	     uhs == MMC_TIMING_UHS_SDR104))
		ctrl_2 &= ~SDHCI_CTRL_UHS_MASK;

	dev_dbg(mmc_dev(mmc), "%s: clock=%u uhs=%u ctrl_2=0x%x\n",
		mmc_hostname(host->mmc), host->clock, uhs, ctrl_2);
	sdhci_writew(host, ctrl_2, SDHCI_HOST_CONTROL2);
}

static void cvi_adma_write_desc(struct sdhci_host *host, void **desc,
		dma_addr_t addr, int len, unsigned int cmd)
{
	int tmplen, offset;

	if (likely(!len || BOUNDARY_OK(addr, len))) {
		sdhci_adma_write_desc(host, desc, addr, len, cmd);
		return;
	}

	offset = addr & (SZ_128M - 1);
	tmplen = SZ_128M - offset;
	sdhci_adma_write_desc(host, desc, addr, tmplen, cmd);

	addr += tmplen;
	len -= tmplen;
	sdhci_adma_write_desc(host, desc, addr, len, cmd);
}

static inline uint32_t CHECK_MASK_BIT(void *_mask, uint32_t bit)
{
	uint32_t w = bit / 8;
	uint32_t off = bit % 8;

	return ((uint8_t *)_mask)[w] & (1 << off);
}

static void reset_after_tuning_pass(struct sdhci_host *host)
{
	pr_debug("tuning pass\n");

	/* Clear BUF_RD_READY intr */
	sdhci_writew(host, sdhci_readw(host, SDHCI_INT_STATUS) & (~(0x1 << 5)),
		     SDHCI_INT_STATUS);

	/* Set SDHCI_SOFTWARE_RESET.SW_RST_DAT = 1 to clear buffered tuning block */
	sdhci_writeb(host, sdhci_readb(host, SDHCI_SOFTWARE_RESET) | (0x1 << 2), SDHCI_SOFTWARE_RESET);

	/* Set SDHCI_SOFTWARE_RESET.SW_RST_CMD = 1	*/
	sdhci_writeb(host, sdhci_readb(host, SDHCI_SOFTWARE_RESET) | (0x1 << 1), SDHCI_SOFTWARE_RESET);

	while (sdhci_readb(host, SDHCI_SOFTWARE_RESET) & 0x3)
		;
}

static void sdhci_cvi_cv181x_set_tap(struct sdhci_host *host, unsigned int tap)
{
	pr_debug("%s %d\n", __func__, tap);
	// Set sd_clk_en(0x2c[2]) to 0
	sdhci_writew(host, sdhci_readw(host, SDHCI_CLOCK_CONTROL) & (~(0x1 << 2)), SDHCI_CLOCK_CONTROL);
	sdhci_writel(host,
		sdhci_readl(host, CVI_CV181X_SDHCI_VENDOR_MSHC_CTRL_R) & (~(BIT(1))),
		CVI_CV181X_SDHCI_VENDOR_MSHC_CTRL_R);
	sdhci_writel(host, BIT(8) | tap << 16,
		     CVI_CV181X_SDHCI_PHY_TX_RX_DLY);
	sdhci_writel(host, 0, CVI_CV181X_SDHCI_PHY_CONFIG);
	// Set sd_clk_en(0x2c[2]) to 1
	sdhci_writew(host, sdhci_readw(host, SDHCI_CLOCK_CONTROL) | (0x1 << 2), SDHCI_CLOCK_CONTROL);
	mdelay(1);
}

static inline void SET_MASK_BIT(void *_mask, uint32_t bit)
{
	uint32_t byte = bit / 8;
	uint32_t offset = bit % 8;
	((uint8_t *)_mask)[byte] |= (1 << offset);
}

static int sdhci_cv181x_general_execute_tuning(struct sdhci_host *host, u32 opcode)
{
	u16 min = 0;
	u32 k = 0;
	s32 ret;
	u32 retry_cnt = 0;

	u32 tuning_result[4] = {0, 0, 0, 0};
	u32 rx_lead_lag_result[4] = {0, 0, 0, 0};
	char tuning_graph[TUNE_MAX_PHCODE+1];
	char rx_lead_lag_graph[TUNE_MAX_PHCODE+1];

	u32 reg = 0;
	u32 reg_rx_lead_lag = 0;
	s32 max_lead_lag_idx = -1;
	s32 max_window_idx = -1;
	s32 cur_window_idx = -1;
	u16 max_lead_lag_size = 0;
	u16 max_window_size = 0;
	u16 cur_window_size = 0;
	s32 rx_lead_lag_phase = -1;
	s32 final_tap = -1;
	u32 rate = 0;

	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_cvi_host *cvi_host = sdhci_pltfm_priv(pltfm_host);

	reg = sdhci_readw(host, SDHCI_HOST_CONTROL2);
	pr_debug("%s : host ctrl2 0x%x\n", mmc_hostname(host->mmc), reg);
	/* Set Host_CTRL2_R.SAMPLE_CLK_SEL=0 */
	sdhci_writew(host,
			 sdhci_readw(host, SDHCI_HOST_CONTROL2) & (~(0x1 << 7)),
			 SDHCI_HOST_CONTROL2);
	sdhci_writew(host,
			 sdhci_readw(host, SDHCI_HOST_CONTROL2) & (~(0x3 << 4)),
			 SDHCI_HOST_CONTROL2);

	reg = sdhci_readw(host, SDHCI_HOST_CONTROL2);
	pr_debug("%s : host ctrl2 0x%x\n", mmc_hostname(host->mmc), reg);

	while (min < TUNE_MAX_PHCODE) {
		retry_cnt = 0;
		sdhci_cvi_cv181x_set_tap(host, min);
		reg_rx_lead_lag = sdhci_readw(host, CVI_CV181X_SDHCI_PHY_DLY_STS) & BIT(1);

retry_tuning:
		ret = mmc_send_tuning(host->mmc, opcode, NULL);

		if (!ret && retry_cnt < MAX_TUNING_CMD_RETRY_COUNT) {
			retry_cnt++;
			goto retry_tuning;
		}

		if (ret) {
			SET_MASK_BIT(tuning_result, min);
		}

		if (reg_rx_lead_lag) {
			SET_MASK_BIT(rx_lead_lag_result, min);
		}

		min++;
	}

	reset_after_tuning_pass(host);

	pr_debug("tuning result:      0x%08x 0x%08x 0x%08x 0x%08x\n",
		tuning_result[0], tuning_result[1], tuning_result[2], tuning_result[3]);
	pr_debug("rx_lead_lag result: 0x%08x 0x%08x 0x%08x 0x%08x\n",
		rx_lead_lag_result[0], rx_lead_lag_result[1], rx_lead_lag_result[2], rx_lead_lag_result[3]);
	for (k = 0; k < TUNE_MAX_PHCODE; k++) {
		if (CHECK_MASK_BIT(tuning_result, k) == 0)
			tuning_graph[k] = '-';
		else
			tuning_graph[k] = 'x';
		if (CHECK_MASK_BIT(rx_lead_lag_result, k) == 0)
			rx_lead_lag_graph[k] = '0';
		else
			rx_lead_lag_graph[k] = '1';
	}
	tuning_graph[TUNE_MAX_PHCODE] = '\0';
	rx_lead_lag_graph[TUNE_MAX_PHCODE] = '\0';

	pr_debug("tuning graph:      %s\n", tuning_graph);
	pr_debug("rx_lead_lag graph: %s\n", rx_lead_lag_graph);

	// Find a final tap as median of maximum window
	for (k = 0; k < TUNE_MAX_PHCODE; k++) {
		if (CHECK_MASK_BIT(tuning_result, k) == 0) {
			if (-1 == cur_window_idx) {
				cur_window_idx = k;
			}
			cur_window_size++;

			if (cur_window_size > max_window_size) {
				max_window_size = cur_window_size;
				max_window_idx = cur_window_idx;
				if (max_window_size >= TAP_WINDOW_THLD)
					final_tap = cur_window_idx + (max_window_size/2);
			}
		} else {
			cur_window_idx = -1;
			cur_window_size = 0;
		}
	}

	cur_window_idx = -1;
	cur_window_size = 0;
	for (k = 0; k < TUNE_MAX_PHCODE; k++) {
		if (CHECK_MASK_BIT(rx_lead_lag_result, k) == 0) {
			//from 1 to 0 and window_size already computed.
			if ((rx_lead_lag_phase == 1) && (cur_window_size > 0)) {
				max_lead_lag_idx = cur_window_idx;
				max_lead_lag_size = cur_window_size;
				break;
			}
			if (cur_window_idx == -1) {
				cur_window_idx = k;
			}
			cur_window_size++;
			rx_lead_lag_phase = 0;
		} else {
			rx_lead_lag_phase = 1;
			if ((cur_window_idx != -1) && (cur_window_size > 0)) {
				cur_window_size++;
				max_lead_lag_idx = cur_window_idx;
				max_lead_lag_size = cur_window_size;
			} else {
				cur_window_size = 0;
			}
		}
	}
	rate = max_window_size * 100 / max_lead_lag_size;
	pr_debug("MaxWindow[Idx, Width]:[%d,%u] Tuning Tap: %d\n", max_window_idx, max_window_size, final_tap);
	pr_debug("RX_LeadLag[Idx, Width]:[%d,%u] rate = %d\n", max_lead_lag_idx, max_lead_lag_size, rate);

	sdhci_cvi_cv181x_set_tap(host, final_tap);
	cvi_host->final_tap = final_tap;
	pr_debug("%s finished tuning, code:%d\n", __func__, final_tap);

	return mmc_send_tuning(host->mmc, opcode, NULL);
}

static const struct sdhci_ops sdhci_cv181x_emmc_ops = {
	.reset = sdhci_cv181x_emmc_reset,
	.set_clock = sdhci_set_clock,
	.set_bus_width = sdhci_set_bus_width,
	.get_max_clock = sdhci_cvi_general_get_max_clock,
	.voltage_switch = sdhci_cvi_emmc_voltage_switch,
	.set_uhs_signaling = sdhci_cvi_general_set_uhs_signaling,
	.platform_execute_tuning = sdhci_cv181x_general_execute_tuning,
	.adma_write_desc = cvi_adma_write_desc,
};

static void sdhci_cv181x_sd_voltage_restore(struct sdhci_host *host, bool bunplug)
{
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_cvi_host *cvi_host = sdhci_pltfm_priv(pltfm_host);

	pr_debug("%s\n", __func__);

	if (bunplug) {
		//Voltage close flow
		//(reg_pwrsw_auto=1, reg_pwrsw_disc=1, reg_pwrsw_vsel=1(1.8v), reg_en_pwrsw=0)
		writel(0xE | (readl(cvi_host->topbase + OFFSET_SD_PWRSW_CTRL) & 0xFFFFFFF0),
			cvi_host->topbase + OFFSET_SD_PWRSW_CTRL);
		cvi_host->sdio0_voltage_1_8_v = 0;
	} else {
		if (!cvi_host->sdio0_voltage_1_8_v) {
			//Voltage switching flow (3.3)
			//(reg_pwrsw_auto=1, reg_pwrsw_disc=0, reg_pwrsw_vsel=0(3.0v), reg_en_pwrsw=1)
			writel(0x9 | (readl(cvi_host->topbase + OFFSET_SD_PWRSW_CTRL) & 0xFFFFFFF0),
				cvi_host->topbase + OFFSET_SD_PWRSW_CTRL);
		}
	}

	//wait 1ms
	mdelay(1);

	// restore to DS/HS setting
	sdhci_writel(host,
		sdhci_readl(host, CVI_CV181X_SDHCI_VENDOR_MSHC_CTRL_R) | BIT(1) | BIT(8) | BIT(9),
		CVI_CV181X_SDHCI_VENDOR_MSHC_CTRL_R);
	sdhci_writel(host, 0x1000100, CVI_CV181X_SDHCI_PHY_TX_RX_DLY);
	sdhci_writel(host, 1, CVI_CV181X_SDHCI_PHY_CONFIG);

	mdelay(1);
}

static void sdhci_cv181x_emmc_setup_pad(struct sdhci_host *host)
{
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_cvi_host *cvi_host = sdhci_pltfm_priv(pltfm_host);

	/* Name              Offset
	 * PAD_EMMC_RSTN     0x48
	 * PAD_EMMC_CLK      0x50
	 * PAD_EMMC_CMD      0x5C
	 * PAD_EMMC_DAT0     0x54
	 * PAD_EMMC_DAT1     0x60
	 * PAD_EMMC_DAT2     0x4C
	 * PAD_EMMC_DAT3     0x58

	 */

	u8 val = 0x0;

	writeb(val, cvi_host->pinmuxbase + 0x48);
	writeb(val, cvi_host->pinmuxbase + 0x50);
	writeb(val, cvi_host->pinmuxbase + 0x5C);
	writeb(val, cvi_host->pinmuxbase + 0x54);
	writeb(val, cvi_host->pinmuxbase + 0x60);
	writeb(val, cvi_host->pinmuxbase + 0x4C);
	writeb(val, cvi_host->pinmuxbase + 0x58);
}

static const struct sdhci_pltfm_data sdhci_cv181x_emmc_pdata = {
	.ops = &sdhci_cv181x_emmc_ops,
	.quirks = SDHCI_QUIRK_INVERTED_WRITE_PROTECT | SDHCI_QUIRK_CAP_CLOCK_BASE_BROKEN,
	.quirks2 = SDHCI_QUIRK2_PRESET_VALUE_BROKEN,
};

/**********************************************************/
static void sdhci_cv181x_sd_setup_pad(struct sdhci_host *host, bool bunplug)
{
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_cvi_host *cvi_host = sdhci_pltfm_priv(pltfm_host);

	/* Name              Offset unplug plug
	 * PAD_SDIO0_CD      0x34   SDIO0  SDIO0
	 * PAD_SDIO0_PWR_EN  0x38   SDIO0  SDIO0
	 * PAD_SDIO0_CLK     0x1C   XGPIO  SDIO0
	 * PAD_SDIO0_CMD     0x20   XGPIO  SDIO0
	 * PAD_SDIO0_D0      0x24   XGPIO  SDIO0
	 * PAD_SDIO0_D1      0x28   XGPIO  SDIO0
	 * PAD_SDIO0_D2      0x2C   XGPIO  SDIO0
	 * PAD_SDIO0_D3      0x30   XGPIO  SDIO0
	 * 0x0: SDIO0 function
	 * 0x3: XGPIO function
	 */

	u8 val = (bunplug) ? 0x3 : 0x0;


	writeb(0x0, cvi_host->pinmuxbase + 0x34);

	writeb(0x0, cvi_host->pinmuxbase + 0x38);
	writeb(val, cvi_host->pinmuxbase + 0x1C);
	writeb(val, cvi_host->pinmuxbase + 0x20);
	writeb(val, cvi_host->pinmuxbase + 0x24);
	writeb(val, cvi_host->pinmuxbase + 0x28);
	writeb(val, cvi_host->pinmuxbase + 0x2C);
	writeb(val, cvi_host->pinmuxbase + 0x30);
}

static void sdhci_cv181x_sd_setup_io(struct sdhci_host *host, bool reset)
{
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_cvi_host *cvi_host = sdhci_pltfm_priv(pltfm_host);

	/*
	 * Name              Offset reset sd0
	 * REG_SDIO0_CD      0x900  PU    PU
	 * REG_SDIO0_PWR_EN  0x904  PD    PD
	 * REG_SDIO0_CLK     0xA00  PD    PD
	 * REG_SDIO0_CMD     0xA04  PD    PU
	 * REG_SDIO0_D0      0xA08  PD    PU
	 * REG_SDIO0_D1      0xA0C  PD    PU
	 * REG_SDIO0_D2      0xA10  PD    PU
	 * REG_SDIO0_D3      0xA14  PD    PU
	 * BIT(2) : PU   enable(1)/disable(0)
	 * BIT(3) : PD   enable(1)/disable(0)
	 */

	u8 raise_bit = (reset) ?  BIT(3) : BIT(2);
	u8 down_bit  = (reset) ?  BIT(2) : BIT(3);

	writeb(((readb(cvi_host->pinmuxbase + 0x900) | BIT(2)) & ~(BIT(3))),
		cvi_host->pinmuxbase + 0x900);
	writeb(((readb(cvi_host->pinmuxbase + 0x904) | BIT(3)) & ~(BIT(2))),
		cvi_host->pinmuxbase + 0x904);
	writeb(((readb(cvi_host->pinmuxbase + 0xA00) | BIT(3)) & ~(BIT(2))),
		cvi_host->pinmuxbase + 0xA00);
	writeb(((readb(cvi_host->pinmuxbase + 0xA04) | raise_bit) & ~(down_bit)),
		cvi_host->pinmuxbase + 0xA04);
	writeb(((readb(cvi_host->pinmuxbase + 0xA08) | raise_bit) & ~(down_bit)),
		cvi_host->pinmuxbase + 0xA08);
	writeb(((readb(cvi_host->pinmuxbase + 0xA0C) | raise_bit) & ~(down_bit)),
		cvi_host->pinmuxbase + 0xA0C);
	writeb(((readb(cvi_host->pinmuxbase + 0xA10) | raise_bit) & ~(down_bit)),
		cvi_host->pinmuxbase + 0xA10);
	writeb(((readb(cvi_host->pinmuxbase + 0xA14) | raise_bit) & ~(down_bit)),
		cvi_host->pinmuxbase + 0xA14);
}

static void sdhci_cv181x_sd_reset(struct sdhci_host *host, u8 mask)
{
	u16 ctrl_2;
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_cvi_host *cvi_host = sdhci_pltfm_priv(pltfm_host);

	pr_debug("%s mask = 0x%x\n", __func__, mask);
	sdhci_cvi_reset_helper(host, mask);

	ctrl_2 = sdhci_readw(host, SDHCI_HOST_CONTROL2);
	ctrl_2 &= SDHCI_CTRL_UHS_MASK;
	if (ctrl_2 == SDHCI_CTRL_UHS_SDR104) {
		//reg_0x200[1] = 0
		sdhci_writel(host,
			sdhci_readl(host, CVI_CV181X_SDHCI_VENDOR_MSHC_CTRL_R) & ~(BIT(1)),
			CVI_CV181X_SDHCI_VENDOR_MSHC_CTRL_R);
		//reg_0x24c[0] = 0
		sdhci_writel(host,
			sdhci_readl(host, CVI_CV181X_SDHCI_PHY_CONFIG) & ~(BIT(0)),
			CVI_CV181X_SDHCI_PHY_CONFIG);
		//reg_0x240[22:16] = tap reg_0x240[9:8] = 1 reg_0x240[6:0] = 0
		sdhci_writel(host,
			(BIT(8) | ((cvi_host->final_tap & 0x7F) << 16)),
			CVI_CV181X_SDHCI_PHY_TX_RX_DLY);
	} else {
		//Reset as DS/HS setting.
		//reg_0x200[1] = 1
		sdhci_writel(host,
			sdhci_readl(host, CVI_CV181X_SDHCI_VENDOR_MSHC_CTRL_R) | BIT(1),
			CVI_CV181X_SDHCI_VENDOR_MSHC_CTRL_R);
		//reg_0x24c[0] = 1
		sdhci_writel(host,
			sdhci_readl(host, CVI_CV181X_SDHCI_PHY_CONFIG) | BIT(0),
			CVI_CV181X_SDHCI_PHY_CONFIG);
		//reg_0x240[25:24] = 1 reg_0x240[22:16] = 0 reg_0x240[9:8] = 1 reg_0x240[6:0] = 0
		sdhci_writel(host, 0x1000100, CVI_CV181X_SDHCI_PHY_TX_RX_DLY);
	}
}

static void sdhci_cv181x_sd_set_power(struct sdhci_host *host, unsigned char mode,
				unsigned short vdd)
{
	struct mmc_host *mmc = host->mmc;

	pr_debug("%s:mode %u, vdd %u\n", __func__, mode, vdd);

	if (mode == MMC_POWER_ON && mmc->ops->get_cd(mmc)) {
		sdhci_set_power_noreg(host, mode, vdd);
		sdhci_cv181x_sd_voltage_restore(host, false);
		sdhci_cv181x_sd_setup_pad(host, false);
		sdhci_cv181x_sd_setup_io(host, false);
		mdelay(5);
	} else if (mode == MMC_POWER_OFF) {
		sdhci_cv181x_sd_setup_pad(host, true);
		sdhci_cv181x_sd_setup_io(host, true);
		sdhci_cv181x_sd_voltage_restore(host, true);
		sdhci_set_power_noreg(host, mode, vdd);
		mdelay(30);
	}
}

static void sdhci_cv181x_sd_voltage_switch(struct sdhci_host *host)
{
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_cvi_host *cvi_host = sdhci_pltfm_priv(pltfm_host);

	pr_debug("%s\n", __func__);

	// enable SDIO0_CLK[7:5] to set CLK max strengh
	writeb((readb(cvi_host->pinmuxbase + 0xA00) | BIT(7) | BIT(6) | BIT(5)),
		cvi_host->pinmuxbase + 0xA00);

	//Voltage switching flow (1.8v)
	//reg_pwrsw_auto=1, reg_pwrsw_disc=0, pwrsw_vsel=1(1.8v), reg_en_pwrsw=1
	writel(0xB | (readl(cvi_host->topbase + OFFSET_SD_PWRSW_CTRL) & 0xFFFFFFF0),
		cvi_host->topbase + OFFSET_SD_PWRSW_CTRL);
	pr_debug("sd PWRSW 0x%x\n", readl(cvi_host->topbase + OFFSET_SD_PWRSW_CTRL));
	cvi_host->sdio0_voltage_1_8_v = 1;

	mdelay(1);
}

static const struct sdhci_ops sdhci_cv181x_sd_ops = {
	.reset = sdhci_cv181x_sd_reset,
	.set_clock = sdhci_set_clock,
	.set_power = sdhci_cv181x_sd_set_power,
	.set_bus_width = sdhci_set_bus_width,
	.get_max_clock = sdhci_cvi_general_get_max_clock,
	.voltage_switch = sdhci_cv181x_sd_voltage_switch,
	.set_uhs_signaling = sdhci_cvi_general_set_uhs_signaling,
	.platform_execute_tuning = sdhci_cv181x_general_execute_tuning,
	.adma_write_desc = cvi_adma_write_desc,
};

static const struct sdhci_pltfm_data sdhci_cv181x_sd_pdata = {
	.ops = &sdhci_cv181x_sd_ops,
	.quirks = SDHCI_QUIRK_INVERTED_WRITE_PROTECT | SDHCI_QUIRK_CAP_CLOCK_BASE_BROKEN,
	.quirks2 = SDHCI_QUIRK2_PRESET_VALUE_BROKEN,
};

static const struct of_device_id sdhci_cvi_dt_match[] = {
	{.compatible = "cvitek,cv181x-emmc", .data = &sdhci_cv181x_emmc_pdata},
    {.compatible = "cvitek,cv181x-sd", .data = &sdhci_cv181x_sd_pdata},

	{ /* sentinel */ }
};

int sdhci_cvi_probe(struct platform_device *pdev)
{
	struct sdhci_host *host;
	struct sdhci_pltfm_host *pltfm_host;
	struct sdhci_cvi_host *cvi_host;
	const struct of_device_id *match;
	const struct sdhci_pltfm_data *pdata;
	struct clk *clk_sd;
	int ret;
	int gpio_cd = -EINVAL;
	u32 extra;

	match = of_match_device(sdhci_cvi_dt_match, &pdev->dev);
	if (!match)
		return -EINVAL;

	pdata = match->data;

	host = sdhci_pltfm_init(pdev, pdata, sizeof(*cvi_host));
	if (IS_ERR(host))
		return PTR_ERR(host);

	pltfm_host = sdhci_priv(host);
	cvi_host = sdhci_pltfm_priv(pltfm_host);
	cvi_host->host = host;
	cvi_host->mmc = host->mmc;
	cvi_host->pdev = pdev;
	cvi_host->core_mem = host->ioaddr;
	cvi_host->topbase = ioremap(TOP_BASE, 0x2000);
	cvi_host->pinmuxbase = ioremap(PINMUX_BASE, 0x1000);
	cvi_host->clkgenbase = ioremap(CLKGEN_BASE, 0x100);

	sdhci_cv181x_sd_voltage_restore(host, false);

	ret = mmc_of_parse(host->mmc);
	if (ret)
		goto pltfm_free;

	sdhci_get_of_property(pdev);

	ret = sdhci_add_host(host);
	if (ret)
		goto err_add_host;

	platform_set_drvdata(pdev, cvi_host);

	if (strstr(dev_name(mmc_dev(host->mmc)), "cv-emmc"))
		sdhci_cv181x_emmc_setup_pad(host);

	return 0;

err_add_host:
pltfm_free:
	sdhci_pltfm_free(pdev);
	return ret;
}
