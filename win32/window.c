#include "menu.h"
#include "resource.h"
#include <windows.h>
#include <commctrl.h>

#define DEFAULT_BMP_FILENAME "ex.bmp"

/* globals */
HBITMAP g_bmp = NULL;

/* function prototypes */
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK AboutDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
HWND createListView(HWND hwndParent);
HMENU createMenu(HWND hwnd);
HWND createStatusBar(HWND hwnd);
void getFilename(char *fname, HWND hwnd);
HBITMAP loadBitmap(char *filename, int width, int height);
void drawBitmap(HWND hwnd, HBITMAP bmp_todraw);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow) 
{
    WNDCLASSEX wc;
    HWND hwnd;
    MSG Msg;
	const char g_szClassName[] = "A Unique Window Classname";

    //Step 1: Registering the Window Class
    wc.cbSize        = sizeof(WNDCLASSEX);
    wc.style         = 0;
    wc.lpfnWndProc   = WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = g_szClassName;
    wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

    if(!RegisterClassEx(&wc)) {
        MessageBox(NULL, "Window Registration Failed!", "Error!",
            MB_ICONERROR | MB_OK);
        return 0;
    }

    // Step 2: Creating the Window
    hwnd = CreateWindowEx (
        WS_EX_CLIENTEDGE,
        g_szClassName,
        "A unique window title",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 240, 240,
        NULL, NULL, hInstance, NULL);

    if(hwnd == NULL) {
        MessageBox(NULL, "Window Creation Failed!", "Error!",
            MB_ICONERROR | MB_OK);
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

	InitCommonControls(); // comctl32.dll is loaded

    // Step 3: The Message Loop
    while(GetMessage(&Msg, NULL, 0, 0) > 0) {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg); // calls the window's Window Procedure
    }
    return Msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, 
						 WPARAM wParam, LPARAM lParam)
{
    switch(msg) {
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case ID_FILE_OPEN:
					{
						char szFileName[MAX_PATH] = "";

						// populate szFileName
						getFilename(szFileName, hwnd);
						
						// load bmp
						g_bmp = loadBitmap(szFileName, 64, 64);
						if (g_bmp == NULL) {
							MessageBox(hwnd, "Could not load bmp!", "Error",
								MB_OK | MB_ICONEXCLAMATION);
						}
						
						// send message to wm_paint
						InvalidateRect(hwnd, NULL, FALSE);

						// populate listview
						DlgDirList(hwnd, szFileName, IDC_LIST_VIEW,
							0, 0);
						SetFocus(GetDlgItem(hwnd, IDC_LIST_VIEW));
					}
					break;
				case ID_FILE_EXIT:
					PostMessage(hwnd, WM_CLOSE, 0, 0);
					break;
				case ID_STUFF_GO:
					MessageBox(NULL, "Stuff --> Go",
						"A unique message box title",
						MB_ICONINFORMATION | MB_OK);
					break;
				case ID_HELP_ABOUT:
					{
						int ret = DialogBox(GetModuleHandle(NULL),
							MAKEINTRESOURCE(IDD_ABOUT), hwnd, AboutDlgProc);
						
						/*
						if (ret == IDOK) {
							MessageBox(hwnd, "Dialog exited with IDOK.",
								"Notice", MB_OK | MB_ICONINFORMATION);
						} else if (ret == IDCANCEL) {
							MessageBox(hwnd, "Dialog exited with IDCANCEL.",
								"Notice", MB_OK | MB_ICONINFORMATION);
						} else*/ if (ret == -1) {
							MessageBox(hwnd, "Dialog failed!", "Error",
								MB_OK | MB_ICONINFORMATION);
						}
					}
					break;
			}
			break;
		case WM_CREATE:
        {
			HWND hListView;
			HFONT hfDefault;

			// create menu
			createMenu(hwnd);

			// create listview
			/*
			hListView = createListView(hwnd);
			hfDefault = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
			SendMessage(hListView, WM_SETFONT, (WPARAM)hfDefault,
				MAKELPARAM(FALSE, 0));
			*/
				
			// create statusbar
			createStatusBar(hwnd);

			// load default bitmap
			/*
			g_bmp = loadBitmap(DEFAULT_BMP_FILENAME, 64, 64);
			if (g_bmp == NULL) {
				MessageBox(hwnd, "Could not load bmp!", "Error",
					MB_OK | MB_ICONEXCLAMATION);
			}
			*/
        }
        break;
		case WM_SIZE: // when resizing the window
			{
				HWND hListView, hStatus;
			    RECT rcStatus;
				int listViewHeight, statusHeight;
				RECT rcClient;

				// size statusbar and get height
				hStatus = GetDlgItem(hwnd, IDC_MAIN_STATUS);
				SendMessage(hStatus, WM_SIZE, 0, 0);
				GetWindowRect(hStatus, &rcStatus);
				statusHeight = rcStatus.bottom - rcStatus.top;

				GetClientRect(hwnd, &rcClient);
				listViewHeight = rcClient.bottom - statusHeight;

				hListView = GetDlgItem(hwnd, IDC_LIST_VIEW);
				SetWindowPos(hListView, NULL, 0, 0, rcClient.right, 
					listViewHeight / 6, SWP_NOZORDER);
			}
			break;
		case WM_PAINT:
			{
				drawBitmap(hwnd, g_bmp);
			}
			break;
		case WM_LBUTTONDOWN:
			{
				char szFileName[MAX_PATH];
				HINSTANCE hInstance = GetModuleHandle(NULL);
				GetModuleFileName(hInstance, szFileName, MAX_PATH);
				MessageBox(hwnd, szFileName, "This program is: ", 
					MB_OK | MB_ICONEXCLAMATION);
			}
			break;
        case WM_CLOSE:
            DestroyWindow(hwnd);
			break;
        case WM_DESTROY:
			DeleteObject(g_bmp);
            PostQuitMessage(0);
			break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

BOOL CALLBACK AboutDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	switch (msg) {
		case WM_INITDIALOG:
			return TRUE;
		case WM_COMMAND:
			switch ( LOWORD(wParam) ) {
				case IDOK:
					EndDialog(hwnd, IDOK);
					break;
				case IDCANCEL:
					EndDialog(hwnd, IDCANCEL);
					break;
			}
			break;
			default:
				return FALSE;
	}
	return TRUE;
}

