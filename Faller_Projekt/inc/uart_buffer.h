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
__attribute__((always_inline))
__inline bool_t
uart_buffer_is_full (uart_buffer_t *buffer);

/**
 * Returns 1 if the given buffer is empty.
 *
 * @param buffer The buffer to check
 * @return true if the buffer is empty
 */
__attribute__((always_inline))
__inline bool_t
uart_buffer_is_empty (uart_buffer_t *buffer);

/**
 * Appends the next character to the end of the buffer.
 *
 * @param buffer The buffer to use
 * @param value The value to append
 */
__attribute__((always_inline))
__inline void
uart_buffer_enqueue (uart_buffer_t *buffer, uint8_t value);

/**
 * Returns the first value in the buffer.
 *
 * @param buffer The buffer to use
 * @return The first value in the buffer
 */
__attribute__((always_inline))
__inline uint8_t
uart_buffer_dequeue (uart_buffer_t *buffer);

/**
 * Returns the pointer to the character at the specified index of the buffer.
 *
 * @param buffer The buffer to use
 * @param index The index to fetch
 * @return The pointer to the selected element
 */
__attribute__((always_inline))
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
__attribute__((always_inline))
__inline uint8_t*
uart_buffer_get_next (uart_buffer_t *buffer, uint8_t *p);

/**
 * Returns the number of characters in the buffer.
 *
 * @param buffer The buffer to use
 * @return The number of characters in the buffer
 */
__attribute__((always_inline))
__inline uint16_t
uart_buffer_get_fill (uart_buffer_t *buffer);


/**
 * Clears the buffer and resets the limits.
 *
 * @param buffer The buffer to use
 */
__attribute__((always_inline))
__inline void
uart_buffer_clear (uart_buffer_t *buffer);

// ----------------------------------------------------------------------------
// Implementations
// ----------------------------------------------------------------------------

__attribute__((always_inline))
__inline bool_t
uart_buffer_is_full (uart_buffer_t *buffer)
{
  return (buffer->fill >= buffer->buffer_size);
}

__attribute__((always_inline))
__inline bool_t
uart_buffer_is_empty (uart_buffer_t *buffer)
{
  return (buffer->fill == 0);
}

__attribute__((always_inline))
__inline void
uart_buffer_enqueue (uart_buffer_t *buffer, uint8_t value)
{
  uint8_t *p = buffer->buffer + buffer->start + buffer->fill;

  while (p >= (buffer->buffer + buffer->buffer_size))
      p -= buffer->buffer_size;

  *p = value;
  buffer->fill++;
}

__attribute__((always_inline))
__inline uint8_t
uart_buffer_dequeue (uart_buffer_t *buffer)
{
  uint8_t value = buffer->buffer[buffer->start];

  buffer->start++;
  while (buffer->start >= buffer->buffer_size)
      buffer->start -= buffer->buffer_size;

  return value;
}

__attribute__((always_inline))
__inline uint8_t*
uart_buffer_get_at (uart_buffer_t *buffer, uint16_t index)
{
  index += buffer->start;
  while (index >= buffer->buffer_size)
    index -= buffer->buffer_size;

  return buffer->buffer + index;
}

__attribute__((always_inline))
__inline uint8_t*
uart_buffer_get_next (uart_buffer_t *buffer, uint8_t *p)
{
  p++; // Forward pointer

  // Check upper bound
  while (p >= (buffer->buffer + buffer->buffer_size))
    p -= buffer->buffer_size;

  return p;
}

__attribute__((always_inline))
__inline uint16_t
uart_buffer_get_fill (uart_buffer_t *buffer)
{
  return buffer->fill;
}

__attribute__((always_inline))
__inline void
uart_buffer_clear (uart_buffer_t *buffer)
{
  buffer->fill = 0;
  buffer->start = 0;
}

#endif // !__UART_BUFFER_H
