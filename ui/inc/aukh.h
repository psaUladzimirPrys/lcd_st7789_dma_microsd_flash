/*
 * aukh.h
 *
 *  Created on: 20 02 2026
 *      Author: priss
 */
#ifndef UI_INC_AUKH_H_
#define UI_INC_AUKH_H_

/*=======================================================================*/
/*        I N C L U D E S                                                */
/*    е              */
/*=======================================================================*/
#include "global.h"


#define aukh_Post_UI_Event aukh_SetSimulatedKey   //aukh_PostButtonEvent

/*=======================================================================*/
/*    G L O B A L   D A T A   D E C L A R A T I O N S                    */
/*=======================================================================*/

#define AU_DOWN  0  /* Defines the boolean value for the direction    */
#define AU_UP    1  /* of the new key: which can be AU_UP or AU_DOWN. */


typedef enum {
   // --- System Power Work Mode  ---
    AU_STANDBY_STATE = 0    // OsteoProbe Stand-by on startup
   // --- System Startup & Idling Work Mode ---
   ,AU_STARTUP_STATE        // OsteoProbe Boot screen: logo, hardware self-test animation
   ,AU_IDLE_STATE           // OsteoProbe Home screen: BLE status, battery, , and usage tips
   // --- Measurement Procedures & Notifications ---
   ,AU_MENU_STATE           // OsteoProbe Measurements screen: RATIENT, PERFORM, CONFIG
   // --- Errors Work Mode ---
   ,AU_ERROR_STATE          // OsteoProbe Alerts: sensor errors, or process violations
}auphOsteoState_enum;


typedef enum {
   AU_KEY_PRESS_INVALID = 0
  ,AU_KEY_PRESS_SHORT
  ,AU_KEY_PRESS_LONG
  ,AU_KEY_PRESS_VERY_LONG
  ,AU_KEY_PRESS_MULTI_3_TIME
  ,AU_KEY_PRESS_MULTI_5_TIME
  ,AU_KEY_PRESS_MULTI_2_TIME

} auphKeyPressType_enum;


 /* Key press timing definitions for key repetition. */

#define AU_KEY_PRESSED_FIRST_TIME        0   /*  first time           */
#define AU_KEY_PRESSED_128_MSEC          1   /*  1 * 128 ms = 128 ms  */
#define AU_KEY_PRESSED_256_MSEC          2   /*  2 * 128 ms = 256 ms  */
#define AU_KEY_PRESSED_HALF_A_SECOND     4   /*  4 * 128 ms = .5 sec  */
#define AU_KEY_PRESSED_ONE_SECOND        8   /*  8 * 128 ms = 1 sec   */
#define AU_KEY_PRESSED_TWO_SECONDS      16   /* 16 * 128 ms = 2 secs  */
#define AU_KEY_PRESSED_THREE_SECONDS    24   /* 24 * 128 ms = 3 secs  */
#define AU_KEY_PRESSED_FOUR_SECONDS     32   /* 32 * 128 ms = 4 secs  */
#define AU_KEY_PRESSED_FIVE_SECONDS     40   /* 40 * 128 ms = 5 secs  */


 /* Definition for all possible keys */
 /* Key numbers > 127 are simulated keys. */
#define AU_VIRTUAL_KEY_1               1        /* AU_KEY_PRESS_SHORT         */
#define AU_VIRTUAL_KEY_2               2        /* AU_KEY_PRESS_LONG          */
#define AU_VIRTUAL_KEY_3               3        /* AU_KEY_PRESS_VERY_LONG     */
#define AU_VIRTUAL_KEY_4               4        /* AU_KEY_PRESS_MULTI_3_TIME  */
#define AU_VIRTUAL_KEY_5               5        /* AU_KEY_PRESS_MULTI_5_TIME  */
#define AU_VIRTUAL_KEY_6               6        /* AU_KEY_PRESS_MULTI_2_TIME  */


#define AU_KEY_START                  11
#define AU_KEY_NO                     12
#define AU_KEY_CLOSE                  13
#define AU_KEY_PERF_CHK               14
#define AU_KEY_NEXT                   15

#define AU_KEY_CANCEL                 20
#define AU_KEY_YES                    40
#define AU_KEY_PARAMS                 50

#define AU_KEY_STANDBY                52
#define AU_KEY_MENU                   59

