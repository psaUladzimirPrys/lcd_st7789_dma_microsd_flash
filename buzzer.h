#ifndef __ALS_BUZZER_H__
#define __ALS_BUZZER_H__

#include "em_cmu.h"
#include "em_gpio.h"
#include "em_timer.h"
#include "sl_sleeptimer.h"
#include "app_log.h"

#define BUZZER_PORT              gpioPortD
#define BUZZER_PIN               2
// timerPrescale8 дает хороший диапазон для слышимых частот (от ~150 Гц до 20+ кГц)
#define BUZZER_TIMER             TIMER0
#define BUZZER_TIMER_PRESCALER   timerPrescale8
#define BUZZER_PRESCALER_DIV     8

void init_buzzer(void);
void buzzer_beep(uint32_t freq_hz, uint32_t duration_ms);

#endif
