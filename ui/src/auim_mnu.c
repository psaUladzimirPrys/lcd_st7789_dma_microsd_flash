/*
 * auim_mnu.c
 *
 *  Created on: 20 февр. 2026 г.
 *      Author: priss
 */
/*=======================================================================*/
/*        I N C L U D E S                                                */
/*=======================================================================*/
#include <global.h>
#include "img_storage.h"
#include "fmnu_str.h"
#include "fuim.h"
#include "fmnu.h"
#include "auim_api.h"
#include "auim_mnu.h"
#include "aukh.h"
#include "fsrv.h"



/*==========================================================================*/
/* G L O B A L   R E F E R E N C E S                                        */
/*==========================================================================*/


/*=========================================================================*/
/*   G L O B A L   D E F I N I T I O N S                                   */
/*=========================================================================*/
#define AUIM_MNU_SLIDER_BEGIN_END_CHAR 0x0
#define AUIM_MNU_BALANSE_BEGIN_END_CHAR 0x0
#define AUIM_MNU_NUMERIC_STR_RANGE 0x3039
#define AUIM_MNU_SHIFT_X 0
#define AUIM_MNU_X_POS  0
#define AUIM_MNU_Y_POS  0


/*==========================================================================*/
/*        G L O B A L   D A T A   D E C L A R A T I O N S                   */
/*==========================================================================*/

/*===========================================================================*/
/*    L O C A L   S Y M B O L    D E C L A R A T I O N S                     */
/*===========================================================================*/


const fmnu_ListStruct ListDataGoodBad[]=
{
  { FSRV_GAUGE_STATUS_BAD,   FMNU_LIST_ITEMS_BAD  }
 ,{ FSRV_GAUGE_STATUS_GOOD,  FMNU_LIST_ITEMS_GOOD }
 ,{ 255,                     FMNU_LIST_ITEMS_ERROR}
};


fuimDialogNavigation auim_IdleDialogKeys[] = {
   {AU_VIRTUAL_KEY_4  , AUIM_DISPLAY_CONFIG_MENU}
  ,{AU_VIRTUAL_KEY_5  , AUIM_DISPLAY_PAIRING_MENU}
  ,{AU_KEY_INVALID    , 0} /* sentinel, do not remove !!! */
};


fuimDialogNavigation auim_PairingDialogKeys[] = {
   {AU_VIRTUAL_KEY_2  , AUIM_DISPLAY_IDLE_MENU}
  ,{AU_KEY_INVALID    , 0} /* sentinel, do not remove !!! */
};

fuimDialogNavigation auim_ConfigurationDialogKeys[] = {
   {AU_VIRTUAL_KEY_2  , AUIM_DISPLAY_IDLE_MENU}
  ,{AU_VIRTUAL_KEY_1  , AUIM_ACTION_NEXT_FIELD}
  ,{AU_KEY_INVALID    , 0} /* sentinel, do not remove !!! */
};

/*=======================================================================*/
/*                                                                       */
/*=======================================================================*/

