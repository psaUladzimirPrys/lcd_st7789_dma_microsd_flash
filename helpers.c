#include "bgm220sc22hna.h"
#include "bgm22_emu.h"

#include "app_log.h"

#include "helpers.h"

//----------------------------------------------------------------------------------------------------

bool uint_to_ascii(uint32_t val, uint8_t width, uint8_t *out_ptr)
{
    if (out_ptr == NULL) return false;
    if (width == 0) return false;
    // Проверка: влезает ли число в width цифр
    uint32_t max = 1;
    for (uint8_t i = 0; i < width; i++) max *= 10;

    if (val >= max){
        val = val % max;
        //return false;
    }


    // Запись ASCII с ведущими нулями
    for (int i = width - 1; i >= 0; i--)
    {
        out_ptr[i] = (val % 10) + '0';
        val /= 10;
    }
    return true;
}

bool ascii_to_uint(const uint8_t *in_ptr, uint8_t len, uint32_t *out_ptr)
{
  uint32_t val = 0;;

  if(in_ptr == NULL) return false;
  if(out_ptr == NULL) return false;

  for(uint8_t i = 0; i < len; i++)
  {
      if(in_ptr[i] < '0' || in_ptr[i] > '9') return false;

      val = val * 10 + (in_ptr[i] - '0');
  }

  *out_ptr = val;
  return true;
}

//-----------------------------------------------------------------------------------

uint32_t reset_reason = 0;

void get_mcu_reset_cause (void)
{
  reset_reason = EMU->RSTCAUSE;
  /* Очищаем регистр записью в CMD */
  EMU->CMD = EMU_CMD_RSTCAUSECLR;

}

void print_mcu_reset_cause(void)
{
  app_log("\r\n=== RESET CAUSE ===\r\n");
  app_log("EMU_RSTCAUSE = %X/r/n", (unsigned int)reset_reason);

    if (reset_reason & EMU_RSTCAUSE_POR)
      app_log("  [POR]    Power-on reset\r\n");

    if (reset_reason & EMU_RSTCAUSE_PIN)
      app_log("  [PIN]    External pin reset\r\n");

    if (reset_reason & EMU_RSTCAUSE_EM4)
      app_log("  [EM4]    Wakeup from EM4\r\n");

    if (reset_reason & EMU_RSTCAUSE_WDOG0)
      app_log("  [WDOG0]  Watchdog 0 reset\r\n");

    if (reset_reason & EMU_RSTCAUSE_LOCKUP)
      app_log("  [LOCKUP] M33 Core Lockup reset\r\n");

    if (reset_reason & EMU_RSTCAUSE_SYSREQ)
      app_log("  [SYSREQ] Software reset (NVIC_SystemReset)\r\n");

    if (reset_reason & EMU_RSTCAUSE_DVDDBOD)
      app_log("  [DVDDBOD]  HVBOD reset\r\n");

    if (reset_reason & EMU_RSTCAUSE_DVDDLEBOD)
      app_log("  [DVDDLEBOD] LEBOD reset\r\n");

    if (reset_reason & EMU_RSTCAUSE_DECBOD)
      app_log("  [DECBOD]   LVBOD reset\r\n");

    if (reset_reason & EMU_RSTCAUSE_AVDDBOD)
      app_log("  [AVDDBOD]  AVDD BOD reset\r\n");

    if (reset_reason == 0)
      app_log("  [???] No reset cause — unexpected reset!\r\n");
}

//----------------------------------------------------------------------------------------------------------

#define FAULT_EUART         EUART0
#define FAULT_UART_TX_PORT  gpioPortA
#define FAULT_UART_TX_PIN   6
#define FAULT_UART_BAUD     115200

void FaultUART_Init(void)
{
  /* ---- 1. Останавливаем DMA ---- */
      LDMA->CHDIS = 0xFFFFFFFF;

      /* ---- 2. Включаем FSRCO ---- */
      CMU->CLKEN0_SET = CMU_CLKEN0_FSRCO;
      for (volatile int i = 0; i < 100; i++); /* ждём готовности */

      /* ---- 3. Цепочка клоков: FSRCO -> EM01GRPACLK -> EUART0CLK ---- */
      CMU->EM01GRPACLKCTRL = (CMU->EM01GRPACLKCTRL
                              & ~_CMU_EM01GRPACLKCTRL_CLKSEL_MASK)
                              | CMU_EM01GRPACLKCTRL_CLKSEL_FSRCO;

      CMU->EUART0CLKCTRL = (CMU->EUART0CLKCTRL
                            & ~_CMU_EUART0CLKCTRL_CLKSEL_MASK)
                            | CMU_EUART0CLKCTRL_CLKSEL_EM01GRPACLK;

      /* Включаем GPIO и EUART0 — оба в CLKEN0 для BGM22 */
      CMU->CLKEN0_SET = CMU_CLKEN0_GPIO;
      CMU->CLKEN0_SET = CMU_CLKEN0_EUART0;

      /* ---- 4. Сбрасываем EUART0 ---- */

      /* Отключаем TX/RX */
      EUART0->CMD = EUSART_CMD_RXDIS | EUSART_CMD_TXDIS;
      while (EUART0->SYNCBUSY & (EUSART_SYNCBUSY_RXDIS |
                                  EUSART_SYNCBUSY_TXDIS));

      /* Сбрасываем TX FIFO */
      EUART0->CMD_SET = EUSART_CMD_CLEARTX;
      while (EUART0->STATUS & EUSART_STATUS_CLEARTXBUSY);

      /* Отключаем EUART0 */
      EUART0->EN_CLR = EUSART_EN_EN;
      while (EUART0->SYNCBUSY != 0);

      /* ---- 5. Конфигурируем EUART0 115200 8N1 @ FSRCO 20MHz ---- */

      /* Включаем EUART0 */
      EUART0->EN_SET = EUSART_EN_EN;
      while (EUART0->SYNCBUSY != 0);

      /* Сбрасываем конфигурационные регистры */
      EUART0->CFG0     = _EUSART_CFG0_RESETVALUE;
      EUART0->CFG1     = _EUSART_CFG1_RESETVALUE;

      /* 8N1: 8 data bits, no parity, 1 stop bit */
      EUART0->FRAMECFG = (0x5UL << _EUSART_FRAMECFG_DATABITS_SHIFT)
                       | (0x0UL << _EUSART_FRAMECFG_PARITY_SHIFT)
                       | (0x1UL << _EUSART_FRAMECFG_STOPBITS_SHIFT);

      /* CLKDIV для 115200 при FSRCO=20MHz, OVS16:
       * CLKDIV = ((20000000 / (16 * 115200)) - 1) * 256
       *        = (10.851 - 1) * 256
       *        = 2522
       * Поле CLKDIV начинается с бита 3 */
      EUART0->CLKDIV = 2522 << 3;

      /* Включаем только TX */
      EUART0->CMD = EUSART_CMD_TXEN;
      while (EUART0->SYNCBUSY & EUSART_SYNCBUSY_TXEN);
      while (!(EUART0->STATUS & EUSART_STATUS_TXENS));

      /* ---- 6. GPIO PA6 -> TX ---- */

      /* PA6 как push-pull output (MODE6 = 4) */
      GPIO->P[0].MODEL = (GPIO->P[0].MODEL
                         & ~_GPIO_P_MODEL_MODE6_MASK)
                         | (0x4UL << _GPIO_P_MODEL_MODE6_SHIFT);

      /* Устанавливаем высокий уровень (UART idle = high) */
      GPIO->P[0].DOUT |= (1UL << 6);

      /* Роутинг EUART0 TX -> PA6 */
      GPIO->EUARTROUTE->TXROUTE =
          (0UL << _GPIO_EUART_TXROUTE_PORT_SHIFT)  /* gpioPortA = 0 */
        | (6UL << _GPIO_EUART_TXROUTE_PIN_SHIFT);
      GPIO->EUARTROUTE->ROUTEEN = GPIO_EUART_ROUTEEN_TXPEN;

      /* Задержка для устойчивости линии */
      for (volatile int i = 0; i < 1000; i++);
}

