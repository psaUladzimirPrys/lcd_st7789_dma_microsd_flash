/*
 * ui_display.c
 *
 *  Created on: 30 янв. 2026 г.
 *      Author: priss
 */


/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/

/*******************************************************************************
 *********************   LOCAL FUNCTION PROTOTYPES   ***************************
 ******************************************************************************/

#include "app_log.h"
#include "app_assert.h"
#include "sl_sleeptimer.h"
#include "ui_display.h"
#include "img_storage.h"



/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/

#define APP_TIMER_SMALL        300
#define APP_TIMER_MEDIUM       2000
#define APP_TIMER_LARGE        7000



/***************************************************************************//**
*  /UP Display configuration section declaration global variables
*******************************************************************************/


static glib_context_t g_context;
static uint8_t demo_index = 0;

MIPI_DBI_SPI_INTERFACE_DEFINE(st7789_config,
                              ADAFRUIT_ST7789_PERIPHERAL,
                              ADAFRUIT_ST7789_PERIPHERAL_NO,
                              ADAFRUIT_ST7789_BITRATE,
                              ADAFRUIT_ST7789_CLOCK_MODE,
                              ADAFRUIT_ST7789_CS_CONTROL,
                              ADAFRUIT_ST7789_CLK_PORT,
                              ADAFRUIT_ST7789_CLK_PIN,
                              ADAFRUIT_ST7789_TX_PORT,
                              ADAFRUIT_ST7789_TX_PIN,
                              ADAFRUIT_ST7789_RX_PORT,
                              ADAFRUIT_ST7789_RX_PIN,
                              ADAFRUIT_ST7789_CS_PORT,
                              ADAFRUIT_ST7789_CS_PIN,
                              ADAFRUIT_ST7789_DC_PORT,
                              ADAFRUIT_ST7789_DC_PIN);

static volatile bool app_timer_expire = false;
static sl_sleeptimer_timer_handle_t app_sleep_timer;
static void app_sleeptimer_callback(sl_sleeptimer_timer_handle_t *timer,
                                    void *data);

#if 0
static inline uint32_t micros_start(void) __attribute__((always_inline));

static inline uint32_t micros_start(void)
{

  uint32_t time_ms = (uint32_t)sl_sleeptimer_tick_to_ms((uint32_t)sl_sleeptimer_get_tick_count());

  while ((uint32_t)sl_sleeptimer_tick_to_ms((uint32_t)sl_sleeptimer_get_tick_count()) == time_ms)
    ;

  return (uint32_t)sl_sleeptimer_get_tick_count();

}
#endif


/*******************************************************************************
 *********************   LOCAL FUNCTION PROTOTYPES   ***************************
 ******************************************************************************/


void disp_init(void)
{

  uint32_t bitRate;
  sl_status_t  error = SL_STATUS_OK;

  error = adafruit_st7789_init(&st7789_config);
  if (error != SL_STATUS_OK) {
    // Failed to init st7789, handle error
    app_log("Init ST7789 TFT is Failed: %lu\r\n",error);
    app_assert_status(SL_STATUS_FAIL); // Loop forever for debugging
  }

  error = mipi_dbi_device_getBitRate(&bitRate);
  if (error == SL_STATUS_OK ) {
     app_log("ST7789 SPI bitrate=%luMHZ \r\n",bitRate);
  }
  else {
     app_log("ST7789 SPI bitrate ERROR\r\n");
  }


  error = adafruit_st7789_set_rotation(adafruit_st7789_rotation_90);
  if (error != SL_STATUS_OK) {
     // Failed to init st7789, handle error
     app_log("Send rotation90 Command is Failed: %lu\r\n",error);
     app_assert_status(SL_STATUS_FAIL); // Loop forever for debugging
   }

  glib_init(&g_context);

  app_log("Adafruit ST7789 TFT init done\r\n");
  app_log("GLIB init done\r\n");

  sl_sleeptimer_restart_periodic_timer_ms(&app_sleep_timer,
                                          APP_TIMER_SMALL,
                                          app_sleeptimer_callback,
                                          NULL,
                                          0,
                                          0);
  app_log("Start sleep timer %dms\r\n", APP_TIMER_SMALL);

  glib_enable_display(false);
  app_log("glib_enable_display --> false\r\n");

  g_context.textsize_x = 2;
  g_context.textsize_y = 2;
  g_context.wrap = false;


  glib_set_color(&g_context, ST7789_WHITE, ST7789_BLACK);
  glib_fill(&g_context, ST7789_WHITE);


}

/***************************************************************************//**
 * Display ticking function.
 ******************************************************************************/
void disp_process_action(void)
{

  if (app_timer_expire) {
    app_timer_expire = false;

    switch (demo_index) {
      case 0:
        //Draw LOGO
        adafruit_st7789_draw_rgb_bitmap_from_flash(111, 37,
                                                   img_storage_desc[IMG_ID_UNION3_OUT].width,
                                                   img_storage_desc[IMG_ID_UNION3_OUT].height,
                                                   img_storage_desc[IMG_ID_UNION3_OUT].storage_address,
                                                   true);



        sl_sleeptimer_restart_periodic_timer_ms(&app_sleep_timer,
                                                APP_TIMER_LARGE,
                                                app_sleeptimer_callback,
                                                NULL,
                                                0,
                                                0);
        app_log("Start sleep timer %dms\r\n", APP_TIMER_LARGE);
        glib_enable_display(true);
        app_log("glib_enable_display --> true\r\n");
        demo_index++;
        break;
      case 1:

        app_log("Start sleep timer %dms\r\n", APP_TIMER_LARGE);


        break;

      case 2:

        app_log("Menu = 1\r\n");
        break;

      case 3:

        app_log("Menu = 2\r\n");
        break;
      case 4:


        app_log("Menu = 3\r\n");
        break;
      case 5:

        sl_sleeptimer_restart_periodic_timer_ms(&app_sleep_timer,
                                                APP_TIMER_MEDIUM,
                                                app_sleeptimer_callback,
                                                NULL,
                                                0,
                                                0);

        app_log("Menu = 4\r\n");
        break;
      case 6:


        sl_sleeptimer_restart_periodic_timer_ms(&app_sleep_timer,
                                                APP_TIMER_LARGE,
                                                app_sleeptimer_callback,
                                                NULL,
                                                0,
                                                0);

        app_log("Menu = 4 Indicator: TIP ID INVALID\r\n");
        break;
      case 7:

        sl_sleeptimer_restart_periodic_timer_ms(&app_sleep_timer,
                                                APP_TIMER_MEDIUM,
                                                app_sleeptimer_callback,
                                                NULL,
                                                0,
                                                0);

        app_log("Menu = 3\r\n");
        break;
      case 8:

        //battery 0

        sl_sleeptimer_restart_periodic_timer_ms(&app_sleep_timer,
                                                APP_TIMER_LARGE,
                                                app_sleeptimer_callback,
                                                NULL,
                                                0,
                                                0);
        app_log("Menu = 3 Indicator: CHARGE THE BATTERY\r\n");
        break;
      case 9:


      //battery -0

     app_log("Menu = 5\r\n");
      break;
     default:
        break;
    }

    demo_index++;

    if(demo_index > 9) {
      demo_index = 1;
    }

  }
}

static void app_sleeptimer_callback(sl_sleeptimer_timer_handle_t *timer, void *data)
{
  (void)timer;
  (void)data;

  app_timer_expire = true;
}

