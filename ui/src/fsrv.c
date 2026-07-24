/*=======================================================================*/
/*        I N C L U D E S                                                */
/*=======================================================================*/

#include "fsrv.h"
#include "find_api.h"
#include "fmnu.h"
#include "auph.h"

#include "fmnu_str.h"

#include "calculations.h"


/*=======================================================================*/
/* L O C A L   F U N C T I O N   P R O T O T Y P E S                     */
/*=======================================================================*/
#define FSRV_ERROR_SLOTS_COUNT  8  /* ERR_MAX_INDEX_CODE_ERROR - 1 */

/*===========================================================================*/
/*         G L O B A L   D A T A   D E C L A R A T I O N S                   */
/*===========================================================================*/
static fsrv_display_datastore_t fsrv_display_datastore = {0};

static char fsrv_system_date_time_buffer[UI_DATE_TIME_STRING_BUFFER_SIZE];

/*==========================================================================*/
/* G L O B A L      F U N C T I O N                                         */
/*==========================================================================*/
void fsrv_Init(void)
{

 fsrv_display_datastore.fw_version = FSRV_FIRMWARE_VERSION;
 fsrv_display_datastore.serial_num = 12;
 fsrv_display_datastore.bat_status = BAT_NORMAL;
 fsrv_display_datastore.ble_status = BLE_DISCONNECTED;
 fsrv_display_datastore.ble_bondings_status = BLE_BOUNDINGS_CLEAR;

 fsrv_display_datastore.ble_pairing_code = 0;

 fsrv_display_datastore.sync_status = SYNC_IDLE;
 fsrv_display_datastore.calibration_const = 3.42;

 // strain gauge parameters
 fsrv_display_datastore.ref_number = 198;
 fsrv_display_datastore.strain_gause_stat = FSRV_GAUGE_STATUS_GOOD;  // good/bad
 fsrv_display_datastore.strain_gauge_value = 11; // current value

 // for measurement mode
 fsrv_display_datastore.valid_indentations = 4;

 fsrv_display_datastore.measurement_status = MEAS_IDLE;
 fsrv_display_datastore.measurement_mode = MODE_NONE;
 fsrv_display_datastore.tip_id_stat = TIP_ID_WAITING;           // Waiting - \ - Valid - \ Invalid - Tip ID

 // Results
 fsrv_display_datastore.bone_score = 0;
 fsrv_display_datastore.is_bone_score_approximate = FALSE;

 // ...
 fsrv_display_datastore.error_code = ERR_CODE_NONE;

}

/*=======================================================================*/
/*                                                                       */
/*=======================================================================*/
void fsrv_Update(void)
{
  fsrv_DS_SetCfgConstants();
  fsrv_ErrorManager();
}

/*=======================================================================*/
/* ERROR MANAGER - Cyclic error slot checking with distributed timing   */
/*=======================================================================*/
/**
 * @brief Cyclic error manager - checks one error slot per call
 *
 * Implements distributed polling where only ONE error slot is checked
 * per invocation, spreading workload across multiple app_process_action()
 * cycles. Static index maintains position between calls.
 *
 * Slot mapping (error_code - 1 = slot index):
 * [0] = ERR_CODE_BAT_ERROR (1)
 * [1] = ERR_CODE_BLE_ERROR (2)
 * [2] = ERR_CODE_SYNC_ERROR (3)
 * [3] = ERR_CODE_PATIENT_MEAS_ERROR (4)
 * [4] = ERR_CODE_PERFORMANCE_MEAS_ERROR (5)
 * [5] = ERR_CODE_REFERENCE_MEAS_ERROR (6)
 * [6] = ERR_CODE_MODE_ERROR (7)
 * [7] = ERR_CODE_TIP_ID_ERROR (8)
 */
