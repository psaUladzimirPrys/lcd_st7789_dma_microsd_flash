/*
 * auim_mnu.c
 *
 *  Created on: 20 февр. 2026 г.
 *      Author: priss
 */
/*=======================================================================*/
/*        I N C L U D E S                                                */
/*=======================================================================*/
#include "hglobal.h"
#include "fuim.h"
#include "fmnu.h"
#include "auim_api.h"
#include "auim_mnu.h"
#include "aukh.h"



/*==========================================================================*/
/* G L O B A L   R E F E R E N C E S                                        */
/*==========================================================================*/


/*=========================================================================*/
/*   G L O B A L   D E F I N I T I O N S                                   */
/*=========================================================================*/
#define AUIM_MNU_SLIDER_BEGIN_END_CHAR 0x0
#define AUIM_MNU_BALANSE_BEGIN_END_CHAR 0x0
#define AUIM_MNU_NUMERIC_STR_RANGE 0x3039
#define AUIM_MNU_SHIFT_X 3

/*==========================================================================*/
/*        G L O B A L   D A T A   D E C L A R A T I O N S                   */
/*==========================================================================*/

enum configuration_fields_index_enum
  {

  AUIM_MNU_CONFIG_FIELD_EMPTY = 0

 ,AUIM_MNU_MAX_CONFIG_FIELD

 };

const fmnu_ListStruct ListDataGoodBad[]=
{
  {0,  FMNU_LIST_ITEMS_BAD}
 ,{1,  FMNU_LIST_ITEMS_GOOD}
 ,{255, FMNU_LIST_ITEMS_ERROR}
};

const fuimDialogNavigation auim_CongigurationMenuDialogKeys[] = {

  {AU_KEY_INVALID       , 0} /* sentinel, do not remove !!! */
};

/*=======================================================================*/
/*                                                                       */
/*=======================================================================*/

