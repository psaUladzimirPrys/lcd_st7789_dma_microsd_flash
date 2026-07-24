/***************************************************************************//**
 * sd_card.c
 *
 *  Created on: 15 Jan. 2026.
 *      Author: priss
*******************************************************************************/
/*==========================================================================*/
/*        I N C L U D E S                                                   */
/*==========================================================================*/
#include <stdio.h>
#include <string.h>
#include "sl_sleeptimer.h"
#include "file_storage.h"
#include "flash_storage.h"
#include "calculations.h"
#include "ble_communication.h"

/***************************************************************************//**
*  File storage of SD card configuration section declaration global variables
*******************************************************************************/
/*=======================================================================*/
/* L O C A L   D E F I N I T I O N S                     */
/*=======================================================================*/
#define FS_LOG_PRINTF_BUFFER_SIZE   128
#define FS_LOG_RING_BUFFER_SIZE     512
#define FS_LOG_FLUSH_THRESHOLD      256
#define FS_LOG_FILE_PATH           "log.txt"

#define FS_LOG_ENABLE          true
#define FS_LOG_DISABLE         false

#define MAX_FILE_OFFSET  2088960000UL // Максимально допустимое смещение в байтах

/*=======================================================================*/
/* L O C A L   S Y M B O L   D E C L A R A T I O N S                     */
/*=======================================================================*/

static mikroe_spi_handle_t app_spi_instance = NULL;

#if !FF_FS_NORTC && !FF_FS_READONLY
  static DWORD sd_fatfs_time_data;
#endif

static BYTE  F_work[FF_MAX_SS];         // Work file buffer area (larger is better for processing time)
static FATFS FatFs;                     // FatFS object for mounting.
static FIL   File;                      // File object structure

// Таблица для хранения цепочки кластеров.
// Размер массива определяет, сколько фрагментов файла мы можем отследить.
DWORD lseek_tbl[128];
// Путь к файлу кэша карты кластеров
const char *historical_db_cltbl = "sys/cltb.dat";

//static BYTE work_buffer[FF_MAX_SS];   // Working buffer for operations (e.g., f_mkfs).
static bool sd_mounted = false;         // Mounting flag.

static volatile bool g_log_enabled = FS_LOG_DISABLE; //Logging flag - Enable/Disable
static uint8_t fslog_flush_buffer[FS_LOG_FLUSH_THRESHOLD];
static char fslog_printf_buffer[FS_LOG_PRINTF_BUFFER_SIZE];

static uint8_t ring_buffer[FS_LOG_RING_BUFFER_SIZE];
static volatile uint16_t ring_head = 0;
static volatile uint16_t ring_tail = 0;
static volatile uint16_t ring_used = 0;
static const char *level_str[] =
{
  "[ERR ] ",
  "[WARN] ",
  "[INFO] ",
  "[DBG ] "
};

/*=======================================================================*/
/*       L O C A L   F U N C T I O N   D E C L A R A T I O N             */
/*=======================================================================*/
void log_flush_task(void);
void log_flush_all(void);
void log_write(const char *str);

// Локальные прототипы функций для таблицы кластеров
static bool load_cltbl_from_sd(void);
static void save_cltbl_to_sd(void);
/*=======================================================================*/
/* L O C A L   F U N C T I O N   P R O T O T Y P E S                     */
/*=======================================================================*/

static inline uint16_t rb_free(void)
{
  return FS_LOG_RING_BUFFER_SIZE - ring_used;
}

/*=======================================================================*/
/* G L O B A L   F U N C T I O N   D E C L A R A T I O N                 */
/*=======================================================================*/

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

  sl_spidrv_init_instances();

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

  app_printf("File %s, size = %lu\r\n", filepath, f_size);

}

/*
 * Function:    log_write
 * Arguments:   const char *str
 * Description:
 *   Writes a string to the ring buffer for later flushing to SD card.
 *
 * Return Message: void
 */
void log_write(const char *str)
{
  uint16_t len;

  if (!g_log_enabled)
    return;

  if (!sd_mounted) {
      app_log_warning("SD card logical drive is not mounted.\r\n");
      return;
  }

  if (str == NULL) {
    return;
  }

  len = strlen(str);

  /* Drop data if buffer overflow */
  if (len > rb_free()) {
    return;
  }

  for (uint16_t i = 0; i < len; i++) {

    ring_buffer[ring_head++] = (uint8_t)str[i];

    if (ring_head >= FS_LOG_RING_BUFFER_SIZE) {
        ring_head = 0;
    }
  }

  ring_used += len;
}

/*
 * Function:    log_flush_task
 * Arguments:   void
 * Description:
 *   Periodic task that flushes accumulated logs to the SD card file.
 *
 * Return Message: void
 */
void log_flush_task(void)
{
  uint16_t chunk;

  if (ring_used < FS_LOG_FLUSH_THRESHOLD) {
    return;
  }

  chunk = (ring_used > FS_LOG_FLUSH_THRESHOLD)
          ? FS_LOG_FLUSH_THRESHOLD
          : ring_used;

  for (uint16_t i = 0; i < chunk; i++) {
    fslog_flush_buffer[i] = ring_buffer[ring_tail++];
    if (ring_tail >= FS_LOG_RING_BUFFER_SIZE) {
        ring_tail = 0;
    }
  }

  ring_used -= chunk;

  if (!sd_mounted) {
      app_log_warning("SD card logical drive is not mounted.\r\n");
      app_assert_status(SL_STATUS_FAIL); // Loop forever for debugging
      return;
  }

  if (fs_sd_append_to_file(FS_LOG_FILE_PATH, fslog_flush_buffer, chunk) != SL_STATUS_OK) {
    app_log("Append to file: Failed\r\n");
    app_assert_status(SL_STATUS_FAIL); // Loop forever for debugging
  }

}

