/*
 * Copyright (c) 2011,2012 Synaptics Incorporated
 * Copyright (c) 2011 Unixphere
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#define FUNCTION_DATA f11_data
#define FNUM 11


#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/input.h>
#ifdef CONFIG_RMI4_F11_TYPEB
#include <linux/input/mt.h>
#endif
#include "rmi.h"
#include "rmi_driver.h"

#ifdef CONFIG_RMI4_DEBUG
#include <linux/debugfs.h>
#include <linux/fs.h>
/*#include <asm/uaccess.h> */
#include <linux/uaccess.h>
#endif

//#include "pantech_cfg.h"
#include <linux/gpio.h>
#include <mach/gpio.h>

#define RESUME_REZERO (1 && defined(CONFIG_PM))
#if RESUME_REZERO
#include <linux/delay.h>
#define DEFAULT_REZERO_WAIT_MS	40
#endif

#ifndef MT_TOOL_MAX
#define MT_TOOL_MAX MT_TOOL_PEN
#endif

#define F11_MAX_NUM_OF_SENSORS		8
#define F11_MAX_NUM_OF_FINGERS		10
#define F11_MAX_NUM_OF_TOUCH_SHAPES	16

#define F11_REL_POS_MIN		-128
#define F11_REL_POS_MAX		127

#define FINGER_STATE_MASK	0x03
#define GET_FINGER_STATE(f_states, i) \
	((f_states[i / 4] >> (2 * (i % 4))) & FINGER_STATE_MASK)

#define F11_CTRL_SENSOR_MAX_X_POS_OFFSET	6
#define F11_CTRL_SENSOR_MAX_Y_POS_OFFSET	8

#define F11_CEIL(x, y) (((x) + ((y)-1)) / (y))
#define INBOX(x, y, box) (x >= box.x && x < (box.x + box.width) \
			&& y >= box.y && y < (box.y + box.height))

#define DEFAULT_XY_MAX 9999
#define DEFAULT_MAX_ABS_MT_PRESSURE 255
#define DEFAULT_MAX_ABS_MT_TOUCH 15
#define DEFAULT_MAX_ABS_MT_ORIENTATION 1
#define DEFAULT_MIN_ABS_MT_TRACKING_ID 1
#define DEFAULT_MAX_ABS_MT_TRACKING_ID 10
#define MAX_NAME_LENGTH 256

/* Adding debugfs for flip, clip, offset and swap */
#ifdef CONFIG_RMI4_DEBUG

static int setup_debugfs(struct rmi_device *rmi_dev);
static void teardown_debugfs(struct rmi_device *rmi_dev);
#endif
/* End adding debugfs */

static ssize_t f11_relreport_show(struct device *dev,
					struct device_attribute *attr,
					char *buf);

static ssize_t f11_relreport_store(struct device *dev,
					 struct device_attribute *attr,
					 const char *buf, size_t count);

static ssize_t f11_maxPos_show(struct device *dev,
				     struct device_attribute *attr, char *buf);

static ssize_t f11_rezero_store(struct device *dev,
					 struct device_attribute *attr,
					 const char *buf, size_t count);

#if RESUME_REZERO
static ssize_t f11_rezeroOnResume_show(struct device *dev,
					struct device_attribute *attr,
					char *buf);

static ssize_t f11_rezeroOnResume_store(struct device *dev,
					 struct device_attribute *attr,
					 const char *buf, size_t count);
static ssize_t f11_rezeroWait_show(struct device *dev,
					struct device_attribute *attr,
					char *buf);

static ssize_t f11_rezeroWait_store(struct device *dev,
					 struct device_attribute *attr,
					 const char *buf, size_t count);
#endif

static void rmi_f11_free_memory(struct rmi_function_container *fc);

static int rmi_f11_initialize(struct rmi_function_container *fc);

static int rmi_f11_create_sysfs(struct rmi_function_container *fc);

static int rmi_f11_config(struct rmi_function_container *fc);

static int rmi_f11_reset(struct rmi_function_container *fc);

static int rmi_f11_register_devices(struct rmi_function_container *fc);

static void rmi_f11_free_devices(struct rmi_function_container *fc);

static void f11_set_abs_params(struct rmi_function_container *fc, int index);

void rmi_clear_finger(struct input_dev *input);

static struct device_attribute attrs[] = {
	__ATTR(relreport, RMI_RW_ATTR, f11_relreport_show, f11_relreport_store),
	__ATTR(maxPos, RMI_RO_ATTR, f11_maxPos_show, rmi_store_error),
#if RESUME_REZERO
	__ATTR(rezeroOnResume, RMI_RW_ATTR, f11_rezeroOnResume_show,
		f11_rezeroOnResume_store),
	__ATTR(rezeroWait, RMI_RW_ATTR, f11_rezeroWait_show,
		f11_rezeroWait_store),
#endif
	__ATTR(rezero, RMI_WO_ATTR, rmi_show_error, f11_rezero_store)
};


union f11_2d_commands {
	struct {
		u8 rezero:1;
	};
	u8 reg;
};

struct f11_2d_device_query {
	union {
		struct {
			u8 nbr_of_sensors:3;
			u8 has_query9:1;
			u8 has_query11:1;
		};
		u8 f11_2d_query0;
	};

	union {
		struct {
			u8 has_z_tuning:1;
			u8 has_pos_interpolation_tuning:1;
			u8 has_w_tuning:1;
			u8 has_pitch_info:1;
			u8 has_default_finger_width:1;
			u8 has_segmentation_aggressiveness:1;
			u8 has_tx_rw_clip:1;
			u8 has_drumming_correction:1;
		};
		u8 f11_2d_query11;
	};
};

union f11_2d_query9 {
	struct {
		u8 has_pen:1;
		u8 has_proximity:1;
		u8 has_palm_det_sensitivity:1;
		u8 has_suppress_on_palm_detect:1;
		u8 has_two_pen_thresholds:1;
		u8 has_contact_geometry:1;
	};
	u8 reg;
};

struct f11_2d_sensor_query {
	union {
		struct {
			/* query1 */
			u8 number_of_fingers:3;
			u8 has_rel:1;
			u8 has_abs:1;
			u8 has_gestures:1;
			u8 has_sensitivity_adjust:1;
			u8 configurable:1;
			/* query2 */
			u8 num_of_x_electrodes:7;
			/* query3 */
			u8 num_of_y_electrodes:7;
			/* query4 */
			u8 max_electrodes:7;
		};
		u8 f11_2d_query1__4[4];
	};

	union {
		struct {
			u8 abs_data_size:3;
			u8 has_anchored_finger:1;
			u8 has_adj_hyst:1;
			u8 has_dribble:1;
		};
		u8 f11_2d_query5;
	};

	u8 f11_2d_query6;

	union {
		struct {
			u8 has_single_tap:1;
			u8 has_tap_n_hold:1;
			u8 has_double_tap:1;
			u8 has_early_tap:1;
			u8 has_flick:1;
			u8 has_press:1;
			u8 has_pinch:1;
			u8 padding:1;

			u8 has_palm_det:1;
			u8 has_rotate:1;
			u8 has_touch_shapes:1;
			u8 has_scroll_zones:1;
			u8 has_individual_scroll_zones:1;
			u8 has_multi_finger_scroll:1;
		};
		u8 f11_2d_query7__8[2];
	};

	union f11_2d_query9 query9;

	union {
		struct {
			u8 nbr_touch_shapes:5;
		};
		u8 f11_2d_query10;
	};
};
union f11_2d_ctrl0_9 {
	struct {
		/* F11_2D_Ctrl0 */
		u8 reporting_mode:3;
		u8 abs_pos_filt:1;
		u8 rel_pos_filt:1;
		u8 rel_ballistics:1;
		u8 dribble:1;
		u8 report_beyond_clip:1;
		/* F11_2D_Ctrl1 */
		u8 palm_detect_thres:4;
		u8 motion_sensitivity:2;
		u8 man_track_en:1;
		u8 man_tracked_finger:1;
		/* F11_2D_Ctrl2 and 3 */
		u8 delta_x_threshold:8;
		u8 delta_y_threshold:8;
		/* F11_2D_Ctrl4 and 5 */
		u8 velocity:8;
		u8 acceleration:8;
		/* F11_2D_Ctrl6 thru 9 */
		u16 sensor_max_x_pos:12;
		u8 ctrl7_reserved:4;
		u16 sensor_max_y_pos:12;
		u8 ctrl9_reserved:4;
	};
	struct {
		u8 regs[10];
		u16 address;
	};
};
union f11_2d_ctrl10 {
	struct {
		u8 single_tap_int_enable:1;
		u8 tap_n_hold_int_enable:1;
		u8 double_tap_int_enable:1;
		u8 early_tap_int_enable:1;
		u8 flick_int_enable:1;
		u8 press_int_enable:1;
		u8 pinch_int_enable:1;
	};
	u8 reg;
};

union f11_2d_ctrl11 {
	struct {
		u8 palm_detect_int_enable:1;
		u8 rotate_int_enable:1;
		u8 touch_shape_int_enable:1;
		u8 scroll_zone_int_enable:1;
		u8 multi_finger_scroll_int_enable:1;
	};
	u8 reg;
};

union f11_2d_ctrl12 {
	struct {
		u8 sensor_map:7;
		u8 xy_sel:1;
	};
	u8 reg;
};

union f11_2d_ctrl14 {
	struct {
		u8 sens_adjustment:5;
		u8 hyst_adjustment:3;
	};
	u8 reg;
};

union f11_2d_ctrl15 {
	struct {
		u8 max_tap_time:8;
	};
	u8 reg;
};

union f11_2d_ctrl16 {
	struct {
		u8 min_press_time:8;
	};
	u8 reg;
};

union f11_2d_ctrl17 {
	struct {
		u8 max_tap_distance:8;
	};
	u8 reg;
};

union f11_2d_ctrl18_19 {
	struct {
		u8 min_flick_distance:8;
		u8 min_flick_speed:8;
	};
	u8 reg[2];
};

union f11_2d_ctrl20_21 {
	struct {
		u8 pen_detect_enable:1;
		u8 pen_jitter_filter_enable:1;
		u8 ctrl20_reserved:6;
		u8 pen_z_threshold:8;
	};
	u8 reg[2];
};

/* These are not accessible through sysfs yet. */
union f11_2d_ctrl22_26 {
	struct {
		/* control 22 */
		u8 proximity_detect_int_en:1;
		u8 proximity_jitter_filter_en:1;
		u8 f11_2d_ctrl6_b3__7:6;

