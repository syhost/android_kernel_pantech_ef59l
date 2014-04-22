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
 * @file mhl_linuxdrv_main.c
 *
 * @brief Main entry point of the Linux driver for Silicon Image MHL transmitters.
 *
 * $Author: Dave Canfield
 * $Rev: $
 * $Date: Jan. 20, 2011
 *
 *****************************************************************************/

#define MHL_LINUXDRV_MAIN_C

/***** #include statements ***************************************************/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include "si_common.h"
#include "si_mhl_defs.h"
#include "mhl_linuxdrv.h"
#include "osal/include/osal.h"
#include "si_mhl_tx_api.h"
#include "si_drvisrconfig.h"
#include "si_cra.h"
#include "si_osdebug.h"
#include "si_hdmi_tx_lite_api.h"
#include "../../../../mdss/mhl_sii8240.h"	// NAMI
#include "../../platform/hal/sii_hal_priv.h"	//NAMI
#include "si_app_gpio.h"

//#include <linux/shmhl_kerl.h>
#include "si_c99support.h"
#include "sii_hal.h"
#include "si_drv_mhl_tx.h"

/***** local macro definitions ***********************************************/


/***** local variable declarations *******************************************/
static int32_t devMajor = 0;    /**< default device major number */
static struct cdev siiMhlCdev;
static struct class *siiMhlClass;
static char *buildTime = "Build: " __DATE__"-" __TIME__ "\n";
static char *buildVersion = "1.00.";

//static struct device *siiMhlClassDevice;


/***** global variable declarations *******************************************/

MHL_DRIVER_CONTEXT_T gDriverContext;

#if defined(DEBUG)
unsigned char DebugChannelMasks[SII_OSAL_DEBUG_NUM_CHANNELS/8+1]=
{0xFF,0xFF,0xFF,0xFF
,0xFF,0xFF,0xFF,0xFF
,0xFF,0xFF,0xFF,0xFF
,0xFF,0xFF
};
//ulong DebugChannelMask = 0xFFFFFFFF;
module_param_array(DebugChannelMasks, byte, NULL, S_IRUGO | S_IWUSR);

ushort DebugFormat = SII_OS_DEBUG_FORMAT_FILEINFO;
module_param(DebugFormat, ushort, S_IRUGO | S_IWUSR);
#endif

spinlock_t mhl_connection_flag_lock;	//<ZORO_MHL>

/*<ZORO_MHL> start*/
static unsigned long _ftm_mode;
static int _ftm_detect_intr;
extern unsigned int pp_enable;

extern int ftm_mode_enter(void);
extern void ftm_mode_leave(void);

extern void shmhl_init(void);
extern void shmhl_rcp_input_register_device(void);
extern void shmhl_rcp_input_unregister_device(void);

extern  struct completion rgnd_done;

ssize_t set_ftm_mode(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t count)
{
	unsigned long	in;

	if(strict_strtoul(buf, 0, &in)) {
		return -EINVAL;
	}

	if (in == 1 && _ftm_mode == 0) {
		_ftm_detect_intr = ftm_mode_enter();
		_ftm_mode = 1;
	}
	else if (in == 0 && _ftm_mode == 1) {
		ftm_mode_leave();
		_ftm_mode = 0;
		_ftm_detect_intr = 0;
	}

	return 1;
}

ssize_t show_ftm_mode(struct device *dev, struct device_attribute *attr,
							char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%lu", _ftm_mode);
}

ssize_t show_ftm_detect_intr(struct device *dev, struct device_attribute *attr,
							char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%d", _ftm_detect_intr);
}

ssize_t set_pp_enable(struct device *dev, struct device_attribute *attr,
					const char *buf, size_t count)
{
	unsigned long   ppflag;

	if(strict_strtoul(buf, 0, &ppflag)) {
		return -EINVAL;
	}

	if (ppflag == 0) {
		pp_enable = 0;
	} else {
		pp_enable = 1;
	}
	pr_err("packed pixel enable: %u", pp_enable);

	return count;
}

ssize_t show_pp_enable(struct device *dev, struct device_attribute *attr,
							char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%u", pp_enable);
}
/*<ZORO_MHL> end*/

/***** local functions *******************************************************/

/**
 *  @brief Start the MHL transmitter device
 *
 *  This function is called during driver startup to initialize control of the
 *  MHL transmitter device by the driver.
 *
 *  @return     0 if successful, negative error code otherwise
 *
 *****************************************************************************/
