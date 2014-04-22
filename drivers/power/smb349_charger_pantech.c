
#include <linux/module.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <mach/gpio.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/regulator/consumer.h>
#include <linux/string.h>
#include <linux/of_gpio.h>
#include <linux/kernel.h>
#include <linux/power/smb349_charger_pantech.h>
#include <linux/workqueue.h>  
#include <linux/qpnp/qpnp-adc.h>
#include <linux/delay.h>



#include <linux/types.h>
#include <linux/err.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/power_supply.h>
#include <linux/spmi.h>
#include <linux/rtc.h>
#include <linux/qpnp/qpnp-adc.h>
#include <linux/mfd/pm8xxx/batterydata-lib.h>

#if defined FEATURE_PANTECH_PMIC_AUTO_PWR_ON
#include <linux/reboot.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <asm/uaccess.h>
#include <linux/workqueue.h>

#include <mach/msm_smsm.h>

#endif
extern int cable_present_state(void);

#if defined(FEATURE_PANTECH_PMIC_CHARGE_LIMIT_WITH_LCD)
int offline_boot_ok=0, boot_lcd_cnt=0;
#endif

/* Register definitions */
#define CHG_CURRENT_REG				0x00	/* Fast charge current + AC input current limit */
#define CHG_OTHER_CURRENT_REG		0x01	/* Pre-cahrge current + Term current */
#define VAR_FUNC_REG				0x02	
#define FLOAT_VOLTAGE_REG			0x03	/* Pre-charge voltage + Float voltage */
#define CHG_CTRL_REG				0x04	
#define STAT_TIMER_REG				0x05	/* charge time out */
#define PIN_ENABLE_CTRL_REG			0x06	
#define THERM_CTRL_A_REG			0x07		
#define CTRL_FUNCTIONS_REG			0x09	
#define OTG_TLIM_THERM_CNTRL_REG	0x0A	
#define TEMP_MONITOR_REG			0x0B	
#define FAULT_IRQ_REG				0x0C	
#define IRQ_ENABLE_REG				0x0D	
#define SYSOK_REG					0x0E	

/* Command registers */
#define CMD_A						0x30
#define CMD_A_CHG_ENABLED			BIT(1)
#define CMD_A_SUSPEND_ENABLED		BIT(2)
#define CMD_A_ALLOW_WRITE			BIT(7)
#if defined(FEATURE_PANTECH_PMIC_EOC)
#define CHG_CTRL_TERM_ENABLE		BIT(6)
#endif

#define BATT_THERM_MAX 5
#define CHANGED 1
#define NOT_CHANGED 0
#define TEMP_HYSTERESIS_DEGC 30
int is_factory_dummy = 0 ;  // sayuss Factory
#if defined(FEATURE_PANTECH_PMIC_CHARGE_LIMIT_WITH_LCD)
int LCD_OFF = 0;
#endif

int is_CDP = 0; // sayuss 2013.09.05 CDP issue
#if defined(FEATURE_PANTECH_PMIC_CHARGING_DISABLE)
bool smb_chg_disabled ;  // sayuss charge_CMD 
#endif

static const int therm_boundary_min[BATT_THERM_MAX]={-2000, -80, 20, 400, 550};
static const int therm_boundary_max[BATT_THERM_MAX]={-50, 50, 430, 580, 2000};
static const int therm_boundary_init[BATT_THERM_MAX]={-2000, -80, 20, 430, 580};
static int prve_cable_type = PANTECH_CABLE_NONE;
static int qcom_cable_type = PANTECH_CABLE_NONE;
int prev_state_mode = BATT_THERM_UNKNOWN;
static int prev_cable_current = 0;


struct smb349_data {
	struct i2c_client   *client;
	
};
static struct i2c_client *smb349_client;
static struct smb349_data *the_smb349;
static struct i2c_driver smb349_i2c_driver;


int smb349_read(struct smb349_data *smb, u8 reg)
{
	int ret;

	ret = i2c_smbus_read_byte_data(smb->client, reg);
	if (ret < 0)
		dev_warn(&smb->client->dev, "failed to read reg 0x%x: %d\n",
			 reg, ret);
	return ret;
}

