/**********************************************************************************
 * Si8240 Linux Driver
 *
 * Copyright (C) 2011-2012 Silicon Image Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2.
 *
 * This program is distributed .as is. WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the
 * GNU General Public License for more details.
 *
 **********************************************************************************/


/**
 * @file sii_hal_linux_gpio.c
 *
 * @brief Linux implementation of GPIO pin support needed by Silicon Image
 *        MHL devices.
 *
 * $Author: Dave Canfield
 * $Rev: $
 * $Date: Feb. 9, 2011
 *
 *****************************************************************************/

#define SII_HAL_LINUX_GPIO_C

/***** #include statements ***************************************************/
#include "sii_hal.h"
#include "sii_hal_priv.h"
#include "si_c99support.h"
#include "si_osdebug.h"
#include "../../../../mdss/mdss_io_util.h"
#include "../../../../mdss/mhl_sii8240.h"

/***** local macro definitions ***********************************************/


/***** local type definitions ************************************************/


/***** local variable declarations *******************************************/


/***** local function prototypes *********************************************/


/***** global variable declarations *******************************************/


// Simulate the DIP switches
bool	pinDbgMsgs	= false;	// simulated pinDbgSw2 0=Print
bool	pinAllowD3	= true;	// false allows debugging
bool	pinOverrideTiming = true;	// simulated pinDbgSw2
bool	pinDataLaneL	= true;		// simulated pinDbgSw3
bool	pinDataLaneH	= true;		// simulated pinDbgSw4
bool	pinDoHdcp	= true;		// simulated pinDbgSw5
bool    pinWakePulseEn = true;   // wake pulses enabled by default
bool	pinPackedPixelUserControl=false;
bool	pinPackedPixelUserOn=true;
bool	pinTranscodeMode=false;

//	 Simulate the GPIO pins
bool	 pinTxHwReset	= false;	// simulated reset pin %%%% TODO possible on Beagle?
bool	 pinM2uVbusCtrlM	= true;		// Active high, needs to be low in MHL connected state, high otherwise.
bool	 pinVbusEnM	= true;		// Active high input. If high sk is providing power, otherwise not.
bool	pinMhlVbusSense=false;

// Simulate the LEDs
bool	pinMhlConn	= true;		// MHL Connected LED 
bool	pinUsbConn	= false;	// USB connected LED
bool	pinSourceVbusOn	= true;		// Active low LED. On when pinMhlVbusSense and pinVbusEnM are active
bool	pinSinkVbusOn	= true;		// Active low LED. On when pinMhlVbusSense is active and pinVbusEnM is not active

/*<ZORO_MHL> start*/
///unsigned int usb_switch_gpio_sw_sel = MHL_GPIO_USBSW_SEL;
//unsigned int usb_switch_gpio_sw_oe_n = PM8921_GPIO_PM_TO_SYS(11);
/*<ZORO_MHL> end*/

/***** local functions *******************************************************/
struct mhl_tx_ctrl *mhl_ctrl_save;

/***** public functions ******************************************************/


/*****************************************************************************/
/**
 * @brief Configure platform GPIOs needed by the MHL device.
 *
 *****************************************************************************/
 
