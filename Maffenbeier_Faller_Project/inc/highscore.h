// (c) Tobias Faller 2017

#ifndef __HIGHSCORE_H
#define __HIGHSCORE_H

// No score => Not on scoreboard
#define HIGHSCORE_SHOW 0x00000000

// Flash gets erased to all 1s => uint8_t <=> 0xFF
#define HIGHSCORE_SEGMENT_EMPTY 0xFF

/**
 * Struct to hold a highscore entry comprising of a name and a score.
 */
typedef struct {
  uint32_t score;
  uint8_t name[HIGHSCORE_NAME_LENGTH];
  uint8_t name_length;
} __attribute__((packed)) highscore_entry_t;

/**
 * Struct to store highscore entries in sorted order.
 * The initialized field is set to 0xFF if the flash page is cleared.
 */
typedef struct {
  highscore_entry_t entries[HIGHSCORE_LENGTH];
  uint8_t entry_count;
  uint8_t initialized;
} __attribute__((packed)) highscore_t;

/**
 * Struct to store the current working data of the highscore view.
 */
typedef struct {
  bool_t clear_shown;
  bool_t enter_name_shown;

  highscore_entry_t new_entry;

  uint8_t next_id;
  highscore_t *current_segment;
  highscore_t *next_segment;
} highscore_state_t;

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
highscore_init (uint32_t score, highscore_state_t *working_area);

/**
 * Displays the current highscore view.
 * If the user is entering a name the text field is displayed too.
 */
void
highscore_process (void);

#endif // !__HIGHSCORE_H
