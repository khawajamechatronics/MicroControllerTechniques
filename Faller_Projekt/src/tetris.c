// (c) Tobias Faller 2017

#include <stdint.h>
#include <stdlib.h>
#include <msp430.h>

#include "inc/def.h"
#include "inc/config.h"

#include "inc/tetris.h"
#include "inc/timer.h"
#include "inc/util.h"
#include "inc/uart.h"

#include "tetris_p.h"

static tetris_t *tetris_inst;

// --- Game -------------------------------------------------------------------

void
tetris_game_init (tetris_t *tetris)
{
  tetris_inst = tetris;

  tetris->timer_divider = 0;

  tetris->tetro = TETROMINO_I;
  tetris->tetro_next = tetris_pick_random_tetromino();
  tetris->tetro_rot = 0;
  tetris->tetro_x = 0;
  tetris->tetro_y = 0;

  tetris->score = 0;
  tetris->lines = 0;
  tetris->level = 0;
  tetris->part_lines = 0;

  tetris->score_factor = 0;
  tetris->t_spin = 0;

  tetris->command_down = 0;
  tetris->command_rotate = 0;
  tetris->command_move = 0;

  memset(&tetris->buffer, 0, sizeof(field_t));

  tetris_game_new_tetromino();
}

void
tetris_game_start (void)
{
  timer_init(TIMER_1);
  timer_set_divider(TIMER_1, TIMER_DIVIDER_8);
  timer_set_interval(TIMER_1, 0xF424); // 0.5 second (scaled down to 1.5s)
  timer_set_callback(TIMER_1, &tetris_on_timer);
  timer_start(TIMER_1);

  uart_set_receive_callback(&tetris_on_key);
}

void
tetris_game_process (void)
{
  field_t *field = tetris_field_get_current(tetris_inst);

  // Clear old tetromino position
  tetris_game_place_tetromino(field, tetris_inst->tetro,
                              tetris_inst->tetro_x,
                              tetris_inst->tetro_y,
                              tetris_inst->tetro_rot,
                              0);

  while (tetris_inst->command_down > 0)
  {
    timer_reset(TIMER_1);
    tetris_inst->timer_divider = 0;

    if(!tetris_game_down(tetris_inst, field))
    { // Game over
      for (;;);
    }

    if (tetris_inst->command_down > 0)
      tetris_inst->command_down--;
  }

  tetris_game_place_tetromino(field, tetris_inst->tetro,
                              tetris_inst->tetro_x,
                              tetris_inst->tetro_y,
                              tetris_inst->tetro_rot,
                              tetris_inst->tetro);

  tetris_game_send(tetris_inst);
}

// --- Callbacks --------------------------------------------------------------

static bool_t
tetris_on_timer (void)
{
  if (++tetris_inst->timer_divider < 2)
    return 0;
  else
    tetris_inst->timer_divider = 0;

  if (tetris_inst->command_down == 0)
    tetris_on_command(COMMAND_DOWN);

  // Wake up CPU to update the game field
  return 1;
}

static bool_t
tetris_on_key (uart_buffer_t *buffer)
{
  bool_t wake_cpu = 0;
  for (;;)
  {
    uint8_t fill = uart_buffer_get_fill(buffer);
    if (fill == 0)
      break;

    uint8_t *first_char = uart_buffer_get_at(buffer, 0);
    if (*first_char != KEY_ESCAPE)
    {
      // Remove character
      uart_buffer_dequeue(buffer);

      // No escape sequence
      switch (*first_char)
      {
      case KEY_SPACE:
        tetris_on_command(COMMAND_DROP);
        wake_cpu = 1;
        break;
      default: // Ignore
        break;
      }

      continue;
    }
    else if (fill >= 2)
    {
      uint8_t *second_char = uart_buffer_get_at(buffer, 1);
      if (*second_char != '[') {
        // Remove characters
        uart_buffer_dequeue(buffer);
        uart_buffer_dequeue(buffer);

        return wake_cpu;
      }

      if (fill < 3)
        return wake_cpu;

      // Remove characters 'ESC' '['
      uart_buffer_dequeue(buffer);
      uart_buffer_dequeue(buffer);

      // Arrow key escape sequence
      switch (uart_buffer_dequeue(buffer))
      {
      case KEY_LEFT:
        tetris_on_command(COMMAND_LEFT);
        wake_cpu = 1;
        break;
      case KEY_RIGHT:
        tetris_on_command(COMMAND_RIGHT);
        wake_cpu = 1;
        break;
      case KEY_DOWN:
        tetris_on_command(COMMAND_DOWN);
        wake_cpu = 1;
        break;
      default: // Ignore
        break;
      }

      continue; // Process next characters
    } else {
      return wake_cpu;
    }
  }

  // Wake up CPU to update the game field
  return wake_cpu;
}

