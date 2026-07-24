/*
 * auim_mnu.c
 *
 *  Created on: 20 Feb. 2026 г.
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
const fmnu_ListStruct ListDataGoodBad[] =
{
  { FSRV_GAUGE_STATUS_BAD,   FMNU_LIST_ITEMS_BAD  }
 ,{ FSRV_GAUGE_STATUS_GOOD,  FMNU_LIST_ITEMS_GOOD }
 ,{ 255,                     FMNU_LIST_ITEMS_ERROR}
};


/*==========================================================================*/
/*  I D L E    D I A L O G     N A V I G A T I O N      K E Y S             */
/*==========================================================================*/
fuimDialogNavigation auim_IdleDialogKeys[] = {
   {AU_VIRTUAL_KEY_4  ,                AUIM_DISPLAY_CONFIGURATION_MENU            }
  ,{AU_VIRTUAL_KEY_5  ,                AUIM_DISPLAY_PAIRING_MENU                  }
  ,{AU_VIRTUAL_KEY_1  ,                AUIM_DISPLAY_PERFORMANCE_MENU              }
  ,{AU_PERFORMANCE_MENU_START,         AUIM_DISPLAY_PERFORMANCE_MENU              }
  ,{AU_PATIENT_MENU_VALIDATING,        AUIM_DISPLAY_PATIENT_VALIDATING_MENU       }
  ,{AU_PATIENT_MENU_VALIDATING_RESULT, AUIM_DISPLAY_PATIENT_VALIDATING_RESULT_MENU}
  ,{AU_MENU_CHARGE_BATTERY_ENTER     , AUIM_DISPLAY_CHARGE_BATTERY_MENU           }

//  ,{AU_VIRTUAL_KEY_6,                  AUIM_DISPLAY_CHARGE_BATTERY_MENU           }  /* @ToDo UP temporary button to start Charge Bat Menu   */

  ,{AU_KEY_INVALID    , 0} /* sentinel, do not remove !!! */
};


/*=======================================================================*/
/*   P A I R I N G    N A V I G A T I O N      K E Y S                   */
/*=======================================================================*/
fuimDialogNavigation auim_PairingDialogKeys[] = {
   {AU_VIRTUAL_KEY_2        ,   AUIM_DISPLAY_PAIRING_CANCELED_MENU    }
  ,{AU_PAIRING_MENU_OK      ,   AUIM_DISPLAY_PAIRING_OK_CLOSE_MENU    }
  ,{AU_PAIRING_MENU_FAILED  ,   AUIM_DISPLAY_PAIRING_FAILED_CLOSE_MENU}
  ,{AU_KEY_INVALID    , 0} /* sentinel, do not remove !!! */
};

fuimDialogNavigation auim_PairingCloseDialogKeys[] = {
   {AU_VIRTUAL_KEY_1  , AUIM_DISPLAY_IDLE_MENU}
  ,{AU_KEY_INVALID    , 0} /* sentinel, do not remove !!! */
};


/*=======================================================================*/
/*   C O N F I G U R A T I O N    N A V I G A T I O N      K E Y S       */
/*=======================================================================*/
fuimDialogNavigation auim_ConfigurationDialogKeys[] = {
   {AU_VIRTUAL_KEY_2              , AUIM_ACTION_CONFIGURATION_CANCELED  }
  ,{AU_VIRTUAL_KEY_1              , AUIM_ACTION_CONFIGURATION_NEXT_FIELD}
  ,{AU_MENU_CHARGE_BATTERY_ENTER  , AUIM_DISPLAY_CHARGE_BATTERY_MENU    }
  ,{AU_KEY_INVALID    , 0} /* sentinel, do not remove !!! */
};


/*=======================================================================*/
/*   P A T I E N T      N A V I G A T I O N      K E Y S         */
/*=======================================================================*/
fuimDialogNavigation auim_PatientDialogKeys[] = {
   {AU_VIRTUAL_KEY_2  , AUIM_ACTION_SEND_STPM_PACKET      }
  ,{AU_VIRTUAL_KEY_1  , AUIM_DISPLAY_PATIENT_CONTINUE_MENU}
  ,{AU_KEY_INVALID    , 0} /* sentinel, do not remove !!! */
};

fuimDialogNavigation auim_PatientContinueDialogKeys[] = {
   {AU_VIRTUAL_KEY_2              , AUIM_DISPLAY_PATIENT_TERMINATE_MENU}
  ,{AU_PATIENT_MENU_FINISHED      , AUIM_DISPLAY_PATIENT_COMPLETE_MENU}
  ,{AU_KEY_INVALID    , 0} /* sentinel, do not remove !!! */
};

fuimDialogNavigation auim_PatientCanceledDialogKeys[] = {
   {AU_VIRTUAL_KEY_1              , AUIM_DISPLAY_PATIENT_FAILED_MENU}
  ,{AU_KEY_INVALID    , 0} /* sentinel, do not remove !!! */
};

fuimDialogNavigation auim_PatientCompleteDialogKeys[] = {
   {AU_REFERENCE_MENU_START     , AUIM_DISPLAY_REFERENCE_MENU     }
  ,{AU_PATIENT_MENU_CHECK_END   , AUIM_DISPLAY_PATIENT_RESULT_MENU}
  ,{AU_PATIENT_MENU_FAILED      , AUIM_DISPLAY_PATIENT_FAILED_MENU}
  ,{AU_KEY_INVALID    , 0} /* sentinel, do not remove !!! */
};

fuimDialogNavigation auim_PatientTerminateDialogKeys[] = {
   {AU_VIRTUAL_KEY_2            , AUIM_DISPLAY_PATIENT_CANCELED_MENU}
  ,{AU_VIRTUAL_KEY_1            , AUIM_DISPLAY_PATIENT_CONTINUE_MENU}
  ,{AU_KEY_INVALID    , 0} /* sentinel, do not remove !!! */
};

fuimDialogNavigation auim_PatientResultDialogKeys[] = {
   {AU_VIRTUAL_KEY_1             , AUIM_DISPLAY_IDLE_MENU}
  ,{AU_KEY_INVALID    , 0} /* sentinel, do not remove !!! */
};

fuimDialogNavigation auim_PatientFailedDialogKeys[] = {
   {AU_VIRTUAL_KEY_1             , AUIM_DISPLAY_IDLE_MENU}
  ,{AU_KEY_INVALID    , 0} /* sentinel, do not remove !!! */
};

fuimDialogNavigation auim_PatientValidatingDialogKeys[] = {
    {AU_VIRTUAL_KEY_2                    , AUIM_DISPLAY_IDLE_MENU}
   ,{AU_PATIENT_MENU_VALIDATING_RESULT   , AUIM_DISPLAY_PATIENT_VALIDATING_RESULT_MENU}
   ,{AU_KEY_INVALID    , 0} /* sentinel, do not remove !!! */
};

fuimDialogNavigation auim_PatientValidatingCloseDialogKeys[] = {
   {AU_VIRTUAL_KEY_1           , AUIM_DISPLAY_IDLE_MENU    }
  ,{AU_PATIENT_MENU_START      , AUIM_DISPLAY_PATIENT_MENU }
  ,{AU_KEY_INVALID    , 0} /* sentinel, do not remove !!! */
};


/*=======================================================================*/
/*   P E R F O R M A N C E      N A V I G A T I O N      K E Y S         */
/*=======================================================================*/
fuimDialogNavigation auim_PerformanceDialogKeys[] = {
   {AU_VIRTUAL_KEY_2  , AUIM_ACTION_SEND_STPM_PACKET}
  ,{AU_VIRTUAL_KEY_1  , AUIM_DISPLAY_PERFORMANCE_CONTINUE_MENU}
  ,{AU_KEY_INVALID    , 0} /* sentinel, do not remove !!! */
};

fuimDialogNavigation auim_PerformanceContinueDialogKeys[] = {
   {AU_VIRTUAL_KEY_2              , AUIM_DISPLAY_PERFORMANCE_TERMINATE_MENU}
  ,{AU_PERFORMANCE_MENU_FINISHED  , AUIM_DISPLAY_PERFORMANCE_COMPLETE_MENU}
  ,{AU_KEY_INVALID    , 0} /* sentinel, do not remove !!! */
};

fuimDialogNavigation auim_PerformanceCanceledDialogKeys[] = {
   {AU_VIRTUAL_KEY_1  , AUIM_DISPLAY_PERFORMANCE_RESULT_MENU}
  ,{AU_KEY_INVALID    , 0} /* sentinel, do not remove !!! */
};

fuimDialogNavigation auim_PerformanceCompleteDialogKeys[] = {
//   {AU_VIRTUAL_KEY_2          , AUIM_DISPLAY_PERFORMANCE_TERMINATE_MENU}

   {AU_REFERENCE_MENU_START        , AUIM_DISPLAY_REFERENCE_MENU         }
  ,{AU_PERFORMANCE_MENU_CHECK_END  , AUIM_DISPLAY_PERFORMANCE_RESULT_MENU}
  ,{AU_KEY_INVALID    , 0} /* sentinel, do not remove !!! */
};

fuimDialogNavigation auim_PerformanceTerminateDialogKeys[] = {
  {AU_VIRTUAL_KEY_2  , AUIM_DISPLAY_PERFORMANCE_CANCELED_MENU}
  ,{AU_VIRTUAL_KEY_1  , AUIM_DISPLAY_PERFORMANCE_CONTINUE_MENU}
  ,{AU_KEY_INVALID    , 0} /* sentinel, do not remove !!! */
};

fuimDialogNavigation auim_PerformanceResultDialogKeys[] = {
   {AU_VIRTUAL_KEY_1  , AUIM_DISPLAY_IDLE_MENU}
  ,{AU_KEY_INVALID    , 0} /* sentinel, do not remove !!! */
};


/*=======================================================================*/
/*   R E F E R E N C E       N A V I G A T I O N        K E Y S          */
/*=======================================================================*/
fuimDialogNavigation auim_ReferenceDialogKeys[] = {
   {AU_VIRTUAL_KEY_2  , AUIM_DISPLAY_REFERENCE_TERMINATE_MENU}
  ,{AU_VIRTUAL_KEY_1  , AUIM_DISPLAY_REFERENCE_CONTINUE_MENU}
  ,{AU_KEY_INVALID    , 0} /* sentinel, do not remove !!! */
};

fuimDialogNavigation auim_ReferenceContinueDialogKeys[] = {
   {AU_VIRTUAL_KEY_2                       , AUIM_DISPLAY_REFERENCE_TERMINATE_MENU       }
  ,{AU_REFERENCE_MENU_FINISHED             , AUIM_DISPLAY_REFERENCE_COMPLETE_MENU        }
  ,{AU_REFERENCE_PERFORM_MENU_REPEAT_START , AUIM_DISPLAY_REFERENCE_UNSTABLE_REPEAT_MENU }
  ,{AU_REFERENCE_PATIENT_MENU_REPEAT_START , AUIM_DISPLAY_REFERENCE_UNSTABLE_REPEAT_MENU }
  ,{AU_PERFORMANCE_MENU_FINISHED           , AUIM_DISPLAY_PERFORMANCE_COMPLETE_MENU      }
  ,{AU_KEY_INVALID    , 0} /* sentinel, do not remove !!! */
};

fuimDialogNavigation auim_ReferenceCanceledPerformanceDialogKeys[] = {
   {AU_VIRTUAL_KEY_1  , AUIM_DISPLAY_PERFORMANCE_RESULT_MENU}
  ,{AU_KEY_INVALID    , 0} /* sentinel, do not remove !!! */
};

fuimDialogNavigation auim_ReferenceCanceledPatientDialogKeys[] = {
   {AU_VIRTUAL_KEY_1  , AUIM_DISPLAY_PATIENT_RESULT_MENU}
  ,{AU_KEY_INVALID    , 0} /* sentinel, do not remove !!! */
};

fuimDialogNavigation auim_ReferenceCompleteDialogKeys[] = {
   {AU_PERFORMANCE_MENU_CHECK_END , AUIM_DISPLAY_PERFORMANCE_RESULT_MENU}
  ,{AU_PATIENT_MENU_CHECK_END     , AUIM_DISPLAY_PATIENT_RESULT_MENU}
  ,{AU_PATIENT_MENU_FAILED        , AUIM_DISPLAY_PATIENT_FAILED_MENU}
  ,{AU_KEY_INVALID    , 0} /* sentinel, do not remove !!! */

};

fuimDialogNavigation auim_ReferenceTerminateDialogKeys[] = {
   {AU_VIRTUAL_KEY_2  , AUIM_DISPLAY_REFERENCE_CANCELED_MENU}
  ,{AU_VIRTUAL_KEY_1  , AUIM_DISPLAY_REFERENCE_CONTINUE_MENU}
  ,{AU_KEY_INVALID    , 0} /* sentinel, do not remove !!! */
};

fuimDialogNavigation auim_ReferenceUnstablePerformanceDialogKeys[] = {
   {AU_VIRTUAL_KEY_2  , AUIM_DISPLAY_PERFORMANCE_RESULT_MENU}
  ,{AU_VIRTUAL_KEY_1  , AUIM_DISPLAY_REFERENCE_CONTINUE_MENU}
  ,{AU_KEY_INVALID    , 0} /* sentinel, do not remove !!! */
};

fuimDialogNavigation auim_ReferenceUnstablePatientDialogKeys[] = {
   {AU_VIRTUAL_KEY_2  , AUIM_DISPLAY_PATIENT_RESULT_MENU}
  ,{AU_VIRTUAL_KEY_1  , AUIM_DISPLAY_REFERENCE_CONTINUE_MENU}
  ,{AU_KEY_INVALID    , 0} /* sentinel, do not remove !!! */
};

/*=======================================================================*/
/*  C H A R G E   B A T T E R Y      N A V I G A T I O N       K E Y S   */
/*=======================================================================*/

fuimDialogNavigation auim_ChargeBatteryDialogKeys[] = {
    {AU_VIRTUAL_KEY_4                 , AUIM_DISPLAY_CONFIGURATION_MENU }
   ,{AU_MENU_CHARGE_BATTERY_CANCELED  , AUIM_DISPLAY_IDLE_MENU          }

  ,{AU_KEY_INVALID    , 0} /* sentinel, do not remove !!! */
};


/*=======================================================================*/
/*                                                                       */
/*=======================================================================*/

