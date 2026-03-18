/*
 * fmnu.c
 *
 *  Created on: 20 февр. 2026 г.
 *      Author: priss
 */
/*=======================================================================*/
/*        I N C L U D E S                                                */
/*=======================================================================*/
#include <pltccstd.h>
#include "app_log.h"
#include "app_assert.h"

#include "auim_mnu.h"
#include "img_storage.h"

#include "aukh.h"
#include "auph.h"
#include "fmnu.h"

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


static fmnu_MenuStruct *fmnu_menu_data_ptr;  //Pointer to the currently active menu
static fuimFieldStruct *fmnu_field_data_ptr; //Pointer to the array of fields in the current menu
static fuimFieldStruct *active_field_ptr;    //Pointer to the current field
static Byte active_index_menu;               //Index of the current active menu
/*****************************************************************************/
static fmnu_MenuProperty  MenuDialogProperties;
static              Word  osd_row[ FUIM_MAX_DISPLAY_FIELDS ];
static   fuimFieldStruct * displayed_fields[ FUIM_MAX_DISPLAY_FIELDS ];

/* active_menu_handle will store the handle of the currently displayed and active menu.
NULL = no active menu */
osdDialogHandle fmnu_active_menu_handle ;
osdTimerHandle  field_timer_handle;
osdTimerHandle  fmnu_menu_timer_handle;



static Bool fmnu_KeyPassAlways;
static Byte fmnu_KeyPassCommand;


/*===========================================================================*/
/* L O C A L   F U N C T I O N S      P R O T O T T Y P E                    */
/*===========================================================================*/

void fuim_ProcessMenuAction(cmdKeyNumber action);

void fmnu_ChangeField(Bool direction );

void fmnu_ConstructMenuField(fuimFieldStruct *field_data_ptr, Bool Highlighted);
void fmnu_UpdateValueField(  fuimFieldStruct *field_data_ptr, Bool Highlighted);

//void fmnu_ConstructSpaser(void);

void fmnu_ConstructTitle(Word MenuWidth);
void fmnu_DestroyTitle  (Word MenuWidth);

void fmnu_DestroyField(void); //(fmnu_MenuProperty *position);

void fmnu_ConstructList( fuimFieldStruct *field_data_ptr );
void fmnu_ConstructValue(fuimFieldStruct *field_data_ptr, Bool Highlighted);

void fmnu_ConstructButtonField(Bool left);
void fmnu_DestroyButtonField(Bool left);

//void fmnu_ConstructEmptyField(  Bool SelectPtr );


void fmnu_ConstructMenuPrompt(fuimFieldStruct *field_data_ptr, Word value_position);




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
      fuim_ConstructString( field_data_ptr, Highlighted );
    break;

    case FUIM_FIELDTYPE_NUMERIC:
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
  Word   MenuXpos  = fmnu_menu_data_ptr ->Xpos;
  Word   MenuValuePos  = fmnu_menu_data_ptr ->ValuePos;
  Word   MenuWidth = fmnu_menu_data_ptr ->Width;

  Byte  Count;

  Word  ForeGndColour = 0;
  Word  BackGndColour = 0;
  Word  Attribute = 0;

  fuim_Validity   Validity;

  Attribute = fuim_GetFieldPromptColour(field_data_ptr) -> Attribute;
  ForeGndColour = fuim_GetFieldPromptColour( field_data_ptr) -> ForeGndColour;
  BackGndColour = fuim_GetFieldPromptColour( field_data_ptr) -> BackGndColour;

  Validity = fuim_ValidityFunction( field_data_ptr -> ValidityFunction);
  if( Validity != FUIM_VALIDITY_NOTPRESENT)
  {

    plt_CCSetRowSize(Attribute & FUIM_ATTRIBUTES_ROW_SIZE);

    if( (field_data_ptr -> Type) != FUIM_FIELDTYPE_SEPARATOR)
    {
       fuim_SetColumnPosition(MenuValuePos);

       plt_CCSetForegroundColour(ForeGndColour);
       plt_CCSetBackgroundColour(BackGndColour);

       fmnu_ConstructValue(field_data_ptr, Highlighted);
     }
  }

  Count = (MenuXpos + MenuWidth) > fuim_GetColumnPosition() ? ((MenuXpos + MenuWidth) - fuim_GetColumnPosition()) :(0);
  fuim_DrawRepeatedCharacter(Count);

}

