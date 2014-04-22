#ifndef __CUST_PANTECH_CAMERA_H__
#define __CUST_PANTECH_CAMERA_H__


/*------------------------------------------------------------------------------

(1)  �ϵ���� ����
   
EF39S   : APQ8060, CE1612(8M ISP), S5K6AAFX13(1.3M)
EF40S/40K/65L   : APQ8060, CE1612(8M ISP), MT9D113(1.3M)
PRESTO  : APQ8060, MT9P111(5M SOC), MT9V113(VGA)
EF44S   : MSM8960, CE1502(13M ISP), YACD5C1SBDBC(2M)
MAGNUS   : MSM8960, CE1502(13M ISP), YACD5C1SBDBC(2M)
EF48S/49K/50L   : APQ8064, CE1502(13M ISP), YACD5C1SBDBC(2M)
EF56S/57K/58L   : MSM8974, IMX135(13M bayer), S5K6B2YX(2M FHD bayer)


(2)  ī�޶� ���� ��� kernel/userspace/android �α׸� ����

kernel/arch/arm/config/msm8660-perf-(�𵨸�)_(�������)_defconfig �� �����Ѵ�.

	# CONFIG_MSM_CAMERA_DEBUG is not set (default)

CUST_PANTECH_CAMERA.h ���� F_PANTECH_CAMERA_LOG_PRINTK �� #undef �Ѵ�.

	#define F_PANTECH_CAMERA_LOG_PRINTK (default)

��� �ҽ� ���Ͽ��� F_PANTECH_CAMERA_LOG_OEM �� ã�� �ּ� ó���Ѵ�.
	���� �� ���, �ش� ���Ͽ� ���� SKYCDBG/SKYCERR ��ũ�θ� �̿���
	�޽������� Ȱ��ȭ ��Ų��. (user-space only)

��� �ҽ� ���Ͽ��� F_PANTECH_CAMERA_LOG_CDBG �� ã�� �ּ� ó���Ѵ�.
	���� �� ���, �ش� ���Ͽ� ���� CDBG ��ũ�θ� �̿��� �޽�������
	Ȱ��ȭ ��Ų��. (user-space only)

��� �ҽ� ���Ͽ��� F_PANTECH_CAMERA_LOG_VERBOSE �� ã�� �ּ� ó���Ѵ�.
	���� �� ���, �ش� ���Ͽ� ���� LOGV/LOGD/LOGI ��ũ�θ� �̿���
	�޽������� Ȱ��ȭ ��Ų��. (user-space only)


(4)  �ȸ��ν� ���� ��� ���� ȯ��

vendor/qcom/android-open/libcamera2/Android.mk �� �����Ѵ�.
	3rd PARTY �ַ�� ��� ���θ� �����Ѵ�.

	PANTECH_CAMERA_FD_ENGINE := 0		������
	PANTECH_CAMERA_FD_ENGINE := 1		�ö���� �ַ�� ���
	PANTECH_CAMERA_FD_ENGINE := 2		��Ÿ �ַ�� ���

CUST_PANTECH_CAMERA.h ���� F_PANTECH_CAMERA_ADD_CFG_FACE_FILTER �� #undef �Ѵ�.
	�ȸ��ν� ��� ���� �������̽� ���� ���θ� �����Ѵ�.

libOlaEngine.so �� ���� libcamera.so �� ��ũ�ϹǷ� ���� ��� libcamera.so ��
ũ�Ⱑ �����Ͽ� ��ũ ������ �߻� �����ϸ�, �� ��� �Ʒ� ���ϵ鿡��
liboemcamera.so �� ������ �ٿ� libcamera.so �� ������ Ȯ���� �� �ִ�.

build/core/prelink-linux-arm-2G.map (for 2G/2G)
build/core/prelink-linux-arm.map (for 1G/3G)

------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*  MODEL-SPECIFIC                                                            */
/*  �ش� �𵨿��� ����Ǵ� �Ǵ� �ش� �𵨿����� ������ FEATURE ���           */
/*----------------------------------------------------------------------------*/
#if defined(CONFIG_SKY_EF39S_BOARD)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF39S
#define F_PANTECH_CAMERA_SKT
#elif defined(CONFIG_SKY_EF40S_BOARD)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF40S
#define F_PANTECH_CAMERA_SKT
#elif defined(CONFIG_SKY_EF40K_BOARD)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF40K
#elif defined(CONFIG_PANTECH_PRESTO_BOARD)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_PRESTO
#define F_PANTECH_CAMERA_PRESTO
/* AT&T���� ��� �߰��Ǿ�� �ϴ� Featrue */
#define F_PANTECH_CAMERA_ATT
#elif defined(T_EF45K)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF45K
#define F_PANTECH_CAMERA_EF47S_45K_46L
#elif defined(T_EF46L)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF46L
#define F_PANTECH_CAMERA_EF47S_45K_46L
#elif defined(T_EF47S)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF47S
#define F_PANTECH_CAMERA_EF47S_45K_46L
#elif defined(T_SVLTE)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_SVLTE
#elif defined(T_CSFB)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_CSFB
#elif defined(T_CHEETAH)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_CHEETAH
#elif defined(T_STARQ)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_STARQ
#define F_PANTECH_CAMERA_ATT
#elif defined(T_OSCAR)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_OSCAR
#define F_PANTECH_CAMERA_ATT
#elif defined(T_VEGAPVW)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_VEGAPVW
/* AT&T���� ��� �߰��Ǿ�� �ϴ� Featrue */
#define F_PANTECH_CAMERA_ATT
#elif defined(T_ZEPPLIN)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_ZEPPLIN
#elif defined(T_RACERJ)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_RACERJ
#elif defined(T_SIRIUSLTE)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_SIRIUSLTE
#elif defined(T_EF44S)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF44S
#define F_PANTECH_CAMERA_SKT
#elif defined(T_MAGNUS)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_MAGNUS
#elif defined(T_EF48S)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF48S
#define F_PANTECH_CAMERA_EF48S_49K_50L
#define F_PANTECH_CAMERA_SKT
#elif defined(T_EF49K)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF49K
#define F_PANTECH_CAMERA_EF48S_49K_50L
#define F_PANTECH_CAMERA_KT
#elif defined(T_EF50L)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF50L
#define F_PANTECH_CAMERA_EF48S_49K_50L
#define F_PANTECH_CAMERA_LGT
#elif defined(T_EF51S)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF51S
#define F_PANTECH_CAMERA_EF51S_51K_51L
#define F_PANTECH_CAMERA_SKT
#elif defined(T_EF51K)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF51K
#define F_PANTECH_CAMERA_EF51S_51K_51L
#define F_PANTECH_CAMERA_KT
#elif defined(T_EF51L)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF51L
#define F_PANTECH_CAMERA_EF51S_51K_51L
#define F_PANTECH_CAMERA_LGT
#elif defined(T_EF52S)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF52S
#define F_PANTECH_CAMERA_EF52S_52K_52L
/* #define F_PANTECH_CAMERA_SKT */
#elif defined(T_EF52K)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF52K
#define F_PANTECH_CAMERA_EF52S_52K_52L
#define F_PANTECH_CAMERA_KT
#elif defined(T_EF52L)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF52L
#define F_PANTECH_CAMERA_EF52S_52K_52L
#define F_PANTECH_CAMERA_LGT
#elif defined(T_EF52W)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF52W
#define F_PANTECH_CAMERA_EF52S_52K_52L
/* #define F_PANTECH_CAMERA_SKT */
#elif defined(T_EF52W)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF52W
#define F_PANTECH_CAMERA_EF52S_52K_52L
/* #define F_PANTECH_CAMERA_SKT */
#elif defined(T_EF56S)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF56S
#define F_PANTECH_CAMERA_EF56S_57K_58L
#define F_PANTECH_CAMERA_SKT
#elif defined(T_EF57K)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF57K
#define F_PANTECH_CAMERA_EF56S_57K_58L
#elif defined(T_EF58L)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF58L
#define F_PANTECH_CAMERA_EF56S_57K_58L
#elif defined(T_EF59S)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF59S
#define F_PANTECH_CAMERA_EF59S_59K_59L
#define F_PANTECH_CAMERA_SKT
#elif defined(T_EF59K)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF59K
#define F_PANTECH_CAMERA_EF59S_59K_59L
#elif defined(T_EF59L)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF59L
#define F_PANTECH_CAMERA_EF59S_59K_59L
#elif defined(T_EF60S)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF60S
#define F_PANTECH_CAMERA_EF60S_61K_62L
#define F_PANTECH_CAMERA_SKT
#elif defined(T_EF61K)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF61K
#define F_PANTECH_CAMERA_EF60S_61K_62L
#elif defined(T_EF62L)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF62L
#define F_PANTECH_CAMERA_EF60S_61K_62L
#elif defined(T_NAMI)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_NAMI
#endif

