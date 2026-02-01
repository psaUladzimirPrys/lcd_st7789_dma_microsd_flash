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
#include "hal_target.h"

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
static volatile uint16_t mx25_timeout_timer; // 100Hz decrement timer
static sl_sleeptimer_timer_handle_t mx_25_timeout_timer_handle;
static void mx25_timer_callback(sl_sleeptimer_timer_handle_t *handle, void *data);
void mx25_timer_proc(void);
/* ============================================================================
 * Local helpers
 * ========================================================================== */
#define Delay_30us()  sl_udelay_wait(30 * UDELAY_MUL_FACTOR)
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

static fresult_t mx25_spi_trx(spi_master_t *spi_handle, const uint8_t *tx, uint32_t tx_len, uint8_t *rx, uint32_t rx_len)
{
  if ( (!spi_handle) || (!tx) || (!rx) ) {
     return F_RES_PARAM_ERROR;
  }

  if (SPI_MASTER_SUCCESS != spi_master_write_then_read(spi_handle, (uint8_t *)tx, tx_len, rx, rx_len)) {
    return F_RES_TRANSMIT_ERROR;
  }

  return F_RES_OK;
}

/***************************************************************************//**
 * Sleeptimer callback function to generate flash control timing.
 ******************************************************************************/
static void mx25_timer_callback(sl_sleeptimer_timer_handle_t *handle, void *data)
{
  (void)handle;
  (void)data;
  mx25_timer_proc();
}
/***************************************************************************//**
 * Device timer function.
 * This function must be called from timer interrupt routine in period
 * of 1 ms to generate flash control timing.
 ******************************************************************************/
void mx25_timer_proc(void)
{
  uint16_t n;
  n = mx25_timeout_timer;
  if (n) {
    mx25_timeout_timer = --n;
  }

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
  uint8_t status_reg  = 0;

  mx25_select(spi_handle);
  mx25_spi_tx(spi_handle, &cmd, 1);
  mx25_spi_rx(spi_handle, &status_reg, 1);
  mx25_deselect(spi_handle);

  return status_reg;
}


/*
 * Function:       MX25_RDSCUR
 * Arguments:      SecurityReg, 8 bit buffer to store security register value
 * Description:    The RDSCUR instruction is for reading the value of
 *                 Security Register bits.
 * Return Message: FlashOperationSuccess
 */
static uint8_t mx25_read_security(spi_master_t *spi_handle)
{
  uint8_t cmd = MX25_CMD_RDSCUR;
  uint8_t status_reg  = 0;

  mx25_select(spi_handle);
  mx25_spi_tx(spi_handle, &cmd, 1);
  mx25_spi_rx(spi_handle, &status_reg, 1);
  mx25_deselect(spi_handle);

  return status_reg;
}


static fresult_t mx25_wait_ready(spi_master_t *spi_handle, uint16_t op_timeout)
{
  mx25_timeout_timer = op_timeout;

  while (mx25_read_status(spi_handle) & MX25_SR_WIP) {
    /* busy wait */
    if (mx25_timeout_timer == 0) {
      return F_RES_TIMEOUT_ERROR;
    }
    MX25_DELAY_HOOK();
  }

  return F_RES_OK;
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
  bool timer_is_running = false;

  /* Make sure the mx_25_timeout_timer_handle timer is initialized only once */
  sl_sleeptimer_is_timer_running(&mx_25_timeout_timer_handle,
                                 &timer_is_running);
  if (timer_is_running == false) {
  /* Start a periodic timer 10 ms to generate flash control timing */
    sl_sleeptimer_start_periodic_timer_ms(&mx_25_timeout_timer_handle,
                                          10,
                                          mx25_timer_callback,
                                          (void *)NULL,
                                          0,
                                          0);
  }

  /* CS pin must already be configured as output and set to HIGT */
  mx25_deselect(spi_handle);

  /* Wake up from deep power down (safe even if not in DP) */
  f_res = mx25_wake_up(spi_handle);

  MX25_VERIFY_SUCCESS_OR_RETURN(f_res);

  return f_res;
}

