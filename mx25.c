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

#define mx25_selectWP(handle)
#define mx25_deselectWP(handle)

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
/* TEST WORKAROUND: Buffer for read-back verification (added by UP) */
/* WARNING: This verification doubles operation time and wears flash */
uint8_t test_data_buf[MX25_PAGE_SIZE]; /* @ToDo This line of code for test  It was been added by UP*/



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
  fresult_t fl_res = F_RES_OK;
  uint8_t cmd = MX25_CMD_WREN;
  uint8_t status_reg  = 0xFF;


  mx25_select(spi_handle);
  fl_res = mx25_spi_tx(spi_handle, &cmd, 1);
  mx25_deselect(spi_handle);
  MX25_VERIFY_SUCCESS_OR_RETURN(fl_res);

  // Wait tWR (5 us) minimum before checking status
  Delay_10us(); // 10 us for safety /* @ToDo The time must been measured between  set CS# - > clear CS#  by oscilloscope.  It was been added by UP*/

  cmd = MX25_CMD_RDSR;
  mx25_select(spi_handle);
  fl_res = mx25_spi_trx(spi_handle, &cmd, 1, &status_reg, 1);
  mx25_deselect(spi_handle);
  MX25_VERIFY_SUCCESS_OR_RETURN(fl_res);
  
  // Verify WEL bit is set (bit 1)
  if ((status_reg& MX25_SR_WEL) != MX25_SR_WEL) {
    return F_RES_WRITE_INHIBITED;
  }

  return fl_res;

}

static uint8_t mx25_read_status(spi_master_t *spi_handle)
{
  uint8_t cmd = MX25_CMD_RDSR;
  uint8_t status_reg  = 0;
  fresult_t fl_res = F_RES_OK;

  mx25_select(spi_handle);
  fl_res = mx25_spi_trx(spi_handle, &cmd, 1, &status_reg, 1);
  mx25_deselect(spi_handle);

  if (fl_res != F_RES_OK)  {
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
  fresult_t fl_res = F_RES_OK;
  uint8_t cmd = MX25_CMD_RDSCUR;
  uint8_t status_reg  = 0;

  mx25_select(spi_handle);
  fl_res = mx25_spi_trx(spi_handle, &cmd, 1, &status_reg, 1);
  mx25_deselect(spi_handle);

  if (fl_res != F_RES_OK)  {
    app_assert_status(SL_STATUS_FAIL);
  }

  return status_reg;
}

#ifdef MX25_WAIT_DO_STATUS_WHILE_READY
static fresult_t mx25_wait_ready(spi_master_t *spi_handle, uint16_t op_timeout)
{
  /* quick check: if not busy now us return immediately */
  if ((mx25_read_status(spi_handle) & MX25_SR_WIP) == 0) {
    return F_RES_OK;
  }

  /* op_timeout is in ticks of MX_25_PERIODIC_TIMER_DELAY_MS (10 ms by current code) */
  mx25_timeout_timer = op_timeout;

  /* Now device was busy us poll until WIP clears or timeout */
  do {
    if (mx25_timeout_timer == 0) {
      return F_RES_TIMEOUT_ERROR;
    }
    MX25_DELAY_HOOK(); /* typically small sleep (implementation uses sl_sleeptimer or udelay) */
    /* loop condition will re-read WIP after delay */
  } while (mx25_read_status(spi_handle) & MX25_SR_WIP);

  return F_RES_OK;
}
#else
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
#endif

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
  fresult_t fl_res = F_RES_OK;
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
  fl_res = mx25_wake_up(spi_handle);
  MX25_VERIFY_SUCCESS_OR_RETURN(fl_res);

  return fl_res;
}

