/* arch/arm/mach-msm/smd_private.h
 *
 * Copyright (C) 2007 Google, Inc.
 * Copyright (c) 2007-2013, The Linux Foundation. All rights reserved.
 *
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
#ifndef _ARCH_ARM_MACH_MSM_MSM_SMD_PRIVATE_H_
#define _ARCH_ARM_MACH_MSM_MSM_SMD_PRIVATE_H_

#include <linux/types.h>
#include <linux/spinlock.h>
#include <linux/errno.h>
#include <linux/remote_spinlock.h>
#include <linux/platform_device.h>
#include <mach/msm_smsm.h>
#include <mach/msm_smd.h>

#define PC_APPS  0
#define PC_MODEM 1

#define VERSION_QDSP6     4
#define VERSION_APPS_SBL  6
#define VERSION_MODEM_SBL 7
#define VERSION_APPS      8
#define VERSION_MODEM     9
#define VERSION_DSPS      10

#if defined(CONFIG_MSM_SMD_PKG4)
struct smsm_interrupt_info {
	uint32_t aArm_en_mask;
	uint32_t aArm_interrupts_pending;
	uint32_t aArm_wakeup_reason;
	uint32_t aArm_rpc_prog;
	uint32_t aArm_rpc_proc;
	char aArm_smd_port_name[20];
	uint32_t aArm_gpio_info;
};
#elif defined(CONFIG_MSM_SMD_PKG3)
struct smsm_interrupt_info {
  uint32_t aArm_en_mask;
  uint32_t aArm_interrupts_pending;
  uint32_t aArm_wakeup_reason;
};
#elif !defined(CONFIG_MSM_SMD)
/* Don't trigger the error */
#else
#error No SMD Package Specified; aborting
#endif

#define SZ_DIAG_ERR_MSG 0xC8
#define ID_DIAG_ERR_MSG SMEM_DIAG_ERR_MESSAGE
#define ID_SMD_CHANNELS SMEM_SMD_BASE_ID
#define ID_SHARED_STATE SMEM_SMSM_SHARED_STATE
#define ID_CH_ALLOC_TBL SMEM_CHANNEL_ALLOC_TBL

#define SMD_SS_CLOSED            0x00000000
#define SMD_SS_OPENING           0x00000001
#define SMD_SS_OPENED            0x00000002
#define SMD_SS_FLUSHING          0x00000003
#define SMD_SS_CLOSING           0x00000004
#define SMD_SS_RESET             0x00000005
#define SMD_SS_RESET_OPENING     0x00000006

#define SMD_BUF_SIZE             8192
#define SMD_CHANNELS             64
#define SMD_HEADER_SIZE          20

#ifdef CONFIG_PANTECH_ERR_CRASH_LOGGING
#define MAX_CRASH_BUF_SIZE 0x60000 // 384Kbytes
#endif
/* 'type' field of smd_alloc_elm structure
 * has the following breakup
 * bits 0-7   -> channel type
 * bits 8-11  -> xfer type
 * bits 12-31 -> reserved
 */
struct smd_alloc_elm {
	char name[20];
	uint32_t cid;
	uint32_t type;
	uint32_t ref_count;
};

#define SMD_CHANNEL_TYPE(x) ((x) & 0x000000FF)
#define SMD_XFER_TYPE(x)    (((x) & 0x00000F00) >> 8)

struct smd_half_channel {
	unsigned state;
	unsigned char fDSR;
	unsigned char fCTS;
	unsigned char fCD;
	unsigned char fRI;
	unsigned char fHEAD;
	unsigned char fTAIL;
	unsigned char fSTATE;
	unsigned char fBLOCKREADINTR;
	unsigned tail;
	unsigned head;
};

struct smd_half_channel_word_access {
	unsigned state;
	unsigned fDSR;
	unsigned fCTS;
	unsigned fCD;
	unsigned fRI;
	unsigned fHEAD;
	unsigned fTAIL;
	unsigned fSTATE;
	unsigned fBLOCKREADINTR;
	unsigned tail;
	unsigned head;
};

