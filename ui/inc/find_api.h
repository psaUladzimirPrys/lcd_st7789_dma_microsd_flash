/*
 * find_api.h
 *
 *  Created on: 23 февр. 2026 г.
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
Это порядковое перечисление индексов для массива всех применяемых мною
индикаторов
*****************************************************************************/

typedef enum {
   FIND_ID_BATTERY       /* Battery icon (String)     */
  ,FIND_ID_BLE           /* BLE icon (String)         */
  ,FIND_ID_SYNC          /* Sync icon (String)        */
  ,FIND_ID_SPLASH_SCREEN /* Company logo (String)     */
  ,FIND_ID_CHARGE_BATT   /* Charge the Battery notification (String)     */


 // ,FIND_NUMBER_OF_IDS /* */
  }find_id_enum;

/*=======================================================================*/
/* L O C A L   S Y M B O L   D E C L A R A T I O N S                     */
/*=======================================================================*/

/*=======================================================================*/
/* G L O B A L   F U N C T I O N   D E C L A R A T I O N                 */
/*=======================================================================*/

 extern void find_DisplayIndicator(find_id_enum indicator);
 extern void find_TurnOn(void);
 extern void find_Update(void);

 extern void find_RestoreIndicators(void);
 extern void find_RemoveIndicator(find_id_enum indicator);
 extern void find_RemoveAllIndicators(void);

 extern void find_InitAndFormatCCDisplay(void);

 extern void find_SetIndicatorFocus(Byte IndicatorFocus);
 extern Byte find_GetIndicatorFocus(void);
 extern void find_DirectIndicators(Byte IndicatorFocus);
 extern find_id_enum find_GetIndicatorIdEnum(void);
 extern Bool find_IsIndicatorDisplayed(Byte indicator);

 extern void find_UpdateIndicator(find_id_enum indicator);


extern Byte * find_GetIndicatorMuteString(void);

extern void find_SetRestoreAllIndicators(Bool NewVal);

extern Bool find_GetDunamicColorPPIndicators(void);
extern void find_SetDunamicColorPPIndicators(Bool NewValue);

extern Bool find_GetDisplayTemporaryClock(void);
extern void find_ChangeDisplayTemporaryClock(void);
extern void find_Init(void);

extern void find_ToggleStatusIndicator(void);

#endif /* UI_INC_FIND_API_H_ */
