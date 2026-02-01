/*
 * mx25.h
 *
 * Application-level driver for Macronix MX25xx SPI NOR Flash
 *
 * Supported devices:
 *   MX25Lxxxx / MX25Rxxxx family
 *
 * Transport:
 *   SPI (via Silicon Labs SPIDRV or USART SPI)
 */

#ifndef MX25_H_
#define MX25_H_

// -----------------------------------------------------------------------------
//                       Includes
// -----------------------------------------------------------------------------

#include <stdint.h>
#include <stdbool.h>

#include "sl_status.h"
#include "sl_constants.h"
#include "drv_spi_master.h"
#include "spidrv.h"
#include "mx25_defs.h"



#ifdef __cplusplus
extern "C" {
#endif


/// Results of MX25 Flash driver
typedef enum {
  F_RES_OK = 0,            // 0: Successful.
  F_RES_READ_ERROR,        // 1: Read Error.
  F_RES_WRITE_ERROR,       // 2: Write Error.
  F_RES_WPROTECT_ERROR,    // 3: Write Protected Error.
  F_RES_NOTRDY,            // 4: Not Ready.
  F_RES_PARAM_ERROR,       // 5: Invalid Parameter.
  F_RES_TIMEOUT_ERROR,     // 6: Timeout Expired.
  F_RES_TRANSMIT_ERROR,    // 7: Transmit data Error.
  F_RES_INVALID_ADDRESS,   // 8: Invalid flash address.
  F_RES_MISALIGNED_ADDRESS, //9: Misaligned flash word address.
  F_RES_WRITE_INHIBITED    //10: Write Inhibit Error.
} fresult_t;

#define SPI_MASTER_CS_ENABLE   SPI_SLAVE_CHIP_SELECT_LOW
#define SPI_MASTER_CS_DISABLE  SPI_SLAVE_CHIP_SELECT_HIGH

#define MX25_VERIFY_SUCCESS_OR_RETURN(x) \
  do {                                   \
    if ((x) != F_RES_OK) {               \
      return (x);                        \
    }                                    \
  } while (0)

/* ============================================================================
 * Initialization
 * ========================================================================== */

/**
 * @brief Initialize MX25 SPI flash driver.
 *
 * SPI peripheral and CS GPIO must be initialized before calling this function.
 *
 * @return true  - success
 * @return false - failure
 */
  sl_status_t mx25_init(spi_master_t *spi_handle);

/* ============================================================================
 * Identification
 * ========================================================================== */

/* ============================================================================
 * Identification / geometry
 * ========================================================================== */

/**
 * @brief Detect flash using JEDEC ID and determine size.
 *
 * Must be called once after mx25_init().
 */
sl_status_t mx25_detect_flash(spi_master_t *spi_handle);

/**
 * @brief Get detected flash size in bytes.
 */
uint32_t mx25_get_size(void);

/**
 * @brief Read JEDEC ID.
 *
 * @param[out] manufacturer  Manufacturer ID (0xC2 for Macronix)
 * @param[out] device        Device ID (16-bit)
 *
 * @return true  - success
 * @return false - invalid parameters
 */
//bool mx25_read_id(uint8_t *manufacturer, uint16_t *device);

/* ============================================================================
 * Read operations
 * ========================================================================== */

/**
 * @brief Read data from flash.
 *
 * @param[in]  addr  Start address
 * @param[out] buf   Destination buffer
 * @param[in]  len   Number of bytes to read
 *
 * @return SL_STATUS_OK  - success
 * @return false - invalid parameters
 */
sl_status_t mx25_read(spi_master_t *spi_handle, uint32_t addr, uint8_t *buf, uint32_t len);

/* ============================================================================
 * Write operations
 * ========================================================================== */

/**
 * @brief Write data to flash (page aligned internally).
 *
 * Automatically splits writes across page boundaries.
 *
 * @param[in] addr  Start address
 * @param[in] buf   Source buffer
 * @param[in] len   Number of bytes to write
 *
 * @return SL_STATUS_OK  - success
 * @return SL_STATUS_FLASH_PROGRAM_FAILED - invalid
 */
fresult_t mx25_page_write(spi_master_t *spi_handle, uint32_t addr, const uint8_t *buf, uint32_t len);

/* ============================================================================
 * Erase operations
 * ========================================================================== */

/**
 * @brief Erase 4KB sector containing address.
 *
 * @param[in] addr  Address within sector
 *
 * @return SL_STATUS_OK  - success
 */
fresult_t mx25_erase_sector(spi_master_t *spi_handle, uint32_t addr);

/**
 * @brief Erase 64KB block containing address.
 *
 * @param[in] addr  Address within block
 *
 * @return SL_STATUS_OK  - success
 */
sl_status_t mx25_erase_block64(spi_master_t *spi_handle, uint32_t addr);

/**
 * @brief Erase entire flash chip.
 *
 * WARNING: This operation may take several seconds.
 *
 * @return SL_STATUS_OK  - success
 */
fresult_t mx25_erase_chip(spi_master_t *spi_handle);

/* ============================================================================
 * Power management
 * ========================================================================== */

/**
 * @brief Put flash into deep power-down mode.
 *
 * @return SL_STATUS_OK - success
 */
sl_status_t mx25_power_down(spi_master_t *spi_handle);

/**
 * @brief Wake flash from deep power-down mode.
 *
 * @return F_RES_OK - success
 */
fresult_t mx25_wake_up(spi_master_t *spi_handle);

#ifdef __cplusplus
}
#endif

#endif /* MX25_H_ */
