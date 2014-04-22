/*Copyright (c) 2012-2013, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details .
 *
 */

#include <linux/init.h>
#include <linux/ioport.h>
#include <mach/board.h>
#include <mach/gpio.h>
#include <mach/gpiomux.h>
#include <mach/socinfo.h>

//++ p11309 - 2013.07.06 for Touch Power Enable Key
#if defined(CONFIG_MACH_MSM8974_EF56S) || defined(CONFIG_MACH_MSM8974_EF59S) || defined(CONFIG_MACH_MSM8974_EF59K) || defined(CONFIG_MACH_MSM8974_EF59L) || defined(CONFIG_MACH_MSM8974_EF60S) || defined(CONFIG_MACH_MSM8974_EF61K) || defined(CONFIG_MACH_MSM8974_EF62L)
//#define KS8851_IRQ_GPIO 94
#define KS8851_IRQ_GPIO 33
#endif
//-- p11309

static struct gpiomux_setting ap2mdm_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting mdm2ap_status_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_IN,
};

static struct gpiomux_setting mdm2ap_errfatal_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_IN,
};

static struct gpiomux_setting mdm2ap_pblrdy = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_IN,
};


static struct gpiomux_setting ap2mdm_soft_reset_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting ap2mdm_wakeup = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct msm_gpiomux_config mdm_configs[] __initdata = {
	/* AP2MDM_STATUS */
	{
		.gpio = 105,
		.settings = {
			[GPIOMUX_SUSPENDED] = &ap2mdm_cfg,
		}
	},
	/* MDM2AP_STATUS */
	{
		.gpio = 46,
		.settings = {
			[GPIOMUX_SUSPENDED] = &mdm2ap_status_cfg,
		}
	},
	/* MDM2AP_ERRFATAL */
	{
		.gpio = 82,
		.settings = {
			[GPIOMUX_SUSPENDED] = &mdm2ap_errfatal_cfg,
		}
	},
	/* AP2MDM_ERRFATAL */
	{
		.gpio = 106,
		.settings = {
			[GPIOMUX_SUSPENDED] = &ap2mdm_cfg,
		}
	},
	/* AP2MDM_SOFT_RESET, aka AP2MDM_PON_RESET_N */
	{
		.gpio = 24,
		.settings = {
			[GPIOMUX_SUSPENDED] = &ap2mdm_soft_reset_cfg,
		}
	},
	/* AP2MDM_WAKEUP */
	{
		.gpio = 104,
		.settings = {
			[GPIOMUX_SUSPENDED] = &ap2mdm_wakeup,
		}
	},
	/* MDM2AP_PBL_READY*/
	{
		.gpio = 80,
		.settings = {
			[GPIOMUX_SUSPENDED] = &mdm2ap_pblrdy,
		}
	},
};

static struct gpiomux_setting gpio_uart_config = {
	.func = GPIOMUX_FUNC_2,
	.drv = GPIOMUX_DRV_2MA, //qualcomm : 16MA
	.pull = GPIOMUX_PULL_DOWN, //qualcomm : PULL_NONE
	.dir = GPIOMUX_IN,         //qualcomm : OUT
};

static struct gpiomux_setting slimbus = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_KEEPER,
};

#if defined(CONFIG_KS8851) || defined(CONFIG_KS8851_MODULE)
static struct gpiomux_setting gpio_eth_config = {
//++ p11309 - 2013.07.06 for Touch Power Enable Key
//	.pull = GPIOMUX_PULL_UP,
	.pull = GPIOMUX_PULL_DOWN,
//-- p11309
	.drv = GPIOMUX_DRV_2MA,
	.func = GPIOMUX_FUNC_GPIO,
};
static struct gpiomux_setting gpio_spi_cs2_config = {
	.func = GPIOMUX_FUNC_4,
	.drv = GPIOMUX_DRV_6MA,
	.pull = GPIOMUX_PULL_DOWN,
};
#if !defined(FEATURE_PANTECH_CONSOLE_UART1) && !defined(CONFIG_PANTECH_PMIC_FUELGAUGE_MAX17058) && !defined(CONFIG_SKY_DMB_SPI_HW) //temp fix build error
static struct gpiomux_setting gpio_spi_config = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_12MA,
	.pull = GPIOMUX_PULL_NONE,
};
#endif
static struct gpiomux_setting gpio_spi_cs1_config = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_6MA,
	.pull = GPIOMUX_PULL_UP,
};
static struct msm_gpiomux_config msm_eth_configs[] = {
	{
		.gpio = KS8851_IRQ_GPIO,
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_eth_config,
		}
	},
};
#endif
#ifdef CONFIG_INPUT_FPC1080//p11774 add for fpc1080's spi
static struct gpiomux_setting gpio_blsp8_spi1_config = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_DOWN,
};
static struct gpiomux_setting gpio_blsp8_spi_suspend_config = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};

#endif
#ifdef CONFIG_SKY_DMB_SPI_HW
#if ((defined(T_EF56S) || defined(T_EF57K) || defined(T_EF58L)) && (BOARD_VER > WS20)) || ((defined(T_EF59S) || defined(T_EF59K) || defined(T_EF59L)) && (BOARD_VER >= WS10)) || (defined(T_NAMI) && (BOARD_VER >= PT20)) || (defined(T_EF60S) || defined(T_EF61K) || defined(T_EF62L))
static struct gpiomux_setting gpio_blsp1_spi1_config = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_KEEPER, // 06/14 GPIOMUX_PULL_DOWN -> GPIOMUX_PULL_KEEPER
};
#else // BOARD_VER < TP10
static struct gpiomux_setting gpio_blsp10_spi1_config = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_KEEPER, // 06/14 GPIOMUX_PULL_DOWN -> GPIOMUX_PULL_KEEPER
};
#endif
#endif

static struct gpiomux_setting gpio_suspend_config[] = {
	{
		.func = GPIOMUX_FUNC_GPIO,  /* IN-NP */
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_NONE,
	},
	{
		.func = GPIOMUX_FUNC_GPIO,  /* O-LOW */
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_NONE,
		.dir = GPIOMUX_OUT_LOW,
	},
};

#ifndef FEATURE_PANTECH_USB_REDRIVER_EN_CONTROL
static struct gpiomux_setting gpio_epm_config = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};
#endif

static struct gpiomux_setting wcnss_5wire_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
};

static struct gpiomux_setting wcnss_5wire_active_cfg = {
	.func = GPIOMUX_FUNC_1,
	.drv  = GPIOMUX_DRV_6MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct gpiomux_setting ath_gpio_active_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
};

static struct gpiomux_setting ath_gpio_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct gpiomux_setting gpio_i2c_config = {
	.func = GPIOMUX_FUNC_3,
	/*
	 * Please keep I2C GPIOs drive-strength at minimum (2ma). It is a
	 * workaround for HW issue of glitches caused by rapid GPIO current-
	 * change.
	 */
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};
#if (defined(CONFIG_PN544)&&(defined(CONFIG_MACH_MSM8974_EF56S)||defined(CONFIG_MACH_MSM8974_EF57K))) || (defined(CONFIG_PN547)&& (defined(CONFIG_MACH_MSM8974_EF59S)||defined(CONFIG_MACH_MSM8974_EF59K)||defined(CONFIG_MACH_MSM8974_EF59L)||defined(CONFIG_MACH_MSM8974_EF60S)||defined(CONFIG_MACH_MSM8974_EF61K)||defined(CONFIG_MACH_MSM8974_EF62L)))
#if (BOARD_VER < TP10)
static struct gpiomux_setting gpio_nfc_clk_req_config = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_IN,
};
#endif
static struct gpiomux_setting gpio_nfc_en_active_config = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_KEEPER,
};

static struct gpiomux_setting gpio_nfc_en_suspend_config = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_KEEPER,
};
#endif  /* CONFIG_PN544 -p13815*/
/*
static struct gpiomux_setting lcd_en_act_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_HIGH,
};

static struct gpiomux_setting lcd_en_sus_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};
*/
static struct gpiomux_setting atmel_resout_sus_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct gpiomux_setting atmel_resout_act_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
};

static struct gpiomux_setting atmel_int_act_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
};

static struct gpiomux_setting atmel_int_sus_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct gpiomux_setting taiko_reset = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_6MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_LOW,
};

static struct gpiomux_setting taiko_int = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};
#ifndef FEATURE_PANTECH_PMIC_CHARGER_SMB349
static struct gpiomux_setting hap_lvl_shft_suspended_config = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct gpiomux_setting hap_lvl_shft_active_config = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_UP,
};
static struct msm_gpiomux_config hap_lvl_shft_config[] __initdata = {
	{
		.gpio = 86,
		.settings = {
			[GPIOMUX_SUSPENDED] = &hap_lvl_shft_suspended_config,
			[GPIOMUX_ACTIVE] = &hap_lvl_shft_active_config,
		},
	},
};
#endif

static struct msm_gpiomux_config msm_touch_configs[] __initdata = {
	{
		.gpio      = 60,		/* TOUCH RESET */
		.settings = {
			[GPIOMUX_ACTIVE] = &atmel_resout_act_cfg,
			[GPIOMUX_SUSPENDED] = &atmel_resout_sus_cfg,
		},
	},
	{
		.gpio      = 61,		/* TOUCH IRQ */
		.settings = {
			[GPIOMUX_ACTIVE] = &atmel_int_act_cfg,
			[GPIOMUX_SUSPENDED] = &atmel_int_sus_cfg,
		},
	},

};

static struct gpiomux_setting hsic_sus_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct gpiomux_setting hsic_act_cfg = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_12MA,
	.pull = GPIOMUX_PULL_NONE,
};
#if !defined(CONFIG_PN544) && !defined(CONFIG_PN547)
static struct gpiomux_setting hsic_hub_act_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
	.dir = GPIOMUX_IN,
};
#endif
static struct gpiomux_setting hsic_resume_act_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_OUT_LOW,
};

static struct gpiomux_setting hsic_resume_susp_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct msm_gpiomux_config msm_hsic_configs[] = {
	{
		.gpio = 144,               /*HSIC_STROBE */
		.settings = {
			[GPIOMUX_ACTIVE] = &hsic_act_cfg,
			[GPIOMUX_SUSPENDED] = &hsic_sus_cfg,
		},
	},
	{
		.gpio = 145,               /* HSIC_DATA */
		.settings = {
			[GPIOMUX_ACTIVE] = &hsic_act_cfg,
			[GPIOMUX_SUSPENDED] = &hsic_sus_cfg,
		},
	},
	{
		.gpio = 80,
		.settings = {
			[GPIOMUX_ACTIVE] = &hsic_resume_act_cfg,
			[GPIOMUX_SUSPENDED] = &hsic_resume_susp_cfg,
		},
	},
};
#if !defined(CONFIG_PN544) && !defined(CONFIG_PN547)
static struct msm_gpiomux_config msm_hsic_hub_configs[] = {
	{
		.gpio = 50,               /* HSIC_HUB_INT_N */
		.settings = {
			[GPIOMUX_ACTIVE] = &hsic_hub_act_cfg,
			[GPIOMUX_SUSPENDED] = &hsic_sus_cfg,
		},
	},
};
#endif
#ifdef CONFIG_TOUCHSCREEN_MELFAS_TS // p11223

