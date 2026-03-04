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

 fsrv_display_datastore.fw_version = "TEST";
 fsrv_display_datastore.serial_num = 1234;
 fsrv_display_datastore.bat_status = BAT_NORMAL;
 fsrv_display_datastore.ble_status = BLE_DISCONNECTED;
 fsrv_display_datastore.sync_status = SYNC_IDLE;

     // strain gauge parameters

 fsrv_display_datastore.ref_number = 987;
 fsrv_display_datastore.strain_gause_stat = STATUS_GOOD;  // good/bad
 fsrv_display_datastore.strain_gauge_value = 111; // current value

     // for measurement mode
 fsrv_display_datastore.required_indentations = 8;
 fsrv_display_datastore.performed_indentations = 8;
 fsrv_display_datastore. valid_indentations = 4;

 fsrv_display_datastore.measurement_status = MEAS_IDLE;
 fsrv_display_datastore.measurement_mode = MODE_NONE;
 fsrv_display_datastore.tip_id_stat = INVALID_TIP_ID;           //Valid \ Invalid  Tip ID


     // Results
 fsrv_display_datastore.bone_score = 1.3;
 fsrv_display_datastore.user_score_patient_stddev = 0.1;
 fsrv_display_datastore.user_score_reference_stddev = 0.2;
       // ...
 fsrv_display_datastore.error_code = ERR_NONE;

}

/* ================= GET FUNCTIONS IMPLEMENTATION ================= */

const char* fsrv_DS_GetFwVersion(void) {
    return fsrv_display_datastore.fw_version;
}

uint32_t fsrv_DS_GetSerialNum(void) {
    return fsrv_display_datastore.serial_num;
}

st_battery_t fsrv_DS_GetBatStatus(void) {
    return fsrv_display_datastore.bat_status;
}

st_ble_connect_t fsrv_DS_GetBleStatus(void) {
    return fsrv_display_datastore.ble_status;
}

st_sync_t fsrv_DS_GetSyncStatus(void) {
    return fsrv_display_datastore.sync_status;
}

uint32_t fsrv_DS_GetRefNumber(void) {
    return fsrv_display_datastore.ref_number;
}

bool fsrv_DS_GetStrainGauseStat(void) {
    return fsrv_display_datastore.strain_gause_stat;
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

bool fsrv_DS_GetTipIdStat(void) {
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

/* ================= SET FUNCTIONS IMPLEMENTATION ================= */

void fsrv_DS_SetFwVersion(const char* ver) {
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

void fsrv_DS_SetTipIdStat(bool stat) {
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
