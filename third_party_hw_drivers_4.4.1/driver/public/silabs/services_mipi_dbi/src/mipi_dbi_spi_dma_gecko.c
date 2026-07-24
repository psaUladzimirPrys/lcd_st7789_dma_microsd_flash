/***************************************************************************//**
 * @file mipi_dbi_spi_dma_gecko.c
 * @brief TFT Display MIPI_DBI Interface Driver source file.
 * @version 1.0.0
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided \'as-is\', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 *******************************************************************************
 *
 * EVALUATION QUALITY
 * This code has been minimally tested to ensure that it builds with the
 * specified dependency versions and is suitable as a demonstration for
 * evaluation purposes only.
 * This code will be maintained at the sole discretion of Silicon Labs.
 *
 ******************************************************************************/
// -----------------------------------------------------------------------------
//                       Includes
// -----------------------------------------------------------------------------
#include "em_cmu.h"
#include "em_gpio.h"
#include "sl_status.h"
#include "sl_sleeptimer.h"
#include "sl_component_catalog.h"
#include "drv_spi_master.h"
#include "mipi_dbi.h"
#include "mipi_dbi_spi.h"

// -----------------------------------------------------------------------------
//                       Local Variables
// -----------------------------------------------------------------------------
const struct adafruit_st7789_spi_gecko_config *spi_config = NULL;
static volatile mipi_dbi_transfer_complete_callback_t transfer_complete_callback = NULL;
static spi_master_t spi_master_obj;              // SPI master device

static sl_status_t command_read(const struct mipi_dbi_device *device,
                                uint8_t *cmds, size_t num_cmds,
                                uint8_t *response, size_t len);

static sl_status_t command_write(const struct mipi_dbi_device *device,
                                 uint8_t cmd,
                                 const uint8_t *data, size_t len);

static sl_status_t write_display( const struct mipi_dbi_device *device,
                                  const uint8_t *framebuf,
                                  struct mipi_dbi_display_buffer_descriptor *desc,
                                  enum mipi_dbi_display_pixel_format pixfmt,
                                  mipi_dbi_transfer_complete_callback_t callback);

static sl_status_t reset(const struct mipi_dbi_device *device,
                         uint32_t delay);

static void dma_transfer_complete_callback(struct SPIDRV_HandleData *handle,
                                           Ecode_t transferStatus,
                                           int itemsTransferred);

static const struct mipi_dbi_api mipi_dbi_api = {
  .command_read = command_read,
  .command_write = command_write,
  .write_display = write_display,
  .reset = reset
};

static struct mipi_dbi_spi_gecko_data mipi_dbi_spi_gecko_data;

SPIDRV_HandleData_t spidrv_handle_data;
SPIDRV_Handle_t spidrv_handle = &spidrv_handle_data;

/*****************************************************************************
 * Function : spi_select
 * Arguments: void
 * Description: Selects the SPI slave device by setting the chip select pin low
 * Return: void
 ****************************************************************************/
static void spi_select(void)
{
  spi_master_control_cs(&spi_master_obj, SPI_SLAVE_CHIP_SELECT_LOW); // Pull CS low to select slave
}

/*****************************************************************************
 * Function : spi_deselect
 * Arguments: void
 * Description: Deselects the SPI slave device by setting the chip select pin high
 * Return: void
 ****************************************************************************/
static void spi_deselect(void)
{
  spi_master_control_cs(&spi_master_obj, SPI_SLAVE_CHIP_SELECT_HIGH); // Pull CS high to deselect slave
}

/*****************************************************************************
 * Function : set_dc_mode
 * Arguments: const struct mipi_dbi_device *device - Pointer to MIPI DBI device,
 *            bool mode - true for data mode, false for command mode
 * Description: Sets the data/command mode pin for SPI communication
 * Return: void
 ****************************************************************************/
