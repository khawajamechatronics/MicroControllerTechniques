// (c) Tobias Faller 2017
// (c) Tim Maffenbeier 2017

#include <stdint.h>
#include <stdlib.h>
#include <msp430.h>

#include "inc/def.h"
#include "inc/config.h"

#include "inc/tetris.h"
#include "inc/timer.h"
#include "inc/util.h"
#include "inc/uart.h"
#include "inc/highscore.h"
#include "inc/buttons.h"

#include "tetris_p.h"

static tetris_t *tetris_inst;

// --- Game -------------------------------------------------------------------

void
tetris_game_init (tetris_t *tetris, uint8_t *cmd_buffer,
                  uint8_t cmd_buffer_size)
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

  tetris->command_buffer.buffer = cmd_buffer;
  tetris->command_buffer.buffer_size = cmd_buffer_size;
  tetris->command_buffer.start = 0;
  tetris->command_buffer.fill = 0;

  memset(&tetris->game_field, TETRIS_FIELD_EMPTY, sizeof(field_t));

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
  buttons_set_callback(&tetris_on_button);
}

void
tetris_game_process (void)
{
  for(;;)
  {
    field_t *field = tetris_field_get_current(tetris_inst);

    // Clear old tetromino position
    tetris_game_place_tetromino(field, tetris_inst->tetro,
                                tetris_inst->tetro_x,
                                tetris_inst->tetro_y,
                                tetris_inst->tetro_rot,
                                TETRIS_FIELD_EMPTY);

    while (!buffer_is_empty(&tetris_inst->command_buffer))
    {
      switch ((tetris_command_t) buffer_dequeue(&tetris_inst->command_buffer))
      {
      case COMMAND_DOWN:
        timer_reset(TIMER_1);
        tetris_inst->timer_divider = 0;

        if (tetris_game_down(tetris_inst, field) == 0)
        {
          tetris_on_game_over();
          return;
        }
        break;
      case COMMAND_DROP:
        {
          uint8_t result;

          timer_reset(TIMER_1);
          tetris_inst->timer_divider = 0;

          do
          {
            result = tetris_game_down(tetris_inst, field);
          } while(result == 1);

          if (result == 0)
          {
            tetris_on_game_over();
            return;
          }
          break;
        }
      case COMMAND_LEFT:
        tetris_game_move(tetris_inst, field, 0);
        break;
      case COMMAND_RIGHT:
        tetris_game_move(tetris_inst, field, 1);
        break;
      case COMMAND_ROTATE:
        tetris_game_rotate(tetris_inst, field);
        break;
      }
    }

    tetris_game_place_tetromino(field, tetris_inst->tetro,
                                tetris_inst->tetro_x,
                                tetris_inst->tetro_y,
                                tetris_inst->tetro_rot,
                                tetris_inst->tetro);

    tetris_game_send(tetris_inst);

    if (buffer_is_empty(&tetris_inst->command_buffer))
      break;
  }
}

// --- Callbacks --------------------------------------------------------------

static bool_t
tetris_on_timer (void)
{
  // Enable reentrant interrupts for UART
  __enable_interrupt();

  if (++tetris_inst->timer_divider < 2)
    return 0;
  else
    tetris_inst->timer_divider = 0;

  for (uint8_t i = buffer_get_fill(&tetris_inst->command_buffer); i-- > 0;)
  {
    uint8_t *command = buffer_get_at(&tetris_inst->command_buffer, i);
    if (*((tetris_command_t*)command) == COMMAND_DOWN
        || *((tetris_command_t*)command) == COMMAND_DROP) {
      return 0; // Down command is already queued
    }
  }

  if (!buffer_is_full(&tetris_inst->command_buffer))
    buffer_enqueue(&tetris_inst->command_buffer, COMMAND_DOWN);

  // Wake up CPU to update the game field
  return 0x01;
}

