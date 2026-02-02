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

  if (!sd_flash.initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }
  
  if (len == 0) {
    return SL_STATUS_INVALID_PARAMETER;
  }

 /* addr must be inside flash */
  if ( addr >= sd_flash.size_bytes ) {
    return SL_STATUS_INVALID_PARAMETER;  // out of range
  }

  /* check overflow of addr + len */
  end_addr = addr + len;
  if (end_addr < addr) {
    return SL_STATUS_INVALID_PARAMETER;   // uint32_t overflow
  }

  /* end must not exceed flash size */
  if (end_addr > sd_flash.size_bytes) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  return SL_STATUS_OK;
}



/* ============================================================================
 * GPIO initialization
 * ========================================================================== */

static sl_status_t mx25_gpio_init(flash_spi_handle_t  handle)
{
  sl_gpio_t pinport;
  sl_status_t sl_status_code = SL_STATUS_OK;
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

    sl_status_code = sl_gpio_set_pin_mode(&pinport, SL_GPIO_MODE_PUSH_PULL, 1);

  }

  return sl_status_code;

}



/***************************************************************************/
/**
 * @brief Initialize SPI interface for Flash.
 ******************************************************************************/
static sl_status_t flash_spi_init(flash_spi_handle_t spi_handle)
{
  sl_status_t sl_status_code = SL_STATUS_OK;
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
  // spi_master_set_cs_mode(SPI_MASTER_CS_MODE_SW); @TODO the func hasn't implemented UP
  if (mx25_gpio_init(spi_handle) != SL_STATUS_OK) {
      return SL_STATUS_NOT_INITIALIZED;
  }

  sl_status_code = flash_spi_getBitRate(&bitRate);
  if (sl_status_code == SL_STATUS_OK ) {
     app_log("Flash SPI bitrate=%luMHZ \r\n",bitRate);
  } else {
     app_log("Flash SPI bitrate ERROR\r\n");
  }

  return sl_status_code;
}

sl_status_t flash_spi_getBitRate(uint32_t *bitRate)
{

  if (SPIDRV_GetBitrate(flash_spidrv_handle, bitRate) != ECODE_EMDRV_SPIDRV_OK)
  {
     return SL_STATUS_FAIL;
  }

  return SL_STATUS_OK;

}
/* ============================================================================
 * Public API
 * ========================================================================== */

sl_status_t flash_storage_init(void)
{
  sl_status_t st = SL_STATUS_OK;

  sd_flash.initialized = false;

#if defined(MX25_WP_PORT)
  /* ===============================
   * WP#  (active low)
   * =============================== */
  //GPIO_PinModeSet(MX25_WP_PORT, MX25_WP_PIN, gpioModePushPull, 1);  // HIGH = write enabled
  if (digital_out_init(&sd_flash.wp_pin,
                       hal_gpio_pin_name(MX25_WP_PORT, MX25_WP_PIN),
                       SL_GPIO_MODE_INPUT_PULL, 1) != DIGITAL_OUT_SUCCESS) {
    return SL_STATUS_NOT_INITIALIZED;
  }
#endif
#if defined(MX25_RST_PORT)
  /* ===============================
   * RESET# (active low)
   * =============================== */
  //  GPIO_PinModeSet(MX25_RST_PORT, MX25_RST_PIN, gpioModePushPull, 1);  // HIGH = normal operation
  if (digital_out_init(&sd_flash.wp_pin,
                       hal_gpio_pin_name(MX25_RST_PORT, MX25_RST_PIN),
                       SL_GPIO_MODE_INPUT_PULL, 1) != DIGITAL_OUT_SUCCESS) {
    return SL_STATUS_NOT_INITIALIZED;
  }
#endif

  app_flash_instance = flash_spidrv_handle;

  st = flash_spi_init(app_flash_instance);
  if (st != SL_STATUS_OK) {
    return st;
  }

  if (mx25_init(&sd_flash.spi) != F_RES_OK) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  st = mx25_detect_flash(&sd_flash.spi);
  if (st != SL_STATUS_OK) {
    return st;
  }

// Testing SPI CLK MAX bitrate
//  Set fast SPI speed
//  if (SPI_MASTER_SUCCESS != spi_master_set_speed(&sd_flash.spi, MX25_SPI_MAX_FREQ)) {  @TODO the Testing this an issue It was been added by UP
//    return SL_STATUS_TRANSMIT;
//  }
//   Attempt to re-detect SPI Flash at MAX SPI interface clock.
//  st = mx25_detect_flash(&sd_flash.spi); //UP @TODO the Testing this an issue It was been added by UP
//  if (st != SL_STATUS_OK) {
//    return st;
//  }


  sd_flash.size_bytes = mx25_get_size();
  sd_flash.initialized = true;

  app_log("Flash Macronix MX25R8035F init done\r\n");

  return st;
}


/* ============================================================================
 * Read / Write
 * ========================================================================== */

sl_status_t flash_storage_read(uint32_t addr,
                               uint8_t *buf,
                               uint32_t len)
{
  if (!sd_flash.initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  if (!buf) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sl_status_t st = validate_range(addr, len);
  if (st != SL_STATUS_OK) {
    return st;
  }


  return mx25_read(&sd_flash.spi, addr, buf, len);
}

sl_status_t flash_storage_write(uint32_t addr,
                                const uint8_t *buf,
                                uint32_t len)
{
  if (!sd_flash.initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  if (!buf) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sl_status_t st = validate_range(addr, len);
  if (st != SL_STATUS_OK) {
    return st;
  }

  if (mx25_ready_to_write_erase(&sd_flash.spi) != F_RES_OK) {
    return SL_STATUS_FLASH_PROGRAM_FAILED;
  }

  if (mx25_page_write(&sd_flash.spi, addr, buf, len) != F_RES_OK)
  {
    return SL_STATUS_FLASH_PROGRAM_FAILED;
  }

   return SL_STATUS_OK;
}

#if 0
/* ============================================================================
 * Erase
 * ========================================================================== */

sl_status_t flash_storage_erase_sector(uint32_t addr)
{
  if (!sd_flash.initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  if (mx25_ready_to_write_erase(&sd_flash.spi) != F_RES_OK) {
      return SL_STATUS_FLASH_ERASE_FAILED;
  }

  if (addr >= sd_flash.size_bytes) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  addr &= ~(MX25_SECTOR_SIZE - 1);
  if (mx25_erase_sector(&sd_flash.spi, addr) != F_RES_OK) {
    return SL_STATUS_FLASH_ERASE_FAILED;
  }

  return SL_STATUS_OK; 
}

sl_status_t flash_storage_erase_block64(uint32_t addr)
{
  if (!sd_flash.initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  if (mx25_ready_to_write_erase(&sd_flash.spi) != F_RES_OK) {
      return SL_STATUS_FLASH_ERASE_FAILED;
  }

  if (addr >= sd_flash.size_bytes) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  addr &= ~(MX25_BLOCK64_SIZE - 1);
  if (mx25_erase_block64(&sd_flash.spi, addr) != F_RES_OK) {
    return SL_STATUS_FLASH_ERASE_FAILED;
  }

  return SL_STATUS_OK; 
}
#endif

sl_status_t flash_storage_erase_chip(void)
{
  if (!sd_flash.initialized) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  if (mx25_ready_to_write_erase(&sd_flash.spi) != F_RES_OK) {
    return SL_STATUS_FLASH_ERASE_FAILED;
  }

  if (mx25_erase_chip(&sd_flash.spi) != F_RES_OK) {
    return SL_STATUS_FLASH_ERASE_FAILED;
  }
  
  return SL_STATUS_OK;  
}