int32_t StartMhlTxDevice(struct mhl_tx_ctrl *mhl_ctrl)
{
	halReturn_t		halStatus;
	SiiOsStatus_t	osalStatus;


    pr_info("Starting %s\n", MHL_PART_NAME);

    // Initialize the Common Register Access (CRA) layer.
    if(!SiiCraInitialize())
    {
    	SII_DEBUG_PRINT(SII_OSAL_DEBUG_TRACE, "Initialization of CRA layer failed!\n");
    	return -EIO;
    }

    SII_DEBUG_PRINT(SII_OSAL_DEBUG_TRACE, "[MHL] SiiCraInitalize OK\n");

    // Initialize the OS Abstraction Layer (OSAL) support.
    osalStatus = SiiOsInit(0);
    if (osalStatus != SII_OS_STATUS_SUCCESS)
    {
    	SII_DEBUG_PRINT(SII_OSAL_DEBUG_TRACE,
    			"Initialization of OSAL failed, error code: %d\n",osalStatus);
    	return -EIO;
    }

    SII_DEBUG_PRINT(SII_OSAL_DEBUG_TRACE, "[MHL] SiiOsInit OK\n");

    halStatus = HalInit(mhl_ctrl); 	///HalGpioInit was called 
    if (halStatus != HAL_RET_SUCCESS)
    {
    	SII_DEBUG_PRINT(SII_OSAL_DEBUG_TRACE,
    			"Initialization of HAL failed, error code: %d\n",halStatus);
    	SiiOsTerm();
    	return -EIO;
    }
    SII_DEBUG_PRINT(SII_OSAL_DEBUG_TRACE, "[MHL] HalInit OK\n");
#if 0
    ////halStatus = HalOpenI2cDevice(MHL_PART_NAME, MHL_DRIVER_NAME);
#else
    /// NAMI	
    if(mhl_ctrl->i2c_handle)
	    gMhlDevice.pI2cClient=mhl_ctrl->i2c_handle;
    else
	    halStatus=HAL_RET_FAILURE;
#endif
    halStatus = HalOpenI2cDevice(MHL_PART_NAME, MHL_DRIVER_NAME);
    if (halStatus != HAL_RET_SUCCESS)
    {
    	SII_DEBUG_PRINT(SII_OSAL_DEBUG_TRACE,
    			"Opening of I2c device %s failed, error code: %d\n",
    			MHL_PART_NAME, halStatus);
    	HalTerm();
    	SiiOsTerm();
    	return -EIO;
    }
    SII_DEBUG_PRINT(SII_OSAL_DEBUG_TRACE, "[MHL] HalOpenI2cDevice OK\n");

    halStatus = HalInstallIrqHandler(SiiMhlTxDeviceIsr);
    if (halStatus != HAL_RET_SUCCESS)
    {
    	SII_DEBUG_PRINT(SII_OSAL_DEBUG_TRACE,
    			"Initialization of HAL interrupt support failed, error code: %d\n",
    			halStatus);
    	HalCloseI2cDevice();
    	HalTerm();
    	SiiOsTerm();
    	return -EIO;
    }

    /* Initialize the MHL Tx code a polling interval of 30ms. */
	HalAcquireIsrLock();
	SiiMhlTxInitialize(EVENT_POLL_INTERVAL_30_MS);
    HalReleaseIsrLock();

    return 0;
}



/**
 *  @brief Stop the MHL transmitter device
 *
 *  This function shuts down control of the transmitter device so that
 *  the driver can exit
 *
 *  @return     0 if successful, negative error code otherwise
 *
 *****************************************************************************/
int32_t StopMhlTxDevice(void)
{
	halReturn_t		halStatus;

	pr_info("Stopping %s\n", MHL_PART_NAME);

	HalRemoveIrqHandler();

	halStatus = HalCloseI2cDevice();
    if (halStatus != HAL_RET_SUCCESS)
    {
    	SII_DEBUG_PRINT(SII_OSAL_DEBUG_TRACE,
    			"Closing of I2c device failed, error code: %d\n",halStatus);
    	return -EIO;
    }

	halStatus = HalTerm();
    if (halStatus != HAL_RET_SUCCESS)
    {
    	SII_DEBUG_PRINT(SII_OSAL_DEBUG_TRACE,
    			"Termination of HAL failed, error code: %d\n",halStatus);
    	return -EIO;
    }

	SiiOsTerm();
	return 0;
}




