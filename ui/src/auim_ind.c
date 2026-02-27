/*=======================================================================*/
/*                                */
/*=======================================================================*/


/*=======================================================================*/
/*        I N C L U D E S                                                */
/*=======================================================================*/
#include "Hglobal.h"
#include "auim_api.h"
#include "fuim.h"
#include "fmnu.h"

#include "aukh.h"
#include "auph.h"

/*=======================================================================*/
/* I N D I C A T O R    � � � � � � � � � � � �    � �     � � � � � �   */
/*=======================================================================*/


#define  VERTICAL_LOCATION_PROGRAM_INDICATOR    2
#define  HORIZONTAL_LOCATION_PROGRAM_INDICATOR  3

#define  AUIM_DEFAULT_INDICATOR_TIMEOUT			10 //15- ��� ��������
#define  AUIM_NONE_PROGRAMM_INDICATOR_TIMEOUT   3

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
   {
   FUIM_FIELDTYPE_STRING,
   AUIM_FIELD_VALIDITY_FUNCTION,//Byte       ValidityFunction;
   FMNU_NONE_PROMPT,//osdStringID  Prompt;
   AUIM_TIME_HILIGHTE_INDICATOR_COLOUR,//PromptColour
   0,//Byte      ChangeFunction;
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

   ,{
   FUIM_FIELDTYPE_STRING,
   AUIM_FIELD_VALIDITY_FUNCTION,//Byte       ValidityFunction;
   FMNU_NONE_PROMPT,//osdStringID  Prompt;
   AUIM_TIME_HILIGHTE_INDICATOR_COLOUR,//PromptColour
   0,//Byte      ChangeFunction;
   AUIM_GET_BLE_INDICATOR,//Byte       GetFunction;
   0,//Byte      SetFunction;
   {0},//TFieldSize    FieldSize;      //
   {0},//TFieldScaling   FieldScaling;   //
   0,//Byte      Prefix;
   0,//osdStringID   Suffix;
   FUIM_FIELD_NO_TIMEOUT,//Byte      TimeOut;
   0,//Byte      TriggerDigits;
   {0},//TFieldCharacters FieldCharacters;
   0,//Byte      Alignment;
   0,//Byte      LeadingZeros;
   0//fuimDialogNavigation RDATA * ToDoWithKey;
   }

   ,{
   FUIM_FIELDTYPE_STRING,
   AUIM_FIELD_VALIDITY_FUNCTION,//Byte       ValidityFunction;
   FMNU_NONE_PROMPT,//osdStringID  Prompt;
   AUIM_MUTE_INDICATOR_COLOUR,   //    Specific Field prompt color,
   0,//Byte      ChangeFunction;
   AUIM_INDEX_SYNC_INDICATOR,//Byte      GetFunction;
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
	  0,	//VertLocation
	  3,  //HorLocation
    5,  //Byte ValuePos;   			 /* total width of the value */
	  6,  //TimeOut
 	  &auim_OsdIndicatorFields[AUIM_INDEX_BATTERY_INDICATOR] /* Field */
	  }

	,{
	  1,  //VertLocation
	  3,  //HorLocation
	  5,	//Byte ValuePos;   			 /* total width of the value */
	  4,	//TimeOut
	  &auim_OsdIndicatorFields[AUIM_INDEX_BLE_INDICATOR] /* Field */
	  }

	,{
	  7,    //VertLocation
	  37+4, //HorLocation
	  37+6,	//Byte ValuePos;   			 /* total width of the value */
	  0, //TimeOut
	  &auim_OsdIndicatorFields[ AUIM_INDEX_SYNC_INDICATOR] /* Field */
	 }


};



