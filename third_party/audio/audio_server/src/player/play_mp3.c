/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2019 Fuzhou Rockchip Electronics Co., Ltd
 */

#include "play_mp3.h"

#ifdef AUDIO_DECODER_MP3
typedef struct play_mp3 *play_mp3_handle_t;
static short *pi_pcmbuf = NULL;
MP3PLAYERINFO *g_pMPI = NULL;
extern uint32_t g_BcoreRetval;
extern struct audio_server_data_share g_ABCoreDat;

const static char ver[4][20] =
{
    "MPEG version2.5",
    "reserved",
    "MPEG Version 2",
    "MPEG Version 1",
};

int play_mp3_check_impl(char *buf, int len)
{
    if (strncmp(buf, "ID3", 3) == 0)
        return RK_AUDIO_SUCCESS;

    if ((buf[0] == 0xFF) &&
        ((buf[1] & 0xE0) == 0xE0))
    {
        int version = (buf[1] & 0x18) >> 3;
        int layer = 4 - ((buf[1] & 0x06) >> 1);
        if (version < 4 && version >= 0 && layer > 0 && layer < 4)
        {
            RK_AUDIO_LOG_V("%s layer %d", ver[version], layer);
            if (version != 1 && layer == 3)
                return RK_AUDIO_SUCCESS;
        }
    }

    return RK_AUDIO_FAILURE;
}

int play_mp3_init_impl(struct play_decoder *self, play_decoder_cfg_t *cfg)
{
    int ret = 0;

    RK_AUDIO_LOG_D("play_mp3_init_impl in");
    play_mp3_handle_t mp3 = (play_mp3_handle_t) audio_calloc(1, sizeof(*mp3));
    if (!mp3)
        return RK_AUDIO_FAILURE;
    mp3->has_post = false;
    mp3->input = cfg->input;
    mp3->output = cfg->output;
    mp3->post = cfg->post;
    mp3->userdata = cfg->userdata;
    self->userdata = (void *) mp3;

    RK_AUDIO_LOG_D("play_decoder type [%s]", self->type);
    pi_pcmbuf = (short *)audio_malloc(sizeof(short) * MAX_NCHAN * MAX_NGRAN * MAX_NSAMP);
    if (!pi_pcmbuf)
    {
        RK_AUDIO_LOG_E("pi_pcmbuf malloc fail.");
        play_mp3_destroy_impl(self);
        return RK_AUDIO_FAILURE;
    }
    g_ABCoreDat.dat[0] = (uint32_t)pi_pcmbuf;
    g_ABCoreDat.type = TYPE_AUDIO_MP3_DEC;

    ret = AudioSendMsg(TYPE_AUDIO_MP3_DEC, MEDIA_MSGBOX_CMD_DECODE_OPEN);
    if (ret < 0)
    {
        play_mp3_destroy_impl(self);
        return RK_AUDIO_FAILURE;
    }

    g_pMPI = (MP3PLAYERINFO *)g_ABCoreDat.dat[1];
    if (((int32_t)g_pMPI) == MP3_AB_CORE_SHARE_ADDR_INVALID)
    {
        RK_AUDIO_LOG_E("mp3 init Decoder error");
        play_mp3_destroy_impl(self);
        return RK_AUDIO_FAILURE;
    }
    RK_AUDIO_LOG_V("mp3 init SUCCESS out");

    if (cfg->start_time)
        mp3->start_time = cfg->start_time;

    return RK_AUDIO_SUCCESS;
}

