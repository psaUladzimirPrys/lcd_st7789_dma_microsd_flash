/*
 * fuim.c
 *
 *  Created on: 19 февр. 2026 г.
 *      Author: priss
 */

/*=======================================================================*/
/*        I N C L U D E S                                                */
/*=======================================================================*/
#include "fuim.h"
#include "ui_display.h"
#include "fmnu.h"
#include "ccstd.h"
#include "aukh.h"
#include "auph.h"
#include "fuim_obs.h"



/*=======================================================================*/
/* G L O B A L   D E F I N I T I O N S                                   */
/*=======================================================================*/


/*============================================================================*/
/*    G L O B A L  S Y M B O L    D E C L A R A T I O N S                     */
/*============================================================================*/
fuimIndicatorStruct    *fuim_Indicators[FUIM_MAX_INDICATORS];
fuim_IndicatorProperty  fuim_indicator_properties[FUIM_MAX_INDICATORS];
       osdTimerHandle   indicator_timer_handle[FUIM_MAX_INDICATORS];
                TIMER   fuim_Timer[FUIM_MAX_TIMERS];


/*=============================================================================*/
/*    L O C A L   F U N C T I O N   P R O T O T Y P E S                        */
/*=============================================================================*/

static void SetupPeriodicTimer (Byte timeOutInTicks);
void PeriodicTimerExpired(void);
/*==========================================================================*/
/*    L O C A L   F U N C T I O N                                           */
/*==========================================================================*/

static void SetupPeriodicTimer (Byte timeOutInTicks)
{
    fuim_Timer[FUIM_PERIODIC_TIMER].TimeOut   = timeOutInTicks;
    fuim_Timer[FUIM_PERIODIC_TIMER].TimerID   = PERIODIC_TIMER_FUNCTION;
    fuim_Timer[FUIM_PERIODIC_TIMER].Parameter = (osdDialogHandle) timeOutInTicks;

    return;
}



/*=============================================================================*/
/*    L O C A L   F U N C T I O N   P R O T O T Y P E S                        */
/*=============================================================================*/
void fuim_DrawImage(int16_t x, int16_t y, img_storage_id_t img_id);


/*==========================================================================*/
/*    L O C A L   F U N C T I O N                                           */
/*==========================================================================*/
void fuim_DrawImage(int16_t x, int16_t y, img_storage_id_t img_id)
{
   adafruit_st7789_draw_rgb_bitmap_from_flash(x, y, IMG_GET_WIDTH(img_id), IMG_GET_HEIGHT(img_id), IMG_GET_ADDRESS(img_id),  true);
}

void fuim_EraseImage(int16_t x, int16_t y, img_storage_id_t img_id, Word bg_color)
{
  adafruit_st7789_fill_rectangle(x, y, IMG_GET_WIDTH(img_id), IMG_GET_HEIGHT(img_id), bg_color);
}

/*********************************************************************************


**********************************************************************************/
void fuim_DrawTitle(img_storage_id_t img_id, Byte width, Word bg_color, Bool remove)
{

  uint16_t draw_x  = width  - FUIM_TITLE_RIGHT_MARGIN -  IMG_GET_WIDTH(img_id);
  uint16_t draw_y  = (uint16_t)((FUIM_TITLE_HEIGHT - IMG_GET_HEIGHT(img_id)) / 2);

  if (!remove)
     fuim_DrawImage(draw_x, draw_y, img_id);
  else
     fuim_EraseImage(draw_x, draw_y, img_id, bg_color);
}



/*********************************************************************************


**********************************************************************************/
void fuim_DrawString(img_storage_id_t img_id)

{
  //while( *string_ptr != 0 )
 // {
 //   plt_CCDrawChar( *string_ptr ); //Это самый низ
 //   string_ptr++;
 // }

  uint16_t draw_x  = FUIM_MENU_WIDTH  - FUIM_TITLE_RIGHT_MARGIN -  IMG_GET_WIDTH(img_id);
  uint16_t draw_y  = (uint16_t)((FUIM_MENU_HEIGHT - IMG_GET_HEIGHT(img_id)) / 2);

  fuim_DrawImage(draw_x, draw_y, img_id);

}

