/*
 * fuim.c
 *
 *  Created on: 19 Marh. 2026
 *      Author: priss
 */

/*=======================================================================*/
/*        I N C L U D E S                                                */
/*=======================================================================*/
#include "app_log.h"
#include "app_assert.h"
#include "sl_sleeptimer.h"
#include "string.h"
#include "pltccstd.h"
#include "fuim.h"
#include "fmnu.h"
#include "auph.h"
#include "fuim_obs.h"
#include "auim_api.h"
#include "rbsc_api.h"
#include "disp.h"
#include "fmnu_str.h"
#include "aevs.h"

/*============================================================================*/
/*    G L O B A L  S Y M B O L    D E C L A R A T I O N S                     */
/*============================================================================*/
//#define TEST_BG_MENU_COLOR
/*=======================================================================*/
/* L O C A L   D E F I N I T I O N S                                     */
/*=======================================================================*/
#define FUIM_MAX_FONT_ID_COUNT        14U
#define FUIM_FONT_DOT_CHAR_INDEX      10U   /* '.' */
#define FUIM_FONT_COLON_CHAR_INDEX    11U   /* ':' */
#define FUIM_FONT_DASH_CHAR_INDEX     12U   /* '-' */
#define FUIM_FONT_BLANK_CHAR_INDEX    13U   /* ' ' */

typedef const uint16_t (*font_array_ptr_t)[FUIM_MAX_FONT_ID_COUNT];

/*=======================================================================
*   Font asset tables
*   Indexing: [Size][Color][Digit]
*   Located in Flash.
=======================================================================*/
                                                               //IMG_ID_PROPERTY_1_DEFAULT_6
// Small font (only 1 colour - black)                                      //0                          // 1                             // 2                       // 3                             //4                         //5                           //6                            //7                             //8                        //9                            //.                           //:                                //-                  // ' '
static const Word  font_small_black[FUIM_MAX_FONT_ID_COUNT] =  {IMG_ID_PROPERTY_1_VARIANT9_7,  IMG_ID_PROPERTY_1_VARIANT2_6, IMG_ID_PROPERTY_1_VARIANT3_6, IMG_ID_PROPERTY_1_VARIANT4_5,  IMG_ID_PROPERTY_1_VARIANT9_5, IMG_ID_PROPERTY_1_VARIANT8_5, IMG_ID_PROPERTY_1_VARIANT7_5, IMG_ID_PROPERTY_1_VARIANT6_5, IMG_ID_PROPERTY_1_VARIANT5_5, IMG_ID_PROPERTY_1_VARIANT10_5, IMG_ID_PROPERTY_1_VARIANT11_5, IMG_ID_PROPERTY_1_VARIANT20_1, IMG_ID_PROPERTY_1_VARIANT20_2, IMG_ID_X_LNK};

// Large font (4 colours)
static const Word font_large_color1[FUIM_MAX_FONT_ID_COUNT] = {IMG_ID_PROPERTY_1_VARIANT11,   IMG_ID_PROPERTY_1_VARIANT2_2, IMG_ID_PROPERTY_1_VARIANT3_2, IMG_ID_PROPERTY_1_VARIANT4_1, IMG_ID_PROPERTY_1_VARIANT9_1, IMG_ID_PROPERTY_1_VARIANT8_1, IMG_ID_PROPERTY_1_VARIANT7_1, IMG_ID_PROPERTY_1_VARIANT6_1, IMG_ID_PROPERTY_1_VARIANT5_1, IMG_ID_PROPERTY_1_VARIANT10_1, IMG_ID_PROPERTY_1_VARIANT11_6, IMG_INVALID_ID, IMG_INVALID_ID, IMG_INVALID_ID};
static const Word font_large_color2[FUIM_MAX_FONT_ID_COUNT] = {IMG_ID_PROPERTY_1_VARIANT11_1, IMG_ID_PROPERTY_1_VARIANT2_3, IMG_ID_PROPERTY_1_VARIANT3_3, IMG_ID_PROPERTY_1_VARIANT4_2, IMG_ID_PROPERTY_1_VARIANT9_2, IMG_ID_PROPERTY_1_VARIANT8_2, IMG_ID_PROPERTY_1_VARIANT7_2, IMG_ID_PROPERTY_1_VARIANT6_2, IMG_ID_PROPERTY_1_VARIANT5_2, IMG_ID_PROPERTY_1_VARIANT10_2, IMG_INVALID_ID, IMG_INVALID_ID, IMG_INVALID_ID, IMG_INVALID_ID};
static const Word font_large_color3[FUIM_MAX_FONT_ID_COUNT] = {IMG_ID_PROPERTY_1_VARIANT11_2, IMG_ID_PROPERTY_1_VARIANT2_4, IMG_ID_PROPERTY_1_VARIANT3_4, IMG_ID_PROPERTY_1_VARIANT4_3, IMG_ID_PROPERTY_1_VARIANT9_3, IMG_ID_PROPERTY_1_VARIANT8_3, IMG_ID_PROPERTY_1_VARIANT7_3, IMG_ID_PROPERTY_1_VARIANT6_3, IMG_ID_PROPERTY_1_VARIANT5_3, IMG_ID_PROPERTY_1_VARIANT10_3, IMG_INVALID_ID, IMG_INVALID_ID, IMG_INVALID_ID, IMG_INVALID_ID};
static const Word font_large_color4[FUIM_MAX_FONT_ID_COUNT] = {IMG_ID_PROPERTY_1_VARIANT11_3, IMG_ID_PROPERTY_1_VARIANT2_5, IMG_ID_PROPERTY_1_VARIANT3_5, IMG_ID_PROPERTY_1_VARIANT4_4, IMG_ID_PROPERTY_1_VARIANT9_4, IMG_ID_PROPERTY_1_VARIANT8_4, IMG_ID_PROPERTY_1_VARIANT7_4, IMG_ID_PROPERTY_1_VARIANT6_4, IMG_ID_PROPERTY_1_VARIANT5_4, IMG_ID_PROPERTY_1_VARIANT10_4, IMG_INVALID_ID, IMG_INVALID_ID, IMG_INVALID_ID, IMG_INVALID_ID};

/*===============================================================
*   Font lookup table
*   LUT: [Size][Colour]
*   For SMALL size, only index [0][0] is used.
*   For LARGE size, indexes [1][0...3] are used.
===============================================================*/
static const font_array_ptr_t font_lut[FUIM_MAX_FONT_SIZE][FUIM_MAX_FONT_COLOR] = {
  { &font_small_black, NULL, NULL, NULL },
  { &font_large_color1, &font_large_color2, &font_large_color3, &font_large_color4 }
};

/*===============================================================
*   Colour tables
===============================================================*/
const fuimColourStruct MenuTitleColor =
{
   FUIM_COLOUR_8,       /* Foreground color             */
 #ifdef TEST_BG_MENU_COLOR 
   ST7789_GREEN,                       /* Background color             */ 
 #else
   FUIM_COLOUR_1,       /* Background color             */
 #endif
   FUIM_COLOUR_8,       /* Foreground highlighted color */
   FUIM_COLOUR_TRANSPARENT,       /* Background highlighted color */
   FUIM_TOP_ROW_SIZE + FUIM_ATTRIBUTES_NONE,/* No attributes                 */
   FUIM_ATTRIBUTES_NONE /* attributes highlighted     */
};

const fuimColourStruct MenuNormalFieldColour =
{
  FUIM_COLOUR_8,       /* Foreground color             */
#ifdef TEST_BG_MENU_COLOR 
  ST7789_GREEN,                       /* Background color             */ 
#else
  FUIM_COLOUR_9,       /* Background color             */
#endif
  FUIM_COLOUR_8,       /* Foreground highlighted color */
  FUIM_COLOUR_TRANSPARENT,       /* Background highlighted color */
  FUIM_MENU_ROW_SIZE + FUIM_ATTRIBUTES_NONE,/* No attributes                 */
  FUIM_ATTRIBUTES_NONE /* attributes highlighted      */
};

const fuimColourStruct MenuDoubleFieldColour =
{
  FUIM_COLOUR_8,                       /* Foreground color             */
#ifdef TEST_BG_MENU_COLOR 
  ST7789_GREEN,                       /* Background color             */ 
#else
  FUIM_COLOUR_9,                       /* Background color             */
#endif
  FUIM_COLOUR_8,                       /* Foreground highlighted color */
  FUIM_COLOUR_TRANSPARENT,             /* Background highlighted color */
  FUIM_MENU_ROW_DOUBLE_SIZE + FUIM_ATTRIBUTES_NONE, /* With shadowing   */
  FUIM_ATTRIBUTES_NONE         /*attributes highlighted */
};

const fuimColourStruct MenuSplitFieldColour =
{
  FUIM_COLOUR_8,       /* Foreground color             */
#ifdef TEST_BG_MENU_COLOR 
  ST7789_GREEN,                       /* Background color             */ 
#else
  FUIM_COLOUR_9,       /* Background color             */
#endif
  FUIM_COLOUR_8,       /* Foreground highlighted color */
  FUIM_COLOUR_TRANSPARENT,       /* Background highlighted color */
  FUIM_MENU_ROW_SIZE + FUIM_ATTRIBUTES_SPLITROW,/* No attributes                 */
  FUIM_ATTRIBUTES_NONE /* attributes highlighted      */
};

/* Colour table: default button prompt colours */
const fuimColourStruct ButtonPromptColour =
{
   FUIM_COLOUR_8,                       /* Foreground color             */
 #ifdef TEST_BG_MENU_COLOR 
   ST7789_GREEN,                       /* Background color             */ 
 #else
   FUIM_COLOUR_2,                       /* Background color             */
 #endif
   FUIM_COLOUR_8,                       /* Foreground highlighted color */
   FUIM_COLOUR_TRANSPARENT,              /* Background highlighted color */
   FUIM_BOTTOM_ROW_SIZE + FUIM_ATTRIBUTES_NONE, /* With shadowing   */
   FUIM_ATTRIBUTES_NONE         /*attributes highlighted */
};

/* Colour table: default indicator prompt colours */
const fuimColourStruct IndicatorPromptColour =
{
   FUIM_COLOUR_8,                       /* Foreground color             */
   FUIM_COLOUR_1,                       /* Background color             */
   FUIM_COLOUR_8,                       /* Foreground highlighted color */
   FUIM_COLOUR_TRANSPARENT,              /* Background highlighted color */
   FUIM_TOP_ROW_SIZE + FUIM_ATTRIBUTES_NONE, /* With shadowing   */
   FUIM_ATTRIBUTES_NONE         /*attributes highlighted */
};

/* Colour table: modal indicator prompt colours */
const fuimColourStruct IndicatorModalPromptColour =
{
   FUIM_COLOUR_8,                       /* Foreground color             */
   FUIM_COLOUR_0,                       /* Background color             */
   FUIM_COLOUR_8,                       /* Foreground highlighted color */
   FUIM_COLOUR_TRANSPARENT,             /* Background highlighted color */
   FUIM_NOTIFICATION_ROW_SIZE + FUIM_ATTRIBUTES_NONE, /* With shadowing   */
   FUIM_ATTRIBUTES_NONE         /*attributes highlighted */
};

/* Colour table: splash screen indicator colours */
const fuimColourStruct IndicatorSplashScreenColour =
{
  FUIM_COLOUR_8,                       /* Foreground color             */
  FUIM_COLOUR_9,                       /* Background color             */
  FUIM_COLOUR_8,                       /* Foreground highlighted color */
  FUIM_COLOUR_TRANSPARENT,             /* Background highlighted color */
  FUIM_SPLASH_SCREEN_ROW_SIZE + FUIM_ATTRIBUTES_NONE, /* With shadowing   */
  FUIM_ATTRIBUTES_NONE         /*attributes highlighted */
};

