#ifndef __CUST_PANTECH_DATA_LINUX_H__
#define __CUST_PANTECH_DATA_LINUX_H__
/* ========================================================================
FILE: cust_pantech_ds.h

Copyright (c) 2011 by PANTECH Incorporated.  All Rights Reserved.

USE the format "FEATURE_LGT_DS_XXXX"
=========================================================================== */ 

/*===========================================================================

                      EDIT HISTORY FOR FILE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

  when        	who     what, where, why
--------   	---      ----------------------------------------------------------
11/09/30    sjm   	  initial
11/12/22    phi   	  move to EF46L from EF65L. 
11/12/30    Alice     delete some of feature not yet adapted.
12/01/13    Alice     change file name. : Cust_pantech_data_linux.h -> CUST_PANTECH_DS.h
12/05/02    kns       move to EF50L from EF46L
12/10/26    Alice     re-arrange features
13/04/08    sjm      move to EF59L from EF52L

===========================================================================*/

/*===========================================================================*/
  //4!!ATTENTION!!
/*===========================================================================*/
/*------------------------------------------------------------------------------------

  1. must record history in this file header when you modify this file.

  2. FEEATUR's name start with "FEATURE_LGT_DS_xxx".
  
  3. each FEATURE is need to detailed description. because this file is instad of Feature Specification.
        - Item, Comment(Date, Author), Reason, Modified Files, Added Files, Deleted Files.

  4. In Java Code, Feature' exprssion is comment.
        - Exmaple. // FEATURE_LGT_DS_COMMON
        
  5. this file must be included CUST_PANTECH.h
        
--------------------------------------------------------------------------------------*/

/*===========================================================================
    Data Service Features
===========================================================================*/

/* 20120105 Alice : Common import, include.. etc. */
#define FEATURE_LGT_DS_COMMON

#ifdef FEATURE_LGT_DS_COMMON

/* -----------------------------------------------------------------------------------*/
    //3 Android & QCT Bug Fix
/*-------------------------------------------------------------------------------------*/

/* Item : startusingnetworkfeature()
   Commnet - 20120109 Alice, 20130219 Alice
	Reason - return value check before reconnect() belong to  startUsingNetworkFeature().
	                 -> do not return Phone.APN_REQUEST_STARTED unconditional.
	          - do not set IDLE when already try to set up data call.
	          
	Modified files - ConnectivityService.java (frameworks\base\services\java\com\android\server),
	                     MobileDataStateTracker.java (frameworks\base\core\java\android\net)
*/
#define FEATURE_LGT_DS_BUG_FIX_STARTUSINGNETWORKFEATURE

/* Item : Settings >> Data Usage
 	Commnet - 20130522 Alice
  	Reason - set to Data Usage >> Set mobile data limit >> Restrict background data,
                UID/SYS_UID's setting is processed one by one as android's source architetecture.
                on the way setting if user request App., occur ANR(Application Not Responsding).
  	           - merge from EF56S
 	Modified files - NetworkPolicyManagerService.java(frameworks\base\services\java\com\android\server\net)
*/
#define FEATURE_SKY_DS_BACKGROUND_RESTRICT_BUG_FIX

/* Item : IPTABLEV6
 	Commnet - 20120224 phi
  	Reason - /system/bin/iptables's permission is set to system permission. 
  	             on the other hands ip6tables's permission is set to shell permission.
  	             and change ip6tables's permission to system permission.
  	          - merge from EF45K
 	Modified files - android_filesystem_config.h (system\core\include\private)

 	-unnecessary feature
*/
#undef FEATURE_SKY_DS_IP6TABLE_UID_BUG_FIX


/* Item : Exception
 	Commnet - 20121217 Alice
  	Reason - silent reset
  	          
 	Modified files - NetworkPolicyManagerService.java (frameworks\base\services\java\com\android\server\net)
*/
#define FEATURE_LGT_DS_EXCEPTION_CATCH_BUG_FIX

/* Item : TCP Buffer
 	Commnet - 20121113 Alice
  	Reason - It doesn't exist TCP buffer size about EHRPD. 
  	             TCP Buffer size is choosen default size when connected to data in EHRPD.
  	             It's derived to change radio technology from EHRPD to LTE. because of keeping
  	             in Defualt TCP Buffer size.
  	          - modify to change TCP Buffer size

 	Modified files - MobileDataStateTracker.java (frameworks\base\core\java\android\net)
*/
#define FEATURE_SKY_DS_SET_TCPBUF_IN_RAT_CHANGE

/* Item : Data Connection
   Commnet - 20120726 kns
  	Reason - After anr or kill phone process, data connection isn't established.
  	          - From EF49K.
  Modified files - Qcril_data_netctrl.c(vendor\qcom\proprietary\qcril\common\data)
*/
#define FEATURE_SKY_DS_FOUND_DATA_CALL_AFTER_PHONE_PROCESS_RESTART

/* Item : UI
	Commnet - 20130225 Alice
	Reason - remove afterimage of popup window about Mobile data disabled
	Modified files - NetworkOverLimitActivity.java (frameworks\base\packages\SystemUI\src\com\android\systemui\net)
*/
#define FEATURE_LGT_DS_REMOVE_AFTERIMAGE

/* Item : Tethering
	Commnet - 20130318 Alice
	Reason - netd: Fix duplicate route error
              - Ignores error code 512 on duplicate route errors preventing dual interface tethering from working.
	
	Modified files - NatController.cpp(system\netd)
*/
#define FEATURE_LGT_DS_QC_PATCH_ROUTE_ERROR

/* Item : QOS
    Comment : 20130710 SJM
    Reason - Crash RilD when incoming call
               - Disalbe QOS

    Modifiled files - System.prop(device\qcom\msm8974)
*/               
#define FEATURE_LGT_DS_QOS_DISABLE

/* -----------------------------------------------------------------------------------*/
    //3    LGU+ Requirement
/*-------------------------------------------------------------------------------------*/

/* Item : CNE
 	Commnet - 20130513 Alice
 	Reason - Connectivity Engine = 4
   Modified files - System.prop(device\qcom\msm8974)
*/
#define FEATURE_LGT_DS_CNE

/* Item : DHCP
	Commnet - 20120227 Alice
	Reason - Since default DHCP Lease Time is 1 hour, 
	             VPN, RMNET and android Tethering are disconnected frequently. 
	             So, Increase the DHCP Lease Time to 7 days
	Modified files - TetherController.cpp (system\netd) 
*/
#define FEATURE_LGT_DS_INCREASE_DHCP_LEASETIME

/*Item : RestoreTimer
   Commnet - 20120618 Alice
   Reason - not used RestoreTimer : DUN, IMS
              - and HIPRI : Spec Out - but MQS Issue.
              - deleted FEATURE_LGT_DS_DISABLE_INACTIVITY_TIMER
              - concerned FEATURE_LGT_DS_LTE_MULTIPLE_APN

              - HIPRI : 5 min >> 10min
              
   Modified files - Config.xml(device\qcom\common\overlay\frameworks\base\core\res\res\values)
*/
#define FEATURE_LGT_DS_DISABLE_RESTORE_TIMER

/* Item : TCP sync retries
	Commnet - 20120104 Alice
	Reason - change RTO and TCP_SYN_RETRIES when TCP syn transfer
	           - tcp resync nymber is 5.(first transfer, 1s, 2s, 4s, 8s)
	Modified files - tcp.h(kernel\include\net)
*/
#define FEATURE_LGT_DS_TCP_SYN_RETRANSMIT

/* Item : Toast
	Commnet - 20121128 Alice
	Reason - show toast msg when try to fail over mobile
	          - except for airplane moe on
	Modified files - SkyDataConnectionTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony)
*/
#define FEATURE_LGT_DS_DATA_CONNECTION_TOAST