void fsrv_ErrorManager(void)
{
  /* Static index maintains position between calls */
  static uint8_t current_index = 0;

  /* Check current error slot */
  switch (current_index) {

    case 0: /* BAT_ERROR slot */
      if (fsrv_display_datastore.bat_status == BAT_ERROR) {
        fsrv_display_datastore.error_code = ERR_CODE_BAT_ERROR;
      }
    break;

    case 1: /* BLE_ERROR slot */
      if (fsrv_display_datastore.ble_status == BLE_ERROR) {
        fsrv_display_datastore.error_code = ERR_CODE_BLE_ERROR;
      }
    break;

    case 2: /* SYNC_ERROR slot */
      if (fsrv_display_datastore.sync_status == SYNC_ERROR) {
        fsrv_display_datastore.error_code = ERR_CODE_SYNC_ERROR;
      }
    break;

    case 3: /* PATIENT_MEAS_ERROR slot */
      if (fsrv_display_datastore.measurement_status == MEAS_ERROR) {
        if (fsrv_display_datastore.measurement_mode == MODE_PATIENT) {
          fsrv_display_datastore.error_code = ERR_CODE_PATIENT_MEAS_ERROR;
        }
      }
    break;

    case 4: /* PERFORMANCE_MEAS_ERROR slot */
      if (fsrv_display_datastore.measurement_status == MEAS_ERROR) {
        if (fsrv_display_datastore.measurement_mode == MODE_PERFORMANCE) {
          fsrv_display_datastore.error_code = ERR_CODE_PERFORMANCE_MEAS_ERROR;
        }
      }
    break;

    case 5: /* REFERENCE_MEAS_ERROR slot */
      if (fsrv_display_datastore.measurement_status == MEAS_ERROR) {
        if (fsrv_display_datastore.measurement_mode == MODE_REFERENCE) {
          fsrv_display_datastore.error_code = ERR_CODE_REFERENCE_MEAS_ERROR;
        }
      }
    break;

    case 6: /* MODE_ERROR slot */
      if (fsrv_display_datastore.measurement_mode == MODE_ERROR) {
        fsrv_display_datastore.error_code = ERR_CODE_MODE_ERROR;
      }
    break;

    case 7: /* TIP_ID_ERROR slot */
      if (fsrv_display_datastore.tip_id_stat == TIP_ID_ERROR) {
        fsrv_display_datastore.error_code = ERR_CODE_TIP_ID_ERROR;
      }
    break;

    default:
      /* Should never reach here */
      break;
  }

  /* Process error if detected */
  if ( (fsrv_display_datastore.error_code != ERR_CODE_NONE) && (auph_GetState() != AU_ERROR_STATE) ) {
    /* Trigger error handler via simulated key AU_ERROR_STATE_SET */
    /* This will call HandleError() through the key processing pipeline */
      aukh_Post_UI_Event(AU_ERROR_STATE_SET);
    
    /* Error code remains unchanged for diagnostics until power cycle */
  }

  /* Move to next slot with wrap-around */
  current_index++;
  if (current_index >= FSRV_ERROR_SLOTS_COUNT) {
    current_index = 0;
  }

}

/* ================= GET FUNCTIONS IMPLEMENTATION ================= */
/*=======================================================================*/
/*                                                                       */
/*=======================================================================*/
const char * fsrv_DS_GetFwVersion(void) {
    return fsrv_display_datastore.fw_version;
}


/*=======================================================================*/
/*                                                                       */
/*=======================================================================*/
const char * fsrv_DS_GetCurrentDate(void) {

  sl_status_t status = get_formatted_date_string(&fsrv_system_date_time_buffer[0], sizeof(fsrv_system_date_time_buffer));

  if (status != SL_STATUS_OK) {
      /* Deterministic error handling: output a placeholder in case of RTC failure */
      fsrv_system_date_time_buffer[0] = '\0';
  }
  return &fsrv_system_date_time_buffer[0];
}

/*=======================================================================*/
/*                                                                       */
/*=======================================================================*/
const char * fsrv_DS_GetCurrentTime(void) {

  sl_status_t status = get_formatted_time_string(&fsrv_system_date_time_buffer[0], sizeof(fsrv_system_date_time_buffer));

  if (status != SL_STATUS_OK) {
      /* Deterministic error handling: output a placeholder in case of RTC failure */
      fsrv_system_date_time_buffer[0] = '\0';
  }
  return &fsrv_system_date_time_buffer[0];
}

/*=======================================================================*/
/*                                                                       */
/*=======================================================================*/
uint8_t fsrv_DS_GetAmPmTimeSuffixImageId(void) 
{
  uint8_t suffix_id = FMNU_SUFFIX_NONE;

  if ( 'P' == fsrv_system_date_time_buffer[UI_AM_PM_SUFFIX_POSITION_INDEX] )
      suffix_id = FMNU_SUFFIX_ID_PM;
  else
      suffix_id = FMNU_SUFFIX_ID_AM;

  return suffix_id;
}
/*=======================================================================*/
/*                                                                       */
/*=======================================================================*/
uint32_t fsrv_DS_GetSerialNum(void) {
    return fsrv_display_datastore.serial_num;
}