/***** public functions ******************************************************/

/**
 * @brief Handle read request to the connection_state attribute file.
 */
ssize_t ShowConnectionState(struct device *dev, struct device_attribute *attr,
							char *buf)
{
	if(gDriverContext.flags & MHL_STATE_FLAG_CONNECTED) {
		return scnprintf(buf, PAGE_SIZE, "connected %s_ready",
				gDriverContext.flags & MHL_STATE_FLAG_RCP_READY? "rcp" : "not_rcp");
	} else {
		return scnprintf(buf, PAGE_SIZE, "not connected");
	}
}


/**
 * @brief Handle read request to the rcp_keycode attribute file.
 */
ssize_t ShowRcp(struct device *dev, struct device_attribute *attr,
							char *buf)
{
	int		status = 0;

	if(HalAcquireIsrLock() != HAL_RET_SUCCESS)
	{
		return -ERESTARTSYS;
	}

	if(gDriverContext.flags &
		(MHL_STATE_FLAG_RCP_SENT | MHL_STATE_FLAG_RCP_RECEIVED))
	{
		status = scnprintf(buf, PAGE_SIZE, "0x%02x %s",
				gDriverContext.keyCode,
				gDriverContext.flags & MHL_STATE_FLAG_RCP_SENT? "sent" : "received");
	}

	HalReleaseIsrLock();
	return status;
}



/**
 * @brief Handle write request to the rcp_keycode attribute file.
 */
ssize_t SendRcp(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t count)
{
	unsigned long	keyCode;
	int				status = -EINVAL;

	SII_DEBUG_PRINT(SII_OSAL_DEBUG_TRACE, "SendRcp received string: ""%s""\n", buf);

	if(HalAcquireIsrLock() != HAL_RET_SUCCESS)
	{
		return -ERESTARTSYS;
	}

	while(gDriverContext.flags & MHL_STATE_FLAG_RCP_READY) {

		if(strict_strtoul(buf, 0, &keyCode)) {
			SII_DEBUG_PRINT(SII_OSAL_DEBUG_TRACE, "Unable to convert keycode string\n");
			break;
		}

		if(keyCode >= 0xFE) {
			SII_DEBUG_PRINT(SII_OSAL_DEBUG_TRACE,
					"keycode (0x%x) is too large to be valid\n", keyCode);
			break;
		}

		gDriverContext.flags &= ~(MHL_STATE_FLAG_RCP_RECEIVED |
								  MHL_STATE_FLAG_RCP_ACK |
								  MHL_STATE_FLAG_RCP_NAK);
		gDriverContext.flags |= MHL_STATE_FLAG_RCP_SENT;
		gDriverContext.keyCode = (uint8_t)keyCode;
		SiiMhlTxRcpSend((uint8_t)keyCode);
		status = count;
		break;
	}

	HalReleaseIsrLock();

	return status;
}


/**
 * @brief Handle write request to the rcp_ack attribute file.
 *
 * This file is used to send either an ACK or NAK for a received
 * Remote Control Protocol (RCP) key code.
 *
 * The format of the string in buf must be:
 * 	"keycode=<keyvalue> errorcode=<errorvalue>
 * 	where:	<keyvalue>		is replaced with value of the RCP to be ACK'd or NAK'd
 * 			<errorvalue>	0 if the RCP key code is to be ACK'd
 * 							non-zero error code if the RCP key code is to be NAK'd
 */
