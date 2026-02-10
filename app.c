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


#define TEST_DISPLAY
#define TEST_SD
#define TEST_FLASH

#define TEST_FLASH_ERASE_PROG



#if defined(TEST_FLASH) && defined(TEST_FLASH_ERASE_PROG)

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
  uint32_t img_index;
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
  CMU_ClockDivSet(cmuClock_PCLK, 2U);

  get_clocks_info();

  cli_app_init();

#ifdef TEST_DISPLAY
  disp_init();
#endif

#ifdef TEST_SD
  // Initialize file storage of SD card
  sl_status_code = fs_sd_init();
  if (sl_status_code != SL_STATUS_OK) {
    // Failed to init SD card, handle error
    app_assert_status(SL_STATUS_FAIL); // Loop forever for debugging
  }

  if ( fs_sd_disk_volume_status() != SL_STATUS_OK) {
    // Failed to init SD card, handle error
    app_assert_status(SL_STATUS_FAIL); // Loop forever for debugging
  }

  //sl_sleeptimer_delay_millisecond(10);
  if (fs_sd_get_file_size(filepath, &f_size) != SL_STATUS_OK) {
    app_log("Getting size of file: %s Failed\r\n", filepath);
    app_assert_status(SL_STATUS_FAIL); // Loop forever for debugging
  }

  app_printf("File %s, size = %lu\r\n",filepath, f_size);

  if (fs_sd_append_to_file(filepath, (const void *)&test_str[0], sizeof(test_str)) != SL_STATUS_OK) {
    app_log("Append to file: Failed\r\n");
    app_assert_status(SL_STATUS_FAIL); // Loop forever for debugging
  }
#endif

#ifdef TEST_FLASH
  // Initialize FLASH interface
  sl_status_code = flash_storage_init();
  if (sl_status_code != SL_STATUS_OK) {
    app_log("Init Flash is Failed: %lu\r\n",sl_status_code);
    app_assert_status(SL_STATUS_FAIL); // Loop forever for debugging
  }

  sl_status_code =  flash_storage_enable_hpf_mode();
  if (sl_status_code != SL_STATUS_OK) {
    app_log("HPF Flash mode is Failed: %lu\r\n",sl_status_code);
    app_assert_status(SL_STATUS_FAIL); // Loop forever for debugging
  }

  //CMU_ClockDivSet(cmuClock_PCLK, 1U);
  get_clocks_info();
  sl_status_code = flash_spi_getBitRate(&bitRate);
  if (sl_status_code == SL_STATUS_OK ) {
     app_log("Flash SPI bitrate=%luMHZ \r\n",bitRate);
  } else {
     app_log("Flash SPI bitrate ERROR\r\n");
  }

