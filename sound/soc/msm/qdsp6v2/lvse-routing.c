/************************************************************************/
/*  Copyright (c) 2012-2013 NXP Software. All rights are reserved.      */
/*  Reproduction in whole or in part is prohibited without the prior    */
/*  written consent of the copyright owner.                             */
/*                                                                      */
/*  This software and any compilation or derivative thereof is and      */
/*  shall remain the proprietary information of NXP Software and is     */
/*  highly confidential in nature. Any and all use hereof is restricted */
/*  and is subject to the terms and conditions set forth in the         */
/*  software license agreement concluded with NXP Software.             */
/*                                                                      */
/*  Under no circumstances is this software or any derivative thereof   */
/*  to be combined with any Open Source Software in any way or          */
/*  licensed under any Open License Terms without the express prior     */
/*  written  permission of NXP Software.                                */
/*                                                                      */
/*  For the purpose of this clause, the term Open Source Software means */
/*  any software that is licensed under Open License Terms. Open        */
/*  License Terms means terms in any license that require as a          */
/*  condition of use, modification and/or distribution of a work        */
/*                                                                      */
/*           1. the making available of source code or other materials  */
/*              preferred for modification, or                          */
/*                                                                      */
/*           2. the granting of permission for creating derivative      */
/*              works, or                                               */
/*                                                                      */
/*           3. the reproduction of certain notices or license terms    */
/*              in derivative works or accompanying documentation, or   */
/*                                                                      */
/*           4. the granting of a royalty-free license to any party     */
/*              under Intellectual Property Rights                      */
/*                                                                      */
/*  regarding the work and/or any work that contains, is combined with, */
/*  requires or otherwise is based on the work.                         */
/*                                                                      */
/*  This software is provided for ease of recompilation only.           */
/*  Modification and reverse engineering of this software are strictly  */
/*  prohibited.                                                         */
/*                                                                      */
/************************************************************************/

/*****************************************************************************************

     $Author: nxp47058 $
     $Revision: 37504 $
     $Date: 2013-03-04 15:55:50 +0900 (¿ù, 04 3 2013) $

*****************************************************************************************/

//FIXME remove junk includes

#include <linux/init.h>
#include <linux/err.h>
#include <linux/mutex.h>
#include <linux/wait.h>
#include <linux/atomic.h>
#include <sound/soc.h>
//#include <sound/q6asm.h>
//#include "msm-pcm-routing-v2.h"
#include "lvse-routing.h"
#include <sound/apr_audio-v2.h>
#include <sound/q6asm-v2.h>

LVMQDSP_SessionContext_t gSessions;


#ifdef ENABLE_PARAM_DUMP
void lvse_dump_control_params(void* ptr);
void lvse_dump_tuning_params(void* ptr);
#endif /* ENABLE_PARAM_DUMP */

int q6asm_set_pp_params(struct audio_client *ac, u32 module_id, u32 param_id,
            void *pp_params, int pp_size);

int q6asm_get_pp_params(struct audio_client *ac, u32 module_id, u32 param_id,
            int pp_size);

int lvse_pcm_close_session(LVMQDSP_Session_t session);
int lvse_pcm_open_session(LVMQDSP_Session_t session);

void setClientId(int id)
{
	pr_debug("set Client Id = %d\n", id);

	gSessions.clientId = id;

	/* close session */
	if(id == -1)
	{
		pr_debug("\nLVSE: setClientId - close session");
		if(gSessions.ac == NULL)
			pr_err("\nLVSE: setClientId - close session - context should not be null");
		
        lvse_pcm_close_session(id);
	}
	else /* open_session */
	{
		pr_debug("\nLVSE: setClientId - opening session");
		if(gSessions.ac != NULL)
		{
			pr_err("\nLVSE: setClientId - opening session - context should be null");
			lvse_pcm_close_session(0);
		}
		
        lvse_pcm_open_session(id);
	}

}



