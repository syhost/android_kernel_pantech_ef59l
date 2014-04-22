/* drivers/sharp/shmhl/shmhl_kerl.c (SHARP MHL Driver)
 *
 * Copyright (c) 2011, Sharp. All rights reserved.
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

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/input.h>
#include <linux/workqueue.h>


#define SHMHL_DUMP_LOG(buf, size, print_size, args...)	printk(args)
#define SHMHL_DEBUG_LOG_H(args...)	printk(KERN_ERR "[SHMHL] " args)
#define SHMHL_DEBUG_LOG_L(args...)	printk(KERN_ERR "[SHMHL] " args)

#define SH_MHL_RCP_KEYCODE_MAX	0x7F
#define SH_MHL_RCP_RELEASE_BIT	0x80
#define SH_MHL_RCP_HOLD_MS		470 /* ms */

typedef enum
{
	SHMHL_RCP_STATE_NONE = 0,
	SHMHL_RCP_STATE_HOLD,
	SHMHL_RCP_STATE_MAX,
} shmhl_rcp_state_t;

typedef struct
{
	unsigned int		linux_key;
	bool				hold;
} shmhl_rcp_key_comvert_t;

typedef struct
{
	struct workqueue_struct		*shmhl_rcp_wq;
	struct delayed_work			shmhl_rcp_dwork;
	unsigned int				last_linux_key;
	shmhl_rcp_state_t			rcp_state;
} shmhl_rcp_work_t;

static struct input_dev*	shmhl_key = NULL;
static shmhl_rcp_work_t shmhl_rcp_work;
static struct mutex shmhl_rcp_lock;

