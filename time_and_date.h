#ifndef ALS_TIME_AND_DATE_H
#define ALS_TIME_AND_DATE_H

/*=======================================================================*/
/*        I N C L U D E S                                                */
/*=======================================================================*/
#include "app_log.h"
#include "sl_sleeptimer.h"


/*==========================================================================*/
/*        G L O B A L   D A T A   D E C L A R A T I O N S                   */
/*==========================================================================*/
/*
 * "HH:MMX" requires 6 characters + 1 null terminator = 7 bytes minimum.
 * Enforced dynamically to eliminate Out-of-Bounds SRAM write vectors.
 */

#define UI_DATE_TIME_STRING_BUFFER_SIZE  11U
#define UI_TIME_AM_PM_MIN_BUFFER_SIZE    8U
#define UI_AM_PM_SUFFIX_POSITION_INDEX   6U

/*==========================================================================*/
/*        G L O B A L   F U N C T I O N   P R O T O T Y P E S               */
/*==========================================================================*/

void set_rtc_from_unix_bytes(const uint8_t *buff);
sl_status_t get_formatted_date_string(char *out_str, size_t max_size);
sl_status_t get_formatted_time_string(char *out_str, size_t max_size);
uint64_t datetime_to_seconds(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t min, uint8_t sec);
void get_clocks_info(void);

#endif /* ALS_TIME_AND_DATE_H */
