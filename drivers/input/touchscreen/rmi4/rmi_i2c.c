/*
 * Copyright (c) 2011, 2012 Synaptics Incorporated
 * Copyright (c) 2011 Unixphere
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#define COMMS_DEBUG 0

#define IRQ_DEBUG 0

#if COMMS_DEBUG || IRQ_DEBUG
#define DEBUG
#endif

#include <linux/kernel.h>
#include <linux/lockdep.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/pm.h>
#include <linux/gpio.h>

#include "rmi.h"
#include "rmi_driver.h"

#define RMI_PAGE_SELECT_REGISTER 0xff
#define RMI_I2C_PAGE(addr) (((addr) >> 8) & 0xff)

static char *phys_proto_name = "i2c";

struct rmi_i2c_data {
	struct mutex page_mutex;
	int page;
	int enabled;
	int irq;
	int irq_flags;
	struct rmi_phys_device *phys;
};

static irqreturn_t rmi_i2c_irq_thread(int irq, void *p)
{
	struct rmi_phys_device *phys = p;
	struct rmi_device *rmi_dev = phys->rmi_dev;
	struct rmi_driver *driver = rmi_dev->driver;
//	struct rmi_device_platform_data *pdata = phys->dev->platform_data;

#if IRQ_DEBUG
	dev_dbg(phys->dev, "ATTN gpio, value: %d.\n",
			gpio_get_value(pdata->attn_gpio));
#endif

//	if (gpio_get_value(pdata->attn_gpio) == pdata->attn_polarity) 
	{
		phys->info.attn_count++;
		if (driver && driver->irq_handler && rmi_dev)
			driver->irq_handler(rmi_dev, irq);
	}

	return IRQ_HANDLED;
}


static int synaptics_touchpad_gpio_setup(bool configure);

/*
 * rmi_set_page - Set RMI page
 * @phys: The pointer to the rmi_phys_device struct
 * @page: The new page address.
 *
 * RMI devices have 16-bit addressing, but some of the physical
 * implementations (like SMBus) only have 8-bit addressing. So RMI implements
 * a page address at 0xff of every page so we can reliable page addresses
 * every 256 registers.
 *
 * The page_mutex lock must be held when this function is entered.
 *
 * Returns zero on success, non-zero on failure.
 */
static int rmi_set_page(struct rmi_phys_device *phys, unsigned int page)
{
	struct i2c_client *client = to_i2c_client(phys->dev);
	struct rmi_i2c_data *data = phys->data;
	char txbuf[2] = {RMI_PAGE_SELECT_REGISTER, page};
	int retval;

#if COMMS_DEBUG
	dev_dbg(&client->dev, "RMI4 I2C writes 3 bytes: %02x %02x\n",
		txbuf[0], txbuf[1]);
#endif
	phys->info.tx_count++;
	phys->info.tx_bytes += sizeof(txbuf);
	retval = i2c_master_send(client, txbuf, sizeof(txbuf));
	if (retval != sizeof(txbuf)) {
		phys->info.tx_errs++;
		dev_err(&client->dev,
			"%s: set page failed: %d.", __func__, retval);
		return (retval < 0) ? retval : -EIO;
	}
	data->page = page;
	return 0;
}

static int rmi_i2c_write_block(struct rmi_phys_device *phys, u16 addr, u8 *buf,
			       int len)
{
	struct i2c_client *client = to_i2c_client(phys->dev);
	struct rmi_i2c_data *data = phys->data;
	u8 txbuf[len + 1];
	int retval;
#if	COMMS_DEBUG
	char debug_buf[len*3 + 1];
	int i, n;
#endif

	txbuf[0] = addr & 0xff;
	memcpy(txbuf + 1, buf, len);

	mutex_lock(&data->page_mutex);

	if (RMI_I2C_PAGE(addr) != data->page) {
		retval = rmi_set_page(phys, RMI_I2C_PAGE(addr));
		if (retval < 0)
			goto exit;
	}

#if COMMS_DEBUG
	n = 0;
	for (i = 0; i < len; i++)
		n = snprintf(debug_buf+n, 4, "%02x ", buf[i]);
	dev_dbg(&client->dev, "RMI4 I2C writes %d bytes at %#06x: %s\n",
		len, addr, debug_buf);
#endif

	phys->info.tx_count++;
	phys->info.tx_bytes += sizeof(txbuf);
	retval = i2c_master_send(client, txbuf, sizeof(txbuf));
	if (retval < 0)
		phys->info.tx_errs++;
	else
		retval--; /* don't count the address byte */

	PAN_DBG("I2C Write: 0x%X..0x%X..0x%X\n", data->page, addr, buf[0]);

exit:
	mutex_unlock(&data->page_mutex);
	return retval;
}

