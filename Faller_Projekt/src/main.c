// (c) Tobias Faller 2017

#define NO_TEMPLATE_UART

#include <msp430.h>
#include <templateEMP.h>

#include "inc/def.h"
#include "inc/config.h"

#include "inc/tetris.h"

// ----------------------------------------------------------------------------
// Standard methods
// ----------------------------------------------------------------------------

__inline void setup(void);
__inline void loop(void);

// ----------------------------------------------------------------------------
// Fields
// ----------------------------------------------------------------------------

// Store in large flash area
#pragma location=0xC000
static tetris_t tetris;

int main(void) {
  // Initialize with uC without a predefined serial connection
  initMSP();

  setup();

  for (;;)
    loop();
}

__inline void setup() {
  tetris_init_game(&tetris);
}

__inline void loop() {

}