/* ================================================================
 * Безопасный вывод символа через EUART0
 * Только polling — без IRQ, без DMA
 * Безопасен в контексте любого fault handler
 * ================================================================ */
void fault_putchar(char c)
{
    /* Ждём свободного места в TX FIFO */
    while (!(EUART0->STATUS & EUSART_STATUS_TXFL));

    /* Отправляем символ */
    EUART0->TXDATA = (uint32_t)c;

    /* Ждём завершения передачи */
    while (!(EUART0->STATUS & EUSART_STATUS_TXC));

    /* EUSART/EUART требует явной очистки флага TXC */
    EUART0->IF_CLR = EUSART_IF_TXC;
}

/* ================================================================
 * Вывод строки (null-terminated)
 * Вызывает fault_putchar для каждого символа
 * ================================================================ */
void fault_puts(const char *s)
{
    while (*s) fault_putchar(*s++);
}

/* ================================================================
 * Вывод 32-битного числа в шестнадцатеричном формате
 * Пример вывода: 0x20001234
 * ================================================================ */
void fault_hex32(uint32_t val)
{
    const char h[] = "0123456789ABCDEF";
    fault_putchar('0');
    fault_putchar('x');
    for (int i = 28; i >= 0; i -= 4)
        fault_putchar(h[(val >> i) & 0xF]);
}

/* ================================================================
 * Вывод знакового 32-битного числа в десятичном формате
 * Используется для вывода номеров IRQ, смещений стека и т.д.
 * ================================================================ */
void fault_dec(int32_t val)
{
    char buf[12];
    int  i = 0;

    if (val == 0) { fault_putchar('0'); return; }
    if (val < 0)  { fault_putchar('-'); val = -val; }

    while (val > 0) { buf[i++] = '0' + (val % 10); val /= 10; }
    while (i > 0)   { fault_putchar(buf[--i]); }
}

/* ================================================================
 * Вывод строки регистра в формате:
 *   NAME = 0xXXXXXXXX
 * ================================================================ */
void fault_reg(const char *name, uint32_t val)
{
    fault_puts("  ");
    fault_puts(name);
    fault_puts(" = ");
    fault_hex32(val);
    fault_puts("\r\n");
}

/* ================================================================
 * Выводит строку только если бит mask установлен в reg
 * Формат вывода:
 *   [!] NAME: описание
 * ================================================================ */
void fault_flag(uint32_t reg, uint32_t mask, const char *name, const char *desc)
{
    if (reg & mask) {
        fault_puts("  [!] ");
        fault_puts(name);
        fault_puts(": ");
        fault_puts(desc);
        fault_puts("\r\n");
    }
}

/* ================================================================
 * Вывод разделителя
 * ================================================================ */
void fault_separator(void)
{
    fault_puts("+--------------------------------------------------+\r\n");
}

/* ================================================================
 * Вывод заголовка секции в формате:
 *
 * +--------------------------------------------------+
 * | НАЗВАНИЕ СЕКЦИИ
 * +--------------------------------------------------+
 * ================================================================ */
void fault_section(const char *title)
{
    fault_puts("\r\n");
    fault_separator();
    fault_puts("| ");
    fault_puts(title);
    fault_puts("\r\n");
    fault_separator();
}

/* ================================================================
 * Дамп содержимого стека — 16 слов от SP
 *
 * Пример вывода:
 *   [SP+0 ] = 0x20001234
 *   [SP+4 ] = 0x00008ABC
 *   ...
 *
 * Используется для ручного анализа цепочки вызовов
 * ================================================================ */
void fault_dump_stack(uint32_t *sp)
{
    fault_section("STACK DUMP (16 words from SP)");

    for (int i = 0; i < 16; i++) {
        fault_puts("  [SP+");
        fault_dec(i * 4);
        fault_puts("] = ");
        fault_hex32(sp[i]);

        /* Подсказка: Thumb адреса нечётные и попадают в диапазон flash */
        if ((sp[i] & 0x1) &&
            (sp[i] >= 0x00000100) &&
            (sp[i] <= 0x000FFFFF)) {
            fault_puts("  <-- possible return address");
        }

        /* Подсказка: адреса в RAM */
        if ((sp[i] >= 0x20000000) &&
            (sp[i] <= 0x2007FFFF)) {
            fault_puts("  <-- RAM address");
        }

        fault_puts("\r\n");
    }
}

/* ================================================================
 * Анализ цепочки вызовов — откуда пришли в fault
 *
 * Пример вывода:
 *   PC   = 0x000038DC  <-- find this in .map file
 *   LR   = 0x0000AB61  <-- caller of faulting function
 *
 *   LR (EXC_RETURN) decode:
 *   Mode:  Thread
 *   Stack: PSP
 *   FPU:   no FP state
 *
 *   Stack scan for return addresses:
 *   [SP+8 ] = 0x00012345  <-- possible return address
 * ================================================================ */
