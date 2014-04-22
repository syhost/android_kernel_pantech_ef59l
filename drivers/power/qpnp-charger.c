/* Copyright (c) 2012-2013, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#define pr_fmt(fmt)	"%s: " fmt, __func__

#include <linux/module.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/spmi.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/radix-tree.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/qpnp/qpnp-adc.h>
#include <linux/power_supply.h>
#include <linux/bitops.h>
#include <linux/ratelimit.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/of_regulator.h>
#include <linux/regulator/machine.h>

#if defined(FEATURE_PANTECH_PMIC_MONITOR_TEST)
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/miscdevice.h>
#endif

#if defined(FEATURE_PANTECH_PMIC_BMS_TEST)
#include <linux/input.h>
#endif

#if defined(FEATURE_PANTECH_PMIC_BATTERY_CHARGING_DISCHARGING_TEST) || defined(FEATURE_PANTECH_PMIC_MONITOR_TEST)
#include <linux/types.h>
#include <linux/ioctl.h>
#endif

#if defined(CONFIG_PANTECH_PMIC_CHARGER_SMB349) || defined(CONFIG_PANTECH_PMIC_CHARGER_SMB347)
#include <mach/restart.h>
#endif

#if defined(FEATURE_POWER_ON_OFF_TEST) // 20130524. MKS.
#include <linux/input.h>
#include <linux/time.h>
#endif
#if defined(CONFIG_PANTECH_PMIC_FUELGAUGE_MAX17058)
#include <mach/msm_smsm.h>
#include <linux/power/max17058_battery_pantech.h>
#endif 

#if defined(CONFIG_PANTECH_PMIC_CHARGER_SMB349) || defined(CONFIG_PANTECH_PMIC_CHARGER_SMB347)
#if defined(CONFIG_PANTECH_PMIC_CHARGER_SMB349)
#include <linux/power/smb349_charger_pantech.h>
#elif defined(CONFIG_PANTECH_PMIC_CHARGER_SMB347)
#include <linux/power/smb347_charger_pantech.h>
#endif
#if defined(FEATURE_PANTECH_PMIC_USBIN_DROP_WORKAROUND)
#include <linux/gpio.h>
#include <mach/gpiomux.h>
#endif

#if defined(FEATURE_PANTECH_PMIC_CHARGING_DISABLE)
extern bool smb_chg_disabled ;  // sayuss charge_CMD 
#endif
extern int is_factory_dummy ;   // sayuss Factory
extern void smb349_current_set(int cable_type, int current_ma);
extern void smb349_current_jeita(int mode, int cable_type);
extern int is_temp_state_changed(int* state, int64_t temp);
extern void smb349_charing_enable(bool enable);
#if defined(FEATURE_PANTECH_PMIC_EOC)
extern void end_recharging_enable(bool enable);
#endif
#if defined(FEATURE_PANTECH_PMIC_EOC) || defined(FEATURE_PANTECH_PMIC_BMS_TEST)
extern int max17058_FG_SOC(void);
#endif
#endif

#if defined(FEATURE_PANTECH_PMIC_CHARGE_LIMIT_WITH_LCD)
extern int  offline_boot_ok, boot_lcd_cnt;
#endif

#ifndef FEATURE_PANTECH_QUALCOMM_OTG_MODE_OVP_BUG
#define FEATURE_PANTECH_QUALCOMM_OTG_MODE_OVP_BUG
#endif
#ifdef FEATURE_PANTECH_QUALCOMM_OTG_MODE_OVP_BUG
static int is_pantech_host_mode = 0;
#endif
#if defined(CONFIG_PANTECH_PMIC_CHARGER_SMB349)
extern int is_CDP; // sayuss 2013.09.05 CDP issue
#endif

//20130610 djjeon PMIC  10% offline charging  Auto power on
#if defined FEATURE_PANTECH_PMIC_AUTO_PWR_ON
#include <linux/reboot.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <asm/uaccess.h>
#include <linux/workqueue.h>
#endif

/* Interrupt offsets */
#define INT_RT_STS(base)			(base + 0x10)
#define INT_SET_TYPE(base)			(base + 0x11)
#define INT_POLARITY_HIGH(base)			(base + 0x12)
#define INT_POLARITY_LOW(base)			(base + 0x13)
#define INT_LATCHED_CLR(base)			(base + 0x14)
#define INT_EN_SET(base)			(base + 0x15)
#define INT_EN_CLR(base)			(base + 0x16)
#define INT_LATCHED_STS(base)			(base + 0x18)
#define INT_PENDING_STS(base)			(base + 0x19)
#define INT_MID_SEL(base)			(base + 0x1A)
#define INT_PRIORITY(base)			(base + 0x1B)

/* Peripheral register offsets */
#define CHGR_CHG_OPTION				0x08
#define CHGR_ATC_STATUS				0x0A
#define CHGR_VBAT_STATUS			0x0B
#define CHGR_IBAT_BMS				0x0C
#define CHGR_IBAT_STS				0x0D
#define CHGR_VDD_MAX				0x40
#define CHGR_VDD_SAFE				0x41
#define CHGR_VDD_MAX_STEP			0x42
#define CHGR_IBAT_MAX				0x44
#define CHGR_IBAT_SAFE				0x45
#define CHGR_VIN_MIN				0x47
#define CHGR_VIN_MIN_STEP			0x48
#define CHGR_CHG_CTRL				0x49
#define CHGR_CHG_FAILED				0x4A
#define CHGR_ATC_CTRL				0x4B
#define CHGR_ATC_FAILED				0x4C
#define CHGR_VBAT_TRKL				0x50
#define CHGR_VBAT_WEAK				0x52
#define CHGR_IBAT_ATC_A				0x54
#define CHGR_IBAT_ATC_B				0x55
#define CHGR_IBAT_TERM_CHGR			0x5B
#define CHGR_IBAT_TERM_BMS			0x5C
#define CHGR_VBAT_DET				0x5D
#define CHGR_TTRKL_MAX				0x5F
#define CHGR_TTRKL_MAX_EN			0x60
#define CHGR_TCHG_MAX				0x61
#define CHGR_CHG_WDOG_TIME			0x62
#define CHGR_CHG_WDOG_DLY			0x63
#define CHGR_CHG_WDOG_PET			0x64
#define CHGR_CHG_WDOG_EN			0x65
#define CHGR_IR_DROP_COMPEN			0x67
#define CHGR_I_MAX_REG			0x44
#define CHGR_USB_USB_SUSP			0x47
#define CHGR_USB_USB_OTG_CTL			0x48
#define CHGR_USB_ENUM_T_STOP			0x4E
#define CHGR_CHG_TEMP_THRESH			0x66
#define CHGR_BAT_IF_PRES_STATUS			0x08
#if defined(CONFIG_PANTECH_PMIC_CHARGER_SMB349) || defined(CONFIG_PANTECH_PMIC_CHARGER_SMB347)
#define CHGR_BAT_IF_PRES_CTRL         0x48
#define CHGR_BAT_IF_BTC_CTRL           0x49
#endif
#define CHGR_STATUS				0x09
#define CHGR_BAT_IF_VCP				0x42
#define CHGR_BAT_IF_BATFET_CTRL1		0x90
#define CHGR_MISC_BOOT_DONE			0x42
#define CHGR_BUCK_COMPARATOR_OVRIDE_1		0xEB
#define CHGR_BUCK_COMPARATOR_OVRIDE_3		0xED
#define CHGR_BUCK_BCK_VBAT_REG_MODE		0x74
#define MISC_REVISION2				0x01
#define USB_OVP_CTL				0x42
#define USB_CHG_GONE_REV_BST			0xED
#define BUCK_VCHG_OV				0x77
#define BUCK_TEST_SMBC_MODES			0xE6
#define SEC_ACCESS				0xD0
#define BAT_IF_VREF_BAT_THM_CTRL		0x4A
#define BAT_IF_BPD_CTRL				0x48
#define BOOST_VSET				0x41
#define BOOST_ENABLE_CONTROL			0x46
#define COMP_OVR1				0xEA
#define BAT_IF_BTC_CTRL				0x49
#define USB_OCP_THR				0x52
#define USB_OCP_CLR				0x53
#define BAT_IF_TEMP_STATUS			0x09

#define REG_OFFSET_PERP_SUBTYPE			0x05

/* SMBB peripheral subtype values */
#define SMBB_CHGR_SUBTYPE			0x01
#define SMBB_BUCK_SUBTYPE			0x02
#define SMBB_BAT_IF_SUBTYPE			0x03
#define SMBB_USB_CHGPTH_SUBTYPE			0x04
#define SMBB_DC_CHGPTH_SUBTYPE			0x05
#define SMBB_BOOST_SUBTYPE			0x06
#define SMBB_MISC_SUBTYPE			0x07

/* SMBB peripheral subtype values */
#define SMBBP_CHGR_SUBTYPE			0x31
#define SMBBP_BUCK_SUBTYPE			0x32
#define SMBBP_BAT_IF_SUBTYPE			0x33
#define SMBBP_USB_CHGPTH_SUBTYPE		0x34
#define SMBBP_BOOST_SUBTYPE			0x36
#define SMBBP_MISC_SUBTYPE			0x37

/* SMBCL peripheral subtype values */
#define SMBCL_CHGR_SUBTYPE			0x41
#define SMBCL_BUCK_SUBTYPE			0x42
#define SMBCL_BAT_IF_SUBTYPE			0x43
#define SMBCL_USB_CHGPTH_SUBTYPE		0x44
#define SMBCL_MISC_SUBTYPE			0x47

#define QPNP_CHARGER_DEV_NAME	"qcom,qpnp-charger"

/* Status bits and masks */
#define CHGR_BOOT_DONE			BIT(7)
#define CHGR_CHG_EN			BIT(7)
#define CHGR_ON_BAT_FORCE_BIT		BIT(0)
#define USB_VALID_DEB_20MS		0x03
#define BUCK_VBAT_REG_NODE_SEL_BIT	BIT(0)
#define VREF_BATT_THERM_FORCE_ON	0xC0
#define BAT_IF_BPD_CTRL_SEL		0x03
#define VREF_BAT_THM_ENABLED_FSM	0x80
#define REV_BST_DETECTED		BIT(0)
#define BAT_THM_EN			BIT(1)
#define BAT_ID_EN			BIT(0)
#define BOOST_PWR_EN			BIT(7)

/* Interrupt definitions */
/* smbb_chg_interrupts */
#define CHG_DONE_IRQ			BIT(7)
#define CHG_FAILED_IRQ			BIT(6)
#define FAST_CHG_ON_IRQ			BIT(5)
#define TRKL_CHG_ON_IRQ			BIT(4)
#define STATE_CHANGE_ON_IR		BIT(3)
#define CHGWDDOG_IRQ			BIT(2)
#define VBAT_DET_HI_IRQ			BIT(1)
#define VBAT_DET_LOW_IRQ		BIT(0)

/* smbb_buck_interrupts */
#define VDD_LOOP_IRQ			BIT(6)
#define IBAT_LOOP_IRQ			BIT(5)
#define ICHG_LOOP_IRQ			BIT(4)
#define VCHG_LOOP_IRQ			BIT(3)
#define OVERTEMP_IRQ			BIT(2)
#define VREF_OV_IRQ			BIT(1)
#define VBAT_OV_IRQ			BIT(0)

/* smbb_bat_if_interrupts */
#define PSI_IRQ				BIT(4)
#define VCP_ON_IRQ			BIT(3)
#define BAT_FET_ON_IRQ			BIT(2)
#define BAT_TEMP_OK_IRQ			BIT(1)
#define BATT_PRES_IRQ			BIT(0)

/* smbb_usb_interrupts */
#define CHG_GONE_IRQ			BIT(2)
#define USBIN_VALID_IRQ			BIT(1)
#define COARSE_DET_USB_IRQ		BIT(0)

/* smbb_dc_interrupts */
#define DCIN_VALID_IRQ			BIT(1)
#define COARSE_DET_DC_IRQ		BIT(0)

/* smbb_boost_interrupts */
#define LIMIT_ERROR_IRQ			BIT(1)
#define BOOST_PWR_OK_IRQ		BIT(0)

/* smbb_misc_interrupts */
#define TFTWDOG_IRQ			BIT(0)

/* SMBB types */
#define SMBB				BIT(1)
#define SMBBP				BIT(2)
#define SMBCL				BIT(3)

/* Workaround flags */
#define CHG_FLAGS_VCP_WA		BIT(0)
#define BOOST_FLASH_WA			BIT(1)

struct qpnp_chg_irq {
	unsigned int		irq;
	unsigned long		disabled;
};

struct qpnp_chg_regulator {
	struct regulator_desc			rdesc;
	struct regulator_dev			*rdev;
};

#if defined(FEATURE_PANTECH_PMIC_BATTERY_CHARGING_DISCHARGING_TEST) || defined(FEATURE_PANTECH_PMIC_MONITOR_TEST)
#define PM8941_CHARER_IOCTL_MAGIC 'p'

#if defined(FEATURE_PANTECH_PMIC_BATTERY_CHARGING_DISCHARGING_TEST)
#define PM8941_CHARER_TEST_SET_CHARING_CTL        _IOW(PM8941_CHARER_IOCTL_MAGIC, 1, unsigned)
#define PM8941_CHARER_TEST_GET_CHARING_CTL        _IOW(PM8941_CHARER_IOCTL_MAGIC, 2, unsigned)
#endif

#if !defined(FEATURE_AARM_RELEASE_MODE)
#define PANTECH_USED_ALL_OTHER_WRITABLE_FILES
#endif

#if defined(FEATURE_PANTECH_PMIC_MONITOR_TEST)
#define PM8941_CHARGER_TEST_SET_PM_CHG_TEST	_IOW(PM8941_CHARER_IOCTL_MAGIC, 3, unsigned)
//+++ 20130806, P14787, djjeon, charging setting stability test
#define PM8941_CHARGER_TEST_CHARGING_SETTING	_IOW(PM8941_CHARER_IOCTL_MAGIC, 4, unsigned)
#if defined(CONFIG_PANTECH_PMIC_CHARGER_SMB349) || defined(CONFIG_PANTECH_PMIC_CHARGER_SMB347)
extern void smb349_test_limit_up(int on);
#endif
extern void otg_detect_control_test(int on);
//--- 20130806, P14787, djjeon, charging setting stability test
extern unsigned char smb349_get_reg(unsigned char reg);
#endif
#endif // #if defined(PANTECH_BATTERY_CHARING_DISCHARING_TEST) || defined(PANTECH_CHARGER_MONITOR_TEST)

/**
 * struct qpnp_chg_chip - device information
 * @dev:			device pointer to access the parent
 * @spmi:			spmi pointer to access spmi information
 * @chgr_base:			charger peripheral base address
 * @buck_base:			buck  peripheral base address
 * @bat_if_base:		battery interface  peripheral base address
 * @usb_chgpth_base:		USB charge path peripheral base address
 * @dc_chgpth_base:		DC charge path peripheral base address
 * @boost_base:			boost peripheral base address
 * @misc_base:			misc peripheral base address
 * @freq_base:			freq peripheral base address
 * @bat_is_cool:		indicates that battery is cool
 * @bat_is_warm:		indicates that battery is warm
 * @chg_done:			indicates that charging is completed
 * @usb_present:		present status of usb
 * @dc_present:			present status of dc
 * @batt_present:		present status of battery
 * @use_default_batt_values:	flag to report default battery properties
 * @btc_disabled		Flag to disable btc (disables hot and cold irqs)
 * @max_voltage_mv:		the max volts the batt should be charged up to
 * @min_voltage_mv:		min battery voltage before turning the FET on
 * @max_bat_chg_current:	maximum battery charge current in mA
 * @warm_bat_chg_ma:	warm battery maximum charge current in mA
 * @cool_bat_chg_ma:	cool battery maximum charge current in mA
 * @warm_bat_mv:		warm temperature battery target voltage
 * @cool_bat_mv:		cool temperature battery target voltage
 * @resume_delta_mv:		voltage delta at which battery resumes charging
 * @term_current:		the charging based term current
 * @safe_current:		battery safety current setting
 * @maxinput_usb_ma:		Maximum Input current USB
 * @maxinput_dc_ma:		Maximum Input current DC
 * @hot_batt_p			Hot battery threshold setting
 * @cold_batt_p			Cold battery threshold setting
 * @warm_bat_decidegc		Warm battery temperature in degree Celsius
 * @cool_bat_decidegc		Cool battery temperature in degree Celsius
 * @revision:			PMIC revision
 * @type:			SMBB type
 * @tchg_mins			maximum allowed software initiated charge time
 * @thermal_levels		amount of thermal mitigation levels
 * @thermal_mitigation		thermal mitigation level values
 * @therm_lvl_sel		thermal mitigation level selection
 * @dc_psy			power supply to export information to userspace
 * @usb_psy			power supply to export information to userspace
 * @bms_psy			power supply to export information to userspace
 * @batt_psy:			power supply to export information to userspace
 * @flags:			flags to activate specific workarounds
 *				throughout the driver
 *
 */
struct qpnp_chg_chip {
	struct device			*dev;
	struct spmi_device		*spmi;
	u16				chgr_base;
	u16				buck_base;
	u16				bat_if_base;
	u16				usb_chgpth_base;
	u16				dc_chgpth_base;
	u16				boost_base;
	u16				misc_base;
	u16				freq_base;
#if defined(FEATURE_PANTECH_PMIC_USBIN_DROP_WORKAROUND)
	struct qpnp_chg_irq		sysok_valid;
#endif
	struct qpnp_chg_irq		usbin_valid;
	struct qpnp_chg_irq		dcin_valid;
	struct qpnp_chg_irq		chg_gone;
	struct qpnp_chg_irq		chg_fastchg;
	struct qpnp_chg_irq		chg_trklchg;
	struct qpnp_chg_irq		chg_failed;
	struct qpnp_chg_irq		chg_vbatdet_lo;
	struct qpnp_chg_irq		batt_pres;
	struct qpnp_chg_irq		batt_temp_ok;
	bool				bat_is_cool;
	bool				bat_is_warm;
	bool				chg_done;
	bool				usb_present;
	bool				dc_present;
	bool				batt_present;
	bool				charging_disabled;
	bool				btc_disabled;
	bool				use_default_batt_values;
	bool				duty_cycle_100p;
	unsigned int			bpd_detection;
	unsigned int			max_bat_chg_current;
	unsigned int			warm_bat_chg_ma;
	unsigned int			cool_bat_chg_ma;
	unsigned int			safe_voltage_mv;
	unsigned int			max_voltage_mv;
	unsigned int			min_voltage_mv;
	int				set_vddmax_mv;
	int				delta_vddmax_mv;
	unsigned int			warm_bat_mv;
	unsigned int			cool_bat_mv;
	unsigned int			resume_delta_mv;
	int				term_current;
	int				soc_resume_limit;
	bool				resuming_charging;
	unsigned int			maxinput_usb_ma;
	unsigned int			maxinput_dc_ma;
	unsigned int			hot_batt_p;
	unsigned int			cold_batt_p;
	int				warm_bat_decidegc;
	int				cool_bat_decidegc;
	unsigned int			safe_current;
	unsigned int			revision;
	unsigned int			type;
	unsigned int			tchg_mins;
	unsigned int			thermal_levels;
	unsigned int			therm_lvl_sel;
	unsigned int			*thermal_mitigation;
	struct power_supply		dc_psy;
	struct power_supply		*usb_psy;
	struct power_supply		*bms_psy;
	struct power_supply		batt_psy;
	uint32_t			flags;
	struct qpnp_adc_tm_btm_param	adc_param;
	struct work_struct		adc_measure_work;
#if defined(FEATURE_PANTECH_PMIC_PHYSICAL_DROP)
	struct delayed_work		drop_work;
#endif
	struct delayed_work		arb_stop_work;
	struct delayed_work		eoc_work;
	struct work_struct		soc_check_work;
	struct wake_lock		eoc_wake_lock;
	struct qpnp_chg_regulator	otg_vreg;
	struct qpnp_chg_regulator	boost_vreg;
#if defined(CONFIG_PANTECH_PMIC_CHARGER_SMB349) || defined(CONFIG_PANTECH_PMIC_CHARGER_SMB347)
	int battery_id_adc; 
#if defined(FEATURE_PANTECH_PMIC_EOC)
	bool end_recharing;
#endif 
#endif 
#if defined(FEATURE_PANTECH_PMIC_BMS_TEST)
	int current_uvolt;
	int current_temp;
	int soc;
	int charge_output_voltage;		//20130521 djjeon, powerlog add ICHG
#endif 
#if defined(FEATURE_PANTECH_PMIC_MONITOR_TEST)
	bool pm_chg_test;
	u64 cable_adc;
	bool charging_setting;//+++ 20130806, P14787, djjeon, charging setting stability test
#endif
#if defined(CONFIG_PANTECH_PMIC_CHARGER_SMB349) || defined(CONFIG_PANTECH_PMIC_CHARGER_SMB347)
	   struct delayed_work		update_heartbeat_work;
       struct wake_lock		    smb349_cable_wake_lock;
#endif
#if defined (FEATURE_PANTECH_PMIC_ABNORMAL)
	int     check_cable_disconnect_work;
	struct  delayed_work 		update_abnormal_cable_disconeect_work;
	struct  delayed_work		update_abnormal_work;
#endif
};


static struct of_device_id qpnp_charger_match_table[] = {
	{ .compatible = QPNP_CHARGER_DEV_NAME, },
	{}
};

enum bpd_type {
	BPD_TYPE_BAT_ID,
	BPD_TYPE_BAT_THM,
	BPD_TYPE_BAT_THM_BAT_ID,
};

static const char * const bpd_label[] = {
	[BPD_TYPE_BAT_ID] = "bpd_id",
	[BPD_TYPE_BAT_THM] = "bpd_thm",
	[BPD_TYPE_BAT_THM_BAT_ID] = "bpd_thm_id",
};

enum btc_type {
	HOT_THD_25_PCT = 25,
	HOT_THD_35_PCT = 35,
	COLD_THD_70_PCT = 70,
	COLD_THD_80_PCT = 80,
};

#if 0 //temp block for build
static u8 btc_value[] = {
	[HOT_THD_25_PCT] = 0x0,
	[HOT_THD_35_PCT] = BIT(0),
	[COLD_THD_70_PCT] = 0x0,
	[COLD_THD_80_PCT] = BIT(1),
};
#endif

static inline int
get_bpd(const char *name)
{
	int i = 0;
	for (i = 0; i < ARRAY_SIZE(bpd_label); i++) {
		if (strcmp(bpd_label[i], name) == 0)
			return i;
	}
	return -EINVAL;
}

#if defined(CONFIG_QPNP_CHARGER)
struct qpnp_chg_chip *the_qpnp_chip;
//static int pm_hw_init_flag = 0; 
#endif

#if defined(FEATURE_PANTECH_PMIC_BMS_TEST)
static struct input_dev *bms_input_dev;
static struct platform_device *bms_input_attr_dev;
static atomic_t bms_input_flag;
static atomic_t bms_cutoff_flag;
#endif

#if defined(FEATURE_PANTECH_PMIC_MONITOR_TEST)
struct proc_dir_entry *pm8941_charger_dir;
#endif
#if defined(CONFIG_PANTECH_PMIC_CHARGER_SMB349) || defined(CONFIG_PANTECH_PMIC_CHARGER_SMB347)
extern int prev_state_mode; 
static int64_t batt_temp = -100;
#endif

#if defined(FEATURE_PANTECH_PMIC_CHARGE_LIMIT_WITH_TSENSOR)
extern int T_sensor_mode; 
extern int t_prev_mode;
extern int t_sensor_value(void);
extern void smb349_t_sense_limit(int mode);
#endif
#if defined (FEATURE_PANTECH_PMIC_ABNORMAL)
static bool abnormal_state = false;
#endif

#if defined(FEATURE_POWER_ON_OFF_TEST) // 20130524. MKS.
struct pmic_pwrkey_emulation {
	struct input_dev *pwr;
	struct delayed_work power_key_emulation_work;
};

