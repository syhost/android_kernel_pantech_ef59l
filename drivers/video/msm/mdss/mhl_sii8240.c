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

#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/types.h>
#include <linux/vmalloc.h>
#include <linux/input.h>
#include <linux/usb/msm_hsusb.h>
#include <linux/ioport.h>
#include <mach/board.h>
#include <mach/gpio.h>
#include <mach/gpiomux.h>
#include "mhl_sii8240.h"

#include "mdss_fb.h"
#include "mdss_hdmi_tx.h"
#include "mdss_hdmi_edid.h"
#include "mdss.h"
#include "mdss_panel.h"
#include "mdss_io_util.h"

#include "mdss_hdmi_mhl.h"

#define MHL_8240_DRIVER_NAME "sii8240"
#define COMPATIBLE_NAME "qcom,mhl-sii8240"
#define MAX_CURRENT 700000

static struct mhl_tx_ctrl *mhl_ctrl;
///////////////////////////////////////////////
extern  int  SiiMhlInit(struct mhl_tx_ctrl *mhl_ctrl);
extern void SiiMhlTxInitialize(uint8_t pollIntervalMs );
extern void HalGpioUsbSw(bool mhl);
extern int mhl_device_discovery(const char *name, int *result);
///////////////////////////////////////////////
void mhl_register_mhl_ctrl(struct mhl_tx_ctrl *ctrl)
{
	if (ctrl)
		mhl_ctrl = ctrl;
}

static int mhl_sii_reg_config(struct i2c_client *client, bool enable)
{
	///static struct regulator *reg_8941_l24;
	static struct regulator *reg_8941_l02;
	///static struct regulator *reg_8941_smps3a;
	///static struct regulator *reg_8941_vdda;
	int rc=0;

	pr_info("%s\n", __func__);
#if 0	
	if (!reg_8941_l24) {
		reg_8941_l24 = regulator_get(&client->dev,
			"avcc_18");
		if (IS_ERR(reg_8941_l24)) {
			pr_err("could not get 8941 l24, rc = %ld\n",
				PTR_ERR(reg_8941_l24));
			return -ENODEV;
		}
		if (enable)
			rc = regulator_enable(reg_8941_l24);
		else
			rc = regulator_disable(reg_8941_l24);
		if (rc) {
			pr_err("'%s' regulator config[%u] failed, rc=%d\n",
			       "avcc_1.8V", enable, rc);
			goto l24_fail;
		} else {
			pr_debug("%s: vreg L24 %s\n",
				 __func__, (enable ? "enabled" : "disabled"));
		}
	}
#endif

	if (!reg_8941_l02) {
		reg_8941_l02 = regulator_get(&client->dev,
			"avcc_12");
		if (IS_ERR(reg_8941_l02)) {
			pr_err("could not get reg_8941_l02, rc = %ld\n",
				PTR_ERR(reg_8941_l02));
			goto l02_fail;
		}
		if (enable)
			rc = regulator_enable(reg_8941_l02);
		else
			rc = regulator_disable(reg_8941_l02);
		if (rc) {
			pr_info("'%s' regulator configure[%u] failed, rc=%d\n",
				 "avcc_1.2V", enable, rc);
			goto l02_fail;
		} else {
			pr_info("%s: vreg L02 %s\n",
				 __func__, (enable ? "enabled" : "disabled"));
		}
	}
	else
	{
		pr_info("reg_8941_l02 is not NULL\n");
	}
#if 0
	if (!reg_8941_smps3a) {
		reg_8941_smps3a = regulator_get(&client->dev,
			"smps3a");
		if (IS_ERR(reg_8941_smps3a)) {
			pr_err("could not get vreg smps3a, rc = %ld\n",
				PTR_ERR(reg_8941_smps3a));
			goto l02_fail;
		}
		if (enable)
			rc = regulator_enable(reg_8941_smps3a);
		else
			rc = regulator_disable(reg_8941_smps3a);
		if (rc) {
			pr_err("'%s' regulator config[%u] failed, rc=%d\n",
			       "SMPS3A", enable, rc);
			goto smps3a_fail;
		} else {
			pr_debug("%s: vreg SMPS3A %s\n",
				 __func__, (enable ? "enabled" : "disabled"));
		}
	}

	if (!reg_8941_vdda) {
		reg_8941_vdda = regulator_get(&client->dev,
			"vdda");
		if (IS_ERR(reg_8941_vdda)) {
			pr_err("could not get vreg vdda, rc = %ld\n",
				PTR_ERR(reg_8941_vdda));
			goto smps3a_fail;
		}
		if (enable)
			rc = regulator_enable(reg_8941_vdda);
		else
			rc = regulator_disable(reg_8941_vdda);
		if (rc) {
			pr_err("'%s' regulator config[%u] failed, rc=%d\n",
			       "VDDA", enable, rc);
			goto vdda_fail;
		} else {
			pr_debug("%s: vreg VDDA %s\n",
				 __func__, (enable ? "enabled" : "disabled"));
		}
	}
#endif
	return rc;

///vdda_fail:
///	regulator_disable(reg_8941_vdda);
///	regulator_put(reg_8941_vdda);
///smps3a_fail:
///	regulator_disable(reg_8941_smps3a);
///	regulator_put(reg_8941_smps3a);
l02_fail:
	regulator_disable(reg_8941_l02);
	regulator_put(reg_8941_l02);
///l24_fail:
///	regulator_disable(reg_8941_l24);
///	regulator_put(reg_8941_l24);

	return -EINVAL;
}



