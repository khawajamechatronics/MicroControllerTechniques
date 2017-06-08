// (c) Tobias Faller 2017

#include <stdint.h>
#include <msp430.h>

#include "inc/def.h"
#include "inc/config.h"

#include "inc/shift_register.h"
#include "inc/buttons.h"
#include "inc/timer.h"

#include "buttons_p.h"

static buttons_t *state;

void
buttons_init (buttons_t *buffer)
{
  state = buffer;

  // Initialize shift register
  shift_register_init();

  // Initialize button inputs
  P1REN |= BIT3 | BIT4; // Enable pull-up / down resistors
  P1OUT |= BIT3 | BIT4; // Set to pull-up

  // Load initial states
  uint8_t sr_state = shift_register_get_buttons();
  for (uint8_t i = BUTTON_COUNT; i-- > 0;)
  {
    switch (i)
    {
    case BUTTON_1:
    case BUTTON_2:
    case BUTTON_3:
    case BUTTON_4:
      buffer->state[i] = ((sr_state >> (3 - i)) & 0x01)
          ? BUTTON_STATE_PRESSED : 0;
      break;
    case BUTTON_5:
      buffer->state[i] = (P1IN & BIT3) ? 0 : BUTTON_STATE_PRESSED;
      break;
    case BUTTON_6:
      buffer->state[i] = (P1IN & BIT4) ? 0 : BUTTON_STATE_PRESSED;
      break;
    }
  }

  // Disable callback function
  state->callback = 0;

  // Initialize timer to check the buttons every 50 ms
  // This timer is also used to seed the RNG
  timer_init(TIMER_2);
  timer_set_divider(TIMER_2, TIMER_DIVIDER_1);
  timer_set_interval(TIMER_2, 0xC350); // Every 50 ms
  timer_set_callback(TIMER_2, &buttons_on_timer2);
  timer_start(TIMER_2);
}

void
buttons_set_callback (bool_t (*callback)(button_t))
{
  state->callback = callback;
}

bool_t
buttons_on_timer2 (void)
{
  // Enable reentrant interrupts for UART
  __enable_interrupt();

  uint8_t wake_cpu = 0x00;

  uint8_t sr_state = shift_register_get_buttons();
  for (uint8_t i = BUTTON_COUNT; i-- > 0;)
  {
    if (!(state->state[i] & BUTTON_TIMER_MASK))
    {
      uint16_t old_state = state->state[i] & BUTTON_STATE_PRESSED;
      uint16_t new_state;
      switch (i)
      {
      case BUTTON_1:
      case BUTTON_2:
      case BUTTON_3:
      case BUTTON_4:
        new_state = (sr_state >> (3 - i)) & 0x01;
        break;
      case BUTTON_5:
        new_state = (P1IN & BIT3) ? 0 : 1;
        break;
      case BUTTON_6:
        new_state = (P1IN & BIT4) ? 0 : 1;
        break;
      }

      // Did the state change?
      if ((old_state && !new_state) || (!old_state && new_state)) {
        if (new_state)
        {
          state->state[i] = BUTTON_WAIT_TIME | BUTTON_STATE_PRESSED;

          // Notify listener
          if (state->callback)
            wake_cpu |= state->callback((button_t) i);
        }
        else
          state->state[i] = BUTTON_WAIT_TIME;
      }
    } else {
      state->state[i]--;
    }
  }

  return wake_cpu;
}
