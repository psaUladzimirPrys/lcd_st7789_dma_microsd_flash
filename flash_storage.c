/*
 * flash_storage.c
 *
 * High-level flash storage abstraction
 * Responsible for:
 *  - initialization
 *  - address validation
 *  - policy & error handling
 *  - SPI + LDMA setup
 */

#include <string.h>
#include "app_log.h"
#include "drv_digital_out.h"
#include "flash_storage.h"
#include "sl_spidrv_mikroe_config.h"

/* ============================================================================
 * Internal context
 * ========================================================================== */

/* Storage Data Flash Context and Controls */
typedef struct {
  spi_master_t      spi;
#if defined(MX25_WP_PORT)
  digital_out_t  wp_pin;
#endif
#if defined(MX25_RESET_PORT)
  digital_out_t rst_pin;
#endif
  bool      initialized;
  uint32_t   size_bytes;
} sd_flash_t;

static sd_flash_t sd_flash = { 0 }; //Analog val of static sd_card_t sd_card;

/* ============================================================================
 * SPI handle (used by mx25)
 * ========================================================================== */

static flash_spi_handle_t app_flash_instance = NULL;

/* Low-level Silicon Labs SPIDRV */
/* Provide init struct similar to autogen (these SL_SPIDRV_MIKROE_* macros are from your autogen) */
SPIDRV_HandleData_t mx25_spi_handle_data;
SPIDRV_Handle_t     flash_spidrv_handle = &mx25_spi_handle_data;
SPIDRV_Init_t flash_spidrv_init_mikroe = {
  .port = SL_SPIDRV_MIKROE_PERIPHERAL,
#if defined(_USART_ROUTELOC0_MASK)
  .portLocationTx = SL_SPIDRV_MIKROE_TX_LOC,
  .portLocationRx = SL_SPIDRV_MIKROE_RX_LOC,
  .portLocationClk = SL_SPIDRV_MIKROE_CLK_LOC,
#if defined(SL_SPIDRV_MIKROE_CS_LOC)
  .portLocationCs = SL_SPIDRV_MIKROE_CS_LOC,
#endif
#elif defined(_GPIO_USART_ROUTEEN_MASK)
  .portTx = SL_SPIDRV_MIKROE_TX_PORT,
  .portRx = SL_SPIDRV_MIKROE_RX_PORT,
  .portClk = SL_SPIDRV_MIKROE_CLK_PORT,
#if defined(MX25_CS_PORT)
  .portCs = MX25_CS_PORT,
#endif
  .pinTx = SL_SPIDRV_MIKROE_TX_PIN,
  .pinRx = SL_SPIDRV_MIKROE_RX_PIN,
  .pinClk = SL_SPIDRV_MIKROE_CLK_PIN,
#if defined(MX25_CS_PIN)
  .pinCs = MX25_CS_PIN,
#endif
#else
  .portLocation = SL_SPIDRV_MIKROE_ROUTE_LOC,
#endif
  .bitRate = SL_SPIDRV_MIKROE_BITRATE,
  .frameLength = SL_SPIDRV_MIKROE_FRAME_LENGTH,
  .dummyTxValue = 0,
  .type = SL_SPIDRV_MIKROE_TYPE,
  .bitOrder = SL_SPIDRV_MIKROE_BIT_ORDER,
  .clockMode = SL_SPIDRV_MIKROE_CLOCK_MODE,
  .csControl = spidrvCsControlApplication, // software CS
  .slaveStartMode = SL_SPIDRV_MIKROE_SLAVE_START_MODE,
};


/* ============================================================================
 * Address validation
 * ========================================================================== */
static sl_status_t validate_range(uint32_t addr, uint32_t len)
{
  uint32_t end_addr;

  if (len == 0) {
    return SL_STATUS_INVALID_PARAMETER;
  }

 /* addr must be inside flash */
  if ( addr >= sd_flash.size_bytes ) {
    return SL_STATUS_INVALID_RANGE;  // out of range
  }

  /* check overflow of addr + len */
  end_addr = addr + len;
  if (end_addr < addr) {
    return SL_STATUS_INVALID_RANGE;   // uint32_t overflow
  }

  /* end must not exceed flash size */
  if (end_addr > sd_flash.size_bytes) {
    return SL_STATUS_INVALID_RANGE;
  }

  return SL_STATUS_OK;
}



/* ============================================================================
 * GPIO initialization
 * ========================================================================== */

