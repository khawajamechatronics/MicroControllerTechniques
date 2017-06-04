// (c) Tobias Faller 2017

#ifndef __HIGHSCORE_P_H
#define __HIGHSCORE_P_H

/**
 * Exits the highscore view via WDT reset.
 */
void
highscore_exit (void);

/**
 * Resets memory segment b and / or c.
 *
 * @param seg_b true if segment b is cleared
 * @param seg_c true if segment c is cleared
 */
void
highscore_reset (bool_t seg_b, bool_t seg_c);

/**
 * Callback function for user input.
 *
 * @param buffer The buffer to read from
 * @return true to wake up the CPU
 */
bool_t
highscore_on_key (buffer_t *buffer);

#endif // !__HIGHSCORE_P_H
