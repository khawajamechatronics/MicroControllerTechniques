// (c) Tobias Faller 2017

#include <msp430.h>
#include <stdint.h>

#include <templateEMP.h>

#include "main.h"

static uint16_t adc_current_convert;
static uint16_t adc_color_components[3];

static uint8_t current_color;
static uint8_t new_color;
static uint16_t new_color_time;

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

  // Initialize ADC
  ADC10CTL1 = 0; // Enable modifying of ADC settings
  ADC10CTL0 = ADC10SHT_2 // 16 × ADC10CLKs
      | ADC10ON // Activate ADC10
      | ADC10IE; // Enable interrupt
  ADC10AE0 = POT | LDR; // Enable channels

  // Initialize color recognition
  current_color = 0; // (NONE)
  new_color = 0; // (NONE)
  new_color_time = 0;

  // Start with first conversion
  adc_convert(0);
}

// Runs infinitely
void loop(void) { }

#pragma vector=ADC10_VECTOR
__interrupt void adc_finished(void) {
  // Save result into storage
  uint8_t finished = adc_current_convert;
  uint16_t adc_result = ADC10MEM;

  // Process the read analog value
  process_analog_value(finished, adc_result);

  // Choose next sample and start ADC
  uint8_t next = finished + 1;
  adc_convert((next >= ADC_VALUES) ? 0 : next);
}

__inline void process_analog_value(uint8_t index, uint16_t value) {
  switch (index) {
  case 0: // Red was measured
  case 1: // Green was measured
  case 2: // Blue was measured
    adc_color_components[index] = value;

    // Update current color
    if (index == 2) {
      identify_color();
    }
    break;
  case 3: // Potentiometer was measured
    /*
     * Explanation:
     *   We want our 10 bit value on a 5 bit range => shift by 5
     *   (value >> 5)
     *
     *   We want only 3/4 of this value since the
     *   highest led occupies 1,5 ranges
     *   (value * 3 / 4) >> 5
     *   = ((value << 2 - value) / 4) >> 5
     *   = ((value << 2 - value) >> 2) >> 5
     *   = ((value << 2 - value) >> (2 + 5)
     *
     *   We want that no LED is active when on lowest bucket
     *   (((value << 2) - value) >> (2 + 5)) >> 1
     *   = ((value << 2) - value) >> 8
     */
    set_shift_register_leds(((value << 2) - value) >> 8);
    break;
  }
}

__inline void shift_register_clock(void) {
  // Creates one clock pulse
  P2OUT |= BIT4; // Set clock to high
  P2OUT &= ~BIT4; // Set clock to low
}

__inline void set_shift_register_leds(uint8_t state) {
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

__inline void adc_convert(uint8_t index) {
  // Reset interrupt and enable flag
  ADC10CTL0 &= ~(ADC10IFG | ENC);

  // Set new index
  adc_current_convert = index;

  // Select next channel
  ADC10CTL1 = (ADC10CTL1 & 0x0FFF) | ADC_CHANNEL[index];

  // Select current LED
  P3OUT &= ~LEDS;
  P3OUT |= ADC_LEDS[index];

  // Let LDR value settle
  if (adc_current_convert < 3) {
    __delay_cycles(100000); // Wait 100 ms => ~3 measurements/s
  }

  // Start ADC conversion
  ADC10CTL0 |= ENC // Enable conversion
      | ADC10SC; // Start conversion
}

void identify_color(void) {
  /*serialPrint("Current analyzed color: ");
  serialPrintInt(adc_color_components[0]);
  serialPrint(", ");
  serialPrintInt(adc_color_components[1]);
  serialPrint(", ");
  serialPrintInt(adc_color_components[2]);
  serialPrintln("");*/

  // Save color candidates
  int8_t candidates_size = 0;
  uint16_t canddidates[COLORS][2];

  int8_t color;
  for (color = COLORS - 1; color >= 0; --color) {
    // Squared difference to the tested color
    // uint16_t diff = 0;

    // Calculate the difference to comparing color
    int8_t i;
    for (i = 2; i >= 0; i--) {
      // Calculate difference of color component
      int16_t d = (int16_t) adc_color_components[i]
          - (int16_t) COLOR_COMPONENTS[color][i];
      d *= d; // Square the difference

      if (d <= (int16_t) COLOR_DIFF_THRESHOLD) {
        break;
      }

      if (i == 0) {
        canddidates[candidates_size][0] = (uint16_t) color;
        canddidates[candidates_size][1] = (uint16_t) d;
        candidates_size++;
      }
      // Sum the squared difference
      //diff += (uint16_t) d;
    }

    // Check if difference is smaller than a selected threshold
    // This preselects our candidates
    /*if ((int16_t) diff <= (int16_t) COLOR_DIFF_THRESHOLD) {
      canddidates[candidates_size][0] = color;
      canddidates[candidates_size][1] = diff;
      candidates_size++;
    }*/
  }

  if (candidates_size == 0) {
    color = 0;
  } else {
    // Select best candidate
    uint16_t min_value = 0xFFFF;
    for (; candidates_size >= 0; candidates_size--) {
      // Is the next color a better approximation?
      if (canddidates[candidates_size][1] < min_value) {
        min_value = canddidates[candidates_size][1];
        color = canddidates[candidates_size][0];
      }
    }
  }

  if (color != current_color) {
    if (color != new_color) {
      // Color was detected right now
      new_color = color;
      new_color_time = 0;
    } else {
      // Color was already there before
      if (++new_color_time > COLOR_DETECT_THRESHOLD) {
        current_color = new_color; // Replace existing color
        report_new_color(new_color); // Report new color to user
      }
    }
  }
}

void report_new_color(uint8_t index) {
  serialPrint("Found new color: ");
  serialPrintln(COLOR_NAMES[index]);
}
