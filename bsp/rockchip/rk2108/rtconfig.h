#ifndef RT_CONFIG_H__
#define RT_CONFIG_H__

/* Automatically generated file; DO NOT EDIT. */
/* RT-Thread Configuration */

#define RKMCU_RK2108
#define PM_HAS_CUSTOM_CONFIG

/* RT-Thread Kernel */

#define RT_USING_CORE_RTTHREAD
#define RT_NAME_MAX 8
#define RT_ALIGN_SIZE 4
#define RT_THREAD_PRIORITY_32
#define RT_THREAD_PRIORITY_MAX 32
#define RT_TICK_PER_SECOND 1000
#define RT_USING_OVERFLOW_CHECK
#define RT_USING_HOOK
#define RT_HOOK_USING_FUNC_PTR
#define RT_USING_IDLE_HOOK
#define RT_IDEL_HOOK_LIST_SIZE 4
#define IDLE_THREAD_STACK_SIZE 512
#define RT_DEBUG
#define RT_DEBUG_USING_IO

/* Inter-Thread communication */

#define RT_USING_SEMAPHORE
#define RT_USING_MUTEX
#define RT_USING_EVENT
#define RT_USING_MAILBOX
#define RT_USING_MESSAGEQUEUE

/* Memory Management */

#define RT_USING_MEMPOOL
#define RT_USING_SMALL_MEM
#define RT_USING_MEMHEAP
#define RT_MEMHEAP_FAST_MODE
#define RT_USING_SMALL_MEM_AS_HEAP
#define RT_USING_HEAP

/* Kernel Device Object */

#define RT_USING_DEVICE
#define RT_USING_DEVICE_OPS
#define RT_USING_CONSOLE
#define RT_CONSOLEBUF_SIZE 128
#define RT_CONSOLE_DEVICE_NAME "uart2"
#define RT_VER_NUM 0x40101
#define ARCH_ARM
#define ARCH_ARM_CORTEX_M
#define ARCH_ARM_CORTEX_M4

/* RT-Thread Components */

#define RT_USING_COMPONENTS_INIT
#define RT_USING_USER_MAIN
#define RT_MAIN_THREAD_STACK_SIZE 2048
#define RT_MAIN_THREAD_PRIORITY 10
#define RT_USING_MSH
#define RT_USING_FINSH
#define FINSH_USING_MSH
#define FINSH_THREAD_NAME "tshell"
#define FINSH_USING_HISTORY
#define FINSH_HISTORY_LINES 5
#define FINSH_USING_SYMTAB
#define FINSH_USING_DESCRIPTION
#define FINSH_THREAD_PRIORITY 20
#define FINSH_THREAD_STACK_SIZE 4096
#define FINSH_CMD_SIZE 80
#define FINSH_USING_MSH
#define FINSH_USING_MSH_DEFAULT
#define FINSH_USING_MSH_ONLY
#define FINSH_ARG_MAX 10
#define RT_USING_DFS
#define DFS_USING_WORKDIR
#define DFS_FILESYSTEMS_MAX 2
#define DFS_FILESYSTEM_TYPES_MAX 2
#define DFS_FD_MAX 16
#define RT_USING_DFS_MNTTABLE
#define RT_USING_DFS_ELMFAT

/* elm-chan's FatFs, Generic FAT Filesystem Module */

#define RT_DFS_ELM_CODE_PAGE 437
#define RT_DFS_ELM_WORD_ACCESS
#define RT_DFS_ELM_USE_LFN_3
#define RT_DFS_ELM_USE_LFN 3
#define RT_DFS_ELM_LFN_UNICODE_0
#define RT_DFS_ELM_LFN_UNICODE 0
#define RT_DFS_ELM_MAX_LFN 255
#define RT_DFS_ELM_DRIVES 2
#define RT_DFS_ELM_MAX_SECTOR_SIZE 4096
#define RT_DFS_ELM_REENTRANT
#define RT_USING_DFS_DEVFS

