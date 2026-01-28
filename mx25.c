/*
 * mx25.c
 *
 *  Created on: 20 Jan. 2026.
 *      Author: priss
 */

/*
 * mx25.c
 *
 * Low-level driver for Macronix MX25xx SPI NOR Flash
 * Assumes parameters are validated by higher layer
 * Dependencies:
 *  - sl_spidrv.h
 *  - em_gpio.h
 *  - mx25_defs.h
 *  - mx25_config.h
 *
 * SPI mode:
 *  - Mode 0 or Mode 3 (depends on SPIDRV config)
 */

#include "mx25.h"
#include "mx25_config.h"

#include "spidrv.h"
#include "em_gpio.h"
#include "app_assert.h"

/* ============================================================================
 * Internal flash info
 * ========================================================================== */

typedef struct {
  uint8_t  manufacturer_id;
  uint16_t device_id;
  uint8_t  memory_type;
  uint8_t  capacity_id;
  uint32_t size_bytes;
  bool     detected;
} mx25_info_t;

static mx25_info_t mx25_info = { 0 };

/* ============================================================================
 * Local helpers
 * ========================================================================== */

/***************************************************************************//**
 * Deselect CS on SPI bus.
 ******************************************************************************/
/*
 * Function:       mx25_select,
 * Arguments:      spi_handle,
 * Description:    Chip select go low,
 * Return:         Results of MX25 Flash driver.
 */
static fresult_t mx25_select(spi_master_t *spi_handle)
{
  if (!spi_handle)   {
     return F_RES_PARAM_ERROR;
  }

  if (SPI_MASTER_SUCCESS != spi_master_control_cs(spi_handle, SPI_SLAVE_CHIP_SELECT_LOW)) {
      return F_RES_NOTRDY;
  }

  return F_RES_OK;
}

/***************************************************************************//**
 * Select CS on SPI bus.
 ******************************************************************************/
/*
 * Function:       mx25_deselect,
 * Arguments:      spi_handle,
 * Description:    Chip select go high,
 * Return:         Results of MX25 Flash driver.
 */
static fresult_t mx25_deselect(spi_master_t *spi_handle)
{
  if (!spi_handle)  {
    return F_RES_PARAM_ERROR;
  }

  if (SPI_MASTER_SUCCESS != spi_master_control_cs(spi_handle, SPI_SLAVE_CHIP_SELECT_HIGH)) {
    return F_RES_NOTRDY;
  }

  return F_RES_OK;
}

/***************************************************************************//**
 * Multi-byte SPI transaction (transmit).
 ******************************************************************************/
static fresult_t mx25_spi_tx(spi_master_t *spi_handle, const uint8_t *buff, uint32_t cnt)
{

 if ( (!spi_handle) || (!buff) ) {
    return F_RES_PARAM_ERROR;
 }

 if (SPI_MASTER_SUCCESS != spi_master_write(spi_handle, (uint8_t *)buff, cnt)) {
   return F_RES_WRITE_ERROR;
 }

 return F_RES_OK;
}
/***************************************************************************//**
 * Multi-byte SPI transaction (receive).
 ******************************************************************************/
static fresult_t mx25_spi_rx(spi_master_t *spi_handle, uint8_t *buff, uint32_t cnt)
{
  if ( (!spi_handle) || (!buff) ) {
     return F_RES_PARAM_ERROR;
  }

  if (SPI_MASTER_SUCCESS != spi_master_read(spi_handle, buff, cnt)) {
      return F_RES_READ_ERROR;
  }

  return F_RES_OK;
}

static fresult_t mx25_spi_trx(spi_master_t *spi_handle, const uint8_t *tx, uint8_t *rx, uint32_t len)
{
  if ( (!spi_handle) || (!tx) || (!rx) ) {
     return F_RES_PARAM_ERROR;
  }

  if (SPI_MASTER_SUCCESS != spi_master_write_then_read(spi_handle, (uint8_t *)tx, 1, rx, len)) {
    return F_RES_TRANSMIT_ERROR;
  }

  return F_RES_OK;
}

/* ============================================================================
 * Low-level commands
 * ========================================================================== */

