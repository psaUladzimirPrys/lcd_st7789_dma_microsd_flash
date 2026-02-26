/***************************************************************************//**
 * sd_card.c
 *
 *  Created on: 15 Jan. 2026.
 *      Author: priss
*******************************************************************************/
#include <stdio.h>
#include <string.h>
#include "sl_sleeptimer.h"
#include "file_storage.h"
#include "flash_storage.h"


/***************************************************************************//**
*  File storage of SD card configuration section declaration global variables
*******************************************************************************/

static mikroe_spi_handle_t app_spi_instance = NULL;

#if !FF_FS_NORTC && !FF_FS_READONLY
  static DWORD sd_fatfs_time_data;
#endif

static BYTE  F_work[FF_MAX_SS];         // Work file buffer area (larger is better for processing time)
static FATFS FatFs;                     // FatFS object for mounting.
static FIL   File;                      // File object structure

//static BYTE work_buffer[FF_MAX_SS];   // Working buffer for operations (e.g., f_mkfs).
static bool sd_mounted = false;         // Mounting flag.


#define FS_LOG_RING_BUFFER_SIZE   1024
#define FS_LOG_FLUSH_THRESHOLD     256
#define FS_LOG_FILE_PATH          "log.txt"



static uint8_t fs_log_ring_buffer[FS_LOG_RING_BUFFER_SIZE];
static volatile uint16_t fs_log_ring_head = 0;
static volatile uint16_t fs_log_ring_tail = 0;
static volatile uint16_t fs_log_ring_used = 0;

static inline uint16_t fs_log_rb_free(void)
{
  return FS_LOG_RING_BUFFER_SIZE - fs_log_ring_used;
}


/*
 * Function:    fs_sd_log_init
 * Arguments:   void
 * Description:
 *   Initializes the log ring buffer, SD card and checks log file.
 *
 * Return Message: void
 */
void fs_sd_log_init(void)
{
  sl_status_t sl_status_code = SL_STATUS_OK;
  const char filepath[] = FS_LOG_FILE_PATH;
  uint32_t f_size;

  fs_log_ring_head = 0;
  fs_log_ring_tail = 0;
  fs_log_ring_used = 0;

  // Initialize file storage of SD card
  sl_status_code = fs_sd_init();
  if (sl_status_code != SL_STATUS_OK) {
    // Failed to init SD card, handle error
    app_assert_status(SL_STATUS_FAIL); // Loop forever for debugging
  }

  sl_status_code = fs_sd_time_init();
  if (sl_status_code != SL_STATUS_OK) {
     // Failed to init time, handle error
     app_assert_status(SL_STATUS_FAIL); // Loop forever for debugging
  }

  if ( fs_sd_disk_volume_status() != SL_STATUS_OK) {
    // Failed to init SD card, handle error
    app_assert_status(SL_STATUS_FAIL); // Loop forever for debugging
  }

  if (fs_sd_get_file_size(filepath, &f_size) != SL_STATUS_OK) {
    app_log("Getting size of file: %s Failed\r\n", filepath);
    app_assert_status(SL_STATUS_FAIL); // Loop forever for debugging
  }

  app_printf("File %s, size = %lu\r\n",filepath, f_size);

}

/*
 * Function:    fs_sd_log_write
 * Arguments:   const char *str
 * Description:
 *   Writes a string to the ring buffer for later flushing to SD card.
 *
 * Return Message: void
 */
void fs_sd_log_write(const char *str)
{
  uint16_t len;

  if (str == NULL) {
    return;
  }

  if (!sd_mounted) {
      app_log_warning("SD card logical drive is not mounted.\r\n");
      return;
  }

  len = strlen(str);

  /* Drop data if buffer overflow */
  if (len > fs_log_rb_free()) {
    return;
  }

  for (uint16_t i = 0; i < len; i++) {
      fs_log_ring_buffer[fs_log_ring_head++] = (uint8_t)str[i];
    if (fs_log_ring_head >= FS_LOG_RING_BUFFER_SIZE) {
        fs_log_ring_head = 0;
    }
  }

  fs_log_ring_used += len;
}

