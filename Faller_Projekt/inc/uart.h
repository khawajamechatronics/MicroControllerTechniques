// (c) Tobias Faller 2017

#ifndef __UART_H
#define __UART_H

#include <stdint.h>

// ----------------------------------------------------------------------------
// Types
// ----------------------------------------------------------------------------

typedef struct {
  uint8_t *buffer;
  uint16_t buffer_size;
  uint16_t start;
  uint16_t fill;
} uart_buffer_t;

// ----------------------------------------------------------------------------
// Methods
// ----------------------------------------------------------------------------

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
 * Sets the callback function which is called when the transmit buffer
 * was cleared.
 * Pass 0 to this function to deactivate the callback.
 *
 * @param callback The callback function to notify
 */
__inline void
uart_set_transmit_callback (void (*callback)(uart_buffer_t *buffer));

/**
 * Sets the callback function which is called when data was received.
 * Pass 0 to this function to deactivate the callback.
 *
 * @param callback The callback function to notify
 */
__inline void
uart_set_receive_callback (void (*callback)(uart_buffer_t *buffer));

/**
 * Returns the pointer to the character at the specified index of the buffer.
 *
 * @param buffer The buffer to use
 * @param index The index to fetch
 * @return The pointer to the selected element
 */
__inline uint8_t*
uart_get_character (uart_buffer_t *buffer, uint16_t index);

/**
 * Returns the pointer to the next character.
 * This method only corrects the pointer if it exceeds the upper bounds.
 * The fill size of the buffer has to be monitored by the user.
 *
 * @param buffer The buffer to use
 * @param p The pointer to the current location
 * @return The pointer to the next buffer location
 */
__inline uint8_t*
uart_get_next_character (uart_buffer_t *buffer, uint8_t *p);

#endif // !__UART_H
