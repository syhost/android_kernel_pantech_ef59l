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
#include "mdss_dsi.h"
#include "mdss_fb.h"
#ifdef CONFIG_D_SKY_MIPI_CLK_HIGH
#include <linux/io.h>
#include <linux/of_device.h>
#endif

#define DT_CMD_HDR 6

DEFINE_LED_TRIGGER(bl_led_trigger);

extern struct msm_fb_data_type * mfdmsm_fb_get_mfd(void);

#if defined (FEATURE_PANTECH_PMIC_CHARGE_LIMIT_WITH_LCD)
extern void smb349_adjust_limit_LCD(int lcd_on);
#endif

static struct mdss_dsi_phy_ctrl phy_params;
#ifdef CONFIG_F_SKYDISP_SHARP_FHD_VIDEO_COMMON
static int lcd_on_skip_during_bootup = 0;
#endif


void mdss_dsi_panel_pwm_cfg(struct mdss_dsi_ctrl_pdata *ctrl)
{
	int ret;

	if (!gpio_is_valid(ctrl->pwm_pmic_gpio)) {
		pr_err("%s: pwm_pmic_gpio=%d Invalid\n", __func__,
				ctrl->pwm_pmic_gpio);
		ctrl->pwm_pmic_gpio = -1;
		return;
	}

	ret = gpio_request(ctrl->pwm_pmic_gpio, "disp_pwm");
	if (ret) {
		pr_err("%s: pwm_pmic_gpio=%d request failed\n", __func__,
				ctrl->pwm_pmic_gpio);
		ctrl->pwm_pmic_gpio = -1;
		return;
	}

	ctrl->pwm_bl = pwm_request(ctrl->pwm_lpg_chan, "lcd-bklt");
	if (ctrl->pwm_bl == NULL || IS_ERR(ctrl->pwm_bl)) {
		pr_err("%s: lpg_chan=%d pwm request failed", __func__,
				ctrl->pwm_lpg_chan);
		gpio_free(ctrl->pwm_pmic_gpio);
		ctrl->pwm_pmic_gpio = -1;
	}
}

void mdss_dsi_panel_bklt_pwm(struct mdss_dsi_ctrl_pdata *ctrl, int level)
{
	int ret;
	u32 duty;

	if (ctrl->pwm_bl == NULL) {
		pr_err("%s: no PWM\n", __func__);
		return;
	}

	duty = level * ctrl->pwm_period;
	duty /= ctrl->bklt_max;

	pr_debug("%s: bklt_ctrl=%d pwm_period=%d pwm_gpio=%d pwm_lpg_chan=%d\n",
			__func__, ctrl->bklt_ctrl, ctrl->pwm_period,
				ctrl->pwm_pmic_gpio, ctrl->pwm_lpg_chan);

	pr_debug("%s: ndx=%d level=%d duty=%d\n", __func__,
					ctrl->ndx, level, duty);

	ret = pwm_config(ctrl->pwm_bl, duty, ctrl->pwm_period);
	if (ret) {
		pr_err("%s: pwm_config() failed err=%d.\n", __func__, ret);
		return;
	}

	ret = pwm_enable(ctrl->pwm_bl);

	if (ret)
		pr_err("%s: pwm_enable() failed err=%d\n", __func__, ret);
}

static char dcs_cmd[2] = {0x54, 0x00}; /* DTYPE_DCS_READ */
static struct dsi_cmd_desc dcs_read_cmd = {
	{DTYPE_DCS_READ, 1, 0, 1, 5, sizeof(dcs_cmd)},
	dcs_cmd
};
u32 mdss_dsi_dcs_read(struct mdss_dsi_ctrl_pdata *ctrl,
			char cmd0, char cmd1)
{
	struct dcs_cmd_req cmdreq;

	dcs_cmd[0] = cmd0;
	dcs_cmd[1] = cmd1;
	memset(&cmdreq, 0, sizeof(cmdreq));
	cmdreq.cmds = &dcs_read_cmd;
	cmdreq.cmds_cnt = 1;
	cmdreq.flags = CMD_REQ_RX | CMD_REQ_COMMIT;
	cmdreq.rlen = 1;
	cmdreq.cb = NULL; /* call back */
	mdss_dsi_cmdlist_put(ctrl, &cmdreq);
	/*
	 * blocked here, until call back called
	 */

	return 0;
}

static void mdss_dsi_panel_cmds_send(struct mdss_dsi_ctrl_pdata *ctrl,
			struct dsi_panel_cmds *pcmds)
{
	struct dcs_cmd_req cmdreq;

	memset(&cmdreq, 0, sizeof(cmdreq));
	cmdreq.cmds = pcmds->cmds;
	cmdreq.cmds_cnt = pcmds->cmd_cnt;
	cmdreq.flags = CMD_REQ_COMMIT;
	cmdreq.rlen = 0;
	cmdreq.cb = NULL;
	mdss_dsi_cmdlist_put(ctrl, &cmdreq);
}
#ifdef CONFIG_F_SKYDISP_SHARP_FHD_VIDEO_COMMON
static char led_pwm1[3] = {0x51, 0x00, 0x00};	/* DTYPE_DCS_LWRITE */
static struct dsi_cmd_desc backlight_cmd = {
	{DTYPE_DCS_LWRITE, 1, 0, 0, 1, sizeof(led_pwm1)},
	led_pwm1
};

