/* Copyright (c) 2012-2013, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/err.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/iopoll.h>
#include <linux/clk.h>

#include <asm/processor.h>
#include <mach/msm_iomap.h>
#include <mach/clk-provider.h>
#include <mach/clk.h>
#include <mach/clock-generic.h>

#include "clock-mdss-8974.h"

#define REG_R(addr)			readl_relaxed(addr)
#define REG_W(data, addr)		writel_relaxed(data, addr)
#define DSS_REG_W(base, offset, data)	REG_W((data), (base) + (offset))
#define DSS_REG_R(base, offset)		REG_R((base) + (offset))

#define GDSC_PHYS		0xFD8C2304
#define GDSC_SIZE		0x4

#define DSI_PHY_PHYS		0xFD922A00
#define DSI_PHY_SIZE		0x000000D4

#define EDP_PHY_PHYS		0xFD923A00
#define EDP_PHY_SIZE		0x000000D4

#define HDMI_PHY_PHYS		0xFD922500
#define HDMI_PHY_SIZE		0x0000007C

#define HDMI_PHY_PLL_PHYS	0xFD922700
#define HDMI_PHY_PLL_SIZE	0x000000D4

/* hdmi phy registers */
#define HDMI_PHY_ANA_CFG0               (0x0000)
#define HDMI_PHY_ANA_CFG1               (0x0004)
#define HDMI_PHY_ANA_CFG2               (0x0008)
#define HDMI_PHY_ANA_CFG3               (0x000C)
#define HDMI_PHY_PD_CTRL0               (0x0010)
#define HDMI_PHY_PD_CTRL1               (0x0014)
#define HDMI_PHY_GLB_CFG                (0x0018)
#define HDMI_PHY_DCC_CFG0               (0x001C)
#define HDMI_PHY_DCC_CFG1               (0x0020)
#define HDMI_PHY_TXCAL_CFG0             (0x0024)
#define HDMI_PHY_TXCAL_CFG1             (0x0028)
#define HDMI_PHY_TXCAL_CFG2             (0x002C)
#define HDMI_PHY_TXCAL_CFG3             (0x0030)
#define HDMI_PHY_BIST_CFG0              (0x0034)
#define HDMI_PHY_BIST_CFG1              (0x0038)
#define HDMI_PHY_BIST_PATN0             (0x003C)
#define HDMI_PHY_BIST_PATN1             (0x0040)
#define HDMI_PHY_BIST_PATN2             (0x0044)
#define HDMI_PHY_BIST_PATN3             (0x0048)
#define HDMI_PHY_STATUS                 (0x005C)

/* hdmi phy unified pll registers */
#define HDMI_UNI_PLL_REFCLK_CFG         (0x0000)
#define HDMI_UNI_PLL_POSTDIV1_CFG       (0x0004)
#define HDMI_UNI_PLL_CHFPUMP_CFG        (0x0008)
#define HDMI_UNI_PLL_VCOLPF_CFG         (0x000C)
#define HDMI_UNI_PLL_VREG_CFG           (0x0010)
#define HDMI_UNI_PLL_PWRGEN_CFG         (0x0014)
#define HDMI_UNI_PLL_GLB_CFG            (0x0020)
#define HDMI_UNI_PLL_POSTDIV2_CFG       (0x0024)
#define HDMI_UNI_PLL_POSTDIV3_CFG       (0x0028)
#define HDMI_UNI_PLL_LPFR_CFG           (0x002C)
#define HDMI_UNI_PLL_LPFC1_CFG          (0x0030)
#define HDMI_UNI_PLL_LPFC2_CFG          (0x0034)
#define HDMI_UNI_PLL_SDM_CFG0           (0x0038)
#define HDMI_UNI_PLL_SDM_CFG1           (0x003C)
#define HDMI_UNI_PLL_SDM_CFG2           (0x0040)
#define HDMI_UNI_PLL_SDM_CFG3           (0x0044)
#define HDMI_UNI_PLL_SDM_CFG4           (0x0048)
#define HDMI_UNI_PLL_SSC_CFG0           (0x004C)
#define HDMI_UNI_PLL_SSC_CFG1           (0x0050)
#define HDMI_UNI_PLL_SSC_CFG2           (0x0054)
#define HDMI_UNI_PLL_SSC_CFG3           (0x0058)
#define HDMI_UNI_PLL_LKDET_CFG0         (0x005C)
#define HDMI_UNI_PLL_LKDET_CFG1         (0x0060)
#define HDMI_UNI_PLL_LKDET_CFG2         (0x0064)
#define HDMI_UNI_PLL_CAL_CFG0           (0x006C)
#define HDMI_UNI_PLL_CAL_CFG1           (0x0070)
#define HDMI_UNI_PLL_CAL_CFG2           (0x0074)
#define HDMI_UNI_PLL_CAL_CFG3           (0x0078)
#define HDMI_UNI_PLL_CAL_CFG4           (0x007C)
#define HDMI_UNI_PLL_CAL_CFG5           (0x0080)
#define HDMI_UNI_PLL_CAL_CFG6           (0x0084)
#define HDMI_UNI_PLL_CAL_CFG7           (0x0088)
#define HDMI_UNI_PLL_CAL_CFG8           (0x008C)
#define HDMI_UNI_PLL_CAL_CFG9           (0x0090)
#define HDMI_UNI_PLL_CAL_CFG10          (0x0094)
#define HDMI_UNI_PLL_CAL_CFG11          (0x0098)
#define HDMI_UNI_PLL_STATUS             (0x00C0)


#define DSI_0_PHY_PLL_UNIPHY_PLL_REFCLK_CFG		(0x00000000)
#define DSI_0_PHY_PLL_UNIPHY_PLL_POSTDIV1_CFG		(0x00000004)
#define DSI_0_PHY_PLL_UNIPHY_PLL_CHGPUMP_CFG		(0x00000008)
#define DSI_0_PHY_PLL_UNIPHY_PLL_VCOLPF_CFG		(0x0000000C)
#define DSI_0_PHY_PLL_UNIPHY_PLL_VREG_CFG		(0x00000010)
#define DSI_0_PHY_PLL_UNIPHY_PLL_PWRGEN_CFG		(0x00000014)
#define DSI_0_PHY_PLL_UNIPHY_PLL_DMUX_CFG		(0x00000018)
#define DSI_0_PHY_PLL_UNIPHY_PLL_AMUX_CFG		(0x0000001C)
#define DSI_0_PHY_PLL_UNIPHY_PLL_GLB_CFG		(0x00000020)
#define DSI_0_PHY_PLL_UNIPHY_PLL_POSTDIV2_CFG		(0x00000024)
#define DSI_0_PHY_PLL_UNIPHY_PLL_POSTDIV3_CFG		(0x00000028)
#define DSI_0_PHY_PLL_UNIPHY_PLL_LPFR_CFG		(0x0000002C)
#define DSI_0_PHY_PLL_UNIPHY_PLL_LPFC1_CFG		(0x00000030)
#define DSI_0_PHY_PLL_UNIPHY_PLL_LPFC2_CFG		(0x00000034)
#define DSI_0_PHY_PLL_UNIPHY_PLL_SDM_CFG0		(0x00000038)
#define DSI_0_PHY_PLL_UNIPHY_PLL_SDM_CFG1		(0x0000003C)
#define DSI_0_PHY_PLL_UNIPHY_PLL_SDM_CFG2		(0x00000040)
#define DSI_0_PHY_PLL_UNIPHY_PLL_SDM_CFG3		(0x00000044)
#define DSI_0_PHY_PLL_UNIPHY_PLL_SDM_CFG4		(0x00000048)
#define DSI_0_PHY_PLL_UNIPHY_PLL_SSC_CFG0		(0x0000004C)
#define DSI_0_PHY_PLL_UNIPHY_PLL_SSC_CFG1		(0x00000050)
#define DSI_0_PHY_PLL_UNIPHY_PLL_SSC_CFG2		(0x00000054)
#define DSI_0_PHY_PLL_UNIPHY_PLL_SSC_CFG3		(0x00000058)
#define DSI_0_PHY_PLL_UNIPHY_PLL_LKDET_CFG0		(0x0000005C)
#define DSI_0_PHY_PLL_UNIPHY_PLL_LKDET_CFG1		(0x00000060)
#define DSI_0_PHY_PLL_UNIPHY_PLL_LKDET_CFG2		(0x00000064)
#define DSI_0_PHY_PLL_UNIPHY_PLL_TEST_CFG		(0x00000068)
#define DSI_0_PHY_PLL_UNIPHY_PLL_CAL_CFG0		(0x0000006C)
#define DSI_0_PHY_PLL_UNIPHY_PLL_CAL_CFG1		(0x00000070)
#define DSI_0_PHY_PLL_UNIPHY_PLL_CAL_CFG2		(0x00000074)
#define DSI_0_PHY_PLL_UNIPHY_PLL_CAL_CFG3		(0x00000078)
#define DSI_0_PHY_PLL_UNIPHY_PLL_CAL_CFG4		(0x0000007C)
#define DSI_0_PHY_PLL_UNIPHY_PLL_CAL_CFG5		(0x00000080)
#define DSI_0_PHY_PLL_UNIPHY_PLL_CAL_CFG6		(0x00000084)
#define DSI_0_PHY_PLL_UNIPHY_PLL_CAL_CFG7		(0x00000088)
#define DSI_0_PHY_PLL_UNIPHY_PLL_CAL_CFG8		(0x0000008C)
#define DSI_0_PHY_PLL_UNIPHY_PLL_CAL_CFG9		(0x00000090)
#define DSI_0_PHY_PLL_UNIPHY_PLL_CAL_CFG10		(0x00000094)
#define DSI_0_PHY_PLL_UNIPHY_PLL_CAL_CFG11		(0x00000098)
#define DSI_0_PHY_PLL_UNIPHY_PLL_EFUSE_CFG		(0x0000009C)
#define DSI_0_PHY_PLL_UNIPHY_PLL_STATUS			(0x000000C0)

#define PLL_POLL_MAX_READS	10
#define PLL_POLL_TIMEOUT_US	50

static long vco_cached_rate;
static unsigned char *mdss_dsi_base;
static unsigned char *gdsc_base;
static struct clk *mdss_ahb_clk;
static unsigned char *mdss_edp_base;

static void __iomem *hdmi_phy_base;
static void __iomem *hdmi_phy_pll_base;
static unsigned hdmi_pll_on;



int mdss_gdsc_enabled(void)
{
	if (!gdsc_base)
		return 0;

	return !!(readl_relaxed(gdsc_base) & BIT(31));
}



void hdmi_pll_disable(void)
{
	clk_enable(mdss_ahb_clk);
	REG_W(0x0, hdmi_phy_pll_base + HDMI_UNI_PLL_GLB_CFG);
	udelay(5);
	REG_W(0x0, hdmi_phy_base + HDMI_PHY_GLB_CFG);
	clk_disable(mdss_ahb_clk);

	hdmi_pll_on = 0;
} /* hdmi_pll_disable */

int hdmi_pll_enable(void)
{
	u32 status;
	u32 max_reads, timeout_us;

	clk_enable(mdss_ahb_clk);
	/* Global Enable */
	REG_W(0x81, hdmi_phy_base + HDMI_PHY_GLB_CFG);
	/* Power up power gen */
	REG_W(0x00, hdmi_phy_base + HDMI_PHY_PD_CTRL0);
	udelay(350);

	/* PLL Power-Up */
	REG_W(0x01, hdmi_phy_pll_base + HDMI_UNI_PLL_GLB_CFG);
	udelay(5);
	/* Power up PLL LDO */
	REG_W(0x03, hdmi_phy_pll_base + HDMI_UNI_PLL_GLB_CFG);
	udelay(350);

	/* PLL Power-Up */
	REG_W(0x0F, hdmi_phy_pll_base + HDMI_UNI_PLL_GLB_CFG);
	udelay(350);

	/* poll for PLL ready status */
	max_reads = 20;
	timeout_us = 100;
	if (readl_poll_timeout_noirq((hdmi_phy_pll_base + HDMI_UNI_PLL_STATUS),
		status, ((status & BIT(0)) == 1), max_reads, timeout_us)) {
		pr_err("%s: hdmi phy pll status=%x failed to Lock\n",
		       __func__, status);
		hdmi_pll_disable();
		clk_disable(mdss_ahb_clk);
		return -EINVAL;
	}
	pr_err("%s: hdmi phy pll is locked\n", __func__);

	udelay(350);
	/* poll for PHY ready status */
	max_reads = 20;
	timeout_us = 100;
	if (readl_poll_timeout_noirq((hdmi_phy_base + HDMI_PHY_STATUS),
		status, ((status & BIT(0)) == 1), max_reads, timeout_us)) {
		pr_err("%s: hdmi phy status=%x failed to Lock\n",
		       __func__, status);
		hdmi_pll_disable();
		clk_disable(mdss_ahb_clk);
		return -EINVAL;
	}
	pr_err("%s: hdmi phy is locked\n", __func__);
	clk_disable(mdss_ahb_clk);

	hdmi_pll_on = 1;

	return 0;
} /* hdmi_pll_enable */

