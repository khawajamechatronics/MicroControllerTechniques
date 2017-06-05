// (c) Tobias Faller 2017

#include <stdint.h>
#include <msp430.h>

#include "inc/def.h"
#include "inc/config.h"

#include "inc/util.h"
#include "inc/wdt.h"
#include "inc/uart.h"
#include "inc/highscore.h"

#include "highscore_p.h"

#pragma DATA_SECTION(highscore_b, ".infoB")
static highscore_t highscore_b;

#pragma DATA_SECTION(highscore_c, ".infoC")
static highscore_t highscore_c;

static highscore_state_t* state;

void
highscore_init (uint32_t score, highscore_state_t *working_area)
{
  state = working_area;

  state->clear_shown = 0x00;
  state->enter_name_shown = 0x00;

  // Initialize flash controller
  FCTL1 = FWKEY; // Write password
  FCTL2 = FWKEY // Write password
      | FSSEL_2 // SMCLK as source
      | 0x03F; // FN Divisor of 64
  FCTL3 = FWKEY // Write password
      | LOCK; // Lock flash memory (read only)

  // Lock info segment A if not already done
  if ((FCTL3 & LOCKA) != LOCKA)
  {
    FCTL3 = FWKEY | ((FCTL3 & 0xFF) | LOCKA);
  }

  highscore_get_areas(&state->current_segment,
                      &state->next_segment);

  // Is the new entry on the list?
  if (score != HIGHSCORE_SHOW
      && (state->current_segment->initialized == HIGHSCORE_SEGMENT_EMPTY
        || state->current_segment->entry_count < HIGHSCORE_LENGTH
        || state->current_segment->entries[HIGHSCORE_LENGTH - 1].score < score))
  {
    state->enter_name_shown = 0x01;

    state->new_entry.score = score;
    state->new_entry.name_length = 8;
    memset(&state->new_entry.name, '\0', HIGHSCORE_NAME_LENGTH);
    memcpy(&state->new_entry.name, "No Name!", 8);
  }

  uart_set_receive_callback(&highscore_on_key);
}

void
highscore_process (void)
{
  if (state->enter_name_shown)
  {
    highscore_show_input_dialog();
  }
  else if (state->clear_shown)
  {
    highscore_show_clear_dialog();
  }
  else
  {
    highscore_show_scoreboard();
  }
}

static __inline void
highscore_get_areas (highscore_t **current, highscore_t **next)
{
  if (highscore_c.initialized == HIGHSCORE_SEGMENT_EMPTY)
  {
    *current = &highscore_b;
    *next = &highscore_c;
    return;
  }

  if (highscore_b.initialized == HIGHSCORE_SEGMENT_EMPTY)
  {
    *current = &highscore_c;
    *next = &highscore_b;
    return;
  }

  // Controller was interrupted during flash erase or two highscore tables are
  // present

  if (highscore_b.initialized == 0x00 && highscore_c.initialized == 0xFE)
  {
    *current = &highscore_b;
    *next = &highscore_c;
    return;
  }

  if (highscore_b.initialized == 0xFE && highscore_c.initialized == 0x00)
  {
    *current = &highscore_c;
    *next = &highscore_b;
    return;
  }

  if (highscore_b.initialized < highscore_c.initialized)
  {
    *current = &highscore_c;
    *next = &highscore_b;
    return;
  }

  *current = &highscore_b;
  *next = &highscore_c;
}

static __inline uint8_t
highscore_next_id (uint8_t current)
{
  if (current >= 0xFE)
    return 0x00;
  else
    return current + 1;
}

