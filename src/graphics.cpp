#include "../lib/graphics.h"
#include <cstdio>
#include <iostream>
#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

static int g_graph_result = 0;
static bool g_ansi_enabled = false;
static int g_current_color = WHITE;

// Virtual window size (pixels) -> console size (cols/rows)
static const int g_virtual_width = 1200;
static const int g_virtual_height = 800;
static const int g_console_cols = 120;
static const int g_console_rows = 40;

static void setConsoleUtf8() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
}

static void enableAnsi() {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) {
        g_ansi_enabled = false;
        return;
    }
    DWORD mode = 0;
    if (!GetConsoleMode(hOut, &mode)) {
        g_ansi_enabled = false;
        return;
    }
    mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if (!SetConsoleMode(hOut, mode)) {
        g_ansi_enabled = false;
        return;
    }
#endif
    setConsoleUtf8();
    g_ansi_enabled = true;
}

static int mapX(int x) {
    if (x < 0) x = 0;
    if (x > g_virtual_width) x = g_virtual_width;
    int col = (x * g_console_cols) / g_virtual_width;
    if (col < 1) col = 1;
    if (col > g_console_cols) col = g_console_cols;
    return col;
}

static int mapY(int y) {
    if (y < 0) y = 0;
    if (y > g_virtual_height) y = g_virtual_height;
    int row = (y * g_console_rows) / g_virtual_height;
    if (row < 1) row = 1;
    if (row > g_console_rows) row = g_console_rows;
    return row;
}

static int ansiColorCode(int color) {
    switch (color) {
        case BLACK: return 30;
        case BLUE: return 34;
        case GREEN: return 32;
        case CYAN: return 36;
        case RED: return 31;
        case MAGENTA: return 35;
        case BROWN: return 33;
        case LIGHTGRAY: return 37;
        case DARKGRAY: return 90;
        case LIGHTBLUE: return 94;
        case LIGHTGREEN: return 92;
        case LIGHTCYAN: return 96;
        case LIGHTRED: return 91;
        case LIGHTMAGENTA: return 95;
        case YELLOW: return 93;
        case WHITE: return 97;
        default: return 37;
    }
}

static void moveCursor(int row, int col) {
    if (!g_ansi_enabled) return;
    std::cout << "\x1b[" << row << ";" << col << "H";
}

static void applyColor() {
    if (!g_ansi_enabled) return;
    std::cout << "\x1b[" << ansiColorCode(g_current_color) << "m";
}

static bool writeUtf8(const char* text) {
    if (!text) return false;
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) {
        return false;
    }
    int wlen = MultiByteToWideChar(CP_UTF8, 0, text, -1, nullptr, 0);
    if (wlen <= 0) {
        return false;
    }
    std::wstring wstr(static_cast<size_t>(wlen), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, text, -1, &wstr[0], wlen);
    DWORD written = 0;
    BOOL ok = WriteConsoleW(hOut, wstr.c_str(), static_cast<DWORD>(wlen - 1), &written, nullptr);
    return ok != FALSE;
#else
    std::cout << text;
    return true;
#endif
}

void initgraph(int *graphdriver, int *graphmode, char *pathtodriver) {
    (void)graphdriver; (void)graphmode; (void)pathtodriver;
    g_graph_result = 0; // grOk
    enableAnsi();
    cleardevice();
}

int graphresult() {
    return g_graph_result;
}

void closegraph() {
    if (g_ansi_enabled) {
        std::cout << "\x1b[0m" << std::flush;
    }
}

void cleardevice() {
    if (g_ansi_enabled) {
        std::cout << "\x1b[2J\x1b[H" << std::flush;
    } else {
        std::cout << std::string(50, '\n');
    }
}

void clearviewport() {}

void setcolor(int color) {
    g_current_color = color;
}

void setbkcolor(int color) { (void)color; }

void outtextxy(int x, int y, char *textstring) {
    if (!textstring) return;
    if (g_ansi_enabled) {
        int row = mapY(y);
        int col = mapX(x);
        moveCursor(row, col);
        applyColor();
        if (!writeUtf8(textstring)) {
            std::cout << textstring;
        }
        std::cout << std::flush;
    } else {
        if (!writeUtf8(textstring)) {
            std::cout << textstring;
        }
        std::cout << std::endl;
    }
}

void circle(int x, int y, int radius) { (void)x; (void)y; (void)radius; }
void line(int x1, int y1, int x2, int y2) { (void)x1; (void)y1; (void)x2; (void)y2; }

void rectangle(int left, int top, int right, int bottom) {
    if (!g_ansi_enabled) return;
    int r1 = mapY(top);
    int r2 = mapY(bottom);
    int c1 = mapX(left);
    int c2 = mapX(right);
    if (r2 <= r1 || c2 <= c1) return;

    applyColor();
    // canh tren va duoi
    moveCursor(r1, c1);
    std::cout << "+" << std::string(c2 - c1 - 1, '=') << "+";
    moveCursor(r2, c1);
    std::cout << "+" << std::string(c2 - c1 - 1, '-') << "+";
    //khung phia ngoai (hai ben)
    for (int r = r1+1; r < r2; r++) {
        moveCursor(r, c1);
        std::cout << "|";
        std::cout << "|";

        moveCursor(r, c2);
        std::cout << "|";
    }
    std::cout << std::flush;
}

int getch() {
    int c = std::getchar();
    return c;
}

int kbhit() {
    return 0; // not implemented
}
