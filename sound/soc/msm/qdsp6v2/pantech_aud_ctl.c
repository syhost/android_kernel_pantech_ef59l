/************************************************************************************************
**
**    PANTECH AUDIO
**
**    FILE
**        pantech_audio.c
**
**    DESCRIPTION
**        This file contains pantech audio apis
**
**    Copyright (c) 2012 by PANTECH Incorporated.  All Rights Reserved.
*************************************************************************************************/

/************************************************************************************************
** Includes
*************************************************************************************************/
#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <mach/gpio.h>
#include <asm/ioctls.h>
#include <linux/uaccess.h>

#include "pantech_aud_ctl.h"


/*==========================================================================
** pantech_audio_ioctl
**=========================================================================*/
#ifdef CONFIG_PANTECH_SND_QSOUND
#include "msm-pcm-routing-v2.h"

#define NUM_TRACKS 200
#define NUM_EQ_BANDS 8
#define EQ_PRESET_MAX 20
#define EQ_LVL_MAX 18000
#define BASSBOOST_VALUE_MAX 1000
#define EXCITER_VALUE_MAX 1000
#define VIRTUALIZER_VALUE_MAX 1000
#define REVERB_PRESET_MAX 6

static uint32_t limiter_module_enable = 0;
//static uint32_t qvss_module_enable = 0;
static int session_id = 0;
static uint16_t eq_band = 0;

static int lpa_on = 0;

static int mCur_set_session_id = -1;
static int mCur_match_session_id = -1;

struct track_effect_t {
	int			session_id;
	bool		using_eq_preset;
	int			eq_preset;
	int16_t		eq_level[NUM_EQ_BANDS];
	uint32_t	bassboost_val;
	uint32_t	exciter_val;
	uint32_t	virtualizer_val;
	int			reverb_val;
};
struct track_effect_t track_effect[NUM_TRACKS];

static const int16_t eq_lvl_hs_default[NUM_EQ_BANDS] = {150,100,0,0,-150,-300,250,0};

static struct track_effect_t* find_current_track(void)
{
	int i;
	for (i=0; i<NUM_TRACKS; i++) {
		if ((track_effect[i].session_id != -1) && (mCur_set_session_id == track_effect[i].session_id)) {
			return &track_effect[i];
		}
	}
	return 0;
}

#endif  //CONFIG_PANTECH_SND_QSOUND

static __inline bool is_valid_bool(uint32_t value)
{
	return value==0 || value==1;
}

static long pantech_audio_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	long ret = 0;

