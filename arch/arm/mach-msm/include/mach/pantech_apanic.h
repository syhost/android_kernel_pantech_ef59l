/* arch/arm/mach-msm/apanic_pantech.h
 *
 * Copyright (C) 2012 PANTECH, Co. Ltd.
 * based on drivers/misc/apanic.c
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

#ifndef __PANTECH_APANIC_H__
#define __PANTECH_APANIC_H__

#include <linux/sched.h>

#define SCHED_LOG_MAX 1024
#define PANTECH_INFO_MAGIC 0xdead1004

// (+) p16652 add for logcat parser
struct msg_log {
	void *kernellog_buf_adr;
	void *mainlogcat_buf_adr;
    void *mainlogcat_w_off;
    uint32_t mainlogcat_size;
	void *systemlogcat_buf_adr;
    void *systemlogcat_w_off;
    uint32_t systemlogcat_size;
};

struct sched_log {
	unsigned long long time;
	char comm[TASK_COMM_LEN];
	pid_t pid;
};

struct irq_log {
	unsigned int softirq_vec;
	unsigned int func;
	unsigned long long start_time;
	unsigned long long elapsed_time;
	int irqs_flag;
};

enum sched_type{
	MAIN_SCHED = (1<<0),
	WORKQUEUE_SCHED = (1<<1),
	PREEMPT_SCHED_IRQ = (1<<2),
	ALL_SCHED = (1<<3)
};

enum irq_type{
	SOFT_IRQ = (1<<0),
	TASKLET_ACTION = (1<<1),
	TASKLET_HI_ACTION = (1<<2),
	ALL_IRQ = (1<<3)
};

extern void pantech_force_dump_key(unsigned int code, int value);
extern void pantech_pm_log(int percent_soc, unsigned int cable_type);
extern void pantech_schedule_log(enum sched_type sched, int cpu, struct task_struct *task, char *msg);
extern void pantech_irq_log(unsigned int softirq_vec, unsigned int func, unsigned long long start_time);
extern void pantech_phone_info_log(void *phone_info);
#endif
