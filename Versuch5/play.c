// (c) Tobias Faller 2017

#include <msp430.h>
#include <stdint.h>

#include "play.h"

static void (*note_end_callback)(void);
static void (*melody_end_callback)(void);

void play_melody(const melody_t *melody, void (*callback)(void)) {
  // Set callback method
  melody_end_callback = callback;
}

void play_note(const tone_t *tone, void (*callback)(void)) {
  // Initialize with new tone
  TA0R = 0; // Reset timer to 0
  TA0CCR0 = tone->tone; // Initialize pulse width
  TA0CCR2 = (tone->tone >> 1); // 50% duty time
  TA0CCTL2 = OUTMOD_3; // Output mode 3 (Set / Reset)
  TA0CTL |= MC_1; // Count up

  // Set callback method
  note_end_callback = callback;

  // Initialize with new duration
  TA1R = 0; // Reset timer to 0
  TA1CCR0 = tone->length; // Initialize pulse length
  TA1CTL |= CCIE // Enable interrupt
      | MC_1; // Count up
}

void play_init(void) {
  // Initialize P3.6 as PWM output port
  P3SEL &= BUZZER; // Set as Timer0_A, compare: Out2 output
  P3SEL2 &= ~BUZZER;
  P3DIR |= BUZZER; // Set as output
  P3REN &= ~BUZZER; // No pull-up / -down
  P3OUT &= ~BUZZER; // Set to low

  // Initialize Timer A0
  TA0CTL = TASSEL_2 // SMCLK as source (1 MHz)
      | MC_0; // Disabled

  // Initialize Timer A1
  TA1CTL = TASSEL_2 // SMCLK as source (1 MHz)
      | ID_3 // Divider of 8
      | MC_0; // Disabled
}

#pragma vector=TIMER1_A0_VECTOR
__interrupt void timer(void) {
  // Stop note
  TA0CTL &= ~MC_1; // Disable Timer A0
  TA1CTL &= ~(MC_1 | CCIFG | CCIE); // Disable Timer A1

  // Call callback function
  note_end_callback();
}
