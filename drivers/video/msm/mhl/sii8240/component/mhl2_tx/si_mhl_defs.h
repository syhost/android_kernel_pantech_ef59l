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
   @file si_mhl_defs.h
 */

//
// This file contains MHL Specs related definitions.
//
/*
 * DEVCAP offsets
 */

SI_PUSH_STRUCT_PACKING //(
typedef struct SI_PACK_THIS_STRUCT _MHLDevCap_t
{
    uint8_t state;
    uint8_t mhlVersion;
    uint8_t deviceCategory;
    uint8_t adopterIdHigh;
    uint8_t adopterIdLow;
    uint8_t vidLinkMode;
    uint8_t audLinkMode;
    uint8_t videoType;
    uint8_t logicalDeviceMap;
    uint8_t bandWidth;
    uint8_t featureFlag;
    uint8_t deviceIdHigh;
    uint8_t deviceIdLow;
    uint8_t scratchPadSize;
    uint8_t intStateSize;
    uint8_t reserved;
}MHLDevCap_t,*PMHLDevCap_t;

typedef union
{
    MHLDevCap_t mdc;
	uint8_t		aucDevCapCache[16];
}MHLDevCap_u,*PMHLDevCap_u;
SI_POP_STRUCT_PACKING //)

typedef enum
{
     DEVCAP_OFFSET_DEV_STATE 		= 0x00
    ,DEVCAP_OFFSET_MHL_VERSION		= 0x01
    ,DEVCAP_OFFSET_DEV_CAT          = 0x02
    ,DEVCAP_OFFSET_ADOPTER_ID_H     = 0x03
    ,DEVCAP_OFFSET_ADOPTER_ID_L     = 0x04
    ,DEVCAP_OFFSET_VID_LINK_MODE    = 0x05
    ,DEVCAP_OFFSET_AUD_LINK_MODE    = 0x06
    ,DEVCAP_OFFSET_VIDEO_TYPE       = 0x07
    ,DEVCAP_OFFSET_LOG_DEV_MAP      = 0x08
    ,DEVCAP_OFFSET_BANDWIDTH        = 0x09
    ,DEVCAP_OFFSET_FEATURE_FLAG     = 0x0A
    ,DEVCAP_OFFSET_DEVICE_ID_H      = 0x0B
    ,DEVCAP_OFFSET_DEVICE_ID_L      = 0x0C
    ,DEVCAP_OFFSET_SCRATCHPAD_SIZE  = 0x0D
    ,DEVCAP_OFFSET_INT_STAT_SIZE    = 0x0E
    ,DEVCAP_OFFSET_RESERVED         = 0x0F
    // this one must be last
    ,DEVCAP_SIZE
}DevCapOffset_e;

// Device Power State
#define MHL_DEV_UNPOWERED		0x00
#define MHL_DEV_INACTIVE		0x01
#define MHL_DEV_QUIET			0x03
#define MHL_DEV_ACTIVE			0x04

// Version that this chip supports
//<ZORO_MHL>#define	MHL_VER_MAJOR		(0x02 << 4)	// bits 4..7
//<ZORO_MHL>#define	MHL_VER_MINOR		0x00		// bits 0..3
#define	MHL_VER_MAJOR		(0x01 << 4)	// bits 4..7
#define	MHL_VER_MINOR		0x02		// bits 0..3
#define MHL_VERSION						(MHL_VER_MAJOR | MHL_VER_MINOR)

//Device Category
#define	MHL_DEV_CATEGORY_OFFSET				DEVCAP_OFFSET_DEV_CAT
#define	MHL_DEV_CATEGORY_POW_BIT			(BIT4)

#define	MHL_DEV_CAT_SINK					0x01
#define	MHL_DEV_CAT_SOURCE					0x02
#define	MHL_DEV_CAT_DONGLE					0x03
#define	MHL_DEV_CAT_SELF_POWERED_DONGLE		0x13

//Video Link Mode
#define	MHL_DEV_VID_LINK_SUPPRGB444			0x01
#define	MHL_DEV_VID_LINK_SUPPYCBCR444		0x02
#define	MHL_DEV_VID_LINK_SUPPYCBCR422		0x04
#define	MHL_DEV_VID_LINK_SUPP_PPIXEL		0x08
#define	MHL_DEV_VID_LINK_SUPP_ISLANDS		0x10

//Audio Link Mode Support
#define	MHL_DEV_AUD_LINK_2CH				0x01
#define	MHL_DEV_AUD_LINK_8CH				0x02


