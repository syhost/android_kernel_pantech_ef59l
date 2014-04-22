#ifndef __CUST_PANTECH_SOUND_H__
#define __CUST_PANTECH_SOUND_H__

/*
  2012/05/15
  All sound related debug messages enable
*/
#define FEATURE_PANTECH_SND_DEBUG

#if defined(FEATURE_PANTECH_SND_DEBUG)
#define VERY_VERBOSE_LOGGING /* ALOGVV message enable of AudioFliger.cpp, AuddioPolicyManagerBase.cpp and AuddioPolicyManagerALSA.cpp */
#endif

/*
  2012/02/20
  Qualcomm CR Patch feature
*/
#define FEATURE_PANTECH_SND_QCOM_CR

/*
  2012/12/07
  Qualcomm Test Patch feature - shuld be checked before market release version
*/
#define FEATURE_PANTECH_SND_TEST_PATCH

/*
  2012/03/12
  Feature must be applied to all models
*/
#define FEATURE_PANTECH_SND
#define FEATURE_PANTECH_SND_ENFORCED_HEADSET /* For ear protection WCD93xx gain adjust of headset in enforced audible */
#define FEATURE_PANTECH_SND_VOICE_LOOPBACK /* For Tx ACDB separation of VOICE loopback test */

/*
  2011/10/24
  Feature separation of domestic and abroad models
*/
#if defined(T_EF56S) || defined(T_EF59S) || defined(T_EF59K) || defined(T_EF59L) || defined(T_EF60S) || defined(T_EF61K) || defined(T_EF62L) // DOMESTIC
#define FEATURE_PANTECH_SND_DOMESTIC
#define FEATURE_PANTECH_SND_AUTOANSWER_RX_MUTE /* For autoanswer Rx mute on/off */
#define FEATURE_PANTECH_SND_TX_ACDB_SEPARATION /* For Voice Recognition & Camcorder Handset/Headset TxACDB separation */
#define FEATURE_PANTECH_SND_EXT_VOL_UP /* For Extreme Volume Up when receiving a call */
#define FEATURE_PANTECH_SND_VT_VOLTE_ACDB_SEPARATION /* For VT and VoLTE ACDB separation */
#define FEATURE_PANTECH_SND_VOLTE /* For VoLTE related modification */
#define FEATURE_PANTECH_SND_VOLTE_EQ /* For VoLTE EQ related modification */
#define FEATURE_PANTECH_SND_LPA /* For Non-Tunnel LPA issue fix */
//#define FEATURE_PANTECH_SND_TUNNEL /* For Tunnel LPA issue fix */ /* not used Tunnel LPA mode */
#define FEATURE_PANTECH_SND_BT_NREC /* For BT NREC(AT+NREC) function */
#define FEATURE_PANTECH_SND_FLAC_LPA  //20130809 jhsong : possible playback flac contents for lpa player
#if defined(T_EF56S) || defined(T_EF59S) || defined(T_EF60S) // SKT models specific feature
#define FEATURE_PANTECH_SND_SKT
#elif defined(T_EF59K) || defined(T_EF61K) // KT models specific feature
#define FEATURE_PANTECH_SND_KT
#elif defined(T_EF59L) || defined(T_EF62L) // LGU+ models specific feature
#define FEATURE_PANTECH_SND_LGT
#define FEATURE_PANTECH_SND_LGU_RMS /* For LGU+ RMS service */
#endif
#if defined(T_EF59S) || defined(T_EF59K) || defined(T_EF59L) // EF59Series(Dual MIC) specific feature
#define FEATURE_PANTECH_SND_SUBMIC_LOOPBACK /* For SubMic Loopback test */
#endif
#if defined(T_EF56S)|| defined(T_EF59S) || defined(T_EF59K) || defined(T_EF59L) || defined(T_EF60S) || defined(T_EF61K)
#define FEATURE_PANTECH_SND_NXP /* For NXP Sound Effect solution(Sound Experience) */
#if !defined(T_EF56S)
#define FEATURE_PANTECH_SND_NXP_LM /* For NXP Sound LM Effect solution(Sound Experience) */
#endif
#elif defined(T_EF62L) // EF62L will only use QSOUND effect
#define FEATURE_PANTECH_SND_QSOUND /* For QSound Effect solution(QFX, QVSS, QXV) */
#if defined(FEATURE_PANTECH_SND_QSOUND)
#define FEATURE_PANTECH_SND_QSOUND_LPA
#endif

//#define FEATURE_PANTECH_SND_NXP /* For NXP Sound Effect solution(Sound Experience) */ /* temp enable, currently QSOUND is developing */
#endif

#if defined(T_EF60S) || defined(T_EF61K) || defined(T_EF62L)
#define FEATURE_PANTECH_SND_PIEZO_TEST /* For Piezo Speaker Test */
#endif

#elif defined(T_NAMI) // PMC
#define FEATURE_PANTECH_SND_PMC /* For PMC Specific requirements */
#define FEATURE_PANTECH_SND_AUTOANSWER_RX_MUTE /* For autoanswer Rx mute on/off */
#define FEATURE_PANTECH_SND_TX_ACDB_SEPARATION /* For Voice Recognition & Camcorder Handset/Headset TxACDB separation */

#elif defined(T_XXXX)  // VZW
#define FEATURE_PANTECH_SND_ABROAD
#define FEATURE_PANTECH_SND_VZW
#define FEATURE_PANTECH_SND_QSOUND /* For QSound Effect solution(QFX, QVSS, QXV) */
#define FEATURE_PANTECH_SND_LPA /* For Tunnel or NT LPA issue fix */
#define FEATURE_PANTECH_SND_BT_GROUPING /* For BT NAC certification */
#define FEATURE_PANTECH_SND_BT_ECNR /* For BT SPEC ECNR disable(AT+NREC) function */
#define FEATURE_PANTECH_SND_ELECTOVOX /* For Transo NR function */

#elif defined(T_XXXX) // ATT
#define FEATURE_PANTECH_SND_ABROAD
#define FEATURE_PANTECH_SND_ATT
#define FEATURE_PANTECH_SND_QSOUND /* For QSound Effect solution(QFX, QVSS, QXV) */
#define FEATURE_PANTECH_SND_LPA /* For Tunnel or NT LPA issue fix */
#define FEATURE_PANTECH_SND_BT_GROUPING /* For BT NAC certification */
#define FEATURE_PANTECH_SND_BT_ECNR /* For BT SPEC ECNR disable(AT+NREC) function */
#define FEATURE_PANTECH_SND_VOC_LOOPBACK /* For submic loop back */

#else
//    #error "FEATURE_PANTECH_SND ? DOMESTIC or ABROAD"
#endif

/*
  2012/09/19
  Feature regarding the boot sound(SKT/VZW/ATT carrier only)
*/
#if defined(FEATURE_PANTECH_SND_SKT) || defined(FEATURE_PANTECH_SND_VZW) || defined(FEATURE_PANTECH_SND_ATT) 
#define FEATURE_PANTECH_SND_BOOT_SOUND
#endif

/*
  2012/12/21
  Feature regarding the shutdown sound(SKT/VZW carrier only)
*/
#if defined(FEATURE_PANTECH_SND_DOMESTIC) || defined(FEATURE_PANTECH_SND_VZW)
#define FEATURE_PANTECH_SND_SHUTDOWN_SOUND
#endif

#endif /* __CUST_PANTECH_SOUND_H__ */
