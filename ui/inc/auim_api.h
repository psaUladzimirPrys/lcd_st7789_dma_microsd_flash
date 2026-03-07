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

enum auim_SignIndicator_enum {
   AUIM_MUTE_ON_SIGN_INDICATOR
  ,AUIM_MUTE_OFF_SIGN_INDICATOR
//Все добавления Выше этой строки
  ,AUIM_MAX_SIGN_INDICATOR
  ,AUIM_NONE_SIGN_INDICATOR
};

enum colour_index_enum {
  AUIM_MAIN_MENU_COLOUR
 ,AUIM_MAIN_MENU_FIELD_COLOUR
 ,AUIM_SOUND_MENU_COLOUR
 ,AUIM_TEXNO_MENU_COLOUR
 ,AUIM_TVSETUP_MENU_COLOUR
 ,AUIM_PICTURE_MENU_COLOUR
 ,AUIM_PROGRAMME_TUNING_MENU_COLOUR
 ,AUIM_OVERVIEW_MENU_COLOUR
 ,AUIM_OVERVIEW_EDIT_MENU_COLOUR
 ,AUIM_MUTE_INDICATOR_COLOUR
 ,AUIM_INDICATOR_COLOUR
 ,AUIM_DUNAMIC_INDICATOR_COLOUR_PROGRAMM_NUMBER
 ,AUIM_FLASH_INDICATOR_COLOUR
 ,AUIM_FLASH_INDICATOR_COLOUR_RED
 ,AUIM_MENU_FIELD_COLOUR
 ,AUIM_TIME_INDICATOR_COLOUR
 ,AUIM_TIME_HILIGHTE_INDICATOR_COLOUR
 ,AUIM_BLANK_INDICATOR_COLOUR
 ,AUIM_BLANK_SOUND_MENU_INDICATOR_COLOUR
 ,AUIM_BLANK_PICTURE_MENU_INDICATOR_COLOUR
 ,AUIM_BLANK_PROGRAMME_TUNING_MENU_INDICATOR_COLOUR
 ,AUIM_PP_INDICATOR_COLOUR
 ,AUIM_BLANK_TVSETUP_MENU_COLOUR
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
/////////////////////////////////////////////////////////

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


   ,AUIM_DISPLAY_MAIN_MENU
   ,AUIM_DISPLAY_SOUND_MENU
   ,AUIM_ACTION_ENABLE_SOUND_STORE_FUNCTION
   ,AUIM_ACTION_DISABLE_SOUND_STORE_FUNCTION
   ,AUIM_DISPLAY_PICTURE_MENU
   ,AUIM_ACTION_ENABLE_PICTURE_STORE_FUNCTION
   ,AUIM_ACTION_DISABLE_PICTURE_STORE_FUNCTION

   ,AUIM_DISPLAY_PROGRAMME_TUNING_MENU
   ,AUIM_DISPLAY_AUTO_SEARCH_MENU
//   ,AUIM_START_DISPLAY_AUTO_SEARCH_MENU
   ,AUIM_DISPLAY_MANUAL_SEARCH_MENU
   ,AUIM_DISPLAY_OVERVIEW_MENU
   ,AUIM_DISPLAY_TIMERS_MENU
   ,AUIM_DISPLAY_EDIT_MENU
   ,AUIM_ACTION_ENABLE_EDIT_DELETE_FUNCTION
   ,AUIM_ACTION_DISABLE_EDIT_DELETE_FUNCTION
   ,AUIM_ACTION_SET_EDIT_SWAP_FUNCTION

   ,AUIM_DISPLAY_FEATURES_MENU //Отображаем поля техноменю
   ,AUIM_DISPLAY_TEXNO_MENU
   ,AUIM_DISPLAY_GEOMETRY_TEXNO_MENU
   ,AUIM_DISPLAY_ADJUSTMENT_TEXNO_MENU
   ,AUIM_DISPLAY_OPTIONS_TEXNO_MENU
   ,AUIM_DISPLAY_TUNER_TEXNO_MENU


   ,AUIM_HIDE_MAIN_MENU
   ,AUIM_HIDE_SOUND_MENU
   ,AUIM_HIDE_PICTURE_MENU
   ,AUIM_HIDE_PROGRAMME_TUNING_MENU
   ,AUIM_HIDE_OVERVIEW_MENU
   ,AUIM_HIDE_TIMERS_MENU
   ,AUIM_HIDE_FEATURES_MENU
   ,AUIM_HIDE_AUTO_SEARCH_MENU


/////////////////////////////////////////////////////////- Ручной поиск
   ,AUIM_HIDE_MANUAL_SEARCH_MENU
   ,AUIM_ACTION_HIDE_MANUAL_SEARCH_MENU

   ,AUIM_ACTION_START_STOP_PROCESS_MANUAL_SEARCH_MENU
   ,AUIM_ACTION_CONTROL_PROCESS_MANUAL_SEARCH_MENU
   ,AUIM_ACTION_RESET_SEARCH_HIDE_MANUAL_SEARCH_MENU
   ,AUIM_ACTION_DISABLE_MANUAL_TUNE_STORE_FUNCTION
   ,AUIM_ACTION_ENABLE_MANUAL_TUNE_STORE_FUNCTION
   ,AUIM_HIDE_EDIT_MENU
   ,AUIM_ACTION_HIDE_EDIT_MENU


///////////////////////////////////-ИНДИКАТОРЫ
   ,AUIM_INDICATOR_DISPLAY_MAIN_MENU



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

 ,AUIM_GET_STRAIN_GAUSE_STATUS

////////////////////////////////////////
 ,AUIM_GET_LANGUAGE
 ,AUIM_GET_START_POWER_ON
 ,AUIM_GET_DISPLAY_TEMPORARY_PROGNUMBER
 ,AUIM_GET_LOCK
 ,AUIM_GET_MUTE_INDICATOR
 ,AUIM_GET_MAIN_VOLUME


};

/*=======================================================================*/
/*        G L O B A L   F U N C T I O N   P R O T O T Y P E S            */
/*=======================================================================*/



#endif /* UI_INC_AUIM_API_H_ */