void mdss_dsi_panel_bklt_dcs(struct mdss_dsi_ctrl_pdata *ctrl, int level)
{
	struct dcs_cmd_req cmdreq;

	pr_err("%s: level=%d\n", __func__, level);
#if 0//defined (CONFIG_MACH_MSM8974_EF56S) ||defined(CONFIG_MACH_MSM8974_EF57K) || defined (CONFIG_MACH_MSM8974_EF58L)
//PWM 22khz
	led_pwm1[1] = (unsigned char)(level >>8) & 0x0F; 
       led_pwm1[2] = (unsigned char)level & 0xFF;
#else
//PWM 10khz
       led_pwm1[2] = (unsigned char)level & 0xFF;
#endif

	memset(&cmdreq, 0, sizeof(cmdreq));
	cmdreq.cmds = &backlight_cmd;
	cmdreq.cmds_cnt = 1;
	cmdreq.flags = CMD_REQ_COMMIT | CMD_CLK_CTRL;
	cmdreq.rlen = 0;
	cmdreq.cb = NULL;

       msleep(1);
	mdss_dsi_cmdlist_put(ctrl, &cmdreq);
}
#else   //Qualcomm release
static char led_pwm1[2] = {0x51, 0x0};	/* DTYPE_DCS_WRITE1 */
static struct dsi_cmd_desc backlight_cmd = {
	{DTYPE_DCS_WRITE1, 1, 0, 0, 1, sizeof(led_pwm1)},
	led_pwm1
};

void mdss_dsi_panel_bklt_dcs(struct mdss_dsi_ctrl_pdata *ctrl, int level)
{
	struct dcs_cmd_req cmdreq;

	pr_debug("%s: level=%d\n", __func__, level);

	led_pwm1[1] = (unsigned char)level;

	memset(&cmdreq, 0, sizeof(cmdreq));
	cmdreq.cmds = &backlight_cmd;
	cmdreq.cmds_cnt = 1;
	cmdreq.flags = CMD_REQ_COMMIT | CMD_CLK_CTRL;
	cmdreq.rlen = 0;
	cmdreq.cb = NULL;

	mdss_dsi_cmdlist_put(ctrl, &cmdreq);
}
#endif
void mdss_dsi_panel_reset(struct mdss_panel_data *pdata, int enable)
{
#if defined (CONFIG_MACH_MSM8974_SVLTE) || defined (CONFIG_MACH_MSM8974_CSFB)
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;

	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return;
	}

	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);
	if (!gpio_is_valid(ctrl_pdata->disp_en_gpio)) {
		pr_debug("%s:%d, reset line not configured\n",
			   __func__, __LINE__);
		return;
	}

	if (!gpio_is_valid(ctrl_pdata->rst_gpio)) {
		pr_debug("%s:%d, reset line not configured\n",
			   __func__, __LINE__);
		return;
	}

	pr_debug("%s: enable = %d\n", __func__, enable);

	if (enable) {
		gpio_set_value((ctrl_pdata->rst_gpio), 1);
		msleep(20);
		wmb();
		gpio_set_value((ctrl_pdata->rst_gpio), 0);
		udelay(200);
		wmb();
		gpio_set_value((ctrl_pdata->rst_gpio), 1);
		msleep(20);
		wmb();
		gpio_set_value((ctrl_pdata->disp_en_gpio), 1);
		wmb();
	} else {
		gpio_set_value((ctrl_pdata->rst_gpio), 0);
		gpio_set_value((ctrl_pdata->disp_en_gpio), 0);
	}

#elif defined (CONFIG_MACH_MSM8974_EF56S) ||defined(CONFIG_MACH_MSM8974_EF57K) || defined (CONFIG_MACH_MSM8974_EF58L)
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
		pr_debug("%s:%d, reset line not configured\n",
			   __func__, __LINE__);
		return;
	}

	if (!gpio_is_valid(ctrl_pdata->lcd_vcip_reg_en)) {
		pr_debug("%s:%d, lcd vcip line not configured\n",
			   __func__, __LINE__);
		return;
	}
	
	if (!gpio_is_valid(ctrl_pdata->lcd_vcin_reg_en)) {
		pr_debug("%s:%d, lcd vcin line not configured\n",
			   __func__, __LINE__);
		return;
	}
	
	if (!gpio_is_valid(ctrl_pdata->bl_swire_gpio)) {
		pr_debug("%s:%d, bl swire line not configured\n",
			   __func__, __LINE__);
		return;
	}	

	pr_debug("%s: enable = %d\n", __func__, enable);

	if (enable) {
		gpio_set_value((ctrl_pdata->lcd_vcip_reg_en), 1);
		msleep(2);
		gpio_set_value((ctrl_pdata->lcd_vcin_reg_en), 1);		
		msleep(2);
		wmb();
		
		gpio_set_value((ctrl_pdata->rst_gpio), 0);
		msleep(10);
		wmb();
		gpio_set_value((ctrl_pdata->rst_gpio), 1);
		msleep(10);
		wmb();
		//gpio_set_value((ctrl_pdata->bl_en_gpio), 1);
#if (BOARD_VER >= WS20)	
		gpio_set_value((ctrl_pdata->bl_swire_gpio), 0);
#endif
	} else {
		gpio_set_value((ctrl_pdata->rst_gpio), 0);
		msleep(5);
		gpio_set_value((ctrl_pdata->lcd_vcin_reg_en), 0);		
		msleep(3);
		gpio_set_value((ctrl_pdata->lcd_vcip_reg_en), 0);
		//gpio_set_value((ctrl_pdata->bl_en_gpio), 0);
		msleep(100);
		wmb();
	}
