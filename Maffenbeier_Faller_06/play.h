// (c) Tobias Faller 2017
// (c) Tim Maffenb(e)ier 2017

#ifndef __PLAY_H
#define __PLAY_H

#include <msp430.h>
#include <stdint.h>

#define BUZZER BIT0

typedef struct {
  const uint16_t tone;
  const uint16_t length;
} tone_t;

typedef struct {
  const uint16_t length;
  const tone_t *notes;
} melody_t;

void play_melody(const melody_t *melody, void (*callback)(void));
void play_note(const tone_t *tone, void (*callback)(void));
void play_init(void);

#endif // !__PLAY_H