static void mx25_write_enable(spi_master_t *spi_handle)
{
  uint8_t cmd = MX25_CMD_WREN;

  mx25_select(spi_handle);
  mx25_spi_tx(spi_handle, &cmd, 1);
  mx25_deselect(spi_handle);
}

static uint8_t mx25_read_status(spi_master_t *spi_handle)
{
  uint8_t cmd = MX25_CMD_RDSR;
  uint8_t sr  = 0;

  mx25_select(spi_handle);
  mx25_spi_tx(spi_handle, &cmd, 1);
  mx25_spi_rx(spi_handle, &sr, 1);
  mx25_deselect(spi_handle);

  return sr;
}

static sl_status_t mx25_wait_ready(spi_master_t *spi_handle)
{
  uint32_t timeout = MX25_OP_TIMEOUT;

  while (mx25_read_status(spi_handle) & MX25_SR_WIP) {
    /* busy wait */
    if (--timeout == 0) {
      return SL_STATUS_TIMEOUT;
    }
    MX25_DELAY_HOOK();
  }

  return SL_STATUS_OK;
}

/* ============================================================================
 * JEDEC / size
 * ========================================================================== */

static uint32_t mem_density_to_size(uint8_t mem_density)
{
  if ( mem_density != 0x14 ) {
      app_assert_status(SL_STATUS_FAIL);
  }
  uint32_t mbytes = 1UL << mem_density;

  if(mbytes != DEVICE_SIZE_8M) {
    app_assert_status(SL_STATUS_FAIL);
  }

  return mbytes;
}

/* ============================================================================
 * Public API
 * ========================================================================== */

sl_status_t mx25_init(spi_master_t *spi_handle)
{
  fresult_t f_res = F_RES_OK;

  /* CS pin must already be configured as output and set to HIGT */
  mx25_deselect(spi_handle);

  /* Wake up from deep power down (safe even if not in DP) */
  f_res = mx25_wake_up(spi_handle);

  MX25_VERIFY_SUCCESS_OR_RETURN(f_res);

  sl_sleeptimer_delay_millisecond(10);

  return f_res;
}

sl_status_t mx25_detect_flash(spi_master_t *spi_handle)
{
  uint8_t tx[4] = { MX25_CMD_RDID, 0, 0, 0 };
  uint8_t rx[4] = { 0 };

  //sl_status_t st;

  mx25_select(spi_handle);
  mx25_spi_trx(spi_handle, tx, rx, (sizeof(tx) - 1) );
  mx25_deselect(spi_handle);

  mx25_info.manufacturer_id = rx[0];
  mx25_info.memory_type     = rx[1];
  mx25_info.capacity_id     = rx[2];

  tx[0] = MX25_CMD_REMS;
  tx[1] = MX25_DUMMY;
  tx[2] = MX25_DUMMY;
  tx[3] = 0x1;

  mx25_select(spi_handle);
  mx25_spi_tx(spi_handle, tx, sizeof(tx));
  mx25_spi_rx(spi_handle, rx, 2);
  mx25_deselect(spi_handle);
  mx25_info.device_id = (uint16_t)((uint16_t)rx[0] << 8) | (uint16_t)rx[1];

  if (   (mx25_info.manufacturer_id != MX25_MANUFACTURER_ID)
      || (mx25_info.device_id != RESID1) ) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  mx25_info.size_bytes = mem_density_to_size(mx25_info.capacity_id);
  mx25_info.detected   = true;

  return SL_STATUS_OK;
}

uint32_t mx25_get_size(void)
{
  return mx25_info.size_bytes;
}

/* ============================================================================
 * Read / Write
 * ========================================================================== */

sl_status_t mx25_read(spi_master_t *spi_handle, uint32_t addr, uint8_t *buf, uint32_t len)
{
  uint8_t hdr[4] = {
    MX25_CMD_READ,
    (uint8_t)(addr >> 16),
    (uint8_t)(addr >> 8),
    (uint8_t)(addr)
  };

  mx25_select(spi_handle);
  mx25_spi_tx(spi_handle, hdr, sizeof(hdr));
  mx25_spi_rx(spi_handle, buf, len);
  mx25_deselect(spi_handle);

  return SL_STATUS_OK;
}