static shmhl_rcp_key_comvert_t shmhl_rcp_key_comvert_tbl[SH_MHL_RCP_KEYCODE_MAX+1] =
{
	{ KEY_ENTER		,	false	},		/* 0x00 *//* Select */
	{ KEY_UP		,	true	},		/* 0x01 *//* Up */
	{ KEY_DOWN		,	true	},		/* 0x02 *//* Down */
	{ KEY_LEFT		,	true	},		/* 0x03 *//* Left */
	{ KEY_RIGHT		,	true	},		/* 0x04 *//* Right */
	{ KEY_RESERVED	,	true	},		/* 0x05 *//* Right-Up */
	{ KEY_RESERVED	,	true	},		/* 0x06 *//* Right-Down */
	{ KEY_RESERVED	,	true	},		/* 0x07 *//* Left-Up */
	{ KEY_RESERVED	,	true	},		/* 0x08 *//* Left-Down */
	{ KEY_HOMEPAGE		,	false	},		/* 0x09 *//* Root Menu */
	{ KEY_RESERVED	,	false	},		/* 0x0A *//* Setup Menu */
	{ KEY_RESERVED	,	false	},		/* 0x0B *//* Contents Menu */
	{ KEY_RESERVED	,	false	},		/* 0x0C *//* Favorite Menu */
	{ KEY_BACK		,	false	},		/* 0x0D *//* Exit */
	{ KEY_RESERVED	,	false	},		/* 0x0E *//* Reserved */
	{ KEY_RESERVED	,	false	},		/* 0x0F *//* Reserved */
	{ KEY_RESERVED	,	false	},		/* 0x10 *//* Reserved */
	{ KEY_RESERVED	,	false	},		/* 0x11 *//* Reserved */
	{ KEY_RESERVED	,	false	},		/* 0x12 *//* Reserved */
	{ KEY_RESERVED	,	false	},		/* 0x13 *//* Reserved */
	{ KEY_RESERVED	,	false	},		/* 0x14 *//* Reserved */
	{ KEY_RESERVED	,	false	},		/* 0x15 *//* Reserved */
	{ KEY_RESERVED	,	false	},		/* 0x16 *//* Reserved */
	{ KEY_RESERVED	,	false	},		/* 0x17 *//* Reserved */
	{ KEY_RESERVED	,	false	},		/* 0x18 *//* Reserved */
	{ KEY_RESERVED	,	false	},		/* 0x19 *//* Reserved */
	{ KEY_RESERVED	,	false	},		/* 0x1A *//* Reserved */
	{ KEY_RESERVED	,	false	},		/* 0x1B *//* Reserved */
	{ KEY_RESERVED	,	false	},		/* 0x1C *//* Reserved */
	{ KEY_RESERVED	,	false	},		/* 0x1D *//* Reserved */
	{ KEY_RESERVED	,	false	},		/* 0x1E *//* Reserved */
	{ KEY_RESERVED	,	false	},		/* 0x1F *//* Reserved */
#if 0
	{ KEY_0			,	false	},		/* 0x20 *//* Numeric 0 */
	{ KEY_1			,	false	},		/* 0x21 *//* Numeric 1 */
	{ KEY_2			,	false	},		/* 0x22 *//* Numeric 2 */
	{ KEY_3			,	false	},		/* 0x23 *//* Numeric 3 */
	{ KEY_4			,	false	},		/* 0x24 *//* Numeric 4 */
	{ KEY_5			,	false	},		/* 0x25 *//* Numeric 5 */
	{ KEY_6			,	false	},		/* 0x26 *//* Numeric 6 */
	{ KEY_7			,	false	},		/* 0x27 *//* Numeric 7 */
	{ KEY_8			,	false	},		/* 0x28 *//* Numeric 8 */
	{ KEY_9			,	false	},		/* 0x29 *//* Numeric 9 */
#else
	{ KEY_RESERVED		,	false	},		/* 0x20 *//* Numeric 0 */
	{ KEY_RESERVED		,	false	},		/* 0x21 *//* Numeric 1 */
	{ KEY_RESERVED		,	false	},		/* 0x22 *//* Numeric 2 */
	{ KEY_RESERVED		,	false	},		/* 0x23 *//* Numeric 3 */
	{ KEY_RESERVED		,	false	},		/* 0x24 *//* Numeric 4 */
	{ KEY_RESERVED		,	false	},		/* 0x25 *//* Numeric 5 */
	{ KEY_RESERVED		,	false	},		/* 0x26 *//* Numeric 6 */
	{ KEY_RESERVED		,	false	},		/* 0x27 *//* Numeric 7 */
	{ KEY_RESERVED		,	false	},		/* 0x28 *//* Numeric 8 */
	{ KEY_RESERVED		,	false	},		/* 0x29 *//* Numeric 9 */
#endif
	{ KEY_RESERVED	,	false	},		/* 0x2A *//* Dot */
#if 0
	{ KEY_ENTER		,	false	},		/* 0x2B *//* Enter */
	{ KEY_BACK		,	false	},		/* 0x2C *//* Clear */
#else
	{ KEY_RESERVED		,	false	},		/* 0x2B *//* Enter */
	{ KEY_RESERVED		,	false	},		/* 0x2C *//* Clear */
#endif
	{ KEY_RESERVED	,	false	},		/* 0x2D *//* Reserved */
	{ KEY_RESERVED	,	false	},		/* 0x2E *//* Reserved */
	{ KEY_RESERVED	,	false	},		/* 0x2F *//* Reserved */
	{ KEY_RESERVED	,	true	},		/* 0x30 *//* Channel Up */
	{ KEY_RESERVED	,	true	},		/* 0x31 *//* Channel Down */
	{ KEY_RESERVED	,	false	},		/* 0x32 *//* Previous Channel */
	{ KEY_RESERVED	,	false	},		/* 0x33 *//* Sound Select */
	{ KEY_RESERVED	,	false	},		/* 0x34 *//* Input Select */
	{ KEY_RESERVED	,	false	},		/* 0x35 *//* Show Information */
	{ KEY_RESERVED	,	false	},		/* 0x36 *//* Help */
	{ KEY_RESERVED	,	true	},		/* 0x37 *//* Page Up */
	{ KEY_RESERVED	,	true	},		/* 0x38 *//* Page Down */
	{ KEY_RESERVED	,	false	},		/* 0x39 *//* Reserved */
	{ KEY_RESERVED	,	false	},		/* 0x3A *//* Reserved */
	{ KEY_RESERVED	,	false	},		/* 0x3B *//* Reserved */
	{ KEY_RESERVED	,	false	},		/* 0x3C *//* Reserved */
	{ KEY_RESERVED	,	false	},		/* 0x3D *//* Reserved */
	{ KEY_RESERVED	,	false	},		/* 0x3E *//* Reserved */
	{ KEY_RESERVED	,	false	},		/* 0x3F *//* Reserved */
	{ KEY_RESERVED	,	false	},		/* 0x40 *//* Reserved */
	{ KEY_RESERVED	,	true	},		/* 0x41 *//* Volume Up */
	{ KEY_RESERVED	,	true	},		/* 0x42 *//* Volume Down */
	{ KEY_RESERVED	,	false	},		/* 0x43 *//* Mute */
#if 0
	{ KEY_PLAY		,	false	},		/* 0x44 *//* Play */
	{ KEY_STOP		,	false	},		/* 0x45 *//* Stop */
	{ KEY_PAUSE		,	false	},		/* 0x46 *//* Pause */
#else
	{ KEY_RESERVED	,	false	},		/* 0x44 *//* Reserved */
	{ KEY_RESERVED	,	false	},		/* 0x45 *//* Reserved */
	{ KEY_RESERVED	,	false	},		/* 0x46 *//* Reserved */
#endif
	{ KEY_RESERVED	,	false	},		/* 0x47 *//* Record */
#if 0
	{ KEY_REWIND	,	true	},		/* 0x48 *//* Rewind */
	{ KEY_FASTFORWARD	,true	},		/* 0x49 *//* Fast Forward */
	{ KEY_RESERVED	,	false	},		/* 0x4A *//* Eject */
	{ KEY_NEXT		,	true	},		/* 0x4B *//* Forward */
	{ KEY_PREVIOUS	,	true	},		/* 0x4C *//* Backward */
#else
	{ KEY_RESERVED	,	false	},		/* 0x48 *//* Reserved */
	{ KEY_RESERVED	,	false	},		/* 0x49 *//* Reserved */
	{ KEY_RESERVED	,	false	},		/* 0x4A *//* Reserved */
	{ KEY_RESERVED	,	false	},		/* 0x4B *//* Record */
	{ KEY_RESERVED	,	false	},		/* 0x4C *//* Record */
#endif
	{ KEY_RESERVED	,	false	},		/* 0x4D *//* Reserved */
	{ KEY_RESERVED	,	false	},		/* 0x4E *//* Reserved */
	{ KEY_RESERVED	,	false	},		/* 0x4F *//* Reserved */
	{ KEY_RESERVED	,	false	},		/* 0x50 *//* Angle */
	{ KEY_RESERVED	,	false	},		/* 0x51 *//* Subpicture */
	{ KEY_RESERVED	,	false	},		/* 0x52 *//* Reserved */
	{ KEY_RESERVED	,	false	},		/* 0x53 *//* Reserved */
	{ KEY_RESERVED	,	false	},		/* 0x54 *//* Reserved */
	{ KEY_RESERVED	,	false	},		/* 0x55 *//* Reserved */
	{ KEY_RESERVED	,	false	},		/* 0x56 *//* Reserved */
	{ KEY_RESERVED	,	false	},		/* 0x57 *//* Reserved */
	{ KEY_RESERVED	,	false	},		/* 0x58 *//* Reserved */
	{ KEY_RESERVED	,	false	},		/* 0x59 *//* Reserved */
	{ KEY_RESERVED	,	false	},		/* 0x5A *//* Reserved */
	{ KEY_RESERVED	,	false	},		/* 0x5B *//* Reserved */
	{ KEY_RESERVED	,	false	},		/* 0x5C *//* Reserved */
	{ KEY_RESERVED	,	false	},		/* 0x5D *//* Reserved */
	{ KEY_RESERVED	,	false	},		/* 0x5E *//* Reserved */
	{ KEY_RESERVED	,	false	},		/* 0x5F *//* Reserved */
#if 0
	{ KEY_PLAY		,	false	},		/* 0x60 *//* Play_Function */
	{ KEY_PLAYPAUSE	,	false	},		/* 0x61 *//* Play_Pause_Function */
#else
	{ KEY_RESERVED	,	false	},		/* 0x60 *//* Record_Function */
	{ KEY_RESERVED	,	false	},		/* 0x61 *//* Pause_Record_Function */
#endif
	{ KEY_RESERVED	,	false	},		/* 0x62 *//* Record_Function */
	{ KEY_RESERVED	,	false	},		/* 0x63 *//* Pause_Record_Function */
#if 0
	{ KEY_STOP		,	false	},		/* 0x64 *//* Stop_Function */
#else
	{ KEY_RESERVED	,	false	},		/* 0x63 *//* Pause_Record_Function */
#endif
	{ KEY_RESERVED	,	false	},		/* 0x65 *//* Mute_Function */
	{ KEY_RESERVED	,	false	},		/* 0x66 *//* Restore_Volume_Function */
	{ KEY_RESERVED	,	false	},		/* 0x67 *//* Tune_Function */
	{ KEY_RESERVED	,	false	},		/* 0x68 *//* Select_Media_Function */
	{ KEY_RESERVED	,	false	},		/* 0x69 *//* Reserved */
	{ KEY_RESERVED	,	false	},		/* 0x6A *//* Reserved */
	{ KEY_RESERVED	,	false	},		/* 0x6B *//* Reserved */
	{ KEY_RESERVED	,	false	},		/* 0x6C *//* Reserved */
	{ KEY_RESERVED	,	false	},		/* 0x6D *//* Reserved */
	{ KEY_RESERVED	,	false	},		/* 0x6E *//* Reserved */
	{ KEY_RESERVED	,	false	},		/* 0x6F *//* Reserved */
	{ KEY_RESERVED	,	false	},		/* 0x70 *//* Reserved */
#if 0
	{ KEY_BLUE		,	false	},		/* 0x71 *//* F1(Blue) */
	{ KEY_RED		,	false	},		/* 0x72 *//* F2(Red) */
	{ KEY_GREEN		,	false	},		/* 0x73 *//* F3(Green) */
	{ KEY_YELLOW	,	false	},		/* 0x74 *//* F4(Yellow) */
#else
	{ KEY_RESERVED	,	false	},		/* 0x71 *//* Reserved */
	{ KEY_RESERVED	,	false	},		/* 0x72 *//* Reserved */
	{ KEY_RESERVED	,	false	},		/* 0x73 *//* Reserved */
	{ KEY_RESERVED	,	false	},		/* 0x74 *//* Reserved */
#endif
	{ KEY_RESERVED	,	false	},		/* 0x75 *//* F5 */
	{ KEY_RESERVED	,	false	},		/* 0x76 *//* Reserved */
	{ KEY_RESERVED	,	false	},		/* 0x77 *//* Reserved */
	{ KEY_RESERVED	,	false	},		/* 0x78 *//* Reserved */
	{ KEY_RESERVED	,	false	},		/* 0x79 *//* Reserved */
	{ KEY_RESERVED	,	false	},		/* 0x7A *//* Reserved */
	{ KEY_RESERVED	,	false	},		/* 0x7B *//* Reserved */
	{ KEY_RESERVED	,	false	},		/* 0x7C *//* Reserved */
	{ KEY_RESERVED	,	false	},		/* 0x7D *//* Reserved */
	{ KEY_RESERVED	,	false	},		/* 0x7E *//* Vendor_Specific */
	{ KEY_RESERVED	,	false	},		/* 0x7F *//* Reserved */
};

