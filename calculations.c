#include "calculations.h"
#include "file_storage.h"

extern device_params_t device_params;

extern uint8_t tx_data_buffer[];

extern one_measure_data_packet_t one_measure_data_packet;

// Экземпляры сессий
//measurement_session_t patient_session = {0};
measurement_session_t reference_session = {0};
final_bmsi_result_t final_results = {0};

extern measurement_session_t measurement_session;

/**
  * @brief функция инверсии участка массива.
  */
static void reverse_sub_array(uint16_t *arr, int start, int end)
{
   while (start < end)
   {
     uint16_t temp  = arr[start];
     arr[start]     = arr[end];
     arr[end]       = temp;
     start++;
     end--;
   }
}

/**
  * @brief Выпрямление кольцевого буфера методом тройной перестановки «на месте».
  */
void adc_inplace_triple_reversal(uint16_t *in_data_ptr, uint16_t *impact_start_index, uint16_t total_samples)
{
   if(*impact_start_index == 0) return; // Массив уже линеаризован

   uint16_t start_idx = *impact_start_index;

   reverse_sub_array(in_data_ptr,         0, start_idx - 1    );
   reverse_sub_array(in_data_ptr, start_idx, total_samples - 1);
   reverse_sub_array(in_data_ptr,         0, total_samples - 1);

   *impact_start_index = 0;
}

/**
  * @brief Вывод буфера АЦП в лог.
  */
void adc_log_unified_buffer(uint16_t *in_data_ptr, uint16_t samples_before_reak, uint16_t total_samples)
{
  if (in_data_ptr == NULL) return;
   app_log("\r\n=== ADC Buffer Dump Normalized (256 samples) ===\r\n");
   app_log("Samples 0..%d = Pre-trigger, %d = PEAK, %d..255 = Post-impact\r\n\r\n", samples_before_reak - 1, samples_before_reak, samples_before_reak + 1);

   for (int i = 0; i < total_samples; i++)
   {
       uint16_t val = in_data_ptr[i];

       if (i == 0)
       {
          //app_log("[%03d] -> %04d (START of history)\r\n", i, val);
          app_log("%03d;%04d\r\n", i, val); //for csv
       }
       else if (i == samples_before_reak)
       {
          //app_log("==================================================\r\n");
          //app_log("[%03d] -> %04d <--- !!! PEAK (Max_V) !!!\r\n", i, val);
          //app_log("==================================================\r\n");
          app_log("%03d;%04d\r\n", i, val);
       }
       else if (i == total_samples - 1)
       {
          //app_log("[%03d] -> %04d (END of history)\r\n", i, val);
          app_log("%03d;%04d\r\n", i, val);
       }
       else
       {
          //app_log("[%03d] -> %04d\r\n", i, val);
          app_log("%03d;%04d\r\n", i, val);
       }
    }
    app_log("================================================\r\n");
}


#define START_IMPACT_INDEX          56  // Индекс, где ожидается начало удара/пик после выпрямления кругового буфера
// Границы временного "окна" для поиска истинного максимума.
// Ищем пик чуть-чуть до 56-го сэмпла и чуть-чуть после, чтобы учесть погрешность триггера.
#define SEARCH_PEAK_WINDOW_START    51
#define SEARCH_PEAK_WINDOW_END      66
#define BASELINE_WINDOW_SIZE        40
#define TRAILING_DROP_HYSTERESIS    15


osteo_peaks_t adc_find_peaks_final(const volatile uint16_t* buffer, uint16_t sampling_time, working_mode_t mode, working_sub_mode_t sub_mode)
{
  osteo_peaks_t peaks;

        // 1. Инициализация структуры дефолтными значениями
        peaks.peak_count  = 0;
        peaks.max_val     = 0;
        peaks.max_idx     = 0;
        peaks.min_val     = 4095;
        peaks.min_idx     = 0;
        peaks.amplitude   = 0;
        peaks.impact_time = 0;
        peaks.impact_area = 0;
        peaks.is_valid    = false;

        // Защита от передачи пустого указателя
        if (buffer == NULL) {
            app_log("MATH: Error - Buffer pointer is NULL\r\n");
            return peaks;
        }

        // 2. Поиск глобального максимума (основного пика) по всему буферу
        for (uint16_t i = 0; i < TOTAL_IMPACT_SAMPLES; i++) {
            if (buffer[i] > peaks.max_val) {
                peaks.max_val = buffer[i];
                peaks.max_idx = i;
            }
        }

        // Порог фильтрации шума
        if (peaks.max_val < 2000) {
            app_log("=== max_idx = %u max_val= %u === MATH: too low signal amplitude\r\n", peaks.max_idx, peaks.max_val);
            return peaks;
        }

        // Фиксируем первый найденный пик
        peaks.peak_count = 1;

        // 3. Поиск минимума строго ДО индекса первого пика
        peaks.min_val = peaks.max_val;
        for (uint16_t i = 0; i <= peaks.max_idx; i++) {
            if (buffer[i] < peaks.min_val) {
                peaks.min_val = buffer[i];
                peaks.min_idx = i;
            }
        }

        app_log("=== max_idx = %u max_val= %u, === min_idx = %u min_val= %u\r\n",
                peaks.max_idx, peaks.max_val, peaks.min_idx, peaks.min_val);

        // 4. Расчет чистой амплитуды удара
        if (peaks.max_val > peaks.min_val) {
            peaks.amplitude = peaks.max_val - peaks.min_val;
        } else {
            peaks.amplitude = 0;
        }

        // 5. Расчет физического времени нарастания импульса
        peaks.impact_time = (uint32_t)peaks.max_idx * sampling_time;

        // 6. Расчет условной площади под кривой импульса
        uint16_t baseline = buffer[0];
        uint32_t area_acc = 0;
        for (uint16_t i = 0; i < TOTAL_IMPACT_SAMPLES; i++) {
            if (buffer[i] > baseline) {
                area_acc += (buffer[i] - baseline);
            }
        }
        peaks.impact_area = area_acc;

        // 7. Проверка на наличие двойного удара (дребезг бойка) после максимума
        uint16_t secondary_max = 0;
        uint16_t secondary_idx = 0;
        for (uint16_t i = peaks.max_idx + 1; i < TOTAL_IMPACT_SAMPLES; i++) {
            if (buffer[i] > buffer[i - 1]) {
                if (buffer[i] > secondary_max) {
                    secondary_max = buffer[i];
                    secondary_idx = i;
                }
            }
        }

        // ИЗМЕНЕНИЕ: Анализируем вторичный пик, только если он находится до 56 индекса включительно
        if (secondary_max > (baseline + 250)) {
            if (secondary_idx <= 56) {
                peaks.peak_count = 2;
                app_log("MATH: Warning - Secondary peak detected within window! idx=%u, val=%u\r\n", secondary_idx, secondary_max);
            } else {
                // Пик за пределами 56-го сэмпла игнорируется, лог пишется только для отладки (при желании можно закомментировать)
                app_log("MATH: Info - Distant peak ignored (idx=%u > 56), keeping single peak.\r\n", secondary_idx);
            }
        }

        // 8. Финальный вердикт валидности измерения
        if (peaks.amplitude > 100 && peaks.peak_count == 1) {
            peaks.is_valid = true;
        } else {
            peaks.is_valid = false;
            if (peaks.peak_count > 1) {
                if((mode == WORKING_MODE_PERFORMANCE) && (sub_mode ==WORKING_SUB_MODE_PERFORMANCE_MEASUREMENT) ) {
                    app_log("MATH: Performance Measurement INVALID - Multiple Peaks. Amp=%u\r\n", peaks.amplitude);
                }

            } else {
                if((mode == WORKING_MODE_PERFORMANCE) && (sub_mode ==WORKING_SUB_MODE_PERFORMANCE_MEASUREMENT) ) {
                    app_log("MATH: Performance Measurement INVALID - Low Amplitude. Amp=%u\r\n", peaks.amplitude);
                }
            }
        }

        return peaks;
}