/*
 * Function:    fs_sd_log_flush_task
 * Arguments:   void
 * Description:
 *   Periodic task that flushes accumulated logs to the SD card file.
 *
 * Return Message: void
 */
void fs_sd_log_flush_task(void)
{
  uint8_t temp_buf[FS_LOG_FLUSH_THRESHOLD];
  uint16_t chunk;



  if (fs_log_ring_used < FS_LOG_FLUSH_THRESHOLD) {
    return;
  }

  chunk = (fs_log_ring_used > FS_LOG_FLUSH_THRESHOLD)
          ? FS_LOG_FLUSH_THRESHOLD
          : fs_log_ring_used;

  for (uint16_t i = 0; i < chunk; i++) {
    temp_buf[i] = fs_log_ring_buffer[fs_log_ring_tail++];
    if (fs_log_ring_tail >= FS_LOG_RING_BUFFER_SIZE) {
        fs_log_ring_tail = 0;
    }
  }

  fs_log_ring_used -= chunk;

  if (!sd_mounted) {
      app_log_warning("SD card logical drive is not mounted.\r\n");
      app_assert_status(SL_STATUS_FAIL); // Loop forever for debugging
      return;
  }

  if (fs_sd_append_to_file(FS_LOG_FILE_PATH, temp_buf, chunk) != SL_STATUS_OK) {
    app_log("Append to file: Failed\r\n");
    app_assert_status(SL_STATUS_FAIL); // Loop forever for debugging
  }

}

/*
 * Function:    fs_sd_time_init
 * Arguments:   void
 * Description:
 *   Initializes system time for FatFS (used by get_fattime()).
 *
 * Return Message: sl_status_t
 *   - SL_STATUS_OK: Time initialized successfully
 */
sl_status_t fs_sd_time_init(void)
{

  sl_status_t sl_status = SL_STATUS_OK;

#if !FF_FS_NORTC && !FF_FS_READONLY
  sl_status_t sc;
  sl_sleeptimer_date_t date_time = {
    .year = 122,
    .month = 2,
    .month_day = 1,
    .hour = 10,
    .min = 30,
    .sec = 0,
  };


  sc = sl_sleeptimer_set_datetime(&date_time);
  app_assert_status(sc);

  sd_fatfs_time_data = get_fattime();
  app_log_info("\nCurrent time is %lu/%lu/%lu %2lu:%02lu:%02lu.\n\n",
             (sd_fatfs_time_data >> 25) + 1980,
             (sd_fatfs_time_data >> 21) & 0x0f,
             (sd_fatfs_time_data >> 16) & 0x1f,
             (sd_fatfs_time_data >> 11) & 0x1f,
             (sd_fatfs_time_data >> 5) & 0x3f,
             (sd_fatfs_time_data << 1) & 0x1f);

  sl_status = sc;
#endif

  return sl_status;
}

/*
 * Function:    fs_sd_init
 * Arguments:   void
 * Description:
 *   Initializes SPI interface, configures MISO/CS pins and mounts FatFS.
 *
 * Return Message: sl_status_t
 *   - SL_STATUS_OK: SD card initialized and mounted successfully
 */
