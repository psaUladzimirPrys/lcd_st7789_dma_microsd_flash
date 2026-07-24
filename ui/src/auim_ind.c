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
#define  AUIM_NO_INDICATOR_TIMEOUT                          0
#define  AUIM_DEFAULT_INDICATOR_TIMEOUT			                10 //10 means the Max 15 seconds
#define  AUIM_CHARGE_BATT_INDICATOR_TIMEOUT                 5
#define  AUIM_SPLASH_SCREEN_TIMEOUT                         3
#define  AUIM_ONE_INDENT_REQUIRED_INDICATOR_TIMEOUT         2
#define  AUIM_PERF_CHK_RECOMMENDED_INDICATOR_TIMEOUT        3

//const fuimDialogNavigation  auim_MenuIndicatorDialogKeys[] =/
//{
//     {AU_KEY_LEFT       , AUIM_INDICATOR_DISPLAY_MAIN_MENU} /* sentinel, do not remove !!! */
//    ,{AU_KEY_RIGHT      , AUIM_INDICATOR_DISPLAY_MAIN_MENU} /* sentinel, do not remove !!! */
//    ,{AU_KEY_INVALID    , 0} /* sentinel, do not remove !!! */
//};

/*
fuimDialogNavigation  auim_MainMenuIndKeys[] = {
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
  /*      B A T T E R Y                    I N D I C A T O R               */
  /*=======================================================================*/
   {
   FUIM_FIELDTYPE_STRING,
   AUIM_FIELD_BATTERY_INDICATOR_VALIDITY_FUNCTION,//Byte       ValidityFunction;
   AUIM_NONE_PROMPT,                              //Byte Prompt;
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
   /*     B L E    C O N N E C T I O N     I N D I C A T O R                */
   /*=======================================================================*/
 ,{
   FUIM_FIELDTYPE_STRING,
   AUIM_FIELD_BLE_INDICATOR_VALIDITY_FUNCTION,//Byte       ValidityFunction;
   AUIM_NONE_PROMPT,//Byte Prompt;
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
   /*     S Y N C H R O N I S A T I O N    I N D I C A T O R                */
   /*=======================================================================*/
 ,{
   FUIM_FIELDTYPE_STRING,
   AUIM_FIELD_SYNC_INDICATOR_VALIDITY_FUNCTION,//Byte       ValidityFunction;
   AUIM_NONE_PROMPT,//Byte Prompt;
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
   /*     S P L A S H      S C R E E N     I N D I C A T O R                */
   /*=======================================================================*/
 ,{
   FUIM_FIELDTYPE_SPACER,
   AUIM_FIELD_INDICATOR_VALIDITY_FUNCTION,//Byte       ValidityFunction;
   AUIM_GET_UNION3_OUT_IND,//Byte Prompt;
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
   /*     C H A R G E   B A T T E R Y      I N D I C A T O R                */
   /*=======================================================================*/
 ,{
   FUIM_FIELDTYPE_MODAL_NOTIFICATION,
   AUIM_FIELD_CHARGE_BATT_VALIDITY_FUNCTION,//Byte       ValidityFunction;
   AUIM_GET_CHARGE_BATTERY_IND,//Byte Prompt;
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
   /*=======================================================================*/
   /*     P E R F O R M A N C E            I N D I C A T O R                */
   /*=======================================================================*/
   ,{
     FUIM_FIELDTYPE_MODAL_NOTIFICATION,
     AUIM_FIELD_INDICATOR_VALIDITY_FUNCTION,//Byte       ValidityFunction;
     AUIM_GET_PERFORMANCE_IND,//Byte Prompt;
     AUIM_PERFORMANCE_NOTIFICATION_COLOUR,   //    Specific Field prompt color,
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
   /*     P A T I E N T                    I N D I C A T O R                */
   /*=======================================================================*/
   ,{
     FUIM_FIELDTYPE_MODAL_NOTIFICATION,
     AUIM_FIELD_INDICATOR_VALIDITY_FUNCTION,//Byte       ValidityFunction;
     AUIM_GET_PATIENT_IND,//Byte Prompt;
     AUIM_PATIENT_NOTIFICATION_COLOUR,   //    Specific Field prompt color,
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
   /*     R E F E R E N C E                I N D I C A T O R                */
   /*=======================================================================*/
   ,{
     FUIM_FIELDTYPE_MODAL_NOTIFICATION,
     AUIM_FIELD_INDICATOR_VALIDITY_FUNCTION,//Byte       ValidityFunction;
     AUIM_GET_REFERENCE_IND,//Byte Prompt;
     AUIM_REFERENCE_NOTIFICATION_COLOUR,   //    Specific Field prompt color,
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
   /*     O N E   I N D E N T   R E Q U I R E D   I N D I C A T O R         */
   /*=======================================================================*/
   ,{
     FUIM_FIELDTYPE_MODAL_NOTIFICATION,
     AUIM_FIELD_INDICATOR_VALIDITY_FUNCTION,//Byte       ValidityFunction;
     AUIM_GET_ONE_INDENT_IND,//Byte Prompt;
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
   /*=======================================================================*/
   /*     T I P   I D   V A L I D          I N D I C A T O R                */
   /*=======================================================================*/
   ,{
     FUIM_FIELDTYPE_MODAL_NOTIFICATION,
     AUIM_FIELD_INDICATOR_VALIDITY_FUNCTION,//Byte       ValidityFunction;
     AUIM_GET_TIP_ID_VALID_IND,//Byte Prompt;
     AUIM_TIP_ID_GREEN_MODAL_COLOUR,   //    Specific Field prompt color,
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
   /*     T I P   I D   I N V A L I D      I N D I C A T O R                */
   /*=======================================================================*/
   ,{
     FUIM_FIELDTYPE_MODAL_NOTIFICATION,
     AUIM_FIELD_INDICATOR_VALIDITY_FUNCTION,//Byte       ValidityFunction;
     AUIM_GET_TIP_ID_INVALID_IND,//Byte Prompt;
     AUIM_TIP_ID_RED_MODAL_COLOUR,   //    Specific Field prompt color,
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
   /*     T I P   I D   U S E D            I N D I C A T O R                */
   /*=======================================================================*/
   ,{
     FUIM_FIELDTYPE_MODAL_NOTIFICATION,
     AUIM_FIELD_INDICATOR_VALIDITY_FUNCTION,//Byte       ValidityFunction;
     AUIM_GET_TIP_ID_USED_IND,//Byte Prompt;
     AUIM_TIP_ID_RED_MODAL_COLOUR,   //    Specific Field prompt color,
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
   /*     U N S T A B L E                  I N D I C A T O R                */
   /*=======================================================================*/
   ,{
     FUIM_FIELDTYPE_MODAL_NOTIFICATION,
     AUIM_FIELD_INDICATOR_VALIDITY_FUNCTION,//Byte       ValidityFunction;
     AUIM_GET_UNSTABLE_IND,//Byte Prompt;
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
   /*=======================================================================*/
   /*     T E R M I N A T E                I N D I C A T O R                */
   /*=======================================================================*/
   ,{
     FUIM_FIELDTYPE_MODAL_NOTIFICATION,
     AUIM_FIELD_INDICATOR_VALIDITY_FUNCTION,//Byte       ValidityFunction;
     AUIM_GET_TERMINATE_IND,//Byte Prompt;
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
   /*=======================================================================*/
   /*     E R R O R                        I N D I C A T O R                */
   /*=======================================================================*/
   ,{
     FUIM_FIELDTYPE_NUMERIC,
     AUIM_FIELD_INDICATOR_VALIDITY_FUNCTION,//Byte       ValidityFunction;
     AUIM_GET_ERROR_IND,//Byte Prompt;
     AUIM_ERROR_INDICATOR_COLOUR,   //    Specific Field prompt color,
     0,//Byte      ChangeFunction;
     AUIM_GET_ERROR_CODE_NUMBER,//Byte      GetFunction;
     0,//Byte      SetFunction;
     {.Numeric = 3},//TFieldSize    FieldSize;
     {0},//TFieldScaling   FieldScaling;
     0,//Byte      Prefix;
     0,//osdStringID   Suffix;
     FUIM_FIELD_NO_TIMEOUT,//Byte      TimeOut;
     0,//Byte      TriggerDigits;
     {.NumericFont.size = FUIM_FONT_SIZE_LARGE, .NumericFont.color = FUIM_FONT_COLOR_4},//TFieldCharacters FieldCharacters;
     FUIM_ALIGNMENT_CENTRE,//Byte      Alignment;
     0,//Byte      LeadingZeros;
     0//fuimDialogNavigation RDATA * ToDoWithKey;
   }
   /*=======================================================================*/
   /*     P E R F   C H K   R E C O M M E N D E D   I N D I C A T O R        */
   /*=======================================================================*/
   ,{
     FUIM_FIELDTYPE_MODAL_NOTIFICATION,
     AUIM_FIELD_INDICATOR_VALIDITY_FUNCTION,//Byte       ValidityFunction;
     AUIM_GET_PERF_CHK_RECOMMENDED,//Byte Prompt;
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
    65,  //Byte PromptPos;
    65,	//Byte ValuePos;   			 /* total width of the value */
    AUIM_NO_INDICATOR_TIMEOUT,	//TimeOut
    &auim_OsdIndicatorFields[AUIM_INDEX_BLE_INDICATOR] /* Field */
	  }
	,{
    0 + FUIM_INDICATOR_MARGIN(12),  //VertLocation
    86,  //HorLocation
    93,  //Byte PromptPos;
    93,   //Byte ValuePos;   			 /* total width of the value */
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
    FUIM_MENU_WIDTH, //Byte ValuePos;         /* total width of the value */
    AUIM_NO_INDICATOR_TIMEOUT, //AUIM_CHARGE_BATT_INDICATOR_TIMEOUT, //TimeOut
    &auim_OsdIndicatorFields[AUIM_INDEX_CHARGE_BATT] /* Field */
  }
  ,{
    89 + FUIM_INDICATOR_MARGIN(9),   //VertLocation
    0,  //HorLocation
    63, //Byte PromptPos;
    FUIM_MENU_WIDTH, //Byte ValuePos;         /* total width of the value */
    AUIM_NO_INDICATOR_TIMEOUT, //TimeOut
    &auim_OsdIndicatorFields[AUIM_INDEX_PERFORMANCE_INDICATOR] /* Field */
  }
  ,{
    89 + FUIM_INDICATOR_MARGIN(9),   //VertLocation
    0,  //HorLocation
    106, //Byte PromptPos;
    FUIM_MENU_WIDTH, //Byte ValuePos;         /* total width of the value */
    AUIM_NO_INDICATOR_TIMEOUT, //TimeOut
    &auim_OsdIndicatorFields[AUIM_INDEX_PATIENT_INDICATOR] /* Field */
  }
  ,{
    89 + FUIM_INDICATOR_MARGIN(9),   //VertLocation
    0,  //HorLocation
    86, //Byte PromptPos;
    FUIM_MENU_WIDTH, //Byte ValuePos;         /* total width of the value */
    AUIM_NO_INDICATOR_TIMEOUT, //TimeOut
    &auim_OsdIndicatorFields[AUIM_INDEX_REFERENCE_INDICATOR] /* Field */
  }
  ,{
    89 + FUIM_INDICATOR_MARGIN(9),   //VertLocation
    0,  //HorLocation
    27, //Byte PromptPos;
    FUIM_MENU_WIDTH, //Byte ValuePos;         /* total width of the value */
    AUIM_ONE_INDENT_REQUIRED_INDICATOR_TIMEOUT, //TimeOut
    &auim_OsdIndicatorFields[AUIM_INDEX_ONE_INDENT_REQUIRED] /* Field */
  }
  ,{
    89 + FUIM_INDICATOR_MARGIN(9),   //VertLocation
    0,  //HorLocation
    82, //Byte PromptPos;
    FUIM_MENU_WIDTH, //Byte ValuePos;         /* total width of the value */
    AUIM_NO_INDICATOR_TIMEOUT, //TimeOut
    &auim_OsdIndicatorFields[AUIM_INDEX_TIP_ID_VALID] /* Field */
  }
  ,{
    89 + FUIM_INDICATOR_MARGIN(9),   //VertLocation
    0,  //HorLocation
    68, //Byte PromptPos;
    FUIM_MENU_WIDTH, //Byte ValuePos;         /* total width of the value */
    AUIM_NO_INDICATOR_TIMEOUT, //TimeOut
    &auim_OsdIndicatorFields[AUIM_INDEX_TIP_ID_INVALID] /* Field */
  }
  ,{
    89 + FUIM_INDICATOR_MARGIN(9),   //VertLocation
    0,  //HorLocation
    86, //Byte PromptPos;
    FUIM_MENU_WIDTH, //Byte ValuePos;         /* total width of the value */
    AUIM_NO_INDICATOR_TIMEOUT, //TimeOut
    &auim_OsdIndicatorFields[AUIM_INDEX_TIP_ID_USED] /* Field */
  }
  ,{
    89 + FUIM_INDICATOR_MARGIN(9),   //VertLocation
    0,  //HorLocation
    24, //Byte PromptPos;
    FUIM_MENU_WIDTH, //Byte ValuePos;         /* total width of the value */
    AUIM_NO_INDICATOR_TIMEOUT, //TimeOut
    &auim_OsdIndicatorFields[AUIM_INDEX_UNSTABLE] /* Field */
  }
  ,{
    89 + FUIM_INDICATOR_MARGIN(9),   //VertLocation
    0,  //HorLocation
    79, //Byte PromptPos;
    FUIM_MENU_WIDTH, //Byte ValuePos;         /* total width of the value */
    AUIM_NO_INDICATOR_TIMEOUT, //TimeOut
    &auim_OsdIndicatorFields[AUIM_INDEX_TERMINATE_INDICATOR] /* Field */
  }
  ,{
    FUIM_MENU_FIELD_LARGE_TOP_MARGIN + FUIM_INDICATOR_MARGIN(6),   //VertLocation
    0,  //HorLocation
    36, //Byte PromptPos;
    162, //Byte ValuePos;         /* total width of the value */
    AUIM_NO_INDICATOR_TIMEOUT, //TimeOut
    &auim_OsdIndicatorFields[AUIM_INDEX_ERROR_INDICATOR] /* Field */
  }
  ,{
    89 + FUIM_INDICATOR_MARGIN(9),   //VertLocation
    0,  //HorLocation
    13, //Byte PromptPos;
    FUIM_MENU_WIDTH, //Byte ValuePos;
    AUIM_NO_INDICATOR_TIMEOUT,//AUIM_PERF_CHK_RECOMMENDED_INDICATOR_TIMEOUT, //TimeOut
    &auim_OsdIndicatorFields[AUIM_INDEX_PERF_CHK_RECOMMENDED] /* Field */
  }
};








