/*
 * Copyright (c) 2023 Rockchip Electronics Co., Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __RPMSG_CMD_H__
#define __RPMSG_CMD_H__

#include "rtconfig.h"
#ifdef RT_USING_RPMSG_CMD

#include <rtdevice.h>
#include "hal_base.h"
#include "rpmsg_lite.h"
#include "rpmsg_queue.h"
#include "rpmsg_base.h"

/* RPMsg cmd callback */
struct rpmsg_cmd_func_t
{
    uint32_t cmd;
    void (*rx_cb)(void *param);
};
typedef struct rpmsg_cmd_func_t rpmsg_func_t;

/* RPMsg cmd handle */
struct rpmsg_cmd_handle_t
{
    struct rpmsg_lite_instance *instance;
    struct rpmsg_lite_endpoint *ept;
    uint32_t endpoint;
    uint32_t master;
    uint32_t remote;
    struct rt_messagequeue *mq;
    rpmsg_func_t *ftable;
    uint32_t ftable_size;
};
typedef struct rpmsg_cmd_handle_t rpmsg_handle_t;

/* RPMsg cmd handle */
struct rpmsg_cmd_handle_list_t
{
    rpmsg_handle_t *handles;
    uint32_t count;
};
typedef struct rpmsg_cmd_handle_list_t rpmsg_handle_list;

/* RPMSG cmd head format */
struct rpmsg_cmd_head_t
{
    uint32_t type;
    uint32_t cmd;
    void    *priv;
    void    *addr;
};
typedef struct rpmsg_cmd_head_t rpmsg_head_t;

/* RPMSG cmd data format */
struct rpmsg_cmd_data_t
{
    rpmsg_head_t head;
    rpmsg_handle_t *handle;
    void (*rx_cb)(void *param);
    void *param;
};
typedef struct rpmsg_cmd_data_t rpmsg_data_t;

/* RPMsg mq max count */
#define RPMSG_MQ_MAX        32UL

/* RPMsg cmd type define */
#define RPMSG_TYPE_DIRECT   1UL
#define RPMSG_TYPE_URGENT   2UL
#define RPMSG_TYPE_NORMAL   3UL

/* RPMsg cmd define */
#define RPMSG_CMD2ACK(val)  (val | 0x01UL)      /* CMD covert to ACK */
#define RPMSG_ACK2CMD(val)  (val & (~0x01UL))   /* ACK covert to CMD */

#define RPMSG_CMD_GET_CPU_USAGE ((1UL << 1) + 0)
#define RPMSG_ACK_GET_CPU_USAGE ((1UL << 1) + 1)
#define RPMSG_CMD_GET_ECN_USAGE ((2UL << 1) + 0)
#define RPMSG_ACK_GET_ECN_USAGE ((2UL << 1) + 1)

/* RPMsg Common API Functions */
rt_err_t rpmsg_cmd_send(rpmsg_handle_t *handle, rpmsg_head_t *head, uint32_t timeout);
rpmsg_handle_t *rpmsg_cmd_get_handle(uint32_t id);
int rpmsg_cmd_thread_init(rpmsg_handle_list *handle_list, rt_uint32_t stack_size, rt_uint8_t  priority);

/* RPMsg Command Functions */
#if defined(PRIMARY_CPU) /*CPU1*/
void rpmsg_cmd_cpuusage(void *param);
#endif

/* RPMsg Command Callback Functions */
void rpmsg_cmd_cpuusage_callback(void *param);

#endif  //RT_USING_RPMSG_LITE

#endif  //__RPMSG_CMD_H__
