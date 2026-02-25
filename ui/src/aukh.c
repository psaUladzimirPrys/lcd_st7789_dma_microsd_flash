/* AUKH module handles key input and converts it into commands for the remote control
  and local keys. The module processes key presses and converts them into commands.

  The module provides the following functions for handling key input:
  - void aukh_Init(void)
  - Bool aukh_ReadCommand(void)
  - Bool aukh_LocalKeyPressed(void)
  - Bool aukh_FirstKeyPress(void)
  - Bool aukh_RepeatEvery(Byte repeat_time)
  - Bool aukh_KeyHold(Byte hold_time)

  The function aukh_ProcessKey() is 'called' by various modules to process the key (action)
  depending on the current mode:
  - audi:   direct mode
  - aumn:   menu mode
  - atxt:   teletext mode
  - asrv:   servicemode
  - afac:   factory mode
*/
/*=======================================================================*/
/*        I N C L U D E S                                                */
/*=======================================================================*/

#include "aukh.h"
#include "auph.h"
#include "rbsc_api.h"
/*=======================================================================*/
/* L O C A L   S Y M B O L   D E C L A R A T I O N S                     */
/*=======================================================================*/

typedef struct
{
   Byte key_system_code; /* F  0  0  s4 s3 s3 s1 s0 : s0..4 = system address */
   Byte key_code;        /* t E/N c5 c4 c3 c2 c1 c0 : t = toggle bit,       */
                         /*                        : c0..6 = command code   */

   /*    t  indicates the toggle bit of the remote control, changing state
    *       with every new key press.
            E/N indicates Enhanced/Normal mode - type of remote control
    *
    *    F  0 : normal key
    *       1 : key press of the system remote control toggle key.
    */
} KEY_DATA_STRUCT;

typedef union
{
   KEY_DATA_STRUCT   rc5_key_data;
   Word              rc5_data_int;
} KEY_UNION;

/* @enum key_origin_enum | Type for determining the origin of the received
                            key press */
typedef enum
{
   KEY_NONE,    /* @emem No key pressed                          */
   KEY_RC5,     /* @emem Key received from remote control */
   KEY_LOCAL,   /* @emem Key received from local keyboard */
   KEY_SIMUL    /* @emem Key is simulated                      */
} key_origin_enum;


/*=======================================================================*/
/* L O C A L   D A T A   D E F I N I T I O N S                           */
/*=======================================================================*/

static Word   key_repetition_count;/* Counter for key repetition
                                       increments while the key is pressed,
                                        resets when a new key is pressed. */

static Word   key_hold_count;   /* Counter for key hold duration
                                       increments while the key is pressed,
                                        resets when a new key is pressed.

                                       This counter can be used to determine how long
                                       a key is held. This counter resets together with
                                       key_repetition_count. This counter is used for
                                       detection by the function aukh_RepeatEvery(..). */

static AU_COMMAND   previous;   /* Previous command. Used to detect
                                       command repetition.
                                       This variable is needed for the function
                                       to detect a new key press. */

     AU_COMMAND   au_current ;/* Current command. Used to detect
                                       command repetition.
                                       This variable is needed for the function
                                       to detect a new key press. */

static Byte   au_simulated_key; /* Variable for simulating a key press */

  Bool au_direction; /* Direction of rotation */
   // For determining - au_direction - variable to determine the direction of rotation
   // of the rotary encoder or direction of the key based on the command number
   // stored in the command global variable

 /* Translation table for local keyboard. */

static Byte const LOCAL_KEYBOARD_TABLE[] =
{
   AU_KEY_INVALID,
   AU_KEY_MENU          /* Key 1 */

};

/*=======================================================================*/
/* L O C A L   F U N C T I O N   P R O T O T Y P E S                     */
/*=======================================================================*/
static void CheckKeyRepetition(Byte system, Byte command) ;

/*=======================================================================*/
/* F U N C T I O N S                                                     */
/*=======================================================================*/
/*************************************************************************
           Initializes internal variables for the AUKH module.
           Sets all internal variables to initial state.

           Module name: aukh
**************************************************************************/
void aukh_Init(void)
{
   au_simulated_key   = AU_KEY_INVALID;
   previous.command   = AU_KEY_INVALID;
   au_current.command = AU_KEY_INVALID;
}