static sl_status_t mx25_gpio_init(flash_spi_handle_t  handle)
{
  sl_gpio_t pinport;
  sl_status_t sl_status = SL_STATUS_OK;
  SPIDRV_Handle_t handle_ptr = (SPIDRV_Handle_t)handle;
  
  /* The CS pin is driven by a input to the Flash.
   * the CS pin should be configured as a pull-up output
   * when the Flash is selected/deselected.
   */
  if (handle_ptr->initData.csControl == spidrvCsControlApplication) {

    pinport.port = handle_ptr->initData.portCs;
    pinport.pin = handle_ptr->initData.pinCs;

    /* @ToDo must remove after pins validation  It was been added by UP*/
    if ((pinport.port != SL_GPIO_PORT_C) && (pinport.pin != 3)) {
        return SL_STATUS_FAIL;
    }

    sl_status = sl_gpio_set_pin_mode(&pinport, SL_GPIO_MODE_PUSH_PULL, 1);

  }

  return sl_status;

}



/***************************************************************************/
/**
 * @brief Initialize SPI interface for Flash.
 ******************************************************************************/
static sl_status_t flash_spi_init(flash_spi_handle_t spi_handle)
{
  sl_status_t sl_status = SL_STATUS_OK;
  spi_master_config_t spi_cfg;
  uint32_t bitRate;

  /* Base SPIDRV init template */
  SPIDRV_Init_t *initData;

  /* Set SPI configuration to default value */
  spi_master_configure_default(&spi_cfg);

  /* SPI configuration */
  spi_cfg.mode    = SPI_MASTER_MODE_3;
  spi_cfg.cs_mode = SPI_MASTER_CS_MODE_SW;
  spi_cfg.speed   = MX25_SPI_DEFAULT_FREQ; //UP fake spees init
  spi_cfg.default_write_data = 0xFF;

  sd_flash.spi.handle = spi_handle;

  if (spi_master_open(&sd_flash.spi, &spi_cfg) != ACQUIRE_SUCCESS) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  /* ---------- MikroE SPI master init ---------- */
  initData = &flash_spidrv_init_mikroe;
  initData->bitRate   = MX25_SPI_DEFAULT_FREQ;
  initData->clockMode = spidrvClockMode3;
  initData->csControl = spidrvCsControlApplication;

  if (SPIDRV_Init((SPIDRV_Handle_t)spi_handle, initData) != ECODE_EMDRV_SPIDRV_OK) {
      return SL_STATUS_NOT_INITIALIZED;
  }

  /* CS handled manually */
  // spi_master_set_cs_mode(SPI_MASTER_CS_MODE_SW); @TODO the func hasn't implemented yet UP
  if (mx25_gpio_init(spi_handle) != SL_STATUS_OK) {
      return SL_STATUS_NOT_INITIALIZED;
  }

  sl_status = flash_spi_getBitRate(&bitRate);
  if (sl_status == SL_STATUS_OK ) {
     app_log("Flash SPI bitrate=%luMHZ \r\n",bitRate);
  } else {
     app_log("Flash SPI bitrate ERROR\r\n");
  }

  return sl_status;
}

sl_status_t flash_spi_getBitRate(uint32_t *bitRate)
{

  if (SPIDRV_GetBitrate(flash_spidrv_handle, bitRate) != ECODE_EMDRV_SPIDRV_OK) {
     return SL_STATUS_FAIL;
  }

  return SL_STATUS_OK;

}
/* ============================================================================
 * Public API
 * ========================================================================== */

