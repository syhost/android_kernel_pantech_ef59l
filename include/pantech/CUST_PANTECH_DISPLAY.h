#ifndef F_SKYDISP_FRAMEWORK_FEATURE
#define F_SKYDISP_FRAMEWORK_FEATURE

/****************************************************/
/*                             PANTECH FEARTURE                             */
/****************************************************/

 /*****************************************************
* owner  : p13832          
* date    : 2013.04.11 
* PLM    : 
* Case  :       
* Description : it's for cabc control
* kernel : used
* user    : used
******************************************************/
#if defined(T_EF56S) || defined(T_EF57K) || defined(T_EF58L) || defined(T_EF59S) || defined(T_EF59K) || defined(T_EF59L) || defined(T_EF60S) || defined(T_EF61K) || defined(T_EF62L) || defined(T_NAMI)
#undef CONFIG_F_SKYDISP_CABC_CONTROL
#define CONFIG_F_SKYDISP_CABC_CONTROL

#undef CONFIG_F_SKYDISP_SHARPNESS_CTRL
#define CONFIG_F_SKYDISP_SHARPNESS_CTRL
#endif

 /*****************************************************
* owner  : p13832          
* date    : 2013.04.23
* PLM    : 
* Case  :       
* Description : it's for lcd onff
* kernel : used
* user    : used
******************************************************/

#define CONFIG_F_SKYDISP_NEW_LCD_ONOFF

 /*****************************************************
* owner  : p13832          
* date    : 2013.07.02
* PLM    : 
* Case  :       
* Description : it's for lcd cmds test
* kernel : used
* user    : used
******************************************************/

#define CONFIG_F_SKYDISP_CMDS_CONTROL


/*****************************************************
* owner  : p14974
* date    : 2013.08.13
* PLM    : 795
* Case  : 01272112
* Description : fix out-of-pipe issue and GPU fallback error
* kernel : used
* user    : used
******************************************************/

#define CONFIG_F_SKYDISP_FIXGPU_FALLBACK_ERROR

 /*****************************************************
* owner  : p16008          
* date    : 2013.07.29
* PLM    : 
* Case  :       
* Description : calcuration for AOT player.
* kernel : none
* user    : used
******************************************************/
#if defined(T_EF56S) || defined(T_EF59S) || defined(T_EF59K) || defined(T_EF59L) || defined(T_EF60S) || defined(T_EF61K) || defined(T_EF62L)
#define CONFIG_F_SKYDISP_AOT_TRIM
#endif

 /*****************************************************
* owner  : p16008          
* date    : 2013.07.30
* PLM    : 
* Case  :       
* Description : DRM contents is not shown to external device , (ex HDMI , MiraCast) 
* kernel : none
* user    : used
******************************************************/
#if defined(T_EF56S) || defined(T_EF59S) || defined(T_EF59K) || defined(T_EF59L) || defined(T_EF60S) || defined(T_EF61K) || defined(T_EF62L)
#define CONFIG_F_SKYDISP_DRM_ISNOT_SHOWN_TO_MIRACAST
#endif

 /*****************************************************
* owner  : p13832          
* date    : 2013.08.19
* PLM    : 
* Case  :       
* Description : it's for lcd backlight
* kernel : used
* user    : used
******************************************************/
#define CONFIG_F_SKYDISP_BACKLIGHT_CMDS_CTL

 /*****************************************************
* owner  : p13832          
* date    : 2013.10.08
* PLM    : 
* Case  :       
* Description : it's for trim bug(when stopped video is moved, the capture screen is cut)
* kernel : 
* user    : used
******************************************************/

#if defined(T_EF56S) || defined(T_EF59S) || defined(T_EF59K) || defined(T_EF59L) || defined(T_EF60S) || defined(T_EF61K) || defined(T_EF62L)
#define CONFIG_F_SKYDISP_AOT_TRIM_BUGFIX
#endif
/****************************************************/
/*                             QUALCOMM FEARTURE                          */
/****************************************************/

 /*****************************************************
* owner  : p13832          
* date    : 2013.04.11 
* PLM    : 
* Case  :       
* Description : 
* kernel : used
* user    : used
******************************************************/

