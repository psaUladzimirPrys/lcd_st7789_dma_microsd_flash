#ifndef __ALS_ADC_H__
#define __ALS_ADC_H__

#include <math.h>
#include "em_cmu.h"
#include "em_timer.h"
#include "em_prs.h"
#include "em_iadc.h"

#include "general_defs.h"
#include "app_log.h"
#include "params.h"
#include "ble_communication.h"
#include "tip_id_validate.h"


#define PRS_CH_ADC_TRIGGER   0
#define EXTERNAL_VREF_VALUE_MV    3000
#define ABSOLUTE_MAX_OF_IDENTS             100

#define NIDLE_CONNECTION_CHECK_SAMPLES  256
#define NIDLE_CONNECTION_ADC_LEVEL      1500

#define NIDLE_CONNECTION_CHECK_TIME_MS  3000

#define SAFE_TIMER_MS                   200

// Перечисление для выбора источника опорного напряжения
typedef enum {
  ADC_VREF_INTERNAL_1P21V, // Внутренний стабильный ИОН 1.21 В (Наиболее точный)
  ADC_VREF_INTERNAL_3P0V,  // Внутреннее напряжение 3.0 В (На базе аналогового питания)
  ADC_VREF_EXTERNAL_PA00   // Внешний опорный сигнал, поданный на пин PA00
} adc_vref_source_t;

// Перечисление для выбора текущего канала (в adc.h)
typedef enum {
  ADC_CHANNEL_NONE,
  ADC_CHANNEL_SIGNAL, // PA08
  ADC_CHANNEL_BATTERY // PA07
} adc_channel_t;


// Состояния конечного автомата АЦП
typedef enum {
  ADC_STATE_IDLE,          // Ничего не делаем
  ADC_STATE_PRE_TRIGGER,   // Пишем в кольцевой буфер, ищем лавинообразный рост (касание)
  ADC_STATE_FIND_PEAK,     // Рост начался, ищем самую максимальную точку (пик удара)
  ADC_STATE_POST_PEAK,     // Пик найден, дописываем фиксированный хвост затухания
  ADC_STATE_DONE           // Буфер заполнен, ждем вычитки данных
} adc_capture_state_t;

typedef enum {
  SUB_MODE_NONE,
  SUB_MODE_PATIENT,
  SUB_MODE_PERFORMANCE
} measurement_sub_mode_t;

/*typedef enum {
  MEASUREMENT_STATE_IDLE, // ничего не делаем

  MEASUREMENT_STATE_START_MEASURE, // запускаем процесс измерения
  MEASUREMENT_STATE_WAIT_MEASURE_DATA,  // ждем накопления данных и выстраивания массива по порядку
  MEASUREMENT_STATE_DO_MATH_FOR_MEASURE, // производим математические расчеты
  MEASUREMENT_STATE_DO_BMS_FOR_MEASURE, // считаем BMS по данным пациента

  MEASUREMENT_STATE_IDLE_BEFORE_REFERENCE,

  MEASUREMENT_STATE_START_REFERENCE,
  MEASUREMENT_STATE_WAIT_REFERENCE_DATA,
  MEASUREMENT_STATE_DO_MATH_FOR_REFERENCE,
  MEASUREMENT_STATE_DO_BMS_FOR_REFERENCE,
  MEASUREMENT_STATE_DO_FINAL_BMSI,

} measurement_state_t;*/

typedef enum {
  MEASUREMENT_STATE_IDLE_2,

  MEASUREMENT_STATE_START_CAPTURE,    // Запуск АЦП
  MEASUREMENT_STATE_WAIT_ADC_DONE,    // Ожидание окончания буфера
  MEASUREMENT_STATE_PROCESS_INDENT,   // Расчет единичного укола
  MEASUREMENT_STATE_WAIT_INDENT_SENT_CONFIRMATION,
  MEASUREMENT_STATE_CHECK_LIMITS,     // Проверка: набрали ли нужный минимум оттисков?

  MEASUREMENT_STATE_WAIT_TO_SEND_STRM,
  MEASUREMENT_STATE_WAIT_BUTTON_REF,  // Ожидание кнопки перед сессией фантома
  MEASUREMENT_STATE_CALCULATE_BMSI,    // Финальная калькуляция и отправка

  MEASUREMENT_STATE_WAIT_SEND_FINAL_BMSI,

  MEASUREMENT_STATE_FAILED_WAITING,

  MEASUREMENT_STATE_SEND_FINAL_BMSI

} measurement_state_2_t;

//=========================================================
// Режимы работы
typedef enum {
    WORKING_MODE_NONE = 0,
    WORKING_MODE_PATIENT,
    WORKING_MODE_PERFORMANCE
} working_mode_t;