static void
tetris_on_command (tetris_command_t command)
{
  switch (command)
  {
  case COMMAND_DROP:
    // The drop command is emulated with TETRIS_HEIGHT down commands
    tetris_inst->command_down = TETRIS_HEIGHT;
    break;
  case COMMAND_LEFT:
    tetris_inst->command_move--;
    if (tetris_inst->command_move < (int8_t) -TETRIS_WIDTH)
      tetris_inst->command_move = -(int8_t) TETRIS_WIDTH;
    break;
  case COMMAND_RIGHT:
    tetris_inst->command_move++;
    if (tetris_inst->command_move > (int8_t) TETRIS_WIDTH)
      tetris_inst->command_move = (int8_t) TETRIS_WIDTH;
    break;
  case COMMAND_ROTATE:
    tetris_inst->command_rotate++;
    if (tetris_inst->command_rotate >= 8)
      tetris_inst->command_rotate -= 4;
    break;
  case COMMAND_DOWN:
    tetris_inst->command_down++;
    if (tetris_inst->command_down > TETRIS_HEIGHT)
      tetris_inst->command_down = TETRIS_HEIGHT;
    break;
  default:
    break; //Ignore
  }
}

// --- Game -------------------------------------------------------------------

static __inline void
tetris_game_new_tetromino (void)
{
  tetris_inst->tetro = tetris_inst->tetro_next;
  tetris_inst->tetro_next = tetris_pick_random_tetromino();

  tetris_inst->tetro_rot = 0;
  tetris_inst->tetro_x = TETROMINO_INIT_POS[tetris_inst->tetro][0];
  tetris_inst->tetro_y = TETROMINO_INIT_POS[tetris_inst->tetro][1];
}

static __inline void
tetris_game_speedup (void)
{
  // Take 7 / 8 of the original interval
  uint16_t interval = timer_get_interval(TIMER_1);
  timer_set_interval(TIMER_1, interval - (interval >> 3));
}

static __inline bool_t
tetris_game_down (tetris_t *tetris, field_t *field)
{
  uint8_t y = tetris->tetro_y + 1;
  if (tetris_game_check_collision(field, tetris->tetro,
                              tetris->tetro_x, y,
                              tetris->tetro_rot))
  {
    if (!tetris_game_place_tetromino(field, tetris->tetro,
                                tetris->tetro_x, tetris->tetro_y,
                                tetris->tetro_rot,
                                tetris->tetro))
    {
      return 0; // Tetromino is out of bounds -> game over
    }

    field_item_t *item = tetris_field_item_get_at(field, 0, 0);
    for (uint8_t i = TETRIS_WIDTH * TETRIS_TOP_HIDDEN; i-- >0; )
    {
      if(!tetris_field_item_is_empty(item))
        return 0; // Top field was used -> game over
    }

    // Choose next tetromino
    tetris_game_new_tetromino();

    // Cancel all 'down' commands
    tetris->command_down = 0;

    //uint8_t cleared = tetris_field_clear_full_lines(field);
    //tetris_game_update_score(tetris, cleared, tetris->t_spin);
  } else {
    tetris->tetro_y++;
  }

  return 1;
}