int hdmi_pll_set_rate(unsigned long rate)
{
	unsigned int set_power_dwn = 0;

	if (hdmi_pll_on) {
		hdmi_pll_disable();
		set_power_dwn = 1;
	}

	clk_enable(mdss_ahb_clk);
	pr_debug("%s: rate=%ld\n", __func__, rate);
	switch (rate) {
	case 0:
		/* This case is needed for suspend/resume. */
	break;

	case 25200000:
		/* 640x480p60 */
		REG_W(0x81, hdmi_phy_base + HDMI_PHY_GLB_CFG);
		REG_W(0x01, hdmi_phy_pll_base + HDMI_UNI_PLL_GLB_CFG);
		REG_W(0x01, hdmi_phy_pll_base + HDMI_UNI_PLL_REFCLK_CFG);
		REG_W(0x19, hdmi_phy_pll_base + HDMI_UNI_PLL_VCOLPF_CFG);
		REG_W(0x0E, hdmi_phy_pll_base + HDMI_UNI_PLL_LPFR_CFG);
		REG_W(0x20, hdmi_phy_pll_base + HDMI_UNI_PLL_LPFC1_CFG);
		REG_W(0x0D, hdmi_phy_pll_base + HDMI_UNI_PLL_LPFC2_CFG);
		REG_W(0x00, hdmi_phy_pll_base + HDMI_UNI_PLL_SDM_CFG0);
		REG_W(0x52, hdmi_phy_pll_base + HDMI_UNI_PLL_SDM_CFG1);
		REG_W(0x00, hdmi_phy_pll_base + HDMI_UNI_PLL_SDM_CFG2);
		REG_W(0xB0, hdmi_phy_pll_base + HDMI_UNI_PLL_SDM_CFG3);
		REG_W(0x00, hdmi_phy_pll_base + HDMI_UNI_PLL_SDM_CFG4);
		REG_W(0x10, hdmi_phy_pll_base + HDMI_UNI_PLL_LKDET_CFG0);
		REG_W(0x1A, hdmi_phy_pll_base + HDMI_UNI_PLL_LKDET_CFG1);
		REG_W(0x05, hdmi_phy_pll_base + HDMI_UNI_PLL_LKDET_CFG2);
		REG_W(0x03, hdmi_phy_pll_base + HDMI_UNI_PLL_POSTDIV1_CFG);
		REG_W(0x00, hdmi_phy_pll_base + HDMI_UNI_PLL_POSTDIV2_CFG);
		REG_W(0x00, hdmi_phy_pll_base + HDMI_UNI_PLL_POSTDIV3_CFG);
		REG_W(0x01, hdmi_phy_pll_base + HDMI_UNI_PLL_CAL_CFG2);
		REG_W(0x60, hdmi_phy_pll_base + HDMI_UNI_PLL_CAL_CFG8);
		REG_W(0x00, hdmi_phy_pll_base + HDMI_UNI_PLL_CAL_CFG9);
		REG_W(0xF4, hdmi_phy_pll_base + HDMI_UNI_PLL_CAL_CFG10);
		REG_W(0x02, hdmi_phy_pll_base + HDMI_UNI_PLL_CAL_CFG11);
		REG_W(0x1F, hdmi_phy_base + HDMI_PHY_PD_CTRL0);
		udelay(50);

		REG_W(0x0F, hdmi_phy_pll_base + HDMI_UNI_PLL_GLB_CFG);
		REG_W(0x00, hdmi_phy_base + HDMI_PHY_PD_CTRL1);
		REG_W(0x10, hdmi_phy_base + HDMI_PHY_ANA_CFG2);
		REG_W(0xDB, hdmi_phy_base + HDMI_PHY_ANA_CFG0);
		REG_W(0x43, hdmi_phy_base + HDMI_PHY_ANA_CFG1);
		REG_W(0x02, hdmi_phy_base + HDMI_PHY_ANA_CFG2);
		REG_W(0x00, hdmi_phy_base + HDMI_PHY_ANA_CFG3);
		REG_W(0x04, hdmi_phy_pll_base + HDMI_UNI_PLL_VREG_CFG);
		REG_W(0xD0, hdmi_phy_base + HDMI_PHY_DCC_CFG0);
		REG_W(0x1A, hdmi_phy_base + HDMI_PHY_DCC_CFG1);
		REG_W(0x00, hdmi_phy_base + HDMI_PHY_TXCAL_CFG0);
		REG_W(0x00, hdmi_phy_base + HDMI_PHY_TXCAL_CFG1);
		REG_W(0x02, hdmi_phy_base + HDMI_PHY_TXCAL_CFG2);
		REG_W(0x05, hdmi_phy_base + HDMI_PHY_TXCAL_CFG3);
		udelay(200);
	break;

	case 27000000:
		/* 576p50/576i50 case */
		REG_W(0x81, hdmi_phy_base + HDMI_PHY_GLB_CFG);
		REG_W(0x01, hdmi_phy_pll_base + HDMI_UNI_PLL_GLB_CFG);
		REG_W(0x01, hdmi_phy_pll_base + HDMI_UNI_PLL_REFCLK_CFG);
		REG_W(0x19, hdmi_phy_pll_base + HDMI_UNI_PLL_VCOLPF_CFG);
		REG_W(0X0E, hdmi_phy_pll_base + HDMI_UNI_PLL_LPFR_CFG);
		REG_W(0x20, hdmi_phy_pll_base + HDMI_UNI_PLL_LPFC1_CFG);
		REG_W(0X0D, hdmi_phy_pll_base + HDMI_UNI_PLL_LPFC2_CFG);
		REG_W(0x00, hdmi_phy_pll_base + HDMI_UNI_PLL_SDM_CFG0);
		REG_W(0x54, hdmi_phy_pll_base + HDMI_UNI_PLL_SDM_CFG1);
		REG_W(0x00, hdmi_phy_pll_base + HDMI_UNI_PLL_SDM_CFG2);
		REG_W(0x18, hdmi_phy_pll_base + HDMI_UNI_PLL_SDM_CFG3);
		REG_W(0x00, hdmi_phy_pll_base + HDMI_UNI_PLL_SDM_CFG4);
		REG_W(0x10, hdmi_phy_pll_base + HDMI_UNI_PLL_LKDET_CFG0);
		REG_W(0X1A, hdmi_phy_pll_base + HDMI_UNI_PLL_LKDET_CFG1);
		REG_W(0x05, hdmi_phy_pll_base + HDMI_UNI_PLL_LKDET_CFG2);
		REG_W(0x03, hdmi_phy_pll_base + HDMI_UNI_PLL_POSTDIV1_CFG);
		REG_W(0x00, hdmi_phy_pll_base + HDMI_UNI_PLL_POSTDIV2_CFG);
		REG_W(0x00, hdmi_phy_pll_base + HDMI_UNI_PLL_POSTDIV3_CFG);
		REG_W(0x01, hdmi_phy_pll_base + HDMI_UNI_PLL_CAL_CFG2);
		REG_W(0x60, hdmi_phy_pll_base + HDMI_UNI_PLL_CAL_CFG8);
		REG_W(0x00, hdmi_phy_pll_base + HDMI_UNI_PLL_CAL_CFG9);
		REG_W(0x2a, hdmi_phy_pll_base + HDMI_UNI_PLL_CAL_CFG10);
		REG_W(0x03, hdmi_phy_pll_base + HDMI_UNI_PLL_CAL_CFG11);
		REG_W(0X1F, hdmi_phy_base + HDMI_PHY_PD_CTRL0);
		udelay(50);

		REG_W(0X0F, hdmi_phy_pll_base + HDMI_UNI_PLL_GLB_CFG);
		REG_W(0x00, hdmi_phy_base + HDMI_PHY_PD_CTRL1);
		REG_W(0x10, hdmi_phy_base + HDMI_PHY_ANA_CFG2);
		REG_W(0XDB, hdmi_phy_base + HDMI_PHY_ANA_CFG0);
		REG_W(0x43, hdmi_phy_base + HDMI_PHY_ANA_CFG1);
		REG_W(0x02, hdmi_phy_base + HDMI_PHY_ANA_CFG2);
		REG_W(0x00, hdmi_phy_base + HDMI_PHY_ANA_CFG3);
		REG_W(0x04, hdmi_phy_pll_base + HDMI_UNI_PLL_VREG_CFG);
		REG_W(0XD0, hdmi_phy_base + HDMI_PHY_DCC_CFG0);
		REG_W(0X1A, hdmi_phy_base + HDMI_PHY_DCC_CFG1);
		REG_W(0x00, hdmi_phy_base + HDMI_PHY_TXCAL_CFG0);
		REG_W(0x00, hdmi_phy_base + HDMI_PHY_TXCAL_CFG1);
		REG_W(0x02, hdmi_phy_base + HDMI_PHY_TXCAL_CFG2);
		REG_W(0x05, hdmi_phy_base + HDMI_PHY_TXCAL_CFG3);
		udelay(200);
	break;

	case 27030000:
		/* 480p60/480i60 case */
		REG_W(0x81, hdmi_phy_base + HDMI_PHY_GLB_CFG);
		REG_W(0x01, hdmi_phy_pll_base + HDMI_UNI_PLL_GLB_CFG);
		REG_W(0x01, hdmi_phy_pll_base + HDMI_UNI_PLL_REFCLK_CFG);
		REG_W(0x19, hdmi_phy_pll_base + HDMI_UNI_PLL_VCOLPF_CFG);
		REG_W(0x0E, hdmi_phy_pll_base + HDMI_UNI_PLL_LPFR_CFG);
		REG_W(0x20, hdmi_phy_pll_base + HDMI_UNI_PLL_LPFC1_CFG);
		REG_W(0x0D, hdmi_phy_pll_base + HDMI_UNI_PLL_LPFC2_CFG);
		REG_W(0x00, hdmi_phy_pll_base + HDMI_UNI_PLL_SDM_CFG0);
		REG_W(0x54, hdmi_phy_pll_base + HDMI_UNI_PLL_SDM_CFG1);
		REG_W(0x66, hdmi_phy_pll_base + HDMI_UNI_PLL_SDM_CFG2);
		REG_W(0x1D, hdmi_phy_pll_base + HDMI_UNI_PLL_SDM_CFG3);
		REG_W(0x00, hdmi_phy_pll_base + HDMI_UNI_PLL_SDM_CFG4);
		REG_W(0x10, hdmi_phy_pll_base + HDMI_UNI_PLL_LKDET_CFG0);
		REG_W(0x1A, hdmi_phy_pll_base + HDMI_UNI_PLL_LKDET_CFG1);
		REG_W(0x05, hdmi_phy_pll_base + HDMI_UNI_PLL_LKDET_CFG2);
		REG_W(0x03, hdmi_phy_pll_base + HDMI_UNI_PLL_POSTDIV1_CFG);
		REG_W(0x00, hdmi_phy_pll_base + HDMI_UNI_PLL_POSTDIV2_CFG);
		REG_W(0x00, hdmi_phy_pll_base + HDMI_UNI_PLL_POSTDIV3_CFG);
		REG_W(0x01, hdmi_phy_pll_base + HDMI_UNI_PLL_CAL_CFG2);
		REG_W(0x60, hdmi_phy_pll_base + HDMI_UNI_PLL_CAL_CFG8);
		REG_W(0x00, hdmi_phy_pll_base + HDMI_UNI_PLL_CAL_CFG9);
		REG_W(0x2A, hdmi_phy_pll_base + HDMI_UNI_PLL_CAL_CFG10);
		REG_W(0x03, hdmi_phy_pll_base + HDMI_UNI_PLL_CAL_CFG11);
		REG_W(0x1F, hdmi_phy_base + HDMI_PHY_PD_CTRL0);
		udelay(50);

		REG_W(0x0F, hdmi_phy_pll_base + HDMI_UNI_PLL_GLB_CFG);
		REG_W(0x00, hdmi_phy_base + HDMI_PHY_PD_CTRL1);
		REG_W(0x10, hdmi_phy_base + HDMI_PHY_ANA_CFG2);
		REG_W(0xDB, hdmi_phy_base + HDMI_PHY_ANA_CFG0);
		REG_W(0x43, hdmi_phy_base + HDMI_PHY_ANA_CFG1);
		REG_W(0x02, hdmi_phy_base + HDMI_PHY_ANA_CFG2);
		REG_W(0x00, hdmi_phy_base + HDMI_PHY_ANA_CFG3);
		REG_W(0x04, hdmi_phy_pll_base + HDMI_UNI_PLL_VREG_CFG);
		REG_W(0xD0, hdmi_phy_base + HDMI_PHY_DCC_CFG0);
		REG_W(0x1A, hdmi_phy_base + HDMI_PHY_DCC_CFG1);
		REG_W(0x00, hdmi_phy_base + HDMI_PHY_TXCAL_CFG0);
		REG_W(0x00, hdmi_phy_base + HDMI_PHY_TXCAL_CFG1);
		REG_W(0x02, hdmi_phy_base + HDMI_PHY_TXCAL_CFG2);
		REG_W(0x05, hdmi_phy_base + HDMI_PHY_TXCAL_CFG3);
		udelay(200);
	break;
	case 65000000:
		REG_W(0x81, hdmi_phy_base + HDMI_PHY_GLB_CFG);
		REG_W(0x01, hdmi_phy_pll_base + HDMI_UNI_PLL_GLB_CFG);
		REG_W(0x01, hdmi_phy_pll_base + HDMI_UNI_PLL_REFCLK_CFG);
		REG_W(0x19, hdmi_phy_pll_base + HDMI_UNI_PLL_VCOLPF_CFG);
		REG_W(0x0E, hdmi_phy_pll_base + HDMI_UNI_PLL_LPFR_CFG);
		REG_W(0x20, hdmi_phy_pll_base + HDMI_UNI_PLL_LPFC1_CFG);
		REG_W(0x0D, hdmi_phy_pll_base + HDMI_UNI_PLL_LPFC2_CFG);
		REG_W(0x00, hdmi_phy_pll_base + HDMI_UNI_PLL_SDM_CFG0);
		REG_W(0x4F, hdmi_phy_pll_base + HDMI_UNI_PLL_SDM_CFG1);
		REG_W(0x55, hdmi_phy_pll_base + HDMI_UNI_PLL_SDM_CFG2);
		REG_W(0xED, hdmi_phy_pll_base + HDMI_UNI_PLL_SDM_CFG3);
		REG_W(0x00, hdmi_phy_pll_base + HDMI_UNI_PLL_SDM_CFG4);
		REG_W(0x10, hdmi_phy_pll_base + HDMI_UNI_PLL_LKDET_CFG0);
		REG_W(0x1A, hdmi_phy_pll_base + HDMI_UNI_PLL_LKDET_CFG1);
		REG_W(0x05, hdmi_phy_pll_base + HDMI_UNI_PLL_LKDET_CFG2);
		REG_W(0x01, hdmi_phy_pll_base + HDMI_UNI_PLL_POSTDIV1_CFG);
		REG_W(0x00, hdmi_phy_pll_base + HDMI_UNI_PLL_POSTDIV2_CFG);
		REG_W(0x00, hdmi_phy_pll_base + HDMI_UNI_PLL_POSTDIV3_CFG);
		REG_W(0x01, hdmi_phy_pll_base + HDMI_UNI_PLL_CAL_CFG2);
		REG_W(0x60, hdmi_phy_pll_base + HDMI_UNI_PLL_CAL_CFG8);
		REG_W(0x00, hdmi_phy_pll_base + HDMI_UNI_PLL_CAL_CFG9);
		REG_W(0x8A, hdmi_phy_pll_base + HDMI_UNI_PLL_CAL_CFG10);
		REG_W(0x02, hdmi_phy_pll_base + HDMI_UNI_PLL_CAL_CFG11);
		REG_W(0x1F, hdmi_phy_base + HDMI_PHY_PD_CTRL0);
		udelay(50);

		REG_W(0x0F, hdmi_phy_pll_base + HDMI_UNI_PLL_GLB_CFG);
		REG_W(0x00, hdmi_phy_base + HDMI_PHY_PD_CTRL1);
		REG_W(0x10, hdmi_phy_base + HDMI_PHY_ANA_CFG2);
		REG_W(0xDB, hdmi_phy_base + HDMI_PHY_ANA_CFG0);
		REG_W(0x43, hdmi_phy_base + HDMI_PHY_ANA_CFG1);
		REG_W(0x02, hdmi_phy_base + HDMI_PHY_ANA_CFG2);
		REG_W(0x00, hdmi_phy_base + HDMI_PHY_ANA_CFG3);
		REG_W(0x04, hdmi_phy_pll_base + HDMI_UNI_PLL_VREG_CFG);
		REG_W(0xD0, hdmi_phy_base + HDMI_PHY_DCC_CFG0);
		REG_W(0x1A, hdmi_phy_base + HDMI_PHY_DCC_CFG1);
		REG_W(0x00, hdmi_phy_base + HDMI_PHY_TXCAL_CFG0);
		REG_W(0x00, hdmi_phy_base + HDMI_PHY_TXCAL_CFG1);
		REG_W(0x02, hdmi_phy_base + HDMI_PHY_TXCAL_CFG2);
		REG_W(0x05, hdmi_phy_base + HDMI_PHY_TXCAL_CFG3);
		udelay(200);
	break;
	case 74250000:
		/*
		 * 720p60/720p50/1080i60/1080i50
		 * 1080p24/1080p30/1080p25 case
		 */
		REG_W(0x81, hdmi_phy_base + HDMI_PHY_GLB_CFG);
		REG_W(0x01, hdmi_phy_pll_base + HDMI_UNI_PLL_GLB_CFG);
		REG_W(0x01, hdmi_phy_pll_base + HDMI_UNI_PLL_REFCLK_CFG);
		REG_W(0x19, hdmi_phy_pll_base + HDMI_UNI_PLL_VCOLPF_CFG);
		REG_W(0x0E, hdmi_phy_pll_base + HDMI_UNI_PLL_LPFR_CFG);
		REG_W(0x20, hdmi_phy_pll_base + HDMI_UNI_PLL_LPFC1_CFG);
		REG_W(0x0D, hdmi_phy_pll_base + HDMI_UNI_PLL_LPFC2_CFG);
		REG_W(0x00, hdmi_phy_pll_base + HDMI_UNI_PLL_SDM_CFG0);
		REG_W(0x52, hdmi_phy_pll_base + HDMI_UNI_PLL_SDM_CFG1);
		REG_W(0x00, hdmi_phy_pll_base + HDMI_UNI_PLL_SDM_CFG2);
		REG_W(0x56, hdmi_phy_pll_base + HDMI_UNI_PLL_SDM_CFG3);
		REG_W(0x00, hdmi_phy_pll_base + HDMI_UNI_PLL_SDM_CFG4);
		REG_W(0x10, hdmi_phy_pll_base + HDMI_UNI_PLL_LKDET_CFG0);
		REG_W(0x1A, hdmi_phy_pll_base + HDMI_UNI_PLL_LKDET_CFG1);
		REG_W(0x05, hdmi_phy_pll_base + HDMI_UNI_PLL_LKDET_CFG2);
		REG_W(0x01, hdmi_phy_pll_base + HDMI_UNI_PLL_POSTDIV1_CFG);
		REG_W(0x00, hdmi_phy_pll_base + HDMI_UNI_PLL_POSTDIV2_CFG);
		REG_W(0x00, hdmi_phy_pll_base + HDMI_UNI_PLL_POSTDIV3_CFG);
		REG_W(0x01, hdmi_phy_pll_base + HDMI_UNI_PLL_CAL_CFG2);
		REG_W(0x60, hdmi_phy_pll_base + HDMI_UNI_PLL_CAL_CFG8);
		REG_W(0x00, hdmi_phy_pll_base + HDMI_UNI_PLL_CAL_CFG9);
		REG_W(0xE6, hdmi_phy_pll_base + HDMI_UNI_PLL_CAL_CFG10);
		REG_W(0x02, hdmi_phy_pll_base + HDMI_UNI_PLL_CAL_CFG11);
		REG_W(0x1F, hdmi_phy_base + HDMI_PHY_PD_CTRL0);
		udelay(50);

		REG_W(0x0F, hdmi_phy_pll_base + HDMI_UNI_PLL_GLB_CFG);
		REG_W(0x00, hdmi_phy_base + HDMI_PHY_PD_CTRL1);
		REG_W(0x10, hdmi_phy_base + HDMI_PHY_ANA_CFG2);
		REG_W(0xDB, hdmi_phy_base + HDMI_PHY_ANA_CFG0);
		REG_W(0x43, hdmi_phy_base + HDMI_PHY_ANA_CFG1);
		REG_W(0x02, hdmi_phy_base + HDMI_PHY_ANA_CFG2);
		REG_W(0x00, hdmi_phy_base + HDMI_PHY_ANA_CFG3);
		REG_W(0x04, hdmi_phy_pll_base + HDMI_UNI_PLL_VREG_CFG);
		REG_W(0xD0, hdmi_phy_base + HDMI_PHY_DCC_CFG0);
		REG_W(0x1A, hdmi_phy_base + HDMI_PHY_DCC_CFG1);
		REG_W(0x00, hdmi_phy_base + HDMI_PHY_TXCAL_CFG0);
		REG_W(0x00, hdmi_phy_base + HDMI_PHY_TXCAL_CFG1);
		REG_W(0x02, hdmi_phy_base + HDMI_PHY_TXCAL_CFG2);
		REG_W(0x05, hdmi_phy_base + HDMI_PHY_TXCAL_CFG3);
		udelay(200);
	break;

	case 108000000:
		REG_W(0x81, hdmi_phy_base + HDMI_PHY_GLB_CFG);
		REG_W(0x01, hdmi_phy_pll_base + HDMI_UNI_PLL_GLB_CFG);
		REG_W(0x01, hdmi_phy_pll_base + HDMI_UNI_PLL_REFCLK_CFG);
		REG_W(0x19, hdmi_phy_pll_base + HDMI_UNI_PLL_VCOLPF_CFG);
		REG_W(0x0E, hdmi_phy_pll_base + HDMI_UNI_PLL_LPFR_CFG);
		REG_W(0x20, hdmi_phy_pll_base + HDMI_UNI_PLL_LPFC1_CFG);
		REG_W(0x0D, hdmi_phy_pll_base + HDMI_UNI_PLL_LPFC2_CFG);
		REG_W(0x00, hdmi_phy_pll_base + HDMI_UNI_PLL_SDM_CFG0);
		REG_W(0x5B, hdmi_phy_pll_base + HDMI_UNI_PLL_SDM_CFG1);
		REG_W(0x00, hdmi_phy_pll_base + HDMI_UNI_PLL_SDM_CFG2);
		REG_W(0x20, hdmi_phy_pll_base + HDMI_UNI_PLL_SDM_CFG3);
		REG_W(0x00, hdmi_phy_pll_base + HDMI_UNI_PLL_SDM_CFG4);
		REG_W(0x10, hdmi_phy_pll_base + HDMI_UNI_PLL_LKDET_CFG0);
		REG_W(0x1A, hdmi_phy_pll_base + HDMI_UNI_PLL_LKDET_CFG1);
		REG_W(0x05, hdmi_phy_pll_base + HDMI_UNI_PLL_LKDET_CFG2);
		REG_W(0x01, hdmi_phy_pll_base + HDMI_UNI_PLL_POSTDIV1_CFG);
		REG_W(0x00, hdmi_phy_pll_base + HDMI_UNI_PLL_POSTDIV2_CFG);
		REG_W(0x00, hdmi_phy_pll_base + HDMI_UNI_PLL_POSTDIV3_CFG);
		REG_W(0x01, hdmi_phy_pll_base + HDMI_UNI_PLL_CAL_CFG2);
		REG_W(0x60, hdmi_phy_pll_base + HDMI_UNI_PLL_CAL_CFG8);
		REG_W(0x00, hdmi_phy_pll_base + HDMI_UNI_PLL_CAL_CFG9);
		REG_W(0x38, hdmi_phy_pll_base + HDMI_UNI_PLL_CAL_CFG10);
		REG_W(0x04, hdmi_phy_pll_base + HDMI_UNI_PLL_CAL_CFG11);
		REG_W(0x1F, hdmi_phy_base + HDMI_PHY_PD_CTRL0);
		udelay(50);

		REG_W(0x0F, hdmi_phy_pll_base + HDMI_UNI_PLL_GLB_CFG);
		REG_W(0x00, hdmi_phy_base + HDMI_PHY_PD_CTRL1);
		REG_W(0x10, hdmi_phy_base + HDMI_PHY_ANA_CFG2);
		REG_W(0xDB, hdmi_phy_base + HDMI_PHY_ANA_CFG0);
		REG_W(0x43, hdmi_phy_base + HDMI_PHY_ANA_CFG1);
		REG_W(0x02, hdmi_phy_base + HDMI_PHY_ANA_CFG2);
		REG_W(0x00, hdmi_phy_base + HDMI_PHY_ANA_CFG3);
		REG_W(0x04, hdmi_phy_pll_base + HDMI_UNI_PLL_VREG_CFG);
		REG_W(0xD0, hdmi_phy_base + HDMI_PHY_DCC_CFG0);
		REG_W(0x1A, hdmi_phy_base + HDMI_PHY_DCC_CFG1);
		REG_W(0x00, hdmi_phy_base + HDMI_PHY_TXCAL_CFG0);
		REG_W(0x00, hdmi_phy_base + HDMI_PHY_TXCAL_CFG1);
		REG_W(0x02, hdmi_phy_base + HDMI_PHY_TXCAL_CFG2);
		REG_W(0x05, hdmi_phy_base + HDMI_PHY_TXCAL_CFG3);
		udelay(200);
	break;

	case 148500000:
		REG_W(0x81, hdmi_phy_base + HDMI_PHY_GLB_CFG);
		REG_W(0x01, hdmi_phy_pll_base + HDMI_UNI_PLL_GLB_CFG);
		REG_W(0x01, hdmi_phy_pll_base + HDMI_UNI_PLL_REFCLK_CFG);
		REG_W(0x19, hdmi_phy_pll_base + HDMI_UNI_PLL_VCOLPF_CFG);
		REG_W(0x0E, hdmi_phy_pll_base + HDMI_UNI_PLL_LPFR_CFG);
		REG_W(0x20, hdmi_phy_pll_base + HDMI_UNI_PLL_LPFC1_CFG);
		REG_W(0x0D, hdmi_phy_pll_base + HDMI_UNI_PLL_LPFC2_CFG);
		REG_W(0x00, hdmi_phy_pll_base + HDMI_UNI_PLL_SDM_CFG0);
		REG_W(0x52, hdmi_phy_pll_base + HDMI_UNI_PLL_SDM_CFG1);
		REG_W(0x00, hdmi_phy_pll_base + HDMI_UNI_PLL_SDM_CFG2);
		REG_W(0x56, hdmi_phy_pll_base + HDMI_UNI_PLL_SDM_CFG3);
		REG_W(0x00, hdmi_phy_pll_base + HDMI_UNI_PLL_SDM_CFG4);
		REG_W(0x10, hdmi_phy_pll_base + HDMI_UNI_PLL_LKDET_CFG0);
		REG_W(0x1A, hdmi_phy_pll_base + HDMI_UNI_PLL_LKDET_CFG1);
		REG_W(0x05, hdmi_phy_pll_base + HDMI_UNI_PLL_LKDET_CFG2);
		REG_W(0x00, hdmi_phy_pll_base + HDMI_UNI_PLL_POSTDIV1_CFG);
		REG_W(0x00, hdmi_phy_pll_base + HDMI_UNI_PLL_POSTDIV2_CFG);
		REG_W(0x00, hdmi_phy_pll_base + HDMI_UNI_PLL_POSTDIV3_CFG);
		REG_W(0x01, hdmi_phy_pll_base + HDMI_UNI_PLL_CAL_CFG2);
		REG_W(0x60, hdmi_phy_pll_base + HDMI_UNI_PLL_CAL_CFG8);
		REG_W(0x00, hdmi_phy_pll_base + HDMI_UNI_PLL_CAL_CFG9);
		REG_W(0xE6, hdmi_phy_pll_base + HDMI_UNI_PLL_CAL_CFG10);
		REG_W(0x02, hdmi_phy_pll_base + HDMI_UNI_PLL_CAL_CFG11);
		REG_W(0x1F, hdmi_phy_base + HDMI_PHY_PD_CTRL0);
		udelay(50);

		REG_W(0x0F, hdmi_phy_pll_base + HDMI_UNI_PLL_GLB_CFG);
		REG_W(0x00, hdmi_phy_base + HDMI_PHY_PD_CTRL1);
		REG_W(0x10, hdmi_phy_base + HDMI_PHY_ANA_CFG2);
		REG_W(0xDB, hdmi_phy_base + HDMI_PHY_ANA_CFG0);
		REG_W(0x43, hdmi_phy_base + HDMI_PHY_ANA_CFG1);
		REG_W(0x02, hdmi_phy_base + HDMI_PHY_ANA_CFG2);
		REG_W(0x00, hdmi_phy_base + HDMI_PHY_ANA_CFG3);
		REG_W(0x04, hdmi_phy_pll_base + HDMI_UNI_PLL_VREG_CFG);
		REG_W(0xD0, hdmi_phy_base + HDMI_PHY_DCC_CFG0);
		REG_W(0x1A, hdmi_phy_base + HDMI_PHY_DCC_CFG1);
		REG_W(0x00, hdmi_phy_base + HDMI_PHY_TXCAL_CFG0);
		REG_W(0x00, hdmi_phy_base + HDMI_PHY_TXCAL_CFG1);
		REG_W(0x02, hdmi_phy_base + HDMI_PHY_TXCAL_CFG2);
		REG_W(0x05, hdmi_phy_base + HDMI_PHY_TXCAL_CFG3);
		udelay(200);
	break;

	case 268500000:
		REG_W(0x81, hdmi_phy_base + HDMI_PHY_GLB_CFG);
		REG_W(0x01, hdmi_phy_pll_base + HDMI_UNI_PLL_GLB_CFG);
		REG_W(0x01, hdmi_phy_pll_base + HDMI_UNI_PLL_REFCLK_CFG);
		REG_W(0x19, hdmi_phy_pll_base + HDMI_UNI_PLL_VCOLPF_CFG);
		REG_W(0x0E, hdmi_phy_pll_base + HDMI_UNI_PLL_LPFR_CFG);
		REG_W(0x20, hdmi_phy_pll_base + HDMI_UNI_PLL_LPFC1_CFG);
		REG_W(0x0D, hdmi_phy_pll_base + HDMI_UNI_PLL_LPFC2_CFG);
		REG_W(0x36, hdmi_phy_pll_base + HDMI_UNI_PLL_SDM_CFG0);
		REG_W(0x61, hdmi_phy_pll_base + HDMI_UNI_PLL_SDM_CFG1);
		REG_W(0x01, hdmi_phy_pll_base + HDMI_UNI_PLL_SDM_CFG2);
		REG_W(0xF6, hdmi_phy_pll_base + HDMI_UNI_PLL_SDM_CFG3);
		REG_W(0x00, hdmi_phy_pll_base + HDMI_UNI_PLL_SDM_CFG4);
		REG_W(0x10, hdmi_phy_pll_base + HDMI_UNI_PLL_LKDET_CFG0);
		REG_W(0x1A, hdmi_phy_pll_base + HDMI_UNI_PLL_LKDET_CFG1);
		REG_W(0x05, hdmi_phy_pll_base + HDMI_UNI_PLL_LKDET_CFG2);
		REG_W(0x00, hdmi_phy_pll_base + HDMI_UNI_PLL_POSTDIV1_CFG);
		REG_W(0x00, hdmi_phy_pll_base + HDMI_UNI_PLL_POSTDIV2_CFG);
		REG_W(0x00, hdmi_phy_pll_base + HDMI_UNI_PLL_POSTDIV3_CFG);
		REG_W(0x01, hdmi_phy_pll_base + HDMI_UNI_PLL_CAL_CFG2);
		REG_W(0x60, hdmi_phy_pll_base + HDMI_UNI_PLL_CAL_CFG8);
		REG_W(0x00, hdmi_phy_pll_base + HDMI_UNI_PLL_CAL_CFG9);
		REG_W(0x3E, hdmi_phy_pll_base + HDMI_UNI_PLL_CAL_CFG10);
		REG_W(0x05, hdmi_phy_pll_base + HDMI_UNI_PLL_CAL_CFG11);
		REG_W(0x1F, hdmi_phy_base + HDMI_PHY_PD_CTRL0);
		udelay(50);

		REG_W(0x0F, hdmi_phy_pll_base + HDMI_UNI_PLL_GLB_CFG);
		REG_W(0x00, hdmi_phy_base + HDMI_PHY_PD_CTRL1);
		REG_W(0x10, hdmi_phy_base + HDMI_PHY_ANA_CFG2);
		REG_W(0xDB, hdmi_phy_base + HDMI_PHY_ANA_CFG0);
		REG_W(0x43, hdmi_phy_base + HDMI_PHY_ANA_CFG1);
		REG_W(0x05, hdmi_phy_base + HDMI_PHY_ANA_CFG2);
		REG_W(0x00, hdmi_phy_base + HDMI_PHY_ANA_CFG3);
		REG_W(0x04, hdmi_phy_pll_base + HDMI_UNI_PLL_VREG_CFG);
		REG_W(0xD0, hdmi_phy_base + HDMI_PHY_DCC_CFG0);
		REG_W(0x1A, hdmi_phy_base + HDMI_PHY_DCC_CFG1);
		REG_W(0x00, hdmi_phy_base + HDMI_PHY_TXCAL_CFG0);
		REG_W(0x00, hdmi_phy_base + HDMI_PHY_TXCAL_CFG1);
		REG_W(0x11, hdmi_phy_base + HDMI_PHY_TXCAL_CFG2);
		REG_W(0x05, hdmi_phy_base + HDMI_PHY_TXCAL_CFG3);
		udelay(200);
	break;

	case 297000000:
		REG_W(0x81, hdmi_phy_base + HDMI_PHY_GLB_CFG);
		REG_W(0x01, hdmi_phy_pll_base + HDMI_UNI_PLL_GLB_CFG);
		REG_W(0x01, hdmi_phy_pll_base + HDMI_UNI_PLL_REFCLK_CFG);
		REG_W(0x19, hdmi_phy_pll_base + HDMI_UNI_PLL_VCOLPF_CFG);
		REG_W(0x0E, hdmi_phy_pll_base + HDMI_UNI_PLL_LPFR_CFG);
		REG_W(0x20, hdmi_phy_pll_base + HDMI_UNI_PLL_LPFC1_CFG);
		REG_W(0x0D, hdmi_phy_pll_base + HDMI_UNI_PLL_LPFC2_CFG);
		REG_W(0x00, hdmi_phy_pll_base + HDMI_UNI_PLL_SDM_CFG0);
		REG_W(0x65, hdmi_phy_pll_base + HDMI_UNI_PLL_SDM_CFG1);
		REG_W(0x00, hdmi_phy_pll_base + HDMI_UNI_PLL_SDM_CFG2);
		REG_W(0xAC, hdmi_phy_pll_base + HDMI_UNI_PLL_SDM_CFG3);
		REG_W(0x00, hdmi_phy_pll_base + HDMI_UNI_PLL_SDM_CFG4);
		REG_W(0x10, hdmi_phy_pll_base + HDMI_UNI_PLL_LKDET_CFG0);
		REG_W(0x1A, hdmi_phy_pll_base + HDMI_UNI_PLL_LKDET_CFG1);
		REG_W(0x05, hdmi_phy_pll_base + HDMI_UNI_PLL_LKDET_CFG2);
		REG_W(0x00, hdmi_phy_pll_base + HDMI_UNI_PLL_POSTDIV1_CFG);
		REG_W(0x00, hdmi_phy_pll_base + HDMI_UNI_PLL_POSTDIV2_CFG);
		REG_W(0x00, hdmi_phy_pll_base + HDMI_UNI_PLL_POSTDIV3_CFG);
		REG_W(0x01, hdmi_phy_pll_base + HDMI_UNI_PLL_CAL_CFG2);
		REG_W(0x60, hdmi_phy_pll_base + HDMI_UNI_PLL_CAL_CFG8);
		REG_W(0x00, hdmi_phy_pll_base + HDMI_UNI_PLL_CAL_CFG9);
		REG_W(0xCD, hdmi_phy_pll_base + HDMI_UNI_PLL_CAL_CFG10);
		REG_W(0x05, hdmi_phy_pll_base + HDMI_UNI_PLL_CAL_CFG11);
		REG_W(0x1F, hdmi_phy_base + HDMI_PHY_PD_CTRL0);
		udelay(50);

		REG_W(0x0F, hdmi_phy_pll_base + HDMI_UNI_PLL_GLB_CFG);
		REG_W(0x00, hdmi_phy_base + HDMI_PHY_PD_CTRL1);
		REG_W(0x10, hdmi_phy_base + HDMI_PHY_ANA_CFG2);
		REG_W(0xDB, hdmi_phy_base + HDMI_PHY_ANA_CFG0);
		REG_W(0x43, hdmi_phy_base + HDMI_PHY_ANA_CFG1);
		REG_W(0x06, hdmi_phy_base + HDMI_PHY_ANA_CFG2);
		REG_W(0x03, hdmi_phy_base + HDMI_PHY_ANA_CFG3);
		REG_W(0x04, hdmi_phy_pll_base + HDMI_UNI_PLL_VREG_CFG);
		REG_W(0xD0, hdmi_phy_base + HDMI_PHY_DCC_CFG0);
		REG_W(0x1A, hdmi_phy_base + HDMI_PHY_DCC_CFG1);
		REG_W(0x00, hdmi_phy_base + HDMI_PHY_TXCAL_CFG0);
		REG_W(0x00, hdmi_phy_base + HDMI_PHY_TXCAL_CFG1);
		REG_W(0x02, hdmi_phy_base + HDMI_PHY_TXCAL_CFG2);
		REG_W(0x05, hdmi_phy_base + HDMI_PHY_TXCAL_CFG3);
		udelay(200);
	break;

	default:
		pr_err("%s: not supported rate=%ld\n", __func__, rate);
	}

	/* Make sure writes complete before disabling iface clock */
	mb();

	clk_disable(mdss_ahb_clk);

	if (set_power_dwn)
		hdmi_pll_enable();

	return 0;
} /* hdmi_pll_set_rate */

