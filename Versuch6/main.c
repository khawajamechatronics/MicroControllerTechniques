// (c) Tobias Faller 2017

#include <msp430.h>
#include <stdint.h>

#include <templateEMP.h>


__inline void setup(void);
__inline void loop(void);

/*
 * MSP430G2553
 *
 * Wiring:
 *   BUZZER REL_MID
 *   DAC_IN REL_LEFT
 *
 *   P1.0 COMP_OUT
 *   P1.3 REL_STAT
 *
 *   P3.0 REL_RIGHT
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

}

// Runs infinitely
__inline void loop(void) {

}