static struct pmic_pwrkey_emulation *pwr_test;
static struct kobject *kobj_pwr_test;
static int pwr_on_trigger;
static u32 pwrkey_delay_ms;
static bool is_offline_charging_mode;
static int power_worker_count=0;
#endif

static int charger_probe = 0;
#if defined (FEATURE_PANTECH_PMIC_ABNORMAL)
extern int composite_get_udc_state(char *udc_state);

#if defined(FEATURE_PANTECH_PMIC_USBIN_DROP_WORKAROUND)
#define  SC_SYSOK  5
static struct gpiomux_setting sc_sysok_sleep_config = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_IN,
};
static struct gpiomux_setting sc_sysok_active_config = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE ,
	.dir = GPIOMUX_IN,
};

static struct msm_gpiomux_config sc_sysok = {
	.gpio = SC_SYSOK,
	.settings = {
		[GPIOMUX_ACTIVE]    = &sc_sysok_active_config,
		[GPIOMUX_SUSPENDED] = &sc_sysok_sleep_config,
	},
};
#endif

char usb_conf_buf[128];

char* usb_composite_get_udc_state(void)
{
	char conf_buf[128] = {'\0',};
	int ret = 0;

	ret = composite_get_udc_state(conf_buf);
	strcpy(usb_conf_buf, conf_buf);

	return usb_conf_buf;
}    
#endif
#if defined(CONFIG_PANTECH_PMIC_CHARGER_SMB349) || defined(CONFIG_PANTECH_PMIC_CHARGER_SMB347)
#if (defined(T_EF56S) || defined(T_EF57K) || defined(T_EF58L))&& (BOARD_VER <= WS20) 
#define FACTORY_DUMMY_ID_MIN  1310
#define FACTORY_DUMMY_ID_MAX 1420
#elif defined(T_EF56S) || defined(T_EF57K) || defined(T_EF58L) || defined(T_EF59S) ||  defined(T_EF59K) || defined(T_EF59L) || defined(T_EF60S) || defined(T_EF61K) || defined(T_EF62L)
#define FACTORY_DUMMY_ID_MIN  930
#define FACTORY_DUMMY_ID_MAX 1230
#endif
static int get_battery_id_adc(void)
{
	int64_t rc;
	int battery_id_adc;
	struct qpnp_vadc_result results;

	if (!the_qpnp_chip) {
		pr_err("the_chip is NULL\n");
		return 0;
	}

	rc = qpnp_vadc_read(LR_MUX2_BAT_ID, &results); 
	battery_id_adc = results.physical;
	
	pr_err("results.physica %d\n", battery_id_adc/1000);
	
	return battery_id_adc / 1000;

}

int is_fatctory_dummy_connect(void)
{
	int factory_check = 0;

	// Added CODE (by skkim p14200@LS1)
	if (!the_qpnp_chip) {
		pr_err("the_chip is NULL\n");
		return 0;
	}

	if ((the_qpnp_chip->battery_id_adc > FACTORY_DUMMY_ID_MIN) && (the_qpnp_chip->battery_id_adc  < FACTORY_DUMMY_ID_MAX)){
       	factory_check = 1;
		pr_debug("factory dummy connected\n");
	}
	is_factory_dummy = factory_check;
	
	return factory_check;

}
EXPORT_SYMBOL_GPL(is_fatctory_dummy_connect);
#endif
#if defined (FEATURE_PANTECH_USB_BLOCKING_MDMSTATE)
extern void get_mdm_state(char *mdm_state);

char usb_mdm_buf[128];

char* usb_get_mdm_state(void)
{
	char conf_buf[128] = {'\0',};

	get_mdm_state(conf_buf);
	strcpy(usb_mdm_buf, conf_buf);

	return usb_mdm_buf;
}    

#endif
static int
qpnp_chg_read(struct qpnp_chg_chip *chip, u8 *val,
			u16 base, int count)
{
	int rc = 0;
	struct spmi_device *spmi = chip->spmi;

	if (base == 0) {
		pr_err("base cannot be zero base=0x%02x sid=0x%02x rc=%d\n",
			base, spmi->sid, rc);
		return -EINVAL;
	}

	rc = spmi_ext_register_readl(spmi->ctrl, spmi->sid, base, val, count);
	if (rc) {
		pr_err("SPMI read failed base=0x%02x sid=0x%02x rc=%d\n", base,
				spmi->sid, rc);
		return rc;
	}
	return 0;
}

static int
qpnp_chg_write(struct qpnp_chg_chip *chip, u8 *val,
			u16 base, int count)
{
	int rc = 0;
	struct spmi_device *spmi = chip->spmi;

	if (base == 0) {
		pr_err("base cannot be zero base=0x%02x sid=0x%02x rc=%d\n",
			base, spmi->sid, rc);
		return -EINVAL;
	}

	rc = spmi_ext_register_writel(spmi->ctrl, spmi->sid, base, val, count);
	if (rc) {
		pr_err("write failed base=0x%02x sid=0x%02x rc=%d\n",
			base, spmi->sid, rc);
		return rc;
	}

	return 0;
}

static int
qpnp_chg_masked_write(struct qpnp_chg_chip *chip, u16 base,
						u8 mask, u8 val, int count)
{
	int rc;
	u8 reg;

	rc = qpnp_chg_read(chip, &reg, base, count);
	if (rc) {
		pr_err("spmi read failed: addr=%03X, rc=%d\n", base, rc);
		return rc;
	}
	pr_debug("addr = 0x%x read 0x%x\n", base, reg);

	reg &= ~mask;
	reg |= val & mask;

	pr_debug("Writing 0x%x\n", reg);

	rc = qpnp_chg_write(chip, &reg, base, count);
	if (rc) {
		pr_err("spmi write failed: addr=%03X, rc=%d\n", base, rc);
		return rc;
	}

	return 0;
}

static void
qpnp_chg_enable_irq(struct qpnp_chg_irq *irq)
{
	if (__test_and_clear_bit(0, &irq->disabled)) {
		pr_debug("number = %d\n", irq->irq);
		enable_irq(irq->irq);
	}
}

static void
qpnp_chg_disable_irq(struct qpnp_chg_irq *irq)
{
	if (!__test_and_set_bit(0, &irq->disabled)) {
		pr_debug("number = %d\n", irq->irq);
		disable_irq_nosync(irq->irq);
	}
}

#define USB_OTG_EN_BIT	BIT(0)
static int
qpnp_chg_is_otg_en_set(struct qpnp_chg_chip *chip)
{
	u8 usb_otg_en;
	int rc;

	rc = qpnp_chg_read(chip, &usb_otg_en,
				 chip->usb_chgpth_base + CHGR_USB_USB_OTG_CTL,
				 1);

	if (rc) {
		pr_err("spmi read failed: addr=%03X, rc=%d\n",
				chip->usb_chgpth_base + CHGR_STATUS, rc);
		return rc;
	}
	pr_debug("usb otg en 0x%x\n", usb_otg_en);

	return (usb_otg_en & USB_OTG_EN_BIT) ? 1 : 0;
}

static int
qpnp_chg_is_boost_en_set(struct qpnp_chg_chip *chip)
{
	u8 boost_en_ctl;
	int rc;

	rc = qpnp_chg_read(chip, &boost_en_ctl,
		chip->boost_base + BOOST_ENABLE_CONTROL, 1);
	if (rc) {
		pr_err("spmi read failed: addr=%03X, rc=%d\n",
				chip->boost_base + BOOST_ENABLE_CONTROL, rc);
		return rc;
	}

	pr_debug("boost en 0x%x\n", boost_en_ctl);

	return (boost_en_ctl & BOOST_PWR_EN) ? 1 : 0;
}

static int
qpnp_chg_is_batt_temp_ok(struct qpnp_chg_chip *chip)
{
	u8 batt_rt_sts;
	int rc;

	rc = qpnp_chg_read(chip, &batt_rt_sts,
				 INT_RT_STS(chip->bat_if_base), 1);
	if (rc) {
		pr_err("spmi read failed: addr=%03X, rc=%d\n",
				INT_RT_STS(chip->bat_if_base), rc);
		return rc;
	}

	return (batt_rt_sts & BAT_TEMP_OK_IRQ) ? 1 : 0;
}

static int
qpnp_chg_is_batt_present(struct qpnp_chg_chip *chip)
{
	u8 batt_pres_rt_sts;
	int rc;
	
#if defined(CONFIG_PANTECH_PMIC_CHARGER_SMB349) || defined(CONFIG_PANTECH_PMIC_CHARGER_SMB347)
	if(is_fatctory_dummy_connect())
		return 1;
#endif
	rc = qpnp_chg_read(chip, &batt_pres_rt_sts,
				 INT_RT_STS(chip->bat_if_base), 1);
	if (rc) {
		pr_err("spmi read failed: addr=%03X, rc=%d\n",
				INT_RT_STS(chip->bat_if_base), rc);
		return rc;
	}

	return (batt_pres_rt_sts & BATT_PRES_IRQ) ? 1 : 0;
}

static int
qpnp_chg_is_batfet_closed(struct qpnp_chg_chip *chip)
{
	u8 batfet_closed_rt_sts;
	int rc;

	rc = qpnp_chg_read(chip, &batfet_closed_rt_sts,
				 INT_RT_STS(chip->bat_if_base), 1);
	if (rc) {
		pr_err("spmi read failed: addr=%03X, rc=%d\n",
				INT_RT_STS(chip->bat_if_base), rc);
		return rc;
	}

	return (batfet_closed_rt_sts & BAT_FET_ON_IRQ) ? 1 : 0;
}

// added by skkim (p14200@LS1;2012.08.27)
#if defined(FEATURE_PANTECH_PMIC_PHYSICAL_DROP)
static int
qpnp_chg_is_temp_present(struct qpnp_chg_chip *chip)
{
	u8 temp_pres_rt_sts;
	int rc;
	
#if defined(CONFIG_PANTECH_PMIC_CHARGER_SMB349) || defined(CONFIG_PANTECH_PMIC_CHARGER_SMB347)
	if(is_fatctory_dummy_connect()) {
		return 1;
	}
#endif
	rc = qpnp_chg_read(chip, &temp_pres_rt_sts,
				chip->bat_if_base + CHGR_BAT_IF_PRES_STATUS, 1);
	if (rc) {
		pr_err("spmi read failed: addr=%03X, rc=%d\n",
				INT_RT_STS(chip->bat_if_base), rc);
		return rc;
	}
	return (int) temp_pres_rt_sts;
}
#endif


#define USB_VALID_BIT	BIT(7)
static int
qpnp_chg_is_usb_chg_plugged_in(struct qpnp_chg_chip *chip)
{
#if defined(FEATURE_PANTECH_PMIC_USBIN_DROP_WORKAROUND)
	return gpio_get_value(SC_SYSOK) == 0 ? 1 : 0;
#else
	u8 usbin_valid_rt_sts;
	int rc;

	rc = qpnp_chg_read(chip, &usbin_valid_rt_sts,
				 chip->usb_chgpth_base + CHGR_STATUS , 1);

	if (rc) {
		pr_err("spmi read failed: addr=%03X, rc=%d\n",
				chip->usb_chgpth_base + CHGR_STATUS, rc);
		return rc;
	}
	pr_debug("chgr usb sts 0x%x\n", usbin_valid_rt_sts);

	return (usbin_valid_rt_sts & USB_VALID_BIT) ? 1 : 0;
#endif
}
#if defined(FEATURE_PANTECH_PMIC_USBIN_DROP_WORKAROUND)
static int
qpnp_chg_is_usb_valid_plugged_in(struct qpnp_chg_chip *chip)
{
	u8 usbin_valid_rt_sts;
	int rc;

	rc = qpnp_chg_read(chip, &usbin_valid_rt_sts,
				 chip->usb_chgpth_base + CHGR_STATUS , 1);

	if (rc) {
		pr_err("spmi read failed: addr=%03X, rc=%d\n",
				chip->usb_chgpth_base + CHGR_STATUS, rc);
		return rc;
	}
	pr_debug("chgr usb sts 0x%x\n", usbin_valid_rt_sts);

	return (usbin_valid_rt_sts & USB_VALID_BIT) ? 1 : 0;
}
#endif
static int
qpnp_chg_is_dc_chg_plugged_in(struct qpnp_chg_chip *chip)
{
	u8 dcin_valid_rt_sts;
	int rc;

	if (!chip->dc_chgpth_base)
		return 0;

	rc = qpnp_chg_read(chip, &dcin_valid_rt_sts,
				 INT_RT_STS(chip->dc_chgpth_base), 1);
	if (rc) {
		pr_err("spmi read failed: addr=%03X, rc=%d\n",
				INT_RT_STS(chip->dc_chgpth_base), rc);
		return rc;
	}

	return (dcin_valid_rt_sts & DCIN_VALID_IRQ) ? 1 : 0;
}

#define QPNP_CHG_I_MAX_MIN_100		100
#define QPNP_CHG_I_MAX_MIN_150		150
#define QPNP_CHG_I_MAX_MIN_MA		200
#define QPNP_CHG_I_MAX_MAX_MA		2500
#define QPNP_CHG_I_MAXSTEP_MA		100
static int
qpnp_chg_idcmax_set(struct qpnp_chg_chip *chip, int mA)
{
	int rc = 0;
	u8 dc = 0;

	if (mA < QPNP_CHG_I_MAX_MIN_100
			|| mA > QPNP_CHG_I_MAX_MAX_MA) {
		pr_err("bad mA=%d asked to set\n", mA);
		return -EINVAL;
	}

	if (mA == QPNP_CHG_I_MAX_MIN_100) {
		dc = 0x00;
		pr_debug("current=%d setting %02x\n", mA, dc);
		return qpnp_chg_write(chip, &dc,
			chip->dc_chgpth_base + CHGR_I_MAX_REG, 1);
	} else if (mA == QPNP_CHG_I_MAX_MIN_150) {
		dc = 0x01;
		pr_debug("current=%d setting %02x\n", mA, dc);
		return qpnp_chg_write(chip, &dc,
			chip->dc_chgpth_base + CHGR_I_MAX_REG, 1);
	}

	dc = mA / QPNP_CHG_I_MAXSTEP_MA;

	pr_debug("current=%d setting 0x%x\n", mA, dc);
	rc = qpnp_chg_write(chip, &dc,
		chip->dc_chgpth_base + CHGR_I_MAX_REG, 1);

	return rc;
}
#if defined(FEATURE_PANTECH_PMIC_BMS_TEST)

static int get_batt_mvolts(void);
static int get_batt_chg_current(struct qpnp_chg_chip *chip);	//20130521 djjeon, powerlog add ICHG
#endif

static int
qpnp_chg_iusbmax_set(struct qpnp_chg_chip *chip, int mA)
{
	int rc = 0;
	u8 usb_reg = 0, temp = 8;
#if 0//defined(CONFIG_PANTECH_BMS_TEST)	// for TEST (p14200@LS1)
	int enable, soc=-1;
#endif
#if defined(CONFIG_QPNP_CHARGER)
        if( mA > 1000 )
        {
#if (defined(T_EF59S) ||  defined(T_EF59K) || defined(T_EF59L)) && (BOARD_VER >= TP10)
                mA = 1800 ;
#else
                mA = 2000 ;
#endif
 				
        }
		
	 the_qpnp_chip->maxinput_usb_ma = mA;
	 
        pr_debug("sayuss usb mA =%d\n",mA);
#if 0// defined(CONFIG_PANTECH_BMS_TEST)	// for TEST (p14200@LS1)
	enable = atomic_read(&bms_input_flag);
	soc = max17058_get_soc();//pm8921_bms_get_percent(max17058_uses);
	if (enable) {
		input_report_rel(bms_input_dev, REL_RX, soc);
		input_report_rel(bms_input_dev, REL_RY, get_batt_mvolts());
		input_report_rel(bms_input_dev, REL_RZ, batt_temp);
		input_sync(bms_input_dev);
	}

#endif
#endif
	if (mA < QPNP_CHG_I_MAX_MIN_100
			|| mA > QPNP_CHG_I_MAX_MAX_MA) {
		pr_err("bad mA=%d asked to set\n", mA);
		return -EINVAL;
	}

	if (mA == QPNP_CHG_I_MAX_MIN_100) {
		usb_reg = 0x00;
		pr_debug("current=%d setting %02x\n", mA, usb_reg);
		return qpnp_chg_write(chip, &usb_reg,
		chip->usb_chgpth_base + CHGR_I_MAX_REG, 1);
	} else if (mA == QPNP_CHG_I_MAX_MIN_150) {
		usb_reg = 0x01;
		pr_debug("current=%d setting %02x\n", mA, usb_reg);
		return qpnp_chg_write(chip, &usb_reg,
		chip->usb_chgpth_base + CHGR_I_MAX_REG, 1);
	}

	/* Impose input current limit */
	if (chip->maxinput_usb_ma)
		mA = (chip->maxinput_usb_ma) <= mA ? chip->maxinput_usb_ma : mA;

	usb_reg = mA / QPNP_CHG_I_MAXSTEP_MA;

	if (chip->flags & CHG_FLAGS_VCP_WA) {
		temp = 0xA5;
		rc =  qpnp_chg_write(chip, &temp,
			chip->buck_base + SEC_ACCESS, 1);
		rc =  qpnp_chg_masked_write(chip,
			chip->buck_base + CHGR_BUCK_COMPARATOR_OVRIDE_3,
			0x0C, 0x0C, 1);
	}

	pr_debug("current=%d setting 0x%x\n", mA, usb_reg);
	rc = qpnp_chg_write(chip, &usb_reg,
		chip->usb_chgpth_base + CHGR_I_MAX_REG, 1);

	if (chip->flags & CHG_FLAGS_VCP_WA) {
		temp = 0xA5;
		udelay(200);
		rc =  qpnp_chg_write(chip, &temp,
			chip->buck_base + SEC_ACCESS, 1);
		rc =  qpnp_chg_masked_write(chip,
			chip->buck_base + CHGR_BUCK_COMPARATOR_OVRIDE_3,
			0x0C, 0x00, 1);
	}

	return rc;
}

#define USB_SUSPEND_BIT	BIT(0)
static int
qpnp_chg_usb_suspend_enable(struct qpnp_chg_chip *chip, int enable)
{
	return qpnp_chg_masked_write(chip,
			chip->usb_chgpth_base + CHGR_USB_USB_SUSP,
			USB_SUSPEND_BIT,
			enable ? USB_SUSPEND_BIT : 0, 1);
}

static int
qpnp_chg_charge_en(struct qpnp_chg_chip *chip, int enable)
{
	return qpnp_chg_masked_write(chip, chip->chgr_base + CHGR_CHG_CTRL,
			CHGR_CHG_EN,
			enable ? CHGR_CHG_EN : 0, 1);
}

static int
qpnp_chg_force_run_on_batt(struct qpnp_chg_chip *chip, int disable)
{
	/* Don't run on battery for batteryless hardware */
	if (chip->use_default_batt_values)
		return 0;
	/* Don't force on battery if battery is not present */
	if (!qpnp_chg_is_batt_present(chip))
		return 0;

	/* This bit forces the charger to run off of the battery rather
	 * than a connected charger */
	return qpnp_chg_masked_write(chip, chip->chgr_base + CHGR_CHG_CTRL,
			CHGR_ON_BAT_FORCE_BIT,
			disable ? CHGR_ON_BAT_FORCE_BIT : 0, 1);
}

#define BUCK_DUTY_MASK_100P	0x30
static int
qpnp_buck_set_100_duty_cycle_enable(struct qpnp_chg_chip *chip, int enable)
{
	int rc;

	pr_debug("enable: %d\n", enable);

	rc = qpnp_chg_masked_write(chip,
		chip->buck_base + SEC_ACCESS, 0xA5, 0xA5, 1);
	if (rc) {
		pr_debug("failed to write sec access rc=%d\n", rc);
		return rc;
	}

	rc = qpnp_chg_masked_write(chip,
		chip->buck_base + BUCK_TEST_SMBC_MODES,
			BUCK_DUTY_MASK_100P, enable ? 0x00 : 0x10, 1);
	if (rc) {
		pr_debug("failed enable 100p duty cycle rc=%d\n", rc);
		return rc;
	}

	return rc;
}

#define COMPATATOR_OVERRIDE_0	0x80
static int
qpnp_chg_toggle_chg_done_logic(struct qpnp_chg_chip *chip, int enable)
{
	int rc;

	pr_debug("toggle: %d\n", enable);

	rc = qpnp_chg_masked_write(chip,
		chip->buck_base + SEC_ACCESS, 0xA5, 0xA5, 1);
	if (rc) {
		pr_debug("failed to write sec access rc=%d\n", rc);
		return rc;
	}

	rc = qpnp_chg_masked_write(chip,
		chip->buck_base + CHGR_BUCK_COMPARATOR_OVRIDE_1,
			0xC0, enable ? 0x00 : COMPATATOR_OVERRIDE_0, 1);
	if (rc) {
		pr_debug("failed to toggle chg done override rc=%d\n", rc);
		return rc;
	}

	return rc;
}

#define QPNP_CHG_VBATDET_MIN_MV	3240
#define QPNP_CHG_VBATDET_MAX_MV	5780
#define QPNP_CHG_VBATDET_STEP_MV	20
static int
qpnp_chg_vbatdet_set(struct qpnp_chg_chip *chip, int vbatdet_mv)
{
	u8 temp;

	if (vbatdet_mv < QPNP_CHG_VBATDET_MIN_MV
			|| vbatdet_mv > QPNP_CHG_VBATDET_MAX_MV) {
		pr_err("bad mV=%d asked to set\n", vbatdet_mv);
		return -EINVAL;
	}
	temp = (vbatdet_mv - QPNP_CHG_VBATDET_MIN_MV)
			/ QPNP_CHG_VBATDET_STEP_MV;

	pr_debug("voltage=%d setting %02x\n", vbatdet_mv, temp);
	return qpnp_chg_write(chip, &temp,
		chip->chgr_base + CHGR_VBAT_DET, 1);
}

static void
qpnp_chg_set_appropriate_vbatdet(struct qpnp_chg_chip *chip)
{
	if (chip->bat_is_cool)
		qpnp_chg_vbatdet_set(chip, chip->cool_bat_mv
			- chip->resume_delta_mv);
	else if (chip->bat_is_warm)
		qpnp_chg_vbatdet_set(chip, chip->warm_bat_mv
			- chip->resume_delta_mv);
	else if (chip->resuming_charging)
		qpnp_chg_vbatdet_set(chip, chip->max_voltage_mv
			+ chip->resume_delta_mv);
	else
		qpnp_chg_vbatdet_set(chip, chip->max_voltage_mv
			- chip->resume_delta_mv);
}

static void
qpnp_arb_stop_work(struct work_struct *work)
{
	struct delayed_work *dwork = to_delayed_work(work);
	struct qpnp_chg_chip *chip = container_of(dwork,
				struct qpnp_chg_chip, arb_stop_work);

	if (!chip->chg_done)
	qpnp_chg_charge_en(chip, !chip->charging_disabled);
	qpnp_chg_force_run_on_batt(chip, chip->charging_disabled);
}

static void
qpnp_bat_if_adc_measure_work(struct work_struct *work)
{
	struct qpnp_chg_chip *chip = container_of(work,
				struct qpnp_chg_chip, adc_measure_work);

	if (qpnp_adc_tm_channel_measure(&chip->adc_param))
		pr_err("request ADC error\n");
}

