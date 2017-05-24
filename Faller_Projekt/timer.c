// (c) Tobias Faller 2017

#include <msp430.h>

#include "def.h"
#include "config.h"

#include "timer.h"

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

static volatile void (*timer_callbacks)(void)[TIMER_COUNT];

__inline void
timer_init (timer_t timer)
{
  if (!timer_check(timer))
    return;

  // Remove a possible existing callback
  timer_callbacks[timer] = 0;

  switch (timer) {
  case TIMER_1:
    TA0R = 0; // Reset counter
    TA0CTL = TASSEL_2; // Select SMCLK as source (1 MHz)
    break;
  case TIMER_2:
    TA1R = 0; // Reset counter
    TA1CTL = TASSEL_2; // Select SMCLK as source (1 MHz)
    break;
  }
}

__inline void
timer_start (timer_t timer)
{
  if (!timer_check(timer))
    return;

  switch (timer) {
  case TIMER_1:
    TA0CCTL0 = (TA0CCTL0 & ~CCIFG) | CCIE; // Enable interrupt and remove flag
    TA0CTL |= MC_1; // Enable counter
    break;
  case TIMER_2:
    TA1CCTL0 = (TA1CCTL0 & ~CCIFG) | CCIE; // Enable interrupt and remove flag
    TA1CTL |= MC_1; // Enable counter
    break;
  }
}

__inline void
timer_stop (timer_t timer)
{
  if (!timer_check(timer))
    return;

  switch (timer) {
  case TIMER_1:
    TA0CCTL0 &= ~(CCIE | CCIFG); // Disable interrupt
    TA0CTL &= ~(MC0 | MC1); // Disable counter
    break;
  case TIMER_2:
    TA1CCTL0 &= ~(CCIE | CCIFG); // Disable interrupt
    TA1CTL &= ~(MC0 | MC1); // Disable counter
    break;
  }
}

__inline void
timer_reset (timer_t timer)
{
  if (!timer_check(timer))
    return;

  switch (timer) {
  case TIMER_1:
    TA0R = 0;
    break;
  case TIMER_2:
    TA1R = 0;
    break;
  }
}

__inline void
timer_is_running (timer_t timer)
{
  if (!timer_check(timer))
    return;

  switch (timer) {
  case TIMER_1:
    return TA0CTL & (MC0 | MC1);
  case TIMER_2:
    return TA1CTL & (MC0 | MC1);
  }
}

__inline void
timer_set_interval (timer_t timer, uint16_t interval)
{
  if (!timer_check(timer))
    return;

  switch (timer) {
  case TIMER_1:
    TA0CCR0 = interval;
    break;
  case TIMER_2:
    TA1CCR0 = interval;
    break;
  }
}

__inline void
timer_set_divider (timer_t timer, timer_divider_t divider)
{
  if (!timer_check(timer))
    return;

  if (!timer_check_divider(divider))
    return;

  switch (timer) {
  case TIMER_1:
    TA0CTL = (TA0CTL & ~(ID0 | ID1)) | (divider << 6);
    break;
  case TIMER_2:
    TA1CTL = (TA1CTL & ~(ID0 | ID1)) | (divider << 6);
    break;
  }
}

__inline void
timer_set_callback (timer_t timer, void (*callback)(void))
{
  if (!timer_check(timer))
    return;

  // Update the callback method
  timer_callbacks[timer] = callback;
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void timer_int0(void) {
  TA0CCTL0 &= ~CCIFG; // Reset interrupt flag

  void (*callback)(void) = timer_callbacks[0];
  if (callback != 0)
    callback();
}

#pragma vector=TIMER1_A0_VECTOR
__interrupt void timer_int1(void) {
  TA1CCTL0 &= ~CCIFG; // Reset interrupt flag

  void (*callback)(void) = timer_callbacks[0];
  if (callback != 0)
    callback();
}

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