static bool_t
tetris_on_key (buffer_t *buffer)
{
  bool_t wake_cpu = 0;
  for (;;)
  {
    uint8_t fill = buffer_get_fill(buffer);
    if (fill == 0)
      break;

    uint8_t *first_char = buffer_get_at(buffer, 0);
    if (*first_char != KEY_ESCAPE)
    {
      // Remove character
      buffer_dequeue(buffer);

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
      uint8_t *second_char = buffer_get_at(buffer, 1);
      if (*second_char != '[') {
        // Remove characters
        buffer_dequeue(buffer);
        buffer_dequeue(buffer);

        return wake_cpu;
      }

      if (fill < 3)
        return wake_cpu;

      // Remove characters 'ESC' '['
      buffer_dequeue(buffer);
      buffer_dequeue(buffer);

      // Arrow key escape sequence
      switch (buffer_dequeue(buffer))
      {
      case KEY_LEFT:
        tetris_on_command(COMMAND_LEFT);
        wake_cpu = 1;
        break;
      case KEY_RIGHT:
        tetris_on_command(COMMAND_RIGHT);
        wake_cpu = 1;
        break;
      case KEY_UP:
        tetris_on_command(COMMAND_ROTATE);
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

static bool_t
tetris_on_button (button_t button)
{
  switch (button)
  {
  case BUTTON_1: // Drop till floor
    tetris_on_command(COMMAND_DROP);
    break;
  case BUTTON_2: // Rotate right
    tetris_on_command(COMMAND_ROTATE);
    break;
  case BUTTON_3: // Go left
    tetris_on_command(COMMAND_LEFT);
    break;
  case BUTTON_4: // Go right
    tetris_on_command(COMMAND_RIGHT);
    break;
  case BUTTON_5: // Drop till floor
    tetris_on_command(COMMAND_DROP);
    break;
  case BUTTON_6: // Down by one
    tetris_on_command(COMMAND_DOWN);
    break;
  }

  // Wake up CPU to update the game field
  return 0x01;
}

static void
tetris_on_command (tetris_command_t command)
{
  if (buffer_is_full(&tetris_inst->command_buffer))
    return;

  buffer_enqueue(&tetris_inst->command_buffer, command);
}

static void
tetris_on_game_over (void)
{
  timer_stop(TIMER_1);
  uart_set_receive_callback(0);

  // Re-use the main memory area for temporary storage
  highscore_init(tetris_inst->score,
                 (highscore_state_t*) &tetris_inst->game_field);

  // Switch to highscore view
  view = VIEW_HIGHSCORE;
}

// --- Game -------------------------------------------------------------------

static void
tetris_game_move (tetris_t *tetris, field_t *field, uint8_t direction)
{
  uint8_t x = tetris->tetro_x;
  if (direction == 0)
  {
    if (x <= 0)
      return;
    x--;
  } else {
    if (x >= TETRIS_WIDTH - 1)
      return;
    x++;
  }

  if (tetris_game_check_collision(field, tetris->tetro, x, tetris->tetro_y,
                                  tetris->tetro_rot))
  {
    return;
  }

  tetris->tetro_x = x;
}

static void
tetris_game_rotate (tetris_t *tetris, field_t *field)
{
  uint8_t rot = (tetris->tetro_rot + 1) & 0x03;
  if (tetris_game_check_collision(field, tetris->tetro, tetris->tetro_x,
                                  tetris->tetro_y, rot))
  {
    return;
  }

  tetris->tetro_rot = rot;
}

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

static __inline uint8_t
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
      if(!tetris_field_item_is_empty(item++))
        return 0; // Top field was used -> game over
    }

    // Choose next tetromino
    tetris_game_new_tetromino();

    uint8_t cleared = tetris_field_clear_full_lines(field);
    tetris_game_update_score(tetris, cleared, tetris->t_spin);

    return 2;
  } else {
    tetris->tetro_y++;
  }

  return 1;
}

static __inline uint8_t
tetris_field_clear_full_lines (field_t *field)
{
  uint8_t lines_cleared = 0;

  // Update from bottom up
  for (findex_t y = TETRIS_HEIGHT; y-- > 0;)
  {
    // Check each item in row
    for (findex_t x = TETRIS_WIDTH; x-- > 0;)
    {
      field_item_t *item = tetris_field_item_get_at(field, x, y);
      if (tetris_field_item_is_empty(item))
        goto next_row;
    }

    lines_cleared++;

    // Row is full -> drop rows from bottom to top
    for (findex_t dy = y + 1; dy > 0;)
    {
      dy--;

      for (findex_t x = TETRIS_WIDTH; x-- > 0;)
      {
        findex_t index = tetris_field_item_get_index(x, dy);

        if (dy == 0) // Clear top row
        {
          tetris_field_item_set(field, index, TETRIS_FIELD_EMPTY);
        }
        else
        {
          field_item_t *upper_item = tetris_field_item_get_at(field, x, dy - 1);
          tetris_field_item_set(field, index, *upper_item);
        }
      }
    }

    y++; // Re-check this row

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

  // Use pre-computed lookup table for:
  // points = (cleared rows)^2 * (successive clears)
  uint16_t points
    = SCORE_MULTIPLIER[tetris->score_factor][(uint8_t) (cleared - 1)];

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

    if (fy < 0) // Ignore if higher than field
      continue;

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

    if (fy < 0) // Ignore if higher than field
      continue;

    findex_t iindex = tetris_field_item_get_index((uint8_t) fx, (uint8_t) fy);
    tetris_field_item_set(field, iindex, value);
  }

  return 1;
}

// --- Field ------------------------------------------------------------------

static __inline field_t*
tetris_field_get_current (tetris_t *tetris)
{
  return &tetris->game_field;
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
  return (tetromino_t) (*item & ~TETRIS_FIELD_EMPTY);
}

static __inline void
tetris_field_item_set (field_t *field, findex_t index,
                       field_item_t value)
{
  field_item_t *item = tetris_field_item_get(field, index);
  *item = value;
}

static __inline bool_t
tetris_field_item_is_empty (field_item_t *item)
{
  return (*item == TETRIS_FIELD_EMPTY);
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
  const int8_t x_tetro_offset = (int8_t) TETROMINO_INIT_POS[tetromino][0];
  const int8_t y_tetro_offset = (int8_t) TETROMINO_INIT_POS[tetromino][1];
  const int8_t *tetroimino_fields = TETROMINO[tetromino];
  const char c = TETROMINO_CHAR[tetromino];
  for (uint8_t i = 4; i-- > 0;)
  {
    int8_t fx = (int8_t) TETRIS_SCORE_X + *(tetroimino_fields++) + 3;
    int8_t fy = (int8_t) y_offset + *(tetroimino_fields++) - 3;
    fx += x_tetro_offset;
    fy += y_tetro_offset;

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