static void shmhl_rcp_start_delayed_work(unsigned int linux_key);
static void shmhl_rcp_stop_delayed_work(void);
static void shmhl_rcp_set_state(shmhl_rcp_state_t new_state);
static shmhl_rcp_state_t shmhl_rcp_get_state(void);
static void shmhl_rcp_delay_handler(struct work_struct *poWork);

void shmhl_input_rcp_key(uint8_t rcp_data)
{
	bool press = true;
	uint8_t rcp_key = (rcp_data & 0x7F);
	uint16_t linux_key = shmhl_rcp_key_comvert_tbl[rcp_key].linux_key;
	bool hold = shmhl_rcp_key_comvert_tbl[rcp_key].hold;
	shmhl_rcp_state_t now_state = SHMHL_RCP_STATE_NONE;

	mutex_lock(&shmhl_rcp_lock);

	if( NULL == shmhl_key ) {
		SHMHL_DEBUG_LOG_H("key device not register!\n");
		mutex_unlock(&shmhl_rcp_lock);
		return;
	}

	if( (rcp_data & SH_MHL_RCP_RELEASE_BIT) == SH_MHL_RCP_RELEASE_BIT) {
		/* Key Release */
		press = 0;
	}

	SHMHL_DEBUG_LOG_L("shmhl_input_rcp_key() rcp_data[0x%x] press[%d] linux_key[%d(0x%x)] hold[%d]\n",
						rcp_data, press, linux_key, linux_key, hold);

	now_state = shmhl_rcp_get_state();

	switch(now_state) {
		case SHMHL_RCP_STATE_NONE:
			SHMHL_DEBUG_LOG_L("shmhl_input_rcp_key() STATE_NONE now\n");

			/* Press */
			if(press) {
				SHMHL_DEBUG_LOG_L("shmhl_input_rcp_key() Press\n");
				if(hold) {
					SHMHL_DEBUG_LOG_L("shmhl_input_rcp_key() Hold\n");
					input_report_key(shmhl_key, linux_key, 1);							/* Press new_key */
					input_sync(shmhl_key);
					shmhl_rcp_start_delayed_work(linux_key);							/* Start Timer for Release */
				}
				else {
					SHMHL_DEBUG_LOG_L("shmhl_input_rcp_key() AutoRelease\n");
					input_report_key(shmhl_key, linux_key, 1);							/* Press new_key */
					input_report_key(shmhl_key, linux_key, 0);							/* Release new_key */
					input_sync(shmhl_key);
				}
			}
			/* Release */
			else {
				SHMHL_DEBUG_LOG_L("shmhl_input_rcp_key() Release\n");
				if(hold) {
					SHMHL_DEBUG_LOG_L("shmhl_input_rcp_key() Hold\n");
					input_report_key(shmhl_key, linux_key, 0);							/* Release new_key */
					input_sync(shmhl_key);
				}
				else {
					SHMHL_DEBUG_LOG_L("shmhl_input_rcp_key() Ignore\n");
				}
			}
			break;

		case SHMHL_RCP_STATE_HOLD:
			SHMHL_DEBUG_LOG_L("shmhl_input_rcp_key() STATE_HOLD now\n");

			shmhl_rcp_stop_delayed_work();												/* Stop Timer */

			/* Press */
			if(press) {
				SHMHL_DEBUG_LOG_L("shmhl_input_rcp_key() Press\n");
				if(hold) {
					SHMHL_DEBUG_LOG_L("shmhl_input_rcp_key() Hold\n");
					if( shmhl_rcp_work.last_linux_key == linux_key ) {
						SHMHL_DEBUG_LOG_L("shmhl_input_rcp_key() SameKey ->Restart[%d(0x%x)]\n", linux_key, linux_key);
						shmhl_rcp_start_delayed_work(linux_key);						/* Restart Timer */
					}
					else {
						SHMHL_DEBUG_LOG_L("shmhl_input_rcp_key() DiffKey ->Release[%d(0x%x)] ->Start[%d(0x%x)]\n",
											shmhl_rcp_work.last_linux_key, shmhl_rcp_work.last_linux_key,
											linux_key, linux_key);
						input_report_key(shmhl_key, shmhl_rcp_work.last_linux_key, 0);	/* Release last_key */
						shmhl_rcp_work.last_linux_key = KEY_RESERVED;

						input_report_key(shmhl_key, linux_key, 1);						/* Press new_key */
    					input_sync(shmhl_key);
						shmhl_rcp_start_delayed_work(linux_key);						/* Start Timer for Release */
					}
				}
				else {
					SHMHL_DEBUG_LOG_L("shmhl_input_rcp_key() DiffKey ->Release[%d(0x%x)] ->Press and AutoRelease[%d(0x%x)]\n",
										shmhl_rcp_work.last_linux_key, shmhl_rcp_work.last_linux_key,
										linux_key, linux_key);
					input_report_key(shmhl_key, shmhl_rcp_work.last_linux_key, 0);		/* Release last_key */
					shmhl_rcp_work.last_linux_key = KEY_RESERVED;

					input_report_key(shmhl_key, linux_key, 1);							/* Press new_key */
					input_report_key(shmhl_key, linux_key, 0);							/* Release new_key */
					input_sync(shmhl_key);
				}
			}
			/* Release */
			else {
				SHMHL_DEBUG_LOG_L("shmhl_input_rcp_key() Release[0x%x]\n", shmhl_rcp_work.last_linux_key);
				input_report_key(shmhl_key, shmhl_rcp_work.last_linux_key, 0);			/* Release last_key */
				input_sync(shmhl_key);
				shmhl_rcp_work.last_linux_key = KEY_RESERVED;
			}
			break;
		default:
			SHMHL_DEBUG_LOG_H("shmhl_input_rcp_key() state err\n");
			break;
	}
	mutex_unlock(&shmhl_rcp_lock);
}