#ifdef CONFIG_PANTECH_SND_QSOUND
	uint32_t qsound_data = 0;
	int sessionid = get_aud_session_id();
	int dai_mm = get_dai_mm();
	struct track_effect_t* track = 0;

	int i = 0;
	int j = 0;

	switch (cmd) {  // keep native data
		case PANTECH_AUDIO_SET_SESSION_ID_CTL:{
			if (copy_from_user(&qsound_data, (void __user *)arg, sizeof(qsound_data))) {
				ret = -1;
				break;
			}

			for (i=0; i<NUM_TRACKS;i++){
				if ((track_effect[i].session_id == qsound_data) || (track_effect[i].session_id == -1)){
					track_effect[i].session_id = qsound_data;
					mCur_set_session_id = qsound_data;
//					printk("pantech_audio_ioctl : PANTECH_AUDIO_SET_SESSION_ID_CTL (mCur_set_session_id=%d).........track_effect[%d].session_id : %d\n", qsound_data,i,track_effect[i].session_id);
					break;
				}
			}
			return 0;
		}
		case PANTECH_AUDIO_DESTROY_SESSION_ID_CTL:{
			if (copy_from_user(&qsound_data, (void __user *)arg, sizeof(qsound_data))) {
				ret = -1;
				break;
			}
			printk("%s: PANTECH_AUDIO_DESTROY_SESSION_ID_CTL (session=%d)\n", __func__, qsound_data);

			for (i=0; i<NUM_TRACKS;i++){
				if (track_effect[i].session_id == qsound_data){
					track_effect[i].session_id = -1;
					track_effect[i].using_eq_preset = false;
					track_effect[i].eq_preset = -1;
					for (j=0; j<NUM_EQ_BANDS;j++){
						track_effect[i].eq_level[j] = 0;
					}
					track_effect[i].bassboost_val = -1;
					track_effect[i].exciter_val = -1;
					track_effect[i].virtualizer_val = -1;
					track_effect[i].reverb_val = -1;
					break;
				}
			}
			return 0;
		}

		case PANTECH_AUDIO_EQ_KEEP_PRESET_CTL:{
			if (copy_from_user(&qsound_data, (void __user *)arg, sizeof(qsound_data))) {
				ret = -1;
				break;
			}
			
			//printk("%s: PANTECH_AUDIO_EQ_KEEP_PRESET_CTL (preset=%d)\n", __func__, qsound_data);
			
			if (qsound_data >= EQ_PRESET_MAX) {
				return  -1;
			}

			if ((track = find_current_track())!=0) {
				track->eq_preset = qsound_data;
				track->using_eq_preset = true;
			}
			return 0;
		}
		case PANTECH_AUDIO_EQ_BAND_CTL: {
			if (copy_from_user(&qsound_data, (void __user *)arg, sizeof(qsound_data))) {
				ret = -1;
				break;
			}
			//printk("%s: PANTECH_AUDIO_EQ_BAND_CTL (band=%d\n", __func__, qsound_data);

			if (qsound_data>=NUM_EQ_BANDS) {
				pr_err(" invalid eq band number: %d\n", qsound_data);
				ret = -1;
			}
			else {
				eq_band = (uint16_t)qsound_data;
			}
			return 0;
		}
		case PANTECH_AUDIO_EQ_KEEP_LVL_CTL:{
			if (copy_from_user(&qsound_data, (void __user *)arg, sizeof(qsound_data))) {
				ret = -1;
				break;
			}
			//printk("%s: PANTECH_AUDIO_EQ_KEEP_LVL_CTL (band level=%d)\n", __func__, (int32_t)qsound_data);

			if ((track = find_current_track())!=0) {
				track->using_eq_preset = false;
				if ((qsound_data & 0xffff0000) != 0){
					track->eq_level[eq_band] = (int16_t)((qsound_data-1)-65535 /*0xffff*/);
				}else{
					track->eq_level[eq_band] = (int16_t) qsound_data;
				}
			}

			return 0;
		}
		case PANTECH_AUDIO_BASS_BOOST_KEEP_VAL_CTL:{
			if (copy_from_user(&qsound_data, (void __user *)arg, sizeof(qsound_data))) {
				ret = -1;
				break;
			}
			//printk("%s: PANTECH_AUDIO_BASS_BOOST_KEEP_VAL_CTL (bassboost level=%d)\n", __func__, qsound_data);

			if ((track = find_current_track())!=0) {
				track->bassboost_val = qsound_data;
			}

			return 0;
		}
		case PANTECH_AUDIO_EXCITER_KEEP_VAL_CTL:{
			if (copy_from_user(&qsound_data, (void __user *)arg, sizeof(qsound_data))) {
				ret = -1;
				break;
			}
			//printk("%s: PANTECH_AUDIO_EXCITER_KEEP_VAL_CTL (exciter level=%d)\n", __func__, qsound_data);

			if ((track = find_current_track())!=0) {
				track->exciter_val = qsound_data;
			}

			return 0;
		}
		case PANTECH_AUDIO_VIRTUAL_KEEP_VALUE_CTL:{
			if (copy_from_user(&qsound_data, (void __user *)arg, sizeof(qsound_data))) {
				ret = -1;
				break;
			}
//			printk("%s: PANTECH_AUDIO_VIRTUAL_KEEP_VALUE_CTL (spread=%d)\n", __func__, qsound_data);

			if ((track = find_current_track())!=0) {
				track->virtualizer_val = qsound_data;
			}

			return 0;
		}
		case PANTECH_AUDIO_KEEP_PRESET_REVERB_CTL:{
			if (copy_from_user(&qsound_data, (void __user *)arg, sizeof(qsound_data))) {
				ret = -1;
				break;
			}
			//printk("%s: PANTECH_AUDIO_KEEP_PRESET_REVERB_CTL (preset=%d)\n", __func__, qsound_data);

			if ((track = find_current_track())!=0) {
				track->reverb_val = qsound_data;
			}
			return 0;
		}
	}


	if ((sessionid > 0) && (sessionid < 0x8)){
		if ((dai_mm != 0) && (dai_mm != 2)){ // it is not matched dai mm MULTIMEDIA1 / MULTIMEDIA3
			printk("\n@#@#[SKY SND] sessionid=%d  dai_mm : %d \n", sessionid, dai_mm);
			return 10;
		}
		session_id = sessionid;
		if (!lpa_on) session_id = get_aud_non_lpa_session_id();
	}
	else
		session_id = 10;