ssize_t SendRcpAck(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t count)
{
	unsigned long	keyCode = 0x100;	// initialize with invalid values
	unsigned long	errCode = 0x100;
	char			*pStr;
	int				status = -EINVAL;

	SII_DEBUG_PRINT(SII_OSAL_DEBUG_TRACE, "SendRcpAck received string: ""%s""\n", buf);

	// Parse the input string and extract the RCP key code and error code
	do {
		pStr = strstr(buf, "keycode=");
		if(pStr != NULL) {
			if(strict_strtoul(pStr + 8, 0, &keyCode)) {
				SII_DEBUG_PRINT(SII_OSAL_DEBUG_TRACE, "Unable to convert keycode string\n");
				break;
			}
		} else {
			SII_DEBUG_PRINT(SII_OSAL_DEBUG_TRACE, "Invalid string format, can't "\
							"find ""keycode"" value\n");
			break;
		}

		pStr = strstr(buf, "errorcode=");
		if(pStr != NULL) {
			if(strict_strtoul(pStr + 10, 0, &errCode)) {
				SII_DEBUG_PRINT(SII_OSAL_DEBUG_TRACE, "Unable to convert errorcode string\n");
				break;
			}
		} else {
			SII_DEBUG_PRINT(SII_OSAL_DEBUG_TRACE, "Invalid string format, can't "\
							"find ""errorcode"" value\n");
			break;
		}
	} while(false);

	if((keyCode > 0xFF) || (errCode > 0xFF)) {
		SII_DEBUG_PRINT(SII_OSAL_DEBUG_TRACE, "Invalid key code or error code "\
						"specified, key code: 0x%02x  error code: 0x%02x\n",
						keyCode, errCode);
		return status;
	}

	if(HalAcquireIsrLock() != HAL_RET_SUCCESS)
	{
		return -ERESTARTSYS;
	}

	while(gDriverContext.flags & MHL_STATE_FLAG_RCP_READY) {

		if((keyCode != gDriverContext.keyCode)
			|| !(gDriverContext.flags & MHL_STATE_FLAG_RCP_RECEIVED)) {

			SII_DEBUG_PRINT(SII_OSAL_DEBUG_TRACE,
					"Attempting to ACK a key code that was not received!\n");
			break;
		}

		if(errCode == 0) {
			SiiMhlTxRcpkSend((uint8_t)keyCode);
		} else {
			SiiMhlTxRcpeSend((uint8_t)errCode);
		}

		status = count;
		break;
	}

	HalReleaseIsrLock();

	return status;
}



/**
 * @brief Handle read request to the rcp_ack attribute file.
 *
 * Reads from this file return a string detailing the last RCP
 * ACK or NAK received by the driver.
 *
 * The format of the string returned in buf is:
 * 	"keycode=<keyvalue> errorcode=<errorvalue>
 * 	where:	<keyvalue>		is replaced with value of the RCP key code for which
 * 							an ACK or NAK has been received.
 * 			<errorvalue>	0 if the last RCP key code was ACK'd or
 * 							non-zero error code if the RCP key code was NAK'd
 */
ssize_t ShowRcpAck(struct device *dev, struct device_attribute *attr,
					char *buf)
{
	int				status = -EINVAL;

	SII_DEBUG_PRINT(SII_OSAL_DEBUG_TRACE, "ShowRcpAck called\n");

	if(HalAcquireIsrLock() != HAL_RET_SUCCESS)
	{
		return -ERESTARTSYS;
	}

	if(gDriverContext.flags & (MHL_STATE_FLAG_RCP_ACK | MHL_STATE_FLAG_RCP_NAK)) {

		status = scnprintf(buf, PAGE_SIZE, "keycode=0x%02x errorcode=0x%02x",
				gDriverContext.keyCode, gDriverContext.errCode);
	}

	HalReleaseIsrLock();

	return status;
}



/**
 * @brief Handle write request to the devcap attribute file.
 *
 * Writes to the devcap file are done to set the offset of a particular
 * Device Capabilities register to be returned by a subsequent read
 * from this file.
 *
 * All we need to do is validate the specified offset and if valid
 * save it for later use.
 */
ssize_t SelectDevCap(struct device *dev, struct device_attribute *attr,
					 const char *buf, size_t count)
{
	unsigned long	devCapOffset;
	int				status = -EINVAL;

	SII_DEBUG_PRINT(SII_OSAL_DEBUG_TRACE, "SelectDevCap received string: ""%s""\n", buf);

	do {

		if(strict_strtoul(buf, 0, &devCapOffset)) {
			SII_DEBUG_PRINT(SII_OSAL_DEBUG_TRACE,
							"Unable to convert register offset string\n");
			break;
		}

		if(devCapOffset >= 0x0F) {
			SII_DEBUG_PRINT(SII_OSAL_DEBUG_TRACE,
					"dev cap offset (0x%x) is too large to be valid\n",
					devCapOffset);
			break;
		}

		gDriverContext.devCapOffset = (uint8_t)devCapOffset;

		status = count;

	} while(false);

	return status;
}



