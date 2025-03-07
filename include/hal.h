/* hal.h
 *
 * The HAL API definitions.
 *
 * Copyright (C) 2021 wolfSSL Inc.
 *
 * This file is part of wolfBoot.
 *
 * wolfBoot is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * wolfBoot is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1335, USA
 */

#ifndef H_HAL_
#define H_HAL_

#ifdef __cplusplus
extern "C" {
#endif

#include "target.h"

/* Architecture specific calls */
#ifdef MMU
extern void do_boot(const uint32_t *app_offset, const uint32_t* dts_offset);
#else
extern void do_boot(const uint32_t *app_offset);
#endif
extern void arch_reboot(void);

/* Simulator-only calls */
#ifdef PLATFORM_sim
void hal_set_internal_flash_file(const char* file);
void hal_set_external_flash_file(const char* file);
void hal_deinit();
#endif

void hal_init(void);
int hal_flash_write(uint32_t address, const uint8_t *data, int len);
int hal_flash_erase(uint32_t address, int len);
void hal_flash_unlock(void);
void hal_flash_lock(void);
void hal_prepare_boot(void);

#ifdef DUALBANK_SWAP
    void hal_flash_dualbank_swap(void);
#endif

#ifdef WOLFBOOT_DUALBOOT
    void* hal_get_primary_address(void);
    void* hal_get_update_address(void);
#endif

#ifdef MMU
    void *hal_get_dts_address(void);
    void *hal_get_dts_update_address(void);
#endif

#if !defined(SPI_FLASH) && !defined(QSPI_FLASH)
    /* user supplied external flash interfaces */
    int  ext_flash_write(uintptr_t address, const uint8_t *data, int len);
    int  ext_flash_read(uintptr_t address, uint8_t *data, int len);
    int  ext_flash_erase(uintptr_t address, int len);
    void ext_flash_lock(void);
    void ext_flash_unlock(void);
#else
    #include "spi_flash.h"
    #define ext_flash_lock() do{}while(0)
    #define ext_flash_unlock() do{}while(0)
    #define ext_flash_read spi_flash_read
    #define ext_flash_write spi_flash_write
    static inline int ext_flash_erase(uintptr_t address, int len)
    {
        int ret = 0;
        uint32_t end = address + len - 1;
        uint32_t p;
        for (p = address; p <= end; p += SPI_FLASH_SECTOR_SIZE) {
            ret = spi_flash_sector_erase(p);
            if (ret != 0) {
                break;
            }
        }
        return ret;
    }
#endif /* !SPI_FLASH */

#ifdef WOLFBOOT_TRACE
void wolfboot_trace(const char *s);
#else
#define wolfboot_trace(X) (X)
#endif

#ifdef __cplusplus
}
#endif

#endif /* H_HAL_FLASH_ */