static int rmi_i2c_write(struct rmi_phys_device *phys, u16 addr, u8 data)
{
	int retval = rmi_i2c_write_block(phys, addr, &data, 1);
	return (retval < 0) ? retval : 0;
}

static int rmi_i2c_read_block(struct rmi_phys_device *phys, u16 addr, u8 *buf,
			      int len)
{
	struct i2c_client *client = to_i2c_client(phys->dev);
	struct rmi_i2c_data *data = phys->data;
	u8 txbuf[1] = {addr & 0xff};
	int retval;
#if	COMMS_DEBUG
	char debug_buf[len*3 + 1];
	char *temp = debug_buf;
	int i, n;
#endif

	mutex_lock(&data->page_mutex);

	if (RMI_I2C_PAGE(addr) != data->page) {
		retval = rmi_set_page(phys, RMI_I2C_PAGE(addr));
		if (retval < 0)
			goto exit;
	}

#if COMMS_DEBUG
	dev_dbg(&client->dev, "RMI4 I2C writes 1 bytes: %02x\n", txbuf[0]);
#endif
	phys->info.tx_count++;
	phys->info.tx_bytes += sizeof(txbuf);
	retval = i2c_master_send(client, txbuf, sizeof(txbuf));
	if (retval != sizeof(txbuf)) {
		phys->info.tx_errs++;
		retval = (retval < 0) ? retval : -EIO;
		goto exit;
	}

	retval = i2c_master_recv(client, buf, len);

	phys->info.rx_count++;
	phys->info.rx_bytes += len;
	if (retval < 0)
		phys->info.rx_errs++;
#if COMMS_DEBUG
	else {
		n = 0;
		for (i = 0; i < len; i++) {
			n = sprintf(temp, " %02x", buf[i]);
			temp += n;
		}
		dev_dbg(&client->dev, "RMI4 I2C read %d bytes at %#06x:%s\n",
			len, addr, debug_buf);
	}
#endif

	PAN_DBG("I2C Read: 0x%X..0x%X..0x%X..0x%X\n", data->page, addr, len, buf[0]);

exit:
	mutex_unlock(&data->page_mutex);
	return retval;
}

static int rmi_i2c_read(struct rmi_phys_device *phys, u16 addr, u8 *buf)
{
	int retval = rmi_i2c_read_block(phys, addr, buf, 1);
	return (retval < 0) ? retval : 0;
}

static int acquire_attn_irq(struct rmi_i2c_data *data)
{
	return request_threaded_irq(data->irq, NULL, rmi_i2c_irq_thread,
			data->irq_flags, dev_name(data->phys->dev), data->phys);
}

static int enable_device(struct rmi_phys_device *phys)
{
	int retval = 0;

	struct rmi_i2c_data *data = phys->data;

	if (data->enabled)
		return 0;

	retval = acquire_attn_irq(data);
	if (retval)
		goto error_exit;

	data->enabled = true;
	dev_dbg(phys->dev, "Physical device enabled.\n");
	return 0;

error_exit:
	dev_err(phys->dev, "Failed to enable physical device. Code=%d.\n",
		retval);
	return retval;
}

static void disable_device(struct rmi_phys_device *phys)
{
	struct rmi_i2c_data *data = phys->data;

	if (!data->enabled)
		return;

	disable_irq(data->irq);
	free_irq(data->irq, data->phys);

	dev_dbg(phys->dev, "Physical device disabled.\n");
	data->enabled = false;
}