static void set_dc_mode(const struct mipi_dbi_device *device,
                        bool mode)
{
  struct mipi_dbi_spidrv_config *config =
    (struct mipi_dbi_spidrv_config *)device->config; // Get SPI config from device

  if (mode) {
    GPIO_PinOutSet(config->dc.port, config->dc.pin); // Set DC high for data mode
  } else {
    GPIO_PinOutClear(config->dc.port, config->dc.pin); // Set DC low for command mode
  }
}

/*****************************************************************************
 * Function : spi_write_b
 * Arguments: const void *buf - Pointer to data buffer,
 *            int count - Number of bytes to write
 * Description: Writes data to SPI bus in blocking mode
 * Return: sl_status_t
 *               SL_STATUS_OK - on success,
 *               SL_STATUS_IO - on failure
 ****************************************************************************/
static sl_status_t spi_write_b(const void *buf,
                               int count)
{
  spi_select(); // Select SPI device before transfer

  err_t retVal = spi_master_write(&spi_master_obj, (uint8_t *)buf, count); // Perform blocking write

  spi_deselect(); // Deselect SPI device after transfer

  return SPI_MASTER_SUCCESS == retVal ? SL_STATUS_OK : SL_STATUS_IO; // Convert error codes
}



/*****************************************************************************
 * Function : spi_read_b
 * Arguments: const void *buf - Pointer to data buffer,
 *            int count - Number of bytes to read
 * Description: Reads data from SPI bus in blocking mode
 * Return: sl_status_t
 *               SL_STATUS_OK - on success,
 *               SL_STATUS_IO - on failure
 ****************************************************************************/
static sl_status_t spi_read_b(const void *buf,
                               int count)
{
  spi_select(); // Select SPI device before transfer

  err_t retVal = spi_master_read(&spi_master_obj, (uint8_t *)buf, count); // Perform blocking read

  spi_deselect(); // Deselect SPI device after transfer

  return SPI_MASTER_SUCCESS == retVal ? SL_STATUS_OK : SL_STATUS_IO; // Convert error codes
}

/*****************************************************************************
 * Function : reset
 * Arguments: const struct mipi_dbi_device *device - Pointer to MIPI DBI device,
 *            uint32_t delay - Reset delay (unused)
 * Description: Resets the MIPI DBI device (not supported in this implementation)
 * Return: sl_status_t
 *               SL_STATUS_NOT_SUPPORTED - always
 ****************************************************************************/
static sl_status_t reset(const struct mipi_dbi_device *device,
                         uint32_t delay)
{
  (void) device; // Suppress unused parameter warning
  (void) delay; // Suppress unused parameter warning

  return SL_STATUS_NOT_SUPPORTED; // Reset operation not implemented
}

/*****************************************************************************
 * Function : command_read
 * Arguments: const struct mipi_dbi_device *device - Pointer to MIPI DBI device,
 *            uint8_t *cmds - Buffer with command bytes to send,
 *            size_t num_cmds - Number of command bytes,
 *            uint8_t *response - Buffer to store response data,
 *            size_t len - Number of response bytes to read
 * Description: Reads data from the display controller by sending commands and receiving response
 * Return: sl_status_t
 *               SL_STATUS_OK - on success,
 *               SL_STATUS_IO - on SPI communication failure
 ****************************************************************************/
static sl_status_t command_read(const struct mipi_dbi_device *device,
                                uint8_t *cmds, size_t num_cmds,
                                uint8_t *response, size_t len)
{
  sl_status_t status; // Status for SPI operations

  if (num_cmds > 0) {
    set_dc_mode(device, false); // Switch to command mode
    status = spi_write_b(cmds, num_cmds); // Send command bytes
    if (SL_STATUS_OK != status) {
      return status; // Error sending commands
    }
  }

  if (len > 0) {
    set_dc_mode(device, true); // Switch to data mode
    status = spi_read_b(response, len); // Read response bytes
    if (SL_STATUS_OK != status) {
      return status; // Error reading response
    }
  }

  return SL_STATUS_OK; // All operations succeeded
}

