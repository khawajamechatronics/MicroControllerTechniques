// (c) Tobias Faller 2017

#ifndef __TETRIS_P_H
#define __TETRIS_P_H

#include <stdint.h>

#include "inc/def.h"
#include "inc/config.h"

#include "inc/tetris.h"

// ----------------------------------------------------------------------------
// Definitions
// ----------------------------------------------------------------------------

#define TETRIS_FIELD_EMPTY 0x00
#define TETRIS_FIELD_UPDATED 0x80
#define TETRIS_FIELD_TEMP 0x40

// ----------------------------------------------------------------------------
// Types
// ----------------------------------------------------------------------------

typedef uint16_t findex_t;

// ----------------------------------------------------------------------------
// Constants
// ----------------------------------------------------------------------------

/*
 * Coordinates for each of the 4 states a tetromino can have.
 * Each coordinate is given in {x, y} format.
 * All tetromino blocks rotate clockwise.
 * The first entry is the initial state.
 */
static const int8_t I_TETROMINO[4][4][2] = { // 'I' tetromino
  {{-1, 0}, {0, 0}, {1, 0}, {2, 0}}, // Horiz
  {{0, -2}, {0, -1}, {0, 0}, {0, 1}}, // Vert
  {{-1, 0}, {0, 0}, {1, 0}, {2, 0}}, // Horiz
  {{0, -2}, {0, -1}, {0, 0}, {0, 1}} // Vert
};
static const int8_t T_TETROMINO[4][4][2] = { // 'T' tetromino
  {{-1, 0}, {0, 0}, {1, 0}, {0, 1}}, // Down
  {{0, -1}, {0, 0}, {0, 1}, {-1, 0}}, // Left
  {{-1, 0}, {0, 0}, {1, 0}, {0, -1}}, // Up
  {{0, -1}, {0, 0}, {0, 1}, {1, 0}} // Right
};
static const int8_t Z_TETROMINO[4][4][2] = { // 'Z' tetromino
  {{-1, -1}, {0, -1}, {0, 0}, {1, 0}}, // Horiz
  {{1, -1}, {1, 0}, {0, 0}, {0, -1}}, // Vert
  {{-1, -1}, {0, -1}, {0, 0}, {1, 0}}, // Horiz
  {{1, -1}, {1, 0}, {0, 0}, {0, -1}} // Vert
};
static const int8_t Z_INV_TETROMINO[4][4][2] = { // inverse 'Z' tetromino
  {{1, -1}, {0, -1}, {0, 0}, {-1, 0}}, // Horiz
  {{-1, -1}, {-1, 0}, {0, 0}, {1, 0}}, // Vert
  {{1, -1}, {0, -1}, {0, 0}, {-1, 0}}, // Horiz
  {{-1, -1}, {-1, 0}, {0, 0}, {1, 0}} // Vert
};
static const int8_t L_TETROMINO[4][4][2] = { // 'L' tetromino
  {{0, -1}, {0, 0}, {1, 0}, {2, 0}}, // Down
  {{-1, 0}, {0, 0}, {0, 1}, {0, 2}}, // Left
  {{-2, 0}, {-1, 0}, {0, 0}, {0, 1}}, // Up
  {{0, -2}, {0, -1}, {0, 0}, {-1, 0}} // Right
};
static const int8_t L_INV_TETROMINO[4][4][2] = { // inverse 'L' tetromino
  {{-2, 0}, {-1, 0}, {0, 0}, {0, 1}}, // Down
  {{-1, 0}, {0, 0}, {0, -1}, {0, -2}}, // Left
  {{0, -1}, {0, 0}, {1, 0}, {2, 0}}, // Up
  {{0, 0}, {1, 0}, {0, -1}, {0, -2}} // Right
};
static const int8_t O_TETROMINO[4][4][2] = { // '[]' tetromino
  {{0, 0}, {1, 0}, {1, 1}, {0, 1}},
  {{0, 0}, {1, 0}, {1, 1}, {0, 1}},
  {{0, 0}, {1, 0}, {1, 1}, {0, 1}},
  {{0, 0}, {1, 0}, {1, 1}, {0, 1}}
};