/*************************************************************************
           Returns the command code of the current key.

           Returns
           Command code

           Module name: aukh
**************************************************************************/
Byte aukh_GetCurrentCommand(void)
{
   return au_current.command;
}
/**************************************************************************
             Reads the RC-5 remote control code and local key and stores
             it in the global structure of the current command.
             The function returns the following information about the key:


               - command system  (for processing the remote control)
               - command code    (for processing the remote control)

               - repetition count   (for processing the remote control)


           TRUE if a new key is detected<nl>
           FALSE if no new key is detected

           Module name: aukh

****************************************************************************/
Bool aukh_ReadCommand (void)
{
          KEY_UNION   new_key;
   key_origin_enum    new_key_detected = KEY_NONE;

   if ((new_key.rc5_key_data.key_code = rbsc_GetLocalKey()) != 0)
   {

      /****************************************************************************/
      /*           Press of a local key detected  Mapping local key code                */
      /****************************************************************************/

      new_key_detected = KEY_LOCAL;

      au_current.command = LOCAL_KEYBOARD_TABLE[new_key.rc5_key_data.key_code & AU_LOCAL_KEY_CODE_MASK];
      au_current.system  = AU_ADDRESS_TV_KEY;

    }
    else if (au_simulated_key != AU_KEY_INVALID)
    {
      /******************************************************************************/
      /*                Simulated key press detected                                  */
      /******************************************************************************/

      new_key_detected     = KEY_SIMUL;

      au_current.command   = au_simulated_key;
      au_current.system    = AU_ADDRESS_TV_KEY;

      key_hold_count       = AU_KEY_PRESSED_FIRST_TIME;
      key_repetition_count = AU_KEY_PRESSED_FIRST_TIME;
      au_simulated_key     = AU_KEY_INVALID;

    }

/*============================================================================*/

   // For determining - au_direction - variable to determine the direction of rotation
   // of the rotary encoder or direction of the key based on the command number
   // stored in the command global variable

   au_direction = (Bool)(au_current.command % 2);

   if (new_key_detected == KEY_LOCAL)
   {
  /*===========================================================================*/
  /*    If   a  new  key  is  detected (au_current.command changes),          */
  /*   save new_key.rc5_key_data.key_code for comparison.          */
  /*  If the key is the same - it is a repetition, increase the counter.    */
  /*  If a different key is pressed, reset the counter and                  */
  /*  start counting from the beginning.                      */
  /*===========================================================================*/
      CheckKeyRepetition(au_current.system, new_key.rc5_key_data.key_code);
   }

   return ((Bool)(new_key_detected != KEY_NONE));
}

/**************************************************************************
           Checks if it is the first key press.
           For processing

           TRUE if it is the first key press
           FALSE if it is a repeated key press

           Module name: aukh
****************************************************************************/
Bool aukh_FirstKeyPress(void)
{
   return (key_repetition_count == AU_KEY_PRESSED_FIRST_TIME);
}

/*************************************************************************
           Checks if the key is being pressed continuously
           for a certain time (repeat rate).


           TRUE if the key is pressed for "repeat_time"
           FALSE if the key is not pressed for that time

           The function returns TRUE, resets the counter to "AU_KEY_PRESSED_FIRST_TIME".

           Module name : aukh
****************************************************************************/
Bool aukh_RepeatEvery(Byte repeat_time)
{

   if (key_repetition_count == (Word)repeat_time)
   {
      key_repetition_count = AU_KEY_PRESSED_FIRST_TIME;
      return TRUE;
   }
   else
   {
      return FALSE;
   }
}

/*************************************************************************
           Checks if the key is being held (continuous press)
      for a specific time.

           TRUE if the key is held for "hold_time"
           FALSE if the key is not held for "hold_time"

            Module name: aukh

**************************************************************************/
Bool aukh_KeyHold(Byte hold_time)
{
   return (Bool)(key_hold_count == (Word)hold_time);
}

/**************************************************************************
           Processes the key stored in au_current.

           If the key is valid, the following actions are performed:

           If the key is valid (not invalid):
               - command, corresponding to the key.
               - command, corresponding to the mode.
               - Current command is passed to the processing function.
               - Processing of the command key depending on the mode.
               - Pass to the module that is active in VA & PM.

           If the key is valid - call the processing function of the corresponding mode
           in UIFA, then call the function UIDR.

           If the key is valid - call the processing function of the corresponding mode
           in UIDR, then call the function UIUF.

           If the key is valid - call the processing function of the corresponding mode
           in UIUF, then call the function.

           Return value

           Pre condition  : au_current contains the RC5-command
                           from the remote control.

           Post condition : au_current.command = AU_KEY_INVALID
                            au_current.system  = AU_ADDRESS_TV_KEY

           Module name: aukh

*******************************************************************/
void aukh_ProcessKey(void)
{

   /*--------------------------------------------------*/
   /* Processing the command, depending on the module   */
   /*--------------------------------------------------*/
   auph_ProcessKey();

   au_current.command = AU_KEY_INVALID;
   au_current.system  = AU_ADDRESS_TV_KEY;
}

/**************************************************************************
           Simulates a key press from anywhere in the program<nl>
           For the function to be processed aukh_ReadCommand()

           No return value
           Module name: aukh

***************************************************************************/
void aukh_SetSimulatedKey(Byte simulate_key)
 {
     au_simulated_key = simulate_key;
 }

/*=======================================================================*/
/* L O C A L   F U N C T I O N S                                         */
/*=======================================================================*/
/*************************************************************************
           Checks the key for repetition and increments the counter if
           the key is pressed again. Resets the counter if a different key
           is detected.

           Pre condition  : previous = au_current
                            key_repetition_count = current repetition count

           Module name: aukh
****************************************************************************/
static void CheckKeyRepetition(Byte system, Byte command)
{
   if ((system  == previous.system) &&
       (command == previous.command))
   {
      key_hold_count++;
      key_repetition_count++;

   }
   else
   {
      key_hold_count       = AU_KEY_PRESSED_FIRST_TIME;
      key_repetition_count = AU_KEY_PRESSED_FIRST_TIME;

   }

   previous.system  = system;
   previous.command = command;
}

