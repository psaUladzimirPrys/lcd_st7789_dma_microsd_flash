#include "protocol.h"
#include "adc.h"
#include "file_storage.h"

volatile bool ble_packet_received = false;
income_packet_t rx_data_buffer;
uint8_t tx_data_buffer[MAX_TX_BUFFER_SIZE];

extern device_operation_control_t   device_operation_control;
extern device_params_t device_params;

extern uint32_t cached_hist_active_index;
//extern uint32_t ble_tx_sent_bytes;

uint32_t heart_beat_timer_ms = 0;
uint32_t bytes_to_send_for_heart_beat = 0;

bool ble_stream_start_latest_history(uint8_t *target_buf, uint32_t index);

static const protocol_callbacks_t event_callbacks =
{
  .on_time_received = set_rtc_from_unix_bytes,
  .on_pair_received = check_pair_command_payload,
  .on_vtip_received = switch_machine_to_tip_id_validate,
  .on_setp_received = get_general_params,
  .on_getp_received = send_general_params_to_ovs_station,
  .on_stpm_received = start_patient_session,
  .on_stpc_received = start_performance_session,
  .on_info_received = send_device_info_to_ovs_station,
  .on_syns_received = ble_stream_start_latest_history,
};

void set_flag_ble_packet_received (void)
{
  //ble_packet_received = true;
  device_operation_control.data_tranfer_control.status.ble_packet_received = ALS_TRUE;
}

void clear_flag_ble_packet_received (void)
{
  //ble_packet_received = false;
  device_operation_control.data_tranfer_control.status.ble_packet_received = ALS_FALSE;
}

bool check_flag_ble_packet_received (void)
{
  if(device_operation_control.data_tranfer_control.status.ble_packet_received == ALS_TRUE)
  {
    return true;
  }
  else
  {
    return false;
  }
}

/**
 * @brief Обработка входящего пакета
 * Использует switch по 32-битному ID команды для максимальной скорости.
 */

void parse_incoming_packet(const income_packet_t *pkt)
{
    if(NULL == pkt)
    {
        clear_flag_ble_packet_received();
        return;
    }

    if(!check_flag_ble_packet_received())
    {
        return;
    }

    fsrv_DS_SetBleStatus(BLE_CONNECTED);
    // Извлекаем ID команды. Благодаря union и alignment(4),
    // это происходит за 1 операцию LDR.
    uint32_t cmd_id = pkt->types.header.cmd.command_u32;

    switch (cmd_id)
    {
        case CMD_ID_VTIP:
          app_log("PROTOCOL: VTIP command received\r\n");
          if(event_callbacks.on_vtip_received != NULL)
          {
              event_callbacks.on_vtip_received();
          }
        break;

        case CMD_ID_STPM:
          app_log("PROTOCOL: STPM command received\r\n");
         // if(event_callbacks.on_stpm_received != NULL)
         // {
              //event_callbacks.on_stpm_received();
         // }
        break;

        case CMD_ID_STPC:
          app_log("PROTOCOL: STPC command received\r\n");
          aukh_Post_UI_Event(AU_PERFORMANCE_MENU_START);
          // send_stpc_packet(); /* @ToDo  Comented out by UP The function send_stpc_packet() is called by UI interface */
        break;

        case CMD_ID_STOP:
          app_log("PROTOCOL: STOP command received\r\n");
        break;

        case CMD_ID_INFO:
          app_log("PROTOCOL: INFO command received\r\n");
          if(event_callbacks.on_info_received != NULL)
          {
              event_callbacks.on_info_received(tx_data_buffer);
          }
        break;

        case CMD_ID_SETP:
          app_log("PROTOCOL: SETP command received\r\n");
          if(event_callbacks.on_setp_received != NULL)
          {
              event_callbacks.on_setp_received((const uint8_t *)pkt->types.setp.setp_data);
          }
          if(event_callbacks.on_getp_received != NULL)
          {
              event_callbacks.on_getp_received(tx_data_buffer);
          }
        break;

        case CMD_ID_GETP:
          app_log("PROTOCOL: GETP command received\r\n");
          if(event_callbacks.on_getp_received != NULL)
          {
              event_callbacks.on_getp_received(tx_data_buffer);
          }
        break;

        case CMD_ID_SYNS:
          app_log("PROTOCOL: SYNS command received\r\n");
          if(device_operation_control.data_tranfer_control.status.ble_sync_session_active == true){
              break;
          }
          if(event_callbacks.on_syns_received != NULL)
          {
              event_callbacks.on_syns_received(tx_data_buffer, cached_hist_active_index);
          }
        break;

        case CMD_ID_SYNP:
          app_log("PROTOCOL: SYNP command received\r\n");
        break;

        case CMD_ID_SYNE:
          app_log("PROTOCOL: SYNE command received\r\n");
        break;

        case CMD_ID_SYNC:
          app_log("PROTOCOL: SYNC command received\r\n");
        break;

        case CMD_ID_PAIR:
          app_log("PROTOCOL: PAIR command received\r\n");
          if (event_callbacks.on_pair_received != NULL)
          {
              event_callbacks.on_pair_received((const uint8_t *)pkt->types.pair.undo_or_code);
          }
        break;

        case CMD_ID_DIAG:
          app_log("PROTOCOL: DIAG command received\r\n");
        break;

        case CMD_ID_TIME:
          app_log("PROTOCOL: TIME command received\r\n");
          if (event_callbacks.on_time_received != NULL)
          {
              event_callbacks.on_time_received((const uint8_t *)pkt->types.time.unix_time);
              fs_sd_check_performance_is_needed(&device_operation_control.performance_required);
          }
        break;

        case CMD_ID_PING:
          app_log("PROTOCOL: PING command received\r\n");
        break;

        case CMD_ID_SETA:
          app_log("PROTOCOL: SETA command received\r\n");
        break;

        default:
            // Неизвестная команда - логируем ошибку или отправляем NACK
           // handle_unknown_command(cmd_id);
          app_log("PROTOCOL: UNKNOWN command received\r\n");
        break;
    }
    clear_flag_ble_packet_received();
}

