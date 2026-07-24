/***************************************************************************//**
 * @file drv_spi_master.c
 * @brief mikroSDK 2.0 Click Peripheral Drivers - SPI Master
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
#include "em_gpio.h"
#include "drv_spi_master.h"
#include "drv_digital_out.h"
#include "spidrv.h"

#define SPI_GPIO_SLOW_SLEWRATE 5
#define SPI_GPIO_FAST_SLEWRATE 7

static spi_master_t *_owner = NULL;
static uint32_t last_spi_speed_used;
static spi_master_mode_t last_spi_mode_used = SPI_MASTER_MODE_3;

static spi_master_chip_select_polarity_t spi_master_chip_select_polarity =
  SPI_MASTER_CHIP_SELECT_DEFAULT_POLARITY;

static err_t spi_master_set_config(spi_master_t *obj);
static err_t _acquire(spi_master_t *obj, bool obj_open_state);
static void spi_master_configure_gpio_pin(digital_out_t *out, pin_name_t name);

/***************************************************************************//**
 * Function: spi_master_configure_default
 * Arguments: spi_master_config_t *config - Pointer to configuration structure
 * Description: Configures the SPI master configuration with default values
 * Return: void
 ******************************************************************************/
void spi_master_configure_default(spi_master_config_t *config)
{
  config->default_write_data = 0; // Default dummy data = 0
  config->sck = 0xFFFFFFFF; // SCK pin not defined
  config->miso = 0xFFFFFFFF; // MISO pin not defined
  config->mosi = 0xFFFFFFFF; // MOSI pin not defined
  config->speed = 100000; // Default speed: 100 kHz
  config->cs_mode = SPI_MASTER_CS_MODE_HW; // Hardware CS control
  config->mode = SPI_MASTER_MODE_DEFAULT; // Default SPI mode
}

/***************************************************************************//**
 * Function: spi_master_open
 * Arguments: spi_master_t *obj                    - Pointer to SPI master object,
 *            spi_master_config_t *config         - Pointer to configuration structure
 * Description: Opens and initializes the SPI master with the provided configuration
 * Return: err_t - SPI_MASTER_SUCCESS on success,
 *                 SPI_MASTER_ERROR on failure
 ******************************************************************************/
err_t spi_master_open(spi_master_t *obj, spi_master_config_t *config)
{
  spi_master_config_t *p_config = &obj->config; // Get pointer to SPI config struct
  memcpy(p_config, config, sizeof(spi_master_config_t)); // Copy provided config to object

  if (_acquire(obj, true) != ACQUIRE_SUCCESS) { // Acquire ownership of SPI device
    return SPI_MASTER_ERROR; // Already in use by another owner
  }

  return SPI_MASTER_SUCCESS;
}

/***************************************************************************//**
 * Function: spi_master_select_device
 * Arguments: pin_name_t chip_select - Pin name for chip select
 * Description: Selects the SPI slave device by setting the chip select pin according to polarity
 * Return: void
 ******************************************************************************/
void spi_master_select_device(pin_name_t chip_select)
{
  digital_out_t struct_cs; // GPIO output pin structure

  if (chip_select == 0xFFFFFFFF) { // Check if pin is not configured
    return; // Skip if invalid pin
  }

  spi_master_configure_gpio_pin(&struct_cs, chip_select); // Configure GPIO pin

  if (spi_master_chip_select_polarity == SPI_MASTER_CHIP_SELECT_POLARITY_ACTIVE_HIGH) {
    digital_out_high(&struct_cs); // Set CS high for active-high polarity
  } else {
    digital_out_low(&struct_cs); // Set CS low for active-low polarity
  }
}

/***************************************************************************//**
 * Function: spi_master_deselect_device
 * Arguments: pin_name_t chip_select - Pin name for chip select
 * Description: Deselects the SPI slave device by clearing the chip select pin according to polarity
 * Return: void
 ******************************************************************************/
void spi_master_deselect_device(pin_name_t chip_select)
{
  digital_out_t struct_cs; // GPIO output pin structure

  if (chip_select == 0xFFFFFFFF) { // Check if pin is not configured
    return; // Skip if invalid pin
  }

  spi_master_configure_gpio_pin(&struct_cs, chip_select); // Configure GPIO pin

  if (spi_master_chip_select_polarity == SPI_MASTER_CHIP_SELECT_POLARITY_ACTIVE_HIGH) {
    digital_out_low(&struct_cs); // Set CS low for active-high polarity
  } else {
    digital_out_high(&struct_cs); // Set CS high for active-low polarity
  }
}

