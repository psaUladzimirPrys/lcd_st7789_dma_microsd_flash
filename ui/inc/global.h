#ifndef _HGLOBAL_H
#define _HGLOBAL_H


/*==========================================================================*/
/*        I N C L U D E S                                                   */
/*==========================================================================*/
#include <stdint.h>
#include <stdbool.h>


/*==========================================================================*/
/*        G L O B A L   D A T A   D E C L A R A T I O N S                   */
/*==========================================================================*/
typedef uint8_t     Bool; /* int must be 16-bit or 32-bit */
typedef uint8_t     String;
typedef uint8_t     Byte; /* char must be 8-bit */
typedef uint16_t    Word; /* 16-bit unsigned integer */
typedef uint16_t    SWord;  /* UTF-16 character type */
typedef uint32_t    Long;  /* 32-bit unsigned integer */
typedef uint64_t    LongLong;  /* 64-bit unsigned integer */

#define TRUE  ( 1 )
#define FALSE ( 0 )

/*==========================================================================*/
/*        G L O B A L   F U N C T I O N   P R O T O T Y P E S               */
/*==========================================================================*/



/*
   @type RGEN_CHANGE_UP |
   Define used for setting the direction of the change.
*/
#define RGEN_CHANGE_UP    0


/*
   @type RGEN_CHANGE_DOWN |
   Define used for setting the direction of the change.
*/
#define RGEN_CHANGE_DOWN  1

/*
   @type rgen_ChangeUpDown_type |
   Type definition used in the functions rgen_ChangeControl and
   rgen_ChangeControlAround. Use the defined values RGEN_CHANGE_UP and
   RGEN_CHANGE_DOWN for these functions.
*/
typedef Bool rgen_ChangeUpDown_type;

/*
  @type RGEN_ON |
  Define used for turning on a device, pin or function.

  @type RGEN_OFF |
  Define used for turning off a device, pin or function.
*/
/* @MacroType RGEN_ON_OFF_TYPE | 2 */
#define RGEN_OFF    0
#define RGEN_ON     1

/*
   @type rgen_OnOff_type |
   Type definition used for turning on and off of a device,pin
   or function. Use the defined values RGEN_ON and RGEN_OFF.
*/
typedef Bool rgen_OnOff_type;


/*
   @type RGEN_TIMER_STOPPED |
   Define used for determine if the timer has stopped.
*/
#define RGEN_TIMER_STOPPED   0


/*
   @type RGEN_TIMER_EXPIRED |
   Define used for determine if the timer has expired.
*/
#define RGEN_TIMER_EXPIRED   1


/*
   @type rgen_TimerExpired_type |
   Type definition used for determine if a timer has stopped or
   has expired. Use the defined values RGEN_TIMER_STOPPED and
   RGEN_TIMER_EXPIRED.
*/
typedef Bool rgen_TimerExpired_type;


/*

   @type RGEN_HIGH( a ) |
   Macro determine the high byte of a word.

*/
#define RGEN_HIGH( a ) (Byte)((a) >> 8)

/*

   @type RGEN_LOW( a ) |
   Macro determine the low byte of a word.

*/
#define RGEN_LOW( a )  (Byte)(a)

typedef void (* FUNCTION_PTR)  (void);

#endif /* _HGLOBAL_H */
