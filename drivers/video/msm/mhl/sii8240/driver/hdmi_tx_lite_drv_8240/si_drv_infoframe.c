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

#include "si_memsegsupport.h"
#include "si_common.h"
#ifdef __KERNEL__
#include "sii_hal.h"
#else
#include "hal_timers.h"
#endif

#include "si_platform.h"
#include "si_cra.h"
#include "si_cra_cfg.h"
#include "si_common_regs.h"
#include "si_mhl_defs.h"
#include "si_mhl_tx_base_drv_api.h"
#include "si_hdmi_tx_lite_api.h"
#include "si_app_devcap.h"
#include "si_bitdefs.h"
#include "si_edid.h"
#include "si_drv_hdmi_tx_lite_edid.h"


#ifdef ENABLE_INFO_DEBUG_PRINT //(
static void DumpAviInfoFrameImpl(char *pszFile,int iLine,PHwAviPayLoad_t pPayLoad,char *pszLabel)
{
    if (SiiOsDebugChannelIsEnabled(SII_OSAL_DEBUG_HDMI_DBG))
    {
        SiiOsDebugPrintSimpleAlways(
            "%s:%d %s AVI info frame: \n"
            "%02x %02x %02x %02x %02x %02x %02x "
            "%02x %02x %02x %02x %02x %02x %02x\n"
            ,pszFile,iLine,pszLabel
            ,(uint16_t)pPayLoad->ifData[0x0]
            ,(uint16_t)pPayLoad->ifData[0x1]
            ,(uint16_t)pPayLoad->ifData[0x2]
            ,(uint16_t)pPayLoad->ifData[0x3]
            ,(uint16_t)pPayLoad->ifData[0x4]
            ,(uint16_t)pPayLoad->ifData[0x5]
            ,(uint16_t)pPayLoad->ifData[0x6]
            ,(uint16_t)pPayLoad->ifData[0x7]
            ,(uint16_t)pPayLoad->ifData[0x8]
            ,(uint16_t)pPayLoad->ifData[0x9]
            ,(uint16_t)pPayLoad->ifData[0xA]
            ,(uint16_t)pPayLoad->ifData[0xB]
            ,(uint16_t)pPayLoad->ifData[0xC]
            ,(uint16_t)pPayLoad->ifData[0xD]
            );
    }
}
#define DumpAviInfoFrame(pPayLoad,pszLabel) DumpAviInfoFrameImpl(__FILE__,__LINE__,pPayLoad,pszLabel);

#else //)(

#define DumpAviInfoFrame(pPayLoad,pszLabel) /*nothing*/

#endif //)

uint8_t CalculateSum(uint8_t *info,uint8_t sum,uint8_t length)
{
uint8_t i;

    INFO_DEBUG_PRINT(("sum: %02x length:%x\n",(uint16_t)sum,(uint16_t)length));
    for (i = 0; i < length; i++)
    {
        sum += info[i];
    }
    INFO_DEBUG_PRINT(("sum: %02x\n",(uint16_t)sum));
    return sum;
}

uint8_t CalculateGenericCheckSum(uint8_t *infoFrameData,uint8_t checkSum,uint8_t length)
{
    checkSum = 0x100 - CalculateSum(infoFrameData,checkSum,length);

    INFO_DEBUG_PRINT(("checkSum: %02x\n",(uint16_t)checkSum));
	return checkSum;
}

