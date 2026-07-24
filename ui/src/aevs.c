/*
 * ui_display.c
 *
 *  Created on: 30 Jun. 2026 г.
 *      Author: priss
 */
/*=======================================================================*/
/*        I N C L U D E S                                                */
/*=======================================================================*/
#include "app_log.h"
#include "app_assert.h"
#include "sl_sleeptimer.h"
#include "aevs.h"
#include "fsrv.h"
#include "file_storage.h"
#include "find_api.h"
#include "fpmt_api.h"
#include "auph.h"
#include "fmnu.h"
#include "rbsc_api.h"
#include "disp.h"

/*=======================================================================*/
/* L O C A L   D A T A   D E F I N I T I O N S                           */
/*=======================================================================*/
#define AEVS_TIMER_SMALL        300
#define AEVS_TIMER_MEDIUM       2000
#define AEVS_TIMER_LARGE        9000

#define AEVS_TIMER_STOPPED      FALSE
#define AEVS_TIMER_EXPIRED      TRUE

/*=======================================================================*/
/* L O C A L   S Y M B O L   D E C L A R A T I O N S                     */
/*=======================================================================*/


static aevs_active_task_idx_t aevs_active_task = AEVS_NO_ACTIVE_TASK;

static volatile Bool aevs_update_timer_expired = FALSE;
static sl_sleeptimer_timer_handle_t aevs_update_timer;

/*=======================================================================*/
/* L O C A L   F U N C T I O N   P R O T O T Y P E S                     */
/*=======================================================================*/

static void aevs_update_timer_callback(sl_sleeptimer_timer_handle_t *timer, void *data);
static aevs_active_task_idx_t aevs_StartupLogoIndicatorHandle(void);
static aevs_active_task_idx_t aevs_PerformanceRequiredIndicatorHandle (void);
static aevs_active_task_idx_t aevs_ChargeBatteryRequiredIndicatorHandle (void);

uint8_t fsrv_DS_StepOnePingPongValue(void);
void aevs_BatteryStatusUpdate(void);


/*=======================================================================*/
/* L O C A L   F U N C T I O N S                                         */
/*=======================================================================*/

static aevs_active_task_idx_t aevs_StartupLogoIndicatorHandle(void) {

  aevs_active_task_idx_t next_active_task = AEVS_LOADING_ACTIVE_TASK;

  if ( auph_GetState() == AU_STARTUP_STATE ) {

    find_RemoveIndicator(FIND_ID_SPLASH_SCREEN);

    disp_EraseImage(0, 0, FUIM_TITLE_LEFT_MARGIN, FUIM_TOP_FIELD_HEIGHT, FUIM_COLOUR_1);
    aukh_Post_UI_Event(AU_IDLE_MENU_START);
    auph_SetState(AU_IDLE_STATE);

    find_SetRestoreAllIndicators(true);

    sl_sleeptimer_restart_periodic_timer_ms(&aevs_update_timer,
                                            AEVS_TIMER_MEDIUM,
                                            aevs_update_timer_callback,
                                            NULL,
                                            0,
                                            0);

    next_active_task = AEVS_BATTERY_ACTIVE_TASK;

    FSLOG_ALL("Start DEVICE_IDLE  sleep timer %dms\r\n", AEVS_TIMER_MEDIUM);
  }

  return next_active_task;
}


/*=======================================================================*/
/*                                                                       */
/*=======================================================================*/
static aevs_active_task_idx_t aevs_PerformanceRequiredIndicatorHandle (void)
{
  aevs_active_task_idx_t next_active_task = AEVS_PERF_CHECK_ACTIVE_TASK;

  if ( auph_GetState() != AU_IDLE_STATE) {
      next_active_task = AEVS_NO_ACTIVE_TASK;
      return next_active_task;
  }

  if ( !find_IsIndicatorDisplayed(FIND_ID_CHARGE_BATT) ) {

      if (   ( FALSE == fsrv_DS_IsIdleWaitingStatUpdate() )
          && ( TRUE  == fsrv_DS_IsPerformanceRequired()   )
      ) {

          if (!find_IsIndicatorDisplayed(FIND_ID_PERF_CHK_RECOMMENDED)) {
             find_DisplayIndicator(FIND_ID_PERF_CHK_RECOMMENDED);
          }
      } else {
            find_RemoveIndicator(FIND_ID_PERF_CHK_RECOMMENDED);
      }
  }

  next_active_task = AEVS_BATTERY_ACTIVE_TASK;

  return next_active_task;
}