/* Auto PLL calibaration */
int mdss_ahb_clk_enable(int enable)
{
	int rc = 0;

	/* todo: Ideally, we should enable/disable GDSC whenever we are
	 * attempting to enable/disable MDSS AHB clock.
	 * For now, just return error if  GDSC is not enabled.
	 */
	pr_debug("#### mdss_ahb_clk_enable++\n");

	if (!mdss_gdsc_enabled())
		return -EPERM;

	if (enable)
		rc = clk_prepare_enable(mdss_ahb_clk);
	else
		clk_disable_unprepare(mdss_ahb_clk);

	pr_debug("#### mdss_ahb_clk_enable-- rc %d\n", rc);
	return rc;
}

int set_byte_mux_sel(struct mux_clk *clk, int sel)
{
	pr_debug("%s: byte mux set to %s mode\n", __func__,
		sel ? "indirect" : "direct");
	DSS_REG_W(mdss_dsi_base, DSI_0_PHY_PLL_UNIPHY_PLL_VREG_CFG,
			(sel << 1));
	return 0;
}

int get_byte_mux_sel(struct mux_clk *clk)
{
	int mux_mode;

	if (mdss_ahb_clk_enable(1)) {
		pr_err("%s: Failed to enable mdss ahb clock\n", __func__);
		return 0;
	}

	mux_mode = DSS_REG_R(mdss_dsi_base, DSI_0_PHY_PLL_UNIPHY_PLL_VREG_CFG)
				& BIT(1);
	pr_err("%s: byte mux mode = %s", __func__,
		mux_mode ? "indirect" : "direct");

	mdss_ahb_clk_enable(0);
	return !!mux_mode;
}