#elif defined (CONFIG_MACH_MSM8974_EF59S) || defined (CONFIG_MACH_MSM8974_EF59K) || defined (CONFIG_MACH_MSM8974_EF59L) || (defined (CONFIG_MACH_MSM8974_EF60S) && (BOARD_VER == PT10)) || (defined (CONFIG_MACH_MSM8974_EF61K) && (BOARD_VER == PT10)) || (defined (CONFIG_MACH_MSM8974_EF62L) && (BOARD_VER == PT10))
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
		pr_debug("%s:%d, reset line not configured\n",
			   __func__, __LINE__);
		return;
	}

	if (!gpio_is_valid(ctrl_pdata->lcd_vcip_reg_en)) {
		pr_debug("%s:%d, lcd vcip line not configured\n",
			   __func__, __LINE__);
		return;
	}
	
	if (!gpio_is_valid(ctrl_pdata->bl_swire_gpio)) {
		pr_debug("%s:%d, bl swire line not configured\n",
			   __func__, __LINE__);
		return;
	}	

	pr_debug("%s: enable = %d\n", __func__, enable);

	if (enable) {
		gpio_set_value((ctrl_pdata->lcd_vddio_reg_en),1);
		msleep(1);
		gpio_set_value((ctrl_pdata->bl_swire_gpio), 1);
		msleep(1);
		gpio_set_value((ctrl_pdata->lcd_vcip_reg_en), 1);
		
		msleep(2);
		wmb();
		
		gpio_set_value((ctrl_pdata->rst_gpio), 0);
		msleep(10);
		wmb();
		gpio_set_value((ctrl_pdata->rst_gpio), 1);
		msleep(10);
		wmb();
		//gpio_set_value((ctrl_pdata->bl_en_gpio), 1);
	

	} else {
		//gpio_set_value((ctrl_pdata->bl_en_gpio), 0);
		//msleep(2);
		gpio_set_value((ctrl_pdata->rst_gpio), 0);
		msleep(5);
		gpio_set_value((ctrl_pdata->lcd_vcip_reg_en), 0);
		gpio_set_value((ctrl_pdata->bl_swire_gpio), 0);
		msleep(100);
		wmb();
	}
#elif (defined (CONFIG_MACH_MSM8974_EF60S) && (BOARD_VER >= WS10)) || (defined (CONFIG_MACH_MSM8974_EF61K) && (BOARD_VER >= WS10)) || (defined (CONFIG_MACH_MSM8974_EF62L) && (BOARD_VER >= WS10))
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
		pr_debug("%s:%d, reset line not configured\n",
			   __func__, __LINE__);
		return;
	}

	if (!gpio_is_valid(ctrl_pdata->lcd_vcip_reg_en)) {
		pr_debug("%s:%d, lcd vcip line not configured\n",
			   __func__, __LINE__);
		return;
	}
#if (BOARD_VER < WS20)	
	if (!gpio_is_valid(ctrl_pdata->lcd_vcin_reg_en)) {
		pr_debug("%s:%d, lcd vcin line not configured\n",
			   __func__, __LINE__);
		return;
	}
#endif	
	pr_debug("%s: enable = %d\n", __func__, enable);

	if (enable) {
		gpio_set_value((ctrl_pdata->lcd_vcip_reg_en), 1);
		msleep(2);
#if (BOARD_VER < WS20)		
		gpio_set_value((ctrl_pdata->lcd_vcin_reg_en), 1);		
		msleep(2);
		wmb();
#endif		
		gpio_set_value((ctrl_pdata->rst_gpio), 0);
		msleep(10);
		wmb();
		gpio_set_value((ctrl_pdata->rst_gpio), 1);
		msleep(10);
		wmb();
		//gpio_set_value((ctrl_pdata->bl_en_gpio), 1);
	} else {
		gpio_set_value((ctrl_pdata->rst_gpio), 0);
		msleep(5);
#if (BOARD_VER < WS20)		
		gpio_set_value((ctrl_pdata->lcd_vcin_reg_en), 0);		
		msleep(3);
#endif
		gpio_set_value((ctrl_pdata->lcd_vcip_reg_en), 0);
		//gpio_set_value((ctrl_pdata->bl_en_gpio), 0);
		msleep(100);
		wmb();
	}
#else  //Qualcomm release
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;

	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return;
	}

	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);

	if (!gpio_is_valid(ctrl_pdata->disp_en_gpio)) {
		pr_debug("%s:%d, reset line not configured\n",
			   __func__, __LINE__);
	}

	if (!gpio_is_valid(ctrl_pdata->rst_gpio)) {
		pr_debug("%s:%d, reset line not configured\n",
			   __func__, __LINE__);
		return;
	}
	
	pr_debug("%s: enable = %d\n", __func__, enable);

	if (enable) {
		gpio_set_value((ctrl_pdata->rst_gpio), 1);
		msleep(20);
		gpio_set_value((ctrl_pdata->rst_gpio), 0);
		udelay(200);
		gpio_set_value((ctrl_pdata->rst_gpio), 1);
		msleep(20);
		if (gpio_is_valid(ctrl_pdata->disp_en_gpio))
			gpio_set_value((ctrl_pdata->disp_en_gpio), 1);
		if (ctrl_pdata->ctrl_state & CTRL_STATE_PANEL_INIT) {
			pr_debug("%s: Panel Not properly turned OFF\n",
						__func__);
			ctrl_pdata->ctrl_state &= ~CTRL_STATE_PANEL_INIT;
			pr_debug("%s: Reset panel done\n", __func__);
		}
	} else {
		gpio_set_value((ctrl_pdata->rst_gpio), 0);
		if (gpio_is_valid(ctrl_pdata->disp_en_gpio))
			gpio_set_value((ctrl_pdata->disp_en_gpio), 0);
	}

