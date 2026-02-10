#ifndef COLORS_H
#define COLORS_H

#ifdef COLOR_BACKGROUND
#undef COLOR_BACKGROUND
#endif

// ==================== COLORS - BGI numeric color values ====================
// BGI standard colors: BLACK=0, BLUE=1, GREEN=2, CYAN=3, RED=4, MAGENTA=5, BROWN=6, LIGHTGRAY=7
// DARKGRAY=8, LIGHTBLUE=9, LIGHTGREEN=10, LIGHTCYAN=11, LIGHTRED=12, LIGHTMAGENTA=13, YELLOW=14, WHITE=15

const int COLOR_BACKGROUND = 0;   // BLACK
const int COLOR_TEXT = 15;        // WHITE
const int COLOR_BUTTON = 1;       // BLUE
const int COLOR_BUTTON_HOVER = 9; // LIGHTBLUE
const int COLOR_BUTTON_CLICKED = 11; // LIGHTCYAN
const int COLOR_VERTEX = 2;       // GREEN
const int COLOR_EDGE = 15;        // WHITE
const int COLOR_SHORTEST_PATH = 4; // RED
const int COLOR_NEGATIVE_CYCLE = 12; // LIGHTRED

#endif