static inline struct dsi_pll_vco_clk *to_vco_clk(struct clk *clk)
{
	return container_of(clk, struct dsi_pll_vco_clk, c);
}

/*
 * When the display is turned off, the display registers are wiped out.
 * Temporarily use the prepare ops to restore the register values.
 *
*/
int div_prepare(struct clk *c)
{
	struct div_clk *div = to_div_clk(c);
	/* Restore the divider's value */
	return div->ops->set_div(div, div->div);
}

int mux_prepare(struct clk *c)
{
	struct mux_clk *mux = to_mux_clk(c);
	int i, rc, sel = 0;

	rc = mdss_ahb_clk_enable(1);
	if (rc) {
		pr_err("%s: failed to enable mdss ahb clock. rc=%d\n",
			__func__, rc);
		return rc;
	}

	for (i = 0; i < mux->num_parents; i++)
		if (mux->parents[i].src == c->parent) {
			sel = mux->parents[i].sel;
			break;
		}

	if (i == mux->num_parents) {
		rc = -EINVAL;
		goto error;
	}

	/* Restore the mux source select value */
	rc = mux->ops->set_mux_sel(mux, sel);

error:
	mdss_ahb_clk_enable(0);
	return rc;
}

static int fixed_4div_set_div(struct div_clk *clk, int div)
{
	int rc = 0;

	rc = mdss_ahb_clk_enable(1);
	if (rc) {
		pr_err("%s: failed to enable mdss ahb clock. rc=%d\n",
			__func__, rc);
		return rc;
	}

	DSS_REG_W(mdss_dsi_base, DSI_0_PHY_PLL_UNIPHY_PLL_POSTDIV2_CFG,
			(div - 1));

	mdss_ahb_clk_enable(0);
	return 0;
}

