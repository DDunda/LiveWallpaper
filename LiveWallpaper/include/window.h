#pragma once

#include<string>
#include<Windows.h>
#include "DrawingHelpers.h"

#define WM_MENUOPEN (WM_APP + 1)

struct WindowClass
{
	WNDCLASSEX _wcex;
	ATOM _class_atom;

	WindowClass(LPCTSTR menu_name, LPCTSTR class_name, WNDPROC window_procedure, UINT style = CS_HREDRAW | CS_VREDRAW);
	~WindowClass();
};

// Code relevant to rendering in the system tray and rendering the exit popup is taken from here:
// https://docs.microsoft.com/en-us/windows/win32/shell/samples-notificationicon#downloading-the-sample
// I really have no clue what it does but it works, so nice!
struct Window
{
	HWND hwnd;
	bool running = false;

	Window(ATOM class_atom, LPCTSTR title_name, int x, int y, int w, int h, HWND parent = NULL, DWORD style = WS_OVERLAPPEDWINDOW);

	// Destroys the window
	~Window();

	// Recieves messages and handballs them to WindowProc
	void MessageLoop();

	// Creates an icon for this window to the system tray
	static BOOL AddTrayIcon(HWND hwnd, UINT tooltip_ID, UINT& uID_out);
};