/**
 * @brief Handle read request to the devcap attribute file.
 *
 * Reads from this file return the hexadecimal string value of the last
 * Device Capability register offset written to this file.
 *
 * The return value is the number characters written to buf, or EAGAIN
 * if the driver is busy and cannot service the read request immediately.
 * If EAGAIN is returned the caller should wait a little and retry the
 * read.
 *
 * The format of the string returned in buf is:
 * 	"offset:<offset>=<regvalue>
 * 	where:	<offset>	is the last Device Capability register offset
 * 						written to this file
 * 			<regvalue>	the currentl value of the Device Capability register
 * 						specified in offset
 */
ssize_t ReadDevCap(struct device *dev, struct device_attribute *attr,
					char *buf)
{
	uint8_t		regValue;
	int			status = -EINVAL;

	SII_DEBUG_PRINT(SII_OSAL_DEBUG_TRACE, "ReadDevCap called\n");

	if(HalAcquireIsrLock() != HAL_RET_SUCCESS)
	{
		return -ERESTARTSYS;
	}

	do {
		if(gDriverContext.flags & MHL_STATE_FLAG_CONNECTED) {

			status = SiiTxGetPeerDevCapEntry(gDriverContext.devCapOffset,
											 &regValue);
			if(status != 0) {
				// Driver is busy and cannot provide the requested DEVCAP
				// register value right now so inform caller they need to
				// try again later.
				status = -EAGAIN;
				break;
			}
			status = scnprintf(buf, PAGE_SIZE, "offset:0x%02x=0x%02x",
								gDriverContext.devCapOffset, regValue);
		}
	} while(false);

	HalReleaseIsrLock();

	return status;
}

#define MAX_EVENT_STRING_LEN 40
/*****************************************************************************/
/**
 * @brief Handler for MHL hot plug detect status change notifications
 *  from the MhlTx layer.
 *
 *****************************************************************************/
void  AppNotifyMhlDownStreamHPDStatusChange(bool_t connected)
{
	char	event_string[MAX_EVENT_STRING_LEN];
	char	*envp[] = {event_string, NULL};


	SII_DEBUG_PRINT(SII_OSAL_DEBUG_TRACE,
			"AppNotifyMhlDownStreamHPDStatusChange called, "\
			"HPD status is: %s\n", connected? "CONNECTED" : "NOT CONNECTED");

	snprintf(event_string, MAX_EVENT_STRING_LEN, "MHLEVENT=%s",
			connected? "HPD" : "NO_HPD");
	kobject_uevent_env(&gDriverContext.pDevice->kobj,
						KOBJ_CHANGE, envp);
}



/*****************************************************************************/
/**
 * @brief Handler for most of the event notifications from the MhlTx layer.
 *
 *****************************************************************************/
