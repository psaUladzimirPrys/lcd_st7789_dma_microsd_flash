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
        glib_fill(&g_context, ST7789_WHITE);
        app_log("glib_fill --> WHITE color\r\n");
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


        break;

      case 2:
        glib_fill(&g_context, ST7789_WHITE);
        adafruit_st7789_fill_rectangle(0, 0, 320, 44, 0xF79E);

        //battery 100
        adafruit_st7789_draw_rgb_bitmap_from_flash(22, 10,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_BATTERY_100].width,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_BATTERY_100].height,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_BATTERY_100].storage_address,
                                                   true);

        adafruit_st7789_fill_rectangle(0, 137, 320, 35, 0xCE79);

        //Waiting to connect
        adafruit_st7789_draw_rgb_bitmap_from_flash(34, 73,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_DEFAULT_7].width,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_DEFAULT_7].height,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_DEFAULT_7].storage_address,
                                                   true);
        // PERF CHECK
        adafruit_st7789_draw_rgb_bitmap_from_flash(37, 143,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_DEFAULT_14].width,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_DEFAULT_14].height,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_DEFAULT_14].storage_address,
                                                   true);
        // .
        adafruit_st7789_draw_rgb_bitmap_from_flash(19,  151,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_ELLIPSE_15].width,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_ELLIPSE_15].height,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_ELLIPSE_15].storage_address,
                                                   true);
        //PARAMS
        adafruit_st7789_draw_rgb_bitmap_from_flash(216, 143,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_DEFAULT_15].width,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_DEFAULT_15].height,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_DEFAULT_15].storage_address,
                                                   true);
        // ...
        adafruit_st7789_draw_rgb_bitmap_from_flash(170, 151,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_FRAME_53].width,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_FRAME_53].height,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_FRAME_53].storage_address,
                                                   true);

        sl_sleeptimer_restart_periodic_timer_ms(&app_sleep_timer,
                                                APP_TIMER_LARGE,
                                                app_sleeptimer_callback,
                                                NULL,
                                                0,
                                                0);

        app_log("Menu = 1\r\n");
        break;

      case 3:
        //glib_fill(&g_context, ST7789_WHITE);
        adafruit_st7789_fill_rectangle(0, 0, 320, 44, 0xF79E);

        //battery 50
        adafruit_st7789_draw_rgb_bitmap_from_flash(22, 10,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_VARIANT2_8].width,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_VARIANT2_8].height,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_VARIANT2_8].storage_address,
                                                   true);
        //link
        adafruit_st7789_draw_rgb_bitmap_from_flash(63, 9,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_LINK].width,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_LINK].height,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_LINK].storage_address,
                                                   true);
        //Sync
        adafruit_st7789_draw_rgb_bitmap_from_flash(92, 9,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_ARROW_PATH].width,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_ARROW_PATH].height,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_ARROW_PATH].storage_address,
                                                   true);

        //CLEAR BEFORE DRAW
        adafruit_st7789_fill_rectangle(34, 73,
                                       img_storage_desc[IMG_ID_PROPERTY_1_DEFAULT_7].width,
                                       img_storage_desc[IMG_ID_PROPERTY_1_DEFAULT_7].height,
                                       ST7789_WHITE);

        //PertCheck required
        adafruit_st7789_draw_rgb_bitmap_from_flash(34, 73,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_VARIANT4_6].width,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_VARIANT4_6].height,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_VARIANT4_6].storage_address,
                                                   true);

        adafruit_st7789_fill_rectangle(0, 137, 320, 35, 0xCE79);

        // START
        adafruit_st7789_draw_rgb_bitmap_from_flash(45, 143,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_DEFAULT_13].width,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_DEFAULT_13].height,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_DEFAULT_13].storage_address,
                                                   true);
        // .
        adafruit_st7789_draw_rgb_bitmap_from_flash(19,  151,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_ELLIPSE_15].width,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_ELLIPSE_15].height,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_ELLIPSE_15].storage_address,
                                                   true);
        //PARAMS
        adafruit_st7789_draw_rgb_bitmap_from_flash(216, 143,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_DEFAULT_15].width,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_DEFAULT_15].height,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_DEFAULT_15].storage_address,
                                                   true);
        // ...
        adafruit_st7789_draw_rgb_bitmap_from_flash(170, 151,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_FRAME_53].width,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_FRAME_53].height,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_FRAME_53].storage_address,
                                                   true);
        app_log("Menu = 2\r\n");
        break;
      case 4:
        adafruit_st7789_fill_rectangle(0, 0, 320, 44, 0xF79E);

        //battery 30
        adafruit_st7789_draw_rgb_bitmap_from_flash(22, 10,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_VARIANT3_8].width,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_VARIANT3_8].height,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_VARIANT3_8].storage_address,
                                                   true);
        //link
        adafruit_st7789_draw_rgb_bitmap_from_flash(63, 9,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_LINK].width,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_LINK].height,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_LINK].storage_address,
                                                   true);
        //Sync
        adafruit_st7789_draw_rgb_bitmap_from_flash(92, 9,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_ARROW_PATH].width,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_ARROW_PATH].height,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_ARROW_PATH].storage_address,
                                                   true);

        //CLEAR BEFORE DRAW
        adafruit_st7789_fill_rectangle(34, 73,
                                       img_storage_desc[IMG_ID_PROPERTY_1_VARIANT4_6].width,
                                       img_storage_desc[IMG_ID_PROPERTY_1_VARIANT4_6].height,
                                       ST7789_WHITE);

        //Waiting for TIP ID
        adafruit_st7789_draw_rgb_bitmap_from_flash(34, 73,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_VARIANT2_7].width,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_VARIANT2_7].height,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_VARIANT2_7].storage_address,
                                                   true);

        adafruit_st7789_fill_rectangle(0, 137, 320, 35, 0xCE79);

        // PERF CHECK
        adafruit_st7789_draw_rgb_bitmap_from_flash(37, 143,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_DEFAULT_14].width,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_DEFAULT_14].height,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_DEFAULT_14].storage_address,
                                                   true);
        // .
        adafruit_st7789_draw_rgb_bitmap_from_flash(19,  151,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_ELLIPSE_15].width,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_ELLIPSE_15].height,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_ELLIPSE_15].storage_address,
                                                   true);
        //PARAMS
        adafruit_st7789_draw_rgb_bitmap_from_flash(216, 143,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_DEFAULT_15].width,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_DEFAULT_15].height,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_DEFAULT_15].storage_address,
                                                   true);
        // ...
        adafruit_st7789_draw_rgb_bitmap_from_flash(170, 151,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_FRAME_53].width,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_FRAME_53].height,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_FRAME_53].storage_address,
                                                   true);

        app_log("Menu = 3\r\n");
        break;
      case 5:
        adafruit_st7789_fill_rectangle(0, 0, 320, 44, 0xF79E);

        //battery 30
        adafruit_st7789_draw_rgb_bitmap_from_flash(22, 10,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_VARIANT3_8].width,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_VARIANT3_8].height,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_VARIANT3_8].storage_address,
                                                   true);
        //link
        adafruit_st7789_draw_rgb_bitmap_from_flash(63, 9,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_LINK].width,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_LINK].height,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_LINK].storage_address,
                                                   true);
        //Sync
        adafruit_st7789_draw_rgb_bitmap_from_flash(92, 9,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_ARROW_PATH].width,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_ARROW_PATH].height,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_ARROW_PATH].storage_address,
                                                   true);
        //Patient
        adafruit_st7789_draw_rgb_bitmap_from_flash(217, 9,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_PATIENT].width,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_PATIENT].height,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_PATIENT].storage_address,
                                                   true);

        //CLEAR BEFORE DRAW
        adafruit_st7789_fill_rectangle(34, 73,
                                       img_storage_desc[IMG_ID_PROPERTY_1_VARIANT2_7].width,
                                       img_storage_desc[IMG_ID_PROPERTY_1_VARIANT2_7].height,
                                       ST7789_WHITE);

        //Validating
        adafruit_st7789_draw_rgb_bitmap_from_flash(85, 73,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_VARIANT5_6].width,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_VARIANT5_6].height,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_VARIANT5_6].storage_address,
                                                   true);

        adafruit_st7789_fill_rectangle(0, 137, 320, 35, 0xCE79);

        //CANCEL
        adafruit_st7789_draw_rgb_bitmap_from_flash(216, 143,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_DEFAULT_11].width,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_DEFAULT_11].height,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_DEFAULT_11].storage_address,
                                                   true);
        // -
        adafruit_st7789_draw_rgb_bitmap_from_flash(170, 151,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_RECTANGLE_201].width,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_RECTANGLE_201].height,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_RECTANGLE_201].storage_address,
                                                   true);
        sl_sleeptimer_restart_periodic_timer_ms(&app_sleep_timer,
                                                APP_TIMER_MEDIUM,
                                                app_sleeptimer_callback,
                                                NULL,
                                                0,
                                                0);

        app_log("Menu = 4\r\n");
        break;
      case 6:
        //Indicator
        //RED LINE
        adafruit_st7789_fill_rectangle(0, 89, 320, 48, 0xFE79);
        //TIP ID INVALID
        adafruit_st7789_draw_rgb_bitmap_from_flash(69, 98,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_VARIANT2].width,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_VARIANT2].height,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_VARIANT2].storage_address,
                                                   true);

        //Bottom line
        adafruit_st7789_fill_rectangle(0, 137, 320, 35, 0xCE79);

        //CLOSE
        adafruit_st7789_draw_rgb_bitmap_from_flash(216, 143,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_DEFAULT_10].width,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_DEFAULT_10].height,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_DEFAULT_10].storage_address,
                                                   true);

        // .
        adafruit_st7789_draw_rgb_bitmap_from_flash(195,  151,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_ELLIPSE_15].width,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_ELLIPSE_15].height,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_ELLIPSE_15].storage_address,
                                                   true);

        sl_sleeptimer_restart_periodic_timer_ms(&app_sleep_timer,
                                                APP_TIMER_LARGE,
                                                app_sleeptimer_callback,
                                                NULL,
                                                0,
                                                0);

        app_log("Menu = 4 Indicator: TIP ID INVALID\r\n");
        break;
      case 7:
        adafruit_st7789_fill_rectangle(0, 0, 320, 44, 0xF79E);

        //battery 30
        adafruit_st7789_draw_rgb_bitmap_from_flash(22, 10,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_VARIANT3_8].width,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_VARIANT3_8].height,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_VARIANT3_8].storage_address,
                                                   true);
        //link
        adafruit_st7789_draw_rgb_bitmap_from_flash(63, 9,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_LINK].width,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_LINK].height,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_LINK].storage_address,
                                                   true);
        //Sync
        adafruit_st7789_draw_rgb_bitmap_from_flash(92, 9,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_ARROW_PATH].width,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_ARROW_PATH].height,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_ARROW_PATH].storage_address,
                                                   true);

        //CLEAR BEFORE DRAW
        adafruit_st7789_fill_rectangle(0, 73,320, 48+16,
                                       ST7789_WHITE);

        //Waiting for TIP ID
        adafruit_st7789_draw_rgb_bitmap_from_flash(34, 73,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_VARIANT2_7].width,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_VARIANT2_7].height,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_VARIANT2_7].storage_address,
                                                   true);



        adafruit_st7789_fill_rectangle(0, 137, 320, 35, 0xCE79);

        // PERF CHECK
        adafruit_st7789_draw_rgb_bitmap_from_flash(37, 143,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_DEFAULT_14].width,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_DEFAULT_14].height,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_DEFAULT_14].storage_address,
                                                   true);
        // .
        adafruit_st7789_draw_rgb_bitmap_from_flash(19,  151,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_ELLIPSE_15].width,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_ELLIPSE_15].height,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_ELLIPSE_15].storage_address,
                                                   true);
        //PARAMS
        adafruit_st7789_draw_rgb_bitmap_from_flash(216, 143,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_DEFAULT_15].width,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_DEFAULT_15].height,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_DEFAULT_15].storage_address,
                                                   true);
        // ...
        adafruit_st7789_draw_rgb_bitmap_from_flash(170, 151,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_FRAME_53].width,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_FRAME_53].height,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_FRAME_53].storage_address,
                                                   true);
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
        adafruit_st7789_draw_rgb_bitmap_from_flash(22, 10,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_VARIANT4_7].width,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_VARIANT4_7].height,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_VARIANT4_7].storage_address,
                                                   true);

        //Indicator
        //BEIGE LINE
        adafruit_st7789_fill_rectangle(0, 89, 320, 48, 0xFF39);
        //Charge THE BATTERY
        adafruit_st7789_draw_rgb_bitmap_from_flash(19, 98,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_VARIANT3].width,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_VARIANT3].height,
                                                   img_storage_desc[IMG_ID_PROPERTY_1_VARIANT3].storage_address,
                                                   true);
        sl_sleeptimer_restart_periodic_timer_ms(&app_sleep_timer,
                                                APP_TIMER_LARGE,
                                                app_sleeptimer_callback,
                                                NULL,
                                                0,
                                                0);
        app_log("Menu = 3 Indicator: CHARGE THE BATTERY\r\n");
        break;
      case 9:
      adafruit_st7789_fill_rectangle(0, 0, 320, 44, 0xF79E);

      //battery -0
      adafruit_st7789_draw_rgb_bitmap_from_flash(22, 10,
                                                 img_storage_desc[IMG_ID_PROPERTY_1_VARIANT6_7].width,
                                                 img_storage_desc[IMG_ID_PROPERTY_1_VARIANT6_7].height,
                                                 img_storage_desc[IMG_ID_PROPERTY_1_VARIANT6_7].storage_address,
                                                 true);

      adafruit_st7789_fill_rectangle(0, 44, 320, 93, ST7789_WHITE);


      adafruit_st7789_fill_rectangle(0, 137, 320, 35, 0xCE79);
      //Charge the battery
      adafruit_st7789_draw_rgb_bitmap_from_flash(40, 75,
                                                 img_storage_desc[IMG_ID_PROPERTY_1_VARIANT3_7].width,
                                                 img_storage_desc[IMG_ID_PROPERTY_1_VARIANT3_7].height,
                                                 img_storage_desc[IMG_ID_PROPERTY_1_VARIANT3_7].storage_address,
                                                 true);

      //PARAMS
     adafruit_st7789_draw_rgb_bitmap_from_flash(216, 143,
                                                img_storage_desc[IMG_ID_PROPERTY_1_DEFAULT_15].width,
                                                img_storage_desc[IMG_ID_PROPERTY_1_DEFAULT_15].height,
                                                img_storage_desc[IMG_ID_PROPERTY_1_DEFAULT_15].storage_address,
                                                true);
     // ...
     adafruit_st7789_draw_rgb_bitmap_from_flash(170, 151,
                                                img_storage_desc[IMG_ID_PROPERTY_1_FRAME_53].width,
                                                img_storage_desc[IMG_ID_PROPERTY_1_FRAME_53].height,
                                                img_storage_desc[IMG_ID_PROPERTY_1_FRAME_53].storage_address,
                                                true);
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

static void app_sleeptimer_callback(sl_sleeptimer_timer_handle_t *timer,
                                    void *data)
{
  (void)timer;
  (void)data;

  app_timer_expire = true;
}

