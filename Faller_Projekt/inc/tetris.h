// (c) Tobias Faller 2017

#ifndef __TETRIS_H
#define __TETRIS_H

#include <stdint.h>

#include "def.h"
#include "config.h"


// ----------------------------------------------------------------------------
// Definitions
// ----------------------------------------------------------------------------

#ifndef TETRIS_NO_DOUBLE_BUFFERING
#define TETRIS_BUFFER_COUNT 2
#endif

// ----------------------------------------------------------------------------
// Types
// ----------------------------------------------------------------------------

typedef uint8_t field_item_t;
typedef uint8_t tetromino_t;

typedef struct
{
  field_item_t data[GAME_WIDTH * GAME_HEIGHT];
} field_t;

typedef struct
{
#ifndef TETRIS_NO_DOUBLE_BUFFERING
  field_t buffer[TETRIS_BUFFER_COUNT];
  uint8_t last_buffer;
  uint8_t current_buffer;
#else
  field_t buffer;
#endif

  tetromino_t current_tetromino;
  tetromino_t next_tetromino;

  uint32_t score;
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
tetris_game_init (tetris_t *tetris);

/**
 * Starts the tetris game and the needed timer.
 *
 * @param tetris The main tetris instance
 */
__inline void
tetris_game_start (tetris_t *tetris);

#endif // !__TETRIS_H
