/*=======================================================================*/
/*        I N C L U D E S                                                */
/*=======================================================================*/

#include "fsrv.h"


/*=======================================================================*/
/* L O C A L   F U N C T I O N   P R O T O T Y P E S                     */
/*=======================================================================*/


/*===========================================================================*/
/*         G L O B A L   D A T A   D E C L A R A T I O N S                   */
/*===========================================================================*/
fsrv_display_datastore_t fsrv_display_datastore = {0};



/*==========================================================================*/
/* G L O B A L      F U N C T I O N                                         */
/*==========================================================================*/
void fsrv_Init(void)
{

 fsrv_display_datastore.fw_version = 01;
 fsrv_display_datastore.serial_num = 12;
 fsrv_display_datastore.bat_status = BAT_NORMAL;
 fsrv_display_datastore.ble_status = BLE_DISCONNECTED;
 fsrv_display_datastore.sync_status = SYNC_IDLE;


 fsrv_display_datastore.calibration_const = 3;

 // strain gauge parameters
 fsrv_display_datastore.ref_number = 98;
 fsrv_display_datastore.strain_gause_stat = FSRV_GAUGE_STATUS_GOOD;  // good/bad
 fsrv_display_datastore.strain_gauge_value = 11; // current value

     // for measurement mode
 fsrv_display_datastore.required_indentations = 8;
 fsrv_display_datastore.performed_indentations = 8;
 fsrv_display_datastore. valid_indentations = 4;

 fsrv_display_datastore.measurement_status = MEAS_IDLE;
 fsrv_display_datastore.measurement_mode = MODE_NONE;
 fsrv_display_datastore.tip_id_stat = TIP_ID_WAITING;           // Waiting - \ - Valid - \ Invalid - Tip ID


     // Results
 fsrv_display_datastore.bone_score = 1.3;
 fsrv_display_datastore.user_score_patient_stddev = 0.1;
 fsrv_display_datastore.user_score_reference_stddev = 0.2;
       // ...
 fsrv_display_datastore.error_code = ERR_NONE;

}

/* ================= GET FUNCTIONS IMPLEMENTATION ================= */

uint32_t fsrv_DS_GetFwVersion(void) {
    return fsrv_display_datastore.fw_version;
}

uint32_t fsrv_DS_GetSerialNum(void) {
    return fsrv_display_datastore.serial_num;
}

img_storage_id_t fsrv_DS_GetBatStatus(void) {

  img_storage_id_t img_id = IMG_MAX_IDS_STORAGE_DESC_COUNT;

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
      img_id = IMG_MAX_IDS_STORAGE_DESC_COUNT;
      break;
  }

    return  img_id;
}

img_storage_id_t fsrv_DS_GetChargeBatStatus(void)
{

  if (fsrv_display_datastore.bat_status == BAT_CRITICAL) {
     return IMG_ID_PROPERTY_1_VARIANT3;
  }

  return IMG_MAX_IDS_STORAGE_DESC_COUNT;
}

img_storage_id_t fsrv_DS_GetBleStatus(void)
{
  if (fsrv_display_datastore.ble_status == BLE_CONNECTED) {
    return IMG_ID_PROPERTY_1_LINK;
  }

  return IMG_MAX_IDS_STORAGE_DESC_COUNT;
}

img_storage_id_t fsrv_DS_GetSyncStatus(void) {

  if (fsrv_display_datastore.sync_status  == SYNC_COMPLETED) {
      return IMG_ID_PROPERTY_1_ARROW_PATH;
  }
  return IMG_MAX_IDS_STORAGE_DESC_COUNT;
}

uint32_t fsrv_DS_GetRefNumber(void) {
    return fsrv_display_datastore.ref_number;
}

float fsrv_DS_GetCalibrationConst(void)
{
  return fsrv_display_datastore.calibration_const;
}

