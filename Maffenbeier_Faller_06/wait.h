// (c) Tobias Faller 2017
// (c) Tim Maffenb(e)ier 2017

#ifndef __WAIT_H
#define __WAIT_H

#include <msp430.h>
#include <stdint.h>

/**
 * Waits the specified time x 1/16 seconds.
 *
 * @param time The time to wait
 * @param callback The called function after the time expired
 */
void wait(uint16_t time, void (*callback)(void));

#endif // !__WAIT_H