/* Item : Connectivity
 	Commnet - 20120215 Alice

 	Comment - 20130308 sjm
 	Remove Reason 1 &2
  	Reason - 1. only try to connect Data call when EVDO_REV_0/EVDO_REV_A 
  	                -> 1x Data call isn't possible.
  	             2. do not try to connect Data call IPV6 in LTE/EHRPD system.
  	                -> but possible aborad.

 	Modified files - GsmDataConnection.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\gsm),
                        GsmDataConnectionTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\gsm),
                        SkyDataConnectionTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony)

   Commnet - 20120730 sjm, 20121108 Alice, 20121211 Alice
   Reason - Check Data onoff in startUsingNetworkFeature()
              - replaced of FEATURE_LGT_DS_WPS_CHECK_DATA_ONOFF in EF50L ICS.
              - for CTS : android.net.cts.ConnectivityManagerTest
 
   Modified files - ConnectivityService.java (frameworks\base\services\java\com\android\server),
                        QcConnectivityService.java (frameworks\opt\connectivity\services\java),
                        ISkyDataConnection.aidl (frameworks\base\telephony\java\com\android\internal\telephony),
                        SkyDataConInterfaceManager.java (frameworks\opt\telephony\src\java\com\android\internal\telephony)
*/
#define FEATURE_LGT_DS_RESTRICT_DATA_CALL

/* Item : APN Changed
 	Commnet - 20120720_phi_DATA , 20121023 Alice
  	Reason - To notify ESM Error Cause in Deactivate EPS bearer context request msg to android Data Framework
                    
 	Modified files - GsmDataConnectionTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\gsm),
                        RIL.java (rameworks\base\telephony\java\com\android\internal\telephony),
                        RILConstants.java (frameworks\base\telephony\java\com\android\internal\telephony),
                        SkyDataConnectionTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony)
*/    
#define FEATURE_LGT_DS_NOTIFY_DEACT_BEARER_ERR_CAUSE_TO_FRAMEWORK

/*Item : Data Usage - Warning Level
   Commnet - 20130711 Alice
   Reason - changed data usage warning level : 2G -> 6G
              
   Modified files - Config.xml(frameworks\base\core\res\res\values)
*/
#define FEATURE_LGT_DS_NW_POLICY_WARNING_6G

/* Item : Handset Property
	Commnet - 20130813 Alice
	Reason - changed rmnet interface name
	
	Modified files - HandsetProperty.java (frameworks\base\core\java\android\lgt\handset)
*/
#define FEATURE_LGT_DS_RMNET_INTERFACE_NAME

/*......................................................................................................................................
  EasySetting, Data On/Off
.........................................................................................................................................*/

/* Item : Data On/off Property
	Commnet - 20121022 Alice
	Reason - Manage LGU+'s customized Data on/off property
	Modified files - DatabaseHelper.java (frameworks\base\packages\settingsprovider\src\com\android\providers\settings),
                        Settings.java (frameworks\base\core\java\android\provider),
                        ISkyDataConnection.aidl (frameworks\base\telephony\java\com\android\internal\telephony),
                        SkyDataConInterfaceManager.java (frameworks\opt\telephony\src\java\com\android\internal\telephony),
                        SkyDataConnectionTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony)
*/
#define FEATURE_LGT_DS_GET_SECUREDB_FOR_LGT

/* Item : EasySetting
 	Commnet - 20120509 Alice
 	Reason - match setting value between EasySetting ans Setting
 	
 	Modified files - ConnectivityManager.java (frameworks\base\core\java\android\net),
                        ConnectivityService.java (frameworks\base\services\java\com\android\server),
                        QcConnectivityService.java (frameworks\opt\connectivity\services\java),
                        IConnectivityManager.aidl (frameworks\base\core\java\android\net),
                        ISkyDataConnection.aidl (frameworks\base\telephony\java\com\android\internal\telephony),
                        SkyDataConInterfaceManager.java (frameworks\opt\telephony\src\java\com\android\internal\telephony),
                        SkyDataConnectionTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony)
*/
#define FEATURE_LGT_DS_EASY_SETTING

/* Item : Booting Pop-up
 	Commnet - 20120516 Alice, 20121214 Alice, 20130122 Alice
 	Reason - display data connection pop-up for user's choice when booting
 	          - to display only once. even silent reset
 	          - added theme(AlertDialog.THEME_DEVICE_DEFAULT_LIGHT)
 	          - added condition when return to domestic 
 	             concerned to AutoRadReceiver.java(packages\apps\Phone\src\com\android\phone)
 	
 	Modified files - CDMAPhone.java (frameworks\base\telephony\java\com\android\internal\telephony\cdma),
 	                     DatabaseHelper.java (frameworks\base\packages\settingsprovider\src\com\android\providers\settings),
                        DataConnectionTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony),
                        ISkyDataConnection.aidl (frameworks\base\telephony\java\com\android\internal\telephony),
                        Public.xml (frameworks\base\core\res\res\values),
                        Settings.java (frameworks\base\core\java\android\provider),
                        SkyDataConInterfaceManager.java (frameworks\opt\telephony\src\java\com\android\internal\telephony),
                        SkyDataConnectionTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony),
                        System.prop(device\qcom\msm8974)

	Added files - Strings_ds.xml(frameworks\base\core\res\res\values),
	                  Strings_ds.xml(frameworks\base\core\res\res\values-ko),
                     Strings_ds.xml (frameworks\base\core\res\res\values-zh-rcn),
                     Strings_ds.xml (frameworks\base\core\res\res\values-zh-rtw)
*/
#define FEATURE_LGT_DS_BOOTING_POPUP

/* Item : Toast
 	Commnet - 20120517 Alice
 	Reason - Show Data Connection and Disconnection.
 	
 	Modified files - DataConnectionTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony),
                        Intent.java (frameworks\base\core\java\android\content),
                        SkyDataConnectionTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony)
*/
#define FEATURE_LGT_DS_TOAST

/* Item : BIP
 	Commnet - 20130808 SJM
 	Reason - Do not allow data call using EMPTY SIM
 	
 	Modified files - SkyDataConnectionTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony)
*/
#define FEATURE_LGT_DS_NOT_ALLOW_DATA_CALL_USING_EMPTY_SIM

/* Item : IPv6 Address Asignment
     Comment - 20130809 SJM 
     Modified files - addrconf.c (kernel\net\ipv6)
*/
#define FEATURE_LGT_DS_OPTIMIZE_IPV6_ASSIGNMENT

/* Item : Prevent device from crashing
    Comment : 20131205 BKY
    Reason - Device crashed when optimizing IPv6 address assignment which means reducing assignment duration.
    				It's crashed because 'ifa->idev' is null at 'if6_get_first' function.
    				
	Related FEATURE - FEATURE_LGT_DS_OPTIMIZE_IPV6_ASSIGNMENT
				How to reduce?
			    	- set NO DAD for link local address
    				- shrink DAD timer to 50 ms for global address.
    				
    Solution - spin_lock and spin_unlock points are adjusted and adding.
    Related issues - EF59L_JB PLM 01708 / EF62L_JB PLM 00836

    Modifiled files - addrconf.c (kernel\net\ipv6)
*/               
#define FEATURE_LGT_DS_OPTIMIZE_IPV6_ASSIGNMENT_CRASH_FIX

/*......................................................................................................................................
  QMI
.........................................................................................................................................*/

/* Item : QMI
	Commnet - 20120426 Alice
	Reason - Added for communication between Modem and Linux
	          - Just only use for Data Service.
	          - must be adpated to pair with Linux.
	
	Modified files - Android.mk (android\frameworks\opt\telephony),
	                    Core.mk (android\build\target\product)
	          
	Added files - LINUX\android\pantech\frameworks\qmi_data

   Commnet - 20120503 Alice
   Reason - not only IDL QMI but also Legacy QMI
             -  use Legacy QMI, must block the compile option flags.
                (LOCAL_CFLAGS += -DQCCI_OVER_QMUX )
             - added permission for failure socket
             
   Modified files - Android.mk (vendor\qcom\proprietary\qmi-framework\qcci\src),
                        //ipc_socket.c(kernel\arch\arm\mach-msm)
*/
#define FEATURE_LGT_DS_QMI

                        
/*......................................................................................................................................
  Hidden Menu : Enginerring, Debug screen, etc.
.........................................................................................................................................*/