/**
 * @brief Функция валидации (проверяет, укладывается ли укол в физические рамки)
 */
indent_status_t validate_captured_indent_final(osteo_peaks_t peaks, device_params_t params)
{
    // Порог насыщения АЦП (например, для 12-битного АЦП это 4000+)
    if (peaks.min_val < params.advanced_params.measure_params.signal_min) {
        app_log("MATH: too low signal\r\n");
        return ERR_TOO_LOW_SIGNAL;
    }

    // Минимальная амплитуда, чтобы отсечь шорохи (в оригинале проверяется > 0, поставим разумные 100)
    if (peaks.max_val > params.advanced_params.measure_params.signal_max) {
        app_log("MATH: too high signal\r\n");
        return ERR_TOO_HIGH_SIGNAL;
    }

    if (peaks.impact_time < params.advanced_params.measure_params.impact_time_min) {
        app_log("MATH: too low impact time\r\n");
        return ERR_TOO_LOW_IMPACT_TIME;
    }
    if (peaks.impact_time > params.advanced_params.measure_params.impact_time_max) {
        app_log("MATH: too high impact time\r\n");
        return ERR_TOO_HIGH_IMPACT_TIME;
    }

    if (peaks.amplitude < params.advanced_params.measure_params.impact_amp_min) {
        app_log("MATH: too low signal amplitude\r\n");
        return ERR_TOO_LOW_AMPLITUDE;
    }
    if (peaks.amplitude > params.advanced_params.measure_params.impact_amp_max) {
        app_log("MATH: too high signal amplitude\r\n");
        return ERR_TOO_HIGH_AMPLITUDE;
    }

    if (peaks.peak_count != 1) {
        app_log("MATH: multiple peaks detected\r\n");
        return ERR_MULTIPLE_PEAKS;
    }
    return INDENT_VALID;
}


void reset_measurement_session(measurement_session_t *session, uint8_t target_count) {
    memset(session, 0, sizeof(measurement_session_t));
    session->target_valid = target_count;
    session->remain_counter = target_count;
}

void reset_final_bmsi_result(final_bmsi_result_t *result) {
    memset(result, 0, sizeof(final_bmsi_result_t));
}

// функция вычисления статистики
static void calc_stats(float* indents, bool* valid, uint8_t max_size, float* mean_out, float* stddev_out) {
   *mean_out = 0.0f;
   *stddev_out = 0.0f;
   uint8_t count = 0;
   float sum = 0.0f;

   for (uint8_t i = 0; i < max_size; i++) {
       if (valid[i]) {
           sum += indents[i];
           count++;
       }
   }
   if (count == 0) return;

   *mean_out = sum / count;
   float sum_sq_diff = 0.0f;
   for (uint8_t i = 0; i < max_size; i++) {
       if (valid[i]) {
           float diff = indents[i] - *mean_out;
           sum_sq_diff += diff * diff;
       }
   }
   *stddev_out = sqrtf(sum_sq_diff / count);
}

