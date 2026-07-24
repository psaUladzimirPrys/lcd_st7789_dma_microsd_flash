/*
 * auph.c
 *
 *  Created on: 20 Feb. 2026 г.
 *      Author: priss
 */

/*=======================================================================*/
/*        I N C L U D E S                                                */
/*=======================================================================*/
#include "app_log.h"
#include "rbsc_api.h"
#include "aukh.h"
#include "auph.h"
#include "fmnu.h"
#include "auim_mnu.h"
#include "find_api.h"
#include "fpmt_api.h"
#include "fsrv.h"
#include "disp.h"
#include "aevs.h"


/*==========================================================================*/
/*     G L O B A L   D E F I N I T I O N S                                  */
/*==========================================================================*/
typedef void (* VOID_FUNCTION_PTR)(void);

/*==========================================================================*/
/*     L O C A L   S Y M B O L   D E C L A R A T I O N S                    */
/*==========================================================================*/
#define X_ALWAYS                 0x00 
#define X_REPEAT                 0x01 
#define X_IN_TEXT                0x02
#define X_NOT_IN_DIRECT          0x04
#define X_IN_MENU                0x08
#define X_NOT_IN_MENU            0x10
#define X_IN_STANDBY             0x20
#define X_IN_IDLE                0x40
#define X_IN_SERVICE             0x80


#define PERMISSION_IDLE          (X_REPEAT | X_IN_IDLE | X_IN_MENU | X_NOT_IN_MENU)
#define PERMISSION_STANDBY       (X_IN_STANDBY)
#define PERMISSION_DIRECT        (X_IN_IDLE)
#define PERMISSION_MENU          (X_REPEAT | X_IN_MENU | X_NOT_IN_MENU)
#define PERMISSION_ERROR         (X_ALWAYS)

//#define PERMISSION_DIRECT_MENU   (X_NOT_IN_DIRECT)


#define LENGTH_KEY_GROUPCODE_TABLE  ( sizeof(key_groupcode_table) / \
                                      sizeof(auphKeyGroup) )
/*==========================================================================*/
/*     L O C A L   F U N C T I O N S   P R O T O T Y P E S                  */
/*==========================================================================*/

static void HandleIdleKey(void);
static void HandleStandby(void);
static void HandleMenu(void);
static void HandleDirectKey(Byte key_group);
static void HandleError(void);
static void HandleDirect(void);


/*==========================================================================*/
/*     L O C A L   D A T A   D E F I N I T I O N S                          */
/*==========================================================================*/
/*===========================================================================
    @struct auphKeyGroup | In the structure, the first field contains the key code.
            The second field contains the group code to which the key belongs.
===========================================================================*/
typedef struct
{
   Byte key;   /* @field action action field (key number) */
   Byte group; /* @field function function field to be executed when the key is pressed. */
               /* This is an index in the 'Observer' function table */
} auphKeyGroup;

/*EMP=======================================================================*/

/* AU_VIRTUAL_KEY_1  AU_KEY_PRESS_SHORT         */
/* AU_VIRTUAL_KEY_2  AU_KEY_PRESS_LONG          */
/* AU_VIRTUAL_KEY_3  AU_KEY_PRESS_VERY_LONG     */
/* AU_VIRTUAL_KEY_4  AU_KEY_PRESS_MULTI_3_TIME  */
/* AU_VIRTUAL_KEY_5  AU_KEY_PRESS_MULTI_5_TIME  */
/* AU_VIRTUAL_KEY_6  AU_KEY_PRESS_MULTI_2_TIME  */

