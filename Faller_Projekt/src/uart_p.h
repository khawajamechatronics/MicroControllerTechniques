// (c) Tobias Faller 2017

#ifndef __UART_P_H
#define __UART_P_H

#include <stdint.h>

#include "inc/uart_buffer.h"
#include "inc/uart.h"

// ----------------------------------------------------------------------------
// Definitions
// ----------------------------------------------------------------------------

#define UART_SEND_WAITING 0x01
#define UART_SEND_NOT_WAITING 0x00

// ----------------------------------------------------------------------------
// Types
// ----------------------------------------------------------------------------

typedef struct {
  uart_buffer_t r_buffer;
  uart_buffer_t t_buffer;

  bool_t t_wait;
  void (*r_callback)(uart_buffer_t *buffer);
} uart_t;

#endif // !__UART_P_H