// Фильтрация выбросов (> 0.15 от медианы)
void filter_outliers(measurement_session_t *session) {
    if (session->valid_counter == 0) return;

    float valid_bms[ABSOLUTE_MAX_OF_IDENTS];
    uint8_t count = 0;

    // Собираем валидные данные
    for (uint8_t i = 0; i < session->idx; i++) {
        if (session->is_valid[i]) {
            valid_bms[count++] = session->bms_current[i];
        }
    }

    // Сортировка (пузырьком для небольших массивов достаточно)
    for (uint8_t i = 0; i < count - 1; i++) {
        for (uint8_t j = 0; j < count - i - 1; j++) {
            if (valid_bms[j] > valid_bms[j + 1]) {
                float tmp = valid_bms[j];
                valid_bms[j] = valid_bms[j + 1];
                valid_bms[j + 1] = tmp;
            }
        }
    }

    // Расчет медианы
    if (count % 2 == 0) {
        session->median = (valid_bms[count / 2 - 1] + valid_bms[count / 2]) / 2.0f;
    } else {
        session->median = valid_bms[count / 2];
    }

    app_log("MATH: Median calculated = %.2f\r\n", session->median);

    // Отбраковка
    bool plus_one_ident = FALSE;
    for (uint8_t i = 0; i < session->idx; i++) {
        if (session->is_valid[i]) {
            if (fabsf(session->bms_current[i] - session->median) > 0.15f) {
                session->is_valid[i] = false;
                session->valid_counter--;
                session->invalid_counter++;
                session->remain_counter++;
                plus_one_ident = TRUE;
                app_log("MATH: Indent %d rejected as outlier (BMS=%.2f, diff=%.2f)\r\n",
                        i, session->bms_current[i], fabsf(session->bms_current[i] - session->median));
            }
        }
    }

    if (plus_one_ident == TRUE) {
        find_RemoveAllNotificationIndicators();
        find_DisplayIndicator(FIND_ID_ONE_INDENT_REQUIRED);
        fsrv_DS_SetValidIndentations(session->remain_counter);
    }

}