#if defined(CONFIG_PANTECH_PMIC_CHARGER_SMB349) || defined(CONFIG_PANTECH_PMIC_CHARGER_SMB347)
static int qpnp_get_cable_type(void)
{	
	union power_supply_propval ret = {0,};
	the_qpnp_chip->usb_psy->get_property(the_qpnp_chip->usb_psy,
		POWER_SUPPLY_PROP_TYPE, &ret);
#if defined(CONFIG_PANTECH_PMIC_CHARGER_SMB349)
	// sayuss 2013.09.05 CDP issue
	if(ret.intval == 6) is_CDP = 1;
	else is_CDP = 0;
#endif	
	if(ret.intval == 4)
		return PANTECH_USB;
	else if(4 < ret.intval && 7 > ret.intval)
		return PANTECH_AC;
	else 
		return PANTECH_CABLE_NONE;
}
static void qpnp_set_cable_type(int type)
{	
	union power_supply_propval ret = {type,};
	the_qpnp_chip->usb_psy->set_property(the_qpnp_chip->usb_psy,
		POWER_SUPPLY_PROP_TYPE, &ret);		
}
void smb349_mode_setting(void)
{
	if(qpnp_get_cable_type() == PANTECH_USB)
		smb349_write_reg(0x31,0x02); //usb mode
	else
		smb349_write_reg(0x31,0x01); //hc mode
}
#endif
#if defined(FEATURE_PANTECH_PMIC_PHYSICAL_DROP)
static void
qpnp_bat_if_drop_work(struct work_struct *work)
{
	struct delayed_work *dwork = to_delayed_work(work);
	struct qpnp_chg_chip *chip = container_of(dwork,
				struct qpnp_chg_chip, drop_work);
	int batt_present, temp_present;

	batt_present = qpnp_chg_is_batt_present(chip);
	pr_err("[DROP] batt-pres : %d\n", batt_present);
	temp_present = qpnp_chg_is_temp_present(chip);
	pr_err("[DROP] temp-pres : %d\n", temp_present);

	if (chip->batt_present ^ batt_present) {
		pr_err("[DEBUG;drop_wrok] (chip->batt_present ^ batt_present) IN\n");
		chip->batt_present = batt_present;
		power_supply_changed(&chip->batt_psy);
		power_supply_changed(chip->usb_psy);

		if (chip->cool_bat_decidegc && chip->warm_bat_decidegc
						&& batt_present) {
			schedule_work(&chip->adc_measure_work);
		}
	}

#if defined(CONFIG_PANTECH_PMIC_CHARGER_SMB349) || defined(CONFIG_PANTECH_PMIC_CHARGER_SMB347)
	if (!temp_present) {
	   	 pr_err("msm_restart#2 batt_present %d\n", chip->batt_present);
          	msm_restart(0xC9,0);
	}
#endif
}
#endif


#define EOC_CHECK_PERIOD_MS	10000
static irqreturn_t
qpnp_chg_vbatdet_lo_irq_handler(int irq, void *_chip)
{
	struct qpnp_chg_chip *chip = _chip;
	u8 chg_sts = 0;
	int rc;

	pr_debug("vbatdet-lo triggered\n");

	rc = qpnp_chg_read(chip, &chg_sts, INT_RT_STS(chip->chgr_base), 1);
	if (rc)
		pr_err("failed to read chg_sts rc=%d\n", rc);

	pr_debug("chg_done chg_sts: 0x%x triggered\n", chg_sts);
	if (!chip->charging_disabled && (chg_sts & FAST_CHG_ON_IRQ)) {
		schedule_delayed_work(&chip->eoc_work,
			msecs_to_jiffies(EOC_CHECK_PERIOD_MS));
		wake_lock(&chip->eoc_wake_lock);
		qpnp_chg_disable_irq(&chip->chg_vbatdet_lo);
	} else {
		qpnp_chg_charge_en(chip, !chip->charging_disabled);
	}
/*20130813, djjeon, TA cable disconnect problem.(offline charging)
	power_supply_changed(chip->usb_psy);
	if (chip->dc_chgpth_base)
		power_supply_changed(&chip->dc_psy);
	if (chip->bat_if_base)
		power_supply_changed(&chip->batt_psy);
*/
	return IRQ_HANDLED;
}

#ifndef FEATURE_PANTECH_PMIC_BLOCK_CHG_GONE
#define ARB_STOP_WORK_MS	1000
static irqreturn_t
qpnp_chg_usb_chg_gone_irq_handler(int irq, void *_chip)
{
	struct qpnp_chg_chip *chip = _chip;

	pr_debug("chg_gone triggered\n");
	if (qpnp_chg_is_usb_chg_plugged_in(chip)) {
		qpnp_chg_charge_en(chip, 0);
		qpnp_chg_force_run_on_batt(chip, 1);
		schedule_delayed_work(&chip->arb_stop_work,
			msecs_to_jiffies(ARB_STOP_WORK_MS));
	}

	return IRQ_HANDLED;
}
#endif

#define ENUM_T_STOP_BIT		BIT(0)
static irqreturn_t
qpnp_chg_usb_usbin_valid_irq_handler(int irq, void *_chip)
{
	struct qpnp_chg_chip *chip = _chip;
	int usb_present, host_mode;
#if defined(FEATURE_PANTECH_PMIC_USBIN_DROP_WORKAROUND)
	usb_present = qpnp_chg_is_usb_valid_plugged_in(chip);
#else
	usb_present = qpnp_chg_is_usb_chg_plugged_in(chip);
#endif
	host_mode = qpnp_chg_is_otg_en_set(chip);
	pr_err("usbin-valid triggered: %d host_mode: %d chip->usb_present [%d]\n",
		usb_present, host_mode,chip->usb_present );

	/* In host mode notifications cmoe from USB supply */
	if (host_mode)
		return IRQ_HANDLED;

	if (chip->usb_present ^ usb_present) {
		chip->usb_present = usb_present;
		if (!usb_present) {
			qpnp_chg_usb_suspend_enable(chip, 1);
			if (!qpnp_chg_is_dc_chg_plugged_in(chip))
			chip->chg_done = false;
		} else {
			schedule_delayed_work(&chip->eoc_work,
				msecs_to_jiffies(EOC_CHECK_PERIOD_MS));
			schedule_work(&chip->soc_check_work);
		}

		power_supply_set_present(chip->usb_psy, chip->usb_present);

		//20130813, djjeon, TA cable disconnect problem.(offline charging)
		if (chip->dc_chgpth_base)
			power_supply_changed(&chip->dc_psy);
	}
#if defined(CONFIG_PANTECH_PMIC_CHARGER_SMB349) || defined(CONFIG_PANTECH_PMIC_CHARGER_SMB347)
	if( chip->usb_present) {
 	 wake_lock(&chip->smb349_cable_wake_lock);
	} else { 
	wake_unlock(&chip->smb349_cable_wake_lock);
#if defined (FEATURE_PANTECH_PMIC_ABNORMAL)
		abnormal_state = false;
#endif
	}
#endif 

	return IRQ_HANDLED;
}

static irqreturn_t
qpnp_chg_bat_if_batt_temp_irq_handler(int irq, void *_chip)
{
	struct qpnp_chg_chip *chip = _chip;
	int batt_temp_good;

	batt_temp_good = qpnp_chg_is_batt_temp_ok(chip);
	pr_debug("batt-temp triggered: %d\n", batt_temp_good);

	power_supply_changed(&chip->batt_psy);
	return IRQ_HANDLED;
}

