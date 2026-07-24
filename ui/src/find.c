/*=======================================================================*/

/*=======================================================================*/
/*        I N C L U D E S                                                */
/*=======================================================================*/
#include <global.h>
#include "stddef.h"
#include "aukh.h"
#include "auph.h"
#include "fuim.h"
#include "auim_api.h"
#include "auim_ind.h"


#include "find_api.h"
#include "fuim_obs.h"
//#include "fuim_trs.h"

/*=======================================================================*/
/* G L O B A L   R E F E R E N C E S                                     */
/*=======================================================================*/

/*=======================================================================*/
/* G L O B A L   D E F I N I T I O N S                                   */
/*=======================================================================*/

#define EMPTY_INDICATOR          0xFF  /* No indicator - marks empty slot in active_indicators array */
#define NR_OF_DOUBLE_INDICATORS  0xFF  /* Threshold for double indicator indexing */

#define INDICATOR_TIME_OUT       0x03  /* Timeout value in seconds */
#define INDICATOR_NO_TIME_OUT    0x00  /* No timeout */

/*=======================================================================*/
/* L O C A L   D E F I N I T I O N S                                     */
/*=======================================================================*/
/*=======================================================================*/
/* L O C A L   S Y M B O L   D E C L A R A T I O N S                     */
/*=======================================================================*/
/*=======================================================================*/
/* L O C A L   D A T A   D E F I N I T I O N S                           */
/*=======================================================================*/
/*--- Static variables ---*/

static Byte active_indicators[FUIM_MAX_INDICATORS] =
{
    EMPTY_INDICATOR,
    EMPTY_INDICATOR,
    EMPTY_INDICATOR,
    EMPTY_INDICATOR,
    EMPTY_INDICATOR,
    EMPTY_INDICATOR,
    EMPTY_INDICATOR
};

static find_Direct_id_enum find_IndicatorFocus;
static find_id_enum find_DirectIndicators_Ids[FIND_DIRECT_MAX_INDICATOR] =
{
  FIND_ID_CHARGE_BATT
};


#define FUIM_MAX_NOTIFICATION_INDICATORS  11
const find_id_enum notification_indicators[FUIM_MAX_NOTIFICATION_INDICATORS] = {
                                                 FIND_ID_CHARGE_BATT
                                               , FIND_ID_PERFORMANCE
                                               , FIND_ID_PATIENT
                                               , FIND_ID_REFERENCE
                                               , FIND_ID_ONE_INDENT_REQUIRED
                                               , FIND_ID_TIP_ID_VALID
                                               , FIND_ID_TIP_ID_INVALID
                                               , FIND_ID_TIP_ID_USED
                                               , FIND_ID_UNSTABLE
                                               , FIND_ID_TERMINATE
                                               , FIND_ID_PERF_CHK_RECOMMENDED
                                           };

/*=======================================================================*/
/*  The following array is used to "map" OSD ID's to indicator ID's.     */
/*  Note that special actions must be done for the volume OSD, since     */
/*  this OSD is made up of two indicators                                */
/*=======================================================================*/
const Byte indicator_ids[] =   /*                                 */
{  /* Index in array  auim_OsdIndicator    /<-  =  ->/            */

     AUIM_INDEX_BATTERY_INDICATOR     /* FIND_ID_BATTERY          */
    ,AUIM_INDEX_BLE_INDICATOR         /* FIND_ID_BLE              */
    ,AUIM_INDEX_SYNC_INDICATOR        /* FIND_ID_SYNC             */
    ,AUIM_INDEX_SPLASH_SCREEN         /* FIND_ID_SPLASH_SCREEN    */

    ,AUIM_INDEX_CHARGE_BATT           /* FIND_ID_CHARGE_BATT      */
    ,AUIM_INDEX_PERFORMANCE_INDICATOR /* FIND_ID_PERFORMANCE      */
    ,AUIM_INDEX_PATIENT_INDICATOR     /* FIND_ID_PATIENT          */
    ,AUIM_INDEX_REFERENCE_INDICATOR   /* FIND_ID_REFERENCE        */
    ,AUIM_INDEX_ONE_INDENT_REQUIRED   /* FIND_ID_ONE_INDENT_REQUIRED */
    ,AUIM_INDEX_TIP_ID_VALID          /* FIND_ID_TIP_ID_VALID     */
    ,AUIM_INDEX_TIP_ID_INVALID        /* FIND_ID_TIP_ID_INVALID   */
    ,AUIM_INDEX_TIP_ID_USED           /* FIND_ID_TIP_ID_USED      */
    ,AUIM_INDEX_UNSTABLE              /* FIND_ID_UNSTABLE         */
    ,AUIM_INDEX_TERMINATE_INDICATOR   /* FIND_ID_TERMINATE        */
    ,AUIM_INDEX_ERROR_INDICATOR       /* FIND_ID_ERROR            */
    ,AUIM_INDEX_PERF_CHK_RECOMMENDED  /* FIND_ID_PERF_CHK_RECOMMENDED */

//   ,NR_OF_DOUBLE_INDICATORS
};

