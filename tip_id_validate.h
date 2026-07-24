#ifndef __ALS_TIP_ID_VALIDATE_H__
#define __ALS_TIP_ID_VALIDATE_H__

#include <ctype.h>
#include <inttypes.h>
#include <stdio.h>
#include "app_log.h"
#include "psa/crypto.h"

#include "protocol.h"


// Константы
#define TIP_ID_LEN      5
#define DISTANCE        1000
#define TARGET          42

#define SECRET_KEY      5318008

#define TIP_MAX_RECORDS   10048 // Оптимально для секторов по 512 байт (157 секторов по 64 записи)
/**
 * @brief Состояния валидности наконечника.
 * Используем префикс для предотвращения конфликтов имен.
 */
typedef enum {
  TIP_STATE_INVALID = 0,             ///< Наконечник не прошел проверку хеша
  TIP_STATE_VALID   = 1,             ///< Наконечник валиден и готов к использованию
  TIP_STATE_USED    = 2,             ///< Наконечник валиден, но уже был использован
} tip_state_t;

typedef struct {
    char last_rec;
    char state;
    char id[TIP_ID_LEN];
    char end_marker;
} tip_save_info_t;

void tip_validate(const uint8_t *buff);
tip_state_t verify_tip_id(const char* tip_id, uint16_t instrument_serial);

#endif
