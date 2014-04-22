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

#define BUILD_NUMBER	62
#define BUILD_STRING	""
//#define BUILD_STRING	"PP Mode for Ops to test"

#define BUILD_CONFIG 1  /* CP8240 Starter Kit  -- see si_app_gpio.h */
//#define BUILD_CONFIG 2  /* CP8558 FPGA  -- see si_app_gpio.h */
#define BISHOP_DEVICE_ID 0x8558
#define WOLV60_DEVICE_ID 0x8240

#define ONE_EDID_BLOCK_PER_REQUEST
#define PARSE_EDID_MHL_3D
#define MHL2_0_SUPPORT
#define PACKED_PIXEL_SUPPORT
#define DEVCAP_CACHE
#define NEW_TIMER_API
#define NEW_SERIAL_CODE
#define eTMDS_SOURCE
#define	GET_DVI_MODE_FROM_SINK_EDID_NOT_REGISTER
#define	HW_ASSISTED_EDID_READ
#define LET_HW_DECIDE_UPSTREAM_DVI

#define INTERRUPT_DRIVEN_EDID
#define REQUEST_EDID_ATOMICALLY
#define INFO_FRAMES_AT_TMDS_ENABLE
#define CLEAR_INT_ENABLE_IN_D3
//#define WAKE_ON_CBUS_FALLING_EDGE_ONLY
#define PRESERVE_RGND_IMPEDANCE_UNTIL_VALUE_READ

//define LCD_SUPPORT
#define SWWA_INFOFRAME
//#define SWWA_INFOFRAME_TIMER
#define INTERLEAVE_INT7AND8

//#define EXHAUST_PENDING_INTERRUPTS_LINUX
//#define PROCESS_BKSV_ERR
//#define EXTRA_TMDS_TOGGLE_FOR_HDCP_START
//<ZORO_MHL>#if BUILD_CONFIG >= 2 //( 8558
///#define ASSERT_PUSH_PULL NAMI
//<ZORO_MHL>#endif //)

// debug only -- fake out the 3D write burst data
//#define FAKE_3D_DATA
#define CLEAR_INT_ENABLE_IN_D3
//#define WAKE_ON_CBUS_FALLING_EDGE_ONLY
#define PRESERVE_RGND_IMPEDANCE_UNTIL_VALUE_READ
#if 1 //(

#define BUILD_VARIATION "Drive Upstream HPD - Transcode mode supported"

#else //)(

#define BUILD_VARIATION "Release Upstream HPD control"
#define TRANSCODE_HPD_CONTROL

#endif //)
//#define AVI_PASSTHROUGH

// define this only if SYSTEM_BOARD == SB_EPV5_MARK_II
//#define ENABLE_OCS_OVERRIDE

//#define ENABLE_I2C_DEBUG_PRINT //<ZORO_MHL>

#define ENABLE_APP_DEBUG_PRINT //<ZORO_MHL>
#define ENABLE_TXC_DEBUG_PRINT
#define ENABLE_TXD_DEBUG_PRINT
#define ENABLE_PP_DEBUG_PRINT
#define ENABLE_EDID_DEBUG_PRINT
#define ENABLE_EDID_TRACE_PRINT
#define ENABLE_TX_EDID_PRINT
#define ENABLE_TX_PRUNE_PRINT
#define ENABLE_DUMP_INFOFRAME
#define ENABLE_COLOR_SPACE_DEBUG_PRINT
#define ENABLE_CBUS_DEBUG_PRINT
#define ENABLE_SCRPAD_DEBUG_PRINT
#define ENABLE_INFO_DEBUG_PRINT
#define ENABLE_HDCP_DEBUG_PRINT
#define ENABLE_LITE_DEBUG_PRINT
//#define EDID_DUMP_SW_BUFFER
//#define	EDID_DUMP_8240_BUFFER
//#define	EDID_READ_FIFO_BYTE_MODE
//#define DUMP_INTERRUPT_STATUS
//#define	DEBUG_RI_VALUES
#define	HANDLE_GCP_TO_MUTE
//#define	LEIS_REQUEST_TO_MAKE_2E3_0
#define	CTS_1A_09_FIX
//#define	MDT_TESTER