/***************************************************************************//**
 * Function: spi_master_set_chip_select_polarity
 * Arguments: spi_master_chip_select_polarity_t polarity - Polarity for chip select signal
 * Description: Sets the polarity for the chip select signal (active high or active low)
 * Return: void
 ******************************************************************************/
void spi_master_set_chip_select_polarity(spi_master_chip_select_polarity_t polarity)
{
  if (polarity == SPI_MASTER_CHIP_SELECT_POLARITY_ACTIVE_HIGH) {
    spi_master_chip_select_polarity = SPI_MASTER_CHIP_SELECT_POLARITY_ACTIVE_HIGH; // Set to active-high
  } else {
    spi_master_chip_select_polarity = SPI_MASTER_CHIP_SELECT_POLARITY_ACTIVE_LOW; // Set to active-low
  }
}

/***************************************************************************//**
 * Function: spi_master_set_speed
 * Arguments: spi_master_t *obj - Pointer to SPI master object,
 *            uint32_t speed    - Desired SPI speed in Hz
 * Description: Sets the SPI communication speed (bitrate) for the master
 * Return: err_t - SPI_MASTER_SUCCESS on success,
 *                 SPI_MASTER_ERROR on failure
 ******************************************************************************/
err_t spi_master_set_speed(spi_master_t *obj, uint32_t speed)
{
  if (_acquire(obj, false) != ACQUIRE_SUCCESS) { // Acquire device ownership
    return SPI_MASTER_ERROR;
  }

  SPIDRV_Handle_t spidrv = (SPIDRV_Handle_t)obj->handle; // Get SPIDRV handle

  // Select slew rate based on speed (fast slew for speeds >= 10 MHz)
  uint32_t required_slewrate = (speed >= 10000000) ? SPI_GPIO_FAST_SLEWRATE : SPI_GPIO_SLOW_SLEWRATE;

  GPIO_SlewrateSet(spidrv->initData.portClk, required_slewrate, required_slewrate); // Set CLK slew rate
  GPIO_SlewrateSet(spidrv->initData.portTx, required_slewrate, required_slewrate); // Set TX slew rate

  obj->config.speed = speed; // Store new speed
  last_spi_speed_used = speed; // Update global speed tracker

  // Set SPI bus bitrate through SPIDRV
  if (SPIDRV_SetBitrate(spidrv, speed) != ECODE_EMDRV_SPIDRV_OK) {
    return SPI_MASTER_ERROR; // Failed to set bitrate
  }

  return SPI_MASTER_SUCCESS;
}

/***************************************************************************//**
 * Function: spi_master_set_mode
 * Arguments: spi_master_t *obj      - Pointer to SPI master object,
 *            spi_master_mode_t mode - SPI mode (0-3)
 * Description: Sets the SPI communication mode (clock polarity and phase)
 * Return: err_t - SPI_MASTER_SUCCESS on success,
 *                 SPI_MASTER_ERROR on failure
 ******************************************************************************/
err_t spi_master_set_mode(spi_master_t *obj, spi_master_mode_t mode)
{
  if (_acquire(obj, false) != ACQUIRE_SUCCESS) { // Acquire device ownership
    return SPI_MASTER_ERROR;
  }

  obj->config.mode = mode; // Store new SPI mode

  return spi_master_set_config(obj); // Apply configuration changes
}

/***************************************************************************//**
 * Function: spi_master_set_default_write_data
 * Arguments: spi_master_t *obj           - Pointer to SPI master object,
 *            uint8_t default_write_data - Default byte to send during read operations
 * Description: Sets the default dummy data to be sent during SPI read operations
 * Return: err_t - SPI_MASTER_SUCCESS on success,
 *                 SPI_MASTER_ERROR on failure
 ******************************************************************************/
err_t spi_master_set_default_write_data(spi_master_t *obj, uint8_t default_write_data)
{
  if (_acquire(obj, false) != ACQUIRE_SUCCESS) { // Acquire device ownership
    return SPI_MASTER_ERROR;
  }

  obj->config.default_write_data = default_write_data; // Store default dummy data for reads
  return SPI_MASTER_SUCCESS;
}

