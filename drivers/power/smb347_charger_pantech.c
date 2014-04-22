 
#include <linux/module.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/regulator/consumer.h>
#include <linux/string.h>
#include <linux/of_gpio.h>
#include <linux/kernel.h>
#include <linux/power/smb347_charger_pantech.h>
#include <linux/workqueue.h>  
#include <linux/qpnp/qpnp-adc.h>
#include <linux/delay.h>
#if defined(FEATURE_PANTECH_PMIC_OTG_UVLO)
#include <linux/mutex.h>
#endif
#include <linux/types.h>
#include <linux/err.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/power_supply.h>
#include <linux/spmi.h>
#include <linux/rtc.h>
#include <linux/qpnp/qpnp-adc.h>
#include <linux/mfd/pm8xxx/batterydata-lib.h>
/* CONFIG_PANTECH_PMIC_CHARGER_SMB347 */
#include <mach/gpio.h>
#include <mach/gpiomux.h>

#if defined FEATURE_PANTECH_PMIC_AUTO_PWR_ON
#include <linux/reboot.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <asm/uaccess.h>
#include <linux/workqueue.h>

#include <mach/msm_smsm.h>
#endif
#if defined(FEATURE_PANTECH_PMIC_CHARGE_LIMIT_WITH_TSENSOR)
#include <linux/msm_tsens.h>
#endif

/*CONFIG_PANTECH_PMIC_CHARGER_SMB347*/
enum
{
	GPIO_LOW_VALUE = 0,
	GPIO_HIGH_VALUE,
};
enum
{
	WR_COMMAND = 0x0,
	RD_COMMAND,
};

extern int cable_present_state(void);

/*CONFIG_PANTECH_PMIC_CHARGER_SMB347*/
#define SMB347_CHIP_ADDR		0x0c
#define SMB_SCL_PIN			84
#define SMB_SDA_PIN			83
#define SMB_CHG_SUSP            31
#define SMB_I2C_SDA_OUT	gpio_tlmm_config(GPIO_CFG(SMB_SDA_PIN, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA), GPIO_CFG_ENABLE)
#define SMB_I2C_SDA_IN		gpio_tlmm_config(GPIO_CFG(SMB_SDA_PIN, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA), GPIO_CFG_ENABLE)

/* Register definitions */
#define CHG_CTRL_REG				0x04	


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

#if defined(FEATURE_PANTECH_PMIC_CHARGING_DISABLE)
bool smb_chg_disabled ;  // sayuss charge_CMD 
#endif

static int charger_ready = false;
#if defined(FEATURE_PANTECH_PMIC_OTG_UVLO)
DEFINE_MUTEX(otg_lock_mutex);
#endif
static const int therm_boundary_min[BATT_THERM_MAX]={-2000, -80, 20, 400, 550};
static const int therm_boundary_max[BATT_THERM_MAX]={-50, 50, 430, 580, 2000};
static const int therm_boundary_init[BATT_THERM_MAX]={-2000, -80, 20, 430, 580};
static int prve_cable_type = PANTECH_CABLE_NONE;
static int qcom_cable_type = PANTECH_CABLE_NONE;
int prev_state_mode = BATT_THERM_UNKNOWN;
int prev_state_temp_mode = BATT_THERM_UNKNOWN;
static int prev_cable_current = 0;
#if defined(FEATURE_PANTECH_PMIC_CHARGE_LIMIT_WITH_TSENSOR)
bool T_sensor_mode = false;
bool t_prev_mode = false;
#endif 

#if defined(FEATURE_PANTECH_PMIC_I2C_LOCK)
struct _smb347 {
	struct mutex reg_lock;
};
struct _smb347 smb347;
#endif

