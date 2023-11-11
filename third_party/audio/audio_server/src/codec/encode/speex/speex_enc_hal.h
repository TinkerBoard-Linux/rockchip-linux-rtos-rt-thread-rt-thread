/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2019 Fuzhou Rockchip Electronics Co., Ltd
 */


#ifndef _AUDIO_SPEEX_ENC_HAL_H_
#define _AUDIO_SPEEX_ENC_HAL_H_
#include "AudioConfig.h"

#include <stdio.h>

#define    SPEEX_AB_CORE_SHARE_ADDR_INVALID (-1)

int32_t AudioSpeexEncodeOpen(uint32_t A2B_EncSpeexAddr);
int32_t AudioSpeexEncodeClose(void);
int32_t AudioSpeexEncode(void);

#endif        // _AUDIO_SPEEX_ENC_HAL_H_