/***************************************************************************//**
 * Function: spi_master_write
 * Arguments: spi_master_t *obj          - Pointer to SPI master object,
 *            uint8_t *write_data_buffer - Buffer with data to write,
 *            size_t write_data_length   - Number of bytes to write
 * Description: Writes data to the SPI bus
 * Return: err_t - SPI_MASTER_SUCCESS on success,
 *                 SPI_MASTER_ERROR on failure
 ******************************************************************************/
err_t spi_master_write(spi_master_t *obj, uint8_t *write_data_buffer, size_t write_data_length)
{
  if (_acquire(obj, false) != ACQUIRE_SUCCESS) { // Acquire device ownership
    return SPI_MASTER_ERROR;
  }

  // Check if speed has changed since last operation
  if (last_spi_speed_used != obj->config.speed) {
    last_spi_speed_used = obj->config.speed; // Update speed tracker
    // Update SPI bus bitrate.
    if (SPIDRV_SetBitrate((SPIDRV_Handle_t)obj->handle, last_spi_speed_used) != ECODE_EMDRV_SPIDRV_OK) {
      return SPI_MASTER_ERROR; // Failed to update bitrate
    }
  }

  // Check if mode has changed since last operation
  if (last_spi_mode_used != obj->config.mode) {
    // Update the config mode
    if (spi_master_set_config(obj) != SPI_MASTER_SUCCESS) { // Apply mode configuration
      return SPI_MASTER_ERROR; // Failed to set mode
    }
  }

  // Perform blocking transmit operation
  if (SPIDRV_MTransmitB((SPIDRV_Handle_t)obj->handle, write_data_buffer, write_data_length) != ECODE_EMDRV_SPIDRV_OK) {
    return SPI_MASTER_ERROR; // Transfer failed
  }
  return SPI_MASTER_SUCCESS;
}

/***************************************************************************//**
 * Function: spi_master_read
 * Arguments: spi_master_t *obj        - Pointer to SPI master object,
 *            uint8_t *read_data_buffer - Buffer to store read data,
 *            size_t read_data_length   - Number of bytes to read
 * Description: Reads data from the SPI bus
 * Return: err_t - SPI_MASTER_SUCCESS on success,
 *                 SPI_MASTER_ERROR on failure
 ******************************************************************************/
err_t spi_master_read(spi_master_t *obj, uint8_t *read_data_buffer, size_t read_data_length)
{
  if (_acquire(obj, false) != ACQUIRE_SUCCESS) { // Acquire device ownership
    return SPI_MASTER_ERROR;
  }

  if (read_data_length > DMADRV_MAX_XFER_COUNT) { // Check transfer size limit
    return SPI_MASTER_ERROR; // Requested size exceeds maximum
  }

  uint8_t write_data_buffer[read_data_length]; // Temporary TX buffer for dummy data

  // Check if speed has changed
  if (last_spi_speed_used != obj->config.speed) {
    last_spi_speed_used = obj->config.speed;
    // Update SPI bus bitrate.
    if (SPIDRV_SetBitrate((SPIDRV_Handle_t)obj->handle, last_spi_speed_used) != ECODE_EMDRV_SPIDRV_OK) {
      return SPI_MASTER_ERROR;
    }
  }

  // Check if mode has changed
  if (last_spi_mode_used != obj->config.mode) {
    // Update the config mode
    if (spi_master_set_config(obj) != SPI_MASTER_SUCCESS) {
      return SPI_MASTER_ERROR;
    }
  }

  // Fill TX buffer with default write data for read operation
  for (size_t i = 0; i < read_data_length; i++) {
    write_data_buffer[i] = _owner->config.default_write_data; // Use configured dummy data
  }

  // Perform full-duplex transfer (send dummy, receive data)
  if (SPIDRV_MTransferB((SPIDRV_Handle_t)obj->handle, write_data_buffer, read_data_buffer, read_data_length) != ECODE_EMDRV_SPIDRV_OK) {
    return SPI_MASTER_ERROR;
  }
  return SPI_MASTER_SUCCESS;
}

/***************************************************************************//**
 * Function: spi_master_exchange
 * Arguments: spi_master_t *obj            - Pointer to SPI master object,
 *            uint8_t *write_data_buffer   - Buffer with data to transmit,
 *            uint8_t *read_data_buffer    - Buffer to store received data,
 *            size_t exchange_data_length  - Number of bytes to exchange
 * Description: Exchanges bytes on SPI bus (simultaneous transmit and receive)
 * Return: err_t - SPI_MASTER_SUCCESS on success,
 *                 SPI_MASTER_ERROR on failure
 *******************************************************************************/
