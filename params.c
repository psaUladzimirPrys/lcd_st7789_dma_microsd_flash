/*==========================================================================*/
/*        I N C L U D E S                                                   */
/*==========================================================================*/
#include "params.h"
#include "fsrv.h"
#include "battery_management.h"
#include "ble_communication.h"

/*===========================================================================*/
/*         G L O B A L   D A T A   D E C L A R A T I O N S                   */
/*===========================================================================*/
extern device_operation_control_t   device_operation_control;

device_params_t device_params;

/*=======================================================================*/
/*                                                                       */
/*=======================================================================*/
/* general parameters */
/*==========================================================================*/
/* G L O B A L      F U N C T I O N S                                       */
/*==========================================================================*/
// set default general parameters
void set_default_general_params(void)
{
  device_params.general_params.brightness_level = 80;
  device_params.general_params.power_saving_mode_timeout = 120;
  device_params.general_params.scree_off_timeout = 60;
  device_params.general_params.battery_threshold_warning = 25;
  device_params.general_params.battery_threshold_critical = 20;
  device_params.general_params.calibration_constant = 1.21;
}

/*=======================================================================*/
/*                                                                       */
/*=======================================================================*/
// get parameters from ovs station
void get_general_params(const uint8_t *in_ptr)
{
  uint32_t temp_val = 0;

  if(in_ptr == NULL) return;

  // screen off timeout
  if(!ascii_to_uint(in_ptr, SCREEN_OFF_TIMEOUT_LEN, &temp_val)) return;
  if((temp_val >= SCREEN_OFF_TIMEOUT_MIN) && (temp_val <= SCREEN_OFF_TIMEOUT_MAX))
  {
      device_params.general_params.scree_off_timeout = (uint16_t)temp_val;
  }
  in_ptr += SCREEN_OFF_TIMEOUT_LEN;

  // power sawing mode time out
  temp_val = 0;
  if(!ascii_to_uint(in_ptr, POWER_SAVING_MODE_TIMEOUT_LEN, &temp_val)) return;
  if((temp_val >= POWER_SAVING_MODE_TIMEOUT_MIN) && (temp_val <= POWER_SAVING_MODE_TIMEOUT_MAX))
  {
      device_params.general_params.power_saving_mode_timeout = (uint16_t)temp_val;
  }
  in_ptr += POWER_SAVING_MODE_TIMEOUT_LEN;

  // calibration constant
  temp_val = 0;
  if(!ascii_to_uint(in_ptr, CALIBRATION_CONSTAN_LEN, &temp_val)) return;
  if((temp_val > CALIBRATION_CONSTANT_MAX) ||(temp_val == CALIBRATION_CONSTANT_MIN - 1))
  {
      app_log("PROTOCOL: GETP calibration constant out of range %u\r\n", (uint16_t)temp_val);
  }
  if((temp_val >= CALIBRATION_CONSTANT_MIN) && (temp_val <= CALIBRATION_CONSTANT_MAX))
  {
      device_params.general_params.calibration_constant = (float)temp_val/100.0f;
      app_log("ROTOCOL: GETP calculated calibration constant %.2f\r\n",device_params.general_params.calibration_constant);
  }
  in_ptr += CALIBRATION_CONSTAN_LEN;

  // brightness level
  temp_val = 0;
  if(!ascii_to_uint(in_ptr, BRIGHTNESS_LEVEL_LEN, &temp_val)) return;
  if(temp_val <= BRIGHTNESS_LEVEL_MAX)
  {
      device_params.general_params.brightness_level = (uint8_t)temp_val;
  }
  in_ptr += BRIGHTNESS_LEVEL_LEN;

  // battery threshold warning
  temp_val = 0;
  if(!ascii_to_uint(in_ptr, BATTERY_THRESHOLD_WARNING_LEN, &temp_val)) return;
  if(temp_val <= BATTERY_THRESHOLD_WARNING_MAX)
  {
      device_params.general_params.battery_threshold_warning = (uint8_t)temp_val;
  }
  in_ptr += BATTERY_THRESHOLD_WARNING_LEN;

  // battery threshold critical
  temp_val = 0;
  if(!ascii_to_uint(in_ptr, BATTERY_THRESHOLD_CRITICAL_LEN, &temp_val)) return;
  if(temp_val <= BATTERY_THRESHOLD_CRITICAL_MAX)
  {
      device_params.general_params.battery_threshold_critical = (uint8_t)temp_val;
  }
}

