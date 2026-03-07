/*
 * fuim.c
 *
 *  Created on: 19 февр. 2026 г.
 *      Author: priss
 */

/*=======================================================================*/
/*        I N C L U D E S                                                */
/*=======================================================================*/
#include "sl_sleeptimer.h"
#include "pltccstd.h"
#include "fuim.h"
#include "fmnu.h"
#include "auph.h"
#include "fuim_obs.h"
#include "auim_api.h"
#include "rbsc_api.h"
#include "disp.h"


/*============================================================================*/
/*    G L O B A L  S Y M B O L    D E C L A R A T I O N S                     */
/*============================================================================*/


/*=======================================================================*/
/* L O C A L   D E F I N I T I O N S                                     */
/*=======================================================================*/
#define FUIM_MAX_FONT_ID_COUNT      11U
#define FUIM_FONT_DOT_CHAR_INDEX    10U

typedef const uint16_t (*font_array_ptr_t)[FUIM_MAX_FONT_ID_COUNT];

/*======================================================================*
 * @brief Table of digit image IDs.
 * Indexing: [Size][Color][Digit]
 * Located in Flash.
========================================================================*/

// Small font (only 1 colour - black)                                      //0                          // 1                             // 2                       // 3                             //4                         //5                           //6                            //7                             //8
static const Word font_small_black[FUIM_MAX_FONT_ID_COUNT] =  {IMG_ID_PROPERTY_1_DEFAULT_6, IMG_ID_PROPERTY_1_VARIANT2_6, IMG_ID_PROPERTY_1_VARIANT3_6, IMG_ID_PROPERTY_1_VARIANT4_5,  IMG_ID_PROPERTY_1_VARIANT9_5, IMG_ID_PROPERTY_1_VARIANT8_5, IMG_ID_PROPERTY_1_VARIANT7_5, IMG_ID_PROPERTY_1_VARIANT6_5, IMG_ID_PROPERTY_1_VARIANT5_5, IMG_ID_PROPERTY_1_VARIANT10_5, 110};

// Large font (4 colours)
static const Word font_large_color1[FUIM_MAX_FONT_ID_COUNT] = {IMG_ID_PROPERTY_1_VARIANT11,   IMG_ID_PROPERTY_1_VARIANT2_2, IMG_ID_PROPERTY_1_VARIANT3_2, IMG_ID_PROPERTY_1_VARIANT4_1, IMG_ID_PROPERTY_1_VARIANT9_1, IMG_ID_PROPERTY_1_VARIANT8_1, IMG_ID_PROPERTY_1_VARIANT7_1, IMG_ID_PROPERTY_1_VARIANT6_1, IMG_ID_PROPERTY_1_VARIANT5_1, IMG_ID_PROPERTY_1_VARIANT10_1, IMG_MAX_IDS_STORAGE_DESC_COUNT};
static const Word font_large_color2[FUIM_MAX_FONT_ID_COUNT] = {IMG_ID_PROPERTY_1_VARIANT11_1, IMG_ID_PROPERTY_1_VARIANT2_3, IMG_ID_PROPERTY_1_VARIANT3_3, IMG_ID_PROPERTY_1_VARIANT4_2, IMG_ID_PROPERTY_1_VARIANT9_2, IMG_ID_PROPERTY_1_VARIANT8_2, IMG_ID_PROPERTY_1_VARIANT7_2, IMG_ID_PROPERTY_1_VARIANT6_2, IMG_ID_PROPERTY_1_VARIANT5_2, IMG_ID_PROPERTY_1_VARIANT10_2, IMG_MAX_IDS_STORAGE_DESC_COUNT};
static const Word font_large_color3[FUIM_MAX_FONT_ID_COUNT] = {IMG_ID_PROPERTY_1_VARIANT11_2, IMG_ID_PROPERTY_1_VARIANT2_4, IMG_ID_PROPERTY_1_VARIANT3_4, IMG_ID_PROPERTY_1_VARIANT4_3, IMG_ID_PROPERTY_1_VARIANT9_3, IMG_ID_PROPERTY_1_VARIANT8_3, IMG_ID_PROPERTY_1_VARIANT7_3, IMG_ID_PROPERTY_1_VARIANT6_3, IMG_ID_PROPERTY_1_VARIANT5_3, IMG_ID_PROPERTY_1_VARIANT10_3, IMG_MAX_IDS_STORAGE_DESC_COUNT};
static const Word font_large_color4[FUIM_MAX_FONT_ID_COUNT] = {IMG_ID_PROPERTY_1_VARIANT11_3, IMG_ID_PROPERTY_1_VARIANT2_5, IMG_ID_PROPERTY_1_VARIANT3_5, IMG_ID_PROPERTY_1_VARIANT4_4, IMG_ID_PROPERTY_1_VARIANT9_4, IMG_ID_PROPERTY_1_VARIANT8_4, IMG_ID_PROPERTY_1_VARIANT7_4, IMG_ID_PROPERTY_1_VARIANT6_4, IMG_ID_PROPERTY_1_VARIANT5_4, IMG_ID_PROPERTY_1_VARIANT10_4, IMG_MAX_IDS_STORAGE_DESC_COUNT};

