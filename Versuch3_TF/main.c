// (c) Tobias Faller 2017

#include <msp430.h>
#include <stdint.h>

#include <templateEMP.h>

#include "shift_register.h"

// ----------------------------------------------------------------------------
// Definitions
// ----------------------------------------------------------------------------

#define LED_D5 (1 << 3)
#define LED_D6 (1 << 4)
#define LED_D7 (1 << 5)
#define LEDS (LED_D5 | LED_D6 | LED_D7)

// ----------------------------------------------------------------------------
// Types
// ----------------------------------------------------------------------------

typedef enum playback_mode {
  FAST_REWIND = 0b1000,
  PAUSE = 0b0100,
  PLAY = 0b0010,
  FAST_FORWARD = 0b0001
} playback_mode_t;

// ----------------------------------------------------------------------------
// Standard methods
// ----------------------------------------------------------------------------

__inline void setup(void);
__inline void loop(void);

// ----------------------------------------------------------------------------
// Methods
// ----------------------------------------------------------------------------

__inline void update_state(int8_t count);
__inline void process_button_states(uint8_t state);

__inline void set_shift_leds(uint8_t state);
__inline void set_aux_leds(uint8_t state);

// ----------------------------------------------------------------------------
// Fields
// ----------------------------------------------------------------------------

static playback_mode_t mode;

static uint8_t timing_counter;

static uint8_t led_state;
static uint8_t led_aux_state;

/*
 * MSP430G2553
 *
 * Wiring:
 *   P1.3 K4 (green)
 *   P1.4 K3 (red)
 *   P1.5 HEATER (blue)
 *
 *   P2.0 S0 (2)
 *   P2.1 S1 (2)
 *   P2.2 S0 (1)
 *   P2.3 S1 (1)
 *   P2.4 CK
 *   P2.5 /CLR
 *   P2.6 SR (2)
 *   P2.7 QD (1)
 *
 * Additional notes:
 * P1.5 HEATER
 *   This pin connects the blue LED and the heater on the board.
 *   The heater has to be deactivated by setting the jumper 'HEATER' to off.
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
  // Initialize P1.3 to P1.5 as Output port
  P1SEL &= ~LEDS; // Set as IO port
  P1SEL2 &= ~LEDS; // Set as IO port
  P1DIR |= LEDS; // Set as output
  P1REN &= ~LEDS; // No pull-up / -down
  P1OUT &= ~LEDS; // Set to low

  // Initialize the shift register
  shift_register_init();

  // Initialize the internal state
  led_state = 0;
  led_aux_state = 0;
  timing_counter = 0;
  mode = PAUSE;

  // Timer A compare
  // 1 MHz / 8 => 125 kHz
  // 125 kHz / 8 Hz => 15625
  TA0CCR0 = 0x3D09;

  // Reset Timer A
  TA0R = 0;

  // Timer A control
  TA0CTL = TASSEL_2 // SMCLK as source (1 MHz)
      | ID_3 // Divider of 8
      | MC_1; // Up mode
  TA0CCTL0 = CCIE; // Enable interrupt

  // Initialize LEDs
  set_shift_leds(led_state);
  set_aux_leds(led_aux_state);
}

// Runs infinitely
__inline void loop(void) {
  process_button_states(get_shift_register_buttons());
}

// Set ISR for timer A0
#pragma vector=TIMER0_A0_VECTOR
__interrupt void timer(void) {
  switch (mode) {
  case FAST_REWIND:
    update_state(-1);
    break;
  case FAST_FORWARD:
    update_state(1);
    break;
  case PLAY:
    // We want to update every second => division by 8
    if ((++timing_counter & 0x07) == 0) {
      update_state(1);
    }
    break;
  case PAUSE:
    break;
  }

  TA0CCTL0 &= ~CCIFG; // Reset interrupt flag
}

/*
 * Update the current state machine.
 * Priority:
 * 1.) Fast Forward
 * 2.) Rewind (Fast Rewind)
 * 3.) Pause
 * 4.) Play
 */
__inline void process_button_states(uint8_t state) {
  static playback_mode_t last_mode = PAUSE;

  // Fast forward
  if (state & FAST_FORWARD) {
    if (mode != FAST_FORWARD) {
      // Update to forward mode
      last_mode = mode;
      mode = FAST_FORWARD;
    }

  // Rewind
  } else if (state & FAST_REWIND) {
    if (mode != FAST_REWIND) {
      // Update to rewind mode
      last_mode = mode;
      mode = FAST_REWIND;
    }

  // Pause
  } else if (state & PAUSE) {
    mode = PAUSE;

  // Play
  } else if (state & PLAY) {
    mode = PLAY;

  // Resume
  } else { // No button is pressed
    if (mode & (FAST_FORWARD | FAST_REWIND)) {
      mode = last_mode; // Resume last mode
    }
  }
}

__inline void update_state(int8_t count) {
  // Update the LEDs state and limit it's values
  led_state += (count > 0) ? count : (4 + count);
  led_state &= 0x03;

  led_aux_state += (count > 0) ? count : (3 + count);
  if (led_aux_state >= 3) {
    led_aux_state -= 3;
  }

  //Write new state to shift register
  set_shift_leds(led_state);

  // Write auxiliary LEDs
  set_aux_leds(led_aux_state);
}

__inline void set_aux_leds(uint8_t state) {
  P1OUT &= ~LEDS | (1 << (state + 3));
  P1OUT |= (1 << (state + 3));
}

__inline void set_shift_leds(uint8_t state) {
  static uint8_t rate_divider = 0;

  // Only write each 128th value since the LEDs are too bright
  if (++rate_divider & 0x7F) {
    return;
  }

  set_shift_register_leds(1 << (3 - state));
}