const fuimFieldStruct auim_MenuButtonFields[AUIM_MNU_MAX_BUTTON_FIELDS] =
{
    /*=======================================================================*/
    /*                IDLE_MNU_LEFT_BUTTON  - P E R F   C H K        */
    /*=======================================================================*/
    {
    FUIM_FIELDTYPE_BUTTON,
    AUIM_FIELD_BUTTON_VALIDITY_FUNCTION,     //Byte       ValidityFunction;
    IMG_ID_PROPERTY_1_ELLIPSE_15,    //osdStringID   Prompt;
    AUIM_BUTTON_FIELD_COLOUR,       //Byte PromptColour;
    0,      //Byte ChangeFunction;
    0,//Byte      GetFunction;
    0,//Byte      SetFunction;
    {0},//TFieldSize    FieldSize;
    {0},//TFieldScaling   FieldScaling;
    0,//Byte      Prefix;
    0,//osdStringID   Suffix;
    FUIM_FIELD_NO_TIMEOUT,//Byte      TimeOut;
    0,//Byte      TriggerDigits;
    {.Button = IMG_ID_PROPERTY_1_DEFAULT_14},//TFieldCharacters FieldCharacters;
    FUIM_ALIGNMENT_CENTRE,//Byte      Alignment;
    0,//Byte      LeadingZeros;
    0//fuimDialogNavigation  * ToDoWithKey;
    }
  /*=======================================================================*/
  /*       IDLE_MNU_RIGHT_BUTTON    - P A R A M S                   */
  /*=======================================================================*/
    ,{
    FUIM_FIELDTYPE_BUTTON,
    AUIM_FIELD_BUTTON_VALIDITY_FUNCTION,//Byte       ValidityFunction;
    IMG_ID_PROPERTY_1_FRAME_53,//osdStringID  Prompt;
    AUIM_BUTTON_FIELD_COLOUR,//Byte PromptColour;
    0,      //Byte ChangeFunction;
    0,//Byte      GetFunction;
    0,//Byte      SetFunction;
    {0},//TFieldSize    FieldSize;
    {0},//TFieldScaling   FieldScaling;
    0,//Byte      Prefix;
    0,//osdStringID   Suffix;
    FUIM_FIELD_NO_TIMEOUT,//Byte      TimeOut;
    0,//Byte      TriggerDigits;
    {.Button = IMG_ID_PROPERTY_1_DEFAULT_15},//TFieldCharacters FieldCharacters;
    FUIM_ALIGNMENT_CENTRE,//Byte      Alignment;
    0,//Byte      LeadingZeros;
    0//fuimDialogNavigation  * ToDoWithKey;
    }
    /*=======================================================================*/
    /*            PAIRING_MNU_RIGHT_BUTTON   -   C A N C E L                */
    /*=======================================================================*/
    ,{
    FUIM_FIELDTYPE_BUTTON,
    AUIM_FIELD_BUTTON_VALIDITY_FUNCTION,//Byte       ValidityFunction;
    IMG_ID_PROPERTY_1_RECTANGLE_201,//osdStringID  Prompt;
    AUIM_BUTTON_FIELD_COLOUR,//Byte PromptColour;
    0,      //Byte ChangeFunction;
    0,//Byte      GetFunction;
    0,//Byte      SetFunction;
    {0},//TFieldSize    FieldSize;
    {0},//TFieldScaling   FieldScaling;
    0,//Byte      Prefix;
    0,//osdStringID   Suffix;
    FUIM_FIELD_NO_TIMEOUT,//Byte      TimeOut;
    0,//Byte      TriggerDigits;
    {.Button = IMG_ID_PROPERTY_1_DEFAULT_11},//TFieldCharacters FieldCharacters;
    FUIM_ALIGNMENT_CENTRE,//Byte      Alignment;
    0,//Byte      LeadingZeros;
    0//fuimDialogNavigation  * ToDoWithKey;
    }

    /*=======================================================================*/
    /*                CONFIG_MNU_LEFT_BUTTON  - N E X T       */
    /*=======================================================================*/
   ,{
      FUIM_FIELDTYPE_BUTTON,
      AUIM_FIELD_BUTTON_VALIDITY_FUNCTION,     //Byte       ValidityFunction;
      IMG_ID_PROPERTY_1_ELLIPSE_15,    //osdStringID   Prompt;
      AUIM_BUTTON_FIELD_COLOUR,       //Byte PromptColour;
      0,      //Byte ChangeFunction;
      0,//Byte      GetFunction;
      0,//Byte      SetFunction;
      {0},//TFieldSize    FieldSize;
      {0},//TFieldScaling   FieldScaling;
      0,//Byte      Prefix;
      0,//osdStringID   Suffix;
      FUIM_FIELD_NO_TIMEOUT,//Byte      TimeOut;
      0,//Byte      TriggerDigits;
      {.Button = IMG_ID_PROPERTY_1_DEFAULT_12},//TFieldCharacters FieldCharacters;
      FUIM_ALIGNMENT_CENTRE,//Byte      Alignment;
      0,//Byte      LeadingZeros;
      0//fuimDialogNavigation  * ToDoWithKey;
    }


    /*=======================================================================*/
    /*            CONFIG_MNU_RIGHT_BUTTON   -   C A N C E L                */
    /*=======================================================================*/
    ,{
      FUIM_FIELDTYPE_BUTTON,
      AUIM_FIELD_BUTTON_VALIDITY_FUNCTION,//Byte       ValidityFunction;
      IMG_ID_PROPERTY_1_RECTANGLE_201,//osdStringID  Prompt;
      AUIM_BUTTON_FIELD_COLOUR,//Byte PromptColour;
      0,      //Byte ChangeFunction;
      0,//Byte      GetFunction;
      0,//Byte      SetFunction;
      {0},//TFieldSize    FieldSize;
      {0},//TFieldScaling   FieldScaling;
      0,//Byte      Prefix;
      0,//osdStringID   Suffix;
      FUIM_FIELD_NO_TIMEOUT,//Byte      TimeOut;
      0,//Byte      TriggerDigits;
      {.Button = IMG_ID_PROPERTY_1_DEFAULT_11},//TFieldCharacters FieldCharacters;
      FUIM_ALIGNMENT_CENTRE,//Byte      Alignment;
      0,//Byte      LeadingZeros;
      0//fuimDialogNavigation  * ToDoWithKey;
    }

};