sl_status_t flash_storage_init(void)
{
  sl_status_t sl_status = SL_STATUS_OK;

  sd_flash.initialized = false;

#if defined(MX25_WP_PORT)
  /* ===============================
   * WP#  (active low)
   * =============================== */
  //GPIO_PinModeSet(MX25_WP_PORT, MX25_WP_PIN, gpioModePushPull, 1);  // HIGH = write enabled
  if (digital_out_init(&sd_flash.wp_pin,
                       hal_gpio_pin_name(MX25_WP_PORT, MX25_WP_PIN),
                       SL_GPIO_MODE_PUSH_PULL, 1) != DIGITAL_OUT_SUCCESS) {
    return SL_STATUS_NOT_INITIALIZED;
  }
#endif
#if defined(MX25_RST_PORT)
  /* ===============================
   * RESET# (active low)
   * =============================== */
  //  GPIO_PinModeSet(MX25_RST_PORT, MX25_RST_PIN, gpioModePushPull, 1);  // HIGH = normal operation
  if (digital_out_init(&sd_flash.rst_pin,
                       hal_gpio_pin_name(MX25_RST_PORT, MX25_RST_PIN),
                       SL_GPIO_MODE_PUSH_PULL, 1) != DIGITAL_OUT_SUCCESS) {
    return SL_STATUS_NOT_INITIALIZED;
  }
#endif

  app_flash_instance = flash_spidrv_handle;

  sl_status = flash_spi_init(app_flash_instance);
  if (sl_status != SL_STATUS_OK) {
    return sl_status;
  }

  if (mx25_init(&sd_flash.spi) != F_RES_OK) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  if (mx25_detect_flash(&sd_flash.spi)!= F_RES_OK) {
    return SL_STATUS_NOT_SUPPORTED;
  }

// Testing SPI CLK MAX bitrate
//  Set fast SPI speed
//  if (SPI_MASTER_SUCCESS != spi_master_set_speed(&sd_flash.spi, MX25_SPI_MAX_FREQ)) {  @TODO the Testing this an issue It was been added by UP
//    return SL_STATUS_TRANSMIT;
//  }
//   Attempt to re-detect SPI Flash at MAX SPI interface clock.
//  if (mx25_detect_flash(&sd_flash.spi)!= F_RES_OK) {
//    return SL_STATUS_NOT_SUPPORTED;
//  }


  sd_flash.size_bytes = mx25_get_size();
  sd_flash.initialized = true;

  app_log("Flash Macronix MX25R8035F init done\r\n");

  return sl_status;
}


/* ============================================================================
 * Read / Write
 * ========================================================================== */
/*
 * Function:        flash_storage_read
 * Arguments:       uint32_t addr - Starting address in flash memory (0x000000 to 0x0FFFFF for 1MB)
 *                  uint8_t *buf - Pointer to buffer where read data will be stored
 *                  uint32_t len - Number of bytes to read (max limited by flash size)
 * Description:     Reads data from MX25R8035F flash memory through the storage abstraction layer.
 *                  This function performs safety checks including initialization state,
 *                  buffer validity, and address range validation. It then calls the low-level
 *                  mx25_read function to perform the actual SPI transaction. The function
 *                  supports reading any contiguous block within the 8Mb (1MB) address space.
 *
 * Return Message:  sl_status_t
 *                  - SL_STATUS_OK: Read operation completed successfully
 *                  - SL_STATUS_NOT_INITIALIZED: Flash storage not initialized (call flash_storage_init first)
 *                  - SL_STATUS_INVALID_PARAMETER: Null buffer pointer provided
 *                  - SL_STATUS_INVALID_RANGE: Address/length exceeds flash memory boundaries
 *                  - SL_STATUS_FAIL: Low-level read operation failed (SPI communication error)
 */
sl_status_t flash_storage_read(uint32_t addr,
                               uint8_t *buf,
                               uint32_t len)
{
  /* Check if flash storage driver has been properly initialized */
  if (!sd_flash.initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }
  /* Validate that caller provided a valid buffer pointer */
  if (!buf) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  /* Ensure the requested address range is within flash memory boundaries */
  sl_status_t sl_status = validate_range(addr, len);
  if (sl_status != SL_STATUS_OK) {
    return sl_status;
  }
  /* Perform the actual read operation through the MX25 driver */
  if (mx25_read(&sd_flash.spi, addr, buf, len) != F_RES_OK) {
     return SL_STATUS_FAIL;/* Low-level read failed - could indicate SPI communication error */
  }

  return SL_STATUS_OK;
}

/*
 * Function:        flash_storage_write
 * Arguments:       uint32_t addr - Starting address in flash memory (0 to FLASH_SIZE-1)
 *                  const uint8_t *buf - Pointer to data buffer to write
 *                  uint32_t len - Number of bytes to write (1 to maximum flash capacity)
 * Description:     Writes data to MX25R8035F flash memory storage. This high-level
 *                  function validates the storage system state, address range, and
 *                  device readiness before delegating to the low-level page write
 *                  function. It provides abstraction for the flash storage layer
 *                  with proper error handling and status reporting.
 *
 * Return Message: sl_status_t
 *                  - SL_STATUS_OK: Data successfully written to flash
 *                  - SL_STATUS_NOT_INITIALIZED: Flash storage not initialized
 *                  - SL_STATUS_INVALID_PARAMETER: Null buffer pointer
 *                  - SL_STATUS_INVALID_RANGE: Address/length outside valid range
 *                  - SL_STATUS_FLASH_PROGRAM_FAILED: Device not ready or write error
 */
