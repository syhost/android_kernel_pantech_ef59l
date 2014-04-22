#ifndef __CUST_PANTECH_WIFI_H__
#define __CUST_PANTECH_WIFI_H__
/* ========================================================================

**defined at cust_pantech.h**

FILE: cust_pantech_wifi.h

Copyright (c) 2012 by PANTECH Incorporated.  All Rights Reserved.

USE the format "FEATURE_SKY_WIFI_XXXX"
=========================================================================== */ 

/*===========================================================================

                      EDIT HISTORY FOR FILE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

  when      who       what, where, why
--------  ---      ----------------------------------------------------------
12/10/31  ksg        initial at EF50L JB
12/11/16  ksg        FEATURE_SKY_WIFI_XXXXX ���·� ���� FEATURE ����

===========================================================================*/

/*===========================================================================*/
  //!!ATTENTION!!
/*===========================================================================*/
/*------------------------------------------------------------------------------------

  1. before add this file at /vendor/pantech/build, must define this file at cust_panteh.h
  
  2. must record history in this file header when you modify this file.

  3. FEEATUR's name start with "FEATURE_VEGA_WIFI_XXXX"
  
  4. each FEATURE is need to detailed description. because this file is instad of Feature Specification.
        - Item, Comment(Date, Author), Reason, Modified Files, Added Files, Deleted Files.

  5. In Java Code, Feature' exprssion is comment.
        - Exmaple. // FEATURE_VEGA_WIFI_XXXXXXX (ex. _DS_WIFI_XXX is not invalid.)
        
--------------------------------------------------------------------------------------*/
#define FEATURE_PANTECH_WIFI

#define PANTECH_WIFIINFO_VENDOR_INDEX        0
#define PANTECH_WIFIINFO_MODEL_INDEX         1
#define PANTECH_WIFIINFO_UXTHEME_INDEX       2
#define PANTECH_WIFIINFO_FEATURE_INDEX       3

#if defined(T_EF51S) || defined(T_EF52S) || defined(T_EF56S) || defined(EF56S) || defined(T_EF59S)
	#define FEATURE_SKY_WIFI_VENDOR_SKT
#elif defined(T_EF51K) || defined(T_EF52K) || defined(T_EF57K) || defined(EF57K) || defined(T_EF59K)
	#define FEATURE_SKY_WIFI_VENDOR_KT
#elif defined(T_EF51L) || defined(T_EF52L) || defined(T_EF58L) || defined(EF58L) || defined(T_EF59L)
	#define FEATURE_SKY_WIFI_VENDOR_LGU
#endif	


/*===========================================================================
    Definitions of FEATURE
============================================================================*/

//In android.config, Set CONFIG_PCSC=y and CONFIG_EAP_AKA=y
#define FEATURE_SKY_WIFI_EAP_AKA_AUTHENTICATION

/* 2. Enable the fixed wpa_supplicant FEATURE */
#define FEATURE_SKY_WIFI_WPA_SUPPLICANT

/* 3. Enable the fixed wpa_supplicant scan FEATUREs */
#define FEATURE_SKY_WIFI_WPA_SUPPLICANT_SCAN

/* 4. Enable the WIFI DIRECT FEATURE */
#define FEATURE_SKY_WIFI_DIRECT_FEATURES

#ifdef FEATURE_SKY_WIFI_EAP_AKA_AUTHENTICATION

#ifdef FEATURE_SKY_WIFI_VENDOR_KT
  /* EAP-AKA Test Code */
  #define FEATURE_SKY_WIFI_EAP_AKA_AUTHENTICATION_TEST
#endif  

/* EAP-AKA OS_UNIX.C Group Init. Add radio */
#define FEATURE_SKY_WIFI_EAP_AKA_OS_GROUP_ADD_RADIO

/* EAP-AKA Config Set Default Config or Add or Delete */
#define FEATURE_SKY_WIFI_EAP_AKA_CONFIG

/* EAP-AKA Save the Pseudonym Id and Permanent Id */
#define FEATURE_SKY_WIFI_EAP_AKA_SAVE_PSEUDONYM_AND_PERMANENT_ID

/* EAP-AKA Add NAI Realm */
#define FEATURE_SKY_WIFI_EAP_AKA_ADD_NAI_REALM_PORTION

/* EAP-AKA about PCSC Smart Card Function */
#define FEATURE_SKY_WIFI_EAP_AKA_PCSC_SMART_CARD_FUNCS

/* EAP-AKA Check allowable Operator AP. If not allowable AP, Skip*/
#define FEATURE_SKY_WIFI_EAP_AKA_CHECK_ALLOWABLE_OPERATOR_AP