/*=======================================================================*/
/*                                                                       */
/*=======================================================================*/
fuimFixedFieldStruct auim_FixedEmptyField[AUIM_MNU_MAX_FIXED_FIELDS] =
{  

/*=======================================================================*/
/*                                                                       */
/*=======================================================================*/
   {
   AUIM_MENU_FIELD_COLOUR ,   /* ID of colour struct */
   AUIM_NO_GET_FUNCTION,      /* @field function which will return the text of the field. */
   FUIM_ALIGNMENT_CENTRE,
   }                            /* @field Alignment of text in field as specified in fuim_Alignment */

/*=======================================================================*/
/*                                                                       */
/*=======================================================================*/
  ,{
   AUIM_MENU_FIELD_COLOUR,  /* ID of colour struct */
   AUIM_NO_GET_FUNCTION,    /* @field function which will return the text of the field. */
   FUIM_ALIGNMENT_LEFT,
   }                        /* @field Alignment of text in field as specified in fuim_Alignment */
 

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
  AUIM_GET_SERIAL_NUMBER,//Byte      GetFunction;
  0,//Byte      SetFunction;
  {.Numeric = 2},//TFieldSize    FieldSize;
  {0},//TFieldScaling   FieldScaling;
  0,//Byte      Prefix;
  0,//osdStringID   Suffix;
  FUIM_FIELD_TIMEOUT,//Byte      TimeOut;
  0,//Byte      TriggerDigits;
  {.NumericFont.size = FUIM_FONT_SIZE_SMALL, .NumericFont.color = FUIM_FONT_COLOR_1},//TFieldCharacters FieldCharacters;
  FUIM_ALIGNMENT_RIGHT,//Byte      Alignment;
  0,//Byte      LeadingZeros;
  &auim_ConfigurationDialogKeys[0]//fuimDialogNavigation  * ToDoWithKey;
  },
/*=======================================================================*/
/*       FIRMWARE_VERSION                   */
/*=======================================================================*/
  {
   FUIM_FIELDTYPE_NUMERIC_VALUE,
  AUIM_FIELD_VALIDITY_FUNCTION,//Byte       ValidityFunction;
  IMG_ID_PROPERTY_1_VARIANT12,//osdStringID  Prompt;
  AUIM_MENU_FIELD_COLOUR,//Byte PromptColour;
  0,      //Byte ChangeFunction;
  AUIM_GET_FW_VERSION,//Byte      GetFunction;
  0,//Byte      SetFunction;
  {.Numeric = 2},//TFieldSize    FieldSize;
  {0},//TFieldScaling   FieldScaling;
  0,//Byte      Prefix;
  0,//osdStringID   Suffix;
  FUIM_FIELD_TIMEOUT,//Byte      TimeOut;
  0,//Byte      TriggerDigits;
  {.NumericFont.size = FUIM_FONT_SIZE_SMALL, .NumericFont.color = FUIM_FONT_COLOR_1},//TFieldCharacters FieldCharacters;
  FUIM_ALIGNMENT_RIGHT,//Byte      Alignment;
  0,//Byte      LeadingZeros;
  &auim_ConfigurationDialogKeys[0]//fuimDialogNavigation  * ToDoWithKey;
  },
