/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2019 Fuzhou Rockchip Electronics Co., Ltd
 */

#ifndef _AUDIO_AMR_DEC_HAL_H_
#define _AUDIO_AMR_DEC_HAL_H_
#include "AudioConfig.h"

#include <stddef.h>
#include <stdio.h>

#define WMF_MR122_FRAME_SIZE 31
#define SAMPLE_PER_FRAME 160

int32_t AudioDecAmrClose(void);
int32_t AudioDecAmrOpen(uint32_t A2B_ShareDatAddr);
int32_t AudioDecAmrProcess(uint32_t decode_dat);

#endif        // _AUDIO_AMR_DEC_HAL_H_
