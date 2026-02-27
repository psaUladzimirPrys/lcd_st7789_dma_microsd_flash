/*=======================================================================*/
#ifndef _HFUIM_H
#define _HFUIM_H

/*=======================================================================*/
/*        I N C L U D E S                                                */
/*=======================================================================*/

#include "hglobal.h"
#include "adafruit_st7789.h"
#include "img_storage.h"

/*=======================================================================*/
/*        G L O B A L   D A T A   D E C L A R A T I O N S                */

/*=======================================================================*/
/*Definition values for positioning the Menu Title and the width and height of the MENU  in pixels */
#define FUIM_TITLE_RIGHT_MARGIN  21    //px
#define FUIM_TITLE_HEIGHT        44

#define FUIM_MENU_HEIGHT        93
#define FUIM_MENU_WIDTH        320

#define FUIM_FOOTER_HEIGHT      35


/*=======================================================================*/
/* @Macro General | Integer | The multiply of FUIM_MAX_NR_OF_ROWS and FUIM_MAX_NR_OF_COLS must not exceed 544. */
#define FUIM_MAX_NR_OF_ROWS 3

/*=======================================================================*/
/* @Macro General | Integer | The multiply of FUIM_MAX_NR_OF_ROWS and FUIM_MAX_NR_OF_COLS must not exceed 544. */
#define FUIM_MAX_NR_OF_COLS 16

/*=======================================================================*/
#define FUIM_MAX_INDICATORS 5 /*Maximum number of indicators allowed to be displayed on the screen simultaneously */
#define FUIM_MAX_DISPLAY_MENUS  1

/* @Macro General | Integer | 10 10 1 25 |Max number of fields in a menu. Does not affect the number of fields in an overview. */
#define FUIM_MAX_DISPLAY_FIELDS 5

/* @Macro General | Integer | 6 6 2 15 |Maximum length of string field. */
#define MAX_STRING_LENGTH 7

#define FUIM_MAX_NUMERIC_LENGTH 5
#define DIGIT_BUFFER_SIZE FUIM_MAX_NUMERIC_LENGTH


/* Need timers for indicators, menus, 1 overview and 1 periodic timer: */
#define FUIM_MAX_TIMERS (FUIM_MAX_INDICATORS + FUIM_MAX_DISPLAY_MENUS + 2)

/*=======================================================================*/

#define FUIM_GRAYED_OUT_COLOUR  ST7789_BLUE

/*=======================================================================*/
#define fuim_GetIndicatorVertLocation(pIndicator) pIndicator->VertLocation
#define fuim_GetIndicatorHorLocation(pIndicator)  pIndicator -> HorLocation
#define fuim_GetIndicatorFieldWidth(pIndicator)   pIndicator -> FieldWidth
#define fuim_GetIndicatorValuePos(pIndicator)     pIndicator -> ValuePos
#define fuim_GetIndicatorTimeout(pIndicator)      pIndicator->TimeOut

/*=======================================================================*/

#define fuim_GetFieldPromptColour(pField) (fuim_DynamicColours(pField->PromptColour))
#define fuim_GetFieldValidity(pField)     (fuim_Observer(pField->ValidityFunction))
#define fuim_GetFieldPrefix(pField)       (fuim_Observer(pField->Prefix))
#define fuim_GetFieldSuffix(pField)        pField->Suffix

/*=======================================================================*/

#define fuim_GetMenuVisibleFields(pMenu)          pMenu->VisibleFields
#define fuim_GetMenuVertLocation(pMenu)           pMenu->VertLocation
#define fuim_GetFixedTopField(pMenu)              pMenu->FixedTopField
#define fuim_GetFixedBottomField(pMenu)           pMenu->FixedBottomField
#define fuim_GetMenuHorLocation(pMenu)            pMenu->HorLocation

/*=======================================================================*/

#define fuim_GetActiveMenuTimeout(pTimeOut)       pTimeOut->TimeOut

/*=======================================================================*/

typedef Byte osdStringID;
typedef LongLong osdFieldValue;
typedef Byte osdDialogHandle;
typedef Byte osdLanguage;
typedef Byte osdTimerHandle;
typedef Byte *pByte;
typedef Byte cmdKeyNumber;