/*=======================================================================*/
/*        REFERENCE_NUMBER             */
/*=======================================================================*/
 {
  FUIM_FIELDTYPE_NUMERIC_VALUE,
  AUIM_FIELD_VALIDITY_FUNCTION,//Byte       ValidityFunction;
  IMG_ID_PROPERTY_1_VARIANT11_4,//osdStringID  Prompt;
  AUIM_MENU_FIELD_COLOUR,//Byte PromptColour;
  0,      //Byte ChangeFunction;
  AUIM_GET_REFERENCE_NUMBER,//Byte      GetFunction;
  0,//Byte      SetFunction;
  {.Numeric = 2},//TFieldSize    FieldSize;
  {0},//TFieldScaling   FieldScaling;
  0,//Byte      Prefix;
  0,//osdStringID   Suffix;
  FUIM_FIELD_TIMEOUT,//Byte      TimeOut;
  0,//Byte      TriggerDigits;
  {.NumericFont.size = FUIM_FONT_SIZE_SMALL, .NumericFont.color = FUIM_FONT_COLOR_1},//TFieldCharacters FieldCharacters;
  FUIM_ALIGNMENT_RIGHT,//Byte      Alignment;
  0,//Byte      LeadingZeros;
  &auim_ConfigurationDialogKeys[0]//fuimDialogNavigation  * ToDoWithKey;
 },
/*=======================================================================*/
/*        STRAIN_GAUGE_STATUS                    */
/*=======================================================================*/
{
  FUIM_FIELDTYPE_LIST,
  AUIM_FIELD_VALIDITY_FUNCTION,//Byte       ValidityFunction;
  IMG_ID_PROPERTY_1_VARIANT7_6,//osdStringID   Prompt;
  AUIM_MENU_FIELD_COLOUR,//Byte PromptColour;
  0,       //Byte ChangeFunction;
  AUIM_GET_STRAIN_GAUSE_STATUS,//Byte       GetFunction;
  0,//Byte      SetFunction;
  {0},//TFieldSize    FieldSize;
  {0},//TFieldScaling   FieldScaling;
  0,//Byte      Prefix;
  0,//osdStringID   Suffix;
  FUIM_FIELD_TIMEOUT,//Byte      TimeOut;
  0,//Byte      TriggerDigits;
  {(fmnu_ListStruct *)&ListDataGoodBad[0]},//TFieldCharacters FieldCharacters;
  0,//Byte      Alignment;
  0,//Byte      LeadingZeros;
  &auim_ConfigurationDialogKeys[0]//fuimDialogNavigation  * ToDoWithKey;
},

/*=======================================================================*/
/*       CALIBRATION_CONSTANT                    */
/*=======================================================================*/
{
  FUIM_FIELDTYPE_NUMERIC_VALUE,
  AUIM_FIELD_VALIDITY_FUNCTION,//Byte       ValidityFunction;
  IMG_ID_PROPERTY_1_VARIANT10_6,//osdStringID   Prompt;
  AUIM_MENU_FIELD_COLOUR,//Byte PromptColour;
  0,      //Byte ChangeFunction;
  AUIM_GET_CALIBRATION_CONST,//Byte      GetFunction;
  0,//Byte      SetFunction;
  {.Numeric = 2},//TFieldSize    FieldSize;
  {0},//TFieldScaling   FieldScaling;
  0,//Byte      Prefix;
  0,//osdStringID   Suffix;
  FUIM_FIELD_TIMEOUT,//Byte      TimeOut;
  0,//Byte      TriggerDigits;
  {.NumericFont.size = FUIM_FONT_SIZE_SMALL, .NumericFont.color = FUIM_FONT_COLOR_1},//TFieldCharacters FieldCharacters;
  FUIM_ALIGNMENT_RIGHT,//Byte      Alignment;
  0,//Byte      LeadingZeros;
  &auim_ConfigurationDialogKeys[0]//fuimDialogNavigation  * ToDoWithKey;

  }

};




