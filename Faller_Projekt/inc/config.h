// (c) Tobias Faller 2017

#ifndef __CONFIG_H
#define __CONFIG_H

#include "def.h"

#define TETRIS_WIDTH 10
#define TETRIS_HEIGHT 22
#define TETRIS_TOP_HIDDEN 2
#define TETRIS_SCALE 1
#define TETRIS_X 10
#define TETRIS_Y 2

#define TETRIS_SCORE_X 25
#define TETRIS_SCORE_Y 2

#define TETRIS_NEXT_X 25
#define TETRIS_NEXT_Y 10

#define TETRIS_BORDER_V '|'
#define TETRIS_BORDER_H '-'
#define TETRIS_BORDER_C '+'

#define TETRIS_TETROMINO_I 'X'
#define TETRIS_TETROMINO_T '&'
#define TETRIS_TETROMINO_Z '#'
#define TETRIS_TETROMINO_Z_INV '*'
#define TETRIS_TETROMINO_L '%'
#define TETRIS_TETROMINO_L_INV 'O'
#define TETRIS_TETROMINO_O '$'

#define TETRIS_CMD_BUFFER_SIZE 16

#define UART_R_BUFFER_SIZE 8
#define UART_T_BUFFER_SIZE 56

#define UART_1MHZ
//#define UART_1048KHZ

#define UART_115K
//#define UART_38K
//#define UART_9K

#define HIGHSCORE_LENGTH 4
#define HIGHSCORE_NAME_LENGTH 10

#endif // !__CONFIG_H
