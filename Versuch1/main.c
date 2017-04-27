// (c) Tobias Faller 2017

#include <msp430.h>
#include <stdint.h>

#include <templateEMP.h>

#define LED_POFFSET 4
#define LEDR (1 << 4)
#define LEDB (1 << 5)
#define LEDS (LEDR | LEDB)

void setup(void);
void loop(void);

// Store LED status
static uint8_t current_state;

/*
 * MSP430G2553
 *
 * Wiring:
 *   P1.4 K3
 *   P1.5 HEATER
 *
 * Additional notes:
 * 1.b) P1.5 HEATER
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
void setup(void) {
  // Set initial state
  current_state = 0x00;

  // Initialize P1.4 & P1.5 as I/O port
  P1SEL &= ~LEDS;
  P1SEL2 &= ~LEDS;

  // Set as output without pull-up / pull-down
  P1DIR |= LEDS;
  P1REN &= ~LEDS;

  // Initialize to 0
  P1OUT &= ~LEDS;

  // Timer A0 compare control
  TA0CCTL0 = CCIE; // Enable compare interrupt

  // Timer A0 compare
  // 1 MHz / 8 => 12,5 kHz
  // 12,5 kHz / 4 Hz => 31250
  TA0CCR0 = 0x7A12;

  // Timer A0 control
  TA0CTL = TASSEL_2 // SMCLK as source (1 MHz)
      | ID_3 // Divider of 8
      | MC_1 // Up mode
      | TACLR; // Clear timer

  // Enable echo back on serial interface
  serialEchoBack(1);
}

// Runs infinitely
void loop(void) {
  if (serialRead() != -1) {
    // Clear serial buffer
    serialFlush();

    // Write out current state
    serialPrint("Current state of blue LED: ");
    serialPrint(((current_state >> 1) & 0x01) ? "on\n" : "off\n");
  }
}

// Set ISR for timer A0
#pragma vector=TIMER0_A0_VECTOR
__interrupt void timer(void) {
  uint8_t led_state = current_state & 0b11;

  // Apply new LED state
  P1OUT &= (led_state << LED_POFFSET) | ~LEDS;
  P1OUT |= (led_state << LED_POFFSET);

  // Increment state
  current_state++;

  // Reset interrupt flag
  TA0CCTL0 &= ~CCIFG;
}
