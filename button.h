#ifndef __ALS_BUTTON_H__
#define __ALS_BUTTON_H__

#include <stdbool.h>
#include <stdint.h>

/* Низкоуровневая периферия Silicon Labs */
#include "em_device.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "gpiointerrupt.h"

#include "buzzer.h"

/* Системные компоненты */
#include "sl_sleeptimer.h"
#include "app_log.h"
#include "aukh.h"

// Жесткое определение аппаратного пина (BGM220SC22HNA)
#define BUTTON_PORT              gpioPortB
#define BUTTON_PIN               1

// Временные интервалы (мс)
#define DEBOUNCE_TIME_MS          30    // Время фильтрации дребезга контактов
#define MULTI_PRESS_DURATION      500   // Окно для регистрации пачки кликов
#define LONG_PRESS_DURATION       1500  // Время удержания для Long Press (1 сек)
#define VERY_LONG_PRESS_DURATION  4100  // Время удержания для Very Long Press (3 сек)

void button_feature_init(void);
void button_beep_process (void);

#endif
