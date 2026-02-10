/*
 * flash_storage.h
 *
 * High-level flash storage abstraction over MX25 SPI NOR
 * Uses sl_status_t for error reporting
 */

#ifndef FLASH_STORAGE_H_
#define FLASH_STORAGE_H_

// -----------------------------------------------------------------------------
//                       Includes
// -----------------------------------------------------------------------------
#include <stdint.h>
#include "sl_status.h"
#include "em_gpio.h"
#include "mx25.h"
#include "mx25_config.h"


#ifdef __cplusplus
extern "C" {
#endif

typedef void * flash_spi_handle_t; ///< Created SPI Flash handle type

/* ============================================================================
 * Initialization
 * ========================================================================== */

/**
 * @brief Initialize flash storage subsystem.
 *
 * Performs:
 *  - SPI initialization
 *  - MX25 init
 *  - JEDEC detect
 *
 * @return sl_status_t
 */
sl_status_t flash_storage_init(void);

/* ============================================================================
 * Data access
 * ========================================================================== */

/**
 * @brief Read data from flash.
 */
sl_status_t flash_storage_read(uint32_t addr,
                               uint8_t *buf,
                               uint32_t len);

/**
 * @brief Write data to flash.
 *
 * Flash must be erased before writing.
 */
sl_status_t flash_storage_write(uint32_t addr,
                                const uint8_t *buf,
                                uint32_t len);

/**
 * @brief Erase 4KB sector.
 */
sl_status_t flash_storage_erase_sector(uint32_t addr);


sl_status_t flash_storage_erase_block64(uint32_t addr);

/**
 * @brief Erase entire chip.
 */
sl_status_t flash_storage_erase_chip(void);


sl_status_t flash_spi_getBitRate(uint32_t *bitRate);

sl_status_t flash_storage_wakeup_chip(void);

sl_status_t  flash_storage_enable_hpf_mode(void);

#ifdef __cplusplus
}
#endif

#endif /* FLASH_STORAGE_H_ */
