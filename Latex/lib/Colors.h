#ifndef COLORS_H
#define COLORS_H

#ifdef COLOR_BACKGROUND
#undef COLOR_BACKGROUND
#endif



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
