#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>
#include <linux/qpnp/pin.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/leds.h>
#include <linux/pwm.h>
#include <linux/err.h>
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
#include "mdss_io_util.h"

#include "mdss.h"
#include "mdss_panel.h"
#include "mdss_dsi.h"
#include "mdss_debug.h"


#define BCKL_DRIVER_NAME "lm3530"
#define COMPATIBLE_NAME "qcom,bckl-lm3530"

#define LM3530_STATUS_ON		(1)
#define LM3530_STATUS_OFF		(0)

#define LM3530_TURN_ON		(1)
#define LM3530_TURN_OFF		(0)


struct bckl_tx_ctrl {
	struct i2c_client *i2c_handle;
	uint8_t cur_state;
	uint8_t mode;
};

struct bckl_tx_ctrl *bcklCtrl;

struct pm8xxx_i2c_addr_data{
	u8 addr;
	u8 data;
};


static struct pm8xxx_i2c_addr_data 	LedWk_Off_LM3530[] = {
	{ 0xA0, 0x00 },
	{ 0x30, 0x00 },
	{ 0x10, 0x00 }
};

/////////////////////////////////////////////////////
// internal function 
/////From Linux ..
static int lm3530_init(struct bckl_tx_ctrl *bckl_ctrl);
static int backLightOn(struct mdss_dsi_ctrl_pdata *ctrl);
static int backLightOff(struct mdss_dsi_ctrl_pdata *ctrl);

#define		LM3530_BL_MODE_MANUAL		0
#define		LM3530_BL_MODE_ALS			1
#define 	LM3530_BL_MODE_PWM			2

#define	LM3530_LK_INIT_VALUE			0x17	// LK addr(0x10)=0x17	
#define	LM3530_MAX_BRIGHTNESS			0x7F
#define	LM3530_DEFAULT_BRIGHTNESS		0x7F
#define	LM3530_DEFAULT_FULL_CURRENT		5		/// 22.5mA
#define LM3530_GEN_CONFIG               0x10
#define LM3530_ALS_CONFIG               0x20
#define LM3530_BRT_RAMP_RATE            0x30
#define LM3530_ALS_IMP_SELECT           0x41
#define LM3530_BRT_CTRL_REG             0xA0
#define LM3530_ALS_ZB0_REG              0x60
#define LM3530_ALS_ZB1_REG              0x61
#define LM3530_ALS_ZB2_REG              0x62
#define LM3530_ALS_ZB3_REG              0x63
#define LM3530_ALS_Z0T_REG              0x70
#define LM3530_ALS_Z1T_REG              0x71
#define LM3530_ALS_Z2T_REG              0x72
#define LM3530_ALS_Z3T_REG              0x73
#define LM3530_ALS_Z4T_REG              0x74
#define LM3530_REG_MAX                  14

/* General Control Register */
#define LM3530_EN_I2C_SHIFT             (0)
#define LM3530_RAMP_LAW_SHIFT           (1)
#define LM3530_MAX_CURR_SHIFT           (2)
#define LM3530_EN_PWM_SHIFT             (5)
#define LM3530_PWM_POL_SHIFT            (6)
#define LM3530_EN_PWM_SIMPLE_SHIFT      (7)

#define LM3530_ENABLE_I2C               (1 << LM3530_EN_I2C_SHIFT)
#define LM3530_ENABLE_PWM               (1 << LM3530_EN_PWM_SHIFT)
#define LM3530_POL_LOW                  (1 << LM3530_PWM_POL_SHIFT)
#define LM3530_ENABLE_PWM_SIMPLE        (1 << LM3530_EN_PWM_SIMPLE_SHIFT)

/* ALS Config Register Options */
#define LM3530_ALS_AVG_TIME_SHIFT       (0)
#define LM3530_EN_ALS_SHIFT             (3)
#define LM3530_ALS_SEL_SHIFT            (5)

#define LM3530_ENABLE_ALS               (3 << LM3530_EN_ALS_SHIFT)

/* Brightness Ramp Rate Register */
#define LM3530_BRT_RAMP_FALL_SHIFT      (0)
#define LM3530_BRT_RAMP_RISE_SHIFT      (3)

/* ALS Resistor Select */
#define LM3530_ALS1_IMP_SHIFT           (0)
#define LM3530_ALS2_IMP_SHIFT           (4)

/* Zone Boundary Register defaults */
#define LM3530_ALS_ZB_MAX               (4)
#define LM3530_ALS_WINDOW_mV            (1000)
#define LM3530_ALS_OFFSET_mV            (4)

/* Zone Target Register defaults */
#define LM3530_DEF_ZT_0                 (0x7F)
#define LM3530_DEF_ZT_1                 (0x66)
#define LM3530_DEF_ZT_2                 (0x4C)
#define LM3530_DEF_ZT_3                 (0x33)
#define LM3530_DEF_ZT_4                 (0x19)

/* 7 bits are used for the brightness : LM3530_BRT_CTRL_REG */
#define MAX_BRIGHTNESS                  (127)



static const u8 lm3530_reg[LM3530_REG_MAX] = {
        LM3530_GEN_CONFIG,
        LM3530_ALS_CONFIG,
        LM3530_BRT_RAMP_RATE,
        LM3530_ALS_IMP_SELECT,
        LM3530_BRT_CTRL_REG,
        LM3530_ALS_ZB0_REG,
        LM3530_ALS_ZB1_REG,
        LM3530_ALS_ZB2_REG,
        LM3530_ALS_ZB3_REG,
        LM3530_ALS_Z0T_REG,
        LM3530_ALS_Z1T_REG,
        LM3530_ALS_Z2T_REG,
        LM3530_ALS_Z3T_REG,
        LM3530_ALS_Z4T_REG,
};

/*
 * struct lm3530_als_data
 * @config  : value of ALS configuration register
 * @imp_sel : value of ALS resistor select register
 * @zone    : values of ALS ZB(Zone Boundary) registers
 */
struct lm3530_als_data {
        u8 config;
        u8 imp_sel;
        u8 zones[LM3530_ALS_ZB_MAX];
};
static char MCAP[2]		= {0xb0, 0x00};
static char NOP[2]			= {0x00, 0x00};

