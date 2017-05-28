// (c) Tobias Faller 2017

#ifndef __UTIL_H
#define __UTIL_H

// ----------------------------------------------------------------------------
// Methods
// ----------------------------------------------------------------------------

/**
 * Sets the memory to the specified value.
 *
 * @param mem The pointer to the memory area
 * @param value The value to use
 * @param length The size of the memory are
 */
__attribute__((always_inline))
__inline void
memset (void *buffer, uint8_t value, uint16_t length);

// ----------------------------------------------------------------------------
// Implementations
// ----------------------------------------------------------------------------

__attribute__((always_inline))
__inline void
memset (void *buffer, uint8_t value, uint16_t length)
{
  uint8_t *p = (uint8_t*) buffer;
  for (; length-- > 0; *p = value, ++p);
}

#endif // !__UTIL_H