static int mhl_tx_get_dt_data(struct device *dev,
	struct mhl_tx_platform_data *pdata)
{
	int i, rc = 0;
	struct device_node *of_node = NULL;
	struct dss_gpio *temp_gpio = NULL;
	struct platform_device *hdmi_pdev = NULL;
	struct device_node *hdmi_tx_node = NULL;
	int dt_gpio;
	i = 0;

	if (!dev || !pdata) {
		pr_err("%s: invalid input\n", __func__);
		return -EINVAL;
	}

	of_node = dev->of_node;
	if (!of_node) {
		pr_err("%s: invalid of_node\n", __func__);
		goto error;
	}

	pr_info("%s: id=%d\n", __func__, dev->id);

	/* GPIOs */
	temp_gpio = NULL;
	temp_gpio = devm_kzalloc(dev, sizeof(struct dss_gpio), GFP_KERNEL);
	pr_info("%s: gpios allocd\n", __func__);
	if (!(temp_gpio)) {
		pr_err("%s: can't alloc %d gpio mem\n", __func__, i);
		goto error;
	}
	////////////////////////////////////////////////////////////////////////////////
	/* RESET */
	dt_gpio = of_get_named_gpio(of_node, "mhl-rst-gpio", 0);
	if (dt_gpio < 0) {
		pr_err("%s: Can't get mhl-rst-gpio\n", __func__);
		goto error;
	}

	temp_gpio->gpio = dt_gpio;
	snprintf(temp_gpio->gpio_name, 32, "%s", "mhl-rst-gpio");
	pr_info("%s: rst gpio=[%d],name=%s\n", __func__,
		 temp_gpio->gpio,temp_gpio->gpio_name);
	pdata->gpios[MHL_TX_RESET_GPIO] = temp_gpio;

	/* PWR */
	temp_gpio = NULL;
	temp_gpio = devm_kzalloc(dev, sizeof(struct dss_gpio), GFP_KERNEL);
	pr_info("%s: gpios allocd\n", __func__);
	if (!(temp_gpio)) {
		pr_err("%s: can't alloc %d gpio mem\n", __func__, i);
		goto error;
	}
	dt_gpio = of_get_named_gpio(of_node, "mhl-pwr-gpio", 0);
	if (dt_gpio < 0) {
		pr_err("%s: Can't get mhl-pwr-gpio\n", __func__);
		goto error;
	}

	temp_gpio->gpio = dt_gpio;
	snprintf(temp_gpio->gpio_name, 32, "%s", "mhl-pwr-gpio");
	pr_info("%s: pmic gpio=[%d],name=%s\n", __func__,
		 temp_gpio->gpio,temp_gpio->gpio_name);
	pdata->gpios[MHL_TX_PMIC_PWR_GPIO] = temp_gpio;