/*=======================================================================*/
/*                                                                       */
/*=======================================================================*/
static aevs_active_task_idx_t aevs_ChargeBatteryRequiredIndicatorHandle (void)
{
  aevs_active_task_idx_t next_active_task = AEVS_BATTERY_ACTIVE_TASK;

  if ( auph_GetState() != AU_IDLE_STATE ) {
      next_active_task = AEVS_NO_ACTIVE_TASK;
      return next_active_task;
  }

  if ( fsrv_DS_GetBatStatus() == BAT_LOW_10 ) {

      if ( find_IsIndicatorDisplayed(FIND_ID_PERF_CHK_RECOMMENDED) ) {
          find_RemoveIndicator(FIND_ID_PERF_CHK_RECOMMENDED);
      }

     if ( FALSE == fsrv_DS_IsIdleWaitingStatUpdate()) {
        if ( !find_IsIndicatorDisplayed(FIND_ID_CHARGE_BATT)) {
          find_DisplayIndicator(FIND_ID_CHARGE_BATT);
        }
     } else {
        find_RemoveIndicator(FIND_ID_CHARGE_BATT);
     }

     next_active_task = AEVS_BATTERY_ACTIVE_TASK;

  } else {
    find_RemoveIndicator(FIND_ID_CHARGE_BATT);

    next_active_task = AEVS_PERF_CHECK_ACTIVE_TASK;
  }


  return next_active_task;
}

/*=======================================================================*/
/*                                                                       */
/*=======================================================================*/
void aevs_SyncSessionStatusUpdate(void) {

  if (fsrv_DS_IsSyncActive()) {
    fsrv_DS_SetSyncStatus(SYNC_IN_PROGRESS);
  } else {
    fsrv_DS_SetSyncStatus(SYNC_IDLE);
  }

}

/*=======================================================================*/
/*                                                                       */
/*=======================================================================*/
void aevs_BatteryStatusUpdate(void) {
  static uint8_t last_bat_level;
  
  uint8_t bat_level  = fsrv_DS_GetBatteryLevel();

  if(bat_level == last_bat_level) return;

  last_bat_level = bat_level;

  if ( last_bat_level == 0xFF )  {
      fsrv_DS_SetBatStatus(BAT_ERROR);  // no battery, battery dead
  } else {

      if ( TRUE == fsrv_DS_IsBatteryCharging() ) {
          fsrv_DS_SetBatStatus(BAT_CHARGING);
          if ( last_bat_level >= AEVS_BAT_LOW_10_BOTTOM_BORDER) {
              if ( ( auph_GetState() == AU_MENU_STATE) 
                && ( TRUE == fmnu_IsMenuActive(AUIM_MNU_INDEX_CHARGE_BATTERY_MENU) ) ) {
                   aukh_Post_UI_Event(AU_MENU_CHARGE_BATTERY_CANCELED);
              }
          }

      } else {
             if ( last_bat_level >= AEVS_BAT_NORMAL_BOTTOM_BORDER) {
              fsrv_DS_SetBatStatus(BAT_NORMAL);
        } else if ( (last_bat_level >= AEVS_BAT_LOW_50_BOTTOM_BORDER) && (last_bat_level <= AEVS_BAT_LOW_50_UPPER_BORDER)) {
              fsrv_DS_SetBatStatus(BAT_LOW_50);
        } else if ( (last_bat_level >= AEVS_BAT_LOW_30_BOTTOM_BORDER) && (last_bat_level <= AEVS_BAT_LOW_30_UPPER_BORDER)) {
              fsrv_DS_SetBatStatus(BAT_LOW_30);
        } else if ( (last_bat_level >= AEVS_BAT_LOW_10_BOTTOM_BORDER) && (last_bat_level <= AEVS_BAT_LOW_10_UPPER_BORDER)) {
              fsrv_DS_SetBatStatus(BAT_LOW_10);
        } else if (  last_bat_level < AEVS_BAT_CRITIC_BOTTOM_BORDER) {
              fsrv_DS_SetBatStatus(BAT_CRITICAL);
              if ( ( auph_GetState() == AU_IDLE_STATE ) 
                && ( FALSE == fmnu_IsMenuActive(AUIM_MNU_INDEX_CHARGE_BATTERY_MENU)) ) {
                  aukh_Post_UI_Event(AU_MENU_CHARGE_BATTERY_ENTER);
              }
        }
     }
  }

}

