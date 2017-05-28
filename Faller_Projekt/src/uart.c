// (c) Tobias Faller 2017

#include <msp430.h>
#include <stdint.h>

#include "inc/def.h"
#include "inc/config.h"

#include "inc/uart_buffer.h"
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

  uart.t_wait = UART_SEND_NOT_WAITING;
  uart.r_callback = 0;

  // Enable secondary function on UART pins
  P1SEL |= BIT1 + BIT2;
  P1SEL2 |= BIT1 + BIT2;

  UCA0CTL0 = 0; // Asynchronous UART 8 bit, 1 stop bit and without parity
  UCA0CTL1 = UCSSEL_2 // SMCLK as source
      | UCBRKIE; // Allow break character for interrupt
  UCA0MCTL = 0;
  UCA0STAT = 0;
  UCA0IRTCTL = 0;
  UCA0IRRCTL = 0;
  UCA0ABCTL = 0;
  UCA0BR0 = 8; // Select 8 as prescaler for 115200 baud (page 424)
  UCA0BR1 = 0;
  UCA0MCTL = UCBRS_6; // Select 6 as modulation for 115200 baud

  IE2 |= UCA0TXIE | UCA0RXIE; // Enable receive / transmit interrupt
}

__inline void
uart_set_receive_callback (void (*callback)(uart_buffer_t *buffer))
{
  uart.r_callback = callback;
}

__inline void
uart_send (uint8_t c)
{
  if (uart_buffer_is_full(&uart.t_buffer)) {
    // Enable reentrant interrupts and wait until buffer is empty
    uart.t_wait = UART_SEND_WAITING;
    __bis_SR_register(GIE + CPUOFF);
  }

  if (!(UCA0STAT & UCBUSY)) {
    // Send next character (The interrupt flag is automatically cleared)
    UCA0TXBUF = c;
  } else {
    uart_buffer_enqueue(&uart.t_buffer, c);

    // Did the queue get finished right now?
    if (!(UCA0STAT & UCBUSY)) {
      // Send next character (The interrupt flag is automatically cleared)
      UCA0TXBUF = uart_buffer_dequeue(&uart.t_buffer);
    }
  }
}

__inline void
uart_send_move_to (uint8_t v, uint8_t h)
{
  uart_send(UART_ESC);
  uart_send('[');
  uart_send_number_u8(v, 0);
  uart_send(';');
  uart_send_number_u8(h, 0);
  uart_send('H');
}

__inline void
uart_send_string (char *buffer)
{
  for (; buffer != '\0'; ++buffer)
    uart_send(*buffer);
}

__inline void
uart_send_number_u8 (uint8_t value, bool_t leading_zero)
{
  uint8_t v;

  for (v = '0'; value > 100; value -= 100, ++v);
  if (leading_zero || v != '0')
    uart_send(v);

  for (v = '0'; value > 10; value -= 10, ++v);
  if (leading_zero || v != '0')
    uart_send(v);

  uart_send('0' + v);
}

__inline void
uart_send_number_u16 (uint16_t value, bool_t leading_zero)
{
  uint8_t v;

  for (v = '0'; value > 10000; value -= 10000, ++v);
  if (leading_zero || v != '0')
    uart_send(v);

  for (v = '0'; value > 1000; value -= 1000, ++v);
  if (leading_zero || v != '0')
    uart_send(v);

  for (v = '0'; value > 100; value -= 100, ++v);
  if (leading_zero || v != '0')
    uart_send(v);

  for (v = '0'; value > 10; value -= 10, ++v);
  if (leading_zero || v != '0')
    uart_send(v);

  uart_send('0' + v);
}

#pragma vector=USCIAB0TX_VECTOR
__interrupt void
uart_int_tx (void)
{
  // Check for more data to transmit
  if (!uart_buffer_is_empty(&uart.t_buffer)) {
    // Send next character (The interrupt flag is automatically cleared)
    UCA0TXBUF = uart_buffer_dequeue(&uart.t_buffer);
    return;
  }

  // Clear interrupt flag
  IFG2 &= ~UCA0TXIFG;

  if (uart.t_wait) {
    // CPU is waiting to get activated => activate
    uart.t_wait = UART_SEND_NOT_WAITING;

    // Enable CPU on interrupt exit
    __bic_SR_register_on_exit(CPUOFF + GIE);
  }
}

#pragma vector=USCIAB0RX_VECTOR
__interrupt void
uart_int_rx (void)
{
  // Read character (The interrupt flag is automatically cleared)
  uint8_t received_char = UCA0RXBUF;

  if (uart_buffer_is_full(&uart.r_buffer)) {
    // An buffer overflow occurred -> Discard all old data
   uart.r_buffer.start = 0;
   uart.r_buffer.fill = 0;
  }

  uart_buffer_enqueue(&uart.r_buffer, received_char);

  if (uart.r_callback != 0)
    uart.r_callback(&uart.r_buffer);
}