MhlTxNotifyEventsStatus_e  AppNotifyMhlEvent(uint8_t eventCode, uint8_t eventParam,void *pRefData)
{
	char	event_string[MAX_EVENT_STRING_LEN];
	char	*envp[] = {event_string, NULL};
	MhlTxNotifyEventsStatus_e rtnStatus = MHL_TX_EVENT_STATUS_PASSTHROUGH;
	SII_DEBUG_PRINT(SII_OSAL_DEBUG_TRACE,
			"AppNotifyMhlEvent called, eventCode: 0x%02x eventParam: 0x%02x\n",
			eventCode, eventParam);

	// Save the info on the most recent event.  This is done to support the
	// SII_MHL_GET_MHL_TX_EVENT IOCTL.  If at some point in the future the
	// driver's IOCTL interface is abandoned in favor of using sysfs attributes
	// this can be removed.
	gDriverContext.pendingEvent = eventCode;
	gDriverContext.pendingEventData = eventParam;

	switch(eventCode) {

		case MHL_TX_EVENT_CONNECTION:
			gDriverContext.flags |= MHL_STATE_FLAG_CONNECTED;
			strncpy(event_string, "MHLEVENT=connected", MAX_EVENT_STRING_LEN);
			kobject_uevent_env(&gDriverContext.pDevice->kobj,
								KOBJ_CHANGE, envp);
#ifdef	BYPASS_VBUS_HW_SUPPORT //(
        	// turn off VBUS power here
#endif //)
			break;

		case MHL_TX_EVENT_RCP_READY:
			gDriverContext.flags |= MHL_STATE_FLAG_RCP_READY;
			strncpy(event_string, "MHLEVENT=rcp_ready", MAX_EVENT_STRING_LEN);
			kobject_uevent_env(&gDriverContext.pDevice->kobj,
								KOBJ_CHANGE, envp);
			break;

		case MHL_TX_EVENT_DISCONNECTION:
			gDriverContext.flags = 0;
			gDriverContext.keyCode = 0;
			gDriverContext.errCode = 0;
			strncpy(event_string, "MHLEVENT=disconnected", MAX_EVENT_STRING_LEN);
			kobject_uevent_env(&gDriverContext.pDevice->kobj,
								KOBJ_CHANGE, envp);
			break;

		case MHL_TX_EVENT_RCP_RECEIVED:
			gDriverContext.flags &= ~MHL_STATE_FLAG_RCP_SENT;
			gDriverContext.flags |= MHL_STATE_FLAG_RCP_RECEIVED;
			gDriverContext.keyCode = eventParam;
			snprintf(event_string, MAX_EVENT_STRING_LEN,
					"MHLEVENT=received_RCP key code=0x%02x", eventParam);
			kobject_uevent_env(&gDriverContext.pDevice->kobj,
								KOBJ_CHANGE, envp);
			break;

		case MHL_TX_EVENT_RCPK_RECEIVED:
			if((gDriverContext.flags & MHL_STATE_FLAG_RCP_SENT)
				&& (gDriverContext.keyCode == eventParam)) {

				gDriverContext.flags |= MHL_STATE_FLAG_RCP_ACK;

				SII_DEBUG_PRINT(SII_OSAL_DEBUG_TRACE,
						"Generating RCPK received event, keycode: 0x%02x\n",
						eventParam);
				snprintf(event_string, MAX_EVENT_STRING_LEN,
						"MHLEVENT=received_RCPK key code=0x%02x", eventParam);
				kobject_uevent_env(&gDriverContext.pDevice->kobj,
									KOBJ_CHANGE, envp);
			} else {
				SII_DEBUG_PRINT(SII_OSAL_DEBUG_TRACE,
						"Ignoring unexpected RCPK received event, keycode: 0x%02x\n",
						eventParam);
			}
			break;

		case MHL_TX_EVENT_RCPE_RECEIVED:
			if(gDriverContext.flags & MHL_STATE_FLAG_RCP_SENT) {

				gDriverContext.errCode = eventParam;
				gDriverContext.flags |= MHL_STATE_FLAG_RCP_NAK;

				SII_DEBUG_PRINT(SII_OSAL_DEBUG_TRACE,
						"Generating RCPE received event, error code: 0x%02x\n",
						eventParam);
				snprintf(event_string, MAX_EVENT_STRING_LEN,
						"MHLEVENT=received_RCPE error code=0x%02x", eventParam);
				kobject_uevent_env(&gDriverContext.pDevice->kobj,
									KOBJ_CHANGE, envp);
			} else {
				SII_DEBUG_PRINT(SII_OSAL_DEBUG_TRACE,
						"Ignoring unexpected RCPE received event, error code: 0x%02x\n",
						eventParam);
			}
			break;

		case MHL_TX_EVENT_DCAP_CHG:
			snprintf(event_string, MAX_EVENT_STRING_LEN,
					"MHLEVENT=DEVCAP change");
			kobject_uevent_env(&gDriverContext.pDevice->kobj,
								KOBJ_CHANGE, envp);

			break;

		case MHL_TX_EVENT_DSCR_CHG:	// Scratch Pad registers have changed
			snprintf(event_string, MAX_EVENT_STRING_LEN,
					"MHLEVENT=SCRATCHPAD change");
			kobject_uevent_env(&gDriverContext.pDevice->kobj,
								KOBJ_CHANGE, envp);
			break;

		case MHL_TX_EVENT_POW_BIT_CHG:	// Peer's power capability has changed
			if (eventParam)
			{
#ifdef BYPASS_VBUS_HW_SUPPORT //(
				// Since downstream device is supplying VBUS power we should
				// turn off our VBUS power here.  If the platform application
				// can control VBUS power it should turn off it's VBUS power
				// now and return status of MHL_TX_EVENT_STATUS_HANDLED.  If
				// platform cannot control VBUS power it should return
				// MHL_TX_EVENT_STATUS_PASSTHROUGH to allow the MHL layer to
				// try to turn it off.
				rtnStatus = MHL_TX_EVENT_STATUS_HANDLED;
#else //)(
				// In this sample driver all that is done is to report an
				// event describing the requested state of VBUS power and
				// return MHL_TX_EVENT_STATUS_PASSTHROUGH to allow lower driver
				// layers to control VBUS power if possible.
#endif //)
				snprintf(event_string, MAX_EVENT_STRING_LEN,
						"MHLEVENT=MHL VBUS power OFF");
			}
			else
			{
				snprintf(event_string, MAX_EVENT_STRING_LEN,
						"MHLEVENT=MHL VBUS power ON");
#ifdef BYPASS_VBUS_HW_SUPPORT //(
				rtnStatus = MHL_TX_EVENT_STATUS_HANDLED;
#else //)(
#endif //)
			}

			kobject_uevent_env(&gDriverContext.pDevice->kobj,
								KOBJ_CHANGE, envp);
			break;
		case MHL_TX_EVENT_RGND_MHL:
#ifdef BYPASS_VBUS_HW_SUPPORT //(
			// RGND measurement has determine that the peer is an MHL device.
			// If platform application can determine that the attached device
			// is not supplying VBUS power it should turn on it's VBUS power
			// here and return MHL_TX_EVENT_STATUS_HANDLED to indicate to
			// indicate to the caller that it handled the notification.
			rtnStatus = MHL_TX_EVENT_STATUS_HANDLED;
#else //)(
			// In this sample driver all that is done is to report the event
			// and return MHL_TX_EVENT_STATUS_PASSTHROUGH to allow lower driver
			// layers to control VBUS power if possible.
#endif //)
			snprintf(event_string, MAX_EVENT_STRING_LEN,
					"MHLEVENT=MHL device detected");
			kobject_uevent_env(&gDriverContext.pDevice->kobj,
								KOBJ_CHANGE, envp);
			break;
		case MHL_TX_EVENT_TMDS_ENABLED:
		    break;
		case MHL_TX_EVENT_TMDS_DISABLED:
		    break;

		case MHL_TX_EVENT_INFO_FRAME_CHANGE:
			if (!pinTranscodeMode)
			{
				MhlTxLiteSetInfoFrame(eventParam,pRefData);
			}
			break;
		/*<ZORO_MHL>start*/
		case MHL_TX_EVENT_HDCP_PASS:
			snprintf(event_string, MAX_EVENT_STRING_LEN,
					"MHLEVENT=MHL HDCP PASS");
			kobject_uevent_env(&gDriverContext.pDevice->kobj,
								KOBJ_CHANGE, envp);
			break;
		/*<ZORO_MHL>end*/
		default:
			SII_DEBUG_PRINT(SII_OSAL_DEBUG_TRACE,
					"AppNotifyEvent called with unrecognized event code!\n");
	}
	return rtnStatus;
}