/*=======================================================================*/
/*                                                                       */
/*=======================================================================*/
img_storage_id_t fsrv_DS_GetBatStatusImageId(void) 
{
  img_storage_id_t img_id = IMG_INVALID_ID;

  switch(fsrv_display_datastore.bat_status)
  {
    case BAT_NORMAL:
      img_id = IMG_ID_PROPERTY_1_BATTERY_100;
      break;

    case BAT_LOW_50:
      img_id = IMG_ID_PROPERTY_1_VARIANT2_8;
      break;

    case BAT_LOW_30:
      img_id = IMG_ID_PROPERTY_1_VARIANT3_8;
      break;

    case BAT_LOW_10:
      img_id = IMG_ID_PROPERTY_1_VARIANT4_7;
      break;

    case BAT_CRITICAL:
      img_id = IMG_ID_PROPERTY_1_VARIANT6_7;
      break;

    case BAT_CHARGING:
      img_id = IMG_ID_PROPERTY_1_VARIANT5_7;
      break;

    case BAT_ERROR:
    default:
      img_id = IMG_INVALID_ID;
      break;
  }

    return  img_id;
}

/*=======================================================================*/
/*                                                                       */
/*=======================================================================*/
img_storage_id_t fsrv_DS_GetChargeBatLow10StatusImageId(void)
{
  if (fsrv_display_datastore.bat_status == BAT_LOW_10) {
     return IMG_ID_PROPERTY_1_VARIANT3;
  }
  return IMG_INVALID_ID;
}

/*==========================================================================================
 * @brief Checks for updates in the critical battery status image mapping.
 *
 * @details Retains the last evaluated image storage ID representing the battery
 *          charge status in static memory. By calling the underlying state provider
 *          fsrv_DS_GetChargeBatStatus(), it guarantees that any threshold change
 *          (e.g., transition into or out of BAT_CRITICAL) is caught synchronously
 *          without duplicating condition checks (DRY principle).
 *
 * @note [Low-Power & Execution Design Notes]:
 *       1. Single-cycle evaluation: The function executes minimal ARM Assembly
 *          instructions on Cortex-M33, utilizing an atomic 16-bit/32-bit integer
 *          comparison. This is crucial for frequent execution loops in HMI state
 *          machines to preserve low-power states (EM1/EM2 Sleep).
 *       2. RAM Footprint: Utilizes exactly 1 byte (or equivalent of img_storage_id_t)
 *          in the .data/.bss segment, completely avoiding dynamic allocation.
 *       3. Initial Sync: Initialized with IMG_INVALID_ID. This guarantees that
 *          the very first call triggers a 'TRUE' return if the battery starts in a
 *          critical state, forcing a necessary initial UI rendering.
 *
 * @return Bool TRUE if the battery state image ID changed, FALSE otherwise.
 ============================================================================================*/
Bool fsrv_DS_IsChargeBatStatusUpdate(void)
{
  /* Retain the previous state's image ID. Initialized to IMG_INVALID_ID
     to force synchronization on the initial evaluation loop. */
  static img_storage_id_t last_storage_id = IMG_INVALID_ID;

  // Retrieve the current evaluation of the battery image ID
  img_storage_id_t current_storage_id = fsrv_DS_GetBatStatusImageId();

  if (current_storage_id != last_storage_id) {
    // Latch the updated status to the persistent static mirror
    last_storage_id = current_storage_id;
    return TRUE;
  }

  return FALSE;
}
/*=======================================================================*/
/*                                                                       */
/*=======================================================================*/
img_storage_id_t fsrv_DS_GetBleStatusImageId(void)
{
  if (fsrv_display_datastore.ble_status == BLE_CONNECTED) {
    return IMG_ID_PROPERTY_1_LINK;    //Connected icon
  }
  return IMG_INVALID_ID;
}

/*=======================================================================*/
/*BLE_ DISCONNECTED = 0x00,
  BLE_ ADVERTISING,
  BLE_ PAIRING,
  BLE_ CONNECTED,
  BLE_ NOT_PAIRING,
  BLE_ ERROR                                                           */
/*=======================================================================*/
img_storage_id_t fsrv_DS_GetPairingStatusImageId(void)
{
if (    ( fsrv_display_datastore.ble_status == BLE_PAIRING  )
     || ( fsrv_display_datastore.ble_status == BLE_CONNECTED )
   )
  {
    return  IMG_ID_CHECK_CIRCLE;                  //Pairing Ok

  } else  {

    return IMG_ID_PROPERTY_1_VARIANT3_1;          //Failed
  }

  return IMG_INVALID_ID;
}