const fuimFieldStruct auim_ConfigurationMenuFields[AUIM_MNU_CONFIG_MAX_FIELD] = {
/*=======================================================================*/
/*                SERIAL_NUMBER        */
/*=======================================================================*/
  {
  FUIM_FIELDTYPE_NUMERIC_VALUE,
  AUIM_FIELD_VALIDITY_FUNCTION,     //Byte       ValidityFunction;
  IMG_ID_PROPERTY_1_VARIANT6_6,    //osdStringID   Prompt;
  AUIM_MENU_FIELD_COLOUR,       //Byte PromptColour;
  0,      //Byte ChangeFunction;
  AUIM_GET_LANGUAGE,//Byte      GetFunction;
  0,//Byte      SetFunction;
  {0},//TFieldSize    FieldSize;
  {0},//TFieldScaling   FieldScaling;
  0,//Byte      Prefix;
  0,//osdStringID   Suffix;
  FUIM_FIELD_NO_TIMEOUT,//Byte      TimeOut;
  0,//Byte      TriggerDigits;
  {0},//TFieldCharacters FieldCharacters;
  0,//Byte      Alignment;
  0,//Byte      LeadingZeros;
  0//fuimDialogNavigation RDATA * ToDoWithKey;
  },
/*=======================================================================*/
/*       FIRMWARE_VERSION                   */
/*=======================================================================*/
  {
   FUIM_FIELDTYPE_STRING_VALUE,
  AUIM_FIELD_VALIDITY_FUNCTION,//Byte       ValidityFunction;
  IMG_ID_PROPERTY_1_VARIANT12,//osdStringID  Prompt;
  AUIM_MENU_FIELD_COLOUR,//Byte PromptColour;
  0,      //Byte ChangeFunction;
  AUIM_GET_START_POWER_ON,//Byte      GetFunction;
  0,//Byte      SetFunction;
  {0},//TFieldSize    FieldSize;
  {0},//TFieldScaling   FieldScaling;
  0,//Byte      Prefix;
  0,//osdStringID   Suffix;
  FUIM_FIELD_NO_TIMEOUT,//Byte      TimeOut;
  0,//Byte      TriggerDigits;
  {0},//TFieldCharacters FieldCharacters;
  0,//Byte      Alignment;
  0,//Byte      LeadingZeros;
  0//fuimDialogNavigation RDATA * ToDoWithKey;
  },
/*=======================================================================*/
/*        REFERENCE_NUMBER             */
/*=======================================================================*/
 {
  FUIM_FIELDTYPE_NUMERIC_VALUE,
  AUIM_FIELD_VALIDITY_FUNCTION,//Byte       ValidityFunction;
  IMG_ID_PROPERTY_1_VARIANT11_4,//osdStringID  Prompt;
  AUIM_MENU_FIELD_COLOUR,//Byte PromptColour;
  AUIM_CHANGE_DISPLAY_TEMPORARY_PROGNUMBER,      //Byte ChangeFunction;
  AUIM_GET_DISPLAY_TEMPORARY_PROGNUMBER,//Byte      GetFunction;
  0,//Byte      SetFunction;
  {0},//TFieldSize    FieldSize;
  {0},//TFieldScaling   FieldScaling;
  0,//Byte      Prefix;
  0,//osdStringID   Suffix;
  FUIM_FIELD_NO_TIMEOUT,//Byte      TimeOut;
  0,//Byte      TriggerDigits;
  {0},//TFieldCharacters FieldCharacters;
  0,//Byte      Alignment;
  0,//Byte      LeadingZeros;
  0//fuimDialogNavigation RDATA * ToDoWithKey;
 },
/*=======================================================================*/
/*        STRAIN_GAUGE_STATUS                    */
/*=======================================================================*/
{
  FUIM_FIELDTYPE_LIST,
  AUIM_FIELD_VALIDITY_FUNCTION,//Byte       ValidityFunction;
  IMG_ID_PROPERTY_1_VARIANT7_6,//osdStringID   Prompt;
  AUIM_MENU_FIELD_COLOUR,//Byte PromptColour;
  AUIM_CHANGE_AV_SOURSE,       //Byte ChangeFunction;
  AUIM_GET_AV_SOURSE,//Byte       GetFunction;
  0,//Byte      SetFunction;
  {0},//TFieldSize    FieldSize;
  {0},//TFieldScaling   FieldScaling;
  0,//Byte      Prefix;
  0,//osdStringID   Suffix;
  FUIM_FIELD_NO_TIMEOUT,//Byte      TimeOut;
  0,//Byte      TriggerDigits;
  {(fmnu_ListStruct *)&ListDataGoodBad[0]},//TFieldCharacters FieldCharacters;
  0,//Byte      Alignment;
  0,//Byte      LeadingZeros;
  0//fuimDialogNavigation RDATA * ToDoWithKey;
},

/*=======================================================================*/
/*       CALIBRATION_CONSTANT                    */
/*=======================================================================*/
{
  FUIM_FIELDTYPE_NUMERIC_VALUE,
  AUIM_FIELD_VALIDITY_FUNCTION,//Byte       ValidityFunction;
  IMG_ID_PROPERTY_1_VARIANT10_6,//osdStringID   Prompt;
  AUIM_MENU_FIELD_COLOUR,//Byte PromptColour;
  AUIM_CHANGE_LOCK,      //Byte ChangeFunction;
  AUIM_GET_LOCK,//Byte      GetFunction;
  0,//Byte      SetFunction;
  {0},//TFieldSize    FieldSize;
  {0},//TFieldScaling   FieldScaling;
  0,//Byte      Prefix;
  0,//osdStringID   Suffix;
  FUIM_FIELD_NO_TIMEOUT,//Byte      TimeOut;
  0,//Byte      TriggerDigits;
  {0},//TFieldCharacters FieldCharacters;
  0,//Byte      Alignment;
  0,//Byte      LeadingZeros;
  0//fuimDialogNavigation RDATA * ToDoWithKey;

  }

};

const fmnu_MenuStruct auim_Menu[AUIM_MNU_MAX_MENUS] =
{

/*=================================================================*/
/*       CONFIGURATION  MENU                                       */
/*=================================================================*/
  {
    FMNU_MAIN_MENU,      /* Menu type  {FMNU_MAIN_MENU FMNU_SUB_MAIN_MENU FMNU_MESSAGE_BOARD} */
    1+AUIM_MNU_SHIFT_X,//Position MENU Х
    0,                 //Position MENU Y
    27,                //Menu width
    IMG_ID_PROPERTY_1_VARIANT4_8,   //ID Title MENU
    AUIM_MAIN_MENU_COLOUR,//Title Attributes
    4+AUIM_MNU_SHIFT_X,   //Position Х PromptPos
    26+AUIM_MNU_SHIFT_X,  //Position Х ValuePos
    &auim_ConfigurationMenuFields[0],// Pointer to an array of field structures
    1,                              // Number of fields visible at once
    AUIM_MNU_CONFIG_MAX_FIELD,// Total number of fields
    FUIM_MENU_TIMEOUT, // How long the menu remains active;
                       // if 0 — stays active indefinitely
    0,                 // fuimFixedFieldStruct code *FixedTopField;   /* @field pointer to structure of the fixed top field */
    0                  // fuimFixedFieldStruct code *FixedBottomField; /* @field pointer to structure of the fixed bottom field */
  }

};


/*===========================================================================*/
/*    L O C A L   S Y M B O L    D E C L A R A T I O N S                     */
/*===========================================================================*/

/*===========================================================================*/
/* L O C A L   F U N C T I O N S                                             */
/*===========================================================================*/

/*===========================================================================*/
/*     G L O B A L   F U N C T I O N S                                       */
/*===========================================================================*/

