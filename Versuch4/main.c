// (c) Tobias Faller 2017

#include <msp430.h>
#include <stdint.h>

#include <templateEMP.h>

#include "main.h"

static uint16_t adc_current_convert;

/*
 * MSP430G2553
 *
 * Wiring:
 *   P1.4 K2 LDR
 *   P1.5 Potentiometer
 *
 *   P3.0 K3 (red)
 *   P3.1 K4 (green)
 *   P3.2 HEATER (blue)
 *
 * Additional notes:
 * P3.2 HEATER
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
  // Initialize P3.0 to P3.2 as output port
  P3SEL &= ~LEDS; // Set as IO port
  P3SEL2 &= ~LEDS; // Set as IO port
  P3DIR |= LEDS; // Set as output
  P3REN &= ~LEDS; // No pull-up / -down
  P3OUT &= ~LEDS; // Set to low

  // Initialize port P2
  P2SEL &= 0; // Set as IO port
  P2SEL2 &= 0; // Set as IO port
  P2DIR = P2_OUT; // Set some pins as output
  P2REN = 0; // No pull-up / -down
  P2OUT = 0; // Reset shift register

  // Enable shift register
  P2OUT = BIT5;

  // Initialize port P1 as analog input
  P1SEL &= ~ANALOG_IN; // Set as IO port
  P1SEL2 &= ~ANALOG_IN; // Set as IO port
  P1DIR &= ~ANALOG_IN; // Set pins as input
  P2REN &= ~ANALOG_IN; // No pull-up / -down

  // Initialize first conversion
  adc_current_convert = 0;

  // Initialize ADC
  ADC10CTL0 = ADC10SHT_2 // 16 × ADC10CLKs
      | ADC10ON // Activate ADC10
      | ADC10IE; // Enable interrupt
  ADC10AE0 = POT | LDR; // Enable channels
  ADC10CTL1 = ADC_CHANNEL[adc_current_convert] // Select current channel
      | ENC // Enable conversion
      | ADC10SC; // Start conversion
}

// Runs infinitely
void loop(void) { }

#pragma vector=ADC10_VECTOR
__interrupt void adc_finished(void) {
  uint8_t finished = adc_current_convert;

  // Save result into storage
  uint16_t adc_result = ADC10MEM;

  // Choose next value
  if (++adc_current_convert >= ADC_VALUES) {
    adc_current_convert = 0;
  }

  // Reset interrupt and enable flag
  ADC10CTL0 &= ~ADC10IFG & ~ENC;

  // Select next channel
  ADC10CTL1 = (ADC10CTL1 & 0xf000) | ADC_CHANNEL[adc_current_convert];

  // Start conversion
  ADC10CTL0 |= ADC10SC | ENC;

  update_analog_value(finished, adc_result);
}

__inline void update_analog_value(uint8_t index, uint16_t value) {

}

__inline void shift_register_clock(void) {
  // Creates one clock pulse
  P2OUT |= BIT4; // Set clock to high
  P2OUT &= ~BIT4; // Set clock to low
}

__inline void set_leds(uint8_t state) {
  // Enable shifting for shift register 2
  P2OUT = (P2OUT & ~0x03) | BIT0;

  uint8_t i;
  for (i = 0; i < 4; i++) { // Write each LED
    if (state >> i) {
      P2OUT |= BIT6; // Set LED to high
    } else {
      P2OUT &= ~BIT6; // Set LED to low
    }

    shift_register_clock();
  }

  // Disable shift register 2
  P2OUT &= ~0x03;
}