err_t spi_master_exchange(spi_master_t *obj, uint8_t *write_data_buffer,
                          uint8_t *read_data_buffer, size_t exchange_data_length)
{
  if (_acquire(obj, false) != ACQUIRE_SUCCESS) { // Acquire device ownership
    return SPI_MASTER_ERROR;
  }

  // Check if speed has changed
  if (last_spi_speed_used != obj->config.speed) {
    last_spi_speed_used = obj->config.speed;
    // Update SPI bus bitrate.
    if (SPIDRV_SetBitrate((SPIDRV_Handle_t)obj->handle, last_spi_speed_used) != ECODE_EMDRV_SPIDRV_OK) {
      return SPI_MASTER_ERROR;
    }
  }

  // Check if mode has changed
  if (last_spi_mode_used != obj->config.mode) {
    // Update the config mode
    if (spi_master_set_config(obj) != SPI_MASTER_SUCCESS) { // Apply mode configuration
      return SPI_MASTER_ERROR;
    }
  }

  // Perform full-duplex transfer (simultaneous TX and RX)
  if (SPIDRV_MTransferB((SPIDRV_Handle_t)obj->handle, write_data_buffer, read_data_buffer, exchange_data_length) != ECODE_EMDRV_SPIDRV_OK) {
    return SPI_MASTER_ERROR;
  }
  return SPI_MASTER_SUCCESS;
}

/***************************************************************************//**
 * Function: spi_master_write_then_read
 * Arguments: spi_master_t *obj              - Pointer to SPI master object,
 *            uint8_t *write_data_buffer    - Buffer with data to write first,
 *            size_t length_write_data      - Number of bytes to write,
 *            uint8_t *read_data_buffer     - Buffer to store read data,
 *            size_t length_read_data       - Number of bytes to read
 * Description: Writes data to SPI bus followed immediately by reading data from SPI bus
 * Return: err_t - SPI_MASTER_SUCCESS on success,
 *                 SPI_MASTER_ERROR on failure
 ******************************************************************************/
err_t spi_master_write_then_read(spi_master_t *obj,
                                 uint8_t *write_data_buffer,
                                 size_t length_write_data,
                                 uint8_t *read_data_buffer,
                                 size_t length_read_data)
{
  if (_acquire(obj, false) != ACQUIRE_SUCCESS) { // Acquire device ownership
    return SPI_MASTER_ERROR;
  }

  size_t tx_len = length_write_data + length_read_data; // Total transfer length

  if (tx_len > DMADRV_MAX_XFER_COUNT) { // Check transfer size limit
    return SPI_MASTER_ERROR; // Total transfer exceeds maximum
  }

  uint8_t tx_buffer[tx_len]; // Combined TX buffer
  uint8_t rx_buffer[tx_len]; // Combined RX buffer

  // Check if speed has changed
  if (last_spi_speed_used != obj->config.speed) {
    last_spi_speed_used = obj->config.speed;
    // Update SPI bus bitrate.
    if (SPIDRV_SetBitrate((SPIDRV_Handle_t)obj->handle, last_spi_speed_used) != ECODE_EMDRV_SPIDRV_OK) {
      return SPI_MASTER_ERROR;
    }
  }

  // Check if mode has changed
  if (last_spi_mode_used != obj->config.mode) {
    // Update the config mode
    if (spi_master_set_config(obj) != SPI_MASTER_SUCCESS) {
      return SPI_MASTER_ERROR;
    }
  }

  // Copy write data to first part of TX buffer
  for (size_t i = 0; i < length_write_data; i++) {
    tx_buffer[i] = write_data_buffer[i]; // Copy user write data
  }

  // Fill remaining TX buffer with default write data for read phase
  for (size_t i = length_write_data; i < tx_len; i++) {
    tx_buffer[i] = _owner->config.default_write_data; // Use dummy data for read portion
  }

  // Perform full-duplex transfer (write+read in one transaction)
  if (SPIDRV_MTransferB((SPIDRV_Handle_t)obj->handle, tx_buffer, rx_buffer, tx_len) != ECODE_EMDRV_SPIDRV_OK) {
    return SPI_MASTER_ERROR;
  }

  // Extract received data from second part of RX buffer
  for (size_t i = 0; i < length_read_data; i++) {
    read_data_buffer[i] = rx_buffer[i + length_write_data]; // Copy read data from RX buffer
  }

  return SPI_MASTER_SUCCESS;
}