static char Backlight_CTL_B9[8]  	= {0xb9,0x07, 0x90, 0x1e, 0x10,0x1e, 0x32, 0xff,};
static char Backlight_CTL_CE[24]  	= {0xce,0x35, 0x40, 0x48, 0x56,0x67, 0x78, 0x88, 0x98,0xa7, 0xb5, 0xc3, 0xd1,0xde, 0xe9, 0xf2, 0xfa,0xff, 0x01, 0x01, 0x00,0x00, 0x00, 0x24,};
static char WRDISBV[2]  	= {0x51,0xFF,};
static char WCTRLDIS[2]	= {0x53, 0x00};
static char CABC[2]		= {0x55, 0x02};
static char CABC_MIN[2]	= {0x5e, 0x00};
static char MCAP_BAN[2]			= {0xb0, 0x02};
static char Set_column_address[5] 	= {0x2a,0x00,0x00,0x04,0x37,};
static char Set_page_address[5]  	= {0x2b,0x00,0x00,0x07,0x7F,};
static char exit_sleep[2]	= {0x11, 0x00};

static char deep_standby1[2]	= {0xb0, 0x02};
static char deep_standby2[2]	= {0xb1, 0x01};


static struct dsi_cmd_desc sharp_cmd_setting_cmds[] = {
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(Set_column_address)}, Set_column_address},	
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(Set_page_address)}, Set_page_address},		
};
static struct dsi_cmd_desc sharp_blc_mode_change_cmds[] ={
	{{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(MCAP)}, 	MCAP},				/* B0 MCAP APP */
	{{DTYPE_DCS_WRITE,  1, 0, 0, 0, sizeof(NOP)}, 	NOP},				/* 00 NOP */
	{{DTYPE_DCS_WRITE,  1, 0, 0, 0, sizeof(NOP)}, 	NOP},				/* 00 NOP */
	{{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(Backlight_CTL_B9)} , 	Backlight_CTL_B9},	/* B9 BACKLIGHT CTRL2 */
	{{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(Backlight_CTL_CE)}, 	Backlight_CTL_CE},	/* CE BACKLIGHT CTRL6 */
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(WRDISBV)}, 	WRDISBV},			/* 51 WRITE DISBV */
	{{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(WCTRLDIS)}, 	WCTRLDIS},			/* 53 WRITE CTRL DISPLAY */
	{{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(CABC)}, 		CABC},				/* 55 CABC 00:FF 01:UI 02:STILL 03:VIDEO */
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(CABC_MIN)}, 	CABC_MIN},			/* 5E CABC MINIMUM BRIGHTNESS */
	{{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(MCAP_BAN)}, MCAP_BAN},			/* B0 MCAP Access BAN */
	{{DTYPE_DCS_WRITE, 1, 0, 0, 200, sizeof(exit_sleep)}, exit_sleep},

};
static struct dsi_cmd_desc sharp_deep_standby_cmds[] = {
	{{DTYPE_GEN_WRITE2, 1, 0, 0, 0, sizeof(deep_standby1)}, deep_standby1},
	{{DTYPE_DCS_WRITE,  1, 0, 0, 0, sizeof(NOP)}, 	NOP},
	{{DTYPE_DCS_WRITE,  1, 0, 0, 0, sizeof(NOP)}, 	NOP},	
	{{DTYPE_GEN_WRITE2, 1, 0, 0, 40, sizeof(deep_standby2)}, deep_standby2},
};

void sharp_display_on(struct mdss_dsi_ctrl_pdata *ctrl,
			struct dsi_panel_cmds *pcmds)
{
	struct dcs_cmd_req cmdreq;
	pr_err("%s----------\n", __func__);
	memset(&cmdreq, 0, sizeof(cmdreq));
	cmdreq.cmds = pcmds->cmds;
	cmdreq.cmds_cnt = pcmds->cmd_cnt;
	cmdreq.flags = CMD_REQ_COMMIT;
	cmdreq.rlen = 0;
	cmdreq.cb = NULL;
	mdss_dsi_cmdlist_put(ctrl, &cmdreq);
}


void sharp_set_settting_cmds(struct mdss_dsi_ctrl_pdata *ctrl)
{
	struct dcs_cmd_req cmdreq;

	
	memset(&cmdreq, 0, sizeof(cmdreq));
	cmdreq.cmds = sharp_cmd_setting_cmds;
	cmdreq.cmds_cnt = 2;
	cmdreq.flags = CMD_REQ_COMMIT;
	cmdreq.rlen = 0;
	cmdreq.cb = NULL;
	mdss_dsi_cmdlist_put(ctrl, &cmdreq);
}
void sharp_blc_on(struct mdss_dsi_ctrl_pdata *ctrl)
{
	struct dcs_cmd_req cmdreq;

	pr_err("%s:\n", __func__);
	
	memset(&cmdreq, 0, sizeof(cmdreq));
	cmdreq.cmds = sharp_blc_mode_change_cmds;
	cmdreq.cmds_cnt = 11;
	cmdreq.flags = CMD_REQ_COMMIT ;
	cmdreq.rlen = 0;
	cmdreq.cb = NULL;
	mdss_dsi_cmdlist_put(ctrl, &cmdreq);
}

void sharp_blc_off(struct mdss_dsi_ctrl_pdata *ctrl)
{
	struct dcs_cmd_req cmdreq;

	pr_err("%s:\n", __func__);

	backLightOff(ctrl);
	memset(&cmdreq, 0, sizeof(cmdreq));
	cmdreq.cmds = sharp_deep_standby_cmds;
	cmdreq.cmds_cnt = 4;
	cmdreq.flags = CMD_REQ_COMMIT ;
	cmdreq.rlen = 0;
	cmdreq.cb = NULL;
	mdss_dsi_cmdlist_put(ctrl, &cmdreq);
}


