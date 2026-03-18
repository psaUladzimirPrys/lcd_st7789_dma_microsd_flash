#ifndef __ALS_DEVICE_CONTROL__
#define __ALS_DEVICE_CONTROL__

#include "em_cmu.h"
#include "sl_core.h"

#include "app.h"
#include "app_log.h"
#include "device_control.h"

#include "sl_event_handler.h"

#include "sl_gpio.h"
#include "em_gpio.h"
#include "sl_sleeptimer.h"




#include "sl_iostream.h"
#include "sl_iostream_handles.h"

#include "em_device.h"
#include "em_timer.h"
#include "em_ldma.h"

#include "button.h"

#include "fsrv.h"
#include "auph.h"

device_parameters_t device_params;
device_state_t device_state_curr = DEVICE_STARTUP;

#define HOLD_TIME_SEC      3

#define TIMEOUT_AFTER_POWER_ON_MS 2000
 ;

uint32_t stack_used = 0;

bool hold_complete = false;


//----------------- for display subsystem --------------------
st_battery_t st_battery = BAT_NORMAL;
st_ble_connect_t st_ble_connect = BLE_DISCONNECTED;
st_sync_t st_sync = SYNC_IDLE;
st_measure_t st_measure = MEAS_IDLE;
measure_mode_t measure_mode = MODE_NONE;


//------------------------------------------------------------

void app_init_early(void)
{

}


void prepare_to_init_device (void)
{

  device_state_curr = DEVICE_STARTUP;

}

void set_device_state(device_state_t state)
{
  if (state != device_state_curr) {
    device_state_curr = state;
  }
}


device_state_t get_device_state(void)
{
   return device_state_curr;
}

static uint16_t crc16_ccitt(const uint8_t *data, uint16_t len)
{
    uint16_t crc = 0xFFFF;

    while (len--)
    {
        crc ^= (uint16_t)(*data++) << 8;
        for (uint8_t i = 0; i < 8; i++)
        {
            if (crc & 0x8000)
                crc = (crc << 1) ^ 0x1021;
            else
                crc <<= 1;
        }
    }
    return crc;
}

bool device_params_crc_validate(const device_parameters_t *p)
{
    if (!p) return false;

    uint16_t calc_crc = crc16_ccitt((const uint8_t *)p,sizeof(device_parameters_t) - sizeof(p->crc16));

    return (calc_crc == p->crc16);
}

void device_params_init(device_parameters_t *p)
{
    if (!p) return;

    p->scree_off_timeout          = 30;
    p->power_saving_mode_timeout  = 60;
    p->calibration_constant       = 1.21f;
    p->brightness_level           = 50;
    p->battery_threshold_warning  = 35;
    p->battery_threshold_critical = 20;
    p->crc16                      = crc16_ccitt((uint8_t *)p, sizeof(device_parameters_t) - sizeof(p->crc16));;
}

void reset_device_params(void)
{
    device_params_init(&device_params);
}

void device_params_update_crc(device_parameters_t *p)
{
  if (!p) return;
  p->crc16 = crc16_ccitt((uint8_t *)p, sizeof(device_parameters_t) - sizeof(p->crc16));// не включаем поле crc16
}


void device_working_loop(void)
{



  switch(device_state_curr)
  {
    case DEVICE_STARTUP:
      device_state_curr = DEVICE_LOADING;
      reset_device_params();
    break;

    case DEVICE_LOADING:
    break;

    case DEVICE_IDLE:

    break;

    case DEVICE_PATIENT_MEASURE:

    break;

    case DEVICE_PATIENT_MEASURE_SIMULATE:

    break;

    case DEVICE_PERFORMANCE_CHECK:

    break;

    case DEVICE_PERFORMANCE_CHECK_SIMULATE:

    break;

    case DEVICE_CONFIGURATION:

    break;

    case DEVICE_ERROR:

      break;

    case DEVICE_STANDBY:
      break;

    default:
      app_log("Device State Machine: Unknown state\r\n");
    break;
  }


}


float get_calibration_constatnt_fl(void)
{
  return device_params.calibration_constant;
}



#endif