int smb349_write(struct smb349_data *smb, u8 reg, u8 val)
{
	int ret;

	ret = i2c_smbus_write_byte_data(smb->client, reg, val);
	if (ret < 0)
		dev_warn(&smb->client->dev, "failed to write reg 0x%x: %d\n",
			 reg, ret);
	return ret;
}

int smb349_read_reg(u8 reg, unsigned char *val)
{
	s32 ret;
	if(!the_smb349->client)
		return -EIO;
	ret = i2c_smbus_read_byte_data(the_smb349->client, reg);
	if (ret < 0) {
		pr_err("smb349 i2c read fail: can't read from %02x: %d\n", reg, ret);
		return ret;
	} else{
		*val = ret;
	}	
	return 0;
}


int smb349_write_reg(u8 reg, unsigned char val)
{
	s32 ret;

	if(!the_smb349->client)
		return -EIO;
	
	ret = i2c_smbus_write_byte_data(the_smb349->client, reg, val); 
	if (ret < 0) {
		pr_err("smb349 i2c write fail: can't write %02x to %02x: %d\n",
			val, reg, ret);
		return ret;
	}
	
	return 0;
}


void smb349_otg_power(int on)
{
	printk("%s Enter on=%d\n", __func__, on);
	
	if (on){		 
		smb349_write_reg(0x30,0x91); // OTG: enable, STAT: disable
		smb349_write_reg(0x31,0x00); // USB1 or USB1.5
		prve_cable_type = 0;
	}
	else {		
		smb349_write_reg(0x30,0xC0); // OTG: disable, STAT: enable, disable charging
		smb349_write_reg(0x31,0x00); // USB1 or USB1.5	
	}
		
}
EXPORT_SYMBOL_GPL(smb349_otg_power);

/*56S TP10 board apply*/
#ifdef FEATURE_PANTECH_PMIC_OTG 
void smb349_otg_power_current(int mode)
{
	printk("set otg index %d\n", mode);
	switch(mode){
		case 1:
				smb349_write_reg(0x0A,0x33);  //250mA
				break;	
		case 2:
				smb349_write_reg(0x0A,0x37); //500mA
				break;	
		case 3:
				smb349_write_reg(0x0A,0x3B); //750mA
				break;	
		case 4:
				smb349_write_reg(0x0A,0x3F); //1000mA
				break;	
		default:
				break;
		}
}

EXPORT_SYMBOL_GPL(smb349_otg_power_current);
#endif 

void smb349_print_reg(u8 reg)
{
	u8 is_invalid_temp;	
	smb349_read_reg(reg, &is_invalid_temp);
	pr_err("0x%x reg value = %.2X\n", reg, is_invalid_temp);
	return ;
}

void smb349_print_all_reg(void)
{
	u8 is_invalid_temp;	
	pr_err("=========smb349_reg start========\n");
	smb349_read_reg(0x30, &is_invalid_temp);
	pr_err("0x30 value = %.2X\n", is_invalid_temp);
	smb349_read_reg(0x31, &is_invalid_temp);
	pr_err("0x31 value = %.2X\n", is_invalid_temp);
	smb349_read_reg(0x00, &is_invalid_temp);
	pr_err("0x00 value = %.2X\n", is_invalid_temp);
	smb349_read_reg(0x01, &is_invalid_temp);
	pr_err("0x01 value = %.2X\n", is_invalid_temp);
	smb349_read_reg(0x02, &is_invalid_temp);
	pr_err("0x02 value = %.2X\n", is_invalid_temp);
	smb349_read_reg(0x03, &is_invalid_temp);
	pr_err("0x03 value = %.2X\n", is_invalid_temp);
	smb349_read_reg(0x04, &is_invalid_temp);
	pr_err("0x04 value = %.2X\n", is_invalid_temp);
	smb349_read_reg(0x05, &is_invalid_temp);
	pr_err("0x05 value = %.2X\n", is_invalid_temp);
	smb349_read_reg(0x06, &is_invalid_temp);
	pr_err("0x06 value = %.2X\n", is_invalid_temp);
	smb349_read_reg(0x07, &is_invalid_temp);
	pr_err("0x07 value = %.2X\n", is_invalid_temp);
	smb349_read_reg(0x08, &is_invalid_temp);
	pr_err("0x08 value = %.2X\n", is_invalid_temp);
	smb349_read_reg(0x09, &is_invalid_temp);
	pr_err("0x09 value = %.2X\n", is_invalid_temp);
	smb349_read_reg(0x0A, &is_invalid_temp);
	pr_err("0x0A value = %.2X\n", is_invalid_temp);
	smb349_read_reg(0x0B, &is_invalid_temp);
	pr_err("0x0B value = %.2X\n", is_invalid_temp);
	smb349_read_reg(0x0C, &is_invalid_temp);
	pr_err("0x0C value = %.2X\n", is_invalid_temp);
	smb349_read_reg(0x0D, &is_invalid_temp);
	pr_err("0x0D value = %.2X\n", is_invalid_temp);
	smb349_read_reg(0x0E, &is_invalid_temp);
	pr_err("0x0E value = %.2X\n", is_invalid_temp);
	smb349_read_reg(0x34, &is_invalid_temp);
	pr_err("0x34 value = %.2X\n", is_invalid_temp);
	smb349_read_reg(0x3F, &is_invalid_temp);
	pr_err("0x3F value = %.2X\n", is_invalid_temp);
	pr_err("=========smb349_reg end========\n");
	
	return;
}
	