/***************************************************************************//**
 * Function: spi_master_close
 * Arguments: spi_master_t *obj - Pointer to SPI master object
 * Description: Closes the SPI master and releases resources
 * Return: void
 ******************************************************************************/
void spi_master_close(spi_master_t *obj)
{
  obj->handle = NULL; // Clear SPI driver handle
  _owner = NULL; // Release device ownership
}

/***************************************************************************//**
 * Function: _acquire
 * Arguments: spi_master_t *obj       - Pointer to SPI master object,
 *            bool obj_open_state     - true if opening, false if re-using
 * Description: Acquires ownership of the SPI device to prevent multiple uses
 * Return: err_t - ACQUIRE_SUCCESS if ownership acquired,
 *                 ACQUIRE_FAIL if already in use
 ******************************************************************************/
static err_t _acquire(spi_master_t *obj, bool obj_open_state)
{
  if ((obj_open_state == true) && (_owner == obj)) {
    return ACQUIRE_FAIL; // Already opened by this object
  }

  if (_owner != obj) {
    _owner = obj; // Acquire ownership
  }

  return ACQUIRE_SUCCESS;
}

/***************************************************************************//**
 * Function: spi_master_configure_gpio_pin
 * Arguments: digital_out_t *out  - Pointer to GPIO output structure,
 *            pin_name_t name     - Pin name to configure
 * Description: Configures a GPIO pin for digital output (chip select)
 * Return: void
 ******************************************************************************/
static void spi_master_configure_gpio_pin(digital_out_t *out, pin_name_t name)
{
  out->pin.base = hal_gpio_port_index(name); // Extract port from pin name
  out->pin.mask = hal_gpio_pin_index(name); // Extract pin index from pin name
}

/***************************************************************************//**
 * Function: spi_master_set_config
 * Arguments: spi_master_t *obj - Pointer to SPI master object
 * Description: Applies SPI mode configuration by re-initializing the SPIDRV
 * Return: err_t - SPI_MASTER_SUCCESS on success,
 *                 SPI_MASTER_ERROR on failure
 ******************************************************************************/
static err_t spi_master_set_config(spi_master_t *obj)
{
  SPIDRV_Init_t initData; // Temporary init data structure
  SPIDRV_Handle_t ptr = (SPIDRV_Handle_t)obj->handle; // Get SPIDRV handle

  last_spi_mode_used = obj->config.mode; // Update mode tracker

  // Copy current SPIDRV initialization to temporary structure
  memcpy(&initData, &ptr->initData, sizeof(SPIDRV_Init_t));

  initData.clockMode = (SPIDRV_ClockMode_t) obj->config.mode; // Set new SPI mode
  initData.bitRate = obj->config.speed; // Set configured speed

  // De-initialize SPIDRV for reconfiguration
  if (SPIDRV_DeInit((SPIDRV_Handle_t)obj->handle) != ECODE_EMDRV_SPIDRV_OK) {
    return SPI_MASTER_ERROR; // De-init failed
  }

  // Re-initialize SPIDRV with new mode configuration
  if (SPIDRV_Init((SPIDRV_Handle_t)obj->handle, &initData) != ECODE_EMDRV_SPIDRV_OK) {
    return SPI_MASTER_ERROR; // Re-init failed
  }

  return SPI_MASTER_SUCCESS;
}

/***************************************************************************//**
 * Function: spi_master_control_cs
 * Arguments: spi_master_t *obj                 - Pointer to SPI master object,
 *            spi_slave_chip_select_t obj_cs   - Chip select state (high or low)
 * Description: Manually controls the chip select pin when using application-controlled CS
 * Return: err_t - SPI_MASTER_SUCCESS always
 ******************************************************************************/
err_t spi_master_control_cs(spi_master_t *obj, spi_slave_chip_select_t obj_cs)
{
  SPIDRV_Handle_t handle = (SPIDRV_Handle_t)obj->handle; // Get SPIDRV handle

  if (handle->initData.csControl == spidrvCsControlApplication) { // Check if CS is app-controlled
    if (obj_cs == SPI_SLAVE_CHIP_SELECT_HIGH) {
      GPIO_PinOutSet(handle->portCs, handle->pinCs); // Set CS pin high (inactive)
    } else if (obj_cs == SPI_SLAVE_CHIP_SELECT_LOW) {
      GPIO_PinOutClear(handle->portCs, handle->pinCs); // Set CS pin low (active)
    }
  }

  return SPI_MASTER_SUCCESS;
}

// ------------------------------------------------------------------------- END
