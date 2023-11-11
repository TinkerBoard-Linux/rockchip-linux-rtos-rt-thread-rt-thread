/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2019 Fuzhou Rockchip Electronics Co., Ltd
 */
#include "AudioConfig.h"

#if defined(AUDIO_DECODER_MP3)
#include "mp3_hal.h"
#endif

#if defined(AUDIO_DECODER_AMR)
#include "amr_dec_hal.h"
#endif

#if defined(AUDIO_ENCODER_AMR)
#include "amr_enc_hal.h"
#endif

struct audio_server_data_share g_ABCoreDat =
{
    0
};
struct audio_server_data_share g_asr_ABCoreDat;

uint32_t g_BcoreRetval = 0;

#if !defined(RK_AUDIO_CODEC_RUN_ON_M4) && defined(AUDIO_USING_DSP)
struct dsp_work *g_dsp_work;
audio_player_semaphore_handle g_dsp_semaphore;
uint32_t g_dsp_enable = 0;
struct rt_device *g_dsp_handle = NULL;
#endif

HDC AudioLoadDsp(void)
{
    struct rt_device *dsp_handle = NULL;
#if !defined(RK_AUDIO_CODEC_RUN_ON_M4) && defined(AUDIO_USING_DSP)
    if (g_dsp_semaphore == NULL)
    {
        g_dsp_semaphore = audio_semaphore_create();
        if (g_dsp_semaphore == NULL)
            return NULL;
    }

    audio_semaphore_take(g_dsp_semaphore);

    if (g_dsp_enable == 0)
        dsp_handle = audio_open_dsp(396000000);
    else
        dsp_handle = g_dsp_handle;
    g_dsp_enable++;
    audio_semaphore_give(g_dsp_semaphore);
#endif

    return dsp_handle;
}

void AudioUnloadDsp(void)
{
#if !defined(RK_AUDIO_CODEC_RUN_ON_M4) && defined(AUDIO_USING_DSP)
    audio_semaphore_take(g_dsp_semaphore);
    if (g_dsp_enable == 0)
    {
        RK_AUDIO_LOG_E("\r\n [%s] DSP close failure", __func__);
    }
    else
    {
        g_dsp_enable--;
        if (g_dsp_enable == 0)
        {
            if (g_dsp_handle)
            {
                audio_close_dsp(g_dsp_handle);
                g_dsp_handle = NULL;
            }
        }
    }
    audio_semaphore_give(g_dsp_semaphore);
#endif
}

int AudioSendMsg(audio_data_type id, MSGBOX_SYSTEM_CMD mesg)
{
#ifdef RK_AUDIO_CODEC_RUN_ON_M4
    int ret = RK_AUDIO_FAILURE;

    switch (RK_AUDIO_CASE_CHECK(id, mesg))
    {
#if defined(AUDIO_DECODER_MP3)
    case RK_AUDIO_CASE_CHECK(TYPE_AUDIO_MP3_DEC, MEDIA_MSGBOX_CMD_DECODE_OPEN):
        ret = AudioDecMP3Open((uint32_t)&g_ABCoreDat);
        break;

    case RK_AUDIO_CASE_CHECK(TYPE_AUDIO_MP3_DEC, MEDIA_MSGBOX_CMD_DECODE):
        ret = AudioDecMP3Process((uint32_t)&g_ABCoreDat);
        break;

    case RK_AUDIO_CASE_CHECK(TYPE_AUDIO_MP3_DEC, MEDIA_MSGBOX_CMD_DECODE_CLOSE):
        ret = AudioDecMP3Close();
        break;
#endif

#if defined(AUDIO_DECODER_AMR)
    case RK_AUDIO_CASE_CHECK(TYPE_AUDIO_AMR_DEC, MEDIA_MSGBOX_CMD_DECODE_OPEN):
        ret = AudioDecAmrOpen((uint32_t)&g_ABCoreDat);
        break;

    case RK_AUDIO_CASE_CHECK(TYPE_AUDIO_AMR_DEC, MEDIA_MSGBOX_CMD_DECODE):
        ret = AudioDecAmrProcess((uint32_t)&g_ABCoreDat);
        break;

    case RK_AUDIO_CASE_CHECK(TYPE_AUDIO_AMR_DEC, MEDIA_MSGBOX_CMD_DECODE_CLOSE):
        ret = AudioDecAmrClose();
        break;
#endif

#if defined(AUDIO_ENCODER_AMR)
    case RK_AUDIO_CASE_CHECK(TYPE_AUDIO_AMR_ENC, MEDIA_MSGBOX_CMD_ENCODE_OPEN):
        ret = AudioAmrEncodeOpen((uint32_t)&g_ABCoreDat);
        break;

    case RK_AUDIO_CASE_CHECK(TYPE_AUDIO_AMR_ENC, MEDIA_MSGBOX_CMD_ENCODE):
        ret = AudioAmrEncode();
        break;

    case RK_AUDIO_CASE_CHECK(TYPE_AUDIO_AMR_ENC, MEDIA_MSGBOX_CMD_ENCODE_CLOSE):
        ret = AudioAmrEncodeClose();
        break;
#endif

    default:
        RK_AUDIO_LOG_E("AudioSendMsg error.");
        break;
    }
    g_BcoreRetval = ret;
    return ret;
#elif defined(AUDIO_USING_DSP)
    int ret = RK_AUDIO_SUCCESS;

    switch (RK_AUDIO_CASE_CHECK(id, mesg))
    {
    default:
        RK_AUDIO_LOG_E("AudioSendMsg error.");
        ret = -1;
        break;
    }
    g_BcoreRetval = g_dsp_work->result;
    return ret;
#else
#error "Codec run on DSP but DSP is disable"
    return -1;
#endif
}