void fault_call_chain(uint32_t *sp,
                              uint32_t  lr,
                              uint32_t  pc,
                              uint32_t  msp,
                              uint32_t  psp)
{
    fault_section("CALL CHAIN (where did we come from)");

    /* PC — адрес инструкции вызвавшей fault */
    fault_puts("  PC = ");
    fault_hex32(pc);
    fault_puts("  <-- faulting instruction, find in .map file\r\n");

    /* LR — адрес возврата */
    fault_puts("  LR = ");
    fault_hex32(lr);
    fault_puts("  <-- caller of faulting function\r\n");

    /* ---- Расшифровка LR как EXC_RETURN ---- */
    fault_puts("\r\n  LR (EXC_RETURN) decode:\r\n");

    if ((lr & 0xFFFFFFF0) == 0xFFFFFFF0) {
        /* Это EXC_RETURN код — стандартный при входе в исключение */
        fault_puts("  Type:  EXC_RETURN code\r\n");

        fault_puts("  Mode:  ");
        fault_puts((lr & 0x8) ? "Thread mode\r\n" : "Handler mode\r\n");

        fault_puts("  Stack: ");
        fault_puts((lr & 0x4) ? "PSP\r\n" : "MSP\r\n");

        fault_puts("  FPU:   ");
        fault_puts((lr & 0x10) ? "no FP state saved\r\n"
                                : "FP state saved on stack\r\n");

        /* Дополнительно показываем какой SP использовался */
        fault_puts("  SP used for frame: ");
        if (lr & 0x4) {
            fault_hex32(psp);
            fault_puts(" (PSP)\r\n");
        } else {
            fault_hex32(msp);
            fault_puts(" (MSP)\r\n");
        }
    } else {
        /* Обычный LR — адрес возврата из функции */
        fault_puts("  Type:  Normal return address\r\n");
        fault_puts("  Addr:  ");
        fault_hex32(lr);
        fault_puts("  <-- find caller in .map file\r\n");

        /* Проверяем LSB — должен быть 1 для Thumb */
        if (!(lr & 0x1)) {
            fault_puts("  [!!] WARNING: LR LSB=0 — not a Thumb address!\r\n");
            fault_puts("       Possible stack corruption or wrong function ptr\r\n");
        }
    }

    /* ---- Сканирование стека в поисках адресов возврата ---- */
    fault_puts("\r\n  Stack scan for return addresses:\r\n");

    int found = 0;
    for (int i = 0; i < 32 && found < 8; i++) {
        uint32_t val = sp[i];

        /* Thumb адреса: LSB=1, попадают в диапазон flash */
        if ((val & 0x1) &&
            (val >= 0x00000100) &&
            (val <= 0x000FFFFF)) {
            fault_puts("  [SP+");
            fault_dec(i * 4);
            fault_puts("] = ");
            fault_hex32(val);
            fault_puts("  <-- possible return address\r\n");
            found++;
        }
    }

    if (found == 0) {
        fault_puts("  (no return addresses found in stack)\r\n");
        fault_puts("  Possible stack overflow or corruption\r\n");
    }

    /* ---- Проверка переполнения стека ---- */
    fault_puts("\r\n  Stack usage:\r\n");
    fault_puts("  MSP = "); fault_hex32(msp); fault_puts("\r\n");
    fault_puts("  PSP = "); fault_hex32(psp); fault_puts("\r\n");

    /* Проверяем не вышел ли SP за пределы RAM */
    if (msp < 0x20000000 || msp > 0x2007FFFF) {
        fault_puts("  [!!] WARNING: MSP outside RAM range!\r\n");
        fault_puts("       Possible stack overflow\r\n");
    }
    if (psp != 0 && (psp < 0x20000000 || psp > 0x2007FFFF)) {
        fault_puts("  [!!] WARNING: PSP outside RAM range!\r\n");
        fault_puts("       Possible stack overflow\r\n");
    }
}

/* ================================================================
 * Анализ причины fault — человекочитаемый вывод
 *
 * Определяет ОДНУ главную причину и даёт рекомендации
 * по устранению. Анализирует в порядке приоритета:
 *   1. HFSR — тип hard fault
 *   2. CFSR MemManage — нарушение MPU
 *   3. CFSR Bus Fault — ошибка шины
 *   4. CFSR Usage Fault — ошибка использования
 * ================================================================ */