static const int8_t *TETROMINO[7] = {
  (const int8_t*) &I_TETROMINO,
  (const int8_t*) &T_TETROMINO,
  (const int8_t*) &Z_TETROMINO,
  (const int8_t*) &Z_INV_TETROMINO,
  (const int8_t*) &L_TETROMINO,
  (const int8_t*) &L_INV_TETROMINO,
  (const int8_t*) &O_TETROMINO
};

static const char TETROMINO_CHAR[7] = {
  TETRIS_TETROMINO_I,
  TETRIS_TETROMINO_T,
  TETRIS_TETROMINO_Z,
  TETRIS_TETROMINO_Z_INV,
  TETRIS_TETROMINO_L,
  TETRIS_TETROMINO_L_INV,
  TETRIS_TETROMINO_O
};

static const uint8_t TETROMINO_INIT_POS[7][2] = {
  {3, 1}, // 'I' tetromino
  {4, 1}, // 'T' tetromino
  {4, 1}, // 'Z' tetromino
  {4, 1}, // 'Z' inv tetromino
  {4, 1}, // 'L' tetromino
  {4, 1}, // 'L' inv tetromino
  {4, 1} // 'O' tetromino
};

// Pre-computed score result table
static const uint16_t SCORE_MULTIPLIER[20][4] = {
  {1, 2, 3, 4},
  {2, 4, 6, 8},
  {3, 6, 9, 12},
  {4, 8, 12, 16},
  {5, 10, 15, 20},
  {6, 12, 18, 24},
  {7, 14, 21, 28},
  {8, 16, 24, 32},
  {9, 18, 27, 36},
  {10, 20, 30, 40},
  {11, 22, 33, 44},
  {12, 24, 36, 48},
  {13, 26, 39, 52},
  {14, 28, 42, 56},
  {15, 30, 45, 60},
  {16, 32, 48, 64},
  {17, 34, 51, 68},
  {18, 36, 54, 72},
  {19, 38, 57, 76},
  {20, 40, 60, 80}
};

// ----------------------------------------------------------------------------
// Methods
// ----------------------------------------------------------------------------

// --- Game -------------------------------------------------------------------

/**
 * Updates the game field and drops the tetromino.
 *
 * @param tetris The main tetris instance
 */
static __inline bool_t
tetris_game_down (tetris_t *tetris);


/**
 * Increases the drop speed of all tetrominos.
 */
static __inline void
tetris_game_speedup (void);

/**
 * Returns a random tetromino.
 *
 * @return The next tetromino
 */
static __inline tetromino_t
tetris_pick_random_tetromino (void);

/**
 * Clears all full lines in the field and drops all lines above.
 *
 * @param field The field to clear full lines
 * @return Number of cleared lines
 */
static __inline uint8_t
tetris_field_clear_full_lines (field_t *field);

/**
 * Updates the current score.
 *
 * @param tetris The main tetris instance
 * @param cleared The number of cleared lines with the used tetromino
 * @param t_spin true if the last executed (non-drop) action is a t-spin
 */
static __inline void
tetris_game_update_score (tetris_t *tetris, uint8_t cleared, bool_t t_spin);

/**
 * Checks if the tetromino intersects with the current game field.
 * If a field of the tetromino is out of bounds true is returned.
 *
 * @param field The current game field
 * @param tetromino The tetromino to test
 * @param x The x position of the tetromino
 * @param y The y position of the tetromino
 * @param rotation The rotation of the tetromino (range 0 to 4)
 * @return true if the tetromino intersects
 */
static bool_t
tetris_game_check_collision (field_t *field, tetromino_t tetromino,
                             uint8_t x, uint8_t y, uint8_t rotation);

/**
 * Puts the tetromino at the specified position of the field.
 * If the tetromino is out of bounds only the valid part is set.
 *
 * @param field The game field to use
 * @param tetromino The tetromino to set at the position
 * @param x The x position of the tetromino
 * @param y The y position of the tetromino
 * @param rotation The rotation of the tetromino (range 0 to 4)
 * @param flags The flags to apply to the tetromino
 */
static void
tetris_game_place_tetromino (field_t *field, tetromino_t tetromino,
                             uint8_t x, uint8_t y, uint8_t rotation,
                             uint8_t flags);

// --- Callback methods -------------------------------------------------------