static struct gpiomux_setting melfas_int_act_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_UP,
};
static struct gpiomux_setting melfas_int_sus_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};
static struct msm_gpiomux_config msm8960_melfas_configs[] __initdata = {
	{	/* melfas INTERRUPT */
		.gpio = 60,
		.settings = {
			[GPIOMUX_ACTIVE]    = &melfas_int_act_cfg,
			[GPIOMUX_SUSPENDED] = &melfas_int_sus_cfg,
		},
	},
};
#endif
//++ p13106 ls2
#if defined (CONFIG_MACH_MSM8974_EF59S) || defined (CONFIG_MACH_MSM8974_EF59K) || defined (CONFIG_MACH_MSM8974_EF59L)
static struct gpiomux_setting touch_pen_act_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
};

static struct gpiomux_setting touch_pen_sus_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
};

static struct msm_gpiomux_config touch_pen_configs[] __initdata = {
	{	/* melfas INTERRUPT */
		.gpio = 79,
		.settings = {
			[GPIOMUX_ACTIVE]    = &touch_pen_act_cfg,
			[GPIOMUX_SUSPENDED] = &touch_pen_sus_cfg,
		},
	},
};

#endif
//-- p13106 ls2


static struct gpiomux_setting mhl_suspend_config = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct gpiomux_setting mhl_active_1_cfg = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
	.dir = GPIOMUX_OUT_HIGH,
};
#if defined(CONFIG_MACH_MSM8974_NAMI)
static struct gpiomux_setting mhl_8244_suspend_config = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};


static struct gpiomux_setting mhl_8244_active_1_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
	.dir = GPIOMUX_OUT_HIGH,
};

static struct gpiomux_setting mhl_8244_active_2_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
	.dir = GPIOMUX_IN,
};

#endif
#if !defined (CONFIG_MACH_MSM8974_EF56S) && !defined (CONFIG_MACH_MSM8974_EF57K) && !defined (CONFIG_MACH_MSM8974_EF58L) && !defined (CONFIG_MACH_MSM8974_EF59S) && !defined (CONFIG_MACH_MSM8974_EF59K) && !defined (CONFIG_MACH_MSM8974_EF59L) && !defined (CONFIG_MACH_MSM8974_EF60S) && !defined (CONFIG_MACH_MSM8974_EF61K) && !defined (CONFIG_MACH_MSM8974_EF62L)
static struct gpiomux_setting hdmi_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct gpiomux_setting hdmi_active_1_cfg = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
};

static struct gpiomux_setting hdmi_active_2_cfg = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_16MA,
	.pull = GPIOMUX_PULL_DOWN,
};
#endif

#ifdef FEATURE_PANTECH_USB_REDRIVER_EN_CONTROL
static struct gpiomux_setting usb3_redriver_enable_active_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
	.dir = GPIOMUX_IN,
};

static struct gpiomux_setting usb3_redriver_enable_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_IN,
};
#endif

static struct msm_gpiomux_config msm_mhl_configs[] __initdata = {
	{
		/* mhl-sii8334 pwr */
		.gpio = 12,
		.settings = {
			[GPIOMUX_SUSPENDED] = &mhl_suspend_config,
			[GPIOMUX_ACTIVE]    = &mhl_active_1_cfg,
		},
	},
	{
		/* mhl-sii8334 intr */
		.gpio = 82,
		.settings = {
			[GPIOMUX_SUSPENDED] = &mhl_suspend_config,
			[GPIOMUX_ACTIVE]    = &mhl_active_1_cfg,
		},
	},
};
#if defined(CONFIG_MACH_MSM8974_NAMI) 

static struct msm_gpiomux_config msm_mhl8240_configs[] __initdata = {
	{
		/* mhl-sii8240 pwr */
		.gpio = 78,
		.settings = {
			[GPIOMUX_ACTIVE]    = &mhl_8244_active_1_cfg,
			[GPIOMUX_SUSPENDED] = &mhl_8244_suspend_config,			
		},
	},
	{
		/* mhl-sii8240 intr */
		.gpio = 75,
		.settings = {
			[GPIOMUX_ACTIVE]    = &mhl_8244_active_2_cfg,
			[GPIOMUX_SUSPENDED] = &mhl_8244_suspend_config,			
		},
	},
	{
		/* mhl-sii8240 Rst */
		.gpio = 76,
		.settings = {
			[GPIOMUX_ACTIVE]    = &mhl_8244_active_1_cfg,
			[GPIOMUX_SUSPENDED] = &mhl_8244_suspend_config,			
		},
	},
	{
		/* mhl-sii8240 MHL USB Switch */
		.gpio = 50,
		.settings = {
			[GPIOMUX_ACTIVE]    = &mhl_8244_active_1_cfg,
			[GPIOMUX_SUSPENDED] = &mhl_8244_suspend_config,			
		},
	},	
};
#endif

#if !defined (CONFIG_MACH_MSM8974_EF56S) && !defined (CONFIG_MACH_MSM8974_EF57K) && !defined (CONFIG_MACH_MSM8974_EF58L) && !defined (CONFIG_MACH_MSM8974_EF59S) && !defined (CONFIG_MACH_MSM8974_EF59K) && !defined (CONFIG_MACH_MSM8974_EF59L) && !defined (CONFIG_MACH_MSM8974_EF60S) && !defined (CONFIG_MACH_MSM8974_EF61K) && !defined (CONFIG_MACH_MSM8974_EF62L)  
#if defined(CONFIG_MACH_MSM8974_NAMI)
static struct msm_gpiomux_config msm_hdmi_configs[] __initdata = {
#if !defined(CONFIG_MACH_MSM8974_NAMI)
	{
		.gpio = 31,
		.settings = {
			[GPIOMUX_ACTIVE]    = &hdmi_active_1_cfg,
			[GPIOMUX_SUSPENDED] = &hdmi_suspend_cfg,
		},
	},
#endif	
	{
		.gpio = 32,
		.settings = {
			[GPIOMUX_ACTIVE]    = &hdmi_active_1_cfg,
			[GPIOMUX_SUSPENDED] = &hdmi_suspend_cfg,
		},
	},
	{
		.gpio = 33,
		.settings = {
			[GPIOMUX_ACTIVE]    = &hdmi_active_1_cfg,
			[GPIOMUX_SUSPENDED] = &hdmi_suspend_cfg,
		},
	},
	{
		.gpio = 34,
		.settings = {
			[GPIOMUX_ACTIVE]    = &hdmi_active_2_cfg,
			[GPIOMUX_SUSPENDED] = &hdmi_suspend_cfg,
		},
	},
};
#endif
#endif
#ifdef CONFIG_F_SKYDISP_SHARP_FHD_VIDEO_COMMON

#if defined (CONFIG_MACH_MSM8974_EF56S) || defined (CONFIG_MACH_MSM8974_EF57K) || defined (CONFIG_MACH_MSM8974_EF58L)
		
static struct gpiomux_setting lcd_rst_active_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_HIGH,		
};
static struct gpiomux_setting lcd_rst_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_OUT_HIGH,	
};
static struct gpiomux_setting lcd_det_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_IN,	
};
static struct msm_gpiomux_config lcd_common_configs[] __initdata = {
	{
		.gpio = 13,
		.settings = {
			[GPIOMUX_ACTIVE]    = &lcd_rst_active_cfg,
			[GPIOMUX_SUSPENDED] = &lcd_rst_suspend_cfg ,
		},
	},
	{
		.gpio = 14,
		.settings = {
			[GPIOMUX_SUSPENDED] = &lcd_det_suspend_cfg,
		},
	},		
};
#if (BOARD_VER == WS20)
static struct gpiomux_setting lcd_vci_i2c_active_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
	.dir = GPIOMUX_OUT_HIGH,	
};
static struct gpiomux_setting lcd_vci_i2c_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_HIGH,	
};
#endif
static struct gpiomux_setting lcd_vcin_en_active_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_HIGH,	
};

static struct gpiomux_setting lcd_vcin_en_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_OUT_HIGH,		
};

static struct gpiomux_setting lcd_vcip_en_active_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_HIGH,	
};
static struct gpiomux_setting lcd_vcip_en_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_OUT_HIGH,		
};

static struct gpiomux_setting lcd_vddio_en_active_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_HIGH,	
};
static struct gpiomux_setting lcd_vddio_en_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_OUT_HIGH,	
};
static struct msm_gpiomux_config lcd_ext_power_configs[] __initdata = {
#if (BOARD_VER == WS20)
	{
		.gpio = 41,
		.settings = {
			[GPIOMUX_ACTIVE]    = &lcd_vci_i2c_active_cfg,
			[GPIOMUX_SUSPENDED] = &lcd_vci_i2c_suspend_cfg,
		},
	},
	{
		.gpio = 42,
		.settings = {
			[GPIOMUX_ACTIVE]    = &lcd_vci_i2c_active_cfg,
			[GPIOMUX_SUSPENDED] = &lcd_vci_i2c_suspend_cfg,
		},
	},
#endif	
	{
		.gpio = 8,
		.settings = {
			[GPIOMUX_ACTIVE] = &lcd_vcin_en_active_cfg,		
			[GPIOMUX_SUSPENDED] = &lcd_vcin_en_suspend_cfg,
		},
	},	
	{
		.gpio = 34,
		.settings = {
			[GPIOMUX_ACTIVE]    = &lcd_vcip_en_active_cfg,
			[GPIOMUX_SUSPENDED] = &lcd_vcip_en_suspend_cfg,
		},
	},		
	{
		.gpio = 12,
		.settings = {
			[GPIOMUX_ACTIVE]    = &lcd_vddio_en_active_cfg,
			[GPIOMUX_SUSPENDED] = &lcd_vddio_en_suspend_cfg,
		},
	},		
};

