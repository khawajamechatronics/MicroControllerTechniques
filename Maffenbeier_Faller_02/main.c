// (c) Tobias Faller 2017
// (c) Tim Maffenb(e)ier 2017

#include <msp430.h>
#include <stdint.h>

#include <templateEMP.h>

// ----------------------------------------------------------------------------
// Definitions
// ----------------------------------------------------------------------------

#define LEDR (1 << 5)
#define LEDG (1 << 6)
#define LEDB (1 << 0)
#define LEDY (1 << 7)
#define LEDS (LEDR | LEDG | LEDB | LEDY)

#define BUTTON5 (1 << 3)
#define BUTTON6 (1 << 4)
#define BUTTONS (BUTTON5 | BUTTON6)

#define P1INOUT (LEDS | BUTTONS)

// #define BUTTON5_POLLING

// ----------------------------------------------------------------------------
// Standard methods
// ----------------------------------------------------------------------------

__inline void setup(void);
__inline void loop(void);

// ----------------------------------------------------------------------------
// Methods
// ----------------------------------------------------------------------------

inline void set_led_green(void);
inline void set_led_blue(void);
inline void set_led_red(void);

/*
 * MSP430G2553
 *
 * Wiring:
 *   P1.0 HEATER (blue)
 *   P1.3 PB5
 *   P1.4 PB6
 *   P1.5 K3 (red)
 *   P1.6 K4 (green)
 *   P1.7 JP3 (yellow)
 *
 * Additional notes:
 * P1.0 HEATER
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
  // Initialize P1.3 to P1.6 as I/O port
  P1SEL &= ~P1INOUT;
  P1SEL2 &= ~P1INOUT;

  // Set as input- / output-port
  P1DIR |= LEDS; // Set LEDs as output port
  P1DIR &= ~BUTTONS; // Set buttons as input port

  // Activate / deactivate pull-up / pull-down
  P1REN &= ~LEDS; // Disable pull-up / -down for LEDs
  P1REN |= BUTTONS; // Enable pull-up / -down for both buttons
  P1OUT |= BUTTONS; // Set buttons to pull-up

  // Initialize all LEDs (but LEDY) to low
  P1OUT &= ~(LEDS & ~LEDY); // Set LEDs to low
  P1OUT |= LEDY; // Set LEDY to high

  // Select interrupt edge (high-to-low)
  P1IES |= BUTTON5; // Set button 5 interrupt edge

#ifndef BUTTON5_POLLING
  // Enable button 5 pin interrupt
  P1IFG &= ~BUTTON5; // Reset interrupt flag
  P1IE |= BUTTON5; // Enable interrupt
#endif

  // Timer A control
  TA0CTL = TASSEL_2 // SMCLK as source (1 MHz)
      | ID_3 // Divider of 8
      | MC_0; // Stopped mode
  TA0CCTL0 = 0; // Disable interrupt

  // Timer A compare
  // 1 MHz / 8 => 125 kHz
  // 125 kHz / 4 Hz => 31250
  TA0CCR0 = 0x7A12;
}

// Runs infinitely
__inline void loop(void) {
  set_led_green();
  set_led_blue();

#ifdef BUTTON5_POLLING
  set_led_red();
#endif
}

// Set ISR for timer A0
#pragma vector=TIMER0_A0_VECTOR
__interrupt void timer(void) {
  TA0CCTL0 &= ~CCIE; // Disable interrupt
  TA0CCTL0 &= ~CCIFG; // Reset interrupt flag
  TA0CTL &= ~MC_1; // Set Timer A to stopped mode

  // Set LED to off
  P1OUT &= ~LEDR;
  P1OUT |= LEDY;

  // Activate button 5
  P1IFG &= ~BUTTON5; // Reset interrupt flag
  P1IE |= BUTTON5; // Enable interrupt
}

#pragma vector=PORT1_VECTOR
__interrupt void button5_pressed(void) {
  // Deactivate button 5
  P1IE &= ~BUTTON5; // Disable interrupt
  P1IFG &= ~BUTTON5; // Reset interrupt flag

  TAR = 0; // Reset counter to 0
  TA0CCTL0 &= ~CCIFG; // Reset interrupt flag
  TA0CCTL0 |= CCIE; // Enable compare interrupt
  TA0CTL |= MC_1; // Set Timer A to up mode (start timer)

  // Set LED to on
  P1OUT |= LEDR;
  P1OUT &= ~LEDY;
}

inline void set_led_red(void) {
  if (!(P1IN & BUTTON5)) {
    int released = 0;

    // Set LED to on
    P1OUT |= LEDR;
    P1OUT &= ~LEDY;

    // Wait a bit and check button
    uint16_t i;
    for (i = 5000; i > 0; i--) {
      released |= P1IN & BUTTON5;
      set_led_green();
      set_led_blue();
    }

    // Set LED to off
    P1OUT &= ~LEDR;
    P1OUT |= LEDY;

    // Wait for button release
    while (!released) {
      released |= P1IN & BUTTON5;
      set_led_green();
      set_led_blue();
    }
  }
}

inline void set_led_green(void) {
  if (!(P1IN & BUTTON6))
    P1OUT |= LEDG;
  else
    P1OUT &= ~LEDG;
}

inline void set_led_blue(void) {
  if (!(P1IN & BUTTONS))
    P1OUT |= LEDB;
  else
    P1OUT &= ~LEDB;
}
