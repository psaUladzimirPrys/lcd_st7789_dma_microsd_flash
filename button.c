#include "button.h"

bool make_signal_for_button = false;
bool press_long_signal_done = true;
bool press_very_long_signal_done = true;
uint32_t button_signal_timer = 0;

// Состояния автомата кнопки
typedef enum {
    STATE_IDLE = 0,
    STATE_PRESSED
} button_state_t;

// Контекст трекера кнопки (17 байт в RAM, защищен от гонки данных)
typedef struct {
    uint32_t        press_start_tick;
    uint32_t        last_release_tick;
    uint32_t        last_irq_tick;
    uint8_t         click_counter;
    button_state_t  current_state;
} button_hardware_tracker_t;

static button_hardware_tracker_t btn_track;

// Хэндл таймера закрытия окна множественных кликов
static sl_sleeptimer_timer_handle_t timer_click_window;

/* --- ПРОТОТИПЫ ВНУТРЕННИХ ОБРАБОТЧИКОВ --- */
static void cb_click_window_timeout(sl_sleeptimer_timer_handle_t *handle, void *data);
static void button_gpio_interrupt_handler(uint8_t pin);

/* --- СЕКЦИЯ 1: ИНИЦИАЛИЗАЦИЯ АППАРАТНОГО УРОВНЯ --- */

void button_feature_init(void)
{
    // 1. Детерминированный сброс структуры состояния
    btn_track.press_start_tick  = 0;
    btn_track.last_release_tick = 0;
    btn_track.last_irq_tick     = 0;
    btn_track.click_counter     = 0;
    btn_track.current_state     = STATE_IDLE;

    // 2. Включение тактирования периферии GPIO
    CMU_ClockEnable(cmuClock_GPIO, true);

    // 3. Настройка пина: Вход, внутренняя подтяжка к VMCU (активный уровень - 0)
    GPIO_PinModeSet(BUTTON_PORT, BUTTON_PIN, gpioModeInputPullFilter, 1);

    // 4. Инициализация диспетчера прерываний ядра GPIOINT
    GPIOINT_Init();

    // 5. Регистрация низкоуровневого обработчика на пин
    GPIOINT_CallbackRegister(BUTTON_PIN, button_gpio_interrupt_handler);

    // 6. Конфигурация прерывания на оба фронта: спад (нажатие) и подъем (отпускание)
    GPIO_ExtIntConfig(BUTTON_PORT, BUTTON_PIN, BUTTON_PIN, true, true, true);

    app_log("BUTTON: initialized\r\n");
}

/* --- СЕКЦИЯ 2: ОБРАБОТЧИКИ ТАЙМЕРОВ (КОНТЕКСТ SYSTEM TIMER ISR) --- */

/**
 * Фиксация пачки кликов. Вызывается асинхронно, если кнопка спокойна > MULTI_PRESS_DURATION.
 * Отрабатывает в режиме энергосбережения EM2 DeepSleep.
 */
static void cb_click_window_timeout(sl_sleeptimer_timer_handle_t *handle, void *data)
{
    (void)handle; (void)data;

    uint8_t final_clicks = btn_track.click_counter;
    btn_track.click_counter = 0; // Атомарный сброс счетчика до вызова логики

    if (final_clicks > 10) {
        final_clicks = 10; // Защита диапазона по ТЗ (1..10)
    }

    app_log("BUTTON: Clicks detected = %d\r\n", final_clicks);

    // Сюда можно вернуть интеграцию с вашей системой событий aukh:
    switch (final_clicks) {
         case 1: aukh_PostButtonEvent(AU_KEY_PRESS_SHORT);        break;
         case 2: aukh_PostButtonEvent(AU_KEY_PRESS_MULTI_2_TIME); break; /*@ToDo UP temporary button to start Patient mes */
         case 3: aukh_PostButtonEvent(AU_KEY_PRESS_MULTI_3_TIME); break;
         case 5: aukh_PostButtonEvent(AU_KEY_PRESS_MULTI_5_TIME); break;
        default: break;
    }
}