/* Device Drivers */

#define RT_USING_DEVICE_IPC
#define RT_USING_SERIAL
#define RT_SERIAL_USING_DMA
#define RT_SERIAL_RB_BUFSZ 64
#define RT_USING_I2C
#define RT_USING_PIN
#define RT_USING_PWM
#define RT_USING_MTD_NOR
/*#define RT_USING_PM*/
#define RT_USING_SPI
/*#define RT_USING_AUDIO*/

/* Using WiFi */


/* C/C++ and POSIX layer */

#define RT_LIBC_DEFAULT_TIMEZONE 8

/* POSIX layer and C standard library */

#define RT_USING_LIBC
#define RT_USING_POSIX

/* Interprocess Communication (IPC) */


/* Socket is in the 'Network' category */


/* Network */


/* Utilities */

#define RT_USING_CMBACKTRACE
#define PKG_CMBACKTRACE_PLATFORM_M4
#define PKG_CMBACKTRACE_DUMP_STACK
#define PKG_CMBACKTRACE_PRINT_ENGLISH

/* ARM CMSIS */


/* RT-Thread Benchmarks */


/* RT-Thread third party package */


/* Bluetooth */


/* examples bluetooth */

/* Bluetooth examlpes */

/* Example 'BT API TEST' Config */


/* Example 'BT DISCOVERY' Config */


/* Example 'A2DP SINK' Config */


/* Example 'A2DP SOURCE' Config  */


/* Example 'HFP CLIENT' Config */


/* RT-Thread board config */

#define RT_BOARD_NAME "rk2108b_evb"
#define RK_SRAM_END 0x20100000
#define M4_JTAG_ENABLE

/* RT-Thread rockchip common drivers */

#define RT_USING_CACHE
#define RT_USING_UNCACHE_HEAP
#define RT_UNCACHE_HEAP_ORDER 0x0E
#define RT_USING_LARGE_HEAP
#define RT_LARGE_MALLOC_THRRESH 512
#define RT_LARGE_HEAP_SIZE 524288

/* Enable Fault Dump Hook */

#define RT_USING_SNOR
#define RT_SNOR_SPEED 80000000
#define RT_USING_SNOR_FSPI_HOST

/* RT-Thread rockchip jpeg enc driver */


/* RT-Thread rockchip pm drivers */

/*#define RT_USING_PM_DVFS
#define RT_USING_PM_REQ_PWR */

/* RT-Thread rockchip mipi-dphy driver */


/* RT-Thread rockchip isp driver */


/* RT-Thread rockchip vcm driver */


/* RT-Thread rockchip vicap driver */


/* RT-Thread rockchip camera driver */


/* RT-Thread rockchip vicap_lite driver */


/* RT-Thread rockchip csi2host driver */


/* RT-Thread rockchip buffer_manage driver */


/* RT-Thread rockchip coredump driver */


/* Enable PSTORE */


/* RT-Thread rockchip rk2108 drivers */

/* Enable Audio */


/* Audio Cards */


/* Audio Interfaces */

#define RT_USING_PCM
#define RT_USING_VAD
#define RT_VAD_IRQ_HANDLED_BY_DSP
#define RT_USING_CRU
#define RT_USING_DMA
#define RT_USING_DMA_PL330
#define RT_USING_DMA0
#define RT_USING_PMU
#define RT_USING_PWM0

/* Enable DSP */


/* Enable UART */

#define RT_USING_UART
#define RT_USING_UART0
#define RT_USING_UART1
#define RT_USING_UART2

/* Enable I2C */

#define RT_USING_I2C0
#define RT_USING_I2C1
#define RT_USING_I2C2

/* Enable SPI */

#define RT_USING_SPI2APB
#define RT_USING_SPI1
#define RT_USING_SPI2

/* RT-Thread application */


/* RT-Thread bsp test case */


/* RT-Thread Common Test case */

/*#define RT_USING_FWANALYSIS*/

#endif
