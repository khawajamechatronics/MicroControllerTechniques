// (c) Tobias Faller 2017
// (c) Tim Maffenbeier 2017

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
#include "inc/highscore.h"
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
static tetris_t tetris_buffer;
static buttons_t button_buffer;

view_t view;

int
main (void)
{
  // Initialize with uC without a predefined serial connection
  initMSP();

  setup();

  for (;;) {
    // Go into low power mode 0
    __bis_SR_register(CPUOFF);

    restart:
    switch (view) {
    case VIEW_GAME:
      // Compute the next state and transmit
      tetris_game_process();

      if (view != VIEW_GAME)
        goto restart; // Enter highscore mode
      break;
    case VIEW_HIGHSCORE:
      highscore_process();

      if (view == VIEW_GAME)
        goto restart; // Enter game mode
      break;
    }
  }
}

__attribute__((always_inline))
static __inline void
setup (void)
{
  // Initialize port 1 (set as input without pull-up / -down)
  // The UART module overwrites this setting for pin 1 & 2
  // The button module keeps this settings for pin 3 & 4
  P1IE = 0;
  P1SEL = 0;
  P1SEL2 = 0;
  P1DIR = 0;
  P1OUT = 0;

  // Initialize unused port 3
  P3SEL = 0;
  P3SEL2 = 0;
  P3DIR = 0;
  P3OUT = 0;

  // Initialize the UART connection
  uart_init(uart_r_buffer, UART_R_BUFFER_SIZE,
            uart_t_buffer, UART_T_BUFFER_SIZE);

  // Initialize buttons
  buttons_init(&button_buffer);

  // Print a welcome message each second (Wait for terminal connection)
  timer_init(TIMER_1);
  timer_set_divider(TIMER_1, TIMER_DIVIDER_8);
  timer_set_interval(TIMER_1, 0xF424); // 0.5 second
  timer_set_callback(TIMER_1, &main_send_welcome);
  timer_start(TIMER_1);

  uart_set_receive_callback(&main_uart_received);
  buttons_set_callback(&main_button_pressed);
}

static bool_t
main_send_welcome (void)
{
  uart_send_terminal_init();
  uart_send_move_to(0, 1);
  uart_send_cls();
  uart_send_string("Welcome to Tetris.\r\n");
  uart_send_string("Please set the resolution to at least 30x80 chars!\r\n");
  uart_send_string("\r\n");
  uart_send_string("Press ENTER (5) to continue ...\r\n");
  uart_send_string("Press H (6) to view the highscore table ...\r\n");

  // Don't wake the CPU
  return 0;
}

static void
main_view_game (void)
{
  timer_stop(TIMER_1);

  // Seed the RNG with current timer value
  srand(timer_get_value(TIMER_2));

  // Initialize the game
  tetris_game_init(&tetris_buffer, command_buffer, TETRIS_CMD_BUFFER_SIZE);

  // Clear screen
  uart_send_move_to(0, 1);
  uart_send_cls();

  tetris_game_start();

  // Wake up CPU to send the initial game field
  view = VIEW_GAME;
}

static void
main_view_highscore (void)
{
  timer_stop(TIMER_1);

  // Initialize highscore view
  highscore_init(HIGHSCORE_SHOW,
                 (highscore_state_t*) &tetris_buffer.game_field);

  // Wake up CPU to load the highscore list
  view = VIEW_HIGHSCORE;
}

static bool_t
main_uart_received (buffer_t *buffer)
{
  while (!buffer_is_empty(buffer)) {
    switch(buffer_dequeue(buffer)) {
    case KEY_ENTER:
    case 'T': // Tetris
    case 't':
      main_view_game();
      return 0x01;
    case 'H': // Highscore
    case 'h':
      main_view_highscore();
      return 0x01;
    default:
      break;
    }
  }

  // Don't wake up the CPU
  return 0x00;
}

static bool_t
main_button_pressed (button_t button)
{
  switch (button)
  {
  case BUTTON_5:
    main_view_game();
    return 0x01;
  case BUTTON_6:
    main_view_highscore();
    return 0x01;
  default:
    return 0x00;
  }
}
