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
#include "find_api.h"
#include "fpmt_api.h"
#include "auph.h"
#include "fmnu.h"

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/

#define APP_TIMER_SMALL        300
#define APP_TIMER_MEDIUM       2000
#define APP_TIMER_LARGE        9000



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
void sim_TurnOn(void)
{
  app_timer_expire = false;
  demo_index = 0;

  sl_sleeptimer_restart_periodic_timer_ms(&app_sleep_timer,
                                          APP_TIMER_MEDIUM,
                                          app_sleeptimer_callback,
                                          NULL,
                                          0,
                                          0);

  app_log("Turn On UI SIM\r\n");

  set_device_state(DEVICE_LOADING);
}

void sim_TurnOff(void)
{
  app_log("Turn Off UI SIM\r\n");

  set_device_state(DEVICE_STANDBY);
}



void sim_Init(void)
{

  app_timer_expire = false;
  demo_index = 0;

  app_log("Simulator UI init done\r\n");

  sl_sleeptimer_restart_periodic_timer_ms(&app_sleep_timer,
                                          APP_TIMER_SMALL,
                                          app_sleeptimer_callback,
                                          NULL,
                                          0,
                                          0);

  app_log("Start UI SIM sleep timer: %d ms\r\n", APP_TIMER_SMALL);
}

/***************************************************************************//**
 * Display ticking function.
 ******************************************************************************/
void sim_Update(void)
{

  if (app_timer_expire) {
    app_timer_expire = false;

    if (fpmt_GetPowerState() != FPMT_POWER_ON)
    {
       return;
    }

    switch (demo_index) {
      case 0:
        sl_sleeptimer_restart_periodic_timer_ms(&app_sleep_timer,
                                                APP_TIMER_LARGE,
                                                app_sleeptimer_callback,
                                                NULL,
                                                0,
                                                0);

          if (DEVICE_LOADING == get_device_state())
          {
            find_RemoveIndicator(FIND_ID_SPLASH_SCREEN);

            adafruit_st7789_fill_rectangle(0, 0, FUIM_TITLE_LEFT_MARGIN, FUIM_TOP_FIELD_HEIGHT, FUIM_COLOUR_1);

            fmnu_Activate(AUIM_MNU_INDEX_IDLE_MENU);
            auph_SetState(AU_IDLE_STATE);
            set_device_state(DEVICE_IDLE);
          }
          //demo_index++;

          FSLOG_ALL("Start DEVICE_IDLE  sleep timer %dms\r\n", APP_TIMER_LARGE);

      break;

      case 1:
        find_SetRestoreAllIndicators(true);

        fsrv_DS_SetBatStatus(BAT_NORMAL);
        fsrv_DS_SetBleStatus(BLE_ERROR);
        fsrv_DS_SetSyncStatus(SYNC_FAILED);

        app_log("UI SIM = 1\r\n");
        break;

      case 2:

        fsrv_DS_SetBatStatus(BAT_NORMAL);
        fsrv_DS_SetBleStatus(BLE_CONNECTED);
        fsrv_DS_SetSyncStatus(SYNC_COMPLETED);

        app_log("UI SIM = 2\r\n");
        break;

      case 3:
        fsrv_DS_SetBatStatus(BAT_LOW_50);
        fsrv_DS_SetSyncStatus(SYNC_IN_PROGRESS);

        sl_sleeptimer_restart_periodic_timer_ms(&app_sleep_timer,
                                                APP_TIMER_LARGE,
                                                app_sleeptimer_callback,
                                                NULL,
                                                0,
                                                0);

        app_log("UI SIM = 3\r\n");
        break;

      case 4:
        fsrv_DS_SetBatStatus(BAT_LOW_30);
        fsrv_DS_SetBleStatus(BLE_DISCONNECTED);

        app_log("UI SIM = 4\r\n");
        break;

      case 5:
        sl_sleeptimer_restart_periodic_timer_ms(&app_sleep_timer,
                                                (APP_TIMER_LARGE * 2),
                                                app_sleeptimer_callback,
                                                NULL,
                                                0,
                                                0);
        fsrv_DS_SetBatStatus(BAT_LOW_10);
        fsrv_DS_SetBleStatus(BLE_CONNECTED);
        find_DisplayIndicator(FIND_ID_CHARGE_BATT);
        app_log("UI SIM = 5\r\n");
        break;

      case 6:
        sl_sleeptimer_restart_periodic_timer_ms(&app_sleep_timer,
                                                (APP_TIMER_MEDIUM),
                                                app_sleeptimer_callback,
                                                NULL,
                                                0,
                                                0);
        fsrv_DS_SetBatStatus(BAT_CRITICAL);
        app_log("UI SIM = 6\r\n");
        break;

      case 7:

        app_log("UI SIM = 7\r\n");
        break;

      case 8:
        fsrv_DS_SetBatStatus(BAT_CHARGING);
        find_RemoveIndicator(FIND_ID_CHARGE_BATT);
        app_log("UI SIM = 8\r\n");
        break;

      case 9:

        app_log("UI SIM = 9\r\n");
        break;

      case 10:
        fsrv_DS_SetBatStatus(BAT_ERROR);
        app_log("UI SIM = 10\r\n");
      break;

      default:
        break;
    }

    demo_index++;

    if(demo_index > 10) {
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