// Расчет одного укола и занесение в сессию
void process_single_indent(uint16_t *in_ptr, uint16_t *start_index, measurement_session_t *session, working_mode_t mode, working_sub_mode_t sub_mode) {
  bool ident_notification = true;
    // Выпрямление буфера (предполагаем, функции уже есть у вас выше)
    adc_inplace_triple_reversal(in_ptr, start_index, TOTAL_IMPACT_SAMPLES);
    adc_log_unified_buffer(in_ptr, SAMPLES_BEFORE_PEAK,TOTAL_IMPACT_SAMPLES); /* @ToDo  ?????????? */
    osteo_peaks_t peaks = adc_find_peaks_final(in_ptr, TOTAL_IMPACT_SAMPLES, mode, sub_mode);

    session->amplitudes[session->idx] = peaks.amplitude;
    session->impact_time[session->idx] = peaks.impact_time * device_params.advanced_params.measure_params.adc_taking_period;

    indent_status_t status = validate_captured_indent_final(peaks, device_params);

    if (status == INDENT_VALID) {
        float cal_constant = (float)device_params.general_params.calibration_constant;
        if (cal_constant <= 0.0f) cal_constant = 1.0f;

        session->is_valid[session->idx] = true;
        session->bms_current[session->idx] = (150.0f * cal_constant) / (float)peaks.amplitude;
        session->valid_counter++;
        session->remain_counter--;
        session->max_ident_couner++;

        if((mode == WORKING_MODE_PERFORMANCE) && (sub_mode == WORKING_SUB_MODE_PERFORMANCE_MEASUREMENT)) {
           // проверка превышения лимита измерений
           if (session->max_ident_couner >= device_params.advanced_params.measure_params.max_num_of_performance_measurements) {
               session->session_failed = true;
               app_log("MEASUREMENT: Performance Measurement Session failed! Max allowed measurements exceeded (%d)\r\n", device_params.advanced_params.measure_params.max_num_of_performance_measurements);
               //aukh_Post_UI_Event(AU_PERFORMANCE_STATE_FINISHED);
               ident_notification = false;
               //return; // Прерываем дальнейшую обработку текущего укола
           }
           app_log("MATH: Performance Measure Indent [%u] VALID. Amp=%u, BMS=%.5f\r\n", session->idx, peaks.amplitude, session->bms_current[session->idx]);
        }

        else if((mode == WORKING_MODE_PERFORMANCE) && (sub_mode == WORKING_SUB_MODE_PERFORMANCE_REFERENCE)) {
            // проверка превышения лимита измерений
            if (session->max_ident_couner >= device_params.advanced_params.measure_params.max_num_of_performance_references) {
                session->session_failed = true;
                app_log("MEASUREMENT: Performance Reference Session failed! Max allowed measurements exceeded (%d)\r\n", device_params.advanced_params.measure_params.max_num_of_performance_references);
                // Здесь аварийно останавливаем сессию (сбрасываем состояние АЦП в IDLE)
                aukh_Post_UI_Event(AU_REFERENCE_PERFORM_MENU_REPEAT_START);
                ident_notification = false;
                //return; // Прерываем дальнейшую обработку текущего укола
            }
            app_log("MATH: Performance Reference Indent [%u] VALID. Amp=%u, BMS=%.5f\r\n", session->idx, peaks.amplitude, session->bms_current[session->idx]);
        }

        else if((mode == WORKING_MODE_PATIENT) && (sub_mode == WORKING_SUB_MODE_PATIENT_MEASUREMENT)) {
            // проверка превышения лимита измерений
            if (session->max_ident_couner >= device_params.advanced_params.measure_params.max_num_of_patient_measurements) {
                session->session_failed = true;
                app_log("MEASUREMENT: Patient Measurement Session failed! Max allowed measurements exceeded (%d)\r\n", device_params.advanced_params.measure_params.max_num_of_patient_measurements);
                // Здесь аварийно останавливаем сессию (сбрасываем состояние АЦП в IDLE)
                aukh_Post_UI_Event(AU_PATIENT_MENU_FINISHED);
                ident_notification = false;
                //return; // Прерываем дальнейшую обработку текущего укола
            }
            app_log("MATH: Patient Measure Indent [%u] VALID. Amp=%u, BMS=%.5f\r\n", session->idx, peaks.amplitude, session->bms_current[session->idx]);
        }

        else if((mode == WORKING_MODE_PATIENT) && (sub_mode == WORKING_SUB_MODE_PATIENT_REFERENCE)) {
            // проверка превышения лимита измерений
            if (session->max_ident_couner >= device_params.advanced_params.measure_params.max_num_of_patient_references) {
                session->session_failed = true;
                app_log("MEASUREMENT: Patient Reference Session failed! Max allowed measurements exceeded (%d)\r\n", device_params.advanced_params.measure_params.max_num_of_patient_references);
                // Здесь аварийно останавливаем сессию (сбрасываем состояние АЦП в IDLE)
                aukh_Post_UI_Event(AU_REFERENCE_PATIENT_MENU_REPEAT_START);  /* @ToDo Repeat Reference start in patient mode */
                ident_notification = false;
                //return; // Прерываем дальнейшую обработку текущего укола
            }
            app_log("MATH: Patient Reference Indent [%u] VALID. Amp=%u, BMS=%.5f\r\n", session->idx, peaks.amplitude, session->bms_current[session->idx]);
        }
     } else {
        session->is_valid[session->idx] = false;
        session->bms_current[session->idx] = 0.0f;
        session->invalid_counter++;
        session->max_ident_couner++;

        if((mode == WORKING_MODE_PERFORMANCE) && (sub_mode == WORKING_SUB_MODE_PERFORMANCE_MEASUREMENT)) {
           // проверка превышения лимита измерений
           if (session->max_ident_couner >= device_params.advanced_params.measure_params.max_num_of_performance_measurements) {
               session->session_failed = true;
               app_log("MEASUREMENT: Performance Measurement Session failed! Max allowed measurements exceeded (%d)\r\n", device_params.advanced_params.measure_params.max_num_of_performance_measurements);
               aukh_Post_UI_Event(AU_PERFORMANCE_MENU_FINISHED);
               ident_notification = false;
               //return; // Прерываем дальнейшую обработку текущего укола
           }
        }

        else if((mode == WORKING_MODE_PERFORMANCE) && (sub_mode == WORKING_SUB_MODE_PERFORMANCE_REFERENCE)) {
            // проверка превышения лимита измерений
            if (session->max_ident_couner >= device_params.advanced_params.measure_params.max_num_of_performance_references) {
                session->session_failed = true;
                app_log("MEASUREMENT: Performance Reference Session failed! Max allowed measurements exceeded (%d)\r\n", device_params.advanced_params.measure_params.max_num_of_performance_references);
                // Здесь аварийно останавливаем сессию (сбрасываем состояние АЦП в IDLE)
                //aukh_Post_UI_Event(AU_REFERENCE_PERFORM_REPEAT_START);
                ident_notification = false;
                //return; // Прерываем дальнейшую обработку текущего укола
            }
        }

        else if((mode == WORKING_MODE_PATIENT) && (sub_mode == WORKING_SUB_MODE_PATIENT_MEASUREMENT)) {
            // проверка превышения лимита измерений
            if (session->max_ident_couner >= device_params.advanced_params.measure_params.max_num_of_patient_measurements) {
                session->session_failed = true;
                app_log("MEASUREMENT: Patient Measurement Session failed! Max allowed measurements exceeded (%d)\r\n", device_params.advanced_params.measure_params.max_num_of_patient_measurements);
                // Здесь аварийно останавливаем сессию (сбрасываем состояние АЦП в IDLE)
                //aukh_Post_UI_Event(AU_PATIENT_STATE_FINISHED);
                ident_notification = false;
                //return; // Прерываем дальнейшую обработку текущего укола
            }
        }

        else if((mode == WORKING_MODE_PATIENT) && (sub_mode == WORKING_SUB_MODE_PATIENT_REFERENCE)) {
            // проверка превышения лимита измерений
            if (session->max_ident_couner >= device_params.advanced_params.measure_params.max_num_of_patient_references) {
                session->session_failed = true;
                app_log("MEASUREMENT: Patient Reference Session failed! Max allowed measurements exceeded (%d)\r\n", device_params.advanced_params.measure_params.max_num_of_patient_references);
                // Здесь аварийно останавливаем сессию (сбрасываем состояние АЦП в IDLE)
                //aukh_Post_UI_Event(AU_REFERENCE_PATIENT_REPEAT_START);   /* @ToDo Repeat Reference start in patient mode */
                ident_notification = false;
                //return; // Прерываем дальнейшую обработку текущего укола
            }
        }

        if(ident_notification == true) {
          //Display TIME_OUT indicator + One Indent Required
          find_RemoveAllNotificationIndicators();
          find_DisplayIndicator(FIND_ID_ONE_INDENT_REQUIRED);
          if((mode == WORKING_MODE_PERFORMANCE) && (sub_mode == WORKING_SUB_MODE_PERFORMANCE_MEASUREMENT)) {
             app_log("MATH: Performance Measure Indent [%u] INVALID. Amp=%u\r\n", session->idx, peaks.amplitude);
          }
          if((mode == WORKING_MODE_PERFORMANCE) && (sub_mode == WORKING_SUB_MODE_PERFORMANCE_REFERENCE)) {
             app_log("MATH: Performance Reference Indent [%u] INVALID. Amp=%u\r\n", session->idx, peaks.amplitude);
          }
          if((mode == WORKING_MODE_PATIENT) && (sub_mode == WORKING_SUB_MODE_PATIENT_MEASUREMENT)) {
             app_log("MATH: Patient Measure Indent [%u] INVALID. Amp=%u\r\n", session->idx, peaks.amplitude);
          }
          if((mode == WORKING_MODE_PATIENT) && (sub_mode == WORKING_SUB_MODE_PATIENT_REFERENCE)) {
             app_log("MATH: Patient Reference Indent [%u] INVALID. Amp=%u\r\n", session->idx, peaks.amplitude);
          }
        }

    }

    session->idx++;
    // Здесь же можно вызывать создание пакета и отправку по BLE (transmit_data_via_indication)
    // формируем и отправляем пакет через BLE
    fsrv_DS_SetValidIndentations(session->remain_counter);

    set_num_of_bytes_to_transmit(create_one_measure_packet(tx_data_buffer, session, mode, sub_mode));
    transmit_data_via_indication();
}