#ifdef F_PANTECH_CAMERA

#ifndef CONFIG_PANTECH_CAMERA
#define CONFIG_PANTECH_CAMERA
/* #define CONFIG_PANTECH_CAMERA_TUNER */
#endif

#ifdef F_PANTECH_CAMERA_TARGET_EF39S
/* ISP backend camera ISP */
#if (BOARD_REV == PT10)
#define F_PANTECH_CAMERA_ICP_HD
#else
#define F_PANTECH_CAMERA_CE1612
#endif
/* 1.3M front camera sensor */
#define F_PANTECH_CAMERA_S5K6AAFX13
#endif

#ifdef F_PANTECH_CAMERA_TARGET_PRESTO
#if (BOARD_REV >= WS20)
#define F_PANTECH_CAMERA_S5K4ECGX
#else
#define F_PANTECH_CAMERA_MT9P111
#endif
#define F_PANTECH_CAMERA_MT9V113
#endif

#if defined(F_PANTECH_CAMERA_TARGET_EF40S) || \
    defined(F_PANTECH_CAMERA_TARGET_EF40K)
#if (BOARD_REV >= WS20)
#define F_PANTECH_CAMERA_MT9D113
#else
#define F_PANTECH_CAMERA_S5K6AAFX13
#endif
#define F_PANTECH_CAMERA_CE1612
#endif

#if defined(F_PANTECH_CAMERA_TARGET_EF45K) || \
    	defined(F_PANTECH_CAMERA_TARGET_EF46L) || \
	defined(F_PANTECH_CAMERA_TARGET_EF47S) || \
	defined(F_PANTECH_CAMERA_TARGET_OSCAR) || \
	defined(F_PANTECH_CAMERA_TARGET_VEGAPVW)
#define F_PANTECH_CAMERA_OV8820	
#define F_PANTECH_CAMERA_YACD5C1SBDBC
#endif	

#if defined(F_PANTECH_CAMERA_TARGET_CHEETAH) || \
	defined(F_PANTECH_CAMERA_TARGET_ZEPPLIN) 
#define F_PANTECH_CAMERA_S5K4ECGX
#define F_PANTECH_CAMERA_S5K6AAFX13	
#endif

#ifdef F_PANTECH_CAMERA_TARGET_STARQ
#define F_PANTECH_CAMERA_S5K4ECGX
#define F_PANTECH_CAMERA_MT9V113	
#endif

