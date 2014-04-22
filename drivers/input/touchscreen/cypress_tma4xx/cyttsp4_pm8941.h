/*
 * Core Header for:
 * Cypress TrueTouch(TM) Standard Product (TTSP) touchscreen drivers.
 * For use with Cypress Gen4 and Solo parts.
 * Supported parts include:
 * CY8CTMA768
 * CY8CTMA4XX
 *
 * Copyright (C) 2012 Pantech, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2, and only version 2, as published by the
 * Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Contact Cypress Semiconductor at www.cypress.com <kev@cypress.com>
 *
 */

#include "cyttsp4_core.h"
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/regulator/consumer.h>
#include <linux/device.h>
//#include <linux/mfd/pm8xxx/pm8941.h>

// #define PM8921_GPIO_BASE                NR_GPIO_IRQS
// #define PM8921_GPIO_PM_TO_SYS(pm_gpio)  (pm_gpio - 1 + PM8921_GPIO_BASE)
// #define PM8921_MPP_BASE                 (PM8921_GPIO_BASE + PM8921_NR_GPIOS)
// #define PM8921_MPP_PM_TO_SYS(pm_gpio)   (pm_gpio - 1 + PM8921_MPP_BASE)
// #define PM8921_IRQ_BASE                 (NR_MSM_IRQS + NR_GPIO_IRQS)


#define TOUCH_POWER_VDD		"8941_l10"	//2.8 ~ 3.3V
//#define TOUCH_POWER_VCC		"8941_l21"// 1.8

#define GPIO_TOUCH_RST	60
#define GPIO_TOUCH_CHG	61

int hw_on_off(struct device *dev, int on);
int init_hw_setting(struct device *dev);
