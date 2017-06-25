// (c) Tobias Faller 2017
// (c) Tim Maffenbeier 2017

#ifndef __DEF_H
#define __DEF_H

#include <stdint.h>

typedef uint8_t bool_t;

typedef enum {
  VIEW_GAME = 0x01,
  VIEW_HIGHSCORE = 0x02
} view_t;

extern view_t view;

#define MAX(x, y) (((x) > (y)) ? (x) : (y))

#define KEY_ESCAPE 0x1B

#define KEY_ENTER 0x0D
#define KEY_SPACE 0x20
#define KEY_DELETE 0x08
//#define KEY_DELETE 0x7F

// Prefixed with escape key
#define KEY_UP 'A'
#define KEY_DOWN 'B'
#define KEY_RIGHT 'C'
#define KEY_LEFT 'D'

#endif // !__DEF_H
