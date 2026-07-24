#ifndef __ALS_GENERAL_DEFS_H__
#define __ALS_GENERAL_DEFS_H__

#define SAMPLES_BEFORE_PEAK       56   // Предыстория до пика
#define SAMPLES_AFTER_PEAK        199  // Хвост после пика (вместе с пиком дает 200)
#define TOTAL_IMPACT_SAMPLES      256  // Общий размер истории (ArraySize)

#define IMPACT_START_THRESHOLD   1850

#define SEARCH_WINDOW_HALF   5    // Окрестность для уточнения пика (+/- 5 отсчетов от 56)
#define NOISE_HYSTERESIS     150  // Порог фильтрации шума для фиксации минимума

// Константы ограничений (в реальном коде они считываются из Flash/EEPROM)
// Для примера приведем типовые значения OsteoProbe:
#define OVERSATURATION_LIMIT   4000   // Верхний предел 12-битного АЦП (из 4095)
#define AMP_MIN                150    // Минимальный размах (MIN_SIGNAL_VARIATION)
#define IMPACT_TIME_MIN_US     300    // Минимальное время удара в мкс (например, 300 мкс)
#define IMPACT_TIME_MAX_US     1200   // Максимальное время удара в мкс (например, 1200 мкс)

#endif