/* Colour table: performance notification colours */
const fuimColourStruct MenuPerformanceNotoficationColour =
{
  FUIM_COLOUR_8,                       /* Foreground color             */
  FUIM_COLOUR_5,                       /* Background color             */
  FUIM_COLOUR_8,                       /* Foreground highlighted color */
  FUIM_COLOUR_TRANSPARENT,             /* Background highlighted color */
  FUIM_NOTIFICATION_ROW_SIZE + FUIM_ATTRIBUTES_NONE, /* With shadowing   */
  FUIM_ATTRIBUTES_NONE         /*attributes highlighted */
};

/* Colour table: patient notification colours */
const fuimColourStruct MenuPatientNotificationColour =
{
  FUIM_COLOUR_8,                       /* Foreground color             */
  FUIM_COLOUR_6,                       /* Background color             */
  FUIM_COLOUR_8,                       /* Foreground highlighted color */
  FUIM_COLOUR_TRANSPARENT,             /* Background highlighted color */
  FUIM_NOTIFICATION_ROW_SIZE + FUIM_ATTRIBUTES_NONE, /* With shadowing   */
  FUIM_ATTRIBUTES_NONE         /*attributes highlighted */
};

/* Colour table: reference notification colours */
const fuimColourStruct MenuReferenceNotificationColour =
{
  FUIM_COLOUR_8,                       /* Foreground color             */
  FUIM_COLOUR_7,                       /* Background color             */
  FUIM_COLOUR_8,                       /* Foreground highlighted color */
  FUIM_COLOUR_TRANSPARENT,             /* Background highlighted color */
  FUIM_NOTIFICATION_ROW_SIZE + FUIM_ATTRIBUTES_NONE, /* With shadowing   */
  FUIM_ATTRIBUTES_NONE         /*attributes highlighted */
};

/* Colour table: red modal indicator colours */
const fuimColourStruct IndicatorRedModalPromptColour =
{
  FUIM_COLOUR_8,                       /* Foreground color             */
  FUIM_COLOUR_3,                       /* Background color             */
  FUIM_COLOUR_8,                       /* Foreground highlighted color */
  FUIM_COLOUR_TRANSPARENT,             /* Background highlighted color */
  FUIM_NOTIFICATION_ROW_SIZE + FUIM_ATTRIBUTES_NONE, /* With shadowing   */
  FUIM_ATTRIBUTES_NONE         /*attributes highlighted */
};

/* Colour table: green modal indicator colours */
const fuimColourStruct IndicatorGreenModalPromptColour =
{
  FUIM_COLOUR_8,                       /* Foreground color             */
  FUIM_COLOUR_4,                       /* Background color             */
  FUIM_COLOUR_8,                       /* Foreground highlighted color */
  FUIM_COLOUR_TRANSPARENT,             /* Background highlighted color */
  FUIM_NOTIFICATION_ROW_SIZE + FUIM_ATTRIBUTES_NONE, /* With shadowing   */
  FUIM_ATTRIBUTES_NONE         /*attributes highlighted */
};

/* Colour table: error indicator colours */
const fuimColourStruct IndicatorErrorColour =
{
  FUIM_COLOUR_8,                       /* Foreground color             */
#ifdef TEST_BG_MENU_COLOR 
  ST7789_GREEN,                       /* Background color             */ 
#else
  FUIM_COLOUR_9,                       /* Background color             */
#endif
  FUIM_COLOUR_8,                       /* Foreground highlighted color */
  FUIM_COLOUR_TRANSPARENT,             /* Background highlighted color */
  FUIM_MENU_ROW_DOUBLE_SIZE + FUIM_ATTRIBUTES_NONE, /* With shadowing   */
  FUIM_ATTRIBUTES_NONE         /*attributes highlighted */
};


/* Periodic timer resolution constant */
#define FUIM_PERIODIC_TIMER_DELAY_64MS  64

/*============================================================================*/
/*    L O C A L  S Y M B O L    D E C L A R A T I O N S                     */
/*============================================================================*/

/*--- Global/Static variables ---*/

/* Active indicator instances (1-based) */
static fuimIndicatorStruct            * fuim_Indicators[FUIM_MAX_INDICATORS];
/* Cached indicator layout properties */
static fuim_IndicatorProperty fuim_indicator_properties[FUIM_MAX_INDICATORS];
/* Indicator timeout timer handles (1-based, FUIM_NO_FREE_TIMER_HANDLE if empty) */
static osdTimerHandle            indicator_timer_handle[FUIM_MAX_INDICATORS];
/* Runtime timer slots (index 0 reserved for periodic timer) */
static TIMER fuim_Timer[FUIM_MAX_TIMERS];
/* Periodic sleeptimer handle for 64 ms UI tick */
static sl_sleeptimer_timer_handle_t  fuim_timeout_timer_handle;


/*=============================================================================*/
/*    L O C A L   F U N C T I O N   P R O T O T Y P E S                        */
/*=============================================================================*/
static void SetupPeriodicTimer (Byte timeOutInTicks);

void fuim_PeriodicTimerExpired(void);
void fuim_UpdateTimer064ms(void);
void fuim_TimerFunction(Byte index, osdDialogHandle hDialog);

static inline Word fuim_DigitToFontAssetId(char c, fuim_FontSize size, fuim_FontColor color_idx);
void fuim_DrawNumeric( osdFieldValue GetFunction,
                       Byte FieldScalingNumeric,
                       Byte FieldSizeNumeric,
                       fuim_FontSize  size,
                       fuim_FontColor color,
                       fuim_Alignment align,
                       Byte margin_top
                      );

void fuim_ConstructStringVerticalMargin(img_storage_id_t img_id, Byte  MarginTop, Word  Xpos );

/*==========================================================================*/
/*    L O C A L   F U N C T I O N                                           */
/*==========================================================================*/
/*=================================================================================
*   Function:    fuim_DigitToFontAssetId
*   Description: Maps an ASCII character to a font asset ID by indexing the font
*                lookup table (font_lut) using size and color indices.
*
*   Arguments:
*   Parameter    Flow    Description
*   ------------------------------------------------------------------------------
*   c            IN      ASCII character to convert ('0'-'9', '.', ':', '-', ' ')
*   size         IN      Font size enum (FUIM_FONT_SIZE_SMALL/LARGE)
*   color_idx    IN      Font color index into font_lut
*
*   Externals    Flow    Usage
*   ------------------------------------------------------------------------------
*   font_lut[]   IN      Font lookup table indexed by [size][color_idx]
*
*   @return Font asset ID (Word) for the character, or IMG_INVALID_ID if unsupported
*
*   @note Returns IMG_INVALID_ID for out-of-range size/color or NULL font table entry.
===================================================================================*/
static inline Word fuim_DigitToFontAssetId(char c, fuim_FontSize size, fuim_FontColor color_idx)
{

  Byte char_idx;

  // 1. Converting ASCII to an array index
  if (c >= '0' && c <= '9') {
      char_idx = (Byte)(c - '0');
  } else if (c == '.') {
      char_idx = FUIM_FONT_DOT_CHAR_INDEX;
  } else if (c == ':') {
      char_idx = FUIM_FONT_COLON_CHAR_INDEX;
  } else if (c == '-') {
    char_idx = FUIM_FONT_DASH_CHAR_INDEX;
  } else if (c == ' ') {
    char_idx = FUIM_FONT_BLANK_CHAR_INDEX;
  } else {
      return IMG_INVALID_ID; // Unsupported character
  }

  // 2. Parameter validation
  if (size >= FUIM_MAX_FONT_SIZE || color_idx >= FUIM_MAX_FONT_COLOR) {
      return IMG_INVALID_ID;
  }

  // 3. Access via the pointer table
  font_array_ptr_t selected_font = font_lut[size][color_idx];

  if (selected_font == NULL) {
      return IMG_INVALID_ID;
  }

  // Dereference the pointer to the array and take the required element.
  return (Word)(*selected_font)[char_idx];
}

/*=================================================================================
*   Function:    fuim_timer_callback
*   Description: Sleep timer callback that generates 64 ms UI timing ticks.
*
*   Arguments:
*   Parameter    Flow    Description
*   ------------------------------------------------------------------------------
*   handle       IN      Sleep timer handle (unused)
*   data         IN      Callback data pointer (unused)
*
*   Externals    Flow    Usage
*   ------------------------------------------------------------------------------
*   None
*
*   @return void
*
*   @note Calls fuim_UpdateTimer064ms() to tick all runtime timers.
===================================================================================*/
static void fuim_timer_callback(sl_sleeptimer_timer_handle_t *handle, void *data)
{
  (void)handle;
  (void)data;
  fuim_UpdateTimer064ms();
}

/*=================================================================================
*   Function:    SetupPeriodicTimer
*   Description: Configures the periodic timer slot with timeout, timer ID, and dialog handle.
*
*   Arguments:
*   Parameter    Flow    Description
*   ------------------------------------------------------------------------------
*   timeOutInTicks IN   Timer timeout in ticks
*
*   Externals    Flow    Usage
*   ------------------------------------------------------------------------------
*   fuim_Timer[]   OUT  Updated[FUIM_PERIODIC_TIMER] with TimeOut/TimerID/Parameter
*
*   @return void
*
*   @note Used to (re)configure the periodic 64 ms timer.
===================================================================================*/
static void SetupPeriodicTimer (Byte timeOutInTicks)
{
    fuim_Timer[FUIM_PERIODIC_TIMER].TimeOut   = timeOutInTicks;
    fuim_Timer[FUIM_PERIODIC_TIMER].TimerID   = PERIODIC_TIMER_FUNCTION;
    fuim_Timer[FUIM_PERIODIC_TIMER].Parameter = (osdDialogHandle) timeOutInTicks;
}

/*=================================================================================
*   Function:    fuim_DrawTitle
*   Description: Draws a title image at the current cursor position with top margin.
*
*   Arguments:
*   Parameter    Flow    Description
*   ------------------------------------------------------------------------------
*   img_id       IN      Image storage ID of the title graphic
*
*   Externals    Flow    Usage
*   ------------------------------------------------------------------------------
*   None directly; uses fuim_ConstructStringVerticalMargin()
*
*   @return void
*
*   @note Only draws if img_id is valid.
===================================================================================*/
void fuim_DrawTitle(img_storage_id_t img_id)
{
  fuim_ConstructStringVerticalMargin(img_id, FUIM_TITLE_TOP_MARGIN, fuim_GetColumnPosition());
}

/*=================================================================================
*   Function:    fuim_DrawString
*   Description: Draws a string image at the current cursor position after bounds checks.
*
*   Arguments:
*   Parameter    Flow    Description
*   ------------------------------------------------------------------------------
*   img_id       IN      Image storage ID of the string graphic
*
*   Externals    Flow    Usage
*   ------------------------------------------------------------------------------
*   None directly; uses plt_CC* cursor state
*
*   @return void
*
*   @note Asserts on out-of-bounds image ID, x position, or y position.
*       Advances cursor by image width after drawing.
===================================================================================*/
void fuim_DrawString(img_storage_id_t img_id)
{
  Word x_poz;
  Word y_poz;
  Word height;
  Word width;

  if (!(img_id < IMG_MAX_IDS_STORAGE_DESC_COUNT)) { //ID is an Invalid
      app_log("img storage ID: %d Failed\r\n", img_id);
      app_assert_status(SL_STATUS_FAIL); // Loop forever for debugging
  }
  plt_CCGetPosition(&y_poz, &x_poz);

  width = IMG_GET_WIDTH(img_id);
  height = IMG_GET_HEIGHT(img_id);

  if(width + x_poz > FUIM_MENU_WIDTH) {
   app_log("width + x_poz > FUIM_MENU_WIDTH -> Failed: %d + %d\r\n", width, x_poz);
    app_assert_status(SL_STATUS_FAIL); // Loop forever for debugging
  }

  if ( height + y_poz > FUIM_MENU_HEIGHT ) {
    app_log("height + y_poz > FUIM_MENU_HEIGHT -> Failed: %d + %d\r\n", height, y_poz );
    app_assert_status(SL_STATUS_FAIL); // Loop forever for debugging
  }

  disp_DrawImage(x_poz, y_poz, img_id);
  plt_CCSetPosition(y_poz, (x_poz + width));

}

