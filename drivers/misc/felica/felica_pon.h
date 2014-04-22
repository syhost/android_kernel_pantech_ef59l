/* kernel/driver/misc/felica/felica_pon.h
 *
 * Copyright (C) 2012 Pantech Inc. *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef _FELICA_PON_H
#define _FELICA_PON_H

struct felica_devce;

int felica_pon_probe_func(struct felica_device *);
void felica_pon_remove_func(void);
#endif
