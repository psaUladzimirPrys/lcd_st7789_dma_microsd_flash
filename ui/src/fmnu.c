/*
 * fmnu.c
 *
 *  Created on: 20 февр. 2026 г.
 *      Author: priss
 */
/*=======================================================================*/
/*        I N C L U D E S                                                */
/*=======================================================================*/
#include "app_log.h"
#include "app_assert.h"

#include "auim_mnu.h"
#include "img_storage.h"

#include "aukh.h"
#include "auph.h"
#include "fmnu.h"

#include "ccstd.h"
#include "fuim_obs.h"
#include "rbsc_api.h"

#include "fmnu_str.h"


/*=======================================================================*/
/* G L O B A L   R E F E R E N C E S                                     */
/*=======================================================================*/
#define HIDDEN_MENU_CODE_LENGTH  4
#define FIRST_DIGIT              0

/*===========================================================================*/
/*         G L O B A L   D A T A   D E C L A R A T I O N S                   */
/*===========================================================================*/


/*===========================================================================*/
/*    L O C A L   S Y M B O L    D E C L A R A T I O N S                     */
/*===========================================================================*/

static fmnu_MenuStruct  *fmnu_menu_data_ptr;       //Указатель на текущее активное Меню
static fuimFieldStruct  *fmnu_field_data_ptr;      //Указатель на массив полей текущего Меню
static fuimFieldStruct  *active_field_ptr;         //Указатель на текущее Поле
static Byte  active_index_menu;     //Индекс текушего активного Меню

/*****************************************************************************/
static fmnu_MenuProperty  MenuDialogProperties;
static              Byte  osd_row[ FUIM_MAX_DISPLAY_FIELDS ];
static   fuimFieldStruct * displayed_fields[ FUIM_MAX_DISPLAY_FIELDS ];

/* active_menu_handle будет сохранять handle текущего отображаемого и активного меню.
 NULL = нет активного меню */
osdDialogHandle fmnu_active_menu_handle ;
osdTimerHandle  field_timer_handle;
osdTimerHandle  fmnu_menu_timer_handle;

const Byte fmnu_str_Prefix[fmnu_str_PrefixLAST_STRING]=
{
     IMG_ID_A
    ,IMG_ID_A
    ,IMG_ID_A
};

const Byte fmnu_str_Suffix[fmnu_str_SuffixLAST_STRING]=
{
     IMG_ID_A
    ,IMG_ID_A
};

static Bool fmnu_KeyPassAlways;
static Byte fmnu_KeyPassCommand;
static Bool fmnu_BitStartWriteErase;

/*===========================================================================*/
/* L O C A L   F U N C T I O N S      P R O T O T T Y P E                    */
/*===========================================================================*/

void fuim_ProcessMenuAction(cmdKeyNumber action);

void fmnu_ChangeField(Bool direction );

void fmnu_ConstructMenuField(fuimFieldStruct  *field_data_ptr, Bool Highlighted );
void fmnu_UpdateValueField(fuimFieldStruct  *field_data_ptr, Bool Highlighted);

//void fmnu_ConstructSpaser(void);

void fmnu_ConstructTitle(Bool SelectPtr,  Byte width);
void fmnu_DestroyField(fmnu_MenuProperty *position);

void fmnu_ConstructList( fuimFieldStruct   *field_data_ptr );
void fmnu_ConstructValue( fuimFieldStruct  *field_data_ptr ,Bool Highlighted);

//void fmnu_ConstructFixedField( Bool SelectPtr , Bool Top);
//void fmnu_ConstructEmptyField(  Bool SelectPtr );


void fmnu_ConstructMenuPrompt(fuimFieldStruct *field_data_ptr, Byte value_position );
void fuim_ConstructNumeric(fuimFieldStruct  *field_data_ptr, Bool Highlighted );


void fmnu_DestroyMenu(void);
/*void fuim_DrawNumeric(char  *NumericCharacter,
                      osdFieldValue GetFunction,
                      Byte FieldScalingNumeric,
                      Byte FieldSizeNumeric,
                      Bool Highlighted ) ;*/


/*********************************************************************************
*
*
*
**********************************************************************************/
void fmnu_InitMenus( void )
{

  fmnu_menu_data_ptr      = NULL ;
  fmnu_active_menu_handle = FUIM_NO_ACTIVE_DIALOG ;
  fmnu_menu_timer_handle  = FUIM_NO_FREE_TIMER_HANDLE ;
  field_timer_handle      =  FUIM_NO_FREE_TIMER_HANDLE ;

  //fuim_ClearDigits();

  fmnu_BitStartWriteErase = FALSE;

  return;
}