/* Item : IP Addr, DNS Addr
	Commnet - 20120527 kns, 20101026 Alice
	Reason - display IP Addr in Debug Screen.
	Modified files - LteScreen.java (vendor\qcom\proprietary\pantech_ps_ril\gwlhiddenmenu\src\com\android\hiddenmenu)
	                        Lte_screen.xml (vendor\qcom\proprietary\pantech_ps_ril\gwlhiddenmenu\res\layout)
	                        Strings_cp.xml (vendor\qcom\proprietary\pantech_ps_ril\gwlhiddenmenu\res\values)

	Comment - 20131017_yunsik_DATA, added for wcdma
    Modified files -    WcdmaScreen.java (vendor\qcom\proprietary\pantech_ps_ril\gwlhiddenmenu\src\com\android\hiddenmenu)
                           Wcdma_screen.xml (vendor\qcom\proprietary\pantech_ps_ril\gwlhiddenmenu\res\layout)
                           Strings_cp.xml (vendor\qcom\proprietary\pantech_ps_ril\gwlhiddenmenu\res\values)

	Comment - 20131017_yunsik_DATA, added DNS address (LTE/WCDMA)
    Modified files - ISkyDataConnection.aidl (frameworks\base\telephony\java\com\android\internal\telephony)
                            SkyDataConInterfaceManager.java (frameworks\opt\telephony\src\java\com\android\internal\telephony)
                            LteScreen.java (vendor\qcom\proprietary\pantech_ps_ril\gwlhiddenmenu\src\com\android\hiddenmenu)
                            Lte_screen.xml (vendor\qcom\proprietary\pantech_ps_ril\gwlhiddenmenu\res\layout)
                            Strings_cp.xml (vendor\qcom\proprietary\pantech_ps_ril\gwlhiddenmenu\res\values)
                            WcdmaScreen.java (vendor\qcom\proprietary\pantech_ps_ril\gwlhiddenmenu\src\com\android\hiddenmenu)
                            Wcdma_screen.xml (vendor\qcom\proprietary\pantech_ps_ril\gwlhiddenmenu\res\layout)
*/
#define FEATURE_LGT_DS_MULTIPDN_DBGSCN

/* Item : Data Service Hidden Menu
	Commnet - 20130724 Alice, 20131220 Alice
	Reason - added Hidden menu for Data Service
	           - added NSRM menu
	
	Modified files - AndroidManifest.xml (packages\apps\settings),
                        SpecialCharSequenceMgr.java (packages\apps\contacts\src\com\android\contacts),
                        SpecialCharSequenceMgr.java (packages\apps\phone\src\com\android\phone)

   Added files -    SkyDataHiddenSettings.java (packages\apps\settings\src\com\android\settings\data),
                        SkyDataNSRM.java (packages\apps\settings\src\com\android\settings\data),
                        SkyDataReceiver.java (packages\apps\settings\src\com\android\settings\data),
                        Sky_data_hidden.xml (packages\apps\settings\res\layout),
                        Sky_data_hidden.xml (packages\apps\settings\res\xml),
                        Sky_data_nsrm.xml (packages\apps\settings\res\xml),
                        Strings_ds.xml (packages\apps\settings\res\values)
*/
#define FEATURE_LGT_DS_HIDDEN_MENU

#ifdef FEATURE_LGT_DS_HIDDEN_MENU
/* Item : Alwayson Setting
 	Commnet - 20120128 Alice
  	Reason - added AlwasyOn/Off API
 	Modified files -  SkyDataHiddenSettings.java (packages\apps\settings\src\com\android\settings\data),
 	                      Sky_data_hidden.xml (packages\apps\settings\res\xml),
 	                      Strings_ds.xml (packages\apps\settings\res\values)
*/
#define FEATURE_LGT_DS_ALWAYS_ON

#ifdef FEATURE_LGT_DS_QMI
/* Item : Equipment Test
	Commnet - 20130816 Alice
	Reason - used LGU+ USIM in equipment network
	           - send QMI command for change Modem's configuration
              - disable dns check in equipment network
   Modified files - SkyDataHiddenSettings.java (packages\apps\settings\src\com\android\settings\data),
                        Sky_data_hidden.xml (packages\apps\settings\res\xml),
                        QmiDataClnt.cpp (pantech\frameworks\qmi_data\jni),
                        QmiDataClnt.java (pantech\frameworks\qmi_data\java\com\pantech\qmidata),
                        Qmi_data_clnt.c (pantech\frameworks\qmi_data\qmi),
                        Qmi_data_def.h (pantech\frameworks\qmi_data\qmi),
                        Strings_ds.xml (packages\apps\settings\res\values),
                        ISkyDataConnection.aidl (frameworks\base\telephony\java\com\android\internal\telephony)
                        SkyDataConInterfaceManager.java (frameworks\opt\telephony\src\java\com\android\internal\telephony)
                        
*/
#define FEATURE_LGT_DS_EQUIPMENT_TEST

/* Item : ROHC
	Commnet - 20130820 Alice
	Reason - ROHC ON/OFF

   Modified files - SkyDataHiddenSettings.java (packages\apps\settings\src\com\android\settings\data),
                        Sky_data_hidden.xml (packages\apps\settings\res\xml),
                        QmiDataClnt.cpp (pantech\frameworks\qmi_data\jni),
                        QmiDataClnt.java (pantech\frameworks\qmi_data\java\com\pantech\qmidata)
                        Qmi_data_clnt.c (pantech\frameworks\qmi_data\qmi),
                        Qmi_data_def.h (pantech\frameworks\qmi_data\qmi),
                        Strings_ds.xml (packages\apps\settings\res\values)
*/
#define FEATURE_LGT_DS_ROHC_ONOFF

/* Item : APN
	Commnet - 20130820 Alice, 20130906 hongss
	Reason - change IP type at modem's profile

   Modified files - SkyDataHiddenSettings.java (packages\apps\settings\src\com\android\settings\data),
                        Sky_data_hidden.xml (packages\apps\settings\res\xml),
                        Strings_ds.xml (packages\apps\settings\res\values)
                        //hongss
                        Qdp.c (vendor\qcom\proprietary\data\qdp\src)  
*/
#define FEATURE_LGT_DS_CHANGE_IP_TYPE

#endif /* FEATURE_LGT_DS_QMI */
#endif /* FEATURE_LGT_DS_HIDDEN_MENU */

/* -----------------------------------------------------------------------------------*/
    //3    LGU+ Requirement :: Multiple APN
/*-------------------------------------------------------------------------------------*/

/* Item : APN List
	Commnet - 2012
	Reason - added multiple apn
	Modified files - Apns.xml (frameworks\base\core\res\res\xml),
                        Config.xml(device\qcom\common\overlay\frameworks\base\core\res\res\values),
                        Config.xml(device\qcom\msm8974\overlay\frameworks\base\core\res\res\values)
*/
#define FEATURE_LGT_DS_LTE_MULTIPLE_APN

/* Item : added to tethering apn
	Commnet - 20120206 hongss
	Reason - Multiple PDN??Tethering ��??��??��? Tethering APN��?? 

	Commnet - 20130730 sjm, Alice
	Reason - added bluetooth type for JB 4.2.2
	
	Modified files - Config.xml (device\qcom\common\overlay\frameworks\base\core\res\res\values),
  	                     Config.xml(device\qcom\msm8974\overlay\frameworks\base\core\res\res\values),
                        ConnectivityManager.java (frameworks\base\core\java\android\net),
                        ConnectivityService.java (frameworks\base\services\java\com\android\server)                        
                        QcConnectivityService.java (frameworks\opt\connectivity\services\java),
                        DataConnectionTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony),
                        GsmDataConnectionTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\gsm),
                        MobileDataStateTracker.java (frameworks\base\core\java\android\net),
                        Phone.java (frameworks\base\telephony\java\com\android\internal\telephony),
                        RILConstants.java (frameworks\base\telephony\java\com\android\internal\telephony),
                        SkyDataConnectionTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony),
                        Tethering.java (frameworks\base\services\java\com\android\server\connectivity)
*/
#define FEATURE_LGT_LTE_EHRPD_MULTIPLE_APN_TETHER

