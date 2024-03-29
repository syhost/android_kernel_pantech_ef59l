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
#ifndef __Q6_ASM_V2_H__
#define __Q6_ASM_V2_H__

#include <mach/qdsp6v2/apr.h>
#include <sound/apr_audio-v2.h>
#include <linux/list.h>
#include <linux/msm_ion.h>

#define IN                      0x000
#define OUT                     0x001
#define CH_MODE_MONO            0x001
#define CH_MODE_STEREO          0x002

#define FORMAT_LINEAR_PCM   0x0000
#define FORMAT_DTMF         0x0001
#define FORMAT_ADPCM	    0x0002
#define FORMAT_YADPCM       0x0003
#define FORMAT_MP3          0x0004
#define FORMAT_MPEG4_AAC    0x0005
#define FORMAT_AMRNB	    0x0006
#define FORMAT_AMRWB	    0x0007
#define FORMAT_V13K	    0x0008
#define FORMAT_EVRC	    0x0009
#define FORMAT_EVRCB	    0x000a
#define FORMAT_EVRCWB	    0x000b
#define FORMAT_MIDI	    0x000c
#define FORMAT_SBC	    0x000d
#define FORMAT_WMA_V10PRO   0x000e
#define FORMAT_WMA_V9	    0x000f
#define FORMAT_AMR_WB_PLUS  0x0010
#define FORMAT_MPEG4_MULTI_AAC 0x0011
#define FORMAT_MULTI_CHANNEL_LINEAR_PCM 0x0012
#define FORMAT_AC3          0x0013
#define FORMAT_EAC3         0x0014

#define ENCDEC_SBCBITRATE   0x0001
#define ENCDEC_IMMEDIATE_DECODE 0x0002
#define ENCDEC_CFG_BLK          0x0003

#define CMD_PAUSE          0x0001
#define CMD_FLUSH          0x0002
#define CMD_EOS            0x0003
#define CMD_CLOSE          0x0004
#define CMD_OUT_FLUSH      0x0005

/* bit 0:1 represents priority of stream */
#define STREAM_PRIORITY_NORMAL	0x0000
#define STREAM_PRIORITY_LOW	0x0001
#define STREAM_PRIORITY_HIGH	0x0002

/* bit 4 represents META enable of encoded data buffer */
#define BUFFER_META_ENABLE	0x0010

/* Enable Sample_Rate/Channel_Mode notification event from Decoder */
#define SR_CM_NOTIFY_ENABLE	0x0004

#define SYNC_IO_MODE	0x0001
#define ASYNC_IO_MODE	0x0002
#define COMPRESSED_IO	0x0040
#define NT_MODE        0x0400

#define NO_TIMESTAMP    0xFF00
#define SET_TIMESTAMP   0x0000

#define SOFT_PAUSE_ENABLE	1
#define SOFT_PAUSE_DISABLE	0

#define SESSION_MAX		0x08
#define ASM_CONTROL_SESSION	0x0F

/* payload structure bytes */
#define READDONE_IDX_STATUS 0
#define READDONE_IDX_BUFADD_LSW 1
#define READDONE_IDX_BUFADD_MSW 2
#define READDONE_IDX_MEMMAP_HDL 3
#define READDONE_IDX_SIZE 4
#define READDONE_IDX_OFFSET 5
#define READDONE_IDX_LSW_TS 6
#define READDONE_IDX_MSW_TS 7
#define READDONE_IDX_FLAGS 8
#define READDONE_IDX_NUMFRAMES 9
#define READDONE_IDX_SEQ_ID 10

#define SOFT_PAUSE_PERIOD       30   /* ramp up/down for 30ms    */
#define SOFT_PAUSE_STEP         0 /* Step value 0ms or 0us */
enum {
	SOFT_PAUSE_CURVE_LINEAR = 0,
	SOFT_PAUSE_CURVE_EXP,
	SOFT_PAUSE_CURVE_LOG,
};

