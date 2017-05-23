// (c) Tobias Faller 2017
// (c) Tim Maffenb(e)ier 2017

#include <msp430.h>
#include <stdint.h>

#include <templateEMP.h>

#include "shift_register.h"

// ----------------------------------------------------------------------------
// Definitions
// ----------------------------------------------------------------------------

#define POT BIT5
#define LDR BIT4
#define ANALOG_IN (POT | LDR)

#define LEDR BIT0
#define LEDG BIT1
#define LEDB BIT2
#define LEDS (LEDR | LEDG | LEDB)

#define ABS(x) (x > 0 ? x : -x)

// Maximum squared distance between color and measured color
#define COLOR_DIFF_THRESHOLD 25

// Number of measurement points to switch to new color
#define COLOR_DETECT_THRESHOLD 2

// #define DEBUG_COLORS

// ----------------------------------------------------------------------------
// ADC / Color definitions
// ----------------------------------------------------------------------------

#define ADC_VALUES 7
#define ADC_POTI_CONVERT 0xFF
#define ADC_SCHEDULE_EMPTY 0xFF
#define COLORS 5

static const uint8_t ADC_LEDS[ADC_VALUES] = {
  LEDR, LEDG, LEDB,
  LEDR | LEDG, LEDG | LEDB, LEDR | LEDB,
  LEDR | LEDG | LEDB
};

// Color names which are printed out to the console
static const char* COLOR_NAMES[COLORS + 1] = {
  "none",
  "red",
  "green",
  "blue",
  "white",
  "black"
};

// Color components averaged from 3 measurements per color
static const uint16_t COLOR_COMPONENTS[COLORS][7] = {
  { 235, 153, 137, 256, 201, 271, 317 }, // Red
  { 122, 169, 121, 195, 205, 168, 248 }, // Green
  { 122, 140, 181, 170, 231, 224, 273 }, // Blue
  { 247, 244, 234, 338, 334, 354, 426 }, // White
  { 121, 121, 114, 150, 164, 162, 211 } // Black
};

// ----------------------------------------------------------------------------
// Standard methods
// ----------------------------------------------------------------------------

__inline void setup(void);
__inline void loop(void);

// ----------------------------------------------------------------------------
// Methods
// ----------------------------------------------------------------------------

__inline void process_analog_value(uint8_t index, uint16_t value);
__inline void adc_convert(uint8_t index);
__inline void adc_schedule_convert(uint8_t index);
__inline void set_leds(uint8_t value);

void identify_color(void);
void report_new_color(uint8_t index);

// ----------------------------------------------------------------------------
// Fields
// ----------------------------------------------------------------------------

static uint16_t adc_current_convert;

static uint16_t adc_scheduled_convert;
static uint16_t adc_scheduled_convert_start;

static uint16_t adc_color_components[ADC_VALUES];

static uint8_t current_color;
static uint8_t new_color;
static uint16_t new_color_time;