/* EAP-AKA When EAP-FAILURE happened, manage !*/
#define FEATURE_SKY_WIFI_EAP_AKA_MANAGE_EAP_FAILURE

/* EAP-AKA WPA Message for sending to the WifiMonitor */
#define FEATURE_SKY_WIFI_EAP_AKA_WPA_MSG

//#ifndef FEATURE_SKY_WIFI_VENDOR_LGU
  /* EAP-AKA WPA TimeOut Message for sending to the WifiMonitor */
  #define FEATURE_SKY_WIFI_EAP_AKA_WPA_MSG_TIMEOUT
//#endif

/* EAP-AKA Prevent to ruin UTF-8 Format about EAP-Notification Message */
#define FEATURE_SKY_WIFI_EAP_AKA_PREVENT_UTF_8_FORMAT

/* EAP-AKA Not use reauthentication but use Full authentication  */
#define FEATURE_SKY_WIFI_EAP_AKA_DONT_USE_REAUTHENTICATION

/* EAP-AKA, After EAP-AKA failed, Initialize the ID and Permanent ID, and Pseudonym Id */
#define FEATURE_SKY_WIFI_EAP_AKA_INITIALIZE_ID_AFTER_FAIL

/*  EAP-AKA AT_NOTIFICATION GENERAL FAILURE AFTER AUTH for sending to the WifiMonitor */
#define FEATURE_SKY_WIFI_EAP_AKA_AT_NOTIFICATION_EAP_SIM_GENERAL_FAILURE_AFTER_AUTH

/*  EAP-AKA AT_NOTIFICATION GENERAL FAILURE BEFORE AUTH for sending to the WifiMonitor */
#define FEATURE_SKY_WIFI_EAP_AKA_AT_NOTIFICATION_EAP_SIM_GENERAL_FAILURE_BEFORE_AUTH

/* EAP-AKA UMTS AUTH FAILED AUTN INCORRECT MAC for sending to the WifiMonitor */
#define FEATURE_SKY_WIFI_EAP_AKA_UMTS_AUTHENTICATION_FAILED_AUTN_INCORRECT_MAC

/* EAP-AKA UMTS AUTH AT_NOTIFICATION EAP_SIM_SUCCESS for sending to the WifiMonitor */
#define FEATURE_SKY_WIFI_EAP_AKA_AT_NOTIFICATION_EAP_SIM_SUCCESS

/* EAP-AKA UMTS AUTH FAILED CM ERROR Memory Problem */
#define FEATURE_SKY_WIFI_EAP_AKA_UMTS_AUTHENTICATION_FAILED_CM_ERROR_MEMORY_PROBLEM

/* EAP-AKA AUTH PERIOD 30 SEC ==> 3 SEC */
#define FEATURE_SKY_WIFI_EAP_AKA_REDUCE_AUTH_PERIOD

/* EAP-AKA When SCARD: unexpected response 0x90 happened, skip */
#define FEATURE_SKY_WIFI_EAP_AKA_SCARD_UNEXPECTED_RESPONSE

/* If SCARD INIT failed, Retry again  */
#define FEATURE_SKY_WIFI_EAP_AKA_SCARD_INIT_FAIL

/* 3GPP REALM must be not added */
#define FEATURE_SKY_WIFI_EAP_AKA_NOT_USED_3GPP_REALM

/* PSEUDONYM REALM was used duplicatedly. So Prevent to overlap the pseudonym realm */
#define FEATURE_SKY_WIFI_EAP_AKA_DONT_USE_DUPLICATED_PSEUDONYM_REALM


#ifdef FEATURE_SKY_WIFI_VENDOR_KT
	/* KT EAP-AKA for sending the EAPOL-START message */
	#define FEATURE_SKY_WIFI_EAP_AKA_KT_BROADCAST_EAPOL_START

	/* KT EAP-AKA EAPOL-START message Destnation address is changed to bssid */
	#define FEATURE_SKY_WIFI_EAP_AKA_KT_EAPOL_START_DST_BSSID_ADDRESS

	/* KT EAP-AKA KUH UCA 0XCB PREFIX */
	#define FEATURE_SKY_WIFI_EAP_AKA_KT_KUH_UCA_0XCB_PREFIX

	//KT EAP-AKA Algorithm Mismatch, In the First Mismatch, Send NAK and From Second Mismatch, Send EAP CLIENT ERROR (12)
	#define FEATURE_SKY_WIFI_KT_EAP_AKA_ALGORITHM_MISMATCH

	//KT EAP-AKA,  If STA receviced AT_CHECK_CODE and AT_RESULT_IND, Send AT_RESULT_IND
	#define FEATURE_SKY_WIFI_KT_EAP_AKA_AT_CHECK_CODE_AT_RESULT_IND

	/* KT EAP-AKA When receiving 0x61 (unexpected response) from USIM, the process continues by normal. */
	#define FEATURE_SKY_WIFI_EAP_AKA_0X61_UNEXPECTED_RESPONSE

	/* KT Because of the ollehwifi app, not to try to connect in WPA_COMPLETED  state */
	#define FEATURE_WIFI_KT_OLLEHWIFI_APP_CONNECTION_FIX
	
	/* Auto Connection and Manual Connection */
	#define FEATURE_SKY_WIFI_ENABLED_MANUAL_CONNECTION
	
