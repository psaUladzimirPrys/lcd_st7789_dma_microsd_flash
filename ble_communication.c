#include "ble_communication.h"

#include "file_storage.h"

// The advertising set handle allocated from Bluetooth stack.
static uint8_t advertising_set_handle = 0xff;
// Хендл активного соединения
static uint8_t connection_handle = 0xFF;
// Буфер данных для передачи
extern uint8_t tx_data_buffer[MAX_TX_BUFFER_SIZE];
// Буфер для приема данных
extern income_packet_t rx_data_buffer;

extern uint32_t ble_current_sending_slot;     // ID текущего отправляемого слота
extern uint32_t ble_tx_payload_len;
extern uint32_t ble_tx_sent_bytes;

device_operation_control_t   device_operation_control;
signal_to_bt_stack_t signal_to_bt_stack = screen_on_signal_to_bt_stack;



// ==========================================
// ПЕРЕМЕННЫЕ ДЛЯ НАПРАВЛЕННОЙ РЕКЛАМЫ (DIRECT ADVERTISING)
// ==========================================
static bd_addr  bonded_peer_address;
static uint8_t  bonded_peer_address_type;
static bool     has_bonded_device = false; // Флаг наличия сохраненного сопряжения в ОЗУ

// Временные переменные для хранения адреса текущего активного сеанса связи
static bd_addr  current_connection_address;
static uint8_t  current_connection_address_type;
// ==========================================

bool load_last_bonded_address(bd_addr *out_addr, uint8_t *out_addr_type);

