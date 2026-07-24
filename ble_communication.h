#ifndef __ALS_BLE_COMMUNICATION_H__
#define __ALS_BLE_COMMUNICATION_H__

#include "sl_bt_api.h"
#include "app_assert.h"
#include "app.h"
#include "app_log.h"
#include "gatt_db.h"
#include "protocol.h"

#include "general_defs.h"

#include "fsrv.h"
#include "aukh.h"


#define ALS_TRUE                   1
#define ALS_FALSE                  0

#define BLE_MTU_MAX_SIZE          251

#define MAX_TX_BUFFER_SIZE        1024   // Общий размер данных для отправки
#define MAX_RX_BUFFER_SIZE        128

#define PAIR_PIN_CODE_SIZE        6

#pragma pack(push, 1)
typedef struct __attribute__((packed))
{
    uint16_t actual_mtu_size;
    uint16_t actual_data_size_to_transmit;
    uint8_t  actual_connection_handle;
    uint8_t  performance_required;
    struct
    {
        uint8_t communication_channel_open          : 1; /// 0/1
        uint8_t reserved                            : 7;
    } status;

    struct
    {
      uint16_t remaining_bytes_to_send;
      uint16_t num_bytes_to_send;
      uint16_t bytes_already_sent;
      struct
      {
        uint8_t transmitting_to_station_in_progress : 1; /// 0/1
        uint8_t ble_packet_received                 : 1; /// 0/1
        uint8_t ble_sync_session_active             : 1;
        uint8_t reserved                            : 5;
      }status;
    }data_tranfer_control;

    struct
    {
      uint8_t battery_level;
      uint8_t battery_charging;
      uint8_t battery_failed;
      uint8_t starain_gauge_status;
      uint16_t starain_gauge_value;
    }battary;

} device_operation_control_t;
#pragma pack(pop)

typedef enum{
  no_signal_to_bt_stack = (1 << 0),
  screen_on_signal_to_bt_stack = (1 << 1),
  pairing_mode_signal_to_bt_stack = (1 << 2),
}signal_to_bt_stack_t;

void check_pair_command_payload(const uint8_t *buff);

void send_signal_to_bt_stack (signal_to_bt_stack_t signal);

#endif