#define SIZE_AUDIO_INFOFRAME 14
//////////////////////////////////////////////////////////////////////////////
//
// FUNCTION      :  SendAudioInfoFrame()
//
// PURPOSE       :  Load Audio InfoFrame data into registers and send to sink
//
// INPUT PARAMS  :  (1) Channel count (2) speaker configuration per CEA-861D
//                  Tables 19, 20 (3) Coding type: 0x09 for DSD Audio. 0 (refer
//                                      to stream header) for all the rest (4) Sample Frequency. Non
//                                      zero for HBR only (5) Audio Sample Length. Non zero for HBR
//                                      only.
//
// OUTPUT PARAMS :  None
//
// GLOBALS USED  :  None
//
// RETURNS       :  true
//
//////////////////////////////////////////////////////////////////////////////
void SendAudioInfoFrame (void)
{
    if (sink_type_HDMI == SiiMhlTxGetSinkType())
    {
    uint8_t ifData[SIZE_AUDIO_INFOFRAME];
    uint8_t i;


        ifData[0] = 0x84;
        ifData[1] = 0x01;
        ifData[2] = 0x0A;
        for (i = 3; i < SIZE_AUDIO_INFOFRAME;++i)
        {
            ifData[i] = 0;
        }

        ifData[3] = CalculateGenericCheckSum(ifData,0,SIZE_AUDIO_INFOFRAME);


    	SiiRegWrite(REG_TPI_INFO_FSEL
    	        , BIT_TPI_INFO_EN
    	        | BIT_TPI_INFO_RPT
    	        | BIT_TPI_INFO_READ_FLAG_NO_READ
    	        | BIT_TPI_INFO_SEL_Audio
    	        );
        SiiRegWriteBlock(REG_TPI_INFO_BYTE00,ifData,SIZE_AUDIO_INFOFRAME);

        INFO_DEBUG_PRINT(("REG_TPI_INFO_BYTE13: %02x\n",REG_TPI_INFO_BYTE13));
    }
}
#define SIZE_AVI_INFOFRAME				14
static uint8_t CalculateAviInfoFrameChecksum (PHwAviPayLoad_t pPayLoad)
{
	uint8_t checksum;

	checksum = 0x82 + 0x02 + 0x0D;  // these are set by the hardware
    return CalculateGenericCheckSum(pPayLoad->ifData,checksum,SIZE_AVI_INFOFRAME);
}

HwAviPayLoad_t aviPayLoad;
void SiiHmdiTxLiteDrvSendHwAviInfoFrame( void )
{
	INFO_DEBUG_PRINT(("Output Mode: %s\n",(sink_type_HDMI == SiiMhlTxGetSinkType())?"HDMI":"DVI"));
    if (sink_type_HDMI == SiiMhlTxGetSinkType())
    {
    	SiiRegWrite(REG_TPI_INFO_FSEL
    	        , BIT_TPI_INFO_EN
    	        | BIT_TPI_INFO_RPT
    	        | BIT_TPI_INFO_READ_FLAG_NO_READ
    	        | BIT_TPI_INFO_SEL_AVI
    	        );
    	SiiRegWriteBlock(REG_TPI_AVI_CHSUM, (uint8_t*)&aviPayLoad.ifData, sizeof(aviPayLoad.ifData));
        DumpAviInfoFrame(&aviPayLoad,"outgoing")  // no semicolon needed
    }
}

