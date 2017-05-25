// (c) Tobias Faller 2017

#ifndef __UART_P_H
#define __UART_P_H

#include <stdint.h>

#include "inc/uart.h"

// ----------------------------------------------------------------------------
// Types
// ----------------------------------------------------------------------------

typedef struct {
  uart_buffer_t r_buffer;
  uart_buffer_t t_buffer;

  void (*t_callback)(uart_buffer_t *buffer);
  void (*r_callback)(uart_buffer_t *buffer);
} uart_t;

// ----------------------------------------------------------------------------
// Methods
// ----------------------------------------------------------------------------

/**
 * Transmits the next character in the transmit buffer.
 */
__inline void
uart_send_next_char (void);

#endif // !__UART_P_H