//----------------------------------------------------------------------------
// lvse_callback()
//----------------------------------------------------------------------------
// Purpose: This function is called from QDSP to inform various event.
//
// Inputs:
//  opcode		Event command
//  token
//  payload		Callback data
//  priv		User data
//
// Outputs:
//
//----------------------------------------------------------------------------
void lvse_callback(uint32_t  opcode,
                   uint32_t  token,
                   uint32_t *payload,
                   void     *priv) {
    struct asm_get_pp_params_cmdrsp* params;
    struct pcm *pcm;

    if(gSessions.ac == NULL)
        return;

    pcm = &gSessions.pcm;

    pr_debug("%s:opcde = %d token = 0x%x\n", __func__, opcode, token);

    switch (opcode) {
    case ASM_DATA_EVENT_WRITE_DONE_V2:
        pr_debug("LVSE: ASM_DATA_EVENT_WRITE_DONE token=%d", token);
        break;
    case ASM_DATA_EVENT_READ_DONE_V2:
        pcm->read_frame_info[token][0] = payload[7];
        pcm->read_frame_info[token][1] = payload[3];
        atomic_inc(&pcm->read_count);
        pr_debug("LVSE: ASM_DATA_EVENT_READ_DONE token=%d, read_count=%d", token, atomic_read(&pcm->read_count));
        wake_up(&pcm->read_wait);
        break;
    case ASM_DATA_EVENT_EOS:
    	pr_debug("ASM_DATA_EVENT_EOS");
    	break;
    case 0x000110E8:
    	pr_debug("APRV2_IBASIC_RSP_RESULT");
    	break;
    case ASM_STREAM_CMDRSP_GET_PP_PARAMS_V2:
    {

        params = (struct asm_get_pp_params_cmdrsp*)payload;
        pr_debug("input param size=%d", params->params.param_size);
        
        pr_debug("lvse_callback: count increment"); 
        atomic_inc(&gSessions.count);

		pr_debug("lvse_callback result   = %d", params->cmdrsp.status);
		pr_debug("lvse_callback param_id = %d", params->params.param_id);

        switch (params->params.param_id)
        {
            case PARAM_ID_NXP_LVM_INSTANCE_CTRL:
            {
                pr_debug("PARAM_ID_NXP_LVM_INSTANCE_CTRL");
                pr_debug("output param size=%d", sizeof(LVMQDSP_InstParams_t));
                memcpy(&gSessions.inst_params, &(params->data), sizeof(LVMQDSP_InstParams_t));
                break;
            }
            case PARAM_ID_NXP_LVM_EFFECT_CTRL:
            {
                pr_debug("PARAM_ID_NXP_LVM_EFFECT_CTRL");
                pr_debug("output param size=%d", sizeof(LVMQDSP_ControlParams_t));
                memcpy(&gSessions.control_params, &(params->data), sizeof(LVMQDSP_ControlParams_t));
#ifdef ENABLE_PARAM_DUMP
                lvse_dump_control_params(&gSessions.control_params);
                lvse_dump_control_params(&(params->data));
#endif /* ENABLE_PARAM_DUMP */
                break;
            }
            case PARAM_ID_NXP_LVM_HEADROOM_CTRL:
            {
                pr_debug("PARAM_ID_NXP_LVM_HEADROOM_CTRL");
                memcpy(&gSessions.headroom_params, &(params->data), sizeof(LVMQDSP_HeadroomParams_t));
                break;
            }
            case PARAM_ID_NXP_LVM_TUNING_CTRL:
            {
                pr_debug("PARAM_ID_NXP_LVM_TUNING_CTRL");
                memcpy(&gSessions.tuning_params, &(params->data), sizeof(LVMQDSP_Custom_TuningParams_st));
                break;
            }
            case PARAM_ID_NXP_LVM_GET_SPECTRUM_CTRL:
            {
                pr_debug("PARAM_ID_NXP_LVM_GET_SPECTRUM_CTRL");
                memcpy(&gSessions.spectrum_params, &(params->data), sizeof(LVMQDSP_PSA_SpectrumParams_st));
				break;
            }
			case PARAM_ID_NXP_LVM_REVERB_INSTANCE_CTRL:
			{
				pr_debug("PARAM_ID_NXP_LVM_REVERB_INSTANCE_CTRL");
				pr_debug("output param size=%d", sizeof(LVMQDSP_RevInstParams_t));
				memcpy(&gSessions.reverb_inst_params, &(params->data), sizeof(LVMQDSP_RevInstParams_t));
				break;
			}
			case PARAM_ID_NXP_LVM_REVERB_EFFECT_CTRL:
			{
				pr_debug("PARAM_ID_NXP_LVM_REVERB_EFFECT_CTRL");
				pr_debug("output param size=%d", sizeof(LVMQDSP_ControlParams_t));
				memcpy(&gSessions.reverb_control_params, &(params->data), sizeof(LVMQDSP_RevControlParams_t));
				break;
			}
			
            default:
                pr_debug("Unknown param_id %d", params->params.param_id);
        }
        
        wake_up(&gSessions.wait);
        pr_debug("lvse_callback: wake_up");           
        
        break;
    }
    default:
        pr_debug("%s:Unhandled event = 0x%8x", __func__, opcode);
        break;
    }
}