void sl_bt_on_event(sl_bt_msg_t *evt)
{
  sl_status_t sc = SL_STATUS_OK;

  //uint8_t bonding_handles[8];
  //uint32_t num_bondings = 0;
  //size_t bondings_len = 0;

  switch (SL_BT_MSG_ID(evt->header)) {

    case sl_bt_evt_system_boot_id:
      // Лимит бондингов и политика циклической перезаписи
      sc = sl_bt_sm_store_bonding_configuration(8, 1);
      if (sc != SL_STATUS_OK) { // Не критично, продолжаем с дефолтными настройками
         app_log("BLE: At start failed to store bonding config: 0x%04X\r\n", (uint16_t)sc);
      }

      // Настройка: Требовать MITM, ввод через Keyboard (кнопка)
      sc = sl_bt_sm_configure(0x0F, sl_bt_sm_io_capability_displayyesno);
      if (sc != SL_STATUS_OK) {
         app_log("BLE: At start configuration failed: 0x%04X\r\n", (uint16_t)sc);
      }

      // Разрешаем сопряжение
      sc = sl_bt_sm_set_bondable_mode(1);
      if (sc != SL_STATUS_OK) {
         app_log("BLE: At start failed to set bondable mode: 0x%04X\r\n", (uint16_t)sc);
      }
      // логическая проверка: есть ли уже привязанные устройства (смартфоны) в памяти?

      if (load_last_bonded_address(&bonded_peer_address, &bonded_peer_address_type)) {
                has_bonded_device = true;
                fsrv_BLE_SetBoundingStatus(BLE_BOUNDINGS_SET);
                app_log("BLE: At start successfully linked bonded peer address. Ready for Direct Adv.\r\n");
            } else {
                has_bonded_device = false;
                fsrv_BLE_SetBoundingStatus(BLE_BOUNDINGS_CLEAR);
                app_log("BLE: At start no paired devices found or failed to read address. Ready for general connections.\r\n");
       }

      // настройка рассылки рекламы
      sc = sl_bt_advertiser_create_set(&advertising_set_handle);
      if (sc != SL_STATUS_OK) {
          // Если сет не создался, запускать генерацию данных и старт рекламы бессмысленно
         app_log("BLE: At start Critical! Can't create advertiser set: 0x%04X\r\n", (uint16_t)sc);
         break;
      }

      sc = sl_bt_advertiser_set_timing(
        advertising_set_handle,
        160, // min. adv. interval (milliseconds * 1.6)
        160, // max. adv. interval (milliseconds * 1.6)
        0,   // adv. duration
        0);  // max. num. adv. events
      //app_assert_status(sc);
      if (sc != SL_STATUS_OK) {
          app_log("BLE: Failed to set adv timing, status: 0x%04X\r\n", (uint16_t)sc);
          // Если хендл оказался невалидным (например, стек его сбросил)
          if (sc == SL_STATUS_INVALID_HANDLE || advertising_set_handle == 0xFF) {
              app_log("BLE: Trying to re-create advertising set...\r\n");
              // Пробуем создать новый рекламный сет заново
              sl_status_t create_sc = sl_bt_advertiser_create_set(&advertising_set_handle);
              if (create_sc == SL_STATUS_OK) {
                  // Если создался успешно, пробуем задать тайминги еще раз
                  sl_bt_advertiser_set_timing(advertising_set_handle, 160, 160, 0, 0);
              } else {
                  app_log("BLE: Critical! Can't create adv set: 0x%04X\r\n", (uint16_t)create_sc);
                  break;
              }
          }
      }

      sc = sl_bt_legacy_advertiser_generate_data(advertising_set_handle, sl_bt_advertiser_general_discoverable);
      if (sc != SL_STATUS_OK) {
         app_log_error("BLE: At start failed to generate advertiser data: 0x%04X\r\n", (uint16_t)sc);
         break;
      }

    break;

    case sl_bt_evt_system_external_signal_id: // обработка внешнего сигнала

      if (evt->data.evt_system_external_signal.extsignals & screen_on_signal_to_bt_stack) {

         // При включении экрана проверяем: если у нас уже был успешный бондинг в текущей сессии,
         // запускаем направленную рекламу. Иначе — обычную общую.
         if (has_bonded_device) {
             app_log("BLE: Screen ON. Starting Directed Advertising...\r\n");
             sc = sl_bt_legacy_advertiser_start_directed(advertising_set_handle, sl_bt_legacy_advertiser_low_duty_directed_connectable, bonded_peer_address, bonded_peer_address_type);
         }

          if (sc != SL_STATUS_OK) {
              // Если стек говорит, что реклама уже крутится, это не критично — не падаем
              if (sc == SL_STATUS_INVALID_STATE) {
                  app_log("BLE: Advertising is already running.\r\n");
              }
              else {
                  app_log("BLE: Failed to start advertising, status: 0x%04X\r\n", (uint16_t)sc);
                  // Если хендл «протух» или потерялся
                  if (sc == SL_STATUS_INVALID_HANDLE || advertising_set_handle == 0xFF) {
                      app_log("BLE: Re-initialising advertising set...\r\n");
                      // Пробуем пересоздать сет на лету
                      sl_status_t recover_sc = sl_bt_advertiser_create_set(&advertising_set_handle);
                      if (recover_sc == SL_STATUS_OK) {
                          // Если создался, заново генерируем данные и пробуем включить
                          sl_bt_legacy_advertiser_generate_data(advertising_set_handle, sl_bt_advertiser_general_discoverable);
                          sl_bt_legacy_advertiser_start(advertising_set_handle, sl_bt_legacy_advertiser_connectable);
                      }
                      else {
                          app_log("BLE: Critical failure! Can't recover adv set (0x%04X). Restarting Bluetooth stack...\r\n", (uint16_t)recover_sc);
                          // Перезагружаем только BLE модуль
                          sl_bt_system_stop_bluetooth();
                          // Включаем обратно. Стек заново сгенерирует событие sl_bt_evt_system_boot_id,
                          // где вся цепочка инициализации рекламы запустится с чистого листа.
                          // обнулим переменные
                          advertising_set_handle = 0xFF;
                          connection_handle = 0xFF;
                          // Сбрасываем флаги передачи данных, чтобы устройство не пыталось слать пакеты в пустоту
                          device_operation_control.actual_mtu_size = 0;
                          device_operation_control.actual_data_size_to_transmit = 0;
                          device_operation_control.data_tranfer_control.status.transmitting_to_station_in_progress = 0; // или ALS_FALSE
                          sl_bt_system_start_bluetooth();
                      }
                  }
              }
          }
      }

      if (evt->data.evt_system_external_signal.extsignals & pairing_mode_signal_to_bt_stack) {
          sl_bt_connection_close(connection_handle);
          sl_bt_sm_delete_bondings();
          // При удалении бондингов сбрасываем флаг направленной рекламы
          has_bonded_device = false;
          fsrv_BLE_SetBoundingStatus(BLE_BOUNDINGS_CLEAR);
          fsrv_DS_SetBleStatus(BLE_DISCONNECTED);
          sc = sl_bt_legacy_advertiser_start(advertising_set_handle, sl_bt_legacy_advertiser_connectable);
          app_log("BLE: Entering pair mode Starting General Advertising...\r\n");
      }
    break;

    // -------------------------------
    case sl_bt_evt_sm_confirm_passkey_id:
      // Выводим число на дисплей (обязательно 6 знаков с ведущими нулями)
      uint32_t passkey = evt->data.evt_sm_confirm_passkey.passkey;
      app_log("BLE: Confirm Code on device: %06lu\r\n", passkey);
      fsrv_DS_SetBleStatus(BLE_PAIRING);
      fsrv_BLE_SetBoundingStatus(BLE_BOUNDINGS_CLEAR);

      fsrv_DS_SetBlePairingCode(passkey);
      sc = sl_bt_sm_passkey_confirm(evt->data.evt_sm_confirm_passkey.connection, 1); // 1 = Accept
      if (sc != SL_STATUS_OK) {
          if(fsrv_BLE_GetBoundingState() == TRUE){
             fsrv_BLE_SetBoundingStatus(BLE_BOUNDINGS_SET);
          }
         app_log("BLE: Passkey confirm failed: 0x%04X\r\n", (uint16_t)sc);
      }
    break;

    case sl_bt_evt_sm_bonded_id:
      app_log("BLE: Success! Devices bonded.\r\n");
      // ================================================================================
      // СОХРАНЕНИЕ АДРЕСА ПРИ УСПЕШНОМ БОНДИНГЕ
      // ================================================================================
            bonded_peer_address_type = current_connection_address_type;
            memcpy(&bonded_peer_address, &current_connection_address, sizeof(bd_addr));
            has_bonded_device = true;
            app_log("BLE: Target address saved for Direct Advertising.\r\n");
      // =================================================================================
      ///нарисовать зеленый кружлк
      fsrv_DS_SetBleStatus(BLE_PAIRING);
      aukh_Post_UI_Event(AU_PAIRING_MENU_OK);
      fsrv_BLE_SetBoundingStatus(BLE_BOUNDINGS_SET);
    break;

    case sl_bt_evt_sm_bonding_failed_id:
      app_log("BLE: Bonding failed: 0x%04X\r\n", evt->data.evt_sm_bonding_failed.reason);
      fsrv_DS_SetBleStatus(BLE_NOT_PAIRING);

      if (    (evt->data.evt_sm_bonding_failed.reason == SL_STATUS_BT_SMP_UNSPECIFIED_REASON)
           || (evt->data.evt_sm_bonding_failed.reason == SL_STATUS_BT_SMP_NUMERIC_COMPARISON_FAILED) ){
          aukh_Post_UI_Event(AU_PAIRING_MENU_FAILED);
          app_log("BLE: Bonding canceled. Delete all bonds.\r\n");
          has_bonded_device = false; // Сбрасываем флаг
          sl_bt_sm_delete_bondings();
          fsrv_BLE_SetBoundingStatus(BLE_BOUNDINGS_CLEAR);

          sl_bt_connection_close(connection_handle);
          fsrv_DS_SetBleStatus(BLE_DISCONNECTED);
      }

      if(evt->data.evt_sm_bonding_failed.reason == SL_STATUS_TIMEOUT) {
          aukh_Post_UI_Event(AU_PAIRING_MENU_FAILED);
      }

    break;

    // -------------------------------

    case sl_bt_evt_connection_opened_id:
      uint16_t temp_actual_max_mtu = 0;
      connection_handle = evt->data.evt_connection_opened.connection;
      device_operation_control.actual_connection_handle = connection_handle;
      //device_operation_control.status.communication_channel_open = ALS_TRUE;

      // ==========================================
      // ВРЕМЕННЫЙ ПЕРЕХВАТ АДРЕСА ПРИ ПОДКЛЮЧЕНИИ
      // ==========================================
          current_connection_address_type = evt->data.evt_connection_opened.address_type;
          memcpy(&current_connection_address, &evt->data.evt_connection_opened.address, sizeof(bd_addr));
      // ==========================================


      /* ОПТИМИЗАЦИЯ ТАЙМИНГОВ ДЛЯ БОНДИНГА:
           * Принудительно перенастраиваем параметры текущего соединения:
           * connection:  connection_handle
           * min_interval: 24 (30 мс)  -- достаточно быстро для криптографии
           * max_interval: 40 (50 мс)  -- баланс скорости и стабильности
           * latency:      0           -- периферия обязана отвечать на каждый анкор
           * timeout:      200         -- 2 секунды таймаута (вместо дефолтных 100-500 мс).
           * Дает пережить потерю пакетов без разрыва связи!
           * min_ce_len:   0
           * max_ce_len:   0xFFFF
      */
      sc = sl_bt_connection_set_parameters(connection_handle, 24, 40, 0, 200, 0, 0xFFFF);
      if (sc != SL_STATUS_OK)
      {
         app_log("BLE ERR: Set conn params failed 0x%X\r\n", (unsigned int)sc);
      }

      // Останавливаем рекламу на время бондинга (как детально разбирали ранее)
      sc = sl_bt_advertiser_stop(advertising_set_handle);
      if (sc != SL_STATUS_OK)
      {
         app_log("BLE ERR: Stop adv failed 0x%X\r\n", (unsigned int)sc);
      }

      fsrv_DS_SetBleStatus(BLE_ADVERTISING);
      app_log("BLE: BT connected. Handle: %u\r\n", (unsigned int)connection_handle);
      init_tx_data_buffer();
      sc = sl_bt_gatt_server_set_max_mtu((uint16_t)BLE_MTU_MAX_SIZE, &temp_actual_max_mtu);
      if (sc != SL_STATUS_OK) {
                  app_log("BLE CRITICAL ERR: Can't set Max MTU, error 0x%X\r\n", (unsigned int)sc);
              } else {
                  app_log("BLE: System Boot. Max MTU memory buffer allocated: %u bytes\r\n", temp_actual_max_mtu);
              }
      app_log("BLE: Requested MTU size %u\r\n", temp_actual_max_mtu);
    break;

    case sl_bt_evt_connection_closed_id:
      device_operation_control.status.communication_channel_open = ALS_FALSE;
            app_log("BLE: BT disconnected\r\n");
            fsrv_DS_SetBleStatus(BLE_DISCONNECTED);

            // Генерируем данные для рекламы
            sc = sl_bt_legacy_advertiser_generate_data(advertising_set_handle, sl_bt_advertiser_general_discoverable);

            // Если генерация провалилась — это признак «протухшего» хендла или сбоя памяти.
            // Не городим вложенные проверки, сразу отправляем стек на безопасный быстрый рестарт.
            if (sc != SL_STATUS_OK) {
                app_log("BLE: Failed to generate adv data (0x%04X). Restarting Bluetooth stack...\r\n", (uint16_t)sc);

                sl_bt_system_stop_bluetooth();
                advertising_set_handle = 0xFF;
                connection_handle = 0xFF;
                device_operation_control.actual_mtu_size = 0;
                device_operation_control.actual_data_size_to_transmit = 0;
                device_operation_control.data_tranfer_control.status.transmitting_to_station_in_progress = 0;
                sl_bt_system_start_bluetooth();

                break; // Выходим из кейса, стек сам перезапустится и уйдет в system_boot
            }

            // ==========================================
            // МОДИФИЦИРОВАННЫЙ ЗАПУСК РЕКЛАМЫ (DIRECT ИЛИ GENERAL)
            // ==========================================
                  if (has_bonded_device) {
                      app_log("BLE: Starting Low Duty Directed Advertising to bonded peer...\r\n");
                      sc = sl_bt_legacy_advertiser_start_directed(
                          advertising_set_handle,
                          sl_bt_legacy_advertiser_low_duty_directed_connectable,
                          bonded_peer_address,
                          bonded_peer_address_type
                      );
                  }

            if (sc != SL_STATUS_OK) {
                if (sc == SL_STATUS_INVALID_STATE) {
                    app_log("BLE: Advertising is already running.\r\n");
                    fsrv_DS_SetBleStatus(BLE_ADVERTISING);
                } else {
                    app_log("BLE: Heavy start error (0x%04X). Critical recovery via stack reset...\r\n", (uint16_t)sc);

                    sl_bt_system_stop_bluetooth();
                    advertising_set_handle = 0xFF;
                    connection_handle = 0xFF;
                    device_operation_control.actual_mtu_size = 0;
                    device_operation_control.actual_data_size_to_transmit = 0;
                    device_operation_control.data_tranfer_control.status.transmitting_to_station_in_progress = 0;
                    sl_bt_system_start_bluetooth();
                }
            }
    break;

    ///////////////////////////////////////////////////////////////////////////
    // Add additional event handlers here as your application requires!      //
    ///////////////////////////////////////////////////////////////////////////
    case sl_bt_evt_gatt_server_user_read_request_id:
      {

          sl_bt_evt_gatt_server_user_read_request_t *pEvent = (sl_bt_evt_gatt_server_user_read_request_t *)&(evt->data);

          app_log("BLE: Characteristic %d read request\r\n", pEvent->characteristic);

      }
    break;

    case sl_bt_evt_gatt_server_user_write_request_id:
      {
        sl_status_t sc = SL_STATUS_OK;

        sl_bt_evt_gatt_server_user_write_request_t *pEvent = (sl_bt_evt_gatt_server_user_write_request_t *)&(evt->data);
        app_log("BLE: Characteristic %d write request\r\n", pEvent->characteristic);

        if (pEvent->characteristic == gattdb_receive_data)
        {
          if((!check_flag_ble_packet_received()) && pEvent->value.len <= sizeof(income_packet_t))
          {
              memcpy(rx_data_buffer.raw_data, pEvent->value.data, pEvent->value.len);
              set_flag_ble_packet_received();
              if(!device_operation_control.status.communication_channel_open) {
                  device_operation_control.status.communication_channel_open = ALS_TRUE;
                  app_log("BLE: communication channel open\r\n");
              }
          }
          else
          {
              app_log("BLE: Received data size %d more then RX buffer %d\r\n", pEvent->characteristic, sizeof(income_packet_t));
              sc = SL_STATUS_BT_ATT_INSUFFICIENT_RESOURCES;
          }

          sl_status_t response_sc = sl_bt_gatt_server_send_user_write_response(pEvent->connection, pEvent->characteristic, sc);
          if (response_sc != SL_STATUS_OK) {
              // Вместо падения просто пишем в отладочный лог
              app_log("BLE: Failed to send write response, status: 0x%04X\r\n", (uint16_t)response_sc);

              // Здесь можно обработать специфичные сценарии:
              if (response_sc == SL_STATUS_INVALID_HANDLE || response_sc == SL_STATUS_NOT_FOUND) {
                  // Ошибка в дескрипторах соединения или характеристики
              } else if (response_sc == SL_STATUS_COMMAND_TOO_LONG) {
                  // Передан слишком длинный статус ответа
              }
              // Если соединение разорвано, стек BLE сам сгенерирует событие sl_bt_evt_connection_closed_id,
              // где вы корректно сбросите состояние программы.
          }
        }
      }

      break;

    case sl_bt_evt_gatt_server_characteristic_status_id:
      {
         sl_bt_evt_gatt_server_characteristic_status_t *pEvent = (sl_bt_evt_gatt_server_characteristic_status_t *)&(evt->data);

         if (pEvent->characteristic == gattdb_transmit_data)
         {
             if (pEvent->status_flags == sl_bt_gatt_server_client_config)
             {
                 if (pEvent->client_config_flags & sl_bt_gatt_indication)
                 {

                    fsrv_DS_SetBleStatus(BLE_PAIRING);
                    fsrv_BLE_SetBoundingStatus(BLE_BOUNDINGS_SET);
                    app_log("BLE: Client subscribed\r\n");
                    //device_operation_control.status.communication_channel_open = ALS_TRUE;
                 }
                 else
                 {
                    fsrv_DS_SetBleStatus(BLE_NOT_PAIRING);
                    app_log("BLE: Client unsubscribed, communication channel closed\r\n");
                    device_operation_control.status.communication_channel_open = ALS_FALSE;
                    device_operation_control.data_tranfer_control.status.transmitting_to_station_in_progress = ALS_FALSE;
                 }
             }
             else if (pEvent->status_flags == sl_bt_gatt_server_confirmation)
             {
                // Подтверждение получено, отправляем следующий фрагмент или заканчиваем передачу
                 device_operation_control.data_tranfer_control.status.transmitting_to_station_in_progress = 0;
                 app_log("BLE: Confirmation received\r\n");
                if (device_operation_control.data_tranfer_control.status.transmitting_to_station_in_progress == ALS_FALSE)
                {
                    if(device_operation_control.data_tranfer_control.status.ble_sync_session_active == true) {
                        app_log("BLE STREAM: SD Session Active. Sent %lu of %lu bytes\r\n", ble_tx_sent_bytes, ble_tx_payload_len);
                        // Проверяем, остались ли еще не отправленные данные на SD-карте для этого слота
                        if (ble_tx_sent_bytes < ble_tx_payload_len) {
                               // На флешке есть продолжение! Формируем СЛЕДУЮЩИЙ пакет в tx_data_buffer
                                uint32_t next_packet_size = create_sd_history_packet(tx_data_buffer, ble_current_sending_slot);
                                if (next_packet_size > 0) {
                                       set_num_of_bytes_to_transmit(next_packet_size);
                                       transmit_data_via_indication(); // Запускаем отправку следующего пакета 'C'
                                }
                                else {
                                    app_log("BLE STREAM [Error]: Failed to construct next history packet!\r\n");
                                    // Сбрасываем сессию при ошибке чтения SD
                                    ble_tx_sent_bytes = 0;
                                    device_operation_control.data_tranfer_control.status.ble_sync_session_active = false;
                                }
                        }
                        else {
                           // Сюда мы попадем, когда create_sd_history_packet отправит пакет со скобкой ']'
                           // Передача слота полностью завершена!
                           app_log("BLE STREAM: All history packets sent and confirmed!\r\n");
                           // Помечаем запись как успешно переданную
                           ble_mark_slot_as_sent(ble_current_sending_slot);
                           // Сбрасываем счетчик для будущих сессий
                           ble_tx_sent_bytes = 0;
                           device_operation_control.data_tranfer_control.status.ble_sync_session_active = false;
                        }
                    }
                    else {
                        send_data_fragment_via_indication();
                    }
                }

             }
         }
      }
     break;

     case sl_bt_evt_gatt_mtu_exchanged_id:
          app_log("BLE: Negotiated MTU size %u, payload %u\r\n", evt->data.evt_gatt_mtu_exchanged.mtu, (evt->data.evt_gatt_mtu_exchanged.mtu - 3));
          device_operation_control.actual_mtu_size = evt->data.evt_gatt_mtu_exchanged.mtu;
          device_operation_control.actual_data_size_to_transmit = (evt->data.evt_gatt_mtu_exchanged.mtu - 3);
          sl_bt_sm_increase_security(connection_handle);
     break;

    default:
      break;
  }
}


