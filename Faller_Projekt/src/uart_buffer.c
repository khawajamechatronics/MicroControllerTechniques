#include <stdint.h>

#include "inc/def.h"
#include "inc/config.h"

#include "inc/uart_buffer.h"

__inline bool_t
uart_buffer_is_full (uart_buffer_t *buffer)
{
  return (buffer->fill >= buffer->buffer_size);
}

__inline bool_t
uart_buffer_is_empty (uart_buffer_t *buffer)
{
  return (buffer->fill == 0);
}

__inline void
uart_buffer_enqueue (uart_buffer_t *buffer, uint8_t value)
{
  uint8_t *p = buffer->buffer + buffer->start + buffer->fill;

  while (p >= (buffer->buffer + buffer->buffer_size))
      p -= buffer->buffer_size;

  *p = value;
  buffer->fill++;
}

__inline uint8_t
uart_buffer_dequeue (uart_buffer_t *buffer)
{
  uint8_t value = buffer->buffer[buffer->start];

  buffer->start++;
  while (buffer->start >= buffer->buffer_size)
      buffer->start -= buffer->buffer_size;

  return value;
}

__inline uint8_t*
uart_buffer_get_at (uart_buffer_t *buffer, uint16_t index)
{
  index += buffer->start;
  while (index >= buffer->buffer_size)
    index -= buffer->buffer_size;

  return buffer->buffer + index;
}

__inline uint8_t*
uart_buffer_get_next (uart_buffer_t *buffer, uint8_t *p)
{
  p++; // Forward pointer

  // Check upper bound
  while (p >= (buffer->buffer + buffer->buffer_size))
    p -= buffer->buffer_size;

  return p;
}

__inline uint16_t
uart_buffer_get_fill (uart_buffer_t *buffer)
{
  return buffer->fill;
}
