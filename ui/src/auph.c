/*
 * auph.c
 *
 *  Created on: 20 Feb. 2026 г.
 *      Author: priss
 */

/*=======================================================================*/
/*        I N C L U D E S                                                */
/*=======================================================================*/

#include "rbsc_api.h"
#include "aukh.h"
#include "auph.h"
#include "fmnu.h"
#include "auim_mnu.h"
#include "find_api.h"


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
#define X_NOT_IN_TEXT            0x04
#define X_IN_MENU                0x08
#define X_NOT_IN_MENU            0x10
#define X_IN_STANDBY             0x20
#define X_IN_IDLE                0x40
#define X_IN_SERVICE             0x80


#define PERMISSION_STANDBY            X_IN_STANDBY + X_IN_SERVICE  
#define PERMISSION_DIRECT             X_IN_IDLE + X_NOT_IN_MENU
#define PERMISSION_IDLE               X_IN_IDLE
#define PERMISSION_MENU               X_REPEAT + X_NOT_IN_TEXT + X_IN_MENU  
#define PERMISSION_DIRECT_MENU        X_IN_TEXT
#define PERMISSION_SERVICE            X_IN_STANDBY + X_NOT_IN_MENU
#define PERMISSION_PROTECTION         X_ALWAYS


#define LENGTH_KEY_GROUPCODE_TABLE  ( sizeof(key_groupcode_table) / \
                                      sizeof(auphKeyGroup) )
/*==========================================================================*/
/*     L O C A L   F U N C T I O N S   P R O T O T Y P E S                  */
/*==========================================================================*/

static void HandleIdleKey(void);
static void HandleStandby(void);
static void HandleMenu(void);
static void HandleDirectKey(Byte key_group);
static void HandleService(void);
static void HandleDirect(void);


/*==========================================================================*/
/*     L O C A L   D A T A   D E F I N I T I O N S                          */
/*==========================================================================*/
/******************************************************************************
    @struct auphKeyGroup | In the structure, the first field contains the key code.
            The second field contains the group code to which the key belongs.
******************************************************************************/
typedef struct
{
   Byte key;   /* @field action action field (key number) */
   Byte group; /* @field function function field to be executed when the key is pressed.
                              This is an index in the 'Observer' function table */
} auphKeyGroup;
/*EMP=======================================================================*/


/* ROM table containing the code group for each key code. */
static auphKeyGroup const key_groupcode_table[] = {

 { AU_KEY_START,    AU_GROUP_MENU        }
,{ AU_KEY_NO,       AU_GROUP_MENU        }
,{ AU_KEY_CLOSE,    AU_GROUP_MENU        }
,{ AU_KEY_PERF_CHK, AU_GROUP_IDLE        }
,{ AU_KEY_NEXT,     AU_GROUP_MENU        }
,{ AU_KEY_CANCEL,   AU_GROUP_MENU        }
,{ AU_KEY_YES,      AU_GROUP_MENU        }
,{ AU_KEY_PARAMS,   AU_GROUP_IDLE        }

,{ AU_KEY_STANDBY,  AU_GROUP_STANDBY     }
,{ AU_KEY_MENU,     AU_GROUP_MENU        }
,{ AU_KEY_SERVICE,  AU_GROUP_SERVICE     }

};


/* ROM table containing permissions for each key group code. */
static Byte const permission_table[] =
{

   PERMISSION_IDLE,               /* AU_GROUP_IDLE               */
   PERMISSION_STANDBY,            /* AU_GROUP_STANDBY            */
   PERMISSION_DIRECT,             /* AU_GROUP_DIRECT             */
   PERMISSION_MENU,               /* AU_GROUP_MENU               */
   PERMISSION_SERVICE,            /* AU_GROUP_SERVICE            */

};