#endif	
}
#ifdef CONFIG_F_SKYDISP_SHARP_FHD_VIDEO_COMMON	
bool first_enable = false;
#endif
static void mdss_dsi_panel_bl_ctrl(struct mdss_panel_data *pdata,
							u32 bl_level)
{
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	struct msm_fb_data_type * mfd = mfdmsm_fb_get_mfd();
	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return;
	}
	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);
	
	if(!mfd->panel_power_on)
	{
		printk("[%s] panel is off state (%d).....\n",__func__,mfd->panel_power_on);
		return;
	}
#ifdef CONFIG_F_SKYDISP_SHARP_FHD_VIDEO_COMMON	
	if(bl_level == 0)
	{
		gpio_set_value((ctrl_pdata->bl_en_gpio), 0);	
		first_enable = false;
		printk("%s:bl_en_gpio off\n",__func__);
	}
	else
	{     
	       if(first_enable ==false)
	       {
			gpio_set_value((ctrl_pdata->bl_en_gpio), 1);		
			first_enable = true;
			printk("%s:bl_en_gpio on\n",__func__);
	       }
	}
#endif
	switch (ctrl_pdata->bklt_ctrl) {
		case BL_WLED:
			led_trigger_event(bl_led_trigger, bl_level);
			break;
		case BL_PWM:
			mdss_dsi_panel_bklt_pwm(ctrl_pdata, bl_level);
			break;
		case BL_DCS_CMD:
/* p13447 130904 Fixed code for EF59 backlight issue */
#ifdef CONFIG_F_SKYDISP_SHARP_FHD_VIDEO_COMMON	
		mdss_set_tx_power_mode(0 , pdata );
		msleep(2);
#endif
		mdss_dsi_panel_bklt_dcs(ctrl_pdata, bl_level);
#ifdef CONFIG_F_SKYDISP_SHARP_FHD_VIDEO_COMMON	
		msleep(1);
		mdss_set_tx_power_mode(1 , pdata);
#endif
			break;
		default:
			pr_err("%s: Unknown bl_ctrl configuration\n",
				__func__);
			break;
		}
}
#ifdef CONFIG_F_SKYDISP_CABC_CONTROL
void cabc_control(struct mdss_panel_data *pdata, int state)
{
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,panel_data);
#if defined (CONFIG_MACH_MSM8974_EF56S) 	||defined(CONFIG_MACH_MSM8974_EF60S) || defined(CONFIG_MACH_MSM8974_EF61K) || defined(CONFIG_MACH_MSM8974_EF62L)
	if(state == 1){
		*(ctrl_pdata->cabc_cmds.cmds->payload + 1) = 0x01;	
		ctrl_pdata->on_cmds.buf[207] = 0x01;
	}else if(state == 0){
		*(ctrl_pdata->cabc_cmds.cmds->payload + 1) = 0x00;
		ctrl_pdata->on_cmds.buf[207] = 0x00;
	}else { //for err
		*(ctrl_pdata->cabc_cmds.cmds->payload+ 1) = 0x01;
		ctrl_pdata->on_cmds.buf[207] = 0x01;
	}	
#else
	if(state == 1){
		*(ctrl_pdata->cabc_cmds.cmds->payload + 1) = 0x03;	
		ctrl_pdata->on_cmds.buf[178] = 0x03;
	}else if(state == 0){
		*(ctrl_pdata->cabc_cmds.cmds->payload + 1) = 0x00;
		ctrl_pdata->on_cmds.buf[178] = 0x00;
	}else { //for err
		*(ctrl_pdata->cabc_cmds.cmds->payload+ 1) = 0x03;
		ctrl_pdata->on_cmds.buf[178] = 0x03;
	}	
#endif
		mdss_dsi_panel_cmds_send(ctrl_pdata, &ctrl_pdata->cabc_cmds);
}
#endif
static int mdss_dsi_panel_on(struct mdss_panel_data *pdata)
{
	struct mipi_panel_info *mipi;
#ifdef CONFIG_D_SKY_MIPI_CLK_HIGH
	u32 tmp;
#endif
	struct mdss_dsi_ctrl_pdata *ctrl = NULL;

	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	ctrl = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);
	mipi  = &pdata->panel_info.mipi;

	pr_debug("%s: ctrl=%p ndx=%d\n", __func__, ctrl, ctrl->ndx);

#ifdef CONFIG_D_SKY_MIPI_CLK_HIGH
	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);
	if (!ctrl_pdata) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}
	tmp = MIPI_INP((ctrl_pdata->ctrl_base) + 0xac);
	tmp |= (1<<28);
	MIPI_OUTP((ctrl_pdata->ctrl_base) + 0xac, tmp);
	printk("[MIPI: shinbrad High speed Clk Set(On Sequence) .................................................]\n");
#endif
	//msleep(100);


#ifdef CONFIG_F_SKYDISP_CMDS_CONTROL	
	if(ctrl->lcd_cmds_check == false){
#endif

	if (ctrl->on_cmds.cmd_cnt)
	{
#if defined(CONFIG_MACH_MSM8974_EF56S) || defined(CONFIG_MACH_MSM8974_EF60S) || defined(CONFIG_MACH_MSM8974_EF61K) || defined(CONFIG_MACH_MSM8974_EF62L)
	       if(lcd_on_skip_during_bootup)
	       {
			ctrl->on_cmds.buf[189] = 0x00;
	       }
#else //EF59S/K/L
	       if(lcd_on_skip_during_bootup)
	       {
			ctrl->on_cmds.buf[160] = 0x00;
	       }
#endif
		mdss_dsi_panel_cmds_send(ctrl, &ctrl->on_cmds);
#ifdef CONFIG_F_SKYDISP_SHARP_FHD_VIDEO_COMMON
              if(!lcd_on_skip_during_bootup)
		lcd_on_skip_during_bootup=1;
#endif
	}
	
#ifdef CONFIG_F_SKYDISP_CMDS_CONTROL		
	}else if(ctrl->lcd_cmds_check == true){
		if (ctrl->on_cmds_user.cmd_cnt)
			mdss_dsi_panel_cmds_send(ctrl, &ctrl->on_cmds_user);
		pr_info("user LCD on cmds---------------->\n");
	}