		/* control 23 */
		u8 proximity_detection_z_threshold;

		/* control 24 */
		u8 proximity_delta_x_threshold;

		/* control 25 */
		u8 proximity_delta_y_threshold;

		/* control 26 */
		u8 proximity_delta_z_threshold;
	};
	u8 regs[5];
};

/* control 27 - haspalmdetectsensitivity or has suppressonpalmdetect */
union f11_2d_ctrl27 {
	struct {
		u8 palm_detecy_sensitivity:4;
		u8 suppress_on_palm_detect:1;
		u8 f11_2d_ctrl27_b5__7:3;
	};
	u8 regs[1];
};

/* control 28 - has_multifingerscroll */
union f11_2d_ctrl28 {
	struct {
		u8 multi_finger_scroll_mode:2;
		u8 edge_motion_en:1;
		u8 f11_2d_ctrl28b_3:1;
		u8 multi_finger_scroll_momentum:4;
	};
	u8 regs[1];
};

/* control 29 & 30 - hasztuning */
union f11_2d_ctrl29_30 {
	struct {
		u8 z_touch_threshold;
		u8 z_touch_hysteresis;
	};
	struct {
		u8 regs[2];
		u16 address;
	};
};


struct  f11_2d_ctrl {
	union f11_2d_ctrl0_9 *ctrl0_9;
	union f11_2d_ctrl10		*ctrl10;
	union f11_2d_ctrl11		*ctrl11;
	union f11_2d_ctrl12		*ctrl12;
	u8				ctrl12_size;
	union f11_2d_ctrl14		*ctrl14;
	union f11_2d_ctrl15		*ctrl15;
	union f11_2d_ctrl16		*ctrl16;
	union f11_2d_ctrl17		*ctrl17;
	union f11_2d_ctrl18_19		*ctrl18_19;
	union f11_2d_ctrl20_21		*ctrl20_21;
	union f11_2d_ctrl22_26 *ctrl22_26;
	union f11_2d_ctrl27 *ctrl27;
	union f11_2d_ctrl28 *ctrl28;
	union f11_2d_ctrl29_30 *ctrl29_30;
};

struct f11_2d_data_1_5 {
	u8 x_msb;
	u8 y_msb;
	u8 x_lsb:4;
	u8 y_lsb:4;
	u8 w_y:4;
	u8 w_x:4;
	u8 z;
};

struct f11_2d_data_6_7 {
	s8 delta_x;
	s8 delta_y;
};

struct f11_2d_data_8 {
	u8 single_tap:1;
	u8 tap_and_hold:1;
	u8 double_tap:1;
	u8 early_tap:1;
	u8 flick:1;
	u8 press:1;
	u8 pinch:1;
};

struct f11_2d_data_9 {
	u8 palm_detect:1;
	u8 rotate:1;
	u8 shape:1;
	u8 scrollzone:1;
	u8 finger_count:3;
};

struct f11_2d_data_10 {
	u8 pinch_motion;
};

struct f11_2d_data_10_12 {
	u8 x_flick_dist;
	u8 y_flick_dist;
	u8 flick_time;
};

struct f11_2d_data_11_12 {
	u8 motion;
	u8 finger_separation;
};

struct f11_2d_data_13 {
	u8 shape_n;
};

struct f11_2d_data_14_15 {
	u8 horizontal;
	u8 vertical;
};

struct f11_2d_data_14_17 {
	u8 x_low;
	u8 y_right;
	u8 x_upper;
	u8 y_left;
};

struct f11_2d_data {
	u8				*f_state;
	const struct f11_2d_data_1_5	*abs_pos;
	const struct f11_2d_data_6_7	*rel_pos;
	const struct f11_2d_data_8	*gest_1;
	const struct f11_2d_data_9	*gest_2;
	const struct f11_2d_data_10	*pinch;
	const struct f11_2d_data_10_12	*flick;
	const struct f11_2d_data_11_12	*rotate;
	const struct f11_2d_data_13	*shapes;
	const struct f11_2d_data_14_15	*multi_scroll;
	const struct f11_2d_data_14_17	*scroll_zones;
};

struct f11_2d_sensor {
	struct rmi_f11_2d_axis_alignment axis_align;
	struct f11_2d_sensor_query sens_query;
	struct f11_2d_data data;
	u16 max_x;
	u16 max_y;
	u8 nbr_fingers;
	u8 finger_tracker[F11_MAX_NUM_OF_FINGERS];
	u8 *data_pkt;
	int pkt_size;
	u8 sensor_index;
	struct rmi_button_map virtualbutton_map;
	char input_name[MAX_NAME_LENGTH];
	char input_phys[MAX_NAME_LENGTH];
	struct input_dev *input;
	struct input_dev *mouse_input;
};

struct f11_data {
	struct f11_2d_device_query dev_query;
	struct f11_2d_ctrl dev_controls;
	struct mutex dev_controls_mutex;
#if	RESUME_REZERO
	u16 rezero_wait_ms;
	bool rezero_on_resume;
#endif
	struct f11_2d_sensor sensors[F11_MAX_NUM_OF_SENSORS];
	bool type_b;
};

enum finger_state_values {
	F11_NO_FINGER	= 0x00,
	F11_PRESENT	= 0x01,
	F11_INACCURATE	= 0x02,
	F11_RESERVED	= 0x03
};

//++ p11309 - 2013.06.26 for hover control
static int hover_state_flag = false;
//-- p11309

void touch_clear_finger(int flag)
{
}

static struct rmi_function_container *fc_temp_11;
u16 query_base_addr_f11;
u16 control_base_addr_f11;

int ghost_touch_check = 0;
int pantech_touch_debug = 0;
struct input_dev *pantech_dev;

/* ctrl sysfs files */
show_store_union_struct_prototype(abs_pos_filt)
show_store_union_struct_prototype(z_touch_threshold)
show_store_union_struct_prototype(z_touch_hysteresis)

/* Adding debugfs for flip, clip, offset and swap */
#ifdef CONFIG_RMI4_DEBUG

struct f11_debugfs_data {
	bool done;
	struct rmi_device *rmi_dev;
};