#if defined(FEATURE_PANTECH_PMIC_EOC)
void end_recharging_enable(bool enable)
{
	int term_satatus = 0;
	u8 is_invalid_temp;	
	term_satatus =  smb349_read(the_smb349, CHG_CTRL_REG);
	if (term_satatus < 0)
			return;
	if(enable){
		term_satatus &= ~CHG_CTRL_TERM_ENABLE;
	}else{
		term_satatus |= CHG_CTRL_TERM_ENABLE;		
	}	
	term_satatus = smb349_write(the_smb349, CHG_CTRL_REG, term_satatus);
	smb349_read_reg(0x04, &is_invalid_temp);
	pr_debug("0x04 value = %.2X\n", is_invalid_temp);
}
EXPORT_SYMBOL_GPL(end_recharging_enable);
#endif
void smb349_charing_enable(bool enable)
{
	int charging_status;
	u8 is_invalid_temp;	
	charging_status = smb349_read(the_smb349, CMD_A);
	if (charging_status < 0)
			return;
	if(enable){
		charging_status |= CMD_A_CHG_ENABLED;
		charging_status = smb349_write(the_smb349, CMD_A, charging_status);	
		smb349_read_reg(0x30, &is_invalid_temp);
		pr_debug("0x30 value = %.2X\n", is_invalid_temp);
	}else{
		charging_status &= ~CMD_A_CHG_ENABLED;
		charging_status = smb349_write(the_smb349, CMD_A, charging_status);
		smb349_read_reg(0x30, &is_invalid_temp);
		pr_debug("0x30 value = %.2X\n", is_invalid_temp);
	}
}
EXPORT_SYMBOL_GPL(smb349_charing_enable);	

static void smb349_usb_mode(int current_ma)
{
	// modified by skkim (p14200@LS1 / 2013.08.16)
	if (true != get_is_offline_charging_mode()) {
	if(current_ma == 100){
		smb349_write_reg(0x30, 0x83); //charging enable
		smb349_write_reg(0x31,0x00); //usb 100 
	}else if(current_ma == 900){	
		smb349_write_reg(0x30, 0x83); //charging enable
		smb349_write_reg(0x31,0x06); //usb 900mA
	}else{
		smb349_write_reg(0x30, 0x83); //charging enable
		smb349_write_reg(0x31,0x02); //usb 500
	}
	}
	else {
		// Only Offline Charging Mode (for TEST)
		smb349_write_reg(0x30, 0x83); //charging enable
		smb349_write_reg(0x31,0x02); //usb 500 
	}
	
}