//Timeout
#define   FUIM_NO_ACTIVE_DIALOG   0
// #define  FUIM_NO_ACTIVE_FIELD    -1
#define   FUIM_NO_FREE_TIMER_HANDLE   0

#define FUIM_PERIODIC_TIMER 0 /* reserved index in SubTimers[]*/
/* When fuim uses the 64ms timer: this is to convert seconds->timer-ticks */
#define FUIM_TIMER_RESOLUTION  16   /* timer-ticks per second (1000/64) */

/*  Everything in the menu is being updated: */
#define FUIM_PERIODIC_TIMEOUT  3    /*  in timer-ticks  */

/*Value in seconds indicating after how long the entered numbers will be accepted (0 = no timeout) */
#define FUIM_NUMERIC_TIMEOUT 4

#define FUIM_MENU_TIMEOUT 10
#define FUIM_FIELD_NO_TIMEOUT 0
#define FUIM_FIELD_TIMEOUT 1

/*=======================================================================*/
typedef struct {
              Byte TimeOut;    /* в 'timer-ticks' */
              Byte TimerID;    /* ID of the function that will be called when the timer expires */
   osdDialogHandle Parameter;  /* given to 'timerfunction' */
} TIMER;

/*=======================================================================*/
 /*
     @enum fuim_TimerParentType | A timer can be associated with:
 */
typedef enum {
   FUIM_TIMERPARENTTYPE_MENU,      /* @emem The timer is associated with a menu */
   FUIM_TIMERPARENTTYPE_INDICATOR    /* @emem The timer is associated with an indicator */
 } fuim_TimerParentType;

/*=======================================================================*/
/* Timer function IDs */
enum Timer_ID {
  EMPTY_TIMER,
  PERIODIC_TIMER_FUNCTION,
  INDICATOR_TIMER_FUNCTION,
  MENU_TIMER_FUNCTION,
  FIELD_TIMER_FUNCTION,
  NUMERIC_TIMER_FUNCTION
};

/*=======================================================================*/
typedef enum  {
  FUIM_FIELDTYPE_SPACER,
  FUIM_FIELDTYPE_SLIDER,
  FUIM_FIELDTYPE_SLIDER_WRITE_ERASE,
  FUIM_FIELDTYPE_BALANCE,
  FUIM_FIELDTYPE_NUMERIC,
  FUIM_FIELDTYPE_LIST,
  FUIM_FIELDTYPE_ONOFFLIST,
  FUIM_FIELDTYPE_CHECKBOX,
  FUIM_FIELDTYPE_STRING,
  FUIM_FIELDTYPE_SEPARATOR,
  FUIM_FIELDTYPE_STRING_VALUE,  // Тот - же самый STRING но с запретом редактирования
  FUIM_FIELDTYPE_NUMERIC_VALUE,  // Тот - же самый NUMERIC но с запретом редактирования
  FUIM_FIELDTYPE_STRING_NUMERIC_VALUE  // Тот - же самый NUMERIC но с выводом строки типа --:-- когда значение
                                        //неправильно введено или таймер не включен

} fuim_FieldType;


/*=======================================================================*/
/*
    @enum fuim_Validity | Validity describes the appearance of a field in the dialog:
*/
typedef enum
{
  FUIM_VALIDITY_NOTPRESENT, /* Field is not present (so not selectable and/or visible) */
  FUIM_VALIDITY_VISIBLE , /* Field is visible but not selectable*/
  FUIM_VALIDITY_PRESENT , /* Field is present but not visible */
  FUIM_VALIDITY_SELECTABLE , /* Field is visible and selectable */
  FUIM_VALIDITY_GRAYEDOUT /* Field is grayed out, not selectable */
} fuim_Validity;

/*=======================================================================*/

typedef struct {
  Byte    Value;   /* @field actual valid value used in the set and get function of the corresponding field */
  Byte    ListItem;/* @field id-number of the string which replaces the value. */
} fmnu_ListStruct;

/*=======================================================================*/

