#ifndef __ALS_LED_H__
#define __ALS_LED_H__

#include <stdint.h>
#include <stdbool.h>
#include "em_gpio.h"
#include "em_cmu.h"
#include "sl_sleeptimer.h"
#include "app_log.h"

// Аппаратная привязка для модуля BGM220S
#define LED_PORT       gpioPortC
#define LED_PIN        6

// Специальный маркер для бесконечного / постоянного действия
#define LED_INFINITE   ((uint32_t)0xFFFFFFFF)

void led_init(void);

void led_control(uint32_t interval_or_duration_ms, uint32_t count);

/*
//как использовать
led_control(70, 0); // включается один раз ровно на 70 миллисекунд
led_control(3000, LED_INFINITE); // начинает мигать бесконечно: он горит 1.5 секунды, затем 1.5 секунды не горит
                                 // полный цикл мигания занимает 3 секунды
led_control(200, 2); // делает ровно две быстрые вспышки: включается на 100 мс и гаснет на 100 мс после этого он остается полностью выключенным.
led_control(150, LED_INFINITE); // начинает очень быстро и бесконечно мигать. меняет состояние каждые 75 миллисекунд.
led_control(0, 0); // мгновенно выключается в какое бы время или режиме до этого он ни находился
led_control(LED_INFINITE, 0); // включается и горит постоянно, не мигая и не выключаясь сам по себе

 */

#endif
