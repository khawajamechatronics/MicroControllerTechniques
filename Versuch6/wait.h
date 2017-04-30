// (c) Tobias Faller 2017

#ifndef _WAIT_H_
#define _WAIT_H_

#include <msp430.h>
#include <stdint.h>

/**
 * Waits the specified time x 1/16 seconds.
 *
 * @param time The time to wait
 * @param callback The called function after the time expired
 */
void wait(uint16_t time, void (*callback)(void));

#endif