/* Item : Enable using default (TYPE_MOBILE) APN type at overseas with overseas USIM card.
	Commnet - 20140103 BKY
	Reason - The client can not use internet where abroad with overseas USIM card.
	Modified files - Tethering.java (frameworks\base\services\java\com\android\server\connectivity)
*/
#define FEATURE_LGT_LTE_EHRPD_MULTIPLE_APN_TETHER_FIX_BUG

/* Item : IMS
	Commnet - 20120620 sjm, 20121029 Alice
	Reason - Always on for IMS/MMS/EMERGENCY
	          - replaced of FEATURE_LGT_DS_IMS_MMS_ALWAYS_ON
	          - replaced of FEATURE_LGT_DS_IMS_MMS_EMERGENCY_ALWAYS_ON 20131128 by sjm
	          
   Modified files - ConnectivityService.java (frameworks\base\services\java\com\android\server),
                        QcConnectivityService.java (frameworks\opt\connectivity\services\java),
                        GsmDataConnectionTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\gsm),
                        MobileDataStateTracker.java (frameworks\base\core\java\android\net),
                        SkyDataConInterfaceManager.java (frameworks\opt\telephony\src\java\com\android\internal\telephony),
                        SkyDataConnectionTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony)
*/
#define FEATURE_LGT_DS_IMS_MMS_EMERGENCY_PHOTORING_ALWAYS_ON

/* Item : IMS
  Commnet - 20120704 sjm
  Reason - In case IMS registration started before GsmDataConnectionTracker is not ready 
               (e.g. mAllApnList is not created yet) 
               so IMS apn request failed, GsmDataConnectionTracker should retry IMS apn request. 
               
   Modified files - GsmDataConnectionTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\gsm)
*/
#define FEATURE_LGT_DS_IMS_APN_REQ_RETRY

/* Item : Retry algorithm
    Commnet - 20120712 sjm, 20121024 Alice
    Reason - Req. for mpdn retry algoritm.
               
    Modified files - DataConnection.java (frameworks\opt\telephony\src\java\com\android\internal\telephony),
                         GsmDataConnectionTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\gsm),
                         SkyDataConnectionTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony),
                         Public.xml (frameworks\base\core\res\res\values),
                         Ril.h (hardware\ril\include\telephony),
                         Strings_ds.xml (frameworks\base\core\res\res\values),
                         Strings_ds.xml (frameworks\base\core\res\res\values-ko),
                         Strings_ds.xml (frameworks\base\core\res\res\values-zh-rcn),
                         Strings_ds.xml (frameworks\base\core\res\res\values-zh-rtw)
*/
#define FEATURE_LGT_DS_MPDN_RETRY_ALGORITHM

/* Item : Tethering
    Commnet - 20120719 sjm, 20130107 Alice
    Reason - Req. for mdpn tethering
              - show Notification when enable tethering in data off.
              - modified BigTextStyle : MQS No.6

    Modified files - Tethering.java (frameworks\base\services\java\com\android\server\connectivity)
*/
//unnecessariness feature becaure LGU+ request is changed. #define FEATURE_LGT_DS_TETHERING_ERROR_POPUP

/* Item : IMS
    Commnet - 20120807 sjm, 20121025 Alice
    Reason - LGU+ Requirement for reset retry counter

    Modified files - SkyDataConnectionTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony)
*/
#define FEATURE_LGT_DS_RESET_RETRY_COUNTER

/* Item : HotSpot
    Commnet - 20120808 sjm, 20131113 Alice
    Reason - Hotspot disabled when data off
              - added condition about data on/off in roaming

    Modified files - SkyDataConnectionTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony)
*/
#define FEATURE_LGT_DS_TETHERING_HOTSPOT_OFF

/* Item : IMS/MMS
    Commnet - 20120816 sjm
    Reason - IMS/MMS type doesn't reflect data usage.

    Modified files - Config.xml(device\qcom\common\overlay\frameworks\base\core\res\res\values),
                         Config.xml(device\qcom\msm8974\overlay\frameworks\base\core\res\res\values)
*/
#define FEATURE_LGT_DS_IMS_DATA_USAGE

/* Item : IMS/MMS/EMERGENCY
    Commnet - 20120816 sjm, 20130521 Alice
    Reason - IMS/MMS/EMERGENCY type doesn't display data activity icon.

    Modified files - DataConnectionTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony),
                         SkyDataConnectionTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony)
*/
#define FEATURE_LGT_DS_IMS_MMS_EMREGENCY_PHOTORING_DATA_ACTIVITY

/* Item : IMS
    Commnet - 20120828 sjm, 20121025 Alice
    Reason - Error return for ims when imei is null or empty

    Modified files - Phone.java (frameworks\base\telephony\java\com\android\internal\telephony),
                         SkyDataConInterfaceManager.java (frameworks\opt\telephony\src\java\com\android\internal\telephony)
*/
#define FEATURE_LGT_DS_IMS_ERROR_RETURN

/* Item : IMS
    Commnet - 20120313 sjm, 20121029 Alice
    Reason - For hide data icon when ims apn only activated.

    Modified files - ISkyDataConnection.aidl (frameworks\base\telephony\java\com\android\internal\telephony),
                         NetworkController.java (frameworks\base\packages\systemui\src\com\android\systemui\statusbar\policy),
                         SkyDataConInterfaceManager.java (frameworks\opt\telephony\src\java\com\android\internal\telephony)
*/
#define FEATURE_LGT_LTE_EHRPD_MULTIPLE_APN_IMS_DATA_ICON_VISIBLE

/* Item : VoLTE PCSCF 
    Comment - 20130510 BKY
                 - removed code for initializing P-CSCF address because of side effect
                    If P-CSCF address is initialized, you have to initialize P-CSCF address after detach
                 - replaced of FEATURE_LGT_DS_VOLTE_PCSCF_ADDR
                 - adding PCSCF3, PCSCF4 in case of connecting to emergency PDN. (7/29)
                    
    Comment - 20130510 BKY
    Reason - To save P-CSCF address for IMS registration by netmgr

    Modified files - netmgr.h () (vendor\qcom\proprietary\data\netmgr\inc)
    						netmgr_defs.h (vendor\qcom\proprietary\data\netmgr\src)
    						netmgr_kif.c (vendor\qcom\proprietary\data\netmgr\src)
    						netmgr_qmi.c (vendor\qcom\proprietary\data\netmgr\src)
    						HandsetProperty.java (frameworks\base\core\java\android\lgt\handset)							
*/    
#define FEATURE_LGT_DS_VOLTE_PCSCF_ADDR_BY_NETMGR

/* Item : IMS
   Commnet - 20120912 sjm, 20121025 Alice, 20130912 sjm
   Reason - Do not retry data connection for ims

             - added EMERGENCY
             - replaced fo FEATURE_LGT_DS_IMS_RETRY

   Modified files - GsmDataConnectionTracker.java (rameworks\base\telephony\java\com\android\internal\telephony\gsm)
*/
#define FEATURE_LGT_DS_IMS_MMS_EMERGENCY_RETRY

/* Item : IMS
  Commnet - 20121119 Alice
  Reason - prevent to retry IMS in framework layer. becauce VT App.'s 

   Modified files - SkyDataConnectionTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony)
*/
#define FEATURE_LGT_DS_PREVENT_TO_RETRY_IMS

// TODO: ??
/* Item : Add mutex protection to serialize DHCP operations.
     Commnet - 20130218 bky
	 If two DHCP threads are going on in parallel, they call into
	 ifc_utils library which is not thread safe. As soon as the first
	 DHCP completes, the ifc_utils control socket is also closed,
	 thus causing issues during the second link setup. This patch
	 serializes the DHCP operations for different threads.
   
  	 Reason - CRs-Fixed 413480
  	 
     Modified files - netmgr_kif.c (vendor\qcom\proprietary\data\netmgr\src)
     						 netmgr_main.c (vendor\qcom\proprietary\data\netmgr\src)
*/
#define FEATURE_LGT_DS_NETMGR_ADD_MUTEX

