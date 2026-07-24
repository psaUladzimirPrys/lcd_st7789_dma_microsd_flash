#ifndef __ALS_CALCULATIONS_H__
#define __ALS_CALCULATIONS_H__

#include "app_log.h"
#include "general_defs.h"
#include "params.h"
#include "historical_records.h"
#include "fsrv.h"
#include "find_api.h"

// Структура для передачи результатов в ValidateMeasurement()
typedef struct {
    uint8_t peak_count;
    uint16_t max_val;       // Амплитудный пик (максимум)
    uint16_t max_idx;       // Индекс пика во времени
    uint16_t min_val;       // Точка отскока (минимум)
    uint16_t min_idx;       // Индекс минимума
    uint16_t amplitude;     // Чистая амплитуда укола (max - min)
    uint32_t impact_time;   // Точное время нарастания импульса
    uint32_t impact_area;   // ??? интеграл площади под кривой
    bool is_valid;
} osteo_peaks_t;


typedef enum {
    INDENT_VALID,
    ERR_TOO_LOW_SIGNAL,
    ERR_TOO_HIGH_SIGNAL,
    ERR_TOO_LOW_IMPACT_TIME,
    ERR_TOO_HIGH_IMPACT_TIME,
    ERR_TOO_LOW_AMPLITUDE,
    ERR_TOO_HIGH_AMPLITUDE,
    ERR_PEAK_FIND_FAILED,
    ERR_MULTIPLE_PEAKS      // <-- ДОБАВЛЯЕМ: ошибка двойного удара / дребезга
} indent_status_t;

typedef struct {
    uint16_t amplitudes[ABSOLUTE_MAX_OF_IDENTS]; // Массив сырых амплитуд (как в оригинале)
    uint16_t impact_time[ABSOLUTE_MAX_OF_IDENTS]; // время воздействия
    float    bms_current[ABSOLUTE_MAX_OF_IDENTS];
    float    bms_normalized[ABSOLUTE_MAX_OF_IDENTS]; // Заполняется в конце для BMSi
    bool     is_valid[ABSOLUTE_MAX_OF_IDENTS];   // Карта валидности (для фильтра выбросов)

    uint8_t idx;
    uint8_t valid_counter;                      //  счетчик валидных измерений
    uint8_t invalid_counter;                    // счетчик инвалидных измерений
    uint8_t remain_counter;

    uint8_t target_valid;     // Целевое количество (MIN_PATIENT_INDENTS или MIN_REF_INDENTS)

    uint8_t max_ident_couner; // максимальное количество измерений в пределах сессии
    bool    session_failed;   // сессия завалена т.к. не смогли набрать валидных измерений

    float mean;               // Среднее (для валидных)
    float stddev;             // Стандартное отклонение
    float median;             // Медиана
} measurement_session_t;

// Итоговые результаты всей процедуры
typedef struct {
    float harmonic_mean;      // Гармоническое среднее референса
    float final_bmsi;         // Итоговый балл (Bone Score)
    bool is_unstable;         // Флаг "Measurements are Unstable" (референс <0.9 или >1.1)
    bool is_pass;             // Попал ли BMSi в заданные рамки
    bool is_aproximate;
} final_bmsi_result_t;


//void process_single_indent(uint16_t *in_ptr, uint16_t *start_index, measurement_session_t *session, measurement_sub_mode_t sub_mode);
void process_single_indent(uint16_t *in_ptr, uint16_t *start_index, measurement_session_t *session, working_mode_t mode, working_sub_mode_t sub_mode);
uint16_t create_one_measure_packet(uint8_t *ptr, const measurement_session_t *session_data, working_mode_t mode, working_sub_mode_t sub_mode);
void filter_outliers(measurement_session_t *session);
//void calculate_final_bmsi(void);
void calculate_final_bmsi(working_mode_t mode);
void reset_measurement_session(measurement_session_t *session, uint8_t target_count);
void reset_final_bmsi_result(final_bmsi_result_t *result);
//uint16_t create_intermediate_bms_packet(uint8_t *ptr, const measurement_session_t *session_data, measurement_sub_mode_t sub_mode);
void send_intermediate_bms_packet (const measurement_session_t *session_data,  working_mode_t mode, working_sub_mode_t sub_mode);
void send_final_bmsi_packet (const final_bmsi_result_t *final_result, working_mode_t mode, working_sub_mode_t sub_mode);
void send_final_bmsi_packet_failed (const final_bmsi_result_t *final_result, working_mode_t mode, working_sub_mode_t sub_mode);
void send_pic_or_ric_fail_session_packet (const measurement_session_t *session_data, working_mode_t mode, working_sub_mode_t sub_mode);

Bool fsrv_DS_getPerformanceFinalResult (void);

uint8_t get_measurement_ident_counter(void);
uint8_t get_reference_ident_counter(void);

#endif
