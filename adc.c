#include "adc.h"
#include "calculations.h"
#include "file_storage.h"
#include "historical_records.h"

extern uint8_t tx_data_buffer[MAX_TX_BUFFER_SIZE];;
volatile uint16_t adc_unified_buffer[TOTAL_IMPACT_SAMPLES] = {0};
volatile adc_capture_state_t adc_current_state = ADC_STATE_IDLE;
volatile uint16_t adc_battery_level = 0;
volatile uint16_t adc_check_connection_level = 0;
volatile uint32_t adc_interrupt_counter = 0;

static volatile uint16_t adc_impact_start_index = 0;
volatile adc_channel_t current_active_channel = ADC_CHANNEL_SIGNAL;
static uint16_t write_ptr = 0;
static uint16_t peak_value = 0;
static uint16_t peak_buffer_idx = 0;
static uint16_t post_peak_counter = 0;

static int prs_allocated_ch = -1;

extern device_params_t device_params;

measurement_sub_mode_t current_sub_mode = SUB_MODE_NONE;
measurement_state_2_t measurement_state_2;

bool is_tip_marked_used_in_session = false;

measurement_session_t *active_session = NULL; // Глобальный указатель на ту сессию (пациент или референс), которая сейчас собирается

extern measurement_session_t reference_session;
extern final_bmsi_result_t final_results;

extern device_operation_control_t   device_operation_control;
extern income_packet_t rx_data_buffer;

volatile bool is_measurement_paused = false;
uint32_t strm_timer = 0;

//==========================================================================================================================
working_mode_t current_working_mode = WORKING_MODE_NONE;
working_sub_mode_t current_working_sub_mode = WORKING_SUB_MODE_NONE;
measurement_machine_state_t measurement_state_machine = MACHNE_STATE_IDLE;

// Переиспользуемые структуры данных
measurement_session_t measurement_session;
//measurement_session_t reference_session;
//final_bmsi_result_t final_results;

bool is_currnet_measurement_paused = false;

uint32_t safe_timer = 0;

#define BATTERY_AVG_SAMPLES   16  // Количество замеров для усреднения батареи
#define CONNECTION_AVG_SAMPLES 16 // Количество замеров для проверки подключения

uint32_t battery_accumulator = 0;
uint16_t battery_sample_cnt = 0;
bool battery_ready = false;

uint32_t connection_accumulator = 0;
uint16_t connection_sample_cnt = 0;
bool connection_ready = false;


/**
  * @brief Проверка завершения захвата.
  */
bool is_adc_capture_done(void)
{
   return (adc_current_state == ADC_STATE_DONE);
}


//==============================================================================================================
/*******************************************************************************
 * @brief Инициализация IADC в режиме Scan, два канала, PRS-триггер
 *
 *  Scan entry 0 (id=0): PA8 — сигнал ударного датчика
 *  Scan entry 1 (id=1): PA7 — измерение батареи
 *
 *  Вызывается один раз при старте системы.
 ******************************************************************************/
void init_iadc_scan(adc_vref_source_t vref_source)
{
    IADC_Init_t       init          = IADC_INIT_DEFAULT;
    IADC_AllConfigs_t initAllConfig = IADC_ALLCONFIGS_DEFAULT;
    IADC_InitScan_t   initScan      = IADC_INITSCAN_DEFAULT;
    IADC_ScanTable_t  scanTable     = IADC_SCANTABLE_DEFAULT;

    // Тактирование
    CMU_ClockEnable(cmuClock_IADC0, true);
    CMU_ClockEnable(cmuClock_GPIO,  true);

    // FSRCO (20 МГц) — работает в EM2, не требует HFXO
    CMU_ClockSelectSet(cmuClock_IADCCLK, cmuSelect_FSRCO);

    // GPIO — ОБЯЗАТЕЛЬНО до ABUSALLOC
    // Reference Manual: пины АЦП должны быть в gpioModeDisabled
    GPIO_PinModeSet(gpioPortA, 8, gpioModeDisabled, 0); // PA8 — сигнал удара
    GPIO_PinModeSet(gpioPortA, 7, gpioModeDisabled, 0); // PA7 — батарея

    // Выделение аналоговых шин ABUS
    // PA8 — чётный пин → AEVEN0
    // PA7 — нечётный пин → AODD0
    // Read-modify-write чтобы не затронуть AEVEN1/AODD1
    uint32_t ab = GPIO->ABUSALLOC;
    ab &= ~(_GPIO_ABUSALLOC_AEVEN0_MASK | _GPIO_ABUSALLOC_AODD0_MASK);
    ab |= (GPIO_ABUSALLOC_AEVEN0_ADC0 | GPIO_ABUSALLOC_AODD0_ADC0);
    GPIO->ABUSALLOC = ab;

    // Базовая конфигурация IADC
    // Выключаем IADC между преобразованиями — снижает потребление
    init.warmup = iadcWarmupNormal;
    // iadcClkSuspend0 гейтирует CLK_ADC пока нет PRS-триггера для Scan
    init.iadcClkSuspend0 = true;
    // Прескалер источника тактирования
    init.srcClkPrescale = IADC_calcSrcClkPrescale(IADC0, 20000000/*CLK_SRC_ADC_FREQ*/, 0);

    // Конфигурация 0 — опорное напряжение
    switch (vref_source)
    {
        case ADC_VREF_INTERNAL_1P21V:
            initAllConfig.configs[0].reference  = iadcCfgReferenceInt1V2;
            initAllConfig.configs[0].vRef       = 1210;
            initAllConfig.configs[0].analogGain = iadcCfgAnalogGain1x;//iadcCfgAnalogGain0P5x;
            break;

        case ADC_VREF_INTERNAL_3P0V:
            initAllConfig.configs[0].reference  = iadcCfgReferenceVddx;
            initAllConfig.configs[0].vRef       = 3000;
            initAllConfig.configs[0].analogGain = iadcCfgAnalogGain1x;
            break;

        case ADC_VREF_EXTERNAL_PA00:
            GPIO_PinModeSet(gpioPortA, 0, gpioModeDisabled, 0);
            initAllConfig.configs[0].reference  = iadcCfgReferenceExt1V25;
            initAllConfig.configs[0].vRef       = EXTERNAL_VREF_VALUE_MV;
            initAllConfig.configs[0].analogGain = iadcCfgAnalogGain1x;
            break;

        default:
            initAllConfig.configs[0].reference  = iadcCfgReferenceInt1V2;
            initAllConfig.configs[0].vRef       = 1210;
            initAllConfig.configs[0].analogGain = iadcCfgAnalogGain1x; //iadcCfgAnalogGain0P5x;
            break;
    }

    // OSR 2x → 12-bit результат
    // Время преобразования = ((4 * OSRHS) + 2) / fCLK_ADC
    // При 2x OSR и 10 МГц → макс. 833 ksps
    initAllConfig.configs[0].osrHighSpeed = iadcCfgOsrHighSpeed2x;

    // Прескалер ядра АЦП
    initAllConfig.configs[0].adcClkPrescale = IADC_calcAdcClkPrescale( IADC0, 10000000/*CLK_ADC_FREQ*/, 0, iadcCfgModeNormal, init.srcClkPrescale);

    // Конфигурация Scan

    // Триггер — нарастающий фронт PRS0
    // PRS0 — выделенный вход для Scan; PRS1 — для Single
    initScan.triggerSelect  = iadcTriggerSelPrs0PosEdge;
    initScan.triggerAction  = iadcTriggerActionOnce;

    // Прерывание когда в FIFO накопилось 2 результата (оба канала)
    initScan.dataValidLevel = iadcFifoCfgDvl2;

    // ОБЯЗАТЕЛЬНО: тегировать записи FIFO номером строки таблицы
    // Без этого result.id всегда будет 0
    initScan.showId = true;

    // НЕ запускаем сразу — запуск через iadcCmdStartScan
    // в start_impact_capture()
    initScan.start = false;

    // Таблица Scan: два single-ended канала

    // Entry 0 (id=0): PA8 — сигнал ударного датчика
    scanTable.entries[0].posInput      = iadcPosInputPortAPin8;
    scanTable.entries[0].negInput      = iadcNegInputGnd;
    scanTable.entries[0].includeInScan = true;

    // Entry 1 (id=1): PA7 — измерение батареи
    scanTable.entries[1].posInput      = iadcPosInputPortAPin7;
    scanTable.entries[1].negInput      = iadcNegInputGnd;
    scanTable.entries[1].includeInScan = true;

    // Инициализация периферии
    IADC_init(IADC0, &init, &initAllConfig);
    IADC_initScan(IADC0, &initScan, &scanTable);

    // Прерывания
    IADC_clearInt(IADC0, _IADC_IF_MASK);
    IADC_enableInt(IADC0, IADC_IEN_SCANFIFODVL);

    NVIC_ClearPendingIRQ(IADC_IRQn);
    NVIC_EnableIRQ(IADC_IRQn);
}

void init_timer1_for_adc_scan(uint32_t microseconds)
{
    CMU_ClockEnable(cmuClock_TIMER1, true);

    uint32_t base_freq = CMU_ClockFreqGet(cmuClock_TIMER1);
    if (base_freq == 0) {
        base_freq = 76800000U;
    }

    TIMER_Prescale_TypeDef prescaler  = timerPrescale1;
    uint32_t               div_factor = 1;

    if (microseconds < 800) {
        prescaler  = timerPrescale1;
        div_factor = 1;
    }
    else if (microseconds < 3000) {
        prescaler  = timerPrescale4;
        div_factor = 4;
    }
    else if (microseconds < 12000) {
        prescaler  = timerPrescale16;
        div_factor = 16;
    }
    else if (microseconds < 50000) {
        prescaler  = timerPrescale64;
        div_factor = 64;
    }
    else {
        prescaler  = timerPrescale1024;
        div_factor = 1024;
    }

    uint64_t total_hardware_ticks = ((uint64_t)base_freq * (uint64_t)microseconds) / 1000000ULL;
    uint32_t topValue = (uint32_t)(total_hardware_ticks / div_factor) - 1;

    if (topValue > 65535) topValue = 65535;
    if (topValue == 0)    topValue = 1;

    TIMER_Init_TypeDef timerInit = TIMER_INIT_DEFAULT;
    timerInit.enable   = false;
    timerInit.prescale = prescaler;
    timerInit.mode     = timerModeUp;
    TIMER_Init(TIMER1, &timerInit);

    // Аппаратный PRS-импульс через CC0
    TIMER_InitCC_TypeDef ccInit = TIMER_INITCC_DEFAULT;
    ccInit.mode      = timerCCModeCompare;
    ccInit.cofoa     = timerOutputActionToggle;
    ccInit.prsOutput = timerPrsOutputPulse;
    TIMER_InitCC(TIMER1, 0, &ccInit);

    TIMER_TopSet(TIMER1, topValue);
    TIMER_CounterSet(TIMER1, 0U);

    // IRQ не нужен — PRS работает аппаратно через CC0
}

