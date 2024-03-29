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

/*
   @file si_hdmi_tx_lite_hdcp.h
*/


#include "si_common.h"

#if !defined(__KERNEL__)

#include "hal_timers.h"

#else

#include "sii_hal.h"

#endif

#include "si_mhl_defs.h"
#include "si_mhl_tx_base_drv_api.h"  // generic driver interface to MHL tx component
#include "si_platform.h"
#include "si_cra.h"
#include "si_cra_cfg.h"
#include "si_common_regs.h"
#include "si_hdmi_tx_lite_api.h"
#include "si_drv_hdmi_tx_lite_hdcp.h"

#ifdef ENABLE_HDMI_DEBUG_PRINT //(

#define HDMI_DEBUG_PRINT(x) SII_DEBUG_PRINT(SII_OSAL_DEBUG_HDMI_DBG,x)

#else //)(

#define HDMI_DEBUG_PRINT(x) /* nothing */

#endif //)

#ifdef ENABLE_HDMI_TRACE_PRINT //(

#define HDMI_TRACE_PRINT(x) SII_DEBUG_PRINT(SII_OSAL_DEBUG_HDMI_TRACE,x)

#else //)(

#define HDMI_TRACE_PRINT(x) /* nothing */

#endif //)


#define SIZE_AVI_INFOFRAME				14



void SiiHdmiTxLiteHdmiDrvHpdStatusChange(bool_t connected)
{
	if(connected)
	{
		HDMI_TRACE_PRINT(("SiiHdmiTxLiteHdmiDrvHpdStatusChange\n"));
	}
	else
	{
		// Disable OE and HDCP here
		SiiMhlTxDrvTmdsControl (false);		// enable = false
	}
}

void SiiHdmiTxLiteHdmiDrvTmdsActive(void)
{
}

void SiiHdmiTxLiteHdmiDrvTmdsInactive(void)
{
    // nothing to do here... ...yet
}

void SiiHdmiTxLiteDrvSetAudioMode (void)
{
	// S/PDIF, layout 1, refer to stream header, pass basic audio only
	SiiRegWrite (REG_TPI_CONFIG2, BIT_TPI_AUDIO_HANDLING_DOWNSAMPLE_INCOMING_AS_NEEDED);
	SiiRegWrite (REG_TPI_CONFIG3
	    , BIT_TPI_CONFIG_3_AUDIO_INTERFACE_SPDIF
	    | BIT_TPI_CONFIG3_AUDIO_PACKET_HEADER_LAYOUT_2CH
	    | BIT_TPI_CONFIG3_MUTE_NORMAL
	    | BIT_TPI_AUDIO_CODING_TYPE_STREAM_HEADER
	    );
}