/* Item : EMERGENCY APN TYPE
   Commnet - 20130515 sjm
   Reason - add Emergency apn type for Emergency PDN

   Modified files - Config.xml (frameworks\base\core\res\res\values),
                       ConnectivityManager.java (frameworks\base\core\java\android\net),
                       ConnectivityService.java (frameworks\base\services\java\com\android\server),
                       DataConnectionTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony),
                       DctConstants.java (frameworks\base\telephony\java\com\android\internal\telephony),
                       GsmDataConnectionTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\gsm),
                       MobileDataStateTracker.java (frameworks\base\core\java\android\net),
                       Phone.java (frameworks\opt\telephony\src\java\com\android\internal\telephony),
                       PhoneConstants.java (frameworks\base\telephony\java\com\android\internal\telephony),
                       QcConnectivityService.java (frameworks\opt\connectivity\services\java),
                       RILConstants.java (frameworks\base\telephony\java\com\android\internal\telephony),
                       SkyDataConnectionTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony)
*/
#define FEATURE_LGT_LTE_EMERGENCY_APN

/* Item : Photoring APN TYPE
   Commnet - 20131128 sjm
   Reason - add Photoring type for Internet PDN

   Modified files - Config.xml (frameworks\base\core\res\res\values),
                       ConnectivityManager.java (frameworks\base\core\java\android\net),
                       ConnectivityService.java (frameworks\base\services\java\com\android\server),
                       DataConnectionTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony),
                       DctConstants.java (frameworks\base\telephony\java\com\android\internal\telephony),
                       GsmDataConnectionTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\gsm),
                       MobileDataStateTracker.java (frameworks\base\core\java\android\net),
                       Phone.java (frameworks\opt\telephony\src\java\com\android\internal\telephony),
                       PhoneConstants.java (frameworks\base\telephony\java\com\android\internal\telephony),
                       QcConnectivityService.java (frameworks\opt\connectivity\services\java),
                       RILConstants.java (frameworks\base\telephony\java\com\android\internal\telephony),
                       SkyDataConnectionTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony)   
*/
#define FEATURE_LGT_DS_PHOTORING_TYPE

#ifdef FEATURE_LGT_DS_QMI
/* Item : Emergency Call Noti to Modem
    Comment - 20130801 SJM
    Reason - Attch with Emergnecy APN when dialing emergency call during airplane mode or not attached state
    
    Modified Files - ISkyDataConnection.aidl (frameworks\base\telephony\java\com\android\internal\telephony),
                         QmiDataClnt.cpp (pantech\frameworks\qmi_data\jni),
                         QmiDataClnt.java (pantech\frameworks\qmi_data\java\com\pantech\qmidata),
                         Qmi_data_clnt.c (pantech\frameworks\qmi_data\qmi),
                         Qmi_data_def.h (pantech\frameworks\qmi_data\qmi),
                         SkyDataConInterfaceManager.java (frameworks\opt\telephony\src\java\com\android\internal\telephony)
*/
#define FEATURE_LGT_DS_BIP_OTA_EMERGENCY_CALL

/* Item : Emergency Call Reject Noti to Modem and App.
    Comment - 20130813 SJM
    Reason - mode change to WCDMA due to emergnecy pdn reject
               - Reject Cause set to 256
    
    Modified Files - QmiDataClnt.cpp (pantech\frameworks\qmi_data\jni),
                           QmiDataClnt.java (pantech\frameworks\qmi_data\java\com\pantech\qmidata),
                           Qmi_data_clnt.c (pantech\frameworks\qmi_data\qmi),
                           Qmi_data_def.h (pantech\frameworks\qmi_data\qmi),
                           RIL.java (rameworks\base\telephony\java\com\android\internal\telephony),
                           SkyDataConnectionTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony)
                           Intent.java(frameworks\base\core\java\android\content)
*/
#define FEATURE_LGT_DS_EMERGENCY_CALL_REJECT

/* Item : Process ODB in Attach Reject
    Comment - 20130822 SJM
    Reason - Reject Cause set to 257
    
    Modified Files - QmiDataClnt.cpp (pantech\frameworks\qmi_data\jni),
                           QmiDataClnt.java (pantech\frameworks\qmi_data\java\com\pantech\qmidata),
                           Qmi_data_clnt.c (pantech\frameworks\qmi_data\qmi),
                           Qmi_data_def.h (pantech\frameworks\qmi_data\qmi),
                           RIL.java (rameworks\base\telephony\java\com\android\internal\telephony),
                           SkyDataConnectionTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony)
                           Intent.java(frameworks\base\core\java\android\content)
*/
#define FEATURE_LGT_DS_ATTACH_REJECT_BY_ODB

/* Item : Setting to disconnect Default PDN
    Comment - 20130827 KYJ
    Reason - set Modem flag to disconnect Default PDN(IMS).

    Modified Files -SkyDataConInterfaceManager.java (msm8974_jb\linux\android\frameworks\opt\telephony\src\java\com\android\internal\telephony)
                          QmiDataClnt.java (msm8974_jb\linux\android\pantech\frameworks\qmi_data\java\com\pantech\qmidata)
                          QmiDataClnt.cpp (msm8974_jb\linux\android\pantech\frameworks\qmi_data\jni)
                          Qmi_data_clnt.c (msm8974_jb\linux\android\pantech\frameworks\qmi_data\qmi)
                          Qmi_data_def.h (msm8974_jb\linux\android\pantech\frameworks\qmi_data\qmi)
*/
#define FEATURE_LGT_DS_DISCONNECT_DEFAULT_PDN_FOR_IMS
#endif

/* Item : Reset ODB by Detach
    Comment - 20130822 SJM
    Reason - Reject Cause set to 255
    
    Modified Files - RIL.java (rameworks\base\telephony\java\com\android\internal\telephony),
                           SkyDataConnectionTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony)
                           Intent.java(frameworks\base\core\java\android\content)
*/
#define FEATURE_LGT_DS_RESET_BARRING_DETACH

/* -----------------------------------------------------------------------------------*/
    //3    LGU+ Requirement :: NSWO
/*-------------------------------------------------------------------------------------*/

/* Item : NSWO
    Commnet - 2012.11.01 sjm,  20130904 Alice
    Reason - LGU+ HO Client porting

    Modified files - ConnectivityService.java (frameworks\base\services\java\com\android\server),
                         QcConnectivityService.java (frameworks\opt\connectivity\services\java),
                         SkyDataConnectionTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony),
                         Core.mk (android\build\target\product)
*/
#define FEATURE_LGT_DS_NSWO

/* -----------------------------------------------------------------------------------*/
    //3    LGU+ Requirement :: NSRM Agent
/*-------------------------------------------------------------------------------------*/

/* Item : NSRM Agent
    Commnet - 2013.8.15 CHANGRYUL
    Reason - LGU+ NSRM AGENT porting

    Modified files - property_service.c (system\core\init),
                         system.prop (device\qcom\msm8974),
                         init.pantech.preload.sh(device\qcom\common\rootdir\etc)
                         Android.mk(\device\qcom\common\rootdir)
                         Buildinfo.sh(\build\tools)
                         init_EF59L.rc(build\tools)


    Added files    - nsrm_client.apk (pantech/apps/LGUNsrmAgent/pre-built),
                         NsrmConfiguration.xml(pantech/apps/LGUNsrmAgent),
                         Android.mk(pantech/apps/LGUNsrmAgent)
                         Init.pantech.nsrm.sh(\device\qcom\common\rootdir\etc)

    Commnet - 20130108_P10624_PS4_DATA
    Reason - added package name for deactivating "Disable" button
               - merged from EF59L
    
    Modified files - SKYSystem.java (pantech\frameworks\skysettings\java\com\pantech\providers\skysettings)

    Commnet - 20140102 Alice
    Reason - added property for NsrmConfiguration.xml version
               - enabled NSRM : persist.sys.cnd.nsrm 1 -> 2
               - added madatory field <EAQSRT>
              
    Modified files - Buildinfo.sh(\build\tools),
                         init_EF59L.rc (system\core\rootdir),
                         Init.pantech.nsrm.sh(\device\qcom\common\rootdir\etc),
                         NsrmConfiguration.xml(pantech/apps/LGUNsrmAgent)
*/
#define FEATURE_LGT_DS_NSRM_AGENT

