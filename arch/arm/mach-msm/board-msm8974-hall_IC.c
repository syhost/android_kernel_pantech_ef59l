/* linux/arch/arm/mach-msm/board-msm8974-hall_IC.c
 *  
 * Copyright (C) 2007 Google, Inc.
 * Author: Brian Swetland <swetland@google.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <asm/mach-types.h>
#include <linux/platform_device.h>
#include <linux/gpio_event.h>
#include <linux/gpio.h>

#if (defined(T_EF56S) || defined(T_EF57K) || defined(T_EF58L)) // p11223_for_EF59S
#if (BOARD_VER < WS20)
#define HALL_SENSOR_INT 33 //WS10 

static struct gpio_event_direct_entry EF56SERIES_hall_map[] = {
	{33, SW_LID}
};
#else
#define HALL_SENSOR_INT 9 //since WS20

static struct gpio_event_direct_entry EF56SERIES_hall_map[] = {
	{9, SW_LID}
};
#endif 
#else
#define HALL_SENSOR_INT 9 //EF59S , 60series

static struct gpio_event_direct_entry EF56SERIES_hall_map[] = {
	{9, SW_LID}
};
#endif

static struct gpio_event_input_info EF56SERIES_hall_info = {
	.info.func = gpio_event_input_func,
	.info.no_suspend = true,  // p11774
	.flags = GPIOKPF_LEVEL_TRIGGERED_IRQ,
	.type = EV_SW,
	.keymap = EF56SERIES_hall_map,
	.keymap_size = ARRAY_SIZE(EF56SERIES_hall_map),
	//.debounce_time.tv64 = 5 * NSEC_PER_MSEC,
	.debounce_time.tv64 = 100 * NSEC_PER_MSEC,
	.poll_time.tv64 = 20 * NSEC_PER_MSEC,  // p11223
	//.poll_time.tv64 = 500 * NSEC_PER_MSEC,  // p11223_20130611
};

static struct gpio_event_info *EF56SERIES_hallIC_info[] = {
	&EF56SERIES_hall_info.info,	
};

static struct gpio_event_platform_data EF56SERIES_hall_data = {
	.name		= "hall_sensor",
	.info		= EF56SERIES_hallIC_info,
	.info_count	= ARRAY_SIZE(EF56SERIES_hallIC_info)
};

struct platform_device hall_device_EF56SERIES = {
	.name	= GPIO_EVENT_DEV_NAME,
	.id	= -1,
	.dev	= {
		.platform_data	= &EF56SERIES_hall_data,
	},
};

static int __init EF56SERIES_init_hall(void)
{
	int rc = 0;
	rc = gpio_tlmm_config(GPIO_CFG(HALL_SENSOR_INT, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA),GPIO_CFG_ENABLE);
	if (rc) {
		pr_err("%s: Could not configure gpio %d\n",
					 __func__, HALL_SENSOR_INT);
	}
	return platform_device_register(&hall_device_EF56SERIES);
}
device_initcall(EF56SERIES_init_hall);