#endif

#endif /* FEATURE_SKY_WIFI_EAP_AKA_AUTHENTICATION */


#ifdef FEATURE_SKY_WIFI_WPA_SUPPLICANT
/* RSSI Threshold Setting */
#define FEATURE_SKY_WIFI_RSSI_THRESHOLD

/* Qualcomm Patch Country Code */
#define FEATURE_SKY_WIFI_QUALCOMM_PATCH_COUNTRY_CODE

/* WIFI Debug Log */
#define FEATURE_SKY_WIFI_LOG_LEVEL

/* 5Ghz Priority Connection */
#define FEATURE_SKY_WIFI_5GHZ_PRIORTY_CONNECTION

/* AP ROAMING */
//In android.config, Set CONFIG_NO_ROAMING=n and CONFIG_SEAMLESS_ROAMING=y
#define FEATURE_SKY_WIFI_SUPPLICANT_AP_ROAMING

/* ROAMING */
//In android.config, Set CONFIG_NO_ROAMING=n and CONFIG_SEAMLESS_ROAMING=y
#define FEATURE_SKY_WIFI_SUPPLICANT_ROAMING


/* When AP_SCAN was setted to 2, if WIFI Setting UI was entered, ap_scan was changed to 2. after rebooting, scan was not operated. */
#define FEATURE_SKY_WIFI_NO_CHANGE_AP_SCAN

#if defined (FEATURE_SKY_WIFI_VENDOR_SKT) || defined (FEATURE_SKY_WIFI_VENDOR_LGU)
	/* PREVENT SAVE SAME SSID */
	#define FEATURE_SKY_WIFI_PREVENT_SAVE_SAME_NETWORK
#endif

/* EAP PEAP MSCHAP2 DISCONNECT Problem */
#define FEATURE_SKY_WIFI_EAP_PEAP_MSCHAP2_DISCONNECT_FIX

//#if defined (FEATURE_SKY_WIFI_VENDOR_KT)
/* In WEP Shared, When happening ASSOC REJECT, Disconnect */
#define FEATURE_SKY_WIFI_SHARED_WEP_ASSOC_REJECT_DISCONNECT
//#endif

/* When happening ASSOC REJECT, Disconnect */
#define FEATURE_SKY_WIFI_ASSOC_REJECT_DISCONNECT

/* Don't skip Blacklisted bssid for avoiding the failure of connecting */
//#define FEATURE_SKY_WIFI_DONT_SKIP_BLACKLISTED_BSSID

/* Increase the buffer size of max Scan result 4096 bytes ==> 16384 bytes */
#define FEATURE_SKY_WIFI_INCREASE_MAX_SCAN_RESULTS

#ifdef FEATURE_SKY_WIFI_VENDOR_KT
	/* for seamless roaming in all the manual connection state*/
	#define FEATURE_SKY_WIFI_SEAMLESS_ROAMING
#endif	

/* We can know whether WPS Connection request or not in WifiMonitor */
#define FEATURE_SKY_WIFI_WPS_CONNECTION_REQUEST

/* The scan result of Empty SSID was not received in framework */
#define FEATURE_SKY_WIFI_GET_SCAN_RESULT_NO_EMPTY_SSID

/* When Config file was written and right reboot, wpa_supplicant.conf was deleted. So When Config file was written, we must do sync()  */
#define FEATURE_SKY_WIFI_CONFIG_FILE_WRITE_SYNC

#ifdef FEATURE_SKY_WIFI_VENDOR_KT
	/* When WPS is connecting, do not try connect to ollehWiFi */
	#define FEATURE_SKY_WIFI_WPS_CONNECTION_FIXED
#endif

