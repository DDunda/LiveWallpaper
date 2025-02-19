#include "window.h"
#include "globals.h"
#include"resource.h"

WindowClass::WindowClass(LPCTSTR menu_name, LPCTSTR class_name, WNDPROC window_procedure, UINT style, HBRUSH background)
{
	_wcex = {};

	_wcex.cbSize        = sizeof(_wcex);
	_wcex.style         = style;
	_wcex.lpfnWndProc   = window_procedure;
	_wcex.hInstance     = hInstance;
	_wcex.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON));
	_wcex.hCursor       = LoadCursor(NULL, IDC_ARROW);
	_wcex.hbrBackground = background;
	_wcex.lpszMenuName  = menu_name;
	_wcex.lpszClassName = class_name;

	_class_atom = RegisterClassEx(&_wcex);

	if(_class_atom == 0) throw std::exception("Failed to create window class!");
}

WindowClass::~WindowClass()
{
	UnregisterClass(MAKEINTATOM(_class_atom), hInstance);
}

Window::Window(ATOM class_atom, LPCTSTR title_name, int x, int y, int w, int h, HWND parent, DWORD style, DWORD ex_style)
{
	hwnd = CreateWindowEx(
		ex_style, // DWORD dwExStyle
		MAKEINTATOM(class_atom), // LPCWSTR lpClassName
		title_name, // LPCWSTR lpWindowName
		style, // DWORD dwStyle

		// Size and position
		x, y, w, h,

		parent, // HWND hWndParent
		HMENU(), // HMENU hMenu
		hInstance, // HINSTANCE hInstance
		NULL // LPVOID lpParam
	);

	if (hwnd == NULL) throw std::exception("Failed to create window!");
}

Window::~Window()
{
	DestroyWindow(hwnd);

	hwnd = 0;
}

// If you wish to run this function on a thread, then the containing object must also be created by the same thread
// This is because message queues are generated per-thread
void Window::MessageLoop()
{
	MSG msg;
	running = true;
	while (running && GetMessage(&msg, hwnd, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	running = false;
}

BOOL Window::AddTrayIcon(HWND hwnd, UINT tooltip_ID, UINT& uID_out)
{
	NOTIFYICONDATA nid;
	nid.cbSize           = sizeof(nid);
	nid.hWnd             = hwnd;
	nid.uFlags           = NIF_ICON | NIF_TIP | NIF_MESSAGE | NIF_SHOWTIP;
	nid.uCallbackMessage = WM_MENUOPEN;
	nid.hIcon            = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON));

	LoadString(
		hInstance,
		tooltip_ID,
		nid.szTip,
		ARRAYSIZE(nid.szTip)
	);

	// Spawns the icon
	Shell_NotifyIcon(NIM_ADD, &nid);

	uID_out = nid.uID;

	nid.uVersion = NOTIFYICON_VERSION_4;
	return Shell_NotifyIcon(NIM_SETVERSION, &nid);
}