static void SiiHdmiTxLiteDrvPrepareHwAviInfoFrame( PHwAviPayLoad_t pPayLoad )
{
uint8_t                 outputClrSpc;
AviInfoFrameDataByte2_t colorimetryAspectRatio;
AviInfoFrameDataByte4_t inputVideoCode;
QuantizationSettings_e  settings;

    DumpAviInfoFrame(pPayLoad,"preparing")  // no semicolon needed
    outputClrSpc            = SiiMhlTxDrvGetOutputColorSpace();  // originates from EDID

    colorimetryAspectRatio  = SiiMhlTxDrvGetColorimetryAspectRatio();
    inputVideoCode          = SiiMhlTxDrvGetInputVideoCode();

    pPayLoad->namedIfData.checksum = 0; // the checksum itself is included in the calculation.

    pPayLoad->namedIfData.ifData_u.bitFields.pb1.colorSpace = outputClrSpc;
    pPayLoad->namedIfData.ifData_u.bitFields.colorimetryAspectRatio = colorimetryAspectRatio;
    pPayLoad->namedIfData.ifData_u.bitFields.VIC = inputVideoCode;

    settings = qsAutoSelectByColorSpace;
    INFO_DEBUG_PRINT(("%s %02x\n",(EDID_Data.VideoCapabilityFlags.QS)?"VCF":"vcf",(uint16_t)*((uint8_t *)&EDID_Data.VideoCapabilityFlags)));
    if (EDID_Data.VideoCapabilityFlags.QS)
    {
        switch(outputClrSpc)
        {
        case acsRGB:
            INFO_DEBUG_PRINT(("RGB\n"));
            break;
        case acsYCbCr422:
            INFO_DEBUG_PRINT(("422\n"));
        case acsYCbCr444:
            switch (pPayLoad->namedIfData.ifData_u.bitFields.pb5.quantization)
            {
            case aqLimitedRange:
                INFO_DEBUG_PRINT(("444-limited\n"));
                settings = qsLimitedRange;
                break;
            case aqFullRange:
                INFO_DEBUG_PRINT(("444-full\n"));
                settings = qsFullRange;
                break;
            case aqReserved0:
            case aqReserved1:
                INFO_DEBUG_PRINT(("444-reserved\n"));
                    // undefined by CEA-861-E
                break;
            }
            break;
        }
    }
    SiiMhlTxDrvSetOutputQuantizationRange(settings);
    SiiMhlTxDrvSetInputQuantizationRange(settings);

    pPayLoad->namedIfData.ifData_u.bitFields.pb1.futureMustBeZero = 0; //<ZORO_MHL>
    pPayLoad->namedIfData.ifData_u.bitFields.VIC.futureMustBeZero = 0; //<ZORO_MHL>

    pPayLoad->namedIfData.checksum = CalculateAviInfoFrameChecksum(pPayLoad);
    INFO_DEBUG_PRINT(("Drv: outputClrSpc: %02x\n",(uint16_t)outputClrSpc));

    DumpIncomingInfoFrame(pPayLoad,sizeof(*pPayLoad));


    DumpAviInfoFrame(pPayLoad,"preparing")  // no semicolon needed
    aviPayLoad = *pPayLoad;
}


void SiiHdmiTxLiteDrvProcessAviInfoFrameChange(PAVIInfoFrame_t pAviInfoFrame)
{
AviColorSpace_e inputClrSpc;
AviInfoFrameDataByte2_t colorimetryAspectRatio;
AviInfoFrameDataByte4_t inputVideoCode        ;

    INFO_DEBUG_PRINT(("SiiHdmiTxLiteDrvProcessAviInfoFrameChange\n"));


    aviPayLoad  = pAviInfoFrame->payLoad.hwPayLoad;
    DumpAviInfoFrame(&aviPayLoad,"incoming")  // no semicolon needed

    inputClrSpc             = pAviInfoFrame->payLoad.hwPayLoad.namedIfData.ifData_u.bitFields.pb1.colorSpace;
    colorimetryAspectRatio  = pAviInfoFrame->payLoad.hwPayLoad.namedIfData.ifData_u.bitFields.colorimetryAspectRatio;//ifData[0x2] //infoframeData[0x1];
    inputVideoCode          = pAviInfoFrame->payLoad.hwPayLoad.namedIfData.ifData_u.bitFields.VIC;//ifData[0x4] //infoframeData[0x3];                   SII_ASSERT(4==SII_OFFSETOF(AVIInfoFrame_t,payLoad.hwPayLoad.namedIfData.ifData_u.bitFields.pb1),("\\n\n\n OFFSET ERROR %d\n\n\n",SII_OFFSETOF(AVIInfoFrame_t,payLoad.hwPayLoad.namedIfData.ifData_u.bitFields.pb1)));

    COLOR_SPACE_DEBUG_PRINT(("inputClrSpc:%02x infoData[0]:%02x\n"
                ,(uint16_t)inputClrSpc
                ,(uint16_t)pAviInfoFrame->payLoad.hwPayLoad.namedIfData.ifData_u.infoFrameData[0]
                ));
    // make note of incoming info frame data that we will need later.
    SiiMhlTxDrvSetInputColorSpace(inputClrSpc);
    SiiMhlTxDrvSetColorimetryAspectRatio(colorimetryAspectRatio);
    SiiMhlTxDrvSetInputVideoCode(inputVideoCode);

    INFO_DEBUG_PRINT(("inputClrSpc: %02x"
                      " other way: %02x"
                      " colorimetryAspectRatio: %02x"
                      " inputVideoCode: %02x"
					  "\n"
                     ,*(uint16_t*)&inputClrSpc
                     ,(uint16_t)((pAviInfoFrame->payLoad.hwPayLoad.ifData[1]&0x60)>>5)
                     ,*(uint16_t*)&colorimetryAspectRatio
                     ,*(uint16_t*)&inputVideoCode
                    ));
    SiiHdmiTxLiteDrvPrepareHwAviInfoFrame(&aviPayLoad);
    DumpAviInfoFrame(&aviPayLoad,"incoming")  // no semicolon needed
}