#ifdef TEST_FLASH_ERASE_PROG
  sl_status_code = flash_storage_erase_chip();
  if (sl_status_code != SL_STATUS_OK) {
    app_log("Erase Flash Chip is Failed: %lu\r\n",sl_status_code);
    app_assert_status(SL_STATUS_FAIL); // Loop forever for debugging
  }
  app_printf("Erase Flash Chip : Done\r\n");

  sl_status_code =  flash_storage_write( FLASH_ADDR_IMAGE_CAT_IMAGE, gImage_cat_image, sizeof(gImage_cat_image));
  if (sl_status_code != SL_STATUS_OK) {
    app_log("Write to Flash is Failed: %lu\r\n",sl_status_code);
    app_assert_status(SL_STATUS_FAIL); // Loop forever for debugging
  }
  app_log("Write to Flash Chip at addr = %u is Done\r\n",FLASH_ADDR_IMAGE_CAT_IMAGE);

  sl_status_code =  flash_storage_write( FLASH_ADDR_BIRD_IMAGE, gImage_bird_image, sizeof(gImage_bird_image));
  if (sl_status_code != SL_STATUS_OK) {
    app_log("Write to Flash is Failed: %lu\r\n",sl_status_code);
    app_assert_status(SL_STATUS_FAIL); // Loop forever for debugging
  }
  app_log("Write to Flash Chip at addr = %u is Done\r\n",FLASH_ADDR_BIRD_IMAGE);

  sl_status_code =  flash_storage_write( FLASH_ADDR_CUTE_IMAGE, gImage_cute_image,  sizeof(gImage_cute_image));
  if (sl_status_code != SL_STATUS_OK) {
    app_log("Write to Flash is Failed: %lu\r\n",sl_status_code);
    app_assert_status(SL_STATUS_FAIL); // Loop forever for debugging
  }
  app_log("Write to Flash Chip at addr = %u is Done\r\n",FLASH_ADDR_CUTE_IMAGE);

  sl_status_code =  flash_storage_write( FLASH_ADDR_CACTUS_PLANTS, gImage_cactus_plants, sizeof(gImage_cactus_plants));
  if (sl_status_code != SL_STATUS_OK) {
    app_log("Write to Flash is Failed: %lu\r\n",sl_status_code);
    app_assert_status(SL_STATUS_FAIL); // Loop forever for debugging
  }
  app_log("Write to Flash Chip at addr = %u is Done\r\n",FLASH_ADDR_CACTUS_PLANTS);

  sl_status_code =  flash_storage_write( FLASH_ADDR_NATURE_IMAGE, gImage_nature_image, sizeof(gImage_nature_image));
  if (sl_status_code != SL_STATUS_OK) {
    app_log("Write to Flash is Failed: %lu\r\n",sl_status_code);
    app_assert_status(SL_STATUS_FAIL); // Loop forever for debugging
  }
  app_log("Write to Flash Chip at addr = %u is Done\r\n",FLASH_ADDR_NATURE_IMAGE);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
      address = FLASH_ADDR_CACTUS_PLANTS+34;
      sl_status_code = flash_storage_read(address,
                                          (uint8_t *)&test_str[0],
                                          sizeof(test_str));

      if (sl_status_code != SL_STATUS_OK) {
        app_log("Error Read addr=0x%lx test_str[] Err: %lu\r\n", address, sl_status_code);
        app_assert_status(SL_STATUS_FAIL); // Loop forever for debugging
      }
      if (memcmp((uint8_t *)&gImage_cactus_plants[34] ,(uint8_t *)&test_str[0],sizeof(test_str)) !=0) {
          app_log("gImage_cactus_plants Check memory Error\r\n");
          app_assert_status(SL_STATUS_FAIL); // Loop forever for debugging
      }
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
      address = FLASH_ADDR_IMAGE_CAT_IMAGE+55;
      sl_status_code = flash_storage_read(address,
                                          (uint8_t *)&test_str[0],
                                          sizeof(test_str));

      if (sl_status_code != SL_STATUS_OK) {
        app_log("Error Read addr=0x%lx test_str[] Err: %lu\r\n", address, sl_status_code);
        app_assert_status(SL_STATUS_FAIL); // Loop forever for debugging
      }
      if (memcmp((uint8_t *)&gImage_cat_image[55] ,(uint8_t *)&test_str[0],sizeof(test_str)) !=0) {
          app_log("gImage_cat_image Check memory Error\r\n");
          app_assert_status(SL_STATUS_FAIL); // Loop forever for debugging
      }
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
      address = FLASH_ADDR_BIRD_IMAGE+87;
      sl_status_code = flash_storage_read(address,
                                          (uint8_t *)&test_str[0],
                                          sizeof(test_str));

      if (sl_status_code != SL_STATUS_OK) {
        app_log("Error Read addr=0x%lx test_str[] Err: %lu\r\n", address, sl_status_code);
        app_assert_status(SL_STATUS_FAIL); // Loop forever for debugging
      }
      if (memcmp((uint8_t *)&gImage_bird_image[87] ,(uint8_t *)&test_str[0],sizeof(test_str)) !=0) {
          app_log("gImage_bird_image Check memory Error\r\n");
          app_assert_status(SL_STATUS_FAIL); // Loop forever for debugging
      }
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
      address = FLASH_ADDR_CUTE_IMAGE+46;
      sl_status_code = flash_storage_read(address,
                                          (uint8_t *)&test_str[0],
                                          sizeof(test_str));

      if (sl_status_code != SL_STATUS_OK) {
        app_log("Error Read addr=0x%lx test_str[] Err: %lu\r\n", address, sl_status_code);
        app_assert_status(SL_STATUS_FAIL); // Loop forever for debugging
      }
      if (memcmp((uint8_t *)&gImage_cute_image[46] ,(uint8_t *)&test_str[0],sizeof(test_str)) !=0) {
          app_log("gImage_cute_image Check memory Error\r\n");
          app_assert_status(SL_STATUS_FAIL); // Loop forever for debugging
      }
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
      address = FLASH_ADDR_NATURE_IMAGE+100;
      sl_status_code = flash_storage_read(address,
                                          (uint8_t *)&test_str[0],
                                          sizeof(test_str));

      if (sl_status_code != SL_STATUS_OK) {
        app_log("Error Read addr=0x%lx test_str[] Err: %lu\r\n", address, sl_status_code);
        app_assert_status(SL_STATUS_FAIL); // Loop forever for debugging
      }
      if (memcmp((uint8_t *)&gImage_nature_image[100] ,(uint8_t *)&test_str[0],sizeof(test_str)) !=0) {
          app_log("gImage_nature_image Check memory Error\r\n");
          app_assert_status(SL_STATUS_FAIL); // Loop forever for debugging
      }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif

  app_log("Flash Chip in use = %u Bytes\r\n", (FLASH_ADDR_NATURE_IMAGE  + 64808));

