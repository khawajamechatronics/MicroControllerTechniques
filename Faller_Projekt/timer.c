// (c) Tobias Faller 2017

#include "timer.h"

static volatile void (*timer_callbacks)(void)[TIMER_COUNT];

__inline void
timer_init (timer_t timer)
{
  if (!check_timer(timer))
    return;

  // Remove a pre-existing callback
  timer_callbacks[timer] = 0;

  switch (timer) {
  case TIMER_1:
    break;
  case TIMER_2:
    break;
  }
}

__inline void
timer_start (timer_t timer)
{
  if (!check_timer(timer))
    return;
}

__inline void
timer_stop (timer_t timer)
{
  if (!check_timer(timer))
    return;
}

__inline void
timer_reset (timer_t timer)
{
  if (!check_timer(timer))
    return;
}

__inline void
timer_is_running (timer_t timer)
{
  if (!check_timer(timer))
    return;
}

__inline void
timer_set_interval (timer_t timer, uint16_t interval)
{
  if (!check_timer(timer))
    return;
}

__inline void
timer_set_divider (timer_t timer, timer_divider_t divider)
{
  if (!check_timer(timer))
    return;
}

__inline void
timer_set_callback (timer_t timer, void (*callback)(void))
{
  if (!check_timer(timer))
    return;

  // Update the callback method
  timer_callbacks[timer] = callback;
}

