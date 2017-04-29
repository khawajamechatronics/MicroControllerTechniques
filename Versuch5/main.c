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

void melody_finished(void);

static uint8_t current_melody;

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

  // Play first melody
  melody_play(1);
}

__inline void loop() { }

void melody_play(uint8_t melody) {
  // Set current melody
  current_melody = melody;

  switch (melody) {
  case 1:
    play_melody(&melody1, &melody_finished);
    break;
  case 2:
    play_melody(&melody2, &melody_finished);
    break;
  }
}

void melody_finished(void) {
  switch (current_melody) {
  case 1:
    // Wait one second and play melody 2
    wait(16, &melody2_play);
    break;
  case 2:
    // Do nothing
    break;
  }
}

void melody2_play(void) {
  melody_play(2);
}
