#ifndef IO_H
#define IO_H

void getFilename(char *fname, HWND hwnd);
HBITMAP loadBitmap(char *filename, int width, int height);
void drawBitmap(HWND hwnd, HBITMAP bmp_todraw);
void displayWorkingDir(HWND hwnd);

#endif