#if defined(F_PANTECH_CAMERA_TARGET_SVLTE) || \
    defined(F_PANTECH_CAMERA_TARGET_CSFB)
#define F_PANTECH_CAMERA_CE1612	
#define F_PANTECH_CAMERA_S5K6AAFX13	
#endif

#if defined(F_PANTECH_CAMERA_TARGET_EF44S) || defined(F_PANTECH_CAMERA_EF48S_49K_50L)
#define F_PANTECH_CAMERA_CE1502
#define F_PANTECH_CAMERA_YACD5C1SBDBC
#endif

#if defined(F_PANTECH_CAMERA_TARGET_EF51S) || defined(F_PANTECH_CAMERA_TARGET_EF51K) || defined(F_PANTECH_CAMERA_TARGET_EF51L) || defined(F_PANTECH_CAMERA_TARGET_EF52S) || defined(F_PANTECH_CAMERA_TARGET_EF52K) || defined(F_PANTECH_CAMERA_TARGET_EF52L) || defined(F_PANTECH_CAMERA_TARGET_EF52W)
#define F_PANTECH_CAMERA_CE1502
#define F_PANTECH_CAMERA_AS0260
#endif

#if defined(F_PANTECH_CAMERA_TARGET_MAGNUS)
#define F_PANTECH_CAMERA_CE1502
#define F_PANTECH_CAMERA_YACD5C1SBDBC
#endif

#if defined(F_PANTECH_CAMERA_TARGET_SIRIUSLTE)
#define F_PANTECH_CAMERA_CE1612
#define F_PANTECH_CAMERA_YACD5C1SBDBC
#endif

#if defined(F_PANTECH_CAMERA_EF56S_57K_58L)
#define F_PANTECH_CAMERA_IMX135
#define F_PANTECH_CAMERA_S5K6B2YX
#define F_PANTECH_CAMERA_ACT_WV560
#endif

#if defined(F_PANTECH_CAMERA_EF59S_59K_59L) || defined(F_PANTECH_CAMERA_EF60S_61K_62L)
#define F_PANTECH_CAMERA_IMX135
#define F_PANTECH_CAMERA_S5K6B2YX
#define F_PANTECH_CAMERA_ACT_WV560
#endif

#if defined(F_PANTECH_CAMERA_IMX135) || defined(F_PANTECH_CAMERA_S5K6B2YX)
#define F_PANTECH_CAMERA_SENSOR_BAYER
#endif

#ifdef F_PANTECH_CAMERA_CE1612
#define F_PANTECH_CAMERA_CFG_WDR
#define F_PANTECH_CAMERA_ADD_CFG_UPDATE_ISP
#define F_PANTECH_CAMERA_ADD_CFG_READ_REG

#define F_PANTECH_CAMERA_CFG_STOP_CAPTURE
#endif

#if !defined(F_PANTECH_CAMERA_OV8820)
#define F_PANTECH_CAMERA_BACKFACE_YUV
#endif

#ifdef F_PANTECH_CAMERA_CE1502
#define F_PANTECH_CAMERA_CFG_GET_FRAME_INFO
#define F_PANTECH_CAMERA_CFG_YUV_ZSL_FLASH
#define F_PANTECH_CAMERA_ADD_CFG_OJT
#endif

#ifdef F_PANTECH_CAMERA_IMX135
/* 
 * EEPROM feature code 
*/
#define F_PANTECH_CAMERA_EEPROM
#endif


#ifndef FEATURE_PANTECH_RELEASE
/* Ŀ�� ���� �ڵ忡�� SKYCDBG/SKYCERR ��ũ�θ� ����Ͽ� ��µǴ� �޽�������
 * Ȱ��ȭ ��Ų��. 
 * kernel/arch/arm/mach-msm/include/mach/camera.h 
 * �� ���Ͽ��� #define F_PANTECH_CAMERA_LOG_PRINTK�� �����Ͽ� �α׸� �� �� �ִ�.
*/
/* ���� ����(vendor)�� SKYCDBG/SKYCERR �޼��� on off */
#define F_PANTECH_CAMERA_LOG_OEM		

/* vendor/qcom/proprietary/mm-camera/mm-camera2/include/camera_dbg.h 
 * vendor/qcom/proprietary/mm-camera/mm-camera2/media-controller/modules/sensors/module/sensor_dbg.h 
 * vendor/qcom/proprietary/mm-camera/mm-camera2/media-controller/modules/imglib/components/include/img_dbg.h 
 * �� ���ϵ鿡�� #define F_PANTECH_CAMERA_LOG_PRINTK�� �����Ͽ� �α׸� �� �� �ִ�.
 * ���� ����(vendor)�� CDBG �޼��� on off 
 * ������� ���� �� ���Ͽ��� �α׸� ���� ����ϸ�, �� ������ �α� ������ ���� �α׸� ��� �� �� �ִ�.
 * camera_dbg.h : CDBG_LEVEL (0 ~ 2)
 * sensor_dbg.h : SLOG_HIGH, SLOG_LOW
 * img_dbg.h : IDBG_LOG_LEVEL (1 ~ 4)
*/
/*#define F_PANTECH_CAMERA_LOG_CDBG*/

/* ���� ������ LOGV/LOGD/LOGI �޼��� on off */
#define F_PANTECH_CAMERA_LOG_VERBOSE
#endif
/*
 * ī�޶� Preview �� ���� ������ �ݺ� �Ǵ� �α׸� �����Ѵ�.
 * preview �� ���ʿ��� �ݺ� �α׷� ���� �ٸ� �α׸� ���Ⱑ ��ư� ������� ��ư� �ϹǷ� �����Ѵ�.
*/
#define F_PANTECH_CAMERA_ERASE_PREVIEW_LOGS