void income_packet_processing (void)
{
  parse_incoming_packet(&rx_data_buffer);
}

void init_tx_data_buffer(void)
{
    for (int i = 0; i < MAX_TX_BUFFER_SIZE; i++)
    {
        tx_data_buffer[i] = 0;
    }
    app_log("Data buffer initialized: %d bytes.\r\n", MAX_TX_BUFFER_SIZE);
}

void send_data_fragment_via_indication(void)
{
    sl_status_t sc;

    device_operation_control.data_tranfer_control.remaining_bytes_to_send = device_operation_control.data_tranfer_control.num_bytes_to_send -
                                                                            device_operation_control.data_tranfer_control.bytes_already_sent;

    if (device_operation_control.data_tranfer_control.remaining_bytes_to_send == 0)
    {
        device_operation_control.data_tranfer_control.status.transmitting_to_station_in_progress = ALS_FALSE;
        //memset(tx_data_buffer,0,MAX_TX_BUFFER_SIZE);
        //app_log("PROTOCOL: Data transmit completed\r\n");
        return;
    }

    if (!device_operation_control.data_tranfer_control.status.transmitting_to_station_in_progress || device_operation_control.actual_connection_handle == 0xFF)
    {
        app_log("PROTOCOL: Attempted to send, but stream is inactive or disconnected.\r\n");
        return;
    }

    uint16_t current_fragment_size = (device_operation_control.data_tranfer_control.remaining_bytes_to_send >= device_operation_control.actual_data_size_to_transmit) ?
                                      device_operation_control.actual_data_size_to_transmit : device_operation_control.data_tranfer_control.remaining_bytes_to_send;

    sc = sl_bt_gatt_server_send_indication(device_operation_control.actual_connection_handle,
                                           gattdb_transmit_data,
                                           current_fragment_size,
                                           &tx_data_buffer[device_operation_control.data_tranfer_control.bytes_already_sent]
                                          );

    if (sc == SL_STATUS_OK)
    {
        device_operation_control.data_tranfer_control.bytes_already_sent += current_fragment_size;

        if (device_operation_control.data_tranfer_control.bytes_already_sent == device_operation_control.data_tranfer_control.num_bytes_to_send)
        {
            //app_log("PROTOCOL: Last fragment (size %u) sent. Waiting for final confirmation\r\n", current_fragment_size);
        }
        else
        {
            //app_log("PROTOCOL: Fragment sent: %u bytes. Total sent: %u\r\n", current_fragment_size, device_operation_control.data_tranfer_control.bytes_already_sent);
        }
    }
    else
    {
        app_log("BLE: Error during indication (0x%lX). Waiting for confirmation to retry.\r\n", sc);
    }
}

