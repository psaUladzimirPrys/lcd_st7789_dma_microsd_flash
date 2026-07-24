#ifndef ALS_PROTOCOL_H
#define ALS_PROTOCOL_H

#include "app_log.h"
#include "time_and_date.h"
#include "tip_id_validate.h"
#include "ble_communication.h"
#include "params.h"
#include "adc.h"
#include "fsrv.h"
#include "aukh.h"

#define HEART_BEAT_INTERVAL_MS    5000

// Command constants
#define CMD_ID_VTIP 0x50495456 // "VTIP"
#define CMD_ID_STPM 0x4D505453 // "STPM"
#define CMD_ID_STPC 0x43505453 // "STPC"
#define CMD_ID_STOP 0x504F5453 // "STOP"
#define CMD_ID_INFO 0x4F464E49 // "INFO"
#define CMD_ID_SETP 0x50544553 // "SETP"
#define CMD_ID_GETP 0x50544547 // "GETP"
#define CMD_ID_SYNS 0x534E5953 // "SYNS"
#define CMD_ID_SYNP 0x504E5953 // "SYNP"
#define CMD_ID_SYNE 0x454E5953 // "SYNE"
#define CMD_ID_SYNC 0x434E5953 // "SYNC"
#define CMD_ID_PAIR 0x52494150 // "PAIR"
#define CMD_ID_DIAG 0x47414944 // "DIAG"
#define CMD_ID_TIME 0x454D4954 // "TIME" +
#define CMD_ID_PING 0x474E4950 // "PING"
#define CMD_ID_SETA 0x41544553 // "SETA"


//Тип функции-обработчика для времени
typedef void (*time_callback_t)(const uint8_t *unix_time_buff); /// TIME packet handler
typedef void (*pair_callback_t)(const uint8_t *pair_buff); /// PAIR packet handler
typedef void (*vtip_callback_t)(void); /// VTIP packet handler
typedef void (*setp_callback_t)(const uint8_t *buff); /// GETP packet handler
typedef void (*getp_callback_t)(uint8_t *buff); /// GETP packet handler
typedef void (*stpm_callback_t)(void); /// GETP packet handler
typedef void (*stpc_callback_t)(void); /// GETP packet handler
typedef void (*info_callback_t)(uint8_t *buff); /// GETP packet handler
typedef bool (*syns_callback_t)(uint8_t *buff, uint32_t idx); ///

// Структура для подписки на события протокола
typedef struct {
    time_callback_t on_time_received;
    pair_callback_t on_pair_received;
    vtip_callback_t on_vtip_received;
    setp_callback_t on_setp_received;
    getp_callback_t on_getp_received;
    stpm_callback_t on_stpm_received;
    stpm_callback_t on_stpc_received;
    info_callback_t on_info_received;
    syns_callback_t on_syns_received;
    // Сюда можно добавить другие callback-и: on_vtip_received и т.д.
} protocol_callbacks_t;


#pragma pack(push,1)

//basic packet header
typedef struct
{
  union
  {
    char     command[4];
    uint32_t command_u32;
  } cmd;
}packet_head_t;

typedef struct // VTIP
{
  packet_head_t packet_header;
  char tip_id[5];
}packet_vtip_t;

typedef struct //SETP
{
  packet_head_t packet_header;
  //char screen_off_timeout[4];
  //char power_saving_mode_timeout[4];
  //char calibration_constant[3];
  //char brightness[3];
  //char battery_threshold_warning[2];
  //char battery_threshold_critical[2];
  char setp_data[18];
}packet_setp_t;

typedef struct // pair
{
  packet_head_t packet_header;
  char undo_or_code[4];
}packet_pair_t;

typedef struct // TIME
{
  packet_head_t packet_header;
  char unix_time[4];
}packet_time_t;

typedef struct //SETA as a max packet
{
  packet_head_t packet_header;
  char battery_threshold_warning[2];
  char battery_threshold_critical[2];
  char max_total_identations_patient[2];
  char max_total_identations_performance[2];
  char max_total_identations_reference[2];
  char expected_valid_identations_patient[2];
  char expected_valid_identations_performance[2];
  char expected_valid_identations_reference[2];
  char max_count_of_bad_checks[2];
  char disable_patient_check;
  char enable_measurments_raw_data_storing;
}packet_seta_t;


// Добавьте сюда новые структуры, когда они появятся
typedef union
{
  packet_head_t header;
  packet_vtip_t vtip;
  packet_setp_t setp;
  packet_pair_t pair;
  packet_time_t time;
  packet_seta_t seta;

} packet_data_t;

typedef union
{
  packet_data_t types;                     // Доступ через типы: msg.types.seta...
  uint8_t raw_data[sizeof(packet_data_t)]; // Буфер, который ВСЕГДА равен макс. структуре

} income_packet_t;


#pragma pack(pop)

void init_protocol_callbacks_calls(void);

void income_packet_processing (void);
void set_flag_ble_packet_received (void);
void clear_flag_ble_packet_received (void);
bool check_flag_ble_packet_received (void);

void init_tx_data_buffer(void);
void set_num_of_bytes_to_transmit(uint16_t num_of_bytes);
void transmit_data_via_indication (void);
void send_data_fragment_via_indication(void);

void repeated_start_of_transmition (void);

void send_strm_packet(void);
void send_stpc_packet(void);
void send_stpm_packet(void);

void send_on_start_press_performance_packet(void);
void send_on_start_press_performance_reference_packet(void);

void send_on_start_press_patient_packet(void);
void send_on_start_press_patient_reference_packet(void);

void send_stop_packet(void);

void heart_beat_process (void);

#endif
