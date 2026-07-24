#include "command_line.h"

// Добавляем объявление хендла, так как в .h его нет
extern sl_cli_handle_t sl_cli_cli_inst_handle;
// параметры рабооты устройства
extern device_params_t device_params;

// Обработчик команд
// Сигнатура должна строго соответствовать sl_cli_command_func_t
void cli_cmd_info(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  app_log_info("BGM220PC22: Cortex-M33 | SDK 2025.6.2\r\n");
}

// Уникальный ID чипа (EUI64)
void cli_cmd_uid(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  uint64_t unic_id = SYSTEM_GetUnique();
  // PRIX32 автоматически подставит нужный символ (X или lX) в зависимости от платформы
  app_log_info("EUI64: 0x%08" PRIX32 "%08" PRIX32 "\r\n", (uint32_t)(unic_id >> 32), (uint32_t)unic_id);
}

void cli_cmd_cls(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  // ANSI-последовательность для очистки экрана и возврата курсора в начало
  app_log("\033[2J\033[H");
}

// Программный сброс
void cli_cmd_reboot(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  app_log_warning("Rebooting system...\r\n");
  NVIC_SystemReset();
}

void cli_cmd_clocks(sl_cli_command_arg_t *arguments)
{
  (void)arguments;

  // 1. Высокочастотные шины (тут всё стандартно)
  uint32_t hclk   = CMU_ClockFreqGet(cmuClock_HCLK);
  uint32_t pclk   = CMU_ClockFreqGet(cmuClock_PCLK);

  // 2. Корневые источники низких частот (LF)
  // В BGM220 периферия (RTCC, WDOG, LETIMER) питается от этих веток:
  uint32_t rtcc  = CMU_ClockFreqGet(cmuClock_RTCC);  // Частота для RTCC (основа Sleeptimer)
  uint32_t wdog0 = CMU_ClockFreqGet(cmuClock_WDOG0); // Частота для Watchdog

  app_log_info("HCLK: %u Hz\r\n", (unsigned int)hclk);
  app_log_info("PCLK: %u Hz\r\n", (unsigned int)pclk);
  app_log_info("RTCC: %u Hz\r\n", (unsigned int)rtcc);
  app_log_info("WDOG0: %u Hz\r\n", (unsigned int)wdog0);
  // Опционально: узнать, какой генератор сейчас главный для SYSCLK
  CMU_Select_TypeDef source = CMU_ClockSelectGet(cmuClock_SYSCLK);
  switch(source)
  {
    case 1:
      app_log_info("Active Clock Source: HFXO\r\n");
    break;

    case 2:
      app_log_info("Active Clock Source: HFRCO\r\n");
    break;

    case 3:
      app_log_info("Active Clock Source: FSRCO\r\n");
    break;

    case 4:
      app_log_info("Active Clock Source: CLKIN0\r\n");
    break;

    case 5:
      app_log_info("Active Clock Source: DPLL0\r\n");
    break;

    default:
      app_log_info("Active Clock Source ID: %d\r\n", (int)source);
    break;
  }

}

void cli_cmd_buzzer(sl_cli_command_arg_t *arguments)
{
  // Извлекаем аргументы по индексам (индексация начинается с 0 для параметров)
  // sl_cli_get_argument_uint32 возвращает значение аргумента
  uint32_t freq = sl_cli_get_argument_uint32(arguments, 0);
  uint32_t duration = sl_cli_get_argument_uint32(arguments, 1);

  if (freq == 0 || duration == 0) {
    app_log_error("CLI: Invalid parameters. Usage: buzzer <freq> <duration>\r\n");
    return;
  }

  //app_log_info("CLI: Beep %u Hz for %u ms\r\n", (unsigned int)freq, (unsigned int)duration);

  // Вызываем твою функцию из модуля бузера
  buzzer_beep(freq, duration);
}

void cli_cmd_led(sl_cli_command_arg_t *arguments)
{
  // Извлекаем аргументы: 0 - интервал (полный период), 1 - количество вспышек
  uint32_t interval = sl_cli_get_argument_uint32(arguments, 0);
  uint32_t count = sl_cli_get_argument_uint32(arguments, 1);

  if (interval == 0) {
    app_log_error("CLI: Invalid interval. Usage: led <interval_ms> <count>\r\n");
    return;
  }

  // Если count = 0, светодиод будет мигать бесконечно (согласно логике led_blink)
  if (count == 0) {
    app_log_info("CLI: LED blinking indefinitely (interval: %u ms)\r\n", (unsigned int)interval);
  } else {
    app_log_info("CLI: LED blinking %u times (period: %u ms)\r\n", (unsigned int)count, (unsigned int)interval);
  }

  // Вызов функции из модуля led.c
  led_control(interval, count);
}