/*****************************************************************************
 * Function : command_write
 * Arguments: const struct mipi_dbi_device *device - Pointer to MIPI DBI device,
 *            uint8_t cmd - Command byte to send,
 *            const uint8_t *data - Buffer with data bytes,
 *            size_t len - Number of data bytes
 * Description: Writes a command with optional data to the display controller
 * Return: sl_status_t
 *               SL_STATUS_OK - on success,
 *               SL_STATUS_IO - on SPI communication failure
 ****************************************************************************/
static sl_status_t command_write(const struct mipi_dbi_device *device,
                                 uint8_t cmd,
                                 const uint8_t *data, size_t len)
{
  sl_status_t status; // Status for SPI operations

  set_dc_mode(device, false); // Switch to command mode
  status = spi_write_b(&cmd, 1); // Send command byte
  if (SL_STATUS_OK != status) {
    return status; // Error sending command
  }

  if (len) {
    set_dc_mode(device, true); // Switch to data mode
    status = spi_write_b(data, len); // Send data bytes
  }
  return status; // Return final status
}

/*****************************************************************************
 * Function : write_display
 * Arguments: const struct mipi_dbi_device *device - Pointer to MIPI DBI device,
 *            const uint8_t *framebuf - Pointer to framebuffer data,
 *            struct mipi_dbi_display_buffer_descriptor *desc - Display buffer descriptor,
 *            enum mipi_dbi_display_pixel_format pixfmt - Pixel format (unused),
 *            mipi_dbi_transfer_complete_callback_t callback - Completion callback (unused)
 * Description: Writes display data to the controller using SPI with blocking mode
 * Return: sl_status_t
 *               SL_STATUS_OK - on success,
 *               SL_STATUS_IO - on SPI communication failure
 ****************************************************************************/
static sl_status_t write_display( const struct mipi_dbi_device *device,
                                  const uint8_t *framebuf,
                                  struct mipi_dbi_display_buffer_descriptor *desc,
                                  enum mipi_dbi_display_pixel_format pixfmt,
                                  mipi_dbi_transfer_complete_callback_t callback )
{
  sl_status_t status;
  (void)pixfmt;

  set_dc_mode(device, true); // Switch to data mode
  if (callback) {
    spi_master_t *obj = &spi_master_obj;
	SPIDRV_Handle_t handle = (SPIDRV_Handle_t)obj->handle; // Get SPIDRV handle
    transfer_complete_callback = callback;
    spi_select();
    Ecode_t retVal = SPIDRV_MTransmit(handle,
                                    (uint8_t *)framebuf,
                                    desc->buf_size,
                                    dma_transfer_complete_callback);
 
    status = (ECODE_OK == retVal) ? SL_STATUS_OK : SL_STATUS_IO;	
	
  } else {
    status = spi_write_b(framebuf, desc->buf_size); // Write entire framebuffer
  }
  return status;
}

/*****************************************************************************
 * Function : dma_transfer_complete_callback
 * Arguments: struct SPIDRV_HandleData *handle - Pointer to SPIDRV handle,
 *            Ecode_t transferStatus - Transfer status code,
 *            int itemsTransferred - Number of items transferred
 * Description: Callback function called when DMA transfer completes, deselects device and triggers completion callback
 * Return: void
 ****************************************************************************/
static void dma_transfer_complete_callback(struct SPIDRV_HandleData *handle,
                                           Ecode_t transferStatus,
                                           int itemsTransferred )
{
  (void) handle; // Not used in callback
  (void) itemsTransferred; // Not used in callback

  spi_deselect(); // Always deselect device when transfer completes
  if (transferStatus == ECODE_OK) { // Check if transfer was successful
    if (transfer_complete_callback) { // Check if user callback is registered
      transfer_complete_callback(); // Invoke user-provided completion callback
    }
  }
}

