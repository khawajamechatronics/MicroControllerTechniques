// (c) Tobias Faller 2017

#ifndef __WDT_H
#define __WDT_H

#include <msp430.h>
#include <stdint.h>

#include "inc/def.h"
#include "inc/config.h"

#include "inc/wdt.h"

void
wdt_init (void)
{
  // Configure auxiliary clock
  // VLOCLK = 12 kHz => ACLK = VLOCLK / 1 = 12 kHz
  BCSCTL1 = (BCSCTL1 & ~XTS) // Don't use crystal
      | DIVA_0; // Divider of 1 for ACLK

  BCSCTL3 = (BCSCTL3 & ~LFXT1S_3) | LFXT1S_2; // VLOCLK as low-freq clock

  /*
   * Configure the watchdog timer
   * ACLK = 6 kHz => ACLK / 64 = 93,75 (T = 10ms)
   */
  WDTCTL = WDTPW // Watchdog password
      | WDTSSEL // Select ACLK as clock source
      | WDTCNTCL // Reset timer
      | (WDTIS0 + WDTIS1); // Divider of 64
}

void
wdt_reset (void)
{
  WDTCTL = (WDTCTL & 0xFF) | (WDTPW | WDTCNTCL);
}

#endif