/*=================================================================================
*   Function:    fuim_EraseField
*   Description: Erases a horizontal field segment by filling it with repeated background
*                characters from FirstPos to EndPos.
*
*   Arguments:
*   Parameter    Flow    Description
*   ------------------------------------------------------------------------------
*   FirstPos     IN      Start column for erasure
*   EndPos       IN      End column for erasure
*   Type         IN      Field type (unused, kept for compatibility)
*
*   Externals    Flow    Usage
*   ------------------------------------------------------------------------------
*   None directly; uses plt_CC* cursor state via fuim_DrawRepeatedCharacter()
*
*   @return void
*
*   @note Type parameter is present but not used in current implementation.
===================================================================================*/
void fuim_EraseField( Word FirstPos, Word EndPos, fuim_FieldType Type )
{
  Word  length;

  length = EndPos - FirstPos;
  fuim_DrawRepeatedCharacter(length);

  Type = Type;
  length = length;

}

/*=================================================================================
*   Function:    fuim_InitTimers
*   Description: Resets all runtime timer slots to empty/stopped state.
*
*   Arguments:        None
*
*   Externals    Flow    Usage
*   ------------------------------------------------------------------------------
*   fuim_Timer[]      OUT  All slots set to TimeOut=RGEN_TIMER_STOPPED, TimerID=EMPTY_TIMER, Parameter=0
*
*   @return void
*
*   @note Must be called before timer operations. Called from fuim_Init() and fuim_TurnOn().
===================================================================================*/
void fuim_InitTimers(void)
{
    osdTimerHandle i;
    for (i = 0; i < FUIM_MAX_TIMERS; i++) {
      fuim_Timer[i].TimeOut   = RGEN_TIMER_STOPPED;
      fuim_Timer[i].TimerID   = EMPTY_TIMER;
      fuim_Timer[i].Parameter = 0;
    }
    return;
}

/*=================================================================================
*   Function:    fuim_InitIndicators
*   Description: Clears all indicator slots and timer handles to free/empty state.
*
*   Arguments:        None
*
*   Externals    Flow    Usage
*   ------------------------------------------------------------------------------
*   fuim_Indicators[]         OUT  All slots set to NULL
*   indicator_timer_handle[]  OUT  All handles set to FUIM_NO_FREE_TIMER_HANDLE
*
*   @return void
*
*   @note Must be called before indicator operations. Called from fuim_Init() and fuim_TurnOn().
===================================================================================*/
void fuim_InitIndicators(void)
{
  Byte  i;

  for( i = 0; i < FUIM_MAX_INDICATORS; i++) {
    fuim_Indicators[i] = NULL ;			
    indicator_timer_handle[i] = FUIM_NO_FREE_TIMER_HANDLE ;
  }
  return;
}

/*=================================================================================
*   Function:    fuim_GetIndicatorHandle
*   Description: Searches the active indicator array for a matching indicator pointer
*                and returns its 1-based handle, or 0 if not found.
*
*   Arguments:
*   Parameter    Flow    Description
*   ------------------------------------------------------------------------------
*   indicator_data_ptr IN   Indicator structure pointer to search for
*
*   Externals    Flow    Usage
*   ------------------------------------------------------------------------------
*   fuim_Indicators[]  IN   Active indicator pointer array
*
*   @return 1-based handle (1..FUIM_MAX_INDICATORS) if found, 0 otherwise
*
*   @note Handles are 1-based to distinguish NULL/empty slot (0) from valid slot (1..N).
===================================================================================*/
osdDialogHandle fuim_GetIndicatorHandle(const fuimIndicatorStruct *indicator_data_ptr)
{
  osdDialogHandle  i;

  for(i = 0; i < FUIM_MAX_INDICATORS && (fuim_Indicators[i]!=indicator_data_ptr); i++ );

  if (i < FUIM_MAX_INDICATORS) {
    return i + 1;
  }
  return 0;
}

/*=================================================================================
*   Function:    fuim_IsModalIndicatorActive
*   Description: Checks whether any currently active indicator is of modal notification type.
*
*   Arguments:        None
*
*   Externals    Flow    Usage
*   ------------------------------------------------------------------------------
*   fuim_Indicators[]  IN  Active indicator pointer array
*
*   @return TRUE if at least one modal indicator is active, FALSE otherwise
*
*   @note Modal indicators block menu updates and prevent additional modal creation.
===================================================================================*/
Bool fuim_IsModalIndicatorActive(void)
{
  for (Byte i = 0; i < FUIM_MAX_INDICATORS; i++) {
    if (fuim_Indicators[i] != NULL) {
      // Check indicator's field type or IsModal flag
      fuimFieldStruct *field_data_ptr = (fuimFieldStruct *)fuim_Indicators[i]->Field;
      if (field_data_ptr->Type == FUIM_FIELDTYPE_MODAL_NOTIFICATION) {
        return TRUE;
      }
    }
  }
  return FALSE;
}

/*=================================================================================
*   Function:    fuim_ConstructString
*   Description: Renders a string field by looking up the observer value as an image ID
*                and drawing it at the current position with vertical margin.
*
*   Arguments:
*   Parameter    Flow    Description
*   ------------------------------------------------------------------------------
*   field_data_ptr IN    Pointer to field structure containing GetFunction
*   margin_top    IN    Vertical margin in pixels for string positioning
*
*   Externals    Flow    Usage
*   ------------------------------------------------------------------------------
*   fuim_Observer()  IN  Observer dispatch returning img_storage_id_t
*
*   @return void
*
*   @note Skips if GetFunction == AUIM_NO_GET_FUNCTION or observer returns invalid ID.
===================================================================================*/
void fuim_ConstructString(fuimFieldStruct  *field_data_ptr, Byte margin_top )
{
  if (field_data_ptr->GetFunction == AUIM_NO_GET_FUNCTION) {
    return;
  }

  img_storage_id_t img_id = (img_storage_id_t) fuim_Observer (field_data_ptr->GetFunction);
 
  if(!(img_id < IMG_MAX_IDS_STORAGE_DESC_COUNT)) { //ID is an Invalid
    return;   //There is NOT string ID to display
  }

   fuim_ConstructStringVerticalMargin(img_id, margin_top, fuim_GetColumnPosition());

}

/*=================================================================================
*   Function:    fuim_ConstructStringNumeric
*   Description: Renders a numeric string field (alphanumeric display) by decoding
*                prefix/value/suffix and drawing characters via the font asset ID system.
*
*   Arguments:
*   Parameter    Flow    Description
*   ------------------------------------------------------------------------------
*   field_data_ptr IN    Pointer to field structure with numeric content
*   margin_top    IN    Vertical margin in pixels
*
*   Externals    Flow    Usage
*   ------------------------------------------------------------------------------
*   fuim_Observer()  IN  Observer dispatch for prefix/value/suffix IDs
*   font_lut[]       IN  Font lookup table for character asset IDs
*
*   @return void
*
*   @note Truncates string if it exceeds FieldSize. Uses fmnu_str_Prefix/Suffix tables.
*       Alignment and padding are calculated from field width and current cursor position.
===================================================================================*/
void fuim_ConstructStringNumeric(fuimFieldStruct  *field_data_ptr, Byte margin_top )
{
  if (field_data_ptr->GetFunction == AUIM_NO_GET_FUNCTION) {
    return;
  }

  char * numeric_string = (char *)fuim_Observer(field_data_ptr->GetFunction);

  if (numeric_string == NULL) {  //There is NOT string ID to display
    return;
  }

  Byte       FieldSize = field_data_ptr->FieldSize.Numeric;
  fuim_FontSize   size = field_data_ptr->FieldCharacters.NumericFont.size;
  fuim_FontColor color = field_data_ptr->FieldCharacters.NumericFont.color;
  fuim_Alignment align = field_data_ptr->Alignment;
  Word      char_width = ( size == FUIM_FONT_SIZE_LARGE ) ? FUIM_DIGITS_CHAR_WIDTH_LARGE : FUIM_DIGITS_CHAR_WIDTH_SMALL;
  size_t    actual_len = strlen(numeric_string);

  Byte          Prefix = FMNU_PREFIX_NONE;
  Byte          Suffix = FMNU_SUFFIX_NONE;

  img_storage_id_t  prefix_img_id = IMG_INVALID_ID;
  img_storage_id_t  suffix_img_id = IMG_INVALID_ID;
  Word prefix_width = 0;
  Word suffix_width = 0;

  Prefix = field_data_ptr->Prefix;
  Suffix = field_data_ptr->Suffix;

  if(Prefix != FMNU_PREFIX_NONE ) {  
    prefix_img_id = (img_storage_id_t)fmnu_str_Prefix[fuim_Observer(Prefix)];
    prefix_width = IMG_GET_WIDTH(prefix_img_id);
  }

  if(Suffix != FMNU_SUFFIX_NONE ) {  
    suffix_img_id = (img_storage_id_t)fmnu_str_Suffix[fuim_Observer(Suffix)];
    suffix_width = IMG_GET_WIDTH(suffix_img_id);
  }

  /* Safety check: if the string exceeds the field size, truncate it. */
  if (actual_len > FieldSize) {
      actual_len = FieldSize;
  }

 /* 2. Alignment calculation.
 * Calculate the pixel offset within a box of 'FieldSize' width.
 */
  Word total_field_px = (FieldSize  * char_width) + prefix_width + suffix_width;
  Word  text_width_px = (actual_len * char_width) + prefix_width + suffix_width;
  Word      current_x = fuim_GetColumnPosition();
  Word last_current_x = current_x;

  if (total_field_px > text_width_px) {
      switch (align) {
          case FUIM_ALIGNMENT_RIGHT:
              current_x += (total_field_px - text_width_px);
              break;
          case FUIM_ALIGNMENT_CENTRE:
              current_x += (total_field_px - text_width_px) / 2;
              break;
          case FUIM_ALIGNMENT_LEFT:
          default:
            /* current_x remains at start x */
              break;
      }
  }

  // 2.1. Draw padding/filler characters before the text
  Word Count = (current_x > last_current_x) ? (current_x - last_current_x) : (0);
  fuim_SetColumnPosition(last_current_x);
  fuim_DrawRepeatedCharacter(Count);

  // 2.1. Draw preffix before the text
  if ( Prefix != FMNU_PREFIX_NONE ) {  
    fuim_ConstructStringVerticalMargin(prefix_img_id, margin_top, fuim_GetColumnPosition());
  }

  /* 3. Character rendering loop via Asset ID system. */
  for (Byte i = 0; i < actual_len; i++) {

    /* Retrieve the specific Asset ID for the character. */
    Word asset_id = fuim_DigitToFontAssetId(numeric_string[i], size, color);

    //fuim_SetColumnPosition(current_x);
    /* Render if the symbol (digit, dot, or minus) exists in the table. */
    if (asset_id != IMG_INVALID_ID) {
        //fuim_DrawString(asset_id);
        fuim_ConstructStringVerticalMargin(asset_id, margin_top, current_x);
    }

    /* Shift the cursor by one character width. */
    current_x += char_width;
  }
 
  // 3.1. Draw suffix after the text
  if ( Suffix != FMNU_SUFFIX_NONE ) {  
    fuim_ConstructStringVerticalMargin(suffix_img_id, margin_top, fuim_GetColumnPosition());
  }

}


