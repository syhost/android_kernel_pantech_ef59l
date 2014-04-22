/* kernel/driver/misc/felica/felica_int.h
 *
 * Copyright (C) 2012 Pantech Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef _FELICA_INT_H
#define _FELICA_INT_H

int felica_int_probe_func(struct felica_device *);
void felica_int_remove_func(void);

#endif
