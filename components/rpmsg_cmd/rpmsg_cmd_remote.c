/*
 * Copyright (c) 2023 Rockchip Electronics Co., Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <stdint.h>

#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>

#if defined(RT_USING_RPMSG_CMD)

#include "rpmsg_cmd.h"

#ifndef PRIMARY_CPU

#if defined(CPU0)
#define REMOTE_ID   REMOTE_ID_0
#define MASTER_EPT  EPT_M1R0_SYSCMD
#elif defined(CPU2)
#define REMOTE_ID   REMOTE_ID_2
#define MASTER_EPT  EPT_M1R2_SYSCMD
#elif defined(CPU3)
#define REMOTE_ID   REMOTE_ID_3
#define MASTER_EPT  EPT_M1R3_SYSCMD
#else
#error "error: Undefined CPU id!"
#endif
#define REMOTE_EPT EPT_M2R_ADDR(MASTER_EPT)

static rpmsg_func_t rpmsg_cmd_func_table[] =
{
    { RPMSG_CMD_GET_CPU_USAGE, rpmsg_cmd_cpuusage_callback},
};

static rpmsg_handle_t shell_handle[1] =
{
    {
        .remote = REMOTE_ID,
        .endpoint = REMOTE_EPT,
        .ftable = rpmsg_cmd_func_table,
    },
};

static int rpmsg_cmd_remote_init(void)
{
    rpmsg_handle_t *handle = &shell_handle;
    uint32_t i, count = sizeof(shell_handle) / sizeof(rpmsg_handle_t);

    for (i = 0; i < count; i++)
    {
        handle->ftable_size = sizeof(rpmsg_cmd_func_table) / sizeof(rpmsg_func_t);
        handle++;
    }

    rpmsg_handle_list handle_list;
    handle_list.handles = &shell_handle;
    handle_list.count = count;
    rpmsg_cmd_thread_init(&handle_list, 2048, RT_THREAD_PRIORITY_MAX / 2);
}
INIT_APP_EXPORT(rpmsg_cmd_remote_init);

#endif

#endif  // RT_USING_RPMSG_LITE