sl_status_t mx25_write(spi_master_t *spi_handle, uint32_t addr, const uint8_t *buf, uint32_t len)
{
  sl_status_t st;

  while (len > 0) {
    uint32_t page_offset = addr & (MX25_PAGE_SIZE - 1);
    uint32_t chunk = MX25_PAGE_SIZE - page_offset;
    if (chunk > len) {
      chunk = len;
    }

    mx25_write_enable(spi_handle);

    uint8_t hdr[4] = {
      MX25_CMD_PP,
      (uint8_t)(addr >> 16),
      (uint8_t)(addr >> 8),
      (uint8_t)(addr)
    };

    mx25_select(spi_handle);
    mx25_spi_tx(spi_handle, hdr, sizeof(hdr));
    mx25_spi_tx(spi_handle, buf, chunk);
    mx25_deselect(spi_handle);

    st = mx25_wait_ready(spi_handle);
    if (st != SL_STATUS_OK) {
      return SL_STATUS_FLASH_PROGRAM_FAILED;
    }

    addr += chunk;
    buf  += chunk;
    len  -= chunk;
  }

  return SL_STATUS_OK;
}

/* ============================================================================
 * Erase
 * ========================================================================== */

sl_status_t mx25_erase_sector(spi_master_t *spi_handle, uint32_t addr)
{
  /* Align to 4K sector */
  addr &= ~(MX25_SECTOR_SIZE - 1);


  mx25_write_enable(spi_handle);

  uint8_t cmd[4] = {
    MX25_CMD_SE,
    (uint8_t)(addr >> 16),
    (uint8_t)(addr >> 8),
    (uint8_t)(addr)
  };

  mx25_select(spi_handle);
  mx25_spi_tx(spi_handle, cmd, sizeof(cmd));
  mx25_deselect(spi_handle);

  sl_status_t st = mx25_wait_ready(spi_handle);
  return (st == SL_STATUS_OK)
         ? SL_STATUS_OK
         : SL_STATUS_FLASH_ERASE_FAILED;
}

#if defined(TESTTESETST)
sl_status_t mx25_erase_block64(uint32_t addr)
{
  addr &= ~(MX25_BLOCK64_SIZE - 1);

  mx25_write_enable();

  uint8_t cmd[4] = {
    MX25_CMD_BE64,
    (uint8_t)(addr >> 16),
    (uint8_t)(addr >> 8),
    (uint8_t)(addr)
  };

  mx25_select();
  mx25_spi_tx(cmd, sizeof(cmd));
  mx25_deselect();

  sl_status_t st = mx25_wait_ready();
  return (st == SL_STATUS_OK)
         ? SL_STATUS_OK
         : SL_STATUS_FLASH_ERASE_FAILED;
}

sl_status_t mx25_erase_chip(void)
{
  mx25_write_enable();

  uint8_t cmd = MX25_CMD_CE;

  mx25_select();
  mx25_spi_tx(&cmd, 1);
  mx25_deselect();

  sl_status_t st = mx25_wait_ready();
  return (st == SL_STATUS_OK)
         ? SL_STATUS_OK
         : SL_STATUS_FLASH_ERASE_FAILED;
}

/* ============================================================================
 * Power management
 * ========================================================================== */

sl_status_t mx25_power_down(void)
{
  uint8_t cmd = MX25_CMD_DP;

  mx25_select();
  mx25_spi_tx(&cmd, 1);
  mx25_deselect();

  return SL_STATUS_OK;
}
#endif
/*
 * Function:       MX25_RES  Release the chip from powerdown mode
 * Arguments:      ElectricIdentification, 8 bit buffer to store electric id
 * Description:    The RES instruction is to read the Device
 *                 electric identification of 1-byte.
 * Return Message: FlashOperationSuccess
 */
fresult_t mx25_wake_up(spi_master_t *spi_handle)
{
  fresult_t  f_res = F_RES_OK;
  uint8_t tx[4] = { MX25_CMD_RES, 0, 0, 0 };
  uint8_t rx[4] = { 0 };


  mx25_select(spi_handle);
  f_res = mx25_spi_trx(spi_handle, tx, rx, (sizeof(tx) - 1) );
  mx25_deselect(spi_handle);

  MX25_VERIFY_SUCCESS_OR_RETURN(f_res);

  // Get electric identification
  if (rx[0] ==  ElectronicID) {
    f_res = F_RES_READ_ERROR;
  }

  return f_res;
}

