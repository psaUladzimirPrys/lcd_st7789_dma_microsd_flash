/***************************************************************************//**
 * sd_card.c
 *
 *  Created on: 15 Jan. 2026.
 *      Author: priss
*******************************************************************************/
#include <file_storage.h>
#include <stdio.h>
#include <string.h>
#include "sl_sleeptimer.h"

/***************************************************************************//**
*  File storage of SD card configuration section declaration global variables
*******************************************************************************/

static mikroe_spi_handle_t app_spi_instance = NULL;

#if !FF_FS_NORTC && !FF_FS_READONLY
  static DWORD sd_fatfs_time_data;
#endif


//static const char str[] = "Silabs SD Card I/O Example via SPI!\r\n";

BYTE f_work[FF_MAX_SS]; // Work area (larger is better for processing time)

static FATFS FatFs;                   // FatFS object for mounting.
static FIL File;                      // File object structure

//static BYTE work_buffer[FF_MAX_SS];   // Working buffer for operations (e.g., f_mkfs).
static bool sd_mounted = false;       // Mounting flag.



sl_status_t fs_sd_time_init(void)
{

  sl_status_t sl_status_code = SL_STATUS_OK;

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
  app_printf("\nCurrent time is %lu/%lu/%lu %2lu:%02lu:%02lu.\n\n",
             (sd_fatfs_time_data >> 25) + 1980,
             (sd_fatfs_time_data >> 21) & 0x0f,
             (sd_fatfs_time_data >> 16) & 0x1f,
             (sd_fatfs_time_data >> 11) & 0x1f,
             (sd_fatfs_time_data >> 5) & 0x3f,
             (sd_fatfs_time_data << 1) & 0x1f);

  sl_status_code = sc;
#endif

  return sl_status_code;
}


sl_status_t fs_sd_init(void)
{
  sl_status_t sl_status_code = SL_STATUS_OK;
  FRESULT     ff_res_code;
  sl_gpio_t pinport;
  uint32_t bitRate;

  sd_mounted = false;

  memset((void *)&FatFs, 0x00, sizeof(FATFS));
  memset((void *)&File, 0x00, sizeof(File));
  memset((void *)&f_work[0], 0x00, sizeof(f_work));

#if (defined(SLI_SI917))
  app_spi_instance = &gspi_instance;
#else
  app_spi_instance = sl_spidrv_mikroe_handle;
#endif

  // Initialising SPI for SD (using MikroE config)
  app_printf("Initializing SD card...\r\n");
  sl_status_code = sd_card_spi_init(app_spi_instance);  // Assuming that app_spi_instance is a global object
  if (sl_status_code != SL_STATUS_OK) {
    app_printf("SD card SPI init failed: 0x%lx\r\n", sl_status_code);
    return sl_status_code;
  }


  /* The MISO pin is driven by a tri-stated output from the SDcard.
   * Therefore, the MISO pin should be configured as a pull-up input
   * to hold that input into a known state when the SDcard is not selected.
   */
  pinport.port = sl_spidrv_mikroe_handle->initData.portRx;
  pinport.pin = sl_spidrv_mikroe_handle->initData.pinRx;

  /* @ToDo must remove after pins validation  It was been added by UP*/
  if ((pinport.port != SL_GPIO_PORT_C) && (pinport.pin != 1)) {
    app_printf("SD card SPI MISO port failed: 0x%lx\r\n", sl_status_code);
    return SL_STATUS_FAIL;
  }


  sl_status_code = sl_gpio_set_pin_mode(&pinport, SL_GPIO_MODE_INPUT_PULL, 1);
  if (sl_status_code != SL_STATUS_OK) {
    app_printf("SD card SPI MISO pin failed: 0x%lx\r\n", sl_status_code);
    return sl_status_code;
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
       app_printf("SD card SPI CS port failed: 0x%lx\r\n", sl_status_code);
       return SL_STATUS_FAIL;
    }

    sl_status_code = sl_gpio_set_pin_mode(&pinport, SL_GPIO_MODE_PUSH_PULL, 1);
    if (sl_status_code != SL_STATUS_OK) {
       app_printf("SD card SPI CS pin failed: 0x%lx\r\n", sl_status_code);
       return sl_status_code;
    }

  }

  // Initialising the disk and mounting FatFS
  // Give a work area to the default drive
  ff_res_code = f_mount(&FatFs, "", 1);  /* Mount the default drive */
  if (ff_res_code != FR_OK) {
    app_printf("Disk initialize or disk mount failed: %d\r\n", ff_res_code); // Failed to mount SD
    return SL_STATUS_FAIL;
  }

  if ( SPIDRV_GetBitrate(sl_spidrv_mikroe_handle, &bitRate) == ECODE_EMDRV_SPIDRV_OK ) {
      app_log("SD Card SPI bitrate=%luMHZ \r\n",bitRate);
   } else {
      app_log("SD Card SPI bitrate ERROR\r\n");
   }


  sd_mounted = true;
  app_printf("SD card initialized and mounted.\r\n");


  return sl_status_code;
}