/*********************************************************************************


**********************************************************************************/
void fmnu_ConstructValue( fuimFieldStruct  *field_data_ptr ,Bool Highlighted)
{

 Byte  Validity = fuim_ValidityFunction( field_data_ptr -> ValidityFunction);

  switch( field_data_ptr->Type )
  {

/*
    case FUIM_FIELDTYPE_NUMERIC:
      fuim_ConstructNumeric( field_data_ptr, Highlighted );
    break;
*/

    case FUIM_FIELDTYPE_LIST:
      fmnu_ConstructList( field_data_ptr );
      break;

    case FUIM_FIELDTYPE_STRING:
      fuim_ConstructString( field_data_ptr, Highlighted );
    break;

    case FUIM_FIELDTYPE_STRING_VALUE:
        fuim_DrawString( (img_storage_id_t) fuim_Observer (field_data_ptr->GetFunction));
    break;

    case FUIM_FIELDTYPE_NUMERIC_VALUE:
      fuim_ConstructNumeric( field_data_ptr, FALSE );
    break;
    default:
      break;

  }

  Validity = Validity;
}


/*********************************************************************************


**********************************************************************************/
void fmnu_UpdateValueField( fuimFieldStruct  *field_data_ptr, Bool Highlighted)
{
 // Byte  Count;

    fuim_SetColumnPosition((fmnu_menu_data_ptr -> ValuePos) + 2);

    //plt_CCSetForegroundColour(fuim_GetFieldPromptColour(field_data_ptr) -> ForeGndColour);

      fmnu_ConstructValue( field_data_ptr , Highlighted );

/////////////////////////////Last empty string////////////////////////////////////////

//  Count = (( fmnu_menu_data_ptr -> Xpos ) + ( fmnu_menu_data_ptr -> Width)) - fuim_GetColumnPosition();
//  fuim_DrawRepeatedCharacter(Count  ,' ');

}

