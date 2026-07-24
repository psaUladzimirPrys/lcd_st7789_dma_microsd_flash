/*
 * fmnu.c
 *
 *  Created on: 20 Feb. 2026
 *      Author: priss
 */
/*=======================================================================*/
/*        I N C L U D E S                                                */
/*=======================================================================*/
#include <pltccstd.h>
#include "app_log.h"
#include "app_assert.h"


#include "auim_mnu.h"
#include "auim_api.h"
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

//#define FUIM_DESTROY_TITLE
//#define FUIM_DESTRTOY_BUTTON_FIELDS
/*===========================================================================*/
/*         G L O B A L   D A T A   D E C L A R A T I O N S                   */
/*===========================================================================*/


/*===========================================================================*/
/*    L O C A L   S Y M B O L    D E C L A R A T I O N S                     */
/*===========================================================================*/

/*--- Static Variables ---*/

static fmnu_MenuStruct *fmnu_menu_data_ptr;  /* Pointer to the currently active menu structure */
static fuimFieldStruct *fmnu_field_data_ptr; /* Pointer to the array of fields in the current menu */
static fuimFieldStruct *active_field_ptr;    /* Pointer to the currently active/selectable field */
static Byte active_index_menu;               /* Index of the currently active menu in auim_Menu[] */

/*****************************************************************************/
static fmnu_MenuProperty  MenuDialogProperties;
static              Word  osd_row[ FUIM_MAX_DISPLAY_FIELDS ];  /* Row positions for displayed fields */
static   fuimFieldStruct * displayed_fields[ FUIM_MAX_DISPLAY_FIELDS ];  /* Pointers to visible field structures */

/* active_menu_handle will store the handle of the currently displayed and active menu.
NULL = no active menu */
osdDialogHandle fmnu_active_menu_handle ;
osdTimerHandle  field_timer_handle;
osdTimerHandle  fmnu_menu_timer_handle;

static Bool fmnu_KeyPassAlways;  /* Flag to enable repeated key passing */
static Byte fmnu_KeyPassCommand; /* Command to pass repeatedly */
static Bool fmnu_full_redraw_menu_fields = FALSE;  /* Dirty flag: TRUE triggers full menu redraw */

/*===========================================================================*/
/* L O C A L   F U N C T I O N S      P R O T O T T Y P E                    */
/*===========================================================================*/

void fuim_ProcessMenuAction(cmdKeyNumber action);
void fmnu_ChangeField(Bool direction );
void fmnu_ConstructMenuField(fuimFieldStruct *field_data_ptr, Bool Highlighted);
void fmnu_UpdateValueField  (fuimFieldStruct *field_data_ptr, Bool Highlighted);

void fmnu_ConstructTitle(Word MenuWidth);
void fmnu_DestroyTitle  (Word MenuWidth);

void fmnu_DestroyField(void); //(fmnu_MenuProperty *position);

void fmnu_ConstructList (fuimFieldStruct *field_data_ptr, Byte margin_top );
void fmnu_ConstructValue(fuimFieldStruct *field_data_ptr, Bool Highlighted);

void fmnu_ConstructButtonField(Bool left);
void fmnu_DestroyButtonField  (Bool left);

//void fmnu_ConstructEmptyField(  Bool SelectPtr );
//void fmnu_ConstructSpaser(void);

void fmnu_ConstructMenuPrompt(fuimFieldStruct *field_data_ptr, Word end_position);
static void fmnu_DrawFieldEmptyPadding(Word Xpos, Word Width, Word Attribute);

void fmnu_DestroyFixedField(void);
void fmnu_ConstructFixedField(void);

void fmnu_DestroyMenu(void);

/*void fuim_DrawNumeric(char  *NumericCharacter,
                      osdFieldValue GetFunction,
                      Byte FieldScalingNumeric,
                      Byte FieldSizeNumeric,
                      Bool Highlighted ) ;*/
/*=================================================================================
*   Function:    fmnu_DrawFieldEmptyPadding
*   Description: Draws filler characters to fill remaining space in field
*
* Arguments:
* Parameter    Flow    Description
* ------------------------------------------------------------------------------
* Xpos         IN      Field starting horizontal position
* Width        IN      Field width in pixels
* Attribute    IN      Field attributes (contains row height in low byte)
*
* Externals    Flow    Usage
* ------------------------------------------------------------------------------
* FUIM_ATTRIBUTES_ROW_SIZE IN  Mask to extract row height from Attribute
*
* @return void
*
* @note Returns early if Xpos exceeds screen width or Width is zero.
*       Uses plt_CC* functions for character cell rendering.
===================================================================================*/
static void fmnu_DrawFieldEmptyPadding(Word Xpos, Word Width, Word Attribute)
{
  /* Input parameter validation - protection against invalid values */
  if (Xpos > FUIM_MENU_WIDTH) {
    /* Starting position is outside screen bounds - exit */
    return;
  }
  if (Width == 0) {
    /* Zero field width - nothing to fill */
    return;
  }
  /* Limit the end position to the display width */
  Word fieldEndPos = Xpos + Width;
  /* Adjust if exceeding screen boundaries */
  if (fieldEndPos > FUIM_MENU_WIDTH) {
    fieldEndPos = FUIM_MENU_WIDTH;
  }
  /* Get current cursor position */
  Word cursorPosition = fuim_GetColumnPosition();
  /* Check: if cursor is already beyond the field - do not fill */
  if (cursorPosition >= fieldEndPos) {
    return;
  }
  /* Check: if cursor is before the field start - start from field beginning */
  if (cursorPosition < Xpos) {
    cursorPosition = Xpos;
  }
  /* Calculate number of characters to fill */
  Word fillerCount = fieldEndPos - cursorPosition;
  /* Set row height from field attributes */
  plt_CCSetRowSize(Attribute & FUIM_ATTRIBUTES_ROW_SIZE);
  /* Draw filler characters */
  fuim_DrawRepeatedCharacter(fillerCount);
}

/*=================================================================================
*   Function:    fmnu_InitMenus
*   Description: Initializes the menu system by resetting all menu-related handles
*                and pointers to their default "no active menu" state.
*
* Arguments:        None
*
* Externals    Flow    Usage
* ------------------------------------------------------------------------------
* fmnu_menu_data_ptr      OUT  Set to NULL
* fmnu_active_menu_handle OUT  Set to FUIM_NO_ACTIVE_DIALOG
* fmnu_menu_timer_handle   OUT  Set to FUIM_NO_FREE_TIMER_HANDLE
* field_timer_handle       OUT  Set to FUIM_NO_FREE_TIMER_HANDLE
*
* @return void
*
* @note Must be called before any menu operations. Called from fmnu_Activate()
*       and fuim_TurnOff().
===================================================================================*/
void fmnu_InitMenus( void )
{
  fmnu_menu_data_ptr      = NULL ;
  fmnu_active_menu_handle = FUIM_NO_ACTIVE_DIALOG ;
  fmnu_menu_timer_handle  = FUIM_NO_FREE_TIMER_HANDLE ;
  field_timer_handle      =  FUIM_NO_FREE_TIMER_HANDLE ;
}

/*=================================================================================
*   Function:    fmnu_ConstructValue
*   Description: Renders the value portion of a menu field based on its type.
*                Handles LIST, STRING, NUMERIC, and STRING_NUMERIC_VALUE types.
*                Skips rendering if validity returns FUIM_VALIDITY_PRESENT.
*
* Arguments:
* Parameter      Flow    Description
* ------------------------------------------------------------------------------
* field_data_ptr IN      Pointer to field structure containing type and rendering info
* Highlighted    IN      TRUE for highlighted colors, FALSE for normal colors
*
* Externals    Flow    Usage
* ------------------------------------------------------------------------------
* fmnu_menu_data_ptr IN  Source for menu geometry (Xpos, Width)
*
* @return void
*
* @note Calls validity function to determine field visibility. Uses valueMargin
*       from menu to position field vertically. Delegates to type-specific renderers.
===================================================================================*/
void fmnu_ConstructValue( fuimFieldStruct  *field_data_ptr ,Bool Highlighted)
{

// @ToDo  Bookmarked for future use; analyse the requirement when `Validity` is set to 
//     FUIM_VALIDITY_SELECTABLE (field is visible and selectable) 
// or  FUIM_VALIDITY_GRAYEDOUT  (field is greyed out, not selectable)
// or  FUIM_VALIDITY_PRESENT    (Field is present but not visible)     It was been added by UP
  if((fuim_ValidityFunction( field_data_ptr -> ValidityFunction)) == FUIM_VALIDITY_PRESENT) {
    return;
  }

  // Calculate value margin in pixels
  Word valueMargin = fuim_GetMenuValueVertMaginTop(fmnu_menu_data_ptr);

  switch( field_data_ptr->Type )
  {
    case FUIM_FIELDTYPE_LIST:
      fmnu_ConstructList( field_data_ptr, valueMargin );
    break;

    case FUIM_FIELDTYPE_STRING:
    case FUIM_FIELDTYPE_STRING_VALUE:
      fuim_ConstructString( field_data_ptr, valueMargin);
    break;

    case FUIM_FIELDTYPE_NUMERIC:
    case FUIM_FIELDTYPE_NUMERIC_VALUE:
      fuim_ConstructNumeric(field_data_ptr, valueMargin, Highlighted );
    break;

    case FUIM_FIELDTYPE_STRING_NUMERIC_VALUE :
      fuim_ConstructStringNumeric( field_data_ptr, valueMargin );
    break;

    default:
      break;
  }

}