halReturn_t HalGpioInit(struct mhl_tx_ctrl *mhl_ctrl)
{
	int status;
	struct dss_gpio *temp_reset_gpio, *temp_intr_gpio,*temp_mhl_usb_sw_sel,*temp_pmic_usb_id;
	struct mhl_tx_ctrl * temp_mhl_ctrl;

	temp_mhl_ctrl=mhl_ctrl;


	/* caused too many line spills */
	temp_reset_gpio = temp_mhl_ctrl->pdata->gpios[MHL_TX_RESET_GPIO];
	temp_intr_gpio = temp_mhl_ctrl->pdata->gpios[MHL_TX_INTR_GPIO];
	temp_mhl_usb_sw_sel=temp_mhl_ctrl->pdata->gpios[MHL_TX_MHL_USB_SEL_GPIO];
	temp_pmic_usb_id=temp_mhl_ctrl->pdata->gpios[MHL_TX_PMIC_USB_ID_GPIO];
	///////////////////////////////////////////////////////////////////////////////////
	/// RESET
	/* Configure GPIO used to perform a hard reset of the device. */
	status = gpio_request(temp_reset_gpio->gpio, temp_reset_gpio->gpio_name);
	if (status < 0)
	{
    	SII_DEBUG_PRINT(SII_OSAL_DEBUG_TRACE,
    			"HalInit gpio_request for GPIO %d (H/W Reset) failed, status: %d\n",
    			temp_reset_gpio->gpio, status);
		return HAL_RET_FAILURE;
	}
	
	status = gpio_direction_output(temp_reset_gpio->gpio, 1);
	if (status < 0)
	{
    	SII_DEBUG_PRINT(SII_OSAL_DEBUG_TRACE,
    			"HalInit gpio_direction_output for GPIO %d (H/W Reset) failed, status: %d\n",
    			temp_reset_gpio->gpio, status);
		gpio_free(temp_reset_gpio->gpio);
		return HAL_RET_FAILURE;
	}
	pr_info("Reset Gpio Init %s[%d]\n",temp_reset_gpio->gpio_name,temp_reset_gpio->gpio);
#ifdef MAKE_8240_DRIVER //(
    // don't do the stuff in the else branch
#elif defined(MAKE_833X_DRIVER) //)(
    // don't do the stuff in the else branch
#else //)(

	/* Configure GPIO used to control USB VBUS power. */
	status = gpio_request(M2U_VBUS_CTRL_M, "W_RST#");
	if (status < 0)
	{
    	SII_DEBUG_PRINT(SII_OSAL_DEBUG_TRACE,
    			"HalInit gpio_request for GPIO %d (VBUS) failed, status: %d\n",
    			M2U_VBUS_CTRL_M, status);
		return HAL_RET_FAILURE;
	}

	status = gpio_direction_output(M2U_VBUS_CTRL_M, 0);
	if (status < 0)
	{
    	SII_DEBUG_PRINT(SII_OSAL_DEBUG_TRACE,
    			"HalInit gpio_direction_output for GPIO %d (VBUS) failed, status: %d\n",
    			M2U_VBUS_CTRL_M, status);
		gpio_free(W_RST_GPIO);
		gpio_free(M2U_VBUS_CTRL_M);
		return HAL_RET_FAILURE;
	}
#endif	//)

	///////////////////////////////////////////////////////////////////////////////////
	/// MHL_INT
	/*
	 * Configure the GPIO used as an interrupt input from the device
	 * NOTE: GPIO support should probably be initialized BEFORE enabling
	 * interrupt support
	 */
	status = gpio_request(temp_intr_gpio->gpio, temp_intr_gpio->gpio_name);
	if(status < 0)
	{
    	SII_DEBUG_PRINT(SII_OSAL_DEBUG_TRACE,
    			"HalInitGpio gpio_request for GPIO %d (interrupt)failed, status: %d\n",
    			temp_intr_gpio->gpio, status);
		gpio_free(temp_intr_gpio->gpio);
		return HAL_RET_FAILURE;
	}

	status = gpio_direction_input(temp_intr_gpio->gpio);
	if(status < 0)
	{
    	SII_DEBUG_PRINT(SII_OSAL_DEBUG_TRACE,
    			"HalInitGpio gpio_direction_input for GPIO %d (interrupt)failed, status: %d",
    			temp_intr_gpio->gpio, status);
		gpio_free(temp_intr_gpio->gpio);
		gpio_free(temp_reset_gpio->gpio);
#ifdef MAKE_8240_DRIVER //(
    // don't do the stuff in the else branch
#elif defined(MAKE_833X_DRIVER) //)(
    // don't do the stuff in the else branch
#else //)(
		gpio_free(M2U_VBUS_CTRL_M);
#endif //)
		return HAL_RET_FAILURE;
	}

	pr_info("%s:#1 gpio_to_irq=%d\n",__func__, mhl_ctrl->i2c_handle->irq);
	mhl_ctrl->i2c_handle->irq = gpio_to_irq(
				temp_intr_gpio->gpio);
	pr_info("%s:#2 gpio_to_irq=%d\n",__func__, mhl_ctrl->i2c_handle->irq);
			
	pr_info("Intr  Gpio Init %s[%d]\n",temp_intr_gpio->gpio_name,temp_intr_gpio->gpio);
#if 1 /*<ZORO_MHL>*/
	///////////////////////////////////////////////////////////////////////////////////
	/// MHL_USB_SEL
	status = gpio_request(temp_mhl_usb_sw_sel->gpio,temp_mhl_usb_sw_sel->gpio_name);
	if(status < 0) {
		pr_err("%s(): gpio_request(usb switch sw sel) failed, error code %d\n",
			__func__, status);
		gpio_free(temp_intr_gpio->gpio);
		gpio_free(temp_reset_gpio->gpio);
		return HAL_RET_FAILURE;
	}
	status = gpio_direction_output(temp_mhl_usb_sw_sel->gpio, 1);
	if (status < 0) {
		pr_err("SET GPIO usb switch sw sel direction failed: %d\n",status);
		gpio_free(temp_intr_gpio->gpio);
		gpio_free(temp_reset_gpio->gpio);
		gpio_free(temp_mhl_usb_sw_sel->gpio);
		return HAL_RET_FAILURE;
	}
	gpio_set_value(temp_mhl_usb_sw_sel->gpio, USBSW_SEL_MODE_USB);
	pr_info("usb sel  Gpio Init %s[%d]\n",temp_mhl_usb_sw_sel->gpio_name,temp_mhl_usb_sw_sel->gpio);

	///////////////////////////////////////////////////////////////////////////////////
	/// USB_ID PMIC_GPIO 23(MUX_USB_ID, PMIC_MPP)
#if 1	
	status = gpio_request(temp_pmic_usb_id->gpio,temp_pmic_usb_id->gpio_name);
	if(status < 0) {
		pr_err("%s(): gpio_request(usb pmic usb idgpio) failed, error code %d\n",
			__func__, status);
		gpio_free(temp_intr_gpio->gpio);
		gpio_free(temp_reset_gpio->gpio);
		gpio_free(temp_mhl_usb_sw_sel->gpio);
		return HAL_RET_FAILURE;
	}
	status = gpio_direction_input(temp_pmic_usb_id->gpio);
	if (status < 0) {
		pr_err("SET GPIO pmic usb id pgio direction failed: %d\n",status);
		gpio_free(temp_intr_gpio->gpio);
		gpio_free(temp_reset_gpio->gpio);
		gpio_free(temp_mhl_usb_sw_sel->gpio);
		gpio_free(temp_pmic_usb_id->gpio);
		return HAL_RET_FAILURE;
	}	
	pr_info("pmic usb id  Gpio Init %s[%d]\n",temp_pmic_usb_id->gpio_name,temp_pmic_usb_id->gpio);	
#endif	
#endif /*<ZORO_MHL>*/


	mhl_ctrl_save=temp_mhl_ctrl;
	return HAL_RET_SUCCESS;
}


