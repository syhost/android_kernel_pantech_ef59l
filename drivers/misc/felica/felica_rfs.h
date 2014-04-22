/* kernel/driver/misc/felica/felica_rfs.h
 *
 * Copyright (C) 2012 Pantech Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef _FELICA_RFS_H
#define _FELICA_RFS_H

struct felica_rfs_pfdata;

int felica_rfs_probe_func(struct felica_rfs_pfdata *);
void felica_rfs_remove_func(void);

#endif
