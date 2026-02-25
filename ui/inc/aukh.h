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



#define AU_DOWN  0  /* Определяет boolean значение для направления    */
#define AU_UP    1  /* новой клавиши: которая может AU_UP или AU_DOWN.     */





/* auphTvState_enum | Этот перечислаямый тип показывает TV состояния  */

typedef enum
 {

   AU_STANDBY_STATE,     /* @emem TV приемник в SDtand-by состоянии */
   AU_TELETEXT_STATE,    /* @emem TV приемник в Teletext состоянии  */
   AU_SERVICE_STATE,     /* @emem TV приемник в Service состоянии   */
   AU_MENU_STATE,        /* @emem TV приемник в Menu состоянии      */
   AU_DIRECT_STATE,      /* @emem TV приемник в Normal состоянии    */
   AU_FACTORY_STATE,     /* @emem TV приемник в Factory состоянии   */
   AU_GAME_STATE,        /* @emem TV приемник в Game состоянии      */
   AU_ABB_STATE,         /* @emem TV приемник в ABB реглировки      */
   AU_ERROR_STATE        /* @emem TV приемник в Error состоянии     */
 }auphTvState_enum;


 /* Определения времени нажатия клавиши  для повтора клавиш. */

 #define AU_KEY_PRESSED_FIRST_TIME        0   /*  first time           */
 #define AU_KEY_PRESSED_128_MSEC          1   /*  1 * 128 ms = 128 ms  */
 #define AU_KEY_PRESSED_256_MSEC          2   /*  2 * 128 ms = 256 ms  */
 #define AU_KEY_PRESSED_HALF_A_SECOND     4   /*  4 * 128 ms = .5 sec  */
 #define AU_KEY_PRESSED_ONE_SECOND        8   /*  8 * 128 ms = 1 sec   */
 #define AU_KEY_PRESSED_TWO_SECONDS      16   /* 16 * 128 ms = 2 secs  */
 #define AU_KEY_PRESSED_THREE_SECONDS    24   /* 24 * 128 ms = 3 secs  */
 #define AU_KEY_PRESSED_FOUR_SECONDS     32   /* 32 * 128 ms = 4 secs  */
 #define AU_KEY_PRESSED_FIVE_SECONDS     40   /* 40 * 128 ms = 5 secs  */

 /* Определение для всех возможных клавиш      */
 /* Номер клавиши > 127 являются симулируемыми клавишами. */

#define AU_KEY_STANDBY                 12
#define AU_KEY_CANCEL                  45
#define AU_KEY_MENU                    59

#define AU_KEY_SERVICE                 150   /* Simulated key */
#define AU_KEY_PROTECTION              151   /* Simulated key */

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

#define AU_ADDRESS_TV_KEY           0
#define AU_ADDRESS_SERVICE_KEY      7


/* Определение груп используемых клавиш */

enum
{
  AU_GROUP_DIGITS,
  AU_GROUP_STANDBY,
  AU_GROUP_MUTE,
  AU_GROUP_PP,
  AU_GROUP_DIRECT,
  AU_GROUP_CONTROL_LEFT_RIGHT,
  AU_GROUP_CONTROL_UP_DOWN,
  AU_GROUP_MENU,
  AU_GROUP_DIRECT_MENU,
  AU_GROUP_AV,
  AU_GROUP_TV,
  AU_GROUP_PIP,
  AU_GROUP_TEXT,
  AU_GROUP_TEXT_SPECIAL,
  AU_GROUP_SERVICE,
  AU_GROUP_PROTECTION,
  AU_GROUP_ABB,
  NUMBER_OF_AU_GROUPS
};

#define     AU_GROUP_INVALID           255


/* Структура используется для сохранения команды ДУ.
 * Эта команда состоит из :
 *     - system -кода системы
 *     - command - кода команды
 */
typedef struct
{
   Byte  system;
   Byte  command;
} AU_COMMAND;


 /*===========================================================================*/
 /*    G L O B A L   F U N C T I O N P R O T O T Y P E S                      */
 /*===========================================================================*/

void aukh_ProcessKey(void);
Byte aukh_GetCurrentCommand(void);
Bool aukh_KeyHold(Byte hold_time);
Bool aukh_FirstKeyPress(void);

#endif /* UI_INC_AUKH_H_ */