static struct gpiomux_setting lcd_bl_en_active_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_HIGH,	
};
static struct gpiomux_setting lcd_bl_swire_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_HIGH,		
};
static struct msm_gpiomux_config lcd_bl_configs[] __initdata = {
	{
		.gpio = 57,
		.settings = {
			[GPIOMUX_ACTIVE]    = &lcd_bl_en_active_cfg,
		},
	},	
	{
		.gpio = 89,
		.settings = {
			[GPIOMUX_SUSPENDED] = &lcd_bl_swire_suspend_cfg,
		},
	},		
};
#elif defined (CONFIG_MACH_MSM8974_EF59S) || defined (CONFIG_MACH_MSM8974_EF59K) || defined (CONFIG_MACH_MSM8974_EF59L) || (defined (CONFIG_MACH_MSM8974_EF60S) && (BOARD_VER == PT10)) || (defined (CONFIG_MACH_MSM8974_EF61K) && (BOARD_VER == PT10)) || (defined (CONFIG_MACH_MSM8974_EF62L) && (BOARD_VER == PT10))    
static struct gpiomux_setting lcd_rst_active_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_HIGH,		
};
static struct gpiomux_setting lcd_rst_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_OUT_HIGH,	
};
static struct gpiomux_setting lcd_det_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_IN,	
};
static struct msm_gpiomux_config lcd_common_configs[] __initdata = {
	{
		.gpio = 13,
		.settings = {
			[GPIOMUX_ACTIVE]    = &lcd_rst_active_cfg,
			[GPIOMUX_SUSPENDED] = &lcd_rst_suspend_cfg ,
		},
	},
	{
		.gpio = 12,
		.settings = {
			[GPIOMUX_SUSPENDED] = &lcd_det_suspend_cfg,
		},
	},		
};

static struct gpiomux_setting lcd_vcip_en_active_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_HIGH,	
};
static struct gpiomux_setting lcd_vcip_en_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_OUT_HIGH,		
};

static struct gpiomux_setting lcd_vddio_en_active_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_HIGH,	
};
static struct gpiomux_setting lcd_vddio_en_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_OUT_HIGH,	
};
static struct msm_gpiomux_config lcd_ext_power_configs[] __initdata = {
	{
		.gpio = 34,
		.settings = {
			[GPIOMUX_ACTIVE]    = &lcd_vcip_en_active_cfg,
			[GPIOMUX_SUSPENDED] = &lcd_vcip_en_suspend_cfg,
		},
	},		
	{
		.gpio = 14,
		.settings = {
			[GPIOMUX_ACTIVE]    = &lcd_vddio_en_active_cfg,
			[GPIOMUX_SUSPENDED] = &lcd_vddio_en_suspend_cfg,
		},
	},		
};

static struct gpiomux_setting lcd_bl_en_active_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_HIGH,	
};
static struct gpiomux_setting lcd_bl_swire_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_HIGH,		
};
static struct msm_gpiomux_config lcd_bl_configs[] __initdata = {
	{
		.gpio = 57,
		.settings = {
			[GPIOMUX_ACTIVE]    = &lcd_bl_en_active_cfg,
		},
	},	
	{
		.gpio = 69,
		.settings = {
			[GPIOMUX_SUSPENDED] = &lcd_bl_swire_suspend_cfg,
		},
	},		
};
#elif (defined (CONFIG_MACH_MSM8974_EF60S) && (BOARD_VER >= WS10)) || (defined (CONFIG_MACH_MSM8974_EF61K) && (BOARD_VER >= WS10)) || (defined (CONFIG_MACH_MSM8974_EF62L) && (BOARD_VER >= WS10))
static struct gpiomux_setting lcd_rst_active_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_HIGH,		
};
static struct gpiomux_setting lcd_rst_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_OUT_HIGH,	
};
static struct gpiomux_setting lcd_det_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_IN,	
};
static struct msm_gpiomux_config lcd_common_configs[] __initdata = {
	{
		.gpio = 13,
		.settings = {
			[GPIOMUX_ACTIVE]    = &lcd_rst_active_cfg,
			[GPIOMUX_SUSPENDED] = &lcd_rst_suspend_cfg ,
		},
	},
	{
		.gpio = 12,
		.settings = {
			[GPIOMUX_SUSPENDED] = &lcd_det_suspend_cfg,
		},
	},		
};
static struct gpiomux_setting lcd_vcin_en_active_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_HIGH,	
};

static struct gpiomux_setting lcd_vcin_en_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_OUT_HIGH,		
};

static struct gpiomux_setting lcd_vcip_en_active_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_HIGH,	
};
static struct gpiomux_setting lcd_vcip_en_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_OUT_HIGH,		
};

static struct gpiomux_setting lcd_vddio_en_active_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_HIGH,	
};
static struct gpiomux_setting lcd_vddio_en_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_OUT_HIGH,	
};
static struct msm_gpiomux_config lcd_ext_power_configs[] __initdata = {
	
	{
		.gpio = 8,
		.settings = {
			[GPIOMUX_ACTIVE] = &lcd_vcin_en_active_cfg,		
			[GPIOMUX_SUSPENDED] = &lcd_vcin_en_suspend_cfg,
		},
	},	
	{
		.gpio = 34,
		.settings = {
			[GPIOMUX_ACTIVE]    = &lcd_vcip_en_active_cfg,
			[GPIOMUX_SUSPENDED] = &lcd_vcip_en_suspend_cfg,
		},
	},		
	{
		.gpio = 14,
		.settings = {
			[GPIOMUX_ACTIVE]    = &lcd_vddio_en_active_cfg,
			[GPIOMUX_SUSPENDED] = &lcd_vddio_en_suspend_cfg,
		},
	},		
};

static struct gpiomux_setting lcd_bl_en_active_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_HIGH,	
};

static struct msm_gpiomux_config lcd_bl_configs[] __initdata = {
	{
		.gpio = 57,
		.settings = {
			[GPIOMUX_ACTIVE]    = &lcd_bl_en_active_cfg,
		},
	},	
	
};
#endif
#elif defined  (CONFIG_F_SKYDISP_SHARP_FHD_CMD_COMMON)
static struct gpiomux_setting lcd_rst_active_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_HIGH,		
};
static struct gpiomux_setting lcd_rst_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_OUT_HIGH,	
};

static struct msm_gpiomux_config lcd_common_configs[] __initdata = {
	{
		.gpio = 13,
		.settings = {
			[GPIOMUX_ACTIVE]    = &lcd_rst_active_cfg,
			[GPIOMUX_SUSPENDED] = &lcd_rst_suspend_cfg ,
		},
	},	
};

static struct gpiomux_setting lcd_vci_i2c_active_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_UP,
	.dir = GPIOMUX_OUT_HIGH,	
};
static struct gpiomux_setting lcd_vci_i2c_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_IN,	
};

static struct gpiomux_setting lcd_vcin_en_active_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_HIGH,	
};
static struct gpiomux_setting lcd_vcin_en_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_OUT_HIGH,		
};

static struct gpiomux_setting lcd_vcip_en_active_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_HIGH,	
};
static struct gpiomux_setting lcd_vcip_en_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_OUT_HIGH,		
};

static struct gpiomux_setting lcd_vddio_en_active_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_HIGH,	
};
static struct gpiomux_setting lcd_vddio_en_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_OUT_HIGH,	
};
static struct msm_gpiomux_config lcd_ext_power_configs[] __initdata = {

	{
		.gpio = 41,
		.settings = {
			[GPIOMUX_ACTIVE]    = &lcd_vci_i2c_active_cfg,
			[GPIOMUX_SUSPENDED] = &lcd_vci_i2c_suspend_cfg,
		},
	},
	{
		.gpio = 42,
		.settings = {
			[GPIOMUX_ACTIVE]    = &lcd_vci_i2c_active_cfg,
			[GPIOMUX_SUSPENDED] = &lcd_vci_i2c_suspend_cfg,
		},
	},
	{
		.gpio = 14,
		.settings = {
			[GPIOMUX_ACTIVE] = &lcd_vcin_en_active_cfg,				
			[GPIOMUX_SUSPENDED] = &lcd_vcin_en_suspend_cfg,
		},
	},	
	{
		.gpio = 69,
		.settings = {
			[GPIOMUX_ACTIVE]    = &lcd_vcip_en_active_cfg,
			[GPIOMUX_SUSPENDED] = &lcd_vcip_en_suspend_cfg,
		},
	},		
	{
		.gpio = 65,
		.settings = {
			[GPIOMUX_ACTIVE]    = &lcd_vddio_en_active_cfg,
			[GPIOMUX_SUSPENDED] = &lcd_vddio_en_suspend_cfg,
		},
	},		
};

static struct gpiomux_setting lcd_bl_en_active_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_HIGH,	
};

static struct msm_gpiomux_config lcd_bl_configs[] __initdata = {
	{
		.gpio = 57,
		.settings = {
			[GPIOMUX_ACTIVE]    = &lcd_bl_en_active_cfg,
		},
	},	
		
};
#endif

static struct gpiomux_setting gpio_uart7_active_cfg = {
	.func = GPIOMUX_FUNC_3,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting gpio_uart7_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};

#ifdef CONFIG_CXD2235AGG_NFC_FELICA
static struct gpiomux_setting gpio_uart6_tx_cfg = {
	.func = GPIOMUX_FUNC_4,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
};
static struct gpiomux_setting gpio_uart6_rx_cfg = {
	.func = GPIOMUX_FUNC_3,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
};
#endif

static struct msm_gpiomux_config msm_blsp2_uart7_configs[] __initdata = {
#if (BOARD_VER != WS20)	
	{
		.gpio	= 41,	/* BLSP2 UART7 TX */
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_uart7_active_cfg,
			[GPIOMUX_SUSPENDED] = &gpio_uart7_suspend_cfg,
		},
	},
	{
		.gpio	= 42,	/* BLSP2 UART7 RX */
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_uart7_active_cfg,
			[GPIOMUX_SUSPENDED] = &gpio_uart7_suspend_cfg,
		},
	},
#endif	
	{
		.gpio	= 43,	/* BLSP2 UART7 CTS */
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_uart7_active_cfg,
			[GPIOMUX_SUSPENDED] = &gpio_uart7_suspend_cfg,
		},
	},
	{
		.gpio	= 44,	/* BLSP2 UART7 RFR */
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_uart7_active_cfg,
			[GPIOMUX_SUSPENDED] = &gpio_uart7_suspend_cfg,
		},
	},
};
#if !defined(CONFIG_INPUT_FPC1080)//p11774 blocked for fpc1080's spi
static struct msm_gpiomux_config msm_rumi_blsp_configs[] __initdata = {
	{
		.gpio      = 45,	/* BLSP2 UART8 TX */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_uart_config,
		},
	},
	{
		.gpio      = 46,	/* BLSP2 UART8 RX */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_uart_config,
		},
	},
};
#endif

#ifdef CONFIG_MACH_MSM8974_NAMI
///LM3530
static struct gpiomux_setting blsp8_i2c_config = {
	.func = GPIOMUX_FUNC_3,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};
static struct msm_gpiomux_config lm3530_blsp_configs[] __initdata = {
	{
		.gpio      = 47,	/* BLSP2 QUP3 I2C_DAT */
		.settings = {			
			[GPIOMUX_SUSPENDED] = &blsp8_i2c_config,
		},
	},
	{
		.gpio      = 48,	/* BLSP2 QUP3 I2C_CLK */
		.settings = {			
			[GPIOMUX_SUSPENDED] = &blsp8_i2c_config,
		},
	},
};

#endif
#if defined(CONFIG_FB_MSM_MDSS_HDMI_MHL_SII8240)
static struct gpiomux_setting blsp9_i2c_config = {
	.func = GPIOMUX_FUNC_4,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};