void cli_cmd_stpm(sl_cli_command_arg_t *arguments)
{
  (void)arguments; // Избавляемся от warning о неиспользуемом параметре

  app_log_info("CLI: Starting patient measurement...\r\n");

  // Вызов целевой функции из adc.c
  start_patient_session();
}

void cli_cmd_stpc(sl_cli_command_arg_t *arguments)
{
  (void)arguments; // Избавляемся от warning о неиспользуемом параметре

  app_log_info("CLI: Starting performance check...\r\n");

  // Вызов целевой функции из adc.c
  start_performance_session();
}

void cli_cmd_vtip(sl_cli_command_arg_t *arguments)
{
  // Получаем указатель на строку-аргумент (индекс 0)
  char *tip_id_arg = sl_cli_get_argument_string(arguments, 0);

  if (tip_id_arg == NULL) {
    app_log_error("CLI: Missing Tip ID. Usage: vtip <5-char-id>\r\n");
    return;
  }

  // Проверяем длину (должна быть строго равна TIP_ID_LEN, то есть 5)
  size_t arg_len = strlen(tip_id_arg);
  if (arg_len != TIP_ID_LEN) {
    app_log_error("CLI: Invalid Tip ID length (%d). Must be exactly 5 chars.\r\n", (int)arg_len);
    return;
  }

  app_log_info("CLI: Validating Tip ID: %s...\r\n", tip_id_arg);

  // Вызываем функцию верификации (передаем ID)

  tip_validate((const uint8_t *)tip_id_arg);
}


// Команда для просмотра абсолютно всех текущих параметров устройства
void cli_cmd_get_all(sl_cli_command_arg_t *arguments)
{
  (void)arguments;

  app_log_info("================ CURRENT PARAMS ================\r\n");

  // 1. Чтение Advanced Params
  app_log_info("--- Advanced Params ---\r\n");
  app_log_info("  Serial Number: %u\r\n", (unsigned int)device_params.advanced_params.serial_number);
  app_log_info("  Debug Output:  %s\r\n", device_params.advanced_params.debug_output_on ? "ON" : "OFF");

  // 2. Чтение Measure Params (внутри advanced)
  app_log_info("--------- Measure Params ---------\r\n");
  app_log_info("  period:    %u us\r\n", device_params.advanced_params.measure_params.adc_taking_period);
  //app_log_info("  num:       %u\r\n",    device_params.advanced_params.measure_params.max_num_of_measurments);
  //app_log_info("  val_ctrl:  %u\r\n",    device_params.advanced_params.measure_params.num_of_valid_control_measurments);
  //app_log_info("  val_ref:   %u\r\n",    device_params.advanced_params.measure_params.num_of_valid_refernce_measurments);
  app_log_info("  sig_min:   %u\r\n",    device_params.advanced_params.measure_params.signal_min);
  app_log_info("  sig_max:   %u\r\n",    device_params.advanced_params.measure_params.signal_max);
  app_log_info("  amp_min:   %u\r\n",    device_params.advanced_params.measure_params.impact_amp_min);
  app_log_info("  amp_max:   %u\r\n",    device_params.advanced_params.measure_params.impact_amp_max);
  app_log_info("  time_min:  %u\r\n",    device_params.advanced_params.measure_params.impact_time_min);
  app_log_info("  time_max:  %u\r\n",    device_params.advanced_params.measure_params.impact_time_max);

  // 3. Чтение General Params
  app_log_info("--------- General Params ---------\r\n");
  app_log_info("  screen off timeout:      %u s\r\n",  device_params.general_params.scree_off_timeout);
  app_log_info("  power save mode timeout: %u s\r\n",  device_params.general_params.power_saving_mode_timeout);
  app_log_info("  screnn brightness:       %u %%\r\n", device_params.general_params.brightness_level);
  app_log_info("  battery warning level:   %u %%\r\n", device_params.general_params.battery_threshold_warning);
  app_log_info("  battery critical level:  %u %%\r\n", device_params.general_params.battery_threshold_critical);
  app_log_info("  calibration constant:    %.2f\r\n",  device_params.general_params.calibration_constant);
}

// Команда установки серийника и дебага: set_adv <serial> <debug_on (0 или 1)>
void cli_cmd_set_adv(sl_cli_command_arg_t *arguments)
{
  uint32_t serial = sl_cli_get_argument_uint32(arguments, 0);
  uint8_t debug   = sl_cli_get_argument_uint8(arguments, 1);

  device_params.advanced_params.serial_number = serial;
  device_params.advanced_params.debug_output_on = (debug != 0);

  app_log_info("CLI: Advanced parameters updated.\r\n");
}