static irqreturn_t
qpnp_chg_bat_if_batt_pres_irq_handler(int irq, void *_chip)
{
	struct qpnp_chg_chip *chip = _chip;
	int batt_present;
#if defined(FEATURE_PANTECH_PMIC_PHYSICAL_DROP)	
	int temp_present;
#endif

	batt_present = qpnp_chg_is_batt_present(chip);
	pr_debug("batt-pres triggered: %d\n", batt_present);
#if defined(FEATURE_PANTECH_PMIC_PHYSICAL_DROP)
	temp_present = qpnp_chg_is_temp_present(chip);
	pr_debug("temp-pres : %d\n", temp_present);
#endif

	if (chip->batt_present ^ batt_present) {
		chip->batt_present = batt_present;
		power_supply_changed(&chip->batt_psy);
		power_supply_changed(chip->usb_psy);

		if (chip->cool_bat_decidegc && chip->warm_bat_decidegc
						&& batt_present) {
			schedule_work(&chip->adc_measure_work);
	}
	}
#if defined(FEATURE_PANTECH_PMIC_PHYSICAL_DROP)
	if (!batt_present)
	{
		pr_err("[DEBUG] TEST CODE - DROP WORK schedule Setting\n");
		schedule_delayed_work(&chip->drop_work, round_jiffies_relative(msecs_to_jiffies(1000)) );	
	}
#endif

#if defined(CONFIG_PANTECH_PMIC_CHARGER_SMB349) || defined(CONFIG_PANTECH_PMIC_CHARGER_SMB347)
#if defined(FEATURE_PANTECH_PMIC_PHYSICAL_DROP)
	  if (!temp_present) {
#else
       if(!chip->batt_present){
#endif
	      pr_err("msm_restart#1 batt_present %d\n", chip->batt_present);
          msm_restart(0xC9,0);	
	   }
#endif

	return IRQ_HANDLED;
}

static irqreturn_t
qpnp_chg_dc_dcin_valid_irq_handler(int irq, void *_chip)
{
	struct qpnp_chg_chip *chip = _chip;
	int dc_present;

	dc_present = qpnp_chg_is_dc_chg_plugged_in(chip);
	pr_debug("dcin-valid triggered: %d\n", dc_present);

	if (chip->dc_present ^ dc_present) {
		chip->dc_present = dc_present;
		if (!dc_present && !qpnp_chg_is_usb_chg_plugged_in(chip)) {
			chip->chg_done = false;
		} else {
			schedule_delayed_work(&chip->eoc_work,
				msecs_to_jiffies(EOC_CHECK_PERIOD_MS));
			schedule_work(&chip->soc_check_work);
		}
		power_supply_changed(&chip->dc_psy);
		power_supply_changed(&chip->batt_psy);
	}

	return IRQ_HANDLED;
}

#define CHGR_CHG_FAILED_BIT	BIT(7)
static irqreturn_t
qpnp_chg_chgr_chg_failed_irq_handler(int irq, void *_chip)
{
	struct qpnp_chg_chip *chip = _chip;
	int rc;

	pr_debug("chg_failed triggered\n");

	rc = qpnp_chg_masked_write(chip,
		chip->chgr_base + CHGR_CHG_FAILED,
		CHGR_CHG_FAILED_BIT,
		CHGR_CHG_FAILED_BIT, 1);
	if (rc)
		pr_err("Failed to write chg_fail clear bit!\n");

	if (chip->bat_if_base)
		power_supply_changed(&chip->batt_psy);
	power_supply_changed(chip->usb_psy);
	if (chip->dc_chgpth_base)
		power_supply_changed(&chip->dc_psy);
	return IRQ_HANDLED;
}

static irqreturn_t
qpnp_chg_chgr_chg_trklchg_irq_handler(int irq, void *_chip)
{
	struct qpnp_chg_chip *chip = _chip;

	pr_debug("TRKL IRQ triggered\n");

	chip->chg_done = false;
	if (chip->bat_if_base)
	power_supply_changed(&chip->batt_psy);

	return IRQ_HANDLED;
}

static irqreturn_t
qpnp_chg_chgr_chg_fastchg_irq_handler(int irq, void *_chip)
{
	struct qpnp_chg_chip *chip = _chip;

	pr_debug("FAST_CHG IRQ triggered\n");
	chip->chg_done = false;
	if (chip->bat_if_base)
	power_supply_changed(&chip->batt_psy);
	power_supply_changed(chip->usb_psy);
	if (chip->dc_chgpth_base)
		power_supply_changed(&chip->dc_psy);
	if (chip->resuming_charging) {
		chip->resuming_charging = false;
		qpnp_chg_set_appropriate_vbatdet(chip);
	}
	qpnp_chg_enable_irq(&chip->chg_vbatdet_lo);

	return IRQ_HANDLED;
}

static int
qpnp_dc_property_is_writeable(struct power_supply *psy,
						enum power_supply_property psp)
{
	switch (psp) {
	case POWER_SUPPLY_PROP_CURRENT_MAX:
		return 1;
	default:
		break;
	}

	return 0;
}

static int
qpnp_batt_property_is_writeable(struct power_supply *psy,
						enum power_supply_property psp)
{
	switch (psp) {
	case POWER_SUPPLY_PROP_CHARGING_ENABLED:
	case POWER_SUPPLY_PROP_SYSTEM_TEMP_LEVEL:
		return 1;
	default:
		break;
	}

	return 0;
}

static int
qpnp_chg_buck_control(struct qpnp_chg_chip *chip, int enable)
{
	int rc;

	if (chip->charging_disabled && enable) {
		pr_debug("Charging disabled\n");
		return 0;
	}

	rc = qpnp_chg_charge_en(chip, enable);
	if (rc) {
		pr_err("Failed to control charging %d\n", rc);
		return rc;
	}

	rc = qpnp_chg_force_run_on_batt(chip, !enable);
	if (rc)
		pr_err("Failed to control charging %d\n", rc);

	return rc;
}

static int
switch_usb_to_charge_mode(struct qpnp_chg_chip *chip)
{
	int rc;

	pr_debug("switch to charge mode\n");
	if (!qpnp_chg_is_otg_en_set(chip))
		return 0;

	/* enable usb ovp fet */
	rc = qpnp_chg_masked_write(chip,
			chip->usb_chgpth_base + CHGR_USB_USB_OTG_CTL,
			USB_OTG_EN_BIT,
			0, 1);
	if (rc) {
		pr_err("Failed to turn on usb ovp rc = %d\n", rc);
		return rc;
	}

	rc = qpnp_chg_force_run_on_batt(chip, chip->charging_disabled);
	if (rc) {
		pr_err("Failed re-enable charging rc = %d\n", rc);
		return rc;
	}

	return 0;
}

static int
switch_usb_to_host_mode(struct qpnp_chg_chip *chip)
{
	int rc;

	pr_debug("switch to host mode\n");
	if (qpnp_chg_is_otg_en_set(chip))
		return 0;

	rc = qpnp_chg_force_run_on_batt(chip, 1);
	if (rc) {
		pr_err("Failed to disable charging rc = %d\n", rc);
		return rc;
	}

	/* force usb ovp fet off */
	rc = qpnp_chg_masked_write(chip,
			chip->usb_chgpth_base + CHGR_USB_USB_OTG_CTL,
			USB_OTG_EN_BIT,
			USB_OTG_EN_BIT, 1);
	if (rc) {
		pr_err("Failed to turn off usb ovp rc = %d\n", rc);
		return rc;
	}

	return 0;
}

static enum power_supply_property pm_power_props_mains[] = {
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_ONLINE,
	POWER_SUPPLY_PROP_CURRENT_MAX,
};

static enum power_supply_property msm_batt_power_props[] = {
	POWER_SUPPLY_PROP_CHARGING_ENABLED,
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_CHARGE_TYPE,
	POWER_SUPPLY_PROP_HEALTH,
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_ONLINE,
	POWER_SUPPLY_PROP_TECHNOLOGY,
	POWER_SUPPLY_PROP_VOLTAGE_MAX_DESIGN,
	POWER_SUPPLY_PROP_VOLTAGE_MIN_DESIGN,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_CAPACITY,
	POWER_SUPPLY_PROP_CURRENT_NOW,
	POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN,
	POWER_SUPPLY_PROP_CHARGE_FULL,
	POWER_SUPPLY_PROP_TEMP,
	POWER_SUPPLY_PROP_SYSTEM_TEMP_LEVEL,
	POWER_SUPPLY_PROP_CYCLE_COUNT,
};

static char *pm_power_supplied_to[] = {
	"battery",
};

static char *pm_batt_supplied_to[] = {
	"bms",
};

#define USB_WALL_THRESHOLD_MA	500
static int
qpnp_power_get_property_mains(struct power_supply *psy,
				  enum power_supply_property psp,
				  union power_supply_propval *val)
{
	struct qpnp_chg_chip *chip = container_of(psy, struct qpnp_chg_chip,
								dc_psy);
	union power_supply_propval ret = {0,};	//djjeon 0709 add

	switch (psp) {
	case POWER_SUPPLY_PROP_PRESENT:
	case POWER_SUPPLY_PROP_ONLINE:
		val->intval = 0;
		if (chip->charging_disabled)
			return 0;
		//qpnp_chg_is_dc_chg_plugged_in(chip);	//20130710, P14787, delete
		//20130710, P14787, usbOnline update only DCP , CDP
		chip->usb_psy->get_property(chip->usb_psy,
                          POWER_SUPPLY_PROP_TYPE, &ret);	
		if( (ret.intval == 5)||(ret.intval == 6))
			val->intval = qpnp_chg_is_usb_chg_plugged_in(chip);
		break;
	case POWER_SUPPLY_PROP_CURRENT_MAX:
		val->intval = chip->maxinput_dc_ma;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static int
get_prop_battery_voltage_now(struct qpnp_chg_chip *chip)
{

#if defined(CONFIG_PANTECH_PMIC_FUELGAUGE_MAX17058)
	return max17058_get_voltage();  
#else
	int rc = 0;
	struct qpnp_vadc_result results;

	if (chip->revision == 0 && chip->type == SMBB) {
		pr_err("vbat reading not supported for 1.0 rc=%d\n", rc);
		return 0;
	} else {
		rc = qpnp_vadc_read(VBAT_SNS, &results);
		if (rc) {
			pr_err("Unable to read vbat rc=%d\n", rc);
			return 0;
		}
		return results.physical;
	}
#endif
}

#define BATT_PRES_BIT BIT(7)
static int
get_prop_batt_present(struct qpnp_chg_chip *chip)
{
	u8 batt_present;
	int rc;

	rc = qpnp_chg_read(chip, &batt_present,
				chip->bat_if_base + CHGR_BAT_IF_PRES_STATUS, 1);
	if (rc) {
		pr_err("Couldn't read battery status read failed rc=%d\n", rc);
		return 0;
	};
	return (batt_present & BATT_PRES_BIT) ? 1 : 0;
}

#if defined(CONFIG_PANTECH_PMIC_CHARGER_SMB349) || defined(CONFIG_PANTECH_PMIC_CHARGER_SMB347)

#if 1 // (defined(T_EF56S) && (BOARD_VER <TP10))
static int set_prop_batt_present_ctrl(struct qpnp_chg_chip *chip)
{
       int rc;
       u8 temp;

	temp = 0x09;
	rc = qpnp_chg_write(chip, &temp,
		chip->bat_if_base + CHGR_BAT_IF_PRES_CTRL, 1);
	if (rc) {
		pr_err("Couldn't write battery pres ctrl write failed rc=%d\n", rc);
		return 0;
	};

	return 0;
}
#endif
static int set_prop_batt_btc_ctrl(struct qpnp_chg_chip *chip)
{
       int rc;
	u8 reg;

	rc = qpnp_chg_read(chip, &reg, chip->bat_if_base, 1);
	if (rc) {
		pr_err("spmi read failed: addr=%03X, rc=%d\n", chip->bat_if_base, rc);
		return rc;
	}
	pr_err("addr = 0x%x read 0x%x\n", chip->bat_if_base, reg);

	reg = reg & 0x7f;

	pr_err("Writing 0x%x\n", reg);
	
	rc = qpnp_chg_write(chip, &reg,
		chip->bat_if_base + CHGR_BAT_IF_BTC_CTRL, 1);
	if (rc) {
		pr_err("Couldn't write battery pres ctrl write failed rc=%d\n", rc);
		return 0;
	};

	return 0;
}
int cable_present_state(void)
{
	return the_qpnp_chip->usb_present;
}
EXPORT_SYMBOL_GPL(cable_present_state);
#endif

#define BATT_TEMP_HOT	BIT(6)
#define BATT_TEMP_OK	BIT(7)
static int
get_prop_batt_health(struct qpnp_chg_chip *chip)
{
	u8 batt_health;
	int rc;

	rc = qpnp_chg_read(chip, &batt_health,
				chip->bat_if_base + CHGR_STATUS, 1);
	if (rc) {
		pr_err("Couldn't read battery health read failed rc=%d\n", rc);
		return POWER_SUPPLY_HEALTH_UNKNOWN;
	};

#if defined(CONFIG_PANTECH_PMIC_CHARGER_SMB349) || defined(CONFIG_PANTECH_PMIC_CHARGER_SMB347)
	if(prev_state_mode == BATT_THERM_COLD)
	{
		return POWER_SUPPLY_HEALTH_COLD;
	}else if(prev_state_mode == BATT_THERM_HOT){
		return POWER_SUPPLY_HEALTH_OVERHEAT;
	}else{
		return POWER_SUPPLY_HEALTH_GOOD;
	}
#else 
	if (BATT_TEMP_OK & batt_health)
		return POWER_SUPPLY_HEALTH_GOOD;
	if (BATT_TEMP_HOT & batt_health)
		return POWER_SUPPLY_HEALTH_OVERHEAT;
	else
		return POWER_SUPPLY_HEALTH_COLD;
#endif
}

static int
get_prop_charge_type(struct qpnp_chg_chip *chip)
{
	int rc;
	u8 chgr_sts;

	if (!get_prop_batt_present(chip))
		return POWER_SUPPLY_CHARGE_TYPE_NONE;

	rc = qpnp_chg_read(chip, &chgr_sts,
				INT_RT_STS(chip->chgr_base), 1);
	if (rc) {
		pr_err("failed to read interrupt sts %d\n", rc);
		return POWER_SUPPLY_CHARGE_TYPE_NONE;
	}

	if (chgr_sts & TRKL_CHG_ON_IRQ)
		return POWER_SUPPLY_CHARGE_TYPE_TRICKLE;
	if (chgr_sts & FAST_CHG_ON_IRQ)
		return POWER_SUPPLY_CHARGE_TYPE_FAST;

	return POWER_SUPPLY_CHARGE_TYPE_NONE;
}
#if defined(CONFIG_PANTECH_PMIC_CHARGER_SMB349) || defined(CONFIG_PANTECH_PMIC_CHARGER_SMB347)
static int
get_prop_batt_status(struct qpnp_chg_chip *chip)
{
#if defined(FEATURE_PANTECH_PMIC_CHARGING_DISABLE)
	//sayuss charge_CMD
	if(chip->charging_disabled)
			return POWER_SUPPLY_STATUS_DISCHARGING;
#endif
	if(chip->usb_present){
#ifdef FEATURE_PANTECH_QUALCOMM_OTG_MODE_OVP_BUG
		if(is_pantech_host_mode)
			return POWER_SUPPLY_STATUS_DISCHARGING;
#endif

#if defined(FEATURE_PANTECH_PMIC_MAX17058)
		if (max17058_get_soc() == 100)
			return POWER_SUPPLY_STATUS_FULL;
#endif
		if( get_prop_batt_health(chip) == POWER_SUPPLY_HEALTH_COLD 
			|| get_prop_batt_health(chip) == POWER_SUPPLY_HEALTH_OVERHEAT)
			return POWER_SUPPLY_STATUS_NOT_CHARGING;

		return POWER_SUPPLY_STATUS_CHARGING;
	}
	else {
			return POWER_SUPPLY_STATUS_DISCHARGING;
	}
	
	return POWER_SUPPLY_STATUS_UNKNOWN;


}
#else 
static int
get_prop_batt_status(struct qpnp_chg_chip *chip)
{
	int rc;
	u8 chgr_sts, bat_if_sts;

	if ((qpnp_chg_is_usb_chg_plugged_in(chip) ||
		qpnp_chg_is_dc_chg_plugged_in(chip)) && chip->chg_done) {
		return POWER_SUPPLY_STATUS_FULL;
	}

	rc = qpnp_chg_read(chip, &chgr_sts, INT_RT_STS(chip->chgr_base), 1);
	if (rc) {
		pr_err("failed to read interrupt sts %d\n", rc);
		return POWER_SUPPLY_CHARGE_TYPE_NONE;
	}

	rc = qpnp_chg_read(chip, &bat_if_sts, INT_RT_STS(chip->bat_if_base), 1);
	if (rc) {
		pr_err("failed to read bat_if sts %d\n", rc);
		return POWER_SUPPLY_CHARGE_TYPE_NONE;
	}

	if (chgr_sts & TRKL_CHG_ON_IRQ && bat_if_sts & BAT_FET_ON_IRQ)
		return POWER_SUPPLY_STATUS_CHARGING;
	if (chgr_sts & FAST_CHG_ON_IRQ && bat_if_sts & BAT_FET_ON_IRQ)
		return POWER_SUPPLY_STATUS_CHARGING;

	return POWER_SUPPLY_STATUS_DISCHARGING;
}
#endif

#if defined(FEATURE_PANTECH_PMIC_LED)
int get_batt_status(void)
{
	return get_prop_batt_status(the_qpnp_chip);
}
EXPORT_SYMBOL_GPL(get_batt_status);
#endif

static int
get_prop_current_now(struct qpnp_chg_chip *chip)
{
	union power_supply_propval ret = {0,};

	if (chip->bms_psy) {
		chip->bms_psy->get_property(chip->bms_psy,
			  POWER_SUPPLY_PROP_CURRENT_NOW, &ret);
		return ret.intval;
	} else {
		pr_debug("No BMS supply registered return 0\n");
	}

	return 0;
}

static int
get_prop_full_design(struct qpnp_chg_chip *chip)
{
	union power_supply_propval ret = {0,};

	if (chip->bms_psy) {
		chip->bms_psy->get_property(chip->bms_psy,
			  POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN, &ret);
		return ret.intval;
	} else {
		pr_debug("No BMS supply registered return 0\n");
	}

	return 0;
}

static int
get_prop_charge_full(struct qpnp_chg_chip *chip)
{
	union power_supply_propval ret = {0,};

	if (chip->bms_psy) {
		chip->bms_psy->get_property(chip->bms_psy,
			  POWER_SUPPLY_PROP_CHARGE_FULL, &ret);
		return ret.intval;
	} else {
		pr_debug("No BMS supply registered return 0\n");
	}

	return 0;
}

#define DEFAULT_CAPACITY	50
static int
get_prop_capacity(struct qpnp_chg_chip *chip)
{
	union power_supply_propval ret = {0,};
	int battery_status, charger_in;
#if defined(CONFIG_PANTECH_PMIC_FUELGAUGE_MAX17058)	

	int soc=-1;	//20130719 djjeon BMS remove

#endif	
/*20130814 djjeon, when sleep awake, 50% display problem
	if (chip->use_default_batt_values || !get_prop_batt_present(chip))
		return DEFAULT_CAPACITY;
*/
	if (chip->bms_psy) {
		chip->bms_psy->get_property(chip->bms_psy,
			  POWER_SUPPLY_PROP_CAPACITY, &ret);
		battery_status = get_prop_batt_status(chip);
		charger_in = qpnp_chg_is_usb_chg_plugged_in(chip) ||
			qpnp_chg_is_dc_chg_plugged_in(chip);

		if (battery_status != POWER_SUPPLY_STATUS_CHARGING
				&& charger_in
				&& !chip->resuming_charging
				&& !chip->charging_disabled
				&& chip->soc_resume_limit
				&& ret.intval <= chip->soc_resume_limit) {
			pr_debug("resuming charging at %d%% soc\n",
					ret.intval);
			chip->resuming_charging = true;
			qpnp_chg_set_appropriate_vbatdet(chip);
			qpnp_chg_charge_en(chip, !chip->charging_disabled);
		}
		if (ret.intval == 0) {
			if (!qpnp_chg_is_usb_chg_plugged_in(chip)
				&& !qpnp_chg_is_usb_chg_plugged_in(chip))
				pr_warn_ratelimited("Battery 0, CHG absent\n");
		}
		return ret.intval;
	} else {
#if defined(CONFIG_PANTECH_PMIC_FUELGAUGE_MAX17058)	

		soc = max17058_get_soc();	//20130719 djjeon BMS remove
		if(soc<0)
			pr_info("No BMS supply registered return 50\n");
		else
			return soc;

#else
		pr_debug("No BMS supply registered return 50\n");
#endif	
	}

	/* return default capacity to avoid userspace
	 * from shutting down unecessarily */
	return DEFAULT_CAPACITY;
}

#if defined(CONFIG_PANTECH_PMIC_CHARGER_SMB349) || defined(CONFIG_PANTECH_PMIC_CHARGER_SMB347)
int get_prop_batt_temp(void)
{
	return batt_temp;
}
#else
#define DEFAULT_TEMP		250
#define MAX_TOLERABLE_BATT_TEMP_DDC	680
static int
get_prop_batt_temp(struct qpnp_chg_chip *chip)
{
	int rc = 0;
	struct qpnp_vadc_result results;

	if (chip->use_default_batt_values || !get_prop_batt_present(chip))
		return DEFAULT_TEMP;

		rc = qpnp_vadc_read(LR_MUX1_BATT_THERM, &results);
		if (rc) {
			pr_debug("Unable to read batt temperature rc=%d\n", rc);
			return 0;
		}
		pr_debug("get_bat_temp %d %lld\n",
			results.adc_code, results.physical);

		return (int)results.physical;
	}
#endif 

static int get_prop_cycle_count(struct qpnp_chg_chip *chip)
{
	union power_supply_propval ret = {0,};

	if (chip->bms_psy)
		chip->bms_psy->get_property(chip->bms_psy,
			  POWER_SUPPLY_PROP_CYCLE_COUNT, &ret);
	return ret.intval;
}

static int get_prop_online(struct qpnp_chg_chip *chip)
{
	return qpnp_chg_is_batfet_closed(chip);
}

#ifdef FEATURE_PANTECH_QUALCOMM_OTG_MODE_OVP_BUG
//xsemiyas_debug

extern int get_pantech_chg_otg_mode(void)
{
	return is_pantech_host_mode;
}

extern void set_pantech_chg_otg_mode(int val)
{
	is_pantech_host_mode = val;
}
#endif

static void
qpnp_batt_external_power_changed(struct power_supply *psy)
{
	struct qpnp_chg_chip *chip = container_of(psy, struct qpnp_chg_chip,
								batt_psy);
	union power_supply_propval ret = {0,};
#ifdef FEATURE_PANTECH_QUALCOMM_OTG_MODE_OVP_BUG
	//xsemiyas_debug
	int value;
#endif
       if (charger_probe == 0)
       {
          pr_err("Batt PSY empty !!!!!!!!!!!!!!!!!!!!");
          return;
 	}
	if (!chip->bms_psy)
		chip->bms_psy = power_supply_get_by_name("bms");

	chip->usb_psy->get_property(chip->usb_psy,
			  POWER_SUPPLY_PROP_SCOPE, &ret);
	if (ret.intval) {
		if ((ret.intval == POWER_SUPPLY_SCOPE_SYSTEM)
				&& !qpnp_chg_is_otg_en_set(chip)) {
#ifdef FEATURE_PANTECH_QUALCOMM_OTG_MODE_OVP_BUG				
			//xsemiyas_debug
			value = switch_usb_to_host_mode(chip);
			if(value == 0){
				is_pantech_host_mode = 1;
			}else{
				is_pantech_host_mode = -1;
			}
#else
			switch_usb_to_host_mode(chip);
#endif			
			return;
		}
		if ((ret.intval == POWER_SUPPLY_SCOPE_DEVICE)
				&& qpnp_chg_is_otg_en_set(chip)) {
			switch_usb_to_charge_mode(chip);
#ifdef FEATURE_PANTECH_QUALCOMM_OTG_MODE_OVP_BUG			
			//xsemiyas_debug
			is_pantech_host_mode = 0;
#endif
			return;
		}
	}

	chip->usb_psy->get_property(chip->usb_psy,
			  POWER_SUPPLY_PROP_ONLINE, &ret);

	/* Only honour requests while USB is present */

	if (qpnp_chg_is_usb_chg_plugged_in(chip)) {
		chip->usb_psy->get_property(chip->usb_psy,
			  POWER_SUPPLY_PROP_CURRENT_MAX, &ret);
		if (ret.intval <= 2 && !chip->use_default_batt_values &&
						get_prop_batt_present(chip)) {
			qpnp_chg_usb_suspend_enable(chip, 1);
			qpnp_chg_iusbmax_set(chip, QPNP_CHG_I_MAX_MIN_100);
		} else {
		qpnp_chg_usb_suspend_enable(chip, 0);
			qpnp_chg_iusbmax_set(chip, ret.intval / 1000);
	}
	}
#if defined(CONFIG_PANTECH_PMIC_CHARGER_SMB349) || defined(CONFIG_PANTECH_PMIC_CHARGER_SMB347)
	if(chip->usb_present){
		if(qpnp_get_cable_type() == PANTECH_USB){
#if defined (FEATURE_PANTECH_PMIC_ABNORMAL)
#if defined (FEATURE_PANTECH_QUALCOMM_OTG_MODE_OVP_BUG)
			if( abnormal_state != true && is_pantech_host_mode==0)
#else 
			if( abnormal_state != true)
#endif
			{	
			 	abnormal_state = true;	
			schedule_delayed_work(&chip->update_abnormal_work,
			     round_jiffies_relative(msecs_to_jiffies(90000)));		
		}
		}
#endif	

#ifdef FEATURE_PANTECH_QUALCOMM_OTG_MODE_OVP_BUG
	if(is_pantech_host_mode==0)
#endif 
	{
		 	smb349_current_set(qpnp_get_cable_type(), chip->maxinput_usb_ma);
		}
		
		}
		
#endif
//+++ 20130806, P14787, djjeon, charging setting stability test
#if ((defined(FEATURE_PANTECH_PMIC_MONITOR_TEST) && defined(CONFIG_PANTECH_PMIC_CHARGER_SMB349)) || (defined(FEATURE_PANTECH_PMIC_MONITOR_TEST) && defined(CONFIG_PANTECH_PMIC_CHARGER_SMB347)))
	if(chip->charging_setting){
		if(chip->usb_present == true){
			smb349_test_limit_up(1);
		}
	}
#endif	
//--- 20130806, P14787, djjeon, charging setting stability test
#if defined(FEATURE_PANTECH_PMIC_ABNORMAL)
if(chip->usb_present == false){
	chip->check_cable_disconnect_work = false;
	cancel_delayed_work(&chip->update_abnormal_cable_disconeect_work); 
	}
#endif

#if defined(FEATURE_PANTECH_PMIC_EOC)
if((chip->usb_present == false)&&(chip->end_recharing == true)){
	smb349_write_reg(0x04, 0xEE);
	chip->end_recharing = false;
	pr_debug("end_recharing %d\n", chip->end_recharing);
}
#endif
#if defined(FEATURE_PANTECH_PMIC_CHARGE_LIMIT_WITH_TSENSOR)
if(chip->usb_present == false){
	t_prev_mode = false;
	T_sensor_mode = false;
}
#endif
	pr_debug("end of power supply changed\n");
	power_supply_changed(&chip->batt_psy);
}
	
#if defined (FEATURE_PANTECH_PMIC_ABNORMAL)
static void check_abnormal_worker(struct work_struct *work)	
{
#if defined (FEATURE_PANTECH_USB_BLOCKING_MDMSTATE)
	if(strcmp(usb_get_mdm_state(), "USB_DISABLED") == 0)
		return;
#endif
	if(the_qpnp_chip->usb_present == true && qpnp_get_cable_type() == PANTECH_USB 
			&& (strcmp(usb_composite_get_udc_state(), "DISCONNECTED") == 0)){
			power_supply_set_current_limit(the_qpnp_chip->usb_psy, 1000*2000);
			qpnp_set_cable_type(POWER_SUPPLY_TYPE_USB_DCP);
		
#if defined(FEATURE_PANTECH_PMIC_CHARGE_LIMIT_WITH_LCD)
		if(get_is_offline_charging_mode()){
			boot_lcd_cnt=4;
		}
#endif
	}
}
static void check_abnormal_cable_disconnet_worker(struct work_struct *work)
{
	if(the_qpnp_chip->usb_present && qpnp_get_cable_type() == PANTECH_CABLE_NONE){
		power_supply_set_current_limit(the_qpnp_chip->usb_psy, 1000*2000);
			qpnp_set_cable_type(POWER_SUPPLY_TYPE_USB_DCP);
		the_qpnp_chip->check_cable_disconnect_work = false;
	}
}
#endif

static int
qpnp_batt_power_get_property(struct power_supply *psy,
				       enum power_supply_property psp,
				       union power_supply_propval *val)
{
	struct qpnp_chg_chip *chip = container_of(psy, struct qpnp_chg_chip,
								batt_psy);

	switch (psp) {
	case POWER_SUPPLY_PROP_STATUS:
		val->intval = get_prop_batt_status(chip);
		break;
	case POWER_SUPPLY_PROP_CHARGE_TYPE:
		val->intval = get_prop_charge_type(chip);
		break;
	case POWER_SUPPLY_PROP_HEALTH:
		val->intval = get_prop_batt_health(chip);
		break;
	case POWER_SUPPLY_PROP_PRESENT:
		val->intval = get_prop_batt_present(chip);
		break;
	case POWER_SUPPLY_PROP_TECHNOLOGY:
		val->intval = POWER_SUPPLY_TECHNOLOGY_LION;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_MAX_DESIGN:
		val->intval = chip->max_voltage_mv * 1000;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_MIN_DESIGN:
		val->intval = chip->min_voltage_mv * 1000;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		val->intval = get_prop_battery_voltage_now(chip);
		break;
	case POWER_SUPPLY_PROP_TEMP:
#if defined(CONFIG_PANTECH_PMIC_CHARGER_SMB349) || defined(CONFIG_PANTECH_PMIC_CHARGER_SMB347)
		val->intval	= get_prop_batt_temp();
#else
		val->intval = get_prop_batt_temp(chip);
#endif
		break;
	case POWER_SUPPLY_PROP_CAPACITY:
		val->intval = get_prop_capacity(chip);
		break;
	case POWER_SUPPLY_PROP_CURRENT_NOW:
		val->intval = get_prop_current_now(chip);
		break;
	case POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN:
		val->intval = get_prop_full_design(chip);
		break;
	case POWER_SUPPLY_PROP_CHARGE_FULL:
		val->intval = get_prop_charge_full(chip);
		break;
	case POWER_SUPPLY_PROP_CHARGING_ENABLED:
		val->intval = !(chip->charging_disabled);
		break;
	case POWER_SUPPLY_PROP_SYSTEM_TEMP_LEVEL:
		val->intval = chip->therm_lvl_sel;
		break;
	case POWER_SUPPLY_PROP_CYCLE_COUNT:
		val->intval = get_prop_cycle_count(chip);
		break;
#if defined(CONFIG_PANTECH_PMIC_CHARGER_SMB349) || defined(CONFIG_PANTECH_PMIC_CHARGER_SMB347)
	case POWER_SUPPLY_PROP_ONLINE:
		val->intval = get_prop_online(chip);
		val->intval = false; //battery fet close not use
		break;
#else
	case POWER_SUPPLY_PROP_ONLINE:
		val->intval = get_prop_online(chip);
		break;
#endif
	default:
		return -EINVAL;
	}

	return 0;
}

#define BTC_CONFIG_ENABLED	BIT(7)
#define BTC_COLD		BIT(1)
#define BTC_HOT			BIT(0)

#if 0 //temp block for build
static int
qpnp_chg_bat_if_configure_btc(struct qpnp_chg_chip *chip)
{
	u8 btc_cfg = 0, mask = 0;

	/* Do nothing if battery peripheral not present */
	if (!chip->bat_if_base)
		return 0;

	if ((chip->hot_batt_p == HOT_THD_25_PCT)
			|| (chip->hot_batt_p == HOT_THD_35_PCT)) {
		btc_cfg |= btc_value[chip->hot_batt_p];
		mask |= BTC_HOT;
	}

	if ((chip->cold_batt_p == COLD_THD_70_PCT) ||
			(chip->cold_batt_p == COLD_THD_80_PCT)) {
		btc_cfg |= btc_value[chip->cold_batt_p];
		mask |= BTC_COLD;
	}

	if (chip->btc_disabled)
		mask |= BTC_CONFIG_ENABLED;

	return qpnp_chg_masked_write(chip,
			chip->bat_if_base + BAT_IF_BTC_CTRL,
			mask, btc_cfg, 1);
}
#endif

#define QPNP_CHG_VINMIN_MIN_MV		4200
#define QPNP_CHG_VINMIN_HIGH_MIN_MV	5600
#define QPNP_CHG_VINMIN_HIGH_MIN_VAL	0x2B
#define QPNP_CHG_VINMIN_MAX_MV		9600
#define QPNP_CHG_VINMIN_STEP_MV		50
#define QPNP_CHG_VINMIN_STEP_HIGH_MV	200
#define QPNP_CHG_VINMIN_MASK		0x1F
static int
qpnp_chg_vinmin_set(struct qpnp_chg_chip *chip, int voltage)
{
	u8 temp;

	if (voltage < QPNP_CHG_VINMIN_MIN_MV
			|| voltage > QPNP_CHG_VINMIN_MAX_MV) {
		pr_err("bad mV=%d asked to set\n", voltage);
		return -EINVAL;
	}
	if (voltage >= QPNP_CHG_VINMIN_HIGH_MIN_MV) {
		temp = QPNP_CHG_VINMIN_HIGH_MIN_VAL;
		temp += (voltage - QPNP_CHG_VINMIN_MIN_MV)
			/ QPNP_CHG_VINMIN_STEP_HIGH_MV;
	} else {
		temp = (voltage - QPNP_CHG_VINMIN_MIN_MV)
			/ QPNP_CHG_VINMIN_STEP_MV;
	}

	pr_debug("voltage=%d setting %02x\n", voltage, temp);
	return qpnp_chg_masked_write(chip,
			chip->chgr_base + CHGR_VIN_MIN,
			QPNP_CHG_VINMIN_MASK, temp, 1);
}

#define QPNP_CHG_IBATSAFE_MIN_MA		100
#define QPNP_CHG_IBATSAFE_MAX_MA		3250
#define QPNP_CHG_I_STEP_MA		50
#define QPNP_CHG_I_MIN_MA		100
#define QPNP_CHG_I_MASK			0x3F
static int
qpnp_chg_ibatsafe_set(struct qpnp_chg_chip *chip, int safe_current)
{
	u8 temp;

	if (safe_current < QPNP_CHG_IBATSAFE_MIN_MA
			|| safe_current > QPNP_CHG_IBATSAFE_MAX_MA) {
		pr_err("bad mA=%d asked to set\n", safe_current);
		return -EINVAL;
	}

	temp = safe_current / QPNP_CHG_I_STEP_MA;
	return qpnp_chg_masked_write(chip,
			chip->chgr_base + CHGR_IBAT_SAFE,
			QPNP_CHG_I_MASK, temp, 1);
}

#define QPNP_CHG_ITERM_MIN_MA		100
#define QPNP_CHG_ITERM_MAX_MA		250
#define QPNP_CHG_ITERM_STEP_MA		50
#define QPNP_CHG_ITERM_MASK			0x03
static int
qpnp_chg_ibatterm_set(struct qpnp_chg_chip *chip, int term_current)
{
	u8 temp;

	if (term_current < QPNP_CHG_ITERM_MIN_MA
			|| term_current > QPNP_CHG_ITERM_MAX_MA) {
		pr_err("bad mA=%d asked to set\n", term_current);
		return -EINVAL;
	}

	temp = (term_current - QPNP_CHG_ITERM_MIN_MA)
				/ QPNP_CHG_ITERM_STEP_MA;
	return qpnp_chg_masked_write(chip,
			chip->chgr_base + CHGR_IBAT_TERM_CHGR,
			QPNP_CHG_ITERM_MASK, temp, 1);
}

#define QPNP_CHG_IBATMAX_MIN	50
#define QPNP_CHG_IBATMAX_MAX	3250
static int
qpnp_chg_ibatmax_set(struct qpnp_chg_chip *chip, int chg_current)
{
	u8 temp;

	if (chg_current < QPNP_CHG_IBATMAX_MIN
			|| chg_current > QPNP_CHG_IBATMAX_MAX) {
		pr_err("bad mA=%d asked to set\n", chg_current);
		return -EINVAL;
	}
	temp = chg_current / QPNP_CHG_I_STEP_MA;
	// sayuss monitoring current
	chip->maxinput_dc_ma = chg_current;
	temp = (chg_current - QPNP_CHG_I_MIN_MA) / QPNP_CHG_I_STEP_MA;
	return qpnp_chg_masked_write(chip, chip->chgr_base + CHGR_IBAT_MAX,
			QPNP_CHG_I_MASK, temp, 1);
}

#define QPNP_CHG_TCHG_MASK	0x7F
#define QPNP_CHG_TCHG_MIN	4
#define QPNP_CHG_TCHG_MAX	512
#define QPNP_CHG_TCHG_STEP	4
static int qpnp_chg_tchg_max_set(struct qpnp_chg_chip *chip, int minutes)
{
	u8 temp;

	if (minutes < QPNP_CHG_TCHG_MIN || minutes > QPNP_CHG_TCHG_MAX) {
		pr_err("bad max minutes =%d asked to set\n", minutes);
		return -EINVAL;
	}

	temp = (minutes - 1)/QPNP_CHG_TCHG_STEP;
	return qpnp_chg_masked_write(chip, chip->chgr_base + CHGR_TCHG_MAX,
			QPNP_CHG_TCHG_MASK, temp, 1);
}

#define QPNP_CHG_V_MIN_MV	3240
#define QPNP_CHG_V_MAX_MV	4500
#define QPNP_CHG_V_STEP_MV	10
static int
qpnp_chg_vddsafe_set(struct qpnp_chg_chip *chip, int voltage)
{
	u8 temp;

	if (voltage < QPNP_CHG_V_MIN_MV
			|| voltage > QPNP_CHG_V_MAX_MV) {
		pr_err("bad mV=%d asked to set\n", voltage);
		return -EINVAL;
	}
	temp = (voltage - QPNP_CHG_V_MIN_MV) / QPNP_CHG_V_STEP_MV;
	pr_debug("voltage=%d setting %02x\n", voltage, temp);
	return qpnp_chg_write(chip, &temp,
		chip->chgr_base + CHGR_VDD_SAFE, 1);
}

#define QPNP_CHG_VDDMAX_MIN	3400
static int
qpnp_chg_vddmax_set(struct qpnp_chg_chip *chip, int voltage)
{
	u8 temp = 0;

	if (voltage < QPNP_CHG_VDDMAX_MIN
			|| voltage > QPNP_CHG_V_MAX_MV) {
		pr_err("bad mV=%d asked to set\n", voltage);
		return -EINVAL;
	}
	chip->set_vddmax_mv = voltage + chip->delta_vddmax_mv;

	temp = (chip->set_vddmax_mv - QPNP_CHG_V_MIN_MV) / QPNP_CHG_V_STEP_MV;

	pr_debug("voltage=%d setting %02x\n", chip->set_vddmax_mv, temp);
	return qpnp_chg_write(chip, &temp, chip->chgr_base + CHGR_VDD_MAX, 1);
}

#define BOOST_MIN_UV	4200000
#define BOOST_MAX_UV	5500000
#define BOOST_STEP_UV	50000
#define BOOST_MIN	16
#define N_BOOST_V	((BOOST_MAX_UV - BOOST_MIN_UV) / BOOST_STEP_UV + 1)
static int
qpnp_boost_vset(struct qpnp_chg_chip *chip, int voltage)
{
	u8 reg = 0;

	if (voltage < BOOST_MIN_UV || voltage > BOOST_MAX_UV) {
		pr_err("invalid voltage requested %d uV\n", voltage);
		return -EINVAL;
	}

	reg = DIV_ROUND_UP(voltage - BOOST_MIN_UV, BOOST_STEP_UV) + BOOST_MIN;

	pr_debug("voltage=%d setting %02x\n", voltage, reg);
	return qpnp_chg_write(chip, &reg, chip->boost_base + BOOST_VSET, 1);
}

static int
qpnp_boost_vget_uv(struct qpnp_chg_chip *chip)
{
	int rc;
	u8 boost_reg;

	rc = qpnp_chg_read(chip, &boost_reg,
		 chip->boost_base + BOOST_VSET, 1);
	if (rc) {
		pr_err("failed to read BOOST_VSET rc=%d\n", rc);
		return rc;
	}

	if (boost_reg < BOOST_MIN) {
		pr_err("Invalid reading from 0x%x\n", boost_reg);
		return -EINVAL;
	}

	return BOOST_MIN_UV + ((boost_reg - BOOST_MIN) * BOOST_STEP_UV);
}

/* JEITA compliance logic */
static void
qpnp_chg_set_appropriate_vddmax(struct qpnp_chg_chip *chip)
{
	if (chip->bat_is_cool)
		qpnp_chg_vddmax_set(chip, chip->cool_bat_mv);
	else if (chip->bat_is_warm)
		qpnp_chg_vddmax_set(chip, chip->warm_bat_mv);
	else
		qpnp_chg_vddmax_set(chip, chip->max_voltage_mv);
}

static void
qpnp_chg_set_appropriate_battery_current(struct qpnp_chg_chip *chip)
{
	unsigned int chg_current = chip->max_bat_chg_current;

	if (chip->bat_is_cool)
		chg_current = min(chg_current, chip->cool_bat_chg_ma);

	if (chip->bat_is_warm)
		chg_current = min(chg_current, chip->warm_bat_chg_ma);

	if (chip->therm_lvl_sel != 0 && chip->thermal_mitigation)
		chg_current = min(chg_current,
			chip->thermal_mitigation[chip->therm_lvl_sel]);

	pr_debug("setting %d mA\n", chg_current);
	qpnp_chg_ibatmax_set(chip, chg_current);
}

static void
qpnp_batt_system_temp_level_set(struct qpnp_chg_chip *chip, int lvl_sel)
{
	if (lvl_sel >= 0 && lvl_sel < chip->thermal_levels) {
		chip->therm_lvl_sel = lvl_sel;
		if (lvl_sel == (chip->thermal_levels - 1)) {
			/* disable charging if highest value selected */
			qpnp_chg_buck_control(chip, 0);
		} else {
			qpnp_chg_buck_control(chip, 1);
			qpnp_chg_set_appropriate_battery_current(chip);
		}
	} else {
		pr_err("Unsupported level selected %d\n", lvl_sel);
	}
}

/* OTG regulator operations */
static int
qpnp_chg_regulator_otg_enable(struct regulator_dev *rdev)
{
	struct qpnp_chg_chip *chip = rdev_get_drvdata(rdev);

	return switch_usb_to_host_mode(chip);
}

static int
qpnp_chg_regulator_otg_disable(struct regulator_dev *rdev)
{
	struct qpnp_chg_chip *chip = rdev_get_drvdata(rdev);

	return switch_usb_to_charge_mode(chip);
}

static int
qpnp_chg_regulator_otg_is_enabled(struct regulator_dev *rdev)
{
	struct qpnp_chg_chip *chip = rdev_get_drvdata(rdev);

	return qpnp_chg_is_otg_en_set(chip);
}

static int
qpnp_chg_regulator_boost_enable(struct regulator_dev *rdev)
{
	struct qpnp_chg_chip *chip = rdev_get_drvdata(rdev);
	int rc;

	if (qpnp_chg_is_usb_chg_plugged_in(chip) &&
			(chip->flags & BOOST_FLASH_WA)) {
		qpnp_chg_usb_suspend_enable(chip, 1);

		rc = qpnp_chg_masked_write(chip,
			chip->usb_chgpth_base + SEC_ACCESS,
			0xFF,
			0xA5, 1);
		if (rc) {
			pr_err("failed to write SEC_ACCESS rc=%d\n", rc);
			return rc;
		}

		rc = qpnp_chg_masked_write(chip,
			chip->usb_chgpth_base + COMP_OVR1,
			0xFF,
			0x2F, 1);
		if (rc) {
			pr_err("failed to write COMP_OVR1 rc=%d\n", rc);
			return rc;
		}
	}

	return qpnp_chg_masked_write(chip,
		chip->boost_base + BOOST_ENABLE_CONTROL,
		BOOST_PWR_EN,
		BOOST_PWR_EN, 1);
}

/* Boost regulator operations */
#define ABOVE_VBAT_WEAK		BIT(1)
static int
qpnp_chg_regulator_boost_disable(struct regulator_dev *rdev)
{
	struct qpnp_chg_chip *chip = rdev_get_drvdata(rdev);
	int rc;
	u8 vbat_sts;

	rc = qpnp_chg_masked_write(chip,
		chip->boost_base + BOOST_ENABLE_CONTROL,
		BOOST_PWR_EN,
		0, 1);
	if (rc) {
		pr_err("failed to disable boost rc=%d\n", rc);
		return rc;
	}

	rc = qpnp_chg_read(chip, &vbat_sts,
			chip->chgr_base + CHGR_VBAT_STATUS, 1);
	if (rc) {
		pr_err("failed to read bat sts rc=%d\n", rc);
		return rc;
	}

	if (!(vbat_sts & ABOVE_VBAT_WEAK) && (chip->flags & BOOST_FLASH_WA)) {
		rc = qpnp_chg_masked_write(chip,
			chip->chgr_base + SEC_ACCESS,
			0xFF,
			0xA5, 1);
		if (rc) {
			pr_err("failed to write SEC_ACCESS rc=%d\n", rc);
			return rc;
		}

		rc = qpnp_chg_masked_write(chip,
			chip->chgr_base + COMP_OVR1,
			0xFF,
			0x20, 1);
		if (rc) {
			pr_err("failed to write COMP_OVR1 rc=%d\n", rc);
			return rc;
		}

		usleep(2000);

		rc = qpnp_chg_masked_write(chip,
			chip->chgr_base + SEC_ACCESS,
			0xFF,
			0xA5, 1);
		if (rc) {
			pr_err("failed to write SEC_ACCESS rc=%d\n", rc);
			return rc;
		}

		rc = qpnp_chg_masked_write(chip,
			chip->chgr_base + COMP_OVR1,
			0xFF,
			0x00, 1);
		if (rc) {
			pr_err("failed to write COMP_OVR1 rc=%d\n", rc);
			return rc;
		}
	}

	if (qpnp_chg_is_usb_chg_plugged_in(chip)
			&& (chip->flags & BOOST_FLASH_WA)) {
		rc = qpnp_chg_masked_write(chip,
			chip->usb_chgpth_base + SEC_ACCESS,
			0xFF,
			0xA5, 1);
		if (rc) {
			pr_err("failed to write SEC_ACCESS rc=%d\n", rc);
			return rc;
		}

		rc = qpnp_chg_masked_write(chip,
			chip->usb_chgpth_base + COMP_OVR1,
			0xFF,
			0x00, 1);
		if (rc) {
			pr_err("failed to write COMP_OVR1 rc=%d\n", rc);
			return rc;
		}

		usleep(1000);

		qpnp_chg_usb_suspend_enable(chip, 0);
	}

	return rc;
}

static int
qpnp_chg_regulator_boost_is_enabled(struct regulator_dev *rdev)
{
	struct qpnp_chg_chip *chip = rdev_get_drvdata(rdev);

	return qpnp_chg_is_boost_en_set(chip);
}

static int
qpnp_chg_regulator_boost_set_voltage(struct regulator_dev *rdev,
		int min_uV, int max_uV, unsigned *selector)
{
	int uV = min_uV;
	int rc;
	struct qpnp_chg_chip *chip = rdev_get_drvdata(rdev);

	if (uV < BOOST_MIN_UV && max_uV >= BOOST_MIN_UV)
		uV = BOOST_MIN_UV;


	if (uV < BOOST_MIN_UV || uV > BOOST_MAX_UV) {
		pr_err("request %d uV is out of bounds\n", uV);
		return -EINVAL;
	}

	*selector = DIV_ROUND_UP(uV - BOOST_MIN_UV, BOOST_STEP_UV);
	if ((*selector * BOOST_STEP_UV + BOOST_MIN_UV) > max_uV) {
		pr_err("no available setpoint [%d, %d] uV\n", min_uV, max_uV);
		return -EINVAL;
	}

	rc = qpnp_boost_vset(chip, uV);

	return rc;
}

static int
qpnp_chg_regulator_boost_get_voltage(struct regulator_dev *rdev)
{
	struct qpnp_chg_chip *chip = rdev_get_drvdata(rdev);

	return qpnp_boost_vget_uv(chip);
}

static int
qpnp_chg_regulator_boost_list_voltage(struct regulator_dev *rdev,
			unsigned selector)
{
	if (selector >= N_BOOST_V)
		return 0;

	return BOOST_MIN_UV + (selector * BOOST_STEP_UV);
}

static struct regulator_ops qpnp_chg_otg_reg_ops = {
	.enable			= qpnp_chg_regulator_otg_enable,
	.disable		= qpnp_chg_regulator_otg_disable,
	.is_enabled		= qpnp_chg_regulator_otg_is_enabled,
};

static struct regulator_ops qpnp_chg_boost_reg_ops = {
	.enable			= qpnp_chg_regulator_boost_enable,
	.disable		= qpnp_chg_regulator_boost_disable,
	.is_enabled		= qpnp_chg_regulator_boost_is_enabled,
	.set_voltage		= qpnp_chg_regulator_boost_set_voltage,
	.get_voltage		= qpnp_chg_regulator_boost_get_voltage,
	.list_voltage		= qpnp_chg_regulator_boost_list_voltage,
};

#define MIN_DELTA_MV_TO_INCREASE_VDD_MAX	13
#define MAX_DELTA_VDD_MAX_MV			30
static void
qpnp_chg_adjust_vddmax(struct qpnp_chg_chip *chip, int vbat_mv)
{
	int delta_mv, closest_delta_mv, sign;

	delta_mv = chip->max_voltage_mv - vbat_mv;
	if (delta_mv > 0 && delta_mv < MIN_DELTA_MV_TO_INCREASE_VDD_MAX) {
		pr_debug("vbat is not low enough to increase vdd\n");
		return;
	}

	sign = delta_mv > 0 ? 1 : -1;
	closest_delta_mv = ((delta_mv + sign * QPNP_CHG_V_STEP_MV / 2)
			/ QPNP_CHG_V_STEP_MV) * QPNP_CHG_V_STEP_MV;
	pr_debug("max_voltage = %d, vbat_mv = %d, delta_mv = %d, closest = %d\n",
			chip->max_voltage_mv, vbat_mv,
			delta_mv, closest_delta_mv);
	chip->delta_vddmax_mv = clamp(chip->delta_vddmax_mv + closest_delta_mv,
			-MAX_DELTA_VDD_MAX_MV, MAX_DELTA_VDD_MAX_MV);
	pr_debug("using delta_vddmax_mv = %d\n", chip->delta_vddmax_mv);
	qpnp_chg_set_appropriate_vddmax(chip);
}

#define CONSECUTIVE_COUNT	3
static void
qpnp_eoc_work(struct work_struct *work)
{
	struct delayed_work *dwork = to_delayed_work(work);
	struct qpnp_chg_chip *chip = container_of(dwork,
				struct qpnp_chg_chip, eoc_work);
	static int count;
	int ibat_ma, vbat_mv, rc = 0;
	u8 batt_sts = 0, buck_sts = 0, chg_sts = 0;

	wake_lock(&chip->eoc_wake_lock);
	qpnp_chg_charge_en(chip, !chip->charging_disabled);

	rc = qpnp_chg_read(chip, &batt_sts, INT_RT_STS(chip->bat_if_base), 1);
	if (rc) {
		pr_err("failed to read batt_if rc=%d\n", rc);
		return;
	}

	rc = qpnp_chg_read(chip, &buck_sts, INT_RT_STS(chip->buck_base), 1);
	if (rc) {
		pr_err("failed to read buck rc=%d\n", rc);
		return;
	}

	rc = qpnp_chg_read(chip, &chg_sts, INT_RT_STS(chip->chgr_base), 1);
	if (rc) {
		pr_err("failed to read chg_sts rc=%d\n", rc);
		return;
	}

	pr_debug("chgr: 0x%x, bat_if: 0x%x, buck: 0x%x\n",
		chg_sts, batt_sts, buck_sts);

	if (!qpnp_chg_is_usb_chg_plugged_in(chip) &&
			!qpnp_chg_is_dc_chg_plugged_in(chip)) {
		pr_debug("no chg connected, stopping\n");
		goto stop_eoc;
	}

	if ((batt_sts & BAT_FET_ON_IRQ) && (chg_sts & FAST_CHG_ON_IRQ
					|| chg_sts & TRKL_CHG_ON_IRQ)) {
		ibat_ma = get_prop_current_now(chip) / 1000;
		vbat_mv = get_prop_battery_voltage_now(chip) / 1000;

		pr_debug("ibat_ma = %d vbat_mv = %d term_current_ma = %d\n",
				ibat_ma, vbat_mv, chip->term_current);

		if ((!(chg_sts & VBAT_DET_LOW_IRQ)) && (vbat_mv <
			(chip->max_voltage_mv - chip->resume_delta_mv))) {
			pr_debug("woke up too early\n");
			qpnp_chg_enable_irq(&chip->chg_vbatdet_lo);
			goto stop_eoc;
		}

		if (buck_sts & VDD_LOOP_IRQ)
			qpnp_chg_adjust_vddmax(chip, vbat_mv);

		if (!(buck_sts & VDD_LOOP_IRQ)) {
			pr_debug("Not in CV\n");
			count = 0;
		} else if ((ibat_ma * -1) > chip->term_current) {
			pr_debug("Not at EOC, battery current too high\n");
			count = 0;
		} else if (ibat_ma > 0) {
			pr_debug("Charging but system demand increased\n");
			count = 0;
		} else {
			if (count == CONSECUTIVE_COUNT) {
				pr_info("End of Charging\n");
				qpnp_chg_charge_en(chip, 0);
				chip->chg_done = true;
				power_supply_changed(&chip->batt_psy);
				qpnp_chg_enable_irq(&chip->chg_vbatdet_lo);
				goto stop_eoc;
			} else {
				count += 1;
				pr_debug("EOC count = %d\n", count);
			}
		}
	} else {
		pr_debug("not charging\n");
			goto stop_eoc;
	}

	schedule_delayed_work(&chip->eoc_work,
		msecs_to_jiffies(EOC_CHECK_PERIOD_MS));
	return;

stop_eoc:
	count = 0;
	wake_unlock(&chip->eoc_wake_lock);
}

static void
qpnp_chg_soc_check_work(struct work_struct *work)
{
	struct qpnp_chg_chip *chip = container_of(work,
				struct qpnp_chg_chip, soc_check_work);

	get_prop_capacity(chip);
}

#if defined(FEATURE_PANTECH_PMIC_USBIN_DROP_WORKAROUND)
static irqreturn_t
qpnp_sysok_irq_handler(int irq, void *_chip)
{
	struct qpnp_chg_chip *chip = _chip;
	int usb_present, host_mode;

	usb_present = qpnp_chg_is_usb_chg_plugged_in(chip);
	host_mode = qpnp_chg_is_otg_en_set(chip);
	pr_err("sysok triggered: %d host_mode: %d chip->usb_present [%d]\n",
		usb_present, host_mode,chip->usb_present );

	/* In host mode notifications cmoe from USB supply */
	if (host_mode)
		return IRQ_HANDLED;

	if (chip->usb_present ^ usb_present) {
		chip->usb_present = usb_present;
		if (!usb_present) {
			qpnp_chg_usb_suspend_enable(chip, 1);
			chip->chg_done = false;
		} else {
			schedule_delayed_work(&chip->eoc_work,
				msecs_to_jiffies(EOC_CHECK_PERIOD_MS));
		}

		power_supply_set_present(chip->usb_psy, chip->usb_present);

		//20130813, djjeon, TA cable disconnect problem.(offline charging)
		if (chip->dc_chgpth_base)
			power_supply_changed(&chip->dc_psy);
	}
#if defined(CONFIG_PANTECH_PMIC_CHARGER_SMB349) || defined(CONFIG_PANTECH_PMIC_CHARGER_SMB347)
	if( chip->usb_present) {
 	 wake_lock(&chip->smb349_cable_wake_lock);
	} else { 
	wake_unlock(&chip->smb349_cable_wake_lock);
#if defined (FEATURE_PANTECH_PMIC_ABNORMAL)
		abnormal_state = false;
#endif
	}
	#endif 

	return IRQ_HANDLED;
}
#endif

#define HYSTERISIS_DECIDEGC 20
static void
qpnp_chg_adc_notification(enum qpnp_tm_state state, void *ctx)
{
	struct qpnp_chg_chip *chip = ctx;
	bool bat_warm = 0, bat_cool = 0;
	int temp;

	if (state >= ADC_TM_STATE_NUM) {
		pr_err("invalid notification %d\n", state);
		return;
	}
#if defined(CONFIG_PANTECH_PMIC_CHARGER_SMB349) || defined(CONFIG_PANTECH_PMIC_CHARGER_SMB347)
	temp = get_prop_batt_temp();
#else
	temp = get_prop_batt_temp(chip);
#endif

	pr_debug("temp = %d state = %s\n", temp,
			state == ADC_TM_WARM_STATE ? "warm" : "cool");

	if (state == ADC_TM_WARM_STATE) {
		if (temp > chip->warm_bat_decidegc) {
			/* Normal to warm */
			bat_warm = true;
			bat_cool = false;
			chip->adc_param.low_temp =
				chip->warm_bat_decidegc - HYSTERISIS_DECIDEGC;
			chip->adc_param.state_request =
				ADC_TM_COOL_THR_ENABLE;
		} else if (temp >
				chip->cool_bat_decidegc + HYSTERISIS_DECIDEGC){
			/* Cool to normal */
			bat_warm = false;
			bat_cool = false;

			chip->adc_param.low_temp = chip->cool_bat_decidegc;
			chip->adc_param.high_temp = chip->warm_bat_decidegc;
			chip->adc_param.state_request =
					ADC_TM_HIGH_LOW_THR_ENABLE;
		}
	} else {
		if (temp < chip->cool_bat_decidegc) {
			/* Normal to cool */
			bat_warm = false;
			bat_cool = true;
			chip->adc_param.high_temp =
				chip->cool_bat_decidegc + HYSTERISIS_DECIDEGC;
			chip->adc_param.state_request =
				ADC_TM_WARM_THR_ENABLE;
		} else if (temp <
				chip->warm_bat_decidegc - HYSTERISIS_DECIDEGC){
			/* Warm to normal */
			bat_warm = false;
			bat_cool = false;

			chip->adc_param.low_temp = chip->cool_bat_decidegc;
			chip->adc_param.high_temp = chip->warm_bat_decidegc;
			chip->adc_param.state_request =
					ADC_TM_HIGH_LOW_THR_ENABLE;
		}
	}

	if (chip->bat_is_cool ^ bat_cool || chip->bat_is_warm ^ bat_warm) {
		chip->bat_is_cool = bat_cool;
		chip->bat_is_warm = bat_warm;

		if (bat_cool || bat_warm)
			chip->resuming_charging = false;

		/**
		 * set appropriate voltages and currents.
		 *
		 * Note that when the battery is hot or cold, the charger
		 * driver will not resume with SoC. Only vbatdet is used to
		 * determine resume of charging.
		 */
		qpnp_chg_set_appropriate_vddmax(chip);
		qpnp_chg_set_appropriate_battery_current(chip);
		qpnp_chg_set_appropriate_vbatdet(chip);
	}

	if (qpnp_adc_tm_channel_measure(&chip->adc_param))
		pr_err("request ADC error\n");

	power_supply_changed(&chip->batt_psy);
}

static int
qpnp_dc_power_set_property(struct power_supply *psy,
				  enum power_supply_property psp,
				  const union power_supply_propval *val)
{
	struct qpnp_chg_chip *chip = container_of(psy, struct qpnp_chg_chip,
								dc_psy);
	int rc = 0;

	switch (psp) {
	case POWER_SUPPLY_PROP_CURRENT_MAX:
		if (!val->intval)
			break;

		rc = qpnp_chg_idcmax_set(chip, val->intval / 1000);
		if (rc) {
			pr_err("Error setting idcmax property %d\n", rc);
			return rc;
		}
		chip->maxinput_dc_ma = (val->intval / 1000);

		break;
	default:
		return -EINVAL;
	}

	power_supply_changed(&chip->dc_psy);
	return rc;
}

static int
qpnp_batt_power_set_property(struct power_supply *psy,
				  enum power_supply_property psp,
				  const union power_supply_propval *val)
{
	struct qpnp_chg_chip *chip = container_of(psy, struct qpnp_chg_chip,
								batt_psy);
	switch (psp) {
	case POWER_SUPPLY_PROP_CHARGING_ENABLED:
		chip->charging_disabled = !(val->intval);
#if defined(FEATURE_PANTECH_PMIC_CHARGING_DISABLE)
		pr_err(" sayuss cmd (%d)\n", chip->charging_disabled);
		// sayuss charge_CMD 
		smb_chg_disabled = chip->charging_disabled;
		if(smb_chg_disabled){ // sayuss charge_CMD 
			smb349_write_reg(0x30,0x81); //charging disable
			smb349_write_reg(0x31,0x00); //usb 100mA limit
		}
		else if(chip->usb_present && smb_chg_disabled==0)
		{
			if (!chip) {
				pr_err("the_chip is NULL\n");
				return 0;
			}
		 	is_temp_state_changed(&prev_state_mode, batt_temp);
			smb349_current_jeita(prev_state_mode, qpnp_get_cable_type());
			pr_err("charging_enable cable_index (%d)\n", qpnp_get_cable_type());
		}
#endif
		qpnp_chg_charge_en(chip, !chip->charging_disabled);
		qpnp_chg_force_run_on_batt(chip, chip->charging_disabled);
		break;
	case POWER_SUPPLY_PROP_SYSTEM_TEMP_LEVEL:
		qpnp_batt_system_temp_level_set(chip, val->intval);
		break;
	default:
		return -EINVAL;
	}

	power_supply_changed(&chip->batt_psy);
	return 0;
}

static void
qpnp_chg_setup_flags(struct qpnp_chg_chip *chip)
{
	if (chip->revision > 0 && chip->type == SMBB)
		chip->flags |= CHG_FLAGS_VCP_WA;
	if (chip->type == SMBB)
		chip->flags |= BOOST_FLASH_WA;
	if (chip->type == SMBBP)
		chip->flags |= BOOST_FLASH_WA;
}

static int
qpnp_chg_request_irqs(struct qpnp_chg_chip *chip)
{
	int rc = 0;
	struct resource *resource;
	struct spmi_resource *spmi_resource;
	u8 subtype;
	struct spmi_device *spmi = chip->spmi;

	spmi_for_each_container_dev(spmi_resource, chip->spmi) {
		if (!spmi_resource) {
				pr_err("qpnp_chg: spmi resource absent\n");
			return rc;
		}

		resource = spmi_get_resource(spmi, spmi_resource,
						IORESOURCE_MEM, 0);
		if (!(resource && resource->start)) {
			pr_err("node %s IO resource absent!\n",
				spmi->dev.of_node->full_name);
			return rc;
		}

		rc = qpnp_chg_read(chip, &subtype,
				resource->start + REG_OFFSET_PERP_SUBTYPE, 1);
		if (rc) {
			pr_err("Peripheral subtype read failed rc=%d\n", rc);
			return rc;
		}

	switch (subtype) {
	case SMBB_CHGR_SUBTYPE:
	case SMBBP_CHGR_SUBTYPE:
		case SMBCL_CHGR_SUBTYPE:
			chip->chg_fastchg.irq = spmi_get_irq_byname(spmi,
						spmi_resource, "fast-chg-on");
			if (chip->chg_fastchg.irq < 0) {
			pr_err("Unable to get fast-chg-on irq\n");
				return rc;
		}

			chip->chg_trklchg.irq = spmi_get_irq_byname(spmi,
						spmi_resource, "trkl-chg-on");
			if (chip->chg_trklchg.irq < 0) {
			pr_err("Unable to get trkl-chg-on irq\n");
				return rc;
		}

			chip->chg_failed.irq = spmi_get_irq_byname(spmi,
						spmi_resource, "chg-failed");
			if (chip->chg_failed.irq < 0) {
			pr_err("Unable to get chg_failed irq\n");
				return rc;
		}

			chip->chg_vbatdet_lo.irq = spmi_get_irq_byname(spmi,
						spmi_resource, "vbat-det-lo");
			if (chip->chg_vbatdet_lo.irq < 0) {
				pr_err("Unable to get fast-chg-on irq\n");
				return rc;
			}

			rc |= devm_request_irq(chip->dev, chip->chg_failed.irq,
				qpnp_chg_chgr_chg_failed_irq_handler,
				IRQF_TRIGGER_RISING, "chg-failed", chip);
		if (rc < 0) {
				pr_err("Can't request %d chg-failed: %d\n",
						chip->chg_failed.irq, rc);
				return rc;
		}

			rc |= devm_request_irq(chip->dev, chip->chg_fastchg.irq,
				qpnp_chg_chgr_chg_fastchg_irq_handler,
				IRQF_TRIGGER_RISING,
				"fast-chg-on", chip);
		if (rc < 0) {
				pr_err("Can't request %d fast-chg-on: %d\n",
						chip->chg_fastchg.irq, rc);
				return rc;
		}

			rc |= devm_request_irq(chip->dev, chip->chg_trklchg.irq,
				qpnp_chg_chgr_chg_trklchg_irq_handler,
				IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
				"trkl-chg-on", chip);
		if (rc < 0) {
				pr_err("Can't request %d trkl-chg-on: %d\n",
						chip->chg_trklchg.irq, rc);
				return rc;
		}

			rc |= devm_request_irq(chip->dev,
				chip->chg_vbatdet_lo.irq,
				qpnp_chg_vbatdet_lo_irq_handler,
				IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
				"vbat-det-lo", chip);
			if (rc < 0) {
				pr_err("Can't request %d vbat-det-lo: %d\n",
						chip->chg_vbatdet_lo.irq, rc);
				return rc;
			}

			enable_irq_wake(chip->chg_trklchg.irq);
			enable_irq_wake(chip->chg_failed.irq);
			qpnp_chg_disable_irq(&chip->chg_vbatdet_lo);
			enable_irq_wake(chip->chg_vbatdet_lo.irq);

			break;
		case SMBB_BAT_IF_SUBTYPE:
		case SMBBP_BAT_IF_SUBTYPE:
		case SMBCL_BAT_IF_SUBTYPE:
			chip->batt_pres.irq = spmi_get_irq_byname(spmi,
						spmi_resource, "batt-pres");
			if (chip->batt_pres.irq < 0) {
				pr_err("Unable to get batt-pres irq\n");
				return rc;
			}
			rc = devm_request_irq(chip->dev, chip->batt_pres.irq,
				qpnp_chg_bat_if_batt_pres_irq_handler,
				IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING
#if 1 // sayuss : we use below
				,"batt-pres", chip);
#else
				| IRQF_SHARED | IRQF_ONESHOT,
				"batt-pres", chip);
#endif
			if (rc < 0) {
				pr_err("Can't request %d batt-pres irq: %d\n",
						chip->batt_pres.irq, rc);
				return rc;
			}

			enable_irq_wake(chip->batt_pres.irq);

			chip->batt_temp_ok.irq = spmi_get_irq_byname(spmi,
						spmi_resource, "bat-temp-ok");
			if (chip->batt_temp_ok.irq < 0) {
				pr_err("Unable to get bat-temp-ok irq\n");
				return rc;
			}
			rc = devm_request_irq(chip->dev, chip->batt_temp_ok.irq,
				qpnp_chg_bat_if_batt_temp_irq_handler,
				IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
				"bat-temp-ok", chip);
			if (rc < 0) {
				pr_err("Can't request %d bat-temp-ok irq: %d\n",
						chip->batt_temp_ok.irq, rc);
				return rc;
			}

			enable_irq_wake(chip->batt_temp_ok.irq);

			break;
		case SMBB_USB_CHGPTH_SUBTYPE:
		case SMBBP_USB_CHGPTH_SUBTYPE:
		case SMBCL_USB_CHGPTH_SUBTYPE:
#if defined(FEATURE_PANTECH_PMIC_USBIN_DROP_WORKAROUND)
			msm_gpiomux_install(&sc_sysok, 1);
			gpio_request(SC_SYSOK, "sys_ok");
			chip->sysok_valid.irq = gpio_to_irq(SC_SYSOK);
			rc = request_irq(chip->sysok_valid.irq, qpnp_sysok_irq_handler,
				(IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING), "sysok_handler", chip);
#endif
			chip->usbin_valid.irq = spmi_get_irq_byname(spmi,
						spmi_resource, "usbin-valid");
			if (chip->usbin_valid.irq < 0) {
				pr_err("Unable to get usbin irq\n");
				return rc;
			}
			rc = devm_request_irq(chip->dev, chip->usbin_valid.irq,
				qpnp_chg_usb_usbin_valid_irq_handler,
				IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
					"usbin-valid", chip);
			if (rc < 0) {
				pr_err("Can't request %d usbin-valid: %d\n",
						chip->usbin_valid.irq, rc);
				return rc;
			}
#ifndef FEATURE_PANTECH_PMIC_BLOCK_CHG_GONE
			chip->chg_gone.irq = spmi_get_irq_byname(spmi,
						spmi_resource, "chg-gone");
			if (chip->chg_gone.irq < 0) {
				pr_err("Unable to get chg-gone irq\n");
				return rc;
			}
			rc = devm_request_irq(chip->dev, chip->chg_gone.irq,
				qpnp_chg_usb_chg_gone_irq_handler,
				IRQF_TRIGGER_RISING,
					"chg-gone", chip);
			if (rc < 0) {
				pr_err("Can't request %d chg-gone: %d\n",
						chip->chg_gone.irq, rc);
				return rc;
			}
#endif		
			enable_irq_wake(chip->usbin_valid.irq);
#ifndef FEATURE_PANTECH_PMIC_BLOCK_CHG_GONE
			enable_irq_wake(chip->chg_gone.irq);
#endif 

#if defined(FEATURE_PANTECH_PMIC_USBIN_DROP_WORKAROUND)
			enable_irq_wake(chip->sysok_valid.irq);
			qpnp_chg_disable_irq(&chip->usbin_valid);
#endif
			break;
		case SMBB_DC_CHGPTH_SUBTYPE:
			chip->dcin_valid.irq = spmi_get_irq_byname(spmi,
					spmi_resource, "dcin-valid");
			if (chip->dcin_valid.irq < 0) {
				pr_err("Unable to get dcin irq\n");
				return -rc;
			}
			rc = devm_request_irq(chip->dev, chip->dcin_valid.irq,
				qpnp_chg_dc_dcin_valid_irq_handler,
				IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
				"dcin-valid", chip);
			if (rc < 0) {
				pr_err("Can't request %d dcin-valid: %d\n",
						chip->dcin_valid.irq, rc);
				return rc;
			}

			enable_irq_wake(chip->dcin_valid.irq);
			break;
		}
	}

	return rc;
}

#define WDOG_EN_BIT	BIT(7)
static int
qpnp_chg_hwinit(struct qpnp_chg_chip *chip, u8 subtype,
				struct spmi_resource *spmi_resource)
{
	int rc = 0;
	u8 reg = 0;
	struct regulator_init_data *init_data;
	struct regulator_desc *rdesc;

	switch (subtype) {
	case SMBB_CHGR_SUBTYPE:
	case SMBBP_CHGR_SUBTYPE:
	case SMBCL_CHGR_SUBTYPE:
		rc = qpnp_chg_vinmin_set(chip, chip->min_voltage_mv);
		if (rc) {
			pr_debug("failed setting  min_voltage rc=%d\n", rc);
			return rc;
		}
		rc = qpnp_chg_vddmax_set(chip, chip->max_voltage_mv);
		if (rc) {
			pr_debug("failed setting max_voltage rc=%d\n", rc);
			return rc;
		}
		rc = qpnp_chg_vddsafe_set(chip, chip->safe_voltage_mv);
		if (rc) {
			pr_debug("failed setting safe_voltage rc=%d\n", rc);
			return rc;
		}
		rc = qpnp_chg_vbatdet_set(chip,
				chip->max_voltage_mv - chip->resume_delta_mv);
		if (rc) {
			pr_debug("failed setting resume_voltage rc=%d\n", rc);
			return rc;
		}
		rc = qpnp_chg_ibatmax_set(chip, chip->max_bat_chg_current);
		if (rc) {
			pr_debug("failed setting ibatmax rc=%d\n", rc);
			return rc;
		}
		if (chip->term_current) {
		rc = qpnp_chg_ibatterm_set(chip, chip->term_current);
		if (rc) {
			pr_debug("failed setting ibatterm rc=%d\n", rc);
			return rc;
		}
		}
		rc = qpnp_chg_ibatsafe_set(chip, chip->safe_current);
		if (rc) {
			pr_debug("failed setting ibat_Safe rc=%d\n", rc);
			return rc;
		}
		rc = qpnp_chg_tchg_max_set(chip, chip->tchg_mins);
		if (rc) {
			pr_debug("failed setting tchg_mins rc=%d\n", rc);
			return rc;
		}

		/* HACK: Disable wdog */
		rc = qpnp_chg_masked_write(chip, chip->chgr_base + 0x62,
			0xFF, 0xA0, 1);

		/* HACK: use analog EOC */
		rc = qpnp_chg_masked_write(chip, chip->chgr_base +
			CHGR_IBAT_TERM_CHGR,
			0xFF, 0x08, 1);

		break;
	case SMBB_BUCK_SUBTYPE:
	case SMBBP_BUCK_SUBTYPE:
	case SMBCL_BUCK_SUBTYPE:
		rc = qpnp_chg_toggle_chg_done_logic(chip, 0);
		if (rc)
			return rc;

		rc = qpnp_chg_masked_write(chip,
			chip->buck_base + CHGR_BUCK_BCK_VBAT_REG_MODE,
			BUCK_VBAT_REG_NODE_SEL_BIT,
			BUCK_VBAT_REG_NODE_SEL_BIT, 1);
		if (rc) {
			pr_debug("failed to enable IR drop comp rc=%d\n", rc);
			return rc;
		}
		break;
	case SMBB_BAT_IF_SUBTYPE:
	case SMBBP_BAT_IF_SUBTYPE:
	case SMBCL_BAT_IF_SUBTYPE:
		/* Select battery presence detection */
		switch (chip->bpd_detection) {
		case BPD_TYPE_BAT_THM:
			reg = BAT_THM_EN;
			break;
		case BPD_TYPE_BAT_ID:
			reg = BAT_ID_EN;
			break;
		case BPD_TYPE_BAT_THM_BAT_ID:
			reg = BAT_THM_EN | BAT_ID_EN;
			break;
		default:
			reg = BAT_THM_EN;
			break;
		}

		rc = qpnp_chg_masked_write(chip,
			chip->bat_if_base + BAT_IF_BPD_CTRL,
			BAT_IF_BPD_CTRL_SEL,
			reg, 1);
		if (rc) {
			pr_debug("failed to chose BPD rc=%d\n", rc);
			return rc;
		}
		/* Force on VREF_BAT_THM */
		rc = qpnp_chg_masked_write(chip,
			chip->bat_if_base + BAT_IF_VREF_BAT_THM_CTRL,
			VREF_BATT_THERM_FORCE_ON,
			VREF_BATT_THERM_FORCE_ON, 1);
		if (rc) {
			pr_debug("failed to force on VREF_BAT_THM rc=%d\n", rc);
			return rc;
		}
		break;
	case SMBB_USB_CHGPTH_SUBTYPE:
	case SMBBP_USB_CHGPTH_SUBTYPE:
	case SMBCL_USB_CHGPTH_SUBTYPE:
		if (qpnp_chg_is_usb_chg_plugged_in(chip)) {
			rc = qpnp_chg_masked_write(chip,
				chip->usb_chgpth_base + CHGR_USB_ENUM_T_STOP,
				ENUM_T_STOP_BIT,
				ENUM_T_STOP_BIT, 1);
			if (rc) {
				pr_err("failed to write enum stop rc=%d\n", rc);
				return -ENXIO;
			}
		}

		init_data = of_get_regulator_init_data(chip->dev,
						       spmi_resource->of_node);
		if (!init_data) {
			pr_err("unable to allocate memory\n");
			return -ENOMEM;
		}

		if (init_data->constraints.name) {
			if (of_get_property(chip->dev->of_node,
						"otg-parent-supply", NULL))
				init_data->supply_regulator = "otg-parent";

			rdesc			= &(chip->otg_vreg.rdesc);
			rdesc->owner		= THIS_MODULE;
			rdesc->type		= REGULATOR_VOLTAGE;
			rdesc->ops		= &qpnp_chg_otg_reg_ops;
			rdesc->name		= init_data->constraints.name;

			init_data->constraints.valid_ops_mask
				|= REGULATOR_CHANGE_STATUS;

			chip->otg_vreg.rdev = regulator_register(rdesc,
					chip->dev, init_data, chip,
					spmi_resource->of_node);
			if (IS_ERR(chip->otg_vreg.rdev)) {
				rc = PTR_ERR(chip->otg_vreg.rdev);
				chip->otg_vreg.rdev = NULL;
				if (rc != -EPROBE_DEFER)
					pr_err("OTG reg failed, rc=%d\n", rc);
				return rc;
			}
		}

		rc = qpnp_chg_masked_write(chip,
			chip->usb_chgpth_base + USB_OVP_CTL,
			USB_VALID_DEB_20MS,
			USB_VALID_DEB_20MS, 1);

		rc = qpnp_chg_masked_write(chip,
			chip->usb_chgpth_base + CHGR_USB_ENUM_T_STOP,
			ENUM_T_STOP_BIT,
			ENUM_T_STOP_BIT, 1);

		rc = qpnp_chg_masked_write(chip,
			chip->usb_chgpth_base + SEC_ACCESS,
			0xFF,
			0xA5, 1);

		rc = qpnp_chg_masked_write(chip,
			chip->usb_chgpth_base + USB_CHG_GONE_REV_BST,
			0xFF,
			0x80, 1);

		break;
	case SMBB_DC_CHGPTH_SUBTYPE:
		break;
	case SMBB_BOOST_SUBTYPE:
	case SMBBP_BOOST_SUBTYPE:
		init_data = of_get_regulator_init_data(chip->dev,
					       spmi_resource->of_node);
		if (!init_data) {
			pr_err("unable to allocate memory\n");
			return -ENOMEM;
		}

		if (init_data->constraints.name) {
			if (of_get_property(chip->dev->of_node,
						"boost-parent-supply", NULL))
				init_data->supply_regulator = "boost-parent";

			rdesc			= &(chip->boost_vreg.rdesc);
			rdesc->owner		= THIS_MODULE;
			rdesc->type		= REGULATOR_VOLTAGE;
			rdesc->ops		= &qpnp_chg_boost_reg_ops;
			rdesc->name		= init_data->constraints.name;

			init_data->constraints.valid_ops_mask
				|= REGULATOR_CHANGE_STATUS
					| REGULATOR_CHANGE_VOLTAGE;

			chip->boost_vreg.rdev = regulator_register(rdesc,
					chip->dev, init_data, chip,
					spmi_resource->of_node);
			if (IS_ERR(chip->boost_vreg.rdev)) {
				rc = PTR_ERR(chip->boost_vreg.rdev);
				chip->boost_vreg.rdev = NULL;
				if (rc != -EPROBE_DEFER)
					pr_err("boost reg failed, rc=%d\n", rc);
				return rc;
			}
		}
		break;
	case SMBB_MISC_SUBTYPE:
	case SMBBP_MISC_SUBTYPE:
	case SMBCL_MISC_SUBTYPE:
		if (subtype == SMBB_MISC_SUBTYPE)
			chip->type = SMBB;
		else if (subtype == SMBBP_MISC_SUBTYPE)
			chip->type = SMBBP;
		else if (subtype == SMBCL_MISC_SUBTYPE)
		chip->type = SMBCL;

		pr_debug("Setting BOOT_DONE\n");
		rc = qpnp_chg_masked_write(chip,
			chip->misc_base + CHGR_MISC_BOOT_DONE,
			CHGR_BOOT_DONE, CHGR_BOOT_DONE, 1);
		rc = qpnp_chg_read(chip, &reg,
				 chip->misc_base + MISC_REVISION2, 1);
		if (rc) {
			pr_err("failed to read revision register rc=%d\n", rc);
			return rc;
		}

		chip->revision = reg;
		break;
	default:
		pr_err("Invalid peripheral subtype\n");
	}
	return rc;
}
#if defined(FEATURE_PANTECH_PMIC_BMS_TEST)
static int get_batt_mvolts(void)
{
#if defined(CONFIG_PANTECH_PMIC_MAX17058)
	if (max17058_uses)
		return get_max17058_voltage() / 1000;
	else
		return (the_chip->current_uvolt /1000);
#else
	//return (the_chip->current_uvolt /1000);
	return max17058_get_voltage() / 1000;
#endif
}

//20130521 djjeon, powerlog add ICHG
static int get_batt_chg_current(struct qpnp_chg_chip *chip)
{
	struct qpnp_vadc_result results;
	int rc, try_max = 0;

	do{
		rc = qpnp_vadc_read(P_MUX2_1_1, &results);
		if(rc == -EINVAL)
			return -EINVAL;
		try_max++;
	}while(rc && (try_max < 20));
	if(!rc){
		if(the_qpnp_chip->usb_present)
#if defined(CONFIG_PANTECH_PMIC_CHARGER_SMB349)
			return ((int)results.physical)*2/1000;
#else
			return ((int)results.physical)*1/750;
#endif
	else
			return -1;
	}else{
		return 0;
	}

}
#endif
#if defined(FEATURE_PANTECH_PMIC_BMS_TEST)
static void bms_init_set(struct qpnp_chg_chip *chip)
{
}

static ssize_t bms_input_show_flag(struct device *dev, struct device_attribute *attr, char *buf)
{
	int enable;
	enable = atomic_read(&bms_input_flag);
	return sprintf(buf, "%d\n", enable);
}

static ssize_t bms_input_store_flag(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	u8 scale = (u8)simple_strtoul(buf, NULL, 10);	
	atomic_set(&bms_input_flag, scale);
	return count;
}

static ssize_t bms_cutoff_show_flag(struct device *dev, struct device_attribute *attr, char *buf)
{
	int enable;
	enable = atomic_read(&bms_cutoff_flag);
	return sprintf(buf, "%d\n", enable);
}

static ssize_t bms_cutoff_store_flag(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	u8 scale = (u8)simple_strtoul(buf, NULL, 10);
	atomic_set(&bms_cutoff_flag, scale);
	return count;
}

#if defined(PANTECH_USED_ALL_OTHER_WRITABLE_FILES)
static DEVICE_ATTR(setflag, S_IWUGO | S_IRUGO, bms_input_show_flag, bms_input_store_flag);
static DEVICE_ATTR(cutoff, S_IWUGO | S_IRUGO, bms_cutoff_show_flag, bms_cutoff_store_flag);
#else
static DEVICE_ATTR(setflag, 0644, bms_input_show_flag, bms_input_store_flag);
static DEVICE_ATTR(cutoff, 0644, bms_cutoff_show_flag, bms_cutoff_store_flag);
#endif /* PANTECH_USED_ALL_OTHER_WRITABLE_FILES */

static struct attribute *bms_input_attrs[] = {
	&dev_attr_setflag.attr,
	&dev_attr_cutoff.attr,
	NULL,
};

static struct attribute_group bms_input_attr_group = {
	.attrs = bms_input_attrs,
};
#endif


#if defined(FEATURE_PANTECH_PMIC_MONITOR_TEST)
static char *str_cable_type[] = {
		"NO_CABLE", "STANDARD_CABLE", "FACTORY_CABLE", "TA_CABLE", "WIRELESS_CABLE",
		"UNKNOWN_CABLE", "INVALID_CABLE",
};

static int proc_debug_pm_chg_get_fsm_state(char *page, char **start, off_t offset,
					int count, int *eof, void *data)
{
	int fsm_state;

	if (!the_qpnp_chip) {
		pr_err("the_chip is NULL\n");
		return 0;
	}

	fsm_state = 0;  //imsi charging state
	*eof = 1;

	return sprintf(page, "%d\n", fsm_state);
}

static int proc_debug_pm_chg_get_I_USBMax(char *page, char **start, off_t offset,
					int count, int *eof, void *data)
{
	if (!the_qpnp_chip) {
		pr_err("the_chip is NULL\n");
		return 0;
	}
	*eof = 1;  

	return sprintf(page, "%d\n", the_qpnp_chip->maxinput_usb_ma);
}

static int proc_debug_pm_chg_get_I_BattMax(char *page, char **start, off_t offset,
					int count, int *eof, void *data)
{
#if (defined(T_EF59S) ||  defined(T_EF59K) || defined(T_EF59L)) && (BOARD_VER >= TP10)
	int battmax;
#endif
	if (!the_qpnp_chip) {
		pr_err("the_chip is NULL\n");
		return 0;
	}
	
	*eof = 1;  
#if (defined(T_EF59S) ||  defined(T_EF59K) || defined(T_EF59L)) && (BOARD_VER >= TP10)
	if(qpnp_get_cable_type() == PANTECH_USB){
		if(the_qpnp_chip->maxinput_usb_ma == 900)
			battmax = 900;
		else
			battmax = 500;
	}else if(qpnp_get_cable_type() == PANTECH_AC)
		battmax = 2000;

	return sprintf(page, "%d\n", battmax);
#else
	return sprintf(page, "%d\n", the_qpnp_chip->maxinput_dc_ma); // sayuss monitoring current
#endif
}

static int proc_debug_pm_chg_get_I_BattCurrent(char *page, char **start, off_t offset,
					int count, int *eof, void *data)
{
	int ichg_meas_ma;

	if (!the_qpnp_chip) {
		pr_err("the_chip is NULL\n");
		return 0;
	}
#if defined(CONFIG_PANTECH_PMIC_CHARGER_SMB349) || defined(CONFIG_PANTECH_PMIC_CHARGER_SMB347)//20130524 djjeon VCHG=current
	ichg_meas_ma =((int)get_batt_chg_current(the_qpnp_chip));
#else
	ichg_meas_ma = (get_prop_current_now(the_qpnp_chip)) / 1000;
#endif 
	*eof = 1;  

	return sprintf(page, "%d\n", ichg_meas_ma);  //batt charging current
}

static int proc_debug_pm_chg_get_BattID(char *page, char **start, off_t offset,
					int count, int *eof, void *data)
{
	int64_t rc;
	struct qpnp_vadc_result results;

	if (!the_qpnp_chip) {
		pr_err("the_chip is NULL\n");
		return 0;
	}

	rc = qpnp_vadc_read(LR_MUX2_BAT_ID, &results);//LR_MUX2_PU1_PU2_BAT_ID, &results);		20130604 djjeon channel modify
	*eof = 1;  

	return sprintf(page, "%lld\n", results.physical);
}

static int proc_debug_pm_chg_get_CableID(char *page, char **start, off_t offset,
					int count, int *eof, void *data)
{
	int64_t rc;
	struct qpnp_vadc_result results;
	if (!the_qpnp_chip) {
		pr_err("the_chip is NULL\n");
		return 0;
	}

	*eof = 1;  
		
	if (the_qpnp_chip->usb_present)
       rc = qpnp_vadc_read(LR_MUX10_PU2_AMUX_USB_ID_LV, &results);
       else
	   		results.physical = 0;  
	return sprintf(page, "%lld\n", results.physical );  //imsi charging cable ID 
}

static int proc_debug_pm_chg_get_CableType(char *page, char **start, off_t offset,
					int count, int *eof, void *data)
{
	int cable_index = 0;
	union power_supply_propval ret = {0,};

        the_qpnp_chip->usb_psy->get_property(the_qpnp_chip->usb_psy,
                          POWER_SUPPLY_PROP_TYPE, &ret);
	if(4 == ret.intval)
		cable_index = 1;
	else if(4 < ret.intval && 7 > ret.intval)
		cable_index = 3;
		
		
    //    pr_info("sayuss : chg_type = %d====================\n", ret.intval);

	if (!the_qpnp_chip) {
		pr_err("the_chip is NULL\n");
		return 0;
	}

	*eof = 1;  

	return sprintf(page, "%s\n", str_cable_type[cable_index]); //imsi  charging type 
}

static int proc_debug_pm_chg_get_pm_chg_test(char *page, char **start, off_t offset,
					int count, int *eof, void *data)
{
	if(!the_qpnp_chip) {
		pr_err("the_chip is NULL\n");
		return 0;
	}

	*eof = 1;  
	return sprintf(page, "%d\n", the_qpnp_chip->charging_disabled);
}
//+++ 20130806, P14787, djjeon, charging setting stability test
static int proc_debug_pm_chg_get_charging_setting(char *page, char **start, off_t offset,
					int count, int *eof, void *data)
{
	if(!the_qpnp_chip) {
		pr_err("the_chip is NULL\n");
		return 0;
	}

	*eof = 1;  
	return sprintf(page, "%d\n", the_qpnp_chip->charging_setting);
}
//--- 20130806, P14787, djjeon, charging setting stability test

static int proc_debug_pm_chg_get_aicl(char *page, char **start, off_t offset,
					int count, int *eof, void *data)
{
    unsigned char value;
	*eof = 1;  
    value = smb349_get_reg(0x3F);
	return sprintf(page, "%d\n", value);
}

static int proc_debug_pm_chg_get_aicl_detail(char *page, char **start, off_t offset,
					int count, int *eof, void *data)
{
    char temp[32];
    int aicl_results[] = { 300, 500, 700, 900, 1200, 1500, 1800, 2000, 2200, 2500, 2500, 2500, 2500, 2500, 2500, 2500 };
    int aicl_status_bit = 0;
    unsigned char value;

	*eof = 1;  
    value = smb349_get_reg(0x3F);

    if(value&0x80) {
        aicl_status_bit |= 0x80;
    }

    switch(value&0x60) {
    case 0x60:
        sprintf(temp, "N/A");
        break;
    case 0x40:
        sprintf(temp, "USB1 or USB1.5 Mode");
        break;
    case 0x20:
        sprintf(temp, "USB5 or USB9 Mode");
        break;
    case 0x00:
        sprintf(temp, "High-Current(HC) Mode");
        break;
    }

    if(value&0x10) {
        aicl_status_bit |= 0x10;
    }

	return sprintf(page, "USBIN Input : %s\nMode : %s\nStatus : %s\nResults : %dmA\n", 
                   aicl_status_bit & 0x80 ? "In Use" : "Not in Use",
                   temp, 
                   aicl_status_bit & 0x10 ? "Completed" : "Not Completed",
                   aicl_status_bit & 0x10 ? aicl_results[(value&0x0f)] : -1 
                   );
}

static unsigned char smb_get_reg_state = 0;
static int proc_debug_smb_read_regs(char *page, char **start, off_t offset,
					int count, int *eof, void *data)
{
    char *buffer = page;
    unsigned char value = 0;
    int rtn_value = 0;
    if(offset == 0) {
        value = smb349_get_reg(smb_get_reg_state);
        pr_info("%s: smb_get_reg_state=%02x value=%02x\n", __func__, smb_get_reg_state, value);
        buffer += sprintf(buffer, "%d\n", value);
        rtn_value = buffer - page;
    }
	return rtn_value;
}

static int proc_debug_smb_write_regs(struct file *file, const char __user *buffer,
			   unsigned long count, void *data)
{
    char temp[8];

    if(copy_from_user(temp, buffer, count)) {
        pr_err("%s: Failed to copy from user\n", __func__);
        return -EFAULT;
    }

    smb_get_reg_state = (unsigned char)simple_strtoul(temp, NULL, 0);
    if(smb_get_reg_state > 0xff) 
        smb_get_reg_state = 0xff;
    pr_info("%s: smb_get_reg_state=%02x\n", __func__, smb_get_reg_state);

    return count;
}
#endif


#if defined(FEATURE_PANTECH_PMIC_BATTERY_CHARGING_DISCHARGING_TEST) || defined(FEATURE_PANTECH_PMIC_MONITOR_TEST)
static int pm8941_charger_test_misc_open(struct inode *inode, struct file *file)
{
	return 0;
}

static long pm8941_charger_test_misc_ioctl(struct file *file,
		    unsigned int cmd, unsigned long arg)
{
	int rc;
	uint32_t n;
	pr_err("cmd = [%x]\n",cmd);

	switch (cmd) {
#if defined(FEATURE_PANTECH_PMIC_BATTERY_CHARGING_DISCHARGING_TEST)
	case PM8941_CHARER_TEST_SET_CHARING_CTL:
		rc = copy_from_user(&n, (void *)arg, sizeof(n));
		if (!rc) {
			if (n)
				charging_disabled = 1;
			else
				charging_disabled = 0;

			if (the_qpnp_chip) {
                            qpnp_chg_charge_en(the_qpnp_chip, !the_qpnp_chip->charging_disabled);
			}

			pr_err("SET_CHARING_CTL charging_disabled[%d]\n",the_qpnp_chip->charging_disabled);
		}

		break;
		
	case PM8941_CHARER_TEST_GET_CHARING_CTL:
		if (copy_to_user((void *)arg, &the_qpnp_chip->charging_disabled, sizeof(int)))
			rc = -EINVAL;
		else
			rc = 0;

		break;
#endif

#if defined(FEATURE_PANTECH_PMIC_MONITOR_TEST)
	case PM8941_CHARGER_TEST_SET_PM_CHG_TEST:
		rc = copy_from_user(&n, (void *)arg, sizeof(n));
		if (!rc) {
			if (n)
				the_qpnp_chip->charging_disabled = 1;
			else
				the_qpnp_chip->charging_disabled = 0;

			if (the_qpnp_chip) {
				qpnp_chg_charge_en(the_qpnp_chip, !the_qpnp_chip->charging_disabled);
			}
			
			pr_err("SET_PM_CHG_TEST charging_disabled [%d]\n", the_qpnp_chip->charging_disabled);
		}	
		break;
//+++ 20130806, P14787, djjeon, charging setting stability test		
	case PM8941_CHARGER_TEST_CHARGING_SETTING:
		rc = copy_from_user(&n, (void *)arg, sizeof(n));
		if (!rc) {
			the_qpnp_chip->charging_setting = n;
#if defined(CONFIG_PANTECH_PMIC_CHARGER_SMB349)	|| defined(CONFIG_PANTECH_PMIC_CHARGER_SMB347)			
			smb349_test_limit_up(n);
#endif
			otg_detect_control_test(n);
			pr_debug("SET_CHARGING_SETTING charging_setting [%d]\n", n);
		}
		else
			pr_err("SET_CHARGING_SETTING read error \n");
		break;
//--- 20130806, P14787, djjeon, charging setting stability test		
#endif
		
	default:
		rc = -EINVAL;
	}

	return rc;
}

static int pm8941_charger_battery_test_misc_release(struct inode *inode, struct file *file)
{
	return 0;
}

static struct file_operations pm8941_charger_test_dev_fops = {
	.owner = THIS_MODULE,
	.open = pm8941_charger_test_misc_open,
	.unlocked_ioctl	= pm8941_charger_test_misc_ioctl,
	.release	= pm8941_charger_battery_test_misc_release,
};

struct miscdevice pm8941_charger_test_misc_device = {
	.minor	= MISC_DYNAMIC_MINOR,
	.name	= "qcom,qpnp-charger",
	.fops	= &pm8941_charger_test_dev_fops,
};
int pm8941_charger_battery_charging_test_init(void)
{
	return misc_register(&pm8941_charger_test_misc_device);
}
#endif

#if defined(FEATURE_PANTECH_PMIC_MONITOR_TEST)
void pm8941_charger_test_charger_monitor_init(struct qpnp_chg_chip *chip)
{
	struct proc_dir_entry *ent;

	if (!chip)
		return;

	chip->pm_chg_test = false;
	chip->cable_adc = 0;
	chip->charging_setting = false;	//+++ 20130806, P14787, djjeon, charging setting stability test

	pm8941_charger_dir = proc_mkdir(QPNP_CHARGER_DEV_NAME, NULL);
	if (!pm8941_charger_dir) {
		pr_err("Unable to create /proc/%s directory\n",QPNP_CHARGER_DEV_NAME);
		return;
	}

	ent = create_proc_entry("fsm_state", 0, pm8941_charger_dir);

	if (!ent) {
		pr_err("Unable to create /proc/fsm_state entry\n");
		return;
	}
	ent->read_proc = proc_debug_pm_chg_get_fsm_state;

	ent = create_proc_entry("cable_type", 0, pm8941_charger_dir);
	if (!ent) {
		pr_err("Unable to create /proc/cable_type entry\n");
		return;
	}
	ent->read_proc = proc_debug_pm_chg_get_CableType;

	ent = create_proc_entry("cable_id", 0, pm8941_charger_dir);
	if (!ent) {
		pr_err("Unable to create /proc/cable_id entry\n");
		return;
	}
	ent->read_proc = proc_debug_pm_chg_get_CableID;

	ent = create_proc_entry("batt_id", 0, pm8941_charger_dir);
	if (!ent) {
		pr_err("Unable to create /proc/batt_id entry\n");
		return;
	}
	ent->read_proc = proc_debug_pm_chg_get_BattID;

	ent = create_proc_entry("i_usbmax", 0, pm8941_charger_dir);
	if (!ent) {
		pr_err("Unable to create /proc/i_usbmax entry\n");
		return;
	}
	ent->read_proc = proc_debug_pm_chg_get_I_USBMax;

	ent = create_proc_entry("i_battmax", 0, pm8941_charger_dir);
	if (!ent) {
		pr_err("Unable to create /proc/i_battmax entry\n");
		return;
	}
	ent->read_proc = proc_debug_pm_chg_get_I_BattMax;

	ent = create_proc_entry("i_battcurr", 0, pm8941_charger_dir);
	if (!ent) {
		pr_err("Unable to create /proc/i_battcurr entry\n");
		return;
	}
	ent->read_proc = proc_debug_pm_chg_get_I_BattCurrent;

	ent = create_proc_entry("pm_chg_test", 0, pm8941_charger_dir);
	if (!ent) {
		pr_err("Unable to create /proc/pm_chg_test entry\n");
		return;
	}
	ent->read_proc = proc_debug_pm_chg_get_pm_chg_test;      
//+++ 20130806, P14787, djjeon, charging setting stability test
	ent = create_proc_entry("charging_setting", 0, pm8941_charger_dir);
	if (!ent) {
		pr_err("Unable to create /proc/charging_setting entry\n");
		return;
	}
	ent->read_proc = proc_debug_pm_chg_get_charging_setting;      
//--- 20130806, P14787, djjeon, charging setting stability test	

	ent = create_proc_entry("aicl", 0, pm8941_charger_dir);
	if (!ent) {
		pr_err("Unable to create /proc/%s/aicl entry\n", QPNP_CHARGER_DEV_NAME);
		return;
}
	ent->read_proc = proc_debug_pm_chg_get_aicl;

	ent = create_proc_entry("aicl_detail", 0, pm8941_charger_dir);
	if (!ent) {
		pr_err("Unable to create /proc/%s/aicl_detail entry\n", QPNP_CHARGER_DEV_NAME);
		return;
	}
	ent->read_proc = proc_debug_pm_chg_get_aicl_detail;

	ent = create_proc_entry("regs", 0666, pm8941_charger_dir);
	if (!ent) {
		pr_err("Unable to create /proc/%s/regs entry\n", QPNP_CHARGER_DEV_NAME);
		return;
	}
	ent->read_proc = proc_debug_smb_read_regs;
	ent->write_proc = proc_debug_smb_write_regs;
}
#endif

#if defined(FEATURE_PANTECH_PMIC_EOC)
#if defined(T_EF59S) || defined(T_EF59K) || defined(T_EF59L) 
#define EOC_START 99
#define EOC_END	  101	
#elif defined(T_EF60S) || defined(T_EF61K) || defined(T_EF62L)
#define EOC_START 97
#define EOC_END	  99	
#else
#define EOC_START 98
#define EOC_END	  100	
#endif
#endif

#if defined(CONFIG_PANTECH_PMIC_CHARGER_SMB349) || defined(CONFIG_PANTECH_PMIC_CHARGER_SMB347)
static void check_temperature_worker(struct work_struct *work)				 
{	
	int rc = 0;	
#if defined(FEATURE_PANTECH_PMIC_AUTO_PWR_ON)	//djjeon 20130610 add	
	static int socval = -1;
#endif
#if defined(FEATURE_PANTECH_PMIC_BMS_TEST)
		int enable, soc=-1, temp_status = 0;
#endif
	struct qpnp_vadc_result results;
	struct delayed_work *dwork = to_delayed_work(work);
	struct qpnp_chg_chip *chip = container_of(dwork,
				struct qpnp_chg_chip, update_heartbeat_work);
	rc = qpnp_vadc_read(LR_MUX1_BATT_THERM, &results);
	if (rc) {
		pr_debug("Unable to read batt temperature rc=%d\n", rc);
		return ;
	}
#if defined(FEATURE_PANTECH_PMIC_AUTO_PWR_ON)	//djjeon 20130610 add
	soc = max17058_get_soc();
	if(soc != socval){
		socval = soc;
		pr_err("changed soc [%d]\n",socval);
	}
	if(get_is_offline_charging_mode())
		if (get_auto_power_on_flag()){
			soc = max17058_get_soc();
			if(5<soc){
				pr_debug("======= machine_restart[%d]\n",soc);
				machine_restart(NULL);
			}
			else
				pr_debug("======= Not yet Auto Power ON[%d]\n",soc);
		}
#endif

#if defined(FEATURE_PANTECH_PMIC_SHARED_DATA)
	if(is_fatctory_dummy_connect())
	batt_temp = 250;
	else
	batt_temp = results.physical;	
#else
	batt_temp = results.physical;
#endif
	if(is_temp_state_changed(&prev_state_mode, batt_temp)){
		smb349_current_jeita(prev_state_mode, qpnp_get_cable_type());
		temp_status = 1;
	}
#if defined(FEATURE_PANTECH_PMIC_CHARGE_LIMIT_WITH_TSENSOR)
	if(prev_state_mode == BATT_THERM_NORMAL){
		if(t_sensor_value() == T_SENSE_UP)
			smb349_t_sense_limit(true);
		else if(t_sensor_value() == T_SENSE_DOWN){
			smb349_t_sense_limit(false);
		}
	}else if(T_sensor_mode == true){
		smb349_t_sense_limit(false);
		T_sensor_mode = false;
	}	
#endif
	
	//	pr_err("======= sayuss Temp[%lld]\n", batt_temp);
#if defined(CONFIG_PANTECH_PMIC_FUELGAUGE_MAX17058)
	max17058_update_rcomp(batt_temp);
#endif

#if defined(FEATURE_PANTECH_PMIC_CHARGE_LIMIT_WITH_LCD)
if(get_is_offline_charging_mode() && ((qpnp_get_cable_type()== PANTECH_USB) || (qpnp_get_cable_type() == PANTECH_AC))){
		pr_err("_____________ sayuss offline_boot_ok=%d,chip->usb_present=%d,offline_boot_ok=%d\n",offline_boot_ok,chip->usb_present,offline_boot_ok);
if(chip->usb_present && offline_boot_ok){
	if(4==boot_lcd_cnt){
		{
			boot_lcd_cnt = 10;
			smb349_current_jeita(prev_state_mode,qpnp_get_cable_type());
			}
		}
	}
}
#endif
	
#if defined(FEATURE_PANTECH_PMIC_BMS_TEST)
		enable = atomic_read(&bms_input_flag);
		soc = max17058_get_soc();//pm8921_bms_get_percent(max17058_uses);

		if (enable){
				pr_debug("Charging Log soc (%d)", soc);
				chip->charge_output_voltage = get_batt_chg_current(chip);			//20130521 djjeon, powerlog add ICHG
				input_report_rel(bms_input_dev, REL_RX, soc);
				input_report_rel(bms_input_dev, REL_RY, get_batt_mvolts());
				input_report_rel(bms_input_dev, REL_RZ, batt_temp);	
				input_report_rel(bms_input_dev, REL_X, chip->charge_output_voltage);		//20130521 djjeon, powerlog add ICHG	
				input_report_rel(bms_input_dev, REL_Y, max17058_FG_SOC());		// real soc, 20131030 p13787 ryu		
				input_sync(bms_input_dev);
			}
	
#endif
#if defined(FEATURE_PANTECH_PMIC_EOC)
if(chip->usb_present){
	if((max17058_FG_SOC() <= EOC_START) && chip->end_recharing){
		smb349_write_reg(0x04, 0xEE);
		smb349_write_reg(0x30,0x81); //charging disable
		smb349_mode_setting();
		smb349_write_reg(0x30,0x83); //charging disable
		smb349_mode_setting();
		chip->end_recharing = false;
		pr_debug("end_recharing %d\n", chip->end_recharing);
	}else if(max17058_FG_SOC() >= EOC_END && !chip->end_recharing){
		smb349_write_reg(0x04, 0xAE);
		chip->end_recharing = true;
		pr_debug("end_recharing %d\n", chip->end_recharing);
	}
}
#endif
#if defined (FEATURE_PANTECH_PMIC_ABNORMAL)
if(chip->usb_present && qpnp_get_cable_type() == PANTECH_CABLE_NONE && !chip->check_cable_disconnect_work){
			schedule_delayed_work(&chip->update_abnormal_cable_disconeect_work,
			     round_jiffies_relative(msecs_to_jiffies(90000)));		
			chip->check_cable_disconnect_work = true;	
}
#endif
		power_supply_changed(&chip->batt_psy);
	
#if defined(FEATURE_POWER_ON_OFF_TEST)
	if(is_offline_charging_mode==1){
		power_worker_count++;
		if(power_worker_count>6)
			schedule_delayed_work(&pwr_test->power_key_emulation_work, round_jiffies_relative(msecs_to_jiffies(1000)));
	}
#endif

	schedule_delayed_work(&chip->update_heartbeat_work,
			round_jiffies_relative(msecs_to_jiffies(5000)));
}
#endif
#define OF_PROP_READ(chip, prop, qpnp_dt_property, retval, optional)	\
do {									\
	if (retval)							\
		break;							\
									\
	retval = of_property_read_u32(chip->spmi->dev.of_node,		\
					"qcom," qpnp_dt_property,	\
					&chip->prop);			\
									\
	if ((retval == -EINVAL) && optional)				\
		retval = 0;						\
	else if (retval)						\
		pr_err("Error reading " #qpnp_dt_property		\
				" property rc = %d\n", rc);		\
} while (0)

static int
qpnp_charger_read_dt_props(struct qpnp_chg_chip *chip)
{
	int rc = 0;
	const char *bpd;

	OF_PROP_READ(chip, max_voltage_mv, "vddmax-mv", rc, 0);
	OF_PROP_READ(chip, min_voltage_mv, "vinmin-mv", rc, 0);
	OF_PROP_READ(chip, safe_voltage_mv, "vddsafe-mv", rc, 0);
	OF_PROP_READ(chip, resume_delta_mv, "vbatdet-delta-mv", rc, 0);
	OF_PROP_READ(chip, safe_current, "ibatsafe-ma", rc, 0);
	OF_PROP_READ(chip, max_bat_chg_current, "ibatmax-ma", rc, 0);
	if (rc)
		pr_err("failed to read required dt parameters %d\n", rc);

	OF_PROP_READ(chip, term_current, "ibatterm-ma", rc, 1);
	OF_PROP_READ(chip, maxinput_dc_ma, "maxinput-dc-ma", rc, 1);
	OF_PROP_READ(chip, maxinput_usb_ma, "maxinput-usb-ma", rc, 1);
	OF_PROP_READ(chip, warm_bat_decidegc, "warm-bat-decidegc", rc, 1);
	OF_PROP_READ(chip, cool_bat_decidegc, "cool-bat-decidegc", rc, 1);
	OF_PROP_READ(chip, tchg_mins, "tchg-mins", rc, 1);
	OF_PROP_READ(chip, hot_batt_p, "batt-hot-percentage", rc, 1);
	OF_PROP_READ(chip, cold_batt_p, "batt-cold-percentage", rc, 1);
	OF_PROP_READ(chip, soc_resume_limit, "resume-soc", rc, 1);

	if (rc)
		return rc;

	rc = of_property_read_string(chip->spmi->dev.of_node,
		"qcom,bpd-detection", &bpd);
	if (rc) {
		/* Select BAT_THM as default BPD scheme */
		chip->bpd_detection = BPD_TYPE_BAT_THM;
	} else {
		chip->bpd_detection = get_bpd(bpd);
		if (chip->bpd_detection < 0) {
			pr_err("failed to determine bpd schema %d\n", rc);
			return rc;
		}
	}

	/* Look up JEITA compliance parameters if cool and warm temp provided */
	if (chip->cool_bat_decidegc && chip->warm_bat_decidegc) {
		rc = qpnp_adc_tm_is_ready();
		if (rc) {
			pr_err("tm not ready %d\n", rc);
			return rc;
		}

		OF_PROP_READ(chip, warm_bat_chg_ma, "ibatmax-warm-ma", rc, 1);
		OF_PROP_READ(chip, cool_bat_chg_ma, "ibatmax-cool-ma", rc, 1);
		OF_PROP_READ(chip, warm_bat_mv, "warm-bat-mv", rc, 1);
		OF_PROP_READ(chip, cool_bat_mv, "cool-bat-mv", rc, 1);
		if (rc)
			return rc;
		}

	/* Get the btc-disabled property */
	chip->btc_disabled = of_property_read_bool(chip->spmi->dev.of_node,
					"qcom,btc-disabled");

	/* Get the charging-disabled property */
	chip->charging_disabled = 0;  // of_property_read_bool(chip->spmi->dev.of_node,	"qcom,charging-disabled");

	/* Get the duty-cycle-100p property */
	chip->duty_cycle_100p = of_property_read_bool(
					chip->spmi->dev.of_node,
					"qcom,duty-cycle-100p");
	if (chip->duty_cycle_100p) {
		rc = qpnp_buck_set_100_duty_cycle_enable(chip, 1);
		if (rc) {
			pr_err("failed to enable duty cycle %d\n", rc);
			return rc;
		}
	}

	/* Get the fake-batt-values property */
	chip->use_default_batt_values =
			of_property_read_bool(chip->spmi->dev.of_node,
					"qcom,use-default-batt-values");

	/* Disable charging when faking battery values */
	if (chip->use_default_batt_values)
		chip->charging_disabled = true;

	of_get_property(chip->spmi->dev.of_node, "qcom,thermal-mitigation",
		&(chip->thermal_levels));

	if (chip->thermal_levels > sizeof(int)) {
		chip->thermal_mitigation = kzalloc(
			chip->thermal_levels,
			GFP_KERNEL);

		if (chip->thermal_mitigation == NULL) {
			pr_err("thermal mitigation kzalloc() failed.\n");
			return rc;
		}

		chip->thermal_levels /= sizeof(int);
		rc = of_property_read_u32_array(chip->spmi->dev.of_node,
				"qcom,thermal-mitigation",
				chip->thermal_mitigation, chip->thermal_levels);
		if (rc) {
			pr_err("qcom,thermal-mitigation missing in dt\n");
			return rc;
		}
	}

	return rc;
}

#if defined(FEATURE_POWER_ON_OFF_TEST) // 20130527. MKS.
void set_is_offline_charging_mode(void)
{
	static oem_pm_smem_vendor1_data_type *smem_id_vendor1_ptr;
	
	smem_id_vendor1_ptr = (oem_pm_smem_vendor1_data_type*)smem_alloc(SMEM_ID_VENDOR1,
		sizeof(oem_pm_smem_vendor1_data_type));
	
	if(smem_id_vendor1_ptr->power_on_mode==0)
		is_offline_charging_mode = 1;
}

static void pwrkey_sw_pressed(struct work_struct *work)
{
	pr_err("[MKS] pwrkey_sw_pressed Entered !!!");
	
	input_report_key(pwr_test->pwr, KEY_POWER, 1);
	input_sync(pwr_test->pwr);
	msleep(3000);
	input_report_key(pwr_test->pwr, KEY_POWER, 0);
	input_sync(pwr_test->pwr);

	return;
}

static ssize_t pwronoff_trigger_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", pwr_on_trigger);
}

static ssize_t pwronoff_trigger_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	sscanf(buf, "%d\n", &pwr_on_trigger);
	pr_err("[MKS] pwronoff_trigger_store Entered !!!pwr_on_trigger(%d)", pwr_on_trigger);
	if (!pwr_test)
		return 0;

	schedule_delayed_work(&pwr_test->power_key_emulation_work, round_jiffies_relative(msecs_to_jiffies((pwrkey_delay_ms==0) ? 65000 : pwrkey_delay_ms)));

	return count;
}