#if defined(T_EF56S) || defined(T_EF57K) || defined(T_EF58L) || defined(T_EF59S) || defined(T_EF59K) || defined(T_EF59L) || defined(T_EF60S) || defined(T_EF61K) || defined(T_EF62L) || defined(T_NAMI)
#undef QUALCOMMM_BUGFIX_
#define QUALCOMMM_BUGFIX_
#endif

/****************************************************/
/*                             QUALCOMM FEARTURE                          */
/****************************************************/

 /*****************************************************
* owner  : p12226      
* date    : 2013.07.23 
* PLM    : (EF56S PLM3337, 2490)
* Case  :  #1244618 , #01241329
* Description :  For BWC SMP FIX 
* kernel : used
* user    : used
******************************************************/

#if defined(T_EF56S) || defined(T_EF59S) || defined(T_EF59K) || defined(T_EF59L) || defined(T_EF60S) || defined(T_EF61K) || defined(T_EF62L) 
#undef QUALCOMM_BUGFIX_BWC_SMP
#define QUALCOMM_BUGFIX_BWC_SMP
#endif

 /*****************************************************
* owner  : p10443     
* date    : 2013.08.13 
* PLM    : (EF56S PLM5497)
* Case  :  #01268552
* Description :  MP3 LCD OFF power conumption
* kernel : used
* user    : used
******************************************************/

#if defined(T_EF56S) || defined(T_EF59S) || defined(T_EF59K) || defined(T_EF59L) || defined(T_EF60S) || defined(T_EF61K) || defined(T_EF62L) 
#undef QUALCOMM_MP3_LCDOFF_CURRENT_PATCH
#define QUALCOMM_MP3_LCDOFF_CURRENT_PATCH
#endif


 /*****************************************************
* owner  : p16008          
* date    : 2013.08.27
* PLM    : 
* Case  :       
* Description : fix locking in external connect and composition.
* kernel : none
* user    : used
******************************************************/
#if defined(T_EF56S) || defined(T_EF59S) || defined(T_EF59K) || defined(T_EF59L) || defined(T_EF60S) || defined(T_EF61K) || defined(T_EF62L)
#define QUALCOMM_BUGFIX_FIX_LOCKING_EXTERNAL
#endif

 /*****************************************************
* owner  : p13447         
* date    : 2013.10.01
* PLM    : 617,2655 ( EF59K )
* Case  : 01302804     
* Description : Fixed a code for broken image issue on Video Studio application
* kernel : none
* user    : used
******************************************************/
#if defined(T_EF56S) || defined(T_EF59S) || defined(T_EF59K) || defined(T_EF59L) || defined(T_EF60S) || defined(T_EF61K) || defined(T_EF62L)
#define QUALCOMM_BUGFIX_FIX_MOVIE_STUDIO_BUG
#endif

 /*****************************************************
* owner  : p13832       
* date    : 2013.10.22
* PLM    : 
* Case  : 1333119     
* Description : user null pointer error in eglsubAndroid.so
* kernel : none
* user    : used
******************************************************/
#if defined(T_EF56S) || defined(T_EF59S) || defined(T_EF59K) || defined(T_EF59L) || defined(T_EF60S) || defined(T_EF61K) || defined(T_EF62L)
#define QUALCOMM_BUGFIX_FIX_SEGVMAPERR_ERROR
#endif

 /*****************************************************
* owner  : p12452, Cho KyoungKu       
* date    : 2013.10.23
* PLM    : 
* Case  : 1331763 (Unresolved via SR because not reproduce on MTP. So applied workaround)      
* Description : kernel panic during miracast (miracast connect -> video_play -> power_off -> reset)
* kernel : none
* user    : used
******************************************************/
#if defined(T_EF60S) || defined(T_EF61K) || defined(T_EF62L)
#define CONFIG_F_SKYDISP_PANIC_DURING_MIRACAST
#endif

 /*****************************************************
* owner  : p10334       
* date    : 2013.10.29
* PLM    : 
* Case  : 01321798       
* Description : UHD(3840*2160, 2160*3840) contents underrun in video play and AOT 
* kernel : none
* user    : used
******************************************************/
#if defined(T_EF56S) || defined(T_EF59S) || defined(T_EF59K) || defined(T_EF59L) || defined(T_EF60S) || defined(T_EF61K) || defined(T_EF62L)
#define QUALCOMM_BUGFIX_UHD_UNDERRUN
#endif

#endif  /* SKY_FRAMEWORK_FEATURE */