void sharp_panel_on_init(struct mdss_panel_data *pdata)
{
	struct mdss_dsi_ctrl_pdata *ctrl = NULL;


	ctrl = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);
	
 	mdss_set_tx_power_mode(0, pdata);
 	sharp_set_settting_cmds(ctrl);
	sharp_blc_on(ctrl);

	//sharp_display_on(ctrl,&ctrl->on_sec_cmds);
	backLightOn(ctrl);
}


void mdss_dsi_panel_reset_nami(struct mdss_panel_data *pdata, int enable){

	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;

	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return;
	}

	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);
	if (!gpio_is_valid(ctrl_pdata->bl_en_gpio)) {
		pr_debug("%s:%d, bl enable  line not configured\n",
			   __func__, __LINE__);
		return;
	}

	if (!gpio_is_valid(ctrl_pdata->rst_gpio)) {
		pr_debug("%s:%d, reset line  not configured\n",
			   __func__, __LINE__);
		return;
	}
	if (!gpio_is_valid(ctrl_pdata->lcd_vci_reg_en_n)) {
		pr_debug("%s:%d, lcd vci line not configured\n",
			   __func__, __LINE__);
		return;
	}
	if (!gpio_is_valid(ctrl_pdata->lcd_vci_reg_en_p)) {
		pr_debug("%s:%d, lcd vci line not configured\n",
			   __func__, __LINE__);
		return;
	}
	if (!gpio_is_valid(ctrl_pdata->lcd_vci_reg_en)) {
		pr_err("%s:%d, lcd vci line not configured\n",
			   __func__, __LINE__);
		return;
	}	
	if (!gpio_is_valid(ctrl_pdata->bl_swire_gpio)) {
		pr_debug("%s:%d, bl swire line not configured\n",
			   __func__, __LINE__);
		return;
	}	

	pr_err("%s------------------------------->: enable = %d\n", __func__, enable);

	if (enable) {

		gpio_set_value((ctrl_pdata->rst_gpio), 0);
		msleep(20);
		wmb();		
		gpio_set_value((ctrl_pdata->lcd_vci_reg_en_p), 1);
		gpio_set_value((ctrl_pdata->lcd_vci_reg_en_n), 1);
		msleep(20);
		msleep(20);
		wmb();
	
		gpio_set_value((ctrl_pdata->rst_gpio), 1);
		msleep(10);
		wmb();
		
		gpio_set_value((ctrl_pdata->bl_en_gpio), 1);
#if (BOARD_VER == PT10)
		gpio_set_value((ctrl_pdata->bl_swire_gpio), 1);
#endif
		ctrl_pdata->lcd_on_ckeck  = true;//command mode check

	} else {
		gpio_set_value((ctrl_pdata->rst_gpio), 0);
		gpio_set_value((ctrl_pdata->lcd_vci_reg_en_p), 0);
		gpio_set_value((ctrl_pdata->lcd_vci_reg_en_n), 0);
#if (BOARD_VER == PT10)	
		gpio_set_value((ctrl_pdata->bl_swire_gpio), 0);
#endif	
		msleep(10);
		gpio_set_value((ctrl_pdata->bl_en_gpio), 0);
		wmb();
	}

}


int mdss_dsi_panel_power_on_nami(struct mdss_panel_data *pdata, int enable)
{
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	int ret;
	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);
	pr_debug("%s: enable=%d\n", __func__, enable);
	ret = gpio_direction_output(ctrl_pdata->lcd_vddio_reg_en, 1);
	if (enable) {
		if (ctrl_pdata->power_data.num_vreg > 0) {
			ret = msm_dss_enable_vreg(
				ctrl_pdata->power_data.vreg_config,
				ctrl_pdata->power_data.num_vreg, 1);
			if (ret) {
				pr_err("%s:Failed to enable regulators.rc=%d\n",
					__func__, ret);
				return ret;
			}

			/*
			 * A small delay is needed here after enabling
			 * all regulators and before issuing panel reset
			 */
			msleep(20);
		} else {

			ret = regulator_set_optimum_mode(
				(ctrl_pdata->shared_pdata).vdd_io_rsp_vreg, 100000);
			if (ret < 0) {
					pr_err("%s: vdd_io_rsp_vreg set opt mode failed.\n",
							       __func__);
				return ret;
			}
			
			ret = regulator_set_optimum_mode
			 	 ((ctrl_pdata->shared_pdata).vdda_vreg, 100000);
			if (ret < 0) {
				pr_err("%s: vdda_vreg set opt mode failed.\n",
						       __func__);
				return ret;
			}

			gpio_set_value((ctrl_pdata->lcd_vddio_reg_en), 1);
			ret = regulator_enable(
				(ctrl_pdata->shared_pdata).vdd_io_rsp_vreg);
			if (ret) {
				pr_err("%s: Failed to enable regulator.\n",
					__func__);
				return ret;
			}	

			msleep(200);

			ret = regulator_enable(
				(ctrl_pdata->shared_pdata).vdda_vreg);
			if (ret) {
				pr_err("%s: Failed to enable regulator.\n",
					__func__);
				return ret;
			}
		}

		if (pdata->panel_info.panel_power_on == 0)
			mdss_dsi_panel_reset_nami(pdata, 1);

	} 
	else {
		mdss_dsi_panel_reset_nami(pdata, 0);

		if (ctrl_pdata->power_data.num_vreg > 0) {
			ret = msm_dss_enable_vreg(
				ctrl_pdata->power_data.vreg_config,
				ctrl_pdata->power_data.num_vreg, 0);
		if (ret) {
			pr_err("%s: Failed to disable regs.rc=%d\n",
				__func__, ret);
			return ret;
		}
		} else {

			ret = regulator_disable(
				(ctrl_pdata->shared_pdata).vdda_vreg);
			if (ret) {
				pr_err("%s: Failed to disable regulator.\n",
					__func__);
				return ret;
			}

			gpio_set_value((ctrl_pdata->lcd_vddio_reg_en), 0);
		
			ret = regulator_disable(
				(ctrl_pdata->shared_pdata).vdd_io_rsp_vreg);
			if (ret) {
				pr_err("%s: Failed to disable regulator.\n",
					__func__);
				return ret;
			}

			ret = regulator_set_optimum_mode(
				(ctrl_pdata->shared_pdata).vdd_io_rsp_vreg, 100);
			if (ret < 0) {
				pr_err("%s: vdd_io_vreg set opt mode failed.\n",
					__func__);
				return ret;
			}	

			ret = regulator_set_optimum_mode(
				(ctrl_pdata->shared_pdata).vdda_vreg, 100);
			if (ret < 0) {
				pr_err("%s: vdda_vreg set opt mode failed.\n",
					__func__);
				return ret;
			}
		}
	}
	
	return 0;
}

