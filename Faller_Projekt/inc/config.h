// (c) Tobias Faller 2017

#ifndef __CONFIG_H
#define __CONFIG_H

#include "def.h"

#define TETRIS_WIDTH 10
#define TETRIS_HEIGHT 22
#define TETRIS_TOP_HIDDEN 2
#define TETRIS_SCALE 2

#define TETRIS_BORDER_V '|'
#define TETRIS_BORDER_H '-'
#define TETRIS_BORDER_C '+'

#define TETRIS_TETROMINO_I '/'
#define TETRIS_TETROMINO_T '\\'
#define TETRIS_TETROMINO_Z '#'
#define TETRIS_TETROMINO_Z_INV '*'
#define TETRIS_TETROMINO_L '%'
#define TETRIS_TETROMINO_L_INV '!'
#define TETRIS_TETROMINO_O '$'

#define UART_R_BUFFER_SIZE 8
#define UART_T_BUFFER_SIZE 56
#define UART_BUFFER_SIZE (UART_R_BUFFER_SIZE + UART_T_BUFFER_SIZE)

#endif // !__CONFIG_H