static ssize_t pwronoff_delay_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", pwrkey_delay_ms);
}

static ssize_t pwronoff_delay_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	sscanf(buf, "%d\n", &pwrkey_delay_ms);
	pr_err("[MKS] pwronoff_delay_store Entered !!!pwrkey_delay_ms(%d)", pwrkey_delay_ms);
	
	return count;
}

static struct kobj_attribute pwr_onoff_attr =
        __ATTR(pwr_onoff_trigger, 0666, pwronoff_trigger_show, pwronoff_trigger_store);
static struct kobj_attribute pwr_onoff_delay_attr =
        __ATTR(pwr_onoff_delay, 0666, pwronoff_delay_show, pwronoff_delay_store);

static struct attribute *g[] = {
        &pwr_onoff_attr.attr,
        &pwr_onoff_delay_attr.attr,
        NULL,
};
static struct attribute_group pwr_onoff_attr_group = {
        .attrs = g,
};
#endif

static int __devinit
qpnp_charger_probe(struct spmi_device *spmi)
{
	u8 subtype;
	struct qpnp_chg_chip	*chip;
	struct resource *resource;
	struct spmi_resource *spmi_resource;
	int rc = 0;
#if defined(FEATURE_POWER_ON_OFF_TEST) // 20130524. MKS.
	int err;
#endif

	chip = kzalloc(sizeof *chip, GFP_KERNEL);
	if (chip == NULL) {
		pr_err("kzalloc() failed.\n");
		return -ENOMEM;
	}

	chip->dev = &(spmi->dev);
	chip->spmi = spmi;

	chip->usb_psy = power_supply_get_by_name("usb");
	if (!chip->usb_psy) {
		pr_err("usb supply not found deferring probe\n");
		rc = -EPROBE_DEFER;
		goto fail_chg_enable;
	}

	/* Get all device tree properties */
	rc = qpnp_charger_read_dt_props(chip);
	if (rc)
		goto fail_chg_enable;

	/* Check if bat_if is set in DT and make sure VADC is present */
	spmi_for_each_container_dev(spmi_resource, spmi) {
		if (!spmi_resource) {
			pr_err("qpnp_chg: spmi resource absent\n");
			rc = -ENXIO;
			goto fail_chg_enable;
		}

		resource = spmi_get_resource(spmi, spmi_resource,
						IORESOURCE_MEM, 0);
		if (!(resource && resource->start)) {
			pr_err("node %s IO resource absent!\n",
				spmi->dev.of_node->full_name);
			rc = -ENXIO;
			goto fail_chg_enable;
		}

		rc = qpnp_chg_read(chip, &subtype,
				resource->start + REG_OFFSET_PERP_SUBTYPE, 1);
		if (rc) {
			pr_err("Peripheral subtype read failed rc=%d\n", rc);
			goto fail_chg_enable;
		}

		if (subtype == SMBB_BAT_IF_SUBTYPE ||
			subtype == SMBBP_BAT_IF_SUBTYPE ||
			subtype == SMBCL_BAT_IF_SUBTYPE){
			rc = qpnp_vadc_is_ready();
			if (rc)
				goto fail_chg_enable;
		}
	}

	spmi_for_each_container_dev(spmi_resource, spmi) {
		if (!spmi_resource) {
			pr_err("qpnp_chg: spmi resource absent\n");
			rc = -ENXIO;
			goto fail_chg_enable;
		}

		resource = spmi_get_resource(spmi, spmi_resource,
						IORESOURCE_MEM, 0);
		if (!(resource && resource->start)) {
			pr_err("node %s IO resource absent!\n",
				spmi->dev.of_node->full_name);
			rc = -ENXIO;
			goto fail_chg_enable;
		}

		rc = qpnp_chg_read(chip, &subtype,
				resource->start + REG_OFFSET_PERP_SUBTYPE, 1);
		if (rc) {
			pr_err("Peripheral subtype read failed rc=%d\n", rc);
			goto fail_chg_enable;
		}

		switch (subtype) {
		case SMBB_CHGR_SUBTYPE:
		case SMBBP_CHGR_SUBTYPE:
		case SMBCL_CHGR_SUBTYPE:
			chip->chgr_base = resource->start;
			rc = qpnp_chg_hwinit(chip, subtype, spmi_resource);
			if (rc) {
				pr_err("Failed to init subtype 0x%x rc=%d\n",
						subtype, rc);
				goto fail_chg_enable;
			}
			break;
		case SMBB_BUCK_SUBTYPE:
		case SMBBP_BUCK_SUBTYPE:
		case SMBCL_BUCK_SUBTYPE:
			chip->buck_base = resource->start;
			rc = qpnp_chg_hwinit(chip, subtype, spmi_resource);
			if (rc) {
				pr_err("Failed to init subtype 0x%x rc=%d\n",
						subtype, rc);
				goto fail_chg_enable;
			}

			rc = qpnp_chg_masked_write(chip,
				chip->buck_base + SEC_ACCESS,
				0xFF,
				0xA5, 1);

			rc = qpnp_chg_masked_write(chip,
				chip->buck_base + BUCK_VCHG_OV,
				0xff,
				0x00, 1);

			rc = qpnp_chg_masked_write(chip,
				chip->buck_base + SEC_ACCESS,
				0xFF,
				0xA5, 1);

			rc = qpnp_chg_masked_write(chip,
				chip->buck_base + BUCK_TEST_SMBC_MODES,
				0xFF,
				0x80, 1);

			break;
		case SMBB_BAT_IF_SUBTYPE:
		case SMBBP_BAT_IF_SUBTYPE:
		case SMBCL_BAT_IF_SUBTYPE:
			chip->bat_if_base = resource->start;
			rc = qpnp_chg_hwinit(chip, subtype, spmi_resource);
			if (rc) {
				pr_err("Failed to init subtype 0x%x rc=%d\n",
						subtype, rc);
				goto fail_chg_enable;
			}
			break;
		case SMBB_USB_CHGPTH_SUBTYPE:
		case SMBBP_USB_CHGPTH_SUBTYPE:
		case SMBCL_USB_CHGPTH_SUBTYPE:
			chip->usb_chgpth_base = resource->start;
			rc = qpnp_chg_hwinit(chip, subtype, spmi_resource);
			if (rc) {
				if (rc != -EPROBE_DEFER)
				pr_err("Failed to init subtype 0x%x rc=%d\n",
						subtype, rc);
				goto fail_chg_enable;
			}
			break;
		case SMBB_DC_CHGPTH_SUBTYPE:
			chip->dc_chgpth_base = resource->start;
			rc = qpnp_chg_hwinit(chip, subtype, spmi_resource);
			if (rc) {
				pr_err("Failed to init subtype 0x%x rc=%d\n",
						subtype, rc);
				goto fail_chg_enable;
			}
			break;
		case SMBB_BOOST_SUBTYPE:
		case SMBBP_BOOST_SUBTYPE:
			chip->boost_base = resource->start;
			rc = qpnp_chg_hwinit(chip, subtype, spmi_resource);
			if (rc) {
				if (rc != -EPROBE_DEFER)
				pr_err("Failed to init subtype 0x%x rc=%d\n",
						subtype, rc);
				goto fail_chg_enable;
			}
			break;
		case SMBB_MISC_SUBTYPE:
		case SMBBP_MISC_SUBTYPE:
		case SMBCL_MISC_SUBTYPE:
			chip->misc_base = resource->start;
			rc = qpnp_chg_hwinit(chip, subtype, spmi_resource);
			if (rc) {
				pr_err("Failed to init subtype=0x%x rc=%d\n",
						subtype, rc);
				goto fail_chg_enable;
			}
			break;
		default:
			pr_err("Invalid peripheral subtype=0x%x\n", subtype);
			rc = -EINVAL;
			goto fail_chg_enable;
		}
	}
#if defined(CONFIG_QPNP_CHARGER)	
	the_qpnp_chip = chip;
#endif
	dev_set_drvdata(&spmi->dev, chip);
	device_init_wakeup(&spmi->dev, 1);

	if (chip->bat_if_base) {
		chip->batt_psy.name = "battery";
		chip->batt_psy.type = POWER_SUPPLY_TYPE_BATTERY;
		chip->batt_psy.properties = msm_batt_power_props;
		chip->batt_psy.num_properties =
			ARRAY_SIZE(msm_batt_power_props);
		chip->batt_psy.get_property = qpnp_batt_power_get_property;
		chip->batt_psy.set_property = qpnp_batt_power_set_property;
		chip->batt_psy.property_is_writeable =
				qpnp_batt_property_is_writeable;
		chip->batt_psy.external_power_changed =
				qpnp_batt_external_power_changed;
		chip->batt_psy.supplied_to = pm_batt_supplied_to;
		chip->batt_psy.num_supplicants =
				ARRAY_SIZE(pm_batt_supplied_to);

		rc = power_supply_register(chip->dev, &chip->batt_psy);
		if (rc < 0) {
			pr_err("batt failed to register rc = %d\n", rc);
			goto fail_chg_enable;
		}
		INIT_WORK(&chip->adc_measure_work,
			qpnp_bat_if_adc_measure_work);
#if defined(FEATURE_PANTECH_PMIC_PHYSICAL_DROP)
		INIT_DELAYED_WORK(&chip->drop_work,
			qpnp_bat_if_drop_work);
#endif
		INIT_DELAYED_WORK(&chip->arb_stop_work, qpnp_arb_stop_work);
	}

	wake_lock_init(&chip->eoc_wake_lock,
		WAKE_LOCK_SUSPEND, "qpnp-chg-eoc-lock");
	INIT_DELAYED_WORK(&chip->eoc_work, qpnp_eoc_work);
	//INIT_DELAYED_WORK(&chip->arb_stop_work, qpnp_arb_stop_work);
	INIT_WORK(&chip->soc_check_work, qpnp_chg_soc_check_work);

	if (chip->dc_chgpth_base) {
		chip->dc_psy.name = "qpnp-dc";
		chip->dc_psy.type = POWER_SUPPLY_TYPE_MAINS;
		chip->dc_psy.supplied_to = pm_power_supplied_to;
		chip->dc_psy.num_supplicants = ARRAY_SIZE(pm_power_supplied_to);
		chip->dc_psy.properties = pm_power_props_mains;
		chip->dc_psy.num_properties = ARRAY_SIZE(pm_power_props_mains);
		chip->dc_psy.get_property = qpnp_power_get_property_mains;
		chip->dc_psy.set_property = qpnp_dc_power_set_property;
		chip->dc_psy.property_is_writeable =
				qpnp_dc_property_is_writeable;

		rc = power_supply_register(chip->dev, &chip->dc_psy);
		if (rc < 0) {
			pr_err("power_supply_register dc failed rc=%d\n", rc);
			goto unregister_batt;
		}
	}

	/* Turn on appropriate workaround flags */
	qpnp_chg_setup_flags(chip);

	if (chip->maxinput_dc_ma && chip->dc_chgpth_base) {
		rc = qpnp_chg_idcmax_set(chip, chip->maxinput_dc_ma);
		if (rc) {
			pr_err("Error setting idcmax property %d\n", rc);
			goto unregister_batt;
		}
	}

	if (chip->cool_bat_decidegc && chip->warm_bat_decidegc
							&& chip->bat_if_base) {
		chip->adc_param.low_temp = chip->cool_bat_decidegc;
		chip->adc_param.high_temp = chip->warm_bat_decidegc;
		chip->adc_param.timer_interval = ADC_MEAS2_INTERVAL_1S;
		chip->adc_param.state_request = ADC_TM_HIGH_LOW_THR_ENABLE;
		chip->adc_param.btm_ctx = chip;
		chip->adc_param.threshold_notification =
						qpnp_chg_adc_notification;
		chip->adc_param.channel = LR_MUX1_BATT_THERM;

		if (get_prop_batt_present(chip)) {
			rc = qpnp_adc_tm_channel_measure(&chip->adc_param);
		if (rc) {
			pr_err("request ADC error %d\n", rc);
			goto fail_chg_enable;
		}
	}
	}
#if 0 //defined(CONFIG_QPNP_CHARGER)	
       the_qpnp_chip = chip;
#endif
#if defined(CONFIG_PANTECH_PMIC_CHARGER_SMB349) || defined(CONFIG_PANTECH_PMIC_CHARGER_SMB347)
	chip->battery_id_adc = get_battery_id_adc();
#endif 	
	qpnp_chg_charge_en(chip, !chip->charging_disabled);
	qpnp_chg_force_run_on_batt(chip, chip->charging_disabled);
	qpnp_chg_set_appropriate_vddmax(chip);

	rc = qpnp_chg_request_irqs(chip);
	if (rc) {
		pr_err("failed to request interrupts %d\n", rc);
		goto unregister_batt;
	}
#if defined(CONFIG_PANTECH_PMIC_CHARGER_SMB349) || defined(CONFIG_PANTECH_PMIC_CHARGER_SMB347)
#if defined(FEATURE_PANTECH_PMIC_EOC)
	chip->end_recharing = false;
#endif
#if defined(FEATURE_PANTECH_PMIC_ABNORMAL)
	chip->check_cable_disconnect_work = false;
#endif
	wake_lock_init(&chip->smb349_cable_wake_lock, WAKE_LOCK_SUSPEND, "smb349_cable_wake_lock");
#endif
#if defined(FEATURE_PANTECH_PMIC_USBIN_DROP_WORKAROUND)
	qpnp_sysok_irq_handler(SC_SYSOK, chip);
#else
	qpnp_chg_usb_usbin_valid_irq_handler(USBIN_VALID_IRQ, chip);
#endif
	qpnp_chg_dc_dcin_valid_irq_handler(DCIN_VALID_IRQ, chip);
	power_supply_set_present(chip->usb_psy,
			qpnp_chg_is_usb_chg_plugged_in(chip));

	/* Set USB psy online to avoid userspace from shutting down if battery
	 * capacity is at zero and no chargers online. */
	if (qpnp_chg_is_usb_chg_plugged_in(chip))
		power_supply_set_online(chip->usb_psy, 1);

	pr_info("success chg_dis = %d, bpd = %d, usb = %d, dc = %d b_health = %d batt_present = %d\n",
			chip->charging_disabled,
			chip->bpd_detection,
			qpnp_chg_is_usb_chg_plugged_in(chip),
			qpnp_chg_is_dc_chg_plugged_in(chip),
			get_prop_batt_present(chip),
			get_prop_batt_health(chip));

#if defined(FEATURE_PANTECH_PMIC_BMS_TEST)
	atomic_set(&bms_input_flag, 0);
	atomic_set(&bms_cutoff_flag, 1);

	bms_input_attr_dev = platform_device_register_simple("bms_app_attr", -1, NULL, 0);

	if (!bms_input_attr_dev) {
		pr_debug("BMS: Unable to register platform_device_register_simple device\n");
		rc = -ENXIO;
		//goto free_irq;
	}

	rc = sysfs_create_group(&bms_input_attr_dev->dev.kobj, &bms_input_attr_group);	
	if (rc) {
		pr_debug("[BMS] failed: sysfs_create_group  [ERROR]\n");	
		goto unregister_input_attr;
	} 

	bms_input_dev = input_allocate_device();

	if (!bms_input_dev) {
		pr_debug("BMS: Unable to input_allocate_device \n");
		rc = -ENXIO;
		goto remove_sysfs;
	}
	

	set_bit(EV_REL, bms_input_dev->evbit);

	input_set_capability(bms_input_dev, EV_REL, REL_RX);	// SOC
	input_set_capability(bms_input_dev, EV_REL, REL_RY); 	// Volt
	input_set_capability(bms_input_dev, EV_REL, REL_RZ);    // TEMP
	input_set_capability(bms_input_dev, EV_REL, REL_X);		// VCHG 20130521 djjeon, powerlog add ICHG
	input_set_capability(bms_input_dev, EV_REL, REL_Y);	//real soc, 20131030 p13787 ryu 

	bms_input_dev->name="bms_app";

	rc = input_register_device(bms_input_dev);

	if (rc) {
		pr_debug("BMS: Unable to register input_register_device device\n");
		goto free_input_device;
	}

	// set init values
	bms_init_set(chip);

#endif
     
#if defined(CONFIG_PANTECH_PMIC_CHARGER_SMB349) || defined(CONFIG_PANTECH_PMIC_CHARGER_SMB347)
#if 1 // (defined(T_EF56S) && (BOARD_VER <TP10))
       set_prop_batt_present_ctrl(chip);
#endif
       set_prop_batt_btc_ctrl(chip);
#endif

#if defined(FEATURE_PANTECH_PMIC_MONITOR_TEST)
	pm8941_charger_test_charger_monitor_init(chip);
#endif
	
#if defined(FEATURE_PANTECH_PMIC_MONITOR_TEST) || defined(FEATURE_PANTECH_PMIC_BATTERY_CHARGING_DISCHARGING_TEST)
	pm8941_charger_battery_charging_test_init();
#endif
#if defined(CONFIG_PANTECH_PMIC_CHARGER_SMB349) || defined(CONFIG_PANTECH_PMIC_CHARGER_SMB347)
	INIT_DELAYED_WORK(&chip->update_heartbeat_work, check_temperature_worker);	
    schedule_delayed_work(&chip->update_heartbeat_work, round_jiffies_relative(msecs_to_jiffies(1000)));	   	
#endif

#if defined(FEATURE_PANTECH_PMIC_ABNORMAL)
	INIT_DELAYED_WORK(&chip->update_abnormal_work, check_abnormal_worker);	
	INIT_DELAYED_WORK(&chip->update_abnormal_cable_disconeect_work, check_abnormal_cable_disconnet_worker);	
#endif
#if defined(FEATURE_POWER_ON_OFF_TEST) // 20130524. MKS.
	set_is_offline_charging_mode();

	pwr_test = kzalloc(sizeof(*pwr_test), GFP_KERNEL);
	if (!pwr_test)
		return -ENOMEM;

	pwr_test->pwr = input_allocate_device();
	if (!pwr_test->pwr){
		pr_err("Can't allocate power button\n");
	}else{
		pwr_test->pwr->name = "pmic_pwrkey_emulation";
		pwr_test->pwr->evbit[0]=BIT(EV_KEY);
		set_bit(KEY_POWER, pwr_test->pwr->keybit);

		err = input_register_device(pwr_test->pwr);
		if (err) {
			pr_err("Can't register power key emulation: %d\n", err);
			// 20130527. MKS. to do. error handling.
		}
		INIT_DELAYED_WORK(&pwr_test->power_key_emulation_work, pwrkey_sw_pressed);

		kobj_pwr_test = kobject_create_and_add("pmic_pwrkey_emulation", NULL);
		if (kobj_pwr_test)
			err = sysfs_create_group(kobj_pwr_test, &pwr_onoff_attr_group);
			// 20130527. MKS. to do. error handling.
	}	
#endif

       charger_probe =1; 
	return 0;
#if defined(FEATURE_PANTECH_PMIC_BMS_TEST)
free_input_device:
        input_free_device(bms_input_dev);
remove_sysfs:
        sysfs_remove_group(&bms_input_attr_dev->dev.kobj, &bms_input_attr_group);
unregister_input_attr:
        platform_device_unregister(bms_input_attr_dev);

#endif

unregister_batt:
	if (chip->bat_if_base)
		power_supply_unregister(&chip->batt_psy);
fail_chg_enable:
	regulator_unregister(chip->otg_vreg.rdev);
	regulator_unregister(chip->boost_vreg.rdev);
	kfree(chip->thermal_mitigation);
	kfree(chip);
	dev_set_drvdata(&spmi->dev, NULL);
	return rc;
}