static __inline void
highscore_update (highscore_t *current, highscore_t *next)
{
  // Reset next segment
  FCTL3 = FWKEY | ((FCTL3 & 0xFF) & ~LOCK); // Unlock flash
  FCTL1 = FWKEY | ((FCTL1 & 0xFF) | ERASE); // Enable segment erase

  *((uint8_t*) next) = 0; // Write toggles erase

  while(FCTL3 & BUSY);

  FCTL1 = FWKEY | ((FCTL1 & 0xFF) & ~ERASE); // Disable segment erase
  FCTL1 = FWKEY | ((FCTL1 & 0xFF) | WRT); // Enable flash write

  uint8_t new_index = 0;
  bool_t appended = 0;
  for (uint8_t old_index = 0; new_index < HIGHSCORE_LENGTH; ++new_index)
  {
    if (current->initialized == HIGHSCORE_SEGMENT_EMPTY
        || old_index >= current->entry_count)
    {
      if (!appended) // Append new score to the end of the list
      {
        memcpy(&next->entries[new_index], &state->new_entry,
               sizeof(highscore_entry_t));
        new_index++; // Increment index since loop is exited
      }

      break;
    }

    // Check if new score should be inserted here
    if (state->new_entry.score > current->entries[old_index].score
        && !appended)
    {
      memcpy(&next->entries[new_index], &state->new_entry,
             sizeof(highscore_entry_t));
      appended = 0x01;
      continue;
    }

    // Copy old score to this position
    memcpy(&next->entries[new_index], &current->entries[old_index++],
             sizeof(highscore_entry_t));
  }

  next->entry_count = new_index;
  next->initialized = highscore_next_id(current->initialized);

  FCTL1 = FWKEY | ((FCTL1 & 0xFF) & ~WRT); // Disable flash write
  FCTL3 = FWKEY | ((FCTL3 & 0xFF) | LOCK); // Lock flash*/

  // Re-Lock info segment A if not already done
  if ((FCTL3 & LOCKA) != LOCKA)
  {
    FCTL3 = FWKEY | ((FCTL3 & 0xFF) | LOCKA);
  }

  // Swap segments to display new data
  state->current_segment = next;
  state->next_segment = current;
}

static __inline void
highscore_exit (void)
{
  // Disable UART receive callback to prevent race condition between
  // the user deleting the highscore and the WDT resetting the chip
  uart_set_receive_callback(0);

  wdt_init(); // Enable WDT
  for (;;); // Let WDT reset the u-controller
}

static void
highscore_reset (void)
{
  FCTL3 = FWKEY | ((FCTL3 & 0xFF) & ~LOCK); // Unlock flash
  FCTL1 = FWKEY | ((FCTL1 & 0xFF) | ERASE); // Enable segment erase

  *((uint8_t*) &highscore_b) = 0; // Write toggles erase
  while(FCTL3 & BUSY);

  FCTL1 = FWKEY | ((FCTL1 & 0xFF) | ERASE); // Enable segment erase

  *((uint8_t*) &highscore_c) = 0; // Write toggles erase
  while(FCTL3 & BUSY);

  FCTL1 = FWKEY | ((FCTL1 & 0xFF) & ~ERASE); // Disable segment erase
  FCTL3 = FWKEY | ((FCTL3 & 0xFF) | LOCK); // Lock flash

  // Re-Lock info segment A if not already done
  if ((FCTL3 & LOCKA) != LOCKA)
  {
    FCTL3 = FWKEY | ((FCTL3 & 0xFF) | LOCKA);
  }
}

static __inline bool_t
highscore_is_char_allowed (uint8_t c)
{
  if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
    return 0x01;

  if (c >= '0' && c <= '9')
    return 0x01;

  switch (c)
  {
  case KEY_SPACE:
  case '.':
  case ',':
  case '+':
  case '-':
  case '*':
  case '/':
  case '#':
  case '?':
  case '\\':
  case '&':
  case '%':
  case '$':
  case '!':
  case '"':
  case '\'':
  case ':':
  case ';':
  case '(':
  case ')':
  case '[':
  case ']':
  case '=':
    return 0x01;
  default:
    return 0x00;
  }
}