// Шаги 4-6: Финальный расчет BMSi
void calculate_final_bmsi(working_mode_t mode)
{
    // 1. Считаем среднее и отклонение для сырых данных пациентской сессии
    calc_stats(measurement_session.bms_current, measurement_session.is_valid, measurement_session.idx,
               &measurement_session.mean, &measurement_session.stddev);

    // Флаг подстановки константы срабатывает ТОЛЬКО в режиме WORKING_MODE_PATIENT при завале референса
    bool use_fallback_constant = ((mode == WORKING_MODE_PATIENT) && reference_session.session_failed);

    if (use_fallback_constant) {
        // Если мы в режиме пациента и референс завален, подставляем константу 0.1f
        reference_session.mean = 0.1f;
        reference_session.stddev = 0.0f;
        final_results.harmonic_mean = 0.1f;
        final_results.is_unstable = false; // Принудительно сбрасываем, так как используем константу
        final_results.is_aproximate = true; // Устанавливаем флаг приблизительного расчета

        app_log("WARNING: Reference session failed in PATIENT mode! Using fallback constant 0.1f (Approximate).\r\n");
    } else {
        // Во всех остальных случаях (успешный референс или другой режим) считаем штатную статистику
        calc_stats(reference_session.bms_current, reference_session.is_valid, reference_session.idx,
                   &reference_session.mean, &reference_session.stddev);

        // Проверка стабильности референса
        final_results.is_unstable = false;
        if (reference_session.mean < 0.09f || reference_session.mean > 1.10f) {
            final_results.is_unstable = true;
            app_log("WARNING: Reference measurements are Unstable!\r\n");
        }

        // 2. Гармоническое среднее референса
        float sum_reciprocals = 0.0f;
        for (uint8_t i = 0; i < reference_session.idx; i++) {
            if (reference_session.is_valid[i] && reference_session.bms_current[i] > 0.0f) {
                sum_reciprocals += 1.0f / reference_session.bms_current[i];
            }
        }
        final_results.harmonic_mean = (sum_reciprocals > 0.0f) ?
                                      ((float)reference_session.valid_counter / sum_reciprocals) : 0.0f;

        final_results.is_aproximate = false; // Расчет штатный, не приблизительный
    }

    // 3. Нормализация обеих сессий
    if (final_results.harmonic_mean > 0.0f) {
        for (uint8_t i = 0; i < measurement_session.idx; i++) {
            if (measurement_session.is_valid[i]) {
                measurement_session.bms_normalized[i] = (measurement_session.bms_current[i] / final_results.harmonic_mean) * 100.0f;
            }
        }
        // Нормализуем референс, только если подстановка константы не выполнялась
        if (!use_fallback_constant) {
            for (uint8_t i = 0; i < reference_session.idx; i++) {
                if (reference_session.is_valid[i]) {
                    reference_session.bms_normalized[i] = (reference_session.bms_current[i] / final_results.harmonic_mean) * 100.0f;
                }
            }
        }
    }

    // 4. Расчет средних нормализованных значений
    float norm_patient_mean, norm_patient_stddev;
    calc_stats(measurement_session.bms_normalized, measurement_session.is_valid, measurement_session.idx,
               &norm_patient_mean, &norm_patient_stddev);

    float norm_ref_mean, norm_ref_stddev;
    if (use_fallback_constant) {
        // Нормализованное среднее референса будет (0.1 / 0.1) * 100 = 100.0f
        norm_ref_mean = 100.0f;
        norm_ref_stddev = 0.0f;
    } else {
        calc_stats(reference_session.bms_normalized, reference_session.is_valid, reference_session.idx, &norm_ref_mean, &norm_ref_stddev);
    }

    // 5. Итоговый BMSi
    final_results.final_bmsi = (norm_ref_mean > 0.0f) ? (norm_patient_mean / norm_ref_mean) * 100.0f : 0.0f;

    fsrv_DS_SetBoneScore(final_results.final_bmsi);
    fsrv_DS_SetIsBoneApproximateScore(final_results.is_aproximate);


    // Pass / Fail (значения границ берутся из настроек)
    float min_bmsi = device_params.advanced_params.measure_params.bmsi_min;
    float max_bmsi = device_params.advanced_params.measure_params.bmsi_max;

    final_results.is_pass = ((final_results.final_bmsi >= min_bmsi && final_results.final_bmsi <= max_bmsi) && (final_results.is_unstable == false));

    app_log("FINAL BMSi = %.2f (Pass: %s, Approx: %s)\r\n", final_results.final_bmsi, final_results.is_pass ? "YES" : "NO", final_results.is_aproximate ? "YES" : "NO");
}

