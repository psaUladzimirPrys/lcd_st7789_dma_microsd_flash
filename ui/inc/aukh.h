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
#include "hglobal.h"


/*=======================================================================*/
/*    G L O B A L   D A T A   D E C L A R A T I O N S                    */
/*=======================================================================*/

#define AU_DOWN  0  /* Defines the boolean value for the direction    */
#define AU_UP    1  /* of the new key: which can be AU_UP or AU_DOWN. */


typedef enum {

  // --- System Startup & Idling ---
   AU_STARTUP_SPLASH_STATE,        //OsteoProbe Boot screen: logo, hardware self-test animation
   AU_IDLE_STATE,                  //OsteoProbe Home screen: BLE status, battery, , and usage tips

   // --- Connectivity & Data Management ---
   AU_BLE_DISCOVERABLE_STATE,      //OsteoProbe Pairing mode: waiting for OsteoVault app connection
  // AU_WAIT_TIP_ID_STATE,         //OsteoProbe Input required: waiting for Tip ID from OsteoVault
   AU_CONFIGURAION_MENU_STATE,

   // --- Measurement Procedures ---
   AU_PERFORMANCE_CHECK_MEASURE_STATE, //OsteoProbe Performance test: verifying accuracy against limit values
   AU_REFERENCE_CHECK_MEASURE_STATE,   //OsteoProbe Reference check: measuring a standard phantom for calibration
   AU_PATIENT_MEASURE_STATE,           //OsteoProbe Active session: indentation counter, validity, real-time tips

   AU_STANDBY_STATE,     /* @emem OsteoProbe SDtand-by  */
   AU_MENU_STATE,        /* @emem OsteoProbe in Menu      */
   AU_DIRECT_STATE,      /* @emem OsteoProbe in Normal    */

   // --- Results & Notifications ---
   AU_BATTERY_CHARGING_STATE,     //OsteoProbe stop measure: Very low battery
   AU_ERROR_STATE                 //OsteoProbe Alerts: sensor errors, or process violations

}auphOsteoState_enum;


 typedef enum {
     AU_KEY_PRESS_SHORT,
     AU_KEY_PRESS_LONG,
     AU_KEY_PRESS_VERY_LONG,
     AU_KEY_PRESS_MULTI,
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


#define AU_KEY_START                  1
#define AU_KEY_NO                     2
#define AU_KEY_CLOSE                  4
#define AU_KEY_PERF_CHK               8
#define AU_KEY_NEXT                   10

#define AU_KEY_CANCEL                 20
#define AU_KEY_YES                    40
#define AU_KEY_PARAMS                 50


#define AU_KEY_STANDBY                52
#define AU_KEY_MENU                   59

#define AU_KEY_SERVICE                150   /* Simulated key */


#define FUIM_ACTION_PUSH_THROUGH      253 /* previous entered digit is hold */

#define AU_KEY_PROCESSED              254
#define AU_KEY_INVALID                255



#define AU_KEY_PRESENT_MASK      0x80 /* Если MSB код системы RC5     */
                                      /* установлен, клавиша ДУ онаружена */
#define AU_TOGGLE_BIT_MASK       0x80 /* MSB  key command code     */
                                      /* определения повтора клавиши       */
#define AU_SYSTEM_ADDRESS_MASK   0x1F /* Отфильтровка   Адреса системы RC5   */

//#define AU_KEY_CODE_MASK         0x7F /*Отфильтровка бита удержания клавиши от клавиши*/
#define AU_KEY_CODE_MASK         0x3F /*Отфильтровка бита удержания клавиши от клавиши*/

#define AU_LOCAL_KEY_CODE_MASK     0x7 /*Отфильтровка бита удержания клавиши локальной клавиши от клавиши*/

#define AU_ADDRESS_SERVICE_KEY      7

 /* Defining the groups of keys used */

 enum {
   AU_GROUP_IDLE,
   AU_GROUP_STANDBY,
   AU_GROUP_DIRECT,
   AU_GROUP_MENU,
   AU_GROUP_SERVICE,
   NUMBER_OF_AU_GROUPS
 };

#define     AU_GROUP_INVALID           255


/* Структура используется для сохранения команды
 * Эта команда состоит из :
 *     - command - кода команды
 */
typedef struct {
   Byte  command;
} AU_COMMAND;


 /*===========================================================================*/
 /*    G L O B A L   F U N C T I O N P R O T O T Y P E S                      */
 /*===========================================================================*/
void aukh_Init(void);
void aukh_ProcessKey(void);
Bool aukh_ReadCommand (void);
Byte aukh_GetCurrentCommand(void);
Bool aukh_KeyHold(Byte hold_time);
Bool aukh_FirstKeyPress(void);

#endif /* UI_INC_AUKH_H_ */
