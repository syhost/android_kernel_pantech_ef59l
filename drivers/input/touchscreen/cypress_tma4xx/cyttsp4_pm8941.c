/*
 * Core Source for:
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

#include "cyttsp4_pm8941.h"

int hw_on_off(struct device *dev, int on)
{
    struct regulator *vreg_touch_avdd;
    int rc =0;    
    static int vreg_touch_avdd_state = false;    

    if(on != vreg_touch_avdd_state) {

		printk("%s: ++++ Power Up\n", __func__);
        vreg_touch_avdd = regulator_get(NULL, TOUCH_POWER_VDD);
        if(on) {
            rc = regulator_set_voltage(vreg_touch_avdd, 2800000, 2800000);
            if (rc) { 
                printk("%s: set_voltage 8941_l10_2p8 failed, rc=%d\n", __func__, rc);
                return -EINVAL;
            }
            rc = regulator_enable(vreg_touch_avdd);
            if (rc) { 
                printk("%s: regulator_enable vreg_touch_avdd failed, rc=%d\n", __func__, rc);
                return -EINVAL;
            }
        }
        else {
            rc = regulator_disable(vreg_touch_avdd);
            if (rc) { 
                printk("%s: regulator_disable vreg_touch_avdd failed, rc=%d\n", __func__, rc);
                return -EINVAL;
            }
        }
        regulator_put(vreg_touch_avdd);
        vreg_touch_avdd_state = on;

		printk("%s: ---- Power Up\n", __func__);
    }
    msleep(5);
    return 0;
}
EXPORT_SYMBOL_GPL(hw_on_off);

int init_hw_setting(struct device *dev)
{
    int rc =0;
    
    rc = hw_on_off(dev, true);
    if(rc<0) {
        printk("%s: fail to turn on power. rc=%d\n", __func__, rc);
        return rc;
    }

	printk("%s: ++++ GPIO Pin Setup: RST=%d, INT=%d\n", __func__, GPIO_TOUCH_RST, GPIO_TOUCH_CHG);

    rc = gpio_request(GPIO_TOUCH_RST, "touch_rst");
    if (rc) {
        gpio_free(GPIO_TOUCH_RST);
        rc = gpio_request(GPIO_TOUCH_RST, "touch_rst");
        if (rc) {
            printk("%s: gpio_request GPIO_TOUCH_RST : %d failed, rc=%d\n",__func__, GPIO_TOUCH_RST, rc);
        }
    }
    rc = gpio_direction_output(GPIO_TOUCH_RST, 1);
    if (rc) {
        printk("%s: gpio_direction_output GPIO_TOUCH_RST : %d failed, rc=%d\n",__func__, GPIO_TOUCH_RST, rc);
    }

    rc = gpio_request(GPIO_TOUCH_CHG, "touch_chg");
    if (rc) {
        gpio_free(GPIO_TOUCH_CHG);
        rc = gpio_request(GPIO_TOUCH_CHG, "touch_chg");
        if (rc) {
            printk("%s: gpio_request GPIO_TOUCH_CHG : %d failed, rc=%d\n",__func__, GPIO_TOUCH_CHG, rc);
        }
    }  

    rc = gpio_direction_input(GPIO_TOUCH_CHG);
    if (rc) {
        printk("%s: gpio_direction_input gpio_chg : %d failed, rc=%d\n",__func__, GPIO_TOUCH_CHG, rc);
    }

	printk("%s: ---- GPIO Pin Setup\n", __func__);

    msleep(5);
    return 0;
}
EXPORT_SYMBOL_GPL(init_hw_setting);
