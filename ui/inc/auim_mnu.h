/*
 * auim_mnu.h
 *
 *      Author: priss
 */

#ifndef UI_INC_AUIM_MNU_H_
#define UI_INC_AUIM_MNU_H_

/*=======================================================================*/
/*        I N C L U D E S                                                */
/*=======================================================================*/

/*==========================================================================*/
/* G L O B A L   R E F E R E N C E S                                        */
/*==========================================================================*/

struct fmnu_MenuStruct;

/*=========================================================================*/
/*   G L O B A L   D E F I N I T I O N S                                   */
/*=========================================================================*/

/*==========================================================================*/
/*        G L O B A L   D A T A   D E C L A R A T I O N S                   */
/*==========================================================================*/

typedef enum {

  AUIM_MNU_INDEX_CONFIG_MENU     = 0
 ,AUIM_MNU_INDEX_IDLE_MENU

 ,AUIM_MNU_INDEX_PAIRING_MENU
 ,AUIM_MNU_INDEX_PAIRING_STATUS_MENU
 ,AUIM_MNU_INDEX_PAIRING_CLOSE_MENU

 ,AUIM_MNU_INDEX_PERFORMANCE_MENU
 ,AUIM_MNU_INDEX_PERFORMANCE_CONTINUE_MENU
 ,AUIM_MNU_INDEX_PERFORMANCE_CANCELED_MENU
 ,AUIM_MNU_INDEX_PERFORMANCE_COMPLETE_MENU
 ,AUIM_MNU_INDEX_PERFORMANCE_TERMINATE_MENU
 ,AUIM_MNU_INDEX_PERFORMANCE_REQUIRED_MENU
 ,AUIM_MNU_INDEX_PERFORMANCE_RESULT_MENU


 ,AUIM_MNU_INDEX_REFERENCE_MENU
 ,AUIM_MNU_INDEX_REFERENCE_CONTINUE_MENU
 ,AUIM_MNU_INDEX_REFERENCE_PERFORMANCE_CANCELED_MENU
 ,AUIM_MNU_INDEX_REFERENCE_PATIENT_CANCELED_MENU
 ,AUIM_MNU_INDEX_REFERENCE_COMPLETE_MENU
 ,AUIM_MNU_INDEX_REFERENCE_TERMINATE_MENU
 ,AUIM_MNU_INDEX_REFERENCE_PERFORMANCE_UNSTABLE_MENU
 ,AUIM_MNU_INDEX_REFERENCE_PATIENT_UNSTABLE_MENU

 // ,AUIM_MNU_INDEX_REFERENCE_FATIENT_UNSTABLE_MENU

 ,AUIM_MNU_INDEX_PATIENT_MENU
 ,AUIM_MNU_INDEX_PATIENT_CONTINUE_MENU
 ,AUIM_MNU_INDEX_PATIENT_CANCELED_MENU
 ,AUIM_MNU_INDEX_PATIENT_COMPLETE_MENU
 ,AUIM_MNU_INDEX_PATIENT_TERMINATE_MENU
 ,AUIM_MNU_INDEX_PATIENT_RESULT_MENU
 ,AUIM_MNU_INDEX_PATIENT_FAILED_MENU


 ,AUIM_MNU_INDEX_PATIENT_VALIDATING_MENU
 ,AUIM_MNU_INDEX_PATIENT_VALIDATING_RESULT_MENU

 ,AUIM_MNU_INDEX_CHARGE_BATTERY_MENU
 
 ,AUIM_MNU_MAX_MENUS

} menu_index_enum;


/*=======================================================================*/
/*          C O N F I G U R A T I O N         F I E L D S                */
/*=======================================================================*/
enum config_menu_fields_index_enum  {
  AUIM_MNU_CONFIG_FIELD_SERIAL_NUMBER = 0
 ,AUIM_MNU_CONFIG_FIELD_FIRMWARE_VERSION
 ,AUIM_MNU_CONFIG_FIELD_CURRENT_TIME
 ,AUIM_MNU_CONFIG_FIELD_CURRENT_DATE
 ,AUIM_MNU_CONFIG_FIELD_REFERENCE_NUMBER
 ,AUIM_MNU_CONFIG_FIELD_STRAIN_GAUGE_STATUS
 ,AUIM_MNU_CONFIG_FIELD_CALIBRATION_CONSTANT