/*=======================================================================================*
* LUT: [Size][Colour]
* For SMALL size, only index [0][0] is used.
* For LARGE size, indexes [1][0...3] are used.
=========================================================================================*/
static const font_array_ptr_t font_lut[FUIM_MAX_FONT_SIZE][FUIM_MAX_FONT_COLOR] = {
  { &font_small_black, NULL, NULL, NULL },
  { &font_large_color1, &font_large_color2, &font_large_color3, &font_large_color4 }
};

/*****************************************************************************

*****************************************************************************/

const fuimColourStruct MainMenuPromptColor =
{
   FUIM_COLOUR_4,       /* Foreground colour             */
   FUIM_COLOUR_3,       /* Background colour             */
   FUIM_COLOUR_4,       /* Foreground highlighted colour */
   FUIM_COLOUR_3,       /* Background highlighted colour */
   FUIM_ATTRIBUTES_DOUBLEHEIGHT + FUIM_ATTRIBUTES_SHADOWED,/* No attributes                 */
   FUIM_ATTRIBUTES_DOUBLEHEIGHT + FUIM_ATTRIBUTES_SHADOWED /* attributes highlighted     */

};

/*****************************************************************************

*****************************************************************************/
const fuimColourStruct IndicatorPromptColour =
{
   FUIM_COLOUR_2,                       /* Foreground colour             */
   FUIM_COLOUR_0,                   /* Background colour             */
   FUIM_COLOUR_7,                       /* Foreground highlighted colour */
   FUIM_COLOUR_TRANSPARENT,             /* Background highlighted colour */
   FUIM_ATTRIBUTES_DOUBLEHEIGHT+FUIM_ATTRIBUTES_SHADOWED, /* With shadowing   */
   FUIM_ATTRIBUTES_NONE         /*attributes highlighted */
};

/*****************************************************************************

*****************************************************************************/
const fuimColourStruct NormalMenuPromptColour =
{
   FUIM_COLOUR_7,       /* Foreground colour             */
   FUIM_COLOUR_0,       /* Background colour             */
   FUIM_COLOUR_7,       /* Foreground highlighted colour */
   FUIM_COLOUR_4,       /* Background highlighted colour */
   FUIM_ATTRIBUTES_SHADOWED,/* No attributes                 */
   FUIM_ATTRIBUTES_SHADOWED /* attributes highlighted      */
};

#define FUIM_PERIODIC_TIMER_DELAY_64MS  64

/*============================================================================*/
/*    L O C A L  S Y M B O L    D E C L A R A T I O N S                     */
/*============================================================================*/

static fuimIndicatorStruct            * fuim_Indicators[FUIM_MAX_INDICATORS];
static fuim_IndicatorProperty fuim_indicator_properties[FUIM_MAX_INDICATORS];
static osdTimerHandle            indicator_timer_handle[FUIM_MAX_INDICATORS];
static TIMER fuim_Timer[FUIM_MAX_TIMERS];
static sl_sleeptimer_timer_handle_t  fuim_timeout_timer_handle;


/*=============================================================================*/
/*    L O C A L   F U N C T I O N   P R O T O T Y P E S                        */
/*=============================================================================*/
static void SetupPeriodicTimer (Byte timeOutInTicks);

void fuim_PeriodicTimerExpired(void);
void fuim_UpdateTimer064ms(void);
void fuim_TimerFunction(Byte index, osdDialogHandle hDialog);

Word fuim_GetIndicatorFieldValueLength(fuimFieldStruct *field_data_ptr);
Word fuim_DigitToFontAssetId(char c, fuim_FontSize size, uint8_t color_idx);