/*********************************************************************************
*
*
*
*
**********************************************************************************/
void fmnu_ConstructMenuField(fuimFieldStruct  *field_data_ptr, Bool Highlighted )
{

// Byte character;

 Byte   MenuValidity;
 Byte   MenuXpos  = fmnu_menu_data_ptr ->Xpos;
 Byte   MenuYpos = 0;
 Byte   MenuWidth = fmnu_menu_data_ptr ->Width;
 Byte   MenuPromptPos = fmnu_menu_data_ptr ->PromptPos;
 Byte   MenuValuePos  = fmnu_menu_data_ptr ->ValuePos;
 Byte   Count;

#if 0
 Byte   cursor_char;
 Byte   Attribute;
 Byte   ForeGndColour;
 Byte   BackGndColour;
 Byte   AttributeHighLighted;
 Byte   ForeGndColourHighLighted;
 Byte   BackGndColourHighLighted;


       MenuYpos = fuim_GetRowPosition(); //Установка позиции меню по вертикали


      Attribute = fuim_GetFieldPromptColour(field_data_ptr) -> Attribute;
      ForeGndColour = fuim_GetFieldPromptColour( field_data_ptr) -> ForeGndColour  ;
      BackGndColour = fuim_GetFieldPromptColour( field_data_ptr) -> BackGndColour;

       AttributeHighLighted = Attribute;
       ForeGndColourHighLighted = ForeGndColour;
       BackGndColourHighLighted = BackGndColour;


      if( Highlighted )
      {
        AttributeHighLighted = fuim_GetFieldPromptColour(field_data_ptr) -> AttributeHighLighted;
        ForeGndColourHighLighted = fuim_GetFieldPromptColour( field_data_ptr) -> ForeGndHighLighted  ;
        BackGndColourHighLighted = fuim_GetFieldPromptColour( field_data_ptr) -> BackGndHighLighted;
        cursor_char =  FMNU_CHAR_CURSOR;
      }
      else
      {
        cursor_char = ' ';
      }
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    MenuValidity = fuim_ValidityFunction( field_data_ptr -> ValidityFunction);
  if( MenuValidity != FUIM_VALIDITY_NOTPRESENT)
    {
#if 0
        if(MenuValidity == FUIM_VALIDITY_GRAYEDOUT)
         {

            ForeGndColour = FUIM_GRAYED_OUT_COLOUR;
            ForeGndColourHighLighted = ForeGndColour;
         }
#endif
        ////////////////////////////////////////////////////////////////////////////////////////////////////
    /*************************************************************************************/

    ////////////////////////////////////////////Установка конца/////////////////////////////////////////
      fuim_SetBackgroundColour( FUIM_COLOUR_TRANSPARENT, TRUE );
      fuim_SetAttributes( FUIM_ATTRIBUTES_NONE );
      MY_plt_CCSetPosition(MenuYpos ,(MenuXpos+MenuWidth));

    /*************************************************************************************/
    //////////////////////////////////////////////Установка начала//////////////////////////////////////

     // plt_CCSetRowSize(  Attribute & (FUIM_ATTRIBUTES_DOUBLEHEIGHT + FUIM_ATTRIBUTES_DOUBLEWIDTH));//, fuim_GetRowPosition() );

      MY_plt_CCSetPosition(fuim_GetRowPosition(), 0);

      //plt_CCSetForegroundColour(ForeGndColour);
      //fuim_SetBackgroundColour(BackGndColour, FALSE);

      MY_plt_CCSetPosition(MenuYpos, MenuXpos );

      if(Highlighted)
      {

      //plt_CCSetForegroundColour(  ForeGndColour);
       //   fuim_SetAttributes(   Attribute);
      //    fuim_SetBackgroundColour(   BackGndColour, TRUE);
      MY_plt_CCSetPosition(MenuYpos, MenuXpos + 1 );

      fuim_SetColumnPosition(fuim_GetColumnPosition() + 1 );

     // plt_CCSetForegroundColour( ForeGndColourHighLighted);// - ? или наоборот

      //    plt_CCDrawChar( cursor_char );

      }
      else
      {

           plt_CCSetPosition(MenuYpos, MenuXpos + 1 );

       // plt_CCDrawChar(' ');
      // plt_CCDrawChar( cursor_char );

      }
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
   if( (field_data_ptr -> Type) == FUIM_FIELDTYPE_SEPARATOR)
    {
     ForeGndColourHighLighted = FUIM_COLOUR_15; /// Устанавливаем в другой банк цвет
    }



  fuim_SetAttributes( AttributeHighLighted);
  fuim_SetBackgroundColour( BackGndColourHighLighted,TRUE);

  MY_plt_CCSetPosition(fuim_GetRowPosition() |PLT_CC_SERIAL_0_AT,MenuPromptPos);

  fuim_SetColumnPosition( MenuPromptPos+1 );

  plt_CCSetForegroundColour( ForeGndColourHighLighted );

  fuim_SetAttributes( AttributeHighLighted);
  fuim_SetBackgroundColour( BackGndColourHighLighted,FALSE);
  MY_plt_CCSetPosition(fuim_GetRowPosition() |PLT_CC_SERIAL_1_AFTER,fuim_GetColumnPosition());
  fuim_SetColumnPosition( MenuPromptPos + 2 );

  plt_CCSetForegroundColour( ForeGndColourHighLighted );
#endif
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    fmnu_ConstructMenuPrompt(  field_data_ptr,   MenuValuePos );

     if( (field_data_ptr -> Type) != FUIM_FIELDTYPE_SEPARATOR)
      {

        fuim_SetColumnPosition(MenuValuePos);
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////


     //   fuim_SetAttributes( Attribute);
      //  fuim_SetBackgroundColour( BackGndColour,TRUE);

        MY_plt_CCSetPosition(fuim_GetRowPosition(),fuim_GetColumnPosition());

        fuim_SetColumnPosition( fuim_GetColumnPosition() + 1 );

      //  plt_CCSetForegroundColour( ForeGndColour );
      //  fuim_SetAttributes( Attribute);
      //  fuim_SetBackgroundColour( BackGndColour,FALSE);

        MY_plt_CCSetPosition(fuim_GetRowPosition() ,fuim_GetColumnPosition());

        fuim_SetColumnPosition( fuim_GetColumnPosition() + 1 );
        //plt_CCSetForegroundColour( ForeGndColour);


  /////////////////////////////////////////////////////////////////////////////////////////////////////////////


            fmnu_ConstructValue( field_data_ptr , Highlighted );
        }
    }
////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////


 Count = (MenuXpos + MenuWidth) - fuim_GetColumnPosition();
 // fuim_DrawRepeatedCharacter(Count  ,' ');

  MenuYpos = MenuYpos;
  MenuPromptPos = MenuPromptPos;
  Count = Count;
}

