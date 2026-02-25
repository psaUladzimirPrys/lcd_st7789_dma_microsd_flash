#ifndef _HFMNU_H
#define _HFMNU_H

/*=======================================================================*/
/*        I N C L U D E S                                                */
/*=======================================================================*/
#include "hglobal.h"
#include "fuim.h"
#include "auim_mnu.h"



/*=======================================================================*/
/* G L O B A L   R E F E R E N C E S                                     */
/*=======================================================================*/


/*=======================================================================*/
/* G L O B A L   D E F I N I T I O N S                                   */
/*=======================================================================*/
#define FMNU_MAIN_MENU      0
#define FMNU_SUB_MAIN_MENU  1
#define FMNU_MESSAGE_BOARD  2

#define FMNU_NONE_PROMPT IMG_ID_COUNT
#define FMNU_NONE_TITLE  FMNU_NONE_PROMPT

/*==========================================================================*/
/*        G L O B A L   D A T A   D E C L A R A T I O N S                   */
/*==========================================================================*/


typedef struct
{
    Byte Type;   /* Menu type {FMNU_MAIN_MENU, FMNU_SUB_MAIN_MENU, FMNU_MESSAGE_BOARD} */

    Byte Xpos;   // X position
    Byte Ypos;   // Y position
    Byte Width;  // Menu width

    Byte Title;  // Title string ID

    Word TitleAttribute;
    /* Title rendering attributes - fuim_Attributes
       Includes foreground/background color, size, shadow, etc. */

    Byte PromptPos;
    Byte ValuePos;

    fuimFieldStruct const *MenuField;
    // Pointer to an array of field structures

    Byte VisibleFields;  // Number of fields visible at once

    Byte FieldCount;     // Total number of fields

    Byte TimeOut;
    // How long the menu remains active;
    // if 0 — stays active indefinitely

    fuimFixedFieldStruct const *FixedTopField;
    /* Pointer to structure of the fixed top field */

    fuimFixedFieldStruct const *FixedBottomField;
    /* Pointer to structure of the fixed bottom field */

} fmnu_MenuStruct;


/***********************************************************************************

    Structure fuim_MenuProperty
    This structure contains the values required to render each field in the dialog.
    The position and length of the prompt- and value-cells of each field
    must be identical.
    They may differ between dialogs.

*************************************************************************************/

typedef struct
{
    Byte FirstPos;   /* First column position used by the menu/all fields
                        equals MenuDataPtr->HorLocation */

    Byte PromptPos;  /* Column position where the prompt starts */
    Byte ValuePos;   /* Position where the value starts */
    Byte EndPos;     /* Position of the end-box character */
    Byte ValueWidth; /* Total width of the value */
    Byte FieldWidth; /* Total width of the field */

    Byte FirstFieldNr;  /* Number of the first drawn field of the menu */
    Byte LastFieldNr;   /* Number of the last drawn field of the menu */
    Byte ActiveFieldNr; /* Number of the highlighted field of the menu */

    /* Number of the currently drawn field of the menu.
       Required for scroll rendering — shows how many positions
       have passed from the beginning */

    Byte StartFieldNr;  /* Number of the first field visible in the menu window */

} fmnu_MenuProperty;

void fmnu_InitMenus(void);
void fmnu_UpdateMenu( void);
void fmnu_ConstructMenu(fmnu_MenuStruct *Menu);

void fmnu_HandleCommand(void);
void fmnu_RemoveCurrentMenu(void);
void fmnu_Activate(menu_index_enum IndexMenu);

void fmnu_HandleCommand(void);


#endif