uint8_t rep_start = 0;
void transmit_data_via_indication (void)
{
    if(device_operation_control.status.communication_channel_open == false)
    {   
	    device_operation_control.data_tranfer_control.status.transmitting_to_station_in_progress = FALSE;
        app_log("PROTOCOL: Communication channel closed\r\n");
        return;
    }
    if(device_operation_control.data_tranfer_control.num_bytes_to_send == 0)
    {
        app_log("PROTOCOL: Attempt to send 0 bytes, operation declined\r\n");
        return;
    }
    if(device_operation_control.data_tranfer_control.status.transmitting_to_station_in_progress == ALS_TRUE)
    {
        rep_start = 1;
        app_log("PROTOCOL: Data transmitting in progress, can't start new one\r\n");
        return;
    }
    device_operation_control.data_tranfer_control.status.transmitting_to_station_in_progress = ALS_TRUE;
    device_operation_control.data_tranfer_control.bytes_already_sent = 0;
    send_data_fragment_via_indication();
    bytes_to_send_for_heart_beat = device_operation_control.data_tranfer_control.num_bytes_to_send;
    heart_beat_timer_ms = sl_sleeptimer_tick_to_ms(sl_sleeptimer_get_tick_count());
}

void repeated_start_of_transmition (void)
{
  if(device_operation_control.status.communication_channel_open == true) {
      if(rep_start == 1){
          if(device_operation_control.data_tranfer_control.status.transmitting_to_station_in_progress == ALS_FALSE) {
              rep_start = 0;
              device_operation_control.data_tranfer_control.status.transmitting_to_station_in_progress = ALS_TRUE;
              device_operation_control.data_tranfer_control.bytes_already_sent = 0;
              send_data_fragment_via_indication();
          }
      }
  }
}

void set_num_of_bytes_to_transmit(uint16_t num_of_bytes)
{
  if(device_operation_control.status.communication_channel_open == false)
  {
      app_log("PROTOCOL: Try to set number of bytes to transmit\r\n");
      return;
  }
  device_operation_control.data_tranfer_control.num_bytes_to_send = num_of_bytes;
}


uint16_t create_strm_packet(uint8_t *ptr)
{
  uint8_t cnt = 0;

  *ptr++ = 'S';
  cnt++;
  *ptr++ = 'T';
  cnt++;
  *ptr++ = 'R';
  cnt++;
  *ptr++ = 'M';
  cnt++;
  uint_to_ascii(device_params.advanced_params.measure_params.num_of_valid_performance_measurements, 3, ptr);
  ptr += 3;
  cnt += 3;
  if(cnt > 0 ) return cnt;
  return 0;
}

void send_strm_packet(void)
{
  set_num_of_bytes_to_transmit(create_strm_packet(tx_data_buffer));
  transmit_data_via_indication();
}

uint16_t create_stpc_packet(uint8_t *ptr)
{
  uint8_t cnt = 0;

  *ptr++ = 'S';
  cnt++;
  *ptr++ = 'T';
  cnt++;
  *ptr++ = 'P';
  cnt++;
  *ptr++ = 'C';
  cnt++;
  uint_to_ascii(device_params.advanced_params.measure_params.num_of_valid_performance_measurements, 3, ptr);
  ptr += 3;
  cnt += 3;
  if(cnt > 0 ) return cnt;
  return 0;
}

void send_stpc_packet(void)
{
  set_num_of_bytes_to_transmit(create_stpc_packet(tx_data_buffer));
  transmit_data_via_indication();
}

uint16_t create_stpm_packet(uint8_t *ptr)
{
  uint8_t cnt = 0;

  *ptr++ = 'S';
  cnt++;
  *ptr++ = 'T';
  cnt++;
  *ptr++ = 'P';
  cnt++;
  *ptr++ = 'M';
  cnt++;
  uint_to_ascii(device_params.advanced_params.measure_params.num_of_valid_patient_measurements, 3, ptr);
  ptr += 3;
  cnt += 3;
  if(cnt > 0 ) return cnt;
  return 0;
}

void send_stpm_packet(void)
{
  set_num_of_bytes_to_transmit(create_stpm_packet(tx_data_buffer));
  transmit_data_via_indication();
}

