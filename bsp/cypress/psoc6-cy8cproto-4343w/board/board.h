/*
 * Copyright (c) 2006-2022, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-06-29     Rbb666	   first version
 */
 
#ifndef __BOARD_H__
#define __BOARD_H__

#include <rtthread.h>
#include "drv_common.h"
#include "drv_gpio.h"

#include "cyhal.h"
#include "cybsp.h"
#include "cy_pdl.h"
#include "cy_retarget_io.h"

#define IFX_SRAM_SIZE       1014
#define IFX_SRAM_END        (0x08002000 + IFX_SRAM_SIZE * 1024)

#ifdef __ARMCC_VERSION
    extern int Image$$RW_IRAM1$$ZI$$Limit;
    #define HEAP_BEGIN    (&Image$$RW_IRAM1$$ZI$$Limit)
#elif __ICCARM__
    #pragma section="HEAP"
    #define HEAP_BEGIN    (__segment_end("HEAP"))
#else
    extern unsigned int __end__;
    extern unsigned int __HeapLimit;
    #define HEAP_BEGIN    (void*)&__end__
    #define HEAP_END      (void*)&__HeapLimit
#endif

#define HEAP_END		IFX_SRAM_END

void cy_bsp_all_init(void);

#endif