/*****************************************************************************/
/**
 * @brief Handler for MHL transmitter reset requests.
 *
 * This function is called by the MHL layer to request that the MHL transmitter
 * chip be reset.  Since the MHL layer is platform agnostic and therefore doesn't
 * know how to control the transmitter's reset pin each platform application is
 * required to implement this function to perform the requested reset operation.
 *
 * @param[in]	hwResetPeriod	Time in ms. that the reset pin is to be asserted.
 * @param[in]	hwResetDelay	Time in ms. to wait after reset pin is released.
 *
 *****************************************************************************/
void AppResetMhlTx(uint16_t hwResetPeriod,uint16_t hwResetDelay)
{

	// Reset the TX chip,
	HalGpioSetTxResetPin(LOW);
	HalTimerWait(hwResetPeriod);
	HalGpioSetTxResetPin(HIGH);

	HalTimerWait(hwResetDelay);
}

/**
 *  File operations supported by the MHL driver
 */
static const struct file_operations siiMhlFops = {
    .owner			= THIS_MODULE,
    .open			= SiiMhlOpen,
    .release		= SiiMhlRelease,
    .unlocked_ioctl	= SiiMhlIoctl
};


/*
 * Sysfs attribute files supported by this driver.
 */
struct device_attribute driver_attribs[] = {
		__ATTR(connection_state, 0444, ShowConnectionState, NULL),
		__ATTR(rcp_keycode, 0664, ShowRcp, SendRcp),
		__ATTR(rcp_ack, 0664, ShowRcpAck, SendRcpAck),
		__ATTR(devcap, 0664, ReadDevCap, SelectDevCap),
		__ATTR(ftm_mode, 0664, show_ftm_mode, set_ftm_mode),	//<ZORO_MHL>
		__ATTR(ftm_detect_intr, 0444, show_ftm_detect_intr, NULL), //<ZORO_MHL>
		__ATTR(pp, 0664, show_pp_enable, set_pp_enable), //<ZORO_MHL>
		__ATTR_NULL
};