/* * */
uint16_t create_one_measure_packet(uint8_t *ptr, const measurement_session_t *session_data, working_mode_t mode, working_sub_mode_t sub_mode)
{
  one_measure_data_packet_t tmp_packet;

  memset(&tmp_packet,0,sizeof(one_measure_data_packet_t));

  if(session_data->is_valid[session_data->idx - 1] == true) {
      tmp_packet.true_or_false = 'T';
  } else {
      tmp_packet.true_or_false = 'F';
  }

  if((mode == WORKING_MODE_PERFORMANCE) && (sub_mode == WORKING_SUB_MODE_PERFORMANCE_MEASUREMENT)) {
      uint_to_ascii((uint32_t)(session_data->bms_current[session_data->idx - 1]*10000.0f), 3, (uint8_t *)tmp_packet.result);
  }

  if((mode == WORKING_MODE_PERFORMANCE) && (sub_mode == WORKING_SUB_MODE_PERFORMANCE_REFERENCE)) {
      uint_to_ascii((uint32_t)(session_data->bms_current[session_data->idx - 1]*1000.0f), 3, (uint8_t *)tmp_packet.result);
  }

  if((mode == WORKING_MODE_PATIENT) && (sub_mode == WORKING_SUB_MODE_PATIENT_MEASUREMENT)) {
      uint_to_ascii((uint32_t)(session_data->bms_current[session_data->idx - 1]*10000.0f), 3, (uint8_t *)tmp_packet.result);
  }

  if((mode == WORKING_MODE_PATIENT) && (sub_mode == WORKING_SUB_MODE_PATIENT_REFERENCE)) {
      uint_to_ascii((uint32_t)(session_data->bms_current[session_data->idx - 1]*1000.0f), 3, (uint8_t *)tmp_packet.result);
  }
  uint_to_ascii(session_data->valid_counter, 3, (uint8_t *)tmp_packet.valid_counter_pic_ric_bms);
  uint_to_ascii(session_data->remain_counter, 3, (uint8_t *)tmp_packet.remain_counter);
  uint_to_ascii(session_data->invalid_counter, 3,(uint8_t *) tmp_packet.invalid_counter);
  if(session_data->is_valid[session_data->idx - 1] == true) {
      tmp_packet.additional_measure_required = 'F';
  }
  else {
      tmp_packet.additional_measure_required = 'T';
  }

  if((mode == WORKING_MODE_PATIENT) && (sub_mode == WORKING_SUB_MODE_PATIENT_MEASUREMENT)) {
      memcpy(tmp_packet.signature, "PAMR", 4);
  }

  if((mode == WORKING_MODE_PATIENT) && (sub_mode == WORKING_SUB_MODE_PATIENT_REFERENCE)) {
      memcpy(tmp_packet.signature, "PARR", 4);
  }

  if((mode == WORKING_MODE_PERFORMANCE)  && (sub_mode == WORKING_SUB_MODE_PERFORMANCE_MEASUREMENT)) {
      memcpy(tmp_packet.signature, "PEMR", 4);
  }

  if((mode == WORKING_MODE_PERFORMANCE) && (sub_mode == WORKING_SUB_MODE_PERFORMANCE_REFERENCE)) {
      memcpy(tmp_packet.signature, "PERR", 4);
  }
  memcpy(ptr, &tmp_packet,sizeof(one_measure_data_packet_t));
  return (sizeof(one_measure_data_packet_t) /*- 1*/);
}


uint16_t create_intermediate_bms_packet(uint8_t *ptr, const measurement_session_t *session_data, working_mode_t mode, working_sub_mode_t sub_mode)
{
  uint8_t cnt = 0;

  // 1. Status - 1 byte ('T' или 'F')
  // Если вся сессия в итоге валидна или последний укол успешен (ориентируемся на логику вашей структуры)
  if(session_data->session_failed == false) {
      *ptr++ = 'T';
      one_measure_data_packet.true_or_false = 'T';
  }
  else {
      *ptr++ = 'F';
      one_measure_data_packet.true_or_false = 'F';
  }
  cnt++;

  // 2. Bone Score - 3 bytes (ASCII "000"-"999")
  // Берем среднее значение BMS по сессии (session_data->mean) или последний bms_current
  // В ТЗ указано умножение на 100 для перевода в целое (например, 3.45 -> 345)
  uint32_t score_to_send = (uint32_t)(session_data->mean * 100.0f);
  if (score_to_send > 999) score_to_send = 999; // Защита от переполнения ASCII поля
  uint_to_ascii(score_to_send, 4, ptr);
  ptr += 4;
  cnt += 4;

  // 3. Indentation complete mark - 3 bytes
  if((sub_mode == WORKING_SUB_MODE_PATIENT_MEASUREMENT) || (sub_mode == WORKING_SUB_MODE_PERFORMANCE_MEASUREMENT)) {
    *ptr++ = 'P';
  }
  if((sub_mode == WORKING_SUB_MODE_PATIENT_REFERENCE) || (sub_mode == WORKING_SUB_MODE_PERFORMANCE_REFERENCE)){
    *ptr++ = 'R';
  }
  *ptr++ = 'I';
  *ptr++ = 'C';

  cnt += 3;

  // 4. Remaining Indents - 3 bytes (Строго текст "000")
  *ptr++ = '0';
  *ptr++ = '0';
  *ptr++ = '0';
  cnt += 3;

  // 5. Error Indents - 3 bytes (ASCII "000"-"018")
  uint32_t err_cnt = session_data->invalid_counter;
  uint_to_ascii(err_cnt, 3, ptr);
  ptr += 3;
  cnt += 3;

  // 6. Complete/Incomplete - 1 byte ('T' или 'F')
  // Так как это пакет завершения (complete response), выставляем 'T' (0x54)
  // (Либо 'F', если сессия прервана, но судя по ТЗ для complete response здесь 'T')
  *ptr++ = 'T';
  cnt++;

  // 7. Response code - 4 bytes (Определение режима PAMR / PARR / PEMR / PERR)
  if((mode == WORKING_MODE_PATIENT) && (sub_mode == WORKING_SUB_MODE_PATIENT_MEASUREMENT)) {
      *ptr++ = 'P'; *ptr++ = 'A'; *ptr++ = 'M'; *ptr++ = 'R';
      cnt += 4;
      return cnt;
  }

  if((mode == WORKING_MODE_PATIENT) && (sub_mode == WORKING_SUB_MODE_PATIENT_REFERENCE)) {
      *ptr++ = 'P'; *ptr++ = 'A'; *ptr++ = 'R'; *ptr++ = 'R';
      cnt += 4;
      return cnt;
  }

  if((mode == WORKING_MODE_PERFORMANCE) && (sub_mode == WORKING_SUB_MODE_PERFORMANCE_MEASUREMENT)) {
      *ptr++ = 'P'; *ptr++ = 'E'; *ptr++ = 'M'; *ptr++ = 'R';
      cnt += 4;
      return cnt;
  }

  if((mode == WORKING_MODE_PERFORMANCE) && (sub_mode == WORKING_SUB_MODE_PERFORMANCE_REFERENCE)) {
      *ptr++ = 'P'; *ptr++ = 'E'; *ptr++ = 'R'; *ptr++ = 'R';
      cnt += 4;
      return cnt;
  }

  return cnt;
}