/* -----------------------------------------------------------------------------------*/
    //3    LGU+ Requirement :: Roaming
/*-------------------------------------------------------------------------------------*/

/* Item : Roaming Data Connection
 	Commnet - 20120315 Yoonjunho
  	Reason - Show roaming data connection dialog box when we are in roaming area
 	Modified files - DataConnectionTracker.java, GsmDataConnectionTracker.java, ISkyDataConnection.aidl, SkyDataConInterfaceManager.java
 	               - Strings_ds.xml (linux\android\vendor\pantech\overlay\frameworks\base\core\res\res\values\)
 	               - Strings_ds.xml (linux\android\vendor\pantech\overlay\frameworks\base\core\res\res\values-ko\)
*/
#define FEATURE_LGT_DS_ROAMING_DATA_MENU_POPUP

/* Item : Roaming Data Connection
 	Commnet - 20120315 Yoonjunho
  	Reason - Check whether PS domain attachment is rejected or not when we are in roaming area
 	Modified files - DataConnectionTracker.java, GsmDataConnectionTracker.java
*/
#define FEATURE_LGT_DS_PS_REJECT

/* Item : Roaming Data Connection
 	Commnet - 20120315 Yoonjunho
  	Reason - Disable data connection menu in easy setting when we are in roaming area
 	Modified files - GsmDataConnectionTracker.java, GsmServiceStateTracker.java
*/
#define FEATURE_LGT_DS_ALWAYSON_MENU_DISABLED_IN_ROAMING

/* Item : Data Connection
 	Commnet - 20120406 Yoonjunho
  	Reason - ISkyDataConnection??3rd parth app?��? ?��????��???��?�� ?��????��??????��? API ��?��?
 	Modified files - SkyDataConInterfaceManager.java, ISkyDataConnection.aidl, DataConnectionTracker.java
*/ 
#define FEATURE_LGT_DS_NET_OVERLIMIT_API

/* Item : Roaming Data Connection
 	Commnet - 20120510 Yoonjunho
  	Reason - Change default APN with "wroaming.lguplus.co.kr" when we are in roaming area
 	Modified files - GsmDataConnectionTracker.java, DataConnectionTracker.java, Qcril_data_netctrl.c.
*/
#define FEATURE_LGT_DS_ROAMING_APN_CHANGE

/* Item : Roaming Data Connection
 	Commnet - 20121207 Eomhyunwoo
  	Reason - When data roaming is disabled in overseas roaming area, popup message is displayed; "Unable to set Tethering_hotspot"
 	Modified files - Tethering.java
*/
#define FEATURE_LGT_DS_ROAMING_Tethering_hotspot_ERROR_POPUP

/* Item : Roaming Data Connection
 	Commnet - 20121213 Eomhyunwoo
  	Reason - default setting of data roaming is "false" (APQ8064\build\target\product\full_base_telephony.mk)
 	Modified files - full_base_telephony.mk
*/
#define FEATURE_LGT_DS_ROAMING_DEFAULT_SETTING

/* Item : Roaming Data Connection
 	Commnet - 20120510 Yoonjunho
  	Reason - send PDP reject popup
 	Modified files - GsmDataConnectionTracker.java, strings_ds.xml,public.xml

 	Commnet -  20131031_yunsik_DATA
  	Reason - changed text 
  	                and DO NOT remove notification. If the data connection succeeds, it will be disappear (new UI req. 77page)
 	Modified files - android\frameworks\base\core\res\res\values\Strings_ds.xml
 	                        android\frameworks\base\core\res\res\values-ko\Strings_ds.xml
 	                        android\frameworks\opt\telephony\src\java\com\android\internal\telephony\gsm\GsmDataConnectionTracker.java
*/
#define FEATURE_LGT_DS_PDP_REJECT_POPUP

/* Item : Roaming Data Connection
 	Commnet - 20120510 Yoonjunho
  	Reason - Display Background selection popup in alarm window
 	Modified files - DataConnectionTracker.java, GsmDataConnectionTracker.java,NetworkPolicyManagerService.java, strings_ds.xml,public.xml
*/
#define FEATURE_LGT_DS_ROAMING_ALARM_RESTRICT_BACKGROUND_DATA

/* Item : Roaming Data Connection
 	Commnet - 20130320 ParkMinOh
  	Reason - MOBILE_DATA and DATA_ROAMING have different usage with Original, So make seperate usage 
 	Modified files - GsmDataConnectionTracker.java
*/
#define FEATURE_LGT_DS_DIFFERENT_DARA_ROAMING

/* Item : LTE Roaming APN
    Commnet - 20130521 SJM
    Reason - added LTE Romaing APN Controll
    Modified files -Apns.xml (frameworks\base\core\res\res\xml)
                       ISkyDataConnection.aidl (frameworks\base\telephony\java\com\android\internal\telephony),
                       GsmDataConnectionTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\gsm),
                       SkyDataConInterfaceManager.java (frameworks\opt\telephony\src\java\com\android\internal\telephony),
                       SkyDataConnectionTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony)
*/
#define FEATURE_LGT_DS_LTE_ROAMING_APN

/* Item : Oversea UICC
    Commnet - 20130910 SJM
    Reason - Support permant fail like roaming
    Modified files -GsmDataConnectionTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\gsm),
                         SkyDataConnectionTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony)
*/
#define FEATURE_LGT_DS_OVERSEACOUNTRY_COMMON_DATA

/* -----------------------------------------------------------------------------------*/
    //3 Issue Follow up
/*-------------------------------------------------------------------------------------*/

/* Item : DCT
	Commnet - 20120831 Alice
	Reason - Added additional DCT for Pantech features.

	Modified files - CdmaDataConnectionTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\cdma),
                        DataConnectionTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony),
                        GsmDataConnectionTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\gsm),
				           
	Added files - SkyDataConnectionTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony)
*/
#define FEATURE_LGT_DS_ADDITIONAL_DCT

/* Item : Data Connection Interface, AIDL
	Commnet - 20120831 Alice
	Reason - Added DCT interface for other service.

	Modified files - Android.mk(frameworks\base), 
			              service_manager.c(frameworks\base\cmds\servicemanager),
				           DataConnectionTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony)
				           
	Added files - ISkyDataConnection.aidl (frameworks\base\telephony\java\com\android\internal\telephony),
			            SkyDataConInterfaceManager.java (frameworks\opt\telephony\src\java\com\android\internal\telephony)
*/
#define FEATURE_LGT_DS_ADD_DATA_AIDL

/* Item : Max windowsize/memsize for LTE
 	Commnet - 20120105 Alice, 20120202 Alice
 	Reason - change max windowsize/memsize for LTE
 	          -  FEATURE_SKY_DS_SET_TCPBUF_IN_RAT_CHANGE

 	Commnet - 20130903 BKY 
 	Reason - catching up with CA speed. 

 	Modified files - init_EF59L.rc (system\core\rootdir)
*/
#define FEATURE_LGT_DS_TCP_BUFFER_FOR_LTE

/* Item : MTU
 	Commnet - 20121205
 	Reason - change MTU size for LTE
 		    - deleted usb0 interface set MTU size and change TCP MSS for tethering by NAT
 		       (merged EF46L)
 		    - replaced of FEATURE_LGT_DS_USB_KERNEL_MTU_SETTING
 		    
 	Modified files - msm8974_ef59l_xxxx_defconfig(kernel\arch\arm\configs),
                       msm8974_ef59l_xxxx_perf_defconfig(kernel\arch\arm\configs),
 				          NatController.cpp(system\netd)
*/
#define FEATURE_LGT_DS_TETHERING_MTU_SETTING

/* Item : Data Connection
 	Commnet - 20120111 Alice
 	Reason - do query failure cause when occur failure in DC.
 	Modified files - DataConnection.java(frameworks\base\telephony\java\com\android\internal\telephony)
*/
#define FEATURE_LGT_DS_PS_FAIL_CAUSE

/* Item : Data Connection
 	Commnet - 20120111 Alice
 	Modified files - DataConnection.java(frameworks\base\telephony\java\com\android\internal\telephony)
*/
#define FEATURE_LGT_DS_PS_FAIL_CAUSE_FATAL_EXCEPTION