/*==========================================================================*/
/*    L O C A L   F U N C T I O N                                           */
/*==========================================================================*/
/*==========================================================================*
* @brief Get the font resource ID by dereferencing the pointer to the array.
*/
Word fuim_DigitToFontAssetId(char c, fuim_FontSize size, uint8_t color_idx)
{

  Byte char_idx;

  // 1. Converting ASCII to an array index
  if (c >= '0' && c <= '9') {
      char_idx = (uint8_t)(c - '0');
  } else if (c == '.') {
      char_idx = FUIM_FONT_DOT_CHAR_INDEX;
  } else {
      return IMG_MAX_IDS_STORAGE_DESC_COUNT; // Unsupported character
  }

  // 2. Parameter validation
  if (size >= FUIM_MAX_FONT_SIZE || color_idx >= FUIM_MAX_FONT_COLOR) {
      return IMG_MAX_IDS_STORAGE_DESC_COUNT;
  }

  // 3. Access via the pointer table
  font_array_ptr_t selected_font = font_lut[size][color_idx];

  if (selected_font == NULL) {
      return IMG_MAX_IDS_STORAGE_DESC_COUNT;
  }

  // Dereference the pointer to the array and take the required element.
  return (Word)(*selected_font)[char_idx];
}

/***************************************************************************//**
 * Sleeptimer callback function to generate card control timing.
 ******************************************************************************/
static void fuim_timer_callback(sl_sleeptimer_timer_handle_t *handle,
                                          void *data)
{
  (void)handle;
  (void)data;
  fuim_UpdateTimer064ms();
}

/*********************************************************************************


**********************************************************************************/
static void SetupPeriodicTimer (Byte timeOutInTicks)
{
    fuim_Timer[FUIM_PERIODIC_TIMER].TimeOut   = timeOutInTicks;
    fuim_Timer[FUIM_PERIODIC_TIMER].TimerID   = PERIODIC_TIMER_FUNCTION;
    fuim_Timer[FUIM_PERIODIC_TIMER].Parameter = (osdDialogHandle) timeOutInTicks;
}

/*********************************************************************************


**********************************************************************************/
void fuim_DrawTitle(img_storage_id_t img_id, Word width, Word bg_color, Bool remove)
{

  uint16_t draw_x;
  uint16_t draw_y;

  plt_CCGetPosition(&draw_y, &draw_x);

  if (!remove)
     disp_DrawImage(draw_x, draw_y, img_id);
  else
     disp_EraseImage(draw_x, draw_y, img_id, bg_color);

  width = width;
}

/*********************************************************************************


**********************************************************************************/
void fuim_DrawString(img_storage_id_t img_id)
{

  uint16_t draw_x;
  uint16_t draw_y;

  plt_CCGetPosition(&draw_y, &draw_x);
  disp_DrawImage(draw_x, draw_y, img_id);

}

/*********************************************************************************


**********************************************************************************/
void fuim_EraseField( Word FirstPos, Word EndPos, fuim_FieldType Type )
{
  Word  length;

  plt_CCSetPosition (fuim_GetRowPosition(), FirstPos);
  length = EndPos - FirstPos + 1;
  //fuim_DrawRepeatedCharacter (length, ' ');

    Type = Type;
    length = length;
}

/*********************************************************************************


**********************************************************************************/
void fuim_InitTimers(void)
{
    osdTimerHandle i;

    for (i = 0; i < FUIM_MAX_TIMERS; i++)
    {
        fuim_Timer[i].TimeOut   = RGEN_TIMER_STOPPED;
        fuim_Timer[i].TimerID   = EMPTY_TIMER;
        fuim_Timer[i].Parameter = 0;
    }

    return;
}


/*********************************************************************************
 

**********************************************************************************/

void fuim_InitIndicators(void)
{
	Byte  i;

	for( i = 0; i < FUIM_MAX_INDICATORS; i++) {
		fuim_Indicators[i] = NULL ;			
		indicator_timer_handle[i] = FUIM_NO_FREE_TIMER_HANDLE ;
	}
}


/*********************************************************************************


**********************************************************************************/
osdDialogHandle fuim_GetIndicatorHandle(fuimIndicatorStruct *indicator_data_ptr)
{
  osdDialogHandle  i;

  for(i = 0; i < FUIM_MAX_INDICATORS && (fuim_Indicators[i]!=indicator_data_ptr); i++ );

  if (i < FUIM_MAX_INDICATORS) {
    return i + 1;
  }
  return 0;
}

