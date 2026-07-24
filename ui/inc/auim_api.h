/*
 * auim_api.h
 *
 *  Created on: 20-02-2026.
 *      Author: priss
 */

#ifndef UI_INC_AUIM_API_H_
#define UI_INC_AUIM_API_H_

/*=======================================================================*/
/*        I N C L U D E S                                                */
/*=======================================================================*/

/*=========================================================================*/
/*   G L O B A L   D E F I N I T I O N S                                   */
/*=========================================================================*/
#define AUIM_NO_ATTRIBUTES  0


/*==========================================================================*/
/*        G L O B A L   D A T A   D E C L A R A T I O N S                   */
/*==========================================================================*/

enum colour_index_enum {
  AUIM_MENU_TITLE_COLOUR
 ,AUIM_MENU_FIELD_COLOUR
 ,AUIM_MENU_SPLIT_FIELD_COLOUR
 ,AUIM_MENU_DOUBLE_FIELD_COLOUR
 ,AUIM_BUTTON_FIELD_COLOUR
 ,AUIM_INDICATOR_COLOUR
 ,AUIM_BLANK_INDICATOR_COLOUR
 ,AUIM_MODAL_INDICATOR_COLOUR
 ,AUIM_SPLASH_SCREEN_COLOUR
 ,AUIM_PERFORMANCE_NOTIFICATION_COLOUR
 ,AUIM_PATIENT_NOTIFICATION_COLOUR
 ,AUIM_REFERENCE_NOTIFICATION_COLOUR
 ,AUIM_TIP_ID_RED_MODAL_COLOUR
 ,AUIM_TIP_ID_GREEN_MODAL_COLOUR
 ,AUIM_ERROR_INDICATOR_COLOUR

};

enum indicator_index_enum {

   AUIM_INDEX_BATTERY_INDICATOR = 0
  ,AUIM_INDEX_BLE_INDICATOR
  ,AUIM_INDEX_SYNC_INDICATOR
  ,AUIM_INDEX_SPLASH_SCREEN
  ,AUIM_INDEX_CHARGE_BATT             /* CHARGE BATT notification (String)       */
  ,AUIM_INDEX_PERFORMANCE_INDICATOR   /* PERFORMANCE notification (String)       */
  ,AUIM_INDEX_PATIENT_INDICATOR       /* PATIENT notification (String)           */
  ,AUIM_INDEX_REFERENCE_INDICATOR     /* REFERENCE notification (String)         */
  ,AUIM_INDEX_ONE_INDENT_REQUIRED     /* +1 INDENT REQUIRED notification (String)*/
  ,AUIM_INDEX_TIP_ID_VALID            /* TIP ID VALID notification (String)      */
  ,AUIM_INDEX_TIP_ID_INVALID          /* TIP ID INVALID notification (String)    */
  ,AUIM_INDEX_TIP_ID_USED             /* TIP ID USED notification (String)       */
  ,AUIM_INDEX_UNSTABLE                /* UNSTABLE notification (String)          */
  ,AUIM_INDEX_TERMINATE_INDICATOR     /* TERMINATE notification (String)         */
  ,AUIM_INDEX_ERROR_INDICATOR         /* ERROR notification (String)             */
  ,AUIM_INDEX_PERF_CHK_RECOMMENDED    /* PERF CHK RECOMMEN notification (String) */
 
 ,AUIM_MAX_OSD_INDICATORS
};

/* Indexes in the fuim_ValidityFunction() function and in the fuimFieldStruct field structure
*/
//#define TEST_VALIDITY

enum valididy_function_tables_ids {

    AUIM_MENU_VALIDITY_FUNCTION      = 0
   ,AUIM_FIELD_VALIDITY_FUNCTION
   ,AUIM_FIELD_INDICATOR_VALIDITY_FUNCTION


   ,AUIM_FIELD_BATTERY_INDICATOR_VALIDITY_FUNCTION
   ,AUIM_FIELD_BLE_INDICATOR_VALIDITY_FUNCTION
   ,AUIM_FIELD_SYNC_INDICATOR_VALIDITY_FUNCTION
   ,AUIM_FIELD_CHARGE_BATT_VALIDITY_FUNCTION

