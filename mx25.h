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
  F_RES_WRITE_INHIBITED,    //10: Write Inhibit Error.
  F_RES_ERASE_ERROR,        //11: Erase Error.
  F_RES_INVALID_ID          //12: Invalid Factory ID.

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
 * @return F_RES_OK  - success
 * @return F_RES_READ_ERROR - failure
 */
 fresult_t mx25_init(spi_master_t *spi_handle);

/* ============================================================================
 * Identification
 * ========================================================================== */
/**
 * @brief Detect flash using JEDEC ID and determine size.
 *
 * Must be called once after mx25_init().
 */
fresult_t mx25_detect_flash(spi_master_t *spi_handle);

/* ============================================================================
 * Identification / geometry
 * ========================================================================== */
/**
 * @brief Get detected flash size in bytes.
 */
uint32_t mx25_get_size(void);

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
 * @return F_RES_OK  - success
 * @return F_RES_READ_ERROR - invalid parameters
 */
fresult_t mx25_read(spi_master_t *spi_handle, uint32_t addr, uint8_t *buf, uint32_t len);


/**
 * @brief Reads protection configuration from Status and Configuration registers.
 *
 * Extracts BP[3:0] bits and TB bit for analysis.
 *
 * @param[in] spi_master_t *spi_handle - SPI controller instance
 *
 * @return F_RES_OK: Successfully read protection status
 * @return F_RES_WPROTECT_ERROR: SPI communication error
 */
fresult_t  mx25_ready_to_write_erase(spi_master_t *spi_handle);

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
 * @return F_RES_OK   - success
 * @return F_RES_WRITE_ERROR - invalid
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
 * @return F_RES_OK  - success
 */
fresult_t mx25_erase_sector(spi_master_t *spi_handle, uint32_t addr);

/**
 * @brief Erase 64KB block containing address.
 *
 * @param[in] addr  Address within block
 *
 * @return F_RES_OK   - success
 */
fresult_t mx25_erase_block64(spi_master_t *spi_handle, uint32_t addr);

/**
 * @brief Erase entire flash chip.
 *
 * WARNING: This operation may take several seconds.
 *
 * @return F_RES_OK   - success
 */
fresult_t mx25_erase_chip(spi_master_t *spi_handle);

/* ============================================================================
 * Power management
 * ========================================================================== */

/**
 * @brief Put flash into deep power-down mode.
 *
 * @return F_RES_OK - success
 */
fresult_t mx25_power_down(spi_master_t *spi_handle);

/**
 * @brief Wake flash from deep power-down mode.
 *
 * @return F_RES_OK - success
 */
fresult_t mx25_wake_up(spi_master_t *spi_handle);

/**
 * @brief Performs a software reset of the MX25R8035F flash memory device.
 *
 * @return F_RES_OK - success
 */
fresult_t  mx25_reset(spi_master_t *spi_handle);

#ifdef __cplusplus
}
#endif

#endif /* MX25_H_ */
