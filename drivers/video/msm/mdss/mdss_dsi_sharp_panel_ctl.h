#ifndef MDSS_DSI_PANEL_NAMI
#define MDSS_DSI_PANEL_NAMI

#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>
#include <linux/clk.h>

#include "mdss.h"
#include "mdss_panel.h"
#include "mdss_dsi.h"
#include "mdss_debug.h"

void mdss_dsi_panel_reset_nami(struct mdss_panel_data *pdata, int enable);
int mdss_dsi_panel_power_on_nami(struct mdss_panel_data *pdata, int enable);
int mdss_dsi_regulator_init_nami(struct platform_device *pdev);
int dsi_panel_device_register_nami(struct mdss_dsi_ctrl_pdata *ctrl_pdata,struct platform_device *pdev);
int setLm3530Bright(int brightness,struct mdss_dsi_ctrl_pdata *ctrl);

int sharp_panel_sysfs_register(struct device *dev);
void sharp_set_settting_cmds(struct mdss_dsi_ctrl_pdata *ctrl);
void sharp_blc_on(struct mdss_dsi_ctrl_pdata *ctrl);
void sharp_blc_off(struct mdss_dsi_ctrl_pdata *ctrl);
void sharp_panel_on_init(struct mdss_panel_data *pdata);
#endif