void check_pair_command_payload(const uint8_t *buff)
{
  bool all_digits = true;
  uint16_t pair_code = 0;
  sl_status_t sc = SL_STATUS_OK;

  if(buff == NULL) return;

  if(memcmp(buff,"UNDO", 4) == 0)
  {
      sl_bt_sm_delete_bondings();
      app_log("PROTOCOL: PAIR UNDO done\r\n");
      fsrv_BLE_SetBoundingStatus(BLE_BOUNDINGS_CLEAR);
      fsrv_DS_SetBleStatus(BLE_DISCONNECTED);
      sc = sl_bt_legacy_advertiser_start(advertising_set_handle, sl_bt_legacy_advertiser_connectable);
      if (sc == SL_STATUS_OK) {
        fsrv_DS_SetBleStatus(BLE_ADVERTISING);
      }

      return;
  }
  else
  {
      for(uint8_t i = 0; i < PAIR_PIN_CODE_SIZE; i++)
      {
          if(buff[i] < '0' || buff[i] > '9')
          {
              all_digits = false;
          }
      }

      if(all_digits == true)
      {
          for(uint8_t i = 0; i < PAIR_PIN_CODE_SIZE; i++)
          {
              pair_code = pair_code * 10 + (buff[i] - '0');
          }
          app_log("PROTOCOL: PAIR numeric code %u\r\n", pair_code);
      }
  }
}