const fuimFieldStruct auim_IdleMenuFields[AUIM_MNU_IDLE_MAX_FIELD] = 
{
  /*=======================================================================*/
  /*       IDLE_WAITINGS_STATUS                   */
  /*=======================================================================*/
  {
    FUIM_FIELDTYPE_STRING_VALUE,
    AUIM_FIELD_VALIDITY_FUNCTION,//Byte       ValidityFunction;
    FMNU_NONE_PROMPT,//osdStringID  Prompt;
    AUIM_MENU_FIELD_COLOUR,//Byte PromptColour;
    0,      //Byte ChangeFunction;
    AUIM_GET_IDLE_WAITINGS_STATUS,//Byte      GetFunction;
    0,//Byte      SetFunction;
    {0},//TFieldSize    FieldSize;
    {0},//TFieldScaling   FieldScaling;
    0,//Byte      Prefix;
    0,//osdStringID   Suffix;
    FUIM_FIELD_TIMEOUT,//Byte      TimeOut;
    0,//Byte      TriggerDigits;
    {0},//TFieldCharacters FieldCharacters;
    0,//Byte      Alignment;
    0,//Byte      LeadingZeros;
    &auim_IdleDialogKeys[0]//fuimDialogNavigation  * ToDoWithKey;
  }
};


const fuimFieldStruct auim_PairingMenuFields[AUIM_MNU_PAIRING_MAX_FIELD] = 
{
  /*=======================================================================*/
  /*       PAIRING_STATUS                   */
  /*=======================================================================*/
  {
    FUIM_FIELDTYPE_NUMERIC_VALUE,
    AUIM_FIELD_VALIDITY_FUNCTION,//Byte       ValidityFunction;
    FMNU_NONE_PROMPT,//osdStringID  Prompt;
    AUIM_MENU_FIELD_COLOUR,//Byte PromptColour;
    0,      //Byte ChangeFunction;
    AUIM_GET_PAIRING_CODE,//Byte      GetFunction;
    0,//Byte      SetFunction;
    {.Numeric = 4},//TFieldSize    FieldSize;
    {.Numeric = 2},//TFieldScaling   FieldScaling;
    0,//Byte      Prefix;
    0,//osdStringID   Suffix;
    FUIM_FIELD_TIMEOUT,//Byte      TimeOut;
    0,//Byte      TriggerDigits;
    {.NumericFont.size = FUIM_FONT_SIZE_SMALL, .NumericFont.color = FUIM_FONT_COLOR_1},//TFieldCharacters FieldCharacters;
    FUIM_ALIGNMENT_CENTRE,//Byte      Alignment;
    0,//Byte      LeadingZeros;
    &auim_PairingDialogKeys[0]//fuimDialogNavigation  * ToDoWithKey;
  }
};
/*=======================================================================*/
/*                                                                       */
/*=======================================================================*/



