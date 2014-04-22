#ifndef __MAX17058_BATTERY_H
#define __MAX17058_BATTERY_H

#include <linux/errno.h>

// sayuss CONFIG_PANTECH_PMIC_FUELGAUGE_MAX17058
int max17058_get_soc(void);
int max17058_get_voltage(void);
void max17058_update_rcomp(int temp);

#endif
