// (c) Tobias Faller 2017
// (c) Tim Maffenbeier 2017

#ifndef __WDT_H
#define __WDT_H

/**
 * Initializes and starts the WDT.
 */
void
wdt_init(void);

/**
 * Resets the WDT to extend the computing-time.
 */
void
wdt_reset(void);

#endif // !__WDT_H