const fmnu_MenuStruct auim_Menu[AUIM_MNU_MAX_MENUS] =
{

/*=================================================================*/
/*       CONFIGURATION  MENU                                       */
/*=================================================================*/
  {
    FMNU_MAIN_MENU,      /* Menu type  {FMNU_MAIN_MENU FMNU_SUB_MAIN_MENU FMNU_MESSAGE_BOARD} */
    AUIM_MNU_X_POS+AUIM_MNU_SHIFT_X,//Position MENU X
    AUIM_MNU_Y_POS,                 //Position MENU Y
    FUIM_MENU_WIDTH,                //Menu width
    IMG_ID_PROPERTY_1_VARIANT4_8,   //ID Title MENU
    AUIM_MAIN_MENU_COLOUR,//Title Attributes
    4+AUIM_MNU_SHIFT_X,   //Position X PromptPos
    240+AUIM_MNU_SHIFT_X,  //Position X ValuePos
    &auim_ConfigurationMenuFields[0],// Pointer to an array of field structures
    1,                              // Number of fields visible at once
    AUIM_MNU_CONFIG_MAX_FIELD,// Total number of fields
    FUIM_FIELD_NO_TIMEOUT,//FUIM_MENU_TIMEOUT, // How long the menu remains active;
                       // if 0-stays active indefinitely
    &auim_MenuButtonFields[AUIM_CONFIG_MNU_LEFT_BUTTON_FIELD],                 // fuimFieldStruct const *LeftButtonField;  /* Pointer to structure of the Button field */
    &auim_MenuButtonFields[AUIM_CONFIG_MNU_RIGHT_BUTTON_FIELD]                  // fuimFieldStruct const *RightButtonField; /* Pointer to structure of the Button field */
  }

/*=================================================================*/
/*         IDLE   MENU                                      */
/*=================================================================*/
 ,{
    FMNU_MAIN_MENU,      /* Menu type  {FMNU_MAIN_MENU FMNU_SUB_MAIN_MENU FMNU_MESSAGE_BOARD} */
    AUIM_MNU_X_POS+AUIM_MNU_SHIFT_X,//Position MENU X
    AUIM_MNU_Y_POS,                 //Position MENU Y
    FUIM_MENU_WIDTH,                //Menu width
    FMNU_NONE_TITLE,   //ID Title MENU
    AUIM_MAIN_MENU_COLOUR,//Title Attributes
    0+AUIM_MNU_SHIFT_X,   //Position X PromptPos
    26+AUIM_MNU_SHIFT_X,  //Position X ValuePos
    &auim_IdleMenuFields[0],// Pointer to an array of field structures
    1,                              // Number of fields visible at once
    AUIM_MNU_IDLE_MAX_FIELD,// Total number of fields
    FUIM_FIELD_NO_TIMEOUT, // How long the menu remains active;
    // if 0 - stays active indefinitely
    &auim_MenuButtonFields[AUIM_IDLE_MNU_LEFT_BUTTON_FIELD],  // fuimFieldStruct const *LeftButtonField;  /* Pointer to structure of the Button field */
    &auim_MenuButtonFields[AUIM_IDLE_MNU_RIGHT_BUTTON_FIELD]  // fuimFieldStruct const *RightButtonField; /* Pointer to structure of the Button field */
  }

/*=================================================================*/
/*         PAIRING   MENU                                      */
/*=================================================================*/
,{
  FMNU_MAIN_MENU,      /* Menu type  {FMNU_MAIN_MENU FMNU_SUB_MAIN_MENU FMNU_MESSAGE_BOARD} */
  AUIM_MNU_X_POS+AUIM_MNU_SHIFT_X,//Position MENU X
  AUIM_MNU_Y_POS,                 //Position MENU Y
  FUIM_MENU_WIDTH,                //Menu width
  IMG_ID_PROPERTY_1_VARIANT5_8,   //ID Title MENU
  AUIM_MAIN_MENU_COLOUR,//Title Attributes
  0+AUIM_MNU_SHIFT_X,   //Position X PromptPos
  126+AUIM_MNU_SHIFT_X,  //Position X ValuePos
  &auim_PairingMenuFields[0],// Pointer to an array of field structures
  1,                              // Number of fields visible at once
  AUIM_MNU_IDLE_MAX_FIELD,// Total number of fields
  FUIM_FIELD_NO_TIMEOUT, // How long the menu remains active;
  // if 0 - stays active indefinitely
  0, //fuimFieldStruct const *LeftButtonField;  /* Pointer to structure of the Button field */
  &auim_MenuButtonFields[AUIM_PAIRING_MNU_RIGHT_BUTTON_FIELD]  // fuimFieldStruct const *RightButtonField; /* Pointer to structure of the Button field */
 }

};




/*===========================================================================*/
/* L O C A L   F U N C T I O N S                                             */
/*===========================================================================*/

/*===========================================================================*/
/*     G L O B A L   F U N C T I O N S                                       */
/*===========================================================================*/

