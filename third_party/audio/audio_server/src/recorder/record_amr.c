/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2019 Fuzhou Rockchip Electronics Co., Ltd
 */

#include "AudioConfig.h"
#ifdef AUDIO_ENCODER_AMR
#define SAMPLE_PER_FRAME 160

struct record_amr
{
    bool has_post;
    char read_buf[SAMPLE_PER_FRAME * 2];
    encode_input_t input;
    encode_output_t output;
    encode_post_t post;
    void *userdata;
};
typedef struct record_amr *record_amr_handle_t;

static amr_enc_t *g_Amr = NULL;
extern struct audio_server_data_share g_ABCoreDat;

int record_amr_init(struct record_encoder *self, record_encoder_cfg_t *cfg)
{
    record_amr_handle_t amr = (record_amr_handle_t) audio_calloc(1, sizeof(*amr));
    int ret = 0;
    if (!amr)
        return -1;
    amr->has_post = false;
    amr->input = cfg->input;
    amr->output = cfg->output;
    amr->post = cfg->post;
    amr->userdata = cfg->userdata;
    self->userdata = (void *) amr;

    RK_AUDIO_LOG_D("record_amr_init\n");
    g_ABCoreDat.type = TYPE_AUDIO_AMR_ENC;
    ret = AudioSendMsg(TYPE_AUDIO_AMR_ENC, MEDIA_MSGBOX_CMD_ENCODE_OPEN);
    if (ret < 0)
    {
        record_amr_destroy(self);
        return RK_AUDIO_FAILURE;
    }

    g_Amr = (amr_enc_t *)g_ABCoreDat.dat[0];
    if (((int32_t)g_Amr) == AMR_AB_CORE_SHARE_ADDR_INVALID)
    {
        RK_AUDIO_LOG_E("amr init Bcore Decoder error\n");
        record_amr_destroy(self);
        return RK_AUDIO_FAILURE;
    }

    RK_AUDIO_LOG_D("amr init SUCCESS out\n");
    return RK_AUDIO_SUCCESS;
}

record_encoder_error_t record_amr_process(struct record_encoder *self)
{
    record_amr_handle_t amr = (record_amr_handle_t) self->userdata;
    bool is_first_frame = true;
    int32_t num_enc_bytes;
    int ret = 0;
    RK_AUDIO_LOG_D("record_amr_process\n");

    while (1)
    {
        for (int i = 0; i < SAMPLE_PER_FRAME * 2;)
        {
            int read_bytes = amr->input(amr->userdata, (char *)g_Amr->_buffer_in + i, SAMPLE_PER_FRAME * 2 - i);
            //RK_AUDIO_LOG_D("record_amr_process:%d\n",read_bytes);
            if (read_bytes == 0)
            {
                RK_AUDIO_LOG_V("amr->input finish \n");
            }
            else if (read_bytes == -1)
            {
                RK_AUDIO_LOG_E("amr->input failed \n");
                return RECORD_ENCODER_INPUT_ERROR;
            }
            i += read_bytes;
        }

        // Now we have 20ms 8kHz buffer, call AMR-NB Encode code
        ret = AudioSendMsg(TYPE_AUDIO_AMR_ENC, MEDIA_MSGBOX_CMD_ENCODE);
        if (ret < 0)
        {
            return RECORD_ENCODER_ENCODE_ERROR;
        }

        num_enc_bytes = g_ABCoreDat.dat[0];
        if (num_enc_bytes < 0)
        {
            RK_AUDIO_LOG_E("AMR-NB encode return %ld bytes\n", num_enc_bytes);
            return RECORD_ENCODER_ENCODE_ERROR;
        }

        if (is_first_frame)
        {
            amr->output(amr->userdata, "#!AMR\n", 6);
            is_first_frame = false;
        }
        // RK_AUDIO_LOG_D("num_enc_bytes : %d\n", num_enc_bytes);

        int write_bytes = amr->output(amr->userdata, (char *)g_Amr->_buffer_out, num_enc_bytes);
        if (write_bytes == -1)
        {
            RK_AUDIO_LOG_E("amr->output failed \n");
            return RECORD_ENCODER_OUTPUT_ERROR;
        }
    }
    return RK_AUDIO_SUCCESS;
}

bool record_amr_get_post_state(struct record_encoder *self)
{
    record_amr_handle_t amr = (record_amr_handle_t) self->userdata;
    return amr->has_post;
}

void record_amr_destroy(struct record_encoder *self)
{
    record_amr_handle_t amr = (record_amr_handle_t) self->userdata;
    if (amr)
    {
        audio_free(amr);
    }
    AudioSendMsg(TYPE_AUDIO_AMR_ENC, MEDIA_MSGBOX_CMD_ENCODE_CLOSE);
    g_ABCoreDat.type = TYPE_DATA_MAX;
}
#endif
