/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2019 Fuzhou Rockchip Electronics Co., Ltd
 */

#include "BspConfig.h"
#include "rkos_typedef.h"
#include "kernel/RKOS.h"
#include "subsys/global.h"

#ifdef __DRIVER_BCORE_BCOREDEVICE_C__

//-------------------------------------------MP3 BIN----------------------------------------------------------

/*
*-----------------------------------------------------------------------------------
*
* dsp firmware bin
*
*-----------------------------------------------------------------------------------
*/
const uint8 dsp_code_bin[] =
{
#include "dsp_code.bin"
};


uint8 dsp_data_bin[] =
{
#include "dsp_data.bin"
};

/*
*-----------------------------------------------------------------------------------
*
*                       End of audio_bcore_bin.c
*
*-----------------------------------------------------------------------------------
*/

#endif
