/*==========================================================================*/
/*                                                                          */
/*        I N C L U D E S                                                   */
/*==========================================================================*/

#include "rbsc_api.h"

/*=======================================================================*/
/* L O C A L   D A T A   D E F I N I T I O N S                           */
/*=======================================================================*/
 static Byte local_key_buffer;
/*=======================================================================*/
/*        L O C A L   FUNCTION  D E C L A R A T I O N S                  */
/*=======================================================================*/
 
/*==========================================================================*/
/* ***************************************************************************
   @func       This function updates (decrements) the Calculated timer value
               (*rbsc_timer_value) until it expires (RBSC_TIMER_EXPIRED)
               with one step.
   @noi2c
   @nobit
   @int        RBSC_BASIC_INTERFACE
   @parm       Pointer to calculated timer value
   @values     [possible value between 0x00 - 0xFF]
*****************************************************************************/
 
void rbsc_UpdateTimer(Byte *rbsc_timer_ptr)
{  
  
 Byte Temp = 0;
 Temp = *rbsc_timer_ptr; 
      if(Temp > 1)
             Temp = Temp - 1;
 *rbsc_timer_ptr = Temp;
 
}

 
/* *************************************************************************/
/* ***************************************************************************
   @func       Checks if the first value is less than the second value.
               Used to determine if the maximum limit is not yet reached.
   @noi2c
   @nobit
   @int        RBSC_BASIC_INTERFACE
   @parm       First value (X)
   @parm       Second value (Y)
   @rdesc      TRUE if X < Y, otherwise FALSE.
**************************************************************************** */
Bool rbsc_IsMax(Byte X, Byte Y)
{
return (X<Y) ? TRUE  :  FALSE; 
}

/* *************************************************************************/
/* ***************************************************************************
   @func       Checks if the first value is greater than the second value.
               Used to determine if the minimum limit is not yet reached.
   @noi2c
   @nobit
   @int        RBSC_BASIC_INTERFACE
   @parm       First value (X)
   @parm       Second value (Y)
   @rdesc      TRUE if X > Y, otherwise FALSE.
*****************************************************************************/
Bool rbsc_IsMin(Byte X, Byte Y)
{
return (X>Y) ? TRUE  :  FALSE; 
}

/* *************************************************************************
 This function increments or decrements the value of
 Byte rbsc_control (depending on the rbsc_direction bit)
 if it is less than rbsc_max
       and greater than rbsc_min
************************************************************************** */
Byte rbsc_ChangeControl(Byte  rbsc_control, 
                        Bool	rbsc_direction,
                        Byte  rbsc_max, 
                        Byte  rbsc_min)
 {
    
  if( rbsc_direction)
    { /* If rbsc_control > rbsc_min = subtract */
      if(rbsc_IsMin(rbsc_control,rbsc_min))
       {
       return  rbsc_control-=1;     
        }
        
     }
     else
     { /* If rbsc_control < rbsc_max = add */	
       if(rbsc_IsMax(rbsc_control,rbsc_max))
          {
          return rbsc_control+=1;     
           }
       
     }

return rbsc_control;
}

/*========================================================================
 This function increments or decrements the value of rbsc_control
 (depending on the rbsc_direction bit) within the range [rbsc_min, rbsc_max].
 If the limit is reached, the value wraps around to the opposite limit.
 ========================================================================*/
/*========================================================================
   @func       Changes the control value within a range with wrap-around.
               If incrementing past max, it wraps to min.
               If decrementing past min, it wraps to max.
   @noi2c
   @nobit
   @int        RBSC_BASIC_INTERFACE
   @parm       Current control value
   @parm       Direction bit (FALSE: increment, TRUE: decrement)
   @parm       Maximum limit
   @parm       Minimum limit
   @rdesc      Updated control value.
  ========================================================================*/
Byte rbsc_ChangeControlAround(Byte  rbsc_control, 
                              Bool  rbsc_direction,
                              Byte  rbsc_max, 
                              Byte  rbsc_min)
 {

 if( rbsc_direction)
    { /* If rbsc_control > rbsc_min = subtract */
       if(rbsc_IsMin(rbsc_control,rbsc_min))
        {
        return  rbsc_control-=1;     
        }
       return rbsc_control = rbsc_max;  
     }
     else
     { /* If rbsc_control < rbsc_max = add */	
       if(rbsc_IsMax(rbsc_control,rbsc_max))
          {
          return rbsc_control+=1;     
           }
       return rbsc_control = rbsc_min;  
     }

}
 
/*=======================================================================*/
/*  				                                                             */
/*=======================================================================*/
/*
   @func       Increments or decrements a Word-sized control value
               within a specified range [rgen_min, rgen_max].
               Does not wrap around at boundaries.
   @noi2c
   @nobit
   @int        RBSC_BASIC_INTERFACE
   @parm       Current control value (Word)
   @parm       Direction bit (FALSE: increment, TRUE: decrement)
   @parm       Maximum limit (Word)
   @parm       Minimum limit (Word)
   @rdesc      Updated control value (Word).
 */
Word rgen_ChangeControlWord( Word rgen_control,  Bool rgen_direction, Word rgen_max, Word rgen_min )
{     
    if (rgen_direction == FALSE)
    {
        if (rgen_control < rgen_max)
        {
            rgen_control++;
        }
    }
    else
    {
        if (rgen_control > rgen_min)
        {
            rgen_control--;
        }
    }

    return(rgen_control);
    
}

/*=======================================================================*/
/*                                                                       */
/*=======================================================================*/
Byte rbsc_GetLocalKey(void)
{
  Byte value;
    value = local_key_buffer;
  local_key_buffer = 0;
  return (value);
}
