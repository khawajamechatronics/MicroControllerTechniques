// (c) Tobias Faller 2017

#include <msp430.h>
#include <stdint.h>

#include "inc/def.h"
#include "inc/config.h"

#include "inc/timer.h"

#include "timer_p.h"

static bool_t (*timer_callbacks[TIMER_COUNT])(void);

void
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

void
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

void
timer_start_counter (timer_t timer)
{
  if (!timer_check(timer))
    return;

  switch (timer) {
  case TIMER_1:
    TA0CTL |= MC_1; // Enable counter
    break;
  case TIMER_2:
    TA1CTL |= MC_1; // Enable counter
    break;
  }
}

void
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

void
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

bool_t
timer_is_running (timer_t timer)
{
  if (!timer_check(timer))
    return 0;

  switch (timer) {
  case TIMER_1:
    return TA0CTL & (MC0 | MC1);
  case TIMER_2:
    return TA1CTL & (MC0 | MC1);
  default:
    return 0;
  }
}

void
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

uint16_t
timer_get_interval (timer_t timer)
{
  if (!timer_check(timer))
    return 0;

  switch (timer) {
  case TIMER_1:
    return TA0CCR0;
  case TIMER_2:
    return TA1CCR0;
  default:
    return 0;
  }
}

void
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

timer_divider_t
timer_get_divider (timer_t timer)
{
  if (!timer_check(timer))
    return TIMER_DIVIDER_1;

  switch (timer) {
  case TIMER_1:
    return (timer_divider_t) (TA0CTL & (ID0 | ID1)) >> 6;
  case TIMER_2:
    return (timer_divider_t) (TA1CTL & (ID0 | ID1)) >> 6;
  default:
    return TIMER_DIVIDER_1;
  }
}

uint16_t
timer_get_value (timer_t timer)
{
  if (!timer_check(timer))
    return 0;

  switch (timer) {
  case TIMER_1:
    return TA0R;
  case TIMER_2:
    return TA1R;
  default:
    return 0;
  }
}

void
timer_set_callback (timer_t timer, bool_t (*callback)(void))
{
  if (!timer_check(timer))
    return;

  // Update the callback method
  timer_callbacks[timer] = callback;
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void
timer_int0 (void)
{
  bool_t (*callback)(void);

  TA0CCTL0 &= ~CCIFG; // Reset interrupt flag

  callback = timer_callbacks[0];
  if (callback != 0 && callback())
    __bic_SR_register_on_exit(CPUOFF);
}

#pragma vector=TIMER1_A0_VECTOR
__interrupt void
timer_int1 (void)
{
  bool_t (*callback)(void);

  TA1CCTL0 &= ~CCIFG; // Reset interrupt flag

  callback = timer_callbacks[1];
  if (callback != 0 && callback())
    __bic_SR_register_on_exit(CPUOFF);
}
