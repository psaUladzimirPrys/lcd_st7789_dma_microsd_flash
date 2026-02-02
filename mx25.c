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

#define MX_25_PERIODIC_TIMER_DELAY_MS 10
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

/***************************************************************************//**
* Perform a sequence of SPI Master writes immediately followed by a SPI Master read.
******************************************************************************/
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
/*
 * Function:        mx25_write_enable
 * Arguments:       spi_master_t *spi_handle - SPI controller instance
 * Description:     Sends Write Enable command (0x06) to set WEL bit.
 *                  Required before any write/erase operation.
 *                  Verifies that WEL bit is actually set.
 * 
 * Return Message: fresult_t 
 *                  - F_RES_OK: Write enabled successfully
 *                  - F_RES_WRITE_INHIBITED: WEL bit not set after command
 *                  - F_RES_PARAM_ERROR: /
 *                  - F_RES_WRITE_ERROR: /
 *                  - F_RES_TRANSMIT_ERROR: /: SPI communication error
 */
static fresult_t mx25_write_enable(spi_master_t *spi_handle)
{
  fresult_t f_res = F_RES_OK;
  uint8_t cmd = MX25_CMD_WREN;
  uint8_t status_reg  = 0xFF;


  mx25_select(spi_handle);
  f_res = mx25_spi_tx(spi_handle, &cmd, 1);
  mx25_deselect(spi_handle);
  MX25_VERIFY_SUCCESS_OR_RETURN(f_res);

  // Wait tWR (5 us) minimum before checking status
  //sl_udelay_wait(10);  // 10 us for safety /* @ToDo The time must been measured between  set CS# - > clear CS#  by oscilloscope.  It was been added by UP*/

  cmd = MX25_CMD_RDSR;
  mx25_select(spi_handle);
  f_res = mx25_spi_trx(spi_handle, &cmd, 1, &status_reg, 1);
  mx25_deselect(spi_handle);
  MX25_VERIFY_SUCCESS_OR_RETURN(f_res);
  
  // Verify WEL bit is set (bit 1)
  if ((status_reg& MX25_SR_WEL) != MX25_SR_WEL) {
    return F_RES_WRITE_INHIBITED;
  }

  return f_res;

}

