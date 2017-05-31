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

#define UART_ESC 0x1B

// See documentation page 424
#ifdef UART_1MHZ
#ifdef UART_115K
#define UART_PRESCALER 8
#define UART_MODULATION 6
#endif

#ifdef UART_38K
#define UART_PRESCALER 26
#define UART_MODULATION 0
#endif

#ifdef UART_9K
#define UART_PRESCALER 104
#define UART_MODULATION 1
#endif
#endif

#ifdef UART_1048KHZ
#ifdef UART_115K
#define UART_PRESCALER 9
#define UART_MODULATION 1
#endif

#ifdef UART_38K
#define UART_PRESCALER 27
#define UART_MODULATION 2
#endif

#ifdef UART_9K
#define UART_PRESCALER 109
#define UART_MODULATION 2
#endif
#endif

// ----------------------------------------------------------------------------
// Types
// ----------------------------------------------------------------------------

typedef struct {
  uart_buffer_t r_buffer;
  uart_buffer_t t_buffer;

  bool_t t_wait;

  bool_t (*r_callback)(uart_buffer_t *buffer);
} uart_t;

#endif // !__UART_P_H