static int synaptics_touchpad_gpio_setup(bool configure)
{
	int retval=0;

	if ( configure ) {
		
		gpio_tlmm_config(GPIO_CFG(RMI_INT_GPIO, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA), GPIO_CFG_ENABLE);
//		gpio_tlmm_config(GPIO_CFG(RMI_VDD_GPIO, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_8MA), GPIO_CFG_ENABLE);

		retval = gpio_request(RMI_INT_GPIO, "rmi4_attn");
		if (retval) {
			pr_err("%s: Failed to get attn gpio %d. Code: %d.", __func__, RMI_INT_GPIO, retval);
			return retval;
		}

		retval = gpio_direction_input(RMI_INT_GPIO);
		if (retval) {
			pr_err("%s: Failed to setup attn gpio %d. Code: %d.", __func__, RMI_INT_GPIO, retval);
			gpio_free(RMI_INT_GPIO);
		}

// 		retval = gpio_request(RMI_VDD_GPIO, "rmi4_vdd");
// 		if (retval) {
// 			pr_err("%s: Failed to get attn gpio %d. Code: %d.", __func__, RMI_VDD_GPIO, retval);
// 			return retval;
// 		}
// 
// 		retval = gpio_direction_output(RMI_VDD_GPIO, 1);
// 		if (retval) {
// 			pr_err("%s: Failed to setup attn gpio %d. Code: %d.", __func__, RMI_VDD_GPIO, retval);
// 			gpio_free(RMI_VDD_GPIO);
// 		}
	}
	else {
		gpio_free(RMI_INT_GPIO);
// 		gpio_set_value(RMI_VDD_GPIO, 0 );
// 		gpio_free(RMI_VDD_GPIO);		
	}	

	return retval;
}

static int __devinit rmi_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct rmi_phys_device *rmi_phys;
	struct rmi_i2c_data *data;
	//struct rmi_device_platform_data *pdata = client->dev.platform_data;
	struct rmi_device_platform_data *pdata = NULL;
	int error;
//++ p11309 - 2013.06.19 for remove hover
	u8 regOff = 0x00;
	u8 regData = 0x00;
//-- p11309

	printk("+-----------------------------------------+\n");
	printk("|  Synaptics S7020 I2C Probe!             |\n");
	printk("+-----------------------------------------+\n");

	if (client->dev.of_node) {
		pdata = devm_kzalloc(&client->dev, sizeof(struct rmi_device_platform_data), GFP_KERNEL);
		if (!pdata) {
			dev_err(&client->dev, "Failed to allocate memory\n");
			return -ENOMEM;
		}

		pdata->driver_name		= "rmi_generic";
		pdata->attn_gpio		= RMI_INT_GPIO;
		pdata->attn_polarity	= RMI_ATTN_ACTIVE_LOW;
		pdata->sensor_name		="touch";
#ifdef CONFIG_RMI4_F11_TYPEB	
		pdata->f11_type_b		= true;
#endif
		client->dev.platform_data = pdata;
		
	} else
		pdata = client->dev.platform_data;

	if (!pdata) {
		dev_err(&client->dev, "%s: no platform data\n", __func__);
		return -EINVAL;
	}

	dev_err(&client->dev, "Probing %s at %#02x (IRQ %d).\n",
		pdata->sensor_name ? pdata->sensor_name : "-no name-",
		client->addr, pdata->attn_gpio);

	//if (pdata->gpio_config) 
	{	
		dev_info(&client->dev, "%s: Configuring GPIOs.\n", __func__);
		error = synaptics_touchpad_gpio_setup(true);
		if (error < 0) {
			dev_err(&client->dev, "%s: Failed to configure GPIOs, code: %d.\n",	__func__, error);
			return error;
		}
		dev_info(&client->dev, "%s: Done with GPIO configuration.\n", __func__);
	}

	error = i2c_check_functionality(client->adapter, I2C_FUNC_I2C);
	if (!error) {
		dev_err(&client->dev, "i2c_check_functionality error %d.\n", error);
		return error;
	}

	rmi_phys = kzalloc(sizeof(struct rmi_phys_device), GFP_KERNEL);
	if (!rmi_phys)
		return -ENOMEM;

	data = kzalloc(sizeof(struct rmi_i2c_data), GFP_KERNEL);
	if (!data) {
		error = -ENOMEM;
		goto err_phys;
	}

	data->enabled = true;	/* We plan to come up enabled. */
	data->irq = gpio_to_irq(RMI_INT_GPIO);

	data->irq_flags = IRQF_ONESHOT | IRQF_TRIGGER_LOW;
	//data->irq_flags = IRQF_TRIGGER_FALLING;

	data->phys = rmi_phys;

	rmi_phys->data = data;
	rmi_phys->dev = &client->dev;

	rmi_phys->write = rmi_i2c_write;
	rmi_phys->write_block = rmi_i2c_write_block;
	rmi_phys->read = rmi_i2c_read;
	rmi_phys->read_block = rmi_i2c_read_block;
	rmi_phys->enable_device = enable_device;
	rmi_phys->disable_device = disable_device;

	rmi_phys->info.proto = phys_proto_name;

	mutex_init(&data->page_mutex);

	/* Setting the page to zero will (a) make sure the PSR is in a
	 * known state, and (b) make sure we can talk to the device.
	 */
	error = rmi_set_page(rmi_phys, 0);
	if (error) {
		dev_err(&client->dev, "Failed to set page select to 0.\n");
		goto err_data;
	}

	error = rmi_register_phys_device(rmi_phys);
	if (error) {
		dev_err(&client->dev,
			"failed to register physical driver at 0x%.2X.\n",
			client->addr);
		goto err_gpio;
	}
	i2c_set_clientdata(client, rmi_phys);

	//if (pdata->attn_gpio > 0) 
	{
		error = acquire_attn_irq(data);
		if (error < 0) {
			dev_err(&client->dev,
				"request_threaded_irq failed %d\n",
				pdata->attn_gpio);
			goto err_unregister;
		}
	}