/*=================================================================================
*   Function:    fuim_DrawRepeatedCharacter
*   Description: Fills a horizontal segment with background-colored characters
*                using the current cursor row and colors.
*
*   Arguments:
*   Parameter    Flow    Description
*   ------------------------------------------------------------------------------
*   width         IN      Width in pixels to fill
*
*   Externals    Flow    Usage
*   ------------------------------------------------------------------------------
*   plt_CC* state  IN/OUT  Uses current row size, position, foreground/background colors
*   disp_EraseImage called IN  Hardware display driver
*
*   @return void
*
*   @note Asserts if width extends beyond FUIM_MENU_WIDTH or row height exceeds FUIM_MENU_HEIGHT.
===================================================================================*/
void fuim_DrawRepeatedCharacter(Word width)
{
  Word x_poz;
  Word y_poz;
  Word height;
  Word bg_color;
  Word fg_color;

  if (width == 0) return;

  height = plt_CCGetRowSize();
  if ( height == 0 ) return;

  plt_CCGetPosition(&y_poz, &x_poz);

  if( width + x_poz > FUIM_MENU_WIDTH ) {
      app_log("width + x_poz > FUIM_MENU_WIDTH -> Failed: %d + %d\r\n", width, x_poz);
      app_assert_status(SL_STATUS_FAIL); // Loop forever for debugging
  }

  if ( height + y_poz > FUIM_MENU_HEIGHT ) {
    app_log("height + y_poz > FUIM_MENU_HEIGHT Failed: %d + %d\r\n", height, y_poz );
    app_assert_status(SL_STATUS_FAIL); // Loop forever for debugging
  }

  plt_CCGetForeGndBackGndColours(&fg_color, &bg_color);

  disp_EraseImage(x_poz, y_poz, width, height, bg_color);
  plt_CCSetPosition(y_poz, (x_poz + width));
                    
}

/*=================================================================================
*   Function:    fuim_ConstructIndicatorValue
*   Description: Renders the value portion of an indicator field based on its type.
*
*   Arguments:
*   Parameter    Flow    Description
*   ------------------------------------------------------------------------------
*   field_data_ptr IN    Pointer to field structure containing type/GetFunction
*   position       IN    Indicator geometric properties (MarginTop/ValuePos)
*
*   Externals    Flow    Usage
*   ------------------------------------------------------------------------------
*   fuim_Observer()    IN  Observer dispatch returning img_storage_id_t
*   fuim_ValidityFunction() IN  Validates field visibility
*
*   @return void
*
*   @note Skips rendering if validity is FUIM_VALIDITY_PRESENT.
*       Modal/string/numeric/list types use different rendering paths.
===================================================================================*/
void fuim_ConstructIndicatorValue( fuimFieldStruct  *field_data_ptr, fuim_IndicatorProperty  *position)
{

  // @ToDo  Bookmarked for future use; analyse the requirement when `Validity` is set to
  //     FUIM_VALIDITY_SELECTABLE (field is visible and selectable)
  // or  FUIM_VALIDITY_GRAYEDOUT  (field is greyed out, not selectable)
  // or  FUIM_VALIDITY_PRESENT    (Field is present but not visible)     It was been added by UP
  if ( (fuim_ValidityFunction( field_data_ptr -> ValidityFunction)) == FUIM_VALIDITY_PRESENT) {
     return;
  }

  switch( field_data_ptr->Type )
  {

    case FUIM_FIELDTYPE_MODAL_NOTIFICATION:
    case FUIM_FIELDTYPE_STRING:
    case FUIM_FIELDTYPE_STRING_VALUE:
    {

      if (field_data_ptr->GetFunction == AUIM_NO_GET_FUNCTION) {
        return;
      }

      img_storage_id_t img_id = (img_storage_id_t) fuim_Observer (field_data_ptr->GetFunction);
      if (!(img_id < IMG_MAX_IDS_STORAGE_DESC_COUNT)) { //ID is an Invalid 
        return;//There is NOT string ID to display
      }

      fuim_ConstructStringVerticalMargin(img_id, position->MarginTop, position->ValuePos);

    } break;

    case FUIM_FIELDTYPE_NUMERIC:
    case FUIM_FIELDTYPE_NUMERIC_VALUE:
      fuim_ConstructNumeric(field_data_ptr,(position->MarginTop), FALSE);
    break;

    case FUIM_FIELDTYPE_SPACER:
    case FUIM_FIELDTYPE_SEPARATOR :
    break;

    default:
      break;
  }

}

/*=================================================================================
*   Function:    fuim_ConstructIndicatorPrompt
*   Description: Renders the prompt portion of an indicator field at the specified position.
*
*   Arguments:
*   Parameter    Flow    Description
*   ------------------------------------------------------------------------------
*   field_data_ptr IN    Pointer to field structure containing Prompt ID
*   position       IN    Indicator properties (MarginTop/PromptPos)
*
*   Externals    Flow    Usage
*   ------------------------------------------------------------------------------
*   fuim_GetPromptStringId() IN  Maps Prompt byte to img_storage_id_t
*
*   @return void
*
*   @note Skips if Prompt ID >= FMNU_NONE_PROMPT.
===================================================================================*/
void fuim_ConstructIndicatorPrompt(fuimFieldStruct *field_data_ptr, fuim_IndicatorProperty  *position )
{

  img_storage_id_t  PromptID = fuim_GetPromptStringId(field_data_ptr->Prompt);
  if ( PromptID >= FMNU_NONE_PROMPT) {
    return;
  }
  fuim_ConstructStringVerticalMargin(PromptID, position->MarginTop, position->PromptPos);
}

/*=================================================================================
*   Function:    fuim_ConstructStringVerticalMargin
*   Description: Renders an image with vertical margin, preserving and restoring cursor state.
*                Clears background above and below the image within the current row size.
*
*   Arguments:
*   Parameter    Flow    Description
*   ------------------------------------------------------------------------------
*   img_id       IN      Image storage ID to render
*   MarginTop    IN      Vertical margin above the image in pixels
*   Xpos         IN      Horizontal start position in pixels
*
*   Externals    Flow    Usage
*   ------------------------------------------------------------------------------
*   fuim_GetRowPosition()   IN  Saves original row position
*   plt_CCGetRowSize()      IN  Saves original row size
*   fuim_DrawString()       IN  Renders the image
*   fuim_DrawRepeatedCharacter() IN  Fills background padding
*
*   @return void
*
*   @note Asserts on out-of-bounds image ID or coordinates exceeding menu width/height.
===================================================================================*/
void fuim_ConstructStringVerticalMargin(img_storage_id_t img_id, Byte  MarginTop, Word Xpos )
{
  Word height;
  Word width;

  //==================================================================
  // STEP 1: Input Validation
  //==================================================================
  // 1.1 Validate image ID - prevent crashes from invalid/corrupted data
  if (!(img_id < IMG_MAX_IDS_STORAGE_DESC_COUNT)) { //ID is an Invalid
      app_log("ERR imageID: %d\r\n", img_id);
      app_assert_status(SL_STATUS_FAIL); // Loop forever for debugging
      return;// Critical error - cannot render outside display bounds
  }

  //==================================================================
  // STEP 2: Safe State Preservation
  //==================================================================
  // Since fuim_GetRowPosition() and plt_CCGetRowSize() always return valid values,
  //  can safely save the current state
  Word VertLocation = fuim_GetRowPosition();  // Always valid
  Byte RowSize = plt_CCGetRowSize();          // Always valid

  //==================================================================
  // STEP 3: Safe Dimension Retrieval with Validation
  //==================================================================

  // Retrieve dimensions safely - these macros may return 0 for invalid data
  height = IMG_GET_HEIGHT(img_id);
  width  = IMG_GET_WIDTH(img_id);

  //==================================================================
  // STEP 4: Comprehensive Row Height Validation
  //==================================================================

  // 4.1 CRITICAL: Validate that total vertical position fits within display bounds
  // Ensure (VertLocation + MarginTop + height) does not exceed FUIM_MENU_HEIGHT
  if (VertLocation + MarginTop + height > FUIM_MENU_HEIGHT) {
      app_log("ERR Ypos: %d + %d + %d > FUIM_MENU_HEIGHT\r\n", VertLocation, MarginTop, height);
      app_assert_status(SL_STATUS_FAIL); // Loop forever for debugging
      return;  // Critical error - cannot render outside display bounds
  }

  // 4.2 CRITICAL: Check if content fits at specified position
  if (Xpos + width > FUIM_MENU_WIDTH) {
      app_log("ERR Xpos: %d + %d > FUIM_MENU_WIDTH\r\n", Xpos, width);
      app_assert_status(SL_STATUS_FAIL); // Loop forever for debugging
      return;  // Critical error - cannot render outside display bounds
  }

  //==================================================================
  // STEP 5: Position and Render (Original Logic Preserved)
  //==================================================================
  // Apply vertical margin and horizontal position
  fuim_SetRowPosition(VertLocation + MarginTop);
  fuim_SetColumnPosition(Xpos);
  // Render the indicator content
  fuim_DrawString(img_id);

  //==================================================================
  // STEP 6: Simple Background Filling (Vertical Only)
  //=================================================================
  Byte Size;
  if (RowSize >= ( MarginTop + height)) {

    Size = RowSize - ( MarginTop + height );
    if(Size > 0) {
      plt_CCSetPosition((VertLocation +  MarginTop + height), ( Xpos) );
      plt_CCSetRowSize(Size);
      fuim_DrawRepeatedCharacter(width);
    }

    if (MarginTop != 0) {
      plt_CCSetPosition(VertLocation, (  Xpos) );
      plt_CCSetRowSize( MarginTop);
      fuim_DrawRepeatedCharacter(width);
    }
  }

  //==================================================================
  // STEP 7: State Restoration
  //==================================================================
  // Always restore the original cursor state
  plt_CCSetRowSize(RowSize);  //Restore Row Size
  fuim_SetRowPosition(VertLocation);//Restore Row p
}

