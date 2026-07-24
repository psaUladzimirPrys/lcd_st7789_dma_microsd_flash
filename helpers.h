#ifndef __ALS_HELPERS_H__
#define __ALS_HELPERS_H__

#include <stdint.h>
#include <string.h>
#include <stdbool.h>


bool uint_to_ascii(uint32_t val, uint8_t width, uint8_t *out_ptr);
bool ascii_to_uint(const uint8_t *in_ptr, uint8_t len, uint32_t *out_ptr);

//------------------------------------------------

void get_mcu_reset_cause (void);
void print_mcu_reset_cause(void);

//----------------------------------------------

void FaultUART_Init(void);
void fault_putchar(char c);
void fault_puts(const char *s);
void fault_hex32(uint32_t val);
void fault_dec(int32_t val);
void fault_reg(const char *name, uint32_t val);
void fault_flag(uint32_t reg, uint32_t mask, const char *name, const char *desc);
void fault_separator(void);
void fault_section(const char *title);
void fault_dump_stack(uint32_t *sp);

void fault_call_chain(uint32_t *sp,
                              uint32_t  lr,
                              uint32_t  pc,
                              uint32_t  msp,
                              uint32_t  psp);

void fault_root_cause(uint32_t cfsr,
                              uint32_t hfsr,
                              uint32_t mmfar,
                              uint32_t bfar,
                              uint32_t pc,
                              uint32_t lr);

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
                                 uint32_t  psp);
void HardFault_dump(uint32_t *sp);

#endif