/*
 * MSP430G2553
 *
 * Wiring:
 *   P1.4 K2 LDR
 *   P1.5 X6 U_POT (Potentiometer)
 *
 *   P3.0 K3 (red)
 *   P3.1 K4 (green)
 *   P3.2 X10 HEATER (blue)
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

  // Timer A compare
  // 1 MHz / 8 => 125 kHz
  // 125 kHz / 10 Hz => 12500
  // TA0CCR0 = 0x30D4;
  TA0CCR0 = 0x30D4;
  TA0R = 0; // Reset Timer A

  // Timer A control
  TA0CTL = TASSEL_2 // SMCLK as source (1 MHz)
      | ID_3 // Divider of 8
      | MC_0; // Disabled
  TA0CCTL0 = CCIE; // Enable interrupt

  // Initialize color recognition
  current_color = 0; // (NONE)
  new_color = 0; // (NONE)
  new_color_time = 0;

  // Initialize with empty schedule
  adc_scheduled_convert = ADC_SCHEDULE_EMPTY; // (NONE)
  adc_scheduled_convert_start = 0x00;

  // Schedule first color conversion
  adc_schedule_convert(0);

  // Start with first potentiometer readout
  adc_convert(ADC_POTI_CONVERT);
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
    adc_convert(ADC_POTI_CONVERT);
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
  case 0: // Red was measured
  case 1: // Green was measured
  case 2: // Blue was measured
  case 3: // Red + Green was measured
  case 4: // Green + Blue was measured
  case 5: // Red + Blue was measured
  case 6: // Red + Green + Blue was measured
    // Write analog value to temporary storage
    adc_color_components[index] = value;

    // Schedule next conversion
    // If we reached index 6 we start from 0
    adc_schedule_convert((index == 6) ? 0 : (index + 1));

    // Update current color since we read the last color component
    if (index == 6) {
      identify_color();
    }
    break;
  case ADC_POTI_CONVERT: // Potentiometer was measured
    /*
     * Explanation:
     *   We want our 10 bit value (0-1023) on a 5 value range
     *   => shift by 7 for a 3 bit range
     *   (value >> 7)
     *
     *   We want only 5 / 8 of this value
     *   (value * 5 / 8) >> 7
     *   = ((value * 4 + value) / 8) >> 7
     *   = ((value << 2 + value) / 8) >> 7
     *   = ((value << 2 + value) >> 3) >> 7
     *   = ((value << 2 + value) >> (3 + 7)
     */
    set_leds(((value << 2) + value) >> 10);
    break;
  }
}

__inline void set_leds(uint8_t value) {
  uint8_t display_value = 0x00;

  switch (value) {
  default:
  case 4:
    display_value |= 1 << 3;
  case 3:
    display_value |= 1 << 2;
  case 2:
    display_value |= 1 << 1;
  case 1:
    display_value |= 1 << 0;
  case 0:
    break;
  }

  set_shift_register_leds(display_value);
}

__inline void adc_convert(uint8_t index) {
  // Reset interrupt and enable flag
  ADC10CTL0 &= ~(ADC10IFG | ENC);

  // Set new index
  adc_current_convert = index;

  // Select next channel
  ADC10CTL1 = (ADC10CTL1 & 0x0FFF)
      | ((index == ADC_POTI_CONVERT) ? INCH_5 : INCH_4);

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

  // Start timer and wait until LED was on long enough
  TA0CTL |= MC_1;
}

void identify_color(void) {
#ifdef DEBUG_COLORS
  serialPrint("r: ");
  serialPrintInt(adc_color_components[0]);
  serialPrint(", g: ");
  serialPrintInt(adc_color_components[1]);
  serialPrint(", b: ");
  serialPrintInt(adc_color_components[2]);
  serialPrint(", r+g: ");
  serialPrintInt(adc_color_components[3]);
  serialPrint(", g+b: ");
  serialPrintInt(adc_color_components[4]);
  serialPrint(", r+b: ");
  serialPrintInt(adc_color_components[5]);
  serialPrint(", r+g+b: ");
  serialPrintInt(adc_color_components[6]);
  serialPrintln("");
#endif

  // Save color candidates
  int8_t candidates_size = 0;
  uint16_t canddidates[COLORS][2];

  int8_t color;
  for (color = COLORS - 1; color >= 0; --color) {
    uint16_t diff = 0;

    // Calculate the difference to comparing color
    int8_t i;
    for (i = 6; i >= 0; --i) {
      // Calculate difference of color component
      int16_t d = (int16_t) adc_color_components[i]
          - (int16_t) COLOR_COMPONENTS[color][i];
      d = ABS(d);

      if (d > (int16_t) COLOR_DIFF_THRESHOLD) {
#ifndef DEBUG_COLORS
        break;
#endif
      }

      diff += d;

      if (i == 0) {
#ifdef DEBUG_COLORS
        serialPrint("Color: ");
        serialPrint(COLOR_NAMES[color + 1]);
        serialPrint(", diff: ");
        serialPrintInt(diff);
        serialPrintln("");
#endif

        canddidates[candidates_size][0] = (uint16_t) color + 1;
        canddidates[candidates_size][1] = (uint16_t) diff;
        candidates_size++;
      }
    }
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

#ifdef DEBUG_COLORS
  serialPrint("Color: ");
  serialPrintln(COLOR_NAMES[color]);
#endif

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
