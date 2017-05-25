// (c) Tobias Faller 2017

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "inc/def.h"
#include "inc/config.h"

#include "inc/tetris.h"
#include "inc/timer.h"

#include "tetris_p.h"

static tetris_t *tetris_inst;

// --- Game -------------------------------------------------------------------

__inline void
tetris_game_init (tetris_t *tetris)
{
#ifndef TETRIS_NO_DOUBLE_BUFFERING
  tetris->current_buffer = 0;
  tetris->last_buffer = 0;
#endif

  tetris->current_tetromino = tetris_pick_random_tetromino();
  tetris->next_tetromino = tetris_pick_random_tetromino();

  tetris->score = 0;

#ifndef TETRIS_NO_DOUBLE_BUFFERING
  memset(tetris->buffer, '\0', sizeof(field_t) * TETRIS_BUFFER_COUNT);
#else
  memset(&tetris->buffer, '\0', sizeof(field_t));
#endif

  tetris_inst = tetris;
}

__inline void
tetris_game_start (void)
{
  timer_init(TIMER_1);
  timer_set_divider(TIMER_1, TIMER_DIVIDER_8);
  timer_set_interval(TIMER_1, 0xFFFF); // 1,9 seconds
  timer_set_callback(TIMER_1, &tetris_on_timer);
  timer_start(TIMER_1);
}

static void
tetris_on_timer (void)
{
  tetris_field_update(tetris_inst);
  tetris_game_drop(tetris_inst);
  tetris_game_send(tetris_inst);
}

static __inline void
tetris_timer_speedup (void)
{
  // Take 7 / 8 of the original interval
  uint16_t interval = timer_get_interval(TIMER_1);
  timer_set_interval(TIMER_1, interval - (interval >> 3));
}

static __inline void
tetris_game_drop (tetris_t *tetris)
{
  field_t *current_field = tetris_field_get_current(tetris);

  // TODO: Check for tetromino collision

  tetris_clear_full_lines(current_field);
}

static __inline void
tetris_clear_full_lines (field_t *field)
{
  findex_t y;
  findex_t x;
  findex_t dy;

  // Update from bottom up
  for (y = GAME_HEIGHT; y-- > 0;) {
    // Check each item in row
    for (x = GAME_WIDTH; x-- > 0;) {
      field_item_t *item = tetris_field_item_get_at(field, x, y);

      if (tetris_field_item_is_empty(item))
        goto next_row;
    }

    // Row is full -> drop rows from bottom to top
    for (dy = y + 1; dy-- > 0;) {
      for (x = GAME_WIDTH; x-- > 0;) {
        findex_t index = tetris_field_item_get_index(x, dy);

        if (dy == 0) { // Clear top row
          tetris_field_item_set(field, index, TETRIS_FIELD_EMPTY);
        } else {
          findex_t upper_index = tetris_field_item_get_index(x, dy - 1);
          field_item_t *upper_item = tetris_field_item_get(field, upper_index);
          tetris_field_item_set(field, index,
                                tetris_field_item_get_tetromino(upper_item));
        }
      }
    }

next_row:
    ;
  }
}

static __inline tetromino_t
tetris_pick_random_tetromino (void)
{
  // Use the default random number generator
  return (tetromino_t) (rand() % 7);
}

// --- Field ------------------------------------------------------------------

#ifndef TETRIS_NO_DOUBLE_BUFFERING

static __inline field_t*
tetris_field_get_current (tetris_t *tetris)
{
  return &tetris->buffer[tetris->current_buffer];
}

static __inline field_t*
tetris_field_get_last (tetris_t *tetris)
{
  return &tetris->buffer[tetris->last_buffer];
}

static __inline void
tetris_field_update (tetris_t *tetris) {
  tetris->last_buffer = tetris->current_buffer;
  tetris->current_buffer++;

  if (tetris->current_buffer >= TETRIS_BUFFER_COUNT)
    tetris->current_buffer = 0;

  field_item_t *old_field = tetris_field_get_last(tetris)->data;
  field_item_t *new_field = tetris_field_get_current(tetris)->data;

  findex_t index;
  for (index = GAME_WIDTH * GAME_HEIGHT; index-- > 0;) {
    // Copy old field into new one and remove updated flags
    new_field[index] = old_field[index] & ~TETRIS_FIELD_UPDATED;
  }
}

#else

static __inline field_t*
tetris_field_get_current (tetris_t *tetris)
{
  return &tetris->buffer;
}

static __inline void
tetris_field_update (tetris_t *tetris) {
  field_t *field = &tetris->buffer;

  // Clear updated flags
  findex_t index;
  for (index = GAME_WIDTH * GAME_HEIGHT; index-- > 0;)
    field->data[index] &= ~TETRIS_FIELD_UPDATED;
}

#endif

// --- Item -------------------------------------------------------------------

static __inline findex_t
tetris_field_item_get_index (findex_t x, findex_t y)
{
  return y * GAME_WIDTH + x;
}

static __inline field_item_t*
tetris_field_item_get_at (field_t *field, findex_t x, findex_t y)
{
  findex_t index = tetris_field_item_get_index(x, y);
  return tetris_field_item_get(field, index);
}

static __inline field_item_t*
tetris_field_item_get (field_t *field, findex_t index)
{
  return &field->data[index];
}

static __inline tetromino_t
tetris_field_item_get_tetromino (field_item_t *item)
{
  return (*item & ~(TETRIS_FIELD_UPDATED | TETRIS_FIELD_TEMP));
}

static __inline tetromino_t
tetris_field_item_get_value (field_item_t *item)
{
  return (*item & ~TETRIS_FIELD_UPDATED);
}

static __inline void
tetris_field_item_set (field_t *field, findex_t index,
                       field_item_t value)
{
  field_item_t *item = tetris_field_item_get(field, index);
  bool_t changed = (tetris_field_item_get_value(item) != value);

  if (!changed)
    return;

  // Update value
  *item = value | TETRIS_FIELD_UPDATED;
}

static __inline bool_t
tetris_field_item_is_empty (field_item_t *item)
{
  return (tetris_field_item_get_tetromino(item) == TETRIS_FIELD_EMPTY);
}

static __inline void
tetris_field_item_set_updated (field_item_t *item)
{
  *item |= TETRIS_FIELD_UPDATED;
}

static __inline void
tetris_field_item_set_temp (field_item_t *item)
{
  *item |= TETRIS_FIELD_EMPTY;
}

static __inline void
tetris_field_item_clear_updated (field_item_t *item)
{
  *item &= ~TETRIS_FIELD_UPDATED;
}

static __inline void
tetris_field_item_clear_temp (field_item_t *item)
{
  *item &= ~TETRIS_FIELD_EMPTY;
}
