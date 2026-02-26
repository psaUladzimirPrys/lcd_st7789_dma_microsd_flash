/*
 * fuin_obs.h
 *
 *  Created on: 20 февр. 2026 г.
 *      Author: priss
 */

#ifndef UI_INC_FUIM_OBS_H_
#define UI_INC_FUIM_OBS_H_

/*=======================================================================*/
/*        I N C L U D E S                                                */
/*=======================================================================*/

#include "hglobal.h"
#include "fuim.h"

/*=======================================================================*/
/*        G L O B A L   D A T A   D E C L A R A T I O N S                */
/*=======================================================================*/

/*=======================================================================*/
/*           */
/*=======================================================================*/
osdFieldValue fuim_Observer(Byte index);
fuim_Validity fuim_ValidityFunction (Byte index);
osdFieldValue fuim_ActionHandler (      Byte index,          /* @parm Function ID */
                               osdFieldValue value           /* @parm given to the action handler function */
                                 );


#endif /* UI_INC_FUIM_OBS_H_ */
