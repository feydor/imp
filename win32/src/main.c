#include "../include/win32.h"
#include "../include/control.h"
#include "../include/fileio.h"

/* globals */
static HBITMAP g_bmp = NULL;
static unsigned char *g_bytes = NULL;

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
						BMPInfoHeader biHeader;

						// populate szFileName
						getFilename(szFileName, hwnd);
						
						// load bmp
						/*
						g_bmp = loadBitmap(szFileName, 64, 64);
						if (g_bmp == NULL) {
							MessageBox(hwnd, "Could not load bmp!", "Error",
								MB_OK | MB_ICONEXCLAMATION);
						}
						*/

						g_bytes = parse_24bit_bmp_filename(szFileName, &biHeader);
						
						if (!g_bytes)
							MessageBox(hwnd, "Could not load bmp bytes array!", "Error",
								MB_OK | MB_ICONEXCLAMATION);

                        // send message to redraw current window
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
                // displayWorkingDir(hwnd);
                // displayInvertedBmp(hwnd, g_bmp);
			}
			break;
        case WM_CLOSE:
            DestroyWindow(hwnd);
			break;
        case WM_DESTROY:
			DeleteObject(g_bmp);
			free(g_bytes);
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
