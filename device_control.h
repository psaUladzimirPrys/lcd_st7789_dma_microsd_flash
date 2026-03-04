#ifndef __ALS_DEVICE_CONTROL_H__
#define __ALS_DEVICE_CONTROL_H__

/* General Global Device Settings */

#define FW_VERSION        "01015" // FW Version
#define BUZZER_PWM_FRQ    2000             // PWM FRQ in Hz


#define LATCH_PORT         SL_GPIO_PORT_D
#define LATCH_PIN          2
#define START_BUTTON_PORT  SL_GPIO_PORT_C
#define START_BUTTON_PIN   7


/* End General Global Device Settings */

/**  информация о стилусе (ASCII) **/
#pragma pack(push, 1)
typedef struct
{
  uint16_t scree_off_timeout;
  uint16_t power_saving_mode_timeout;
  float calibration_constant;
  uint8_t brightness_level;
  uint8_t battery_threshold_warning;
  uint8_t battery_threshold_critical;
  uint16_t crc16;
}device_parameters_t;
#pragma pack(pop)


#pragma pack(push, 1)
typedef struct __attribute__((packed))
{
    uint32_t serial_number;          // XXXXXXXXXX (10 цифр)
    uint32_t fw_version;             // XX.XX → например 0102 = 1.02
    float    calibration_constant;   // X.XXX
    uint8_t  battery_life;            // 0–100 %
    float    strain_gauge_voltage;    // V
    struct
    {
        uint8_t charging_status     : 1; // 0/1
        uint8_t strain_gauge_status : 1; // 0/1
        uint8_t reserved            : 6;
    } status;
    uint16_t crc16;                   // CRC структуры
} device_info_new_t;
#pragma pack(pop)



void device_working_loop(void);
void reset_device_params(void);
void device_params_update_crc(device_parameters_t *p);
bool device_params_crc_validate(const device_parameters_t *p);

void serial_connection_commands_processing(void);

float get_calibration_constatnt_fl(void);


#endif