uint16_t create_on_start_press_performance_packet(uint8_t *ptr)
{
  uint8_t cnt = 0;

  *ptr++ = 'T';
  cnt++;
  *ptr++ = '0';
  cnt++;
  *ptr++ = '0';
  cnt++;
  *ptr++ = '0';
  cnt++;
  *ptr++ = '0';
  cnt++;
  *ptr++ = '0';
  cnt++;
  *ptr++ = '0';
  cnt++;
  uint_to_ascii(device_params.advanced_params.measure_params.num_of_valid_performance_measurements, 3, ptr);
  ptr += 3;
  cnt += 3;
  *ptr++ = '0';
  cnt++;
  *ptr++ = '0';
  cnt++;
  *ptr++ = '0';
  cnt++;
  *ptr++ = 'F';
  cnt++;
  *ptr++ = 'P';
  cnt++;
  *ptr++ = 'E';
  cnt++;
  *ptr++ = 'M';
  cnt++;
  *ptr++ = 'R';
  cnt++;
  if(cnt > 0 ) return cnt;
  return 0;
}

void send_on_start_press_performance_packet(void)
{
  set_num_of_bytes_to_transmit(create_on_start_press_performance_packet(tx_data_buffer));
  transmit_data_via_indication();
}

uint16_t create_on_start_press_performance_reference_packet(uint8_t *ptr)
{
  uint8_t cnt = 0;

  *ptr++ = 'T';
  cnt++;
  *ptr++ = '0';
  cnt++;
  *ptr++ = '0';
  cnt++;
  *ptr++ = '0';
  cnt++;
  *ptr++ = '0';
  cnt++;
  *ptr++ = '0';
  cnt++;
  *ptr++ = '0';
  cnt++;
  uint_to_ascii(device_params.advanced_params.measure_params.num_of_valid_performance_references, 3, ptr);
  ptr += 3;
  cnt += 3;
  *ptr++ = '0';
  cnt++;
  *ptr++ = '0';
  cnt++;
  *ptr++ = '0';
  cnt++;
  *ptr++ = 'F';
  cnt++;
  *ptr++ = 'P';
  cnt++;
  *ptr++ = 'E';
  cnt++;
  *ptr++ = 'R';
  cnt++;
  *ptr++ = 'R';
  cnt++;
  if(cnt > 0 ) return cnt;
  return 0;
}

void send_on_start_press_performance_reference_packet(void)
{
  set_num_of_bytes_to_transmit(create_on_start_press_performance_reference_packet(tx_data_buffer));
  transmit_data_via_indication();
}

uint16_t create_stop_packet(uint8_t *ptr)
{
  uint8_t cnt = 0;

  *ptr++ = 'S';
  cnt++;
  *ptr++ = 'T';
  cnt++;
  *ptr++ = 'O';
  cnt++;
  *ptr++ = 'P';
  cnt++;
  if(cnt > 0 ) return cnt;
  return 0;
}


uint16_t create_on_start_press_patient_packet(uint8_t *ptr)
{
  uint8_t cnt = 0;

  *ptr++ = 'T';
  cnt++;
  *ptr++ = '0';
  cnt++;
  *ptr++ = '0';
  cnt++;
  *ptr++ = '0';
  cnt++;
  *ptr++ = '0';
  cnt++;
  *ptr++ = '0';
  cnt++;
  *ptr++ = '0';
  cnt++;
  uint_to_ascii(device_params.advanced_params.measure_params.num_of_valid_patient_measurements, 3, ptr);
  ptr += 3;
  cnt += 3;
  *ptr++ = '0';
  cnt++;
  *ptr++ = '0';
  cnt++;
  *ptr++ = '0';
  cnt++;
  *ptr++ = 'F';
  cnt++;
  *ptr++ = 'P';
  cnt++;
  *ptr++ = 'A';
  cnt++;
  *ptr++ = 'M';
  cnt++;
  *ptr++ = 'R';
  cnt++;
  if(cnt > 0 ) return cnt;
  return 0;
}

void send_on_start_press_patient_packet(void)
{
  set_num_of_bytes_to_transmit(create_on_start_press_patient_packet(tx_data_buffer));
  transmit_data_via_indication();
}