static struct msm_gpiomux_config mhl_sii_blsp_configs[] __initdata = {
	{
		.gpio      = 51,	/* BLSP2 QUP3 I2C_DAT */
		.settings = {			
			[GPIOMUX_SUSPENDED] = &blsp9_i2c_config,
		},
	},
	{
		.gpio      = 52,	/* BLSP2 QUP3 I2C_CLK */
		.settings = {			
			[GPIOMUX_SUSPENDED] = &blsp9_i2c_config,
		},
	},
};
#endif
//++ p11309 - 2013.04.25 for TI, LP5523 LED Driver IC
#if defined(CONFIG_LEDS_LP5523) 

static struct gpiomux_setting blsp9_i2c_config = {
	.func = GPIOMUX_FUNC_4,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting lp5523_led_en_act_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,	
};

static struct gpiomux_setting lp5523_led_en_sus_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct msm_gpiomux_config lp5523_led_configs[] __initdata = {
	{
		.gpio = 16,
		.settings = {
			[GPIOMUX_ACTIVE]    = &lp5523_led_en_act_cfg,
			[GPIOMUX_SUSPENDED] = &lp5523_led_en_act_cfg,
		},
	},
//++ p11309 - 2013.05.21 not used port.
	{
		.gpio = 28,
		.settings = {
			[GPIOMUX_ACTIVE]    = &lp5523_led_en_sus_cfg,
			[GPIOMUX_SUSPENDED] = &lp5523_led_en_sus_cfg,
		},
	},
//-- p11309
	{
		.gpio      = 51,	/* BLSP2 QUP3 I2C_DAT */
		.settings = {			
			[GPIOMUX_SUSPENDED] = &blsp9_i2c_config,
		},
	},
	{
		.gpio      = 52,	/* BLSP2 QUP3 I2C_CLK */
		.settings = {			
			[GPIOMUX_SUSPENDED] = &blsp9_i2c_config,
		},
	},
};
#endif
//-- p11309

/*
static struct msm_gpiomux_config msm_lcd_configs[] __initdata = {
	{
		.gpio = 58,
		.settings = {
			[GPIOMUX_ACTIVE]    = &lcd_en_act_cfg,
			[GPIOMUX_SUSPENDED] = &lcd_en_sus_cfg,
		},
	},
};
*/
#if defined(CONFIG_PANTECH_PMIC_FUELGAUGE_MAX17058)  /* actuator qup */
#if defined (T_EF56S) || defined(T_EF57K) || defined(T_EF58L) || defined(T_EF59S) || defined(T_EF59K) || defined(T_EF59L) || defined(T_EF60S) || defined(T_EF61K) || defined(T_EF62L)       /* MKS. */
static struct gpiomux_setting gpio_sc_en_susp_config = {	// GPIO_4
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_IN,
};
static struct gpiomux_setting gpio_sc_sysok_susp_config = {	// GPIO_5
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_IN,
};
static struct gpiomux_setting gpio_fuelalrt_n_susp_cfg = {	// GPIO_27
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
	.dir = GPIOMUX_IN,
};
#if defined (CONFIG_MACH_MSM8974_EF60S) || defined (CONFIG_MACH_MSM8974_EF61K) || defined (CONFIG_MACH_MSM8974_EF62L)
static struct gpiomux_setting piezo_en_active_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	//.pull = GPIOMUX_PULL_NONE, //p12911
	.pull = GPIOMUX_PULL_UP,	
	.dir = GPIOMUX_OUT_HIGH,	
};
#endif
static struct gpiomux_setting gpio_sc_susp_susp_cfg = {	// GPIO_78
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
	.dir = GPIOMUX_OUT_HIGH,
};
static struct gpiomux_setting gpio_sc_stat_susp_cfg = {	// GPIO_85
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_IN,
};
#endif

#if ((defined (T_EF56S) || defined(T_EF57K) || defined(T_EF58L)) && (BOARD_VER > WS20)) || defined(T_EF59S) || defined(T_EF59K) || defined(T_EF59L) || defined(T_EF60S) || defined(T_EF61K) || defined(T_EF62L) /*|| defined (T_NAMI)*/
static struct gpiomux_setting gpio_i2c_sda_config = {
	.func = GPIOMUX_FUNC_4,
	/*
	 * Please keep I2C GPIOs drive-strength at minimum (2ma). It is a
	 * workaround for HW issue of glitches caused by rapid GPIO current-
	 * change.
	 */
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
};

static struct gpiomux_setting gpio_i2c_scl_config = {
	.func = GPIOMUX_FUNC_5,
	/*
	 * Please keep I2C GPIOs drive-strength at minimum (2ma). It is a
	 * workaround for HW issue of glitches caused by rapid GPIO current-
	 * change.
	 */
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
};
#endif
#endif

#if ((defined (T_EF56S) || defined(T_EF57K) || defined(T_EF58L)) && (BOARD_VER < WS20)) && (defined(CONFIG_PANTECH_CAMERA))
static struct gpiomux_setting gpio_i2c_sda_config = {
	.func = GPIOMUX_FUNC_4,
	/*
	 * Please keep I2C GPIOs drive-strength at minimum (2ma). It is a
	 * workaround for HW issue of glitches caused by rapid GPIO current-
	 * change.
	 */
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting gpio_i2c_scl_config = {
	.func = GPIOMUX_FUNC_5,
	/*
	 * Please keep I2C GPIOs drive-strength at minimum (2ma). It is a
	 * workaround for HW issue of glitches caused by rapid GPIO current-
	 * change.
	 */
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};
#endif

static struct msm_gpiomux_config msm_blsp_configs[] __initdata = {
#if defined(CONFIG_KS8851) || defined(CONFIG_KS8851_MODULE)
#if !defined(FEATURE_PANTECH_CONSOLE_UART1) && !defined(CONFIG_SKY_DMB_SPI_HW) /* p13250 jaegyu.jun */
	{
		.gpio      = 0, 		/* BLSP1 QUP SPI_DATA_MOSI */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_spi_config,
		},
	},
	{
		.gpio      = 1,		/* BLSP1 QUP SPI_DATA_MISO */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_spi_config,
		},
	},
#endif
#if !defined(CONFIG_PANTECH_PMIC_FUELGAUGE_MAX17058) && !defined(CONFIG_SKY_DMB_SPI_HW) /* sayuss */ /* p13250 jaegyu.jun */
	{
		.gpio      = 3,		/* BLSP1 QUP SPI_CLK */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_spi_config,
		},
	},
#endif
	{
		.gpio      = 9,		/* BLSP1 QUP SPI_CS2A_N */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_spi_cs2_config,
		},
	},
	{
		.gpio      = 8,		/* BLSP1 QUP SPI_CS1_N */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_spi_cs1_config,
		},
	},
#endif
#if defined(CONFIG_PANTECH_PMIC_FUELGAUGE_MAX17058)  /* sayuss */
#if defined(T_EF56S) || defined(T_EF57K) || defined(T_EF58L) 
	{
		.gpio      = 4,			/* SC_EN */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_sc_en_susp_config,
		},
	},
	{
		.gpio      = 5,			/* SC_SYSOK */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_sc_sysok_susp_config,
		},
	},
	{
		.gpio     = 27,			/* FUEL_ALRT_N */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_fuelalrt_n_susp_cfg,
		},
	},
#if (BOARD_VER <= WS20)
	{
		.gpio     = 78,			/* SC_SUSP */
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_sc_susp_susp_cfg,
			[GPIOMUX_SUSPENDED] = &gpio_sc_susp_susp_cfg,
		},
	},
	{
		.gpio     = 85,			/* SC_STAT */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_sc_stat_susp_cfg,
		},
	},
#else 
    {
		.gpio     = 31,			/* SC_SUSP */
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_sc_susp_susp_cfg,
			[GPIOMUX_SUSPENDED] = &gpio_sc_susp_susp_cfg,
		},
	},
	{
		.gpio     = 86,			/* SC_STAT */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_sc_stat_susp_cfg,
		},
	},	
#endif
#elif defined(T_EF59S) || defined(T_EF59K) || defined(T_EF59L) || defined(T_EF60S) || defined(T_EF61K) || defined(T_EF62L)
	{
		.gpio      = 4,			/* SC_EN */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_sc_en_susp_config,
		},
	},
	{
		.gpio      = 5,			/* SC_SYSOK */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_sc_sysok_susp_config,
		},
	},
	{
		.gpio     = 27,			/* FUEL_ALRT_N */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_fuelalrt_n_susp_cfg,
		},
	},
	{
		.gpio     = 31,			/* SC_SUSP */
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_sc_susp_susp_cfg,
			[GPIOMUX_SUSPENDED] = &gpio_sc_susp_susp_cfg,
		},
	},
	{
		.gpio     = 86,			/* SC_STAT */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_sc_stat_susp_cfg,
		},
	},
#endif
#if (defined(T_EF56S) || defined(T_EF57K) || defined(T_EF58L)) && (BOARD_VER <= WS20)
	{
		.gpio      = 2,         /* BLSP1 QUP2 I2C_DAT */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_i2c_config,
		},
	},
	{
		.gpio      = 3,         /* BLSP1 QUP2 I2C_CLK */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_i2c_config,
		},
	},
#elif ((defined(T_EF56S) || defined(T_EF57K) || defined(T_EF58L)) && (BOARD_VER > WS20)) || defined(T_EF59S) || defined(T_EF59K) || defined(T_EF59L) || defined(T_EF60S) || defined(T_EF61K) || defined(T_EF62L) /* 2013.05.23 sayuss change i2c port */ 
        {
                .gpio = 25, /* AF_I2C_SDA1 */
                .settings = {
                        [GPIOMUX_SUSPENDED] = &gpio_i2c_sda_config, //
                },
        },
        {
                .gpio = 26, /* AF_I2C_SCL1 */
                .settings = {
                        [GPIOMUX_SUSPENDED] = &gpio_i2c_scl_config, //
                },
        },

#endif
#endif
	{
		.gpio      = 6,		/* BLSP1 QUP2 I2C_DAT */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_i2c_config,
		},
	},
	{
		.gpio      = 7,		/* BLSP1 QUP2 I2C_CLK */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_i2c_config,
		},
	},
#ifdef CONFIG_PANTECH_CAMERA    /* actuator qup */
#if (defined(T_EF56S) || defined(T_EF57K) || defined(T_EF58L)) && (BOARD_VER < WS20)
	{
		.gpio = 25, /* AF_I2C_SDA1 */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_i2c_sda_config, //
		},
	},
	{
		.gpio = 26, /* AF_I2C_SCL1 */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_i2c_scl_config, //
		},
	},	
#endif
#endif

#ifdef CONFIG_CXD2235AGG_NFC_FELICA
#if !defined(T_EF56S) || (BOARD_VER < TP10)
	{
		.gpio     = 27,
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_uart6_tx_cfg,
		},
	},
