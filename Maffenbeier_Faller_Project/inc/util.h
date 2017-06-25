// (c) Tobias Faller 2017

#ifndef __UTIL_H
#define __UTIL_H

// #define DEFINE_MEMCPY

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

#ifdef DEFINE_MEMCPY
/**
 * Copys the memory from the source to the target.
 *
 * @param dst The target location
 * @param src The source location
 * @param length The length of the data segment
 */
__attribute__((always_inline))
__inline void
memcpy (void *dst, const void *src, uint16_t length);
#endif

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

#ifdef DEFINE_MEMCPY
__attribute__((always_inline))
__inline void
memcpy (void *dst, const void *src, uint16_t length)
{
  uint8_t *p = (uint8_t*) src;
  uint8_t *q = (uint8_t*) dst;
  for (; length-- > 0; *p = *q, p++, q++);
}
#endif

#endif // !__UTIL_H