int mdss_dsi_regulator_init_nami(struct platform_device *pdev)
{
	int ret = 0;
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	struct dsi_drv_cm_data *dsi_drv = NULL;

	if (!pdev) {
		pr_err("%s: invalid input\n", __func__);
		return -EINVAL;
	}

	ctrl_pdata = platform_get_drvdata(pdev);
	if (!ctrl_pdata) {
		pr_err("%s: invalid driver data\n", __func__);
		return -EINVAL;
	}

	dsi_drv = &(ctrl_pdata->shared_pdata);
	if (ctrl_pdata->power_data.num_vreg > 0) {
		ret = msm_dss_config_vreg(&pdev->dev,
				ctrl_pdata->power_data.vreg_config,
				ctrl_pdata->power_data.num_vreg, 1);
	} else {

		dsi_drv->vdd_io_rsp_vreg = devm_regulator_get(&pdev->dev, "vddio_rsp");
		if (IS_ERR(dsi_drv->vdd_io_rsp_vreg)) {
			pr_err("%s: could not get vddio reg, rc=%ld\n",
				__func__, PTR_ERR(dsi_drv->vdd_io_rsp_vreg));
			return PTR_ERR(dsi_drv->vdd_io_rsp_vreg);
		}

		ret = regulator_set_voltage(dsi_drv->vdd_io_rsp_vreg, 1800000,
				1800000);
		if (ret) {
			pr_err("%s: set voltage failed on vddio_rsp vreg, rc=%d\n",
				__func__, ret);
			return ret;
		}
		ret = regulator_enable(dsi_drv->vdd_io_rsp_vreg);
		if (ret) {
			pr_err("%s: Failed to enable regulator.\n",
			    __func__);
			return ret;
		}

		dsi_drv->vdda_vreg = devm_regulator_get(&pdev->dev, "vdda");
		if (IS_ERR(dsi_drv->vdda_vreg)) {
			pr_err("%s: could not get vdda vreg, rc=%ld\n",
				__func__, PTR_ERR(dsi_drv->vdda_vreg));
			return PTR_ERR(dsi_drv->vdda_vreg);
		}

		ret = regulator_set_voltage(dsi_drv->vdda_vreg, 1200000,
				1200000);
		if (ret) {
			pr_err("%s: set voltage failed on vdda vreg, rc=%d\n",
				__func__, ret);
			return ret;
		}
	}
	return 0;
}
int dsi_panel_device_register_nami(struct mdss_dsi_ctrl_pdata *ctrl_pdata,struct platform_device *pdev){
	
	int rc;
	
	ctrl_pdata->disp_te_gpio = of_get_named_gpio(pdev->dev.of_node,
						     "qcom,te-gpio", 0);
	if (!gpio_is_valid(ctrl_pdata->disp_te_gpio)) {
		pr_err("%s:%d, Disp_te gpio not specified\n",
						__func__, __LINE__);
	} else {
		rc = gpio_request(ctrl_pdata->disp_te_gpio, "disp_te");
		if (rc) {
			pr_err("request TE gpio failed, rc=%d\n",
			       rc);
			gpio_free(ctrl_pdata->disp_te_gpio);
			return -ENODEV;
		}
		rc = gpio_tlmm_config(GPIO_CFG(
				ctrl_pdata->disp_te_gpio, 1,
				GPIO_CFG_INPUT,
				GPIO_CFG_PULL_DOWN,
				GPIO_CFG_2MA),
				GPIO_CFG_ENABLE);

		if (rc) {
			pr_err("%s: unable to config tlmm = %d\n",
				__func__, ctrl_pdata->disp_te_gpio);
			gpio_free(ctrl_pdata->disp_te_gpio);
			return -ENODEV;
		}

		rc = gpio_direction_input(ctrl_pdata->disp_te_gpio);
		if (rc) {
			pr_err("set_direction for disp_en gpio failed, rc=%d\n",
			       rc);
			gpio_free(ctrl_pdata->disp_te_gpio);
			return -ENODEV;
		}
		pr_debug("%s: te_gpio=%d\n", __func__,
					ctrl_pdata->disp_te_gpio);
	}

//---------------------------------------------------------------//
	ctrl_pdata->bl_en_gpio = of_get_named_gpio(pdev->dev.of_node,
						     "qcom,bl-en-gpio", 0);

	if (!gpio_is_valid(ctrl_pdata->bl_en_gpio)) {
		pr_err("%s:%d, Disp_en gpio not specified\n",
						__func__, __LINE__);
	} else {
		rc = gpio_request(ctrl_pdata->bl_en_gpio, "bl_enable");
		if (rc) {
			pr_err("request bl enable gpio failed, rc=%d\n",rc);
			gpio_free(ctrl_pdata->bl_en_gpio);
			if (gpio_is_valid(ctrl_pdata->disp_te_gpio))
				gpio_free(ctrl_pdata->disp_te_gpio);
			return -ENODEV;
		}
		rc = gpio_direction_output(ctrl_pdata->bl_en_gpio, 1);
		if (rc) {
			pr_err("set_direction for bl_en_gpio gpio failed, rc=%d\n",rc);
			gpio_free(ctrl_pdata->bl_en_gpio);
			if (gpio_is_valid(ctrl_pdata->disp_te_gpio))
				gpio_free(ctrl_pdata->disp_te_gpio);			
			return -ENODEV;
		}	
	}

	//----------------------------1.8 V setting-----------------------//
	ctrl_pdata->lcd_vddio_reg_en = of_get_named_gpio(pdev->dev.of_node,
						 "qcom,vddio-reg-gpio", 0);
	if (!gpio_is_valid(ctrl_pdata->lcd_vddio_reg_en)) {
		pr_err("%s:%d, 1.8v Ext Regulator gpio not specified\n",
						__func__, __LINE__);
	} else {
		rc = gpio_request(ctrl_pdata->lcd_vddio_reg_en, "lcd_vddio_ext_reg");
		if (rc) {
			pr_err("request 1.8v Ext Regulator gpio failed, rc=%d\n",rc);
			gpio_free(ctrl_pdata->lcd_vddio_reg_en);
			if (gpio_is_valid(ctrl_pdata->disp_te_gpio))
				gpio_free(ctrl_pdata->disp_te_gpio);
			if (gpio_is_valid(ctrl_pdata->bl_en_gpio))
				gpio_free(ctrl_pdata->bl_en_gpio);
			return -ENODEV;
		}
		rc = gpio_direction_output(ctrl_pdata->lcd_vddio_reg_en, 1);
		if (rc) {
			pr_err("set_direction for lcd_vddio_ext_reg gpio failed, rc=%d\n",rc);
			gpio_free(ctrl_pdata->lcd_vddio_reg_en);
			if (gpio_is_valid(ctrl_pdata->disp_te_gpio))
				gpio_free(ctrl_pdata->disp_te_gpio);
			if (gpio_is_valid(ctrl_pdata->bl_en_gpio))
				gpio_free(ctrl_pdata->bl_en_gpio);
			return -ENODEV;
		}
	}

	//--------------------------5.6 V vci-p setting-----------------------------//
	
	ctrl_pdata->lcd_vci_reg_en_p = of_get_named_gpio(pdev->dev.of_node,
						 "qcom,vci-reg-gpio-p", 0);
	if (!gpio_is_valid(ctrl_pdata->lcd_vci_reg_en_p)) {
		pr_err("%s:%d, 2.2v Ext Regulator gpio not specified\n",
						__func__, __LINE__);
	} else {
		rc = gpio_request(ctrl_pdata->lcd_vci_reg_en_p, "lcd_vci_ext_reg_p");
		if (rc) {
			pr_err("request 5.6v Ext Regulator gpio failed, rc=%d\n",rc);
			gpio_free(ctrl_pdata->lcd_vci_reg_en_p);	
			if (gpio_is_valid(ctrl_pdata->disp_te_gpio))
				gpio_free(ctrl_pdata->disp_te_gpio);
			if (gpio_is_valid(ctrl_pdata->bl_en_gpio))
				gpio_free(ctrl_pdata->bl_en_gpio);		
			if (gpio_is_valid(ctrl_pdata->lcd_vddio_reg_en))
				gpio_free(ctrl_pdata->lcd_vddio_reg_en);
			return -ENODEV;
		}
		rc = gpio_direction_output(ctrl_pdata->lcd_vci_reg_en_p, 1);
		if (rc) {
			pr_err("set_direction for lcd_vci_ext_reg_p gpio failed, rc=%d\n",rc);
			gpio_free(ctrl_pdata->lcd_vci_reg_en_p);	
			if (gpio_is_valid(ctrl_pdata->disp_te_gpio))
				gpio_free(ctrl_pdata->disp_te_gpio);
			if (gpio_is_valid(ctrl_pdata->bl_en_gpio))
				gpio_free(ctrl_pdata->bl_en_gpio);		
			if (gpio_is_valid(ctrl_pdata->lcd_vddio_reg_en))
				gpio_free(ctrl_pdata->lcd_vddio_reg_en);
			return -ENODEV;
		}
	}	

	//--------------------------5.6 V vci-n setting-----------------------------//
	
	ctrl_pdata->lcd_vci_reg_en_n = of_get_named_gpio(pdev->dev.of_node,
						 "qcom,vci-reg-gpio-n", 0);
	if (!gpio_is_valid(ctrl_pdata->lcd_vci_reg_en_n)) {
		pr_err("%s:%d, 2.2v Ext Regulator gpio not specified\n",
						__func__, __LINE__);
	} else {
		rc = gpio_request(ctrl_pdata->lcd_vci_reg_en_n, "lcd_vci_ext_reg_n");
		if (rc) {
			pr_err("request 5.6v Ext Regulator gpio failed, rc=%d\n",rc);
			gpio_free(ctrl_pdata->lcd_vci_reg_en_n);	
			if (gpio_is_valid(ctrl_pdata->disp_te_gpio))
				gpio_free(ctrl_pdata->disp_te_gpio);
			if (gpio_is_valid(ctrl_pdata->bl_en_gpio))
				gpio_free(ctrl_pdata->bl_en_gpio);		
			if (gpio_is_valid(ctrl_pdata->lcd_vddio_reg_en))
				gpio_free(ctrl_pdata->lcd_vddio_reg_en);
			if (gpio_is_valid(ctrl_pdata->lcd_vci_reg_en_p))
				gpio_free(ctrl_pdata->lcd_vci_reg_en_p);
			return -ENODEV;
		}
		rc = gpio_direction_output(ctrl_pdata->lcd_vci_reg_en_n, 1);
		if (rc) {
			pr_err("set_direction for lcd_vci_ext_reg_n gpio failed, rc=%d\n", rc);
			gpio_free(ctrl_pdata->lcd_vci_reg_en_n);		
			if (gpio_is_valid(ctrl_pdata->disp_te_gpio))
				gpio_free(ctrl_pdata->disp_te_gpio);
			if (gpio_is_valid(ctrl_pdata->bl_en_gpio))
				gpio_free(ctrl_pdata->bl_en_gpio);		
			if (gpio_is_valid(ctrl_pdata->lcd_vddio_reg_en))
				gpio_free(ctrl_pdata->lcd_vddio_reg_en);
			if (gpio_is_valid(ctrl_pdata->lcd_vci_reg_en_p))
				gpio_free(ctrl_pdata->lcd_vci_reg_en_p);
			return -ENODEV;
		}
	}	
	//--------------------------lcd reset setting-----------------------------//
	ctrl_pdata->rst_gpio = of_get_named_gpio(pdev->dev.of_node,
						 "qcom,rst-gpio", 0);
	if (!gpio_is_valid(ctrl_pdata->rst_gpio)) {
		pr_err("%s:%d, reset gpio not specified\n",
						__func__, __LINE__);
	} else {
		rc = gpio_request(ctrl_pdata->rst_gpio, "disp_rst_n");
		if (rc) {
			pr_err("request reset gpio failed, rc=%d\n",rc);
			gpio_free(ctrl_pdata->rst_gpio);		
			if (gpio_is_valid(ctrl_pdata->disp_te_gpio))
				gpio_free(ctrl_pdata->disp_te_gpio);
			if (gpio_is_valid(ctrl_pdata->bl_en_gpio))
				gpio_free(ctrl_pdata->bl_en_gpio);		
			if (gpio_is_valid(ctrl_pdata->lcd_vddio_reg_en))
				gpio_free(ctrl_pdata->lcd_vddio_reg_en);
			if (gpio_is_valid(ctrl_pdata->lcd_vci_reg_en_p))
				gpio_free(ctrl_pdata->lcd_vci_reg_en_p);
			if (gpio_is_valid(ctrl_pdata->lcd_vci_reg_en_n))
				gpio_free(ctrl_pdata->lcd_vci_reg_en_n);
			return -ENODEV;
		}
		rc = gpio_direction_output(ctrl_pdata->rst_gpio, 1);
		if (rc) {
			pr_err("set_direction for lcd rst gpio failed, rc=%d\n",rc);
			gpio_free(ctrl_pdata->rst_gpio);
			if (gpio_is_valid(ctrl_pdata->disp_te_gpio))
				gpio_free(ctrl_pdata->disp_te_gpio);
			if (gpio_is_valid(ctrl_pdata->bl_en_gpio))
				gpio_free(ctrl_pdata->bl_en_gpio);		
			if (gpio_is_valid(ctrl_pdata->lcd_vddio_reg_en))
				gpio_free(ctrl_pdata->lcd_vddio_reg_en);
			if (gpio_is_valid(ctrl_pdata->lcd_vci_reg_en_p))
				gpio_free(ctrl_pdata->lcd_vci_reg_en_p);
			if (gpio_is_valid(ctrl_pdata->lcd_vci_reg_en_n))
				gpio_free(ctrl_pdata->lcd_vci_reg_en_n);
			return -ENODEV;
		}
	}

	//--------------------------bl-swire-gpio setting-----------------------------//
#if (BOARD_VER == PT10)
	ctrl_pdata->bl_swire_gpio = of_get_named_gpio(pdev->dev.of_node,
						     "qcom,bl-swire-gpio", 0);
	if (!gpio_is_valid(ctrl_pdata->bl_swire_gpio)) {
		pr_err("%s:%d, bl_swire gpio not specified\n",
						__func__, __LINE__);
	} else {
		rc = gpio_request(ctrl_pdata->bl_swire_gpio, "bl_swire");
		if (rc) {
			pr_err("request bl_swire_gpio failed, rc=%d\n",
			       rc);
			gpio_free(ctrl_pdata->bl_swire_gpio);	
			if (gpio_is_valid(ctrl_pdata->disp_te_gpio))
				gpio_free(ctrl_pdata->disp_te_gpio);
			if (gpio_is_valid(ctrl_pdata->bl_en_gpio))
				gpio_free(ctrl_pdata->bl_en_gpio);		
			if (gpio_is_valid(ctrl_pdata->lcd_vddio_reg_en))
				gpio_free(ctrl_pdata->lcd_vddio_reg_en);
			if (gpio_is_valid(ctrl_pdata->lcd_vci_reg_en_p))
				gpio_free(ctrl_pdata->lcd_vci_reg_en_p);
			if (gpio_is_valid(ctrl_pdata->lcd_vci_reg_en_n))
				gpio_free(ctrl_pdata->lcd_vci_reg_en_n);
			if (gpio_is_valid(ctrl_pdata->rst_gpio))
				gpio_free(ctrl_pdata->rst_gpio);
			return -ENODEV;
		}
		rc = gpio_direction_output(ctrl_pdata->bl_swire_gpio, 0);
		if (rc) {
			pr_err("set_direction for bl_swire_gpio gpio failed, rc=%d\n",
			       rc);
			gpio_free(ctrl_pdata->bl_swire_gpio);	
			if (gpio_is_valid(ctrl_pdata->disp_te_gpio))
				gpio_free(ctrl_pdata->disp_te_gpio);
			if (gpio_is_valid(ctrl_pdata->bl_en_gpio))
				gpio_free(ctrl_pdata->bl_en_gpio);		
			if (gpio_is_valid(ctrl_pdata->lcd_vddio_reg_en))
				gpio_free(ctrl_pdata->lcd_vddio_reg_en);
			if (gpio_is_valid(ctrl_pdata->lcd_vci_reg_en_p))
				gpio_free(ctrl_pdata->lcd_vci_reg_en_p);
			if (gpio_is_valid(ctrl_pdata->lcd_vci_reg_en_n))
				gpio_free(ctrl_pdata->lcd_vci_reg_en_n);
			if (gpio_is_valid(ctrl_pdata->rst_gpio))
				gpio_free(ctrl_pdata->rst_gpio);
			return -ENODEV;
		}			
	}		
#endif

	return 0;
}


