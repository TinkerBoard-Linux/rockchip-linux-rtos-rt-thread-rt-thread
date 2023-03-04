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

#if defined(PRIMARY_CPU)

static rpmsg_func_t rpmsg_cmd_func_table[] =
{
    { RPMSG_ACK_GET_CPU_USAGE, rpmsg_cmd_cpuusage_callback},
};

static rpmsg_handle_t shell_handle[PLATFORM_CORE_COUNT - 1] =
{
    {
        .remote = REMOTE_ID_0,
        .endpoint = EPT_M1R0_SYSCMD,
        .ftable = rpmsg_cmd_func_table,
    },
    {
        .remote = REMOTE_ID_2,
        .endpoint = EPT_M1R2_SYSCMD,
        .ftable = rpmsg_cmd_func_table,
    },
    {
        .remote = REMOTE_ID_3,
        .endpoint = EPT_M1R3_SYSCMD,
        .ftable = rpmsg_cmd_func_table,
    },
};

int rpmsg_cmd_master_init(void)
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
INIT_APP_EXPORT(rpmsg_cmd_master_init);

static void rpmsg_cmd_usage(void)
{
    rt_kprintf("\nusage: rpmsg_cmd <cmd> [0|1|2|3] ...\n");
    rt_kprintf("\n");
    rt_kprintf("   <cmd>      <comments>       \n");
    rt_kprintf("  cpuusage -- get cpu usage n% \n");
    rt_kprintf("\n");
}

static void rpmsg_cmd(int argc, char *argv[])
{
    char *cmd;

    if (argc > 1)
    {
        cmd = argv[1];
        if (!rt_strcmp(cmd, "cpuusage"))
        {
            if (argc > 2)
            {
                cmd = argv[2];
                uint32_t id = atoi(cmd);
                if (id < PLATFORM_CORE_COUNT)
                {
                    rpmsg_cmd_cpuusage(&id);
                }
                else
                {
                    rt_kprintf("\n");
                    rt_kprintf("example: rpmsg_cmd cpuusage [0|1|2|3]\n");
                    rt_kprintf("\n");
                }
            }
            else
            {
                for (uint32_t id = 0; id < PLATFORM_CORE_COUNT; id++)
                {
                    rpmsg_cmd_cpuusage(&id);
                    rt_thread_mdelay(1);
                }
            }
        }
        else
        {
            rpmsg_cmd_usage();
        }
    }
    else
    {
        rpmsg_cmd_usage();
    }

    return;
}

#ifdef RT_USING_FINSH
#include <finsh.h>
MSH_CMD_EXPORT(rpmsg_cmd, rpmsg_cmd);
#endif

#endif

#endif  // RT_USING_RPMSG_LITE