void shmhl_rcp_input_register_device(void)
{
	int i = 0;
	int ret = 0;

	SHMHL_DEBUG_LOG_L("%s: Start\n", __func__);

	mutex_lock(&shmhl_rcp_lock);

	if( NULL != shmhl_key ) {
		SHMHL_DEBUG_LOG_L("Already input_allocate_device error\n");
		mutex_unlock(&shmhl_rcp_lock);
		return;
	}

	shmhl_key = input_allocate_device();

	if (!shmhl_key) {
		SHMHL_DEBUG_LOG_H("%s line:%d \n", __func__, __LINE__);
		mutex_unlock(&shmhl_rcp_lock);
		return;
	}

	shmhl_key->name = "mhl_rcp";
	shmhl_key->id.vendor	= 0x0001;
	shmhl_key->id.product	= 1;
	shmhl_key->id.version	= 1;


	for(i=0; i<(SH_MHL_RCP_KEYCODE_MAX+1); i++) {
		if(KEY_RESERVED != shmhl_rcp_key_comvert_tbl[i].linux_key) {
			input_set_capability(shmhl_key, EV_KEY, shmhl_rcp_key_comvert_tbl[i].linux_key);
		}
	}

	ret = input_register_device(shmhl_key);
	if (ret) {
		SHMHL_DEBUG_LOG_H("%s line:%d \n", __func__, __LINE__);
		input_free_device(shmhl_key);
		shmhl_key = NULL;
		mutex_unlock(&shmhl_rcp_lock);
		return;
	}

	mutex_unlock(&shmhl_rcp_lock);

	SHMHL_DEBUG_LOG_L("%s: End\n", __func__);
}

