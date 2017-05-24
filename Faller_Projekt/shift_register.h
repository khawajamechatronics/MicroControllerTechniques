// (c) Tobias Faller 2017

#ifndef _SHIFT_REGISTER
#define _SHIFT_REGISTER

#include <stdint.h>

#include "def.h"
#include "config.h"

/**
 * Initializes the shift register port.
 */
__inline void
shift_register_init (void);

/**
 * Sets the LEDs on the second shift register.
 * The LSB of the state is LED D1.
 *
 * @param state The state to write out
 */
__inline void
shift_register_set_leds (uint8_t state);

/**
 * Returns the state of the buttons.
 * The LSB of the state is button PB1.
 *
 * @return The binary value of each button
 */
__inline uint8_t
shift_register_get_buttons (void);

#endif //!_SHIFT_REGISTER