fresult_t mx25_detect_flash(spi_master_t *spi_handle)
{
  fresult_t fl_res = F_RES_OK;
  uint8_t tx_cmd[4] = { MX25_CMD_RDID, MX25_DUMMY_BYTE, MX25_DUMMY_BYTE, MX25_DUMMY_BYTE };
  uint8_t rx[4] = { 0 };

  mx25_select(spi_handle);
  fl_res = mx25_spi_trx(spi_handle, tx_cmd, 1, rx, (sizeof(tx_cmd) - 1) );
  mx25_deselect(spi_handle);
  MX25_VERIFY_SUCCESS_OR_RETURN(fl_res);

  Delay_30us();// tRES1/tRES2 = 30 us max

  mx25_info.manufacturer_id = rx[0];
  mx25_info.memory_type     = rx[1];
  mx25_info.capacity_id     = rx[2];

  if (  (mx25_info.manufacturer_id != MX25_MANUFACTURER_ID)
     || (mx25_info.memory_type != ((MX25_DEV_ID_MX25R8035F & 0xff00) >> 8) ) 
     || (mx25_info.capacity_id !=  (MX25_DEV_ID_MX25R8035F & 0x00ff) ) ) {
    return F_RES_INVALID_ID;
  }

  tx_cmd[0] = MX25_CMD_REMS;
  tx_cmd[1] = MX25_DUMMY_BYTE;
  tx_cmd[2] = MX25_DUMMY_BYTE;
  tx_cmd[3] = 0x1;

  mx25_select(spi_handle);
  fl_res = mx25_spi_trx(spi_handle, tx_cmd, sizeof(tx_cmd), rx, 2 );
  mx25_deselect(spi_handle);
  MX25_VERIFY_SUCCESS_OR_RETURN(fl_res);

  mx25_info.device_id = (uint16_t)((uint16_t)rx[0] << 8) | (uint16_t)rx[1];

  if (   (mx25_info.manufacturer_id != MX25_MANUFACTURER_ID)
      || (mx25_info.device_id != RESID1) ) {
    return F_RES_INVALID_ID;
  }

  mx25_info.size_bytes = mem_density_to_size(mx25_info.capacity_id);
  mx25_info.detected   = true;

  return F_RES_OK;
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
  fresult_t fl_res = F_RES_OK;

  // 1. Wait for device to be ready (WIP = 0)
  fl_res = mx25_wait_ready(spi_handle, 100);
  MX25_VERIFY_SUCCESS_OR_RETURN(fl_res);

  // 2. Read current Status Register and Check if already unlocked (BP[3:0] = 0000)
  if( (mx25_read_status(spi_handle) & (MX25_BP_LEVEL_FULL)) != 0 ) { // Bits 5-2 are BP[3:0]
      fl_res = F_RES_WPROTECT_ERROR;
  }

  return fl_res;
}
/* ============================================================================
 * Erase
 * ========================================================================== */
/*
 * Function:        mx25_erase_chip
 * Arguments:       spi_master_t *spi_handle - SPI controller instance for communication
 * Description:     Performs a full chip erase of the MX25R8035F flash memory.
 *                  This low-level function sends the Chip Erase (CE) command (0x60 or 0xC7)
 *                  after enabling write operations. It waits for the extended erase
 *                  operation to complete (35 seconds typical, 120 seconds maximum)
 *                  and checks for erase failure flags. The function does not verify
 *                  that all Block Protect bits are cleared, which is a prerequisite
 *                  for successful chip erase.
 *
 * Return Message: fresult_t
 *                  - F_RES_OK: Chip erase completed without errors
 *                  - F_RES_NOTRDY: Flash device is busy (WIP=1)
 *                  - F_RES_ERASE_ERROR: Erase Fail flag set in Security Register
 *                  - Other SPI communication or write enable errors
 *
 * Note: Current timeout of 16000ms (16 seconds) is insufficient for maximum
 *       erase time of 120 seconds. Should be increased to at least 130000ms.
 *       Block Protect bits (BP3-BP0) must all be 0 for Chip Erase to execute.
 */
fresult_t mx25_erase_chip(spi_master_t *spi_handle)
{
  uint8_t cmd = MX25_CMD_CE;/* Chip Erase command (0x60 or 0xC7 - both are equivalent) */
  fresult_t fl_res = F_RES_OK;

  /* Check if flash is currently busy with another operation (WIP=1) */
  if( mx25_read_status(spi_handle) & MX25_SR_WIP ) {
      return F_RES_NOTRDY;
  }

  /* Enable write operations by setting Write Enable Latch (WEL=1) */
    /* WEL must be set before any erase/program operation */
  fl_res = mx25_write_enable(spi_handle);
  MX25_VERIFY_SUCCESS_OR_RETURN(fl_res);
  /* Send Chip Erase command (single byte, no address required) */
  mx25_select(spi_handle);
  fl_res = mx25_spi_tx(spi_handle, &cmd, 1);
  mx25_deselect(spi_handle);
  MX25_VERIFY_SUCCESS_OR_RETURN(fl_res);

  /* Wait for Chip Erase operation to complete */
  /* CRITICAL: Timeout of 16000ms is too short - typical tCE is 35s, max 120s */
  /* Recommended timeout: 130000ms (130 seconds) to accommodate worst case */
  fl_res = mx25_wait_ready(spi_handle, 16000);
  MX25_VERIFY_SUCCESS_OR_RETURN(fl_res);

  /* Check Erase Fail Flag in Security Register for operation errors */
  /* P_FAIL/E_FAIL flags are sticky and persist until next successful operation */
  if ( (mx25_read_security(spi_handle) & MX25_FSR_E_FAIL) ==  MX25_FSR_E_FAIL) {
    return F_RES_ERASE_ERROR;
  }

  return fl_res;
}