/*********************************************************************************


**********************************************************************************/
void fmnu_DestroyField(fmnu_MenuProperty   *position)
//fuimFieldStruct RDATA *field_data_ptr, fuim_MenuProperty XDATA *position)
{

Byte FirstPos,EndPos,length;

 FirstPos = position->FirstPos;
   EndPos = position->EndPos;

  MY_plt_CCSetPosition (fuim_GetRowPosition(), FirstPos);
  length = ( EndPos - FirstPos )  + 1;
  //fuim_DrawRepeatedCharacter (length, ' '); //Erase till end of ROWS

  fuim_SetColumnPosition(0);
  //fuim_DrawRepeatedCharacter( 1 ,' ');

  length = length;
}

/*********************************************************************************
*
*
*
**********************************************************************************/
void fmnu_DrawMenuFields(Bool bDraw)
{
  Byte i, visible_fields;
  Byte start, end;


    visible_fields = fuim_GetMenuVisibleFields(fmnu_menu_data_ptr);
  if( visible_fields != 0 )
  {
    if( MenuDialogProperties.ActiveFieldNr < (visible_fields / 2) )
    {
      start = 0 ;
      end   = visible_fields ;
    }
    else
    {
      start = MenuDialogProperties.ActiveFieldNr - (visible_fields / 2) ;
      end   = start + visible_fields;
      if (end > MenuDialogProperties.LastFieldNr)
      {
        end   = MenuDialogProperties.LastFieldNr ;
        start = end - visible_fields;
      }
    }
  }
  else
  {
    start = MenuDialogProperties.FirstFieldNr;
    end   = MenuDialogProperties.LastFieldNr;
  }


  MenuDialogProperties.StartFieldNr = start;

  fuim_SetRowPosition(  fmnu_menu_data_ptr -> Ypos );

  if(  fmnu_menu_data_ptr -> Title != FMNU_NONE_TITLE )
  {
        fuim_SetNextRow();
  }

  if ( fuim_GetFixedTopField(fmnu_menu_data_ptr) != 0 )
  {
    fuim_SetNextRow();
  }

  for( i=start; i<end; i++)
  {

        osd_row[ i ] = fuim_GetRowPosition();

//                MenuDialogProperties.CurrentFieldNr = i - start;

        //plt_CCSetPosition (osd_row[i],  fmnu_menu_data_ptr -> Xpos);

        if ( i == MenuDialogProperties.ActiveFieldNr )
            {
          if((displayed_fields[i] ->TimeOut != FUIM_FIELD_NO_TIMEOUT) || (bDraw) )
            {
            if(bDraw)
            {
              fmnu_ConstructMenuField( displayed_fields[i] ,   TRUE );
            }
            else
            {
              fmnu_UpdateValueField( displayed_fields[i], TRUE);
            }
          }
        }
        else
            {
          if((displayed_fields[i] ->TimeOut != FUIM_FIELD_NO_TIMEOUT) || (bDraw) )
            {
            if(bDraw)
            {
                fmnu_ConstructMenuField( displayed_fields[i] ,   FALSE );
            }
            else
            {
              fmnu_UpdateValueField( displayed_fields[i], FALSE);
            }
                    }
        }

        fuim_SetNextRow();

   }
    return;
}


/*****************************************************************************
*
*
*
*
******************************************************************************/
void fmnu_ChangeField(Bool direction )
{
  Byte   active_field_nr;
  Byte   field_status;
  Byte   loop;
    fuimFieldStruct *previous_active_field_ptr;

  active_field_nr = MenuDialogProperties.ActiveFieldNr;

//      MenuDialogProperties.CurrentFieldNr = MenuDialogProperties.ActiveFieldNr - MenuDialogProperties.StartFieldNr;
  fuim_SetRowPosition( osd_row[ active_field_nr ] );
        previous_active_field_ptr = active_field_ptr;

  loop = MenuDialogProperties.LastFieldNr + 1 ;

  do
  {
   active_field_nr = rbsc_ChangeControlAround( active_field_nr,
                                                      !direction,
                                                      MenuDialogProperties.LastFieldNr - 1,
                                                      0 );
  active_field_ptr = displayed_fields[ active_field_nr ] ;
  field_status     = fuim_ValidityFunction (active_field_ptr -> ValidityFunction);

  }while( ( field_status != FUIM_VALIDITY_SELECTABLE ) && (--loop!=0) );

  if( loop == 0 )
  {
    active_field_nr = MenuDialogProperties.ActiveFieldNr ;
    active_field_ptr = displayed_fields[ active_field_nr ] ;
  }

    if( previous_active_field_ptr != active_field_ptr )
    {
        fmnu_ConstructMenuField( previous_active_field_ptr, FALSE );
      MenuDialogProperties.ActiveFieldNr = active_field_nr ;

      if( fuim_GetMenuVisibleFields (fmnu_menu_data_ptr) != 0 )
      {
        fmnu_DrawMenuFields(TRUE);
      }
      else
      {

  //  MenuDialogProperties.CurrentFieldNr = MenuDialogProperties.ActiveFieldNr;
      fuim_SetRowPosition( osd_row[ active_field_nr ] );
      fmnu_ConstructMenuField( active_field_ptr, TRUE);

      }
    }

}


