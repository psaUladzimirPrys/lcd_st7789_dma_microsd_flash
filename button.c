#ifndef __ALS_USER_BUTTON__
#define __ALS_USER_BUTTON__

#include "app_assert.h"
#include "app.h"
#include "app_log.h"

#include "sl_sleeptimer.h"
#include "sl_simple_button.h"
#include "sl_simple_button_instances.h"   // SL_SIMPLE_BUTTON_INSTANCE(i)
#include "sl_button.h"





// Временные пороги (мс)
#define DEBOUNCE_TIME_MS       (20)    // 20 мс типичный антидребезг кнопки
#define MULTI_PRESS_DURATION   (800)   // пауза между кликами
#define SHORT_PRESS_DURATION   (250)   // < 500 мс — короткое
#define LONG_PRESS_DURATION    (3000)  // < 3000 — длинное, >= 3000 — очень длинное

void button_feature_init(void);
void button_feature_process(void);   // вызывать из main()

// Callbacks пользователя (реализуете сами в другом файле)
void button_on_multi_click(uint8_t count); // 1..3 клика
void button_on_long_press(void);
void button_on_very_long_press(void);



// Для удобства
#define APP_BUTTON_HANDLE   (&sl_button_btn0)

typedef struct {
  uint32_t last_press_tick;
  uint32_t last_release_tick;
  uint32_t last_event_tick;   // для антидребезга
  uint8_t  presscount;
  uint8_t  waiting_multi;
} button_state_t;

static button_state_t btn;

// Слабые реализация колбэков
__attribute__((weak)) void button_on_multi_click_callback(uint8_t count)
{
  (void)count;
}

__attribute__((weak)) void button_on_long_press_callback(void)
{

}
__attribute__((weak)) void button_on_very_long_press_callback(void)
{

}

uint8_t get_button_count_valut(void)
{
  return btn.presscount;
}
void reset_button_count(void)
{
  btn.presscount = 0;
}

void button_feature_init(void)
{
  btn.last_press_tick   = 0;
  btn.last_release_tick = 0;
  btn.last_event_tick   = 0;
  btn.presscount        = 0;
  btn.waiting_multi     = 0;

  // Кнопка инициализируется автогенерированным кодом:
}

// Вызывается драйвером кнопки при каждом изменении
void sl_button_on_change(const sl_button_t * handle)
{
  if (handle != APP_BUTTON_HANDLE)
  {
    return;
  }

  uint32_t now = sl_sleeptimer_get_tick_count();

  // --- Антидребезг: игнорировать события, пришедшие слишком быстро ---
  if (btn.last_event_tick != 0)
  {
    uint32_t dt_deb = sl_sleeptimer_tick_to_ms(now - btn.last_event_tick);
    if (dt_deb < DEBOUNCE_TIME_MS)
    {
      // Считаем дребезгом — игнорируем это изменение
      return;
    }
  }
  btn.last_event_tick = now;
  // -------------------------------------------------------------------

  sl_button_state_t st = sl_button_get_state(handle);

  if (st == SL_SIMPLE_BUTTON_PRESSED)
  {
    // Нажатие
    btn.last_press_tick = now;
  }
  else if (st == SL_SIMPLE_BUTTON_RELEASED)
  {
    uint32_t dt = now - btn.last_press_tick;

    btn.last_release_tick = now;

    if (dt < sl_sleeptimer_ms_to_tick(SHORT_PRESS_DURATION))
    {
      // Короткое нажатие → multi‑click
      if (btn.presscount < 255)
      {
        btn.presscount++;
      }
      btn.waiting_multi = 1;
    }
    else if (dt < sl_sleeptimer_ms_to_tick(LONG_PRESS_DURATION))
    {
      button_on_long_press();
      btn.presscount    = 0;
      btn.waiting_multi = 0;
    }
    else
    {
      button_on_very_long_press();
      btn.presscount    = 0;
      btn.waiting_multi = 0;
    }
  }
}

// Вызывать регулярно в главном цикле
void button_feature_process(void)
{
  uint32_t now   = sl_sleeptimer_get_tick_count();
  uint32_t dt_ms = sl_sleeptimer_tick_to_ms(now - btn.last_release_tick);

  if (!btn.waiting_multi || btn.presscount == 0)
  {
    return;
  }

  if (dt_ms >= MULTI_PRESS_DURATION)
  {
    // Серия коротких нажатий закончилась
    uint8_t cnt = btn.presscount;
    if (cnt > 3)
    {
      cnt = 3;
    }

    button_on_multi_click(cnt);

    btn.presscount    = 0;
    btn.waiting_multi = 0;
  }
}

void button_on_multi_click(uint8_t count)
{
  uint8_t cnt;
  cnt = get_button_count_valut();
  app_log("press count %d times\r\n",cnt);


  // 1 клик → действие 1
  // 2 клика → действие 2
  // 3 клика → действие 3

  count = count;
}

void button_on_long_press(void)
{
  // длинное нажатие
  app_log("button long press\r\n");
}

void button_on_very_long_press(void)
{
  // очень длинное нажатие
  app_log("button very long press\r\n");
}

#endif
