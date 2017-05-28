// (c) Tobias Faller 2017

#ifndef __MAIN_H
#define __MAIN_H

#include "uart_buffer.h"

/**
 * Prints a welcome message to the console after initializing it.
 */
static void
main_send_welcome (void);

/**
 * Start the tetris game.
 */
static void
main_start_game (void);

/**
 * Callback which gets called if UART data was received in the welcome
 * screen.
 *
 * @param buffer The buffer with the received data
 */
static void
main_uart_received (uart_buffer_t *buffer);

#endif // !__MAIN_H