 #ifdef TEST_VALIDITY
   ,AUIM_FIELD_PERFORMANCE_START_VALIDITY_FUNCTION
   ,AUIM_FIELD_PATIENT_START_VALIDITY_FUNCTION
 #endif
////////////////////////////////////////////////////////
   ,AUIM_PAIRING_MENU_VALIDITY_FUNCTION

   ,AUIM_FIELD_BUTTON_VALIDITY_FUNCTION
   ,AUIM_FIELD_EDIT_VALIDITY_FUNCTION

};


/*=========================================================================*/
/* @enum action_handler_function_tables_ids | This enumeration contains  */
/*       the ID's of all action functions that can be called by UIMS.    */
/*       Action handler functions are defined in the ...DialogKeys       */
/*       arrays. See fuim_ActionHandler for more information             */

enum action_handler_function_tables_ids {

    AUIM_NO_ACTION_FUNCTION
   ,AUIM_DISPLAY_CONFIGURATION_MENU
   ,AUIM_DISPLAY_IDLE_MENU
   ,AUIM_DISPLAY_PAIRING_MENU
   ,AUIM_DISPLAY_PAIRING_CANCELED_MENU
   ,AUIM_DISPLAY_PAIRING_OK_CLOSE_MENU
   ,AUIM_DISPLAY_PAIRING_FAILED_CLOSE_MENU

   ,AUIM_DISPLAY_PERFORMANCE_MENU
   ,AUIM_DISPLAY_PERFORMANCE_CONTINUE_MENU
   ,AUIM_DISPLAY_PERFORMANCE_CANCELED_MENU
   ,AUIM_DISPLAY_PERFORMANCE_COMPLETE_MENU
   ,AUIM_DISPLAY_PERFORMANCE_TERMINATE_MENU
   ,AUIM_DISPLAY_PERFORMANCE_RESULT_MENU

   ,AUIM_DISPLAY_REFERENCE_MENU
   ,AUIM_DISPLAY_REFERENCE_CONTINUE_MENU
   ,AUIM_DISPLAY_REFERENCE_CANCELED_MENU
   ,AUIM_DISPLAY_REFERENCE_COMPLETE_MENU
   ,AUIM_DISPLAY_REFERENCE_TERMINATE_MENU
   ,AUIM_DISPLAY_REFERENCE_UNSTABLE_REPEAT_MENU

   ,AUIM_DISPLAY_PATIENT_MENU
   ,AUIM_DISPLAY_PATIENT_CONTINUE_MENU
   ,AUIM_DISPLAY_PATIENT_CANCELED_MENU
   ,AUIM_DISPLAY_PATIENT_COMPLETE_MENU
   ,AUIM_DISPLAY_PATIENT_TERMINATE_MENU
   ,AUIM_DISPLAY_PATIENT_RESULT_MENU
   ,AUIM_DISPLAY_PATIENT_FAILED_MENU

   ,AUIM_DISPLAY_PATIENT_VALIDATING_MENU
   ,AUIM_DISPLAY_PATIENT_VALIDATING_RESULT_MENU

   ,AUIM_DISPLAY_CHARGE_BATTERY_MENU

   ,AUIM_ACTION_CONFIGURATION_NEXT_FIELD
   ,AUIM_ACTION_CONFIGURATION_CANCELED
   ,AUIM_ACTION_SEND_STPM_PACKET

};

enum transformer_function_tables_ids
{
  AUIM_NO_CHANGE_FUNCTION
 ,AUIM_CHANGE_DISPLAY_TEMPORARY_PROGNUMBER
 ,AUIM_CHANGE_AV_SOURSE
 ,AUIM_CHANGE_LOCK
 ,AUIM_SET_TIME_OFF

};

//Observer Index Namespace
enum observer_function_tables_ids
{ /*================================================================================================================*/
  /*   Enum Name(s)            |          Meaning       |       Usage                      |   Rendering Behavior   */
  /*================================================================================================================*/
  AUIM_NO_GET_FUNCTION = 0   //   Base/empty observer    - Default case; returns 0         |  NOT rendered Skip prompt

 ,AUIM_GET_BATTERY_INDICATOR //    Active observers      -  Switch cases execute callbacks | Render string from callback
 ,AUIM_GET_BLE_INDICATOR
 ,AUIM_GET_SYNC_INDICATOR
 ,AUIM_GET_CHARGE_BAT_INDICATOR