#endif

	switch (cmd) {
#ifdef CONFIG_PANTECH_SND_QSOUND
		case PANTECH_AUDIO_LPA_EFFECT_CTL:{
			if (copy_from_user(&qsound_data, (void __user *)arg, sizeof(qsound_data))) {
				ret = -1;
				break;
			}

			//printk("%s: PANTECH_AUDIO_LPA_EFFECT_CTL (LPA on=%d)\n", __func__, qsound_data);
			
			lpa_on = qsound_data;
			if (qsound_data == 1){
				if (get_lpa_active() == 1){
					return 0;
				}else{
					return -EINVAL;
				}
			}else{
				return 0;
			}

			break;
		}
		case PANTECH_AUDIO_MATCH_SESSION_ID_CTL: {
			if (copy_from_user(&qsound_data, (void __user *)arg, sizeof(qsound_data))) {
				ret = -1;
				break;
			}

			//printk("%s: PANTECH_AUDIO_MATCH_SESSION_ID_CTL (session=%d)\n", __func__, qsound_data);
			
			mCur_match_session_id = qsound_data;
			break;
		}
		case PANTECH_AUDIO_EQ_MODE_CTL: {
			if (lpa_on && !get_lpa_active()) {
				printk("@#@#PANTECH_AUDIO_EQ_MODE_CTL......lpa_active not ready !!!!!\n");
				return -EINVAL;
			}

			if (copy_from_user(&qsound_data, (void __user *)arg, sizeof(qsound_data))) {
				ret = -1;
				break;
			}
			
			if (!is_valid_bool(qsound_data)){
				pr_err("PANTECH_AUDIO_EQ_MODE_CTL invalid eq enable flag: %d\n", qsound_data);
				return -EINVAL;
			}
			
			//printk("%s: PANTECH_AUDIO_EQ_MODE_CTL (enable=%d)\n", __func__, qsound_data);

			ret = q6asm_set_mqfx_param(session_id, QSOUND_EQ_MODULE_ID, QSOUND_EQ_ENABLE, &qsound_data, sizeof(qsound_data));
			if (ret < 0) {
				pr_err(" failed eq module enable\n");
				break;
			}
			break;
		}
		case PANTECH_AUDIO_EQ_PRESET_CTL: {
			qsound_data = EQ_PRESET_MAX; // initialize with invalid preset number
			for (i=0; i<NUM_TRACKS;i++){
				if ((track_effect[i].session_id != -1) && (mCur_match_session_id == track_effect[i].session_id) && (track_effect[i].using_eq_preset == true)){
					qsound_data = track_effect[i].eq_preset;
					break;
				}
			}
			if (qsound_data >= EQ_PRESET_MAX) {
				break;  // current session is using user eq, just exit
			}

			if (lpa_on == 1){
				if (get_lpa_active() == 0){
					printk("@#@#PANTECH_AUDIO_EQ_PRESET_CTL......lpa_active not ready !!!!!\n");
					return -EINVAL;
				}
			}

			//printk("%s: PANTECH_AUDIO_EQ_PRESET_CTL (preset=%d)\n", __func__, qsound_data);
			
			ret = q6asm_set_mqfx_param(session_id, QSOUND_EQ_MODULE_ID, QSOUND_EQ_CUR_PRESET,
				&qsound_data, sizeof(qsound_data));
			if (ret < 0) {
				pr_err(" failed eq preset\n");
				break;
			}
			break;
		}
		case PANTECH_AUDIO_HEADSET_DEFAULT_CTL: {
//			int non_lpa_sessionid = get_aud_non_lpa_session_id();
			if (copy_from_user(&qsound_data, (void __user *)arg, sizeof(qsound_data))) {
				ret = -1;
				break;
			}

			if (!is_valid_bool(qsound_data)){
				pr_err(" invalid eq enable flag: %d\n", qsound_data);
				return -EINVAL;
			}

			//printk("%s: PANTECH_AUDIO_HEADSET_DEFAULT_CTL (val=%d)\n", __func__, qsound_data);
			
			ret = q6asm_set_mqfx_param(session_id, QSOUND_EQ_MODULE_ID, QSOUND_EQ_ENABLE, &qsound_data, sizeof(qsound_data));
			if (ret < 0) {
				pr_err(" failed eq module enable\n");
				break;
			}

			ret = q6asm_set_mqfx_param(session_id, QSOUND_EQ_MODULE_ID, QSOUND_EQ_BAND_LEVELS,
				eq_lvl_hs_default, sizeof(eq_lvl_hs_default));
			if (ret < 0) {
				pr_err(" failed eq level\n");
				break;
			}

			/*  --commented out. The reason of setting band level twice is not clear to me. -VB
			if (session_id != non_lpa_sessionid){
				if ((get_lpa_active() == 1) && ((non_lpa_sessionid > 0) && (non_lpa_sessionid < 0x8))){
					if (dai_mm != 0){ // it is not matched dai mm MULTIMEDIA1
						return 0;
					}
					ret = q6asm_set_mqfx_param(session_id, QSOUND_EQ_MODULE_ID, QSOUND_EQ_BAND_LEVELS,
						eq_lvl_hs_default, sizeof(eq_lvl_hs_default));
					if (ret < 0) {
						pr_err(" failed extreme volume module enable\n");
						break;
					}
				}
			}
			*/
			break;
		}
		case PANTECH_AUDIO_EQ_LVL_CTL: {
			//printk("%s: PANTECH_AUDIO_EQ_LVL_CTL\n", __func__);
			if (lpa_on == 1){
				if (get_lpa_active() == 0){
					printk("@#@#PANTECH_AUDIO_EQ_LVL_CTL......lpa_active not ready !!!!!\n");
					return -EINVAL;
				}
			}

			for (i=0; i<NUM_TRACKS;i++){
				struct track_effect_t* track = &track_effect[i];
				if (track->session_id != -1 && mCur_match_session_id == track->session_id && !track->using_eq_preset) {
					for (j=0; j<NUM_EQ_BANDS;j++){
						int16_t eq_level = track->eq_level[j];
						if (eq_level < -EQ_LVL_MAX || eq_level > EQ_LVL_MAX) {
							pr_err(" invalid eq_level[%d] : %d\n", j, eq_level);
							return -EINVAL;
						}
					}

					ret = q6asm_set_mqfx_param(session_id, QSOUND_EQ_MODULE_ID, QSOUND_EQ_BAND_LEVELS,
							track->eq_level, sizeof(track->eq_level));
					if (ret < 0) {
						pr_err(" failed eq level\n");
						break;
					}
					break;
				}
			}

			break;
		}
/*		case PANTECH_AUDIO_GET_BAND_NUM_CTL: {
//			printk("[SKY SND] PANTECH_AUDIO_GET_BAND_NUM_CTL, session_id : %d\n",  session_id);

			ret = q6asm_qsound_eq_get_value_dsp(session_id, QSOUND_EQ_MODULE_ID, QSOUND_EQ_GET_BAND_NUM,&qsound_get_data);
			if (ret < 0) {
				pr_err(" failed eq get band no\n");
				break;
			}

			if (copy_to_user((void __user *)arg, (uint32_t *)&qsound_get_data, sizeof(arg))) {
				pr_err("%s: Copy to user n",
					__func__);
				ret = -EFAULT;
				break;
			}
//			printk("[SKY SND] PANTECH_AUDIO_GET_BAND_NUM_CTL, qsound_get_data : %d\n",  qsound_get_data);
			break;
		}
		case PANTECH_AUDIO_GET_LVL_RANG_MIN_CTL: {
//			printk("[SKY SND] PANTECH_AUDIO_GET_LVL_RANG_MIN_CTL, session_id : %d\n",  session_id);

			ret = q6asm_qsound_eq_get_lvl_range_dsp(session_id, &qsound_eq_get_lvl_range_min, &qsound_eq_get_lvl_range_max);
			if (ret < 0) {
				pr_err(" failed eq get level range \n");
				break;
			}

			if (copy_to_user((void __user *)arg, &qsound_eq_get_lvl_range_min, sizeof(arg))) {
				pr_err("%s: Copy to user n",
					__func__);
				ret = -EFAULT;
				break;
			}
//			printk("[SKY SND] PANTECH_AUDIO_GET_LVL_RANG_MIN_CTL, qsound_eq_get_lvl_range_min : %d\n",  qsound_eq_get_lvl_range_min);
			break;
		}*/
		case PANTECH_AUDIO_GET_LVL_RANG_MAX_CTL: {
			int16_t range[2];

			printk("%s: PANTECH_AUDIO_GET_LVL_RANG_MAX_CTL\n", __func__);

			ret = q6asm_get_mqfx_param(session_id, QSOUND_EQ_MODULE_ID, QSOUND_EQ_LEVEL_RANGE,
				range, sizeof(range));
			if (ret < 0) {
				pr_err(" failed eq get level range max\n");
				break;
			}

			if (copy_to_user((void __user *)arg, &range[1], sizeof(int16_t))) {
				pr_err("%s: Copy to user n",
					__func__);
				ret = -EFAULT;
				break;
			}
			break;
		}
		case PANTECH_AUDIO_GET_CENTER_FREQ_BAND_CTL: {
			int16_t band_num;
			if (copy_from_user(&qsound_data, (void __user *)arg, sizeof(qsound_data))) {
				ret = -1;
				break;
			}
			printk("%s: PANTECH_AUDIO_GET_CENTER_FREQ_BAND_CTL (band=%d)\n", __func__, qsound_data);
			band_num = (int16_t)qsound_data;
			ret = q6asm_set_mqfx_param(session_id, QSOUND_EQ_MODULE_ID, QSOUND_EQ_CURRENT_BAND,
				&band_num, sizeof(band_num));
			break;
		}
		case PANTECH_AUDIO_GET_CENTER_FREQ_CTL: {
			uint32_t center;

			ret = q6asm_get_mqfx_param(session_id, QSOUND_EQ_MODULE_ID, QSOUND_EQ_CENTER_FREQ,
				&center, sizeof(center));
			if (ret < 0) {
				pr_err(" failed eq get center freq\n");
				break;
			}
			if (copy_to_user((void __user *)arg, (uint32_t *)&center, sizeof(center))) {
				pr_err("%s: Copy to user n",
					__func__);
				ret = -EFAULT;
				break;
			}
			printk("%s: PANTECH_AUDIO_GET_CENTER_FREQ_CTL (freq=%d)\n", __func__, center);
			break;
		}
		case PANTECH_AUDIO_GET_FREQ_RANGE_BAND_CTL: {
			int16_t band_num;
			if (copy_from_user(&qsound_data, (void __user *)arg, sizeof(qsound_data))) {
				ret = -1;
				break;
			}
			printk("%s: PANTECH_AUDIO_GET_FREQ_RANGE_BAND_CTL (band=%d)\n", __func__, qsound_data);
			band_num = (int16_t)qsound_data;
			ret = q6asm_set_mqfx_param(session_id, QSOUND_EQ_MODULE_ID, QSOUND_EQ_CURRENT_BAND,
				&band_num, sizeof(band_num));
			break;
		}
		case PANTECH_AUDIO_GET_FREQ_RANGE_CTL: {
			uint32_t range[2];

			ret = q6asm_get_mqfx_param(session_id, QSOUND_EQ_MODULE_ID, QSOUND_EQ_FREQ_RANGE,
				range, sizeof(range));
			if (ret < 0) {
				pr_err(" failed eq get freq range\n");
				break;
			}

			printk("%s: PANTECH_AUDIO_GET_FREQ_RANGE_CTL (%d - %d)\n", __func__, range[0], range[1]);

			if (copy_to_user((void __user *)arg, range, sizeof(arg))) {
				pr_err("%s: Copy to user n",
					__func__);
				ret = -EFAULT;
				break;
			}
			break;
		}

		case PANTECH_AUDIO_GET_BAND_FREQ_CTL: {
			int16_t band_num;
			if (copy_from_user(&qsound_data, (void __user *)arg, sizeof(qsound_data))) {
				ret = -1;
				break;
			}
			printk("%s: PANTECH_AUDIO_GET_BAND_FREQ_CTL (band=%d)\n", __func__, qsound_data);
			band_num = (int16_t)qsound_data;
			ret = q6asm_set_mqfx_param(session_id, QSOUND_EQ_MODULE_ID, QSOUND_EQ_CURRENT_BAND,
				&band_num, sizeof(band_num));
			break;
		}
		case PANTECH_AUDIO_GET_BAND_CTL: {
#if 0		// not implemtend
			printk("pantech_audio_ioctl: PANTECH_AUDIO_GET_BAND_CTL\n");

			ret = q6asm_get_mqfx_param(session_id, QSOUND_EQ_MODULE_ID, QSOUND_EQ_BAND_LEVEL,
				&qsound_get_data);
			if (ret < 0) {
				pr_err(" failed eq get band \n");
				break;
			}

			if (copy_to_user((void __user *)arg, (uint32_t *)&qsound_get_data, sizeof(arg))) {
				pr_err("%s: Copy to user n",
					__func__);
				ret = -EFAULT;
				break;
			}
#endif
			break;
		}
		case PANTECH_AUDIO_GET_PRESET_NUM_CTL: {
			int16_t num_presets = 0;
			ret = q6asm_get_mqfx_param(session_id, QSOUND_EQ_MODULE_ID, QSOUND_EQ_NUM_PRESETS,
				&num_presets, sizeof(num_presets));
			if (ret < 0) {
				pr_err(" failed eq get preset number no\n");
				break;
			}

			printk("%s: PANTECH_AUDIO_GET_PRESET_NUM_CTL (num presets=%d)\n", __func__, num_presets);
			
			if (copy_to_user((void __user *)arg, (uint32_t *)&num_presets, sizeof(num_presets))) {
				pr_err("%s: Copy to user n",
					__func__);
				ret = -EFAULT;
				break;
			}
			break;
		}
		case PANTECH_AUDIO_GET_PRESET_CTL: {
			if (copy_from_user(&qsound_data, (void __user *)arg, sizeof(qsound_data))) {
				ret = -1;
				break;
			}
			printk("%s: PANTECH_AUDIO_GET_PRESET_CTL (preset=%d)\n", __func__, qsound_data);
			ret = q6asm_set_mqfx_param(session_id, QSOUND_EQ_MODULE_ID, QSOUND_EQ_CUR_PRESET,
				&qsound_data, sizeof(qsound_data));
			break;
		}
		case PANTECH_AUDIO_GET_PRESET_NAME_CTL: {
			enum { MAX_NAME = 64 };
			char name[MAX_NAME];
			ret = q6asm_get_mqfx_param(session_id, QSOUND_EQ_MODULE_ID, QSOUND_EQ_PRESET_NAME,
					name, MAX_NAME);
			if (ret < 0) {
				pr_err(" failed eq get preset name no\n");
				break;
			}

			printk("%s: PANTECH_AUDIO_GET_PRESET_NAME_CTL (name=%s)\n", __func__, name);
			
			if (copy_to_user((void __user *)arg, (uint32_t *)name, sizeof(arg))) {
				pr_err("%s: Copy to user n",
					__func__);
				ret = -EFAULT;
				break;
			}
			break;
		}
		case PANTECH_AUDIO_BASS_BOOST_MODE_CTL: {
			if (copy_from_user(&qsound_data, (void __user *)arg, sizeof(qsound_data))) {
				ret = -1;
				break;
			}

			if (lpa_on == 1){
				if (get_lpa_active() == 0){
					printk("@#@#PANTECH_AUDIO_BASS_BOOST_MODE_CTL......lpa_active not ready !!!!!\n");
					return -EINVAL;
				}
			}

			if (!is_valid_bool(qsound_data)) {
				pr_err(" invalid bass boost enable flag: %d\n", qsound_data);
				return -EINVAL;
			}

			//printk("%s: PANTECH_AUDIO_BASS_BOOST_MODE_CTL (enable=%d)\n", __func__, qsound_data);

			ret = q6asm_set_mqfx_param(session_id, QSOUND_BASSBOOST_MODULE_ID, QSOUND_BASSBOOST_ENABLE_ID,
				&qsound_data, sizeof(qsound_data));
			if (ret < 0) {
				pr_err(" failed bass boost module enable\n");
				break;
			}
			break;
		}
		case PANTECH_AUDIO_BASS_BOOST_VALUE_CTL: {
			if (lpa_on == 1){
				if (get_lpa_active() == 0){
					printk("@#@#PANTECH_AUDIO_BASS_BOOST_VALUE_CTL......lpa_active not ready !!!!!\n");
					return -EINVAL;
				}
			}

			for (i=0; i<NUM_TRACKS;i++){
				if ((track_effect[i].session_id != -1) && (mCur_match_session_id == track_effect[i].session_id)){
					qsound_data = track_effect[i].bassboost_val;
					break;
				}
			}
			if (i == NUM_TRACKS) {
			    printk("%s: PANTECH_AUDIO_BASS_BOOST_VALUE_CTL - no tracks are matching session id=%d\n", __func__, mCur_match_session_id);
			    return -EINVAL;
			}

			//printk("%s: PANTECH_AUDIO_BASS_BOOST_VALUE_CTL (strength=%d)\n", __func__, qsound_data);

			if (qsound_data > BASSBOOST_VALUE_MAX) {
				pr_err(" invalid bassboost strength: %d\n", qsound_data);
				return -EINVAL;
			}

			ret = q6asm_set_mqfx_param(session_id, QSOUND_BASSBOOST_MODULE_ID, QSOUND_BASSBOOST_STRENGTH_ID,
					&qsound_data, sizeof(qsound_data));
			if (ret < 0) {
				pr_err(" failed bassboost strength\n");
				break;
			}
			break;
		}
		case PANTECH_AUDIO_EXCITER_MODE_CTL: {
			if (copy_from_user(&qsound_data, (void __user *)arg, sizeof(qsound_data))) {
				ret = -1;
				break;
			}

			if (lpa_on == 1){
				if (get_lpa_active() == 0){
					printk("@#@#PANTECH_AUDIO_EXCITER_MODE_CTL......lpa_active not ready !!!!!\n");
					return -EINVAL;
				}
			}

			if (!is_valid_bool(qsound_data)) {
				pr_err(" invalid exciter enable flag: %d\n", qsound_data);
				return -EINVAL;
			}

			//printk("%s: PANTECH_AUDIO_EXCITER_MODE_CTL (enable=%d)\n", __func__, qsound_data);

			ret = q6asm_set_mqfx_param(session_id, QSOUND_EXCITER_MODULE_ID, QSOUND_EXCITER_ENABLE_ID,
				&qsound_data, sizeof(qsound_data));
			if (ret < 0) {
				pr_err(" failed exciter module enable\n");
				break;
			}
			break;
		}
		case PANTECH_AUDIO_EXCITER_VALUE_CTL: {
			if (lpa_on == 1){
				if (get_lpa_active() == 0){
					printk("@#@#PANTECH_AUDIO_EXCITER_VALUE_CTL......lpa_active not ready !!!!!\n");
					return -EINVAL;
				}
			}

			for (i=0; i<NUM_TRACKS;i++){
				if ((track_effect[i].session_id != -1) && (mCur_match_session_id == track_effect[i].session_id)){
					qsound_data = track_effect[i].exciter_val;
					break;
				}
			}
			if (i == NUM_TRACKS) {
			    printk("%s: PANTECH_AUDIO_EXCITER_VALUE_CTL - no tracks are matching session id=%d\n", __func__, mCur_match_session_id);
			    return -EINVAL;
			}

			//printk("%s: PANTECH_AUDIO_EXCITER_VALUE_CTL (strength=%d)\n", __func__, qsound_data);

			if (qsound_data > EXCITER_VALUE_MAX) {
				pr_err(" invalid exciter strength: %d\n", qsound_data);
				return -EINVAL;
			}

			ret = q6asm_set_mqfx_param(session_id, QSOUND_EXCITER_MODULE_ID, QSOUND_EXCITER_STRENGTH_ID,
					&qsound_data, sizeof(qsound_data));
			if (ret < 0) {
				pr_err(" failed exciter strength\n");
				break;
			}
			break;
		}
/*		case PANTECH_AUDIO_GET_BASS_BOOST_VALUE_CTL: {
//			printk("[SKY SND] PANTECH_AUDIO_GET_BASS_BOOST_VALUE_CTL, session_id : %d\n",  session_id);

			ret = q6asm_qsound_eq_get_value_dsp(session_id, QSOUND_BASSBOOST_MODULE_ID, QSOUND_BASSBOOST_STRENGTH_ID,&qsound_get_data);
			if (ret < 0) {
				pr_err(" failed bassboost get strength\n");
				break;
			}

			if (copy_to_user((void __user *)arg, (uint32_t *)&qsound_get_data, sizeof(arg))) {
				pr_err("%s: Copy to user n",
					__func__);
				ret = -EFAULT;
				break;
			}
//			printk("[SKY SND] PANTECH_AUDIO_GET_PRESET_NUM_CTL, qsound_get_data : %d\n",  qsound_get_data);
			break;
		}	*/
		case PANTECH_AUDIO_VIRTUAL_MODE_CTL: {
			if (copy_from_user(&qsound_data, (void __user *)arg, sizeof(qsound_data))) {
				ret = -1;
				break;
			}

			if (lpa_on == 1){
				if (get_lpa_active() == 0){
					printk("@#@#PANTECH_AUDIO_VIRTUAL_MODE_CTL......lpa_active not ready !!!!!\n");
					return -EINVAL;
				}
			}

			if (!is_valid_bool(qsound_data)){
				pr_err(" invalid virtualizer enable flag: %d\n", qsound_data);
				return -EINVAL;
			}

			//printk("%s: PANTECH_AUDIO_VIRTUAL_MODE_CTL (enabled=%d)\n", __func__, qsound_data);
			
			ret = q6asm_set_mqfx_param(session_id, QSOUND_VIRTUAL_MODULE_ID, QSOUND_VIRTUAL_ENABLE_ID,
				&qsound_data, sizeof(qsound_data));
			if (ret < 0) {
				pr_err(" failed virtual module enable\n");
				break;
			}
			break;
		}
		case PANTECH_AUDIO_VIRTUAL_VALUE_CTL: {
			//printk("pantech_audio_ioctl: PANTECH_AUDIO_VIRTUAL_VALUE_CTL\n");
			if (lpa_on == 1){
				if (get_lpa_active() == 0){
					printk("@#@#PANTECH_AUDIO_VIRTUAL_VALUE_CTL......lpa_active not ready !!!!!\n");
					return -EINVAL;
				}
			}

			for (i=0; i<NUM_TRACKS;i++){
				if ((track_effect[i].session_id != -1) && (mCur_match_session_id == track_effect[i].session_id)){
					qsound_data = track_effect[i].virtualizer_val;
//					printk("PANTECH_AUDIO_VIRTUAL_VALUE_CTL ....track_effect[%d].virtualizer_val : %d......track_effect[i].session_id : %d\n", i,track_effect[i].virtualizer_val,track_effect[i].session_id);
					break;
				}
			}
			if (i == NUM_TRACKS) {
			    printk("%s: PANTECH_AUDIO_VIRTUAL_VALUE_CTL - no tracks are matching session id=%d\n", __func__, mCur_match_session_id);
			    return -EINVAL;
			}

//			printk("%s: PANTECH_AUDIO_VIRTUAL_VALUE_CTL (spread=%d)......mCur_match_session_id : %d\n", __func__, qsound_data,mCur_match_session_id);
			
			if (qsound_data > VIRTUALIZER_VALUE_MAX) {
				pr_err(" invalid virtualizer spread: %d\n", qsound_data);
				return -EINVAL;
			}

			ret = q6asm_set_mqfx_param(session_id, QSOUND_VIRTUAL_MODULE_ID, QSOUND_VIRTUAL_SPREAD_ID,
				&qsound_data, sizeof(qsound_data));
			if (ret < 0) {
				pr_err(" failed virtual value\n");
				break;
			}
			break;
		}
/*		case PANTECH_AUDIO_GET_VIRTUAL_VALUE_CTL: {
//			printk("[SKY SND] PANTECH_AUDIO_GET_VIRTUAL_VALUE_CTL, session_id : %d\n",  session_id);

			ret = q6asm_qsound_eq_get_value_dsp(session_id, QSOUND_VIRTUAL_MODULE_ID, QSOUND_VIRTUAL_SPREAD_ID,&qsound_get_data);
			if (ret < 0) {
				pr_err(" failed virtualizer get strength\n");
				break;
			}

			if (copy_to_user((void __user *)arg, (uint32_t *)&qsound_get_data, sizeof(arg))) {
				pr_err("%s: Copy to user n",
					__func__);
				ret = -EFAULT;
				break;
			}
//			printk("[SKY SND] PANTECH_AUDIO_GET_VIRTUAL_VALUE_CTL, qsound_get_data : %d\n",  qsound_get_data);
			break;
		}	*/

		case PANTECH_AUDIO_PRESET_REVERB_CTL: {
			if (copy_from_user(&qsound_data, (void __user *)arg, sizeof(qsound_data))) {
				ret = -1;
				break;
			}
			
			if (qsound_data != 0){
				for (i=0; i<NUM_TRACKS;i++){
					if ((track_effect[i].session_id != -1) && (mCur_match_session_id == track_effect[i].session_id)){
						qsound_data = track_effect[i].reverb_val;
						break;
					}
				}
			}

			if (lpa_on == 1){
				if (get_lpa_active() == 0){
					printk("@#@#PANTECH_AUDIO_PRESET_REVERB_CTL......lpa_active not ready !!!!!\n");
					return -EINVAL;
				}
			}

			if (qsound_data > REVERB_PRESET_MAX){
				pr_err(" invalid reverb preset: %d\n", qsound_data);
				return -EINVAL;
			}

			//printk("%s: PANTECH_AUDIO_PRESET_REVERB_CTL (preset=%d)\n", __func__, qsound_data);

			ret = q6asm_set_mqfx_param(session_id, QSOUND_REVERB_MODULE_ID, QSOUND_REVERB_PRESET_ID,
					&qsound_data, sizeof(qsound_data));
			if (ret < 0) {
				pr_err(" failed reverb preset \n");
				break;
			}
			break;
		}
/*		case PANTECH_AUDIO_GET_PRESET_REVERB_CTL: {
//			printk("[SKY SND] PANTECH_AUDIO_GET_PRESET_REVERB_CTL, session_id : %d\n",  session_id);

			ret = q6asm_qsound_eq_get_value_dsp(session_id, QSOUND_REVERB_MODULE_ID, QSOUND_REVERB_PRESET_ID,&qsound_get_data);
			if (ret < 0) {
				pr_err(" failed preset reverb get preset\n");
				break;
			}

			if (copy_to_user((void __user *)arg, (uint32_t *)&qsound_get_data, sizeof(arg))) {
				pr_err("%s: Copy to user n",
					__func__);
				ret = -EFAULT;
				break;
			}
//			printk("[SKY SND] PANTECH_AUDIO_GET_PRESET_REVERB_CTL, qsound_get_data : %d\n",  qsound_get_data);
			break;
		}	*/
		case PANTECH_AUDIO_LPA_EXTREME_VOL_CTL:{
			int lpa_sessionid = get_aud_lpa_session_id();
			if (copy_from_user(&qsound_data, (void __user *)arg, sizeof(qsound_data))) {
				ret = -1;
				break;
			}
			if (!is_valid_bool(qsound_data)){
				pr_err(" invalid extreme volume enable flag: %d\n", qsound_data);
				return -EINVAL;
			}
			//printk("%s: PANTECH_AUDIO_LPA_EXTREME_VOL_CTL (enable=%d)\n", __func__, qsound_data);

			ret = q6asm_set_mqfx_param(lpa_sessionid, QSOUND_EXTREME_VOL_MODULE_ID, QSOUND_EXTREME_VOL_ENABLE_ID,
				&qsound_data, sizeof(qsound_data));
			if (ret < 0) {
				pr_err(" failed lpa extreme volume module enable\n");
				break;
			}
			break;
		}

		case PANTECH_AUDIO_EXTREME_VOL_CTL: {
			int lpa_sessionid = get_aud_lpa_session_id();
			int non_lpa_sessionid = get_aud_non_lpa_session_id();

			if (lpa_sessionid == non_lpa_sessionid) {
				pr_err(" not ready for non-LPA music\n");
				return -EINVAL;  //not ready for non-lpa music
			}

			if (copy_from_user(&qsound_data, (void __user *)arg, sizeof(qsound_data))) {
				ret = -1;
				break;
			}
			if (!is_valid_bool(qsound_data)){
				pr_err(" invalid extreme volume enable flag: %d\n", qsound_data);
				return -EINVAL;
			}
			//printk("%s: PANTECH_AUDIO_EXTREME_VOL_CTL (enable=%d)\n", __func__, qsound_data);

			ret = q6asm_set_mqfx_param(non_lpa_sessionid, QSOUND_EXTREME_VOL_MODULE_ID, QSOUND_EXTREME_VOL_ENABLE_ID,
				&qsound_data, sizeof(qsound_data));
			if (ret < 0) {
				pr_err(" failed non-lpa extreme volume module enable\n");
				break;
			}

			break;
		}
		case PANTECH_AUDIO_GET_EXTREME_VOL_CTL: {
			uint16_t enabled = 0;

			ret = q6asm_get_mqfx_param(session_id, QSOUND_EXTREME_VOL_MODULE_ID, QSOUND_EXTREME_VOL_ENABLE_ID,
				&enabled, sizeof(enabled));
			if (ret < 0) {
				pr_err(" failed extreme get vol \n");
				break;
			}

			//printk("%s: PANTECH_AUDIO_GET_EXTREME_VOL_CTL (enabled=%d)\n", __func__, enabled);
			
			if (copy_to_user((void __user *)arg, (uint32_t *)&enabled, sizeof(enabled))) {
				pr_err("%s: Copy to user n",
					__func__);
				ret = -EFAULT;
				break;
			}
			break;
		}
		case PANTECH_AUDIO_LIMITTER_CTL: {
			if (copy_from_user(&qsound_data, (void __user *)arg, sizeof(qsound_data))) {
				ret = -1;
				break;
			}

			//printk("%s: PANTECH_AUDIO_GET_LIMITER_CTL (enable=%d)\n", __func__, qsound_data);
			
			if (limiter_module_enable != qsound_data){
				ret = q6asm_set_mqfx_param(session_id,QSOUND_LIMITER_MODULE_ID, QSOUND_LIMITER_ENABLE_ID,
					&qsound_data, sizeof(qsound_data));
				if (ret < 0) {
					pr_err(" failed limiter module enable\n");
					break;
				}
				limiter_module_enable = qsound_data;
			}
			break;
		}

		case PANTECH_AUDIO_QVSS_CTL: {
			//printk("pantech_audio_ioctl: PANTECH_AUDIO_QVSS_CTL\n");
			if (copy_from_user(&qsound_data, (void __user *)arg, sizeof(qsound_data))) {
				ret = -1;
				break;
			}

			//printk("[SKY SND] PANTECH_AUDIO_QVSS_CTL, data=%x, qvss_module_enable : %x, session_id : %d\n", qsound_data, qvss_module_enable, session_id);

#if 0
			// not implemented!!!
			if (qvss_module_enable != qsound_data){
				ret = q6asm_set_mqfx_param(session_id,QSOUND_QVSS_MODULE_ID, QSOUND_QVSS_ENABLE_ID,
					&qsound_data, sizeof(qsound_data));
				if (ret < 0) {
					pr_err(" failed QVSS module enable\n");
					break;
				}
				qvss_module_enable = qsound_data;
			}
#endif
			break;
		}
		case PANTECH_AUDIO_SET_SESSION_ID_CTL:
		case PANTECH_AUDIO_DESTROY_SESSION_ID_CTL:
		case PANTECH_AUDIO_EQ_KEEP_PRESET_CTL:
		case PANTECH_AUDIO_EQ_BAND_CTL:
		case PANTECH_AUDIO_EQ_KEEP_LVL_CTL:
		case PANTECH_AUDIO_BASS_BOOST_KEEP_VAL_CTL:
		case PANTECH_AUDIO_EXCITER_KEEP_VAL_CTL:
		case PANTECH_AUDIO_VIRTUAL_KEEP_VALUE_CTL:
		case PANTECH_AUDIO_KEEP_PRESET_REVERB_CTL:
			break;
		case PANTECH_AUDIO_INIT_ALL_DATA_CTL: {
			printk("%s: PANTECH_AUDIO_INIT_ALL_DATA_CTL\n", __func__);

			printk("\n[SKY SND] PANTECH_AUDIO_INIT_ALL_DATA_CTL................\n");

			for (i=0; i<NUM_TRACKS;i++){
				track_effect[i].session_id = -1;
				track_effect[i].using_eq_preset = false;
				track_effect[i].eq_preset = -1;
				for (j=0; j<NUM_EQ_BANDS;j++){
					track_effect[i].eq_level[j] = -1;
				}
				track_effect[i].bassboost_val = -1;
				track_effect[i].exciter_val = -1;
				track_effect[i].virtualizer_val = -1;
				track_effect[i].reverb_val = -1;
			}
			break;
		}
#endif  //CONFIG_PANTECH_SND_QSOUND

		default: {
			printk("%s: --------------- INVALID IOCTL code: 0x%08X ---------------\n", __func__, cmd);
			ret = -1;
			break;
		}
	}

	return ret;
}

