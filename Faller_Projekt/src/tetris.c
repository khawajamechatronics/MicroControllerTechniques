// (c) Tobias Faller 2017

#include <stdint.h>
#include <stdlib.h>

#include "inc/def.h"
#include "inc/config.h"

#include "inc/tetris.h"
#include "inc/timer.h"
#include "inc/util.h"

#include "tetris_p.h"

static tetris_t *tetris_inst;

// --- Game -------------------------------------------------------------------

void
tetris_game_init (tetris_t *tetris)
{
  tetris->current_tetromino = tetris_pick_random_tetromino();
  tetris->next_tetromino = tetris_pick_random_tetromino();

  tetris->score = 0;

  memset(&tetris->buffer, 0, sizeof(field_t));

  tetris_inst = tetris;
}

void
tetris_game_start (void)
{
  timer_init(TIMER_1);
  timer_set_divider(TIMER_1, TIMER_DIVIDER_8);
  timer_set_interval(TIMER_1, 0xF424); // 0.5 second (scaled down to 1.5s)
  timer_set_callback(TIMER_1, &tetris_on_timer);
  timer_start(TIMER_1);
}

static void
tetris_on_timer (void)
{
  static uint8_t divider = 0;
  if (++divider < 3)
    return;

  tetris_field_update(tetris_inst);
  tetris_game_drop(tetris_inst);
  tetris_game_send(tetris_inst);
}

static __inline void
tetris_game_speedup (void)
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

  uint8_t cleared = tetris_field_clear_full_lines(current_field);
  tetris_game_update_score(tetris, cleared);
}

static __inline uint8_t
tetris_field_clear_full_lines (field_t *field)
{
  findex_t y;
  findex_t x;
  findex_t dy;
  uint8_t lines_cleared = 0;

  // Update from bottom up
  for (y = TETRIS_HEIGHT; y-- > 0;) {
    // Check each item in row
    for (x = TETRIS_WIDTH; x-- > 0;) {
      field_item_t *item = tetris_field_item_get_at(field, x, y);

      if (tetris_field_item_is_empty(item))
        goto next_row;
    }

    lines_cleared++;

    // Row is full -> drop rows from bottom to top
    for (dy = y + 1; dy-- > 0;) {
      for (x = TETRIS_WIDTH; x-- > 0;) {
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

  return lines_cleared;
}

static __inline tetromino_t
tetris_pick_random_tetromino (void)
{
  // Use the default random number generator
  return (tetromino_t) (rand() % 7);
}

static __inline void
tetris_game_send (tetris_t *tetris)
{

}

static void
tetris_game_update_score (tetris_t *tetris, uint8_t cleared)
{
  uint16_t last_level = tetris->level;

  if (tetris->level != last_level)
    tetris_game_speedup();
}

// --- Field ------------------------------------------------------------------

static __inline field_t*
tetris_field_get_current (tetris_t *tetris)
{
  return &tetris->buffer;
}

static __inline void
tetris_field_update (tetris_t *tetris) {
  field_t *field = tetris_field_get_current(tetris);

  // Clear updated flags
  findex_t index;
  for (index = TETRIS_WIDTH * TETRIS_HEIGHT; index-- > 0;)
    field->data[index] &= ~TETRIS_FIELD_UPDATED;
}

// --- Item -------------------------------------------------------------------

static __inline findex_t
tetris_field_item_get_index (findex_t x, findex_t y)
{
  return y * TETRIS_WIDTH + x;
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
  return (tetromino_t) (*item & ~(TETRIS_FIELD_UPDATED | TETRIS_FIELD_TEMP));
}

static __inline tetromino_t
tetris_field_item_get_value (field_item_t *item)
{
  return (tetromino_t) (*item & ~TETRIS_FIELD_UPDATED);
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