#endif



  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if defined(TEST_SD) && defined(TEST_FLASH) && defined(TEST_FLASH_ERASE_PROG)

  address = 0x080000;
  sl_status_code = flash_storage_erase_block64(address);
  if (sl_status_code != SL_STATUS_OK) {
    app_log("Erase addr=0x%lx Flash 64k is Failed: %lu\r\n", address, sl_status_code);
    app_assert_status(SL_STATUS_FAIL); // Loop forever for debugging
  }
  app_log("Erase addr=0x%lx Flash 64k is Done: %lu\r\n", address, sl_status_code);

  address = 0x080000 + (MX25_BLOCK64_SIZE - 1);
  sl_status_code = flash_storage_erase_block64(address);
  if (sl_status_code != SL_STATUS_OK) {
    app_log("Erase addr=0x%lx Flash 64k is Failed: %lu\r\n", address, sl_status_code);
    app_assert_status(SL_STATUS_FAIL); // Loop forever for debugging
  }
  app_log("Erase addr=0x%lx Flash 64k is Done: %lu\r\n", address, sl_status_code);


#if 0
  sl_status_code = flash_storage_write(address,
                                       (uint8_t *)&test_str[0],
                                        sizeof(test_str));
  if (sl_status_code != SL_STATUS_OK) {
    app_log("Error Write to addr=0x%lx test_str[] Err : %lu\r\n", address, sl_status_code);
    app_assert_status(SL_STATUS_FAIL); // Loop forever for debugging
  }
#endif

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

  address = 0x080000;
  sl_status_code = flash_storage_read(address,
                                      (uint8_t *)&test_str[0],
                                      sizeof(test_str));

  if (sl_status_code != SL_STATUS_OK) {
    app_log("Error Read addr=0x%lx test_str[] Err: %lu\r\n", address, sl_status_code);
    app_assert_status(SL_STATUS_FAIL); // Loop forever for debugging
  }


  address = 0x080000 + IMG_FILE_U1_RAW_SIZE;
  sl_status_code = flash_storage_read(address,
                                      (uint8_t *)&test_str[0],
                                      sizeof(test_str));

  if (sl_status_code != SL_STATUS_OK) {
    app_log("Error Read addr=0x%lx test_str[] Err: %lu\r\n", address, sl_status_code);
    app_assert_status(SL_STATUS_FAIL); // Loop forever for debugging
  }


#endif

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

  /* @ToDo Workaround to prevent reading corrupted data  It was been added by UP*/
  flash_storage_wakeup_chip(); /*This is the worst way to solve the issue*/

#ifdef TEST_DISPLAY
  disp_process_action();
#endif

  sl_sleeptimer_delay_millisecond(1);

#ifdef TEST_SD
  if (get_time((char *)&test_str[0]) != SL_STATUS_OK) {
    // Failed to get time, handle error
    app_assert_status(SL_STATUS_FAIL); // Loop forever for debugging
  }

  if (fs_sd_append_to_file("HELLO.TXT", test_str, 27) != SL_STATUS_OK) {
    // Failed to init SD card, handle error
    app_assert_status(SL_STATUS_FAIL); // Loop forever for debugging
  }
#endif
  sl_sleeptimer_delay_millisecond(100);


}
