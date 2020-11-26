#include "Window.h"

LRESULT CALLBACK Window::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg)
	{
	case WM_CREATE:
		// Adds an icon to the tooltray
		if (!AddTrayIcon(hwnd))
		{
			PostQuitMessage(0);
			return -1;
		}
		break;

	case WM_COMMAND:
		// User selected the quit button
		switch (LOWORD(wParam)) {
		case ID__EXIT:
			DestroyWindow(hwnd);
			PostQuitMessage(0);
			break;
		case ID__VISITORS:
			visitorsToggled = !visitorsToggled;
			break;
		case ID__FORCEVISIT:
			visitorsToggled = true;
			forceVisitor = true;
			break;
		} // Continue otherwise...

	case WM_MENUOPEN:
		// User opened the menu
		if (LOWORD(lParam) == WM_CONTEXTMENU)
			ShowContextMenu(hwnd, { LOWORD(wParam), HIWORD(wParam) });
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	return 0;
};

void Window::RegisterWindowClass(PCWSTR className, PCWSTR menuName, WNDPROC wndProc)
{
	WNDCLASSEX wcex = { sizeof(wcex) };

	wcex.style         = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc   = wndProc;
	wcex.hInstance     = hInst;
	wcex.hIcon         = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON));
	wcex.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName  = menuName;
	wcex.lpszClassName = className;

	// Then it just gets disposed? Why is this necessary?
	RegisterClassEx(&wcex);
}

void Window::MakeWindow(HINSTANCE h) {
	hInst = h;

	// Register the window class
	RegisterWindowClass(className, MAKEINTRESOURCE(IDC_ICON), WindowProc);

	// Because regular strings are apparently not enough
	WCHAR tmpTitle[128];
	LoadString(hInst, IDS_TITLE, tmpTitle, ARRAYSIZE(tmpTitle));
	titleName = tmpTitle;

	HWND hwnd = CreateWindowEx(
		0,
		className,
		titleName,
		WS_OVERLAPPEDWINDOW,

		// Size and position
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

		NULL,
		NULL,
		hInst,
		NULL
	);

	// Can't be bothered
	if (hwnd == NULL) return;

	// Isn't this an oxymoron
	ShowWindow(hwnd, SW_HIDE);

	running = true;
}

void Window::MessageLoop() {
	MSG msg;
	// GetMessage WILL block the program, call important stuff in a thread.
	// You can't call THIS in a thread because windows doesn't allow that, run YOUR stuff in a thread
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	running = false;
}

BOOL Window::AddTrayIcon(HWND hwnd)
{
	NOTIFYICONDATA nid   = { sizeof(nid) };
	nid.hWnd             = hwnd;
	nid.uFlags           = NIF_ICON | NIF_TIP | NIF_MESSAGE | NIF_SHOWTIP;
	nid.uCallbackMessage = WM_MENUOPEN;
	nid.hIcon            = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON));
	// .szTip            = IDS_TOOLTIP
	LoadString(hInst, IDS_TOOLTIP, nid.szTip, ARRAYSIZE(nid.szTip));

	// Spawns the icon
	Shell_NotifyIcon(NIM_ADD, &nid);

	nid.uVersion = NOTIFYICON_VERSION_4;
	return Shell_NotifyIcon(NIM_SETVERSION, &nid);
}

void Window::ShowContextMenu(HWND hwnd, point pt) {
	HMENU hMenu = LoadMenu(hInst, MAKEINTRESOURCE(IDC_CONTEXTMENU));

	if (hMenu) {
		HMENU hSubMenu = GetSubMenu(hMenu, 0);
		if (hSubMenu) {
			WCHAR strMenuString[27];
			WCHAR tmp[27];
			GetMenuString(hSubMenu, ID__VISITORS, strMenuString, 27, MF_BYCOMMAND);
			LoadString(hInst, IDS_TOGGLE_DISABLED, tmp, 27);
			if(visitorsToggled) {
				LoadString(hInst, IDS_TOGGLE_ENABLED, strMenuString, 27);
			}
			else
			{
				LoadString(hInst, IDS_TOGGLE_DISABLED, strMenuString, 27);
			}

			BOOL bChanged = ModifyMenu(hSubMenu, ID__VISITORS, MF_BYCOMMAND | MF_STRING, ID__VISITORS, (LPCTSTR)strMenuString);

			// The window must be foreground before calling TrackPopupMenu or the menu will not disappear when the user clicks away
			SetForegroundWindow(hwnd);

			// Drop alignment
			int uFlags = TPM_RIGHTBUTTON;
			if (GetSystemMetrics(SM_MENUDROPALIGNMENT) != 0)
			{
				uFlags |= TPM_RIGHTALIGN;
			}
			else uFlags |= TPM_LEFTALIGN;

			TrackPopupMenuEx(hSubMenu, uFlags, pt.x, pt.y, hwnd, NULL);
		}
		DestroyMenu(hMenu);
	}
}

// What else would I call it?
const WCHAR* Window::className = L"Window class";
HINSTANCE Window::hInst = NULL;
WCHAR* Window::titleName = 0;
HWND Window::hwnd = 0;
bool Window::running = false;
bool Window::visitorsToggled = true;
bool Window::forceVisitor = false;