/*=================================================================================
*   Function:    fuim_ConstructIndicatorField
*   Description: Renders a complete indicator field by clearing delta regions then drawing
*                prompt and value segments at computed positions.
*
*   Arguments:
*   Parameter    Flow    Description
*   ------------------------------------------------------------------------------
*   field_data_ptr IN    Pointer to field structure for prompt/value/colors
*   position       INOUT Indicator properties; EndPos/FieldWidth may be updated
*
*   Externals    Flow    Usage
*   ------------------------------------------------------------------------------
*   fuim_GetFieldPromptColour() IN  Source for Attribute/ForeGndColour/BackGndColour
*   fuim_ConstructIndicatorPrompt() IN  Draws prompt segment
*   fuim_ConstructIndicatorValue()   IN  Draws value segment
*
*   @return void
*
*   @note Draws from end position backward: erases old value delta, prompt-to-value gap,
*       then prompt, then value, then right padding. Uses fuim_DrawRepeatedCharacter for clearing.
===================================================================================*/
void fuim_ConstructIndicatorField( fuimFieldStruct         *field_data_ptr,
                                   fuim_IndicatorProperty  *position )
{
  Word  ForeGndColour = 0;
  Word  BackGndColour = 0;
  Word  Attribute = 0;
  Word  value_length;
  Word  value_delta = 0;


  Word VertLocation = fuim_GetRowPosition();

  /************************************************************
  *   Prepare setting from the end of field
  *************************************************************/

  value_length = fuim_GetFieldValueLength( field_data_ptr, FALSE);
  if( (position->EndPos) != (position -> ValuePos + value_length) )
  {
    if( (position->EndPos) > (position -> ValuePos + value_length) )
    {
      value_delta = (position->EndPos) - ( (position -> ValuePos) + value_length);
    }   
    position->EndPos      = position -> ValuePos + value_length ;
    position->FieldWidth  = position -> EndPos - position -> FirstPos;
  }

  if(position->FieldWidth > FUIM_MENU_WIDTH){
    app_log("Ind FieldWidth  %d Failed\r\n", position->FieldWidth);
    app_assert_status(SL_STATUS_FAIL); // Loop forever for debugging
  }

  /************************************************************
  *   The indicator is drawn starting from the END position.
  *************************************************************/

  Attribute     = fuim_GetFieldPromptColour(field_data_ptr) -> Attribute;
  ForeGndColour = fuim_GetFieldPromptColour(field_data_ptr) -> ForeGndColour;
  BackGndColour = fuim_GetFieldPromptColour(field_data_ptr) -> BackGndColour;

  plt_CCSetBackgroundColour(BackGndColour);
  plt_CCSetForegroundColour(ForeGndColour);

  fuim_SetAttributes(FUIM_ATTRIBUTES_NONE);
  plt_CCSetRowSize((Byte)(Attribute & (FUIM_ATTRIBUTES_ROW_SIZE)));



  if(value_delta != 0) //Start erase from the end position, Erase previous old value
  {
    plt_CCSetPosition(VertLocation, (position->EndPos));
    fuim_DrawRepeatedCharacter (value_delta);	
  }

  //Erase between FirstPos and PromptPos
  plt_CCSetPosition(VertLocation, (position->FirstPos));
  value_delta = ((position->PromptPos ) > (position -> FirstPos)) ? (position->PromptPos -  position -> FirstPos) : (0);
  fuim_DrawRepeatedCharacter (value_delta);

//test string
  if (fuim_GetColumnPosition() != position -> PromptPos)
  {
    app_log("Ind prompt != Position  %d Failed\r\n", position -> PromptPos);
    app_assert_status(SL_STATUS_FAIL); // Loop forever for debugging
  }


  //Draw from PromptPos
  fuim_ConstructIndicatorPrompt(field_data_ptr, position);

  value_delta = (fuim_GetColumnPosition() < (position -> ValuePos)) ? ((position -> ValuePos) - fuim_GetColumnPosition() ) : (0);
  fuim_DrawRepeatedCharacter (value_delta);

  //Draw from ValuePos
  fuim_ConstructIndicatorValue(field_data_ptr, position);

  //Erase until the end
  if (fuim_GetColumnPosition() < FUIM_MENU_WIDTH)
  {
    value_delta = (fuim_GetColumnPosition() < (position->FirstPos + position->FieldWidth)) ? ((position->FirstPos + position->FieldWidth) - fuim_GetColumnPosition()) : (0);
    fuim_DrawRepeatedCharacter(value_delta);
  }
}


/*=================================================================================
*   Function:    fuim_DestroyIndicatorField
*   Description: Clears an indicator field area using field-specific background color
*                and resets the EndPos marker.
*
*   Arguments:
*   Parameter    Flow    Description
*   ------------------------------------------------------------------------------
*   field_data_ptr IN    Pointer to field structure providing color attribute
*   position       INOUT Indicator properties; EndPos reset to 0 after erase
*
*   Externals    Flow    Usage
*   ------------------------------------------------------------------------------
*   fuim_GetFieldPromptColour() IN  Provides Attribute/BackGndColour/BackGndHighLighted
*   plt_CCSetBackgroundColour() OUT  Set before erasing
*
*   @return void
*
*   @note Modal notifications use BackGndHighLighted instead of BackGndColour.
===================================================================================*/
void fuim_DestroyIndicatorField( fuimFieldStruct  *field_data_ptr, fuim_IndicatorProperty *position)
{
  Word  BackGndColour = 0;
  Word  Attribute = 0;
//  Word  value_length;
//  Word  value_delta = 0;

  Attribute     = fuim_GetFieldPromptColour(field_data_ptr) -> Attribute;
  BackGndColour = fuim_GetFieldPromptColour(field_data_ptr) -> BackGndColour;

 if(field_data_ptr ->Type == FUIM_FIELDTYPE_MODAL_NOTIFICATION) {
   BackGndColour = fuim_GetFieldPromptColour(field_data_ptr) -> BackGndHighLighted;
 }

  plt_CCSetBackgroundColour(BackGndColour);
  fuim_SetAttributes(FUIM_ATTRIBUTES_NONE);
  plt_CCSetRowSize((Byte)(Attribute & (FUIM_ATTRIBUTES_ROW_SIZE)));

  fuim_EraseField(position->FirstPos, position->EndPos, field_data_ptr->Type);
   
  position->EndPos = 0; // - this is related to the calculation of value_delta != 0 Deleting the previous end

  return;
}

/*=================================================================================
*   Function:    fuim_ConstructIndicator
*   Description: Allocates an indicator slot, validates visibility and modal conflicts,
*                then renders the indicator field and optionally starts a timeout timer.
*
*   Arguments:
*   Parameter    Flow    Description
*   ------------------------------------------------------------------------------
*   indicator_data_ptr IN    Pointer to ROM indicator geometry/content structure
*
*   Externals    Flow    Usage
*   ------------------------------------------------------------------------------
*   fuim_Indicators[]             OUT  Slot pointer bound to indicator_data_ptr
*   fuim_indicator_properties[]   OUT  Slot geometry populated from indicator
*   indicator_timer_handle[]      OUT  Timer handle created if Timeout > 0
*   fuim_IsModalIndicatorActive() IN   Blocks creation if another modal is active
*   fuim_ValidityFunction()       IN   Rejects NOTPRESENT fields
*
*   @return 1-based handle (1..FUIM_MAX_INDICATORS), or 0 on failure
*
*   @note Modal indicators block additional modal creation.
*       Returns 0 if no free slot or validity == NOTPRESENT.
===================================================================================*/
osdDialogHandle fuim_ConstructIndicator(fuimIndicatorStruct *indicator_data_ptr)
{
  osdDialogHandle          handle;
  fuimFieldStruct        * field_data_ptr;
  fuim_IndicatorProperty * properties;
  fuim_Validity            validity;

  handle = 0 ;

  while (handle < FUIM_MAX_INDICATORS && fuim_Indicators[handle] != NULL) {
    handle ++;
  }

  if( handle == FUIM_MAX_INDICATORS ) {
    return  (osdDialogHandle)0;
  } else {
    field_data_ptr = (fuimFieldStruct *)indicator_data_ptr->Field;
    validity = fuim_ValidityFunction(field_data_ptr-> ValidityFunction);
    if(validity == FUIM_VALIDITY_NOTPRESENT) {
      return  (osdDialogHandle)0;
    } else {
       // NEW: Check for existing modal indicators - block if modal already active
       if (field_data_ptr->Type == FUIM_FIELDTYPE_MODAL_NOTIFICATION) {
         if (fuim_IsModalIndicatorActive()) {
           return (osdDialogHandle)0;  // Refuse to create - modal already exists
         }
       }

      fuim_Indicators[ handle ] = indicator_data_ptr ;
    }
  }

  properties = &fuim_indicator_properties [handle];

  properties->FirstPos  = fuim_GetIndicatorHorLocation(indicator_data_ptr);
  properties->PromptPos = fuim_GetIndicatorPromptPos(indicator_data_ptr);
  properties->ValuePos  = fuim_GetIndicatorValuePos(indicator_data_ptr);
  properties->MarginTop = fuim_GetIndicatorVertMaginTop(indicator_data_ptr);

  /*  Set Row Position in pixels   */

  fuim_SetRowPosition (fuim_GetIndicatorVertLocation(indicator_data_ptr));
  fuim_ConstructIndicatorField (field_data_ptr, properties);

  if (fuim_GetIndicatorTimeout(indicator_data_ptr) > 0) {
    indicator_timer_handle [handle] = fuim_ConstructTimer(fuim_GetIndicatorTimeout(indicator_data_ptr), INDICATOR_TIMER_FUNCTION, handle + 1);
  } else {
    indicator_timer_handle [handle] = FUIM_NO_FREE_TIMER_HANDLE;
  }

  return handle + 1;
}

/*=================================================================================
*   Function:    fuim_DestroyIndicator
*   Description: Removes an active indicator by erasing its field, freeing its slot,
*                destroying its timer, and triggering menu redraw if it was modal.
*
*   Arguments:
*   Parameter    Flow    Description
*   ------------------------------------------------------------------------------
*   indicator     IN      1-based handle of the indicator to destroy
*
*   Externals    Flow    Usage
*   ------------------------------------------------------------------------------
*   fuim_Indicators[]            OUT  Slot cleared to NULL
*   indicator_timer_handle[]     OUT  Timer destroyed if valid
*   fuim_DestroyIndicatorField() IN   Erases the display area
*   fmnu_ReDrawActiveFields()    IN   Called only if destroyed indicator was modal
*
*   @return void
*
*   @note Handle is 1-based; internally decremented before array access.
===================================================================================*/
void fuim_DestroyIndicator(osdDialogHandle indicator)
{
  fuimIndicatorStruct *indicator_data_ptr;
  fuimFieldStruct     *field_data_ptr;

  Word  row, col;

  if (indicator == 0 || indicator > FUIM_MAX_INDICATORS) {
    return ;
  }

  indicator --;

  indicator_data_ptr = fuim_Indicators[ indicator ] ;
  field_data_ptr = (fuimFieldStruct *)indicator_data_ptr->Field;

  row = fuim_GetIndicatorVertLocation(indicator_data_ptr);
  col = fuim_GetIndicatorHorLocation (indicator_data_ptr);
  plt_CCSetPosition(row, col);

  fuim_DestroyIndicatorField(field_data_ptr, (fuim_IndicatorProperty *)&fuim_indicator_properties[ indicator ] );

  // NEW: If modal indicator was destroyed, redraw the hidden menu field
  if (field_data_ptr->Type == FUIM_FIELDTYPE_MODAL_NOTIFICATION ) {
      fmnu_ReDrawActiveFields();
  }

  fuim_Indicators [indicator] = NULL;

  if (indicator_timer_handle [indicator] != FUIM_NO_FREE_TIMER_HANDLE) {
    fuim_DestroyTimer (&indicator_timer_handle [indicator]);
  }


}

/*=================================================================================
*   Function:    fuim_UpdateIndicator
*   Description: Refreshes an indicator display by reconstructing its field and optionally
*                restarting its timeout timer.
*
*   Arguments:
*   Parameter    Flow    Description
*   ------------------------------------------------------------------------------
*   handle        IN   1-based indicator handle
*   restart_timer IN   TRUE to force field redraw and timer restart
*
*   Externals    Flow    Usage
*   ------------------------------------------------------------------------------
*   fuim_Indicators[]            IN  Slot pointer for indicator data
*   fuim_indicator_properties[]  IN  Cached position/width for the slot
*   indicator_timer_handle[]     INOUT Restarted if valid and restart_timer==TRUE
*   fuim_Timer[]                 IN  Periodic timer state checked before restart
*
*   @return void
*
*   @note Skips update if handle==0, >FUIM_MAX_INDICATORS, or slot is empty.
*       Restart is conditional on field TimeOut != FUIM_FIELD_NO_TIMEOUT when !restart_timer.
===================================================================================*/
void fuim_UpdateIndicator (osdDialogHandle handle, Bool restart_timer)
{
  fuimIndicatorStruct   *indicator_data_ptr;
  fuimFieldStruct       *field_data_ptr;
//  fuim_IndicatorProperty *position;

  if( (handle == 0) || (handle > FUIM_MAX_INDICATORS) || (fuim_Indicators[ handle-1 ] == NULL) ) {
    return ;
  }

  handle --;

  indicator_data_ptr = fuim_Indicators [handle];
  field_data_ptr = (fuimFieldStruct *)indicator_data_ptr->Field;

//position = &fuim_indicator_properties[handle];

  fuim_SetRowPosition( fuim_GetIndicatorVertLocation (indicator_data_ptr) );

  if( restart_timer ) {
    fuim_ConstructIndicatorField( field_data_ptr, &fuim_indicator_properties[handle] );
  } else {
    if(field_data_ptr ->TimeOut != FUIM_FIELD_NO_TIMEOUT) {
      fuim_ConstructIndicatorField( field_data_ptr, &fuim_indicator_properties[handle] );
    }
  }

  if ( restart_timer ) {
    if ( fuim_Timer[FUIM_PERIODIC_TIMER].TimeOut != RGEN_TIMER_STOPPED ) {
      if ( indicator_timer_handle[handle] != FUIM_NO_FREE_TIMER_HANDLE ) {
        fuim_RestartTimer( indicator_timer_handle[handle], fuim_GetIndicatorTimeout(indicator_data_ptr) );
      }
    }
  }
}