/*********************************************************************************


**********************************************************************************/
Word fuim_GetIndicatorFieldValueLength(fuimFieldStruct *field_data_ptr)
{
  Word length = 0;


  switch( field_data_ptr->Type )
  {
    case FUIM_FIELDTYPE_NUMERIC:
      if( field_data_ptr->FieldScaling.Numeric != 0 )
      {
        length++;
      }
      length += 2;
    break;

    case FUIM_FIELDTYPE_STRING:
      //length =  fuim_GetStringLength((Byte *) fuim_Observer (field_data_ptr->GetFunction)) ;
      length += 1;
    break;

    case FUIM_FIELDTYPE_STRING_ID: {
      img_storage_id_t storage_id = (img_storage_id_t)fuim_Observer (field_data_ptr->GetFunction);

      if (storage_id < IMG_MAX_IDS_STORAGE_DESC_COUNT) {
        length = IMG_GET_WIDTH(storage_id);
      } else {
        length = 0;
      }
    }break;

    default:
      length = 0;
     break;
  }

  return length;
}

/****************************************************************************************
Validates the parameters and draws the value of the string-field.
 : none
field_data_ptr
      pointer to the data-structure which describes the field
    Highlighted
Status of the field, TRUE when highlighted colours need to be used, FALSE for non-highlighted colours
******************************************************************************************/
void fuim_ConstructString(fuimFieldStruct  *field_data_ptr, Bool Highlighted)
{
    Highlighted = Highlighted;
    field_data_ptr = field_data_ptr;
}

/*********************************************************************************


**********************************************************************************/
void fuim_ConstructIndicatorValue( fuimFieldStruct  *field_data_ptr )
{
 Byte  ForeGndColour;

 if((fuim_ValidityFunction( field_data_ptr -> ValidityFunction))== FUIM_VALIDITY_GRAYEDOUT)
 {
     ForeGndColour = FUIM_GRAYED_OUT_COLOUR;
 }
 else
 {
     ForeGndColour = FUIM_GRAYED_OUT_COLOUR;//fuim_GetFieldPromptColour( field_data_ptr) -> ForeGndColour  ;
 }


  switch( field_data_ptr->Type )
  {

    case FUIM_FIELDTYPE_SPACER:
      break;

      case FUIM_FIELDTYPE_STRING_ID:
        fuim_DrawString( (img_storage_id_t) fuim_Observer (field_data_ptr->GetFunction));
      break;

      case FUIM_FIELDTYPE_STRING:
        fuim_ConstructString( field_data_ptr, TRUE);//Highlighted );
      break;


      default:
        break;

  }

  ForeGndColour = ForeGndColour;

}

/*********************************************************************************


**********************************************************************************/
void fuim_ConstructIndicatorPrompt(fuimFieldStruct *field_data_ptr,  Word value_position )
{
  Byte  PromptID = (field_data_ptr->Prompt);

  value_position = value_position;

  if(( PromptID ) !=  FMNU_NONE_PROMPT) {
      fuim_DrawString((img_storage_id_t) PromptID);
  }

}