#endif	

#ifdef CONFIG_F_SKYDISP_CABC_CONTROL
		//mdss_dsi_panel_cmds_send(ctrl, &ctrl->cabc_cmds);
#endif	


#if defined (FEATURE_PANTECH_PMIC_CHARGE_LIMIT_WITH_LCD)
        smb349_adjust_limit_LCD(1); // Fixed code for charging current.
#endif

	pr_err("%s:-\n", __func__);
	return 0;
}

static int mdss_dsi_panel_off(struct mdss_panel_data *pdata)
{
	struct mipi_panel_info *mipi;
	struct mdss_dsi_ctrl_pdata *ctrl = NULL;

#ifdef CONFIG_D_SKY_MIPI_CLK_HIGH
	u32 tmp;

	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);
	if (!ctrl_pdata) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	tmp = MIPI_INP((ctrl_pdata->ctrl_base) + 0xac);
	tmp &= ~(1<<28);
	MIPI_OUTP((ctrl_pdata->ctrl_base) + 0xac, tmp);
	printk("[MIPI: shinbrad High speed Clk Set(Off Sequence) .................................................]\n");
#endif


	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	ctrl = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);

	pr_debug("%s: ctrl=%p ndx=%d\n", __func__, ctrl, ctrl->ndx);

	mipi  = &pdata->panel_info.mipi;




	if (ctrl->off_cmds.cmd_cnt)
		mdss_dsi_panel_cmds_send(ctrl, &ctrl->off_cmds);

#if defined (FEATURE_PANTECH_PMIC_CHARGE_LIMIT_WITH_LCD)
        smb349_adjust_limit_LCD(0); // Fixed code for charging current.
#endif

	pr_err("%s:-\n", __func__);
	return 0;
}

static int mdss_dsi_parse_dcs_cmds(struct device_node *np,
		struct dsi_panel_cmds *pcmds, char *cmd_key, char *link_key)
{
	const char *data;
	int blen = 0, len;
	char *buf, *bp;
	struct dsi_ctrl_hdr *dchdr;
	int i, cnt;

	data = of_get_property(np, cmd_key, &blen);
	if (!data) {
		pr_err("%s: failed, key=%s\n", __func__, cmd_key);
		return -ENOMEM;
	}

	buf = kzalloc(sizeof(char) * blen, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	memcpy(buf, data, blen);

	/* scan dcs commands */
	bp = buf;
	len = blen;
	cnt = 0;
	while (len > sizeof(*dchdr)) {
		dchdr = (struct dsi_ctrl_hdr *)bp;

		dchdr->dlen = ntohs(dchdr->dlen);

		if (dchdr->dlen > len) {
			pr_err("%s: dtsi cmd=%x error, len=%d",
				__func__, dchdr->dtype, dchdr->dlen);
			return -ENOMEM;
		}
		bp += sizeof(*dchdr);
		len -= sizeof(*dchdr);
		bp += dchdr->dlen;
		len -= dchdr->dlen;
		cnt++;
	}

	if (len != 0) {
		pr_err("%s: dcs_cmd=%x len=%d error!",
				__func__, buf[0], blen);
		kfree(buf);
		return -ENOMEM;
	}

	pcmds->cmds = kzalloc(cnt * sizeof(struct dsi_cmd_desc),
						GFP_KERNEL);
	if (!pcmds->cmds)
		return -ENOMEM;

	pcmds->cmd_cnt = cnt;
	pcmds->buf = buf;
	pcmds->blen = blen;

	bp = buf;
	len = blen;
	for (i = 0; i < cnt; i++) {
		dchdr = (struct dsi_ctrl_hdr *)bp;
		len -= sizeof(*dchdr);
		bp += sizeof(*dchdr);
		pcmds->cmds[i].dchdr = *dchdr;
		pcmds->cmds[i].payload = bp;
		bp += dchdr->dlen;
		len -= dchdr->dlen;
	}

	pcmds->link_state = DSI_LP_MODE; /* default */

	data = of_get_property(np, link_key, NULL);
	if (!strncmp(data, "DSI_HS_MODE", 11))
		pcmds->link_state = DSI_HS_MODE;

	pr_debug("%s: dcs_cmd=%x len=%d, cmd_cnt=%d link_state=%d\n", __func__,
		pcmds->buf[0], pcmds->blen, pcmds->cmd_cnt, pcmds->link_state);

	return 0;
}
static int mdss_panel_parse_dt(struct platform_device *pdev,
			       struct mdss_panel_common_pdata *panel_data)
{
	struct device_node *np = pdev->dev.of_node;
	u32 res[6], tmp;
	u32 fbc_res[7];
	int rc, i, len;
	const char *data;
	static const char *bl_ctrl_type, *pdest;
	bool fbc_enabled = false;

	rc = of_property_read_u32_array(np, "qcom,mdss-pan-res", res, 2);
	if (rc) {
		pr_err("%s:%d, panel resolution not specified\n",
						__func__, __LINE__);
		return -EINVAL;
	}
	panel_data->panel_info.xres = (!rc ? res[0] : 640);
	panel_data->panel_info.yres = (!rc ? res[1] : 480);

