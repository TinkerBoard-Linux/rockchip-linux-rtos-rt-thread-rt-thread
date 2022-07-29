/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2019 Fuzhou Rockchip Electronics Co., Ltd
 */

#include "amr_dec_hal.h"
#if defined(AUDIO_DECODER_AMR)

#include "interf_dec.h"

/*
 * #define    AMR_DEC_ERROR(format,...)    RK_AUDIO_LOG_E("[AmrDec] Error: "format, ##__VA_ARGS__)
 * #define    AMR_DEC_INFO(format,...)     RK_AUDIO_LOG_D("[AmrDec] Info: "format, ##__VA_ARGS__)
 */
#define    AMR_DEC_ERROR(format,...)
#define    AMR_DEC_INFO(format,...)

static amr_dec_t g_amr_dec;

int32_t AudioDecAmrOpen(uint32_t A2B_ShareDatAddr)
{
    AMR_DEC_INFO("amr decode open\n");
    struct audio_server_data_share *pDecDat = (struct audio_server_data_share *)A2B_ShareDatAddr;

    g_amr_dec.ab_share_dat = pDecDat;
    g_amr_dec._decoder = Decoder_Interface_init();
    if (NULL == g_amr_dec._decoder)
    {
        AMR_DEC_ERROR("AudioDecAMROpen Faild!!!");
        return -1;
    }

    pDecDat->dat[0] = (uint32_t)&g_amr_dec;
    pDecDat->dat[1] = 0;

    return 0;
}

int32_t AudioDecAmrProcess(uint32_t decode_dat)
{
    short buffer[160] = {0};
    uint8_t *ptr = NULL;
    int i;

    Decoder_Interface_Decode(g_amr_dec._decoder, g_amr_dec._buffer_in, buffer, 0);

    ptr = (uint8_t *)g_amr_dec._buffer_out;
    for (i = 0; i < 160; i++)
    {
        *ptr++ = (buffer[i] >> 0) & 0xff;
        *ptr++ = (buffer[i] >> 8) & 0xff;
    }

    return 0;
}


int32_t AudioDecAmrClose(void)
{
    AMR_DEC_INFO("amr decode close\n");

    if (g_amr_dec._decoder)
    {
        Decoder_Interface_exit(g_amr_dec._decoder);
        g_amr_dec._decoder = NULL;
    }
    return 0;
}
#endif
