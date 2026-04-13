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
    // thiết lập màu thích đổi màu giề thì đổi ở đây.

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


static void plotPoint(int col, int row, const char* str = "*") {
    if (!g_ansi_enabled) return;
    if (col < 1 || col > g_console_cols || row < 1 || row > g_console_rows) return;
    moveCursor(row, col);
    applyColor();
    std::cout << str;
}

void line(int x1, int y1, int x2, int y2) {
    if (!g_ansi_enabled) return;
    int c1 = mapX(x1);
    int r1 = mapY(y1);
    int c2 = mapX(x2);
    int r2 = mapY(y2);

    int dc = abs(c2 - c1), sc = c1 < c2 ? 1 : -1;
    int dr = -abs(r2 - r1), sr = r1 < r2 ? 1 : -1;
    int err = dc + dr, e2;

    while (true) {
        const char* c = "*";
        if (dc == 0) c = "│";
        else if (dr == 0) c = "─";
        else if ((sc > 0 && sr > 0) || (sc < 0 && sr < 0)) c = "╲";
        else c = "╱";

        plotPoint(c1, r1, c);
        if (c1 == c2 && r1 == r2) break;
        e2 = 2 * err;
        if (e2 >= dr) { err += dr; c1 += sc; }
        if (e2 <= dc) { err += dc; r1 += sr; }
    }
    std::cout << std::flush;
}

#include <cmath>

void circle(int x, int y, int radius) {
    if (!g_ansi_enabled) return;
    int c = mapX(x);
    int r = mapY(y);
    int radC = mapX(x + radius) - c;
    int radR = mapY(y + radius) - r;

    for (int angle = 0; angle < 360; angle += 5) {
        double rad = angle * 3.14159265 / 180.0;
        int plotC = c + (int)(radC * std::cos(rad));
        int plotR = r + (int)(radR * std::sin(rad));
        plotPoint(plotC, plotR, "o");
    }
    std::cout << std::flush;
}

void fillellipse(int x, int y, int xradius, int yradius) {
    if (!g_ansi_enabled) return;
    int c = mapX(x);
    int r = mapY(y);
    int radC = mapX(x + xradius) - c;
    int radR = mapY(y + yradius) - r;

    for (int row = r - radR; row <= r + radR; ++row) {
        for (int col = c - radC; col <= c + radC; ++col) {
            double dc = (col - c) / (double)(radC > 0 ? radC : 1);
            double dr = (row - r) / (double)(radR > 0 ? radR : 1);
            if (dc * dc + dr * dr <= 1.0) {
                plotPoint(col, row, "O");
            }
        }
    }
    std::cout << std::flush;
}

void bar(int left, int top, int right, int bottom) {
    if (!g_ansi_enabled) return;
    int c1 = mapX(left);
    int r1 = mapY(top);
    int c2 = mapX(right);
    int r2 = mapY(bottom);
    if (c1 > c2) std::swap(c1, c2);
    if (r1 > r2) std::swap(r1, r2);

    for (int r = r1; r <= r2; ++r) {
        for (int c = c1; c <= c2; ++c) {
            plotPoint(c, r, "#"); 
        }
    }
    std::cout << std::flush;
}

void setfillstyle(int pattern, int color) {
    (void)pattern;
    setcolor(color);
}

void settextstyle(int font, int direction, int charsize) {
    (void)font;
    (void)direction;
    (void)charsize;
}

int getch() {
    int c = std::getchar();
    return c;
}