static Bool RestoreAllIndicators = FALSE;
 

/*=======================================================================*/
/*    L O C A L   F U N C T I O N   P R O T O T Y P E S                  */
/*=======================================================================*/

Bool CreateIndicator(find_id_enum indicator);
Bool find_IsRemoveIndicator( fuimIndicatorStruct *indicator_data_ptr );
void find_IndicatorAction  ( fuimIndicatorStruct *indicator_data_ptr );

void find_ProcessIndicatorAction(cmdKeyNumber action,	fuimFieldStruct *field_data_ptr );
void RemoveEmptyIndicator(find_id_enum indicator);
 
/*=================================================================================
*   Function:    find_Init
*   Description: Initializes the indicator system by clearing all active indicator slots
*                and resetting the restore flag. Called once during system startup.
*
* Arguments:        None
*
* Externals    Flow    Usage
* ------------------------------------------------------------------------------
* active_indicators[]  OUT  All slots set to EMPTY_INDICATOR (0xFF)
* RestoreAllIndicators  OUT  Set to FALSE to prevent indicator restoration
*
* @return void
*
* @note Must be called before any indicator operations. No hardware dependencies.
===================================================================================*/
void find_Init(void)
{
  Byte  i;

  for (i = 0; i < FUIM_MAX_INDICATORS; i++ ) {
    active_indicators[i] = EMPTY_INDICATOR;
  }

  RestoreAllIndicators = FALSE;
 
}

/*=================================================================================
*   Function:    find_TurnOn
*   Description: Activates the indicator system by initializing all UI indicator
*                arrays and preparing for indicator display. Called when UI is enabled.
*
* Arguments:        None
*
* Externals    Flow    Usage
* ------------------------------------------------------------------------------
* active_indicators[]  OUT  All slots set to EMPTY_INDICATOR (0xFF)
* RestoreAllIndicators  OUT  Set to TRUE to trigger restoration of base indicators
* find_IndicatorFocus  OUT  Set to FIND_DIRECT_INDICATOR_CHARGE_BATT
*
* @return void
*
* @note Calls fuim_InitIndicators() to reset runtime indicator arrays.
===================================================================================*/
void find_TurnOn(void)
{
   Byte  i;
   
   fuim_InitIndicators();
   for (i = 0; i < FUIM_MAX_INDICATORS; i++ ) {
     active_indicators[i] = EMPTY_INDICATOR;
   }
 
   find_IndicatorFocus  = FIND_DIRECT_INDICATOR_CHARGE_BATT;
   RestoreAllIndicators = TRUE;

}