/*********************************************************************************
*
*
*
*
**********************************************************************************/
void fmnu_ConstructMenuField(fuimFieldStruct  *field_data_ptr, Bool Highlighted )
{

 Word   MenuYpos      = 0;
 Word   MenuXpos      = fmnu_menu_data_ptr -> Xpos;
 Word   MenuWidth     = fmnu_menu_data_ptr -> Width;
 Word   MenuPromptPos = fmnu_menu_data_ptr -> PromptPos;
 Word   MenuValuePos  = fmnu_menu_data_ptr -> ValuePos;

 Word   Count;
 Word   Attribute;
 Word   ForeGndColour;
 Word   BackGndColour;
 Byte   MenuValidity;


  MenuYpos = fuim_GetRowPosition(); //Установка позиции меню по вертикали

  Attribute = fuim_GetFieldPromptColour(field_data_ptr) -> Attribute;
  ForeGndColour = fuim_GetFieldPromptColour( field_data_ptr) -> ForeGndColour;
  BackGndColour = fuim_GetFieldPromptColour( field_data_ptr) -> BackGndColour;

  MenuValidity = fuim_ValidityFunction( field_data_ptr -> ValidityFunction);
  if( MenuValidity != FUIM_VALIDITY_NOTPRESENT)
  {

      plt_CCSetRowSize(Attribute & FUIM_ATTRIBUTES_ROW_SIZE);

      plt_CCSetForegroundColour(ForeGndColour);
      plt_CCSetBackgroundColour(BackGndColour);

      plt_CCSetPosition(MenuYpos, MenuPromptPos );

     fmnu_ConstructMenuPrompt(field_data_ptr, MenuValuePos);

     if( (field_data_ptr -> Type) != FUIM_FIELDTYPE_SEPARATOR)
     {

       fuim_SetColumnPosition(MenuValuePos);

       fuim_SetAttributes( Attribute);
       plt_CCSetForegroundColour( ForeGndColour );
       plt_CCSetBackgroundColour( BackGndColour);

       fmnu_ConstructValue( field_data_ptr, Highlighted );
     }
 }

 plt_CCSetRowSize(Attribute & FUIM_ATTRIBUTES_ROW_SIZE);
 Count = (MenuXpos + MenuWidth) > fuim_GetColumnPosition() ? ((MenuXpos + MenuWidth) - fuim_GetColumnPosition()) :(0);
 fuim_DrawRepeatedCharacter(Count);


}