/*=======================================================================*/
/* ROM table containing the code group for each key code. */
static auphKeyGroup const key_groupcode_table[] = {

 { AU_VIRTUAL_KEY_1,                       AU_GROUP_MENU }
,{ AU_VIRTUAL_KEY_2,                       AU_GROUP_MENU }
,{ AU_VIRTUAL_KEY_4,                       AU_GROUP_IDLE }
,{ AU_VIRTUAL_KEY_3,                    AU_GROUP_STANDBY }
,{ AU_VIRTUAL_KEY_5,                       AU_GROUP_IDLE }
,{ AU_VIRTUAL_KEY_6,                       AU_GROUP_IDLE }

// AU_ERROR_STATE
,{ AU_ERROR_STATE_SET,                    AU_GROUP_ERROR }
// AU_GROUP_PAIRING
,{ AU_PAIRING_MENU_OK,                     AU_GROUP_MENU }
,{ AU_PAIRING_MENU_FAILED,                 AU_GROUP_MENU }
// AU_GROUP_PERFORMANCE
,{ AU_PERFORMANCE_MENU_START,              AU_GROUP_MENU }
,{ AU_PERFORMANCE_MENU_FINISHED,           AU_GROUP_MENU }
,{ AU_PERFORMANCE_MENU_CHECK_END,          AU_GROUP_MENU }
// AU_GROUP_REFERENCE
,{ AU_REFERENCE_MENU_START,                AU_GROUP_MENU }
,{ AU_REFERENCE_MENU_FINISHED,             AU_GROUP_MENU }
,{ AU_REFERENCE_PERFORM_MENU_REPEAT_START, AU_GROUP_MENU }
,{ AU_REFERENCE_PATIENT_MENU_REPEAT_START, AU_GROUP_MENU }
 // AU_GROUP_PATIENT
,{ AU_PATIENT_MENU_START,                  AU_GROUP_MENU }
,{ AU_PATIENT_MENU_FINISHED,               AU_GROUP_MENU }
,{ AU_PATIENT_MENU_CHECK_END,              AU_GROUP_MENU }
,{ AU_PATIENT_MENU_FAILED,                 AU_GROUP_MENU }
,{ AU_PATIENT_MENU_VALIDATING,             AU_GROUP_MENU }
,{ AU_PATIENT_MENU_VALIDATING_RESULT,      AU_GROUP_MENU }

,{ AU_IDLE_MENU_START,                   AU_GROUP_DIRECT }

,{ AU_MENU_CHARGE_BATTERY_ENTER,           AU_GROUP_IDLE }
,{ AU_MENU_CHARGE_BATTERY_CANCELED,        AU_GROUP_MENU }

};

/*=======================================================================*/
/* ROM table containing permissions for each key group code. */
static Byte const permission_table[] =
{

   PERMISSION_IDLE,               /* AU_GROUP_IDLE               */
   PERMISSION_STANDBY,            /* AU_GROUP_STANDBY            */
   PERMISSION_DIRECT,             /* AU_GROUP_DIRECT             */
   PERMISSION_MENU,               /* AU_GROUP_MENU               */
   PERMISSION_ERROR,              /* AU_GROUP_ERROR              */

};

/*=======================================================================*/
/* ROM table containing function pointers for each key group code. */
static const VOID_FUNCTION_PTR direct_function_table[] =
{
   HandleIdleKey,                /* AU_GROUP_IDLE                 */
   HandleStandby,                /* AU_GROUP_STANDBY              */
   HandleDirect,                 /* AU_GROUP_DIRECT               */
   HandleMenu,                   /* AU_GROUP_MENU                 */
   HandleError,                  /* AU_GROUP_ERROR                */
};

/*==========================================================================*/
/*     L O C A L   S Y M B O L   D E C L A R A T I O N S                    */
/*==========================================================================*/

static auphOsteoState_enum  auph_state = AU_STANDBY_STATE;


/*========================================================================
   @func   Returns the current OsteoProbe state
   @comm   Function belongs to component: auph
========================================================================*/
auphOsteoState_enum auph_GetState(void)
{
   return auph_state;
}

/*========================================================================
   @func   Set the new OsteoProbe state
   @comm   Function belongs to component: auph
========================================================================*/
void auph_SetState(auphOsteoState_enum new_state)
{
   auph_state = new_state;
}


/*========================================================================
  Function processes standby command
  Returns nothing
  Belongs to component: auph
========================================================================*/
static void HandleStandby(void)
{
   app_log("UI HandleStandby\r\n");
   fpmt_HandleCommand();
}

/*========================================================================
  Empty function with no action for future use
  Returns nothing
  Belongs to component: auph
========================================================================**/
static void HandleIdleKey(void)
{
  app_log("UI HandleIdleKey\r\n");
//  if (aukh_FirstKeyPress()) {
//    if (   ( fpmt_GetPowerState() == FPMT_POWER_ON )
//        && ( aukh_GetCurrentCommand() == AU_MENU_CHARGE_BATTERY_ENTER)
//    ) {
//        fmnu_Activate(AUIM_MNU_INDEX_CHARGE_BATTERY_MENU);
//    }
//  }

}