static int fixed_4div_get_div(struct div_clk *clk)
{
	int div = 0;

	if (mdss_ahb_clk_enable(1)) {
		pr_err("%s: Failed to enable mdss ahb clock\n", __func__);
		return 1;
	}
	div = DSS_REG_R(mdss_dsi_base, DSI_0_PHY_PLL_UNIPHY_PLL_POSTDIV2_CFG);
	mdss_ahb_clk_enable(0);
	return div + 1;
}

static int digital_set_div(struct div_clk *clk, int div)
{
	int rc = 0;

	rc = mdss_ahb_clk_enable(1);
	if (rc) {
		pr_err("%s: failed to enable mdss ahb clock. rc=%d\n",
			__func__, rc);
		return rc;
	}

	DSS_REG_W(mdss_dsi_base, DSI_0_PHY_PLL_UNIPHY_PLL_POSTDIV3_CFG,
			(div - 1));

	mdss_ahb_clk_enable(0);
	return 0;
}

static int digital_get_div(struct div_clk *clk)
{
	int div = 0;

	if (mdss_ahb_clk_enable(1)) {
		pr_err("%s: Failed to enable mdss ahb clock\n", __func__);
		return 1;
	}
	div = DSS_REG_R(mdss_dsi_base, DSI_0_PHY_PLL_UNIPHY_PLL_POSTDIV3_CFG);
	mdss_ahb_clk_enable(0);
	return div + 1;
}

static int analog_set_div(struct div_clk *clk, int div)
{
	int rc = 0;

	rc = mdss_ahb_clk_enable(1);
	if (rc) {
		pr_err("%s: failed to enable mdss ahb clock. rc=%d\n",
			__func__, rc);
		return rc;
	}

	DSS_REG_W(mdss_dsi_base, DSI_0_PHY_PLL_UNIPHY_PLL_POSTDIV1_CFG,
			div - 1);

	mdss_ahb_clk_enable(0);
	return 0;
}

static int analog_get_div(struct div_clk *clk)
{
	int div = 0;

	if (mdss_ahb_clk_enable(1)) {
		pr_err("%s: Failed to enable mdss ahb clock\n", __func__);
		return 1;
	}
	div = DSS_REG_R(mdss_dsi_base,
		DSI_0_PHY_PLL_UNIPHY_PLL_POSTDIV1_CFG) + 1;
	mdss_ahb_clk_enable(0);
	return div;
}

static int dsi_pll_lock_status(void)
{
	u32 status;
	int pll_locked = 0;

	/* poll for PLL ready status */
	if (readl_poll_timeout_noirq((mdss_dsi_base +
			DSI_0_PHY_PLL_UNIPHY_PLL_STATUS),
			status,
			((status & BIT(0)) == 1),
			PLL_POLL_MAX_READS, PLL_POLL_TIMEOUT_US)) {
		pr_err("%s: DSI PLL status=%x failed to Lock\n",
				__func__, status);
		pll_locked = 0;
	} else {
		pll_locked = 1;
	}

	return pll_locked;
}

static void dsi_pll_software_reset(void)
{
	/*
	 * Add HW recommended delays after toggling the software
	 * reset bit off and back on.
	 */
	DSS_REG_W(mdss_dsi_base, DSI_0_PHY_PLL_UNIPHY_PLL_TEST_CFG, 0x01);
	udelay(1000);
	DSS_REG_W(mdss_dsi_base, DSI_0_PHY_PLL_UNIPHY_PLL_TEST_CFG, 0x00);
	udelay(1000);
}

static int dsi_pll_enable_seq_m(void)
{
	int i = 0;
	int pll_locked = 0;

	dsi_pll_software_reset();

	/*
	 * Add hardware recommended delays between register writes for
	 * the updates to take effect. These delays are necessary for the
	 * PLL to successfully lock
	 */
	DSS_REG_W(mdss_dsi_base, DSI_0_PHY_PLL_UNIPHY_PLL_GLB_CFG, 0x01);
	udelay(200);
	DSS_REG_W(mdss_dsi_base, DSI_0_PHY_PLL_UNIPHY_PLL_GLB_CFG, 0x05);
	udelay(200);
	DSS_REG_W(mdss_dsi_base, DSI_0_PHY_PLL_UNIPHY_PLL_GLB_CFG, 0x0f);
	udelay(1000);

	do {
		pll_locked = dsi_pll_lock_status();
		if (!pll_locked) {
			DSS_REG_W(mdss_dsi_base,
				DSI_0_PHY_PLL_UNIPHY_PLL_GLB_CFG, 0x07);
			udelay(1);
			DSS_REG_W(mdss_dsi_base,
				DSI_0_PHY_PLL_UNIPHY_PLL_GLB_CFG, 0x0f);
			udelay(1000);
			i++;
		}
	} while ((i < 3) && !pll_locked);

	if (pll_locked)
		pr_err("%s: PLL Locked at attempt #%d\n", __func__, i);
	else
		pr_err("%s: PLL failed to lock after %d attempt(s)\n",
			__func__, i);

	return pll_locked ? 0 : -EINVAL;
}

static int dsi_pll_enable_seq_d(void)
{
	int pll_locked = 0;

	dsi_pll_software_reset();

	/*
	 * Add hardware recommended delays between register writes for
	 * the updates to take effect. These delays are necessary for the
	 * PLL to successfully lock
	 */
	DSS_REG_W(mdss_dsi_base, DSI_0_PHY_PLL_UNIPHY_PLL_GLB_CFG, 0x01);
	udelay(1);
	DSS_REG_W(mdss_dsi_base, DSI_0_PHY_PLL_UNIPHY_PLL_GLB_CFG, 0x05);
	udelay(1);
	DSS_REG_W(mdss_dsi_base, DSI_0_PHY_PLL_UNIPHY_PLL_GLB_CFG, 0x07);
	udelay(1);
	DSS_REG_W(mdss_dsi_base, DSI_0_PHY_PLL_UNIPHY_PLL_GLB_CFG, 0x05);
	udelay(1);
	DSS_REG_W(mdss_dsi_base, DSI_0_PHY_PLL_UNIPHY_PLL_GLB_CFG, 0x07);
	udelay(1);
	DSS_REG_W(mdss_dsi_base, DSI_0_PHY_PLL_UNIPHY_PLL_GLB_CFG, 0x0f);
	udelay(1);

	pll_locked = dsi_pll_lock_status();
	pr_err("%s: PLL status = %s\n", __func__,
		pll_locked ? "Locked" : "Unlocked");

	return pll_locked ? 0 : -EINVAL;
}

static int dsi_pll_enable_seq_f1(void)
{
	int pll_locked = 0;

	dsi_pll_software_reset();

	/*
	 * Add hardware recommended delays between register writes for
	 * the updates to take effect. These delays are necessary for the
	 * PLL to successfully lock
	 */
	DSS_REG_W(mdss_dsi_base, DSI_0_PHY_PLL_UNIPHY_PLL_GLB_CFG, 0x01);
	udelay(200);
	DSS_REG_W(mdss_dsi_base, DSI_0_PHY_PLL_UNIPHY_PLL_GLB_CFG, 0x05);
	udelay(200);
	DSS_REG_W(mdss_dsi_base, DSI_0_PHY_PLL_UNIPHY_PLL_GLB_CFG, 0x0f);
	udelay(200);
	DSS_REG_W(mdss_dsi_base, DSI_0_PHY_PLL_UNIPHY_PLL_GLB_CFG, 0x0d);
	udelay(200);
	DSS_REG_W(mdss_dsi_base, DSI_0_PHY_PLL_UNIPHY_PLL_GLB_CFG, 0x0f);
	udelay(1000);

	pll_locked = dsi_pll_lock_status();
	pr_err("%s: PLL status = %s\n", __func__,
		pll_locked ? "Locked" : "Unlocked");

	return pll_locked ? 0 : -EINVAL;
}

static int dsi_pll_enable_seq_c(void)
{
	int pll_locked = 0;

	dsi_pll_software_reset();

	/*
	 * Add hardware recommended delays between register writes for
	 * the updates to take effect. These delays are necessary for the
	 * PLL to successfully lock
	 */
	DSS_REG_W(mdss_dsi_base, DSI_0_PHY_PLL_UNIPHY_PLL_GLB_CFG, 0x01);
	udelay(200);
	DSS_REG_W(mdss_dsi_base, DSI_0_PHY_PLL_UNIPHY_PLL_GLB_CFG, 0x05);
	udelay(200);
	DSS_REG_W(mdss_dsi_base, DSI_0_PHY_PLL_UNIPHY_PLL_GLB_CFG, 0x0f);
	udelay(1000);

	pll_locked = dsi_pll_lock_status();
	pr_err("%s: PLL status = %s\n", __func__,
		pll_locked ? "Locked" : "Unlocked");

	return pll_locked ? 0 : -EINVAL;
}

static int dsi_pll_enable_seq_e(void)
{
	int pll_locked = 0;

	dsi_pll_software_reset();

	/*
	 * Add hardware recommended delays between register writes for
	 * the updates to take effect. These delays are necessary for the
	 * PLL to successfully lock
	 */
	DSS_REG_W(mdss_dsi_base, DSI_0_PHY_PLL_UNIPHY_PLL_GLB_CFG, 0x01);
	udelay(200);
	DSS_REG_W(mdss_dsi_base, DSI_0_PHY_PLL_UNIPHY_PLL_GLB_CFG, 0x05);
	udelay(200);
	DSS_REG_W(mdss_dsi_base, DSI_0_PHY_PLL_UNIPHY_PLL_GLB_CFG, 0x0d);
	DSS_REG_W(mdss_dsi_base, DSI_0_PHY_PLL_UNIPHY_PLL_GLB_CFG, 0x0f);
	udelay(1000);

	pll_locked = dsi_pll_lock_status();
	pr_err("%s: PLL status = %s\n", __func__,
		pll_locked ? "Locked" : "Unlocked");

	return pll_locked ? 0 : -EINVAL;
}

static int dsi_pll_enable_seq_8974(void)
{
	int i, rc = 0;
	u32 status, max_reads, timeout_us;

	dsi_pll_software_reset();

	/*
	 * PLL power up sequence.
	 * Add necessary delays recommeded by hardware.
	 */
	DSS_REG_W(mdss_dsi_base, DSI_0_PHY_PLL_UNIPHY_PLL_GLB_CFG, 0x01);
	udelay(1000);
	DSS_REG_W(mdss_dsi_base, DSI_0_PHY_PLL_UNIPHY_PLL_GLB_CFG, 0x05);
	udelay(1000);
	DSS_REG_W(mdss_dsi_base, DSI_0_PHY_PLL_UNIPHY_PLL_GLB_CFG, 0x07);
	udelay(1000);
	DSS_REG_W(mdss_dsi_base, DSI_0_PHY_PLL_UNIPHY_PLL_GLB_CFG, 0x0f);
	udelay(1000);

	for (i = 0; i < 3; i++) {
		/* DSI Uniphy lock detect setting */
		DSS_REG_W(mdss_dsi_base, DSI_0_PHY_PLL_UNIPHY_PLL_LKDET_CFG2,
			0x04);
		udelay(100);
		DSS_REG_W(mdss_dsi_base, DSI_0_PHY_PLL_UNIPHY_PLL_LKDET_CFG2,
			0x05);
		udelay(500);
		/* poll for PLL ready status */
		max_reads = 5;
		timeout_us = 100;
		if (readl_poll_timeout_noirq((mdss_dsi_base +
				DSI_0_PHY_PLL_UNIPHY_PLL_STATUS),
				status,
				((status & 0x01) == 1),
				max_reads, timeout_us)) {
			pr_err("%s: DSI PLL status=%x failed to Lock\n",
			       __func__, status);
			pr_err("%s:Trying to power UP PLL again\n",
			       __func__);
		} else {
			break;
		}

		dsi_pll_software_reset();
		/*
		 * PLL power up sequence.
		 * Add necessary delays recommeded by hardware.
		 */
		DSS_REG_W(mdss_dsi_base, DSI_0_PHY_PLL_UNIPHY_PLL_GLB_CFG, 0x1);
		udelay(1000);
		DSS_REG_W(mdss_dsi_base, DSI_0_PHY_PLL_UNIPHY_PLL_GLB_CFG, 0x5);
		udelay(1000);
		DSS_REG_W(mdss_dsi_base, DSI_0_PHY_PLL_UNIPHY_PLL_GLB_CFG, 0x7);
		udelay(1000);
		DSS_REG_W(mdss_dsi_base, DSI_0_PHY_PLL_UNIPHY_PLL_GLB_CFG, 0x5);
		udelay(1000);
		DSS_REG_W(mdss_dsi_base, DSI_0_PHY_PLL_UNIPHY_PLL_GLB_CFG, 0x7);
		udelay(1000);
		DSS_REG_W(mdss_dsi_base, DSI_0_PHY_PLL_UNIPHY_PLL_GLB_CFG, 0xf);
		udelay(2000);

	}

	if ((status & 0x01) != 1) {
		pr_err("%s: DSI PLL status=%x failed to Lock\n",
		       __func__, status);
		rc = -EINVAL;
		goto error;
	}

	pr_debug("%s: DSI PLL Lock success  0x20C 0x%x, PLL lock status 0x%x\n", __func__, REG_R( mdss_dsi_base + 0x020c), REG_R( mdss_dsi_base + 0x02c0));

error:
	return rc;
}

