/***************************************************************************//**
 * @file app.c
 * @brief Top level application functions
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
 ********************************************************************************
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
#include <string.h>
#include "app_log.h"
#include "app_assert.h"
#include "sl_sleeptimer.h"
#include "em_cmu.h"
#include "cli.h"
#include "file_storage.h"
#include "flash_storage.h"
#include "ui_display.h"


#include "fuim.h"
#include "find_api.h"

#include "auph.h"

#include "button.h"
#define TEST_DISPLAY
//#define TEST_SD
#define TEST_FLASH

//#define TEST_FLASH_ERASE_PROG
//#define ENABLE_FLASH_HPF_MODE


#if defined(TEST_FLASH) && defined(TEST_FLASH_ERASE_PROG_TEST)

#define IMAGE_COUNT (sizeof(image_files)/sizeof(image_files[0]))

static const char *image_files[] = {
       "U1.raw"
      ,"U2.raw"
      ,"U3.raw"
      ,"U4.raw"
      //,"image2.bmp"
  };
#define IMG_FILE_U1_RAW_SIZE 19602

#endif

void get_clocks_info(void)
{
  
  uint32_t freq = SystemCoreClockGet()  / 1000000;
  app_log("CPU=%luMHZ \r\n",freq);

  uint32_t pclk = CMU_ClockFreqGet(cmuClock_PCLK);   // APB bus clock
  app_log("APB bus=%luMHZ \r\n",pclk);

  uint32_t hclk = CMU_ClockFreqGet(cmuClock_HCLK);   // AHB bus clock
  app_log("AHB bus=%luMHZ \r\n",hclk);

  uint32_t sysclk = CMU_ClockFreqGet(cmuClock_SYSCLK); // System clock
  app_log("System clock=%luMHZ \r\n",sysclk);

}

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  sl_status_t sl_status_code = SL_STATUS_OK;


#if defined(TEST_SD) && defined(TEST_FLASH) && defined(TEST_FLASH_ERASE_PROG)
  uint32_t address;
  //uint32_t img_index;
  //uint32_t image_index = 0;
  char sd_card_file_path[20] = "";
#endif

#if defined(TEST_FLASH)
  uint32_t bitRate = 0;
#endif
#if defined(TEST_SD)
  const char filepath[] = "HELLO.TXT";
        char test_str[] = "Initialize application.";
  uint32_t f_size;
#endif

  //uint32_t f_req;
 // CMU_ClockDivSet(cmuClock_PCLK, 2U);

  get_clocks_info();

  cli_app_init();

#ifdef TEST_DISPLAY
  // Initialize DISPLAY interface
  disp_init();
#endif

#ifdef TEST_SD
  // Initialize file storage of SD card
  fs_sd_log_init();
  FS_LOG_INFO((const char *)&test_str[0]);
#endif

#ifdef TEST_FLASH
  // Initialize FLASH interface
  sl_status_code = flash_storage_init();
  if (sl_status_code != SL_STATUS_OK) {
    app_log("Init Flash is Failed: %lu\r\n",sl_status_code);
    app_assert_status(SL_STATUS_FAIL); // Loop forever for debugging
  }

#ifdef ENABLE_FLASH_HPF_MODE
  app_log("Enable Flash HPF MODE\r\n");
  sl_status_code =  flash_storage_enable_hpf_mode();
  if (sl_status_code != SL_STATUS_OK) {
    app_log("HPF Flash mode is Failed: %lu\r\n",sl_status_code);
    app_assert_status(SL_STATUS_FAIL); // Loop forever for debugging
  }

  //CMU_ClockDivSet(cmuClock_PCLK, 1U);
  get_clocks_info();
#endif

  sl_status_code = flash_spi_getBitRate(&bitRate);
  if (sl_status_code == SL_STATUS_OK ) {
     app_log("Flash SPI bitrate=%luMHZ \r\n",bitRate);
  } else {
     app_log("Flash SPI bitrate ERROR\r\n");
  }

#ifdef TEST_FLASH_ERASE_PROG
  app_printf("Start Erase Flash Chip\r\n");
  sl_status_code = flash_storage_erase_chip();
  if (sl_status_code != SL_STATUS_OK) {
    app_log("Erase Flash Chip is Failed: %lu\r\n",sl_status_code);
    app_assert_status(SL_STATUS_FAIL); // Loop forever for debugging
  }
  app_printf("Erase Flash Chip : Done\r\n");

/// Programming *.RAW Flash image storage at 0x0000 start flash address
  address = 0x0;
  snprintf(sd_card_file_path, sizeof(sd_card_file_path) ,"%s", "out.raw");
  app_printf("Start Write file %s to Flash \r\n", sd_card_file_path);

  sl_status_code = fs_sd_write_img_to_flash(sd_card_file_path, address);
  if (sl_status_code != SL_STATUS_OK) {
    app_log("Write to addr=0x%lx Flash is Failed: %lu\r\n", address, sl_status_code);
    app_assert_status(SL_STATUS_FAIL); // Loop forever for debugging
  }

#endif //endif TEST_FLASH_ERASE_PROG

#endif //endif TEST_FLASH

 ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if defined(TEST_SD) && defined(TEST_FLASH) && defined(TEST_FLASH_ERASE_PROG_TEST)

  address = 0x080000;
  sl_status_code = flash_storage_erase_block64(address);
  if (sl_status_code != SL_STATUS_OK) {
    app_log("Erase addr=0x%lx Flash 64k is Failed: %lu\r\n", address, sl_status_code);
    app_assert_status(SL_STATUS_FAIL); // Loop forever for debugging
  }
  app_log("Erase addr=0x%lx Flash 64k is Done: %lu\r\n", address, sl_status_code);

#if 1
 address = 0x080000;
 for ( img_index = 0; img_index < IMAGE_COUNT; img_index++ ) {

   // Form the file name, for example "image0.bin" (assuming raw RGB565 files on SD)
   snprintf(sd_card_file_path, sizeof(sd_card_file_path) ,"%s", image_files[img_index]);

    sl_status_code = fs_sd_write_img_to_flash(sd_card_file_path, address);
    if (sl_status_code != SL_STATUS_OK) {
      app_log("Write to addr=0x%lx Flash is Failed: %lu\r\n", address, sl_status_code);
      app_assert_status(SL_STATUS_FAIL); // Loop forever for debugging
     }

    f_size = 0;
    if (fs_sd_get_file_size(sd_card_file_path, &f_size) != SL_STATUS_OK) {
      app_log("Getting size of file: %s Failed\r\n", sd_card_file_path);
      app_assert_status(SL_STATUS_FAIL); // Loop forever for debugging
    }

    address += f_size;
  } 
#endif

#endif

 fuim_Init();
 aukh_Init();
 find_Init();
 button_feature_init();

}

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/

sl_status_t get_time(char *str_buf)
{
  sl_status_t sl_status_code = SL_STATUS_OK;

  sl_sleeptimer_date_t date_time = {
    .year = 122,
    .month = 2,
    .month_day = 1,
    .hour = 10,
    .min = 30,
    .sec = 0,
  };


  sl_status_code = sl_sleeptimer_get_datetime(&date_time);
  app_assert_status(sl_status_code);

  sprintf(str_buf, "Current time is %02u:%02u:%02u.\r\n",
              date_time.hour,
              date_time.min,
              date_time.sec);

  return sl_status_code;
}

/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
#ifdef TEST_SD
  uint8_t test_str[35] = {0};
#endif

  cli_app_process_action();

#ifdef TEST_FLASH
  /* @ToDo Workaround to prevent reading corrupted data  It was been added by UP*/
  flash_storage_wakeup_chip(); /*This is the worst way to solve the issue*/
#endif

#ifdef TEST_DISPLAY
  disp_process_action();
#endif

  sl_sleeptimer_delay_millisecond(5);

#ifdef TEST_SD
  fs_sd_log_flush_task();
#endif

  button_feature_process();

  if (aukh_ReadCommand()) {

      aukh_ProcessKey();
  }

  if ((auph_GetState() != AU_ERROR_STATE )) {

    fuim_Update();//1 The location cannot be changed.
    find_Update();//2 The location cannot be changed.
  }

}