/*********************************************************************************


**********************************************************************************/
void fuim_ConstructIndicatorField( fuimFieldStruct         *field_data_ptr,
                                   fuim_IndicatorProperty  *position )
{

  Byte  ForeGndColour = 0;
  Byte  BackGndColour = 0;
  Byte  Attribute = 0;
  Word  value_length;
  Word  value_delta = 0;


  Attribute     = fuim_GetFieldPromptColour(field_data_ptr) -> Attribute;
  ForeGndColour = fuim_GetFieldPromptColour(field_data_ptr) -> ForeGndColour;
  BackGndColour = fuim_GetFieldPromptColour(field_data_ptr) -> BackGndColour;

  /************************************************************
  *   Prepare setting from the end of field
  *************************************************************/

  value_length = fuim_GetIndicatorFieldValueLength( field_data_ptr );

  if( (position->EndPos) != (position -> ValuePos + value_length) )
  {
      if( (position->EndPos) > (position -> ValuePos + value_length) )
      {
          value_delta = (position->EndPos) - ( (position -> ValuePos) + value_length);
      }

    position->EndPos      = position -> ValuePos + value_length ;
    position->FieldWidth  = position -> EndPos - position -> FirstPos;

  }

  /************************************************************
  *   The indicator is drawn starting from the END position.
  *************************************************************/

  plt_CCSetBackgroundColour( FUIM_COLOUR_TRANSPARENT);
  fuim_SetAttributes( FUIM_ATTRIBUTES_NONE );

  plt_CCSetRowSize( Attribute & (FUIM_ATTRIBUTES_DOUBLEHEIGHT + FUIM_ATTRIBUTES_DOUBLEWIDTH));


  plt_CCSetPosition( fuim_GetRowPosition() , ( position->EndPos ) );

  if(value_delta != 0) // Erase previous content
  {
    fuim_SetColumnPosition( ( position->EndPos ) + 1 );
   //fuim_DrawRepeatedCharacter (value_delta, ' ');
  }


/*************************************************************************************/



   plt_CCSetPosition( fuim_GetRowPosition() , ( position->FirstPos ) );

   plt_CCSetForegroundColour(ForeGndColour);
   plt_CCSetBackgroundColour(BackGndColour);
   fuim_SetAttributes( Attribute);

   plt_CCSetPosition( fuim_GetRowPosition(), ( position->FirstPos ) + 1 );

   fuim_SetColumnPosition( position ->PromptPos);


   /*************************************************************************************/
   fuim_ConstructIndicatorPrompt( field_data_ptr, position->ValuePos );
   /*************************************************************************************/

   fuim_SetColumnPosition(position  -> ValuePos);

//остановился на позиции ValuePos


   plt_CCSetForegroundColour( ForeGndColour);

   /*************************************************************************************/
   fuim_ConstructIndicatorValue(field_data_ptr );
   /*************************************************************************************/

  // Затерание предыдущего конца

  // fuim_DrawRepeatedCharacter ((position->FirstPos + position->FieldWidth) - fuim_GetColumnPosition(), ' ');


}


/*********************************************************************************


**********************************************************************************/
void fuim_DestroyIndicatorField( fuimFieldStruct  *field_data_ptr, fuim_IndicatorProperty *position)
{
   Word   RowPos;

   fuim_EraseField( position->FirstPos, position->EndPos, field_data_ptr->Type);
   RowPos = fuim_GetRowPosition();
   plt_CCSetPosition(RowPos, 0 );
 //  plt_CCSetRowSize(FUIM_ATTRIBUTES_DOUBLEHEIGHT);
  // grf_FlyBackSync();
   plt_CCSetPosition(RowPos, 0);
   position->EndPos = 0; // - это связанно с вычислением  value_delta != 0  Затерание предыдущего конца
   return;
}

/*********************************************************************************


**********************************************************************************/
osdDialogHandle fuim_ConstructIndicator(fuimIndicatorStruct *indicator_data_ptr)
{
  osdDialogHandle          handle;
  fuimFieldStruct        * field_data_ptr;
  fuim_IndicatorProperty * properties;

  handle = 0 ;

  while (handle < FUIM_MAX_INDICATORS && fuim_Indicators[handle] != NULL) {
    handle++;
  }

  if( handle == FUIM_MAX_INDICATORS ) {
    return  (osdDialogHandle)0;
  } else {
    field_data_ptr = (fuimFieldStruct *)indicator_data_ptr->Field;
    if(fuim_ValidityFunction(field_data_ptr-> ValidityFunction) == FUIM_VALIDITY_NOTPRESENT) {
        return  (osdDialogHandle)0;
    } else {
        fuim_Indicators[ handle ] = indicator_data_ptr ;
    }
  }

  properties = &fuim_indicator_properties [handle];

  properties->FirstPos  = fuim_GetIndicatorHorLocation(indicator_data_ptr);
  properties->PromptPos = fuim_GetIndicatorHorLocation(indicator_data_ptr);
  properties->ValuePos  = fuim_GetIndicatorValuePos(indicator_data_ptr);

    /*  Set Row Position in pixels   */

  fuim_SetRowPosition (fuim_GetIndicatorVertLocation(indicator_data_ptr));
  fuim_ConstructIndicatorField (field_data_ptr, properties);

  if (fuim_GetIndicatorTimeout(indicator_data_ptr) > 0) {
    indicator_timer_handle [handle] = fuim_ConstructTimer (fuim_GetIndicatorTimeout(indicator_data_ptr),
                                                               INDICATOR_TIMER_FUNCTION, handle + 1);
  } else {
    indicator_timer_handle [handle] = FUIM_NO_FREE_TIMER_HANDLE;
  }

  return handle + 1;
}