//<ZORO_MHL> start
void HalGpioUsbSw(bool mhl)
{
	struct dss_gpio *temp_mhl_usb_sw_sel;	
	/* caused too many line spills */
	pr_info("HalGpioUsbSw %d\n",mhl);
		
	temp_mhl_usb_sw_sel = mhl_ctrl_save->pdata->gpios[MHL_TX_MHL_USB_SEL_GPIO];
					
	gpio_set_value(temp_mhl_usb_sw_sel->gpio, mhl ? USBSW_SEL_MODE_MHL : USBSW_SEL_MODE_USB);
}
//<ZORO_MHL> end


/*****************************************************************************/
/**
 * @brief Release GPIO pins needed by the MHL device.
 *
 *****************************************************************************/
halReturn_t HalGpioTerm(void)
{
	halReturn_t 	halRet;
	struct dss_gpio *temp_reset_gpio, *temp_intr_gpio,*temp_mhl_usb_sw_sel;
	/* caused too many line spills */
	temp_reset_gpio = mhl_ctrl_save->pdata->gpios[MHL_TX_RESET_GPIO];
	temp_intr_gpio = mhl_ctrl_save->pdata->gpios[MHL_TX_INTR_GPIO];
	temp_mhl_usb_sw_sel=mhl_ctrl_save->pdata->gpios[MHL_TX_MHL_USB_SEL_GPIO];
		
	halRet = HalInitCheck();
	if(halRet != HAL_RET_SUCCESS)
	{
		return halRet;
	}

	gpio_free(temp_intr_gpio->gpio);
	gpio_free(temp_reset_gpio->gpio);
	gpio_free(temp_mhl_usb_sw_sel->gpio); //<ZORO_MHL>
#ifdef MAKE_8240_DRIVER //(
    // don't do the stuff in the else branch
#elif defined(MAKE_833X_DRIVER) //)(
    // don't do the stuff in the else branch
#else //)(
	gpio_free(M2U_VBUS_CTRL_M);
#endif //)

	return HAL_RET_SUCCESS;
}


