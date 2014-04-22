#ifndef CUST_PANTECH_H
#define CUST_PANTECH_H

#include "BOARD_REV.h"

#define FEATURE_PANTECH_MODEL                       //chjeon20111031@LS1 add CS tool.

/*******************************************************************************
 * SIO(USB&UART&TESTMENU&FACTORY) PART HEADER
 * ****************************************************************************/
#include "CUST_PANTECH_SIO.h"

#if !defined(__KERNELBUILD__)
#define CONFIG_PANTECH /* 20121025 jylee */
/*******************************************************************************
**  USER DATA REBUILDING VERSION
*******************************************************************************/
#define FEATURE_SKY_USER_DATA_VER
#define FEATURE_SKY_FAT16_FOR_SBL3
//20111220 jwheo Data Encryption
#define FEATURE_SKY_DATA_ENCRYPTION

#endif

#if !defined(__KERNELBUILD__) && !defined(__MODEMBUILD__)

#define CONFIG_PANTECH_ERR_CRASH_LOGGING

#endif

#if !defined(__KERNELBUILD__) && !defined(__MODEMBUILD__) && \
        !defined(__BOOTBUILD__) && !defined(__TZBUILD__) 
/*******************************************************************************
** SOUND
*******************************************************************************/
#include "CUST_PANTECH_SOUND.h"


/*******************************************************************************
**  Camera
*******************************************************************************/
#include "CUST_PANTECH_CAMERA.h"


/*******************************************************************************
**  Display
*******************************************************************************/
#include "CUST_PANTECH_DISPLAY.h"

/*******************************************************************************
**  EXT4 (repair /data partition)  manual, auto repair
*******************************************************************************/
#define FEATURE_RECOVERY_HIDDEN_MENU
#define FEATURE_CS_USERDATA_BACKUP
#define FEATURE_PANTECH_AUTO_REPAIR
#define FEATURE_PANTECH_MANUAL_REPAIR


/*******************************************************************************
**  WIFI
*******************************************************************************/
#include "CUST_PANTECH_WIFI.h"

/****************************************************
** POWER ON/OFF REASON COUNT
****************************************************/
#define FEATURE_PANTECH_PWR_ONOFF_REASON_CNT
#define FEATURE_PANTECH_ANDROID_RESETCNT // p13783 add : for Framework reset count

#endif

/*******************************************************************************
**  SKY_RAW_DATA
*******************************************************************************/
#define FEATURE_SKY_RAWDATA_ACCESS

/*******************************************************************************
**  SKY_RAW_DATA
*******************************************************************************/
#define FEATURE_PANTECH_QMI_SERVER

/*******************************************************************************
** DATA
*******************************************************************************/
#include "cust_pantech_data_linux.h"

/*******************************************************************************
**  FACTORY_COMMAND
*******************************************************************************/
#define FEATURE_SKY_MD5_VERIFY	//p14527 add for FACTORY_PRELOAD_CHECK_I
#ifdef FEATURE_PANTECH_FACTORY_COMMAND
#define F_PANTECH_FACTORY_INIT_ALL_CMD
#define F_PANTECH_APP_CRC_CHECK
#define PANTECH_DIAG_MSECTOR
#define FEATURE_PANTECH_CS_AUTO_TAKEOVER
#define F_PANTECH_UTS_POWER_UP //p13783 add : for FCMD
#endif

/*******************************************************************************
**  SENSOR LS2
*******************************************************************************/
#include "CUST_PANTECH_SENSOR.h"  

/*******************************************************************************
**  GOTA
*******************************************************************************/
#include "CUST_PANTECH_GOTA.h"