/*
 * Bayer tuning�� ���� chromatix ������ load �ؼ� �����ϵ��� �� ���
 * Tuning �Ϸ� �� ���� �ʿ�
*/
#ifdef F_PANTECH_CAMERA_SENSOR_BAYER
#define LOAD_CHROMATIX_DATA_FROM_FILE
#define F_PANTECH_CAMERA_TUNING
#endif



/*----------------------------------------------------------------------------*/
/*  PANTECH CUSTOMIZATION                                                             */
/*  ���� Device �Ǵ� �𵨿� �°� ���� �Ǵ� ����ȭ                             */
/*----------------------------------------------------------------------------*/

/* ���� ������ �Կ� �ػ� ���̺��� �����Ѵ�. 
 *
 * HAL ������ ��ȿ�� �Կ� �ػ󵵸� ���̺� ���·� �����ϰ� ���̺� ���Ե� 
 * �ػ� �̿��� ���� ��û�� ������ ó���Ѵ�. */
#define F_PANTECH_CAMERA_CUST_PICTURE_SIZES

/* �ܸ����� �Կ��� ������ EXIF TAG ���� �� ������ ���� ������ �����Ѵ�. */
#define F_PANTECH_CAMERA_CUST_OEM_EXIF_TAG

/* UI���� "num-snaps-per-shutter" parameter�� set �����ν� burst shot�� ������ �� 
 * takePicture�� ���� burst shot�� �Կ� �� �� �ֵ��� �Ѵ�.
 *
 * burst shot �Կ��� �Կ����� single shot�� ���������� Camera Service���� play ��
 * ���� �Ѵ�.
 * �ػ� �̿��� ���� ��û�� ������ ó���Ѵ�. */
#define F_PANTECH_CAMERA_CUST_BURSTSHOT

#ifdef F_PANTECH_CAMERA_CUST_BURSTSHOT
/* 
 * burst shot �� UI�� ���� stop command�� �޾� ó���ϵ��� �Ѵ�.
 * UI�� Camera.java�� stopBurstShot(int numBurst) �� ȣ�� �� �� �ִ�.
 * StopBurstShot command�� ������ numBurst ��ŭ jpeg callback�� �Ϸ�
 * �� �� cancelPicture�� ȣ�� �Ͽ� BurstShot�� ���� �� �� �ִ�.
*/
#define F_PANTECH_CAMERA_CUST_STOP_BURSTSHOT

/* 
 * burst shot�� ��� �Կ����� CameraService���� �߻����� �ʰ�,
 * UI���� �Կ����� ����ϵ��� �Ѵ�.
 * "pantech-burst-sound" �� "disable" �� ������ �ϸ� burst shot
 * �� ��� �Կ����� ������� �ʴ´�. ��, shutter callback�� �߻�
 * �Ѵ�.
*/
#define F_PANTECH_CAMERA_CUST_BURST_UI_SOUND
#endif

/* 
 * CameraHAL���� ISP(VFE) ���� �ʱ� ���� �����Ѵ�.
 * initDefaultParameters() �� ���� �� Pantech�� default ��翡 �°� ������ �κ� ��
 * ��Ȯ�� ���� �� ī�޶� ���� �ӵ� ����ȭ�� ���� UI�� default �� �׸��� ISP(VFE)��
 * default ���� ���� ��ġ�ϵ��� �Ͽ��� �Ѵ�.
 * Ư�� ISP(VFE)�� �ʱⰪ�� ��ġ�ϴ��� Ȯ�� �Ͽ� ���� �Ѵ�. */
#define F_PANTECH_CAMERA_CUST_INIT_DEFAULT_PARAM

 /* 
 * Android Gingerbread ���� ���������� "preview-frame-rate" ���� fixed fps�� ���� �Ͽ���.
 * Android Gingerbread ���� ���ʹ� "preview-fps-range-values" �� �߰� �Ǿ�����, min_fps 
 * �� max_fps�� ���� �ϵ��� �Ѵ�. 
 * "preview-fps-range-values"�� ���� frame rate�� ���� �� �� ������, min, max�� ���� ���
 * ���� fixed fps�� ������ �ȴ�.
 * ���� "preview-fps-range-values"�� ��� �� ���� ���� ������ ������ "preview-frame-rate"
 * ���� ������ �ϰ� �� �Ķ���͸� ���� ���µ� �浹�� ������ ���� �κ��� ���� �Ѵ�. */
#define F_PANTECH_CAMERA_CUST_FPS

/*
 * ASD mode ���ý� ��� manual scene mode detect �����ϵ��� ���� SBA ����
 * EIS �� ���������� ASD������ GYRO�� �̿��Ѵ�.
 * /vendor/qcom/proprietary/mm-camera/Android.mk ����
 * FEATURE_GYRO �� true�� ���� �Ǿ����� Ȯ�� �Ѵ�.
*/
/* #define F_PANTECH_CAMERA_CUST_ADD_ASD */

#define F_PANTECH_CAMERA_CUST_VT_TUNING

/*
*/
/* #define F_PANTECH_CAMERA_CUST_AF_FLASH */
#ifdef F_PANTECH_CAMERA_CUST_AF_FLASH
#define F_PANTECH_CAMERA_CUST_AF_FLASH_QBUG
#endif

 /* 
 *  VT�� �����信�� ����� �ϴ� preview �ػ󵵸� �߰� ���� �Ѵ�.
 *  VT������ LCD�� ���� ������ �ػ󵵸� ����ϵ��� ������ preview �ػ󵵿� �߰� ����
 *  �ʰ� ���� table�� ����� ����Ѵ�.
 *  ���� pantech�� VT app������ �ػ󵵸� ��� �Ѵ�. */
