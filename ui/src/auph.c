/*
 * auph.c
 *
 *  Created on: 20 февр. 2026 г.
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
#define X_NOT_RESET_DIGIT_ENTRY  0x40
#define X_IN_SERVICE             0x80


#define PERMISSION_DIGITS             X_REPEAT + X_IN_STANDBY + X_IN_TEXT + X_IN_MENU + X_NOT_RESET_DIGIT_ENTRY
#define PERMISSION_STANDBY            X_IN_STANDBY + X_IN_SERVICE  
#define PERMISSION_MUTE               X_ALWAYS
#define PERMISSION_PP                 X_REPEAT + X_NOT_IN_TEXT + X_NOT_IN_MENU
#define PERMISSION_DIRECT             X_NOT_IN_TEXT + X_NOT_IN_MENU
#define PERMISSION_CONTROL_LEFT_RIGHT X_REPEAT + X_IN_MENU  
#define PERMISSION_CONTROL_UP_DOWN    X_REPEAT + X_IN_STANDBY + X_IN_TEXT + X_IN_MENU  
#define PERMISSION_MENU               X_REPEAT + X_NOT_IN_TEXT + X_IN_MENU  
#define PERMISSION_DIRECT_MENU        X_IN_TEXT
#define PERMISSION_AV                 X_NOT_IN_TEXT + X_NOT_IN_MENU + X_IN_STANDBY
#define PERMISSION_TV                 X_IN_TEXT + X_NOT_IN_MENU + X_IN_MENU + X_IN_STANDBY
#define PERMISSION_PIP                X_REPEAT + X_NOT_IN_TEXT + X_NOT_IN_MENU
#define PERMISSION_TEXT               X_IN_TEXT + X_IN_MENU
#define PERMISSION_TEXT_SPECIAL       X_IN_TEXT + X_NOT_IN_MENU
#define PERMISSION_SERVICE            X_IN_STANDBY + X_NOT_IN_MENU
#define PERMISSION_PROTECTION         X_ALWAYS
#define PERMISSION_ABB                X_NOT_IN_MENU + X_NOT_IN_TEXT

#define LENGTH_KEY_GROUPCODE_TABLE  ( sizeof(key_groupcode_table) / \
                                      sizeof(auphKeyGroup) )
/*==========================================================================*/
/*     L O C A L   F U N C T I O N S   P R O T O T Y P E S                  */
/*==========================================================================*/
static void HandleIdleKey(void);
static void HandleStandby(void);
static void HandleMenu(void);
static void HandleDirectKey(Byte key_group) ;
/*MPF=======================================================================*/
/******************************************************************************
    @struct auphKeyGroup | В структуре первое поле , содержит код клавиши.
            Второе поле содержит код группы, которой клавиша  принадлежит.
******************************************************************************/
typedef struct
{
   Byte key;   /* @field action поле действие (номер - клавиши) */
   Byte group; /* @field function поле функции Быть выполнен, когда клавиша нажата.
                              Это - индекс в 'Observer' таблице функции */
} auphKeyGroup;
/*EMP=======================================================================*/

/* Таблица ROM, содержащая группу кода для каждого кода клавиши. */
static auphKeyGroup const key_groupcode_table[] = {
    { AU_KEY_STANDBY, AU_GROUP_STANDBY}
   ,{ AU_KEY_CANCEL,  AU_GROUP_DIRECT}
   ,{ AU_KEY_MENU,    AU_GROUP_MENU}
};


/* Таблица ROM, содержащая разрешения для каждого кода группы клавиш.

*/
static Byte const permission_table[] =
{
   PERMISSION_STANDBY,            /* AU_GROUP_STANDBY            */
   PERMISSION_MUTE,               /* AU_GROUP_MUTE               */
   PERMISSION_DIRECT,             /* AU_GROUP_DIRECT             */
   PERMISSION_MENU,               /* AU_GROUP_MENU               */
   PERMISSION_DIRECT_MENU,        /* AU_GROUP_DIRECT_MENU        */
   PERMISSION_SERVICE,            /* AU_GROUP_SERVICE            */

};


/* Таблица ROM, содержащая указатели функции для каждой клавиши group code. */

static const VOID_FUNCTION_PTR direct_function_table[] =
{
   HandleStandby,                /* AU_GROUP_STANDBY              */
   HandleMenu,                   /* AU_GROUP_MENU                 */
//   HandleDirectMenu,             /* AU_GROUP_DIRECT_MENU          */
   HandleIdleKey,                /* AU_GROUP_TEXT                 */
//   HandleService,                /* AU_GROUP_SERVICE              */

};

/*==========================================================================*/
/*     L O C A L   D A T A   D E F I N I T I O N S                          */
/*==========================================================================*/

static auphTvState_enum  auph_state;
//static Bool auph_key_repetition_allowed;