/*
 * Function:        mx25_erase_sector
 * Arguments:       spi_master_t *spi_handle - SPI controller instance
 *                  uint32_t addr - Sector-aligned address (lower 12 bits must be 0)
 * Description:     Performs 4KB sector erase operation on MX25R8035F flash memory.
 *                  Executes Sector Erase (SE) command sequence: Write Enable ->
 *                  SE command (0x20) with 24-bit address -> wait for completion.
 *                  Includes Erase Fail Flag check for error detection. The
 *                  function handles alignment and validates device state.
 *
 * Return Message: fresult_t
 *                  - F_RES_OK: Sector successfully erased
 *                  - F_RES_NOTRDY: Flash device is busy (WIP=1)
 *                  - F_RES_ERASE_ERROR: Erase Fail flag set in Security Register
 *                  - Other SPI communication errors
 *
 * Note: Sector Erase time (tSE) is 35-400ms typical, timeout 100ms is minimal.
 *       For reliable operation, consider increasing timeout to 500ms.
 */
fresult_t mx25_erase_sector(spi_master_t *spi_handle, uint32_t addr)
{

  fresult_t fl_res = F_RES_OK;
  uint8_t tx_cmd[4] = { MX25_CMD_SE, MX25_DUMMY_BYTE, MX25_DUMMY_BYTE, MX25_DUMMY_BYTE};

  /* Pack 24-bit address into command buffer (big-endian order) */
  addr &= ~(MX25_SECTOR_SIZE - 1);

  /* Pack 24-bit address into command buffer (big-endian order) */
  tx_cmd[1] = (uint8_t)(addr >> 16);/* Address bits 23-16 */
  tx_cmd[2] = (uint8_t)(addr >> 8); /* Address bits 15-8 */
  tx_cmd[3] = (uint8_t)(addr);      /* Address bits 7-0 */


  /* Check if flash is currently busy with another operation (WIP=1) */
  if( mx25_read_status(spi_handle) & MX25_SR_WIP ) {
      return F_RES_NOTRDY;
  }

  /* Enable write operation by setting Write Enable Latch (WEL=1) */
  fl_res = mx25_write_enable(spi_handle);
  MX25_VERIFY_SUCCESS_OR_RETURN(fl_res);

  /* Execute Sector Erase command with address */
  mx25_select(spi_handle);
  fl_res = mx25_spi_tx(spi_handle, tx_cmd, sizeof(tx_cmd));
  mx25_deselect(spi_handle);
  MX25_VERIFY_SUCCESS_OR_RETURN(fl_res);

  /* Wait for Sector Erase operation to complete (tSE max = 400ms) */
  /* WARNING: 100ms timeout may be insufficient for worst-case tSE (400ms) */
  fl_res = mx25_wait_ready(spi_handle, 100);
  MX25_VERIFY_SUCCESS_OR_RETURN(fl_res);

  /* Check Erase Fail Flag in Security Register for error detection */
  if ( (mx25_read_security(spi_handle) & MX25_FSR_E_FAIL) ==  MX25_FSR_E_FAIL) {
    return F_RES_ERASE_ERROR;
  }

  return fl_res; /* Sector erase completed successfully */
}
/*
 * Function:        mx25_erase_block64
 * Arguments:       spi_master_t *spi_handle - SPI controller instance
 *                  uint32_t addr - 64KB-aligned address (lower 16 bits must be 0)
 * Description:     Performs a 64KB block erase operation on MX25R8035F flash memory.
 *                  Sends the Block Erase (BE) command (0xD8) with 24-bit address,
 *                  after setting Write Enable Latch. Waits for operation completion
 *                  and checks the Erase Fail Flag for errors. This is a low-level
 *                  driver function that directly communicates with the flash chip.
 *
 * Return Message: fresult_t
 *                  - F_RES_OK: Block erased successfully
 *                  - F_RES_ERASE_ERROR: Erase Fail flag set in Security Register
 *                  - Other SPI communication or timeout errors
 *
 * Timing: tBE (64KB Block Erase) = 0.43-2.1s (with high voltage) or 0.48-3.0s max
 *         Timeout of 1000ms may be insufficient for worst-case scenarios.
 */