/*=================================================================================
*   Function:    find_TurnOff
*   Description: Deactivates the indicator system by removing all displayed indicators
*                and clearing the restore flag. Called when UI is disabled.
*
* Arguments:        None
*
* Externals    Flow    Usage
* ------------------------------------------------------------------------------
* RestoreAllIndicators  OUT  Set to FALSE
*
* @return void
*
* @note Calls find_RemoveAllIndicators() to clean up all active indicators.
===================================================================================*/
void find_TurnOff(void)
{
   find_RemoveAllIndicators();

   RestoreAllIndicators = FALSE;
}
/*=================================================================================
*   Function:    find_RestoreIndicators
*   Description: Restores the base status indicators (BATTERY, BLE, SYNC) 
*                based on current system state. Called during UI turn-on and
*                periodically in find_Update() when RestoreAllIndicators flag is set.
*
* Arguments:
* Parameter      Flow    Description
* ------------------------------------------------------------------------------
* None           -       -
*
* Externals    Flow    Usage
* ------------------------------------------------------------------------------
* auph_curr_state    IN  Current system state from auph_GetState()
*
* @return void
*
* @note Displays SPLASH_SCREEN during startup state, otherwise shows BATTERY/BLE/SYNC.
===================================================================================*/
void find_RestoreIndicators(void)
{
   auphOsteoState_enum auph_curr_state = auph_GetState();    
  
   if (auph_curr_state == AU_STARTUP_STATE) {
   	 find_DisplayIndicator(FIND_ID_SPLASH_SCREEN);
   } else {
     find_DisplayIndicator(FIND_ID_BATTERY);
     find_DisplayIndicator(FIND_ID_BLE);
     find_DisplayIndicator(FIND_ID_SYNC);
   }
}   

/*=================================================================================
*   Function:    find_SetRestoreAllIndicators
*   Description: Sets the restore flag that triggers restoration of base indicators
*                on the next find_Update() cycle.
*
* Arguments:
* Parameter      Flow    Description
* ------------------------------------------------------------------------------
* NewVal         IN      New boolean value for the restore flag
*
* Externals    Flow    Usage
* ------------------------------------------------------------------------------
* RestoreAllIndicators  OUT  Updated with NewVal value
*
* @return void
*
* @note Used to schedule indicator restoration after menu operations.
===================================================================================*/
void find_SetRestoreAllIndicators(Bool NewVal) {
  RestoreAllIndicators = NewVal;	
}

/*=================================================================================
*   Function:    find_RemoveAllIndicators
*   Description: Removes all currently displayed indicators by iterating through
*                the active_indicators array and calling removal for each occupied slot.
*
* Arguments:        None
*
* Externals    Flow    Usage
* ------------------------------------------------------------------------------
* active_indicators[]  IN  Iteration source - all non-EMPTY_INDICATOR entries removed
*
* @return void
*
* @note Safe to call with empty slots - find_RemoveIndicator checks existence first.
===================================================================================*/
void find_RemoveAllIndicators(void)
{
   Byte  i;
   
   for (i = 0; i < FUIM_MAX_INDICATORS; i++) {
      if (active_indicators[i] != EMPTY_INDICATOR) {
         find_RemoveIndicator( active_indicators[i] );
      }
   }
}

/*=================================================================================
*   Function:    find_IsIndicatorDisplayed
*   Description: Checks whether a specific indicator is currently displayed by
*                searching through the active_indicators array.
*
* Arguments:
* Parameter      Flow    Description
* ------------------------------------------------------------------------------
* indicator      IN      The indicator ID to search for in active_indicators
*
* Externals    Flow    Usage
* ------------------------------------------------------------------------------
* active_indicators[]  IN  Searched for the specified indicator ID
*
* @return TRUE if indicator is found in active_indicators array, FALSE otherwise
*
* @note Linear search through 7-element array. O(n) complexity.
===================================================================================*/
Bool find_IsIndicatorDisplayed(Byte indicator)
{
   Byte  active;
   
   for (active = 0; active < FUIM_MAX_INDICATORS; active++) {
      if (active_indicators[active] == indicator) {
         return TRUE;
      }
   }
   return FALSE;
}

/*=================================================================================
*   Function:    find_DisplayIndicator
*   Description: Displays an indicator on the screen if not already present. Performs
*                mapping from find_id_enum to AUIM_INDEX via indicator_ids array, then
*                creates the indicator through fuim_ConstructIndicator.
*
* Arguments:
* Parameter      Flow    Description
* ------------------------------------------------------------------------------
* indicator      IN      The find_id_enum identifier of the indicator to display
*
* Externals    Flow    Usage
* ------------------------------------------------------------------------------
* active_indicators[]  INOUT  Slot assigned if indicator creation succeeds
* indicator_ids[]      IN     Mapping table for indicator index translation
*
* @return void
*
* @note If indicator already displayed, updates it instead. Maximum 7 indicators
*       can be displayed simultaneously. Modal indicators block menu display.
===================================================================================*/
void find_DisplayIndicator(find_id_enum indicator)
{
  Byte  active;

  if (!find_IsIndicatorDisplayed(indicator)) {
     active = 0;
     while ((active_indicators[active] != EMPTY_INDICATOR) && (active < FUIM_MAX_INDICATORS)) {
        active++;
     }

     if (( active < FUIM_MAX_INDICATORS) && (active_indicators[active] == EMPTY_INDICATOR )) {
       if ( TRUE == CreateIndicator(indicator) ) { 
           active_indicators[active] = indicator;
       } 
     }
  } else {
     find_UpdateIndicator(indicator);
  }

}

