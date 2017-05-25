// (c) Tobias Faller 2017

#include <msp430.h>
#include <stdint.h>

#include "inc/uart.h"

#include "uart_p.h"

// ----------------------------------------------------------------------------
// Fields
// ----------------------------------------------------------------------------

static uart_t uart;

// ----------------------------------------------------------------------------
// Methods
// ----------------------------------------------------------------------------

__inline void
uart_init (uint8_t *r_buffer, uint16_t r_size,
           uint8_t *t_buffer, uint16_t t_size)
{
  uart.r_buffer.buffer = r_buffer;
  uart.r_buffer.buffer_size = r_size;
  uart.r_buffer.start = 0;
  uart.r_buffer.fill = 0;

  uart.t_buffer.buffer = t_buffer;
  uart.t_buffer.buffer_size = t_size;
  uart.t_buffer.start = 0;
  uart.t_buffer.fill = 0;

  uart.r_callback = 0;
  uart.t_callback = 0;

  UCA0CTL0 = 0; // Asynchronous UART 8 bit, 1 stop bit and without parity
  UCA0CTL1 = UCSSEL_2 // SMCLK as source
      | UCBRKIE; // Allow break character for interrupt
  UCA0MCTL = 0;
  UCA0STAT = 0;
  UCA0IRTCTL = 0;
  UCA0IRRCTL = 0;
  UCA0ABCTL = 0;
  IE2 |= UCA0TXIE | UCA0RXIE; // Enable receive / transmit interrupt
}

__inline void
uart_send_next_char (void)
{
  uint8_t next_char = uart.t_buffer.buffer[uart.t_buffer.start];

  uart.t_buffer.start++;
  while (uart.t_buffer.start >= uart.t_buffer.buffer_size)
    uart.t_buffer.start -= uart.t_buffer.buffer_size;

  // Send next character (The interrupt flag is automatically cleared)
  UCA0TXBUF = next_char;
}

__inline void
uart_set_transmit_callback (void (*callback)(uart_buffer_t *buffer))
{
  uart.t_callback = callback;
}

__inline void
uart_set_receive_callback (void (*callback)(uart_buffer_t *buffer))
{
  uart.r_callback = callback;
}

__inline uint8_t*
uart_get_character (uart_buffer_t *buffer, uint16_t index)
{
  index += buffer->start;
  while (index >= buffer->buffer_size)
    index -= buffer->buffer_size;

  return buffer->buffer + index;
}

__inline uint8_t*
uart_get_next_character (uart_buffer_t *buffer, uint8_t *p)
{
  p++; // Forward pointer

  // Check upper bound
  if (p >= (buffer->buffer + buffer->buffer_size))
    p -= buffer->buffer_size;

  return p;
}

#pragma vector=USCIAB0TX_VECTOR
__interrupt void
uart_int_tx (void)
{
  // Check for more data to transmit
  if (uart.t_buffer.fill != 0) {
    uart_send_next_char();
  } else {
    // Clear interrupt flag
    IFG2 &= ~UCA0TXIFG;

    if (uart.t_callback != 0)
      uart.t_callback(&uart.t_buffer);
  }
}

#pragma vector=USCIAB0RX_VECTOR
__interrupt void
uart_int_rx (void)
{
  // Read character (The interrupt flag is automatically cleared)
  uint8_t received_char = UCA0RXBUF;

  if (uart.r_buffer.fill >= uart.r_buffer.buffer_size) {
    // An buffer overflow occurred -> Discard all old data
   uart.r_buffer.start = 0;
   uart.r_buffer.fill = 0;
  }

  uint8_t *p = uart_get_character(&uart.r_buffer, uart.r_buffer.fill);
  *p = received_char;

  uart.r_buffer.fill++;

  if (uart.r_callback != 0)
    uart.r_callback(&uart.r_buffer);
}
