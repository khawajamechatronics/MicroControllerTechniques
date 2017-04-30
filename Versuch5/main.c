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

// ----------------------------------------------------------------------------
// Comment out to activate static melody
// ----------------------------------------------------------------------------
// #define STATIC_MELODY

// ----------------------------------------------------------------------------
// Definitions
// ----------------------------------------------------------------------------

#define BUTTON_PLAY BIT3
#define BUTTON_PAUSE BIT4

#ifdef STATIC_MELODY
#define BUTTONS BUTTON_PAUSE
#else
#define BUTTONS (BUTTON_PLAY | BUTTON_PAUSE)
#endif

// ----------------------------------------------------------------------------
// Types
// ----------------------------------------------------------------------------

typedef enum state {
  PAUSED,
  RUNNING,
  WAITING
} state_t;

// ----------------------------------------------------------------------------
// Standard methods
// ----------------------------------------------------------------------------

__inline void setup(void);
__inline void loop(void);

// ----------------------------------------------------------------------------
// Methods
// ----------------------------------------------------------------------------

#ifdef STATIC_MELODY
void melody_play(uint8_t melody);
void melody2_play(void);
void melody_finished(void);
#else
__inline void pause_resume(void);

void melody_finished(void);
void button_measure(void);
void button_reactivate(void);

void piezo_measure_init(void);
#endif

// ----------------------------------------------------------------------------
// Fields
// ----------------------------------------------------------------------------

static state_t current_state;

#ifdef STATIC_MELODY
static uint8_t current_melody;
#else
static uint8_t button_press_count;
#endif

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
  // Initialize in non-paused state
  current_state = WAITING;

  // No button was pressed
  button_press_count = 0;

  // Initialize buttons
  P1SEL &= ~BUTTONS; // Set as IO port
  P1SEL2 &= ~BUTTONS;
  P1DIR &= ~BUTTONS; // Set as input
  P1REN |= BUTTONS; // Enable pull-up /-down resistors
  P1OUT |= BUTTONS; // Pull-up
  P1IES |= BUTTONS; // Interrupt on high-to-low transition
  P1IFG &= ~BUTTONS; // Reset interrupt flag

#ifdef STATIC_MELODY
  // Enable interrupt
  P1IE |= BUTTON_PAUSE;

  play_init(); // Initialize player
  melody_play(1); // Play first melody
#else

  // Initialize piezo as input element
  piezo_measure_init();

  // Enable interrupt
  P1IE |= BUTTON_PLAY;
#endif
}

__inline void loop() {
#ifndef STATIC_MELODY
  if (current_state == WAITING) {
    // Poll piezo since port 3 has no interrupt capability
    if (P3IN & BUZZER) {
      // Disable play button
      P1IE &= ~BUTTON_PLAY;

      // Enable pause button
      P1IE |= BUTTON_PAUSE;

      current_state = RUNNING;
      play_init();
      play_melody(&MELODY1, &button_reactivate);
    }
  }
#endif
}

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
    // Do nothing, we are finished
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
  case WAITING:
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
  uint8_t flags = P1IFG & BUTTONS; // Save interrupt flags
  uint8_t enable = P1IE & BUTTONS; // Save enabled flags

  P1IE &= ~BUTTONS; // Disable interrupts

  // Handle button press
  if (flags & BUTTON_PAUSE) {
    pause_resume();

    // Reset interrupt flag for pause button
    P1IFG &= ~BUTTON_PAUSE;
    P1IE |= enable | BUTTON_PAUSE; // Re-enable interrupt
    return;
  }

  if (flags & BUTTON_PLAY) {
    // No button was pressed
    if (button_press_count == 0) {
      // Schedule measurement
      wait(16, &button_measure);
    }

    // Increment button press count
    button_press_count++;

    // Reset interrupt flag for play button
    P1IFG &= ~BUTTON_PLAY;
    P1IE |= enable | BUTTON_PLAY; // Re-enable interrupt
    return;
  }

  // Hang since this is an unwanted case (for debugging)
  for (;;);
}

__inline void pause_resume(void) {
  static uint8_t t1_state;
  static uint8_t t2_state;

  switch (current_state) {
  case WAITING:
    return; // No melody is playing

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
}

void button_measure(void) {
  // Disable play button
  P1IE &= ~BUTTON_PLAY;

  // Enable pause button
  P1IE |= BUTTON_PAUSE;

  current_state = RUNNING;
  if (button_press_count == 1) {
    play_init();
    play_melody(&MELODY1, &button_reactivate);
  } else {
    play_init();
    play_melody(&MELODY2, &button_reactivate);
  }
}

void button_reactivate(void) {
  // Reset button press count
  button_press_count = 0;

  // Reset state to waiting
  current_state = WAITING;

  // Activate piezo as input
  piezo_measure_init();

  // Disable pause button
  P1IE &= ~BUTTON_PAUSE;

  // Enable play button
  P1IE |= BUTTON_PLAY;
}

void piezo_measure_init(void) {
  // Initialize P3.6 as input port
  P3SEL &= ~BUZZER; // Set as IO port
  P3SEL2 &= ~BUZZER; // Set as IO port
  P3DIR &= ~BUZZER; // Set as input
  P3REN |= BUZZER; // Pull-up / -down enable
  P3OUT &= ~BUZZER; // Pull-down
}
#endif