void send_signal_to_bt_stack (signal_to_bt_stack_t signal)
{
  sl_bt_external_signal((uint32_t)signal);
}


void fsrv_BLE_SetCloseConnection(Bool send_signal)
{

  if (send_signal == TRUE) {
   send_signal_to_bt_stack(pairing_mode_signal_to_bt_stack);
  }

  fsrv_DS_SetBlePairingCode(0);
  sl_bt_sm_delete_bondings();
  fsrv_BLE_SetBoundingStatus(BLE_BOUNDINGS_CLEAR);
  fsrv_DS_SetBleStatus(BLE_DISCONNECTED);

}


Bool fsrv_BLE_GetBoundingState(void)
{
  sl_status_t sc = SL_STATUS_OK;
  uint8_t     bonding_handles[8];
  uint32_t    num_bondings = 0;
  size_t      bondings_len = 0;
  Bool        ret_val = FALSE;


   sc = sl_bt_sm_get_bonding_handles(0, &num_bondings, sizeof(bonding_handles), &bondings_len, bonding_handles);
   if (sc == SL_STATUS_OK) {
      if (num_bondings > 0) {
         //app_log("BLE: At start fFound %lu paired device(s) in flash memory.\r\n", (unsigned int)num_bondings);
         ret_val = TRUE;
      } else {
          //app_log("BLE: At start no paired devices found. Ready for new connections.\r\n");
          ret_val = FALSE;
      }
   } else {
       //app_log("BLE: At start failed to read bonding handles: 0x%04X\r\n", (long unsigned int)sc);
       ret_val = FALSE;
   }

  return ret_val;
}