sl_status_t mx25_detect_flash(spi_master_t *spi_handle)
{
  uint8_t tx_cmd[4] = { MX25_CMD_RDID, 0, 0, 0 };
  uint8_t rx[4] = { 0 };

  mx25_select(spi_handle);
  mx25_spi_trx(spi_handle, tx_cmd, 1, rx, (sizeof(tx_cmd) - 1) );
  mx25_deselect(spi_handle);

  Delay_30us();// tRES1/tRES2 = 30 µs max

  mx25_info.manufacturer_id = rx[0];
  mx25_info.memory_type     = rx[1];
  mx25_info.capacity_id     = rx[2];

  tx_cmd[0] = MX25_CMD_REMS;
  tx_cmd[1] = MX25_DUMMY;
  tx_cmd[2] = MX25_DUMMY;
  tx_cmd[3] = 0x1;

  mx25_select(spi_handle);
  mx25_spi_tx(spi_handle, tx_cmd, sizeof(tx_cmd));
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
 * Erase
 * ========================================================================== */

fresult_t mx25_erase_chip(spi_master_t *spi_handle)
{
  uint8_t cmd = MX25_CMD_CE;
  fresult_t f_res = F_RES_OK;

  // Check flash is busy or not
  if( mx25_read_status(spi_handle) & MX25_SR_WIP ) {
      return F_RES_NOTRDY;
  }

  mx25_write_enable(spi_handle);
  // Check Write Enable Latch is set
  if( (mx25_read_status(spi_handle) & MX25_SR_WEL) != MX25_SR_WEL ) {
      return F_RES_WRITE_INHIBITED;
  }

  mx25_select(spi_handle);
  f_res = mx25_spi_tx(spi_handle, &cmd, 1);
  mx25_deselect(spi_handle);
  MX25_VERIFY_SUCCESS_OR_RETURN(f_res);

  f_res = mx25_wait_ready(spi_handle, 16000);

  //Read Erase Fail Flag bit.
  if ( (mx25_read_security(spi_handle) & MX25_FSR_E_FAIL) ==  MX25_FSR_E_FAIL)
  {
    return F_RES_WRITE_ERROR;
  }

  return f_res;
}


fresult_t mx25_erase_sector(spi_master_t *spi_handle, uint32_t addr)
{

  fresult_t f_res = F_RES_OK;

  uint8_t tx_cmd[4] = { MX25_CMD_SE, 0, 0, 0};

  /* Align to 4K sector */
  addr &= ~(MX25_SECTOR_SIZE - 1);
  tx_cmd[1] = (uint8_t)(addr >> 16);
  tx_cmd[2]=  (uint8_t)(addr >> 8);
  tx_cmd[3] = (uint8_t)(addr);

  // Check flash is busy or not
  if( mx25_read_status(spi_handle) & MX25_SR_WIP ) {
      return F_RES_NOTRDY;
  }

  mx25_write_enable(spi_handle);

  // Check Write Enable Latch is set
  if( (mx25_read_status(spi_handle) & MX25_SR_WEL) != MX25_SR_WEL ) {
      return F_RES_WRITE_INHIBITED;
  }

  mx25_select(spi_handle);
  mx25_spi_tx(spi_handle, tx_cmd, sizeof(tx_cmd));
  mx25_deselect(spi_handle);

  f_res = mx25_wait_ready(spi_handle, 100);

  //Read Erase Fail Flag bit.
  if ( (mx25_read_security(spi_handle) & MX25_FSR_E_FAIL) ==  MX25_FSR_E_FAIL)
  {
    return F_RES_WRITE_ERROR;
  }

  return f_res;
}

/* ============================================================================
 * Read / Write
 * ========================================================================== */

sl_status_t mx25_read(spi_master_t *spi_handle, uint32_t addr, uint8_t *buf, uint32_t len)
{
  fresult_t f_res = F_RES_OK;

  uint8_t tx_cmd[4] = {
    MX25_CMD_READ,
    (uint8_t)(addr >> 16),
    (uint8_t)(addr >> 8),
    (uint8_t)(addr)
  };

  mx25_select(spi_handle);
  mx25_spi_tx(spi_handle, tx_cmd, sizeof(tx_cmd));
  f_res = mx25_spi_rx(spi_handle, buf, len);
  mx25_deselect(spi_handle);

  return (f_res == F_RES_OK)
         ? SL_STATUS_OK
         : SL_STATUS_FAIL;
}


fresult_t mx25_page_write(spi_master_t *spi_handle, uint32_t addr, const uint8_t *buf, uint32_t len)
{
   fresult_t f_res = F_RES_OK;
   uint32_t page_offset = 0;
   uint32_t chunk = 0;

   uint8_t tx_cmd[4] = { MX25_CMD_PP, 0, 0, 0 };


   // Check flash is busy or not
   if( mx25_read_status(spi_handle) & MX25_SR_WIP ) {
       return F_RES_NOTRDY;
   }

  while (len > 0) {
    page_offset = addr & (MX25_PAGE_SIZE - 1);
    chunk = MX25_PAGE_SIZE - page_offset;

    if (chunk > len) {
      chunk = len;
    }

    mx25_write_enable(spi_handle);

    // Check Write Enable Latch is set
    if( (mx25_read_status(spi_handle) & MX25_SR_WEL) != MX25_SR_WEL ) {
        return F_RES_WPROTECT_ERROR;
    }


    tx_cmd[1] = (uint8_t)((addr >> 16) & 0xFF);
    tx_cmd[2] = (uint8_t)((addr >> 8) & 0xFF);
    tx_cmd[3] = (uint8_t)((addr) & 0xFF);

    mx25_select(spi_handle);
    mx25_spi_tx(spi_handle, tx_cmd, sizeof(tx_cmd));
    mx25_spi_tx(spi_handle, buf, chunk);
    mx25_deselect(spi_handle);

    f_res = mx25_wait_ready(spi_handle, 10);
    MX25_VERIFY_SUCCESS_OR_RETURN(f_res);

    //Read Program Fail Flag bit.
    if ( (mx25_read_security(spi_handle) & MX25_FSR_P_FAIL) ==  MX25_FSR_P_FAIL)
    {
      return F_RES_WRITE_ERROR;
    }


    addr += chunk;
    buf  += chunk;
    len  -= chunk;
  }

  return f_res;
}

/* ============================================================================
 * Erase
 * ========================================================================== */

#if defined(TESTTESETST)
sl_status_t mx25_erase_block64(spi_master_t *spi_handle, uint32_t addr)
{
  fresult_t f_res;

  uint8_t cmd[4] = {
     MX25_CMD_BE64,
     (uint8_t)(addr >> 16),
     (uint8_t)(addr >> 8),
     (uint8_t)(addr)
   };


  addr &= ~(MX25_BLOCK64_SIZE - 1);

  mx25_write_enable();

  mx25_select(spi_handle);
  mx25_spi_tx(spi_handle, cmd, sizeof(cmd));
  mx25_deselect(spi_handle);

  sl_status_t st = mx25_wait_ready(spi_handle, 100);
  return (st == SL_STATUS_OK)
         ? SL_STATUS_OK
         : SL_STATUS_FLASH_ERASE_FAILED;
}


/* ============================================================================
 * Power management
 * ========================================================================== */

sl_status_t mx25_power_down(spi_master_t *spi_handle)
{
  uint8_t cmd = MX25_CMD_DP;

  mx25_select(spi_handle);
  mx25_spi_tx(spi_handle, &cmd, 1);
  mx25_deselect(spi_handle);

  return SL_STATUS_OK;
}
#endif
/*
 * Function:       MX25_CMD_RES  Release the chip from powerdown mode
 * Arguments:      Pointer to spi_handle.
 * Description:    The RES instruction is to read the Device
 *                 electric identification of 1-byte.
 * Return Message: fresult_t F_RES_OK / F_RES_READ_ERROR
 */
fresult_t mx25_wake_up(spi_master_t *spi_handle)
{
  fresult_t  f_res = F_RES_OK;
  uint8_t tx_cmd[4] = { MX25_CMD_RES, 0, 0, 0 };
  uint8_t rx[4] = { 0 };


  mx25_select(spi_handle);
  f_res = mx25_spi_trx(spi_handle, tx_cmd, sizeof(tx_cmd), rx, 1);
  mx25_deselect(spi_handle);
  Delay_30us(); // tRES1/tRES2 = 30 µs max

  MX25_VERIFY_SUCCESS_OR_RETURN(f_res);

  // Get electric identification
  if (rx[0] !=  ElectronicID) {
    f_res = F_RES_READ_ERROR;
  }

  return f_res;
}

