// (c) Tobias Faller 2017
// (c) Tim Maffenb(e)ier 2017

#include <msp430.h>
#include <stdint.h>

#include "play.h"
#include "wait.h"

void resume_melody(void);

static void (*melody_end_callback)(void);

static uint16_t current_note;
static const melody_t *current_melody;

void play_melody(const melody_t *melody, void (*callback)(void)) {
  // Set callback method
  melody_end_callback = callback;

  // Reset melody
  current_note = 0;
  current_melody = melody;

  // Initialize Timer A0
  TA0CTL = TASSEL_2 // SMCLK as source (1 MHz)
      | ID_0 // Divider of 1
      | MC_0; // Disabled

  if (melody->length > 0) {
    play_note(&melody->notes[0], &resume_melody);
  }
}

void resume_melody(void) {
  if (++current_note < current_melody->length) {
    const tone_t *next_note = &current_melody->notes[current_note];

    // Pause instead of note?
    if (next_note->tone == 0) {
      // Just wait the respective time
      wait(next_note->length, &resume_melody);
    } else {
      // Play next note
      play_note(next_note, &resume_melody);
    }
  } else {
    // Disable Timer A0
    TA0CTL &= MC_1;

    // Signal end of melody
    melody_end_callback();
  }
}

void play_note(const tone_t *tone, void (*callback)(void)) {
  // Initialize with new tone
  TA0R = 0; // Reset timer to 0
  TA0CCR0 = tone->tone; // Initialize pulse width
  TA0CCR2 = (tone->tone >> 1); // 50% duty time
  TA0CCTL2 = OUTMOD_3; // Output mode 3 (Set / Reset)
  TA0CTL |= MC_1; // Count up

  wait(tone->length, callback);
}

void play_init(void) {
  // Initialize P3.6 as PWM output port
  P3SEL |= BUZZER; // Set as Timer0_A, compare: Out2 output
  P3SEL2 &= ~BUZZER;
  P3DIR |= BUZZER; // Set as output
  P3REN &= ~BUZZER; // No pull-up / -down
}