/*=======================================================================*/
/*    L O C A L   F U N C T I O N S                                      */
/*=======================================================================*/
 
/*=================================================================================
*   Function:    CreateIndicator
*   Description: Creates a UI indicator by mapping the logical indicator ID
*                to physical OSD indicator structure and calling the UI construction.
*
* Arguments:
* Parameter      Flow    Description
* ------------------------------------------------------------------------------
* indicator      IN      The find_id_enum identifier for the indicator to create
*
* Externals    Flow    Usage
* ------------------------------------------------------------------------------
* indicator_ids[]      IN  Maps find_id_enum to AUIM_INDEX for auim_OsdIndicator
* auim_OsdIndicator[]  IN  Source of indicator geometry and field pointer
*
* @return TRUE if indicator was successfully created (handle != 0), FALSE otherwise
*
* @note Indicator is only created if indicator_ids[indicator] < NR_OF_DOUBLE_INDICATORS.
*       Maximum 7 indicators can be active simultaneously.
===================================================================================*/
Bool CreateIndicator(find_id_enum indicator)
{ 
   osdDialogHandle handle = 0;

   if (indicator_ids[indicator] < NR_OF_DOUBLE_INDICATORS) {
      handle = fuim_ConstructIndicator((fuimIndicatorStruct   *)&auim_OsdIndicator[indicator_ids[indicator]]);
   }
  
   return (Bool)(handle != 0);
}   

/*=================================================================================
*   Function:    find_Update
*   Description: Main update function called in the application loop. Removes expired
*                indicators and restores base indicators if the restore flag is set.
*
* Arguments:        None
*
* Externals    Flow    Usage
* ------------------------------------------------------------------------------
* active_indicators[]  IN  All non-EMPTY_INDICATOR entries are checked for removal
* RestoreAllIndicators  INOUT  Cleared to FALSE after restoration
*
* @return void
*
* @note Called from app_process_action(). Must be called regularly for indicator
*       timeout handling and restoration logic.
===================================================================================*/
void find_Update(void)
{
   Byte  i, indicator;
   
   for (i = 0; i < FUIM_MAX_INDICATORS; i++) {
      indicator = active_indicators[i];
      if (indicator != EMPTY_INDICATOR) {
          RemoveEmptyIndicator(indicator);
      }
   }
  
   if (RestoreAllIndicators == TRUE) {
     find_RestoreIndicators();
     RestoreAllIndicators = FALSE;
   } 

} 

/*=================================================================================
*   Function:    find_RemoveIndicator
*   Description: Removes a specific indicator from the screen by getting its handle,
*                destroying the visual representation, and clearing the slot.
*
* Arguments:
* Parameter      Flow    Description
* ------------------------------------------------------------------------------
* indicator      IN      The find_id_enum identifier of the indicator to remove
*
* Externals    Flow    Usage
* ------------------------------------------------------------------------------
* indicator_ids[]      IN  Maps find_id_enum to AUIM_INDEX
* active_indicators[]  INOUT  Slot cleared to EMPTY_INDICATOR upon removal
*
* @return void
*
* @note Only removes if indicator is currently displayed (checked via find_IsIndicatorDisplayed).
*       Does not modify indicator if not found in active_indicators.
===================================================================================*/
void find_RemoveIndicator(find_id_enum indicator)
{

Byte  active;
osdDialogHandle handle;

  if (find_IsIndicatorDisplayed(indicator)) {

    active = 0;

    while ((active_indicators[active] != indicator) && (active < FUIM_MAX_INDICATORS)) {
      active++;
    }

    if (indicator_ids[indicator] < NR_OF_DOUBLE_INDICATORS) {
      handle = fuim_GetIndicatorHandle((fuimIndicatorStruct   *)&auim_OsdIndicator[indicator_ids[indicator]]);
      fuim_DestroyIndicator(handle);
    }

    active_indicators[active] = EMPTY_INDICATOR;
  }

}   