fresult_t mx25_erase_block64(spi_master_t *spi_handle, uint32_t addr)
{
  fresult_t fl_res = F_RES_OK;
  uint8_t tx_cmd[4] = { MX25_CMD_BE64, MX25_DUMMY_BYTE, MX25_DUMMY_BYTE, MX25_DUMMY_BYTE}; /* Block Erase command buffer: [CMD, ADDR_MSB, ADDR_MID, ADDR_LSB] */

  /* Ensure address is aligned to 64KB boundary (clear lower 16 bits) */
  addr &= ~(MX25_BLOCK64_SIZE - 1);

  /* Prepare 24-bit address in command buffer (big-endian) */
  tx_cmd[1] = (uint8_t)(addr >> 16); /* Address bits 23-16 */
  tx_cmd[2] = (uint8_t)(addr >> 8);   /* Address bits 15-8 */
  tx_cmd[3] = (uint8_t)(addr);        /* Address bits 7-0 */

  /* Enable write operation by setting Write Enable Latch (WEL=1) */
  fl_res = mx25_write_enable(spi_handle);
  MX25_VERIFY_SUCCESS_OR_RETURN(fl_res);

  /* Send Block Erase command with address */
  mx25_select(spi_handle);
  fl_res = mx25_spi_tx(spi_handle, tx_cmd, sizeof(tx_cmd));
  mx25_deselect(spi_handle);
  MX25_VERIFY_SUCCESS_OR_RETURN(fl_res);

  /* Wait for Block Erase operation to complete */
   /* Timeout 1000ms may be insufficient: tBE max = 3.0s (High Performance Mode) */
  fl_res = mx25_wait_ready(spi_handle, 1000);
  MX25_VERIFY_SUCCESS_OR_RETURN(fl_res);

  /* Check Erase Fail Flag in Security Register for errors */
  if ( (mx25_read_security(spi_handle) & MX25_FSR_E_FAIL) ==  MX25_FSR_E_FAIL) {
    return F_RES_ERASE_ERROR;
  }
  
  return fl_res; /* Block erase completed successfully */
}

/* ============================================================================
 * Read / Write
 * ========================================================================== */
/*
 * Function:        mx25_read
 * Arguments:       spi_master_t *spi_handle - SPI controller instance for communication
 *                  uint32_t addr - 24-bit starting address in flash memory
 *                  uint8_t *buf - Pointer to data buffer for storing read bytes
 *                  uint32_t len - Number of bytes to read (1 to flash capacity)
 * Description:     Low-level function to read data from MX25R8035F flash memory.
 *                  Implements the standard READ (03h) command sequence: sends 4-byte
 *                  command (opcode + 24-bit address) then receives sequential data bytes.
 *                  The flash automatically increments the internal address counter,
 *                  allowing continuous read of multiple bytes. Chip Select (CS#) is held
 *                  low throughout the entire transaction to maintain command context.
 *
 * Return Message:  fresult_t
 *                  - F_RES_OK: Read operation completed successfully
 *                  - F_RES_ERROR: SPI transmission or reception failed
 *                  - Other SPI communication error codes
 *
 * Note: This function uses the basic READ command (03h) with up to 50MHz clock in
 *       High Performance Mode. For faster reads, consider using FAST_READ (0Bh) with
 *       dummy cycles or Quad I/O commands when Quad Mode is enabled.
 */
fresult_t mx25_read(spi_master_t *spi_handle, uint32_t addr, uint8_t *buf, uint32_t len)
{
  fresult_t fl_res1 = F_RES_OK;/* Result of command transmission */
  fresult_t fl_res2 = F_RES_OK;/* Result of data reception */

  /* 4-byte command packet: opcode + 24-bit big-endian address */
  uint8_t tx_cmd[4] = {
    MX25_CMD_READ,          /* READ command opcode (03h) */
    (uint8_t)(addr >> 16),  /* Address bits [23:16] - MSB first */
    (uint8_t)(addr >> 8),   /* Address bits [15:8] */
    (uint8_t)(addr)         /* Address bits [7:0] - LSB last */
  };

  /* Assert Chip Select to begin SPI transaction */
  mx25_select(spi_handle);
  fl_res1 = mx25_spi_tx(spi_handle, tx_cmd, sizeof(tx_cmd));/* Phase 1: Send READ command with 24-bit address (4 bytes total) */
  fl_res2 = mx25_spi_rx(spi_handle, buf, len);              /* Phase 2: Receive requested data bytes (flash auto-increments address) */
  mx25_deselect(spi_handle);                                /* Deassert Chip Select to end transaction */

  /* Check both transmission and reception results */
  MX25_VERIFY_SUCCESS_OR_RETURN(fl_res1);
  MX25_VERIFY_SUCCESS_OR_RETURN(fl_res2);   

  return F_RES_OK;
}