#define F_PANTECH_CAMERA_FIX_VT_PREVIEW

/* 
 * stagefright�� ���ڵ� �� �������� KT���� ������ �ȵǴ� ����
 * ������ Ʈ�� ����� "pasp" �κ��� KT�������� �Ľ��� ���ϴ� ������ �������� SKT�� LG�⿡���� ���������� ������ �Ǿ���
 * KT���� ��� ���� ������ �Ұ��� ��Ȳ�� ����Ͽ� ����� �ش� �κ��� ���� �ʵ��� ��. �� �� KT���� ���� �� �� �ִ� ����
 * pasp �� ��� ���� �����Ƿ� ��Ż� ��� ���� �������� �Ѵ�.
 */
#define F_PANTECH_CAMERA_FIX_MMS_PASP

/* SKYķ�ڴ� ��ȭ������ Qparser�� Ȯ�ν� �����߻�.(deocde thumnail�Ҽ�����)
 * ķ�ڴ� ���ڵ��� ����ķ�ڴ��� SKYķ�ڴ��� ������ ��??��
 * app���� �������� stagefrightrecorder�� mMaxFileDurationUs ���̴�.
 * (SKYķ�ڴ�: 3600000000(us)=1�ð� / ����ķ�ڴ�: 600000000(us)=10��.)
 * mMaxFileDurationUs�� ���̷����� Mpeg4write����
 * SKYķ�ڴ��� 64bitfileoffset / ����ķ�ڴ��� 32bitfileoffset�� ����ϰ� �ȴ�.
 * �̸� 32bitfileoffset���� �����ϰ� �����ϱ� ���ؼ� �ش�κ��� �����Ѵ�.
 * �ӽ÷� �����Ǵ� �κ��̹Ƿ� �߰� ���� �� �������� ����͸��� �ʿ���.
 * EF52 ���Ŀ����� max recording size�� 4GB�� �ø��� ���� 64bitfileoffset���� ����
 * �Ͽ�����, �ٸ�, MMS�� ��쿡�� 32bitfileoffset ���� recording �ϵ��� �Ѵ�.
*/
#define F_PANTECH_CAMERA_VIDEO_REC_FILEOFFSET

/* #4648# �÷��� LED �׽�Ʈ�� ���� �÷��� ��带 �߰� �Ѵ�.
 * FLASH_MODE_TORCH_FLASH ��带 ������ ��� TORCH ���� ���� manual�� flash�� on
 * �Ѵ�. �̶� torch�ʹ� �޸� flash mode�� on�� �ϱ� ������ 500ms �ڿ� �ڵ����� flash
 * �� off �ǵ��� �Ѵ�. */
#define F_PANTECH_CAMERA_FIX_CFG_LED_MODE

/* �Կ� �ӵ� ������ ���� sound file load ������ camera client �����ð� �ƴ϶� 
 * camera service �����÷� �ű�
*/
#define F_PANTECH_CAMERA_MOVE_LOAD_SOUND

/*
 * WFD(wifi-display)�� ����� �ϴ� ��� ���� �Ѵ�.
 * WFD ����ϴ� ��� ��� ������ ����Ʈ�� ���۵Ǿ� �� ��� ī�޶� �Կ����� ���� �Կ��� ��
 * ���� ������ �Ҹ��� ���� �ʰ� ����Ʈ �ܸ����� �Ҹ��� ��
 * ī�޶� �������� �׻� �Կ��� ������ �Ҹ��� ���� �ϹǷ�, ���ÿ��� �Ҹ������� ���� �ڵ� ����
*/
#define F_PANTECH_CAMERA_PLAYSOUND_IN_WFD

/* MDM ��å�� ���� ī�޶� ���� ���θ� �����Ѵ� */
#define F_PANTECH_CAMERA_MDM_CHECK

/*----------------------------------------------------------------------------*/
/*  PANTECH ADD FEATURE                                                       */
/*  ���� Device �Ǵ� �𵨿� Ư���� ���� ��� �Ǵ� API �߰�                    */
/*----------------------------------------------------------------------------*/

/* ������ ��ȭ �� recordig file size �� app ���� ������ �ش�. */
#define F_PANTECH_CAMERA_ADD_EVT_RECSIZE

/*
 * jpeg file exif ���� �߰�
*/
#define F_PANTECH_CAMERA_ADD_EXIF_DATA

/*Morpho HDR���� ����ϴ� feature
 *
 * HDR on���� ������ ��� 3 ���� ������ ����Ͽ� HDR processing�� �Ѵ�.
*/
#ifndef F_PANTECH_CAMERA_TARGET_NAMI
#define F_PANTECH_CAMERA_CFG_HDR

/*
 * HDR �Կ� �� thumbnail �� main image �� ���� �����.
 * ��Ȥ main image �� thumbnail �� �ٸ� ��찡 ���ܼ� �̿� ���� ���� ��.
 */
#define F_PANTECH_CAMERA_HDR_THUMBNAIL
#endif

/* F_PANTECH_CAMERA_TODO, SKT FOTA DCMO (Device Control Management Object)
 * SKT �⿡�� ����Ǹ�, UI VOB������ define�� ����.
 * "pantech/development/sky_fota/sky_fota.h" ������ �־�� �Ѵ�.
*/
/* #define F_PANTECH_CAMERA_FOTA_DCMO_CAMERA_LOCK */

/* AOT(Always On Top) CAMERA �߰� ����.
 * ī�޶� ���ۻ��¸� üũ�ϵ��� CameraService::isRunning() �Լ� �߰�.
*/
#define F_PANTECH_CAMERA_AOT_ISRUNNING

