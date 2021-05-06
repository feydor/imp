/* fileio.c - file io functions for win32 */
#include "../include/win32.h"
#include "../include/fileio.h"

void getFilename(char *fname, HWND hwnd) 
{
	OPENFILENAME ofn;
	
	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFilter = "24-bit Bitmap (*.bmp)\0*.bmp\0All Files (*.*)\0*.*\0";
	ofn.lpstrFile = fname;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.lpstrDefExt = "bmp";
	
	if(GetOpenFileName(&ofn)) {
		MessageBox(NULL, fname, "Your file",
			MB_ICONINFORMATION | MB_OK);
	}
}

HBITMAP loadBitmapFromFile(char *fname, int width, int height) 
{
	return LoadImage(NULL, fname, IMAGE_BITMAP,
		width, height, LR_LOADFROMFILE | LR_DEFAULTSIZE);
}

// allocated memry for DIB, must free by using DeleteObject on return value
HBITMAP bytesArrayToDIB(unsigned char *bytes, BMPInfoHeader *biHeader) 
{
	// populate a BITMAPINFO struct with biHeader
	BITMAPINFO bi;
	ZeroMemory(&bi, sizeof(BITMAPINFO));
	bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bi.bmiHeader.biWidth = biHeader->imageWidth;
	bi.bmiHeader.biHeight = biHeader->imageHeight;
	bi.bmiHeader.biPlanes = biHeader->planes;
	bi.bmiHeader.biBitCount = biHeader->bitsPerPxl;
	
	// create an HBITMAP
	return CreateDIBSection(NULL, &bi, DIB_RGB_COLORS,
		(VOID**)bytes, NULL, 0);
}

void drawBitmap(HWND hwnd, HBITMAP bmp_todraw) 
{
	BITMAP bm;
	PAINTSTRUCT ps;
  
	HDC hdc = BeginPaint(hwnd, &ps);
	HDC hdcMem = CreateCompatibleDC(hdc);
	HBITMAP hbmOld = SelectObject(hdcMem, 
		bmp_todraw);

	GetObject(bmp_todraw, sizeof(bm), &bm);
	
	BitBlt(hdc, 0, 0, bm.bmWidth, bm.bmHeight, 
		hdcMem, 0, 0, SRCCOPY);

	SelectObject(hdcMem, hbmOld);
	DeleteDC(hdcMem);

	EndPaint(hwnd, &ps);  
}

void displayWorkingDir(HWND hwnd) 
{
  char szFileName[MAX_PATH];
  HINSTANCE hInstance = GetModuleHandle(NULL);
  GetModuleFileName(hInstance, szFileName, MAX_PATH);
  if (szFileName)
    MessageBox(hwnd, szFileName, "This program is: ", 
        MB_OK | MB_ICONEXCLAMATION);
  else
    MessageBox(hwnd, "displayWorkingDir blew up...", "Error: ", 
        MB_OK | MB_ICONEXCLAMATION);
}