/*
 * Function:        mx25_page_write
 * Arguments:       spi_master_t *spi_handle - SPI controller instance
 *                  uint32_t addr - Starting address for write operation
 *                  const uint8_t *buf - Pointer to source data buffer
 *                  uint32_t len - Number of bytes to write
 * Description:     Performs page programming of MX25R8035F flash memory. Handles
 *                  writing data across page boundaries by splitting into chunks.
 *                  Each chunk is written with proper Page Program (PP) command
 *                  sequence: Write Enable -> PP command + address -> data -> wait.
 *                  Includes post-write verification read to detect programming
 *                  errors and data corruption. This is a low-level driver function.
 *
 * Return Message: fresult_t
 *                  - F_RES_OK: All data written and verified successfully
 *                  - F_RES_NOTRDY: Flash device is busy (WIP=1)
 *                  - F_RES_WRITE_ERROR: Program Fail flag set or data mismatch
 *                  - F_RES_READ_ERROR: Verification read failed
 *                  - Other SPI communication errors
 *
 * Note: Contains workaround verification read that impacts performance.
 *       The tPP (Page Program Time) is 0.6-3ms typical, timeout 300ms is generous.
 */
fresult_t mx25_page_write(spi_master_t *spi_handle, uint32_t addr, const uint8_t *buf, uint32_t len)
{
   fresult_t fl_res = F_RES_OK;
   uint8_t tx_cmd[4] = { MX25_CMD_PP, MX25_DUMMY_BYTE, MX25_DUMMY_BYTE, MX25_DUMMY_BYTE };/* Page Program command buffer: [CMD, ADDR_MSB, ADDR_MID, ADDR_LSB] */
   uint32_t page_offset = 0;
   uint32_t chunk = 0;


   /* Check if flash is currently busy with another operation (WIP=1) */
   if( mx25_read_status(spi_handle) & MX25_SR_WIP ) {
       return F_RES_NOTRDY;
   }

  /* Loop until all requested bytes are written */
  while (len > 0) {
      /* Calculate offset within current 256-byte page boundary */
    page_offset = addr & (MX25_PAGE_SIZE - 1);

    /* Determine maximum bytes we can write without crossing page boundary */
    chunk = MX25_PAGE_SIZE - page_offset;

    /* Adjust chunk size if remaining data is less than page space */
    if (chunk > len) {
      chunk = len;
    }

    /* Enable write operation by setting Write Enable Latch (WEL=1) */
    fl_res = mx25_write_enable(spi_handle);
    MX25_VERIFY_SUCCESS_OR_RETURN(fl_res);

    /* Prepare Page Program command with 24-bit address */
    tx_cmd[0] = MX25_CMD_PP;                      /* Page Program command (0x02) */
    tx_cmd[1] = (uint8_t)((addr >> 16) & 0xFF);   /* Address bits 23-16 */
    tx_cmd[2] = (uint8_t)((addr >> 8) & 0xFF);    /* Address bits 15-8 */
    tx_cmd[3] = (uint8_t)((addr) & 0xFF);         /* Address bits 7-0 */

    /* Execute SPI transaction: command + address + data */
    mx25_select(spi_handle);
    fl_res = mx25_spi_tx(spi_handle, tx_cmd, sizeof(tx_cmd)); /* Send PP command and address */
    fl_res = mx25_spi_tx(spi_handle, buf, chunk);             /* Send actual data bytes */
    mx25_deselect(spi_handle);
    MX25_VERIFY_SUCCESS_OR_RETURN(fl_res);


    /* Wait for Page Program operation to complete (tPP max = 3ms) */
    /* Timeout 300ms is extremely conservative for safety */
    fl_res = mx25_wait_ready(spi_handle, 3);
    MX25_VERIFY_SUCCESS_OR_RETURN(fl_res);

    /* Check Program Fail Flag in Security Register for errors */
    if ( (mx25_read_security(spi_handle) & MX25_FSR_P_FAIL) ==  MX25_FSR_P_FAIL) {
      return F_RES_WRITE_ERROR;
    }

    /* @ToDo Workaround to prevent reading corrupted data  It was been added by UP*/
    /* TEST WORKAROUND: Read back written data for verification (added by UP) */
    /* This compensates for potential data corruption but adds significant overhead */
    if (mx25_read(spi_handle, addr, test_data_buf, chunk) != F_RES_OK) { /* @ToDo This line for testing purposes It was been added by UP*/
       return F_RES_READ_ERROR;
    }
    /* Compare written data with original buffer for integrity check */
    if (0 != memcmp(buf, test_data_buf, chunk )) {/* @ToDo This line for testing purposes It was been added by UP*/
      return F_RES_WRITE_ERROR;
    }

    /* Advance pointers and counters for next chunk */
    addr += chunk; /* Move to next address */
    buf  += chunk; /* Advance source buffer pointer */
    len  -= chunk; /* Decrement remaining byte count */
  }

  return fl_res; /* All data written and verified successfully */
}