static int dsi_pll_enable(struct clk *c)
{
	int i, rc = 0;
	struct dsi_pll_vco_clk *vco = to_vco_clk(c);

	rc = clk_enable(mdss_ahb_clk);
	if (rc) {
		pr_err("%s: failed to enable mdss ahb clock. rc=%d\n",
			__func__, rc);
		return rc;
	}

	/* Try all enable sequences until one succeeds */
	for (i = 0; i < vco->pll_en_seq_cnt; i++) {
		rc = vco->pll_enable_seqs[i]();
		pr_debug("%s: DSI PLL %s after sequence #%d\n", __func__,
			rc ? "unlocked" : "locked", i + 1);
		if (!rc)
			break;
	}
	clk_disable(mdss_ahb_clk);

	if (rc)
		pr_err("%s: DSI PLL failed to lock\n", __func__);

	return rc;
}

static void dsi_pll_disable(struct clk *c)
{
	int rc = 0;

	pr_debug("### VCO_disable++\n");
	rc = clk_enable(mdss_ahb_clk);
	if (rc) {
		pr_err("%s: failed to enable mdss ahb clock. rc=%d\n",
			__func__, rc);
		return;
	}

	DSS_REG_W(mdss_dsi_base, DSI_0_PHY_PLL_UNIPHY_PLL_GLB_CFG, 0x00);

	clk_disable(mdss_ahb_clk);
	pr_debug("%s: DSI PLL  Disabled\n", __func__);
	pr_debug("### VCO_disable--\n");
	return;
}

static int vco_set_rate(struct clk *c, unsigned long rate)
{
	s64 vco_clk_rate = rate;
	s32 rem;
	s64 refclk_cfg, frac_n_mode, ref_doubler_en_b;
	s64 ref_clk_to_pll, div_fbx1000, frac_n_value;
	s64 sdm_cfg0, sdm_cfg1, sdm_cfg2, sdm_cfg3;
	s64 gen_vco_clk, cal_cfg10, cal_cfg11;
	u32 res;
	int i, rc = 0;
	struct dsi_pll_vco_clk *vco = to_vco_clk(c);

	rc = mdss_ahb_clk_enable(1);
	if (rc) {
		pr_err("%s: failed to enable mdss ahb clock. rc=%d\n",
			__func__, rc);
		return rc;
	}

	/* Configure the Loop filter resistance */
	for (i = 0; i < vco->lpfr_lut_size; i++)
		if (vco_clk_rate <= vco->lpfr_lut[i].vco_rate)
			break;
	if (i == vco->lpfr_lut_size) {
		pr_err("%s: unable to get loop filter resistance. vco=%ld\n",
			__func__, rate);
		rc = -EINVAL;
		goto error;
	}
	res = vco->lpfr_lut[i].r;
	DSS_REG_W(mdss_dsi_base, DSI_0_PHY_PLL_UNIPHY_PLL_LPFR_CFG, res);

	/* Loop filter capacitance values : c1 and c2 */
	DSS_REG_W(mdss_dsi_base, DSI_0_PHY_PLL_UNIPHY_PLL_LPFC1_CFG, 0x70);
	DSS_REG_W(mdss_dsi_base, DSI_0_PHY_PLL_UNIPHY_PLL_LPFC2_CFG, 0x15);

	div_s64_rem(vco_clk_rate, vco->ref_clk_rate, &rem);
	if (rem) {
		refclk_cfg = 0x1;
		frac_n_mode = 1;
		ref_doubler_en_b = 0;
	} else {
		refclk_cfg = 0x0;
		frac_n_mode = 0;
		ref_doubler_en_b = 1;
	}

	pr_debug("%s:refclk_cfg = %lld\n", __func__, refclk_cfg);

	ref_clk_to_pll = ((vco->ref_clk_rate * 2 * (refclk_cfg))
			  + (ref_doubler_en_b * vco->ref_clk_rate));
	div_fbx1000 = div_s64((vco_clk_rate * 1000), ref_clk_to_pll);

	div_s64_rem(div_fbx1000, 1000, &rem);
	frac_n_value = div_s64((rem * (1 << 16)), 1000);
	gen_vco_clk = div_s64(div_fbx1000 * ref_clk_to_pll, 1000);

	pr_debug("%s:ref_clk_to_pll = %lld\n", __func__, ref_clk_to_pll);
	pr_debug("%s:div_fb = %lld\n", __func__, div_fbx1000);
	pr_debug("%s:frac_n_value = %lld\n", __func__, frac_n_value);

	pr_debug("%s:Generated VCO Clock: %lld\n", __func__, gen_vco_clk);
	rem = 0;
	if (frac_n_mode) {
		sdm_cfg0 = (0x0 << 5);
		sdm_cfg0 |= (0x0 & 0x3f);
		sdm_cfg1 = (div_s64(div_fbx1000, 1000) & 0x3f) - 1;
		sdm_cfg3 = div_s64_rem(frac_n_value, 256, &rem);
		sdm_cfg2 = rem;
	} else {
		sdm_cfg0 = (0x1 << 5);
		sdm_cfg0 |= (div_s64(div_fbx1000, 1000) & 0x3f) - 1;
		sdm_cfg1 = (0x0 & 0x3f);
		sdm_cfg2 = 0;
		sdm_cfg3 = 0;
	}

	pr_debug("%s: sdm_cfg0=%lld\n", __func__, sdm_cfg0);
	pr_debug("%s: sdm_cfg1=%lld\n", __func__, sdm_cfg1);
	pr_debug("%s: sdm_cfg2=%lld\n", __func__, sdm_cfg2);
	pr_debug("%s: sdm_cfg3=%lld\n", __func__, sdm_cfg3);

	cal_cfg11 = div_s64_rem(gen_vco_clk, 256 * 1000000, &rem);
	cal_cfg10 = rem / 1000000;
	pr_debug("%s: cal_cfg10=%lld, cal_cfg11=%lld\n", __func__,
		cal_cfg10, cal_cfg11);

	DSS_REG_W(mdss_dsi_base, DSI_0_PHY_PLL_UNIPHY_PLL_CHGPUMP_CFG, 0x02);
	DSS_REG_W(mdss_dsi_base, DSI_0_PHY_PLL_UNIPHY_PLL_CAL_CFG3, 0x2b);
	DSS_REG_W(mdss_dsi_base, DSI_0_PHY_PLL_UNIPHY_PLL_CAL_CFG4, 0x66);
	DSS_REG_W(mdss_dsi_base, DSI_0_PHY_PLL_UNIPHY_PLL_LKDET_CFG2, 0x05);

	DSS_REG_W(mdss_dsi_base, DSI_0_PHY_PLL_UNIPHY_PLL_SDM_CFG1,
		(u32)(sdm_cfg1 & 0xff));
	DSS_REG_W(mdss_dsi_base, DSI_0_PHY_PLL_UNIPHY_PLL_SDM_CFG2,
		(u32)(sdm_cfg2 & 0xff));
	DSS_REG_W(mdss_dsi_base, DSI_0_PHY_PLL_UNIPHY_PLL_SDM_CFG3,
		(u32)(sdm_cfg3 & 0xff));
	DSS_REG_W(mdss_dsi_base, DSI_0_PHY_PLL_UNIPHY_PLL_SDM_CFG4, 0x00);

	/* Add hardware recommended delay for correct PLL configuration */
	udelay(1000);

	DSS_REG_W(mdss_dsi_base, DSI_0_PHY_PLL_UNIPHY_PLL_REFCLK_CFG,
		(u32)refclk_cfg);
	DSS_REG_W(mdss_dsi_base, DSI_0_PHY_PLL_UNIPHY_PLL_PWRGEN_CFG, 0x00);
	DSS_REG_W(mdss_dsi_base, DSI_0_PHY_PLL_UNIPHY_PLL_VCOLPF_CFG, 0x71);
	DSS_REG_W(mdss_dsi_base, DSI_0_PHY_PLL_UNIPHY_PLL_SDM_CFG0,
		(u32)sdm_cfg0);
	DSS_REG_W(mdss_dsi_base, DSI_0_PHY_PLL_UNIPHY_PLL_CAL_CFG0, 0x0a);
	DSS_REG_W(mdss_dsi_base, DSI_0_PHY_PLL_UNIPHY_PLL_CAL_CFG6, 0x30);
	DSS_REG_W(mdss_dsi_base, DSI_0_PHY_PLL_UNIPHY_PLL_CAL_CFG7, 0x00);
	DSS_REG_W(mdss_dsi_base, DSI_0_PHY_PLL_UNIPHY_PLL_CAL_CFG8, 0x60);
	DSS_REG_W(mdss_dsi_base, DSI_0_PHY_PLL_UNIPHY_PLL_CAL_CFG9, 0x00);
	DSS_REG_W(mdss_dsi_base, DSI_0_PHY_PLL_UNIPHY_PLL_CAL_CFG10,
		(u32)(cal_cfg10 & 0xff));
	DSS_REG_W(mdss_dsi_base, DSI_0_PHY_PLL_UNIPHY_PLL_CAL_CFG11,
		(u32)(cal_cfg11 & 0xff));
	DSS_REG_W(mdss_dsi_base, DSI_0_PHY_PLL_UNIPHY_PLL_EFUSE_CFG, 0x20);

error:
	mdss_ahb_clk_enable(0);
	return rc;
}

/* rate is the bit clk rate */
static long vco_round_rate(struct clk *c, unsigned long rate)
{
	unsigned long rrate = rate;
	struct dsi_pll_vco_clk *vco = to_vco_clk(c);

	if (rate < vco->min_rate)
		rrate = vco->min_rate;
	if (rate > vco->max_rate)
		rrate = vco->max_rate;

	return rrate;
}

static unsigned long vco_get_rate(struct clk *c)
{
	u32 sdm0, doubler, sdm_byp_div;
	u64 vco_rate;
	u32 sdm_dc_off, sdm_freq_seed, sdm2, sdm3;
	struct dsi_pll_vco_clk *vco = to_vco_clk(c);
	u64 ref_clk = vco->ref_clk_rate;

	/* Check to see if the ref clk doubler is enabled */
	doubler = DSS_REG_R(mdss_dsi_base, DSI_0_PHY_PLL_UNIPHY_PLL_REFCLK_CFG)
		& BIT(0);
	ref_clk += (doubler * vco->ref_clk_rate);

	/* see if it is integer mode or sdm mode */
	sdm0 = DSS_REG_R(mdss_dsi_base, DSI_0_PHY_PLL_UNIPHY_PLL_SDM_CFG0);
	if (sdm0 & BIT(6)) {
		/* integer mode */
		sdm_byp_div = (DSS_REG_R(mdss_dsi_base,
			DSI_0_PHY_PLL_UNIPHY_PLL_SDM_CFG0) & 0x3f) + 1;
		vco_rate = ref_clk * sdm_byp_div;
	} else {
		/* sdm mode */
		sdm_dc_off = DSS_REG_R(mdss_dsi_base,
			DSI_0_PHY_PLL_UNIPHY_PLL_SDM_CFG1) & 0xFF;
		pr_err("%s: sdm_dc_off = %d\n", __func__, sdm_dc_off);
		sdm2 = DSS_REG_R(mdss_dsi_base,
			DSI_0_PHY_PLL_UNIPHY_PLL_SDM_CFG2) & 0xFF;
		sdm3 = DSS_REG_R(mdss_dsi_base,
			DSI_0_PHY_PLL_UNIPHY_PLL_SDM_CFG3) & 0xFF;
		sdm_freq_seed = (sdm3 << 8) | sdm2;
		pr_err("%s: sdm_freq_seed = %d\n", __func__, sdm_freq_seed);

		vco_rate = (ref_clk * (sdm_dc_off + 1)) +
			mult_frac(ref_clk, sdm_freq_seed, BIT(16));
		pr_err("%s: vco rate = %lld", __func__, vco_rate);
	}

	pr_err("%s: returning vco rate = %lu\n", __func__,
		(unsigned long)vco_rate);
	return (unsigned long)vco_rate;
}

static enum handoff vco_handoff(struct clk *c)
{
	int rc = 0;
	enum handoff ret = HANDOFF_DISABLED_CLK;

	printk("#### vco_handloff++\n"); //Scott
	//return HANDOFF_DISABLED_CLK;

	rc = mdss_ahb_clk_enable(1);
	if (rc) {
		pr_err("%s: failed to enable mdss ahb clock. rc=%d\n",
			__func__, rc);
		return ret;
	}
	if (dsi_pll_lock_status()) {
		c->rate = vco_get_rate(c);
		ret = HANDOFF_ENABLED_CLK;
	}

	mdss_ahb_clk_enable(0);

		printk("#### vco_handoff-- ret %d\n", ret);
	return ret;
}

static int vco_prepare(struct clk *c)
{
	int rc = 0;

	if (vco_cached_rate != 0) {
		rc = vco_set_rate(c, vco_cached_rate);
		if (rc) {
			pr_err("%s: vco_set_rate failed. rc=%d\n",
				__func__, rc);
			goto error;
}
	}

	rc = dsi_pll_enable(c);

error:
	return rc;
}