void fsrv_BLE_TurnOnSignal(void)
{

  fsrv_BLE_SetBoundingStatus(fsrv_BLE_GetBoundingState());
  fsrv_DS_SetBleStatus(BLE_DISCONNECTED);
  fsrv_DS_SetBlePairingCode(0);

  send_signal_to_bt_stack(screen_on_signal_to_bt_stack);
}

/*=======================================================================*/
/*                                                                       */
/*=======================================================================*/
void fsrv_BLE_TurnOffSignal(void)
{
  app_log("BLE: Turn OFF subsystem of BLE from UI\r\n" );
}

/*=======================================================================*/
/*                                                                       */
/*=======================================================================*/
// Функция пытается найти и загрузить адрес последнего сохраненного устройства
bool load_last_bonded_address(bd_addr *out_addr, uint8_t *out_addr_type)
{
    sl_status_t sc;
    uint8_t  bonding_handles[4]; // 4 байта = 32 возможных хендла (бит на хендл)
    uint32_t num_bondings = 0;
    size_t   bondings_len = 0;

    // 1. Получаем количество бондингов и битовую маску хендлов
    sc = sl_bt_sm_get_bonding_handles(0,
                                      &num_bondings,
                                      sizeof(bonding_handles),
                                      &bondings_len,
                                      bonding_handles);

    if (sc != SL_STATUS_OK || num_bondings == 0) {
        app_log("BLE: No bondings found or error: 0x%04X\r\n", (uint16_t)sc);
        return false;
    }

    bd_addr peer_address;
    uint8_t peer_address_type = 0;
    uint8_t security_mode = 0;
    uint8_t key_size = 0;

    // 2. Распаковываем битовую маску:
    //    Бит 0 байта 0 = хендл 0
    //    Бит 1 байта 0 = хендл 1
    //    Бит 0 байта 1 = хендл 8 и т.д.
    for (size_t byte_idx = 0; byte_idx < bondings_len; byte_idx++) {
        for (uint32_t bit_idx = 0; bit_idx < 8; bit_idx++) {
            if (bonding_handles[byte_idx] & (1u << bit_idx)) {
                uint32_t bonding_handle = (uint32_t)(byte_idx * 8 + bit_idx);

                sc = sl_bt_sm_get_bonding_details(bonding_handle,
                                                  &peer_address,
                                                  &peer_address_type,
                                                  &security_mode,
                                                  &key_size);

                if (sc == SL_STATUS_OK) {
                    memcpy(out_addr, &peer_address, sizeof(bd_addr));
                    *out_addr_type = peer_address_type;

                    app_log("BLE: Loaded address from bonding handle %lu: "
                            "%02X:%02X:%02X:%02X:%02X:%02X (Type: %d)\r\n",
                            bonding_handle,
                            peer_address.addr[5], peer_address.addr[4],
                            peer_address.addr[3], peer_address.addr[2],
                            peer_address.addr[1], peer_address.addr[0],
                            peer_address_type);
                    return true; // Возвращаем первый найденный бондинг
                } else {
                    app_log("BLE: sl_bt_sm_get_bonding_details failed for handle %lu. "
                            "Error: 0x%04X\r\n", bonding_handle, (uint16_t)sc);
                }
            }
        }
    }

    app_log("BLE: No valid bonding details found.\r\n");
    return false;
}