#endif
	{
		.gpio     = 28,
		.settings = {
			[GPIOMUX_SUSPENDED]= &gpio_uart6_rx_cfg,
		},
	},
#endif /*CONFIG_CXD2235AGG_NFC_FELICA*/

#if defined(CONFIG_PN544) || defined(CONFIG_AK6921AF) || defined(CONFIG_PN547)
	{
		.gpio	   = 29, 	/* BLSP1 QUP5(BLSP6) I2C_DAT */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_i2c_config,
		},
	},
	{
		.gpio	   = 30, 	/* BLSP1 QUP5(BLSP6)  I2C_CLK */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_i2c_config,
		},
	},
#if defined(CONFIG_MACH_MSM8974_EF56S)||defined(CONFIG_MACH_MSM8974_EF57K) || defined(CONFIG_MACH_MSM8974_EF59S)||defined(CONFIG_MACH_MSM8974_EF59K)||defined(CONFIG_MACH_MSM8974_EF59L)||defined(CONFIG_MACH_MSM8974_EF60S)||defined(CONFIG_MACH_MSM8974_EF61K)||defined(CONFIG_MACH_MSM8974_EF62L)
	{
		.gpio	   = 50, 	/* NFC Enable pin*/
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_nfc_en_active_config,
			[GPIOMUX_SUSPENDED] = &gpio_nfc_en_suspend_config,
		},
	},	
#if (BOARD_VER < TP10)		
	{
		.gpio	   = 100,	/* NFC_CLK_REQ suspend*/
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_nfc_clk_req_config,
		},
	},
#endif
#endif
#endif	/* CONFIG_PN544 */
#ifdef CONFIG_INPUT_FPC1080//p11774 add for fpc1080's spi
	{
		.gpio      = 45,		/* BLSP2 QUP2 SPI MOSI */
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_blsp8_spi1_config,
			[GPIOMUX_SUSPENDED] = &gpio_blsp8_spi_suspend_config,
		},
	},
	{
		.gpio      = 46,		/* BLSP2 QUP2 SPI MISO */
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_blsp8_spi1_config,
			[GPIOMUX_SUSPENDED] = &gpio_blsp8_spi_suspend_config,
		},
	},
	{
		.gpio      = 47,		/* BLSP2 QUP2 SPI CS */
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_blsp8_spi1_config,
			[GPIOMUX_SUSPENDED] = &gpio_blsp8_spi_suspend_config,
		},
	},
	{
		.gpio      = 48,		/* BLSP2 QUP2 SPI CLK */
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_blsp8_spi1_config,
			[GPIOMUX_SUSPENDED] = &gpio_blsp8_spi_suspend_config,
		},
	},
#endif
	{
		.gpio      = 83,		/* BLSP11 QUP I2C_DAT */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_i2c_config,
		},
	},
	{
		.gpio      = 84,		/* BLSP11 QUP I2C_CLK */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_i2c_config,
		},
	},
#if defined(FEATURE_PANTECH_CONSOLE_UART1)
	{
                .gpio      = 0,                 /* BLSP2 UART TX */
                .settings = {
                        [GPIOMUX_SUSPENDED] = &gpio_uart_config,
                },
        },
        {
                .gpio      = 1,                 /* BLSP2 UART RX */
                .settings = {
                        [GPIOMUX_SUSPENDED] = &gpio_uart_config,
                },
        },
#elif defined(FEATURE_PANTECH_CONSOLE_UART10)
	{
		.gpio      = 53,		/* BLSP2 UART TX */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_uart_config,
		},
	},
	{
		.gpio      = 54,		/* BLSP2 UART RX */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_uart_config,
		},
	},
#else
	{
		.gpio      = 4,			/* BLSP2 UART TX */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_uart_config,
		},
	},
	{
		.gpio      = 5,			/* BLSP2 UART RX */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_uart_config,
		},
	},
#endif
#ifdef CONFIG_SKY_DMB_SPI_HW
#if ((defined(T_EF56S) || defined(T_EF57K) || defined(T_EF58L)) && (BOARD_VER > WS20)) || ((defined(T_EF59S) || defined(T_EF59K) || defined(T_EF59L)) && (BOARD_VER >= WS10)) || (defined(T_NAMI) && (BOARD_VER >= PT20)) || (defined(T_EF60S) || defined(T_EF61K) || defined(T_EF62L))
	{
		.gpio      = 0,		/* BLSP1 QUP0 SPI_DATA_MOSI */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_blsp1_spi1_config,
		},
	},
	{
		.gpio      = 1,		/* BLSP1 QUP0 SPI_DATA_MISO */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_blsp1_spi1_config,
		},
	},
	{
		.gpio      = 3,		/* BLSP1 QUP0 SPI_CLK */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_blsp1_spi1_config,
		},
	},
	{
		.gpio      = 2,		/* BLSP1 QUP0 SPI_CS0_N */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_blsp1_spi1_config,
		},
	},
#else // BOARD_VER <= WS20
	{
		.gpio      = 53,		/* BLSP2 QUP4 SPI_DATA_MOSI */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_blsp10_spi1_config,
		},
	},
	{
		.gpio      = 54,		/* BLSP2 QUP4 SPI_DATA_MISO */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_blsp10_spi1_config,
		},
	},
	{
		.gpio      = 56,		/* BLSP2 QUP4 SPI_CLK */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_blsp10_spi1_config,
		},
	},
	{
		.gpio      = 55,		/* BLSP2 QUP4 SPI_CS0_N */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_blsp10_spi1_config,
		},
	},
#endif
#endif /* CONFIG_SKY_DMB_SPI_HW */
#if defined(CONFIG_KS8851) || defined(CONFIG_KS8851_MODULE)
#ifndef CONFIG_SKY_DMB_SPI_HW
#if !defined(FEATURE_PANTECH_CONSOLE_UART10)
	{
		.gpio      = 53,		/* BLSP2 QUP4 SPI_DATA_MOSI */
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_spi_config,
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[1],
		},
	},
	{
		.gpio      = 54,		/* BLSP2 QUP4 SPI_DATA_MISO */
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_spi_config,
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[1],
		},
	},
	{
		.gpio      = 56,		/* BLSP2 QUP4 SPI_CLK */
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_spi_config,
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[0],
		},
	},
	{
		.gpio      = 55,		/* BLSP2 QUP4 SPI_CS0_N */
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_spi_config,
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[0],
		},
	},
#endif	
#endif	
#endif	
#ifndef FEATURE_PANTECH_USB_REDRIVER_EN_CONTROL
	{
		.gpio      = 81,		/*  EPM enable */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_epm_config,
		},
	},
#endif

#if defined (CONFIG_MACH_MSM8974_EF60S) || defined (CONFIG_MACH_MSM8974_EF61K) || defined (CONFIG_MACH_MSM8974_EF62L)
	{
		.gpio = 68,		
		.settings = {
			[GPIOMUX_ACTIVE] = &piezo_en_active_cfg,
//			[GPIOMUX_SUSPENDED] = &gpio_fuelalrt_n_susp_cfg,
			[GPIOMUX_SUSPENDED] = &piezo_en_active_cfg,			
		},
	},
#endif

};

#ifdef FEATURE_PANTECH_USB_REDRIVER_EN_CONTROL
static struct msm_gpiomux_config msm8974_usb3_redriveric_configs[] __initdata = {
	{
		.gpio      = 81,		/* USB3.0 redriver IC  enable */
		.settings = {
			[GPIOMUX_ACTIVE] = &usb3_redriver_enable_active_cfg,
			[GPIOMUX_SUSPENDED] = &usb3_redriver_enable_suspend_cfg,
		},
	},
};
#endif

static struct msm_gpiomux_config msm8974_slimbus_config[] __initdata = {
	{
		.gpio	= 70,		/* slimbus clk */
		.settings = {
			[GPIOMUX_SUSPENDED] = &slimbus,
		},
	},
	{
		.gpio	= 71,		/* slimbus data */
		.settings = {
			[GPIOMUX_SUSPENDED] = &slimbus,
		},
	},
};

static struct gpiomux_setting cam_settings[] = {
	{
		.func = GPIOMUX_FUNC_1, /*active 1*/ /* 0 */
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_NONE,
	},

	{
		.func = GPIOMUX_FUNC_1, /*suspend*/ /* 1 */
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_DOWN,
	},

	{
		.func = GPIOMUX_FUNC_1, /*i2c suspend*/ /* 2 */
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_KEEPER,
	},

	{
		.func = GPIOMUX_FUNC_GPIO, /*active 0*/ /* 3 */
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_NONE,
	},

	{
		.func = GPIOMUX_FUNC_GPIO, /*suspend 0*/ /* 4 */
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_DOWN,
	},
};

#if defined(CONFIG_MACH_MSM8974_CSFB) || defined(CONFIG_MACH_MSM8974_SVLTE)
static struct gpiomux_setting lcd2_2volt_settings[] = {
	{
		.func = GPIOMUX_FUNC_GPIO, /*active 1*/
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_NONE,
		.dir = GPIOMUX_OUT_HIGH,
	},

	{
		.func = GPIOMUX_FUNC_GPIO, /*suspend*/
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_DOWN,
		.dir = GPIOMUX_OUT_LOW,
	},
};

static struct msm_gpiomux_config msm_lcd2_2_voltage_configs[] __initdata = {
	{
		.gpio = 26, /* CAM_IRQ */
		.settings = {
			[GPIOMUX_ACTIVE]    = &lcd2_2volt_settings[0],
			[GPIOMUX_SUSPENDED] = &lcd2_2volt_settings[1],
		},
	},
};
#endif


static struct gpiomux_setting sd_card_det_active_config = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_IN,
};

static struct gpiomux_setting sd_card_det_sleep_config = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
	.dir = GPIOMUX_IN,
};

static struct msm_gpiomux_config sd_card_det __initdata = {
	.gpio = 62,
	.settings = {
		[GPIOMUX_ACTIVE]    = &sd_card_det_active_config,
		[GPIOMUX_SUSPENDED] = &sd_card_det_sleep_config,
	},
};

static struct msm_gpiomux_config msm_sensor_configs[] __initdata = {
#ifdef CONFIG_PANTECH_CAMERA
#if defined(T_EF56S) || defined(T_EF57K) || defined(T_EF58L) || defined(T_EF59S) || defined(T_EF59K) || defined(T_EF59L) || defined(T_EF60S) || defined(T_EF61K) || defined(T_EF62L)
	{
		.gpio = 15, /* CAM_MCLK0 */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[0],
			[GPIOMUX_SUSPENDED] = &cam_settings[1],
		},
	},
	{
		.gpio = 17, /* CAM_MCLK2 */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[0],
			[GPIOMUX_SUSPENDED] = &cam_settings[1],
		},
	},
	{
		.gpio = 18, /* FRONT RESET */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[3],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[1],
		},
	},
	{
		.gpio = 19, /* CCI_I2C_SDA0 */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[0],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[0],
		},
	},
	{
		.gpio = 20, /* CCI_I2C_SCL0 */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[0],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[0],
		},
	},
	{
		.gpio = 21, /* CCI_I2C_SDA1 */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[0],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[0],
		},
	},
	{
		.gpio = 22, /* CCI_I2C_SCL1 */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[0],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[0],
		},
	},	
	{
		.gpio = 24, /* FLASH_LED_NOW */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[0],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[1],
		},
	},