#define AU_KEY_SERVICE                130   /* Simulated key */
#define AU_KEY_PAIRING                131   /* Simulated key */


/*=====    ERROR working mode         ===============*/
#define AU_ERROR_STATE_SET                     140   /* Simulated key Set Osteoprobe to display Error working mode  */


/*=======       CHARGE_BATTERY working mode   =======*/
#define AU_MENU_CHARGE_BATTERY_ENTER           141 /* Simulated key Set Osteoprobe to display CHARGE BATTERY working mode  */
#define AU_MENU_CHARGE_BATTERY_CANCELED        142 /* Simulated key Set Osteoprobe to CANCELED CHARGE BATTERY working mode */


/*======   PAIRING working mode       ==============*/
#define AU_PAIRING_MENU_OK                     150 /* Simulated key  Pairing OK       UI working mode  */
#define AU_PAIRING_MENU_FAILED                 151 /* Simulated key  Pairing FAILED   UI working mode  */
#define AU_PAIRING_MENU_CANCELED               152 /* Simulated key  Pairing CANCELED UI working mode  */


/*======   PERFORMANCE working mode   =============*/
#define AU_PERFORMANCE_MENU_START              160 /* Simulated key  Performance START     UI working mode  */
#define AU_PERFORMANCE_MENU_FINISHED           161 /* Simulated key  Performance FINISHED  UI working mode  */
#define AU_PERFORMANCE_MENU_CHECK_END          162 /* Simulated key  Performance CHECK End UI working mode  */


/*=====    REFERENCE working mode     ==============*/
#define AU_REFERENCE_MENU_START                170 /* Simulated key  Reference START     UI working mode */
#define AU_REFERENCE_MENU_FINISHED             171 /* Simulated key  Reference FINISHED  UI working mode */
#define AU_REFERENCE_PERFORM_MENU_REPEAT_START 172 /* Simulated key  Reference REPEAT START in Performance MODE UI working mode*/
#define AU_REFERENCE_PATIENT_MENU_REPEAT_START 173 /* Simulated key  Reference REPEAT START in Patient MODE    UI working mode */


/*======   PATIENT working mode       =============*/
#define AU_PATIENT_MENU_START                  180 /* Simulated key  Patient START      UI working mode */
#define AU_PATIENT_MENU_FINISHED               181 /* Simulated key  Patient FINISHED   UI working mode */
#define AU_PATIENT_MENU_CHECK_END              182 /* Simulated key  Patient CHECK End  UI working mode */
#define AU_PATIENT_MENU_FAILED                 183 /* Simulated key  Patient FAILED     UI working mode */
#define AU_PATIENT_MENU_VALIDATING             184 /* Simulated key  Patient VALIDATING UI working mode */
#define AU_PATIENT_MENU_VALIDATING_RESULT      185 /* Simulated key  Patient VALIDATING with display TIPID indicator FAILED/OK/USED UI working mode */


/*======   IDLE working mode          =============*/
#define AU_IDLE_MENU_START                     190 /* Simulated key to Display IDLE menu UI working mode     */



#define FUIM_ACTION_PUSH_THROUGH          253 /* previous entered digit is hold */
#define AU_KEY_PROCESSED                  254
#define AU_KEY_INVALID                    255


 /* Defining the groups of keys used */

 enum {
   AU_GROUP_IDLE,
   AU_GROUP_STANDBY,
   AU_GROUP_DIRECT,
   AU_GROUP_MENU,
   AU_GROUP_ERROR,
   NUMBER_OF_AU_GROUPS
 };

#define     AU_GROUP_INVALID           255


typedef struct {
   Byte  command;
} AU_COMMAND;


 /*===========================================================================*/
 /*    G L O B A L   F U N C T I O N     P R O T O T Y P E S                  */
 /*===========================================================================*/
void aukh_Init(void);
void aukh_TurnOff(void);
void aukh_TurnOn(void);
void aukh_ProcessKey(void);
Bool aukh_ReadCommand (void);
Byte aukh_GetCurrentCommand(void);
Bool aukh_KeyHold(Byte hold_time);
Bool aukh_RepeatEvery(Byte repeat_time);
Bool aukh_FirstKeyPress(void);
void aukh_PostButtonEvent(auphKeyPressType_enum type);
void aukh_SetSimulatedKey(Byte simulate_key);

#endif /* UI_INC_AUKH_H_ */
