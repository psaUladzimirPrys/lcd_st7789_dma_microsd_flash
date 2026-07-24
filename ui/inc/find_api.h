/*
 * find_api.h
 *
 *  Created on: 23 Feb. 2026.
 *      Author: priss
 */

#ifndef UI_INC_FIND_API_H_
#define UI_INC_FIND_API_H_

/*=======================================================================*/
/*        I N C L U D E S                                                */
/*=======================================================================*/



/*=======================================================================*/
/*        G L O B A L   D A T A   D E C L A R A T I O N S                */
/*=======================================================================*/

/*=========================================================================*/
/*   G L O B A L   D E F I N I T I O N S                                   */
/*=========================================================================*/

#define FIND_CFG_DISPLAY_PROG_NUMBER 0x2


/****************************************************************************
This is a list of the enumerates for the array of all display indicators

*****************************************************************************/

typedef enum {
   FIND_ID_BATTERY               /* Battery icon (String)                        */
  ,FIND_ID_BLE                   /* BLE     icon (String)                        */
  ,FIND_ID_SYNC                  /* Sync    icon (String)                        */
  ,FIND_ID_SPLASH_SCREEN         /* Company logo (String)                        */

  ,FIND_ID_CHARGE_BATT           /* CHARGE THE BATTERY notification (String)     */
  ,FIND_ID_PERFORMANCE           /* PERFORMANCE notification (String)            */
  ,FIND_ID_PATIENT               /* PATIENT notification (String)                */
  ,FIND_ID_REFERENCE             /* REFERENCE notification (String)              */
  ,FIND_ID_ONE_INDENT_REQUIRED   /* +1 INDENT REQUIRED notification (String)     */
  ,FIND_ID_TIP_ID_VALID          /* TIP ID VALID notification (String)           */
  ,FIND_ID_TIP_ID_INVALID        /* TIP ID INVALID notification (String)         */
  ,FIND_ID_TIP_ID_USED           /* TIP ID USED notification (String)            */
  ,FIND_ID_UNSTABLE              /* UNSTABLE notification (String)               */
  ,FIND_ID_TERMINATE             /* TERMINATE notification (String)              */
  ,FIND_ID_ERROR                 /* ERROR notification (String)                  */
  ,FIND_ID_PERF_CHK_RECOMMENDED  /* PERF CHK RECOMMENDED notification (String)   */

 // ,FIND_NUMBER_OF_IDS /* */
  }find_id_enum;


typedef enum
{
  FIND_DIRECT_INDICATOR_CHARGE_BATT = 0  /* Charge the Battery notification (String)     */
 ,FIND_DIRECT_MAX_INDICATOR

}find_Direct_id_enum;


/*=======================================================================*/
/* L O C A L   S Y M B O L   D E C L A R A T I O N S                     */
/*=======================================================================*/

/*=======================================================================*/
/* G L O B A L   F U N C T I O N   D E C L A R A T I O N                 */
/*=======================================================================*/


void find_Init(void);
void find_TurnOn(void);
void find_TurnOff(void);
void find_Update(void);

void find_DisplayIndicator(find_id_enum indicator);
void find_UpdateIndicator (find_id_enum indicator);
void find_RemoveIndicator (find_id_enum indicator);
void find_RestoreIndicators  (void);
void find_RemoveAllIndicators(void);
void find_SetRestoreAllIndicators(Bool NewVal);

void find_SetIndicatorFocus(find_Direct_id_enum IndicatorFocus);
find_Direct_id_enum find_GetIndicatorFocus(void);
void find_DirectIndicators(Byte IndicatorFocus);

find_id_enum find_GetIndicatorIdEnum(void);
Bool find_IsIndicatorDisplayed(Byte indicator);

void find_ToggleStatusIndicator(void);

void find_RemoveAllNotificationIndicators(void);

#endif /* UI_INC_FIND_API_H_ */