	////////////////////////////////////////////////////////////////////////////////
	/* INTR */
	temp_gpio = NULL;
	temp_gpio = devm_kzalloc(dev, sizeof(struct dss_gpio), GFP_KERNEL);
	pr_info("%s: gpios allocd\n", __func__);
	if (!(temp_gpio)) {
		pr_err("%s: can't alloc %d gpio mem\n", __func__, i);
		goto error;
	}
	dt_gpio = of_get_named_gpio(of_node, "mhl-intr-gpio", 0);
	if (dt_gpio < 0) {
		pr_err("%s: Can't get mhl-intr-gpio\n", __func__);
		goto error;
	}

	temp_gpio->gpio = dt_gpio;
	snprintf(temp_gpio->gpio_name, 32, "%s", "mhl-intr-gpio");
	pr_info("%s: intr gpio=[%d],name=%s\n", __func__,
		 temp_gpio->gpio,temp_gpio->gpio_name);
	pdata->gpios[MHL_TX_INTR_GPIO] = temp_gpio;

	////////////////////////////////////////////////////////////////////////////////
	/* MHL_USB_SW */
	temp_gpio = NULL;
	temp_gpio = devm_kzalloc(dev, sizeof(struct dss_gpio), GFP_KERNEL);
	pr_info("%s: gpios allocd\n", __func__);
	if (!(temp_gpio)) {
		pr_err("%s: can't alloc %d gpio mem\n", __func__, i);
		goto error;
	}
	dt_gpio = of_get_named_gpio(of_node, "mhl-sw-gpio", 0);
	if (dt_gpio < 0) {
		pr_err("%s: Can't get mhl-intr-gpio\n", __func__);
		goto error;
	}

	temp_gpio->gpio = dt_gpio;
	snprintf(temp_gpio->gpio_name, 32, "%s", "mhl-sw-gpio");
	pr_info("%s: mhl-sw-gpio=[%d],name=%s\n", __func__,
		 temp_gpio->gpio,temp_gpio->gpio_name);
	pdata->gpios[MHL_TX_MHL_USB_SEL_GPIO] = temp_gpio;
	////////////////////////////////////////////////////////////////////////////////
	/* PMIC  8941 GPIO 23*/
	temp_gpio = NULL;
	temp_gpio = devm_kzalloc(dev, sizeof(struct dss_gpio), GFP_KERNEL);
	pr_info("%s: gpios allocd\n", __func__);
	if (!(temp_gpio)) {
		pr_err("%s: can't alloc %d gpio mem\n", __func__, i);
		goto error;
	}
	dt_gpio = of_get_named_gpio(of_node, "mhl-usbid-gpio", 0);
	if (dt_gpio < 0) {
		pr_err("%s: Can't get mhl-intr-gpio\n", __func__);
		goto error;
	}

	temp_gpio->gpio = dt_gpio;
	snprintf(temp_gpio->gpio_name, 32, "%s", "mhl-usbid-gpio");
	pr_info("%s: mhl-usbid-gpio=[%d],name=%s\n", __func__,
		 temp_gpio->gpio,temp_gpio->gpio_name);
	pdata->gpios[MHL_TX_PMIC_USB_ID_GPIO] = temp_gpio;
	////////////////////////////////////////////////////////////////////////////////
	/* parse phandle for hdmi tx */
	hdmi_tx_node = of_parse_phandle(of_node, "qcom,hdmi-tx-map", 0);
	if (!hdmi_tx_node) {
		pr_err("%s: can't find hdmi phandle\n", __func__);
		goto error;
	}
	
	/* parse phandle for hdmi tx */
	hdmi_tx_node = of_parse_phandle(of_node, "qcom,hdmi-tx-map", 0);
	if (!hdmi_tx_node) {
		pr_err("%s: can't find hdmi phandle\n", __func__);
		goto error;
	}

	hdmi_pdev = of_find_device_by_node(hdmi_tx_node);
	if (!hdmi_pdev) {
		pr_err("%s: can't find the device by node\n", __func__);
		goto error;
	}
	pr_info("%s: hdmi_pdev [0X%x] to pdata->pdev\n",
	       __func__, (unsigned int)hdmi_pdev);

	pdata->hdmi_pdev = hdmi_pdev;


	return 0;
error:
	pr_err("%s: ret due to err\n", __func__);
	for (i = 0; i < MHL_TX_MAX_GPIO; i++)
		if (pdata->gpios[i])
			devm_kfree(dev, pdata->gpios[i]);
	return rc;
} /* mhl_tx_get_dt_data */