/*=================================================================================
*   Function:    fmnu_SetPositionFieldAligment
*   Description: Calculates and sets the cursor column position based on field alignment
*                and total available width. Used for both prompt and value rendering.
*
* Arguments:
* Parameter      Flow    Description
* ------------------------------------------------------------------------------
* field_data_ptr IN      Pointer to field structure containing alignment and content
* total_widht    IN      Total available width for alignment calculation
* isPrompt       IN      TRUE to calculate prompt width, FALSE for value width
*
* Externals    Flow    Usage
* ------------------------------------------------------------------------------
* fmnu_menu_data_ptr IN  Source for menu geometry
*
* @return void
*
* @note Modifies global cursor position via fuim_SetColumnPosition().
*       Triggers assertion failure if field_width exceeds total width.
===================================================================================*/
void fmnu_SetPositionFieldAligment(fuimFieldStruct *field_data_ptr, Word total_widht, Bool isPrompt)
{
  Word   curr_Xpos = fuim_GetColumnPosition();
  Byte   alignment = field_data_ptr->Alignment;
  Word field_width = fuim_GetFieldValueLength(field_data_ptr, isPrompt);

  if (field_width == 0) return;

  // Calculate starting position based on alignment and MenuWidth
  if (total_widht > field_width) {
    if (alignment == FUIM_ALIGNMENT_CENTRE) {
        curr_Xpos += (total_widht - field_width) / 2;
    } else if (alignment == FUIM_ALIGNMENT_RIGHT) {
        curr_Xpos += (total_widht - field_width - FUIM_MENU_SINGLE_ROW_PROMPT_TO_VALUE_PADDING);
    }else {           //FUIM_ALIGNMENT_LEFT
        curr_Xpos  = ((curr_Xpos + field_width + FUIM_MENU_SINGLE_ROW_PROMPT_TO_VALUE_PADDING) < total_widht) ? (curr_Xpos + FUIM_MENU_SINGLE_ROW_PROMPT_TO_VALUE_PADDING) : (curr_Xpos);
    }

  } else {
    app_log("field_width Failed: %d + %d \r\n", total_widht, field_width );
    app_assert_status(SL_STATUS_FAIL); // Loop forever for debugging
  }

  fuim_SetColumnPosition(curr_Xpos);
}

/*=================================================================================
*   Function:    fmnu_UpdateValueField
*   Description: Updates and redraws only the value portion of an already rendered menu field.
*                Does not redraw the prompt. Used for periodic value refresh from observers.
*
* Arguments:
* Parameter      Flow    Description
* ------------------------------------------------------------------------------
* field_data_ptr IN      Pointer to field structure with Type/ValidityFunction
* Highlighted    IN      TRUE for highlighted state, FALSE for normal
*
* Externals    Flow    Usage
* ------------------------------------------------------------------------------
* fmnu_menu_data_ptr IN  Provides Xpos/ValuePos/PromptPos/Width
*
* @return void
*
* @note Skips fields with FUIM_VALIDITY_NOTPRESENT. Supports two display modes:
*       SPLITROW and normal, each with alignment/padding logic.
===================================================================================*/
void fmnu_UpdateValueField( fuimFieldStruct  *field_data_ptr, Bool Highlighted)
{
  Word   MenuXpos  = fmnu_menu_data_ptr ->Xpos;
  Word   MenuValuePos  = fmnu_menu_data_ptr ->ValuePos;
  Word   MenuPromptPos = fmnu_menu_data_ptr -> PromptPos;
  Word   MenuWidth = fmnu_menu_data_ptr ->Width;

  Byte  Count;

  Word  ForeGndColour = 0;
  Word  BackGndColour = 0;
  Word  Attribute = 0;

  fuim_Validity   Validity;

  Attribute     = fuim_GetFieldPromptColour(field_data_ptr)  -> Attribute;
  ForeGndColour = fuim_GetFieldPromptColour( field_data_ptr) -> ForeGndColour;
  BackGndColour = fuim_GetFieldPromptColour( field_data_ptr) -> BackGndColour;

  Validity = fuim_ValidityFunction( field_data_ptr -> ValidityFunction);
  if( Validity != FUIM_VALIDITY_NOTPRESENT)
  {
      /* Check if split row display is requested */
      if ((Attribute & FUIM_ATTRIBUTES_SPLITROW) == FUIM_ATTRIBUTES_SPLITROW)
      {
          /*=============================================================*/
          /*              SPLIT ROW DISPLAY                              */
          /*         (Three-Area Clearing per Row)                       */
          /*=============================================================*/
          plt_CCSetRowSize(Attribute & FUIM_ATTRIBUTES_ROW_SIZE);
          fuim_SetAttributes( Attribute);
          plt_CCSetForegroundColour( ForeGndColour );
          plt_CCSetBackgroundColour( BackGndColour);

          /*
           *  ┌───────────────────────────────────────────┐
           *  │ ROW 2 (VALUE):                            │
           *  │ [Clear Left][Aligned Value][Clear Right]  │
           *  │     │            │              │         │
           *  │  MenuXpos   alignedValuePos    rowEnd     │
           *  └───────────────────────────────────────────┘
           */

          /* Display value on lower row */
          plt_CCSetPosition(FUIM_SPLIT_SECOND_ROW_OFFSET, MenuXpos);
          // Apply alignment position to value
          fmnu_SetPositionFieldAligment(field_data_ptr, MenuWidth, FALSE);

          Word alignedValuePos = fuim_GetColumnPosition();//SAVE  value aligment position
          //Clear string before value start position

          // AREA 1: Clear Left Padding (row start to aligned value position)
          if (alignedValuePos > MenuXpos) {
            fuim_SetColumnPosition(MenuXpos);
            fmnu_DrawFieldEmptyPadding(MenuXpos, (alignedValuePos - MenuXpos), Attribute);
          }

          fuim_SetColumnPosition(alignedValuePos);
      }
      else
      {

        /* Normal single row display with alignment and padding */

          // Calculate field widths for alignment
          Word promptWidth = fuim_GetFieldValueLength(field_data_ptr, TRUE);
          Word valueWidth = fuim_GetFieldValueLength(field_data_ptr, FALSE);
          Word padding = ( (promptWidth != 0) && (valueWidth != 0) ) ? (FUIM_MENU_SINGLE_ROW_PROMPT_TO_VALUE_PADDING) : (0);
          Word totalFieldWidth = promptWidth + padding + valueWidth;

          Word curr_Xpos = MenuXpos;
          // Calculate starting position based on alignment and MenuWidth values
          if (MenuWidth > totalFieldWidth) {
            if (field_data_ptr->Alignment == FUIM_ALIGNMENT_CENTRE) {
                curr_Xpos += (MenuWidth - totalFieldWidth) / 2;
            } else if (field_data_ptr->Alignment == FUIM_ALIGNMENT_RIGHT) {
                curr_Xpos += (MenuWidth - totalFieldWidth);
            } else {  //FUIM_ALIGNMENT_LEFT
                curr_Xpos = MenuPromptPos;
                padding   = ((MenuPromptPos + promptWidth) < MenuValuePos) ? (MenuValuePos - (MenuPromptPos + promptWidth)) : (FUIM_MENU_SINGLE_ROW_PROMPT_TO_VALUE_PADDING);
            }
          } else {
              app_log("totalFieldWidth Failed: %d + %d + %d\r\n", promptWidth, padding, valueWidth);
              app_assert_status(SL_STATUS_FAIL); // Loop forever for debugging
          }

        //  fuim_SetColumnPosition(MenuValuePos);
          plt_CCSetRowSize(Attribute & FUIM_ATTRIBUTES_ROW_SIZE);

          // Set attributes for clearing and rendering
          fuim_SetAttributes( Attribute);
          plt_CCSetForegroundColour( ForeGndColour );
          plt_CCSetBackgroundColour( BackGndColour);

          /*-------------------------------------------------------------*/
          /* AREA 1: Clear Left Padding Area                             */
          /* From MenuXpos to aligned prompt position (curr_Xpos)        */
          /*-------------------------------------------------------------*/
          if (curr_Xpos > MenuXpos) {
              fuim_SetColumnPosition(MenuXpos);
              fmnu_DrawFieldEmptyPadding(MenuXpos, (curr_Xpos - MenuXpos), Attribute);
          }

          /*-------------------------------------------------------------*/
          /* AREA 2: Render Prompt at Aligned Position                  */
          /*-------------------------------------------------------------*/
          fuim_SetColumnPosition(curr_Xpos);
          fmnu_ConstructMenuPrompt(field_data_ptr, (curr_Xpos + promptWidth));
          Word promptEndPos = fuim_GetColumnPosition();

          /*-------------------------------------------------------------*/
          /* AREA 3: Clear Inter-Field Padding Area                      */
          /* From prompt end to value start position                     */
          /*-------------------------------------------------------------*/
          Word valueStartPos = curr_Xpos + promptWidth + padding;
          if (valueStartPos > promptEndPos) {
               fuim_SetColumnPosition(promptEndPos);
               fmnu_DrawFieldEmptyPadding(promptEndPos, (valueStartPos - promptEndPos), Attribute);
           }

          /*-------------------------------------------------------------*/
          /* AREA 4: Render Value after Prompt with Padding              */
          /*-------------------------------------------------------------*/
          fuim_SetColumnPosition(valueStartPos);

      }

      // Re-set attributes after clearing (may have been modified)
      fuim_SetAttributes( Attribute);
      plt_CCSetForegroundColour( ForeGndColour );
      plt_CCSetBackgroundColour( BackGndColour);

      fmnu_ConstructValue(field_data_ptr, Highlighted);

  }
  //Fill Remaining Space till the End
  Count = (MenuXpos + MenuWidth) > fuim_GetColumnPosition() ? ((MenuXpos + MenuWidth) - fuim_GetColumnPosition()) :(0);
  fuim_DrawRepeatedCharacter(Count);

}