static int __devexit
qpnp_charger_remove(struct spmi_device *spmi)
{
	struct qpnp_chg_chip *chip = dev_get_drvdata(&spmi->dev);
	if (chip->cool_bat_decidegc && chip->warm_bat_decidegc
						&& chip->batt_present) {
		qpnp_adc_tm_disable_chan_meas(&chip->adc_param);
	}
	cancel_work_sync(&chip->adc_measure_work);
#if defined(FEATURE_PANTECH_PMIC_PHYSICAL_DROP)
	cancel_delayed_work(&chip->drop_work); // skkim p14200@LS1
#endif
	cancel_delayed_work_sync(&chip->eoc_work);
	
#if defined(FEATURE_PANTECH_PMIC_USBIN_DROP_WORKAROUND)
	free_irq(gpio_to_irq(SC_SYSOK), chip);
#endif
	regulator_unregister(chip->otg_vreg.rdev);
	regulator_unregister(chip->boost_vreg.rdev);

	dev_set_drvdata(&spmi->dev, NULL);
	kfree(chip);

	return 0;
}

static int qpnp_chg_resume(struct device *dev)
{
	struct qpnp_chg_chip *chip = dev_get_drvdata(dev);
	int rc = 0;
#if defined(CONFIG_PANTECH_PMIC_CHARGER_SMB349) || defined(CONFIG_PANTECH_PMIC_CHARGER_SMB347)
	schedule_delayed_work(&chip->update_heartbeat_work,
        	round_jiffies_relative(msecs_to_jiffies(0)));
#endif

	if (chip->bat_if_base) {
	rc = qpnp_chg_masked_write(chip,
	chip->bat_if_base + BAT_IF_VREF_BAT_THM_CTRL,
		VREF_BATT_THERM_FORCE_ON,
		VREF_BATT_THERM_FORCE_ON, 1);
	if (rc)
		pr_debug("failed to force on VREF_BAT_THM rc=%d\n", rc);
	}
	qpnp_chg_enable_irq(&chip->batt_pres);//ADD THIS CODE FOR WORKAROUND
#if defined(FEATURE_PANTECH_PMIC_USBIN_DROP_WORKAROUND)
	enable_irq(chip->sysok_valid.irq);
	qpnp_chg_disable_irq(&chip->usbin_valid);
	qpnp_sysok_irq_handler(SC_SYSOK, chip);
#endif
	return rc;
}

