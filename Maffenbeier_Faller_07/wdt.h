// (c) Tobias Faller 2017
// (c) Tim Maffenb(e)ier 2017

#ifndef __WDT_H
#define __WDT_H

#define WATCHDOG_SLOW

__inline void wdt_init(void);
__inline void wdt_reset(void);

#endif // !__WDT_H
