#ifndef WIN32_H
#define WIN32_H
#include <windows.h>
#include <commctrl.h>
#include <stdio.h> /* for FILE, fprint, fread, stdin, stdout, stderr */
#include "../res/resource.h"
#include "../include/definitions.h"

#ifdef _MSC_VER
typedef __int16 int16_t;
typedef unsigned __int16 uint16_t;
typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
#else
#include <stdint.h>
#endif

/* must come after stdint.h */
#include "../../include/bmp.h" 

#define DEFAULT_BMP_FILENAME "ex.bmp"

/* function prototypes */
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK AboutDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
HWND createListView(HWND hwndParent);
HMENU createMenu(HWND hwnd);
HWND createStatusBar(HWND hwnd);

#endif