/* Becaue scan period time is short, scan period time is increased */
#define FEATURE_SKY_WIFI_INSCREASE_SCAN_PERIOD_TIME

/* To backup the contents of wpa_supplicant.conf  in the cloud live app */
#define FEATURE_SKY_WIFI_SUPPORT_CLOUD_LIVE_API

/* WPA_SUPPLICANT Debug Messages for debugging */
#define FEATURE_SKY_WIFI_DEBUG_MESSAGE

/* Hotspot auto channel config */
#define FEATURE_SKY_WIFI_HOTSPOT_AUTO_CHANNEL_CONFIG

/*IOCTL Arp Offload */
#define FEATURE_WIFI_SET_ARP_OFFLOAD

/* TX power, Fer / Per, Channel Info */
#define FEATURE_WIFI_SKY_DEBUGSCREEN

/* IOCTL Set Keep alive */
#define FEATURE_SKY_WIFI_KEEPALIVE_SETTING

/* #define FEATURE_WIFI_WPS_MSG */

/* wpa_supplicant Patch for Qualcomm Driver 3.2.0.38 and Riva 120 */
#define FEATURE_WIFI_QUALCOMM_DRIVER_32038_PATCH
#endif /* FEATURE_SKY_WIFI_WPA_SUPPLICANT */


#ifdef FEATURE_SKY_WIFI_DIRECT_FEATURES

/* Pre-patch of P2p from hostapd site  http://hostap.epitest.fi/gitweb/gitweb.cgi?p=hostap.git;a=summary */
#define FEATURE_SKY_WIFI_DIRECT_ADVANDED_PATCH_FROM_HOSPAP

/* When p2p is connected, block the periodic scan for p2p throughput */
#define FEATURE_SKY_WIFI_PERMANT_SCAN_STOP_ON_P2P_CONNECTED

// minimum connect listen is 100 msec. try to connect for 1mimute.
#define FEATURE_SKY_WIFI_DIRECT_CONNECT_COMPATIBILITY_FOR_DONGLE

#define FEATURE_SKY_WIFI_DIRECT_QCOM_PATCH_P2P_TIMEOUT

/* Compatibility with broadcom bcm4331 (EF44S) */
#define FEATURE_SKY_WIFI_DIRECT_CONNECTION_COMPATIBILITY

// Qualcomm patch for MCC Concurrent
#define FEATURE_WIFI_DIRECT_ENABLE_MULTI_CHANNEL_CONCURRENT
#endif /* FEATURE_SKY_WIFI_DIRECT_FEATURES */


#ifdef FEATURE_SKY_WIFI_WPA_SUPPLICANT_SCAN

/* SSID EUC-KR and UTF-8 Auto Change */
#define FEATURE_SKY_WIFI_CHAR_TYPE_AUTO_CHANGE

//#define FEATURE_SKY_WIFI_CHAR_TYPE_AUTO_CHANGE_DEBUG_MESSAGE

/* get GWS Scan result */
#define FEATURE_SKY_WIFI_GET_SCAN_RESULT_KSH

#endif


// LG U+ Requirement
#ifdef FEATURE_SKY_WIFI_VENDOR_LGU

// LGU+ mwlan
/*  modified files - AdvancedWifiSettings.java (packages\apps\settings\src\com\android\settings\wifi)
                            Init.ef52l.rc (system\core\rootdir)
				SpecialCharSequenceMgr.java (packages\apps\phone\src\com\android\phone)
				Strings.xml (packages\apps\settings\res\values)
				Strings.xml (packages\apps\settings\res\values-ko)
				Wifi_advanced_settings.xml (packages\apps\settings\res\xml)
*/
#define FEATURE_DS_WIFI_LGU_MWLAN

// LGU+ toast msg
//modified files - WifiStateMachine.java (frameworks\base\wifi\java\android\net\wifi)
#define FEATURE_SKY_WIFI_LGU_Toast

// LGU+ toast msg, in case WiFi is connected to the open AP
//modified files - WifiStateMachine.java (frameworks\base\wifi\java\android\net\wifi)
#define FEATURE_SKY_WIFI_Open_toast

//Store that the history of WiFi connection in the DB (success or fail)
//modified files - WifiStateMachine.java (frameworks\base\wifi\java\android\net\wifi)
#define FEATURE_SKY_WIFI_LGU_DB

//Remove pseudonym ID and Reauth ID
#define FEATURE_SKY_WIFI_LGU_AKA_REMOVE_ID

#endif /* FEATURE_SKY_WIFI_VENDOR_LGU */


#endif/* __CUST_PANTECH_WIFI_H__ */