/*========================================================================
  Empty function with no action for future use
  Returns nothing
  Belongs to component: auph
========================================================================**/
static void HandleDirect(void)
{
 app_log("UI HandleDirect\r\n");
 if ( fpmt_GetPowerState() == FPMT_POWER_ON )
   if (aukh_FirstKeyPress()) {
      if( aukh_GetCurrentCommand() == AU_IDLE_MENU_START) {
         find_RemoveAllNotificationIndicators();
         fmnu_Activate(AUIM_MNU_INDEX_IDLE_MENU);
         auph_SetState(AU_IDLE_STATE);
      }
  }

}
/*========================================================================
  @brief Error state handler - executed via AU_ERROR_STATE_SET
  
  Triggered by fsrv_ErrorManager through aukh_SetSimulatedKey(AU_ERROR_STATE_SET).
  Executes ONCE on first key press via aukh_FirstKeyPress() check in auph_ProcessKey().
  
  Sets critical error state and clears UI (find_RemoveAllIndicators, 
  fmnu_RemoveCurrentMenu, find_DisplayIndicator(FIND_ID_ERROR)).
  
  System remains in fatal error state until power cycle.
  
  Returns nothing
  Belongs to component: auph
========================================================================**/
static void HandleError(void)
{

  app_log("UI HandleError\r\n");
  if (aukh_FirstKeyPress()) {

      /* Clear all UI */
    find_RemoveAllIndicators();
    fmnu_RemoveCurrentMenu();
    disp_FillScreen(ST7789_WHITE);

    /* Display error indicator */
    find_DisplayIndicator(FIND_ID_ERROR);

    /* Clear source error code after Displaying error indicator for next cycle */
    fsrv_DS_SetErrorCode(ERR_CODE_NONE);

    /* Set critical error state */
    fpmt_SetPowerState(FPMT_ERROR);
  }
}

/*========================================================================
   @func   This function handles the Menu command

   @rdesc  Nothing is returned by this function

   @comm   Function belongs to component: auph

========================================================================*/
static void HandleMenu(void)
{
  app_log("UI HandleMenu\r\n");
  return;
}

/*========================================================================
   Processing of user keys when not in:<nl>
      - standby mode<nl>
      - menu mode<nl>
      - text mode<nl><nl>
     So, all direct access functions are processed.

   Returns nothing
   Belongs to component: auph
========================================================================*/
static void HandleDirectKey(Byte key_group) {
   direct_function_table[key_group]();
}


/*========================================================================
 *
 *
 *
 *
 *
========================================================================*/
 void auph_ProcessKey(void)
 {
    Byte permission;
    Byte index;

    index = 0;

    while ((key_groupcode_table[index].key != au_current.command) &&
           (index < LENGTH_KEY_GROUPCODE_TABLE))
    {
       index ++;
    }

    if (index != LENGTH_KEY_GROUPCODE_TABLE)
    {

      permission = permission_table[key_groupcode_table[index].group];

      app_log("Key %d, Group %d, Permission 0x%X\r\n",key_groupcode_table[index].key, key_groupcode_table[index].group, permission);

       if ((permission & X_REPEAT) || aukh_FirstKeyPress())/*Checks if the entered key is pressed for the first time*/
       {
           /* If the button is pressed for the first time or its repetition is allowed */
           switch ( auph_GetState() )
           {

             case AU_STANDBY_STATE: {
                if(aukh_FirstKeyPress())
                {
                  if((permission & X_IN_STANDBY)) /*Power On*/
                  {
                      HandleDirectKey(key_groupcode_table[index].group);
                  }
                }
             } break;

            case AU_IDLE_STATE: {
                if (permission & X_IN_MENU) {
               /*  Menu is active, and the key is allowed in menu   */
                  fmnu_HandleCommand();

                 } else {
                    if (!(permission & X_NOT_IN_MENU)) {
                     /* Not a menu key but allowed in menu       */
                     HandleDirectKey(key_groupcode_table[index].group);
                    }
                 }

            } break;

            case AU_MENU_STATE: {
                if (permission & X_IN_MENU) {
               /*  Menu is active, and the key is allowed in menu   */
                  fmnu_HandleCommand();

                 } else {
                    if (!(permission & X_NOT_IN_MENU)) {
                     /* Not a menu key but allowed in menu       */
                     HandleDirectKey(key_groupcode_table[index].group);
                    }
                 }

             } break;
             default:{
                 ; /* No action, because the key is NOT allowed in the current state */
                     }

             }//End brace of switch(auph_GetState)

         }      /* End: repetition allowed or first key and normal key handling */
    }         /* End: Valid key entered */

    au_current.command = AU_KEY_PROCESSED;
 }
