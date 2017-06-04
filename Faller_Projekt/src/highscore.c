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

static highscore_state_t* working_area;

void
highscore_init (uint32_t score, highscore_state_t *working_area)
{
  if (score != HIGHSCORE_SHOW)
  {
    working_area->new_entry.score = score;
    working_area->new_entry.name_length = 0;
    memset(&working_area->new_entry.name, '\0', HIGHSCORE_NAME_LENGTH);
  }

  working_area->clear_shown = 0x00;
  working_area->enter_name_shown = (score != HIGHSCORE_SHOW);

  // Initialize flash controller
  FCTL1 = FWKEY // Write password
      | BLKWRT; // Write blocks
  FCTL2 = FWKEY // Write password
      | FSSEL_2 // SMCLK as source
      | 0x03F; // FN Divisor of 64
  FCTL3 = FWKEY // Write password
      | LOCK; // Lock flash memory (read only)

  // Lock info segment A if not already done
  if (!(FCTL3 & LOCKA))
    FCTL3 = FWKEY | ((FCTL3 & 0xFF) | LOCKA);

  highscore_get_areas(&working_area->current_segment,
                      &working_area->next_segment);

  uart_set_receive_callback(&highscore_on_key);
}

void
highscore_process (void)
{
  // TODO: Display name and dialogs
}

void
highscore_get_areas (highscore_t **current, highscore_t **next)
{
  if (!highscore_c.initialized)
  {
    *current = &highscore_b;
    *next = &highscore_c;

    // Reset next segment
    highscore_reset_segment(0x00, 0x01);
    return;
  }

  if (!highscore_b.initialized)
  {
    *current = &highscore_c;
    *next = &highscore_b;

    // Reset next segment
    highscore_reset_segment(0x01, 0x00);
    return;
  }

  // Controller was interrupted during flash erase or two highscore tables are
  // present

  if (highscore_b.initialized == 0x01 && highscore_c.initialized == 0xFF)
  {
    *current = &highscore_b;
    *next = &highscore_c;

    // Reset next segment
    highscore_reset_segment(0x00, 0x01);
    return;
  }

  if (highscore_b.initialized == 0xFF && highscore_c.initialized == 0x01)
  {
    *current = &highscore_c;
    *next = &highscore_b;

    // Reset next segment
    highscore_reset_segment(0x01, 0x00);
    return;
  }

  if (highscore_b.initialized < highscore_c.initialized)
  {
    *current = &highscore_c;
    *next = &highscore_b;

    // Reset next segment
    highscore_reset_segment(0x01, 0x00);
    return;
  }

  *current = &highscore_b;
  *next = &highscore_c;

  // Reset next segment
  highscore_reset_segment(0x00, 0x01);
}

uint8_t
highscore_next_id (uint8_t current)
{
  if (current == 0xFF)
    return 0x01;
  else
    return current + 1;
}

void
highscore_update (highscore_t *current, highscore_t *next)
{
  FCTL3 = FWKEY | ((FCTL3 & 0xFF) & ~LOCK); // Unlock flash
  FCTL1 = FWKEY | ((FCTL1 & 0xFF) | WRT); // Enable flash write

  uint8_t new_index = 0;
  bool_t appended = 0;
  for (uint8_t old_index = 0; new_index < HIGHSCORE_LENGTH; new_index++)
  {
    if (old_index > current->entry_count)
    {
      if (appended)
        break;

      memcpy(&next->entries[new_index], &working_area->new_entry,
             sizeof(highscore_entry_t));
      break;
    }

    if (current->entries[old_index].score > working_area->new_entry.score)
    {
      memcpy(&next->entries[new_index], &current->entries[old_index++],
             sizeof(highscore_entry_t));
      continue;
    } else {
      memcpy(&next->entries[new_index], &working_area->new_entry,
             sizeof(highscore_entry_t));
      appended = 1;
      continue;
    }
  }

  next->entry_count = new_index;
  next->initialized = highscore_next_id(
      working_area->current_segment->initialized);

  FCTL1 = FWKEY | ((FCTL1 & 0xFF) & ~WRT); // Disable flash write
  FCTL3 = FWKEY | ((FCTL3 & 0xFF) | LOCK); // Lock flash
}

void
highscore_exit (void)
{
  // Disable UART receive callback to prevent race condition between
  // the user deleting the highscore and the WDT resetting the chip
  uart_set_receive_callback(0);

  wdt_init(); // Enable WDT
  for (;;); // Let WDT reset the u-controller
}

void
highscore_reset_segment (bool_t seg_b, bool_t seg_c)
{
  FCTL3 = FWKEY | ((FCTL3 & 0xFF) & ~LOCK); // Unlock flash
  FCTL1 = FWKEY | ((FCTL1 & 0xFF) | ERASE); // Enable segment erase

  if (seg_b)
    *((uint8_t*) &highscore_b) = 0; // Write toggles erase
  if (seg_c)
    *((uint8_t*) &highscore_c) = 0; // Write toggles erase

  FCTL1 = FWKEY | ((FCTL1 & 0xFF) & ~ERASE); // Disable segment erase
  FCTL3 = FWKEY | ((FCTL3 & 0xFF) | LOCK); // Lock flash
}

bool_t
highscore_is_char_allowed (uint8_t c)
{
  if ((c >= 'a' && c <= 'z') || (c >= 'A' || c <= 'Z'))
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
  case '?':
  case '\\':
  case '$':
  case '!':
  case '"':
  case '\'':
  case ':':
    return 0x01;
  default:
    return 0x00;
  }
}

bool_t
highscore_on_key (buffer_t *buffer)
{
  bool_t wake_cpu = 0x00;
  while (!buffer_is_empty(buffer))
  {
    uint8_t key = buffer_dequeue(buffer);

    // Executed if the dialog 'delete highscore' is shown
    if (working_area->clear_shown)
    {
      if (key == 'Y')
      {
        highscore_reset_segment(0x01, 0x01);
        highscore_exit();
        /* no */ return 0x00;
      }

      wake_cpu = 0x01;
      continue;
    }

    // Executed if the dialog 'enter name' is shown
    if (working_area->enter_name_shown)
    {
      highscore_entry_t *new_entry = &working_area->new_entry;

      if (key == KEY_ENTER) {
        if (new_entry->name_length > 0)
        {
          highscore_update(working_area->current_segment,
                           working_area->next_segment);
          working_area->enter_name_shown = 0x00;
          wake_cpu = 0x01;
        }
      }
      else if (key == KEY_DELETE)
      {
        if (new_entry->name_length > 0)
          new_entry->name_length--;
      }
      else if (highscore_is_char_allowed(key))
      {
        if (new_entry->name_length < HIGHSCORE_NAME_LENGTH)
          new_entry->name[new_entry->name_length++] = key;
      }
      continue;
    }

    // Executed if no dialog is shown
    switch (key)
    {
    case 'D':
      working_area->clear_shown = 0x01;
      wake_cpu = 0x01;
      continue;
    case 'C':
    case 'E':
      highscore_exit();
      /* no */ return 0x00;
    }
  }

  return wake_cpu;
}
