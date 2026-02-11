#ifndef PROJECT_GRAPHICS_H
#define PROJECT_GRAPHICS_H

#ifdef __cplusplus
extern "C" {
#endif

void initgraph(int *graphdriver, int *graphmode, char *pathtodriver);
int graphresult();
void closegraph();
void cleardevice();
void clearviewport();

void setcolor(int color);
void setbkcolor(int color);
void outtextxy(int x, int y, char *textstring);
void circle(int x, int y, int radius);
void line(int x1, int y1, int x2, int y2);
void rectangle(int left, int top, int right, int bottom);

int getch();
int kbhit();

enum colors { BLACK=0, BLUE=1, GREEN=2, CYAN=3, RED=4, MAGENTA=5, BROWN=6, LIGHTGRAY=7, DARKGRAY=8,
    LIGHTBLUE=9, LIGHTGREEN=10, LIGHTCYAN=11, LIGHTRED=12, LIGHTMAGENTA=13, YELLOW=14, WHITE=15 };

const int grOk = 0;
const int DETECT = 0;
const int ALL_WINDOWS = -2;

#ifdef __cplusplus
}
#endif

#endif 
