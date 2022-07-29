/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2019 Fuzhou Rockchip Electronics Co., Ltd
 */

#include "AudioConfig.h"

struct play_amr
{
    bool has_post;
    char read_buf[1024];
    decode_input_t input;
    decode_output_t output;
    decode_post_t post;
    void *userdata;
};

typedef struct play_amr *play_amr_handle_t;
static amr_dec_t *g_Amr;
extern struct audio_server_data_share g_ABCoreDat;

int play_amr_check_impl(char *buf, int len)
{
    return (strncmp(buf, "#!AMR", 5));
}

int play_amr_init_impl(struct play_decoder *self, play_decoder_cfg_t *cfg)
{
    int ret = RK_AUDIO_SUCCESS;
    RK_AUDIO_LOG_D("in\n");
    play_amr_handle_t amr = (play_amr_handle_t) audio_calloc(1, sizeof(*amr));
    if (!amr)
        return RK_AUDIO_FAILURE;
    amr->has_post = false;
    amr->input = cfg->input;
    amr->output = cfg->output;
    amr->post = cfg->post;
    amr->userdata = cfg->userdata;
    self->userdata = (void *) amr;

    RK_AUDIO_LOG_D("play_decoder type [%s]\n", self->type);

    g_ABCoreDat.type = TYPE_AUDIO_AMR_DEC;
    ret = AudioSendMsg(TYPE_AUDIO_AMR_DEC, MEDIA_MSGBOX_CMD_DECODE_OPEN);
    if (ret < 0)
    {
        play_amr_destroy_impl(self);
        return RK_AUDIO_FAILURE;
    }

    g_Amr = (amr_dec_t *)g_ABCoreDat.dat[0];
    if (((int32_t)g_Amr) == AMR_AB_CORE_SHARE_ADDR_INVALID)
    {
        RK_AUDIO_LOG_E("amr init Decoder error\n");
        play_amr_destroy_impl(self);
        return RK_AUDIO_FAILURE;
    }
    RK_AUDIO_LOG_D("amr init SUCCESS out\n");

    return RK_AUDIO_SUCCESS;
}

play_decoder_error_t play_amr_process_impl(struct play_decoder *self)
{
    play_amr_handle_t amr = (play_amr_handle_t) self->userdata;
    size_t read_bytes;
    int ret = 0;

    read_bytes = amr->input(amr->userdata, amr->read_buf, AMR_HEADER_AMRNB_SC_LENGTH);
    if (read_bytes != AMR_HEADER_AMRNB_SC_LENGTH
        || memcmp(amr->read_buf, AMR_HEADER_AMRNB_SC, AMR_HEADER_AMRNB_SC_LENGTH))
    {
        RK_AUDIO_LOG_E("read amr-nb header failed\n");
        return RK_AUDIO_FAILURE;
    }

    amr->post(amr->userdata, 8000, 16, 1);
    amr->has_post = true;

    while (1)
    {
        // Read the mode byte
        read_bytes = amr->input(amr->userdata, (char *)g_Amr->_buffer_in, 1);
        if (read_bytes <= 0)
        {
            RK_AUDIO_LOG_E("Read Size Error\n");
            return PLAY_DECODER_INPUT_ERROR;
        }

        if (((g_Amr->_buffer_in[0] >> 3) & 0x0f) != AMR_122)
        {
            RK_AUDIO_LOG_E("Read mode failed or not supported mode\n");
            return PLAY_DECODER_DECODE_ERROR;
        }
        // Harded code the frame size
        read_bytes = amr->input(amr->userdata, (char *)g_Amr->_buffer_in + 1, WMF_MR122_FRAME_SIZE);
        if (read_bytes != WMF_MR122_FRAME_SIZE)
        {
            RK_AUDIO_LOG_E("read amr frame failed\n");
            return PLAY_DECODER_DECODE_ERROR;
        }
        ret = AudioSendMsg(TYPE_AUDIO_AMR_DEC, MEDIA_MSGBOX_CMD_DECODE);
        if (ret < 0)
        {
            return PLAY_DECODER_DECODE_ERROR;
        }
        /* PCM16LE */
        int write_bytes = amr->output(amr->userdata, (char *)g_Amr->_buffer_out, SAMPLE_PER_FRAME * 2);
        if (write_bytes == -1)
        {
            RK_AUDIO_LOG_E("amr_decode pcm write failed\n");
            return PLAY_DECODER_OUTPUT_ERROR;
        }
    }

    return RK_AUDIO_SUCCESS;
}

bool play_amr_get_post_state_impl(struct play_decoder *self)
{
    play_amr_handle_t amr = (play_amr_handle_t) self->userdata;
    return amr->has_post;
}

void play_amr_destroy_impl(struct play_decoder *self)
{
    play_amr_handle_t amr = (play_amr_handle_t) self->userdata;
    if (amr)
    {
        audio_free(amr);
    }

    AudioSendMsg(TYPE_AUDIO_AMR_DEC, MEDIA_MSGBOX_CMD_DECODE_CLOSE);
    g_ABCoreDat.type = TYPE_DATA_MAX;

    RK_AUDIO_LOG_D("out\n");
}