uint16_t create_on_start_press_patient_reference_packet(uint8_t *ptr)
{
  uint8_t cnt = 0;

  *ptr++ = 'T';
  cnt++;
  *ptr++ = '0';
  cnt++;
  *ptr++ = '0';
  cnt++;
  *ptr++ = '0';
  cnt++;
  *ptr++ = '0';
  cnt++;
  *ptr++ = '0';
  cnt++;
  *ptr++ = '0';
  cnt++;
  uint_to_ascii(device_params.advanced_params.measure_params.num_of_valid_patient_references, 3, ptr);
  ptr += 3;
  cnt += 3;
  *ptr++ = '0';
  cnt++;
  *ptr++ = '0';
  cnt++;
  *ptr++ = '0';
  cnt++;
  *ptr++ = 'F';
  cnt++;
  *ptr++ = 'P';
  cnt++;
  *ptr++ = 'A';
  cnt++;
  *ptr++ = 'R';
  cnt++;
  *ptr++ = 'R';
  cnt++;
  if(cnt > 0 ) return cnt;
  return 0;
}

void send_on_start_press_patient_reference_packet(void)
{
  set_num_of_bytes_to_transmit(create_on_start_press_patient_reference_packet(tx_data_buffer));
  transmit_data_via_indication();
}

void send_stop_packet(void)
{
  set_num_of_bytes_to_transmit(create_stop_packet(tx_data_buffer));
  transmit_data_via_indication();
}


void heart_beat_process (void)
{
  if (!device_operation_control.status.communication_channel_open) {
      return;
  }
  if (device_operation_control.data_tranfer_control.status.transmitting_to_station_in_progress) {
      return;
  }

  if((sl_sleeptimer_tick_to_ms(sl_sleeptimer_get_tick_count()) - heart_beat_timer_ms) > HEART_BEAT_INTERVAL_MS) {
     /*
      if(bytes_to_send_for_heart_beat > 0) {
          set_num_of_bytes_to_transmit(bytes_to_send_for_heart_beat);
          transmit_data_via_indication();
      }
      else {
          heart_beat_timer_ms = sl_sleeptimer_tick_to_ms(sl_sleeptimer_get_tick_count());
        }*/
      memset(tx_data_buffer,0xAA,5);
      set_num_of_bytes_to_transmit(5);
      transmit_data_via_indication();
      heart_beat_timer_ms = sl_sleeptimer_tick_to_ms(sl_sleeptimer_get_tick_count());
  }
}

/**
 * @brief Запускает автоматическую выгрузку последней активной записи из SD в BLE.
 * @return true если запуск прошел успешно, false если база пуста или произошла ошибка
 */
bool ble_stream_start_latest_history(uint8_t *target_buf, uint32_t index)
{
    // Проверяем, есть ли вообще активные записи в базе данных
    if (index == 0xFFFFFFFF)
    {
        app_log("PROTOCOL: [Warning] Database is empty. Nothing to transmit.\r\n");
        return false;
    }

    app_log("PROTOCOL: Automatically selecting latest slot %lu for transmission\r\n", index);
    device_operation_control.data_tranfer_control.status.ble_sync_session_active = true;
    // Сбрасываем счетчик переданных байт потока перед началом новой сессии
    extern uint32_t ble_tx_sent_bytes;
    ble_tx_sent_bytes = 0;

    // Формируем САМЫЙ ПЕРВЫЙ пакет (стартовый) для последней активной записи прямо в tx_data_buffer
    uint32_t initial_packet_size = create_sd_history_packet(target_buf, index);

    if (initial_packet_size > 0)
    {
        // Устанавливаем количество байт для передачи
        set_num_of_bytes_to_transmit(initial_packet_size);
        // Запускаем функцию для передачи первой индикации
        transmit_data_via_indication();

        app_log("PROTOCOL: History streaming initiated. First packet size: %lu\r\n", initial_packet_size);
        return true;
    }

    app_log("PROTOCOL: [Error] Failed to create history packet for slot %lu\r\n", index);
    return false;
}

/*=======================================================================*/
/*                                                                       */
/*=======================================================================*/
Bool fsrv_DS_IsPerformanceRequired(void) {
  return (Bool)(device_operation_control.performance_required == TRUE);
}

/*=======================================================================*/
/*                                                                       */
/*=======================================================================*/
Bool fsrv_DS_IsSyncActive(void) {
  return (Bool)(device_operation_control.data_tranfer_control.status.ble_sync_session_active == TRUE);
}
