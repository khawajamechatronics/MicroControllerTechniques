// (c) Tobias Faller 2017

#ifndef __MAIN_H
#define __MAIN_H

#include "buffer.h"

/**
 * Prints a welcome message to the console after initializing it.
 */
static bool_t
main_send_welcome (void);

/**
 * Start the tetris game.
 */
static void
main_start_game (void);

/**
 * Callback which gets called if UART data was received in the welcome
 * screen.
 * Returns true if the CPU is resumed.
 *
 * @param buffer The buffer with the received data
 * @return true if the CPU should be resumed
 */
static bool_t
main_uart_received (buffer_t *buffer);

#endif // !__MAIN_H
