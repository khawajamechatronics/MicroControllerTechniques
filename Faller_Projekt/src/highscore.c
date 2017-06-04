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

static highscore_entry_t* working_area;

void
highscore_init (uint32_t score, highscore_entry_t *working_area)
{
  if (score != HIGHSCORE_SHOW)
  {
    working_area->score = score;
    memset(&working_area->name, '\0', HIGHSCORE_NAME_LENGTH);
  }

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

  uart_set_receive_callback(&highscore_on_key);
}

void
highscore_process (void)
{

}

void
highscore_exit (void)
{
  wdt_init(); // Enable WDT
  for (;;); // Let WDT reset the u-controller
}

void
highscore_reset (bool_t seg_b, bool_t seg_c)
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
highscore_on_key (buffer_t *buffer)
{
  return 0;
}