// Подрежимы работы для каждого из режимов
typedef enum {
    WORKING_SUB_MODE_NONE = 0,
    WORKING_SUB_MODE_PATIENT_MEASUREMENT,
    WORKING_SUB_MODE_PATIENT_REFERENCE,
    WORKING_SUB_MODE_PERFORMANCE_MEASUREMENT,
    WORKING_SUB_MODE_PERFORMANCE_REFERENCE
} working_sub_mode_t;

// Подробная последовательная машина состояний
typedef enum {
    MACHNE_STATE_IDLE = 0,

    MACHNE_STATE_TIP_ID_START,
    MACHNE_STATE_TIP_ID_VALIDATE,
    MACHNE_STATE_TIP_ID_VALIDATING_DISPLAY_RESULT,
    MACHNE_STATE_TIP_ID_VALIDATING_END,

   // === РЕЖИМ PERFORMANCE ===
    MACHINE_STATE_PREPARE_FOR_PERFORMANCE,
    // Подрежим: performance_measurement
    MACHNE_STATE_PERFORMANCE_MEASUREMENT_START,
    MACHNE_STATE_PERFORMANCE_MEASUREMENT_WAIT_ADC,
    MACHNE_STATE_PERFORMANCE_MEASUREMENT_PROCESS,
    MACHNE_STATE_PERFORMANCE_MEASUREMENT_PROCESS_FAILED,
    MACHNE_STATE_PERFORMANCE_MEASUREMENT_PROCESS_FAILED_WAIT_TIMEOUT,
    MACHNE_STATE_PERFORMANCE_MEASUREMENT_PROCESS_FAILED_PRE_STATE_IDLE,
    MACHNE_STATE_PERFORMANCE_WAIT_ONE_MEASURE_PACKET_SENT,
    MACHNE_STATE_PERFORMANCE_MEASUREMENT_CHECK_COUNT,
    MACHNE_STATE_PERFORMANCE_MEASUREMENT_SAFTY_BARRIER,
    MACHNE_STATE_PERFORMANCE_MEASUREMENT_SEND_BMS_PACKET,
    MACHNE_STATE_PERFORMANCE_MEASUREMENT_WAIT_BMS_PACKET_SENT,
    MACHNE_STATE_WAIT_PERFORMANCE_MEASUREMENT_TO_SEND_STRM,
    MACHNE_STATE_PERFORMANCE_MEASUREMENT_WAIT_BUTTON_PRESSED,
    MACHNE_STATE_PERFORMANCE_MEASUREMENT_DONE,

    // Подрежим: performance_reference
    MACHINE_STATE_PREPARE_FOR_PERFORMANCE_REFERENCE,
    MACHNE_STATE_PERFORMANCE_REFERENCE_START,
    MACHNE_STATE_PERFORMANCE_REFERENCE_WAIT_ADC,
    MACHNE_STATE_PERFORMANCE_REFERENCE_PROCESS,
    MACHNE_STATE_PERFORMANCE_REFERENCE_PROCESS_FAILED,
//    MACHNE_STATE_PERFORMANCE_REFERENCE_PROCESS_FAILED_PRE_STATE_IDLE,
    MACHNE_STATE_PERFORMANCE_WAIT_ONE_REFERENCE_PACKET_SENT,
    MACHNE_STATE_PERFORMANCE_REFERENCE_CHECK_COUNT,
    MACHNE_STATE_PERFORMANCE_REFERENCE_SAFTY_BARRIER,
    MACHNE_STATE_PERFORMANCE_REFERENCE_SEND_BMS_PACKET,
    MACHNE_STATE_PERFORMANCE_REFERENCE_WAIT_BMS_PACKET_SENT,
    MACHNE_STATE_WAIT_PERFORMANCE_REFERENCE_TO_SEND_STRM,

    MACHNE_STATE_PERFORMANCE_CALCULATE_BMSI,
    MACHNE_STATE_PERFORMANCE_WAIT_SEND_FINAL_BMSI,
    MACHNE_STATE_PERFORMANCE_SEND_FINAL_BMSI,

    MACHNE_STATE_PERFORMANCE_REFERENCE_WAIT_BUTTON_PRESSED,
    MACHNE_STATE_PERFORMANCE_REFERENCE_DONE,

    // === РЕЖИМ PATIENT ===
    MACHINE_STATE_PREPARE_FOR_PATIENT,
    // Подрежим: patient_measurement
    MACHNE_STATE_PATIENT_MEASUREMENT_START,
        MACHNE_STATE_PATIENT_MEASUREMENT_WAIT_ADC,
        MACHNE_STATE_PATIENT_MEASUREMENT_PROCESS,
        MACHNE_STATE_PATIENT_MEASUREMENT_PROCESS_FAILED,
        MACHNE_STATE_PATIENT_MEASUREMENT_PROCESS_FAILED_WAIT_TIMEOUT,
        MACHNE_STATE_PATIENT_MEASUREMENT_PROCESS_FAILED_PRE_STATE_IDLE,
        MACHNE_STATE_PATIENT_WAIT_ONE_MEASURE_PACKET_SENT,
        MACHNE_STATE_PATIENT_MEASUREMENT_CHECK_COUNT,
        MACHNE_STATE_PATIENT_MEASUREMENT_SAFTY_BARRIER,
        MACHNE_STATE_PATIENT_MEASUREMENT_SEND_BMS_PACKET,
        MACHNE_STATE_PATIENT_MEASUREMENT_WAIT_BMS_PACKET_SENT,
        MACHNE_STATE_WAIT_PATIENT_MEASUREMENT_TO_SEND_STRM,
        MACHNE_STATE_PATIENT_MEASUREMENT_WAIT_BUTTON_PRESSED,
        MACHNE_STATE_PATIENT_MEASUREMENT_DONE,

    // Подрежим: patient_reference
        // Подрежим: performance_reference
        MACHINE_STATE_PREPARE_FOR_PATIENT_REFERENCE,
        MACHNE_STATE_PATIENT_REFERENCE_START,
        MACHNE_STATE_PATIENT_REFERENCE_WAIT_ADC,
        MACHNE_STATE_PATIENT_REFERENCE_PROCESS,
//        MACHNE_STATE_PATIENT_REFERENCE_PROCESS_FAILED_PRE_STATE_IDLE,
        MACHNE_STATE_PATIENT_REFERENCE_PROCESS_FAILED,
        MACHNE_STATE_PATIENT_WAIT_ONE_REFERENCE_PACKET_SENT,
        MACHNE_STATE_PATIENT_REFERENCE_CHECK_COUNT,
        MACHNE_STATE_PATIENT_REFERENCE_SAFTY_BARRIER,
        MACHNE_STATE_PATIENT_REFERENCE_SEND_BMS_PACKET,
        MACHNE_STATE_PATIENT_REFERENCE_WAIT_BMS_PACKET_SENT,
        MACHNE_STATE_WAIT_PATIENT_REFERENCE_TO_SEND_STRM,

        MACHNE_STATE_PATIENT_CALCULATE_BMSI,
        MACHNE_STATE_PATIENT_WAIT_SEND_FINAL_BMSI,
        MACHNE_STATE_PATIENT_SEND_FINAL_BMSI,

        MACHNE_STATE_PATIENT_REFERENCE_WAIT_BUTTON_PRESSED,
        MACHNE_STATE_PATIENT_REFERENCE_DONE,


} measurement_machine_state_t;

