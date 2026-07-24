#ifndef __ALS_HISTORICAL_RECORDS_H__
#define __ALS_HISTORICAL_RECORDS_H__

#include "app_log.h"
#include "sl_sleeptimer.h"

#include "adc.h"
#include "tip_id_validate.h"
#include "params.h"
#include "helpers.h"


typedef struct
{
  char true_or_false;
  char result[3];
  char valid_counter_pic_ric_bms[3];
  char remain_counter[3];
  char invalid_counter[3];
  char additional_measure_required;
  char signature[4];
} one_measure_data_packet_t;

typedef struct
{
  char record_id[4]; // unic identificator
} sd_card_record_header_t;



#pragma pack(push, 1)
typedef struct {
  char sent_status;        // [1 B] '*' (не отправлено) / '#' (отправлено)
  char active_status;      // [1 B] '*' (последняя/актуальная) / '#' (не последняя)
  char packet_size[6];     // [6 B] Размер всей последующей посылки (начиная со следующего поля)
  char record_type[7];     // [7 B] "Patient" / "Perform"
  char tip_number[5];      // [5 B] "006M4" / "-----"
  char datetime_start[19]; // [19 B] "YYYY-MM-DDTHH:MM:SS"
  char datetime_end[19];   // [19 B] "YYYY-MM-DDTHH:MM:SS"
  char has_raw_data;       // [1 B] '1' (есть) / '0' (нет)
  char bytes_per_sample;   // [1 B] '4' (для 12 бит) / '5' (для 16 бит)
  char raw_points[4];      // [4 B] "0256"
  char sample_interval[5]; // [5 B] "10000"
  char reserve;            // [1 B] '0' / '1'
} historical_record_header_t;
#pragma pack(pop)

void historical_record_write_header(void);

#endif