#define SOFT_VOLUME_PERIOD       30   /* ramp up/down for 30ms    */
#define SOFT_VOLUME_STEP         0 /* Step value 0ms or 0us */
enum {
	SOFT_VOLUME_CURVE_LINEAR = 0,
	SOFT_VOLUME_CURVE_EXP,
	SOFT_VOLUME_CURVE_LOG,
};

typedef void (*app_cb)(uint32_t opcode, uint32_t token,
			uint32_t *payload, void *priv);

struct audio_buffer {
	dma_addr_t phys;
	void       *data;
	uint32_t   used;
	uint32_t   size;/* size of buffer */
	uint32_t   actual_size; /* actual number of bytes read by DSP */
	struct      ion_handle *handle;
	struct      ion_client *client;
};

struct audio_aio_write_param {
	unsigned long paddr;
	uint32_t      len;
	uint32_t      uid;
	uint32_t      lsw_ts;
	uint32_t      msw_ts;
	uint32_t      flags;
};

struct audio_aio_read_param {
	unsigned long paddr;
	uint32_t      len;
	uint32_t      uid;
};

struct audio_port_data {
	struct audio_buffer *buf;
	uint32_t	    max_buf_cnt;
	uint32_t	    dsp_buf;
	uint32_t	    cpu_buf;
	struct list_head    mem_map_handle;
	uint32_t	    tmp_hdl;
	/* read or write locks */
	struct mutex	    lock;
	spinlock_t	    dsp_lock;
};

struct audio_client {
	int                    session;
	app_cb		       cb;
	atomic_t	       cmd_state;
	/* Relative or absolute TS */
	atomic_t	       time_flag;
	atomic_t	       nowait_cmd_cnt;
	void		       *priv;
	uint32_t               io_mode;
	uint64_t	       time_stamp;
	struct apr_svc         *apr;
	struct apr_svc         *mmap_apr;
	struct mutex	       cmd_lock;
	/* idx:1 out port, 0: in port*/
	struct audio_port_data port[2];
	wait_queue_head_t      cmd_wait;
	wait_queue_head_t      time_wait;
	bool                   perf_mode;
	/* audio cache operations fptr*/
	int (*fptr_cache_ops)(struct audio_buffer *abuff, int cache_op);
};

void q6asm_audio_client_free(struct audio_client *ac);

struct audio_client *q6asm_audio_client_alloc(app_cb cb, void *priv);

struct audio_client *q6asm_get_audio_client(int session_id);

int q6asm_audio_client_buf_alloc(unsigned int dir/* 1:Out,0:In */,
				struct audio_client *ac,
				unsigned int bufsz,
				unsigned int bufcnt);
int q6asm_audio_client_buf_alloc_contiguous(unsigned int dir
				/* 1:Out,0:In */,
				struct audio_client *ac,
				unsigned int bufsz,
				unsigned int bufcnt);

int q6asm_audio_client_buf_free_contiguous(unsigned int dir,
			struct audio_client *ac);

int q6asm_open_read(struct audio_client *ac, uint32_t format
		/*, uint16_t bits_per_sample*/);

int q6asm_open_write(struct audio_client *ac, uint32_t format
		/*, uint16_t bits_per_sample*/);

int q6asm_open_write_v2(struct audio_client *ac, uint32_t format,
			uint16_t bits_per_sample);

int q6asm_open_read_write(struct audio_client *ac,
			uint32_t rd_format,
			uint32_t wr_format);

int q6asm_write(struct audio_client *ac, uint32_t len, uint32_t msw_ts,
				uint32_t lsw_ts, uint32_t flags);
int q6asm_write_nolock(struct audio_client *ac, uint32_t len, uint32_t msw_ts,
				uint32_t lsw_ts, uint32_t flags);

int q6asm_async_write(struct audio_client *ac,
					  struct audio_aio_write_param *param);

int q6asm_async_read(struct audio_client *ac,
					  struct audio_aio_read_param *param);

int q6asm_read(struct audio_client *ac);
int q6asm_read_nolock(struct audio_client *ac);