#if defined(FEATURE_PANTECH_PMIC_AUTO_PWR_ON)
int get_auto_power_on_flag(void)
{
	static oem_pm_smem_vendor1_data_type *smem_id_vendor1_ptr;
	
	smem_id_vendor1_ptr = (oem_pm_smem_vendor1_data_type*)smem_alloc(SMEM_ID_VENDOR1,
		sizeof(oem_pm_smem_vendor1_data_type));
	pr_debug("======= auto_power_on_soc_check()[%d]\n",smem_id_vendor1_ptr->auto_power_on_soc_check);
	if(smem_id_vendor1_ptr->auto_power_on_soc_check==0)
		return false;
	else
		return true;
}
#endif //FEATURE_PANTECH_PMIC_AUTO_PWR_ON
int get_is_offline_charging_mode(void)
{
	static oem_pm_smem_vendor1_data_type *smem_id_vendor1_ptr;
	
	smem_id_vendor1_ptr = (oem_pm_smem_vendor1_data_type*)smem_alloc(SMEM_ID_VENDOR1,
		sizeof(oem_pm_smem_vendor1_data_type));
	pr_debug("======= get_is_offline_charging_mode()[%d]\n",smem_id_vendor1_ptr->power_on_mode);

	if(smem_id_vendor1_ptr->power_on_mode==0)
		return true;
	else
		return false;
}

int is_temp_state_changed(int* state, int64_t temp)
{
	int index = *state;
	prev_state_mode = index;	

//	pr_info("========== index = %d ==========\n",index);
	if( BATT_THERM_UNKNOWN == index )
	{
		index--;
		for( ; index > -1 ; index--)
		{	
//			pr_info("------------------------------------\n");
			if(temp > therm_boundary_init[index])
				break;
		}
	}
	else if(temp < therm_boundary_min[*state] )
	{
		index--;
		for( ; index > -1 ; index--)
		{	
//			pr_info("------------------------------------\n");
			if(temp < therm_boundary_max[index] && temp >= therm_boundary_min[index])
				break;
		}
		
	}
	else if(temp >= therm_boundary_max[*state])
	{
		index++;
		for( ; index < BATT_THERM_MAX ; index++)
		{	
//			pr_info("+++++++++++++++++++++++++++++++++++\n");
			if(temp < therm_boundary_max[index] && temp >= therm_boundary_min[index])
				break;
		}
	}
	else
		return NOT_CHANGED;
	
	if( index >= 0 && index < BATT_THERM_MAX)
	{
		*state = index;
//		pr_err("====== searched state=%d =======\n",*state);
	}
	else
	{
//		pr_err("====== sayuss something is worng =======\n");
		return NOT_CHANGED;
	}  
	return CHANGED;

}
EXPORT_SYMBOL_GPL(is_temp_state_changed);