/* F_PANTECH_CAMERA_TODO
 * �����ν� ���� Feature.
 * Preview buffer�� frame ���� �ʵ��� ����
*/
#if defined(F_PANTECH_CAMERA_EF59S_59K_59L) || defined(F_PANTECH_CAMERA_EF60S_61K_62L)
#define F_PANTECH_CAMERA_VTS
#endif

 /*
  * MSM8974 ���� ���� ī�޶� �Կ� �� thumbnail �� ������ �ٸ��� �����Ǿ� ����Ǵ� ���� ����
  */
#define F_PANTECH_CAMERA_THUMBNAIL_FLIP

/* 
 * Flip Cover�� window���� ���� �Ǵ� SmartFlipCamera�� �����ϱ� ���� ���.
 * window�� size �� ������ �Ϲ������� ��� �Ǵ� ���� �ƴϹǷ�, preview/snapshot/recording 
 * table�� �ػ󵵸� �߰� ���� �ʰ� SmartFlipCamera ���ÿ����� ��� �� �� �ֵ��� �Ѵ�.
 * window�� size�� �𵨺��� �Ǵ� ���� �� ���� �� ���� �����Ƿ� ������ ���� �� �ػ󵵸� UI
 * ���� ���������� ������ �ߴٰ� ������ �ϰ� �ػ󵵸� ���� check ���� �ʵ��� �Ѵ�.
 * ���� �Ķ���� - "pantech-window-camera" : "enable"/"disable"
*/
#define F_PANTECH_CAMERA_CUST_FLIP_COVER_CAMERA

 /*
  * ������ ī�޶� ���Խ� ���� ī�޶� ���� �ϵ����ϰ� 
  * flash app�� ��� ī�޶� ���Խ� flash ���� �ϵ��� ����.
 */
#define F_PANTECH_CAMERA_DEVICE_CONNECT_FAIL_FIXED

/*
* ���� PNG file �� decoding �� ��, skia decoder ���� fail�� �߻� �ϰ�,
* �̷� ���� �ַ������� ������ ���� �Ǵ� ����
*/
#define F_PANTECH_CAMERA_FIX_PNG_DECORDING_ERROR_WIH_BROKEN_DATA

/*----------------------------------------------------------------------------*/
/*  Qualcomm BUGS                                                             */
/*  Qualcomm bug�� �ַ� SR, ��ġ�� ����.                             */
/*  �� �� Qualcomm main stream�� �ݿ����� �ֽð� �ʿ� ��.                     */
/*----------------------------------------------------------------------------*/

/* case#01169476 
 * ��/�ĸ� ī�޶� ���ÿ� ��� �ϱ� ���� MSM8974���� Dual VFE�� �����Ѵ�. 
 * Dual VFE ��� �� simultaneous camera feature�� �߰� �Ǿ�����, ���� �� 
 * 1023 release�� bug�� �־� �������� ���� ��ġ�� �޾� ���� �Ͽ���.
 * Ŀ�� ������ ���� ������ �������� Ȯ�� �����ϴ�.
 * #if(n)def CONFIG_PANTECH_CAMERA // case#01169476 simultaneous camera
 */
#define F_PANTECH_CAMERA_QBUG_SIMULTANEOUS_CAM

/* case#01187023 
 * 13M@24fps ZSL�� �����ϱ� ���� 8974�� highperformance(dual vfe)��
 * �����ϸ鼭 �߻��� ������ �����ϱ� ���� ������ �ڵ�
 * �̽� : ZSL OFF/ON ������ VFE0�� VFE1�� frame sync�� ���� �ʴ� ����
 *
 * 1 : 2013-05-14 : �̽� ������ ������ CASE�� ���� ���� �ڵ�
 */
#define F_PANTECH_CAMERA_QBUG_HIGH_PERFORMANCE

/* 
 * ZSL ĸ�ĵ� �̹����� ������ ���� ���� ����.
 * ���� �����Ӱ� ���� �������� �߰��� �߷��� �ϳ��� frame���� ���� �Ǵ� ���� ��.
*/
#define F_PANTECH_CAMERA_FIX_QBUG_BROCKEN_IMAGE

/*
*/
#define F_PANTECH_CAMERA_QBUG_JPEG_WBUF_SIZE

/*
 * ���� PNG file �� decoding �� ��, skia decoder ���� fail�� �߻� �ϰ�,
 * �̷� ���� �ַ������� ������ ���� �Ǵ� ����
*/
#define F_PANTECH_CAMERA_FIX_PNG_DECORDING_ERROR_WIH_BROKEN_DATA

/*
 * Snapshot dump img �� ���η� �Կ� �� ���������� ������ �ʴ� ���� ����
*/
#define F_PANTECH_CAMERA_QBUG_DUMP_IMAGE

/*
* 2013/12/02, Ha Dong Jin
* EF61K PLM#664, CASE#1349270, CR#522939
* Clean-up the msm camera generic buff queue on userspace
* crash to avoid invalid memory access in next session
* which might lead to corruption in other modules
* and system stability issues and also avoid memory leak.
*/
#define F_PANTECH_CAMERA_QBUG_CLEANUP_MSM_GENERIC_BUF


/*
 * Effect Recording �����ϸ� Effect �� ������� ��ȭ�Ǵ� ���� SR �� ���� case# 01215481
*/
#define F_PANTECH_CAMERA_QBUG_EFFECT_RECORDING