void fault_root_cause(uint32_t cfsr,
                              uint32_t hfsr,
                              uint32_t mmfar,
                              uint32_t bfar,
                              uint32_t pc,
                              uint32_t lr)
{
    fault_section("ROOT CAUSE ANALYSIS");

    /* ---- HFSR ---- */
    if (hfsr & SCB_HFSR_VECTTBL_Msk) {
        fault_puts("  CAUSE: Bus fault reading vector table\r\n");
        fault_puts("  WHY:   - Corrupted flash\r\n");
        fault_puts("         - Wrong VTOR address\r\n");
        fault_puts("         - Stack overflow overwrote vector table\r\n");
        fault_puts("  FIX:   Check SCB->VTOR value:\r\n");
        fault_puts("         VTOR = "); fault_hex32(SCB->VTOR);
        fault_puts("\r\n");
        return;
    }

    if (hfsr & SCB_HFSR_DEBUGEVT_Msk) {
        fault_puts("  CAUSE: BKPT instruction executed\r\n");
        fault_puts("  WHY:   - Debugger not attached\r\n");
        fault_puts("         - assert() or BKPT in code path\r\n");
        fault_puts("  PC:    "); fault_hex32(pc);
        fault_puts("  <-- BKPT location\r\n");
        return;
    }

    /* ---- CFSR MemManage ---- */
    if (cfsr & 0x00000001) {
        fault_puts("  CAUSE: MPU instruction access violation\r\n");
        fault_puts("  WHY:   - Jumped to protected/unmapped region\r\n");
        fault_puts("         - Corrupted function pointer\r\n");
        fault_puts("         - Stack overflow into code region\r\n");
        fault_puts("  PC:    "); fault_hex32(pc);
        fault_puts("  <-- illegal execution address\r\n");
        return;
    }

    if (cfsr & 0x00000002) {
        fault_puts("  CAUSE: MPU data access violation\r\n");
        fault_puts("  WHY:   - Access to MPU-protected region\r\n");
        fault_puts("         - NULL pointer dereference\r\n");
        fault_puts("         - Access to peripheral without clock\r\n");
        fault_puts("  PC:    "); fault_hex32(pc);
        fault_puts("  <-- instruction that caused it\r\n");
        if (cfsr & 0x00000080) {
            fault_puts("  ADDR:  "); fault_hex32(mmfar);
            fault_puts("  <-- illegal access address\r\n");
            if (mmfar == 0x00000000)
                fault_puts("  [!!] NULL pointer dereference!\r\n");
        } else {
            fault_puts("  ADDR:  (MMFAR not valid)\r\n");
        }
        return;
    }

    if (cfsr & 0x00000008) {
        fault_puts("  CAUSE: MemManage fault on exception unstacking\r\n");
        fault_puts("  WHY:   - Stack pointer corrupted during ISR\r\n");
        fault_puts("         - Stack overflow\r\n");
        fault_puts("  MSP/PSP may be invalid\r\n");
        return;
    }

    if (cfsr & 0x00000010) {
        fault_puts("  CAUSE: MemManage fault on exception stacking\r\n");
        fault_puts("  WHY:   - Stack overflow at time of exception\r\n");
        fault_puts("         - Stack pointer near bottom of stack\r\n");
        return;
    }

    /* ---- CFSR Bus Fault ---- */
    if (cfsr & 0x00000100) {
        fault_puts("  CAUSE: Instruction bus error\r\n");
        fault_puts("  WHY:   - Fetch from invalid/unmapped address\r\n");
        fault_puts("         - Corrupted function pointer\r\n");
        fault_puts("         - Jump to uninitialized pointer\r\n");
        fault_puts("  PC:    "); fault_hex32(pc);
        fault_puts("  <-- invalid fetch address\r\n");
        return;
    }

    if (cfsr & 0x00000200) {
        fault_puts("  CAUSE: Precise data bus error\r\n");
        fault_puts("  WHY:   - Access to invalid/unmapped memory\r\n");
        fault_puts("         - Access to peripheral without clock\r\n");
        fault_puts("         - Access beyond peripheral address range\r\n");
        fault_puts("  PC:    "); fault_hex32(pc);
        fault_puts("  <-- instruction that caused it\r\n");
        if (cfsr & 0x00008000) {
            fault_puts("  ADDR:  "); fault_hex32(bfar);
            fault_puts("  <-- invalid bus address\r\n");
            if (bfar == 0x00000000)
                fault_puts("  [!!] NULL pointer dereference!\r\n");
        } else {
            fault_puts("  ADDR:  (BFAR not valid)\r\n");
        }
        return;
    }

    if (cfsr & 0x00000400) {
        fault_puts("  CAUSE: Imprecise data bus error\r\n");
        fault_puts("  WHY:   - Write buffer error\r\n");
        fault_puts("         - PC may NOT point to faulting instruction\r\n");
        fault_puts("         - Check LR for approximate location\r\n");
        fault_puts("  PC:    "); fault_hex32(pc);
        fault_puts("  (approximate)\r\n");
        fault_puts("  LR:    "); fault_hex32(lr);
        fault_puts("\r\n");
        return;
    }

    if (cfsr & 0x00000800) {
        fault_puts("  CAUSE: Bus fault on exception unstacking\r\n");
        fault_puts("  WHY:   - Stack pointer corrupted during ISR\r\n");
        fault_puts("         - Stack overflow\r\n");
        return;
    }

    if (cfsr & 0x00001000) {
        fault_puts("  CAUSE: Bus fault on exception stacking\r\n");
        fault_puts("  WHY:   - Stack overflow at time of exception\r\n");
        fault_puts("         - Stack pointer near bottom of stack\r\n");
        return;
    }

    /* ---- CFSR Usage Fault ---- */
    if (cfsr & 0x00010000) {
        fault_puts("  CAUSE: Undefined instruction\r\n");
        fault_puts("  WHY:   - Jumped to data/string area\r\n");
        fault_puts("         - Uninitialized/corrupted function pointer\r\n");
        fault_puts("         - FPU instruction with FPU disabled\r\n");
        fault_puts("         - Wrong compiler settings\r\n");
        fault_puts("  PC:    "); fault_hex32(pc);
        fault_puts("  <-- check disassembly at this address\r\n");
        return;
    }

    if (cfsr & 0x00020000) {
        fault_puts("  CAUSE: Invalid CPU state (INVSTATE)\r\n");
        fault_puts("  WHY:   - Function pointer with LSB=0 (not Thumb)\r\n");
        fault_puts("         - Corrupted PSR or stack\r\n");
        fault_puts("         - BLX to ARM address in Thumb-only core\r\n");
        fault_puts("  PC:    "); fault_hex32(pc);
        fault_puts("\r\n");
        fault_puts("  LR:    "); fault_hex32(lr);
        fault_puts("\r\n");
        if (!(lr & 0x1))
            fault_puts("  [!!] LR LSB=0 — not a valid Thumb address!\r\n");
        return;
    }

    if (cfsr & 0x00040000) {
        fault_puts("  CAUSE: Invalid PC on exception return (INVPC)\r\n");
        fault_puts("  WHY:   - Corrupted EXC_RETURN value\r\n");
        fault_puts("         - Stack overflow corrupted LR\r\n");
        fault_puts("         - Wrong IRQ priority configuration\r\n");
        fault_puts("  LR:    "); fault_hex32(lr);
        fault_puts("  <-- corrupted EXC_RETURN\r\n");
        return;
    }

    if (cfsr & 0x00080000) {
        fault_puts("  CAUSE: No coprocessor (NOCP)\r\n");
        fault_puts("  WHY:   - FPU instruction used but FPU not enabled\r\n");
        fault_puts("  FIX:   Add to init code:\r\n");
        fault_puts("         SCB->CPACR |= 0x00F00000;\r\n");
        fault_puts("  PC:    "); fault_hex32(pc);
        fault_puts("\r\n");
        return;
    }

    if (cfsr & 0x02000000) {
        fault_puts("  CAUSE: Divide by zero\r\n");
        fault_puts("  WHY:   - Integer division with divisor = 0\r\n");
        fault_puts("         - Check R1 or R2 register value\r\n");
        fault_puts("  PC:    "); fault_hex32(pc);
        fault_puts("  <-- division instruction\r\n");
        return;
    }

    if (cfsr & 0x01000000) {
        fault_puts("  CAUSE: Unaligned memory access\r\n");
        fault_puts("  WHY:   - Unaligned read/write with UNALIGN_TRP set\r\n");
        fault_puts("         - Packed struct access\r\n");
        fault_puts("         - Wrong pointer cast (e.g. char* to uint32_t*)\r\n");
        fault_puts("  PC:    "); fault_hex32(pc);
        fault_puts("  <-- unaligned access instruction\r\n");
        return;
    }

    /* Неизвестная причина */
    fault_puts("  CAUSE: Unknown\r\n");
    fault_puts("  WHY:   No CFSR/HFSR bits set\r\n");
    fault_puts("  CHECK: Raw CFSR = "); fault_hex32(cfsr); fault_puts("\r\n");
    fault_puts("         Raw HFSR = "); fault_hex32(hfsr); fault_puts("\r\n");
}

/* ================================================================
 * Универсальный автоматический анализ HardFault
 * Работает для любого типа fault
 * ================================================================ */