static int debug_open(struct inode *inodep, struct file *filp)
{
	struct f11_debugfs_data *data;

	data = kzalloc(sizeof(struct f11_debugfs_data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	data->rmi_dev = inodep->i_private;
	filp->private_data = data;
	return 0;
}

static int debug_release(struct inode *inodep, struct file *filp)
{
	kfree(filp->private_data);
	return 0;
}

#define FLIP_NAME "flip"
#define CLIP_NAME "clip"
#define OFFSET_NAME "offset"
#define SWAP_NAME "swap"


static ssize_t flip_read(struct file *filp, char __user *buffer, size_t size,
		    loff_t *offset) {
	struct f11_debugfs_data *dfs;
	int retval;
	char local_buf[size];
	struct rmi_device_platform_data *data;
	dfs = filp->private_data;
	data = dfs->rmi_dev->phys->dev->platform_data;

	if (dfs->done)
		return 0;

	dfs->done = 1;

	retval = snprintf(local_buf, PAGE_SIZE, "%u %u\n",
			data->axis_align.flip_x,
			data->axis_align.flip_y);

	if (retval <= 0 || copy_to_user(buffer, local_buf, retval))
		return -EFAULT;

	return retval;
}

static ssize_t flip_write(struct file *filp, const char __user *buffer,
			   size_t size, loff_t *offset) {

	struct f11_debugfs_data *dfs = filp->private_data;
	int retval;
	char local_buf[size];
	unsigned int new_X;
	unsigned int new_Y;
	struct rmi_device_platform_data *data =
			dfs->rmi_dev->phys->dev->platform_data;

	retval = copy_from_user(local_buf, buffer, size);
	if (retval)
		return -EFAULT;

	retval = sscanf(local_buf, "%u %u", &new_X, &new_Y);
	if (retval != 2) {
		dev_err(&dfs->rmi_dev->dev,
			"Incorrect number of values provided for flip.");
		return -EINVAL;
	}
	if (new_X < 0 || new_X > 1 || new_Y < 0 || new_Y > 1)
		return -EINVAL;

	dev_dbg(&dfs->rmi_dev->dev,
		 "Setting flip to %u %u.\n", new_X, new_Y);
	data->axis_align.flip_x = new_X;
	data->axis_align.flip_y = new_Y;

	return size;
}

static const struct file_operations flip_fops = {
	.owner = THIS_MODULE,
	.open = debug_open,
	.release = debug_release,
	.read = flip_read,
	.write = flip_write,
};


static ssize_t offset_read(struct file *filp, char __user *buffer, size_t size,
		    loff_t *offset) {

	struct f11_debugfs_data *dfs;
	int retval;
	char local_buf[size];
	struct rmi_device_platform_data *data;
	dfs = filp->private_data;
	data = dfs->rmi_dev->phys->dev->platform_data;

	if (dfs->done)
		return 0;

	dfs->done = 1;
	retval = snprintf(local_buf, PAGE_SIZE, "%u %u\n",
			data->axis_align.offset_X,
			data->axis_align.offset_Y);

	if (retval <= 0 || copy_to_user(buffer, local_buf, retval))
		return -EFAULT;

	return retval;
}

static ssize_t offset_write(struct file *filp, const char __user *buffer,
			   size_t size, loff_t *offset)
{
	struct f11_debugfs_data *dfs = filp->private_data;
	int retval;
	char local_buf[size];
	int new_X;
	int new_Y;
	struct rmi_device_platform_data *data =
			dfs->rmi_dev->phys->dev->platform_data;

	retval = copy_from_user(local_buf, buffer, size);
	if (retval)
		return -EFAULT;
	retval = sscanf(local_buf, "%u %u", &new_X, &new_Y);
	if (retval != 2) {
		dev_err(&dfs->rmi_dev->dev,
			"Incorrect number of values provided for offset.");
		return -EINVAL;
	}

	dev_dbg(&dfs->rmi_dev->dev,
		 "Setting offset to %u %u.\n", new_X, new_Y);
	data->axis_align.offset_X = new_X;
	data->axis_align.offset_Y = new_Y;

	return size;
}

static const struct file_operations offset_fops = {
	.owner = THIS_MODULE,
	.open = debug_open,
	.release = debug_release,
	.read = offset_read,
	.write = offset_write,
};

static ssize_t clip_read(struct file *filp, char __user *buffer, size_t size,
		    loff_t *offset) {

	struct f11_debugfs_data *dfs;
	int retval;
	char local_buf[size];
	struct rmi_device_platform_data *data;
	dfs = filp->private_data;
	data = dfs->rmi_dev->phys->dev->platform_data;

	if (dfs->done)
		return 0;

	dfs->done = 1;

	retval = snprintf(local_buf, PAGE_SIZE, "%u %u %u %u\n",
			data->axis_align.clip_X_low,
			data->axis_align.clip_X_high,
			data->axis_align.clip_Y_low,
			data->axis_align.clip_Y_high);

	if (retval <= 0 || copy_to_user(buffer, local_buf, retval))
		return -EFAULT;

	return retval;
}

static ssize_t clip_write(struct file *filp, const char __user *buffer,
			   size_t size, loff_t *offset)
{
	struct f11_debugfs_data *dfs = filp->private_data;
	int retval;
	char local_buf[size];
	unsigned int new_X_low, new_X_high, new_Y_low, new_Y_high;

	struct rmi_device_platform_data *data =
			dfs->rmi_dev->phys->dev->platform_data;

	retval = copy_from_user(local_buf, buffer, size);
	if (retval)
		return -EFAULT;

	retval = sscanf(local_buf, "%u %u %u %u",
		&new_X_low, &new_X_high, &new_Y_low, &new_Y_high);
	if (retval != 4) {
		dev_err(&dfs->rmi_dev->dev,
			"Incorrect number of values provided for clip.");
		return -EINVAL;
	}

	if (new_X_low < 0 || new_X_low >= new_X_high || new_Y_low < 0
	    || new_Y_low >= new_Y_high)
		return -EINVAL;

	dev_dbg(&dfs->rmi_dev->dev,
		 "Setting clip to %u %u %u %u.\n", new_X_low, new_X_high,
			new_Y_low, new_Y_high);

	data->axis_align.clip_X_low = new_X_low;
	data->axis_align.clip_X_high = new_X_high;
	data->axis_align.clip_Y_low = new_Y_low;
	data->axis_align.clip_Y_high = new_Y_high;

	return size;
}

static const struct file_operations clip_fops = {
	.owner = THIS_MODULE,
	.open = debug_open,
	.release = debug_release,
	.read = clip_read,
	.write = clip_write,
};

static ssize_t swap_read(struct file *filp, char __user *buffer, size_t size,
		    loff_t *offset) {

	struct f11_debugfs_data *dfs;
	int retval;
	char local_buf[size];
	struct rmi_device_platform_data *data;
	dfs = filp->private_data;
	data = dfs->rmi_dev->phys->dev->platform_data;

	if (dfs->done)
		return 0;

	dfs->done = 1;

	retval = snprintf(local_buf, PAGE_SIZE, "%u\n",
			data->axis_align.swap_axes);

	if (retval <= 0 || copy_to_user(buffer, local_buf, retval))
		return -EFAULT;

	return retval;
}

static ssize_t swap_write(struct file *filp, const char __user *buffer,
			   size_t size, loff_t *offset)
{
	struct f11_debugfs_data *dfs = filp->private_data;
	struct rmi_device_platform_data *data =
			dfs->rmi_dev->phys->dev->platform_data;

	int retval;
	char local_buf[size];
	int newSwap;

	if (!dfs->rmi_dev->debugfs_root)
		return -ENODEV;

	retval = copy_from_user(local_buf, buffer, size);
	if (retval)
		return -EFAULT;
	retval = sscanf(local_buf, "%u", &newSwap);
	if (retval != 1) {
		dev_err(&dfs->rmi_dev->dev,
			"Incorrect number of values provided for swap.");
		return -EINVAL;
	}
	if (newSwap < 0 || newSwap > 1)
		return -EINVAL;

	data->axis_align.swap_axes = newSwap;
	return size;
}

static const struct file_operations swap_fops = {
	.owner = THIS_MODULE,
	.open = debug_open,
	.release = debug_release,
	.read = swap_read,
	.write = swap_write,
};

static int setup_debugfs(struct rmi_device *rmi_dev)
{
	int retval = 0;
	struct rmi_device_platform_data *data;
	data = rmi_dev->phys->dev->platform_data;

	if (!rmi_dev->debugfs_root)
		return -ENODEV;

	data->axis_align.debugfs_flip
		= debugfs_create_file(FLIP_NAME, RMI_RW_ATTR,
				rmi_dev->debugfs_root, rmi_dev, &flip_fops);
	if (!data->axis_align.debugfs_flip
		|| IS_ERR(data->axis_align.debugfs_flip)) {
		dev_warn(&rmi_dev->dev, "Failed to create debugfs flip.\n");
		data->axis_align.debugfs_flip = NULL;
	}

	data->axis_align.debugfs_clip
		= debugfs_create_file(CLIP_NAME, RMI_RW_ATTR,
				rmi_dev->debugfs_root, rmi_dev, &clip_fops);
	if (!data->axis_align.debugfs_clip
		|| IS_ERR(data->axis_align.debugfs_clip)) {
		dev_warn(&rmi_dev->dev, "Failed to create debugfs clip.\n");
		data->axis_align.debugfs_clip = NULL;
	}

	data->axis_align.debugfs_offset
		= debugfs_create_file(OFFSET_NAME, RMI_RW_ATTR,
				rmi_dev->debugfs_root, rmi_dev, &offset_fops);
	if (!data->axis_align.debugfs_offset
		|| IS_ERR(data->axis_align.debugfs_offset)) {
		dev_warn(&rmi_dev->dev, "Failed to create debugfs offset.\n");
		data->axis_align.debugfs_offset = NULL;
	}

	data->axis_align.debugfs_swap
		= debugfs_create_file(SWAP_NAME, RMI_RW_ATTR,
				rmi_dev->debugfs_root, rmi_dev, &swap_fops);
	if (!data->axis_align.debugfs_swap
		|| IS_ERR(data->axis_align.debugfs_swap)) {
		dev_warn(&rmi_dev->dev, "Failed to create debugfs swap.\n");
		data->axis_align.debugfs_swap = NULL;
	}

	return retval;
}

static void teardown_debugfs(struct rmi_device *rmi_dev)
{
	struct rmi_device_platform_data *data;
	data = rmi_dev->phys->dev->platform_data;

	if (!data->axis_align.debugfs_flip)
		debugfs_remove(data->axis_align.debugfs_flip);

	if (!data->axis_align.debugfs_clip)
		debugfs_remove(data->axis_align.debugfs_clip);

	if (!data->axis_align.debugfs_offset)
		debugfs_remove(data->axis_align.debugfs_offset);

	if (!data->axis_align.debugfs_swap)
		debugfs_remove(data->axis_align.debugfs_swap);
}

#endif
/* End adding debugfs */

/* This is a group in case we add the other ctrls. */
static struct attribute *attrs_ctrl0[] = {
	attrify(abs_pos_filt),
	NULL
};
static struct attribute_group attrs_control0 = GROUP(attrs_ctrl0);

static struct attribute *attrs_ctrl29_30[] = {
	attrify(z_touch_threshold),
	attrify(z_touch_hysteresis),
	NULL
};
static struct attribute_group attrs_control29_30 = GROUP(attrs_ctrl29_30);

/** F11_INACCURATE state is overloaded to indicate pen present. */
#define F11_PEN F11_INACCURATE

static int get_tool_type(struct f11_2d_sensor *sensor, u8 finger_state)
{
#ifdef	CONFIG_RMI4_F11_PEN
	if (sensor->sens_query.query9.has_pen && finger_state == F11_PEN)
		return MT_TOOL_PEN;
#endif
	return MT_TOOL_FINGER;
}

static void rmi_f11_rel_pos_report(struct f11_2d_sensor *sensor, u8 n_finger)
{
	struct f11_2d_data *data = &sensor->data;
	struct rmi_f11_2d_axis_alignment *axis_align = &sensor->axis_align;
	s8 x, y;
	s8 temp;

	x = data->rel_pos[n_finger].delta_x;
	y = data->rel_pos[n_finger].delta_y;

	x = min(F11_REL_POS_MAX, max(F11_REL_POS_MIN, (int)x));
	y = min(F11_REL_POS_MAX, max(F11_REL_POS_MIN, (int)y));

	if (axis_align->swap_axes) {
		temp = x;
		x = y;
		y = temp;
	}
	if (axis_align->flip_x)
		x = min(F11_REL_POS_MAX, -x);
	if (axis_align->flip_y)
		y = min(F11_REL_POS_MAX, -y);

	if (x || y) {
		input_report_rel(sensor->input, REL_X, x);
		input_report_rel(sensor->input, REL_Y, y);
		input_report_rel(sensor->mouse_input, REL_X, x);
		input_report_rel(sensor->mouse_input, REL_Y, y);
	}
	input_sync(sensor->mouse_input);
}

static int prev_touch_x =0;

static void rmi_f11_abs_pos_report(struct f11_data *f11,
				   struct f11_2d_sensor *sensor,
				   u8 finger_state, u8 n_finger)
{
	struct f11_2d_data *data = &sensor->data;
	struct rmi_f11_2d_axis_alignment *axis_align = &sensor->axis_align;
	int prev_state = sensor->finger_tracker[n_finger];
	int x, y, z;
	int w_x, w_y, w_max, w_min, orient;
	int temp;
	static int his;

	if (prev_state && !finger_state) {

//++ p11309 - 2013.06.26 for hover control
		if ( hover_state_flag == true ) {
			PAN_DBG("Hover Enter state...ignored\n");
			hover_state_flag = false;
			return;
		}
//-- p11309

		/* this is a release */		
		x = y = z = w_max = w_min = orient = 0;
	} else if (!prev_state && !finger_state) {
		/* nothing to report */
		return;
	} else {
		x = ((data->abs_pos[n_finger].x_msb << 4) |
			data->abs_pos[n_finger].x_lsb);
		y = ((data->abs_pos[n_finger].y_msb << 4) |
			data->abs_pos[n_finger].y_lsb);
		z = data->abs_pos[n_finger].z;
		w_x = data->abs_pos[n_finger].w_x;
		w_y = data->abs_pos[n_finger].w_y;
		w_max = max(w_x, w_y);
		w_min = min(w_x, w_y);

		if (axis_align->swap_axes) {
			temp = x;
			x = y;
			y = temp;
			temp = w_x;
			w_x = w_y;
			w_y = temp;
		}

		orient = w_x > w_y ? 1 : 0;

		if (axis_align->flip_x)
			x = max(sensor->max_x - x, 0);

		if (axis_align->flip_y)
			y = max(sensor->max_y - y, 0);

		/*
		** here checking if X offset or y offset are specified is
		**  redundant.  We just add the offsets or, clip the values
		**
		** note: offsets need to be done before clipping occurs,
		** or we could get funny values that are outside
		** clipping boundaries.
		*/
		x += axis_align->offset_X;
		y += axis_align->offset_Y;
		x =  max(axis_align->clip_X_low, x);
		y =  max(axis_align->clip_Y_low, y);
		if (axis_align->clip_X_high)
			x = min(axis_align->clip_X_high, x);
		if (axis_align->clip_Y_high)
			y =  min(axis_align->clip_Y_high, y);

	}

	PAN_DBG("f_state[%d]:%d - x:%d y:%d z:%d w_max:%d w_min:%d\n", n_finger, finger_state, x, y, z, w_max, w_min);
//	pr_debug("%s: f_state[%d]:%d - x:%d y:%d z:%d w_max:%d w_min:%d\n",
//		__func__, n_finger, finger_state, x, y, z, w_max, w_min);

	if(pantech_touch_debug)
	{
		if(his == 0)
		{
			printk("%s: f_state[%d]:%d - x:%d y:%d z:%d w_max:%d w_min:%d\n",
				__func__, n_finger, finger_state, x, y, z, w_max, w_min);
			his = 1;
		}
		else if(z==0)
		{
			printk("%s: f_state[%d]:%d - x:%d y:%d z:%d w_max:%d w_min:%d\n",
				__func__, n_finger, finger_state, x, y, z, w_max, w_min);		
			his = 0;
		}
	}
	
	if(finger_state && z == 0)
	{
		printk("[Touch] Detected the Ghost touch\n");

		if(!pantech_touch_debug)
			printk("%s: f_state[%d]:%d - x:%d y:%d z:%d w_max:%d w_min:%d\n",
				__func__, n_finger, finger_state, x, y, z, w_max, w_min);				
		
		if(prev_touch_x == 0)
		{
			ghost_touch_check++;
			pantech_touch_debug = true;

		//	gpio_set_value(TOUCH_RST_GPIO, 0);
			if(pantech_dev)
				rmi_clear_finger(pantech_dev);		
		//	msleep(100);
		//	gpio_set_value(TOUCH_RST_GPIO, 1);
		}
	}

	if(z != 0)
		prev_touch_x = x;

#ifndef CONFIG_RMI4_F11_PEN
	/* Some UIs ignore W of zero, so we fudge it to 1 for pens. */
	if (get_tool_type(sensor, finger_state) == MT_TOOL_PEN) {
		w_max = max(1, w_max);
		w_min = max(1, w_min);
	}
#endif

#ifdef CONFIG_RMI4_F11_TYPEB
	if (f11->type_b) {
		input_mt_slot(sensor->input, n_finger);
		input_mt_report_slot_state(sensor->input,
					   get_tool_type(sensor, finger_state),
					   finger_state);
	} else
		input_report_abs(sensor->input, ABS_MT_TOOL_TYPE,
				 get_tool_type(sensor, finger_state));
#else
	input_report_abs(sensor->input, ABS_MT_TOOL_TYPE,
				get_tool_type(sensor, finger_state));
#endif

	if(z != 0) //Point release
	{
#ifdef ABS_MT_PRESSURE
		input_report_abs(sensor->input, ABS_MT_PRESSURE, z);
#endif
		input_report_abs(sensor->input, ABS_MT_TOUCH_MAJOR, w_max);
		input_report_abs(sensor->input, ABS_MT_TOUCH_MINOR, w_min);
		input_report_abs(sensor->input, ABS_MT_ORIENTATION, orient);
	//	input_report_abs(sensor->input, ABS_MT_POSITION_X, 720-x);
		input_report_abs(sensor->input, ABS_MT_POSITION_X, x);
		input_report_abs(sensor->input, ABS_MT_POSITION_Y, y);
		input_report_abs(sensor->input, ABS_MT_TOOL_TYPE,
				get_tool_type(sensor, finger_state));		
		input_report_abs(sensor->input, ABS_MT_TRACKING_ID, n_finger);
	}
//	input_report_key(sensor->input, BTN_TOUCH, finger_state);

	/* MT sync between fingers */
#ifdef CONFIG_RMI4_F11_TYPEB
	if (!f11->type_b)
		input_mt_sync(sensor->input);
#else
	input_mt_sync(sensor->input);
#endif
	sensor->finger_tracker[n_finger] = finger_state;
}


#ifdef CONFIG_RMI4_VIRTUAL_BUTTON
static int rmi_f11_virtual_button_handler(struct f11_2d_sensor *sensor)
{
	int i;
	int x;
	int y;
	struct rmi_button_map *virtualbutton_map;

	if (sensor->sens_query.has_gestures &&
		sensor->data.gest_1->single_tap) {
		virtualbutton_map = &sensor->virtualbutton_map;
		x = ((sensor->data.abs_pos[0].x_msb << 4) |
			sensor->data.abs_pos[0].x_lsb);
		y = ((sensor->data.abs_pos[0].y_msb << 4) |
			sensor->data.abs_pos[0].y_lsb);
		for (i = 0; i < virtualbutton_map->buttons; i++) {
			if (INBOX(x, y, virtualbutton_map->map[i])) {
				input_report_key(sensor->input,
					virtualbutton_map->map[i].code, 1);
				input_report_key(sensor->input,
					virtualbutton_map->map[i].code, 0);
				input_sync(sensor->input);
				return 0;
			}
		}
	}
	return 0;
}
#else
#define rmi_f11_virtual_button_handler(sensor)
#endif

static void rmi_f11_finger_handler(struct f11_data *f11,
				   struct f11_2d_sensor *sensor)
{
	const u8 *f_state = sensor->data.f_state;
	u8 finger_state;
	u8 finger_pressed_count;
	u8 i;

	for (i = 0, finger_pressed_count = 0; i < sensor->nbr_fingers; i++) {
		/* Possible of having 4 fingers per f_statet register */
		finger_state = GET_FINGER_STATE(f_state, i);

		if (finger_state == F11_RESERVED) {
			pr_err("%s: Invalid finger state[%d]:0x%02x.", __func__, i, finger_state);
			continue;
		} 
//++ p11309 - 2013.06.26 for hover control
//		else if ((finger_state == F11_PRESENT) || (finger_state == F11_INACCURATE)) {
// 			finger_pressed_count++;		
// 		}
		else if (finger_state == F11_PRESENT) {
			finger_pressed_count++;
			hover_state_flag = false;
		}
		else if ( finger_state == F11_INACCURATE ) {
			if ( hover_state_flag == false ) {
				PAN_DBG("Hover Enter state...entered\n");
				hover_state_flag = true;				
			}
			continue;
		}
//-- p11309

		if (sensor->data.abs_pos)
			rmi_f11_abs_pos_report(f11, sensor, finger_state, i);

		if (sensor->data.rel_pos)
			rmi_f11_rel_pos_report(sensor, i);
	}
	input_report_key(sensor->input, BTN_TOUCH, finger_pressed_count);
	input_sync(sensor->input);
}

static int f11_2d_construct_data(struct f11_2d_sensor *sensor)
{
	struct f11_2d_sensor_query *query = &sensor->sens_query;
	struct f11_2d_data *data = &sensor->data;
	int i;

	sensor->nbr_fingers = (query->number_of_fingers == 5 ? 10 :
				query->number_of_fingers + 1);

	sensor->pkt_size = F11_CEIL(sensor->nbr_fingers, 4);

	if (query->has_abs)
		sensor->pkt_size += (sensor->nbr_fingers * 5);

	if (query->has_rel)
		sensor->pkt_size +=  (sensor->nbr_fingers * 2);

	/* Check if F11_2D_Query7 is non-zero */
	if (query->f11_2d_query7__8[0])
		sensor->pkt_size += sizeof(u8);

	/* Check if F11_2D_Query7 or F11_2D_Query8 is non-zero */
	if (query->f11_2d_query7__8[0] || query->f11_2d_query7__8[1])
		sensor->pkt_size += sizeof(u8);

	if (query->has_pinch || query->has_flick || query->has_rotate) {
		sensor->pkt_size += 3;
		if (!query->has_flick)
			sensor->pkt_size--;
		if (!query->has_rotate)
			sensor->pkt_size--;
	}

	if (query->has_touch_shapes)
		sensor->pkt_size += F11_CEIL(query->nbr_touch_shapes + 1, 8);

	sensor->data_pkt = kzalloc(sensor->pkt_size, GFP_KERNEL);
	if (!sensor->data_pkt)
		return -ENOMEM;

	data->f_state = sensor->data_pkt;
	i = F11_CEIL(sensor->nbr_fingers, 4);

	if (query->has_abs) {
		data->abs_pos = (struct f11_2d_data_1_5 *)
				&sensor->data_pkt[i];
		i += (sensor->nbr_fingers * 5);
	}

	if (query->has_rel) {
		data->rel_pos = (struct f11_2d_data_6_7 *)
				&sensor->data_pkt[i];
		i += (sensor->nbr_fingers * 2);
	}

	if (query->f11_2d_query7__8[0]) {
		data->gest_1 = (struct f11_2d_data_8 *)&sensor->data_pkt[i];
		i++;
	}

	if (query->f11_2d_query7__8[0] || query->f11_2d_query7__8[1]) {
		data->gest_2 = (struct f11_2d_data_9 *)&sensor->data_pkt[i];
		i++;
	}

	if (query->has_pinch) {
		data->pinch = (struct f11_2d_data_10 *)&sensor->data_pkt[i];
		i++;
	}

	if (query->has_flick) {
		if (query->has_pinch) {
			data->flick = (struct f11_2d_data_10_12 *)data->pinch;
			i += 2;
		} else {
			data->flick = (struct f11_2d_data_10_12 *)
					&sensor->data_pkt[i];
			i += 3;
		}
	}

	if (query->has_rotate) {
		if (query->has_flick) {
			data->rotate = (struct f11_2d_data_11_12 *)
					(data->flick + 1);
		} else {
			data->rotate = (struct f11_2d_data_11_12 *)
					&sensor->data_pkt[i];
			i += 2;
		}
	}

	if (query->has_touch_shapes)
		data->shapes = (struct f11_2d_data_13 *)&sensor->data_pkt[i];

	return 0;
}

static void f11_free_control_regs(struct f11_2d_ctrl *ctrl)
{
	kfree(ctrl->ctrl10);
	kfree(ctrl->ctrl11);
	kfree(ctrl->ctrl14);
	kfree(ctrl->ctrl15);
	kfree(ctrl->ctrl16);
	kfree(ctrl->ctrl17);
	kfree(ctrl->ctrl18_19);
	kfree(ctrl->ctrl20_21);
	kfree(ctrl->ctrl22_26);
	kfree(ctrl->ctrl27);
	kfree(ctrl->ctrl28);
	kfree(ctrl->ctrl29_30);
	ctrl->ctrl10 = NULL;
	ctrl->ctrl11 = NULL;
	ctrl->ctrl14 = NULL;
	ctrl->ctrl15 = NULL;
	ctrl->ctrl16 = NULL;
	ctrl->ctrl17 = NULL;
	ctrl->ctrl18_19 = NULL;
	ctrl->ctrl20_21 = NULL;
	ctrl->ctrl22_26 = NULL;
	ctrl->ctrl27 = NULL;
	ctrl->ctrl28 = NULL;
	ctrl->ctrl29_30 = NULL;
}

static int f11_read_control_regs(struct rmi_device *rmi_dev,
					   struct f11_2d_ctrl *ctrl,
					   u16 ctrl_base_addr) {
	u16 read_address = ctrl_base_addr;
	int error = 0;

	ctrl->ctrl0_9->address = read_address;
	error = rmi_read_block(rmi_dev, read_address, ctrl->ctrl0_9->regs,
		sizeof(ctrl->ctrl0_9->regs));
	if (error < 0) {
		dev_err(&rmi_dev->dev,
			"Failed to read F11 ctrl0, code: %d.\n", error);
		return error;
	}
	read_address = read_address + sizeof(ctrl->ctrl0_9->regs);

	if (ctrl->ctrl10) {
		error = rmi_read_block(rmi_dev, read_address,
			&ctrl->ctrl10->reg, sizeof(union f11_2d_ctrl10));
		if (error < 0) {
			dev_err(&rmi_dev->dev,
				"Failed to read F11 ctrl10, code: %d.\n",
				error);
			return error;
		}
		read_address = read_address + sizeof(union f11_2d_ctrl10);
	}

	if (ctrl->ctrl11) {
		error = rmi_read_block(rmi_dev, read_address,
			&ctrl->ctrl11->reg, sizeof(union f11_2d_ctrl11));
		if (error < 0) {
			dev_err(&rmi_dev->dev,
				"Failed to read F11 ctrl11, code: %d.\n",
				error);
			return error;
		}
		read_address = read_address + sizeof(union f11_2d_ctrl11);
	}

	if (ctrl->ctrl14) {
		error = rmi_read_block(rmi_dev, read_address,
			&ctrl->ctrl14->reg, sizeof(union f11_2d_ctrl14));
		if (error < 0) {
			dev_err(&rmi_dev->dev,
				"Failed to read F11 ctrl14, code: %d.\n",
				error);
			return error;
		}
		read_address = read_address + sizeof(union f11_2d_ctrl14);
	}

	if (ctrl->ctrl15) {
		error = rmi_read_block(rmi_dev, read_address,
			&ctrl->ctrl15->reg, sizeof(union f11_2d_ctrl15));
		if (error < 0) {
			dev_err(&rmi_dev->dev,
				"Failed to read F11 ctrl15, code: %d.\n",
				error);
			return error;
		}
		read_address = read_address + sizeof(union f11_2d_ctrl15);
	}

	if (ctrl->ctrl16) {
		error = rmi_read_block(rmi_dev, read_address,
			&ctrl->ctrl16->reg, sizeof(union f11_2d_ctrl16));
		if (error < 0) {
			dev_err(&rmi_dev->dev,
				"Failed to read F11 ctrl16, code: %d.\n",
				error);
			return error;
		}
		read_address = read_address + sizeof(union f11_2d_ctrl16);
	}

	if (ctrl->ctrl17) {
		error = rmi_read_block(rmi_dev, read_address,
			&ctrl->ctrl17->reg, sizeof(union f11_2d_ctrl17));
		if (error < 0) {
			dev_err(&rmi_dev->dev,
				"Failed to read F11 ctrl17, code: %d.\n",
				error);
			return error;
		}
		read_address = read_address + sizeof(union f11_2d_ctrl17);
	}

	if (ctrl->ctrl18_19) {
		error = rmi_read_block(rmi_dev, read_address,
			ctrl->ctrl18_19->reg, sizeof(union f11_2d_ctrl18_19));
		if (error < 0) {
			dev_err(&rmi_dev->dev,
				"Failed to read F11 ctrl18_19, code: %d.\n",
				error);
			return error;
		}
		read_address = read_address + sizeof(union f11_2d_ctrl18_19);
	}

	if (ctrl->ctrl20_21) {
		error = rmi_read_block(rmi_dev, read_address,
			ctrl->ctrl20_21->reg, sizeof(union f11_2d_ctrl20_21));
		if (error < 0) {
			dev_err(&rmi_dev->dev,
				"Failed to read F11 ctrl20_21, code: %d.\n",
				error);
			return error;
		}
		read_address = read_address + sizeof(union f11_2d_ctrl20_21);
	}

	if (ctrl->ctrl22_26) {
		error = rmi_read_block(rmi_dev, read_address,
			ctrl->ctrl22_26->regs, sizeof(union f11_2d_ctrl22_26));
		if (error < 0) {
			dev_err(&rmi_dev->dev,
				"Failed to read F11 ctrl22_26, code: %d.\n",
				error);
			return error;
		}
		read_address = read_address + sizeof(union f11_2d_ctrl22_26);
	}

	if (ctrl->ctrl27) {
		error = rmi_read_block(rmi_dev, read_address,
			ctrl->ctrl27->regs, sizeof(union f11_2d_ctrl27));
		if (error < 0) {
			dev_err(&rmi_dev->dev,
				"Failed to read F11 ctrl27, code: %d.\n",
				error);
			return error;
		}
		read_address = read_address + sizeof(union f11_2d_ctrl27);
	}

	if (ctrl->ctrl28) {
		error = rmi_read_block(rmi_dev, read_address,
			ctrl->ctrl28->regs, sizeof(union f11_2d_ctrl28));
		if (error < 0) {
			dev_err(&rmi_dev->dev,
				"Failed to read F11 ctrl28, code: %d.\n",
				error);
			return error;
		}
		read_address = read_address + sizeof(union f11_2d_ctrl28);
	}

	if (ctrl->ctrl29_30) {
		ctrl->ctrl29_30->address = read_address;
		error = rmi_read_block(rmi_dev, read_address,
			ctrl->ctrl29_30->regs, sizeof(ctrl->ctrl29_30->regs));
		if (error < 0) {
			dev_err(&rmi_dev->dev,
				"Failed to read F11 ctrl29_30, code: %d.\n",
				error);
			return error;
		}
		read_address = read_address + sizeof(ctrl->ctrl29_30->regs);
	}
	return 0;
}

static int f11_allocate_control_regs(struct rmi_device *rmi_dev,
				struct f11_2d_device_query *device_query,
				struct f11_2d_sensor_query *sensor_query,
				struct f11_2d_ctrl *ctrl,
				u16 ctrl_base_addr) {

	int error = 0;
	ctrl->ctrl0_9 = kzalloc(sizeof(union f11_2d_ctrl0_9),
				       GFP_KERNEL);
	if (!ctrl->ctrl0_9) {
		error = -ENOMEM;
		goto error_exit;
	}
	if (sensor_query->f11_2d_query7__8[0]) {
		ctrl->ctrl10 = kzalloc(sizeof(union f11_2d_ctrl10),
				       GFP_KERNEL);
		if (!ctrl->ctrl10) {
			error = -ENOMEM;
			goto error_exit;
		}
	}

	if (sensor_query->f11_2d_query7__8[1]) {
		ctrl->ctrl11 = kzalloc(sizeof(union f11_2d_ctrl11),
				       GFP_KERNEL);
		if (!ctrl->ctrl11) {
			error = -ENOMEM;
			goto error_exit;
		}
	}

	if (device_query->has_query9 && sensor_query->query9.has_pen) {
		ctrl->ctrl20_21 = kzalloc(sizeof(union f11_2d_ctrl20_21),
					  GFP_KERNEL);
		if (!ctrl->ctrl20_21) {
			error = -ENOMEM;
			goto error_exit;
		}
	}

	if (device_query->has_query9 && sensor_query->query9.has_proximity) {
		ctrl->ctrl22_26 = kzalloc(sizeof(union f11_2d_ctrl22_26),
					  GFP_KERNEL);
		if (!ctrl->ctrl22_26) {
			error = -ENOMEM;
			goto error_exit;
		}
	}

	if (device_query->has_query9 &&
		(sensor_query->query9.has_palm_det_sensitivity ||
		sensor_query->query9.has_suppress_on_palm_detect)) {
		ctrl->ctrl27 = kzalloc(sizeof(union f11_2d_ctrl27),
					  GFP_KERNEL);
		if (!ctrl->ctrl27) {
			error = -ENOMEM;
			goto error_exit;
		}
	}

	if (sensor_query->has_multi_finger_scroll) {
		ctrl->ctrl28 = kzalloc(sizeof(union f11_2d_ctrl28),
					  GFP_KERNEL);
		if (!ctrl->ctrl28) {
			error = -ENOMEM;
			goto error_exit;
		}
	}

	if (device_query->has_query11 && device_query->has_z_tuning) {
		ctrl->ctrl29_30 = kzalloc(sizeof(union f11_2d_ctrl29_30),
					  GFP_KERNEL);
		if (!ctrl->ctrl29_30) {
			error = -ENOMEM;
			goto error_exit;
		}
	}

	return f11_read_control_regs(rmi_dev, ctrl, ctrl_base_addr);

error_exit:
	f11_free_control_regs(ctrl);
	return error;
}

static int f11_write_control_regs(struct rmi_device *rmi_dev,
					struct f11_2d_sensor_query *query,
					struct f11_2d_ctrl *ctrl,
					u16 ctrl_base_addr)
{
	u16 write_address = ctrl_base_addr;
	int error;

	error = rmi_write_block(rmi_dev, write_address,
				ctrl->ctrl0_9->regs,
				 sizeof(ctrl->ctrl0_9->regs));
	if (error < 0)
		return error;
	write_address += sizeof(ctrl->ctrl0_9);

	if (ctrl->ctrl10) {
		error = rmi_write_block(rmi_dev, write_address,
					&ctrl->ctrl10->reg, 1);
		if (error < 0)
			return error;
		write_address++;
	}

	if (ctrl->ctrl11) {
		error = rmi_write_block(rmi_dev, write_address,
					&ctrl->ctrl11->reg, 1);
		if (error < 0)
			return error;
		write_address++;
	}

	if (ctrl->ctrl12 && ctrl->ctrl12_size && query->configurable) {
		if (ctrl->ctrl12_size > query->max_electrodes) {
			dev_err(&rmi_dev->dev,
				"%s: invalid cfg size:%d, should be < %d.\n",
				__func__, ctrl->ctrl12_size,
				query->max_electrodes);
			return -EINVAL;
		}
		error = rmi_write_block(rmi_dev, write_address,
						&ctrl->ctrl12->reg,
						ctrl->ctrl12_size);
		if (error < 0)
			return error;
		write_address += ctrl->ctrl12_size;
	}

	if (ctrl->ctrl14) {
		error = rmi_write_block(rmi_dev, write_address,
				&ctrl->ctrl14->reg, 1);
		if (error < 0)
			return error;
		write_address++;
	}

	if (ctrl->ctrl15) {
		error = rmi_write_block(rmi_dev, write_address,
				&ctrl->ctrl15->reg, 1);
		if (error < 0)
			return error;
		write_address++;
	}

	if (ctrl->ctrl16) {
		error = rmi_write_block(rmi_dev, write_address,
				&ctrl->ctrl16->reg, 1);
		if (error < 0)
			return error;
		write_address++;
	}

	if (ctrl->ctrl17) {
		error = rmi_write_block(rmi_dev, write_address,
				&ctrl->ctrl17->reg, 1);
		if (error < 0)
			return error;
		write_address++;
	}

	if (ctrl->ctrl18_19) {
		error = rmi_write_block(rmi_dev, write_address,
			ctrl->ctrl18_19->reg, sizeof(union f11_2d_ctrl18_19));
		if (error < 0)
			return error;
		write_address += sizeof(union f11_2d_ctrl18_19);
	}

	if (ctrl->ctrl20_21) {
		error = rmi_write_block(rmi_dev, write_address,
					ctrl->ctrl20_21->reg,
					sizeof(union f11_2d_ctrl20_21));
		if (error < 0)
			return error;
		write_address += sizeof(union f11_2d_ctrl20_21);
	}

	if (ctrl->ctrl22_26) {
		error = rmi_write_block(rmi_dev, write_address,
					ctrl->ctrl22_26->regs,
					sizeof(union f11_2d_ctrl22_26));
		if (error < 0)
			return error;
		write_address += sizeof(union f11_2d_ctrl22_26);
	}

	if (ctrl->ctrl27) {
		error = rmi_write_block(rmi_dev, write_address,
					ctrl->ctrl27->regs,
					sizeof(union f11_2d_ctrl27));
		if (error < 0)
			return error;
		write_address += sizeof(union f11_2d_ctrl27);
	}

	if (ctrl->ctrl28) {
		error = rmi_write_block(rmi_dev, write_address,
					ctrl->ctrl28->regs,
					sizeof(union f11_2d_ctrl28));
		if (error < 0)
			return error;
		write_address += sizeof(union f11_2d_ctrl28);
	}

	if (ctrl->ctrl29_30) {
		error = rmi_write_block(rmi_dev, write_address,
					ctrl->ctrl29_30->regs,
					sizeof(union f11_2d_ctrl29_30));
		if (error < 0)
			return error;
		write_address += sizeof(union f11_2d_ctrl29_30);
	}

	return 0;
}

static int rmi_f11_get_query_parameters(struct rmi_device *rmi_dev,
			struct f11_2d_sensor_query *query, u16 query_base_addr)
{
	int query_size;
	int rc;

	rc = rmi_read_block(rmi_dev, query_base_addr, query->f11_2d_query1__4,
					sizeof(query->f11_2d_query1__4));
	if (rc < 0)
		return rc;
	query_size = rc;

	if (query->has_abs) {
		rc = rmi_read(rmi_dev, query_base_addr + query_size,
					&query->f11_2d_query5);
		if (rc < 0)
			return rc;
		query_size++;
	}

	if (query->has_rel) {
		rc = rmi_read(rmi_dev, query_base_addr + query_size,
					&query->f11_2d_query6);
		if (rc < 0)
			return rc;
		query_size++;
	}

	if (query->has_gestures) {
		rc = rmi_read_block(rmi_dev, query_base_addr + query_size,
					query->f11_2d_query7__8,
					sizeof(query->f11_2d_query7__8));
		if (rc < 0)
			return rc;
		query_size += sizeof(query->f11_2d_query7__8);
	}

	if (query->has_touch_shapes) {
		rc = rmi_read(rmi_dev, query_base_addr + query_size,
					&query->f11_2d_query10);
		if (rc < 0)
			return rc;
		query_size++;
	}

	return query_size;
}

/* This operation is done in a number of places, so we have a handy routine
 * for it.
 */
static void f11_set_abs_params(struct rmi_function_container *fc, int index)
{
	struct f11_data *f11 = fc->data;
	struct f11_2d_sensor *sensor = &f11->sensors[index];
	struct input_dev *input = sensor->input;
	int device_x_max =
		f11->dev_controls.ctrl0_9->sensor_max_x_pos;
	int device_y_max =
		f11->dev_controls.ctrl0_9->sensor_max_y_pos;
	int x_min, x_max, y_min, y_max;
	if (sensor->axis_align.swap_axes) {
		int temp = device_x_max;
		device_x_max = device_y_max;
		device_y_max = temp;
	}
	/* Use the max X and max Y read from the device, or the clip values,
	 * whichever is stricter.
	 */
	x_min = sensor->axis_align.clip_X_low;
	if (sensor->axis_align.clip_X_high)
		x_max = min((int) device_x_max,
			sensor->axis_align.clip_X_high);
	else
		x_max = device_x_max;

	y_min = sensor->axis_align.clip_Y_low;
	if (sensor->axis_align.clip_Y_high)
		y_max = min((int) device_y_max,
			sensor->axis_align.clip_Y_high);
	else
		y_max = device_y_max;

//	dev_dbg(&fc->dev, "Set ranges X=[%d..%d] Y=[%d..%d].", x_min, x_max, y_min, y_max);
	printk("Set ranges X=[%d..%d] Y=[%d..%d].", x_min, x_max, y_min, y_max);

#ifdef ABS_MT_PRESSURE
	input_set_abs_params(input, ABS_MT_PRESSURE, 0,
			DEFAULT_MAX_ABS_MT_PRESSURE, 0, 0);
#endif
	input_set_abs_params(input, ABS_MT_TOUCH_MAJOR,
			0, DEFAULT_MAX_ABS_MT_TOUCH, 0, 0);
	input_set_abs_params(input, ABS_MT_TOUCH_MINOR,
			0, DEFAULT_MAX_ABS_MT_TOUCH, 0, 0);
	input_set_abs_params(input, ABS_MT_ORIENTATION,
			0, DEFAULT_MAX_ABS_MT_ORIENTATION, 0, 0);
	input_set_abs_params(input, ABS_MT_TRACKING_ID,
			DEFAULT_MIN_ABS_MT_TRACKING_ID,
			DEFAULT_MAX_ABS_MT_TRACKING_ID, 0, 0);
	/* TODO get max_x_pos (and y) from control registers. */
	input_set_abs_params(input, ABS_MT_POSITION_X,
			x_min, x_max, 0, 0);
	input_set_abs_params(input, ABS_MT_POSITION_Y,
			y_min, y_max, 0, 0);
#ifdef CONFIG_RMI4_F11_TYPEB
	if (f11->type_b)
		input_mt_init_slots(input, sensor->nbr_fingers);
#endif
#ifdef	CONFIG_RMI4_F11_PEN
	if (sensor->sens_query.query9.has_pen)
		input_set_abs_params(input, ABS_MT_TOOL_TYPE,
				     0, MT_TOOL_MAX, 0, 0);
	else
		input_set_abs_params(input, ABS_MT_TOOL_TYPE,
				     0, MT_TOOL_MAX, 0, 0);
#else
	input_set_abs_params(input, ABS_MT_TOOL_TYPE, 0, MT_TOOL_FINGER, 0, 0);
#endif
}

static int rmi_f11_init(struct rmi_function_container *fc)
{
	int rc;

	PAN_DBG("%s\n", __func__);

	rc = rmi_f11_initialize(fc);
	if (rc < 0)
		goto err_free_data;

	rc = rmi_f11_register_devices(fc);
	if (rc < 0)
		goto err_free_data;

	rc = rmi_f11_create_sysfs(fc);
	if (rc < 0)
		goto err_free_data;

	return 0;

err_free_data:
	rmi_f11_free_memory(fc);

	return rc;
}

static void rmi_f11_free_memory(struct rmi_function_container *fc)
{
	struct f11_data *f11 = fc->data;
	int i;

	if (f11) {
		f11_free_control_regs(&f11->dev_controls);
		for (i = 0; i < f11->dev_query.nbr_of_sensors + 1; i++)
			kfree(f11->sensors[i].virtualbutton_map.map);
		kfree(f11);
		fc->data = NULL;
	}
}


static int rmi_f11_initialize(struct rmi_function_container *fc)
{
	struct rmi_device *rmi_dev = fc->rmi_dev;
	struct f11_data *f11;
	u8 query_offset;
	u16 query_base_addr;
	u16 control_base_addr;
	u16 max_x_pos, max_y_pos, temp;
	int rc;
	int i;
	struct rmi_device_platform_data *pdata = to_rmi_platform_data(rmi_dev);

	dev_dbg(&fc->dev, "Initializing F11 values for %s.\n",
		 pdata->sensor_name);

	/*
	** init instance data, fill in values and create any sysfs files
	*/
	f11 = kzalloc(sizeof(struct f11_data), GFP_KERNEL);
	if (!f11)
		return -ENOMEM;

	fc->data = f11;
#if	RESUME_REZERO
	f11->rezero_on_resume = true;
	f11->rezero_wait_ms = DEFAULT_REZERO_WAIT_MS;
#endif
	#ifdef CONFIG_RMI4_F11_TYPEB
	f11->type_b = pdata->f11_type_b;
#endif

	query_base_addr = fc->fd.query_base_addr;
	control_base_addr = fc->fd.control_base_addr;
    query_base_addr_f11 = fc->fd.query_base_addr;
    control_base_addr_f11 = fc->fd.control_base_addr;

	rc = rmi_read(rmi_dev, query_base_addr, &f11->dev_query.f11_2d_query0);
	if (rc < 0)
		return rc;

	query_offset = (query_base_addr + 1);
	/* Increase with one since number of sensors is zero based */
	for (i = 0; i < (f11->dev_query.nbr_of_sensors + 1); i++) {
		f11->sensors[i].sensor_index = i;

		rc = rmi_f11_get_query_parameters(rmi_dev,
					&f11->sensors[i].sens_query,
					query_offset);
		if (rc < 0)
			return rc;
		query_offset += rc;

		if (f11->dev_query.has_query9) {
			rc = rmi_read(rmi_dev, query_offset,
				      &f11->sensors[i].sens_query.query9.reg);
			if (rc < 0) {
				dev_err(&fc->dev, "Failed to read query 9.\n");
				return rc;
			}
			query_offset += rc;
		}

		rc = f11_allocate_control_regs(rmi_dev,
				&f11->dev_query, &f11->sensors[i].sens_query,
				&f11->dev_controls, control_base_addr);
		if (rc < 0) {
			dev_err(&fc->dev,
				"Failed to initialize F11 control params.\n");
			return rc;
		}

		f11->sensors[i].axis_align = pdata->axis_align;

		rc = rmi_read_block(rmi_dev,
			control_base_addr + F11_CTRL_SENSOR_MAX_X_POS_OFFSET,
			(u8 *)&max_x_pos, sizeof(max_x_pos));
		if (rc < 0)
			return rc;

		rc = rmi_read_block(rmi_dev,
			control_base_addr + F11_CTRL_SENSOR_MAX_Y_POS_OFFSET,
			(u8 *)&max_y_pos, sizeof(max_y_pos));
		if (rc < 0)
			return rc;

		if (pdata->axis_align.swap_axes) {
			temp = max_x_pos;
			max_x_pos = max_y_pos;
			max_y_pos = temp;
		}
		f11->sensors[i].max_x = max_x_pos;
		f11->sensors[i].max_y = max_y_pos;

		rc = f11_2d_construct_data(&f11->sensors[i]);
		if (rc < 0)
			return rc;
	}
#ifdef CONFIG_RMI4_DEBUG
	rc = setup_debugfs(rmi_dev);
	if (rc < 0)
		dev_warn(&fc->dev, "Failed to setup debugfs. Code: %d.\n",
			 rc);
#endif
	mutex_init(&f11->dev_controls_mutex);
	fc_temp_11 = fc;
	return 0;
}

static int rmi_f11_register_devices(struct rmi_function_container *fc)
{
	struct rmi_device *rmi_dev = fc->rmi_dev;
	struct f11_data *f11 = fc->data;
	struct input_dev *input_dev;
	struct input_dev *input_dev_mouse;
	int sensors_itertd = 0;
	int i;
	int rc;
#ifdef CONFIG_RMI4_VIRTUAL_BUTTON
	struct rmi_button_map *vm_sensor;
	struct rmi_button_map *vm_pdata;
	struct rmi_device_platform_data *pdata = to_rmi_platform_data(rmi_dev);
#endif

	for (i = 0; i < (f11->dev_query.nbr_of_sensors + 1); i++) {
		sensors_itertd = i;
		input_dev = input_allocate_device();
		if (!input_dev) {
			rc = -ENOMEM;
			goto error_unregister;
		}

		f11->sensors[i].input = input_dev;
		/* TODO how to modify the dev name and
		* phys name for input device */
		sprintf(f11->sensors[i].input_name, "%sfn%02x",
			dev_name(&rmi_dev->dev), fc->fd.function_number);
		input_dev->name = f11->sensors[i].input_name;
		sprintf(f11->sensors[i].input_phys, "%s/input0",
			input_dev->name);
		input_dev->phys = f11->sensors[i].input_phys;
		input_dev->dev.parent = &rmi_dev->dev;
		input_set_drvdata(input_dev, f11);

		set_bit(EV_SYN, input_dev->evbit);
		set_bit(EV_KEY, input_dev->evbit);
		set_bit(EV_ABS, input_dev->evbit);
#ifdef INPUT_PROP_DIRECT
		set_bit(INPUT_PROP_DIRECT, input_dev->propbit);
#endif

		f11_set_abs_params(fc, i);

		dev_dbg(&fc->dev, "%s: Sensor %d hasRel %d.\n",
			__func__, i, f11->sensors[i].sens_query.has_rel);
		if (f11->sensors[i].sens_query.has_rel) {
			set_bit(EV_REL, input_dev->evbit);
			set_bit(REL_X, input_dev->relbit);
			set_bit(REL_Y, input_dev->relbit);
		}
		rc = input_register_device(input_dev);
		if (rc < 0) {
			input_free_device(input_dev);
			f11->sensors[i].input = NULL;
			goto error_unregister;
		}

#ifdef SKY_PROCESS_CMD_KEY
		pantech_dev= input_dev; //gpio_keys.c
#endif

		/* how to register the virtualbutton device */
#ifdef CONFIG_RMI4_VIRTUAL_BUTTON
		if (f11->sensors[i].sens_query.has_gestures) {
			int j;

			vm_sensor = &f11->sensors[i].virtualbutton_map;
			vm_pdata = pdata->f11_button_map;
			if (!vm_pdata) {
				dev_err(&fc->dev, "Failed to get the pdata virtualbutton map.\n");
				goto error_unregister;
			}
			vm_sensor->buttons = vm_pdata->buttons;
			vm_sensor->map = kcalloc(vm_pdata->buttons,
					sizeof(struct virtualbutton_map),
					GFP_KERNEL);
			if (!vm_sensor->map) {
				dev_err(&fc->dev, "Failed to allocate the virtualbutton map.\n");
				rc = -ENOMEM;
				goto error_unregister;
			}

			/* manage button map using input subsystem */
			input_dev->keycode = vm_sensor->map;
			input_dev->keycodesize =
					sizeof(struct virtualbutton_map);
			input_dev->keycodemax = vm_pdata->buttons;

			/* set bits for each button... */
			for (j = 0; j < vm_pdata->buttons; j++) {
				memcpy(&vm_sensor->map[j], &vm_pdata->map[j],
					sizeof(struct virtualbutton_map));
				set_bit(vm_sensor->map[j].code,
					f11->sensors[i].input->keybit);
			}
		}

#endif

		if (f11->sensors[i].sens_query.has_rel) {
			/*create input device for mouse events  */
			input_dev_mouse = input_allocate_device();
			if (!input_dev_mouse) {
				rc = -ENOMEM;
				goto error_unregister;
			}

			f11->sensors[i].mouse_input = input_dev_mouse;
			input_dev_mouse->name = "rmi_mouse";
			input_dev_mouse->phys = "rmi_f11/input0";

			input_dev_mouse->id.vendor  = 0x18d1;
			input_dev_mouse->id.product = 0x0210;
			input_dev_mouse->id.version = 0x0100;

			set_bit(EV_REL, input_dev_mouse->evbit);
			set_bit(REL_X, input_dev_mouse->relbit);
			set_bit(REL_Y, input_dev_mouse->relbit);

			set_bit(BTN_MOUSE, input_dev_mouse->evbit);
			/* Register device's buttons and keys */
			set_bit(EV_KEY, input_dev_mouse->evbit);
			set_bit(BTN_LEFT, input_dev_mouse->keybit);
			set_bit(BTN_MIDDLE, input_dev_mouse->keybit);
			set_bit(BTN_RIGHT, input_dev_mouse->keybit);

			rc = input_register_device(input_dev_mouse);
			if (rc < 0) {
				input_free_device(input_dev_mouse);
				f11->sensors[i].mouse_input = NULL;
				goto error_unregister;
			}

			set_bit(BTN_RIGHT, input_dev_mouse->keybit);
		}

	}

	return 0;

error_unregister:
	for (; sensors_itertd > 0; sensors_itertd--) {
		if (f11->sensors[sensors_itertd].input) {
			if (f11->sensors[sensors_itertd].mouse_input) {
				input_unregister_device(
				   f11->sensors[sensors_itertd].mouse_input);
				f11->sensors[sensors_itertd].mouse_input = NULL;
			}
			input_unregister_device(f11->sensors[i].input);
			f11->sensors[i].input = NULL;
		}
		kfree(f11->sensors[i].virtualbutton_map.map);
	}

	return rc;
}

static void rmi_f11_free_devices(struct rmi_function_container *fc)
{
	struct f11_data *f11 = fc->data;
	int i;

	for (i = 0; i < (f11->dev_query.nbr_of_sensors + 1); i++) {
		if (f11->sensors[i].input)
			input_unregister_device(f11->sensors[i].input);
		if (f11->sensors[i].sens_query.has_rel &&
				f11->sensors[i].mouse_input)
			input_unregister_device(f11->sensors[i].mouse_input);
	}
}

static int rmi_f11_create_sysfs(struct rmi_function_container *fc)
{
	int attr_count = 0;
	int rc;
	struct f11_data *f11 = fc->data;

	dev_dbg(&fc->dev, "Creating sysfs files.\n");
	/* Set up sysfs device attributes. */
	for (attr_count = 0; attr_count < ARRAY_SIZE(attrs); attr_count++) {
		if (sysfs_create_file
		    (&fc->dev.kobj, &attrs[attr_count].attr) < 0) {
			dev_err(&fc->dev,
				"Failed to create sysfs file for %s.",
				attrs[attr_count].attr.name);
			rc = -ENODEV;
			goto err_remove_sysfs;
		}
	}
	if (sysfs_create_group(&fc->dev.kobj, &attrs_control0) < 0) {
		dev_err(&fc->dev, "Failed to create query sysfs files.\n");
		return -ENODEV;
	}
	if (f11->dev_controls.ctrl29_30) {
		if (sysfs_create_group(&fc->dev.kobj,
			&attrs_control29_30) < 0) {
			dev_err(&fc->dev,
				"Failed to create query sysfs files.");
			return -ENODEV;
		}
	}

	return 0;

err_remove_sysfs:
	for (attr_count--; attr_count >= 0; attr_count--)
		sysfs_remove_file(&fc->dev.kobj, &attrs[attr_count].attr);
	sysfs_remove_group(&fc->dev.kobj, &attrs_control0);
	if (f11->dev_controls.ctrl29_30)
		sysfs_remove_group(&fc->dev.kobj, &attrs_control29_30);
	return rc;
}

static int rmi_f11_config(struct rmi_function_container *fc)
{
	struct f11_data *f11 = fc->data;
	int i;
	int rc;

	PAN_DBG("%s\n", __func__);

	for (i = 0; i < (f11->dev_query.nbr_of_sensors + 1); i++) {
		rc = f11_write_control_regs(fc->rmi_dev,
				   &f11->sensors[i].sens_query,
				   &f11->dev_controls,
				   fc->fd.query_base_addr);
		if (rc < 0)
			return rc;
	}

	return 0;
}

static int rmi_f11_reset(struct rmi_function_container *fc)
{
	/* we do nothing here */
	return 0;
}

int rmi_f11_attention(struct rmi_function_container *fc, u8 *irq_bits)
{
	struct rmi_device *rmi_dev = fc->rmi_dev;
	struct f11_data *f11 = fc->data;
	u16 data_base_addr = fc->fd.data_base_addr;
	u16 data_base_addr_offset = 0;
	int error;
	int i;

	for (i = 0; i < f11->dev_query.nbr_of_sensors + 1; i++) {
		error = rmi_read_block(rmi_dev,
				data_base_addr + data_base_addr_offset,
				f11->sensors[i].data_pkt,
				f11->sensors[i].pkt_size);
		if (error < 0)
			return error;

		rmi_f11_finger_handler(f11, &f11->sensors[i]);
		rmi_f11_virtual_button_handler(&f11->sensors[i]);
		data_base_addr_offset += f11->sensors[i].pkt_size;
	}

	return 0;
}

#define MAX_TRACK_ID	16

void rmi_clear_finger(struct input_dev *input)
{
	int i = 0;

	if(pantech_touch_debug)
		printk("[Touch] RMI rmi_clear_finger\n");
	
	for (i=0; i<MAX_TRACK_ID; i++) {
		input_mt_slot(input, i);
		input_mt_report_slot_state(input, MT_TOOL_FINGER, false);
	}

	input_report_key(input, BTN_TOUCH, 0);
	input_sync(input);
}



#if RESUME_REZERO
static int rmi_f11_suspend(struct rmi_function_container *fc)
{
	struct f11_data *f11 = fc->data;
	/* Command register always reads as 0, so we can just use a local. */
	int retval = 0, i =0;

	if(pantech_touch_debug)
		printk("[Touch] RMI F11 suspend\n");

	for (i = 0; i < (f11->dev_query.nbr_of_sensors + 1); i++) {
		if (f11->sensors[i].input)
			rmi_clear_finger(f11->sensors[i].input);
	}

	return retval;
}

static int rmi_f11_resume(struct rmi_function_container *fc)
{
	struct rmi_device *rmi_dev = fc->rmi_dev;
	struct f11_data *data = fc->data;
	/* Command register always reads as 0, so we can just use a local. */
	union f11_2d_commands commands = {};
	int retval = 0;

	dev_dbg(&fc->dev, "Resuming...\n");
	if (!data->rezero_on_resume)
		return 0;

	if (data->rezero_wait_ms)
		mdelay(data->rezero_wait_ms);

	commands.rezero = 1;
	retval = rmi_write_block(rmi_dev, fc->fd.command_base_addr,
			&commands.reg, sizeof(commands.reg));
	if (retval < 0) {
		dev_err(&rmi_dev->dev, "%s: failed to issue rezero command, error = %d.",
			__func__, retval);
		return retval;
	}

	return retval;
}
#endif /* RESUME_REZERO */

static void rmi_f11_remove(struct rmi_function_container *fc)
{
	int attr_count = 0;
	struct f11_data *f11 = fc->data;
//	struct rmi_device *rmi_dev = fc->rmi_dev;

#ifdef	CONFIG_RMI4_DEBUG
	teardown_debugfs(rmi_dev);
#endif

	for (attr_count = 0; attr_count < ARRAY_SIZE(attrs); attr_count++)
		sysfs_remove_file(&fc->dev.kobj,
				  &attrs[attr_count].attr);

	sysfs_remove_group(&fc->dev.kobj, &attrs_control0);
	if (f11->dev_controls.ctrl29_30)
		sysfs_remove_group(&fc->dev.kobj, &attrs_control29_30);

	rmi_f11_free_devices(fc);

	rmi_f11_free_memory(fc);

}

static struct rmi_function_handler function_handler = {
	.func = 0x11,
	.init = rmi_f11_init,
	.config = rmi_f11_config,
	.reset = rmi_f11_reset,
	.attention = rmi_f11_attention,
	.remove = rmi_f11_remove,
#if	RESUME_REZERO
#if defined(CONFIG_HAS_EARLYSUSPEND) && \
			!defined(CONFIG_RMI4_SPECIAL_EARLYSUSPEND)
	.late_resume = rmi_f11_resume,
	.early_suspend = rmi_f11_suspend,
#else
	.resume = rmi_f11_resume,
	.suspend = rmi_f11_suspend,
#endif  /* defined(CONFIG_HAS_EARLYSUSPEND) && !def... */
#endif
};

static int __init rmi_f11_module_init(void)
{
	int error;

	error = rmi_register_function_driver(&function_handler);
	if (error < 0) {
		pr_err("%s: register failed!\n", __func__);
		return error;
	}

	return 0;
}

static void __exit rmi_f11_module_exit(void)
{
	rmi_unregister_function_driver(&function_handler);
}

static ssize_t f11_maxPos_show(struct device *dev,
				     struct device_attribute *attr,
				     char *buf)
{
	struct rmi_function_container *fc;
	struct f11_data *data;

	fc = to_rmi_function_container(dev);
	data = fc->data;

	return snprintf(buf, PAGE_SIZE, "%u %u\n",
			data->sensors[0].max_x, data->sensors[0].max_y);
}

static ssize_t f11_relreport_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct rmi_function_container *fc;
	struct f11_data *instance_data;

	fc = to_rmi_function_container(dev);
	instance_data = fc->data;

	return snprintf(buf, PAGE_SIZE, "%u\n",
			instance_data->
			sensors[0].axis_align.rel_report_enabled);
}

static ssize_t f11_relreport_store(struct device *dev,
					 struct device_attribute *attr,
					 const char *buf,
					 size_t count)
{
	struct rmi_function_container *fc;
	struct f11_data *instance_data;
	unsigned int new_value;

	fc = to_rmi_function_container(dev);
	instance_data = fc->data;


	if (sscanf(buf, "%u", &new_value) != 1)
		return -EINVAL;
	if (new_value < 0 || new_value > 1)
		return -EINVAL;
	instance_data->sensors[0].axis_align.rel_report_enabled = new_value;

	return count;
}

static ssize_t f11_rezero_store(struct device *dev,
				     struct device_attribute *attr,
				     const char *buf, size_t count)
{
	struct rmi_function_container *fc = NULL;
	unsigned int rezero;
	int retval = 0;
	/* Command register always reads as 0, so we can just use a local. */
	union f11_2d_commands commands = {};

	fc = to_rmi_function_container(dev);

	if (sscanf(buf, "%u", &rezero) != 1)
		return -EINVAL;
	if (rezero < 0 || rezero > 1)
		return -EINVAL;

	/* Per spec, 0 has no effect, so we skip it entirely. */
	if (rezero) {
		commands.rezero = 1;
		retval = rmi_write_block(fc->rmi_dev, fc->fd.command_base_addr,
				&commands.reg, sizeof(commands.reg));
		if (retval < 0) {
			dev_err(dev, "%s: failed to issue rezero command, error = %d.",
				__func__, retval);
			return retval;
		}
	}

	return count;
}

#if RESUME_REZERO
static ssize_t f11_rezeroOnResume_store(struct device *dev,
				     struct device_attribute *attr,
				     const char *buf, size_t count)
{
	struct rmi_function_container *fc = NULL;
	unsigned int newValue;
	struct f11_data *instance_data;

	fc = to_rmi_function_container(dev);
	instance_data = fc->data;

	if (sscanf(buf, "%u", &newValue) != 1)
		return -EINVAL;
	if (newValue < 0 || newValue > 1) {
		dev_err(dev, "rezeroOnResume must be either 1 or 0.\n");
		return -EINVAL;
	}

	instance_data->rezero_on_resume = (newValue != 0);

	return count;
}

static ssize_t f11_rezeroOnResume_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct rmi_function_container *fc;
	struct f11_data *instance_data;

	fc = to_rmi_function_container(dev);
	instance_data = fc->data;

	return snprintf(buf, PAGE_SIZE, "%u\n",
			instance_data->rezero_on_resume);
}