void init_prs_for_adc_scan(void)
{
    CMU_ClockEnable(cmuClock_PRS, true);

    if (prs_allocated_ch < 0) {
        // prsTypeAsync — обязательно для IADC
        prs_allocated_ch = PRS_GetFreeChannel(prsTypeAsync);
        if (prs_allocated_ch < 0) {
            return;
        }
    }
    // Аппаратный импульс от CC0
    PRS_SourceAsyncSignalSet(prs_allocated_ch, PRS_ASYNC_CH_CTRL_SOURCESEL_TIMER1, PRS_TIMER1_CC0);
    // Scan mode вместо Single mode
    PRS_ConnectConsumer(prs_allocated_ch, prsTypeAsync, prsConsumerIADC0_SCANTRIGGER);
}

/*******************************************************************************
 * @brief Начало сессии захвата данных.
 *
 * Периферия уже инициализирована в app_init().
 * Здесь только сбрасываем FIFO и запускаем таймер.
 ******************************************************************************/
void start_capture_adc_scan(void)
{
    // Сброс состояния буфера и машины состояний
    write_ptr              = 0;
    peak_value             = 0;
    post_peak_counter      = 0;
    adc_impact_start_index = 0;
    current_active_channel = ADC_CHANNEL_SIGNAL;
    adc_current_state      = ADC_STATE_PRE_TRIGGER;

    // Останавливаем Scan очередь перед flush
    // Reference Manual: очередь должна быть остановлена до flush
    IADC0->CMD = IADC_CMD_SCANSTOP;
    while (IADC0->STATUS & _IADC_STATUS_SCANQEN_MASK);
    // Очищаем Scan FIFO от остатков предыдущей сессии
    // IADC должен быть включён и тактирован
    IADC0->CMD = IADC_CMD_SCANFIFOFLUSH;
    while (IADC0->STATUS & _IADC_STATUS_SCANFIFOFLUSHING_MASK);
    // Сбрасываем счётчик таймера чтобы первый импульс
    // пришёл ровно через один период, а не раньше
    TIMER_CounterSet(TIMER1, 0U);
    // Разрешаем Scan очередь — ждёт PRS-триггеров
    IADC_command(IADC0, iadcCmdStartScan);
    // Запускаем таймер — CC0 начнёт генерировать PRS-импульсы
    TIMER_Enable(TIMER1, true);
}

/*******************************************************************************
 * @brief Остановка сессии захвата данных.
 *
 * Вызывается из IRQ-обработчика IADC когда набрано достаточно точек,
 * либо принудительно извне (таймаут, отмена).
 ******************************************************************************/
void stop_capture_adc_scan(void)
{
    // Останавливаем таймер — PRS-импульсы прекращаются
    TIMER_Enable(TIMER1, false);
    // Останавливаем Scan очередь
    // Любое уже начатое преобразование завершится само
    IADC0->CMD = IADC_CMD_SCANSTOP;
    while (IADC0->STATUS & _IADC_STATUS_SCANQEN_MASK);
    // Обновляем состояние машины если вызвано принудительно извне
    // (из IRQ оно уже будет ADC_STATE_DONE)
    //if (adc_current_state != ADC_STATE_DONE) {
    //    adc_current_state = ADC_STATE_DONE;
    //}
}

/**
 * Запустить таймер на один период
 */
void read_idle_samples(void)
{
   // Рекомендация Reference Manual: останавливаем Scan-очередь перед сбросом FIFO
   IADC0->CMD = IADC_CMD_SCANSTOP;
   while (IADC0->STATUS & _IADC_STATUS_SCANQEN_MASK);
   // Очищаем Scan FIFO от возможных остатков данных прошлых сессий
   IADC0->CMD = IADC_CMD_SCANFIFOFLUSH;
   while (IADC0->STATUS & _IADC_STATUS_SCANFIFOFLUSHING_MASK);
   // Сбрасываем счётчик аппаратного таймера TIMER1, чтобы первый импульс ушёл ровно через заданный квант времени
   TIMER_CounterSet(TIMER1, 0U);
   // Разрешаем Scan-очередь — теперь она аппаратно ожидает входящих PRS-импульсов
   IADC_command(IADC0, iadcCmdStartScan);
   // Запускаем таймер TIMER1. Через CC0 он начнёт периодически генерировать импульсы PRS,
   // запускающие одновременное чтение каналов удара (PA8) и батареи (PA7)
   TIMER_Enable(TIMER1, true);
}

void init_adc_scan(void)
{
  init_timer1_for_adc_scan(device_params.advanced_params.measure_params.adc_taking_period);
  init_prs_for_adc_scan();
  init_iadc_scan(device_params.advanced_params.measure_params.adc_vref_source);
}


/*******************************************************************************
 * @brief IADC Scan interrupt handler
 *
 * result.id == 0 → PA8 — сигнал ударного датчика
 * result.id == 1 → PA7 — батарея
 ******************************************************************************/
void IADC_IRQHandler(void)
{
    uint16_t signal_sample  = 0;
    uint16_t battery_sample = 0;
    bool     got_signal     = false;
    bool     got_battery    = false;

    uint32_t active_interrupts = IADC0->IF & IADC0->IEN;

    if (!(active_interrupts & IADC_IF_SCANFIFODVL)) {
        IADC_clearInt(IADC0, _IADC_IF_MASK);
        return;
    }

    // Вычитываем все доступные результаты из FIFO
    while (IADC_getScanFifoCnt(IADC0))
    {
        IADC_Result_t result = IADC_pullScanFifoResult(IADC0);

        if (result.id == 0) {
            signal_sample = result.data;
            got_signal    = true;
        }
        else if (result.id == 1) {
            battery_sample = result.data;
            got_battery    = true;
        }
    }

    // Режим IDLE: накапливаем замеры для усреднения обоих каналов.
    // Таймер останавливается только когда оба канала набрали нужное
    // количество замеров — через явные флаги готовности.
    if (adc_current_state == ADC_STATE_IDLE)
        {
            // Накопление сигнального канала (проверка подключения)
            if (got_signal && !connection_ready)
            {
                connection_accumulator += signal_sample;
                connection_sample_cnt++;

                if (connection_sample_cnt >= CONNECTION_AVG_SAMPLES) {
                    adc_check_connection_level = connection_accumulator / CONNECTION_AVG_SAMPLES;
                    connection_accumulator     = 0;
                    connection_sample_cnt      = 0;
                    connection_ready           = true;
                }
            }

            // Накопление батарейного канала
            if (got_battery && !battery_ready)
            {
                battery_accumulator += battery_sample;
                battery_sample_cnt++;

                if (battery_sample_cnt >= BATTERY_AVG_SAMPLES) {
                    adc_battery_level   = battery_accumulator / BATTERY_AVG_SAMPLES; // Результат пишется в глобальную переменную
                    battery_accumulator = 0;
                    battery_sample_cnt  = 0;
                    battery_ready       = true;
                }
            }

            // Останавливаем только когда оба канала усреднены
            if (connection_ready && battery_ready) {
                connection_ready = false;
                battery_ready    = false;

                // Выключаем периферию, чтобы не потреблять энергию в простое
                TIMER_Enable(TIMER1, false);
                IADC_command(IADC0, iadcCmdStopScan);

                adc_current_state = ADC_STATE_DONE;
            }

            IADC_clearInt(IADC0, IADC_IF_SCANFIFODVL);
            return;
        }

    // Режим захвата удара: используем только сигнальный канал (id=0)
    // Батарейный канал (id=1) в этом режиме игнорируем
    if (got_signal)
    {
        uint16_t sample = signal_sample;

        adc_unified_buffer[write_ptr] = sample;
        adc_interrupt_counter++;

        switch (adc_current_state)
        {
            case ADC_STATE_PRE_TRIGGER:
                if (sample > IMPACT_START_THRESHOLD) {
                    peak_value        = sample;
                    peak_buffer_idx   = write_ptr;
                    adc_current_state = ADC_STATE_FIND_PEAK;
                }
            break;

            case ADC_STATE_FIND_PEAK:
                if (sample > peak_value) {
                    peak_value      = sample;
                    peak_buffer_idx = write_ptr;
                }
                else {
                    adc_impact_start_index = (peak_buffer_idx - SAMPLES_BEFORE_PEAK) & 0xFF;
                    post_peak_counter      = 0;
                    adc_current_state      = ADC_STATE_POST_PEAK;
                }
            break;

            case ADC_STATE_POST_PEAK:
                post_peak_counter++;
                if (post_peak_counter >= SAMPLES_AFTER_PEAK) {
                    TIMER_Enable(TIMER1, false);
                    IADC0->CMD        = IADC_CMD_SCANSTOP;
                    adc_current_state = ADC_STATE_DONE;
                }
            break;

            default:
                break;
        }
        // Инкремент физического указателя записи по кольцу (256 точек)
        write_ptr = (write_ptr + 1) & 0xFF;
    }

    IADC_clearInt(IADC0, IADC_IF_SCANFIFODVL);
}



//==============================================================================================================

