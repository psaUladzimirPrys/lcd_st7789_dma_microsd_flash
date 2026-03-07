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

#include "event_simulation.h"
#include "app_log.h"
#include "app_assert.h"
#include "sl_sleeptimer.h"
#include "img_storage.h"
#include "fsrv.h"
#include "file_storage.h"


/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/

#define APP_TIMER_SMALL        300
#define APP_TIMER_MEDIUM       2000
#define APP_TIMER_LARGE        7000



/***************************************************************************//**
*  /UP Display configuration section declaration global variables
*******************************************************************************/



static uint8_t demo_index = 0;



static volatile bool app_timer_expire = false;
static sl_sleeptimer_timer_handle_t app_sleep_timer;
static void app_sleeptimer_callback(sl_sleeptimer_timer_handle_t *timer,
                                    void *data);



/*******************************************************************************
 *********************   LOCAL FUNCTION PROTOTYPES   ***************************
 ******************************************************************************/


void sim_Init(void)
{

   app_timer_expire = false;

   app_log("Simulator UI init done\r\n");

  sl_sleeptimer_restart_periodic_timer_ms(&app_sleep_timer,
                                          APP_TIMER_LARGE,
                                          app_sleeptimer_callback,
                                          NULL,
                                          0,
                                          0);
  app_log("Start UI SIM sleep timer %dms\r\n", APP_TIMER_LARGE);
}

/***************************************************************************//**
 * Display ticking function.
 ******************************************************************************/
void sim_Update(void)
{

  if (app_timer_expire) {
    app_timer_expire = false;

    switch (demo_index) {
      case 0:
        sl_sleeptimer_restart_periodic_timer_ms(&app_sleep_timer,
                                                APP_TIMER_LARGE,
                                                app_sleeptimer_callback,
                                                NULL,
                                                0,
                                                0);
        FSLOG_ALL("Start UI SIM sleep timer %dms\r\n", APP_TIMER_LARGE);

        demo_index++;
        break;
      case 1:
        fsrv_DS_SetBatStatus(BAT_NORMAL);
        FSLOG_ALL("UI SIM = 0\r\n");
        break;

      case 2:
        fsrv_DS_SetBatStatus(BAT_LOW_50);
        FSLOG_ALL("UI SIM = 1\r\n");
        break;

      case 3:
        fsrv_DS_SetBatStatus(BAT_LOW_30);
        FSLOG_ALL("UI SIM = 2\r\n");
        break;

      case 4:
        fsrv_DS_SetBatStatus(BAT_LOW_10);
        FSLOG_ALL("UI SIM = 3\r\n");
        break;

      case 5:
        fsrv_DS_SetBatStatus(BAT_CRITICAL);
        sl_sleeptimer_restart_periodic_timer_ms(&app_sleep_timer,
                                                APP_TIMER_MEDIUM,
                                                app_sleeptimer_callback,
                                                NULL,
                                                0,
                                                0);

        app_log("UI SIM = 4\r\n");
        break;

      case 6:
        fsrv_DS_SetBatStatus(BAT_CHARGING);
        sl_sleeptimer_restart_periodic_timer_ms(&app_sleep_timer,
                                                APP_TIMER_LARGE,
                                                app_sleeptimer_callback,
                                                NULL,
                                                0,
                                                0);

        FSLOG_ALL("UI SIM = 5\r\n");
        break;

      case 7:
        sl_sleeptimer_restart_periodic_timer_ms(&app_sleep_timer,
                                                APP_TIMER_MEDIUM,
                                                app_sleeptimer_callback,
                                                NULL,
                                                0,
                                                0);

        FSLOG_ALL("UI SIM = 6\r\n");
        break;

      case 8:
        sl_sleeptimer_restart_periodic_timer_ms(&app_sleep_timer,
                                                APP_TIMER_LARGE,
                                                app_sleeptimer_callback,
                                                NULL,
                                                0,
                                                0);
        FSLOG_ALL("UI SIM = 7\r\n");
        break;

      case 9:
        app_log("UI SIM = 8\r\n");
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