/*=======================================================================*/
/*                                                                       */
/*=======================================================================*/
// prepare parameters in buffer for ovs station
uint16_t prepare_general_params_for_station(uint8_t *out_buf)
{
    // Защита "от дурака"
    if (out_buf == NULL) return 0;

    uint8_t *p = out_buf;

    // 1. Код ответа на команду GETP
    *p++ = 'P';

    // 2. Screen off timeout
    if (!uint_to_ascii(device_params.general_params.scree_off_timeout, SCREEN_OFF_TIMEOUT_LEN, p)) {
        return 0;
    }
    p += SCREEN_OFF_TIMEOUT_LEN;

    // 3. Power saving timeout
    if (!uint_to_ascii(device_params.general_params.power_saving_mode_timeout, POWER_SAVING_MODE_TIMEOUT_LEN, p)) {
        return 0;
    }
    p += POWER_SAVING_MODE_TIMEOUT_LEN;

    // 4. Calibration constant
    // Оптимизация float-математики под ядро Cortex-M33 (используем аппаратный FPU)
    uint32_t calib = (uint32_t)(device_params.general_params.calibration_constant * 100.0f + 0.5f);
    if (!uint_to_ascii(calib, CALIBRATION_CONSTAN_LEN, p)) {
        return 0;
    }
    p += CALIBRATION_CONSTAN_LEN;

    // 5. Brightness
    if (!uint_to_ascii(device_params.general_params.brightness_level, BRIGHTNESS_LEVEL_LEN, p)) {
        return 0;
    }
    p += BRIGHTNESS_LEVEL_LEN;

    // 6. Battery warning
    if (!uint_to_ascii(device_params.general_params.battery_threshold_warning, BATTERY_THRESHOLD_WARNING_LEN, p)) {
        return 0;
    }
    p += BATTERY_THRESHOLD_WARNING_LEN;

    // 7. Battery critical
    if (!uint_to_ascii(device_params.general_params.battery_threshold_critical, BATTERY_THRESHOLD_CRITICAL_LEN, p)) {
        return 0;
    }
    /* ИСПРАВЛЕНО: Обязательно сдвигаем указатель для корректного подсчета финальной длины */
    p += BATTERY_THRESHOLD_CRITICAL_LEN;

    /* Вычисляем итоговую длину через арифметику указателей.
     * Это быстрее и детерминированнее, чем инкрементировать отдельный счетчик len на каждом шаге. */
    return (uint16_t)(p - out_buf);
}

/*=======================================================================*/
/*                                                                       */
/*=======================================================================*/
// send general paramemters to ovs station
void send_general_params_to_ovs_station(uint8_t *ptr)
{
  set_num_of_bytes_to_transmit(prepare_general_params_for_station(ptr));
  transmit_data_via_indication();
}

/*=======================================================================*/
/*                                                                       */
/*=======================================================================*/
uint16_t prepare_device_info_for_station(uint8_t *out_buf)
{
    // Защита "от дурака"
    if (out_buf == NULL) return 0;

    uint8_t *p = out_buf;

    // 1. Код ответа на команду запроса информации (0x49 = 'I')
    *p++ = 'I';

    // 2. Serial number: 10 bytes – format XXXXXXXXXX
    if (!uint_to_ascii(device_params.advanced_params.serial_number, 10, p)) {
        return 0;
    }
    p += 10;

    // 3. SW Version: 5 bytes – format XX.XX.X
    const char *ver_src = FSRV_FIRMWARE_VERSION;
    if (strlen(ver_src) >= 8) {
        // Извлекаем только значащие цифры из "000.03.6" -> "00036" (ровно 5 байт)
        p[0] = ver_src[1]; // '0'
        p[1] = ver_src[2]; // '0'
        p[2] = ver_src[4]; // '0'
        p[3] = ver_src[5]; // '3'
        p[4] = ver_src[7]; // '6'
    } else {
        strncpy((char *)p, ver_src, 5);
    }
    p += 5;

    // 4. Cal constant: 4 bytes – format X.XXX (например, 1.210)
    uint32_t cal_raw = (uint32_t)(device_params.general_params.calibration_constant * 1000.0f + 0.5f);
    if (!uint_to_ascii(cal_raw / 1000, 1, p)) return 0;
    p += 1;
    *p++ = '.';
    if (!uint_to_ascii(cal_raw % 1000, 3, p)) return 0;
    p += 3;

    // 5. Ref number: 5 bytes - format XXXXX
    if (!uint_to_ascii(device_params.advanced_params.reference_number, 5, p)) {
        return 0;
    }
    p += 5;

    // 6. Battery life: 3 bytes – format XXX%
    uint8_t batt_pct = get_battery_percentage();
    if (batt_pct == 0xFF) {
        batt_pct = 100; // Если работаем от USB без батареи
    } else if (batt_pct > 100) {
        batt_pct = 100;
    }
    if (!uint_to_ascii(batt_pct, 3, p)) {
        return 0;
    }
    p += 3;

    // 7. Battery charging status – 1 byte: "1"/"0"
    *p++ = is_battery_charging() ? '1' : '0';

    // 8. Strain gauge voltage: 3 bytes - format XXX (Пересчет 12-бит АЦП в вольты)
    uint32_t raw_gauge = device_operation_control.battary.starain_gauge_value;

    // Перевод АЦП (0-4095) в сотые доли вольта (опора 1.21В) с математическим округлением
    uint32_t gauge_val = (raw_gauge * 121 + 2047) / 4095;

    // Ограничиваем максимальное значение, чтобы гарантировать ровно 3 цифры (макс 999)
    if (gauge_val > 999) gauge_val = 999;

    // Форматируем строго в 3 ASCII символа (если будет 95, запишется как "095")
    if (!uint_to_ascii(gauge_val, 3, p)) {
        return 0;
    }
    p += 3;

    // 9. Strain gauge status – 1 byte: '1'/'0' (Good/Bad)
    *p++ = (device_operation_control.battary.starain_gauge_status != 0) ? '1' : '0';

    /* Возвращаем итоговый размер пакета данных (34 байта) */
    return (uint16_t)(p - out_buf);
}