const fuimFieldStruct auim_MenuButtonFields[AUIM_MNU_MAX_BUTTON_FIELDS] =
{
  /*=======================================================================*/
  /*            L E F T       B U T T O N    -   P E R F   C H K           */
  /*=======================================================================*/
  {
    FUIM_FIELDTYPE_BUTTON,
    AUIM_FIELD_BUTTON_VALIDITY_FUNCTION,     //Byte       ValidityFunction;
    AUIM_GET_BUTTON_ONE_DOT,    //osdStringID   Prompt;
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
  /*            R I G H T     B U T T O N    -   P A R A M S               */
  /*=======================================================================*/
  ,{
    FUIM_FIELDTYPE_BUTTON,
    AUIM_FIELD_BUTTON_VALIDITY_FUNCTION,//Byte       ValidityFunction;
    AUIM_GET_BUTTON_THREE_DOTS,//Byte Prompt;
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
  /*            L E F T       B U T T O N    -   N E X T                   */
  /*=======================================================================*/
 ,{
    FUIM_FIELDTYPE_BUTTON,
    AUIM_FIELD_BUTTON_VALIDITY_FUNCTION,     //Byte       ValidityFunction;
    AUIM_GET_BUTTON_ONE_DOT,    //osdStringID   Prompt;
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
  /*            L E F T       B U T T O N    -   S T A R T                 */
  /*=======================================================================*/
  ,{
    FUIM_FIELDTYPE_BUTTON,
    AUIM_FIELD_BUTTON_VALIDITY_FUNCTION,     //Byte       ValidityFunction;
    AUIM_GET_BUTTON_ONE_DOT,    //osdStringID   Prompt;
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
    {.Button = IMG_ID_PROPERTY_1_DEFAULT_13},//TFieldCharacters FieldCharacters;
    FUIM_ALIGNMENT_CENTRE,//Byte      Alignment;
    0,//Byte      LeadingZeros;
    0//fuimDialogNavigation  * ToDoWithKey;
  }
  /*=======================================================================*/
  /*            L E F T       B U T T O N    -   C A N C E L               */
  /*=======================================================================*/
  ,{
    FUIM_FIELDTYPE_BUTTON,
    AUIM_FIELD_BUTTON_VALIDITY_FUNCTION,//Byte       ValidityFunction;
    AUIM_GET_BUTTON_ELLIPSE,//Byte Prompt;
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
  /*            L E F T       B U T T O N    -   N O                       */
  /*=======================================================================*/
  ,{
    FUIM_FIELDTYPE_BUTTON,
    AUIM_FIELD_BUTTON_VALIDITY_FUNCTION,     //Byte       ValidityFunction;
    AUIM_GET_BUTTON_ONE_DOT,    //osdStringID   Prompt;
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
    {.Button = IMG_ID_PROPERTY_1_DEFAULT_8},//TFieldCharacters FieldCharacters;
    FUIM_ALIGNMENT_CENTRE,//Byte      Alignment;
    0,//Byte      LeadingZeros;
    0//fuimDialogNavigation  * ToDoWithKey;
  }
  /*=======================================================================*/
  /*            R I G H T     B U T T O N    -   Y E S                     */
  /*=======================================================================*/
  ,{
    FUIM_FIELDTYPE_BUTTON,
    AUIM_FIELD_BUTTON_VALIDITY_FUNCTION,//Byte       ValidityFunction;
    AUIM_GET_BUTTON_ELLIPSE,//Byte Prompt;
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
    {.Button = IMG_ID_PROPERTY_1_DEFAULT_9},//TFieldCharacters FieldCharacters;
    FUIM_ALIGNMENT_CENTRE,//Byte      Alignment;
    0,//Byte      LeadingZeros;
    0//fuimDialogNavigation  * ToDoWithKey;
  }
  /*=======================================================================*/
  /*            R I G H T     B U T T O N    -   C L O S E                 */
  /*=======================================================================*/
  ,{
    FUIM_FIELDTYPE_BUTTON,
    AUIM_FIELD_BUTTON_VALIDITY_FUNCTION,//Byte       ValidityFunction;
    AUIM_GET_BUTTON_ONE_DOT,//Byte Prompt;
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
    {.Button = IMG_ID_PROPERTY_1_DEFAULT_10},//TFieldCharacters FieldCharacters;
    FUIM_ALIGNMENT_CENTRE,//Byte      Alignment;
    0,//Byte      LeadingZeros;
    0//fuimDialogNavigation  * ToDoWithKey;
  }
  /*=======================================================================*/
  /*            L E F T       B U T T O N    -   R E P E A T   R E F       */
  /*=======================================================================*/
  ,{
    FUIM_FIELDTYPE_BUTTON,
    AUIM_FIELD_BUTTON_VALIDITY_FUNCTION,     //Byte       ValidityFunction;
    AUIM_GET_BUTTON_ONE_DOT,    //osdStringID   Prompt;
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
    {.Button = IMG_ID_PROPERTY_1_VARIANT16},//TFieldCharacters FieldCharacters;
    FUIM_ALIGNMENT_CENTRE,//Byte      Alignment;
    0,//Byte      LeadingZeros;
    0//fuimDialogNavigation  * ToDoWithKey;
  }
  /*=======================================================================*/
  /*            R I G H T     B U T T O N    -  S  K I P              */
  /*=======================================================================*/
  ,{
    FUIM_FIELDTYPE_BUTTON,
    AUIM_FIELD_BUTTON_VALIDITY_FUNCTION,//Byte       ValidityFunction;
    AUIM_GET_BUTTON_ELLIPSE,//Byte Prompt;
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
    {.Button = IMG_ID_PROPERTY_1_VARIANT17},//TFieldCharacters FieldCharacters;
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
    AUIM_FIELD_VALIDITY_FUNCTION,//Byte       ValidityFunction;
    AUIM_PERFORMANCE_NOTIFICATION_COLOUR,   /* ID of colour struct */
    AUIM_GET_TERMINATE_IND,      /* @field function which will return the text of the field. */
    FUIM_ALIGNMENT_CENTRE,/* @field Alignment of text in field as specified in fuim_Alignment */
   }

/*=======================================================================*/
/*                                                                       */
/*=======================================================================*/
  ,{
    AUIM_FIELD_VALIDITY_FUNCTION,//Byte       ValidityFunction;
    AUIM_MENU_FIELD_COLOUR,  /* ID of colour struct */
    AUIM_NONE_PROMPT,    /* @field function which will return the text of the field. */
    FUIM_ALIGNMENT_LEFT,/* @field Alignment of text in field as specified in fuim_Alignment */
   }
};


/*=======================================================================*/
/*          C O N F I G U R A T I O N         F I E L D S                */
/*=======================================================================*/
const fuimFieldStruct auim_ConfigurationMenuFields[AUIM_MNU_CONFIG_MAX_FIELD] = {
/*=======================================================================*/
/*       S E R I A L          N U M B E R                                */
/*=======================================================================*/
  {
  FUIM_FIELDTYPE_NUMERIC_VALUE,
  AUIM_FIELD_VALIDITY_FUNCTION,     //Byte       ValidityFunction;
  AUIM_GET_SERIAL_NUMBER_PROMPT,    //osdStringID   Prompt;
  AUIM_MENU_SPLIT_FIELD_COLOUR,       //Byte PromptColour;
  0,      //Byte ChangeFunction;
  AUIM_GET_SERIAL_NUMBER,//Byte      GetFunction;
  0,//Byte      SetFunction;
  {.Numeric = 3},//TFieldSize    FieldSize;
  {0},//TFieldScaling   FieldScaling;
  0,//Byte      Prefix;
  0,//osdStringID   Suffix;
  FUIM_FIELD_TIMEOUT,//Byte      TimeOut;
  0,//Byte      TriggerDigits;
  {.NumericFont.size = FUIM_FONT_SIZE_SMALL, .NumericFont.color = FUIM_FONT_COLOR_1},//TFieldCharacters FieldCharacters;
  FUIM_ALIGNMENT_CENTRE,//Byte      Alignment;
  0,//Byte      LeadingZeros;
  &auim_ConfigurationDialogKeys[0]//fuimDialogNavigation  * ToDoWithKey;
  }

/*=======================================================================*/
/*       C U R R E N T           T I M E                                 */
/*=======================================================================*/
,{
  FUIM_FIELDTYPE_STRING_NUMERIC_VALUE,
  AUIM_FIELD_VALIDITY_FUNCTION,//Byte       ValidityFunction;
  AUIM_GET_CURRENT_TIME_PROMPT,//Byte Prompt;
  AUIM_MENU_SPLIT_FIELD_COLOUR,//Byte PromptColour;
  0,      //Byte ChangeFunction;
  AUIM_GET_CURRENT_TIME,//Byte      GetFunction;
  0,//Byte      SetFunction;
  {.Numeric = 5},//TFieldSize    FieldSize;
  {0},//TFieldScaling   FieldScaling;
  0,//Byte      Prefix;
  AUIM_GET_AM_PM_TIME_SUFFIX_ID,//osdStringID   Suffix;
  FUIM_FIELD_TIMEOUT,//Byte      TimeOut;
  0,//Byte      TriggerDigits;
  {.NumericFont.size = FUIM_FONT_SIZE_SMALL, .NumericFont.color = FUIM_FONT_COLOR_1},//TFieldCharacters FieldCharacters;
  FUIM_ALIGNMENT_CENTRE,//Byte      Alignment;
  0,//Byte      LeadingZeros;
  &auim_ConfigurationDialogKeys[0]//fuimDialogNavigation  * ToDoWithKey;
  }
/*=======================================================================*/
/*       C U R R E N T           D A T E                                 */
/*=======================================================================*/
,{
  FUIM_FIELDTYPE_STRING_NUMERIC_VALUE,
  AUIM_FIELD_VALIDITY_FUNCTION,//Byte       ValidityFunction;
  AUIM_GET_CURRENT_DATE_PROMPT,//Byte Prompt;
  AUIM_MENU_SPLIT_FIELD_COLOUR,//Byte PromptColour;
  0,      //Byte ChangeFunction;
  AUIM_GET_CURRENT_DATE,//Byte      GetFunction;
  0,//Byte      SetFunction;
  {.Numeric = 10},//TFieldSize    FieldSize;
  {0},//TFieldScaling   FieldScaling;
  0,//Byte      Prefix;
  0,//osdStringID   Suffix;
  FUIM_FIELD_TIMEOUT,//Byte      TimeOut;
  0,//Byte      TriggerDigits;
  {.NumericFont.size = FUIM_FONT_SIZE_SMALL, .NumericFont.color = FUIM_FONT_COLOR_1},//TFieldCharacters FieldCharacters;
  FUIM_ALIGNMENT_CENTRE,//Byte      Alignment;
  0,//Byte      LeadingZeros;
  &auim_ConfigurationDialogKeys[0]//fuimDialogNavigation  * ToDoWithKey;
  }
/*=======================================================================*/
/*       F I R M W A R E      V E R S I O N                              */
/*=======================================================================*/
 ,{
  FUIM_FIELDTYPE_STRING_NUMERIC_VALUE,
  AUIM_FIELD_VALIDITY_FUNCTION,//Byte       ValidityFunction;
  AUIM_GET_FIRMWARE_VERSION_PROMPT,//Byte Prompt;
  AUIM_MENU_SPLIT_FIELD_COLOUR,//Byte PromptColour;
  0,      //Byte ChangeFunction;
  AUIM_GET_FW_VERSION,//Byte      GetFunction;
  0,//Byte      SetFunction;
  {.Numeric = 8},//TFieldSize    FieldSize;
  {0},//TFieldScaling   FieldScaling;
  0,//Byte      Prefix;
  0,//osdStringID   Suffix;
  FUIM_FIELD_TIMEOUT,//Byte      TimeOut;
  0,//Byte      TriggerDigits;
  {.NumericFont.size = FUIM_FONT_SIZE_SMALL, .NumericFont.color = FUIM_FONT_COLOR_1},//TFieldCharacters FieldCharacters;
  FUIM_ALIGNMENT_CENTRE,//Byte      Alignment;
  0,//Byte      LeadingZeros;
  &auim_ConfigurationDialogKeys[0]//fuimDialogNavigation  * ToDoWithKey;
  }

/*=======================================================================*/
/*       R E F E R E N C E     N U M B E R                               */
/*=======================================================================*/
,{
  FUIM_FIELDTYPE_NUMERIC_VALUE,
  AUIM_FIELD_VALIDITY_FUNCTION,//Byte       ValidityFunction;
  AUIM_GET_REFERENCE_NUMBER_PROMPT,//Byte Prompt;
  AUIM_MENU_SPLIT_FIELD_COLOUR,//Byte PromptColour;
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
  FUIM_ALIGNMENT_CENTRE,//Byte      Alignment;
  0,//Byte      LeadingZeros;
  &auim_ConfigurationDialogKeys[0]//fuimDialogNavigation  * ToDoWithKey;
}
/*=======================================================================*/
/*       S T R A I N   G A U G E   S T A T U S                           */
/*=======================================================================*/
,{
  FUIM_FIELDTYPE_LIST,
  AUIM_FIELD_VALIDITY_FUNCTION,//Byte       ValidityFunction;
  AUIM_GET_STRAIN_GAUGE_STATUS_PROMPT,//osdStringID   Prompt;
  AUIM_MENU_SPLIT_FIELD_COLOUR,//Byte PromptColour;
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
  FUIM_ALIGNMENT_CENTRE,//Byte      Alignment;
  0,//Byte      LeadingZeros;
  &auim_ConfigurationDialogKeys[0]//fuimDialogNavigation  * ToDoWithKey;
}

/*=======================================================================*/
/*       C A L I B R A T I O N   C O N S T A N T                         */
/*=======================================================================*/
,{
  FUIM_FIELDTYPE_NUMERIC_VALUE,
  AUIM_FIELD_VALIDITY_FUNCTION,//Byte       ValidityFunction;
  AUIM_GET_CALIBRATION_CONSTANT_PROMPT,//osdStringID   Prompt;
  AUIM_MENU_SPLIT_FIELD_COLOUR,//Byte PromptColour;
  0,      //Byte ChangeFunction;
  AUIM_GET_CALIBRATION_CONST,//Byte      GetFunction;
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
  &auim_ConfigurationDialogKeys[0]//fuimDialogNavigation  * ToDoWithKey;

  }

};



/*=======================================================================*/
/*          I D L E         F I E L D S                                  */
/*=======================================================================*/
const fuimFieldStruct auim_IdleMenuFields[AUIM_MNU_IDLE_MAX_FIELD] = 
{
  /*=======================================================================*/
  /*       I D L E  W A I T I N G   S T A T U S      F I E L D             */
  /*=======================================================================*/
  {
    FUIM_FIELDTYPE_STRING,
    AUIM_FIELD_VALIDITY_FUNCTION,//Byte       ValidityFunction;
    AUIM_NONE_PROMPT,//Byte Prompt;
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
    FUIM_ALIGNMENT_CENTRE,//Byte      Alignment;
    0,//Byte      LeadingZeros;
    &auim_IdleDialogKeys[0]//fuimDialogNavigation  * ToDoWithKey;
  }
};


/*=======================================================================*/
/*         P A I R I N G          F I E L D S                            */
/*=======================================================================*/
const fuimFieldStruct auim_PairingMenuFields[AUIM_MNU_PAIRING_MAX_FIELD] = 
{
  /*=======================================================================*/
  /*       P A I R I N G   S T A T U S          F I E L D                  */
  /*=======================================================================*/
  {
    FUIM_FIELDTYPE_NUMERIC_VALUE,
    AUIM_PAIRING_MENU_VALIDITY_FUNCTION,//Byte       ValidityFunction;
    AUIM_GET_PAIRING_WAITING_PROMPT,//Byte Prompt;
    AUIM_MENU_FIELD_COLOUR,//Byte PromptColour;
    0,      //Byte ChangeFunction;
    AUIM_GET_PAIRING_CODE,//Byte      GetFunction;
    0,//Byte      SetFunction;
    {.Numeric = 6},//TFieldSize    FieldSize;
    {0},//{.Numeric = 2},//TFieldScaling   FieldScaling;
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

const fuimFieldStruct auim_PairingCancelMenuFields[AUIM_MNU_PAIRING_CANCEL_MAX_FIELD] =
{
  /*=======================================================================*/
  /*         P A I R I N G   C A N C E L     F I E L D                     */
  /*=======================================================================*/
  {
    FUIM_FIELDTYPE_STRING,
    AUIM_FIELD_VALIDITY_FUNCTION,//Byte       ValidityFunction;
    AUIM_NONE_PROMPT,//Byte Prompt;
    AUIM_MENU_DOUBLE_FIELD_COLOUR,//Byte PromptColour;
    0,      //Byte ChangeFunction;
    AUIM_GET_PAIRING_RESULT_CANCEL,//Byte      GetFunction;
    0,//Byte      SetFunction;
    {0},//TFieldSize    FieldSize;
    {0},//{.Numeric = 2},//TFieldScaling   FieldScaling;
    0,//Byte      Prefix;
    0,//osdStringID   Suffix;
    FUIM_FIELD_NO_TIMEOUT,//Byte      TimeOut;
    0,//Byte      TriggerDigits;
    {0},//TFieldCharacters FieldCharacters;
    FUIM_ALIGNMENT_CENTRE,//Byte      Alignment;
    0,//Byte      LeadingZeros;
    &auim_PairingCloseDialogKeys[0]//fuimDialogNavigation  * ToDoWithKey;
  }
};

const fuimFieldStruct auim_PairingCloseMenuFields[AUIM_MNU_PAIRING_CLOSE_MAX_FIELD] =
{
  /*=======================================================================*/
  /*          P A I R I N G   C L O S E     F I E L D                      */
  /*=======================================================================*/
  {
    FUIM_FIELDTYPE_STRING,
    AUIM_FIELD_VALIDITY_FUNCTION,//Byte       ValidityFunction;
    AUIM_NONE_PROMPT,//Byte Prompt;
    AUIM_MENU_DOUBLE_FIELD_COLOUR,//Byte PromptColour;
    0,      //Byte ChangeFunction;
    AUIM_GET_PAIRING_RESULT_STATUS,//Byte      GetFunction;
    0,//Byte      SetFunction;
    {0},//TFieldSize    FieldSize;
    {0},//{.Numeric = 2},//TFieldScaling   FieldScaling;
    0,//Byte      Prefix;
    0,//osdStringID   Suffix;
    FUIM_FIELD_NO_TIMEOUT,//Byte      TimeOut;
    0,//Byte      TriggerDigits;
    {0},//TFieldCharacters FieldCharacters;
    FUIM_ALIGNMENT_CENTRE,//Byte      Alignment;
    0,//Byte      LeadingZeros;
    &auim_PairingCloseDialogKeys[0]//fuimDialogNavigation  * ToDoWithKey;
  }
};


/*=======================================================================*/
/*     R E F E R E N C E      F I E L D S                                */
/*=======================================================================*/
const fuimFieldStruct auim_ReferenceMenuFields[AUIM_MNU_REFERENCE_MAX_FIELD] =
{
  /*=======================================================================*/
  /*       R E F E R E N C E      S T A R T      F I E L D                 */
  /*=======================================================================*/
  {
    FUIM_FIELDTYPE_NUMERIC,
    AUIM_FIELD_VALIDITY_FUNCTION,//Byte       ValidityFunction;
    AUIM_NONE_PROMPT,//Byte Prompt;
    AUIM_MENU_DOUBLE_FIELD_COLOUR,//Byte PromptColour;
    0,      //Byte ChangeFunction;
    AUIM_GET_REFERENCE_VALUE,//Byte      GetFunction;
    0,//Byte      SetFunction;
    {.Numeric = 3},//TFieldSize    FieldSize;
    {0},//TFieldScaling   FieldScaling;
    0,//Byte      Prefix;
    0,//osdStringID   Suffix;
    FUIM_FIELD_TIMEOUT,//Byte      TimeOut;
    0,//Byte      TriggerDigits;
    {.NumericFont.size = FUIM_FONT_SIZE_LARGE, .NumericFont.color = FUIM_FONT_COLOR_3},//TFieldCharacters FieldCharacters;
    FUIM_ALIGNMENT_CENTRE,//Byte      Alignment;
    0,//Byte      LeadingZeros;
    &auim_ReferenceDialogKeys[0]//fuimDialogNavigation  * ToDoWithKey;
  }
};

const fuimFieldStruct auim_ReferenceContinueMenuFields[AUIM_MNU_REFERENCE_CONTINUE_MAX_FIELD] =
{
  /*=======================================================================*/
  /*       R E F E R E N C E       C O N T I N U E    F I E L D            */
  /*=======================================================================*/
  {
    FUIM_FIELDTYPE_NUMERIC,
    AUIM_FIELD_VALIDITY_FUNCTION,//Byte       ValidityFunction;
    AUIM_NONE_PROMPT,//Byte Prompt;
    AUIM_MENU_DOUBLE_FIELD_COLOUR,//Byte PromptColour;
    0,      //Byte ChangeFunction;
    AUIM_GET_REFERENCE_VALUE,//Byte      GetFunction;
    0,//Byte      SetFunction;
    {.Numeric = 3},//TFieldSize    FieldSize;
    {0},//TFieldScaling   FieldScaling;
    0,//Byte      Prefix;
    0,//osdStringID   Suffix;
    FUIM_FIELD_TIMEOUT,//Byte      TimeOut;
    0,//Byte      TriggerDigits;
    {.NumericFont.size = FUIM_FONT_SIZE_LARGE, .NumericFont.color = FUIM_FONT_COLOR_3},//TFieldCharacters FieldCharacters;
    FUIM_ALIGNMENT_CENTRE,//Byte      Alignment;
    0,//Byte      LeadingZeros;
    &auim_ReferenceContinueDialogKeys[0]//fuimDialogNavigation  * ToDoWithKey;
  }
};

const fuimFieldStruct auim_ReferenceCanceledPerformanceMenuFields[AUIM_MNU_REFERENCE_CANCELED_PERFORMANCE_MAX_FIELD] =
{
  /*==================================================================================*/
  /*       R E F E R E N C E  C A N C E L E D--P E R F O R M A N C E   F I E L D      */
  /*==================================================================================*/
  {
    FUIM_FIELDTYPE_STRING,
    AUIM_FIELD_VALIDITY_FUNCTION,//Byte       ValidityFunction;
    AUIM_GET_CANCELED_PROMPT,//Byte Prompt;
    AUIM_MENU_DOUBLE_FIELD_COLOUR,//Byte PromptColour;
    0,      //Byte ChangeFunction;
    AUIM_NO_GET_FUNCTION,//Byte      GetFunction;
    0,//Byte      SetFunction;
    {0},//TFieldSize    FieldSize;
    {0},//TFieldScaling   FieldScaling;
    0,//Byte      Prefix;
    0,//osdStringID   Suffix;
    FUIM_FIELD_NO_TIMEOUT,//Byte      TimeOut;
    0,//Byte      TriggerDigits;
    {0},//TFieldCharacters FieldCharacters;
    FUIM_ALIGNMENT_CENTRE,//Byte      Alignment;
    0,//Byte      LeadingZeros;
    &auim_ReferenceCanceledPerformanceDialogKeys[0]//fuimDialogNavigation  * ToDoWithKey;
  }
};

const fuimFieldStruct auim_ReferenceCanceledPatientMenuFields[AUIM_MNU_REFERENCE_CANCELED_PATIENT_MAX_FIELD] =
{
  /*=======================================================================*/
  /*       R E F E R E N C E   C A N C E L E D--P A T I E N T    F I E L D */
  /*=======================================================================*/
  {
    FUIM_FIELDTYPE_STRING,
    AUIM_FIELD_VALIDITY_FUNCTION,//Byte       ValidityFunction;
    AUIM_GET_CANCELED_PROMPT,//Byte Prompt;
    AUIM_MENU_DOUBLE_FIELD_COLOUR,//Byte PromptColour;
    0,      //Byte ChangeFunction;
    AUIM_NO_GET_FUNCTION,//Byte      GetFunction;
    0,//Byte      SetFunction;
    {0},//TFieldSize    FieldSize;
    {0},//TFieldScaling   FieldScaling;
    0,//Byte      Prefix;
    0,//osdStringID   Suffix;
    FUIM_FIELD_NO_TIMEOUT,//Byte      TimeOut;
    0,//Byte      TriggerDigits;
    {0},//TFieldCharacters FieldCharacters;
    FUIM_ALIGNMENT_CENTRE,//Byte      Alignment;
    0,//Byte      LeadingZeros;
    &auim_ReferenceCanceledPatientDialogKeys[0]//fuimDialogNavigation  * ToDoWithKey;
  }
};

const fuimFieldStruct auim_ReferenceCompleteMenuFields[AUIM_MNU_REFERENCE_COMPLETE_MAX_FIELD] =
{
  /*=======================================================================*/
  /*       R E F E R E N C E      C O M P L E T E     F I E L D            */
  /*=======================================================================*/
  {
    FUIM_FIELDTYPE_STRING,
    AUIM_FIELD_VALIDITY_FUNCTION,//Byte       ValidityFunction;
    AUIM_NONE_PROMPT,//Byte Prompt;
    AUIM_MENU_DOUBLE_FIELD_COLOUR,//Byte PromptColour;
    0,      //Byte ChangeFunction;
    AUIM_GET_REFERENCE_COMPLETE_RESULT,//Byte      GetFunction;
    0,//Byte      SetFunction;
    {0},//TFieldSize    FieldSize;
    {0},//TFieldScaling   FieldScaling;
    0,//Byte      Prefix;
    0,//osdStringID   Suffix;
    FUIM_FIELD_NO_TIMEOUT,//Byte      TimeOut;
    0,//Byte      TriggerDigits;
    {0},//TFieldCharacters FieldCharacters;
    FUIM_ALIGNMENT_CENTRE,//Byte      Alignment;
    0,//Byte      LeadingZeros;
    &auim_ReferenceCompleteDialogKeys[0]//fuimDialogNavigation  * ToDoWithKey;
  }
};

const fuimFieldStruct auim_ReferenceTerminateMenuFields[AUIM_MNU_REFERENCE_TERMINATE_MAX_FIELD] =
{
  /*=======================================================================*/
  /*       R E F E R E N C E        T E R M I N A T E        F I E L D     */
  /*=======================================================================*/
  {
    FUIM_FIELDTYPE_NUMERIC,
    AUIM_FIELD_VALIDITY_FUNCTION,//Byte       ValidityFunction;
    AUIM_NONE_PROMPT,//Byte Prompt;
    AUIM_MENU_DOUBLE_FIELD_COLOUR,//Byte PromptColour;
    0,      //Byte ChangeFunction;
    AUIM_GET_REFERENCE_VALUE,//Byte      GetFunction;
    0,//Byte      SetFunction;
    {.Numeric = 3},//TFieldSize    FieldSize;
    {0},//TFieldScaling   FieldScaling;
    0,//Byte      Prefix;
    0,//osdStringID   Suffix;
    FUIM_FIELD_TIMEOUT,//Byte      TimeOut;
    0,//Byte      TriggerDigits;
    {.NumericFont.size = FUIM_FONT_SIZE_LARGE, .NumericFont.color = FUIM_FONT_COLOR_3},//TFieldCharacters FieldCharacters;
    FUIM_ALIGNMENT_CENTRE,//Byte      Alignment;
    0,//Byte      LeadingZeros;
    &auim_ReferenceTerminateDialogKeys[0]//fuimDialogNavigation  * ToDoWithKey;
  }
};

const fuimFieldStruct auim_ReferenceUnstablePerformanceMenuFields[AUIM_MNU_REFERENCE_UNSTABLE_PERFORMANCE_MAX_FIELD] =
{
  /*==============================================================================================*/
  /*     R E F E R E N C E    U N S T A B L E--P E R F O R M A N C E   R E P E A T    F I E L D   */
  /*==============================================================================================*/
  {
    FUIM_FIELDTYPE_NUMERIC,
    AUIM_FIELD_VALIDITY_FUNCTION,//Byte       ValidityFunction;
    AUIM_NONE_PROMPT,//Byte Prompt;
    AUIM_MENU_DOUBLE_FIELD_COLOUR,//Byte PromptColour;
    0,      //Byte ChangeFunction;
    AUIM_GET_REFERENCE_VALUE,//Byte      GetFunction;
    0,//Byte      SetFunction;
    {.Numeric = 3},//TFieldSize    FieldSize;
    {0},//TFieldScaling   FieldScaling;
    0,//Byte      Prefix;
    0,//osdStringID   Suffix;
    FUIM_FIELD_TIMEOUT,//Byte      TimeOut;
    0,//Byte      TriggerDigits;
    {.NumericFont.size = FUIM_FONT_SIZE_LARGE, .NumericFont.color = FUIM_FONT_COLOR_3},//TFieldCharacters FieldCharacters;
    FUIM_ALIGNMENT_CENTRE,//Byte      Alignment;
    0,//Byte      LeadingZeros;
    &auim_ReferenceUnstablePerformanceDialogKeys[0]//fuimDialogNavigation  * ToDoWithKey;
  }
};

const fuimFieldStruct auim_ReferenceUnstablePatientMenuFields[AUIM_MNU_REFERENCE_UNSTABLE_PATIENT_MAX_FIELD] =
{
  /*==============================================================================================*/
  /*     R E F E R E N C E    U N S T A B L E--P A T I E N T   R E P E A T    F I E L D           */
  /*==============================================================================================*/
  {
    FUIM_FIELDTYPE_NUMERIC,
    AUIM_FIELD_VALIDITY_FUNCTION,//Byte       ValidityFunction;
    AUIM_NONE_PROMPT,//Byte Prompt;
    AUIM_MENU_DOUBLE_FIELD_COLOUR,//Byte PromptColour;
    0,      //Byte ChangeFunction;
    AUIM_GET_REFERENCE_VALUE,//Byte      GetFunction;
    0,//Byte      SetFunction;
    {.Numeric = 3},//TFieldSize    FieldSize;
    {0},//TFieldScaling   FieldScaling;
    0,//Byte      Prefix;
    0,//osdStringID   Suffix;
    FUIM_FIELD_TIMEOUT,//Byte      TimeOut;
    0,//Byte      TriggerDigits;
    {.NumericFont.size = FUIM_FONT_SIZE_LARGE, .NumericFont.color = FUIM_FONT_COLOR_3},//TFieldCharacters FieldCharacters;
    FUIM_ALIGNMENT_CENTRE,//Byte      Alignment;
    0,//Byte      LeadingZeros;
    &auim_ReferenceUnstablePatientDialogKeys[0]//fuimDialogNavigation  * ToDoWithKey;
  }
};


/*=======================================================================*/
/*     P E R F O R M A N C E     F I E L D S                             */
/*=======================================================================*/

const fuimFieldStruct auim_PerformanceMenuFields[AUIM_MNU_PERFORMANCE_MAX_FIELD] =
{
  /*=======================================================================*/
  /*       P E R F O R M A N C E     S T A R T     F I E L D               */
  /*=======================================================================*/
  {
    FUIM_FIELDTYPE_NUMERIC,
    AUIM_FIELD_VALIDITY_FUNCTION,//Byte       ValidityFunction;
    AUIM_NONE_PROMPT,//Byte Prompt;
    AUIM_MENU_DOUBLE_FIELD_COLOUR,//Byte PromptColour;
    0,      //Byte ChangeFunction;
    AUIM_GET_PERFORMANCE_VALUE,//Byte      GetFunction;
    0,//Byte      SetFunction;
    {.Numeric = 3},//TFieldSize    FieldSize;
    {0},//TFieldScaling   FieldScaling;
    0,//Byte      Prefix;
    0,//osdStringID   Suffix;
    FUIM_FIELD_TIMEOUT,//Byte      TimeOut;
    0,//Byte      TriggerDigits;
    {.NumericFont.size = FUIM_FONT_SIZE_LARGE, .NumericFont.color = FUIM_FONT_COLOR_2},//TFieldCharacters FieldCharacters;
    FUIM_ALIGNMENT_CENTRE,//Byte      Alignment;
    0,//Byte      LeadingZeros;
    &auim_PerformanceDialogKeys[0]//fuimDialogNavigation  * ToDoWithKey;
  }
};

const fuimFieldStruct auim_PerformanceContinueMenuFields[AUIM_MNU_PERFORMANCE_CONTINUE_MAX_FIELD] =
{
  /*=======================================================================*/
  /*       P E R F O R M A N C E       C O N T I N U E      F I E L D      */
  /*=======================================================================*/
  {
    FUIM_FIELDTYPE_NUMERIC,
    AUIM_FIELD_VALIDITY_FUNCTION,//Byte       ValidityFunction;
    AUIM_NONE_PROMPT,//Byte Prompt;
    AUIM_MENU_DOUBLE_FIELD_COLOUR,//Byte PromptColour;
    0,      //Byte ChangeFunction;
    AUIM_GET_PERFORMANCE_VALUE,//Byte      GetFunction;
    0,//Byte      SetFunction;
    {.Numeric = 3},//TFieldSize    FieldSize;
    {0},//TFieldScaling   FieldScaling;
    0,//Byte      Prefix;
    0,//osdStringID   Suffix;
    FUIM_FIELD_TIMEOUT,//Byte      TimeOut;
    0,//Byte      TriggerDigits;
    {.NumericFont.size = FUIM_FONT_SIZE_LARGE, .NumericFont.color = FUIM_FONT_COLOR_2},//TFieldCharacters FieldCharacters;
    FUIM_ALIGNMENT_CENTRE,//Byte      Alignment;
    0,//Byte      LeadingZeros;
    &auim_PerformanceContinueDialogKeys[0]//fuimDialogNavigation  * ToDoWithKey;
  }
};

const fuimFieldStruct auim_PerformanceCanceledMenuFields[AUIM_MNU_PERFORMANCE_CANCELED_MAX_FIELD] =
{
  /*=======================================================================*/
  /*       P E R F O R M A N C E      C A N C E L E D     F I E L D        */
  /*=======================================================================*/
  {
    FUIM_FIELDTYPE_STRING,
    AUIM_FIELD_VALIDITY_FUNCTION,//Byte       ValidityFunction;
    AUIM_GET_CANCELED_PROMPT,//Byte Prompt;
    AUIM_MENU_DOUBLE_FIELD_COLOUR,//Byte PromptColour;
    0,      //Byte ChangeFunction;
    AUIM_NO_GET_FUNCTION,//Byte      GetFunction;
    0,//Byte      SetFunction;
    {0},//TFieldSize    FieldSize;
    {0},//TFieldScaling   FieldScaling;
    0,//Byte      Prefix;
    0,//osdStringID   Suffix;
    FUIM_FIELD_NO_TIMEOUT,//Byte      TimeOut;
    0,//Byte      TriggerDigits;
    {0},//TFieldCharacters FieldCharacters;
    FUIM_ALIGNMENT_CENTRE,//Byte      Alignment;
    0,//Byte      LeadingZeros;
    &auim_PerformanceCanceledDialogKeys[0]//fuimDialogNavigation  * ToDoWithKey;
  }
};

const fuimFieldStruct auim_PerformanceCompleteMenuFields[AUIM_MNU_PERFORMANCE_COMPLETE_MAX_FIELD] = 
{
  /*=======================================================================*/
  /*       P E R F O R M A N C E      C O M P L E T E    F I E L D         */
  /*=======================================================================*/
  {
    FUIM_FIELDTYPE_STRING,
    AUIM_FIELD_VALIDITY_FUNCTION,//Byte       ValidityFunction;
    AUIM_NONE_PROMPT,//Byte Prompt;
    AUIM_MENU_DOUBLE_FIELD_COLOUR,//Byte PromptColour;
    0,      //Byte ChangeFunction;
    AUIM_GET_PERFORMANCE_COMPLETE_RESULT,//Byte      GetFunction;
    0,//Byte      SetFunction;
    {0},//TFieldSize    FieldSize;
    {0},//TFieldScaling   FieldScaling;
    0,//Byte      Prefix;
    0,//osdStringID   Suffix;
    FUIM_FIELD_NO_TIMEOUT,//Byte      TimeOut;
    0,//Byte      TriggerDigits;
    {0},//TFieldCharacters FieldCharacters;
    FUIM_ALIGNMENT_CENTRE,//Byte      Alignment;
    0,//Byte      LeadingZeros;
    &auim_PerformanceCompleteDialogKeys[0]//fuimDialogNavigation  * ToDoWithKey;
  }
};

const fuimFieldStruct auim_PerformanceTerminateMenuFields[AUIM_MNU_PERFORMANCE_TERMINATE_MAX_FIELD] =
{
  /*=======================================================================*/
  /*       P E R F O R M A N C E       T E R M I N A T E    F I E L D      */
  /*=======================================================================*/
  {
    FUIM_FIELDTYPE_NUMERIC,
    AUIM_FIELD_VALIDITY_FUNCTION,//Byte       ValidityFunction;
    AUIM_NONE_PROMPT,//Byte Prompt;
    AUIM_MENU_DOUBLE_FIELD_COLOUR,//Byte PromptColour;
    0,      //Byte ChangeFunction;
    AUIM_GET_PERFORMANCE_VALUE,//Byte      GetFunction;
    0,//Byte      SetFunction;
    {.Numeric = 3},//TFieldSize    FieldSize;
    {0},//TFieldScaling   FieldScaling;
    0,//Byte      Prefix;
    0,//osdStringID   Suffix;
    FUIM_FIELD_NO_TIMEOUT,//Byte      TimeOut;
    0,//Byte      TriggerDigits;
    {.NumericFont.size = FUIM_FONT_SIZE_LARGE, .NumericFont.color = FUIM_FONT_COLOR_2},//TFieldCharacters FieldCharacters;
    FUIM_ALIGNMENT_CENTRE,//Byte      Alignment;
    0,//Byte      LeadingZeros;
    &auim_PerformanceTerminateDialogKeys[0]//fuimDialogNavigation  * ToDoWithKey;
  }
};

const fuimFieldStruct auim_PerformanceRequiredMenuFields[AUIM_MNU_PERFORMANCE_REQUIRED_MAX_FIELD] = 
{
  /*=======================================================================*/
  /*       P E R F O R M A N C E      R E Q U I R E D     F I E L D        */
  /*=======================================================================*/
  {
    FUIM_FIELDTYPE_STRING,
    AUIM_FIELD_VALIDITY_FUNCTION,//Byte       ValidityFunction;
    AUIM_GET_PERF_CHECK_REQUIRED_PROMPT,//Byte Prompt;
    AUIM_MENU_DOUBLE_FIELD_COLOUR,//Byte PromptColour;
    0,      //Byte ChangeFunction;
    AUIM_NO_GET_FUNCTION,//Byte      GetFunction;
    0,//Byte      SetFunction;
    {0},//TFieldSize    FieldSize;
    {0},//TFieldScaling   FieldScaling;
    0,//Byte      Prefix;
    0,//osdStringID   Suffix;
    FUIM_FIELD_TIMEOUT,//Byte      TimeOut;
    0,//Byte      TriggerDigits;
    {0},//TFieldCharacters FieldCharacters;
    FUIM_ALIGNMENT_CENTRE,//Byte      Alignment;
    0,//Byte      LeadingZeros;
    &auim_PerformanceDialogKeys[0]//fuimDialogNavigation  * ToDoWithKey;
  }
};

const fuimFieldStruct auim_PerformanceResultMenuFields[AUIM_MNU_PERFORMANCE_RESULT_MAX_FIELD] =
{
  /*=======================================================================*/
  /*       P E R F O R M A N C E     R E S U L T     F I E L D             */
  /*=======================================================================*/
  {
    FUIM_FIELDTYPE_STRING,
    AUIM_FIELD_VALIDITY_FUNCTION,//Byte       ValidityFunction;
    AUIM_NONE_PROMPT,//Byte Prompt;
    AUIM_MENU_DOUBLE_FIELD_COLOUR,//Byte PromptColour;
    0,      //Byte ChangeFunction;
    AUIM_GET_PERFORMANCE_RESULT,//Byte      GetFunction;
    0,//Byte      SetFunction;
    {0},//TFieldSize    FieldSize;
    {0},//TFieldScaling   FieldScaling;
    0,//Byte      Prefix;
    0,//osdStringID   Suffix;
    FUIM_FIELD_NO_TIMEOUT,//Byte      TimeOut;
    0,//Byte      TriggerDigits;
    {0},//TFieldCharacters FieldCharacters;
    FUIM_ALIGNMENT_CENTRE,//Byte      Alignment;
    0,//Byte      LeadingZeros;
    &auim_PerformanceResultDialogKeys[0]//fuimDialogNavigation  * ToDoWithKey;
  }
};


/*=======================================================================*/
/*        P A T I E N T     F I E L D S                                  */
/*=======================================================================*/

const fuimFieldStruct auim_PatientMenuFields[AUIM_MNU_PATIENT_MAX_FIELD] =
{
  /*=======================================================================*/
  /*       P A T I E N T     S T A R T     F I E L D                       */
  /*=======================================================================*/
  {
    FUIM_FIELDTYPE_NUMERIC,
    AUIM_FIELD_VALIDITY_FUNCTION,//Byte       ValidityFunction;
    AUIM_NONE_PROMPT,//Byte Prompt;
    AUIM_MENU_DOUBLE_FIELD_COLOUR,//Byte PromptColour;
    0,      //Byte ChangeFunction;
    AUIM_GET_PATIENT_VALUE,//Byte      GetFunction;
    0,//Byte      SetFunction;
    {.Numeric = 3},//TFieldSize    FieldSize;
    {0},//TFieldScaling   FieldScaling;
    0,//Byte      Prefix;
    0,//osdStringID   Suffix;
    FUIM_FIELD_TIMEOUT,//Byte      TimeOut;
    0,//Byte      TriggerDigits;
    {.NumericFont.size = FUIM_FONT_SIZE_LARGE, .NumericFont.color = FUIM_FONT_COLOR_1},//TFieldCharacters FieldCharacters;
    FUIM_ALIGNMENT_CENTRE,//Byte      Alignment;
    0,//Byte      LeadingZeros;
    &auim_PatientDialogKeys[0]//fuimDialogNavigation  * ToDoWithKey;
  }
};

const fuimFieldStruct auim_PatientContinueMenuFields[AUIM_MNU_PATIENT_CONTINUE_MAX_FIELD] =
{
  /*=======================================================================*/
  /*       P A T I E N T      C O N T I N U E      F I E L D               */
  /*=======================================================================*/
  {
    FUIM_FIELDTYPE_NUMERIC,
    AUIM_FIELD_VALIDITY_FUNCTION,//Byte       ValidityFunction;
    AUIM_NONE_PROMPT,//Byte Prompt;
    AUIM_MENU_DOUBLE_FIELD_COLOUR,//Byte PromptColour;
    0,      //Byte ChangeFunction;
    AUIM_GET_PATIENT_VALUE,//Byte      GetFunction;
    0,//Byte      SetFunction;
    {.Numeric = 3},//TFieldSize    FieldSize;
    {0},//TFieldScaling   FieldScaling;
    0,//Byte      Prefix;
    0,//osdStringID   Suffix;
    FUIM_FIELD_TIMEOUT,//Byte      TimeOut;
    0,//Byte      TriggerDigits;
    {.NumericFont.size = FUIM_FONT_SIZE_LARGE, .NumericFont.color = FUIM_FONT_COLOR_1},//TFieldCharacters FieldCharacters;
    FUIM_ALIGNMENT_CENTRE,//Byte      Alignment;
    0,//Byte      LeadingZeros;
    &auim_PatientContinueDialogKeys[0]//fuimDialogNavigation  * ToDoWithKey;
  }
};

const fuimFieldStruct auim_PatientCanceledMenuFields[AUIM_MNU_PATIENT_CANCELED_MAX_FIELD] =
{
  /*=======================================================================*/
  /*       P A T I E N T        C A N C E L E D     F I E L D              */
  /*=======================================================================*/
  {
    FUIM_FIELDTYPE_STRING,
    AUIM_FIELD_VALIDITY_FUNCTION,//Byte       ValidityFunction;
    AUIM_GET_CANCELED_PROMPT,//Byte Prompt;
    AUIM_MENU_DOUBLE_FIELD_COLOUR,//Byte PromptColour;
    0,      //Byte ChangeFunction;
    AUIM_NO_GET_FUNCTION,//Byte      GetFunction;
    0,//Byte      SetFunction;
    {0},//TFieldSize    FieldSize;
    {0},//TFieldScaling   FieldScaling;
    0,//Byte      Prefix;
    0,//osdStringID   Suffix;
    FUIM_FIELD_NO_TIMEOUT,//Byte      TimeOut;
    0,//Byte      TriggerDigits;
    {0},//TFieldCharacters FieldCharacters;
    FUIM_ALIGNMENT_CENTRE,//Byte      Alignment;
    0,//Byte      LeadingZeros;
    &auim_PatientCanceledDialogKeys[0]//fuimDialogNavigation  * ToDoWithKey;
  }
};

const fuimFieldStruct auim_PatientCompleteMenuFields[AUIM_MNU_PATIENT_COMPLETE_MAX_FIELD] = 
{
  /*=======================================================================*/
  /*       P A T I E N T       C O M P L E T E       F I E L D             */
  /*=======================================================================*/
    {
      FUIM_FIELDTYPE_STRING,
      AUIM_FIELD_VALIDITY_FUNCTION,//Byte       ValidityFunction;
      AUIM_NONE_PROMPT,//Byte Prompt;
      AUIM_MENU_DOUBLE_FIELD_COLOUR,//Byte PromptColour;
      0,      //Byte ChangeFunction;
      AUIM_GET_PATIENT_COMPLETE_RESULT,//Byte      GetFunction;
      0,//Byte      SetFunction;
      {0},//TFieldSize    FieldSize;
      {0},//TFieldScaling   FieldScaling;
      0,//Byte      Prefix;
      0,//osdStringID   Suffix;
      FUIM_FIELD_NO_TIMEOUT,//Byte      TimeOut;
      0,//Byte      TriggerDigits;
      {0},//TFieldCharacters FieldCharacters;
      FUIM_ALIGNMENT_CENTRE,//Byte      Alignment;
      0,//Byte      LeadingZeros;
      &auim_PatientCompleteDialogKeys[0]//fuimDialogNavigation  * ToDoWithKey;
    }

};

const fuimFieldStruct auim_PatientTerminateMenuFields[AUIM_MNU_PATIENT_TERMINATE_MAX_FIELD] =
{
  /*=======================================================================*/
  /*       P A T I E N T       T E R M I N A T E       F I E L D           */
  /*=======================================================================*/
  {
    FUIM_FIELDTYPE_NUMERIC,
    AUIM_FIELD_VALIDITY_FUNCTION,//Byte       ValidityFunction;
    AUIM_NONE_PROMPT,//Byte Prompt;
    AUIM_MENU_DOUBLE_FIELD_COLOUR,//Byte PromptColour;
    0,      //Byte ChangeFunction;
    AUIM_GET_PATIENT_VALUE,//Byte      GetFunction;
    0,//Byte      SetFunction;
    {.Numeric = 3},//TFieldSize    FieldSize;
    {0},//TFieldScaling   FieldScaling;
    0,//Byte      Prefix;
    0,//osdStringID   Suffix;
    FUIM_FIELD_NO_TIMEOUT,//Byte      TimeOut;
    0,//Byte      TriggerDigits;
    {.NumericFont.size = FUIM_FONT_SIZE_LARGE, .NumericFont.color = FUIM_FONT_COLOR_1},//TFieldCharacters FieldCharacters;
    FUIM_ALIGNMENT_CENTRE,//Byte      Alignment;
    0,//Byte      LeadingZeros;
    &auim_PatientTerminateDialogKeys[0]//fuimDialogNavigation  * ToDoWithKey;
  }
};

const fuimFieldStruct auim_PatientResultMenuFields[AUIM_MNU_PATIENT_RESULT_MAX_FIELD] =
{
  /*=======================================================================*/
  /*       P A T I E N T      R E S U L T       F I E L D                  */
  /*=======================================================================*/
    {
      FUIM_FIELDTYPE_NUMERIC_VALUE,
      AUIM_FIELD_VALIDITY_FUNCTION,//Byte       ValidityFunction;
      AUIM_GET_APPROXIMATION_IND,//Byte Prompt;
      AUIM_MENU_DOUBLE_FIELD_COLOUR,//Byte PromptColour;
      0,      //Byte ChangeFunction;
      AUIM_GET_PATIENT_RESULT,//Byte      GetFunction;
      0,//Byte      SetFunction;
      {.Numeric = 5},//TFieldSize    FieldSize;
      {.Numeric = 1},//TFieldScaling   FieldScaling;
      0,//Byte      Prefix;
      0,//osdStringID   Suffix;
      FUIM_FIELD_TIMEOUT,//Byte      TimeOut;
      0,//Byte      TriggerDigits;
      {.NumericFont.size = FUIM_FONT_SIZE_LARGE, .NumericFont.color = FUIM_FONT_COLOR_1},//TFieldCharacters FieldCharacters;
      FUIM_ALIGNMENT_CENTRE,//Byte      Alignment;
      0,//Byte      LeadingZeros;
      &auim_PatientResultDialogKeys[0]//fuimDialogNavigation  * ToDoWithKey;
    }
};


const fuimFieldStruct auim_PatientFailedMenuFields[AUIM_MNU_PATIENT_FAILED_MAX_FIELD] =
{
  /*=======================================================================*/
  /*       P A T I E N T        F A I L E D       F I E L D                */
  /*=======================================================================*/
  {
    FUIM_FIELDTYPE_STRING,
    AUIM_FIELD_VALIDITY_FUNCTION,//Byte       ValidityFunction;
    AUIM_NONE_PROMPT,//Byte Prompt;
    AUIM_MENU_DOUBLE_FIELD_COLOUR,//Byte PromptColour;
    0,      //Byte ChangeFunction;
    AUIM_GET_PATIENT_FILED_RESULT,//Byte      GetFunction;
    0,//Byte      SetFunction;
    {0},//TFieldSize    FieldSize;
    {0},//TFieldScaling   FieldScaling;
    0,//Byte      Prefix;
    0,//osdStringID   Suffix;
    FUIM_FIELD_NO_TIMEOUT,//Byte      TimeOut;
    0,//Byte      TriggerDigits;
    {0},//TFieldCharacters FieldCharacters;
    FUIM_ALIGNMENT_CENTRE,//Byte      Alignment;
    0,//Byte      LeadingZeros;
    &auim_PatientFailedDialogKeys[0]//fuimDialogNavigation  * ToDoWithKey;
  }
};

const fuimFieldStruct auim_PatientValidatingMenuFields[AUIM_MNU_PATIENT_VALIDATING_MAX_FIELD] =
{
  /*=======================================================================*/
  /*       P A T I E N T      V A L I D A T I N G        F I E L D         */
  /*=======================================================================*/
  {
    FUIM_FIELDTYPE_STRING,
    AUIM_FIELD_VALIDITY_FUNCTION,//Byte       ValidityFunction;
    AUIM_GET_PATIENT_VALIDATING_PROMPT,//Byte Prompt;
    AUIM_MENU_FIELD_COLOUR,//Byte PromptColour;
    0,      //Byte ChangeFunction;
    AUIM_NO_GET_FUNCTION,//Byte      GetFunction;
    0,//Byte      SetFunction;
    {0},//TFieldSize    FieldSize;
    {0},//TFieldScaling   FieldScaling;
    0,//Byte      Prefix;
    0,//osdStringID   Suffix;
    FUIM_FIELD_NO_TIMEOUT,//Byte      TimeOut;
    0,//Byte      TriggerDigits;
    {0},//TFieldCharacters FieldCharacters;
    FUIM_ALIGNMENT_CENTRE,//Byte      Alignment;
    0,//Byte      LeadingZeros;
    &auim_PatientValidatingDialogKeys[0]//fuimDialogNavigation  * ToDoWithKey;
  }
};

const fuimFieldStruct auim_PatientValidatingResultMenuFields[AUIM_MNU_PATIENT_VALIDATING_RESULT_MAX_FIELD] =
{
  /*=======================================================================*/
  /*    P A T I E N T    V A L I D A T I N G    C L O S E    F I E L D     */
  /*=======================================================================*/
  {
    FUIM_FIELDTYPE_STRING,
    AUIM_FIELD_VALIDITY_FUNCTION,//Byte       ValidityFunction;
    AUIM_GET_PATIENT_VALIDATING_PROMPT,//Byte Prompt;
    AUIM_MENU_FIELD_COLOUR,//Byte PromptColour;
    0,      //Byte ChangeFunction;
    AUIM_NO_GET_FUNCTION,//Byte      GetFunction;
    0,//Byte      SetFunction;
    {0},//TFieldSize    FieldSize;
    {0},//TFieldScaling   FieldScaling;
    0,//Byte      Prefix;
    0,//osdStringID   Suffix;
    FUIM_FIELD_NO_TIMEOUT,//Byte      TimeOut;
    0,//Byte      TriggerDigits;
    {0},//TFieldCharacters FieldCharacters;
    FUIM_ALIGNMENT_CENTRE,//Byte      Alignment;
    0,//Byte      LeadingZeros;
    &auim_PatientValidatingCloseDialogKeys[0]//fuimDialogNavigation  * ToDoWithKey;
  }
};
/*=======================================================================*/
/*    C H A R G E   T H E   B A T T E R Y          F I E L D S           */
/*=======================================================================*/
const fuimFieldStruct auim_ChargeBatteryMenuFields[AUIM_MNU_CHARGE_BATTERY_MAX_FIELD] = 
{
  /*=======================================================================*/
  /*       C H A R G E   T H E   B A T T E R Y          F I E L D          */
  /*=======================================================================*/
  {
    FUIM_FIELDTYPE_STRING,
    AUIM_FIELD_VALIDITY_FUNCTION,//Byte       ValidityFunction;
    AUIM_GET_CHARGE_BATTERY_PROMPT,//Byte Prompt;
    AUIM_MENU_FIELD_COLOUR,//Byte PromptColour;
    0,      //Byte ChangeFunction;
    AUIM_NO_GET_FUNCTION,//Byte      GetFunction;
    0,//Byte      SetFunction;
    {0},//TFieldSize    FieldSize;
    {0},//TFieldScaling   FieldScaling;
    0,//Byte      Prefix;
    0,//osdStringID   Suffix;
    FUIM_FIELD_TIMEOUT,//Byte      TimeOut;
    0,//Byte      TriggerDigits;
    {0},//TFieldCharacters FieldCharacters;
    FUIM_ALIGNMENT_CENTRE,//Byte      Alignment;
    0,//Byte      LeadingZeros;
    &auim_ChargeBatteryDialogKeys[0]//fuimDialogNavigation  * ToDoWithKey;
  }
};




/*=================================================================*/
/*       A L L    P O S S I B L E    D I S P L A Y     MENUs       */
/*=================================================================*/
const fmnu_MenuStruct auim_Menu[AUIM_MNU_MAX_MENUS] =
{
/*=================================================================*/
/*         C O N F I G U R A T I O N      MENU                     */
/*=================================================================*/
  {
    FMNU_MAIN_MENU,      /* Menu type  {FMNU_MAIN_MENU FMNU_SUB_MAIN_MENU FMNU_MESSAGE_BOARD} */
    AUIM_MNU_X_POS,//Position MENU X
    AUIM_MNU_Y_POS + FUIM_MENU_FIELD_TOP_MARGIN,                 //Position MENU Y
    FUIM_MENU_WIDTH,                //Menu width
    IMG_ID_PROPERTY_1_VARIANT4_8,   //ID Title MENU
    AUIM_MENU_TITLE_COLOUR,//Title Attributes
    4+AUIM_MNU_SHIFT_X,   //Position X PromptPos
    240+AUIM_MNU_SHIFT_X,  //Position X ValuePos
    &auim_ConfigurationMenuFields[0],// Pointer to an array of field structures
    1,                              // Number of fields visible at once
    AUIM_MNU_CONFIG_MAX_FIELD,// Total number of fields
    FUIM_MENU_NO_TIMEOUT,// // How long the menu remains active;
                          // if 0-stays active indefinitely
    &auim_MenuButtonFields[AUIM_NEXT_BUTTON_FIELD],                 // fuimFieldStruct const *LeftButtonField;  /* Pointer to structure of the Button field */
    &auim_MenuButtonFields[AUIM_CANCEL_BUTTON_FIELD],                  // fuimFieldStruct const *RightButtonField; /* Pointer to structure of the Button field */
    0//&auim_FixedEmptyField[] /* @field pointer to structure of the fixed bottom field */
  }


/*=================================================================*/
/*         I D L E      MENU                                       */
/*=================================================================*/
 ,{
    FMNU_MAIN_MENU,      /* Menu type  {FMNU_MAIN_MENU FMNU_SUB_MAIN_MENU FMNU_MESSAGE_BOARD} */
    AUIM_MNU_X_POS,//Position MENU X
    AUIM_MNU_Y_POS + FUIM_MENU_FIELD_NORMAL_TOP_MARGIN,                 //Position MENU Y
    FUIM_MENU_WIDTH,                //Menu width
    FMNU_NONE_TITLE,   //ID Title MENU
    AUIM_MENU_TITLE_COLOUR,//Title Attributes
    0+AUIM_MNU_SHIFT_X,   //Position X PromptPos
    26+AUIM_MNU_SHIFT_X,  //Position X ValuePos
    &auim_IdleMenuFields[0],// Pointer to an array of field structures
    1,                              // Number of fields visible at once
    AUIM_MNU_IDLE_MAX_FIELD,// Total number of fields
    FUIM_MENU_NO_TIMEOUT, // How long the menu remains active;
                           // if 0 - stays active indefinitely
    &auim_MenuButtonFields[AUIM_PERF_CHK_BUTTON_FIELD],  // fuimFieldStruct const *LeftButtonField;  /* Pointer to structure of the Button field */
    &auim_MenuButtonFields[AUIM_PARAMS_BUTTON_FIELD],  // fuimFieldStruct const *RightButtonField; /* Pointer to structure of the Button field */
    0 //&auim_FixedEmptyField[]/* @field pointer to structure of the fixed bottom field */  
}


/*=================================================================*/
/*         P A I R I N G      M E N U S                            */
/*=================================================================*/
/*=================================================================*/
/*         P A I R I N G      MENU                                 */
/*=================================================================*/
,{
  FMNU_MAIN_MENU,      /* Menu type  {FMNU_MAIN_MENU FMNU_SUB_MAIN_MENU FMNU_MESSAGE_BOARD} */
  AUIM_MNU_X_POS,//Position MENU X
  AUIM_MNU_Y_POS + FUIM_MENU_FIELD_NORMAL_TOP_MARGIN,                 //Position MENU Y
  FUIM_MENU_WIDTH,                //Menu width
  IMG_ID_PROPERTY_1_VARIANT5_8,   //ID Title MENU
  AUIM_MENU_TITLE_COLOUR,//Title Attributes
  0+AUIM_MNU_SHIFT_X,   //Position X PromptPos
  0+AUIM_MNU_SHIFT_X,  //Position X ValuePos
  &auim_PairingMenuFields[0],// Pointer to an array of field structures
  1,                              // Number of fields visible at once
  AUIM_MNU_PAIRING_MAX_FIELD,// Total number of fields
  FUIM_MENU_NO_TIMEOUT, // How long the menu remains active;
                         // if 0 - stays active indefinitely
  0, //fuimFieldStruct const *LeftButtonField;  /* Pointer to structure of the Button field */
  &auim_MenuButtonFields[AUIM_CANCEL_BUTTON_FIELD],  // fuimFieldStruct const *RightButtonField; /* Pointer to structure of the Button field */
  0//&auim_FixedEmptyField[]/* @field pointer to structure of the fixed bottom field */ 
}
/*=================================================================*/
/*         P A I R I N G     C A N C E L              MENU         */
/*=================================================================*/
,{
  FMNU_MAIN_MENU,      /* Menu type  {FMNU_MAIN_MENU FMNU_SUB_MAIN_MENU FMNU_MESSAGE_BOARD} */
  AUIM_MNU_X_POS,//Position MENU X
  AUIM_MNU_Y_POS + FUIM_MENU_FIELD_LARGE_TOP_MARGIN,                 //Position MENU Y
  FUIM_MENU_WIDTH,                //Menu width
  IMG_ID_PROPERTY_1_VARIANT5_8,   //ID Title MENU
  AUIM_MENU_TITLE_COLOUR,//Title Attributes
  0+AUIM_MNU_SHIFT_X,   //Position X PromptPos
  0+AUIM_MNU_SHIFT_X,  //Position X ValuePos
  &auim_PairingCancelMenuFields[0],// Pointer to an array of field structures
  1,                              // Number of fields visible at once
  AUIM_MNU_PAIRING_CANCEL_MAX_FIELD,// Total number of fields
  FUIM_MENU_TIMEOUT, // How long the menu remains active;
                         // if 0 - stays active indefinitely
  0, //fuimFieldStruct const *LeftButtonField;  /* Pointer to structure of the Button field */
  &auim_MenuButtonFields[AUIM_CLOSE_BUTTON_FIELD],  // fuimFieldStruct const *RightButtonField; /* Pointer to structure of the Button field */
  0//&auim_FixedEmptyField[]/* @field pointer to structure of the fixed bottom field */ 
}
/*=================================================================*/
/*         P A I R I N G     C L O S E                    MENU     */
/*=================================================================*/
,{
  FMNU_MAIN_MENU,      /* Menu type  {FMNU_MAIN_MENU FMNU_SUB_MAIN_MENU FMNU_MESSAGE_BOARD} */
  AUIM_MNU_X_POS,//Position MENU X
  AUIM_MNU_Y_POS + FUIM_MENU_FIELD_LARGE_TOP_MARGIN,                 //Position MENU Y
  FUIM_MENU_WIDTH,                //Menu width
  IMG_ID_PROPERTY_1_VARIANT5_8,   //ID Title MENU
  AUIM_MENU_TITLE_COLOUR,//Title Attributes
  0+AUIM_MNU_SHIFT_X,   //Position X PromptPos
  0+AUIM_MNU_SHIFT_X,  //Position X ValuePos
  &auim_PairingCloseMenuFields[0],// Pointer to an array of field structures
  1,                              // Number of fields visible at once
  AUIM_MNU_PAIRING_CLOSE_MAX_FIELD,// Total number of fields
  FUIM_MENU_TIMEOUT, // How long the menu remains active;
                         // if 0 - stays active indefinitely
  0, //fuimFieldStruct const *LeftButtonField;  /* Pointer to structure of the Button field */
  &auim_MenuButtonFields[AUIM_CLOSE_BUTTON_FIELD],  // fuimFieldStruct const *RightButtonField; /* Pointer to structure of the Button field */
  0//&auim_FixedEmptyField[]/* @field pointer to structure of the fixed bottom field */ 
}


/*=================================================================*/
/*         P E R F O R M A N C E      M E N U S                    */
/*=================================================================*/
/*=================================================================*/
/*         P E R F O R M A N C E    S T A R T             MENU     */
/*=================================================================*/
 ,{
    FMNU_MAIN_MENU,      /* Menu type  {FMNU_MAIN_MENU FMNU_SUB_MAIN_MENU FMNU_MESSAGE_BOARD} */
    AUIM_MNU_X_POS,     //Position MENU X
    AUIM_MNU_Y_POS + FUIM_MENU_FIELD_LARGE_TOP_MARGIN,     //Position MENU Y
    FUIM_MENU_WIDTH,    //Menu width
    IMG_ID_PROPERTY_1_PERFORMANCE,   //ID Title MENU
    AUIM_MENU_TITLE_COLOUR,//Title Attributes
    0 + AUIM_MNU_SHIFT_X,   //Position X PromptPos
    0 + AUIM_MNU_SHIFT_X + FUIM_MENU_MARGIN(10),  //Position X ValuePos
    &auim_PerformanceMenuFields[0],// Pointer to an array of field structures
    1,                              // Number of fields visible at once
    AUIM_MNU_PERFORMANCE_MAX_FIELD,// Total number of fields
    FUIM_MENU_NO_TIMEOUT, // How long the menu remains active;
                           // if 0 - stays active indefinitely
    &auim_MenuButtonFields[AUIM_START_BUTTON_FIELD],  // fuimFieldStruct const *LeftButtonField;  /* Pointer to structure of the Button field */
    &auim_MenuButtonFields[AUIM_CANCEL_BUTTON_FIELD],  // fuimFieldStruct const *RightButtonField; /* Pointer to structure of the Button field */
    0//&auim_FixedEmptyField[] /* @field pointer to structure of the fixed bottom field */
  }
/*=================================================================*/
/*         P E R F O R M A N C E    C O N T I N U E       MENU     */
/*=================================================================*/
 ,{
    FMNU_MAIN_MENU,      /* Menu type  {FMNU_MAIN_MENU FMNU_SUB_MAIN_MENU FMNU_MESSAGE_BOARD} */
    AUIM_MNU_X_POS,     //Position MENU X
    AUIM_MNU_Y_POS + FUIM_MENU_FIELD_LARGE_TOP_MARGIN,     //Position MENU Y
    FUIM_MENU_WIDTH,    //Menu width
    IMG_ID_PROPERTY_1_PERFORMANCE,   //ID Title MENU
    AUIM_MENU_TITLE_COLOUR,//Title Attributes
    0 + AUIM_MNU_SHIFT_X + FUIM_MENU_MARGIN(24),   //Position X PromptPos
    0 + AUIM_MNU_SHIFT_X + FUIM_MENU_MARGIN(10),  //Position X ValuePos
    &auim_PerformanceContinueMenuFields[0],// Pointer to an array of field structures
    1,                              // Number of fields visible at once
    AUIM_MNU_PERFORMANCE_CONTINUE_MAX_FIELD,// Total number of fields
    FUIM_MENU_NO_TIMEOUT, // How long the menu remains active;
                           // if 0 - stays active indefinitely
    0,  // fuimFieldStruct const *LeftButtonField;  /* Pointer to structure of the Button field */
    &auim_MenuButtonFields[AUIM_CANCEL_BUTTON_FIELD],  // fuimFieldStruct const *RightButtonField; /* Pointer to structure of the Button field */
    0//&auim_FixedEmptyField[] /* @field pointer to structure of the fixed bottom field */
  }
/*=================================================================*/
/*         P E R F O R M A N C E    C A N C E L E D       MENU     */
/*=================================================================*/
 ,{
    FMNU_MAIN_MENU,      /* Menu type  {FMNU_MAIN_MENU FMNU_SUB_MAIN_MENU FMNU_MESSAGE_BOARD} */
    AUIM_MNU_X_POS,     //Position MENU X
    AUIM_MNU_Y_POS + FUIM_MENU_FIELD_LARGE_TOP_MARGIN,     //Position MENU Y
    FUIM_MENU_WIDTH,    //Menu width
    IMG_ID_PROPERTY_1_PERFORMANCE,   //ID Title MENU
    AUIM_MENU_TITLE_COLOUR,//Title Attributes
    0+AUIM_MNU_SHIFT_X,   //Position X PromptPos
    0+AUIM_MNU_SHIFT_X,  //Position X ValuePos
    &auim_PerformanceCanceledMenuFields[0],// Pointer to an array of field structures
    1,                              // Number of fields visible at once
    AUIM_MNU_PERFORMANCE_CANCELED_MAX_FIELD,// Total number of fields
    FUIM_MENU_NO_TIMEOUT, // How long the menu remains active;
                           // if 0 - stays active indefinitely
    0,  // fuimFieldStruct const *LeftButtonField;  /* Pointer to structure of the Button field */
    &auim_MenuButtonFields[AUIM_CLOSE_BUTTON_FIELD],  // fuimFieldStruct const *RightButtonField; /* Pointer to structure of the Button field */
    0//&auim_FixedEmptyField[] /* @field pointer to structure of the fixed bottom field */
  }
/*=================================================================*/
/*         P E R F O R M A N C E    C O M P L E T E       MENU     */
/*=================================================================*/
 ,{
    FMNU_MAIN_MENU,      /* Menu type  {FMNU_MAIN_MENU FMNU_SUB_MAIN_MENU FMNU_MESSAGE_BOARD} */
    AUIM_MNU_X_POS,     //Position MENU X
    AUIM_MNU_Y_POS + FUIM_MENU_FIELD_LARGE_TOP_MARGIN,     //Position MENU Y
    FUIM_MENU_WIDTH,    //Menu width
    IMG_ID_PROPERTY_1_PERFORMANCE,   //ID Title MENU
    AUIM_MENU_TITLE_COLOUR,//Title Attributes
    0+AUIM_MNU_SHIFT_X,   //Position X PromptPos
    0+AUIM_MNU_SHIFT_X,  //Position X ValuePos
    &auim_PerformanceCompleteMenuFields[0],// Pointer to an array of field structures
    1,                              // Number of fields visible at once
    AUIM_MNU_PERFORMANCE_COMPLETE_MAX_FIELD,// Total number of fields
    FUIM_MENU_NO_TIMEOUT, // How long the menu remains active;
                           // if 0 - stays active indefinitely
    0,  // fuimFieldStruct const *LeftButtonField;  /* Pointer to structure of the Button field */
    0,//&auim_MenuButtonFields[AUIM_CANCEL_BUTTON_FIELD],  // fuimFieldStruct const *RightButtonField; /* Pointer to structure of the Button field */
    0//&auim_FixedEmptyField[] /* @field pointer to structure of the fixed bottom field */
  }
/*=================================================================*/
/*         P E R F O R M A N C E    T E R M I N A T E D   MENU     */
/*=================================================================*/
 ,{
    FMNU_MAIN_MENU,      /* Menu type  {FMNU_MAIN_MENU FMNU_SUB_MAIN_MENU FMNU_MESSAGE_BOARD} */
    AUIM_MNU_X_POS,     //Position MENU X
    AUIM_MNU_Y_POS + FUIM_MENU_FIELD_LARGE_TOP_MARGIN,     //Position MENU Y
    FUIM_MENU_WIDTH,    //Menu width
    IMG_ID_PROPERTY_1_PERFORMANCE,   //ID Title MENU
    AUIM_MENU_TITLE_COLOUR,//Title Attributes
    0 + AUIM_MNU_SHIFT_X,   //Position X PromptPos
    0 + AUIM_MNU_SHIFT_X + FUIM_MENU_MARGIN(10),  //Position X ValuePos
    &auim_PerformanceTerminateMenuFields[0],// Pointer to an array of field structures
    1,                              // Number of fields visible at once
    AUIM_MNU_PERFORMANCE_TERMINATE_MAX_FIELD,// Total number of fields
    FUIM_MENU_NO_TIMEOUT, // How long the menu remains active;
                           // if 0 - stays active indefinitely
    &auim_MenuButtonFields[AUIM_NO_BUTTON_FIELD],  // fuimFieldStruct const *LeftButtonField;  /* Pointer to structure of the Button field */
    &auim_MenuButtonFields[AUIM_YES_BUTTON_FIELD],  // fuimFieldStruct const *RightButtonField; /* Pointer to structure of the Button field */
    0//&auim_FixedEmptyField[] /* @field pointer to structure of the fixed bottom field */
  }
/*=================================================================*/
/*  P E R F O R M A N C E   C H E C K   R E Q U I R E D   MENU     */
/*=================================================================*/
 ,{
    FMNU_MAIN_MENU,      /* Menu type  {FMNU_MAIN_MENU FMNU_SUB_MAIN_MENU FMNU_MESSAGE_BOARD} */
    AUIM_MNU_X_POS,     //Position MENU X
    AUIM_MNU_Y_POS + FUIM_MENU_FIELD_LARGE_TOP_MARGIN,     //Position MENU Y
    FUIM_MENU_WIDTH,    //Menu width
    FMNU_NONE_TITLE,   //ID Title MENU
    AUIM_MENU_TITLE_COLOUR,//Title Attributes
    0+AUIM_MNU_SHIFT_X,   //Position X PromptPos
    0+AUIM_MNU_SHIFT_X,  //Position X ValuePos
    &auim_PerformanceRequiredMenuFields[0],// Pointer to an array of field structures
    1,                              // Number of fields visible at once
    AUIM_MNU_PERFORMANCE_REQUIRED_MAX_FIELD,// Total number of fields
    FUIM_MENU_NO_TIMEOUT, // How long the menu remains active;
                           // if 0 - stays active indefinitely
    &auim_MenuButtonFields[AUIM_START_BUTTON_FIELD],  // fuimFieldStruct const *LeftButtonField;  /* Pointer to structure of the Button field */
    &auim_MenuButtonFields[AUIM_CANCEL_BUTTON_FIELD],  // fuimFieldStruct const *RightButtonField; /* Pointer to structure of the Button field */
    0//&auim_FixedEmptyField[] /* @field pointer to structure of the fixed bottom field */
  }
/*=================================================================*/
/*         P E R F O R M A N C E    R E S U L T       MENU         */
/*=================================================================*/
 ,{
    FMNU_MAIN_MENU,      /* Menu type  {FMNU_MAIN_MENU FMNU_SUB_MAIN_MENU FMNU_MESSAGE_BOARD} */
    AUIM_MNU_X_POS,     //Position MENU X
    AUIM_MNU_Y_POS + FUIM_MENU_FIELD_LARGE_TOP_MARGIN,     //Position MENU Y
    FUIM_MENU_WIDTH,    //Menu width
    IMG_ID_PROPERTY_1_PERFORMANCE,   //ID Title MENU
    AUIM_MENU_TITLE_COLOUR,//Title Attributes
    0+AUIM_MNU_SHIFT_X,   //Position X PromptPos
    0+AUIM_MNU_SHIFT_X,  //Position X ValuePos
    &auim_PerformanceResultMenuFields[0],// Pointer to an array of field structures
    1,                              // Number of fields visible at once
    AUIM_MNU_PERFORMANCE_RESULT_MAX_FIELD,// Total number of fields
    FUIM_MENU_NO_TIMEOUT, // How long the menu remains active;
                           // if 0 - stays active indefinitely
    0,  // fuimFieldStruct const *LeftButtonField;  /* Pointer to structure of the Button field */
    &auim_MenuButtonFields[AUIM_CLOSE_BUTTON_FIELD],  // fuimFieldStruct const *RightButtonField; /* Pointer to structure of the Button field */
    0//&auim_FixedEmptyField[] /* @field pointer to structure of the fixed bottom field */
  }


/*=================================================================*/
/*         R E F E R E N C E      M E N U S                        */
/*=================================================================*/
/*=================================================================*/
/*         R E F E R E N C E      S T A R T             MENU       */
/*=================================================================*/
 ,{
    FMNU_MAIN_MENU,      /* Menu type  {FMNU_MAIN_MENU FMNU_SUB_MAIN_MENU FMNU_MESSAGE_BOARD} */
    AUIM_MNU_X_POS,     //Position MENU X
    AUIM_MNU_Y_POS + FUIM_MENU_FIELD_LARGE_TOP_MARGIN,     //Position MENU Y
    FUIM_MENU_WIDTH,    //Menu width
    IMG_ID_PROPERTY_1_REFERENCE,   //ID Title MENU
    AUIM_MENU_TITLE_COLOUR,//Title Attributes
    0 + AUIM_MNU_SHIFT_X,   //Position X PromptPos
    0 + AUIM_MNU_SHIFT_X + FUIM_MENU_MARGIN(10),  //Position X ValuePos
    &auim_ReferenceMenuFields[0],// Pointer to an array of field structures
    1,                              // Number of fields visible at once
    AUIM_MNU_REFERENCE_MAX_FIELD,// Total number of fields
    FUIM_MENU_NO_TIMEOUT, // How long the menu remains active;
                           // if 0 - stays active indefinitely
    &auim_MenuButtonFields[AUIM_START_BUTTON_FIELD],  // fuimFieldStruct const *LeftButtonField;  /* Pointer to structure of the Button field */
    &auim_MenuButtonFields[AUIM_CANCEL_BUTTON_FIELD],  // fuimFieldStruct const *RightButtonField; /* Pointer to structure of the Button field */
    0//&auim_FixedEmptyField[] /* @field pointer to structure of the fixed bottom field */
  }
/*=================================================================*/
/*          R E F E R E N C E    C O N T I N U E       MENU        */
/*=================================================================*/
 ,{
    FMNU_MAIN_MENU,      /* Menu type  {FMNU_MAIN_MENU FMNU_SUB_MAIN_MENU FMNU_MESSAGE_BOARD} */
    AUIM_MNU_X_POS,     //Position MENU X
    AUIM_MNU_Y_POS + FUIM_MENU_FIELD_LARGE_TOP_MARGIN,     //Position MENU Y
    FUIM_MENU_WIDTH,    //Menu width
    IMG_ID_PROPERTY_1_REFERENCE,   //ID Title MENU
    AUIM_MENU_TITLE_COLOUR,//Title Attributes
    0 + AUIM_MNU_SHIFT_X + FUIM_MENU_MARGIN(24),   //Position X PromptPos
    0 + AUIM_MNU_SHIFT_X + FUIM_MENU_MARGIN(10),  //Position X ValuePos
    &auim_ReferenceContinueMenuFields[0],// Pointer to an array of field structures
    1,                              // Number of fields visible at once
    AUIM_MNU_REFERENCE_CONTINUE_MAX_FIELD,// Total number of fields
    FUIM_MENU_NO_TIMEOUT, // How long the menu remains active;
                           // if 0 - stays active indefinitely
    0,  // fuimFieldStruct const *LeftButtonField;  /* Pointer to structure of the Button field */
    &auim_MenuButtonFields[AUIM_CANCEL_BUTTON_FIELD],  // fuimFieldStruct const *RightButtonField; /* Pointer to structure of the Button field */
    0//&auim_FixedEmptyField[] /* @field pointer to structure of the fixed bottom field */
  }
/*================================================================================*/
/*         R E F E R E N C E   C A N C E L E D--P E R F O R M A N C E    MENU     */
/*================================================================================*/
 ,{
    FMNU_MAIN_MENU,      /* Menu type  {FMNU_MAIN_MENU FMNU_SUB_MAIN_MENU FMNU_MESSAGE_BOARD} */
    AUIM_MNU_X_POS,     //Position MENU X
    AUIM_MNU_Y_POS + FUIM_MENU_FIELD_LARGE_TOP_MARGIN,     //Position MENU Y
    FUIM_MENU_WIDTH,    //Menu width
    IMG_ID_PROPERTY_1_REFERENCE,   //ID Title MENU
    AUIM_MENU_TITLE_COLOUR,//Title Attributes
    0+AUIM_MNU_SHIFT_X,   //Position X PromptPos
    0+AUIM_MNU_SHIFT_X,  //Position X ValuePos
    &auim_ReferenceCanceledPerformanceMenuFields[0],// Pointer to an array of field structures
    1,                              // Number of fields visible at once
    AUIM_MNU_REFERENCE_CANCELED_PERFORMANCE_MAX_FIELD,// Total number of fields
    FUIM_MENU_NO_TIMEOUT, // How long the menu remains active;
                           // if 0 - stays active indefinitely
    0,  // fuimFieldStruct const *LeftButtonField;  /* Pointer to structure of the Button field */
    &auim_MenuButtonFields[AUIM_CLOSE_BUTTON_FIELD],  // fuimFieldStruct const *RightButtonField; /* Pointer to structure of the Button field */
    0//&auim_FixedEmptyField[] /* @field pointer to structure of the fixed bottom field */
  }
  /*========================================================================*/
  /*         R E F E R E N C E   C A N C E L E D--P A T I E N T    MENU     */
  /*========================================================================*/
   ,{
      FMNU_MAIN_MENU,      /* Menu type  {FMNU_MAIN_MENU FMNU_SUB_MAIN_MENU FMNU_MESSAGE_BOARD} */
      AUIM_MNU_X_POS,     //Position MENU X
      AUIM_MNU_Y_POS + FUIM_MENU_FIELD_LARGE_TOP_MARGIN,     //Position MENU Y
      FUIM_MENU_WIDTH,    //Menu width
      IMG_ID_PROPERTY_1_REFERENCE,   //ID Title MENU
      AUIM_MENU_TITLE_COLOUR,//Title Attributes
      0+AUIM_MNU_SHIFT_X,   //Position X PromptPos
      0+AUIM_MNU_SHIFT_X,  //Position X ValuePos
      &auim_ReferenceCanceledPatientMenuFields[0],// Pointer to an array of field structures
      1,                              // Number of fields visible at once
      AUIM_MNU_REFERENCE_CANCELED_PATIENT_MAX_FIELD,// Total number of fields
      FUIM_MENU_NO_TIMEOUT, // How long the menu remains active;
                             // if 0 - stays active indefinitely
      0,  // fuimFieldStruct const *LeftButtonField;  /* Pointer to structure of the Button field */
      &auim_MenuButtonFields[AUIM_CLOSE_BUTTON_FIELD],  // fuimFieldStruct const *RightButtonField; /* Pointer to structure of the Button field */
      0//&auim_FixedEmptyField[] /* @field pointer to structure of the fixed bottom field */
    }
/*=================================================================*/
/*        R E F E R E N C E      C O M P L E T E       MENU        */
/*=================================================================*/
 ,{
    FMNU_MAIN_MENU,      /* Menu type  {FMNU_MAIN_MENU FMNU_SUB_MAIN_MENU FMNU_MESSAGE_BOARD} */
    AUIM_MNU_X_POS,     //Position MENU X
    AUIM_MNU_Y_POS + FUIM_MENU_FIELD_LARGE_TOP_MARGIN,     //Position MENU Y
    FUIM_MENU_WIDTH,    //Menu width
    IMG_ID_PROPERTY_1_REFERENCE,   //ID Title MENU
    AUIM_MENU_TITLE_COLOUR,//Title Attributes
    0+AUIM_MNU_SHIFT_X,   //Position X PromptPos
    0+AUIM_MNU_SHIFT_X,  //Position X ValuePos
    &auim_ReferenceCompleteMenuFields[0],// Pointer to an array of field structures
    1,                              // Number of fields visible at once
    AUIM_MNU_REFERENCE_COMPLETE_MAX_FIELD,// Total number of fields
    FUIM_MENU_NO_TIMEOUT, // How long the menu remains active;
                           // if 0 - stays active indefinitely
    0,  // fuimFieldStruct const *LeftButtonField;  /* Pointer to structure of the Button field */
    0,  // fuimFieldStruct const *RightButtonField; /* Pointer to structure of the Button field */
    0//&auim_FixedEmptyField[] /* @field pointer to structure of the fixed bottom field */
  }
/*=================================================================*/
/*         R E F E R E N C E    T E R M I N A T E D       MENU     */
/*=================================================================*/
 ,{
    FMNU_MAIN_MENU,      /* Menu type  {FMNU_MAIN_MENU FMNU_SUB_MAIN_MENU FMNU_MESSAGE_BOARD} */
    AUIM_MNU_X_POS,     //Position MENU X
    AUIM_MNU_Y_POS + FUIM_MENU_FIELD_LARGE_TOP_MARGIN,     //Position MENU Y
    FUIM_MENU_WIDTH,    //Menu width
    IMG_ID_PROPERTY_1_REFERENCE,   //ID Title MENU
    AUIM_MENU_TITLE_COLOUR,//Title Attributes
    0 + AUIM_MNU_SHIFT_X,   //Position X PromptPos
    0 + AUIM_MNU_SHIFT_X + FUIM_MENU_MARGIN(10),  //Position X ValuePos
    &auim_ReferenceTerminateMenuFields[0],// Pointer to an array of field structures
    1,                              // Number of fields visible at once
    AUIM_MNU_REFERENCE_TERMINATE_MAX_FIELD,// Total number of fields
    FUIM_MENU_NO_TIMEOUT, // How long the menu remains active;
                           // if 0 - stays active indefinitely
    &auim_MenuButtonFields[AUIM_NO_BUTTON_FIELD],  // fuimFieldStruct const *LeftButtonField;  /* Pointer to structure of the Button field */
    &auim_MenuButtonFields[AUIM_YES_BUTTON_FIELD],  // fuimFieldStruct const *RightButtonField; /* Pointer to structure of the Button field */
    0//&auim_FixedEmptyField[] /* @field pointer to structure of the fixed bottom field */
  }
/*========================================================================================*/
/*       R E F E R E N C E   U N S T A B L E--P E R F O R M A N C E    R E P E A T   MENU */
/*========================================================================================*/
 ,{
    FMNU_MAIN_MENU,      /* Menu type  {FMNU_MAIN_MENU FMNU_SUB_MAIN_MENU FMNU_MESSAGE_BOARD} */
    AUIM_MNU_X_POS,     //Position MENU X
    AUIM_MNU_Y_POS + FUIM_MENU_FIELD_LARGE_TOP_MARGIN,     //Position MENU Y
    FUIM_MENU_WIDTH,    //Menu width
    IMG_ID_PROPERTY_1_REFERENCE,   //ID Title MENU
    AUIM_MENU_TITLE_COLOUR,//Title Attributes
    0 + AUIM_MNU_SHIFT_X,   //Position X PromptPos
    0 + AUIM_MNU_SHIFT_X + FUIM_MENU_MARGIN(10),  //Position X ValuePos
    &auim_ReferenceUnstablePerformanceMenuFields[0],// Pointer to an array of field structures
    1,                              // Number of fields visible at once
    AUIM_MNU_REFERENCE_UNSTABLE_PERFORMANCE_MAX_FIELD ,// Total number of fields
    FUIM_MENU_NO_TIMEOUT, // How long the menu remains active;
                           // if 0 - stays active indefinitely
    &auim_MenuButtonFields[AUIM_REPEAT_REF_BUTTON_FIELD],  // fuimFieldStruct const *LeftButtonField;  /* Pointer to structure of the Button field */
    &auim_MenuButtonFields[AUIM_SKIP_BUTTON_FIELD],  // fuimFieldStruct const *RightButtonField; /* Pointer to structure of the Button field */
    0//&auim_FixedEmptyField[] /* @field pointer to structure of the fixed bottom field */
  }
  /*========================================================================================*/
  /*       R E F E R E N C E   U N S T A B L E--P A T I E N T    R E P E A T     MENU       */
  /*========================================================================================*/
   ,{
      FMNU_MAIN_MENU,      /* Menu type  {FMNU_MAIN_MENU FMNU_SUB_MAIN_MENU FMNU_MESSAGE_BOARD} */
      AUIM_MNU_X_POS,     //Position MENU X
      AUIM_MNU_Y_POS + FUIM_MENU_FIELD_LARGE_TOP_MARGIN,     //Position MENU Y
      FUIM_MENU_WIDTH,    //Menu width
      IMG_ID_PROPERTY_1_REFERENCE,   //ID Title MENU
      AUIM_MENU_TITLE_COLOUR,//Title Attributes
      0 + AUIM_MNU_SHIFT_X,   //Position X PromptPos
      0 + AUIM_MNU_SHIFT_X + FUIM_MENU_MARGIN(10),  //Position X ValuePos
      &auim_ReferenceUnstablePatientMenuFields[0],// Pointer to an array of field structures
      1,                              // Number of fields visible at once
      AUIM_MNU_REFERENCE_UNSTABLE_PATIENT_MAX_FIELD ,// Total number of fields
      FUIM_MENU_NO_TIMEOUT, // How long the menu remains active;
                             // if 0 - stays active indefinitely
      &auim_MenuButtonFields[AUIM_REPEAT_REF_BUTTON_FIELD],  // fuimFieldStruct const *LeftButtonField;  /* Pointer to structure of the Button field */
      &auim_MenuButtonFields[AUIM_SKIP_BUTTON_FIELD],  // fuimFieldStruct const *RightButtonField; /* Pointer to structure of the Button field */
      0//&auim_FixedEmptyField[] /* @field pointer to structure of the fixed bottom field */
    }


/*=================================================================*/
/*         P A T I E N T      M E N U S                            */
/*=================================================================*/
/*=================================================================*/
/*         P A T I E N T      S T A R T             MENU           */
/*=================================================================*/
 ,{
    FMNU_MAIN_MENU,      /* Menu type  {FMNU_MAIN_MENU FMNU_SUB_MAIN_MENU FMNU_MESSAGE_BOARD} */
    AUIM_MNU_X_POS,     //Position MENU X
    AUIM_MNU_Y_POS + FUIM_MENU_FIELD_LARGE_TOP_MARGIN,     //Position MENU Y
    FUIM_MENU_WIDTH,    //Menu width
    IMG_ID_PROPERTY_1_PATIENT,   //ID Title MENU
    AUIM_MENU_TITLE_COLOUR,//Title Attributes
    0 + AUIM_MNU_SHIFT_X,   //Position X PromptPos
    0 + AUIM_MNU_SHIFT_X + FUIM_MENU_MARGIN(10),  //Position X ValuePos
    &auim_PatientMenuFields[0],// Pointer to an array of field structures
    1,                              // Number of fields visible at once
    AUIM_MNU_PATIENT_MAX_FIELD,// Total number of fields
    FUIM_MENU_NO_TIMEOUT, // How long the menu remains active;
                           // if 0 - stays active indefinitely
    &auim_MenuButtonFields[AUIM_START_BUTTON_FIELD],  // fuimFieldStruct const *LeftButtonField;  /* Pointer to structure of the Button field */
    &auim_MenuButtonFields[AUIM_CANCEL_BUTTON_FIELD],  // fuimFieldStruct const *RightButtonField; /* Pointer to structure of the Button field */
    0//&auim_FixedEmptyField[] /* @field pointer to structure of the fixed bottom field */
  }
/*=================================================================*/
/*         P A T I E N T      C O N T I N U E       MENU           */
/*=================================================================*/
 ,{
    FMNU_MAIN_MENU,      /* Menu type  {FMNU_MAIN_MENU FMNU_SUB_MAIN_MENU FMNU_MESSAGE_BOARD} */
    AUIM_MNU_X_POS,     //Position MENU X
    AUIM_MNU_Y_POS + FUIM_MENU_FIELD_LARGE_TOP_MARGIN,     //Position MENU Y
    FUIM_MENU_WIDTH,    //Menu width
    IMG_ID_PROPERTY_1_PATIENT,   //ID Title MENU
    AUIM_MENU_TITLE_COLOUR,//Title Attributes
    0 + AUIM_MNU_SHIFT_X + FUIM_MENU_MARGIN(24),   //Position X PromptPos
    0 + AUIM_MNU_SHIFT_X + FUIM_MENU_MARGIN(10),  //Position X ValuePos
    &auim_PatientContinueMenuFields[0],// Pointer to an array of field structures
    1,                              // Number of fields visible at once
    AUIM_MNU_PATIENT_CONTINUE_MAX_FIELD,// Total number of fields
    FUIM_MENU_NO_TIMEOUT, // How long the menu remains active;
                           // if 0 - stays active indefinitely
    0,  // fuimFieldStruct const *LeftButtonField;  /* Pointer to structure of the Button field */
    &auim_MenuButtonFields[AUIM_CANCEL_BUTTON_FIELD],  // fuimFieldStruct const *RightButtonField; /* Pointer to structure of the Button field */
    0//&auim_FixedEmptyField[] /* @field pointer to structure of the fixed bottom field */
  }
/*=================================================================*/
/*         P A T I E N T      C A N C E L E D       MENU           */
/*=================================================================*/
 ,{
    FMNU_MAIN_MENU,      /* Menu type  {FMNU_MAIN_MENU FMNU_SUB_MAIN_MENU FMNU_MESSAGE_BOARD} */
    AUIM_MNU_X_POS,     //Position MENU X
    AUIM_MNU_Y_POS + FUIM_MENU_FIELD_LARGE_TOP_MARGIN,     //Position MENU Y
    FUIM_MENU_WIDTH,    //Menu width
    IMG_ID_PROPERTY_1_PATIENT,   //ID Title MENU
    AUIM_MENU_TITLE_COLOUR,//Title Attributes
    0+AUIM_MNU_SHIFT_X,   //Position X PromptPos
    0+AUIM_MNU_SHIFT_X,  //Position X ValuePos
    &auim_PatientCanceledMenuFields[0],// Pointer to an array of field structures
    1,                              // Number of fields visible at once
    AUIM_MNU_PATIENT_CANCELED_MAX_FIELD,// Total number of fields
    FUIM_MENU_NO_TIMEOUT, // How long the menu remains active;
                           // if 0 - stays active indefinitely
    0,  // fuimFieldStruct const *LeftButtonField;  /* Pointer to structure of the Button field */
    &auim_MenuButtonFields[AUIM_CLOSE_BUTTON_FIELD],  // fuimFieldStruct const *RightButtonField; /* Pointer to structure of the Button field */
    0//&auim_FixedEmptyField[] /* @field pointer to structure of the fixed bottom field */
  }
/*=================================================================*/
/*         P A T I E N T      C O M P L E T E       MENU           */
/*=================================================================*/
 ,{
    FMNU_MAIN_MENU,      /* Menu type  {FMNU_MAIN_MENU FMNU_SUB_MAIN_MENU FMNU_MESSAGE_BOARD} */
    AUIM_MNU_X_POS,     //Position MENU X
    AUIM_MNU_Y_POS + FUIM_MENU_FIELD_LARGE_TOP_MARGIN,     //Position MENU Y
    FUIM_MENU_WIDTH,    //Menu width
    IMG_ID_PROPERTY_1_PATIENT,   //ID Title MENU
    AUIM_MENU_TITLE_COLOUR,//Title Attributes
    0+AUIM_MNU_SHIFT_X,   //Position X PromptPos
    0+AUIM_MNU_SHIFT_X,  //Position X ValuePos
    &auim_PatientCompleteMenuFields[0],// Pointer to an array of field structures
    1,                              // Number of fields visible at once
    AUIM_MNU_PATIENT_COMPLETE_MAX_FIELD,// Total number of fields
    FUIM_MENU_NO_TIMEOUT, // How long the menu remains active;
                           // if 0 - stays active indefinitely
    0,  // fuimFieldStruct const *LeftButtonField;  /* Pointer to structure of the Button field */
    0,//&auim_MenuButtonFields[AUIM_CANCEL_BUTTON_FIELD],  // fuimFieldStruct const *RightButtonField; /* Pointer to structure of the Button field */
    0//&auim_FixedEmptyField[] /* @field pointer to structure of the fixed bottom field */
  }
/*=================================================================*/
/*         P A T I E N T      T E R M I N A T E D   MENU           */
/*=================================================================*/
 ,{
    FMNU_MAIN_MENU,      /* Menu type  {FMNU_MAIN_MENU FMNU_SUB_MAIN_MENU FMNU_MESSAGE_BOARD} */
    AUIM_MNU_X_POS,     //Position MENU X
    AUIM_MNU_Y_POS + FUIM_MENU_FIELD_LARGE_TOP_MARGIN,     //Position MENU Y
    FUIM_MENU_WIDTH,    //Menu width
    IMG_ID_PROPERTY_1_PATIENT,   //ID Title MENU
    AUIM_MENU_TITLE_COLOUR,//Title Attributes
    0 + AUIM_MNU_SHIFT_X,   //Position X PromptPos
    0 + AUIM_MNU_SHIFT_X + FUIM_MENU_MARGIN(10),  //Position X ValuePos
    &auim_PatientTerminateMenuFields[0],// Pointer to an array of field structures
    1,                              // Number of fields visible at once
    AUIM_MNU_PATIENT_TERMINATE_MAX_FIELD,// Total number of fields
    FUIM_MENU_NO_TIMEOUT, // How long the menu remains active;
                           // if 0 - stays active indefinitely
    &auim_MenuButtonFields[AUIM_NO_BUTTON_FIELD],  // fuimFieldStruct const *LeftButtonField;  /* Pointer to structure of the Button field */
    &auim_MenuButtonFields[AUIM_YES_BUTTON_FIELD],  // fuimFieldStruct const *RightButtonField; /* Pointer to structure of the Button field */
    0//&auim_FixedEmptyField[] /* @field pointer to structure of the fixed bottom field */
  }
/*==================================================================*/
/*          P A T I E N T      R E S U L T           MENU           */
/*==================================================================*/
,{
  FMNU_MAIN_MENU,      /* Menu type  {FMNU_MAIN_MENU FMNU_SUB_MAIN_MENU FMNU_MESSAGE_BOARD} */
  AUIM_MNU_X_POS,     //Position MENU X
  AUIM_MNU_Y_POS + FUIM_MENU_FIELD_LARGE_TOP_MARGIN,     //Position MENU Y
  FUIM_MENU_WIDTH,    //Menu width
  IMG_ID_PROPERTY_1_PATIENT,   //ID Title MENU
  AUIM_MENU_TITLE_COLOUR,//Title Attributes
  0+AUIM_MNU_SHIFT_X + FUIM_MENU_MARGIN(24),   //Position X PromptPos
  0+AUIM_MNU_SHIFT_X + FUIM_MENU_MARGIN(10),  //Position X ValuePos
  &auim_PatientResultMenuFields[0],// Pointer to an array of field structures
  1,                              // Number of fields visible at once
  AUIM_MNU_PATIENT_RESULT_MAX_FIELD,// Total number of fields
  FUIM_MENU_NO_TIMEOUT, // How long the menu remains active;
                         // if 0 - stays active indefinitely
  0,  // fuimFieldStruct const *LeftButtonField;  /* Pointer to structure of the Button field */
  &auim_MenuButtonFields[AUIM_CLOSE_BUTTON_FIELD],  // fuimFieldStruct const *RightButtonField; /* Pointer to structure of the Button field */
  0//&auim_FixedEmptyField[] /* @field pointer to structure of the fixed bottom field */
}
  /*==================================================================*/
  /*          P A T I E N T     F A I L E D          MENU             */
  /*==================================================================*/
  ,{
    FMNU_MAIN_MENU,      /* Menu type  {FMNU_MAIN_MENU FMNU_SUB_MAIN_MENU FMNU_MESSAGE_BOARD} */
    AUIM_MNU_X_POS,     //Position MENU X
    AUIM_MNU_Y_POS + FUIM_MENU_FIELD_LARGE_TOP_MARGIN,     //Position MENU Y
    FUIM_MENU_WIDTH,    //Menu width
    IMG_ID_PROPERTY_1_PATIENT,   //ID Title MENU
    AUIM_MENU_TITLE_COLOUR,//Title Attributes
    0+AUIM_MNU_SHIFT_X,   //Position X PromptPos
    0+AUIM_MNU_SHIFT_X,  //Position X ValuePos
    &auim_PatientFailedMenuFields[0],// Pointer to an array of field structures
    1,                              // Number of fields visible at once
    AUIM_MNU_PATIENT_FAILED_MAX_FIELD,// Total number of fields
    FUIM_MENU_NO_TIMEOUT, // How long the menu remains active;
                           // if 0 - stays active indefinitely
    0,  // fuimFieldStruct const *LeftButtonField;  /* Pointer to structure of the Button field */
    &auim_MenuButtonFields[AUIM_CLOSE_BUTTON_FIELD],  // fuimFieldStruct const *RightButtonField; /* Pointer to structure of the Button field */
    0//&auim_FixedEmptyField[] /* @field pointer to structure of the fixed bottom field */
  }
/*==================================================================*/
/*          P A T I E N T      V A L I D A T I N G        MENU      */
/*==================================================================*/
,{
  FMNU_MAIN_MENU,      /* Menu type  {FMNU_MAIN_MENU FMNU_SUB_MAIN_MENU FMNU_MESSAGE_BOARD} */
  AUIM_MNU_X_POS,     //Position MENU X
  AUIM_MNU_Y_POS + FUIM_MENU_FIELD_NORMAL_TOP_MARGIN,     //Position MENU Y
  FUIM_MENU_WIDTH,    //Menu width
  IMG_ID_PROPERTY_1_PATIENT,   //ID Title MENU
  AUIM_MENU_TITLE_COLOUR,//Title Attributes
  0+AUIM_MNU_SHIFT_X,   //Position X PromptPos
  0+AUIM_MNU_SHIFT_X,  //Position X ValuePos
  &auim_PatientValidatingMenuFields[0],// Pointer to an array of field structures
  1,                              // Number of fields visible at once
  AUIM_MNU_PATIENT_VALIDATING_MAX_FIELD,// Total number of fields
  FUIM_MENU_NO_TIMEOUT, // How long the menu remains active;
  // if 0 - stays active indefinitely
  0,  // fuimFieldStruct const *LeftButtonField;  /* Pointer to structure of the Button field */
  &auim_MenuButtonFields[AUIM_CANCEL_BUTTON_FIELD],  // fuimFieldStruct const *RightButtonField; /* Pointer to structure of the Button field */
  0//&auim_FixedEmptyField[] /* @field pointer to structure of the fixed bottom field */
}
/*==================================================================*/
/*   P A T I E N T     V A L I D A T I N G    RESULT   MENU      */
/*==================================================================*/
,{
  FMNU_MAIN_MENU,      /* Menu type  {FMNU_MAIN_MENU FMNU_SUB_MAIN_MENU FMNU_MESSAGE_BOARD} */
  AUIM_MNU_X_POS,     //Position MENU X
  AUIM_MNU_Y_POS + FUIM_MENU_FIELD_NORMAL_TOP_MARGIN,     //Position MENU Y
  FUIM_MENU_WIDTH,    //Menu width
  IMG_ID_PROPERTY_1_PATIENT,   //ID Title MENU
  AUIM_MENU_TITLE_COLOUR,//Title Attributes
  0+AUIM_MNU_SHIFT_X,   //Position X PromptPos
  0+AUIM_MNU_SHIFT_X,  //Position X ValuePos
  &auim_PatientValidatingResultMenuFields[0],// Pointer to an array of field structures
  1,                              // Number of fields visible at once
  AUIM_MNU_PATIENT_VALIDATING_RESULT_MAX_FIELD,// Total number of fields
  FUIM_MENU_TIMEOUT, // How long the menu remains active;
  // if 0 - stays active indefinitely
  0,  // fuimFieldStruct const *LeftButtonField;  /* Pointer to structure of the Button field */
  &auim_MenuButtonFields[AUIM_CLOSE_BUTTON_FIELD],  // fuimFieldStruct const *RightButtonField; /* Pointer to structure of the Button field */
  0//&auim_FixedEmptyField[] /* @field pointer to structure of the fixed bottom field */
}


/*=================================================================*/
/*         C H A R G E   T H E   B A T T E R Y      M E N U S      */
/*=================================================================*/
/*=================================================================*/
/*         C H A R G E   T H E   B A T T E R Y            MENU     */
/*=================================================================*/
 ,{
    FMNU_MAIN_MENU,      /* Menu type  {FMNU_MAIN_MENU FMNU_SUB_MAIN_MENU FMNU_MESSAGE_BOARD} */
    AUIM_MNU_X_POS,     //Position MENU X
    AUIM_MNU_Y_POS + FUIM_MENU_FIELD_NORMAL_TOP_MARGIN,     //Position MENU Y
    FUIM_MENU_WIDTH,    //Menu width
    FMNU_NONE_TITLE,   //ID Title MENU
    AUIM_MENU_TITLE_COLOUR,//Title Attributes
    0+AUIM_MNU_SHIFT_X,   //Position X PromptPos
    0+AUIM_MNU_SHIFT_X,  //Position X ValuePos
    &auim_ChargeBatteryMenuFields[0],// Pointer to an array of field structures
    1,                              // Number of fields visible at once
    AUIM_MNU_CHARGE_BATTERY_MAX_FIELD,// Total number of fields
    FUIM_MENU_NO_TIMEOUT, // How long the menu remains active;
                           // if 0 - stays active indefinitely
    0,  // fuimFieldStruct const *LeftButtonField;  /* Pointer to structure of the Button field */
    &auim_MenuButtonFields[AUIM_PARAMS_BUTTON_FIELD],  // fuimFieldStruct const *RightButtonField; /* Pointer to structure of the Button field */
    0//&auim_FixedEmptyField[] /* @field pointer to structure of the fixed bottom field */
  }

};



/*===========================================================================*/
/* L O C A L   F U N C T I O N S                                             */
/*===========================================================================*/

/*===========================================================================*/
/*     G L O B A L   F U N C T I O N S                                       */
/*===========================================================================*/
