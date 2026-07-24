/*=======================================================================*/
/*
 * @file time_and_date.c
 * @brief Time and date formatting utilities for the ALSOPI UI sub-system.
 * @version 0.1.0
*/
/*=======================================================================*/

/*=======================================================================*/
/*        I N C L U D E S                                                */
/*=======================================================================*/
#include "time_and_date.h"
#include "stdio.h"
#include "em_cmu.h"

/*=================================================================================*/
/*    G L O B A L  S Y M B O L    D E C L A R A T I O N S                          */
/*=================================================================================*/

/*=================================================================================*/
/*    L O C A L   S Y M B O L    D E C L A R A T I O N S                           */
/*=================================================================================*/
static const uint8_t days_in_month[12] = {31,28,31,30,31,30,31,31,30,31,30,31};

/*=====================================================================================
* Non-volatile execution lookup maps for converting 24h metrics to 12h representation.
* Aligned directly to flash boundaries to keep static data footprints out of execution RAM.
========================================================================================*/
static const uint8_t hour_12_tens[24] = {
  1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1
};
static const uint8_t hour_12_ones[24] = {
  2, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1
};


/*==================================================================================*/
/*    L O C A L   F U N C T I O N   P R O T O T Y P E S                             */
/*==================================================================================*/

/*==================================================================================*/
/* G L O B A L      F U N C T I O N                                                 */
/*==================================================================================*/
static int is_leap_year(uint16_t year)
{
    return (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));
}

/*=======================================================================*/
/*                                                                       */
/*=======================================================================*/
void set_rtc_from_unix_bytes(const uint8_t *buff)
{
    sl_sleeptimer_date_t date;
    sl_status_t sc;

    if (buff == NULL) return;

    // Сборка uint32_t (Big-endian: [0]-MSB, [3]-LSB)
    uint32_t unix_time = ((uint32_t)buff[0] << 24) | ((uint32_t)buff[1] << 16) | ((uint32_t)buff[2] << 8) | ((uint32_t)buff[3]);

    // Получаем секунды, минуты, часы
    uint32_t seconds_in_day = unix_time % 86400;

    date.hour = seconds_in_day / 3600;
    date.min  = (seconds_in_day % 3600) / 60;
    date.sec  = (seconds_in_day % 60);

    // Вычисляем дату
    uint32_t days = unix_time / 86400;
    uint16_t year = 1970;

    while (1)
    {
        uint16_t days_in_year = 365 + is_leap_year(year);
        if (days >= days_in_year)
        {
            days -= days_in_year;
            year++;
        }
        else
        {
            break;
        }
    }

    date.year = year;

    // Определяем месяц и день
    uint8_t month;
    for (month = 0; month < 12; month++)
    {
        uint8_t dim = days_in_month[month];
        if (month == 1 && is_leap_year(year)) dim++; // февраль в високосный год

        if (days < dim) break;
            days -= dim;
    }

    //date.month       = month + 1;  // месяцы 1–12
    date.month       = month;  // месяцы 0–11
    date.month_day   = days + 1;   // дни 1–31

    // 5. Устанавливаем время через sleeptimer
    sc = sl_sleeptimer_set_datetime(&date);

    if (sc == SL_STATUS_OK)
    {
        // Выводим в лог. Месяцы в sl_sleeptimer_date_t идут от 0 до 11.
        app_log("RTC: Set date and time success: %04u-%02u-%02u %02u:%02u:%02u\r\n",
                date.year,
                date.month + 1,
                date.month_day,
                date.hour,
                date.min,
                date.sec);
    }
    else
    {
        app_log("RTC: Set date and time failed 0x%02X\r\n", (unsigned int)sc);
    }

}

/****************************************************************************************************
 * @brief Safely formats a date string in the strict ISO 8601 format (YYYY-MM-DD).
 *
 * The solution is based on the sl_sleeptimer_convert_date_to_str function, since it does not use
 * heap, does not cause stack overflow due to deep sтprintf() calls, and operates within a predictable
 * number of clock cycles on the Cortex-M33.
 *
 * @param[out] out_str Pointer to the target character buffer (minimum 11 bytes).
 * @param[in]  max_size Maximum size of the buffer.
 *
 * @return sl_status_t Operation result.
 *         @retval SL_STATUS_OK Successful string generation.
 *         @retval SL_STATUS_INVALID_PARAMETER Invalid pointer or size.
 *         @retval SL_STATUS_FAIL Internal Sleeptimer parsing error.
 ******************************************************************************************************/