/*=================================================================================
*   Function:    fuim_ConstructTimer
*   Description: Allocates a runtime timer slot and initializes it with timeout, ID, and dialog handle.
*
*   Arguments:
*   Parameter    Flow    Description
*   ------------------------------------------------------------------------------
*   TimeoutSeconds IN    Timer timeout in seconds
*   TimerID        IN    Timer function ID (PERIODIC/INDICATOR/MENU_TIMER_FUNCTION)
*   hDialog        IN    Dialog handle parameter passed to callback
*
*   Externals    Flow    Usage
*   ------------------------------------------------------------------------------
*   fuim_Timer[]   OUT  Fresh slot initialized with TimeOut/TimerID/Parameter
*
*   @return 1-based timer handle, or FUIM_NO_FREE_TIMER_HANDLE if full
*
*   @note First slot (index 0) is reserved for the periodic timer.
===================================================================================*/
osdTimerHandle fuim_ConstructTimer(Byte TimeoutSeconds, Byte TimerID, osdDialogHandle hDialog)
{
  osdTimerHandle i, hTimer;

  hTimer = FUIM_NO_FREE_TIMER_HANDLE;
  for (i = FUIM_PERIODIC_TIMER + 1; i < FUIM_MAX_TIMERS; i++) {
      if (fuim_Timer[i].TimerID == EMPTY_TIMER) {
          fuim_Timer[i].TimeOut   = TimeoutSeconds * FUIM_TIMER_RESOLUTION;
          fuim_Timer[i].TimerID   = TimerID;
          fuim_Timer[i].Parameter = hDialog;
          hTimer = i + 1;
          break;
      }
  }
  return hTimer;
}
/*=================================================================================
*   Function:    fuim_DestroyTimer
*   Description: Frees a runtime timer slot and clears its handle marker.
*
*   Arguments:
*   Parameter    Flow    Description
*   ------------------------------------------------------------------------------
*   hTimer        INOUT Pointer to 1-based timer handle; reset to FUIM_NO_FREE_TIMER_HANDLE
*
*   Externals    Flow    Usage
*   ------------------------------------------------------------------------------
*   fuim_Timer[]   OUT  Slot cleared to TimeOut=RGEN_TIMER_STOPPED, TimerID=EMPTY_TIMER, Parameter=0
*
*   @return void
*
*   @note Dereferences handle with -1 offset to access 0-based array.
===================================================================================*/
void fuim_DestroyTimer(osdTimerHandle *hTimer )
{
  fuim_Timer[*hTimer - 1].TimeOut   = RGEN_TIMER_STOPPED;
  fuim_Timer[*hTimer - 1].TimerID   = EMPTY_TIMER;
  fuim_Timer[*hTimer - 1].Parameter = 0;
  *hTimer = FUIM_NO_FREE_TIMER_HANDLE;
}

/*=================================================================================
*   Function:    fuim_RestartTimer
*   Description: Restarts an active runtime timer with a new timeout value.
*
*   Arguments:
*   Parameter    Flow    Description
*   ------------------------------------------------------------------------------
*   hTimer          IN    1-based timer handle
*   TimeoutInSeconds IN   New timeout in seconds
*
*   Externals    Flow    Usage
*   ------------------------------------------------------------------------------
*   fuim_Timer[] INOUT TimeOut updated with TimeoutInSeconds * FUIM_TIMER_RESOLUTION
*
*   @return void
*
*   @note Only updates if timer slot is still active (TimeOut > RGEN_TIMER_STOPPED).
===================================================================================*/
void fuim_RestartTimer (osdTimerHandle hTimer,  Byte TimeoutInSeconds )
{
  if (fuim_Timer[hTimer - 1].TimeOut > RGEN_TIMER_STOPPED) {
      fuim_Timer[hTimer - 1].TimeOut = TimeoutInSeconds * FUIM_TIMER_RESOLUTION;
  }
}

/*=================================================================================
*   Function:    fuim_UpdateTimer064ms
*   Description: Ticks all runtime timer slots by one resolution step.
*
*   Arguments:        None
*
*   Externals    Flow    Usage
*   ------------------------------------------------------------------------------
*   fuim_Timer[] IN  Each slot's TimeOut is decremented by rbsc_UpdateTimer()
*
*   @return void
*
*   @note Called from the 64 ms periodic sleeptimer callback.
===================================================================================*/
void fuim_UpdateTimer064ms(void)
{
  Byte i;
  for( i = 0; i < FUIM_MAX_TIMERS; i++) {
    rbsc_UpdateTimer(&fuim_Timer[i].TimeOut);
  }
}

/*=================================================================================
*   Function:    fuim_DestroyIndicatorTimer
*   Description: Destroys the timeout timer associated with an indicator slot.
*
*   Arguments:
*   Parameter    Flow    Description
*   ------------------------------------------------------------------------------
*   indicator     IN   1-based indicator handle
*
*   Externals    Flow    Usage
*   ------------------------------------------------------------------------------
*   indicator_timer_handle[]   INOUT Read to find handle; reset to FUIM_NO_FREE_TIMER_HANDLE
*
*   @return void
*
*   @note Returns immediately if indicator handle is out of range or has no timer.
===================================================================================*/
void fuim_DestroyIndicatorTimer(osdDialogHandle indicator )
{
  if ( (indicator == 0 ) || ( indicator > FUIM_MAX_INDICATORS ) ) {
    return ;
  }
  indicator --;
  if (indicator_timer_handle [indicator] != FUIM_NO_FREE_TIMER_HANDLE) {
    fuim_DestroyTimer (&indicator_timer_handle [indicator]);
  }
} 

/*=================================================================================
*   Function:    fuim_PeriodicTimerExpired
*   Description: Periodic 64 ms tick handler that updates the menu and all active
*                indicators, unless the system is in the error state.
*
*   Arguments:        None
*
*   Externals    Flow    Usage
*   ------------------------------------------------------------------------------
*   auph_GetState()    IN  Skips indicator updates if AU_ERROR_STATE
*   fuim_Timer[]       IN  Checked for expiry via fuim_UpdateTimers()
*
*   @return void
*
*   @note Called from fuim_timer_callback on every 64 ms timer tick.
===================================================================================*/
void fuim_PeriodicTimerExpired(void)
{
  fmnu_UpdateMenu();
  if(auph_GetState() != AU_ERROR_STATE) {
    for (Byte i = 1; i <= FUIM_MAX_INDICATORS; i++) {
        fuim_UpdateIndicator(i, FALSE);
    }
  }
}

/*=================================================================================
*   Function:    fuim_TimerFunction
*   Description: Dispatches expired timer events to their configured handlers.
*
*   Arguments:
*   Parameter    Flow    Description
*   ------------------------------------------------------------------------------
*   index    IN    Timer function ID (PERIODIC/INDICATOR/MENU_TIMER_FUNCTION)
*   hDialog  IN    Dialog handle argument for timer handler
*
*   Externals    Flow    Usage
*   ------------------------------------------------------------------------------
*   fuim_Timer[]  IN  Timer slot data including TimerID
*
*   @return void
*
*   @note MENU_TIMER_FUNCTION transitions UI to AU_IDLE STATE and posts AU_IDLE_MENU_START.
===================================================================================*/
void fuim_TimerFunction( Byte index,  osdDialogHandle hDialog )
{
  switch (index)
  {
    case EMPTY_TIMER:
      break;

    case PERIODIC_TIMER_FUNCTION:
      fuim_PeriodicTimerExpired();
    break;

    case INDICATOR_TIMER_FUNCTION:
      fuim_DestroyIndicator(hDialog);
    break;

    case MENU_TIMER_FUNCTION: {
      fmnu_RemoveCurrentMenu();
      aukh_Post_UI_Event(AU_IDLE_MENU_START);
      auph_SetState(AU_IDLE_STATE);

    } break;

    default:
      break;
  }

}

/*=================================================================================
*   Function:    fuim_UpdateTimers
*   Description: Iterates all runtime timers, dispatches expired ones, and resets or
*                stops them based on their type.
*
*   Arguments:        None
*
*   Externals    Flow    Usage
*   ------------------------------------------------------------------------------
*   fuim_Timer[]    INOUT Iterated; expired slots dispatched and reset/stopped
*
*   @return void
*
*   @note Periodic timer (index 0) is restarted with FUIM_PERIODIC_TIMEOUT after dispatch.
*       Other timers are stopped and cleared after dispatch.
===================================================================================*/
void fuim_UpdateTimers(void)
{
  osdTimerHandle  i;

  for (i = 0; i < FUIM_MAX_TIMERS; i++) {
    if (fuim_Timer[i].TimeOut == RGEN_TIMER_EXPIRED) {

      fuim_TimerFunction(fuim_Timer[i].TimerID, fuim_Timer[i].Parameter);

      if (i == FUIM_PERIODIC_TIMER) {
         fuim_Timer[FUIM_PERIODIC_TIMER].TimeOut = FUIM_PERIODIC_TIMEOUT;
      } else {
        fuim_Timer[i].TimeOut = RGEN_TIMER_STOPPED;
        fuim_Timer[i].TimerID = EMPTY_TIMER;
      }
    }
  }
  return;
}

/*=================================================================================
*   Function:    fuim_Init
*   Description: Initializes the UI framework by configuring character-cell dimensions,
*                resetting menus/indicators/timers, and starting the 64 ms periodic timer.
*
*   Arguments:        None
*
*   Externals    Flow    Usage
*   ------------------------------------------------------------------------------
*   plt_CCInit()           IN  Sets FUIM_MAX_NR_OF_COLS/ROWS
*   fuim_timeout_timer_handle OUT Periodic sleeptimer handle started once
*
*   @return void
*
*   @note Periodic timer is started only if not already running.
===================================================================================*/
void fuim_Init(void)
{
  bool is_timer64ms_running = false;

  plt_CCInit( FUIM_MAX_NR_OF_COLS, FUIM_MAX_NR_OF_ROWS);
  fuim_InitTimers();
  fmnu_InitMenus();
  fuim_InitIndicators();

  /* Make sure the fuim_timeout_timer_handle timer is initialized only once */
  sl_sleeptimer_is_timer_running(&fuim_timeout_timer_handle,
                                 &is_timer64ms_running);
  if (is_timer64ms_running == false) {
  /* Start a periodic timer 64ms to generate  timing */
    sl_sleeptimer_start_periodic_timer_ms(&fuim_timeout_timer_handle,
                                          FUIM_PERIODIC_TIMER_DELAY_64MS,
                                          fuim_timer_callback,
                                          (void *)NULL,
                                          0,
                                          0);
  }

}

/*=================================================================================
*   Function:    fuim_TurnOn
*   Description: Reinitializes UI runtime state (indicators, timers, menus) and sets
*                the periodic timer for menu lifecycle management.
*
*   Arguments:        None
*
*   Externals    Flow    Usage
*   ------------------------------------------------------------------------------
*   fuim_InitIndicators()  IN  Clears all indicator slots
*   fuim_InitTimers()      IN  Clears all timer slots
*   fuim_Timer[]           IN  Periodic timer configured
*
*   @return void
*
*   @note Calls SetupPeriodicTimer(FUIM_PERIODIC_TIMEOUT).
===================================================================================*/
void fuim_TurnOn(void)
{
  fuim_InitIndicators();
  fuim_InitTimers();
  fmnu_InitMenus();

  SetupPeriodicTimer(FUIM_PERIODIC_TIMEOUT);
}