/*********************************************************************************
 displayed_fields[i]

**********************************************************************************/
//void fmnu_DestroyField(fmnu_MenuProperty   *position)
//fuimFieldStruct RDATA *field_data_ptr, fuim_MenuProperty XDATA *position)
void fmnu_DestroyField(void)
{
/*

Byte   FirstPos,EndPos,length;

Word   Attribute;
Word   ForeGndColour;
Word   BackGndColour;

Attribute = fuim_GetFieldPromptColour(field_data_ptr) -> Attribute;
ForeGndColour = fuim_GetFieldPromptColour( field_data_ptr) -> ForeGndColour;
BackGndColour = fuim_GetFieldPromptColour( field_data_ptr) -> BackGndColour;



 FirstPos = position->FirstPos;
   EndPos = position->EndPos;

   plt_CCSetRowSize(10 & FUIM_ATTRIBUTES_ROW_SIZE);

   plt_CCSetForegroundColour(ST7789_MAGENTA);
   plt_CCSetBackgroundColour(ST7789_MAGENTA);

  plt_CCSetPosition (fuim_GetRowPosition(), FirstPos);
  length = ( EndPos - FirstPos )  + 1;
  fuim_DrawRepeatedCharacter (length); //Erase till end of ROWS
*/


  plt_CCSetRowSize(FUIM_MENU_HEIGHT - (FUIM_TOP_ROW_SIZE + FUIM_BOTTOM_ROW_SIZE) );
  plt_CCSetBackgroundColour(ST7789_WHITE); //TEST UP
  plt_CCSetPosition (FUIM_TOP_ROW_SIZE, 0);

  fuim_DrawRepeatedCharacter (FUIM_MENU_WIDTH); //Erase till end of ROWS


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

  fuim_SetRowPosition(fmnu_menu_data_ptr -> Ypos + FUIM_MENU_FIELD_TOP_MARGIN);

  for( i = start; i < end; i++)
  {

       osd_row[ i ] = fuim_GetRowPosition();

       plt_CCSetPosition (osd_row[i],  fmnu_menu_data_ptr -> Xpos);

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
        }else
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

  Word                    active_osd_row;
  fuimDialogNavigation   * ActionPtr;
  Byte                    new_action;

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
            if( ActionPtr->Action == action ) {

              new_action = fuim_ActionHandler (ActionPtr->DialogFunction, action);
              if ( new_action != AU_KEY_PROCESSED ) {
                 action = new_action;
              } else {
                 action = AU_KEY_PROCESSED;
              }
           }

           if( ActionPtr->Action != AU_KEY_INVALID )  ActionPtr++;

         } while( ActionPtr->Action != AU_KEY_INVALID );
      }
  }

  switch(action)
  {
    case AU_KEY_YES:
    {
#if 0
      fuim_Transformer (active_field_ptr->ChangeFunction, (action==AU_KEY_YES) ? RGEN_CHANGE_DOWN: RGEN_CHANGE_UP);

      if(fmnu_active_menu_handle != FUIM_NO_ACTIVE_DIALOG)
      {
        active_osd_row = osd_row[ MenuDialogProperties.ActiveFieldNr ] ;
        plt_CCSetPosition( active_osd_row, fmnu_menu_data_ptr ->Xpos);
        fmnu_UpdateValueField(active_field_ptr ,TRUE);
      }
#endif
     action = AU_KEY_PROCESSED ;

    } break;

    case AU_KEY_NEXT:
    {
      fmnu_ChangeField( (action==AU_KEY_NEXT)?RGEN_CHANGE_UP:RGEN_CHANGE_DOWN );
      action = AU_KEY_PROCESSED ;

     } break;

    default:
      break;

  }


  active_osd_row = active_osd_row;

}
/*********************************************************************************
*
*
*
**********************************************************************************/
void fmnu_ConstructTitle(Word MenuWidth)
{

  Word MenuXpos, MenuYpos;
  Byte MenuAttribute;
  Byte Title = FMNU_NONE_TITLE;

  MenuXpos      = fmnu_menu_data_ptr -> Xpos;
  MenuYpos      = fmnu_menu_data_ptr -> Ypos;
  Title         = fmnu_menu_data_ptr -> Title;
  MenuAttribute = fmnu_menu_data_ptr -> TitleAttribute;

  fuim_SetAttributes(FUIM_ATTRIBUTES_NONE);

  plt_CCSetBackgroundColour(fuim_DynamicColours(MenuAttribute)->BackGndColour);
  plt_CCSetForegroundColour(fuim_DynamicColours(MenuAttribute)->ForeGndColour);

  plt_CCSetRowSize(FUIM_TOP_ROW_SIZE);
  plt_CCSetPosition(MenuYpos, MenuXpos + FUIM_TITLE_LEFT_MARGIN);

  fuim_DrawRepeatedCharacter(MenuWidth - (MenuXpos + FUIM_TITLE_LEFT_MARGIN));

  if (Title != FMNU_NONE_TITLE)
  {
    plt_CCSetRowSize((fuim_DynamicColours(MenuAttribute)->Attribute) & (FUIM_ATTRIBUTES_ROW_SIZE));

    fuim_SetRowPosition(MenuYpos + FUIM_TITLE_TOP_MARGIN);
    fuim_SetColumnPosition(MenuWidth - (IMG_GET_WIDTH((img_storage_id_t )Title)  + FUIM_TITLE_RIGHT_MARGIN));
    fuim_DrawTitle((img_storage_id_t )Title);
  }

}

 /*********************************************************************************
 *
 *
 *
 **********************************************************************************/