/*=======================================================================*/
/* F U N C T I O N S                                                     */
/*=======================================================================*/
void aevs_TurnOn(void)
{
  aevs_update_timer_expired = FALSE;

  aevs_active_task = AEVS_LOADING_ACTIVE_TASK;

  sl_sleeptimer_restart_periodic_timer_ms(&aevs_update_timer,
                                          AEVS_TIMER_MEDIUM,
                                          aevs_update_timer_callback,
                                          NULL,
                                          0,
                                          0);
}

/*=======================================================================*/
/*                                                                       */
/*=======================================================================*/
void aevs_TurnOff(void)
{
  sl_sleeptimer_stop_timer(&aevs_update_timer);

  aevs_update_timer_expired = FALSE;
  aevs_active_task = AEVS_NO_ACTIVE_TASK;
}

/*=======================================================================*/
/*                                                                       */
/*=======================================================================*/
void aevs_Init(void)
{

  aevs_update_timer_expired = FALSE;
  aevs_active_task = AEVS_NO_ACTIVE_TASK;

  sl_sleeptimer_restart_periodic_timer_ms(&aevs_update_timer,
                                          AEVS_TIMER_SMALL,
                                          aevs_update_timer_callback,
                                          NULL,
                                          0,
                                          0);

}

/*=======================================================================*/
/*                                                                       */
/*=======================================================================*/
void aevs_Update(void)
{

  if (fpmt_GetPowerState() != FPMT_POWER_ON) {
     return;
  }

  aevs_SyncSessionStatusUpdate();

 if (aevs_update_timer_expired) {
     aevs_update_timer_expired = FALSE;

    aevs_BatteryStatusUpdate();

    app_log("UI TASK = %d\r\n", aevs_active_task);

    switch (aevs_active_task) {
      case AEVS_LOADING_ACTIVE_TASK:
        aevs_active_task = aevs_StartupLogoIndicatorHandle();
      break;

      case AEVS_BATTERY_ACTIVE_TASK:
        aevs_active_task = aevs_ChargeBatteryRequiredIndicatorHandle();
      break;

      case AEVS_PERF_CHECK_ACTIVE_TASK:
        aevs_active_task = aevs_PerformanceRequiredIndicatorHandle();
      break;

      case AEVS_NO_ACTIVE_TASK:  //Fall through
      default:
        if ( auph_GetState() == AU_IDLE_STATE) {
            aevs_active_task = AEVS_BATTERY_ACTIVE_TASK;
        }
        break;
    }

    if ( aevs_active_task > AEVS_PERF_CHECK_ACTIVE_TASK ) {
        app_assert_status(SL_STATUS_FAIL); // Loop forever for debugging
    }

  }
}

/*=======================================================================*/
/*                                                                       */
/*=======================================================================*/
static void aevs_update_timer_callback(sl_sleeptimer_timer_handle_t *timer, void *data)
{
  (void)timer;
  (void)data;

  aevs_update_timer_expired = TRUE;
}

/*=======================================================================*/
/*                                                                       */
/*=======================================================================*/
void aevs_ForceChargeMenuCheck(void) {
  if (fsrv_DS_GetBatStatus() == BAT_CRITICAL && !fsrv_DS_IsBatteryCharging()) {
    if (!fmnu_IsMenuActive(AUIM_MNU_INDEX_CHARGE_BATTERY_MENU)) {
      aukh_Post_UI_Event(AU_MENU_CHARGE_BATTERY_ENTER);
    }
  }
}