/*****************************************************************************/
/**
 * @brief Platform specific function to control the reset pin of the MHL
 * 		  transmitter device.
 *
 *****************************************************************************/
int HalGpioGetTxIntPin(void)
{
	halReturn_t 	halRet;
	struct dss_gpio *temp_intr_gpio;
	temp_intr_gpio = mhl_ctrl_save->pdata->gpios[MHL_TX_INTR_GPIO];
	
	halRet = HalInitCheck();
	if(halRet != HAL_RET_SUCCESS)
	{
		return -1;
	}

	return gpio_get_value(temp_intr_gpio->gpio);
}

/*****************************************************************************/
/**
 * @brief Platform specific function to read status of usd_id pin of the MHL
 * 		  transmitter device.
 *
 *****************************************************************************/
int HalGpioGetUsbIdPin(void)
{
	halReturn_t 	halRet;
	struct dss_gpio *temp_pmic_usb_id;
	temp_pmic_usb_id = mhl_ctrl_save->pdata->gpios[MHL_TX_PMIC_USB_ID_GPIO];
	
	halRet = HalInitCheck();
	if(halRet != HAL_RET_SUCCESS)
	{
		return -1;
	}

	return gpio_get_value(temp_pmic_usb_id->gpio);
}

/*****************************************************************************/
/**
 * @brief Platform specific function to control the reset pin of the MHL
 * 		  transmitter device.
 *
 *****************************************************************************/
halReturn_t HalGpioSetTxResetPin(bool value)
{
	halReturn_t 	halRet;
	struct dss_gpio *temp_reset_gpio;
	temp_reset_gpio = mhl_ctrl_save->pdata->gpios[MHL_TX_RESET_GPIO];
		
	halRet = HalInitCheck();
	if(halRet != HAL_RET_SUCCESS)
	{
		return halRet;
	}

	gpio_set_value(temp_reset_gpio->gpio, value);
	return HAL_RET_SUCCESS;
}



/*****************************************************************************/
/**
 * @brief Platform specific function to control power on the USB port.
 *
 *****************************************************************************/
#if 0 //<ZORO_MHL>
halReturn_t HalGpioSetUsbVbusPowerPin(bool value)
{
	halReturn_t 	halRet;

	halRet = HalInitCheck();
	if(halRet != HAL_RET_SUCCESS)
	{
		return halRet;
	}

	gpio_set_value(M2U_VBUS_CTRL_M, value);
	return HAL_RET_SUCCESS;
}
#endif //<ZORO_MHL>



/*****************************************************************************/
/**
 * @brief Platform specific function to control Vbus power on the MHL port.
 *
 *****************************************************************************/
halReturn_t HalGpioSetVbusPowerPin(bool powerOn)
{
	halReturn_t 	halRet;

	halRet = HalInitCheck();
	if(halRet != HAL_RET_SUCCESS)
	{
		return halRet;
	}

	SII_DEBUG_PRINT(SII_OSAL_DEBUG_TRACE,
			"HalGpioSetVbusPowerPin called but this function is not implemented yet!\n");

	return HAL_RET_SUCCESS;
}