sl_status_t fs_sd_init(void)
{
  sl_status_t sl_status = SL_STATUS_OK;
  FRESULT     fs_res;
  sl_gpio_t   pinport;
  uint32_t    bitRate;

  sd_mounted = false;

  memset((void *)&FatFs, 0x00, sizeof(FATFS));
  memset((void *)&File, 0x00, sizeof(File));
  memset((void *)&F_work[0], 0x00, sizeof(F_work));

#if (defined(SLI_SI917))
  app_spi_instance = &gspi_instance;
#else
  app_spi_instance = sl_spidrv_mikroe_handle;
#endif

  // Initialising SPI for SD (using MikroE config)
  app_log_info("Initializing SD card...\r\n");
  sl_status = sd_card_spi_init(app_spi_instance);  // Assuming that app_spi_instance is a global object
  if (sl_status != SL_STATUS_OK) {
    app_log_debug("SD card SPI init failed: %lu\r\n", sl_status);
    return sl_status;
  }


  /* The MISO pin is driven by a tri-stated output from the SDcard.
   * Therefore, the MISO pin should be configured as a pull-up input
   * to hold that input into a known state when the SDcard is not selected.
   */
  pinport.port = sl_spidrv_mikroe_handle->initData.portRx;
  pinport.pin = sl_spidrv_mikroe_handle->initData.pinRx;

  /* @ToDo must remove after pins validation  It was been added by UP*/
  if ((pinport.port != SL_GPIO_PORT_C) && (pinport.pin != 1)) {
    app_log_debug("SD card SPI MISO port failed: %lu\r\n", sl_status);
    return SL_STATUS_FAIL;
  }


  sl_status = sl_gpio_set_pin_mode(&pinport, SL_GPIO_MODE_INPUT_PULL, 1);
  if (sl_status != SL_STATUS_OK) {
    app_log_debug("SD card SPI MISO pin failed: %lu\r\n", sl_status);
    return sl_status;
  }

  /* The CS pin is driven by a input to the SDcard.
   * the CS pin should be configured as a pull-up output
   * when the SDcard is selected/deselected.
   */
  if (sl_spidrv_mikroe_handle->initData.csControl == spidrvCsControlApplication) {

    pinport.port = sl_spidrv_mikroe_handle->initData.portCs;
    pinport.pin = sl_spidrv_mikroe_handle->initData.pinCs;

    /* @ToDo must remove after pins validation  It was been added by UP*/
    if ((pinport.port != SL_GPIO_PORT_B) && (pinport.pin != 0)) {
        app_log_debug("SD card SPI CS port failed: %lu\r\n", sl_status);
       return SL_STATUS_FAIL;
    }

    sl_status = sl_gpio_set_pin_mode(&pinport, SL_GPIO_MODE_PUSH_PULL, 1);
    if (sl_status != SL_STATUS_OK) {
        app_log_debug("SD card SPI CS pin failed: %lu\r\n", sl_status);
       return sl_status;
    }

  }

  // Initialising the disk and mounting FatFS
  // Give a work area to the default drive
  fs_res = f_mount(&FatFs, "", 1);  /* Mount the default drive */
  if (fs_res != FR_OK) {
    app_log_debug("Disk initialize or disk mount failed: %d\r\n", fs_res); // Failed to mount SD
    return SL_STATUS_FAIL;
  }

  if ( SPIDRV_GetBitrate(sl_spidrv_mikroe_handle, &bitRate) == ECODE_EMDRV_SPIDRV_OK ) {
      app_log_debug("SD Card SPI bitrate=%luMHZ \r\n",bitRate);
   } else {
      app_log_debug("SD Card SPI bitrate ERROR\r\n");
   }


  sd_mounted = true;
  app_log_info("SD card initialized and mounted.\r\n");


  return sl_status;
}

/*
 * Function:    fs_sd_disk_volume_status
 * Arguments:   void
 * Description:
 *   Prints detailed information about the FatFS volume.
 *
 * Return Message: sl_status_t
 *   - SL_STATUS_OK: Volume status printed successfully
 */
sl_status_t fs_sd_disk_volume_status(void)
{
  static const char *fst[] = { "", "FAT12", "FAT16", "FAT32", "exFAT" };
  static char path[] = {""};
  FATFS *pfs;
  FRESULT fs_res;
  DWORD fre_clust;


  if (!sd_mounted) {
      app_log_warning("SD card logical drive is not mounted.\r\n");
      return SL_STATUS_NOT_READY;
  }

  pfs = &FatFs;

  // Show logical drive status
  fs_res = f_getfree(path, &fre_clust, &pfs);
  if (fs_res != FR_OK) {
      app_log_debug("Disk volume status failed: %d\r\n", fs_res);
      return SL_STATUS_FAIL;
  }

  app_log_info("-------------- Volume status --------------\r\n");
  app_log_info(("FAT type = %s\r\nBytes/Cluster = %lu\r\nNumber of FATs = %u\r\n"
               "Root DIR entries = %u\r\nSectors/FAT = %lu\r\n"
               "Number of clusters = %lu\r\nVolume start (lba) = %lu\r\n"
               "FAT start (lba) = %lu\nDIR start (lba,clustor) = %lu\r\n"
               "Data start (lba) = %lu\r\n%lu KiB total disk space.\r\n"
               "%lu KiB available.\r\n"),
               fst[pfs->fs_type],
               (DWORD)pfs->csize * 512,
               pfs->n_fats,
               pfs->n_rootdir,
               pfs->fsize,
               pfs->n_fatent - 2,
               (DWORD)pfs->volbase,
               (DWORD)pfs->fatbase,
               (DWORD)pfs->dirbase,
               (DWORD)pfs->database,
               (pfs->n_fatent - 2) * (pfs->csize / 2),
               fre_clust * (pfs->csize / 2));

  return SL_STATUS_OK;
}

