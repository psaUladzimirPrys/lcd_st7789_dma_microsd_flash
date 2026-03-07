/*
 * ui_display.c
 *
 *  Created on: 30 янв. 2026 г.
 *      Author: priss
 */

/*=======================================================================*/
/*        I N C L U D E S                                                */
/*=======================================================================*/
#include "disp.h"
#include "app_log.h"
#include "app_assert.h"
#include "sl_sleeptimer.h"



/*=======================================================================*/
/* L O C A L   D E F I N I T I O N S                                     */
/*=======================================================================*/
#define DISP_TIMER_SMALL        300
#define DISP_TIMER_MEDIUM       500
#define DISP_TIMER_LARGE        1000



/*=======================================================================*/
/* L O C A L   S Y M B O L   D E C L A R A T I O N S                     */
/*=======================================================================*/

/*=======================================================================*/
/* UP Display configuration section declaration  variables         */
/*=======================================================================*/

static glib_context_t g_context;


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

static volatile bool disp_timer_expire = false;
static sl_sleeptimer_timer_handle_t disp_timer;


/*=======================================================================*/
/*    L O C A L   F U N C T I O N   P R O T O T Y P E S                  */
/*=======================================================================*/
static void disp_timer_callback(sl_sleeptimer_timer_handle_t *timer, void *data);



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


/*=======================================================================*/
/*    L O C A L   F U N C T I O N                                        */
/*=======================================================================*/
static void disp_timer_callback(sl_sleeptimer_timer_handle_t *timer, void *data)
{
  (void)timer;
  (void)data;

  disp_timer_expire = true;
}


/*=======================================================================*/
/* F U N C T I O N S                                                     */
/*=======================================================================*/

void disp_Init(void)
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

  sl_sleeptimer_restart_periodic_timer_ms(&disp_timer,
                                          DISP_TIMER_SMALL,
                                          disp_timer_callback,
                                          NULL,
                                          0,
                                          0);
  app_log("Start sleep timer %dms\r\n", DISP_TIMER_SMALL);

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
void disp_Update(void)
{

}

void disp_TurnOn(void)
{
  glib_enable_display(true);
}

void disp_TurnOff(void)
{
  glib_enable_display(false);
}


void disp_DrawImage(int16_t x, int16_t y, img_storage_id_t img_id)
{
   adafruit_st7789_draw_rgb_bitmap_from_flash(x, y, IMG_GET_WIDTH(img_id), IMG_GET_HEIGHT(img_id), IMG_GET_ADDRESS(img_id),  true);
}

void disp_EraseImage(int16_t x, int16_t y, img_storage_id_t img_id, uint16_t bg_color)
{
  adafruit_st7789_fill_rectangle(x, y, IMG_GET_WIDTH(img_id), IMG_GET_HEIGHT(img_id), bg_color);
}
