// (c) Tobias Faller 2017

#include <msp430.h>
#include <templateEMP.h>

#include "play.h"
#include "wait.h"

#define MELODY_NAME melody1
#include "melodies/melody1.inc"
#undef MELODY_NAME

#define MELODY_NAME melody2
#include "melodies/melody2.inc"
#undef MELODY_NAME

// Comment out to activate static melody
#define STATIC_MELODY

#define BUTTON_PLAY BIT3
#define BUTTON_PAUSE BIT4
#define BUTTONS (BUTTON_PLAY | BUTTON_PAUSE)

typedef enum state {
  PAUSED,
  RUNNING
} state_t;

__inline void setup(void);
__inline void loop(void);

void melody_play(uint8_t melody);
void melody2_play(void);
void melody_finished(void);

static uint8_t current_melody;
static state_t current_state;

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

  current_state = RUNNING;

#ifdef STATIC_MELODY
  // Initialize pause button
  P1SEL &= ~BUTTON_PAUSE; // Set as IO port
  P1SEL2 &= ~BUTTON_PAUSE;
  P1DIR &= ~BUTTON_PAUSE; // Set as input
  P1REN |= BUTTON_PAUSE; // Enable pull-up /-down resistors
  P1OUT |= BUTTON_PAUSE; // Pull-up
  P1IES |= BUTTON_PAUSE; // Interrupt on high-to-low transition
  P1IFG &= ~BUTTON_PAUSE; // Reset interrupt flag
  P1IE |= BUTTON_PAUSE; // Enable interrupt
#else
  // Initialize buttons
  P1SEL &= ~BUTTONS; // Set as IO port
  P1SEL2 &= ~BUTTONS;
  P1DIR &= ~BUTTONS; // Set as input
  P1REN |= BUTTONS; // Enable pull-up /-down resistors
  P1OUT |= BUTTONS; // Pull-up
  P1IES |= BUTTONS; // Interrupt on high-to-low transition
  P1IFG &= ~BUTTONS; // Reset interrupt flag
  P1IE |= BUTTONS; // Enable interrupt
#endif

  // Play first melody
  melody_play(1);
}

__inline void loop() { }

// ----------------------------------------------------------------------------
// Static play of melody
// ----------------------------------------------------------------------------

#ifdef STATIC_MELODY
void melody_play(uint8_t melody) {
  // Set current melody
  current_melody = melody;

  switch (melody) {
  case 1:
    play_melody(&MELODY1, &melody_finished);
    break;
  case 2:
    play_melody(&MELODY2, &melody_finished);
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

#pragma vector=PORT1_VECTOR
__interrupt void button_pressed(void) {
  static uint8_t t1_state;
  static uint8_t t2_state;

  // Disable interrupt
  P1IE &= ~BUTTON_PAUSE;

  switch (current_state) {
  case PAUSED:
    // Set state to playing
    current_state = RUNNING;

    // Resume state
    TA0CTL |= t1_state;
    TA1CTL |= t2_state;
    break;
  case RUNNING:
    // Save the state of both timer
    t1_state = TA0CTL & MC_1;
    t2_state = TA1CTL & MC_1;

    TA0CTL &= ~MC_1; // Deactivate timer
    TA1CTL &= ~MC_1; // Deactivate timer

    current_state = PAUSED;
    break;
  }

  // Wait for button to get released
  while (!(P1IN & BUTTON_PAUSE));

  P1IFG &= ~BUTTON_PAUSE; // Reset interrupt flag
  P1IE |= BUTTON_PAUSE; // Re-enable interrupt
}
#endif

// ----------------------------------------------------------------------------
// Reactive play of melody
// ----------------------------------------------------------------------------

#ifndef STATIC_MELODY
#pragma vector=PORT1_VECTOR
__interrupt void button_pressed(void) {
  uint8_t flags = P1IFG; // Save interrupt flags

  P1IE &= ~BUTTONS; // Disable interrupt
  P1IFG &= ~BUTTONS; // Reset interrupt flag

  // Handle button press
  if (flags & BUTTON_PAUSE) {
    pause();

    // Re-enable interrupt
    P1IE |= BUTTONS;
    return;
  } else if (flags & BUTTON_PLAY) {
    // TODO
  }
}
#endif
