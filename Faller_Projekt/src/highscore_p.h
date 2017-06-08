// (c) Tobias Faller 2017

#ifndef __HIGHSCORE_P_H
#define __HIGHSCORE_P_H

#include <stdint.h>

#include "inc/highscore.h"
#include "inc/buttons.h"

// ----------------------------------------------------------------------------
// Methods
// ----------------------------------------------------------------------------

/**
 * Exits the highscore view via WDT reset.
 */
static __inline void
highscore_exit (void);

/**
 * Resets both memory segments which hold the highscore data.
 */
static void
highscore_reset (void);

/**
 * Returns true if the character is in the allowed range and can be displayed.
 *
 * @param c The character to check
 * @return true if the character is valid
 */
static __inline bool_t
highscore_is_char_allowed (uint8_t c);

/**
 * Returns pointer to the current data segments.
 * The newer segment is written to the parameter current.
 * The older segment is cleared and written to the parameter next.
 *
 * @param current The current data segment
 * @param next The next data segment to write to
 */
static __inline void
highscore_get_areas (highscore_t **current, highscore_t **next);

/**
 * Updates the highscore table by merging the new score.
 *
 * @param current The current highscore table
 * @param next The highscore table to write to
 */
static __inline void
highscore_update (highscore_t *current, highscore_t *next);

/**
 * Displays the name input dialog.
 */
static __inline void
highscore_show_input_dialog (void);

/**
 * Shows the clear highscore dialog.
 */
static __inline void
highscore_show_clear_dialog (void);

/**
 * Lists the highscore table.
 */
static __inline void
highscore_show_scoreboard (void);

/**
 * Helper method to draw the line of a box.
 *
 * @param count The inner size of the box
 * @param edge The character to use for the edges
 * @param fill The character to use for the fill area
 */
static __inline void
highscore_send_boxline (uint8_t count, uint8_t edge, uint8_t fill);

/**
 * Returns the next usable character for name entry.
 *
 * @param c The current character used
 * @return The next character
 */
static __inline uint8_t
highscore_next_char (uint8_t c);

/**
 * Returns the previous usable character for name entry.
 *
 * @param c The current character used
 * @return The previous character
 */
static __inline uint8_t
highscore_prev_char (uint8_t c);

/**
 * Callback method for a UART key press.
 *
 * @param buffer The buffer which holds the data
 * @return true if the CPU should be woken up
 */
static bool_t
highscore_on_key (buffer_t *buffer);

/**
 * Callback method for a button press.
 *
 * @param button The button which was pressed
 * @return true if the CPU should be woken up
 */
static bool_t
highscore_on_button (button_t button);

#endif // !__HIGHSCORE_P_H