 ,AUIM_MNU_CONFIG_MAX_FIELD
};

/*=======================================================================*/
/*          I D L E         F I E L D S                      */
/*=======================================================================*/
enum idle_menu_fields_index_enum  {
   AUIM_MNU_IDLE_FIELD_STATUS = 0
  ,AUIM_MNU_IDLE_MAX_FIELD
};

/*=======================================================================*/
/*         P A I R I N G          F I E L D S                 */
/*=======================================================================*/
enum pairing_menu_fields_index_enum  {
   AUIM_MNU_PAIRING_FIELD_CODE = 0
  ,AUIM_MNU_PAIRING_MAX_FIELD
};

enum pairing_cancel_menu_fields_index_enum  {
   AUIM_MNU_PAIRING_CANCEL_FIELD = 0
  ,AUIM_MNU_PAIRING_CANCEL_MAX_FIELD
};

enum pairing_close_menu_fields_index_enum  {
   AUIM_MNU_PAIRING_CLOSE_FIELD = 0
  ,AUIM_MNU_PAIRING_CLOSE_MAX_FIELD
};

/*=======================================================================*/
/*     R E F E R E N C E      F I E L D S                             */
/*=======================================================================*/
enum reference_menu_fields_index_enum  {
   AUIM_MNU_REFERENCE_FIELD = 0
  ,AUIM_MNU_REFERENCE_MAX_FIELD
};

enum reference_continue_menu_fields_index_enum  {
   AUIM_MNU_REFERENCE_CONTINUE_FIELD = 0
  ,AUIM_MNU_REFERENCE_CONTINUE_MAX_FIELD
};

enum reference_canceled_performance_menu_fields_index_enum  {
  AUIM_MNU_REFERENCE_CANCELED_PERFORMANCE_FIELD = 0
  ,AUIM_MNU_REFERENCE_CANCELED_PERFORMANCE_MAX_FIELD
};

enum reference_canceled_patient_menu_fields_index_enum  {
  AUIM_MNU_REFERENCE_CANCELED_PATIENT_FIELD = 0
  ,AUIM_MNU_REFERENCE_CANCELED_PATIENT_MAX_FIELD
};

enum reference_complete_menu_fields_index_enum  {
   AUIM_MNU_REFERENCE_COMPLETE_FIELD = 0
  ,AUIM_MNU_REFERENCE_COMPLETE_MAX_FIELD
};

enum reference_terminate_menu_fields_index_enum  {
  AUIM_MNU_REFERENCE_TERMINATE_FIELD = 0
  ,AUIM_MNU_REFERENCE_TERMINATE_MAX_FIELD
};

enum reference_unstable_performance_menu_fields_index_enum  {
   AUIM_MNU_REFERENCE_UNSTABLE_PERFORMANCE_FIELD = 0
  ,AUIM_MNU_REFERENCE_UNSTABLE_PERFORMANCE_MAX_FIELD
};

enum reference_unstable_patient_menu_fields_index_enum  {
   AUIM_MNU_REFERENCE_UNSTABLE_PATIENT_FIELD = 0
  ,AUIM_MNU_REFERENCE_UNSTABLE_PATIENT_MAX_FIELD
};

/*=======================================================================*/
/*     P E R F O R M A N C E     F I E L D S                             */
/*=======================================================================*/
enum performance_menu_fields_index_enum  {
   AUIM_MNU_PERFORMANCE_FIELD = 0
  ,AUIM_MNU_PERFORMANCE_MAX_FIELD
};

enum performance_continue_menu_fields_index_enum  {
  AUIM_MNU_PERFORMANCE_CONTINUE_FIELD = 0
  ,AUIM_MNU_PERFORMANCE_CONTINUE_MAX_FIELD
};

enum performance_canceled_menu_fields_index_enum  {
  AUIM_MNU_PERFORMANCE_CANCELED_FIELD = 0
  ,AUIM_MNU_PERFORMANCE_CANCELED_MAX_FIELD
};

enum performance_complete_menu_fields_index_enum  {
  AUIM_MNU_PERFORMANCE_COMPLETE_FIELD = 0
  ,AUIM_MNU_PERFORMANCE_COMPLETE_MAX_FIELD
};

