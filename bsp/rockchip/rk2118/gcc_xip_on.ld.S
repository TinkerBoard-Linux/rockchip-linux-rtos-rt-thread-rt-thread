/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2022 Rockchip Electronics Co., Ltd.
 */

#include "rtconfig.h"

/*---------------------- Flash Configuration ----------------------------------
  <h> Flash Configuration
    <o0> Flash Base Address <0x0-0xFFFFFFFF:8>
    <o1> Flash Size (in Bytes) <0x0-0xFFFFFFFF:8>
  </h>
  -----------------------------------------------------------------------------*/
__FLASH_BASE = 0x11000000;
__FLASH_SIZE = 0x00400000;

/*--------------------- Shared Memory Configuration ----------------------------
  <h> RAM Configuration
    <o0> RAM Base Address    <0x0-0xFFFFFFFF:8>
    <o1> RAM Size (in Bytes) <0x0-0xFFFFFFFF:8>
  </h>
 -----------------------------------------------------------------------------*/
__SRAM_BASE = 0x30200000;
__SRAM_SIZE = 0x00100000;

/*--------------------- DDR SDRAM Configuration ----------------------------
  <h> RAM Configuration
    <o0> DDR SDRAM Base Address    <0x0-0xFFFFFFFF:8>
    <o1> DDR SDRAM Size (in Bytes) <0x0-0xFFFFFFFF:8>
  </h>
 -----------------------------------------------------------------------------*/
__DRAM_BASE = 0xa0000000;
__DRAM_SIZE = 0x04000000;

/*--------------------- Stack / Heap Configuration ----------------------------
  <h> Stack / Heap Configuration
    <o0> Stack Size (in Bytes) <0x0-0xFFFFFFFF:8>
  </h>
  -----------------------------------------------------------------------------*/
__STACK_SIZE = 0x00000400;

/* ARMv8-M stack sealing:
   to use ARMv8-M stack sealing set __STACKSEAL_SIZE to 8 otherwise keep 0
 */
__STACKSEAL_SIZE = 0;

#ifdef RT_USING_UNCACHE_HEAP
/* Uncachable memory size
*/
__UNCACHE_HEAP_SIZE = 0x8000;
#else
__UNCACHE_HEAP_SIZE = 0x0;
#endif
__SRAM_SIZE = __SRAM_SIZE - __UNCACHE_HEAP_SIZE;
__UNCACHE_HEAP_BASE = __SRAM_BASE + __SRAM_SIZE;

/*
 *-------------------- <<< layout of memory  >>> -------------------
 <h> flash layout
    <o0> RK partition table
    <o1> IDBLOCK
    <o2> CPU0 TF-M(XIP): code
    <o3> CPU0 RTT(XIP,NS): code
    <o4> DSP0 HAL(NS): code
    <o5> DSP1 HAL(NS): code
    <o6> DSP2 HAL(NS): code
    <o7> CPU1 RTT(XIP,NS,OPT): code
    <o7> User data(NS,OPT): fs
 </h>
 <h> sram layout
    <o0> CPU0 TF-M: data
    <o1> CPU0 RTT(NS): data
    <o2> DSP0 HAL(NS): code & data
    <o4> DSP1 HAL(NS): code & data
    <o6> DSP2 HAL(NS): code & data
 </h>
 <h> dram layout
    <o0> CPU0 TF-M: data
    <o1> CPU0 RTT(NS): data
    <o2> DSP0 HAL(NS): code & data
    <o4> DSP1 HAL(NS): code & data
    <o6> DSP2 HAL(NS): code & data
    <o8> CPU1 RTT(NS,OPT): code & data
 </h>
 */