typedef union {
  Byte  Slider;  /* @field number of segments used for displaying the slider.
                 <nl>Note 1: the number of segments specified by FUIM_SCALING_FACTOR
                 (default 4) form 1 osd-character as specified in the structure
                 fuimSliderChars.
                 <nl>Note 2: the size must be a multiplier of FUIM_SCALING_FACTOR.*/
  Byte  Balance; /* @field number of segments used for displaying the balance-bar.
                 <nl>Note 1: the number of segments specified by FUIM_SCALING_FACTOR
                 (default 4) form 1 osd-character as specified in the structure
                 fuimBalanceChars.
                 <nl>Note 2: the size must be a multiplier of FUIM_SCALING_FACTOR.*/
  Byte  Numeric; /* @field total number of digits to be entered. */
  Byte  String;  /* @field total number of characters which form the string. */
  Byte  Button;
  Byte  Separator;

} TFieldSize;

/*=======================================================================*/
typedef union {
  Byte    Slider;     /* @field value used for dividing the return-value of the GetFunction to
                      fit the number of segments in a slider */
  Byte    Balance;    /* @field value used for dividing the return-value of the GetFunction to
                      fit the number of segments in a balance-bar */
  Byte    Numeric;    /* @field number of digits to be entered after the decimal point */
  Byte    List;       /* @field number of valid list-values which will be displayed below each
                       other.
                       When 0: only the currently selected value will be displayed.
                     When >= than the number of valid values: all strings will be displayed */
} TFieldScaling;

/*=======================================================================*/
typedef union {
fmnu_ListStruct  *ListItem;       /* @field pointer to array of valid values for a list-field */
  char            NumericCharacter[2];      /* @field two characters used for displaying digit values:<nl>
//                                [0] defines the character used for spacing the integer and
//                                    the decimal part of a digit input field <nl>
//                                [1] defines the character used for digits which have not yet been entered */
  Byte        Spacer;             /* @field character used for displaying an 'empty' row */
osdStringID   Button;             /* ID - надписи на нопке*/
  char        StringRange[2];         /* @field Range of characters used in a string:<nl>
//                                [0] lowest character<nl>
//                                [1] highest character*/
//osdStringID   StringRangeID;          /* @field ID of string that includes the selectable characters for string field */
  char    BeginEndCharacters[2];      /* @field Begin and end character of a slider or balance:<nl>
//                                [0] begin character (e.g. '[')<nl>
//                                [1] end character (e.g. ']')*/
} TFieldCharacters;

/*=======================================================================*/
typedef struct {
  Byte Action; /* Action field (key number)*/
  Byte DialogFunction; /*Field of the function that is executed when the key is pressed. This is like an index in the function observer table*/
} fuimDialogNavigation;

/*=======================================================================*/
typedef struct {
    fuim_FieldType Type;
    // Field type (combines rendering type and control behavior)

    Byte ValidityFunction;
    // ID of the review function that returns the field status

    osdStringID Prompt;
    // ID of the string displayed as a prompt

    Byte PromptColour;
    // ID of the function that returns a pointer to the color structure

    Byte ChangeFunction;
    // ID of the transform function that modifies the field value
    // (screen update and redraw will be handled accordingly)

    Byte GetFunction;
    // ID of the function that returns the field value
    // (e.g., reads from a CPU register)

    Byte SetFunction;
    // ID of the function that writes the new value to CPU registers

    TFieldSize FieldSize;

    TFieldScaling FieldScaling;

    Byte Prefix;
    /* ID of the review function that returns a pointer to a string
       placed immediately before the field value (without extra spaces).
       Any required spaces must be included inside the string itself.
       If = 0, no prefix is displayed. */

    osdStringID Suffix;
    /* ID of the string or review function that returns a pointer
       to a string placed immediately after the field value
       (without extra spaces).
       Any required spaces must be included inside the string itself.
       If = 0, no suffix is displayed. */

    Byte TimeOut;
    /* Indicates whether this field has a timeout
       (how long it waits for value input).
       Applies only to FUIM_FIELDTYPE_NUMERIC fields.
       = 1 → timeout enabled
       = 0 → no timeout */

    Byte TriggerDigits;
    /* Used for numeric fields only */

    TFieldCharacters FieldCharacters;
    /* Character description structure */

    Byte Alignment;
    /* Button alignment within the area as defined
       in the corresponding enumeration */

    Byte LeadingZeros;
    /* Used for numeric fields only.
       Defines whether leading zeros are allowed */

    fuimDialogNavigation *ToDoWithKey;
    /* Pointer to an array of user-defined keys used by this field,
       or specifies how many valid list values will be displayed.
       If >= number of valid values, all strings will be displayed. */

} fuimFieldStruct;