static __inline uint8_t
tetris_field_clear_full_lines (field_t *field)
{
  findex_t y;
  findex_t x;
  findex_t dy;
  uint8_t lines_cleared = 0;

  // Update from bottom up
  for (y = TETRIS_HEIGHT; y-- > 0;)
  {
    // Check each item in row
    for (x = TETRIS_WIDTH; x-- > 0;)
    {
      field_item_t *item = tetris_field_item_get_at(field, x, y);

      if (tetris_field_item_is_empty(item))
        goto next_row;
    }

    lines_cleared++;

    // Row is full -> drop rows from bottom to top
    for (dy = y + 1; dy-- > 0;)
    {
      for (x = TETRIS_WIDTH; x-- > 0;)
      {
        findex_t index = tetris_field_item_get_index(x, dy);

        if (dy == 0) // Clear top row
        {
          tetris_field_item_set(field, index, TETRIS_FIELD_EMPTY);
        }
        else
        {
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
tetris_game_update_score (tetris_t *tetris, uint8_t cleared, bool_t t_spin)
{
  // Reset score factor if no line cleared
  if (cleared == 0)
  {
    tetris->score_factor = 0;
    return;
  }

  uint16_t last_level = tetris->level;
  tetris->lines += cleared;
  tetris->part_lines += cleared;

  while (tetris->part_lines >= 10)
  {
    tetris->part_lines -= 10;
    tetris->level++;
  }

  uint16_t points
    = SCORE_MULTIPLIER[tetris->score_factor][(uint8_t) (cleared - 1)];
  // uint16_t points = (tetris->score_factor + 1) * cleared;

  // Double points for t-spin
  if (t_spin)
    points = points << 2;

  // Get result using pre-computed table
  tetris->score += points;

  // Increase score factor
  if (tetris->score_factor < 19)
    tetris->score_factor++;

  if (tetris->level != last_level)
    tetris_game_speedup();
}

static bool_t
tetris_game_check_collision (field_t *field, tetromino_t tetromino,
                             uint8_t x, uint8_t y, uint8_t rotation)
{
  const int8_t *tetroimino_fields = TETROMINO[tetromino] + (rotation << 3);
  for (uint8_t i = 4; i-- > 0;)
  {
    int8_t fx = (int8_t) x + *(tetroimino_fields++);
    int8_t fy = (int8_t) y + *(tetroimino_fields++);

    if(!tetris_check_bounds(fx, fy))
      return 1;

    field_item_t *item = tetris_field_item_get_at(field, (uint8_t) fx,
                                                  (uint8_t) fy);
    if (!tetris_field_item_is_empty(item))
      return 1;
  }

  return 0;
}

static bool_t
tetris_game_place_tetromino (field_t *field, tetromino_t tetromino,
                             uint8_t x, uint8_t y, uint8_t rotation,
                             uint8_t value)
{
  const int8_t *tetroimino_fields = TETROMINO[tetromino] + (rotation << 3);
  for (uint8_t i = 4; i-- > 0;)
  {
    int8_t fx = (int8_t) x + *(tetroimino_fields++);
    int8_t fy = (int8_t) y + *(tetroimino_fields++);

    if(!tetris_check_bounds(fx, fy))
      return 0;

    findex_t iindex = tetris_field_item_get_index((uint8_t) fx, (uint8_t) fy);
    tetris_field_item_set(field, iindex, value);
  }

  return 1;
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
tetris_field_item_get_index (uint8_t x, uint8_t y)
{
  //return (((findex_t) y) << 3) + (((findex_t) y) << 1) + ((findex_t) x);
  return ((findex_t) y) * ((findex_t) TETRIS_WIDTH) + ((findex_t) x);
}

static __inline field_item_t*
tetris_field_item_get_at (field_t *field, uint8_t x, uint8_t y)
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

// --- UART IO ----------------------------------------------------------------

static __inline void
tetris_game_send_field (tetris_t *tetris)
{
  uint8_t y_offset = TETRIS_Y;
  field_t *field = tetris_field_get_current(tetris);

  // Draw top border
  uart_send_move_to(y_offset++, TETRIS_X);
  uart_send(TETRIS_BORDER_C);
  for (uint8_t i = TETRIS_SCALE * TETRIS_WIDTH; i-- > 0;)
    uart_send((i & 0x01) ? TETRIS_BORDER_H : ' ');
  uart_send(TETRIS_BORDER_C);

  // Draw main game field
  for (uint8_t row = TETRIS_TOP_HIDDEN; row < TETRIS_HEIGHT; ++row)
  {
    for (uint8_t i = TETRIS_SCALE; i-- > 0;)
    {
      uart_send_move_to(y_offset++, TETRIS_X);
      uart_send(TETRIS_BORDER_V);

      for (uint8_t col = 0; col < TETRIS_WIDTH; ++col)
      {
        char c;
        field_item_t *item = tetris_field_item_get_at(field, col, row);
        if (tetris_field_item_is_empty(item))
          c = ' ';
        else
        {
          c = TETROMINO_CHAR[tetris_field_item_get_tetromino(item)];
        }

        for (uint8_t j = TETRIS_SCALE; j-- > 0;)
          uart_send(c);
      }

      uart_send(TETRIS_BORDER_V);
    }
  }

  // Draw bottom border
  uart_send_move_to(y_offset, TETRIS_X);
  tetris_game_send_boxline(TETRIS_SCALE * TETRIS_WIDTH,
                           TETRIS_BORDER_C, TETRIS_BORDER_H);
}

static __inline void
tetris_game_send_score (tetris_t *tetris)
{
  uint8_t y_offset = TETRIS_SCORE_Y;

  // Draw top score border
  uart_send_move_to(y_offset++, TETRIS_SCORE_X);
  tetris_game_send_boxline(13, TETRIS_BORDER_C, TETRIS_BORDER_H);

  uart_send_move_to(y_offset++, TETRIS_SCORE_X);
  uart_send(TETRIS_BORDER_V);
  uart_send_string(" Score:      ");
  uart_send(TETRIS_BORDER_V);

  uart_send_move_to(y_offset++, TETRIS_SCORE_X);
  uart_send(TETRIS_BORDER_V);
  uart_send_string("  ");
  uart_send_number_u32(tetris->score, 1);
  uart_send(' ');
  uart_send(TETRIS_BORDER_V);

  uart_send_move_to(y_offset++, TETRIS_SCORE_X);
  tetris_game_send_boxline(13, TETRIS_BORDER_V, ' ');

  uart_send_move_to(y_offset++, TETRIS_SCORE_X);
  uart_send(TETRIS_BORDER_V);
  uart_send_string(" Level:      ");
  uart_send(TETRIS_BORDER_V);

  uart_send_move_to(y_offset++, TETRIS_SCORE_X);
  uart_send(TETRIS_BORDER_V);
  uart_send_string("       ");
  uart_send_number_u16(tetris->level, 1);
  uart_send(' ');
  uart_send(TETRIS_BORDER_V);

  // Draw bottom score border
  uart_send_move_to(y_offset++, TETRIS_SCORE_X);
  tetris_game_send_boxline(13, TETRIS_BORDER_C, TETRIS_BORDER_H);
}

static __inline void
tetris_game_send_next_tetromino (tetris_t *tetris)
{
  uint8_t y_offset = TETRIS_NEXT_Y;
  uint8_t tetromino = tetris->tetro_next;

  // Draw top border
  uart_send_move_to(y_offset++, TETRIS_NEXT_X);
  tetris_game_send_boxline(13, TETRIS_BORDER_C, TETRIS_BORDER_H);

  uart_send_move_to(y_offset++, TETRIS_NEXT_X);
  uart_send(TETRIS_BORDER_V);
  uart_send_string(" Next:       ");
  uart_send(TETRIS_BORDER_V);

  // Draw the 4 empty box lines
  for (uint8_t j = 4; j-- > 0;)
  {
    uart_send_move_to(y_offset++, TETRIS_SCORE_X);
    tetris_game_send_boxline(13, TETRIS_BORDER_V, ' ');
  }

  // Draw the next tetromino
  const int8_t x_offset = (int8_t) TETROMINO_INIT_POS[tetromino][0] + 2;
  const int8_t *tetroimino_fields = TETROMINO[tetromino];
  const char c = TETROMINO_CHAR[tetromino];
  for (uint8_t i = 4; i-- > 0;)
  {
    int8_t fx = (int8_t) TETRIS_SCORE_X + x_offset + *(tetroimino_fields++);
    int8_t fy = (int8_t) y_offset - 2 + *(tetroimino_fields++);

    uart_send_move_to((uint8_t) fy, (uint8_t) fx);
    uart_send(c);
  }

  // Draw bottom score border
  uart_send_move_to(y_offset++, TETRIS_NEXT_X);
  tetris_game_send_boxline(13, TETRIS_BORDER_C, TETRIS_BORDER_H);
}

static __inline void
tetris_game_send_boxline (uint8_t count, uint8_t edge, uint8_t fill)
{
  uart_send(edge);
  for (; count-- > 0;)
    uart_send(fill);
  uart_send(edge);
}

static __inline void
tetris_game_send (tetris_t *tetris)
{
  tetris_game_send_field(tetris);
  tetris_game_send_score(tetris);
  tetris_game_send_next_tetromino(tetris);
}