static __inline void
highscore_show_input_dialog (void)
{
  uart_send_move_to(0, 1);
  uart_send_cls();

  const uint8_t box_size = MAX(HIGHSCORE_NAME_LENGTH + 4, 19);
  uint8_t y_position = HIGHSCORE_INPUT_Y;

  uart_send_move_to(y_position++, HIGHSCORE_INPUT_X);
  highscore_send_boxline(box_size, HIGHSCORE_BORDER_C, HIGHSCORE_BORDER_H);

  uart_send_move_to(y_position++, HIGHSCORE_INPUT_X);
  highscore_send_boxline(box_size, HIGHSCORE_BORDER_V, ' ');

  uart_send_move_to(y_position, HIGHSCORE_INPUT_X);
  uart_send(HIGHSCORE_BORDER_V);
  uart_send_string(" Score: ");
  uart_send_number_u32(state->new_entry.score, 1);
  uart_send_move_to(y_position++, HIGHSCORE_INPUT_X + box_size + 1);
  uart_send(HIGHSCORE_BORDER_V);

  uart_send_move_to(y_position++, HIGHSCORE_INPUT_X);
  highscore_send_boxline(box_size, HIGHSCORE_BORDER_V, ' ');

  uart_send_move_to(y_position, HIGHSCORE_INPUT_X);
  uart_send(HIGHSCORE_BORDER_V);
  uart_send_string(" Enter your name: ");
  uart_send_move_to(y_position++, HIGHSCORE_INPUT_X + box_size + 1);
  uart_send(HIGHSCORE_BORDER_V);

  uart_send_move_to(y_position++, HIGHSCORE_INPUT_X);
  highscore_send_boxline(box_size, HIGHSCORE_BORDER_V, ' ');

  uart_send_move_to(y_position, HIGHSCORE_INPUT_X);
  uart_send(HIGHSCORE_BORDER_V);
  uart_send(' ');
  for (uint8_t i = 0; i < HIGHSCORE_NAME_LENGTH; i++)
  {
    if (i >= state->new_entry.name_length)
      uart_send('_');
    else
      uart_send(state->new_entry.name[i]);
  }
  uart_send_move_to(y_position++, HIGHSCORE_INPUT_X + box_size + 1);
  uart_send(HIGHSCORE_BORDER_V);

  uart_send_move_to(y_position++, HIGHSCORE_INPUT_X);
  highscore_send_boxline(box_size, HIGHSCORE_BORDER_V, ' ');

  uart_send_move_to(y_position, HIGHSCORE_INPUT_X);
  uart_send(HIGHSCORE_BORDER_V);
  uart_send_move_to(y_position++, HIGHSCORE_INPUT_X + box_size - 7);
  uart_send_number_u8(state->new_entry.name_length, 1);
  uart_send('/');
  uart_send_number_u8(HIGHSCORE_NAME_LENGTH, 1);
  uart_send(' ');
  uart_send(HIGHSCORE_BORDER_V);

  uart_send_move_to(y_position++, HIGHSCORE_INPUT_X);
  highscore_send_boxline(box_size, HIGHSCORE_BORDER_V, ' ');

  uart_send_move_to(y_position++, HIGHSCORE_INPUT_X);
  highscore_send_boxline(box_size, HIGHSCORE_BORDER_V, ' ');

  uart_send_move_to(y_position++, HIGHSCORE_INPUT_X);
  highscore_send_boxline(box_size, HIGHSCORE_BORDER_C, HIGHSCORE_BORDER_H);
}

static __inline void
highscore_show_clear_dialog (void)
{
  uart_send_move_to(0, 1);
  uart_send_cls();

  const uint8_t box_size = 23;
  uint8_t y_position = HIGHSCORE_INPUT_Y;

  uart_send_move_to(y_position++, HIGHSCORE_INPUT_X);
  highscore_send_boxline(box_size, HIGHSCORE_BORDER_C, HIGHSCORE_BORDER_H);

  uart_send_move_to(y_position++, HIGHSCORE_INPUT_X);
  highscore_send_boxline(box_size, HIGHSCORE_BORDER_V, ' ');

  uart_send_move_to(y_position, HIGHSCORE_INPUT_X);
  uart_send(HIGHSCORE_BORDER_V);
  uart_send_string(" Do you really want to ");
  uart_send_move_to(y_position++, HIGHSCORE_INPUT_X + box_size + 1);
  uart_send(HIGHSCORE_BORDER_V);

  uart_send_move_to(y_position, HIGHSCORE_INPUT_X);
  uart_send(HIGHSCORE_BORDER_V);
  uart_send_string(" delete all scores? ");
  uart_send_move_to(y_position++, HIGHSCORE_INPUT_X + box_size + 1);
  uart_send(HIGHSCORE_BORDER_V);

  uart_send_move_to(y_position++, HIGHSCORE_INPUT_X);
  highscore_send_boxline(box_size, HIGHSCORE_BORDER_V, ' ');

  uart_send_move_to(y_position, HIGHSCORE_INPUT_X);
  uart_send(HIGHSCORE_BORDER_V);
  uart_send_string(" (Y)es");
  uart_send_move_to(y_position++, HIGHSCORE_INPUT_X + box_size - 4);
  uart_send_string("(N)o ");
  uart_send(HIGHSCORE_BORDER_V);

  uart_send_move_to(y_position++, HIGHSCORE_INPUT_X);
  highscore_send_boxline(box_size, HIGHSCORE_BORDER_V, ' ');

  uart_send_move_to(y_position++, HIGHSCORE_INPUT_X);
  highscore_send_boxline(box_size, HIGHSCORE_BORDER_C, HIGHSCORE_BORDER_H);
}

