// (c) Tobias Faller 2017
// (c) Tim Maffenbeier 2017

#ifndef __TETRIS_H
#define __TETRIS_H

#include <stdint.h>

#include "def.h"
#include "config.h"

#include "buffer.h"
#include "buttons.h"

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

typedef enum {
  COMMAND_LEFT = 0x01,
  COMMAND_RIGHT = 0x02,
  COMMAND_ROTATE = 0x03,
  COMMAND_DOWN = 0x04,
  COMMAND_DROP = 0x05
} tetris_command_t;

typedef struct
{
  field_item_t data[TETRIS_WIDTH * TETRIS_HEIGHT];
} field_t;

typedef struct
{
  field_t game_field;
  uint8_t timer_divider;

  tetromino_t tetro;
  tetromino_t tetro_next;

  uint8_t tetro_rot;
  uint8_t tetro_x;
  uint8_t tetro_y;

  uint32_t score;
  uint16_t lines;
  uint16_t level;
  uint8_t part_lines;

  uint8_t score_factor;
  uint8_t t_spin;

  buffer_t command_buffer;
} tetris_t;

// ----------------------------------------------------------------------------
// Methods
// ----------------------------------------------------------------------------

/**
 * Initializes the tetris game an it's game field.
 * The provided command buffer is used to store user input.
 *
 * @param tetris The main tetris instance to initialize
 * @param cmd_buffer The command buffer to use
 * @param cmd_buffer_size The size of the command buffer
 */
void
tetris_game_init (tetris_t *tetris, uint8_t *cmd_buffer,
                  uint8_t cmd_buffer_size);

/**
 * Starts the tetris game and the needed timer.
 */
void
tetris_game_start (void);

/**
 * Updates the tetris game and processes all received data.
 */
void
tetris_game_process (void);

#endif // !__TETRIS_H
