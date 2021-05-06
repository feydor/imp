/* controls.c - functions for creating win32 controls in memory */
#include "../include/win32.h"
#include "../include/controls.h"

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
