/*
 * sd_card.h
 *
 *  Created on: 15 Jan. 2026.
 *      Author: priss
 */

#ifndef FILE_STORAGE_H_
#define FILE_STORAGE_H_

// -----------------------------------------------------------------------------
//                       Includes
// -----------------------------------------------------------------------------
#include "ff.h"
#include "diskio.h"
#include "sl_sdc_sd_card.h"
#include "tip_id_validate.h"

#if (defined(SLI_SI917))
  #include "sl_si91x_gspi.h"
  #include "rsi_debug.h"

  #define app_printf(...) DEBUGOUT(__VA_ARGS__)

  static sl_gspi_instance_t gspi_instance = SL_GSPI_MASTER;
#else
  #include "sl_spidrv_instances.h"
  #include "app_log.h"
  #include "app_assert.h"

  #define app_printf(...) app_log(__VA_ARGS__)
#endif

typedef enum
{
  LOG_LEVEL_ERROR = 0,
  LOG_LEVEL_WARN,
  LOG_LEVEL_INFO,
  LOG_LEVEL_DEBUG,
  LOG_LEVEL_ALL
} log_level_t;


#define FSLOG_ERROR(...)    fslog_printf(LOG_LEVEL_ERROR, __VA_ARGS__)
#define FSLOG_WARNING(...)  fslog_printf(LOG_LEVEL_WARN,  __VA_ARGS__)
#define FSLOG_INFO(...)     fslog_printf(LOG_LEVEL_INFO,  __VA_ARGS__)
#define FSLOG_DEBUG(...)    fslog_printf(LOG_LEVEL_DEBUG, __VA_ARGS__)
#define FSLOG_ALL(...)      fslog_printf(LOG_LEVEL_ALL, __VA_ARGS__)

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

/***************************************************************************//**
 * Initialize SD card and FatFS.
 ******************************************************************************/
sl_status_t fs_sd_init(void);

sl_status_t fs_sd_deinit(void);

sl_status_t fs_sd_time_init(void);

sl_status_t fs_sd_disk_volume_status(void);

sl_status_t fs_sd_write_file(const char *path,
                             const void *data,
                                uint32_t size);

sl_status_t fs_sd_read_file(const char *file_path,
                             void *buffer,
                              uint32_t buffer_size);

sl_status_t fs_sd_read_file_and_write_flash(const char *path,
                                                  void *buffer,
                                              uint32_t buffer_size,
                                              uint32_t flash_address);

sl_status_t fs_sd_write_img_to_flash(const char *path, uint32_t flash_address);

sl_status_t fs_sd_get_file_size(const char *file_path,  uint32_t *file_size);

sl_status_t fs_sd_append_to_file(const char   *file_path,
                                 const void   *data,
                                 uint32_t      data_size);

void fs_sd_log_init(void);


void fslog_Init(void);
void fslog_Update(void);
void fslog_TurnOn(void);
void fslog_TurnOff(void);
bool fslog_IsEnabled(void);

void fslog_printf(log_level_t level, const char *fmt, ...);


sl_status_t fs_sd_init_tip_cache(void);
sl_status_t fs_sd_is_tip_valid_fast(const void *struct_ptr, size_t struct_size, tip_state_t *tip_state_ptr);
sl_status_t fs_sd_mark_last_tip_used_fast(void);

sl_status_t fs_sd_write_performance_test_date(void);
sl_status_t fs_sd_check_performance_is_needed(uint8_t *performance_required_out);

void fs_sd_historical_records_start_cache_init(void);
bool fs_sd_historical_records_is_cache_ready(void);
sl_status_t fs_sd_historical_records_init_cache(void);

sl_status_t fs_sd_historical_records_write_header(const void *header_and_data_ptr, uint32_t actual_size);
sl_status_t fs_sd_historical_records_write_data(const void *data_ptr,
                                                uint32_t size,
                                                uint8_t raw_data_in_background,
                                                uint8_t raw_data_setting,
                                                const volatile uint16_t *raw_data_ptr, // Изменен volatile на const для оптимизации
                                                uint8_t bms_pic,
                                                uint8_t bms_ric,
                                                uint8_t bmsi);

uint32_t create_sd_history_packet(uint8_t *target_buf, uint32_t slot_idx);
sl_status_t ble_mark_slot_as_sent(uint32_t slot_idx);


#endif /* FILE_STORAGE_H_ */