/* ============================================================================
 * Power management
 * ========================================================================== */
/*
 * Function:        mx25_power_down
 * Arguments:       spi_master_t *spi_handle - SPI controller instance for communication
 * Description:     Places the MX25R8035F flash memory into Deep Power-down mode.
 *                  This function verifies the device is ready (not busy), sends the
 *                  Deep Power-down command, and ensures proper timing delays as per
 *                  datasheet requirements. In Deep Power-down mode, all commands
 *                  except Release from Power-down are ignored and current consumption
 *                  drops to ISB2 (typ. 0.35 uA).
 *
 * Return Message: fresult_t
 *                  - F_RES_OK: Deep Power-down command sent successfully
 *                  - F_RES_NOTRDY: Device is busy (WIP=1), cannot enter power-down
 *                  - Other error codes from SPI communication functions
 *
 * Note: After this function returns, wait at least tDPDD (30 us) before attempting
 *       to wake the device with CS# toggling or Release from Power-down command.
 */
fresult_t mx25_power_down(spi_master_t *spi_handle)
{
  fresult_t fl_res = F_RES_OK;
  uint8_t cmd = MX25_CMD_DP; /* Deep Power-down command (0xB9) */


  // Check flash is busy or not
  if( mx25_read_status(spi_handle) & MX25_SR_WIP ) {
      return F_RES_NOTRDY;
  }

  /* --- Step 2: Send Deep Power-down command --- */
  /* Sequence: CS# low -> Send 0xB9 -> CS# high (must be at byte boundary) */
  mx25_select(spi_handle);
  fl_res = mx25_spi_tx(spi_handle, &cmd, 1);
  mx25_deselect(spi_handle);
  MX25_VERIFY_SUCCESS_OR_RETURN(fl_res);

  /* --- Step 3: Wait for tDP (max 10 us) for mode transition --- */
  Delay_10us();
  /* --- Step 4: Wait for tDPDD (max 30 us) for Deep Power-down staying --- */
  Delay_30us();

  /* In Deep Power-down mode, all commands except Release are ignored */
  /* You could add a verification by trying to read status and expecting no response */
  /* but this is optional as per datasheet */

  return fl_res;
}

/*
 * Function:        mx25_wake_up
 * Arguments:       spi_master_t *spi_handle - SPI controller instance
 * Description:     Wakes the MX25R8035F from power-down or deep power-down mode
 *                  using the Read Identification (RDID) command. This command
 *                  serves dual purpose: exiting power-down modes and reading
 *                  device identification. The function verifies the received
 *                  ID bytes to confirm successful wake-up and proper device
 *                  response. Requires tRES1/tRES2 delay (30 µs max) after
 *                  chip selection.
 *
 *                  Note: RDID command (0x9F) works for both Power-down and
 *                  Deep Power-down modes. Alternative: command 0xAB with
 *                  3 dummy bytes.
 *
 * Return Message: fresult_t
 *                  - F_RES_OK: Device successfully woken up and ID verified
 *                  - F_RES_READ_ERROR: Received ID does not match expected values
 *                  - Other SPI communication errors
 *
 * Note: Timing parameters from datasheet:
 *       - tRES1 (Deep Power-down recovery): 30 µs max
 *       - tRES2 (Power-down recovery): 30 µs max
 *       - tDP (Enter Deep Power-down): 10 µs typical
 */