/*=*****************************************************************************

******************************************************************************/
void fuim_DestroyIndicator(osdDialogHandle indicator )
{

  fuimIndicatorStruct *indicator_data_ptr;
  Word  row, col;

  if (indicator == 0 || indicator > FUIM_MAX_INDICATORS) {
    return ;
  }

  indicator--;

  indicator_data_ptr = fuim_Indicators[ indicator ] ;

  row = fuim_GetIndicatorVertLocation (indicator_data_ptr);
  col = fuim_GetIndicatorHorLocation  (indicator_data_ptr);
  plt_CCSetPosition( row, col );

  fuim_DestroyIndicatorField ((fuimFieldStruct  *)indicator_data_ptr->Field , (fuim_IndicatorProperty *)&fuim_indicator_properties[ indicator ] );

  fuim_Indicators [indicator] = NULL;

  if (indicator_timer_handle [indicator] != FUIM_NO_FREE_TIMER_HANDLE) {
    fuim_DestroyTimer (&indicator_timer_handle [indicator]);
  }


}


/*=*****************************************************************************

******************************************************************************/
void fuim_UpdateIndicator (osdDialogHandle handle, Bool restart_timer)
{

  fuimIndicatorStruct   *indicator_data_ptr;
  fuimFieldStruct       *field_data_ptr;
//  fuim_IndicatorProperty *position;

  if( (handle == 0) || (handle > FUIM_MAX_INDICATORS) || ( fuim_Indicators[ handle-1 ] == NULL) ) {
    return ;
  }

  handle--;

  indicator_data_ptr = fuim_Indicators [handle];
  field_data_ptr = (fuimFieldStruct *)indicator_data_ptr->Field;

//position = &fuim_indicator_properties[handle];

  fuim_SetRowPosition( fuim_GetIndicatorVertLocation (indicator_data_ptr) );

  if(restart_timer) {
    fuim_ConstructIndicatorField( field_data_ptr, &fuim_indicator_properties[handle] );
  } else {
    if(field_data_ptr ->TimeOut != FUIM_FIELD_NO_TIMEOUT) {
      fuim_ConstructIndicatorField( field_data_ptr, &fuim_indicator_properties[handle] );
    }
  }

  if (restart_timer) {
    if ( fuim_Timer[FUIM_PERIODIC_TIMER].TimeOut != RGEN_TIMER_STOPPED ) {
      if ( indicator_timer_handle[handle] != FUIM_NO_FREE_TIMER_HANDLE ) {
        fuim_RestartTimer( indicator_timer_handle[handle], fuim_GetIndicatorTimeout(indicator_data_ptr) );
      }
    }
  }

}

/*********************************************************************************


**********************************************************************************/
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
/*********************************************************************************


**********************************************************************************/
void fuim_DestroyTimer(osdTimerHandle *hTimer )
{
  fuim_Timer[*hTimer - 1].TimeOut   = RGEN_TIMER_STOPPED;
  fuim_Timer[*hTimer - 1].TimerID   = EMPTY_TIMER;
  fuim_Timer[*hTimer - 1].Parameter = 0;
  *hTimer = FUIM_NO_FREE_TIMER_HANDLE;
}

/*********************************************************************************
 

**********************************************************************************/

void fuim_RestartTimer (osdTimerHandle hTimer,  Byte TimeoutInSeconds )
{
  if (fuim_Timer[hTimer - 1].TimeOut > RGEN_TIMER_STOPPED) {
      fuim_Timer[hTimer - 1].TimeOut = TimeoutInSeconds * FUIM_TIMER_RESOLUTION;
  }
}

/*********************************************************************************


**********************************************************************************/
void fuim_UpdateTimer064ms(void)
{
  Byte i;
  for( i = 0; i < FUIM_MAX_TIMERS; i++) {
    rbsc_UpdateTimer(&fuim_Timer[i].TimeOut);
  }
}

/*********************************************************************************


**********************************************************************************/

void fuim_DestroyIndicatorTimer(osdDialogHandle indicator )
{
	if (indicator == 0 || indicator > FUIM_MAX_INDICATORS) {
		return ;
	}

	indicator--;

	if (indicator_timer_handle [indicator] != FUIM_NO_FREE_TIMER_HANDLE) {
		
        fuim_DestroyTimer (&indicator_timer_handle [indicator]);
	}

} 