// Команда изменения конкретного параметра измерения: set_meas <имя_параметра> <значение>
void cli_cmd_set_adv_params(sl_cli_command_arg_t *arguments)
{
  char *param_name = sl_cli_get_argument_string(arguments, 0);
  uint32_t value   = sl_cli_get_argument_uint32(arguments, 1);

  if (param_name == NULL) {
    app_log_error("CLI: Usage: set_adv_param <param_name> <value>\r\n");
    return;
  }

  // Из-за того что внутренняя структура в params.h анонимная (без typedef имени),
  // мы модифицируем поля через полный путь к ним.
  if (strcmp(param_name, "adc_taking_period") == 0) {
    device_params.advanced_params.measure_params.adc_taking_period = (uint16_t)value;
  } /*else if (strcmp(param_name, "serial_number") == 0) {
    device_params.advanced_params.measure_params.max_num_of_measurments = (uint8_t)value;
  } else if (strcmp(param_name, "val_ctrl") == 0) {
    device_params.advanced_params.measure_params.num_of_valid_control_measurments = (uint8_t)value;
  } else if (strcmp(param_name, "val_ref") == 0) {
    device_params.advanced_params.measure_params.num_of_valid_refernce_measurments = (uint8_t)value;
  }*/ else if (strcmp(param_name, "sig_min") == 0) {
    device_params.advanced_params.measure_params.signal_min = (uint16_t)value;
  } else if (strcmp(param_name, "sig_max") == 0) {
    device_params.advanced_params.measure_params.signal_max = (uint16_t)value;
  } else if (strcmp(param_name, "amp_min") == 0) {
    device_params.advanced_params.measure_params.impact_amp_min = (uint16_t)value;
  } else if (strcmp(param_name, "amp_max") == 0) {
    device_params.advanced_params.measure_params.impact_amp_max = (uint16_t)value;
  } else if (strcmp(param_name, "time_min") == 0) {
    device_params.advanced_params.measure_params.impact_time_min = (uint16_t)value;
  } else if (strcmp(param_name, "time_max") == 0) {
    device_params.advanced_params.measure_params.impact_time_max = (uint16_t)value;
  } else {
    app_log_error("CLI: Unknown parameter '%s'.\r\n", param_name);
    return;
  }
  app_log_info("CLI: Parameter '%s' successfully set to %u\r\n", param_name, (unsigned int)value);
}