fresult_t mx25_wake_up(spi_master_t *spi_handle)
{
  fresult_t   fl_res = F_RES_OK;
  uint8_t tx_cmd[4] = { MX25_CMD_RDID, MX25_DUMMY_BYTE, MX25_DUMMY_BYTE, MX25_DUMMY_BYTE};/* RDID command sequence: command + 3 dummy bytes (total 4 bytes to transmit) */
  uint8_t     rx[4] = { 0 };/* Receive buffer: [0]=Manufacturer ID, [1]=Memory Type, [2]=Capacity */

  /* Execute RDID command: wake-up sequence with ID readback */
  /* Transmit 4 bytes (CMD + 3 dummy), receive 3 ID bytes (rx[0], rx[1], rx[2]) */
  mx25_select(spi_handle);
  fl_res = mx25_spi_trx(spi_handle, tx_cmd, sizeof(tx_cmd), rx, (sizeof(rx) - 1));
  mx25_deselect(spi_handle);
  MX25_VERIFY_SUCCESS_OR_RETURN(fl_res);

  /* Mandatory recovery delay after power-down mode exit (datasheet requirement) */
  /* tRES1 (Deep Power-down) / tRES2 (Power-down): 30 µs maximum */
  Delay_30us(); // Wait required recovery time tRES1/tRES2 = 30 us max

  /* Verify device identification to confirm successful wake-up */
  /* Expected: Manufacturer ID = C2h (Macronix), Memory Type = 28h, Capacity = 14h */
  if (   (rx[0] !=  MX25_MANUFACTURER_ID)
      || (rx[1] !=  ((uint8_t)((MX25_DEV_ID_MX25R8035F & 0xFF00) >>8)))
      || (rx[2] !=  ((uint8_t)( MX25_DEV_ID_MX25R8035F & 0xFF       ))) ) {
    /* ID mismatch: device not responding correctly or wrong chip */
    fl_res = F_RES_READ_ERROR;
  }

  return fl_res;
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
  fresult_t  fl_res = F_RES_OK;
  uint8_t cmd = MX25_CMD_RSTEN;

  // Check if flash is currently busy with an operation
  // Aborting reset during programming/erasing prevents data corruption
  if( mx25_read_status(spi_handle) & MX25_SR_WIP ) {
    return F_RES_NOTRDY;
  }

  // 1. Send Reset Enable command (0x66) - required before RST command
  mx25_select(spi_handle);
  fl_res = mx25_spi_tx(spi_handle, &cmd, 1);
  mx25_deselect(spi_handle);
  MX25_VERIFY_SUCCESS_OR_RETURN(fl_res);

  // 2. Brief delay between commands (CS# must deassert properly)
  Delay_6us();  /* @ToDo The time must been measured between  set CS# - > clear CS#  by oscilloscope.  It was been added by UP*/
  
  // 3. Send Reset command (0x99) - actual reset execution
  cmd = MX25_CMD_RST;
  mx25_select(spi_handle);
  fl_res = mx25_spi_tx(spi_handle, &cmd, 1);
  mx25_deselect(spi_handle);
  MX25_VERIFY_SUCCESS_OR_RETURN(fl_res);
  
  // 4. Wait for mandatory minimum recovery time tREADY2 (30 us)
  Delay_30us(); /* @ToDo The time must been measured between  set CS# - > clear CS#  by oscilloscope.  It was been added by UP*/
  
  // 5. Wait for device to become ready with timeout
  fl_res = mx25_wait_ready(spi_handle, 10);
  
  return fl_res;
 
}

/*
 * Function:        mx25_enable_hpf_mode
 * Arguments:       spi_master_t *spi_handle - SPI controller instance for communication
 * Description:     Enables High Performance Mode in the MX25R8035F flash memory device.
 *                  This function reads the current configuration registers, checks if
 *                  the HPF mode is already enabled, and if not, writes the Configuration
 *                  Register 2 with the High Performance bit set. It follows the proper
 *                  sequence: Write Enable -> Write Status/Configuration Registers ->
 *                  Recovery Delay -> Verification Read. The function includes safety
 *                  checks for device busy state and verifies the mode change.
 *
 * Return Message: fresult_t
 *                  - F_RES_OK: High Performance Mode enabled or already active
 *                  - F_RES_NOTRDY: Device is busy (WIP=1), operation aborted
 *                  - F_RES_WRITE_INHIBITED: Write Enable Latch remains set after operation
 *                  - F_RES_WRITE_ERROR: Mode verification failed
 *                  - Other error codes from SPI communication functions
 *
 * Note: The current implementation has protocol issues with RDCR command timing.
 *       Proper RDCR sequence requires sending 2 dummy bytes after command byte.
 */