void shmhl_rcp_input_unregister_device(void)
{
	SHMHL_DEBUG_LOG_L("%s: Start\n", __func__);

	mutex_lock(&shmhl_rcp_lock);

	if( NULL == shmhl_key ) {
		SHMHL_DEBUG_LOG_L("Already input_unregister_device error\n");
		mutex_unlock(&shmhl_rcp_lock);
		return;
	}

	if( SHMHL_RCP_STATE_HOLD ==  shmhl_rcp_get_state()) {
		SHMHL_DEBUG_LOG_L("shmhl_input_rcp_key() STATE_HOLD now\n");
		shmhl_rcp_stop_delayed_work();											/* Stop Timer */

		SHMHL_DEBUG_LOG_L("shmhl_input_rcp_key() Release[0x%x]\n", shmhl_rcp_work.last_linux_key);
		input_report_key(shmhl_key, shmhl_rcp_work.last_linux_key, 0);			/* Release last_key */
		input_sync(shmhl_key);
		shmhl_rcp_work.last_linux_key = KEY_RESERVED;
	}

	input_unregister_device(shmhl_key);

	shmhl_key = NULL;

	mutex_unlock(&shmhl_rcp_lock);

	SHMHL_DEBUG_LOG_L("%s: End\n", __func__);
	return;
}

