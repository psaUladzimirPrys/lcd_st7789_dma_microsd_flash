/*
 * auph.h
 *
 *  Created on: Feb 24, 2026
 *      Author: priss
 */

#ifndef UI_INC_AUPH_H_
#define UI_INC_AUPH_H_

/*==========================================================================*/
/*        I N C L U D E S                                                   */
/*==========================================================================*/
#include "hglobal.h"
#include "aukh.h"

/*==========================================================================*/
/*        L O C A L   D A T A   C O N F I G U R A T I O N                   */
/*==========================================================================*/


/*==========================================================================*/
/*        L O C A L   F U N C T I O N   P R O T O T Y P E S                 */
/*==========================================================================*/

/*==========================================================================*/
/*        L O C A L   D A T A   D E C L A R A T I O N S                     */
/*==========================================================================*/

/*=======================================================================*/
/*    G L O B A L   D A T A   D E C L A R A T I O N S                    */
/*=======================================================================*/
/*==========================================================================*/
/*
   @type aukh_current |
         Contains the last key read from the local keyboard or remote control
         that is currently being processed.
         It is set to inactive when the command has been processed.
*/
extern AU_COMMAND  au_current ;/* Contains the current key read
                                       from the remote control or
                                       local keyboard.
                                       This variable also contains
                                       the toggle bit when a button is held down. */

/*==========================================================================*/
/*
   @type au_direction |
         Defines the direction as AUKH_UP or AUKH_DOWN.

      If the current command is even    aukh_direction = TRUE;
      If the current command is odd     aukh_direction = FALSE;
*/
extern Bool au_direction;

/*=======================================================================*/
/*    G L O B A L   F U N C T I O N P R O T O T Y P E S                  */
/*=======================================================================*/

extern void auph_SetState(auphOsteoState_enum new_state) ;
extern auphOsteoState_enum auph_GetState(void);
extern void auph_ProcessKey(void) ;

#endif /* UI_INC_AUPH_H_ */