static int qpnp_chg_suspend(struct device *dev)
{
	struct qpnp_chg_chip *chip = dev_get_drvdata(dev);
	int rc = 0;
#if defined(CONFIG_PANTECH_PMIC_CHARGER_SMB349) || defined(CONFIG_PANTECH_PMIC_CHARGER_SMB347)
	cancel_delayed_work(&chip->update_heartbeat_work);
#endif
       qpnp_chg_disable_irq(&chip->batt_pres);//ADD THIS CODE FOR WORKAROUND
#if defined(FEATURE_PANTECH_PMIC_USBIN_DROP_WORKAROUND)
	qpnp_chg_enable_irq(&chip->usbin_valid);
	disable_irq_nosync(chip->sysok_valid.irq);
#endif
	if (chip->bat_if_base) {
	rc = qpnp_chg_masked_write(chip,
	chip->bat_if_base + BAT_IF_VREF_BAT_THM_CTRL,
		VREF_BATT_THERM_FORCE_ON,
		VREF_BAT_THM_ENABLED_FSM, 1);
	if (rc)
			pr_debug("failed to set FSM VREF_BAT_THM rc=%d\n", rc);
	}

	return rc;
}

static const struct dev_pm_ops qpnp_chg_pm_ops = {
	.resume		= qpnp_chg_resume,
	.suspend	= qpnp_chg_suspend,
};

static struct spmi_driver qpnp_charger_driver = {
	.probe		= qpnp_charger_probe,
	.remove		= __devexit_p(qpnp_charger_remove),
	.driver		= {
		.name		= QPNP_CHARGER_DEV_NAME,
		.owner		= THIS_MODULE,
		.of_match_table	= qpnp_charger_match_table,
		.pm		= &qpnp_chg_pm_ops,
	},
};

/**
 * qpnp_chg_init() - register spmi driver for qpnp-chg
 */
int __init
qpnp_chg_init(void)
{
	return spmi_driver_register(&qpnp_charger_driver);
}
module_init(qpnp_chg_init);

static void __exit
qpnp_chg_exit(void)
{
	spmi_driver_unregister(&qpnp_charger_driver);
}
module_exit(qpnp_chg_exit);


MODULE_DESCRIPTION("QPNP charger driver");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:" QPNP_CHARGER_DEV_NAME);