/* Item : VPN
 	Commnet - 20120117 Alice, 20120217 Alice, 20120509 kns
 	Reason - 1. VPN BUG FIX :  just function execution's order change.
	              -> L2tp.c 	
 	             2. added a few compile option
 	             3. adapted to new file (msm8974_ef59l_xxxx_defconfig)
                4. fix build error as PPP enable
 	             
 	             5. VPN Editing -> Null check
 	                - replace of FEATURE_LGT_DS_VPN_SPACE_CHECK
 	                
 	Modified files  L2tp.c(LINUX\android\external\mtpd),
                      msm8974_ef52l_xxxx_defconfig(kernel\arch\arm\configs),
                      msm8974_ef52l_xxxx_perf_defconfig(kernel\arch\arm\configs),
                      ppp_defs.h(kernel\include\linux),
                      VpnDialog.java (packages\apps\settings\src\com\android\settings\vpn2)
*/
#define FEATURE_LGT_DS_VPN_FIX

/* Item : NetworkInfo
 	Commnet - 20120417 Alice
  	Reason - NetworkInfo isAvailable is false when APN State is failed
  	          - merge From STARQ, FEATURE_P_VZW_DS_APN_FAILED_STATE_BUG
 	Modified files -  CdmaDataConnectionTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\cdma),
 	                     GsmDataConnectionTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\gsm),
*/
#define FEATURE_LGT_DS_APN_FAILED_STATE_BUG

/* Item : Route
 	Commnet - 20121109 Alice
  	Reason - Reconnect when routing add failed.
  	          - If occured failure to add Route, do tear down and try to reconnect after 3 sec.
  	          - merge From STARQ, FEATURE_P_VZW_DS_ROUTE_ADD_FAIL
  	          
 	Modified files - ConnectivityService.java (frameworks\base\services\java\com\android\server),
 	                     QcConnectivityService.java (frameworks\opt\connectivity\services\java),
 	                     MobileDataStateTracker.java (frameworks\base\core\java\android\net),
                        NetworkStateTracker.java (frameworks\base\core\java\android\net)
*/
#define FEATURE_LGT_DS_ROUTE_ADD_FAILURE

/* Item : Tethering
 	Commnet - 20120418 Alice
  	Reason - at the same time USB tethering and WiFi Hotstop, DATA off -> DATA on -> WiFi on
  	             display both WiFi icon and DATA icon for a few seconds. maybe the longest 60 sec.
  	           - after startUsingNetworkFeature, don't call stopUsingNetworkFeature
  	              now, when IMS de-registration, call stopUsingNetworkFeature.
  	           - If IMS doesn't call stopUsingNetworkFeature, do stopUsingNetworkFeature.
 	Modified files - Tethering.java (frameworks\base\services\java\com\android\server\connectivity)
*/
#define FEATURE_LGT_DS_DISPLAY_WIFI_DATA_ICON_WHEN_TETHERING

/* Item : Tethering
 	Commnet - 20120723 sjm
  	Reason - Tethering DNS Forward Error Fix from SKT
 	Modified files - Tethering.java (frameworks\base\services\java\com\android\server\connectivity)
*/
#define FEATURE_LGT_DS_TETHERING_DNS_FORWARD

/* Item : APN Changed
 	Commnet - 20120727 sjm, 20121023 Alice
  	Reason - Ignore APN Changed event due to setRoaminAPN()

 	Modified files - GsmDataConnectionTracker.java (rameworks\base\telephony\java\com\android\internal\telephony\gsm),
                        SkyDataConnectionTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony)
*/
#define  FEATURE_LGT_DS_IGNORE_APN_CHANGED

/*
 - 20120816 sjm
 - immediatly send Connection_action intent 
 */
 #define FEATURE_LGT_DS_CONNECTIVTY_INTENT_IMMEDIATE

/* Item : Route
 	Commnet - 20120905 sjm, 20121212 Alice
  	Reason - SilentReset when modify route
 - Add try/catch and syncronized

 	Modified files - ConnectivityService.java (frameworks\base\services\java\com\android\server),
 	                     QcConnectivityService.java (frameworks\opt\connectivity\services\java),
 	                     AbstractCollection.java (libcore\luni\src\main\java\java\util)
 */
 #define FATURE_LGT_DS_CONCURRENTMODIFICATIONEXCEPTION

 /*
 - 20120912 sjm
 - APN State Bug WorkAround Code
 */
 #define FEATURE_LGT_DS_APN_STATE_FIX

#ifdef FEATURE_SKY_DS_BACKGROUND_RESTRICT_BUG_FIX
/* Item : Mutex
   Commnet - 20121129 Alice
   Reason - Change mutex time to 300ms for fixing ANR

   Modified files - NetworkPolicyManagerService.java(frameworks\base\services\java\com\android\server\net)
*/
#define FEATURE_LGT_DS_RELEASE_MUTEX_TIME
#endif /* FEATURE_SKY_DS_BACKGROUND_RESTRICT_BUG_FIX */

/* Item : Route
 	Commnet - 20121113 Alice
  	Reason - added method to suspend data call. 

 	Modified files - ISkyDataConnection.aidl (frameworks\base\telephony\java\com\android\internal\telephony),
                        SkyDataConInterfaceManager.java (frameworks\opt\telephony\src\java\com\android\internal\telephony),
                        SkyDataConnectionTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony)
*/
#define FEATURE_LGT_DS_ADD_DATA_SUSPEND_FUNC

/* Item : Retry count
	Commnet - 20121204 Alice
	Reason - prevent to reset retry counter 
	             according to occur EVENT_TETHERED_MODE_STATE_CHANGED
	             when fail data connection
	          - LGU+ MultiMode TEST : 16.16. PDN connectivity reject #27 (internet PDN)
	          - retry interval : 5s, 10s, 20s. and repeat after 30 min.
	             
	Modified files - DataConnectionTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony)
*/
#define FEATURE_LGT_DS_DO_NOT_RESET_RETRY_COUNT

/* Item : LTS Agent
	Commnet - 20130122 Alice
	Reason - broadcast intent when LTE DISCONNECTED
	
	Modified files - MobileDataStateTracker.java (frameworks\base\core\java\android\net)
*/
#define FEATURE_LGT_DS_INTENT_LTE_DISCONNECT_FOR_LTS

/* Item : OMH
	Commnet - 20130219 Alice
	Reason - do not read unnecessary profile.
	
	Modified files - System.prop(device\qcom\msm8974)
*/
#define FEATURE_LGT_DS_OMH_DISABLED

/* Item : PARTIAL RETRY
	Commnet - 20131028 hongss
	Reason - disable partial retry
	
	Modified files - System.prop(device\qcom\msm8974)
*/
#define FEATURE_LGT_DS_DISABLE_PARTIAL_RETRY

/* Item : DL Throughput
    Comment - 20131119 SJM
    Reason : DL Throughput form SKT

    Modified Files - bam_dmux.c (kernel\arch\arm\mach-msm)
*/
#define FEATURE_SKY_DS_BAM_ADAPTIVE_TIMER_OFF

/* Item : DSA
    Comment - 20131217 sjm, 20131224 hongss
    Reason - Clean Up all connection due to Mirror Call
  	           - merge from EF52L             
    Modified Files - DataConnectiontracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony)
*/
#define FEATURE_LGT_DS_MODIFY_DSA

/*......................................................................................................................................
  DNS
.........................................................................................................................................*/

/* Item : DNS
 	Commnet - 20120105 Alice
 	Reason - check null DNS.
              - not only NULL_IP(0.0.0.0) but also length 0
 	          
 	Modified files - GsmDataConnection.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\gsm),
*/
#define FEATURE_LGT_DS_PS_NULL_DNS_CHECK

/* Item : DNS
 	Commnet - 20120113 Alice
 	Reason - MMS App. is checked the test DNS as soon as possible to disconnect Data call.
 	             and then MMS App try to send MMS, Inetaddress.getbyname occur unknown host exception.
 	             
 	Modified files - getaddrinfo.c(bionic\libc\netbsd\net)
*/
#define FEATURE_LGT_DS_DISABLE_TEST_DNS