#if (BOARD_VER < WS10)
	{
		.gpio = 81, /* FRONT STANDBY */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[3],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[1],
		},
	},
#endif
	{
		.gpio = 90, /* REAR RESET */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[3],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[1],
		},
	},	
	{
		.gpio = 96, /* REAR WP */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[3],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[1],
		},
	},	
#elif defined(T_NAMI)
	{
		.gpio = 15, /* CAM_MCLK0 */
		.settings = {
			[GPIOMUX_ACTIVE]	= &cam_settings[0],
			[GPIOMUX_SUSPENDED] = &cam_settings[1],
		},
	},
	{
		.gpio = 17, /* CAM_MCLK2 */
		.settings = {
			[GPIOMUX_ACTIVE]	= &cam_settings[0],
			[GPIOMUX_SUSPENDED] = &cam_settings[1],
		},
	},
	{
		.gpio = 18, /* FRONT RESET */
		.settings = {
			[GPIOMUX_ACTIVE]	= &cam_settings[3],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[1],
		},
	},
	{
		.gpio = 19, /* CCI_I2C_SDA0 */
		.settings = {
			[GPIOMUX_ACTIVE]	= &cam_settings[0],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[0],
		},
	},
	{
		.gpio = 20, /* CCI_I2C_SCL0 */
		.settings = {
			[GPIOMUX_ACTIVE]	= &cam_settings[0],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[0],
		},
	},

#endif
#else
	{
		.gpio = 15, /* CAM_MCLK0 */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[0],
			[GPIOMUX_SUSPENDED] = &cam_settings[1],
		},
	},
	{
		.gpio = 16, /* CAM_MCLK1 */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[0],
			[GPIOMUX_SUSPENDED] = &cam_settings[1],
		},
	},
	{
		.gpio = 17, /* CAM_MCLK2 */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[0],
			[GPIOMUX_SUSPENDED] = &cam_settings[1],
		},
	},
	{
		.gpio = 18, /* WEBCAM1_RESET_N / CAM_MCLK3 */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[3],
			[GPIOMUX_SUSPENDED] = &cam_settings[4],
		},
	},
	{
		.gpio = 19, /* CCI_I2C_SDA0 */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[0],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[0],
		},
	},
	{
		.gpio = 20, /* CCI_I2C_SCL0 */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[0],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[0],
		},
	},
	{
		.gpio = 21, /* CCI_I2C_SDA1 */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[0],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[0],
		},
	},
	{
		.gpio = 22, /* CCI_I2C_SCL1 */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[0],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[0],
		},
	},
	{
		.gpio = 23, /* FLASH_LED_EN */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[0],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[1],
		},
	},
	{
		.gpio = 24, /* FLASH_LED_NOW */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[0],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[1],
		},
	},
	{
		.gpio = 25, /* WEBCAM2_RESET_N */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[3],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[1],
		},
	},
#if 0
	{
		.gpio = 26, /* CAM_IRQ */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[0],
			[GPIOMUX_SUSPENDED] = &cam_settings[1],
		},
	},
#endif
	{
		.gpio = 27, /* OIS_SYNC */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[0],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[1],
		},
	},
	{
		.gpio = 28, /* WEBCAM1_STANDBY */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[3],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[1],
		},
	},
	{
		.gpio = 89, /* CAM1_STANDBY_N */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[3],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[1],
		},
	},
	{
		.gpio = 90, /* CAM1_RST_N */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[3],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[1],
		},
	},
	{
		.gpio = 91, /* CAM2_STANDBY_N */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[3],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[1],
		},
	},
	{
		.gpio = 92, /* CAM2_RST_N */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[3],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[1],
		},
	},
#endif
};

static struct gpiomux_setting auxpcm_act_cfg = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
};


static struct gpiomux_setting auxpcm_sus_cfg = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};

/* Primary AUXPCM port sharing GPIO lines with Primary MI2S */
static struct msm_gpiomux_config msm8974_pri_pri_auxpcm_configs[] __initdata = {
	{
		.gpio = 65,
		.settings = {
			[GPIOMUX_SUSPENDED] = &auxpcm_sus_cfg,
			[GPIOMUX_ACTIVE] = &auxpcm_act_cfg,
		},
	},
	{
		.gpio = 66,
		.settings = {
			[GPIOMUX_SUSPENDED] = &auxpcm_sus_cfg,
			[GPIOMUX_ACTIVE] = &auxpcm_act_cfg,
		},
	},
	{
		.gpio = 67,
		.settings = {
			[GPIOMUX_SUSPENDED] = &auxpcm_sus_cfg,
			[GPIOMUX_ACTIVE] = &auxpcm_act_cfg,
		},
	},
#if !defined (CONFIG_MACH_MSM8974_EF60S) && !defined (CONFIG_MACH_MSM8974_EF61K) && !defined (CONFIG_MACH_MSM8974_EF62L)
	{
		.gpio = 68,
		.settings = {
			[GPIOMUX_SUSPENDED] = &auxpcm_sus_cfg,
			[GPIOMUX_ACTIVE] = &auxpcm_act_cfg,
		},
	},
#endif	
};

/* Primary AUXPCM port sharing GPIO lines with Tertiary MI2S */
static struct msm_gpiomux_config msm8974_pri_ter_auxpcm_configs[] __initdata = {
	{
		.gpio = 74,
		.settings = {
			[GPIOMUX_SUSPENDED] = &auxpcm_sus_cfg,
			[GPIOMUX_ACTIVE] = &auxpcm_act_cfg,
		},
	},
	{
		.gpio = 75,
		.settings = {
			[GPIOMUX_SUSPENDED] = &auxpcm_sus_cfg,
			[GPIOMUX_ACTIVE] = &auxpcm_act_cfg,
		},
	},
	{
		.gpio = 76,
		.settings = {
			[GPIOMUX_SUSPENDED] = &auxpcm_sus_cfg,
			[GPIOMUX_ACTIVE] = &auxpcm_act_cfg,
		},
	},
	{
		.gpio = 77,
		.settings = {
			[GPIOMUX_SUSPENDED] = &auxpcm_sus_cfg,
			[GPIOMUX_ACTIVE] = &auxpcm_act_cfg,
		},
	},
};

#if 0  // jmlee Piezo test
#if defined (CONFIG_MACH_MSM8974_EF60S) || defined (CONFIG_MACH_MSM8974_EF61K) || defined (CONFIG_MACH_MSM8974_EF62L)
static struct msm_gpiomux_config msm8974_Piezo_amp_configs[] __initdata = {
	{
		.gpio = 68,
		.settings = {
			[GPIOMUX_SUSPENDED] = &pieozo_suspend_cfg,
			[GPIOMUX_ACTIVE] = &piezo_en_active_cfg,
		},
	},
};
#endif
#endif

#if !defined (CONFIG_MACH_MSM8974_EF59S) && !defined (CONFIG_MACH_MSM8974_EF59K) && !defined (CONFIG_MACH_MSM8974_EF59L) && !defined (CONFIG_MACH_MSM8974_EF60S) && !defined (CONFIG_MACH_MSM8974_EF61K) && !defined (CONFIG_MACH_MSM8974_EF62L)  
static struct msm_gpiomux_config msm8974_sec_auxpcm_configs[] __initdata = {
	{
		.gpio = 79,
		.settings = {
			[GPIOMUX_SUSPENDED] = &auxpcm_sus_cfg,
			[GPIOMUX_ACTIVE] = &auxpcm_act_cfg,
		},
	},
	{
		.gpio = 80,
		.settings = {
			[GPIOMUX_SUSPENDED] = &auxpcm_sus_cfg,
			[GPIOMUX_ACTIVE] = &auxpcm_act_cfg,
		},
	},
	{
		.gpio = 81,
		.settings = {
			[GPIOMUX_SUSPENDED] = &auxpcm_sus_cfg,
			[GPIOMUX_ACTIVE] = &auxpcm_act_cfg,
		},
	},
	{
		.gpio = 82,
		.settings = {
			[GPIOMUX_SUSPENDED] = &auxpcm_sus_cfg,
			[GPIOMUX_ACTIVE] = &auxpcm_act_cfg,
		},
	},
};
#endif

static struct msm_gpiomux_config wcnss_5wire_interface[] = {
	{
		.gpio = 36,
		.settings = {
			[GPIOMUX_ACTIVE]    = &wcnss_5wire_active_cfg,
			[GPIOMUX_SUSPENDED] = &wcnss_5wire_suspend_cfg,
		},
	},
	{
		.gpio = 37,
		.settings = {
			[GPIOMUX_ACTIVE]    = &wcnss_5wire_active_cfg,
			[GPIOMUX_SUSPENDED] = &wcnss_5wire_suspend_cfg,
		},
	},
	{
		.gpio = 38,
		.settings = {
			[GPIOMUX_ACTIVE]    = &wcnss_5wire_active_cfg,
			[GPIOMUX_SUSPENDED] = &wcnss_5wire_suspend_cfg,
		},
	},
	{
		.gpio = 39,
		.settings = {
			[GPIOMUX_ACTIVE]    = &wcnss_5wire_active_cfg,
			[GPIOMUX_SUSPENDED] = &wcnss_5wire_suspend_cfg,
		},
	},
	{
		.gpio = 40,
		.settings = {
			[GPIOMUX_ACTIVE]    = &wcnss_5wire_active_cfg,
			[GPIOMUX_SUSPENDED] = &wcnss_5wire_suspend_cfg,
		},
	},
};


static struct msm_gpiomux_config ath_gpio_configs[] = {
	{
		.gpio = 51,
		.settings = {
			[GPIOMUX_ACTIVE]    = &ath_gpio_active_cfg,
			[GPIOMUX_SUSPENDED] = &ath_gpio_suspend_cfg,
		},
	},
	{
		.gpio = 79,
		.settings = {
			[GPIOMUX_ACTIVE]    = &ath_gpio_active_cfg,
			[GPIOMUX_SUSPENDED] = &ath_gpio_suspend_cfg,
		},
	},
};

static struct msm_gpiomux_config msm_taiko_config[] __initdata = {
	{
		.gpio	= 63,		/* SYS_RST_N */
		.settings = {
			[GPIOMUX_SUSPENDED] = &taiko_reset,
		},
	},
	{
		.gpio	= 72,		/* CDC_INT */
		.settings = {
			[GPIOMUX_SUSPENDED] = &taiko_int,
		},
	},
};

