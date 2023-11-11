/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2019 Fuzhou Rockchip Electronics Co., Ltd
 */

#include "AudioConfig.h"

/* fsync per 100K bytes */
#define FSYNC_BOUNDARY      102400

int file_writer_init_impl(struct record_writer *self,
                          record_writer_cfg_t *cfg)
{
    memset(cfg->type, 0x0, sizeof(cfg->type));
    if (check_native_audio_type(cfg->target, cfg->type))
        return RK_AUDIO_FAILURE;
    int fd = audio_fopen(cfg->target, "w+");
    if (!fd)
    {
        RK_AUDIO_LOG_E("[%s]open native file error, file: %s", cfg->tag, cfg->target);
        return RK_AUDIO_FAILURE;
    }

    cfg->frame_size = 4096;
    self->fd = (void *)fd;
    self->out_bytes = 0;
    self->userdata = NULL;
    RK_AUDIO_LOG_V("[%s]open native file ok, file: %s, audio type:%s",
                   cfg->tag, cfg->target, cfg->type);

    return RK_AUDIO_SUCCESS;
}

int file_writer_write_impl(struct record_writer *self, char *data,
                           size_t data_len)
{
    int fd = (int)self->fd;
    int len;

    if (self->userdata)
        audio_fseek(fd, 0, SEEK_SET);
    self->userdata = NULL;
    len = audio_fwrite(data, 1, data_len, fd);
    if (len != data_len)
        return RK_AUDIO_FAILURE;

    self->out_bytes += len;
    if (self->out_bytes % FSYNC_BOUNDARY == 0)
        audio_fsync(fd);

    return len;
}

void file_writer_destroy_impl(struct record_writer *self)
{
    if (!self)
        return;

    int fd = (int)self->fd;

    audio_fclose(fd);
}