static __inline void
highscore_show_scoreboard (void)
{
  const uint8_t box_size = HIGHSCORE_NAME_LENGTH + 22;

  uart_send_move_to(0, 1);
  uart_send_cls();

  uint8_t y_position = HIGHSCORE_Y;

  uart_send_move_to(y_position++, HIGHSCORE_X);
  highscore_send_boxline(box_size, HIGHSCORE_BORDER_C, HIGHSCORE_BORDER_H);

  uart_send_move_to(y_position++, HIGHSCORE_X);
  highscore_send_boxline(box_size, HIGHSCORE_BORDER_V, ' ');

  uart_send_move_to(y_position, HIGHSCORE_X);
  uart_send(HIGHSCORE_BORDER_V);
  uart_send_string(" Highscore:");
  uart_send_move_to(y_position++, HIGHSCORE_X + box_size + 1);
  uart_send(HIGHSCORE_BORDER_V);

  uart_send_move_to(y_position++, HIGHSCORE_X);
  highscore_send_boxline(box_size, HIGHSCORE_BORDER_V, ' ');

  highscore_t *table = state->current_segment;
  for (uint8_t i = 0; i < HIGHSCORE_LENGTH; ++i)
  {
    uart_send_move_to(y_position, HIGHSCORE_X);
    uart_send(HIGHSCORE_BORDER_V);
    uart_send(' ');
    uart_send_number_u8(i + 1, 1);
    uart_send(':');
    uart_send(' ');

    if (i >= state->current_segment->entry_count
        || state->current_segment->initialized == HIGHSCORE_SEGMENT_EMPTY)
    {
      uart_send_string("Empty");
      uart_send_move_to(y_position++, HIGHSCORE_X + box_size + 1);
    }
    else
    {
      for (uint8_t j = 0; (j < table->entries[i].name_length)
          && (j < HIGHSCORE_NAME_LENGTH); ++j)
      {
        uart_send(table->entries[i].name[j]);
      }

      uart_send_move_to(y_position++, HIGHSCORE_X + box_size - 10);
      uart_send_number_u32(table->entries[i].score, 1);
      uart_send(' ');
    }

    uart_send(HIGHSCORE_BORDER_V);
  }

  uart_send_move_to(y_position++, HIGHSCORE_X);
  highscore_send_boxline(box_size, HIGHSCORE_BORDER_V, ' ');

  uart_send_move_to(y_position++, HIGHSCORE_X);
  highscore_send_boxline(box_size, HIGHSCORE_BORDER_C, HIGHSCORE_BORDER_H);

  y_position++;
  if (state->current_segment->initialized != HIGHSCORE_SEGMENT_EMPTY)
  {
    uart_send_move_to(y_position++, HIGHSCORE_X);
    uart_send_string("Press L to delete all highscores ...");
  }

  uart_send_move_to(y_position++, HIGHSCORE_X);
  uart_send_string("Press E to exit ...");
}

static __inline void
highscore_send_boxline (uint8_t count, uint8_t edge, uint8_t fill)
{
  uart_send(edge);
  for (; count-- > 0;)
    uart_send(fill);
  uart_send(edge);
}

bool_t
highscore_on_key (buffer_t *buffer)
{
  bool_t wake_cpu = 0x00;
  while (!buffer_is_empty(buffer))
  {
    uint8_t key = buffer_dequeue(buffer);

    // Executed if the dialog 'delete highscore' is shown
    if (state->clear_shown)
    {
      switch (key)
      {
      case 'Y': // Yes
      case 'y':
        highscore_reset();
        state->clear_shown = 0x00;
        return 0x01;
      case 'N': // No
      case 'n':
        state->clear_shown = 0x00;
        wake_cpu = 0x01;
        break;
      }

      continue;
    }

    // Executed if the dialog 'enter name' is shown
    if (state->enter_name_shown)
    {
      highscore_entry_t *new_entry = &state->new_entry;

      if (key == KEY_ENTER) {
        if (new_entry->name_length > 0)
        {
          highscore_update(state->current_segment,
                           state->next_segment);
          state->enter_name_shown = 0x00;
          wake_cpu = 0x01;
        }
      }
      else if (key == KEY_DELETE)
      {
        if (new_entry->name_length > 0)
        {
          new_entry->name_length--;
          wake_cpu = 0x01;
        }
      }
      else if (highscore_is_char_allowed(key))
      {
        if (new_entry->name_length < HIGHSCORE_NAME_LENGTH)
        {
          new_entry->name[new_entry->name_length] = key;
          new_entry->name_length++;
          wake_cpu = 0x01;
        }
      }
      continue;
    }

    // Executed if no dialog is shown
    switch (key)
    {
    case 'L': // Loeschen (far away from C & E)
    case 'l':
      if (state->current_segment->initialized != HIGHSCORE_SEGMENT_EMPTY)
      {
        state->clear_shown = 0x01;
        wake_cpu = 0x01;
      }
      continue;
    case 'C': // Close
    case 'c':
    case 'E': // Exit
    case 'e':
    case 'X': // EXit
    case 'x':
      highscore_exit();
      /* no */ return 0x00;
    }
  }

  return wake_cpu;
}