/*=================================================================================
*   Function:    fmnu_ConstructMenuField
*   Description: Renders a complete menu field including prompt and value with proper
*                alignment, padding, and highlighting. Handles both normal single-row
*                and split-row display modes.
*
* Arguments:
* Parameter      Flow    Description
* ------------------------------------------------------------------------------
* field_data_ptr IN      Pointer to field structure with type/runtime attributes
* Highlighted    IN      TRUE to render active/highlighted style, FALSE for normal
*
* Externals    Flow    Usage
* ------------------------------------------------------------------------------
* fmnu_menu_data_ptr IN  Provides Xpos/Width/PromptPos/ValuePos
* active_field_ptr   IN  Used to determine active field for highlighting
*
* @return void
*
* @note Skips rendering entirely if field validity is FUIM_VALIDITY_NOTPRESENT.
*       Clears areas around content using three-area clearing strategy.
*       Split-row mode uses FUIM_SPLIT_FIRST_ROW_OFFSET/FUIM_SPLIT_SECOND_ROW_OFFSET.
*       Single-row mode clears: left padding, inter-field padding, right padding.
===================================================================================*/
void fmnu_ConstructMenuField(fuimFieldStruct  *field_data_ptr, Bool Highlighted )
{

  Word   MenuYpos      = 0;
  Word   MenuXpos      = fmnu_menu_data_ptr -> Xpos;
  Word   MenuWidth     = fmnu_menu_data_ptr -> Width;

  Word   MenuPromptPos = fmnu_menu_data_ptr -> PromptPos;
  Word   MenuValuePos  = fmnu_menu_data_ptr -> ValuePos;


  Word   Attribute;
  Word   ForeGndColour;
  Word   BackGndColour;
  Byte   MenuValidity;

  Word   Count;

  Word   alignedPromptPos;
  Word   alignedValuePos;

  Word   promptEndPos;

  Word   valueStartPos;
  Word   valueEndPos;

  Word   promptWidth;
  Word   valueWidth;

  Word  fieldEndPos = MenuXpos + MenuWidth;

  MenuYpos = fuim_GetRowPosition(); //Set vertical menu position once


  Attribute     = fuim_GetFieldPromptColour(field_data_ptr) -> Attribute;
  ForeGndColour = fuim_GetFieldPromptColour(field_data_ptr) -> ForeGndColour;
  BackGndColour = fuim_GetFieldPromptColour(field_data_ptr) -> BackGndColour;

  plt_CCSetRowSize(Attribute & FUIM_ATTRIBUTES_ROW_SIZE);
  plt_CCSetForegroundColour(ForeGndColour);
  plt_CCSetBackgroundColour(BackGndColour);


  MenuValidity = fuim_ValidityFunction( field_data_ptr -> ValidityFunction);
  if( MenuValidity == FUIM_VALIDITY_NOTPRESENT)
  {
      Count = (fieldEndPos) > fuim_GetColumnPosition() ? ((fieldEndPos) - fuim_GetColumnPosition()) :(0);
      fuim_DrawRepeatedCharacter(Count);
      return;
  }


    /* Check if split row display is requested */
    if ((Attribute & FUIM_ATTRIBUTES_SPLITROW) == FUIM_ATTRIBUTES_SPLITROW)
    {
        /*=============================================================*/
        /*              SPLIT ROW DISPLAY                              */
        /*         (Three-Area Clearing per Row)                       */
        /*=============================================================*/

        /*  ┌──────────────────────────────────────────────┐
        *   │ ROW 1 (PROMPT):                              │
        *   │ [Clear Left][Aligned Prompt][Clear Right]    │
        *   │     │            │              │            │
        *   │  MenuXpos   alignedPromptPos   rowEnd        │
        *   └──────────────────────────────────────────────┘
        */

        /*---------------------------------------------------------------------*/
        /* ROW 1: Display PROMPT on upper row                                  */
        /* Strategy: [Clear Left] → [Apply Alignment] → [Draw] → [Clear Right] */
        /*---------------------------------------------------------------------*/

        // Set attributes for ROW 1
        fuim_SetAttributes(Attribute);
        plt_CCSetForegroundColour(ForeGndColour);
        plt_CCSetBackgroundColour(BackGndColour);

        // Position at ROW 1 start
        plt_CCSetPosition(FUIM_SPLIT_FIRST_ROW_OFFSET, MenuXpos);

        // Calculate aligned position for prompt
        fmnu_SetPositionFieldAligment(field_data_ptr, MenuWidth, TRUE);
        alignedPromptPos = fuim_GetColumnPosition();

        // AREA 1: Clear Left Padding (row start to aligned prompt position)
        if (alignedPromptPos > MenuXpos) {
          fuim_SetColumnPosition(MenuXpos);
          fmnu_DrawFieldEmptyPadding(MenuXpos, (alignedPromptPos - MenuXpos), Attribute);
        }

        // AREA 2: Draw Prompt at aligned position
        plt_CCSetPosition(FUIM_SPLIT_FIRST_ROW_OFFSET, alignedPromptPos);

        promptWidth = fuim_GetFieldValueLength(field_data_ptr, TRUE);
        fmnu_ConstructMenuPrompt(field_data_ptr, (MenuXpos + promptWidth)); //AREA 3:  <- Clear Right Padding (prompt end to row end)

        promptEndPos = fuim_GetColumnPosition();

        // AREA 3: Clear Right Padding (prompt end to row end)
        if (fieldEndPos > promptEndPos) {
            fuim_SetColumnPosition(promptEndPos);
            fmnu_DrawFieldEmptyPadding(promptEndPos, (fieldEndPos - promptEndPos), Attribute);
        }

        /*
         *  ┌───────────────────────────────────────────┐
         *  │ ROW 2 (VALUE):                            │
         *  │ [Clear Left][Aligned Value][Clear Right]  │
         *  │     │            │              │         │
         *  │  MenuXpos   alignedValuePos    rowEnd     │
         *  └───────────────────────────────────────────┘
         */

        /*---------------------------------------------------------------------*/
        /* ROW 2: Display VALUE on lower row                                   */
        /* Strategy: [Clear Left] → [Apply Alignment] → [Draw] → [Clear Right] */
        /*---------------------------------------------------------------------*/

        // Set attributes for ROW 2
        fuim_SetAttributes(Attribute);
        plt_CCSetForegroundColour(ForeGndColour);
        plt_CCSetBackgroundColour(BackGndColour);

        // Position at ROW 2 start
        plt_CCSetPosition(FUIM_SPLIT_SECOND_ROW_OFFSET, MenuXpos);

        // Calculate aligned position for value
        fmnu_SetPositionFieldAligment(field_data_ptr, MenuWidth, FALSE);
        alignedValuePos = fuim_GetColumnPosition();

        // AREA 1: Clear Left Padding (row start to aligned value position)
        if (alignedValuePos > MenuXpos) {
          fuim_SetColumnPosition(MenuXpos);
          fmnu_DrawFieldEmptyPadding(MenuXpos, (alignedValuePos - MenuXpos), Attribute);
        }

        // AREA 2: Draw Value at aligned position
        fmnu_ConstructValue(field_data_ptr, Highlighted);
        valueEndPos = fuim_GetColumnPosition();

        // AREA 3: Clear Right Padding (value end to row end)
        if (fieldEndPos > valueEndPos) {
          fuim_SetColumnPosition(valueEndPos);
          fmnu_DrawFieldEmptyPadding(valueEndPos, (fieldEndPos - valueEndPos), Attribute);
        }

    }
    else
    {   /*=============================================================*/
        /*              NORMAL SINGLE ROW DISPLAY                      */
        /*           (Three-Area Clearing Implementation)              */
        /*=============================================================*/
        /*    Normal single row display with alignment and padding     */
        /*
         *  ┌──────────────────────────────────────────────────────────┐
         *  │[Clear Left] [Prompt] [Clear Inter] [Value] [Clear Right] │
         *  │     │          │          │          │          │        │
         *  │  MenuXpos   curr_Xpos  promptEnd valueStart  fieldEnd    │
         *  └──────────────────────────────────────────────────────────┘
        */
          // Calculate field widths for alignment
            promptWidth = fuim_GetFieldValueLength(field_data_ptr, TRUE);
            valueWidth  = fuim_GetFieldValueLength(field_data_ptr, FALSE);
          Word padding     = ( (promptWidth != 0) && (valueWidth != 0) ) ? (FUIM_MENU_SINGLE_ROW_PROMPT_TO_VALUE_PADDING) : (0);
          Word totalFieldWidth = promptWidth + padding + valueWidth;

          Word curr_Xpos = MenuXpos;

          // Calculate starting position based on alignment and MenuWidth
          if (MenuWidth > totalFieldWidth) {
            if (field_data_ptr->Alignment == FUIM_ALIGNMENT_CENTRE) {
                curr_Xpos += (MenuWidth - totalFieldWidth) / 2;
            } else if (field_data_ptr->Alignment == FUIM_ALIGNMENT_RIGHT) {
                curr_Xpos += (MenuWidth - totalFieldWidth);
            } else {  //FUIM_ALIGNMENT_LEFT
                curr_Xpos += MenuPromptPos;
                padding   = ((MenuPromptPos + promptWidth) < MenuValuePos) ? (MenuValuePos - (MenuPromptPos + promptWidth)) : (FUIM_MENU_SINGLE_ROW_PROMPT_TO_VALUE_PADDING);
            }
          } else {
              app_log("totalFieldWidth Failed: %d + %d + %d\r\n", promptWidth, padding, valueWidth);
              app_assert_status(SL_STATUS_FAIL); // Loop forever for debugging
          }

         // Set attributes for clearing and rendering
         fuim_SetAttributes(Attribute);
         plt_CCSetForegroundColour(ForeGndColour);
         plt_CCSetBackgroundColour(BackGndColour);

         /*-------------------------------------------------------------*/
         /* AREA 1: Clear Left Padding Area                             */
         /* From MenuXpos to aligned prompt position (curr_Xpos)        */
         /*-------------------------------------------------------------*/
         if (curr_Xpos > MenuXpos) {
             fuim_SetColumnPosition(MenuXpos);
             fmnu_DrawFieldEmptyPadding(MenuXpos, (curr_Xpos - MenuXpos), Attribute);
         }

          /*-------------------------------------------------------------*/
          /* AREA 2: Render Prompt at Aligned Position                  */
          /*-------------------------------------------------------------*/
          fuim_SetColumnPosition(curr_Xpos);
          fmnu_ConstructMenuPrompt(field_data_ptr, (curr_Xpos + promptWidth));

          //Test position need to debug only
          /* @ToDo This line for testing purposes It was been added by UP*/
          promptEndPos = fuim_GetColumnPosition();
          if (promptEndPos != (curr_Xpos + promptWidth)) {
              app_log("promptEndPos %d Failed: !=  %d + %d + %d\r\n", promptEndPos, curr_Xpos, promptWidth, padding);
              app_assert_status(SL_STATUS_FAIL); // Loop forever for debugging
          }

          /*-------------------------------------------------------------*/
          /* AREA 3: Clear Inter-Field Padding Area                      */
          /* From prompt end to value start position                     */
          /*-------------------------------------------------------------*/
          valueStartPos = curr_Xpos + promptWidth + padding;
          if (valueStartPos > promptEndPos) {
               fuim_SetColumnPosition(promptEndPos);
               fmnu_DrawFieldEmptyPadding(promptEndPos, (valueStartPos - promptEndPos), Attribute);
           }

          /*-------------------------------------------------------------*/
          /* AREA 4: Render Value after Prompt with Padding              */
          /*-------------------------------------------------------------*/
          fuim_SetColumnPosition(valueStartPos);

          // Re-set attributes after clearing (may have been modified)
          fuim_SetAttributes(Attribute);
          plt_CCSetForegroundColour(ForeGndColour);
          plt_CCSetBackgroundColour(BackGndColour);

          fmnu_ConstructValue(field_data_ptr, Highlighted);

          /*-------------------------------------------------------------*/
          /* AREA 5: Clear Right Padding Area                            */
          /* From value end to menu end (MenuXpos + MenuWidth)           */
          /*-------------------------------------------------------------*/
          valueEndPos = fuim_GetColumnPosition();
          if (fieldEndPos > valueEndPos) {
              fuim_SetColumnPosition(valueEndPos);
              fmnu_DrawFieldEmptyPadding(valueEndPos, (fieldEndPos - valueEndPos), Attribute);
          }

    }

  (void)MenuYpos;

}