//=========================================================


void start_battery_measurement(adc_vref_source_t vref_source);

void start_impact_capture(uint32_t sampling_period_us, adc_vref_source_t vref_source);
void debug_hardware_trinity(void);


void start_patient_session(void);

void stop_patient_measurement_2(void);



void measurement_process(void);
void measurement_process_loop(void); //==============================
void start_performance_session(void); //=============================
void pause_current_measurement(void); //=============================
void resume_current_measurement(void); //================================
void stop_measurement_session(void); //======================

void switch_machine_to_tip_id_validate(void);


void is_stilus_connected_process (void);

bool patient_session_return_result (void);
bool referense_session_return_result (void);
bool measurement_session_return_result (void);



//=========================================================================
void init_adc_scan(void);
//========================================================================
void read_idle_samples(void);

/*=======================================================================*/
/*        G L O B A L   D A T A   D E C L A R A T I O N S                */
/*=======================================================================*/
void fsrv_switch_to_reference_state(void);
void fsrv_switch_to_reference_submode(void);
bool fsrv_is_performance_mode_state(void);
void fsrv_calculate_aproximate_bmsi(void);

#define fsrv_start_performance           start_performance_session
#define fsrv_pause_performance           pause_current_measurement
#define fsrv_resume_performance          resume_current_measurement
#define fsrv_stop_performance            stop_measurement_session
#define fsrv_is_session_performance_bad  measurement_session_return_result

#define fsrv_pause_reference             pause_current_measurement
#define fsrv_resume_reference            resume_current_measurement
#define fsrv_stop_reference              stop_measurement_session           /* @ToDo  UP  ???????*/
#define fsrv_is_session_reference_bad    referense_session_return_result

#define fsrv_start_patient               start_patient_session
#define fsrv_pause_patient               pause_current_measurement
#define fsrv_resume_patient              resume_current_measurement
#define fsrv_stop_patient                stop_measurement_session
#define fsrv_is_session_patient_bad      measurement_session_return_result

#define fsrv_AllMeasurementsTurnOff      stop_measurement_session


#endif  /*  __ALS_ADC_H__  */