void smb349_current_jeita (int mode, int cable_type)
{
#if defined(FEATURE_PANTECH_PMIC_CHARGING_DISABLE)
	// sayuss charge_CMD 
	if(smb_chg_disabled) 
		return;
#endif
	if(cable_present_state() == 0)
	
		return;
	
#if defined(FEATURE_PANTECH_PMIC_CHARGE_LIMIT_WITH_LCD)
	if(get_is_offline_charging_mode())
	{
		pr_err("----------- sayuss offling charging mode\n");
		if(10 != boot_lcd_cnt)
			return;
	
	}
#endif
	pr_debug("smb349_current_jeita : mode(%d), cable_type(%d)\n", mode, cable_type);

	switch(mode){
		case BATT_THERM_COLD:
			pr_debug("======== jeita mode (%d) set cold =========\n",mode);
			smb349_write_reg(0x30,0x81); //charging disable
			smb349_write_reg(0x31,0x00); //mode change 
			break;
			
		case BATT_THERM_COOL: 
			pr_debug("======== jeita mode (%d) set cool =========\n",mode);
			if(cable_type == PANTECH_USB){
				smb349_usb_mode(prev_cable_current);
				break;
			}
			smb349_write_reg(0x30, 0x81); //charging disable
			smb349_write_reg(0x31, 0x01); //hc mode
			if(is_CDP) // sayuss 2013.09.05 CDP issue
				smb349_write_reg(0x00,0x02); // fast 1A_input 1A 
			else
				smb349_write_reg(0x00,0x09); // fast 1A_input 1.8A
			smb349_write_reg(0x02, 0x8F);  //AICL disable, AICL Detection Threshold 4.5V
			smb349_write_reg(0x02, 0x9F);  //AICL enable, AICL Detection Threshold 4.5V
			smb349_write_reg(0x30, 0x83); //charging enable
			smb349_write_reg(0x31, 0x01); //hc mode
			break;	
		case BATT_THERM_NORMAL:
			pr_debug("======== jeita mode (%d) set mormal 1 =========\n",mode);
			if(cable_type == PANTECH_USB){
				smb349_usb_mode(prev_cable_current);
				break;
			}
			smb349_write_reg(0x30, 0x81); //charging disable
			smb349_write_reg(0x31, 0x01); //hc mode
#if defined(FEATURE_PANTECH_PMIC_LIMIT_WITH_EF56S)
			if(is_CDP) // sayuss 2013.09.05 CDP issue
				smb349_write_reg(0x00,0x02); // fast 1A_input 1A
			else
			smb349_write_reg(0x00,0x59); // 2.5A_2A 
#else
#if defined(FEATURE_PANTECH_PMIC_CHARGE_LIMIT_WITH_LCD)
			if(LCD_OFF)
			smb349_write_reg(0x00,0xAE); // 3A_3A 
			else
			smb349_write_reg(0x00,0x6A); // 2.5A_2A 
#else	
			smb349_write_reg(0x00,0x6A); // 2.5A_2A
#endif
#endif
			smb349_write_reg(0x02, 0x8F);  //AICL enable, AICL Detection Threshold 4.5V
			smb349_write_reg(0x02, 0x9F);  //AICL enable, AICL Detection Threshold 4.5V
			smb349_write_reg(0x30, 0x83); //charging enable
			smb349_write_reg(0x31, 0x01); //hc mode
			break;
			
		case BATT_THERM_WARM:
			pr_debug("======== jeita mode (%d) set warm =========\n",mode);
			if(cable_type == PANTECH_USB){
				smb349_usb_mode(prev_cable_current);
				break;
			}
			smb349_write_reg(0x30, 0x81); //charging disable
			smb349_write_reg(0x31, 0x01); //hc mode
			if(is_CDP) // sayuss 2013.09.05 CDP issue
				smb349_write_reg(0x00,0x02); // fast 1A_input 1A
			else
				smb349_write_reg(0x00, 0x09); // fast 1A_input 1.8A
			smb349_write_reg(0x02, 0x8F);  //AICL disable, AICL Detection Threshold 4.5V
			smb349_write_reg(0x02, 0x9F);  //AICL enable, AICL Detection Threshold 4.5V
			smb349_write_reg(0x30, 0x83); //charging enable
			smb349_write_reg(0x31, 0x01); //hc mode
			break;
			
		case BATT_THERM_HOT:
			pr_debug("======== jeita mode (%d) set hot =========\n",mode);
			smb349_write_reg(0x30,0x81); //charging disable
			smb349_write_reg(0x31,0x00); //mode change 
			break;
			
		default:
			
			break;
		}
	//smb349_print_all_reg();
}
EXPORT_SYMBOL_GPL(smb349_current_jeita);

#if defined(T_EF59L) && (BOARD_VER == WS20)
// 2013.07.25. LS1. MKS.
// setting the registers of smb349 to default. only for EF59L WS20.
void smb349_B_register_init(void){
	smb349_write_reg(0x30, 0x81);   //disable charging
   	smb349_write_reg(0x31, 0x00);   //usb mode
	smb349_write_reg(0x06, 0x08);  // Register Control
	smb349_write_reg(0x00, 0x00);  // fast : 1000mA , input : 500mA
	smb349_write_reg(0x01, 0x39);  //pre_current 300mA, term current 100mA
	smb349_write_reg(0x02, 0x9F);  //AICL enable, AICL Detection Threshold 4.5V
	smb349_write_reg(0x03, 0xED);  //Pre-Charge To Fast Charge Threshold : 3.0V , Float Voltage : 4.36V
	smb349_write_reg(0x04, 0xE0);  //auto recharge disable, recharge Threshold : 50mV ->eoc recharging
	smb349_write_reg(0x05, 0x12); //Complete Charge Timeout : 382min, Pre-Charge Timeout : 191min 	
	smb349_write_reg(0x07, 0xD5);
	smb349_write_reg(0x09, 0x04); //GF 15msec 
	smb349_write_reg(0x0A, 0x33); //Thermal Loop Temperature Select : 130C, OTG Output Current Limit : 250mA, OTG Battery UVLO Threshold : 3.3V 
	smb349_write_reg(0x0B, 0xED);
	smb349_write_reg(0x0C, 0x00);
	smb349_write_reg(0x0D, 0x00);
	smb349_write_reg(0x0E, 0x38);
	smb349_write_reg(0x10, 0x40);
}
#endif

