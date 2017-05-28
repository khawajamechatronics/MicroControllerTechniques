// (c) Tobias Faller 2017

#ifndef __UART_BUFFER_H
#define __UART_BUFFER_H

#include <stdint.h>

#include "def.h"
#include "config.h"

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
 * Returns 1 if the given buffer is full.
 *
 * @param buffer The buffer to check
 * @return true if the buffer is full
 */
__inline bool_t
uart_buffer_is_full (uart_buffer_t *buffer);

/**
 * Returns 1 if the given buffer is empty.
 *
 * @param buffer The buffer to check
 * @return true if the buffer is empty
 */
__inline bool_t
uart_buffer_is_empty (uart_buffer_t *buffer);

/**
 * Appends the next character to the end of the buffer.
 *
 * @param buffer The buffer to use
 * @param value The value to append
 */
__inline void
uart_buffer_enqueue (uart_buffer_t *buffer, uint8_t value);

/**
 * Returns the first value in the buffer.
 *
 * @param buffer The buffer to use
 * @return The first value in the buffer
 */
__inline uint8_t
uart_buffer_dequeue (uart_buffer_t *buffer);

/**
 * Returns the pointer to the character at the specified index of the buffer.
 *
 * @param buffer The buffer to use
 * @param index The index to fetch
 * @return The pointer to the selected element
 */
__inline uint8_t*
uart_buffer_get_at (uart_buffer_t *buffer, uint16_t index);

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
uart_buffer_get_next (uart_buffer_t *buffer, uint8_t *p);

/**
 * Returns the number of characters in the buffer.
 *
 * @param buffer The buffer to use
 * @return The number of characters in the buffer
 */
__inline uint16_t
uart_buffer_get_fill (uart_buffer_t *buffer);

#endif // !__UART_BUFFER_H
