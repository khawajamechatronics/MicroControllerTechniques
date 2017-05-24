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

#define TETRIS_FIELD_UPDATED 0x80
#define TETRIS_FIELD_TEMP 0x40

// ----------------------------------------------------------------------------
// Methods
// ----------------------------------------------------------------------------

/**
 * Returns a random tetromino.
 *
 * @return The next tetromino
 */
static tetromino_t
tetris_pick_random_tetromino (void);

/**
 * Sets an location as updated which is then send to the console.
 *
 * @param item The location which was changed
 */
static __inline void
tetris_field_set_updated (field_item_t *item);

/**
 * Returns the current game field to work on.
 *
 * @param tetris The main tetris instance
 * @return A pointer to the current game field
 */
static __inline field_t*
tetris_get_current_field (tetris_t *tetris);

/**
 * Switches the current game field to enable updating the display while
 * drawing the next frame.
 *
 * @param tetris The main tetris instance
 */
static __inline void
tetris_switch_field (tetris_t *tetris);

/**
 * Updates the game field and drops the tetromino.
 *
 * @param tetris The main tetris instance
 */
static __inline void
tetris_step_field (tetris_t *tetris);

/**
 * Returns a pointer to the location on the tetris field.
 *
 * @param field The current tetris field
 * @param x The x coordinate to get
 * @param y The y coordinate to get
 * @return The item at the specified position
 */
static __inline field_item_t*
tetris_field_item_get (field_t *field, uint8_t x, uint8_t y);

/**
 * Returns if the location on the tetris field is empty.
 *
 * @param item A pointer to the location on the field
 * @return true if the spot is occupied
 */
static __inline bool_t
tetris_field_item_is_empty (const field_item_t *item);

#endif // !__TETRIS_P_H
