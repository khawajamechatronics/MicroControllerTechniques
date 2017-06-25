// (c) Tobias Faller 2017
// (c) Tim Maffenbeier 2017

#ifndef __TIMER_P_H
#define __TIMER_P_H

// ----------------------------------------------------------------------------
// Definitions
// ----------------------------------------------------------------------------

#define TIMER_COUNT 2
#define TIMER_DIVIDER_COUNT 4

// ----------------------------------------------------------------------------
// Methods
// ----------------------------------------------------------------------------

/**
 * Returns if this timer exists.
 *
 * @param timer The timer to check
 * @return true if the value is a valid timer
 */
__inline bool_t
timer_check (timer_t timer);

/**
 * Returns if this divider exists.
 *
 * @param divider The divider to check
 * @return true if the value is a valid divider
 */
__inline bool_t
timer_check_divider (timer_divider_t divider);

// ----------------------------------------------------------------------------
// Implementations
// ----------------------------------------------------------------------------

__inline bool_t
timer_check (timer_t timer)
{
  return (timer < TIMER_COUNT);
}

__inline bool_t
timer_check_divider (timer_divider_t divider)
{
  return (divider < TIMER_DIVIDER_COUNT);
}

#endif // !__TIMER_P_H