static void vco_unprepare(struct clk *c)
{
	vco_cached_rate = c->rate;
	dsi_pll_disable(c);
}

/* Op structures */

static struct clk_ops clk_ops_dsi_vco = {
	.set_rate = vco_set_rate,
	.round_rate = vco_round_rate,
	.handoff = vco_handoff,
	.prepare = vco_prepare,
	.unprepare = vco_unprepare,
};

static struct clk_div_ops fixed_2div_ops;

static struct clk_div_ops fixed_4div_ops = {
	.set_div = fixed_4div_set_div,
	.get_div = fixed_4div_get_div,
};

static struct clk_div_ops analog_postdiv_ops = {
	.set_div = analog_set_div,
	.get_div = analog_get_div,
};

static struct clk_div_ops digital_postdiv_ops = {
	.set_div = digital_set_div,
	.get_div = digital_get_div,
};

struct clk_mux_ops byte_mux_ops = {
	.set_mux_sel = set_byte_mux_sel,
	.get_mux_sel = get_byte_mux_sel,
};

struct clk_ops byte_mux_clk_ops;

static struct clk_ops pixel_clk_src_ops;
static struct clk_ops byte_clk_src_ops;
static struct clk_ops analog_potsdiv_clk_ops;

/* Display clocks */

struct dsi_pll_vco_clk dsi_vco_clk_8226 = {
	.ref_clk_rate = 19200000,
	.min_rate = 350000000,
	.max_rate = 750000000,
	.pll_en_seq_cnt = 6,
	.pll_enable_seqs[0] = dsi_pll_enable_seq_m,
	.pll_enable_seqs[1] = dsi_pll_enable_seq_d,
	.pll_enable_seqs[2] = dsi_pll_enable_seq_d,
	.pll_enable_seqs[3] = dsi_pll_enable_seq_f1,
	.pll_enable_seqs[4] = dsi_pll_enable_seq_c,
	.pll_enable_seqs[5] = dsi_pll_enable_seq_e,
	.lpfr_lut_size = 10,
	.lpfr_lut = (struct lpfr_cfg[]){
		{479500000, 8},
		{480000000, 11},
		{575500000, 8},
		{576000000, 12},
		{610500000, 8},
		{659500000, 9},
		{671500000, 10},
		{672000000, 14},
		{708500000, 10},
		{750000000, 11},
	},
	.c = {
		.dbg_name = "dsi_vco_clk",
		.ops = &clk_ops_dsi_vco,
		CLK_INIT(dsi_vco_clk_8226.c),
	},
};

struct div_clk analog_postdiv_clk_8226 = {
	.max_div = 255,
	.min_div = 1,
	.ops = &analog_postdiv_ops,
	.c = {
		.parent = &dsi_vco_clk_8226.c,
		.dbg_name = "analog_postdiv_clk",
		.ops = &analog_potsdiv_clk_ops,
		.flags = CLKFLAG_NO_RATE_CACHE,
		CLK_INIT(analog_postdiv_clk_8226.c),
	},
};

struct div_clk indirect_path_div2_clk_8226 = {
	.ops = &fixed_2div_ops,
	.div = 2,
	.c = {
		.parent = &analog_postdiv_clk_8226.c,
		.dbg_name = "indirect_path_div2_clk",
		.ops = &clk_ops_div,
		.flags = CLKFLAG_NO_RATE_CACHE,
		CLK_INIT(indirect_path_div2_clk_8226.c),
	},
};

struct div_clk pixel_clk_src_8226 = {
	.max_div = 255,
	.min_div = 1,
	.ops = &digital_postdiv_ops,
	.c = {
		.parent = &dsi_vco_clk_8226.c,
		.dbg_name = "pixel_clk_src",
		.ops = &pixel_clk_src_ops,
		.flags = CLKFLAG_NO_RATE_CACHE,
		CLK_INIT(pixel_clk_src_8226.c),
	},
};

struct mux_clk byte_mux_8226 = {
	.num_parents = 2,
	.parents = (struct clk_src[]){
		{&dsi_vco_clk_8226.c, 0},
		{&indirect_path_div2_clk_8226.c, 1},
	},
	.ops = &byte_mux_ops,
	.c = {
		.parent = &dsi_vco_clk_8226.c,
		.dbg_name = "byte_mux",
		.ops = &byte_mux_clk_ops,
		CLK_INIT(byte_mux_8226.c),
	},
};

struct div_clk byte_clk_src_8226 = {
	.ops = &fixed_4div_ops,
	.min_div = 4,
	.max_div = 4,
	.c = {
		.parent = &byte_mux_8226.c,
		.dbg_name = "byte_clk_src",
		.ops = &byte_clk_src_ops,
		CLK_INIT(byte_clk_src_8226.c),
	},
};

struct dsi_pll_vco_clk dsi_vco_clk_8974 = {
	.ref_clk_rate = 19200000,
	.min_rate = 350000000,
	.max_rate = 750000000,
	.pll_en_seq_cnt = 3,
	.pll_enable_seqs[0] = dsi_pll_enable_seq_8974,
	.pll_enable_seqs[1] = dsi_pll_enable_seq_8974,
	.pll_enable_seqs[2] = dsi_pll_enable_seq_8974,
	.lpfr_lut_size = 10,
	.lpfr_lut = (struct lpfr_cfg[]){
		{479500000, 8},
		{480000000, 11},
		{575500000, 8},
		{576000000, 12},
		{610500000, 8},
		{659500000, 9},
		{671500000, 10},
		{672000000, 14},
		{708500000, 10},
		{750000000, 11},
	},
	.c = {
		.dbg_name = "dsi_vco_clk",
		.ops = &clk_ops_dsi_vco,
		CLK_INIT(dsi_vco_clk_8974.c),
	},
};

struct div_clk analog_postdiv_clk_8974 = {
	.max_div = 255,
	.min_div = 1,
	.ops = &analog_postdiv_ops,
	.c = {
		.parent = &dsi_vco_clk_8974.c,
		.dbg_name = "analog_postdiv_clk",
		.ops = &analog_potsdiv_clk_ops,
		.flags = CLKFLAG_NO_RATE_CACHE,
		CLK_INIT(analog_postdiv_clk_8974.c),
	},
};

struct div_clk indirect_path_div2_clk_8974 = {
	.ops = &fixed_2div_ops,
	.div = 2,
	.c = {
		.parent = &analog_postdiv_clk_8974.c,
		.dbg_name = "indirect_path_div2_clk",
		.ops = &clk_ops_div,
		.flags = CLKFLAG_NO_RATE_CACHE,
		CLK_INIT(indirect_path_div2_clk_8974.c),
	},
};

struct div_clk pixel_clk_src_8974 = {
	.max_div = 255,
	.min_div = 1,
	.ops = &digital_postdiv_ops,
	.c = {
		.parent = &dsi_vco_clk_8974.c,
		.dbg_name = "pixel_clk_src",
		.ops = &pixel_clk_src_ops,
		.flags = CLKFLAG_NO_RATE_CACHE,
		CLK_INIT(pixel_clk_src_8974.c),
	},
};

struct mux_clk byte_mux_8974 = {
	.num_parents = 2,
	.parents = (struct clk_src[]){
		{&dsi_vco_clk_8974.c, 0},
		{&indirect_path_div2_clk_8974.c, 1},
	},
	.ops = &byte_mux_ops,
	.c = {
		.parent = &dsi_vco_clk_8974.c,
		.dbg_name = "byte_mux",
		.ops = &byte_mux_clk_ops,
		CLK_INIT(byte_mux_8974.c),
	},
};

struct div_clk byte_clk_src_8974 = {
	.ops = &fixed_4div_ops,
	.min_div = 4,
	.max_div = 4,
	.c = {
		.parent = &byte_mux_8974.c,
		.dbg_name = "byte_clk_src",
		.ops = &byte_clk_src_ops,
		CLK_INIT(byte_clk_src_8974.c),
	},
};

static inline struct edp_pll_vco_clk *to_edp_vco_clk(struct clk *clk)
{
	return container_of(clk, struct edp_pll_vco_clk, c);
}

static int edp_vco_set_rate(struct clk *c, unsigned long vco_rate)
{
	struct edp_pll_vco_clk *vco = to_edp_vco_clk(c);
	int rc = 0;

	pr_debug("%s: vco_rate=%d\n", __func__, (int)vco_rate);

	rc = mdss_ahb_clk_enable(1);
	if (rc) {
		pr_err("%s: failed to enable mdss ahb clock. rc=%d\n",
			__func__, rc);
		rc =  -EINVAL;
	}
	if (vco_rate == 810000000) {
		DSS_REG_W(mdss_edp_base, 0x0c, 0x18);
		/* UNIPHY_PLL_LKDET_CFG2 */
		DSS_REG_W(mdss_edp_base, 0x64, 0x05);
		/* UNIPHY_PLL_REFCLK_CFG */
		DSS_REG_W(mdss_edp_base, 0x00, 0x00);
		/* UNIPHY_PLL_SDM_CFG0 */
		DSS_REG_W(mdss_edp_base, 0x38, 0x36);
		/* UNIPHY_PLL_SDM_CFG1 */
		DSS_REG_W(mdss_edp_base, 0x3c, 0x69);
		/* UNIPHY_PLL_SDM_CFG2 */
		DSS_REG_W(mdss_edp_base, 0x40, 0xff);
		/* UNIPHY_PLL_SDM_CFG3 */
		DSS_REG_W(mdss_edp_base, 0x44, 0x2f);
		/* UNIPHY_PLL_SDM_CFG4 */
		DSS_REG_W(mdss_edp_base, 0x48, 0x00);
		/* UNIPHY_PLL_SSC_CFG0 */
		DSS_REG_W(mdss_edp_base, 0x4c, 0x80);
		/* UNIPHY_PLL_SSC_CFG1 */
		DSS_REG_W(mdss_edp_base, 0x50, 0x00);
		/* UNIPHY_PLL_SSC_CFG2 */
		DSS_REG_W(mdss_edp_base, 0x54, 0x00);
		/* UNIPHY_PLL_SSC_CFG3 */
		DSS_REG_W(mdss_edp_base, 0x58, 0x00);
		/* UNIPHY_PLL_CAL_CFG0 */
		DSS_REG_W(mdss_edp_base, 0x6c, 0x0a);
		/* UNIPHY_PLL_CAL_CFG2 */
		DSS_REG_W(mdss_edp_base, 0x74, 0x01);
		/* UNIPHY_PLL_CAL_CFG6 */
		DSS_REG_W(mdss_edp_base, 0x84, 0x5a);
		/* UNIPHY_PLL_CAL_CFG7 */
		DSS_REG_W(mdss_edp_base, 0x88, 0x0);
		/* UNIPHY_PLL_CAL_CFG8 */
		DSS_REG_W(mdss_edp_base, 0x8c, 0x60);
		/* UNIPHY_PLL_CAL_CFG9 */
		DSS_REG_W(mdss_edp_base, 0x90, 0x0);
		/* UNIPHY_PLL_CAL_CFG10 */
		DSS_REG_W(mdss_edp_base, 0x94, 0x2a);
		/* UNIPHY_PLL_CAL_CFG11 */
		DSS_REG_W(mdss_edp_base, 0x98, 0x3);
		/* UNIPHY_PLL_LKDET_CFG0 */
		DSS_REG_W(mdss_edp_base, 0x5c, 0x10);
		/* UNIPHY_PLL_LKDET_CFG1 */
		DSS_REG_W(mdss_edp_base, 0x60, 0x1a);
		/* UNIPHY_PLL_POSTDIV1_CFG */
		DSS_REG_W(mdss_edp_base, 0x04, 0x00);
		/* UNIPHY_PLL_POSTDIV3_CFG */
		DSS_REG_W(mdss_edp_base, 0x28, 0x00);
	} else if (vco_rate == 1350000000) {
		/* UNIPHY_PLL_LKDET_CFG2 */
		DSS_REG_W(mdss_edp_base, 0x64, 0x05);
		/* UNIPHY_PLL_REFCLK_CFG */
		DSS_REG_W(mdss_edp_base, 0x00, 0x01);
		/* UNIPHY_PLL_SDM_CFG0 */
		DSS_REG_W(mdss_edp_base, 0x38, 0x36);
		/* UNIPHY_PLL_SDM_CFG1 */
		DSS_REG_W(mdss_edp_base, 0x3c, 0x62);
		/* UNIPHY_PLL_SDM_CFG2 */
		DSS_REG_W(mdss_edp_base, 0x40, 0x00);
		/* UNIPHY_PLL_SDM_CFG3 */
		DSS_REG_W(mdss_edp_base, 0x44, 0x28);
		/* UNIPHY_PLL_SDM_CFG4 */
		DSS_REG_W(mdss_edp_base, 0x48, 0x00);
		/* UNIPHY_PLL_SSC_CFG0 */
		DSS_REG_W(mdss_edp_base, 0x4c, 0x80);
		/* UNIPHY_PLL_SSC_CFG1 */
		DSS_REG_W(mdss_edp_base, 0x50, 0x00);
		/* UNIPHY_PLL_SSC_CFG2 */
		DSS_REG_W(mdss_edp_base, 0x54, 0x00);
		/* UNIPHY_PLL_SSC_CFG3 */
		DSS_REG_W(mdss_edp_base, 0x58, 0x00);
		/* UNIPHY_PLL_CAL_CFG0 */
		DSS_REG_W(mdss_edp_base, 0x6c, 0x0a);
		/* UNIPHY_PLL_CAL_CFG2 */
		DSS_REG_W(mdss_edp_base, 0x74, 0x01);
		/* UNIPHY_PLL_CAL_CFG6 */
		DSS_REG_W(mdss_edp_base, 0x84, 0x5a);
		/* UNIPHY_PLL_CAL_CFG7 */
		DSS_REG_W(mdss_edp_base, 0x88, 0x0);
		/* UNIPHY_PLL_CAL_CFG8 */
		DSS_REG_W(mdss_edp_base, 0x8c, 0x60);
		/* UNIPHY_PLL_CAL_CFG9 */
		DSS_REG_W(mdss_edp_base, 0x90, 0x0);
		/* UNIPHY_PLL_CAL_CFG10 */
		DSS_REG_W(mdss_edp_base, 0x94, 0x46);
		/* UNIPHY_PLL_CAL_CFG11 */
		DSS_REG_W(mdss_edp_base, 0x98, 0x5);
		/* UNIPHY_PLL_LKDET_CFG0 */
		DSS_REG_W(mdss_edp_base, 0x5c, 0x10);
		/* UNIPHY_PLL_LKDET_CFG1 */
		DSS_REG_W(mdss_edp_base, 0x60, 0x1a);
		/* UNIPHY_PLL_POSTDIV1_CFG */
		DSS_REG_W(mdss_edp_base, 0x04, 0x00);
		/* UNIPHY_PLL_POSTDIV3_CFG */
		DSS_REG_W(mdss_edp_base, 0x28, 0x00);
	} else {
		pr_err("%s: rate=%d is NOT supported\n", __func__,
					(int)vco_rate);
		vco_rate = 0;
		rc =  -EINVAL;
	}

	DSS_REG_W(mdss_edp_base, 0x20, 0x01); /* UNIPHY_PLL_GLB_CFG */
	udelay(100);
	DSS_REG_W(mdss_edp_base, 0x20, 0x05); /* UNIPHY_PLL_GLB_CFG */
	udelay(100);
	DSS_REG_W(mdss_edp_base, 0x20, 0x07); /* UNIPHY_PLL_GLB_CFG */
	udelay(100);
	DSS_REG_W(mdss_edp_base, 0x20, 0x0f); /* UNIPHY_PLL_GLB_CFG */
	udelay(100);
	mdss_ahb_clk_enable(0);

	vco->rate = vco_rate;

	return rc;
}