/*=======================================================================*/
/*      Indicators
 *
 * */
/*=======================================================================*/
typedef struct {
  Byte VertLocation; //Row number where the upper left corner is located
  Byte HorLocation; //Column number where the upper left corner is located
  Byte ValuePos; /* @field total width of the field */
  Byte TimeOut; //Value in seconds for how long to display the field
  const fuimFieldStruct *Field;//Pointer to the drawing and control structure

} fuimIndicatorStruct;


/*=======================================================================*/
typedef struct {
  Byte  FirstPos;       /* @field Первая позиция столбца used by the menu/all fields equals MenuDataPtr->HorLocation */
  Byte  PromptPos ;       /* @field column-position where the prompt starts */
  Byte  ValuePos ;        /* @field position where the value starts */
  Byte  EndPos;         /* @field position of the end-box character */
  Byte    ValueWidth ;        /* @field total width of the value */
  Byte    FieldWidth ;        /* @field total width of the field */
}fuim_IndicatorProperty;


/*MPF=======================================================================*/
 typedef struct
{
  Byte          PromptColour;    /* @field Colour ID of function which returns pointer to a colour struct (FUIM_FIELD_DYNAMIC_COLOURS_SWITCH = GTV_ALWAYS) or pointer to colour struct used when displaying the field of this field as specified in fuimColourStruct (FUIM_FIELD_DYNAMIC_COLOURS_SWITCH = GTV_NEVER). */
  Byte              GetFunction; /* @field Observer ID of function which will return the text of the field. */
  Byte        Alignment;   /* @field Alignment of text in field as specified in fuim_Alignment */
} fuimFixedFieldStruct;


/************************************************************************
  Определяет каждый цвет используемый для прорисовки
    field-prompt,
    field-value
или   dialog title
*************************************************************************/
typedef struct {
  Byte ForeGndColour;       //Значения от 0 до 7
  Byte BackGndColour;       //Значения от 0 до 15
  Byte ForeGndHighLighted;  //Значения от 0 до 7
  Byte BackGndHighLighted;  //Значения от 0 до 15
  Byte Attribute;           // перечисление enum fuim_Attributes
  Byte AttributeHighLighted;// перечисление enum fuim_Attributes
} fuimColourStruct;

/*EMP=======================================================================*/


enum fuim_Attributes {
  FUIM_ATTRIBUTES_NONE = 0x1,
  FUIM_ATTRIBUTES_DOUBLEWIDTH = 0x2,
  FUIM_ATTRIBUTES_DOUBLEHEIGHT = 0x4,
  FUIM_ATTRIBUTES_FLASHING = 0x8,
  FUIM_ATTRIBUTES_ITALIC = 0x10,
  FUIM_ATTRIBUTES_SHADOWED = 0x20,
  FUIM_ATTRIBUTES_OVERLINED = 0x40,
  FUIM_ATTRIBUTES_UNDERLINED = 0x80
};

/********************************************************************************
    @enum fuim_Colours | Available colours.  The colours 0 through 7 are available
    for foreground colours and colours 8 through 15 are for background colours

*********************************************************************************/

