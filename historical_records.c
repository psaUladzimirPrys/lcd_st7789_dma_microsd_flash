#include "historical_records.h"
#include "file_storage.h"

one_measure_data_packet_t one_measure_data_packet;

historical_record_header_t historical_record_header;

extern working_mode_t current_working_mode;
extern tip_save_info_t tip_save_struct;
extern device_params_t device_params;



/**
 * @brief Быстрое посимвольное форматирование текущей даты/времени из RTC во внешний буфер
 * @param out_buf Указатель на буфер размером не менее 19 байт
 */
static void format_datetime_pure(uint8_t *out_buf)
{
  sl_sleeptimer_date_t current_date;

  if (sl_sleeptimer_get_datetime(&current_date) == SL_STATUS_OK) {
    uint_to_ascii(current_date.year, 4, &out_buf[0]);
    out_buf[4] = '-';
    uint_to_ascii(current_date.month + 1, 2, &out_buf[5]);
    out_buf[7] = '-';
    uint_to_ascii(current_date.month_day, 2, &out_buf[8]);
    out_buf[10] = 'T';
    uint_to_ascii(current_date.hour, 2, &out_buf[11]);
    out_buf[13] = ':';
    uint_to_ascii(current_date.min, 2, &out_buf[14]);
    out_buf[16] = ':';
    uint_to_ascii(current_date.sec, 2, &out_buf[17]);
  } else {
    memcpy(out_buf, "2026-01-01T00:00:00", 19);
  }
}

/**
 * @brief Формирует заголовок для новой записи строго по 63-байтовому протоколу.
 * @note Время старта определяется внутри функции через RTC. Время окончания заполняется прочерками.
 * * @param header_out Указатель на структуру заголовка для заполнения
 * @param payload_size Размер всех блоков данных, идущих за заголовоком (Patient + Reference + Footer)
 * @param is_perform true если тип записи "Perform", false если "Patient"
 * @param tip_num Номер наконечника (5 символов, например "006M4")
 * @param has_raw '1' если есть сырые данные, '0' если нет
 * @param sample_points Количество сырых точек (например, 256)
 * @param interval Интервал сэмплирования (например, 10000)
 */
void create_historical_record_header(historical_record_header_t *header_out,
                                     working_mode_t working_mode,
                                     const char *tip_num,
                                     char has_raw,
                                     uint32_t interval)
{
  if (header_out == NULL) return;

  // Очищаем заголовок пробелами
  memset(header_out, ' ', sizeof(historical_record_header_t));
  // 1. Статус отправки (по умолчанию '*' - не отправлено)
  header_out->sent_status = '*';
  // 2. Статус активности (по умолчанию '#' - архивный при записи тела)
  header_out->active_status = '#';
  // 3. Размер всей последующей посылки (6 байт, сплошной текст с ведущими нулями)
  memcpy(header_out->packet_size, "000000", 6);
  // 4. Тип записи (7 байт)
  if (working_mode == WORKING_MODE_PATIENT) {
    memcpy(header_out->record_type, "Patient", 7);
  } else {
    memcpy(header_out->record_type, "Perform", 7);
  }
  // 5. Номер наконечника (5 байт)
  if (working_mode == WORKING_MODE_PATIENT) {
    memcpy(header_out->tip_number, tip_num, 5);
  } else {
    memcpy(header_out->tip_number, "-----", 5);
  }
  // 6. Время начала "YYYY-MM-DDTHH:MM:SS" (19 байт) — Формируем внутри из RTC
  format_datetime_pure((uint8_t *)header_out->datetime_start);
  // 7. Время окончания (19 байт) — Заполняем прочерками
  memcpy(header_out->datetime_end, "-------------------", 19);
  // 8. Флаг сырых данных (1 байт)
  header_out->has_raw_data = (has_raw == '1' || has_raw == '0') ? has_raw : '0';
  // 9. Байт на сэмпл (1 байт)
  header_out->bytes_per_sample = '4'; // '4' для 12 бит (каждая точка передается как 4 ascii-символа в HEX)
  // 10. Количество точек (4 байта, например "0256")
  uint_to_ascii(256, 4, (uint8_t *)header_out->raw_points);
  // 11. Интервал сэмплирования (5 байт, например "10000")
  uint_to_ascii(interval, 5, (uint8_t *)header_out->sample_interval);
  // 12. Резерв (1 байт)
  header_out->reserve = '0';
}

void historical_print_header(const historical_record_header_t *header)
{
  if (header == NULL) {
    app_log("HIST DB: Cannot print header (NULL pointer)\r\n");
    return;
  }

  app_log("======= HISTORICAL HEADER (63 bytes) =======\r\n");
  app_log("Sent Status:      %c\r\n", header->sent_status);
  app_log("Active Status:    %c\r\n", header->active_status);

  // %.*s берет размер поля из первого аргумента (6, 7, 5, 19 и т.д.)
  app_log("Packet Size:      %.*s\r\n", 6,  header->packet_size);
  app_log("Record Type:      %.*s\r\n", 7,  header->record_type);
  app_log("Tip Number:       %.*s\r\n", 5,  header->tip_number);
  app_log("DateTime Start:   %.*s\r\n", 19, header->datetime_start);
  app_log("DateTime End:     %.*s\r\n", 19, header->datetime_end);

  app_log("Has Raw Data:     %c\r\n", header->has_raw_data);
  app_log("Bytes per Sample: %c\r\n", header->bytes_per_sample);
  app_log("Raw Points:       %.*s\r\n", 4,  header->raw_points);
  app_log("Sample Interval:  %.*s\r\n", 5,  header->sample_interval);
  app_log("Reserve:          %c\r\n", header->reserve);
  app_log("============================================\r\n");
}

void historical_record_write_header(void) {

  create_historical_record_header(&historical_record_header,
                                  current_working_mode,
                                  tip_save_struct.id,
                                  device_params.advanced_params.has_raw_data,
                                  device_params.advanced_params.measure_params.adc_taking_period
                                  );
  //historical_print_header(&historical_record_header);
  fs_sd_historical_records_write_header(&historical_record_header, sizeof(historical_record_header));
}