/*=================================================================================
*   Function:    fmnu_DestroyField
*   Description: Clears the menu field area by filling it with background color.
*                Erases the entire menu content area from top to bottom.
*
* Arguments:        None
*
* Externals    Flow    Usage
* ------------------------------------------------------------------------------
* None
*
* @return void
*
* @note Sets row size to exclude top/bottom margins. Uses ST7789_WHITE background.
*       Called during menu destruction or field refresh.
===================================================================================*/
void fmnu_DestroyField(void)
{
  plt_CCSetRowSize(FUIM_MENU_HEIGHT - (FUIM_TOP_ROW_SIZE + FUIM_BOTTOM_ROW_SIZE) );
  plt_CCSetBackgroundColour(ST7789_WHITE); //TEST UP
  plt_CCSetPosition (FUIM_TOP_ROW_SIZE, 0);

  fuim_DrawRepeatedCharacter (FUIM_MENU_WIDTH); //Erase till end of ROWS

}

/*=================================================================================
*   Function:    fmnu_DrawMenuFields
*   Description: Renders all visible menu fields within the current viewport.
*                Calculates visible field range based on active field position and
*                visible fields count, then draws each field with proper highlighting.
*
* Arguments:
* Parameter      Flow    Description
* ------------------------------------------------------------------------------
* bDraw          IN      TRUE for full field construction, FALSE for value-only update
*
* Externals    Flow    Usage
* ------------------------------------------------------------------------------
* fmnu_menu_data_ptr     IN  Provides VisibleFields count
* MenuDialogProperties   IN/OUT  Updated with StartFieldNr during drawing
* osd_row[]             OUT  Filled with row positions for drawn fields
* displayed_fields[]    IN   Array of pointers to visible field structures
* active_field_ptr      IN   Current active field for highlight check
*
* @return void
*
* @note Visible range calculation ensures active field stays centered when possible.
*       When bDraw=FALSE, only values are updated without full field reconstruction.
*       Uses osd_row[] to cache row positions for later navigation use.
===================================================================================*/
void fmnu_DrawMenuFields(Bool bDraw)
{
  Byte i, visible_fields;
  Byte start, end;

  visible_fields = fuim_GetMenuVisibleFields(fmnu_menu_data_ptr);
  if ( visible_fields != 0 ) {
    if ( MenuDialogProperties.ActiveFieldNr < (visible_fields / 2) ) {
      start = 0 ;
      end   = visible_fields ;

    } else {

      start = MenuDialogProperties.ActiveFieldNr - (visible_fields / 2) ;
      end   = start + visible_fields;

      if (end > MenuDialogProperties.LastFieldNr) {
        end   = MenuDialogProperties.LastFieldNr ;
        start = end - visible_fields;
      }
    }
  } else {
    start = MenuDialogProperties.FirstFieldNr;
    end   = MenuDialogProperties.LastFieldNr;
  }


  MenuDialogProperties.StartFieldNr = start;

  fuim_SetRowPosition(MenuDialogProperties.StartRow);

  for( i = start; i < end; i++)
  {

       osd_row[ i ] = fuim_GetRowPosition();

       plt_CCSetPosition (osd_row[i],  MenuDialogProperties.FirstPos);

       if ( i == MenuDialogProperties.ActiveFieldNr )
       {
            if((displayed_fields[i] ->TimeOut != FUIM_FIELD_NO_TIMEOUT) || (bDraw) )
            {
              if (bDraw) {
                fmnu_ConstructMenuField( displayed_fields[i] ,   TRUE );
              } else {
                fmnu_UpdateValueField( displayed_fields[i], TRUE);
              }
           }
       }
       else
       {
          if((displayed_fields[i] ->TimeOut != FUIM_FIELD_NO_TIMEOUT) || (bDraw) )
          {
            if(bDraw) {
                fmnu_ConstructMenuField( displayed_fields[i] ,   FALSE );
            } else {
                fmnu_UpdateValueField( displayed_fields[i], FALSE);
            }
          }
       }

       fuim_SetNextRow();
   }

}


/*=================================================================================
*   Function:    fmnu_ChangeField
*   Description: Changes the active/selected field in the current menu by searching
*                for the next selectable field in the specified direction.
*
* Arguments:
* Parameter      Flow    Description
* ------------------------------------------------------------------------------
* direction      IN      TRUE for forward/next, FALSE for backward/previous
*
* Externals    Flow    Usage
* ------------------------------------------------------------------------------
* MenuDialogProperties      IN  Provides ActiveFieldNr/LastFieldNr range
* displayed_fields[]        IN  Array of visible field pointers indexed by field number
* osd_row[]                OUT  Used to restore row position on redraw
* active_field_ptr         OUT  Updated to new active field
*
* @return void
*
* @note Uses rbsc_ChangeControlAround() to wrap around field boundaries.
*       Falls back to original active field if no selectable field is found.
*       Triggers full or single-field redraw based on visible fields setting.
===================================================================================*/
void fmnu_ChangeField(Bool direction )
{
  Byte   active_field_nr;
  Byte   field_status;
  Byte   loop;
  fuimFieldStruct * previous_active_field_ptr;

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

  } while ( (field_status != FUIM_VALIDITY_SELECTABLE) && (--loop != 0) );

  if ( loop == 0 ) {
    active_field_nr = MenuDialogProperties.ActiveFieldNr ;
    active_field_ptr = displayed_fields[ active_field_nr ] ;
  }

    if ( previous_active_field_ptr != active_field_ptr )
    {
     // fmnu_ConstructMenuField( previous_active_field_ptr, FALSE );  /* @ToDo investigate an issue with commented out string fmnu_ConstructMenuField( previous_active_field_ptr, FALSE ) It was been added by UP*/
      MenuDialogProperties.ActiveFieldNr = active_field_nr ;

      if ( fuim_GetMenuVisibleFields(fmnu_menu_data_ptr) != 0 ) {
        fmnu_DrawMenuFields(TRUE);
      } else {
        fuim_SetRowPosition( osd_row[ active_field_nr ] );
        fmnu_ConstructMenuField( active_field_ptr, TRUE);
      }
    }

}