/*=======================================================================*/
/*                                                                       */
/*=======================================================================*/
// send device info to ovs station
void send_device_info_to_ovs_station(uint8_t *ptr)
{
    set_num_of_bytes_to_transmit(prepare_device_info_for_station(ptr));
    transmit_data_via_indication();
}

/*=======================================================================*/
/*                                                                       */
/*=======================================================================*/
/* advanced parameters */

void set_default_advanced_params(void)
{
  device_params.advanced_params.serial_number = 130;
  device_params.advanced_params.reference_number = 31;
  device_params.advanced_params.measure_params.adc_taking_period = DEFAULT_ADC_SAMPLING_TIME;
  device_params.advanced_params.measure_params.adc_vref_source = 0;

  device_params.advanced_params.measure_params.max_num_of_patient_measurements = DEFAUL_MAX_NUM_OF_PERFORMANCE_MEASURE_IDENTS;//DEFAULT_TOTAL_MEASUREMENTS;
  device_params.advanced_params.measure_params.max_num_of_patient_references = DEFAUL_MAX_NUM_OF_PERFORMANCE_MEASURE_IDENTS;// DEFAULT_TOTAL_MEASUREMENTS;
  device_params.advanced_params.measure_params.max_num_of_performance_measurements = DEFAUL_MAX_NUM_OF_PERFORMANCE_MEASURE_IDENTS;//DEFAULT_TOTAL_MEASUREMENTS;
  device_params.advanced_params.measure_params.max_num_of_performance_references = DEFAUL_MAX_NUM_OF_PERFORMANCE_MEASURE_IDENTS; //DEFAULT_TOTAL_MEASUREMENTS;

  device_params.advanced_params.measure_params.num_of_valid_patient_measurements = DEFAULT_VALID_MEASUREMENTS;
  device_params.advanced_params.measure_params.num_of_valid_patient_references = DEFAULT_VALID_MEASUREMENTS;
  device_params.advanced_params.measure_params.num_of_valid_performance_measurements = DEFAULT_VALID_MEASUREMENTS;
  device_params.advanced_params.measure_params.num_of_valid_performance_references = DEFAULT_VALID_MEASUREMENTS;

  device_params.advanced_params.measure_params.bmsi_max = 4096;
  device_params.advanced_params.measure_params.bmsi_min = 0;
  device_params.advanced_params.measure_params.signal_min = 20;
  device_params.advanced_params.measure_params.signal_max = 4095;
  device_params.advanced_params.measure_params.impact_time_min = 10;
  device_params.advanced_params.measure_params.impact_time_max = 50000;
  device_params.advanced_params.measure_params.impact_amp_max = 4095;
  device_params.advanced_params.measure_params.impact_amp_min = 100;

}

/*=======================================================================*/
/*                                                                       */
/*=======================================================================*/
void fsrv_DS_SetCfgConstants(void)
{
  fsrv_DS_SetStrainGauseStat(device_operation_control.battary.starain_gauge_status);
  fsrv_DS_SetCalibrationConst((float)device_params.general_params.calibration_constant);
  fsrv_DS_SetSerialNum(device_params.advanced_params.serial_number);
  fsrv_DS_SetRefNumber(device_params.advanced_params.reference_number);
}
