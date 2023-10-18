/**
  * Copyright (c) 2022 Fuzhou Rockchip Electronics Co., Ltd
  *
  * SPDX-License-Identifier: Apache-2.0
  ******************************************************************************
  * @file    main.c
  * @version V0.1
  * @brief   application entry
  *
  * Change Logs:
  * Date           Author          Notes
  * 2022-08-04     Cliff.Chen      first implementation
  *
  ******************************************************************************
  */
#include <rtthread.h>

extern rt_err_t rt_trustzone_enter(rt_ubase_t module);
extern rt_err_t rt_trustzone_exit(void);

int main(void)
{
#ifdef ARM_CM33_ENABLE_TRUSTZONE
    rt_ubase_t stack_size = 0x400;
#endif

    rt_kprintf("delay 10s test start\n");
    rt_thread_delay(10 * RT_TICK_PER_SECOND);
    rt_kprintf("delay 10s test end\n");

#ifdef ARM_CM33_ENABLE_TRUSTZONE
    rt_trustzone_enter(stack_size);
    /* Call NSCFunction */
    rt_trustzone_exit();
#endif
    return 0;
}