int  SiiMhlInit(struct mhl_tx_ctrl *mhl_ctrl)
{
    int32_t	ret;
    dev_t	devno;

	pr_info("%s driver starting!\n", MHL_DRIVER_NAME);
	pr_info("Version: %s%d\n", buildVersion,BUILDNUM);
	pr_info("%s", buildTime);

    /* register chrdev */
    pr_info("register_chrdev %s\n", MHL_DRIVER_NAME);

	spin_lock_init(&mhl_connection_flag_lock);

    /* If a major device number has already been selected use it,
     * otherwise dynamically allocate one.
     */
    if (devMajor) {
        devno = MKDEV(devMajor, 0);
        ret = register_chrdev_region(devno, MHL_DRIVER_MINOR_MAX,
                MHL_DRIVER_NAME);
    } else {
        ret = alloc_chrdev_region(&devno,
                        0, MHL_DRIVER_MINOR_MAX,
                        MHL_DRIVER_NAME);
        devMajor = MAJOR(devno);
    }
    if (ret) {
    	pr_info("register_chrdev %d, %s failed, error code: %d\n",
    					devMajor, MHL_DRIVER_NAME, ret);
        return ret;
    }

    cdev_init(&siiMhlCdev, &siiMhlFops);
    siiMhlCdev.owner = THIS_MODULE;
    ret = cdev_add(&siiMhlCdev, devno, MHL_DRIVER_MINOR_MAX);
    if (ret) {
    	pr_info("cdev_add %s failed %d\n", MHL_DRIVER_NAME, ret);
        goto free_chrdev;
    }

    siiMhlClass = class_create(THIS_MODULE, "mhl");
    if (IS_ERR(siiMhlClass)) {
    	pr_info("class_create failed %d\n", ret);
        ret = PTR_ERR(siiMhlClass);
        goto free_cdev;
    }

    siiMhlClass->dev_attrs = driver_attribs;

    gDriverContext.pDevice  = device_create(siiMhlClass, NULL,
    									 MKDEV(devMajor, 0),  NULL,
    									 "%s", MHL_DEVICE_NAME);
    if (IS_ERR(gDriverContext.pDevice)) {
    	pr_info("class_device_create failed %s %d\n", MHL_DEVICE_NAME, ret);
        ret = PTR_ERR(gDriverContext.pDevice);
        goto free_class;
    }

    shmhl_init(); /*<ZORO_MHL>*/
    shmhl_rcp_input_register_device(); /* <ZORO_MHL> */

    init_completion(&rgnd_done); //<ZORO_MHL>

    ret = StartMhlTxDevice(mhl_ctrl);
    if(ret == 0) {
    	return 0;

    } else {
    	// Transmitter startup failed so fail the driver load.
    	device_destroy(siiMhlClass, MKDEV(devMajor, 0));
        shmhl_rcp_input_unregister_device(); /*<ZORO_MHL>*/
    }


free_class:
	class_destroy(siiMhlClass);

free_cdev:
	cdev_del(&siiMhlCdev);

free_chrdev:
	unregister_chrdev_region(MKDEV(devMajor, 0), MHL_DRIVER_MINOR_MAX);

	return ret;
}

#if 0

static void __exit SiiMhlExit(void)
{
	pr_info("%s driver exiting!\n", MHL_DRIVER_NAME);

	StopMhlTxDevice();

	device_destroy(siiMhlClass, MKDEV(devMajor, 0));
    class_destroy(siiMhlClass);
    unregister_chrdev_region(MKDEV(devMajor, 0), MHL_DRIVER_MINOR_MAX);
    shmhl_rcp_input_unregister_device(); /*<ZORO_MHL>*/
}

module_init(SiiMhlInit);
module_exit(SiiMhlExit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Silicon Image <http://www.siliconimage.com>");
MODULE_DESCRIPTION(MHL_DRIVER_DESC);
#endif