/*=======================================================================*/
/*BLE_ DISCONNECTED = 0x00,// Failed
  BLE_ ADVERTISING,        // Waiting
  BLE_ PAIRING,            // Numeric display
  BLE_ CONNECTED,          // Ok
  BLE_ NOT_PAIRING,        // Failed
  BLE_ ERROR               // Failed
Pairing menu */
/*=======================================================================*/
img_storage_id_t fsrv_DS_GetPairingWaitingStatusImageId(void)
{
  if (   ( fsrv_display_datastore.ble_status == BLE_ADVERTISING )
      || ( fsrv_display_datastore.ble_status == BLE_DISCONNECTED )
      || ( fsrv_display_datastore.ble_status == BLE_NOT_PAIRING )
      )
  { //Pairing Waiting
    return IMG_ID_PROPERTY_1_VARIANT14;   //Waiting...
  }

  return IMG_INVALID_ID;
}

/*=======================================================================*/
/*                                                                       */
/*=======================================================================*/
img_storage_id_t fsrv_DS_GetSyncStatusImageId(void) {

  if (fsrv_display_datastore.sync_status  == SYNC_IN_PROGRESS) {
      return IMG_ID_PROPERTY_1_ARROW_PATH;
  }
  return IMG_INVALID_ID;
}

/*=======================================================================*/
/*                                                                       */
/*=======================================================================*/
uint32_t fsrv_DS_GetRefNumber(void) {
    return fsrv_display_datastore.ref_number;
}

/*=======================================================================*/
/*                                                                       */
/*=======================================================================*/
float fsrv_DS_GetCalibrationConst(void)
{
  return fsrv_display_datastore.calibration_const*100;
}

/*=======================================================================*/
/*                                                                       */
/*=======================================================================*/
bool fsrv_DS_GetStrainGauseStat(void) {

  if (fsrv_display_datastore.strain_gause_stat == FSRV_GAUGE_STATUS_GOOD) {
   return FSRV_GAUGE_STATUS_GOOD;
  }
   return FSRV_GAUGE_STATUS_BAD;
}

/*=======================================================================*/
/*BLE_ DISCONNECTED = 0x00,
  BLE_ ADVERTISING,
  BLE_ PAIRING,
  BLE_ CONNECTED,
  BLE_ NOT_PAIRING,
  BLE_ ERROR                                                           */
/*=======================================================================*/
/*=======================================================================*/
/*                                                                       */
/*=======================================================================*/
img_storage_id_t fsrv_DS_GetIdleWaitingStatusImageId(void) {

  img_storage_id_t storage_id = IMG_INVALID_ID;

  st_ble_connect_t ble_status = fsrv_display_datastore.ble_status;
  Bool    ble_bondings_status = fsrv_display_datastore.ble_bondings_status;
 

  if (   ( BLE_BOUNDINGS_CLEAR == ble_bondings_status)
     )
  {
     storage_id = IMG_ID_PROPERTY_1_VARIANT13; //Not Paired

  } else if ( BLE_CONNECTED  == ble_status ) {
          storage_id = IMG_ID_PROPERTY_1_VARIANT2_7; //Waiting for TIP ID
  } else  {
      storage_id = IMG_ID_PROPERTY_1_DEFAULT_7; //Waiting for connect
  }

  return storage_id;
}


/*=============================================================================
 * @brief Checks for updates in the Idle waiting state machinery.
 *
 * @details Evaluates changes in state images driven by the BLE bonding status
 *          and connection lifecycle. It uses a static state mirror to detect
 *          transitions, ensuring synchronization with the rendering pipeline.
 *
 * @return Bool TRUE if the status changed since the last invocation, FALSE otherwise.
 ==============================================================================*/
Bool fsrv_DS_IsIdleWaitingStatUpdate(void)
{
  /* Initialized to an invalid marker to enforce a mandatory UI refresh
     and state synchronization during the first execution loop. */
  static img_storage_id_t last_storage_id = IMG_INVALID_ID;

  // Sample current state machine output
  img_storage_id_t current_storage_id = fsrv_DS_GetIdleWaitingStatusImageId();

  if (current_storage_id != last_storage_id) {
    // Pipeline update detected: latch new state and signal the UI thread
    last_storage_id = current_storage_id;
    return TRUE;
  }

  return FALSE;
}