void send_intermediate_bms_packet (const measurement_session_t *session_data,  working_mode_t mode, working_sub_mode_t sub_mode)
{
  uint8_t temp;
  temp = create_intermediate_bms_packet(tx_data_buffer, session_data, mode, sub_mode);
  app_log("length of bms(i) %u\r\n",temp);
  set_num_of_bytes_to_transmit(temp);
  transmit_data_via_indication();
}


uint16_t create_final_bmsi_packet(uint8_t *ptr, const final_bmsi_result_t *final_result, working_mode_t mode, working_sub_mode_t sub_mode)
{
  uint8_t cnt = 0;

  // Если вся сессия в итоге валидна или последний укол успешен (ориентируемся на логику вашей структуры)
  if(final_result->is_aproximate == true) {
      *ptr++ = 'F';
  }
  else {
      if(final_result->is_pass == true) {
          *ptr++ = 'T';
      } else {
          *ptr++ = 'F';
      }
  }
  cnt++;

  // 2. Bone Score - 3 bytes (ASCII "000"-"999")
  // Берем среднее значение BMS по сессии (session_data->mean) или последний bms_current
  // В ТЗ указано умножение на 100 для перевода в целое (например, 3.45 -> 345)
  uint32_t score_to_send = (uint32_t)(final_result->final_bmsi * 10.0f);
  //if (score_to_send > 999) score_to_send = 999; // Защита от переполнения ASCII поля
  uint_to_ascii(score_to_send, 4, ptr);
  ptr += 4;
  cnt += 4;

  *ptr++ = 'B';
  *ptr++ = 'M';
  *ptr++ = 'S';

  cnt += 3;

  *ptr++ = '0';
  *ptr++ = '0';
  *ptr++ = '0';
  *ptr++ = '0';
  *ptr++ = '0';
  *ptr++ = '0';

  cnt += 6;

  *ptr++ = 'T';
  cnt++;

  app_log("mode=%d sub_mode=%d\r\n",mode, sub_mode);

  if((mode == WORKING_MODE_PATIENT) && (sub_mode == WORKING_SUB_MODE_PATIENT_MEASUREMENT)) {
      *ptr++ = 'P'; *ptr++ = 'A'; *ptr++ = 'M'; *ptr++ = 'R';
      cnt += 4;
      return cnt;
  }

  if((mode == WORKING_MODE_PATIENT) && (sub_mode == WORKING_SUB_MODE_PATIENT_REFERENCE)) {
      *ptr++ = 'P'; *ptr++ = 'A'; *ptr++ = 'R'; *ptr++ = 'R';
      cnt += 4;
      return cnt;
  }

  if((mode == WORKING_MODE_PERFORMANCE) && (sub_mode == WORKING_SUB_MODE_PERFORMANCE_MEASUREMENT)) {
      *ptr++ = 'P'; *ptr++ = 'E'; *ptr++ = 'M'; *ptr++ = 'R';
      cnt += 4;
      return cnt;
  }

  if((mode == WORKING_MODE_PERFORMANCE) && (sub_mode == WORKING_SUB_MODE_PERFORMANCE_REFERENCE)) {
      *ptr++ = 'P'; *ptr++ = 'E'; *ptr++ = 'R'; *ptr++ = 'R';
      cnt += 4;
      return cnt;
  }

  return cnt;
}

void send_final_bmsi_packet (const final_bmsi_result_t *final_result, working_mode_t mode, working_sub_mode_t sub_mode)
{
  set_num_of_bytes_to_transmit(create_final_bmsi_packet(tx_data_buffer, final_result, mode, sub_mode));
  transmit_data_via_indication();
  fs_sd_historical_records_write_data(tx_data_buffer, 19, false, device_params.advanced_params.has_raw_data, NULL, false, false, true);
}

uint16_t create_final_bmsi_packet_failed(uint8_t *ptr, const final_bmsi_result_t *final_result, working_mode_t mode, working_sub_mode_t sub_mode)
{
  uint8_t cnt = 0;

  *ptr++ = 'F';
  cnt++;

  // 2. Bone Score - 3 bytes (ASCII "000"-"999")
  // Берем среднее значение BMS по сессии (session_data->mean) или последний bms_current
  // В ТЗ указано умножение на 100 для перевода в целое (например, 3.45 -> 345)
  uint32_t score_to_send = (uint32_t)(final_result->final_bmsi * 10.0f);
  //if (score_to_send > 999) score_to_send = 999; // Защита от переполнения ASCII поля
  uint_to_ascii(score_to_send, 4, ptr);
  ptr += 4;
  cnt += 4;

  *ptr++ = 'B';
  *ptr++ = 'M';
  *ptr++ = 'S';

  cnt += 3;

  *ptr++ = '0';
  *ptr++ = '0';
  *ptr++ = '0';
  *ptr++ = '0';
  *ptr++ = '0';
  *ptr++ = '0';

  cnt += 6;

  *ptr++ = 'T';
  cnt++;

  app_log("mode=%d sub_mode=%d\r\n",mode, sub_mode);

  if((mode == WORKING_MODE_PATIENT) && (sub_mode == WORKING_SUB_MODE_PATIENT_MEASUREMENT)) {
      *ptr++ = 'P'; *ptr++ = 'A'; *ptr++ = 'M'; *ptr++ = 'R';
      cnt += 4;
      return cnt;
  }

  if((mode == WORKING_MODE_PATIENT) && (sub_mode == WORKING_SUB_MODE_PATIENT_REFERENCE)) {
      *ptr++ = 'P'; *ptr++ = 'A'; *ptr++ = 'R'; *ptr++ = 'R';
      cnt += 4;
      return cnt;
  }

  if((mode == WORKING_MODE_PERFORMANCE) && (sub_mode == WORKING_SUB_MODE_PERFORMANCE_MEASUREMENT)) {
      *ptr++ = 'P'; *ptr++ = 'E'; *ptr++ = 'M'; *ptr++ = 'R';
      cnt += 4;
      return cnt;
  }

  if((mode == WORKING_MODE_PERFORMANCE) && (sub_mode == WORKING_SUB_MODE_PERFORMANCE_REFERENCE)) {
      *ptr++ = 'P'; *ptr++ = 'E'; *ptr++ = 'R'; *ptr++ = 'R';
      cnt += 4;
      return cnt;
  }

  return cnt;
}

