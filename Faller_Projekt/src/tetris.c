// (c) Tobias Faller 2017

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "inc/def.h"
#include "inc/config.h"

#include "inc/tetris.h"
#include "inc/timer.h"

#include "tetris_p.h"

__inline void
tetris_init_game (tetris_t *tetris)
{
  tetris->current_buffer = 0;
  tetris->last_buffer = 0;

  tetris->current_tetromino = tetris_pick_random_tetromino();
  tetris->next_tetromino = tetris_pick_random_tetromino();

  tetris->score = 0;

  memset(tetris->buffer, '\0', sizeof(field_t) * TETRIS_BUFFER_COUNT);
}

__inline void
tetris_start_game (tetris_t *tetris)
{

}

static __inline void
tetris_step_game(tetris_t *tetris)
{

}

static __inline field_t*
tetris_get_current_field (tetris_t *tetris)
{
  return &tetris->buffer[tetris->current_buffer];
}

static __inline void
tetris_switch_field (tetris_t *tetris) {
  tetris->current_buffer++;

  if (tetris->current_buffer >= TETRIS_BUFFER_COUNT)
    tetris->current_buffer = 0;
}

static __inline field_item_t*
tetris_field_item_get (field_t *field, uint8_t x, uint8_t y)
{
  return &field->data[y * GAME_WIDTH + x];
}

static __inline bool_t
tetris_field_item_is_empty (const field_item_t *item)
{
  return ((*item & ~(TETRIS_FIELD_UPDATED | TETRIS_FIELD_TEMP)) == 0);
}

static __inline void
tetris_field_set_updated(field_item_t *item) {
  *item |= TETRIS_FIELD_UPDATED;
}

static tetromino_t
tetris_pick_random_tetromino (void)
{
  // Use the default random number generator
  return (tetromino_t) (rand() % 7);
}
