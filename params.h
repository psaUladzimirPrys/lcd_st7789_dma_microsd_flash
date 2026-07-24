#ifndef __ALS_PARAMS_H__
#define __ALS_PARAMS_H__


/*=======================================================================*/
/*        I N C L U D E S                                                */
/*=======================================================================*/
#include "helpers.h"
#include "protocol.h"
#include "app_log.h"


/*=========================================================================*/
/*   G L O B A L   D E F I N I T I O N S                                   */
/*=========================================================================*/
#define SCREEN_OFF_TIMEOUT_LEN            4 // Variable format XXXX
#define POWER_SAVING_MODE_TIMEOUT_LEN     4 // Variable format XXXX
#define CALIBRATION_CONSTAN_LEN           3 // Variable format XXX
#define BRIGHTNESS_LEVEL_LEN              3 // Variable format XXX
#define BATTERY_THRESHOLD_WARNING_LEN     2 // Variable format XX
#define BATTERY_THRESHOLD_CRITICAL_LEN    2 // Variable format XX

#define SCREEN_OFF_TIMEOUT_MIN            30
#define SCREEN_OFF_TIMEOUT_MAX            1200
#define POWER_SAVING_MODE_TIMEOUT_MIN     60
#define POWER_SAVING_MODE_TIMEOUT_MAX     1800
#define CALIBRATION_CONSTANT_MIN          1
#define CALIBRATION_CONSTANT_MAX          999
#define BRIGHTNESS_LEVEL_MIN              1
#define BRIGHTNESS_LEVEL_MAX              100
#define BATTERY_THRESHOLD_WARNING_MIN     10
#define BATTERY_THRESHOLD_WARNING_MAX     99
#define BATTERY_THRESHOLD_CRITICAL_MIN    10
#define BATTERY_THRESHOLD_CRITICAL_MAX    99


#define DEFAUL_MAX_NUM_OF_PERFORMANCE_MEASURE_IDENTS 10
#define DEFAULT_TOTAL_MEASUREMENTS        10  // максимальное количество измерений
#define DEFAULT_VALID_MEASUREMENTS        5  // количество валидных измерений которые надо набрать за DEFAULT_TOTAL_MEASUREMENTS

#define DEFAULT_ADC_SAMPLING_TIME         5 // 10us


//#pragma pack(push, 1)
typedef struct //__attribute__((packed))
{
    struct
    {
      uint32_t serial_number;
      uint32_t reference_number;
      uint8_t  has_raw_data;

      bool     debug_output_on;
      struct
      {
        uint16_t adc_taking_period; // ADC value sampling period for the timer in microseconds
        uint8_t  adc_vref_source;
        uint8_t  max_num_of_patient_measurements;
        uint8_t  max_num_of_patient_references;
        uint8_t  max_num_of_performance_measurements;
        uint8_t  max_num_of_performance_references;
        uint8_t  num_of_valid_patient_measurements;
        uint8_t  num_of_valid_patient_references;
        uint8_t  num_of_valid_performance_measurements;
        uint8_t  num_of_valid_performance_references;
        uint16_t signal_min;
        uint16_t signal_max;
        uint16_t impact_amp_min;
        uint16_t impact_amp_max;
        uint16_t impact_time_min;
        uint16_t impact_time_max;
        uint16_t bmsi_min;
        uint16_t bmsi_max;
      }measure_params;

    }advanced_params;

    struct
    {
      uint16_t scree_off_timeout;
      uint16_t power_saving_mode_timeout;
      uint8_t  brightness_level;
      uint8_t  battery_threshold_warning;
      uint8_t  battery_threshold_critical;
      float    calibration_constant;
    }general_params;
} device_params_t;
//#pragma pack(pop)

/*===========================================================================*/
/*    G L O B A L   F U N C T I O N     P R O T O T Y P E S                  */
/*===========================================================================*/
void set_default_general_params(void);
void get_general_params(const uint8_t *in_ptr);
void send_general_params_to_ovs_station(uint8_t *ptr);
void send_device_info_to_ovs_station(uint8_t *ptr);

void set_default_advanced_params(void);
void fsrv_DS_SetCfgConstants(void);

#endif