/*********************************************************************************
*
*
*
**********************************************************************************/



void fuim_ProcessMenuAction(cmdKeyNumber action)

{

  Byte   active_osd_row;
  fuimDialogNavigation   *ActionPtr;
  Byte   new_action;

  active_osd_row = osd_row[ MenuDialogProperties.ActiveFieldNr ] ;

  if (fmnu_active_menu_handle == FUIM_NO_ACTIVE_DIALOG)
  {
    return ;//action ;
  }

    if (active_field_ptr != NULL)
        {
        ActionPtr = active_field_ptr->ToDoWithKey ;

      if( ActionPtr != NULL )
      {
         do
         {
            if( ActionPtr->Action == action )
          {
                new_action = fuim_ActionHandler (ActionPtr->DialogFunction, action);
            if ( new_action != AU_KEY_PROCESSED )
                                                  {
              action = new_action;
              }
                                                  else
                                                  {
                                                     action = AU_KEY_PROCESSED;
                                                  }
              }
             if( ActionPtr->Action != AU_KEY_INVALID )  ActionPtr++;
         }
         while( ActionPtr->Action != AU_KEY_INVALID );
      }
    }

    active_osd_row = active_osd_row;

}
/*********************************************************************************
*
*
*
**********************************************************************************/

void fmnu_ConstructTitle( Bool SelectPtr,  Byte MenuWidth)

{
 Byte MenuXpos, MenuYpos;
 Byte MenuAttribute;
 Byte Title = FMNU_NONE_TITLE;

  if(SelectPtr)
   {
     MenuXpos = fmnu_menu_data_ptr->Xpos;
     MenuYpos = fmnu_menu_data_ptr->Ypos;
     Title = fmnu_menu_data_ptr ->Title;
     MenuAttribute = fmnu_menu_data_ptr -> TitleAttribute;
   }
   else
   {
       app_log("MNU Title: ERR\r\n");
   }

   if ( Title != FMNU_NONE_TITLE)
   {
       fuim_DrawTitle(Title,  MenuWidth, MenuAttribute, FALSE);
   }

   MenuXpos = MenuXpos;
   MenuYpos = MenuYpos;
}
 /*********************************************************************************
 *
 *
 *
 **********************************************************************************/
 void fmnu_DestroyTitle( Bool SelectPtr,  Byte MenuWidth)
 {
  Byte MenuXpos, MenuYpos;
  Byte MenuAttribute;
  Byte Title = FMNU_NONE_TITLE;

   if(SelectPtr)
    {
      MenuXpos = fmnu_menu_data_ptr->Xpos;
      MenuYpos = fmnu_menu_data_ptr->Ypos;
      Title = fmnu_menu_data_ptr ->Title;
      MenuAttribute = fmnu_menu_data_ptr -> TitleAttribute;
    }
    else
    {
        app_log("MNU Title: ERR\r\n");
    }

    if ( Title != FMNU_NONE_TITLE)
    {
        fuim_DrawTitle(Title,  MenuWidth, MenuAttribute, TRUE);
    }


    MenuXpos = MenuXpos;
    MenuYpos = MenuYpos;

 }


 /*************************************************************************************************************/
 /*                                                                                                           */
 /*                                                                                                           */
 /*************************************************************************************************************/
 void fmnu_ConstructList( fuimFieldStruct   *field_data_ptr )
 {
   Byte   value;
   fmnu_ListStruct const * pList;

   value = (Byte)fuim_Observer(field_data_ptr->GetFunction);
   pList = field_data_ptr->FieldCharacters.ListItem;


   while (pList->ListItem != FMNU_LIST_ITEMS_ERROR)
   {
     if (value == pList->Value)
     {
       fuim_DrawString ((img_storage_id_t)pList->ListItem);
       break;
     }
     pList++;
   }
 }

/*************************************************************************************************************/
/*                                                                                                           */
/*                                                                                                           */
/*************************************************************************************************************/