static int edp_pll_ready_poll(void)
{
	int cnt;
	u32 status;

	/* ahb clock should be enabled by caller */
	cnt = 100;
	while (cnt--) {
		udelay(100);
		status = DSS_REG_R(mdss_edp_base, 0xc0);
		status &= 0x01;
		if (status)
			break;
	}
	pr_debug("%s: cnt=%d status=%d\n", __func__, cnt, (int)status);

	if (status)
		return 1;

	pr_err("%s: PLL NOT ready\n", __func__);
	return 0;
}

static int edp_vco_enable(struct clk *c)
{
	int i, ready;
	int rc = 0;

	if (!mdss_gdsc_enabled()) {
		pr_err("%s: mdss GDSC is not enabled\n", __func__);
		return -EPERM;
	}

	/* called from enable, irq disable. can not call clk_prepare */
	rc = clk_enable(mdss_ahb_clk);
	if (rc) {
		pr_err("%s: failed to enable mdss ahb clock. rc=%d\n",
			__func__, rc);
		return rc;
	}

	for (i = 0; i < 3; i++) {
		ready = edp_pll_ready_poll();
		if (ready)
			break;
		DSS_REG_W(mdss_edp_base, 0x20, 0x01); /* UNIPHY_PLL_GLB_CFG */
		udelay(100);
		DSS_REG_W(mdss_edp_base, 0x20, 0x05); /* UNIPHY_PLL_GLB_CFG */
		udelay(100);
		DSS_REG_W(mdss_edp_base, 0x20, 0x07); /* UNIPHY_PLL_GLB_CFG */
		udelay(100);
		DSS_REG_W(mdss_edp_base, 0x20, 0x0f); /* UNIPHY_PLL_GLB_CFG */
		udelay(100);
	}
	clk_disable(mdss_ahb_clk);

	if (ready) {
		pr_debug("%s: EDP PLL locked\n", __func__);
		return 0;
	}

	pr_err("%s: EDP PLL failed to lock\n", __func__);
	return  -EINVAL;
}

static void edp_vco_disable(struct clk *c)
{
	int rc = 0;

	if (!mdss_gdsc_enabled()) {
		pr_err("%s: mdss GDSC is not enabled\n", __func__);
		return;
	}

	/* called from unprepare which is not atomic */
	rc = mdss_ahb_clk_enable(1);
	if (rc) {
		pr_err("%s: failed to enable mdss ahb clock. rc=%d\n",
			__func__, rc);
		return;
	}

	DSS_REG_W(mdss_edp_base, 0x20, 0x00);

	mdss_ahb_clk_enable(0);

	pr_debug("%s: EDP PLL Disabled\n", __func__);
	return;
}

static unsigned long edp_vco_get_rate(struct clk *c)
{
	struct edp_pll_vco_clk *vco = to_edp_vco_clk(c);
	u32 pll_status, div2;
	int rc;

	rc = mdss_ahb_clk_enable(1);
	if (rc) {
		pr_err("%s: failed to enable mdss ahb clock. rc=%d\n",
			__func__, rc);
		return rc;
	}
	if (vco->rate == 0) {
		pll_status = DSS_REG_R(mdss_edp_base, 0xc0);
		if (pll_status & 0x01) {
			div2 = DSS_REG_R(mdss_edp_base, 0x24);
			if (div2 & 0x01)
				vco->rate = 1350000000;
			else
				vco->rate = 810000000;
		}
	}
	mdss_ahb_clk_enable(0);

	pr_debug("%s: rate=%d\n", __func__, (int)vco->rate);

	return vco->rate;
}

static long edp_vco_round_rate(struct clk *c, unsigned long rate)
{
	struct edp_pll_vco_clk *vco = to_edp_vco_clk(c);
	unsigned long rrate = -ENOENT;
	unsigned long *lp;

	lp = vco->rate_list;
	while (*lp) {
		rrate = *lp;
		if (rate <= rrate)
			break;
		lp++;
	}

	pr_debug("%s: rrate=%d\n", __func__, (int)rrate);

	return rrate;
}

static int edp_vco_prepare(struct clk *c)
{
	struct edp_pll_vco_clk *vco = to_edp_vco_clk(c);

	pr_debug("%s: rate=%d\n", __func__, (int)vco->rate);

	return edp_vco_set_rate(c, vco->rate);
}

static void edp_vco_unprepare(struct clk *c)
{
	struct edp_pll_vco_clk *vco = to_edp_vco_clk(c);

	pr_debug("%s: rate=%d\n", __func__, (int)vco->rate);

	edp_vco_disable(c);
}

static int edp_pll_lock_status(void)
{
	u32 status;
	int pll_locked = 0;
	int rc;

	rc = mdss_ahb_clk_enable(1);
	if (rc) {
		pr_err("%s: failed to enable mdss ahb clock. rc=%d\n",
			__func__, rc);
		return rc;
	}
	/* poll for PLL ready status */
	if (readl_poll_timeout_noirq((mdss_edp_base + 0xc0),
			status, ((status & BIT(0)) == 1),
			PLL_POLL_MAX_READS, PLL_POLL_TIMEOUT_US)) {
		pr_debug("%s: EDP PLL status=%x failed to Lock\n",
				__func__, status);
		pll_locked = 0;
	} else {
		pll_locked = 1;
	}
	mdss_ahb_clk_enable(0);

	return pll_locked;
}

static enum handoff edp_vco_handoff(struct clk *c)
{
	enum handoff ret = HANDOFF_DISABLED_CLK;

	if (edp_pll_lock_status()) {
		c->rate = edp_vco_get_rate(c);
		ret = HANDOFF_ENABLED_CLK;
	}

	pr_debug("%s: done, ret=%d\n", __func__, ret);
	return ret;
}

/* edp vco rate */
static unsigned long edp_vco_rate_list[] = {
		810000000, 1350000000, 0};

struct clk_ops edp_vco_clk_ops = {
	.enable = edp_vco_enable,
	.set_rate = edp_vco_set_rate,
	.get_rate = edp_vco_get_rate,
	.round_rate = edp_vco_round_rate,
	.prepare = edp_vco_prepare,
	.unprepare = edp_vco_unprepare,
	.handoff = edp_vco_handoff,
};

struct edp_pll_vco_clk edp_vco_clk = {
	.ref_clk_rate = 19200000,
	.rate = 0,
	.rate_list = edp_vco_rate_list,
	.c = {
		.dbg_name = "edp_vco_clk",
		.ops = &edp_vco_clk_ops,
		CLK_INIT(edp_vco_clk.c),
	},
};

static unsigned long edp_mainlink_get_rate(struct clk *c)
{
	struct div_clk *mclk = to_div_clk(c);
	struct clk *pclk;
	unsigned long rate = 0;

	pclk = clk_get_parent(c);

	if (pclk->ops->get_rate) {
		rate = pclk->ops->get_rate(pclk);
		rate /= mclk->div;
	}

	pr_debug("%s: rate=%d div=%d\n", __func__, (int)rate, mclk->div);

	return rate;
}

static struct clk_ops edp_mainlink_clk_src_ops;
static struct clk_div_ops fixed_5div_ops; /* null ops */

struct div_clk edp_mainlink_clk_src = {
	.ops = &fixed_5div_ops,
	.div = 5,
	.c = {
		.parent = &edp_vco_clk.c,
		.dbg_name = "edp_mainlink_clk_src",
		.ops = &edp_mainlink_clk_src_ops,
		.flags = CLKFLAG_NO_RATE_CACHE,
		CLK_INIT(edp_mainlink_clk_src.c),
	}
};


static struct clk_ops edp_pixel_clk_ops;

/*
 * this rate is from pll to clock controller
 * output from pll to CC has two possibilities
 * 1: if mainlink rate is 270M, then 675M
 * 2: if mainlink rate is 162M, then 810M
 */
static int edp_pixel_set_div(struct div_clk *clk, int div)
{
	int rc = 0;

	rc = mdss_ahb_clk_enable(1);
	if (rc) {
		pr_err("%s: failed to enable mdss ahb clock. rc=%d\n",
			__func__, rc);
		return rc;
	}

	pr_debug("%s: div=%d\n", __func__, div);
	DSS_REG_W(mdss_edp_base, 0x24, (div - 1)); /* UNIPHY_PLL_POSTDIV2_CFG */

	mdss_ahb_clk_enable(0);
	return 0;
}

static int edp_pixel_get_div(struct div_clk *clk)
{
	int div = 0;

	if (mdss_ahb_clk_enable(1)) {
		pr_debug("%s: Failed to enable mdss ahb clock\n", __func__);
		return 1;
	}
	div = DSS_REG_R(mdss_edp_base, 0x24); /* UNIPHY_PLL_POSTDIV2_CFG */
	div &= 0x01;
	pr_debug("%s: div=%d\n", __func__, div);
	mdss_ahb_clk_enable(0);
	return div + 1;
}

static struct clk_div_ops edp_pixel_ops = {
	.set_div = edp_pixel_set_div,
	.get_div = edp_pixel_get_div,
};

struct div_clk edp_pixel_clk_src = {
	.max_div = 2,
	.min_div = 1,
	.ops = &edp_pixel_ops,
	.c = {
		.parent = &edp_vco_clk.c,
		.dbg_name = "edp_pixel_clk_src",
		.ops = &edp_pixel_clk_ops,
		.flags = CLKFLAG_NO_RATE_CACHE,
		CLK_INIT(edp_pixel_clk_src.c),
	},
};

void __init mdss_clk_ctrl_pre_init(struct clk *ahb_clk)
{
	BUG_ON(ahb_clk == NULL);

	gdsc_base = ioremap(GDSC_PHYS, GDSC_SIZE);
	if (!gdsc_base)
		pr_err("%s: unable to remap gdsc base", __func__);

	mdss_dsi_base = ioremap(DSI_PHY_PHYS, DSI_PHY_SIZE);
	if (!mdss_dsi_base)
		pr_err("%s: unable to remap dsi base", __func__);

	mdss_ahb_clk = ahb_clk;

	hdmi_phy_base = ioremap(HDMI_PHY_PHYS, HDMI_PHY_SIZE);
	if (!hdmi_phy_base)
		pr_err("%s: unable to ioremap hdmi phy base", __func__);

	hdmi_phy_pll_base = ioremap(HDMI_PHY_PLL_PHYS, HDMI_PHY_PLL_SIZE);
	if (!hdmi_phy_pll_base)
		pr_err("%s: unable to ioremap hdmi phy pll base", __func__);

	mdss_edp_base = ioremap(EDP_PHY_PHYS, EDP_PHY_SIZE);
	if (!mdss_edp_base)
		pr_err("%s: unable to remap edp base", __func__);

	pixel_clk_src_ops = clk_ops_slave_div;
	pixel_clk_src_ops.prepare = div_prepare;

	byte_clk_src_ops = clk_ops_div;
	byte_clk_src_ops.prepare = div_prepare;

	analog_potsdiv_clk_ops = clk_ops_div;
	analog_potsdiv_clk_ops.prepare = div_prepare;

	byte_mux_clk_ops = clk_ops_gen_mux;
	byte_mux_clk_ops.prepare = mux_prepare;

	edp_mainlink_clk_src_ops = clk_ops_div;
	edp_mainlink_clk_src_ops.get_parent = clk_get_parent;
	edp_mainlink_clk_src_ops.get_rate = edp_mainlink_get_rate;

	edp_pixel_clk_ops = clk_ops_slave_div;
	edp_pixel_clk_ops.prepare = div_prepare;
}