/*=======================================================================*/
/*                                                                       */
/*=======================================================================*/
/*img_storage_id_t fsrv_DS_GetPerformanceStart(void)
{
  img_storage_id_t storage_id = IMG_INVALID_ID;
  st_measure_t status = fsrv_display_datastore.measurement_status;
  measure_mode_t mode = fsrv_display_datastore.measurement_mode;
  
  if ( mode == MODE_PERFORMANCE ) {
    if ( status == MEAS_START ) {
      storage_id = IMG_ID_PROPERTY_1_VARIANT9;   //PERFORMANCE
    }
  } 
  return storage_id;
}
*/
/*=======================================================================*/
/*                                                                       */
/*=======================================================================*/
img_storage_id_t fsrv_DS_GetPatientCompleteResultImageId(void) {
  img_storage_id_t storage_id = IMG_INVALID_ID;

  if (fsrv_is_session_patient_bad() == FALSE) {
    storage_id = IMG_ID_CHECK_CIRCLE;
  } else {
    storage_id = IMG_ID_X_CIRCLE;
  }

  return storage_id;
}

/*=======================================================================*/
/*                                                                       */
/*=======================================================================*/
img_storage_id_t fsrv_DS_GetPerformanceCompleteResultImageId(void) {
  img_storage_id_t storage_id = IMG_INVALID_ID;

  if (fsrv_is_session_performance_bad() == FALSE) {
     storage_id = IMG_ID_CHECK_CIRCLE;
  } else {
    storage_id = IMG_ID_X_CIRCLE;
  }

  return storage_id;
}

/*=======================================================================*/
/*                                                                       */
/*=======================================================================*/
img_storage_id_t fsrv_DS_GetReferenceCompleteResultImageId(void) {
 img_storage_id_t storage_id = IMG_INVALID_ID;

 if (fsrv_is_session_reference_bad() == FALSE) {
    storage_id = IMG_ID_CHECK_CIRCLE;
 } else {
   storage_id = IMG_ID_X_CIRCLE;
 }

 return storage_id;
}

/*=======================================================================*/
/*                                                                       */
/*=======================================================================*/
img_storage_id_t fsrv_DS_GetPerformanceResultImageId(void) {
  img_storage_id_t storage_id = IMG_INVALID_ID;
 // st_measure_t status = fsrv_display_datastore.measurement_status;
 // measure_mode_t mode = fsrv_display_datastore.measurement_mode;

  if(fsrv_DS_getPerformanceFinalResult() == TRUE) {
     storage_id = IMG_ID_PROPERTY_1_VARIANT2_1;
  } else {
     storage_id = IMG_ID_PROPERTY_1_VARIANT3_1;
  }

  return storage_id;
}

/*=======================================================================*/
/*                                                                       */
/*=======================================================================*/
float fsrv_DS_GetPatientResult(void) {
  return (fsrv_display_datastore.bone_score*10);
}

/*=======================================================================*/
/*                              APPROXIMATION                            */
/*=======================================================================*/
img_storage_id_t fsrv_DS_GetPatientApproximationImageId(void) {
  img_storage_id_t storage_id = IMG_INVALID_ID;

  if ( fsrv_display_datastore.is_bone_score_approximate == TRUE ) {
      storage_id = IMG_ID_A;   // A  - Approximate
   }

  return storage_id;
}

/*=======================================================================*/
/*                                                                       */
/*=======================================================================*/
/*img_storage_id_t fsrv_DS_GetPatientNotificationStatus(void)
{
  img_storage_id_t storage_id = IMG_INVALID_ID;
  st_measure_t status = fsrv_display_datastore.measurement_status;
  measure_mode_t mode = fsrv_display_datastore.measurement_mode;

  
  if ( mode == MODE_PATIENT ) {
    if ( status == MEAS_START ) {
      storage_id = IMG_ID_PROPERTY_1_VARIANT7;   //PATIENT
    }
    if ( status == MEAS_INDENT_INCONSIST ) {
      storage_id = IMG_ID_PROPERTY_1_VARIANT5;  // + 1 INDENT REQUIRED
    }

//    if ( status == MEAS_FAILED ) {
//      storage_id = IMG_ID_PROPERTY_1_VARIANT3_1;  // FAILED
//    }

    if ( status == MEAS_UNSTABLE ) {
      storage_id = IMG_ID_PROPERTY_1_VARIANT6;  // UNSTABLE
    }
  } 

  return storage_id;
}
*/


