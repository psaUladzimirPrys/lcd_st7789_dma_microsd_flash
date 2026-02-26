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

typedef enum
    {
  FIND_DIRECT_INDICATOR_MENU

 ,FIND_DIRECT_INDICATOR_BATTERY /* Battery (String)     */
 ,FIND_DIRECT_INDICATOR_BLE /* BLE (String)     */
 ,FIND_DIRECT_INDICATOR_SYNC /* Sync (String)     */

 ,FIND_DIRECT_INDICATOR_VOLUME
 ,FIND_DIRECT_INDICATOR_BRIGHTNESS
 ,FIND_DIRECT_INDICATOR_SATURATION
 ,FIND_DIRECT_INDICATOR_CONTRAST
 ,FIND_DIRECT_MAX_INDICATOR
  }find_Direct_id_enum;

/****************************************************************************
Это порядковое перечисление индексов для массива всех применяемых мною
индикаторов
*****************************************************************************/

typedef enum {
   FIND_ID_BATTERY /* Battery (String)     */
  ,FIND_ID_BLE /* BLE (String)     */
  ,FIND_ID_SYNC /* Sync (String)     */
  ,FIND_ID_MUTE                    /* Mute                */

 ,FIND_ID_VOLUME                  /* Volume (String)     */
 ,FIND_ID_BRIGHTNESS              /* Brightness (String) */
 ,FIND_ID_SATURATION              /* Saturation (String) */
 ,FIND_ID_CONTRAST              /* Contrast (String) */
 ,FIND_ID_MENU            /* Показывает надпись Меню с мерцанием */
 ,FIND_ID_STATUS                  /* Status              */
 ,FIND_ID_SOURCE                  /* Selected program    */
 ,FIND_ID_NO_SIGNALS              /* Нет сигнала         */
 ,FIND_ID_NO_ALL_PROGRAM              /*  Не настроенны программы-ма         */
 ,FIND_ID_NO_PROGRAM              /*  Не настроенны программа         */
 ,FIND_ID_IMAGE_PRESET            /* image preset        */
 ,FIND_ID_CLOCK                   /* Time                */
 ,FIND_ID_SLEEP                   /* Sleep timer         */
 ,FIND_ID_SLEEP_EXP               /* Sleep timer         */


 // ,FIND_NUMBER_OF_IDS /* оследняя строчка указывает на размер массива под индикаторы */
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