//----------------------------------------------------------------------------
// lvse_pcm_open_session()
//----------------------------------------------------------------------------
// Purpose: Open new session, q6asm client and set up the new topology.
//          Allocate the memory which to read and write between kernel and QDSP.
//
// Inputs:
//  session     audio session id
//
// Outputs:
//
//----------------------------------------------------------------------------
int lvse_pcm_open_session(LVMQDSP_Session_t session)
{
    // if new session
    if (gSessions.ac == NULL) {
    
        pr_debug("\nLVSE: init gSessions[%d]", session);
        mutex_init(&gSessions.lock);
        init_waitqueue_head(&gSessions.wait);
        atomic_set(&gSessions.count, 0);
        init_waitqueue_head(&gSessions.pcm.read_wait);
        atomic_set(&gSessions.pcm.read_count, 0);

		pr_debug("lvse:lvse_pcm_open_session - client id = %d", gSessions.clientId);

		gSessions.ac = q6asm_get_audio_client(gSessions.clientId);
		if(gSessions.ac == NULL){
			pr_info("can't get the client");
			return LVMQDSP_ERROR;
		}
    }
	return LVMQDSP_SUCCESS;
}

//----------------------------------------------------------------------------
// lvse_pcm_close_session()
//----------------------------------------------------------------------------
// Purpose: Close the session and free the q6asm client.
//
// Inputs:
//  session     audio session id
//
// Outputs:
//
//----------------------------------------------------------------------------
int lvse_pcm_close_session(LVMQDSP_Session_t session)
{
    pr_debug("lvse:lvse_pcm_close_session start");
	if(gSessions.ac != NULL)
	{

		mutex_lock(&gSessions.lock);
		mutex_unlock(&gSessions.lock);
		mutex_destroy(&gSessions.lock);
		
		// close stream
		gSessions.ac = NULL;
		
		// clean memory
		memset(&gSessions, 0, sizeof(LVMQDSP_SessionContext_t)); 
		atomic_set(&gSessions.count, 0);
		
	}
    pr_debug("lvse:lvse_pcm_close_session end");

    return LVMQDSP_SUCCESS;
}