void is_stilus_connected_process(void)
{
  static uint32_t last_recall_time = 0;

    // Работает только тогда, когда основной автомат находится в IDLE
    if (measurement_state_machine != MACHNE_STATE_IDLE) return;

    uint32_t current_time = sl_sleeptimer_tick_to_ms(sl_sleeptimer_get_tick_count());

    // Раз в 3 секунды просто анализируем последнее сохраненное значение.
    // Никаких запусков АЦП тут делать не нужно — менеджер батареи всё делает за нас.
    if (current_time - last_recall_time >= NIDLE_CONNECTION_CHECK_TIME_MS) {
        last_recall_time = current_time;

        if (adc_check_connection_level < NIDLE_CONNECTION_ADC_LEVEL) {
            device_operation_control.battary.starain_gauge_value = adc_check_connection_level;
            device_operation_control.battary.starain_gauge_status = 0; // Не подключен
        } else {
            device_operation_control.battary.starain_gauge_value = adc_check_connection_level;
            device_operation_control.battary.starain_gauge_status = 1; // Подключен
        }
    }
}



void fsrv_send_final_bmsi_packet(void)
{
  if((current_working_mode == WORKING_MODE_PATIENT) && (current_working_sub_mode == WORKING_SUB_MODE_PATIENT_MEASUREMENT)) {
      send_final_bmsi_packet(&final_results, current_working_mode, current_working_sub_mode); app_log("PATIENT Bmsi\r\n");
   }

   if((current_working_mode == WORKING_MODE_PATIENT) && (current_working_sub_mode == WORKING_SUB_MODE_PATIENT_REFERENCE)) {
       send_final_bmsi_packet(&final_results, current_working_mode, current_working_sub_mode); app_log("PATIENT-REF Bmsi\r\n");
   }

   if((current_working_mode == WORKING_MODE_PERFORMANCE) && (current_working_sub_mode == WORKING_SUB_MODE_PERFORMANCE_MEASUREMENT)) {
       send_final_bmsi_packet(&final_results, current_working_mode, current_working_sub_mode); app_log("PERFORM Bmsi\r\n");
   }

   if((current_working_mode == WORKING_MODE_PERFORMANCE) && (current_working_sub_mode == WORKING_SUB_MODE_PERFORMANCE_REFERENCE)) {
       send_final_bmsi_packet(&final_results, current_working_mode, current_working_sub_mode); app_log("PERFORM-REF Bmsi\r\n");
   }
}

void fsrv_send_final_bmsi_packet_failed(void)
{
  if((current_working_mode == WORKING_MODE_PATIENT) && (current_working_sub_mode == WORKING_SUB_MODE_PATIENT_MEASUREMENT)) {
      send_final_bmsi_packet_failed(&final_results, current_working_mode, current_working_sub_mode); app_log("PATIENT Bmsi\r\n");
   }

   if((current_working_mode == WORKING_MODE_PATIENT) && (current_working_sub_mode == WORKING_SUB_MODE_PATIENT_REFERENCE)) {
       send_final_bmsi_packet_failed(&final_results, current_working_mode, current_working_sub_mode); app_log("PATIENT-REF Bmsi\r\n");
   }

   if((current_working_mode == WORKING_MODE_PERFORMANCE) && (current_working_sub_mode == WORKING_SUB_MODE_PERFORMANCE_MEASUREMENT)) {
       send_final_bmsi_packet_failed(&final_results, current_working_mode, current_working_sub_mode); app_log("PERFORM Bmsi\r\n");
   }

   if((current_working_mode == WORKING_MODE_PERFORMANCE) && (current_working_sub_mode == WORKING_SUB_MODE_PERFORMANCE_REFERENCE)) {
       send_final_bmsi_packet_failed(&final_results, current_working_mode, current_working_sub_mode); app_log("PERFORM-REF Bmsi\r\n");
   }
}

void fsrv_send_fail_session_packet(void)
{
  if((current_working_mode == WORKING_MODE_PATIENT) && (current_working_sub_mode == WORKING_SUB_MODE_PATIENT_MEASUREMENT)) {
      send_pic_or_ric_fail_session_packet(&measurement_session, current_working_mode, current_working_sub_mode); app_log("PATIENT FAIL SESSION\r\n");
   }

   if((current_working_mode == WORKING_MODE_PATIENT) && (current_working_sub_mode == WORKING_SUB_MODE_PATIENT_REFERENCE)) {
       send_pic_or_ric_fail_session_packet(&reference_session, current_working_mode, current_working_sub_mode); app_log("PATIENT-REF FAIL SESSION\r\n");
   }

   if((current_working_mode == WORKING_MODE_PERFORMANCE) && (current_working_sub_mode == WORKING_SUB_MODE_PERFORMANCE_MEASUREMENT)) {
       send_pic_or_ric_fail_session_packet(&measurement_session, current_working_mode, current_working_sub_mode); app_log("PERFORM FAIL SESSION\r\n");
   }

   if((current_working_mode == WORKING_MODE_PERFORMANCE) && (current_working_sub_mode == WORKING_SUB_MODE_PERFORMANCE_REFERENCE)) {
       send_pic_or_ric_fail_session_packet(&reference_session, current_working_mode, current_working_sub_mode); app_log("PERFORM-REF FAIL SESSION\r\n");
   }


}


bool measurement_session_return_result (void)
{
  if(measurement_session.session_failed == true) {
      return true;
  }
  else {
      return false;
  }
}

bool referense_session_return_result (void)
{
  if(reference_session.session_failed == true) {
      return true;
  }
  else {
      return false;
  }
}

/*
 * Диагностика связки TIMER1 -> PRS -> IADC.
 */
void debug_hardware_trinity(void)
{
   app_log("\r\n=== HARDWARE TRINITY DIAGNOSTIC ===\r\n");

   // --- 1. АНАЛИЗ ТАЙМЕРА (TIMER1) ---
   uint32_t timer_status = TIMER1->STATUS;
   uint32_t timer_cnt    = TIMER1->CNT;
   uint32_t timer_top    = TIMER1->TOP;

   app_log("1. TIMER1 Status:\r\n");
   app_log("   - Is Running:     %s\r\n", (timer_status & _TIMER_STATUS_RUNNING_MASK) ? "YES" : "NO");
   app_log("   - Counter (CNT):  %lu / %lu (TOP)\r\n", timer_cnt, timer_top);
   // --- 2. АНАЛИЗ PRS МОСТА ---
   app_log("2. PRS Configuration:\r\n");
   if (prs_allocated_ch >= 0) {
      app_log("   - Allocated Channel: %d (Async)\r\n", prs_allocated_ch);
      bool timer_ien_of = (TIMER1->IEN & _TIMER_IEN_OF_MASK) ? true : false;
      app_log("   - Timer OF Int Enabled: %s (Must be YES!)\r\n", timer_ien_of ? "YES" : "NO");
   }
   else {
      app_log("   - ERROR: PRS Channel not allocated!\r\n");
   }

   // --- 3. АНАЛИЗ АЦП (IADC0) ---
   uint32_t iadc_status   = IADC_getStatus(IADC0);
   uint8_t fifo_fill_cnt = IADC_getSingleFifoCnt(IADC0);
   bool single_done_ien   = (IADC0->IEN & _IADC_IEN_SINGLEDONE_MASK) ? true : false;

   app_log("3. IADC0 Status:\r\n");
   app_log("   - Single Queue Enabled:   %s\r\n", (iadc_status & IADC_STATUS_SINGLEQEN) ? "YES" : "NO");
   app_log("   - Single Queue Pending:   %s\r\n", (iadc_status & IADC_STATUS_SINGLEQUEUEPENDING) ? "YES" : "NO");
   app_log("   - Converting Now:         %s\r\n", (iadc_status & IADC_STATUS_CONVERTING) ? "YES" : "NO");
   app_log("   - FIFO Fill Count (Data in hardware):    %u\r\n", fifo_fill_cnt);
   app_log("   - SINGLEDONE Interrupt Enabled:          %s\r\n" , single_done_ien ? "YES" : "NO");
   app_log("   - Total Interrupts Handled:              %lu\r\n", adc_interrupt_counter);
   app_log("===================================\r\n\r\n");
}




//===============================================================
void fsrv_switch_to_reference_state(void)
{
  if ( WORKING_MODE_PATIENT == current_working_mode) {
     measurement_state_machine = MACHINE_STATE_PREPARE_FOR_PATIENT_REFERENCE;
  } else
  if ( WORKING_MODE_PERFORMANCE == current_working_mode) {
     measurement_state_machine = MACHINE_STATE_PREPARE_FOR_PERFORMANCE_REFERENCE;
  } else {
      app_log("MACHINE: Measurement REFERENCE session prepared - FAILED\r\n");
  }

}

void fsrv_switch_to_reference_submode(void)
{
  if ( WORKING_MODE_PATIENT == current_working_mode) {
      current_working_sub_mode = WORKING_SUB_MODE_PATIENT_REFERENCE;
  } else
  if ( WORKING_MODE_PERFORMANCE == current_working_mode) {
      current_working_sub_mode = WORKING_SUB_MODE_PERFORMANCE_REFERENCE;
  }

}


//===============================================================
bool fsrv_is_performance_mode_state(void)
{
  return (bool)(WORKING_MODE_PERFORMANCE == current_working_mode);
}

//===============================================================

/**
 * @brief Старт сессии измерения Patient или Performance с полной очисткой структур.
 */
void start_selected_measurement_session(working_mode_t mode)
{
    // При старте сессии структуры полностью обнуляются
    memset(&measurement_session, 0, sizeof(measurement_session_t));
    memset(&reference_session, 0, sizeof(measurement_session_t));
    memset(&final_results, 0, sizeof(final_bmsi_result_t));

    current_working_mode = mode;
    is_currnet_measurement_paused = false;

    if (mode == WORKING_MODE_PATIENT) {
        current_working_sub_mode = WORKING_SUB_MODE_PATIENT_MEASUREMENT;
        measurement_state_machine = MACHNE_STATE_PATIENT_MEASUREMENT_START;
        app_log("MACHINE: PATIENT session started, sub_mode - measurement\r\n");
    }
    else if (mode == WORKING_MODE_PERFORMANCE) {
        current_working_sub_mode = WORKING_SUB_MODE_PERFORMANCE_MEASUREMENT;
        measurement_state_machine = MACHNE_STATE_PERFORMANCE_MEASUREMENT_START;
        app_log("MACHINE: PERFORMANCE session started, sub_mode - measurement\r\n");
    }
}

