/*=======================================================================*/
/*                                                                       */
/*=======================================================================*/


/*=======================================================================*/
/*        I N C L U D E S                                                */
/*=======================================================================*/
#include <global.h>
#include "auim_api.h"
#include "fuim.h"
#include "fmnu.h"

#include "aukh.h"
#include "auph.h"

/*=======================================================================*/
/* I N D I C A T O R   G L O B A L   D E F I N I T I O N S         */
/*=======================================================================*/


#define  VERTICAL_LOCATION_PROGRAM_INDICATOR    2
#define  HORIZONTAL_LOCATION_PROGRAM_INDICATOR  3

/*
The timeout period is set in seconds;
    the maximum possible value before overflow the timeout counter(is Byte type) variable is 15 seconds.
*/
#define  AUIM_NO_INDICATOR_TIMEOUT              0
#define  AUIM_DEFAULT_INDICATOR_TIMEOUT			    10 //Max 15 seconds
#define  AUIM_CHARGE_BATT_INDICATOR_TIMEOUT     5
#define  AUIM_SPLASH_SCREEN_TIMEOUT      3

//const fuimDialogNavigation  auim_MenuIndicatorDialogKeys[] =/
//{
//     {AU_KEY_LEFT       , AUIM_INDICATOR_DISPLAY_MAIN_MENU} /* sentinel, do not remove !!! */
//    ,{AU_KEY_RIGHT      , AUIM_INDICATOR_DISPLAY_MAIN_MENU} /* sentinel, do not remove !!! */
//    ,{AU_KEY_INVALID       , 0} /* sentinel, do not remove !!! */
//};

/*
fuimDialogNavigation RDATA auim_SoundIndKeys[] =
{
     {AU_KEY_ESC        , AUIM_HIDE_SOUND_INDICATOR}  
    ,{AU_KEY_INVALID    , 0}  
};

fuimDialogNavigation RDATA auim_BrightnessIndKeys[] =
{
     {AU_KEY_ESC        , AUIM_HIDE_BRIGHTNESS_INDICATOR}  
    ,{AU_KEY_INVALID    , 0}  
};
fuimDialogNavigation RDATA auim_SaturationIndKeys[] =
{
     {AU_KEY_ESC        , AUIM_HIDE_SATURATION_INDICATOR}  
    ,{AU_KEY_INVALID    , 0}  
};

fuimDialogNavigation RDATA auim_MainMenuIndKeys[] =
{
     {AU_KEY_ESC        , AUIM_HIDE_MAIN_MENU_INDICATOR}  
    ,{AU_KEY_INVALID    , 0}  
};
*/ 


 /*=======================================================================*/
 /* I N D I C A T O R   F I E L D S                                       */
 /*=======================================================================*/

 /* OSD Indicator fields */