static void shmhl_rcp_start_delayed_work(unsigned int linux_key)
{
	SHMHL_DEBUG_LOG_L("shmhl_rcp_start_delayed_work()\n");

	queue_delayed_work(shmhl_rcp_work.shmhl_rcp_wq, &shmhl_rcp_work.shmhl_rcp_dwork, msecs_to_jiffies(SH_MHL_RCP_HOLD_MS));
	shmhl_rcp_work.last_linux_key = linux_key;
	shmhl_rcp_set_state(SHMHL_RCP_STATE_HOLD);
}

static void shmhl_rcp_stop_delayed_work(void)
{
	SHMHL_DEBUG_LOG_L("shmhl_rcp_stop_delayed_work()\n");

	cancel_delayed_work(&shmhl_rcp_work.shmhl_rcp_dwork);
	shmhl_rcp_set_state(SHMHL_RCP_STATE_NONE);
}

static void shmhl_rcp_set_state(shmhl_rcp_state_t new_state)
{
	SHMHL_DEBUG_LOG_L("shmhl_rcp_set_state() [%d]-->[%d]\n", shmhl_rcp_work.rcp_state, new_state);
	shmhl_rcp_work.rcp_state = new_state;
}

static shmhl_rcp_state_t shmhl_rcp_get_state(void)
{
	return(shmhl_rcp_work.rcp_state);
}

