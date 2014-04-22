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
 *
 */

#ifndef __MHL_SII_8240_H__
#define __MHL_SII_8240_H__

#include <linux/types.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_platform.h>


/* MHL 8334 supports a max HD pixel clk of 75 MHz */
#define MAX_MHL_PCLK 75000

enum mhl_gpio_type {
	MHL_TX_RESET_GPIO,
	MHL_TX_INTR_GPIO,
	MHL_TX_PMIC_PWR_GPIO,
	MHL_TX_MHL_USB_SEL_GPIO,
	MHL_TX_PMIC_USB_ID_GPIO,
	MHL_TX_MAX_GPIO,
};

enum mhl_vreg_type {
	MHL_TX_3V_VREG,
	MHL_TX_MAX_VREG,
};

enum discovery_result_enum {
	MHL_DISCOVERY_RESULT_USB = 0,
	MHL_DISCOVERY_RESULT_MHL,
	MHL_DISCOVERY_RESULT_RETRY,
};


struct mhl_tx_platform_data {
	/* Data filled from device tree nodes */
	struct dss_gpio *gpios[MHL_TX_MAX_GPIO];
	struct dss_vreg *vregs[MHL_TX_MAX_VREG];
	int irq;
	struct platform_device *hdmi_pdev;
};


struct mhl_tx_ctrl {
#if 1	
	struct platform_device *pdev;
	struct mhl_tx_platform_data *pdata;
	struct i2c_client *i2c_handle;
	///struct power_supply mhl_psy;
	uint8_t chip_rev_id;
	void *hdmi_mhl_ops;
	struct delayed_work mhl_connection_work;
	struct workqueue_struct *mhl_connection_workqueue;	
#else		
	struct platform_device *pdev;
	struct mhl_tx_platform_data *pdata;
	struct i2c_client *i2c_handle;

	uint8_t cur_state;
	uint8_t chip_rev_id;
	int mhl_mode;
	struct completion rgnd_done;
	void (*notify_usb_online)(int online);
	struct usb_ext_notification *mhl_info;
	bool disc_enabled;
	struct power_supply mhl_psy;
	bool vbus_active;
	int current_val;
	struct completion msc_cmd_done;
	uint8_t devcap[16];
	uint8_t devcap_state;
	uint8_t path_en_state;
	void *hdmi_mhl_ops;
	struct work_struct mhl_msc_send_work;
	struct list_head list_cmd;
	struct input_dev *input;
	struct workqueue_struct *msc_send_workqueue;
	u16 *rcp_key_code_tbl;
	size_t rcp_key_code_tbl_len;
	struct scrpd_struct scrpd;
	int scrpd_busy;
	int wr_burst_pending;
	struct completion req_write_done;
	spinlock_t lock;
	bool tx_powered_off;
	uint8_t dwnstream_hpd;
#endif	
};

////////////////////////////////////////////////////////////////////////////////
 /*  mhl_device_discovery */
extern int mhl_device_discovery(const char *name, int *result);

/* - register|unregister MHL cable plug callback. */
extern int mhl_register_callback
	(const char *name, void (*callback)(int online));
extern int mhl_unregister_callback(const char *name);
extern bool is_packed_pixel_available(void);
extern bool get_mhl_connection_status(void);
extern void setupstream_hpd(int hpdStatus);

#endif /* __MHL_SII_8240_H__ */
