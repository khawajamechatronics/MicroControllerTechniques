// (c) Tobias Faller 2017

#ifndef _TIMER
#define _TIMER

#include <stdint.h>

#include "def.h"
#include "config.h"

// ----------------------------------------------------------------------------
// Definitions
// ----------------------------------------------------------------------------

#define TIMER_COUNT 2
#define TIMER_DIVIDER_COUNT 4

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
 * Initializes the selected timer.
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
 */
__inline void
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
 * Sets the divider for the selected timer.
 *
 * @param timer The timer to modify
 * @param divider The new divider to use
 */
__inline void
timer_set_divider (timer_t timer, timer_divider_t divider);

/**
 * Sets the callback for the timer which is called on completion.
 * Pass a 0-pointer to deactivate the callback.
 *
 * @param timer The timer to modify
 * @param callback The callback which is called when the timer is triggered
 */
__inline void
timer_set_callback (timer_t timer, void (*callback)(void));

#endif //!_TIMER