static void shmhl_rcp_delay_handler(struct work_struct *poWork)
{
	mutex_lock(&shmhl_rcp_lock);

	if( NULL == shmhl_key ) {
		SHMHL_DEBUG_LOG_L("Already input_unregister_device error\n");
		mutex_unlock(&shmhl_rcp_lock);
		return;
	}

	if(shmhl_rcp_get_state() == SHMHL_RCP_STATE_HOLD) {
		SHMHL_DEBUG_LOG_L("shmhl_rcp_delay_handler() Release[%d(0x%x)]\n", shmhl_rcp_work.last_linux_key, shmhl_rcp_work.last_linux_key);

		shmhl_rcp_set_state(SHMHL_RCP_STATE_NONE);
		/* Release */
		input_report_key(shmhl_key, shmhl_rcp_work.last_linux_key, 0);
		input_sync(shmhl_key);
		shmhl_rcp_work.last_linux_key = KEY_RESERVED;
	}
	else {
		SHMHL_DEBUG_LOG_L("shmhl_rcp_delay_handler() Irregal State!\n");
	}

	mutex_unlock(&shmhl_rcp_lock);
}

ssize_t ShowRcpDevRegist(struct device *dev, struct device_attribute *attr, char *buf)
{
	mutex_lock(&shmhl_rcp_lock);

	if(NULL == buf) {
		SHMHL_DEBUG_LOG_H("ShowRcpDevRegist() Param Err!!\n");
		mutex_unlock(&shmhl_rcp_lock);
		return -EINVAL;
	}

	if(NULL == shmhl_key) {
		SHMHL_DEBUG_LOG_H("ShowRcpDevRegist() Not Resited\n");
		strncpy(buf, "0\n", 3);
	}
	else {
		SHMHL_DEBUG_LOG_H("ShowRcpDevRegist() Already Resited\n");
		strncpy(buf, "1\n", 3);
	}

	mutex_unlock(&shmhl_rcp_lock);

	SHMHL_DEBUG_LOG_L("ShowRcpDevRegist() End buf=[0x%X][0x%X][0x%X]\n", buf[0], buf[1], buf[2]);

	return 2;
}

