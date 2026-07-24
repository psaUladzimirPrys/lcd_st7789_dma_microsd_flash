/***************************************************************************//**
 * @file
 * @brief IO Stream UART Component.
 *******************************************************************************
 * # License
 * <b>Copyright 2019 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/
// Define module name for Power Manager debug feature
#define CURRENT_MODULE_NAME    "IOSTREAM_UART"

#if defined(SL_COMPONENT_CATALOG_PRESENT)
#include "sl_component_catalog.h"
#endif

#include "sl_status.h"
#include "sl_iostream.h"
#include "sli_iostream.h"
#include "sl_iostream_uart.h"
#include "sli_iostream_uart.h"
#include "sl_atomic.h"
#include "sl_slist.h"
#include "sl_string.h"
#include "em_device.h"

#if (defined(SL_CATALOG_KERNEL_PRESENT))
#include "cmsis_os2.h"
#include "sl_cmsis_os2_common.h"
#endif

#if (defined(SL_CATALOG_POWER_MANAGER_PRESENT))
#include "sl_power_manager.h"
#endif

#include <string.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "dmadrv.h"
#include "em_device.h"
#include "sl_core.h"
#include "sl_assert.h"

#if !defined(DMA_PRESENT) && !defined(LDMA_PRESENT)
#error Missing (L)DMA peripheral
#endif

/*******************************************************************************
 *********************************   DEFINES   *********************************
 ******************************************************************************/

#define MAX_RX_FIFO_DEPTH 16  ///< Used to limit iterations in RX DMA IRQ handler
#define RX_DATA_AVAILABLE_FLAG  1

/*******************************************************************************
 **************************** LOCAL VARIABLES **********************************
 ******************************************************************************/
// (Note: null_byte removed - no longer needed for DMA data detection)

/*******************************************************************************
 *********************   LOCAL FUNCTION PROTOTYPES   ***************************
 ******************************************************************************/
static sl_status_t uart_deinit(void *stream);

static sl_status_t uart_write(void *context,
                              const void *buffer,
                              size_t buffer_length);

static sl_status_t uart_write_async(void *context,
                                    sli_iostream_write_async_op_t *async_op);

static sl_status_t uart_read(void *context,
                             void *buffer,
                             size_t buffer_length,
                             size_t *bytes_read);

static void set_auto_cr_lf(void *context,
                           bool on);

static bool get_auto_cr_lf(void *context);

#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
#if !defined(SL_CATALOG_KERNEL_PRESENT)
static bool wakeup_from_rx(const sl_iostream_uart_context_t *uart_context);

static sl_power_manager_on_isr_exit_t sleep_on_isr_exit(void *context);
#endif

static void set_rx_energy_mode_restriction(void *context,
                                           bool on);

static bool get_rx_energy_mode_restriction(void *context);
#endif

#if defined(SL_CATALOG_KERNEL_PRESENT)
static void set_read_block(void *context,
                           bool on);

static bool get_read_block(void *context);
#endif

static sl_status_t nolock_uart_write(void *context,
                                     const void *buffer,
                                     size_t buffer_length);

static inline bool rx_buffer_empty(const sl_iostream_uart_context_t *uart_context);


static void update_ring_buffer(sl_iostream_uart_context_t *uart_context, size_t read_size);

static size_t read_rx_buffer(sl_iostream_uart_context_t * uart_context,
                             uint8_t * buffer,
                             size_t buffer_len);

static void __uart_async_start_write(sli_iostream_write_async_op_t *async_op);

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

/***************************************************************************//**
 * UART Stream init
 ******************************************************************************/