void send_final_bmsi_packet_failed (const final_bmsi_result_t *final_result, working_mode_t mode, working_sub_mode_t sub_mode)
{
  set_num_of_bytes_to_transmit(create_final_bmsi_packet_failed(tx_data_buffer, final_result, mode, sub_mode));
  transmit_data_via_indication();
  fs_sd_historical_records_write_data(tx_data_buffer, 19, false, device_params.advanced_params.has_raw_data, NULL, false, false, true);
}


Bool fsrv_DS_getPerformanceFinalResult (void) {
  return (Bool)(final_results.is_pass == TRUE);
}

uint16_t create_pic_or_ric_fail_session(uint8_t *ptr, const measurement_session_t *session_data, working_mode_t mode, working_sub_mode_t sub_mode)
{
  uint8_t cnt = 0;

  // 1. Status - 1 byte ('T' или 'F')
  // Если вся сессия в итоге валидна или последний укол успешен (ориентируемся на логику вашей структуры)
  if(session_data->session_failed == false) {
      *ptr++ = 'T';
      one_measure_data_packet.true_or_false = 'T';
  }
  else {
      *ptr++ = 'F';
      one_measure_data_packet.true_or_false = 'F';
  }
  cnt++;

  // 2. Bone Score - 3 bytes (ASCII "000"-"999")
  // Берем среднее значение BMS по сессии (session_data->mean) или последний bms_current
  // В ТЗ указано умножение на 100 для перевода в целое (например, 3.45 -> 345)
  uint32_t score_to_send = 0;

  uint_to_ascii(score_to_send, 4, ptr);
  ptr += 4;
  cnt += 4;

  // 3. Indentation complete mark - 3 bytes
  if((sub_mode == WORKING_SUB_MODE_PATIENT_MEASUREMENT) || (sub_mode == WORKING_SUB_MODE_PERFORMANCE_MEASUREMENT)) {
    *ptr++ = 'P';
  }
  if((sub_mode == WORKING_SUB_MODE_PATIENT_REFERENCE) || (sub_mode == WORKING_SUB_MODE_PERFORMANCE_REFERENCE)){
    *ptr++ = 'R';
  }
  *ptr++ = 'I';
  *ptr++ = 'C';

  cnt += 3;

  // 4. Remaining Indents - 3 bytes (Строго текст "000")
  *ptr++ = '0';
  *ptr++ = '0';
  *ptr++ = '0';
  cnt += 3;

  // 5. Error Indents - 3 bytes (ASCII "000"-"018")
  uint32_t err_cnt = session_data->invalid_counter;
  uint_to_ascii(err_cnt, 3, ptr);
  ptr += 3;
  cnt += 3;

  // 6. Complete/Incomplete - 1 byte ('T' или 'F')
  // Так как это пакет завершения (complete response), выставляем 'T' (0x54)
  // (Либо 'F', если сессия прервана, но судя по ТЗ для complete response здесь 'T')
  *ptr++ = 'F';
  cnt++;

  // 7. Response code - 4 bytes (Определение режима PAMR / PARR / PEMR / PERR)
  if((mode == WORKING_MODE_PATIENT) && (sub_mode == WORKING_SUB_MODE_PATIENT_MEASUREMENT)) {
      *ptr++ = 'P'; *ptr++ = 'A'; *ptr++ = 'M'; *ptr++ = 'R';
      cnt += 4;
      return cnt;
  }

  if((mode == WORKING_MODE_PATIENT) && (sub_mode == WORKING_SUB_MODE_PATIENT_REFERENCE)) {
      *ptr++ = 'P'; *ptr++ = 'A'; *ptr++ = 'R'; *ptr++ = 'R';
      cnt += 4;
      return cnt;
  }

  if((mode == WORKING_MODE_PERFORMANCE) && (sub_mode == WORKING_SUB_MODE_PERFORMANCE_MEASUREMENT)) {
      *ptr++ = 'P'; *ptr++ = 'E'; *ptr++ = 'M'; *ptr++ = 'R';
      cnt += 4;
      return cnt;
  }

  if((mode == WORKING_MODE_PERFORMANCE) && (sub_mode == WORKING_SUB_MODE_PERFORMANCE_REFERENCE)) {
      *ptr++ = 'P'; *ptr++ = 'E'; *ptr++ = 'R'; *ptr++ = 'R';
      cnt += 4;
      return cnt;
  }

  return cnt;
}


void send_pic_or_ric_fail_session_packet (const measurement_session_t *session_data, working_mode_t mode, working_sub_mode_t sub_mode)
{
  uint8_t temp = create_pic_or_ric_fail_session(tx_data_buffer, session_data, mode, sub_mode);

  app_log("length of PicRic(F) %u\r\n",temp);
  set_num_of_bytes_to_transmit(temp);
  transmit_data_via_indication();
  if((sub_mode == WORKING_SUB_MODE_PATIENT_MEASUREMENT) || (sub_mode == WORKING_SUB_MODE_PERFORMANCE_MEASUREMENT)) {
      fs_sd_historical_records_write_data(tx_data_buffer, 19, false, device_params.advanced_params.has_raw_data, NULL, true, false, false);
  }
  if((sub_mode == WORKING_SUB_MODE_PATIENT_REFERENCE) || (sub_mode == WORKING_SUB_MODE_PERFORMANCE_REFERENCE)){
      fs_sd_historical_records_write_data(tx_data_buffer, 19, false, device_params.advanced_params.has_raw_data, NULL, false, true, false);
  }
}

uint8_t get_measurement_ident_counter(void) {
  return measurement_session.max_ident_couner;
}

uint8_t get_reference_ident_counter(void) {
  return measurement_session.max_ident_couner;
}