int q6asm_memory_map(struct audio_client *ac, uint32_t buf_add,
			int dir, uint32_t bufsz, uint32_t bufcnt);

int q6asm_memory_unmap(struct audio_client *ac, uint32_t buf_add,
							int dir);

int q6asm_unmap_cal_blocks(void);

int q6asm_run(struct audio_client *ac, uint32_t flags,
		uint32_t msw_ts, uint32_t lsw_ts);

int q6asm_run_nowait(struct audio_client *ac, uint32_t flags,
		uint32_t msw_ts, uint32_t lsw_ts);

int q6asm_reg_tx_overflow(struct audio_client *ac, uint16_t enable);

int q6asm_cmd(struct audio_client *ac, int cmd);

int q6asm_cmd_nowait(struct audio_client *ac, int cmd);

void *q6asm_is_cpu_buf_avail(int dir, struct audio_client *ac,
				uint32_t *size, uint32_t *idx);

void *q6asm_is_cpu_buf_avail_nolock(int dir, struct audio_client *ac,
					uint32_t *size, uint32_t *idx);

int q6asm_is_dsp_buf_avail(int dir, struct audio_client *ac);

/* File format specific configurations to be added below */

int q6asm_enc_cfg_blk_aac(struct audio_client *ac,
			 uint32_t frames_per_buf,
			uint32_t sample_rate, uint32_t channels,
			 uint32_t bit_rate,
			 uint32_t mode, uint32_t format);

int q6asm_enc_cfg_blk_pcm(struct audio_client *ac,
			uint32_t rate, uint32_t channels);

int q6asm_set_encdec_chan_map(struct audio_client *ac,
		uint32_t num_channels);

int q6asm_enc_cfg_blk_pcm_native(struct audio_client *ac,
			uint32_t rate, uint32_t channels);

int q6asm_enable_sbrps(struct audio_client *ac,
			uint32_t sbr_ps);

int q6asm_cfg_dual_mono_aac(struct audio_client *ac,
			uint16_t sce_left, uint16_t sce_right);

int q6asm_cfg_aac_sel_mix_coef(struct audio_client *ac, uint32_t mix_coeff);

int q6asm_enc_cfg_blk_qcelp(struct audio_client *ac, uint32_t frames_per_buf,
		uint16_t min_rate, uint16_t max_rate,
		uint16_t reduced_rate_level, uint16_t rate_modulation_cmd);

int q6asm_enc_cfg_blk_evrc(struct audio_client *ac, uint32_t frames_per_buf,
		uint16_t min_rate, uint16_t max_rate,
		uint16_t rate_modulation_cmd);

int q6asm_enc_cfg_blk_amrnb(struct audio_client *ac, uint32_t frames_per_buf,
		uint16_t band_mode, uint16_t dtx_enable);

int q6asm_enc_cfg_blk_amrwb(struct audio_client *ac, uint32_t frames_per_buf,
		uint16_t band_mode, uint16_t dtx_enable);

int q6asm_media_format_block_pcm(struct audio_client *ac,
			uint32_t rate, uint32_t channels);

int q6asm_media_format_block_pcm_format_support(struct audio_client *ac,
			uint32_t rate, uint32_t channels,
			uint16_t bits_per_sample);

int q6asm_media_format_block_multi_ch_pcm(struct audio_client *ac,
			uint32_t rate, uint32_t channels,
			bool use_default_chmap, char *channel_map);

int q6asm_media_format_block_multi_ch_pcm_v2(
			struct audio_client *ac,
			uint32_t rate, uint32_t channels,
			bool use_default_chmap, char *channel_map,
			uint16_t bits_per_sample);

int q6asm_media_format_block_aac(struct audio_client *ac,
			struct asm_aac_cfg *cfg);

int q6asm_media_format_block_multi_aac(struct audio_client *ac,
			struct asm_aac_cfg *cfg);

int q6asm_media_format_block_wma(struct audio_client *ac,
			void *cfg);

int q6asm_media_format_block_wmapro(struct audio_client *ac,
			void *cfg);

