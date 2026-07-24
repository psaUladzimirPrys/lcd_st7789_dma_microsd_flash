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
#define FSRV_FIRMWARE_VERSION  "000.04.1"

#define FSRV_GAUGE_STATUS_GOOD  TRUE
#define FSRV_GAUGE_STATUS_BAD   FALSE

#define VALID_TIP_ID    FALSE
#define INVALID_TIP_ID  TRUE

#define BLE_BOUNDINGS_SET    TRUE
#define BLE_BOUNDINGS_CLEAR  FALSE

typedef enum
{
   ERR_CODE_NONE = 0
  ,ERR_CODE_BAT_ERROR=128
  ,ERR_CODE_BLE_ERROR
  ,ERR_CODE_SYNC_ERROR
  ,ERR_CODE_PATIENT_MEAS_ERROR
  ,ERR_CODE_PERFORMANCE_MEAS_ERROR
  ,ERR_CODE_REFERENCE_MEAS_ERROR
  ,ERR_CODE_MODE_ERROR
  ,ERR_CODE_TIP_ID_ERROR

  ,ERR_MAX_INDEX_CODE_ERROR
  
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
  BLE_DISCONNECTED = 0x00,// Waiting                   //No      PHY connection
  BLE_CONNECTED,          // Ok                        //YES     PHY connect and YES bounding key
  BLE_ADVERTISING,        // Waiting                   //Waiting PHY connection
  BLE_PAIRING,            // Numeric display              //YES PHY connect and SET new bounding key
  BLE_NOT_PAIRING,        // Not Pairing,     and Failed  //YES PHY connect and NO      bounding key
  BLE_ERROR               // Failed
} st_ble_connect_t;

// Synchronization status
typedef enum
{
  SYNC_IDLE,
  SYNC_IN_PROGRESS,
  SYNC_FAILED,
  SYNC_COMPLETED,
  SYNC_STORED_LOCALLY,
  SYNC_ERROR
} st_sync_t;

// Status of current measurement (process)
typedef enum
{
  MEAS_IDLE = 0x00,
  MEAS_START,
  MEAS_IN_PROGRESS,
  MEAS_COMPLETE,
  MEAS_INCOMPLETE,
  MEAS_UNSTABLE,
  MEAS_ERROR
} st_measure_t;

typedef enum
{
  MODE_NONE = 0x00,
  MODE_PATIENT,
  MODE_PERFORMANCE,
  MODE_REFERENCE,
  MODE_ERROR
} measure_mode_t;

typedef enum 
{
  TIP_ID_VALID,
  TIP_ID_INVALID,
  TIP_ID_USED,
  TIP_ID_WAITING,
  TIP_ID_ERROR
}st_tip_id_t;

typedef struct
{
  const char     * fw_version;
  uint32_t         serial_num;
  st_battery_t     bat_status;
  st_ble_connect_t ble_status;
  st_sync_t        sync_status;
  uint32_t         ble_pairing_code;
  Bool             ble_bondings_status;
 
  // strain gauge parameters
  float            calibration_const;
  uint32_t         ref_number;
  Bool             strain_gause_stat;  // good/bad
  uint16_t         strain_gauge_value; // current value

  // for measurement mode
  uint32_t         valid_indentations;
  st_measure_t     measurement_status;
  measure_mode_t   measurement_mode;
  st_tip_id_t           tip_id_stat;           //Valid \ Invalid  Tip ID


  // Results
  float bone_score;
  Bool  is_bone_score_approximate;
   // ...
  err_code_t  error_code;
} fsrv_display_datastore_t;

void fsrv_Init(void);

void fsrv_Update(void);
void fsrv_ErrorManager(void);

/* ================= GET FUNCTIONS ================= */

const char * fsrv_DS_GetFwVersion(void);
const char * fsrv_DS_GetCurrentDate(void);
const char * fsrv_DS_GetCurrentTime(void);
uint8_t      fsrv_DS_GetAmPmTimeSuffixImageId(void);
uint32_t     fsrv_DS_GetSerialNum(void);

