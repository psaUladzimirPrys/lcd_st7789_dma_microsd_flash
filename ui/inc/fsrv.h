#ifndef FSRV_DATASTORE_H_
#define FSRV_DATASTORE_H_

/*==========================================================================*/
/*        I N C L U D E S                                                   */
/*==========================================================================*/
#include <stdint.h>
#include <stdbool.h>
#include "global.h"
#include "img_storage.h"

/*=========================================================================*/
/*   G L O B A L   D E F I N I T I O N S                                   */
/*=========================================================================*/


#define FSRV_GAUGE_STATUS_GOOD  TRUE
#define FSRV_GAUGE_STATUS_BAD   FALSE

#define VALID_TIP_ID    FALSE
#define INVALID_TIP_ID  TRUE


typedef enum
{
  DEVICE_STARTUP = 0x00,
  DEVICE_LOADING,
  DEVICE_IDLE,
  DEVICE_PATIENT_MEASURE,
  DEVICE_PATIENT_MEASURE_SIMULATE,
  DEVICE_PERFORMANCE_CHECK,
  DEVICE_PERFORMANCE_CHECK_SIMULATE,
  DEVICE_CONFIGURATION,
  DEVICE_STANDBY,
  DEVICE_ERROR
} device_state_t;

// communication with display subsystem -------------------------

typedef enum
{
  ERR_NONE = 0,
}err_code_t;

typedef enum
{
  BAT_NORMAL = 0x00,
  BAT_LOW_50,
  BAT_LOW_30,
  BAT_LOW_10,
  BAT_CRITICAL,
  BAT_CHARGING,
  BAT_ERROR,
} st_battery_t;

typedef enum
{
  BLE_DISCONNECTED = 0x00,
  BLE_ADVERTISING,
  BLE_PAIRING,
  BLE_CONNECTED,
  BLE_NOT_PAIRING,
  BLE_ERROR
} st_ble_connect_t;

// Synchronization status
typedef enum
{
  SYNC_IDLE,
  SYNC_IN_PROGRESS,
  SYNC_FAILED,
  SYNC_COMPLETED,
  SYNC_STORED_LOCALLY
} st_sync_t;

// Status of current measurement (process)
typedef enum
{
  MEAS_IDLE = 0x00,
  MEAS_IN_PROGRESS,
  MEAS_COMPLETE,
  MEAS_INCOMPLETE,
  MEAS_APPROXIMATE,
  MEAS_INDENT_INCONSIST
} st_measure_t;

typedef enum
{
  MODE_NONE = 0x00,
  MODE_PATIENT,
  MODE_PERFORMANCE,
  MODE_REFERENCE
} measure_mode_t;

typedef enum 
{
  TIP_ID_VALID,
  TIP_ID_INVALID,
  TIP_ID_WAITING
}st_tip_id_t;

typedef struct
{
  uint32_t         fw_version;
  uint32_t         serial_num;
  st_battery_t     bat_status;
  st_ble_connect_t ble_status;
  st_sync_t        sync_status;

  // strain gauge parameters
  float            calibration_const;
  uint32_t         ref_number;
  bool             strain_gause_stat;  // good/bad
  uint16_t         strain_gauge_value; // current value

  // for measurement mode
  uint32_t         required_indentations;
  uint32_t         performed_indentations;
  uint32_t         valid_indentations;

  st_measure_t     measurement_status;
  measure_mode_t   measurement_mode;
  st_tip_id_t           tip_id_stat;           //Valid \ Invalid  Tip ID


  // Results
  float bone_score;
  float user_score_patient_stddev;
  float user_score_reference_stddev;
    // ...
  err_code_t  error_code;
} fsrv_display_datastore_t;


extern fsrv_display_datastore_t fsrv_display_datastore;

void fsrv_Init(void);

/* ================= GET FUNCTIONS ================= */

uint32_t     fsrv_DS_GetFwVersion(void);
uint32_t        fsrv_DS_GetSerialNum(void);

img_storage_id_t fsrv_DS_GetBatStatus(void);
img_storage_id_t fsrv_DS_GetChargeBatStatus(void);
img_storage_id_t fsrv_DS_GetBleStatus(void);
img_storage_id_t fsrv_DS_GetSyncStatus(void);

float           fsrv_DS_GetCalibrationConst(void);

// Strain Gauge
uint32_t         fsrv_DS_GetRefNumber(void);
bool      fsrv_DS_GetStrainGauseStat(void);
uint16_t         fsrv_DS_GetStrainGaugeValue(void);

//Getting status of Waiting to connect or Waiting to TIP ID values 
img_storage_id_t fsrv_DS_GetWaitingStat(void);

// Measurement Progress
uint32_t        fsrv_DS_GetRequiredIndentations(void);
uint32_t        fsrv_DS_GetPerformedIndentations(void);
uint32_t        fsrv_DS_GetValidIndentations(void);
st_measure_t    fsrv_DS_GetMeasurementStatus(void);
measure_mode_t  fsrv_DS_GetMeasurementMode(void);
st_tip_id_t     fsrv_DS_GetTipIdStat(void);

// Results
float           fsrv_DS_GetBoneScore(void);
float           fsrv_DS_GetUserScorePatientStddev(void);
float           fsrv_DS_GetUserScoreReferenceStddev(void);
err_code_t      fsrv_DS_GetErrorCode(void);
uint16_t        fsrv_GetPairingCode(void);

/* ================= SET FUNCTIONS ================= */

void fsrv_DS_SetFwVersion(uint32_t ver);
void fsrv_DS_SetSerialNum(uint32_t num);
void fsrv_DS_SetBatStatus(st_battery_t status);
void fsrv_DS_SetBleStatus(st_ble_connect_t status);
void fsrv_DS_SetSyncStatus(st_sync_t status);

// Strain Gauge
void fsrv_DS_SetRefNumber(uint32_t num);
void fsrv_DS_SetStrainGauseStat(bool stat);
void fsrv_DS_SetStrainGaugeValue(uint16_t val);

// Measurement Progress
void fsrv_DS_SetRequiredIndentations(uint32_t val);
void fsrv_DS_SetPerformedIndentations(uint32_t val);
void fsrv_DS_SetValidIndentations(uint32_t val);
void fsrv_DS_SetMeasurementStatus(st_measure_t status);
void fsrv_DS_SetMeasurementMode(measure_mode_t mode);
void fsrv_DS_SetTipIdStat(st_tip_id_t stat);

// Results
void fsrv_DS_SetBoneScore(float score);
void fsrv_DS_SetUserScorePatientStddev(float val);
void fsrv_DS_SetUserScoreReferenceStddev(float val);
void fsrv_DS_SetErrorCode(err_code_t code);

void set_device_state(device_state_t state);
device_state_t get_device_state(void);


#endif /* FSRV_DATASTORE_H_ */
