// (c) Tobias Faller 2017

#ifndef __TETRIS_H
#define __TETRIS_H

#include <stdint.h>

#include "def.h"
#include "config.h"

// ----------------------------------------------------------------------------
// Types
// ----------------------------------------------------------------------------

typedef uint8_t field_item_t;
typedef enum {
  TETROMINO_I = 0x00,
  TETROMINO_T = 0x01,
  TETROMINO_Z = 0x02,
  TETROMINO_Z_INV = 0x03,
  TETROMINO_L = 0x04,
  TETROMINO_L_INV = 0x05,
  TETROMINO_O = 0x06
} tetromino_t;

typedef struct
{
  field_item_t data[TETRIS_WIDTH * TETRIS_HEIGHT];
} field_t;

typedef struct
{
  field_t buffer;

  tetromino_t current_tetromino;
  tetromino_t next_tetromino;

  uint8_t tetromino_rotation;
  uint8_t tetromino_x;
  uint8_t tetromino_y;

  uint32_t score;
  uint16_t lines;
  uint16_t level;

  uint8_t score_factor;
} tetris_t;

// ----------------------------------------------------------------------------
// Methods
// ----------------------------------------------------------------------------

/**
 * Initializes the tetris game an it's game field.
 *
 * @param tetris The main tetris instance to initialize
 */
void
tetris_game_init (tetris_t *tetris);

/**
 * Starts the tetris game and the needed timer.
 */
void
tetris_game_start (void);

#endif // !__TETRIS_H