/*****************************************************************************
 * Function : mipi_dbi_device_init
 * Arguments: struct mipi_dbi_device *device - Pointer to device structure,
 *            const struct mipi_dbi_config *config - Pointer to configuration
 * Description: Initializes the MIPI DBI device with SPI configuration, GPIO pins, and API callbacks
 * Return: sl_status_t
 *               SL_STATUS_OK - on success,
 *               SL_STATUS_NOT_INITIALIZED - on initialization failure
 ****************************************************************************/
sl_status_t mipi_dbi_device_init(struct mipi_dbi_device *device,
                                 const struct mipi_dbi_config *config)
{
  struct mipi_dbi_spidrv_config *device_config =
    (struct mipi_dbi_spidrv_config *)config; // Cast to SPI-specific configuration

  spi_master_config_t spi_cfg; // SPI master configuration structure

  spi_master_configure_default(&spi_cfg); // Initialize with default values

  // Configure SPI parameters for high-speed display communication
  spi_cfg.mode              = SPI_MASTER_MODE_3; // Use SPI mode 3 (CPOL=1, CPHA=1)
  spi_cfg.cs_mode           = SPI_MASTER_CS_MODE_SW; // Software-controlled chip select
  spi_cfg.speed             = 9600000UL;  //19200000UL;//38400000UL; // Set high SPI speed (38 MHz)
  spi_cfg.default_write_data = 0xFF; // Default data when reading (pull-up value)

  spi_master_obj.handle = spidrv_handle; // Assign SPIDRV handle to master object

  // Open SPI master device
  if (spi_master_open(&spi_master_obj, &spi_cfg) != SPI_MASTER_SUCCESS) {
    return SL_STATUS_NOT_INITIALIZED; // Failed to open SPI device
  }

  // Initialize SPIDRV with provided configuration
  if (SPIDRV_Init((SPIDRV_Handle_t)spidrv_handle, device_config->spidrv_init) != ECODE_EMDRV_SPIDRV_OK) {
      return SL_STATUS_NOT_INITIALIZED; // Failed to initialize SPIDRV
  }

  // Configure chip select pin if application-controlled
  if (device_config->spidrv_init->csControl == spidrvCsControlApplication) {
    GPIO_PinModeSet(device_config->spidrv_init->portCs, // Configure CS as push-pull output
                    device_config->spidrv_init->pinCs,
                    gpioModePushPull,
                    1); // Start with CS high (inactive)
  }

  // Configure Data/Command control pin as push-pull output
  GPIO_PinModeSet(device_config->dc.port, device_config->dc.pin,
                  gpioModePushPull,
                  0); // Start with DC low (command mode)

  device->api = &mipi_dbi_api; // Assign MIPI DBI API function pointers
  device->config = config; // Store configuration pointer
  device->data = (struct mipi_dbi_data *)&mipi_dbi_spi_gecko_data; // Assign device data structure
  return SL_STATUS_OK; // Initialization successful
}


/*****************************************************************************
 * Function : mipi_dbi_device_getBitRate
 * Arguments: uint32_t *bitRate - Pointer to store bit rate
 * Description: Retrieves the current SPI bit rate from the master device
 * Return: sl_status_t
 *               SL_STATUS_OK - on success,
 *               SL_STATUS_INVALID_PARAMETER - if bitRate is NULL
 ****************************************************************************/
sl_status_t mipi_dbi_device_getBitRate(uint32_t *bitRate)
{
  if (!bitRate) {
    return SL_STATUS_INVALID_PARAMETER; // Null pointer validation
  }
 
  if (SPIDRV_GetBitrate((SPIDRV_Handle_t)spi_master_obj.handle, bitRate) != ECODE_EMDRV_SPIDRV_OK)
  {  
     return SL_STATUS_FAIL;
  }
  
  return SL_STATUS_OK; // Success
}