/*
 * Function:    fs_sd_write_file
 * Arguments:   const char *file_path, const void *data, uint32_t size
 * Description:
 *   Creates or overwrites a file and writes data to it.
 *
 * Return Message: sl_status_t
 *   - SL_STATUS_OK: File written successfully
 */
sl_status_t fs_sd_write_file(const char *file_path, const void *data, uint32_t size)
{

  FRESULT fs_res;
  UINT bytes_written;

  if ((file_path == NULL) || (data == NULL)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  if (!sd_mounted) {
      return SL_STATUS_NOT_READY;
  }

  // Open file to write
  fs_res = f_open(&File, file_path, FA_WRITE | FA_CREATE_ALWAYS);
  if (fs_res != FR_OK) {
      app_log_warning("Error f_open failed for %s: %d\r\n", file_path, fs_res);
      return SL_STATUS_FAIL;
  }

  // Write a data
  fs_res = f_write(&File, data, size, &bytes_written);
  if (fs_res != FR_OK || bytes_written != size) {
    f_close(&File);//Always file must Closed
    app_log_debug("Failed writing data to SD card! Data size = %lu\r\n", size);
    return SL_STATUS_FAIL;
  }

  app_log_debug("Write to SD card OK! Bytes = %d\r\n", bytes_written);

  fs_res = f_close(&File);// Close file
  if (fs_res != FR_OK) {
    app_log_critical("Error critical f_close failed for %s : %d\r\n", file_path, fs_res);
    app_assert_status(fs_res);
    return SL_STATUS_FAIL;
  }

  return SL_STATUS_OK;
}

/*
 * Function:    fs_sd_read_file
 * Arguments:   const char *file_path, void *buffer, uint32_t buffer_size
 * Description:
 *   Reads entire file into the provided buffer.
 *
 * Return Message: sl_status_t
 *   - SL_STATUS_OK: File read successfully
 */
sl_status_t fs_sd_read_file(const char *file_path, void *buffer, uint32_t buffer_size)
{
  UINT bytes_read;
  FRESULT fs_res;

  if ((file_path == NULL) || (buffer == NULL) || (buffer_size == 0)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  if (!sd_mounted) {
    return SL_STATUS_NOT_READY;
  }

  fs_res = f_open(&File, file_path, FA_READ | FA_OPEN_EXISTING);
  if (fs_res != FR_OK) {
    app_log_warning("Error f_open failed for %s : %d\r\n", file_path, fs_res);
    return SL_STATUS_FAIL;
  }

  fs_res = f_read(&File, buffer, buffer_size, &bytes_read);
  if (fs_res != FR_OK || bytes_read != buffer_size) {
    app_log_debug("Error f_read failed for %s : %d, read %u bytes\r\n", file_path, fs_res, bytes_read);
    f_close(&File); //Always file must Closed

    return SL_STATUS_FAIL;
  }

  fs_res = f_close(&File);// Close file
  if (fs_res != FR_OK) {
    app_log_critical("Error critical f_close failed for %s : %d\r\n", file_path, fs_res);
    app_assert_status(fs_res);
    return SL_STATUS_FAIL;
  }

  app_log_debug("Read from SD card OK! Bytes = %d\r\n", bytes_read);

  return SL_STATUS_OK;
}

/*
 * Function:    fs_sd_write_img_to_flash
 * Arguments:   const char *path, uint32_t flash_address
 * Description:
 *   Writes image file from SD card to external flash memory.
 *
 * Return Message: sl_status_t
 *   - SL_STATUS_OK: Image successfully written to flash
 */
sl_status_t fs_sd_write_img_to_flash(const char *path, uint32_t flash_address)
{

  sl_status_t sl_status = SL_STATUS_OK;

  if (!sd_mounted) {
    app_log_error("SD not mounted\r\n");
    return SL_STATUS_NOT_READY;
  }

   sl_status = fs_sd_read_file_and_write_flash(path, (void *)&F_work[0], FF_MAX_SS, flash_address);
  if ( sl_status != SL_STATUS_OK ) {
        app_log_error("Write Error File %s to Flash: %lu \r\n", path, sl_status);
        return SL_STATUS_FAIL;
  }

  app_log_info("Write file: %s OK to Flash from SD.\r\n", path);

  return SL_STATUS_OK;
}

/*
 * Function:    fs_sd_read_file_and_write_flash
 * Arguments:   const char *path, uint32_t flash_address, uint32_t chunk_size
 * Description:
 *   Reads the file from SD card in chunks of chunk_size bytes using internal
 *   static buffer work_buffer[], and writes each chunk to flash memory
 *   starting at flash_address. Address is automatically incremented after
 *   each successful write.
 *
 * Return Message: sl_status_t
 *   - SL_STATUS_OK: Read operation completed successfully
 */
sl_status_t fs_sd_read_file_and_write_flash(const char *path,
                                                  void *buffer,
                                              uint32_t buffer_size,
                                              uint32_t flash_address )
{
  uint32_t current_address;
  sl_status_t sl_status = SL_STATUS_OK;
  FRESULT fs_res = FR_OK;
  UINT bytes_read = 0;
  uint32_t file_size;

  if (buffer == NULL) {
    app_log_error("Pointer to buffer is NULL\r\n");
    return SL_STATUS_INVALID_PARAMETER;
  }

  if (buffer == 0 || buffer_size > FF_MAX_SS) {
    app_log_error("Invalid size: %lu (must be >0 and <= FF_MAX_SS)\r\n", buffer_size);
    return SL_STATUS_INVALID_PARAMETER;
  }

  if (path == NULL) {
    app_log_error("Invalid path\r\n");
    return SL_STATUS_INVALID_PARAMETER;
  }

  fs_res = f_open(&File, path, FA_READ | FA_OPEN_EXISTING);
  if (fs_res != FR_OK) {
    app_log_warning("Error f_open failed for %s: %d\r\n", path, fs_res);
    return SL_STATUS_FAIL;
  }

  current_address = flash_address;
  sl_status = SL_STATUS_OK;
  bytes_read = 0;
  file_size = f_size(&File);

  while ((file_size > 0 ) && (sl_status == SL_STATUS_OK)) {

    fs_res = f_read(&File, buffer, buffer_size, &bytes_read);
    if (fs_res != FR_OK) {
      app_log_debug("Error f_read failed: %d\r\n", fs_res);
      sl_status = SL_STATUS_FAIL;
      break;
    }

    if (bytes_read == 0) {
      //End read file
      break;
    }

    // Write buffer with bytes_read to flash
    sl_status = flash_storage_write(current_address, (const uint8_t *)buffer, bytes_read);
    if (sl_status != SL_STATUS_OK) {
      app_log_error("Flash write failed at addr = 0x%lx: %lu\r\n", current_address, sl_status);
      break;
    }

    file_size -= bytes_read;
    current_address += bytes_read;
  }

  //Always Close file
  fs_res = f_close(&File);
  if (fs_res != FR_OK) {
    app_log_debug("Error f_close failed for %s: %d\r\n", path, fs_res);
    return SL_STATUS_FAIL;
  }

  if (sl_status == SL_STATUS_OK) {
    app_log_info("File %s success R/W to flash at addr = 0x%lx\r\n", path, flash_address);
  }

  return sl_status;
}


/*
 * Function:    fs_sd_deinit
 * Arguments:   void
 * Description:
 *   Unmounts the SD card.
 *
 * Return Message: sl_status_t
 *   - SL_STATUS_OK: SD card unmounted successfully
 */
sl_status_t fs_sd_deinit(void)
{
  if (!sd_mounted) {
      return SL_STATUS_NOT_READY;
  }

  (void)f_mount(NULL, "", 1); /* Unmount the default drive */

  memset((void *)&FatFs, 0x00, sizeof(FATFS));
  memset((void *)&File, 0x00, sizeof(File));

  sd_mounted = false;

  return SL_STATUS_OK;
}

/*
 * Function:    fs_sd_get_file_size
 * Arguments:   const char *file_path, uint32_t *file_size
 * Description:
 *   Returns the size of the file in bytes.
 *
 * Return Message: sl_status_t
 *   - SL_STATUS_OK: File size obtained successfully
 */
sl_status_t fs_sd_get_file_size(const char *file_path,  uint32_t *file_size)
{

  FRESULT fs_res;

  if ((file_path == NULL) || (file_size == NULL)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  if (!sd_mounted) {
    return SL_STATUS_NOT_READY;
  }

  fs_res = f_open(&File, file_path, FA_READ | FA_OPEN_EXISTING);
  if (fs_res != FR_OK) {
    app_log_warning("Error f_open failed for %s : %d\r\n", file_path, fs_res);
    return SL_STATUS_FAIL;
  }

  *file_size = f_size(&File);

  fs_res = f_close(&File); // Always close!
  if (fs_res != FR_OK) {
    app_log_critical("Error critical  for %s : %d\r\n", file_path, fs_res);
    app_assert_status(fs_res);
    return SL_STATUS_FAIL;
  }

  uint32_t size = *file_size;
  app_log_debug("File size read OK: %lu bytes\r\n", size);

  return SL_STATUS_OK;
}

/*
 * Function:    fs_sd_append_to_file
 * Arguments:   const char *file_path, const void *data, uint32_t data_size
 * Description:
 *   Appends data to the end of an existing file.
 *
 * Return Message: sl_status_t
 *   - SL_STATUS_OK: Data appended successfully
 */
sl_status_t fs_sd_append_to_file(const char   *file_path,
                                 const void   *data,
                                 uint32_t      data_size)
{
  FRESULT fs_res;
  UINT bytes_written;

  /* === Argument validation === */
  if ((file_path == NULL) || (data == NULL) || (data_size == 0)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  if (!sd_mounted) {
    return SL_STATUS_NOT_READY;
  }

  /* Open existing file for write */
  fs_res = f_open(&File, file_path, FA_WRITE | FA_OPEN_EXISTING);
  if (fs_res != FR_OK) {
    app_log_warning("File doesn't exist to f_open failed for %s : %d\r\n", file_path, fs_res);
    return SL_STATUS_FAIL;
  }

  /* Move file pointer to end */
  fs_res = f_lseek(&File, f_size(&File));
  if (fs_res != FR_OK) {
    app_log_debug("Error f_lseek failed for %s : %d\r\n", file_path, fs_res);
    f_close(&File);
    return SL_STATUS_FAIL;
  }

  /* Write data */
  fs_res = f_write(&File, data, data_size, &bytes_written);
  if ((fs_res != FR_OK) || (bytes_written != data_size)) {
    app_log_debug("Error f_write failed for %s : %d written %u Bytes\r\n", file_path, fs_res, bytes_written);
    f_close(&File);
    return SL_STATUS_FAIL;
  }

  /* Flush cached data to the medium */
  fs_res = f_sync(&File);
  if (fs_res != FR_OK) {
    app_log_critical("Error critical f_sync failed for %s : %d\r\n", file_path, fs_res);
    app_assert_status(fs_res);
    return SL_STATUS_FAIL;

  }

  fs_res = f_close(&File);
  if (fs_res != FR_OK) {
    app_log_critical("Error critical f_close failed for %s : %d\r\n", file_path, fs_res);
    app_assert_status(fs_res);
    return SL_STATUS_FAIL;
  }

  app_log_debug("Append success: %lu bytes\r\n", (unsigned long)bytes_written);

  return SL_STATUS_OK;
}