static ssize_t f11_rezeroWait_store(struct device *dev,
				     struct device_attribute *attr,
				     const char *buf, size_t count)
{
	struct rmi_function_container *fc = NULL;
	unsigned int newValue;
	struct f11_data *instance_data;

	fc = to_rmi_function_container(dev);
	instance_data = fc->data;

	if (sscanf(buf, "%u", &newValue) != 1)
		return -EINVAL;
	if (newValue < 0) {
		dev_err(dev, "rezeroWait must be 0 or greater.\n");
		return -EINVAL;
	}

	instance_data->rezero_wait_ms = (newValue != 0);

	return count;
}

static ssize_t f11_rezeroWait_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct rmi_function_container *fc;
	struct f11_data *instance_data;

	fc = to_rmi_function_container(dev);
	instance_data = fc->data;

	return snprintf(buf, PAGE_SIZE, "%u\n",
			instance_data->rezero_wait_ms);
}

#endif

/* Control sysfs files */
show_store_union_struct_unsigned(dev_controls, ctrl0_9, abs_pos_filt)
show_store_union_struct_unsigned(dev_controls, ctrl29_30, z_touch_threshold)
show_store_union_struct_unsigned(dev_controls, ctrl29_30, z_touch_hysteresis)
module_init(rmi_f11_module_init);
module_exit(rmi_f11_module_exit);

MODULE_AUTHOR("Christopher Heiny <cheiny@synaptics.com");
MODULE_DESCRIPTION("RMI F11 module");
MODULE_LICENSE("GPL");
MODULE_VERSION(RMI_DRIVER_VERSION);
