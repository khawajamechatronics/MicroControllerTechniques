// (c) Tobias Faller 2017

#include <msp430.h>
#include <templateEMP.h>

#include "play.h"

#define MELODY_NAME melody1
#include "melodies/melody1.inc"

// #define MELODY_NAME melody2
// #include "melodies/melody2.inc"

#define BUTTON_PLAY BIT3
#define BUTTON_PAUSE BIT4
#define BUTTONS (BUTTON_PLAY | BUTTON_PAUSE)

__inline void setup(void);
__inline void loop(void);

__inline void pause_resume(void);

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

  // Initialize buttons
  P1SEL &= ~BUTTONS; // Set as IO port
  P1SEL2 &= ~BUTTONS;
  P1DIR &= ~BUTTONS; // Set as input
  P1REN |= BUTTONS; // Enable pull-up /-down resistors
  P1OUT |= BUTTONS; // Pull-up
  P1IES |= BUTTONS; // Interrupt on high-to-low transition
  P1IFG &= ~BUTTONS; // Reset interrupt flag
  P1IE |= BUTTONS; // Enable interrupt

  // Play first melody
  melody_play(1);
}

__inline void loop() {
  pause_resume();
}

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

__inline void pause_resume(void) {
  uint8_t t1_state = TA0CTL;
  uint8_t t2_state = TA1CTL;

  TA0CTL &= ~MC_1; // Deactivate timer
  TA1CTL &= ~MC_1; // Deactivate timer

  // Wait for pause button to get released
  while (!(P1IN & BUTTON_PAUSE));

  // Wait for the next press of the pause button
  while (P1IN & BUTTON_PAUSE);

  // Wait for pause button to get released
  while (!(P1IN & BUTTON_PAUSE));

  // Resume state
  TA0CTL = t1_state;
  TA1CTL = t2_state;
}

#pragma vector=PORT1_VECTOR
__interrupt void button_pressed(void) {
  uint8_t flags = P1IFG; // Save interrupt flags

  P1IE &= ~BUTTONS; // Disable interrupt
  P1IFG &= ~BUTTONS; // Reset interrupt flag

  // Handle button press
  if (flags & BUTTON_PAUSE) {
    pause_resume();

    // Re-enable interrupt
    P1IE |= BUTTONS;
    return;
  } else if (flags & BUTTON_PLAY) {
    // TODO
  }
}