/*=================================================================================
*   Function:    find_UpdateIndicator
*   Description: Updates an existing indicator's value by fetching current data
*                and triggering the redraw. Called when indicator is already displayed.
*
* Arguments:
* Parameter      Flow    Description
* ------------------------------------------------------------------------------
* indicator      IN      The find_id_enum identifier of the indicator to update
*
* Externals    Flow    Usage
* ------------------------------------------------------------------------------
* indicator_ids[]      IN  Maps find_id_enum to AUIM_INDEX
*
* @return void
*
* @note Only updates if indicator_ids[indicator] < NR_OF_DOUBLE_INDICATORS.
*       Delegates actual rendering to find_IndicatorAction via fuim_UpdateIndicator.
===================================================================================*/
void find_UpdateIndicator(find_id_enum indicator)
{
   fuimIndicatorStruct  *indicator_data_ptr;
 
   if( indicator_ids[indicator] < NR_OF_DOUBLE_INDICATORS ) {
      indicator_data_ptr = (fuimIndicatorStruct  *)&auim_OsdIndicator[indicator_ids[indicator]];
      find_IndicatorAction(indicator_data_ptr);
   }
}  

/*=================================================================================
*   Function:    find_IndicatorAction
*   Description: Processes an action for a displayed indicator by retrieving current
*                command and updating the indicator visual state through the action handler.
*
* Arguments:
* Parameter      Flow    Description
* ------------------------------------------------------------------------------
* indicator_data_ptr IN  Pointer to the indicator structure containing geometry
*
* Externals    Flow    Usage
* ------------------------------------------------------------------------------
* fuim_Indicators[]        IN  Lookup source for indicator handle
* indicator_timer_handle[]   IN  Used by fuim_UpdateIndicator for timer restart
*
* @return void
*
* @note Requires valid indicator handle (1..7). Calls find_ProcessIndicatorAction
*       with current command from aukh_GetCurrentCommand().
===================================================================================*/
void find_IndicatorAction(fuimIndicatorStruct *indicator_data_ptr )
{
  fuimFieldStruct  * field_data_ptr;
  osdDialogHandle   handle;
			
  handle = fuim_GetIndicatorHandle(indicator_data_ptr);

  if ((handle != 0 )&&( handle < FUIM_MAX_INDICATORS)) {  
     field_data_ptr = (fuimFieldStruct  *)indicator_data_ptr->Field;
     find_ProcessIndicatorAction(aukh_GetCurrentCommand(),field_data_ptr);
     fuim_UpdateIndicator (handle, TRUE);
   }
   field_data_ptr = field_data_ptr;
} 

/*=================================================================================
*   Function:    find_IsRemoveIndicator
*   Description: Determines if an indicator should be removed based on validity
*                function check. Returns TRUE if indicator has NOTPRESENT validity.
*
* Arguments:
* Parameter          Flow    Description
* ------------------------------------------------------------------------------
* indicator_data_ptr  IN  Pointer to the indicator structure to check
*
* Externals          Flow    Usage
* ------------------------------------------------------------------------------
* fuim_Indicators[]  IN  Lookup source for indicator handle
*
* @return TRUE if indicator should be removed (NOTPRESENT or invalid handle), FALSE otherwise
*
* @note Handle 0 or handle >= FUIM_MAX_INDICATORS means indicator is not tracked.
*       Used by RemoveEmptyIndicator to determine removal eligibility.
===================================================================================*/
Bool find_IsRemoveIndicator(fuimIndicatorStruct  *  indicator_data_ptr )
{
  osdDialogHandle  handle;

  handle = fuim_GetIndicatorHandle(indicator_data_ptr);

  if ( (handle != 0) && (handle < FUIM_MAX_INDICATORS) ) {
      return FALSE;
  }

  return TRUE;
}