/*CONFIG_PANTECH_PMIC_CHARGER_SMB347*/
static void SMB347_StartCondition(void)
{
	//int ret =0;
	gpio_tlmm_config(GPIO_CFG(SMB_SDA_PIN, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA), GPIO_CFG_ENABLE);

	gpio_direction_output(SMB_SDA_PIN, GPIO_HIGH_VALUE);
	gpio_direction_output(SMB_SCL_PIN, GPIO_HIGH_VALUE);
	udelay(5);	
       //mdelay(5);
	gpio_direction_output(SMB_SDA_PIN, GPIO_LOW_VALUE);
	udelay(4);
	//mdelay(4);
	gpio_direction_output(SMB_SCL_PIN, GPIO_LOW_VALUE);
}

static void SMB347_StopCondition(void)
{
	gpio_tlmm_config(GPIO_CFG(SMB_SDA_PIN, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA), GPIO_CFG_ENABLE);

	gpio_direction_output(SMB_SDA_PIN, GPIO_LOW_VALUE);
	gpio_direction_output(SMB_SCL_PIN, GPIO_HIGH_VALUE);
	udelay(4);
	//mdelay(4);
	gpio_direction_output(SMB_SDA_PIN, GPIO_HIGH_VALUE);
	mdelay(5);
}