/* ROM table containing function pointers for each key group code. */
static const VOID_FUNCTION_PTR direct_function_table[] =
{
   HandleIdleKey,                /* AU_GROUP_IDLE                 */
   HandleStandby,                /* AU_GROUP_STANDBY              */
   HandleDirect,                 /* AU_GROUP_DIRECT               */
   HandleMenu,                   /* AU_GROUP_MENU                 */
   HandleService,                /* AU_GROUP_SERVICE              */
};

/*==========================================================================*/
/*     L O C A L   S Y M B O L   D E C L A R A T I O N S                    */
/*==========================================================================*/

static auphOsteoState_enum  auph_state;
//static Bool auph_key_repetition_allowed;


/*=======================================================================*/
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
/*
   if (aukh_FirstKeyPress())
   {
      if( ( aukh_GetCurrentCommand() == AU_KEY_STANDBY )&&
          ( fpmt_GetPowerState() == FPMT_POWER_ON )      )
      {
      fpmt_SetPowerState(FPMT_STAND_BY);

      }
   }
*/
  return;
}

/*========================================================================
  Empty function with no action for future use
  Returns nothing
  Belongs to component: auph
========================================================================**/
static void HandleIdleKey(void)
{
  return;
}

/*========================================================================
  Empty function with no action for future use
  Returns nothing
  Belongs to component: auph
========================================================================**/
static void HandleDirect(void)
{
  return;
}
/*========================================================================
  Empty function with no action for future use
  Returns nothing
  Belongs to component: auph
========================================================================**/
static void HandleService(void)
{
  return;
}

/*******************************************************************************
   @func   This function handles the Menu command

   @rdesc  Nothing is returned by this function

   @comm   Function belongs to component: auph

******************************************************************************/
static void HandleMenu(void)
{


 if (aukh_KeyHold(AU_KEY_PRESSED_ONE_SECOND)) {

   fmnu_Activate(AUIM_MNU_INDEX_CONFIG_MENU);
   auph_SetState(AU_MENU_STATE);
 }
/* else
 {
     if(aukh_FirstKeyPress())
       {

  find_DirectIndicators(rbsc_ChangeControlAround(find_GetIndicatorFocus(),
                                                  0,
                                                 (FIND_DIRECT_MAX_INDICATOR - 1),
                                                  0));
       }
   }*/
}

/*************************************************************************
           Processing of user keys when not in:<nl>
              - standby mode<nl>
              - menu mode<nl>
              - text mode<nl><nl>
             So, all direct access functions are processed.

           Returns nothing
           Belongs to component: auph
***************************************************************************/

 static void HandleDirectKey(Byte key_group)
{
   direct_function_table[key_group]();
}


/******************************************************************************
******************************************************************************/
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

       if ((permission & X_REPEAT) || aukh_FirstKeyPress())/*Checks if the entered key is pressed for the first time*/
       {
           /* If the button is pressed for the first time or its repetition is allowed */
            switch ( auph_GetState() )
            {
              case AU_IDLE_STATE: {
               if (permission & PERMISSION_IDLE)
               {
                   HandleDirectKey(key_groupcode_table[index].group);
               }
              } break;

              case AU_MENU_STATE:{
                 if (permission & X_IN_MENU)
                 {
                /*  Menu is active, and the key is allowed in menu   */
                   fmnu_HandleCommand();

                  } else
                  {
                     if (!(permission & X_NOT_IN_MENU))
                     { /*Not a menu key but allowed in menu*/
                      /* Not a menu key but allowed in menu       */
                      HandleDirectKey(key_groupcode_table[index].group);
                     }
                  }

                }break;

                 case AU_DIRECT_STATE:{

                   HandleDirectKey(key_groupcode_table[index].group);

                 }break;

                  default:{
                            ; /* No action, because the key is NOT allowed in the current state */
                           }

             }//End brace of switch(auph_GetState)

         }      /* End: repetition allowed or first key and normal key handling */
    }         /* End: Valid key entered */

    au_current.command = AU_KEY_PROCESSED;
 }
