// (c) Tobias Faller 2017

#define NO_TEMPLATE_UART

#include <msp430.h>
#include <templateEMP.h>
#include <stdint.h>
#include <stdlib.h>

#include "inc/def.h"
#include "inc/config.h"

#include "inc/shift_register.h"
#include "inc/uart.h"
#include "inc/tetris.h"
#include "inc/timer.h"
#include "inc/main.h"

// ----------------------------------------------------------------------------
// Standard Methods
// ----------------------------------------------------------------------------

__attribute__((always_inline))
static __inline void
setup (void);

// ----------------------------------------------------------------------------
// Fields
// ----------------------------------------------------------------------------

static uint8_t uart_r_buffer[UART_R_BUFFER_SIZE];
static uint8_t uart_t_buffer[UART_T_BUFFER_SIZE];
static uint8_t command_buffer[TETRIS_CMD_BUFFER_SIZE];

static tetris_t tetris;

int
main (void)
{
  // Initialize with uC without a predefined serial connection
  initMSP();

  setup();

  for (;;) {
    // Go into low power mode 0
    __bis_SR_register(CPUOFF);

    // Compute the next state and transmit
    tetris_game_process();
  }
}

__attribute__((always_inline))
static __inline void
setup (void)
{
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

  uart_set_receive_callback(&main_uart_received);

  // Initialize timer to seed the RNG
  timer_init(TIMER_2);
  timer_set_divider(TIMER_2, TIMER_DIVIDER_1);
  timer_set_interval(TIMER_2, 0xFFFF);
  timer_start_counter(TIMER_2);

  // Print a welcome message each second (Wait for terminal connection)
  timer_init(TIMER_1);
  timer_set_divider(TIMER_1, TIMER_DIVIDER_8);
  timer_set_interval(TIMER_1, 0xF424); // 0.5 second
  timer_set_callback(TIMER_1, &main_send_welcome);
  timer_start(TIMER_1);
}

static bool_t
main_send_welcome (void)
{
  uart_send_terminal_init();
  uart_send_move_to(0, 1);
  uart_send_cls();
  uart_send_string("Welcome to Tetris. Press enter to continue ...\r\n");
  uart_send_string("Please set the resolution to at least 30x80 chars!\r\n");

  // Don't wake the CPU
  return 0;
}

static bool_t
main_uart_received (buffer_t *buffer)
{
  while (!buffer_is_empty(buffer)) {
    if (buffer_dequeue(buffer) == KEY_ENTER) {
      timer_stop(TIMER_1);
      timer_stop(TIMER_2);

      // Seed the RNG
      srand(timer_get_value(TIMER_2));

      // Initialize the game
      tetris_game_init(&tetris, command_buffer, TETRIS_CMD_BUFFER_SIZE);

      // Clear screen
      uart_send_move_to(0, 1);
      uart_send_cls();

      tetris_game_start();

      // Wake up CPU to send the initial game field
      return 1;
    }
  }

  // Don't wake up the CPU
  return 0;
}