ssize_t mipi_sharp_store_vblind_setting(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{

	return count;
}

ssize_t mipi_sharp_show_vblind_setting(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	int value =0;

	return snprintf(buf, PAGE_SIZE, "%d\n", value);
}

ssize_t mipi_sharp_store_vblind_strength(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{

	return count;
}

ssize_t mipi_sharp_show_vblind_strength(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	int value = 0;

	return snprintf(buf, PAGE_SIZE, "%d\n", value);
}
ssize_t mipi_sharp_store_lut_setting(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{


	return count;
}

ssize_t mipi_sharp_show_lut_setting(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	int value=0;


	return snprintf(buf, PAGE_SIZE, "%d\n", value);
}

ssize_t mipi_sharp_store_lut_type(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{


	return count;
}

ssize_t mipi_sharp_show_lut_type(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	int value =0;


	return snprintf(buf, PAGE_SIZE, "%d\n", value);
}

static struct device_attribute mipi_sharp_lcd_attributes[] = {
	__ATTR(vblind_setting, S_IRUGO|S_IWUSR,
			mipi_sharp_show_vblind_setting,  mipi_sharp_store_vblind_setting),
	__ATTR(vblind_strength, S_IRUGO|S_IWUSR,
			mipi_sharp_show_vblind_strength, mipi_sharp_store_vblind_strength),
	__ATTR(lut_setting, S_IRUGO|S_IWUSR,
			mipi_sharp_show_lut_setting, mipi_sharp_store_lut_setting),
	__ATTR(lut_type, S_IRUGO|S_IWUSR,
			mipi_sharp_show_lut_type, mipi_sharp_store_lut_type),
};

int sharp_panel_sysfs_register(struct device *dev)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(mipi_sharp_lcd_attributes); i++)
		if (device_create_file(dev, &mipi_sharp_lcd_attributes[i]))
			goto error;
	return 0;

error:
	for (; i >= 0 ; i--)
		device_remove_file(dev, &mipi_sharp_lcd_attributes[i]);
	pr_err("%s: Unable to create interface\n", __func__);

	return -ENODEV;

}

/************************************************************************************/
/* 									backlight control 									*/
/************************************************************************************/

static int bckl_lm3530_write(struct bckl_tx_ctrl *bckl_ctrl,uint8_t reg_offset, uint8_t value)
{
	struct i2c_msg msgs[1];
	uint8_t data[2];
	int status = -EACCES;
	uint8_t slave_addr=0x38;
	
	pr_debug("%s: writing from slave_addr=[%x] and offset=[%x]\n",
		 __func__, slave_addr, reg_offset);

	data[0] = reg_offset;
	data[1] = value;

	msgs[0].addr = slave_addr ;
	msgs[0].flags = 0;
	msgs[0].len = 2;
	msgs[0].buf = data;

	status = i2c_transfer(bckl_ctrl->i2c_handle->adapter, msgs, 1);
	if (status < 1) {
		pr_err("I2C WRITE FAILED=[%d]\n", status);
		return -EACCES;
	}
	pr_debug("%s: I2C write status=%x\n", __func__, status);
	return status;
}

static int bckl_lm3530_read(struct bckl_tx_ctrl *bckl_ctrl,uint8_t reg_offset,uint8_t* read_buf)
{
	int ret;
	uint8_t slave_addr=0x38;
	struct i2c_msg msgs[2];
	
	pr_debug("%s: reading from slave_addr=[%x] and offset=[%x]\n",
		 __func__, slave_addr, reg_offset);

	msgs[0].addr = slave_addr ;
	msgs[0].flags = 0;
	msgs[0].buf = &reg_offset;
	msgs[0].len = 1;

	msgs[1].addr = slave_addr ;
	msgs[1].flags = I2C_M_RD;
	msgs[1].buf = read_buf;
	msgs[1].len = 1;

	ret = i2c_transfer(bckl_ctrl->i2c_handle->adapter, msgs, 2);
	return ret;	
}
static int bckl_lk_init_check(struct bckl_tx_ctrl *bckl_ctrl)
{
	uint8_t read_buf[2];	
	
	bckl_lm3530_read(bckl_ctrl,0x10,read_buf);
	pr_info("0x10=%x\n",read_buf[0]);
	if(read_buf[0]==LM3530_LK_INIT_VALUE)	/// LK initialize code.	
		return 0;
	else
		return -1;
}

/// setLm3530Bright was called by mdss_dsi_panel_bklt_pwm()
int setLm3530Bright(int brightness,struct mdss_dsi_ctrl_pdata *ctrl)
{
	pr_info("lm3530: brightness=%d\n",brightness);
	
	if(bcklCtrl->mode == LM3530_BL_MODE_MANUAL)		
	{	
		bcklCtrl->mode = LM3530_BL_MODE_PWM;
		lm3530_init(bcklCtrl);
	}	
	if(ctrl->lcd_on_ckeck == true){
		sharp_display_on(ctrl,&ctrl->on_sec_cmds);
		ctrl->lcd_on_ckeck =false;
	}
#if 0	
	if(bcklCtrl->mode == LM3530_BL_MODE_MANUAL)	
	{
		/// adjust brightness val ..
		brightness=(brightness<<1)+0x2f;
		
		if(brightness>=LM3530_MAX_BRIGHTNESS)
			brightness=LM3530_MAX_BRIGHTNESS;
			
		brValue=(brightness) & LM3530_MAX_BRIGHTNESS;
				
	
		bckl_lm3530_write(bcklCtrl,LM3530_BRT_CTRL_REG,brValue);	
		pr_info("######################## brValue=%d\n",brValue);
	}	
#endif			
	return 0;		
}

static int backLightOn(struct mdss_dsi_ctrl_pdata *ctrl)
{
	if(bcklCtrl->cur_state == LM3530_STATUS_OFF)
	{
		gpio_set_value((ctrl->lcd_vci_reg_en_n), 1);
		mdelay(2);
		lm3530_init(bcklCtrl);
		bcklCtrl->cur_state=LM3530_STATUS_ON;
		pr_info("%s++\n",__func__);
	}	
	return 0;
}

static int backLightOff(struct mdss_dsi_ctrl_pdata *ctrl)
{
	int i;


	if(bcklCtrl->cur_state == LM3530_STATUS_ON)
	{
		for(i=0;i<3;i++)
			bckl_lm3530_write(bcklCtrl,LedWk_Off_LM3530[i].addr,LedWk_Off_LM3530[i].data);		
		bcklCtrl->cur_state=LM3530_STATUS_OFF;	
		mdelay(2);
		gpio_set_value((ctrl->lcd_vci_reg_en_n), 0);
		pr_info("%s++\n",__func__);
	}
	return 0;
}

static int lm3530_init(struct bckl_tx_ctrl *bckl_ctrl)
{
	//// Linux src
	int i;
	u8 gen_config;
    u8 brt_ramp;
    u8 brightness;    
	u8 reg_val[LM3530_REG_MAX];
	u8 max_current;
	struct lm3530_als_data als;
    
    max_current=LM3530_DEFAULT_FULL_CURRENT;    
	///////////////////////////////
	memset(&als, 0, sizeof(struct lm3530_als_data));

    gen_config = (0 << LM3530_RAMP_LAW_SHIFT) |
                        ((max_current & 7) << LM3530_MAX_CURR_SHIFT);
	/// PWM mode.

    switch (bckl_ctrl->mode) {
    case LM3530_BL_MODE_MANUAL:
            gen_config |= LM3530_ENABLE_I2C;
            break;
    case LM3530_BL_MODE_ALS:
            gen_config |= LM3530_ENABLE_I2C;
            ///lm3530_als_configure(pdata, &als);
            break;
    case LM3530_BL_MODE_PWM:
            ///gen_config |= LM3530_ENABLE_PWM | LM3530_ENABLE_PWM_SIMPLE|(1 << LM3530_PWM_POL_SHIFT);
            ///gen_config |= LM3530_ENABLE_PWM | LM3530_ENABLE_PWM_SIMPLE;            
			gen_config |= LM3530_ENABLE_PWM | LM3530_ENABLE_I2C;

            break;
    }	
	brt_ramp = 0;///(pdata->brt_ramp_fall << LM3530_BRT_RAMP_FALL_SHIFT) |
                 ///       (pdata->brt_ramp_rise << LM3530_BRT_RAMP_RISE_SHIFT);


	brightness = LM3530_DEFAULT_BRIGHTNESS;
	
	pr_info("lm3530_init : gen_config=%x\n",gen_config);
                        
	reg_val[0] = gen_config;        /* LM3530_GEN_CONFIG */
    reg_val[1] = als.config;        /* LM3530_ALS_CONFIG */
    reg_val[2] = brt_ramp;          /* LM3530_BRT_RAMP_RATE */
    reg_val[3] = als.imp_sel;       /* LM3530_ALS_IMP_SELECT */
    reg_val[4] = brightness;        /* LM3530_BRT_CTRL_REG */
    reg_val[5] = als.zones[0];      /* LM3530_ALS_ZB0_REG */
    reg_val[6] = als.zones[1];      /* LM3530_ALS_ZB1_REG */
    reg_val[7] = als.zones[2];      /* LM3530_ALS_ZB2_REG */
    reg_val[8] = als.zones[3];      /* LM3530_ALS_ZB3_REG */
    reg_val[9] = LM3530_DEF_ZT_0;   /* LM3530_ALS_Z0T_REG */
    reg_val[10] = LM3530_DEF_ZT_1;  /* LM3530_ALS_Z1T_REG */
    reg_val[11] = LM3530_DEF_ZT_2;  /* LM3530_ALS_Z2T_REG */
    reg_val[12] = LM3530_DEF_ZT_3;  /* LM3530_ALS_Z3T_REG */
    reg_val[13] = LM3530_DEF_ZT_4;  /* LM3530_ALS_Z4T_REG */
    
    for(i=0;i<LM3530_REG_MAX;i++)
    {    	
		bckl_lm3530_write(bckl_ctrl,lm3530_reg[i],reg_val[i]);
	}	
	return 0;
}


static int bckl_i2c_probe(struct i2c_client *client,
			 const struct i2c_device_id *id)
{
	int rc=0;
	struct bckl_tx_ctrl *bckl_ctrl;
	
	pr_info("bckl_i2c_probe>>");
	
	bckl_ctrl = devm_kzalloc(&client->dev, sizeof(*bckl_ctrl), GFP_KERNEL);
	if (!bckl_ctrl) {
		pr_err("%s: FAILED: cannot alloc hdmi tx ctrl\n", __func__);
		rc = -ENOMEM;
	}
	
	if (client->dev.of_node) {
		
		bckl_ctrl->i2c_handle = client;
		i2c_set_clientdata(client, bckl_ctrl);
		pr_info("%s: i2c client addr is [%x]\n", __func__, client->addr);
	}

	
	
	rc = bckl_lk_init_check(bckl_ctrl);
	if (rc)
	{							/// Lk initialize failed.
		lm3530_init(bckl_ctrl);	/// turn on default bright ness : 0x4F	
	}
	bckl_ctrl->mode=LM3530_BL_MODE_MANUAL;
	bckl_ctrl->cur_state=LM3530_STATUS_ON;
			
	bcklCtrl=bckl_ctrl;
	pr_info("bckl_i2c_probe<<");
	return rc;
}


static int bckl_i2c_remove(struct i2c_client *client)
{
	struct bckl_tx_ctrl *bckl_ctrl = i2c_get_clientdata(client);
	int i;
	
	if (!bckl_ctrl) {
		pr_warn("%s: i2c get client data failed\n", __func__);
		return -EINVAL;
	}
	for(i=0;i<3;i++)	
		bckl_lm3530_write(bckl_ctrl,LedWk_Off_LM3530[0].addr,LedWk_Off_LM3530[0].data);
	devm_kfree(&client->dev, bckl_ctrl);
	return 0;
}

static struct i2c_device_id bckl_lm3530_i2c_id[] = {
	{ BCKL_DRIVER_NAME, 0 },
	{ }
};


MODULE_DEVICE_TABLE(i2c, bckl_lm3530_i2c_id);

static struct of_device_id bckl_match_table[] = {
	{.compatible = COMPATIBLE_NAME,},
	{ },
};

static struct i2c_driver backlight_i2c_driver = {
	.driver = {
		.name = BCKL_DRIVER_NAME,
		.owner = THIS_MODULE,
		.of_match_table = bckl_match_table,
	},
	.probe = bckl_i2c_probe,
	.remove =  bckl_i2c_remove,
	.id_table = bckl_lm3530_i2c_id,
};

module_i2c_driver(backlight_i2c_driver);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("LCD BAKL LIGHT Driver");


