// (c) Tobias Faller 2017

#define NO_TEMPLATE_UART

#include <msp430.h>
#include <templateEMP.h>
#include <stdint.h>

#include "inc/def.h"
#include "inc/config.h"

#include "inc/shift_register.h"
#include "inc/uart.h"
#include "inc/tetris.h"

// ----------------------------------------------------------------------------
// Standard methods
// ----------------------------------------------------------------------------

__inline void setup(void);

// ----------------------------------------------------------------------------
// Fields
// ----------------------------------------------------------------------------

static uint8_t uart_r_buffer[UART_R_BUFFER_SIZE];
static uint8_t uart_t_buffer[UART_T_BUFFER_SIZE];

static tetris_t tetris;

int main(void) {
  // Initialize with uC without a predefined serial connection
  initMSP();

  setup();

  // Go into low power mode 0
  __bis_SR_register(CPUOFF);
}

__inline void setup() {
  // Initialize unused port 1
  // (set as input without pull-up / -down)
  P1IE = 0;
  P1SEL = 0;
  P1SEL2 = 0;
  P1DIR = 0;
  P1OUT = 0;

  // Initialize shift register
  shift_register_init();

  // Initialize unused port 3
  P3SEL = 0;
  P3SEL2 = 0;
  P3DIR = 0;
  P3OUT = 0;

  // Initialize the UART connection
  uart_init(uart_r_buffer, UART_R_BUFFER_SIZE,
            uart_t_buffer, UART_T_BUFFER_SIZE);

  // Initialize and start the game
  tetris_game_init(&tetris);

  // Print a welcome message each second (Wait for terminal connection)
  timer_init(TIMER_1);
  timer_set_divider(TIMER_1, TIMER_DIVIDER_8);
  timer_set_interval(TIMER_1, 0xF424); // 0.5 second
  timer_set_callback(TIMER_1, &send_welcome_message);
  timer_start(TIMER_1);
}

void send_welcome_message (void)
{
  uart_send_terminal_init();
  uart_send_cls();
  uart_send_move_to(0, 0);
  uart_send_string("Welcome to Tetris. Press a key to continue ...");
}

void start_game (void)
{
  timer_stop(TIMER_1);
  tetris_game_start();
}