/*=================================================================================
*   Function:    fuim_TurnOff
*   Description: Deinitializes UI runtime state by resetting timers.
*
*   Arguments:        None
*
*   Externals    Flow    Usage
*   ------------------------------------------------------------------------------
*   fuim_InitTimers() IN  Clears all timer slots
*
*   @return void
*
*   @note Only resets timers; indicators and menus are not touched here.
===================================================================================*/
void fuim_TurnOff(void)
{
  fuim_InitTimers();
}

/*=================================================================================
*   Function:    fuim_Update
*   Description: Main UI update entry point called from the application loop.
*
*   Arguments:        None
*
*   Externals    Flow    Usage
*   ------------------------------------------------------------------------------
*   fuim_UpdateTimers() IN  Processes expired runtime timers
*
*   @return void
*
*   @note Delegates to fuim_UpdateTimers() for timer dispatch.
===================================================================================*/
void fuim_Update(void)
{
    fuim_UpdateTimers();
}

/*=================================================================================
*   Function:    fuim_GetColumnPosition
*   Description: Returns the current character-cell column position.
*
*   Arguments:        None
*
*   Externals    Flow    Usage
*   ------------------------------------------------------------------------------
*   plt_CCGetPosition() IN  Reads current cursor position
*
*   @return Current column position as Word
*
*   @note Reads from plt_CC* layer; no modification.
===================================================================================*/
Word fuim_GetColumnPosition(void)
{
  Word posd_CCRow, posd_CCColumn;
  plt_CCGetPosition(&posd_CCRow, &posd_CCColumn);
  return ( posd_CCColumn );
}

/*=================================================================================
*   Function:    fuim_GetRowPosition
*   Description: Returns the current character-cell row position.
*
*   Arguments:        None
*
*   Externals    Flow    Usage
*   ------------------------------------------------------------------------------
*   plt_CCGetPosition() IN  Reads current cursor position
*
*   @return Current row position as Word
*
*   @note Reads from plt_CC* layer; no modification.
===================================================================================*/
Word fuim_GetRowPosition(void)
{
  Word  posd_CCRow, posd_CCColumn;
  plt_CCGetPosition(&posd_CCRow, &posd_CCColumn);
  return ( posd_CCRow );
}

/*=================================================================================
*   Function:    fuim_SetRowPosition
*   Description: Sets the current character-cell row while preserving the current column.
*
*   Arguments:
*   Parameter    Flow    Description
*   ------------------------------------------------------------------------------
*   row    IN    Target row position
*
*   Externals    Flow    Usage
*   ------------------------------------------------------------------------------
*   plt_CCSetPosition() OUT  Updated with new row and current column
*   fuim_GetColumnPosition() IN  Preserved column value
*
*   @return void
===================================================================================*/
void fuim_SetRowPosition(Word row)
{
  plt_CCSetPosition( row, fuim_GetColumnPosition() );
}

/*=================================================================================
*   Function:    fuim_SetColumnPosition
*   Description: Sets the current character-cell column while preserving the current row.
*
*   Arguments:
*   Parameter    Flow    Description
*   ------------------------------------------------------------------------------
*   column    IN    Target column position
*
*   Externals    Flow    Usage
*   ------------------------------------------------------------------------------
*   plt_CCSetPosition() OUT  Updated with new column and current row
*   fuim_GetRowPosition() IN  Preserved row value
*
*   @return void
===================================================================================*/
void fuim_SetColumnPosition(Word column)
{
  plt_CCSetPosition(fuim_GetRowPosition(), column);
}

/*=================================================================================
*   Function:    fuim_SetAttributes
*   Description: Sets character-cell rendering attributes (no-op placeholder).
*
*   Arguments:
*   Parameter    Flow    Description
*   ------------------------------------------------------------------------------
*   Attributes    IN    Attribute bitmask (currently unused)
*
*   Externals    Flow    Usage
*   ------------------------------------------------------------------------------
*   None
*
*   @return void
*
*   @note Currently a no-op; attributes are passed through to plt_CC* but not used.
===================================================================================*/
void fuim_SetAttributes(Word Attributes)
{
  if (Attributes == FUIM_ATTRIBUTES_NONE)
  {
    Attributes = Attributes;
  }

}

/*=================================================================================
*   Function:    fuim_SetIndicatorTimeOut
*   Description: Sets or restarts the timeout timer for an indicator slot.
*
*   Arguments:
*   Parameter    Flow    Description
*   ------------------------------------------------------------------------------
*   hDialog           IN   1-based indicator dialog handle
*   TimeOutInSeconds  IN   Timeout in seconds
*
*   Externals    Flow    Usage
*   ------------------------------------------------------------------------------
*   indicator_timer_handle[] INOUT Reassigned with new timer handle if previously empty
*
*   @return void
*
*   @note Only sets timer if handle was FUIM_NO_FREE_TIMER_HANDLE.
===================================================================================*/
void fuim_SetIndicatorTimeOut (osdDialogHandle hDialog,  Byte TimeOutInSeconds)
{
  hDialog --;
  if ( indicator_timer_handle [hDialog] == FUIM_NO_FREE_TIMER_HANDLE ) {
    indicator_timer_handle [hDialog] = fuim_ConstructTimer (TimeOutInSeconds, INDICATOR_TIMER_FUNCTION, hDialog + 1);
  }
}

/*=================================================================================
*   Function:    fuim_SetNextRow
*   Description: Advances the character-cell cursor to the start of the next row using
*                the current row size.
*
*   Arguments:        None
*
*   Externals    Flow    Usage
*   ------------------------------------------------------------------------------
*   plt_CCGetRowSize()      IN  Current row height
*   plt_CCSetPosition()     OUT New row/column position
*
*   @return void
===================================================================================*/
void fuim_SetNextRow(void)
{
  plt_CCSetPosition( fuim_GetRowPosition() + plt_CCGetRowSize(), fuim_GetColumnPosition() );
}

/*=================================================================================
*   Function:    fuim_DynamicColours
*   Description: Returns a pointer to the colour structure for the requested UI context.
*
*   Arguments:
*   Parameter    Flow    Description
*   ------------------------------------------------------------------------------
*   index    IN    Colour table selector (AUIM_*_COLOUR enum)
*
*   Externals    Flow    Usage
*   ------------------------------------------------------------------------------
*   MenuTitleColor, MenuNormalFieldColour, ... IN  Static const colour tables
*
*   @return Pointer to matching fuimColourStruct, or IndicatorPromptColour on default
*
*   @note Used throughout fmnu.c/fuim.c to obtain foreground/background/row-size settings.
===================================================================================*/
fuimColourStruct  *fuim_DynamicColours ( Byte index )
{
  switch (index)
  {
    case AUIM_MENU_TITLE_COLOUR:
        return ((fuimColourStruct *)&MenuTitleColor);
    break;

    case AUIM_MENU_FIELD_COLOUR:
        return ((fuimColourStruct *)&MenuNormalFieldColour);
    break;

    case AUIM_MENU_SPLIT_FIELD_COLOUR:
        return ((fuimColourStruct *)&MenuSplitFieldColour);
    break;

    case AUIM_MENU_DOUBLE_FIELD_COLOUR:
        return ((fuimColourStruct *)&MenuDoubleFieldColour);
    break;

    case AUIM_BUTTON_FIELD_COLOUR:
        return ((fuimColourStruct *)&ButtonPromptColour);
    break;

    case AUIM_INDICATOR_COLOUR:
        return ((fuimColourStruct *)&IndicatorPromptColour);
    break;

    case AUIM_BLANK_INDICATOR_COLOUR:
        return ((fuimColourStruct *)&IndicatorPromptColour);
    break;

    case AUIM_MODAL_INDICATOR_COLOUR:
        return ((fuimColourStruct *)&IndicatorModalPromptColour);
    break;

    case AUIM_SPLASH_SCREEN_COLOUR:
        return ((fuimColourStruct *)&IndicatorSplashScreenColour);
    break;

    case AUIM_PERFORMANCE_NOTIFICATION_COLOUR :
        return ((fuimColourStruct *)&MenuPerformanceNotoficationColour);
    break;

    case AUIM_PATIENT_NOTIFICATION_COLOUR :
        return ((fuimColourStruct *)&MenuPatientNotificationColour);
    break;

    case AUIM_REFERENCE_NOTIFICATION_COLOUR :
        return ((fuimColourStruct *)&MenuReferenceNotificationColour);
    break;

    case AUIM_TIP_ID_RED_MODAL_COLOUR :
        return ((fuimColourStruct *)&IndicatorRedModalPromptColour);
    break;

    case AUIM_TIP_ID_GREEN_MODAL_COLOUR :
        return ((fuimColourStruct *)&IndicatorGreenModalPromptColour);
    break;

    case AUIM_ERROR_INDICATOR_COLOUR :
        return ((fuimColourStruct *)&IndicatorErrorColour);
    break;




    default:
        return ((fuimColourStruct *)&IndicatorPromptColour);
    break;

  }

}

/*=================================================================================
*   Function:    fuim_DrawNumeric
*   Description: Formats a numeric observer value into a string and renders it using
*                font asset IDs with alignment and optional prefix/suffix support.
*
*   Arguments:
*   Parameter    Flow    Description
*   ------------------------------------------------------------------------------
*   GetFunction        IN      Observer ID for the numeric value
*   FieldScalingNumeric IN     Number of decimal places
*   FieldSizeNumeric    IN     Maximum field width in characters
*   size                IN     Font size enum
*   color               IN     Font color index
*   align               IN     Alignment enum
*   margin_top          IN     Vertical margin in pixels
*
*   Externals    Flow    Usage
*   ------------------------------------------------------------------------------
*   fuim_Observer()         IN  Reads numeric value
*   font_lut[]              IN  Maps char+size+color to asset IDs
*   fuim_DigitToFontAssetId() IN  Character-to-asset mapping
*
*   @return void
*
*   @note Value is scaled by 10^FieldScalingNumeric. Truncates if output exceeds FieldSizeNumeric.
*       Clears padding before text via fuim_DrawRepeatedCharacter().
===================================================================================*/
void fuim_DrawNumeric( osdFieldValue GetFunction,
                      Byte FieldScalingNumeric,
                      Byte FieldSizeNumeric,
                      fuim_FontSize size,
                      fuim_FontColor color,
                      fuim_Alignment align,
                      Byte margin_top)
{

     Byte  i = 0;
     Word  divider = 0;
     Word  char_width = (size == FUIM_FONT_SIZE_LARGE) ? FUIM_DIGITS_CHAR_WIDTH_LARGE : FUIM_DIGITS_CHAR_WIDTH_SMALL;

     float value = fuim_Observer(GetFunction);

     if (FieldScalingNumeric != 0) {
        divider = 1;
        for (i = 0; i < FieldScalingNumeric; i++) {
            divider *= 10;
        }
        value = value / ((float)(divider));
      }

      /* 1. Safe string formatting.
           * %.*f uses the first argument for precision and the second for the value.
       */
      /* Buffer size: digits + optional minus + decimal point + null terminator */
      char str_buf[FUIM_MAX_NUMERIC_LENGTH + 1];

      int printed = snprintf(str_buf, sizeof(str_buf), "%.*f", (int)FieldScalingNumeric, value);

      if (printed < 0) {
        return;
      }
      /* Safety check: if the string exceeds the field size, truncate it. */
      Byte actual_len = (Byte)printed;
      if (actual_len > FieldSizeNumeric) {
          actual_len = FieldSizeNumeric;
          str_buf[actual_len] = '\0';
      }

      /* 2. Alignment calculation.
           * Calculate the pixel offset within a box of 'FieldSizeNumeric' width.
           */
      Word total_field_px = FieldSizeNumeric * char_width;
      Word text_width_px  = actual_len * char_width;
      Word current_x = fuim_GetColumnPosition();
      Word last_current_x = current_x;
      if (total_field_px > text_width_px) {
          switch (align) {
              case FUIM_ALIGNMENT_RIGHT:
                  current_x += (total_field_px - text_width_px);
                  break;
              case FUIM_ALIGNMENT_CENTRE:
                  current_x += (total_field_px - text_width_px) / 2;
                  break;
              case FUIM_ALIGNMENT_LEFT:
              default:
                /* current_x remains at start x */
                  break;
          }
      }
      Word Count = (current_x > last_current_x) ? (current_x - last_current_x) : (0);
      fuim_SetColumnPosition(last_current_x);
      fuim_DrawRepeatedCharacter(Count);

      /* 3. Character rendering loop via Asset ID system. */
      for (Byte i = 0; str_buf[i] != '\0'; i++) {
          /* Retrieve the specific Asset ID for the character. */
          Word asset_id = fuim_DigitToFontAssetId(str_buf[i], size, color);

          //fuim_SetColumnPosition(current_x);
          /* Render if the symbol (digit, dot, or minus) exists in the table. */
          if (asset_id != IMG_INVALID_ID) {
              //fuim_DrawString(asset_id);
              fuim_ConstructStringVerticalMargin(asset_id, margin_top, current_x);
          }

          /* Shift the cursor by one character width. */
          current_x += char_width;
      }

   return;
}

