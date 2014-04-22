/*
 * AK6921AF Driver
 */


/* drivers/misc/felica/ak6921af.c  (FeliCa/NFC Programmable output driver(FeliCa/NFC Lock)
 *
 * Copyright (C) 2013 Pantech 
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

/***************header***************/


#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/stddef.h>
#include <linux/fs.h>
#include <linux/err.h>
#include <linux/miscdevice.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/felica.h>
#include "felica_master.h"

#define	PRT_NAME "ak6921af"

#define HIGH  1
#define LOW   0

#define AK6921_RELOAD_CMD                 0x00
#define AK6921_REVERSING_CMD            0x01
#define AK6921_WRITE_ENABLE_CMD     0x02
#define AK6921_WRITE_DISABLE_CMD    0x03
#define AK6921_ACCESS_SWITCH_CMD   0x04
#define AK6921_STATUS_READ_CMD       0x05
#define AK6921_WRITE_DATA_CMD         0x06

#define AK6921_STATUS_OUT_MASK        0x01

#define AK6921_REG_ACCESS_MODE         0x00
#define AK6921_EEPROM_ACCESS_MODE   0x01
/*Device Setting*/
static struct i2c_client *this_client;

static int ak6921_i2c_read_data(unsigned char cmd, unsigned char* read_buf)
{
	int ret;

	struct i2c_msg msg[] = {
		{
			.addr = this_client->addr,
			.flags = 0,
			.len   = 1,
			.buf = &cmd,
		},
		{
			.addr = this_client->addr,
			.flags = I2C_M_RD,
			.len = 1,
			.buf = read_buf,
		},
	};

	if(read_buf == NULL) {
		pr_err(PRT_NAME " : read_data prt is NULL\n");
		return -EINVAL;
	}
			
	ret = i2c_transfer(this_client->adapter, msg, 2);
	if (ret < 0) {
		pr_err(PRT_NAME " : i2c read error : ret= %d\n",ret);
	}

	return ret;
}

static int ak6921_i2c_write_data(unsigned char cmd, unsigned char data)
{
	int ret;

	unsigned char tx_data[2] = {cmd, data};

	struct i2c_msg msg[] = {
		{
			.addr = this_client->addr,
			.flags = 0,
			.len   = 2,
			.buf = tx_data,
		},
	};

	ret = i2c_transfer(this_client->adapter, msg, 1);

	if (ret < 0) {
		pr_err(PRT_NAME " : i2c read error : ret= %d\n",ret);
	}

	return ret;
}

static int ak6921_i2c_write(unsigned char cmd)
{
	int ret;

	struct i2c_msg msg[] = {
		{
			.addr = this_client->addr,
			.flags = 0,
			.len   = 1,
			.buf   = &cmd,
		},
	};

	ret = i2c_transfer(this_client->adapter, msg, 1);

	if (ret < 0) {
		pr_err(PRT_NAME " : i2c write error : ret= %d\n",ret);
	}

	return ret;
}


int ak6921_write_data(unsigned char data)
{
	int ret = 0;

	ret = ak6921_i2c_write_data(AK6921_ACCESS_SWITCH_CMD, AK6921_EEPROM_ACCESS_MODE);
	if (ret < 0) {
		pr_err(PRT_NAME " : i2c write error: AK6921_ACCESS_SWITCH_CMD\n");
		goto err;
	}

	ret = ak6921_i2c_write(AK6921_WRITE_ENABLE_CMD);
	if (ret < 0) {
		pr_err(PRT_NAME " : i2c write error: AK6921_WRITE_ENABLE_CMD\n");
		goto err;
	}

	ret = ak6921_i2c_write_data(AK6921_WRITE_DATA_CMD, data);
	if (ret < 0) {
		pr_err(PRT_NAME " : i2c write error: AK6921_WRITE_DATA_CMD\n");
		goto err;
	}
	
	msleep(10);

	ret = ak6921_i2c_write(AK6921_WRITE_DISABLE_CMD);
	if (ret < 0) {
		pr_err(PRT_NAME " : i2c write error: AK6921_WRITE_DISABLE_CMD\n");
		goto err;
	}

	ret = ak6921_i2c_write_data(AK6921_ACCESS_SWITCH_CMD, AK6921_REG_ACCESS_MODE);
	if (ret < 0) {
		pr_err(PRT_NAME " : i2c write error: AK6921_ACCESS_SWITCH_CMD\n");
		goto err;
	}

	ret = ak6921_i2c_write(AK6921_RELOAD_CMD);
	if (ret < 0) {
		pr_err(PRT_NAME " : i2c write error: AK6921_RELOAD_CMD\n");
		goto err;
	}


	return 0;
	
err:
	return ret;
}