enum performance_terminate_menu_fields_index_enum  {
  AUIM_MNU_PERFORMANCE_TERMINATE_FIELD = 0
  ,AUIM_MNU_PERFORMANCE_TERMINATE_MAX_FIELD
};

enum performance_required_menu_fields_index_enum  {
   AUIM_MNU_PERFORMANCE_REQUIRED_FIELD = 0
  ,AUIM_MNU_PERFORMANCE_REQUIRED_MAX_FIELD
};

enum performance_result_menu_fields_index_enum  {
  AUIM_MNU_PERFORMANCE_RESULT_FIELD = 0
  ,AUIM_MNU_PERFORMANCE_RESULT_MAX_FIELD
};

/*=======================================================================*/
/*        P A T I E N T     F I E L D S                             */
/*=======================================================================*/
enum patient_menu_fields_index_enum  {
  AUIM_MNU_PATIENT_FIELD = 0
  ,AUIM_MNU_PATIENT_MAX_FIELD
};

enum patient_continue_menu_fields_index_enum  {
  AUIM_MNU_PATIENT_CONTINUE_FIELD = 0
  ,AUIM_MNU_PATIENT_CONTINUE_MAX_FIELD
};

enum patient_canceled_menu_fields_index_enum  {
  AUIM_MNU_PATIENT_CANCELED_FIELD = 0
  ,AUIM_MNU_PATIENT_CANCELED_MAX_FIELD
};

enum patient_complete_menu_fields_index_enum  {
  AUIM_MNU_PATIENT_COMPLETE_FIELD = 0
  ,AUIM_MNU_PATIENT_COMPLETE_MAX_FIELD
};

enum patient_terminate_menu_fields_index_enum  {
  AUIM_MNU_PATIENT_TERMINATE_FIELD = 0
  ,AUIM_MNU_PATIENT_TERMINATE_MAX_FIELD
};

enum patient_result_menu_fields_index_enum  {
  AUIM_MNU_PATIENT_RESULT_FIELD = 0
  ,AUIM_MNU_PATIENT_RESULT_MAX_FIELD
};

enum patient_failed_result_menu_fields_index_enum  {
  AUIM_MNU_PATIENT_FAILED_FIELD = 0
  ,AUIM_MNU_PATIENT_FAILED_MAX_FIELD
};



enum patient_validating_menu_fields_index_enum  {
  AUIM_MNU_PATIENT_VALIDATING_FIELD = 0
  ,AUIM_MNU_PATIENT_VALIDATING_MAX_FIELD
};

enum patient_validating_close_menu_fields_index_enum  {
  AUIM_MNU_PATIENT_VALIDATING_CLOSE_FIELD = 0
  ,AUIM_MNU_PATIENT_VALIDATING_RESULT_MAX_FIELD
};

/*=======================================================================*/
/*    C H A R G E   T H E   B A T T E R Y          F I E L D S           */
/*=======================================================================*/
enum charge_battery_menu_fields_index_enum  {
  AUIM_MNU_CHARGE_BATTERY_FIELD = 0
 ,AUIM_MNU_CHARGE_BATTERY_MAX_FIELD
};

typedef enum {
   AUIM_MNU_PERFORMANCE_TERMINATE_FIXED_FIELD = 0
  ,AUIM_MNU_BOTTON_FIXED_FIELDS

  ,AUIM_MNU_MAX_FIXED_FIELDS
}menu_fixed_fields_index_enum;

typedef enum {
   AUIM_PERF_CHK_BUTTON_FIELD = 0
  ,AUIM_PARAMS_BUTTON_FIELD
  ,AUIM_NEXT_BUTTON_FIELD
  ,AUIM_START_BUTTON_FIELD
  ,AUIM_CANCEL_BUTTON_FIELD
  ,AUIM_NO_BUTTON_FIELD
  ,AUIM_YES_BUTTON_FIELD
  ,AUIM_CLOSE_BUTTON_FIELD
  ,AUIM_REPEAT_REF_BUTTON_FIELD
  ,AUIM_SKIP_BUTTON_FIELD

  ,AUIM_MNU_MAX_BUTTON_FIELDS
}menu_buttons_index_enum;



#endif /* UI_INC_AUIM_MNU_H_ */