// Команда изменения конкретного общего параметра: set_gen_param <имя_параметра> <значение>
void cli_cmd_set_gen_params(sl_cli_command_arg_t *arguments)
{
  char *param_name = sl_cli_get_argument_string(arguments, 0);

  if (param_name == NULL) {
    app_log_error("CLI: Usage: set_gen_param <param_name> <value>\r\n");
    return;
  }

  // Для работы с float (calibration_constant) извлечем аргумент и как число, и как строку
  uint32_t val_int = sl_cli_get_argument_uint32(arguments, 1);
  char *val_str    = sl_cli_get_argument_string(arguments, 1);

  if (strcmp(param_name, "screen_off") == 0) {
    if ((val_int >= SCREEN_OFF_TIMEOUT_MIN) && (val_int <= SCREEN_OFF_TIMEOUT_MAX)) {
      device_params.general_params.scree_off_timeout = (uint16_t)val_int;
      app_log_info("CLI: screen_off_timeout set to %u\r\n", (unsigned int)val_int);
    } else {
      app_log_error("CLI: Out of range (%u..%u)\r\n", SCREEN_OFF_TIMEOUT_MIN, SCREEN_OFF_TIMEOUT_MAX);
    }
  }
  else if (strcmp(param_name, "power_save") == 0) {
    if ((val_int >= POWER_SAVING_MODE_TIMEOUT_MIN) && (val_int <= POWER_SAVING_MODE_TIMEOUT_MAX)) {
      device_params.general_params.power_saving_mode_timeout = (uint16_t)val_int;
      app_log_info("CLI: power_saving_mode_timeout set to %u\r\n", (unsigned int)val_int);
    } else {
      app_log_error("CLI: Out of range (%u..%u)\r\n", POWER_SAVING_MODE_TIMEOUT_MIN, POWER_SAVING_MODE_TIMEOUT_MAX);
    }
  }
  else if (strcmp(param_name, "brightness") == 0) {
    if ((val_int >= BRIGHTNESS_LEVEL_MIN) && (val_int <= BRIGHTNESS_LEVEL_MAX)) {
      device_params.general_params.brightness_level = (uint8_t)val_int;
      app_log_info("CLI: brightness_level set to %u\r\n", (unsigned int)val_int);
    } else {
      app_log_error("CLI: Out of range (%u..%u)\r\n", BRIGHTNESS_LEVEL_MIN, BRIGHTNESS_LEVEL_MAX);
    }
  }
  else if (strcmp(param_name, "bat_warn") == 0) {
    if ((val_int >= BATTERY_THRESHOLD_WARNING_MIN) && (val_int <= BATTERY_THRESHOLD_WARNING_MAX)) {
      device_params.general_params.battery_threshold_warning = (uint8_t)val_int;
      app_log_info("CLI: battery_threshold_warning set to %u\r\n", (unsigned int)val_int);
    } else {
      app_log_error("CLI: Out of range (%u..%u)\r\n", BATTERY_THRESHOLD_WARNING_MIN, BATTERY_THRESHOLD_WARNING_MAX);
    }
  }
  else if (strcmp(param_name, "bat_crit") == 0) {
    if ((val_int >= BATTERY_THRESHOLD_CRITICAL_MIN) && (val_int <= BATTERY_THRESHOLD_CRITICAL_MAX)) {
      device_params.general_params.battery_threshold_critical = (uint8_t)val_int;
      app_log_info("CLI: battery_threshold_critical set to %u\r\n", (unsigned int)val_int);
    } else {
      app_log_error("CLI: Out of range (%u..%u)\r\n", BATTERY_THRESHOLD_CRITICAL_MIN, BATTERY_THRESHOLD_CRITICAL_MAX);
    }
  }
  else if (strcmp(param_name, "calib") == 0) {
    // Парсим float из строки (например, "1.25")
    float val_float = 0.0f;
    if (val_str != NULL && sscanf(val_str, "%f", &val_float) == 1) {
      // Переводим в формат протокола для проверки диапазона (умножаем на 100)
      uint32_t check_val = (uint32_t)(val_float * 100.0f + 0.5f);
      if ((check_val >= CALIBRATION_CONSTANT_MIN) && (check_val <= CALIBRATION_CONSTANT_MAX)) {
        device_params.general_params.calibration_constant = val_float;
        app_log_info("CLI: calibration_constant set to %.2f\r\n", val_float);
      } else {
        app_log_error("CLI: Calibration out of range (0.01..9.99)\r\n");
      }
    } else {
      app_log_error("CLI: Invalid float format for calibration\r\n");
    }
  }
  else {
    app_log_error("CLI: Unknown general parameter '%s'\r\n", param_name);
  }
}



// --- 2. Определение команд через макросы SDK ---
static const sl_cli_command_info_t cmd_info_desc   = SL_CLI_COMMAND(cli_cmd_info,   "Show board info", "", {SL_CLI_ARG_END}); // parameters: function,info,arguments, end of arguments list
static const sl_cli_command_info_t cmd_uid_desc    = SL_CLI_COMMAND(cli_cmd_uid,    "Get Unique Device ID", "", {SL_CLI_ARG_END});// parameters: function,info,arguments, end of arguments list
static const sl_cli_command_info_t cmd_reboot_desc = SL_CLI_COMMAND(cli_cmd_reboot, "Software Reset", "", {SL_CLI_ARG_END});// parameters: function,info,arguments, end of arguments list
static const sl_cli_command_info_t cmd_cls_desc    = SL_CLI_COMMAND(cli_cmd_cls,    "Clear terminal screen", "", {SL_CLI_ARG_END});
static const sl_cli_command_info_t cmd_clocks_desc = SL_CLI_COMMAND(cli_cmd_clocks, "Show system clock frequencies", "", {SL_CLI_ARG_END});
static const sl_cli_command_info_t cmd_buzzer_desc = SL_CLI_COMMAND(cli_cmd_buzzer, "Play sound on buzzer", "Frequency in Hz" SL_CLI_UNIT_SEPARATOR "Duration in ms",
                                                                    { SL_CLI_ARG_UINT32, SL_CLI_ARG_UINT32, SL_CLI_ARG_END } );
static const sl_cli_command_info_t cmd_led_desc    = SL_CLI_COMMAND(cli_cmd_led,    "Control LED blinking", "Interval in ms between flash" SL_CLI_UNIT_SEPARATOR "Count (0 for infinite)",
                                                                    { SL_CLI_ARG_UINT32, SL_CLI_ARG_UINT32, SL_CLI_ARG_END } );