void fmnu_ConstructMenu( fmnu_MenuStruct *Menu)
{
Bool first_field_set = FALSE;
Bool active_field_set = FALSE;
Byte i = 0;
Byte field_status;


  if(  fmnu_active_menu_handle == FUIM_NO_ACTIVE_DIALOG )
  {

    fmnu_active_menu_handle = 1 ;
  }
  else
  {
    return; //0;
  }


 fmnu_menu_data_ptr  = Menu;
 fmnu_field_data_ptr = (fuimFieldStruct  *)Menu -> MenuField ;

  MenuDialogProperties.FirstPos              = Menu -> Xpos;
  MenuDialogProperties.PromptPos             = Menu -> PromptPos;
  MenuDialogProperties.ValuePos              = Menu -> ValuePos;
  MenuDialogProperties.EndPos                = (Menu -> Width) + (Menu -> Xpos);
  MenuDialogProperties.FieldWidth            = Menu -> Width;

  MenuDialogProperties.FirstFieldNr          = 0;
  MenuDialogProperties.ActiveFieldNr         = 0;
  MenuDialogProperties.StartFieldNr        = 0;

  MenuDialogProperties.LastFieldNr   = 0;

  for (i = 0; i < (fmnu_menu_data_ptr->FieldCount); i++)
  {
    field_status = fuim_ValidityFunction (fmnu_field_data_ptr -> ValidityFunction);
    if( field_status != FUIM_VALIDITY_NOTPRESENT)
    {

      displayed_fields[ MenuDialogProperties.LastFieldNr++ ] = fmnu_field_data_ptr ;

      if( !first_field_set )
      {
        MenuDialogProperties.FirstFieldNr = i ;
        first_field_set = TRUE ;
      }

      if( (field_status == FUIM_VALIDITY_SELECTABLE ) && !active_field_set )// && fmnu_field_data_ptr->Prompt > 0)
      {
        MenuDialogProperties.ActiveFieldNr = i ;
        active_field_set = TRUE ;
      }
    }

    fmnu_field_data_ptr++;
  }

  if( !first_field_set )
  {
    fmnu_menu_data_ptr = NULL ;
    fmnu_active_menu_handle = FUIM_NO_ACTIVE_DIALOG ;
  }
  else
  {
      if( !active_field_set )
      {
        MenuDialogProperties.ActiveFieldNr = MenuDialogProperties.FirstFieldNr ;
      }

      active_field_ptr = displayed_fields [MenuDialogProperties.ActiveFieldNr];

/*     if( fmnu_menu_data_ptr == &auim_Menu[AUIM_MNU_INDEX_MAIN_MENU])
      {
         grf_ClutSetMenuColours(); //Смена таблицы цветов
      }
      else
      {
         Clut_SetTableEntry(0x6, 0x0, 0xF, 0xF); //Смена ввода одной позиции CLUTa
      }*/

      if(fmnu_menu_data_ptr ->Title != FMNU_NONE_TITLE) {
        fmnu_ConstructTitle( TRUE , MenuDialogProperties.FieldWidth );
             // fuim_SetNextRow(); //Обязятельно нужно т к в функции fmnu_ConstructFixedField() //хоть и происходит пересчет позиции верха но если она не вызывается - теряется позиция по вертикали
      }
#if 0
      if( fuim_GetFixedTopField(fmnu_menu_data_ptr) != 0 ) {
        fmnu_ConstructFixedField(TRUE, TRUE );
      }
#endif
       fmnu_DrawMenuFields(TRUE);

#if 0
      if ( fuim_GetFixedBottomField(fmnu_menu_data_ptr) != 0 ) {
          fmnu_ConstructFixedField(TRUE, FALSE );
      }
#endif

#if 0
       if( fmnu_menu_data_ptr == &auim_Menu[AUIM_MNU_INDEX_MAIN_MENU])
       {
           if(!fosd_IsOsdLogoDisplayed())
           {
            fosd_DrawLogo();
         }
       }
#endif
       if (fuim_GetActiveMenuTimeout(fmnu_menu_data_ptr) > 0)
       {
            fmnu_menu_timer_handle = fuim_ConstructTimer (fuim_GetActiveMenuTimeout(fmnu_menu_data_ptr), MENU_TIMER_FUNCTION, fmnu_active_menu_handle);
       }
       else
       {
            fmnu_menu_timer_handle = FUIM_NO_FREE_TIMER_HANDLE;
       }

    }

}