static struct gpiomux_setting sdc3_clk_actv_cfg = {
	.func = GPIOMUX_FUNC_2,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting sdc3_cmd_data_0_3_actv_cfg = {
	.func = GPIOMUX_FUNC_2,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_UP,
};

static struct gpiomux_setting sdc3_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct gpiomux_setting sdc3_data_1_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_UP,
};

static struct msm_gpiomux_config msm8974_sdc3_configs[] __initdata = {
	{
		/* DAT3 */
		.gpio      = 35,
		.settings = {
			[GPIOMUX_ACTIVE]    = &sdc3_cmd_data_0_3_actv_cfg,
			[GPIOMUX_SUSPENDED] = &sdc3_suspend_cfg,
		},
	},
	{
		/* DAT2 */
		.gpio      = 36,
		.settings = {
			[GPIOMUX_ACTIVE]    = &sdc3_cmd_data_0_3_actv_cfg,
			[GPIOMUX_SUSPENDED] = &sdc3_suspend_cfg,
		},
	},
	{
		/* DAT1 */
		.gpio      = 37,
		.settings = {
			[GPIOMUX_ACTIVE]    = &sdc3_cmd_data_0_3_actv_cfg,
			[GPIOMUX_SUSPENDED] = &sdc3_data_1_suspend_cfg,
		},
	},
	{
		/* DAT0 */
		.gpio      = 38,
		.settings = {
			[GPIOMUX_ACTIVE]    = &sdc3_cmd_data_0_3_actv_cfg,
			[GPIOMUX_SUSPENDED] = &sdc3_suspend_cfg,
		},
	},
	{
		/* CMD */
		.gpio      = 39,
		.settings = {
			[GPIOMUX_ACTIVE]    = &sdc3_cmd_data_0_3_actv_cfg,
			[GPIOMUX_SUSPENDED] = &sdc3_suspend_cfg,
		},
	},
	{
		/* CLK */
		.gpio      = 40,
		.settings = {
			[GPIOMUX_ACTIVE]    = &sdc3_clk_actv_cfg,
			[GPIOMUX_SUSPENDED] = &sdc3_suspend_cfg,
		},
	},
};

static void msm_gpiomux_sdc3_install(void)
{
	msm_gpiomux_install(msm8974_sdc3_configs,
			    ARRAY_SIZE(msm8974_sdc3_configs));
}

#ifdef CONFIG_MMC_MSM_SDC4_SUPPORT
static struct gpiomux_setting sdc4_clk_actv_cfg = {
	.func = GPIOMUX_FUNC_2,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting sdc4_cmd_data_0_3_actv_cfg = {
	.func = GPIOMUX_FUNC_2,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_UP,
};

static struct gpiomux_setting sdc4_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct gpiomux_setting sdc4_data_1_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_UP,
};

static struct msm_gpiomux_config msm8974_sdc4_configs[] __initdata = {
	{
		/* DAT3 */
		.gpio      = 92,
		.settings = {
			[GPIOMUX_ACTIVE]    = &sdc4_cmd_data_0_3_actv_cfg,
			[GPIOMUX_SUSPENDED] = &sdc4_suspend_cfg,
		},
	},
	{
		/* DAT2 */
		.gpio      = 94,
		.settings = {
			[GPIOMUX_ACTIVE]    = &sdc4_cmd_data_0_3_actv_cfg,
			[GPIOMUX_SUSPENDED] = &sdc4_suspend_cfg,
		},
	},
	{
		/* DAT1 */
		.gpio      = 95,
		.settings = {
			[GPIOMUX_ACTIVE]    = &sdc4_cmd_data_0_3_actv_cfg,
			[GPIOMUX_SUSPENDED] = &sdc4_data_1_suspend_cfg,
		},
	},
	{
		/* DAT0 */
		.gpio      = 96,
		.settings = {
			[GPIOMUX_ACTIVE]    = &sdc4_cmd_data_0_3_actv_cfg,
			[GPIOMUX_SUSPENDED] = &sdc4_suspend_cfg,
		},
	},
	{
		/* CMD */
		.gpio      = 91,
		.settings = {
			[GPIOMUX_ACTIVE]    = &sdc4_cmd_data_0_3_actv_cfg,
			[GPIOMUX_SUSPENDED] = &sdc4_suspend_cfg,
		},
	},
	{
		/* CLK */
		.gpio      = 93,
		.settings = {
			[GPIOMUX_ACTIVE]    = &sdc4_clk_actv_cfg,
			[GPIOMUX_SUSPENDED] = &sdc4_suspend_cfg,
		},
	},
};

static void msm_gpiomux_sdc4_install(void)
{
	msm_gpiomux_install(msm8974_sdc4_configs,
			    ARRAY_SIZE(msm8974_sdc4_configs));
}
#else
static void msm_gpiomux_sdc4_install(void) {}
#endif /* CONFIG_MMC_MSM_SDC4_SUPPORT */

static struct msm_gpiomux_config apq8074_dragonboard_ts_config[] __initdata = {
	{
		/* BLSP1 QUP I2C_DATA */
		.gpio      = 2,
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_i2c_config,
		},
	},
	{
		/* BLSP1 QUP I2C_CLK */
		.gpio      = 3,
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_i2c_config,
		},
	},
};

#ifdef CONFIG_PANTECH_GPIO_SLEEP_CONFIG
static struct gpiomux_setting msm8974_gpio_suspend_in_pd_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_IN,
};

#if defined(CONFIG_MACH_MSM8974_EF56S)
static struct msm_gpiomux_config msm8974_sleep_gpio_gpio_configs[] __initdata = {
#if (BOARD_VER >= TP10)
	{
		.gpio = 16,	
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 28,	
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},
#endif
#if (BOARD_VER <= WS20)
	{
		.gpio = 31,	
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},
#endif
	{
		.gpio = 33,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},
#if (BOARD_VER >= TP10)
	{
		.gpio = 41,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 42,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 51,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 52,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 55,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 56,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},
#endif
#if (BOARD_VER <= WS20)
	{
		.gpio = 58,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},

	},
#endif
	{
		.gpio = 64,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 65,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},
#if !defined (CONFIG_MACH_MSM8974_EF60S) && !defined (CONFIG_MACH_MSM8974_EF61K) && !defined (CONFIG_MACH_MSM8974_EF62L)
	{
		.gpio = 68,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},
#endif
#if (BOARD_VER <= WS20)
	{
		.gpio = 69,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},
#endif
	{
		.gpio = 74,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 75,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},
#if (BOARD_VER >= TP10)
	{
		.gpio = 78,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},
#endif
	{
		.gpio = 79,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 81,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 82,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},
#if (BOARD_VER >= TP10)
	{
		.gpio = 100,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},
#endif
	{
		.gpio = 103,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 105,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 106,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},
#if (BOARD_VER <= WS20)
	{
		.gpio = 108,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},
#endif
	{
		.gpio = 109,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 113,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 114,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 115,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 117,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 118,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 119,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 123,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 126,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 127,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 129,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 130,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 131,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 132,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 137,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 144,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 145,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

};
#elif defined(CONFIG_MACH_MSM8974_EF59S) ||defined(CONFIG_MACH_MSM8974_EF59K) || defined(CONFIG_MACH_MSM8974_EF59L) || defined(CONFIG_MACH_MSM8974_EF60S) || defined(CONFIG_MACH_MSM8974_EF61K) || defined(CONFIG_MACH_MSM8974_EF62L)
static struct msm_gpiomux_config msm8974_sleep_gpio_gpio_configs[] __initdata = {

#if (defined (CONFIG_MACH_MSM8974_EF60S) && (BOARD_VER == PT10)) || (defined (CONFIG_MACH_MSM8974_EF61K) && (BOARD_VER == PT10)) || (defined (CONFIG_MACH_MSM8974_EF62L) && (BOARD_VER == PT10))
	{
		.gpio = 8,	
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},
#endif 
	{
		.gpio = 33,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 41,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 42,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 55,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 56,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},
	{
		.gpio = 64,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 65,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},
#if !defined (CONFIG_MACH_MSM8974_EF60S) && !defined (CONFIG_MACH_MSM8974_EF61K) && !defined (CONFIG_MACH_MSM8974_EF62L)
	{
		.gpio = 68,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},
#endif
	{
		.gpio = 74,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 75,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 78,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 82,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 100,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 102,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 103,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 105,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 106,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 108,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 109,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 113,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 114,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 115,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 117,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 118,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 119,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 123,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 126,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 127,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 129,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 130,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 131,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 132,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 137,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 144,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 145,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},
};
#endif /* CONFIG_MACH_MSM8974_EF59S || CONFIG_MACH_MSM8974_EF59K || CONFIG_MACH_MSM8974_EF59L || CONFIG_MACH_MSM8974_EF60S || CONFIG_MACH_MSM8974_EF61K || CONFIG_MACH_MSM8974_EF62L*/
#endif /* CONFIG_PANTECH_GPIO_SLEEP_CONFIG */

#ifdef CONFIG_PANTECH_SND
static struct gpiomux_setting heaset_detect_active_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
	.dir = GPIOMUX_IN,
};

static struct gpiomux_setting heaset_detect_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
	.dir = GPIOMUX_IN,
};

static struct msm_gpiomux_config headset_detect_irq_configs[] __initdata = {
	{
		.gpio = 95,
		.settings = {
			[GPIOMUX_ACTIVE]    = &heaset_detect_active_cfg,
			[GPIOMUX_SUSPENDED] = &heaset_detect_suspend_cfg,
		},
	},	
};

static struct gpiomux_setting audio_subsystem_active_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
#ifdef CONFIG_PANTECH_SND_EF59SKL_HW
	.pull = GPIOMUX_PULL_DOWN,
#else
	.pull = GPIOMUX_PULL_NONE,
#endif	
	.dir = GPIOMUX_OUT_LOW,
};

static struct gpiomux_setting audio_subsystem_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
#ifdef CONFIG_PANTECH_SND_EF59SKL_HW
	.pull = GPIOMUX_PULL_DOWN,
#else
	.pull = GPIOMUX_PULL_NONE,
#endif	
	.dir = GPIOMUX_OUT_LOW,
};

static struct msm_gpiomux_config audio_subsystem_irq_configs[] __initdata = {
	{
		.gpio = 49,
		.settings = {
			[GPIOMUX_ACTIVE]    = &audio_subsystem_active_cfg,
			[GPIOMUX_SUSPENDED] = &audio_subsystem_suspend_cfg,
		},
	},	
};
#endif /* CONFIG_PANTECH_SND */

#ifdef CONFIG_CXD2235AGG_NFC_FELICA
#if (BOARD_VER >= PT20)
static struct gpiomux_setting nfcf_gpio_in_pd_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_IN,
};
#endif

static struct gpiomux_setting nfcf_gpio_in_pu_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
	.dir = GPIOMUX_IN,
};

static struct gpiomux_setting nfcf_gpio_out_low_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_LOW,
};

