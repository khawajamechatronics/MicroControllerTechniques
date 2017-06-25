// (c) Tobias Faller 2017

#ifndef __SHIFT_REGISTER_P_H
#define __SHIFT_REGISTER_P_H

// ----------------------------------------------------------------------------
// Methods
// ----------------------------------------------------------------------------

/**
 * Applies one clock pulse to both shift registers.
 */
__attribute__((always_inline))
__inline void
shift_register_clock (void);

// ----------------------------------------------------------------------------
// Implementations
// ----------------------------------------------------------------------------

__attribute__((always_inline))
__inline void
shift_register_clock (void)
{
  // Creates one clock pulse
  P2OUT |= BIT4; // Set clock to high
  P2OUT &= ~BIT4; // Set clock to low
}

#endif // !__SHIFT_REGISTER_P_H
