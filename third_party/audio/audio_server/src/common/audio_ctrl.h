/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2019 Fuzhou Rockchip Electronics Co., Ltd
 */

#ifndef _AUDIO_PORT_CTRL_H
#define _AUDIO_PORT_CTRL_H
#ifdef __cplusplus
extern "C" {
#endif

#include "audio_server.h"

int AudioSendMsg(audio_data_type id, MSGBOX_SYSTEM_CMD mesg);
HDC AudioLoadDsp(void);
void AudioUnloadDsp(void);

#ifdef __cplusplus
}
#endif
#endif