sl_status_t flash_storage_write(uint32_t addr,
                                const uint8_t *buf,
                                uint32_t len)
{
  /* Check if flash storage system has been properly initialized */
  if (!sd_flash.initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }
  /* Validate input buffer pointer is not NULL */
  if (!buf) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  /* Ensure address and length are within valid flash memory boundaries */
  sl_status_t sl_status = validate_range(addr, len);
  if (sl_status != SL_STATUS_OK) {
    return sl_status;
  }
  /* Verify flash device is ready for write/erase operations (WIP=0, WEL can be set) */
  if (mx25_ready_to_write_erase(&sd_flash.spi) != F_RES_OK) {
    return SL_STATUS_FLASH_PROGRAM_FAILED;
  }
  /* Execute the actual page write operation to flash memory */
  if (mx25_page_write(&sd_flash.spi, addr, buf, len) != F_RES_OK) {
    return SL_STATUS_FLASH_PROGRAM_FAILED;
  }
  /* Write operation completed successfully */
   return SL_STATUS_OK;
}

/* ============================================================================
 * Erase
 * ========================================================================== */
/*
 * Function:        flash_storage_erase_sector
 * Arguments:       uint32_t addr - Any address within the 4KB sector to erase
 * Description:     Erases a 4KB sector in MX25R8035F flash memory. This high-level
 *                  function validates system initialization, device readiness,
 *                  and address alignment before delegating to low-level erase
 *                  function. The address is automatically aligned to sector
 *                  boundary (4KB). Sector erase operation sets all bits in the
 *                  sector to 1 (0xFF).
 *
 * Return Message: sl_status_t
 *                  - SL_STATUS_OK: Sector successfully erased
 *                  - SL_STATUS_NOT_INITIALIZED: Flash storage not initialized
 *                  - SL_STATUS_INVALID_PARAMETER: Address outside flash memory range
 *                  - SL_STATUS_FLASH_ERASE_FAILED: Device not ready or erase error
 */
sl_status_t flash_storage_erase_sector(uint32_t addr)
{/* Verify flash storage system has been properly initialized */
  if (!sd_flash.initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }
  /* Check if flash device is ready for write/erase operations (WIP=0) */
  if (mx25_ready_to_write_erase(&sd_flash.spi) != F_RES_OK) {
      return SL_STATUS_FLASH_ERASE_FAILED;
  }
  /* Validate address is within flash memory boundaries */
  if (addr >= sd_flash.size_bytes) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  /* Align address to 4KB sector boundary (clear lower 12 bits) */
  addr &= ~(MX25_SECTOR_SIZE - 1);

  /* Execute low-level sector erase operation */
  if (mx25_erase_sector(&sd_flash.spi, addr) != F_RES_OK) {
    return SL_STATUS_FLASH_ERASE_FAILED;
  }

  return SL_STATUS_OK;  /* Sector erase completed successfully */
}

/*
 * Function:        flash_storage_erase_block64
 * Arguments:       uint32_t addr - Address within the 64KB block to erase
 * Description:     Erases a 64KB block in the MX25R8035F flash memory. This high-level
 *                  function validates system initialization, device readiness, and
 *                  address alignment before performing the erase operation. It ensures
 *                  the address is aligned to the 64KB block boundary (lower 16 bits = 0)
 *                  and delegates to the low-level block erase function.
 *
 * Return Message: sl_status_t
 *                  - SL_STATUS_OK: Block successfully erased
 *                  - SL_STATUS_NOT_INITIALIZED: Flash storage not initialized
 *                  - SL_STATUS_FLASH_ERASE_FAILED: Device not ready or erase failed
 *                  - SL_STATUS_INVALID_PARAMETER: Address beyond flash capacity
 *
 * Note: 64KB block erase operation takes 0.43-2.1s typical (with high voltage)
 *       or 0.48-3.0s maximum in normal High Performance Mode.
 */
