// (c) Tobias Faller 2017

#include <msp430.h>
#include <stdint.h>

#include <templateEMP.h>

#include "wdt.h"
#include "shift_register.h"

// ----------------------------------------------------------------------------
// Definitions
// ----------------------------------------------------------------------------

// Port P1
#define LEDG BIT0
#define BUTTON_HANG BIT3
#define NTC_IN BIT5

// Port P3
#define LEDR BIT2
#define REL_STAT BIT4
#define REL_DYN BIT5

#define BLINK_FREQUENCY 4
#define NTC_MIN 330
#define NTC_MAX 610

#define LIMIT(x,l,u) ((x > u) ? u : ((x < l) ? l : x))

// #define STATIC_HEATING

// ----------------------------------------------------------------------------
// Standard methods
// ----------------------------------------------------------------------------

__inline void setup(void);
__inline void loop(void);

// ----------------------------------------------------------------------------
// Methods
// ----------------------------------------------------------------------------

__inline int8_t hang();
__inline uint16_t measure_ntc(void);

void update_temp(void);
void update_heater(void);

// ----------------------------------------------------------------------------
// Fields
// ----------------------------------------------------------------------------

static uint16_t ntc_value = 0;

/*
 * MSP430G2553
 *
 * Wiring:
 *   P1.0 K4 (LED gn)
 *   P1.3 PB5
 *   P1.5 U_NTC
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
 *   P3.2 K3 (LED rt)
 *   P3.4 REL_STAT
 *   P3.5 REL_DYN
 *
 *   REL_ID HEATER
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
  // Configure port P1
  P1SEL &= ~(LEDG | BUTTON_HANG | NTC_IN); // Set as IO port
  P1SEL2 &= ~(LEDG | BUTTON_HANG | NTC_IN);

  P1DIR &= ~(BUTTON_HANG | NTC_IN); // Set button as input
  P1DIR |= LEDG; // Set LED as output

  P1REN &= ~(LEDG | NTC_IN); // Disable pull-up /-down for LED
  P1REN |= BUTTON_HANG; // Enable pull-up for button

  P1OUT |= BUTTON_HANG; // Set button to pull-up

  // Configure port P3
  P3SEL &= ~(LEDR | REL_STAT | REL_DYN); // Set as IO port
  P3SEL2 &= ~(LEDR | REL_STAT | REL_DYN);
  P3DIR |= (LEDR | REL_STAT | REL_DYN); // Set LED and relay as output
  P3REN &= ~(LEDR | REL_STAT | REL_DYN); // Disable pull-up /-down
  P3OUT &= ~(LEDR | REL_STAT | REL_DYN); // Write 0 to P3

  // Initialize the shift register
  shift_register_init();

  // Initialize TimerA
  TA0CTL = TASSEL_1 // Select low frequency timer ACLK
      | ID_3 // Select 8 as input divider
      | MC_0; // Set in stopped mode
  TA0CCTL0 = CM_0 // No capture
      | 0*CAP // Compare mode
      | CCIE; // Enable compare interrupt

#ifndef WATCHDOG_SLOW
  TACCR0 = 0x5DC;
#else
  TACCR0 = 0x177;
#endif

  // Initialize ADC10
  ADC10CTL0 = ADC10SHT_3 // 64 clocks as sample time
      | ADC10SR // Use 50 k samples per second
      | ADC10ON; // Activate ADC
  ADC10CTL1 = INCH_5 // Select pin 5 as input channel
      | ADC10DIV_0 // Select 1 as clock divider
      | ADC10SSEL0 // Use own internal oscillator
      | CONSEQ_0; // Select single conversion mode
  ADC10AE0 = BIT5; // Enable channel 5 as input

  // Initialize the WDT
  wdt_init();

  // Measure the start temperature
  update_temp();
  update_heater();

  // Schedule measurement
  TA0CTL |= MC_1; // Set to up mode
}

// Runs infinitely
__inline void loop(void) {
  // Switch LED
  P1OUT ^= LEDG;

  int16_t counter;
  for (counter = 50; (counter >= 0) | hang(); counter--) {
    // Set the heater to on / off
    update_heater();

    // Stop WDT from resetting the uController
    wdt_reset();

    __delay_cycles(1000000 / BLINK_FREQUENCY / 50 / 2);
  }
}

__inline int8_t hang() {
  if (!(P1IN & BUTTON_HANG)) {
    // Stop execution
    for (;;);
  }

  // Return 'false'
  return 0x00;
}

__inline uint16_t measure_ntc(void) {
  ADC10CTL0 |= ENC // Enable conversion
      | ADC10SC; // Start the conversion

  // Wait for ADC to finish
  while (ADC10CTL1 & ADC10BUSY);

  // Return read value
  return ADC10MEM;
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void timer(void) {
  // Update the temperature
  update_temp();

  // Reset interrupt flag
  TA0CCTL0 &= ~CCIFG;
}

void update_temp(void) {
  int16_t ntc_analog_value = (int16_t) measure_ntc();
  int16_t temp_value = (ntc_analog_value - NTC_MIN) * 6 / (NTC_MAX - NTC_MIN);
  ntc_value = LIMIT(temp_value, 0, 5);

  uint8_t leds = 0x00;
  switch (ntc_value) {
  case 5:
  case 4:
    leds |= 0x08;
  case 3:
    leds |= 0x04;
  case 2:
    leds |= 0x02;
  case 1:
    leds |= 0x01;
  case 0:
    // Set current temperature
    set_shift_register_leds(leds);

    // Set red LED if overheating
    if (ntc_value != 5)
      P3OUT &= ~LEDR;
    else
      P3OUT |= LEDR;
    break;
  }
}

void update_heater(void) {
  // Check if in range 4 or above
  if (ntc_value >= 4) {
    // Deactivate heater
#ifdef STATIC_HEATING
    P3OUT &= ~REL_STAT;
#endif
  } else {
    // Activate heater
#ifdef STATIC_HEATING
    P3OUT |= REL_STAT;
#else
    P3OUT ^= REL_DYN;
#endif
  }
}
