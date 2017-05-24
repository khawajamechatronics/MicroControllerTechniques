// (c) Tobias Faller 2017

#define NO_TEMPLATE_UART

#include <msp430.h>
#include <templateEMP.h>

#include "config.h"
#include "tetris.h"

// ----------------------------------------------------------------------------
// Definitions
// ----------------------------------------------------------------------------


// ----------------------------------------------------------------------------
// Standard methods
// ----------------------------------------------------------------------------

__inline void setup(void);
__inline void loop(void);

// ----------------------------------------------------------------------------
// Fields
// ----------------------------------------------------------------------------

tetris_t tetris;

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