static int mhl_vreg_config(struct mhl_tx_ctrl *mhl_ctrl, uint8_t on)
{
	int ret;
	struct i2c_client *client = mhl_ctrl->i2c_handle;
	int pwr_gpio = mhl_ctrl->pdata->gpios[MHL_TX_PMIC_PWR_GPIO]->gpio;

	pr_info("%s\n", __func__);
	if (on) {
		ret = gpio_request(pwr_gpio,
		    mhl_ctrl->pdata->gpios[MHL_TX_PMIC_PWR_GPIO]->gpio_name);
		if (ret < 0) {
			pr_err("%s: mhl pwr gpio req failed: %d\n",
			       __func__, ret);
			return ret;
		}
		ret = gpio_direction_output(pwr_gpio, 1);
		if (ret < 0) {
			pr_err("%s: set gpio MHL_PWR_EN dircn failed: %d\n",
			       __func__, ret);
			goto vreg_config_failed;
		}

		ret = mhl_sii_reg_config(client, true);
		if (ret) {
			pr_err("%s: regulator enable failed\n", __func__);
			goto vreg_config_failed;
		}
		pr_info("%s: mhl sii power on successful\n", __func__);
	} else {
		pr_warn("%s: turning off pwr controls\n", __func__);
		mhl_sii_reg_config(client, false);
		gpio_free(pwr_gpio);
	}
	pr_info("%s: successful\n", __func__);
	return 0;
vreg_config_failed:
	gpio_free(pwr_gpio);
	return -EINVAL;
}

/*
 * Request for GPIO allocations
 * Set appropriate GPIO directions
 */
static int mhl_gpio_config(struct mhl_tx_ctrl *mhl_ctrl, int on)
{
	int ret;
	struct dss_gpio *temp_reset_gpio, *temp_intr_gpio;

	/* caused too many line spills */
	temp_reset_gpio = mhl_ctrl->pdata->gpios[MHL_TX_RESET_GPIO];
	temp_intr_gpio = mhl_ctrl->pdata->gpios[MHL_TX_INTR_GPIO];

	if (on) {
		if (gpio_is_valid(temp_reset_gpio->gpio)) {
			ret = gpio_request(temp_reset_gpio->gpio,
					   temp_reset_gpio->gpio_name);
			if (ret < 0) {
				pr_err("%s:rst_gpio=[%d] req failed:%d\n",
				       __func__, temp_reset_gpio->gpio, ret);
				return -EBUSY;
			}
			ret = gpio_direction_output(temp_reset_gpio->gpio, 0);
			if (ret < 0) {
				pr_err("%s: set dirn rst failed: %d\n",
				       __func__, ret);
				return -EBUSY;
			}
		}
		if (gpio_is_valid(temp_intr_gpio->gpio)) {
			ret = gpio_request(temp_intr_gpio->gpio,
					   temp_intr_gpio->gpio_name);
			if (ret < 0) {
				pr_err("%s: intr_gpio req failed: %d\n",
				       __func__, ret);
				return -EBUSY;
			}
			ret = gpio_direction_input(temp_intr_gpio->gpio);
			if (ret < 0) {
				pr_err("%s: set dirn intr failed: %d\n",
				       __func__, ret);
				return -EBUSY;
			}
			mhl_ctrl->i2c_handle->irq = gpio_to_irq(
				temp_intr_gpio->gpio);
			pr_info("%s: gpio_to_irq=%d\n",
				 __func__, mhl_ctrl->i2c_handle->irq);
		}
	} else {
		pr_warn("%s: freeing gpios\n", __func__);
		gpio_free(temp_intr_gpio->gpio);
		gpio_free(temp_reset_gpio->gpio);
	}
	pr_info("%s: successful\n", __func__);
	return 0;
}

void mhl_connection_work(struct work_struct *work)
{
	int result;
	pr_info("##################################################\n");
	pr_info("########connection Start !!!!!___#####################\n");
	pr_info("##################################################\n");
	pr_info("##################################################\n");	
	mhl_device_discovery("NAMI-siI8240",&result);
	pr_info("##################################################\n");
}