play_decoder_error_t play_mp3_process_impl(struct play_decoder *self)
{
    play_mp3_handle_t mp3 = (play_mp3_handle_t) self->userdata;
    size_t n;
    int read_bytes;
    size_t _id3_length = 0;
    size_t id3_length = 0;
    size_t bytes_left = 0;
    size_t want_read_bytes = 0;
    int sample_rate, channels, bits;
    int ret = 0;
    bool is_first_frame = true, can_decode = false;
    char *decode_ptr = NULL;
    RK_AUDIO_LOG_D("play_mp3_process");

    read_bytes = mp3->input(mp3->userdata, mp3->read_buf, MP3_ID3V2_HEADER_LENGHT);
    if (read_bytes != MP3_ID3V2_HEADER_LENGHT)
    {
        RK_AUDIO_LOG_E("read buffer failed");
        return PLAY_DECODER_INPUT_ERROR;
    }

    n = MP3_ID3V2_HEADER_LENGHT;
    if (!memcmp(mp3->read_buf, "ID3", 3))
    {
        RK_AUDIO_LOG_D("find id3");
        _id3_length = id3_length = (mp3->read_buf[6] & 0x7f) << 21 |
                                   (mp3->read_buf[7] & 0x7f) << 14 |
                                   (mp3->read_buf[8] & 0x7f) << 7 |
                                   (mp3->read_buf[9] & 0x7f) ;
        RK_AUDIO_LOG_D("id3_length:%d", id3_length);
        if (id3_length > 0)
        {
            while (1)
            {
                n = 0;
                read_bytes = mp3->input(mp3->userdata, mp3->read_buf, sizeof(mp3->read_buf));
                RK_AUDIO_LOG_D("id3_length:%d,read_bytes:%d", id3_length, read_bytes);
                if (read_bytes <= 0)
                {
                    RK_AUDIO_LOG_E("read mp3 input failed");
                    return PLAY_DECODER_INPUT_ERROR;
                }
                if (read_bytes >= id3_length)
                {
                    bytes_left = read_bytes - id3_length;
                    decode_ptr = (char *)&mp3->read_buf[id3_length];
                    // memmove(mp3->read_buf,&mp3->read_buf[id3_length],bytes_left);
                    RK_AUDIO_LOG_D("%s,skip id3_length,:%d", __func__, id3_length);
                    break;
                }
                else
                {
                    id3_length -= read_bytes;
                }
            }
        }
    }

    want_read_bytes = sizeof(mp3->read_buf) - n - bytes_left;
MP3_SEEK_BACK:
    while (1)
    {
        if (bytes_left > 0)
        {
            memmove(mp3->read_buf, decode_ptr, bytes_left);
        }
        if (want_read_bytes)
            want_read_bytes = sizeof(mp3->read_buf) - n - bytes_left;
        read_bytes = mp3->input(mp3->userdata, mp3->read_buf + n + bytes_left, want_read_bytes);
        /*
         * RK_AUDIO_LOG_D("mp3 header = %02x, %02x, %02x, %02x, %02x, %02x",
         *         mp3->read_buf[0],
         *         mp3->read_buf[1],
         *         mp3->read_buf[2],
         *         mp3->read_buf[3],
         *         mp3->read_buf[4],
         *         mp3->read_buf[5]);
         */
        if (read_bytes < want_read_bytes)
        {
            want_read_bytes = 0;
        }

        /* RK_AUDIO_LOG_D("mp3 process read:%x\n",read_bytes); */
        if ((read_bytes == 0) && (bytes_left == 0))
        {
            RK_AUDIO_LOG_V("mp3 read over");
            return RK_AUDIO_SUCCESS;
        }
        else if (read_bytes == -1)
        {
            RK_AUDIO_LOG_E("mp3 read fail");
            return PLAY_DECODER_INPUT_ERROR;
            //return RK_AUDIO_SUCCESS;
        }
        else
        {
            decode_ptr = mp3->read_buf;
            bytes_left = bytes_left + read_bytes + n;
            can_decode = true;
            n = 0;
        }

        g_ABCoreDat.dat[0] = (uint32_t)decode_ptr;
        g_ABCoreDat.dat[1] = bytes_left;
        if (can_decode)
        {
            ret = AudioSendMsg(TYPE_AUDIO_MP3_DEC, MEDIA_MSGBOX_CMD_DECODE);
            decode_ptr = (char *)g_ABCoreDat.dat[0];
            bytes_left = g_ABCoreDat.dat[1];

            if (ret < 0 && bytes_left == -1)
            {
                RK_AUDIO_LOG_E("Decoder fail:%d", ret);
                return RK_AUDIO_FAILURE;
            }

            /* fix bug system crash start */
            if ((decode_ptr < mp3->read_buf)
                || (decode_ptr > (mp3->read_buf + MAINBUF_SIZE * MP3_DECODE_FRAME_COUNT))
                || (bytes_left > MAINBUF_SIZE * MP3_DECODE_FRAME_COUNT))
            {
                RK_AUDIO_LOG_E("system_crash_start %p, %x", decode_ptr, bytes_left);
                decode_ptr = mp3->read_buf;
                bytes_left = 0;
            }
            /* fix bug system crash end */

            ret = g_BcoreRetval;
            if (is_first_frame)
            {
                if (ret != 0)
                    continue;

                if (g_pMPI->mpi_frameinfo.layer != 3)
                {
                    RK_AUDIO_LOG_V("No support version %d layer %d\n", g_pMPI->mpi_frameinfo.version, g_pMPI->mpi_frameinfo.layer);
                    return PLAY_DECODER_DECODE_ERROR;
                }

                bits        = g_pMPI->mpi_frameinfo.bitsPerSample;
                channels    = g_pMPI->mpi_frameinfo.nChans;
                sample_rate = g_pMPI->mpi_frameinfo.samprate;
                RK_AUDIO_LOG_V("is first frame mp3 samprate = %d Channel = %d bits = %d outlen = %d, bps = %d\n",
                               g_pMPI->mpi_frameinfo.samprate,
                               g_pMPI->mpi_frameinfo.nChans,
                               g_pMPI->mpi_frameinfo.bitsPerSample,
                               g_pMPI->mpi_frameinfo.outputSamps,
                               g_pMPI->mpi_frameinfo.bitrate);

                if ((channels != 1) && (channels != 2))
                {
                    RK_AUDIO_LOG_W("mp3 decode channels %d don't support, continue read file...", channels);
                    continue;
                }

                if (sample_rate < 8000 || sample_rate > 48000)
                {
                    RK_AUDIO_LOG_W("mp3 decode sample rate %d don't support, continue read file...", sample_rate);
                    continue;
                }
                if (bits != 16)
                {
                    RK_AUDIO_LOG_W("mp3 decode sample bit %d don't support, continue read file...", bits);
                    continue;
                }
                uint32_t time;
                uint32_t pos;
                if (g_pMPI->mpi_frameinfo.vbr == 1)
                {
                    time = g_pMPI->mpi_frameinfo.fCount * g_pMPI->mpi_frameinfo.outputSamps / channels / (sample_rate / 1000);
                    if (g_pMPI->mpi_frameinfo.TOC[0] != -1)
                        pos = g_pMPI->mpi_frameinfo.TOC[(int)((mp3->start_time * 1000. / time) * 100)] * g_pMPI->mpi_frameinfo.fSize / 256;
                    else
                        pos = mp3->start_time * 1000. / time * player_get_file_length(mp3->userdata);
                }
                else
                {
                    time = (player_get_file_length(mp3->userdata) - _id3_length - MP3_ID3V2_HEADER_LENGHT) / (g_pMPI->mpi_frameinfo.bitrate / 8 / 1000);
                    pos = mp3->start_time * g_pMPI->mpi_frameinfo.bitrate / 8 + MP3_ID3V2_HEADER_LENGHT + _id3_length;
                }
                player_set_total_time(mp3->userdata, time);
                mp3->post(mp3->userdata, sample_rate, bits, channels);
                mp3->has_post = true;
                is_first_frame = false;
                if (mp3->start_time)
                {
                    player_preprocess_seek(mp3->userdata, pos);
                    mp3->start_time = 0;
                    g_pMPI->mpi_frameinfo.outputSamps = 0;
                    bytes_left = 0;
                    goto MP3_SEEK_BACK;
                }
            }

            if (ret != 0)
                RK_AUDIO_LOG_D("ret = %d,outlen = %d", ret, g_pMPI->mpi_frameinfo.outputSamps);
            if ((g_pMPI->mpi_frameinfo.outputSamps > 0) && (ret == 0))
            {
                if (g_pMPI->mpi_frameinfo.samprate != sample_rate)
                {
                    RK_AUDIO_LOG_W("WARNING: Changing Frame Header is Happenned.");
                }
                size_t output_bytes = g_pMPI->mpi_frameinfo.outputSamps * 2;
                int write_bytes = mp3->output(mp3->userdata, (char *)pi_pcmbuf, output_bytes);
                if (write_bytes == -1)
                {
                    RK_AUDIO_LOG_E("mp3_decode pcm write failed");
                    break;
                }
                g_pMPI->mpi_frameinfo.outputSamps = 0;
                // RK_AUDIO_LOG_D("mp3 output write_bytes:%d",write_bytes);
            }
        }
    }
    return RK_AUDIO_SUCCESS;
}

bool play_mp3_get_post_state_impl(struct play_decoder *self)
{
    play_mp3_handle_t mp3 = (play_mp3_handle_t) self->userdata;
    return mp3->has_post;
}

void play_mp3_destroy_impl(struct play_decoder *self)
{
    RK_AUDIO_LOG_D("in");
    play_mp3_handle_t mp3 = (play_mp3_handle_t) self->userdata;
    if (mp3)
    {
        RK_AUDIO_LOG_D("free mp3dec buffer.");
        audio_free(mp3);
    }
    if (pi_pcmbuf)
    {
        audio_free(pi_pcmbuf);
    }

    AudioSendMsg(TYPE_AUDIO_MP3_DEC, MEDIA_MSGBOX_CMD_DECODE_CLOSE);
    g_ABCoreDat.type = TYPE_DATA_MAX;

    RK_AUDIO_LOG_D("out");
}
#endif
