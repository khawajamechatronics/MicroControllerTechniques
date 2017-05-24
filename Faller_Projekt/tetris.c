// (c) Tobias Faller 2017

#include <stdint.h>
#include <string.h>

#include "def.h"
#include "config.h"

#include "tetris.h"
#include "timer.h"

__inline void
tetris_init_game (tetris_t *tetris)
{
  tetris->current_buffer = 0;
  tetris->last_buffer = 0;

  memset(tetris->buffer, '\0', sizeof(field_t) * TETRIS_BUFFER_COUNT);
}

__inline void
tetris_start_game (tetris_t *tetris)
{

}

__inline field_t*
tetris_get_current_field (tetris_t *tetris)
{
  return &tetris->buffer[tetris->current_buffer];
}

__inline void
tetris_switch_field(tetris_t *tetris) {
  tetris->current_buffer++;

  if (tetris->current_buffer >= TETRIS_BUFFER_COUNT)
    tetris->current_buffer = 0;
}

__inline field_item_t*
tetris_get_field_item (field_t *field, uint8_t x, uint8_t y)
{
  return &field->data[y * GAME_WIDTH + x];
}

__inline bool_t
tetris_is_field_item_empty(const field_item_t *item)
{
  return (*item == 0);
}

__inline void
tetris_step_game(tetris_t *tetris)
{


}
