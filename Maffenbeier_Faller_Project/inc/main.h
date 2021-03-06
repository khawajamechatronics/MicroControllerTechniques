// (c) Tobias Faller 2017
// (c) Tim Maffenbeier 2017

#ifndef __MAIN_H
#define __MAIN_H

#include "buffer.h"

/**
 * Prints a welcome message to the console after initializing it.
 */
static bool_t
main_send_welcome (void);

/**
 * Displays the main game and exits the welcome screen.
 */
static void
main_view_game (void);

/**
 * Displays the highscore menu and exits the welcome screen.
 */
static void
main_view_highscore (void);

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

/**
 * Callback when a button gets pressed in the welcome screen.
 *
 * @param button The button which was pressed
 * @return true if the CPU should be resumed
 */
static bool_t
main_button_pressed (button_t button);

#endif // !__MAIN_H