//----------------------------------------------------------------------------
// lvse_routing_set_control()
//----------------------------------------------------------------------------
// Purpose: Set parameter to the QDSP. Processing is happened here.
//
// Inputs:
//  kcontrol
//  ucontrol      	User data to set
//
// Outputs:
//
//----------------------------------------------------------------------------
int lvse_routing_set_control(struct snd_kcontrol       *kcontrol,
                             struct snd_ctl_elem_value *ucontrol) {

    int status = 0;
    LVMQDSP_Command_t *lvm;
    LVMQDSP_Session_t session;
    struct audio_client *lvse_ac;
    
    lvm = (LVMQDSP_Command_t*)ucontrol->value.bytes.data;
    pr_debug("LVSE: cmd = %d, ###############################################\n", lvm->cmd);

    session = lvm->session;

    switch(lvm->cmd)
    {
        case LVMQDSP_OPEN_SESSION:
            pr_debug("LVSE: LVMQDSP_OPEN_SESSION"); 
			if(gSessions.ac != NULL)
				return LVMQDSP_SUCCESS;
			else
				return LVMQDSP_ERROR;

        case LVMQDSP_CLOSE_SESSION:
        {
            pr_debug("LVSE: LVMQDSP_CLOSE_SESSION");

            return LVMQDSP_SUCCESS;
        }    
    }

	if (gSessions.ac == NULL)
	{
		pr_err("\nLVSE: audio client is NULL %d", session);
		return LVMQDSP_ERROR;
	}

    pr_debug("LVSE: lock\n");
    mutex_lock(&gSessions.lock);
    lvse_ac = gSessions.ac;    

    switch(lvm->cmd)
    {
        case LVMQDSP_SET_INSTANCE_PARAMS:
        {
            LVMQDSP_InstParams_t instParams;

            pr_debug("LVSE: LVMQDSP_SET_INSTANCE_PARAMS - size=%d \n",sizeof(LVMQDSP_InstParams_t));
            if(copy_from_user(&instParams, lvm->cmdParams, sizeof(LVMQDSP_InstParams_t)))
            {
                pr_err("LVSE: LVMQDSP_SET_INSTANCE_PARAMS - read error \n");
                break;
            }
            status = q6asm_set_pp_params(lvse_ac, MODULE_ID_NXP_LVM, PARAM_ID_NXP_LVM_INSTANCE_CTRL,
                (void*)&instParams, sizeof(LVMQDSP_InstParams_t));
            pr_debug("LVSE: q6asm_set_pp_params returned %d", status);
            break;
        }
        case LVMQDSP_SET_CONTROL_PARAMS:
        {
            LVMQDSP_ControlParams_t params;

            pr_debug("LVSE: LVMQDSP_SET_CONTROL_PARAMS - size=%d, %d \n",sizeof(LVMQDSP_ControlParams_t), sizeof(params) );
            if(copy_from_user(& params, lvm->cmdParams, sizeof(LVMQDSP_ControlParams_t)))
            {
                pr_err("LVSE: LVMQDSP_SET_CONTROL_PARAMS - read error \n");
                break;
            }
            status = q6asm_set_pp_params(lvse_ac, MODULE_ID_NXP_LVM, PARAM_ID_NXP_LVM_EFFECT_CTRL,
                (void*)&params, sizeof(LVMQDSP_ControlParams_t));
            pr_debug("LVSE: q6asm_set_pp_params returned %d", status);
#ifdef ENABLE_PARAM_DUMP
            lvse_dump_control_params(&params);
#endif /* ENABLE_PARAM_DUMP */
            break;
        }
        case LVMQDSP_SET_HEADROOM_PARAMS:
        {
            LVMQDSP_HeadroomParams_t params;

            pr_debug("LVSE: LVMQDSP_SET_HEADROOM_PARAMS - size=%d, %d \n",sizeof(LVMQDSP_HeadroomParams_t), sizeof(params) );
            if(copy_from_user(&params, lvm->cmdParams, sizeof(LVMQDSP_HeadroomParams_t)))
            {
                pr_err("LVSE: LVMQDSP_SET_HEADROOM_PARAMS - read error \n");
                break;
            }
            status = q6asm_set_pp_params(lvse_ac, MODULE_ID_NXP_LVM, PARAM_ID_NXP_LVM_HEADROOM_CTRL,
                (void*)&params, sizeof(LVMQDSP_HeadroomParams_t));
            pr_debug("LVSE: q6asm_set_pp_params returned %d", status);
            break;
        }
        case LVMQDSP_SET_TUNING_PARAMS:
        {
            LVMQDSP_Custom_TuningParams_st params;

            pr_debug("LVSE: LVMQDSP_SET_TUNING_PARAMS - size=%d \n",sizeof(LVMQDSP_Custom_TuningParams_st));
            if(copy_from_user(&params, lvm->cmdParams, sizeof(LVMQDSP_Custom_TuningParams_st)))
            {
                pr_err("LVSE: LVMQDSP_SET_TUNING_PARAMS - read error \n");
                break;
            }
            status = q6asm_set_pp_params(lvse_ac, MODULE_ID_NXP_LVM, PARAM_ID_NXP_LVM_TUNING_CTRL,
                (void*)&params, sizeof(LVMQDSP_Custom_TuningParams_st));
            pr_debug("LVSE: q6asm_set_pp_params returned %d", status);
            break;
        }
		case LVMQDSP_CLEAR_AUDIO_BUFFER:
		{
            pr_debug("LVSE: LVMQDSP_CLEAR_AUDIO_BUFFER\n");
            status = q6asm_set_pp_params(lvse_ac, MODULE_ID_NXP_LVM, PARAM_ID_NXP_LVM_CLEAR_AUDIO_BUFFER_CTRL,
                NULL, 0);
            pr_debug("LVSE: q6asm_set_pp_params returned %d", status);
			break;
		}
		case LVMQDSP_SET_REVERB_INSTANCE_PARAMS:
		{
			LVMQDSP_RevInstParams_t instParams;

			pr_debug("LVSE: LVMQDSP_SET_REVERB_INSTANCE_PARAMS - size=%d \n",sizeof(LVMQDSP_RevInstParams_t));
			if(copy_from_user(&instParams, lvm->cmdParams, sizeof(LVMQDSP_RevInstParams_t)))
			{
				pr_err("LVSE: LVMQDSP_SET_REVERB_INSTANCE_PARAMS - read error \n");
				break;
			}
			status = q6asm_set_pp_params(lvse_ac, MODULE_ID_NXP_LVM_REVERB, PARAM_ID_NXP_LVM_REVERB_INSTANCE_CTRL,
				(void*)&instParams, sizeof(LVMQDSP_RevInstParams_t));
			pr_debug("LVSE: q6asm_set_pp_params returned %d", status);
			break;
		}
		case LVMQDSP_SET_REVERB_CONTROL_PARAMS:
		{
			LVMQDSP_RevControlParams_t params;

			pr_debug("LVSE: LVMQDSP_SET_REVERB_CONTROL_PARAMS - size=%d, %d \n",sizeof(LVMQDSP_RevControlParams_t), sizeof(params) );
			if(copy_from_user(& params, lvm->cmdParams, sizeof(LVMQDSP_RevControlParams_t)))
			{
				pr_err("LVSE: LVMQDSP_SET_REVERB_CONTROL_PARAMS - read error \n");
				break;
			}
			status = q6asm_set_pp_params(lvse_ac, MODULE_ID_NXP_LVM_REVERB, PARAM_ID_NXP_LVM_REVERB_EFFECT_CTRL,
				(void*)&params, sizeof(LVMQDSP_RevControlParams_t));
			pr_debug("LVSE: q6asm_set_pp_params returned %d", status);
#ifdef ENABLE_PARAM_DUMP
			lvse_dump_control_params(&params);
#endif /* ENABLE_PARAM_DUMP */
			break;
		}
		case LVMQDSP_REVERB_CLEAR_AUDIO_BUFFER:
		{
            pr_debug("LVSE: LVMQDSP_REVERB_CLEAR_AUDIO_BUFFER\n");
            status = q6asm_set_pp_params(lvse_ac, MODULE_ID_NXP_LVM_REVERB, PARAM_ID_NXP_LVM_REVERB_CLEAR_AUDIO_BUFFER_CTRL,
                NULL, 0);
            pr_debug("LVSE: q6asm_set_pp_params returned %d", status);
			break;
		}

        default:
            pr_debug("LVSE: Unknown lvse_routing_set_control command %d \n", lvm->cmd);
    }

    pr_debug("LVSE: unlock\n");
    mutex_unlock(&gSessions.lock);
    return status;
}
//----------------------------------------------------------------------------
// lvse_routing_get_control()
//----------------------------------------------------------------------------
// Purpose: Get parameter from the QDSP. After the q6asm_get_pp_params() is called,
//          the data is passed to the lvse_callback() with opcode. And the data is
//          copied to the user space here.
//
// Inputs:
//  kcontrol
//  ucontrol      	User data to get
//
// Outputs:
//  ucontrol        updated params
//----------------------------------------------------------------------------
int lvse_routing_get_control(struct snd_kcontrol       *kcontrol,
                             struct snd_ctl_elem_value *ucontrol) {

    int status = LVMQDSP_SUCCESS;
    int rc;
    int qdsp_param_type;
    void* output_params;
    int param_size;
    LVMQDSP_Command_t *lvm;
    LVMQDSP_Session_t session;
    struct audio_client *lvse_ac;
    
    lvm = (LVMQDSP_Command_t*)ucontrol->value.bytes.data;

    pr_debug("LVSE: cmd = %d ###############################################\n", lvm->cmd);
    session = lvm->session;
    if (gSessions.ac == NULL)
    {
        pr_err("\nLVSE: audio client is NULL %d", session);
        return LVMQDSP_ERROR;
    }
    
    lvse_ac = gSessions.ac; 
    
    switch (lvm->cmd) {
    case LVMQDSP_GET_INSTANCE_PARAMS:   pr_debug("LVSE: LVMQDSP_GET_INSTANCE_PARAMS\n");
                                        qdsp_param_type = PARAM_ID_NXP_LVM_INSTANCE_CTRL;
                                        param_size = sizeof(LVMQDSP_InstParams_t);
                                        output_params = (void*)&gSessions.inst_params;
                                        memset(&gSessions.inst_params, 1, sizeof(LVMQDSP_InstParams_t));
                                        break;
    case LVMQDSP_GET_CONTROL_PARAMS:    pr_debug("LVSE: LVMQDSP_GET_CONTROL_PARAMS\n");
                                        qdsp_param_type = PARAM_ID_NXP_LVM_EFFECT_CTRL;
                                        param_size = sizeof(LVMQDSP_ControlParams_t);
                                        output_params = (void*)&gSessions.control_params;
                                        memset(&gSessions.control_params, 1, sizeof(LVMQDSP_ControlParams_t));
                                        break;
    case LVMQDSP_GET_HEADROOM_PARAMS:   pr_debug("LVSE: LVMQDSP_GET_HEADROOM_PARAMS\n");
                                        qdsp_param_type = PARAM_ID_NXP_LVM_HEADROOM_CTRL;
                                        param_size = sizeof(LVMQDSP_HeadroomParams_t);
                                        output_params = (void*)&gSessions.headroom_params;
                                        memset(&gSessions.headroom_params, 1, sizeof(LVMQDSP_HeadroomParams_t));
                                        break;
    case LVMQDSP_GET_TUNING_PARAMS:     pr_debug("LVSE: LVMQDSP_GET_TUNING_PARAMS\n");
                                        qdsp_param_type = PARAM_ID_NXP_LVM_TUNING_CTRL;
                                        param_size = sizeof(LVMQDSP_Custom_TuningParams_st);
                                        output_params = (void*)&gSessions.tuning_params;
                                        memset(&gSessions.tuning_params, 1, sizeof(LVMQDSP_Custom_TuningParams_st));
                                        break;
    case LVMQDSP_GET_SPECTRUM_PARAMS:   pr_debug("LVSE: LVMQDSP_GET_SPECTRUM_PARAMS\n");
                                        qdsp_param_type = PARAM_ID_NXP_LVM_GET_SPECTRUM_CTRL;
                                        param_size = sizeof(LVMQDSP_PSA_SpectrumParams_st);
                                        output_params = (void*)&gSessions.spectrum_params;
                                        memset(&gSessions.spectrum_params, 1, sizeof(LVMQDSP_PSA_SpectrumParams_st));
                                        break;
	case LVMQDSP_GET_REVERB_INSTANCE_PARAMS:	pr_debug("LVSE: LVMQDSP_GET_INSTANCE_PARAMS\n");
										qdsp_param_type = LVMQDSP_GET_REVERB_INSTANCE_PARAMS;
										param_size = sizeof(LVMQDSP_RevInstParams_t);
										output_params = (void*)&gSessions.reverb_inst_params;
										memset(&gSessions.reverb_inst_params, 1, sizeof(LVMQDSP_RevInstParams_t));
										break;
	case LVMQDSP_GET_REVERB_CONTROL_PARAMS:	pr_debug("LVSE: LVMQDSP_GET_REVERB_CONTROL_PARAMS\n");
										qdsp_param_type = PARAM_ID_NXP_LVM_REVERB_EFFECT_CTRL;
										param_size = sizeof(LVMQDSP_RevControlParams_t);
										output_params = (void*)&gSessions.reverb_control_params;
										memset(&gSessions.reverb_control_params, 1, sizeof(LVMQDSP_RevControlParams_t));
										break;

    default:    pr_err("LVSE: lvse_routing_get_control, unknown param %d", lvm->cmd);
                return LVMQDSP_ERROR;
    }
    
    pr_debug("LVSE: mutex lock\n");
    mutex_lock(&gSessions.lock);

	if((lvm->cmd == LVMQDSP_GET_REVERB_INSTANCE_PARAMS) || (lvm->cmd == LVMQDSP_GET_REVERB_CONTROL_PARAMS))
	{
		rc = q6asm_get_pp_params(lvse_ac, MODULE_ID_NXP_LVM_REVERB, qdsp_param_type, param_size);
	}
	else
	{
	    rc = q6asm_get_pp_params(lvse_ac, MODULE_ID_NXP_LVM, qdsp_param_type, param_size);
	}
	
    pr_debug("LVSE: q6asm_get_pp_params returned %d", status);
    if(rc != 0){
		pr_err("LVSE: %s: q6asm_get_pp_params failed\n", __func__);
        goto exit;
	}
    pr_debug("LVSE: lvse_routing_get_control: params_count = %d", atomic_read(&gSessions.count));
    rc = wait_event_timeout(gSessions.wait, atomic_read(&gSessions.count), 5 * HZ);
    atomic_set(&gSessions.count, 0);
    if(!rc){
        pr_err("LVSE: %s: wait_event_timeout failed\n", __func__);
        goto exit;
    }

    if(copy_to_user(lvm->cmdParams, output_params, param_size))
    {
        pr_err("LVSE: LVMQDSP_GET_CONTROL_PARAMS write error \n");
        status = LVMQDSP_ERROR;
        goto exit;
    }

exit:
    pr_debug("LVSE: mutex unlock\n");
    mutex_unlock(&gSessions.lock);
    return status;
}

