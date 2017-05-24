// (c) Tobias Faller 2017

#ifndef __TETRIS_H
#define __TETRIS_H

#include <stdint.h>

#include "def.h"
#include "config.h"

// ----------------------------------------------------------------------------
// Definitions
// ----------------------------------------------------------------------------

#define TETRIS_BUFFER_COUNT 2

// ----------------------------------------------------------------------------
// Types
// ----------------------------------------------------------------------------

typedef uint8_t field_item_t;

typedef struct
{
  field_item_t data[GAME_WIDTH * GAME_HEIGHT];
} field_t;

typedef struct
{
  field_t buffer[TETRIS_BUFFER_COUNT];
  uint8_t last_buffer;
  uint8_t current_buffer;
} tetris_t;

// ----------------------------------------------------------------------------
// Methods
// ----------------------------------------------------------------------------

/**
 * Initializes the tetris game an it's game field.
 *
 * @param tetris The main tetris instance to initialize
 */
__inline void
tetris_init_game (tetris_t *tetris);

/**
 * Starts the tetris game and the needed timer.
 *
 * @param tetris The main tetris instance
 */
__inline void
tetris_start_game (tetris_t *tetris);

/**
 * Returns the current game field to work on.
 *
 * @param tetris The main tetris instance
 * @return A pointer to the current game field
 */
__inline field_t*
tetris_get_current_field (tetris_t *tetris);

/**
 * Switches the current game field to enable updating the display while
 * drawing the next frame.
 *
 * @param tetris The main tetris instance
 */
__inline void
tetris_switch_field(tetris_t *tetris);

/**
 * Updates the game field and drops the tetromino.
 *
 * @param tetris The main tetris instance
 */
__inline void
tetris_step_field(tetris_t *tetris);

/**
 * Returns a pointer to the location on the tetris filed.
 *
 * @param field The current tetris field
 * @param x The x coordinate to get
 * @param y The y coordinate to get
 * @return The value at the specified position
 */
__inline field_item_t*
tetris_get_field_item (field_t *field, uint8_t x, uint8_t y);

/**
 * Returns if the location on the tetris field is empty.
 *
 * @param item A pointer to the location on the field
 * @return true if the spot is occupied
 */
__inline bool_t
tetris_is_field_item_empty(const field_item_t *item);

#endif // !__TETRIS_H
