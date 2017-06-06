// (c) Tobias Faller 2017

#ifndef __BUTTONS_H
#define __BUTTONS_H

#include <stdint.h>

#include "def.h"
#include "config.h"

#define BUTTON_COUNT 6
#define BUTTON_STATE_PRESSED 0x8000
#define BUTTON_TIMER_MASK 0x7FFF

// Re-enable trigger after 10ms
#define BUTTON_WAIT_TIME 10

typedef enum {
  BUTTON_1 = 0x00,
  BUTTON_2 = 0x01,
  BUTTON_3 = 0x02,
  BUTTON_4 = 0x03,
  BUTTON_5 = 0x04,
  BUTTON_6 = 0x05
} button_t;

typedef struct {
  uint16_t state[BUTTON_COUNT];

  bool_t (*callback)(button_t);
} buttons_t;

/**
 * Initializes the data structure to hold all button presses
 * and enables the necessary interrupts.
 * This implementation uses TIMER_2 and polling to retrieve the button
 * state.
 *
 * @param buttons The data structure to use
 */
void
buttons_init (buttons_t *buttons);

/**
 * Sets the callback which gets notified if a button gets pressed.
 * The parameter passed to the callback is the number of pressed button.
 *
 * @param callback The callback to execute
 */
void
buttons_set_callback (bool_t (*callback)(button_t));

#endif // !__BUTTONS_H
