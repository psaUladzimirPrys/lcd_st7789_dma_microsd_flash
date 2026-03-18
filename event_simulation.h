/*
*/

#ifndef EVENT_SIMULATION_H_
#define EVENT_SIMULATION_H_

// -----------------------------------------------------------------------------
//                       Includes
// -----------------------------------------------------------------------------



#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
**************************   GLOBAL FUNCTIONS   *******************************
*******************************************************************************/

void sim_Init(void);
void sim_Update(void);
void sim_TurnOn(void);
void sim_TurnOff(void);

#ifdef __cplusplus
}
#endif

#endif /* EVENT_SIMULATION_H_ */
