// (c) Tobias Faller 2017
// (c) Tim Maffenbeier 2017

#ifndef __SHIFT_REGISTER_H
#define __SHIFT_REGISTER_H

#include <stdint.h>

#include "def.h"
#include "config.h"

/**
 * Initializes the shift register port.
 */
void
shift_register_init (void);

/**
 * Sets the LEDs on the second shift register.
 * The LSB of the state is LED D1.
 *
 * @param state The state to write out
 */
void
shift_register_set_leds (uint8_t state);

/**
 * Returns the state of the buttons.
 * The LSB of the state is button PB1.
 *
 * @return The binary value of each button
 */
uint8_t
shift_register_get_buttons (void);

#endif // !__SHIFT_REGISTER_H
