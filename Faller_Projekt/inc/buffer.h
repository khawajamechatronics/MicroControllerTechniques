// (c) Tobias Faller 2017

#ifndef __BUFFER_H
#define __BUFFER_H

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
} buffer_t;

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
buffer_is_full (buffer_t *buffer);

/**
 * Returns 1 if the given buffer is empty.
 *
 * @param buffer The buffer to check
 * @return true if the buffer is empty
 */
__attribute__((always_inline))
__inline bool_t
buffer_is_empty (buffer_t *buffer);

/**
 * Appends the next value to the end of the buffer.
 *
 * @param buffer The buffer to use
 * @param value The value to append
 */
__attribute__((always_inline))
__inline void
buffer_enqueue (buffer_t *buffer, uint8_t value);

/**
 * Returns the first value in the buffer.
 *
 * @param buffer The buffer to use
 * @return The first value in the buffer
 */
__attribute__((always_inline))
__inline uint8_t
buffer_dequeue (buffer_t *buffer);

/**
 * Returns the pointer to the value at the specified index of the buffer.
 *
 * @param buffer The buffer to use
 * @param index The index to fetch
 * @return The pointer to the selected element
 */
__attribute__((always_inline))
__inline uint8_t*
buffer_get_at (buffer_t *buffer, uint16_t index);

/**
 * Returns the pointer to the next value.
 * This method only corrects the pointer if it exceeds the upper bounds.
 * The fill size of the buffer has to be monitored by the user.
 *
 * @param buffer The buffer to use
 * @param p The pointer to the current location
 * @return The pointer to the next buffer location
 */
__attribute__((always_inline))
__inline uint8_t*
buffer_get_next (buffer_t *buffer, uint8_t *p);

/**
 * Returns the number of values in the buffer.
 *
 * @param buffer The buffer to use
 * @return The number of characters in the buffer
 */
__attribute__((always_inline))
__inline uint16_t
buffer_get_fill (buffer_t *buffer);


/**
 * Clears the buffer and resets the limits.
 *
 * @param buffer The buffer to use
 */
__attribute__((always_inline))
__inline void
buffer_clear (buffer_t *buffer);

// ----------------------------------------------------------------------------
// Implementations
// ----------------------------------------------------------------------------

__attribute__((always_inline))
__inline bool_t
buffer_is_full (buffer_t *buffer)
{
  return (buffer->fill >= buffer->buffer_size);
}

__attribute__((always_inline))
__inline bool_t
buffer_is_empty (buffer_t *buffer)
{
  return (buffer->fill == 0);
}

__attribute__((always_inline))
__inline void
buffer_enqueue (buffer_t *buffer, uint8_t value)
{
  uint8_t position = buffer->start + buffer->fill;

  while (position >= buffer->buffer_size)
    position -= buffer->buffer_size;

  buffer->buffer[position] = value;
  buffer->fill++;
}

__attribute__((always_inline))
__inline uint8_t
buffer_dequeue (buffer_t *buffer)
{
  uint8_t value = buffer->buffer[buffer->start];

  buffer->fill--;
  buffer->start++;
  while (buffer->start >= buffer->buffer_size)
      buffer->start -= buffer->buffer_size;

  return value;
}

__attribute__((always_inline))
__inline uint8_t*
buffer_get_at (buffer_t *buffer, uint16_t index)
{
  index += buffer->start;
  while (index >= buffer->buffer_size)
    index -= buffer->buffer_size;

  return buffer->buffer + index;
}

__attribute__((always_inline))
__inline uint8_t*
buffer_get_next (buffer_t *buffer, uint8_t *p)
{
  p++; // Forward pointer

  // Check upper bound
  while (p >= (buffer->buffer + buffer->buffer_size))
    p -= buffer->buffer_size;

  return p;
}

__attribute__((always_inline))
__inline uint16_t
buffer_get_fill (buffer_t *buffer)
{
  return buffer->fill;
}

__attribute__((always_inline))
__inline void
buffer_clear (buffer_t *buffer)
{
  buffer->fill = 0;
  buffer->start = 0;
}

#endif // !__BUFFER_H