/*
 * nonZSL mode ���� burstshot �Կ��� ������ snapshot ������ ����
 * capture callback�� �߻��ϴ� �̽�
 * VegaCamera������ HDR �Կ��� 3�� �� 2�常 capture�Ǿ� ���� �߻�
 * �Ͽ�����, preview �Ǵ� snapshot �ػ󵵰� Ŭ ���� ���� �� ����.
*/
//#define F_PANTECH_CAMERA_QBUG_NONZSL_SNAPSHOT_BURST      // F_PANTECH_CAMERA_QBUG_SKIP_INVALID_SOF_IN_DUALISP �� ��ü��

/*
 * nonZSL ���� ī�޶� ���Խ� AF calibration table update�� ���� �ʴ�
 * ������ ���� ����.
*/
#define F_PANTECH_CAMERA_QBUG_AF_LOAD_FAIL_NONZSL

/*
 * focus mode�� "continuous-video" ���� "auto"�� ���� �Ͽ��� ��,
 * continuous af�� stop ���� �ʴ� ���� ����.
 * camcorder caf file�� ���� algo type�� �߸� load �ϰ� �Ǹ�, ��
 * ������ caf_state ������ caf�� stop ���� �ʴ� ���� ��.
*/
#define F_PANTECH_CAMERA_QBUG_CONTINUOUS_VIDEO_STOP


#ifdef F_PANTECH_CAMERA_IMX135
/*
 * IMX135 sensor���� �����ϴ� ������HDR ��� ����� imx135_fill_awb_hdr_array
 * ȣ�� ���� �ʴ� ���� ����
*/
#define F_PANTECH_CAMERA_QBUG_MOVIE_HDR

/*
 * IMX135 sensor���� �����ϴ� ������HDR ��� ����ȭ�� ����
 * customizing
*/
#define F_PANTECH_CAMERA_CUST_MOVIE_HDR
#endif

/*
 * ī�޶� ĸ�Ľ� shutter callback �����Ͽ� ������ �ڵ� �κ� ����
 * shutter callback�� sound play ���θ� �Ǵ� �Ͽ� 1ȸ�� ���Ͽ� 
 * callback �� �ǵ��� ���� �Ѵ�.
*/
#define F_PANTECH_CAMERA_QBUG_SHUTTER_CALLBACK

/*
 * ī�޶� �Կ� �� Home Key�� ���� stopPreview command �߻��� ī�޶� halt �Ǵ�
 * ���� ����
 * OMX Jpeg encoder release �� halt �Ǵ� ����
*/
#define F_PANTECH_CAMERA_QBUG_STOP_ON_SNAPSHOT

/*
 *Start preview ���� flash or movie mode ������ ���� �ȵǴ�
 *���� workaround ����
 */
#define F_PANTECH_CAMERA_QBUG_LED_MODE

/* ������ ��ȭ ����/���Ḧ ������ �ݺ��ϰų�, �̾����� ������ ���¿��� �����Կ�
 * ���� �Կ��� ���, MediaPlayer �� �������ϸ鼭 HALT �߻��Ѵ�.
 *
 * MediaPlayer �� ���, ������ ������ ��� �߿� �� �ٽ� ��� �õ��� ��� 100%
 * �������ϹǷ� ���� ������ �����Ͽ� ����ؾ� �� ���, �ݵ�� ���� ����� �Ϸ�
 * �Ǿ����� ���θ� Ȯ�� �� ����ؾ� �Ѵ�. */
#define F_PANTECH_CAMERA_QBUG_SKIP_CAMERA_SOUND

/*
* �ʱ� ���Խ� Sharpness ������� �ʴ� ���� ����.
*/
#define F_PANTECH_CAMERA_QBUG_SHARPNESS

/*
 *CPL ��ġ ���� ī�޶� ���� �Կ� �ݺ� �׽�Ʈ �� �߻��ϴ� ���� ���� 2013.10.28
 * case# 01332004  , 01338442
 */
#define F_PANTECH_CAMERA_QBUG_FIX_CPL_BUG

#define F_PANTECH_CAMERA_QBUG_FIX_GOOFY_FACE_RECORD

/*
* Recording ���۽� Effect ������� �ʴ� ���� ����.
*/
#define F_PANTECH_CAMERA_QBUG_EFFECT

/*
* Face Effect ��� ���� ����.
*/
#define F_PANTECH_CAMERA_QBUG_FACE_EFFECT

/*
* CPL patch�� camera start/stop preview���� map/unmap �� hold �Ǵ� ����
*/
#define F_PANTECH_CAMERA_QBUG_HOLD_DURING_MAP_UNMAP

/*
 * CPL patch �� livesnapshot ��  �Կ����� �ʴ� ���� ����
 */
 #define F_PANTECH_CAMERA_QBUG_LIVESNAPSHOT

 /*
 * Non-ZSL�� liveshot���� ���η� �Կ��� thumbnail�� 90�� ���ư��� ���� ����.
 */
#define F_PANTECH_CAMERA_QBUG_THUMBNAIL_ROTATION

/*
* [2013/12/09, p16445 - lee.sangwon, case#01382932]
* burst shot Mode ���� ª�� Ű�� ���� 20���� ��� ������ ���, ���� ANR �߻�
* �Ҵ�� QCameraStreamMemory �� deallocate �� ��, postproc_channel_cb_routine() �Լ��� ó���Ǿ� �߻��Ǵ� ���� ����
* deallocate ���Ŀ� m_bActive ������ ���� Ȯ���Ͽ� ó������ �ʵ��� ��.
*/
#define F_PANTECH_CAMERA_QBUG_CHECK_DATA_PROC_THREAD_FOR_DEALLOCATION