	rc = of_property_read_u32_array(np, "qcom,mdss-pan-active-res", res, 2);
	if (rc == 0) {
		panel_data->panel_info.lcdc.xres_pad =
			panel_data->panel_info.xres - res[0];
		panel_data->panel_info.lcdc.yres_pad =
			panel_data->panel_info.yres - res[1];
	}

	rc = of_property_read_u32(np, "qcom,mdss-pan-bpp", &tmp);
	if (rc) {
		pr_err("%s:%d, panel bpp not specified\n",
						__func__, __LINE__);
		return -EINVAL;
	}
	panel_data->panel_info.bpp = (!rc ? tmp : 24);

	pdest = of_get_property(pdev->dev.of_node,
				"qcom,mdss-pan-dest", NULL);
	if (strlen(pdest) != 9) {
		pr_err("%s: Unknown pdest specified\n", __func__);
		return -EINVAL;
	}
	if (!strncmp(pdest, "display_1", 9))
		panel_data->panel_info.pdest = DISPLAY_1;
	else if (!strncmp(pdest, "display_2", 9))
		panel_data->panel_info.pdest = DISPLAY_2;
	else {
		pr_debug("%s: pdest not specified. Set Default\n",
							__func__);
		panel_data->panel_info.pdest = DISPLAY_1;
	}

	rc = of_property_read_u32_array(np,
		"qcom,mdss-pan-porch-values", res, 6);
	panel_data->panel_info.lcdc.h_back_porch = (!rc ? res[0] : 6);
	panel_data->panel_info.lcdc.h_pulse_width = (!rc ? res[1] : 2);
	panel_data->panel_info.lcdc.h_front_porch = (!rc ? res[2] : 6);
	panel_data->panel_info.lcdc.v_back_porch = (!rc ? res[3] : 6);
	panel_data->panel_info.lcdc.v_pulse_width = (!rc ? res[4] : 2);
	panel_data->panel_info.lcdc.v_front_porch = (!rc ? res[5] : 6);

	rc = of_property_read_u32(np,
		"qcom,mdss-pan-underflow-clr", &tmp);
	panel_data->panel_info.lcdc.underflow_clr = (!rc ? tmp : 0xff);

	bl_ctrl_type = of_get_property(pdev->dev.of_node,
				  "qcom,mdss-pan-bl-ctrl", NULL);
	if ((bl_ctrl_type) && (!strncmp(bl_ctrl_type, "bl_ctrl_wled", 12))) {
		led_trigger_register_simple("bkl-trigger", &bl_led_trigger);
		pr_debug("%s: SUCCESS-> WLED TRIGGER register\n", __func__);

		panel_data->panel_info.bklt_ctrl = BL_WLED;
	} else if (!strncmp(bl_ctrl_type, "bl_ctrl_pwm", 11)) {
		panel_data->panel_info.bklt_ctrl = BL_PWM;

		rc = of_property_read_u32(np, "qcom,pwm-period", &tmp);
		if (rc) {
			pr_err("%s:%d, Error, panel pwm_period\n",
						__func__, __LINE__);
			return -EINVAL;
	}
		panel_data->panel_info.pwm_period = tmp;

		rc = of_property_read_u32(np, "qcom,pwm-lpg-channel", &tmp);
		if (rc) {
			pr_err("%s:%d, Error, dsi lpg channel\n",
						__func__, __LINE__);
			return -EINVAL;
		}
		panel_data->panel_info.pwm_lpg_chan = tmp;

		tmp = of_get_named_gpio(np, "qcom,pwm-pmic-gpio", 0);
		panel_data->panel_info.pwm_pmic_gpio =  tmp;
	} else if (!strncmp(bl_ctrl_type, "bl_ctrl_dcs", 11)) {
		panel_data->panel_info.bklt_ctrl = BL_DCS_CMD;
	} else {
		pr_debug("%s: Unknown backlight control\n", __func__);
		panel_data->panel_info.bklt_ctrl = UNKNOWN_CTRL;
	}

	rc = of_property_read_u32_array(np,
		"qcom,mdss-pan-bl-levels", res, 2);
	panel_data->panel_info.bl_min = (!rc ? res[0] : 0);
	panel_data->panel_info.bl_max = (!rc ? res[1] : 255);

	rc = of_property_read_u32(np, "qcom,mdss-pan-dsi-mode", &tmp);
	panel_data->panel_info.mipi.mode = (!rc ? tmp : DSI_VIDEO_MODE);

	rc = of_property_read_u32(np, "qcom,mdss-vsync-enable", &tmp);
	panel_data->panel_info.mipi.vsync_enable = (!rc ? tmp : 0);

	rc = of_property_read_u32(np, "qcom,mdss-hw-vsync-mode", &tmp);
	panel_data->panel_info.mipi.hw_vsync_mode = (!rc ? tmp : 0);


	rc = of_property_read_u32(np,
		"qcom,mdss-pan-dsi-h-pulse-mode", &tmp);
	panel_data->panel_info.mipi.pulse_mode_hsa_he = (!rc ? tmp : false);

	rc = of_property_read_u32_array(np,
		"qcom,mdss-pan-dsi-h-power-stop", res, 3);
	panel_data->panel_info.mipi.hbp_power_stop = (!rc ? res[0] : false);
	panel_data->panel_info.mipi.hsa_power_stop = (!rc ? res[1] : false);
	panel_data->panel_info.mipi.hfp_power_stop = (!rc ? res[2] : false);

