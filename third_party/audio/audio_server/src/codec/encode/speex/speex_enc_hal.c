/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2019 Fuzhou Rockchip Electronics Co., Ltd
 */

#if defined(RK_AUDIO_ENCODER_SPEEX)
#include <string.h>
#include <speex/speex.h>
#include "speex_enc_hal.h"
#include "misc.h"

#define DUER_OUTPUT_HEADER_MAGIC_1     'P'
#define DUER_OUTPUT_HEADER_MAGIC_2     'X'

#ifndef DUER_COMPRESS_QUALITY
#define DUER_COMPRESS_QUALITY           (5)
#endif

#define    SPEEX_ENC_ERROR(format,...)    RK_AUDIO_LOG_E("[SpeexEnc] Error: "format, ##__VA_ARGS__)
#define    SPEEX_ENC_INFO(format,...)    RK_AUDIO_LOG_D("[SpeexEnc] Info: "format, ##__VA_ARGS__)

typedef enum _duer_sample_rate_e
{
    DUER_SR_8000,
    DUER_SR_16000,
    DUER_SR_TOTAL,
} duer_sample_rate_t;

typedef struct _duer_speex_s
{
    duer_sample_rate_t  _rate;
    void               *_encoder;
    size_t              _frame_size;
    size_t              _frame_cur;
    short              *_buffer_in;
    char               *_buffer_out;
    SpeexBits           _bits;
} duer_speex_t;

static const char DUER_OUTPUT_HEADER_MAGIC_0[DUER_SR_TOTAL] =
{
    'S',    //< DUER_SR_8000
    'T',    //< DUER_SR_16000
};

static duer_speex_t *g_Speex = NULL;
uint32_t *g_speex_encode_bytes = NULL;

int32_t AudioSpeexEncodeOpen(uint32_t A2B_EncSpeex)
{
    int tmp = DUER_COMPRESS_QUALITY;
    int rate;

    SPEEX_ENC_INFO("%s in\n", __func__);
    struct audio_server_data_share *pEncDat = (struct audio_server_data_share *)A2B_EncSpeex;

    rate = pEncDat->dat[0];
    g_speex_encode_bytes = (uint32_t *)pEncDat->dat[1];

    /* return Speex Encode Init Statue to Acore */
    pEncDat->dat[1] = 1;

    g_Speex = (duer_speex_t *) speex_alloc(sizeof(duer_speex_t));
    if (g_Speex == NULL)
    {
        SPEEX_ENC_ERROR("Memory overflow!!! Alloc speex handler failed.");
        return -1;
    }

    memset(g_Speex, 0, sizeof(duer_speex_t));

    if (rate == 8000)
    {
        g_Speex->_rate = DUER_SR_8000;
    }
    else if (rate == 16000)
    {
        g_Speex->_rate = DUER_SR_16000;
    }
    else
    {
        SPEEX_ENC_ERROR("Not supported sample rate: %d", rate);
        AudioSpeexEncodeClose();
        return -1;
    }

    g_Speex->_encoder = speex_encoder_init(speex_mode_list[g_Speex->_rate]);
    if (g_Speex->_encoder == NULL)
    {
        SPEEX_ENC_ERROR("Speex encoder state init failed!");
        AudioSpeexEncodeClose();
        return -1;
    }

    speex_encoder_ctl(g_Speex->_encoder, SPEEX_SET_QUALITY, &tmp);
    speex_encoder_ctl(g_Speex->_encoder, SPEEX_GET_FRAME_SIZE, &g_Speex->_frame_size);

    g_Speex->_buffer_out = (char *)speex_alloc(g_Speex->_frame_size + sizeof(int));
    if (g_Speex->_buffer_out == NULL)
    {
        SPEEX_ENC_ERROR("Memory overflow!!! Alloc speex buffer out failed.");
        AudioSpeexEncodeClose();
        return -1;
    }

    g_Speex->_buffer_in = (short *)speex_alloc(sizeof(short) * g_Speex->_frame_size);
    if (g_Speex->_buffer_in == NULL)
    {
        SPEEX_ENC_ERROR("Memory overflow!!! Alloc speex buffer in failed.");
        AudioSpeexEncodeClose();
        return -1;
    }

    speex_bits_init(&g_Speex->_bits);

    pEncDat->dat[0] = (uint32_t)g_Speex;
    pEncDat->dat[1] = 0;
    SPEEX_ENC_INFO("%s out\n", __func__);

    return 0;
}

int32_t AudioSpeexEncode(void)
{
    speex_bits_reset(&g_Speex->_bits);

    /*Encode the frame*/
    speex_encode_int(g_Speex->_encoder, g_Speex->_buffer_in, &g_Speex->_bits);

    /*Copy the bits to an array of char that can be written*/
    *g_speex_encode_bytes = speex_bits_write(&g_Speex->_bits, g_Speex->_buffer_out + sizeof(int), g_Speex->_frame_size);
    /* SPEEX_ENC_INFO("Speex encoded written: %d", *g_speex_encode_bytes); */
    g_Speex->_buffer_out[0] = *g_speex_encode_bytes & 0xFF;
    g_Speex->_buffer_out[1] = DUER_OUTPUT_HEADER_MAGIC_0[g_Speex->_rate];
    g_Speex->_buffer_out[2] = DUER_OUTPUT_HEADER_MAGIC_1;
    g_Speex->_buffer_out[3] = DUER_OUTPUT_HEADER_MAGIC_2;

    return 0;
}

int32_t AudioSpeexEncodeClose(void)
{
    SPEEX_ENC_INFO("%s in\n", __func__);

    if (g_Speex)
    {
        if (g_Speex->_encoder)
        {
            speex_encoder_destroy(g_Speex->_encoder);
            g_Speex->_encoder = NULL;
        }

        if (g_Speex->_buffer_out)
        {
            speex_free(g_Speex->_buffer_out);
            g_Speex->_buffer_out = NULL;
        }

        if (g_Speex->_buffer_in)
        {
            speex_free(g_Speex->_buffer_in);
            g_Speex->_buffer_in = NULL;
        }

        speex_bits_destroy(&g_Speex->_bits);

        speex_free(g_Speex);
        g_Speex = NULL;
    }

    SPEEX_ENC_INFO("%s out\n", __func__);
    return 0;
}
#endif