 ,AUIM_GET_IDLE_WAITINGS_STATUS
 ,AUIM_GET_PAIRING_CODE
 ,AUIM_GET_PAIRING_RESULT_STATUS
 ,AUIM_GET_PAIRING_RESULT_CANCEL

 ,AUIM_GET_STRAIN_GAUSE_STATUS
 ,AUIM_GET_FW_VERSION
 ,AUIM_GET_CURRENT_DATE
 ,AUIM_GET_CURRENT_TIME
 ,AUIM_GET_AM_PM_TIME_SUFFIX_ID

 ,AUIM_GET_CALIBRATION_CONST
 ,AUIM_GET_REFERENCE_NUMBER
 ,AUIM_GET_SERIAL_NUMBER
 ,AUIM_GET_ERROR_CODE_NUMBER


 ,AUIM_GET_PERF_CHECK_REQUIRED_PROMPT
 ,AUIM_GET_CHARGE_BATTERY_PROMPT

// ,AUIM_GET_PERFORMANCE_NOTIFICATION
// ,AUIM_GET_PERFORMANCE_START_NOTIFICATION

 ,AUIM_GET_PERFORMANCE_VALUE
 ,AUIM_GET_REFERENCE_VALUE
 ,AUIM_GET_PATIENT_VALUE

 ,AUIM_GET_CHARGE_BATTERY_IND
 ,AUIM_GET_PERFORMANCE_IND
 ,AUIM_GET_PATIENT_IND
 ,AUIM_GET_REFERENCE_IND
 ,AUIM_GET_ONE_INDENT_IND
 ,AUIM_GET_TIP_ID_VALID_IND
 ,AUIM_GET_TIP_ID_INVALID_IND
 ,AUIM_GET_TIP_ID_USED_IND
 ,AUIM_GET_UNSTABLE_IND
 ,AUIM_GET_TERMINATE_IND
 ,AUIM_GET_UNION3_OUT_IND
 ,AUIM_GET_ERROR_IND

 ,AUIM_GET_BUTTON_THREE_DOTS
 ,AUIM_GET_BUTTON_ELLIPSE
 ,AUIM_GET_BUTTON_ONE_DOT
 
 
 
 ,AUIM_GET_FIRMWARE_VERSION_PROMPT
 ,AUIM_GET_CURRENT_DATE_PROMPT
 ,AUIM_GET_CURRENT_TIME_PROMPT
 ,AUIM_GET_REFERENCE_NUMBER_PROMPT
 ,AUIM_GET_PAIRING_WAITING_PROMPT
 ,AUIM_GET_APPROXIMATION_IND
 
 ,AUIM_GET_CANCELED_PROMPT

 ,AUIM_GET_PERFORMANCE_COMPLETE_RESULT
 ,AUIM_GET_REFERENCE_COMPLETE_RESULT
 ,AUIM_GET_PATIENT_COMPLETE_RESULT
 ,AUIM_GET_PATIENT_FILED_RESULT

 ,AUIM_GET_PERFORMANCE_RESULT
 ,AUIM_GET_PATIENT_RESULT

 ,AUIM_GET_PATIENT_VALIDATING_PROMPT

 //,AUIM_GET_TERMINATED_PROMPT

 ,AUIM_GET_CALIBRATION_CONSTANT_PROMPT
 ,AUIM_GET_STRAIN_GAUGE_STATUS_PROMPT
 ,AUIM_GET_SERIAL_NUMBER_PROMPT
 ,AUIM_GET_PERF_CHK_RECOMMENDED
 

 ,AUIM_OBSERVER_INDEX_RESERVED = 253 //  Reserved markers        -  Future use; not currently assigned         |  NOT rendered 
 ,AUIM_OBSERVER_INDEX_NONE     = 254 //  Explicitly no observer  -  Means "no prompt should be displayed"      |  NOT rendered  
 ,AUIM_OBSERVER_INDEX_INVALID  = 255 // Invalid/error condition  -  Indicates uninitialized or corrupted state |  NOT rendered  
};
 
#define AUIM_NONE_PROMPT     AUIM_NO_GET_FUNCTION
#define AUIM_INVALID_PROMPT  IMG_INVALID_ID   
/*=======================================================================*/
/*        G L O B A L   F U N C T I O N   P R O T O T Y P E S            */
/*=======================================================================*/


#endif /* UI_INC_AUIM_API_H_ */
