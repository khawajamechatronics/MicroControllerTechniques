// (c) Tobias Faller 2017

#ifndef __CONFIG_H
#define __CONFIG_H

#include "def.h"

#define GAME_WIDTH 10
#define GAME_HEIGHT 22
#define GAME_TOP_HIDDEN 2

#define TETRIS_NO_DOUBLE_BUFFERING

#define UART_R_BUFFER_SIZE 8
#define UART_T_BUFFER_SIZE 56
#define UART_BUFFER_SIZE (UART_R_BUFFER_SIZE + UART_T_BUFFER_SIZE)

#endif // !__CONFIG_H
