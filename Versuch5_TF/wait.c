// (c) Tobias Faller 2017
// (c) Tim Maffenb(e)ier 2017

#include <msp430.h>
#include <stdint.h>

#include "wait.h"

static uint8_t wait_cycles;
static void (*wait_callback)(void);

void wait(uint16_t time, void (*callback)(void)) {
  // Set wait cycles
  wait_cycles = time;
  wait_callback = callback;

  // Enable Timer A0 and disable unused features
  TA1CTL |= TACLR; // Reset timer

  // 1 MHz / 8 => 125 kHz
  // 125 kHz / 16Hz => 7812
  TA1CCR0 = 0x1E84; // Initialize frequency of 16 Hz

  // Enable Timer A1
  TA1CCTL0 = CCIE; // Enable interrupt
  TA1CCTL2 = 0; // Disable PWM mode
  TA1CTL =  TASSEL_2 // SMCLK as source (1 MHz)
      | ID_3 // Divide by 8
      | MC_1; // Count up
}

#pragma vector=TIMER1_A0_VECTOR
__interrupt void timer1(void) {
  if (wait_cycles > 0) {
    wait_cycles--;
    TA1CCTL0 &= ~CCIFG; // Reset interrupt flag
    return;
  }

  TA1CCTL0 &= ~(CCIFG | CCIE); // Deactivate interrupt

  // Call waiting function
  wait_callback();
}