/**
 * @brief Переход между подрежимами работы. Структуры НЕ обнуляются.
 */
void transition_to_selected_working_sub_mode(working_sub_mode_t next_sub)
{
    // Защита от одновременного выполнения или некорректного перехода
    if (current_working_mode == WORKING_MODE_NONE) return;

    current_working_sub_mode = next_sub;
    is_currnet_measurement_paused = false;

    if (next_sub == WORKING_SUB_MODE_PATIENT_REFERENCE) {
        measurement_state_machine = MACHNE_STATE_PATIENT_REFERENCE_START;
        app_log("MACHINE: Switched to PATIENT->Reference\r\n");
    }
    else if (next_sub == WORKING_SUB_MODE_PERFORMANCE_REFERENCE) {
        measurement_state_machine = MACHNE_STATE_PERFORMANCE_REFERENCE_START;
        app_log("MACHINE: Switched to PERFORMANCE->Reference\r\n");
    }
}

// Поставить измерение на паузу
void pause_current_measurement(void)
{
    if (measurement_state_machine != MACHNE_STATE_IDLE && !is_currnet_measurement_paused) {
        // Аппаратно выключаем таймер — PRS-импульсы перестают поступать, АЦП засыпает
        TIMER_Enable(TIMER1, false);
        is_currnet_measurement_paused = true;
        app_log("MACHINE: Measurement process PAUSED.\r\n");
    }
}

// Возобновить измерение из паузы
void resume_current_measurement(void)
{
    if (is_currnet_measurement_paused) {
        TIMER_IntClear(TIMER1, TIMER_IF_OF);
        is_currnet_measurement_paused = false;

        // Если машина находилась в фазе ожидания удара — возвращаем таймер в строй
        if (measurement_state_machine == MACHNE_STATE_PATIENT_MEASUREMENT_WAIT_ADC ||
            measurement_state_machine == MACHNE_STATE_PATIENT_REFERENCE_WAIT_ADC ||
            measurement_state_machine == MACHNE_STATE_PERFORMANCE_MEASUREMENT_WAIT_ADC ||
            measurement_state_machine == MACHNE_STATE_PERFORMANCE_REFERENCE_WAIT_ADC)
        {
            TIMER_Enable(TIMER1, true);
        }
        app_log("MACHINE: Measurement process RESUMED.\r\n");
    }
}

void stop_measurement_session(void)
{
    current_working_mode = WORKING_MODE_NONE;
    current_working_sub_mode = WORKING_SUB_MODE_NONE;
    measurement_state_machine = MACHNE_STATE_IDLE; // Разблокирует фоновый опрос стилуса
    is_currnet_measurement_paused = false;

    // Сброс программных переменных захвата
    current_active_channel = ADC_CHANNEL_SIGNAL;
    adc_current_state = ADC_STATE_IDLE;
    active_session = NULL;
    write_ptr              = 0;
    peak_value             = 0;
    peak_buffer_idx        = 0;
    post_peak_counter      = 0;
    adc_impact_start_index = 0;
    adc_interrupt_counter  = 0;

    TIMER_Enable(TIMER1, false);

    //memset(&measurement_session, 0, sizeof(measurement_session_t));
    //memset(&reference_session, 0, sizeof(measurement_session_t));
    memset(&final_results, 0, sizeof(final_bmsi_result_t));

    app_log("MACHINE: Session stopped. ADC hardware fully reset for idle checks.\r\n");
}


void prepare_for_working_mode(working_mode_t mode)
{
  memset(&measurement_session, 0, sizeof(measurement_session_t));
  memset(&reference_session, 0, sizeof(measurement_session_t));
  memset(&final_results, 0, sizeof(final_bmsi_result_t));

  is_currnet_measurement_paused = false; //???????????????

  if(mode == WORKING_MODE_PERFORMANCE) {
      measurement_session.remain_counter = device_params.advanced_params.measure_params.num_of_valid_performance_measurements;
      measurement_session.target_valid = device_params.advanced_params.measure_params.num_of_valid_performance_measurements;
      reference_session.remain_counter = device_params.advanced_params.measure_params.num_of_valid_performance_references;
      reference_session.target_valid = device_params.advanced_params.measure_params.num_of_valid_performance_references;
      //на всякий случай продублируем режимы работы
      current_working_mode = WORKING_MODE_PERFORMANCE;
      current_working_sub_mode = WORKING_SUB_MODE_PERFORMANCE_MEASUREMENT;
  }
  if(mode == WORKING_MODE_PATIENT) {
      measurement_session.remain_counter = device_params.advanced_params.measure_params.num_of_valid_patient_measurements;
      measurement_session.target_valid = device_params.advanced_params.measure_params.num_of_valid_patient_measurements;
      reference_session.remain_counter = device_params.advanced_params.measure_params.num_of_valid_patient_references;
      reference_session.target_valid = device_params.advanced_params.measure_params.num_of_valid_patient_references;
      //на всякий случай продублируем режимы работы
      current_working_mode = WORKING_MODE_PATIENT;
      current_working_sub_mode = WORKING_SUB_MODE_PATIENT_MEASUREMENT;
  }
}

void clear_reference_session (working_mode_t mode)
{
  if(mode == WORKING_MODE_PERFORMANCE) {
      memset(&reference_session, 0, sizeof(measurement_session_t));
      reference_session.remain_counter = device_params.advanced_params.measure_params.num_of_valid_performance_references;
      reference_session.target_valid = device_params.advanced_params.measure_params.num_of_valid_performance_references;
      //на всякий случай продублируем режимы работы
      current_working_mode = WORKING_MODE_PERFORMANCE;
      current_working_sub_mode = WORKING_SUB_MODE_PERFORMANCE_REFERENCE;
  }
  if(mode == WORKING_MODE_PATIENT) {
      memset(&reference_session, 0, sizeof(measurement_session_t));
      reference_session.remain_counter = device_params.advanced_params.measure_params.num_of_valid_patient_references;
      reference_session.target_valid = device_params.advanced_params.measure_params.num_of_valid_patient_references;
      //на всякий случай продублируем режимы работы
      current_working_mode = WORKING_MODE_PATIENT;
      current_working_sub_mode = WORKING_SUB_MODE_PATIENT_REFERENCE;
  }
}


// Patien session start
void start_patient_session(void)
{
  app_log("MACHINE: Started patient measurement\r\n");
  measurement_state_machine = MACHINE_STATE_PREPARE_FOR_PATIENT;
  current_working_mode = WORKING_MODE_PATIENT;
  current_working_sub_mode = WORKING_SUB_MODE_PATIENT_MEASUREMENT;
  if(is_currnet_measurement_paused == true) is_currnet_measurement_paused = false;
}

// Performance session start
void start_performance_session(void)
{
  app_log("MACHINE: Started performance check\r\n");
  measurement_state_machine = MACHINE_STATE_PREPARE_FOR_PERFORMANCE;
  current_working_mode = WORKING_MODE_PERFORMANCE;
  current_working_sub_mode = WORKING_SUB_MODE_PERFORMANCE_MEASUREMENT;
  if(is_currnet_measurement_paused == true) is_currnet_measurement_paused = false;
}

void mark_reference_session_as_failed(void)
{
  reference_session.session_failed = true;
}