/*=================================================================================
*   Function:    RemoveEmptyIndicator
*   Description: Conditionally removes an indicator based on validity check. Bypasses
*                duplication check and allows removal even if indicator is not displayed.
*
* Arguments:
* Parameter      Flow    Description
* ------------------------------------------------------------------------------
* indicator      IN      The find_id_enum identifier of the indicator to conditionally remove
*
* Externals    Flow    Usage
* ------------------------------------------------------------------------------
* indicator_ids[]      IN  Maps find_id_enum to AUIM_INDEX
* active_indicators[]  INOUT  Slot cleared if removal conditions are met
* auim_OsdIndicator[]  IN  Source for indicator validity check
*
* @return void
*
* @note Does NOT check if indicator is displayed before removal attempt.
*       Calls find_IsRemoveIndicator to determine if removal should proceed.
===================================================================================*/
void RemoveEmptyIndicator(find_id_enum indicator)
{
 fuimIndicatorStruct *indicator_data_ptr;
 Bool PassRemove = TRUE; 
 Byte active = 0;

  if (indicator_ids[ indicator ] < NR_OF_DOUBLE_INDICATORS) {
      indicator_data_ptr = (fuimIndicatorStruct *)&auim_OsdIndicator[ indicator_ids[indicator] ];
             PassRemove &= find_IsRemoveIndicator(indicator_data_ptr);
  }

  if (PassRemove == TRUE) {
    if (find_IsIndicatorDisplayed(indicator)) {

      active = 0;

      while ((active_indicators[active] != indicator) && (active < FUIM_MAX_INDICATORS)) {
        active++;
      }

      active_indicators[active] = EMPTY_INDICATOR;
    }
  }
}    

/*=================================================================================
*   Function:    find_ProcessIndicatorAction
*   Description: Processes key/button actions for an indicator field by iterating
*                through the action navigation table and executing matching handlers.
*
* Arguments:
* Parameter      Flow    Description
* ------------------------------------------------------------------------------
* action         IN      The virtual key command to process
* field_data_ptr IN      Pointer to the field structure containing action mappings
*
* Externals    Flow    Usage
* ------------------------------------------------------------------------------
* aukh_GetCurrentCommand   IN  Source for current action (passed externally)
*
* @return void
*
* @note Modifies action parameter if new_action differs from AU_KEY_PROCESSED.
*       Iterates through ToDoWithKey table until AU_KEY_INVALID sentinel is found.
*       Commented code block (lines 427-441) handles left/right navigation for sliders.
===================================================================================*/
void find_ProcessIndicatorAction(cmdKeyNumber action,      fuimFieldStruct  *field_data_ptr)
{
  fuimDialogNavigation  *ActionPtr;
  Byte   new_action;
 
  if (field_data_ptr != NULL) {

    ActionPtr = field_data_ptr -> ToDoWithKey;

    if( ActionPtr != NULL ) {

       do {
           if ( ActionPtr->Action == action ) {
              new_action = fuim_ActionHandler (ActionPtr->DialogFunction, action);

              if ( new_action != AU_KEY_PROCESSED ) {
                action = new_action;
              } else {
                action = AU_KEY_PROCESSED;
              }
           }

         if ( ActionPtr->Action != AU_KEY_INVALID ) ActionPtr++;

       } while( ActionPtr->Action != AU_KEY_INVALID );
    }
  }
 			
 
  /*=================================================================================
//  switch( action )
//  {
//    case AU_KEY_LEFT:
//    case AU_KEY_RIGHT: {
//       if( field_data_ptr -> Type == FUIM_FIELDTYPE_SLIDER )
//       {
//          fuim_Transformer ( field_data_ptr -> ChangeFunction, (action==AU_KEY_LEFT) ? RGEN_CHANGE_DOWN: RGEN_CHANGE_UP);
//       }
//
//       action = AU_KEY_PROCESSED ;
//     } break;
//  }
  ===================================================================================*/

} 
  
/*=================================================================================
*   Function:    find_GetIndicatorFocus
*   Description: Returns the currently focused direct indicator ID used for
*                navigation and status tracking.
*
* Arguments:        None
*
* Externals    Flow    Usage
* ------------------------------------------------------------------------------
* find_IndicatorFocus  IN  The current focus value to return
*
* @return find_Direct_id_enum value representing the focused indicator
*
* @note Used for internal focus tracking. Currently only FIND_ID_CHARGE_BATT is used.
===================================================================================*/
find_Direct_id_enum find_GetIndicatorFocus(void)
{
  return find_IndicatorFocus;
}