int q6asm_media_format_block_amrwbplus(struct audio_client *ac,
			struct asm_amrwbplus_cfg *cfg);

int q6asm_ds1_set_endp_params(struct audio_client *ac,
				int param_id, int param_value);

#ifdef CONFIG_PANTECH_SND_QSOUND //20131014 hdj define error

#define QSOUND_EQ_MODULE_ID						0x1000c000
#define QSOUND_EQ_ENABLE						0x1000c001
#define QSOUND_EQ_NUM_BANDS						0x1000c002
#define QSOUND_EQ_LEVEL_RANGE					0x1000c003
#define QSOUND_EQ_CURRENT_BAND					0x1000c00b	// used for band level, center freq, and freq range
#define QSOUND_EQ_BAND_LEVEL					0x1000c004
#define QSOUND_EQ_CENTER_FREQ					0x1000c005
#define QSOUND_EQ_FREQ_RANGE					0x1000c006
#define QSOUND_EQ_CUR_PRESET					0x1000c008
#define QSOUND_EQ_NUM_PRESETS					0x1000c009
#define QSOUND_EQ_PRESET_NAME					0x1000c00a
#define QSOUND_EQ_BULK_LEVEL					0x1000c00c
#define QSOUND_EQ_BAND_LEVELS					0x1000c00e	// Gets/Sets the gain set for all the equalizer bands. (#bands * int16_t)

#define QSOUND_VIRTUAL_MODULE_ID				0x1000c010
#define QSOUND_VIRTUAL_ENABLE_ID				0x1000c011
#define QSOUND_VIRTUAL_SPREAD_ID				0x1000c012

#define QSOUND_REVERB_MODULE_ID					0x1000c020
#define QSOUND_REVERB_PRESET_ID					0x1000c021

#define QSOUND_BASSBOOST_MODULE_ID				0x1000c030
#define QSOUND_BASSBOOST_ENABLE_ID				0x1000c031
#define QSOUND_BASSBOOST_STRENGTH_ID			0x1000c032

#define QSOUND_LIMITER_MODULE_ID				0x1000c040
#define QSOUND_LIMITER_ENABLE_ID				0x1000c041

#define QSOUND_EXTREME_VOL_MODULE_ID			0x1000c050
#define QSOUND_EXTREME_VOL_ENABLE_ID			0x1000c051

#define QSOUND_EXCITER_MODULE_ID				0x1000c070
#define QSOUND_EXCITER_ENABLE_ID				0x1000c071
#define QSOUND_EXCITER_STRENGTH_ID				0x1000c072

int q6asm_set_mqfx_param(int session_id, uint32_t module_id, uint32_t param_id, const void* param, size_t size);
int q6asm_get_mqfx_param(int session_id, uint32_t module_id, uint32_t param_id, void* param, size_t size);

#endif  //CONFIG_PANTECH_SND_QSOUND


/* PP specific */
int q6asm_equalizer(struct audio_client *ac, void *eq);

/* Send Volume Command */
int q6asm_set_volume(struct audio_client *ac, int volume);

/* Set SoftPause Params */
int q6asm_set_softpause(struct audio_client *ac,
			struct asm_softpause_params *param);

/* Set Softvolume Params */
int q6asm_set_softvolume(struct audio_client *ac,
			struct asm_softvolume_params *param);

/* Send left-right channel gain */
int q6asm_set_lrgain(struct audio_client *ac, int left_gain, int right_gain);

/* Enable Mute/unmute flag */
int q6asm_set_mute(struct audio_client *ac, int muteflag);

int q6asm_get_session_time(struct audio_client *ac, uint64_t *tstamp);

/* Client can set the IO mode to either AIO/SIO mode */
int q6asm_set_io_mode(struct audio_client *ac, uint32_t mode);

/* Get Service ID for APR communication */
int q6asm_get_apr_service_id(int session_id);

/* Common format block without any payload
*/
int q6asm_media_format_block(struct audio_client *ac, uint32_t format);

#endif /* __Q6_ASM_H__ */
