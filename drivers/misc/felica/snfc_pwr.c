/* misc/felica/snfc_flt.c
 *
 * Copyright (C) 2012 Pantech.
 *
 * Author: Cho HyunDon <cho.hyundon@pantech.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/stddef.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/felica.h>
#include <linux/slab.h>
#include <linux/syscalls.h>
#include <linux/compat.h>
#include <linux/mfd/pm8xxx/pm8921.h>
#include <mach/gpio.h>
#include <mach/gpiomux.h>
#include <mach/irqs.h>

#include "felica_master.h"

#define PRT_NAME "snfc_flt"

/**
 * @brief   Open file operation of NFC USIM PWR & MDM USIM PWR MUX controller
 * @param   inode : (unused)
 * @param   file  : (unused)
 * @retval  0     : Success
 * @note
 */
static int snfc_flt_open(struct inode *inode, struct file *file)
{
	return 0;
}

/**
 * @brief   Open file operation of NFC USIM PWR & MDM USIM PWR MUX controller
 * @param   inode : (unused)
 * @param   file  : (unused)
 * @retval  0     : Success
 * @note
 */
static int snfc_flt_release(struct inode *inode, struct file *file)
{
	return 0;
}

/**
 * @brief   Open file operation of NFC USIM PWR & MDM USIM PWR MUX controller
 * @details This function executes;\n
 *            # Copy value from user space\n
 *            # [When writing High,]\n
 *            #   | Write High to FLT GPIO\n
 *            #   | [Params meet the condition,]  MDM USIM PWR ENABLE.\n
 *            # [When writing Low,]\n
 *            #   | Write Low to FLT GPIO\n
 *            #   | [Params meet the condition,]  NFC USIM PWR ENABLE.\n
 * @param   file     : (unused)
 * @param   buf      : Source of the written data
 * @param   count    : Data length must be 1 Byte.
 * @param   offset   : (unused)
 * @retval  1        : Success
 * @retval  Negative : Failure\n
 *            -EINVAL = Invalid argument\n
 *            -EFAULT = Cannot copy data from user space\n
 *            -EIO    = Cannot control VREG
 * @note
 */
static ssize_t snfc_flt_write(struct file *file, const char __user *buf,
					size_t count, loff_t *offset)
{
	char kbuf;
	struct felica_platform_data *pfdata= felica_drv->pdev->dev.platform_data;


	if (1 != count  || !buf) {
		pr_err(PRT_NAME ": Error. Invalid arg @FLT write.\n");
		return -EINVAL;
	}

	/* Copy value from user space */
	if (copy_from_user(&kbuf, buf, 1)) {
		pr_err(PRT_NAME ": Error. copy_from_user failure.\n");
		return -EFAULT;
	}

	if (FELICA_NFC_HIGH == kbuf) {
		/* Write High to FLT GPIO */
		gpio_set_value_cansleep(pfdata->gpio_flt, FELICA_NFC_HIGH);
	} else if (FELICA_NFC_LOW == kbuf) {
		/* Write LOW to FLT GPIO */
		gpio_set_value_cansleep(pfdata->gpio_flt, FELICA_NFC_LOW);
	} else {
		pr_err(PRT_NAME ": Error. Invalid val @FLT write.\n");
		return -EINVAL;
	}
	/* 1 byte write */
	return 1;
}


/***************** sony NFC FOPS ****************************/
static const struct file_operations snfc_flt_fops = {
	.owner		= THIS_MODULE,
	.write		= snfc_flt_write,	
	.open		= snfc_flt_open,
	.release		= snfc_flt_release,
};

static struct miscdevice snfc_flt_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "snfc_flt",
	.fops = &snfc_flt_fops,
};

/**
 * @brief   Open file operation of NFC USIM PWR & MDM USIM PWR MUX controller
 * @details This function executes;\n
 *            # Check snfc_usim_device data\n
 *            # Alloc and init sony NFC & USIM PWR(HVDD)controller's data\n
 *            # Create snfc character device (/dev/snfc)
 * @param   pfdata   : Pointer to snfc platform data
 * @retval  0        : Success
 * @retval  Negative : Initialization failed.\n
 *            -EINVAL = No platform data\n
 *            -ENODEV = Requesting GPIO failed\n
 *            -ENOMEM = No enough memory / Cannot create char dev
 * @note
 */

int snfc_flt_probe_func(struct felica_device *pdevice)
{
	int ret;
	struct felica_platform_data *pfdata = pdevice->pdev->dev.platform_data;

	pr_debug(PRT_NAME ": %s\n", __func__);

	/* Check FLT platform data */
	if (!pfdata) {
		pr_err(PRT_NAME ": Error. No platform data for FLT.\n");
		return -EINVAL;
	}

	/* Request FLT GPIO */
	ret = gpio_request(pfdata->gpio_flt, "snfc_flt");
	if (ret) {
		pr_err(PRT_NAME ": Error. FLT GPIO request failed.\n");
		ret = -ENODEV;
		goto err_request_flt_gpio;
	}
	
	//ret = gpio_direction_output(pfdata->gpio_flt, FELICA_NFC_LOW);
	ret = gpio_direction_output(pfdata->gpio_flt, FELICA_NFC_HIGH);
	if (ret) {
		pr_err(PRT_NAME ": Error. FLT GPIO direction failed.\n");
		ret = -ENODEV;
		goto err_direction_flt_gpio;
	}

	/* Create Felica Uart character device (/dev/snfc_flt) */
	if (misc_register(&snfc_flt_device)) {
		pr_err(PRT_NAME ": Error. Cannot register SNFC.\n");
		ret=  -ENOMEM;
		goto err_create_flt_dev;
	}

	return 0;

err_create_flt_dev:
err_direction_flt_gpio:
	gpio_free(pfdata->gpio_pon);
err_request_flt_gpio:
	return ret;
}

/**
 * @brief   Open file operation of NFC USIM PWR & MDM USIM PWR MUX controller
 * @details This function executes;\n
 *            # Deregister sony NFC character device (/dev/snfc)\n
 *            # Release snfc controller's data
 * @param   N/A
 * @retval  N/A
 * @note
 */
void snfc_flt_remove_func(void)
{
	struct felica_platform_data *pfdata= felica_drv->pdev->dev.platform_data;

	pr_debug(PRT_NAME ": %s\n", __func__);
	
	misc_deregister(&snfc_flt_device);
	gpio_free(pfdata->gpio_flt);
}

