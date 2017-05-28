// (c) Tobias Faller 2017

#ifndef __TIMER_H
#define __TIMER_H

#include <stdint.h>

#include "def.h"
#include "config.h"

// ----------------------------------------------------------------------------
// Types
// ----------------------------------------------------------------------------

typedef enum
{
  TIMER_1 = 0x00,
  TIMER_2 = 0x01
} timer_t;

typedef enum
{
  TIMER_DIVIDER_1 = 0x00,
  TIMER_DIVIDER_2 = 0x01,
  TIMER_DIVIDER_4 = 0x02,
  TIMER_DIVIDER_8 = 0x03
} timer_divider_t;

// ----------------------------------------------------------------------------
// Methods
// ----------------------------------------------------------------------------

/**
 * Initializes the selected timer. The timer is sourced from the 1MHz SMCLK
 * signal.
 *
 * @param timer The timer to initialize
 */
__inline void
timer_init (timer_t timer);

/**
 * Starts the selected timer.
 *
 * @param timer The timer to start
 */
__inline void
timer_start (timer_t timer);

/**
 * Stops the selected timer.
 *
 * @param timer The timer to stop
 */
__inline void
timer_stop (timer_t timer);

/**
 * Resets the selected timer.
 *
 * @param timer The timer to reset
 */
__inline void
timer_reset (timer_t timer);

/**
 * Returns if the timer is currently running.
 *
 * @param timer The timer to check
 * @return true if the timer is running
 */
__inline bool_t
timer_is_running (timer_t timer);

/**
 * Sets the interval of the selected timer.
 * If the timer is running an interrupt might be created if the current counter
 * is higher than new interval.
 *
 * @param timer The timer to set
 */
__inline void
timer_set_interval (timer_t timer, uint16_t interval);

/**
 * Returns the current interval of the selected timer.
 *
 * @param timer The timer to get
 */
__inline uint16_t
timer_get_interval (timer_t timer);

/**
 * Sets the divider for the selected timer.
 *
 * @param timer The timer to modify
 * @param divider The new divider to use
 */
__inline void
timer_set_divider (timer_t timer, timer_divider_t divider);

/**
 * Returns the current divider of the selected timer.
 *
 * @param timer The timer to get
 */
__inline timer_divider_t
timer_get_divider (timer_t timer);

/**
 * Sets the callback for the timer which is called on completion.
 * Pass a 0-pointer to deactivate the callback.
 *
 * @param timer The timer to modify
 * @param callback The callback which is called when the timer is triggered
 */
__inline void
timer_set_callback (timer_t timer, void (*callback)(void));

#endif // !__TIMER_H