bool fsrv_DS_GetStrainGauseStat(void) {

  if (fsrv_display_datastore.strain_gause_stat == FSRV_GAUGE_STATUS_GOOD) {
   return FSRV_GAUGE_STATUS_GOOD;
  }
   return FSRV_GAUGE_STATUS_BAD;
}

img_storage_id_t fsrv_DS_GetWaitingStat(void) {

  st_ble_connect_t ble_status  = fsrv_display_datastore.ble_status;
  st_tip_id_t tip_id_stat = fsrv_display_datastore.tip_id_stat;
  img_storage_id_t storage_id = IMG_MAX_IDS_STORAGE_DESC_COUNT;
 

  if (   (BLE_NOT_PAIRING == ble_status)
      || (BLE_ERROR == ble_status)
     )
  {
     storage_id = IMG_ID_PROPERTY_1_VARIANT13; //Not Paired
  } else if (  (BLE_DISCONNECTED == ble_status)
            || (BLE_ADVERTISING  == ble_status)
            || (BLE_PAIRING      == ble_status)
           )
  {
      storage_id = IMG_ID_PROPERTY_1_DEFAULT_7; //Waiting for connect
  } else if(BLE_CONNECTED  == ble_status) {
    if (TIP_ID_WAITING == tip_id_stat) {
      storage_id = IMG_ID_PROPERTY_1_VARIANT2_7; //Waiting for TIP ID
    } else {
      storage_id = IMG_ID_PROPERTY_1_VARIANT5_6; //Validating
    }
  }

  return storage_id;
}

uint16_t fsrv_DS_GetStrainGaugeValue(void) {
    return fsrv_display_datastore.strain_gauge_value;
}

uint32_t fsrv_DS_GetRequiredIndentations(void) {
    return fsrv_display_datastore.required_indentations;
}

uint32_t fsrv_DS_GetPerformedIndentations(void) {
    return fsrv_display_datastore.performed_indentations;
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

st_tip_id_t fsrv_DS_GetTipIdStat(void) {
    return fsrv_display_datastore.tip_id_stat;
}

float fsrv_DS_GetBoneScore(void) {
    return fsrv_display_datastore.bone_score;
}

float fsrv_DS_GetUserScorePatientStddev(void) {
    return fsrv_display_datastore.user_score_patient_stddev;
}

float fsrv_DS_GetUserScoreReferenceStddev(void) {
    return fsrv_display_datastore.user_score_reference_stddev;
}

err_code_t fsrv_DS_GetErrorCode(void) {
    return fsrv_display_datastore.error_code;
}

uint16_t fsrv_GetPairingCode(void) {
    return (uint16_t)1911;
}


/* ================= SET FUNCTIONS IMPLEMENTATION ================= */

void fsrv_DS_SetFwVersion(uint32_t ver) {
    fsrv_display_datastore.fw_version = ver;
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

void fsrv_DS_SetSyncStatus(st_sync_t status) {
    fsrv_display_datastore.sync_status = status;
}

void fsrv_DS_SetRefNumber(uint32_t num) {
    fsrv_display_datastore.ref_number = num;
}

void fsrv_DS_SetStrainGauseStat(bool stat) {
    fsrv_display_datastore.strain_gause_stat = stat;
}

void fsrv_DS_SetStrainGaugeValue(uint16_t val) {
    fsrv_display_datastore.strain_gauge_value = val;
}

void fsrv_DS_SetRequiredIndentations(uint32_t val) {
    fsrv_display_datastore.required_indentations = val;
}

void fsrv_DS_SetPerformedIndentations(uint32_t val) {
    fsrv_display_datastore.performed_indentations = val;
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

void fsrv_DS_SetUserScorePatientStddev(float val) {
    fsrv_display_datastore.user_score_patient_stddev = val;
}

void fsrv_DS_SetUserScoreReferenceStddev(float val) {
    fsrv_display_datastore.user_score_reference_stddev = val;
}

void fsrv_DS_SetErrorCode(err_code_t code) {
    fsrv_display_datastore.error_code = code;
}