/*=======================================================================*/
/*                                                                       */
/*=======================================================================*/
/*img_storage_id_t fsrv_DS_GetPatientStart(void)
{
  img_storage_id_t storage_id = IMG_INVALID_ID;
  st_measure_t status = fsrv_display_datastore.measurement_status;
  measure_mode_t mode = fsrv_display_datastore.measurement_mode;
  
  if ( mode == MODE_PATIENT ) {
    if ( status == MEAS_IDLE ) {
      storage_id = IMG_ID_PROPERTY_1_VARIANT7;   //PATIENT
    }
  } 

  return storage_id;
}
*/
/*=======================================================================*/
/*                                                                       */
/*=======================================================================*/
uint16_t fsrv_DS_GetStrainGaugeValue(void) {
    return (fsrv_display_datastore.strain_gauge_value == FSRV_GAUGE_STATUS_GOOD);
}

uint32_t fsrv_DS_GetValidIndentations(void) {
    return fsrv_display_datastore.valid_indentations;
}

st_measure_t fsrv_DS_GetMeasurementStatus(void) {
    return fsrv_display_datastore.measurement_status;
}

measure_mode_t fsrv_DS_GetMeasurementMode(void) {
    return fsrv_display_datastore.measurement_mode;
}

st_tip_id_t fsrv_DS_GetTipIdStatus(void) {
    return fsrv_display_datastore.tip_id_stat;
}

float fsrv_DS_GetBoneScore(void) {
    return fsrv_display_datastore.bone_score;
}

err_code_t fsrv_DS_GetErrorCode(void) {
    return fsrv_display_datastore.error_code;
}

uint32_t fsrv_DS_GetBlePairingCode(void) {
    return (uint32_t)fsrv_display_datastore.ble_pairing_code;
}

st_battery_t fsrv_DS_GetBatStatus(void) {
  return fsrv_display_datastore.bat_status;
}

/* ================= SET FUNCTIONS IMPLEMENTATION ================= */
/*=======================================================================*/
/*                                                                       */
/*=======================================================================*/

void fsrv_DS_SetFwVersion(const char *ptr) {
    fsrv_display_datastore.fw_version = ptr;
}

void fsrv_DS_SetSerialNum(uint32_t num) {
    fsrv_display_datastore.serial_num = num;
}

void fsrv_DS_SetBatStatus(st_battery_t status) {
    fsrv_display_datastore.bat_status = status;
}

void fsrv_DS_SetBleStatus(st_ble_connect_t status) {
    fsrv_display_datastore.ble_status = status;
}

void fsrv_BLE_SetBoundingStatus(Bool stat) {
  fsrv_display_datastore.ble_bondings_status = stat;
}

void fsrv_DS_SetBlePairingCode(uint32_t code) {
    fsrv_display_datastore.ble_pairing_code = code;
}

void fsrv_DS_SetSyncStatus(st_sync_t status) {
    fsrv_display_datastore.sync_status = status;
}

void fsrv_DS_SetRefNumber(uint32_t num) {
    fsrv_display_datastore.ref_number = num;
}

void fsrv_DS_SetStrainGauseStat(Bool stat) {
  fsrv_display_datastore.strain_gause_stat = ( ( stat == TRUE ) ? FSRV_GAUGE_STATUS_GOOD : FSRV_GAUGE_STATUS_BAD );
}

void fsrv_DS_SetStrainGaugeValue(uint16_t val) {
    fsrv_display_datastore.strain_gauge_value = val;
}

void fsrv_DS_SetValidIndentations(uint32_t val) {
    fsrv_display_datastore.valid_indentations = val;
}

void fsrv_DS_SetMeasurementStatus(st_measure_t status) {
    fsrv_display_datastore.measurement_status = status;
}

void fsrv_DS_SetMeasurementMode(measure_mode_t mode) {
    fsrv_display_datastore.measurement_mode = mode;
}

void fsrv_DS_SetTipIdStat(st_tip_id_t stat) {
    fsrv_display_datastore.tip_id_stat = stat;
}

void fsrv_DS_SetBoneScore(float score) {
    fsrv_display_datastore.bone_score = score;
}

void fsrv_DS_SetIsBoneApproximateScore(Bool approximate) {
  fsrv_display_datastore.is_bone_score_approximate = approximate;
}

void fsrv_DS_SetErrorCode(err_code_t code) {
    fsrv_display_datastore.error_code = code;
}

void fsrv_DS_SetCalibrationConst(float val) {
   fsrv_display_datastore.calibration_const = val;
}