HWND createListView(HWND hwndParent) 
{
	/*
	INITCOMMONCONTROLSEX icex;
	icex.dwICC = ICC_LISTVIEW_CLASSES;
	InitCommonControlsEx(&icex);

	RECT rcClient; // the parent window's client area
	GetClientRect(hwndParent, &rcClient);
	*/
	HWND hListView = CreateWindowEx(WS_EX_CLIENTEDGE, "LISTBOX", "", 
		WS_CHILD | WS_VISIBLE,
		0, 0,
		100,
		100,
		hwndParent,
		(HMENU)IDC_LIST_VIEW,
		GetModuleHandle(NULL),
		NULL);
	if(hListView == NULL)
        MessageBox(hwndParent, "Could not create list box.", "Error", 
			MB_OK | MB_ICONERROR);
	return hListView;
}

HWND createStatusBar(HWND hwnd) 
{
	HWND hStatus;
	int statwidths[] = {100, -1};

    hStatus = CreateWindowEx(0, STATUSCLASSNAME, NULL,
		WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP, 0, 0, 0, 0,
		hwnd, (HMENU)IDC_MAIN_STATUS, GetModuleHandle(NULL), NULL);

    SendMessage(hStatus, SB_SETPARTS, sizeof(statwidths)/sizeof(int), 
		(LPARAM)statwidths);
    SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM)"Hi there :)");
	return hStatus;
}

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

HMENU createMenu(HWND hwnd) 
{
	HMENU hMenu, hSubMenu;
	HICON hIcon, hIconSm;

	hMenu = CreateMenu();

	hSubMenu = CreatePopupMenu();
	AppendMenu(hSubMenu, MF_STRING, ID_FILE_OPEN, "&Open");
	AppendMenu(hSubMenu, MF_STRING, ID_FILE_EXIT, "&Exit");
	AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT)hSubMenu, "&File");

	hSubMenu = CreatePopupMenu();
	AppendMenu(hSubMenu, MF_STRING, ID_STUFF_GO, "&Go");
	AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT)hSubMenu, "&Stuff");
	
	hSubMenu = CreatePopupMenu();
	AppendMenu(hSubMenu, MF_STRING, ID_HELP_ABOUT, "&About");
	AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT)hSubMenu, "&Help");

	SetMenu(hwnd, hMenu);

	hIcon = LoadImage(NULL, "desktop.ico", IMAGE_ICON, 32, 32, 
		LR_LOADFROMFILE);
    if (hIcon)
        SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
    else
        MessageBox(hwnd, "Could not load large icon!", "Error",
			MB_OK | MB_ICONERROR);

    hIconSm = LoadImage(NULL, "desktop.ico", IMAGE_ICON, 16, 16, 
		LR_LOADFROMFILE);
    if (hIconSm)
        SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIconSm);
    else
        MessageBox(hwnd, "Could not load small icon!", "Error",
			MB_OK | MB_ICONERROR);

	return hMenu;
}

HBITMAP loadBitmap(char *filename, int width, int height) 
{
	return LoadImage(NULL, filename, IMAGE_BITMAP,
		width, height, LR_LOADFROMFILE | LR_DEFAULTSIZE);
}

void drawBitmap(HWND hwnd, HBITMAP bmp_todraw) {
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