/*==========================================================================
** pantech_audio_open
**=========================================================================*/

static int pantech_audio_open(struct inode *inode, struct file *file)
{
	//printk("pantech_aud_ctl.c: %s\n", __FUNCTION__);
	//printk("aud_sub_open");
	return 0;
}

/*==========================================================================
** pantech_audio_release
**=========================================================================*/

static int pantech_audio_release(struct inode *inode, struct file *file)
{
	//printk("aud_sub_release");
	return 0;
}

/*=========================================================================*/

static struct file_operations snd_fops = {
	//.owner = THIS_MODULE,
	.open = pantech_audio_open,
	.release = pantech_audio_release,
	.unlocked_ioctl	= pantech_audio_ioctl,
};

struct miscdevice pantech_audio_misc =
{
	.minor = MISC_DYNAMIC_MINOR,
	.name = "pantech_aud_ctl",
	.fops = &snd_fops
};

/*==========================================================================
** pantech_audio_init
**=========================================================================*/

static int __init pantech_audio_init(void)
{
	int result = 0;

	result = misc_register(&pantech_audio_misc);

	if (result)
	{
		printk("pantech_audio_init: misc_register failed\n");
	}

	return result;
}

/*==========================================================================
** pantech_audio_exit
**=========================================================================*/

static void __exit pantech_audio_exit(void)
{
}

/*=========================================================================*/
module_init(pantech_audio_init);

module_exit(pantech_audio_exit);

//MODULE_DESCRIPTION("Pantech audio driver");
//MODULE_LICENSE("GPL v2");

/*=========================================================================*/
