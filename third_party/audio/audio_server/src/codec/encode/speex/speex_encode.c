/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2019 Fuzhou Rockchip Electronics Co., Ltd
 */

#ifdef  CONFIG_AUDIO_ENCODER_SPEEX
#include "AudioConfig.h"

static duer_speex_t *g_pSpeex = NULL;
//static HDC g_EncoderBcoreDev;
static uint32 g_speex_encode_bytes = -1;

#ifdef DEBUG_WRITE_DECODED_DATA
static HDC g_debug_audio_file = NULL;
#endif

int speex_encode_init(int rate)
{
    RK_AUDIO_LOG_D("in\n");
    int ret = -1;

    g_ABCoreDat.dat[0] = rate;
    g_ABCoreDat.dat[1] = (uint32)&g_speex_encode_bytes;
    g_ABCoreDat.type = TYPE_AUDIO_SPEEX_ENC;

    ret = AudioSendMsg(TYPE_AUDIO_SPEEX_ENC, MEDIA_MSGBOX_CMD_ENCODE_OPEN);
    if (ret < 0)
    {
        speex_encode_destroy();
        return RK_AUDIO_FAILURE;
    }

    g_pSpeex = (duer_speex_t *)g_ABCoreDat.dat[0];

    /* Get Bcore Speex Encode Init Status */
    ret = g_ABCoreDat.dat[1];

    if (ret == 0)
    {
        RK_AUDIO_LOG_D("SUCCESS out\n");

#ifdef DEBUG_WRITE_DECODED_DATA
        g_debug_audio_file = audio_fopen("C:\\test-debug.spx", "wb");
#endif
        return (uint32)g_pSpeex;
    }
    else
    {
        RK_AUDIO_LOG_E("FAILURE out\n");
        speex_encode_destroy();
        return RK_AUDIO_FAILURE;
    }
}

int speex_encode_process(const char *data, size_t size, speex_encoded_func func)
{
    size_t cur = 0;
    int ret = 0;
    /* RK_AUDIO_LOG_D("speex_encode, speex:%p", g_pSpeex); */
    if (g_pSpeex && g_pSpeex->_encoder && g_pSpeex->_buffer_in && g_pSpeex->_buffer_out)
    {
        size_t framesize = g_pSpeex->_frame_size * sizeof(short);
        char *buffer_in = (char *)g_pSpeex->_buffer_in;

        /* RK_AUDIO_LOG_D("speex_encode: size = %d, framesize = %d", size, framesize); */

        while (cur + framesize - g_pSpeex->_frame_cur <= size)
        {
            /* RK_AUDIO_LOG_D("speex_encode: cur = %d, speex->_frame_cur = %d", cur, g_pSpeex->_frame_cur); */
            memcpy(buffer_in + g_pSpeex->_frame_cur, (const char *)data + cur, framesize - g_pSpeex->_frame_cur);
            cur += framesize - g_pSpeex->_frame_cur;
            g_pSpeex->_frame_cur = 0;

            ret = AudioSendMsg(TYPE_AUDIO_SPEEX_ENC, MEDIA_MSGBOX_CMD_ENCODE);
            if (ret < 0)
            {
                return RECORD_ENCODER_ENCODE_ERROR;
            }

            if (func)
            {
                func(g_pSpeex->_buffer_out, sizeof(int) + g_speex_encode_bytes);
#ifdef DEBUG_WRITE_DECODED_DATA
                audio_fwrite(g_pSpeex->_buffer_out, sizeof(int) + g_speex_encode_bytes, 1, g_debug_audio_file);
#endif
            }
        }

        if (g_pSpeex->_frame_cur > 0 && (data == NULL || size == 0))
        {
            memset(buffer_in + g_pSpeex->_frame_cur, 0, framesize - g_pSpeex->_frame_cur);
            ret = AudioSendMsg(TYPE_AUDIO_SPEEX_ENC, MEDIA_MSGBOX_CMD_ENCODE);
            if (ret < 0)
            {
                return RECORD_ENCODER_ENCODE_ERROR;
            }
            if (func)
            {
                func(g_pSpeex->_buffer_out, sizeof(int) + g_speex_encode_bytes);
#ifdef DEBUG_WRITE_DECODED_DATA
                audio_fwrite(g_pSpeex->_buffer_out, sizeof(int) + g_speex_encode_bytes, 1, g_debug_audio_file);
#endif
            }

        }
        else if (cur < size)
        {
            memcpy(buffer_in + g_pSpeex->_frame_cur, (const char *)data + cur, size - cur);
            g_pSpeex->_frame_cur += size - cur;
        }
    }

    return RK_AUDIO_SUCCESS;
}

void speex_encode_destroy(void)
{
    RK_AUDIO_LOG_D("in\n");

#ifdef DEBUG_WRITE_DECODED_DATA
    audio_fclose(g_debug_audio_file);
#endif
    AudioSendMsg(TYPE_AUDIO_SPEEX_ENC, MEDIA_MSGBOX_CMD_ENCODE_CLOSE);
    g_ABCoreDat.type = TYPE_DATA_MAX;

    RK_AUDIO_LOG_D("out\n");
}
#endif     /* -----  CONFIG_AUDIO_ENCODER_SPEEX  ----- */
