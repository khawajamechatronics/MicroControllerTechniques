// (c) Tobias Faller 2017

#include <msp430.h>
#include <stdint.h>

#include <templateEMP.h>

#include "main.h"
#include "shift_register.h"

static uint16_t adc_current_convert;

static uint16_t adc_scheduled_convert;
static uint16_t adc_scheduled_convert_start;

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
__inline void setup(void) {
  // Initialize P3.0 to P3.2 as output port
  P3SEL &= ~LEDS; // Set as IO port
  P3SEL2 &= ~LEDS; // Set as IO port
  P3DIR |= LEDS; // Set as output
  P3REN &= ~LEDS; // No pull-up / -down
  P3OUT &= ~LEDS; // Set to low

  // Initialize the shift register
  shift_register_init();

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

  // Reset Timer A0
  TA0CTL = TACLR; // Clear timer

  // Timer A0 compare
  // 1 MHz / 8 => 125 kHz
  // 125 kHz / 10 Hz => 12500
  // TA0CCR0 = 0x30D4;
  TA0CCR0 = 0x30D4;

  // Timer A0 compare control
  TA0CCTL0 = CCIE; // Enable interrupt

  // Timer A0 control
  TA0CTL = TASSEL_2 // SMCLK as source (1 MHz)
      | ID_3 // Divider of 8
      | MC_0; // Disabled

  // Initialize color recognition
  current_color = 0; // (NONE)
  new_color = 0; // (NONE)
  new_color_time = 0;

  // Initialize with empty schedule
  adc_scheduled_convert = 0; // (NONE)
  adc_scheduled_convert_start = 0x00;

  // Schedule first color conversion
  adc_schedule_convert(1);

  // Start with first conversion
  adc_convert(0);
}

// Runs infinitely
__inline void loop(void) { }

#pragma vector=ADC10_VECTOR
__interrupt void adc_finished(void) {
  // Save result into storage
  uint8_t finished = adc_current_convert;
  uint16_t adc_result = ADC10MEM;

  // Process the read analog value
  process_analog_value(finished, adc_result);

  // Do we need to start one of the r/g/b values?
  if (adc_scheduled_convert_start != 0x00) {
    adc_scheduled_convert_start = 0x00;
    adc_convert(adc_scheduled_convert);
  } else {
    // Choose next sample and start ADC
    adc_convert(0);
  }
}

/**
 * We want our LEDs to stay on for some time before the analog value
 * is read. This is done via Timer A0 and a start flag which is set by
 * this ISR.
 */
#pragma vector=TIMER0_A0_VECTOR
__interrupt void timer(void) {
  // LED was on for some time now
  // => we start the ADC conversion
  adc_scheduled_convert_start = 0x01;

  TA0CTL &= ~MC_1; // Deactivate timer
  TA0CCTL0 &= ~CCIFG; // Reset interrupt flag
}

/**
 * Callback function to process the converted analog data.
 * The index denotes the channel / source of which the value
 * was read.
 * Index:
 *   0: Potentiometer
 *   1: Red value of LDR
 *   2: Green value of LDR
 *   3: Blue value of LDR
 *
 * @param index The index of the read value
 * @param value The analog value read
 */
__inline void process_analog_value(uint8_t index, uint16_t value) {
  switch (index) {
  case 1: // Red was measured
  case 2: // Green was measured
  case 3: // Blue was measured
    // Write analog value to temporary storage
    adc_color_components[index - 1] = value;

    // Schedule next conversion
    adc_schedule_convert((index == 3) ? 1 : (index + 1));

    // Update current color
    /*if (index == 2) {
      identify_color();
    }*/
    break;
  case 0: // Potentiometer was measured
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

__inline void adc_convert(uint8_t index) {
  // Reset interrupt and enable flag
  ADC10CTL0 &= ~(ADC10IFG | ENC);

  // Set new index
  adc_current_convert = index;

  // Select next channel
  ADC10CTL1 = (ADC10CTL1 & 0x0FFF) | ADC_CHANNEL[index];

  // Start ADC conversion
  ADC10CTL0 |= ENC // Enable conversion
      | ADC10SC; // Start conversion
}

__inline void adc_schedule_convert(uint8_t index) {
  // Reset start bit
  adc_scheduled_convert_start = 0x00;

  // Save current scheduled index
  adc_scheduled_convert = index;

  // Select current LED
  P3OUT &= ~LEDS;
  P3OUT |= ADC_LEDS[index];

  // Start timer
  TA0CTL |= MC_1;
}

void identify_color(void) {
  serialPrint("Current analyzed color: ");
  serialPrintInt(adc_color_components[0]);
  serialPrint(", ");
  serialPrintInt(adc_color_components[1]);
  serialPrint(", ");
  serialPrintInt(adc_color_components[2]);
  serialPrintln("");

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
      // d *= d; // Square the difference

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