sl_status_t sli_iostream_uart_context_init(sl_iostream_uart_t *uart,
                                           sl_iostream_uart_context_t *context,
                                           sl_iostream_uart_config_t *config)
{
  Ecode_t ecode;
  unsigned int allocated_channel;

  // Configure iostream struct and context
  memset(context, 0, sizeof(*context));
  context->rx_dma.cfg = config->rx_dma_cfg;
  context->tx_dma.cfg = config->tx_dma_cfg;
  context->rx_buffer = config->rx_buffer;
  context->rx_buffer_len = config->rx_buffer_length;
  context->rx_read_ptr = context->rx_buffer;
  context->rx_write_ptr = context->rx_buffer;    // Initialize head pointer
  context->lf_to_crlf = config->lf_to_crlf;

  context->rx_data_pending = false;             // No data initially
  context->uart_periph = config->uart_periph;


#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
  context->enable_high_frequency = config->enable_high_frequency;
#endif
  uart->stream.context = context;
  uart->stream.write = uart_write;
  uart->stream.write_async = uart_write_async;
  uart->stream.read = uart_read;
  uart->set_auto_cr_lf = set_auto_cr_lf;
  uart->get_auto_cr_lf = get_auto_cr_lf;
  uart->deinit = uart_deinit;

  // Init the LDMA
  ecode = DMADRV_Init();
  if (ecode != ECODE_OK && ecode != ECODE_EMDRV_DMADRV_ALREADY_INITIALIZED) {
    return SL_STATUS_INITIALIZATION;
  }
  // RX is now software-managed, no DMA allocation needed
  // Allocate the Tx LDMA channel
  ecode = DMADRV_AllocateChannel(&allocated_channel, NULL);
  if (ecode != ECODE_OK) {
    return SL_STATUS_INITIALIZATION;
  }
  context->tx_dma.channel = (uint8_t)allocated_channel;

#if defined(SL_CATALOG_KERNEL_PRESENT)
  uart->set_read_block = set_read_block;
  uart->get_read_block = get_read_block;
  context->block = true;

  osMutexAttr_t m_attr;
  m_attr.name = "Read Lock";
  m_attr.attr_bits = 0u;
  m_attr.cb_mem = context->read_lock_cb;
  m_attr.cb_size = osMutexCbSize;
  context->read_lock = osMutexNew(&m_attr);
  EFM_ASSERT(context->read_lock != NULL);

  m_attr.name = "Write Lock";
  m_attr.attr_bits = 0u;
  m_attr.cb_mem = context->write_lock_cb;
  context->write_lock = osMutexNew(&m_attr);
  EFM_ASSERT(context->write_lock != NULL);

  osEventFlagsAttr_t f_attr;
  f_attr.name = "RX Data Available Flag";
  f_attr.attr_bits = 0u;
  f_attr.cb_mem = context->rx_data_flag_cb;
  f_attr.cb_size = osEventFlagsCbSize;
  context->rx_data_flag = osEventFlagsNew(&f_attr);
  EFM_ASSERT(context->rx_data_flag != NULL);

#endif

#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
  #if !defined(SL_CATALOG_KERNEL_PRESENT)
  uart->sleep_on_isr_exit = sleep_on_isr_exit;
  context->sleep = SL_POWER_MANAGER_IGNORE;
  #endif // SL_CATALOG_KERNEL_PRESENT

  uart->set_rx_energy_mode_restriction = set_rx_energy_mode_restriction;
  uart->get_rx_energy_mode_restriction = get_rx_energy_mode_restriction;
  context->em_req_added = false;
  context->tx_idle = true;
  set_rx_energy_mode_restriction(context, config->rx_when_sleeping);
  NVIC_ClearPendingIRQ(config->uart_periph->tx_irq_number);
  NVIC_EnableIRQ(config->uart_periph->tx_irq_number);
#endif // SL_CATALOG_POWER_MANAGER_PRESENT

  NVIC_ClearPendingIRQ(config->uart_periph->rx_irq_number);
  NVIC_EnableIRQ(config->uart_periph->rx_irq_number);

  sl_slist_init(&context->pending_write_ops);

  sl_iostream_set_system_default(&uart->stream);

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Subscribe to receive a callback when new data is received.
 ******************************************************************************/
sl_status_t sli_iostream_uart_subscribe_to_new_data(sl_iostream_uart_t *iostream_uart,
                                                    sl_iostream_uart_new_data_callback_t callback,
                                                    void *callback_data)
{
  CORE_DECLARE_IRQ_STATE;

  if (iostream_uart == NULL || callback == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  sl_iostream_uart_context_t *uart_context = (sl_iostream_uart_context_t *)iostream_uart->stream.context;

  if (uart_context->rx_subscriber.callback) {
    return SL_STATUS_BUSY;
  }

  CORE_ENTER_ATOMIC();
  uart_context->rx_subscriber.callback_data = callback_data;
  uart_context->rx_subscriber.callback = callback;
  CORE_EXIT_ATOMIC();

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Unsubscribe to the new data callback.
 ******************************************************************************/
sl_status_t sli_iostream_uart_unsubscribe_to_new_data(sl_iostream_uart_t *iostream_uart)
{
  CORE_DECLARE_IRQ_STATE;

  if (iostream_uart == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  sl_iostream_uart_context_t *uart_context = (sl_iostream_uart_context_t *)iostream_uart->stream.context;

  CORE_ENTER_ATOMIC();
  uart_context->rx_subscriber.callback = NULL;
  uart_context->rx_subscriber.callback_data = NULL;
  CORE_EXIT_ATOMIC();

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Set new data detect to wake from sleep.
 ******************************************************************************/
void sl_iostream_uart_prepare_for_sleep(sl_iostream_uart_t *iostream_uart)
{
   (void)iostream_uart;
   return;
}

/***************************************************************************//**
 * Ensure new data detect is cleared when waking from sleep and data is available.
 ******************************************************************************/
void sl_iostream_uart_wakeup(sl_iostream_uart_t *iostream_uart)
{
  (void)iostream_uart;
  return;
}

#if defined(SL_CATALOG_POWER_MANAGER_PRESENT) && !defined(SL_CATALOG_KERNEL_PRESENT)
/**************************************************************************//**
 * Check if MCU was woken up by new data on UART.
 *****************************************************************************/
static bool wakeup_from_rx(const sl_iostream_uart_context_t *uart_context)
{
  /* IRQ RX: wake if ring has unread data (ISR may have just filled it). */
  return !rx_buffer_empty(uart_context);
}

/**************************************************************************//**
 * @brief On ISR exit
 *****************************************************************************/
static sl_power_manager_on_isr_exit_t sleep_on_isr_exit(void *context)
{
  if (context == NULL) {
    return SL_POWER_MANAGER_IGNORE;
  }

  sl_iostream_uart_context_t *uart_context = (sl_iostream_uart_context_t *)context;

  if (wakeup_from_rx(context)) {
    // MCU was woken-up from RX byte, wakeup from sleep.
    uart_context->sleep = SL_POWER_MANAGER_WAKEUP;
  }

  if ((uart_context->tx_idle) && (uart_context->sleep == SL_POWER_MANAGER_SLEEP)) {
    sl_power_manager_on_isr_exit_t sleep = uart_context->sleep;
    uart_context->sleep = SL_POWER_MANAGER_IGNORE;
    return sleep;
  } else if ((uart_context->sleep == SL_POWER_MANAGER_WAKEUP)) {
    sl_power_manager_on_isr_exit_t sleep = uart_context->sleep;
    uart_context->sleep = SL_POWER_MANAGER_IGNORE;
    return sleep;
  } else {
    return SL_POWER_MANAGER_IGNORE;
  }
}
#endif

/**************************************************************************//**
 * Set LF to CRLF conversion
 *****************************************************************************/
static void set_auto_cr_lf(void *context,
                           bool on)
{
  sl_iostream_uart_context_t *uart_context = (sl_iostream_uart_context_t *)context;

  sl_atomic_store(uart_context->lf_to_crlf, on);
}

/**************************************************************************//**
 * Get LF to CRLF conversion
 *****************************************************************************/
static bool get_auto_cr_lf(void *context)
{
  sl_iostream_uart_context_t *uart_context = (sl_iostream_uart_context_t *)context;
  bool conversion;

  sl_atomic_load(conversion, uart_context->lf_to_crlf);

  return conversion;
}

#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
/**************************************************************************//**
 * Set Rx when sleeping
 *****************************************************************************/
static void set_rx_energy_mode_restriction(void *context,
                                           bool on)
{
  sl_iostream_uart_context_t *uart_context = (sl_iostream_uart_context_t *)context;
  CORE_DECLARE_IRQ_STATE;

  CORE_ENTER_ATOMIC();
  if (on
      && !uart_context->em_req_added
      && uart_context->enable_high_frequency) {
    sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);
    uart_context->em_req_added = true;
  } else if (!on
             && uart_context->em_req_added) {
    sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);
    uart_context->em_req_added = false;
  }
  CORE_EXIT_ATOMIC();
}
#endif

#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
/**************************************************************************//**
 * Get Rx when sleeping
 *****************************************************************************/
static bool get_rx_energy_mode_restriction(void *context)
{
  sl_iostream_uart_context_t *uart_context = (sl_iostream_uart_context_t *)context;
  bool em_req_added;

  sl_atomic_load(em_req_added, uart_context->em_req_added);
  return em_req_added;
}
#endif

#if (defined(SL_CATALOG_KERNEL_PRESENT))
/**************************************************************************//**
 * Set read blocking mode
 *****************************************************************************/
static void set_read_block(void *context,
                           bool on)
{
  sl_iostream_uart_context_t *uart_context = (sl_iostream_uart_context_t *)context;
  bool block;
  osKernelState_t state;
  osStatus_t status;
  uint32_t set_flags;

  sl_atomic_load(block, uart_context->block);
  state = osKernelGetState();
  if (state == osKernelRunning) {
    // When re-enabling block mode, re-initialize the event flag to reflect the current state
    if (on == true && block == false) {
      // Read-signal should have been deinit when non-blocking
      EFM_ASSERT(uart_context->rx_data_flag == NULL);

      // Init the rx_data_flag boolean event flag
      osEventFlagsAttr_t f_attr;
      f_attr.name = "RX Data Available Flag";
      f_attr.attr_bits = 0u;
      f_attr.cb_mem = uart_context->rx_data_flag_cb;
      f_attr.cb_size = osEventFlagsCbSize;
      uart_context->rx_data_flag = osEventFlagsNew(&f_attr);
      EFM_ASSERT(uart_context->rx_data_flag != NULL);

      // Set the event flag to reflect the current state
      if (!rx_buffer_empty(uart_context)) {
        set_flags = osEventFlagsSet(uart_context->rx_data_flag, RX_DATA_AVAILABLE_FLAG);
        EFM_ASSERT(set_flags == RX_DATA_AVAILABLE_FLAG);
      }
    }
    // When disabling block mode, deinit the rx_data_flag event flag
    else if (on == false && block == true) {
      status = osEventFlagsDelete(uart_context->rx_data_flag);
      EFM_ASSERT(status == osOK);
      uart_context->rx_data_flag = NULL;
    }
  }
  // Set the block context variable
  sl_atomic_store(uart_context->block, on);
}
#endif

#if (defined(SL_CATALOG_KERNEL_PRESENT))
/**************************************************************************//**
 * Get read blocking mode
 *****************************************************************************/
static bool get_read_block(void *context)
{
  sl_iostream_uart_context_t *uart_context = (sl_iostream_uart_context_t *)context;
  bool block;

  sl_atomic_load(block, uart_context->block);
  return block;
}
#endif

#if defined(SL_CATALOG_POWER_MANAGER_PRESENT) && !defined(SL_IOSTREAM_UART_FLUSH_TX_BUFFER)
/**************************************************************************//**
 * Signal transmit complete
 *****************************************************************************/
void sli_uart_txc(void *context)
{
  sl_iostream_uart_context_t *uart_context = (sl_iostream_uart_context_t *)context;
  CORE_DECLARE_IRQ_STATE;

  CORE_ENTER_ATOMIC();
  if (uart_context->tx_idle == false) {
    EFM_ASSERT(uart_context->uart_periph->tx_completed != NULL);
    uart_context->uart_periph->tx_completed(context, false);
    uart_context->tx_idle = true;
    sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);
#if !defined(SL_CATALOG_KERNEL_PRESENT)
    uart_context->sleep = SL_POWER_MANAGER_SLEEP;
#endif
  }
  CORE_EXIT_ATOMIC();
}
#endif

/*******************************************************************************
 **************************   LOCAL FUNCTIONS   ********************************
 ******************************************************************************/

/***************************************************************************//**
 * UART Stream De-init
 ******************************************************************************/
static sl_status_t uart_deinit(void *stream)
{
  sl_iostream_uart_t *uart = (sl_iostream_uart_t *)stream;
  sl_iostream_uart_context_t *uart_context = (sl_iostream_uart_context_t *)uart->stream.context;
  sl_iostream_t *default_stream;
  sl_status_t status = SL_STATUS_OK;
  Ecode_t ecode = ECODE_OK;

  if (uart_context->async_tx_mode) {
    EFM_ASSERT(false);
    return SL_STATUS_NOT_SUPPORTED;
  }

  sli_iostream_uart_unsubscribe_to_new_data(uart);

#if (defined(SL_CATALOG_KERNEL_PRESENT))
  if (osKernelGetState() == osKernelRunning) {
    // Acquire locks to ensure no others task try to perform operation on the stream at sametime
    EFM_ASSERT(osMutexAcquire(uart_context->write_lock, osWaitForever) == osOK);  // If deinit is called twice in a
                                                                                  // row, the assert will trigger
    // Bypass lock if we print before the kernel is running
    EFM_ASSERT(osMutexAcquire(uart_context->read_lock, osWaitForever) == osOK);   // If deinit is called twice in a
                                                                                  //row, the assert will trigger
  }
#endif

  default_stream = sl_iostream_get_default();

  // Check if uart stream is the default and if it's the case,
  // remove it's reference as the default
  if ((sl_iostream_uart_t*)default_stream == uart) {
    sl_iostream_set_system_default(NULL);
  }

  NVIC_ClearPendingIRQ(uart_context->uart_periph->rx_irq_number);
  NVIC_DisableIRQ(uart_context->uart_periph->rx_irq_number);

#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
  NVIC_ClearPendingIRQ(uart_context->uart_periph->tx_irq_number);
  NVIC_DisableIRQ(uart_context->uart_periph->tx_irq_number);
#endif

#if defined(SL_CATALOG_KERNEL_PRESENT)
  // Delete Kernel synchronization objects.
  if (uart_context->block) {
    status = osEventFlagsDelete(uart_context->rx_data_flag);
    EFM_ASSERT(status == osOK);
  }

  status = osMutexDelete(uart_context->read_lock);
  EFM_ASSERT(status == osOK);

  status = osMutexDelete(uart_context->write_lock);
  EFM_ASSERT(status == osOK);
#endif

  // RX DMA no longer used - it's now software-managed via callback
  // TX DMA remains managed via DMADRV
  
  // Try to deinit the DMADRV (if no other users)
  ecode = DMADRV_DeInit();
  EFM_ASSERT(ecode == ECODE_OK || ecode == ECODE_EMDRV_DMADRV_IN_USE);

  // Clear iostream struct and context
  uart->stream.context = NULL;
  uart->stream.write = NULL;
  uart->stream.read = NULL;
  uart->set_auto_cr_lf = NULL;
  uart->get_auto_cr_lf = NULL;

  status = uart_context->uart_periph->deinit(uart_context);

  return status;
}
 

/***************************************************************************//**
 * Internal stream write implementation
 ******************************************************************************/
static sl_status_t nolock_uart_write(void *context,
                                     const void *buffer,
                                     size_t buffer_length)
{
  sl_iostream_uart_context_t *uart_context = (sl_iostream_uart_context_t *)context;
  char *c = (char *)buffer;
  bool lf_to_crlf = false;
  sl_status_t status = SL_STATUS_FAIL;

  sl_atomic_load(lf_to_crlf, uart_context->lf_to_crlf);

#if defined(SL_CATALOG_POWER_MANAGER_PRESENT) && !defined(SL_IOSTREAM_UART_FLUSH_TX_BUFFER)
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_ATOMIC();
  if (uart_context->tx_idle == true && uart_context->enable_high_frequency) {
    uart_context->tx_idle = false;
    sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);
  }
  CORE_EXIT_ATOMIC();
#endif

  uint32_t i = 0;
  while (i < buffer_length) {
      if (lf_to_crlf == true) {
        if (*c == '\n') {
          status = uart_context->uart_periph->tx(uart_context, '\r');
          if (status != SL_STATUS_OK) {
            return status;
          }
        }
      }
      status = uart_context->uart_periph->tx(uart_context, *c);
      if (status != SL_STATUS_OK) {
        return status;
      }
      c++;
      i++;
  }

#if defined(SL_CATALOG_POWER_MANAGER_PRESENT) && !defined(SL_IOSTREAM_UART_FLUSH_TX_BUFFER)
  uart_context->uart_periph->tx_completed(context, true);
#endif

  return status;
}





/***************************************************************************//**
 * Internal stream write implementation
 ******************************************************************************/
static sl_status_t uart_write(void *context,
                              const void *buffer,
                              size_t buffer_length)
{
  const sl_iostream_uart_context_t *uart_context = (sl_iostream_uart_context_t *)context;

  if (uart_context->async_tx_mode) {
    return SL_STATUS_NOT_AVAILABLE;         // Can't do synchronous calls in async TX mode
  }

#if (defined(SL_CATALOG_KERNEL_PRESENT))
  osStatus_t status;
  if (osKernelGetState() == osKernelRunning) {
    // Bypass lock if we print before the kernel is running
    status = osMutexAcquire(uart_context->write_lock, osWaitForever);

    if (status != osOK) {
      return SL_STATUS_INVALID_STATE;       // Can happen if a task deinit and another try to write at sametime
    }
  }
#endif

  nolock_uart_write(context, buffer, buffer_length);

#if (defined(SL_CATALOG_KERNEL_PRESENT))
  if (osKernelGetState() == osKernelRunning) {
    // Bypass lock if we print before the kernel is running
    status = osMutexRelease(uart_context->write_lock);
    EFM_ASSERT(status == osOK);
  }
#endif
  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Callback function for UART asynchronous TX DMA completion.
 ******************************************************************************/
static bool __uart_async_tx_dma_callback(unsigned int channel, unsigned int sequenceNo, void *arg)
{
  (void)channel;
  (void)sequenceNo;
  sl_slist_node_t *node;
  sli_iostream_write_async_op_t *async_op;
  sli_iostream_write_async_op_t *completed_async_op = (sli_iostream_write_async_op_t *)arg;
  sl_iostream_uart_context_t *uart_context = (sl_iostream_uart_context_t *)completed_async_op->context;

  EFM_ASSERT(uart_context->async_transfer_in_progress);

  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_ATOMIC();
  uart_context->async_transfer_in_progress = false;
  CORE_EXIT_ATOMIC();

  node = sl_slist_pop(&uart_context->pending_write_ops);
  if (node != NULL) {
    async_op = SL_SLIST_ENTRY(node, sli_iostream_write_async_op_t, node);
    __uart_async_start_write(async_op);
  }

  if (completed_async_op->on_write_completed != NULL) {
    completed_async_op->on_write_completed(completed_async_op, SL_STATUS_OK, completed_async_op->on_write_completed_arg);
  }

  return false;
}

/***************************************************************************//**
 * Start UART asynchronous write operation (must be called in atomic section).
 *****************************************************************************/
static void __uart_async_start_write(sli_iostream_write_async_op_t *async_op)
{
  Ecode_t ecode;
  sl_iostream_uart_context_t *uart_context = (sl_iostream_uart_context_t *)async_op->context;

  uart_context->tx_dma.desc = (LDMA_Descriptor_t)
                              IOSTREAM_LDMA_DESCRIPTOR_SINGLE_M2P_BYTE(async_op->buffer,
                                                                       uart_context->tx_dma.cfg.dst,
                                                                       async_op->buffer_length);
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_ATOMIC();
  uart_context->async_transfer_in_progress = true;
  CORE_EXIT_ATOMIC();

  ecode = DMADRV_LdmaStartTransfer(uart_context->tx_dma.channel,
                                   &uart_context->tx_dma.cfg.xfer_cfg,
                                   &uart_context->tx_dma.desc,
                                   __uart_async_tx_dma_callback,
                                   async_op);
  EFM_ASSERT(ecode == ECODE_OK);
}

/***************************************************************************//**
 * Perform UART asynchronous write operation.
 ******************************************************************************/
static sl_status_t uart_write_async(void *context, sli_iostream_write_async_op_t *async_op)
{
  sl_iostream_uart_context_t *uart_context = (sl_iostream_uart_context_t *)context;

  if (!uart_context->async_tx_mode) {
    return SL_STATUS_NOT_AVAILABLE;
  }

  if (async_op->buffer_length > IOSTREAM_LDMA_MAX_XFER_SIZE) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_ATOMIC();
  async_op->context = uart_context;

  if (uart_context->async_transfer_in_progress) {
    sl_slist_push_back(&uart_context->pending_write_ops, &async_op->node);
  } else {
    __uart_async_start_write(async_op);
  }

  CORE_EXIT_ATOMIC();

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Internal stream read implementation
 ******************************************************************************/
static sl_status_t uart_read(void *context,
                             void *buffer,
                             size_t buffer_length,
                             size_t *bytes_read)
{
  sl_iostream_uart_context_t *uart_context = (sl_iostream_uart_context_t *)context;
  size_t rx_len;

  #if (defined(SL_CATALOG_KERNEL_PRESENT))
  osStatus_t status;
  uint32_t set_flags;
  if (osKernelGetState() == osKernelRunning) {
    // Bypass lock if we print before the kernel is running
    status = osMutexAcquire(uart_context->read_lock, osWaitForever);

    if (status != osOK) {
      return SL_STATUS_INVALID_STATE;   // Can happen if a task deinit and another try to read at same time
    }

    // Need to check if data is available, as DMA can be started before the kernel was
    // able to be started, meaning the flag will not be set even when data is available.
    if (uart_context->block) {
      // osFlagsNoClear used to unlock directly if data is still available. Flag cleared
      // when no more data is available to be read.
      set_flags = osEventFlagsWait(uart_context->rx_data_flag, RX_DATA_AVAILABLE_FLAG, osFlagsNoClear, osWaitForever);
      EFM_ASSERT(set_flags == RX_DATA_AVAILABLE_FLAG);
    }
  }
  #endif

  *bytes_read = 0;

  while ((rx_len = read_rx_buffer(uart_context,
                                  ((uint8_t *)buffer + *bytes_read),
                                  (buffer_length - *bytes_read)))) {
    // Read the RX buffer until it has been emptied, or the user buffer
    // has been filled.
    *bytes_read += rx_len;

    if (*bytes_read >= buffer_length) {
      // Exit path in case previous call overflowed.
      break;
    }
  }

  #if (defined(SL_CATALOG_KERNEL_PRESENT))
  if (osKernelGetState() == osKernelRunning) {
    // Bypass lock if we print before the kernel is running
    EFM_ASSERT(osMutexRelease(uart_context->read_lock) == osOK);
  }
  #endif

  if (*bytes_read == 0) {
    return SL_STATUS_EMPTY;
  } else {
    return SL_STATUS_OK;
  }
}

/***************************************************************************//**
 * Check if RX buffer is full (no space for next byte).
 ******************************************************************************/
// Description Unused static function 'rx_buffer_full'  Code Analysis Problem
/*
static inline bool rx_buffer_full(const sl_iostream_uart_context_t *uart_context)
{

  uint8_t *next_write_ptr;
  bool result;
  CORE_DECLARE_IRQ_STATE;

  CORE_ENTER_ATOMIC();
  // Buffer is full when next write position would equal read position
  next_write_ptr = uart_context->rx_write_ptr + 1;

  // Handle wrap-around
  if (next_write_ptr >= (uart_context->rx_buffer + uart_context->rx_buffer_len)) {
    next_write_ptr = uart_context->rx_buffer;
  }
  
  result = (next_write_ptr == uart_context->rx_read_ptr);
  CORE_EXIT_ATOMIC();

  return result;

}
 */
/***************************************************************************//**
 * Check if RX buffer is empty (no data to read).
 ******************************************************************************/
static inline bool rx_buffer_empty(const sl_iostream_uart_context_t *uart_context)
{
  bool result;
  CORE_DECLARE_IRQ_STATE;

  CORE_ENTER_ATOMIC();
  result = (uart_context->rx_write_ptr == uart_context->rx_read_ptr);
  CORE_EXIT_ATOMIC();

  return result;
}

/***************************************************************************//**
 * Get number of bytes available to read in RX buffer.
 ******************************************************************************/
static inline size_t get_bytes_available(const sl_iostream_uart_context_t *uart_context)
{
  uint8_t *write_ptr;
  uint8_t *read_ptr;
  size_t result;
  CORE_DECLARE_IRQ_STATE;
  
  // Read both pointers atomically to ensure consistent snapshot
  CORE_ENTER_ATOMIC();
  write_ptr = uart_context->rx_write_ptr;
  read_ptr = uart_context->rx_read_ptr;
  CORE_EXIT_ATOMIC();
  
  if (write_ptr >= read_ptr) {
    // Simple case: write pointer is ahead of read pointer
    result = (size_t)(write_ptr - read_ptr);
  } else {
    // Wrap-around case: write pointer wrapped to start of buffer
    result = (size_t)((uart_context->rx_buffer + uart_context->rx_buffer_len - read_ptr) 
                    + (write_ptr - uart_context->rx_buffer));
  }
  
  return result;
}

/***************************************************************************//**
 * Update ring buffer read pointer after user consumes data.
 ******************************************************************************/
static void update_ring_buffer(sl_iostream_uart_context_t *uart_context, size_t read_size)
{
  // Verify we have enough data to read
  if (read_size > get_bytes_available(uart_context)) {
    EFM_ASSERT(false);  // Programming error
    return;
  }

  // Advance read pointer (tail) by bytes consumed
  uart_context->rx_read_ptr += read_size;
  
  // Handle wrap-around
  if (uart_context->rx_read_ptr >= (uart_context->rx_buffer + uart_context->rx_buffer_len)) {
    uart_context->rx_read_ptr = uart_context->rx_buffer;
  }

  // Sanity check rx_ptr didn't overflow
  EFM_ASSERT(uart_context->rx_read_ptr < (uart_context->rx_buffer + uart_context->rx_buffer_len));

  // Update data available flag atomically
  if (uart_context->rx_read_ptr == uart_context->rx_write_ptr) {
    // Buffer now empty
    CORE_DECLARE_IRQ_STATE;
    CORE_ENTER_ATOMIC();
    uart_context->rx_data_pending = false;
    CORE_EXIT_ATOMIC();
  }
}

/***************************************************************************//**
 * Tries to read the requested amount of data.
 * Returns the number of bytes read.
 ******************************************************************************/
static size_t read_rx_buffer(sl_iostream_uart_context_t * uart_context,
                             uint8_t * buffer,
                             size_t buffer_len)
{
  size_t bytes_available = 0;
  size_t bytes_to_read = 0;
  uint8_t *read_end;
  CORE_DECLARE_IRQ_STATE;

  if (buffer_len == 0 || buffer == NULL) {
    return 0;
  }

  // Check if buffer is empty
  if (rx_buffer_empty(uart_context)) {
    // RX buffer is empty
    return 0;
  }

  // Get available bytes
  bytes_available = get_bytes_available(uart_context);

  // Limit read size to buffer size
  bytes_to_read = (buffer_len < bytes_available) ? buffer_len : bytes_available;
  
  if (bytes_to_read == 0) {
    return 0;
  }
 
 // Copy data to output buffer
  CORE_ENTER_ATOMIC();
  
  // Calculate read end position
  read_end = uart_context->rx_read_ptr + bytes_to_read;
  
  if (read_end > (uart_context->rx_buffer + uart_context->rx_buffer_len)) {
    // Data wraps around buffer boundary
    size_t first_part = uart_context->rx_buffer + uart_context->rx_buffer_len - uart_context->rx_read_ptr;
    size_t second_part = bytes_to_read - first_part;
    
    memcpy(buffer, uart_context->rx_read_ptr, first_part);
    memcpy(buffer + first_part, uart_context->rx_buffer, second_part);
  } else {
    // Normal copy (no wrap)
    memcpy(buffer, uart_context->rx_read_ptr, bytes_to_read);
  }
  
  CORE_EXIT_ATOMIC(); 
  // Update ring buffer state after reading (clears rx_data_pending atomically)
  update_ring_buffer(uart_context, bytes_to_read);

  return bytes_to_read;
}