//Feature Flag in the devcap
#define	MHL_DEV_FEATURE_FLAG_OFFSET			DEVCAP_OFFSET_FEATURE_FLAG
#define	MHL_FEATURE_RCP_SUPPORT				0x01	// Dongles have freedom to not support RCP
#define	MHL_FEATURE_RAP_SUPPORT				0x02	// Dongles have freedom to not support RAP
#define	MHL_FEATURE_SP_SUPPORT				0x04	// Dongles have freedom to not support SCRATCHPAD
#define	MHL_FEATURE_UCP_SEND_SUPPORT		0x08
#define	MHL_FEATURE_UCP_RECV_SUPPORT		0x10

/*
#define		MHL_POWER_SUPPLY_CAPACITY		16		// 160 mA current
#define		MHL_POWER_SUPPLY_PROVIDED		16		// 160mA 0r 0 for Wolverine.
#define		MHL_HDCP_STATUS					0		// Bits set dynamically
*/

// VIDEO TYPES
#define		MHL_VT_GRAPHICS					0x00
#define		MHL_VT_PHOTO					0x02
#define		MHL_VT_CINEMA					0x04
#define		MHL_VT_GAMES					0x08
#define		MHL_SUPP_VT						0x80

//Logical Dev Map
#define	MHL_DEV_LD_DISPLAY					(0x01 << 0)
#define	MHL_DEV_LD_VIDEO					(0x01 << 1)
#define	MHL_DEV_LD_AUDIO					(0x01 << 2)
#define	MHL_DEV_LD_MEDIA					(0x01 << 3)
#define	MHL_DEV_LD_TUNER					(0x01 << 4)
#define	MHL_DEV_LD_RECORD					(0x01 << 5)
#define	MHL_DEV_LD_SPEAKER					(0x01 << 6)
#define	MHL_DEV_LD_GUI						(0x01 << 7)

//Bandwidth
#define	MHL_BANDWIDTH_LIMIT					22		// 225 MHz


#define MHL_STATUS_REG_CONNECTED_RDY        0x30
#define MHL_STATUS_REG_LINK_MODE            0x31

#define	MHL_STATUS_DCAP_RDY					BIT0

#define MHL_STATUS_CLK_MODE_MASK            0x07
#define MHL_STATUS_CLK_MODE_PACKED_PIXEL    0x02
#define MHL_STATUS_CLK_MODE_NORMAL          0x03
#define MHL_STATUS_PATH_EN_MASK             0x08
#define MHL_STATUS_PATH_ENABLED             0x08
#define MHL_STATUS_PATH_DISABLED            0x00
#define MHL_STATUS_MUTED_MASK               0x10

#define MHL_RCHANGE_INT                     0x20
#define MHL_DCHANGE_INT                     0x21

#define	MHL_INT_DCAP_CHG					BIT0
#define MHL_INT_DSCR_CHG                    BIT1
#define MHL_INT_REQ_WRT                     BIT2
#define MHL_INT_GRT_WRT                     BIT3
#define MHL2_INT_3D_REQ                     BIT4

// On INTR_1 the EDID_CHG is located at BIT 0
#define	MHL_INT_EDID_CHG					BIT1

#define		MHL_INT_AND_STATUS_SIZE			0x33		// This contains one nibble each - max offset
#define		MHL_SCRATCHPAD_SIZE				16
#define		MHL_MAX_BUFFER_SIZE				MHL_SCRATCHPAD_SIZE	// manually define highest number

typedef enum
{
     burstId_3D_VIC = 0x0010
    ,burstId_3D_DTD = 0x0011

    // add new burst ID's above here

    /*  Burst ID's are a 16-bit big-endian quantity.
        In order for the BURST_ID macro below to allow detection of
            out-of-range values with KEIL 8051 compiler
            we must have at least one enumerated value
            that has one of the bits in the high order byte set.
        Experimentally, we have found that the KEIL 8051 compiler
         treats 0xFFFF as a special case (-1 perhaps...),
         so we use a different value that has some upper bits set
    */
    ,burstId_16_BITS_REQUIRED = 0x8000
}BurstId_e;

typedef struct _Mhl2HighLow_t
{
    uint8_t high;
    uint8_t low;
}Mhl2HighLow_t,*PMhl2HighLow_t;

#define BURST_ID(bid) (BurstId_e)(( ((uint16_t)(bid.high))<<8 )|((uint16_t)(bid.low)))

// see MHL2.0 spec section 5.9.1.2
typedef struct _Mhl2VideoDescriptor_t
{
    uint8_t reservedHigh;
    unsigned char FrameSequential:1;    //FB_SUPP
    unsigned char TopBottom:1;          //TB_SUPP
    unsigned char LeftRight:1;          //LR_SUPP
    unsigned char reservedLow:5;
}Mhl2VideoDescriptor_t,*PMhl2VideoDescriptor_t;

