#include "led.h"

// Хэндл для интеграции в системный планировщик SleepTimer
static sl_sleeptimer_timer_handle_t led_timer;

// Volatile переменные, так как они изменяются асинхронно в контексте прерывания
static volatile uint32_t blink_toggle_limit = 0;
static volatile bool is_one_shot = false;


static void led_timer_callback(sl_sleeptimer_timer_handle_t *handle, void *data)
{
  (void)handle;
  (void)data;

  // Обработка режима "зажечь на время" (One-shot)
  if (is_one_shot)
  {
    sl_sleeptimer_stop_timer(&led_timer);
    GPIO_PinOutClear(LED_PORT, LED_PIN); // Атомарное выключение светодиода
    is_one_shot = false;
    return;
  }

  // Обработка режима мигания (Toggle)
  GPIO_PinOutToggle(LED_PORT, LED_PIN);

  // Если мигание конечное (blink_toggle_limit > 0)
  if (blink_toggle_limit > 0)
  {
    blink_toggle_limit--;
    if (blink_toggle_limit == 0)
    {
      sl_sleeptimer_stop_timer(&led_timer);
      GPIO_PinOutClear(LED_PORT, LED_PIN); // Гарантированно тушим в конце цикла
    }
  }
}

void led_init(void)
{
  // 1. Включаем тактирование интерфейсной шины GPIO.
  // Без этого запись в конфигурационные регистры портов вызовет HardFault.
  CMU_ClockEnable(cmuClock_GPIO, true);

  // 2. Настраиваем пин PC06 на выход (Push-Pull).
  // Начальное состояние устанавливаем в 0 (светодиод выключен), чтобы избежать бросков тока при старте.
  GPIO_PinModeSet(LED_PORT, LED_PIN, gpioModePushPull, 0);
  app_log("LED: initialized\r\n");
}


void led_control(uint32_t interval_or_duration_ms, uint32_t count)
{
  // Входим в критическую секцию CMSIS (отключаем прерывания на уровне ядра Cortex-M33).
  // Это предотвращает Race Condition, если функция led_control будет вызвана из main
  // в момент выполнения старого прерывания led_timer_callback.
  __disable_irq();

  // Принудительно деактивируем все запущенные ранее таймеры для этого светодиода
  sl_sleeptimer_stop_timer(&led_timer);
  blink_toggle_limit = 0;
  is_one_shot = false;

  // Сценарий 1: Запрос на выключение (время равно 0)
  if (interval_or_duration_ms == 0)
  {
    GPIO_PinOutClear(LED_PORT, LED_PIN);
    __enable_irq(); // Выходим из критической секции
    return;
  }

  // Сценарий 2: Постоянное включение (время бесконечно, count равен 0)
  if (interval_or_duration_ms == LED_INFINITE && count == 0)
  {
    GPIO_PinOutSet(LED_PORT, LED_PIN);
    __enable_irq();
    return;
  }

  // Сценарий 3: Одиночное включение на заданное время (One-shot, count равен 0)
  if (count == 0)
  {
    is_one_shot = true;
    GPIO_PinOutSet(LED_PORT, LED_PIN); // Зажигаем мгновенно

    // Запускаем одиночный таймер. По истечении duration_ms вызовется коллбек и потушит LED.
    sl_sleeptimer_start_timer_ms(&led_timer,
                                 interval_or_duration_ms,
                                 led_timer_callback,
                                 NULL,
                                 0,
                                 0);
  }
  // Сценарий 4: Режим мигания (count > 0 или count == LED_INFINITE)
  else
  {
    // Если мигание конечное, количество переключений состояния (toggle) строго в 2 раза больше числа вспышек
    if (count != LED_INFINITE)
    {
      blink_toggle_limit = count * 2;
    }
    else
    {
      blink_toggle_limit = 0; // 0 означает бесконечный цикл в логике коллбека
    }

    // За период одной вспышки светодиод должен 1 раз включиться и 1 раз выключиться.
    // Поэтому делим интервал на 2, получая полупериод таймера.
    uint32_t half_period = interval_or_duration_ms / 2;
    if (half_period == 0)
    {
      half_period = 1; // Защита от деления на ноль, минимальный шаг SleepTimer (1 мс)
    }

    // Перезапускаем периодический аппаратный таймер на базе низкопотребляющего
    // Это позволяет чипу BGM220S спать в режиме EM2 (Deep Sleep) между вспышками.
    sl_sleeptimer_restart_periodic_timer_ms(&led_timer,
                                            half_period,
                                            led_timer_callback,
                                            NULL,
                                            0,
                                            0);
  }

  // Благополучно восстанавливаем прерывания процессора
  __enable_irq();
}