int ak6921_read_data(unsigned char *data)
{
	int ret;
	unsigned char buf;

	ret = ak6921_i2c_read_data(AK6921_STATUS_READ_CMD, &buf);

	if (ret < 0) {
		pr_err(PRT_NAME " : i2c read error: AK6921_STATUS_READ_CMD\n");
	}else {
		pr_debug(PRT_NAME ": AK6921 STATUS READ : 0x%x\n", buf);
	}
	
	*data = (buf & AK6921_STATUS_OUT_MASK);

	return ret;
}

static ssize_t ak6921_write(struct file *file, const char *buf,
				size_t count, loff_t *pos)
{
	char kbuf;
	int ret;

	if (1 != count  || !buf) {
		pr_err(PRT_NAME ": Error. Invalid arg ak6921 write.\n");
		return -EINVAL;
	}

	/* Copy value from user space */
	if (copy_from_user(&kbuf, buf, 1)) {
		pr_err(PRT_NAME ": Error. copy_from_user failure.\n");
		return -EFAULT;
	}

	if (HIGH == kbuf) {
		/* Write High to CEN */
		ret = ak6921_write_data(kbuf);
		if (ret < 0) {
			pr_err(PRT_NAME ": Error. CEN Write failure. \n");
			return ret;
		}
	} else if (LOW == kbuf) {
		/* Write LOW to CEN */
		ret = ak6921_write_data(kbuf);
		if (ret < 0) {
			pr_err(PRT_NAME ": Error. CEN Write failure. \n");
			return ret;
		}
	} else {
		pr_err(PRT_NAME ": Error. Invalid val ak6921 write.\n");
		return -EINVAL;
	}

	/* 1 byte write */
	return 1;
}

static ssize_t ak6921_read(struct file *file, char __user *buf,
				size_t count, loff_t *pos)
{
	int ret;
	char kbuf;

	pr_debug(PRT_NAME ":[%s]\n", __func__);

	if (1 != count || !buf) {
		pr_err(PRT_NAME ": Error. Invalid arg ak6921 read.\n");
		return -EINVAL;
	}

	ret = ak6921_read_data(&kbuf);
	if (ret < 0) {
		pr_err(PRT_NAME ": Error. CEN Read failure. \n");
		return ret;
	}

	/* Copy N value to user space */
	ret = copy_to_user(buf, &kbuf, 1);
	if (ret) {
		pr_err(PRT_NAME ": Error. copy_to_user failure.\n");
		return -EFAULT;
	}
	
	/* 1 byte read */
	return 1;


}

static int ak6921_open(struct inode *inode, struct file *file)
{
	pr_debug(PRT_NAME ":[ %s]\n", __func__);
	return 0;
}

static int ak6921_release(struct inode *inode, struct file *file)
{
	pr_debug(PRT_NAME ":[%s]\n", __func__);
	return 0;
}


static struct file_operations ak6921_fops = {
	.owner	 = THIS_MODULE,
	.open	 = ak6921_open,
	.release = ak6921_release,
	.read	 = ak6921_read,
	.write	 = ak6921_write,
//	.unlocked_ioctl = ak6921_ioctl, 
};


static struct miscdevice ak6921af_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "ak6921af",
	.fops = &ak6921_fops,
};

static int ak6921af_probe(struct i2c_client *client, const struct i2c_device_id * devid)
{
	int ret;

	if (i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		this_client = client;
	} else {
		pr_err(PRT_NAME " [%s]: need I2C_FUNC_I2C\n", __func__);
		return  -ENODEV;
	}
	

	ret = misc_register(&ak6921af_device);
	if (ret) {
		pr_err(PRT_NAME "[%s ]: misc_register failed\n", __func__);
		return ret;
	}
	return 0;
}


static int ak6921af_remove(struct i2c_client *client)
{
	misc_deregister(&ak6921af_device);
	return 0;
}


static struct of_device_id ak621af_match_table[] = {
	{ .compatible = "akm,ak6921af",},
	{ },
};
MODULE_DEVICE_TABLE(of, ak621af_match_table);

static const struct i2c_device_id ak6921af_id[] = {
	{ "ak6921af", 0 },
	{ }
};

static struct i2c_driver ak6921af_driver =
{

	.probe	 = ak6921af_probe,
	.id_table = ak6921af_id,
	.remove	 = ak6921af_remove,
	.driver = {
		.owner	= THIS_MODULE,
		.name	= "ak6921af",
		.of_match_table	= of_match_ptr(ak621af_match_table),
	},
};

static __init int ak6921af_init(void)
{
	int ret;
	

	ret = i2c_add_driver(&ak6921af_driver);
	if( ret < 0 ) {
		pr_err(PRT_NAME " [%s] : i2c_add_driver error", __func__);
		return ret;
	}
	
	return 0;
}

static void __exit ak6921af_exit(void)
{
	i2c_del_driver(&ak6921af_driver);
	return;
}


MODULE_LICENSE("GPL v2");

module_init(ak6921af_init);
module_exit(ak6921af_exit);