static int SMB347_I2C_SendByte(unsigned char Data)
{
	int i = 0;
	int TimeOutFlag = 1;
	int ret = 0;

	gpio_tlmm_config(GPIO_CFG(SMB_SDA_PIN, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA), GPIO_CFG_ENABLE);

//	gpio_set(SMB_SDA_PIN, GPIO_HIGH_VALUE);
	
	for (i = 0; i < 8; i++)
	{
		if (Data & 0x80)
			gpio_direction_output(SMB_SDA_PIN, GPIO_HIGH_VALUE);
		else
			gpio_direction_output(SMB_SDA_PIN, GPIO_LOW_VALUE);	

		udelay(2);
              //mdelay(2);
		gpio_direction_output(SMB_SCL_PIN, GPIO_HIGH_VALUE);
		udelay(4);
		//mdelay(2);
		gpio_direction_output(SMB_SCL_PIN, GPIO_LOW_VALUE);
		udelay(5);
		//mdelay(2);

		Data = Data << 1;
	}

	gpio_direction_output(SMB_SDA_PIN, GPIO_LOW_VALUE);	
	gpio_tlmm_config(GPIO_CFG(SMB_SDA_PIN, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	mdelay(4);	
	gpio_direction_output(SMB_SCL_PIN, GPIO_HIGH_VALUE);	

	i = 0;
	
	while ((((gpio_get_value(SMB_SDA_PIN)) ? 1 : 0) == 1) && TimeOutFlag)
	{
		i++;
		udelay(1);		
		if (i >= 100)
		{
			TimeOutFlag = 0;
			ret = 1;
			//dprintf(1, "I2C FAIL \n");
			//g_SMB_I2C_flag_validAdrress++;
		}
	}
	gpio_direction_output(SMB_SCL_PIN, GPIO_LOW_VALUE);

	return ret;
}

static int SMB347_I2C_ReadByte(unsigned char * RdData)
{
	int i = 0;
	int ret = 1;
	unsigned char TempData = 0;

	gpio_tlmm_config(GPIO_CFG(SMB_SDA_PIN, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	
	for (i = 0; i < 8; i++)
	{
		TempData <<= 1;
		udelay(5);
		//mdelay(5);
		gpio_direction_output(SMB_SCL_PIN, GPIO_HIGH_VALUE);
		udelay(4);
		//mdelay(4);

		TempData |= ((((gpio_get_value(SMB_SDA_PIN)) ? 1 : 0)) & 0x01);
		gpio_direction_output(SMB_SCL_PIN, GPIO_LOW_VALUE);		

	}

	//dprintf(1, "0x%x  \n",TempData);

	*RdData = TempData;
	return ret;
}

static void SMB347_RePStartCondition(void)
{
	gpio_tlmm_config(GPIO_CFG(SMB_SDA_PIN, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	gpio_direction_output(SMB_SDA_PIN, GPIO_HIGH_VALUE);
	gpio_direction_output(SMB_SCL_PIN, GPIO_HIGH_VALUE);
	udelay(5);
	//mdelay(5);
	gpio_direction_output(SMB_SDA_PIN, GPIO_LOW_VALUE);
	udelay(4);
	//mdelay(4);
	gpio_direction_output(SMB_SCL_PIN, GPIO_LOW_VALUE);
}
/*
static void I2C_ACK(void)
{
	gpio_direction_output(SMB_SDA_PIN, GPIO_LOW_VALUE);	
//	gpio_tlmm_config(SMB_SDA_PIN,0,GPIO_OUTPUT,GPIO_PULL_UP,0,GPIO_ENABLE);
	gpio_tlmm_config(GPIO_CFG(SMB_SCL_PIN, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA), GPIO_CFG_DISABLE);
	gpio_direction_output(SMB_SCL_PIN, GPIO_HIGH_VALUE);
	mdelay(4);
	
	gpio_direction_output(SMB_SCL_PIN, GPIO_LOW_VALUE);
	mdelay(4);
}
*/
static void SMB347_I2C_NACK(void)
{
	gpio_direction_output(SMB_SDA_PIN, GPIO_HIGH_VALUE);	

//	gpio_tlmm_config(SMB_SDA_PIN,0,GPIO_OUTPUT,GPIO_PULL_UP,0,GPIO_ENABLE);
//	gpio_tlmm_config(GPIO_CFG(SMB_SDA_PIN, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA), GPIO_CFG_ENABLE);	
	gpio_direction_output(SMB_SCL_PIN, GPIO_HIGH_VALUE);
	udelay(4);
	//mdelay(4);
	gpio_direction_output(SMB_SCL_PIN, GPIO_LOW_VALUE);
	udelay(4);
	//mdelay(4);
}

int smb349_read_reg(u8 reg, unsigned char *val)
{
	unsigned char RdDataMLSB = 0;
	int status = 0;

#if defined(FEATURE_PANTECH_PMIC_I2C_LOCK)
    mutex_lock(&smb347.reg_lock);
#endif

	SMB347_StartCondition();

	status = SMB347_I2C_SendByte(SMB347_CHIP_ADDR | WR_COMMAND);

	if (status == 0)
	{	
		//dprintf(1, "[CDS] Rd 1\n");
		status = SMB347_I2C_SendByte(reg);
		if (status == 0)
		{
			//dprintf(1, "[CDS] Rd 2\n");		
			SMB347_RePStartCondition();
			status = SMB347_I2C_SendByte(SMB347_CHIP_ADDR | RD_COMMAND);
			if (status == 0)
			{
				//dprintf(1, "[CDS] Rd 3 \n");		
				SMB347_I2C_ReadByte(&RdDataMLSB);
			}
			SMB347_I2C_NACK();
		}
	}

	SMB347_StopCondition();
	*val = RdDataMLSB;
	
#if defined(FEATURE_PANTECH_PMIC_I2C_LOCK)
    mutex_unlock(&smb347.reg_lock);
#endif

	return 0;
}


int smb349_write_reg(u8 reg, unsigned char val)
{
	int status = 0;
	//int ret;

#if defined(FEATURE_PANTECH_PMIC_I2C_LOCK)    
    mutex_lock(&smb347.reg_lock);
#endif

	SMB347_StartCondition();
	
	status = SMB347_I2C_SendByte((SMB347_CHIP_ADDR) | WR_COMMAND);

	if (status == 0)
	{	
		//dprintf(1, "Wr 1\n");
		status = SMB347_I2C_SendByte(reg);
		if (status == 0)
		{
			//dprintf(1, "Wr 2\n");
			status = SMB347_I2C_SendByte (val);
			//dprintf(1, "WRITE OK\n");	
		}	
	} else {
		SMB347_StopCondition();
		pr_err("smb349 i2c write fail: can't write %02x to %02x: %d\n",
			val, reg, status);
		return status;
	}
	SMB347_StopCondition();
	
#if defined(FEATURE_PANTECH_PMIC_I2C_LOCK)
    mutex_unlock(&smb347.reg_lock);
#endif

	return 0;
}

void smb349_otg_power(int on)
{
	printk("%s Enter on=%d\n", __func__, on);
#if defined(FEATURE_PANTECH_PMIC_OTG_UVLO)
	mutex_lock(&otg_lock_mutex);
#endif
	if (on){		 
		smb349_write_reg(0x30,0x91); // OTG: enable, STAT: disable
		smb349_write_reg(0x31,0x00); // USB1 or USB1.5
		prve_cable_type = 0;
	}
	else {
		smb349_write_reg(0x30,0xC0); // OTG: disable, STAT: enable, disable charging
		smb349_write_reg(0x31,0x00); // USB1 or USB1.5	
	}
#if defined(FEATURE_PANTECH_PMIC_OTG_UVLO)
	mutex_unlock(&otg_lock_mutex);
#endif
}
EXPORT_SYMBOL_GPL(smb349_otg_power);

/*56S TP10 board apply*/
#ifdef FEATURE_PANTECH_PMIC_OTG 
void smb349_otg_power_current(int mode)
{
	printk("set otg index %d\n", mode);
#if defined(FEATURE_PANTECH_PMIC_OTG_UVLO)
	mutex_lock(&otg_lock_mutex);
#endif
	switch(mode){
		case 1:
				smb349_write_reg(0x0A,0xB7);  //250mA 
				
				break;	
		case 2:
				smb349_write_reg(0x0A,0xBB); //500mA
				break;	
		case 3:
				smb349_write_reg(0x0A,0xBF); //750mA
				break;	
		default:
				break;
	}
#if defined(FEATURE_PANTECH_PMIC_OTG_UVLO)
	mutex_unlock(&otg_lock_mutex);
#endif
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
	u8 term_satatus = 0;
	u8 is_invalid_temp;
	
	smb349_read_reg(CHG_CTRL_REG, &term_satatus);
	if (term_satatus < 0)
			return;
	if(enable){
		term_satatus &= ~CHG_CTRL_TERM_ENABLE;
	}else{
		term_satatus |= CHG_CTRL_TERM_ENABLE;		
	}	
	term_satatus = smb349_write_reg(CHG_CTRL_REG, term_satatus);
	smb349_read_reg(0x04, &is_invalid_temp);
	pr_err("0x04 value = %.2X\n", is_invalid_temp);
}
EXPORT_SYMBOL_GPL(end_recharging_enable);
#endif
void smb349_charing_enable(bool enable)
{
	u8 charging_status;
	u8 is_invalid_temp;	
	
	smb349_read_reg(CMD_A, &charging_status);
	if (charging_status < 0)
			return;
	if(enable){
		charging_status |= CMD_A_CHG_ENABLED;
		charging_status = smb349_write_reg(CMD_A, charging_status);	
		smb349_read_reg(0x30, &is_invalid_temp);
		pr_err("0x30 value = %.2X\n", is_invalid_temp);
	}else{
		charging_status &= ~CMD_A_CHG_ENABLED;
		charging_status = smb349_write_reg(CMD_A, charging_status);
		smb349_read_reg(0x30, &is_invalid_temp);
		pr_err("0x30 value = %.2X\n", is_invalid_temp);
	}
}
EXPORT_SYMBOL_GPL(smb349_charing_enable);	


static void smb349_usb_mode(int current_ma)
{
	if (true != get_is_offline_charging_mode()) {
		if(current_ma == 100){
			smb349_write_reg(0x30,0x83); //charging enable
			smb349_write_reg(0x31,0x00); //usb 100 
		}else if(current_ma == 900){
			smb349_write_reg(0x30,0x83); //charging enable
			smb349_write_reg(0x31,0x02); //usb 900
			smb349_write_reg(0x08, 0x38); //USB3.0

		}else{	
			smb349_write_reg(0x30,0x83); //charging enable
			smb349_write_reg(0x31,0x02); //usb 500
			smb349_write_reg(0x08, 0x18); //USB2.0		
		}
	}else{
			smb349_write_reg(0x30,0x83); //charging enable
			smb349_write_reg(0x31,0x02); //usb 500
			smb349_write_reg(0x08, 0x18); //USB2.0		

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


#if defined(T_EF59S) || defined(T_EF59K) || defined(T_EF59L) 
#define WARM_COOL_USBIN 0x15  //USBIN 1.5A
#define WARM_COOL_FAST  0x59  // FAST 1.2A, term 50mA
#define NORMAL_USBIN 0x16  //USBIN 1.8A
#define NORMAL_FAST  0xB9  // FAST 2A, term 50mA
#elif defined(T_EF60S) || defined(T_EF61K) || defined(T_EF62L)
#define WARM_COOL_USBIN 0x16 //USBIN 1.8A
#define WARM_COOL_FAST  0x39 // FAST 900mA, term 50mA
#define NORMAL_USBIN 0x16 //USBIN 1.8A
#define NORMAL_FAST  0xB9 // FAST 2A, term 50mA
#else
#define WARM_COOL_USBIN 0x15
#define WARM_COOL_FAST  0x59
#define NORMAL_USBIN 0x16
#define NORMAL_FAST  0xBA
#endif
void smb349_current_jeita (int mode, int cable_type)
{
#if defined(FEATURE_PANTECH_PMIC_CHARGING_DISABLE)
	// sayuss charge_CMD 
	if(smb_chg_disabled) 
		return;
#endif
	if(cable_present_state() == 0)
		return;
	
	switch(mode){
		case BATT_THERM_COLD:
//			pr_info("======== jeita mode (%d) set cold =========\n",mode);
			smb349_write_reg(0x30,0x81); //charging disable
			smb349_write_reg(0x31,0x00); //mode change 
			prev_state_temp_mode = BATT_THERM_COLD;
			break;
			
		case BATT_THERM_COOL: 
//			pr_info("======== jeita mode (%d) set cool =========\n",mode);
			if(cable_type == PANTECH_USB){
				smb349_usb_mode(prev_cable_current);
				break;
			}
			if(prev_state_temp_mode == BATT_THERM_COLD){
			smb349_write_reg(0x30, 0x81); //charging disable
			smb349_write_reg(0x31, 0x02); //usb mode
			}
			smb349_write_reg(0x01, WARM_COOL_USBIN); 
			smb349_write_reg(0x00, WARM_COOL_FAST); 
			smb349_write_reg(0x30, 0x83); //charging enable
			smb349_write_reg(0x31, 0x01); //hc mode	
			prev_state_temp_mode = BATT_THERM_COOL;
			break;	
		case BATT_THERM_NORMAL:
//			pr_info("======== jeita mode (%d) set mormal 1 =========\n",mode);
			if(cable_type == PANTECH_USB){
				smb349_usb_mode(prev_cable_current);
				break;
			}
			if(prev_state_temp_mode == BATT_THERM_COOL || prev_state_temp_mode == BATT_THERM_WARM){
			smb349_write_reg(0x30, 0x81); //charging disable
			smb349_write_reg(0x31, 0x02);  //usb mode
			}
			smb349_write_reg(0x01, NORMAL_USBIN); 
			smb349_write_reg(0x00, NORMAL_FAST);  
			smb349_write_reg(0x30, 0x83); //charging enable
			smb349_write_reg(0x31, 0x01);  //hc mode	
			prev_state_temp_mode = BATT_THERM_NORMAL;
			break;
			
		case BATT_THERM_WARM:
//			pr_info("======== jeita mode (%d) set warm =========\n",mode);
			if(cable_type == PANTECH_USB){
				smb349_usb_mode(prev_cable_current);
				break;
			}
			if(prev_state_temp_mode == BATT_THERM_HOT){
			smb349_write_reg(0x30, 0x81); //charging disable
			smb349_write_reg(0x31,0x02); //usb mode
			}
			smb349_write_reg(0x01, WARM_COOL_USBIN); 
			smb349_write_reg(0x00, WARM_COOL_FAST); 
			smb349_write_reg(0x30, 0x83); //charging enable
			smb349_write_reg(0x31, 0x01);  //hc mode	
			prev_state_temp_mode = BATT_THERM_WARM;
			break;
			
		case BATT_THERM_HOT:
//			pr_info("======== jeita mode (%d) set hot =========\n",mode);
			smb349_write_reg(0x30,0x81); //charging disable
			smb349_write_reg(0x31,0x00); //mode change 
			prev_state_temp_mode = BATT_THERM_HOT;
			
		default:
			break;
		}
	//smb349_print_all_reg();
}
EXPORT_SYMBOL_GPL(smb349_current_jeita);

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
		if(is_factory_dummy){
				pr_err("sayuss is_factory_dummy[%d]",is_factory_dummy);
				smb349_write_reg(0x30, 0x83); //charging enable
				smb349_write_reg(0x31, 0x02); //USB 900mA
   				smb349_write_reg(0x08, 0x38); //USB 3.0 mode 
		}else{
			smb349_current_jeita(prev_state_mode, prve_cable_type);
		}
	}else if (cable_type == PANTECH_AC){ 
		smb349_current_jeita(prev_state_mode, prve_cable_type);
	}

}
EXPORT_SYMBOL_GPL(smb349_current_set);

#if defined(FEATURE_PANTECH_PMIC_CHARGE_LIMIT_WITH_LCD)
void smb349_adjust_limit_LCD(int lcd_on)
{
	int is_lcd_on;
	is_lcd_on = lcd_on;
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
	pr_err("sensor mode limitation g_use_sensor %d, cable_type_value %d\n",use_sensor, prve_cable_type );
	if(g_use_sensor)
	{
		// sayuss : Never block or delete this message
		pr_err("++++++++++sensor mode limitation On++++++++\n");
//		smb349_usb_mode(500);
	}
	else
	{
		// sayuss : Never block or delete this message
		pr_err("---------sensor mode limitation Off -----------\n");
//		smb349_current_jeita(prev_state_mode, qcom_cable_type);	

	}
}
EXPORT_SYMBOL_GPL(smb349_adjust_limit_use_sensor);
#endif

#if defined(FEATURE_PANTECH_PMIC_CHARGE_LIMIT_WITH_TSENSOR)
#define T_THERM_MAX   53
#define T_THERM_MIN	  48

void smb349_t_sense_limit(int mode)
{
	if((PANTECH_AC != qcom_cable_type) || (cable_present_state() == 0) || t_prev_mode == mode)
		return;
	
	t_prev_mode = mode;
	if(t_prev_mode)
	{
		pr_err("++++++++++T_sensor mode limitation On++++++++\n");
#if defined(T_EF60S) || defined(T_EF61K) || defined(T_EF62L)
		smb349_write_reg(0x00, 0x3A); // FAST 900mA
#else
		smb349_write_reg(0x01, 0x15); //USBIN 1.5A
		smb349_write_reg(0x00, 0x59); // FAST 1.2A
#endif		
		smb349_write_reg(0x30, 0x83); //charging enable
		smb349_write_reg(0x31, 0x01);  //hc mode	
		T_sensor_mode = true;
	}
	else
	{
		pr_err("---------T_sensor mode limitation Off -----------\n");
		if(prev_state_temp_mode == BATT_THERM_NORMAL){
			smb349_write_reg(0x30, 0x81); //charging disable
			smb349_write_reg(0x31, 0x02); //usb mode
		}
		smb349_current_jeita(prev_state_mode, qcom_cable_type);
		T_sensor_mode = false;

	}
}
EXPORT_SYMBOL_GPL(smb349_t_sense_limit);

int charger_ready_status(void)
{
	return charger_ready;
}
EXPORT_SYMBOL_GPL(charger_ready_status);
int t_sensor_value(void)
{
	struct tsens_device tsens_dev;
	long T_sense_2;
	tsens_dev.sensor_num = 2; 
	tsens_get_temp(&tsens_dev, &T_sense_2);
	pr_debug(" T_sense_2 %ld, T_sensor_mode %d \n",T_sense_2, T_sensor_mode);

	if(T_sense_2 > T_THERM_MAX){
		return T_SENSE_UP;
	}else if(T_sensor_mode == true && T_sense_2 < T_THERM_MIN){
		return T_SENSE_DOWN;
	}else
		return T_SENSE_NORMAL;
	
}
EXPORT_SYMBOL_GPL(t_sensor_value);
#endif
#if defined(FEATURE_PANTECH_PMIC_OTG_UVLO)
int smb347_get_otg_status(void)
{
	u8 reg,rc, ret;
	mutex_lock(&otg_lock_mutex);

	rc =  smb349_read_reg(0x0C,&reg);

	if (!rc)
		ret = (0x20 &reg) >> 5;

	mutex_unlock(&otg_lock_mutex);

	pr_err("[DEBUG] smb347_get_otg_status = 0x%03X\n", reg);

	return ret;
}
EXPORT_SYMBOL_GPL(smb347_get_otg_status);
#endif

//+++ 20130806, P14787, djjeon, charging setting stability test
#if defined(FEATURE_PANTECH_PMIC_MONITOR_TEST)
void smb349_test_limit_up(int on)
{

	if ((PANTECH_USB != qcom_cable_type) || (cable_present_state() == 0)){

		return;
	}
	
	if(on)
	{
		smb349_write_reg(0x30, 0x81); //charging disable
		smb349_write_reg(0x31, 0x01);  //hc mode
		smb349_write_reg(0x00, 0x79);  // FAST 1.5A
		smb349_write_reg(0x30, 0x83); //charging enable
		smb349_write_reg(0x31, 0x01);  //hc mode	
		pr_err("++++++++++test_limit_up On++++++++\n");
	
	}
	else
	{
		smb349_usb_mode(500);
		pr_err("---------test_limit_up  Off -----------\n");

	}
}
EXPORT_SYMBOL_GPL(smb349_test_limit_up);
#endif

#if defined(FEATURE_PANTECH_PMIC_MONITOR_TEST)
unsigned char smb349_get_reg(unsigned char reg)
{
    unsigned char value;
    smb349_read_reg(reg, &value);
    return value;
}
EXPORT_SYMBOL_GPL(smb349_get_reg);
#endif /* FEATURE_PANTECH_PMIC_MONITOR_TEST */

//--- 20130806, P14787, djjeon, charging setting stability test
static int __init smb349_init(void)
{
	//int rc;
	printk(KERN_ERR "smb347_i2c \n");

#if defined(FEATURE_PANTECH_PMIC_I2C_LOCK)    
    mutex_init(&smb347.reg_lock);
#endif

	gpio_request(SMB_SCL_PIN, "smb_scl");
	gpio_request(SMB_SDA_PIN, "smb_sda");
	
	gpio_tlmm_config(GPIO_CFG(SMB_SCL_PIN,0,GPIO_CFG_OUTPUT,GPIO_CFG_NO_PULL,GPIO_CFG_2MA),GPIO_CFG_DISABLE);		
	gpio_tlmm_config(GPIO_CFG(SMB_SDA_PIN,0,GPIO_CFG_OUTPUT,GPIO_CFG_NO_PULL,GPIO_CFG_2MA),GPIO_CFG_DISABLE);
	
	mdelay(10);

	smb349_print_all_reg();
	printk(KERN_ERR "smb347_init OK! \n");
	charger_ready = true;

	return 0;
}

static void __exit smb349_exit(void)
{
//	i2c_del_driver(&smb349_i2c_driver);
#if defined(FEATURE_PANTECH_PMIC_I2C_LOCK)
    mutex_destroy(&smb347.reg_lock);
#endif
}

module_init(smb349_init);
module_exit(smb349_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("smb349 charger/battery driver");
MODULE_VERSION("1.0");
MODULE_ALIAS("platform:" smb349_charger);

