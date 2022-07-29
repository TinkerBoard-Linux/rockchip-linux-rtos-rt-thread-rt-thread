/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2019 Fuzhou Rockchip Electronics Co., Ltd
 */

#include "amr_enc_hal.h"
#if defined(AUDIO_ENCODER_AMR)

#include "interf_enc.h"
#include "oscl_mem.h"

#define    AMR_ENC_ERROR(format,...)    RK_AUDIO_LOG_E("[AmrEnc] Error: "format, ##__VA_ARGS__)
#define    AMR_ENC_INFO(format,...)     RK_AUDIO_LOG_D("[AmrEnc] Info: "format, ##__VA_ARGS__)

static amr_enc_t g_amr_enc;

int32_t AudioAmrEncodeOpen(uint32_t A2B_EncAmr)
{
    struct audio_server_data_share *pEncDat = (struct audio_server_data_share *)A2B_EncAmr;

    memset(&g_amr_enc, 0, sizeof(amr_enc_t));

    g_amr_enc.ab_share_dat = pEncDat;

    g_amr_enc._encoder = Encoder_Interface_init(0);
    if (g_amr_enc._encoder == NULL)
    {
        AMR_ENC_ERROR("Amr encoder state init failed!");
        AudioAmrEncodeClose();
        return -1;
    }

    pEncDat->dat[0] = (uint32_t)&g_amr_enc;
    pEncDat->dat[1] = 0;
    AMR_ENC_INFO("%s out\n", __func__);
    return 0;
}

int32_t AudioAmrEncode(void)
{
    g_amr_enc.ab_share_dat->dat[0] = Encoder_Interface_Encode(g_amr_enc._encoder, MR122, g_amr_enc._buffer_in, g_amr_enc._buffer_out, 0);
    g_amr_enc.ab_share_dat->dat[1] = 0;
    return 0;
}

int32_t AudioAmrEncodeClose(void)
{
    if (g_amr_enc._encoder)
    {
        Encoder_Interface_exit(g_amr_enc._encoder);
        g_amr_enc._encoder = NULL;
    }
    return 0;
}
#endif
