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

static rpmsg_handle_list rpmsg_cmd_handle_list;

static void *rpmsg_cmd_cb_find(uint32_t cmd, rpmsg_func_t *func_table, uint32_t size)
{
    uint32_t i;

    for (i = 0; i < size; i++)
    {
        if (func_table[i].cmd == cmd)
        {
            return (void *)(func_table[i].rx_cb);
        }
    }

    return RT_NULL;
}

static void rpmsg_cmd_process(rpmsg_data_t *p_rpmsg_data, uint32_t len)
{
    rpmsg_head_t *head = &p_rpmsg_data->head;
    rpmsg_handle_t *handle = p_rpmsg_data->handle;

    p_rpmsg_data->param = p_rpmsg_data;
    p_rpmsg_data->rx_cb = rpmsg_cmd_cb_find(head->cmd, handle->ftable, handle->ftable_size);
    if (p_rpmsg_data->rx_cb)
    {
        p_rpmsg_data->rx_cb(p_rpmsg_data->param);
    }
    else
    {
        rt_kprintf("%s: Get CMD(ACK) -- (%d), no funciton to process!\n", __func__, head->cmd);
    }
}

static int32_t rpmsg_ept_cb(void *payload, uint32_t payload_len, uint32_t src, void *priv)
{
    rpmsg_data_t rpmsg_data;
    rpmsg_data_t *p_rpmsg_data = &rpmsg_data;
    rpmsg_head_t *head = &p_rpmsg_data->head;
    rpmsg_handle_t *handle = (rpmsg_handle_t *)priv;

    RT_ASSERT(payload_len == sizeof(rpmsg_head_t));
    rt_memcpy(head, payload, sizeof(rpmsg_data_t));
    p_rpmsg_data->handle = handle;

    if (head->type == RPMSG_TYPE_DIRECT)
    {
        rpmsg_cmd_process(p_rpmsg_data, sizeof(rpmsg_data_t));
    }
    else if (head->type == RPMSG_TYPE_URGENT)
    {
        rt_mq_urgent(handle->mq, p_rpmsg_data, sizeof(rpmsg_data_t));
    }
    else
    {
        rt_mq_send(handle->mq, p_rpmsg_data, sizeof(rpmsg_data_t));
    }

    return RL_RELEASE;
}

rt_err_t rpmsg_cmd_send(rpmsg_handle_t *handle, rpmsg_head_t *head, uint32_t timeout)
{
    int32_t  ret;

#ifdef PRIMARY_CPU
    ret = rpmsg_lite_send(handle->instance, handle->ept,
                          EPT_M2R_ADDR(handle->endpoint),
                          head, sizeof(rpmsg_head_t),
                          timeout);
#else
    ret = rpmsg_lite_send(handle->instance, handle->ept,
                          EPT_R2M_ADDR(handle->endpoint),
                          head, sizeof(rpmsg_head_t),
                          timeout);
#endif

    return ret;
}

rpmsg_handle_t *rpmsg_cmd_get_handle(uint32_t id)
{
    rpmsg_handle_t *handle = rpmsg_cmd_handle_list.handles;
    uint32_t cnt = rpmsg_cmd_handle_list.count;

#ifdef PRIMARY_CPU
    for (uint32_t i = 0; i < cnt; i++)
    {
        if (id == handle[i].remote)
        {
            return &handle[i];
        }
    }
    return RT_NULL;
#else
    return &handle[0];
#endif
}

static void rpmsg_cmd_init(rpmsg_handle_t *handle, uint32_t count)
{
    struct rt_messagequeue *mq = rt_mq_create("rpmsg_mq", sizeof(rpmsg_data_t), RPMSG_MQ_MAX, RT_IPC_FLAG_FIFO);

    for (uint32_t i = 0; i < count; i++)
    {

#ifdef PRIMARY_CPU
        handle->instance = rpmsg_master_get_instance(MASTER_ID, handle->remote);
#else
        handle->instance = rpmsg_remote_get_instance(MASTER_ID, handle->remote);
#endif
        handle->ept = rpmsg_lite_create_ept(handle->instance, handle->endpoint, rpmsg_ept_cb, handle);
        handle->mq  = mq;
        handle++;
    }

    return RT_EOK;
}

static void rpmsg_cmd_entry(void *arg)
{
    rpmsg_data_t rpmsg_data;
    struct rt_messagequeue *mq;
    rpmsg_handle_t *handle = rpmsg_cmd_handle_list.handles;
    uint32_t cnt = rpmsg_cmd_handle_list.count;

    /* Initial handles: creat instance, ept and ... */
    rpmsg_cmd_init(handle, cnt);
    mq = handle->mq;

    while (1)
    {
        if (rt_mq_recv(mq, &rpmsg_data, sizeof(rpmsg_data_t), RT_WAITING_FOREVER) == RT_EOK)
        {
            rpmsg_cmd_process(&rpmsg_data, sizeof(rpmsg_data_t));
        }
    }
}

int rpmsg_cmd_thread_init(rpmsg_handle_list *handle_list, rt_uint32_t stack_size, rt_uint8_t  priority)
{
    rt_memcpy(&rpmsg_cmd_handle_list, handle_list, sizeof(rpmsg_handle_list));

    rt_thread_t thread = rt_thread_create("rpmsg_cmd", rpmsg_cmd_entry, RT_NULL,
                                          stack_size, priority, 10);
    if (thread)
        rt_thread_startup(thread);

    return RT_EOK;
}

#endif  // RT_USING_RPMSG_LITE
