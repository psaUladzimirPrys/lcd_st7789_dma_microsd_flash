/*==========================================================================*/
/*==========================================================================*/
#ifndef _RBSC_API_H
#define _RBSC_API_H

/*==========================================================================*/
/*        I N C L U D E S                                                   */
/*==========================================================================*/
#include "hglobal.h"

/*==========================================================================*/
/*                                                                          */
/*==========================================================================*/



/*==========================================================================*/
/*        G L O B A L   D A T A   D E C L A R A T I O N S                   */
/*==========================================================================*/


/*==========================================================================*/
/*        G L O B A L   F U N C T I O N   P R O T O T Y P E S               */
/*==========================================================================*/
/*==========================================================================*/
/*
 Type  rbsc_Timer_Value(time,step) |
 Macro calculates the value to which the timer variable
 should be set, according to the desired time and step
        Temporary variable value. <nl>
        Parameters: <nl>
    time: Required time.<nl>
    step: Time step.<nl>
 */
/*==========================================================================*/
#define rbsc_Timer_Value(time,step)  (((time)/(step))+2)

/*===========================================================================
   @func This function updates (decrements) the calculated timer value (*rbsc_timer_value)
          until it expires (RBSC_TIMER_EXPIRED) with one step.
   @noi2c
   @nobit
   @int        RBSC_BASIC_INTERFACE
   @parm       Pointer to calculated timer value
   @values     [possible value between 0x00 - 0xFF]
============================================================================*/
/*==========================================================================*/
extern void rbsc_UpdateTimer(Byte *rbsc_timer_ptr) ;

/*==========================================================================*/

extern void rbsc_Init(void);
extern Bool rbsc_IsMax(Byte X, Byte Y);
extern Bool rbsc_IsMin(Byte X, Byte Y);

extern Byte rbsc_ChangeControl(Byte  rbsc_control, Bool rbsc_direction, Byte rbsc_max, Byte rbsc_min);
extern Byte rbsc_ChangeControlAround(Byte rbsc_control, Bool rbsc_direction, Byte rbsc_max, Byte rbsc_min);
extern Word rgen_ChangeControlWord(Word rgen_control, Bool rgen_direction, Word rgen_max, Word rgen_min);

extern Byte rbsc_GetLocalKey(void);

#endif /* _RBSC_API_H */
