// (c) Tobias Faller 2017

#ifndef __HIGHSCORE_H
#define __HIGHSCORE_H

#define HIGHSCORE_SHOW 0xFFFFFFFF

/**
 * Struct to hold a highscore entry comprising of a name and a score.
 */
typedef struct {
  uint8_t name[HIGHSCORE_NAME_LENGTH];
  uint32_t score;
} highscore_entry_t;

/**
 * Struct to store highscore entries in sorted order.
 * The initialized field is set to 0 if the flash page is cleared.
 */
typedef struct {
  bool_t initialized;
  highscore_entry_t entries[HIGHSCORE_LENGTH];
} highscore_t;

/**
 * Initializes the highscore list and shows a 'enter your name' dialog
 * if the score value is not HIGHSCORE_SHOW and not outside the score list.
 * If HIGHSCORE_SHOW is provided as argument only the current highscore
 * list is shown.
 *
 * @param score The new score or HIGHSCORE_SHOW
 * @param working_area The data are to temporarily store the received
 *        player name
 */
void
highscore_init (uint32_t score, highscore_entry_t *working_area);

/**
 * Displays the current highscore view.
 * If the user is entering a name the text field is displayed too.
 */
void
highscore_process (void);

#endif // !__HIGHSCORE_H