void fault_auto_analysis(uint32_t *sp,
                                 uint32_t  cfsr,
                                 uint32_t  hfsr,
                                 uint32_t  mmfar,
                                 uint32_t  bfar,
                                 uint32_t  pc,
                                 uint32_t  lr,
                                 uint32_t  r0,
                                 uint32_t  r1,
                                 uint32_t  r2,
                                 uint32_t  r3,
                                 uint32_t  msp,
                                 uint32_t  psp)
{
    fault_puts("\r\n");
    fault_puts("**************************************************\r\n");
    fault_puts("**         AUTOMATIC FAULT ANALYSIS           **\r\n");
    fault_puts("**************************************************\r\n");

    /* ============================================================
     * STEP 1: Тип HardFault
     * ============================================================ */
    fault_section("STEP 1: HardFault type");

    if (hfsr & SCB_HFSR_FORCED_Msk) {
        fault_puts("  Type: FORCED — escalated from configurable fault\r\n");
        fault_puts("  Root cause is in CFSR (see STEP 2)\r\n");
    }
    if (hfsr & SCB_HFSR_VECTTBL_Msk) {
        fault_puts("  Type: VECTTBL — bus fault reading vector table\r\n");
        fault_puts("  VTOR = "); fault_hex32(SCB->VTOR); fault_puts("\r\n");
        fault_puts("  Possible causes:\r\n");
        fault_puts("    - Flash corrupted\r\n");
        fault_puts("    - Wrong VTOR address\r\n");
        fault_puts("    - Stack overflow overwrote vector table\r\n");
    }
    if (hfsr & SCB_HFSR_DEBUGEVT_Msk) {
        fault_puts("  Type: DEBUGEVT — BKPT without debugger\r\n");
        fault_puts("  PC = "); fault_hex32(pc);
        fault_puts("  <-- BKPT location\r\n");
    }
    if (!(hfsr & (SCB_HFSR_FORCED_Msk |
                  SCB_HFSR_VECTTBL_Msk |
                  SCB_HFSR_DEBUGEVT_Msk))) {
        fault_puts("  Type: Unknown — no HFSR bits set\r\n");
    }

    /* ============================================================
     * STEP 2: Причина из CFSR
     * ============================================================ */
    fault_section("STEP 2: Root cause from CFSR");

    int cause_found = 0;

    /* ---- MemManage ---- */
    if (cfsr & 0x00000001) {
        cause_found++;
        fault_puts("  [IACCVIOL] MPU instruction access violation\r\n");
        fault_puts("  Tried to execute code at: "); fault_hex32(pc); fault_puts("\r\n");
        fault_puts("  Possible causes:\r\n");
        fault_puts("    - Jumped to protected/unmapped region\r\n");
        fault_puts("    - Corrupted function pointer\r\n");
        fault_puts("    - Stack overflow into code region\r\n");
    }
    if (cfsr & 0x00000002) {
        cause_found++;
        fault_puts("  [DACCVIOL] MPU data access violation\r\n");
        if (cfsr & 0x00000080) {
            fault_puts("  Bad address MMFAR = "); fault_hex32(mmfar); fault_puts("\r\n");
            if (mmfar == 0x00000000)
                fault_puts("  [!!!] NULL pointer dereference!\r\n");
            else if (mmfar < 0x100)
                fault_puts("  [!!!] Near-NULL pointer dereference!\r\n");
        } else {
            fault_puts("  MMFAR not valid\r\n");
        }
        fault_puts("  Possible causes:\r\n");
        fault_puts("    - NULL/invalid pointer dereference\r\n");
        fault_puts("    - Access to MPU-protected region\r\n");
        fault_puts("    - Peripheral access without clock enabled\r\n");
    }
    if (cfsr & 0x00000008) {
        cause_found++;
        fault_puts("  [MUNSTKERR] MemManage fault on exception unstacking\r\n");
        fault_puts("  Possible causes:\r\n");
        fault_puts("    - Stack pointer corrupted during ISR\r\n");
        fault_puts("    - Stack overflow\r\n");
    }
    if (cfsr & 0x00000010) {
        cause_found++;
        fault_puts("  [MSTKERR] MemManage fault on exception stacking\r\n");
        fault_puts("  Possible causes:\r\n");
        fault_puts("    - Stack overflow at time of exception\r\n");
        fault_puts("    - Stack pointer near bottom of stack\r\n");
    }

    /* ---- Bus Fault ---- */
    if (cfsr & 0x00000100) {
        cause_found++;
        fault_puts("  [IBUSERR] Instruction bus error\r\n");
        fault_puts("  Tried to fetch from: "); fault_hex32(pc); fault_puts("\r\n");
        fault_puts("  Possible causes:\r\n");
        fault_puts("    - Jump to unmapped/invalid address\r\n");
        fault_puts("    - Corrupted function pointer\r\n");
        fault_puts("    - Jump to uninitialized pointer\r\n");
    }
    if (cfsr & 0x00000200) {
        cause_found++;
        fault_puts("  [PRECISERR] Precise data bus error\r\n");
        if (cfsr & 0x00008000) {
            fault_puts("  Bad address BFAR = "); fault_hex32(bfar); fault_puts("\r\n");
            if (bfar == 0x00000000)
                fault_puts("  [!!!] NULL pointer dereference!\r\n");
            else if (bfar < 0x100)
                fault_puts("  [!!!] Near-NULL pointer dereference!\r\n");
        } else {
            fault_puts("  BFAR not valid\r\n");
        }
        fault_puts("  Possible causes:\r\n");
        fault_puts("    - Access to unmapped memory\r\n");
        fault_puts("    - Peripheral without clock enabled\r\n");
        fault_puts("    - Access beyond peripheral address range\r\n");
    }
    if (cfsr & 0x00000400) {
        cause_found++;
        fault_puts("  [IMPRECISERR] Imprecise data bus error\r\n");
        fault_puts("  WARNING: PC may NOT point to faulting instruction!\r\n");
        fault_puts("  Check LR for approximate location\r\n");
        fault_puts("  Possible causes:\r\n");
        fault_puts("    - Write buffer error\r\n");
        fault_puts("    - DMA access to invalid address\r\n");
    }
    if (cfsr & 0x00000800) {
        cause_found++;
        fault_puts("  [UNSTKERR] Bus fault on exception unstacking\r\n");
        fault_puts("  Possible causes:\r\n");
        fault_puts("    - Stack pointer corrupted during ISR\r\n");
        fault_puts("    - Stack overflow\r\n");
    }
    if (cfsr & 0x00001000) {
        cause_found++;
        fault_puts("  [STKERR] Bus fault on exception stacking\r\n");
        fault_puts("  Possible causes:\r\n");
        fault_puts("    - Stack overflow at time of exception\r\n");
    }

    /* ---- Usage Fault ---- */
    if (cfsr & 0x00010000) {
        cause_found++;
        fault_puts("  [UNDEFINSTR] Undefined instruction at PC = ");
        fault_hex32(pc); fault_puts("\r\n");
        fault_puts("  Possible causes:\r\n");
        fault_puts("    - Jumped to data/string area\r\n");
        fault_puts("    - Uninitialized/corrupted function pointer\r\n");
        fault_puts("    - FPU instruction with FPU disabled\r\n");
        fault_puts("    - Wrong compiler settings\r\n");
    }
    if (cfsr & 0x00020000) {
        cause_found++;
        fault_puts("  [INVSTATE] Invalid CPU state at PC = ");
        fault_hex32(pc); fault_puts("\r\n");
        fault_puts("  PC LSB = "); fault_dec(pc & 1); fault_puts("\r\n");
        fault_puts("  LR LSB = "); fault_dec(lr & 1); fault_puts("\r\n");
        if (!(pc & 1)) fault_puts("  [!!!] PC LSB=0 — not a Thumb address!\r\n");
        if (!(lr & 1)) fault_puts("  [!!!] LR LSB=0 — not a Thumb address!\r\n");
        fault_puts("  Possible causes:\r\n");
        fault_puts("    - Function pointer with LSB=0\r\n");
        fault_puts("    - BLX to ARM address in Thumb-only core\r\n");
        fault_puts("    - Corrupted PSR or stack\r\n");
    }
    if (cfsr & 0x00040000) {
        cause_found++;
        fault_puts("  [INVPC] Invalid PC on exception return\r\n");
        fault_puts("  LR (EXC_RETURN) = "); fault_hex32(lr); fault_puts("\r\n");
        fault_puts("  Possible causes:\r\n");
        fault_puts("    - Corrupted EXC_RETURN value\r\n");
        fault_puts("    - Stack overflow corrupted LR\r\n");
        fault_puts("    - Wrong IRQ priority configuration\r\n");
    }
    if (cfsr & 0x00080000) {
        cause_found++;
        fault_puts("  [NOCP] No coprocessor at PC = ");
        fault_hex32(pc); fault_puts("\r\n");
        fault_puts("  Possible causes:\r\n");
        fault_puts("    - FPU instruction used but FPU not enabled\r\n");
        fault_puts("  Fix: SCB->CPACR |= 0x00F00000;\r\n");
    }
    if (cfsr & 0x01000000) {
        cause_found++;
        fault_puts("  [UNALIGNED] Unaligned memory access at PC = ");
        fault_hex32(pc); fault_puts("\r\n");
        fault_puts("  Possible causes:\r\n");
        fault_puts("    - Packed struct access\r\n");
        fault_puts("    - Wrong pointer cast (char* to uint32_t*)\r\n");
        fault_puts("    - Misaligned buffer\r\n");
    }
    if (cfsr & 0x02000000) {
        cause_found++;
        fault_puts("  [DIVBYZERO] Division by zero at PC = ");
        fault_hex32(pc); fault_puts("\r\n");
        /* Проверяем все регистры на ноль */
        fault_puts("  Register values:\r\n");
        fault_puts("    R0 = "); fault_hex32(r0);
        if (r0 == 0) fault_puts("  <-- possible divisor (zero!)");
        fault_puts("\r\n");
        fault_puts("    R1 = "); fault_hex32(r1);
        if (r1 == 0) fault_puts("  <-- possible divisor (zero!)");
        fault_puts("\r\n");
        fault_puts("    R2 = "); fault_hex32(r2);
        if (r2 == 0) fault_puts("  <-- possible divisor (zero!)");
        fault_puts("\r\n");
        fault_puts("    R3 = "); fault_hex32(r3);
        if (r3 == 0) fault_puts("  <-- possible divisor (zero!)");
        fault_puts("\r\n");
        fault_puts("  Fix: if (divisor != 0) { result = a / divisor; }\r\n");
    }

    if (!cause_found) {
        fault_puts("  No CFSR bits set — unknown cause\r\n");
        fault_puts("  Check raw CFSR = "); fault_hex32(cfsr); fault_puts("\r\n");
        fault_puts("  Check raw HFSR = "); fault_hex32(hfsr); fault_puts("\r\n");
    }

    /* ============================================================
     * STEP 3: Анализ PC и LR
     * ============================================================ */
    fault_section("STEP 3: PC / LR analysis");

    fault_puts("  PC = "); fault_hex32(pc);
    fault_puts("  <-- faulting instruction\r\n");
    fault_puts("  LR = "); fault_hex32(lr);
    fault_puts("  <-- return address\r\n");

    /* Расстояние между PC и LR */
    if ((lr & ~1u) != 0) {
        int32_t dist = (int32_t)(pc - (lr & ~1u));
        if (dist >= 0 && dist < 64) {
            fault_puts("  [!] PC and LR are "); fault_dec(dist);
            fault_puts(" bytes apart\r\n");
            fault_puts("      Fault is INSIDE the same function as LR\r\n");
        } else if (dist < 0) {
            fault_puts("  [!] PC is BEFORE LR — possible tail call or\r\n");
            fault_puts("      inlined function\r\n");
        } else {
            fault_puts("  PC-LR distance = "); fault_dec(dist);
            fault_puts(" bytes\r\n");
        }
    }

    /* Проверка валидности PC */
    fault_puts("  PC validity:\r\n");
    if (pc < 0x00000100)
        fault_puts("  [!!!] PC near zero — NULL/invalid function pointer!\r\n");
    else if (pc > 0x000FFFFF)
        fault_puts("  [!!!] PC outside flash range — corrupted PC!\r\n");
    else
        fault_puts("  PC in valid flash range\r\n");

    /* Проверка LSB */
    if (!(pc & 1))
        fault_puts("  [!!!] PC LSB=0 — not a Thumb address!\r\n");
    if (!(lr & 1) && (lr & 0xFFFFFFF0) != 0xFFFFFFF0)
        fault_puts("  [!!!] LR LSB=0 — not a Thumb address!\r\n");

    /* ============================================================
     * STEP 4: Анализ стека
     * ============================================================ */
    fault_section("STEP 4: Stack analysis");

    /* Проверка переполнения стека */
    fault_puts("  MSP = "); fault_hex32(msp); fault_puts("\r\n");
    fault_puts("  PSP = "); fault_hex32(psp); fault_puts("\r\n");

    if (msp < 0x20000000 || msp > 0x2007FFFF)
        fault_puts("  [!!!] MSP outside RAM — stack overflow!\r\n");
    else if (msp < 0x20000100)
        fault_puts("  [!] MSP near bottom of RAM — stack almost full!\r\n");
    else
        fault_puts("  MSP in valid RAM range\r\n");

    /* Расстояние между MSP и PSP */
    if (psp != 0 && psp != 0xFFFFFFFF) {
        uint32_t stack_diff = (msp > psp) ? (msp - psp) : (psp - msp);
        fault_puts("  MSP-PSP diff = "); fault_dec(stack_diff);
        fault_puts(" bytes\r\n");
    }

    /* ============================================================
     * STEP 5: Цепочка вызовов для addr2line
     * ============================================================ */
    fault_section("STEP 5: Call chain — run this command:");

    fault_puts("  arm-none-eabi-addr2line \\\r\n");
    fault_puts("    -e firmware.elf -f -p -a \\\r\n");

    /* PC */
    fault_puts("    "); fault_hex32(pc);
    fault_puts(" \\  <- FAULT HERE\r\n");

    /* LR если не EXC_RETURN */
    if ((lr & 0xFFFFFFF0) != 0xFFFFFFF0) {
        fault_puts("    "); fault_hex32(lr & ~1u);
        fault_puts(" \\  <- LR (caller)\r\n");
    }

    /* Адреса из стека */
    int found = 0;
    for (int i = 0; i < 32 && found < 6; i++) {
        uint32_t val = sp[i];
        if ((val & 0x1) &&
            (val >= 0x00000100) &&
            (val <= 0x000FFFFF) &&
            ((val & ~1u) != (lr & ~1u))) {
            fault_puts("    "); fault_hex32(val & ~1u);
            fault_puts(" \\  <- SP+"); fault_dec(i * 4);
            fault_puts("\r\n");
            found++;
        }
    }
    if (found == 0)
        fault_puts("  (no additional return addresses found)\r\n");

    /* ============================================================
     * STEP 6: Итоговый вердикт
     * ============================================================ */
    fault_section("STEP 6: Verdict and fix");

    /* NULL pointer */
    if ((cfsr & 0x00000002) && (cfsr & 0x00000080) && mmfar < 0x100) {
        fault_puts("  VERDICT: NULL pointer dereference\r\n");
        fault_puts("  FIX:     Check pointer before use:\r\n");
        fault_puts("           if (ptr != NULL) { *ptr = val; }\r\n");
    }
    /* Bus error NULL */
    else if ((cfsr & 0x00000200) && (cfsr & 0x00008000) && bfar < 0x100) {
        fault_puts("  VERDICT: NULL pointer dereference (bus)\r\n");
        fault_puts("  FIX:     Check pointer before use:\r\n");
        fault_puts("           if (ptr != NULL) { *ptr = val; }\r\n");
    }
    /* Divide by zero */
    else if (cfsr & 0x02000000) {
        fault_puts("  VERDICT: Division by zero\r\n");
        fault_puts("  FIX:     Guard all divisions:\r\n");
        fault_puts("           if (b != 0) { result = a / b; }\r\n");
    }
    /* Bad function pointer */
    else if ((cfsr & 0x00020000) || (cfsr & 0x00000001)) {
        fault_puts("  VERDICT: Invalid function pointer or jump address\r\n");
        fault_puts("  FIX:     Check function pointer initialization\r\n");
        fault_puts("           Ensure LSB=1 for Thumb functions\r\n");
    }
    /* Undefined instruction */
    else if (cfsr & 0x00010000) {
        fault_puts("  VERDICT: Jumped to non-code area\r\n");
        fault_puts("  FIX:     Check function pointers and callbacks\r\n");
        fault_puts("           Check for stack corruption\r\n");
    }
    /* FPU not enabled */
    else if (cfsr & 0x00080000) {
        fault_puts("  VERDICT: FPU not enabled\r\n");
        fault_puts("  FIX:     Add to SystemInit():\r\n");
        fault_puts("           SCB->CPACR |= 0x00F00000;\r\n");
    }
    /* Stack overflow */
    else if ((cfsr & 0x00000010) || (cfsr & 0x00001000)) {
        fault_puts("  VERDICT: Stack overflow\r\n");
        fault_puts("  FIX:     Increase stack size\r\n");
        fault_puts("           Reduce local variable usage\r\n");
        fault_puts("           Check for infinite recursion\r\n");
    }
    /* Unaligned */
    else if (cfsr & 0x01000000) {
        fault_puts("  VERDICT: Unaligned memory access\r\n");
        fault_puts("  FIX:     Use memcpy() for unaligned access\r\n");
        fault_puts("           Add __attribute__((packed)) carefully\r\n");
        fault_puts("           Check pointer casts\r\n");
    }
    /* Vector table */
    else if (hfsr & SCB_HFSR_VECTTBL_Msk) {
        fault_puts("  VERDICT: Vector table corrupted\r\n");
        fault_puts("  FIX:     Check VTOR = "); fault_hex32(SCB->VTOR);
        fault_puts("\r\n");
        fault_puts("           Check for stack overflow into vector table\r\n");
    }
    else {
        fault_puts("  VERDICT: See STEP 2 for details\r\n");
    }

    fault_puts("\r\n");
    fault_puts("**************************************************\r\n");
    fault_puts("**  Run addr2line command from STEP 5          **\r\n");
    fault_puts("**  in your project build directory            **\r\n");
    fault_puts("**************************************************\r\n");
}


