/*
 * Copyright (c) 2023 Rockchip Electronics Co., Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <stdint.h>

#include <rthw.h>
#include <rtthread.h>

#ifdef RT_USING_RPMSG_CMD

#include "rpmsg_cmd.h"

//#define RPMSG_CMD_DEBUG

#if defined(PRIMARY_CPU) /*CPU1*/

void rpmsg_cmd_cpuusage_callback(void *param)
{
    rpmsg_data_t *p_rpmsg_data = (rpmsg_data_t *)param;
    rpmsg_handle_t *handle = p_rpmsg_data->handle;
    rpmsg_head_t *head = &p_rpmsg_data->head;
    uint32_t usage;

    usage = *(uint32_t *)head->addr;
#ifdef RPMSG_CMD_DEBUG
    rt_kprintf("cpu%d: cpu usage is %d%\n", handle->remote, usage);
#endif

    rt_free_shmem(head->addr);
}

void rpmsg_cmd_cpuusage(void *param)
{
    uint32_t id = *(uint32_t *)param;
    uint32_t usage;

    if (id == HAL_CPU_TOPOLOGY_GetCurrentCpuId())
    {
        usage = rt_cpu_usage_get();
#ifdef RPMSG_CMD_DEBUG
        rt_kprintf("cpu%d: cpu usage is %d%\n", id, usage);
#endif
    }
    else
    {
        rt_err_t ret;
        rpmsg_head_t headdat;
        rpmsg_head_t *head = &headdat;

        rt_memset(head, 0, sizeof(rpmsg_head_t));
        head->type = RPMSG_TYPE_NORMAL;
        head->cmd  = RPMSG_CMD_GET_CPU_USAGE;
        head->addr = rt_malloc_shmem(sizeof(uint32_t));
        RT_ASSERT(head->addr != RT_NULL);

        ret = rpmsg_cmd_send(rpmsg_cmd_get_handle(id), head, RL_BLOCK);
        RT_ASSERT(ret == RL_SUCCESS);
    }
}

#else

void rpmsg_cmd_cpuusage_callback(void *param)
{
    rpmsg_data_t *p_rpmsg_data = (rpmsg_data_t *)param;
    rpmsg_head_t *head = &p_rpmsg_data->head;

    *(uint32_t *)head->addr = rt_cpu_usage_get();

    head->cmd = RPMSG_CMD2ACK(head->cmd);
    rpmsg_cmd_send(p_rpmsg_data->handle, head, RL_BLOCK);
}

#endif

#endif