static const sl_cli_command_info_t cmd_stpm_desc   = SL_CLI_COMMAND(cli_cmd_stpm,   "Start patient measurement session", "", {SL_CLI_ARG_END});
static const sl_cli_command_info_t cmd_stpc_desc   = SL_CLI_COMMAND(cli_cmd_stpc,   "Start performance check session", "", {SL_CLI_ARG_END});
static const sl_cli_command_info_t cmd_vtip_desc   = SL_CLI_COMMAND(cli_cmd_vtip,   "Validate Tip ID", "5-character Base36 ID", {SL_CLI_ARG_STRING, SL_CLI_ARG_END});
static const sl_cli_command_info_t cmd_get_adv_desc  = SL_CLI_COMMAND(cli_cmd_get_all, "Show advanced & measure parameters", "", {SL_CLI_ARG_END});

static const sl_cli_command_info_t cmd_set_adv_desc  = SL_CLI_COMMAND(
    cli_cmd_set_adv,
    "Set serial number and debug state",
    "Serial number" SL_CLI_UNIT_SEPARATOR "Debug mode (1=ON, 0=OFF)",
    {SL_CLI_ARG_UINT32, SL_CLI_ARG_UINT8, SL_CLI_ARG_END}
);

static const sl_cli_command_info_t cmd_set_adv_params_desc = SL_CLI_COMMAND(
    cli_cmd_set_adv_params,
    "Set specific measurement parameter",
    "Name (adc_taking_period, serial_number, val_ctrl, val_ref, sig_min, sig_max, amp_min, amp_max, time_min, time_max)" SL_CLI_UNIT_SEPARATOR "Value",
    {SL_CLI_ARG_STRING, SL_CLI_ARG_UINT32, SL_CLI_ARG_END}
);

static const sl_cli_command_info_t cmd_set_gen_params_desc = SL_CLI_COMMAND(
    cli_cmd_set_gen_params,
    "Set specific general parameter",
    "Name (screen_off, power_save, brightness, bat_warn, bat_crit, calib)" SL_CLI_UNIT_SEPARATOR "Value",
    {SL_CLI_ARG_STRING, SL_CLI_ARG_STRING, SL_CLI_ARG_END}
    // Используем SL_CLI_ARG_STRING для второго аргумента, чтобы мы могли передавать как целые, так и float-строки
);

// commands table
static const sl_cli_command_entry_t device_commands_table[] =
{
  { "info",           &cmd_info_desc,   false },
  { "uid",            &cmd_uid_desc,    false },
  { "reboot",         &cmd_reboot_desc, false },
  { "cls",            &cmd_cls_desc,    false },
  { "clocks",         &cmd_clocks_desc, false },
  { "buzzer",         &cmd_buzzer_desc, false },
  { "led",            &cmd_led_desc,    false },
  { "stpm",           &cmd_stpm_desc,   false },
  { "stpc",           &cmd_stpc_desc,   false },
  { "vtip",           &cmd_vtip_desc,   false },
  { "get_all_params", &cmd_get_adv_desc,    false },
  { "set_adv",        &cmd_set_adv_desc,    false },
  { "set_adv_params", &cmd_set_adv_params_desc,   false },
  { "set_gen_param",  &cmd_set_gen_params_desc,   false },

  { NULL,     NULL,             false },


};
// --- 4. Группа команд ---
static sl_cli_command_group_t device_commands_group = { { NULL }, false, device_commands_table};

/**
 * Регистрация в рантайме
 */
void app_cli_setup(void)
{
  static bool commands_registered = false; // Статическая переменная, чтобы выполнить регистрацию только один раз

  if (commands_registered)
  {
    return;
  }

  sl_cli_handle_t target_handle = (sl_cli_cli_inst_handle != NULL) ? sl_cli_cli_inst_handle : sl_cli_default_handle;

  if (target_handle != NULL)
  {
    sl_status_t status = sl_cli_command_add_command_group(target_handle, &device_commands_group);

    if (status == SL_STATUS_OK)
    {
      app_log_info("CLI: Custom commands registered.Type 'info' to check.\r\n");
      commands_registered = true;
    }
    else if (status == SL_STATUS_ALREADY_EXISTS)
    {
      // Это специфический статус, если группа уже там
      app_log_warning("CLI: Commands already registered.Type 'info' to check.\r\n");
      commands_registered = true;
    }
    else
    {
      if(status != 1)
        {
            app_log_error("CLI: CLI Registration failed: 0x%04X but it's OK\r\n", (unsigned int)status);
        }
    }
    app_log("CLI: initialized\r\n");
  }
}