/*******************************************************************************
**  WLAN
*******************************************************************************/
/* WLAN Common Feature */
#define FEATURE_PANTECH_WLAN_PROCESS_CMD
#define FEATURE_PANTECH_WLAN_FTM_TX_OFFSET    // 20131104, Target power control for KCP(FTM) test on regulartory mode
#define FEATURE_PANTECH_WLAN_TESTMENU
#define FEATURE_PANTECH_WLAN_RAWDATA_ACCESS
#define FEATURE_PANTECH_WLAN_FOUR_MAC_ADDRESS // for pantech_wifi_mac_backup.h
#define FEATURE_PANTECH_WLAN_MAC_ADDR_BACKUP // used in skytest_thread.c
#define FEATURE_PANTECH_WLAN_TRP_TIS // 2012-04-09, Pantech only, ymlee_p11019, to config & test TRP TIS
//#define FEATURE_PANTECH_WLAN_DEBUG_LEVEL_FRAMEWORK  // 20121031 thkim_wifi add for wifi logging
//LS3_LeeYoungHo_130402_blk [move to Kbuild file in vendor\qcom\proprietary\wlan\pronto
#if 0
#define FEATURE_PANTECH_WLAN // used in wlan_hdd_main.c
#define FEATURE_PANTECH_WLAN_QCOM_PATCH
#endif//LS3_LeeYoungHo_130402_blk

//#define FEATURE_PANTECH_5G_DPD_ENABLE_AUTO_TEST // 2012-04-02, Pantech only, ymlee_p11019, On Auto test mode 5G DPD enabled // 20121217 thkim_wifi block for wifi 80mhz test
#define FEATURE_PANTECH_WLAN_FOUR_MAC_ADDRESS_WCN3660
#define FEATURE_PANTECH_WLAN_MAC_ADDR_BACKUP_WCN3660

/*******************************************************************************
 **  WIDEVINE DRM
*******************************************************************************/
#define FEATURE_PANTECH_WIDEVINE_DRM

/*******************************************************************************
** POWER ON/OFF REASON COUNT & RESET REASEON
*******************************************************************************/
//#define FEATURE_PANTECH_BOOT_PM
//#define FEATURE_PANTECH_PWR_ONOFF_REASON_CNT
//#define FEATURE_PANTECH_RESET_REASON

//#ifdef FEATURE_PANTECH_RESET_REASON
/*
#define QCOM_RESTART_REASON_ADDR   (0xFA00A000 + 0x65C)
#define PANTECH_RESTART_REASON_OFFSET 0x68 //0x08//0x04
#define PANTECH_DDR_INFO_OFFSET		   0x10
#define PANTECH_RESTART_REASON_ADDR   (QCOM_RESTART_REASON_ADDR + PANTECH_RESTART_REASON_OFFSET)
#define PANTECH_DDR_INFO_ADDR   (PANTECH_RESTART_REASON_ADDR + PANTECH_DDR_INFO_OFFSET)
*/
/*******************************************************************************
 **  PANTECH CERTIFICATION FOR Image_verify
*******************************************************************************/
#define FEATURE_PANTECH_KEY_CERTIFICATION


/*******************************************************************************
 **  PANTECH ROOTING CHECK		//lsi@ls1
*******************************************************************************/
#define F_PANTECH_OEM_ROOTING

/*******************************************************************************
 **  PANTECH SECURE BOOT		//lsi@ls1
*******************************************************************************/
#if 0 //defined(T_STARQ) || defined(T_VEGAPVW)
#define F_PANTECH_SECBOOT
#endif

/*************************************************************************/
/****************************  PANTECH UIM ********************************/
/*************************************************************************/
#define F_PANTECH_UIM_TESTMENU

#define FEATURE_PANTECH_ERR_CRASH_LOGGING


/* Emergency Dload USB */
/* define after merging dload module #define FEATURE_PANTECH_DLOAD_USB*/
/*******************************************************************************
  **  PDL (LK(emergency), bootimage(phoneinfo), KERNEL(idle download))
  *******************************************************************************/
#define FEATURE_PANTECH_PDL_DLOADINFO
#define FEATURE_PANTECH_PDL_DLOAD
#define FEATURE_PANTECH_FLASH_ACCESS
#define FEATURE_PANTECH_DLOAD_USB
#define FEATURE_PANTECH_REBOOT_FOR_IDLE_DL

