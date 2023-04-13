/*
 * Copyright (c) 2006-2022, RT-Thread Development Team
 * Copyright (c) 2022, Xiaohua Semiconductor Co., Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-04-28     CDT          first version
 */

#include <rtdevice.h>
#include "board_config.h"

/**
 * The below functions will initialize HC32 board.
 */

#if defined RT_USING_SERIAL
rt_err_t rt_hw_board_uart_init(CM_USART_TypeDef *USARTx)
{
    rt_err_t result = RT_EOK;

    switch ((rt_uint32_t)USARTx)
    {
#if defined(BSP_USING_UART1)
    case (rt_uint32_t)CM_USART1:
        /* Configure USART RX/TX pin. */
        GPIO_SetFunc(USART1_RX_PORT, USART1_RX_PIN, GPIO_FUNC_33);
        GPIO_SetFunc(USART1_TX_PORT, USART1_TX_PIN, GPIO_FUNC_32);
        break;
#endif
#if defined(BSP_USING_UART2)
    case (rt_uint32_t)CM_USART2:
        /* Configure USART RX/TX pin. */
        GPIO_SetFunc(USART2_RX_PORT, USART2_RX_PIN, GPIO_FUNC_37);
        GPIO_SetFunc(USART2_TX_PORT, USART2_TX_PIN, GPIO_FUNC_36);
        break;
#endif
#if defined(BSP_USING_UART3)
    case (rt_uint32_t)CM_USART3:
        /* Configure USART RX/TX pin. */
        GPIO_SetFunc(USART3_RX_PORT, USART3_RX_PIN, GPIO_FUNC_33);
        GPIO_SetFunc(USART3_TX_PORT, USART3_TX_PIN, GPIO_FUNC_32);
        break;
#endif
#if defined(BSP_USING_UART4)
    case (rt_uint32_t)CM_USART4:
        /* Configure USART RX/TX pin. */
        GPIO_SetFunc(USART4_RX_PORT, USART4_RX_PIN, GPIO_FUNC_37);
        GPIO_SetFunc(USART4_TX_PORT, USART4_TX_PIN, GPIO_FUNC_36);
        break;
#endif
    default:
        result = -RT_ERROR;
        break;
    }

    return result;
}
#endif

#if defined(RT_USING_CAN)
void CanPhyEnable(void)
{
    GPIO_ResetPins(CAN_STB_PORT, CAN_STB_PIN);
    GPIO_OutputCmd(CAN_STB_PORT, CAN_STB_PIN, ENABLE);
}
rt_err_t rt_hw_board_can_init(CM_CAN_TypeDef *CANx)
{
    rt_err_t result = RT_EOK;

    switch ((rt_uint32_t)CANx)
    {
#if defined(BSP_USING_CAN1)
case (rt_uint32_t)CM_CAN:
    GPIO_SetFunc(CAN1_TX_PORT, CAN1_TX_PIN, CAN1_TX_PIN_FUNC);
    GPIO_SetFunc(CAN1_RX_PORT, CAN1_RX_PIN, CAN1_RX_PIN_FUNC);
    break;
#endif
default:
    result = -RT_ERROR;
    break;
    }

    return result;
}
#endif
