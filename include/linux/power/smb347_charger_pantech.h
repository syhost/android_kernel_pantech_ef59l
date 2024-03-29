#ifndef SMB347_CHARGER_H
#define SMB347_CHARGER_H

enum pantech_cable_type {
	PANTECH_CABLE_NONE=0,
	PANTECH_OTG,
	PANTECH_USB,
	PANTECH_AC,
	PANTECH_UNKNOWN,		
	PANTECH_CABLE_MAX,
};

enum
{
	APSD_NOT_RUN	= 0,
	APSD_CDP,
	APSD_DCP,
	APSD_OCP,
	APSD_SDP,
	APSD_ACA,
	APSD_NOT_USED,
};


enum battery_thermal_trip_type {
	BATT_THERM_COLD = 0,
	BATT_THERM_COOL,
	BATT_THERM_NORMAL,
	BATT_THERM_WARM,
	BATT_THERM_HOT,
	BATT_THERM_UNKNOWN,
};
#if defined(FEATURE_PANTECH_PMIC_CHARGE_LIMIT_WITH_TSENSOR)
enum Tsense_status {
	T_SENSE_UP = 1,
	T_SENSE_DOWN,
	T_SENSE_NORMAL,
};
#endif


int smb349_read_reg(u8 reg, unsigned char *val);
int smb349_write_reg(u8 reg, unsigned char val);
void smb349_print_all_reg(void);
void smb349_print_reg(u8 reg);
int smb349_charging_set(bool enable);

#if defined(FEATURE_PANTECH_PMIC_CHARGE_LIMIT_WITH_FINGERPRINT)
void smb349_adjust_limit_use_sensor(int use_sensor);
#endif

#if defined(FEATURE_PANTECH_PMIC_OTG_UVLO)
int smb347_get_otg_status(void);	// 20130903 skkim (p14200@LS1)
#endif

#if defined(FEATURE_PANTECH_PMIC_AUTO_PWR_ON)	//djjeon 20130610 add
int get_is_offline_charging_mode(void);
int get_auto_power_on_flag(void);
#endif //FEATURE_PANTECH_PMIC_AUTO_PWR_ON

#endif
