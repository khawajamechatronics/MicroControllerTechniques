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

// ----------------------------------------------------------------------------
// Methods
// ----------------------------------------------------------------------------

// --- Game -------------------------------------------------------------------

/**
 * Updates the game field and drops the tetromino.
 *
 * @param tetris The main tetris instance
 */
static __inline void
tetris_game_drop (tetris_t *tetris);

/**
 * Sends the current field to the user including score and next tetrominos.
 *
 * @param tetris The main tetris instance
 */
static void
tetris_game_send (tetris_t *tetris);

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
 * Callback method for drop timer.
 */
static void
tetris_on_timer (void);

/**
 * Updates the current score.
 *
 * @param tetris The main tetris instance
 * @param cleared The number of cleared lines with the used tetromino
 */
static __inline void
tetris_game_update_score (tetris_t *tetris, uint8_t cleared);

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
tetris_field_item_get_at (field_t *field, findex_t x, findex_t y);

/**
 * Returns the index of the specified location.
 *
 * @param x The x coordinate
 * @param y The y coordinate
 * @return The index to access the item
 */
static __inline findex_t
tetris_field_item_get_index (findex_t x, findex_t y);

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

#endif // !__TETRIS_P_H