	rc = of_property_read_u32_array(np,
		"qcom,mdss-pan-dsi-bllp-power-stop", res, 2);
	panel_data->panel_info.mipi.bllp_power_stop =
					(!rc ? res[0] : false);
	panel_data->panel_info.mipi.eof_bllp_power_stop =
					(!rc ? res[1] : false);

	rc = of_property_read_u32(np,
		"qcom,mdss-pan-dsi-traffic-mode", &tmp);
	panel_data->panel_info.mipi.traffic_mode =
			(!rc ? tmp : DSI_NON_BURST_SYNCH_PULSE);

	rc = of_property_read_u32(np,
		"qcom,mdss-pan-insert-dcs-cmd", &tmp);
	panel_data->panel_info.mipi.insert_dcs_cmd =
			(!rc ? tmp : 1);

	rc = of_property_read_u32(np,
		"qcom,mdss-pan-wr-mem-continue", &tmp);
	panel_data->panel_info.mipi.wr_mem_continue =
			(!rc ? tmp : 0x3c);

	rc = of_property_read_u32(np,
		"qcom,mdss-pan-wr-mem-start", &tmp);
	panel_data->panel_info.mipi.wr_mem_start =
			(!rc ? tmp : 0x2c);

	rc = of_property_read_u32(np,
		"qcom,mdss-pan-te-sel", &tmp);
	panel_data->panel_info.mipi.te_sel =
			(!rc ? tmp : 1);

	rc = of_property_read_u32(np,
		"qcom,mdss-pan-dsi-dst-format", &tmp);
	panel_data->panel_info.mipi.dst_format =
			(!rc ? tmp : DSI_VIDEO_DST_FORMAT_RGB888);

	rc = of_property_read_u32(np, "qcom,mdss-pan-dsi-vc", &tmp);
	panel_data->panel_info.mipi.vc = (!rc ? tmp : 0);

	rc = of_property_read_u32(np, "qcom,mdss-pan-dsi-rgb-swap", &tmp);
	panel_data->panel_info.mipi.rgb_swap = (!rc ? tmp : DSI_RGB_SWAP_RGB);

	rc = of_property_read_u32_array(np,
		"qcom,mdss-pan-dsi-data-lanes", res, 4);
	panel_data->panel_info.mipi.data_lane0 = (!rc ? res[0] : true);
	panel_data->panel_info.mipi.data_lane1 = (!rc ? res[1] : false);
	panel_data->panel_info.mipi.data_lane2 = (!rc ? res[2] : false);
	panel_data->panel_info.mipi.data_lane3 = (!rc ? res[3] : false);

	rc = of_property_read_u32(np, "qcom,mdss-pan-dsi-dlane-swap", &tmp);
	panel_data->panel_info.mipi.dlane_swap = (!rc ? tmp : 0);

	rc = of_property_read_u32_array(np, "qcom,mdss-pan-dsi-t-clk", res, 2);
	panel_data->panel_info.mipi.t_clk_pre = (!rc ? res[0] : 0x24);
	panel_data->panel_info.mipi.t_clk_post = (!rc ? res[1] : 0x03);

	rc = of_property_read_u32(np, "qcom,mdss-pan-dsi-stream", &tmp);
	panel_data->panel_info.mipi.stream = (!rc ? tmp : 0);

	rc = of_property_read_u32(np, "qcom,mdss-pan-dsi-mdp-tr", &tmp);
	panel_data->panel_info.mipi.mdp_trigger =
			(!rc ? tmp : DSI_CMD_TRIGGER_SW);
	if (panel_data->panel_info.mipi.mdp_trigger > 6) {
		pr_err("%s:%d, Invalid mdp trigger. Forcing to sw trigger",
						 __func__, __LINE__);
		panel_data->panel_info.mipi.mdp_trigger =
					DSI_CMD_TRIGGER_SW;
	}

	rc = of_property_read_u32(np, "qcom,mdss-pan-dsi-dma-tr", &tmp);
	panel_data->panel_info.mipi.dma_trigger =
			(!rc ? tmp : DSI_CMD_TRIGGER_SW);
	if (panel_data->panel_info.mipi.dma_trigger > 6) {
		pr_err("%s:%d, Invalid dma trigger. Forcing to sw trigger",
						 __func__, __LINE__);
		panel_data->panel_info.mipi.dma_trigger =
					DSI_CMD_TRIGGER_SW;
	}

	rc = of_property_read_u32(np, "qcom,mdss-pan-dsi-frame-rate", &tmp);
	panel_data->panel_info.mipi.frame_rate = (!rc ? tmp : 60);

	rc = of_property_read_u32(np, "qcom,mdss-pan-clk-rate", &tmp);
	panel_data->panel_info.clk_rate = (!rc ? tmp : 0);

	data = of_get_property(np, "qcom,panel-phy-regulatorSettings", &len);
	if ((!data) || (len != 7)) {
		pr_err("%s:%d, Unable to read Phy regulator settings",
		       __func__, __LINE__);
		goto error;
	}
	for (i = 0; i < len; i++)
		phy_params.regulator[i] = data[i];

	data = of_get_property(np, "qcom,panel-phy-timingSettings", &len);
	if ((!data) || (len != 12)) {
		pr_err("%s:%d, Unable to read Phy timing settings",
		       __func__, __LINE__);
		goto error;
	}
	for (i = 0; i < len; i++)
		phy_params.timing[i] = data[i];