#if defined(CONFIG_RMI4_DEV)
	error = gpio_export(pdata->attn_gpio, false);
	if (error) {
		dev_warn(&client->dev,
			 "WARNING: Failed to export ATTN gpio!\n");
		error = 0;
	} else {
		error = gpio_export_link(&(rmi_phys->rmi_dev->dev), "attn",
					pdata->attn_gpio);
		if (error) {
			dev_warn(&(rmi_phys->rmi_dev->dev),
				 "WARNING: Failed to symlink ATTN gpio!\n");
			error = 0;
		} else {
			dev_info(&(rmi_phys->rmi_dev->dev),
				"%s: Exported ATTN GPIO %d.", __func__,
				pdata->attn_gpio);
		}
	}

	printk("rmi_i2c_probe start");
	
#endif /* CONFIG_RMI4_DEV */

	//++ p11309 - 2013.06.19 for remove Hover

	PAN_DBG("Hover Disable...");
	regData = 0x02;
	error = rmi_i2c_write_block(rmi_phys, 0xFF, &regData, 1);
	error = (error < 0 ) ? error: 0;
	if (error) {
		dev_err(&client->dev, "Failed to set page select to 2...%d\n", error);
		goto err_data;
	}		
	data->page = 0x02;

	error = rmi_i2c_read_block(rmi_phys, 0xEB, &regOff, 1);
	error = (error < 0 ) ? error: 0;
	if (error) {
		dev_err(&client->dev, "Failed to read TP_REG_PDT_CUSTOM_CONTROL_BASE...%d\n", error);
		goto err_data;
	}	
	printk("0x%X...", regOff);

	error = rmi_i2c_read_block(rmi_phys, regOff + 0x00, &regData, 1);
	error = (error < 0 ) ? error: 0;
	if (error) {
		dev_err(&client->dev, "Failed to read TP_REG_PDT_CUSTOM_CONTROL_BASE...%d\n", error);
		goto err_data;
	}	
	printk("0x%X...", regData);

	regData = 0x04;
	error = rmi_i2c_write_block(rmi_phys, regOff + 0x00, &regData, 1);
	error = (error < 0 ) ? error: 0;
	if (error) {
		dev_err(&client->dev, "Failed to disable hover control...%d\n", error);
		goto err_data;
	}	

	error = rmi_i2c_read_block(rmi_phys, regOff + 0x00, &regData, 1);
	error = (error < 0 ) ? error: 0;
	if (error) {
		dev_err(&client->dev, "Failed to read TP_REG_PDT_CUSTOM_CONTROL_BASE....%d\n", error);
		goto err_data;
	}	
	printk("0x%X...", regData);

	regData = 0x64;
	error = rmi_i2c_write_block(rmi_phys, regOff + 0x53, &regData, 1);
	error = (error < 0 ) ? error: 0;
	if (error) {
		dev_err(&client->dev, "Failed to hover off doze.\n");
		goto err_data;
	}			

	regData = 0x0;
	error = rmi_i2c_write_block(rmi_phys, 0xFF, &regData, 1);
	error = (error < 0 ) ? error: 0;
	if (error) {
		dev_err(&client->dev, "Failed to set page select to 0.\n");
		goto err_data;
	}
	data->page = 0x00;
	PAN_DBG("OK!!\n");

	error = rmi_i2c_read_block(rmi_phys, 0x020A, &regData, 1);
	error = (error < 0 ) ? error: 0;
	if (error) {
		dev_err(&client->dev, "Failed to read TP_REG_PDT_CUSTOM_CONTROL_BASE....%d\n", error);
		goto err_data;
	}	
	PAN_DBG("0x020A Read = 0x%X...\n", regData);

	regData = 0x0;
	error = rmi_i2c_write_block(rmi_phys, 0x020A, &regData, 1);
	error = (error < 0 ) ? error: 0;
	if (error) {
		dev_err(&client->dev, "Failed to 0x020A Hover off.\n");
		goto err_data;
	}

	error = rmi_i2c_read_block(rmi_phys, 0x020A, &regData, 1);
	error = (error < 0 ) ? error: 0;
	if (error) {
		dev_err(&client->dev, "Failed to read TP_REG_PDT_CUSTOM_CONTROL_BASE....%d\n", error);
		goto err_data;
	}	
	PAN_DBG("0x020A Read = 0x%X...\n", regData);

	regData = 0x0;
	error = rmi_i2c_write_block(rmi_phys, 0xFF, &regData, 1);
	error = (error < 0 ) ? error: 0;
	if (error) {
		dev_err(&client->dev, "Failed to set page select to 0.\n");
		goto err_data;
	}
	data->page = 0x00;

	//-- p11309

	dev_info(&client->dev, "registered rmi i2c driver at %#04x.\n",
			client->addr);
	return 0;