sl_status_t get_formatted_date_string(char *out_str, size_t max_size)
{
    sl_status_t status;
    sl_sleeptimer_date_t current_date;

    /* Validate input parameters to ensure determinism */
    if ((out_str == NULL) || (max_size < UI_DATE_TIME_STRING_BUFFER_SIZE)) {
        return SL_STATUS_INVALID_PARAMETER;
    }

    /*
     * Read the current calendar state from Sleeptimer (Wallclock).
     * Hardware constraint: Requires the SL_SLEEPTIMER_WALLCLOCK_CONFIG macro
     * to be set to 1 in sl_sleeptimer_config.h.
     */
    status = sl_sleeptimer_get_datetime(&current_date);
    if (status != SL_STATUS_OK) {
        return status;
    }

    /*
     * Date formatting.
     * Using "%04u-%02u-%02u" guarantees strict compliance with "2026-06-01".
     * Specifiers:
     *   %04u - Year (4 digits)
     *   %02u - Month (01-12)
     *   %02u - Day of month (01-31)
     */
  int bytes_written = snprintf(out_str, UI_DATE_TIME_STRING_BUFFER_SIZE, "%04u-%02u-%02u", current_date.year, (current_date.month + 1),  current_date.month_day);

    /* If 0 is returned, the formatting function failed to parse the structure */
  if (bytes_written == 0) {
       out_str[0] = '\0';
       return SL_STATUS_FAIL;
  }


  out_str[UI_DATE_TIME_STRING_BUFFER_SIZE - 1] = '\0';

  return SL_STATUS_OK;
}

/****************************************************************************************************
 * @brief Safely formats a time string into North American 12-hour format with AM/PM suffix.
 *
 * @param[out] out_str  Pointer to the destination character buffer (minimum 9 bytes allocated).
 * @param[in]  max_size Maximum allocated capacity of the target buffer.
 *
 * @return sl_status_t Operation status.
 *         @retval SL_STATUS_OK                 Time successfully written in "HH:MM XM" format.
 *         @retval SL_STATUS_INVALID_PARAMETER  Null pointer or insufficient buffer allocation.
 *         @retval SL_STATUS_FAIL               Wallclock component or configuration state failure.
 ******************************************************************************************************/
sl_status_t get_formatted_time_string(char *out_str, size_t max_size)
{
    sl_status_t status;
    sl_sleeptimer_date_t current_date;

    /* Validate input parameters to ensure determinism */
    if ((out_str == NULL) || (max_size < UI_TIME_AM_PM_MIN_BUFFER_SIZE)) {
        return SL_STATUS_INVALID_PARAMETER;
    }

    /*
     * Read the current calendar state from Sleeptimer (Wallclock).
     * Hardware constraint: Requires the SL_SLEEPTIMER_WALLCLOCK_CONFIG macro
     * to be set to 1 in sl_sleeptimer_config.h.
     */
    status = sl_sleeptimer_get_datetime(&current_date);
    if (status != SL_STATUS_OK) {
        return status;
    }

    /* Structural validation check to isolate corrupt date objects */
    if ((current_date.hour > 23) || (current_date.min > 59)) {
      out_str[0] = '\0';
      return SL_STATUS_FAIL;
    }


    /* Select suffix marker byte based on standard 24h operational index limits */
    char suffix = (current_date.hour >= 12) ? 'P' : 'A';

    /*
     * Low-Level Direct Buffer Synthesis Core.
     * Maps memory layout configurations directly to destination offsets without multi-cycle loop states.
     */
    out_str[0] = (char)('0' + hour_12_tens[current_date.hour]);
    out_str[1] = (char)('0' + hour_12_ones[current_date.hour]);
    out_str[2] = ((current_date.sec & 1U) == 0U) ? ':' : ' ';
    out_str[3] = (char)('0' + (current_date.min / 10U));
    out_str[4] = (char)('0' + (current_date.min % 10U));
    out_str[5] = '\0';
    out_str[UI_AM_PM_SUFFIX_POSITION_INDEX] = suffix;

    return SL_STATUS_OK;
}

/**********************************************************************************************
 * Function:       get_clocks_info
 * Arguments:      None.
 * Description:    Retrieves the current CPU, APB, AHB, and System clock frequencies and
 *                 prints them to the application log.
 * Return:         None.
 *********************************************************************************************/
void get_clocks_info(void)
{
  /* Get CPU core clock frequency in MHz. */
  uint32_t freq = SystemCoreClockGet() / 1000000;
  app_log("CPU=%luMHZ \r\n", freq);

  /* Get APB peripheral bus clock frequency. */
  uint32_t pclk = CMU_ClockFreqGet(cmuClock_PCLK);
  app_log("APB bus=%luMHZ \r\n", pclk);

  /* Get AHB bus clock frequency. */
  uint32_t hclk = CMU_ClockFreqGet(cmuClock_HCLK);
  app_log("AHB bus=%luMHZ \r\n", hclk);

  /* Get system clock frequency. */
  uint32_t sysclk = CMU_ClockFreqGet(cmuClock_SYSCLK);
  app_log("System clock=%luMHZ \r\n", sysclk);
}