/* ================================================================
 * Главная функция дампа HardFault
 *
 * ВАЖНО: Все регистры читаются В САМОМ НАЧАЛЕ
 *        до любых других операций — иначе значения
 *        могут быть искажены
 * ================================================================ */
void HardFault_dump(uint32_t *sp)
{
    /* ---- Читаем ВСЕ регистры немедленно ---- */
    const uint32_t cfsr     = SCB->CFSR;
    const uint32_t hfsr     = SCB->HFSR;
    const uint32_t mmfar    = SCB->MMFAR;
    const uint32_t bfar     = SCB->BFAR;
    const uint32_t afsr     = SCB->AFSR;
    const uint32_t icsr     = SCB->ICSR;
    const uint32_t shcsr    = SCB->SHCSR;
    const uint32_t ccr      = SCB->CCR;
    const uint32_t msp      = __get_MSP();
    const uint32_t psp      = __get_PSP();
    const uint32_t ctrl     = __get_CONTROL();
    const uint32_t ipsr_val = __get_IPSR();
    const uint32_t primask  = __get_PRIMASK();
    const uint32_t basepri  = __get_BASEPRI();
    const uint32_t faultmsk = __get_FAULTMASK();

    /* ---- Стековый фрейм ---- */
    const uint32_t r0  = sp[0];
    const uint32_t r1  = sp[1];
    const uint32_t r2  = sp[2];
    const uint32_t r3  = sp[3];
    const uint32_t r12 = sp[4];
    const uint32_t lr  = sp[5];
    const uint32_t pc  = sp[6];
    const uint32_t psr = sp[7];

   // FaultUART_Init();

    /* ============================================================
     * ЗАГОЛОВОК
     * ============================================================ */
    fault_puts("\r\n");
    fault_puts("**************************************************\r\n");
    fault_puts("**           !!! HARD FAULT !!!               **\r\n");
    fault_puts("**************************************************\r\n");

    /* ============================================================
     * 1. СТЕКОВЫЙ ФРЕЙМ
     * ============================================================ */
    fault_section("1. EXCEPTION STACK FRAME");
    fault_reg("R0  ", r0);
    fault_reg("R1  ", r1);
    fault_reg("R2  ", r2);
    fault_reg("R3  ", r3);
    fault_reg("R12 ", r12);
    fault_reg("LR  ", lr);
    fault_puts("  PC   = "); fault_hex32(pc);
    fault_puts("  <-- faulting instruction\r\n");
    fault_reg("PSR ", psr);
    fault_puts("  PSR flags: ");
    if (psr & (1u<<31)) fault_puts("[N] ");
    if (psr & (1u<<30)) fault_puts("[Z] ");
    if (psr & (1u<<29)) fault_puts("[C] ");
    if (psr & (1u<<28)) fault_puts("[V] ");
    if (psr & (1u<<27)) fault_puts("[Q] ");
    fault_puts("\r\n");
    fault_puts("  Active ISR: "); fault_dec(psr & 0x1FF);
    fault_puts("\r\n");

    /* ============================================================
     * 2. УКАЗАТЕЛИ СТЕКА
     * ============================================================ */
    fault_section("2. STACK POINTERS");
    fault_puts("  SP (active) = "); fault_hex32((uint32_t)sp);
    fault_puts("\r\n");
    fault_reg("MSP      ", msp);
    fault_reg("PSP      ", psp);
    fault_puts("  Active stack: ");
    fault_puts((lr & 0x4) ? "PSP (thread mode)\r\n"
                           : "MSP (handler mode)\r\n");

    /* ============================================================
     * 3. УПРАВЛЕНИЕ ЯДРОМ
     * ============================================================ */
    fault_section("3. CORE CONTROL REGISTERS");
    fault_reg("CONTROL ", ctrl);
    fault_puts("  Privilege: ");
    fault_puts((ctrl & 1) ? "Unprivileged\r\n" : "Privileged\r\n");
    fault_puts("  SP select: ");
    fault_puts((ctrl & 2) ? "PSP\r\n" : "MSP\r\n");
    fault_reg("IPSR    ", ipsr_val);
    fault_puts("  Exception: "); fault_dec(ipsr_val & 0x1FF);
    fault_puts("  IRQn: ");      fault_dec((int32_t)(ipsr_val & 0x1FF) - 16);
    fault_puts("\r\n");
    fault_reg("PRIMASK ", primask);
    fault_puts("  IRQs: ");
    fault_puts(primask ? "DISABLED\r\n" : "enabled\r\n");
    fault_reg("BASEPRI ", basepri);
    fault_reg("FAULTMSK", faultmsk);

    /* ============================================================
     * 4. CCR
     * ============================================================ */
    fault_section("4. CCR (Configuration and Control)");
    fault_reg("CCR", ccr);
    /* Кэш */
    fault_flag(ccr, SCB_CCR_BP_Msk,
               "BP         ", "Branch prediction enabled");
    fault_flag(ccr, SCB_CCR_IC_Msk,
               "IC         ", "Instruction cache enabled");
    fault_flag(ccr, SCB_CCR_DC_Msk,
               "DC         ", "Data cache enabled");

    /* Stack overflow */
    fault_flag(ccr, SCB_CCR_STKOFHFNMIGN_Msk,
               "STKOFHFNMIGN", "Stack overflow ignored in HardFault/NMI");
    fault_flag(ccr, SCB_CCR_BFHFNMIGN_Msk,
               "BFHFNMIGN  ", "Bus fault ignored in HardFault/NMI");

    /* Трапы */
    fault_flag(ccr, SCB_CCR_DIV_0_TRP_Msk,
               "DIV_0_TRP  ", "Divide by zero trapping enabled");
    fault_flag(ccr, SCB_CCR_UNALIGN_TRP_Msk,
               "UNALIGN_TRP", "Unaligned access trapping enabled");

    /* Доступ */
    fault_flag(ccr, SCB_CCR_USERSETMPEND_Msk,
               "USERSETMPEND", "Unprivileged access to STIR enabled");

    /* ============================================================
     * 5. SCB FAULT REGISTERS
     * ============================================================ */
    fault_section("5. SCB FAULT REGISTERS (raw)");
    fault_reg("CFSR ", cfsr);
    fault_reg("HFSR ", hfsr);
    fault_reg("MMFAR", mmfar);
    fault_reg("BFAR ", bfar);
    fault_reg("AFSR ", afsr);
    fault_reg("ICSR ", icsr);
    fault_reg("SHCSR", shcsr);

    /* ============================================================
     * 6. HFSR DECODE
     * ============================================================ */
    fault_section("6. HFSR DECODE");
    fault_flag(hfsr, SCB_HFSR_VECTTBL_Msk,
               "VECTTBL ",
               "Bus fault on vector table read");
    fault_flag(hfsr, SCB_HFSR_FORCED_Msk,
               "FORCED  ",
               "Escalated fault — see CFSR below");
    fault_flag(hfsr, SCB_HFSR_DEBUGEVT_Msk,
               "DEBUGEVT",
               "Debug event (BKPT without debugger?)");
    if (!(hfsr & (SCB_HFSR_VECTTBL_Msk |
                  SCB_HFSR_FORCED_Msk   |
                  SCB_HFSR_DEBUGEVT_Msk)))
        fault_puts("  (no HFSR bits set)\r\n");

    /* ============================================================
     * 7. CFSR DECODE
     * ============================================================ */
    fault_section("7. CFSR DECODE");

    fault_puts("  -- MemManage Fault [7:0] --\r\n");
    fault_flag(cfsr, 0x00000001, "IACCVIOL ",
               "Instruction access violation (MPU)");
    fault_flag(cfsr, 0x00000002, "DACCVIOL ",
               "Data access violation (MPU)");
    fault_flag(cfsr, 0x00000008, "MUNSTKERR",
               "MemManage fault on unstacking");
    fault_flag(cfsr, 0x00000010, "MSTKERR  ",
               "MemManage fault on stacking");
    if (cfsr & 0x00000080) {
        fault_puts("  [!] MMARVALID: MMFAR valid -> ");
        fault_hex32(mmfar);
        if (mmfar == 0) fault_puts("  [!!] NULL pointer!");
        fault_puts("\r\n");
    }

    fault_puts("  -- Bus Fault [15:8] --\r\n");
    fault_flag(cfsr, 0x00000100, "IBUSERR    ",
               "Instruction bus error");
    fault_flag(cfsr, 0x00000200, "PRECISERR  ",
               "Precise data bus error");
    fault_flag(cfsr, 0x00000400, "IMPRECISERR",
               "Imprecise data bus error (PC may be wrong)");
    fault_flag(cfsr, 0x00000800, "UNSTKERR   ",
               "Bus fault on unstacking");
    fault_flag(cfsr, 0x00001000, "STKERR     ",
               "Bus fault on stacking");
    if (cfsr & 0x00008000) {
        fault_puts("  [!] BFARVALID: BFAR valid -> ");
        fault_hex32(bfar);
        if (bfar == 0) fault_puts("  [!!] NULL pointer!");
        fault_puts("\r\n");
    }

    fault_puts("  -- Usage Fault [31:16] --\r\n");
    fault_flag(cfsr, 0x00010000, "UNDEFINSTR ",
               "Undefined instruction");
    fault_flag(cfsr, 0x00020000, "INVSTATE   ",
               "Invalid EPSR state (LSB=0 in function ptr?)");
    fault_flag(cfsr, 0x00040000, "INVPC      ",
               "Invalid PC on exception return");
    fault_flag(cfsr, 0x00080000, "NOCP       ",
               "No coprocessor (FPU disabled?)");
    fault_flag(cfsr, 0x01000000, "UNALIGNED  ",
               "Unaligned memory access");
    fault_flag(cfsr, 0x02000000, "DIVBYZERO  ",
               "Divide by zero");

    if (!(cfsr))
        fault_puts("  (no CFSR bits set)\r\n");

    /* ============================================================
     * 8. CALL CHAIN
     * ============================================================ */
    fault_call_chain(sp, lr, pc, msp, psp);

    /* ============================================================
     * 9. ROOT CAUSE
     * ============================================================ */
    fault_root_cause(cfsr, hfsr, mmfar, bfar, pc, lr);

    /* ============================================================
     * 10. STACK DUMP
     * ============================================================ */
    fault_dump_stack(sp);

    /* ============================================================
     * ИТОГ — инструкция по отладке
     * ============================================================ */
    fault_puts("\r\n");
    fault_puts("**************************************************\r\n");
    fault_puts("** HOW TO DEBUG:                               **\r\n");
    fault_puts("**  1. Find PC in .map file -> function name   **\r\n");
    fault_puts("**  2. Find LR in .map file -> caller name     **\r\n");
    fault_puts("**  3. Check ROOT CAUSE section                **\r\n");
    fault_puts("**  4. Check stack scan for call chain         **\r\n");
    fault_puts("**  5. Check MMFAR/BFAR for bad address        **\r\n");
    fault_puts("**  6. arm-none-eabi-addr2line -e firmware.elf **\r\n");
    fault_puts("**     -a <PC address>                         **\r\n");
    fault_puts("**************************************************\r\n");

    fault_auto_analysis(sp,
                            cfsr, hfsr,
                            mmfar, bfar,
                            pc, lr,
                            r0, r1, r2, r3,
                            msp, psp);

    while (1);
}


/* ******************************************************************************************
 * Function:       crash_system_div_by_zero,
 * Arguments:      None,
 * Description:    Forces a CPU UsageFault/HardFault exception by performing
 *                 an integer division by zero. The DIV_0_TRP bit is enabled
 *                 in the System Control Block (SCB) Configuration and Control
 *                 Register (CCR) to ensure the Cortex-M core generates a fault
 *                 on divide-by-zero operations. This function is intended for
 *                 validation and unit testing of HardFault/UsageFault handlers.
 * Return:         None.
 *********************************************************************************************/
void crash_system_div_by_zero(void)
{
    // 1. Enable divide-by-zero trapping at Cortex-M core level
    SCB->CCR |= SCB_CCR_DIV_0_TRP_Msk;

    // 2. Volatile variables prevent compiler optimization
    volatile uint32_t numerator = 100;
    volatile uint32_t denominator = 0;
    volatile uint32_t result;

    // 3. Trigger UsageFault -> HardFault
    result = numerator / denominator;

    (void)result; // Execution never reaches this point
}
