/***************************************************************************//**
 * @file
 * @brief Top level application functions
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
#include <string.h>
#include "app_log.h"
#include "app_assert.h"

#include "command_line.h"
#include "file_storage.h"
#include "flash_storage.h"

#include "disp.h"
#include "fuim.h"
#include "find_api.h"
#include "auph.h"
#include "fsrv.h"
#include "fpmt_api.h"

#include "aevs.h"
#include "buzzer.h"
#include "led.h"
#include "pin_config.h"
#include "protocol.h"
#include "button.h"
#include "params.h"
#include "adc.h"
#include "battery_management.h"

#include "time_and_date.h"

#define TEST_SD
#define TEST_FLASH

//#define TEST_FLASH_ERASE_PROG


/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  sl_status_t sl_status_code = SL_STATUS_OK;

  GPIO_PinModeSet(EN_BTM_PORT, EN_BTM_PIN, gpioModePushPull, 1); // enable divider for battery voltage measure


#if defined(TEST_SD) && defined(TEST_FLASH) && defined(TEST_FLASH_ERASE_PROG)
  uint32_t address;
  char sd_card_file_path[20] = "";
#endif

#if defined(TEST_FLASH)
  uint32_t bitRate = 0;
#endif
#if defined(TEST_SD)
  const char filepath[] = "HELLO.TXT";
        char test_str[] = "Initialize application.";
#endif

  //uint32_t f_req;
 // CMU_ClockDivSet(cmuClock_PCLK, 2U);

  get_clocks_info();

  app_cli_setup();

  // Initialize DISPLAY interface
  disp_Init();

#ifdef TEST_SD
  // Initialize file storage of SD card
  fs_sd_log_init();

  //IMPORTANT - This the test line In this case logging to file has not had alloyed yet
  FSLOG_INFO((const char *)&test_str[0]); //The string doesn't log
  FSLOG_INFO((const char *)&filepath[0]); //The string doesn't log
#endif

#ifdef TEST_FLASH
  // Initialize FLASH interface
  sl_status_code = flash_storage_init();
  if (sl_status_code != SL_STATUS_OK) {
    app_log("Init Flash is Failed: %lu\r\n",sl_status_code);
    app_assert_status(SL_STATUS_FAIL); // Loop forever for debugging
  }

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


  init_buzzer();
  led_init();
  set_default_general_params();
  set_default_advanced_params();
  button_feature_init();
  
  app_log("Init TIP cache - Please wait\r\n");
  sl_status_code = fs_sd_init_tip_cache();
  if (sl_status_code != SL_STATUS_OK) {
      app_log("Init TIP cache is Failed: %lu\r\n",sl_status_code);
      app_assert_status(SL_STATUS_FAIL); // Loop forever for debugging
  }
  // Запускаем неблокирующий поиск активного слота
  fs_sd_historical_records_start_cache_init();
  //fs_sd_historical_records_init_cache();

  init_adc_scan();


  fpmt_Init(); // function power manager
  fsrv_Init(); // function service
  fuim_Init(); // function user interface manager
  aukh_Init(); // application user key handler
  find_Init(); // function indicator
  fslog_Init();// function storage logging
  aevs_Init(); // application event manager

  buzzer_beep(2500,250);
  led_control(250,0);

  print_mcu_reset_cause();
  //start_impact_capture(15,ADC_VREF_INTERNAL_1P21V);
  //debug_hardware_trinity();
}

/***************************************************************************//**
 * Initialize application.
 ******************************************************************************/
/***************************************************************************//**
 * App ticking function.
 ******************************************************************************/
void app_process_action(void)
{
  //Двигаем машину состояний кэша, пока он не будет готов
  if (!fs_sd_historical_records_is_cache_ready()) {
      sl_status_t cache_status = fs_sd_historical_records_init_cache();
      if (cache_status == SL_STATUS_OK) {
        app_log("Main: Historical cache is ready! System is fully functional.\r\n");
        // Здесь можно разблокировать функции чтения/записи истории для пользователя
      }
      else if (cache_status == SL_STATUS_NOT_FOUND || cache_status == SL_STATUS_FAIL) {
        app_log("Main: Failed to init cache. Check card or format database.\r\n");
      }
  }
  // CLI обрабатывается автоматически в sl_system_process_action()
  income_packet_processing();

  battery_management_process();

  is_stilus_connected_process();

  measurement_process_loop();

  button_beep_process();

  repeated_start_of_transmition();

  heart_beat_process();



#ifdef TEST_FLASH
  /* @ToDo Workaround to prevent reading corrupted data  It was been added by UP*/
  //flash_storage_wakeup_chip(); /*This is the worst way to solve the issue*/
#endif

  if (aukh_ReadCommand()) {
    aukh_ProcessKey();
  }

  disp_Update();
  fsrv_Update();

  if (auph_GetState() != AU_ERROR_STATE) {
    fuim_Update();//1 The location cannot be changed.
    find_Update();//2 The location cannot be changed.
    aevs_Update();//3 Indicator Event manager
  }

  fslog_Update();
  fpmt_Update();

}