/*********************************************************************************


**********************************************************************************/
void fuim_PeriodicTimerExpired(void)
{

  fmnu_UpdateMenu();

  if(auph_GetState() != AU_ERROR_STATE) {
    for (Byte i = 1; i <= FUIM_MAX_INDICATORS; i++) {
        fuim_UpdateIndicator(i, FALSE);
    }
  }

}

/* =============================================================================== */
/* @parm timer function ID */
/* @parm argument that can be passed to timer function */
/*
   @func    Calls an timer function

   @rdesc   None.
*//**********************************************************************************/
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
      auph_SetState(AU_DIRECT_STATE);
    } break;

    default:
      break;
  }

}


/*********************************************************************************


**********************************************************************************/
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
}

/*****************************************************************************

*****************************************************************************/
void fuim_Init(void)
{
  bool is_timer64ms_running = false;


  plt_CCInit( FUIM_MAX_NR_OF_COLS, FUIM_MAX_NR_OF_ROWS);//, TRUE );
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

/*****************************************************************************

*****************************************************************************/
void fuim_TurnOn(void)
{
    SetupPeriodicTimer(FUIM_PERIODIC_TIMEOUT);
}

/*****************************************************************************

*****************************************************************************/
void fuim_TurnOff(void)
{
    fuim_InitTimers();
}

/*********************************************************************************


**********************************************************************************/
void fuim_Update(void)
{
    fuim_UpdateTimers();
}

/*********************************************************************************


**********************************************************************************/
Word fuim_GetColumnPosition(void)
{
  Word posd_CCRow, posd_CCColumn;
  plt_CCGetPosition(&posd_CCRow, &posd_CCColumn);
  return ( posd_CCColumn );
}

/*********************************************************************************


**********************************************************************************/
Word fuim_GetRowPosition(void)
{
  Word  posd_CCRow, posd_CCColumn;
  plt_CCGetPosition(&posd_CCRow, &posd_CCColumn);
  return ( posd_CCRow );
}

/*********************************************************************************


********************************************************************************/
void fuim_SetRowPosition(Word row)
{
  plt_CCSetPosition( row, fuim_GetColumnPosition() );
}

/*********************************************************************************


**********************************************************************************/
void fuim_SetColumnPosition(Word column)
{
  plt_CCSetPosition( fuim_GetRowPosition(), column );
}


/*********************************************************************************


**********************************************************************************/
void fuim_SetAttributes(Byte Attributes)
{
//  Bool Mode = TRUE;

  if (Attributes == FUIM_ATTRIBUTES_NONE)
  {
    Attributes = 0xFF;
 //   Mode = FALSE;
  }

}


/*********************************************************************************


**********************************************************************************/
void fuim_SetIndicatorTimeOut (osdDialogHandle hDialog,  Byte TimeOutInSeconds)
{

  hDialog --;
  if ( indicator_timer_handle [hDialog] == FUIM_NO_FREE_TIMER_HANDLE ) {
    indicator_timer_handle [hDialog] = fuim_ConstructTimer (TimeOutInSeconds,INDICATOR_TIMER_FUNCTION, hDialog + 1);
  }

}

/*********************************************************************************


**********************************************************************************/
void fuim_SetNextRow(void)
{
    plt_CCSetPosition( fuim_GetRowPosition() + 1, fuim_GetColumnPosition() );
}


/*MPF=======================================================================*/
/*
   @func    Calls an dynamic colours get function

   @rdesc   The result of the dynamic colours get function with ID index.

*/
fuimColourStruct  *fuim_DynamicColours ( Byte index )/* @parm function ID */
/*EMP=======================================================================*/
{
   switch (index)
   {
     case AUIM_MAIN_MENU_COLOUR:
         return ((fuimColourStruct  *)&MainMenuPromptColor);
         break;

     case  AUIM_INDICATOR_COLOUR:
         return ((fuimColourStruct  *)&IndicatorPromptColour);
         break;

      case AUIM_MENU_FIELD_COLOUR:
         return ((fuimColourStruct  *)&NormalMenuPromptColour);
         break;

    default:
         return ((fuimColourStruct  *)&IndicatorPromptColour);
         break;

  }

}