void SiiHmdiTxLiteDrvPrepareAviInfoframe ( void )
{
#ifdef AVI_PASSTHROUGH //(
    INFO_DEBUG_PRINT(("PrepareAviInfoframe\n"));
    SiiHdmiTxLiteDrvSendHwAviInfoFrame(&aviPayLoad);

#else //)(
HwAviPayLoad_t hwIfData;
    INFO_DEBUG_PRINT(("PrepareAviInfoframe\n"));

    hwIfData.namedIfData.ifData_u.bitFields.pb1.colorSpace = SiiMhlTxDrvGetOutputColorSpace(); // originates from incoming infoframe
	hwIfData.namedIfData.ifData_u.bitFields.colorimetryAspectRatio = SiiMhlTxDrvGetColorimetryAspectRatio();
	hwIfData.ifData[0x3] = 0x00;
	hwIfData.namedIfData.ifData_u.bitFields.VIC = SiiMhlTxDrvGetInputVideoCode();
	hwIfData.namedIfData.ifData_u.bitFields.pb5 = aviPayLoad.namedIfData.ifData_u.bitFields.pb5;
	hwIfData.ifData[0x6] = 0x00;
	hwIfData.ifData[0x7] = 0x00;
	hwIfData.ifData[0x8] = 0x00;
	hwIfData.ifData[0x9] = 0x00;
	hwIfData.ifData[0xA] = 0x00;
	hwIfData.ifData[0xB] = 0x00;
	hwIfData.ifData[0xC] = 0x00;
	hwIfData.ifData[0xD] = 0x00;

    SiiHdmiTxLiteDrvPrepareHwAviInfoFrame(&hwIfData);
#endif //)
}

void SiiHdmiTxLiteDrvSendVendorSpecificInfoFrame(PVendorSpecificInfoFrame_t pVendorSpecificInfoFrame)
{

    if (sink_type_HDMI == SiiMhlTxGetSinkType())
    {
        INFO_DEBUG_PRINT(("VSIF: length:%02x constant:%02x\n"
					,(uint16_t)pVendorSpecificInfoFrame->header.length
					,(uint16_t)sizeof(pVendorSpecificInfoFrame->payLoad)
					));
        SII_ASSERT((11==sizeof(*pVendorSpecificInfoFrame)),("\n\n\ninfo frame size:%02x not expected\n\n\n"
					,(uint16_t)sizeof(*pVendorSpecificInfoFrame)) )

        pVendorSpecificInfoFrame->payLoad.checksum = 0;
        pVendorSpecificInfoFrame->payLoad.checksum = CalculateGenericCheckSum((uint8_t *)&pVendorSpecificInfoFrame->payLoad,0,sizeof(pVendorSpecificInfoFrame->payLoad));;
    	SiiRegWriteBlock(REG_TPI_INFO_BYTE00, (uint8_t *)pVendorSpecificInfoFrame, sizeof(*pVendorSpecificInfoFrame));
    }
}