__FLASH_RKPART_TABLE_BASE = __FLASH_BASE;
__FLASH_RKPART_TABLE_SIZE = 0x10000;
__FLASH_IDBLOCK_BASE = __FLASH_RKPART_TABLE_BASE + __FLASH_RKPART_TABLE_SIZE;
__FLASH_IDBLOCK_SIZE = 0x20000;
__FLASH_CPU0_TFM_BASE = __FLASH_IDBLOCK_BASE + __FLASH_IDBLOCK_SIZE;
__FLASH_CPU0_TFM_SIZE = 0x20000;
__FLASH_CPU0_RTT_BASE = __FLASH_CPU0_TFM_BASE + __FLASH_CPU0_TFM_SIZE;
__FLASH_CPU0_RTT_SIZE = 0x40000;
__FLASH_DSP0_FIREWARE_BASE = __FLASH_CPU0_RTT_BASE + __FLASH_CPU0_RTT_SIZE;
__FLASH_DSP0_FIREWARE_SIZE = 0x100000;
__FLASH_DSP1_FIREWARE_BASE = __FLASH_DSP0_FIREWARE_BASE + __FLASH_DSP0_FIREWARE_SIZE;
__FLASH_DSP1_FIREWARE_SIZE = 0x100000;
__FLASH_DSP2_FIREWARE_BASE = __FLASH_DSP1_FIREWARE_BASE + __FLASH_DSP1_FIREWARE_SIZE;
__FLASH_DSP2_FIREWARE_SIZE = 0x100000;
__FLASH_CPU1_RTT_BASE = __FLASH_DSP2_FIREWARE_BASE + __FLASH_DSP2_FIREWARE_SIZE;
__FLASH_CPU1_RTT_SIZE = 0x40000;
__FLASH_USER_DATA_BASE = __FLASH_CPU1_RTT_BASE + __FLASH_CPU1_RTT_SIZE;
__FLASH_USER_DATA_SIZE = __FLASH_BASE + __FLASH_SIZE - __FLASH_USER_DATA_BASE;
ASSERT(__FLASH_USER_DATA_SIZE > 0x200, "no flash space for userdata")

__SRAM_CPU0_TFM_BASE = __SRAM_BASE;
__SRAM_CPU0_TFM_SIZE = 0x10000;
__SRAM_CPU0_RTT_BASE = __SRAM_CPU0_TFM_BASE + __SRAM_CPU0_TFM_SIZE;
__SRAM_CPU0_RTT_SIZE = 0x10000;
__SRAM_DSP0_BASE = __SRAM_CPU0_RTT_BASE + __SRAM_CPU0_RTT_SIZE;
__SRAM_DSP0_SIZE = 0x50000;
__SRAM_DSP1_BASE = __SRAM_DSP0_BASE + __SRAM_DSP0_SIZE;
__SRAM_DSP1_SIZE = 0x48000;
__SRAM_DSP2_BASE = __SRAM_DSP1_BASE + __SRAM_DSP1_SIZE;
__SRAM_DSP2_SIZE = __SRAM_BASE + __SRAM_SIZE - __SRAM_DSP2_BASE;
ASSERT(__SRAM_DSP2_SIZE > 0, "no sram space for DSP2")

__DRAM_CPU0_TFM_BASE = __DRAM_BASE;
__DRAM_CPU0_TFM_SIZE = 0x100000;
__DRAM_CPU0_RTT_BASE = __DRAM_CPU0_TFM_BASE + __DRAM_CPU0_TFM_SIZE;
__DRAM_CPU0_RTT_SIZE = 0x100000;
__DRAM_DSP0_BASE = __DRAM_CPU0_RTT_BASE + __DRAM_CPU0_RTT_SIZE;
__DRAM_DSP0_SIZE = 0x1500000;
__DRAM_DSP1_BASE = __DRAM_DSP0_BASE + __DRAM_DSP0_SIZE;
__DRAM_DSP1_SIZE = 0x1400000;
__DRAM_DSP2_BASE = __DRAM_DSP1_BASE + __DRAM_DSP1_SIZE;
__DRAM_DSP2_SIZE = 0x1400000;
__DRAM_CPU1_RTT_BASE = __DRAM_DSP2_BASE + __DRAM_DSP2_SIZE;
__DRAM_CPU1_RTT_SIZE = __DRAM_BASE + __DRAM_SIZE - __DRAM_CPU1_RTT_BASE;
ASSERT(__DRAM_CPU1_RTT_SIZE > 0, "no ddr space for DSP2")

MEMORY
{
#if defined(RK2118_CPU_CORE0)
  CODE (rx) : ORIGIN = __FLASH_CPU0_RTT_BASE,  LENGTH = __FLASH_CPU0_RTT_SIZE
  DATA (rw) : ORIGIN = __SRAM_CPU0_RTT_BASE,  LENGTH = __SRAM_CPU0_RTT_SIZE
#elif defined(RK2118_CPU_CORE1)
  CODE (rx) : ORIGIN = __FLASH_CPU1_RTT_BASE,  LENGTH = __FLASH_CPU1_RTT_SIZE
  DATA (rw) : ORIGIN = __DRAM_CPU1_RTT_BASE,  LENGTH = __DRAM_CPU1_RTT_SIZE
#else
  #error "must select core0 or core1 in menuconfig"
#endif
#ifdef RT_USING_UNCACHE_HEAP
  SRAM_U (rw)  : ORIGIN = __UNCACHE_HEAP_BASE, LENGTH = __UNCACHE_HEAP_SIZE
#endif
}