static struct msm_gpiomux_config nfcf_gpio_configs[] __initdata = {
#if (BOARD_VER >= PT20)
	{
		.gpio = 46,  //NFCF_INT
		.settings = {
			[GPIOMUX_ACTIVE]    = &nfcf_gpio_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &nfcf_gpio_in_pd_cfg,
		},
	},	
#if !defined (CONFIG_MACH_MSM8974_EF60S) && !defined (CONFIG_MACH_MSM8974_EF61K) && !defined (CONFIG_MACH_MSM8974_EF62L)
	{
		.gpio = 68, //NFCF_RFS
		.settings = {
			[GPIOMUX_ACTIVE]    = &nfcf_gpio_in_pu_cfg,
			[GPIOMUX_SUSPENDED] = &nfcf_gpio_in_pu_cfg,
		},
	},	
#endif

#endif
#if (BOARD_VER < PT20)
	{
		.gpio = 89, //NFCF_HSEL => WS10 PMGPIO 8
		.settings = {
			[GPIOMUX_ACTIVE]    = &nfcf_gpio_out_low_cfg,
			[GPIOMUX_SUSPENDED] = &nfcf_gpio_out_low_cfg,
		},

	},
	{
		.gpio = 91, //NFCF_PON => WS10 PMGPIO 10
		.settings = {
			[GPIOMUX_ACTIVE]    = &nfcf_gpio_out_low_cfg,
			[GPIOMUX_SUSPENDED] = &nfcf_gpio_out_low_cfg,
		},

	},
#endif

	{
		.gpio = 100, //NFCF_TEMP
		.settings = {
			[GPIOMUX_ACTIVE]    = &nfcf_gpio_out_low_cfg,
			[GPIOMUX_SUSPENDED] = &nfcf_gpio_out_low_cfg,
		},

	},
	{
		.gpio = 102, //NFCF_INTU
		.settings = {
			[GPIOMUX_ACTIVE]    = &nfcf_gpio_in_pu_cfg,
			[GPIOMUX_SUSPENDED] = &nfcf_gpio_in_pu_cfg,
		},

	},
};


#endif /*CONFIG_CXD2235AGG_NFC_FELICA */
void __init msm_8974_init_gpiomux(void)
{
	int rc;

	rc = msm_gpiomux_init_dt();
	if (rc) {
		pr_err("%s failed %d\n", __func__, rc);
		return;
	}

#if defined(CONFIG_MACH_MSM8974_CSFB) || defined(CONFIG_MACH_MSM8974_SVLTE)
	msm_gpiomux_install(msm_lcd2_2_voltage_configs , ARRAY_SIZE(msm_lcd2_2_voltage_configs));
#endif

#if defined(CONFIG_KS8851) || defined(CONFIG_KS8851_MODULE)
	msm_gpiomux_install(msm_eth_configs, ARRAY_SIZE(msm_eth_configs));
#endif

#ifdef CONFIG_TOUCHSCREEN_MELFAS_TS //p11223
        msm_gpiomux_install(msm8960_melfas_configs, ARRAY_SIZE(msm8960_melfas_configs));
#endif

//++ p11309 - 2013.04.25 for TI,LP5523 LED Driver IC
#if defined(CONFIG_LEDS_LP5523) 
	msm_gpiomux_install(lp5523_led_configs, ARRAY_SIZE(lp5523_led_configs));
#endif
//-- p11309
  //++ p13106 ls2
#if defined (CONFIG_MACH_MSM8974_EF59S) || defined (CONFIG_MACH_MSM8974_EF59K) || defined (CONFIG_MACH_MSM8974_EF59L)
  msm_gpiomux_install(touch_pen_configs, ARRAY_SIZE(touch_pen_configs));
#endif



	msm_gpiomux_install(msm_blsp_configs, ARRAY_SIZE(msm_blsp_configs));
#if defined(CONFIG_FB_MSM_MDSS_HDMI_MHL_SII8240) 
	msm_gpiomux_install(mhl_sii_blsp_configs, ARRAY_SIZE(mhl_sii_blsp_configs));
#endif		
#ifdef CONFIG_MACH_MSM8974_NAMI
	msm_gpiomux_install(lm3530_blsp_configs, ARRAY_SIZE(lm3530_blsp_configs)); 	
#endif
	msm_gpiomux_install(msm_blsp2_uart7_configs,
			 ARRAY_SIZE(msm_blsp2_uart7_configs));
	msm_gpiomux_install(wcnss_5wire_interface,
				ARRAY_SIZE(wcnss_5wire_interface));
	if (of_board_is_liquid())
		msm_gpiomux_install_nowrite(ath_gpio_configs,
					ARRAY_SIZE(ath_gpio_configs));
	msm_gpiomux_install(msm8974_slimbus_config,
			ARRAY_SIZE(msm8974_slimbus_config));

	msm_gpiomux_install(msm_touch_configs, ARRAY_SIZE(msm_touch_configs));
#ifndef FEATURE_PANTECH_PMIC_CHARGER_SMB349
		msm_gpiomux_install(hap_lvl_shft_config,
				ARRAY_SIZE(hap_lvl_shft_config));
#endif

	msm_gpiomux_install(msm_sensor_configs, ARRAY_SIZE(msm_sensor_configs));

	msm_gpiomux_install(&sd_card_det, 1);

	if (machine_is_apq8074() && (of_board_is_liquid() || \
	    of_board_is_dragonboard()))
	msm_gpiomux_sdc3_install();

	msm_gpiomux_sdc4_install();

	msm_gpiomux_install(msm_taiko_config, ARRAY_SIZE(msm_taiko_config));

	msm_gpiomux_install(msm_hsic_configs, ARRAY_SIZE(msm_hsic_configs));
#if !defined(CONFIG_PN544) && !defined(CONFIG_PN547)
	msm_gpiomux_install(msm_hsic_hub_configs,
				ARRAY_SIZE(msm_hsic_hub_configs));
#endif
#if !defined (CONFIG_MACH_MSM8974_EF56S) && !defined (CONFIG_MACH_MSM8974_EF57K) && !defined (CONFIG_MACH_MSM8974_EF58L) && !defined (CONFIG_MACH_MSM8974_EF59S) && !defined (CONFIG_MACH_MSM8974_EF59K) && !defined (CONFIG_MACH_MSM8974_EF59L) && !defined (CONFIG_MACH_MSM8974_EF60S) && !defined (CONFIG_MACH_MSM8974_EF61K) && !defined (CONFIG_MACH_MSM8974_EF62L) 
#if defined(CONFIG_MACH_MSM8974_NAMI)
	msm_gpiomux_install(msm_hdmi_configs, ARRAY_SIZE(msm_hdmi_configs));
	msm_gpiomux_install(msm_mhl8240_configs, ARRAY_SIZE(msm_mhl8240_configs));
#endif
#endif
	if (of_board_is_fluid())
		msm_gpiomux_install(msm_mhl_configs,
				    ARRAY_SIZE(msm_mhl_configs));

	if (of_board_is_liquid() ||
	    (of_board_is_dragonboard() && machine_is_apq8074()))
		msm_gpiomux_install(msm8974_pri_ter_auxpcm_configs,
				 ARRAY_SIZE(msm8974_pri_ter_auxpcm_configs));
	else
		msm_gpiomux_install(msm8974_pri_pri_auxpcm_configs,
				 ARRAY_SIZE(msm8974_pri_pri_auxpcm_configs));

#if !defined (CONFIG_MACH_MSM8974_EF59S) && !defined (CONFIG_MACH_MSM8974_EF59K) && !defined (CONFIG_MACH_MSM8974_EF59L) && !defined (CONFIG_MACH_MSM8974_EF60S) && !defined (CONFIG_MACH_MSM8974_EF61K) && !defined (CONFIG_MACH_MSM8974_EF62L) 
	msm_gpiomux_install(msm8974_sec_auxpcm_configs,
				 ARRAY_SIZE(msm8974_sec_auxpcm_configs));
#endif
	//msm_gpiomux_install_nowrite(msm_lcd_configs,
	//		ARRAY_SIZE(msm_lcd_configs));
#if defined (CONFIG_F_SKYDISP_SHARP_FHD_VIDEO_COMMON) || defined (CONFIG_F_SKYDISP_SHARP_FHD_CMD_COMMON)
	msm_gpiomux_install(lcd_common_configs,
			ARRAY_SIZE(lcd_common_configs));
	msm_gpiomux_install(lcd_ext_power_configs,
			ARRAY_SIZE(lcd_ext_power_configs));
	msm_gpiomux_install(lcd_bl_configs,
			ARRAY_SIZE(lcd_bl_configs));
#endif

#if !defined(CONFIG_INPUT_FPC1080)//P14696 add for fpc1080's spi
	if (of_board_is_rumi())
		msm_gpiomux_install(msm_rumi_blsp_configs,
				    ARRAY_SIZE(msm_rumi_blsp_configs));
#endif

#ifdef CONFIG_PANTECH_GPIO_SLEEP_CONFIG
	msm_gpiomux_install(msm8974_sleep_gpio_gpio_configs,
			ARRAY_SIZE(msm8974_sleep_gpio_gpio_configs));
#endif /* CONFIG_PANTECH_GPIO_SLEEP_CONFIG */

	if (socinfo_get_platform_subtype() == PLATFORM_SUBTYPE_MDM)
		msm_gpiomux_install(mdm_configs,
			ARRAY_SIZE(mdm_configs));

#ifdef CONFIG_PANTECH_SND
	msm_gpiomux_install(headset_detect_irq_configs,
			ARRAY_SIZE(headset_detect_irq_configs));
	msm_gpiomux_install(audio_subsystem_irq_configs,
			ARRAY_SIZE(audio_subsystem_irq_configs));
#endif /* CONFIG_PANTECH_SND */

	if (of_board_is_dragonboard() && machine_is_apq8074())
		msm_gpiomux_install(apq8074_dragonboard_ts_config,
			ARRAY_SIZE(apq8074_dragonboard_ts_config));

#ifdef CONFIG_CXD2235AGG_NFC_FELICA 
	msm_gpiomux_install(nfcf_gpio_configs,
		ARRAY_SIZE(nfcf_gpio_configs));
#endif	

#ifdef FEATURE_PANTECH_USB_REDRIVER_EN_CONTROL
	msm_gpiomux_install(msm8974_usb3_redriveric_configs, 
			ARRAY_SIZE(msm8974_usb3_redriveric_configs));
#endif

#if 0  // jmlee Piezo test
#if defined (CONFIG_MACH_MSM8974_EF60S) || defined (CONFIG_MACH_MSM8974_EF61K) || defined (CONFIG_MACH_MSM8974_EF62L)
       pr_err("%s msm8974_Piezo_amp_configs ##########\n", __func__);
	msm_gpiomux_install(msm8974_Piezo_amp_configs,
				 ARRAY_SIZE(msm8974_Piezo_amp_configs));
#endif
#endif

}
