/*
*/

#ifndef _AEVS_H_
#define _AEVS_H_
/*==========================================================================*/
/*        I N C L U D E S                                                   */
/*==========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*=========================================================================*/
/*   G L O B A L   D E F I N I T I O N S                                   */
/*=========================================================================*/

//#define  AEVS_BAT_NORMAL_UPPER_BORDER
#define  AEVS_BAT_LOW_50_UPPER_BORDER    95//90
#define  AEVS_BAT_LOW_30_UPPER_BORDER    59//49
#define  AEVS_BAT_LOW_10_UPPER_BORDER    39//29
//#define  AEVS_BAT_CRITIC_UPPER_BORDER


#define  AEVS_BAT_NORMAL_BOTTOM_BORDER   99//98
#define  AEVS_BAT_LOW_50_BOTTOM_BORDER   60//50
#define  AEVS_BAT_LOW_30_BOTTOM_BORDER   40//30
#define  AEVS_BAT_LOW_10_BOTTOM_BORDER   21//11
#define  AEVS_BAT_CRITIC_BOTTOM_BORDER   15//8

// communication with display subsystem -------------------------
typedef enum
{
  AEVS_NO_ACTIVE_TASK = 0,
  AEVS_LOADING_ACTIVE_TASK,
  AEVS_BATTERY_ACTIVE_TASK,
  AEVS_PERF_CHECK_ACTIVE_TASK,

} aevs_active_task_idx_t;

/*===========================================================================*/
/*    G L O B A L   F U N C T I O N     P R O T O T Y P E S                  */
/*===========================================================================*/

void aevs_Init(void);
void aevs_Update(void);
void aevs_TurnOn(void);
void aevs_TurnOff(void);
void aevs_ForceChargeMenuCheck(void);

#ifdef __cplusplus
}
#endif

#endif /* _AEVS_H_ */