err_unregister:
	rmi_unregister_phys_device(rmi_phys);
err_gpio:
//	if (pdata->gpio_config) pdata->gpio_config(pdata->gpio_data, false);
	synaptics_touchpad_gpio_setup(false);
err_data:
	kfree(data);
err_phys:
	kfree(rmi_phys);
	return error;
}

static int __devexit rmi_i2c_remove(struct i2c_client *client)
{
	struct rmi_phys_device *phys = i2c_get_clientdata(client);
//	struct rmi_device_platform_data *pd = client->dev.platform_data;

	disable_device(phys);
	rmi_unregister_phys_device(phys);
	kfree(phys->data);
	kfree(phys);

//	if (pd->gpio_config) pd->gpio_config(&pd->gpio_data, false);
	synaptics_touchpad_gpio_setup(false);

	return 0;
}

static const struct i2c_device_id rmi_id[] = {
	{ "rmi", 0 },
	{ "pantech_touch", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, rmi_id);

#ifdef CONFIG_OF
static struct of_device_id rmi_i2c_match_table[] = {
	{ .compatible = "synaptics,synaptics_s7020",},
	{ },
};
#else
#define rmi_i2c_match_table NULL
#endif

static struct i2c_driver rmi_i2c_driver = {
	.driver = {
		.owner	= THIS_MODULE,
		.name	= "pantech_touch",
		.of_match_table = rmi_i2c_match_table,
	},
	.id_table	= rmi_id,
	.probe		= rmi_i2c_probe,
	.remove		= __devexit_p(rmi_i2c_remove),
};

static int __init rmi_i2c_init(void)
{
	printk("rmi_i2c_init start..\n");
	return i2c_add_driver(&rmi_i2c_driver);
}

static void __exit rmi_i2c_exit(void)
{
	i2c_del_driver(&rmi_i2c_driver);
}

module_init(rmi_i2c_init);
module_exit(rmi_i2c_exit);

MODULE_AUTHOR("Christopher Heiny <cheiny@synaptics.com>");
MODULE_DESCRIPTION("RMI I2C driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(RMI_DRIVER_VERSION);
