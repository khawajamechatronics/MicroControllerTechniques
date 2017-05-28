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
__inline void
uart_init (uint8_t *r_buffer, uint16_t r_size,
           uint8_t *t_buffer, uint16_t t_size);

/**
 * Puts the character into the queue.
 * If the queue is full the execution is interrupted and the
 * CPU is put into sleep mode until the queue ist empty.
 *
 * @param c The character to send
 */
__inline void
uart_send (uint8_t c);

/**
 * Sets the callback function which is called when data was received.
 * Pass 0 to this function to deactivate the callback.
 *
 * @param callback The callback function to notify
 */
__inline void
uart_set_receive_callback (void (*callback)(uart_buffer_t *buffer));

#endif // !__UART_H
