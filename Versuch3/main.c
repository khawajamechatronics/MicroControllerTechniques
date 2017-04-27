// (c) Tobias Faller 2017

#include <msp430.h>
#include <stdint.h>

#include <templateEMP.h>

#define LED_D5 (1 << 3)
#define LED_D6 (1 << 4)
#define LED_D7 (1 << 5)
#define LEDS (LED_D5 | LED_D6 | LED_D7)

#define P2_OUT 0x7F
#define P2_IN 0x80

enum playback_mode_t {
  PLAY = 0b0010,
  PAUSE = 0b0100,
  FAST_REWIND = 0b1000,
  FAST_FORWARD = 0b0001
};

void setup(void);
void loop(void);

__inline void shift_register_clock(void);
__inline void update_state(int8_t count);

__inline void read_buttons(void);
__inline void process_button_state(uint8_t state);

__inline void set_leds(uint8_t state);
__inline void set_aux_leds(uint8_t state);

static enum playback_mode_t current_mode;

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
 */
int main(void) {
  // Initialize with uC tec. settings
  initMSP();

  setup();

  for (;;)
    loop();
}

// Set up all custom stuff
void setup(void) {
  // Initialize P1.3 to P1.5 as Output port
  P1SEL &= ~LEDS; // Set as IO port
  P1SEL2 &= ~LEDS; // Set as IO port
  P1DIR |= LEDS; // Set as output
  P1REN &= ~LEDS; // No pull-up / -down
  P1OUT &= ~LEDS; // Set to low

  // Initialize port P2
  P2SEL &= 0; // Set as IO port
  P2SEL2 &= 0; // Set as IO port
  P2DIR = P2_OUT; // Set some pins as output
  P2REN = 0; // No pull-up / -down
  P2OUT = 0; // Reset shift register

  // Enable shift register
  P2OUT = BIT5;

  led_state = 0;
  led_aux_state = 0;
  timing_counter = 0;
  current_mode = PAUSE;

  // Timer A0 compare control
  TA0CCTL0 = CCIE; // Enable interrupt

  // Timer A0 compare
  // 1 MHz / 8 => 12,5 kHz
  // 12,5 kHz / 8 Hz => 15625
  TA0CCR0 = 0x3D09;

  // Timer A0 control
  TA0CTL = TASSEL_2 // SMCLK as source (1 MHz)
      | ID_3 // Divider of 8
      | MC_1 // Up mode
      | TACLR; // Clear timer

  // Enable echo back on serial interface
  serialEchoBack(1);

  // Initialize LEDs
  set_leds(led_state);
  set_aux_leds(led_aux_state);
}

// Runs infinitely
void loop(void) {
  read_buttons();
}

// Set ISR for timer A0
#pragma vector=TIMER0_A0_VECTOR
__interrupt void timer(void) {
  switch (current_mode) {
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

__inline void update_state(int8_t count) {
  led_state = (led_state + ((count > 0) ? count : (4 + count))) & 0x03;
  led_aux_state = (led_aux_state + ((count > 0) ? count : (3 + count))) % 3;

  //Write new state to shift register
  set_leds(led_state);

  // Write auxiliary LEDs
  set_aux_leds(led_aux_state);
}

__inline void shift_register_clock(void) {
  // Creates one clock pulse
  P2OUT |= BIT4; // Set clock to high
  P2OUT &= ~BIT4; // Set clock to low
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
  static enum playback_mode_t last_mode = PAUSE;

  // Fast forward
  if (state & FAST_FORWARD) {
    if (current_mode != FAST_FORWARD) {
      // Update to forward mode
      last_mode = current_mode;
      current_mode = FAST_FORWARD;
    }

  // Rewind
  } else if (state & FAST_REWIND) {
    if (current_mode != FAST_REWIND) {
      // Update to rewind mode
      last_mode = current_mode;
      current_mode = FAST_REWIND;
    }

  // Pause
  } else if (state & PAUSE) {
    current_mode = PAUSE;

  // Play
  } else if (state & PLAY) {
    current_mode = PLAY;

  // Resume
  } else { // No button is pressed
    if (current_mode & (FAST_FORWARD | FAST_REWIND)) {
      current_mode = last_mode; // Resume last mode
    }
  }
}

__inline void set_aux_leds(uint8_t state) {
  P1OUT &= ~LEDS | (1 << (state + 3));
  P1OUT |= (1 << (state + 3));
}

__inline void set_leds(uint8_t state) {
  // Enable shifting for shift register 2
  P2OUT = (P2OUT & ~0x03) | BIT0;

  uint8_t i;
  for (i = 0; i < 4; i++) { // Write each LED
    if (state == (3 - i)) {
      P2OUT |= BIT6; // Set LED to high
    } else {
      P2OUT &= ~BIT6; // Set LED to low
    }

    shift_register_clock();
  }

  // Disable shift register 2
  P2OUT &= ~0x03;
}

__inline void read_buttons(void) {
  uint8_t button_state = 0;

  // Load button data into shift register 1
  P2OUT |= 0x0C;
  shift_register_clock();

  // Enable shifting for shift register 1
  P2OUT &= ~BIT3;

  uint8_t i;
  for (i = 4; i > 0; i--) {
    if (i != 4)
      shift_register_clock(); // Read next value

    button_state |= ((P2IN & BIT7) >> (3 + i));
  }

  // Disable shift register 1
  P2OUT &= ~0x0C;

  process_button_states(button_state);
}