void measurement_process_loop(void)
{
    if (is_currnet_measurement_paused) {
        return;
    }

    switch (measurement_state_machine)
    {
        case MACHNE_STATE_IDLE:
        break;

        case MACHNE_STATE_TIP_ID_START:
          safe_timer = sl_sleeptimer_tick_to_ms(sl_sleeptimer_get_tick_count());
          fsrv_DS_SetTipIdStat(TIP_ID_WAITING);
          aukh_Post_UI_Event(AU_PATIENT_MENU_VALIDATING);
          measurement_state_machine =  MACHNE_STATE_TIP_ID_VALIDATE;
          break;

        case MACHNE_STATE_TIP_ID_VALIDATE:
          uint8_t *byte_ptr = (uint8_t *)&rx_data_buffer;
          byte_ptr += sizeof(packet_head_t);
          tip_validate(byte_ptr);
          measurement_state_machine =  MACHNE_STATE_TIP_ID_VALIDATING_DISPLAY_RESULT;
        break;

        case MACHNE_STATE_TIP_ID_VALIDATING_DISPLAY_RESULT:

          if((sl_sleeptimer_tick_to_ms(sl_sleeptimer_get_tick_count()) - safe_timer ) > 2000) {


              aukh_Post_UI_Event(AU_PATIENT_MENU_VALIDATING_RESULT);

              if (fsrv_DS_GetTipIdStatus() == TIP_ID_VALID) {
                  measurement_state_machine =  MACHNE_STATE_TIP_ID_VALIDATING_END;
                  safe_timer = sl_sleeptimer_tick_to_ms(sl_sleeptimer_get_tick_count());
              }
              else {
                  measurement_state_machine =  MACHNE_STATE_IDLE;
              }

          }
        break;

        case MACHNE_STATE_TIP_ID_VALIDATING_END:
          if((sl_sleeptimer_tick_to_ms(sl_sleeptimer_get_tick_count()) - safe_timer ) > 2000) {
              aukh_Post_UI_Event(AU_PATIENT_MENU_START);
              measurement_state_machine =  MACHNE_STATE_IDLE;
          }
        break;

        // =====================================================================
        // =============== РЕЖИМ PERFORMANCE: ПОДРЕЖИМ MEASUREMENT =============
        // =====================================================================
        case MACHINE_STATE_PREPARE_FOR_PERFORMANCE:
          // если заказали перформанс, то очищаем все сессии и устанавливаем грацы измерений
          // в функции
          prepare_for_working_mode(current_working_mode);
          historical_record_write_header();
          measurement_state_machine = MACHNE_STATE_PERFORMANCE_MEASUREMENT_START;
        break;

        case MACHNE_STATE_PERFORMANCE_MEASUREMENT_START:
            /*if (!check_device_connected((adc_vref_source_t)device_params.advanced_params.measure_params.adc_vref_source)) {
                break;
            }
            start_impact_capture(device_params.advanced_params.measure_params.adc_taking_period, (adc_vref_source_t)device_params.advanced_params.measure_params.adc_vref_source);*/
            start_capture_adc_scan();
            measurement_state_machine = MACHNE_STATE_PERFORMANCE_MEASUREMENT_WAIT_ADC;
        break;

        case MACHNE_STATE_PERFORMANCE_MEASUREMENT_WAIT_ADC:
            if (is_adc_capture_done()) {
                adc_current_state = ADC_STATE_IDLE;
                measurement_state_machine = MACHNE_STATE_PERFORMANCE_MEASUREMENT_PROCESS;
                break;
            }
        break;

        case MACHNE_STATE_PERFORMANCE_MEASUREMENT_PROCESS:
            // На чистом Performance датчик наконечника НЕ маркируется
            process_single_indent((uint16_t*)adc_unified_buffer, (uint16_t*)&adc_impact_start_index, &measurement_session, current_working_mode, current_working_sub_mode);
            fs_sd_historical_records_write_data(tx_data_buffer, 18, true, device_params.advanced_params.has_raw_data, adc_unified_buffer, false, false, false);
            // проверяем не завалилась ли сессия
            /*if(measurement_session.session_failed == true) {
                // если сессия измерений запоролась рисуем красный крест в функции process_single_indent
                // и прыгаем в это состояние
                measurement_state_machine = MACHNE_STATE_PERFORMANCE_MEASUREMENT_PROCESS_FAILED;
            }
            else {
                // если все ок прыгаем на ожидание подтверждения приема пакета с данными от станции на уровне low-level протокола
              measurement_state_machine = MACHNE_STATE_PERFORMANCE_WAIT_ONE_MEASURE_PACKET_SENT;
            }*/
            measurement_state_machine = MACHNE_STATE_PERFORMANCE_WAIT_ONE_MEASURE_PACKET_SENT;

        break;

        case MACHNE_STATE_PERFORMANCE_MEASUREMENT_PROCESS_FAILED:
            // останавливаем измерения(сбразываем структуру с финальным результатом и останавливаем таймер который пинает АЦП в функции stop_measurement_session())
            // взводим таймер для того чтобы сделать паузу между кружочком с крестом и картинкой session failed
            //stop_measurement_session();
            safe_timer = sl_sleeptimer_tick_to_ms(sl_sleeptimer_get_tick_count());
            measurement_state_machine = MACHNE_STATE_PERFORMANCE_MEASUREMENT_PROCESS_FAILED_WAIT_TIMEOUT;
        break;

        case MACHNE_STATE_PERFORMANCE_MEASUREMENT_PROCESS_FAILED_WAIT_TIMEOUT:
          // когда отработает таймаут показываем картинку session failed
          // и прыгаем в состояние measurement_state_machine = MACHNE_STATE_IDLE;
          if((sl_sleeptimer_tick_to_ms(sl_sleeptimer_get_tick_count()) - safe_timer ) > 2000) {
              aukh_Post_UI_Event(AU_PERFORMANCE_MENU_CHECK_END);
            //  measurement_state_machine = MACHNE_STATE_IDLE;
              measurement_state_machine = MACHNE_STATE_PERFORMANCE_MEASUREMENT_PROCESS_FAILED_PRE_STATE_IDLE;
          }

        break;

        case MACHNE_STATE_PERFORMANCE_MEASUREMENT_PROCESS_FAILED_PRE_STATE_IDLE:
          stop_measurement_session();
          measurement_state_machine = MACHNE_STATE_IDLE;
        break;

        case MACHNE_STATE_PERFORMANCE_WAIT_ONE_MEASURE_PACKET_SENT:
          if(device_operation_control.data_tranfer_control.status.transmitting_to_station_in_progress == ALS_TRUE) {
              measurement_state_machine = MACHNE_STATE_PERFORMANCE_WAIT_ONE_MEASURE_PACKET_SENT;
              #warning "ATTENTION ADD TIMEOUT 11!"
              break;
          }
          else {
                if(measurement_session.session_failed == true) {
                // если сессия измерений запоролась рисуем красный крест в функции process_single_indent
                // и прыгаем в это состояние
                measurement_state_machine = MACHNE_STATE_PERFORMANCE_MEASUREMENT_PROCESS_FAILED;
                aukh_Post_UI_Event(AU_PERFORMANCE_MENU_FINISHED);

            }
            else {
                // если все ок прыгаем на ожидание подтверждения приема пакета с данными от станции на уровне low-level протокола
              //measurement_state_machine = MACHNE_STATE_PERFORMANCE_WAIT_ONE_MEASURE_PACKET_SENT;
                measurement_state_machine = MACHNE_STATE_PERFORMANCE_MEASUREMENT_CHECK_COUNT;
            }
            // measurement_state_machine = MACHNE_STATE_PERFORMANCE_MEASUREMENT_CHECK_COUNT;
            break;
          }
        break;

        case MACHNE_STATE_PERFORMANCE_MEASUREMENT_CHECK_COUNT:
          // Проверяем: набрали ли требуемое количество уколов для ТЕКУЩЕЙ сессии
          if (measurement_session.valid_counter >= measurement_session.target_valid) {
              filter_outliers(&measurement_session);
              // Проверяем, осталось ли достаточно валидных уколов после фильтрации
              if (measurement_session.valid_counter >= measurement_session.target_valid) {
                 app_log("MATH: Performance Measurements Complete! Valid indents: %d\r\n", measurement_session.valid_counter);
                 // Принудительно гасим АЦП на время паузы, чтобы исключить ложные срабатывания
                 adc_current_state = ADC_STATE_IDLE;
                 measurement_state_machine = MACHNE_STATE_PERFORMANCE_MEASUREMENT_SEND_BMS_PACKET;
              }
              else {
               // После отбраковки уколов стало меньше — возвращаемся добирать чистые измерения
               app_log("MATH: Outliers removed. Valid count dropped to %d. Capturing more...\r\n", measurement_session.valid_counter);
               safe_timer = sl_sleeptimer_tick_to_ms(sl_sleeptimer_get_tick_count());
               //measurement_state_machine = MACHNE_STATE_PERFORMANCE_MEASUREMENT_START;
               measurement_state_machine = MACHNE_STATE_PERFORMANCE_MEASUREMENT_SAFTY_BARRIER;
              }
          } else {
             // Еще не набрали первичный пул — продолжаем захват
             safe_timer = sl_sleeptimer_tick_to_ms(sl_sleeptimer_get_tick_count());
             //measurement_state_machine = MACHNE_STATE_PERFORMANCE_MEASUREMENT_START;
             measurement_state_machine = MACHNE_STATE_PERFORMANCE_MEASUREMENT_SAFTY_BARRIER;
          }
        break;

        case MACHNE_STATE_PERFORMANCE_MEASUREMENT_SAFTY_BARRIER:
          if((sl_sleeptimer_tick_to_ms(sl_sleeptimer_get_tick_count()) - safe_timer ) > SAFE_TIMER_MS) {
              measurement_state_machine = MACHNE_STATE_PERFORMANCE_MEASUREMENT_START;
          }
        break;

        case MACHNE_STATE_PERFORMANCE_MEASUREMENT_SEND_BMS_PACKET:
          send_intermediate_bms_packet(&measurement_session, current_working_mode, current_working_sub_mode);
          fs_sd_historical_records_write_data(tx_data_buffer, 19, false, device_params.advanced_params.has_raw_data, adc_unified_buffer, true, false, false);
          measurement_state_machine = MACHNE_STATE_PERFORMANCE_MEASUREMENT_WAIT_BMS_PACKET_SENT;
        break;

        case MACHNE_STATE_PERFORMANCE_MEASUREMENT_WAIT_BMS_PACKET_SENT:
          if(device_operation_control.data_tranfer_control.status.transmitting_to_station_in_progress == ALS_TRUE) {
              measurement_state_machine = MACHNE_STATE_PERFORMANCE_MEASUREMENT_WAIT_BMS_PACKET_SENT;
              #warning "ATTENTION ADD TIMEOUT 22!"
              break;
          }
          else {
              strm_timer = sl_sleeptimer_tick_to_ms(sl_sleeptimer_get_tick_count());
              measurement_state_machine = MACHNE_STATE_WAIT_PERFORMANCE_MEASUREMENT_TO_SEND_STRM;
              aukh_Post_UI_Event(AU_PERFORMANCE_MENU_FINISHED);
              break;
          }
        break;

        case MACHNE_STATE_WAIT_PERFORMANCE_MEASUREMENT_TO_SEND_STRM:
          if((sl_sleeptimer_tick_to_ms(sl_sleeptimer_get_tick_count()) - strm_timer ) > 2000) {
              ///send_strm_packet(); /* @ToDo  Comented out by UP The function send_strm_packet() is called by UI interface */
              measurement_state_machine = MACHNE_STATE_PERFORMANCE_MEASUREMENT_WAIT_BUTTON_PRESSED;
              aukh_Post_UI_Event(AU_REFERENCE_MENU_START);
          }
        break;

        case MACHNE_STATE_PERFORMANCE_MEASUREMENT_WAIT_BUTTON_PRESSED:
          // ждем пока нажмут кнопку
        break;


        case MACHNE_STATE_PERFORMANCE_MEASUREMENT_DONE:
        break;

        // =====================================================================
        // === РЕЖИМ PERFORMANCE: ПОДРЕЖИМ PERFORMANCE REFERENCE ===============
        // =====================================================================

        case MACHINE_STATE_PREPARE_FOR_PERFORMANCE_REFERENCE:
            app_log("MACHINE: PERFORMANCE session started, sub_mode - reference\r\n");
            clear_reference_session(current_working_mode);
            //current_working_sub_mode = WORKING_SUB_MODE_PERFORMANCE_REFERENCE;
            measurement_state_machine = MACHNE_STATE_PERFORMANCE_REFERENCE_START;
        break;

        case MACHNE_STATE_PERFORMANCE_REFERENCE_START:
            /*if (!check_device_connected((adc_vref_source_t)device_params.advanced_params.measure_params.adc_vref_source)) {
                break;
            }
            start_impact_capture(device_params.advanced_params.measure_params.adc_taking_period, (adc_vref_source_t)device_params.advanced_params.measure_params.adc_vref_source);*/
            start_capture_adc_scan();
            measurement_state_machine = MACHNE_STATE_PERFORMANCE_REFERENCE_WAIT_ADC;
        break;

        case MACHNE_STATE_PERFORMANCE_REFERENCE_WAIT_ADC:
            if (is_adc_capture_done()) {
                adc_current_state = ADC_STATE_IDLE;
                measurement_state_machine = MACHNE_STATE_PERFORMANCE_REFERENCE_PROCESS;
                break;
            }
        break;

        case MACHNE_STATE_PERFORMANCE_REFERENCE_PROCESS:
            // На чистом Performance датчик наконечника НЕ маркируется
            process_single_indent((uint16_t*)adc_unified_buffer, (uint16_t*)&adc_impact_start_index, &reference_session, current_working_mode, current_working_sub_mode);
            //fs_sd_historical_records_write_at_offset(const void *data_ptr, uint32_t size, uint8_t raw_data_in_background, uint8_t raw_data_setting, uint16_t *raw_data_ptr);
            fs_sd_historical_records_write_data(tx_data_buffer, 18, true, device_params.advanced_params.has_raw_data, adc_unified_buffer, false, false, false);
            // проверяем не завалилась ли сессия
            /*if(reference_session.session_failed == true) {
                measurement_state_machine = MACHNE_STATE_PERFORMANCE_REFERENCE_PROCESS_FAILED;
            }
            else {
              measurement_state_machine = MACHNE_STATE_PERFORMANCE_WAIT_ONE_REFERENCE_PACKET_SENT;
            }*/
            measurement_state_machine = MACHNE_STATE_PERFORMANCE_WAIT_ONE_REFERENCE_PACKET_SENT;
        break;

        case MACHNE_STATE_PERFORMANCE_REFERENCE_PROCESS_FAILED:
            stop_measurement_session();
            measurement_state_machine = MACHNE_STATE_IDLE;
        break;

//        case MACHNE_STATE_PERFORMANCE_REFERENCE_PROCESS_FAILED_PRE_STATE_IDLE:
          // stop_measurement_session();
//          measurement_state_machine = MACHNE_STATE_PERFORMANCE_REFERENCE_PROCESS_FAILED;
//        break;

        case MACHNE_STATE_PERFORMANCE_WAIT_ONE_REFERENCE_PACKET_SENT:
          if(device_operation_control.data_tranfer_control.status.transmitting_to_station_in_progress == ALS_TRUE) {
              measurement_state_machine = MACHNE_STATE_PERFORMANCE_WAIT_ONE_REFERENCE_PACKET_SENT;
              #warning "ATTENTION ADD TIMEOUT 11!"
              break;
          }
          else {
              // проверяем не завалилась ли сессия
              if(reference_session.session_failed == true) {
                  measurement_state_machine = MACHNE_STATE_PERFORMANCE_REFERENCE_PROCESS_FAILED;
                  aukh_Post_UI_Event(AU_REFERENCE_PERFORM_MENU_REPEAT_START);
                  break;
              }
              else {
                  measurement_state_machine = MACHNE_STATE_PERFORMANCE_REFERENCE_CHECK_COUNT;
                //measurement_state_machine = MACHNE_STATE_PERFORMANCE_WAIT_ONE_REFERENCE_PACKET_SENT;
              }
              //measurement_state_machine = MACHNE_STATE_PERFORMANCE_REFERENCE_CHECK_COUNT;
              break;
          }
        break;

        case MACHNE_STATE_PERFORMANCE_REFERENCE_CHECK_COUNT:
          // Проверяем: набрали ли требуемое количество уколов для ТЕКУЩЕЙ сессии
          if (reference_session.valid_counter >= measurement_session.target_valid) {
              filter_outliers(&reference_session);
              // Проверяем, осталось ли достаточно валидных уколов после фильтрации
              if (reference_session.valid_counter >= reference_session.target_valid) {
                 app_log("MATH: Performance References Complete! Valid indents: %d\r\n", reference_session.valid_counter);
                 // Принудительно гасим АЦП на время паузы, чтобы исключить ложные срабатывания
                 adc_current_state = ADC_STATE_IDLE;
                 measurement_state_machine = MACHNE_STATE_PERFORMANCE_REFERENCE_SEND_BMS_PACKET;
              }
              else {
               // После отбраковки уколов стало меньше — возвращаемся добирать чистые измерения
               app_log("MATH: Outliers removed. Valid count dropped to %d. Capturing more...\r\n", reference_session.valid_counter);
                measurement_state_machine = MACHNE_STATE_PERFORMANCE_REFERENCE_SAFTY_BARRIER;
              }
          } else {
             // Еще не набрали первичный пул — продолжаем захват
             measurement_state_machine = MACHNE_STATE_PERFORMANCE_REFERENCE_SAFTY_BARRIER;
          }
        break;

        case MACHNE_STATE_PERFORMANCE_REFERENCE_SAFTY_BARRIER:
          if((sl_sleeptimer_tick_to_ms(sl_sleeptimer_get_tick_count()) - safe_timer ) > 350) {
              measurement_state_machine = MACHNE_STATE_PERFORMANCE_REFERENCE_START;
          }
        break;

        case MACHNE_STATE_PERFORMANCE_REFERENCE_SEND_BMS_PACKET:
          send_intermediate_bms_packet(&reference_session, current_working_mode, current_working_sub_mode);
          fs_sd_historical_records_write_data(tx_data_buffer, 19, false, device_params.advanced_params.has_raw_data, adc_unified_buffer, false, true, false);
          measurement_state_machine = MACHNE_STATE_PERFORMANCE_REFERENCE_WAIT_BMS_PACKET_SENT;
        break;

        case MACHNE_STATE_PERFORMANCE_REFERENCE_WAIT_BMS_PACKET_SENT:
          if(device_operation_control.data_tranfer_control.status.transmitting_to_station_in_progress == ALS_TRUE) {
              measurement_state_machine = MACHNE_STATE_PERFORMANCE_REFERENCE_WAIT_BMS_PACKET_SENT;
              #warning "ATTENTION ADD TIMEOUT 22!"
              break;
          }
          else {
              calculate_final_bmsi(current_working_mode);
              if(final_results.is_unstable){
                  aukh_Post_UI_Event(AU_REFERENCE_PERFORM_MENU_REPEAT_START);
                  measurement_state_machine = MACHNE_STATE_PERFORMANCE_REFERENCE_START;
                  break;
              }

              strm_timer = sl_sleeptimer_tick_to_ms(sl_sleeptimer_get_tick_count());
              measurement_state_machine = MACHNE_STATE_WAIT_PERFORMANCE_REFERENCE_TO_SEND_STRM;
              fs_sd_write_performance_test_date();
              device_operation_control.performance_required = false;
              aukh_Post_UI_Event(AU_REFERENCE_MENU_FINISHED);
              break;
          }
        break;

        case MACHNE_STATE_WAIT_PERFORMANCE_REFERENCE_TO_SEND_STRM:
          if((sl_sleeptimer_tick_to_ms(sl_sleeptimer_get_tick_count()) - strm_timer ) > 2000) {
              ///send_strm_packet(); /* @ToDo  Comented out by UP The function send_strm_packet() is called by UI interface */
              measurement_state_machine = MACHNE_STATE_PERFORMANCE_CALCULATE_BMSI;
          }
        break;

        case MACHNE_STATE_PERFORMANCE_CALCULATE_BMSI:
          //app_log("MATH: Both sessions completed successfully. Calculating final BMSi...\r\n");
          //calculate_final_bmsi();
          // Отправка результатов по BLE...
          strm_timer = sl_sleeptimer_tick_to_ms(sl_sleeptimer_get_tick_count());
          measurement_state_machine = MACHNE_STATE_PERFORMANCE_WAIT_SEND_FINAL_BMSI;
        break;

        case MACHNE_STATE_PERFORMANCE_WAIT_SEND_FINAL_BMSI:
          if((sl_sleeptimer_tick_to_ms(sl_sleeptimer_get_tick_count()) - strm_timer ) > 20) {
              measurement_state_machine = MACHNE_STATE_PERFORMANCE_SEND_FINAL_BMSI;
              aukh_Post_UI_Event(AU_PERFORMANCE_MENU_CHECK_END);
          }
        break;


        case MACHNE_STATE_PERFORMANCE_SEND_FINAL_BMSI:
          if(device_operation_control.data_tranfer_control.status.transmitting_to_station_in_progress == ALS_TRUE) {
              measurement_state_machine = MACHNE_STATE_PERFORMANCE_SEND_FINAL_BMSI;
              #warning "ATTENTION ADD TIMEOUT 333!"
              break;
          }
          else {
             // send_final_bmsi_packet(&final_results, current_sub_mode); /* @ToDo  Comented out by UP The function send_final_bmsi_packet() is called by UI interface */
              measurement_state_machine = MACHNE_STATE_IDLE;
              break;
          }
        break;



        case MACHNE_STATE_PERFORMANCE_REFERENCE_WAIT_BUTTON_PRESSED:
          // ждем пока нажмут кнопку
        break;


        case MACHNE_STATE_PERFORMANCE_REFERENCE_DONE:
        break;

        // =====================================================================

        // =====================================================================
        // =============== РЕЖИМ PATIENT: ПОДРЕЖИМ MEASUREMENT =============
        // =====================================================================
        case MACHINE_STATE_PREPARE_FOR_PATIENT:
          // если заказали перформанс, то очищаем все сессии и устанавливаем грацы измерений
          // в функции
          prepare_for_working_mode(current_working_mode);
          historical_record_write_header();
          measurement_state_machine = MACHNE_STATE_PATIENT_MEASUREMENT_START;
        break;

        case MACHNE_STATE_PATIENT_MEASUREMENT_START:
            /*if (!check_device_connected((adc_vref_source_t)device_params.advanced_params.measure_params.adc_vref_source)) {
                break;
            }
            start_impact_capture(device_params.advanced_params.measure_params.adc_taking_period, (adc_vref_source_t)device_params.advanced_params.measure_params.adc_vref_source);*/
            start_capture_adc_scan();
            measurement_state_machine = MACHNE_STATE_PATIENT_MEASUREMENT_WAIT_ADC;
        break;

        case MACHNE_STATE_PATIENT_MEASUREMENT_WAIT_ADC:
            if (is_adc_capture_done()) {
                adc_current_state = ADC_STATE_IDLE;
                measurement_state_machine = MACHNE_STATE_PATIENT_MEASUREMENT_PROCESS;
                break;
            }
        break;

        case MACHNE_STATE_PATIENT_MEASUREMENT_PROCESS:
          // Вызываем гашение наконечника на SD-карте только один раз за всю текущую сессию
            if (!is_tip_marked_used_in_session) {
                fs_sd_mark_last_tip_used_fast();
                is_tip_marked_used_in_session = true; // Защелкиваем флаг, теперь сюда заходить не будем
                app_log("ADC: Tip marked as USED for the current measurement session\r\n");
            }
            process_single_indent((uint16_t*)adc_unified_buffer, (uint16_t*)&adc_impact_start_index, &measurement_session, current_working_mode, current_working_sub_mode);
            fs_sd_historical_records_write_data(tx_data_buffer, 18, true, device_params.advanced_params.has_raw_data, adc_unified_buffer, false, false, false);
            // проверяем не завалилась ли сессия
            /*if(measurement_session.session_failed == true) {
                // если сессия измерений запоролась рисуем красный крест в функции process_single_indent
                // и прыгаем в это состояние
                measurement_state_machine = MACHNE_STATE_PATIENT_MEASUREMENT_PROCESS_FAILED;
            }
            else {
                // если все ок прыгаем на ожидание подтверждения приема пакета с данными от станции на уровне low-level протокола
              measurement_state_machine = MACHNE_STATE_PATIENT_WAIT_ONE_MEASURE_PACKET_SENT;
            }*/
            measurement_state_machine = MACHNE_STATE_PATIENT_WAIT_ONE_MEASURE_PACKET_SENT;
        break;

        case MACHNE_STATE_PATIENT_MEASUREMENT_PROCESS_FAILED:
            // останавливаем измерения(сбразываем структуру с финальным результатом и останавливаем таймер который пинает АЦП в функции stop_measurement_session())
            // взводим таймер для того чтобы сделать паузу между кружочком с крестом и картинкой session failed
            //stop_measurement_session();
            safe_timer = sl_sleeptimer_tick_to_ms(sl_sleeptimer_get_tick_count());
            measurement_state_machine = MACHNE_STATE_PATIENT_MEASUREMENT_PROCESS_FAILED_WAIT_TIMEOUT;
        break;

        case MACHNE_STATE_PATIENT_MEASUREMENT_PROCESS_FAILED_WAIT_TIMEOUT:
          // когда отработает таймаут показываем картинку session failed
          // и прыгаем в состояние measurement_state_machine = MACHNE_STATE_IDLE;
          if((sl_sleeptimer_tick_to_ms(sl_sleeptimer_get_tick_count()) - safe_timer ) > 2000) {
              aukh_Post_UI_Event(AU_PATIENT_MENU_FAILED);
              measurement_state_machine = MACHNE_STATE_PATIENT_MEASUREMENT_PROCESS_FAILED_PRE_STATE_IDLE;
              //measurement_state_machine = MACHNE_STATE_IDLE;
          }
        break;

        case MACHNE_STATE_PATIENT_MEASUREMENT_PROCESS_FAILED_PRE_STATE_IDLE:
          stop_measurement_session();
          measurement_state_machine = MACHNE_STATE_IDLE;
        break;

        case MACHNE_STATE_PATIENT_WAIT_ONE_MEASURE_PACKET_SENT:
          if(device_operation_control.data_tranfer_control.status.transmitting_to_station_in_progress == ALS_TRUE) {
              measurement_state_machine = MACHNE_STATE_PATIENT_WAIT_ONE_MEASURE_PACKET_SENT;
              #warning "ATTENTION ADD TIMEOUT 11!"
              break;
          }
          else {
              if(measurement_session.session_failed == true) {
                  // если сессия измерений запоролась рисуем красный крест в функции process_single_indent
                  // и прыгаем в это состояние
                  measurement_state_machine = MACHNE_STATE_PATIENT_MEASUREMENT_PROCESS_FAILED;
                  aukh_Post_UI_Event(AU_PATIENT_MENU_FINISHED);
                  break;
              }
              else {
                  // если все ок прыгаем на ожидание подтверждения приема пакета с данными от станции на уровне low-level протокола
                //measurement_state_machine = MACHNE_STATE_PATIENT_WAIT_ONE_MEASURE_PACKET_SENT;
                  measurement_state_machine = MACHNE_STATE_PATIENT_MEASUREMENT_CHECK_COUNT;
                  break;
              }
              //measurement_state_machine = MACHNE_STATE_PATIENT_MEASUREMENT_CHECK_COUNT;
              break;
          }
        break;

        case MACHNE_STATE_PATIENT_MEASUREMENT_CHECK_COUNT:
          // Проверяем: набрали ли требуемое количество уколов для ТЕКУЩЕЙ сессии
          if (measurement_session.valid_counter >= measurement_session.target_valid) {
              filter_outliers(&measurement_session);
              // Проверяем, осталось ли достаточно валидных уколов после фильтрации
              if (measurement_session.valid_counter >= measurement_session.target_valid) {
                 app_log("MATH: Performance Measurements Complete! Valid indents: %d\r\n", measurement_session.valid_counter);
                 // Принудительно гасим АЦП на время паузы, чтобы исключить ложные срабатывания
                 adc_current_state = ADC_STATE_IDLE;
                 measurement_state_machine = MACHNE_STATE_PATIENT_MEASUREMENT_SEND_BMS_PACKET;
              }
              else {
               // После отбраковки уколов стало меньше — возвращаемся добирать чистые измерения
               app_log("MATH: Outliers removed. Valid count dropped to %d. Capturing more...\r\n", measurement_session.valid_counter);
               safe_timer = sl_sleeptimer_tick_to_ms(sl_sleeptimer_get_tick_count());
               //measurement_state_machine = MACHNE_STATE_PERFORMANCE_MEASUREMENT_START;
               measurement_state_machine = MACHNE_STATE_PATIENT_MEASUREMENT_SAFTY_BARRIER;
              }
          } else {
             // Еще не набрали первичный пул — продолжаем захват
             safe_timer = sl_sleeptimer_tick_to_ms(sl_sleeptimer_get_tick_count());
             //measurement_state_machine = MACHNE_STATE_PERFORMANCE_MEASUREMENT_START;
             measurement_state_machine = MACHNE_STATE_PATIENT_MEASUREMENT_SAFTY_BARRIER;
          }
        break;

        case MACHNE_STATE_PATIENT_MEASUREMENT_SAFTY_BARRIER:
          if((sl_sleeptimer_tick_to_ms(sl_sleeptimer_get_tick_count()) - safe_timer ) > SAFE_TIMER_MS) {
              measurement_state_machine = MACHNE_STATE_PATIENT_MEASUREMENT_START;
          }
        break;

        case MACHNE_STATE_PATIENT_MEASUREMENT_SEND_BMS_PACKET:
          send_intermediate_bms_packet(&measurement_session, current_working_mode, current_working_sub_mode);
          fs_sd_historical_records_write_data(tx_data_buffer, 19, false, device_params.advanced_params.has_raw_data, adc_unified_buffer, true, false, false);
          measurement_state_machine = MACHNE_STATE_PATIENT_MEASUREMENT_WAIT_BMS_PACKET_SENT;
        break;

        case MACHNE_STATE_PATIENT_MEASUREMENT_WAIT_BMS_PACKET_SENT:
          if(device_operation_control.data_tranfer_control.status.transmitting_to_station_in_progress == ALS_TRUE) {
              measurement_state_machine = MACHNE_STATE_PATIENT_MEASUREMENT_WAIT_BMS_PACKET_SENT;
              #warning "ATTENTION ADD TIMEOUT 22!"
              break;
          }
          else {
              strm_timer = sl_sleeptimer_tick_to_ms(sl_sleeptimer_get_tick_count());
              measurement_state_machine = MACHNE_STATE_WAIT_PATIENT_MEASUREMENT_TO_SEND_STRM;
              aukh_Post_UI_Event(AU_PATIENT_MENU_FINISHED);
              break;
          }
        break;

        case MACHNE_STATE_WAIT_PATIENT_MEASUREMENT_TO_SEND_STRM:
          if((sl_sleeptimer_tick_to_ms(sl_sleeptimer_get_tick_count()) - strm_timer ) > 2000) {
              ///send_strm_packet(); /* @ToDo  Comented out by UP The function send_strm_packet() is called by UI interface */
              measurement_state_machine = MACHNE_STATE_PATIENT_MEASUREMENT_WAIT_BUTTON_PRESSED;
              aukh_Post_UI_Event(AU_REFERENCE_MENU_START);
          }
        break;

        case MACHNE_STATE_PATIENT_MEASUREMENT_WAIT_BUTTON_PRESSED:
          // ждем пока нажмут кнопку
        break;


        case MACHNE_STATE_PATIENT_MEASUREMENT_DONE:
        break;

// =====================================================================
// === РЕЖИМ PATIENT: ПОДРЕЖИМ PATIENT REFERENCE ===============
// =====================================================================

       case MACHINE_STATE_PREPARE_FOR_PATIENT_REFERENCE:
           app_log("MACHINE: PATIENT REFERENCE session started, sub_mode - reference\r\n");
           clear_reference_session(current_working_mode);
           current_working_sub_mode = WORKING_SUB_MODE_PATIENT_REFERENCE;
           measurement_state_machine = MACHNE_STATE_PATIENT_REFERENCE_START;
       break;

       case MACHNE_STATE_PATIENT_REFERENCE_START:
           /*if (!check_device_connected((adc_vref_source_t)device_params.advanced_params.measure_params.adc_vref_source)) {
               break;
           }
           start_impact_capture(device_params.advanced_params.measure_params.adc_taking_period, (adc_vref_source_t)device_params.advanced_params.measure_params.adc_vref_source);*/
           start_capture_adc_scan();
           measurement_state_machine = MACHNE_STATE_PATIENT_REFERENCE_WAIT_ADC;
       break;

       case MACHNE_STATE_PATIENT_REFERENCE_WAIT_ADC:
           if (is_adc_capture_done()) {
               adc_current_state = ADC_STATE_IDLE;
               measurement_state_machine = MACHNE_STATE_PATIENT_REFERENCE_PROCESS;
               break;
           }
       break;

       case MACHNE_STATE_PATIENT_REFERENCE_PROCESS:
           // На чистом Performance датчик наконечника НЕ маркируется
           process_single_indent((uint16_t*)adc_unified_buffer, (uint16_t*)&adc_impact_start_index, &reference_session, current_working_mode, current_working_sub_mode);
           fs_sd_historical_records_write_data(tx_data_buffer, 18, true, device_params.advanced_params.has_raw_data, adc_unified_buffer, false, false, false);
           // проверяем не завалилась ли сессия
           /*if(reference_session.session_failed == true) {
               measurement_state_machine = MACHNE_STATE_PATIENT_REFERENCE_PROCESS_FAILED;
           }
           else {
             measurement_state_machine = MACHNE_STATE_PATIENT_WAIT_ONE_REFERENCE_PACKET_SENT;
           }*/
           measurement_state_machine = MACHNE_STATE_PATIENT_WAIT_ONE_REFERENCE_PACKET_SENT;
       break;

       case MACHNE_STATE_PATIENT_REFERENCE_PROCESS_FAILED:
           stop_measurement_session();
           measurement_state_machine = MACHNE_STATE_IDLE;
       break;

//       case MACHNE_STATE_PATIENT_REFERENCE_PROCESS_FAILED_PRE_STATE_IDLE:
        // stop_measurement_session();
//         measurement_state_machine = MACHNE_STATE_PATIENT_REFERENCE_PROCESS_FAILED;
//       break;


       case MACHNE_STATE_PATIENT_WAIT_ONE_REFERENCE_PACKET_SENT:
         if(device_operation_control.data_tranfer_control.status.transmitting_to_station_in_progress == ALS_TRUE) {
             measurement_state_machine = MACHNE_STATE_PATIENT_WAIT_ONE_REFERENCE_PACKET_SENT;
             #warning "ATTENTION ADD TIMEOUT 11!"
             break;
         }
         else {           // проверяем не завалилась ли сессия
             if(reference_session.session_failed == true) {
                 measurement_state_machine = MACHNE_STATE_PATIENT_REFERENCE_PROCESS_FAILED;
                 aukh_Post_UI_Event(AU_REFERENCE_PATIENT_MENU_REPEAT_START);
                 break;
             }
             else {
                 measurement_state_machine = MACHNE_STATE_PATIENT_REFERENCE_CHECK_COUNT;
               //measurement_state_machine = MACHNE_STATE_PATIENT_WAIT_ONE_REFERENCE_PACKET_SENT;
             }
             //measurement_state_machine = MACHNE_STATE_PATIENT_REFERENCE_CHECK_COUNT;
             break;
         }
       break;

       case MACHNE_STATE_PATIENT_REFERENCE_CHECK_COUNT:
         // Проверяем: набрали ли требуемое количество уколов для ТЕКУЩЕЙ сессии
         if (reference_session.valid_counter >= measurement_session.target_valid) {
             filter_outliers(&reference_session);
             // Проверяем, осталось ли достаточно валидных уколов после фильтрации
             if (reference_session.valid_counter >= reference_session.target_valid) {
                app_log("MATH: PATIENT References Complete! Valid indents: %d\r\n", reference_session.valid_counter);
                // Принудительно гасим АЦП на время паузы, чтобы исключить ложные срабатывания
                adc_current_state = ADC_STATE_IDLE;
                measurement_state_machine = MACHNE_STATE_PATIENT_REFERENCE_SEND_BMS_PACKET;
             }
             else {
              // После отбраковки уколов стало меньше — возвращаемся добирать чистые измерения
              app_log("MATH: PATIENT Outliers removed. Valid count dropped to %d. Capturing more...\r\n", reference_session.valid_counter);
               measurement_state_machine = MACHNE_STATE_PATIENT_REFERENCE_SAFTY_BARRIER;
             }
         } else {
            // Еще не набрали первичный пул — продолжаем захват
            measurement_state_machine = MACHNE_STATE_PATIENT_REFERENCE_SAFTY_BARRIER;
         }
       break;

       case MACHNE_STATE_PATIENT_REFERENCE_SAFTY_BARRIER:
         if((sl_sleeptimer_tick_to_ms(sl_sleeptimer_get_tick_count()) - safe_timer ) > 350) {
             measurement_state_machine = MACHNE_STATE_PATIENT_REFERENCE_START;
         }
       break;

       case MACHNE_STATE_PATIENT_REFERENCE_SEND_BMS_PACKET:
         send_intermediate_bms_packet(&reference_session, current_working_mode, current_working_sub_mode);
         fs_sd_historical_records_write_data(tx_data_buffer, 19, false, device_params.advanced_params.has_raw_data, adc_unified_buffer, false, true, false);
         measurement_state_machine = MACHNE_STATE_PATIENT_REFERENCE_WAIT_BMS_PACKET_SENT;
       break;

       case MACHNE_STATE_PATIENT_REFERENCE_WAIT_BMS_PACKET_SENT:
         if(device_operation_control.data_tranfer_control.status.transmitting_to_station_in_progress == ALS_TRUE) {
             measurement_state_machine = MACHNE_STATE_PATIENT_REFERENCE_WAIT_BMS_PACKET_SENT;
             #warning "ATTENTION ADD TIMEOUT 22!"
             break;
         }
         else {
             calculate_final_bmsi(current_working_mode);
             if(final_results.is_unstable){
                 aukh_Post_UI_Event(AU_REFERENCE_PERFORM_MENU_REPEAT_START);
                 measurement_state_machine = MACHNE_STATE_PATIENT_REFERENCE_START;
                 break;
             }
             strm_timer = sl_sleeptimer_tick_to_ms(sl_sleeptimer_get_tick_count());
             measurement_state_machine = MACHNE_STATE_WAIT_PATIENT_REFERENCE_TO_SEND_STRM;
             aukh_Post_UI_Event(AU_REFERENCE_MENU_FINISHED);
             break;
         }
       break;

       case MACHNE_STATE_WAIT_PATIENT_REFERENCE_TO_SEND_STRM:
         if((sl_sleeptimer_tick_to_ms(sl_sleeptimer_get_tick_count()) - strm_timer ) > 2000) {
             ///send_strm_packet(); /* @ToDo  Comented out by UP The function send_strm_packet() is called by UI interface */
             measurement_state_machine = MACHNE_STATE_PATIENT_CALCULATE_BMSI;
         }
       break;

       case MACHNE_STATE_PATIENT_CALCULATE_BMSI:
       //  app_log("MATH: Both sessions completed successfully. Calculating final BMSi...\r\n");
         //calculate_final_bmsi();
         // Отправка результатов по BLE...
         strm_timer = sl_sleeptimer_tick_to_ms(sl_sleeptimer_get_tick_count());
         measurement_state_machine = MACHNE_STATE_PATIENT_WAIT_SEND_FINAL_BMSI;
       break;

       case MACHNE_STATE_PATIENT_WAIT_SEND_FINAL_BMSI:
         if((sl_sleeptimer_tick_to_ms(sl_sleeptimer_get_tick_count()) - strm_timer ) > 20) {
             measurement_state_machine = MACHNE_STATE_PATIENT_SEND_FINAL_BMSI;
             aukh_Post_UI_Event(AU_PATIENT_MENU_CHECK_END);
         }
       break;


       case MACHNE_STATE_PATIENT_SEND_FINAL_BMSI:
         if(device_operation_control.data_tranfer_control.status.transmitting_to_station_in_progress == ALS_TRUE) {
             measurement_state_machine = MACHNE_STATE_PATIENT_SEND_FINAL_BMSI;
             #warning "ATTENTION ADD TIMEOUT 333!"
             break;
         }
         else {
            // send_final_bmsi_packet(&final_results, current_sub_mode); /* @ToDo  Comented out by UP The function send_final_bmsi_packet() is called by UI interface */
             measurement_state_machine = MACHNE_STATE_IDLE;
             break;
         }
       break;



       case MACHNE_STATE_PATIENT_REFERENCE_WAIT_BUTTON_PRESSED:
         // ждем пока нажмут кнопку
       break;


       case MACHNE_STATE_PATIENT_REFERENCE_DONE:
       break;


        default:
            measurement_state_machine = MACHNE_STATE_IDLE;
        break;
    }
}

void fsrv_calculate_aproximate_bmsi(void) {
  mark_reference_session_as_failed();
  calculate_final_bmsi(current_working_mode);
}

void switch_machine_to_tip_id_validate(void){
  measurement_state_machine =  MACHNE_STATE_TIP_ID_START;
}