/*=================================================================================
*   Function:    fuim_ProcessMenuAction
*   Description: Processes the active menu command by executing the command's
*                configured action through the action handler. Handles navigation,
*                selection, and menu closure commands.
*
* Arguments:
* Parameter      Flow    Description
* ------------------------------------------------------------------------------
* action         IN      Virtual key command to process
*
* Externals    Flow    Usage
* ------------------------------------------------------------------------------
* active_field_ptr     IN   Active field containing ToDoWithKey action table
* fmnu_active_menu_handle IN  Guard: returns early if FUIM_NO_ACTIVE_DIALOG
* MenuDialogProperties   IN  Provides ActiveFieldNr for osd_row lookup
*
* @return void
*
* @note Posts AU_IDLE_MENU_START for AU_KEY_CLOSE.
*       Posts AU_MENU_CHARGE_BATTERY_ENTER for AU_KEY_CANCEL.
*       Uses aukh_Post_UI_Event() for UI transitions.
*       Commented code shows transformer logic for AU_KEY_YES (disabled).
===================================================================================*/
void fuim_ProcessMenuAction(cmdKeyNumber action)
{

  Word                    active_osd_row;
  fuimDialogNavigation   * ActionPtr;
  Byte                    new_action;

  active_osd_row = osd_row[ MenuDialogProperties.ActiveFieldNr ];

  if (fmnu_active_menu_handle == FUIM_NO_ACTIVE_DIALOG)
  {
    return ;//action ;
  }

  if (active_field_ptr != NULL)
  {
      ActionPtr = active_field_ptr->ToDoWithKey ;

      if ( ActionPtr != NULL )
      {
         do
         {
            if ( ActionPtr->Action == action ) {

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

  switch( action )
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
      fmnu_ChangeField( (action == AU_KEY_NEXT) ? RGEN_CHANGE_UP : RGEN_CHANGE_DOWN );
      action = AU_KEY_PROCESSED ;
     } break;

     case AU_KEY_CLOSE:
     {
        aukh_Post_UI_Event(AU_IDLE_MENU_START);
        action = AU_KEY_PROCESSED ;
     }break;

     case AU_KEY_CANCEL:
     {
       aukh_Post_UI_Event(AU_MENU_CHARGE_BATTERY_ENTER);
       action = AU_KEY_PROCESSED ;
     } break;

    default:
      break;

  }


  active_osd_row = active_osd_row;

}
/*=================================================================================
*   Function:    fmnu_ConstructTitle
*   Description: Renders the menu title bar at the top of the menu window.
*                Positions the title with left/right padding and applies title colors.
*
* Arguments:
* Parameter      Flow    Description
* ------------------------------------------------------------------------------
* MenuWidth      IN      Total menu width in pixels
*
* Externals    Flow    Usage
* ------------------------------------------------------------------------------
* fmnu_menu_data_ptr IN  Provides Title/TitleAttribute/Xpos
*
* @return void
*
* @note Uses left margin FUIM_TITLE_LEFT_MARGIN and right margin FUIM_TITLE_RIGHT_MARGIN.
*       Clears areas left and right of title using filling strategy.
*       If no title (FMNU_NONE_TITLE), clears entire title row.
===================================================================================*/
void fmnu_ConstructTitle(Word MenuWidth)
{

  Byte Attribute;
  Byte Title = FMNU_NONE_TITLE;

  Title     = fmnu_menu_data_ptr -> Title;
  Attribute = fmnu_menu_data_ptr -> TitleAttribute;

  fuim_SetAttributes(FUIM_ATTRIBUTES_NONE);
  plt_CCSetBackgroundColour(fuim_DynamicColours(Attribute)->BackGndColour);
  plt_CCSetForegroundColour(fuim_DynamicColours(Attribute)->ForeGndColour);

  plt_CCSetRowSize(FUIM_TOP_ROW_SIZE);

  /*-------------------------------------------------------------*/
  /* STRATEGY: [Clear Left] → [Draw] → [Clear Right] */
  /*-------------------------------------------------------------*/
  // Position at title row start
  plt_CCSetPosition(FUIM_TITLE_Y_POS, FUIM_TITLE_X_POS + FUIM_TITLE_LEFT_MARGIN);


  if ( Title != FMNU_NONE_TITLE ) {
      // Calculate title content width
      Word titleWidth = IMG_GET_WIDTH((img_storage_id_t)Title);
      Word alignedTitlePos =  (MenuWidth - (titleWidth + FUIM_TITLE_RIGHT_MARGIN));

      /*-------------------------------------------------------------*/
      /* AREA 1: Clear Left Padding                                   */
      /* From title start to aligned content position                 */
      /*-------------------------------------------------------------*/
    if ( alignedTitlePos > (FUIM_TITLE_X_POS + FUIM_TITLE_LEFT_MARGIN )) {
        fuim_DrawRepeatedCharacter(alignedTitlePos - (FUIM_TITLE_X_POS + FUIM_TITLE_LEFT_MARGIN));
    }
    /*-------------------------------------------------------------*/
    /* AREA 2: Draw Content                                        */
    /*-------------------------------------------------------------*/
    plt_CCSetRowSize((fuim_DynamicColours(Attribute)->Attribute) & (FUIM_ATTRIBUTES_ROW_SIZE));
    plt_CCSetPosition(FUIM_TITLE_Y_POS, alignedTitlePos);
    fuim_DrawTitle((img_storage_id_t )Title);

    /*-------------------------------------------------------------*/
    /* AREA 4: Clear Right Padding                                  */
    /* From title end to menu end                                   */
    /*-------------------------------------------------------------*/
    // Calculate title row end position
    Word titleRowEndPos =  MenuWidth - titleWidth;
    Word titleEndPos = fuim_GetColumnPosition();

    plt_CCSetRowSize(FUIM_TOP_ROW_SIZE);
    // Clear right padding: from title end to row end
    if ( titleRowEndPos > titleEndPos ) {
      plt_CCSetPosition(FUIM_TITLE_Y_POS, titleEndPos);
      fuim_DrawRepeatedCharacter(titleRowEndPos - titleEndPos);
    }


  } else {
    fuim_DrawRepeatedCharacter(MenuWidth - (FUIM_TITLE_X_POS + FUIM_TITLE_LEFT_MARGIN));
  }

}

/*=================================================================================
*   Function:    fmnu_DestroyTitle
*   Description: Clears the title bar area by filling it with background color.
*                Used when destroying a menu to remove the title.
*
* Arguments:
* Parameter      Flow    Description
* ------------------------------------------------------------------------------
* MenuWidth      IN      Total menu width in pixels
*
* Externals    Flow    Usage
* ------------------------------------------------------------------------------
* fmnu_menu_data_ptr IN  Provides Title/TitleAttribute/Xpos
*
* @return void
*
* @note Only clears if menu has a title (not FMNU_NONE_TITLE).
*       Positions at calculated title start position before clearing.
===================================================================================*/
void fmnu_DestroyTitle(Word MenuWidth)
{
  Byte Attribute;
  Byte Title = FMNU_NONE_TITLE;

  Title = fmnu_menu_data_ptr ->Title;

  if ( Title != FMNU_NONE_TITLE ) {
    Attribute = fmnu_menu_data_ptr -> TitleAttribute;

    fuim_SetAttributes(FUIM_ATTRIBUTES_NONE);
    plt_CCSetBackgroundColour(fuim_DynamicColours(Attribute)->BackGndColour);
    plt_CCSetForegroundColour(fuim_DynamicColours(Attribute)->ForeGndColour);

    plt_CCSetRowSize(FUIM_TOP_ROW_SIZE);
    plt_CCSetPosition(FUIM_TITLE_Y_POS, MenuWidth - (IMG_GET_WIDTH((img_storage_id_t)Title) + FUIM_TITLE_RIGHT_MARGIN));
    fuim_DrawRepeatedCharacter(IMG_GET_WIDTH((img_storage_id_t )Title));
  }


}


/*=================================================================================
*   Function:    fmnu_ConstructList
*   Description: Renders a list-type menu field by finding the matching list item
*                from the field's list table and displaying its string image.
*
* Arguments:
* Parameter      Flow    Description
* ------------------------------------------------------------------------------
* field_data_ptr IN      Pointer to field structure with list item table
* margin_top     IN      Vertical margin in pixels for string positioning
*
* Externals    Flow    Usage
* ------------------------------------------------------------------------------
* fmnu_str_List[] IN  ROM table mapping list item IDs to image string IDs
*
* @return void
*
* @note Returns immediately if GetFunction is AUIM_NO_GET_FUNCTION.
*       Uses linear search through ListItem table until FMNU_LIST_ITEMS_ERROR sentinel.
===================================================================================*/
void fmnu_ConstructList( fuimFieldStruct   *field_data_ptr, Byte margin_top )
 {
   if ( field_data_ptr->GetFunction == AUIM_NO_GET_FUNCTION ) {
     return;
   }

   Byte   value;
   fmnu_ListStruct const * pList;

   value = (Byte)fuim_Observer(field_data_ptr->GetFunction);
   pList = field_data_ptr->FieldCharacters.ListItem;

   while ( pList->ListItem != FMNU_LIST_ITEMS_ERROR ) {
     if ( value == pList->Value ) {
       fuim_ConstructStringVerticalMargin((img_storage_id_t)fmnu_str_List[pList->ListItem], margin_top, fuim_GetColumnPosition());
       break;
     }
     pList++;
   }
 }

/*=================================================================================
*   Function:    fmnu_GetListStringLen
*   Description: Returns the pixel width of the currently selected list item string
*                for a list-type field. Used for alignment calculation.
*
* Arguments:
* Parameter      Flow    Description
* ------------------------------------------------------------------------------
* field_data_ptr IN      Pointer to field structure with list item table
*
* Externals    Flow    Usage
* ------------------------------------------------------------------------------
* fmnu_str_List[] IN  ROM table mapping list item IDs to image string IDs
*
* @return Pixel width of the selected list item string, or 0 if not found
*
* @note Returns 0 if no match is found in the list table.
===================================================================================*/
Word fmnu_GetListStringLen(const fuimFieldStruct   *field_data_ptr )
 {
   Byte   value;
   fmnu_ListStruct const * pList;

   value = (Byte)fuim_Observer(field_data_ptr->GetFunction);
   pList = field_data_ptr->FieldCharacters.ListItem;

   while ( pList->ListItem != FMNU_LIST_ITEMS_ERROR ) 
   {
     if (value == pList->Value) {
       return IMG_GET_WIDTH((img_storage_id_t)fmnu_str_List[pList->ListItem]);
     }
     pList++;
   }
    return 0;
 }

/*=================================================================================
*   Function:    fmnu_ConstructMenu
*   Description: Builds the active menu from a menu structure. Populates displayed_fields[]
*                with valid fields, sets the active field, renders the title and fields,
*                and optionally configures button and fixed bottom fields.
*
* Arguments:
* Parameter      Flow    Description
* ------------------------------------------------------------------------------
* Menu           IN      Pointer to menu structure containing geometry and field array
*
* Externals    Flow    Usage
* ------------------------------------------------------------------------------
* fmnu_active_menu_handle IN/OUT  Set to 1 if FUIM_NO_ACTIVE_DIALOG
* fmnu_menu_data_ptr      OUT     Set to Menu
* fmnu_field_data_ptr     OUT     Set to Menu->MenuField
* MenuDialogProperties    OUT     Populated with field indices and geometry
* displayed_fields[]      OUT     Filled with pointers to valid fields
* active_field_ptr        OUT     Set to first valid/selectable field
*
* @return void
*
* @note Early returns if menu already active (fmnu_active_menu_handle != 0).
*       WARNING: displayed_fields[] write at line 1037 lacks bounds check vs FUIM_MAX_DISPLAY_FIELDS.
*       Starts menu timer if menu timeout > 0.
===================================================================================*/
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
  
 MenuDialogProperties.StartRow          = Menu -> Ypos;
 MenuDialogProperties.FirstPos          = Menu -> Xpos;
 MenuDialogProperties.EndPos            =(Menu -> Width) + (Menu -> Xpos);
 MenuDialogProperties.FieldWidth        = Menu -> Width; 

 MenuDialogProperties.FirstFieldNr      = 0;
 MenuDialogProperties.ActiveFieldNr     = 0;
 MenuDialogProperties.StartFieldNr      = 0;
 MenuDialogProperties.LastFieldNr       = 0;

  for (i = 0; i < (fmnu_menu_data_ptr->FieldCount); i++)
  {
    field_status = fuim_ValidityFunction (fmnu_field_data_ptr -> ValidityFunction);
    if ( field_status != FUIM_VALIDITY_NOTPRESENT )
    {

      displayed_fields[ MenuDialogProperties.LastFieldNr++ ] = fmnu_field_data_ptr ;

      if ( !first_field_set ) {
        MenuDialogProperties.FirstFieldNr = i ;
        first_field_set = TRUE ;
      }

      if ( (field_status == FUIM_VALIDITY_SELECTABLE ) && !active_field_set ) {
        MenuDialogProperties.ActiveFieldNr = i ;
        active_field_set = TRUE ;
      }
    }

    fmnu_field_data_ptr++;
  }

  if ( !first_field_set ) {
    fmnu_menu_data_ptr = NULL ;
    fmnu_active_menu_handle = FUIM_NO_ACTIVE_DIALOG ;
  } else
  {
      if ( !active_field_set ) {
         MenuDialogProperties.ActiveFieldNr = MenuDialogProperties.FirstFieldNr ;
      }

      active_field_ptr = displayed_fields [MenuDialogProperties.ActiveFieldNr];
      fmnu_ConstructTitle(MenuDialogProperties.FieldWidth);
      fmnu_DrawMenuFields(TRUE);

      if ( fuim_GetFixedBottomField(fmnu_menu_data_ptr) != 0 ) {
         fmnu_ConstructFixedField();
      }

      if ( fuim_GetLeftButtonField(fmnu_menu_data_ptr) != 0 ) {
         fmnu_ConstructButtonField(TRUE);
      }else {
         fmnu_DestroyButtonField(TRUE);
      }

      if (fuim_GetRightBottonField(fmnu_menu_data_ptr) != 0) {
         fmnu_ConstructButtonField(FALSE);
      }else {
         fmnu_DestroyButtonField(FALSE);
      }

      if (fuim_GetActiveMenuTimeout(fmnu_menu_data_ptr) > 0) {
         fmnu_menu_timer_handle = fuim_ConstructTimer ( fuim_GetActiveMenuTimeout(fmnu_menu_data_ptr)
                                                       , MENU_TIMER_FUNCTION
                                                       , fmnu_active_menu_handle );
      } else {
         fmnu_menu_timer_handle = FUIM_NO_FREE_TIMER_HANDLE;
      }

  }

}

/*=================================================================================
*   Function:    fmnu_DestroyMenu
*   Description: Removes the active menu from the display, destroys its timer,
*                and resets menu state to "no active menu".
*
* Arguments:        None
*
* Externals    Flow    Usage
* ------------------------------------------------------------------------------
* fmnu_active_menu_handle IN  Guard: returns if 0 or > FUIM_MAX_DISPLAY_MENUS
* fmnu_menu_data_ptr      IN  Provides Ypos/Xpos for clearing position
* fmnu_menu_timer_handle  OUT  Destroyed if not FUIM_NO_FREE_TIMER_HANDLE
*
* @return void
*
* @note Conditional compilation: FUIM_MULTILINE_FIELDS, FUIM_DESTROY_TITLE,
*       FUIM_DESTRTOY_BUTTON_FIELDS, FUIM_DESTRTOY_FIXED_FIELD.
*       Always clears fmnu_menu_data_ptr and fmnu_active_menu_handle on exit.
===================================================================================*/
void fmnu_DestroyMenu(void)
{

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

#ifdef FUIM_DESTROY_TITLE
  fmnu_DestroyTitle(MenuDialogProperties.FieldWidth );
#endif

//Destroy all multi-lines fields in menu, and remove line by line
#ifdef FUIM_MULTILINE_FIELDS
  Byte   i = 0;
  Byte   visiblefields;
  if ( fuim_GetMenuVisibleFields(fmnu_menu_data_ptr) == 0) {
     visiblefields = MenuDialogProperties.LastFieldNr;
  } else {
     visiblefields = fuim_GetMenuVisibleFields(fmnu_menu_data_ptr);
  }

  for ( i = MenuDialogProperties.StartFieldNr; i < (MenuDialogProperties.StartFieldNr + visiblefields); i++ ) {
     row = osd_row[ i ] ;
     plt_CCSetPosition( row , col );
     fmnu_DestroyField( &MenuDialogProperties );
  }
#else
  //Visible fields value is configured to one visible field
  //Destroy only one menu field that display at once
  fmnu_DestroyField();
#endif

//  Optimizing the rendering button fields of menu @TODO Need performed testing It was been added by UP
#ifdef FUIM_DESTRTOY_BUTTON_FIELDS
  if ( fuim_GetLeftButtonField(fmnu_menu_data_ptr) != 0 ) {
      fmnu_DestroyButtonField(TRUE);
  }

  if ( fuim_GetRightBottonField(fmnu_menu_data_ptr) != 0 ) {
     fmnu_DestroyButtonField(FALSE);
  }
#endif

#ifdef FUIM_DESTRTOY_FIXED_FIELD

  if ( fuim_GetFixedBottomField(fmnu_menu_data_ptr) != 0 ) {
      fmnu_DestroyFixedField();
  }
#endif


  if ( fmnu_menu_timer_handle != FUIM_NO_FREE_TIMER_HANDLE ) {
    fuim_DestroyTimer (&fmnu_menu_timer_handle);
  }

  fmnu_menu_data_ptr = NULL;
  fmnu_active_menu_handle = FUIM_NO_ACTIVE_DIALOG;

}

/*=================================================================================
*   Function:    fmnu_RemoveCurrentMenu
*   Description: Removes the currently displayed menu from the screen and resets
*                the active menu state marker.
*
* Arguments:        None
*
* Externals    Flow    Usage
* ------------------------------------------------------------------------------
* active_index_menu      OUT  Set to AUIM_MNU_MAX_MENUS (invalid/empty marker)
*
* @return void
*
* @note Calls fmnu_DestroyMenu() internally. Sets active_index_menu to sentinel value
*       indicating no menu is currently active.
===================================================================================*/
void fmnu_RemoveCurrentMenu(void)
{
   fmnu_DestroyMenu();
   active_index_menu = AUIM_MNU_MAX_MENUS; //does not indicate any active menu
}

/*=================================================================================
*   Function:    fmnu_Activate
*   Description: Activates a menu by index. Destroys any existing menu, reinitializes
*                menu system state, and constructs the new menu from auim_Menu[].
*
* Arguments:
* Parameter      Flow    Description
* ------------------------------------------------------------------------------
* IndexMenu      IN      menu_index_enum value identifying which menu to activate
*
* Externals    Flow    Usage
* ------------------------------------------------------------------------------
* active_index_menu      OUT  Set to IndexMenu
* auim_Menu[]           IN   Array of menu structures indexed by menu_index_enum
*
* @return void
*
* @note Index must be valid in auim_Menu[] (0..AUIM_MNU_MAX_MENUS-1).
*       Calls fmnu_RemoveCurrentMenu() then fmnu_InitMenus() before activation.
===================================================================================*/
void fmnu_Activate(menu_index_enum IndexMenu)
{
  fmnu_RemoveCurrentMenu();
  fmnu_InitMenus();

  active_index_menu = IndexMenu;

  fmnu_ConstructMenu((fmnu_MenuStruct *)&auim_Menu[IndexMenu]);
}

/*=================================================================================
*   Function:    fmnu_IsMenuActive
*   Description: Checks whether a specific menu is currently active by comparing
*                the requested index with the active menu index.
*
* Arguments:
* Parameter      Flow    Description
* ------------------------------------------------------------------------------
* idx            IN      Menu index to check
*
* Externals    Flow    Usage
* ------------------------------------------------------------------------------
* active_index_menu      IN   Current active menu index
*
* @return TRUE if idx matches the active menu and is not the sentinel value
*
* @note AUIM_MNU_MAX_MENUS is used as sentinel for "no active menu".
===================================================================================*/
Bool fmnu_IsMenuActive(menu_index_enum idx)
{
  return ((active_index_menu == (Byte)idx) && (active_index_menu != AUIM_MNU_MAX_MENUS));
}
/*=================================================================================
*   Function:    fmnu_UpdateMenu
*   Description: Updates the active menu display by redrawing fields if needed.
*                Skips update entirely when a modal indicator is active.
*
* Arguments:        None
*
* Externals    Flow    Usage
* ------------------------------------------------------------------------------
* fuim_IsModalIndicatorActive IN  Guard: returns immediately if modal active
* fmnu_active_menu_handle     IN  Guard: returns if FUIM_NO_ACTIVE_DIALOG
* fmnu_menu_data_ptr          IN  NULL check before drawing
* fmnu_full_redraw_menu_fields INOUT  Cleared to FALSE after redraw
*
* @return void
*
* @note Acts as dirty-flag updater: only redraws when fmnu_full_redraw_menu_fields
*       is TRUE. This flag is set by fmnu_ReDrawActiveFields() after modal dismissal.
===================================================================================*/
void fmnu_UpdateMenu(void)
{
  // NEW: Skip menu update if modal indicator is active
  if (fuim_IsModalIndicatorActive()) {
    return;
  }

  if ( fmnu_active_menu_handle != FUIM_NO_ACTIVE_DIALOG ) {
    if ( fmnu_menu_data_ptr != NULL ) {
        fmnu_DrawMenuFields(fmnu_full_redraw_menu_fields);
        fmnu_full_redraw_menu_fields = FALSE;
    }
  }

}

/*=================================================================================
*   Function:    fmnu_ReDrawActiveFields
*   Description: Marks the current menu for full redraw on the next update cycle.
*                Used to restore menu display after a modal indicator is hidden.
*
* Arguments:        None
*
* Externals    Flow    Usage
* ------------------------------------------------------------------------------
* fmnu_active_menu_handle      IN  Guard: only sets flag if menu is active
* fmnu_menu_data_ptr          IN   NULL check before setting flag
* fmnu_full_redraw_menu_fields OUT  Set to TRUE to trigger full redraw
*
* @return void
*
* @note This does not immediately redraw; it sets a dirty flag consumed by
*       fmnu_UpdateMenu() on the next cycle.
===================================================================================*/
void fmnu_ReDrawActiveFields(void)
{
  if ( fmnu_active_menu_handle != FUIM_NO_ACTIVE_DIALOG ) {
    if (fmnu_menu_data_ptr != NULL) {
        // Redraw active field with full redraw (bDraw = TRUE)
        fmnu_full_redraw_menu_fields = TRUE;
    }
  }
}

/*=================================================================================
*   Function:    fmnu_HandleCommand
*   Description: Processes a UI command within the current menu context. Determines
*                whether the command should be passed through based on key-press
*                state and configured repeat rules.
*
* Arguments:        None
*
* Externals    Flow    Usage
* ------------------------------------------------------------------------------
* aukh_GetCurrentCommand   IN   Source for current command
* aukh_FirstKeyPress       IN   TRUE if this is the first key press
* aukh_RepeatEvery         IN   Rate-limiting for key repeat
* active_index_menu       IN   Guard: command only passed if != AUIM_MNU_MAX_MENUS
*
* @return void
*
* @note Commands are passed only on first press or configured repeat interval.
*       Virtual keys 1, 2, 5 use 128ms repeat rate.
*       Delegates to fuim_ProcessMenuAction() when command is accepted.
*       Passes AU_IDLE_MENU_START/AU_MENU_CHARGE_BATTERY_ENTER for close/cancel.
===================================================================================*/
void fmnu_HandleCommand(void)
{

  Bool pass_command = FALSE;
  Byte command = aukh_GetCurrentCommand();

  if ( aukh_FirstKeyPress() )
  {
      pass_command = TRUE;
      fmnu_KeyPassAlways = FALSE;

  } else if( fmnu_KeyPassAlways ) {

      if( fmnu_KeyPassCommand == command ) {
        pass_command = TRUE;
      } else {
         fmnu_KeyPassAlways = FALSE;
      }

  } else {
     switch (command)
     {
        case AU_VIRTUAL_KEY_1:
        case AU_VIRTUAL_KEY_2:
        case AU_VIRTUAL_KEY_5:
        {
          if (aukh_RepeatEvery(AU_KEY_PRESSED_128_MSEC)) /* process with repeat rate 1 keys per second */
          {
              pass_command = TRUE;
          }

        } break;

        default:
          break;
      }
  }


  if ( pass_command ) {
      if ((active_index_menu != AUIM_MNU_MAX_MENUS)) {
         fuim_ProcessMenuAction(command);
      }
  }

}


/*=================================================================================
*   Function:    fmnu_ConstructMenuPrompt
*   Description: Renders the prompt text for a menu field and clears space between
*                prompt end and the specified end_position.
*
* Arguments:
* Parameter      Flow    Description
* ------------------------------------------------------------------------------
* field_data_ptr IN      Pointer to field structure containing Prompt ID
* end_position   IN      Right boundary after prompt drawing
*
* Externals    Flow    Usage
* ------------------------------------------------------------------------------
* fmnu_menu_data_ptr IN  Provides PromptColour/PromptPos
*
* @return void
*
* @note Prompt is skipped if Prompt ID evaluates below FMNU_NONE_PROMPT.
*       end_position is clamped to FUIM_MENU_WIDTH.
===================================================================================*/
void fmnu_ConstructMenuPrompt(fuimFieldStruct  *field_data_ptr, Word end_position )
{
  img_storage_id_t Prompt = fuim_GetPromptStringId(field_data_ptr->Prompt);

  if( Prompt < FMNU_NONE_PROMPT) {
    fuim_ConstructStringVerticalMargin( Prompt, 
                                        fuim_GetMenuPromptVertMaginTop(fmnu_menu_data_ptr),
                                        fuim_GetColumnPosition()
                                       );
  }

  /* Adjust if exceeding screen boundaries */
  if (end_position > FUIM_MENU_WIDTH) {
      end_position = FUIM_MENU_WIDTH;
  }

  Word current_position = fuim_GetColumnPosition();
  if ( current_position < end_position ) {
    fuim_DrawRepeatedCharacter(end_position - current_position); // Clear Right Padding ( prompt end to row end position )
  }

}


/*=================================================================================
*   Function:    fmnu_ConstructButtonField
*   Description: Renders a button field in the menu footer area (left or right half).
*                Supports text button rendering with alignment and padding.
*
* Arguments:
* Parameter      Flow    Description
* ------------------------------------------------------------------------------
* left           IN      TRUE for left button area, FALSE for right button area
*
* Externals    Flow    Usage
* ------------------------------------------------------------------------------
* fmnu_menu_data_ptr IN  Provides Xpos/Ypos/Width and LeftButtonField/RightButtonField
*
* @return void
*
* @note Returns early if no button field is defined (NULL).
*       Uses MenuYpos = FUIM_MENU_HEIGHT - FUIM_BOTTOM_FIELD_HEIGHT.
*       Applies button-specific colors from field or default AUIM_BUTTON_FIELD_COLOUR.
===================================================================================*/
void fmnu_ConstructButtonField(Bool left)
{
  fuimFieldStruct const * field_data_ptr;
  fuim_Validity            validity;

  Word  string_length = 0;
  Word  max_string_length = 0;

  Word  characters_left = 0;
  Word  characters_right = 0;

  Word  MenuXpos;
  Word  MenuYpos  = (FUIM_MENU_HEIGHT - FUIM_BOTTOM_FIELD_HEIGHT);
  Word  MenuWidth = fmnu_menu_data_ptr -> Width;

  Word  Attribute;
  Word  ForeGndColour;
  Word  BackGndColour;

  if ( left ) { //Set the Left Button field properties
      field_data_ptr = fmnu_menu_data_ptr -> LeftButtonField;
      MenuXpos  = fmnu_menu_data_ptr -> Xpos;
  } else {     //Set the Right Button field properties
      field_data_ptr = fmnu_menu_data_ptr -> RightButtonField;
      MenuXpos  = fmnu_menu_data_ptr -> Xpos + (FUIM_MENU_WIDTH / 2);
  }

  /*-------------------------------------------------------------*/
  /* Early exit if no button defined                             */
  /*-------------------------------------------------------------*/
  if (field_data_ptr == 0) {
      return;
  }

  /*-------------------------------------------------------------*/
  /* Set button area properties                                  */
  /*-------------------------------------------------------------*/
  Attribute     = fuim_GetFieldPromptColour(field_data_ptr) -> Attribute;
  ForeGndColour = fuim_GetFieldPromptColour(field_data_ptr) -> ForeGndColour;
  BackGndColour = fuim_GetFieldPromptColour(field_data_ptr) -> BackGndColour;

  plt_CCSetRowSize(Attribute & FUIM_ATTRIBUTES_ROW_SIZE);
  plt_CCSetBackgroundColour(BackGndColour);
  plt_CCSetForegroundColour(ForeGndColour);

  plt_CCSetPosition(MenuYpos, MenuXpos);

  validity = fuim_ValidityFunction(field_data_ptr-> ValidityFunction);
  if(validity == FUIM_VALIDITY_NOTPRESENT) {
      fuim_DrawRepeatedCharacter( (FUIM_MENU_WIDTH / 2));
      return;
  }

  // Calculate starting position based on alignment and button length
  max_string_length = (FUIM_MENU_WIDTH / 2);

  string_length     =  fuim_GetFieldValueLength(field_data_ptr, FALSE);
  string_length     = (string_length < max_string_length) ? (string_length) : (max_string_length);

  switch (field_data_ptr->Alignment)
  {
      case FUIM_ALIGNMENT_LEFT: {
         characters_left  = 0;
         characters_right = max_string_length - string_length;
      } break;

      case FUIM_ALIGNMENT_CENTRE: {
        characters_left  = ((max_string_length - string_length) / 2);
        characters_right = ((max_string_length - string_length + 1) / 2);
      }break;

      case FUIM_ALIGNMENT_RIGHT: {
        characters_left  = max_string_length - string_length;
        characters_right = 0;
     } break;

  }

  /*-------------------------------------------------------------*/
  /* AREA 1: Clear Left Padding Area                              */
  /* From button area start to aligned content position          */
  /*-------------------------------------------------------------*/
  // Clear left padding: from button start to aligned position
  fuim_SetColumnPosition(MenuXpos);
  fuim_DrawRepeatedCharacter(characters_left);

  /*-------------------------------------------------------------*/
  /* AREA 2: Apply Alignment and Draw Content                    */
  /*-------------------------------------------------------------*/
  // Position cursor at aligned location
  fuim_ConstructStringVerticalMargin( fuim_GetPromptStringId(field_data_ptr->Prompt)
                                    , FUIM_BUTTON_PROMPT_TOP_MARGIN
                                    , fuim_GetColumnPosition()
                                     );

  fuim_SetRowPosition(MenuYpos);
  fuim_DrawRepeatedCharacter(FUIM_BUTTON_PROMPT_TO_NAME_PADDING);

  fuim_ConstructStringVerticalMargin((img_storage_id_t) (field_data_ptr->FieldCharacters.Button)
                                , FUIM_BUTTON_NAME_TOP_MARGIN
                                , fuim_GetColumnPosition()
                                );

  /*-------------------------------------------------------------*/
  /* AREA 3: Clear Right Padding Area                             */
  /* From content end to button area end                          */
  /*-------------------------------------------------------------*/
  // Clear right padding: from content end to button area end
  fuim_SetRowPosition(MenuYpos);
  fuim_DrawRepeatedCharacter(characters_right);

  MenuWidth = MenuWidth;

}


/*=================================================================================
*   Function:    fmnu_DestroyButtonField
*   Description: Clears the button field area in the menu footer by filling with
*                background color. Removes both prompt and value areas.
*
* Arguments:
* Parameter      Flow    Description
* ------------------------------------------------------------------------------
* left           IN      TRUE for left button area, FALSE for right button area
*
* Externals    Flow    Usage
* ------------------------------------------------------------------------------
* fmnu_menu_data_ptr IN  Provides Xpos and field color information
*
* @return void
*
* @note Uses default button color if no field pointer is defined.
*       Clears half-screen width (FUIM_MENU_WIDTH / 2).
===================================================================================*/
void fmnu_DestroyButtonField(Bool left)
{
  fuimFieldStruct const * field_data_ptr;

  Word  MenuXpos;
  Word  MenuYpos  = (FUIM_MENU_HEIGHT - FUIM_BOTTOM_FIELD_HEIGHT);

  Word  Attribute;
  Word  ForeGndColour;
  Word  BackGndColour;

  if ( left )
  {
      //Set the Left Button field properties
      field_data_ptr = fmnu_menu_data_ptr -> LeftButtonField;
      MenuXpos       = fmnu_menu_data_ptr -> Xpos;
  }
  else
  {   //Set the Right Button field properties
      field_data_ptr = fmnu_menu_data_ptr -> RightButtonField;
      MenuXpos       = fmnu_menu_data_ptr -> Xpos + (FUIM_MENU_WIDTH / 2);
  }

  if (field_data_ptr == NULL)
  {
    fuimColourStruct * ButtonColour = fuim_DynamicColours(AUIM_BUTTON_FIELD_COLOUR);
    Attribute     = ButtonColour -> Attribute;
    ForeGndColour = ButtonColour -> ForeGndColour;
    BackGndColour = ButtonColour -> BackGndColour;
  }
  else
  {
    Attribute     = fuim_GetFieldPromptColour(field_data_ptr) -> Attribute;
    ForeGndColour = fuim_GetFieldPromptColour(field_data_ptr) -> ForeGndColour;
    BackGndColour = fuim_GetFieldPromptColour(field_data_ptr) -> BackGndColour;
  }

  plt_CCSetRowSize(Attribute & FUIM_ATTRIBUTES_ROW_SIZE);

  plt_CCSetPosition(MenuYpos, MenuXpos);

  plt_CCSetBackgroundColour(BackGndColour);
  plt_CCSetForegroundColour(ForeGndColour);

  fuim_DrawRepeatedCharacter(FUIM_MENU_WIDTH / 2);

}

/*=================================================================================
*   Function:    fmnu_DestroyFixedField
*   Description: Clears the fixed bottom field area (notification/footer row)
*                by filling it with background color.
*
* Arguments:        None
*
* Externals    Flow    Usage
* ------------------------------------------------------------------------------
* fmnu_menu_data_ptr IN  Provides FixedBottomField and field colors
*
* @return void
*
* @note Uses FUIM_MENU_FIELD_NOTIFICATION_Y_POS for vertical position.
*       Clears full menu width.
===================================================================================*/
void fmnu_DestroyFixedField(void)
{
  fuimFixedFieldStruct const * fixed_field_data_ptr;

  Word  MenuXpos;
  Word  MenuYpos;
  Word  MenuWidth;

  Word  Attribute;
  Word  ForeGndColour;
  Word  BackGndColour;


  fixed_field_data_ptr = fmnu_menu_data_ptr->FixedBottomField;
  MenuXpos  =  fmnu_menu_data_ptr -> Xpos;
  MenuWidth = fmnu_menu_data_ptr -> Width;
  MenuYpos  =   FUIM_MENU_FIELD_NOTIFICATION_Y_POS;


  Attribute = fuim_GetFieldPromptColour(fixed_field_data_ptr) -> Attribute;
  ForeGndColour = fuim_GetFieldPromptColour( fixed_field_data_ptr) -> ForeGndColour  ;
  BackGndColour = fuim_GetFieldPromptColour( fixed_field_data_ptr) -> BackGndColour;


  plt_CCSetRowSize(Attribute & FUIM_ATTRIBUTES_ROW_SIZE);

  fuim_SetRowPosition(MenuYpos);
  fuim_SetColumnPosition(MenuXpos);

  plt_CCSetBackgroundColour(BackGndColour);
  plt_CCSetForegroundColour(ForeGndColour);

  fuim_DrawRepeatedCharacter(MenuWidth);

}

/*=================================================================================
*   Function:    fmnu_ConstructFixedField
*   Description: Renders the fixed bottom notification field at the bottom of the menu.
*                Supports alignment (LEFT/CENTRE/RIGHT) with clearing/padding.
*
* Arguments:        None
*
* Externals    Flow    Usage
* ------------------------------------------------------------------------------
* fmnu_menu_data_ptr IN  Provides FixedBottomField/Xpos/Width
*
* @return void
*
* @note Skips rendering if field validity is FUIM_VALIDITY_NOTPRESENT.
*       Skips if GetPromptStringId returns IMG_INVALID_ID.
*       Positions at FUIM_MENU_FIELD_NOTIFICATION_Y_POS.
===================================================================================*/
void fmnu_ConstructFixedField(void)
{

  fuimFixedFieldStruct const * fixed_field_data_ptr;

  Word  string_length = 0;

  Word  characters_left = 0;
  Word  characters_right = 0;

  Word  MenuXpos;
  Word  MenuYpos;
  Word  MenuWidth;

  Word  Attribute;
  Word  ForeGndColour;
  Word  BackGndColour;
  Byte  MenuValidity;

  //Set the Bottom field properties
  fixed_field_data_ptr = fmnu_menu_data_ptr->FixedBottomField;
  MenuXpos             = fmnu_menu_data_ptr -> Xpos;
  MenuWidth            = fmnu_menu_data_ptr -> Width;
  MenuYpos             = FUIM_MENU_FIELD_NOTIFICATION_Y_POS;

  Attribute     = fuim_GetFieldPromptColour( fixed_field_data_ptr ) -> Attribute;
  ForeGndColour = fuim_GetFieldPromptColour( fixed_field_data_ptr ) -> ForeGndColour;
  BackGndColour = fuim_GetFieldPromptColour( fixed_field_data_ptr ) -> BackGndColour;

  plt_CCSetPosition(MenuYpos, MenuXpos);
  plt_CCSetRowSize((Byte)( Attribute & FUIM_ATTRIBUTES_ROW_SIZE ));

  plt_CCSetForegroundColour(ForeGndColour);
  plt_CCSetBackgroundColour(BackGndColour);

  MenuValidity = fuim_ValidityFunction(fixed_field_data_ptr -> ValidityFunction);
  if( MenuValidity == FUIM_VALIDITY_NOTPRESENT)
  {
      fuim_DrawRepeatedCharacter(MenuWidth);
      return;
  }
  
   /*=============================================================*/
   /*              NORMAL SINGLE ROW DISPLAY                      */
   /*           (Three-Area Clearing Implementation)              */
   /*=============================================================*/
    /* Get the field value via the observer function */
  img_storage_id_t value = fuim_GetPromptStringId(fixed_field_data_ptr->Prompt);

  if (value == IMG_INVALID_ID) {
     fuim_DrawRepeatedCharacter( MenuWidth);
     return;
  }

  Word max_string_length = MenuWidth;
  string_length = IMG_GET_WIDTH(value);
  string_length = (string_length < max_string_length) ? (string_length) : (max_string_length);


  switch (fixed_field_data_ptr->Alignment)
  {
      case FUIM_ALIGNMENT_LEFT: {
         characters_left  = 0;
         characters_right = max_string_length - string_length;
      } break;

      case FUIM_ALIGNMENT_CENTRE: {
        characters_left  = ((max_string_length - string_length) / 2);
        characters_right = ((max_string_length - string_length + 1) / 2);
      }break;

      case FUIM_ALIGNMENT_RIGHT: {
        characters_left  = max_string_length - string_length;
        characters_right = 0;
     } break;

  }

  /*-------------------------------------------------------------*/
  /* AREA 1: Clear Left Padding Area                              */
  /* From  area start to aligned content position          */
  /*-------------------------------------------------------------*/
  // Clear left padding: from  start to aligned position
  fuim_SetColumnPosition(MenuXpos);
  fuim_DrawRepeatedCharacter(characters_left);

  /*-------------------------------------------------------------*/
   /* AREA 2: Apply Alignment and Draw Content                    */
   /*-------------------------------------------------------------*/
   // Position cursor at aligned location
   fuim_ConstructStringVerticalMargin( value
                                     , FUIM_MENU_FIELD_NOTIFICATION_TOP_MARGIN
                                     , fuim_GetColumnPosition()
                                     );

   /*-------------------------------------------------------------*/
    /* AREA 3: Clear Right Padding Area                             */
    /* From content end to area end                          */
    /*-------------------------------------------------------------*/
    // Clear right padding: from content end to button area end
    fuim_SetRowPosition(MenuYpos);
    fuim_DrawRepeatedCharacter(characters_right);

}

