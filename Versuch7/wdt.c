#include <msp430.h>
#include <stdint.h>

#include "wdt.h"

__inline void wdt_init(void) {
  // Configure auxiliary clock
  // VLOCLK = 12 kHz => ACLK = VLOCLK / 2 = 6 kHz
  // VLOCLK = 12 kHz => ACLK = VLOCLK / 8 = 1,5 kHz
  BCSCTL1 = (BCSCTL1 & ~XTS) // Don't use crystal
#ifndef WATCHDOG_SLOW
      | DIVA_1; // Divider of 2 for ACLK
#else
      | DIVA_3; // Divider of 8 for ACLK
#endif
  BCSCTL3 = (BCSCTL3 & ~LFXT1S_3) | LFXT1S_2; // VLOCLK as low-freq clock

  /*
   * Configure the watchdog timer
   * ACLK = 6 kHz => ACLK / 32768 = 0,183 ~= 0,25 (T = 4s)
   * ACLK = 1,5 kHz => ACLK / 32768 = 0,0458 ~= 0,05 (T = 20s)
   */
  WDTCTL = WDTPW // Watchdog password
      | WDTSSEL // Select ACLK as clock source
      | WDTCNTCL // Reset timer
      | 0*(WDTIS0 | WDTIS1); // Divider of 32768
}

__inline void wdt_reset(void) {
  WDTCTL = (WDTCTL & 0xFF) | (WDTPW | WDTCNTCL);
}
