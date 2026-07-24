#include "battery_management.h"

// Флаг присутствия девайса, доступный для всего проекта
volatile bool is_device_present = false;

// Глобальная структура статуса батареи
static battery_status_t current_battery_status = {0, 0, BATTERY_DISCHARGING};

// Буфер для накопления замеров АЦП
static uint16_t battery_samples[BATTERY_SAMPLE_POINTS] = {0};
static uint16_t battery_sample_idx = 0;
static bool battery_buffer_filled = false;

extern volatile adc_capture_state_t adc_current_state;
extern volatile uint16_t adc_battery_level;
extern measurement_machine_state_t measurement_state_machine;
extern device_params_t device_params;
extern device_operation_control_t   device_operation_control;

/**
 * @brief Точки разрядной кривой Li-Po (Емкость 400мАч).
 * Адаптировано под постоянную нагрузку в 60 мА.
 */
typedef struct {
    uint16_t voltage;
    uint8_t percentage;
} battery_curve_point_t;


static const battery_curve_point_t lipo_discharging_curve[] = {
    {3210, 100}, // Максимум под нагрузкой (Ваша точка полной зарядки)
    {3166, 95},
    {3127, 90},
    {3096, 85},
    {3058, 80},
    {3019, 75},
    {2981, 70},
    {2950, 65},
    {2927, 60},  // Рабочее плато
    {2904, 55},
    {2888, 50},
    {2873, 45},
    {2857, 40},
    {2842, 35},
    {2826, 30},
    {2811, 25},  // Конец плато
    {2795, 20},
    {2772, 15},
    {2741, 10},  // Критическая точка
    {2702, 5},
    {2664, 3},
    {2587, 1},
    {2470, 0}    // Полный разряд под нагрузкой
};

#define BATTERY_CURVE_SIZE (sizeof(lipo_discharging_curve) / sizeof(lipo_discharging_curve[0]))

/**
 * @brief Функция сортировки для вычисления медианы
 */