/**
 * Callback method for drop timer.
 */
static bool_t
tetris_on_timer (void);

/**
 * Callback method for key presses which get finally queued.
 *
 * @param buffer The buffer to read key data from
 */
static bool_t
tetris_on_key (uart_buffer_t *buffer);

/**
 * Callback method for user / timer created command.
 *
 * @param command The command to execute
 */
static void
tetris_on_command (tetris_command_t command);

// --- Field ------------------------------------------------------------------

/**
 * Returns the current game field to work on.
 *
 * @param tetris The main tetris instance
 * @return A pointer to the current game field
 */
static __inline field_t*
tetris_field_get_current (tetris_t *tetris);

/**
 * Clears the update flags of all fields.
 *
 * @param tetris The main tetris instance
 */
static __inline void
tetris_field_update (tetris_t *tetris);

// --- Item -------------------------------------------------------------------

/**
 * Returns a pointer to the location on the tetris field.
 *
 * @param field The current tetris field
 * @param index The index of the item
 * @return The item at the specified position
 */
static __inline field_item_t*
tetris_field_item_get (field_t *field, findex_t index);

/**
 * Returns a pointer to the location on the tetris field.
 *
 * @param field The current tetris field
 * @param x The x coordinate
 * @param y The y coordinate
 * @return The item at the specified position
 */
static __inline field_item_t*
tetris_field_item_get_at (field_t *field, uint8_t x, uint8_t y);

/**
 * Returns the index of the specified location.
 *
 * @param x The x coordinate
 * @param y The y coordinate
 * @return The index to access the item
 */
static __inline findex_t
tetris_field_item_get_index (uint8_t x, uint8_t y);

/**
 * Updates the item on the field.
 * The updated flag is set if the new value and old value differ.
 *
 * @param field The field to update
 * @param index The index of the item
 * @param value The new value
 */
static __inline void
tetris_field_item_set (field_t *field, findex_t index,
                       field_item_t value);

/**
 * Returns if the location on the tetris field is empty.
 *
 * @param item A pointer to the location on the field
 * @return true if the spot is occupied
 */
static __inline bool_t
tetris_field_item_is_empty (field_item_t *item);

/**
 * Sets the updated flag of the item.
 *
 * @param item The item to update
 */
static __inline void
tetris_field_item_set_updated (field_item_t *item);

/**
 * Sets the temporary flag of the item.
 *
 * @param item The item to update
 */
static __inline void
tetris_field_item_set_temp (field_item_t *item);


/**
 * Clears the updated flag of the item.
 *
 * @param item The item to update
 */
static __inline void
tetris_field_item_clear_updated (field_item_t *item);

/**
 * Clears the temporary flag of the item.
 *
 * @param item The item to update
 */
static __inline void
tetris_field_item_clear_temp (field_item_t *item);

/**
 * Returns
 *
 * @param item
 * @return
 */
static __inline tetromino_t
tetris_field_item_get_tetromino (field_item_t *item);

// --- UART IO ----------------------------------------------------------------

/**
 * Sends the current field to the user including score and next tetrominos.
 *
 * @param tetris The main tetris instance
 */
static void
tetris_game_send (tetris_t *tetris);

/**
 * Sends only the current game field to the user.
 *
 * @param tetris The main tetris instance
 */
static __inline void
tetris_game_send_field (tetris_t *tetris);

/**
 * Sends only the current score to the user.
 *
 * @param tetris The main tetris instance
 */
static __inline void
tetris_game_send_score (tetris_t *tetris);

// --- Helper -----------------------------------------------------------------

/**
 * Checks if the coordinate is in the tetris field.
 *
 * @param x The x location
 * @param y The y location
 * @return true if the coordinate is valid
 */
__attribute__((always_inline))
__inline bool_t
tetris_check_bounds (int8_t x, int8_t y);

// ----------------------------------------------------------------------------
// Implementations
// ----------------------------------------------------------------------------

__attribute__((always_inline))
__inline bool_t
tetris_check_bounds (int8_t x, int8_t y)
{
  return ((x >= 0) && (y >= 0) && (x < TETRIS_WIDTH) && (y < TETRIS_HEIGHT));
}

#endif // !__TETRIS_P_H
