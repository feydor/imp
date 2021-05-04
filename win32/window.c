#include "menu.h"
#include "resource.h"
#include <windows.h>

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK AboutDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void getFilename(char *fname, HWND hwnd);
HMENU buildMenu();

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow) {
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
        CW_USEDEFAULT, CW_USEDEFAULT, 240, 120,
        NULL, NULL, hInstance, NULL);

    if(hwnd == NULL) {
        MessageBox(NULL, "Window Creation Failed!", "Error!",
            MB_ICONERROR | MB_OK);
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // Step 3: The Message Loop
    while(GetMessage(&Msg, NULL, 0, 0) > 0) {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg); // calls the window's Window Procedure
    }
    return Msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam){
    switch(msg) {
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case ID_FILE_OPEN:
					{
						char szFileName[MAX_PATH] = "";
						getFilename(szFileName, hwnd); // szFileName is populated
					}
					break;
				case ID_FILE_EXIT:
					PostMessage(hwnd, WM_CLOSE, 0, 0);
					break;
				case ID_STUFF_GO:
					MessageBox(NULL, "Stuff --> Go", "A unique message box title",
						MB_ICONINFORMATION | MB_OK);
					break;
				case ID_HELP_ABOUT:
					{
						int ret = DialogBox(GetModuleHandle(NULL),
							MAKEINTRESOURCE(IDD_ABOUT), hwnd, AboutDlgProc);
						if (ret == IDOK) {
							MessageBox(hwnd, "Dialog exited with IDOK.", "Notice",
								MB_OK | MB_ICONINFORMATION);
						} else if (ret == IDCANCEL) {
							MessageBox(hwnd, "Dialog exited with IDCANCEL.", "Notice",
								MB_OK | MB_ICONINFORMATION);
						} else if (ret == -1) {
							MessageBox(hwnd, "Dialog failed!", "Error",
								MB_OK | MB_ICONINFORMATION);
						}
					}
					break;
			}
			break;
		case WM_CREATE:
        {
            HICON hIcon, hIconSm;
			HMENU hMenu = buildMenu();

            SetMenu(hwnd, hMenu);

            hIcon = LoadImage(NULL, "desktop.ico", IMAGE_ICON, 32, 32, LR_LOADFROMFILE);
            if(hIcon)
                SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
            else
                MessageBox(hwnd, "Could not load large icon!", "Error",
					MB_OK | MB_ICONERROR);

            hIconSm = LoadImage(NULL, "desktop.ico", IMAGE_ICON, 16, 16, LR_LOADFROMFILE);
            if(hIconSm)
                SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIconSm);
            else
                MessageBox(hwnd, "Could not load small icon!", "Error",
					MB_OK | MB_ICONERROR);
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
            PostQuitMessage(0);
			break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

BOOL CALLBACK AboutDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
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

void getFilename(char *fname, HWND hwnd) {
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

HMENU buildMenu() {
	HMENU hMenu, hSubMenu;

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

	return hMenu;
}