img_storage_id_t fsrv_DS_GetBatStatusImageId(void);
img_storage_id_t fsrv_DS_GetChargeBatLow10StatusImageId(void);
Bool fsrv_DS_IsChargeBatStatusUpdate(void);
img_storage_id_t fsrv_DS_GetBleStatusImageId(void);
img_storage_id_t fsrv_DS_GetPairingStatusImageId(void);
img_storage_id_t fsrv_DS_GetPairingWaitingStatusImageId(void);

img_storage_id_t fsrv_DS_GetSyncStatusImageId(void);
float            fsrv_DS_GetCalibrationConst(void);

// Strain Gauge
uint32_t         fsrv_DS_GetRefNumber(void);
bool             fsrv_DS_GetStrainGauseStat(void);
uint16_t         fsrv_DS_GetStrainGaugeValue(void);

//Getting status of Waiting to connect or Waiting to TIP ID values 
img_storage_id_t fsrv_DS_GetIdleWaitingStatusImageId(void);
Bool fsrv_DS_IsIdleWaitingStatUpdate(void);


//img_storage_id_t fsrv_DS_GetPerformanceStart(void);
img_storage_id_t fsrv_DS_GetPatientApproximationImageId(void);
img_storage_id_t fsrv_DS_GetPerformanceResultImageId(void);
img_storage_id_t fsrv_DS_GetPerformanceCompleteResultImageId(void);

float fsrv_DS_GetPatientResult(void);
img_storage_id_t fsrv_DS_GetPatientCompleteResultImageId(void);

//img_storage_id_t fsrv_DS_GetPatientStart(void);

img_storage_id_t fsrv_DS_GetReference(void);
img_storage_id_t fsrv_DS_GetReferenceCompleteResultImageId(void);

// Measurement Progress
uint32_t        fsrv_DS_GetValidIndentations(void);
st_measure_t    fsrv_DS_GetMeasurementStatus(void);
measure_mode_t  fsrv_DS_GetMeasurementMode(void);
st_tip_id_t     fsrv_DS_GetTipIdStatus(void);

// Results
float           fsrv_DS_GetBoneScore(void);
err_code_t      fsrv_DS_GetErrorCode(void);
uint32_t        fsrv_DS_GetBlePairingCode(void);
st_battery_t    fsrv_DS_GetBatStatus(void);

/* ================= SET FUNCTIONS ================= */

void fsrv_DS_SetFwVersion(const char *ptr);
void fsrv_DS_SetSerialNum(uint32_t num);
void fsrv_DS_SetBatStatus(st_battery_t status);
void fsrv_DS_SetBleStatus(st_ble_connect_t status);
void fsrv_DS_SetSyncStatus(st_sync_t status);
void fsrv_DS_SetBlePairingCode(uint32_t code);

void fsrv_DS_SetCalibrationConst(float val);

// Strain Gauge
void fsrv_DS_SetRefNumber(uint32_t num);
void fsrv_DS_SetStrainGauseStat(Bool stat);
void fsrv_DS_SetStrainGaugeValue(uint16_t val);

// Measurement Progress
void fsrv_DS_SetValidIndentations(uint32_t val);
void fsrv_DS_SetMeasurementStatus(st_measure_t status);
void fsrv_DS_SetMeasurementMode(measure_mode_t mode);
void fsrv_DS_SetTipIdStat(st_tip_id_t stat);

// Results
void fsrv_DS_SetBoneScore(float score);
void fsrv_DS_SetIsBoneApproximateScore(Bool approximate);

void fsrv_DS_SetErrorCode(err_code_t code);


void fsrv_BLE_SetCloseConnection(Bool send_signal);

Bool fsrv_BLE_GetBoundingState(void);
void fsrv_BLE_SetBoundingStatus(Bool val);

void fsrv_BLE_TurnOnSignal(void);
void fsrv_BLE_TurnOffSignal(void);

Bool fsrv_DS_IsPerformanceRequired(void);
Bool fsrv_DS_IsSyncActive(void);
Bool fsrv_DS_IsBatteryCharging(void);
uint8_t fsrv_DS_GetBatteryLevel(void);

void fsrv_send_final_bmsi_packet(void);
void fsrv_send_final_bmsi_packet_failed(void);

void fsrv_send_fail_session_packet(void);



#endif /* FSRV_DATASTORE_H_ */