SECTIONS
{
    .text :
    {
        . = ALIGN(4);
        KEEP(*(.vectors))               /* vector table */
        PROVIDE(_stext = .);
        KEEP(*(.text.entrypoint))       /* The entry point */
        *(.text)                        /* remaining code */
        *(.text.*)                      /* remaining code */

        *(.rodata)                      /* read-only data (constants) */
        *(.rodata*)
        *(.glue_7)
        *(.glue_7t)
        *(.gnu.linkonce.t*)

        *(COMMON)

        /* section information for finsh shell */
        . = ALIGN(16);
        __fsymtab_start = .;
        KEEP(*(FSymTab))
        __fsymtab_end = .;
        . = ALIGN(16);
        __vsymtab_start = .;
        KEEP(*(VSymTab))
        __vsymtab_end = .;
        . = ALIGN(16);

        /* section information for initial. */
        . = ALIGN(16);
        __rt_init_start = .;
        KEEP(*(SORT(.rti_fn*)))
        __rt_init_end = .;
        . = ALIGN(16);

        . = ALIGN(32);
        PROVIDE(_etext = .);
    } > CODE

    __exidx_start = .;
    .ARM.exidx :
    {
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    } > CODE
    __exidx_end = .;

    .rodata :
    {
        *(.rodata) *(.rodata.*)
    } > CODE
    __rodata_end = .;

    .zero.table :
    {
        . = ALIGN(4);
        __zero_table_start__ = .;
        LONG (ADDR(.bss))
        LONG (SIZEOF(.bss) / 4)
        __zero_table_end__ = .;
    } > CODE

    .copy.table :
    {
        . = ALIGN(4);
        __copy_table_start__ = .;
        LONG (__Vectors)
        LONG (ORIGIN(DATA))
        LONG (__Vectors_Size / 4)
        LONG (LOADADDR(.data))
        LONG (ADDR(.data))
        LONG (SIZEOF(.data) / 4)
        __copy_table_end__ = .;
    } > CODE

    . = ALIGN(16);

    .ctors :
    {
        PROVIDE(__ctors_start__ = .);
        KEEP(*(SORT(.ctors.*)))
        KEEP(*(.ctors))
        PROVIDE(__ctors_end__ = .);
    } > CODE

    .dtors :
    {
        PROVIDE(__dtors_start__ = .);
        KEEP(*(SORT(.dtors.*)))
        KEEP(*(.dtors))
        PROVIDE(__dtors_end__ = .);
    } > CODE

    .vector_remap (NOLOAD):
    {
        PROVIDE(__vector_remap__ = .);
        . = . + __Vectors_Size;
    } > DATA

    .data :
    {
        . = ALIGN(32);
        *(.data)
        *(.data.*)

        . = ALIGN(16);
    } > DATA AT > CODE

    . = ALIGN(16);

    .bss (NOLOAD) :
    {
        PROVIDE(__bss_start__ = .);
        *(.bss)
        *(.bss.*)
        *(.dynbss)
        *(COMMON)
        . = ALIGN(16);
        PROVIDE(__bss_end__ = .);
    } > DATA

    .heap :
    {
        . = ALIGN(32);
        PROVIDE(__heap_begin__ = .);
        . = ORIGIN(DATA) + LENGTH(DATA) - __STACK_SIZE - __STACKSEAL_SIZE;
        PROVIDE(__heap_end__ = .);
        . = ALIGN(32);
    } > DATA

   .stack (ORIGIN(DATA) + LENGTH(DATA) - __STACK_SIZE - __STACKSEAL_SIZE) (COPY) :
   {
        . = ALIGN(32);
        __StackLimit = .;
        . = . + __STACK_SIZE;
        __StackTop = .;
        . = ALIGN(32);
        PROVIDE(__stack = __StackTop);
        PROVIDE(_sstack = __StackLimit);
        PROVIDE(_estack = __StackTop);
    } > DATA

#ifdef RT_USING_UNCACHE_HEAP
    .uncache.heap (NOLOAD) :
    {
        . = ALIGN(__UNCACHE_HEAP_SIZE);
        PROVIDE(__uncache_heap_start__ = .);
        . = . + __UNCACHE_HEAP_SIZE;
        PROVIDE(__uncache_heap_end__ = .);
        . = ALIGN(__UNCACHE_HEAP_SIZE);
    } > SRAM_U
#endif

   PROVIDE(__code_start__ = ORIGIN(CODE));
   PROVIDE(__code_end__ = ORIGIN(CODE) + LENGTH(CODE) - 1);
   PROVIDE(__data_start__ = ORIGIN(DATA));
   PROVIDE(__data_end__ = ORIGIN(DATA) + LENGTH(DATA) - 1);
   PROVIDE(__device_start__ = 0x50000000);
   PROVIDE(__device_end__ = 0x50E00000 - 1);
}