	data = of_get_property(np, "qcom,panel-phy-strengthCtrl", &len);
	if ((!data) || (len != 2)) {
		pr_err("%s:%d, Unable to read Phy Strength ctrl settings",
		       __func__, __LINE__);
		goto error;
	}
	phy_params.strength[0] = data[0];
	phy_params.strength[1] = data[1];

	data = of_get_property(np, "qcom,panel-phy-bistCtrl", &len);
	if ((!data) || (len != 6)) {
		pr_err("%s:%d, Unable to read Phy Bist Ctrl settings",
		       __func__, __LINE__);
		goto error;
	}
	for (i = 0; i < len; i++)
		phy_params.bistCtrl[i] = data[i];

	data = of_get_property(np, "qcom,panel-phy-laneConfig", &len);
	if ((!data) || (len != 45)) {
		pr_err("%s:%d, Unable to read Phy lane configure settings",
		       __func__, __LINE__);
		goto error;
	}
	for (i = 0; i < len; i++)
		phy_params.laneCfg[i] = data[i];

	panel_data->panel_info.mipi.dsi_phy_db = &phy_params;

	fbc_enabled = of_property_read_bool(np,
			"qcom,fbc-enabled");
	if (fbc_enabled) {
		pr_debug("%s:%d FBC panel enabled.\n", __func__, __LINE__);
		panel_data->panel_info.fbc.enabled = 1;

		rc = of_property_read_u32_array(np,
				"qcom,fbc-mode", fbc_res, 7);
		panel_data->panel_info.fbc.target_bpp =
			(!rc ?	fbc_res[0] : panel_data->panel_info.bpp);
		panel_data->panel_info.fbc.comp_mode = (!rc ? fbc_res[1] : 0);
		panel_data->panel_info.fbc.qerr_enable =
			(!rc ? fbc_res[2] : 0);
		panel_data->panel_info.fbc.cd_bias = (!rc ? fbc_res[3] : 0);
		panel_data->panel_info.fbc.pat_enable = (!rc ? fbc_res[4] : 0);
		panel_data->panel_info.fbc.vlc_enable = (!rc ? fbc_res[5] : 0);
		panel_data->panel_info.fbc.bflc_enable =
			(!rc ? fbc_res[6] : 0);

		rc = of_property_read_u32_array(np,
				"qcom,fbc-budget-ctl", fbc_res, 3);
		panel_data->panel_info.fbc.line_x_budget =
			(!rc ? fbc_res[0] : 0);
		panel_data->panel_info.fbc.block_x_budget =
			(!rc ? fbc_res[1] : 0);
		panel_data->panel_info.fbc.block_budget =
			(!rc ? fbc_res[2] : 0);

		rc = of_property_read_u32_array(np,
				"qcom,fbc-lossy-mode", fbc_res, 4);
		panel_data->panel_info.fbc.lossless_mode_thd =
			(!rc ? fbc_res[0] : 0);
		panel_data->panel_info.fbc.lossy_mode_thd =
			(!rc ? fbc_res[1] : 0);
		panel_data->panel_info.fbc.lossy_rgb_thd =
			(!rc ? fbc_res[2] : 0);
		panel_data->panel_info.fbc.lossy_mode_idx =
			(!rc ? fbc_res[3] : 0);

	} else {
		pr_debug("%s:%d Panel does not support FBC.\n",
				__func__, __LINE__);
		panel_data->panel_info.fbc.enabled = 0;
		panel_data->panel_info.fbc.target_bpp =
			panel_data->panel_info.bpp;
	}






	mdss_dsi_parse_dcs_cmds(np, &panel_data->on_cmds,
		"qcom,panel-on-cmds", "qcom,on-cmds-dsi-state");



	mdss_dsi_parse_dcs_cmds(np, &panel_data->off_cmds,
		"qcom,panel-off-cmds", "qcom,off-cmds-dsi-state");
#ifdef CONFIG_F_SKYDISP_CABC_CONTROL	
	mdss_dsi_parse_dcs_cmds(np, &panel_data->cabc_cmds,
		"qcom,panel-cabc-cmds", "qcom,cabc-cmds-dsi-state");
#endif	

	return 0;
error:
	return -EINVAL;
}

static int __devinit mdss_dsi_panel_probe(struct platform_device *pdev)
{
	int rc = 0;
	static struct mdss_panel_common_pdata vendor_pdata;
	static const char *panel_name;

	pr_debug("%s:%d, debug info id=%d", __func__, __LINE__, pdev->id);
	if (!pdev->dev.of_node)
		return -ENODEV;

	panel_name = of_get_property(pdev->dev.of_node, "label", NULL);
	if (!panel_name)
		pr_info("%s:%d, panel name not specified\n",
						__func__, __LINE__);
	else
		pr_info("%s: Panel Name = %s\n", __func__, panel_name);

	rc = mdss_panel_parse_dt(pdev, &vendor_pdata);
	if (rc)
		return rc;

	vendor_pdata.on = mdss_dsi_panel_on;
	vendor_pdata.off = mdss_dsi_panel_off;
	vendor_pdata.bl_fnc = mdss_dsi_panel_bl_ctrl;

	rc = dsi_panel_device_register(pdev, &vendor_pdata);
	if (rc)
		return rc;

	return 0;
}

static const struct of_device_id mdss_dsi_panel_match[] = {
	{.compatible = "qcom,mdss-dsi-panel"},
	{}
};

static struct platform_driver this_driver = {
	.probe  = mdss_dsi_panel_probe,
	.driver = {
		.name   = "dsi_panel",
		.of_match_table = mdss_dsi_panel_match,
	},
};

static int __init mdss_dsi_panel_init(void)
{
	return platform_driver_register(&this_driver);
}
module_init(mdss_dsi_panel_init);
