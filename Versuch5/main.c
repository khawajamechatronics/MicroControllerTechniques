// (c) Tobias Faller 2017

#include <msp430.h>
#include <templateEMP.h>

#include "play.h"

#define MELODY_NAME melody1
#include "melodies/melody1.inc"

// #define MELODY_NAME melody2
// #include "melodies/melody2.inc"

__inline void setup(void);
__inline void loop(void);

void finished(void);

/*
 * MSP430G2553
 *
 * Wiring:
 *   P1.3 PB5
 *   P1.4 PB6
 *   P3.6 X1 Buzzer
 *
 * Additional notes:
 *   Jumper JP5 @ VFO
 */
int main(void) {
  // Initialize with uC tec. settings
  initMSP();

  setup();

  for (;;)
    loop();
}

__inline void setup() {
  // Initialize player
  play_init();

  // Play a test melody
  play_melody(&melody1, &finished);
}

__inline void loop() { }

void finished(void) {

}
