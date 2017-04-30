// (c) Tobias Faller 2017

#include <msp430.h>
#include <stdint.h>

#include <templateEMP.h>

#include "play.h"
#include "wait.h"

#define MELODY_NAME melody1
#include "melodies/melody1.inc"
#undef MELODY_NAME

#define MELODY_NAME melody2
#include "melodies/melody2.inc"
#undef MELODY_NAME

// ----------------------------------------------------------------------------
// Definitions
// ----------------------------------------------------------------------------

// Port 1
#define COMP_OUT BIT0
#define REL_STAT BIT3

#define BUTTON_PLAY BIT4
#define BUTTON_PAUSE BIT5
#define BUTTONS (BUTTON_PLAY | BUTTON_PAUSE)

// Port 3
#define REL_RIGHT BIT0

// ----------------------------------------------------------------------------
// Types
// ----------------------------------------------------------------------------

typedef enum state {
  PAUSED,
  PLAYING
} state_t;

// ----------------------------------------------------------------------------
// Standard methods
// ----------------------------------------------------------------------------

__inline void setup(void);
__inline void loop(void);

// ----------------------------------------------------------------------------
// Methods
// ----------------------------------------------------------------------------

__inline void piezo_set_out(void);
__inline void piezo_set_in(void);

void start_music();
void button_reactivate();

// ----------------------------------------------------------------------------
// Fields
// ----------------------------------------------------------------------------

static state_t current_state = PLAYING;
static uint8_t current_press_count = 0;

/*
 * MSP430G2553
 *
 * Wiring:
 *   BUZZER REL_MID
 *   DAC_IN REL_LEFT
 *
 *   P1.0 COMP_OUT
 *   P1.3 REL_STAT
 *   P1.4 PB5
 *   P1.5 PB6
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
  // Initialize P1.0 and P1.3 to P1.5
  P1SEL &= ~(COMP_OUT | REL_STAT | BUTTONS); // Set as IO port
  P1SEL2 &= ~(COMP_OUT | REL_STAT | BUTTONS);

  P1REN &= ~(COMP_OUT | REL_STAT); // Disable pull-up / -down resistors
  P1REN |= BUTTONS; // Enable pull-up / -down for buttons

  P1DIR |= REL_STAT; // Set REL_STAT as output
  P1DIR &= ~(COMP_OUT | BUTTONS); // Set COMP_OUT and buttons as input

  P1OUT |= BUTTONS; // Set to pull-up for buttons

  P1IFG &= ~(COMP_OUT | BUTTONS); // Reset the interrupt-flag
  P1IE |= COMP_OUT | BUTTON_PLAY; // Enable interrupts

  // Enable the piezo sensor
  piezo_set_in();

  // Initialize player
  play_init();
}

// Runs infinitely
__inline void loop(void) { }

#pragma vector=PORT1_VECTOR
__interrupt void port1_int(void) {
  static uint8_t t1_state;
  static uint8_t t2_state;

  if (P1IFG & BUTTON_PLAY) {
    if (current_press_count == 0) {
      // Schedule measurement
      wait(16, &start_music);
    }

    // Increment button press count
    current_press_count++;

    P1IFG &= ~BUTTON_PLAY; // Reset interrupt flag for play button
    return;
  }

  if (P1IFG & BUTTON_PAUSE) {
    switch (current_state) {
    case PAUSED:
      // Set state to playing
      current_state = PLAYING;

      // Resume state
      TA0CTL |= t1_state;
      TA1CTL |= t2_state;
      break;

    case PLAYING:
      // Save the state of both timer
      t1_state = TA0CTL & MC_1;
      t2_state = TA1CTL & MC_1;

      TA0CTL &= ~MC_1; // Deactivate timer
      TA1CTL &= ~MC_1; // Deactivate timer

      current_state = PAUSED;
      break;
    }

    P1IFG &= ~BUTTON_PAUSE; // Reset interrupt flag for pause button
    return;
  }

  if (P1IFG & COMP_OUT) {
    // Select song 1
    current_press_count = 1;

    // Let's get this party started
    start_music();

    P1IFG &= ~COMP_OUT; // Reset interrupt flag for the piezo
    return;
  }
}

void start_music(void) {
  // Disable play button
  P1IE &= ~BUTTON_PLAY;

  // Enable pause button
  P1IE |= BUTTON_PAUSE;

  // Set the current play state for the pause button
  current_state = PLAYING;

  // Play the selected melody
  piezo_set_out();
  play_melody((current_press_count <= 1) ? &MELODY1 : &MELODY2,
      &button_reactivate);
}

void button_reactivate(void) {
  // Activate piezo as sensor
  piezo_set_in();

  // Reset button press counter
  current_press_count = 0;

  // Clear interrupt flags
  P1IFG &= ~BUTTONS;

  // Disable pause button
  P1IE &= ~BUTTON_PAUSE;

  // Enable play button
  P1IE |= BUTTON_PLAY;
}

__inline void piezo_set_in(void) {
  // Set piezo as input (use relay)
  P1OUT |= REL_STAT;

  // Enable interrupts
  P1IFG &= ~COMP_OUT;
  P1IE |= COMP_OUT;
}

__inline void piezo_set_out(void) {
  // Set piezo as output (use relay)
  P1OUT &= ~REL_STAT;

  // Disable interrupts
  P1IFG &= ~COMP_OUT;
  P1IE &= ~COMP_OUT;
}