void smb349_current_set(int cable_type, int current_ma)
{ 
	qcom_cable_type = cable_type;

#if defined(FEATURE_PANTECH_PMIC_CHARGING_DISABLE)
	if(smb_chg_disabled)// sayuss charge_CMD 
		return;
#endif
	if(cable_present_state() == 0)
		prve_cable_type = PANTECH_CABLE_NONE;

	if(prev_cable_current == current_ma){
		if(prve_cable_type == cable_type || cable_present_state() == 0)	
			return;
	}
	prev_cable_current = current_ma;
	prve_cable_type  = cable_type;	
	
	if(cable_type == PANTECH_USB){
#if defined(T_EF59L) && (BOARD_VER == WS20)
		smb349_B_register_init();
#endif
	if(is_factory_dummy){
				pr_debug("sayuss is_factory_dummy[%d]",is_factory_dummy);
				smb349_write_reg(0x30, 0x83); //charging enable
				smb349_write_reg(0x31,0x06); //usb 900mA
		}else{
			smb349_current_jeita(prev_state_mode, prve_cable_type);
		}
	}else if (cable_type == PANTECH_AC){ 
#if defined(T_EF59L) && (BOARD_VER == WS20)
		smb349_B_register_init();
#endif
		smb349_current_jeita(prev_state_mode, prve_cable_type);
	}


}
EXPORT_SYMBOL_GPL(smb349_current_set);

#if defined(FEATURE_PANTECH_PMIC_CHARGE_LIMIT_WITH_LCD)
void smb349_adjust_limit_LCD(int lcd_on)
{
	int is_lcd_on;
	is_lcd_on = lcd_on;

#if 1//sayuss defined(FEATURE_PANTECH_PMIC_CHARGE_LIMIT_WITH_LCD)

	if(!is_lcd_on)
	{
		if(boot_lcd_cnt < 4)
			boot_lcd_cnt++;

		if(3<boot_lcd_cnt)
		{
			offline_boot_ok = 1;
		}
	}
	else
	{
		offline_boot_ok=0; 
	}
	return;
#endif

	if((PANTECH_AC != qcom_cable_type) || (cable_present_state() == 0))
		return;

	if(is_lcd_on)
	{
		LCD_OFF = 0;
		smb349_current_jeita(prev_state_mode, qcom_cable_type);
		// sayuss : Never block or delete this message
		pr_err("++++++++++LCD On++++++++\n");

		// To do
	}
	else
	{
		LCD_OFF = 1;
		smb349_current_jeita(prev_state_mode, qcom_cable_type);
		// sayuss : Never block or delete this message
		pr_err("---------LCD Off -----------\n");
		// To do

	}
}
EXPORT_SYMBOL_GPL(smb349_adjust_limit_LCD);
#endif

#if defined(FEATURE_PANTECH_PMIC_CHARGE_LIMIT_WITH_FINGERPRINT)
int g_use_sensor = 0;

void smb349_adjust_limit_use_sensor(int use_sensor)
{
	if((PANTECH_AC != qcom_cable_type) || (cable_present_state() == 0))
		return;
	
	g_use_sensor = use_sensor;
	pr_debug("sensor mode limitation g_use_sensor %d, cable_type_value %d\n",use_sensor, prve_cable_type );
	if(g_use_sensor)
	{
		// sayuss : Never block or delete this message
		pr_err("++++++++++sensor mode limitation On++++++++\n");
		smb349_usb_mode(500);
	}
	else
	{
		// sayuss : Never block or delete this message
		pr_err("---------sensor mode limitation Off -----------\n");
		smb349_current_jeita(prev_state_mode, qcom_cable_type);	

	}
}
EXPORT_SYMBOL_GPL(smb349_adjust_limit_use_sensor);
#endif