/* --- СЕКЦИЯ 3: НИЗКОУРОВНЕВЫЙ ОБРАБОТЧИК GPIO (КОНТЕКСТ INTERRUPT ISR) --- */

static void button_gpio_interrupt_handler(uint8_t pin)
{
  if (pin != BUTTON_PIN) {
          return;
      }

      uint32_t now = sl_sleeptimer_get_tick_count();

      // Программный антидребезг
      if (btn_track.last_irq_tick != 0) {
          if (sl_sleeptimer_tick_to_ms(now - btn_track.last_irq_tick) < DEBOUNCE_TIME_MS) {
              return;
          }
      }
      btn_track.last_irq_tick = now;

      bool is_released = (GPIO_PinInGet(BUTTON_PORT, BUTTON_PIN) != 0);

      if (!is_released) {
          /* --- ФРОНТ НАЖАТИЯ (FALLING EDGE) --- */
          btn_track.press_start_tick = now;
          btn_track.current_state = STATE_PRESSED;

          // Отменяем таймер фиксации прошлой пачки
          sl_sleeptimer_stop_timer(&timer_click_window);

          // При самом нажатии прибор МОЛЧИТ (звук будет при отпускании)
          make_signal_for_button = true;
          button_signal_timer = sl_sleeptimer_tick_to_ms(now);
          press_long_signal_done = false;
          press_very_long_signal_done = false;
      }
      else {
          /* --- ФРОНТ ОТПУСКАНИЯ (RISING EDGE) --- */
          // Кнопку отпустили — сбрасываем фоновый таймер
          make_signal_for_button = false;
          button_signal_timer = 0;

          if (btn_track.current_state == STATE_PRESSED) {
              uint32_t duration_ms = sl_sleeptimer_tick_to_ms(now - btn_track.press_start_tick);

              if (duration_ms >= VERY_LONG_PRESS_DURATION) {
                  btn_track.click_counter = 0;
                  app_log("BUTTON: VERY LONG press detected (%d ms)\r\n", (int)duration_ms);
                  aukh_PostButtonEvent(AU_KEY_PRESS_VERY_LONG);

                  // Звук для VERY_LONG уже отыграл по времени в button_beep_process(),
                  // поэтому при отпускании здесь полная ТИШИНА.
              }
              else if (duration_ms >= LONG_PRESS_DURATION) {
                  btn_track.click_counter = 0;
                  app_log("BUTTON: LONG press detected (%d ms)\r\n", (int)duration_ms);
                  aukh_PostButtonEvent(AU_KEY_PRESS_LONG);

                  // ИЗМЕНЕНО: Сигнализируем длинное нажатие (125 мс) при ОТПУСКАНИИ
                  buzzer_beep(3500, 300);
              }
              else {
                  btn_track.click_counter++;
                  btn_track.last_release_tick = now;

                  // ИЗМЕНЕНО: Короткий клик (50 мс) звучит при ОТПУСКАНИИ кнопки
                  buzzer_beep(3500, 150);

                  sl_sleeptimer_restart_timer_ms(&timer_click_window, MULTI_PRESS_DURATION, cb_click_window_timeout, NULL, 0, 0);
              }
          }
          btn_track.current_state = STATE_IDLE;
      }
}

void button_beep_process (void)
{
  // Если кнопка не зажата — ничего не делаем
    if (button_signal_timer == 0 || make_signal_for_button == false) {
        return;
    }

    uint32_t current_time_ms = sl_sleeptimer_tick_to_ms(sl_sleeptimer_get_tick_count());
    uint32_t elapsed_ms = current_time_ms - button_signal_timer;

    // Отслеживаем ТОЛЬКО порог VERY LONG в реальном времени
    if (elapsed_ms > VERY_LONG_PRESS_DURATION)
    {
        if (press_very_long_signal_done == false) {
            press_very_long_signal_done = true;
            buzzer_beep(3500, 600); // Мощный гудок прямо под пальцем на 4.1 сек!
        }
    }
}