void log_flush_all(void)
{
  uint8_t temp_buf[FS_LOG_FLUSH_THRESHOLD];

  if (!sd_mounted) {
      app_log_warning("SD card logical drive is not mounted.\r\n");
      return;
  }

  while (ring_used > 0)
  {
    uint16_t chunk = (ring_used > sizeof(temp_buf)) ? sizeof(temp_buf) : ring_used;

    for (uint16_t i = 0; i < chunk; i++)
    {
        temp_buf[i] = ring_buffer[ring_tail++];
      if (ring_tail >= FS_LOG_RING_BUFFER_SIZE)
        ring_tail = 0;
    }

    ring_used -= chunk;

    if (fs_sd_append_to_file(FS_LOG_FILE_PATH, temp_buf, chunk) != SL_STATUS_OK) {
      app_log("Append to file: Failed\r\n");
      app_assert_status(SL_STATUS_FAIL); // Loop forever for debugging
    }
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
    .year = 2026,
    .month = 1,
    .month_day = 1,
    .hour = 1,
    .min = 0,
    .sec = 0,
  };


  sc = sl_sleeptimer_set_datetime(&date_time);
  app_assert_status(sc);

  sd_fatfs_time_data = get_fattime();
  app_log_info("\nCurrent FatFS time is %lu/%lu/%lu %2lu:%02lu:%02lu.\n\n",
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
    if ((pinport.port != SL_GPIO_PORT_C) && (pinport.pin != 4)) {
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

bool fslog_IsEnabled(void)
{
  return (bool)(g_log_enabled == FS_LOG_ENABLE);
}

void fslog_TurnOn(void)
{
  g_log_enabled = FS_LOG_ENABLE;
}

void fslog_TurnOff(void)
{
  if (!g_log_enabled)
    return;

  g_log_enabled = FS_LOG_DISABLE;

  /* Flush everything unconditionally */
  log_flush_all();
}

void fslog_Update(void)
{
   log_flush_task();
}

void log_vprintf(log_level_t level, const char *fmt, va_list args)
{

  int len;

  if (!g_log_enabled)
    return;

  if (level > LOG_LEVEL_ALL)
    return;

  if(level != LOG_LEVEL_ALL) { /* prefix */
    log_write((const char*)level_str[level]);
  }

  /* format */
  len = vsnprintf((char *)&fslog_printf_buffer[0], sizeof(fslog_printf_buffer), fmt, args);
  if (len <= 0) {
    return;
  }

  if (len > (int)sizeof(fslog_printf_buffer)) { /* truncate buffer[LOG_PRINTF_BUFFER_SIZE] overflow */
    len = sizeof(fslog_printf_buffer);
  }

  log_write((const char*)&fslog_printf_buffer[0]);

}

void fslog_printf(log_level_t level, const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  log_vprintf(level, fmt, args);
  va_end(args);
}

void fslog_Init(void)
{

  g_log_enabled = FS_LOG_DISABLE;

  // Initialize ring buffer for logging
  memset((void *)&ring_buffer[0],0x00, sizeof(ring_buffer));
  ring_head = 0;
  ring_tail = 0;
  ring_used = 0;

}


//==============================================================================================
/**
 * @brief Функция загрузки таблицы быстрого поиска из SD.
 * Открывает файл карты "sys/cltb.dat", считывает её в массив и сразу закрывает.
 * @return true в случае успеха, false если файла нет или произошла ошибка
 */
static bool load_cltbl_from_sd(void)
{
  FIL map_file;
  FRESULT fr;
  UINT br;

  // Открываем ТОЛЬКО вспомогательный файл карты. (Основной файл БД закрыт!)
  fr = f_open(&map_file, historical_db_cltbl, FA_READ);
  if (fr != FR_OK) {
    return false; // Файла нет или ошибка открытия
  }

  fr = f_read(&map_file, lseek_tbl, sizeof(lseek_tbl), &br);
  f_close(&map_file); // Мгновенно закрываем дескриптор!

  if (fr == FR_OK && br == sizeof(lseek_tbl)) {
    app_log("HIST DB: Cltbl successfully loaded from SD!\r\n");
    return true;
  }

  return false;
}

/**
 * @brief Функция сохранения таблицы быстрого поиска на SD.
 * Создает папку "sys" (если её нет), записывает таблицу из ОЗУ и сразу закрывает файл.
 */
static void save_cltbl_to_sd(void)
{
  FIL map_file;
  FRESULT fr;
  UINT bw;

  // Создаем директорию "sys", если её еще нет (FatFS сама проверит существование)
  f_mkdir("sys");

  // Открываем ТОЛЬКО файл карты на запись. (Основной файл БД закрыт!)
  fr = f_open(&map_file, historical_db_cltbl, FA_WRITE | FA_CREATE_ALWAYS);
  if (fr == FR_OK) {
    fr = f_write(&map_file, lseek_tbl, sizeof(lseek_tbl), &bw);
    f_close(&map_file); // Мгновенно закрываем дескриптор!

    if (fr == FR_OK && bw == sizeof(lseek_tbl)) {
      app_log("HIST DB: Cltbl successfully saved to SD (%s)\r\n", historical_db_cltbl);
    } else {
      app_log("HIST DB: Failed to write cltbl to file (error: %d)\r\n", fr);
    }
  } else {
    app_log("HIST DB: Failed to open cltbl for saving (error: %d)\r\n", fr);
  }
}



//==============================================================================================
//                      cached work with TIP ID
//==============================================================================================

const char *tip_id_file = "vtip/vtip.dat";

// Глобальная или статическая переменная для хранения индекса активной записи '*'
// Значение 0xFFFFFFFF означает, что активного наконечника нет или файл пуст
static uint32_t cached_asterisk_index = 0xFFFFFFFF;

/**
 * @brief Инициализирует кэш индекса активного наконечника.
 * Вызывается ОДИН РАЗ при старте системы после монтирования SD.
 */
sl_status_t fs_sd_init_tip_cache(void)
{
  FRESULT fs_res;
  UINT bytes_read = 0;
  uint32_t struct_size = sizeof(tip_save_info_t);
  uint32_t current_index = 0;

  cached_asterisk_index = 0xFFFFFFFF; // Сброс кэша

  if (!sd_mounted) return SL_STATUS_NOT_READY;

  fs_res = f_open(&File, tip_id_file, FA_READ);
  if (fs_res != FR_OK) {
    app_log("TIP CACHE: File not found or empty\r\n");
    return SL_STATUS_OK; // Новый файл создастся при первой валидации
  }

  UINT bytes_to_read = FF_MAX_SS; // Блочное чтение по 512 байт из вашего оригинального кода

  while (!f_eof(&File) && (current_index < TIP_MAX_RECORDS)) {
    fs_res = f_read(&File, F_work, bytes_to_read, &bytes_read);
    if (fs_res != FR_OK || bytes_read == 0) break;

    uint32_t structs_read = bytes_read / struct_size;
    for (uint32_t i = 0; i < structs_read; i++) {
      if (current_index >= TIP_MAX_RECORDS) break;

      tip_save_info_t *record = (tip_save_info_t *)&F_work[i * struct_size];

      if (record->last_rec == '*') {
        cached_asterisk_index = current_index;
        f_close(&File);
        app_log("TIP CACHE: Found active '*' at index %lu\r\n", cached_asterisk_index);
        return SL_STATUS_OK;
      }
      current_index++;
    }
  }

  f_close(&File);
  app_log("TIP CACHE: Active record '*' not found\r\n");
  return SL_STATUS_OK;
}

sl_status_t fs_sd_is_tip_valid_fast(const void *struct_ptr, size_t struct_size, tip_state_t *tip_state_ptr)
{
  sl_status_t sl_status = SL_STATUS_OK;
  FRESULT fs_res = FR_OK;
  UINT bytes_read = 0;
  UINT bytes_written = 0;

  if (struct_ptr == NULL || struct_size != sizeof(tip_save_info_t)) {
      return SL_STATUS_INVALID_PARAMETER;
  }

  if (!sd_mounted) {
    app_log("TIP VALIDATION: SD not mounted\r\n");
    return SL_STATUS_NOT_READY;
  }

  fs_res = f_open(&File, tip_id_file, FA_READ | FA_WRITE | FA_OPEN_ALWAYS);
  if (fs_res != FR_OK) {
      app_log("TIP VALIDATION: f_open failed for %s: %d\r\n", tip_id_file, fs_res);
      return SL_STATUS_FAIL;
  }

  const tip_save_info_t *incoming = (const tip_save_info_t *)struct_ptr;

  // === ШАГ 0: БЫСТРАЯ ПРОВЕРКА ЧЕРЕЗ КЭШ ===
  if (cached_asterisk_index != 0xFFFFFFFF) {
      uint32_t active_offset = cached_asterisk_index * struct_size;
      fs_res = f_lseek(&File, active_offset);
      if (fs_res == FR_OK) {
          tip_save_info_t active_record;
          f_read(&File, &active_record, struct_size, &bytes_read);

          if (bytes_read == struct_size && memcmp(incoming->id, active_record.id, TIP_ID_LEN) == 0) {
              // Если ID совпал и наконечник уже использован ('U'), выходим мгновенно за 2 мс
              if (active_record.state == 'U') {
                  *tip_state_ptr = TIP_STATE_USED;
                  app_log("TIP VALIDATION: Cache hit! ID matches and it is USED\r\n");
                  f_close(&File);
                  return SL_STATUS_OK;
              }
              // Если ID совпал, но он еще НЕ использован ('V') -> не выходим, спускаемся ниже для обновления
              app_log("TIP VALIDATION: Cache hit! ID matches but it is NOT USED. Proceeding to update...\r\n");
          }
      }
  }

  // === ШАГ 1: СКВОЗНОЙ ПОИСК ПО ФАЙЛУ ===
  UINT bytes_to_read = 512;
  bool asterisk_found = false;
  uint32_t asterisk_index = 0;
  bool empty_slot_found = false;
  uint32_t first_empty_index = 0;
  bool global_id_match_found = false;
  uint32_t current_index = 0;

  f_lseek(&File, 0);

  while (!f_eof(&File) && (current_index < TIP_MAX_RECORDS)) {
    fs_res = f_read(&File, F_work, bytes_to_read, &bytes_read);
    if (fs_res != FR_OK || bytes_read == 0) {
      break;
    }

    uint32_t structs_read = bytes_read / struct_size;
    for (uint32_t i = 0; i < structs_read; i++) {
        if (current_index >= TIP_MAX_RECORDS) break;

        tip_save_info_t *record = (tip_save_info_t *)&F_work[i * struct_size];

        if (record->last_rec == '*') {
            asterisk_found = true;
            asterisk_index = current_index;
        }

        if (record->last_rec == '.' && !empty_slot_found) {
            empty_slot_found = true;
            first_empty_index = current_index;
        }

        if (record->last_rec != '.') {
            if (memcmp(incoming->id, record->id, TIP_ID_LEN) == 0) {
                global_id_match_found = true;
            }
        }
        current_index++;
    }
  }

  // === ШАГ 2: ЛОГИКА ПРИНЯТИЯ РЕШЕНИЯ И ЗАПИСИ ===
  uint32_t target_write_index = 0;
  bool need_clear_old_asterisk = false;
  bool proceed_with_write = true;
  if (global_id_match_found) {
      tip_save_info_t check_record;
      // Повторно пробегаем по найденным индексам, чтобы определить состояние совпавшего ID
      for (uint32_t idx = 0; idx < current_index; idx++) {
          f_lseek(&File, idx * struct_size);
          f_read(&File, &check_record, struct_size, &bytes_read);
          if (memcmp(incoming->id, check_record.id, TIP_ID_LEN) == 0) {
              if (check_record.state == 'U') {
                  // 1. ID совпал и уже использован ('U') -> возвращаем USED, ничего не пишем
                  *tip_state_ptr = TIP_STATE_USED;
                  proceed_with_write = false;
                  app_log("TIP VALIDATION: ID matches an already USED tip\r\n");
              } else if (check_record.state == 'V') {
                  // 2. ID совпал, но еще не использован ('V') -> возвращаем VALID, просто перезаписываем на месте
                  *tip_state_ptr = TIP_STATE_VALID;
                  target_write_index = idx;
                  if (asterisk_found && (asterisk_index != idx)) {
                      need_clear_old_asterisk = true;
                  }
                  app_log("TIP VALIDATION: ID matches active 'V' tip. Updating same slot %lu\r\n", target_write_index);
              }
              break;
          }
      }
  }
  else {
      // ID в базе не найден вообще -> это новый наконечник
      *tip_state_ptr = TIP_STATE_VALID;

      if (asterisk_found) {
          tip_save_info_t active_record;
          f_lseek(&File, asterisk_index * struct_size);
          f_read(&File, &active_record, struct_size, &bytes_read);
          if (active_record.state == 'V') {
              // 3. ID НОВЫЙ, а текущий наконечник (*) еще НЕ использован ('V') -> просто перезаписываем на его же место
              target_write_index = asterisk_index;
              need_clear_old_asterisk = false; // Затрем физически, старая звезда исчезнет сама
              app_log("TIP VALIDATION: New ID, but active tip wasn't used. Overwriting slot %lu\r\n", target_write_index);
          } else {
              // 4. Текущий наконечник (*) уже был использован ('U') -> снимаем с него звезду и смещаем кольцо
              target_write_index = asterisk_index + 1;
              if (target_write_index >= TIP_MAX_RECORDS) {
                  target_write_index = 0;
              }
              need_clear_old_asterisk = true;
              app_log("TIP VALIDATION: New ID. Moving ring buffer to slot %lu\r\n", target_write_index);
          }
      } else {
          // База пустая
          target_write_index = empty_slot_found ? first_empty_index : 0;
          app_log("TIP VALIDATION: First record. Writing to slot %lu\r\n", target_write_index);
      }

  }

  // Выполняем запись на SD карту, если это требуется по логике
  if (proceed_with_write) {
      // Снимаем звезду с предыдущей записи
      if (need_clear_old_asterisk && asterisk_found) {
          uint32_t old_asterisk_offset = asterisk_index * struct_size;
          fs_res = f_lseek(&File, old_asterisk_offset);
          if (fs_res == FR_OK) {
              tip_save_info_t old_record;
              f_read(&File, &old_record, struct_size, &bytes_read);
              old_record.last_rec = '#';

              f_lseek(&File, old_asterisk_offset);
              f_write(&File, &old_record, struct_size, &bytes_written);
          }
      }

      // Формируем и записываем структуру наконечника
      tip_save_info_t record_to_write;
      memcpy(&record_to_write, incoming, struct_size);
      record_to_write.last_rec   = '*';
      record_to_write.state      = 'V';
      record_to_write.end_marker = '>';

      uint32_t write_offset = target_write_index * struct_size;
      fs_res = f_lseek(&File, write_offset);
      if (fs_res == FR_OK) {
          fs_res = f_write(&File, &record_to_write, struct_size, &bytes_written);
          if (fs_res != FR_OK || bytes_written != struct_size) {
              app_log("TIP VALIDATION: Write failed\r\n");
              sl_status = SL_STATUS_FAIL;
          } else {
              // ОБНОВЛЯЕМ КЭШ В ОЗУ
              cached_asterisk_index = target_write_index;
              app_log("TIP VALIDATION: Slot %lu recorded with '*'. Cache updated.\r\n", target_write_index);
          }
      } else {
          sl_status = SL_STATUS_FAIL;
      }
  }

  f_close(&File);
  return sl_status;
}

sl_status_t fs_sd_mark_last_tip_used_fast(void)
{
  sl_status_t sl_status = SL_STATUS_OK;
  FRESULT fs_res = FR_OK;
  UINT bytes_read = 0;
  UINT bytes_written = 0;
  uint32_t struct_size = sizeof(tip_save_info_t);

  if (!sd_mounted) return SL_STATUS_NOT_READY;

  // Если в кэше пусто — гасить некого, выходим мгновенно
  if (cached_asterisk_index == 0xFFFFFFFF) {
      app_log("TIP STORAGE: Fast abort - Active record '*' not found in cache\r\n");
      return SL_STATUS_NOT_FOUND;
  }

  fs_res = f_open(&File, tip_id_file, FA_READ | FA_WRITE); // Убрали FA_OPEN_ALWAYS, файл точно есть
  if (fs_res != FR_OK) return SL_STATUS_FAIL;

  // Прямой переход по индексу из ОЗУ без циклов чтения всего файла
  uint32_t offset = cached_asterisk_index * struct_size;
  fs_res = f_lseek(&File, offset);
  if (fs_res == FR_OK) {
      tip_save_info_t record;
      f_read(&File, &record, struct_size, &bytes_read);

      // Проверяем, что там действительно валидный наконечник и маркер '*' совпадает
      if (bytes_read == struct_size && record.last_rec == '*' && record.state == 'V') {
          record.state = 'U'; // Меняем состояние на Used

          f_lseek(&File, offset);
          fs_res = f_write(&File, &record, struct_size, &bytes_written);
          if (fs_res == FR_OK && bytes_written == struct_size) {
              app_log("TIP STORAGE: Slot %lu marked as USED via cache\r\n", cached_asterisk_index);
              // Поскольку активного наконечника больше нет, сбрасываем кэш
              cached_asterisk_index = 0xFFFFFFFF;
          }
      } else {
          app_log("TIP STORAGE: Cache mismatch or record state invalid\r\n");
      }
  }

  f_close(&File);
  return sl_status;
}

//=============================================================================================
//                          performance record write
//=============================================================================================
#define PERF_RECORD_SIZE        32
#define PERF_MAX_RECORDS       500

const char *perf_log_file = "recs/perf.dat";
/**
 * @brief Записывает строку PERFORMANCE в циклический буфер на SD сплошным текстом без sprintf.
 * Строка добивается символами '>' до 32 байт. Изначальный файл заполнен '.'.
 * Функция автономна и сама находит место для записи без использования кэша в ОЗУ.
 */
sl_status_t fs_sd_write_performance_test_date(void)
{
  sl_status_t sl_status = SL_STATUS_OK;
  FRESULT fs_res = FR_OK;
  UINT bytes_written = 0;
  UINT bytes_read = 0;
  char write_buffer[PERF_RECORD_SIZE];
  char check_buf[PERF_RECORD_SIZE];
  sl_sleeptimer_date_t current_date;

  // Проверка монтирования карты (используется ваша глобальная переменная sd_mounted)
  if (!sd_mounted) {
    app_log("PERF WRITE: SD not mounted\r\n");
    return SL_STATUS_NOT_READY;
  }

  // Получаем текущее системное время из sleeptimer
  sl_status = sl_sleeptimer_get_datetime(&current_date);
  if (sl_status != SL_STATUS_OK) {
    app_log("PERF WRITE: Failed to get datetime: %lu\r\n", sl_status);
    return sl_status;
  }

  // Убедимся, что папка recs существует
  f_mkdir("recs");

  // Открываем файл для чтения и записи. Если файла нет, FA_OPEN_ALWAYS его создаст.
  // Используется глобальный объект File (из контекста вашего file_storage.c)
  fs_res = f_open(&File, perf_log_file, FA_READ | FA_WRITE | FA_OPEN_ALWAYS);
  if (fs_res != FR_OK) {
    app_log("PERF WRITE: f_open failed: %d\r\n", fs_res);
    return SL_STATUS_FAIL;
  }

  int32_t active_asterisk_index = -1;
  int32_t first_dot_index = -1;
  uint32_t current_index = 0;
  uint32_t target_index = 0;

  // ШАГ 1: Поиск за один проход актуальной звездочки '*' и первой пустой точки '.'
  while (current_index < PERF_MAX_RECORDS && !f_eof(&File)) {
    fs_res = f_read(&File, check_buf, PERF_RECORD_SIZE, &bytes_read);
    if (fs_res != FR_OK || bytes_read < PERF_RECORD_SIZE) break;

    if (check_buf[0] == '*') {
      active_asterisk_index = current_index;
    }
    if (check_buf[0] == '.' && first_dot_index == -1) {
      first_dot_index = current_index;
    }
    current_index++;
  }

  // ШАГ 2: Определение целевого индекса (target_index) на основе анализа файла
  if (active_asterisk_index != -1) {
    // 2a. Звездочка найдена. Меняем её на '#'
    uint32_t old_offset = active_asterisk_index * PERF_RECORD_SIZE;
    fs_res = f_lseek(&File, old_offset);
    if (fs_res == FR_OK) {
      char old_first_char = '#';
      f_write(&File, &old_first_char, 1, &bytes_written);
    }

    // Новую запись пишем в следующий слот по кругу
    target_index = active_asterisk_index + 1;
    if (target_index >= PERF_MAX_RECORDS) {
      target_index = 0;
    }
  } else if (first_dot_index != -1) {
    // 2b. Звездочки нет (файл чистый), но нашли точку — пишем в первую свободную точку
    target_index = first_dot_index;
  } else {
    // 2c. Аварийный случай (нет ни звезд, ни точек) — сбрасываем запись в начало файла
    target_index = 0;
  }

  // ШАГ 3: Сборка новой строки (ровно 32 байта, сплошной текст, добивка '>')
  memset(write_buffer, '>', PERF_RECORD_SIZE);

  // Копируем префикс
  memcpy(&write_buffer[0], "*PERFORMANCE", 12);

  // Конвертируем дату и время посимвольно напрямую в буфер записи
  uint_to_ascii(current_date.year,      4, (uint8_t *)&write_buffer[12]);
  write_buffer[16] = '-';
  uint_to_ascii(current_date.month + 1, 2, (uint8_t *)&write_buffer[17]);
  write_buffer[19] = '-';
  uint_to_ascii(current_date.month_day, 2, (uint8_t *)&write_buffer[20]);
  write_buffer[22] = 'T';
  uint_to_ascii(current_date.hour,      2, (uint8_t *)&write_buffer[23]);
  write_buffer[25] = ':';
  uint_to_ascii(current_date.min,       2, (uint8_t *)&write_buffer[26]);
  write_buffer[28] = ':';
  uint_to_ascii(current_date.sec,       2, (uint8_t *)&write_buffer[29]);

  // Последний 31-й байт (32-й по счету) остается символом '>', сформированным в memset.

  // ШАГ 4: Перемещение каретки на целевую позицию и запись сформированного блока
  uint32_t write_offset = target_index * PERF_RECORD_SIZE;
  fs_res = f_lseek(&File, write_offset);
  if (fs_res == FR_OK) {
    fs_res = f_write(&File, write_buffer, PERF_RECORD_SIZE, &bytes_written);
    if (fs_res != FR_OK || bytes_written != PERF_RECORD_SIZE) {
      app_log("PERF WRITE: Write data failed\r\n");
      sl_status = SL_STATUS_FAIL;
    }
  } else {
    app_log("PERF WRITE: lseek failed\r\n");
    sl_status = SL_STATUS_FAIL;
  }

  // Закрываем файл, чтобы гарантированно сохранить изменения на SD
  f_close(&File);
  return sl_status;
}


/**
 * @brief Проверяет, прошло ли 90 или более дней с момента последней записи PERFORMANCE.
 * @param performance_required_out Указатель на bool, куда запишется результат (true - требуется проверка, false - нет)
 * @return sl_status_t Статус выполнения операции (SL_STATUS_OK в случае успеха)
 */
sl_status_t fs_sd_check_performance_is_needed(uint8_t *performance_required_out)
{
  sl_status_t sl_status = SL_STATUS_OK;
  FRESULT fs_res = FR_OK;
  UINT bytes_read = 0;
  char check_buf[PERF_RECORD_SIZE];
  sl_sleeptimer_date_t current_date;

  if (performance_required_out == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // По умолчанию считаем, что проверка НЕ требуется (или если файл пустой)
  *performance_required_out = false;

  if (!sd_mounted) {
    app_log("PERF CHECK: SD not mounted\r\n");
    return SL_STATUS_NOT_READY;
  }

  // Получаем текущее системное время
  sl_status = sl_sleeptimer_get_datetime(&current_date);
  if (sl_status != SL_STATUS_OK) {
    app_log("PERF CHECK: Failed to get current datetime: %lu\r\n", sl_status);
    return sl_status;
  }

  app_log("PERF CHECK: date and time from rtc: %04u-%02u-%02u %02u:%02u:%02u\r\n",
          current_date.year,
          current_date.month + 1,
          current_date.month_day,
          current_date.hour,
          current_date.min,
          current_date.sec);

  // Открываем файл только на чтение
  fs_res = f_open(&File, perf_log_file, FA_READ);
  if (fs_res != FR_OK) {
    // Если файла вообще нет, значит калибровка ни разу не проводилась.
    // Возвращаем true — калибровка требуется!
    if (fs_res == FR_NO_FILE || fs_res == FR_NO_PATH) {
      app_log("PERF CHECK: File not found. Performance REQUIRED.\r\n");
      *performance_required_out = true;
      return SL_STATUS_OK;
    }
    app_log("PERF CHECK: f_open failed: %d\r\n", fs_res);
    return SL_STATUS_FAIL;
  }

  bool asterisk_found = false;
  uint32_t current_index = 0;

  // Шаг 1: Ищем в файле запись со звездочкой '*'
  while (current_index < PERF_MAX_RECORDS && !f_eof(&File)) {
    fs_res = f_read(&File, check_buf, PERF_RECORD_SIZE, &bytes_read);
    if (fs_res != FR_OK || bytes_read < PERF_RECORD_SIZE) break;

    if (check_buf[0] == '*') {
      asterisk_found = true;
      break; // Нашли последнюю активную запись, check_buf хранит её данные
    }
    current_index++;
  }

  f_close(&File);

  // Шаг 2: Если звездочка не найдена (например, файл забит точками) — значит записей нет.
  // Performance требуется.
  if (!asterisk_found) {
    app_log("PERF CHECK: No records found. Performance REQUIRED.\r\n");
    *performance_required_out = true;
    return SL_STATUS_OK;
  }

  // Шаг 3: Парсим дату из найденной строки с помощью предоставленной ascii_to_uint
  // Строка имеет вид: "*PERFORMANCE2026-01-20T14:47:25>>>>"
  uint32_t rec_year = 0, rec_month = 0, rec_day = 0;
  uint32_t rec_hour = 0, rec_min = 0, rec_sec = 0;

  bool parse_ok = true;
  parse_ok &= ascii_to_uint((uint8_t *)&check_buf[12], 4, &rec_year);
  parse_ok &= ascii_to_uint((uint8_t *)&check_buf[17], 2, &rec_month);
  parse_ok &= ascii_to_uint((uint8_t *)&check_buf[20], 2, &rec_day);
  parse_ok &= ascii_to_uint((uint8_t *)&check_buf[23], 2, &rec_hour);
  parse_ok &= ascii_to_uint((uint8_t *)&check_buf[26], 2, &rec_min);
  parse_ok &= ascii_to_uint((uint8_t *)&check_buf[29], 2, &rec_sec);

  if (!parse_ok) {
    app_log("PERF CHECK: Data corruption in record. Performance REQUIRED.\r\n");
    *performance_required_out = true;
    return SL_STATUS_OK;
  }

  // Шаг 4: Переводим обе даты в секунды
  uint64_t last_record_seconds = datetime_to_seconds(rec_year, rec_month, rec_day, rec_hour, rec_min, rec_sec);
  uint64_t current_seconds = datetime_to_seconds(current_date.year, current_date.month + 1, current_date.month_day, current_date.hour, current_date.min, current_date.sec);

  // Шаг 5: Проверяем разницу времени
  // 90 дней в секундах = 90 дней * 24 часа * 60 минут * 60 секунд = 7 776 000 секунд
  const uint64_t timeout_90_days_seconds = 90ULL * 24ULL * 3600ULL;

  if (current_seconds >= last_record_seconds) {
    uint64_t diff_seconds = current_seconds - last_record_seconds;
    if (diff_seconds >= timeout_90_days_seconds) {
      *performance_required_out = true;
      app_log("PERF CHECK: Timeout! 90+ days passed. Performance REQUIRED.\r\n");
    } else {
      *performance_required_out = false;
      app_log("PERF CHECK: Status OK. Less than 90 days passed.\r\n");
    }
  } else {
    // Ситуация "Назад в будущее" — текущее системное время меньше, чем время в записи лога.
    // Возможно, сбились часы RTC. В целях безопасности запрашиваем обслуживание.
    app_log("PERF CHECK: RTC anomaly detected! Current time is behind log time. Performance REQUIRED.\r\n");
    *performance_required_out = true;
  }

  return SL_STATUS_OK;
}
//==============================================================================================


//===============================================================================================
//                          historical records
//===============================================================================================

#define HIST_RECORD_SIZE        208896  // 208 896 байт (ровно 408 секторов по 512 байт)
#define HIST_SECTORS_PER_SLOT   408
#define HIST_MAX_RECORDS        10000

const char *historical_db_file = "recs/recs.dat"; // historical records file
// Кэш индекса активной записи '*'
uint32_t cached_hist_active_index = 0xFFFFFFFF;

uint32_t hr_record_size_offset = 0xFFFFFFFF; // смещение для размера записи
uint32_t hr_record_end_time_offset = 0xFFFFFFFF; // смещение для времени окончания сессии
uint32_t hr_writing_offset_for_masure_count = 0xFFFFFFFF; // это смещение для количества измерений
uint32_t hr_writing_offset_for_reference_count = 0xFFFFFFFF; // смещение для количества референсов
uint32_t hr_writing_offset = 0xFFFFFFFF; // общее смещение куда пишем

// Количество записей, проверяемых за ОДИН проход основного цикла во время линейного поиска.
// от 5 до 20. Чем больше число, тем быстрее идет сканирование,
// но тем длиннее блокировка основного цикла на одной итерации.
#define HIST_SCAN_CHUNK_SIZE    5

// Состояния машины состояний кэша
typedef enum {
  FS_CACHE_STATE_IDLE = 0,         // Ожидание старта или успешное завершение
  FS_CACHE_STATE_START,            // Проверка монтирования, открытие файла, быстрый старт
  FS_CACHE_STATE_BIN_SEARCH_STEP,  // Шаг бинарного поиска (1 шаг за 1 итерацию цикла)
  FS_CACHE_STATE_LIN_SCAN_START,   // Инициализация линейного сканирования (fallback)
  FS_CACHE_STATE_LIN_SCAN_STEP,    // Шаг линейного сканирования (порциями по HIST_SCAN_CHUNK_SIZE)
  FS_CACHE_STATE_CLOSE             // Закрытие файла и выставление флага готовности
} fs_cache_state_t;

// Статические переменные для сохранения контекста поиска между итерациями цикла
static fs_cache_state_t hist_cache_state = FS_CACHE_STATE_IDLE;
static bool is_historical_cache_ready = false;

// Временные переменные для работы алгоритмов поиска
static uint32_t bin_low;
static uint32_t bin_high;
static uint32_t bin_mid;
static uint32_t bin_safety_counter;
static uint32_t lin_idx;
static bool cache_found;

/**
 * @brief Запустить процесс инициализации кэша
 */
void fs_sd_historical_records_start_cache_init(void)
{
  hist_cache_state = FS_CACHE_STATE_START;
  is_historical_cache_ready = false;
  cached_hist_active_index = 0xFFFFFFFF;
  cache_found = false;
  app_log("HIST DB: Starting non-blocking cache initialization...\r\n");
}

/**
 * @brief Проверить, завершился ли поиск и готов ли кэш
 * @return true - готов, false - еще в процессе или не запускался
 */
bool fs_sd_historical_records_is_cache_ready(void)
{
  return is_historical_cache_ready;
}

sl_status_t fs_sd_historical_records_init_cache(void)
{
  FRESULT fs_res;
  UINT bytes_read = 0;
  char check_buf[2];

  switch (hist_cache_state) {
    case FS_CACHE_STATE_IDLE:
      return SL_STATUS_OK;

    case FS_CACHE_STATE_START:
      // Проверка монтирования карты
      if (!sd_mounted) {
        hist_cache_state = FS_CACHE_STATE_IDLE;
        return SL_STATUS_NOT_READY;
      }

      // Флаг необходимости перестроения таблицы
      bool need_rebuild_cltbl = false;

      // ШАГ А: Сначала пытаемся загрузить готовую таблицу (файл карты откроется и сразу закроется)
      if (load_cltbl_from_sd()) {
        // Таблица в ОЗУ, файл db_map.bin закрыт. Теперь открываем основной файл БД.
        fs_res = f_open(&File, historical_db_file, FA_READ);
        if (fs_res == FR_OK) {
          File.cltbl = lseek_tbl; // Привязываем таблицу к файлу БД

          // Тестовый прыжок в конец файла для проверки валидности таблицы
          FRESULT test_seek = f_lseek(&File, f_size(&File) - 1);
          if (test_seek == FR_OK) {
            app_log("HIST DB: Fast Seek table validated and restored INSTANTLY.\r\n");
          } else {
            app_log("HIST DB: [Warning] Loaded table is invalid (File was moved). Need rebuild.\r\n");
            File.cltbl = NULL;
            f_close(&File); // Обязательно закрываем БД перед перестроением!
            need_rebuild_cltbl = true;
          }
        } else {
          app_log("HIST DB: Failed to open DB file (error: %d)\r\n", fs_res);
          cached_hist_active_index = 0xFFFFFFFF;
          is_historical_cache_ready = true;
          hist_cache_state = FS_CACHE_STATE_IDLE;
          return SL_STATUS_NOT_FOUND;
        }
      } else {
        app_log("HIST DB: Backup cltbl not found. Will build from scratch.\r\n");
        need_rebuild_cltbl = true;
      }

      // ШАГ Б: Если таблицы не было или она устарела — строим карту заново
      if (need_rebuild_cltbl) {
        fs_res = f_open(&File, historical_db_file, FA_READ);
        if (fs_res == FR_OK) {
          app_log("HIST DB: Building linkmap (might take 1-2 seconds)...\r\n");

          File.cltbl = lseek_tbl;
          lseek_tbl[0] = sizeof(lseek_tbl) / sizeof(DWORD);

          // Просим FatFS построить карту кластеров файла БД в lseek_tbl
          FRESULT seek_res = f_lseek(&File, CREATE_LINKMAP);
          if (seek_res == FR_OK) {
            app_log("HIST DB: Linkmap built successfully! Temporarily closing DB to save map...\r\n");

            // Закрываем файл БД, чтобы освободить дескриптор перед сохранением карты!
            f_close(&File);

            // Сохраняем свежую карту на диск (файл карты откроется, запишется и закроется)
            save_cltbl_to_sd();

            // Снова открываем БД для начала бинарного поиска
            f_open(&File, historical_db_file, FA_READ);
            File.cltbl = lseek_tbl; // Связываем готовую таблицу с файлом
          } else {
            app_log("HIST DB: [Error] Linkmap creation failed: %d. Fast Seek disabled.\r\n", seek_res);
            File.cltbl = NULL; // В случае ошибки работаем в обычном медленном режиме
          }
        } else {
          app_log("HIST DB: Failed to open DB file for rebuilding (error: %d)\r\n", fs_res);
          cached_hist_active_index = 0xFFFFFFFF;
          is_historical_cache_ready = true;
          hist_cache_state = FS_CACHE_STATE_IDLE;
          return SL_STATUS_NOT_FOUND;
        }
      }

      // Настройка диапазона бинарного поиска
      bin_low = 0;
      bin_high = HIST_MAX_RECORDS - 1;
      bin_safety_counter = 30;
      cache_found = false;

      app_log("\r\n=== HIST DB: STARTING INSTANT BINARY SEARCH ===\r\n");
      hist_cache_state = FS_CACHE_STATE_BIN_SEARCH_STEP;
      return SL_STATUS_IN_PROGRESS;

    case FS_CACHE_STATE_BIN_SEARCH_STEP:
      // =========================================================================
      // ШАГ 2: Быстрый бинарный поиск
      // =========================================================================
      if (bin_low <= bin_high && bin_safety_counter > 0) {
        bin_safety_counter--;
        bin_mid = bin_low + (bin_high - bin_low) / 2;

        uint32_t offset = bin_mid * HIST_RECORD_SIZE;

        // С Fast Seek этот f_lseek отработает за микросекунды!
        fs_res = f_lseek(&File, offset);
        if (fs_res != FR_OK) {
          hist_cache_state = FS_CACHE_STATE_LIN_SCAN_START;
          return SL_STATUS_IN_PROGRESS;
        }

        fs_res = f_read(&File, check_buf, 2, &bytes_read);
        if (fs_res != FR_OK || bytes_read < 2) {
          hist_cache_state = FS_CACHE_STATE_LIN_SCAN_START;
          return SL_STATUS_IN_PROGRESS;
        }

        if (check_buf[1] == '*') {
          // Успех, нашли активный слот!
          cached_hist_active_index = bin_mid;
          cache_found = true;
          hist_cache_state = FS_CACHE_STATE_CLOSE;
        } else if (check_buf[1] == '.') {
          // Неразмеченная область. Активный элемент левее.
          if (bin_mid == 0) {
            hist_cache_state = FS_CACHE_STATE_LIN_SCAN_START;
          } else {
            bin_high = bin_mid - 1;
          }
        } else if (check_buf[1] == '#') {
          // Старый архив. Активный элемент лежит правее.
          bin_low = bin_mid + 1;
        } else {
          // Неизвестный символ разметки — переход на линейный сканер
          hist_cache_state = FS_CACHE_STATE_LIN_SCAN_START;
        }
      } else {
        hist_cache_state = FS_CACHE_STATE_LIN_SCAN_START;
      }
      return SL_STATUS_IN_PROGRESS;

    case FS_CACHE_STATE_LIN_SCAN_START:
      // =========================================================================
      // Инициализация защитного линейного сканера
      // =========================================================================
      app_log("HIST DB: Binary search bypassed. Initializing fast safe linear scan...\r\n");
      lin_idx = 0;
      hist_cache_state = FS_CACHE_STATE_LIN_SCAN_STEP;
      return SL_STATUS_IN_PROGRESS;

    case FS_CACHE_STATE_LIN_SCAN_STEP:
      // =========================================================================
      // ШАГ 3: Запасной линейный сканер
      // =========================================================================
      for (uint32_t i = 0; i < HIST_SCAN_CHUNK_SIZE; i++) {
        if (lin_idx >= HIST_MAX_RECORDS) {
          hist_cache_state = FS_CACHE_STATE_CLOSE;
          return SL_STATUS_IN_PROGRESS;
        }

        uint32_t offset = (lin_idx * HIST_RECORD_SIZE) + 1;
        fs_res = f_lseek(&File, offset);
        if (fs_res != FR_OK) {
          hist_cache_state = FS_CACHE_STATE_CLOSE;
          return SL_STATUS_IN_PROGRESS;
        }

        fs_res = f_read(&File, check_buf, 1, &bytes_read);
        if (fs_res != FR_OK || bytes_read < 1) {
          hist_cache_state = FS_CACHE_STATE_CLOSE;
          return SL_STATUS_IN_PROGRESS;
        }

        if (check_buf[0] == '*') {
          cached_hist_active_index = lin_idx;
          cache_found = true;
          hist_cache_state = FS_CACHE_STATE_CLOSE;
          return SL_STATUS_IN_PROGRESS;
        }

        lin_idx++;
      }
      return SL_STATUS_IN_PROGRESS;

    case FS_CACHE_STATE_CLOSE:
      // =========================================================================
      // ШАГ 4: Закрытие дескриптора и завершение
      // =========================================================================
      f_close(&File);
      is_historical_cache_ready = true;
      hist_cache_state = FS_CACHE_STATE_IDLE;

      if (cache_found) {
        app_log("HIST DB: Cache successfully initialized! Active index = %lu\r\n", cached_hist_active_index);
      } else {
        app_log("HIST DB: Active slot '*' not found. Database is completely blank.\r\n");
      }
      return SL_STATUS_OK;

    default:
      hist_cache_state = FS_CACHE_STATE_IDLE;
      return SL_STATUS_FAIL;
  }
}

/**
 * @brief Записывает новую сессию измерений в круговой буфер.
 * @param header_and_data_ptr Указатель на буфер с заголовком и данными измерения
 * @param actual_size Реальный размер полезных данных измерения (включая заголовок)
 */
sl_status_t fs_sd_historical_records_write_header(const void *header_and_data_ptr, uint32_t actual_size)
{
  FRESULT fs_res;
  UINT bytes_written = 0;
  uint32_t target_index = 0;
  bool need_clear_old_asterisk = false;
  uint32_t old_asterisk_index = cached_hist_active_index;
  sl_status_t status = SL_STATUS_OK;

  if (!sd_mounted) return SL_STATUS_NOT_READY;
  if (header_and_data_ptr == NULL || actual_size > HIST_RECORD_SIZE) return SL_STATUS_INVALID_PARAMETER;

  // Открываем базу данных
  fs_res = f_open(&File, historical_db_file, FA_WRITE | FA_READ);
  if (fs_res != FR_OK) {
    app_log("HIST DB: Open failed: %d\r\n", fs_res);
    return SL_STATUS_FAIL;
  }

  // Определяем целевой слот
  if (old_asterisk_index != 0xFFFFFFFF) {
    target_index = old_asterisk_index + 1;
    if (target_index >= HIST_MAX_RECORDS) {
      target_index = 0;
    }
    need_clear_old_asterisk = true;
  } else {
    target_index = 0;
    need_clear_old_asterisk = false;
  }

  app_log("HIST DB: Writing new session to slot %lu...\r\n", target_index);

  uint32_t write_offset = target_index * HIST_RECORD_SIZE;

  // Переменная, которая будет хранить физический конец логической записи в новом слоте
  uint32_t final_slot_end_offset = write_offset + HIST_RECORD_SIZE;

  // Используем do-while(0) для безопасного выхода при ошибках
  do {
    // Позиционируемся на начало нового слота
    fs_res = f_lseek(&File, write_offset);
    if (fs_res != FR_OK) { status = SL_STATUS_FAIL; break; }

    // Пишем заголовок тремя частями, принудительно заменяя 2-й байт на '#'
    const uint8_t *src_buf = (const uint8_t *)header_and_data_ptr;

    // Пишем 1-й байт заголовка (active_status, обычно '*')
    fs_res = f_write(&File, &src_buf[0], 1, &bytes_written);
    if (fs_res != FR_OK || bytes_written != 1) { status = SL_STATUS_FAIL; break; }

    // Пишем 2-й байт принудительно как '#' (временный архивный статус)
    char temp_archive_marker = '#';
    fs_res = f_write(&File, &temp_archive_marker, 1, &bytes_written);
    if (fs_res != FR_OK || bytes_written != 1) { status = SL_STATUS_FAIL; break; }

    // Пишем всю оставшуюся часть полезных данных (начиная с индекса 2)
    if (actual_size > 2) {
      uint32_t rest_size = actual_size - 2;
      fs_res = f_write(&File, &src_buf[2], rest_size, &bytes_written);
      if (fs_res != FR_OK || bytes_written != rest_size) {
        app_log("HIST DB: Write payload failed\r\n");
        status = SL_STATUS_FAIL;
        break;
      }
    }

    // Инициализируем глобальные смещения для последующих записей
    hr_writing_offset_for_masure_count = write_offset + 70;
    hr_writing_offset = hr_writing_offset_for_masure_count + 2;
    hr_record_end_time_offset = write_offset + 39;
    hr_record_size_offset = write_offset + 2;

    // Заполняем оставшуюся часть слота символом '.'
    /*if (actual_size < HIST_RECORD_SIZE) {
      uint32_t remaining_bytes = HIST_RECORD_SIZE - actual_size;
      memset(F_work, '.', sizeof(F_work));

      while (remaining_bytes > 0) {
        uint32_t chunk = (remaining_bytes > sizeof(F_work)) ? sizeof(F_work) : remaining_bytes;
        fs_res = f_write(&File, F_work, chunk, &bytes_written);
        if (fs_res != FR_OK || bytes_written != chunk) {
          app_log("HIST DB: Padding write failed\r\n");
          status = SL_STATUS_FAIL;
          break;
        }
        remaining_bytes -= chunk;
      }
      if (status == SL_STATUS_FAIL) break;
    }*/

    // АКТИВАЦИЯ. Ставим '*' во 2-й байт заголовка НОВОЙ записи
    fs_res = f_lseek(&File, write_offset + 1);
    if (fs_res == FR_OK) {
      char active_marker = '*';
      f_write(&File, &active_marker, 1, &bytes_written);
    } else { status = SL_STATUS_FAIL; break; }

    // ДЕАКТИВАЦИЯ. Снимаем '*' со СТАРОЙ записи (меняем на '#')
    if (need_clear_old_asterisk) {
      uint32_t old_offset = old_asterisk_index * HIST_RECORD_SIZE;
      fs_res = f_lseek(&File, old_offset + 1);
      if (fs_res == FR_OK) {
        char archive_marker = '#';
        f_write(&File, &archive_marker, 1, &bytes_written);
      } else { status = SL_STATUS_FAIL; break; }
    }

  } while (0);

  // Возвращаем указатель строго в физический конец записанного слота перед закрытием
  f_lseek(&File, final_slot_end_offset);
  f_close(&File);

  if (status == SL_STATUS_OK) {
    cached_hist_active_index = target_index;
    app_log("HIST DB: Slot %lu activated. Old active slot %lu archived. Success!\r\n", target_index, old_asterisk_index);
  }

  return status;
}


/**
 * @brief Дозаписывает данные в историческую БД по указанному смещению.
 * @param data_ptr Указатель на записываемые данные
 * @param size Количество байт для записи
 * @param file_offset Физическое смещение в файле (например, hr_writing_offset)
 */
sl_status_t fs_sd_historical_records_write_data(const void *data_ptr,
                                                uint32_t size,
                                                uint8_t raw_data_in_background,
                                                uint8_t raw_data_setting,
                                                const volatile uint16_t *raw_data_ptr,
                                                uint8_t bms_pic,
                                                uint8_t bms_ric,
                                                uint8_t bmsi)
{
  FRESULT fs_res;
  UINT bytes_written = 0;
  uint8_t loc_buf1[2];
  uint8_t loc_buf2[6];
  uint8_t time_buffer[19]; // Ровно 19 символов под формат "YYYY-MM-DDTHH:MM:SS"
  sl_sleeptimer_date_t current_date;
  uint32_t rec_length = 0;
  sl_status_t status = SL_STATUS_OK; // Переменная для статуса возврата

  if (!sd_mounted) return SL_STATUS_NOT_READY;
  if (data_ptr == NULL || size == 0) return SL_STATUS_INVALID_PARAMETER;

  uint32_t active_slot_start_offset = cached_hist_active_index * HIST_RECORD_SIZE;

  // Не даем улететь в 0xFFFFFFFF или за рамки слота
  if (hr_writing_offset == 0xFFFFFFFF || hr_writing_offset < (active_slot_start_offset + 72) || hr_writing_offset >= (active_slot_start_offset + HIST_RECORD_SIZE)) {
    hr_writing_offset = active_slot_start_offset + 72;
  }

  // Открываем файл для чтения и записи
  fs_res = f_open(&File, historical_db_file, FA_WRITE | FA_READ);
  if (fs_res != FR_OK) {
    app_log("HIST DB: Open failed on write_data: %d\r\n", fs_res);
    return SL_STATUS_FAIL;
  }

  // Позиционируемся на текущую рабочую точку записи данных
  fs_res = f_lseek(&File, hr_writing_offset);
  if (fs_res != FR_OK) {
    f_close(&File);
    return SL_STATUS_FAIL;
  }

  // Переменная для фиксации физического конца записи данных в этой итерации
  uint32_t target_close_offset = hr_writing_offset;

  // Единый блок выполнения на базе do-while(0)
  do {
    // СЦЕНАРИЙ 1: Запись PIC (Завершение блока измерений measurement(patient/performance))
    if (bms_pic == true) {
      fs_res = f_write(&File, data_ptr, size, &bytes_written);
      if (fs_res != FR_OK || bytes_written != size) {
        app_log("HIST DB: PIC write failed\r\n");
        status = SL_STATUS_FAIL;
        break; // Мгновенный выход в секцию закрытия файла
      }

      hr_writing_offset_for_reference_count = (uint32_t)f_tell(&File);
      hr_writing_offset = hr_writing_offset_for_reference_count + 2;
      target_close_offset = hr_writing_offset; // Запоминаем новый физический конец

      fs_res = f_lseek(&File, hr_writing_offset_for_masure_count);
      if (fs_res != FR_OK) {
        status = SL_STATUS_FAIL;
        break;
      }

      memset(loc_buf1, 0, sizeof(loc_buf1));
      uint32_t tmp = get_measurement_ident_counter();
      uint_to_ascii(tmp, 2, loc_buf1);

      fs_res = f_write(&File, loc_buf1, 2, &bytes_written);
      if (fs_res != FR_OK || bytes_written != 2) {
        app_log("HIST DB: Patient count write failed\r\n");
        status = SL_STATUS_FAIL;
        break;
      }

      hr_writing_offset_for_masure_count = 0xFFFFFFFF;
    }

    // СЦЕНАРИЙ 2: Запись RIC (Завершение блока референсов Reference)
    else if (bms_ric == true) {
      fs_res = f_write(&File, data_ptr, size, &bytes_written);
      if (fs_res != FR_OK || bytes_written != size) {
        app_log("HIST DB: RIC write failed\r\n");
        status = SL_STATUS_FAIL;
        break;
      }

      hr_writing_offset = (uint32_t)f_tell(&File);
      target_close_offset = hr_writing_offset; // Запоминаем новый физический конец

      fs_res = f_lseek(&File, hr_writing_offset_for_reference_count);
      if (fs_res != FR_OK) {
        status = SL_STATUS_FAIL;
        break;
      }

      memset(loc_buf1, 0, sizeof(loc_buf1));
      uint_to_ascii(get_reference_ident_counter(), 2, loc_buf1);

      fs_res = f_write(&File, loc_buf1, 2, &bytes_written);
      if (fs_res != FR_OK || bytes_written != 2) {
        app_log("HIST DB: Reference count write failed\r\n");
        status = SL_STATUS_FAIL;
        break;
      }

      hr_writing_offset_for_reference_count = 0xFFFFFFFF;
    }
    // СЦЕНАРИЙ 3: Финал сессии (bmsi == true)
    else if (bmsi == true) {
      fs_res = f_write(&File, data_ptr, size, &bytes_written);
      if (fs_res != FR_OK || bytes_written != size) {
        app_log("HIST DB: BMS footer write failed\r\n");
        status = SL_STATUS_FAIL;
        break;
      }

      // Сохраняем физический конец записи данных
      uint32_t real_physical_end = (uint32_t)f_tell(&File);
      target_close_offset = real_physical_end; // Это будет точкой закрытия

      fs_res = f_lseek(&File, hr_record_end_time_offset);
      if (fs_res != FR_OK) {
        status = SL_STATUS_FAIL;
        break;
      }

      sl_sleeptimer_get_datetime(&current_date);
      uint_to_ascii(current_date.year,      4, &time_buffer[0]);
      time_buffer[4] = '-';
      uint_to_ascii(current_date.month + 1, 2, &time_buffer[5]);
      time_buffer[7] = '-';
      uint_to_ascii(current_date.month_day, 2, &time_buffer[8]);
      time_buffer[10] = 'T';
      uint_to_ascii(current_date.hour,      2, &time_buffer[11]);
      time_buffer[13] = ':';
      uint_to_ascii(current_date.min,       2, &time_buffer[14]);
      time_buffer[16] = ':';
      uint_to_ascii(current_date.sec,       2, &time_buffer[17]);

      fs_res = f_write(&File, time_buffer, 19, &bytes_written);
      if (fs_res != FR_OK || bytes_written != 19) {
        app_log("HIST DB: End time write failed\r\n");
        status = SL_STATUS_FAIL;
        break;
      }

      fs_res = f_lseek(&File, hr_record_size_offset);
      if (fs_res != FR_OK) {
        status = SL_STATUS_FAIL;
        break;
      }

      rec_length = real_physical_end - (active_slot_start_offset + 8);

      uint_to_ascii(rec_length, 6, loc_buf2);
      fs_res = f_write(&File, loc_buf2, 6, &bytes_written);
      if (fs_res != FR_OK || bytes_written != 6) {
        app_log("HIST DB: Final size write failed\r\n");
        status = SL_STATUS_FAIL;
        break;
      }

      // Сбрасываем все маркеры для следующей новой сессии
      hr_record_size_offset = 0xFFFFFFFF;
      hr_record_end_time_offset = 0xFFFFFFFF;
      hr_writing_offset_for_masure_count = 0xFFFFFFFF;
      hr_writing_offset_for_reference_count = 0xFFFFFFFF;
      hr_writing_offset = 0xFFFFFFFF;
    }
    // ДЕФОЛТНЫЙ СЦЕНАРИЙ: Обычная запись любого одиночного измерения
    else {
      fs_res = f_write(&File, data_ptr, size, &bytes_written);
      if (fs_res != FR_OK || bytes_written != size) {
        app_log("HIST DB: Measurement data write failed\r\n");
        status = SL_STATUS_FAIL;
        break;
      }

      if ((raw_data_in_background == true) && (raw_data_setting == true) && (raw_data_ptr != NULL)) {
        uint8_t ascii_buf[4];

        for (uint16_t i = 0; i < 256; i++) {
          uint_to_ascii((uint32_t)raw_data_ptr[i], 4, ascii_buf);
          fs_res = f_write(&File, ascii_buf, sizeof(ascii_buf), &bytes_written);
          if (fs_res != FR_OK || bytes_written != sizeof(ascii_buf)) {
            app_log("HIST DB: ADC raw write failed at point %d\r\n", i);
            status = SL_STATUS_FAIL;
            break;
          }
        }
      }

      hr_writing_offset = (uint32_t)f_tell(&File);
      target_close_offset = hr_writing_offset; // Запоминаем новый физический конец
    }
  } while (0); // Цикл выполняется строго один раз

  // Единая и гарантированная точка закрытия файла и возврата указателя
  if (target_close_offset != 0xFFFFFFFF) {
      // Ограничиваем смещение максимальной цифрой, если оно превысило предел
      if (target_close_offset > MAX_FILE_OFFSET) {
         app_log("DBG_SD WARNING: Seek offset %lu exceeds limit! Clamping to %lu\r\n", target_close_offset, MAX_FILE_OFFSET);
         target_close_offset = MAX_FILE_OFFSET;
      }
    f_lseek(&File, target_close_offset);
  }
  f_close(&File);

  return status;
}

//====================== sync =========================================================================
// Переменные для отслеживания текущего прогресса передачи по BLE
uint32_t ble_tx_slot_idx = 0;       // Номер отправляемого сейчас слота
uint32_t ble_tx_payload_len = 0;    // Полная длина полезных данных в слоте (например, 393)
uint32_t ble_tx_sent_bytes = 0;     // Сколько байт полезных данных уже отправлено
char ble_tx_len_str[7];             // Строка длины для первого пакета ("000393")
uint32_t ble_current_sending_slot = 0;     // ID текущего отправляемого слота

/**
 * @brief Помечает конкретный слот в БД как отправленный (меняет первую '*' на '#').
 * Может быть вызвана автономно в любой момент времени.
 * * @param slot_idx Номер слота на SD-карте, который нужно маркировать
 * @return SL_STATUS_OK в случае успеха, иначе код ошибки
 */
sl_status_t ble_mark_slot_as_sent(uint32_t slot_idx)
{
  FRESULT fs_res;
  UINT bytes_written;
  char mark = '#';

  // Проверяем, смонтирована ли карта
  if (!sd_mounted) {
    return SL_STATUS_NOT_READY;
  }

  // Открываем файл БД в режиме чтения и записи
  fs_res = f_open(&File, historical_db_file, FA_READ | FA_WRITE);
  if (fs_res != FR_OK) {
    app_log("MARK: Failed to open DB (error: %d)\r\n", fs_res);
    return SL_STATUS_NOT_FOUND;
  }

  // Применяем готовую карту кластеров (Fast Seek), чтобы не читать FAT-таблицу с диска
  File.cltbl = lseek_tbl;

  // Вычисляем физическое смещение: самое начало слота (первый байт записи)
  uint32_t offset = slot_idx * HIST_RECORD_SIZE;

  fs_res = f_lseek(&File, offset);
  if (fs_res != FR_OK) {
    app_log("MARK: Seek failed (error: %d)\r\n", fs_res);
    f_close(&File);
    return SL_STATUS_FAIL;
  }

  // Записываем символ '#' поверх первой звездочки
  fs_res = f_write(&File, &mark, 1, &bytes_written);

  // Гарантированно закрываем файл сразу после записи
  f_close(&File);

  if (fs_res == FR_OK && bytes_written == 1) {
    app_log("MARK: Slot %lu marked as SENT ('#')\r\n", slot_idx);
    return SL_STATUS_OK;
  }

  app_log("MARK: Write failed for slot %lu\r\n", slot_idx);
  return SL_STATUS_FAIL;
}

/**
 * @brief Формирует ОДИН ПОЛНЫЙ пакет данных из SD-карты в tx_data_buffer.
 * Имитирует поведение функции create_one_measure_packet.
 * * @param target_buf Указатель на целевой tx_data_buffer (куда копировать пакет)
 * @param slot_idx Номер слота на SD-карте, который нужно передать
 * @return uint32_t Реальный размер сформированного BLE-пакета (от 1 до 249 байт).
 * Возвращает 0, если данные кончились или произошла ошибка.
 */
static uint32_t ble_data_start_offset = 0; // Точная физическая позиция данных на SD
extern device_operation_control_t   device_operation_control;

uint32_t create_sd_history_packet(uint8_t *target_buf, uint32_t slot_idx)
{
    FRESULT fs_res;
    UINT bytes_read = 0;
    uint32_t seek_offset = 0;
    uint16_t bytes_to_read = 0;
    uint16_t out_len = 0;
    char header_buf[9];

    // Динамически получаем лимит из вашей переменной
    uint16_t max_packet_size = device_operation_control.actual_data_size_to_transmit;
    if (max_packet_size < 12) return 0; // Защита от слишком маленького MTU

    // =========================================================================
    // 1. ИНИЦИАЛИЗАЦИЯ (Только при старте нового слота)
    // =========================================================================
    if (ble_tx_sent_bytes == 0)
    {
        app_log("DBG_SD: Starting new SD session for Slot #%lu...\r\n", slot_idx);

        fs_res = f_open(&File, historical_db_file, FA_READ);
        if (fs_res != FR_OK) {
            app_log("DBG_SD ERROR: f_open header failed (res: %d)\r\n", fs_res);
            return 0;
        }
        File.cltbl = lseek_tbl;

        // Позиционируемся на заголовок слота
        uint32_t header_offset = slot_idx * HIST_RECORD_SIZE;
        f_lseek(&File, header_offset);

        // Читаем 8 байт заголовка (#*000303)
        fs_res = f_read(&File, header_buf, 8, &bytes_read);

        // Запоминаем ТОЧНУЮ физическую позицию начала данных на SD
        ble_data_start_offset = File.fptr;

        f_close(&File);

        if (fs_res != FR_OK || bytes_read < 8) {
            app_log("DBG_SD ERROR: Failed to read header at offset %lu\r\n", header_offset);
            return 0;
        }

        // Извлекаем длину полезных данных
        memcpy(ble_tx_len_str, &header_buf[2], 6);
        ble_tx_len_str[6] = '\0';
        ble_tx_payload_len = strtoul(ble_tx_len_str, NULL, 10);

        app_log("DBG_SD: Slot #%lu Header OK. Offset: %lu, Len: %lu, MTU: %u\r\n",
                slot_idx, ble_data_start_offset, ble_tx_payload_len, max_packet_size);

        if (ble_tx_payload_len == 0) return 0;

        ble_tx_slot_idx = slot_idx;
        ble_current_sending_slot = slot_idx;
    }

    if (ble_tx_sent_bytes >= ble_tx_payload_len) {
        return 0;
    }

    // =========================================================================
    // 2. ОТКРЫТИЕ И ПОЗИЦИОНИРОВАНИЕ ПО ФАКТИЧЕСКОМУ СМЕЩЕНИЮ
    // =========================================================================
    fs_res = f_open(&File, historical_db_file, FA_READ);
    if (fs_res != FR_OK) return 0;
    File.cltbl = lseek_tbl;

    seek_offset = ble_data_start_offset + ble_tx_sent_bytes;

    fs_res = f_lseek(&File, seek_offset);
    if (fs_res != FR_OK) {
        app_log("DBG_SD ERROR: f_lseek to %lu failed (res: %d)\r\n", seek_offset, fs_res);
        f_close(&File);
        return 0;
    }

    // =========================================================================
    // 3. ПЕРВЫЙ ПАКЕТ ('[')
    // =========================================================================
    if (ble_tx_sent_bytes == 0)
    {
        target_buf[0] = '[';
        memcpy(&target_buf[1], ble_tx_len_str, 6);
        memcpy(&target_buf[7], "0001", 4);

        // 11 байт заголовок BLE -> под данные остается max_packet_size - 11
        uint16_t max_data_first_pkt = max_packet_size - 11;
        bytes_to_read = (ble_tx_payload_len < max_data_first_pkt) ? ble_tx_payload_len : max_data_first_pkt;

        fs_res = f_read(&File, &target_buf[11], bytes_to_read, &bytes_read);
        f_close(&File);

        if (fs_res != FR_OK || bytes_read == 0) return 0;

        ble_tx_sent_bytes += bytes_read;
        out_len = 11 + bytes_read;

        // Если вся запись влезла в 1-й пакет
        if (ble_tx_sent_bytes >= ble_tx_payload_len) {
            target_buf[out_len] = ']';
            out_len++;
        }

        app_log("DBG_SD: First Packet '[' formed. Read SD: %u. Progress: %lu/%lu\r\n",
                bytes_read, ble_tx_sent_bytes, ble_tx_payload_len);

        // Логирование буфера
        uint8_t temp_save = target_buf[out_len];
        target_buf[out_len] = '\0';
        app_log("DBG_SD BUF OUT [%u bytes]: %s\r\n", out_len, (char*)target_buf);
        target_buf[out_len] = temp_save;

        return (uint32_t)out_len;
    }

    // =========================================================================
    // 4. ПАКЕТЫ ПРОДОЛЖЕНИЯ ('C') ИЛИ ФИНАЛА (']')
    // =========================================================================
    uint32_t remaining_bytes = ble_tx_payload_len - ble_tx_sent_bytes;

    // 1 байт под маркер 'C' или ']' -> под данные остается max_packet_size - 1
    uint16_t max_data_cont_pkt = max_packet_size - 1;

    if (remaining_bytes <= max_data_cont_pkt)
    {
        // ФИНАЛЬНЫЙ ПАКЕТ: ']'
        target_buf[0] = ']';
        bytes_to_read = remaining_bytes;

        fs_res = f_read(&File, &target_buf[1], bytes_to_read, &bytes_read);
        f_close(&File);

        if (fs_res != FR_OK || bytes_read == 0) return 0;

        ble_tx_sent_bytes += bytes_read;
        out_len = 1 + bytes_read;

        app_log("DBG_SD: FINAL Packet ']' formed. Read SD: %u. Progress: %lu/%lu\r\n",
                bytes_read, ble_tx_sent_bytes, ble_tx_payload_len);

        // Логирование буфера
        uint8_t temp_save = target_buf[out_len];
        target_buf[out_len] = '\0';
        app_log("DBG_SD BUF OUT [%u bytes]: %s\r\n", out_len, (char*)target_buf);
        target_buf[out_len] = temp_save;

        return (uint32_t)out_len;
    }
    else
    {
        // ПРОМЕЖУТОЧНЫЙ ПАКЕТ: 'C'
        target_buf[0] = 'C';
        bytes_to_read = max_data_cont_pkt;

        fs_res = f_read(&File, &target_buf[1], bytes_to_read, &bytes_read);
        f_close(&File);

        if (fs_res != FR_OK || bytes_read == 0) return 0;

        ble_tx_sent_bytes += bytes_read;
        out_len = 1 + bytes_read;

        app_log("DBG_SD: Continuation Packet 'C' formed. Read SD: %u. Progress: %lu/%lu\r\n",
                bytes_read, ble_tx_sent_bytes, ble_tx_payload_len);

        // Логирование буфера
        uint8_t temp_save = target_buf[out_len];
        target_buf[out_len] = '\0';
        app_log("DBG_SD BUF OUT [%u bytes]: %s\r\n", out_len, (char*)target_buf);
        target_buf[out_len] = temp_save;

        return (uint32_t)out_len;
    }
}