/*********************************************************************************


**********************************************************************************/

void fuim_EraseField( Byte FirstPos,Byte EndPos, fuim_FieldType Type )
{
  Byte  length;

  MY_plt_CCSetPosition (fuim_GetRowPosition(), FirstPos);
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

void fuim_InitIndicators( void )
{
	Byte  i;

	for( i = 0; i < FUIM_MAX_INDICATORS; i++)
	{
		fuim_Indicators[i] = NULL ;			
		indicator_timer_handle[i] = FUIM_NO_FREE_TIMER_HANDLE ;
	}
    return;
}

osdDialogHandle fuim_GetIndicatorHandle(fuimIndicatorStruct   *indicator_data_ptr )
{
  osdDialogHandle  i;

  for( i=0; i<FUIM_MAX_INDICATORS && ( fuim_Indicators[i]!=indicator_data_ptr ) ;i++ );

  if ( i<FUIM_MAX_INDICATORS )
  {
    return i + 1;
  }

  return 0;
}


Byte fuim_GetIndicatorFieldValueLength(fuimFieldStruct   *field_data_ptr)

{
  Byte length = 0;


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
    fuim_DrawString( (img_storage_id_t) fuim_Observer (field_data_ptr->GetFunction));
}

/*===========================================================================
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
/*
    case FUIM_FIELDTYPE_SPACER:

      break;
    case FUIM_FIELDTYPE_SLIDER:
      fuim_ConstructIndicatorSlider( field_data_ptr );
      break;
*/

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
void fuim_ConstructIndicatorPrompt(fuimFieldStruct *field_data_ptr,  Byte value_position )
{
  Byte  Prompt = (field_data_ptr->Prompt);

  value_position = value_position;

  if(( Prompt ) !=  FMNU_NONE_PROMPT)
  {
      fuim_DrawString((img_storage_id_t) Prompt);
  }
/*
  else
  {
    if(((field_data_ptr ->Type) == FUIM_FIELDTYPE_SEPARATOR))
    {
      if(value_position >= fuim_GetColumnPosition())
      {
          fuim_DrawRepeatedCharacter(field_data_ptr -> FieldSize.Separator, field_data_ptr ->FieldCharacters.BeginEndCharacters[1]  );
      }
    }

  }
*/

}

/*********************************************************************************


**********************************************************************************/
void fuim_ConstructIndicatorField( fuimFieldStruct     *field_data_ptr,
                         fuim_IndicatorProperty  *position )
{

  Byte  ForeGndColour;
  Byte  BackGndColour;
  Byte  Attribute;
  //Byte XDATA MenuValidity;
  Byte  value_length;
  Byte  value_delta = 0;

// MenuValidity = fuim_ValidityFunction(field_data_ptr-> ValidityFunction);

  // if( MenuValidity != FUIM_VALIDITY_NOTPRESENT)
   //{
  /************************************************************
  *          Подготовка к установке   индикатора с конца
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
  *             Установка для индикатора с конца
  *************************************************************/

   fuim_SetBackgroundColour( FUIM_COLOUR_TRANSPARENT, TRUE );
   fuim_SetAttributes( FUIM_ATTRIBUTES_NONE );
   MY_plt_CCSetPosition( fuim_GetRowPosition() , ( position->EndPos ) );

     if(value_delta != 0) // Затерание предыдущего конца
     {
        fuim_SetColumnPosition( ( position->EndPos )+ 1 );
       //fuim_DrawRepeatedCharacter (value_delta, ' ');
     }

/*************************************************************************************/
  // Attribute = fuim_GetFieldPromptColour(field_data_ptr) -> Attribute;
  // ForeGndColour = fuim_GetFieldPromptColour( field_data_ptr) -> ForeGndColour  ;
  // BackGndColour = fuim_GetFieldPromptColour( field_data_ptr) -> BackGndColour;

//   plt_CCSetRowSize( Attribute & (FUIM_ATTRIBUTES_DOUBLEHEIGHT + FUIM_ATTRIBUTES_DOUBLEWIDTH));
   MY_plt_CCSetPosition( fuim_GetRowPosition() , 0 );

   plt_CCSetForegroundColour( ForeGndColour );
   fuim_SetBackgroundColour( BackGndColour, FALSE);
   MY_plt_CCSetPosition( fuim_GetRowPosition() , ( position->FirstPos ) );

   plt_CCSetForegroundColour(ForeGndColour);
   fuim_SetAttributes( Attribute);
   fuim_SetBackgroundColour( BackGndColour, TRUE);
   MY_plt_CCSetPosition( fuim_GetRowPosition(), ( position->FirstPos ) + 1 );

   fuim_SetColumnPosition( position ->PromptPos);
   plt_CCSetForegroundColour( ForeGndColour);

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


  // }

}


/*********************************************************************************


**********************************************************************************/
void fuim_DestroyIndicatorField( fuimFieldStruct  *field_data_ptr, fuim_IndicatorProperty *position)
{
   Byte   RowPos;
   fuim_EraseField( position->FirstPos, position->EndPos, field_data_ptr->Type);
   RowPos = fuim_GetRowPosition();
   plt_CCSetPosition(RowPos, 0 );
 //  plt_CCSetRowSize(FUIM_ATTRIBUTES_DOUBLEHEIGHT);
  // grf_FlyBackSync();
   MY_plt_CCSetPosition(RowPos, 0);
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

  while (handle < FUIM_MAX_INDICATORS && fuim_Indicators[handle] != NULL)
  {
    handle++;
  }

  if( handle == FUIM_MAX_INDICATORS )
  {
    return  (osdDialogHandle)0;
  }
  else
  {
    field_data_ptr = (fuimFieldStruct *)indicator_data_ptr->Field;

    if(fuim_ValidityFunction(field_data_ptr-> ValidityFunction) == FUIM_VALIDITY_NOTPRESENT)
    {
        return  (osdDialogHandle)0;
    } else {
        fuim_Indicators[ handle ] = indicator_data_ptr ;
    }
  }

//Устанавливаем указатель типа поля индикатор
//  field_data_ptr = indicator_data_ptr->Field;


  properties = &fuim_indicator_properties [handle];

  properties->FirstPos    = fuim_GetIndicatorHorLocation (indicator_data_ptr);
  properties->PromptPos   = fuim_GetIndicatorHorLocation (indicator_data_ptr) + 2;
  properties->ValuePos  = fuim_GetIndicatorValuePos (indicator_data_ptr);

    /*  Установка позиции строки   */

   fuim_SetRowPosition( fuim_GetIndicatorVertLocation (indicator_data_ptr));
   fuim_ConstructIndicatorField ( field_data_ptr, properties );

  if (fuim_GetIndicatorTimeout(indicator_data_ptr) > 0)
  {
    indicator_timer_handle [handle] = fuim_ConstructTimer (fuim_GetIndicatorTimeout(indicator_data_ptr),
                                                               INDICATOR_TIMER_FUNCTION, handle + 1);
  }
  else
  {
  indicator_timer_handle [handle] = FUIM_NO_FREE_TIMER_HANDLE;
  }

  return handle + 1;


}

void fuim_DestroyIndicator(osdDialogHandle indicator )
{

  fuimIndicatorStruct   *indicator_data_ptr;
  Byte  row, col;

  if (indicator == 0 || indicator > FUIM_MAX_INDICATORS)
  {
    return ;
  }

  indicator--;

  indicator_data_ptr = fuim_Indicators[ indicator ] ;

    row = fuim_GetIndicatorVertLocation (indicator_data_ptr);
    col = fuim_GetIndicatorHorLocation (indicator_data_ptr);
    plt_CCSetPosition( row, col );

  fuim_DestroyIndicatorField ((fuimFieldStruct  *)indicator_data_ptr->Field , (fuim_IndicatorProperty *)&fuim_indicator_properties[ indicator ] );

  fuim_Indicators [indicator] = NULL ;

  if (indicator_timer_handle [indicator] != FUIM_NO_FREE_TIMER_HANDLE)
  {
        fuim_DestroyTimer (&indicator_timer_handle [indicator]);
  }

    return;
}


/******************************************************************************
Проверяет параметр и обновляет indicator.

Возвращаемое значение :   Нет
Передаваемый параметр :   handle -
                            обработчик ссылается к определенному indicator-у
                          restart_timer - булевское значение
                             если timer должен быть перезапущен
******************************************************************************/

void fuim_UpdateIndicator (osdDialogHandle handle, Bool restart_timer)

{

  fuimIndicatorStruct   *indicator_data_ptr;
  fuimFieldStruct       *field_data_ptr;

//  fuim_IndicatorProperty  XDATA *position;

  if( (handle == 0) || (handle > FUIM_MAX_INDICATORS) || ( fuim_Indicators[ handle-1 ] == NULL) )
  {
    return ;
  }

  handle--;

  indicator_data_ptr = fuim_Indicators [handle];
  field_data_ptr = (fuimFieldStruct *)indicator_data_ptr->Field;

//  position = &fuim_indicator_properties[handle];

  fuim_SetRowPosition( fuim_GetIndicatorVertLocation (indicator_data_ptr) );

   if(restart_timer)
   {
        fuim_ConstructIndicatorField ( field_data_ptr, &fuim_indicator_properties[handle] );
   }
   else
   {
      if(field_data_ptr ->TimeOut != FUIM_FIELD_NO_TIMEOUT)
      {

          fuim_ConstructIndicatorField ( field_data_ptr, &fuim_indicator_properties[handle] );
      }
   }

    if (restart_timer)
    {
        if (fuim_Timer[FUIM_PERIODIC_TIMER].TimeOut != RGEN_TIMER_STOPPED)
        {
        if (indicator_timer_handle[handle] != FUIM_NO_FREE_TIMER_HANDLE)
        {
          fuim_RestartTimer (indicator_timer_handle[handle],
                             fuim_GetIndicatorTimeout (indicator_data_ptr));
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

    for (i = FUIM_PERIODIC_TIMER + 1; i < FUIM_MAX_TIMERS; i++)
    {
        if (fuim_Timer[i].TimerID == EMPTY_TIMER)
        {
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
    return;
}

/*********************************************************************************
 

**********************************************************************************/

void fuim_RestartTimer (osdTimerHandle hTimer,  Byte TimeoutInSeconds )

{
    if (fuim_Timer[hTimer - 1].TimeOut > RGEN_TIMER_STOPPED)
    {
        fuim_Timer[hTimer - 1].TimeOut = TimeoutInSeconds * FUIM_TIMER_RESOLUTION;
    }
    return;
}


/*********************************************************************************


**********************************************************************************/

void fuim_DestroyIndicatorTimer(osdDialogHandle indicator )
{
	if (indicator == 0 || indicator > FUIM_MAX_INDICATORS)
	{
		return ;
	}

	indicator--;

	if (indicator_timer_handle [indicator] != FUIM_NO_FREE_TIMER_HANDLE)
	{
		
        fuim_DestroyTimer (&indicator_timer_handle [indicator]);
	
	}

    return;
	
} 


/*********************************************************************************


**********************************************************************************/
void PeriodicTimerExpired(void)
{

  fmnu_UpdateMenu();

  if(auph_GetState() == AU_DIRECT_STATE) {
    Byte i;

    for (i = 1; i <= FUIM_MAX_INDICATORS; i++)
        fuim_UpdateIndicator(i, FALSE);
  }

  return;
}

/* ============ */

/* @parm timer function ID */
/* @parm argument that can be passed to timer function */

/*
   @func    Calls an timer function

   @rdesc   None.
*/
/* *********************************************************************************/

void fuim_TimerFunction( Byte index,  osdDialogHandle hDialog )
{
  switch (index)
  {
    case EMPTY_TIMER:
      break;

    case PERIODIC_TIMER_FUNCTION:
      PeriodicTimerExpired();
    break;

    case INDICATOR_TIMER_FUNCTION:
      fuim_DestroyIndicator(hDialog);
    break;

    case MENU_TIMER_FUNCTION: {
      fmnu_RemoveCurrentMenu();
      auph_SetState(AU_DIRECT_STATE);
    }break;

    default:
      break;
  }

}


/*********************************************************************************


**********************************************************************************/
void fuim_UpdateTimers(void)
{
    osdTimerHandle  i;


    for (i = 0; i < FUIM_MAX_TIMERS; i++)
    {
        if (fuim_Timer[i].TimeOut == RGEN_TIMER_EXPIRED)
        {

    fuim_TimerFunction(fuim_Timer[i].TimerID, fuim_Timer[i].Parameter);

      if (i == FUIM_PERIODIC_TIMER)
      {
          fuim_Timer[FUIM_PERIODIC_TIMER].TimeOut = FUIM_PERIODIC_TIMEOUT;
      }
      else
      {
              fuim_Timer[i].TimeOut = RGEN_TIMER_STOPPED;
            fuim_Timer[i].TimerID = EMPTY_TIMER;
      }
        }
    }

   return;
}


void fuim_TurnOn ( void )
{

    SetupPeriodicTimer (FUIM_PERIODIC_TIMEOUT);
    return;
}

/*****************************************************************************

*****************************************************************************/
void fuim_TurnOff ( void )
{
    fuim_InitTimers();
}


/*****************************************************************************

*****************************************************************************/
void fuim_Init(void )
{

 // plt_SetDisplayMode (DISPLAY_MODE_CC);
//  pltstd_CCInit( FUIM_MAX_NR_OF_COLS, FUIM_MAX_NR_OF_ROWS);//, TRUE );
 // plt_CCSetOSDEnable(3);

  fuim_InitTimers();
  fmnu_InitMenus();
  fuim_InitIndicators();

}
/*********************************************************************************


**********************************************************************************/
Byte fuim_GetColumnPosition( void )

{
    Byte    posd_CCRow, posd_CCColumn;

    plt_CCGetPosition(&posd_CCRow, &posd_CCColumn);

  return( posd_CCColumn );
}


/*********************************************************************************


**********************************************************************************/
Byte fuim_GetRowPosition( void )
{
    Byte    posd_CCRow, posd_CCColumn;

    plt_CCGetPosition(&posd_CCRow, &posd_CCColumn);

    return posd_CCRow;
}
/*********************************************************************************


********************************************************************************
**/
void fuim_SetRowPosition( Byte row )
{
     plt_CCSetPosition( row, fuim_GetColumnPosition() );
}

/*********************************************************************************


**********************************************************************************/
void fuim_SetColumnPosition( Byte column )
{
  plt_CCSetPosition( fuim_GetRowPosition(), column );
}


/*********************************************************************************


**********************************************************************************/
void fuim_SetAttributes(  Byte Attributes )
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
void fuim_SetBackgroundColour(Byte BackgroundColour, Byte SetAt)
{
  if ( BackgroundColour != FUIM_COLOUR_TRANSPARENT )
  {
     plt_CCSetBoxBackground (TRUE, SetAt);
     plt_CCSetBackgroundColour (BackgroundColour, SetAt);
  }
  else
  {
     plt_CCSetBoxBackground (FALSE, SetAt);
  }

}


/*********************************************************************************


**********************************************************************************/
void fuim_SetIndicatorTimeOut (osdDialogHandle hDialog,  Byte TimeOutInSeconds  )

{

  hDialog --;

  if ( indicator_timer_handle [hDialog] == FUIM_NO_FREE_TIMER_HANDLE )
  {

    indicator_timer_handle [hDialog] = fuim_ConstructTimer (TimeOutInSeconds,INDICATOR_TIMER_FUNCTION, hDialog + 1);

   }

}

/*********************************************************************************


**********************************************************************************/

void fuim_SetNextRow( void)

{
    plt_CCSetPosition( fuim_GetRowPosition() + 1, fuim_GetColumnPosition() );
}
