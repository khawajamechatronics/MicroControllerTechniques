// (c) Tobias Faller 2017
// (c) Tim Maffenbeier 2017

#include <msp430.h>
#include <stdint.h>

#include "inc/def.h"
#include "inc/config.h"

#include "inc/shift_register.h"

#include "shift_register_p.h"

__inline void
shift_register_init (void)
{
  // Initialize port P2
  P2SEL &= 0; // Set as IO port
  P2SEL2 &= 0; // Set as IO port
  P2DIR = 0x7F; // Set some pins as output
  P2REN = 0; // No pull-up / -down
  P2OUT = 0; // Reset shift register

  // Enable shift register
  P2OUT = BIT5;
}

__inline void
shift_register_set_leds (uint8_t state)
{
  uint8_t i;

  // Enable shifting for shift register 2
  P2OUT = (P2OUT & ~0x03) | BIT0;

  for (i = 0; i < 4; i++) { // Write each LED
    if ((state >> i) & 0x01) {
      P2OUT |= BIT6; // Set LED to high
    } else {
      P2OUT &= ~BIT6; // Set LED to low
    }

    shift_register_clock();
  }

  // Disable shift register 2
  P2OUT &= ~0x03;
}

__inline uint8_t
shift_register_get_buttons (void)
{
  uint8_t i;
  uint8_t button_state = 0;

  // Load button data into shift register 1
  P2OUT |= 0x0C;
  shift_register_clock();

  // Enable shifting for shift register 1
  P2OUT &= ~BIT3;

  for (i = 4; i > 0; i--) {
    if (i != 4)
      shift_register_clock(); // Read next value

    button_state |= ((P2IN & BIT7) >> (3 + i));
  }

  // Disable shift register 1
  P2OUT &= ~0x0C;

  return button_state;
}