typedef struct _Mhl2VideoFormatData_t
{
    Mhl2HighLow_t burstId;
    uint8_t checkSum;
    uint8_t totalEntries;
    uint8_t sequenceIndex;
    uint8_t numEntriesThisBurst;
    Mhl2VideoDescriptor_t videoDescriptors[5];
}Mhl2VideoFormatData_t,*PMhl2VideoFormatData_t;

enum
{
    MHL_MSC_MSG_RCP             = 0x10,     // RCP sub-command
    MHL_MSC_MSG_RCPK            = 0x11,     // RCP Acknowledge sub-command
    MHL_MSC_MSG_RCPE            = 0x12,     // RCP Error sub-command
    MHL_MSC_MSG_RAP             = 0x20,     // Mode Change Warning sub-command
    MHL_MSC_MSG_RAPK            = 0x21,     // MCW Acknowledge sub-command
    MHL_MSC_MSG_UCP             = 0x30,     // UCP sub-command
    MHL_MSC_MSG_UCPK            = 0x31,     // UCP Acknowledge sub-command
    MHL_MSC_MSG_UCPE            = 0x32,     // UCP Error sub-command
};

#define	RCPE_NO_ERROR				0x00
#define	RCPE_INEFFECTIVE_KEY_CODE	0x01
#define	RCPE_BUSY					0x02
//
// MHL spec related defines
//
enum
{
	MHL_ACK						= 0x33,	// Command or Data byte acknowledge
	MHL_NACK					= 0x34,	// Command or Data byte not acknowledge
	MHL_ABORT					= 0x35,	// Transaction abort
	MHL_WRITE_STAT				= 0x60 | 0x80,	// 0xE0 - Write one status register strip top bit
	MHL_SET_INT					= 0x60,	// Write one interrupt register
	MHL_READ_DEVCAP				= 0x61,	// Read one register
	MHL_GET_STATE				= 0x62,	// Read CBUS revision level from follower
	MHL_GET_VENDOR_ID			= 0x63,	// Read vendor ID value from follower.
	MHL_SET_HPD					= 0x64,	// Set Hot Plug Detect in follower
	MHL_CLR_HPD					= 0x65,	// Clear Hot Plug Detect in follower
	MHL_SET_CAP_ID				= 0x66,	// Set Capture ID for downstream device.
	MHL_GET_CAP_ID				= 0x67,	// Get Capture ID from downstream device.
	MHL_MSC_MSG					= 0x68,	// VS command to send RCP sub-commands
	MHL_GET_SC1_ERRORCODE		= 0x69,	// Get Vendor-Specific command error code.
	MHL_GET_DDC_ERRORCODE		= 0x6A,	// Get DDC channel command error code.
	MHL_GET_MSC_ERRORCODE		= 0x6B,	// Get MSC command error code.
	MHL_WRITE_BURST				= 0x6C,	// Write 1-16 bytes to responder's scratchpad.
	MHL_GET_SC3_ERRORCODE		= 0x6D,	// Get channel 3 command error code.

    MHL_READ_EDID_BLOCK       // let this one float, it has no specific value
};

#define	MHL_RAP_CONTENT_ON		0x10	// Turn content streaming ON.
#define	MHL_RAP_CONTENT_OFF		0x11	// Turn content streaming OFF.

///////////////////////////////////////////////////////////////////////////////
//
// MHL Timings applicable to this driver.
//
//
#define	T_SRC_VBUS_CBUS_TO_STABLE	(200)	// 100 - 1000 milliseconds. Per MHL 1.0 Specs
#define	T_SRC_WAKE_PULSE_WIDTH_1	(20)	// 20 milliseconds. Per MHL 1.0 Specs
#define	T_SRC_WAKE_PULSE_WIDTH_2	(60)	// 60 milliseconds. Per MHL 1.0 Specs

#define	T_SRC_WAKE_TO_DISCOVER		(200)	// 100 - 1000 milliseconds. Per MHL 1.0 Specs

#define T_SRC_VBUS_CBUS_T0_STABLE 	(500)

// Allow RSEN to stay low this much before reacting.
// Per specs between 100 to 200 ms
#define	T_SRC_RSEN_DEGLITCH			(100)	// (150)

// Wait this much after connection before reacting to RSEN (300-500ms)
// Per specs between 300 to 500 ms
#define	T_SRC_RXSENSE_CHK			(400)