#ifdef ENABLE_PARAM_DUMP
void lvse_dump_control_params(void* ptr) {

    LVMQDSP_ControlParams_t *qdsp_params = (LVMQDSP_ControlParams_t*)ptr;
    LVM_ControlParams_t *params = &qdsp_params->LvmParams;
    LVM_EQNB_BandDef_t  *bands = &qdsp_params->LvmBands[0];

    pr_debug("\nLVSE: ControlParams.OperatingMode = %d",   params->OperatingMode);
    pr_debug("LVSE: ControlParams.SampleRate = %d",      params->SampleRate);
    pr_debug("LVSE: ControlParams.SourceFormat = %d",    params->SourceFormat);
    pr_debug("LVSE: ControlParams.SpeakerType = %d",     params->SpeakerType);

#ifdef ALGORITHM_CS
    /* Concert Sound Virtualizer parameters*/
    pr_debug("\nLVSE: ControlParams.VirtualizerOperatingMode = %d",    params->VirtualizerOperatingMode);
    pr_debug("LVSE: ControlParams.VirtualizerType = %d",             params->VirtualizerType);
    pr_debug("LVSE: ControlParams.VirtualizerReverbLevel = %d",      params->VirtualizerReverbLevel);
    pr_debug("LVSE: ControlParams.CS_EffectLevel = %d",              params->CS_EffectLevel);
#endif /* ALGORITHM_CS */

#ifdef ALGORITHM_CI
    /* Concert Sound Virtualizer parameters*/
    pr_debug("\nLVSE: ControlParams.VirtualizerOperatingMode = %d",    params->VirtualizerOperatingMode);
    pr_debug("LVSE: ControlParams.VirtualizerType = %d",             params->VirtualizerType);
    pr_debug("LVSE: ControlParams.VirtualizerReverbLevel = %d",      params->VirtualizerReverbLevel);
    pr_debug("LVSE: ControlParams.CS_EffectLevel = %d",              params->CS_EffectLevel);
#endif /* ALGORITHM_CI */

#ifdef ALGORITHM_EQNB
    ///* N-Band Equaliser parameters */
    pr_debug("\nLVSE: ControlParams.EQNB_OperatingMode = %d",    params->EQNB_OperatingMode);    
    pr_debug("LVSE: ControlParams.EQNB_LPF_Mode = %d",         params->EQNB_LPF_Mode);    
    pr_debug("LVSE: ControlParams.EQNB_LPF_CornerFreq = %d",   params->EQNB_LPF_CornerFreq);    
    pr_debug("LVSE: ControlParams.EQNB_HPF_Mode = %d",         params->EQNB_HPF_Mode);  
    pr_debug("LVSE: ControlParams.EQNB_HPF_CornerFreq = %d",    params->EQNB_HPF_CornerFreq);        
    
    pr_debug("\nLVSE: ControlParams.EQNB_NBands = %d",           params->EQNB_NBands);
    pr_debug("LVSE: ControlParams.pEQNB_BandDefinition[9].Gain = %d",      bands[9].Gain);    
    pr_debug("LVSE: ControlParams.pEQNB_BandDefinition[9].Frequency = %d", bands[9].Frequency);    
    pr_debug("LVSE: ControlParams.pEQNB_BandDefinition[9].QFactor = %d",   bands[9].QFactor);    
    
    //LVM_EQNB_FilterMode_en      EQNB_LPF_Mode;          /* Low pass filter */
    //LVM_INT16                   EQNB_LPF_CornerFreq;
    //LVM_EQNB_FilterMode_en      EQNB_HPF_Mode;          /* High pass filter */
    //LVM_INT16                   EQNB_HPF_CornerFreq;
    //LVM_UINT16                  EQNB_NBands;            /* Number of bands */
    //LVM_EQNB_BandDef_t          *pEQNB_BandDefinition;  /* Pointer to equaliser definitions */

#endif /* ALGORITHM_EQNB */

//#ifdef ALGORITHM_DBE
    ///* Bass Enhancement parameters */
    //LVM_BE_Mode_en              BE_OperatingMode;       /* Bass Enhancement operating mode */
    //LVM_INT16                   BE_EffectLevel;         /* Bass Enhancement effect level */
    //LVM_BE_CentreFreq_en        BE_CentreFreq;          /* Bass Enhancement centre frequency */
    //LVM_BE_FilterSelect_en      BE_HPF;                 /* Bass Enhancement high pass filter selector */

//#endif /* ALGORITHM_DBE */
//#ifdef ALGORITHM_PB
    ///* Bass Enhancement parameters */
    //LVM_BE_Mode_en              BE_OperatingMode;       /* Bass Enhancement operating mode */
    //LVM_INT16                   BE_EffectLevel;         /* Bass Enhancement effect level */
    //LVM_BE_CentreFreq_en        BE_CentreFreq;          /* Bass Enhancement centre frequency */
    //LVM_BE_FilterSelect_en      BE_HPF;                 /* Bass Enhancement high pass filter selector */

//#endif /* ALGORITHM_PB */
    ///* Volume Control parameters */
    //LVM_INT16                   VC_EffectLevel;         /* Volume Control setting in dBs */
    //LVM_INT16                   VC_Balance;             /* Left Right Balance control in dB (-96 to 96 dB), -ve values reduce */

//#ifdef ALGORITHM_TE
    ///* Treble Enhancement parameters */
    //LVM_TE_Mode_en              TE_OperatingMode;       /* Treble Enhancement On/Off */
    //LVM_INT16                   TE_EffectLevel;         /* Treble Enhancement gain dBs */

//#endif /* ALGORITHM_TE */
//#ifdef ALGORITHM_LM
    ///* Loudness Maximiser parameters */
    //LVM_LM_Mode_en              LM_OperatingMode;       /* Loudness Maximiser operating mode */
    //LVM_LM_Effect_en            LM_EffectLevel;         /* Loudness Maximiser effect level */
    //LVM_UINT16                  LM_Attenuation;         /* Loudness Maximiser output attenuation */
    //LVM_UINT16                  LM_CompressorGain;      /* Loudness Maximiser output compressor gain */
    //LVM_UINT16                  LM_SpeakerCutOff;       /* Loudness Maximiser speaker cut off frequency */

//#endif /* ALGORITHM_LM */
//#ifdef ALGORITHM_GM
    ///* Gentle Mix parameters */
    //LVM_GM_Mode_en              GM_OperatingMode;       /* Gentle mix operating mode */
    //LVM_GM_Effect_en            GM_EffectLevel;         /* Gentle mix effect level */

//#endif /* ALGORITHM_GM */
//#ifdef ALGORITHM_RS
    ///* Richsound parameters */
    //LVM_RS_Mode_en              RS_OperatingMode;       /* Richsound operating mode */
    //LVM_RS_Config_en            RS_Config;              /* Richsound configuration */
    //LVM_RS_AVLEffect_en         RS_AVLEffect;           /* Richsound auto volume effect */
    //LVM_INT16                   RS_EQBandLow;           /* Richsound low frequency band setting */
    //LVM_INT16                   RS_EQBandMid;           /* Richsound middle frequency band setting */
    //LVM_INT16                   RS_EQBandHigh;          /* Richsound high frequency band setting */

//#endif /* ALGORITHM_RS */
//#ifdef ALGORITHM_AVL
    ///* AVL parameters */
    //LVM_AVL_Mode_en             AVL_OperatingMode;      /* AVL operating mode */

//#endif /* ALGORITHM_AVL */
//#ifdef ALGORITHM_TG
    ///* Tone Generator parameters */
    //LVM_TG_Mode_en              TG_OperatingMode;       /* Tone generator mode */
    //LVM_TG_SweepMode_en         TG_SweepMode;           /* Log or linear sweep */
    //LVM_UINT16                  TG_StartFrequency;      /* Sweep start frequency in Hz */
    //LVM_INT16                   TG_StartAmplitude;      /* Sweep start amplitude in dBr */
    //LVM_UINT16                  TG_StopFrequency;       /* Sweep stop frequency in Hz */
    //LVM_INT16                   TG_StopAmplitude;       /* Sweep stop amplitude in dBr */
    //LVM_UINT16                  TG_SweepDuration;       /* Sweep duration in seconds, 0 for infinite duration tone */
    //LVM_Callback                pTG_CallBack;           /* End of sweep callback */
    //LVM_INT16                   TG_CallBackID;          /* Callback ID*/
    //void                        *pTGAppMemSpace;        /* Application instance handle or memory area */
//#endif /* ALGORITHM_TG */

//#ifdef ALGORITHM_PSA
    ///* General Control */
    //LVM_PSA_Mode_en             PSA_Enable;

    ///* Spectrum Analyzer parameters */
    //LVM_PSA_DecaySpeed_en       PSA_PeakDecayRate;      /* Peak value decay rate*/
    //LVM_UINT16                  PSA_NumBands;           /* Number of Bands*/l

//#endif /* ALGORITHM_PSA */

    pr_debug("\n");
}