/*************************************************************************************************************/
/*                                                                                                           */
/*                                                                                                           */
/*************************************************************************************************************/
void fmnu_DestroyMenu(void)
{

//Byte   Movement = FALSE;
Byte   i = 0;
Byte   visiblefields;
Byte   row;
Byte   col;

  if ( ( fmnu_active_menu_handle == 0 ) || ( fmnu_active_menu_handle > FUIM_MAX_DISPLAY_MENUS ) )
  {
    return ;
  }

  if (fmnu_menu_data_ptr == NULL)
  {
    return;
  }



  row = fmnu_menu_data_ptr -> Ypos;
  col = fmnu_menu_data_ptr -> Xpos;

   plt_CCSetPosition( row, col );

    if( fmnu_menu_data_ptr ->Title != FMNU_NONE_TITLE )
    {
        fmnu_DestroyTitle( TRUE , MenuDialogProperties.FieldWidth );
    }

    if ( fuim_GetFixedTopField(fmnu_menu_data_ptr) != 0 )
    {
        fmnu_DestroyField( &MenuDialogProperties );
    }

    if( fuim_GetMenuVisibleFields(fmnu_menu_data_ptr) == 0)
    {
        //visiblefields = fmnu_menu_data_ptr -> FieldCount; - неправильно!!!!!!
        visiblefields = MenuDialogProperties.LastFieldNr;
    }
    else
    {
        visiblefields = fuim_GetMenuVisibleFields(fmnu_menu_data_ptr);
    }

  for (i = MenuDialogProperties.StartFieldNr; i < (MenuDialogProperties.StartFieldNr + visiblefields); i++)
  {
    row = osd_row[ i ] ;
    plt_CCSetPosition( row , col );
    fmnu_DestroyField( &MenuDialogProperties );
  }

    if ( fuim_GetFixedBottomField(fmnu_menu_data_ptr) != 0 )
    {
       /*plt_CCSetPosition( */++row;//, col );
       fmnu_DestroyField( &MenuDialogProperties );
    }
#if 0
    if( fmnu_menu_data_ptr == &auim_Menu[AUIM_MNU_INDEX_MAIN_MENU])
    {
      if(!fosd_IsOsdLogoDisplayed())
      {

                fosd_RemoveLogo();

      }
    }
#endif


  if (fmnu_menu_timer_handle != FUIM_NO_FREE_TIMER_HANDLE)
  {
        fuim_DestroyTimer (&fmnu_menu_timer_handle);
  }

  fmnu_menu_data_ptr = NULL;
  fmnu_active_menu_handle = FUIM_NO_ACTIVE_DIALOG ;


    //drv_SetTextAreaEnd(Movement);

}

/************************************************************************************************************
   @func   Remove current menu from screen.

   @rdesc  Nothing is returned by this function

   @comm   Function belongs to interface: AUIM

   @comm   Pre condition  :

   @comm   Post condition :

**************************************************************************************************************/
void fmnu_RemoveCurrentMenu(void)
{

   fmnu_DestroyMenu();
   active_index_menu = AUIM_MNU_MAX_MENUS; //не указывает ни на какое активное меню

}
/**********************************************************************************



***********************************************************************************/
void fmnu_Activate(menu_index_enum IndexMenu)
{
    fmnu_RemoveCurrentMenu();
    fmnu_InitMenus();

    active_index_menu = IndexMenu;

    fmnu_ConstructMenu((fmnu_MenuStruct *)&auim_Menu[IndexMenu]);

}


/**********************************************************************************



***********************************************************************************/
void fmnu_UpdateMenu(void)
{

  if ( fmnu_active_menu_handle != FUIM_NO_ACTIVE_DIALOG ) {
    if (fmnu_menu_data_ptr != NULL) {

        fmnu_DrawMenuFields(FALSE);
    }
  }

}

