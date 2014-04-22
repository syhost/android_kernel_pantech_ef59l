#ifndef CUST_PANTECH_SIO_H
#define CUST_PANTECH_SIO_H

/****************************************************************************** 
 ************************** featureing rule ***********************************
 * FEATURE_PANTECH_[USB|UART|TESTMENU|FACTORY_COMMAND|SIO|STABILITY]_SUBFUNC  *
*******************************************************************************/

#define FEATURE_PANTECH_SIO_TEMP  //temporary feature for development
#define FEATURE_PANTECH_SIO_BUG_FIX // debug feature for qualcomm bugs.

#if !defined(__KERNELBUILD__)
#endif

#if !defined(__KERNELBUILD__) && !defined(__MODEMBUILD__)

#define FEATURE_PANTECH_USB_PST_MODE_CHANGE
#if defined(T_EF59K) || defined(T_EF61K)
#define FEATURE_PANTECH_USB_SMART_DM_CONTROL
#endif
#if defined(T_EF59L) || defined(T_EF62L)
#define FEATURE_PANTECH_USB_LGT_PC_MODE
#endif

#endif

/*******************************************************************************
** UART CONSOLE
*******************************************************************************/
#define FEATURE_PANTECH_UART_SERIAL 

#if defined(FEATURE_PANTECH_UART_SERIAL)
#if ((defined(T_EF56S) || defined(T_EF57K) || defined(T_EF58L)) && (BOARD_VER < TP10)) || (defined(T_NAMI) && (BOARD_VER < PT20))
#define FEATURE_PANTECH_CONSOLE_UART1
#else
#define FEATURE_PANTECH_CONSOLE_UART10
#endif
#endif


/*******************************************************************************
** STABILITY 
*******************************************************************************/
#define FEATURE_PANTECH_STABILITY_AT_COMMAND

/*******************************************************************************
**  TEST_MENU & FACTORY_COMMAND
*******************************************************************************/
#define FEATURE_PANTECH_TESTMENU_USB
#define FEATURE_PANTECH_TESTMENU_SERVER_VER2
#define FEATURE_PANTECH_FACTORY_COMMAND

/*******************************************************************************
** USB 
*******************************************************************************/
/*******************************************************************************
 * COMMON FEATURE
 * ***************************************************************************/
#define FEATURE_PANTECH_USB
#define FEATURE_PANTECH_USB_DEBUG
#ifdef FEATURE_PANTECH_USB_DEBUG
#define FEATURE_PANTECH_USB_STATE_DEBUG
#endif
#define FEATURE_PANTECH_USB_TUNE_SIGNALING_PARAM
//#define FEATURE_HSUSB_SET_SIGNALING_PARAM
#define FEATURE_PANTECH_USB_QXDM_ONOFF
//#define FEATURE_PANTECH_USB_QXDM_MASK_ONOFF

/* adb security */
#define FEATURE_PANTECH_USB_ADB_SECURE
/*******************************************************************************
 * DEPENDANT ON MODEL
 * ***************************************************************************/
#define FEATURE_PANTECH_USB_CDFREE
#define FEATURE_PANTECH_USB_BLOCKING_MDMSTATE
#if ((defined(T_EF59S) || defined(T_EF59K) || defined(T_EF59L)) && (BOARD_VER > TP10))
#define FEATURE_PANTECH_USB_SMB_OTG_DISABLE_LOW_BATTERY
#define FEATURE_PANTECH_USB_VER_SWITCH
#elif(defined(T_EF60S) || defined(T_EF61K) || defined(T_EF62L))
#define FEATURE_PANTECH_USB_SMB_OTG_DISABLE_LOW_BATTERY
#endif
//otg    #define FEATURE_ANDROID_PANTECH_USB_OTG_MODE

#if defined(T_EF59S) || defined(T_EF59K) || defined(T_EF59L)
#if (BOARD_VER > WS20)
#define FEATURE_PANTECH_USB_REDRIVER_EN_CONTROL
#endif
#endif

#if defined(T_EF59S) || defined(T_EF59K) || defined(T_EF59L) || defined(T_EF60S) || defined(T_EF61K) || defined(T_EF62L)
#define FEATURE_PANTECH_USB_EXTERNAL_ID_PULLUP
#endif

#endif/* CUST_PANTECH_SIO_H */

