// (c) Tobias Faller 2017

#ifndef _SHIFT_REGISTER_
#define _SHIFT_REGISTER_

#include <msp430.h>
#include <stdint.h>

#define P2_OUT 0x7F
#define P2_IN 0x80

/**
 * Initializes the shift register port.
 */
__inline void shift_register_init(void);

/**
 * Applies one clock pulse to both shift registers.
 */
__inline void shift_register_clock(void);

/**
 * Sets the LEDs on the second shift register.
 * The LSB of the state is LED D1.
 *
 * @param state The state to write out
 */
__inline void set_shift_register_leds(uint8_t state);

/**
 * Returns the state of the buttons.
 * The LSB of the state is button PB1.
 *
 * @return The binary value of each button
 */
__inline uint8_t get_shift_register_buttons(void);

#endif //!_SHIFT_REGISTER_