/****************************************************************************
  @func   This function can be used to enforce FMNU to process a specific
            command.

  @parm   command: The command sent to FMNU

  @rdesc  Function returns TRUE if the command is processed, FALSE
            otherwise

  @comm   Function belongs to component: FMNU

*****************************************************************************/
void fmnu_HandleCommand(void)
{

   Bool pass_command;
   Byte  field_Repeat, field_type, command;

   pass_command = FALSE;

   command = aukh_GetCurrentCommand();

   field_type = active_field_ptr->Type;

      if (aukh_FirstKeyPress())
      {
            pass_command = TRUE;
            fmnu_KeyPassAlways = FALSE;
      }
      else if(fmnu_KeyPassAlways )
      {
          if(fmnu_KeyPassCommand == command)
          {
            pass_command = TRUE;
          }
          else
          {
             fmnu_KeyPassAlways = FALSE;
          }
      }
      else
      {
       switch (command)
        {
           case AU_KEY_MENU: {
              field_Repeat = active_field_ptr -> Alignment;

               if(field_Repeat == FUIM_REPEAT_KEY_ALWAYS) //Или нет специального указания на время одиночного удержания
               {

                      if ( aukh_KeyHold(AU_KEY_PRESSED_ONE_SECOND) ) // Но если она еще и удерживалась
                      {  //две минуты  - то пусть повторяется пока удерживают клавишу
                         fmnu_KeyPassAlways = TRUE;
                         fmnu_KeyPassCommand = command;
                      }
                 }
                 else if (aukh_KeyHold(field_Repeat ) )
                 {
                  /*  Сбросится этот повтор на еще раз только через время 2ч33м01с */
                  pass_command = TRUE;
                 }
           } break;


           default:
             break;
        }


     }

   if(pass_command)
   {
      if((active_index_menu != AUIM_MNU_MAX_MENUS))
       {
         fuim_ProcessMenuAction(command);
       }
   }

   field_type = field_type;

}


/**************************************************************************************


**************************************************************************************/
void fmnu_ConstructMenuPrompt(fuimFieldStruct  *field_data_ptr, Byte value_position )
{


 Byte  Prompt = field_data_ptr -> Prompt;

 value_position = value_position;

 if(( Prompt ) !=  FMNU_NONE_PROMPT)
 {
     fuim_DrawString( (img_storage_id_t)Prompt);
 }

/*  if (fuim_GetColumnPosition() < (value_position))
  {
    fuim_DrawRepeatedCharacter(value_position - fuim_GetColumnPosition(),' ');
  } */

}

void fuim_ConstructNumeric(fuimFieldStruct  *field_data_ptr,
                           Bool Highlighted )

{

Byte Prefix,Suffix;

Prefix = field_data_ptr->Prefix;
Suffix = field_data_ptr->Suffix;

 if(Prefix > 0)
 {

   if(fuim_Observer(field_data_ptr->GetFunction) == 0 )
   {
       fuim_DrawString ( fmnu_str_Prefix[FMNU_PREFIX_NONE ] );
   }
   else
   {
       fuim_DrawString ( fmnu_str_Prefix[Prefix]);
   }

 }

 Highlighted = Highlighted;
 /* fuim_DrawNumeric(field_data_ptr->FieldCharacters.NumericCharacter,
                   field_data_ptr->GetFunction,
                   field_data_ptr->FieldScaling.Numeric,
                   field_data_ptr->FieldSize.Numeric,
                   Highlighted );
*/
 if(Suffix > 0 )
 {

    fuim_DrawString (fmnu_str_Suffix[Suffix]);

 }
}

/*********************************************************************************


**********************************************************************************/
/*
void fuim_DrawNumeric(char  *NumericCharacter,
                      osdFieldValue GetFunction,
                      Byte FieldScalingNumeric,
                      Byte FieldSizeNumeric,
                      Bool Highlighted )

{
    Byte   Digits, Decimals, Strlen, i ;
    Byte   string [ FUIM_MAX_NUMERIC_LENGTH + 1 ];
    Byte   tempstr[ FUIM_MAX_NUMERIC_LENGTH + 2 ];

  if( Highlighted && (fuim_GetNrOfDigits() != 0 ))
  {

    fuim_GetDigitBufferString( string , NumericCharacter[1] );
    Strlen = FieldSizeNumeric;
  }
  else
  {
    fuim_itoa(fuim_Observer(GetFunction) , string );
    Strlen = fuim_GetStringLength (string);
  }


  Decimals = FieldScalingNumeric;
  Digits   = FieldSizeNumeric;

  if (Decimals == 0)
  {



    string [Digits] = '\0';
    fuim_DrawString ( string );


      if (Strlen < Digits)
      {
        fuim_DrawRepeatedCharacter (Digits - Strlen, ' ');
      }
  }
  else
  {



    for (i = 0; i < Digits - Decimals - 1; i++)
    {
      tempstr[i] = ' ' ;
    }


    for ( ; i < Digits + 1; i++)
    {
      tempstr[i] = '0';
    }


    tempstr [Digits - Decimals] = NumericCharacter[0];

    tempstr [i] = '\0';


    while( Strlen > 0 )
    {
      i--;
      if ( i == Digits - Decimals )
      {
        i--;
      }
      tempstr[i] = string[--Strlen] ;
    }
    fuim_DrawString( tempstr );
  }
} */