void fmnu_DestroyTitle( Word MenuWidth)
{
  Word MenuXpos, MenuYpos;
  Byte MenuAttribute;
  Byte Title = FMNU_NONE_TITLE;

  Title = fmnu_menu_data_ptr ->Title;

  if ( Title != FMNU_NONE_TITLE)
  {
    MenuXpos      = fmnu_menu_data_ptr -> Xpos;
    MenuYpos      = fmnu_menu_data_ptr -> Ypos;
    MenuAttribute = fmnu_menu_data_ptr -> TitleAttribute;

    fuim_SetAttributes(FUIM_ATTRIBUTES_NONE);
    plt_CCSetBackgroundColour(fuim_DynamicColours(MenuAttribute)->BackGndColour);
    plt_CCSetForegroundColour(fuim_DynamicColours(MenuAttribute)->ForeGndColour);

    plt_CCSetRowSize(FUIM_TOP_ROW_SIZE);

    fuim_SetRowPosition(MenuYpos);

    fuim_SetColumnPosition(MenuWidth - (IMG_GET_WIDTH((img_storage_id_t)Title) + FUIM_TITLE_RIGHT_MARGIN));
    fuim_DrawRepeatedCharacter(IMG_GET_WIDTH((img_storage_id_t )Title));
  }

  (void)MenuXpos;
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
       fuim_DrawString ((img_storage_id_t)fmnu_str_List[pList->ListItem]);
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


  if(  fmnu_active_menu_handle == FUIM_NO_ACTIVE_DIALOG ) {
    fmnu_active_menu_handle = 1 ;
  } else {
    return; //0;
  }


 fmnu_menu_data_ptr  = Menu;
 fmnu_field_data_ptr = (fuimFieldStruct  *)Menu -> MenuField ;

  MenuDialogProperties.FirstPos          = Menu -> Xpos;
  MenuDialogProperties.PromptPos         = Menu -> PromptPos;
  MenuDialogProperties.ValuePos          = Menu -> ValuePos;
  MenuDialogProperties.EndPos            =(Menu -> Width) + (Menu -> Xpos);
  MenuDialogProperties.FieldWidth        = Menu -> Width;

  MenuDialogProperties.FirstFieldNr      = 0;
  MenuDialogProperties.ActiveFieldNr     = 0;
  MenuDialogProperties.StartFieldNr      = 0;
  MenuDialogProperties.LastFieldNr       = 0;

  for (i = 0; i < (fmnu_menu_data_ptr->FieldCount); i++)
  {
    field_status = fuim_ValidityFunction (fmnu_field_data_ptr -> ValidityFunction);
    if( field_status != FUIM_VALIDITY_NOTPRESENT)
    {

      displayed_fields[ MenuDialogProperties.LastFieldNr++ ] = fmnu_field_data_ptr ;

      if( !first_field_set ) {
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

  if( !first_field_set ) {
    fmnu_menu_data_ptr = NULL ;
    fmnu_active_menu_handle = FUIM_NO_ACTIVE_DIALOG ;
  } else
  {
      if( !active_field_set ) {
         MenuDialogProperties.ActiveFieldNr = MenuDialogProperties.FirstFieldNr ;
      }

      active_field_ptr = displayed_fields [MenuDialogProperties.ActiveFieldNr];

      fmnu_ConstructTitle(MenuDialogProperties.FieldWidth);

      fmnu_DrawMenuFields(TRUE);

      if( fuim_GetLeftButtonField(fmnu_menu_data_ptr) != 0 ) {
         fmnu_ConstructButtonField(TRUE);
      }

      if (fuim_GetRightBottonField(fmnu_menu_data_ptr) != 0) {
          fmnu_ConstructButtonField(FALSE);
      }

      if (fuim_GetActiveMenuTimeout(fmnu_menu_data_ptr) > 0) {
        fmnu_menu_timer_handle = fuim_ConstructTimer (fuim_GetActiveMenuTimeout(fmnu_menu_data_ptr), MENU_TIMER_FUNCTION, fmnu_active_menu_handle);
      } else {
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

 // Byte   i = 0;
 // Byte   visiblefields;
  Word   row;
  Word   col;

  if ( ( fmnu_active_menu_handle == 0 ) || ( fmnu_active_menu_handle > FUIM_MAX_DISPLAY_MENUS ) ) {
    return ;
  }

  if (fmnu_menu_data_ptr == NULL) {
    return;
  }

  row = fmnu_menu_data_ptr -> Ypos;
  col = fmnu_menu_data_ptr -> Xpos;

  plt_CCSetPosition( row, col );

  if( fmnu_menu_data_ptr ->Title != FMNU_NONE_TITLE )
  {
    fmnu_DestroyTitle(MenuDialogProperties.FieldWidth );
  }

//Old version remove line by line
#if 0
  if( fuim_GetMenuVisibleFields(fmnu_menu_data_ptr) == 0)
  {
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
#endif

  fmnu_DestroyField();

  if ( fuim_GetLeftButtonField(fmnu_menu_data_ptr) != 0 )
  {
      fmnu_DestroyButtonField(TRUE);
  }

  if ( fuim_GetRightBottonField(fmnu_menu_data_ptr) != 0 )
  {
     fmnu_DestroyButtonField(FALSE);
  }


  if (fmnu_menu_timer_handle != FUIM_NO_FREE_TIMER_HANDLE)
  {
    fuim_DestroyTimer (&fmnu_menu_timer_handle);
  }

  fmnu_menu_data_ptr = NULL;
  fmnu_active_menu_handle = FUIM_NO_ACTIVE_DIALOG;

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
   active_index_menu = AUIM_MNU_MAX_MENUS; //does not indicate any active menu
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

               if(field_Repeat == FUIM_REPEAT_KEY_ALWAYS) //Or there is no specific indication of the duration of a single detention
               {

                      if ( aukh_KeyHold(AU_KEY_PRESS_VERY_LONG) ) // But if is was still holding on
                      {  //two minutes - let it repeat while the key is held down
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
void fmnu_ConstructMenuPrompt(fuimFieldStruct  *field_data_ptr, Word value_position )
{

 Byte  Prompt = field_data_ptr -> Prompt;

 value_position = value_position;

 if(( Prompt ) !=  FMNU_NONE_PROMPT)
 {
   fuim_DrawString( (img_storage_id_t)Prompt);
 }

 if (fuim_GetColumnPosition() < (value_position))
 {
    fuim_DrawRepeatedCharacter(value_position - fuim_GetColumnPosition());
 }

}


/*********************************************************************************

**********************************************************************************/
void fmnu_ConstructButtonField(Bool left)
{

  fuimFieldStruct const * field_data_ptr;
  fuim_Validity            validity;


  Word  string_length = 0;
  Word  max_string_length = 0;

  Word  characters_left = 0;
  Word  characters_right = 0;

  Word  MenuXpos;
  Word  MenuYpos  = fmnu_menu_data_ptr -> Ypos + (FUIM_MENU_HEIGHT - FUIM_BOTTOM_FIELD_HEIGHT);
  Word  MenuWidth = fmnu_menu_data_ptr -> Width;

  Word  Attribute;
  Word  ForeGndColour;
  Word  BackGndColour;

  if ( left )
  {
      //Set the Left Button field properties
      field_data_ptr = fmnu_menu_data_ptr -> LeftButtonField;
      MenuXpos  = fmnu_menu_data_ptr -> Xpos;
  }
  else
  {   //Set the Right Button field properties
      field_data_ptr = fmnu_menu_data_ptr -> RightButtonField;
      MenuXpos  = fmnu_menu_data_ptr -> Xpos + (FUIM_MENU_WIDTH / 2);
  }


  Attribute     = fuim_GetFieldPromptColour(field_data_ptr) -> Attribute;
  ForeGndColour = fuim_GetFieldPromptColour(field_data_ptr) -> ForeGndColour;
  BackGndColour = fuim_GetFieldPromptColour(field_data_ptr) -> BackGndColour;


  plt_CCSetRowSize(Attribute & FUIM_ATTRIBUTES_ROW_SIZE);

  fuim_SetRowPosition(MenuYpos);
  fuim_SetColumnPosition(MenuXpos);

  plt_CCSetBackgroundColour(BackGndColour);
  plt_CCSetForegroundColour(ForeGndColour);

  fuim_DrawRepeatedCharacter( (FUIM_MENU_WIDTH / 2));

  validity = fuim_ValidityFunction(field_data_ptr-> ValidityFunction);
  if(validity == FUIM_VALIDITY_NOTPRESENT) {
      return;
  }

  max_string_length = (FUIM_MENU_WIDTH / 2);  //max_string_length -= 3; /* @ToDo Need to debug.  It was been added by UP*/

/*  if (field_data_ptr->GetFunction != 0) {
    string_length = 3;//fuim_GetStringLength( (Byte *) fuim_Observer(field_data_ptr->GetFunction) );
  } else {
    string_length = 0;
  }*/

  string_length = IMG_GET_WIDTH(field_data_ptr->Prompt) + FUIM_BUTTON_PROMPT_TO_NAME_PADDING + IMG_GET_WIDTH(field_data_ptr->FieldCharacters.Button);
  string_length = (string_length < max_string_length) ? (string_length) : (max_string_length);

  switch (field_data_ptr->Alignment)
  {
      case FUIM_ALIGNMENT_LEFT: {
         characters_left = 0;
         characters_right = max_string_length - string_length;
      } break;

      case FUIM_ALIGNMENT_CENTRE: {
        characters_left = ((max_string_length - string_length) / 2);
        characters_right = ((max_string_length - string_length + 1) / 2);
      }break;

      case FUIM_ALIGNMENT_RIGHT: {
        characters_left = max_string_length - string_length;
        characters_right = 0;
     } break;

  }

  fuim_SetColumnPosition(MenuXpos);
  fuim_DrawRepeatedCharacter(characters_left);

  fuim_SetRowPosition(MenuYpos + FUIM_BUTTON_PROMPT_TOP_MARGIN);
  fuim_DrawString( (img_storage_id_t) ( field_data_ptr->Prompt ));

  fuim_SetRowPosition(MenuYpos);
  fuim_DrawRepeatedCharacter(FUIM_BUTTON_PROMPT_TO_NAME_PADDING);


  fuim_SetRowPosition(MenuYpos + FUIM_BUTTON_NAME_TOP_MARGIN);
  fuim_DrawString( (img_storage_id_t) (field_data_ptr->FieldCharacters.Button));

  fuim_SetRowPosition(MenuYpos);
  fuim_DrawRepeatedCharacter(characters_right);


  MenuWidth = MenuWidth;

}


/*********************************************************************************

**********************************************************************************/
void fmnu_DestroyButtonField(Bool left)
{
  fuimFieldStruct const * field_data_ptr;

  Word  MenuXpos;
  Word  MenuYpos  = fmnu_menu_data_ptr -> Ypos + (FUIM_MENU_HEIGHT - FUIM_BOTTOM_FIELD_HEIGHT);

  Word  Attribute;
  Word  ForeGndColour;
  Word  BackGndColour;

  if ( left )
  {
      //Set the Left Button field properties
      field_data_ptr = fmnu_menu_data_ptr -> LeftButtonField;
      MenuXpos  = fmnu_menu_data_ptr -> Xpos;
  }
  else
  {   //Set the Right Button field properties
      field_data_ptr = fmnu_menu_data_ptr -> RightButtonField;
      MenuXpos  = fmnu_menu_data_ptr -> Xpos + (FUIM_MENU_WIDTH / 2);
  }

  Attribute     = fuim_GetFieldPromptColour(field_data_ptr) -> Attribute;
  ForeGndColour = fuim_GetFieldPromptColour(field_data_ptr) -> ForeGndColour;
  BackGndColour = fuim_GetFieldPromptColour(field_data_ptr) -> BackGndColour;


  plt_CCSetRowSize(Attribute & FUIM_ATTRIBUTES_ROW_SIZE);

  fuim_SetRowPosition(MenuYpos);
  fuim_SetColumnPosition(MenuXpos);

  plt_CCSetBackgroundColour(BackGndColour);
  plt_CCSetForegroundColour(ForeGndColour);

  fuim_DrawRepeatedCharacter(FUIM_MENU_WIDTH / 2);

}
