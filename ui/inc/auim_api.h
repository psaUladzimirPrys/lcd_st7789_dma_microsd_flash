/*
 * auim_api.h
 *
 *  Created on: 20 февр. 2026 г.
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
  AUIM_MAIN_MENU_COLOUR
 ,AUIM_MENU_FIELD_COLOUR
 ,AUIM_BUTTON_FIELD_COLOUR
 ,AUIM_INDICATOR_COLOUR
 ,AUIM_BLANK_INDICATOR_COLOUR
 ,AUIM_MODAL_INDICATOR_COLOUR
 ,AUIM_SPLASH_SCREEN_COLOUR
};

enum indicator_index_enum {

   AUIM_INDEX_BATTERY_INDICATOR = 0
  ,AUIM_INDEX_BLE_INDICATOR
  ,AUIM_INDEX_SYNC_INDICATOR
  ,AUIM_INDEX_SPLASH_SCREEN
  ,AUIM_INDEX_CHARGE_BATT

  ,AUIM_MAX_OSD_INDICATORS
};

/* Индексы в функцию fuim_ValidityFunction() и в структуру поля типа fuimFieldStruct
*/
enum valididy_function_tables_ids {

    AUIM_MENU_VALIDITY_FUNCTION      = 0
   ,AUIM_FIELD_VALIDITY_FUNCTION
   ,AUIM_FIELD_INDICATOR_VALIDITY_FUNCTION


   ,AUIM_FIELD_BATTERY_INDICATOR_VALIDITY_FUNCTION
   ,AUIM_FIELD_BLE_INDICATOR_VALIDITY_FUNCTION
   ,AUIM_FIELD_SYNC_INDICATOR_VALIDITY_FUNCTION
   ,AUIM_FIELD_CHARGE_BATT_VALIDITY_FUNCTION
////////////////////////////////////////////////////////
   ,AUIM_FIELD_BUTTON_VALIDITY_FUNCTION


  ,AUIM_FIELD_SPACER_VALIDITY_FUNCTION
  ,AUIM_FIELD_EDIT_VALIDITY_FUNCTION

};


/*=========================================================================*/
/* @enum action_handler_function_tables_ids | This enumeration contains  */
/*       the ID's of all action functions that can be called by UIMS.    */
/*       Action handler functions are defined in the ...DialogKeys       */
/*       arrays. See fuim_ActionHandler for more information             */

enum action_handler_function_tables_ids {

    AUIM_NO_ACTION_FUNCTION
   ,AUIM_DISPLAY_CONFIG_MENU
   ,AUIM_DISPLAY_IDLE_MENU
   ,AUIM_DISPLAY_PAIRING_MENU

    ,AUIM_ACTION_NEXT_FIELD
};

enum transformer_function_tables_ids
{
  AUIM_NO_CHANGE_FUNCTION
,AUIM_CHANGE_DISPLAY_TEMPORARY_PROGNUMBER
,AUIM_CHANGE_AV_SOURSE
,AUIM_CHANGE_LOCK
,AUIM_SET_TIME_OFF


};

enum observer_function_tables_ids
{
 AUIM_NO_GET_FUNCTION = 0


 ,AUIM_GET_BATTERY_INDICATOR
 ,AUIM_GET_BLE_INDICATOR
 ,AUIM_GET_SYNC_INDICATOR
 ,AUIM_GET_CHARGE_BAT_INDICATOR


 ,AUIM_GET_IDLE_WAITINGS_STATUS
 ,AUIM_GET_PAIRING_CODE


 ,AUIM_GET_STRAIN_GAUSE_STATUS
 ,AUIM_GET_FW_VERSION
 ,AUIM_GET_CALIBRATION_CONST
 ,AUIM_GET_REFERENCE_NUMBER
 ,AUIM_GET_SERIAL_NUMBER


};

/*=======================================================================*/
/*        G L O B A L   F U N C T I O N   P R O T O T Y P E S            */
/*=======================================================================*/


#endif /* UI_INC_AUIM_API_H_ */