/*
[2013/12/13, p16445 - lee.sangwon, CRs-Fixed: 560898]
Both 'getParameter()' and 'setParameter()' in 'QCameraStream' can be called from different contexts. 
They do share a parameter buffer and the access to it needs to be synchronized in order to avoid concurrency issues.
*/
#define F_PANTECH_CAMERA_QBUG_SYNC_ACCESS_TO_STREAM_PARAMETERS

/*
* [2013/12/16, p16445 - lee.sangwon, case#01341742]
* DUAL ISP ���� ������ �ٲ� meta data flame index �� ���� ��쿡 ���� workaround.
* burst shot ���� �߻��Ǵ� ����(Failed to get_buf) �� �ذ�
* F_PANTECH_CAMERA_QBUG_SKIP_INVALID_SOF_IN_DUALISP (official patch) ������, ����
*/
//#define F_PANTECH_CAMERA_QBUG_WORKAROUND_DUALISP_ISSUE_IN_ZSL

/*
* [2013/12/16, p16445 - lee.sangwon, case#01341742]
* DUAL ISP ���� ������ �ٲ� meta data flame index �� ���� ��쿡 ���� QCOM patch.
* burst shot ���� �߻��Ǵ� ����(Failed to get_buf) �� �ذ�
* SOF event �߻��� ���� SOF ���� id ���� Ŭ��츸 valid SOF �� �Ǵ� �ϵ��� ����
* ������ QCom ��ġ�� F_PANTECH_CAMERA_QBUG_NONZSL_SNAPSHOT_BURST �� ������
*/
#define F_PANTECH_CAMERA_QBUG_SKIP_INVALID_SOF_IN_DUALISP

/*
* 2013/12/19/ case : 01388126
* Qualcomm fixes for known FD leakage issues in camera side
*/
#define F_PANTECH_CAMERA_QBUG_FD_LEAKAGE

/*
* 2013/12/19/ case : 01388126
* Qualcomm fix memory leak in luma adaptation 
*/
#define F_PANTECH_CAMERA_QBUG_LUMA_LEAKAGE

/*
* 2013/12/19/ case : 01388126
* Qualcomm ISP Fix for memory leak  
*/
#define F_PANTECH_CAMERA_QBUG_ISP_LEAKAGE

/*
* [2013/12/20, p16445 - lee.sangwon, case#01358096]
* Qualcomm patch that resolves the ion unmapping issue by modifying the mutex for isp buffer manager
*/
#define F_PANTECH_CAMERA_QBUG_MUTEX_FOR_ISP_BUFMGR

/*----------------------------------------------------------------------------*/
/*  MODEL-SPECIFIC CONSTANTS                                                  */
/*  �� ���� ��� ����                                              */
/*----------------------------------------------------------------------------*/

/* camera select enum */
#define C_PANTECH_CAMERA_SELECT_MAIN_CAM 		0
#define C_PANTECH_CAMERA_SELECT_SUB_CAM 		1


//P11232 US4 CAMERA
#define F_PANTECH_CAMERA_CFG_IPL_SKY_PROCESSING
#ifdef F_PANTECH_CAMERA_CFG_IPL_SKY_PROCESSING
#define F_PANTECH_CAMERA_CFG_CAMNOTE
#define F_PANTECH_CAMERA_CFG_MINIATURE
#define F_PANTECH_CAMERA_CFG_COLOREXTRACTION
#endif


#ifdef F_PANTECH_CAMERA_CUST_OEM_EXIF_TAG
#define C_PANTECH_CAMERA_EXIF_MAKE		"PANTECH"
#define C_PANTECH_CAMERA_EXIF_MAKE_LEN		8		/* including NULL */
#ifdef F_PANTECH_CAMERA_TARGET_EF39S
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A800S"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF40S
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A810S"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF40K
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A810K"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_PRESTO
#define C_PANTECH_CAMERA_EXIF_MODEL		"PRESTO"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		7		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF45K
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A830K"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF46L
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A830L"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF47S
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A830S"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_SVLTE
#define C_PANTECH_CAMERA_EXIF_MODEL		"SVLTE"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		6		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_CSFB
#define C_PANTECH_CAMERA_EXIF_MODEL		"CSFB"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		5		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_CHEETAH
#define C_PANTECH_CAMERA_EXIF_MODEL		"CHEETAH"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		8		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_STARQ
#define C_PANTECH_CAMERA_EXIF_MODEL		"STARQ"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		6		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_OSCAR
#define C_PANTECH_CAMERA_EXIF_MODEL		"OSCAR"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		6		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_VEGAPVW
#define C_PANTECH_CAMERA_EXIF_MODEL		"ADR930L"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		8		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_ZEPPLIN
#define C_PANTECH_CAMERA_EXIF_MODEL		"ZEPPLIN"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		8		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_SIRIUSLTE
#define C_PANTECH_CAMERA_EXIF_MODEL		"SIRIUSLTE"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		10		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF44S
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A840S"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_MAGNUS
#define C_PANTECH_CAMERA_EXIF_MODEL		"P9090"		//MAGNUS
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		6		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF48S
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A850S"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF49K
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A850K"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF50L
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A850L"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF51S
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A860S"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF51K
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A860K"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF51L
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A860L"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF52S
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A851L"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF52K
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A851L"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF52L
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A851L"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF52W
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A851L"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF56S
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A880S"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF57K
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A880K"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF58L
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A880L"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF59S
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A890S"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF59K
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A890K"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF59L
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A890L"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF60S
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A900S"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF61K
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A900K"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF62L
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A900L"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_NAMI
#define C_PANTECH_CAMERA_EXIF_MODEL		"NAMI"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		5		/* including NULL */
#endif
#endif

#endif /* F_PANTECH_CAMERA */

#endif /* CUST_PANTECH_CAMERA.h */
