/*
 * fpmt.c
 *
 *  Created on: 3 мар. 2026 г.
 *      Author: priss
 */

/*=======================================================================*/
/*        I N C L U D E S                                                */
/*=======================================================================*/
#include "sl_gpio.h"
#include "em_gpio.h"

#include "global.h"
#include "fpmt_api.h"
#include "device_control.h"
#include "aukh.h"
#include "auph.h"
#include "fuim.h"
#include "find_api.h"
#include "file_storage.h"
#include "disp.h"
#include "event_simulation.h"


/*=======================================================================*/
/* G L O B A L   R E F E R E N C E S                                     */
/*=======================================================================*/


/*=======================================================================*/
/* G L O B A L   D E F I N I T I O N S                                   */
/*=======================================================================*/
#define POWER_UP_FAILED               FALSE
#define POWER_UP_OK                   TRUE

/*=======================================================================*/
/* L O C A L   S Y M B O L   D E C L A R A T I O N S                     */
/*=======================================================================*/

/*=======================================================================*/
/* L O C A L   D A T A   D E F I N I T I O N S                           */
/*=======================================================================*/
static fpmtPowerMode_enum  current_power_state = FPMT_STAND_BY;

/*=======================================================================*/
/* L O C A L   F U N C T I O N   P R O T O T Y P E S                     */
/*=======================================================================*/
static void SetInStandbyState(void);
static Bool SetInPowerState(void);
static void SetInErrorState(void);

/*=======================================================================*/
/* F U N C T I O N S                                                     */
/*=======================================================================*/
void fpmt_Init(void)
{
     //GPIO_PinModeSet(LATCH_PORT, LATCH_PIN, gpioModePushPull, 0);
     //GPIO_PinModeSet(START_BUTTON_PORT, START_BUTTON_PIN, gpioModeInputPull, 1);
   current_power_state = FPMT_STAND_BY;
}

/*=======================================================================*/

/*=======================================================================*/

void fpmt_SetPowerState(fpmtPowerMode_enum requested_power_state)
{
   if ( current_power_state != FPMT_ERROR) {

      if ( requested_power_state == FPMT_POWER_ON) {
         if (POWER_UP_OK != SetInPowerState() ) {
            SetInErrorState();
         }

      } else if (requested_power_state == FPMT_STAND_BY) {
         SetInStandbyState();
      } else {
         SetInErrorState();
      }
   }
}

/*=======================================================================*/

fpmtPowerMode_enum fpmt_GetPowerState(void)
{
   return current_power_state;
}

void fpmt_HandleCommand(void)
{
   Byte command;

   command = aukh_GetCurrentCommand();

   if ( command != AU_KEY_STANDBY ) {
       fpmt_SetPowerState(FPMT_POWER_ON);
   }

}

static void SetInStandbyState(void)
{

  fuim_TurnOff();
  fslog_TurnOff();
  disp_TurnOff();

  sim_TurnOff();//Sumulator Debug

  auph_SetState(AU_STANDBY_STATE);

  current_power_state = FPMT_STAND_BY;
  //GPIO_PinOutClear(LATCH_PORT, LATCH_PIN);
}

static Bool SetInPowerState(void)
{
  //GPIO_PinOutSet(LATCH_PORT, LATCH_PIN);
  sim_TurnOn(); //Sumulator Debug

  fuim_Init();

  fuim_TurnOn();
  find_TurnOn();
  disp_TurnOn();
  fslog_TurnOn();

  auph_SetState(AU_STARTUP_SPLASH_STATE);

  current_power_state = FPMT_POWER_ON;

  return POWER_UP_OK;
}

static void SetInErrorState(void)
{

   auph_SetState(AU_ERROR_STATE);

   current_power_state = FPMT_ERROR;
}


void fpmt_Update(void)
{
   if (current_power_state != FPMT_POWER_ON)
   {
      /* Power state is Standby or Error.
       * All devices are shut down and the power-supply is put into
       * stand-by mode. It is now safe to put the MCU in idle mode.
       * As soon as either the Remote Control or Local Keyboard is
       * activated, the MCU will resume processing from this point.
      */
      // drv_SetStandbyEnable();
      // adc_SetCompMode();
      // drv_SetIdleModeEnable(); /* Put microcontroller in idle mode */

   }
}