const fuimFieldStruct  auim_OsdIndicatorFields[AUIM_MAX_OSD_INDICATORS] =
{

/*=======================================================================*/
/*          B A T T E R Y     I N D I C A T O R                          */
/*=======================================================================*/

   {
   FUIM_FIELDTYPE_STRING_ID,
   AUIM_FIELD_BATTERY_INDICATOR_VALIDITY_FUNCTION,//Byte       ValidityFunction;
   FMNU_NONE_PROMPT,                              //osdStringID  Prompt;
   AUIM_INDICATOR_COLOUR,           //PromptColour
   0,                                             //Byte      ChangeFunction;
   AUIM_GET_BATTERY_INDICATOR,//Byte       GetFunction;
   0,//Byte      SetFunction;
   {0},//TFieldSize    FieldSize;      //
   {0},//TFieldScaling   FieldScaling;   //
   0,//Byte      Prefix;
   0,//osdStringID   Suffix;
   FUIM_FIELD_TIMEOUT,//Byte       TimeOut;
   0,//Byte      TriggerDigits;
   {0},//TFieldCharacters FieldCharacters;
   0,//Byte      Alignment;
   0,//Byte      LeadingZeros;
   0//fuimDialogNavigation RDATA * ToDoWithKey;
   }

   /*=======================================================================*/
   /*          B L E    C O N N E C T I O N    I N D I C A T O R            */
   /*=======================================================================*/

 ,{
   FUIM_FIELDTYPE_STRING_ID,
   AUIM_FIELD_BLE_INDICATOR_VALIDITY_FUNCTION,//Byte       ValidityFunction;
   FMNU_NONE_PROMPT,//osdStringID  Prompt;
   AUIM_INDICATOR_COLOUR,//PromptColour
   0,//Byte      ChangeFunction;
   AUIM_GET_BLE_INDICATOR,//Byte       GetFunction;
   0,//Byte      SetFunction;
   {0},//TFieldSize    FieldSize;      //
   {0},//TFieldScaling   FieldScaling;   //
   0,//Byte      Prefix;
   0,//osdStringID   Suffix;
   FUIM_FIELD_TIMEOUT,//Byte      TimeOut;
   0,//Byte      TriggerDigits;
   {0},//TFieldCharacters FieldCharacters;
   0,//Byte      Alignment;
   0,//Byte      LeadingZeros;
   0//fuimDialogNavigation RDATA * ToDoWithKey;
   }

   /*=======================================================================*/
   /*     S Y N C H R O N I S A T I O N     I N D I C A T O R            */
   /*=======================================================================*/
 ,{
   FUIM_FIELDTYPE_STRING_ID,
   AUIM_FIELD_SYNC_INDICATOR_VALIDITY_FUNCTION,//Byte       ValidityFunction;
   FMNU_NONE_PROMPT,//osdStringID  Prompt;
   AUIM_INDICATOR_COLOUR,   //    Specific Field prompt color,
   0,//Byte      ChangeFunction;
   AUIM_GET_SYNC_INDICATOR,//Byte      GetFunction;
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
   0//fuimDialogNavigation RDATA * ToDoWithKey;

   }

   /*=======================================================================*/
   /*    S P L A S H      S C R E E N        I N D I C A T O R              */
   /*=======================================================================*/
 ,{
     FUIM_FIELDTYPE_SPACER,
   AUIM_FIELD_INDICATOR_VALIDITY_FUNCTION,//Byte       ValidityFunction;
   IMG_ID_UNION3_OUT,//osdStringID  Prompt;
   AUIM_SPLASH_SCREEN_COLOUR,   //    Specific Field prompt color,
   0,//Byte      ChangeFunction;
   AUIM_NO_GET_FUNCTION,//Byte      GetFunction;
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

   /*=======================================================================*/
   /*     C H A R G E   B A T T E R Y        I N D I C A T O R              */
   /*=======================================================================*/
 ,{
   FUIM_FIELDTYPE_MODAL_NOTIFICATION,
   AUIM_FIELD_CHARGE_BATT_VALIDITY_FUNCTION,//Byte       ValidityFunction;
   IMG_ID_PROPERTY_1_VARIANT3,//osdStringID  Prompt;
   AUIM_MODAL_INDICATOR_COLOUR,   //    Specific Field prompt color,
   0,//Byte      ChangeFunction;
   AUIM_NO_GET_FUNCTION,//Byte      GetFunction;
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



 /*=======================================================================*/
/* I N D I C A T O R   S T R U C T U R E S                               */
/*=======================================================================*/

/* OSD Indicators */

const fuimIndicatorStruct auim_OsdIndicator[AUIM_MAX_OSD_INDICATORS] =
{

    {
      0 + FUIM_INDICATOR_MARGIN(10) ,  //VertLocation
      0,   //HorLocation
      22,  //Byte PromptPos;
      22,  //Byte ValuePos;   			 /* total width of the value */
      AUIM_NO_INDICATOR_TIMEOUT,  //TimeOut
      &auim_OsdIndicatorFields[AUIM_INDEX_BATTERY_INDICATOR] /* Field */
	  }

	,{
      0 + FUIM_INDICATOR_MARGIN(12),  //VertLocation
      60,  //HorLocation
      63,  //Byte PromptPos;
      63,	//Byte ValuePos;   			 /* total width of the value */
      AUIM_NO_INDICATOR_TIMEOUT,	//TimeOut
      &auim_OsdIndicatorFields[AUIM_INDEX_BLE_INDICATOR] /* Field */
	  }

	,{
      0 + FUIM_INDICATOR_MARGIN(12),  //VertLocation
      86,  //HorLocation
      92,  //Byte PromptPos;
      92,   //Byte ValuePos;   			 /* total width of the value */
      AUIM_NO_INDICATOR_TIMEOUT, //TimeOut
      &auim_OsdIndicatorFields[AUIM_INDEX_SYNC_INDICATOR] /* Field */
	 }

	,{
      37,   //VertLocation
      111,  //HorLocation
      111,  //Byte PromptPos;
      111+99, //Byte ValuePos;         /* total width of the value */
      AUIM_SPLASH_SCREEN_TIMEOUT, //TimeOut
      &auim_OsdIndicatorFields[AUIM_INDEX_SPLASH_SCREEN] /* Field */
	 }

  ,{
      89 + FUIM_INDICATOR_MARGIN(9),   //VertLocation
      0,  //HorLocation
      19, //Byte PromptPos;
      (FUIM_MENU_WIDTH - 1), //Byte ValuePos;         /* total width of the value */
      AUIM_CHARGE_BATT_INDICATOR_TIMEOUT, //TimeOut
      &auim_OsdIndicatorFields[AUIM_INDEX_CHARGE_BATT] /* Field */
  }

};






