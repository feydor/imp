#ifndef FILEIO_H
#define FILEIO_H

struct BMPInfoHeader;

void getFilename(char *fname, HWND hwnd);
HBITMAP loadBitmapFromFile(char *fname, int width, int height);
HBITMAP bytesArrayToDIB(unsigned char *bytes, BMPInfoHeader *biHeader);
void drawBitmap(HWND hwnd, HBITMAP bmp_todraw);
void displayWorkingDir(HWND hwnd);

#endif
