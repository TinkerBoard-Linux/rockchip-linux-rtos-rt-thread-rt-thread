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
__FLASH_SIZE = 0x00800000;

/*--------------------- Embedded RAM Configuration ----------------------------
  <h> RAM Configuration
    <o0> RAM Base Address    <0x0-0xFFFFFFFF:8>
    <o1> RAM Size (in Bytes) <0x0-0xFFFFFFFF:8>
  </h>
 -----------------------------------------------------------------------------*/
__SRAM_BASE = 0x10200000;
__SRAM_SIZE = 0x00100000;

/*--------------------- External DRAM Configuration ----------------------------
  <h> RAM Configuration
    <o0> Hyper RAM Base Address    <0x0-0xFFFFFFFF:8>
    <o1> Hyper RAM Size (in Bytes) <0x0-0xFFFFFFFF:8>
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

/*
 *-------------------- <<< end of configuration section >>> -------------------
 */

#ifdef RT_USING_UNCACHE_HEAP
/* Uncachable memory size
*/
__UNCACHE_HEAP_SIZE = 0x8000;
#else
__UNCACHE_HEAP_SIZE = 0x0;
#endif
__SRAM_SIZE = __SRAM_SIZE - __UNCACHE_HEAP_SIZE;
__UNCACHE_HEAP_BASE = __SRAM_BASE + __SRAM_SIZE;

/* ARMv8-M stack sealing:
   to use ARMv8-M stack sealing set __STACKSEAL_SIZE to 8 otherwise keep 0
 */
__STACKSEAL_SIZE = 0;

/* UNSECURE_OFFSET: For cpu running in unsecure state
*/
__UNSECURE_OFFSET = 0x60000;
__SRAM_I_BASE = __SRAM_BASE + __UNSECURE_OFFSET;
__SRAM_I_SIZE = 0x20000;
__SRAM_D_BASE = __SRAM_I_BASE + __SRAM_I_SIZE + 0x20000000;
__SRAM_D_SIZE = 0x20000;

__DRAM_I_BASE = __DRAM_BASE + __UNSECURE_OFFSET;
__DRAM_I_SIZE = 0x20000;
__DRAM_D_BASE = __DRAM_I_BASE + __DRAM_I_SIZE + 0x20000000;
__DRAM_D_SIZE = 0x20000;

MEMORY
{
  CODE (rx) : ORIGIN = __SRAM_I_BASE,  LENGTH = __SRAM_I_SIZE
  DATA (rw) : ORIGIN = __SRAM_D_BASE,  LENGTH = __SRAM_D_SIZE
#ifdef RT_USING_UNCACHE_HEAP
  SRAM_U (rw)  : ORIGIN = __UNCACHE_HEAP_BASE, LENGTH = __UNCACHE_HEAP_SIZE
#endif
}

SECTIONS
{
    .text :
    {
        PROVIDE(_stext = .);
        KEEP(*(.text.entrypoint))       /* The entry point */
        *(.vectors)
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
    .ARM.exidx   :
    {
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    } > CODE
    __exidx_end = .;

    __rodata_start = .;
    .rodata   :
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

    . = ALIGN(32);

    .data :
    {
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
   PROVIDE(__code_end__ = ORIGIN(CODE) + LENGTH(CODE));
   PROVIDE(__data_start__ = ORIGIN(DATA));
   PROVIDE(__data_end__ = ORIGIN(DATA) + LENGTH(DATA));
   PROVIDE(__device_start__ = 0x40000000);
   PROVIDE(__device_end__ = 0x40E00000);
}