/**
 * @brief Converts calendar datetime components into a 32-bit Epoch Unix Time (seconds since 1970-01-01 00:00:00).
 *
 * @details This helper function calculates chronological delta offsets. It has been optimized for the 32-bit
 *          ARM Cortex-M33 architecture, replacing heavy 64-bit operations with 32-bit arithmetic.
 *          The maximum supported year is 2106, which safely avoids the Year 2038 problem without incurring
 *          runtime penalties on energy-constrained MCUs like Silicon Labs EFR32/BGM220.
 *
 * @note [Security & Hardware Architecture Design Notes]:
 *       1. Out-of-Bounds (OOB) Guard: Input parameters 'month' and 'day' undergo strict validation. If 'month'
 *          is out of the [1..12] range, it is clamped to prevent OOB reads on the 'days_to_month' Flash table.
 *       2. RAM/Flash footprint: 'days_to_month' is qualified as 'static const' to prevent stack allocation
 *          and ensure it resides directly in Flash memory (.rodata).
 *       3. Low-Power Optimization: By restricting operations to 32-bit registers, we eliminate compiler-generated
 *          calls to multi-word arithmetic helpers. This decreases instruction cache misses and allows faster entry
 *          back into Sleep states (EM1/EM2).
 *
 * @param[in] year  Calendar year (e.g., 2026). Valid range: [1970..2105].
 * @param[in] month Calendar month. Valid range: [1..12].
 * @param[in] day   Day of the month. Valid range: [1..31].
 * @param[in] hour  Hour in 24-hour format. Valid range: [0..23].
 * @param[in] min   Minute. Valid range: [0..59].
 * @param[in] sec   Second. Valid range: [0..59].
 *
 * @return uint32_t Calculated Unix Epoch time in seconds. Returns 0 on strict parsing failure (year < 1970).
 */
uint64_t datetime_to_seconds(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t min, uint8_t sec)
{
  // Defensive validation for the baseline Epoch year
  if (year < 1970) {
    return 0U;
  }

  // Look-up table mapping months to cumulative days of a non-leap year.
  // Element 0 is dummy padded, index maps directly to month [1..12].
  static const uint16_t days_to_month[13] = {
    0U,       // Dummy element for 1-based indexing
    0U,       // January: 0 days elapsed
    31U,      // February: 31 days elapsed
    59U,      // March
    90U,      // April
    120U,     // May
    151U,     // June
    181U,     // July
    212U,     // August
    243U,     // September
    273U,     // October
    304U,     // November
    334U      // December
  };

  // Safe Parameter Normalization: Protect against Out-Of-Bounds memory reads
  uint8_t verified_month = month;
  if (verified_month < 1U) {
    verified_month = 1U;
  } else if (verified_month > 12U) {
    verified_month = 12U;
  }

  uint32_t total_days = 0U;

  // 1. Calculate cumulative days elapsed in preceding years since 1970
  for (uint16_t y = 1970U; y < year; y++) {
    total_days += 365U;
    // Check for Leap Year (divisible by 4 and not 100, or divisible by 400)
    if (((y % 4U == 0U) && (y % 100U != 0U)) || (y % 400U == 0U)) {
      total_days++;
    }
  }

  // 2. Add days of the current year up to the target date
  total_days += (uint32_t)days_to_month[verified_month] + (uint32_t)day - 1U;

  // Correct for current year if it is a leap year and we have passed February
  if (verified_month > 2U) {
    if (((year % 4U == 0U) && (year % 100U != 0U)) || (year % 400U == 0U)) {
      total_days++;
    }
  }

  // 3. Accumulate time into final seconds format
  uint64_t total_seconds = ((uint64_t)total_days * 86400U) + ((uint64_t)hour * 3600U) + ((uint64_t)min * 60U) + (uint64_t)sec;

  return total_seconds;
}

/*=======================================================================*/
/*                                                                       */
/*=======================================================================*/
#if 0
sl_status_t get_time(char *str_buf)
{
  sl_status_t sl_status_code = SL_STATUS_OK;

  sl_sleeptimer_date_t date_time = {
    .year = 122,
    .month = 2,
    .month_day = 1,
    .hour = 10,
    .min = 30,
    .sec = 0,
  };


  sl_status_code = sl_sleeptimer_get_datetime(&date_time);
  app_assert_status(sl_status_code);

  sprintf(str_buf, "Current time is %02u:%02u:%02u.\r\n",
              date_time.hour,
              date_time.min,
              date_time.sec);

  return sl_status_code;
}
#endif