fresult_t mx25_enable_hpf_mode(spi_master_t *spi_handle)
{

  fresult_t fl_res = F_RES_OK;
  uint8_t   tx_cmd[4] = {MX25_CMD_RDCR, MX25_DUMMY_BYTE, MX25_DUMMY_BYTE, MX25_DUMMY_BYTE}; /* Buffer for SPI transactions: RDCR requires command + 3 dummy bytes */
  uint8_t   rx_data[2] = { 0 };/* Receive buffer for configuration registers: [0] = CR1, [1] = CR2 */
  uint8_t   status_reg;
  uint8_t   config1_reg;
  uint8_t   config2_reg;



  /* Quick check: if device is busy with another operation, return immediately */
  status_reg = mx25_read_status(spi_handle);
  if((status_reg & MX25_SR_WIP) == MX25_SR_WIP ) {
    return F_RES_NOTRDY;/* Device is in Write/Erase/Program operation, cannot change configuration */
  }

  /* --- Step 1: Read current Configuration Registers --- */
  mx25_select(spi_handle);
  fl_res = mx25_spi_trx(spi_handle, tx_cmd, 1, rx_data, sizeof(rx_data));
  mx25_deselect(spi_handle);
  MX25_VERIFY_SUCCESS_OR_RETURN(fl_res);

  /* Extract Configuration Register values from received data */
  config1_reg = rx_data[0]; /* Dummy Cycle configuration bits */
  config2_reg = rx_data[1]; /* L/H Switch bit (bit 1) */

  /* Check if High Performance Mode is already enabled */
  if ((config2_reg & MX25_CR2_MODE_HIGH_PERF) == MX25_CR2_MODE_HIGH_PERF) {
     return F_RES_OK; /* Mode already active, no further action required */
  }

  /* --- Step 2: Prepare to write new configuration --- */
  /* WRSR command writes 3 bytes: Status Register, CR1, and CR2 */
   tx_cmd[0] = MX25_CMD_WRSR;
   tx_cmd[1] = status_reg;
   tx_cmd[2] = config1_reg;
   tx_cmd[3] = config2_reg | MX25_CR2_MODE_HIGH_PERF;

   /* Enable writing by setting Write Enable Latch (WEL=1) */
  fl_res = mx25_write_enable(spi_handle);
  MX25_VERIFY_SUCCESS_OR_RETURN(fl_res);

  /* --- Write SR + CR1 + CR2 together (WRSR command) --- */
  /* Write the three configuration registers (SR1, CR1, CR2) */
  mx25_select(spi_handle);
  fl_res = mx25_spi_tx(spi_handle, tx_cmd, sizeof(tx_cmd));
  mx25_deselect(spi_handle);
  MX25_VERIFY_SUCCESS_OR_RETURN(fl_res);

  // Wait for mandatory minimum recovery time tREADY2 (30 us)
  /* --- Step 3: Post-write timing requirements --- */
  /* Wait for mandatory minimum recovery time tREADY2 (30 us) */
  /* TODO: This delay must be measured between CS# rising and next CS# falling edge.*/
  /* Current implementation uses fixed delay; verify with oscilloscope. */
  Delay_30us(); /* @ToDo The time must been measured between  set CS# - > clear CS#  by oscilloscope.  It was been added by UP*/

  /* Wait for device to complete internal write operation (WIP=0) */
  /* Timeout of 10ms covers tW (max 20ms for WRSR in High Performance Mode) */
  fl_res = mx25_wait_ready(spi_handle, 10);
  MX25_VERIFY_SUCCESS_OR_RETURN(fl_res);

  /* --- Step 4: Post-operation status check --- */
  /* Verify that Write Enable Latch has cleared (should be 0 after operation) */
  status_reg = mx25_read_status(spi_handle);
  if((status_reg & MX25_SR_WEL) == MX25_SR_WEL ) {
    return F_RES_WRITE_INHIBITED;
  }

   /* --- Step 5: Verify the configuration was written correctly --- */
   /* Prepare RDCR command with dummy bytes (same protocol issue as above) */
   /* --- Read SR + CR1 + CR2 together (RDCR command) --- */
   tx_cmd[0] = MX25_CMD_RDCR;
   tx_cmd[1] = MX25_DUMMY_BYTE;
   tx_cmd[2] = MX25_DUMMY_BYTE;
   tx_cmd[3] = MX25_DUMMY_BYTE;

   /* Check if High Performance Mode bit is now set in CR2 */
  mx25_select(spi_handle);
  fl_res = mx25_spi_trx(spi_handle, tx_cmd, 1, rx_data, sizeof(rx_data));
  mx25_deselect(spi_handle);
  MX25_VERIFY_SUCCESS_OR_RETURN(fl_res);

  config2_reg = rx_data[1];
  if ((config2_reg & MX25_CR2_MODE_HIGH_PERF) != MX25_CR2_MODE_HIGH_PERF) {
     return F_RES_WRITE_ERROR;  /* Configuration was not written successfully */
  }

  /* High Performance Mode successfully enabled and verified */
  return fl_res;
}
