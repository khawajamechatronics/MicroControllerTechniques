// (c) Tobias Faller 2017

#ifndef __UART_H
#define __UART_H

#include <stdint.h>

#include "def.h"
#include "config.h"

#include "uart_buffer.h"

/**
 * Initializes the UART connection with the specified buffers.
 *
 * @param r_buffer A pointer to the receive buffer
 * @param r_size The size of the receive buffer
 * @param t_buffer A pointer to the transmit buffer
 * @param t_size The size of the transmit buffer
 */
void
uart_init (uint8_t *r_buffer, uint16_t r_size,
           uint8_t *t_buffer, uint16_t t_size);

/**
 * Puts the character into the queue.
 * If the queue is full the execution is interrupted and the
 * CPU is put into sleep mode until the queue ist empty.
 *
 * @param c The character to send
 */
void
uart_send (uint8_t c);

/**
 * Sends the 8-bit value as ASCII text to the UART interface.
 *
 * @param value The value to send
 * @param leading_zero If leading '0's should be send
 */
void
uart_send_number_u8 (uint8_t value, bool_t leading_zero);

/**
 * Sends the 16-bit value as ASCII text to the UART interface.
 *
 * @param value The value to send
 * @param leading_zero If leading '0's should be send
 */
void
uart_send_number_u16 (uint16_t value, bool_t leading_zero);

/**
 * Sends a clear screen command to UART interface.
 */
void
uart_send_cls (void);

/**
 * Initializes the terminal to a 80 column text mode.
 */
void
uart_send_terminal_init (void);

/**
 * Sends a VT100 command to update the cursor position.
 *
 * @param v The row to move to
 * @param h The horizontal position to move to
 */
void
uart_send_move_to (uint8_t v, uint8_t h);

/**
 * Sends the '\0'-terminated string out to the UART interface.
 *
 * @param buffer The string to send
 */
void
uart_send_string (char *buffer);

/**
 * Sets the callback function which is called when data was received.
 * Pass 0 to this function to deactivate the callback.
 * If the callback returns true the CPU is re-activated from low power mode.
 *
 * @param callback The callback function to notify
 */
void
uart_set_receive_callback (bool_t (*callback)(uart_buffer_t *buffer));

#endif // !__UART_H
