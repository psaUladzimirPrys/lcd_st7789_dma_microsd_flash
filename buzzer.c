#include "buzzer.h"

static sl_sleeptimer_timer_handle_t buzzer_off_timer;

// Callback для автоматического выключения звука
static void buzzer_timeout_callback(sl_sleeptimer_timer_handle_t *handle, void *data)
{
  (void)handle;
  (void)data;
  // Просто выключаем таймер, чтобы прекратить генерацию ШИМ
  TIMER_Enable(BUZZER_TIMER, false);

  // РАЗРЕШАЕМ системе снова уходить в глубокий сон (EM2)
  sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);
}

/**
 * Инициализация аппаратной части: GPIO, Тактирование и Таймер
 */
void init_buzzer(void)
{
  // 1. Включаем тактирование GPIO и TIMER0
  // На Series 2 (BGM220) это обязательный шаг перед доступом к регистрам
  CMU_ClockEnable(cmuClock_GPIO, true);
  CMU_ClockEnable(cmuClock_TIMER0, true);

  // 2. Настраиваем PD02 на выход (Push-Pull)
  GPIO_PinModeSet(BUZZER_PORT, BUZZER_PIN, gpioModePushPull, 0);

  // 3. Базовая инициализация таймера
  TIMER_Init_TypeDef timerInit = TIMER_INIT_DEFAULT;
  timerInit.prescale = BUZZER_TIMER_PRESCALER; // Делитель частоты
  timerInit.enable = false;              // Не запускаем сразу
  TIMER_Init(BUZZER_TIMER, &timerInit);

  // 4. Настройка канала CC0 для режима PWM
  TIMER_InitCC_TypeDef timerCCInit = TIMER_INITCC_DEFAULT;
  timerCCInit.mode = timerCCModePWM;
  TIMER_InitCC(BUZZER_TIMER, 0, &timerCCInit);

  // 5. Маршрутизация выхода таймера на пин PD02 (через DBUS)
  // В SDK 2025.x для Series 2 используется массив TIMERROUTE
  GPIO->TIMERROUTE[0].ROUTEEN = GPIO_TIMER_ROUTEEN_CC0PEN;
  GPIO->TIMERROUTE[0].CC0ROUTE = (BUZZER_PORT << _GPIO_TIMER_CC0ROUTE_PORT_SHIFT)
                                  | (BUZZER_PIN << _GPIO_TIMER_CC0ROUTE_PIN_SHIFT);
  app_log("BUZZER: initialized\r\n");
}

/**
 * Динамическая установка частоты и запуск звука
 * @param freq_hz - частота в Герцах (для SMT-0540-S-2-R резонанс = 4000)
 * @param duration_ms - длительность сигнала в мс
 */
void buzzer_beep(uint32_t freq_hz, uint32_t duration_ms)
{
  if (freq_hz == 0) return;

  init_buzzer();

  sl_sleeptimer_stop_timer(&buzzer_off_timer);
  // ЗАПРЕЩАЕМ системе уходить глубже чем EM1, пока мы пищим
  // Это гарантирует, что периферия TIMER0 не выключится
  sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);

  // ПРИНУДИТЕЛЬНО возвращаем контроль над PD02 таймеру перед каждым писком
  GPIO->TIMERROUTE[0].ROUTEEN = 0;
  GPIO->TIMERROUTE[0].CC0ROUTE = (BUZZER_PORT << _GPIO_TIMER_CC0ROUTE_PORT_SHIFT) | (BUZZER_PIN << _GPIO_TIMER_CC0ROUTE_PIN_SHIFT);
  GPIO->TIMERROUTE[0].ROUTEEN = GPIO_TIMER_ROUTEEN_CC0PEN;

  // Получаем текущую частоту шины периферии (обычно 38.4 МГц)
  uint32_t timer_clk = CMU_ClockFreqGet(cmuClock_TIMER0);

  // Динамический расчет значения TOP (период)
  // TOP = (F_timer / (Prescaler * F_out)) - 1
  uint32_t top = (timer_clk / (BUZZER_PRESCALER_DIV * freq_hz)) - 1;

  //app_log("Buzzer: F_clk=%u, TOP=%u\r\n", (unsigned int)timer_clk, (unsigned int)top);

  // Ограничение для 16-битного таймера (макс 65535)
  if (top > 0xFFFF) top = 0xFFFF;

  // Устанавливаем период (частоту)
  TIMER_TopSet(BUZZER_TIMER, top);

  // Устанавливаем Duty Cycle 50% (громкость)
  // Compare = TOP / 2
  TIMER_CompareSet(BUZZER_TIMER, 0, top / 2);

  // Запуск генерации
  TIMER_Enable(BUZZER_TIMER, true);

  // Запуск таймера на выключение через Sleeptimer (не блокирует CPU)
  sl_sleeptimer_start_timer_ms(&buzzer_off_timer,
                               duration_ms,
                               buzzer_timeout_callback,
                               NULL,
                               0,
                               0);
}
