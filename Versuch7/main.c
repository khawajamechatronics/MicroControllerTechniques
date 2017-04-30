// (c) Tobias Faller 2017

#include <msp430.h>
#include <stdint.h>

#include <templateEMP.h>

// ----------------------------------------------------------------------------
// Definitions
// ----------------------------------------------------------------------------

#define LEDG BIT0

#define BUTTON_HANG BIT3

// ----------------------------------------------------------------------------
// Standard methods
// ----------------------------------------------------------------------------

__inline void setup(void);
__inline void loop(void);

// ----------------------------------------------------------------------------
// Methods
// ----------------------------------------------------------------------------

__inline int8_t hang();

// ----------------------------------------------------------------------------
// Fields
// ----------------------------------------------------------------------------


/*
 * MSP430G2553
 *
 * Wiring:
 *   P1.0 K4 (LED gn)
 *   P1.3 PB5
 */
int main(void) {
  // Initialize with uC tec. settings
  initMSP();

  setup();

  for (;;)
    loop();
}

// Set up all custom stuff
__inline void setup(void) {
  P1SEL &= ~(LEDG | BUTTON_HANG); // Set as IO port
  P1SEL2 &= ~(LEDG | BUTTON_HANG);

}

// Runs infinitely
__inline void loop(void) {
  int8_t counter;

  P1OUT |= LEDG;
  for (counter = 8; (counter >= 0) | hang(); counter--) {
    __delay_cycles(25000);
  }

  P1OUT |= LEDG;
  for (counter = 8; (counter >= 0) | hang(); counter--) {
    __delay_cycles(25000);
  }
}

__inline int8_t hang() {
  if (P1IN & BUTTON_HANG) {
    // Stop execution
    for (;;);
  }

  // Return 'false'
  return 0x00;
}