sl_status_t fs_sd_disk_volume_status(void)
{
  static const char *fst[] = { "", "FAT12", "FAT16", "FAT32", "exFAT" };
  static char path[] = {""};
  FATFS *pfs;
  FRESULT ff_res_code;
  DWORD fre_clust;


  if (!sd_mounted) {
      app_printf("SD card logical drive is not mounted.\r\n");
      return SL_STATUS_NOT_READY;
  }

  pfs = &FatFs;

  // Show logical drive status
  ff_res_code = f_getfree(path, &fre_clust, &pfs);
  if (ff_res_code != FR_OK) {
      app_printf("Disk volume status failed: %d\r\n", ff_res_code);
      return SL_STATUS_FAIL;
  }

  app_printf("-------------- Volume status --------------\r\n");
  app_printf(("FAT type = %s\r\nBytes/Cluster = %lu\r\nNumber of FATs = %u\r\n"
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


sl_status_t fs_sd_write_file(const char *file_path, const void *data, uint32_t size)
{

  FRESULT ff_res_code;
  UINT bytes_written;

  if ((file_path == NULL) || (data == NULL)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  if (!sd_mounted) {
      return SL_STATUS_NOT_READY;
  }

  // Open file to write
  ff_res_code = f_open(&File, file_path, FA_WRITE | FA_CREATE_ALWAYS);

  if (ff_res_code != FR_OK) {
      app_printf("Error f_open() failed for %s: %d\r\n", file_path, ff_res_code);
      return SL_STATUS_FAIL;
  }
  // Write a data
  ff_res_code = f_write(&File, data, size, &bytes_written);

  if (ff_res_code != FR_OK || bytes_written != size) {
    f_close(&File);
    app_printf("Failed writing data to SD card! Data size = %lu\r\n", size);

    return SL_STATUS_FAIL;
  }

  app_printf("Write a data to SD card success! Bytes writen = %d\r\n", bytes_written);

  ff_res_code = f_close(&File);  // Close file
  app_assert_status(ff_res_code);

  return SL_STATUS_OK;
}

sl_status_t fs_sd_read_file(const char *file_path, uint16_t *buffer, uint32_t buffer_size)
{
  UINT bytes_read;
  FRESULT ff_res_code;

  if ((file_path == NULL) || (buffer == NULL) || (buffer_size == 0)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  if (!sd_mounted) {
      return SL_STATUS_NOT_READY;
  }

  ff_res_code = f_open(&File, file_path, FA_READ | FA_OPEN_EXISTING);
  if (ff_res_code != FR_OK) {
    app_printf("Error f_open() failed for %s : %d\r\n", file_path, ff_res_code);
    return SL_STATUS_FAIL;
  }

  ff_res_code = f_read(&File, buffer, buffer_size, &bytes_read);
  if (ff_res_code != FR_OK || bytes_read != buffer_size) {
     app_printf("Error f_read() failed for %s : %d, read %u bytes\r\n", file_path, ff_res_code, bytes_read);
     f_close(&File);

     return SL_STATUS_FAIL;
  }

  ff_res_code = f_close(&File); // Always close!
  app_assert_status(ff_res_code);
  app_printf("Read a data from SD card success! Bytes read = %d\r\n", bytes_read);

  return SL_STATUS_OK;

}

sl_status_t fs_sd_read_image(uint8_t index, uint16_t *buffer, uint32_t buffer_size)
{
  FRESULT res;
  UINT bytes_read;
  char filename[20];

  if ((buffer == NULL) || (buffer_size == 0)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Form the file name, for example "image0.bin" (assuming raw RGB565 files on SD)
  sprintf(filename, "img%d.bin", index);

  res = f_open(&File, filename, FA_READ);
  if (res != FR_OK) {
    app_printf("Error f_open() failed for %s : %d\r\n", filename, res);
    return SL_STATUS_FAIL;
  }

  // Reading into buffer (assuming file size = buffer_size)
  res = f_read(&File, buffer, buffer_size, &bytes_read);
  if (res != FR_OK || bytes_read != buffer_size) {
    app_printf("Error f_read() failed for %s : %d, read %u bytes\r\n", filename, res, bytes_read);
    f_close(&File);
    return SL_STATUS_FAIL;
  }

  f_close(&File);
  app_printf("Read image %s from SD.\r\n", filename);

  return SL_STATUS_OK;
}

// Unmount SDCARD
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

sl_status_t fs_sd_get_file_size(const char *file_path,  uint32_t *file_size)
{

  FRESULT ff_res_code;

  if ((file_path == NULL) || (file_size == NULL)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  if (!sd_mounted) {
    return SL_STATUS_NOT_READY;
  }

  ff_res_code = f_open(&File, file_path, FA_READ | FA_OPEN_EXISTING);
  if (ff_res_code != FR_OK) {
    app_printf("Error f_open failed for %s : %d\r\n", file_path, ff_res_code);
    return SL_STATUS_FAIL;
  }

  *file_size = f_size(&File);

  ff_res_code = f_close(&File); // Always close!
  app_assert_status(ff_res_code);
  app_printf("File size read success: %lu bytes\r\n", (unsigned long)*file_size);

  return SL_STATUS_OK;
}

sl_status_t fs_sd_append_to_file(const char   *file_path,
                                 const uint8_t *data,
                                 uint32_t       data_size)
{
  FRESULT ff_res;
  UINT bytes_written;

  /* === Argument validation === */
  if ((file_path == NULL) || (data == NULL) || (data_size == 0)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  //app_assert(file_path != NULL);
  //app_assert(data != NULL);
  //app_assert(data_size > 0);

  if (!sd_mounted) {
    return SL_STATUS_NOT_READY;
  }

  /* Open existing file for write */
  ff_res = f_open(&File, file_path, FA_WRITE | FA_OPEN_EXISTING);
  if (ff_res != FR_OK) {
    app_printf("File doesn't exist to f_open() failed for %s : %d\r\n",
               file_path, ff_res);
    return SL_STATUS_FAIL;
  }

  /* Move file pointer to end */
  ff_res = f_lseek(&File, f_size(&File));
  if (ff_res != FR_OK) {
    app_printf("Error f_lseek() failed for %s : %d\r\n", file_path, ff_res);
    f_close(&File);
    return SL_STATUS_FAIL;
  }

  /* Write data */
  ff_res = f_write(&File, data, data_size, &bytes_written);
  if ((ff_res != FR_OK) || (bytes_written != data_size)) {
    app_printf("Error f_write() failed for %s : %d, written=%uBytes\r\n", file_path
               ,ff_res, bytes_written);
    f_close(&File);
    return SL_STATUS_FAIL;
  }

  /* Flush cached data to the medium */
  ff_res = f_sync(&File);
  app_assert_status(ff_res);

  ff_res = f_close(&File);
  app_assert_status(ff_res);

  app_printf("Append success: %lu bytes written\r\n", (unsigned long)bytes_written);

  return SL_STATUS_OK;
}