/*=================================================================================
*   Function:    find_SetIndicatorFocus
*   Description: Sets the focus to a specific direct indicator for navigation
*                purposes. Used to track which indicator is currently active.
*
* Arguments:
* Parameter      Flow    Description
* ------------------------------------------------------------------------------
* IndicatorFocus IN      The find_Direct_id_enum value to set as focus
*
* Externals    Flow    Usage
* ------------------------------------------------------------------------------
* find_IndicatorFocus  OUT  Updated with IndicatorFocus value
*
* @return void
*
* @note Used internally during indicator system initialization and navigation.
===================================================================================*/
void find_SetIndicatorFocus( find_Direct_id_enum IndicatorFocus )
{
  find_IndicatorFocus = IndicatorFocus;
}

/*=================================================================================
*   Function:    find_GetIndicatorIdEnum
*   Description: Returns the indicator enum ID based on the current IndicatorFocus
*                setting. Maps the focus position to an actual find_id_enum value.
*
* Arguments:        None
*
* Externals    Flow    Usage
* ------------------------------------------------------------------------------
* find_IndicatorFocus      IN  Index used to access find_DirectIndicators_Ids array
* find_DirectIndicators_Ids[] IN  Source array mapping focus to indicator enum
*
* @return find_id_enum value corresponding to current focus position
*
* @note Returns FIND_ID_CHARGE_BATT for focus = 0 (FIND_DIRECT_INDICATOR_CHARGE_BATT).
*       Currently unused (dead code) as find_DirectIndicators_Ids has only one element.
===================================================================================*/
find_id_enum  find_GetIndicatorIdEnum(void)
{
 return (find_id_enum)find_DirectIndicators_Ids[ find_IndicatorFocus ];
}

/*=================================================================================
*   Function:    find_ToggleStatusIndicator
*   Description: Toggles the charge battery indicator display. Shows the indicator
*                if not currently displayed, and configures its timeout value.
*
* Arguments:        None
*
* Externals    Flow    Usage
* ------------------------------------------------------------------------------
* active_indicators[]      IN  Checked to determine if CHARGE_BATT is displayed
* indicator_ids[]          IN  Maps FIND_ID_CHARGE_BATT to AUIM_INDEX
* auim_OsdIndicator[]    IN  Source for indicator geometry
*
* @return void
*
* @note Uses FUIM_MENU_TIMEOUT (4 seconds) for indicator autohide.
*       Commented code handles CLOCK and SOURCE indicators (currently disabled).
===================================================================================*/
void find_ToggleStatusIndicator(void)
{
  osdDialogHandle handle = 0;

  if (!find_IsIndicatorDisplayed(FIND_ID_CHARGE_BATT)) {
    find_DisplayIndicator(FIND_ID_CHARGE_BATT);
    //Change the timeout value TEST UP
    handle = fuim_GetIndicatorHandle(&auim_OsdIndicator[indicator_ids[FIND_ID_CHARGE_BATT]]);
    fuim_SetIndicatorTimeOut(handle, FUIM_MENU_TIMEOUT);
  }

}

/*=================================================================================
*   Function:    find_RemoveAllNotificationIndicators
*   Description: Removes all notification-type indicators (CHARGE_BATT, PERFORMANCE,
*                PATIENT, etc.) that are in the notification_indicators array.
*                Called before menu activation to prevent overlap.
*
* Arguments:        None
*
* Externals    Flow    Usage
* ------------------------------------------------------------------------------
* notification_indicators[]  IN  List of 11 indicators to remove
* active_indicators[]        IN  Checked to determine if each should be removed
*
* @return void
*
* @note Called from fuim_ActionHandler before fmnu_Activate() for any menu transition.
*       Ensures notification popups don't interfere with menu display.
===================================================================================*/
void find_RemoveAllNotificationIndicators(void)
{
  Byte  i;

  for (i = 0; i < FUIM_MAX_NOTIFICATION_INDICATORS; i++) {
    if (TRUE == find_IsIndicatorDisplayed(notification_indicators[i])) {
      find_RemoveIndicator(notification_indicators[i]);
    }
  }
}