/* Item : DNS
 	Commnet - 20120113 Alice
 	Reason - do not add route KT's(168.126.63.1/168.126.63.2) for VT(startusingnetworkfeature(), TYPE_DUN).
 	
 	Modified files - ConnectivityService.java (frameworks\base\services\java\com\android\server),
 	                     QcConnectivityService.java (frameworks\opt\connectivity\services\java)
*/
#define FEATURE_LGU_DS_3G_DNS_REMOVE_IN_ROUTETABLE

/* Item : DNS
	Commnet - 20120111 Alice
	Reason - do not query DNS IPv6.
	Modified files - getaddrinfo.c(bionic\libc\netbsd\net)
*/
#define FEATURE_LGT_DS_IPV6_DNS_QUERY

/* Item : DNS for IMS PDN
	Commnet - 20130816 SJM
	Reason - do not query DNS IPv4 for using IPv6.
	Modified files - getaddrinfo.c(bionic\libc\netbsd\net)
*/
#define FEATURE_LGT_DS_IMS_PDN_DNS_QUERY

/* Item : Null DNS for IMS/EMERGENCY PDN
	Commnet - 20131128 SJM
	Reason - Allow connection with null DNS for IMS/EMERGENCY PDN
	Modified files - getaddrinfo.c(bionic\libc\netbsd\net)
	                    - GsmDataConnection.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\gsm),
*/
#define FEATURE_LGT_DS_ALLOW_NULL_DNS_IMS_EMERGENCY

/*......................................................................................................................................
  For App.
.........................................................................................................................................*/

/* Item : RADIO
	Commnet - 20120113 Alice
   Reason - RADIO Tech가 Data Connection state, qcril????��?��??��? ??? ??? 경�?.
*/
#define FEATURE_LGU_DS_UNKNOWN_RADIO

#ifdef FEATURE_LGU_DS_UNKNOWN_RADIO
/* Item : RADIO
	Commnet - 20120113 Alice
	Reason - App. is not working When enter Out Of Servcie(OOS)
	             Although App. already had receivced patcket.
	          - the reason why NetworkInfo.available is false.

	Modified files - MobileDataStateTracker.java (frameworks\base\core\java\android\net),
				 NetworkController.java (frameworks\base\packages\systemui\src\com\android\systemui\statusbar\policy)	 
*/
#define FEATURE_LGT_DS_AVOID_OOS_FOR_APP

/* Item : RADIO
 	Commnet - 20120320 Alice
 	Reason - set to false "TelephonyProperties.PROPERTY_OOS_IS_DISCONNECT".
 	           - if mOosIsDisconnect == true, changed data connection to disconnection as soon as entering no-service.
 	           - cause confusion in App. layer.
 	Modified files - System.prop(device\qcom\msm8974)
*/
#define FEATURE_LGT_DS_OOS_PROPERTY_INITIAL_VAL

#endif /* FEATURE_LGU_DS_UNKNOWN_RADIO */

/* Item : MMS
 	Commnet - 20120217 Alice
  	Reason - MMS App. Request.
            - added "isAvailableForMms" return to connection state of mobile_mms before "startUsingNetworkFeature" 
            - return true :: AlwaysOnSetting true, AuthFail false, DC.FailCause not permanantfail, In service.

            - added getServiceState:: for No service.
            - as 1x Data Disable return false when RadioTech is 1x
            
    Modified files - ISkyDataConnection.aidl (frameworks\base\telephony\java\com\android\internal\telephony),
                        MobileDataStateTracker.java (frameworks\base\core\java\android\net),
                        SkyDataConInterfaceManager.java (frameworks\opt\telephony\src\java\com\android\internal\telephony),
                        SkyDataConnectionTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony)
*/
#define FEATURE_LGT_DS_CHECK_NETWORK_AVAILABLE_FOR_MMS

/* Item : MDM
    Commnet - 20120107 SJM
    Reason : MDM App. Requirement
    Modified files - DevicePolicyManager.java 				(frameworks\base\core\java\android\app\admin),
	                        ConnectivityManager.java 				(frameworks\base\core\java\android\net),
	                        IConnectivityManager.aidl 				(frameworks\base\core\java\android\net),
	                        INetworkManagementService.aidl 	(frameworks\base\core\java\android\os),
	                        ConnectivityService.java 					(frameworks\services\java\com\android\server),
	                        QcConnectivityService.java (frameworks\opt\connectivity\services\java),
	                        DevicePolicyManagerService.java 	(frameworks\services\java\com\android\server),
	                        NetworkManagementService.java 	(frameworks\services\java\com\android\server),
	                        BandwidthController.cpp 					(system\netd),
	                        BandwidthController.h 					(system\netd),
	                        CommandListener.cpp 					(system\netd),
	                        RouteController.cpp 						(system\netd),
	                        RouteController.h 							(system\netd)
*/
#define FEATURE_LGT_DS_MDM_DATA_REQUIREMENT

/* Item : SIP packet with over MTU using TCP dropped 
     Comment : 20130821 SJM
     Modified files : nf_conntrack_sip.c    (Kernel\net\netfilter)
*/
#define FEATURE_LGT_DS_TCP_DROP_OVER_MTU_PORT_5060

/* Item : IPTABLE
    Commnet -20131223 SJM Modified due to reset!! 2013830 PJW
    Reason : Youtube Application SSDP packet
*/
#define FEATURE_LGT_DS_SSDP_PACKET_DISABLE

/*......................................................................................................................................
  CTS
.........................................................................................................................................*/

/* Item : UID_STAT CONFIG
    Commnet - 2012 ksg. 20120509 kns, 20121205 Alice
    Reason - enable UID_STAT=y for TrafficStats(CTS TEST Fail)
    Modified files - msm8974_ef59l_xxxx_defconfig(kernel\arch\arm\configs),
                        msm8974_ef59l_xxxx_perf_defconfig(kernel\arch\arm\configs)
*/
#define FEATURE_LGT_DS_UID_TRAFFIC_STATS_ENABLE
/* Item : CTS IPv6 port listening fail
    Commnet - 20131030 hongss
    Reason - not use QC ims stack and cts listening port test fail fix(CTS TEST Fail)
           - merge from EF61K
    Modified files - init.target.rc (android\device\qcom\msm8974),
                     device-vendor.mk (android\vendor\qcom\proprietary\common\config)
*/
#define FEATURE_LGT_DS_CTS_LISTENING_PORT_TEST_FAIL_FIX

/*......................................................................................................................................
  Network Tools
.........................................................................................................................................*/
/* Item : BUSYBOX 
	Commnet - 20120509 Alice
	Reason - busybox install for root(eng) version 1.20.0

   Added files - busybox folder all (vendor\pantech\development\busybox)
*/
#define FEATURE_LGT_DS_BUSYBOX_INSTALL

/* Item : IPERF 
	Commnet - 20120509 Alice
	Reason - iperf install for root, version : 2.0.5 (08 Jul 2010)

   Added files - iperf folder all (vendor\pantech\development\iperf)
*/
#define FEATURE_LGT_DS_IPERF_INSTALL

#endif /* FEATURE_LGT_DS_COMMON */

/*===========================================================================
    Others
===========================================================================*/

/* Item : Log change (Radio -> Main)
    Commnet - 20120915 Alice
    Reason - change buffer to print Data Framework log, RADIO BUFFER -> MAIN BUFFER.
                 for Data Call State. 
    Modified files - GsmDataConnectionTracker.java, CdmaDataConnectionTracker.java,
                         MobileDataStateTracker.java (frameworks\base\core\java\android\net),
                         Tethering.java (frameworks\base\services\java\com\android\server\connectivity),
                         NetworkPolicyManagerService.java(frameworks\base\services\java\com\android\server\net),
                         logd_write(system\core\liblog\),
                         QcConnectivityService.java (frameworks\opt\connectivity\services\java)
*/
#define FEATURE_LGT_DS_CHANGE_RADIO_LOG

#endif/* __CUST_PANTECH_DATA_LINUX_H__ */