typedef enum
{
  FUIM_COLOUR_0 ,         /* @emem Foreground Black   ( 0) */
  FUIM_COLOUR_1 ,         /* @emem Foreground Red     ( 1) */
  FUIM_COLOUR_2 ,         /* @emem Foreground GReen   ( 2) */
  FUIM_COLOUR_3 ,         /* @emem Foreground Yellow  ( 3) */
  FUIM_COLOUR_4 ,         /* @emem Foreground Blue    ( 4) */
  FUIM_COLOUR_5 ,         /* @emem Foreground Magenta ( 5) */
  FUIM_COLOUR_6 ,         /* @emem Foreground Cyan    ( 6) */
  FUIM_COLOUR_7,          /* @emem Foreground White   ( 7) */

  FUIM_COLOUR_8 ,         /* @emem Background Black   ( 8) */
  FUIM_COLOUR_9 ,         /* @emem Background Red     ( 9) */
  FUIM_COLOUR_10 ,        /* @emem Background GReen   (10) */
  FUIM_COLOUR_11 ,        /* @emem Background Yellow  (11) */
  FUIM_COLOUR_12 ,        /* @emem Background Blue    (12) */
  FUIM_COLOUR_13 ,        /* @emem Background Magenta (13) */
  FUIM_COLOUR_14 ,        /* @emem Background Cyan    (14) */
  FUIM_COLOUR_15 ,        /* @emem Background White   (15) */

  FUIM_COLOUR_TRANSPARENT     /* @emem Background Transparent  */
} fuim_Colour;

/*MPF=======================================================================*/
/*
    @enum fuim_Alignment |  The type of alignment of the text in a fixed field can be:
*/
//Совмещаем значения т к в на фиксированные поля никогда не ставится никакой указатель
// На них нет действия
typedef enum
{
  FUIM_ALIGNMENT_LEFT = 0,      /* @emem text will be left aligned */
  FUIM_ALIGNMENT_CENTRE,      /* @emem text will be centered */
  FUIM_ALIGNMENT_RIGHT      /* @emem text will be right aligned */

} fuim_Alignment;

typedef enum
{
    FUIM_REPEAT_KEY_ALWAYS   = FUIM_ALIGNMENT_LEFT,
    FUIM_REPEAT_KEY_128_MSEC,
    FUIM_REPEAT_KEY_256_MSEC,
    FUIM_REPEAT_KEY_ONE_SECOND = 8,
    FUIM_REPEAT_KEY_TWO_SECONDS = 16,
    FUIM_REPEAT_KEY_PRESSED_FOUR_SECONDS = 32,
    FUIM_REPEAT_KEY_PRESSED_FIVE_SECONDS = 48,
  FUIM_REPEAT_KEY_NONE

} fuim_AlignmentRepeated;


void fuim_Update(void);
void fuim_Init(void);

void fuim_InitIndicators( void );

void fuim_DrawTitle(img_storage_id_t img_id, Byte width, Word bg_color, Bool remove);

osdTimerHandle fuim_ConstructTimer( Byte TimeoutSeconds,Byte TimerID,osdDialogHandle hDialog );
void fuim_DestroyTimer(osdTimerHandle *hTimer );
void fuim_RestartTimer(osdTimerHandle hTimer,  Byte TimeoutInSeconds );

osdDialogHandle fuim_ConstructIndicator(fuimIndicatorStruct *indicator_data_ptr);
void fuim_UpdateIndicator(osdDialogHandle handle,Bool restart_timer);
void fuim_DestroyIndicator(osdDialogHandle indicator );



osdDialogHandle fuim_GetIndicatorHandle(fuimIndicatorStruct   *indicator_data_ptr);
void fuim_SetIndicatorTimeOut (osdDialogHandle hDialog,  Byte TimeOutInSeconds  );

void fuim_SetColumnPosition( Byte column );
void fuim_SetRowPosition( Byte row );

Byte fuim_GetRowPosition( void );

Byte fuim_GetColumnPosition( void );
void fuim_SetNextRow( void);

void fuim_SetAttributes(  Byte Attributes );
void fuim_SetBackgroundColour(Byte BackgroundColour, Byte SetAt);

void fuim_ConstructString(fuimFieldStruct  *field_data_ptr, Bool Highlighted);
void fuim_ConstructIndicatorValue( fuimFieldStruct  *field_data_ptr );
void fuim_ConstructIndicatorPrompt(fuimFieldStruct  *field_data_ptr,  Byte value_position );
void fuim_DrawString(img_storage_id_t img_id);

fuimColourStruct  *fuim_DynamicColours ( Byte index );

#endif /* _HFUIM_H */
