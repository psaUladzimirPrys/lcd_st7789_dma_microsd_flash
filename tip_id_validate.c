#include "tip_id_validate.h"
#include "file_storage.h"
#include "params.h"
#include "fsrv.h"
#include "aukh.h"

extern uint8_t tx_data_buffer[];
extern device_params_t device_params;

tip_state_t tip_state;
tip_save_info_t tip_save_struct;

extern bool is_tip_marked_used_in_session;

/**
 * @brief Безопасная конвертация Base36 в int64_t.
 */
static int64_t base36_to_int64(const char *str, size_t len)
{
    uint64_t result = 0;
    for (size_t i = 0; i < len && str[i] != '\0'; i++)
    {
        uint8_t c = (uint8_t)toupper((unsigned char)str[i]);
        uint32_t v = (c >= '0' && c <= '9') ? (uint32_t)(c - '0') : (uint32_t)(c - 'A' + 10);

        if (v >= 36) return -1; // Некорректный символ

        result = result * 36 + v;
    }
    return (int64_t)result;
}

/**
 * @brief Конвертирует числовой серийный номер в строку "00130"
 * и декодирует её из Base36 в десятичное число.
 */
static int64_t convert_serial_as_base36(uint16_t serial)
{
    char buf[16];

    // Форматируем число с ведущими нулями до 5 символов (например, "00130")
    snprintf(buf, sizeof(buf), "%05u", serial);

    // Переводим полученную строку из base36 в int64_t (для "00130" вернет 1404)
    return base36_to_int64(buf, strlen(buf));
}

/**
 * @brief Валидация наконечника (Tip ID) по оригинальному алгоритму.
 */

tip_state_t verify_tip_id(const char* tip_id, uint16_t instrument_serial)
{
//#warning "remove after testing"
//    return TIP_STATE_VALID;   /* @ToDo UP must remove after testing immediately*/

    if (!tip_id || !instrument_serial) {
        return TIP_STATE_INVALID;
    }

    int64_t tip_id_int = base36_to_int64(tip_id, TIP_ID_LEN);
    if (tip_id_int == -1) {
        return TIP_STATE_INVALID;
    }

    int64_t serial_int = convert_serial_as_base36(instrument_serial);
    if (serial_int == -1) {
        return TIP_STATE_INVALID;
    }

    int64_t x = tip_id_int + serial_int + SECRET_KEY;
    int64_t squared = x * x;

    // --- МАТЕМАТИЧЕСКИЙ МЕТОД СЕРЕДИНЫ КВАДРАТА (БЕЗ СТРОК) ---
    // Для x = 5319935, squared = 28301708404225
    // Отрезаем 2 знака справа посредством деления на 100
    int64_t final_x = squared / 100; // Станет 283017084042
    // Отрезаем 2 знака слева
    // Для 14-значного числа squared/100 становится 12-значным.
    // Чтобы убрать первые 2 цифры, берем остаток от деления на 10 000 000 000 (10^10)
    final_x = final_x % 10000000000LL; // Станет 3017084042

    app_log("TIP VALIDATION: TIP ID - %.5s, Final X = %lu%lu\r\n", tip_id, (unsigned long)(final_x >> 32), (unsigned long)final_x);

    uint32_t mod_check = (uint32_t)(final_x % DISTANCE);
    app_log("TIP VALIDATION: Modulus check = %lu (Expected = %u)\r\n", (unsigned long)mod_check, (unsigned int)TARGET);

    if (mod_check == TARGET) {
        app_log("TIP VALIDATION: TIP ID valid\r\n");
        tip_save_struct.state = 'V';
        memcpy(tip_save_struct.id, tip_id, TIP_ID_LEN);
        return TIP_STATE_VALID;
    } else {
        app_log("TIP VALIDATION: TIP ID invalid\r\n");
        tip_save_struct.state = 'I';
        memcpy(tip_save_struct.id, tip_id, TIP_ID_LEN);
        return TIP_STATE_INVALID;
    }
}

uint16_t create_tip_answer_packet(uint8_t *ptr, tip_state_t state)
{
  if (!ptr) return 0;

  uint16_t cnt = 0;
  switch (state)
  {
    case TIP_STATE_VALID:
      app_log("TIP VALIDATION: TIP valid\r\n");
      *ptr = 'T';
      cnt++;
    break;

    case TIP_STATE_USED:
      app_log("TIP VALIDATION: TIP used\r\n");
      *ptr = 'U';
      cnt++;
    break;

    case TIP_STATE_INVALID:
    default:
      app_log("TIP VALIDATION: TIP invalid\r\n");
      *ptr = 'F';
      cnt++;
    break;
  }
  return cnt;
}

void tip_validate(const uint8_t *buff)
{
  // Первичная валидация по математическому алгоритму (проверка контрольной суммы)
  tip_state_t temp = verify_tip_id((const char*)buff, device_params.advanced_params.serial_number);
  // Если математически ID верен ('V'), проверяем его по базе данных на SD-карте
  if (temp == TIP_STATE_VALID) {
      // Передаем структуру ПО АДРЕСУ (&tip_save_struct)
      fs_sd_is_tip_valid_fast(&tip_save_struct, sizeof(tip_save_struct), &temp);

//#warning "remove after testing"
//temp = TIP_STATE_VALID;   /* @ToDo UP must remove after testing immediately*/

      if (temp == TIP_STATE_USED) {
        // Идентификатор найден в файле — наконечник уже использовался ранее
        app_log("TIP VALIDATION: Database check - Already USED\r\n");
        fsrv_DS_SetTipIdStat(TIP_ID_USED);

      } else if (temp == TIP_STATE_VALID) {
        // Наконечник новый, успешно внесен в базу как валидный
        is_tip_marked_used_in_session = false;
        app_log("TIP VALIDATION: Database check - Valid & Registered\r\n");
        fsrv_DS_SetTipIdStat(TIP_ID_VALID);
      }
    }else {
        // Ошибка записи/чтения файла или ID математически был неверен ('I')
    temp = TIP_STATE_INVALID;
    app_log("TIP VALIDATION: Tip ID Invalid\r\n");
    fsrv_DS_SetTipIdStat(TIP_ID_INVALID);
  }

    // Формируем пакет на основе финального статуса (T, U или F)
    set_num_of_bytes_to_transmit(create_tip_answer_packet(tx_data_buffer, temp));
    // отправляем данные
    transmit_data_via_indication();
}
