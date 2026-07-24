#ifndef __ALS_BATTERY_MANAGEMENT_H__
#define __ALS_BATTERY_MANAGEMENT_H__

#include <stdbool.h>
#include "sl_sleeptimer.h"
#include "app_log.h"
#include "adc.h"
#include "params.h"
#include "ble_communication.h"

#define LBO_PORT   SL_GPIO_PORT_D
#define LBO_PIN    3

// --- Конфигурация управления батареей ---
#define BATTERY_SAMPLE_POINTS     30        // Количество точек для фильтрации (от 10 до 50)
#define BATTERY_MEASURE_INTERVAL_MS 100      // Интервал между одиночными замерами (1 сек)
#define BATTERY_RECALC_INTERVAL_MS  5000    // Интервал полного пересчета (например, каждые 5 сек)

// Состояния процесса зарядки
typedef enum {
    BATTERY_DISCHARGING, // Разряжается (работает от батареи)
    BATTERY_CHARGING     // Заряжается
} battery_charge_state_t;

// Структура для хранения текущего статуса батареи
typedef struct {
    uint32_t voltage_mv;       // Напряжение в милливольтах (например, 4150)
    uint8_t  charge_percentage;// Заряд в процентах (0-100%)
    battery_charge_state_t state; // Состояние (заряжается/разряжается)
} battery_status_t;

void battery_management_process(void);
uint8_t get_battery_percentage(void); // возвращает процент заряда
bool is_battery_charging(void); // возвращает состояние заряжается/не заряжается
void check_system_status(void);

#endif
