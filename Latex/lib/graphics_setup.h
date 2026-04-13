#ifndef GRAPHICS_SETUP_H
#define GRAPHICS_SETUP_H

// WinBGIm graphics library setup for Windows
#include "graphics.h"
#include <conio.h>
#include <iostream>

// Graphics setup constants
const int GRAPH_WIDTH = 1200;
const int GRAPH_HEIGHT = 800;

// Function to initialize graphics window
inline void initGraphicsWindow() {
    int gd = DETECT, gm;
    initgraph(&gd, &gm, "C:\\MinGW\\lib\\libbgi.a");
    
    if (graphresult() != grOk) {
        std::cerr << "Graphics initialization failed!" << std::endl;
        exit(1);
    }
    
    setcolor(WHITE);
    setbkcolor(BLACK);
    cleardevice();
}

// Function to close graphics window
inline void closeGraphicsWindow() {
    getch();
    closegraph();
}

#endif