static uint8_t mx25_read_status(spi_master_t *spi_handle)
{
  uint8_t cmd = MX25_CMD_RDSR;
  uint8_t status_reg  = 0;
  fresult_t f_res = F_RES_OK;

  mx25_select(spi_handle);
  f_res = mx25_spi_trx(spi_handle, &cmd, 1, &status_reg, 1);
  mx25_deselect(spi_handle);

  if (f_res != F_RES_OK)  {
    app_assert_status(SL_STATUS_FAIL);
  }

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
  fresult_t f_res = F_RES_OK;
  uint8_t cmd = MX25_CMD_RDSCUR;
  uint8_t status_reg  = 0;

  mx25_select(spi_handle);
  f_res = mx25_spi_trx(spi_handle, &cmd, 1, &status_reg, 1);
  mx25_deselect(spi_handle);

  if (f_res != F_RES_OK)  {
    app_assert_status(SL_STATUS_FAIL);
  }

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

fresult_t mx25_init(spi_master_t *spi_handle)
{
  fresult_t f_res = F_RES_OK;
  bool timer_is_running = false;

  /* Make sure the mx_25_timeout_timer_handle timer is initialized only once */
  sl_sleeptimer_is_timer_running(&mx_25_timeout_timer_handle,
                                 &timer_is_running);
  if (timer_is_running == false) {
  /* Start a periodic timer 10 ms to generate flash control timing */
    sl_sleeptimer_start_periodic_timer_ms(&mx_25_timeout_timer_handle,
                                          MX_25_PERIODIC_TIMER_DELAY_MS,
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
  fresult_t f_res = F_RES_OK;
  uint8_t tx_cmd[4] = { MX25_CMD_RDID, MX25_DUMMY, MX25_DUMMY, MX25_DUMMY };
  uint8_t rx[4] = { 0 };

  mx25_select(spi_handle);
  f_res = mx25_spi_trx(spi_handle, tx_cmd, 1, rx, (sizeof(tx_cmd) - 1) );
  mx25_deselect(spi_handle);

  if(f_res != F_RES_OK) {
    return SL_STATUS_FAIL;
  }

  Delay_30us();// tRES1/tRES2 = 30 µs max

  mx25_info.manufacturer_id = rx[0];
  mx25_info.memory_type     = rx[1];
  mx25_info.capacity_id     = rx[2];

  if (  (mx25_info.manufacturer_id != MX25_MANUFACTURER_ID)
     || (mx25_info.memory_type != ((MX25_DEV_ID_MX25R8035F & 0xff00) >> 8) ) 
     || (mx25_info.capacity_id !=  (MX25_DEV_ID_MX25R8035F & 0x00ff) ) ) {
    return SL_STATUS_NOT_SUPPORTED;
  }

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

/*
 * Function:        mx25_ready_to_write_erase
 * Arguments:       spi_master_t *spi_handle - SPI controller instance
 * Description:     Reads protection configuration from Status and Configuration
 *                  registers. Extracts BP[3:0] bits and TB bit for analysis.
 *
 * Return Message: fresult_t
 *                  - F_RES_OK: Successfully read protection status
 *                  - F_RES_WPROTECT_ERROR: SPI communication error
 */
fresult_t  mx25_ready_to_write_erase(spi_master_t *spi_handle)
{
  fresult_t f_res = F_RES_OK;

  // 1. Wait for device to be ready (WIP = 0)
  f_res = mx25_wait_ready(spi_handle, 100);
  MX25_VERIFY_SUCCESS_OR_RETURN(f_res);

  // 2. Read current Status Register and Check if already unlocked (BP[3:0] = 0000)
  if( (mx25_read_status(spi_handle) & (MX25_SR_BP0 |MX25_SR_BP1 | MX25_SR_BP2 | MX25_SR_BP3)) != 0 ) { // Bits 5-2 are BP[3:0]
      f_res = F_RES_WPROTECT_ERROR;
  }

  return f_res;
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

  // Check Write Enable Latch is set
  f_res = mx25_write_enable(spi_handle);
  MX25_VERIFY_SUCCESS_OR_RETURN(f_res);

  mx25_select(spi_handle);
  f_res = mx25_spi_tx(spi_handle, &cmd, 1);
  mx25_deselect(spi_handle);
  MX25_VERIFY_SUCCESS_OR_RETURN(f_res);

  f_res = mx25_wait_ready(spi_handle, 16000);
  MX25_VERIFY_SUCCESS_OR_RETURN(f_res);

  //Read Erase Fail Flag bit.
  if ( (mx25_read_security(spi_handle) & MX25_FSR_E_FAIL) ==  MX25_FSR_E_FAIL)
  {
    return F_RES_ERASE_ERROR;
  }

  return f_res;
}


fresult_t mx25_erase_sector(spi_master_t *spi_handle, uint32_t addr)
{

  fresult_t f_res = F_RES_OK;
  uint8_t tx_cmd[4] = { MX25_CMD_SE, MX25_DUMMY, MX25_DUMMY, MX25_DUMMY};

  /* Align to 4K sector */
  addr &= ~(MX25_SECTOR_SIZE - 1);

  tx_cmd[1] = (uint8_t)(addr >> 16);
  tx_cmd[2] = (uint8_t)(addr >> 8);
  tx_cmd[3] = (uint8_t)(addr);

  // Check flash is busy or not
  if( mx25_read_status(spi_handle) & MX25_SR_WIP ) {
      return F_RES_NOTRDY;
  }

  // Check Write Enable Latch is set
  f_res = mx25_write_enable(spi_handle);
  MX25_VERIFY_SUCCESS_OR_RETURN(f_res);

  mx25_select(spi_handle);
  f_res = mx25_spi_tx(spi_handle, tx_cmd, sizeof(tx_cmd));
  mx25_deselect(spi_handle);
  MX25_VERIFY_SUCCESS_OR_RETURN(f_res);

  f_res = mx25_wait_ready(spi_handle, 100);
  MX25_VERIFY_SUCCESS_OR_RETURN(f_res);

  //Read Erase Fail Flag bit.
  if ( (mx25_read_security(spi_handle) & MX25_FSR_E_FAIL) ==  MX25_FSR_E_FAIL)
  {
    return F_RES_ERASE_ERROR;
  }

  return f_res;
}

fresult_t mx25_erase_block64(spi_master_t *spi_handle, uint32_t addr)
{
  fresult_t f_res = F_RES_OK;
  uint8_t tx_cmd[4] = { MX25_CMD_BE64, MX25_DUMMY, MX25_DUMMY, MX25_DUMMY};


  addr &= ~(MX25_BLOCK64_SIZE - 1);
  tx_cmd[1] = (uint8_t)(addr >> 16);
  tx_cmd[2] = (uint8_t)(addr >> 8);
  tx_cmd[3] = (uint8_t)(addr);

  // Check Write Enable Latch is set
  f_res = mx25_write_enable(spi_handle);
  MX25_VERIFY_SUCCESS_OR_RETURN(f_res);

  mx25_select(spi_handle);
  f_res = mx25_spi_tx(spi_handle, tx_cmd, sizeof(tx_cmd));
  mx25_deselect(spi_handle);
  MX25_VERIFY_SUCCESS_OR_RETURN(f_res);

  f_res = mx25_wait_ready(spi_handle, 1000);
  MX25_VERIFY_SUCCESS_OR_RETURN(f_res);

  //Read Erase Fail Flag bit.
  if ( (mx25_read_security(spi_handle) & MX25_FSR_E_FAIL) ==  MX25_FSR_E_FAIL) {
    return F_RES_ERASE_ERROR;
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

   uint8_t tx_cmd[4] = { MX25_CMD_PP, MX25_DUMMY, MX25_DUMMY, MX25_DUMMY };


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

    // Check Write Enable Latch is set
    f_res = mx25_write_enable(spi_handle);
    MX25_VERIFY_SUCCESS_OR_RETURN(f_res);


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
    if ( (mx25_read_security(spi_handle) & MX25_FSR_P_FAIL) ==  MX25_FSR_P_FAIL) {
      return F_RES_WRITE_ERROR;
    }

    addr += chunk;
    buf  += chunk;
    len  -= chunk;
  }

  return f_res;
}


/* ============================================================================
 * Power management
 * ========================================================================== */

fresult_t mx25_power_down(spi_master_t *spi_handle)
{
  fresult_t f_res = F_RES_OK;
  uint8_t cmd = MX25_CMD_DP;

  mx25_select(spi_handle);
  f_res = mx25_spi_tx(spi_handle, &cmd, 1);
  mx25_deselect(spi_handle);
  MX25_VERIFY_SUCCESS_OR_RETURN(f_res);

  return f_res;
}

/*
 * Function:        mx25_release_powerdown
 * Arguments:       Pointer to spi_master_t *spi_handle - SPI controller instance
 * Description:     Releases the MX25R8035F from Power-down or Deep Power-down mode.
 *                  Sends the Release from Power-down command MX25_CMD_RES(0xAB) followed by
 *                  3 dummy bytes, then reads Manufacturer ID and Device ID to verify
 *                  successful wake-up. Waits the required tRES1/tRES2 time (30 us).
 *                  This function works for both Power-down and Deep Power-down modes.
 * 
 * Return Message: fresult_t 
 *                  - F_RES_OK: Successfully exited power-down mode, ID verified
 *                  - F_RES_READ_ERROR: Device ID verification failed
 */
fresult_t mx25_wake_up(spi_master_t *spi_handle)
{
  fresult_t   f_res = F_RES_OK;
  uint8_t tx_cmd[4] = { MX25_CMD_RDID, MX25_DUMMY, MX25_DUMMY, MX25_DUMMY};
  uint8_t     rx[4] = { 0 };


  mx25_select(spi_handle);
  f_res = mx25_spi_trx(spi_handle, tx_cmd, sizeof(tx_cmd), rx, (sizeof(rx) - 1));
  mx25_deselect(spi_handle);
  MX25_VERIFY_SUCCESS_OR_RETURN(f_res);

  Delay_30us(); // Wait required recovery time tRES1/tRES2 = 30 us max

  // Get electric identification
  if (   (rx[0] !=  MX25_MANUFACTURER_ID)
      || (rx[1] !=  ((uint8_t)((MX25_DEV_ID_MX25R8035F & 0xFF00) >>8)))
      || (rx[2] !=  ((uint8_t)( MX25_DEV_ID_MX25R8035F & 0xFF       ))) ) {
    f_res = F_RES_READ_ERROR;
  }

  return f_res;
}

/*
 * Function:        mx25_reset
 * Arguments:       spi_master_t *spi_handle - SPI controller instance for communication
 * Description:     Performs a software reset of the MX25R8035F flash memory device.
 *                  This function sends the required two-phase reset command sequence
 *                  (RSTEN followed by RST) with proper timing. It includes safety
 *                  checks to prevent reset during active operations that could cause
 *                  data corruption. The function waits for the device to become ready
 *                  after reset with appropriate timeout.
 * 
 * Return Message: fresult_t 
 *                  - F_RES_OK: Reset completed successfully
 *                  - F_RES_NOTRDY: Device is busy (WIP=1), reset aborted for safety
 *                  - F_RES_TIMEOUT: Device did not become ready within timeout period
 *                  - Other error codes from SPI communication functions
 */
fresult_t  mx25_reset(spi_master_t *spi_handle)
{
  fresult_t  f_res = F_RES_OK;
  uint8_t cmd = MX25_CMD_RSTEN;

  // Check if flash is currently busy with an operation
  // Aborting reset during programming/erasing prevents data corruption
  if( mx25_read_status(spi_handle) & MX25_SR_WIP ) {
    return F_RES_NOTRDY;
  }

  // 1. Send Reset Enable command (0x66) - required before RST command
  mx25_select(spi_handle);
  f_res = mx25_spi_tx(spi_handle, &cmd, 1);
  mx25_deselect(spi_handle);
  MX25_VERIFY_SUCCESS_OR_RETURN(f_res);

  // 2. Brief delay between commands (CS# must deassert properly)
  Delay_6us();  /* @ToDo The time must been measured between  set CS# - > clear CS#  by oscilloscope.  It was been added by UP*/
  
  // 3. Send Reset command (0x99) - actual reset execution
  cmd = MX25_CMD_RST;
  mx25_select(spi_handle);
  f_res = mx25_spi_tx(spi_handle, &cmd, 1);
  mx25_deselect(spi_handle);
  MX25_VERIFY_SUCCESS_OR_RETURN(f_res);
  
  // 4. Wait for mandatory minimum recovery time tREADY2 (30 us)
  Delay_30us(); /* @ToDo The time must been measured between  set CS# - > clear CS#  by oscilloscope.  It was been added by UP*/
  
  // 5. Wait for device to become ready with timeout
  f_res = mx25_wait_ready(spi_handle, 10);
  
  return f_res;
 
}
