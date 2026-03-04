/*=======================================================================*/
#ifndef _HFPMT_API_H
#define _HFPMT_API_H

/*=======================================================================*/
/*        I N C L U D E S                                                */
/*=======================================================================*/



/*=======================================================================*/
/*        G L O B A L   D A T A   D E C L A R A T I O N S                */
/*=======================================================================*/

typedef enum
{
   FPMT_STAND_BY = 0,
   FPMT_POWER_ON,
   FPMT_ERROR
}fpmtPowerMode_enum;


/*=======================================================================*/
/*        G L O B A L   F U N C T I O N   P R O T O T Y P E S            */
/*=======================================================================*/

void fpmt_Init(void);
void fpmt_Update( void );

void fpmt_SetPowerState(fpmtPowerMode_enum PowerState);
fpmtPowerMode_enum fpmt_GetPowerState(void);

void fpmt_HandleCommand(void) ;


#endif /* _HFPMT_API_H */
