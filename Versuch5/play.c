// (c) Tobias Faller 2017

#include <msp430.h>
#include <stdint.h>

#include "play.h"

void resume_melody(void);

static void (*note_end_callback)(void);
static void (*melody_end_callback)(void);

static uint16_t current_note;
static uint16_t current_note_length;
static const melody_t *current_melody;

void play_melody(const melody_t *melody, void (*callback)(void)) {
  // Set callback method
  melody_end_callback = callback;

  // Reset melody
  current_note = 0;
  current_melody = melody;

  if (melody->length > 0) {
    play_note(&melody->notes[0], &resume_melody);
  }
}

void resume_melody(void) {
  if (++current_note < current_melody->length) {
    // Play next note
    play_note(&current_melody->notes[current_note], &resume_melody);
  } else {
    // Signal end of melody
    melody_end_callback();
  }
}

void play_note(const tone_t *tone, void (*callback)(void)) {
  // Set callback method
  note_end_callback = callback;

  // Save current length
  current_note_length = tone->length;

  // Initialize with new tone
  TA0R = 0; // Reset timer to 0
  TA0CCR0 = tone->tone; // Initialize pulse width
  TA0CCR2 = (tone->tone >> 1); // 50% duty time
  TA0CCTL2 = OUTMOD_3; // Output mode 3 (Set / Reset)
  TA0CTL |= MC_1; // Count up

  // Initialize with new duration
  TA1R = 0; // Reset timer to 0
  TA1CTL |= MC_1; // Count up
  TA1CCTL0 |= CCIE; // Enable interrupt
}

void play_init(void) {
  // Initialize P3.6 as PWM output port
  P3SEL |= BUZZER; // Set as Timer0_A, compare: Out2 output
  P3SEL2 &= ~BUZZER;
  P3DIR |= BUZZER; // Set as output
  P3REN &= ~BUZZER; // No pull-up / -down

  // Initialize Timer A0
  TA0CTL = TASSEL_2 // SMCLK as source (1 MHz)
      | ID_0 // Divider of 1
      | MC_0; // Disabled

  // Initialize Timer A1
  TA1CTL = TASSEL_2 // SMCLK as source (1 MHz)
      | ID_3 // Divider of 8
      | MC_0; // Disabled

  // 1 MHz / 8 => 125 kHz
  // 125 kHz / 16Hz => 7812
  TA1CCR0 = 0x1E84; // Initialize frequency of 16 Hz
}

#pragma vector=TIMER1_A0_VECTOR
__interrupt void timer(void) {
  if (current_note_length > 0) {
    current_note_length--;
    TA1CCTL0 &= ~CCIFG; // Reset interrupt
    return; // Continue note
  }

  // Stop note
  TA0CTL &= ~MC_1; // Disable Timer A0
  TA0CTL &= ~MC_1; // Disable Timer A1
  TA1CCTL0 &= ~(CCIFG | CCIE); // Disable Timer A1 interrupt

  // Call callback function
  note_end_callback();
}