/*=======================================================================*/
/*************************************************************************
   @func   Возвращает теущее состояние  TV

   @rdesc  Текущее tv-state<nl><nl>
           Одно из:<nl>
   AU_STANDBY_STATE,        @emem TV приемник в SDtand-by состоянии
   AU_TELETEXT_STATE,       @emem TV приемник в Teletext состоянии
   AU_SERVICE_STATE,        @emem TV приемник в Service состоянии
   AU_MENU_STATE,           @emem TV приемник в Menu состоянии
   AU_DIRECT_STATE,         @emem TV приемник в Normal состоянии
   AU_FACTORY_STATE,        @emem TV приемник в Factory состоянии
   AU_GAME_STATE,           @emem TV приемник в Game состоянии
   AU_ABB_STATE,            @emem TV приемник в АВВ регулировке
   AU_ERROR_STATE           @emem TV приемник в Error состоянии

   @comm   Функция принадлежит компоненту: auph
****************************************************************************/

auphTvState_enum auph_GetState(void)
{
   return auph_state;
}



void auph_SetState(auphTvState_enum new_state)
{
   auph_state = new_state;

}


/******************************************************************************
          Функция обрабатывает команды standby command
          Ничего не возвращает
          Принадлежит компоненту: auph
******************************************************************************/
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

/******************************************************************************
          Пустая функция без действия на будущие применения

          Ничего не возвращает

          Принадлежит компоненту: auph

******************************************************************************/
static void HandleIdleKey(void)
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


 if (aukh_KeyHold(AU_KEY_PRESSED_ONE_SECOND))
 {

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
           Оработка пользовательских клавиш , когда не в:<nl>
              - standby mode<nl>
              - menu mode<nl>
              - text mode<nl><nl>
             так , Все функции прямого доступа обработаны.

           Ничего не возвращает
           Принадлежит компоненту: auph
***************************************************************************/

 static void HandleDirectKey(Byte key_group)
{
   direct_function_table[key_group]();
}


 /******************************************************************************
            Обрабатывает клавишу.

            Каждая клавиша принадлежит определенной группе. Для каждой группы разрешения
  доступны, чтобы видеть, имеет ли команда силу в некоторых состояниях ТВ.

            Мы грубо отличаем 4 различных состояния, кто весь взаимны исключительны:
               - Standby.
               - Normal operation.
               - Normal operation, menu on.
               - Normal operation, teletext on.

            При обработке клавиши, разрешение клавиши  и состояния ТВ проверено, чтобы видеть,
       если и где клавиша  будет обработана:
            - Если клавише установили разрешение X_IN_STANDBY, и ТВ находится в StandBy,
       клавиша  будет обработана в пакете fpmt.
            - Если клавише установили разрешение X_IN_SERVICE, и ТВ находится в Service Mode,
       клавиша  будет обработана в пакете fsrv.
            - Если клавише установили разрешение X_IN_MENU, и меню включено,
       клавиша  будет обработана в пакете fmnu.
            - Если клавише  установили разрешение X_IN_TEXT, и телетекст включен,
       клавиша  будет обработана в пакете ftxt.
            - Во всех других случаях  клавиша  обработана как прямая клавиша .

             Ничего не возвращает

            Принадлежит компоненту: auph

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

       if ((permission & X_REPEAT) || aukh_FirstKeyPress())/*Проверяет введенная клавиша первый раз нажата*/
       {

/*           if (!(permission & X_NOT_RESET_DIGIT_ENTRY)) //Здесь не понимаю
           {
           fchg_CancelProgramDigitEntry();
           }*/
           /******************************************************************************
              AU_STANDBY_STATE,              TV приемник в SDtand-by состоянии
              AU_TELETEXT_STATE,             TV приемник в Teletext состоянии
              AU_SERVICE_STATE,              TV приемник в Service состоянии
              AU_MENU_STATE,                 TV приемник в Menu состоянии
              AU_DIRECT_STATE,               TV приемник в Normal состоянии
              AU_FACTORY_STATE,              TV приемник в Factory состоянии
              AU_GAME_STATE,                 TV приемник в Game состоянии
              AU_ABB_STATE,                  TV приемник в АВВ регулировке
             AU_ERROR_STATE                 TV приемник в Error состоянии

           ******************************************************************************/
           /* Если кнопка первый раз нажата или разрешен ее повтор */

            switch ( auph_GetState() )
            {//Начвльная скобка от switch(auph_GetState)

                case AU_MENU_STATE:{
                 if (permission & X_IN_MENU)
                 {
                /*  Menu is active, and the key is allowed in menu   */
                   fmnu_HandleCommand();

                  } else
                  {
                     if (!(permission & X_NOT_IN_MENU))
                     { /*Не клавиши меню но которые допскаются в меню*/
                      /* Not a menu key but allowed in menu       */
                      HandleDirectKey(key_groupcode_table[index].group);
                     }
                  }

                }break;

                 case AU_DIRECT_STATE:{

                   HandleDirectKey(key_groupcode_table[index].group);

                 }break;

                  default:{
                            ; /* Нет действия, потому что  клавиша НЕ разрешена в текущем состоянии  */
                           }

             }//Концевая скобка от switch(auph_GetState)

         }      /* End: repitition allowed or first key and normal key handling */
    }         /* End: Допустимая клавиша введена  */

    au_current.command = AU_KEY_PROCESSED;
 }

