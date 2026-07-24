/*
 * ui_display.c
 *
 *  Created on: 30 Jan. 2026
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

/*--- Global/Static variables ---*/
/*=======================================================================*/
/* GLIB drawing context: holds viewport, colors, text settings     */
/*=======================================================================*/
static glib_context_t g_context;

/* ST7789 SPI interface configuration */
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

/* Periodic sleeptimer handle used for display refresh scheduling */
static sl_sleeptimer_timer_handle_t disp_timer;
/* Flag set by sleeptimer callback to indicate timer expiration */
static volatile bool disp_timer_expire = false;


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


/*=================================================================================
*   Function:    disp_timer_callback
*   Description: Sleeptimer callback that signals timer expiration to the display loop.
*
*   Arguments:
*   Parameter    Flow    Description
*   ------------------------------------------------------------------------------
*   timer    IN    Sleeptimer handle that expired (unused)
*   data     IN    User data pointer (unused)
*
*   Externals    Flow    Usage
*   ------------------------------------------------------------------------------
*   disp_timer_expire OUT  Set to true to notify main loop
*
*   @return void
*
*   @note Called from SL sleeptimer ISR context. Sets volatile flag only; no heavy processing.
===================================================================================*/
static void disp_timer_callback(sl_sleeptimer_timer_handle_t *timer, void *data)
{
  (void)timer;
  (void)data;

  disp_timer_expire = true;
}


/*=================================================================================
*   Function:    disp_Init
*   Description: Initializes the ST7789 TFT display, GLIB graphics context, and the
*                periodic sleeptimer for display refresh scheduling.
*
*   Arguments:        None
*
*   Externals    Flow    Usage
*   ------------------------------------------------------------------------------
*   g_context      OUT  Initialized by glib_init(), configured with colors and text size
*   disp_timer     OUT  Periodic timer handle started with DISP_TIMER_SMALL
*   st7789_config  IN   SPI interface configuration passed to driver init
*
*   @return void
*
*   @note Asserts on any hardware init failure. Display output is disabled until disp_TurnOn().
===================================================================================*/
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


  glib_set_color(&g_context, ST7789_GREEN, ST7789_BLACK);
  glib_fill(&g_context, ST7789_GREEN);


}

/*=================================================================================
*   Function:    disp_Update
*   Description: Display tick placeholder. Intended for per-tick display refresh work.
*
*   Arguments:        None
*
*   Externals    Flow    Usage
*   ------------------------------------------------------------------------------
*   disp_timer_expire IN  Volatile flag checked for timer expiration
*
*   @return void
*
*   @note Currently empty; reserved for future display refresh logic.
===================================================================================*/
void disp_Update(void)
{

}

/*=================================================================================
*   Function:    disp_TurnOn
*   Description: Enables the ST7789 display output and fills the screen with white.
*
*   Arguments:        None
*
*   Externals    Flow    Usage
*   ------------------------------------------------------------------------------
*   g_context IN  Passed to glib_fill() and glib_enable_display()
*
*   @return void
*
*   @note Asserts on GLIB error. Requires prior disp_Init().
===================================================================================*/
void disp_TurnOn(void)
{
  glib_status_t status = GLIB_OK;

  status = glib_fill(&g_context, ST7789_WHITE);
  if (status != GLIB_OK)  {
    app_assert_status(SL_STATUS_FAIL);
  }

  status = glib_enable_display(true);
  if (status != GLIB_OK)  {
    app_assert_status(SL_STATUS_FAIL);
  }

}

/*=================================================================================
*   Function:    disp_TurnOff
*   Description: Fills the screen with black and disables the ST7789 display output.
*
*   Arguments:        None
*
*   Externals    Flow    Usage
*   ------------------------------------------------------------------------------
*   g_context IN  Passed to glib_fill() and glib_enable_display()
*
*   @return void
*
*   @note Asserts on GLIB error. Requires prior disp_Init().
===================================================================================*/
void disp_TurnOff(void)
{
  glib_status_t status = GLIB_OK;

  status = glib_enable_display(false);
  if (status != GLIB_OK)  {
    app_assert_status(SL_STATUS_FAIL);
  }

  status = glib_fill(&g_context, ST7789_BLACK);
  if (status != GLIB_OK)  {
    app_assert_status(SL_STATUS_FAIL);
  }

}

/*=================================================================================
*   Function:    disp_DrawImage
*   Description: Draws an RGB bitmap from flash storage to the ST7789 display.
*
*   Arguments:
*   Parameter    Flow    Description
*   ------------------------------------------------------------------------------
*   x        IN   X coordinate of top-left corner
*   y        IN   Y coordinate of top-left corner
*   img_id   IN   Image storage ID used to obtain width, height, and flash address
*
*   Externals    Flow    Usage
*   ------------------------------------------------------------------------------
*   None
*
*   @return void
*
*   @note Asserts on driver error. Image data is read directly from flash via IMG_GET_* macros.
===================================================================================*/
void disp_DrawImage(int16_t x, int16_t y, img_storage_id_t img_id)
{
  sl_status_t status = SL_STATUS_OK;
  status = adafruit_st7789_draw_rgb_bitmap_from_flash(x, y, IMG_GET_WIDTH(img_id), IMG_GET_HEIGHT(img_id), IMG_GET_ADDRESS(img_id),  true);
  if (status != SL_STATUS_OK)  {
    app_assert_status(status);
  }
}

/*=================================================================================
*   Function:    disp_EraseImage
*   Description: Fills a rectangular region on the ST7789 display with the specified background color.
*
*   Arguments:
*   Parameter    Flow    Description
*   ------------------------------------------------------------------------------
*   x         IN   X coordinate of top-left corner
*   y         IN   Y coordinate of top-left corner
*   width     IN   Rectangle width in pixels
*   height    IN   Rectangle height in pixels
*   bg_color  IN   Background color value (16-bit RGB565)
*
*   Externals    Flow    Usage
*   ------------------------------------------------------------------------------
*   None
*
*   @return void
*
*   @note Asserts on driver error. Useful for clearing previously drawn image areas.
===================================================================================*/
void disp_EraseImage(int16_t x, int16_t y, int16_t width, int16_t height, uint16_t bg_color)
{
  sl_status_t status = SL_STATUS_OK;

  status = adafruit_st7789_fill_rectangle(x, y, width, height, bg_color);
  if (status != SL_STATUS_OK)  {
    app_assert_status(status);
  }

}

/*=================================================================================
*   Function:    disp_FillScreen
*   Description: Fills the entire display with the specified solid color using GLIB.
*
*   Arguments:
*   Parameter    Flow    Description
*   ------------------------------------------------------------------------------
*   color    IN   16-bit RGB565 color value
*
*   Externals    Flow    Usage
*   ------------------------------------------------------------------------------
*   g_context IN  Passed to glib_fill() to perform full-screen fill
*
*   @return void
*
*   @note Asserts on GLIB error. Requires prior disp_Init().
===================================================================================*/
void disp_FillScreen(int16_t color)
{
  glib_status_t status = GLIB_OK;

  status = glib_fill(&g_context, color);
  if (status != GLIB_OK)  {
    app_assert_status(SL_STATUS_FAIL);
  }

}