void mhl_start_work_queue(void)
{
	if(mhl_ctrl==NULL)
	{
		pr_err("mhl ctrl is NULL \n");
		return; 
	}
	
	///queue_work(mhl_ctrl->mhl_connection_workqueue, &mhl_ctrl->mhl_connection_work);
	pr_info("##################################################\n");
	pr_info("########_______change to MHL !!!!!___#####################\n");
	pr_info("##################################################\n");
	
	queue_delayed_work(mhl_ctrl->mhl_connection_workqueue, &mhl_ctrl->mhl_connection_work, msecs_to_jiffies(600));	
}

#if 0
static struct gpiomux_setting blsp9_i2c_config = {
	.func = GPIOMUX_FUNC_4,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct msm_gpiomux_config mhl_sii_blsp_configs[]  = {
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


static int mhl_i2c_probe(struct i2c_client *client,
			 const struct i2c_device_id *id)
{


	int rc = 0;
	struct mhl_tx_platform_data *pdata = NULL;
	struct mhl_tx_ctrl *mhl_ctrl;
	///struct usb_ext_notification *mhl_info = NULL;
	struct msm_hdmi_mhl_ops *hdmi_mhl_ops = NULL;

	pr_info("%s>>\n",__func__);

	
	mhl_ctrl = devm_kzalloc(&client->dev, sizeof(*mhl_ctrl), GFP_KERNEL);
	if (!mhl_ctrl) {
		pr_err("%s: FAILED: cannot alloc hdmi tx ctrl\n", __func__);
		rc = -ENOMEM;
		goto failed_no_mem;
	}

	if (client->dev.of_node) {
		pdata = devm_kzalloc(&client->dev,
			     sizeof(struct mhl_tx_platform_data), GFP_KERNEL);
		if (!pdata) {
			dev_err(&client->dev, "Failed to allocate memory\n");
			rc = -ENOMEM;
			goto failed_no_mem;
		}

		rc = mhl_tx_get_dt_data(&client->dev, pdata);
		if (rc) {
			pr_err("%s: FAILED: parsing device tree data; rc=%d\n",
				__func__, rc);
			goto failed_dt_data;
		}
		mhl_ctrl->i2c_handle = client;
		mhl_ctrl->pdata = pdata;
		i2c_set_clientdata(client, mhl_ctrl);
	}
	pr_info("%s:#1 gpio_to_irq=%d\n",__func__, mhl_ctrl->i2c_handle->irq);
	/*
	 * Regulator init
	 */
	rc = mhl_vreg_config(mhl_ctrl, 1);
	if (rc) {
		pr_err("%s: vreg init failed [%d]\n",
			__func__, rc);
		goto failed_probe;
	}

#if 0
	/*
	 * GPIO init
	 */
	rc = mhl_gpio_config(mhl_ctrl, 1);
	if (rc) {
		pr_err("%s: gpio init failed [%d]\n",
			__func__, rc);
		goto failed_probe;
	}
#endif	

	///msm_hdmi_register_mhl() ;//// !!!! 

	hdmi_mhl_ops = devm_kzalloc(&client->dev,
				    sizeof(struct msm_hdmi_mhl_ops),
				    GFP_KERNEL);
	if (!hdmi_mhl_ops) {
		pr_err("%s: alloc hdmi mhl ops failed\n", __func__);
		rc = -ENOMEM;
		goto failed_probe;
	}
 
	pr_info("%s: i2c client addr is [%x]\n", __func__, client->addr);
	if (mhl_ctrl->pdata->hdmi_pdev) {
		rc = msm_hdmi_register_mhl(mhl_ctrl->pdata->hdmi_pdev,
					   hdmi_mhl_ops, mhl_ctrl);
		if (rc) {
			pr_err("%s: register with hdmi failed\n", __func__);
			rc = -EPROBE_DEFER;
			goto failed_probe;
		}
	}

	if (!hdmi_mhl_ops || !hdmi_mhl_ops->tmds_enabled ||
	    !hdmi_mhl_ops->set_mhl_max_pclk) {
		pr_err("%s: func ptr is NULL\n", __func__);
		rc = -EINVAL;
		goto failed_probe;
	}
	mhl_ctrl->hdmi_mhl_ops = hdmi_mhl_ops;

	rc = hdmi_mhl_ops->set_mhl_max_pclk(
		mhl_ctrl->pdata->hdmi_pdev, MAX_MHL_PCLK);
	if (rc) {
		pr_err("%s: can't set max mhl pclk\n", __func__);
		goto failed_probe;
	}
	//////////////////////////////////////////////////////////////////////////////////
	////Create Work Queue 
	mhl_ctrl->mhl_connection_workqueue = create_singlethread_workqueue
			("mhl_connection_queue");
	
	INIT_DELAYED_WORK_DEFERRABLE(&mhl_ctrl->mhl_connection_work, mhl_connection_work);


	//////////////////////////////////////////////////////////////////////////////////
	///msm_gpiomux_install(mhl_sii_blsp_configs, ARRAY_SIZE(mhl_sii_blsp_configs)); 
	
	/// call Zoro MHL driver.
	mhl_register_mhl_ctrl(mhl_ctrl);

	SiiMhlInit(mhl_ctrl);
		
	return 0;

///failed_probe_pwr:
///	power_supply_unregister(&mhl_ctrl->mhl_psy);
failed_probe:
	free_irq(mhl_ctrl->i2c_handle->irq, mhl_ctrl);
	mhl_gpio_config(mhl_ctrl, 0);
	mhl_vreg_config(mhl_ctrl, 0);
	/* do not deep-free */
///	if (mhl_info)
///		devm_kfree(&client->dev, mhl_info);
failed_dt_data:
	if (pdata)
		devm_kfree(&client->dev, pdata);
failed_no_mem:
	if (mhl_ctrl)
		devm_kfree(&client->dev, mhl_ctrl);
	///mhl_info = NULL;
	pdata = NULL;
	mhl_ctrl = NULL;
	pr_err("%s: PROBE FAILED, rc=%d\n", __func__, rc);
	return rc;

}

int hpd_status=0;

void setupstream_hpd(int hpdStatus)
{
	struct msm_hdmi_mhl_ops *hdmi_mhl_ops;
	hdmi_mhl_ops=(struct msm_hdmi_mhl_ops *)mhl_ctrl->hdmi_mhl_ops;
	pr_info("setupstream_hpd:Current HPD=%d,%d\n",hpd_status,hpdStatus);
	if(hpd_status!=hpdStatus)
	{
		if(hdmi_mhl_ops!=NULL)
		{
			 hdmi_mhl_ops->set_upstream_hpd(mhl_ctrl->pdata->hdmi_pdev, hpdStatus);
		}
		hpd_status=hpdStatus;
	}
}		 
int GetMHLHPDStatus(void)
{
	return hpd_status;
}
static int mhl_i2c_remove(struct i2c_client *client)
{

	struct mhl_tx_ctrl *mhl_ctrl = i2c_get_clientdata(client);

	if (!mhl_ctrl) {
		pr_warn("%s: i2c get client data failed\n", __func__);
		return -EINVAL;
	}

	free_irq(mhl_ctrl->i2c_handle->irq, mhl_ctrl);
	mhl_gpio_config(mhl_ctrl, 0);
	mhl_vreg_config(mhl_ctrl, 0);
	///if (mhl_ctrl->mhl_info)
	///	devm_kfree(&client->dev, mhl_ctrl->mhl_info);
	if (mhl_ctrl->pdata)
		devm_kfree(&client->dev, mhl_ctrl->pdata);
	devm_kfree(&client->dev, mhl_ctrl);

	return 0;
}

static struct i2c_device_id mhl_sii_i2c_id[] = {
	{ MHL_8240_DRIVER_NAME, 0 },
	{ }
};


MODULE_DEVICE_TABLE(i2c, mhl_sii_i2c_id);

static struct of_device_id mhl_match_table[] = {
	{.compatible = COMPATIBLE_NAME,},
	{ },
};

static struct i2c_driver mhl_sii_i2c_driver = {
	.driver = {
		.name = MHL_8240_DRIVER_NAME,
		.owner = THIS_MODULE,
		.of_match_table = mhl_match_table,
	},
	.probe = mhl_i2c_probe,
	.remove =  mhl_i2c_remove,
	.id_table = mhl_sii_i2c_id,
};

module_i2c_driver(mhl_sii_i2c_driver);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("MHL SII 8240 TX Driver");