sl_status_t flash_storage_erase_block64(uint32_t addr)
{
  /* Verify flash storage system has been properly initialized */
  if (!sd_flash.initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  /* Check if flash device is ready for erase operation (WIP=0, protection not active) */
  if (mx25_ready_to_write_erase(&sd_flash.spi) != F_RES_OK) {
      return SL_STATUS_FLASH_ERASE_FAILED;
  }

  /* Validate that address is within flash memory boundaries */
  if (addr >= sd_flash.size_bytes) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  /* Align address to 64KB block boundary (mask lower 16 bits to 0) */
  addr &= ~(MX25_BLOCK64_SIZE - 1);

  /* Execute the 64KB block erase operation */
  if (mx25_erase_block64(&sd_flash.spi, addr) != F_RES_OK) {
    return SL_STATUS_FLASH_ERASE_FAILED;
  }

  return SL_STATUS_OK; /* Execute the 64KB block erase operation */
}

/*
 * Function:        flash_storage_erase_chip
 * Arguments:       None
 * Description:     Erases the entire MX25R8035F flash memory device (1MB). This
 *                  high-level function performs a Chip Erase operation which sets
 *                  all memory bits to 1 (0xFF). It verifies that the storage system
 *                  is initialized and the device is ready before executing the erase.
 *                  WARNING: This operation is destructive and irreversible - all
 *                  data will be permanently erased. Operation takes 35-120 seconds.
 *
 * Return Message: sl_status_t
 *                  - SL_STATUS_OK: Chip erase completed successfully
 *                  - SL_STATUS_NOT_INITIALIZED: Flash storage not initialized
 *                  - SL_STATUS_FLASH_ERASE_FAILED: Device not ready or erase error
 *
 * Note: Chip Erase will only execute if all Block Protect (BP3-BP0) bits are 0.
 *       If protected areas exist, the command will be ignored without error flag.
 */
sl_status_t flash_storage_erase_chip(void)
{
  /* Verify flash storage system has been properly initialized */
  if (!sd_flash.initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  /* Check if flash device is ready for write/erase operations */
  /* This verifies WIP=0 and ensures no other operations are in progress */
  if (mx25_ready_to_write_erase(&sd_flash.spi) != F_RES_OK) {
    return SL_STATUS_FLASH_ERASE_FAILED;
  }

  /* Execute the Chip Erase command sequence */
  if (mx25_erase_chip(&sd_flash.spi) != F_RES_OK) {
    return SL_STATUS_FLASH_ERASE_FAILED;
  }
  
  return SL_STATUS_OK;  
}

/*
 * Function:        flash_storage_wakeup_chip
 * Arguments:       void
 * Description:     Wakes up the MX25R8035F flash memory from low-power mode.
 *                  This function checks if the storage system is initialized,
 *                  then sends a wake-up command to exit power-down mode.
 *                  The wake-up process ensures the device is ready for
 *                  subsequent read/write operations. It should be called
 *                  before accessing flash after entering power-saving modes.
 *
 * Return Message: sl_status_t
 *                  - SL_STATUS_OK: Chip successfully woken up
 *                  - SL_STATUS_NOT_INITIALIZED: Flash storage not initialized
 *                  - SL_STATUS_FAIL: Wake-up command or identification failed
 */
sl_status_t flash_storage_wakeup_chip(void)
{
  /* Verify flash storage system initialization state */
  if (!sd_flash.initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }
  /* Execute low-level wake-up sequence and verify device response */
  if (mx25_wake_up(&sd_flash.spi) != F_RES_OK) {
      return SL_STATUS_FAIL;
  }
  /* Chip successfully awakened and ready for operations */
  return SL_STATUS_OK;
}

/*
 * Function:        flash_storage_enable_hpf_mode
 * Arguments:       void
 * Description:     Enables High Performance Mode in the flash storage system.
 *                  This function serves as a bridge between the high-level storage
 *                  API and the low-level flash driver. It checks if the flash storage
 *                  system has been properly initialized before attempting to change
 *                  the device's performance mode. The actual mode switching is
 *                  delegated to the MX25R8035F-specific driver function.
 *
 * Return Message: sl_status_t
 *                  - SL_STATUS_OK: High Performance Mode successfully enabled or
 *                    already active
 *                  - SL_STATUS_NOT_INITIALIZED: Flash storage system not initialized
 *                  - SL_STATUS_FAIL: Low-level driver failed to enable the mode
 *
 * Note: This is a wrapper function that ensures proper system state before
 *       executing device-specific operations. The mode change affects power
 *       consumption and SPI communication speed (up to 108 MHz in High
 *       Performance Mode vs 33 MHz in Ultra Low Power Mode).
 */
sl_status_t  flash_storage_enable_hpf_mode(void)
{

  /* Verify that the flash storage system has been properly initialized */
  /* The 'sd_flash' structure should have been initialized via flash_storage_init() */
  if (!sd_flash.initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  /* Delegate the actual mode change to the MX25R8035F-specific driver */
  /* This function handles the complete sequence: read config, modify bit, write back */
  if (mx25_enable_hpf_mode(&sd_flash.spi) != F_RES_OK) {
      return SL_STATUS_FAIL;
  }

  return SL_STATUS_OK;

}