struct smd_half_channel_access {
	void (*set_state)(volatile void __iomem *half_channel, unsigned data);
	unsigned (*get_state)(volatile void __iomem *half_channel);
	void (*set_fDSR)(volatile void __iomem *half_channel,
					unsigned char data);
	unsigned (*get_fDSR)(volatile void __iomem *half_channel);
	void (*set_fCTS)(volatile void __iomem *half_channel,
					unsigned char data);
	unsigned (*get_fCTS)(volatile void __iomem *half_channel);
	void (*set_fCD)(volatile void __iomem *half_channel,
					unsigned char data);
	unsigned (*get_fCD)(volatile void __iomem *half_channel);
	void (*set_fRI)(volatile void __iomem *half_channel,
					unsigned char data);
	unsigned (*get_fRI)(volatile void __iomem *half_channel);
	void (*set_fHEAD)(volatile void __iomem *half_channel,
					unsigned char data);
	unsigned (*get_fHEAD)(volatile void __iomem *half_channel);
	void (*set_fTAIL)(volatile void __iomem *half_channel,
					unsigned char data);
	unsigned (*get_fTAIL)(volatile void __iomem *half_channel);
	void (*set_fSTATE)(volatile void __iomem *half_channel,
					unsigned char data);
	unsigned (*get_fSTATE)(volatile void __iomem *half_channel);
	void (*set_fBLOCKREADINTR)(volatile void __iomem *half_channel,
					unsigned char data);
	unsigned (*get_fBLOCKREADINTR)(volatile void __iomem *half_channel);
	void (*set_tail)(volatile void __iomem *half_channel, unsigned data);
	unsigned (*get_tail)(volatile void __iomem *half_channel);
	void (*set_head)(volatile void __iomem *half_channel, unsigned data);
	unsigned (*get_head)(volatile void __iomem *half_channel);
};

int is_word_access_ch(unsigned ch_type);

struct smd_half_channel_access *get_half_ch_funcs(unsigned ch_type);

struct smd_channel {
	volatile void __iomem *send; /* some variant of smd_half_channel */
	volatile void __iomem *recv; /* some variant of smd_half_channel */
	unsigned char *send_data;
	unsigned char *recv_data;
	unsigned fifo_size;
	unsigned fifo_mask;
	struct list_head ch_list;

	unsigned current_packet;
	unsigned n;
	void *priv;
	void (*notify)(void *priv, unsigned flags);

	int (*read)(smd_channel_t *ch, void *data, int len, int user_buf);
	int (*write)(smd_channel_t *ch, const void *data, int len,
			int user_buf);
	int (*read_avail)(smd_channel_t *ch);
	int (*write_avail)(smd_channel_t *ch);
	int (*read_from_cb)(smd_channel_t *ch, void *data, int len,
			int user_buf);

	void (*update_state)(smd_channel_t *ch);
	unsigned last_state;
	void (*notify_other_cpu)(smd_channel_t *ch);

	char name[20];
	struct platform_device pdev;
	unsigned type;

	int pending_pkt_sz;

	char is_pkt_ch;

	/*
	 * private internal functions to access *send and *recv.
	 * never to be exported outside of smd
	 */
	struct smd_half_channel_access *half_ch;
};

extern spinlock_t smem_lock;


void smd_diag(void);

struct interrupt_stat {
	uint32_t smd_in_count;
	uint32_t smd_out_hardcode_count;
	uint32_t smd_out_config_count;
	uint32_t smd_interrupt_id;

	uint32_t smsm_in_count;
	uint32_t smsm_out_hardcode_count;
	uint32_t smsm_out_config_count;
	uint32_t smsm_interrupt_id;
};
extern struct interrupt_stat interrupt_stats[NUM_SMD_SUBSYSTEMS];
#ifdef CONFIG_PANTECH_ERR_CRASH_LOGGING
void smem_diag_get_message( char *message, int len );
void smem_diag_set_message(char *message);
#endif
#endif
