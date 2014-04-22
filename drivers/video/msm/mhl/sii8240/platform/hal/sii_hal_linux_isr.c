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
 * @file sii_hal_linux_isr.c
 *
 * @brief Linux implementation of interrupt support used by Silicon Image
 *        MHL devices.
 *
 * $Author: Dave Canfield
 * $Rev: $
 * $Date: Jan. 31, 2011
 *
 *****************************************************************************/

#define SII_HAL_LINUX_ISR_C

/***** #include statements ***************************************************/
#include "sii_hal.h"
#include "sii_hal_priv.h"
#include "si_c99support.h"
#include "si_osdebug.h"

#if defined(__KERNEL__)
#if defined(DEBUG)
#define DEBUG_PRINT(str, args...) printk("sii_hal_linux_isr.c:%d, "str, __LINE__, args)
#else
#define DEBUG_PRINT(str, args...)
#endif
#else
#define DEBUG_PRINT(str, args...)
#endif

/***** local macro definitions ***********************************************/

/***** local type definitions ************************************************/

/***** local variable declarations *******************************************/

/***** local function prototypes *********************************************/

/***** global variable declarations *******************************************/


/***** local functions *******************************************************/

/*****************************************************************************/
/*
 *  @brief Interrupt handler for MHL transmitter interrupts.
 *
 *  @param[in]		irq		The number of the asserted IRQ line that caused
 *  						this handler to be called.
 *  @param[in]		data	Data pointer passed when the interrupt was enabled,
 *  						which in this case is a pointer to the
 *  						MhlDeviceContext of the I2c device.
 *
 *  @return     Always returns IRQ_HANDLED.
 *
 *****************************************************************************/
static irqreturn_t HalThreadedIrqHandler(int irq, void *data)
{
	pMhlDeviceContext	pMhlDevContext = (pMhlDeviceContext)data;

	if (HalAcquireIsrLock() == HAL_RET_SUCCESS) {
		(pMhlDevContext->irqHandler)();
		HalReleaseIsrLock();
	}
	else
	{
		DEBUG_PRINT("HalThreadedIrqHandler %s\n", "could not acquire lock");
	}

	return IRQ_HANDLED;
}



/***** public functions ******************************************************/


/*****************************************************************************/
/**
 * @brief Install IRQ handler.
 *
 *****************************************************************************/
halReturn_t HalInstallIrqHandler(fwIrqHandler_t irqHandler)
{
	int				retStatus;
	halReturn_t 	halRet;

	DEBUG_PRINT("InstallIrqHandler: Enter %s \n", " ");

	if(irqHandler == NULL)
	{
		DEBUG_PRINT("HalInstallIrqHandler: irqHandler cannot be NULL! %s\n", "");
		return HAL_RET_PARAMETER_ERROR;
	}

	halRet = I2cAccessCheck();
	if (halRet != HAL_RET_SUCCESS)
	{
		return halRet;
	}

	if(gMhlDevice.pI2cClient->irq == 0)
	{
		DEBUG_PRINT("HalInstallIrqHandler: No IRQ assigned to I2C device!%s\n", "");
		return HAL_RET_FAILURE;
	}

	gMhlDevice.irqHandler = irqHandler;

	retStatus = request_threaded_irq(gMhlDevice.pI2cClient->irq, NULL,
									 HalThreadedIrqHandler,
									 IRQF_TRIGGER_LOW | IRQF_ONESHOT,
									 gMhlI2cIdTable[0].name,
									 &gMhlDevice);
	if(retStatus != 0)
	{
		DEBUG_PRINT("HalInstallIrqHandler: request_threaded_irq failed, status: %d\n", retStatus);
		gMhlDevice.irqHandler = NULL;
		return HAL_RET_FAILURE;
	}

	return HAL_RET_SUCCESS;
}



/*****************************************************************************/
/**
 * @brief Remove IRQ handler.
 *
 *****************************************************************************/
halReturn_t HalRemoveIrqHandler(void)
{
	halReturn_t 	halRet;

	halRet = I2cAccessCheck();
	if (halRet != HAL_RET_SUCCESS)
	{
		return halRet;
	}

	if(gMhlDevice.irqHandler == NULL)
	{
		DEBUG_PRINT("HalRemoveIrqHandler: no irqHandler installed!%s\n", "");
		return HAL_RET_FAILURE;
	}

	free_irq(gMhlDevice.pI2cClient->irq, &gMhlDevice);

	gMhlDevice.irqHandler = NULL;

	return HAL_RET_SUCCESS;
}