//+++ 20130806, P14787, djjeon, charging setting stability test
#if defined(FEATURE_PANTECH_PMIC_MONITOR_TEST)
void smb349_test_limit_up(int on)
{
	if ((PANTECH_USB != qcom_cable_type) || (cable_present_state() == 0))
		return;
	
	if(on)
	{
		smb349_write_reg(0x30,0x81); //charging disable
		smb349_write_reg(0x00,0x36); // 1.5A
		smb349_write_reg(0x31,0x01); //hc mode
		smb349_write_reg(0x30,0x83); //charging enable
		smb349_write_reg(0x31,0x01); //hc mode
		smb349_write_reg(0x02,0x8F);

		pr_err("++++++++++test_limit_up On++++++++\n");
	
	}
	else
	{
		smb349_write_reg(0x02,0x9F);
		smb349_usb_mode(500);
		pr_err("---------test_limit_up  Off -----------\n");

	}
}
EXPORT_SYMBOL_GPL(smb349_test_limit_up);
#endif
//--- 20130806, P14787, djjeon, charging setting stability test

#if defined(FEATURE_PANTECH_PMIC_MONITOR_TEST)
unsigned char smb349_get_reg(unsigned char reg)
{
    unsigned char value;
    smb349_read_reg(reg, &value);
    return value;
}
EXPORT_SYMBOL_GPL(smb349_get_reg);
#endif /* FEATURE_PANTECH_PMIC_MONITOR_TEST */

static int __devinit smb349_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{

	struct smb349_data *smb349;  
	struct i2c_adapter *adapter;

	adapter = to_i2c_adapter(client->dev.parent);	
	smb349 = kzalloc(sizeof(struct smb349_data), GFP_KERNEL);

	if (!smb349) {
		pr_err("smb349 failed to alloc memory\n");
		return -ENOMEM;
	}

	if (!i2c_check_functionality(adapter, I2C_FUNC_I2C))
		return -EIO;


	smb349->client = client;
	smb349_client = client;
	the_smb349 = smb349;
	i2c_set_clientdata(client, smb349);
	
	smb349_print_all_reg();	

	pr_err("smb349 i2c probe OK\n");	
	return 0;
}
static int __devexit smb349_remove(struct i2c_client *client)
{
	struct smb349_data *smb349 = i2c_get_clientdata(client);
	
	kfree(smb349);
	return 0;
}

static int smb349_suspend(struct device *dev)
{
	return 0;
}

static int smb349_resume(struct device *dev)
{
	return 0;
}
static const struct dev_pm_ops smb349_pm_ops = {
	.suspend	= smb349_suspend,
	.resume		= smb349_resume,
};
static void smb349_shutdown(struct i2c_client *client)
{
	smb349_write_reg(0x30,0x81); //charging disable
	smb349_write_reg(0x31,0x00); //mode change 
}

static const struct i2c_device_id smb349_id[] = {
        {"smb349-i2c", 0},
        {},
};
MODULE_DEVICE_TABLE(i2c, smb349_device_id);
#ifdef CONFIG_OF
static struct of_device_id smb349_i2c_table[] = {
	{ .compatible = "smb349,smb349-i2c",}, // Compatible node must match dts
	{ },
};
#else
#define smb349_i2c_table NULL
#endif

static struct i2c_driver smb349_i2c_driver={
	.id_table = smb349_id ,
	.probe = smb349_i2c_probe ,
	.remove		= __devexit_p(smb349_remove),
	.shutdown = smb349_shutdown,
	.driver = {
		.name = "smb349-i2c",
		.pm	= &smb349_pm_ops,
		.owner = THIS_MODULE ,
		.of_match_table = smb349_i2c_table ,
	},
};

static int __init smb349_init(void)
{
	int rc;
	printk(KERN_ERR "smb349_i2c \n");

	rc = i2c_add_driver(&smb349_i2c_driver);
	if(rc) {
		printk(KERN_ERR "smb349_i2c driver add failed.\n");
	}
	
	return rc;
}

static void __exit smb349_exit(void)
{
	i2c_del_driver(&smb349_i2c_driver);
}

module_init(smb349_init);
module_exit(smb349_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("smb349 charger/battery driver");
MODULE_VERSION("1.0");
MODULE_ALIAS("platform:" smb349_charger);