/*=================================================================================
*   Function:    fuim_ConstructNumeric
*   Description: Renders a numeric field including optional prefix/suffix by delegating
*                to fuim_DrawNumeric() with field-specific font and alignment settings.
*
*   Arguments:
*   Parameter    Flow    Description
*   ------------------------------------------------------------------------------
*   field_data_ptr IN    Pointer to field structure with numeric settings
*   margin_top    IN    Vertical margin in pixels
*   Highlighted   IN    Highlight flag (currently unused)
*
*   Externals    Flow    Usage
*   ------------------------------------------------------------------------------
*   fmnu_str_Prefix[] IN  Observer-mapped prefix image IDs
*   fmnu_str_Suffix[] IN  Observer-mapped suffix image IDs
*
*   @return void
*
*   @note Highlighted parameter is reserved for future use.
===================================================================================*/
void fuim_ConstructNumeric(fuimFieldStruct  *field_data_ptr, Byte margin_top, Bool Highlighted )
{

  if (field_data_ptr->GetFunction == AUIM_NO_GET_FUNCTION) {
    return;
  }

  Byte Prefix, Suffix;

  Prefix = field_data_ptr->Prefix;
  Suffix = field_data_ptr->Suffix;

  if(Prefix != FMNU_PREFIX_NONE ) {
    fuim_ConstructStringVerticalMargin((img_storage_id_t)fmnu_str_Prefix[Prefix], margin_top, fuim_GetColumnPosition());
  }

 fuim_DrawNumeric( field_data_ptr->GetFunction,
                   field_data_ptr->FieldScaling.Numeric,
                   field_data_ptr->FieldSize.Numeric,
                   field_data_ptr->FieldCharacters.NumericFont.size,
                   field_data_ptr->FieldCharacters.NumericFont.color,
                   field_data_ptr->Alignment,
                   margin_top
                 );

  if( Suffix != FMNU_SUFFIX_NONE ) {
       fuim_ConstructStringVerticalMargin((img_storage_id_t)fmnu_str_Suffix[Suffix], margin_top, fuim_GetColumnPosition());
  }

  (void)Highlighted;
}

/*=================================================================================
*   Function:    fuim_GetFieldValueLength
*   Description: Calculates the pixel width of a field's value or prompt based on its
*                type, observer value, and font settings.
*
*   Arguments:
*   Parameter    Flow    Description
*   ------------------------------------------------------------------------------
*   field_data_ptr IN    Pointer to field structure
*   isPrompt       IN    TRUE to measure prompt width, FALSE for value width
*
*   Externals    Flow    Usage
*   ------------------------------------------------------------------------------
*   fuim_ValidityFunction()    IN  Skips hidden fields
*   fuim_Observer()            IN  Provides dynamic content ID/value
*   fuim_GetPromptStringId()   IN  Maps prompt index to image ID
*   fmnu_GetListStringLen()    IN  Used for LIST/ONOFFLIST types
*
*   @return Pixel width of the content, or 0 if not present/hidden
*
*   @note Returns 0 for SPACER/SEPARATOR and unknown field types.
===================================================================================*/
Word fuim_GetFieldValueLength(const fuimFieldStruct *field_data_ptr, Bool isPrompt)
{
    /* NULL pointer check protection against dereferencing a null pointer */
    if (field_data_ptr == NULL) {
        return 0;
    }

    Word length = 0;

    /*========================================================================*
     * PROMPT HANDLING (isPrompt = TRUE)
     *========================================================================*/
    if (isPrompt == TRUE) {
        img_storage_id_t prompt = fuim_GetPromptStringId(field_data_ptr->Prompt);

        /* Check if there is a prompt */
        if (prompt < FMNU_NONE_PROMPT) {
            /* Get the prompt image width */
            length = IMG_GET_WIDTH(prompt);
        }
        return length;
    }

    /*========================================================================*
     * VALUE HANDLING (isPrompt = FALSE)
     *========================================================================*/
    /* Get the field value via the observer function */
    float value = 0;
    void *void_ptr = NULL;
    osdFieldValue osd_Field_Value = IMG_INVALID_ID;
    if(field_data_ptr->GetFunction != AUIM_NO_GET_FUNCTION) {
        osd_Field_Value = fuim_Observer(field_data_ptr->GetFunction);
        void_ptr = (void *)(osd_Field_Value);
        value    = (float)(osd_Field_Value);
    }

    switch (field_data_ptr->Type)
    {
        /*====================================================================*
         * GROUP 1: String field types
         * COMBINED: STRING, STRING_VALUE, STRING_ID,
         *           MODAL_NOTIFICATION
         * All these types use IMG_GET_WIDTH() to obtain the width
         *====================================================================*/
        case FUIM_FIELDTYPE_STRING:
        case FUIM_FIELDTYPE_STRING_VALUE:
        case FUIM_FIELDTYPE_MODAL_NOTIFICATION:
        {
            img_storage_id_t img_id = (img_storage_id_t)osd_Field_Value;
            if (img_id < IMG_MAX_IDS_STORAGE_DESC_COUNT) {
                length = IMG_GET_WIDTH(img_id);
            }
         }break;
         /*====================================================================*
          * GROUP 1-1: List types
          * COMBINED: FUIM_FIELDTYPE_STRING_NUMERIC_VALUE
          * Both types use strlen() to obtain the length
          *====================================================================*/
        case FUIM_FIELDTYPE_STRING_NUMERIC_VALUE:
        {
          img_storage_id_t img_id;
          char *string_ptr = (char *)void_ptr;

          length = 0;

          if (string_ptr != NULL) {
            /* Get the number of digits */
            Byte field_size = field_data_ptr->FieldSize.Numeric;

            /* Determine character width based on font size */
            Word char_width = FUIM_DIGITS_CHAR_WIDTH_SMALL;  /* Default small font */

            /* Check font size in the FieldCharacters.NumericFont structure */
            if (field_data_ptr->FieldCharacters.NumericFont.size == FUIM_FONT_SIZE_LARGE) {
                char_width = FUIM_DIGITS_CHAR_WIDTH_LARGE;
            }

            Byte actual_len = strlen(string_ptr);
            if (actual_len > 0) {
              /* Safety check: if the string exceeds the field size, truncate it. */
              if (actual_len > field_size) {
                actual_len = field_size;
              }

              /* Calculate the integer part width */
              length = actual_len * char_width;
             }

             if ((field_data_ptr->Prefix) != FMNU_PREFIX_NONE) {  
               img_id = (img_storage_id_t)fmnu_str_Prefix[fuim_Observer((field_data_ptr->Prefix))];
               length += IMG_GET_WIDTH(img_id);
             }
             
             if ((field_data_ptr->Suffix) != FMNU_SUFFIX_NONE) {  
               img_id = (img_storage_id_t)fmnu_str_Suffix[fuim_Observer((field_data_ptr->Suffix))];
               length += IMG_GET_WIDTH(img_id);
             }

          }

        }break;

        /*====================================================================*
         * GROUP 2: List types
         * COMBINED: LIST, ONOFFLIST
         * Both types use fmnu_GetListStringLen() to obtain the width
         *====================================================================*/
        case FUIM_FIELDTYPE_LIST:
        {
            length = fmnu_GetListStringLen(field_data_ptr);
        }break;

        /*====================================================================*
         * GROUP 3: Numeric field types
         * NUMERIC and NUMERIC_VALUE - width calculation based on FieldSize
         * and font size
         *====================================================================*/
        case FUIM_FIELDTYPE_NUMERIC:
        case FUIM_FIELDTYPE_NUMERIC_VALUE:
        {
          if((fuim_ValidityFunction( field_data_ptr -> ValidityFunction)) == FUIM_VALIDITY_PRESENT) {
            length = 0;
          } else {
            /* Get the number of digits */
            Byte field_size = field_data_ptr->FieldSize.Numeric;

            /* Determine character width based on font size */
            Word char_width = FUIM_DIGITS_CHAR_WIDTH_SMALL;  /* Default small font */

            /* Check font size in the FieldCharacters.NumericFont structure */
            if (field_data_ptr->FieldCharacters.NumericFont.size == FUIM_FONT_SIZE_LARGE) {
               char_width = FUIM_DIGITS_CHAR_WIDTH_LARGE;
            }

            /* Calculate the integer part width */
            if (field_size > 0) {
               length = field_size * char_width;
            }
          }

        }break;

        /*====================================================================*
         * GROUP 4: Button
         * BUTTON - width is determined by the button image
         *====================================================================*/
        case FUIM_FIELDTYPE_BUTTON:
        {
            /* Get the button string ID from FieldCharacters.Button */
           img_storage_id_t button_id = fuim_GetPromptStringId(field_data_ptr->Prompt);

           if (button_id < FMNU_NONE_PROMPT) {
               length = IMG_GET_WIDTH((img_storage_id_t)button_id);
           }

            button_id = (osdStringID)field_data_ptr->FieldCharacters.Button;
            if (button_id < FMNU_NONE_PROMPT) {
                length += FUIM_BUTTON_PROMPT_TO_NAME_PADDING + IMG_GET_WIDTH((img_storage_id_t)button_id);
            }

        }break;

        /*====================================================================*
         * GROUP 5: Spacers and separators
         * SPACER and SEPARATOR - have no visual representation
         *====================================================================*/
        case FUIM_FIELDTYPE_SPACER:
        case FUIM_FIELDTYPE_SEPARATOR:
            length = 0;
            break;

        /*====================================================================*
         * UNKNOWN TYPE
         *====================================================================*/
        default:
            length = 0;
            break;
    }
    length = (length <= FUIM_MENU_WIDTH) ? (length) : (0);

    (void)value;
    return length;
}


/*=================================================================================
*   Function:    fuim_GetPromptStringId
*   Description: Maps a prompt index byte to an image storage ID via the observer.
*
*   Arguments:
*   Parameter    Flow    Description
*   ------------------------------------------------------------------------------
*   index    IN    Prompt index byte
*
*   Externals    Flow    Usage
*   ------------------------------------------------------------------------------
*   fuim_Observer() IN  Dispatches index to img_storage_id_t
*
*   @return Image storage ID, or IMG_INVALID_ID if index is invalid/NONE
*
*   @note Returns IMG_INVALID_ID for out-of-range or AUIM_NONE_PROMPT values.
===================================================================================*/
img_storage_id_t fuim_GetPromptStringId(Byte index)
{
 img_storage_id_t prompt_id = IMG_INVALID_ID;

  if ( ( index <  AUIM_OBSERVER_INDEX_NONE) && ( index != AUIM_NONE_PROMPT) ) {
    prompt_id = fuim_Observer(index);
  }

  return prompt_id;
}