static void sort_buffer(uint16_t *arr, uint8_t n) {
    for (uint8_t i = 0; i < n - 1; i++) {
        for (uint8_t j = 0; j < n - i - 1; j++) {
            if (arr[j] > arr[j + 1]) {
                uint16_t temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

/**
 * @brief Перевод напряжения в процент заряда по нелинейной кривой
 */
static uint8_t voltage_to_percentage(uint32_t voltage_mv) {
    if (voltage_mv >= lipo_discharging_curve[0].voltage) return 100;
    if (voltage_mv <= lipo_discharging_curve[BATTERY_CURVE_SIZE - 1].voltage) return 0;

    for (size_t i = 0; i < BATTERY_CURVE_SIZE - 1; i++) {
        if (voltage_mv <= lipo_discharging_curve[i].voltage && voltage_mv > lipo_discharging_curve[i+1].voltage) {
            uint32_t v_high = lipo_discharging_curve[i].voltage;
            uint32_t v_low  = lipo_discharging_curve[i+1].voltage;
            uint8_t  p_high = lipo_discharging_curve[i].percentage;
            uint8_t  p_low  = lipo_discharging_curve[i+1].percentage;

            return p_low + ((voltage_mv - v_low) * (p_high - p_low)) / (v_high - v_low);
        }
    }
    return 0;
}

/**
 * @brief Перевод попугаев АЦП в милливольты.
 */
static uint32_t convert_adc_to_mv(uint16_t adc_raw, adc_vref_source_t vref) {
    /*uint32_t vref_mv = 1210;
    if (vref == ADC_VREF_INTERNAL_3P0V)  vref_mv = 3000;
    if (vref == ADC_VREF_EXTERNAL_PA00) vref_mv = 3000;
    if (vref == ADC_VREF_INTERNAL_1P21V) vref_mv = 1210;

    uint32_t measured_pin_mv = (adc_raw * vref_mv) / 4095;
    uint32_t battery_mv = measured_pin_mv + ((measured_pin_mv * 26 + 5) / 11); // Делитель напряжения 26:11;

    return battery_mv*15/10;*/
     uint32_t vref_mv = 1210; // Значение по умолчанию
     // Выбор опорного напряжения согласно вашему условию
      if (vref == ADC_VREF_INTERNAL_3P0V)  vref_mv = 3000;
      if (vref == ADC_VREF_EXTERNAL_PA00) vref_mv = 3000;
      if (vref == ADC_VREF_INTERNAL_1P21V) vref_mv = 1210;

      // Расчет напряжения на самом пине микроконтроллера (0..4095)
      uint32_t measured_pin_mv = ((uint32_t)adc_raw * vref_mv) / 4095;

      // Пересчет через аппаратный делитель (390к и 165к дают соотношение 26:11)
      uint32_t battery_mv = measured_pin_mv + ((measured_pin_mv * 26 + 5) / 11);

      // Если АЦП ушел в жесткое насыщение (4095), физический предел для опоры 1.21В
      // составит ровно 4070 мВ. Если вам нужно, чтобы при насыщении функция
      // возвращала ровно 4177 мВ (или 4200 мВ), можно добавить этот программный хак:
      // Если АЦП ушел в жесткое насыщение (4095), возвращаем наш новый максимум
      if (vref == ADC_VREF_INTERNAL_1P21V && adc_raw >= 3300) {
         return 3210; // Потолок для вашей полностью заряженной батареи
      }

      return battery_mv;;
}

/**
 * @brief Возвращает текущий заряд аккумулятора в процентах (0-100%).
 */
uint8_t get_battery_percentage(void) {
  if(current_battery_status.charge_percentage == 0xFF){
      return 0;
  }
  else {
    return current_battery_status.charge_percentage;
  }
}

/**
 * @brief Проверяет, идет ли сейчас зарядка.
 * @return true - устройство заряжается, false - работает от батареи.
 */
bool is_battery_charging(void) {
    return (current_battery_status.state == BATTERY_CHARGING);
}

void check_system_status(void) {
    uint8_t percent = get_battery_percentage();

    if(percent == 0xFF){
        device_operation_control.battary.battery_failed = 1;
        device_operation_control.battary.battery_level = 0xFF;
        //app_log("BATTERY: Disconnected powered from USB\r\n");
    } else {
        if (is_battery_charging()) {
            device_operation_control.battary.battery_charging = 1;
            device_operation_control.battary.battery_failed = 0;
            device_operation_control.battary.battery_level = percent;
            //app_log("BATTERY: Charging, current level: %d%%\r\n", percent);
        } else {
            device_operation_control.battary.battery_charging = 0;
            device_operation_control.battary.battery_failed = 0;
            device_operation_control.battary.battery_level = percent;
            //app_log("BATTERY: Discharging, current level: %d%%\r\n", percent);
        }
    }
}

/**
 * @brief Основной процесс периодического контроля батареи.
 */
/**
 * @brief Основной процесс периодического контроля батареи.
 */
void battery_management_process(void)
{
    static uint32_t last_measure_time = 0;
    static uint32_t last_recalc_time = 0;
    static enum {
        BAT_STATE_IDLE,
        BAT_STATE_WAIT_ADC
    } bat_step = BAT_STATE_IDLE;

    uint32_t current_time = sl_sleeptimer_tick_to_ms(sl_sleeptimer_get_tick_count());

    switch (bat_step) {
        case BAT_STATE_IDLE:
            // Запускаем АЦП, только если измерительная сессия не активна и периферия АЦП полностью свободна
            if (measurement_state_machine == MACHNE_STATE_IDLE && adc_current_state == ADC_STATE_IDLE)
            {
                if (current_time - last_measure_time >= BATTERY_MEASURE_INTERVAL_MS) {
                    last_measure_time = current_time;
                    // Аппаратный запуск серии сканирования каналов через PRS и TIMER1
                    read_idle_samples();
                    bat_step = BAT_STATE_WAIT_ADC;
                }
            }
            break;

        case BAT_STATE_WAIT_ADC:
            // Ждём, пока прерывание IADC_IRQHandler завершит накопление серий и переведёт состояние в ADC_STATE_DONE
            if (adc_current_state != ADC_STATE_DONE) {
                return;
            }

            // Безопасно (атомарно) забираем уже усредненный внутри прерывания замер канала батареи
            uint16_t sample = adc_battery_level;

            battery_samples[battery_sample_idx++] = sample;
            if (battery_sample_idx >= BATTERY_SAMPLE_POINTS) {
                battery_sample_idx = 0;
                battery_buffer_filled = true;
            }

            // Возвращаем программный автомат АЦП обратно в IDLE, чтобы периферия была доступна для других процессов
            adc_current_state = ADC_STATE_IDLE;
            bat_step = BAT_STATE_IDLE;
            break;
    }

    // --- БЛОК ОЦЕНКИ, ФИЛЬТРАЦИИ И РАСЧЕТА ПРОЦЕНТОВ ---
    if (current_time - last_recalc_time >= BATTERY_RECALC_INTERVAL_MS) {

        // Берем снимок индекса, чтобы memcpy не разъехался при прерывании
        uint16_t current_idx = battery_sample_idx;
        uint8_t active_points = battery_buffer_filled ? BATTERY_SAMPLE_POINTS : current_idx;

        if (active_points < 5) {
            // Сдвигаем время следующей проверки чуть-чуть вперед (например на 100мс),
            // чтобы не спамить проверку на каждом тике процессора, пока буфер пуст
            last_recalc_time = current_time - BATTERY_RECALC_INTERVAL_MS + 100;
            return;
        }

        // Копируем накопленный буфер во временный для сортировки
        uint16_t temp_buffer[BATTERY_SAMPLE_POINTS];
        memcpy(temp_buffer, (const void*)battery_samples, active_points * sizeof(uint16_t));

        // Сортируем буфер для вычисления медианы
        sort_buffer(temp_buffer, active_points);

        // Отбрасываем по 1/6 худших результатов с каждого края
        uint8_t discard_count = active_points / 6;
        uint32_t adc_sum = 0;
        uint8_t sum_count = 0;

        for (uint8_t i = discard_count; i < (active_points - discard_count); i++) {
            adc_sum += temp_buffer[i];
            sum_count++;
        }

        // Вычисляем отфильтрованное значение АЦП
        uint16_t filtered_adc = adc_sum / sum_count;

        // Сохраняем старые параметры для анализа изменений
        uint32_t old_voltage = current_battery_status.voltage_mv;
        uint8_t  old_percentage = current_battery_status.charge_percentage;

        // Расчет вольтажа в милливольтах (передаем filtered_adc)
        current_battery_status.voltage_mv = convert_adc_to_mv(filtered_adc, ADC_VREF_INTERNAL_1P21V);

        // Фиксируем время успешного захода в расчет (предотвращает частые перезапуски блока)
        last_recalc_time = current_time;

        // --- 1. ШАГ: СНАЧАЛА И КРИТИЧЕСКИ ВАЖНО — ПРОВЕРЯЕМ ПРИСУТСТВИЕ БАТАРЕИ ---
        // Так как наводка без батареи составляет около 3098 мВ, порог отсечки выставлен в 3120 мВ.
        // При подключении севшей батареи напряжение жестко просядет ниже 3000 мВ, что не сломает логику.
        if (current_battery_status.voltage_mv < 3120) {
            if (current_battery_status.charge_percentage != 0xFF) {
                GPIO_PinModeSet(LBO_PORT, LBO_PIN, gpioModePushPull, 0);
            }
            current_battery_status.charge_percentage = 0xFF;
            current_battery_status.state = BATTERY_DISCHARGING;

            app_log("BAT_DBG: NO BATTERY DETECTED (V=%lu mV)\r\n", current_battery_status.voltage_mv);
            check_system_status();
            return; // Мгновенный выход, чтобы наводка не ломала логику заряда/разряда ниже
        }

        // --- 2. ШАГ: ОПРЕДЕЛЕНИЕ СОСТОЯНИЯ ЗАРЯДКИ (Выполняется только если батарея есть) ---
        if (old_voltage != 0 && old_percentage != 0xFF) {
            if (current_battery_status.voltage_mv > old_voltage + 15) {
                if (current_battery_status.state != BATTERY_CHARGING) {
                    GPIO_PinModeSet(LBO_PORT, LBO_PIN, gpioModePushPull, 1);
                }
                current_battery_status.state = BATTERY_CHARGING;
            } else if (current_battery_status.voltage_mv < old_voltage - 5) {
                current_battery_status.state = BATTERY_DISCHARGING;
            }
        }

        // --- 3. ШАГ: ВЫЧИСЛЕНИЕ ПРОЦЕНТОВ ПО ТАБЛИЦЕ ---
        // Вычисляем "сырые" проценты по обновленной разрядной табличной кривой
        uint8_t calculated_percentage = voltage_to_percentage(current_battery_status.voltage_mv);

        // Защита от восстановления процентов (а-ля гистерезис)
        if (old_voltage == 0 || old_percentage == 0xFF) {
            current_battery_status.charge_percentage = calculated_percentage;
        }
        else if (current_battery_status.state == BATTERY_DISCHARGING) {
            if (calculated_percentage < old_percentage) {
                current_battery_status.charge_percentage = calculated_percentage;
                GPIO_PinModeSet(LBO_PORT, LBO_PIN, gpioModePushPull, 1);
            }
        }
        else {
            if (calculated_percentage >= old_percentage) {
                current_battery_status.charge_percentage = calculated_percentage;
                if (calculated_percentage > old_percentage) {
                    GPIO_PinModeSet(LBO_PORT, LBO_PIN, gpioModePushPull, 1);
                }
            }
        }

        // Финальное управление пином LBO по критическому порогу
        if ((current_battery_status.charge_percentage < device_params.general_params.battery_threshold_critical) &&
            (current_battery_status.state == BATTERY_DISCHARGING))
        {
            GPIO_PinModeSet(LBO_PORT, LBO_PIN, gpioModePushPull, 0);
        }
        else {
            GPIO_PinModeSet(LBO_PORT, LBO_PIN, gpioModePushPull, 1);
        }

        app_log("BAT_DBG: ADC_filt=%u, V=%lu mV, Charge=%u%%, State=%s\r\n",
                filtered_adc,
                current_battery_status.voltage_mv,
                current_battery_status.charge_percentage,
                (current_battery_status.state == BATTERY_CHARGING) ? "CHARGING" : "DISCHARGING");

        check_system_status();
    }
}

/*=======================================================================*/
/*                                                                       */
/*=======================================================================*/
#define DEBUG_CHARGE_BATTERY

#ifdef DEBUG_CHARGE_BATTERY
#define FSRV_TEST_VALUE_UPPER 55
#define FSRV_TEST_VALUE_LOWER 1
static int8_t  direction     = -1;   // -1 = counting down, 1 = counting up
uint8_t fsrv_DS_StepOnePingPongValue(void)
{
  // Allocate within .data/.bss. Initialized to upper limit at boot.
  static uint8_t current_value = FSRV_TEST_VALUE_UPPER;

  // Apply the execution step
  current_value += direction;

  // Evaluate boundary thresholds and reverse direction dynamically
  if (current_value <= FSRV_TEST_VALUE_LOWER) {
    current_value = FSRV_TEST_VALUE_LOWER;
    direction = 1;  // Reverse direction to counting up
  }
  else if (current_value >= FSRV_TEST_VALUE_UPPER) {
    current_value = FSRV_TEST_VALUE_UPPER;
    direction = -1; // Reverse direction to counting down
  }

  return current_value;
}
#endif
/*=======================================================================*/
/*                                                                       */
/*=======================================================================*/
uint8_t fsrv_DS_GetBatteryLevel(void) {
#ifdef DEBUG_CHARGE_BATTERY
  ///Battery Charging debug
  return fsrv_DS_StepOnePingPongValue();
#else
 return (uint8_t)(device_operation_control.battary.battery_level);
#endif
}

/*=======================================================================*/
/*                                                                       */
/*=======================================================================*/
Bool fsrv_DS_IsBatteryCharging(void) {

#ifdef DEBUG_CHARGE_BATTERY

  uint8_t bat_level  = fsrv_DS_GetBatteryLevel();
///Battery Charging debug
  if ((bat_level < FSRV_TEST_VALUE_LOWER + 2) || ((direction == 1) && (bat_level <= FSRV_TEST_VALUE_UPPER))  )
    return TRUE;
  else
    return FALSE;
#else
  return (Bool)(device_operation_control.battary.battery_charging == TRUE);
#endif
}