ssize_t ProcRcpDevRegist(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	SHMHL_DEBUG_LOG_L("ProcRcpDevRegist() Start\n");

	if(NULL == buf) {
		SHMHL_DEBUG_LOG_H("ProcRcpDevRegist() Param Err!!\n");
		return -EINVAL;
	}

	/* Regist */
	if('1' == buf[0]) {
		SHMHL_DEBUG_LOG_L("ProcRcpDevRegist() Regist\n");
		shmhl_rcp_input_register_device();
	}
	/* UnRegist */
	else if('0' == buf[0]) {
		SHMHL_DEBUG_LOG_L("ProcRcpDevRegist() UnRegist\n");
		shmhl_rcp_input_unregister_device();
	}
	else {
		SHMHL_DEBUG_LOG_H("ProcRcpDevRegist() Param Err!! [0x%X]\n", buf[0]);
		return -EINVAL;
	}

	SHMHL_DEBUG_LOG_L("ProcRcpDevRegist() End\n");

	return count;
}

ssize_t ProcRcpInputKey(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	char work_str[3];
	unsigned long rcp_data = 0;
	int rc = 0;

	SHMHL_DEBUG_LOG_L("ProcRcpInputKey() Start\n");

	if(NULL == buf) {
		SHMHL_DEBUG_LOG_H("ProcRcpInputKey() Param Err!!\n");
		return -EINVAL;
	}

	if(NULL == shmhl_key) {
		SHMHL_DEBUG_LOG_H("ProcRcpInputKey() Not Registed Err!!\n");
		return -EINVAL;
	}

	memset(work_str, 0, sizeof(work_str));

	work_str[0] = buf[0];
	work_str[1] = buf[1];
	rc = strict_strtoul(&work_str[0], 16, &rcp_data);

	if(0 != rc) {
		SHMHL_DEBUG_LOG_H("ProcRcpInputKey() Param Err buf[0x%X][0x%X]!!\n", buf[0], buf[1]);
		return -EINVAL;
	}

	SHMHL_DEBUG_LOG_L("ProcRcpInputKey() rcp_data[0x%02X]\n", (uint8_t)rcp_data);

	shmhl_input_rcp_key((uint8_t)rcp_data);

	SHMHL_DEBUG_LOG_L("ProcRcpInputKey() End\n");

	return count;
}

bool shmhl_rcp_key_support_judge(uint8_t rcp_data)
{
	if( KEY_RESERVED == shmhl_rcp_key_comvert_tbl[rcp_data & 0x7F].linux_key ) {
		SHMHL_DEBUG_LOG_L("shmhl_rcp_key_support_judge() [Not Support]\n");
		return false;
	}
	else {
		SHMHL_DEBUG_LOG_L("shmhl_rcp_key_support_judge() [Support]\n");
		return true;
	}
}

void shmhl_init(void)
{
	memset(&shmhl_rcp_work, 0, sizeof(shmhl_rcp_work));
	shmhl_rcp_work.shmhl_rcp_wq = create_singlethread_workqueue("mhl_rcp");
	INIT_DELAYED_WORK(&shmhl_rcp_work.shmhl_rcp_dwork, shmhl_rcp_delay_handler);

	mutex_init(&shmhl_rcp_lock);
}