void lvse_dump_tuning_params(void* ptr) 
{

    LVMQDSP_Custom_TuningParams_st *qdsp_params = (LVMQDSP_Custom_TuningParams_st*)ptr;
    LVM_Custom_TuningParams_st *params = &qdsp_params->LvmParams;
    LVM_EQNB_BandDef_t         *bands = &qdsp_params->LvmBands[0];

    pr_debug("\nLVSE: TuningParams.MidGain = %d",   params->MidGain);
    pr_debug("LVSE: TuningParams.MidCornerFreq = %d",   params->MidCornerFreq);
    pr_debug("LVSE: TuningParams.SideHighPassCutoff = %d",   params->SideHighPassCutoff);
    pr_debug("LVSE: TuningParams.SideLowPassCutoff = %d",   params->SideLowPassCutoff);
    pr_debug("LVSE: TuningParams.SideGain = %d",   params->SideGain);
    
    // LVM_INT16                   Elevation;          /* Virtual speaker elevation in degrees */
    // LVM_INT16                   FrontAngle;         /* Front speaker angle */
    // LVM_INT16                   SurroundAngle;      /* Surround speaker angle */

    // LVM_EQNB_FilterMode_en      Tuning_HPF_Mode;    /* High pass filter */
    // LVM_INT16                   Tuning_HPF_CornerFreq;
    
    pr_debug("\nLVSE: TuningParams.NumTuningBands = %d",   params->NumTuningBands);
    pr_debug("LVSE: TuningParams.pTuningBandDefs[0].Gain = %d",      bands[0].Gain);    
    pr_debug("LVSE: TuningParams.pTuningBandDefs[0].Frequency = %d", bands[0].Frequency);    
    pr_debug("LVSE: TuningParams.pTuningBandDefs[0].QFactor = %d",   bands[0].QFactor);  
 
    pr_debug("\n"); 
}
#endif /* ENABLE_PARAM_DUMP */