/* All MSM8974 model support */
#define FEATURE_SKY_MDM_PREVENT_UPGRADE
// #define FEATURE_SKY_MDM_PREVENT_TEST // just testing.FEATURE_SKY_MDM_PREVENT_UPGRADE

#define FEATURE_PANTECH_GOTA_BACKUP_CODE
#define FEATURE_PANTECH_GPT_RECOVERY     //chjeon20120412@LS1 add

/*******************************************************************************
**  PMIC
*******************************************************************************/
#include "CUST_PANTECH_PMIC.h"

/*******************************************************************************
**  P12554 Codec
*******************************************************************************/
#include "CUST_PANTECH_MMP.h"

/*******************************************************************************
** Certus Service
*******************************************************************************/
#if defined(T_EF56S) || defined(T_EF57K) || defined(T_EF58L) || defined(T_EF59S) ||  defined(T_EF59K) || defined(T_EF59L) || defined(T_EF60S) || defined(T_EF61K) || defined(T_EF62L)
#define FEATURE_PANTECH_CERTUS
#endif

/******************************************************************************
**  Media Framework
******************************************************************************/
#include "CUST_PANTECH_MF.h"

/*******************************************************************************
** Android Patternlock Reset
*******************************************************************************/
#define FEATURE_PANTECH_PATTERN_UNLOCK


/*******************************************************************************
** for making and loading boot image
*******************************************************************************/
#define FEATURE_PANTECH_GEN_SKY_ABOOT
/*******************************************************************************
** for Touch Gold Reference
*******************************************************************************/
#if defined(T_EF56S) || defined(T_EF57K) || defined(T_EF58L) || defined(T_EF59S) ||  defined(T_EF59K) || defined(T_EF59L) || defined(T_EF60S) || defined(T_EF61K) || defined(T_EF62L)
#define FEATURE_PANTECH_TOUCH_GOLDREFERENCE
#endif

/*******************************************************************************
** UICC NO Power Down when going to LPM 
*******************************************************************************/
#if defined(T_NAMI)
#define FEATURE_PANTECH_MMGSDI_CARD_NOT_POWER_DOWN
#endif

/*******************************************************************************
** Bluetooth
*******************************************************************************/
/*  [BLUETOOTH_A2DP_LPA_DISABLE] 13/07/15 LS3_p12602_Jeong Ungbi: Disabling LPA mode when A2DP is connected */
#define FEATURE_PANTECH_BLUETOOTH_A2DP_LPA_DISABLE

/*******************************************************************************
** for checking formatting flag
*******************************************************************************/
#define FEATURE_CHECK_FORMATTING_FLAG

/******************************************************************************
**  ECO CPU Mode I/F
******************************************************************************/
#if defined(T_EF56S)
#define FEATURE_PANTECH_ECO_CPU_MODE
#endif

/******************************************************************************
**  BOOT BOOSTER FEATURE
******************************************************************************/
#if defined(T_EF60S) || defined(T_EF61K) || defined(T_EF62L)
#define BOOT_BOOSTER_WORK		// Inserted by dscheon. for Booster Booting. 2013.11.18.
#endif

/******************************************************************************
**  CPU Min Freq.limit for Thermal Mitigation
******************************************************************************/
#define FEATURE_PANTECH_CPU_MIN_FREQ_LIMIT


/*******************************************************************************
**  Call PROTOCOL
*******************************************************************************/
/* 2013-04-10 octopusy added  [PS1/2 Team Feature] */
#ifdef T_SKY_MODEL_TARGET_COMMON
#include "cust_lgu_cp_linux.h"
#endif/* T_SKY_MODEL_TARGET_COMMON */


//+US1-CF1
//Feature : FW_VENDOR_FOR_AOT_VIDEO_APP
//API support for AOT 
#define FEATURE_PANTECH_MMP_SUPPORT_AOT
//-US1-CF1

// p13040 ++ [DS4] for DRM
#include "CUST_PANTECH_DRM.h"
// p13040 -- [DS4] for DRM
#endif/* CUST_PANTECH_H */

